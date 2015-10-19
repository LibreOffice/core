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

#include <rtl/alloc.h>
#include <rtl/ustring.hxx>
#include <tools/debug.hxx>
#include <tools/stream.hxx>
#include <vcl/jobset.hxx>

#include <jobset.h>
#include <memory>

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
    mbPapersizeFromSetup = false;
}

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
        mpDriverData = static_cast<sal_uInt8*>(rtl_allocateMemory( mnDriverDataLen ));
        memcpy( mpDriverData, rJobSetup.mpDriverData, mnDriverDataLen );
    }
    else
        mpDriverData = NULL;
    mbPapersizeFromSetup = rJobSetup.mbPapersizeFromSetup;
    maValueMap          = rJobSetup.maValueMap;
}

ImplJobSetup::~ImplJobSetup()
{
    rtl_freeMemory( mpDriverData );
}

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

ImplJobSetup* JobSetup::ImplGetConstData()
{
    if ( !mpData )
        mpData = new ImplJobSetup;
    return mpData;
}

const ImplJobSetup* JobSetup::ImplGetConstData() const
{
    if ( !mpData )
        const_cast<JobSetup*>(this)->mpData = new ImplJobSetup;
    return mpData;
}

JobSetup::JobSetup()
{

    mpData = NULL;
}

JobSetup::JobSetup( const JobSetup& rJobSetup )
{
    DBG_ASSERT( !rJobSetup.mpData || (rJobSetup.mpData->mnRefCount < 0xFFFE), "JobSetup: RefCount overflow" );

    mpData = rJobSetup.mpData;
    if ( mpData )
        mpData->mnRefCount++;
}

JobSetup::~JobSetup()
{

    if ( mpData )
    {
        if ( mpData->mnRefCount == 1 )
            delete mpData;
        else
            mpData->mnRefCount--;
    }
}

OUString JobSetup::GetPrinterName() const
{
    if ( mpData )
        return mpData->maPrinterName;
    else
        return OUString();
}

OUString JobSetup::GetDriverName() const
{
    if ( mpData )
        return mpData->maDriver;
    else
        return OUString();
}

JobSetup& JobSetup::operator=( const JobSetup& rJobSetup )
{
    DBG_ASSERT( !rJobSetup.mpData || (rJobSetup.mpData->mnRefCount) < 0xFFFE, "JobSetup: RefCount overflow" );

    // Increment refcount first, so that we can assign to ourselves
    if ( rJobSetup.mpData )
        rJobSetup.mpData->mnRefCount++;

    // If it's not static ImpData and the last reference, delete it, else
    // decrement refcount
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

bool JobSetup::operator==( const JobSetup& rJobSetup ) const
{

    if ( mpData == rJobSetup.mpData )
        return true;

    if ( !mpData || !rJobSetup.mpData )
        return false;

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
        return true;

    return false;
}

SvStream& ReadJobSetup( SvStream& rIStream, JobSetup& rJobSetup )
{
    SAL_WARN_IF( !rIStream.GetVersion(), "vcl", "JobSetup::>> - Solar-Version not set on rOStream" );

    {
        sal_uInt16 nLen = 0;
        rIStream.ReadUInt16( nLen );
        if (nLen <= 4)
            return rIStream;

        sal_uInt16 nSystem = 0;
        rIStream.ReadUInt16( nSystem );
        const size_t nRead = nLen - sizeof(nLen) - sizeof(nSystem);
        if (nRead > rIStream.remainingSize())
        {
            SAL_WARN("vcl", "Parsing error: " << rIStream.remainingSize() <<
                     " max possible entries, but " << nRead << " claimed, truncating");
            return rIStream;
        }
        sal_Size nFirstPos = rIStream.Tell();
        std::unique_ptr<char[]> pTempBuf(new char[nRead]);
        rIStream.Read(pTempBuf.get(), nRead);
        if (nRead >= sizeof(ImplOldJobSetupData))
        {
            ImplOldJobSetupData* pData = reinterpret_cast<ImplOldJobSetupData*>(pTempBuf.get());
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
            pJobData->maPrinterName = OStringToOUString(pData->cPrinterName, aStreamEncoding);
            pJobData->maDriver = OStringToOUString(pData->cDriverName, aStreamEncoding);

            // Are these our new JobSetup files?
            if ( nSystem == JOBSET_FILE364_SYSTEM ||
                 nSystem == JOBSET_FILE605_SYSTEM )
            {
                Impl364JobSetupData* pOldJobData    = reinterpret_cast<Impl364JobSetupData*>(pTempBuf.get() + sizeof( ImplOldJobSetupData ));
                sal_uInt16 nOldJobDataSize          = SVBT16ToShort( pOldJobData->nSize );
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
                    sal_uInt8* pDriverData = reinterpret_cast<sal_uInt8*>(pOldJobData) + nOldJobDataSize;
                    pJobData->mpDriverData = static_cast<sal_uInt8*>(rtl_allocateMemory( pJobData->mnDriverDataLen ));
                    memcpy( pJobData->mpDriverData, pDriverData, pJobData->mnDriverDataLen );
                }
                if( nSystem == JOBSET_FILE605_SYSTEM )
                {
                    rIStream.Seek( nFirstPos + sizeof( ImplOldJobSetupData ) + sizeof( Impl364JobSetupData ) + pJobData->mnDriverDataLen );
                    while( rIStream.Tell() < nFirstPos + nRead )
                    {
                        OUString aKey = read_uInt16_lenPrefixed_uInt8s_ToOUString(rIStream, RTL_TEXTENCODING_UTF8);
                        OUString aValue = read_uInt16_lenPrefixed_uInt8s_ToOUString(rIStream, RTL_TEXTENCODING_UTF8);
                        if( aKey == "COMPAT_DUPLEX_MODE" )
                        {
                            if( aValue == "DUPLEX_UNKNOWN" )
                                pJobData->meDuplexMode = DUPLEX_UNKNOWN;
                            else if( aValue == "DUPLEX_OFF" )
                                pJobData->meDuplexMode = DUPLEX_OFF;
                            else if( aValue == "DUPLEX_SHORTEDGE" )
                                pJobData->meDuplexMode = DUPLEX_SHORTEDGE;
                            else if( aValue == "DUPLEX_LONGEDGE" )
                                pJobData->meDuplexMode = DUPLEX_LONGEDGE;
                        }
                        else
                            pJobData->maValueMap[ aKey ] = aValue;
                    }
                    DBG_ASSERT( rIStream.Tell() == nFirstPos+nRead, "corrupted job setup" );
                    // ensure correct stream position
                    rIStream.Seek(nFirstPos + nRead);
                }
            }
        }
    }

    return rIStream;
}

SvStream& WriteJobSetup( SvStream& rOStream, const JobSetup& rJobSetup )
{
    SAL_WARN_IF( !rOStream.GetVersion(), "vcl", "JobSetup::<< - Solar-Version not set on rOStream" );

    // We do not have a new FileFormat at this point in time
    // #define JOBSET_FILEFORMAT2      3780
    // if ( rOStream.GetVersion() < JOBSET_FILEFORMAT2 )
    {
        sal_uInt16 nLen = 0;
        if ( !rJobSetup.mpData )
            rOStream.WriteUInt16( nLen );
        else
        {
            sal_uInt16 nSystem = JOBSET_FILE605_SYSTEM;

            const ImplJobSetup* pJobData = rJobSetup.ImplGetConstData();
            Impl364JobSetupData aOldJobData;
            sal_uInt16 nOldJobDataSize = sizeof( aOldJobData );
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
            OString aPrnByteName(OUStringToOString(rJobSetup.GetPrinterName(), RTL_TEXTENCODING_UTF8));
            strncpy( aOldData.cPrinterName, aPrnByteName.getStr(), 63 );
            OString aDriverByteName(OUStringToOString(rJobSetup.GetDriverName(), RTL_TEXTENCODING_UTF8));
            strncpy( aOldData.cDriverName, aDriverByteName.getStr(), 31 );
//          nLen = sizeof( aOldData ) + 4 + nOldJobDataSize + pJobData->mnDriverDataLen;
            int nPos = rOStream.Tell();
            rOStream.WriteUInt16( nLen );
            rOStream.WriteUInt16( nSystem );
            rOStream.Write( &aOldData, sizeof( aOldData ) );
            rOStream.Write( &aOldJobData, nOldJobDataSize );
            rOStream.Write( pJobData->mpDriverData, pJobData->mnDriverDataLen );
            std::unordered_map< OUString, OUString, OUStringHash >::const_iterator it;
            for( it = pJobData->maValueMap.begin(); it != pJobData->maValueMap.end(); ++it )
            {
                write_uInt16_lenPrefixed_uInt8s_FromOUString(rOStream, it->first, RTL_TEXTENCODING_UTF8);
                write_uInt16_lenPrefixed_uInt8s_FromOUString(rOStream, it->second, RTL_TEXTENCODING_UTF8);
            }
            write_uInt16_lenPrefixed_uInt8s_FromOString(rOStream, "COMPAT_DUPLEX_MODE");
            switch( pJobData->meDuplexMode )
            {
                case DUPLEX_UNKNOWN:
                    write_uInt16_lenPrefixed_uInt8s_FromOString(rOStream, "DUPLEX_UNKNOWN");
                    break;
                case DUPLEX_OFF:
                    write_uInt16_lenPrefixed_uInt8s_FromOString(rOStream, "DUPLEX_OFF");
                    break;
                case DUPLEX_SHORTEDGE:
                    write_uInt16_lenPrefixed_uInt8s_FromOString(rOStream, "DUPLEX_SHORTEDGE");
                    break;
                case DUPLEX_LONGEDGE:
                    write_uInt16_lenPrefixed_uInt8s_FromOString(rOStream, "DUPLEX_LONGEDGE");
                    break;
            }
            nLen = sal::static_int_cast<sal_uInt16>(rOStream.Tell() - nPos);
            rOStream.Seek( nPos );
            rOStream.WriteUInt16( nLen );
            rOStream.Seek( nPos + nLen );
        }
    }

    return rOStream;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
