/*************************************************************************
 *
 *  $RCSfile: ZipPackageFolder.hxx,v $
 *
 *  $Revision: 1.16 $
 *
 *  last change: $Author: mtg $ $Date: 2001-03-16 17:11:40 $
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
#ifndef _ZIP_PACKAGE_FOLDER_HXX
#define _ZIP_PACKAGE_FOLDER_HXX

#ifndef _COM_SUN_STAR_IO_XOUTPUTSTREAM_HPP_
#include <com/sun/star/io/XOutputStream.hpp>
#endif

#ifndef _COM_SUN_STAR_IO_XACTIVEDATASINK_HPP_
#include <com/sun/star/io/XActiveDataSink.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XENUMERATIONACCESS_HPP_
#include <com/sun/star/container/XEnumerationAccess.hpp>
#endif

#ifndef _COM_SUN_STAR_PACKAGE_ZIPENTRY_HPP_
#include <com/sun/star/packages/ZipEntry.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XUNOTUNNEl_HPP_
#include <com/sun/star/lang/XUnoTunnel.hpp>
#endif

#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif

#ifndef _COM_SUN_STAR_LANG_XSINGLESERVICEFACTORY_HPP_
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#endif

#include <hash_map>

struct eqFunc
{
    sal_Bool operator()( const rtl::OUString &r1,
                         const rtl::OUString &r2) const
    {
        return r1 == r2;
    }
};

struct hashFunc
{
    sal_Int32 operator()(const rtl::OUString &r1) const
    {
        return r1.hashCode();
    }
};
typedef std::hash_map < rtl::OUString,
                        com::sun::star::uno::Reference < com::sun::star::lang::XUnoTunnel >,
                        hashFunc,
                        eqFunc > TunnelHash;

typedef std::hash_map < rtl::OUString,
                        com::sun::star::uno::Reference < com::sun::star::container::XNameContainer >,
                        hashFunc,
                        eqFunc > NameHash;

/* This include must be after the above struct and typedef declarations.
 * Ugly...but true :)
 */

#ifndef _ZIP_PACKAGE_FOLDER_ENUMERATION_HXX
#include "ZipPackageFolderEnumeration.hxx"
#endif

#ifndef _ZIP_PACKAGE_ENTRY_HXX
#include "ZipPackageEntry.hxx"
#endif

#ifndef _ZIP_PACKAGE_STREAM_HXX
#include "ZipPackageStream.hxx"
#endif

#ifndef _ZIP_PACKAGE_HXX
#include "ZipPackage.hxx"
#endif

#ifndef _ZIP_PACKAGE_BUFFER_HXX
#include "ZipPackageBuffer.hxx"
#endif

#ifndef _ZIP_OUTPUT_STREAM_HXX
#include "ZipOutputStream.hxx"
#endif

#ifndef _ZIP_FILE_HXX
#include "ZipFile.hxx"
#endif

#ifndef _MANIFEST_ENTRY_HXX
#include "ManifestEntry.hxx"
#endif

class ZipPackage;
class ZipPackageFolder : public ZipPackageEntry,
                         public ::cppu::OWeakObject,
                         public ::com::sun::star::container::XNameContainer,
                         public ::com::sun::star::container::XEnumerationAccess
{
private:
    TunnelHash aContents;
public:
    ZipPackage *pPackage;
    com::sun::star::uno::Reference < com::sun::star::lang::XSingleServiceFactory > xPackage;

    ZipPackageFolder ( void );
    virtual ~ZipPackageFolder( void );

    static void copyZipEntry( com::sun::star::packages::ZipEntry &rDest, const com::sun::star::packages::ZipEntry &rSource);
    // Recursive functions
    void  saveContents(rtl::OUString &rPath, std::vector < ManifestEntry * > &rManList, ZipOutputStream & rZipOut)
        throw(::com::sun::star::uno::RuntimeException);
    void  updateReferences( ZipFile * pNewZipFile);
    void  releaseUpwardRef( void );

    // XInterface
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& rType )
        throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL acquire(  )
        throw();
    virtual void SAL_CALL release(  )
        throw();

    // XNameContainer
    virtual void SAL_CALL insertByName( const ::rtl::OUString& aName, const ::com::sun::star::uno::Any& aElement )
        throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::container::ElementExistException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeByName( const ::rtl::OUString& Name )
        throw(::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    // XEnumerationAccess
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration > SAL_CALL createEnumeration(  )
        throw(::com::sun::star::uno::RuntimeException);

    // XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType(  )
        throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements(  )
        throw(::com::sun::star::uno::RuntimeException);

    // XNameAccess
    virtual ::com::sun::star::uno::Any SAL_CALL getByName( const ::rtl::OUString& aName )
        throw(::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getElementNames(  )
        throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasByName( const ::rtl::OUString& aName )
        throw(::com::sun::star::uno::RuntimeException);

    // XNameReplace
    virtual void SAL_CALL replaceByName( const ::rtl::OUString& aName, const ::com::sun::star::uno::Any& aElement )
        throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    // XUnoTunnel
    static ::com::sun::star::uno::Sequence < sal_Int8 > getUnoTunnelImplementationId( void )
        throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier )
        throw(::com::sun::star::uno::RuntimeException);
};
#endif
