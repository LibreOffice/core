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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_toolkit.hxx"

#include <toolkit/controls/tabpagecontainer.hxx>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <toolkit/helper/unopropertyarrayhelper.hxx>
#include <toolkit/helper/property.hxx>
#include <com/sun/star/awt/XVclWindowPeer.hpp>
#include <comphelper/processfactory.hxx>
#include <osl/diagnose.h>
#include <com/sun/star/awt/XControlModel.hpp>

using ::rtl::OUString;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::view;

#define WRONG_TYPE_EXCEPTION "Type must be ::com::sun::star::awt::tab::XTabPage!"
//  ----------------------------------------------------
//  class UnoControlTabPageContainerModel
//  ----------------------------------------------------
UnoControlTabPageContainerModel::UnoControlTabPageContainerModel()
{
    ImplRegisterProperty( BASEPROPERTY_BACKGROUNDCOLOR );
    ImplRegisterProperty( BASEPROPERTY_BORDER );
    ImplRegisterProperty( BASEPROPERTY_BORDERCOLOR );
    ImplRegisterProperty( BASEPROPERTY_DEFAULTCONTROL );
    ImplRegisterProperty( BASEPROPERTY_ENABLED );
    ImplRegisterProperty( BASEPROPERTY_HELPTEXT );
    ImplRegisterProperty( BASEPROPERTY_HELPURL );
    ImplRegisterProperty( BASEPROPERTY_PRINTABLE );
    ImplRegisterProperty( BASEPROPERTY_TEXT );
}

::rtl::OUString UnoControlTabPageContainerModel::getServiceName() throw(::com::sun::star::uno::RuntimeException)
{
    return ::rtl::OUString::createFromAscii( szServiceName_UnoControlTabPageContainerModel );
}

uno::Any SAL_CALL UnoControlTabPageContainerModel::queryInterface( const uno::Type & rType )
     throw( uno::RuntimeException )
{
    Any aReturn = UnoControlModel::queryInterface( rType );
    if ( !aReturn.hasValue() )
         aReturn = ::cppu::queryInterface ( rType, static_cast< XIndexContainer*>( this ) );
     return aReturn;
}
Any UnoControlTabPageContainerModel::queryAggregation( const Type & rType ) throw(RuntimeException)
{
    uno::Any aAny;
     if( rType == ::getCppuType((const uno::Reference< XIndexContainer >*)0) )
         aAny <<= uno::Reference< XIndexContainer >( this );
     else if( rType == ::getCppuType((const uno::Reference< XContainer >*)0) )
         aAny <<= uno::Reference< XContainer >( this );
     else
        aAny <<= UnoControlModel::queryAggregation( rType );
    return aAny;
}
uno::Any UnoControlTabPageContainerModel::ImplGetDefaultValue( sal_uInt16 nPropId ) const
{
    switch(nPropId)
    {
        case BASEPROPERTY_DEFAULTCONTROL:
            return uno::makeAny( ::rtl::OUString::createFromAscii( szServiceName_UnoControlTabPageContainer ) );
        default:
            return UnoControlModel::ImplGetDefaultValue( nPropId );
    }
}

::cppu::IPropertyArrayHelper& UnoControlTabPageContainerModel::getInfoHelper()
{
     static UnoPropertyArrayHelper* pHelper = NULL;
     if ( !pHelper )
     {
        com::sun::star::uno::Sequence<sal_Int32>    aIDs = ImplGetPropertyIds();
         pHelper = new UnoPropertyArrayHelper( aIDs );
    }
     return *pHelper;
}
::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > UnoControlTabPageContainerModel::getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException)
{
    static ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}

void SAL_CALL UnoControlTabPageContainerModel::insertByIndex( ::sal_Int32 nIndex, const com::sun::star::uno::Any& aElement) throw (IllegalArgumentException, IndexOutOfBoundsException, WrappedTargetException, uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );
    uno::Reference < ::awt::tab::XTabPage > xTabPage;
    if(aElement >>= xTabPage)
    {
        if ( sal_Int32( m_aTabPageVector.size()) ==nIndex )
            m_aTabPageVector.push_back( xTabPage );
        else if ( sal_Int32( m_aTabPageVector.size()) > nIndex )
        {
            std::vector< uno::Reference< ::awt::tab::XTabPage > >::iterator aIter = m_aTabPageVector.begin();
            aIter += nIndex;
            m_aTabPageVector.insert( aIter, xTabPage );
        }
        else
            throw IndexOutOfBoundsException( ::rtl::OUString(), (OWeakObject *)this );
    }
    else
        throw IllegalArgumentException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( WRONG_TYPE_EXCEPTION )),
            (OWeakObject *)this, 2 );
}
// -----------------------------------------------------------------------------
void SAL_CALL UnoControlTabPageContainerModel::removeByIndex( ::sal_Int32 /*Index*/ ) throw (lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException)
{
}
// XIndexReplace
void SAL_CALL UnoControlTabPageContainerModel::replaceByIndex( ::sal_Int32 /*Index*/, const uno::Any& /*Element*/ ) throw (lang::IllegalArgumentException, lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException)
{
}
// -----------------------------------------------------------------------------
// XIndexAccess
::sal_Int32 SAL_CALL UnoControlTabPageContainerModel::getCount(  ) throw (uno::RuntimeException)
{
    sal_Int32 nElementCount = sal_Int32( m_aTabPageVector.size());
    return nElementCount >0 ? nElementCount : 0;
}
// -----------------------------------------------------------------------------
uno::Any SAL_CALL UnoControlTabPageContainerModel::getByIndex( ::sal_Int32 nIndex ) throw (lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException)
{
    (void)nIndex;
    return uno::Any();
}
// -----------------------------------------------------------------------------
// XElementAccess
uno::Type SAL_CALL UnoControlTabPageContainerModel::getElementType(  ) throw (uno::RuntimeException)
{
    return ::getCppuType(static_cast<  Reference< com::sun::star::awt::XControlModel>* >(NULL));
}
// -----------------------------------------------------------------------------
::sal_Bool SAL_CALL UnoControlTabPageContainerModel::hasElements(  ) throw (uno::RuntimeException)
{
    return false;
}
// XContainer
void UnoControlTabPageContainerModel::addContainerListener( const Reference< XContainerListener >& l ) throw(RuntimeException)
{
     (void) l;
}

void UnoControlTabPageContainerModel::removeContainerListener( const Reference< XContainerListener >& l ) throw(RuntimeException)
{
     (void) l;
}

//  ----------------------------------------------------
//  class UnoControlTabPageContainer
//  ----------------------------------------------------
UnoControlTabPageContainer::UnoControlTabPageContainer():
    m_aTabPageListeners( *this )
{
}

Any SAL_CALL UnoControlTabPageContainer::queryInterface( const Type & rType ) throw(RuntimeException)
{
    return UnoControlContainer::queryInterface(rType);
}

OUString UnoControlTabPageContainer::GetComponentServiceName()
{
    return OUString::createFromAscii( "TabPageContainer" );
}

//// uno::XInterface
uno::Any UnoControlTabPageContainer::queryAggregation( const uno::Type & rType ) throw(uno::RuntimeException)
{
    uno::Any aRet = ::cppu::queryInterface( rType,
        SAL_STATIC_CAST( awt::tab::XTabPageContainer*, this ) );
    return (aRet.hasValue() ? aRet : ControlContainerBase::queryAggregation( rType ));
}

void SAL_CALL UnoControlTabPageContainer::dispose(  ) throw(RuntimeException)
{
    lang::EventObject aEvt;
    aEvt.Source = (::cppu::OWeakObject*)this;
    m_aTabPageListeners.disposeAndClear( aEvt );
    UnoControl::dispose();
}

void UnoControlTabPageContainer::createPeer( const uno::Reference< awt::XToolkit > & rxToolkit, const uno::Reference< awt::XWindowPeer >  & rParentPeer ) throw(uno::RuntimeException)
{
    UnoControlBase::createPeer( rxToolkit, rParentPeer );

    Reference< XTabPageContainer >  xTPContainer( getPeer(), UNO_QUERY_THROW );
    xTPContainer->addTabPageListener(&m_aTabPageListeners);
}

// -------------------------------------------------------------------
// XTabPageContainer

::sal_Int16 SAL_CALL UnoControlTabPageContainer::getActiveTabPageID() throw (::com::sun::star::uno::RuntimeException)
{
    return m_nActiveTabPageId;
}
void SAL_CALL UnoControlTabPageContainer::setActiveTabPageID( ::sal_Int16 _activetabpageid ) throw (::com::sun::star::uno::RuntimeException)
{
    m_nActiveTabPageId = _activetabpageid;
}
::sal_Int32 SAL_CALL UnoControlTabPageContainer::getTabPageCount(  ) throw (::com::sun::star::uno::RuntimeException)
{
    return 0;
}
::sal_Bool SAL_CALL UnoControlTabPageContainer::isTabPageActive( ::sal_Int16 /*tabPageIndex*/ ) throw (::com::sun::star::uno::RuntimeException)
{
    return false;
}
::com::sun::star::uno::Reference< ::com::sun::star::awt::tab::XTabPage > SAL_CALL UnoControlTabPageContainer::getTabPage( ::sal_Int16 /*tabPageIndex*/ ) throw (::com::sun::star::uno::RuntimeException)
{
    return NULL;
}
::com::sun::star::uno::Reference< ::com::sun::star::awt::tab::XTabPage > SAL_CALL UnoControlTabPageContainer::getTabPageByID( ::sal_Int16 /*tabPageID */) throw (::com::sun::star::uno::RuntimeException)
{
    return NULL;
}
void SAL_CALL UnoControlTabPageContainer::addTabPageListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::tab::XTabPageContainerListener >& listener ) throw (::com::sun::star::uno::RuntimeException)
{
    m_aTabPageListeners.addInterface( listener );
}
void SAL_CALL UnoControlTabPageContainer::removeTabPageListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::tab::XTabPageContainerListener >& listener ) throw (::com::sun::star::uno::RuntimeException)
{
    m_aTabPageListeners.removeInterface( listener );
}

//// ::com::sun::star::awt::XControlContainer
void SAL_CALL UnoControlTabPageContainer::setStatusText( const ::rtl::OUString& StatusText ) throw (::com::sun::star::uno::RuntimeException)
{
    (void)StatusText;
}
::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl > > SAL_CALL UnoControlTabPageContainer::getControls(  ) throw (::com::sun::star::uno::RuntimeException)
{
    return 0;
}
uno::Reference< awt::XControl > UnoControlTabPageContainer::getControl( const ::rtl::OUString& rName ) throw(uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );
    (void)rName;
    //return mpControls->getControlForName( rName );
    return uno::Reference< awt::XControl >();
}
void SAL_CALL UnoControlTabPageContainer::addControl( const ::rtl::OUString& Name, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl >& Control ) throw (::com::sun::star::uno::RuntimeException)
{
    (void)Name;
    (void)Control;
}
void SAL_CALL UnoControlTabPageContainer::removeControl( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl >& Control ) throw (::com::sun::star::uno::RuntimeException)
{
    (void)Control;
}

Reference< XInterface > SAL_CALL UnoControlTabPageContainer_CreateInstance( const Reference< XMultiServiceFactory >& )
{
    return Reference < XInterface >( ( ::cppu::OWeakObject* ) new UnoControlTabPageContainer );
}

Reference< XInterface > SAL_CALL UnoControlTabPageContainerModel_CreateInstance( const Reference< XMultiServiceFactory >& )
{
    return Reference < XInterface >( ( ::cppu::OWeakObject* ) new UnoControlTabPageContainerModel );
}
