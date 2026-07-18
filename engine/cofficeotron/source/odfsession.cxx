/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * cofficeotron - a C++ port of the Java(tm) Office-o-tron
 *
 * Copyright (c) 2009-2010 Griffin Brown Digital Publishing Ltd.
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "odfsession.hxx"

#include <set>
#include <vector>

#include "xmlutil.hxx"

namespace
{
const char* const ODF_MANIFEST_NS = "urn:oasis:names:tc:opendocument:xmlns:manifest:1.0";
const char* const ODF_OFFICE_NS = "urn:oasis:names:tc:opendocument:xmlns:office:1.0";
const char* const ODF_META_NS = "urn:oasis:names:tc:opendocument:xmlns:meta:1.0";
const char* const ODF_TEXT_NS = "urn:oasis:names:tc:opendocument:xmlns:text:1.0";
const char* const ODF_DRAW_NS = "urn:oasis:names:tc:opendocument:xmlns:drawing:1.0";
const char* const ODF_FORM_NS = "urn:oasis:names:tc:opendocument:xmlns:form:1.0";
const char* const ODF_PRESENTATION_NS = "urn:oasis:names:tc:opendocument:xmlns:presentation:1.0";
const char* const ODF_SMIL_NS = "urn:oasis:names:tc:opendocument:xmlns:smil-compatible:1.0";

struct OdfSchemaPaths
{
    const char* version;
    const char* documentSchema;
    const char* manifestSchema;
};

const OdfSchemaPaths odfSchemas[] = {
    { "1.0", "odf/odf1.0/OpenDocument-schema-v1.0-os.rnc",
      "odf/odf1.0/OpenDocument-manifest-schema-v1.0-os.rnc" },
    { "1.1", "odf/odf1.1/OpenDocument-schema-v1.1.rnc",
      "odf/odf1.1/OpenDocument-manifest-schema-v1.1.rnc" },
    { "1.2", "odf/odf1.2/OpenDocument-v1.2-os-schema.rnc",
      "odf/odf1.2/OpenDocument-v1.2-os-manifest-schema.rnc" },
    { "1.3", "odf/odf1.3/OpenDocument-v1.3-schema.rnc",
      "odf/odf1.3/OpenDocument-v1.3-manifest-schema.rnc" },
    { "1.4", "odf/odf1.4/OpenDocument-v1.4-schema.rnc",
      "odf/odf1.4/OpenDocument-v1.4-manifest-schema.rnc" },
};

const OdfSchemaPaths* schemasForVersion(const std::string& version)
{
    for (const OdfSchemaPaths& entry : odfSchemas)
        if (version == entry.version)
            return &entry;
    return nullptr;
}

struct QualifiedNameEntry
{
    const char* nsHref;
    const char* localName;
};

// The ID type attributes of ODF 1.1.
const QualifiedNameEntry idTypes[] = {
    { ODF_TEXT_NS, "id" },
    { ODF_DRAW_NS, "id" },
    { ODF_FORM_NS, "id" },
};

// The IDREF and IDREFS type attributes.
const QualifiedNameEntry idRefTypes[] = {
    { ODF_TEXT_NS, "change-id" },      { ODF_DRAW_NS, "nav-order" },
    { ODF_DRAW_NS, "start-shape" },    { ODF_DRAW_NS, "end-shape" },
    { ODF_DRAW_NS, "control" },        { ODF_DRAW_NS, "caption-id" },
    { ODF_DRAW_NS, "shape-id" },       { ODF_PRESENTATION_NS, "master-element" },
    { ODF_SMIL_NS, "target-element" },
};

// Elements on which text:id is not of type ID.
const QualifiedNameEntry textIdOddBalls[] = {
    { ODF_TEXT_NS, "alphabetical-index-mark-end" },
    { ODF_TEXT_NS, "alphabetical-index-mark-start" },
    { ODF_TEXT_NS, "note" },
    { ODF_TEXT_NS, "toc-mark-end" },
    { ODF_TEXT_NS, "toc-mark-start" },
    { ODF_TEXT_NS, "user-index-mark-end" },
    { ODF_TEXT_NS, "user-index-mark-start" },
};

template <size_t N>
bool setContains(const QualifiedNameEntry (&entries)[N], const std::string& nsHref,
                 const std::string& localName)
{
    for (const QualifiedNameEntry& entry : entries)
        if (nsHref == entry.nsHref && localName == entry.localName)
            return true;
    return false;
}

void splitWhitespace(const std::string& value, std::vector<std::string>& tokens)
{
    size_t position = 0;
    while (position < value.size())
    {
        while (position < value.size() && std::isspace(static_cast<unsigned char>(value[position])))
            ++position;
        size_t start = position;
        while (position < value.size()
               && !std::isspace(static_cast<unsigned char>(value[position])))
            ++position;
        if (position > start)
            tokens.push_back(value.substr(start, position - start));
    }
}

// Reports duplicate ID values while walking, and references to absent
// IDs at the end of the walk.
class IdHarvester
{
public:
    IdHarvester(ValidationReport& commentary)
        : commentary(commentary)
    {
    }

    void walk(const XmlNode* element)
    {
        std::string elementNs = namespaceOf(element);
        std::string elementLocal = localNameOf(element);

        for (const XmlAttribute& attribute : element->attributes)
        {
            const std::string& attributeNs = attribute.nsHref;
            const std::string& attributeLocal = attribute.localName;
            const std::string& value = attribute.value;

            if (setContains(idTypes, attributeNs, attributeLocal)
                && !setContains(textIdOddBalls, elementNs, elementLocal))
            {
                if (!idValues.insert(value).second)
                {
                    ++duplicateCount;
                    if (duplicateCount <= ErrorCapper::threshold)
                    {
                        commentary.addComment("ERROR",
                                              "Duplicate ID value found: \"" + value + "\"");
                    }
                }
            }
            else if (setContains(idRefTypes, attributeNs, attributeLocal))
            {
                std::vector<std::string> tokens;
                splitWhitespace(value, tokens);
                for (const std::string& token : tokens)
                {
                    if (!idValues.count(token))
                        pendingReferences.push_back(token);
                }
            }
        }

        for (const auto& child : element->children)
            if (child->isElement())
                walk(child.get());
    }

    void reportFindings()
    {
        if (duplicateCount > ErrorCapper::threshold)
        {
            commentary.addComment(
                "WARN", "<i>" + std::to_string(duplicateCount - ErrorCapper::threshold)
                            + " duplicate ID message(s) omitted for the sake of brevity</i>");
        }

        int absentCount = 0;
        for (const std::string& reference : pendingReferences)
        {
            if (!idValues.count(reference))
            {
                ++absentCount;
                if (absentCount <= ErrorCapper::threshold)
                {
                    commentary.addComment("WARNING",
                                          "Reference to absent ID \"" + reference + "\"");
                }
            }
        }
        if (absentCount > ErrorCapper::threshold)
        {
            commentary.addComment(
                "WARN", "<i>" + std::to_string(absentCount - ErrorCapper::threshold)
                            + " absent ID message(s) omitted for the sake of brevity</i>");
        }
    }

private:
    ValidationReport& commentary;
    std::set<std::string> idValues;
    std::vector<std::string> pendingReferences;
    int duplicateCount = 0;
};

void harvestGenerator(const XmlNode* element, std::string& generator)
{
    if (namespaceOf(element) == ODF_META_NS && localNameOf(element) == "generator")
        generator += directTextContent(element);

    for (const auto& child : element->children)
        if (child->isElement())
            harvestGenerator(child.get(), generator);
}

std::string trimmed(const std::string& text)
{
    size_t begin = 0;
    size_t end = text.size();
    while (begin < end && std::isspace(static_cast<unsigned char>(text[begin])))
        ++begin;
    while (end > begin && std::isspace(static_cast<unsigned char>(text[end - 1])))
        --end;
    return text.substr(begin, end - begin);
}

std::string basenameOf(const std::string& path)
{
    size_t slash = path.rfind('/');
    return slash == std::string::npos ? path : path.substr(slash + 1);
}

void validateOdfDoc(const XmlNode* document, const std::string& entryName,
                    const std::string& version, ValidationReport& commentary, SchemaCache& schemas)
{
    const OdfSchemaPaths* paths = schemasForVersion(version);
    if (!paths)
    {
        commentary.addComment("FATAL", "No schema is available for ODF version " + version);
        return;
    }

    std::string schemaError;
    int schema = schemas.getSchema(paths->documentSchema, schemaError);
    if (!schema)
    {
        commentary.addComment("FATAL", schemaError);
        return;
    }

    ErrorCapper capper(commentary, basenameOf(entryName));

    commentary.incIndent();
    bool valid = validateElement(document->rootElement(), schema, capper);
    capper.addOmissionNote();
    commentary.decIndent();

    if (valid)
        commentary.addComment("The document is valid");
    else
        commentary.addComment("ERROR", "The document is invalid");
}

void validateManifest(const XmlNode* manifestDoc, const std::string& version,
                      ValidationReport& commentary, SchemaCache& schemas)
{
    commentary.addComment("Validating manifest");
    commentary.incIndent();

    const OdfSchemaPaths* paths = schemasForVersion(version);
    std::string schemaError;
    int schema = paths ? schemas.getSchema(paths->manifestSchema, schemaError) : 0;
    if (!schema)
    {
        commentary.addComment("FATAL", schemaError.empty() ? "No manifest schema is available"
                                                           : schemaError);
        commentary.decIndent();
        return;
    }

    ErrorCapper capper(commentary, "META-INF/manifest.xml");
    if (validateElement(manifestDoc->rootElement(), schema, capper))
        commentary.addComment("Manifest is valid");
    else
        commentary.addComment("ERROR", "Manifest is invalid");

    commentary.decIndent();
}

} // namespace

void runOdfValidation(const ZipArchive& zip, ValidationReport& report, SchemaCache& schemas,
                      const OdfSessionOptions& options)
{
    std::string manifestBytes;
    if (!zip.extract("META-INF/manifest.xml", manifestBytes))
    {
        report.addComment("ERROR", "The manifest cannot be extracted or is corrupt");
        return;
    }

    std::string parseError;
    XmlDocPtr manifestDoc = parseXmlMemory(manifestBytes, "META-INF/manifest.xml", parseError);
    if (!manifestDoc)
    {
        report.addComment("ERROR", "The manifest cannot be parsed");
        return;
    }

    std::vector<std::string> itemRefs;
    std::vector<std::string> itemTypes;
    XmlNode* manifestRoot = manifestDoc->rootElement();
    if (manifestRoot)
    {
        for (const auto& node : manifestRoot->children)
        {
            if (!node->isElement() || namespaceOf(node.get()) != ODF_MANIFEST_NS
                || localNameOf(node.get()) != "file-entry")
                continue;

            std::string fullPath;
            std::string mediaType;
            getNsAttribute(node.get(), ODF_MANIFEST_NS, "full-path", fullPath);
            getNsAttribute(node.get(), ODF_MANIFEST_NS, "media-type", mediaType);
            itemRefs.push_back(fullPath);
            itemTypes.push_back(mediaType);
        }
    }

    // The highest office:version seen on a validated document; 1.2 and
    // later also get their manifest validated.
    std::string manifestVersion;

    for (size_t i = 0; i < itemRefs.size(); ++i)
    {
        const std::string& entry = itemRefs[i];
        const std::string& mimeType = itemTypes[i];

        if (!entry.empty() && entry.back() == '/')
            continue; // folders cannot be validated

        if (mimeType.empty())
        {
            report.addComment("WARN", "Manifest entry for \"" + entry
                                          + "\" should have a MIME type, but has an empty string");
        }

        if (mimeType.find("xml") == std::string::npos)
        {
            // Not declared as XML, but anything with an .xml extension is
            // still worth trying.
            std::string trimmedEntry = trimmed(entry);
            if (trimmedEntry.size() < 4
                || trimmedEntry.compare(trimmedEntry.size() - 4, 4, ".xml") != 0)
                continue;
        }

        report.addComment("Processing manifest entry: " + entry);
        report.incIndent();

        std::string entryBytes;
        XmlDocPtr entryDoc;
        if (zip.extract(entry, entryBytes))
            entryDoc = parseXmlMemory(entryBytes, entry, parseError);

        if (!entryDoc)
        {
            report.addComment("WARN", "Referenced resource in manifest cannot be found/processed");
            report.decIndent();
            continue;
        }

        XmlNode* root = entryDoc->rootElement();
        if (!root)
        {
            report.addComment("WARN", "Referenced resource in manifest cannot be found/processed");
            report.decIndent();
            continue;
        }

        if (options.checkIds)
        {
            IdHarvester harvester(report);
            harvester.walk(root);
            harvester.reportFindings();
        }

        std::string generator;
        harvestGenerator(root, generator);

        if (namespaceOf(root) == ODF_OFFICE_NS)
        {
            std::string version;
            bool hasVersion = getNsAttribute(root, ODF_OFFICE_NS, "version", version);

            report.addComment("It has root element named &lt;" + localNameOf(root)
                              + "> in the namespace <tt>" + namespaceOf(root) + "</tt>");

            if (options.forceIs)
            {
                report.addComment("WARN", "Forcing validation against ISO/IEC 26300");
                version = "1.0";
            }
            else if (hasVersion)
            {
                report.addComment("It claims to be ODF version " + version);
            }
            else
            {
                report.addComment("WARN", "It has no version attribute! (assuming ODF v1.1)");
                version = "1.1";
            }

            validateOdfDoc(entryDoc.get(), entry, version, report, schemas);

            if (version >= "1.2" && version <= "1.4" && version > manifestVersion)
                manifestVersion = version;
        }

        if (!generator.empty())
        {
            report.addComment("The generator value is: \"<b>" + trimmed(generator) + "</b>\"");
        }

        report.decIndent();
    }

    if (!manifestVersion.empty())
        validateManifest(manifestDoc.get(), manifestVersion, report, schemas);

    report.addComment("Grand total count of validity errors: "
                      + std::to_string(report.getErrCount()));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
