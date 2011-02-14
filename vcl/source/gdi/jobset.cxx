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
#include "precompiled_vcl.hxx"

#include <tools/debug.hxx>
#include <tools/stream.hxx>

#include <rtl/alloc.h>

#include <vcl/jobset.hxx>

#include <jobset.h>

// =======================================================================

DBG_NAME( JobSetup )

#define JOBSET_FILEFORMAT2      3780
#define JOBSET_FILE364_SYSTEM   ((sal_uInt16)0xFFFF)
#define JOBSET_FILE605_SYSTEM   ((sal_uInt16)0xFFFE)

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
    meDuplexMode        = DUPLEX_UNKNOWN;
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
    meDuplexMode        = rJobSetup.meDuplexMode;
    mnPaperBin          = rJobSetup.mnPaperBin;
    mePaperFormat       = rJobSetup.mePaperFormat;
    mnPaperWidth        = rJobSetup.mnPaperWidth;
    mnPaperHeight       = rJobSetup.mnPaperHeight;
    mnDriverDataLen     = rJobSetup.mnDriverDataLen;
    if ( rJobSetup.mpDriverData )
    {
        mpDriverData = (sal_uInt8*)rtl_allocateMemory( mnDriverDataLen );
        memcpy( mpDriverData, rJobSetup.mpDriverData, mnDriverDataLen );
    }
    else
        mpDriverData = NULL;
    maValueMap          = rJobSetup.maValueMap;
}

// -----------------------------------------------------------------------

ImplJobSetup::~ImplJobSetup()
{
    rtl_freeMemory( mpDriverData );
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

String JobSetup::GetValue( const String& rKey ) const
{
    if( mpData )
    {
        ::std::hash_map< ::rtl::OUString, ::rtl::OUString, ::rtl::OUStringHash >::const_iterator it;
        it = mpData->maValueMap.find( rKey );
        return it != mpData->maValueMap.end() ? String( it->second ) : String();
    }
    return String();
}

// -----------------------------------------------------------------------

void JobSetup::SetValue( const String& rKey, const String& rValue )
{
    if( ! mpData )
        mpData = new ImplJobSetup();

    mpData->maValueMap[ rKey ] = rValue;
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

sal_Bool JobSetup::operator==( const JobSetup& rJobSetup ) const
{
    DBG_CHKTHIS( JobSetup, NULL );
    DBG_CHKOBJ( &rJobSetup, JobSetup, NULL );

    if ( mpData == rJobSetup.mpData )
        return sal_True;

    if ( !mpData || !rJobSetup.mpData )
        return sal_False;

    ImplJobSetup* pData1 = mpData;
    ImplJobSetup* pData2 = rJobSetup.mpData;
    if ( (pData1->mnSystem          == pData2->mnSystem)                &&
         (pData1->maPrinterName     == pData2->maPrinterName)           &&
         (pData1->maDriver          == pData2->maDriver)                &&
         (pData1->meOrientation     == pData2->meOrientation)           &&
         (pData1->meDuplexMode      == pData2->meDuplexMode)            &&
         (pData1->mnPaperBin        == pData2->mnPaperBin)              &&
         (pData1->mePaperFormat     == pData2->mePaperFormat)           &&
         (pData1->mnPaperWidth      == pData2->mnPaperWidth)            &&
         (pData1->mnPaperHeight     == pData2->mnPaperHeight)           &&
         (pData1->mnDriverDataLen   == pData2->mnDriverDataLen)         &&
         (memcmp( pData1->mpDriverData, pData2->mpDriverData, pData1->mnDriverDataLen ) == 0)                                                           &&
         (pData1->maValueMap        == pData2->maValueMap)
         )
        return sal_True;

    return sal_False;
}

// -----------------------------------------------------------------------

SvStream& operator>>( SvStream& rIStream, JobSetup& rJobSetup )
{
    DBG_ASSERTWARNING( rIStream.GetVersion(), "JobSetup::>> - Solar-Version not set on rOStream" );

    // Zur Zeit haben wir noch kein neues FileFormat
//    if ( rIStream.GetVersion() < JOBSET_FILEFORMAT2 )
    {
        sal_uInt16 nLen;
        sal_uInt16 nSystem;
        sal_Size nFirstPos = rIStream.Tell();
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

            rtl_TextEncoding aStreamEncoding = RTL_TEXTENCODING_UTF8;
            if( nSystem == JOBSET_FILE364_SYSTEM )
                aStreamEncoding = rIStream.GetStreamCharSet();

            rJobSetup.mpData = new ImplJobSetup;
            ImplJobSetup* pJobData = rJobSetup.mpData;
            pJobData->maPrinterName = UniString( pData->cPrinterName, aStreamEncoding );
            pJobData->maDriver      = UniString( pData->cDriverName, aStreamEncoding );

            // Sind es unsere neuen JobSetup-Daten?
            if ( nSystem == JOBSET_FILE364_SYSTEM ||
                 nSystem == JOBSET_FILE605_SYSTEM )
            {
                Impl364JobSetupData* pOldJobData    = (Impl364JobSetupData*)(pTempBuf + sizeof( ImplOldJobSetupData ));
                sal_uInt16 nOldJobDataSize              = SVBT16ToShort( pOldJobData->nSize );
                pJobData->mnSystem                  = SVBT16ToShort( pOldJobData->nSystem );
                pJobData->mnDriverDataLen           = SVBT32ToUInt32( pOldJobData->nDriverDataLen );
                pJobData->meOrientation             = (Orientation)SVBT16ToShort( pOldJobData->nOrientation );
                pJobData->meDuplexMode              = DUPLEX_UNKNOWN;
                pJobData->mnPaperBin                = SVBT16ToShort( pOldJobData->nPaperBin );
                pJobData->mePaperFormat             = (Paper)SVBT16ToShort( pOldJobData->nPaperFormat );
                pJobData->mnPaperWidth              = (long)SVBT32ToUInt32( pOldJobData->nPaperWidth );
                pJobData->mnPaperHeight             = (long)SVBT32ToUInt32( pOldJobData->nPaperHeight );
                if ( pJobData->mnDriverDataLen )
                {
                    sal_uInt8* pDriverData = ((sal_uInt8*)pOldJobData) + nOldJobDataSize;
                    pJobData->mpDriverData = (sal_uInt8*)rtl_allocateMemory( pJobData->mnDriverDataLen );
                    memcpy( pJobData->mpDriverData, pDriverData, pJobData->mnDriverDataLen );
                }
                if( nSystem == JOBSET_FILE605_SYSTEM )
                {
                    rIStream.Seek( nFirstPos + sizeof( ImplOldJobSetupData ) + 4 + sizeof( Impl364JobSetupData ) + pJobData->mnDriverDataLen );
                    while( rIStream.Tell() < nFirstPos + nLen )
                    {
                        String aKey, aValue;
                        rIStream.ReadByteString( aKey, RTL_TEXTENCODING_UTF8 );
                        rIStream.ReadByteString( aValue, RTL_TEXTENCODING_UTF8 );
                        if( aKey.EqualsAscii( "COMPAT_DUPLEX_MODE" ) )
                        {
                            if( aValue.EqualsAscii( "DUPLEX_UNKNOWN" ) )
                                pJobData->meDuplexMode = DUPLEX_UNKNOWN;
                            else if( aValue.EqualsAscii( "DUPLEX_OFF" ) )
                                pJobData->meDuplexMode = DUPLEX_OFF;
                            else if( aValue.EqualsAscii( "DUPLEX_SHORTEDGE" ) )
                                pJobData->meDuplexMode = DUPLEX_SHORTEDGE;
                            else if( aValue.EqualsAscii( "DUPLEX_LONGEDGE" ) )
                                pJobData->meDuplexMode = DUPLEX_LONGEDGE;
                        }
                        else
                            pJobData->maValueMap[ aKey ] = aValue;
                    }
                    DBG_ASSERT( rIStream.Tell() == nFirstPos+nLen, "corrupted job setup" );
                    // ensure correct stream position
                    rIStream.Seek( nFirstPos + nLen );
                }
            }
        }
        delete[] pTempBuf;
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
        sal_uInt16 nLen = 0;
        if ( !rJobSetup.mpData )
            rOStream << nLen;
        else
        {
            sal_uInt16 nSystem = JOBSET_FILE605_SYSTEM;

            const ImplJobSetup* pJobData = rJobSetup.ImplGetConstData();
            Impl364JobSetupData aOldJobData;
            sal_uInt16              nOldJobDataSize = sizeof( aOldJobData );
            ShortToSVBT16( nOldJobDataSize, aOldJobData.nSize );
            ShortToSVBT16( pJobData->mnSystem, aOldJobData.nSystem );
            UInt32ToSVBT32( pJobData->mnDriverDataLen, aOldJobData.nDriverDataLen );
            ShortToSVBT16( (sal_uInt16)(pJobData->meOrientation), aOldJobData.nOrientation );
            ShortToSVBT16( pJobData->mnPaperBin, aOldJobData.nPaperBin );
            ShortToSVBT16( (sal_uInt16)(pJobData->mePaperFormat), aOldJobData.nPaperFormat );
            UInt32ToSVBT32( (sal_uLong)(pJobData->mnPaperWidth), aOldJobData.nPaperWidth );
            UInt32ToSVBT32( (sal_uLong)(pJobData->mnPaperHeight), aOldJobData.nPaperHeight );

            ImplOldJobSetupData aOldData;
            memset( &aOldData, 0, sizeof( aOldData ) );
            ByteString aPrnByteName( rJobSetup.GetPrinterName(), RTL_TEXTENCODING_UTF8 );
            strncpy( aOldData.cPrinterName, aPrnByteName.GetBuffer(), 63 );
            ByteString aDriverByteName( rJobSetup.GetDriverName(), RTL_TEXTENCODING_UTF8 );
            strncpy( aOldData.cDriverName, aDriverByteName.GetBuffer(), 31 );
//          nLen = sizeof( aOldData ) + 4 + nOldJobDataSize + pJobData->mnDriverDataLen;
            int nPos = rOStream.Tell();
            rOStream << nLen;
            rOStream << nSystem;
            rOStream.Write( (char*)&aOldData, sizeof( aOldData ) );
            rOStream.Write( (char*)&aOldJobData, nOldJobDataSize );
            rOStream.Write( (char*)pJobData->mpDriverData, pJobData->mnDriverDataLen );
            ::std::hash_map< ::rtl::OUString, ::rtl::OUString, ::rtl::OUStringHash >::const_iterator it;
            for( it = pJobData->maValueMap.begin(); it != pJobData->maValueMap.end(); ++it )
            {
                rOStream.WriteByteString( it->first, RTL_TEXTENCODING_UTF8 );
                rOStream.WriteByteString( it->second, RTL_TEXTENCODING_UTF8 );
            }
            rOStream.WriteByteString( "COMPAT_DUPLEX_MODE" ) ;
            switch( pJobData->meDuplexMode )
            {
            case DUPLEX_UNKNOWN: rOStream.WriteByteString( "DUPLEX_UNKNOWN" );break;
            case DUPLEX_OFF: rOStream.WriteByteString( "DUPLEX_OFF" );break;
            case DUPLEX_SHORTEDGE: rOStream.WriteByteString( "DUPLEX_SHORTEDGE" );break;
            case DUPLEX_LONGEDGE: rOStream.WriteByteString( "DUPLEX_LONGEDGE" );break;
            }
            nLen = sal::static_int_cast<sal_uInt16>(rOStream.Tell() - nPos);
            rOStream.Seek( nPos );
            rOStream << nLen;
            rOStream.Seek( nPos + nLen );
        }
    }
/*
    else
    {
    }
*/

    return rOStream;
}
