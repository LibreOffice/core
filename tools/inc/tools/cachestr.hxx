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

#ifndef _CACHESTR_HXX
#define _CACHESTR_HXX

#include <tools/stream.hxx>
#include <tools/string.hxx>
#include <tools/link.hxx>
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

    Link            aFilenameLinkHdl;

    TOOLS_DLLPRIVATE virtual sal_uIntPtr   GetData( void* pData, sal_uIntPtr nSize );
    TOOLS_DLLPRIVATE virtual sal_uIntPtr   PutData( const void* pData, sal_uIntPtr nSize );
    TOOLS_DLLPRIVATE virtual sal_uIntPtr   SeekPos( sal_uIntPtr nPos );
    TOOLS_DLLPRIVATE virtual void    FlushData();
    TOOLS_DLLPRIVATE virtual void    SetSize( sal_uIntPtr nSize );

public:
                    SvCacheStream( sal_uIntPtr nMaxMemSize = 0 );
                    SvCacheStream( const String &rFileName,
                                   sal_uIntPtr nExpectedSize = 0,
                                   sal_uIntPtr nMaxMemSize = 0 );
                    ~SvCacheStream();

    void            SetFilenameHdl( const Link& rLink);
    const Link&     GetFilenameHdl() const;
    void            SetFilename( const String& rFN )
                 { aFileName = rFN; } // darf nur vom FilenameHdl gerufen werden!
    const String&   GetFilename() const { return aFileName; }

    void            SwapOut();
    const void*     GetBuffer();
    sal_uIntPtr           GetSize();

    sal_Bool            IsPersistent() { return bPersistent != 0; }
    void            SetPersistence( sal_Bool b = sal_True ) { bPersistent = b; }
    void            SetSwapStream( SvStream *p )
                 { pSwapStream = p; } // darf nur vom FilenameHdl gerufen werden!
};

#endif
