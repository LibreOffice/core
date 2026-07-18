/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * cofficeotron - a C++ port of the Java(tm) Office-o-tron
 *
 * Copyright (c) 2009 Griffin Brown Digital Publishing Ltd.
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "ooxmlsession.hxx"

#include "mcefilter.hxx"
#include "opcpackage.hxx"
#include "schemamap.hxx"
#include "xmlutil.hxx"

namespace
{

// Returns the number of validity errors found in the part.
int validateTarget(const ZipArchive& zip, const OOXMLTarget& target,
                   const OOXMLSchemaMapping& mapping, ValidationReport& report,
                   SchemaCache& schemas)
{
    std::string schemaName = mapping.schemaName;
    std::string partName = target.targetAsPartName();

    report.addComment("Validating part \"" + partName + "\" using schema \"" + schemaName
                      + "\" ...");
    report.incIndent();

    if (std::string(mapping.contentType)
        == "application/vnd.openxmlformats-officedocument.vmlDrawing")
    {
        report.addComment("WARN", "Warning: deprecated content (VML) detected");
    }

    int errorsInPart = 0;

    if (schemaName.empty())
    {
        report.addComment("No schema known to validate content of type: "
                          + std::string(mapping.contentType));
    }
    else
    {
        // Validation errors carry the raw relationship target as their
        // location.
        ErrorCapper capper(report, target.name);

        std::string entryName = partName;
        if (!entryName.empty() && entryName[0] == '/')
            entryName = entryName.substr(1);

        std::string bytes;
        if (zip.extract(entryName, bytes))
        {
            std::string parseError;
            XmlDocPtr doc = parseXmlMemory(bytes, entryName, parseError);
            if (!doc)
            {
                capper.fatalError(parseError);
            }
            else
            {
                bool vmlStream = schemaName.find("vml") != std::string::npos;
                MCEFilter(doc.get(), vmlStream, capper);

                std::string schemaError;
                int schema = schemas.getSchema(schemaName, schemaError);
                if (!schema)
                {
                    report.addComment("ERROR", schemaError);
                }
                else
                {
                    // MCE filtering can remove the root element (VML
                    // streams for example): whatever elements remain at
                    // the top are validated one by one; none at all
                    // means nothing is left to check.
                    for (const auto& node : doc->children)
                        if (node->isElement())
                            validateElement(node.get(), schema, capper);
                }

                if (capper.getInstanceErrCount() > 0)
                {
                    report.addComment("\"" + partName + "\" contains "
                                      + std::to_string(capper.getInstanceErrCount())
                                      + " validity error"
                                      + (capper.getInstanceErrCount() > 1 ? "s" : ""));
                    errorsInPart = capper.getInstanceErrCount();
                }
                else
                {
                    report.addComment("\"" + partName + "\" is schema-valid");
                }
                capper.addOmissionNote();
            }
        }
    }

    report.decIndent();
    return errorsInPart;
}

} // namespace

int runOOXMLValidation(const ZipArchive& zip, ValidationReport& report, SchemaCache& schemas)
{
    OPCPackage opc(zip);
    opc.process();

    int errCount = 0;

    report.addComment("Checking OPC Package ...");
    report.incIndent();

    for (const OOXMLTarget& target : opc.targets())
    {
        const std::string& mimeType = target.mimeType;
        const OOXMLSchemaMapping* mapping = findMappingForContentType(mimeType);
        if (!mapping)
            continue;

        if (target.type != mapping->relType)
        {
            ++errCount;
            report.addComment("ERROR", "Entry with MIME type \"" + mimeType
                                           + "\" has unrecognized relationship type \""
                                           + target.type + "\" (see ISO/IEC 29500-1:2008, Clause "
                                           + mapping->clause + ")");
        }
    }

    if (errCount > 0)
    {
        report.addComment("ERROR", std::to_string(errCount) + " problem"
                                       + (errCount > 1 ? "s" : "") + " found with OPC package");
    }
    else
    {
        report.addComment("No problems found with OPC package");
    }

    report.decIndent();

    report.addComment("Validating " + std::to_string(opc.targets().size()) + " parts ...");

    for (const OOXMLTarget& target : opc.targets())
    {
        std::string mimeType = target.mimeType;
        const OOXMLSchemaMapping* mapping = findMappingForContentType(mimeType);

        // No override for this part: [Content_Types].xml may still map
        // its extension to a default content type.
        if (!mapping)
        {
            auto it = opc.defaultTypes().find(target.extension());
            if (it != opc.defaultTypes().end())
            {
                mimeType = it->second;
                mapping = findMappingForContentType(mimeType);
            }
        }

        if (!mapping)
        {
            report.addComment("Cannot determine schema for Part named (\"<![CDATA["
                              + target.targetAsPartName() + "]]>\")");
            continue;
        }

        errCount += validateTarget(zip, target, *mapping, report, schemas);
    }

    if (errCount > 0)
    {
        report.addComment("Grand total of errors in submitted package: "
                          + std::to_string(errCount));
    }

    return errCount;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
