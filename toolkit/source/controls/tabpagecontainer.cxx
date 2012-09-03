/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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


#include <toolkit/controls/geometrycontrolmodel.hxx>
#include <toolkit/controls/tabpagecontainer.hxx>
#include <toolkit/controls/tabpagemodel.hxx>
#include <toolkit/helper/property.hxx>
#include <toolkit/helper/unopropertyarrayhelper.hxx>

#include <com/sun/star/awt/XControlModel.hpp>
#include <com/sun/star/awt/XVclWindowPeer.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <comphelper/processfactory.hxx>
#include <osl/diagnose.h>
#include <tools/diagnose_ex.h>
#include <vcl/svapp.hxx>

using ::rtl::OUString;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::view;
using ::com::sun::star::awt::tab::XTabPageModel;

#define WRONG_TYPE_EXCEPTION "Type must be ::com::sun::star::awt::tab::XTabPageModel!"
//  ----------------------------------------------------
//  class UnoControlTabPageContainerModel
//  ----------------------------------------------------
UnoControlTabPageContainerModel::UnoControlTabPageContainerModel( const Reference< XMultiServiceFactory >& i_factory )
    :UnoControlTabPageContainerModel_Base( i_factory )
    ,maContainerListeners( *this )
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

namespace
{
    Reference< XTabPageModel > lcl_createTabPageModel( ::comphelper::ComponentContext const & i_context,
        Sequence< Any > const & i_initArguments )
    {
        try
        {
            Reference< XTabPageModel > const xTabPageModel(
                *( new OGeometryControlModel< UnoControlTabPageModel >( i_context.getLegacyServiceFactory() ) ),
                UNO_QUERY_THROW
            );

            Reference< XInitialization > const xInit( xTabPageModel, UNO_QUERY_THROW );
            xInit->initialize( i_initArguments );

            return xTabPageModel;
        }
        catch( const RuntimeException& )
        {
            throw;
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
        return NULL;
    }
}

Reference< XTabPageModel > SAL_CALL UnoControlTabPageContainerModel::createTabPage( ::sal_Int16 i_tabPageID ) throw (RuntimeException)
{
    Sequence< Any > aInitArgs(1);
    aInitArgs[0] <<= i_tabPageID;
    return lcl_createTabPageModel( maContext, aInitArgs );
}

Reference< XTabPageModel > SAL_CALL UnoControlTabPageContainerModel::loadTabPage( ::sal_Int16 i_tabPageID, const ::rtl::OUString& i_resourceURL ) throw (RuntimeException)
{
    Sequence< Any > aInitArgs(2);
    aInitArgs[0] <<= i_tabPageID;
    aInitArgs[1] <<= i_resourceURL;
    return lcl_createTabPageModel( maContext, aInitArgs );
}

void SAL_CALL UnoControlTabPageContainerModel::insertByIndex( ::sal_Int32 nIndex, const com::sun::star::uno::Any& aElement) throw (IllegalArgumentException, IndexOutOfBoundsException, WrappedTargetException, uno::RuntimeException)
{
    SolarMutexGuard aSolarGuard;
    uno::Reference < XTabPageModel > xTabPageModel;
    if(aElement >>= xTabPageModel)
    {
        if ( sal_Int32( m_aTabPageVector.size()) ==nIndex )
            m_aTabPageVector.push_back( xTabPageModel );
        else if ( sal_Int32( m_aTabPageVector.size()) > nIndex )
        {
            std::vector< uno::Reference< XTabPageModel > >::iterator aIter = m_aTabPageVector.begin();
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
UnoControlTabPageContainer::UnoControlTabPageContainer( const Reference< XMultiServiceFactory >& i_factory )
    :UnoControlTabPageContainer_Base( i_factory )
    ,m_aTabPageListeners( *this )
{
}

OUString UnoControlTabPageContainer::GetComponentServiceName()
{
    return OUString(RTL_CONSTASCII_USTRINGPARAM("TabPageContainer"));
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
        xTPContainer->addTabPageContainerListener(&m_aTabPageListeners);
}

// -------------------------------------------------------------------
// XTabPageContainer

::sal_Int16 SAL_CALL UnoControlTabPageContainer::getActiveTabPageID() throw (RuntimeException)
{
    SolarMutexGuard aSolarGuard;
    Reference< XTabPageContainer >  xTPContainer( getPeer(), UNO_QUERY_THROW );
    return xTPContainer->getActiveTabPageID();
}
void SAL_CALL UnoControlTabPageContainer::setActiveTabPageID( ::sal_Int16 _activetabpageid ) throw (RuntimeException)
{
    SolarMutexGuard aSolarGuard;
    Reference< XTabPageContainer >  xTPContainer( getPeer(), UNO_QUERY_THROW );
    xTPContainer->setActiveTabPageID(_activetabpageid);
}
::sal_Int16 SAL_CALL UnoControlTabPageContainer::getTabPageCount(  ) throw (RuntimeException)
{
    SolarMutexGuard aSolarGuard;
    Reference< XTabPageContainer >  xTPContainer( getPeer(), UNO_QUERY_THROW );
    return xTPContainer->getTabPageCount();
}
::sal_Bool SAL_CALL UnoControlTabPageContainer::isTabPageActive( ::sal_Int16 tabPageIndex ) throw (RuntimeException)
{
    SolarMutexGuard aSolarGuard;
    Reference< XTabPageContainer >  xTPContainer( getPeer(), UNO_QUERY_THROW );
    return xTPContainer->isTabPageActive(tabPageIndex);
}
Reference< ::com::sun::star::awt::tab::XTabPage > SAL_CALL UnoControlTabPageContainer::getTabPage( ::sal_Int16 tabPageIndex ) throw (RuntimeException)
{
    SolarMutexGuard aSolarGuard;
    Reference< XTabPageContainer >  xTPContainer( getPeer(), UNO_QUERY_THROW );
    return xTPContainer->getTabPage(tabPageIndex);
}
Reference< ::com::sun::star::awt::tab::XTabPage > SAL_CALL UnoControlTabPageContainer::getTabPageByID( ::sal_Int16 tabPageID ) throw (RuntimeException)
{
    SolarMutexGuard aSolarGuard;
    Reference< XTabPageContainer >  xTPContainer( getPeer(), UNO_QUERY_THROW );
    return xTPContainer->getTabPageByID(tabPageID);
}
void SAL_CALL UnoControlTabPageContainer::addTabPageContainerListener( const Reference< ::com::sun::star::awt::tab::XTabPageContainerListener >& listener ) throw (RuntimeException)
{
    m_aTabPageListeners.addInterface( listener );
    if( getPeer().is() && m_aTabPageListeners.getLength() == 1 )
    {
        uno::Reference < awt::tab::XTabPageContainer >  xTabPageContainer( getPeer(), uno::UNO_QUERY );
        xTabPageContainer->addTabPageContainerListener( &m_aTabPageListeners );
    }
}
void SAL_CALL UnoControlTabPageContainer::removeTabPageContainerListener( const Reference< ::com::sun::star::awt::tab::XTabPageContainerListener >& listener ) throw (RuntimeException)
{
    if( getPeer().is() && m_aTabPageListeners.getLength() == 1 )
    {
        uno::Reference < awt::tab::XTabPageContainer >  xTabPageContainer( getPeer(), uno::UNO_QUERY );
        xTabPageContainer->addTabPageContainerListener( &m_aTabPageListeners );
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
    SolarMutexGuard aSolarGuard;
    ControlContainerBase::addControl(Name,Control);
    Reference< XContainerListener > xContainerListener( getPeer(), UNO_QUERY );
    ContainerEvent aEvent;
    aEvent.Source = getModel();
    aEvent.Element <<= Control;
    xContainerListener->elementInserted( aEvent );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
