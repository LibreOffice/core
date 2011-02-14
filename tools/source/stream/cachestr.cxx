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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_tools.hxx"

#include <tools/debug.hxx>
#include <tools/stream.hxx>
#include <tools/cachestr.hxx>
#include <tools/tempfile.hxx>

/*************************************************************************
|*
|*    SvCacheStream::SvCacheStream()
|*
|*    Beschreibung      STREAM.SDW
|*    Ersterstellung    OV 27.09.94
|*    Letzte Aenderung  OV 27.09.94
|*
*************************************************************************/

SvCacheStream::SvCacheStream( sal_uIntPtr nMaxMemSize )
{
    if( !nMaxMemSize )
        nMaxMemSize = 20480;
    SvStream::bIsWritable = sal_True;
    nMaxSize        = nMaxMemSize;
    bPersistent     = sal_False;
    pSwapStream     = 0;
    pCurrentStream  = new SvMemoryStream( nMaxMemSize );
    pTempFile       = 0;
}

/*************************************************************************
|*
|*    SvCacheStream::SvCacheStream()
|*
|*    Beschreibung      STREAM.SDW
|*    Ersterstellung    OV 27.09.94
|*    Letzte Aenderung  OV 27.09.94
|*
*************************************************************************/

SvCacheStream::SvCacheStream( const String &rFileName,
                              sal_uIntPtr nExpectedSize,
                              sal_uIntPtr nMaxMemSize )
{
    if( !nMaxMemSize )
        nMaxMemSize = 20480;

    if( nExpectedSize > nMaxMemSize )
        nExpectedSize = nMaxMemSize; // oder gleich in File schreiben
    else if( !nExpectedSize )
        nExpectedSize = 4096;

    SvStream::bIsWritable = sal_True;
    nMaxSize        = nMaxMemSize;
    bPersistent     = sal_True;
    aFileName       = rFileName;
    pSwapStream     = 0;
    pCurrentStream  = new SvMemoryStream( nExpectedSize );
    pTempFile       = 0;
}

/*************************************************************************
|*
|*    SvCacheStream::~SvCacheStream()
|*
|*    Beschreibung      STREAM.SDW
|*    Ersterstellung    OV 27.09.94
|*    Letzte Aenderung  OV 27.09.94
|*
*************************************************************************/

SvCacheStream::~SvCacheStream()
{
       if( pCurrentStream != pSwapStream )
        delete pSwapStream;
    delete pCurrentStream;

    if( pSwapStream && !bPersistent && pTempFile )
    {
        // temporaeres File loeschen
        pTempFile->EnableKillingFile( sal_True );
    }

    delete pTempFile;
}

/*************************************************************************
|*
|*    SvCacheStream::SwapOut()
|*
|*    Beschreibung      STREAM.SDW
|*    Ersterstellung    OV 27.09.94
|*    Letzte Aenderung  OV 27.09.94
|*
*************************************************************************/

void SvCacheStream::SwapOut()
{
    if( pCurrentStream != pSwapStream )
    {
        if( !pSwapStream && !aFileName.Len() )
        {
            if (aFilenameLinkHdl.IsSet())
            {
                // pSwapStream wird zum Schutz gegen Reentranz genutzt
                pSwapStream = pCurrentStream;
                Link aLink( aFilenameLinkHdl );
                aFilenameLinkHdl = Link();
                aLink.Call(this);
                // pSwapStream nur zuruecksetzen, wenn nicht ueber
                // SetSwapStream geaendert
                if( pSwapStream == pCurrentStream ) pSwapStream = 0;
            }
            else
            {
                pTempFile = new TempFile;
                aFileName = pTempFile->GetName();
            }
        }

        sal_uIntPtr nPos = pCurrentStream->Tell();
        pCurrentStream->Seek( 0 );
        if( !pSwapStream )
            pSwapStream = new SvFileStream( aFileName, STREAM_READWRITE | STREAM_TRUNC );
        *pSwapStream << *pCurrentStream;
        pSwapStream->Flush();
        delete pCurrentStream;
        pCurrentStream = pSwapStream;
        pCurrentStream->Seek( nPos );
    }
}

/*************************************************************************
|*
|*    SvCacheStream::GetData()
|*
|*    Beschreibung      STREAM.SDW
|*    Ersterstellung    OV 27.09.94
|*    Letzte Aenderung  OV 27.09.94
|*
*************************************************************************/

sal_uIntPtr SvCacheStream::GetData( void* pData, sal_uIntPtr nSize )
{
    return pCurrentStream->Read( pData, nSize );
}

/*************************************************************************
|*
|*    SvCacheStream::PutData()
|*
|*    Beschreibung      STREAM.SDW
|*    Ersterstellung    OV 27.09.94
|*    Letzte Aenderung  OV 27.09.94
|*
*************************************************************************/

sal_uIntPtr SvCacheStream::PutData( const void* pData, sal_uIntPtr nSize )
{
    // lieber unnoetig auslagern als unnoetig umkopieren
    if( pCurrentStream != pSwapStream
        && pCurrentStream->Tell() + nSize > nMaxSize )
        SwapOut();
    return pCurrentStream->Write( pData, nSize );
}

/*************************************************************************
|*
|*    SvCacheStream::SeekPos()
|*
|*    Beschreibung      STREAM.SDW
|*    Ersterstellung    OV 27.09.94
|*    Letzte Aenderung  OV 27.09.94
|*
*************************************************************************/

sal_uIntPtr SvCacheStream::SeekPos( sal_uIntPtr nPos )
{
    return pCurrentStream->Seek( nPos );
}

/*************************************************************************
|*
|*    SvCacheStream::FlushData()
|*
|*    Beschreibung      STREAM.SDW
|*    Ersterstellung    OV 27.09.94
|*    Letzte Aenderung  OV 27.09.94
|*
*************************************************************************/

void SvCacheStream::FlushData()
{
    pCurrentStream->Flush();
    if( pCurrentStream != pSwapStream
        && ((SvMemoryStream*)pCurrentStream)->GetSize() > nMaxSize )
        SwapOut();
}

/*************************************************************************
|*
|*    SvCacheStream::GetStr()
|*
|*    Beschreibung      STREAM.SDW
|*    Ersterstellung    OV 27.09.94
|*    Letzte Aenderung  OV 27.09.94
|*
*************************************************************************/

const void* SvCacheStream::GetBuffer()
{
    Flush();
    if( pCurrentStream != pSwapStream )
        return ((SvMemoryStream*)pCurrentStream)->GetData();
    else
        return 0;
}

/*************************************************************************
|*
|*    SvCacheStream::SetSize()
|*
|*    Beschreibung      STREAM.SDW
|*    Ersterstellung    OV 27.09.94
|*    Letzte Aenderung  OV 27.09.94
|*
*************************************************************************/

void SvCacheStream::SetSize( sal_uIntPtr nSize )
{
    pCurrentStream->SetStreamSize( nSize );
}

/*************************************************************************
|*
|*    SvCacheStream::GetSize()
|*
|*    Beschreibung      STREAM.SDW
|*    Ersterstellung    OV 27.09.94
|*    Letzte Aenderung  OV 27.09.94
|*
*************************************************************************/

sal_uIntPtr SvCacheStream::GetSize()
{
    // ACHTUNG: SvMemoryStream::GetSize() gibt Groesse
    // des allozierten Buffers zurueck
    Flush();
    sal_uIntPtr nTemp = Tell();
    sal_uIntPtr nLength = Seek( STREAM_SEEK_TO_END );
    Seek( nTemp );
    return nLength;
}

void    SvCacheStream::SetFilenameHdl( const Link& rLink)
{
    aFilenameLinkHdl = rLink;
}

const Link& SvCacheStream::GetFilenameHdl() const
{
    return aFilenameLinkHdl;
}
