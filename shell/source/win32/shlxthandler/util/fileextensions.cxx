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
#include "internal/fileextensions.hxx"
#include <sal/macros.h>

//------------------------------------

const std::string WRITER_FILE_EXTENSIONS   = "sxwstwsxgodtottodm";
const std::string CALC_FILE_EXTENSIONS     = "sxcstcodsots";
const std::string DRAW_FILE_EXTENSIONS     = "sxdstdodgotg";
const std::string IMPRESS_FILE_EXTENSIONS  = "sxistiodpotp";
const std::string MATH_FILE_EXTENSIONS     = "sxmodf";
const std::string WEB_FILE_EXTENSIONS      = "oth";
const std::string DATABASE_FILE_EXTENSIONS = "odb";

FileExtensionEntry OOFileExtensionTable[] = {
    { ".sxw", L".sxw", "soffice.StarWriterDocument.6"       },
    { ".sxc", L".sxc", "soffice.StarCalcDocument.6"         },
    { ".sxi", L".sxi", "soffice.StarImpressDocument.6"      },
    { ".sxd", L".sxd", "soffice.StarDrawDocument.6"         },
    { ".sxm", L".sxm", "soffice.StarMathDocument.6"         },
    { ".stw", L".stw", "soffice.StarWriterTemplate.6"       },
    { ".sxg", L".sxg", "soffice.StarWriterGlobalDocument.6" },
    { ".std", L".std", "soffice.StarDrawTemplate.6"         },
    { ".sti", L".sti", "soffice.StarImpressTemplate.6"      },
    { ".stc", L".stc", "soffice.StarCalcTemplate.6"         },
    { ".odt", L".odt", "LibreOffice.WriterDocument.1"       },
    { ".ott", L".ott", "LibreOffice.WriterTemplate.1"       },
    { ".odm", L".odm", "LibreOffice.WriterGlobalDocument.1" },
    { ".oth", L".oth", "LibreOffice.WriterWebTemplate.1"    },
    { ".ods", L".ods", "LibreOffice.CalcDocument.1"         },
    { ".ots", L".ots", "LibreOffice.CalcTemplate.1"         },
    { ".odg", L".odg", "LibreOffice.DrawDocument.1"         },
    { ".otg", L".otg", "LibreOffice.DrawTemplate.1"         },
    { ".odp", L".odp", "LibreOffice.ImpressDocument.1"      },
    { ".otp", L".otp", "LibreOffice.ImpressTemplate.1"      },
    { ".odf", L".odf", "LibreOffice.MathDocument.1"         },
    { ".odb", L".odb", "LibreOffice.DatabaseDocument.1"     }
    };


size_t OOFileExtensionTableSize = sizeof(OOFileExtensionTable)/sizeof(OOFileExtensionTable[0]);

//---------------------------------
/** Return the extension of a file
    name without the '.'
*/
std::string get_file_name_extension(const std::string& file_name)
{
    std::string::size_type idx = file_name.find_last_of(".");

    if (std::string::npos != idx++)
        return std::string(file_name.begin() + idx, file_name.end());

    return std::string();
}

//---------------------------------
/** Return the type of a file
*/

char easytolower( char in )
{
    if( in<='Z' && in>='A' )
        return in-('Z'-'z');
    return in;
}

File_Type_t get_file_type(const std::string& file_name)
{
    std::string fext = get_file_name_extension(file_name);
    std::transform(fext.begin(), fext.end(), fext.begin(), easytolower);

    if (std::string::npos != WRITER_FILE_EXTENSIONS.find(fext))
        return WRITER;
    else if (std::string::npos != CALC_FILE_EXTENSIONS.find(fext))
        return CALC;
    else if (std::string::npos != DRAW_FILE_EXTENSIONS.find(fext))
        return DRAW;
    else if (std::string::npos != IMPRESS_FILE_EXTENSIONS.find(fext))
        return IMPRESS;
    else if (std::string::npos != MATH_FILE_EXTENSIONS.find(fext))
        return MATH;
    else if (std::string::npos != WEB_FILE_EXTENSIONS.find(fext))
        return WEB;
    else if (std::string::npos != DATABASE_FILE_EXTENSIONS.find(fext))
        return DATABASE;
    else
        return UNKNOWN;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
