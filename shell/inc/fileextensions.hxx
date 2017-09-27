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

#ifndef INCLUDED_SHELL_INC_INTERNAL_FILEEXTENSIONS_HXX
#define INCLUDED_SHELL_INC_INTERNAL_FILEEXTENSIONS_HXX

#include "global.hxx"

#include <string>
#include <filepath.hxx>

// A simple table with information about the currently used OO file extensions
// for instance ".sxw" and information about windows registry keys which are
// necessary for properly registering of the shell extensions

/** A FileExtensionEntry consists of the extension as ansi and as
    unicode string and of the currently used registry forward key
    for this extension
*/
struct FileExtensionEntry
{
    const char*    ExtensionA;            // e.g. ".sxw"
    const wchar_t* ExtensionU;            // e.g. L".sxw"
    const wchar_t* RegistryForwardKey;    // e.g. "soffice.StarWriterDocument.6"
};

extern const FileExtensionEntry OOFileExtensionTable[];

extern const size_t OOFileExtensionTableSize;

/** Return the extension of a file
    name without the '.'
*/
Filepath_t get_file_name_extension(const Filepath_t& file_name);


/** Return the type of a file
*/

enum File_Type_t { UNKNOWN, WRITER, CALC, DRAW, IMPRESS, MATH, WEB, DATABASE };

File_Type_t get_file_type(const Filepath_t& file_name);

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
