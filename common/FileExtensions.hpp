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

#include <common/ContainerUtil.hpp>

#include <cctype>
#include <map>
#include <set>
#include <string>

namespace FileExtensions
{
/// Map a file extension to a document type for password-protected icons.
inline std::string getDocTypeFromExtension(const std::string& ext)
{
    static const Util::UnorderedStringMap<std::string> extToType = {
        // Writer
        { "odt", "writer" }, { "fodt", "writer" }, { "doc", "writer" }, { "docx", "writer" },
        { "docm", "writer" }, { "dot", "writer" }, { "dotx", "writer" }, { "dotm", "writer" },
        { "rtf", "writer" }, { "txt", "writer" }, { "wpd", "writer" }, { "wps", "writer" },
        { "sxw", "writer" }, { "stw", "writer" }, { "ott", "writer" }, { "otm", "writer" },
        { "hwp", "writer" }, { "wri", "writer" }, { "abw", "writer" }, { "pages", "writer" },
        // Calc
        { "ods", "calc" }, { "fods", "calc" }, { "xls", "calc" }, { "xlsx", "calc" },
        { "xlsm", "calc" }, { "xlsb", "calc" }, { "xla", "calc" }, { "xltx", "calc" },
        { "xltm", "calc" }, { "csv", "calc" }, { "tsv", "calc" }, { "sxc", "calc" },
        { "stc", "calc" }, { "ots", "calc" }, { "dbf", "calc" }, { "numbers", "calc" },
        // Impress
        { "odp", "impress" }, { "fodp", "impress" }, { "ppt", "impress" }, { "pptx", "impress" },
        { "pptm", "impress" }, { "pot", "impress" }, { "potx", "impress" }, { "potm", "impress" },
        { "ppsx", "impress" }, { "sxi", "impress" }, { "sti", "impress" }, { "otp", "impress" },
        { "key", "impress" },
        // Draw
        { "odg", "draw" }, { "fodg", "draw" }, { "vsd", "draw" }, { "vss", "draw" },
        { "pub", "draw" }, { "sxd", "draw" }, { "std", "draw" }, { "otg", "draw" },
        { "cdr", "draw" }, { "wpg", "draw" }, { "cgm", "draw" }, { "emf", "draw" },
        { "wmf", "draw" },
    };

    auto it = extToType.find(ext);
    return (it != extToType.end()) ? it->second : "writer";
}

// The document kind the tab strip keys its icon and colour off. The engine
// decides what a file really is when it opens it; the strip needs a name for it
// before that, and the extension is what there is to go on.
inline std::string docTypeFromFilename(const std::string& filename)
{
    const auto lastPeriod = filename.find_last_of('.');
    if (lastPeriod == std::string::npos)
        return "other";

    std::string extension = filename.substr(lastPeriod + 1);
    for (char& c : extension)
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));

    static const std::set<std::string> writer = { "odt", "doc", "docx", "docm", "rtf", "txt", "fodt" };
    static const std::set<std::string> calc = { "ods", "xls", "xlsx", "xlsm", "csv", "fods" };
    static const std::set<std::string> impress = { "odp", "ppt", "pptx", "pptm", "ppsx", "fodp" };
    static const std::set<std::string> draw = { "odg", "fodg" };

    if (writer.count(extension))
        return "writer";
    if (calc.count(extension))
        return "calc";
    if (impress.count(extension))
        return "impress";
    if (draw.count(extension))
        return "draw";
    return "other";
}

/// The document type of a lower-case suffix, for the Qt app.
inline std::string docTypeFromSuffix(const std::string& ext)
{
    static const std::set<std::string> writerExt = { "odt", "ott", "doc", "docx",
                                                     "rtf", "txt", "fodt" };
    static const std::set<std::string> calcExt = { "ods", "ots", "xls", "xlsx", "csv", "fods" };
    static const std::set<std::string> impressExt = { "odp", "otp", "ppt", "pptx", "fodp" };
    static const std::set<std::string> drawExt = { "odg", "otg", "fodg" };
    if (writerExt.contains(ext))
        return "writer";
    if (calcExt.contains(ext))
        return "calc";
    if (impressExt.contains(ext))
        return "impress";
    if (drawExt.contains(ext))
        return "draw";
    return "other";
}

// This is the list of ViewFilter_Application::isFilteredExtension in
// engine/sfx2/source/control/templatelocalview.cxx, plus dotm.
inline const std::map<std::string, std::string>& typeOfTemplateExtension()
{
    static const std::map<std::string, std::string> types = {
        { "ott", "writer" },  { "ots", "calc" },   { "otp", "impress" },  { "otg", "draw" },
        { "otm", "writer" },  { "oth", "writer" }, { "stw", "writer" },   { "stc", "calc" },
        { "sti", "impress" }, { "std", "draw" },   { "dotx", "writer" },  { "dotm", "writer" },
        { "xltx", "calc" },   { "xltm", "calc" },  { "potx", "impress" }, { "potm", "impress" },
        { "dot", "writer" },  { "xlt", "calc" },   { "pot", "impress" },
    };
    return types;
}

// The extension of the document that each template file creates. The older StarOffice templates
// create an ODF document.
inline const std::map<std::string, std::string>& documentExtensionOfTemplateExtension()
{
    static const std::map<std::string, std::string> extensions = {
        { "ott", "odt" },   { "ots", "ods" },   { "otp", "odp" },   { "otg", "odg" },
        { "otm", "odm" },   { "oth", "html" },  { "stw", "odt" },   { "stc", "ods" },
        { "sti", "odp" },   { "std", "odg" },   { "dotx", "docx" }, { "dotm", "docm" },
        { "xltx", "xlsx" }, { "xltm", "xlsm" }, { "potx", "pptx" }, { "potm", "pptm" },
        { "dot", "doc" },   { "xlt", "xls" },   { "pot", "ppt" },
    };
    return extensions;
}

// Extensions of ordinary documents: ODF, flat ODF, OOXML, the older binary formats and RTF.
inline const std::map<std::string, std::string>& typeOfDocumentExtension()
{
    static const std::map<std::string, std::string> types = {
        { "odt", "writer" },   { "ods", "calc" },     { "odp", "impress" },  { "odg", "draw" },
        { "fodt", "writer" },  { "fods", "calc" },    { "fodp", "impress" }, { "fodg", "draw" },
        { "docx", "writer" },  { "docm", "writer" },  { "xlsx", "calc" },    { "xlsm", "calc" },
        { "pptx", "impress" }, { "pptm", "impress" }, { "doc", "writer" },   { "rtf", "writer" },
        { "xls", "calc" },     { "ppt", "impress" },
    };
    return types;
}
} // namespace FileExtensions

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
