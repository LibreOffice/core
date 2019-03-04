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

#include <comphelper/docpasswordhelper.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <osl/thread.h>
#include <osl/diagnose.h>
#include <sal/log.hxx>
#include <tools/solar.h>
#include <ftools.hxx>
#include <xistream.hxx>
#include <xlstring.hxx>
#include <xiroot.hxx>

#include <vector>
#include <memory>

using namespace ::com::sun::star;

// Decryption
XclImpDecrypter::XclImpDecrypter() :
    mnError( EXC_ENCR_ERROR_UNSUPP_CRYPT ),
    mnOldPos( STREAM_SEEK_TO_END ),
    mnRecSize( 0 )
{
}

XclImpDecrypter::XclImpDecrypter( const XclImpDecrypter& rSrc ) :
    ::comphelper::IDocPasswordVerifier(),
    mnError( rSrc.mnError ),
    mnOldPos( STREAM_SEEK_TO_END ),
    mnRecSize( 0 )
{
}

XclImpDecrypter::~XclImpDecrypter()
{
}

XclImpDecrypterRef XclImpDecrypter::Clone() const
{
    XclImpDecrypterRef xNewDecr;
    if( IsValid() )
        xNewDecr.reset( OnClone() );
    return xNewDecr;
}

::comphelper::DocPasswordVerifierResult XclImpDecrypter::verifyPassword( const OUString& rPassword, uno::Sequence< beans::NamedValue >& o_rEncryptionData )
{
    o_rEncryptionData = OnVerifyPassword( rPassword );
    mnError = o_rEncryptionData.getLength() ? ERRCODE_NONE : ERRCODE_ABORT;
    return o_rEncryptionData.getLength() ? ::comphelper::DocPasswordVerifierResult::OK : ::comphelper::DocPasswordVerifierResult::WrongPassword;
}

::comphelper::DocPasswordVerifierResult XclImpDecrypter::verifyEncryptionData( const uno::Sequence< beans::NamedValue >& rEncryptionData )
{
    bool bValid = OnVerifyEncryptionData( rEncryptionData );
    mnError = bValid ? ERRCODE_NONE : ERRCODE_ABORT;
    return bValid ? ::comphelper::DocPasswordVerifierResult::OK : ::comphelper::DocPasswordVerifierResult::WrongPassword;
}

void XclImpDecrypter::Update( const SvStream& rStrm, sal_uInt16 nRecSize )
{
    if( IsValid() )
    {
        sal_uInt64 const nNewPos = rStrm.Tell();
        if( (mnOldPos != nNewPos) || (mnRecSize != nRecSize) )
        {
            OnUpdate( mnOldPos, nNewPos, nRecSize );
            mnOldPos = nNewPos;
            mnRecSize = nRecSize;
        }
    }
}

sal_uInt16 XclImpDecrypter::Read( SvStream& rStrm, void* pData, sal_uInt16 nBytes )
{
    sal_uInt16 nRet = 0;
    if( pData && nBytes )
    {
        if( IsValid() )
        {
            Update( rStrm, mnRecSize );
            nRet = OnRead( rStrm, static_cast< sal_uInt8* >( pData ), nBytes );
            mnOldPos = rStrm.Tell();
        }
        else
            nRet = static_cast<sal_uInt16>(rStrm.ReadBytes(pData, nBytes));
    }
    return nRet;
}

XclImpBiff5Decrypter::XclImpBiff5Decrypter( sal_uInt16 nKey, sal_uInt16 nHash ) :
    mnKey( nKey ),
    mnHash( nHash )
{
}

XclImpBiff5Decrypter::XclImpBiff5Decrypter( const XclImpBiff5Decrypter& rSrc ) :
    XclImpDecrypter( rSrc ),
    maEncryptionData( rSrc.maEncryptionData ),
    mnKey( rSrc.mnKey ),
    mnHash( rSrc.mnHash )
{
    if( IsValid() )
        maCodec.InitCodec( maEncryptionData );
}

XclImpBiff5Decrypter* XclImpBiff5Decrypter::OnClone() const
{
    return new XclImpBiff5Decrypter( *this );
}

uno::Sequence< beans::NamedValue > XclImpBiff5Decrypter::OnVerifyPassword( const OUString& rPassword )
{
    maEncryptionData.realloc( 0 );

    /*  Convert password to a byte string. TODO: this needs some fine tuning
        according to the spec... */
    OString aBytePassword = OUStringToOString( rPassword, osl_getThreadTextEncoding() );
    sal_Int32 nLen = aBytePassword.getLength();
    if( (0 < nLen) && (nLen < 16) )
    {
        // init codec
        maCodec.InitKey( reinterpret_cast<sal_uInt8 const *>(aBytePassword.getStr()) );

        if ( maCodec.VerifyKey( mnKey, mnHash ) )
        {
            maEncryptionData = maCodec.GetEncryptionData();

            // since the export uses Std97 encryption always we have to request it here
            ::std::vector< sal_uInt16 > aPassVect( 16 );
            sal_Int32 nInd = 0;
            std::for_each(aPassVect.begin(), aPassVect.begin() + nLen,
                [&rPassword, &nInd](sal_uInt16& rPass) {
                    rPass = static_cast< sal_uInt16 >( rPassword[nInd] );
                    ++nInd;
                });

            uno::Sequence< sal_Int8 > aDocId = ::comphelper::DocPasswordHelper::GenerateRandomByteSequence( 16 );
            OSL_ENSURE( aDocId.getLength() == 16, "Unexpected length of the sequence!" );

            ::msfilter::MSCodec_Std97 aCodec97;
            aCodec97.InitKey(aPassVect.data(), reinterpret_cast<sal_uInt8 const *>(aDocId.getConstArray()));

            // merge the EncryptionData, there should be no conflicts
            ::comphelper::SequenceAsHashMap aEncryptionHash( maEncryptionData );
            aEncryptionHash.update( ::comphelper::SequenceAsHashMap( aCodec97.GetEncryptionData() ) );
            aEncryptionHash >> maEncryptionData;
        }
    }

    return maEncryptionData;
}

bool XclImpBiff5Decrypter::OnVerifyEncryptionData( const uno::Sequence< beans::NamedValue >& rEncryptionData )
{
    maEncryptionData.realloc( 0 );

    if( rEncryptionData.getLength() )
    {
        // init codec
        maCodec.InitCodec( rEncryptionData );

        if ( maCodec.VerifyKey( mnKey, mnHash ) )
            maEncryptionData = rEncryptionData;
    }

    return maEncryptionData.getLength();
}

void XclImpBiff5Decrypter::OnUpdate( std::size_t /*nOldStrmPos*/, std::size_t nNewStrmPos, sal_uInt16 nRecSize )
{
    maCodec.InitCipher();
    maCodec.Skip( (nNewStrmPos + nRecSize) & 0x0F );
}

sal_uInt16 XclImpBiff5Decrypter::OnRead( SvStream& rStrm, sal_uInt8* pnData, sal_uInt16 nBytes )
{
    sal_uInt16 nRet = static_cast<sal_uInt16>(rStrm.ReadBytes(pnData, nBytes));
    maCodec.Decode( pnData, nRet );
    return nRet;
}

XclImpBiff8Decrypter::XclImpBiff8Decrypter(const std::vector<sal_uInt8>& rSalt,
                                           const std::vector<sal_uInt8>& rVerifier,
                                           const std::vector<sal_uInt8>& rVerifierHash)
    : maSalt(rSalt)
    , maVerifier(rVerifier)
    , maVerifierHash(rVerifierHash)
    , mpCodec(nullptr)
{
}

XclImpBiff8Decrypter::XclImpBiff8Decrypter(const XclImpBiff8Decrypter& rSrc)
    : XclImpDecrypter(rSrc)
    , maEncryptionData(rSrc.maEncryptionData)
    , maSalt(rSrc.maSalt)
    , maVerifier(rSrc.maVerifier)
    , maVerifierHash(rSrc.maVerifierHash)
    , mpCodec(nullptr)
{
}

XclImpBiff8StdDecrypter::XclImpBiff8StdDecrypter(const XclImpBiff8StdDecrypter& rSrc)
    : XclImpBiff8Decrypter(rSrc)
{
    mpCodec = &maCodec;
    if (IsValid())
        maCodec.InitCodec(maEncryptionData);
}

XclImpBiff8StdDecrypter* XclImpBiff8StdDecrypter::OnClone() const
{
    return new XclImpBiff8StdDecrypter(*this);
}

XclImpBiff8CryptoAPIDecrypter::XclImpBiff8CryptoAPIDecrypter(const XclImpBiff8CryptoAPIDecrypter& rSrc)
    : XclImpBiff8Decrypter(rSrc)
{
    mpCodec = &maCodec;
    if (IsValid())
        maCodec.InitCodec(maEncryptionData);
}

XclImpBiff8CryptoAPIDecrypter* XclImpBiff8CryptoAPIDecrypter::OnClone() const
{
    return new XclImpBiff8CryptoAPIDecrypter(*this);
}

uno::Sequence< beans::NamedValue > XclImpBiff8Decrypter::OnVerifyPassword( const OUString& rPassword )
{
    maEncryptionData.realloc( 0 );

    sal_Int32 nLen = rPassword.getLength();
    if( (0 < nLen) && (nLen < 16) )
    {
        // copy string to sal_uInt16 array
        ::std::vector< sal_uInt16 > aPassVect( 16 );
        const sal_Unicode* pcChar = rPassword.getStr();
        std::for_each(aPassVect.begin(), aPassVect.begin() + nLen,
            [&pcChar](sal_uInt16& rPass) {
                rPass = static_cast< sal_uInt16 >( *pcChar );
                ++pcChar;
            });

        // init codec
        mpCodec->InitKey(aPassVect.data(), maSalt.data());
        if (mpCodec->VerifyKey(maVerifier.data(), maVerifierHash.data()))
            maEncryptionData = mpCodec->GetEncryptionData();
    }

    return maEncryptionData;
}

bool XclImpBiff8Decrypter::OnVerifyEncryptionData( const uno::Sequence< beans::NamedValue >& rEncryptionData )
{
    maEncryptionData.realloc( 0 );

    if( rEncryptionData.getLength() )
    {
        // init codec
        mpCodec->InitCodec( rEncryptionData );

        if (mpCodec->VerifyKey(maVerifier.data(), maVerifierHash.data()))
            maEncryptionData = rEncryptionData;
    }

    return maEncryptionData.getLength();
}

void XclImpBiff8Decrypter::OnUpdate( std::size_t nOldStrmPos, std::size_t nNewStrmPos, sal_uInt16 /*nRecSize*/ )
{
    if( nNewStrmPos != nOldStrmPos )
    {
        sal_uInt32 nOldBlock = GetBlock( nOldStrmPos );
        sal_uInt16 nOldOffset = GetOffset( nOldStrmPos );

        sal_uInt32 nNewBlock = GetBlock( nNewStrmPos );
        sal_uInt16 nNewOffset = GetOffset( nNewStrmPos );

        /*  Rekey cipher, if block changed or if previous offset in same block. */
        if( (nNewBlock != nOldBlock) || (nNewOffset < nOldOffset) )
        {
            mpCodec->InitCipher( nNewBlock );
            nOldOffset = 0;     // reset nOldOffset for next if() statement
        }

        /*  Seek to correct offset. */
        if( nNewOffset > nOldOffset )
            mpCodec->Skip( nNewOffset - nOldOffset );
    }
}

sal_uInt16 XclImpBiff8Decrypter::OnRead( SvStream& rStrm, sal_uInt8* pnData, sal_uInt16 nBytes )
{
    sal_uInt16 nRet = 0;

    sal_uInt8* pnCurrData = pnData;
    sal_uInt16 nBytesLeft = nBytes;
    while( nBytesLeft )
    {
        sal_uInt16 nBlockLeft = EXC_ENCR_BLOCKSIZE - GetOffset( rStrm.Tell() );
        sal_uInt16 nDecBytes = ::std::min< sal_uInt16 >( nBytesLeft, nBlockLeft );

        // read the block from stream
        nRet = nRet + static_cast<sal_uInt16>(rStrm.ReadBytes(pnCurrData, nDecBytes));
        // decode the block inplace
        mpCodec->Decode( pnCurrData, nDecBytes, pnCurrData, nDecBytes );
        if( GetOffset( rStrm.Tell() ) == 0 )
            mpCodec->InitCipher( GetBlock( rStrm.Tell() ) );

        pnCurrData += nDecBytes;
        nBytesLeft = nBytesLeft - nDecBytes;
    }

    return nRet;
}

sal_uInt32 XclImpBiff8Decrypter::GetBlock( std::size_t nStrmPos )
{
    return static_cast< sal_uInt32 >( nStrmPos / EXC_ENCR_BLOCKSIZE );
}

sal_uInt16 XclImpBiff8Decrypter::GetOffset( std::size_t nStrmPos )
{
    return static_cast< sal_uInt16 >( nStrmPos % EXC_ENCR_BLOCKSIZE );
}

// Stream
XclImpStreamPos::XclImpStreamPos() :
    mnPos( STREAM_SEEK_TO_BEGIN ),
    mnNextPos( STREAM_SEEK_TO_BEGIN ),
    mnCurrSize( 0 ),
    mnRawRecId( EXC_ID_UNKNOWN ),
    mnRawRecSize( 0 ),
    mnRawRecLeft( 0 ),
    mbValid( false )
{
}

void XclImpStreamPos::Set(
        const SvStream& rStrm, std::size_t nNextPos, std::size_t nCurrSize,
        sal_uInt16 nRawRecId, sal_uInt16 nRawRecSize, sal_uInt16 nRawRecLeft,
        bool bValid )
{
    mnPos = rStrm.Tell();
    mnNextPos = nNextPos;
    mnCurrSize = nCurrSize;
    mnRawRecId = nRawRecId;
    mnRawRecSize = nRawRecSize;
    mnRawRecLeft = nRawRecLeft;
    mbValid = bValid;
}

void XclImpStreamPos::Get(
        SvStream& rStrm, std::size_t& rnNextPos, std::size_t& rnCurrSize,
        sal_uInt16& rnRawRecId, sal_uInt16& rnRawRecSize, sal_uInt16& rnRawRecLeft,
        bool& rbValid ) const
{
    rStrm.Seek( mnPos );
    rnNextPos = mnNextPos;
    rnCurrSize = mnCurrSize;
    rnRawRecId = mnRawRecId;
    rnRawRecSize = mnRawRecSize;
    rnRawRecLeft = mnRawRecLeft;
    rbValid = mbValid;
}

XclBiff XclImpStream::DetectBiffVersion( SvStream& rStrm )
{
    XclBiff eBiff = EXC_BIFF_UNKNOWN;

    rStrm.Seek( STREAM_SEEK_TO_BEGIN );
    sal_uInt16 nBofId, nBofSize;
    rStrm.ReadUInt16( nBofId ).ReadUInt16( nBofSize );

    if( (4 <= nBofSize) && (nBofSize <= 16) ) switch( nBofId )
    {
        case EXC_ID2_BOF:
            eBiff = EXC_BIFF2;
        break;
        case EXC_ID3_BOF:
            eBiff = EXC_BIFF3;
        break;
        case EXC_ID4_BOF:
            eBiff = EXC_BIFF4;
        break;
        case EXC_ID5_BOF:
        {
            sal_uInt16 nVersion;
            rStrm.ReadUInt16( nVersion );
            // #i23425# #i44031# #i62752# there are some *really* broken documents out there...
            switch( nVersion & 0xFF00 )
            {
                case 0:             eBiff = EXC_BIFF5;  break;  // #i44031# #i62752#
                case EXC_BOF_BIFF2: eBiff = EXC_BIFF2;  break;
                case EXC_BOF_BIFF3: eBiff = EXC_BIFF3;  break;
                case EXC_BOF_BIFF4: eBiff = EXC_BIFF4;  break;
                case EXC_BOF_BIFF5: eBiff = EXC_BIFF5;  break;
                case EXC_BOF_BIFF8: eBiff = EXC_BIFF8;  break;
                default:    SAL_WARN("sc",  "XclImpStream::DetectBiffVersion - unknown BIFF version: 0x" << std::hex << nVersion );
            }
        }
        break;
    }
    return eBiff;
}

XclImpStream::XclImpStream( SvStream& rInStrm, const XclImpRoot& rRoot ) :
    mrStrm( rInStrm ),
    mrRoot( rRoot ),
    mnGlobRecId( EXC_ID_UNKNOWN ),
    mbGlobValidRec( false ),
    mbHasGlobPos( false ),
    mnNextRecPos( STREAM_SEEK_TO_BEGIN ),
    mnCurrRecSize( 0 ),
    mnComplRecSize( 0 ),
    mbHasComplRec( false ),
    mnRecId( EXC_ID_UNKNOWN ),
    mnAltContId( EXC_ID_UNKNOWN ),
    mnRawRecId( EXC_ID_UNKNOWN ),
    mnRawRecSize( 0 ),
    mnRawRecLeft( 0 ),
    mcNulSubst( '?' ),
    mbCont( true ),
    mbUseDecr( false ),
    mbValidRec( false ),
    mbValid( false )
{
    mnStreamSize = mrStrm.TellEnd();
    mrStrm.Seek( STREAM_SEEK_TO_BEGIN );
}

XclImpStream::~XclImpStream()
{
}

bool XclImpStream::StartNextRecord()
{
    maPosStack.clear();

    /*  #i4266# Counter to ignore zero records (id==len==0) (i.e. the application
        "Crystal Report" writes zero records between other records) */
    std::size_t nZeroRecCount = 5;
    bool bIsZeroRec = false;

    do
    {
        mbValidRec = ReadNextRawRecHeader();
        bIsZeroRec = (mnRawRecId == 0) && (mnRawRecSize == 0);
        if( bIsZeroRec ) --nZeroRecCount;
        mnNextRecPos = mrStrm.Tell() + mnRawRecSize;
    }
    while( mbValidRec && ((mbCont && IsContinueId( mnRawRecId )) || (bIsZeroRec && nZeroRecCount)) );

    mbValidRec = mbValidRec && !bIsZeroRec;
    mbValid = mbValidRec;
    SetupRecord();

    return mbValidRec;
}

bool XclImpStream::StartNextRecord( std::size_t nNextRecPos )
{
    mnNextRecPos = nNextRecPos;
    return StartNextRecord();
}

void XclImpStream::ResetRecord( bool bContLookup, sal_uInt16 nAltContId )
{
    if( mbValidRec )
    {
        maPosStack.clear();
        RestorePosition( maFirstRec );
        mnCurrRecSize = mnComplRecSize = mnRawRecSize;
        mbHasComplRec = !bContLookup;
        mbCont = bContLookup;
        mnAltContId = nAltContId;
        EnableDecryption();
    }
}

void XclImpStream::RewindRecord()
{
    mnNextRecPos = maFirstRec.GetPos();
    mbValid = mbValidRec = false;
}

void XclImpStream::SetDecrypter( XclImpDecrypterRef const & xDecrypter )
{
    mxDecrypter = xDecrypter;
    EnableDecryption();
    SetupDecrypter();
}

void XclImpStream::CopyDecrypterFrom( const XclImpStream& rStrm )
{
    XclImpDecrypterRef xNewDecr;
    if( rStrm.mxDecrypter )
        xNewDecr = rStrm.mxDecrypter->Clone();
    SetDecrypter( xNewDecr );
}

void XclImpStream::EnableDecryption( bool bEnable )
{
    mbUseDecr = bEnable && mxDecrypter && mxDecrypter->IsValid();
}

void XclImpStream::PushPosition()
{
    maPosStack.emplace_back( );
    StorePosition( maPosStack.back() );
}

void XclImpStream::PopPosition()
{
    OSL_ENSURE( !maPosStack.empty(), "XclImpStream::PopPosition - stack empty" );
    if( !maPosStack.empty() )
    {
        RestorePosition( maPosStack.back() );
        maPosStack.pop_back();
    }
}

void XclImpStream::StoreGlobalPosition()
{
    StorePosition( maGlobPos );
    mnGlobRecId = mnRecId;
    mbGlobValidRec = mbValidRec;
    mbHasGlobPos = true;
}

void XclImpStream::SeekGlobalPosition()
{
    OSL_ENSURE( mbHasGlobPos, "XclImpStream::SeekGlobalPosition - no position stored" );
    if( mbHasGlobPos )
    {
        RestorePosition( maGlobPos );
        mnRecId = mnGlobRecId;
        mnComplRecSize = mnCurrRecSize;
        mbHasComplRec = !mbCont;
        mbValidRec = mbGlobValidRec;
    }
}

std::size_t XclImpStream::GetRecPos() const
{
    return mbValid ? (mnCurrRecSize - mnRawRecLeft) : EXC_REC_SEEK_TO_END;
}

std::size_t XclImpStream::GetRecSize()
{
    if( !mbHasComplRec )
    {
        PushPosition();
        while( JumpToNextContinue() ) ;  // JumpToNextContinue() adds up mnCurrRecSize
        mnComplRecSize = mnCurrRecSize;
        mbHasComplRec = true;
        PopPosition();
    }
    return mnComplRecSize;
}

std::size_t XclImpStream::GetRecLeft()
{
    return mbValid ? (GetRecSize() - GetRecPos()) : 0;
}

sal_uInt16 XclImpStream::GetNextRecId()
{
    sal_uInt16 nRecId = EXC_ID_UNKNOWN;
    if( mbValidRec )
    {
        PushPosition();
        while( JumpToNextContinue() ) ;  // skip following CONTINUE records
        if( mnNextRecPos < mnStreamSize )
        {
            mrStrm.Seek( mnNextRecPos );
            mrStrm.ReadUInt16( nRecId );
        }
        PopPosition();
    }
    return nRecId;
}

sal_uInt16 XclImpStream::PeekRecId( std::size_t nPos )
{
    sal_uInt16 nRecId = EXC_ID_UNKNOWN;
    if (mbValidRec && nPos < mnStreamSize)
    {
        sal_uInt64 const nCurPos = mrStrm.Tell();
        mrStrm.Seek(nPos);
        mrStrm.ReadUInt16( nRecId );
        mrStrm.Seek(nCurPos);
    }
    return nRecId;
}

sal_uInt8 XclImpStream::ReaduInt8()
{
    sal_uInt8 nValue = 0;
    if( EnsureRawReadSize( 1 ) )
    {
        if( mbUseDecr )
            mxDecrypter->Read( mrStrm, &nValue, 1 );
        else
            mrStrm.ReadUChar( nValue );
        --mnRawRecLeft;
    }
    return nValue;
}

sal_Int16 XclImpStream::ReadInt16()
{
    sal_Int16 nValue = 0;
    if( EnsureRawReadSize( 2 ) )
    {
        if( mbUseDecr )
        {
            SVBT16 pnBuffer;
            mxDecrypter->Read( mrStrm, pnBuffer, 2 );
            nValue = static_cast< sal_Int16 >( SVBT16ToUInt16( pnBuffer ) );
        }
        else
            mrStrm.ReadInt16( nValue );
        mnRawRecLeft -= 2;
    }
    return nValue;
}

sal_uInt16 XclImpStream::ReaduInt16()
{
    sal_uInt16 nValue = 0;
    if( EnsureRawReadSize( 2 ) )
    {
        if( mbUseDecr )
        {
            SVBT16 pnBuffer;
            mxDecrypter->Read( mrStrm, pnBuffer, 2 );
            nValue = SVBT16ToUInt16( pnBuffer );
        }
        else
            mrStrm.ReadUInt16( nValue );
        mnRawRecLeft -= 2;
    }
    return nValue;
}

sal_Int32 XclImpStream::ReadInt32()
{
    sal_Int32 nValue = 0;
    if( EnsureRawReadSize( 4 ) )
    {
        if( mbUseDecr )
        {
            SVBT32 pnBuffer;
            mxDecrypter->Read( mrStrm, pnBuffer, 4 );
            nValue = static_cast< sal_Int32 >( SVBT32ToUInt32( pnBuffer ) );
        }
        else
            mrStrm.ReadInt32( nValue );
        mnRawRecLeft -= 4;
    }
    return nValue;
}

sal_uInt32 XclImpStream::ReaduInt32()
{
    sal_uInt32 nValue = 0;
    if( EnsureRawReadSize( 4 ) )
    {
        if( mbUseDecr )
        {
            SVBT32 pnBuffer;
            mxDecrypter->Read( mrStrm, pnBuffer, 4 );
            nValue = SVBT32ToUInt32( pnBuffer );
        }
        else
            mrStrm.ReadUInt32( nValue );
        mnRawRecLeft -= 4;
    }
    return nValue;
}

double XclImpStream::ReadDouble()
{
    double nValue = 0;
    if( EnsureRawReadSize( 8 ) )
    {
        if( mbUseDecr )
        {
            SVBT64 pnBuffer;
            mxDecrypter->Read( mrStrm, pnBuffer, 8 );
            nValue = SVBT64ToDouble( pnBuffer );
        }
        else
            mrStrm.ReadDouble( nValue );
        mnRawRecLeft -= 8;
    }
    return nValue;
}

std::size_t XclImpStream::Read( void* pData, std::size_t nBytes )
{
    std::size_t nRet = 0;
    if( mbValid && pData && (nBytes > 0) )
    {
        sal_uInt8* pnBuffer = static_cast< sal_uInt8* >( pData );
        std::size_t nBytesLeft = nBytes;

        while( mbValid && (nBytesLeft > 0) )
        {
            sal_uInt16 nReadSize = GetMaxRawReadSize( nBytesLeft );
            sal_uInt16 nReadRet = ReadRawData( pnBuffer, nReadSize );
            nRet += nReadRet;
            mbValid = (nReadSize == nReadRet);
            OSL_ENSURE( mbValid, "XclImpStream::Read - stream read error" );
            pnBuffer += nReadRet;
            nBytesLeft -= nReadRet;
            if( mbValid && (nBytesLeft > 0) )
                JumpToNextContinue();
            OSL_ENSURE( mbValid, "XclImpStream::Read - record overread" );
        }
    }
    return nRet;
}

std::size_t XclImpStream::CopyToStream( SvStream& rOutStrm, std::size_t nBytes )
{
    std::size_t nRet = 0;
    if( mbValid && (nBytes > 0) )
    {
        const std::size_t nMaxBuffer = 4096;
        std::unique_ptr<sal_uInt8[]> pnBuffer(new sal_uInt8[ ::std::min( nBytes, nMaxBuffer ) ]);
        std::size_t nBytesLeft = nBytes;

        while( mbValid && (nBytesLeft > 0) )
        {
            std::size_t nReadSize = ::std::min( nBytesLeft, nMaxBuffer );
            nRet += Read( pnBuffer.get(), nReadSize );
            // writing more bytes than read results in invalid memory access
            SAL_WARN_IF(nRet != nReadSize, "sc", "read less bytes than requested");
            rOutStrm.WriteBytes(pnBuffer.get(), nReadSize);
            nBytesLeft -= nReadSize;
        }
    }
    return nRet;
}

void XclImpStream::CopyRecordToStream( SvStream& rOutStrm )
{
    if( mbValidRec )
    {
        PushPosition();
        RestorePosition( maFirstRec );
        CopyToStream( rOutStrm, GetRecSize() );
        PopPosition();
    }
}

void XclImpStream::Seek( std::size_t nPos )
{
    if( mbValidRec )
    {
        std::size_t nCurrPos = GetRecPos();
        if( !mbValid || (nPos < nCurrPos) ) // from invalid state or backward
        {
            RestorePosition( maFirstRec );
            Ignore( nPos );
        }
        else if( nPos > nCurrPos )          // forward
        {
            Ignore( nPos - nCurrPos );
        }
    }
}

void XclImpStream::Ignore( std::size_t nBytes )
{
    // implementation similar to Read(), but without really reading anything
    std::size_t nBytesLeft = nBytes;
    while( mbValid && (nBytesLeft > 0) )
    {
        sal_uInt16 nReadSize = GetMaxRawReadSize( nBytesLeft );
        mbValid = checkSeek(mrStrm, mrStrm.Tell() + nReadSize);
        mnRawRecLeft = mnRawRecLeft - nReadSize;
        nBytesLeft -= nReadSize;
        if (mbValid && nBytesLeft > 0)
            JumpToNextContinue();
        OSL_ENSURE( mbValid, "XclImpStream::Ignore - record overread" );
    }
}

std::size_t XclImpStream::ReadUniStringExtHeader(
        bool& rb16Bit, bool& rbRich, bool& rbFareast,
        sal_uInt16& rnFormatRuns, sal_uInt32& rnExtInf, sal_uInt8 nFlags )
{
    OSL_ENSURE( !::get_flag( nFlags, EXC_STRF_UNKNOWN ), "XclImpStream::ReadUniStringExt - unknown flags" );
    rb16Bit = ::get_flag( nFlags, EXC_STRF_16BIT );
    rbRich = ::get_flag( nFlags, EXC_STRF_RICH );
    rbFareast = ::get_flag( nFlags, EXC_STRF_FAREAST );
    rnFormatRuns = rbRich ? ReaduInt16() : 0;
    rnExtInf = rbFareast ? ReaduInt32() : 0;
    return rnExtInf + 4 * rnFormatRuns;
}

std::size_t XclImpStream::ReadUniStringExtHeader( bool& rb16Bit, sal_uInt8 nFlags )
{
    bool bRich, bFareast;
    sal_uInt16 nCrun;
    sal_uInt32 nExtInf;
    return ReadUniStringExtHeader( rb16Bit, bRich, bFareast, nCrun, nExtInf, nFlags );
}

OUString XclImpStream::ReadRawUniString( sal_uInt16 nChars, bool b16Bit )
{
    OUStringBuffer aRet;
    sal_uInt16 nCharsLeft = nChars;
    sal_uInt16 nReadSize;

    std::unique_ptr<sal_Unicode[]> pcBuffer(new sal_Unicode[ nCharsLeft + 1 ]);

    while( IsValid() && (nCharsLeft > 0) )
    {
        if( b16Bit )
        {
            nReadSize = ::std::min< sal_uInt16 >( nCharsLeft, mnRawRecLeft / 2 );
            OSL_ENSURE( (nReadSize <= nCharsLeft) || !(mnRawRecLeft & 0x1),
                "XclImpStream::ReadRawUniString - missing a byte" );
        }
        else
            nReadSize = GetMaxRawReadSize( nCharsLeft );

        sal_Unicode* pcUniChar = pcBuffer.get();
        sal_Unicode* pcEndChar = pcBuffer.get() + nReadSize;

        if( b16Bit )
        {
            sal_uInt16 nReadChar;
            for( ; IsValid() && (pcUniChar < pcEndChar); ++pcUniChar )
            {
                nReadChar = ReaduInt16();
                (*pcUniChar) = (nReadChar == EXC_NUL) ? mcNulSubst : static_cast< sal_Unicode >( nReadChar );
            }
        }
        else
        {
            sal_uInt8 nReadChar;
            for( ; IsValid() && (pcUniChar < pcEndChar); ++pcUniChar )
            {
                nReadChar = ReaduInt8();
                (*pcUniChar) = (nReadChar == EXC_NUL_C) ? mcNulSubst : static_cast< sal_Unicode >( nReadChar );
            }
        }

        *pcEndChar = '\0';
        aRet.append( pcBuffer.get(), pcUniChar - pcBuffer.get()  );

        nCharsLeft = nCharsLeft - nReadSize;
        if( nCharsLeft > 0 )
            JumpToNextStringContinue( b16Bit );
    }

    return aRet.makeStringAndClear();
}

OUString XclImpStream::ReadUniString( sal_uInt16 nChars, sal_uInt8 nFlags )
{
    bool b16Bit;
    std::size_t nExtSize = ReadUniStringExtHeader( b16Bit, nFlags );
    OUString aRet( ReadRawUniString( nChars, b16Bit ) );
    Ignore( nExtSize );
    return aRet;
}

OUString XclImpStream::ReadUniString( sal_uInt16 nChars )
{
    return ReadUniString( nChars, ReaduInt8() );
}

OUString XclImpStream::ReadUniString()
{
    return ReadUniString( ReaduInt16() );
}

void XclImpStream::IgnoreRawUniString( sal_uInt16 nChars, bool b16Bit )
{
    sal_uInt16 nCharsLeft = nChars;
    sal_uInt16 nReadSize;

    while( IsValid() && (nCharsLeft > 0) )
    {
        if( b16Bit )
        {
            nReadSize = ::std::min< sal_uInt16 >( nCharsLeft, mnRawRecLeft / 2 );
            OSL_ENSURE( (nReadSize <= nCharsLeft) || !(mnRawRecLeft & 0x1),
                "XclImpStream::IgnoreRawUniString - missing a byte" );
            Ignore( nReadSize * 2 );
        }
        else
        {
            nReadSize = GetMaxRawReadSize( nCharsLeft );
            Ignore( nReadSize );
        }

        nCharsLeft = nCharsLeft - nReadSize;
        if( nCharsLeft > 0 )
            JumpToNextStringContinue( b16Bit );
    }
}

void XclImpStream::IgnoreUniString( sal_uInt16 nChars, sal_uInt8 nFlags )
{
    bool b16Bit;
    std::size_t nExtSize = ReadUniStringExtHeader( b16Bit, nFlags );
    IgnoreRawUniString( nChars, b16Bit );
    Ignore( nExtSize );
}

void XclImpStream::IgnoreUniString( sal_uInt16 nChars )
{
    IgnoreUniString( nChars, ReaduInt8() );
}

OUString XclImpStream::ReadRawByteString( sal_uInt16 nChars )
{
    nChars = GetMaxRawReadSize(nChars);
    std::unique_ptr<sal_Char[]> pcBuffer(new sal_Char[ nChars + 1 ]);
    sal_uInt16 nCharsRead = ReadRawData( pcBuffer.get(), nChars );
    pcBuffer[ nCharsRead ] = '\0';
    OUString aRet( pcBuffer.get(), strlen(pcBuffer.get()), mrRoot.GetTextEncoding() );
    return aRet;
}

OUString XclImpStream::ReadByteString( bool b16BitLen )
{
    return ReadRawByteString( b16BitLen ? ReaduInt16() : ReaduInt8() );
}

// private --------------------------------------------------------------------

void XclImpStream::StorePosition( XclImpStreamPos& rPos )
{
    rPos.Set( mrStrm, mnNextRecPos, mnCurrRecSize, mnRawRecId, mnRawRecSize, mnRawRecLeft, mbValid );
}

void XclImpStream::RestorePosition( const XclImpStreamPos& rPos )
{
    rPos.Get( mrStrm, mnNextRecPos, mnCurrRecSize, mnRawRecId, mnRawRecSize, mnRawRecLeft, mbValid );
    SetupDecrypter();
}

bool XclImpStream::ReadNextRawRecHeader()
{
    bool bRet = checkSeek(mrStrm, mnNextRecPos) && (mnNextRecPos + 4 <= mnStreamSize);
    if (bRet)
    {
        mrStrm.ReadUInt16( mnRawRecId ).ReadUInt16( mnRawRecSize );
        bRet = mrStrm.good();
    }
    return bRet;
}

void XclImpStream::SetupDecrypter()
{
    if( mxDecrypter )
        mxDecrypter->Update( mrStrm, mnRawRecSize );
}

void XclImpStream::SetupRawRecord()
{
    // pre: mnRawRecSize contains current raw record size
    // pre: mrStrm points to start of raw record data
    mnNextRecPos = mrStrm.Tell() + mnRawRecSize;
    mnRawRecLeft = mnRawRecSize;
    mnCurrRecSize += mnRawRecSize;
    SetupDecrypter();   // decrypter works on raw record level
}

void XclImpStream::SetupRecord()
{
    mnRecId = mnRawRecId;
    mnAltContId = EXC_ID_UNKNOWN;
    mnCurrRecSize = 0;
    mnComplRecSize = mnRawRecSize;
    mbHasComplRec = !mbCont;
    SetupRawRecord();
    SetNulSubstChar();
    EnableDecryption();
    StorePosition( maFirstRec );
}

bool XclImpStream::IsContinueId( sal_uInt16 nRecId ) const
{
    return (nRecId == EXC_ID_CONT) || (nRecId == mnAltContId);
}

bool XclImpStream::JumpToNextContinue()
{
    mbValid = mbValid && mbCont && ReadNextRawRecHeader() && IsContinueId( mnRawRecId );
    if( mbValid )   // do not setup a following non-CONTINUE record
        SetupRawRecord();
    return mbValid;
}

bool XclImpStream::JumpToNextStringContinue( bool& rb16Bit )
{
    OSL_ENSURE( mnRawRecLeft == 0, "XclImpStream::JumpToNextStringContinue - unexpected garbage" );

    if( mbCont && (GetRecLeft() > 0) )
    {
        JumpToNextContinue();
    }
    else if( mnRecId == EXC_ID_CONT )
    {
        // CONTINUE handling is off, but we have started reading in a CONTINUE record
        // -> start next CONTINUE for TXO import
        mbValidRec = ReadNextRawRecHeader() && ((mnRawRecId != 0) || (mnRawRecSize > 0));
        mbValid = mbValidRec && (mnRawRecId == EXC_ID_CONT);
        // we really start a new record here - no chance to return to string origin
        if( mbValid )
            SetupRecord();
    }
    else
        mbValid = false;

    if( mbValid )
        rb16Bit = ::get_flag( ReaduInt8(), EXC_STRF_16BIT );
    return mbValid;
}

bool XclImpStream::EnsureRawReadSize( sal_uInt16 nBytes )
{
    if( mbValid && nBytes )
    {
        while( mbValid && !mnRawRecLeft ) JumpToNextContinue();
        mbValid = mbValid && (nBytes <= mnRawRecLeft);
        OSL_ENSURE( mbValid, "XclImpStream::EnsureRawReadSize - record overread" );
    }
    return mbValid;
}

sal_uInt16 XclImpStream::GetMaxRawReadSize( std::size_t nBytes ) const
{
    return static_cast< sal_uInt16 >( ::std::min< std::size_t >( nBytes, mnRawRecLeft ) );
}

sal_uInt16 XclImpStream::ReadRawData( void* pData, sal_uInt16 nBytes )
{
    OSL_ENSURE( (nBytes <= mnRawRecLeft), "XclImpStream::ReadRawData - record overread" );
    sal_uInt16 nRet = 0;
    if( mbUseDecr )
        nRet = mxDecrypter->Read( mrStrm, pData, nBytes );
    else
        nRet = static_cast<sal_uInt16>(mrStrm.ReadBytes(pData, nBytes));
    mnRawRecLeft = mnRawRecLeft - nRet;
    return nRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
