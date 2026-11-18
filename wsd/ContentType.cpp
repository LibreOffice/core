/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <config.h>

#include "ContentType.hpp"

#include <common/Util.hpp>

#include <string>
#include <string_view>
#include <unordered_map>

namespace ContentType
{

std::string_view fromFileName(const std::string_view fileName)
{
    static std::unordered_map<std::string_view, std::string_view> contentTypes{
        { "svg", "image/svg+xml" },
        { "pot", "application/vnd.ms-powerpoint" },
        { "xla", "application/vnd.ms-excel" },

        // Writer documents
        { "sxw", "application/vnd.sun.xml.writer" },
        { "odt", "application/vnd.oasis.opendocument.text" },
        { "fodt", "application/vnd.oasis.opendocument.text-flat-xml" },

        // Calc documents
        { "sxc", "application/vnd.sun.xml.calc" },
        { "ods", "application/vnd.oasis.opendocument.spreadsheet" },
        { "fods", "application/vnd.oasis.opendocument.spreadsheet-flat-xml" },

        // Impress documents
        { "sxi", "application/vnd.sun.xml.impress" },
        { "odp", "application/vnd.oasis.opendocument.presentation" },
        { "fodp", "application/vnd.oasis.opendocument.presentation-flat-xml" },

        // Draw documents
        { "sxd", "application/vnd.sun.xml.draw" },
        { "odg", "application/vnd.oasis.opendocument.graphics" },
        { "fodg", "application/vnd.oasis.opendocument.graphics-flat-xml" },

        // Chart documents
        { "odc", "application/vnd.oasis.opendocument.chart" },

        // Text master documents
        { "sxg", "application/vnd.sun.xml.writer.global" },
        { "odm", "application/vnd.oasis.opendocument.text-master" },

        // Math documents
        // In fact Math documents are not supported at all.
        // See: https://bugs.documentfoundation.org/show_bug.cgi?id=97006
        { "sxm", "application/vnd.sun.xml.math" },
        { "odf", "application/vnd.oasis.opendocument.formula" },

        // Text template documents
        { "stw", "application/vnd.sun.xml.writer.template" },
        { "ott", "application/vnd.oasis.opendocument.text-template" },

        // Writer master document templates
        { "otm", "application/vnd.oasis.opendocument.text-master-template" },

        // Spreadsheet template documents
        { "stc", "application/vnd.sun.xml.calc.template" },
        { "ots", "application/vnd.oasis.opendocument.spreadsheet-template" },

        // Presentation template documents
        { "sti", "application/vnd.sun.xml.impress.template" },
        { "otp", "application/vnd.oasis.opendocument.presentation-template" },

        // Drawing template documents
        { "std", "application/vnd.sun.xml.draw.template" },
        { "otg", "application/vnd.oasis.opendocument.graphics-template" },

        // MS Word
        { "doc", "application/msword" },
        { "dot", "application/msword" },

        // MS Excel
        { "xls", "application/vnd.ms-excel" },

        // MS PowerPoint
        { "ppt", "application/vnd.ms-powerpoint" },

        // OOXML wordprocessing
        { "docx", "application/vnd.openxmlformats-officedocument.wordprocessingml.document" },
        { "docm", "application/vnd.ms-word.document.macroEnabled.12" },
        { "dotx", "application/vnd.openxmlformats-officedocument.wordprocessingml.template" },
        { "dotm", "application/vnd.ms-word.template.macroEnabled.12" },

        // OOXML spreadsheet
        { "xltx", "application/vnd.openxmlformats-officedocument.spreadsheetml.template" },
        { "xltm", "application/vnd.ms-excel.template.macroEnabled.12" },
        { "xlsx", "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet" },
        { "xlsb", "application/vnd.ms-excel.sheet.binary.macroEnabled.12" },
        { "xlsm", "application/vnd.ms-excel.sheet.macroEnabled.12" },

        // OOXML presentation
        { "pptx", "application/vnd.openxmlformats-officedocument.presentationml.presentation" },
        { "pptm", "application/vnd.ms-powerpoint.presentation.macroEnabled.12" },
        { "potx", "application/vnd.openxmlformats-officedocument.presentationml.template" },
        { "potm", "application/vnd.ms-powerpoint.template.macroEnabled.12" },

        // Others
        { "wpd", "application/vnd.wordperfect" },
        { "pdb", "application/x-aportisdoc" },
        { "hwp", "application/x-hwp" },
        { "wps", "application/vnd.ms-works" },
        { "wri", "application/x-mswrite" },
        { "dif", "application/x-dif-document" },
        { "slk", "text/spreadsheet" },
        { "csv", "text/csv" },
        { "tsv", "text/tab-separated-values" },
        { "dbf", "application/x-dbase" },
        { "wk1", "application/vnd.lotus-1-2-3" },
        { "wks", "application/vnd.lotus-1-2-3" },
        { "wq2", "application/vnd.lotus-1-2-3" },
        { "123", "application/vnd.lotus-1-2-3" },
        { "wb1", "application/vnd.lotus-1-2-3" },
        { "wq1", "application/vnd.lotus-1-2-3" },
        { "xlr", "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet" },
        { "qpw", "application/vnd.ms-office" },
        { "cgm", "image/cgm" },
        { "dxf", "image/vnd.dxf" },
        { "emf", "image/x-emf" },
        { "wmf", "image/x-wmf" },
        { "cdr", "application/coreldraw" },
        { "vsd", "application/vnd.visio2013" },
        { "vss", "application/vnd.visio" },
        { "pub", "application/x-mspublisher" },
        { "lrf", "application/x-sony-bbeb" },
        { "gnumeric", "application/x-gnumeric" },
        { "mw", "application/macwriteii" },
        { "numbers", "application/x-iwork-numbers-sffnumbers" },
        { "oth", "application/vnd.oasis.opendocument.text-web" },
        { "p65", "application/x-pagemaker" },
        { "rtf", "text/rtf" },
        { "txt", "text/plain" },
        { "fb2", "application/x-fictionbook+xml" },
        { "cwk", "application/clarisworks" },
        { "wpg", "image/x-wpg" },
        { "pages", "application/x-iwork-pages-sffpages" },
        { "ppsx", "application/vnd.openxmlformats-officedocument.presentationml.slideshow" },
        { "key", "application/x-iwork-keynote-sffkey" },
        { "abw", "application/x-abiword" },
        { "fh", "image/x-freehand" },
        { "sxs", "application/vnd.sun.xml.chart" },
        { "602", "application/x-t602" },
        { "bmp", "image/bmp" },
        { "png", "image/png" },
        { "gif", "image/gif" },
        { "tiff", "image/tiff" },
        { "jpg", "image/jpeg" },
        { "jpeg", "image/jpeg" },
        { "pdf", "application/pdf" },
    };

    const auto dotPos = fileName.rfind('.');
    if (dotPos == std::string_view::npos || dotPos + 1 >= fileName.size())
        return "application/octet-stream";

    // Extension only (after the last dot, no path separators).
    std::string ext(fileName.substr(dotPos + 1));
    const auto slashPos = ext.rfind('/');
    if (slashPos != std::string::npos)
        return "application/octet-stream";

    const auto it = contentTypes.find(Util::toLowerInplace(ext));
    if (it != contentTypes.end())
        return it->second;

    return "application/octet-stream";
}

bool isSpreadsheet(const std::string_view fileName)
{
    const std::string_view contentType = fromFileName(fileName);

    return contentType == "application/vnd.oasis.opendocument.spreadsheet" ||
           contentType == "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet" ||
           contentType == "application/vnd.ms-excel";
}

} // namespace ContentType

/* vim:set shiftwidth=4 expandtab: */
