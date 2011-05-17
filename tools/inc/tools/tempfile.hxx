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
#ifndef _TOOLS_TEMPFILE_HXX
#define _TOOLS_TEMPFILE_HXX

#include <tools/string.hxx>
#include "tools/toolsdllapi.h"

struct TempFile_Impl;
class TOOLS_DLLPUBLIC TempFile
{
    TempFile_Impl*  pImp;
    sal_Bool        bKillingFileEnabled;

public:
                    // Create a temporary file or directory in a given folder or the default tempfile folder
                    TempFile( const String* pParent=NULL, sal_Bool bDirectory=sal_False );

                    // Create a temporary file or directory in a given folder or the default tempfile folder; its name starts
                    // with some given characters followed by a counter ( example: rLeadingChars="abc" means "abc0","abc1"
                    // and so on, depending on existing files in that folder ).
                    // The extension string may be f.e. ".txt" or "", if no extension string is given, ".tmp" is used
                    TempFile( const String& rLeadingChars, const String* pExtension=NULL, const String* pParent=NULL, sal_Bool bDirectory=sal_False );

                    // TempFile will be removed from disk in dtor if EnableKillingTempFile was called before.
                    // TempDirs will be removed recursively in that case.
                    ~TempFile();

    sal_Bool        IsValid() const;

                    // Returns the real name of the tempfile in file URL scheme.
    String          GetName() const;

                    // If enabled the file will be removed from disk when the dtor is called ( default is not enabled )
    void            EnableKillingFile( sal_Bool bEnable=sal_True )
                    { bKillingFileEnabled = bEnable; }

    sal_Bool        IsKillingFileEnabled() const
                    { return bKillingFileEnabled; }

                    // Only create a name for a temporary file that would be valid at that moment.
    static String   CreateTempName( const String* pParent=NULL );

                    // The TempNameBase is a folder in the default ( system ) tempfile folder.
                    // This subfolder will be used if a TempFile or TempName is created without a parent name.
                    // The caller of the SetTempNameBase is responsible for deleting this folder and all temporary files in it.
                    // The argument must be a simple name, not a complete URL.
                    // The return value of both methods is the complete URL of the tempname base folder.
    static String   SetTempNameBaseDirectory( const String &rBaseName );
    static String   GetTempNameBaseDirectory();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
