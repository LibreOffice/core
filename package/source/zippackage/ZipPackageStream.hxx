/*************************************************************************
 *
 *  $RCSfile: ZipPackageStream.hxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: kz $ $Date: 2003-09-11 10:18:17 $
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
#define _ZIP_PACKAGE_STREAM_HXX

#ifndef _COM_SUN_STAR_IO_XACTIVEDATASINK_HPP_
#include <com/sun/star/io/XActiveDataSink.hpp>
#endif
#ifndef _COM_SUN_STAR_PACKAGES_XDATASINKENCRSUPPORT_HPP_
#include <com/sun/star/packages/XDataSinkEncrSupport.hpp>
#endif

#ifndef _ZIP_PACKAGE_ENTRY_HXX
#include <ZipPackageEntry.hxx>
#endif
#ifndef _VOS_REF_H_
#include <vos/ref.hxx>
#endif
#ifndef _ENCRYPTION_DATA_HXX_
#include <EncryptionData.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE2_HXX
#include <cppuhelper/implbase2.hxx>
#endif

#define PACKAGE_STREAM_NOTSET           0
#define PACKAGE_STREAM_PACKAGEMEMBER    1
#define PACKAGE_STREAM_DETECT           2
#define PACKAGE_STREAM_DATA             3
#define PACKAGE_STREAM_RAW              4

class ZipPackage;
struct ZipEntry;
#if defined( MACOSX ) && ( __GNUC__ < 3 )
class ZipPackageStream : public ZipPackageEntry,
                                             public ::cppu::OWeakObject,
                                                 public ::com::sun::star::io::XActiveDataSink,
                                                public ::com::sun::star::package::XDataSinkEncrSupport
#else
class ZipPackageStream : public cppu::ImplInheritanceHelper2
<
    ZipPackageEntry,
    ::com::sun::star::io::XActiveDataSink,
    ::com::sun::star::packages::XDataSinkEncrSupport
>
#endif
{
    static com::sun::star::uno::Sequence < sal_Int8 > aImplementationId;
protected:
    com::sun::star::uno::Reference < com::sun::star::io::XInputStream > xStream;
    ZipPackage          &rZipPackage;
    sal_Bool            bToBeCompressed, bToBeEncrypted, bHaveOwnKey, bIsEncrypted;
    vos::ORef < EncryptionData > xEncryptionData;

    sal_uInt8   m_nStreamMode;
    sal_uInt32  m_nMagicalHackPos;
    sal_uInt32  m_nMagicalHackSize;

public:
    sal_Bool HasOwnKey ()        { return bHaveOwnKey;}
    sal_Bool IsToBeCompressed () { return bToBeCompressed;}
    sal_Bool IsToBeEncrypted ()  { return bToBeEncrypted;}
    sal_Bool IsEncrypted ()      { return bIsEncrypted;}
    sal_Bool IsPackageMember ()  { return m_nStreamMode == PACKAGE_STREAM_PACKAGEMEMBER;}
    vos::ORef < EncryptionData > & getEncryptionData ()
    { return xEncryptionData;}
    const com::sun::star::uno::Sequence < sal_Int8 >& getKey ()
    { return xEncryptionData->aKey;}
    const com::sun::star::uno::Sequence < sal_uInt8 >& getInitialisationVector ()
    { return xEncryptionData->aInitVector;}
    const com::sun::star::uno::Sequence < sal_uInt8 >& getDigest ()
    { return xEncryptionData->aDigest;}
    const com::sun::star::uno::Sequence < sal_uInt8 >& getSalt ()
    { return xEncryptionData->aSalt;}
    const sal_Int32 getIterationCount ()
    { return xEncryptionData->nIterationCount;}
    const sal_Int32 getSize ()
    { return aEntry.nSize;}

    sal_uInt8 GetStreamMode() { return m_nStreamMode; }
    sal_uInt32 GetMagicalHackPos() { return m_nMagicalHackPos; }
    sal_uInt32 GetMagicalHackSize() { return m_nMagicalHackSize; }

    void SetToBeCompressed (sal_Bool bNewValue) { bToBeCompressed = bNewValue;}
    void SetIsEncrypted (sal_Bool bNewValue) { bIsEncrypted = bNewValue;}
    void SetToBeEncrypted (sal_Bool bNewValue)
    {
        bToBeEncrypted  = bNewValue;
        if ( bToBeEncrypted && xEncryptionData.isEmpty())
            xEncryptionData = new EncryptionData;
        else if ( !bToBeEncrypted && !xEncryptionData.isEmpty() )
            xEncryptionData.unbind();
    }
    void SetPackageMember (sal_Bool bNewValue);
    void setKey (const com::sun::star::uno::Sequence < sal_Int8 >& rNewKey )
    { xEncryptionData->aKey = rNewKey;}
    void setInitialisationVector (const com::sun::star::uno::Sequence < sal_uInt8 >& rNewVector )
    { xEncryptionData->aInitVector = rNewVector;}
    void setSalt (const com::sun::star::uno::Sequence < sal_uInt8 >& rNewSalt )
    { xEncryptionData->aSalt = rNewSalt;}
    void setDigest (const com::sun::star::uno::Sequence < sal_uInt8 >& rNewDigest )
    { xEncryptionData->aDigest = rNewDigest;}
    void setIterationCount (const sal_Int32 nNewCount)
    { xEncryptionData->nIterationCount = nNewCount;}
    void setSize (const sal_Int32 nNewSize);

    ZipPackageStream (ZipPackage & rNewPackage);
    virtual ~ZipPackageStream( void );

    sal_Bool ParsePackageRawStream();

    void setZipEntry( const ZipEntry &rInEntry);
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > SAL_CALL getRawData()
        throw(::com::sun::star::uno::RuntimeException);

    static ::com::sun::star::uno::Sequence < sal_Int8 >& static_getImplementationId()
    {
        return aImplementationId;
    }

#if defined( MACOSX ) && ( __GNUC__ < 3 )
    // XInterface
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& rType )
        throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL acquire(  )
        throw();
    virtual void SAL_CALL release(  )
        throw();
#endif

    // XActiveDataSink
    virtual void SAL_CALL setInputStream( const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& aStream )
        throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > SAL_CALL getInputStream(  )
        throw(::com::sun::star::uno::RuntimeException);

    // XDataSinkEncrSupport
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > SAL_CALL getDataStream()
        throw ( ::com::sun::star::packages::WrongPasswordException,
                ::com::sun::star::io::IOException,
                ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > SAL_CALL getRawStream()
        throw ( ::com::sun::star::packages::NoEncryptionException,
                ::com::sun::star::io::IOException,
                ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL setDataStream(
                    const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& aStream )
        throw ( ::com::sun::star::io::IOException,
                ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL setRawStream(
                    const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& aStream )
        throw ( ::com::sun::star::packages::EncryptionNotAllowedException,
                ::com::sun::star::packages::NoRawFormatException,
                ::com::sun::star::io::IOException,
                ::com::sun::star::uno::RuntimeException);

    // XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier )
        throw(::com::sun::star::uno::RuntimeException);

    // XPropertySet
    virtual void SAL_CALL setPropertyValue( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue )
        throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const ::rtl::OUString& PropertyName )
        throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(  )
        throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  )
        throw (::com::sun::star::uno::RuntimeException);
};
#endif
