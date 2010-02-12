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

#include "oox/xls/biffdetector.hxx"
#include <algorithm>
#include <rtl/strbuf.hxx>
#include <com/sun/star/io/XInputStream.hpp>
#include <comphelper/mediadescriptor.hxx>
#include "oox/helper/binaryinputstream.hxx"
#include "oox/helper/olestorage.hxx"

using ::rtl::OUString;
using ::rtl::OStringBuffer;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::uno::RuntimeException;
using ::com::sun::star::uno::XInterface;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::lang::XMultiServiceFactory;
using ::com::sun::star::beans::PropertyValue;
using ::com::sun::star::io::XInputStream;
using ::comphelper::MediaDescriptor;

namespace oox {
namespace xls {

// ============================================================================

Sequence< OUString > BiffDetector_getSupportedServiceNames()
{
    Sequence< OUString > aServiceNames( 1 );
    aServiceNames[ 0 ] = CREATE_OUSTRING( "com.sun.star.frame.ExtendedTypeDetection" );
    return aServiceNames;
}

OUString BiffDetector_getImplementationName()
{
    return CREATE_OUSTRING( "com.sun.star.comp.oox.BiffDetector" );
}

Reference< XInterface > SAL_CALL BiffDetector_createInstance( const Reference< XMultiServiceFactory >& rxFactory ) throw( Exception )
{
    return static_cast< ::cppu::OWeakObject* >( new BiffDetector( rxFactory ) );
}

// ============================================================================

BiffDetector::BiffDetector( const Reference< XMultiServiceFactory >& rxFactory ) :
    mxFactory( rxFactory )
{
}

BiffDetector::~BiffDetector()
{
}

BiffType BiffDetector::detectStreamBiffVersion( BinaryInputStream& rInStream )
{
    BiffType eBiff = BIFF_UNKNOWN;
    if( !rInStream.isEof() && rInStream.isSeekable() && (rInStream.getLength() > 4) )
    {
        sal_Int64 nOldPos = rInStream.tell();
        rInStream.seek( 0 );
        sal_uInt16 nBofId, nBofSize;
        rInStream >> nBofId >> nBofSize;

        if( (4 <= nBofSize) && (nBofSize <= 16) && (rInStream.tell() + nBofSize <= rInStream.getLength()) )
        {
            switch( nBofId )
            {
                case BIFF2_ID_BOF:
                    eBiff = BIFF2;
                break;
                case BIFF3_ID_BOF:
                    eBiff = BIFF3;
                break;
                case BIFF4_ID_BOF:
                    eBiff = BIFF4;
                break;
                case BIFF5_ID_BOF:
                {
                    if( 6 <= nBofSize )
                    {
                        sal_uInt16 nVersion;
                        rInStream >> nVersion;
                        // #i23425# #i44031# #i62752# there are some *really* broken documents out there...
                        switch( nVersion & 0xFF00 )
                        {
                            case 0:                 eBiff = BIFF5;  break;  // #i44031# #i62752#
                            case BIFF_BOF_BIFF2:    eBiff = BIFF2;  break;
                            case BIFF_BOF_BIFF3:    eBiff = BIFF3;  break;
                            case BIFF_BOF_BIFF4:    eBiff = BIFF4;  break;
                            case BIFF_BOF_BIFF5:    eBiff = BIFF5;  break;
                            case BIFF_BOF_BIFF8:    eBiff = BIFF8;  break;
                            default:    OSL_ENSURE( false,
                                OStringBuffer( "lclDetectStreamBiffVersion - unknown BIFF version: 0x" ).
                                append( static_cast< sal_Int32 >( nVersion ), 16 ).getStr() );
                        }
                    }
                }
                break;
                // else do nothing, no BIFF stream
            }
        }
        rInStream.seek( nOldPos );
    }
    return eBiff;
}

BiffType BiffDetector::detectStorageBiffVersion( OUString& orWorkbookStreamName, StorageRef xStorage )
{
    static const OUString saBookName = CREATE_OUSTRING( "Book" );
    static const OUString saWorkbookName = CREATE_OUSTRING( "Workbook" );

    BiffType eBiff = BIFF_UNKNOWN;
    if( xStorage.get() )
    {
        if( xStorage->isStorage() )
        {
            // try to open the "Book" stream
            BinaryXInputStream aBookStrm5( xStorage->openInputStream( saBookName ), true );
            BiffType eBookStrm5Biff = detectStreamBiffVersion( aBookStrm5 );

            // try to open the "Workbook" stream
            BinaryXInputStream aBookStrm8( xStorage->openInputStream( saWorkbookName ), true );
            BiffType eBookStrm8Biff = detectStreamBiffVersion( aBookStrm8 );

            // decide which stream to use
            if( (eBookStrm8Biff != BIFF_UNKNOWN) && ((eBookStrm5Biff == BIFF_UNKNOWN) || (eBookStrm8Biff > eBookStrm5Biff)) )
            {
                /*  Only "Workbook" stream exists; or both streams exist,
                    and "Workbook" has higher BIFF version than "Book" stream. */
                eBiff = eBookStrm8Biff;
                orWorkbookStreamName = saWorkbookName;
            }
            else if( eBookStrm5Biff != BIFF_UNKNOWN )
            {
                /*  Only "Book" stream exists; or both streams exist,
                    and "Book" has higher BIFF version than "Workbook" stream. */
                eBiff = eBookStrm5Biff;
                orWorkbookStreamName = saBookName;
            }
        }
        else
        {
            // no storage, try plain input stream from medium (even for BIFF5+)
            BinaryXInputStream aStrm( xStorage->openInputStream( OUString() ), false );
            eBiff = detectStreamBiffVersion( aStrm );
            orWorkbookStreamName = OUString();
        }
    }

    return eBiff;
}

// com.sun.star.lang.XServiceInfo interface -----------------------------------

OUString SAL_CALL BiffDetector::getImplementationName() throw( RuntimeException )
{
    return BiffDetector_getImplementationName();
}

sal_Bool SAL_CALL BiffDetector::supportsService( const OUString& rService ) throw( RuntimeException )
{
    const Sequence< OUString > aServices( BiffDetector_getSupportedServiceNames() );
    const OUString* pArray = aServices.getConstArray();
    const OUString* pArrayEnd = pArray + aServices.getLength();
    return ::std::find( pArray, pArrayEnd, rService ) != pArrayEnd;
}

Sequence< OUString > SAL_CALL BiffDetector::getSupportedServiceNames() throw( RuntimeException )
{
    return BiffDetector_getSupportedServiceNames();
}

// com.sun.star.document.XExtendedFilterDetect interface ----------------------

OUString SAL_CALL BiffDetector::detect( Sequence< PropertyValue >& rDescriptor ) throw( RuntimeException )
{
    OUString aTypeName;

    MediaDescriptor aDescriptor( rDescriptor );
    aDescriptor.addInputStream();

    Reference< XInputStream > xInStrm( aDescriptor[ MediaDescriptor::PROP_INPUTSTREAM() ], UNO_QUERY );
    if( xInStrm.is() )
    {
        OUString aWorkbookName;
        StorageRef xStorage( new OleStorage( mxFactory, xInStrm, true ) );
        switch( detectStorageBiffVersion( aWorkbookName, xStorage ) )
        {
            case BIFF2:
            case BIFF3:
            case BIFF4: aTypeName = CREATE_OUSTRING( "calc_MS_Excel_40" );  break;
            case BIFF5: aTypeName = CREATE_OUSTRING( "calc_MS_Excel_95" );  break;
            case BIFF8: aTypeName = CREATE_OUSTRING( "calc_MS_Excel_97" );  break;
            default:;
        }
    }

    return aTypeName;
}

// ============================================================================

} // namespace xls
} // namespace oox

