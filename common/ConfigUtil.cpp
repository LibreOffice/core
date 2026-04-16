/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/*
 * Configuration file utilities and helpers.
 * Functions: getString(), getInt(), getBool() - Config value accessors
 */

#include <config.h>

#include "ConfigUtil.hpp"

#include <common/Util.hpp>

#include <Poco/Util/AbstractConfiguration.h>
#include <Poco/Util/XMLConfiguration.h>

#include <cassert>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>


namespace ConfigUtil
{
static const Poco::Util::AbstractConfiguration* Config = nullptr;

#if ENABLE_SSL
RuntimeConstant<bool> SslEnabled;
RuntimeConstant<bool> SslTermination;
#endif

// Add default values of new entries here, so there is a sensible default in case
// the setting is missing from the config file. It is possible that users do not
// update their config files, and we are backward compatible.
// These defaults should be the same
// 1) here
// 2) in the 'default' attribute in coolwsd.xml, which is for documentation
// 3) the default parameter of getConfigValue() call. That is used when the
//    setting is present in coolwsd.xml, but empty (i.e. use the default).
// NOTE: Poco doesn't index the first entry in an array, so omit '[0]'.
// NOTE: This is sorted, please keep it sorted as it's friendlier to readers,
//       except for properties, which are sorted before the value, e.g.
//       "setting[@name]" before "setting", which is more readable.
static const std::unordered_map<std::string, std::string> DefAppConfig = {
    { "accessibility.enable", "false" },
    { "admin_console.enable", "true" },
    { "admin_console.enable_pam", "false" },
    { "admin_console.logging.admin_action", "true" },
    { "admin_console.logging.admin_login", "true" },
    { "admin_console.logging.metrics_fetch", "true" },
    { "admin_console.logging.monitor_connect", "true" },
    { "admin_console.password", "" },
    { "admin_console.username", "" },
    { "allowed_languages", "de_DE en_GB en_US es_ES fr_FR it nl pt_BR pt_PT ru" },
    { "allow_update_popup", "true" },
    { "browser_logging", "false" },
    { "cache_files.path", "cache" },
    { "cache_files.expiry_min", "3000" },
    { "certificates.database_path", "" },
    { "child_root_path", "jails" },
    { "deepl.api_url", "" },
    { "deepl.auth_key", "" },
    { "deepl.enabled", "false" },
    { "document_signing.enable", "true" },
    { "enable_websocket_urp", "false" },
    { "experimental_features", "false" },
    { "extra_export_formats.impress_bmp", "false" },
    { "extra_export_formats.impress_gif", "false" },
    { "extra_export_formats.impress_png", "false" },
    { "extra_export_formats.impress_svg", "false" },
    { "extra_export_formats.impress_swf", "false" },
    { "extra_export_formats.impress_tiff", "false" },
#if ENABLE_FEATURE_LOCK
    { "feature_lock.calc_unlock_highlights", CALC_UNLOCK_HIGHLIGHTS },
    { "feature_lock.draw_unlock_highlights", DRAW_UNLOCK_HIGHLIGHTS },
    { "feature_lock.impress_unlock_highlights", IMPRESS_UNLOCK_HIGHLIGHTS },
    { "feature_lock.is_lock_readonly", "false" },
    { "feature_lock.locked_commands", LOCKED_COMMANDS },
    { "feature_lock.locked_hosts.fallback[@disabled_commands]", "false" },
    { "feature_lock.locked_hosts.fallback[@read_only]", "false" },
    { "feature_lock.locked_hosts.host", "localhost" },
    { "feature_lock.locked_hosts.host[@disabled_commands]", "false" },
    { "feature_lock.locked_hosts.host[@read_only]", "false" },
    { "feature_lock.locked_hosts[@allow]", "false" },
    { "feature_lock.unlock_description", UNLOCK_DESCRIPTION },
    { "feature_lock.unlock_link", UNLOCK_LINK },
    { "feature_lock.unlock_title", UNLOCK_TITLE },
    { "feature_lock.writer_unlock_highlights", WRITER_UNLOCK_HIGHLIGHTS },
#endif
    { "fetch_update_check", "10" },
    { "fonts_missing.handling", "log" },
    { "file_server_root_path", "browser/.." },
#if !MOBILEAPP
    { "help_url", HELP_URL },
#endif
    { "hexify_embedded_urls", "false" },
    { "home_mode.enable", "false" },
    { "indirection_endpoint.geolocation_setup.enable", "false" },
    { "indirection_endpoint.geolocation_setup.timezone", "" },
    { "indirection_endpoint.migration_timeout_secs", "180" },
    { "indirection_endpoint.server_name", "" },
    { "indirection_endpoint.url", "" },
    { "languagetool.api_key", "" },
    { "languagetool.base_url", "" },
    { "languagetool.enabled", "false" },
    { "languagetool.rest_protocol", "" },
    { "languagetool.ssl_verification", "true" },
    { "languagetool.user_name", "" },
#if !MOBILEAPP
    // { "logging.anonymize.anonymize_user_data", "false" }, // Do not set to fallback on filename/username.
    { "logging.anonymize.anonymization_salt", "82589933" },
    { "logging.color", "true" },
    { "logging.disable_server_audit", "false" },
    { "logging.disabled_areas", "Socket,WebSocket,Admin,Pixel" },
    { "logging.docstats", "false" },
    { "logging.file.property[@name]", "path" },
    { "logging.file.property", COOLWSD_LOGFILE },
    { "logging.file.property[1][@name]", "rotation" },
    { "logging.file.property[1]", "never" },
    { "logging.file.property[2][@name]", "archive" },
    { "logging.file.property[2]", "timestamp" },
    { "logging.file.property[3][@name]", "compress" },
    { "logging.file.property[3]", "true" },
    { "logging.file.property[4][@name]", "purgeAge" },
    { "logging.file.property[4]", "10 days" },
    { "logging.file.property[5][@name]", "purgeCount" },
    { "logging.file.property[5]", "10" },
    { "logging.file.property[6][@name]", "rotateOnOpen" },
    { "logging.file.property[6]", "true" },
    { "logging.file.property[7][@name]", "flush" },
    { "logging.file.property[7]", "false" },
    { "logging.file[@enable]", "false" },
    { "logging.least_verbose_level_settable_from_client", "fatal" },
    { "logging.level", COOLWSD_LOGLEVEL },
    { "logging.level_startup", "trace" },
    { "logging.lokit_sal_log", "-INFO-WARN" },
    { "logging.most_verbose_level_settable_from_client", "notice" },
    { "logging.protocol", "false" },
    { "logging.userstats", "false" },
    { "logging_ui_cmd.file.property[@name]", "path" },
    { "logging_ui_cmd.file.property", COOLWSD_LOGFILE_UICMD },
    { "logging_ui_cmd.file.property[1][@name]", "purgeCount" },
    { "logging_ui_cmd.file.property[1]", "10" },
    { "logging_ui_cmd.file.property[2][@name]", "rotateOnOpen" },
    { "logging_ui_cmd.file.property[2]", "true" },
    { "logging_ui_cmd.file.property[3][@name]", "flush" },
    { "logging_ui_cmd.file.property[3]", "false" },
    { "logging_ui_cmd.file[@enable]", "false" },
    { "logging_ui_cmd.merge", "true" },
    { "logging_ui_cmd.merge_display_end_time", "false" },
#endif
    { "mount_jail_tree", "true" },
    { "net.connection_timeout_secs", "30" },
    { "net.content_security_policy", "" },
    { "net.frame_ancestors", "" },
    { "net.listen", "any" },
    { "net.lok_allow.host", R"(192\.168\.[0-9]{1,3}\.[0-9]{1,3})" },
    { "net.lok_allow.host[10]", R"(::ffff:172\.3[01]\.[0-9]{1,3}\.[0-9]{1,3})" },
    { "net.lok_allow.host[11]", R"(10\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3})" },
    { "net.lok_allow.host[12]", R"(::ffff:10\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3})" },
    { "net.lok_allow.host[13]", R"(localhost)" },
    { "net.lok_allow.host[1]", R"(::ffff:192\.168\.[0-9]{1,3}\.[0-9]{1,3})" },
    { "net.lok_allow.host[2]", R"(127\.0\.0\.1)" },
    { "net.lok_allow.host[3]", R"(::ffff:127\.0\.0\.1)" },
    { "net.lok_allow.host[4]", R"(::1)" },
    { "net.lok_allow.host[5]", R"(172\.1[6789]\.[0-9]{1,3}\.[0-9]{1,3})" },
    { "net.lok_allow.host[6]", R"(::ffff:172\.1[6789]\.[0-9]{1,3}\.[0-9]{1,3})" },
    { "net.lok_allow.host[7]", R"(172\.2[0-9]\.[0-9]{1,3}\.[0-9]{1,3})" },
    { "net.lok_allow.host[8]", R"(::ffff:172\.2[0-9]\.[0-9]{1,3}\.[0-9]{1,3})" },
    { "net.lok_allow.host[9]", R"(172\.3[01]\.[0-9]{1,3}\.[0-9]{1,3})" },
    { "net.post_allow.host", R"(192\.168\.[0-9]{1,3}\.[0-9]{1,3})" },
    { "net.post_allow.host[10]", R"(::ffff:172\.3[01]\.[0-9]{1,3}\.[0-9]{1,3})" },
    { "net.post_allow.host[11]", R"(10\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3})" },
    { "net.post_allow.host[12]", R"(::ffff:10\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3})" },
    { "net.post_allow.host[1]", R"(::ffff:192\.168\.[0-9]{1,3}\.[0-9]{1,3})" },
    { "net.post_allow.host[2]", R"(127\.0\.0\.1)" },
    { "net.post_allow.host[3]", R"(::ffff:127\.0\.0\.1)" },
    { "net.post_allow.host[4]", R"(::1)" },
    { "net.post_allow.host[5]", R"(172\.1[6789]\.[0-9]{1,3}\.[0-9]{1,3})" },
    { "net.post_allow.host[6]", R"(::ffff:172\.1[6789]\.[0-9]{1,3}\.[0-9]{1,3})" },
    { "net.post_allow.host[7]", R"(172\.2[0-9]\.[0-9]{1,3}\.[0-9]{1,3})" },
    { "net.post_allow.host[8]", R"(::ffff:172\.2[0-9]\.[0-9]{1,3}\.[0-9]{1,3})" },
    { "net.post_allow.host[9]", R"(172\.3[01]\.[0-9]{1,3}\.[0-9]{1,3})" },
    { "net.proto", "all" },
    { "net.proxy_prefix", "false" },
    { "net.service_root", "" },
    { "num_prespawn_children", NUM_PRESPAWN_CHILDREN },
    { "overwrite_mode.enable", "false" },
    { "per_document.always_save_on_exit", "false" },
    { "per_document.autosave_duration_secs", "300" },
    { "per_document.background_autosave", "true" },
    { "per_document.background_manualsave", "true" },
    { "per_document.batch_priority", "5" },
    { "per_document.bgsave_priority", "5" },
    { "per_document.bgsave_timeout_secs", "120" },
    { "per_document.cleanup.bad_behavior_period_secs", "60" },
    { "per_document.cleanup.cleanup_interval_ms", "10000" },
    { "per_document.cleanup.idle_time_secs", "300" },
    { "per_document.cleanup.limit_cpu_per", "85" },
    { "per_document.cleanup.limit_dirty_mem_mb", "3072" },
    { "per_document.cleanup.lost_kit_grace_period_secs", "120" },
    { "per_document.cleanup[@enable]", "true" },
    { "per_document.idle_timeout_secs", "3600" },
    { "per_document.idlesave_duration_secs", "30" },
    { "per_document.limit_convert_secs", "100" },
    { "per_document.limit_file_size_mb", "0" },
    { "per_document.limit_load_secs", "100" },
    { "per_document.limit_num_open_files", "0" },
    { "per_document.limit_stack_mem_kb", "8000" },
    { "per_document.limit_store_failures", "5" },
    { "per_document.limit_virt_mem_mb", "0" },
    { "per_document.max_concurrency", "4" },
    { "per_document.min_time_between_saves_ms", "500" },
    { "per_document.min_time_between_uploads_ms", "5000" },
    { "per_document.redlining_as_comments", "false" },
    { "per_view.custom_os_info", "" },
    { "per_view.idle_timeout_secs", "900" },
    { "per_view.min_saved_message_timeout_secs", "6" },
    { "per_view.out_of_focus_timeout_secs", "300" },
    { "product_name", APP_NAME },
    { "quarantine_files.expiry_min", "3000" },
    { "quarantine_files.limit_dir_size_mb", "250" },
    { "quarantine_files.max_versions_to_maintain", "5" },
    { "quarantine_files.path", "" },
    { "quarantine_files[@enable]", "false" },
    { "view_mode.file_extensions", "" },
    { "remote_asset_config.url", "" },
    { "remote_config.remote_url", "" },
    { "remote_font_config.url", "" },
#if ENABLE_FEATURE_RESTRICTION
    { "restricted_commands", "" },
#endif
    { "security.allow_external_scripting", "false" },
    { "security.capabilities", "true" },
    { "security.enable_macros_execution", "false" },
    { "security.enable_metrics_unauthenticated", "false" },
    { "security.enable_websocket_urp", "false" },
    { "security.jwt_expiry_secs", "1800" },
    { "security.macro_security_level", "1" },
    { "security.seccomp", "true" },
    { "security.server_signature", "false" },
    { "server_name", "" },
    { "serverside_config.idle_timeout_secs", "3600" },
    { "serverside_config.max_idle_subforkits", "5" },
    { "ssl.ca_file_path", COOLWSD_CONFIGDIR "/ca-chain.cert.pem" },
    { "ssl.cert_file_path", COOLWSD_CONFIGDIR "/cert.pem" },
    { "ssl.cipher_list", "" },
    { "ssl.enable", "true" },
    { "ssl.hpkp.max_age", "1000" },
    { "ssl.hpkp.max_age[@enable]", "true" },
    { "ssl.hpkp.pins.pin", "" },
    { "ssl.hpkp.report_uri", "" },
    { "ssl.hpkp.report_uri[@enable]", "false" },
    { "ssl.hpkp[@enable]", "false" },
    { "ssl.hpkp[@report_only]", "false" },
    { "ssl.key_file_path", COOLWSD_CONFIGDIR "/key.pem" },
#if !MOBILEAPP
    { "ssl.ssl_verification", SSL_VERIFY },
#endif
    { "ssl.sts.enabled", "false" },
    { "ssl.sts.max_age", "31536000" },
    { "ssl.termination", "false" },
    { "stop_on_config_change", "false" },
    { "storage.filesystem[@allow]", "false" },
    { "storage.ssl.as_scheme", "true" },
    { "storage.ssl.ca_file_path", "" },
    { "storage.ssl.cert_file_path", "" },
    { "storage.ssl.cipher_list", "" },
    // { "storage.ssl.enable" - deliberately not set; for back-compat
    { "storage.ssl.key_file_path", "" },
    { "storage.wopi.alias_groups[@mode]", "first" },
    { "storage.wopi.is_legacy_server", "false" },
    { "storage.wopi.locking.refresh", "900" },
    { "storage.wopi.max_file_size", "0" },
    { "storage.wopi[@allow]", "true" },
    { "sys_template_path", "systemplate" },
    { "trace.filter.message", "" },
    { "trace.outgoing.record", "false" },
    { "trace.path", "" },
    { "trace.path[@compress]", "true" },
    { "trace.path[@snapshot]", "false" },
    { "trace[@enable]", "false" },
#if !MOBILEAPP
    { "trace_event.path", COOLWSD_TRACEEVENTFILE },
    { "trace_event[@enable]", "false" },
#endif
    { "user_interface.mode", "default" },
    { "user_interface.statusbar_save_indicator", "true" },
    { "user_interface.use_integration_theme", "true" },
    { "user_interface.brandProductName", "" },
    { "user_interface.brandProductURL", "" },
    { "user_interface.logoURL", "" },
    { "wasm.enable", "false" },
    { "wasm.force", "false" },
    { "watermark.opacity", "0.2" },
    { "watermark.text", "" },
    { "welcome.enable", "false" },
    { "zotero.enable", "true" },
};

void initialize(const Poco::Util::AbstractConfiguration* config)
{
    assert(config && "Cannot initialize with invalid config instance");
    assert(!Config && "Config is already initialized.");
    Config = config;

#if ENABLE_SSL
    SslEnabled.set(getBool("ssl.enable", true));
    SslTermination.set(getBool("ssl.termination", false));
#endif
}

void initialize(const std::string& xml)
{
    static Poco::AutoPtr<Poco::Util::XMLConfiguration> XmlConfig;
    assert(!XmlConfig && "Config is already initialized.");

    std::istringstream iss(xml);
    XmlConfig.reset(new Poco::Util::XMLConfiguration(iss));
    initialize(XmlConfig);
}

void initializeFromFile(const std::string& filename)
{
    std::ifstream ifs(filename);
    if (!ifs)
    {
        throw std::invalid_argument("The config xml file [" + filename +
                                    "] is invalid or not found");
    }

    std::ostringstream oss;
    oss << ifs.rdbuf();
    initialize(oss.str());
}

bool isInitialized() { return Config != nullptr; }

const std::unordered_map<std::string, std::string>& getDefaultAppConfig() { return DefAppConfig; }

/// Recursively extract the sub-keys of the given parent key.
void extract(const std::string& parentKey, const Poco::Util::AbstractConfiguration& config,
             std::map<std::string, std::string>& map)
{
    if (parentKey.empty() || !config.has(parentKey))
    {
        // For some unclear reason, we sometimes get
        // an empty key. This results in all entries
        // have a leading '.', which is unexpected.
        return;
    }

    std::vector<std::string> keys;
    config.keys(parentKey, keys);
    const std::string parentKeyDot = parentKey + '.';
    for (const std::string& subKey : keys)
    {
        const auto key = parentKeyDot + subKey;
        if (config.has(key))
        {
            map.emplace(key, getConfigValue(config, key, std::string()));
            extract(key, config, map);
        }
    }
}

std::map<std::string, std::string> extractAll(const Poco::Util::AbstractConfiguration& config)
{
    std::map<std::string, std::string> map;

    std::vector<std::string> keys;
    config.keys(keys);
    for (const std::string& key : keys)
    {
        extract(key, config, map);
    }

    // These keys have no values, but Poco gives us the values of
    // their children concatenated, which is worse than useless.
    // E.g. logging.file: /tmp/coolwsd.lognevertimestamptrue10 days10truefalse
    map.erase("admin_console.logging");
    map.erase("feature_lock.locked_hosts");
    map.erase("indirection_endpoint.geolocation_setup");
    map.erase("logging.anonymize");
    map.erase("logging.file");
    map.erase("logging_ui_cmd.file");
    map.erase("net.lok_allow");
    map.erase("net.post_allow");
    map.erase("per_document.cleanup");
    map.erase("ssl.hpkp");
    map.erase("ssl.hpkp.pins");
    map.erase("ssl.sts");
    map.erase("storage.filesystem");
    map.erase("storage.ssl");
    map.erase("storage.wopi");
    map.erase("storage.wopi.alias_groups");
    map.erase("storage.wopi.locking");
    map.erase("trace.filter");
    map.erase("trace.outgoing");

    return map;
}

std::string getLoggableConfig(const Poco::Util::AbstractConfiguration& config)
{
    const std::map<std::string, std::string> allConfigs = extractAll(config);
    std::ostringstream ossConfig;
    for (const auto& pair : allConfigs)
    {
        const auto it = DefAppConfig.find(pair.first);
        if (it == DefAppConfig.end() || it->second != pair.second)
        {
            if (pair.first == "admin_console.username" || pair.first == "admin_console.password" ||
                pair.first == "admin_console.secure_password" ||
                pair.first == "languagetool.api_key" || pair.first == "deepl.auth_key" ||
                pair.first == "logging.anonymize.anonymization_salt" || pair.first == "support_key")
            {
                // Redact sensitive entries.
                ossConfig << '\t' << pair.first << ": <redacted>\n";
            }
            else
            {
                ossConfig << '\t' << pair.first << ": " << pair.second << '\n';
            }
        }
    }

    return ossConfig.str();
}

std::string getString(const std::string& key, const std::string& def)
{
    assert(Config && "Config is not initialized.");
    return (Config != nullptr) ? Config->getString(key, def) : def;
}

bool getBool(const std::string& key, const bool def)
{
    if constexpr (Util::isFuzzing())
    {
        return def;
    }

    assert(Config && "Config is not initialized.");
    return (Config != nullptr) ? Config->getBool(key, def) : def;
}

int getInt(const std::string& key, const int def)
{
    assert(Config && "Config is not initialized.");
    return (Config != nullptr) ? Config->getInt(key, def) : def;
}

bool has(const std::string& key)
{
    assert(Config && "Config is not initialized.");
    return (Config != nullptr) ? Config->has(key) : false;
}

} // namespace ConfigUtil

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
