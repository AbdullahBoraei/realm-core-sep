///////////////////////////////////////////////////////////////////////////
//
// Copyright 2024 Realm Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
////////////////////////////////////////////////////////////////////////////

#ifndef REALM_NOINST_PENDING_RESET_STORE_HPP
#define REALM_NOINST_PENDING_RESET_STORE_HPP

#include <realm/status.hpp>
#include <realm/timestamp.hpp>
#include <realm/sync/client_base.hpp>
#include <realm/sync/config.hpp>
#include <realm/sync/protocol.hpp>

#include <realm/sync/noinst/sync_metadata_schema.hpp>

#include <ostream>
#include <optional>

namespace realm::sync {

struct PendingReset {
    using Action = sync::ProtocolErrorInfo::Action;
    Timestamp time;
    ClientResyncMode mode;
    Action action = Action::NoAction;
    std::optional<Status> error;
};

std::ostream& operator<<(std::ostream& os, const sync::PendingReset& pr);
bool operator==(const sync::PendingReset& lhs, const sync::PendingReset& rhs);
bool operator==(const sync::PendingReset& lhs, const PendingReset::Action& action);

class PendingResetStore {
public:
    // Store the pending reset tracking information - it is an error if the tracking info already
    // exists in the store
    // Requires a writable transaction and changes must be committed manually
    static void track_reset(const TransactionRef& wr_tr, ClientResyncMode mode, PendingReset::Action action,
                            const std::optional<Status>& error = std::nullopt);
    // Clear the pending reset tracking information, if it exists
    // Requires a writable transaction and changes must be committed manually
    static void clear_pending_reset(const TransactionRef& wr_tr);
    static std::optional<PendingReset> has_pending_reset(const TransactionRef& rd_tr);

    static int64_t from_reset_action(PendingReset::Action action);
    static PendingReset::Action to_reset_action(int64_t action);
    static ClientResyncMode to_resync_mode(int64_t mode);
    static int64_t from_resync_mode(ClientResyncMode mode);

private:
    // The instantiated class is only used internally
    PendingResetStore(const TransactionRef& rd_tr);

    std::vector<SyncMetadataTable> m_internal_tables;
    TableKey m_pending_reset_table;
    ColKey m_id;
    ColKey m_version;
    ColKey m_timestamp;
    ColKey m_recovery_mode;
    ColKey m_action;
    ColKey m_error_code;
    ColKey m_error_message;
    std::optional<int64_t> m_schema_version = std::nullopt;

    // Returns true if the schema was loaded
    static std::optional<PendingResetStore> load_schema(const TransactionRef& rd_tr);
    // Loads the schema or creates it if it doesn't exist
    // Requires a writable transaction and changes must be committed manually
    static PendingResetStore load_or_create_schema(const TransactionRef& wr_tr);

    // Try to read the pending reset info from v1 of the schema
    static std::optional<PendingReset> read_legacy_pending_reset(const TransactionRef& rd_tr);
};

} // namespace realm::sync

#endif // REALM_NOINST_PENDING_RESET_STORE_HPP
