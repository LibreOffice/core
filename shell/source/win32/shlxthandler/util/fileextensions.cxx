/*************************************************************************
 *
 *  $RCSfile: fileextensions.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: pjunck $ $Date: 2004-11-03 08:02:17 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef FILEEXTENSIONS_HXX_INCLUDED
#include "internal/fileextensions.hxx"
#endif

//------------------------------------
//
//------------------------------------

const std::string WRITER_FILE_EXTENSIONS   = "sxwstwsxgootottoom";
const std::string CALC_FILE_EXTENSIONS     = "sxcstcoosots";
const std::string DRAW_FILE_EXTENSIONS     = "sxdstdoodotd";
const std::string IMPRESS_FILE_EXTENSIONS  = "sxistioopotp";
const std::string MATH_FILE_EXTENSIONS     = "sxmoof";
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
    { ".oot", L".oot", "openoffice.WriterDocument.1"       },
    { ".ott", L".ott", "openoffice.WriterTemplate.1"       },
    { ".oom", L".oom", "openoffice.WriterGlobalDocument.1" },
    { ".oth", L".oth", "openoffice.WriterWebDocument.1"    },
    { ".oos", L".oos", "openoffice.CalcDocument.1"         },
    { ".ots", L".ots", "openoffice.CalcTemplate.1"         },
    { ".ood", L".ood", "openoffice.DrawDocument.1"         },
    { ".otd", L".otd", "openoffice.DrawTemplate.1"         },
    { ".oop", L".oop", "openoffice.ImpressDocument.1"      },
    { ".otp", L".otp", "openoffice.ImpressTemplate.1"      },
    { ".oof", L".oof", "openoffice.MathDocument.1"         },
    { ".odb", L".odb", "openoffice.DatabaseDocument.1"     }
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

