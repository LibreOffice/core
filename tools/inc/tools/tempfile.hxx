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
    /** Create a temporary file or directory in a given folder or the default
        tempfile folder. */
    TempFile( const String* pParent=NULL, sal_Bool bDirectory=sal_False );

    /** Create a temporary file or directory in a given folder or the default
        tempfile folder; its name starts with some given characters followed by
        a counter ( example: rLeadingChars="abc" means "abc0","abc1" and so on,
        depending on existing files in that folder ).

        The extension string may be f.e. ".txt" or "", if no extension string is
        given, ".tmp" is used.
    */
    TempFile( const String& rLeadingChars, const String* pExtension=NULL, const String* pParent=NULL, sal_Bool bDirectory=sal_False );

    /** TempFile will be removed from disk in dtor if EnableKillingTempFile was
        called before. TempDirs will be removed recursively in that case. */
    ~TempFile();

    sal_Bool IsValid() const;

    /** Returns the real name of the tempfile in file URL scheme. */
    String GetName() const;

    /** If enabled the file will be removed from disk when the dtor is called
        (default is not enabled) */
    void EnableKillingFile( sal_Bool bEnable=sal_True ) { bKillingFileEnabled = bEnable; }

    sal_Bool IsKillingFileEnabled() const { return bKillingFileEnabled; }

    /** Only create a name for a temporary file that would be valid at that moment. */
    static String   CreateTempName( const String* pParent=NULL );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
