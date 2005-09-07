/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fileextensions.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:36:17 $
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
#define FILEEXTENSIONS_HXX_INCLUDED

#ifndef GLOBAL_HXX_INCLUDED
#include "internal/global.hxx"
#endif

#include <string>

// A simple table with information about the currently used OO file extensions
// for instance ".sxw" and information about windows registry keys which are
// necessary for properly registering of the shell extensions

/** A FileExtensionEntry consists of the extension as ansi and as
    unicode string and of the currently used registry forward key
    for this extension
*/
struct FileExtensionEntry
{
    char*    ExtensionAnsi;         // e.g. ".sxw"
    wchar_t* ExtensionUnicode;      // e.g. L".sxw"
    char*    RegistryForwardKey;    // e.g. "soffice.StarWriterDocument.6"
};

extern FileExtensionEntry OOFileExtensionTable[];

extern size_t OOFileExtensionTableSize;

//---------------------------------
/** Return the extension of a file
    name without the '.'
*/
std::string get_file_name_extension(const std::string& file_name);

//---------------------------------
/** Return the type of a file
*/

enum File_Type_t { UNKNOWN, WRITER, CALC, DRAW, IMPRESS, MATH, WEB, DATABASE };

File_Type_t get_file_type(const std::string& file_name);

#endif
