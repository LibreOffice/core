/*************************************************************************
 *
 *  $RCSfile: jobset.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:05:38 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#define _SV_JOBSET_CXX

#ifndef _DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _STREAM_HXX
#include <tools/stream.hxx>
#endif

#ifndef _SV_JOBSET_HXX
#include <jobset.hxx>
#endif
#ifndef _SV_JOBSET_H
#include <jobset.h>
#endif

// =======================================================================

DBG_NAME( JobSetup );

#define JOBSET_FILEFORMAT2      3780
#define JOBSET_FILE364_SYSTEM   ((USHORT)0xFFFF)

struct ImplOldJobSetupData
{
    char    cPrinterName[64];
    char    cDeviceName[32];
    char    cPortName[32];
    char    cDriverName[32];
};

struct Impl364JobSetupData
{
    SVBT16  nSize;
    SVBT16  nSystem;
    SVBT32  nDriverDataLen;
    SVBT16  nOrientation;
    SVBT16  nPaperBin;
    SVBT16  nPaperFormat;
    SVBT32  nPaperWidth;
    SVBT32  nPaperHeight;
};

// =======================================================================

ImplJobSetup::ImplJobSetup()
{
    mnRefCount          = 1;
    mnSystem            = 0;
    meOrientation       = ORIENTATION_PORTRAIT;
    mnPaperBin          = 0;
    mePaperFormat       = PAPER_USER;
    mnPaperWidth        = 0;
    mnPaperHeight       = 0;
    mnDriverDataLen     = 0;
    mpDriverData        = NULL;
}

// -----------------------------------------------------------------------

ImplJobSetup::ImplJobSetup( const ImplJobSetup& rJobSetup ) :
    maPrinterName( rJobSetup.maPrinterName ),
    maDriver( rJobSetup.maDriver )
{
    mnRefCount          = 1;
    mnSystem            = rJobSetup.mnSystem;
    meOrientation       = rJobSetup.meOrientation;
    mnPaperBin          = rJobSetup.mnPaperBin;
    mePaperFormat       = rJobSetup.mePaperFormat;
    mnPaperWidth        = rJobSetup.mnPaperWidth;
    mnPaperHeight       = rJobSetup.mnPaperHeight;
    mnDriverDataLen     = rJobSetup.mnDriverDataLen;
    if ( rJobSetup.mpDriverData )
    {
        mpDriverData = new BYTE[mnDriverDataLen];
        memcpy( mpDriverData, rJobSetup.mpDriverData, mnDriverDataLen );
    }
    else
        mpDriverData = NULL;
}

// -----------------------------------------------------------------------

ImplJobSetup::~ImplJobSetup()
{
    delete mpDriverData;
}

// =======================================================================

ImplJobSetup* JobSetup::ImplGetData()
{
    if ( !mpData )
        mpData = new ImplJobSetup;
    else if ( mpData->mnRefCount != 1 )
    {
        mpData->mnRefCount--;
        mpData = new ImplJobSetup( *mpData );
    }

    return mpData;
}

// -----------------------------------------------------------------------

ImplJobSetup* JobSetup::ImplGetConstData()
{
    if ( !mpData )
        mpData = new ImplJobSetup;
    return mpData;
}

// -----------------------------------------------------------------------

const ImplJobSetup* JobSetup::ImplGetConstData() const
{
    if ( !mpData )
        ((JobSetup*)this)->mpData = new ImplJobSetup;
    return mpData;
}

// =======================================================================

JobSetup::JobSetup()
{
    DBG_CTOR( JobSetup, NULL );

    mpData = NULL;
}

// -----------------------------------------------------------------------

JobSetup::JobSetup( const JobSetup& rJobSetup )
{
    DBG_CTOR( JobSetup, NULL );
    DBG_CHKOBJ( &rJobSetup, JobSetup, NULL );
    DBG_ASSERT( !rJobSetup.mpData || (rJobSetup.mpData->mnRefCount < 0xFFFE), "JobSetup: RefCount overflow" );

    mpData = rJobSetup.mpData;
    if ( mpData )
        mpData->mnRefCount++;
}

// -----------------------------------------------------------------------

JobSetup::~JobSetup()
{
    DBG_DTOR( JobSetup, NULL );

    if ( mpData )
    {
        if ( mpData->mnRefCount == 1 )
            delete mpData;
        else
            mpData->mnRefCount--;
    }
}

// -----------------------------------------------------------------------

XubString JobSetup::GetPrinterName() const
{
    if ( mpData )
        return mpData->maPrinterName;
    else
    {
        XubString aName;
        return aName;
    }
}

// -----------------------------------------------------------------------

XubString JobSetup::GetDriverName() const
{
    if ( mpData )
        return mpData->maDriver;
    else
    {
        XubString aDriver;
        return aDriver;
    }
}

// -----------------------------------------------------------------------

JobSetup& JobSetup::operator=( const JobSetup& rJobSetup )
{
    DBG_CHKTHIS( JobSetup, NULL );
    DBG_CHKOBJ( &rJobSetup, JobSetup, NULL );
    DBG_ASSERT( !rJobSetup.mpData || (rJobSetup.mpData->mnRefCount) < 0xFFFE, "JobSetup: RefCount overflow" );

    // Zuerst Referenzcounter erhoehen, damit man sich selbst zuweisen kann
    if ( rJobSetup.mpData )
        rJobSetup.mpData->mnRefCount++;

    // Wenn es keine statischen ImpDaten sind, dann loeschen, wenn es
    // die letzte Referenz ist, sonst Referenzcounter decrementieren
    if ( mpData )
    {
        if ( mpData->mnRefCount == 1 )
            delete mpData;
        else
            mpData->mnRefCount--;
    }

    mpData = rJobSetup.mpData;

    return *this;
}

// -----------------------------------------------------------------------

BOOL JobSetup::operator==( const JobSetup& rJobSetup ) const
{
    DBG_CHKTHIS( JobSetup, NULL );
    DBG_CHKOBJ( &rJobSetup, JobSetup, NULL );

    if ( mpData == rJobSetup.mpData )
        return TRUE;

    if ( !mpData || !rJobSetup.mpData )
        return FALSE;

    ImplJobSetup* pData1 = mpData;
    ImplJobSetup* pData2 = rJobSetup.mpData;
    if ( (pData1->mnSystem          == pData2->mnSystem)                &&
         (pData1->maPrinterName     == pData2->maPrinterName)           &&
         (pData1->maDriver          == pData2->maDriver)                &&
         (pData1->meOrientation     == pData2->meOrientation)           &&
         (pData1->mnPaperBin        == pData2->mnPaperBin)              &&
         (pData1->mePaperFormat     == pData2->mePaperFormat)           &&
         (pData1->mnPaperWidth      == pData2->mnPaperWidth)            &&
         (pData1->mnPaperHeight     == pData2->mnPaperHeight)           &&
         (pData1->mnDriverDataLen   == pData2->mnDriverDataLen)         &&
         (memcmp( pData1->mpDriverData, pData2->mpDriverData, pData1->mnDriverDataLen ) == 0) )
        return TRUE;

    return FALSE;
}

// -----------------------------------------------------------------------

SvStream& operator>>( SvStream& rIStream, JobSetup& rJobSetup )
{
    DBG_ASSERTWARNING( rIStream.GetVersion(), "JobSetup::>> - Solar-Version not set on rOStream" );

    // Zur Zeit haben wir noch kein neues FileFormat
//    if ( rIStream.GetVersion() < JOBSET_FILEFORMAT2 )
    {
        USHORT nLen;
        USHORT nSystem;
        rIStream >> nLen;
        if ( !nLen )
            return rIStream;
        rIStream >> nSystem;
        char* pTempBuf = new char[nLen];
        rIStream.Read( pTempBuf,  nLen - sizeof( nLen ) - sizeof( nSystem ) );
        if ( nLen >= sizeof(ImplOldJobSetupData)+4 )
        {
            ImplOldJobSetupData* pData = (ImplOldJobSetupData*)pTempBuf;
            if ( rJobSetup.mpData )
            {
                if ( rJobSetup.mpData->mnRefCount == 1 )
                    delete rJobSetup.mpData;
                else
                    rJobSetup.mpData->mnRefCount--;
            }
            rJobSetup.mpData = new ImplJobSetup;
            ImplJobSetup* pJobData = rJobSetup.mpData;
            pJobData->maPrinterName = UniString( pData->cPrinterName, RTL_TEXTENCODING_UTF8 );
            pJobData->maDriver      = UniString( pData->cDriverName, RTL_TEXTENCODING_UTF8 );

            // Sind es unsere neuen JobSetup-Daten?
            if ( nSystem == JOBSET_FILE364_SYSTEM )
            {
                Impl364JobSetupData* pOldJobData    = (Impl364JobSetupData*)(pTempBuf + sizeof( ImplOldJobSetupData ));
                USHORT nOldJobDataSize              = SVBT16ToShort( pOldJobData->nSize );
                pJobData->mnSystem                  = SVBT16ToShort( pOldJobData->nSystem );
                pJobData->mnDriverDataLen           = SVBT32ToLong( pOldJobData->nDriverDataLen );
                pJobData->meOrientation             = (Orientation)SVBT16ToShort( pOldJobData->nOrientation );
                pJobData->mnPaperBin                = SVBT16ToShort( pOldJobData->nPaperBin );
                pJobData->mePaperFormat             = (Paper)SVBT16ToShort( pOldJobData->nPaperFormat );
                pJobData->mnPaperWidth              = (long)SVBT32ToLong( pOldJobData->nPaperWidth );
                pJobData->mnPaperHeight             = (long)SVBT32ToLong( pOldJobData->nPaperHeight );
                if ( pJobData->mnDriverDataLen )
                {
                    BYTE* pDriverData = ((BYTE*)pOldJobData) + nOldJobDataSize;
                    pJobData->mpDriverData = new BYTE[pJobData->mnDriverDataLen];
                    memcpy( pJobData->mpDriverData, pDriverData, pJobData->mnDriverDataLen );
                }
            }
        }
        delete pTempBuf;
    }
/*
    else
    {
    }
*/

    return rIStream;
}

// -----------------------------------------------------------------------

SvStream& operator<<( SvStream& rOStream, const JobSetup& rJobSetup )
{
    DBG_ASSERTWARNING( rOStream.GetVersion(), "JobSetup::<< - Solar-Version not set on rOStream" );

    // Zur Zeit haben wir noch kein neues FileFormat
//    if ( rOStream.GetVersion() < JOBSET_FILEFORMAT2 )
    {
        USHORT nLen = 0;
        if ( !rJobSetup.mpData )
            rOStream << nLen;
        else
        {
            USHORT nSystem = JOBSET_FILE364_SYSTEM;

            const ImplJobSetup* pJobData = rJobSetup.ImplGetConstData();
            Impl364JobSetupData aOldJobData;
            USHORT              nOldJobDataSize = sizeof( aOldJobData );
            ShortToSVBT16( nOldJobDataSize, aOldJobData.nSize );
            ShortToSVBT16( pJobData->mnSystem, aOldJobData.nSystem );
            LongToSVBT32( pJobData->mnDriverDataLen, aOldJobData.nDriverDataLen );
            ShortToSVBT16( (USHORT)(pJobData->meOrientation), aOldJobData.nOrientation );
            ShortToSVBT16( pJobData->mnPaperBin, aOldJobData.nPaperBin );
            ShortToSVBT16( (USHORT)(pJobData->mePaperFormat), aOldJobData.nPaperFormat );
            LongToSVBT32( (ULONG)(pJobData->mnPaperWidth), aOldJobData.nPaperWidth );
            LongToSVBT32( (ULONG)(pJobData->mnPaperHeight), aOldJobData.nPaperHeight );

            ImplOldJobSetupData aOldData;
            memset( &aOldData, 0, sizeof( aOldData ) );
            ByteString aPrnByteName( rJobSetup.GetPrinterName(), RTL_TEXTENCODING_UTF8 );
            strncpy( aOldData.cPrinterName, aPrnByteName.GetBuffer(), 63 );
            ByteString aDriverByteName( rJobSetup.GetDriverName(), RTL_TEXTENCODING_UTF8 );
            strncpy( aOldData.cDriverName, aDriverByteName.GetBuffer(), 31 );
            nLen = sizeof( aOldData ) + 4 + nOldJobDataSize + pJobData->mnDriverDataLen;
            rOStream << nLen;
            rOStream << nSystem;
            rOStream.Write( (char*)&aOldData, sizeof( aOldData ) );
            rOStream.Write( (char*)&aOldJobData, nOldJobDataSize );
            rOStream.Write( (char*)pJobData->mpDriverData, pJobData->mnDriverDataLen );
        }
    }
/*
    else
    {
    }
*/

    return rOStream;
}
