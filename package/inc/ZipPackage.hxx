/*************************************************************************
 *
 *  $RCSfile: ZipPackage.hxx,v $
 *
 *  $Revision: 1.26 $
 *
 *  last change: $Author: mtg $ $Date: 2001-09-14 14:50:28 $
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
#ifndef _ZIP_PACKAGE_HXX
#define _ZIP_PACKAGE_HXX

#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XHIERARCHICALNAMEACCESS_HPP_
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSINGLESERVICEFACTORY_HPP_
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XINPUTSTREAM_HPP_
#include <com/sun/star/io/XInputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XOUTPUTSTREAM_HPP_
#include <com/sun/star/io/XOutputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XSEEKABLE_HPP_
#include <com/sun/star/io/XSeekable.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XCHANGESBATCH_HPP_
#include <com/sun/star/util/XChangesBatch.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XUNOTUNNEL_HPP_
#include <com/sun/star/lang/XUnoTunnel.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_TASK_XINTERACTIONHANDLER_HPP_
#include <com/sun/star/task/XInteractionHandler.hpp>
#endif
#ifndef _HASHMAPS_HXX
#include <HashMaps.hxx>
#endif
#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif
#include <vector>

class ZipPackageFolder;
class ZipFile;
class ByteGrabber;

enum SegmentEnum
{
    e_Aborted = -1000,
    e_Retry,
    e_Finished,
    e_Success = 0
};
class ZipPackage :
                   public cppu::OWeakObject,
                   public com::sun::star::lang::XInitialization,
                   public com::sun::star::lang::XSingleServiceFactory,
                   public com::sun::star::lang::XUnoTunnel,
                   public com::sun::star::container::XHierarchicalNameAccess,
                   public com::sun::star::util::XChangesBatch,
                   public com::sun::star::beans::XPropertySet
{
protected:
    ::com::sun::star::uno::Sequence < sal_Int8 > aEncryptionKey;
    FolderHash       aRecent;
    ::rtl::OUString  sURL;
    sal_Int32        nSegmentSize;
    sal_Bool         bHasEncryptedEntries;

    ::com::sun::star::uno::Reference < com::sun::star::container::XNameContainer > xRootFolder;
    ::com::sun::star::uno::Reference < com::sun::star::io::XInputStream > xContentStream;
    ::com::sun::star::uno::Reference < com::sun::star::io::XSeekable > xContentSeek;
    ::com::sun::star::uno::Reference < com::sun::star::task::XInteractionHandler > xInteractionHandler;
    const ::com::sun::star::uno::Reference < com::sun::star::lang::XMultiServiceFactory > xFactory;

    ZipPackageFolder *pRootFolder;
    ZipFile          *pZipFile;

    void getZipFileContents();
    // this one is for removable media
    SegmentEnum writeSegment(
                       const ::rtl::OUString & rFileName,
                       ::rtl::OUString & rMountPath,
                       ::com::sun::star::uno::Reference < com::sun::star::io::XInputStream > &xStream,
                       const sal_Int16 nDiskNum);
    // this one is for non-removable media
    SegmentEnum writeSegment(
                       const ::rtl::OUString & rFileName,
                       ::com::sun::star::uno::Reference < com::sun::star::io::XInputStream > &xStream );
    SegmentEnum readSegment(
                       const ::rtl::OUString & rFileName,
                       ::rtl::OUString & rMountPath,
                       ::com::sun::star::uno::Reference < com::sun::star::io::XOutputStream > &xTempOut,
                       const sal_Int16 nDiskNum);
    void getInteractionHandler();
    sal_Bool HandleError ( com::sun::star::uno::Any &rAny, sal_uInt16 eContinuations );
    sal_Bool HandleError ( oslFileError, sal_uInt16 eContinuations, const ::rtl::OUString &rFileName );
    sal_Int32 RequestDisk ( ::rtl::OUString &rMountPath, sal_Int16 nDiskNum);
    void unSpanFile ( );
    sal_Bool checkEnd ( com::sun::star::uno::Sequence < sal_Int8 > &rSequence );
    void writeTempFile();

public:
    ZipPackage (const ::com::sun::star::uno::Reference < com::sun::star::lang::XMultiServiceFactory > &xNewFactory);
    virtual ~ZipPackage( void );
    ZipFile& getZipFile() { return *pZipFile;}
    const com::sun::star::uno::Sequence < sal_Int8 > & getEncryptionKey ( ) {return aEncryptionKey;}

    // XInterface
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& rType )
        throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL acquire(  )
        throw();
    virtual void SAL_CALL release(  )
        throw();

    // XInitialization
    virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments )
        throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
    // XHierarchicalNameAccess
    virtual ::com::sun::star::uno::Any SAL_CALL getByHierarchicalName( const ::rtl::OUString& aName )
        throw(::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasByHierarchicalName( const ::rtl::OUString& aName )
        throw(::com::sun::star::uno::RuntimeException);
    // XSingleServiceFactory
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL createInstance(  )
        throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL createInstanceWithArguments( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments )
        throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
    // XChangesBatch
    virtual void SAL_CALL commitChanges(  )
        throw(::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasPendingChanges(  )
        throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::util::ElementChange > SAL_CALL getPendingChanges(  )
        throw(::com::sun::star::uno::RuntimeException);
    // XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier )
        throw(::com::sun::star::uno::RuntimeException);
    com::sun::star::uno::Sequence < sal_Int8 > getUnoTunnelImplementationId( void )
        throw(::com::sun::star::uno::RuntimeException);
    // XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  )
        throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setPropertyValue( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue )
        throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const ::rtl::OUString& PropertyName )
        throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addPropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener )
        throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removePropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& aListener )
        throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener )
        throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener )
        throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    // Uno componentiseralation
    static ::rtl::OUString getImplementationName();
    static ::com::sun::star::uno::Sequence < ::rtl::OUString > getSupportedServiceNames();
    static ::com::sun::star::uno::Reference < com::sun::star::lang::XSingleServiceFactory > createServiceFactory( com::sun::star::uno::Reference < com::sun::star::lang::XMultiServiceFactory > const & rServiceFactory );
    virtual sal_Bool SAL_CALL supportsService(rtl::OUString const & rServiceName)
        throw (com::sun::star::uno::RuntimeException);
};
#endif
