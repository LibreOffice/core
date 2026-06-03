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

#pragma once

#include <common/JsonUtil.hpp>
#include <common/Util.hpp>

#define KIT_USE_UNSTABLE_API
#include <COKit/COKit.h>
#include <COKit/COKitEnums.h>

#include <cstdlib>
#include <string>
#include <unordered_map>

namespace LOKitHelper
{
    constexpr auto tunnelledDialogImageCacheSize = 100;

    struct StringDeleter
    {
        void operator()(char* string) { std::free(string); }
    };
    using ScopedString = std::unique_ptr<char, StringDeleter>;

    inline std::string documentTypeToString(COKitDocumentType type)
    {
        switch (type)
        {
        case KIT_DOCTYPE_TEXT:
            return "text";
        case KIT_DOCTYPE_SPREADSHEET:
            return "spreadsheet";
        case KIT_DOCTYPE_PRESENTATION:
            return "presentation";
        case KIT_DOCTYPE_DRAWING:
            return "drawing";
        default:
            return "other-" + std::to_string(type);
        }
    }

    inline std::string getPartData(COKitDocument *loKitDocument, int part)
    {
        ScopedString ptrToData(loKitDocument->pClass->getPartInfo(loKitDocument, part));
        std::string result(ptrToData.get());
        return result;
    }

    inline std::string MapToJSONString(std::unordered_map<std::string, std::string> &map)
    {
        std::string resultingString = "{";
        for (std::unordered_map<std::string, std::string>::iterator i = map.begin(); i != map.end(); i++)
        {
            resultingString += "\"" + i->first + "\": " + i->second + ',';
        }
        resultingString.pop_back();
        resultingString += "}";

        return resultingString;
    }

    inline int getMode(const std::string &partData)
    {
        Poco::JSON::Parser parser;
        Poco::Dynamic::Var partJsonVar = parser.parse(partData);
        const Poco::SharedPtr<Poco::JSON::Object>& partObject = partJsonVar.extract<Poco::JSON::Object::Ptr>();

        if (partObject->has("mode"))
            return std::atoi(partObject->get("mode").toString().c_str());
        return 0;
    }

    inline std::string partHasComments(const std::string &partData)
    {
        Poco::JSON::Parser parser;
        Poco::Dynamic::Var partJsonVar = parser.parse(partData);
        const Poco::SharedPtr<Poco::JSON::Object>& partObject = partJsonVar.extract<Poco::JSON::Object::Ptr>();
        if (partObject->has("partHasComments"))
            return partObject->get("partHasComments").toString();
        return "false";
    }

    inline void fetchPartsData(COKitDocument *loKitDocument, std::unordered_map<std::string, std::string> &resultInfo, int partsCount, int &mode)
    {
        /*
            Except for Writer.

            Since parts should be an array, we will start an array and put parts into it.
            We are building a JSON array.
        */

        std::string resultingPartsArray = "[";

        for (int i = 0; i < partsCount; ++i)
        {
            std::string partData = getPartData(loKitDocument, i); // Part data is sent from the core side as JSON string.
            resultingPartsArray += partData + (i < partsCount - 1 ? ", ": "]");

            if (i == 0)
                mode = getMode(partData);
        }

        resultInfo["parts"] = std::move(resultingPartsArray);
    }

    // TODO: create a struct with all the resultInfo properties and
    inline void fetchWriterSpecificData(COKitDocument *loKitDocument, std::unordered_map<std::string, std::string> &resultInfo, int& mode, std::string& hasComments)
    {
        std::string rectangles = loKitDocument->pClass->getPartPageRectangles(loKitDocument);

        rectangles = Util::replace(rectangles, ";", "], [");

        resultInfo["pagerectangles"] = "[ [" + rectangles + "] ]";

        // Fetch mode for a potentially non-standard redline render mode.
        std::string partData = getPartData(loKitDocument, 0);
        mode = getMode(partData);
        hasComments = partHasComments(partData);
    }

    inline void fetchCalcSpecificData(COKitDocument *loKitDocument, std::unordered_map<std::string, std::string> &resultInfo, int part)
    {
        long lastColumn, lastRow;
        loKitDocument->pClass->getDataArea(loKitDocument, part, &lastColumn, &lastRow);
        resultInfo["lastcolumn"] = std::to_string(lastColumn);
        resultInfo["lastrow"] = std::to_string(lastRow);

        ScopedString value(loKitDocument->pClass->getCommandValues(loKitDocument, ".uno:DefinePrintArea"));
        if (value)
        {
            resultInfo["printranges"] = std::string(value.get());
        }
    }

    inline std::string getDocumentTypeAsString(COKitDocument *loKitDocument)
    {
        assert(loKitDocument && "null loKitDocument");
        const auto type = static_cast<COKitDocumentType>(loKitDocument->pClass->getDocumentType(loKitDocument));
        return documentTypeToString(type);
    }

    inline std::string documentStatus(COKitDocument *loKitDocument, bool diffSizePages = false)
    {
        assert(loKitDocument && "null loKitDocument");
        const auto type = static_cast<COKitDocumentType>(loKitDocument->pClass->getDocumentType(loKitDocument));

        std::unordered_map<std::string, std::string> resultInfo;

        const int partsCount = loKitDocument->pClass->getParts(loKitDocument);
        const int selectedPart = loKitDocument->pClass->getPart(loKitDocument);

        long width, height;
        loKitDocument->pClass->getDocumentSize(loKitDocument, &width, &height);
        int viewId = loKitDocument->pClass->getView(loKitDocument);

        resultInfo["type"] = '"' + documentTypeToString(type) + '"';
        resultInfo["partscount"] = std::to_string(partsCount);
        resultInfo["selectedpart"] = std::to_string(selectedPart);
        resultInfo["width"] = std::to_string(width);
        resultInfo["height"] = std::to_string(height);
        resultInfo["viewid"] = std::to_string(viewId);

        ScopedString value(loKitDocument->pClass->getCommandValues(loKitDocument, ".uno:ReadOnly"));
        if (value)
        {
            const std::string isReadOnly = std::string(value.get());

            bool readOnly = (isReadOnly.find("true") != std::string::npos);
            resultInfo["readonly"] = readOnly ? "true": "false";
        }

        ScopedString externalLinks(loKitDocument->pClass->getCommandValues(loKitDocument, ".uno:ExternalLinksDisabled"));
        if (externalLinks)
        {
            const std::string isExternalLinksDisabled = std::string(externalLinks.get());

            bool externalLinksDisabled = (isExternalLinksDisabled.find("true") != std::string::npos);
            resultInfo["externallinksdisabled"] = externalLinksDisabled ? "true": "false";
        }

        ScopedString values(loKitDocument->pClass->getCommandValues(loKitDocument, ".uno:AllPageSize"));
        if (values)
        {
            Poco::JSON::Parser parser;
            const auto var = parser.parse(values.get());
            const auto& obj = var.extract<Poco::JSON::Object::Ptr>();
            if (obj && obj->has("parts"))
            {
                const auto parts = obj->getArray("parts");
                std::ostringstream os;
                parts->stringify(os);
                resultInfo["partdimensions"] = os.str();
            }
        }

        if (diffSizePages)
        {
            resultInfo["currentpageresized"] = std::to_string(true);
            return MapToJSONString(resultInfo);
        }

        int mode = 0;
        std::string hasComments = "false";

        if (type == KIT_DOCTYPE_SPREADSHEET)
            fetchCalcSpecificData(loKitDocument, resultInfo, selectedPart);
        else if (type == KIT_DOCTYPE_TEXT)
            fetchWriterSpecificData(loKitDocument, resultInfo, mode, hasComments);

        if (type == KIT_DOCTYPE_SPREADSHEET || type == KIT_DOCTYPE_PRESENTATION || type == KIT_DOCTYPE_DRAWING)
            fetchPartsData(loKitDocument, resultInfo, partsCount, mode);

        resultInfo["mode"] = std::to_string(mode);
        resultInfo["partHasComments"] = std::move(hasComments);

        return MapToJSONString(resultInfo);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
