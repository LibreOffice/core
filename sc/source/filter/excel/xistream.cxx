/*************************************************************************
 *
 *  $RCSfile: xistream.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: obo $ $Date: 2004-08-11 09:53:27 $
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

// ============================================================================

#ifndef SC_XISTREAM_HXX
#include "xistream.hxx"
#endif

#ifndef SC_XIROOT_HXX
#include "xiroot.hxx"
#endif

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

void XclImpDecrypter::Update( SvStream& rStrm, sal_uInt16 nRecSize )
{
    if( IsValid() )
    {
        ULONG nNewPos = rStrm.Tell();
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

void XclImpDecrypter::SetHasValidPassword( bool bValid )
{
    mnError = bValid ? ERRCODE_NONE : EXC_ENCR_ERROR_WRONG_PASS;
}

// ----------------------------------------------------------------------------

XclImpBiff5Decrypter::XclImpBiff5Decrypter( const XclImpRoot& rRoot, sal_uInt16 nKey, sal_uInt16 nHash )
{
    Init( XclCryptoHelper::GetBiff5WbProtPassword(), nKey, nHash );
    if( !IsValid() )
    {
        //! TODO: correct byte string encoding in all cases?
        ByteString aPass( rRoot.QueryPassword(), RTL_TEXTENCODING_MS_1252 );
        Init( aPass, nKey, nHash );
    }
}

XclImpBiff5Decrypter::XclImpBiff5Decrypter( const XclImpBiff5Decrypter& rSrc ) :
    XclImpDecrypter( rSrc )
{
    if( rSrc.IsValid() )
    {
        memcpy( mpnPassw, rSrc.mpnPassw, sizeof( mpnPassw ) );
        maCodec.InitKey( mpnPassw );
    }
}

XclImpBiff5Decrypter* XclImpBiff5Decrypter::OnClone() const
{
    return new XclImpBiff5Decrypter( *this );
}

void XclImpBiff5Decrypter::OnUpdate( ULONG nOldStrmPos, ULONG nNewStrmPos, sal_uInt16 nRecSize )
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

void XclImpBiff5Decrypter::Init( const ByteString& rPass, sal_uInt16 nKey, sal_uInt16 nHash )
{
    xub_StrLen nLen = rPass.Len();
    bool bValid = (0 < nLen) && (nLen < 16);

    if( bValid )
    {
        // transform ByteString to sal_uInt8 array
        memset( mpnPassw, 0, sizeof( mpnPassw ) );
        for( xub_StrLen nChar = 0; nChar < nLen; ++nChar )
            mpnPassw[ nChar ] = static_cast< sal_uInt8 >( rPass.GetChar( nChar ) );
        // init codec
        maCodec.InitKey( mpnPassw );
        bValid = maCodec.VerifyKey( nKey, nHash );
    }

    SetHasValidPassword( bValid );
}

// ----------------------------------------------------------------------------

XclImpBiff8Decrypter::XclImpBiff8Decrypter(
        const XclImpRoot& rRoot, sal_uInt8 pnDocId[ 16 ],
        sal_uInt8 pnSaltData[ 16 ], sal_uInt8 pnSaltHash[ 16 ] )
{
    Init( XclCryptoHelper::GetBiff8WbProtPassword(), pnDocId, pnSaltData, pnSaltHash );
    if( !IsValid() )
        Init( rRoot.QueryPassword(), pnDocId, pnSaltData, pnSaltHash );
}

XclImpBiff8Decrypter::XclImpBiff8Decrypter( const XclImpBiff8Decrypter& rSrc ) :
    XclImpDecrypter( rSrc )
{
    if( rSrc.IsValid() )
    {
        memcpy( mpnPassw, rSrc.mpnPassw, sizeof( mpnPassw ) );
        memcpy( mpnDocId, rSrc.mpnDocId, sizeof( mpnDocId ) );
        maCodec.InitKey( mpnPassw, mpnDocId );
    }
}

XclImpBiff8Decrypter* XclImpBiff8Decrypter::OnClone() const
{
    return new XclImpBiff8Decrypter( *this );
}

void XclImpBiff8Decrypter::OnUpdate( ULONG nOldStrmPos, ULONG nNewStrmPos, sal_uInt16 nRecSize )
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
        nRet += static_cast< sal_uInt16 >( rStrm.Read( pnCurrData, nDecBytes ) );
        // decode the block inplace
        maCodec.Decode( pnCurrData, nDecBytes, pnCurrData, nDecBytes );
        if( GetOffset( rStrm.Tell() ) == 0 )
            maCodec.InitCipher( GetBlock( rStrm.Tell() ) );

        pnCurrData += nDecBytes;
        nBytesLeft -= nDecBytes;
    }

    return nRet;
}

void XclImpBiff8Decrypter::Init(
        const String& rPass, sal_uInt8 pnDocId[ 16 ],
        sal_uInt8 pnSaltData[ 16 ], sal_uInt8 pnSaltHash[ 16 ] )
{
    xub_StrLen nLen = rPass.Len();
    bool bValid = (0 < nLen) && (nLen < 16);

    if( bValid )
    {
        // transform String to sal_uInt16 array
        memset( mpnPassw, 0, sizeof( mpnPassw ) );
        for( xub_StrLen nChar = 0; nChar < nLen; ++nChar )
            mpnPassw[ nChar ] = static_cast< sal_uInt16 >( rPass.GetChar( nChar ) );
        // copy document ID
        memcpy( mpnDocId, pnDocId, sizeof( mpnDocId ) );
        // init codec
        maCodec.InitKey( mpnPassw, mpnDocId );
        bValid = maCodec.VerifyKey( pnSaltData, pnSaltHash );
    }

    SetHasValidPassword( bValid );
}

sal_uInt32 XclImpBiff8Decrypter::GetBlock( ULONG nStrmPos ) const
{
    return static_cast< sal_uInt32 >( nStrmPos / EXC_ENCR_BLOCKSIZE );
}

sal_uInt16 XclImpBiff8Decrypter::GetOffset( ULONG nStrmPos ) const
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
        const SvStream& rStrm, ULONG nNextPos, sal_uInt32 nCurrSize,
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
        SvStream& rStrm, ULONG& rnNextPos, sal_uInt32& rnCurrSize,
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
    XclBiff eBiff = xlBiffUnknown;

    rStrm.Seek( STREAM_SEEK_TO_BEGIN );
    sal_uInt16 nBofId, nBofSize;
    rStrm >> nBofId >> nBofSize;

    switch( nBofId )
    {
        case EXC_ID2_BOF:
            eBiff = xlBiff2;
            DBG_ASSERT( nBofSize == 4, "XclImpStream::DetectBiffVersion - wrong BIFF2 BOF size" );
        break;
        case EXC_ID3_BOF:
            eBiff = xlBiff3;
            DBG_ASSERT( nBofSize == 6, "XclImpStream::DetectBiffVersion - wrong BIFF3 BOF size" );
        break;
        case EXC_ID4_BOF:
            eBiff = xlBiff4;
            DBG_ASSERT( nBofSize == 6, "XclImpStream::DetectBiffVersion - wrong BIFF4 BOF size" );
        break;
        case EXC_ID5_BOF:
        {
            sal_uInt16 nVersion;
            rStrm >> nVersion;
            switch( nVersion )
            {
                case EXC_BOF_BIFF5:
                    eBiff = xlBiff5;
                    DBG_ASSERT( nBofSize == 8, "XclImpStream::DetectBiffVersion - wrong BIFF5 BOF size" );
                break;
                case EXC_BOF_BIFF8:
                    eBiff = xlBiff8;
                    DBG_ASSERT( nBofSize == 16, "XclImpStream::DetectBiffVersion - wrong BIFF8 BOF size" );
                break;
                default:
                    DBG_ERRORFILE( "XclImpStream::DetectBiffVersion - unknown BIFF version" );
            }
        }
        break;
        default:
            DBG_ERRORFILE( "XclImpStream::DetectBiffVersion - no BIFF stream" );
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
    mnStreamSize = mrStrm.Seek( STREAM_SEEK_TO_END );
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
    sal_uInt32 nZeroRecCount = 5;
    bool bIsZeroRec = false;

    do
    {
        mbValidRec = ReadNextRawRecHeader();
        bIsZeroRec = !mnRawRecId && !mnRawRecSize;
        if( bIsZeroRec ) --nZeroRecCount;
        mnNextRecPos = mrStrm.Tell() + mnRawRecSize;
    }
    while( mbValidRec && ((mbCont && IsContinueId( mnRawRecId )) || (bIsZeroRec && nZeroRecCount)) );

    mbValidRec = mbValidRec && !bIsZeroRec;
    mbValid = mbValidRec;
    SetupRecord();

    return mbValidRec;
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

void XclImpStream::SetDecrypter( XclImpDecrypterRef xDecrypter )
{
    mxDecrypter = xDecrypter;
    EnableDecryption();
    SetupDecrypter();
}

void XclImpStream::CopyDecrypterFrom( const XclImpStream& rStrm )
{
    XclImpDecrypterRef xNewDecr;
    if( rStrm.mxDecrypter.get() )
        xNewDecr = rStrm.mxDecrypter->Clone();
    SetDecrypter( xNewDecr );
}

bool XclImpStream::HasValidDecrypter() const
{
    return mxDecrypter.get() && mxDecrypter->IsValid();
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

void XclImpStream::RejectPosition()
{
    DBG_ASSERT( !maPosStack.empty(), "XclImpStream::RejectPosition - stack empty" );
    if( !maPosStack.empty() )
        maPosStack.pop_back();
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

sal_uInt32 XclImpStream::GetRecPos() const
{
    return mbValid ? (mnCurrRecSize - mnRawRecLeft) : EXC_REC_SEEK_TO_END;
}

sal_uInt32 XclImpStream::GetRecSize()
{
    if( !mbHasComplRec )
    {
        PushPosition();
        while( JumpToNextContinue() );  // JumpToNextContinue() adds up mnCurrRecSize
        mnComplRecSize = mnCurrRecSize;
        mbHasComplRec = true;
        PopPosition();
    }
    return mnComplRecSize;
}

sal_uInt32 XclImpStream::GetRecLeft()
{
    return mbValid ? (GetRecSize() - GetRecPos()) : 0;
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
            rnValue = static_cast< sal_Int32 >( SVBT32ToLong( pnBuffer ) );
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
            rnValue = SVBT32ToLong( pnBuffer );
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
            sal_uInt32 nValue = SVBT32ToLong( pnBuffer );
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
    sal_Int8 nValue;
    operator>>( nValue );
    return nValue;
}

sal_uInt8 XclImpStream::ReaduInt8()
{
    sal_uInt8 nValue;
    operator>>( nValue );
    return nValue;
}

sal_Int16 XclImpStream::ReadInt16()
{
    sal_Int16 nValue;
    operator>>( nValue );
    return nValue;
}

sal_uInt16 XclImpStream::ReaduInt16()
{
    sal_uInt16 nValue;
    operator>>( nValue );
    return nValue;
}

sal_Int32 XclImpStream::ReadInt32()
{
    sal_Int32 nValue;
    operator>>( nValue );
    return nValue;
}

sal_uInt32 XclImpStream::ReaduInt32()
{
    sal_uInt32 nValue;
    operator>>( nValue );
    return nValue;
}

float XclImpStream::ReadFloat()
{
    float fValue;
    operator>>( fValue );
    return fValue;
}

double XclImpStream::ReadDouble()
{
    double fValue;
    operator>>( fValue );
    return fValue;
}

sal_uInt32 XclImpStream::Read( void* pData, sal_uInt32 nBytes )
{
    sal_uInt32 nRet = 0;
    if( mbValid && pData && nBytes )
    {
        sal_uInt8* pnBuffer = reinterpret_cast< sal_uInt8* >( pData );
        sal_uInt32 nBytesLeft = nBytes;

        while( mbValid && nBytesLeft )
        {
            sal_uInt16 nReadSize = GetMaxRawReadSize( nBytesLeft );
            sal_uInt16 nReadRet = ReadRawData( pnBuffer, nReadSize );
            nRet += nReadRet;
            mbValid = (nReadSize == nReadRet);
            DBG_ASSERT( mbValid, "XclImpStream::Read - stream read error" );
            pnBuffer += nReadRet;
            nBytesLeft -= nReadRet;
            if( mbValid && nBytesLeft )
                JumpToNextContinue();
            DBG_ASSERT( mbValid, "XclImpStream::Read - record overread" );
        }
    }
    return nRet;
}

sal_uInt32 XclImpStream::CopyToStream( SvStream& rOutStrm, sal_uInt32 nBytes )
{
    sal_uInt32 nRet = 0;
    if( mbValid && nBytes )
    {
        const sal_uInt32 nMaxBuffer = 0x1000;
        sal_uInt8* pnBuffer = new sal_uInt8[ ::std::min( nBytes, nMaxBuffer ) ];
        sal_uInt32 nBytesLeft = nBytes;

        while( mbValid && nBytesLeft )
        {
            sal_uInt32 nReadSize = ::std::min( nBytesLeft, nMaxBuffer );
            nRet += Read( pnBuffer, nReadSize );
            rOutStrm.Write( pnBuffer, static_cast< ULONG >( nReadSize ) );
            nBytesLeft -= nReadSize;
        }

        delete[] pnBuffer;
    }
    return nRet;
}

sal_uInt32 XclImpStream::CopyRecordToStream( SvStream& rOutStrm )
{
    sal_uInt32 nRet = 0;
    if( mbValidRec )
    {
        PushPosition();
        RestorePosition( maFirstRec );
        nRet = CopyToStream( rOutStrm, GetRecSize() );
        PopPosition();
    }
    return nRet;
}

void XclImpStream::Seek( sal_uInt32 nPos )
{
    if( mbValidRec )
    {
        sal_uInt32 nCurrPos = GetRecPos();
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

void XclImpStream::Ignore( sal_uInt32 nBytes )
{
    // implementation similar to Read(), but without really reading anything
    sal_uInt32 nBytesLeft = nBytes;
    while( mbValid && nBytesLeft )
    {
        sal_uInt16 nReadSize = GetMaxRawReadSize( nBytesLeft );
        mrStrm.SeekRel( nReadSize );
        mnRawRecLeft -= nReadSize;
        nBytesLeft -= nReadSize;
        if( nBytesLeft )
            JumpToNextContinue();
        DBG_ASSERT( mbValid, "XclImpStream::Ignore - record overread" );
    }
}

// ----------------------------------------------------------------------------

sal_uInt32 XclImpStream::ReadUniStringExtHeader(
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

sal_uInt32 XclImpStream::ReadUniStringExtHeader( bool& rb16Bit, sal_uInt8 nFlags )
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

    while( IsValid() && nCharsLeft )
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

        nCharsLeft -= nReadSize;
        if( nCharsLeft )
            JumpToNextStringContinue( b16Bit );
    }

    delete[] pcBuffer;
    return aRet;
}

String XclImpStream::ReadUniString( sal_uInt16 nChars, sal_uInt8 nFlags )
{
    bool b16Bit;
    sal_uInt32 nExtSize = ReadUniStringExtHeader( b16Bit, nFlags );
    String aRet( ReadRawUniString( nChars, b16Bit ) );
    SkipUniStringExtData( nExtSize );
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

    while( IsValid() && nCharsLeft )
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

        nCharsLeft -= nReadSize;
        if( nCharsLeft )
            JumpToNextStringContinue( b16Bit );
    }
}

void XclImpStream::IgnoreUniString( sal_uInt16 nChars, sal_uInt8 nFlags )
{
    bool b16Bit;
    sal_uInt32 nExtSize = ReadUniStringExtHeader( b16Bit, nFlags );
    IgnoreRawUniString( nChars, b16Bit );
    SkipUniStringExtData( nExtSize );
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
    String aRet( pcBuffer, mrRoot.GetCharSet() );
    delete[] pcBuffer;
    return aRet;
}

String XclImpStream::ReadByteString( bool b16BitLen )
{
    return ReadRawByteString( ReadByteStrLen( b16BitLen ) );
}

void XclImpStream::IgnoreRawByteString( sal_uInt16 nChars )
{
    Ignore( nChars );
}

void XclImpStream::IgnoreByteString( bool b16BitLen )
{
    IgnoreRawByteString( ReadByteStrLen( b16BitLen ) );
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
    bool bRet = (mnNextRecPos < mnStreamSize);
    if( bRet )
        mrStrm >> mnRawRecId >> mnRawRecSize;
    return bRet;
}

void XclImpStream::SetupDecrypter()
{
    if( mxDecrypter.get() )
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
    mbValid = mbValid && (mbCont || IsContinueId( mnRecId ));
    if( mbValid )
        mbValid = ReadNextRawRecHeader() && IsContinueId( mnRawRecId );
    if( mbValid )   // do not setup a following non-CONTINUE record
        SetupRawRecord();
    return mbValid;
}

bool XclImpStream::JumpToNextStringContinue( bool& rb16Bit )
{
    DBG_ASSERT( !mnRawRecLeft, "XclImpStream::JumpToNextStringContinue - unexpected garbage" );

    if( mbCont && GetRecLeft() )
    {
        JumpToNextContinue();
    }
    else if( mnRecId == EXC_ID_CONT )
    {
        // CONTINUE handling is off, but we have started reading in a CONTINUE record
        // -> start next CONTINUE for TXO import
        mbValidRec = ReadNextRawRecHeader() && (mnRawRecId || mnRawRecSize);
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

sal_uInt16 XclImpStream::GetMaxRawReadSize( sal_uInt32 nBytes ) const
{
    return static_cast< sal_uInt16 >( ::std::min< sal_uInt32 >( nBytes, mnRawRecLeft ) );
}

sal_uInt16 XclImpStream::ReadRawData( void* pData, sal_uInt16 nBytes )
{
    DBG_ASSERT( (nBytes <= mnRawRecLeft), "XclImpStream::ReadRawData - record overread" );
    sal_uInt16 nRet = 0;
    if( mbUseDecr )
        nRet = mxDecrypter->Read( mrStrm, pData, nBytes );
    else
        nRet = static_cast< sal_uInt16 >( mrStrm.Read( pData, nBytes ) );
    mnRawRecLeft -= nRet;
    return nRet;
}

// ============================================================================

