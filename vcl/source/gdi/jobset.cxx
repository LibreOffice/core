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
#include <sal/log.hxx>
#include <tools/solar.h>
#include <tools/stream.hxx>
#include <vcl/jobset.hxx>
#include <jobset.h>
#include <memory>
#include <rtl/instance.hxx>

#define JOBSET_FILE364_SYSTEM   (sal_uInt16(0xFFFF))
#define JOBSET_FILE605_SYSTEM   (sal_uInt16(0xFFFE))

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
    mnSystem            = 0;
    meOrientation       = Orientation::Portrait;
    meDuplexMode        = DuplexMode::Unknown;
    mnPaperBin          = 0;
    mePaperFormat       = PAPER_USER;
    mnPaperWidth        = 0;
    mnPaperHeight       = 0;
    mnDriverDataLen     = 0;
    mpDriverData        = nullptr;
    mbPapersizeFromSetup = false;
    meSetupMode         = PrinterSetupMode::DocumentGlobal;
}

ImplJobSetup::ImplJobSetup( const ImplJobSetup& rJobSetup ) :
    mnSystem( rJobSetup.GetSystem() ),
    maPrinterName( rJobSetup.GetPrinterName() ),
    maDriver( rJobSetup.GetDriver() ),
    meOrientation( rJobSetup.GetOrientation() ),
    meDuplexMode( rJobSetup.GetDuplexMode() ),
    mnPaperBin( rJobSetup.GetPaperBin() ),
    mePaperFormat( rJobSetup.GetPaperFormat() ),
    mnPaperWidth( rJobSetup.GetPaperWidth() ),
    mnPaperHeight( rJobSetup.GetPaperHeight() ),
    mnDriverDataLen( rJobSetup.GetDriverDataLen() ),
    mbPapersizeFromSetup( rJobSetup.GetPapersizeFromSetup() ),
    meSetupMode( rJobSetup.GetPrinterSetupMode() ),
    maValueMap( rJobSetup.GetValueMap() )
 {
    if ( rJobSetup.GetDriverData() )
    {
        mpDriverData = static_cast<sal_uInt8*>(std::malloc( mnDriverDataLen ));
        memcpy( mpDriverData, rJobSetup.GetDriverData(), mnDriverDataLen );
    }
    else
        mpDriverData = nullptr;
}

ImplJobSetup::~ImplJobSetup()
{
    std::free( mpDriverData );
}

void ImplJobSetup::SetSystem(sal_uInt16 nSystem)
{
    mnSystem = nSystem;
}

void ImplJobSetup::SetPrinterName(const OUString& rPrinterName)
{
    maPrinterName = rPrinterName;
}

void ImplJobSetup::SetDriver(const OUString& rDriver)
{
    maDriver = rDriver;
}

void ImplJobSetup::SetOrientation(Orientation eOrientation)
{
    meOrientation = eOrientation;
}

void ImplJobSetup::SetDuplexMode(DuplexMode eDuplexMode)
{
    meDuplexMode = eDuplexMode;
}

void ImplJobSetup::SetPaperBin(sal_uInt16 nPaperBin)
{
    mnPaperBin = nPaperBin;
}

void ImplJobSetup::SetPaperFormat(Paper ePaperFormat)
{
    mePaperFormat = ePaperFormat;
}

void ImplJobSetup::SetPaperWidth(long nPaperWidth)
{
    mnPaperWidth = nPaperWidth;
}

void ImplJobSetup::SetPaperHeight(long nPaperHeight)
{
    mnPaperHeight = nPaperHeight;
}

void ImplJobSetup::SetDriverDataLen(sal_uInt32 nDriverDataLen)
{
    mnDriverDataLen = nDriverDataLen;
}

void ImplJobSetup::SetDriverData(sal_uInt8* pDriverData)
{
    mpDriverData = pDriverData;
}

void ImplJobSetup::SetPapersizeFromSetup(bool bPapersizeFromSetup)
{
    mbPapersizeFromSetup = bPapersizeFromSetup;
}

void ImplJobSetup::SetPrinterSetupMode(PrinterSetupMode eMode)
{
    meSetupMode = eMode;
}

void ImplJobSetup::SetValueMap( const OUString& rKey, const OUString& rValue )
{
    maValueMap [ rKey ] = rValue;
}

JobSetup& JobSetup::operator=( const JobSetup& ) = default;

JobSetup& JobSetup::operator=( JobSetup&& ) = default;

bool ImplJobSetup::operator==( const ImplJobSetup& rImplJobSetup ) const
{
    return mnSystem          == rImplJobSetup.mnSystem        &&
         maPrinterName     == rImplJobSetup.maPrinterName   &&
         maDriver          == rImplJobSetup.maDriver        &&
         meOrientation     == rImplJobSetup.meOrientation   &&
         meDuplexMode      == rImplJobSetup.meDuplexMode    &&
         mnPaperBin        == rImplJobSetup.mnPaperBin      &&
         mePaperFormat     == rImplJobSetup.mePaperFormat   &&
         mnPaperWidth      == rImplJobSetup.mnPaperWidth    &&
         mnPaperHeight     == rImplJobSetup.mnPaperHeight   &&
         mbPapersizeFromSetup == rImplJobSetup.mbPapersizeFromSetup &&
         mnDriverDataLen   == rImplJobSetup.mnDriverDataLen &&
         maValueMap        == rImplJobSetup.maValueMap      &&
         memcmp( mpDriverData, rImplJobSetup.mpDriverData, mnDriverDataLen ) == 0;
}

namespace
{
    struct theGlobalDefault :
        public rtl::Static< JobSetup::ImplType, theGlobalDefault > {};
}

JobSetup::JobSetup() : mpData(theGlobalDefault::get())
{
}

JobSetup::JobSetup( const JobSetup& ) = default;

JobSetup::~JobSetup() = default;

bool JobSetup::operator==( const JobSetup& rJobSetup ) const
{
    return mpData == rJobSetup.mpData;
}

const ImplJobSetup& JobSetup::ImplGetConstData() const
{
    return *mpData;
}

ImplJobSetup& JobSetup::ImplGetData()
{
    return *mpData;
}

OUString const & JobSetup::GetPrinterName() const
{
    return mpData->GetPrinterName();
}

bool JobSetup::IsDefault() const
{
    return mpData.same_object(theGlobalDefault::get());
}

SvStream& ReadJobSetup( SvStream& rIStream, JobSetup& rJobSetup )
{
    {
        sal_uInt16 nLen = 0;
        rIStream.ReadUInt16( nLen );
        if (nLen <= 4)
            return rIStream;

        sal_uInt16 nSystem = 0;
        rIStream.ReadUInt16( nSystem );
        size_t nRead = nLen - sizeof(nLen) - sizeof(nSystem);
        if (nRead > rIStream.remainingSize())
        {
            SAL_WARN("vcl", "Parsing error: " << rIStream.remainingSize() <<
                     " max possible entries, but " << nRead << " claimed, truncating");
            return rIStream;
        }
        sal_uInt64 const nFirstPos = rIStream.Tell();
        std::unique_ptr<char[]> pTempBuf(new char[nRead]);
        nRead = rIStream.ReadBytes(pTempBuf.get(), nRead);
        if (nRead >= sizeof(ImplOldJobSetupData))
        {
            ImplOldJobSetupData* pData = reinterpret_cast<ImplOldJobSetupData*>(pTempBuf.get());

            rtl_TextEncoding aStreamEncoding = RTL_TEXTENCODING_UTF8;
            if( nSystem == JOBSET_FILE364_SYSTEM )
                aStreamEncoding = rIStream.GetStreamCharSet();

            ImplJobSetup& rJobData = rJobSetup.ImplGetData();

            pData->cPrinterName[SAL_N_ELEMENTS(pData->cPrinterName) - 1] = 0;
            rJobData.SetPrinterName( OStringToOUString(pData->cPrinterName, aStreamEncoding) );
            pData->cDriverName[SAL_N_ELEMENTS(pData->cDriverName) - 1] = 0;
            rJobData.SetDriver( OStringToOUString(pData->cDriverName, aStreamEncoding) );

            // Are these our new JobSetup files?
            if ( nSystem == JOBSET_FILE364_SYSTEM ||
                 nSystem == JOBSET_FILE605_SYSTEM )
            {
                Impl364JobSetupData* pOldJobData    = reinterpret_cast<Impl364JobSetupData*>(pTempBuf.get() + sizeof( ImplOldJobSetupData ));
                sal_uInt16 nOldJobDataSize          = SVBT16ToUInt16( pOldJobData->nSize );
                rJobData.SetSystem( SVBT16ToUInt16( pOldJobData->nSystem ) );
                rJobData.SetDriverDataLen( SVBT32ToUInt32( pOldJobData->nDriverDataLen ) );
                rJobData.SetOrientation( static_cast<Orientation>(SVBT16ToUInt16( pOldJobData->nOrientation )) );
                rJobData.SetDuplexMode( DuplexMode::Unknown );
                rJobData.SetPaperBin( SVBT16ToUInt16( pOldJobData->nPaperBin ) );
                rJobData.SetPaperFormat( static_cast<Paper>(SVBT16ToUInt16( pOldJobData->nPaperFormat )) );
                rJobData.SetPaperWidth( static_cast<long>(SVBT32ToUInt32( pOldJobData->nPaperWidth )) );
                rJobData.SetPaperHeight( static_cast<long>(SVBT32ToUInt32( pOldJobData->nPaperHeight )) );
                if ( rJobData.GetDriverDataLen() )
                {
                    const char* pDriverData = reinterpret_cast<const char*>(pOldJobData) + nOldJobDataSize;
                    const char* pDriverDataEnd = pDriverData + rJobData.GetDriverDataLen();
                    if (pDriverDataEnd > pTempBuf.get() + nRead)
                    {
                        SAL_WARN("vcl", "corrupted job setup");
                    }
                    else
                    {
                        sal_uInt8* pNewDriverData = static_cast<sal_uInt8*>(
                            std::malloc( rJobData.GetDriverDataLen() ));
                        memcpy( pNewDriverData, pDriverData, rJobData.GetDriverDataLen() );
                        rJobData.SetDriverData( pNewDriverData );
                    }
                }
                if( nSystem == JOBSET_FILE605_SYSTEM )
                {
                    rIStream.Seek( nFirstPos + sizeof( ImplOldJobSetupData ) +
                        sizeof( Impl364JobSetupData ) + rJobData.GetDriverDataLen() );
                    while( rIStream.Tell() < nFirstPos + nRead )
                    {
                        OUString aKey = read_uInt16_lenPrefixed_uInt8s_ToOUString(rIStream, RTL_TEXTENCODING_UTF8);
                        OUString aValue = read_uInt16_lenPrefixed_uInt8s_ToOUString(rIStream, RTL_TEXTENCODING_UTF8);
                        if( aKey == "COMPAT_DUPLEX_MODE" )
                        {
                            if( aValue == "DuplexMode::Unknown" )
                                rJobData.SetDuplexMode( DuplexMode::Unknown );
                            else if( aValue == "DuplexMode::Off" )
                                rJobData.SetDuplexMode( DuplexMode::Off );
                            else if( aValue == "DuplexMode::ShortEdge" )
                                rJobData.SetDuplexMode( DuplexMode::ShortEdge );
                            else if( aValue == "DuplexMode::LongEdge" )
                                rJobData.SetDuplexMode( DuplexMode::LongEdge );
                        }
                        else
                            rJobData.SetValueMap(aKey, aValue);
                    }
                    SAL_WARN_IF( rIStream.Tell() != nFirstPos+nRead, "vcl", "corrupted job setup" );
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
    {
        sal_uInt16 nLen = 0;
        if ( rJobSetup.IsDefault() )
            rOStream.WriteUInt16( nLen );
        else
        {
            const ImplJobSetup& rJobData = rJobSetup.ImplGetConstData();
            Impl364JobSetupData aOldJobData;
            sal_uInt16 nOldJobDataSize = sizeof( aOldJobData );
            ShortToSVBT16( nOldJobDataSize, aOldJobData.nSize );
            ShortToSVBT16( rJobData.GetSystem(), aOldJobData.nSystem );
            UInt32ToSVBT32( rJobData.GetDriverDataLen(), aOldJobData.nDriverDataLen );
            ShortToSVBT16( static_cast<sal_uInt16>(rJobData.GetOrientation()), aOldJobData.nOrientation );
            ShortToSVBT16( rJobData.GetPaperBin(), aOldJobData.nPaperBin );
            ShortToSVBT16( static_cast<sal_uInt16>(rJobData.GetPaperFormat()), aOldJobData.nPaperFormat );
            UInt32ToSVBT32( static_cast<sal_uLong>(rJobData.GetPaperWidth()), aOldJobData.nPaperWidth );
            UInt32ToSVBT32( static_cast<sal_uLong>(rJobData.GetPaperHeight()), aOldJobData.nPaperHeight );

            ImplOldJobSetupData aOldData;
            memset( &aOldData, 0, sizeof( aOldData ) );
            OString aPrnByteName(OUStringToOString(rJobData.GetPrinterName(), RTL_TEXTENCODING_UTF8));
            strncpy(aOldData.cPrinterName, aPrnByteName.getStr(), SAL_N_ELEMENTS(aOldData.cPrinterName) - 1);
            OString aDriverByteName(OUStringToOString(rJobData.GetDriver(), RTL_TEXTENCODING_UTF8));
            strncpy(aOldData.cDriverName, aDriverByteName.getStr(), SAL_N_ELEMENTS(aOldData.cDriverName) - 1);
            int nPos = rOStream.Tell();
            rOStream.WriteUInt16( 0 );
            rOStream.WriteUInt16( JOBSET_FILE605_SYSTEM );
            rOStream.WriteBytes( &aOldData, sizeof( aOldData ) );
            rOStream.WriteBytes( &aOldJobData, nOldJobDataSize );
            rOStream.WriteBytes( rJobData.GetDriverData(), rJobData.GetDriverDataLen() );

            const std::unordered_map< OUString, OUString >& rValueMap(
                rJobData.GetValueMap());

            for (auto const& value : rValueMap)
            {
                write_uInt16_lenPrefixed_uInt8s_FromOUString(rOStream, value.first, RTL_TEXTENCODING_UTF8);
                write_uInt16_lenPrefixed_uInt8s_FromOUString(rOStream, value.second, RTL_TEXTENCODING_UTF8);
            }
            write_uInt16_lenPrefixed_uInt8s_FromOString(rOStream, "COMPAT_DUPLEX_MODE");
            switch( rJobData.GetDuplexMode() )
            {
                case DuplexMode::Unknown:
                    write_uInt16_lenPrefixed_uInt8s_FromOString(rOStream, "DuplexMode::Unknown");
                    break;
                case DuplexMode::Off:
                    write_uInt16_lenPrefixed_uInt8s_FromOString(rOStream, "DuplexMode::Off");
                    break;
                case DuplexMode::ShortEdge:
                    write_uInt16_lenPrefixed_uInt8s_FromOString(rOStream, "DuplexMode::ShortEdge");
                    break;
                case DuplexMode::LongEdge:
                    write_uInt16_lenPrefixed_uInt8s_FromOString(rOStream, "DuplexMode::LongEdge");
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
