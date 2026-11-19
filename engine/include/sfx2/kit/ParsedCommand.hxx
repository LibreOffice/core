/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sfx2/dllapi.h>

#include <rtl/ustring.hxx>

#include <map>
#include <optional>
#include <string_view>

/**
 * One command, split into its name and its query parameters.
 *
 * Make sure that it does not outlive the original command string since
 * KitParsedCommand references it directly without making copies.
 */
struct SFX2_DLLPUBLIC KitParsedCommand
{
    /** The name, without the ".uno:" prefix and the query. */
    std::string_view aName;
    /** The query parameters, still URL-encoded. */
    std::map<std::string_view, std::string_view> aParams;

    /**
     * Parse a command of the form ".uno:Name?key1=value1&key2=value2".
     * Without the ".uno:" prefix.
     */
    static KitParsedCommand parse(std::string_view aCommand);

    /**
     * Get the raw value of the parameter, if present.
     */
    std::optional<std::string_view> get(std::string_view aKey) const;

    /**
     * Get the URL-decoded value of the parameter, or an empty string if not present.
     */
    OUString getDecoded(std::string_view aKey) const;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
