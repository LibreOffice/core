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

#include <com/sun/star/awt/XVclContainerPeer.hpp>
#include <com/sun/star/beans/XPropertyChangeListener.hpp>

#include <cppuhelper/typeprovider.hxx>
#include <cppuhelper/implbase1.hxx>
#include <rtl/memory.h>
#include <rtl/uuid.h>

#include <toolkit/controls/unocontrolcontainer.hxx>
#include <toolkit/helper/property.hxx>
#include <toolkit/helper/servicenames.hxx>
#include <comphelper/sequence.hxx>

#include <tools/debug.hxx>
#include <vcl/svapp.hxx>
#include <vcl/window.hxx>

#include <limits>
#include <map>
#include <boost/shared_ptr.hpp>
#include <com/sun/star/awt/VclWindowPeerAttribute.hpp>

using namespace ::com::sun::star;

extern WorkWindow* lcl_GetDefaultWindow();

//  ----------------------------------------------------
//  class UnoControlHolder
//  ----------------------------------------------------
struct UnoControlHolder
{
    uno::Reference< awt::XControl > mxControl;
    ::rtl::OUString                 msName;

public:
    UnoControlHolder( const ::rtl::OUString& rName, const uno::Reference< awt::XControl > & rControl )
    :   mxControl( rControl ),
        msName( rName )
    {
    }

    inline const ::rtl::OUString&                   getName() const { return msName; }
    inline const uno::Reference< awt::XControl >&   getControl() const { return mxControl; }
};

class UnoControlHolderList
{
public:
    typedef sal_Int32                                       ControlIdentifier;
private:
    typedef ::boost::shared_ptr< UnoControlHolder >         ControlInfo;
    typedef ::std::map< ControlIdentifier, ControlInfo >    ControlMap;

private:
    ControlMap  maControls;

public:
    UnoControlHolderList();
    ~UnoControlHolderList();

    /** adds a control with the given name to the list
        @param _rxControl
            the control to add. Must not be <NULL/>
        @param _pBName
            the name of the control, or <NULL/> if an automatic name should be generated
        @return
            the identifier of the newly added control
    */
    ControlIdentifier   addControl( const uno::Reference< awt::XControl >& _rxControl, const ::rtl::OUString* _pName );

    /** returns the number of controls in the list
    */
    inline size_t       size() const { return maControls.size(); }

    /** determines whether or not the list is empty
    */
    inline bool         empty() const { return maControls.empty(); }

    /** retrieves all controls currently in the list
        @return
            the number of controls in the list
    */
    size_t  getControls( uno::Sequence< uno::Reference< awt::XControl > >& _out_rControls ) const;

    /** retrieves all identifiers of all controls currently in the list
        @return
            the number of controls in the list
    */
    size_t  getIdentifiers( uno::Sequence< sal_Int32 >& _out_rIdentifiers ) const;

    /** returns the first control which is registered under the given name
    */
    uno::Reference< awt::XControl >
            getControlForName( const ::rtl::OUString& _rName ) const;

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
        const ::rtl::OUString*  _pName
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
    ::rtl::OUString impl_getFreeName_throw();
};

//------------------------------------------------------------------------
UnoControlHolderList::UnoControlHolderList()
{
}

//------------------------------------------------------------------------
UnoControlHolderList::~UnoControlHolderList()
{
}

//------------------------------------------------------------------------
UnoControlHolderList::ControlIdentifier UnoControlHolderList::addControl( const uno::Reference< awt::XControl >& _rxControl, const ::rtl::OUString* _pName )
{
    return impl_addControl( _rxControl, _pName );
}

//------------------------------------------------------------------------
size_t UnoControlHolderList::getControls( uno::Sequence< uno::Reference< awt::XControl > >& _out_rControls ) const
{
    _out_rControls.realloc( maControls.size() );
    uno::Reference< awt::XControl >* pControls = _out_rControls.getArray();
    for (   ControlMap::const_iterator loop = maControls.begin();
            loop != maControls.end();
            ++loop, ++pControls
        )
        *pControls = loop->second->getControl();
    return maControls.size();
}

//------------------------------------------------------------------------
size_t UnoControlHolderList::getIdentifiers( uno::Sequence< sal_Int32 >& _out_rIdentifiers ) const
{
    _out_rIdentifiers.realloc( maControls.size() );
    sal_Int32* pIndentifiers = _out_rIdentifiers.getArray();
    for (   ControlMap::const_iterator loop = maControls.begin();
            loop != maControls.end();
            ++loop, ++pIndentifiers
        )
        *pIndentifiers = loop->first;
    return maControls.size();
}

//------------------------------------------------------------------------
uno::Reference< awt::XControl > UnoControlHolderList::getControlForName( const ::rtl::OUString& _rName ) const
{
    for (   ControlMap::const_iterator loop = maControls.begin();
            loop != maControls.end();
            ++loop
        )
        if ( loop->second->getName() == _rName )
            return loop->second->getControl();
    return uno::Reference< awt::XControl >();
}

//------------------------------------------------------------------------
UnoControlHolderList::ControlIdentifier UnoControlHolderList::getControlIdentifier( const uno::Reference< awt::XControl >& _rxControl )
{
    for (   ControlMap::iterator loop = maControls.begin();
            loop != maControls.end();
            ++loop
        )
    {
        if ( loop->second->getControl().get() == _rxControl.get() )
            return loop->first;
    }
    return -1;
}

//------------------------------------------------------------------------
bool UnoControlHolderList::getControlForIdentifier( UnoControlHolderList::ControlIdentifier _nIdentifier, uno::Reference< awt::XControl >& _out_rxControl ) const
{
    ControlMap::const_iterator pos = maControls.find( _nIdentifier );
    if ( pos == maControls.end() )
        return false;
    _out_rxControl = pos->second->getControl();
    return true;
}

//------------------------------------------------------------------------
void UnoControlHolderList::removeControlById( UnoControlHolderList::ControlIdentifier _nId )
{
    ControlMap::iterator pos = maControls.find( _nId );
    DBG_ASSERT( pos != maControls.end(), "UnoControlHolderList::removeControlById: invalid id!" );
    if ( pos == maControls.end() )
        return;

    maControls.erase( pos );
}

//------------------------------------------------------------------------
void UnoControlHolderList::replaceControlById( ControlIdentifier _nId, const uno::Reference< awt::XControl >& _rxNewControl )
{
    DBG_ASSERT( _rxNewControl.is(), "UnoControlHolderList::replaceControlById: invalid new control!" );

    ControlMap::iterator pos = maControls.find( _nId );
    DBG_ASSERT( pos != maControls.end(), "UnoControlHolderList::replaceControlById: invalid id!" );
    if ( pos == maControls.end() )
        return;

    pos->second.reset( new UnoControlHolder( pos->second->getName(), _rxNewControl ) );
}

//------------------------------------------------------------------------
UnoControlHolderList::ControlIdentifier UnoControlHolderList::impl_addControl( const uno::Reference< awt::XControl >& _rxControl, const ::rtl::OUString* _pName )
{
    DBG_ASSERT( _rxControl.is(), "UnoControlHolderList::impl_addControl: invalid control!" );

    ::rtl::OUString sName = _pName ? *_pName : impl_getFreeName_throw();
    sal_Int32 nId = impl_getFreeIdentifier_throw();

    maControls[ nId ] = ControlInfo( new UnoControlHolder( sName, _rxControl ) );
    return nId;
}

//------------------------------------------------------------------------
UnoControlHolderList::ControlIdentifier UnoControlHolderList::impl_getFreeIdentifier_throw()
{
    for ( ControlIdentifier candidateId = 0; candidateId < ::std::numeric_limits< ControlIdentifier >::max(); ++candidateId )
    {
        ControlMap::const_iterator existent = maControls.find( candidateId );
        if ( existent == maControls.end() )
            return candidateId;
    }
    throw uno::RuntimeException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "out of identifiers" ) ), NULL );
}

//------------------------------------------------------------------------
::rtl::OUString UnoControlHolderList::impl_getFreeName_throw()
{
    ::rtl::OUString name( RTL_CONSTASCII_USTRINGPARAM( "control_" ) );
    for ( ControlIdentifier candidateId = 0; candidateId < ::std::numeric_limits< ControlIdentifier >::max(); ++candidateId )
    {
        ::rtl::OUString candidateName( name + ::rtl::OUString::valueOf( candidateId ) );
        ControlMap::const_iterator loop = maControls.begin();
        for ( ; loop != maControls.end(); ++loop )
        {
            if ( loop->second->getName() == candidateName )
                break;
        }
        if ( loop == maControls.end() )
            return candidateName;
    }
    throw uno::RuntimeException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "out of identifiers" ) ), NULL );
}
//  ----------------------------------------------------
//  Function to set the controls' visibility according
//  to the dialog's "Step" property
//  ----------------------------------------------------
void implUpdateVisibility
(
    sal_Int32 nDialogStep,
    uno::Reference< awt::XControlContainer > xControlContainer
)
{
    uno::Sequence< uno::Reference< awt::XControl > >
        aCtrls = xControlContainer->getControls();
    const uno::Reference< awt::XControl >* pCtrls = aCtrls.getConstArray();
    sal_uInt32 nCtrls = aCtrls.getLength();
    sal_Bool bCompleteVisible = (nDialogStep == 0);
    for( sal_uInt32 n = 0; n < nCtrls; n++ )
    {
        uno::Reference< awt::XControl > xControl = pCtrls[ n ];

        sal_Bool bVisible = bCompleteVisible;
        if( !bVisible )
        {
            uno::Reference< awt::XControlModel > xModel( xControl->getModel() );
            uno::Reference< beans::XPropertySet > xPSet
                ( xModel, uno::UNO_QUERY );
            uno::Reference< beans::XPropertySetInfo >
                xInfo = xPSet->getPropertySetInfo();
            ::rtl::OUString aPropName(RTL_CONSTASCII_USTRINGPARAM( "Step" ) );
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


//  ----------------------------------------------------
//  class DialogStepChangedListener
//  ----------------------------------------------------
typedef ::cppu::WeakImplHelper1< beans::XPropertyChangeListener > PropertyChangeListenerHelper;

class DialogStepChangedListener: public PropertyChangeListenerHelper
{
private:
    uno::Reference< awt::XControlContainer > mxControlContainer;

public:
    DialogStepChangedListener( uno::Reference< awt::XControlContainer > xControlContainer )
        : mxControlContainer( xControlContainer ) {}

    // XEventListener
    virtual void SAL_CALL disposing( const  lang::EventObject& Source ) throw( uno::RuntimeException);

    // XPropertyChangeListener
    virtual void SAL_CALL propertyChange( const  beans::PropertyChangeEvent& evt ) throw( uno::RuntimeException);

};

void SAL_CALL DialogStepChangedListener::disposing( const  lang::EventObject& /*_rSource*/)
    throw( uno::RuntimeException)
{
    mxControlContainer.clear();
}

void SAL_CALL DialogStepChangedListener::propertyChange( const  beans::PropertyChangeEvent& evt )
    throw( uno::RuntimeException)
{
    // evt.PropertyName HAS to be "Step" because we only use the listener for that
    sal_Int32 nDialogStep = 0;
    evt.NewValue >>= nDialogStep;
    implUpdateVisibility( nDialogStep, mxControlContainer );
}

//  ----------------------------------------------------
//  class UnoControlContainer
//  ----------------------------------------------------
UnoControlContainer::UnoControlContainer( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& i_factory )
    :UnoControlContainer_Base( i_factory )
    ,maCListeners( *this )
{
    mpControls = new UnoControlHolderList;
}

UnoControlContainer::UnoControlContainer( const uno::Reference< lang::XMultiServiceFactory >& i_factory, const uno::Reference< awt::XWindowPeer >& xP )
    :UnoControlContainer_Base( i_factory )
    ,maCListeners( *this )
{
    setPeer( xP );
    mbDisposePeer = sal_False;
    mpControls = new UnoControlHolderList;
}

UnoControlContainer::~UnoControlContainer()
{
    DELETEZ( mpControls );
}

void UnoControlContainer::ImplActivateTabControllers()
{
    sal_uInt32 nCount = maTabControllers.getLength();
    for ( sal_uInt32 n = 0; n < nCount; n++ )
    {
        maTabControllers.getArray()[n]->setContainer( this );
        maTabControllers.getArray()[n]->activateTabOrder();
    }
}

// lang::XComponent
void UnoControlContainer::dispose(  ) throw(uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    lang::EventObject aDisposeEvent;
    aDisposeEvent.Source = static_cast< uno::XAggregation* >( this );

    // Notify listeners about disposal of this Container (This is much faster if they
    // listen on the controls and the container).
    maDisposeListeners.disposeAndClear( aDisposeEvent );
    maCListeners.disposeAndClear( aDisposeEvent );


    uno::Sequence< uno::Reference< awt::XControl > > aCtrls = getControls();
    uno::Reference< awt::XControl >* pCtrls = aCtrls.getArray();
    uno::Reference< awt::XControl >* pCtrlsEnd = pCtrls + aCtrls.getLength();

    for( ; pCtrls < pCtrlsEnd; ++pCtrls )
    {
        removingControl( *pCtrls );
        // Delete control
        (*pCtrls)->dispose();
    }


    // Delete all structures
    DELETEZ( mpControls );
    mpControls = new UnoControlHolderList;

    UnoControlBase::dispose();
}

// lang::XEventListener
void UnoControlContainer::disposing( const lang::EventObject& _rEvt ) throw(uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    uno::Reference< awt::XControl >  xControl( _rEvt.Source, uno::UNO_QUERY );
    if ( xControl.is() )
        removeControl( xControl );

    UnoControlBase::disposing( _rEvt );
}

// container::XContainer
void UnoControlContainer::addContainerListener( const uno::Reference< container::XContainerListener >& rxListener ) throw(uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    maCListeners.addInterface( rxListener );
}

void UnoControlContainer::removeContainerListener( const uno::Reference< container::XContainerListener >& rxListener ) throw(uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    maCListeners.removeInterface( rxListener );
}


::sal_Int32 SAL_CALL UnoControlContainer::insert( const uno::Any& _rElement ) throw (lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    uno::Reference< awt::XControl > xControl;
    if ( !( _rElement >>= xControl ) || !xControl.is() )
        throw lang::IllegalArgumentException(
            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Elements must support the XControl interface." ) ),
            *this,
            1
        );

    return impl_addControl( xControl, NULL );
}

void SAL_CALL UnoControlContainer::removeByIdentifier( ::sal_Int32 _nIdentifier ) throw (container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    uno::Reference< awt::XControl > xControl;
    if ( !mpControls->getControlForIdentifier( _nIdentifier, xControl ) )
        throw container::NoSuchElementException(
            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "There is no element with the given identifier." ) ),
            *this
        );

    impl_removeControl( _nIdentifier, xControl, NULL );
}

void SAL_CALL UnoControlContainer::replaceByIdentifer( ::sal_Int32 _nIdentifier, const uno::Any& _rElement ) throw (lang::IllegalArgumentException, container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    uno::Reference< awt::XControl > xExistentControl;
    if ( !mpControls->getControlForIdentifier( _nIdentifier, xExistentControl ) )
        throw container::NoSuchElementException(
            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "There is no element with the given identifier." ) ),
            *this
        );

    uno::Reference< awt::XControl > xNewControl;
    if ( !( _rElement >>= xNewControl ) )
        throw lang::IllegalArgumentException(
            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Elements must support the XControl interface." ) ),
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

uno::Any SAL_CALL UnoControlContainer::getByIdentifier( ::sal_Int32 _nIdentifier ) throw (container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    uno::Reference< awt::XControl > xControl;
    if ( !mpControls->getControlForIdentifier( _nIdentifier, xControl ) )
        throw container::NoSuchElementException();
    return uno::makeAny( xControl );
}

uno::Sequence< ::sal_Int32 > SAL_CALL UnoControlContainer::getIdentifiers(  ) throw (uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    uno::Sequence< ::sal_Int32 > aIdentifiers;
    mpControls->getIdentifiers( aIdentifiers );
    return aIdentifiers;
}

// container::XElementAccess
uno::Type SAL_CALL UnoControlContainer::getElementType(  ) throw (uno::RuntimeException)
{
    return awt::XControlModel::static_type();
}

::sal_Bool SAL_CALL UnoControlContainer::hasElements(  ) throw (uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );
    return !mpControls->empty();
}

// awt::XControlContainer
void UnoControlContainer::setStatusText( const ::rtl::OUString& rStatusText ) throw(uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    // Descend the parent hierarchy
    uno::Reference< awt::XControlContainer >  xContainer( mxContext, uno::UNO_QUERY );
    if( xContainer.is() )
        xContainer->setStatusText( rStatusText );
}

uno::Sequence< uno::Reference< awt::XControl > > UnoControlContainer::getControls(  ) throw(uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );
    uno::Sequence< uno::Reference< awt::XControl > > aControls;
    mpControls->getControls( aControls );
    return aControls;
}

uno::Reference< awt::XControl > UnoControlContainer::getControl( const ::rtl::OUString& rName ) throw(uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );
    return mpControls->getControlForName( rName );
}

void UnoControlContainer::addingControl( const uno::Reference< awt::XControl >& _rxControl )
{
    if ( _rxControl.is() )
    {
        uno::Reference< uno::XInterface > xThis;
        OWeakAggObject::queryInterface( ::getCppuType( static_cast< uno::Reference< uno::XInterface >* >( NULL ) ) ) >>= xThis;

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
        _rxControl->createPeer( NULL, xMyPeer );
        ImplActivateTabControllers();
    }

}

sal_Int32 UnoControlContainer::impl_addControl( const uno::Reference< awt::XControl >& _rxControl, const ::rtl::OUString* _pName )
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );
    UnoControlHolderList::ControlIdentifier id = mpControls->addControl( _rxControl, _pName );

    addingControl( _rxControl );

    impl_createControlPeerIfNecessary( _rxControl );

    if ( maCListeners.getLength() )
    {
        container::ContainerEvent aEvent;
        aEvent.Source = *this;
        _pName ? ( aEvent.Accessor <<= *_pName ) : ( aEvent.Accessor <<= (sal_Int32)id );
        aEvent.Element <<= _rxControl;
        maCListeners.elementInserted( aEvent );
    }

    return id;
}

void UnoControlContainer::addControl( const ::rtl::OUString& rName, const uno::Reference< awt::XControl >& rControl ) throw(uno::RuntimeException)
{
    if ( rControl.is() )
        impl_addControl( rControl, &rName );
}

void UnoControlContainer::removingControl( const uno::Reference< awt::XControl >& _rxControl )
{
    if ( _rxControl.is() )
    {
        _rxControl->removeEventListener( this );
        _rxControl->setContext( NULL );
    }
}

void UnoControlContainer::impl_removeControl( sal_Int32 _nId, const uno::Reference< awt::XControl >& _rxControl, const ::rtl::OUString* _pNameAccessor )
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
        _pNameAccessor ? ( aEvent.Accessor <<= *_pNameAccessor ) : ( aEvent.Accessor <<= _nId );
        aEvent.Element <<= _rxControl;
        maCListeners.elementRemoved( aEvent );
    }
}

void UnoControlContainer::removeControl( const uno::Reference< awt::XControl >& _rxControl ) throw(uno::RuntimeException)
{
    if ( _rxControl.is() )
    {
        ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

        UnoControlHolderList::ControlIdentifier id = mpControls->getControlIdentifier( _rxControl );
        if ( id != -1 )
            impl_removeControl( id, _rxControl, NULL );
    }
}



// awt::XUnoControlContainer
void UnoControlContainer::setTabControllers( const uno::Sequence< uno::Reference< awt::XTabController > >& TabControllers ) throw(uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    maTabControllers = TabControllers;
}

uno::Sequence< uno::Reference< awt::XTabController > > UnoControlContainer::getTabControllers(  ) throw(uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    return maTabControllers;
}

void UnoControlContainer::addTabController( const uno::Reference< awt::XTabController >& TabController ) throw(uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    sal_uInt32 nCount = maTabControllers.getLength();
    maTabControllers.realloc( nCount + 1 );
    maTabControllers[ nCount ] = TabController;
}

void UnoControlContainer::removeTabController( const uno::Reference< awt::XTabController >& TabController ) throw(uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    sal_uInt32 nCount = maTabControllers.getLength();
    const uno::Reference< awt::XTabController >* pLoop = maTabControllers.getConstArray();
    for ( sal_uInt32 n = 0; n < nCount; ++n, ++pLoop )
    {
        if( pLoop->get() == TabController.get() )
        {
            ::comphelper::removeElementAt( maTabControllers, n );
            break;
        }
    }
}

// awt::XControl
void UnoControlContainer::createPeer( const uno::Reference< awt::XToolkit >& rxToolkit, const uno::Reference< awt::XWindowPeer >& rParent ) throw(uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    if( !getPeer().is() )
    {
        sal_Bool bVis = maComponentInfos.bVisible;
        if( bVis )
            UnoControl::setVisible( sal_False );

        uno::Reference< beans::XPropertySet > xTmpPropSet
                ( getModel(), uno::UNO_QUERY );

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
            ::rtl::OUString aPropName(RTL_CONSTASCII_USTRINGPARAM( "Step" ) );
            if ( xInfo->hasPropertyByName( aPropName ) )
            {
                ::com::sun::star::uno::Any aVal = xPSet->getPropertyValue( aPropName );
                sal_Int32 nDialogStep = 0;
                aVal >>= nDialogStep;
                uno::Reference< awt::XControlContainer > xContainer =
                    (static_cast< awt::XControlContainer* >(this));
                implUpdateVisibility( nDialogStep, xContainer );

                uno::Reference< beans::XPropertyChangeListener > xListener =
                    (static_cast< beans::XPropertyChangeListener* >(
                        new DialogStepChangedListener( xContainer ) ) );
                xPSet->addPropertyChangeListener( aPropName, xListener );
            }

            uno::Sequence< uno::Reference< awt::XControl > > aCtrls = getControls();
            sal_uInt32 nCtrls = aCtrls.getLength();
            for( sal_uInt32 n = 0; n < nCtrls; n++ )
                aCtrls.getArray()[n]->createPeer( rxToolkit, getPeer() );

            uno::Reference< awt::XVclContainerPeer >  xC( getPeer(), uno::UNO_QUERY );
            if ( xC.is() )
                xC->enableDialogControl( sal_True );
            ImplActivateTabControllers();
        }

        if( bVis && !isDesignMode() )
            UnoControl::setVisible( sal_True );
    }
}


// awt::XWindow
void UnoControlContainer::setVisible( sal_Bool bVisible ) throw(uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    UnoControl::setVisible( bVisible );
    if( !mxContext.is() && bVisible )
        // This is a Topwindow, thus show it automatically
        createPeer( uno::Reference< awt::XToolkit > (), uno::Reference< awt::XWindowPeer > () );
}

void UnoControlContainer::PrepareWindowDescriptor( ::com::sun::star::awt::WindowDescriptor& rDesc )
{
    // HACK due to the fact that we can't really use VSCROLL & HSCROLL
    // for Dialog  ( ::com::sun::star::awt::VclWindowPeerAttribute::VSCROLL
    // has the same value as
    // ::com::sun::star::awt::WindowAttribute::NODECORATION )
    // For convenience in the PropBrowse using HSCROLL and VSCROLL ensures
    // the Correct text. We exchange them here and the control knows
    // about this hack ( it sucks badly I know )
    if ( rDesc.WindowAttributes & ::com::sun::star::awt::VclWindowPeerAttribute::VSCROLL )
    {
       rDesc.WindowAttributes &= ~::com::sun::star::awt::VclWindowPeerAttribute::VSCROLL;
       rDesc.WindowAttributes |= ::com::sun::star::awt::VclWindowPeerAttribute::AUTOVSCROLL;
    }
    if ( rDesc.WindowAttributes & ::com::sun::star::awt::VclWindowPeerAttribute::HSCROLL )
    {
       rDesc.WindowAttributes &= ~::com::sun::star::awt::VclWindowPeerAttribute::HSCROLL;
       rDesc.WindowAttributes |= ::com::sun::star::awt::VclWindowPeerAttribute::AUTOHSCROLL;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
