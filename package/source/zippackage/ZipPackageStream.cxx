/*************************************************************************
 *
 *  $RCSfile: ZipPackageStream.cxx,v $
 *
 *  $Revision: 1.27 $
 *
 *  last change: $Author: mtg $ $Date: 2001-10-02 22:27:29 $
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
 *  Contributor(s): Martin Gallwey (gallwey@sun.com)
 *
 *
 ************************************************************************/

#ifndef _ZIP_PACKAGE_STREAM_HXX
#include <ZipPackageStream.hxx>
#endif
#ifndef _ZIP_PACKAGE_HXX
#include <ZipPackage.hxx>
#endif
#ifndef _ZIP_FILE_HXX
#include <ZipFile.hxx>
#endif
#ifndef _VOS_DIAGNOSE_H_
#include <vos/diagnose.hxx>
#endif
#ifndef _COM_SUN_STAR_PACKAGES_ZIP_ZIPCONSTANTS_HPP_
#include <com/sun/star/packages/zip/ZipConstants.hpp>
#endif

using namespace com::sun::star::packages::zip::ZipConstants;
using namespace com::sun::star::packages::zip;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star;
using namespace cppu;
using namespace rtl;

::cppu::class_data5 ZipPackageStream::s_cd =
{
    5 +1, sal_False, sal_False,
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    {
        { (::cppu::fptr_getCppuType)(::com::sun::star::uno::Type const & (SAL_CALL *)( ::com::sun::star::uno::Reference< ::com::sun::star::io::XActiveDataSink > const * )) &getCppuType, ((sal_Int32)(::com::sun::star::io::XActiveDataSink *) (ZipPackageStream * ) 16) - 16 },
        { (::cppu::fptr_getCppuType)(::com::sun::star::uno::Type const & (SAL_CALL *)( ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > const * )) &getCppuType, ((sal_Int32)(::com::sun::star::beans::XPropertySet *) (ZipPackageStream * ) 16) - 16 },
        { (::cppu::fptr_getCppuType)(::com::sun::star::uno::Type const & (SAL_CALL *)( ::com::sun::star::uno::Reference< ::com::sun::star::container::XNamed > const * ))   &getCppuType, ((sal_Int32)(::com::sun::star::container::XNamed *)   (ZipPackageStream * ) 16) - 16 },
        { (::cppu::fptr_getCppuType)(::com::sun::star::uno::Type const & (SAL_CALL *)( ::com::sun::star::uno::Reference< ::com::sun::star::container::XChild > const * ))   &getCppuType, ((sal_Int32)(::com::sun::star::container::XChild *)   (ZipPackageStream * ) 16) - 16 },
        { (::cppu::fptr_getCppuType)(::com::sun::star::uno::Type const & (SAL_CALL *)( ::com::sun::star::uno::Reference< ::com::sun::star::lang::XUnoTunnel > const * ))    &getCppuType, ((sal_Int32)(::com::sun::star::lang::XUnoTunnel *)    (ZipPackageStream * ) 16) - 16 },
        { (::cppu::fptr_getCppuType)(::com::sun::star::uno::Type const & (SAL_CALL *)( ::com::sun::star::uno::Reference< ::com::sun::star::lang::XTypeProvider > const * )) &getCppuType, ((sal_Int32)(::com::sun::star::lang::XTypeProvider *) (ZipPackageStream * ) 16) - 16 }
    }
};

ZipPackageStream::ZipPackageStream (ZipPackage & rNewPackage )
: rZipPackage(rNewPackage)
, bToBeCompressed ( sal_True )
, bToBeEncrypted ( sal_False )
, bPackageMember ( sal_False )
, bHaveOwnKey ( sal_False )
, xEncryptionData ( )
, ZipPackageEntry ( false )
{
    aEntry.nVersion     = -1;
    aEntry.nFlag        = 0;
    aEntry.nMethod      = -1;
    aEntry.nTime        = -1;
    aEntry.nCrc         = -1;
    aEntry.nCompressedSize  = -1;
    aEntry.nSize        = -1;
    aEntry.nOffset      = -1;
}

ZipPackageStream::~ZipPackageStream( void )
{
}

void ZipPackageStream::setZipEntry( const ZipEntry &rInEntry)
{
    aEntry.nVersion = rInEntry.nVersion;
    aEntry.nFlag = rInEntry.nFlag;
    aEntry.nMethod = rInEntry.nMethod;
    aEntry.nTime = rInEntry.nTime;
    aEntry.nCrc = rInEntry.nCrc;
    aEntry.nCompressedSize = rInEntry.nCompressedSize;
    aEntry.nSize = rInEntry.nSize;
    aEntry.nOffset = rInEntry.nOffset;
    aEntry.sName = rInEntry.sName;
}

// XActiveDataSink
void SAL_CALL ZipPackageStream::setInputStream( const Reference< io::XInputStream >& aStream )
        throw(RuntimeException)
{
    xStream = aStream;
    SetPackageMember ( sal_False );
    aEntry.nTime = -1;
}

Reference< io::XInputStream > SAL_CALL ZipPackageStream::getRawStream( )
        throw(RuntimeException)
{
    if (IsPackageMember())
    {
        try
        {
            if ( !xEncryptionData.isEmpty() && !bHaveOwnKey )
                xEncryptionData->aKey = rZipPackage.getEncryptionKey();
            return rZipPackage.getZipFile().getRawStream(aEntry, xEncryptionData);
        }
        catch (ZipException &)//rException)
        {
            VOS_ENSURE( 0, "ZipException thrown");//rException.Message);
            return Reference < io::XInputStream > ();
        }
    }
    else
        return xStream;
}

Reference< io::XInputStream > SAL_CALL ZipPackageStream::getInputStream(  )
        throw(RuntimeException)
{
    if (IsPackageMember())
    {
        try
        {
            if ( !xEncryptionData.isEmpty() && !bHaveOwnKey )
                xEncryptionData->aKey = rZipPackage.getEncryptionKey();
            return rZipPackage.getZipFile().getInputStream( aEntry, xEncryptionData);
        }
        catch (ZipException &)//rException)
        {
            VOS_ENSURE( 0,"ZipException thrown");//rException.Message);
            return Reference < io::XInputStream > ();
        }
    }
    else
        return xStream;
}


sal_Int64 SAL_CALL ZipPackageStream::getSomething( const Sequence< sal_Int8 >& aIdentifier )
    throw(RuntimeException)
{
    sal_Int64 nMe = 0;
    if ( aIdentifier.getLength() == 16 &&
         0 == rtl_compareMemory(getUnoTunnelImplementationId().getConstArray(), aIdentifier.getConstArray(), 16 ) )
        nMe = reinterpret_cast < sal_Int64 > ( this );
    return nMe;
}

// XPropertySet
void SAL_CALL ZipPackageStream::setPropertyValue( const OUString& aPropertyName, const Any& aValue )
        throw(beans::UnknownPropertyException, beans::PropertyVetoException, IllegalArgumentException, WrappedTargetException, RuntimeException)
{
    if (aPropertyName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("MediaType")))
    {
        aValue >>= sMediaType;

        if (sMediaType.getLength() > 0)
        {
            if ( sMediaType.indexOf (OUString( RTL_CONSTASCII_USTRINGPARAM ( "text" ) ) ) != -1)
                bToBeCompressed = sal_True;
            else
                bToBeCompressed = sal_False;
        }
    }
    else if (aPropertyName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("Size") ) )
        aValue >>= aEntry.nSize;
    else if (aPropertyName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("Encrypted") ) )
    {
        aValue >>= bToBeEncrypted;
        if ( bToBeEncrypted && xEncryptionData.isEmpty())
            xEncryptionData = new EncryptionData;
    }
    else if (aPropertyName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("EncryptionKey") ) )
    {
        if ( xEncryptionData.isEmpty())
            xEncryptionData = new EncryptionData;
        bHaveOwnKey = bToBeEncrypted = sal_True;
        if ( !( aValue >>= xEncryptionData->aKey ) )
            throw IllegalArgumentException();
    }
#if SUPD>617
    else if (aPropertyName.equalsAsciiL ( RTL_CONSTASCII_STRINGPARAM ( "Compressed" ) ) )
#else
    else if (aPropertyName.equalsAsciiL ( RTL_CONSTASCII_STRINGPARAM ( "Compress" ) ) )
#endif
        aValue >>= bToBeCompressed;
    else
        throw beans::UnknownPropertyException();
}

Any SAL_CALL ZipPackageStream::getPropertyValue( const OUString& PropertyName )
        throw(beans::UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    Any aAny;
    if (PropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "MediaType" ) ) )
    {
        aAny <<= sMediaType;
        return aAny;
    }
    else if (PropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( "Size" ) ) )
    {
        aAny <<= aEntry.nSize;
        return aAny;
    }
    else if (PropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( "Encrypted" ) ) )
    {
        aAny <<= bToBeEncrypted;
        return aAny;
    }
#if SUPD>617
    else if (PropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( "Compressed" ) ) )
#else
    else if (PropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( "Compress" ) ) )
#endif
    {
        aAny <<= bToBeCompressed;
        return aAny;
    }
    else if (PropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "EncryptionKey" ) ) )
    {
        aAny <<= xEncryptionData.isEmpty () ? Sequence < sal_Int8 > () : xEncryptionData->aKey;
        return aAny;
    }
    else
        throw beans::UnknownPropertyException();
}
void ZipPackageStream::setSize (const sal_Int32 nNewSize)
{
    if (aEntry.nCompressedSize != nNewSize )
        aEntry.nMethod = DEFLATED;
    aEntry.nSize = nNewSize;
}
