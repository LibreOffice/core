/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: xestream.cxx,v $
 * $Revision: 1.11.30.2 $
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
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <utility>

#include <rtl/ustring.hxx>
#include <sax/fshelper.hxx>
#include <unotools/streamwrap.hxx>

#include "precompiled_sc.hxx"
#include "docuno.hxx"
#include "xestream.hxx"
#include "xladdress.hxx"
#include "xlstring.hxx"
#include "xeroot.hxx"
#include "xestyle.hxx"
#include "rangelst.hxx"
#include "compiler.hxx"

#include <oox/core/tokens.hxx>
#include <formula/grammar.hxx>

using ::com::sun::star::beans::PropertyValue;
using ::com::sun::star::io::XOutputStream;
using ::com::sun::star::io::XStream;
using ::com::sun::star::lang::XComponent;
using ::com::sun::star::lang::XMultiServiceFactory;
using ::com::sun::star::lang::XServiceInfo;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::UNO_QUERY;
using ::rtl::OString;
using ::rtl::OUString;
using ::utl::OStreamWrapper;
using namespace formula;
// ============================================================================

XclExpStream::XclExpStream( SvStream& rOutStrm, const XclExpRoot& rRoot, sal_uInt16 nMaxRecSize ) :
    mrStrm( rOutStrm ),
    mrRoot( rRoot ),
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

void XclExpStream::StartRecord( sal_uInt16 nRecId, sal_Size nRecSize )
{
    DBG_ASSERT( !mbInRec, "XclExpStream::StartRecord - another record still open" );
    mnMaxContSize = mnCurrMaxSize = mnMaxRecSize;
    mnPredictSize = nRecSize;
    mbInRec = true;
    InitRecord( nRecId );
    SetSliceSize( 0 );
}

void XclExpStream::EndRecord()
{
    DBG_ASSERT( mbInRec, "XclExpStream::EndRecord - no record open" );
    UpdateRecSize();
    mrStrm.Seek( STREAM_SEEK_TO_END );
    mbInRec = false;
}

void XclExpStream::SetSliceSize( sal_uInt16 nSize )
{
    mnMaxSliceSize = nSize;
    mnSliceSize = 0;
}

sal_Size XclExpStream::Write( const void* pData, sal_Size nBytes )
{
    sal_Size nRet = 0;
    if( pData && (nBytes > 0) )
    {
        if( mbInRec )
        {
            const sal_uInt8* pBuffer = reinterpret_cast< const sal_uInt8* >( pData );
            sal_Size nBytesLeft = nBytes;
            bool bValid = true;

            while( bValid && (nBytesLeft > 0) )
            {
                sal_Size nWriteLen = ::std::min< sal_Size >( PrepareWrite(), nBytesLeft );
                sal_Size nWriteRet = mrStrm.Write( pBuffer, nWriteLen );
                bValid = (nWriteLen == nWriteRet);
                DBG_ASSERT( bValid, "XclExpStream::Write - stream write error" );
                pBuffer += nWriteRet;
                nRet += nWriteRet;
                nBytesLeft -= nWriteRet;
                UpdateSizeVars( nWriteRet );
            }
        }
        else
            nRet = mrStrm.Write( pData, nBytes );
    }
    return nRet;
}

void XclExpStream::WriteZeroBytes( sal_Size nBytes )
{
    if( mbInRec )
    {
        sal_Size nBytesLeft = nBytes;
        while( nBytesLeft > 0 )
        {
            sal_Size nWriteLen = ::std::min< sal_Size >( PrepareWrite(), nBytesLeft );
            WriteRawZeroBytes( nWriteLen );
            nBytesLeft -= nWriteLen;
            UpdateSizeVars( nWriteLen );
        }
    }
    else
        WriteRawZeroBytes( nBytes );
}

sal_Size XclExpStream::CopyFromStream( SvStream& rInStrm, sal_Size nBytes )
{
    sal_Size nStrmPos = rInStrm.Tell();
    rInStrm.Seek( STREAM_SEEK_TO_END );
    sal_Size nStrmSize = rInStrm.Tell();
    rInStrm.Seek( nStrmPos );

    sal_Size nBytesLeft = ::std::min( nBytes, nStrmSize - nStrmPos );
    sal_Size nRet = 0;
    if( nBytesLeft > 0 )
    {
        const sal_Size nMaxBuffer = 4096;
        sal_uInt8* pBuffer = new sal_uInt8[ ::std::min( nBytesLeft, nMaxBuffer ) ];
        bool bValid = true;

        while( bValid && (nBytesLeft > 0) )
        {
            sal_Size nWriteLen = ::std::min( nBytesLeft, nMaxBuffer );
            rInStrm.Read( pBuffer, nWriteLen );
            sal_Size nWriteRet = Write( pBuffer, nWriteLen );
            bValid = (nWriteLen == nWriteRet);
            nRet += nWriteRet;
            nBytesLeft -= nWriteRet;
        }
        delete[] pBuffer;
    }
    return nRet;
}

//UNUSED2008-05  void XclExpStream::WriteUnicodeBuffer( const sal_uInt16* pBuffer, sal_Size nChars, sal_uInt8 nFlags )
//UNUSED2008-05  {
//UNUSED2008-05      SetSliceSize( 0 );
//UNUSED2008-05      if( pBuffer && (nChars > 0) )
//UNUSED2008-05      {
//UNUSED2008-05          sal_uInt16 nCharLen = (nFlags & EXC_STRF_16BIT) ? 2 : 1;
//UNUSED2008-05          for( sal_Size nIndex = 0; nIndex < nChars; ++nIndex )
//UNUSED2008-05          {
//UNUSED2008-05              if( mbInRec && (mnCurrSize + nCharLen > mnCurrMaxSize) )
//UNUSED2008-05              {
//UNUSED2008-05                  StartContinue();
//UNUSED2008-05                  // repeat only 16bit flag
//UNUSED2008-05                  operator<<( static_cast< sal_uInt8 >( nFlags & EXC_STRF_16BIT ) );
//UNUSED2008-05              }
//UNUSED2008-05              if( nCharLen == 2 )
//UNUSED2008-05                  operator<<( pBuffer[ nIndex ] );
//UNUSED2008-05              else
//UNUSED2008-05                  operator<<( static_cast< sal_uInt8 >( pBuffer[ nIndex ] ) );
//UNUSED2008-05          }
//UNUSED2008-05      }
//UNUSED2008-05  }

void XclExpStream::WriteUnicodeBuffer( const ScfUInt16Vec& rBuffer, sal_uInt8 nFlags )
{
    SetSliceSize( 0 );
    nFlags &= EXC_STRF_16BIT;   // repeat only 16bit flag
    sal_uInt16 nCharLen = nFlags ? 2 : 1;

    ScfUInt16Vec::const_iterator aEnd = rBuffer.end();
    for( ScfUInt16Vec::const_iterator aIter = rBuffer.begin(); aIter != aEnd; ++aIter )
    {
        if( mbInRec && (mnCurrSize + nCharLen > mnCurrMaxSize) )
        {
            StartContinue();
            operator<<( nFlags );
        }
        if( nCharLen == 2 )
            operator<<( *aIter );
        else
            operator<<( static_cast< sal_uInt8 >( *aIter ) );
    }
}

//UNUSED2008-05  void XclExpStream::WriteByteStringBuffer( const ByteString& rString, sal_uInt16 nMaxLen )
//UNUSED2008-05  {
//UNUSED2008-05      SetSliceSize( 0 );
//UNUSED2008-05      Write( rString.GetBuffer(), ::std::min< sal_Size >( rString.Len(), nMaxLen ) );
//UNUSED2008-05  }

// ER: #71367# Xcl has an obscure sense of whether starting a new record or not,
// and crashes if it encounters the string header at the very end of a record.
// Thus we add 1 to give some room, seems like they do it that way but with another count (10?)
void XclExpStream::WriteByteString( const ByteString& rString, sal_uInt16 nMaxLen, bool b16BitCount )
{
    SetSliceSize( 0 );
    sal_Size nLen = ::std::min< sal_Size >( rString.Len(), nMaxLen );
    if( !b16BitCount )
        nLen = ::std::min< sal_Size >( nLen, 0xFF );

    sal_uInt16 nLeft = PrepareWrite();
    sal_uInt16 nLenFieldSize = b16BitCount ? 2 : 1;
    if( mbInRec && (nLeft <= nLenFieldSize) )
        StartContinue();

    if( b16BitCount )
        operator<<( static_cast< sal_uInt16 >( nLen ) );
    else
        operator<<( static_cast< sal_uInt8 >( nLen ) );
    Write( rString.GetBuffer(), nLen );
}

void XclExpStream::WriteCharBuffer( const ScfUInt8Vec& rBuffer )
{
    SetSliceSize( 0 );
    Write( &rBuffer[ 0 ], rBuffer.size() );
}

sal_Size XclExpStream::SetSvStreamPos( sal_Size nPos )
{
    DBG_ASSERT( !mbInRec, "XclExpStream::SetSvStreamPos - not allowed inside of a record" );
    return mbInRec ? 0 : mrStrm.Seek( nPos );
}

// private --------------------------------------------------------------------

void XclExpStream::InitRecord( sal_uInt16 nRecId )
{
    mrStrm.Seek( STREAM_SEEK_TO_END );
    mrStrm << nRecId;

    mnLastSizePos = mrStrm.Tell();
    mnHeaderSize = static_cast< sal_uInt16 >( ::std::min< sal_Size >( mnPredictSize, mnCurrMaxSize ) );
    mrStrm << mnHeaderSize;
    mnCurrSize = mnSliceSize = 0;
}

void XclExpStream::UpdateRecSize()
{
    if( mnCurrSize != mnHeaderSize )
    {
        mrStrm.Seek( mnLastSizePos );
        mrStrm << mnCurrSize;
    }
}

void XclExpStream::UpdateSizeVars( sal_Size nSize )
{
    DBG_ASSERT( mnCurrSize + nSize <= mnCurrMaxSize, "XclExpStream::UpdateSizeVars - record overwritten" );
    mnCurrSize = mnCurrSize + static_cast< sal_uInt16 >( nSize );

    if( mnMaxSliceSize > 0 )
    {
        DBG_ASSERT( mnSliceSize + nSize <= mnMaxSliceSize, "XclExpStream::UpdateSizeVars - slice overwritten" );
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

void XclExpStream::WriteRawZeroBytes( sal_Size nBytes )
{
    const sal_uInt32 nData = 0;
    sal_Size nBytesLeft = nBytes;
    while( nBytesLeft >= sizeof( nData ) )
    {
        mrStrm << nData;
        nBytesLeft -= sizeof( nData );
    }
    if( nBytesLeft )
        mrStrm.Write( &nData, nBytesLeft );
}

// ============================================================================

rtl::OUString XclXmlUtils::GetStreamName( const char* sStreamDir, const char* sStream, sal_Int32 nId )
{
    rtl::OUStringBuffer sBuf;
    if( sStreamDir )
        sBuf.appendAscii( sStreamDir );
    sBuf.appendAscii( sStream );
    if( nId )
        sBuf.append( nId );
    sBuf.appendAscii( ".xml" );
    return sBuf.makeStringAndClear();
}

rtl::OString XclXmlUtils::ToOString( const Color& rColor )
{
    char buf[9];
    sprintf( buf, "%.2X%.2X%.2X%.2X", rColor.GetTransparency(), rColor.GetRed(), rColor.GetGreen(), rColor.GetBlue() );
    buf[8] = '\0';
    return OString( buf );
}

rtl::OString XclXmlUtils::ToOString( const ::rtl::OUString& s )
{
    return OUStringToOString( s, RTL_TEXTENCODING_UTF8  );
}

rtl::OString XclXmlUtils::ToOString( const String& s )
{
    return rtl::OString( s.GetBuffer(), s.Len(), RTL_TEXTENCODING_UTF8 );
}

rtl::OString XclXmlUtils::ToOString( const ScAddress& rAddress )
{
    String sAddress;
    rAddress.Format( sAddress, SCA_VALID, NULL, ScAddress::Details( FormulaGrammar::CONV_XL_A1 ) );
    return ToOString( sAddress );
}

rtl::OString XclXmlUtils::ToOString( const ScfUInt16Vec& rBuffer )
{
    const sal_uInt16* pBuffer = &rBuffer [0];
    return ::rtl::OString( pBuffer, rBuffer.size(), RTL_TEXTENCODING_UTF8 );
}

rtl::OString XclXmlUtils::ToOString( const ScRange& rRange )
{
    String sRange;
    rRange.Format( sRange, SCA_VALID, NULL, ScAddress::Details( FormulaGrammar::CONV_XL_A1 ) );
    return ToOString( sRange );
}

rtl::OString XclXmlUtils::ToOString( const ScRangeList& rRangeList )
{
    String s;
    rRangeList.Format( s, SCA_VALID, NULL, FormulaGrammar::CONV_XL_A1, ' ' );
    return ToOString( s );
}

static ScAddress lcl_ToAddress( const XclAddress& rAddress )
{
    ScAddress aAddress;

    // For some reason, ScRange::Format() returns omits row numbers if
    // the row is >= MAXROW or the column is >= MAXCOL, and Excel doesn't
    // like "A:IV" (i.e. no row numbers).  Prevent this.
    aAddress.SetRow( std::min<sal_Int32>( rAddress.mnRow, MAXROW-1 ) );
    aAddress.SetCol( std::min<sal_Int32>( rAddress.mnCol, MAXCOL-1 ) );

    return aAddress;
}

rtl::OString XclXmlUtils::ToOString( const XclAddress& rAddress )
{
    return ToOString( lcl_ToAddress( rAddress ) );
}

rtl::OString XclXmlUtils::ToOString( const XclExpString& s )
{
    DBG_ASSERT( !s.IsRich(), "XclXmlUtils::ToOString(XclExpString): rich text string found!" );
    return ToOString( s.GetUnicodeBuffer() );
}

static ScRange lcl_ToRange( const XclRange& rRange )
{
    ScRange aRange;

    aRange.aStart = lcl_ToAddress( rRange.maFirst );
    aRange.aEnd   = lcl_ToAddress( rRange.maLast );

    return aRange;
}

rtl::OString XclXmlUtils::ToOString( const XclRangeList& rRanges )
{
    ScRangeList aRanges;
    for( XclRangeList::const_iterator i = rRanges.begin(), end = rRanges.end();
            i != end; ++i )
    {
        aRanges.Append( lcl_ToRange( *i ) );
    }
    return ToOString( aRanges );
}

OUString XclXmlUtils::ToOUString( const char* s )
{
    return OUString( s, (sal_Int32) strlen( s ), RTL_TEXTENCODING_ASCII_US );
}

OUString XclXmlUtils::ToOUString( const ScfUInt16Vec& rBuf, sal_Int32 nStart, sal_Int32 nLength )
{
    if( nLength == -1 )
        nLength = rBuf.size();

    return OUString( &rBuf[nStart], nLength );
}

OUString XclXmlUtils::ToOUString( const String& s )
{
    return OUString( s.GetBuffer(), s.Len() );
}

rtl::OUString XclXmlUtils::ToOUString( ScDocument& rDocument, const ScAddress& rAddress, ScTokenArray* pTokenArray )
{
    ScCompiler aCompiler( &rDocument, rAddress, *pTokenArray);
    aCompiler.SetGrammar(FormulaGrammar::GRAM_NATIVE_XL_A1);
    String s;
    aCompiler.CreateStringFromTokenArray( s );
    return ToOUString( s );
}

OUString XclXmlUtils::ToOUString( const XclExpString& s )
{
    DBG_ASSERT( !s.IsRich(), "XclXmlUtils::ToOString(XclExpString): rich text string found!" );
    return ToOUString( s.GetUnicodeBuffer() );
}

const char* XclXmlUtils::ToPsz( bool b )
{
    return b ? "true" : "false";
}

// ============================================================================

XclExpXmlStream::XclExpXmlStream( const Reference< XMultiServiceFactory >& rSMgr, SvStream& rStrm, const XclExpRoot& rRoot )
    : XmlFilterBase( rSMgr )
    , mrRoot( rRoot )
{
    Sequence< PropertyValue > aArgs( 1 );
    const OUString sStream( RTL_CONSTASCII_USTRINGPARAM( "StreamForOutput" ) );
    aArgs[0].Name  = sStream;
    aArgs[0].Value <<= Reference< XStream > ( new OStreamWrapper( rStrm ) );

    XServiceInfo* pInfo = rRoot.GetDocModelObj();
    Reference< XComponent > aComponent( pInfo, UNO_QUERY );
    setSourceDocument( aComponent );
    filter( aArgs );

    PushStream( CreateOutputStream(
                OUString::createFromAscii( "xl/workbook.xml" ),
                OUString::createFromAscii( "xl/workbook.xml" ),
                Reference< XOutputStream >(),
                "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml",
                "http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" ) );
}

XclExpXmlStream::~XclExpXmlStream()
{
}

sax_fastparser::FSHelperPtr& XclExpXmlStream::GetCurrentStream()
{
    DBG_ASSERT( !maStreams.empty(), "XclExpXmlStream::GetCurrentStream - no current stream" );
    return maStreams.top();
}

void XclExpXmlStream::PushStream( sax_fastparser::FSHelperPtr aStream )
{
    maStreams.push( aStream );
}

void XclExpXmlStream::PopStream()
{
    DBG_ASSERT( !maStreams.empty(), "XclExpXmlStream::PopStream - stack is empty!" );
    maStreams.pop();
}

OUString XclExpXmlStream::GetIdForPath( const OUString& sPath )
{
    if( maOpenedStreamMap.find( sPath ) == maOpenedStreamMap.end() )
        return OUString();
    return maOpenedStreamMap[ sPath ].first;
}

sax_fastparser::FSHelperPtr XclExpXmlStream::GetStreamForPath( const OUString& sPath )
{
    if( maOpenedStreamMap.find( sPath ) == maOpenedStreamMap.end() )
        return sax_fastparser::FSHelperPtr();
    return maOpenedStreamMap[ sPath ].second;
}

sax_fastparser::FSHelperPtr& XclExpXmlStream::WriteAttributes( sal_Int32 nAttribute, ... )
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
                ->writeEscaped( pValue )
                ->write( "\"" );
        }

        nAttribute = va_arg( args, sal_Int32 );
        if( nAttribute == FSEND )
            break;
    } while( true );
    va_end( args );

    return rStream;
}

static void lcl_WriteValue( sax_fastparser::FSHelperPtr& rStream, sal_Int32 nElement, const char* pValue )
{
    if( !pValue )
        return;
    rStream->singleElement( nElement,
            XML_val, pValue,
            FSEND );
}

static const char* lcl_GetUnderlineStyle( FontUnderline eUnderline, bool& bHaveUnderline )
{
    bHaveUnderline = true;
    switch( eUnderline )
    {
        // OOXTODO: doubleAccounting, singleAccounting
        // OOXTODO: what should be done with the other FontUnderline values?
        case UNDERLINE_SINGLE:  return "single";
        case UNDERLINE_DOUBLE:  return "double";
        case UNDERLINE_NONE:
        default:                bHaveUnderline = false; return "none";
    }
}

static const char* lcl_ToVerticalAlignmentRun( SvxEscapement eEscapement, bool& bHaveAlignment )
{
    bHaveAlignment = true;
    switch( eEscapement )
    {
        case SVX_ESCAPEMENT_SUPERSCRIPT:    return "superscript";
        case SVX_ESCAPEMENT_SUBSCRIPT:      return "subscript";
        case SVX_ESCAPEMENT_OFF:
        default:                            bHaveAlignment = false; return "baseline";
    }
}

sax_fastparser::FSHelperPtr& XclExpXmlStream::WriteFontData( const XclFontData& rFontData, sal_Int32 nFontId )
{
    bool bHaveUnderline, bHaveVertAlign;
    const char* pUnderline = lcl_GetUnderlineStyle( rFontData.GetScUnderline(), bHaveUnderline );
    const char* pVertAlign = lcl_ToVerticalAlignmentRun( rFontData.GetScEscapement(), bHaveVertAlign );

    sax_fastparser::FSHelperPtr& rStream = GetCurrentStream();

    lcl_WriteValue( rStream, nFontId,        XclXmlUtils::ToOString( rFontData.maName ).getStr() );
    lcl_WriteValue( rStream, XML_charset,    rFontData.mnCharSet != 0 ? OString::valueOf( (sal_Int32) rFontData.mnCharSet ).getStr() : NULL );
    lcl_WriteValue( rStream, XML_family,     OString::valueOf( (sal_Int32) rFontData.mnFamily ).getStr() );
    lcl_WriteValue( rStream, XML_b,          rFontData.mnWeight > 400 ? XclXmlUtils::ToPsz( rFontData.mnWeight > 400 ) : NULL );
    lcl_WriteValue( rStream, XML_i,          rFontData.mbItalic ? XclXmlUtils::ToPsz( rFontData.mbItalic ) : NULL );
    lcl_WriteValue( rStream, XML_strike,     rFontData.mbStrikeout ? XclXmlUtils::ToPsz( rFontData.mbStrikeout ) : NULL );
    lcl_WriteValue( rStream, XML_outline,    rFontData.mbOutline ? XclXmlUtils::ToPsz( rFontData.mbOutline ) : NULL );
    lcl_WriteValue( rStream, XML_shadow,     rFontData.mbShadow ? XclXmlUtils::ToPsz( rFontData.mbShadow ) : NULL );
    // OOXTODO: lcl_WriteValue( rStream, XML_condense, );    // mac compatibility setting
    // OOXTODO: lcl_WriteValue( rStream, XML_extend, );      // compatibility setting
    if( rFontData.maColor != Color( 0xFF, 0xFF, 0xFF, 0xFF ) )
        rStream->singleElement( XML_color,
                // OOXTODO: XML_auto,       bool
                // OOXTODO: XML_indexed,    uint
                XML_rgb,    XclXmlUtils::ToOString( rFontData.maColor ).getStr(),
                // OOXTODO: XML_theme,      index into <clrScheme/>
                // OOXTODO: XML_tint,       double
                FSEND );
    lcl_WriteValue( rStream, XML_sz,         OString::valueOf( (double) (rFontData.mnHeight / 20.0) ) );  // Twips->Pt
    lcl_WriteValue( rStream, XML_u,          bHaveUnderline ? pUnderline : NULL );
    lcl_WriteValue( rStream, XML_vertAlign,  bHaveVertAlign ? pVertAlign : NULL );

    return rStream;
}

sax_fastparser::FSHelperPtr XclExpXmlStream::CreateOutputStream (
    const OUString& sFullStream,
    const OUString& sRelativeStream,
    const Reference< XOutputStream >& xParentRelation,
    const char* sContentType,
    const char* sRelationshipType,
    ::rtl::OUString* pRelationshipId )
{
    OUString sRelationshipId;
    if (xParentRelation.is())
        sRelationshipId = addRelation( xParentRelation, OUString::createFromAscii( sRelationshipType), sRelativeStream );
    else
        sRelationshipId = addRelation( OUString::createFromAscii( sRelationshipType ), sRelativeStream );

    if( pRelationshipId )
        *pRelationshipId = sRelationshipId;

    sax_fastparser::FSHelperPtr p = openOutputStreamWithSerializer( sFullStream, OUString::createFromAscii( sContentType ) );

    maOpenedStreamMap[ sFullStream ] = std::make_pair( sRelationshipId, p );

    return p;
}

bool XclExpXmlStream::importDocument() throw()
{
    return false;
}

sal_Int32 XclExpXmlStream::getSchemeClr( sal_Int32 /*nColorSchemeToken*/ ) const
{
    return -1;
}

const oox::vml::DrawingPtr XclExpXmlStream::getDrawings()
{
    return oox::vml::DrawingPtr();
}

const oox::drawingml::Theme* XclExpXmlStream::getCurrentTheme() const
{
    return NULL;
}

const oox::drawingml::table::TableStyleListPtr XclExpXmlStream::getTableStyles()
{
    return oox::drawingml::table::TableStyleListPtr();
}

oox::drawingml::chart::ChartConverter& XclExpXmlStream::getChartConverter()
{
    // DO NOT CALL
    return * (oox::drawingml::chart::ChartConverter*) NULL;
}

bool XclExpXmlStream::exportDocument() throw()
{
    return false;
}

::rtl::OUString XclExpXmlStream::implGetImplementationName() const
{
    return CREATE_OUSTRING( "TODO" );
}

void XclExpXmlStream::Trace( const char* format, ...)
{
    va_list ap;
    va_start( ap, format );
    vfprintf( stderr, format, ap );
    va_end( ap );
}

