/*************************************************************************
 *
 *  $RCSfile: xstorage.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2003-10-30 09:48:40 $
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

#ifndef __XSTORAGE_HXX_
#define __XSTORAGE_HXX_


#ifndef _COM_SUN_STAR_EMBED_XSTORAGE_HPP_
#include <com/sun/star/embed/XStorage.hpp>
#endif

#ifndef _COM_SUN_STAR_EMBED_XTRANSACTEDOBJECT_HPP_
#include <com/sun/star/embed/XTransactedObject.hpp>
#endif

#ifndef _COM_SUN_STAR_EMBED_XTRANSACTIONBROADCASTER_HPP_
#include <com/sun/star/embed/XTransactionBroadcaster.hpp>
#endif

#ifndef _COM_SUN_STAR_EMBED_XCLASSIFIEDOBJECT_HPP_
#include <com/sun/star/embed/XClassifiedObject.hpp>
#endif

#ifndef _COM_SUN_STAR_EMBED_XENCRYPTIONPROTECTEDSOURCE_HPP_
#include <com/sun/star/embed/XEncryptionProtectedSource.hpp>
#endif

#ifndef _COM_SUN_STAR_UTIL_XMODIFIABLE_HPP_
#include <com/sun/star/util/XModifiable.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif

#ifndef _COM_SUN_STAR_UTIL_XCLOSEABLE_HPP_
#include <com/sun/star/util/XCloseable.hpp>
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

#include <list>

#include "mutexholder.hxx"


namespace cppu
{
    class OTypeCollection;
};

struct StorInternalData_Impl
{
    SotMutexHolderRef m_rSharedMutexRef;
    ::cppu::OMultiTypeInterfaceContainerHelper m_aListenersContainer; // list of listeners
    ::cppu::OTypeCollection* m_pTypeCollection;
    sal_Bool m_bIsRoot;

    // the mutex reference MUST NOT be empty
    StorInternalData_Impl( const SotMutexHolderRef& rMutexRef, sal_Bool bRoot )
    : m_rSharedMutexRef( rMutexRef )
    , m_aListenersContainer( rMutexRef->GetMutex() )
    , m_pTypeCollection( NULL )
    , m_bIsRoot( bRoot )
    {}

    ~StorInternalData_Impl();
};

//================================================
// a common implementation for an entry

struct OStorage_Impl;
struct OWriteStream_Impl;

struct SotElement_Impl
{
    ::rtl::OUString             m_aName;
    ::rtl::OUString             m_aOriginalName;
    sal_Bool                    m_bIsRemoved;
    sal_Bool                    m_bIsInserted;
    sal_Bool                    m_bIsStorage;

    OStorage_Impl*              m_pStorage;
    OWriteStream_Impl*          m_pStream;

public:
                                SotElement_Impl( const ::rtl::OUString& rName, sal_Bool bStor, sal_Bool bNew );
                                ~SotElement_Impl();
};

typedef ::std::list< SotElement_Impl* > SotElementList_Impl;

//=========================================================================
// Main storage implementation

class OStorage;

struct OStorage_Impl
{
    SotMutexHolderRef           m_rMutexRef;

    OStorage*                   m_pAntiImpl;    // only valid if external references exists

    sal_Int32                   m_nStorageMode; // open mode ( read/write/trunc/nocreate )
    sal_Bool                    m_bIsModified;  // only modified elements will be sent to the original content
    sal_Bool                    m_bCommited;    // sending the streams is coordinated by the root storage of the package

    sal_Bool                    m_bIsRoot;      // marks this storage as root storages that manages all commits and reverts
    sal_Bool                    m_bListCreated;


    SotElementList_Impl                         m_aChildrenList;
    SotElementList_Impl                         m_aDeletedList;

    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer > m_xPackageFolder;

    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XSingleServiceFactory > m_xPackage;
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >  m_xFactory;

    // valid only for root storage
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > m_xInputStream; // ??? may be stored in properties
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XStream > m_xStream; // ??? may be stored in properties
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > m_xProperties;

    // must be empty in case of root storage
    OStorage_Impl* m_pParent;

    sal_Bool        m_bControlMediaType;
    ::rtl::OUString m_aMediaType;

    //////////////////////////////////////////
    // Constructors

    OStorage_Impl(  ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > xInputStream,
                    sal_Int32 nMode,
                    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > xProperties,
                    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xFactory );

    OStorage_Impl(  ::com::sun::star::uno::Reference< ::com::sun::star::io::XStream > xStream,
                    sal_Int32 nMode,
                    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > xProperties,
                    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xFactory );

    // constructor for a substorage
    OStorage_Impl(  OStorage_Impl* pParent,
                    sal_Int32 nMode,
                    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer > xPackageFolder,
                    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XSingleServiceFactory > xPackage,
                    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xFactory );

    ~OStorage_Impl();

    void OpenOwnPackage();
    void ReadContents();

    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > GetServiceFactory();
    SotElementList_Impl& GetChildrenList();
    void GetStorageProperties();

    void InsertIntoPackageFolder(
            const ::rtl::OUString& aName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >& xParentPackageFolder );

    void Commit();
    void Revert();

    void SetModifiedInternally( sal_Bool bModified );

    void CopyToStorage( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xDest );
    void CopyStorageElement( SotElement_Impl* pElement,
                            ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage > xDest,
                            ::rtl::OUString aName );

    void SetModified( sal_Bool bModified );

    SotElement_Impl* FindElement( const ::rtl::OUString& rName );


    SotElement_Impl* InsertStream( ::rtl::OUString aName, sal_Bool bEncr );
    SotElement_Impl* InsertRawStream( ::rtl::OUString aName, const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& xInStream );

    SotElement_Impl* InsertStorage( ::rtl::OUString aName, sal_Int32 nStorageMode );
    SotElement_Impl* InsertElement( ::rtl::OUString aName, sal_Bool bIsStorage );

    void OpenSubStorage( SotElement_Impl* pElement, sal_Int32 nStorageMode );
    void OpenSubStream( SotElement_Impl* pElement );

    ::com::sun::star::uno::Sequence< ::rtl::OUString > GetElementNames();

    void RemoveElement( SotElement_Impl* pElement );
    void ClearElement( SotElement_Impl* pElement );
    void DisposeChildren();
};


class OStorage  : public ::com::sun::star::lang::XTypeProvider
                , public ::com::sun::star::embed::XStorage
                , public ::com::sun::star::embed::XTransactedObject
                , public ::com::sun::star::embed::XTransactionBroadcaster
                , public ::com::sun::star::util::XModifiable
                , public ::com::sun::star::container::XNameAccess
                , public ::com::sun::star::lang::XComponent
                , public ::com::sun::star::embed::XEncryptionProtectedSource
                , public ::com::sun::star::beans::XPropertySet
                , public ::cppu::OWeakObject
{
    OStorage_Impl*  m_pImpl;
    StorInternalData_Impl* m_pData;

protected:

    void Commit_Impl();

    SotElement_Impl* OpenStreamElement_Impl( const ::rtl::OUString& aStreamName, sal_Int32 nOpenMode, sal_Bool bEncr );

    void BroadcastModified();

    void BroadcastTransaction( sal_Int8 nMessage );

public:

    OStorage(   ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > xInputStream,
                sal_Int32 nMode,
                ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > xProperties,
                ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xFactory );

    OStorage(   ::com::sun::star::uno::Reference< ::com::sun::star::io::XStream > xStream,
                sal_Int32 nMode,
                ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > xProperties,
                ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xFactory );

    OStorage(   OStorage_Impl* pImpl );

    virtual ~OStorage();

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
                ::com::sun::star::embed::StorageWTException,
                ::com::sun::star::uno::RuntimeException );

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XStream > SAL_CALL openStreamElement(
            const ::rtl::OUString& aStreamName, sal_Int32 nOpenMode )
        throw ( ::com::sun::star::embed::InvalidStorageException,
                ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::packages::WrongPasswordException,
                ::com::sun::star::io::IOException,
                ::com::sun::star::embed::StorageWTException,
                ::com::sun::star::uno::RuntimeException );

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XStream > SAL_CALL openEncryptedStreamElement(
            const ::rtl::OUString& aStreamName, sal_Int32 nOpenMode, const ::com::sun::star::uno::Sequence< sal_Int8 >& aKey )
        throw ( ::com::sun::star::embed::InvalidStorageException,
                ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::packages::NoEncryptionException,
                ::com::sun::star::packages::WrongPasswordException,
                ::com::sun::star::io::IOException,
                ::com::sun::star::embed::StorageWTException,
                ::com::sun::star::uno::RuntimeException );

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage > SAL_CALL openStorageElement(
            const ::rtl::OUString& aStorName, sal_Int32 nStorageMode )
        throw ( ::com::sun::star::embed::InvalidStorageException,
                ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::io::IOException,
                ::com::sun::star::embed::StorageWTException,
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
                ::com::sun::star::embed::StorageWTException,
                ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL renameElement( const ::rtl::OUString& rEleName, const ::rtl::OUString& rNewName )
        throw ( ::com::sun::star::embed::InvalidStorageException,
                ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::container::NoSuchElementException,
                ::com::sun::star::container::ElementExistException,
                ::com::sun::star::io::IOException,
                ::com::sun::star::embed::StorageWTException,
                ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL copyElementTo(    const ::rtl::OUString& aElementName,
                                        const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xDest,
                                        const ::rtl::OUString& aNewName )
        throw ( ::com::sun::star::embed::InvalidStorageException,
                ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::container::NoSuchElementException,
                ::com::sun::star::container::ElementExistException,
                ::com::sun::star::io::IOException,
                ::com::sun::star::embed::StorageWTException,
                ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL moveElementTo(    const ::rtl::OUString& aElementName,
                                        const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xDest,
                                        const ::rtl::OUString& rNewName )
        throw ( ::com::sun::star::embed::InvalidStorageException,
                ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::container::NoSuchElementException,
                ::com::sun::star::container::ElementExistException,
                ::com::sun::star::io::IOException,
                ::com::sun::star::embed::StorageWTException,
                ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL insertRawEncrStreamElement( const ::rtl::OUString& aStreamName,
                                const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& xInStream )
        throw ( ::com::sun::star::embed::InvalidStorageException,
                ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::packages::NoRawFormatException,
                ::com::sun::star::container::ElementExistException,
                ::com::sun::star::io::IOException,
                ::com::sun::star::embed::StorageWTException,
                ::com::sun::star::uno::RuntimeException);

    //____________________________________________________________________________________________________
    // XTransactedObject
    //____________________________________________________________________________________________________

    virtual void SAL_CALL commit()
        throw ( ::com::sun::star::io::IOException,
                ::com::sun::star::embed::StorageWTException,
                ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL revert()
        throw ( ::com::sun::star::io::IOException,
                ::com::sun::star::embed::StorageWTException,
                ::com::sun::star::uno::RuntimeException );

    //____________________________________________________________________________________________________
    // XTransactionBroadcaster
    //____________________________________________________________________________________________________

    virtual void SAL_CALL addTransactionListener(
            const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XTransactionListener >& aListener )
        throw ( ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL removeTransactionListener(
            const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XTransactionListener >& aListener )
        throw ( ::com::sun::star::uno::RuntimeException );

    //____________________________________________________________________________________________________
    //  XModifiable
    //____________________________________________________________________________________________________

    virtual sal_Bool SAL_CALL isModified()
        throw ( ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL setModified( sal_Bool bModified )
        throw ( ::com::sun::star::beans::PropertyVetoException,
                ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL addModifyListener(
            const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >& aListener )
        throw ( ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL removeModifyListener(
            const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >& aListener )
        throw ( ::com::sun::star::uno::RuntimeException );

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
    //  XEncryptionProtectedSource
    //____________________________________________________________________________________________________

    virtual void SAL_CALL setEncryptionKey( const ::com::sun::star::uno::Sequence< sal_Int8 >& aKey )
        throw( ::com::sun::star::uno::RuntimeException );

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

};

#endif

