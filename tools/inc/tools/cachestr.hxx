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

#ifndef _CACHESTR_HXX
#define _CACHESTR_HXX

#include <tools/stream.hxx>
#include <tools/string.hxx>
#include "tools/toolsdllapi.h"

// -----------------
// - SvCacheStream -
// -----------------

class TempFile;
class TOOLS_DLLPUBLIC SvCacheStream : public SvStream
{
private:
    String          aFileName;
    sal_uIntPtr           nMaxSize;
    int             bPersistent;

    SvStream*       pSwapStream;
    SvStream*       pCurrentStream;
    TempFile*       pTempFile;

    TOOLS_DLLPRIVATE virtual sal_uIntPtr   GetData( void* pData, sal_uIntPtr nSize );
    TOOLS_DLLPRIVATE virtual sal_uIntPtr   PutData( const void* pData, sal_uIntPtr nSize );
    TOOLS_DLLPRIVATE virtual sal_uIntPtr   SeekPos( sal_uIntPtr nPos );
    TOOLS_DLLPRIVATE virtual void    FlushData();
    TOOLS_DLLPRIVATE virtual void    SetSize( sal_uIntPtr nSize );

public:
                    SvCacheStream( sal_uIntPtr nMaxMemSize = 0 );
                    ~SvCacheStream();

    void            SetFilename( const String& rFN )
                 { aFileName = rFN; } // call only from FilenameHdl
    const String&   GetFilename() const { return aFileName; }

    void            SwapOut();
    const void*     GetBuffer();
    sal_uIntPtr           GetSize();

    sal_Bool            IsPersistent() { return bPersistent != 0; }
    void            SetPersistence( sal_Bool b = sal_True ) { bPersistent = b; }
    void            SetSwapStream( SvStream *p )
                 { pSwapStream = p; } // call only from FilenameHdl
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
