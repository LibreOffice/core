/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cachestr.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 20:08:36 $
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

#ifndef _CACHESTR_HXX
#define _CACHESTR_HXX

#ifndef _STREAM_HXX
#include <tools/stream.hxx>
#endif
#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef _LINK_HXX
#include <tools/link.hxx>
#endif
#ifndef INCLUDED_TOOLSDLLAPI_H
#include "tools/toolsdllapi.h"
#endif

// -----------------
// - SvCacheStream -
// -----------------

class TempFile;
class TOOLS_DLLPUBLIC SvCacheStream : public SvStream
{
private:
    String          aFileName;
    ULONG           nMaxSize;
    int             bPersistent;

    SvStream*       pSwapStream;
    SvStream*       pCurrentStream;
    TempFile*       pTempFile;

    Link            aFilenameLinkHdl;

    TOOLS_DLLPRIVATE virtual ULONG   GetData( void* pData, ULONG nSize );
    TOOLS_DLLPRIVATE virtual ULONG   PutData( const void* pData, ULONG nSize );
    TOOLS_DLLPRIVATE virtual ULONG   SeekPos( ULONG nPos );
    TOOLS_DLLPRIVATE virtual void    FlushData();
    TOOLS_DLLPRIVATE virtual void    SetSize( ULONG nSize );

public:
                    SvCacheStream( ULONG nMaxMemSize = 0 );
                    SvCacheStream( const String &rFileName,
                                   ULONG nExpectedSize = 0,
                                   ULONG nMaxMemSize = 0 );
                    ~SvCacheStream();

    void            SetFilenameHdl( const Link& rLink);
    const Link&     GetFilenameHdl() const;
    void            SetFilename( const String& rFN )
                 { aFileName = rFN; } // darf nur vom FilenameHdl gerufen werden!
    const String&   GetFilename() const { return aFileName; }

    void            SwapOut();
    const void*     GetBuffer();
    ULONG           GetSize();

    BOOL            IsPersistent() { return bPersistent != 0; }
    void            SetPersistence( BOOL b = TRUE ) { bPersistent = b; }
    void            SetSwapStream( SvStream *p )
                 { pSwapStream = p; } // darf nur vom FilenameHdl gerufen werden!
};

#endif
