/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <com/sun/star/awt/XVclContainerPeer.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#include <com/sun/star/container/NoSuchElementException.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <cppuhelper/implbase.hxx>

#include <controls/unocontrolcontainer.hxx>
#include <comphelper/sequence.hxx>

#include <tools/debug.hxx>

#include <limits>
#include <map>
#include <memory>
#include <com/sun/star/awt/VclWindowPeerAttribute.hpp>
#include <utility>

using namespace ::com::sun::star;



namespace {

struct UnoControlHolder
{
    uno::Reference< awt::XControl > mxControl;
    OUString                        msName;

public:
    UnoControlHolder( OUString aName, uno::Reference< awt::XControl > xControl )
    :   mxControl(std::move( xControl )),
        msName(std::move( aName ))
    {
    }

    const OUString&                   getName() const { return msName; }
    const uno::Reference< awt::XControl >&   getControl() const { return mxControl; }
};

}

class UnoControlHolderList
{
public:
    typedef sal_Int32                                       ControlIdentifier;
private:
    typedef std::shared_ptr< UnoControlHolder >         ControlInfo;
    typedef ::std::map< ControlIdentifier, ControlInfo >    ControlMap;

private:
    ControlMap  maControls;

public:
    UnoControlHolderList();

    /** adds a control with the given name to the list
        @param _rxControl
            the control to add. Must not be <NULL/>
        @param _pBName
            the name of the control, or <NULL/> if an automatic name should be generated
        @return
            the identifier of the newly added control
    */
    ControlIdentifier   addControl( const uno::Reference< awt::XControl >& _rxControl, const OUString* _pName );

    /** determines whether or not the list is empty
    */
    bool         empty() const { return maControls.empty(); }

    /** retrieves all controls currently in the list
    */
    void  getControls( uno::Sequence< uno::Reference< awt::XControl > >& _out_rControls ) const;

    /** retrieves all identifiers of all controls currently in the list
    */
    void  getIdentifiers( uno::Sequence< sal_Int32 >& _out_rIdentifiers ) const;

    /** returns the first control which is registered under the given name
    */
    uno::Reference< awt::XControl >
            getControlForName( const OUString& _rName ) const;

    /** returns the identifier which a control is registered for, or -1 if the control
            isn't registered
    */
    ControlIdentifier
            getControlIdentifier( const uno::Reference< awt::XControl >& _rxControl );

    /** retrieves the control for a given id
        @param _nIdentifier
            the identifier for the control
        @param _out_rxControl
            takes the XControl upon successful return
        @return
            <TRUE/> if and only if a control with the given id is part of the list
    */
    bool    getControlForIdentifier( ControlIdentifier _nIdentifier, uno::Reference< awt::XControl >& _out_rxControl ) const;

    /** removes a control from the list, given by id
        @param _nId
            The identifier of the control to remove.
    */
    void    removeControlById( ControlIdentifier _nId );

    /** replaces a control from the list with another one
        @param _nId
            The identifier of the control to replace
        @param _rxNewControl
            the new control to put into the list
    */
    void    replaceControlById( ControlIdentifier _nId, const uno::Reference< awt::XControl >& _rxNewControl );

private:
    /** adds a control
    @param _rxControl
        the control to add to the container
    @param _pName
        pointer to the name of the control. Might be <NULL/>, in this case, a name is generated.
    @return
        the identifier of the newly inserted control
    */
    ControlIdentifier impl_addControl(
        const uno::Reference< awt::XControl >& _rxControl,
        const OUString*  _pName
    );

    /** finds a free identifier
        @throw uno::RuntimeException
            if no free identifier can be found
    */
    ControlIdentifier impl_getFreeIdentifier_throw();

    /** finds a free name
        @throw uno::RuntimeException
            if no free name can be found
    */
    OUString impl_getFreeName_throw();
};


UnoControlHolderList::UnoControlHolderList()
{
}


UnoControlHolderList::ControlIdentifier UnoControlHolderList::addControl( const uno::Reference< awt::XControl >& _rxControl, const OUString* _pName )
{
    return impl_addControl( _rxControl, _pName );
}


void UnoControlHolderList::getControls( uno::Sequence< uno::Reference< awt::XControl > >& _out_rControls ) const
{
    _out_rControls.realloc( maControls.size() );
    uno::Reference< awt::XControl >* pControls = _out_rControls.getArray();
    for (const auto& rEntry : maControls)
    {
        *pControls = rEntry.second->getControl();
        ++pControls;
    }
}


void UnoControlHolderList::getIdentifiers( uno::Sequence< sal_Int32 >& _out_rIdentifiers ) const
{
    _out_rIdentifiers.realloc( maControls.size() );
    sal_Int32* pIdentifiers = _out_rIdentifiers.getArray();
    for (const auto& rEntry : maControls)
    {
        *pIdentifiers = rEntry.first;
        ++pIdentifiers;
    }
}


uno::Reference< awt::XControl > UnoControlHolderList::getControlForName( const OUString& _rName ) const
{
    auto loop = std::find_if(maControls.begin(), maControls.end(),
        [&_rName](const ControlMap::value_type& rEntry) { return rEntry.second->getName() == _rName; });
    if (loop != maControls.end())
        return loop->second->getControl();
    return uno::Reference< awt::XControl >();
}


UnoControlHolderList::ControlIdentifier UnoControlHolderList::getControlIdentifier( const uno::Reference< awt::XControl >& _rxControl )
{
    auto loop = std::find_if(maControls.begin(), maControls.end(),
        [&_rxControl](const ControlMap::value_type& rEntry) { return rEntry.second->getControl().get() == _rxControl.get(); });
    if (loop != maControls.end())
        return loop->first;
    return -1;
}


bool UnoControlHolderList::getControlForIdentifier( UnoControlHolderList::ControlIdentifier _nIdentifier, uno::Reference< awt::XControl >& _out_rxControl ) const
{
    ControlMap::const_iterator pos = maControls.find( _nIdentifier );
    if ( pos == maControls.end() )
        return false;
    _out_rxControl = pos->second->getControl();
    return true;
}


void UnoControlHolderList::removeControlById( UnoControlHolderList::ControlIdentifier _nId )
{
    ControlMap::iterator pos = maControls.find( _nId );
    DBG_ASSERT( pos != maControls.end(), "UnoControlHolderList::removeControlById: invalid id!" );
    if ( pos == maControls.end() )
        return;

    maControls.erase( pos );
}


void UnoControlHolderList::replaceControlById( ControlIdentifier _nId, const uno::Reference< awt::XControl >& _rxNewControl )
{
    DBG_ASSERT( _rxNewControl.is(), "UnoControlHolderList::replaceControlById: invalid new control!" );

    ControlMap::iterator pos = maControls.find( _nId );
    DBG_ASSERT( pos != maControls.end(), "UnoControlHolderList::replaceControlById: invalid id!" );
    if ( pos == maControls.end() )
        return;

    pos->second = std::make_shared<UnoControlHolder>( pos->second->getName(), _rxNewControl );
}


UnoControlHolderList::ControlIdentifier UnoControlHolderList::impl_addControl( const uno::Reference< awt::XControl >& _rxControl, const OUString* _pName )
{
    DBG_ASSERT( _rxControl.is(), "UnoControlHolderList::impl_addControl: invalid control!" );

    OUString sName = _pName ? *_pName : impl_getFreeName_throw();
    sal_Int32 nId = impl_getFreeIdentifier_throw();

    maControls[ nId ] = std::make_shared<UnoControlHolder>( sName, _rxControl );
    return nId;
}


UnoControlHolderList::ControlIdentifier UnoControlHolderList::impl_getFreeIdentifier_throw()
{
    for ( ControlIdentifier candidateId = 0; candidateId < ::std::numeric_limits< ControlIdentifier >::max(); ++candidateId )
    {
        ControlMap::const_iterator existent = maControls.find( candidateId );
        if ( existent == maControls.end() )
            return candidateId;
    }
    throw uno::RuntimeException(u"out of identifiers"_ustr );
}


OUString UnoControlHolderList::impl_getFreeName_throw()
{
    for ( ControlIdentifier candidateId = 0; candidateId < ::std::numeric_limits< ControlIdentifier >::max(); ++candidateId )
    {
        OUString candidateName( "control_" + OUString::number( candidateId ) );
        if ( std::none_of(maControls.begin(), maControls.end(),
                [&candidateName](const ControlMap::value_type& rEntry) { return rEntry.second->getName() == candidateName; }) )
            return candidateName;
    }
    throw uno::RuntimeException(u"out of identifiers"_ustr );
}

//  Function to set the controls' visibility according
//  to the dialog's "Step" property

static void implUpdateVisibility
(
    sal_Int32 nDialogStep,
    const uno::Reference< awt::XControlContainer >& xControlContainer
)
{
    const uno::Sequence< uno::Reference< awt::XControl > >
        aCtrls = xControlContainer->getControls();
    bool bCompleteVisible = (nDialogStep == 0);
    for( const uno::Reference< awt::XControl >& xControl : aCtrls )
    {
        bool bVisible = bCompleteVisible;
        if( !bVisible )
        {
            uno::Reference< awt::XControlModel > xModel( xControl->getModel() );
            uno::Reference< beans::XPropertySet > xPSet
                ( xModel, uno::UNO_QUERY );
            uno::Reference< beans::XPropertySetInfo >
                xInfo = xPSet->getPropertySetInfo();
            OUString aPropName( u"Step"_ustr );
            sal_Int32 nControlStep = 0;
            if ( xInfo->hasPropertyByName( aPropName ) )
            {
                uno::Any aVal = xPSet->getPropertyValue( aPropName );
                aVal >>= nControlStep;
            }
            bVisible = (nControlStep == 0) || (nControlStep == nDialogStep);
        }

        uno::Reference< awt::XWindow> xWindow
            ( xControl, uno::UNO_QUERY );
        if( xWindow.is() )
            xWindow->setVisible( bVisible );
    }
}



typedef ::cppu::WeakImplHelper< beans::XPropertyChangeListener > PropertyChangeListenerHelper;

namespace {

class DialogStepChangedListener: public PropertyChangeListenerHelper
{
private:
    uno::Reference< awt::XControlContainer > mxControlContainer;

public:
    explicit DialogStepChangedListener( uno::Reference< awt::XControlContainer > xControlContainer )
        : mxControlContainer(std::move( xControlContainer )) {}

    // XEventListener
    virtual void SAL_CALL disposing( const  lang::EventObject& Source ) override;

    // XPropertyChangeListener
    virtual void SAL_CALL propertyChange( const  beans::PropertyChangeEvent& evt ) override;

};

}

void SAL_CALL DialogStepChangedListener::disposing( const  lang::EventObject& /*_rSource*/)
{
    mxControlContainer.clear();
}

void SAL_CALL DialogStepChangedListener::propertyChange( const  beans::PropertyChangeEvent& evt )
{
    // evt.PropertyName HAS to be "Step" because we only use the listener for that
    sal_Int32 nDialogStep = 0;
    evt.NewValue >>= nDialogStep;
    implUpdateVisibility( nDialogStep, mxControlContainer );
}



UnoControlContainer::UnoControlContainer()
    :maCListeners( *this )
{
    mpControls.reset(new UnoControlHolderList);
}

UnoControlContainer::UnoControlContainer(const uno::Reference< awt::XVclWindowPeer >& xP )
    :maCListeners( *this )
{
    setPeer( xP );
    mbDisposePeer = false;
    mpControls.reset(new UnoControlHolderList);
}

UnoControlContainer::~UnoControlContainer()
{
}

void UnoControlContainer::ImplActivateTabControllers()
{
    for ( auto& rTabController : asNonConstRange(maTabControllers) )
    {
        rTabController->setContainer( this );
        rTabController->activateTabOrder();
    }
}

// lang::XComponent
void UnoControlContainer::dispose(  )
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    lang::EventObject aDisposeEvent;
    aDisposeEvent.Source = static_cast< uno::XAggregation* >( this );

    // Notify listeners about disposal of this Container (This is much faster if they
    // listen on the controls and the container).
    maDisposeListeners.disposeAndClear( aDisposeEvent );
    maCListeners.disposeAndClear( aDisposeEvent );


    const uno::Sequence< uno::Reference< awt::XControl > > aCtrls = getControls();

    for( uno::Reference< awt::XControl > const & control : aCtrls )
    {
        removingControl( control );
        // Delete control
        control->dispose();
    }


    // Delete all structures
    mpControls.reset(new UnoControlHolderList);

    UnoControlBase::dispose();
}

// lang::XEventListener
void UnoControlContainer::disposing( const lang::EventObject& _rEvt )
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    uno::Reference< awt::XControl >  xControl( _rEvt.Source, uno::UNO_QUERY );
    if ( xControl.is() )
        removeControl( xControl );

    UnoControlBase::disposing( _rEvt );
}

// container::XContainer
void UnoControlContainer::addContainerListener( const uno::Reference< container::XContainerListener >& rxListener )
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    maCListeners.addInterface( rxListener );
}

void UnoControlContainer::removeContainerListener( const uno::Reference< container::XContainerListener >& rxListener )
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    maCListeners.removeInterface( rxListener );
}


::sal_Int32 SAL_CALL UnoControlContainer::insert( const uno::Any& _rElement )
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    uno::Reference< awt::XControl > xControl;
    if ( !( _rElement >>= xControl ) || !xControl.is() )
        throw lang::IllegalArgumentException(
            u"Elements must support the XControl interface."_ustr,
            *this,
            1
        );

    return impl_addControl( xControl );
}

void SAL_CALL UnoControlContainer::removeByIdentifier( ::sal_Int32 _nIdentifier )
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    uno::Reference< awt::XControl > xControl;
    if ( !mpControls->getControlForIdentifier( _nIdentifier, xControl ) )
        throw container::NoSuchElementException(
            u"There is no element with the given identifier."_ustr,
            *this
        );

    impl_removeControl( _nIdentifier, xControl );
}

void SAL_CALL UnoControlContainer::replaceByIdentifer( ::sal_Int32 _nIdentifier, const uno::Any& _rElement )
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    uno::Reference< awt::XControl > xExistentControl;
    if ( !mpControls->getControlForIdentifier( _nIdentifier, xExistentControl ) )
        throw container::NoSuchElementException(
            u"There is no element with the given identifier."_ustr,
            *this
        );

    uno::Reference< awt::XControl > xNewControl;
    if ( !( _rElement >>= xNewControl ) )
        throw lang::IllegalArgumentException(
            u"Elements must support the XControl interface."_ustr,
            *this,
            1
        );

    removingControl( xExistentControl );

    mpControls->replaceControlById( _nIdentifier, xNewControl );

    addingControl( xNewControl );

    impl_createControlPeerIfNecessary( xNewControl );

    if ( maCListeners.getLength() )
    {
        container::ContainerEvent aEvent;
        aEvent.Source = *this;
        aEvent.Accessor <<= _nIdentifier;
        aEvent.Element <<= xNewControl;
        aEvent.ReplacedElement <<= xExistentControl;
        maCListeners.elementReplaced( aEvent );
    }
}

uno::Any SAL_CALL UnoControlContainer::getByIdentifier( ::sal_Int32 _nIdentifier )
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    uno::Reference< awt::XControl > xControl;
    if ( !mpControls->getControlForIdentifier( _nIdentifier, xControl ) )
        throw container::NoSuchElementException();
    return uno::Any( xControl );
}

uno::Sequence< ::sal_Int32 > SAL_CALL UnoControlContainer::getIdentifiers(  )
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    uno::Sequence< ::sal_Int32 > aIdentifiers;
    mpControls->getIdentifiers( aIdentifiers );
    return aIdentifiers;
}

// container::XElementAccess
uno::Type SAL_CALL UnoControlContainer::getElementType(  )
{
    return cppu::UnoType<awt::XControlModel>::get();
}

sal_Bool SAL_CALL UnoControlContainer::hasElements(  )
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );
    return !mpControls->empty();
}

// awt::XControlContainer
void UnoControlContainer::setStatusText( const OUString& rStatusText )
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    // Descend the parent hierarchy
    uno::Reference< awt::XControlContainer >  xContainer( mxContext, uno::UNO_QUERY );
    if( xContainer.is() )
        xContainer->setStatusText( rStatusText );
}

uno::Sequence< uno::Reference< awt::XControl > > UnoControlContainer::getControls(  )
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );
    uno::Sequence< uno::Reference< awt::XControl > > aControls;
    mpControls->getControls( aControls );
    return aControls;
}

uno::Reference< awt::XControl > UnoControlContainer::getControl( const OUString& rName )
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );
    return mpControls->getControlForName( rName );
}

void UnoControlContainer::addingControl( const uno::Reference< awt::XControl >& _rxControl )
{
    if ( _rxControl.is() )
    {
        uno::Reference< uno::XInterface > xThis;
        OWeakAggObject::queryInterface( cppu::UnoType<uno::XInterface>::get() ) >>= xThis;

        _rxControl->setContext( xThis );
        _rxControl->addEventListener( this );
    }
}

void UnoControlContainer::impl_createControlPeerIfNecessary( const uno::Reference< awt::XControl >& _rxControl )
{
    OSL_PRECOND( _rxControl.is(), "UnoControlContainer::impl_createControlPeerIfNecessary: invalid control, this will crash!" );

    // if the container already has a peer, then also create a peer for the control
    uno::Reference< awt::XWindowPeer > xMyPeer( getPeer() );

    if( xMyPeer.is() )
    {
        _rxControl->createPeer( nullptr, xMyPeer );
        ImplActivateTabControllers();
    }

}

sal_Int32 UnoControlContainer::impl_addControl( const uno::Reference< awt::XControl >& _rxControl, const OUString* _pName )
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );
    UnoControlHolderList::ControlIdentifier id = mpControls->addControl( _rxControl, _pName );

    addingControl( _rxControl );

    impl_createControlPeerIfNecessary( _rxControl );

    if ( maCListeners.getLength() )
    {
        container::ContainerEvent aEvent;
        aEvent.Source = *this;
        if (_pName)
            aEvent.Accessor <<= *_pName;
        else
            aEvent.Accessor <<= static_cast<sal_Int32>(id);
        aEvent.Element <<= _rxControl;
        maCListeners.elementInserted( aEvent );
    }

    return id;
}

void UnoControlContainer::addControl( const OUString& rName, const uno::Reference< awt::XControl >& rControl )
{
    if ( rControl.is() )
        impl_addControl( rControl, &rName );
}

void UnoControlContainer::removingControl( const uno::Reference< awt::XControl >& _rxControl )
{
    if ( _rxControl.is() )
    {
        _rxControl->removeEventListener( this );
        _rxControl->setContext( nullptr );
    }
}

void UnoControlContainer::impl_removeControl( sal_Int32 _nId, const uno::Reference< awt::XControl >& _rxControl )
{
#ifdef DBG_UTIL
    {
        uno::Reference< awt::XControl > xControl;
        bool bHas = mpControls->getControlForIdentifier( _nId, xControl );
        DBG_ASSERT( bHas && xControl == _rxControl, "UnoControlContainer::impl_removeControl: inconsistency in the parameters!" );
    }
#endif
    removingControl( _rxControl );

    mpControls->removeControlById( _nId );

    if ( maCListeners.getLength() )
    {
        container::ContainerEvent aEvent;
        aEvent.Source = *this;
        aEvent.Accessor <<= _nId;
        aEvent.Element <<= _rxControl;
        maCListeners.elementRemoved( aEvent );
    }
}

void UnoControlContainer::removeControl( const uno::Reference< awt::XControl >& _rxControl )
{
    if ( _rxControl.is() )
    {
        ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

        UnoControlHolderList::ControlIdentifier id = mpControls->getControlIdentifier( _rxControl );
        if ( id != -1 )
            impl_removeControl( id, _rxControl );
    }
}


// awt::XUnoControlContainer
void UnoControlContainer::setTabControllers( const uno::Sequence< uno::Reference< awt::XTabController > >& TabControllers )
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    maTabControllers = TabControllers;
}

uno::Sequence< uno::Reference< awt::XTabController > > UnoControlContainer::getTabControllers(  )
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    return maTabControllers;
}

void UnoControlContainer::addTabController( const uno::Reference< awt::XTabController >& TabController )
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    sal_uInt32 nCount = maTabControllers.getLength();
    maTabControllers.realloc( nCount + 1 );
    maTabControllers.getArray()[ nCount ] = TabController;
}

void UnoControlContainer::removeTabController( const uno::Reference< awt::XTabController >& TabController )
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    auto pTabController = std::find_if(std::cbegin(maTabControllers), std::cend(maTabControllers),
        [&TabController](const uno::Reference< awt::XTabController >& rTabController) {
            return rTabController.get() == TabController.get(); });
    if (pTabController != std::cend(maTabControllers))
    {
        auto n = static_cast<sal_Int32>(std::distance(std::cbegin(maTabControllers), pTabController));
        ::comphelper::removeElementAt( maTabControllers, n );
    }
}

// awt::XControl
void UnoControlContainer::createPeer( const uno::Reference< awt::XToolkit >& rxToolkit, const uno::Reference< awt::XWindowPeer >& rParent )
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    if( getPeer().is() )
        return;

    bool bVis = maComponentInfos.bVisible;
    if( bVis )
        UnoControl::setVisible( false );

    // Create a new peer
    UnoControl::createPeer( rxToolkit, rParent );

    // Create all children's peers
    if ( !mbCreatingCompatiblePeer )
    {
        // Evaluate "Step" property
        uno::Reference< awt::XControlModel > xModel( getModel() );
        uno::Reference< beans::XPropertySet > xPSet
            ( xModel, uno::UNO_QUERY );
        uno::Reference< beans::XPropertySetInfo >
            xInfo = xPSet->getPropertySetInfo();
        OUString aPropName( u"Step"_ustr );
        if ( xInfo->hasPropertyByName( aPropName ) )
        {
            css::uno::Any aVal = xPSet->getPropertyValue( aPropName );
            sal_Int32 nDialogStep = 0;
            aVal >>= nDialogStep;
            uno::Reference< awt::XControlContainer > xContainer =
                static_cast< awt::XControlContainer* >(this);
            implUpdateVisibility( nDialogStep, xContainer );

            uno::Reference< beans::XPropertyChangeListener > xListener =
                new DialogStepChangedListener(xContainer);
            xPSet->addPropertyChangeListener( aPropName, xListener );
        }

        uno::Sequence< uno::Reference< awt::XControl > > aCtrls = getControls();
        for( auto& rCtrl : asNonConstRange(aCtrls) )
            rCtrl->createPeer( rxToolkit, getPeer() );

        uno::Reference< awt::XVclContainerPeer >  xC( getPeer(), uno::UNO_QUERY );
        if ( xC.is() )
            xC->enableDialogControl( true );
        ImplActivateTabControllers();
    }

    if( bVis && !isDesignMode() )
        UnoControl::setVisible( true );
}


// awt::XWindow
void UnoControlContainer::setVisible( sal_Bool bVisible )
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    UnoControl::setVisible( bVisible );
    if( !mxContext.is() && bVisible )
        // This is a Topwindow, thus show it automatically
        createPeer( uno::Reference< awt::XToolkit > (), uno::Reference< awt::XWindowPeer > () );
}

OUString UnoControlContainer::getImplementationName()
{
    return u"stardiv.Toolkit.UnoControlContainer"_ustr;
}

css::uno::Sequence<OUString> UnoControlContainer::getSupportedServiceNames()
{
    auto s(UnoControlBase::getSupportedServiceNames());
    s.realloc(s.getLength() + 2);
    auto ps = s.getArray();
    ps[s.getLength() - 2] = "com.sun.star.awt.UnoControlContainer";
    ps[s.getLength() - 1] = "stardiv.vcl.control.ControlContainer";
    return s;
}

void UnoControlContainer::PrepareWindowDescriptor( css::awt::WindowDescriptor& rDesc )
{
    // HACK due to the fact that we can't really use VSCROLL & HSCROLL
    // for Dialog  ( css::awt::VclWindowPeerAttribute::VSCROLL
    // has the same value as
    // css::awt::WindowAttribute::NODECORATION )
    // For convenience in the PropBrowse using HSCROLL and VSCROLL ensures
    // the Correct text. We exchange them here and the control knows
    // about this hack ( it sucks badly I know )
    if ( rDesc.WindowAttributes & css::awt::VclWindowPeerAttribute::VSCROLL )
    {
       rDesc.WindowAttributes &= ~css::awt::VclWindowPeerAttribute::VSCROLL;
       rDesc.WindowAttributes |= css::awt::VclWindowPeerAttribute::AUTOVSCROLL;
    }
    if ( rDesc.WindowAttributes & css::awt::VclWindowPeerAttribute::HSCROLL )
    {
       rDesc.WindowAttributes &= ~css::awt::VclWindowPeerAttribute::HSCROLL;
       rDesc.WindowAttributes |= css::awt::VclWindowPeerAttribute::AUTOHSCROLL;
    }
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
stardiv_Toolkit_UnoControlContainer_get_implementation(
    css::uno::XComponentContext *,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new UnoControlContainer());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
