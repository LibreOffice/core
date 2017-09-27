/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "algorithm"
#include "fileextensions.hxx"
#include <rtl/character.hxx>
#include <sal/macros.h>


const std::wstring WRITER_FILE_EXTENSIONS   = L"sxwstwsxgodtottodm";
const std::wstring CALC_FILE_EXTENSIONS     = L"sxcstcodsots";
const std::wstring DRAW_FILE_EXTENSIONS     = L"sxdstdodgotg";
const std::wstring IMPRESS_FILE_EXTENSIONS  = L"sxistiodpotp";
const std::wstring MATH_FILE_EXTENSIONS     = L"sxmodf";
const std::wstring WEB_FILE_EXTENSIONS      = L"oth";
const std::wstring DATABASE_FILE_EXTENSIONS = L"odb";

const FileExtensionEntry OOFileExtensionTable[] = {
    { ".sxw", L".sxw", L"soffice.StarWriterDocument.6"       },
    { ".sxc", L".sxc", L"soffice.StarCalcDocument.6"         },
    { ".sxi", L".sxi", L"soffice.StarImpressDocument.6"      },
    { ".sxd", L".sxd", L"soffice.StarDrawDocument.6"         },
    { ".sxm", L".sxm", L"soffice.StarMathDocument.6"         },
    { ".stw", L".stw", L"soffice.StarWriterTemplate.6"       },
    { ".sxg", L".sxg", L"soffice.StarWriterGlobalDocument.6" },
    { ".std", L".std", L"soffice.StarDrawTemplate.6"         },
    { ".sti", L".sti", L"soffice.StarImpressTemplate.6"      },
    { ".stc", L".stc", L"soffice.StarCalcTemplate.6"         },
    { ".odt", L".odt", L"LibreOffice.WriterDocument.1"       },
    { ".ott", L".ott", L"LibreOffice.WriterTemplate.1"       },
    { ".odm", L".odm", L"LibreOffice.WriterGlobalDocument.1" },
    { ".oth", L".oth", L"LibreOffice.WriterWebTemplate.1"    },
    { ".ods", L".ods", L"LibreOffice.CalcDocument.1"         },
    { ".ots", L".ots", L"LibreOffice.CalcTemplate.1"         },
    { ".odg", L".odg", L"LibreOffice.DrawDocument.1"         },
    { ".otg", L".otg", L"LibreOffice.DrawTemplate.1"         },
    { ".odp", L".odp", L"LibreOffice.ImpressDocument.1"      },
    { ".otp", L".otp", L"LibreOffice.ImpressTemplate.1"      },
    { ".odf", L".odf", L"LibreOffice.MathDocument.1"         },
    { ".odb", L".odb", L"LibreOffice.DatabaseDocument.1"     }
    };


const size_t OOFileExtensionTableSize = SAL_N_ELEMENTS(OOFileExtensionTable);


/** Return the extension of a file
    name without the '.'
*/
std::wstring get_file_name_extension(const std::wstring& file_name)
{
    std::wstring::size_type idx = file_name.find_last_of(L".");

    if (std::wstring::npos != idx++)
        return std::wstring(file_name.begin() + idx, file_name.end());

    return std::wstring();
}


/** Return the type of a file
*/

File_Type_t get_file_type(const std::wstring& file_name)
{
    std::wstring fext = get_file_name_extension(file_name);
    std::transform(
        fext.begin(), fext.end(), fext.begin(),
        [](wchar_t c) {
            return rtl::toAsciiLowerCase(c); });

    if (std::wstring::npos != WRITER_FILE_EXTENSIONS.find(fext))
        return WRITER;
    else if (std::wstring::npos != CALC_FILE_EXTENSIONS.find(fext))
        return CALC;
    else if (std::wstring::npos != DRAW_FILE_EXTENSIONS.find(fext))
        return DRAW;
    else if (std::wstring::npos != IMPRESS_FILE_EXTENSIONS.find(fext))
        return IMPRESS;
    else if (std::wstring::npos != MATH_FILE_EXTENSIONS.find(fext))
        return MATH;
    else if (std::wstring::npos != WEB_FILE_EXTENSIONS.find(fext))
        return WEB;
    else if (std::wstring::npos != DATABASE_FILE_EXTENSIONS.find(fext))
        return DATABASE;
    else
        return UNKNOWN;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
