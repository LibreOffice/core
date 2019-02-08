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

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <utility>

#include <filter/msfilter/util.hxx>
#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/random.h>
#include <sax/fshelper.hxx>
#include <unotools/streamwrap.hxx>
#include <sot/storage.hxx>
#include <tools/urlobj.hxx>

#include <docuno.hxx>
#include <xestream.hxx>
#include <xladdress.hxx>
#include <xlstring.hxx>
#include <xltools.hxx>
#include <xeroot.hxx>
#include <xcl97rec.hxx>
#include <rangelst.hxx>
#include <compiler.hxx>
#include <formulacell.hxx>
#include <tokenarray.hxx>
#include <tokenstringcontext.hxx>
#include <refreshtimerprotector.hxx>
#include <globstr.hrc>
#include <scresid.hxx>
#include <root.hxx>

#include <docsh.hxx>
#include <viewdata.hxx>
#include <excdoc.hxx>

#include <oox/token/tokens.hxx>
#include <oox/token/relationship.hxx>
#include <oox/export/utils.hxx>
#include <formula/grammar.hxx>
#include <oox/ole/vbaexport.hxx>
#include <excelvbaproject.hxx>

#include <com/sun/star/task/XStatusIndicator.hpp>
#include <memory>
#include <comphelper/storagehelper.hxx>

#define DEBUG_XL_ENCRYPTION 0

using ::com::sun::star::uno::XInterface;
using ::std::vector;

using namespace com::sun::star;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::sheet;
using namespace ::com::sun::star::uno;
using namespace ::formula;
using namespace ::oox;

XclExpStream::XclExpStream( SvStream& rOutStrm, const XclExpRoot& rRoot, sal_uInt16 nMaxRecSize ) :
    mrStrm( rOutStrm ),
    mrRoot( rRoot ),
    mbUseEncrypter( false ),
    mnMaxRecSize( nMaxRecSize ),
    mnCurrMaxSize( 0 ),
    mnMaxSliceSize( 0 ),
    mnHeaderSize( 0 ),
    mnCurrSize( 0 ),
    mnSliceSize( 0 ),
    mnPredictSize( 0 ),
    mnLastSizePos( 0 ),
    mbInRec( false )
{
    if( mnMaxRecSize == 0 )
        mnMaxRecSize = (mrRoot.GetBiff() <= EXC_BIFF5) ? EXC_MAXRECSIZE_BIFF5 : EXC_MAXRECSIZE_BIFF8;
    mnMaxContSize = mnMaxRecSize;
}

XclExpStream::~XclExpStream()
{
    mrStrm.Flush();
}

void XclExpStream::StartRecord( sal_uInt16 nRecId, std::size_t nRecSize )
{
    OSL_ENSURE( !mbInRec, "XclExpStream::StartRecord - another record still open" );
    DisableEncryption();
    mnMaxContSize = mnCurrMaxSize = mnMaxRecSize;
    mnPredictSize = nRecSize;
    mbInRec = true;
    InitRecord( nRecId );
    SetSliceSize( 0 );
    EnableEncryption();
}

void XclExpStream::EndRecord()
{
    OSL_ENSURE( mbInRec, "XclExpStream::EndRecord - no record open" );
    DisableEncryption();
    UpdateRecSize();
    mrStrm.Seek( STREAM_SEEK_TO_END );
    mbInRec = false;
}

void XclExpStream::SetSliceSize( sal_uInt16 nSize )
{
    mnMaxSliceSize = nSize;
    mnSliceSize = 0;
}

XclExpStream& XclExpStream::operator<<( sal_Int8 nValue )
{
    PrepareWrite( 1 );
    if (mbUseEncrypter && HasValidEncrypter())
        mxEncrypter->Encrypt(mrStrm, nValue);
    else
        mrStrm.WriteSChar( nValue );
    return *this;
}

XclExpStream& XclExpStream::operator<<( sal_uInt8 nValue )
{
    PrepareWrite( 1 );
    if (mbUseEncrypter && HasValidEncrypter())
        mxEncrypter->Encrypt(mrStrm, nValue);
    else
        mrStrm.WriteUChar( nValue );
    return *this;
}

XclExpStream& XclExpStream::operator<<( sal_Int16 nValue )
{
    PrepareWrite( 2 );
    if (mbUseEncrypter && HasValidEncrypter())
        mxEncrypter->Encrypt(mrStrm, nValue);
    else
        mrStrm.WriteInt16( nValue );
    return *this;
}

XclExpStream& XclExpStream::operator<<( sal_uInt16 nValue )
{
    PrepareWrite( 2 );
    if (mbUseEncrypter && HasValidEncrypter())
        mxEncrypter->Encrypt(mrStrm, nValue);
    else
        mrStrm.WriteUInt16( nValue );
    return *this;
}

XclExpStream& XclExpStream::operator<<( sal_Int32 nValue )
{
    PrepareWrite( 4 );
    if (mbUseEncrypter && HasValidEncrypter())
        mxEncrypter->Encrypt(mrStrm, nValue);
    else
        mrStrm.WriteInt32( nValue );
    return *this;
}

XclExpStream& XclExpStream::operator<<( sal_uInt32 nValue )
{
    PrepareWrite( 4 );
    if (mbUseEncrypter && HasValidEncrypter())
        mxEncrypter->Encrypt(mrStrm, nValue);
    else
        mrStrm.WriteUInt32( nValue );
    return *this;
}

XclExpStream& XclExpStream::operator<<( float fValue )
{
    PrepareWrite( 4 );
    if (mbUseEncrypter && HasValidEncrypter())
        mxEncrypter->Encrypt(mrStrm, fValue);
    else
        mrStrm.WriteFloat( fValue );
    return *this;
}

XclExpStream& XclExpStream::operator<<( double fValue )
{
    PrepareWrite( 8 );
    if (mbUseEncrypter && HasValidEncrypter())
        mxEncrypter->Encrypt(mrStrm, fValue);
    else
        mrStrm.WriteDouble( fValue );
    return *this;
}

std::size_t XclExpStream::Write( const void* pData, std::size_t nBytes )
{
    std::size_t nRet = 0;
    if( pData && (nBytes > 0) )
    {
        if( mbInRec )
        {
            const sal_uInt8* pBuffer = static_cast< const sal_uInt8* >( pData );
            std::size_t nBytesLeft = nBytes;
            bool bValid = true;

            while( bValid && (nBytesLeft > 0) )
            {
                std::size_t nWriteLen = ::std::min< std::size_t >( PrepareWrite(), nBytesLeft );
                std::size_t nWriteRet = nWriteLen;
                if (mbUseEncrypter && HasValidEncrypter())
                {
                    OSL_ENSURE(nWriteLen > 0, "XclExpStream::Write: write length is 0!");
                    vector<sal_uInt8> aBytes(nWriteLen);
                    memcpy(&aBytes[0], pBuffer, nWriteLen);
                    mxEncrypter->EncryptBytes(mrStrm, aBytes);
                    // TODO: How do I check if all the bytes have been successfully written ?
                }
                else
                {
                    nWriteRet = mrStrm.WriteBytes(pBuffer, nWriteLen);
                    bValid = (nWriteLen == nWriteRet);
                    OSL_ENSURE( bValid, "XclExpStream::Write - stream write error" );
                }
                pBuffer += nWriteRet;
                nRet += nWriteRet;
                nBytesLeft -= nWriteRet;
                UpdateSizeVars( nWriteRet );
            }
        }
        else
            nRet = mrStrm.WriteBytes(pData, nBytes);
    }
    return nRet;
}

void XclExpStream::WriteZeroBytes( std::size_t nBytes )
{
    if( mbInRec )
    {
        std::size_t nBytesLeft = nBytes;
        while( nBytesLeft > 0 )
        {
            std::size_t nWriteLen = ::std::min< std::size_t >( PrepareWrite(), nBytesLeft );
            WriteRawZeroBytes( nWriteLen );
            nBytesLeft -= nWriteLen;
            UpdateSizeVars( nWriteLen );
        }
    }
    else
        WriteRawZeroBytes( nBytes );
}

void XclExpStream::WriteZeroBytesToRecord( std::size_t nBytes )
{
    if (!mbInRec)
        // not in record.
        return;

    for (std::size_t i = 0; i < nBytes; ++i)
        *this << sal_uInt8(0)/*nZero*/;
}

void XclExpStream::CopyFromStream(SvStream& rInStrm, sal_uInt64 const nBytes)
{
    sal_uInt64 const nRemaining(rInStrm.remainingSize());
    sal_uInt64 nBytesLeft = ::std::min(nBytes, nRemaining);
    if( nBytesLeft > 0 )
    {
        const std::size_t nMaxBuffer = 4096;
        std::unique_ptr<sal_uInt8[]> pBuffer(
            new sal_uInt8[ ::std::min<std::size_t>(nBytesLeft, nMaxBuffer) ]);
        bool bValid = true;

        while( bValid && (nBytesLeft > 0) )
        {
            std::size_t nWriteLen = ::std::min<std::size_t>(nBytesLeft, nMaxBuffer);
            rInStrm.ReadBytes(pBuffer.get(), nWriteLen);
            std::size_t nWriteRet = Write( pBuffer.get(), nWriteLen );
            bValid = (nWriteLen == nWriteRet);
            nBytesLeft -= nWriteRet;
        }
    }
}

void XclExpStream::WriteUnicodeBuffer( const ScfUInt16Vec& rBuffer, sal_uInt8 nFlags )
{
    SetSliceSize( 0 );
    nFlags &= EXC_STRF_16BIT;   // repeat only 16bit flag
    sal_uInt16 nCharLen = nFlags ? 2 : 1;

    for( const auto& rItem : rBuffer )
    {
        if( mbInRec && (mnCurrSize + nCharLen > mnCurrMaxSize) )
        {
            StartContinue();
            operator<<( nFlags );
        }
        if( nCharLen == 2 )
            operator<<( rItem );
        else
            operator<<( static_cast< sal_uInt8 >( rItem ) );
    }
}

// Xcl has an obscure sense of whether starting a new record or not,
// and crashes if it encounters the string header at the very end of a record.
// Thus we add 1 to give some room, seems like they do it that way but with another count (10?)
void XclExpStream::WriteByteString( const OString& rString )
{
    SetSliceSize( 0 );
    std::size_t nLen = ::std::min< std::size_t >( rString.getLength(), 0x00FF );
    nLen = ::std::min< std::size_t >( nLen, 0xFF );

    sal_uInt16 nLeft = PrepareWrite();
    if( mbInRec && (nLeft <= 1) )
        StartContinue();

    operator<<( static_cast< sal_uInt8 >( nLen ) );
    Write( rString.getStr(), nLen );
}

void XclExpStream::WriteCharBuffer( const ScfUInt8Vec& rBuffer )
{
    SetSliceSize( 0 );
    Write( &rBuffer[ 0 ], rBuffer.size() );
}

void XclExpStream::SetEncrypter( XclExpEncrypterRef const & xEncrypter )
{
    mxEncrypter = xEncrypter;
}

bool XclExpStream::HasValidEncrypter() const
{
    return mxEncrypter && mxEncrypter->IsValid();
}

void XclExpStream::EnableEncryption( bool bEnable )
{
    mbUseEncrypter = bEnable && HasValidEncrypter();
}

void XclExpStream::DisableEncryption()
{
    EnableEncryption(false);
}

void XclExpStream::SetSvStreamPos(sal_uInt64 const nPos)
{
    OSL_ENSURE( !mbInRec, "XclExpStream::SetSvStreamPos - not allowed inside of a record" );
    mbInRec ? 0 : mrStrm.Seek( nPos );
}

// private --------------------------------------------------------------------

void XclExpStream::InitRecord( sal_uInt16 nRecId )
{
    mrStrm.Seek( STREAM_SEEK_TO_END );
    mrStrm.WriteUInt16( nRecId );

    mnLastSizePos = mrStrm.Tell();
    mnHeaderSize = static_cast< sal_uInt16 >( ::std::min< std::size_t >( mnPredictSize, mnCurrMaxSize ) );
    mrStrm.WriteUInt16( mnHeaderSize );
    mnCurrSize = mnSliceSize = 0;
}

void XclExpStream::UpdateRecSize()
{
    if( mnCurrSize != mnHeaderSize )
    {
        mrStrm.Seek( mnLastSizePos );
        mrStrm.WriteUInt16( mnCurrSize );
    }
}

void XclExpStream::UpdateSizeVars( std::size_t nSize )
{
    OSL_ENSURE( mnCurrSize + nSize <= mnCurrMaxSize, "XclExpStream::UpdateSizeVars - record overwritten" );
    mnCurrSize = mnCurrSize + static_cast< sal_uInt16 >( nSize );

    if( mnMaxSliceSize > 0 )
    {
        OSL_ENSURE( mnSliceSize + nSize <= mnMaxSliceSize, "XclExpStream::UpdateSizeVars - slice overwritten" );
        mnSliceSize = mnSliceSize + static_cast< sal_uInt16 >( nSize );
        if( mnSliceSize >= mnMaxSliceSize )
            mnSliceSize = 0;
    }
}

void XclExpStream::StartContinue()
{
    UpdateRecSize();
    mnCurrMaxSize = mnMaxContSize;
    mnPredictSize -= mnCurrSize;
    InitRecord( EXC_ID_CONT );
}

void XclExpStream::PrepareWrite( sal_uInt16 nSize )
{
    if( mbInRec )
    {
        if( (mnCurrSize + nSize > mnCurrMaxSize) ||
            ((mnMaxSliceSize > 0) && (mnSliceSize == 0) && (mnCurrSize + mnMaxSliceSize > mnCurrMaxSize)) )
            StartContinue();
        UpdateSizeVars( nSize );
    }
}

sal_uInt16 XclExpStream::PrepareWrite()
{
    sal_uInt16 nRet = 0;
    if( mbInRec )
    {
        if( (mnCurrSize >= mnCurrMaxSize) ||
            ((mnMaxSliceSize > 0) && (mnSliceSize == 0) && (mnCurrSize + mnMaxSliceSize > mnCurrMaxSize)) )
            StartContinue();
        UpdateSizeVars( 0 );

        nRet = (mnMaxSliceSize > 0) ? (mnMaxSliceSize - mnSliceSize) : (mnCurrMaxSize - mnCurrSize);
    }
    return nRet;
}

void XclExpStream::WriteRawZeroBytes( std::size_t nBytes )
{
    const sal_uInt32 nData = 0;
    std::size_t nBytesLeft = nBytes;
    while( nBytesLeft >= sizeof( nData ) )
    {
        mrStrm.WriteUInt32( nData );
        nBytesLeft -= sizeof( nData );
    }
    if( nBytesLeft )
        mrStrm.WriteBytes(&nData, nBytesLeft);
}

XclExpBiff8Encrypter::XclExpBiff8Encrypter( const XclExpRoot& rRoot ) :
    mnOldPos(STREAM_SEEK_TO_END),
    mbValid(false)
{
    Sequence< NamedValue > aEncryptionData = rRoot.GetEncryptionData();
    if( !aEncryptionData.hasElements() )
        // Empty password.  Get the default biff8 password.
        aEncryptionData = XclExpRoot::GenerateDefaultEncryptionData();
    Init( aEncryptionData );
}

XclExpBiff8Encrypter::~XclExpBiff8Encrypter()
{
}

void XclExpBiff8Encrypter::GetSaltDigest( sal_uInt8 pnSaltDigest[16] ) const
{
    if ( sizeof( mpnSaltDigest ) == 16 )
        memcpy( pnSaltDigest, mpnSaltDigest, 16 );
}

void XclExpBiff8Encrypter::GetSalt( sal_uInt8 pnSalt[16] ) const
{
    if ( sizeof( mpnSalt ) == 16 )
        memcpy( pnSalt, mpnSalt, 16 );
}

void XclExpBiff8Encrypter::GetDocId( sal_uInt8 pnDocId[16] ) const
{
    if ( sizeof( mpnDocId ) == 16 )
    memcpy( pnDocId, mpnDocId, 16 );
}

void XclExpBiff8Encrypter::Encrypt( SvStream& rStrm, sal_uInt8 nData )
{
    vector<sal_uInt8> aByte(1);
    aByte[0] = nData;
    EncryptBytes(rStrm, aByte);
}

void XclExpBiff8Encrypter::Encrypt( SvStream& rStrm, sal_uInt16 nData )
{
    ::std::vector<sal_uInt8> pnBytes(2);
    pnBytes[0] = nData & 0xFF;
    pnBytes[1] = (nData >> 8) & 0xFF;
    EncryptBytes(rStrm, pnBytes);
}

void XclExpBiff8Encrypter::Encrypt( SvStream& rStrm, sal_uInt32 nData )
{
    ::std::vector<sal_uInt8> pnBytes(4);
    pnBytes[0] = nData & 0xFF;
    pnBytes[1] = (nData >>  8) & 0xFF;
    pnBytes[2] = (nData >> 16) & 0xFF;
    pnBytes[3] = (nData >> 24) & 0xFF;
    EncryptBytes(rStrm, pnBytes);
}

void XclExpBiff8Encrypter::Encrypt( SvStream& rStrm, float fValue )
{
    ::std::vector<sal_uInt8> pnBytes(4);
    memcpy(&pnBytes[0], &fValue, 4);
    EncryptBytes(rStrm, pnBytes);
}

void XclExpBiff8Encrypter::Encrypt( SvStream& rStrm, double fValue )
{
    ::std::vector<sal_uInt8> pnBytes(8);
    memcpy(&pnBytes[0], &fValue, 8);
    EncryptBytes(rStrm, pnBytes);
}

void XclExpBiff8Encrypter::Encrypt( SvStream& rStrm, sal_Int8 nData )
{
    Encrypt(rStrm, static_cast<sal_uInt8>(nData));
}

void XclExpBiff8Encrypter::Encrypt( SvStream& rStrm, sal_Int16 nData )
{
    Encrypt(rStrm, static_cast<sal_uInt16>(nData));
}

void XclExpBiff8Encrypter::Encrypt( SvStream& rStrm, sal_Int32 nData )
{
    Encrypt(rStrm, static_cast<sal_uInt32>(nData));
}

void XclExpBiff8Encrypter::Init( const Sequence< NamedValue >& rEncryptionData )
{
    mbValid = false;

    if( maCodec.InitCodec( rEncryptionData ) )
    {
        maCodec.GetDocId( mpnDocId );

        // generate the salt here
        rtlRandomPool aRandomPool = rtl_random_createPool ();
        rtl_random_getBytes( aRandomPool, mpnSalt, 16 );
        rtl_random_destroyPool( aRandomPool );

        memset( mpnSaltDigest, 0, sizeof( mpnSaltDigest ) );

        // generate salt hash.
        ::msfilter::MSCodec_Std97 aCodec;
        aCodec.InitCodec( rEncryptionData );
        aCodec.CreateSaltDigest( mpnSalt, mpnSaltDigest );

        // verify to make sure it's in good shape.
        mbValid = maCodec.VerifyKey( mpnSalt, mpnSaltDigest );
    }
}

sal_uInt32 XclExpBiff8Encrypter::GetBlockPos( std::size_t nStrmPos )
{
    return static_cast< sal_uInt32 >( nStrmPos / EXC_ENCR_BLOCKSIZE );
}

sal_uInt16 XclExpBiff8Encrypter::GetOffsetInBlock( std::size_t nStrmPos )
{
    return static_cast< sal_uInt16 >( nStrmPos % EXC_ENCR_BLOCKSIZE );
}

void XclExpBiff8Encrypter::EncryptBytes( SvStream& rStrm, vector<sal_uInt8>& aBytes )
{
    sal_uInt64 nStrmPos = rStrm.Tell();
    sal_uInt16 nBlockOffset = GetOffsetInBlock(nStrmPos);
    sal_uInt32 nBlockPos = GetBlockPos(nStrmPos);

#if DEBUG_XL_ENCRYPTION
    fprintf(stdout, "XclExpBiff8Encrypter::EncryptBytes: stream pos = %ld  offset in block = %d  block pos = %ld\n",
            nStrmPos, nBlockOffset, nBlockPos);
#endif

    sal_uInt16 nSize = static_cast< sal_uInt16 >( aBytes.size() );
    if (nSize == 0)
        return;

#if DEBUG_XL_ENCRYPTION
    fprintf(stdout, "RAW: ");
    for (sal_uInt16 i = 0; i < nSize; ++i)
        fprintf(stdout, "%2.2X ", aBytes[i]);
    fprintf(stdout, "\n");
#endif

    if (mnOldPos != nStrmPos)
    {
        sal_uInt16 nOldOffset = GetOffsetInBlock(mnOldPos);
        sal_uInt32 nOldBlockPos = GetBlockPos(mnOldPos);

        if ( (nBlockPos != nOldBlockPos) || (nBlockOffset < nOldOffset) )
        {
            maCodec.InitCipher(nBlockPos);
            nOldOffset = 0;
        }

        if (nBlockOffset > nOldOffset)
            maCodec.Skip(nBlockOffset - nOldOffset);
    }

    sal_uInt16 nBytesLeft = nSize;
    sal_uInt16 nPos = 0;
    while (nBytesLeft > 0)
    {
        sal_uInt16 nBlockLeft = EXC_ENCR_BLOCKSIZE - nBlockOffset;
        sal_uInt16 nEncBytes = ::std::min(nBlockLeft, nBytesLeft);

        bool bRet = maCodec.Encode(&aBytes[nPos], nEncBytes, &aBytes[nPos], nEncBytes);
        OSL_ENSURE(bRet, "XclExpBiff8Encrypter::EncryptBytes: encryption failed!!");

        std::size_t nRet = rStrm.WriteBytes(&aBytes[nPos], nEncBytes);
        OSL_ENSURE(nRet == nEncBytes, "XclExpBiff8Encrypter::EncryptBytes: fail to write to stream!!");

        nStrmPos = rStrm.Tell();
        nBlockOffset = GetOffsetInBlock(nStrmPos);
        nBlockPos = GetBlockPos(nStrmPos);
        if (nBlockOffset == 0)
            maCodec.InitCipher(nBlockPos);

        nBytesLeft -= nEncBytes;
        nPos += nEncBytes;
    }
    mnOldPos = nStrmPos;
}

static const char* lcl_GetErrorString( FormulaError nScErrCode )
{
    sal_uInt8 nXclErrCode = XclTools::GetXclErrorCode( nScErrCode );
    switch( nXclErrCode )
    {
        case EXC_ERR_NULL:  return "#NULL!";
        case EXC_ERR_DIV0:  return "#DIV/0!";
        case EXC_ERR_VALUE: return "#VALUE!";
        case EXC_ERR_REF:   return "#REF!";
        case EXC_ERR_NAME:  return "#NAME?";
        case EXC_ERR_NUM:   return "#NUM!";
        case EXC_ERR_NA:
        default:            return "#N/A";
    }
}

void XclXmlUtils::GetFormulaTypeAndValue( ScFormulaCell& rCell, const char*& rsType, OUString& rsValue )
{
    sc::FormulaResultValue aResValue = rCell.GetResult();

    switch (aResValue.meType)
    {
        case sc::FormulaResultValue::Error:
            rsType = "e";
            rsValue = ToOUString(lcl_GetErrorString(aResValue.mnError));
        break;
        case sc::FormulaResultValue::Value:
            rsType = "n";
            rsValue = OUString::number(aResValue.mfValue);
        break;
        case sc::FormulaResultValue::String:
            rsType = "str";
            rsValue = rCell.GetString().getString();
        break;
        case sc::FormulaResultValue::Invalid:
        default:
            // TODO : double-check this to see if this is correct.
            rsType = "inlineStr";
            rsValue = rCell.GetString().getString();
    }
}

OUString XclXmlUtils::GetStreamName( const char* sStreamDir, const char* sStream, sal_Int32 nId )
{
    OUStringBuffer sBuf;
    if( sStreamDir )
        sBuf.appendAscii( sStreamDir );
    sBuf.appendAscii( sStream );
    if( nId )
        sBuf.append( nId );
    if( strstr(sStream, "vml") )
        sBuf.append( ".vml" );
    else
        sBuf.append( ".xml" );
    return sBuf.makeStringAndClear();
}

OString XclXmlUtils::ToOString( const Color& rColor )
{
    char buf[9];
    sprintf( buf, "%.2X%.2X%.2X%.2X", 0xFF-rColor.GetTransparency(), rColor.GetRed(), rColor.GetGreen(), rColor.GetBlue() );
    buf[8] = '\0';
    return OString( buf );
}

OString XclXmlUtils::ToOString( const OUString& s )
{
    return OUStringToOString( s, RTL_TEXTENCODING_UTF8  );
}

OStringBuffer& XclXmlUtils::ToOString( OStringBuffer& s, const ScAddress& rAddress )
{
    rAddress.Format(s, ScRefFlags::VALID, nullptr, ScAddress::Details( FormulaGrammar::CONV_XL_A1));
    return s;
}

OString XclXmlUtils::ToOString( const ScfUInt16Vec& rBuffer )
{
    if(rBuffer.empty())
        return OString();

    const sal_uInt16* pBuffer = &rBuffer [0];
    return OString(
        reinterpret_cast<sal_Unicode const *>(pBuffer), rBuffer.size(),
        RTL_TEXTENCODING_UTF8);
}

OString XclXmlUtils::ToOString( const ScRange& rRange, bool bFullAddressNotation )
{
    OUString sRange(rRange.Format( ScRefFlags::VALID, nullptr,
                                   ScAddress::Details( FormulaGrammar::CONV_XL_A1 ),
                                   bFullAddressNotation ) );
    return ToOString( sRange );
}

OString XclXmlUtils::ToOString( const ScRangeList& rRangeList )
{
    OUString s;
    rRangeList.Format(s, ScRefFlags::VALID, nullptr, FormulaGrammar::CONV_XL_OOX, ' ');
    return ToOString( s );
}

static ScAddress lcl_ToAddress( const XclAddress& rAddress )
{
    ScAddress aAddress;

    // For some reason, ScRange::Format() returns omits row numbers if
    // the row is >= MAXROW or the column is >= MAXCOL, and Excel doesn't
    // like "A:IV" (i.e. no row numbers).  Prevent this.
    // KOHEI: Find out if the above comment is still true.
    aAddress.SetRow( std::min<sal_Int32>( rAddress.mnRow, MAXROW ) );
    aAddress.SetCol( static_cast<sal_Int16>(std::min<sal_Int32>( rAddress.mnCol, MAXCOL )) );

    return aAddress;
}

OStringBuffer& XclXmlUtils::ToOString( OStringBuffer& s, const XclAddress& rAddress )
{
    return ToOString( s, lcl_ToAddress( rAddress ));
}

OString XclXmlUtils::ToOString( const XclExpString& s )
{
    OSL_ENSURE( !s.IsRich(), "XclXmlUtils::ToOString(XclExpString): rich text string found!" );
    return ToOString( s.GetUnicodeBuffer() );
}

static ScRange lcl_ToRange( const XclRange& rRange )
{
    ScRange aRange;

    aRange.aStart = lcl_ToAddress( rRange.maFirst );
    aRange.aEnd   = lcl_ToAddress( rRange.maLast );

    return aRange;
}

OString XclXmlUtils::ToOString( const XclRangeList& rRanges )
{
    ScRangeList aRanges;
    for( const auto& rRange : rRanges )
    {
        aRanges.push_back( lcl_ToRange( rRange ) );
    }
    return ToOString( aRanges );
}

OUString XclXmlUtils::ToOUString( const char* s )
{
    return OUString( s, static_cast<sal_Int32>(strlen( s )), RTL_TEXTENCODING_ASCII_US );
}

OUString XclXmlUtils::ToOUString( const ScfUInt16Vec& rBuf, sal_Int32 nStart, sal_Int32 nLength )
{
    if( nLength == -1 || ( nLength > (static_cast<sal_Int32>(rBuf.size()) - nStart) ) )
        nLength = (rBuf.size() - nStart);

    return nLength > 0
        ? OUString(
            reinterpret_cast<sal_Unicode const *>(&rBuf[nStart]), nLength)
        : OUString();
}

OUString XclXmlUtils::ToOUString(
    sc::CompileFormulaContext& rCtx, const ScAddress& rAddress, const ScTokenArray* pTokenArray )
{
    ScCompiler aCompiler( rCtx, rAddress, const_cast<ScTokenArray&>(*pTokenArray));

    /* TODO: isn't this the same as passed in rCtx and thus superfluous? */
    aCompiler.SetGrammar(FormulaGrammar::GRAM_OOXML);

    OUStringBuffer aBuffer( pTokenArray->GetLen() * 5 );
    aCompiler.CreateStringFromTokenArray( aBuffer );
    return aBuffer.makeStringAndClear();
}

OUString XclXmlUtils::ToOUString( const XclExpString& s )
{
    OSL_ENSURE( !s.IsRich(), "XclXmlUtils::ToOString(XclExpString): rich text string found!" );
    return ToOUString( s.GetUnicodeBuffer() );
}

sax_fastparser::FSHelperPtr XclXmlUtils::WriteElement( sax_fastparser::FSHelperPtr pStream, sal_Int32 nElement, sal_Int32 nValue )
{
    pStream->startElement( nElement, FSEND );
    pStream->write( nValue );
    pStream->endElement( nElement );

    return pStream;
}

sax_fastparser::FSHelperPtr XclXmlUtils::WriteElement( sax_fastparser::FSHelperPtr pStream, sal_Int32 nElement, sal_Int64 nValue )
{
    pStream->startElement( nElement, FSEND );
    pStream->write( nValue );
    pStream->endElement( nElement );

    return pStream;
}

sax_fastparser::FSHelperPtr XclXmlUtils::WriteElement( sax_fastparser::FSHelperPtr pStream, sal_Int32 nElement, const char* sValue )
{
    pStream->startElement( nElement, FSEND );
    pStream->write( sValue );
    pStream->endElement( nElement );

    return pStream;
}

static void lcl_WriteValue( const sax_fastparser::FSHelperPtr& rStream, sal_Int32 nElement, const char* pValue )
{
    if( !pValue )
        return;
    rStream->singleElement( nElement,
            XML_val, pValue,
            FSEND );
}

static const char* lcl_GetUnderlineStyle( FontLineStyle eUnderline, bool& bHaveUnderline )
{
    bHaveUnderline = true;
    switch( eUnderline )
    {
        // OOXTODO: doubleAccounting, singleAccounting
        // OOXTODO: what should be done with the other FontLineStyle values?
        case LINESTYLE_SINGLE:  return "single";
        case LINESTYLE_DOUBLE:  return "double";
        case LINESTYLE_NONE:
        default:                bHaveUnderline = false; return "none";
    }
}

static const char* lcl_ToVerticalAlignmentRun( SvxEscapement eEscapement, bool& bHaveAlignment )
{
    bHaveAlignment = true;
    switch( eEscapement )
    {
        case SvxEscapement::Superscript:    return "superscript";
        case SvxEscapement::Subscript:      return "subscript";
        case SvxEscapement::Off:
        default:                            bHaveAlignment = false; return "baseline";
    }
}

sax_fastparser::FSHelperPtr XclXmlUtils::WriteFontData( sax_fastparser::FSHelperPtr pStream, const XclFontData& rFontData, sal_Int32 nFontId )
{
    bool bHaveUnderline, bHaveVertAlign;
    const char* pUnderline = lcl_GetUnderlineStyle( rFontData.GetScUnderline(), bHaveUnderline );
    const char* pVertAlign = lcl_ToVerticalAlignmentRun( rFontData.GetScEscapement(), bHaveVertAlign );

    lcl_WriteValue( pStream, XML_b,          rFontData.mnWeight > 400 ? ToPsz( true ) : nullptr );
    lcl_WriteValue( pStream, XML_i,          rFontData.mbItalic       ? ToPsz( true ) : nullptr );
    lcl_WriteValue( pStream, XML_strike,     rFontData.mbStrikeout    ? ToPsz( true ) : nullptr );
    // OOXTODO: lcl_WriteValue( rStream, XML_condense, );    // mac compatibility setting
    // OOXTODO: lcl_WriteValue( rStream, XML_extend, );      // compatibility setting
    lcl_WriteValue( pStream, XML_outline,    rFontData.mbOutline      ? ToPsz( true ) : nullptr );
    lcl_WriteValue( pStream, XML_shadow,     rFontData.mbShadow       ? ToPsz( true ) : nullptr );
    lcl_WriteValue( pStream, XML_u,          bHaveUnderline           ? pUnderline    : nullptr );
    lcl_WriteValue( pStream, XML_vertAlign,  bHaveVertAlign           ? pVertAlign    : nullptr );
    lcl_WriteValue( pStream, XML_sz,         OString::number( rFontData.mnHeight / 20.0 ).getStr() );  // Twips->Pt
    if( rFontData.maColor != Color( 0xFF, 0xFF, 0xFF, 0xFF ) )
        pStream->singleElement( XML_color,
                // OOXTODO: XML_auto,       bool
                // OOXTODO: XML_indexed,    uint
                XML_rgb,    XclXmlUtils::ToOString( rFontData.maColor ).getStr(),
                // OOXTODO: XML_theme,      index into <clrScheme/>
                // OOXTODO: XML_tint,       double
                FSEND );
    lcl_WriteValue( pStream, nFontId,        XclXmlUtils::ToOString( rFontData.maName ).getStr() );
    lcl_WriteValue( pStream, XML_family,     OString::number(  rFontData.mnFamily ).getStr() );
    lcl_WriteValue( pStream, XML_charset,    rFontData.mnCharSet != 0 ? OString::number(  rFontData.mnCharSet ).getStr() : nullptr );

    return pStream;
}

XclExpXmlStream::XclExpXmlStream( const uno::Reference< XComponentContext >& rCC, bool bExportVBA, bool bExportTemplate )
    : XmlFilterBase( rCC ),
      mpRoot( nullptr ),
      mbExportVBA(bExportVBA),
      mbExportTemplate(bExportTemplate)
{
}

XclExpXmlStream::~XclExpXmlStream()
{
    assert(maStreams.empty() && "Forgotten PopStream()?");
}

sax_fastparser::FSHelperPtr& XclExpXmlStream::GetCurrentStream()
{
    OSL_ENSURE( !maStreams.empty(), "XclExpXmlStream::GetCurrentStream - no current stream" );
    return maStreams.top();
}

void XclExpXmlStream::PushStream( sax_fastparser::FSHelperPtr const & aStream )
{
    maStreams.push( aStream );
}

void XclExpXmlStream::PopStream()
{
    OSL_ENSURE( !maStreams.empty(), "XclExpXmlStream::PopStream - stack is empty!" );
    maStreams.pop();
}

sax_fastparser::FSHelperPtr XclExpXmlStream::GetStreamForPath( const OUString& sPath )
{
    if( maOpenedStreamMap.find( sPath ) == maOpenedStreamMap.end() )
        return sax_fastparser::FSHelperPtr();
    return maOpenedStreamMap[ sPath ].second;
}

sax_fastparser::FSHelperPtr& XclExpXmlStream::WriteAttributesInternal( sal_Int32 nAttribute, ... )
{
    sax_fastparser::FSHelperPtr& rStream = GetCurrentStream();

    va_list args;
    va_start( args, nAttribute );
    do {
        const char* pValue = va_arg( args, const char* );
        if( pValue )
        {
            rStream->write( " " )
                ->writeId( nAttribute )
                ->write( "=\"" )
                ->writeEscaped( OUString(pValue, strlen(pValue), RTL_TEXTENCODING_UTF8) )
                ->write( "\"" );
        }

        nAttribute = va_arg( args, sal_Int32 );
        if( nAttribute == FSEND_internal )
            break;
    } while( true );
    va_end( args );

    return rStream;
}
sax_fastparser::FSHelperPtr XclExpXmlStream::CreateOutputStream (
    const OUString& sFullStream,
    const OUString& sRelativeStream,
    const uno::Reference< XOutputStream >& xParentRelation,
    const char* sContentType,
    const char* sRelationshipType,
    OUString* pRelationshipId )
{
    OUString sRelationshipId;
    if (xParentRelation.is())
        sRelationshipId = addRelation( xParentRelation, OUString::createFromAscii( sRelationshipType), sRelativeStream );
    else
        sRelationshipId = addRelation( OUString::createFromAscii( sRelationshipType ), sRelativeStream );

    if( pRelationshipId )
        *pRelationshipId = sRelationshipId;

    sax_fastparser::FSHelperPtr p = openFragmentStreamWithSerializer( sFullStream, OUString::createFromAscii( sContentType ) );

    maOpenedStreamMap[ sFullStream ] = std::make_pair( sRelationshipId, p );

    return p;
}

bool XclExpXmlStream::importDocument() throw()
{
    return false;
}

oox::vml::Drawing* XclExpXmlStream::getVmlDrawing()
{
    return nullptr;
}

const oox::drawingml::Theme* XclExpXmlStream::getCurrentTheme() const
{
    return nullptr;
}

const oox::drawingml::table::TableStyleListPtr XclExpXmlStream::getTableStyles()
{
    return oox::drawingml::table::TableStyleListPtr();
}

oox::drawingml::chart::ChartConverter* XclExpXmlStream::getChartConverter()
{
    // DO NOT CALL
    return nullptr;
}

ScDocShell* XclExpXmlStream::getDocShell()
{
    uno::Reference< XInterface > xModel( getModel(), UNO_QUERY );

    ScModelObj *pObj = dynamic_cast < ScModelObj* >( xModel.get() );

    if ( pObj )
        return static_cast < ScDocShell* >( pObj->GetEmbeddedObject() );

    return nullptr;
}

bool XclExpXmlStream::exportDocument()
{
    ScDocShell* pShell = getDocShell();
    ScDocument& rDoc = pShell->GetDocument();
    ScRefreshTimerProtector aProt(rDoc.GetRefreshTimerControlAddress());

    uno::Reference<task::XStatusIndicator> xStatusIndicator = getStatusIndicator();

    if (xStatusIndicator.is())
        xStatusIndicator->start(ScResId(STR_SAVE_DOC), 100);

    // NOTE: Don't use SotStorage or SvStream any more, and never call
    // SfxMedium::GetOutStream() anywhere in the xlsx export filter code!
    // Instead, write via XOutputStream instance.
    tools::SvRef<SotStorage> rStorage = static_cast<SotStorage*>(nullptr);
    XclExpObjList::ResetCounters();

    XclExpRootData aData(
        EXC_BIFF8, *pShell->GetMedium (), rStorage, rDoc,
        msfilter::util::getBestTextEncodingFromLocale(
            Application::GetSettings().GetLanguageTag().getLocale()));
    aData.meOutput = EXC_OUTPUT_XML_2007;
    aData.maXclMaxPos.Set( EXC_MAXCOL_XML_2007, EXC_MAXROW_XML_2007, EXC_MAXTAB_XML_2007 );
    aData.maMaxPos.SetCol( ::std::min( aData.maScMaxPos.Col(), aData.maXclMaxPos.Col() ) );
    aData.maMaxPos.SetRow( ::std::min( aData.maScMaxPos.Row(), aData.maXclMaxPos.Row() ) );
    aData.maMaxPos.SetTab( ::std::min( aData.maScMaxPos.Tab(), aData.maXclMaxPos.Tab() ) );
    aData.mpCompileFormulaCxt.reset( new sc::CompileFormulaContext(&rDoc) );
    // set target path to get correct relative links to target document, not source
    INetURLObject aPath(getFileUrl());
    aData.maBasePath = aPath.GetPath() + "\\";
    aData.maBasePath = "file:///" + aData.maBasePath.replace('\\', '/');

    XclExpRoot aRoot( aData );

    mpRoot = &aRoot;
    aRoot.GetOldRoot().pER = &aRoot;
    aRoot.GetOldRoot().eDateiTyp = Biff8;
    // Get the viewsettings before processing
    if( ScDocShell::GetViewData() )
        ScDocShell::GetViewData()->WriteExtOptions( mpRoot->GetExtDocOptions() );

    OUString const workbook = "xl/workbook.xml";
    const char* pWorkbookContentType = nullptr;
    if (mbExportVBA)
    {
        if (mbExportTemplate)
        {
            pWorkbookContentType = "application/vnd.ms-excel.template.macroEnabled.main+xml";
        }
        else
        {
            pWorkbookContentType = "application/vnd.ms-excel.sheet.macroEnabled.main+xml";
        }
    }
    else
    {
        if (mbExportTemplate)
        {
            pWorkbookContentType = "application/vnd.openxmlformats-officedocument.spreadsheetml.template.main+xml";
        }
        else
        {
            pWorkbookContentType = "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml";
        }
    }

    PushStream( CreateOutputStream( workbook, workbook,
                                    uno::Reference <XOutputStream>(),
                                    pWorkbookContentType,
                                    OUStringToOString(oox::getRelationship(Relationship::OFFICEDOCUMENT), RTL_TEXTENCODING_UTF8).getStr() ) );

    if (mbExportVBA)
    {
        VbaExport aExport(getModel());
        if (aExport.containsVBAProject())
        {
            SvMemoryStream aVbaStream(4096, 4096);
            tools::SvRef<SotStorage> pVBAStorage(new SotStorage(aVbaStream));
            aExport.exportVBA( pVBAStorage.get() );
            aVbaStream.Seek(0);
            css::uno::Reference<css::io::XInputStream> xVBAStream(
                    new utl::OInputStreamWrapper(aVbaStream));
            css::uno::Reference<css::io::XOutputStream> xVBAOutput =
                openFragmentStream("xl/vbaProject.bin", "application/vnd.ms-office.vbaProject");
            comphelper::OStorageHelper::CopyInputToOutput(xVBAStream, xVBAOutput);

            addRelation(GetCurrentStream()->getOutputStream(), oox::getRelationship(Relationship::VBAPROJECT), "vbaProject.bin");
        }
    }

    // destruct at the end of the block
    {
        ExcDocument aDocRoot( aRoot );
        if (xStatusIndicator.is())
            xStatusIndicator->setValue(10);
        aDocRoot.ReadDoc();
        if (xStatusIndicator.is())
            xStatusIndicator->setValue(40);
        aDocRoot.WriteXml( *this );
    }

    PopStream();
    // Free all FSHelperPtr, to flush data before committing storage
    maOpenedStreamMap.clear();

    commitStorage();

    if (xStatusIndicator.is())
        xStatusIndicator->end();
    mpRoot = nullptr;
    return true;
}

::oox::ole::VbaProject* XclExpXmlStream::implCreateVbaProject() const
{
    return new ::oox::xls::ExcelVbaProject( getComponentContext(), uno::Reference< XSpreadsheetDocument >( getModel(), UNO_QUERY ) );
}

OUString XclExpXmlStream::getImplementationName()
{
    return OUString( "TODO" );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
