/*************************************************************************
 *
 *  $RCSfile: ZipPackage.hxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: mtg $ $Date: 2000-11-29 03:18:48 $
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
#ifndef _ZIP_PACKAGE_HXX
#define _ZIP_PACKAGE_HXX

#ifndef _CPPUHELPER_IMPLBASE4_HXX_
#include <cppuhelper/implbase4.hxx> // helper for implementations
#endif

#ifndef _CPPUHELPER_FACTORY_HXX_
#include <cppuhelper/factory.hxx>
#endif

#ifndef _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XHIERARCHICALNAMEACCESS_HPP_
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XSINGLESERVICEFACTORY_HPP_
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#endif

#ifndef _COM_SUN_STAR_UTIL_XCHANGESBATCH_HPP_
#include <com/sun/star/util/XChangesBatch.hpp>
#endif

#ifndef _UCBHELPER_CONTENT_HXX
#include <ucbhelper/content.hxx>
#endif

#ifndef _ZIP_FILE_HXX
#include "ZipFile.hxx"
#endif

#ifndef _ZIP_OUTPUT_STREAM_HXX
#include "ZipOutputStream.hxx"
#endif

#ifndef _ZIP_PACKAGE_FOLDER_HXX
#include "ZipPackageFolder.hxx"
#endif

#ifndef _ZIP_PACKAGE_STREAM_HXX
#include "ZipPackageStream.hxx"
#endif

#ifndef _ZIP_PACKAGE_SINK_HXX
#include "ZipPackageSink.hxx"
#endif

#ifndef _ZIP_PACKAGE_BUFFER_HXX
#include "ZipPackageBuffer.hxx"
#endif

#ifdef _DEBUG_RECURSION_
#include "testzip.hxx"
#endif

#ifndef _MANIFEST_ENTRY_HXX
#include "ManifestEntry.hxx"
#endif

#ifndef _MANIFEST_WRITER_HXX
#include "ManifestWriter.hxx"
#endif

#ifndef _MANIFEST_READER_HXX
#include "ManifestReader.hxx"
#endif

#include <vector>

class ZipPackage : public cppu::WeakImplHelper4<
                        com::sun::star::lang::XInitialization,
                        com::sun::star::container::XHierarchicalNameAccess,
                        com::sun::star::lang::XSingleServiceFactory,
                        com::sun::star::util::XChangesBatch
                        >
{
private:
    ZipPackageFolder *pRootFolder;
    ZipFile          *pZipFile;
    ::ucb::Content   *pContent;
    ::std::vector < com::sun::star::uno::Reference < com::sun::star::lang::XSingleServiceFactory > > aContainedZips;
    ::com::sun::star::uno::Reference < com::sun::star::package::XZipFile > xZipFile;
    ::com::sun::star::uno::Reference < com::sun::star::io::XOutputStream > xBuffer;
    ::com::sun::star::uno::Reference < com::sun::star::lang::XUnoTunnel > xRootFolder;
    ::com::sun::star::uno::Reference < com::sun::star::io::XInputStream > xContentStream;
    ::com::sun::star::uno::Reference < com::sun::star::io::XSeekable > xContentSeek;
    const ::com::sun::star::uno::Reference < com::sun::star::lang::XMultiServiceFactory > xFactory;
    sal_Bool isZipFile(com::sun::star::package::ZipEntry &rEntry);
    void getZipFileContents();
    void destroyFolderTree( ::com::sun::star::uno::Reference < ::com::sun::star::lang::XUnoTunnel > xFolder );
public:
    ZipPackage (com::sun::star::uno::Reference < com::sun::star::io::XInputStream > &xInput,
                const ::com::sun::star::uno::Reference < com::sun::star::lang::XMultiServiceFactory > &xNewFactory);
    ZipPackage (const ::com::sun::star::uno::Reference < com::sun::star::lang::XMultiServiceFactory > &xNewFactory);
    ZipPackageFolder * getRootFolder()
    {
        return pRootFolder;
    }
    virtual ~ZipPackage( void );
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
    // Uno componentiseralation
    com::sun::star::uno::Reference< com::sun::star::uno::XInterface > ZipFile_create(
            const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory > & xMgr );
};
#endif
