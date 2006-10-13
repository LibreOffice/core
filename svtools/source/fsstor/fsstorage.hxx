/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fsstorage.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-10-13 11:26:44 $
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

#ifndef __XSTORAGE_HXX_
#define __XSTORAGE_HXX_

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

#ifndef _COM_SUN_STAR_EMBED_XSTORAGE_HPP_
#include <com/sun/star/embed/XStorage.hpp>
#endif

#ifndef _COM_SUN_STAR_EMBED_XHIERARCHICALSTORAGEACCESS_HPP_
#include <com/sun/star/embed/XHierarchicalStorageAccess.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif

#ifndef _COM_SUN_STAR_IO_XSTREAM_HPP_
#include <com/sun/star/io/XStream.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XSINGLESERVICEFACTORY_HPP_
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XTYPEPROVIDER_HPP_
#include <com/sun/star/lang/XTypeProvider.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif

#ifndef _COM_SUN_STAR_PACKAGES_NOENCRYPTIONEXCEPTION_HPP_
#include <com/sun/star/packages/NoEncryptionException.hpp>
#endif

#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif

#ifndef _CPPUHELPER_INTERFACECONTAINER_H_
#include <cppuhelper/interfacecontainer.h>
#endif

#include <ucbhelper/content.hxx>

struct FSStorage_Impl;
class FSStorage : public ::com::sun::star::lang::XTypeProvider
                , public ::com::sun::star::embed::XStorage
                , public ::com::sun::star::embed::XHierarchicalStorageAccess
                , public ::com::sun::star::beans::XPropertySet
                , public ::cppu::OWeakObject
{
    ::osl::Mutex m_aMutex;
    FSStorage_Impl* m_pImpl;

protected:

public:

    FSStorage(  const ::ucb::Content& aContent,
                sal_Int32 nMode,
                ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > xProperties,
                ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xFactory );

    virtual ~FSStorage();

    ::ucb::Content* GetContent();

    void CopyStreamToSubStream( const ::rtl::OUString& aSourceURL,
                                const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xDest,
                                const ::rtl::OUString& aNewEntryName );

    void CopyContentToStorage_Impl( ::ucb::Content* pContent,
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xDest );

    static sal_Bool MakeFolderNoUI( const String& rFolder, sal_Bool bNewOnly );

    //____________________________________________________________________________________________________
    //  XInterface
    //____________________________________________________________________________________________________

    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& rType )
        throw( ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL acquire() throw();

    virtual void SAL_CALL release() throw();

    //____________________________________________________________________________________________________
    //  XTypeProvider
    //____________________________________________________________________________________________________

    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes()
        throw( ::com::sun::star::uno::RuntimeException );

    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId()
        throw( ::com::sun::star::uno::RuntimeException );

    //____________________________________________________________________________________________________
    //  XStorage
    //____________________________________________________________________________________________________

    virtual void SAL_CALL copyToStorage( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xDest )
        throw ( ::com::sun::star::embed::InvalidStorageException,
                ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::io::IOException,
                ::com::sun::star::embed::StorageWrappedTargetException,
                ::com::sun::star::uno::RuntimeException );

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XStream > SAL_CALL openStreamElement(
            const ::rtl::OUString& aStreamName, sal_Int32 nOpenMode )
        throw ( ::com::sun::star::embed::InvalidStorageException,
                ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::packages::WrongPasswordException,
                ::com::sun::star::io::IOException,
                ::com::sun::star::embed::StorageWrappedTargetException,
                ::com::sun::star::uno::RuntimeException );

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XStream > SAL_CALL openEncryptedStreamElement(
            const ::rtl::OUString& aStreamName, sal_Int32 nOpenMode, const ::rtl::OUString& aPass )
        throw ( ::com::sun::star::embed::InvalidStorageException,
                ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::packages::NoEncryptionException,
                ::com::sun::star::packages::WrongPasswordException,
                ::com::sun::star::io::IOException,
                ::com::sun::star::embed::StorageWrappedTargetException,
                ::com::sun::star::uno::RuntimeException );

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage > SAL_CALL openStorageElement(
            const ::rtl::OUString& aStorName, sal_Int32 nStorageMode )
        throw ( ::com::sun::star::embed::InvalidStorageException,
                ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::io::IOException,
                ::com::sun::star::embed::StorageWrappedTargetException,
                ::com::sun::star::uno::RuntimeException );

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XStream > SAL_CALL cloneStreamElement(
            const ::rtl::OUString& aStreamName )
        throw ( ::com::sun::star::embed::InvalidStorageException,
                ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::packages::WrongPasswordException,
                ::com::sun::star::io::IOException,
                ::com::sun::star::embed::StorageWrappedTargetException,
                ::com::sun::star::uno::RuntimeException );

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XStream > SAL_CALL cloneEncryptedStreamElement(
            const ::rtl::OUString& aStreamName, const ::rtl::OUString& aPass )
        throw ( ::com::sun::star::embed::InvalidStorageException,
                ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::packages::NoEncryptionException,
                ::com::sun::star::packages::WrongPasswordException,
                ::com::sun::star::io::IOException,
                ::com::sun::star::embed::StorageWrappedTargetException,
                ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL copyLastCommitTo(
            const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xTargetStorage )
        throw ( ::com::sun::star::embed::InvalidStorageException,
                ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::io::IOException,
                ::com::sun::star::embed::StorageWrappedTargetException,
                ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL copyStorageElementLastCommitTo(
            const ::rtl::OUString& aStorName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xTargetStorage )
        throw ( ::com::sun::star::embed::InvalidStorageException,
                ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::io::IOException,
                ::com::sun::star::embed::StorageWrappedTargetException,
                ::com::sun::star::uno::RuntimeException );

    virtual sal_Bool SAL_CALL isStreamElement( const ::rtl::OUString& aElementName )
        throw ( ::com::sun::star::container::NoSuchElementException,
                ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::embed::InvalidStorageException,
                ::com::sun::star::uno::RuntimeException );

    virtual sal_Bool SAL_CALL isStorageElement( const ::rtl::OUString& aElementName )
        throw ( ::com::sun::star::container::NoSuchElementException,
                ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::embed::InvalidStorageException,
                ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL removeElement( const ::rtl::OUString& aElementName )
        throw ( ::com::sun::star::embed::InvalidStorageException,
                ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::container::NoSuchElementException,
                ::com::sun::star::io::IOException,
                ::com::sun::star::embed::StorageWrappedTargetException,
                ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL renameElement( const ::rtl::OUString& rEleName, const ::rtl::OUString& rNewName )
        throw ( ::com::sun::star::embed::InvalidStorageException,
                ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::container::NoSuchElementException,
                ::com::sun::star::container::ElementExistException,
                ::com::sun::star::io::IOException,
                ::com::sun::star::embed::StorageWrappedTargetException,
                ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL copyElementTo(    const ::rtl::OUString& aElementName,
                                        const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xDest,
                                        const ::rtl::OUString& aNewName )
        throw ( ::com::sun::star::embed::InvalidStorageException,
                ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::container::NoSuchElementException,
                ::com::sun::star::container::ElementExistException,
                ::com::sun::star::io::IOException,
                ::com::sun::star::embed::StorageWrappedTargetException,
                ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL moveElementTo(    const ::rtl::OUString& aElementName,
                                        const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xDest,
                                        const ::rtl::OUString& rNewName )
        throw ( ::com::sun::star::embed::InvalidStorageException,
                ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::container::NoSuchElementException,
                ::com::sun::star::container::ElementExistException,
                ::com::sun::star::io::IOException,
                ::com::sun::star::embed::StorageWrappedTargetException,
                ::com::sun::star::uno::RuntimeException );

    //____________________________________________________________________________________________________
    //  XNameAccess
    //____________________________________________________________________________________________________

    virtual ::com::sun::star::uno::Any SAL_CALL getByName( const ::rtl::OUString& aName )
        throw ( ::com::sun::star::container::NoSuchElementException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException );

    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getElementNames()
        throw ( ::com::sun::star::uno::RuntimeException );

    virtual sal_Bool SAL_CALL hasByName( const ::rtl::OUString& aName )
        throw ( ::com::sun::star::uno::RuntimeException );

    virtual ::com::sun::star::uno::Type SAL_CALL getElementType()
        throw ( ::com::sun::star::uno::RuntimeException );

    virtual sal_Bool SAL_CALL hasElements()
        throw ( ::com::sun::star::uno::RuntimeException );

    //____________________________________________________________________________________________________
    //  XComponent
    //____________________________________________________________________________________________________

    virtual void SAL_CALL dispose()
        throw ( ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL addEventListener(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener )
        throw ( ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL removeEventListener(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener )
        throw ( ::com::sun::star::uno::RuntimeException );

    //____________________________________________________________________________________________________
    //  XPropertySet
    //____________________________________________________________________________________________________

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo()
        throw ( ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL setPropertyValue( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue )
        throw ( ::com::sun::star::beans::UnknownPropertyException,
                ::com::sun::star::beans::PropertyVetoException,
                ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException );

    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const ::rtl::OUString& PropertyName )
        throw ( ::com::sun::star::beans::UnknownPropertyException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL addPropertyChangeListener(
            const ::rtl::OUString& aPropertyName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener )
        throw ( ::com::sun::star::beans::UnknownPropertyException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL removePropertyChangeListener(
            const ::rtl::OUString& aPropertyName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& aListener )
        throw ( ::com::sun::star::beans::UnknownPropertyException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL addVetoableChangeListener(
            const ::rtl::OUString& PropertyName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener )
        throw ( ::com::sun::star::beans::UnknownPropertyException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL removeVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener )
        throw ( ::com::sun::star::beans::UnknownPropertyException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException );

    //____________________________________________________________________________________________________
    //  XHierarchicalStorageAccess
    //____________________________________________________________________________________________________

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::embed::XExtendedStorageStream > SAL_CALL openStreamElementByHierarchicalName( const ::rtl::OUString& sStreamPath, ::sal_Int32 nOpenMode )
        throw ( ::com::sun::star::embed::InvalidStorageException,
                ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::packages::WrongPasswordException,
                ::com::sun::star::io::IOException,
                ::com::sun::star::embed::StorageWrappedTargetException,
                ::com::sun::star::uno::RuntimeException );

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::embed::XExtendedStorageStream > SAL_CALL openEncryptedStreamElementByHierarchicalName( const ::rtl::OUString& sStreamName, ::sal_Int32 nOpenMode, const ::rtl::OUString& sPassword )
        throw ( ::com::sun::star::embed::InvalidStorageException,
                ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::packages::NoEncryptionException,
                ::com::sun::star::packages::WrongPasswordException,
                ::com::sun::star::io::IOException,
                ::com::sun::star::embed::StorageWrappedTargetException,
                ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL removeStreamElementByHierarchicalName( const ::rtl::OUString& sElementPath )
        throw ( ::com::sun::star::embed::InvalidStorageException,
                ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::container::NoSuchElementException,
                ::com::sun::star::io::IOException,
                ::com::sun::star::embed::StorageWrappedTargetException,
                ::com::sun::star::uno::RuntimeException );
};

#endif

