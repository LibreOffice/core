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

#include <config.h>

#include <wsd/McpDeck.hpp>

#include <common/FileUtil.hpp>
#include <common/JsonUtil.hpp>
#include <common/Log.hpp>
#include <common/Util.hpp>
#include <wsd/COOLWSD.hpp>

#include <Poco/DOM/DOMParser.h>
#include <Poco/DOM/Document.h>
#include <Poco/DOM/Element.h>
#include <Poco/DOM/NodeList.h>
#include <Poco/DirectoryIterator.h>
#include <Poco/JSON/Object.h>
#include <Poco/Path.h>
#include <Poco/StreamCopier.h>
#include <Poco/Zip/ZipArchive.h>
#include <Poco/Zip/ZipStream.h>

#include <fstream>
#include <map>
#include <mutex>
#include <optional>
#include <set>
#include <sstream>

namespace
{

/// The metadata part of an ODF package, and the user-defined property a design template declares
/// its manifest in.
constexpr const char* MetadataEntry = "meta.xml";
constexpr const char* ManifestPropertyName = "AIDesignManifest";

/// The lowest manifest schema version this reads. A manifest that declares an older version, or
/// none at all, is treated as absent, which is what the document engine does with it too.
constexpr int LowestManifestSchemaVersion = 1;

/// The directories holding design templates, relative to the document engine. The bundled set comes
/// first and the per-config preset directory second, the same order and the same two places the
/// engine looks in, so both offer the same designs under the same names.
std::vector<std::string> designTemplateDirectories()
{
    const std::string engine = COOLWSD::LoTemplate;
    return { engine + "/share/cool-ai-templates", engine + "/share/template/common/presnt" };
}

/// The text a meta:user-defined element carries, when it is the one naming the design manifest. An
/// element with any other name gives an empty string.
std::string manifestTextOfProperty(const Poco::XML::Element* property)
{
    if (!property || property->getAttribute("meta:name") != ManifestPropertyName)
        return std::string();
    return property->innerText();
}

/// The manifest of the template at the given path. A template is read once, and again only after
/// its file has changed, so a listing reads the template files only the first time.
std::string cachedDesignManifest(const std::string& templatePath)
{
    struct CachedManifest
    {
        std::size_t modifiedTimeMs;
        std::size_t size;
        std::string manifest;
    };
    static std::mutex mutex;
    static std::map<std::string, CachedManifest> cache;

    const FileUtil::Stat stat(templatePath);
    std::lock_guard<std::mutex> lock(mutex);
    const auto it = cache.find(templatePath);
    if (it != cache.end() && it->second.modifiedTimeMs == stat.modifiedTimeMs() &&
        it->second.size == stat.size())
        return it->second.manifest;

    std::string manifest = McpDeck::readDesignManifest(templatePath);
    cache[templatePath] = CachedManifest{ stat.modifiedTimeMs(), stat.size(), manifest };
    return manifest;
}

} // anonymous namespace

namespace McpDeck
{

std::string readDesignManifest(const std::string& templatePath)
{
    try
    {
        std::ifstream package(templatePath, std::ios::binary);
        if (!package.good())
            return std::string();

        const Poco::Zip::ZipArchive archive(package);
        const Poco::Zip::ZipArchive::FileHeaders::const_iterator entry =
            archive.findHeader(MetadataEntry);
        if (entry == archive.headerEnd())
            return std::string();

        Poco::Zip::ZipInputStream metadataStream(package, entry->second);
        std::ostringstream metadata;
        Poco::StreamCopier::copyStream(metadataStream, metadata);

        Poco::XML::DOMParser parser;
        Poco::AutoPtr<Poco::XML::Document> document = parser.parseString(metadata.str());
        Poco::AutoPtr<Poco::XML::NodeList> properties =
            document->getElementsByTagName("meta:user-defined");
        for (unsigned long i = 0; i < properties->length(); ++i)
        {
            const std::string manifest = manifestTextOfProperty(
                dynamic_cast<Poco::XML::Element*>(properties->item(i)));
            if (!manifest.empty())
                return manifest;
        }
    }
    catch (const std::exception& exc)
    {
        LOG_WRN("MCP: design template [" << templatePath
                                         << "] has no readable metadata: " << exc.what());
    }
    return std::string();
}

DesignTemplate designTemplateFromManifest(const std::string& name,
                                          const std::string& manifestJson)
{
    DesignTemplate design;
    design.name = name;
    design.budgets = DeckSpec::budgetsFromConfig();

    Poco::JSON::Object::Ptr manifest = new Poco::JSON::Object();
    if (manifestJson.empty() || !JsonUtil::parseJSON(manifestJson, manifest))
        return design;

    int schemaVersion = 0;
    JsonUtil::findJSONValue(manifest, "schemaVersion", schemaVersion);
    if (schemaVersion < LowestManifestSchemaVersion)
        return design;

    // The parts the template has a design for, read from the master-to-part map and reported in the
    // deck's own part order, whatever order the manifest lists its masters in.
    if (Poco::JSON::Object::Ptr masters = manifest->getObject("masters"))
    {
        std::set<std::string> covered;
        for (const std::string& master : masters->getNames())
        {
            const std::string part = masters->optValue<std::string>(master, std::string());
            if (DeckSpec::isKnownPart(part))
                covered.insert(part);
        }
        for (const char* part : { "opening", "divider", "body", "closing" })
        {
            if (covered.count(part))
                design.parts.emplace_back(part);
        }
    }

    // The image style comes from a file an integrator may edit, and it only ever composes an image
    // prompt, so it is sanitized here at the trust boundary.
    design.artDirection =
        DeckSpec::sanitizeArtDirection(manifest->optValue<std::string>("artDirection", std::string()));

    // A manifest limit applies only when it is lower than the configured one, so a template can ask
    // for a smaller deck than the server allows but never a larger one.
    if (Poco::JSON::Object::Ptr budgets = manifest->getObject("budgets"))
    {
        const auto tighten = [&budgets](const std::string& key, int& limit)
        {
            int declared = 0;
            if (JsonUtil::findJSONValue(budgets, key, declared) && declared >= 1)
                limit = std::min(limit, declared);
        };
        tighten("maxSlides", design.budgets.maxSlides);
        tighten("maxItemsPerBullets", design.budgets.maxItemsPerBullets);
        tighten("maxItemLength", design.budgets.maxItemLength);
        tighten("maxTitleLength", design.budgets.maxTitleLength);
    }

    return design;
}

std::vector<DesignTemplate> installedDesignTemplates()
{
    std::vector<DesignTemplate> designs;
    std::set<std::string> seen;

    for (const std::string& directory : designTemplateDirectories())
    {
        try
        {
            const Poco::DirectoryIterator end;
            for (Poco::DirectoryIterator it(directory); it != end; ++it)
            {
                const Poco::Path path(it->path());
                if (Util::toLower(path.getExtension()) != "otp")
                    continue;

                const std::string name = path.getBaseName();
                if (!DeckSpec::isSafeDesignName(name))
                {
                    LOG_WRN("MCP: design template [" << name << "] is skipped, its name must be at "
                                                        "most 64 characters of letters, digits, "
                                                        "space, hyphen, or underscore");
                    continue;
                }
                // The first template seen under a name wins, so a bundled design keeps its name
                // when a preset of the same name is installed too.
                if (!seen.insert(Util::toLower(name)).second)
                    continue;

                designs.push_back(
                    designTemplateFromManifest(name, cachedDesignManifest(it->path())));
            }
        }
        catch (const std::exception&)
        {
            // A directory that is not there simply offers no designs.
        }
    }

    return designs;
}

} // namespace McpDeck

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
