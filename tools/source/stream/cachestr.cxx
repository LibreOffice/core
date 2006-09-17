/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cachestr.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 01:01:19 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_tools.hxx"

#include <debug.hxx>
#include <stream.hxx>
#include <cachestr.hxx>
#include <tempfile.hxx>

/*************************************************************************
|*
|*    SvCacheStream::SvCacheStream()
|*
|*    Beschreibung      STREAM.SDW
|*    Ersterstellung    OV 27.09.94
|*    Letzte Aenderung  OV 27.09.94
|*
*************************************************************************/

SvCacheStream::SvCacheStream( ULONG nMaxMemSize )
{
    if( !nMaxMemSize )
        nMaxMemSize = 20480;
    SvStream::bIsWritable = TRUE;
    nMaxSize        = nMaxMemSize;
    bPersistent     = FALSE;
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
                              ULONG nExpectedSize,
                              ULONG nMaxMemSize )
{
    if( !nMaxMemSize )
        nMaxMemSize = 20480;

    if( nExpectedSize > nMaxMemSize )
        nExpectedSize = nMaxMemSize; // oder gleich in File schreiben
    else if( !nExpectedSize )
        nExpectedSize = 4096;

    SvStream::bIsWritable = TRUE;
    nMaxSize        = nMaxMemSize;
    bPersistent     = TRUE;
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
        pTempFile->EnableKillingFile( TRUE );
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

        ULONG nPos = pCurrentStream->Tell();
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

ULONG SvCacheStream::GetData( void* pData, ULONG nSize )
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

ULONG SvCacheStream::PutData( const void* pData, ULONG nSize )
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

ULONG SvCacheStream::SeekPos( ULONG nPos )
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

void SvCacheStream::SetSize( ULONG nSize )
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

ULONG SvCacheStream::GetSize()
{
    // ACHTUNG: SvMemoryStream::GetSize() gibt Groesse
    // des allozierten Buffers zurueck
    Flush();
    ULONG nTemp = Tell();
    ULONG nLength = Seek( STREAM_SEEK_TO_END );
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
