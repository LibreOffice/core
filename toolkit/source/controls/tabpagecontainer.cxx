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
#include <toolkit/controls/geometrycontrolmodel.hxx>
#include <com/sun/star/awt/XVclWindowPeer.hpp>
#include <comphelper/processfactory.hxx>
#include <osl/diagnose.h>
#include <vcl/svapp.hxx>
#include <vos/mutex.hxx>
#include <com/sun/star/awt/XControlModel.hpp>
#include <tools/diagnose_ex.h>

using ::rtl::OUString;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::view;

#define WRONG_TYPE_EXCEPTION "Type must be ::com::sun::star::awt::tab::XTabPageModel!"
//  ----------------------------------------------------
//  class UnoControlTabPageContainerModel
//  ----------------------------------------------------
UnoControlTabPageContainerModel::UnoControlTabPageContainerModel() : maContainerListeners( *this )
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

::rtl::OUString UnoControlTabPageContainerModel::getServiceName() throw(RuntimeException)
{
    return ::rtl::OUString::createFromAscii( szServiceName_UnoControlTabPageContainerModel );
}

uno::Any UnoControlTabPageContainerModel::ImplGetDefaultValue( sal_uInt16 nPropId ) const
{
    switch(nPropId)
    {
        case BASEPROPERTY_DEFAULTCONTROL:
            return uno::makeAny( ::rtl::OUString::createFromAscii( szServiceName_UnoControlTabPageContainer ) );
        case BASEPROPERTY_BORDER:
            return uno::makeAny((sal_Int16) 0);              // No Border
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
Reference< ::com::sun::star::beans::XPropertySetInfo > UnoControlTabPageContainerModel::getPropertySetInfo(  ) throw(RuntimeException)
{
    static Reference< ::com::sun::star::beans::XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}

void SAL_CALL UnoControlTabPageContainerModel::insertByIndex( ::sal_Int32 nIndex, const com::sun::star::uno::Any& aElement) throw (IllegalArgumentException, IndexOutOfBoundsException, WrappedTargetException, uno::RuntimeException)
{
    vos::OGuard aSolarGuard( Application::GetSolarMutex() );
    uno::Reference < ::awt::tab::XTabPageModel > xTabPageModel;
    if(aElement >>= xTabPageModel)
    {
        if ( sal_Int32( m_aTabPageVector.size()) ==nIndex )
            m_aTabPageVector.push_back( xTabPageModel );
        else if ( sal_Int32( m_aTabPageVector.size()) > nIndex )
        {
            std::vector< uno::Reference< ::awt::tab::XTabPageModel > >::iterator aIter = m_aTabPageVector.begin();
            aIter += nIndex;
            m_aTabPageVector.insert( aIter, xTabPageModel );
        }
        else
            throw IndexOutOfBoundsException( ::rtl::OUString(), (OWeakObject *)this );
        ContainerEvent aEvent;
        aEvent.Source = *this;
        aEvent.Element <<= aElement;
        aEvent.Accessor <<= ::rtl::OUString::valueOf(nIndex);
        maContainerListeners.elementInserted( aEvent );
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
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );
    return sal_Int32( m_aTabPageVector.size());
}
// -----------------------------------------------------------------------------
uno::Any SAL_CALL UnoControlTabPageContainerModel::getByIndex( ::sal_Int32 nIndex ) throw (lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );
    if ( nIndex < 0 || nIndex > sal_Int32(m_aTabPageVector.size()) )
        throw lang::IndexOutOfBoundsException();
    return uno::makeAny(m_aTabPageVector[nIndex]);
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
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );
    return !m_aTabPageVector.empty();
}
// XContainer
void UnoControlTabPageContainerModel::addContainerListener( const Reference< XContainerListener >& l ) throw(RuntimeException)
{
     maContainerListeners.addInterface( l );
}

void UnoControlTabPageContainerModel::removeContainerListener( const Reference< XContainerListener >& l ) throw(RuntimeException)
{
     maContainerListeners.removeInterface( l );
}

//  ----------------------------------------------------
//  class UnoControlTabPageContainer
//  ----------------------------------------------------
UnoControlTabPageContainer::UnoControlTabPageContainer():
    m_aTabPageListeners( *this )
{
}

OUString UnoControlTabPageContainer::GetComponentServiceName()
{
    return OUString::createFromAscii( "TabPageContainer" );
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
    if ( m_aTabPageListeners.getLength() )
        xTPContainer->addTabPageListener(&m_aTabPageListeners);
}

// -------------------------------------------------------------------
// XTabPageContainer

::sal_Int16 SAL_CALL UnoControlTabPageContainer::getActiveTabPageID() throw (RuntimeException)
{
    vos::OGuard aSolarGuard( Application::GetSolarMutex() );
    Reference< XTabPageContainer >  xTPContainer( getPeer(), UNO_QUERY_THROW );
    return xTPContainer->getActiveTabPageID();
}
void SAL_CALL UnoControlTabPageContainer::setActiveTabPageID( ::sal_Int16 _activetabpageid ) throw (RuntimeException)
{
    vos::OGuard aSolarGuard( Application::GetSolarMutex() );
    Reference< XTabPageContainer >  xTPContainer( getPeer(), UNO_QUERY_THROW );
    xTPContainer->setActiveTabPageID(_activetabpageid);
}
::sal_Int32 SAL_CALL UnoControlTabPageContainer::getTabPageCount(  ) throw (RuntimeException)
{
    vos::OGuard aSolarGuard( Application::GetSolarMutex() );
    Reference< XTabPageContainer >  xTPContainer( getPeer(), UNO_QUERY_THROW );
    return xTPContainer->getTabPageCount();
}
::sal_Bool SAL_CALL UnoControlTabPageContainer::isTabPageActive( ::sal_Int16 tabPageIndex ) throw (RuntimeException)
{
    vos::OGuard aSolarGuard( Application::GetSolarMutex() );
    Reference< XTabPageContainer >  xTPContainer( getPeer(), UNO_QUERY_THROW );
    return xTPContainer->isTabPageActive(tabPageIndex);
}
Reference< ::com::sun::star::awt::tab::XTabPage > SAL_CALL UnoControlTabPageContainer::getTabPage( ::sal_Int16 tabPageIndex ) throw (RuntimeException)
{
    vos::OGuard aSolarGuard( Application::GetSolarMutex() );
    Reference< XTabPageContainer >  xTPContainer( getPeer(), UNO_QUERY_THROW );
    return xTPContainer->getTabPage(tabPageIndex);
}
Reference< ::com::sun::star::awt::tab::XTabPage > SAL_CALL UnoControlTabPageContainer::getTabPageByID( ::sal_Int16 tabPageID ) throw (RuntimeException)
{
    vos::OGuard aSolarGuard( Application::GetSolarMutex() );
    Reference< XTabPageContainer >  xTPContainer( getPeer(), UNO_QUERY_THROW );
    return xTPContainer->getTabPageByID(tabPageID);
}
void SAL_CALL UnoControlTabPageContainer::addTabPageListener( const Reference< ::com::sun::star::awt::tab::XTabPageContainerListener >& listener ) throw (RuntimeException)
{
    m_aTabPageListeners.addInterface( listener );
    if( getPeer().is() && m_aTabPageListeners.getLength() == 1 )
    {
        uno::Reference < awt::tab::XTabPageContainer >  xTabPageContainer( getPeer(), uno::UNO_QUERY );
        xTabPageContainer->addTabPageListener( &m_aTabPageListeners );
    }
}
void SAL_CALL UnoControlTabPageContainer::removeTabPageListener( const Reference< ::com::sun::star::awt::tab::XTabPageContainerListener >& listener ) throw (RuntimeException)
{
    if( getPeer().is() && m_aTabPageListeners.getLength() == 1 )
    {
        uno::Reference < awt::tab::XTabPageContainer >  xTabPageContainer( getPeer(), uno::UNO_QUERY );
        xTabPageContainer->addTabPageListener( &m_aTabPageListeners );
    }
    m_aTabPageListeners.removeInterface( listener );
}

void UnoControlTabPageContainer::updateFromModel()
{
    UnoControlTabPageContainer_Base::updateFromModel();
    Reference< XContainerListener > xContainerListener( getPeer(), UNO_QUERY );
    ENSURE_OR_RETURN_VOID( xContainerListener.is(), "UnoListBoxControl::updateFromModel: a peer which is no ItemListListener?!" );

    ContainerEvent aEvent;
    aEvent.Source = getModel();
    Sequence< Reference< XControl > > aControls = getControls();
    const Reference< XControl >* pCtrls = aControls.getConstArray();
    const Reference< XControl >* pCtrlsEnd = pCtrls + aControls.getLength();

    for ( ; pCtrls < pCtrlsEnd; ++pCtrls )
    {
        aEvent.Element <<= *pCtrls;
        xContainerListener->elementInserted( aEvent );
    }
}
void SAL_CALL UnoControlTabPageContainer::addControl( const ::rtl::OUString& Name, const Reference< ::com::sun::star::awt::XControl >& Control ) throw (RuntimeException)
{
    vos::OGuard aSolarGuard( Application::GetSolarMutex() );
    ControlContainerBase::addControl(Name,Control);
    Reference< XContainerListener > xContainerListener( getPeer(), UNO_QUERY );
    ContainerEvent aEvent;
    aEvent.Source = getModel();
    aEvent.Element <<= Control;
    xContainerListener->elementInserted( aEvent );
}

Reference< XInterface > SAL_CALL UnoControlTabPageContainer_CreateInstance( const Reference< XMultiServiceFactory >& )
{
    return Reference < XInterface >( ( ::cppu::OWeakObject* ) new UnoControlTabPageContainer );
}

Reference< XInterface > SAL_CALL UnoControlTabPageContainerModel_CreateInstance( const Reference< XMultiServiceFactory >& )
{
    return Reference < XInterface >( ( ::cppu::OWeakObject* ) new OGeometryControlModel<UnoControlTabPageContainerModel>() );
}
