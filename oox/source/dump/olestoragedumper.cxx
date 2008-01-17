/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: olestoragedumper.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-17 08:05:58 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#include "oox/dump/olestoragedumper.hxx"
#include <osl/thread.h>
#include <osl/file.hxx>
#include <rtl/tencinfo.h>
#include <com/sun/star/util/DateTime.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include "oox/helper/binaryoutputstream.hxx"
#include "oox/core/filterbase.hxx"

#if OOX_INCLUDE_DUMPER

using ::rtl::OUString;
using ::rtl::OUStringBuffer;
using ::rtl::OString;
using ::rtl::OStringToOUString;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::util::DateTime;
using ::com::sun::star::io::XInputStream;
using ::com::sun::star::io::XOutputStream;

namespace oox {
namespace dump {

// ============================================================================

namespace {

const sal_Int32 OLEPROP_ID_DICTIONARY   = 0;
const sal_Int32 OLEPROP_ID_CODEPAGE     = 1;

const sal_Int32 OLEPROP_TYPE_INT16      = 2;
const sal_Int32 OLEPROP_TYPE_INT32      = 3;
const sal_Int32 OLEPROP_TYPE_FLOAT      = 4;
const sal_Int32 OLEPROP_TYPE_DOUBLE     = 5;
const sal_Int32 OLEPROP_TYPE_DATE       = 7;
const sal_Int32 OLEPROP_TYPE_STRING     = 8;
const sal_Int32 OLEPROP_TYPE_STATUS     = 10;
const sal_Int32 OLEPROP_TYPE_BOOL       = 11;
const sal_Int32 OLEPROP_TYPE_VARIANT    = 12;
const sal_Int32 OLEPROP_TYPE_INT8       = 16;
const sal_Int32 OLEPROP_TYPE_UINT8      = 17;
const sal_Int32 OLEPROP_TYPE_UINT16     = 18;
const sal_Int32 OLEPROP_TYPE_UINT32     = 19;
const sal_Int32 OLEPROP_TYPE_INT64      = 20;
const sal_Int32 OLEPROP_TYPE_UINT64     = 21;
const sal_Int32 OLEPROP_TYPE_STRING8    = 30;
const sal_Int32 OLEPROP_TYPE_STRING16   = 31;
const sal_Int32 OLEPROP_TYPE_FILETIME   = 64;
const sal_Int32 OLEPROP_TYPE_BLOB       = 65;
const sal_Int32 OLEPROP_TYPE_STREAM     = 66;
const sal_Int32 OLEPROP_TYPE_STORAGE    = 67;
const sal_Int32 OLEPROP_TYPE_CLIPFMT    = 71;

const sal_uInt16 CODEPAGE_UNICODE       = 1200;

} // namespace

// ============================================================================

OlePropertyStreamObject::OlePropertyStreamObject( const ObjectBase& rParent, const OUString& rOutFileName, BinaryInputStreamRef xStrm ) :
    InputStreamObject( rParent, rOutFileName, xStrm )
{
}

void OlePropertyStreamObject::implDump()
{
    Input& rIn = in();
    Output& rOut = out();

    OUStringVector aGuidVec;
    ::std::vector< sal_uInt32 > aStartPosVec;

    // dump header
    writeEmptyItem( "HEADER" );
    {
        IndentGuard aIndGuard( rOut );
        dumpHex< sal_uInt16 >( "byte-order", "OLEPROP-BYTE-ORDER" );
        dumpDec< sal_uInt16 >( "version" );
        dumpDec< sal_uInt16 >( "os-minor" );
        dumpDec< sal_uInt16 >( "os-type", "OLEPROP-OSTYPE" );
        dumpGuid( "guid" );
        sal_Int32 nSectCount = dumpDec< sal_Int32 >( "section-count" );

        // dump table of section positions
        {
            TableGuard aTabGuard( rOut, 15, 60 );
            rOut.resetItemIndex();
            for( sal_Int32 nSectIdx = 0; (nSectIdx < nSectCount) && rIn.isValidPos(); ++nSectIdx )
            {
                MultiItemsGuard aMultiGuard( rOut );
                writeEmptyItem( "#section" );
                aGuidVec.push_back( dumpGuid( "guid" ) );
                aStartPosVec.push_back( dumpHex< sal_uInt32 >( "start-pos" ) );
            }
        }
    }
    rOut.emptyLine();

    // dump sections
    for( size_t nSectIdx = 0; (nSectIdx < aStartPosVec.size()) && rIn.isValidPos(); ++nSectIdx )
        dumpSection( aGuidVec[ nSectIdx ], aStartPosVec[ nSectIdx ] );
}

void OlePropertyStreamObject::dumpSection( const OUString& rGuid, sal_uInt32 nStartPos )
{
    Input& rIn = in();
    Output& rOut = out();

    // property ID names
    mxPropIds = cfg().createNameList< ConstList >( "OLEPROP-IDS" );
    OUString aGuidName = cfg().getStringOption( rGuid, OUString() );
    if( aGuidName.equalsAscii( "GlobalDocProp" ) )
        mxPropIds->includeList( cfg().getNameList( "OLEPROP-GLOBALIDS" ) );
    else if( aGuidName.equalsAscii( "BuiltinDocProp" ) )
        mxPropIds->includeList( cfg().getNameList( "OLEPROP-BUILTINIDS" ) );
    else
        mxPropIds->includeList( cfg().getNameList( "OLEPROP-BASEIDS" ) );

    // property ID/position map
    typedef ::std::map< sal_Int32, sal_uInt32 > PropertyPosMap;
    PropertyPosMap aPropMap;

    // dump section header line
    writeSectionHeader( rGuid, nStartPos );

    // seek to section
    IndentGuard aIndGuard( rOut );
    if( startElement( nStartPos ) )
    {
        // dump section header
        dumpDec< sal_Int32 >( "size" );
        sal_Int32 nPropCount = dumpDec< sal_Int32 >( "property-count" );

        // dump table of property positions
        {
            TableGuard aTabGuard( rOut, 15, 25 );
            rOut.resetItemIndex();
            for( sal_Int32 nPropIdx = 0; (nPropIdx < nPropCount) && rIn.isValidPos(); ++nPropIdx )
            {
                MultiItemsGuard aMultiGuard( rOut );
                writeEmptyItem( "#property" );
                sal_Int32 nPropId = dumpDec< sal_Int32 >( "id", mxPropIds );
                sal_uInt32 nPropPos = nStartPos + dumpHex< sal_uInt32 >( "start-pos" );
                aPropMap[ nPropId ] = nPropPos;
            }
        }
    }
    rOut.emptyLine();

    // code page property
    meTextEnc = osl_getThreadTextEncoding();
    mbIsUnicode = false;
    PropertyPosMap::iterator aCodePageIt = aPropMap.find( OLEPROP_ID_CODEPAGE );
    if( aCodePageIt != aPropMap.end() )
    {
        dumpCodePageProperty( aCodePageIt->second );
        aPropMap.erase( aCodePageIt );
    }

    // dictionary property
    PropertyPosMap::iterator aDictIt = aPropMap.find( OLEPROP_ID_DICTIONARY );
    if( aDictIt != aPropMap.end() )
    {
        dumpDictionaryProperty( aDictIt->second );
        aPropMap.erase( aDictIt );
    }

    // other properties
    for( PropertyPosMap::const_iterator aIt = aPropMap.begin(), aEnd = aPropMap.end(); aIt != aEnd; ++aIt )
        dumpProperty( aIt->first, aIt->second );

    // remove the user defined list of property ID names
    cfg().eraseNameList( "OLEPROP-IDS" );
}

void OlePropertyStreamObject::dumpProperty( sal_Int32 nPropId, sal_uInt32 nStartPos )
{
    writePropertyHeader( nPropId, nStartPos );
    IndentGuard aIndGuard( out() );
    if( startElement( nStartPos ) )
        dumpPropertyContents( nPropId );
    out().emptyLine();
}

void OlePropertyStreamObject::dumpCodePageProperty( sal_uInt32 nStartPos )
{
    writePropertyHeader( OLEPROP_ID_CODEPAGE, nStartPos );
    IndentGuard aIndGuard( out() );
    if( startElement( nStartPos ) )
    {
        sal_Int32 nType = dumpPropertyType();
        if( nType == OLEPROP_TYPE_INT16 )
        {
            sal_uInt16 nCodePage = dumpDec< sal_uInt16 >( "codepage", "CODEPAGES" );
            rtl_TextEncoding nNewTextEnc = rtl_getTextEncodingFromWindowsCodePage( nCodePage );
            if( nNewTextEnc != RTL_TEXTENCODING_DONTKNOW )
                meTextEnc = nNewTextEnc;
            mbIsUnicode = nCodePage == CODEPAGE_UNICODE;
        }
        else
            dumpPropertyContents( OLEPROP_ID_CODEPAGE );
    }
    out().emptyLine();
}

void OlePropertyStreamObject::dumpDictionaryProperty( sal_uInt32 nStartPos )
{
    writePropertyHeader( OLEPROP_ID_DICTIONARY, nStartPos );
    IndentGuard aIndGuard( out() );
    if( startElement( nStartPos ) )
    {
        sal_Int32 nCount = dumpDec< sal_Int32 >( "count" );
        for( sal_Int32 nIdx = 0; (nIdx < nCount) && in().isValidPos(); ++nIdx )
        {
            MultiItemsGuard aMultiGuard( out() );
            TableGuard aTabGuard( out(), 10, 20 );
            sal_Int32 nId = dumpDec< sal_Int32 >( "id" );
            OUString aName = dumpString8( "name" );
            if( mxPropIds.get() )
                mxPropIds->setName( nId, aName );
        }
    }
    out().emptyLine();
}

void OlePropertyStreamObject::dumpPropertyContents( sal_Int32 nPropId )
{
    sal_Int32 nType = dumpPropertyType();
    if( getFlag< sal_Int32 >( nType, 0x1000 ) ) // vector
    {
        sal_Int32 nBaseType = nType & 0x0FFF;
        sal_Int32 nElemCount = dumpDec< sal_Int32 >( "element-count" );
        for( sal_Int32 nElemIdx = 0; (nElemIdx < nElemCount) && in().isValidPos(); ++nElemIdx )
        {
            out().resetItemIndex( nElemIdx );
            writeEmptyItem( "#element" );
            IndentGuard aIndGuard( out() );
            dumpPropertyValue( nPropId, nBaseType );
        }
    }
    else if( !getFlag< sal_Int32 >( nType, 0x7000 ) )
    {
        dumpPropertyValue( nPropId, nType );
    }
}

void OlePropertyStreamObject::dumpPropertyValue( sal_Int32 nPropId, sal_Int32 nBaseType )
{
    switch( nBaseType )
    {
        case OLEPROP_TYPE_INT16:        dumpDec< sal_Int16 >( "value" );        break;
        case OLEPROP_TYPE_INT32:        dumpDec< sal_Int32 >( "value" );        break;
        case OLEPROP_TYPE_FLOAT:        dumpDec< float >( "value" );            break;
        case OLEPROP_TYPE_DOUBLE:       dumpDec< double >( "value" );           break;
        case OLEPROP_TYPE_DATE:         dumpDec< double >( "date" );            break;
        case OLEPROP_TYPE_STRING:       dumpString8( "value" );                 break;
        case OLEPROP_TYPE_STATUS:       dumpHex< sal_Int32 >( "status" );       break;
        case OLEPROP_TYPE_BOOL:         dumpBool< sal_Int16 >( "value" );       break;
        case OLEPROP_TYPE_VARIANT:      dumpPropertyContents( nPropId );        break;
        case OLEPROP_TYPE_INT8:         dumpDec< sal_Int8 >( "value" );         break;
        case OLEPROP_TYPE_UINT8:        dumpDec< sal_uInt8 >( "value" );        break;
        case OLEPROP_TYPE_UINT16:       dumpDec< sal_uInt16 >( "value" );       break;
        case OLEPROP_TYPE_UINT32:       dumpDec< sal_uInt32 >( "value" );       break;
        case OLEPROP_TYPE_INT64:        dumpDec< sal_Int64 >( "value" );        break;
        case OLEPROP_TYPE_UINT64:       dumpDec< sal_uInt64 >( "value" );       break;
        case OLEPROP_TYPE_STRING8:      dumpString8( "value" );                 break;
        case OLEPROP_TYPE_STRING16:     dumpString16( "value" );                break;
        case OLEPROP_TYPE_FILETIME:     dumpFileTime( "file-time" );            break;
        case OLEPROP_TYPE_BLOB:         dumpBlob( "data" );                     break;
        case OLEPROP_TYPE_STREAM:       dumpString8( "stream-name" );           break;
        case OLEPROP_TYPE_STORAGE:      dumpString8( "storage-name" );          break;
        case OLEPROP_TYPE_CLIPFMT:      dumpBlob( "clip-data" );                break;
    }
}

sal_Int32 OlePropertyStreamObject::dumpPropertyType()
{
    return dumpHex< sal_Int32 >( "type", "OLEPROP-TYPE" );
}

void OlePropertyStreamObject::dumpBlob( const sal_Char* pcName )
{
    sal_Int32 nSize = dumpDec< sal_Int32 >( "data-size" );
    if( nSize > 0 )
        dumpBinary( pcName, nSize );
}

OUString OlePropertyStreamObject::dumpString8( const sal_Char* pcName )
{
    sal_Int32 nLen = dumpDec< sal_Int32 >( "string-len" );
    return mbIsUnicode ? dumpCharArray16( pcName, nLen ) : dumpCharArray8( pcName, nLen );
}

OUString OlePropertyStreamObject::dumpCharArray8( const sal_Char* pcName, sal_Int32 nCharCount )
{
    OUString aData;
    size_t nLen = getLimitedValue< size_t, sal_Int32 >( nCharCount, 0, 1024 );
    if( nLen > 0 )
    {
        ::std::vector< sal_Char > aBuffer( nLen + 1 );
        in().read( &aBuffer.front(), nLen );
        aBuffer[ nLen ] = 0;
        aData = OStringToOUString( OString( &aBuffer.front() ), meTextEnc );
    }
    writeStringItem( pcName, aData );
    return aData;
}

OUString OlePropertyStreamObject::dumpString16( const sal_Char* pcName )
{
    sal_Int32 nLen = dumpDec< sal_Int32 >( "string-len" );
    return dumpCharArray16( pcName, nLen );
}

OUString OlePropertyStreamObject::dumpCharArray16( const sal_Char* pcName, sal_Int32 nCharCount )
{
    size_t nLen = getLimitedValue< size_t, sal_Int32 >( nCharCount, 0, 1024 );
    ::std::vector< sal_Unicode > aBuffer;
    aBuffer.reserve( nLen + 1 );
    for( size_t nIdx = 0; nIdx < nLen; ++nIdx )
        aBuffer.push_back( static_cast< sal_Unicode >( in().readValue< sal_uInt16 >() ) );
    aBuffer.push_back( 0 );
    OUString aData( &aBuffer.front() );
    writeStringItem( pcName, aData );
    if( nLen & 1 ) dumpUnused( 2 ); // always padding to 32bit
    return aData;
}

DateTime OlePropertyStreamObject::dumpFileTime( const sal_Char* pcName )
{
    DateTime aDateTime;

    ItemGuard aItem( out(), pcName );
    sal_Int64 nFileTime = dumpDec< sal_Int64 >( 0 );
    // file time is in 10^-7 seconds (100 nanoseconds), convert to 1/100 seconds
    nFileTime /= 100000;
    // entire days
    sal_Int64 nDays = nFileTime / sal_Int64( 360000 * 24 );
    // number of entire years
    sal_Int64 nYears = (nDays - (nDays / (4 * 365)) + (nDays / (100 * 365)) - (nDays / (400 * 365))) / 365;
    // remaining days in the year
    sal_Int64 nDaysInYear = nDays - (nYears * 365 + nYears / 4 - nYears / 100 + nYears / 400);
    // the year (file dates start from 1601-01-01)
    aDateTime.Year = static_cast< sal_uInt16 >( 1601 + nYears );
    // leap year?
    bool bLeap = ((aDateTime.Year % 4 == 0) && (aDateTime.Year % 100 != 0)) || (aDateTime.Year % 400 == 0);
    // static arrays with number of days in month
    static const sal_Int64 spnDaysInMonth[]  = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    static const sal_Int64 spnDaysInMonthL[] = { 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    const sal_Int64* pnDaysInMonth = bLeap ? spnDaysInMonthL : spnDaysInMonth;
    // the month
    aDateTime.Month = 1;
    while( nDaysInYear >= *pnDaysInMonth )
    {
        nDaysInYear -= *pnDaysInMonth++;
        ++aDateTime.Month;
    }
    // the day
    aDateTime.Day = static_cast< sal_uInt16 >( nDaysInYear + 1 );
    // number of 1/100 seconds in the day
    sal_Int64 nTimeInDay = nFileTime % sal_Int64( 360000 * 24 );
    // 1/100 seconds
    aDateTime.HundredthSeconds = static_cast< sal_uInt16 >( nTimeInDay % 100 );
    nTimeInDay /= 100;
    // seconds
    aDateTime.Seconds = static_cast< sal_uInt16 >( nTimeInDay % 60 );
    nTimeInDay /= 60;
    // minutes
    aDateTime.Minutes = static_cast< sal_uInt16 >( nTimeInDay % 60 );
    nTimeInDay /= 60;
    // hours
    aDateTime.Hours = static_cast< sal_uInt16 >( nTimeInDay );

//    aDateTime.convertToLocalTime();
    writeDateTimeItem( 0, aDateTime );
    return aDateTime;
}

bool OlePropertyStreamObject::startElement( sal_uInt32 nStartPos )
{
    sal_Int64 nStartPos64 = static_cast< sal_Int64>( nStartPos );
    bool bPosOk = nStartPos64 < in().getSize();
    if( bPosOk )
        in().seek( nStartPos64 );
    else
        writeInfoItem( "stream-state", OOX_DUMP_ERR_STREAM );
    return bPosOk;
}

void OlePropertyStreamObject::writeSectionHeader( const OUString& rGuid, sal_uInt32 nStartPos )
{
    MultiItemsGuard aMultiGuard( out() );
    writeEmptyItem( "SECTION" );
    writeGuidItem( "guid", rGuid );
    writeHexItem( "pos", nStartPos );
}

void OlePropertyStreamObject::writePropertyHeader( sal_Int32 nPropId, sal_uInt32 nStartPos )
{
    MultiItemsGuard aMultiGuard( out() );
    writeEmptyItem( "PROPERTY" );
    writeDecItem( "id", nPropId, mxPropIds );
    writeHexItem( "pos", nStartPos );
}

// ============================================================================
// ============================================================================

} // namespace dump
} // namespace oox

#endif

