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
#include "precompiled_sc.hxx"

#include <comphelper/docpasswordhelper.hxx>
#include <comphelper/sequenceashashmap.hxx>

#include "xistream.hxx"
#include "xlstring.hxx"
#include "xiroot.hxx"

#include <vector>

using ::rtl::OString;
using ::rtl::OUString;
using ::rtl::OUStringToOString;

using namespace ::com::sun::star;

// ============================================================================
// Decryption
// ============================================================================

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

::comphelper::DocPasswordVerifierResult XclImpDecrypter::verifyPassword( const ::rtl::OUString& rPassword, uno::Sequence< beans::NamedValue >& o_rEncryptionData )
{
    o_rEncryptionData = OnVerifyPassword( rPassword );
    mnError = o_rEncryptionData.getLength() ? ERRCODE_NONE : ERRCODE_ABORT;
    return o_rEncryptionData.getLength() ? ::comphelper::DocPasswordVerifierResult_OK : ::comphelper::DocPasswordVerifierResult_WRONG_PASSWORD;
}

::comphelper::DocPasswordVerifierResult XclImpDecrypter::verifyEncryptionData( const uno::Sequence< beans::NamedValue >& rEncryptionData )
{
    bool bValid = OnVerifyEncryptionData( rEncryptionData );
    mnError = bValid ? ERRCODE_NONE : ERRCODE_ABORT;
    return bValid ? ::comphelper::DocPasswordVerifierResult_OK : ::comphelper::DocPasswordVerifierResult_WRONG_PASSWORD;
}

void XclImpDecrypter::Update( SvStream& rStrm, sal_uInt16 nRecSize )
{
    if( IsValid() )
    {
        sal_Size nNewPos = rStrm.Tell();
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
            nRet = OnRead( rStrm, reinterpret_cast< sal_uInt8* >( pData ), nBytes );
            mnOldPos = rStrm.Tell();
        }
        else
            nRet = static_cast< sal_uInt16 >( rStrm.Read( pData, nBytes ) );
    }
    return nRet;
}

// ----------------------------------------------------------------------------

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

uno::Sequence< beans::NamedValue > XclImpBiff5Decrypter::OnVerifyPassword( const ::rtl::OUString& rPassword )
{
    maEncryptionData.realloc( 0 );

    /*  Convert password to a byte string. TODO: this needs some finetuning
        according to the spec... */
    OString aBytePassword = OUStringToOString( rPassword, osl_getThreadTextEncoding() );
    sal_Int32 nLen = aBytePassword.getLength();
    if( (0 < nLen) && (nLen < 16) )
    {
        // init codec
        maCodec.InitKey( (sal_uInt8*)aBytePassword.getStr() );

        if ( maCodec.VerifyKey( mnKey, mnHash ) )
        {
            maEncryptionData = maCodec.GetEncryptionData();

            // since the export uses Std97 encryption always we have to request it here
            ::std::vector< sal_uInt16 > aPassVect( 16 );
            ::std::vector< sal_uInt16 >::iterator aIt = aPassVect.begin();
            for( sal_Int32 nInd = 0; nInd < nLen; ++nInd, ++aIt )
                *aIt = static_cast< sal_uInt16 >( rPassword.getStr()[nInd] );

            uno::Sequence< sal_Int8 > aDocId = ::comphelper::DocPasswordHelper::GenerateRandomByteSequence( 16 );
            OSL_ENSURE( aDocId.getLength() == 16, "Unexpected length of the senquence!" );

            ::msfilter::MSCodec_Std97 aCodec97;
            aCodec97.InitKey( &aPassVect.front(), (sal_uInt8*)aDocId.getConstArray() );

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

void XclImpBiff5Decrypter::OnUpdate( sal_Size /*nOldStrmPos*/, sal_Size nNewStrmPos, sal_uInt16 nRecSize )
{
    maCodec.InitCipher();
    maCodec.Skip( (nNewStrmPos + nRecSize) & 0x0F );
}

sal_uInt16 XclImpBiff5Decrypter::OnRead( SvStream& rStrm, sal_uInt8* pnData, sal_uInt16 nBytes )
{
    sal_uInt16 nRet = static_cast< sal_uInt16 >( rStrm.Read( pnData, nBytes ) );
    maCodec.Decode( pnData, nRet );
    return nRet;
}

// ----------------------------------------------------------------------------

XclImpBiff8Decrypter::XclImpBiff8Decrypter( sal_uInt8 pnSalt[ 16 ],
        sal_uInt8 pnVerifier[ 16 ], sal_uInt8 pnVerifierHash[ 16 ] ) :
    maSalt( pnSalt, pnSalt + 16 ),
    maVerifier( pnVerifier, pnVerifier + 16 ),
    maVerifierHash( pnVerifierHash, pnVerifierHash + 16 )
{
}

XclImpBiff8Decrypter::XclImpBiff8Decrypter( const XclImpBiff8Decrypter& rSrc ) :
    XclImpDecrypter( rSrc ),
    maEncryptionData( rSrc.maEncryptionData ),
    maSalt( rSrc.maSalt ),
    maVerifier( rSrc.maVerifier ),
    maVerifierHash( rSrc.maVerifierHash )
{
    if( IsValid() )
        maCodec.InitCodec( maEncryptionData );
}

XclImpBiff8Decrypter* XclImpBiff8Decrypter::OnClone() const
{
    return new XclImpBiff8Decrypter( *this );
}

uno::Sequence< beans::NamedValue > XclImpBiff8Decrypter::OnVerifyPassword( const ::rtl::OUString& rPassword )
{
    maEncryptionData.realloc( 0 );

    sal_Int32 nLen = rPassword.getLength();
    if( (0 < nLen) && (nLen < 16) )
    {
        // copy string to sal_uInt16 array
        ::std::vector< sal_uInt16 > aPassVect( 16 );
        const sal_Unicode* pcChar = rPassword.getStr();
        const sal_Unicode* pcCharEnd = pcChar + nLen;
        ::std::vector< sal_uInt16 >::iterator aIt = aPassVect.begin();
        for( ; pcChar < pcCharEnd; ++pcChar, ++aIt )
            *aIt = static_cast< sal_uInt16 >( *pcChar );

        // init codec
        maCodec.InitKey( &aPassVect.front(), &maSalt.front() );
        if ( maCodec.VerifyKey( &maVerifier.front(), &maVerifierHash.front() ) )
            maEncryptionData = maCodec.GetEncryptionData();
    }

    return maEncryptionData;
}

bool XclImpBiff8Decrypter::OnVerifyEncryptionData( const uno::Sequence< beans::NamedValue >& rEncryptionData )
{
    maEncryptionData.realloc( 0 );

    if( rEncryptionData.getLength() )
    {
        // init codec
        maCodec.InitCodec( rEncryptionData );

        if ( maCodec.VerifyKey( &maVerifier.front(), &maVerifierHash.front() ) )
            maEncryptionData = rEncryptionData;
    }

    return maEncryptionData.getLength();
}

void XclImpBiff8Decrypter::OnUpdate( sal_Size nOldStrmPos, sal_Size nNewStrmPos, sal_uInt16 /*nRecSize*/ )
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
            maCodec.InitCipher( nNewBlock );
            nOldOffset = 0;     // reset nOldOffset for next if() statement
        }

        /*  Seek to correct offset. */
        if( nNewOffset > nOldOffset )
            maCodec.Skip( nNewOffset - nOldOffset );
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
        nRet = nRet + static_cast< sal_uInt16 >( rStrm.Read( pnCurrData, nDecBytes ) );
        // decode the block inplace
        maCodec.Decode( pnCurrData, nDecBytes, pnCurrData, nDecBytes );
        if( GetOffset( rStrm.Tell() ) == 0 )
            maCodec.InitCipher( GetBlock( rStrm.Tell() ) );

        pnCurrData += nDecBytes;
        nBytesLeft = nBytesLeft - nDecBytes;
    }

    return nRet;
}

sal_uInt32 XclImpBiff8Decrypter::GetBlock( sal_Size nStrmPos ) const
{
    return static_cast< sal_uInt32 >( nStrmPos / EXC_ENCR_BLOCKSIZE );
}

sal_uInt16 XclImpBiff8Decrypter::GetOffset( sal_Size nStrmPos ) const
{
    return static_cast< sal_uInt16 >( nStrmPos % EXC_ENCR_BLOCKSIZE );
}

// ============================================================================
// Stream
// ============================================================================

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
        const SvStream& rStrm, sal_Size nNextPos, sal_Size nCurrSize,
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
        SvStream& rStrm, sal_Size& rnNextPos, sal_Size& rnCurrSize,
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

// ============================================================================

XclBiff XclImpStream::DetectBiffVersion( SvStream& rStrm )
{
    XclBiff eBiff = EXC_BIFF_UNKNOWN;

    rStrm.Seek( STREAM_SEEK_TO_BEGIN );
    sal_uInt16 nBofId, nBofSize;
    rStrm >> nBofId >> nBofSize;

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
            rStrm >> nVersion;
            // #i23425# #i44031# #i62752# there are some *really* broken documents out there...
            switch( nVersion & 0xFF00 )
            {
                case 0:             eBiff = EXC_BIFF5;  break;  // #i44031# #i62752#
                case EXC_BOF_BIFF2: eBiff = EXC_BIFF2;  break;
                case EXC_BOF_BIFF3: eBiff = EXC_BIFF3;  break;
                case EXC_BOF_BIFF4: eBiff = EXC_BIFF4;  break;
                case EXC_BOF_BIFF5: eBiff = EXC_BIFF5;  break;
                case EXC_BOF_BIFF8: eBiff = EXC_BIFF8;  break;
                default:    DBG_ERROR1( "XclImpStream::DetectBiffVersion - unknown BIFF version: 0x%04hX", nVersion );
            }
        }
        break;
    }
    return eBiff;
}

XclImpStream::XclImpStream( SvStream& rInStrm, const XclImpRoot& rRoot, bool bContLookup ) :
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
    mbCont( bContLookup ),
    mbUseDecr( false ),
    mbValidRec( false ),
    mbValid( false )
{
    mrStrm.Seek( STREAM_SEEK_TO_END );
    mnStreamSize = mrStrm.Tell();
    mrStrm.Seek( STREAM_SEEK_TO_BEGIN );
    DBG_ASSERT( mnStreamSize < STREAM_SEEK_TO_END, "XclImpStream::XclImpStream - stream error" );
}

XclImpStream::~XclImpStream()
{
}

bool XclImpStream::StartNextRecord()
{
    maPosStack.clear();

    /*  #i4266# Counter to ignore zero records (id==len==0) (i.e. the application
        "Crystal Report" writes zero records between other records) */
    sal_Size nZeroRecCount = 5;
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

bool XclImpStream::StartNextRecord( sal_Size nNextRecPos )
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

void XclImpStream::SetDecrypter( XclImpDecrypterRef xDecrypter )
{
    mxDecrypter = xDecrypter;
    EnableDecryption();
    SetupDecrypter();
}

void XclImpStream::CopyDecrypterFrom( const XclImpStream& rStrm )
{
    XclImpDecrypterRef xNewDecr;
    if( rStrm.mxDecrypter.is() )
        xNewDecr = rStrm.mxDecrypter->Clone();
    SetDecrypter( xNewDecr );
}

bool XclImpStream::HasValidDecrypter() const
{
    return mxDecrypter.is() && mxDecrypter->IsValid();
}

void XclImpStream::EnableDecryption( bool bEnable )
{
    mbUseDecr = bEnable && HasValidDecrypter();
}

// ----------------------------------------------------------------------------

void XclImpStream::PushPosition()
{
    maPosStack.push_back( XclImpStreamPos() );
    StorePosition( maPosStack.back() );
}

void XclImpStream::PopPosition()
{
    DBG_ASSERT( !maPosStack.empty(), "XclImpStream::PopPosition - stack empty" );
    if( !maPosStack.empty() )
    {
        RestorePosition( maPosStack.back() );
        maPosStack.pop_back();
    }
}

//UNUSED2008-05  void XclImpStream::RejectPosition()
//UNUSED2008-05  {
//UNUSED2008-05      DBG_ASSERT( !maPosStack.empty(), "XclImpStream::RejectPosition - stack empty" );
//UNUSED2008-05      if( !maPosStack.empty() )
//UNUSED2008-05          maPosStack.pop_back();
//UNUSED2008-05  }

void XclImpStream::StoreGlobalPosition()
{
    StorePosition( maGlobPos );
    mnGlobRecId = mnRecId;
    mbGlobValidRec = mbValidRec;
    mbHasGlobPos = true;
}

void XclImpStream::SeekGlobalPosition()
{
    DBG_ASSERT( mbHasGlobPos, "XclImpStream::SeekGlobalPosition - no position stored" );
    if( mbHasGlobPos )
    {
        RestorePosition( maGlobPos );
        mnRecId = mnGlobRecId;
        mnComplRecSize = mnCurrRecSize;
        mbHasComplRec = !mbCont;
        mbValidRec = mbGlobValidRec;
    }
}

sal_Size XclImpStream::GetRecPos() const
{
    return mbValid ? (mnCurrRecSize - mnRawRecLeft) : EXC_REC_SEEK_TO_END;
}

sal_Size XclImpStream::GetRecSize()
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

sal_Size XclImpStream::GetRecLeft()
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
            mrStrm >> nRecId;
        }
        PopPosition();
    }
    return nRecId;
}

// ----------------------------------------------------------------------------

XclImpStream& XclImpStream::operator>>( sal_Int8& rnValue )
{
    if( EnsureRawReadSize( 1 ) )
    {
        if( mbUseDecr )
            mxDecrypter->Read( mrStrm, &rnValue, 1 );
        else
            mrStrm >> rnValue;
        --mnRawRecLeft;
    }
    return *this;
}

XclImpStream& XclImpStream::operator>>( sal_uInt8& rnValue )
{
    if( EnsureRawReadSize( 1 ) )
    {
        if( mbUseDecr )
            mxDecrypter->Read( mrStrm, &rnValue, 1 );
        else
            mrStrm >> rnValue;
        --mnRawRecLeft;
    }
    return *this;
}

XclImpStream& XclImpStream::operator>>( sal_Int16& rnValue )
{
    if( EnsureRawReadSize( 2 ) )
    {
        if( mbUseDecr )
        {
            SVBT16 pnBuffer;
            mxDecrypter->Read( mrStrm, pnBuffer, 2 );
            rnValue = static_cast< sal_Int16 >( SVBT16ToShort( pnBuffer ) );
        }
        else
            mrStrm >> rnValue;
        mnRawRecLeft -= 2;
    }
    return *this;
}

XclImpStream& XclImpStream::operator>>( sal_uInt16& rnValue )
{
    if( EnsureRawReadSize( 2 ) )
    {
        if( mbUseDecr )
        {
            SVBT16 pnBuffer;
            mxDecrypter->Read( mrStrm, pnBuffer, 2 );
            rnValue = SVBT16ToShort( pnBuffer );
        }
        else
            mrStrm >> rnValue;
        mnRawRecLeft -= 2;
    }
    return *this;
}

XclImpStream& XclImpStream::operator>>( sal_Int32& rnValue )
{
    if( EnsureRawReadSize( 4 ) )
    {
        if( mbUseDecr )
        {
            SVBT32 pnBuffer;
            mxDecrypter->Read( mrStrm, pnBuffer, 4 );
            rnValue = static_cast< sal_Int32 >( SVBT32ToUInt32( pnBuffer ) );
        }
        else
            mrStrm >> rnValue;
        mnRawRecLeft -= 4;
    }
    return *this;
}

XclImpStream& XclImpStream::operator>>( sal_uInt32& rnValue )
{
    if( EnsureRawReadSize( 4 ) )
    {
        if( mbUseDecr )
        {
            SVBT32 pnBuffer;
            mxDecrypter->Read( mrStrm, pnBuffer, 4 );
            rnValue = SVBT32ToUInt32( pnBuffer );
        }
        else
            mrStrm >> rnValue;
        mnRawRecLeft -= 4;
    }
    return *this;
}

XclImpStream& XclImpStream::operator>>( float& rfValue )
{
    if( EnsureRawReadSize( 4 ) )
    {
        if( mbUseDecr )
        {
            SVBT32 pnBuffer;
            mxDecrypter->Read( mrStrm, pnBuffer, 4 );
            sal_uInt32 nValue = SVBT32ToUInt32( pnBuffer );
            memcpy( &rfValue, &nValue, 4 );
        }
        else
            mrStrm >> rfValue;
        mnRawRecLeft -= 4;
    }
    return *this;
}

XclImpStream& XclImpStream::operator>>( double& rfValue )
{
    if( EnsureRawReadSize( 8 ) )
    {
        if( mbUseDecr )
        {
            SVBT64 pnBuffer;
            mxDecrypter->Read( mrStrm, pnBuffer, 8 );
            rfValue = SVBT64ToDouble( pnBuffer );
        }
        else
            mrStrm >> rfValue;
        mnRawRecLeft -= 8;
    }
    return *this;
}

sal_Int8 XclImpStream::ReadInt8()
{
    sal_Int8 nValue(0);
    operator>>( nValue );
    return nValue;
}

sal_uInt8 XclImpStream::ReaduInt8()
{
    sal_uInt8 nValue(0);
    operator>>( nValue );
    return nValue;
}

sal_Int16 XclImpStream::ReadInt16()
{
    sal_Int16 nValue(0);
    operator>>( nValue );
    return nValue;
}

sal_uInt16 XclImpStream::ReaduInt16()
{
    sal_uInt16 nValue(0);
    operator>>( nValue );
    return nValue;
}

sal_Int32 XclImpStream::ReadInt32()
{
    sal_Int32 nValue(0);
    operator>>( nValue );
    return nValue;
}

sal_uInt32 XclImpStream::ReaduInt32()
{
    sal_uInt32 nValue(0);
    operator>>( nValue );
    return nValue;
}

float XclImpStream::ReadFloat()
{
    float fValue(0.0);
    operator>>( fValue );
    return fValue;
}

double XclImpStream::ReadDouble()
{
    double fValue(0.0);
    operator>>( fValue );
    return fValue;
}

sal_Size XclImpStream::Read( void* pData, sal_Size nBytes )
{
    sal_Size nRet = 0;
    if( mbValid && pData && (nBytes > 0) )
    {
        sal_uInt8* pnBuffer = reinterpret_cast< sal_uInt8* >( pData );
        sal_Size nBytesLeft = nBytes;

        while( mbValid && (nBytesLeft > 0) )
        {
            sal_uInt16 nReadSize = GetMaxRawReadSize( nBytesLeft );
            sal_uInt16 nReadRet = ReadRawData( pnBuffer, nReadSize );
            nRet += nReadRet;
            mbValid = (nReadSize == nReadRet);
            DBG_ASSERT( mbValid, "XclImpStream::Read - stream read error" );
            pnBuffer += nReadRet;
            nBytesLeft -= nReadRet;
            if( mbValid && (nBytesLeft > 0) )
                JumpToNextContinue();
            DBG_ASSERT( mbValid, "XclImpStream::Read - record overread" );
        }
    }
    return nRet;
}

sal_Size XclImpStream::CopyToStream( SvStream& rOutStrm, sal_Size nBytes )
{
    sal_Size nRet = 0;
    if( mbValid && (nBytes > 0) )
    {
        const sal_Size nMaxBuffer = 4096;
        sal_uInt8* pnBuffer = new sal_uInt8[ ::std::min( nBytes, nMaxBuffer ) ];
        sal_Size nBytesLeft = nBytes;

        while( mbValid && (nBytesLeft > 0) )
        {
            sal_Size nReadSize = ::std::min( nBytesLeft, nMaxBuffer );
            nRet += Read( pnBuffer, nReadSize );
            rOutStrm.Write( pnBuffer, nReadSize );
            nBytesLeft -= nReadSize;
        }

        delete[] pnBuffer;
    }
    return nRet;
}

sal_Size XclImpStream::CopyRecordToStream( SvStream& rOutStrm )
{
    sal_Size nRet = 0;
    if( mbValidRec )
    {
        PushPosition();
        RestorePosition( maFirstRec );
        nRet = CopyToStream( rOutStrm, GetRecSize() );
        PopPosition();
    }
    return nRet;
}

void XclImpStream::Seek( sal_Size nPos )
{
    if( mbValidRec )
    {
        sal_Size nCurrPos = GetRecPos();
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

void XclImpStream::Ignore( sal_Size nBytes )
{
    // implementation similar to Read(), but without really reading anything
    sal_Size nBytesLeft = nBytes;
    while( mbValid && (nBytesLeft > 0) )
    {
        sal_uInt16 nReadSize = GetMaxRawReadSize( nBytesLeft );
        mrStrm.SeekRel( nReadSize );
        mnRawRecLeft = mnRawRecLeft - nReadSize;
        nBytesLeft -= nReadSize;
        if( nBytesLeft > 0 )
            JumpToNextContinue();
        DBG_ASSERT( mbValid, "XclImpStream::Ignore - record overread" );
    }
}

// ----------------------------------------------------------------------------

sal_Size XclImpStream::ReadUniStringExtHeader(
        bool& rb16Bit, bool& rbRich, bool& rbFareast,
        sal_uInt16& rnFormatRuns, sal_uInt32& rnExtInf, sal_uInt8 nFlags )
{
    DBG_ASSERT( !::get_flag( nFlags, EXC_STRF_UNKNOWN ), "XclImpStream::ReadUniStringExt - unknown flags" );
    rb16Bit = ::get_flag( nFlags, EXC_STRF_16BIT );
    rbRich = ::get_flag( nFlags, EXC_STRF_RICH );
    rbFareast = ::get_flag( nFlags, EXC_STRF_FAREAST );
    rnFormatRuns = rbRich ? ReaduInt16() : 0;
    rnExtInf = rbFareast ? ReaduInt32() : 0;
    return rnExtInf + 4 * rnFormatRuns;
}

sal_Size XclImpStream::ReadUniStringExtHeader( bool& rb16Bit, sal_uInt8 nFlags )
{
    bool bRich, bFareast;
    sal_uInt16 nCrun;
    sal_uInt32 nExtInf;
    return ReadUniStringExtHeader( rb16Bit, bRich, bFareast, nCrun, nExtInf, nFlags );
}

// ----------------------------------------------------------------------------

String XclImpStream::ReadRawUniString( sal_uInt16 nChars, bool b16Bit )
{
    String aRet;
    sal_uInt16 nCharsLeft = nChars;
    sal_uInt16 nReadSize;

    sal_Unicode* pcBuffer = new sal_Unicode[ nCharsLeft + 1 ];

    while( IsValid() && (nCharsLeft > 0) )
    {
        if( b16Bit )
        {
            nReadSize = ::std::min< sal_uInt16 >( nCharsLeft, mnRawRecLeft / 2 );
            DBG_ASSERT( (nReadSize <= nCharsLeft) || !(mnRawRecLeft & 0x1),
                "XclImpStream::ReadRawUniString - missing a byte" );
        }
        else
            nReadSize = GetMaxRawReadSize( nCharsLeft );

        sal_Unicode* pcUniChar = pcBuffer;
        sal_Unicode* pcEndChar = pcBuffer + nReadSize;

        if( b16Bit )
        {
            sal_uInt16 nReadChar;
            for( ; IsValid() && (pcUniChar < pcEndChar); ++pcUniChar )
            {
                operator>>( nReadChar );
                (*pcUniChar) = (nReadChar == EXC_NUL) ? mcNulSubst : static_cast< sal_Unicode >( nReadChar );
            }
        }
        else
        {
            sal_uInt8 nReadChar;
            for( ; IsValid() && (pcUniChar < pcEndChar); ++pcUniChar )
            {
                operator>>( nReadChar );
                (*pcUniChar) = (nReadChar == EXC_NUL_C) ? mcNulSubst : static_cast< sal_Unicode >( nReadChar );
            }
        }

        *pcEndChar = '\0';
        aRet.Append( pcBuffer );

        nCharsLeft = nCharsLeft - nReadSize;
        if( nCharsLeft > 0 )
            JumpToNextStringContinue( b16Bit );
    }

    delete[] pcBuffer;
    return aRet;
}

String XclImpStream::ReadUniString( sal_uInt16 nChars, sal_uInt8 nFlags )
{
    bool b16Bit;
    sal_Size nExtSize = ReadUniStringExtHeader( b16Bit, nFlags );
    String aRet( ReadRawUniString( nChars, b16Bit ) );
    Ignore( nExtSize );
    return aRet;
}

String XclImpStream::ReadUniString( sal_uInt16 nChars )
{
    return ReadUniString( nChars, ReaduInt8() );
}

String XclImpStream::ReadUniString()
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
            DBG_ASSERT( (nReadSize <= nCharsLeft) || !(mnRawRecLeft & 0x1),
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
    sal_Size nExtSize = ReadUniStringExtHeader( b16Bit, nFlags );
    IgnoreRawUniString( nChars, b16Bit );
    Ignore( nExtSize );
}

void XclImpStream::IgnoreUniString( sal_uInt16 nChars )
{
    IgnoreUniString( nChars, ReaduInt8() );
}

void XclImpStream::IgnoreUniString()
{
    IgnoreUniString( ReaduInt16() );
}

// ----------------------------------------------------------------------------

String XclImpStream::ReadRawByteString( sal_uInt16 nChars )
{
    sal_Char* pcBuffer = new sal_Char[ nChars + 1 ];
    sal_uInt16 nCharsRead = ReadRawData( pcBuffer, nChars );
    pcBuffer[ nCharsRead ] = '\0';
    String aRet( pcBuffer, mrRoot.GetTextEncoding() );
    delete[] pcBuffer;
    return aRet;
}

String XclImpStream::ReadByteString( bool b16BitLen )
{
    return ReadRawByteString( ReadByteStrLen( b16BitLen ) );
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
    mrStrm.Seek( mnNextRecPos );
    bool bRet = mnNextRecPos + 4 <= mnStreamSize;
    if( bRet )
        mrStrm >> mnRawRecId >> mnRawRecSize;
    return bRet;
}

void XclImpStream::SetupDecrypter()
{
    if( mxDecrypter.is() )
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
    DBG_ASSERT( mnRawRecLeft == 0, "XclImpStream::JumpToNextStringContinue - unexpected garbage" );

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
        DBG_ASSERT( mbValid, "XclImpStream::EnsureRawReadSize - record overread" );
    }
    return mbValid;
}

sal_uInt16 XclImpStream::GetMaxRawReadSize( sal_Size nBytes ) const
{
    return static_cast< sal_uInt16 >( ::std::min< sal_Size >( nBytes, mnRawRecLeft ) );
}

sal_uInt16 XclImpStream::ReadRawData( void* pData, sal_uInt16 nBytes )
{
    DBG_ASSERT( (nBytes <= mnRawRecLeft), "XclImpStream::ReadRawData - record overread" );
    sal_uInt16 nRet = 0;
    if( mbUseDecr )
        nRet = mxDecrypter->Read( mrStrm, pData, nBytes );
    else
        nRet = static_cast< sal_uInt16 >( mrStrm.Read( pData, nBytes ) );
    mnRawRecLeft = mnRawRecLeft - nRet;
    return nRet;
}

// ============================================================================

