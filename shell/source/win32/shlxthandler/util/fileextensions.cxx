/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_shell.hxx"
#include "algorithm"
#include "internal/fileextensions.hxx"
#include <sal/macros.h>

//------------------------------------
//
//------------------------------------

const std::string WRITER_FILE_EXTENSIONS   = "sxwstwsxgodtottodm";
const std::string CALC_FILE_EXTENSIONS     = "sxcstcodsots";
const std::string DRAW_FILE_EXTENSIONS     = "sxdstdodgotg";
const std::string IMPRESS_FILE_EXTENSIONS  = "sxistiodpotp";
const std::string MATH_FILE_EXTENSIONS     = "sxmodf";
const std::string WEB_FILE_EXTENSIONS      = "oth";
const std::string DATABASE_FILE_EXTENSIONS = "odb";

FileExtensionEntry OOFileExtensionTable[] = {
    { ".sxw", L".sxw", "soffice.StarWriterDocument.6"      },
    { ".sxc", L".sxc", "soffice.StarCalcDocument.6"        },
    { ".sxi", L".sxi", "soffice.StarImpressDocument.6"     },
    { ".sxd", L".sxd", "soffice.StarDrawDocument.6"        },
    { ".sxm", L".sxm", "soffice.StarMathDocument.6"        },
    { ".stw", L".stw", "soffice.StarWriterTemplate.6"      },
    { ".sxg", L".sxg", "soffice.StarWriterGlobalDocument.6"},
    { ".std", L".std", "soffice.StarDrawTemplate.6"        },
    { ".sti", L".sti", "soffice.StarImpressTemplate.6"     },
    { ".stc", L".stc", "soffice.StarCalcTemplate.6"        },
    { ".odt", L".odt", "opendocument.WriterDocument.1"       },
    { ".ott", L".ott", "opendocument.WriterTemplate.1"       },
    { ".odm", L".odm", "opendocument.WriterGlobalDocument.1" },
    { ".oth", L".oth", "opendocument.WriterWebTemplate.1"    },
    { ".ods", L".ods", "opendocument.CalcDocument.1"         },
    { ".ots", L".ots", "opendocument.CalcTemplate.1"         },
    { ".odg", L".odg", "opendocument.DrawDocument.1"         },
    { ".otg", L".otg", "opendocument.DrawTemplate.1"         },
    { ".odp", L".odp", "opendocument.ImpressDocument.1"      },
    { ".otp", L".otp", "opendocument.ImpressTemplate.1"      },
    { ".odf", L".odf", "opendocument.MathDocument.1"         },
    { ".odb", L".odb", "opendocument.DatabaseDocument.1"     }
    };


size_t OOFileExtensionTableSize = SAL_N_ELEMENTS(OOFileExtensionTable);

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
