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

// Configuration related utilities.
// Placed here to reduce avoid polluting
// Util.hpp with the config headers.
// This is designed to be used from both wsd and kit.

#pragma once

#include <common/Log.hpp>
#include <common/Util.hpp>

#include <Poco/Path.h>
#include <Poco/Util/AbstractConfiguration.h>
#include <Poco/Util/Application.h>
#include <Poco/Util/MapConfiguration.h>

#include <atomic>
#include <string>
#include <map>
#include <type_traits>
#include <unordered_map>

namespace ConfigUtil
{
/// Helper class to hold default configuration entries.
class AppConfigMap final : public Poco::Util::MapConfiguration
{
public:
    AppConfigMap() = default;
    AppConfigMap(const std::unordered_map<std::string, std::string>& map)
    {
        for (const auto& pair : map)
        {
            setRaw(pair.first, pair.second);
        }
    }

    void reset(const std::map<std::string, std::string>& map)
    {
        clear();
        for (const auto& pair : map)
        {
            setRaw(pair.first, pair.second);
        }
    }
};

/// A logical constant that is allowed to initialize
/// exactly once and checks usage before initialization.
template <typename T> class RuntimeConstant
{
    T _value;
    std::atomic<bool> _initialized;

public:
    RuntimeConstant()
        : _value()
        , _initialized(false)
    {
    }

    /// Use a compile-time const instead.
    RuntimeConstant(const T& value) = delete;

    const T& get()
    {
        assert(_initialized);

        // Don't incur a high cost in release builds.

        if constexpr (Util::isDebugEnabled())
        {
            if (_initialized)
            {
                return _value;
            }

            throw std::runtime_error("RuntimeConstant instance read before being initialized.");
        }
        else
        {
            return _value;
        }
    }

    void set(const T& value)
    {
        assert(!_initialized);

        _initialized = true;
        _value = value;
    }
};

#if ENABLE_SSL
extern RuntimeConstant<bool> SslEnabled;
extern RuntimeConstant<bool> SslTermination;
#endif

/// Initialize the config from an XML string.
void initialize(const std::string& xml);

/// Initialize the config given a pointer to a long-lived pointer.
void initialize(const Poco::Util::AbstractConfiguration* config);

/// Initialize the config from a file given it's name.
void initializeFromFile(const std::string& filename);

/// Check if the config has been initialized
bool isInitialized();

/// Returns the default config.
const std::unordered_map<std::string, std::string>& getDefaultAppConfig();

/// Extract all entries as key-value pairs. We use map to have the entries sorted.
std::map<std::string, std::string> extractAll(const Poco::Util::AbstractConfiguration& config);

/// Returns the config in a loggable string form.
std::string getLoggableConfig(const Poco::Util::AbstractConfiguration& config);

/// Returns the value of an entry as string or @def if it is not found.
std::string getString(const std::string& key, const std::string& def);

/// Returns true if and only if the property with the given key exists.
bool has(const std::string& key);

/// Returns the value of an entry as string or @def if it is not found.
bool getBool(const std::string& key, bool def);

/// Returns the value of an entry as int or @def if it is not found.
int getInt(const std::string& key, int def);

/// Return true if SSL is enabled in the config and no fuzzing is enabled.
inline bool isSslEnabled()
{
#if defined(ENABLE_SSL) && ENABLE_SSL
    if constexpr (!Util::isFuzzing())
    {
#if ENABLE_DEBUG
        // Unit-tests enable/disable SSL at will.
        CONFIG_STATIC const bool isSslEnabled = getBool("ssl.enable", true);
        return isSslEnabled;
#else
        return SslEnabled.get();
#endif
    }
#endif

    return false;
}

/// Returns true if SSL Termination is enabled in the config and no fuzzing is enabled.
inline bool isSSLTermination()
{
#if defined(ENABLE_SSL) && ENABLE_SSL
    return !Util::isFuzzing() && SslTermination.get();
#else
    return false;
#endif
}

/// Return true if build is support key enabled (ENABLE_SUPPORT_KEY is defined)
constexpr bool isSupportKeyEnabled()
{
#ifdef ENABLE_SUPPORT_KEY
    return ENABLE_SUPPORT_KEY;
#else
    return false;
#endif
}

class ConfigRawValueGetter
{
    const Poco::Util::AbstractConfiguration* _config;

    ConfigRawValueGetter(const ConfigRawValueGetter&) = default;
    ConfigRawValueGetter(ConfigRawValueGetter&&) = default;
    ConfigRawValueGetter& operator=(const ConfigRawValueGetter&) = default;
    ConfigRawValueGetter& operator=(ConfigRawValueGetter&&) = default;

public:
    explicit ConfigRawValueGetter(const Poco::Util::AbstractConfiguration& config)
        : _config(&config)
    {
    }

    ~ConfigRawValueGetter() = default;

    void operator()(const std::string& name, int& value) const { value = _config->getInt(name); }
    void operator()(const std::string& name, unsigned int& value) const
    {
        value = _config->getUInt(name);
    }
    void operator()(const std::string& name, long int& value) const
    {
        value = _config->getInt64(name);
    }
    void operator()(const std::string& name, unsigned long int& value) const
    {
        value = _config->getUInt64(name);
    }
    void operator()(const std::string& name, long long& value) const
    {
        value = _config->getInt64(name);
    }
    void operator()(const std::string& name, unsigned long long& value) const
    {
        value = _config->getUInt64(name);
    }
    void operator()(const std::string& name, bool& value) const { value = _config->getBool(name); }
    void operator()(const std::string& name, std::string& value) const
    {
        value = _config->getString(name);
    }
    void operator()(const std::string& name, double& value) const
    {
        value = _config->getDouble(name);
    }
};

template <typename T>
static bool getRawConfig(const Poco::Util::AbstractConfiguration& config, const std::string& name,
                         T& value)
{
    try
    {
        ConfigRawValueGetter{ config }(name, value);
        return true;
    }
    catch (...)
    {
    }

    return false;
}

template <typename T>
static T getConfigValue(const Poco::Util::AbstractConfiguration& config, const std::string& name,
                        T def)
{
    T value = def;
    if (getRawConfig(config, name, value) || getRawConfig(config, name + "[@default]", value))
    {
        return value;
    }

    return def;
}

/// Reads and processes path entries with the given property
/// from the configuration.
/// Converts relative paths to absolute.
static std::string getPathFromConfig(const Poco::Util::AbstractConfiguration& config,
                                     const std::string& property)
{
    std::string path = config.getString(property);
    if (path.empty() && config.hasProperty(property + "[@default]"))
    {
        // Use the default value if empty and a default provided.
        path = config.getString(property + "[@default]");
    }

    // Reconstruct absolute path if relative.
    if (!Poco::Path(path).isAbsolute() && config.hasProperty(property + "[@relative]") &&
        config.getBool(property + "[@relative]"))
    {
        path = Poco::Path(Poco::Util::Application::instance().commandPath())
                   .parent()
                   .append(path)
                   .toString();
    }

    return path;
}

/// Returns the value for integral, floating-point, or string type
/// of the specified application configuration,
/// or the default, if one doesn't exist.
template <typename T>
static T getConfigValue(const std::string& name, T def)
    requires(std::is_integral_v<T> || std::is_floating_point_v<T> || std::is_same_v<T, std::string>)
{
    if constexpr (Util::isFuzzing())
    {
        return def;
    }

    return getConfigValue(Poco::Util::Application::instance().config(), name, std::move(def));
}

/// Returns the chrono value of the specified application configuration,
/// or the default, if one doesn't exist.
/// By default, we expect non-negative values and will log warning otherwise.
template <typename T>
T getConfigValue(const Poco::Util::AbstractConfiguration& config, const std::string& name,
                 const typename T::rep def, const typename T::rep min = 0)
{
    if constexpr (Util::isFuzzing())
    {
        return T(def);
    }

    auto value = def;
    if (getRawConfig(config, name, value) || getRawConfig(config, name + "[@default]", value))
    {
        if (value < min)
        {
            LOG_WRN("Config value for " << name << " [" << value
                                        << "] is below the required minimum [" << min
                                        << "] and will be overridden by the minimum");
            value = min;
        }

        return T(value);
    }

    return T(def);
}

/// Returns the chrono value of the specified application configuration,
/// or the default, if one doesn't exist.
/// By default, we expect non-negative values.
template <typename T>
inline T getConfigValue(const std::string& name, const typename T::rep def,
                        const typename T::rep min = 0)
{
    if constexpr (Util::isFuzzing())
    {
        return T(def);
    }

    return getConfigValue<T>(Poco::Util::Application::instance().config(), name, def, min);
}

/// Returns the chrono value of the specified application configuration,
/// or the default, if one doesn't exist.
/// By default, we expect non-negative values.
template <typename T>
inline T getConfigValue(const std::string& name, const T def, const T min = T(0))
    requires std::is_integral_v<typename T::rep>
{
    return getConfigValue<T>(name, def.count(), min.count());
}

/// Returns the value of the specified application configuration,
/// or the default, if one doesn't exist.
template <typename T> static T getConfigValueNonZero(const std::string& name, T def)
{
    static_assert(std::is_integral_v<T>, "Meaningless on non-integral types");

    if constexpr (Util::isFuzzing())
    {
        return def;
    }

    const T res = getConfigValue(Poco::Util::Application::instance().config(), name, def);
    return res <= T(0) ? T(0) : res;
}

/// Reads and processes path entries with the given property
/// from the configuration.
/// Converts relative paths to absolute.
inline std::string getPathFromConfig(const std::string& name)
{
    return getPathFromConfig(Poco::Util::Application::instance().config(), name);
}

/// Reads and processes path entries with the given property
/// from the configuration. If value is empty then it reads from fallback
/// Converts relative paths to absolute.
inline std::string getPathFromConfigWithFallback(const std::string& name,
                                                 const std::string& fallbackName)
{
    std::string value;
    // the expected path might not exist, in which case Poco throws an exception
    try
    {
        value = getPathFromConfig(name);
    }
    catch (...)
    {
    }

    if (!value.empty())
        return value;

    return getPathFromConfig(fallbackName);
}

/// Returns true if and only if the property with the given key exists.
inline bool hasProperty(const std::string& key)
{
    return Poco::Util::Application::instance().config().hasProperty(key);
}

} // namespace ConfigUtil

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
