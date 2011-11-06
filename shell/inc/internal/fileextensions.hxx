/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef FILEEXTENSIONS_HXX_INCLUDED
#define FILEEXTENSIONS_HXX_INCLUDED

#include "internal/global.hxx"

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
