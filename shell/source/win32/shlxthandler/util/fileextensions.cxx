/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fileextensions.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:06:01 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef FILEEXTENSIONS_HXX_INCLUDED
#include "internal/fileextensions.hxx"
#endif

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

File_Type_t get_file_type(const std::string& file_name)
{
    std::string fext = get_file_name_extension(file_name);

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

