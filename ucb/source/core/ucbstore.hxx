/*************************************************************************
 *
 *  $RCSfile: ucbstore.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kso $ $Date: 2000-10-25 06:32:48 $
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
 *  Contributor(s): Kai Sommerfeld ( kso@sun.com )
 *
 *
 ************************************************************************/

#ifndef _UCBSTORE_HXX
#define _UCBSTORE_HXX

#ifndef _COM_SUN_STAR_LANG_XTYPEPROVIDER_HPP_
#include <com/sun/star/lang/XTypeProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMED_HPP_
#include <com/sun/star/container/XNamed.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XPROPERTYSETREGISTRYFACTORY_HPP_
#include <com/sun/star/ucb/XPropertySetRegistryFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XPROPERTYSETREGISTRY_HPP_
#include <com/sun/star/ucb/XPropertySetRegistry.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XPERSISTENTPROPERTYSET_HPP_
#include <com/sun/star/ucb/XPersistentPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYCONTAINER_HPP_
#include <com/sun/star/beans/XPropertyContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSETINFOCHANGENOTIFIER_HPP_
#include <com/sun/star/beans/XPropertySetInfoChangeNotifier.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYACCESS_HPP_
#include <com/sun/star/beans/XPropertyAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif

#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif

#ifndef _UCBHELPER_MACROS_HXX
#include <ucbhelper/macros.hxx>
#endif

//=========================================================================

#define STORE_SERVICE_NAME          "com.sun.star.ucb.Store"
#define PROPSET_REG_SERVICE_NAME    "com.sun.star.ucb.PropertySetRegistry"
#define PERS_PROPSET_SERVICE_NAME   "com.sun.star.ucb.PersistentPropertySet"

//=========================================================================

struct UcbStore_Impl;

class UcbStore :
                public cppu::OWeakObject,
                public com::sun::star::lang::XTypeProvider,
                public com::sun::star::lang::XServiceInfo,
                public com::sun::star::ucb::XPropertySetRegistryFactory
{
    com::sun::star::uno::Reference<
                com::sun::star::lang::XMultiServiceFactory > m_xSMgr;
    UcbStore_Impl* m_pImpl;

public:
    UcbStore(
        const com::sun::star::uno::Reference<
                com::sun::star::lang::XMultiServiceFactory >& rXSMgr );
    virtual ~UcbStore();

    // XInterface
    XINTERFACE_DECL()

    // XTypeProvider
    XTYPEPROVIDER_DECL()

    // XServiceInfo
    XSERVICEINFO_DECL()

    // XPropertySetRegistryFactory
    virtual com::sun::star::uno::Reference<
                com::sun::star::ucb::XPropertySetRegistry > SAL_CALL
    createPropertySetRegistry( const rtl::OUString& URL )
        throw( com::sun::star::uno::RuntimeException );

    // New
    void removeRegistry( const rtl::OUString& URL );
};

//=========================================================================

struct PropertySetRegistry_Impl;

class PropertySetRegistry :
                public cppu::OWeakObject,
                public com::sun::star::lang::XTypeProvider,
                public com::sun::star::lang::XServiceInfo,
                public com::sun::star::ucb::XPropertySetRegistry,
                public com::sun::star::container::XNameAccess
{
    friend class PersistentPropertySet;

    com::sun::star::uno::Reference<
                com::sun::star::lang::XMultiServiceFactory > m_xSMgr;
    PropertySetRegistry_Impl* m_pImpl;

private:
    void add   ( PersistentPropertySet* pSet );
    void remove( PersistentPropertySet* pSet );

    void renamePropertySet( const rtl::OUString& rOldKey,
                            const rtl::OUString& rNewKey );

public:
    PropertySetRegistry(
        const com::sun::star::uno::Reference<
                com::sun::star::lang::XMultiServiceFactory >& rXSMgr,
        UcbStore& rCreator );
    virtual ~PropertySetRegistry();

    // XInterface
    XINTERFACE_DECL()

    // XTypeProvider
    XTYPEPROVIDER_DECL()

    // XServiceInfo
    XSERVICEINFO_NOFACTORY_DECL()

    // XPropertySetRegistry
    virtual com::sun::star::uno::Reference<
                com::sun::star::ucb::XPersistentPropertySet > SAL_CALL
    openPropertySet( const rtl::OUString& key, sal_Bool create )
        throw( com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL
    removePropertySet( const rtl::OUString& key )
        throw( com::sun::star::uno::RuntimeException );

    // XElementAccess ( XNameAccess is derived from it )
    virtual com::sun::star::uno::Type SAL_CALL
    getElementType()
        throw( com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL
    hasElements()
        throw( com::sun::star::uno::RuntimeException );

    // XNameAccess
    virtual com::sun::star::uno::Any SAL_CALL
    getByName( const rtl::OUString& aName )
        throw( com::sun::star::container::NoSuchElementException,
               com::sun::star::lang::WrappedTargetException,
               com::sun::star::uno::RuntimeException );
    virtual com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL
    getElementNames()
        throw( com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL
    hasByName( const rtl::OUString& aName )
        throw( com::sun::star::uno::RuntimeException );

    // Non-interface methods
    com::sun::star::uno::Reference< com::sun::star::uno::XInterface >
    getRootConfigReadAccess();
    com::sun::star::uno::Reference< com::sun::star::uno::XInterface >
    getConfigWriteAccess( const rtl::OUString& rPath );

};

//=========================================================================

struct PersistentPropertySet_Impl;

class PersistentPropertySet :
                public cppu::OWeakObject,
                public com::sun::star::lang::XTypeProvider,
                public com::sun::star::lang::XServiceInfo,
                public com::sun::star::lang::XComponent,
                public com::sun::star::ucb::XPersistentPropertySet,
                public com::sun::star::container::XNamed,
                public com::sun::star::beans::XPropertyContainer,
                public com::sun::star::beans::XPropertySetInfoChangeNotifier,
                public com::sun::star::beans::XPropertyAccess
{
    com::sun::star::uno::Reference<
            com::sun::star::lang::XMultiServiceFactory > m_xSMgr;
    PersistentPropertySet_Impl* m_pImpl;

private:
    void notifyPropertyChangeEvent(
        const com::sun::star::beans::PropertyChangeEvent& rEvent ) const;
    void notifyPropertySetInfoChange(
        const com::sun::star::beans::PropertySetInfoChangeEvent& evt ) const;

public:
    PersistentPropertySet(
        const com::sun::star::uno::Reference<
                com::sun::star::lang::XMultiServiceFactory >& rXSMgr,
        PropertySetRegistry& rCreator,
        const rtl::OUString& rKey );
    virtual ~PersistentPropertySet();

    // XInterface
    XINTERFACE_DECL()

    // XTypeProvider
    XTYPEPROVIDER_DECL()

    // XServiceInfo
    XSERVICEINFO_NOFACTORY_DECL()

    // XComponent
    virtual void SAL_CALL
    dispose()
        throw( com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL
    addEventListener( const com::sun::star::uno::Reference<
                            com::sun::star::lang::XEventListener >& Listener )
        throw( com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL
    removeEventListener( const com::sun::star::uno::Reference<
                            com::sun::star::lang::XEventListener >& Listener )
        throw( com::sun::star::uno::RuntimeException );

    // XPropertySet
    virtual com::sun::star::uno::Reference<
                com::sun::star::beans::XPropertySetInfo > SAL_CALL
    getPropertySetInfo()
        throw( com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL
    setPropertyValue( const rtl::OUString& aPropertyName,
                      const com::sun::star::uno::Any& aValue )
        throw( com::sun::star::beans::UnknownPropertyException,
               com::sun::star::beans::PropertyVetoException,
               com::sun::star::lang::IllegalArgumentException,
               com::sun::star::lang::WrappedTargetException,
               com::sun::star::uno::RuntimeException );
    virtual com::sun::star::uno::Any SAL_CALL
    getPropertyValue( const rtl::OUString& PropertyName )
        throw( com::sun::star::beans::UnknownPropertyException,
               com::sun::star::lang::WrappedTargetException,
               com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL
    addPropertyChangeListener( const rtl::OUString& aPropertyName,
                               const com::sun::star::uno::Reference<
                                   com::sun::star::beans::XPropertyChangeListener >& xListener )
        throw( com::sun::star::beans::UnknownPropertyException,
               com::sun::star::lang::WrappedTargetException,
               com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL
    removePropertyChangeListener( const rtl::OUString& aPropertyName,
                                  const com::sun::star::uno::Reference<
                                      com::sun::star::beans::XPropertyChangeListener >& aListener )
        throw( com::sun::star::beans::UnknownPropertyException,
               com::sun::star::lang::WrappedTargetException,
               com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL
    addVetoableChangeListener( const rtl::OUString& PropertyName,
                               const com::sun::star::uno::Reference<
                                   com::sun::star::beans::XVetoableChangeListener >& aListener )
        throw( com::sun::star::beans::UnknownPropertyException,
               com::sun::star::lang::WrappedTargetException,
               com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL
    removeVetoableChangeListener( const rtl::OUString& PropertyName,
                                  const com::sun::star::uno::Reference<
                                      com::sun::star::beans::XVetoableChangeListener >& aListener )
        throw( com::sun::star::beans::UnknownPropertyException,
               com::sun::star::lang::WrappedTargetException,
               com::sun::star::uno::RuntimeException );

    // XPersistentPropertySet
    virtual com::sun::star::uno::Reference<
                com::sun::star::ucb::XPropertySetRegistry > SAL_CALL
    getRegistry()
        throw( com::sun::star::uno::RuntimeException );
    virtual rtl::OUString SAL_CALL
    getKey()
        throw( com::sun::star::uno::RuntimeException );

    // XNamed
    virtual rtl::OUString SAL_CALL
    getName()
        throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL
    setName( const ::rtl::OUString& aName )
        throw( ::com::sun::star::uno::RuntimeException );

    // XPropertyContainer
    virtual void SAL_CALL
    addProperty( const rtl::OUString& Name,
                 sal_Int16 Attributes,
                 const com::sun::star::uno::Any& DefaultValue )
        throw( com::sun::star::beans::PropertyExistException,
               com::sun::star::beans::IllegalTypeException,
               com::sun::star::lang::IllegalArgumentException,
               com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL
    removeProperty( const rtl::OUString& Name )
        throw( com::sun::star::beans::UnknownPropertyException,
               com::sun::star::beans::NotRemoveableException,
               com::sun::star::uno::RuntimeException );

    // XPropertySetInfoChangeNotifier
    virtual void SAL_CALL
    addPropertySetInfoChangeListener( const com::sun::star::uno::Reference<
            com::sun::star::beans::XPropertySetInfoChangeListener >& Listener )
        throw( com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL
    removePropertySetInfoChangeListener( const com::sun::star::uno::Reference<
            com::sun::star::beans::XPropertySetInfoChangeListener >& Listener )
        throw( com::sun::star::uno::RuntimeException );

    // XPropertyAccess
    virtual com::sun::star::uno::Sequence<
                com::sun::star::beans::PropertyValue > SAL_CALL
    getPropertyValues()
        throw( com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL
    setPropertyValues( const com::sun::star::uno::Sequence<
                            com::sun::star::beans::PropertyValue >& aProps )
        throw( com::sun::star::beans::UnknownPropertyException,
               com::sun::star::beans::PropertyVetoException,
               com::sun::star::lang::IllegalArgumentException,
               com::sun::star::lang::WrappedTargetException,
               com::sun::star::uno::RuntimeException );

    // Non-interface methods.
    PropertySetRegistry& getPropertySetRegistry();
    const rtl::OUString&
    getFullKey( const com::sun::star::uno::Reference<
                            com::sun::star::uno::XInterface >& xEscaper );
};

#endif /* !_UCBSTORE_HXX */
