/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sal/config.h>

#include <tools/toolsdllapi.h>
#include <rtl/string.hxx>

#include <boost/property_tree/ptree.hpp>

#include <memory>
#include <optional>
#include <string_view>

namespace tools
{
/** JSON path navigation.
 *
 *  Wrapper around boost::property_tree::ptree, that allows XPath style
 *  search and navigation. Paths are "/" separated. Numeric segments index
 *  into arrays. Other segments look up child nodes by key. A leading "/"
 *  is optional. A numeric segment always indexes (it does not look up an
 *  object key that happens to be all digits).
 *
 *  Examples:
 *      "/primitives", "primitives", "/primitives/0/type"
 *
 *  A JsonPath always shares ownership of the underlying ptree storage
 *  via a shared_ptr, so the tree is guaranteed to outlive every JsonPath
 *  that points into it. The only public way to construct a root JsonPath
 *  is JsonPath::parse(). Sub-paths obtained from at() share ownership
 *  with their parent.
 *
 *  Missing paths and type mismatches are reported as std::nullopt or nullptr.
 */
class TOOLS_DLLPUBLIC JsonPath
{
public:
    /// Parse JSON text. The returned JsonPath owns its tree. Returns
    /// std::nullopt if the JSON is malformed.
    static std::optional<JsonPath> parse(std::string_view sJson) noexcept;

    JsonPath(const JsonPath&) = default;
    JsonPath(JsonPath&&) noexcept = default;
    JsonPath& operator=(const JsonPath&) = default;
    JsonPath& operator=(JsonPath&&) noexcept = default;

    /// Returns the underlying ptree.
    const boost::property_tree::ptree& tree() const noexcept { return *mpTree; }

    /// True if sPath resolves to a node under this wrapper.
    bool has(std::string_view sPath) const noexcept;

    /// Pointer to the sub-tree at sPath, or nullptr if not found.
    const boost::property_tree::ptree* find(std::string_view sPath) const noexcept;

    /// Sub-path wrapper at sPath, or std::nullopt if not found. The
    /// returned JsonPath shares ownership with this one.
    std::optional<JsonPath> at(std::string_view sPath) const noexcept;

    /// Wrap a node already known to be part of this tree as a new
    /// JsonPath sharing ownership with this. The caller must ensure
    /// rSubTree is part of the tree rooted at this->tree().
    JsonPath sub(const boost::property_tree::ptree& rSubTree) const noexcept;

    /// Value at sPath read as an OString. Empty path returns this node's
    /// value. Returns std::nullopt if the path is missing.
    std::optional<OString> getString(std::string_view sPath = std::string_view()) const noexcept;

    /// Value at sPath read as an integer. Returns std::nullopt if the
    /// path is missing or the value is not an integer.
    std::optional<sal_Int64> getInt(std::string_view sPath = std::string_view()) const noexcept;

    /// Value at sPath read as a double. Returns std::nullopt if the path
    /// is missing or the value is not a number.
    std::optional<double> getDouble(std::string_view sPath = std::string_view()) const noexcept;

    /// Value at sPath read as a bool. Returns std::nullopt if the path
    /// is missing or the value is not a bool.
    std::optional<bool> getBool(std::string_view sPath = std::string_view()) const noexcept;

    /// Number of children at sPath (array length or object key count).
    /// Returns std::nullopt if the path is missing.
    std::optional<size_t> getSize(std::string_view sPath = std::string_view()) const noexcept;

private:
    JsonPath(std::shared_ptr<const boost::property_tree::ptree> pOwned,
             const boost::property_tree::ptree& rTree) noexcept;

    /// Owns the root ptree storage. Always non-null. Sub-paths obtained
    /// from at() share this shared_ptr with their parent.
    std::shared_ptr<const boost::property_tree::ptree> mpOwned;

    /// Always non-null. Points into *mpOwned at the current node.
    const boost::property_tree::ptree* mpTree;
};

} // namespace tools

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
