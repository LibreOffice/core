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
    mnSystem            = 0;
    meOrientation       = ORIENTATION_PORTRAIT;
    meDuplexMode        = DUPLEX_UNKNOWN;
    mnPaperBin          = 0;
    mePaperFormat       = PAPER_USER;
    mnPaperWidth        = 0;
    mnPaperHeight       = 0;
    mnDriverDataLen     = 0;
    mpDriverData        = nullptr;
    mbPapersizeFromSetup = false;
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
    mnDriverDataLen( rJobSetup.GetDriverDataLen() )
{
    if ( rJobSetup.GetDriverData() )
    {
        mpDriverData = static_cast<sal_uInt8*>(rtl_allocateMemory( mnDriverDataLen ));
        memcpy( mpDriverData, rJobSetup.GetDriverData(), mnDriverDataLen );
    }
    else
        mpDriverData = nullptr;
    mbPapersizeFromSetup = rJobSetup.GetPapersizeFromSetup();
    maValueMap          = rJobSetup.GetValueMap();
}

ImplJobSetup::~ImplJobSetup()
{
    rtl_freeMemory( mpDriverData );
}

sal_uInt16 ImplJobSetup::GetSystem() const
{
    return mnSystem;
}

void ImplJobSetup::SetSystem(sal_uInt16 rSystem) const
{
    const_cast< ImplJobSetup* >(this)->mnSystem = rSystem;
}

OUString ImplJobSetup::GetPrinterName() const
{
    return maPrinterName;
}

void ImplJobSetup::SetPrinterName(OUString rPrinterName) const
{
    const_cast< ImplJobSetup* >(this)->maPrinterName = rPrinterName;
}

OUString ImplJobSetup::GetDriver() const
{
    return maDriver;
}

void ImplJobSetup::SetDriver(OUString rDriver) const
{
    const_cast< ImplJobSetup* >(this)->maDriver = rDriver;
}

Orientation ImplJobSetup::GetOrientation() const
{
    return meOrientation;
}

void ImplJobSetup::SetOrientation(Orientation rOrientation) const
{
    const_cast< ImplJobSetup* >(this)->meOrientation = rOrientation;
}

DuplexMode ImplJobSetup::GetDuplexMode() const
{
    return meDuplexMode;
}

void ImplJobSetup::SetDuplexMode(DuplexMode rDuplexMode) const
{
    const_cast< ImplJobSetup* >(this)->meDuplexMode = rDuplexMode;
}

sal_uInt16 ImplJobSetup::GetPaperBin() const
{
    return mnPaperBin;
}

void ImplJobSetup::SetPaperBin(sal_uInt16 rPaperBin) const
{
    const_cast< ImplJobSetup* >(this)->mnPaperBin = rPaperBin;
}

Paper ImplJobSetup::GetPaperFormat() const
{
    return mePaperFormat;
}

void ImplJobSetup::SetPaperFormat(Paper rPaperFormat) const
{
    const_cast< ImplJobSetup* >(this)->mePaperFormat = rPaperFormat;
}

long ImplJobSetup::GetPaperWidth() const
{
    return mnPaperWidth;
}

void ImplJobSetup::SetPaperWidth(long rPaperWidth) const
{
    const_cast< ImplJobSetup* >(this)->mnPaperWidth = rPaperWidth;
}

long ImplJobSetup::GetPaperHeight() const
{
    return mnPaperHeight;
}

void ImplJobSetup::SetPaperHeight(long rPaperHeight) const
{
    const_cast< ImplJobSetup* >(this)->mnPaperHeight = rPaperHeight;
}

sal_uInt32 ImplJobSetup::GetDriverDataLen() const
{
    return mnDriverDataLen;
}

void ImplJobSetup::SetDriverDataLen(sal_uInt32 rDriverDataLen) const
{
    const_cast< ImplJobSetup* >(this)->mnDriverDataLen = rDriverDataLen;
}

sal_uInt8* ImplJobSetup::GetDriverData() const
{
    return mpDriverData;
}

void ImplJobSetup::SetDriverData(sal_uInt8* rDriverData) const
{
    const_cast< ImplJobSetup* >(this)->mpDriverData = rDriverData;
}

bool ImplJobSetup::GetPapersizeFromSetup() const
{
    return mbPapersizeFromSetup;
}


void ImplJobSetup::SetPapersizeFromSetup(bool rPapersizeFromSetup) const
{
    const_cast< ImplJobSetup* >(this)->mbPapersizeFromSetup = rPapersizeFromSetup;
}

std::unordered_map< OUString, OUString, OUStringHash > ImplJobSetup::GetValueMap() const
{
    return maValueMap;
}


void ImplJobSetup::SetValueMap( OUString rKey, OUString rValue ) const
{
    const_cast< ImplJobSetup* >(this)->maValueMap [ rKey ] = rValue;
}

JobSetup& JobSetup::operator=( const JobSetup& rJobSetup )
{
    mpData = rJobSetup.mpData;
    return *this;
}

bool ImplJobSetup::operator==( const ImplJobSetup& rImplJobSetup ) const
{
    if ( mnSystem          == rImplJobSetup.mnSystem        &&
         maPrinterName     == rImplJobSetup.maPrinterName   &&
         maDriver          == rImplJobSetup.maDriver        &&
         meOrientation     == rImplJobSetup.meOrientation   &&
         meDuplexMode      == rImplJobSetup.meDuplexMode    &&
         mnPaperBin        == rImplJobSetup.mnPaperBin      &&
         mePaperFormat     == rImplJobSetup.mePaperFormat   &&
         mnPaperWidth      == rImplJobSetup.mnPaperWidth    &&
         mnPaperHeight     == rImplJobSetup.mnPaperHeight   &&
         mnDriverDataLen   == rImplJobSetup.mnDriverDataLen &&
         maValueMap        == rImplJobSetup.maValueMap      &&
         memcmp( mpDriverData, rImplJobSetup.mpDriverData, mnDriverDataLen ) == 0)
        return true;
    return false;
}

JobSetup::JobSetup() : mpData()
{
}

JobSetup::JobSetup( const JobSetup& rJobSetup ) : mpData(rJobSetup.mpData)
{
}

JobSetup::~JobSetup()
{
}

bool JobSetup::operator==( const JobSetup& rJobSetup ) const
{
    return mpData == rJobSetup.mpData;
}

const ImplJobSetup* JobSetup::ImplGetConstData() const
{
    return mpData.get();
}

OUString JobSetup::GetPrinterName() const
{
    return mpData.get()->GetPrinterName();
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
        const size_t nRead = nLen - sizeof(nLen) - sizeof(nSystem);
        if (nRead > rIStream.remainingSize())
        {
            SAL_WARN("vcl", "Parsing error: " << rIStream.remainingSize() <<
                     " max possible entries, but " << nRead << " claimed, truncating");
            return rIStream;
        }
        sal_Size nFirstPos = rIStream.Tell();
        std::unique_ptr<char[]> pTempBuf(new char[nRead]);
        rIStream.ReadBytes(pTempBuf.get(), nRead);
        if (nRead >= sizeof(ImplOldJobSetupData))
        {
            ImplOldJobSetupData* pData = reinterpret_cast<ImplOldJobSetupData*>(pTempBuf.get());

            rtl_TextEncoding aStreamEncoding = RTL_TEXTENCODING_UTF8;
            if( nSystem == JOBSET_FILE364_SYSTEM )
                aStreamEncoding = rIStream.GetStreamCharSet();

            const ImplJobSetup* pJobData = rJobSetup.ImplGetConstData();

            pJobData->SetPrinterName( OStringToOUString(pData->cPrinterName, aStreamEncoding) );
            pJobData->SetDriver( OStringToOUString(pData->cDriverName, aStreamEncoding) );

            // Are these our new JobSetup files?
            if ( nSystem == JOBSET_FILE364_SYSTEM ||
                 nSystem == JOBSET_FILE605_SYSTEM )
            {
                Impl364JobSetupData* pOldJobData    = reinterpret_cast<Impl364JobSetupData*>(pTempBuf.get() + sizeof( ImplOldJobSetupData ));
                sal_uInt16 nOldJobDataSize          = SVBT16ToShort( pOldJobData->nSize );
                pJobData->SetSystem( SVBT16ToShort( pOldJobData->nSystem ) );
                pJobData->SetDriverDataLen( SVBT32ToUInt32( pOldJobData->nDriverDataLen ) );
                pJobData->SetOrientation( (Orientation)SVBT16ToShort( pOldJobData->nOrientation ) );
                pJobData->SetDuplexMode( DUPLEX_UNKNOWN );
                pJobData->SetPaperBin( SVBT16ToShort( pOldJobData->nPaperBin ) );
                pJobData->SetPaperFormat( (Paper)SVBT16ToShort( pOldJobData->nPaperFormat ) );
                pJobData->SetPaperWidth( (long)SVBT32ToUInt32( pOldJobData->nPaperWidth ) );
                pJobData->SetPaperHeight( (long)SVBT32ToUInt32( pOldJobData->nPaperHeight ) );
                if ( pJobData->GetDriverDataLen() )
                {
                    sal_uInt8* pDriverData = reinterpret_cast<sal_uInt8*>(pOldJobData) + nOldJobDataSize;
                    pJobData->SetDriverData( static_cast<sal_uInt8*>(
                        rtl_allocateMemory( pJobData->GetDriverDataLen() )));
                    memcpy( pJobData->GetDriverData(), pDriverData, pJobData->GetDriverDataLen() );
                }
                if( nSystem == JOBSET_FILE605_SYSTEM )
                {
                    rIStream.Seek( nFirstPos + sizeof( ImplOldJobSetupData ) +
                        sizeof( Impl364JobSetupData ) + pJobData->GetDriverDataLen() );
                    while( rIStream.Tell() < nFirstPos + nRead )
                    {
                        OUString aKey = read_uInt16_lenPrefixed_uInt8s_ToOUString(rIStream, RTL_TEXTENCODING_UTF8);
                        OUString aValue = read_uInt16_lenPrefixed_uInt8s_ToOUString(rIStream, RTL_TEXTENCODING_UTF8);
                        if( aKey == "COMPAT_DUPLEX_MODE" )
                        {
                            if( aValue == "DUPLEX_UNKNOWN" )
                                pJobData->SetDuplexMode( DUPLEX_UNKNOWN );
                            else if( aValue == "DUPLEX_OFF" )
                                pJobData->SetDuplexMode( DUPLEX_OFF );
                            else if( aValue == "DUPLEX_SHORTEDGE" )
                                pJobData->SetDuplexMode( DUPLEX_SHORTEDGE );
                            else if( aValue == "DUPLEX_LONGEDGE" )
                                pJobData->SetDuplexMode( DUPLEX_LONGEDGE );
                        }
                        else
                            pJobData->SetValueMap(aKey, aValue);
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
    {
        sal_uInt16 nSystem = JOBSET_FILE605_SYSTEM;

        const ImplJobSetup* pJobData = rJobSetup.ImplGetConstData();
        Impl364JobSetupData aOldJobData;
        sal_uInt16 nOldJobDataSize = sizeof( aOldJobData );
        ShortToSVBT16( nOldJobDataSize, aOldJobData.nSize );
        ShortToSVBT16( pJobData->GetSystem(), aOldJobData.nSystem );
        UInt32ToSVBT32( pJobData->GetDriverDataLen(), aOldJobData.nDriverDataLen );
        ShortToSVBT16( (sal_uInt16)(pJobData->GetOrientation()), aOldJobData.nOrientation );
        ShortToSVBT16( pJobData->GetPaperBin(), aOldJobData.nPaperBin );
        ShortToSVBT16( (sal_uInt16)(pJobData->GetPaperFormat()), aOldJobData.nPaperFormat );
        UInt32ToSVBT32( (sal_uLong)(pJobData->GetPaperWidth()), aOldJobData.nPaperWidth );
        UInt32ToSVBT32( (sal_uLong)(pJobData->GetPaperHeight()), aOldJobData.nPaperHeight );

        ImplOldJobSetupData aOldData;
        memset( &aOldData, 0, sizeof( aOldData ) );
        OString aPrnByteName(OUStringToOString(pJobData->GetPrinterName(), RTL_TEXTENCODING_UTF8));
        strncpy( aOldData.cPrinterName, aPrnByteName.getStr(), 63 );
        OString aDriverByteName(OUStringToOString(pJobData->GetDriver(), RTL_TEXTENCODING_UTF8));
        strncpy( aOldData.cDriverName, aDriverByteName.getStr(), 31 );
        int nPos = rOStream.Tell();
        rOStream.WriteUInt16( nLen );
        rOStream.WriteUInt16( nSystem );
        rOStream.WriteBytes( &aOldData, sizeof( aOldData ) );
        rOStream.WriteBytes( &aOldJobData, nOldJobDataSize );
        rOStream.WriteBytes( pJobData->GetDriverData(), pJobData->GetDriverDataLen() );
        std::unordered_map< OUString, OUString, OUStringHash >::const_iterator it;
        for( it = pJobData->GetValueMap().begin(); it != pJobData->GetValueMap().end(); ++it )
        {
            write_uInt16_lenPrefixed_uInt8s_FromOUString(rOStream, it->first, RTL_TEXTENCODING_UTF8);
            write_uInt16_lenPrefixed_uInt8s_FromOUString(rOStream, it->second, RTL_TEXTENCODING_UTF8);
        }
        write_uInt16_lenPrefixed_uInt8s_FromOString(rOStream, "COMPAT_DUPLEX_MODE");
        switch( pJobData->GetDuplexMode() )
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

    return rOStream;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
