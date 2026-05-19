/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <tools/JsonPath.hxx>

#include <boost/property_tree/json_parser.hpp>

#include <algorithm>
#include <sstream>
#include <string>

namespace tools
{
namespace
{
/// Tokenizer for JsonPath "/" separated paths. Yields one non-empty
/// segment at a time, classifying each as either a positional index
/// (all decimal digits) or an object key.
class PathTokens
{
public:
    enum class Kind
    {
        Index,
        Key,
    };

    struct Token
    {
        std::string sValue;
        Kind eKind;
    };

    explicit PathTokens(std::string_view sPath) noexcept
        : msPath(sPath)
        , mnPosition(0)
    {
    }

    /// Next non-empty segment, or std::nullopt when the path is
    /// exhausted. Empty segments (leading "/", "//", trailing "/") are
    /// silently skipped.
    std::optional<Token> next() noexcept
    {
        while (mnPosition < msPath.size())
        {
            size_t nEnd = msPath.find('/', mnPosition);
            if (nEnd == std::string_view::npos)
                nEnd = msPath.size();
            std::string sSegment(msPath.substr(mnPosition, nEnd - mnPosition));
            mnPosition = nEnd + 1;
            if (sSegment.empty())
                continue;
            // Classify before moving: argument evaluation order is
            // unspecified, so combining both into one initializer
            // could read from a moved-from string.
            const Kind eKind = classify(sSegment);
            return Token{ std::move(sSegment), eKind };
        }
        return std::nullopt;
    }

private:
    static Kind classify(const std::string& sSegment) noexcept
    {
        const bool bAllDigits = std::all_of(sSegment.begin(), sSegment.end(), [](char character) {
            return character >= '0' && character <= '9';
        });
        return bAllDigits ? Kind::Index : Kind::Key;
    }

    std::string_view msPath;
    size_t mnPosition;
};

/// Descend by positional index into the current node's children. Returns
/// nullptr if the segment does not parse or the index is out of range.
const boost::property_tree::ptree* findChildByIndex(const boost::property_tree::ptree& rNode,
                                                    const std::string& sSegment) noexcept
{
    size_t nIndex = 0;
    try
    {
        nIndex = std::stoul(sSegment);
    }
    catch (const std::exception&)
    {
        return nullptr;
    }
    auto iterator = rNode.begin();
    for (size_t index = 0; iterator != rNode.end() && index < nIndex; ++iterator, ++index)
    {
    }
    if (iterator == rNode.end())
        return nullptr;
    return &iterator->second;
}

/// Descend by key into the current node's children. Returns nullptr if
/// the key is absent.
const boost::property_tree::ptree* findChildByKey(const boost::property_tree::ptree& rNode,
                                                  const std::string& sKey) noexcept
{
    auto iterator = rNode.find(sKey);
    if (iterator == rNode.not_found())
        return nullptr;
    return &iterator->second;
}

const boost::property_tree::ptree* walkPath(const boost::property_tree::ptree& rRoot,
                                            std::string_view sPath) noexcept
{
    const boost::property_tree::ptree* pNode = &rRoot;
    PathTokens aTokens(sPath);
    while (auto oToken = aTokens.next())
    {
        pNode = oToken->eKind == PathTokens::Kind::Index ? findChildByIndex(*pNode, oToken->sValue)
                                                         : findChildByKey(*pNode, oToken->sValue);
        if (!pNode)
            return nullptr;
    }
    return pNode;
}

template <typename T> std::optional<T> readValue(const boost::property_tree::ptree* pNode) noexcept
{
    if (!pNode)
        return std::nullopt;
    try
    {
        return pNode->get_value<T>();
    }
    catch (const std::exception&)
    {
        return std::nullopt;
    }
}

} // end anonymous namespace

JsonPath::JsonPath(std::shared_ptr<const boost::property_tree::ptree> pOwned,
                   const boost::property_tree::ptree& rTree) noexcept
    : mpOwned(std::move(pOwned))
    , mpTree(&rTree)
{
}

std::optional<JsonPath> JsonPath::parse(std::string_view sJson) noexcept
{
    auto pTree = std::make_shared<boost::property_tree::ptree>();
    try
    {
        std::stringstream aStream{ std::string(sJson) };
        boost::property_tree::read_json(aStream, *pTree);
    }
    catch (const std::exception&)
    {
        return std::nullopt;
    }
    const auto& rTree = *pTree;
    return JsonPath(std::shared_ptr<const boost::property_tree::ptree>(std::move(pTree)), rTree);
}

bool JsonPath::has(std::string_view sPath) const noexcept
{
    return walkPath(*mpTree, sPath) != nullptr;
}

const boost::property_tree::ptree* JsonPath::find(std::string_view sPath) const noexcept
{
    return walkPath(*mpTree, sPath);
}

std::optional<JsonPath> JsonPath::at(std::string_view sPath) const noexcept
{
    const boost::property_tree::ptree* pNode = walkPath(*mpTree, sPath);
    if (!pNode)
        return std::nullopt;
    return JsonPath(mpOwned, *pNode);
}

JsonPath JsonPath::sub(const boost::property_tree::ptree& rSubTree) const noexcept
{
    return JsonPath(mpOwned, rSubTree);
}

std::optional<OString> JsonPath::getString(std::string_view sPath) const noexcept
{
    auto oValue = readValue<std::string>(walkPath(*mpTree, sPath));
    if (!oValue)
        return std::nullopt;
    return OString(oValue->data(), oValue->size());
}

std::optional<sal_Int64> JsonPath::getInt(std::string_view sPath) const noexcept
{
    return readValue<sal_Int64>(walkPath(*mpTree, sPath));
}

std::optional<double> JsonPath::getDouble(std::string_view sPath) const noexcept
{
    return readValue<double>(walkPath(*mpTree, sPath));
}

std::optional<bool> JsonPath::getBool(std::string_view sPath) const noexcept
{
    return readValue<bool>(walkPath(*mpTree, sPath));
}

std::optional<size_t> JsonPath::getSize(std::string_view sPath) const noexcept
{
    const boost::property_tree::ptree* pNode = walkPath(*mpTree, sPath);
    if (!pNode)
        return std::nullopt;
    return pNode->size();
}

} // namespace tools

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
