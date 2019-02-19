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

#include "pcrservices.hxx"
#include "propcontroller.hxx"
#include "pcrstrings.hxx"
#include "standardcontrol.hxx"
#include "linedescriptor.hxx"
#include <strings.hrc>
#include "propertyeditor.hxx"
#include "modulepcr.hxx"
#include "formstrings.hxx"
#include "formmetadata.hxx"
#include "formbrowsertools.hxx"
#include "propertycomposer.hxx"

#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/lang/NoSupportException.hpp>
#include <com/sun/star/util/XCloseable.hpp>
#include <com/sun/star/inspection/PropertyControlType.hpp>
#include <com/sun/star/ucb/AlreadyInitializedException.hpp>
#include <com/sun/star/lang/XSingleComponentFactory.hpp>
#include <com/sun/star/util/VetoException.hpp>
#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>
#include <comphelper/types.hxx>
#include <toolkit/awt/vclxwindow.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <comphelper/property.hxx>
#include <vcl/weld.hxx>
#include <vcl/svapp.hxx>
#include <vcl/tabpage.hxx>
#include <osl/mutex.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <cppuhelper/component_context.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <cppuhelper/supportsservice.hxx>

#include <algorithm>
#include <functional>
#include <sal/macros.h>
#include <sal/log.hxx>


// !!! outside the namespace !!!
extern "C" void createRegistryInfo_OPropertyBrowserController()
{
    ::pcr::OAutoRegistration< ::pcr::OPropertyBrowserController > aAutoRegistration;
}


namespace pcr
{


    using namespace ::com::sun::star;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::awt;
    using namespace ::com::sun::star::form;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::script;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::container;
    using namespace ::com::sun::star::frame;
    using namespace ::com::sun::star::util;
    using namespace ::com::sun::star::inspection;
    using namespace ::com::sun::star::ucb;
    using namespace ::comphelper;


    //= OPropertyBrowserController


    OPropertyBrowserController::OPropertyBrowserController( const Reference< XComponentContext >& _rxContext )
            :m_xContext(_rxContext)
            ,m_aDisposeListeners( m_aMutex )
            ,m_aControlObservers( m_aMutex )
            ,m_pView(nullptr)
            ,m_bContainerFocusListening( false )
            ,m_bSuspendingPropertyHandlers( false )
            ,m_bConstructed( false )
            ,m_bBindingIntrospectee( false )
    {
    }


    OPropertyBrowserController::~OPropertyBrowserController()
    {
        // stop listening for property changes
        acquire();
        stopInspection( true );
    }


    IMPLEMENT_FORWARD_REFCOUNT( OPropertyBrowserController, OPropertyBrowserController_Base )


    Any SAL_CALL OPropertyBrowserController::queryInterface( const Type& _rType )
    {
        Any aReturn = OPropertyBrowserController_Base::queryInterface( _rType );
        if ( !aReturn.hasValue() )
            aReturn = ::cppu::queryInterface(
                _rType,
                static_cast< XObjectInspectorUI* >( this )
            );
        return aReturn;
    }


    void OPropertyBrowserController::startContainerWindowListening()
    {
        if (m_bContainerFocusListening)
            return;

        if (m_xFrame.is())
        {
            Reference< XWindow > xContainerWindow = m_xFrame->getContainerWindow();
            if (xContainerWindow.is())
            {
                xContainerWindow->addFocusListener(this);
                m_bContainerFocusListening = true;
            }
        }

        DBG_ASSERT(m_bContainerFocusListening, "OPropertyBrowserController::startContainerWindowListening: unable to start listening (inconsistence)!");
    }


    void OPropertyBrowserController::stopContainerWindowListening()
    {
        if (!m_bContainerFocusListening)
            return;

        if (m_xFrame.is())
        {
            Reference< XWindow > xContainerWindow = m_xFrame->getContainerWindow();
            if (xContainerWindow.is())
            {
                xContainerWindow->removeFocusListener(this);
                m_bContainerFocusListening = false;
            }
        }

        DBG_ASSERT(!m_bContainerFocusListening, "OPropertyBrowserController::stopContainerWindowListening: unable to stop listening (inconsistence)!");
    }


    Reference< XObjectInspectorModel > SAL_CALL OPropertyBrowserController::getInspectorModel()
    {
        return m_xModel;
    }


    void OPropertyBrowserController::impl_initializeView_nothrow()
    {
        OSL_PRECOND( haveView(), "OPropertyBrowserController::impl_initializeView_nothrow: not to be called when we have no view!" );
        if ( !haveView() )
            return;

        if ( !m_xModel.is() )
            // allowed
            return;

        try
        {
            getPropertyBox().EnableHelpSection( m_xModel->getHasHelpSection() );
            getPropertyBox().SetHelpLineLimites( m_xModel->getMinHelpTextLines(), m_xModel->getMaxHelpTextLines() );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION("extensions.propctrlr");
        }
    }


    bool OPropertyBrowserController::impl_isReadOnlyModel_throw() const
    {
        if ( !m_xModel.is() )
            return false;

        return m_xModel->getIsReadOnly();
    }


    void OPropertyBrowserController::impl_startOrStopModelListening_nothrow( bool _bDoListen ) const
    {
        try
        {
            Reference< XPropertySet > xModelProperties( m_xModel, UNO_QUERY );
            if ( !xModelProperties.is() )
                // okay, so the model doesn't want to change its properties
                // dynamically - fine with us
                return;

            void (SAL_CALL XPropertySet::*pListenerOperation)( const OUString&, const Reference< XPropertyChangeListener >& )
                = _bDoListen ? &XPropertySet::addPropertyChangeListener : &XPropertySet::removePropertyChangeListener;

            (xModelProperties.get()->*pListenerOperation)(
                OUString(  "IsReadOnly"  ),
                const_cast< OPropertyBrowserController* >( this )
            );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION("extensions.propctrlr");
        }
    }


    void OPropertyBrowserController::impl_bindToNewModel_nothrow( const Reference< XObjectInspectorModel >& _rxInspectorModel )
    {
        impl_startOrStopModelListening_nothrow( false );
        m_xModel = _rxInspectorModel;
        impl_startOrStopModelListening_nothrow( true );

        // initialize the view, if we already have one
        if ( haveView() )
            impl_initializeView_nothrow();

        // inspect again, if we already have inspectees
        if ( !m_aInspectedObjects.empty() )
            impl_rebindToInspectee_nothrow( m_aInspectedObjects );
    }


    void SAL_CALL OPropertyBrowserController::setInspectorModel( const Reference< XObjectInspectorModel >& _inspectorModel )
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        if ( m_xModel == _inspectorModel )
            return;

        impl_bindToNewModel_nothrow( _inspectorModel );
    }


    Reference< XObjectInspectorUI > SAL_CALL OPropertyBrowserController::getInspectorUI()
    {
        // we're derived from this interface, though we do not expose it in queryInterface and getTypes.
        return this;
    }


    void SAL_CALL OPropertyBrowserController::inspect( const Sequence< Reference< XInterface > >& _rObjects )
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );

        if ( m_bSuspendingPropertyHandlers || !suspendAll_nothrow() )
        {   // we already are trying to suspend the component (this is somewhere up the stack)
            // OR one of our property handlers raised a veto against closing. Well, we *need* to close
            // it in order to inspect another object.
            throw VetoException();
        }
        if ( m_bBindingIntrospectee )
            throw VetoException();

        m_bBindingIntrospectee = true;
        impl_rebindToInspectee_nothrow( InterfaceArray( _rObjects.begin(), _rObjects.end() ) );
        m_bBindingIntrospectee = false;

    }


    Reference< XDispatch > SAL_CALL OPropertyBrowserController::queryDispatch( const URL& /*URL*/, const OUString& /*TargetFrameName*/, ::sal_Int32 /*SearchFlags*/ )
    {
        // we don't have any dispatches at all, right now
        return Reference< XDispatch >();
    }


    Sequence< Reference< XDispatch > > SAL_CALL OPropertyBrowserController::queryDispatches( const Sequence< DispatchDescriptor >& Requests )
    {
        Sequence< Reference< XDispatch > > aReturn;
        sal_Int32 nLen = Requests.getLength();
        aReturn.realloc( nLen );

        Reference< XDispatch >* pReturn     = aReturn.getArray();
        const   Reference< XDispatch >* pReturnEnd  = aReturn.getArray() + nLen;
        const   DispatchDescriptor*     pDescripts  = Requests.getConstArray();

        for ( ; pReturn != pReturnEnd; ++ pReturn, ++pDescripts )
            *pReturn = queryDispatch( pDescripts->FeatureURL, pDescripts->FrameName, pDescripts->SearchFlags );

        return aReturn;
    }


    void SAL_CALL OPropertyBrowserController::initialize( const Sequence< Any >& _arguments )
    {
        if ( m_bConstructed )
            throw AlreadyInitializedException();

        StlSyntaxSequence< Any > arguments( _arguments );
        if ( arguments.empty() )
        {   // constructor: "createDefault()"
            m_bConstructed = true;
            return;
        }

        Reference< XObjectInspectorModel > xModel;
        if ( arguments.size() == 1 )
        {   // constructor: "createWithModel( XObjectInspectorModel )"
            if ( !( arguments[0] >>= xModel ) )
                throw IllegalArgumentException( OUString(), *this, 0 );
            createWithModel( xModel );
            return;
        }

        throw IllegalArgumentException( OUString(), *this, 0 );
    }


    void OPropertyBrowserController::createWithModel( const Reference< XObjectInspectorModel >& _rxModel )
    {
        osl_atomic_increment( &m_refCount );
        {
            setInspectorModel( _rxModel );
        }
        osl_atomic_decrement( &m_refCount );

        m_bConstructed = true;
    }


    void SAL_CALL OPropertyBrowserController::attachFrame( const Reference< XFrame >& _rxFrame )
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );

        if (_rxFrame.is() && haveView())
            throw RuntimeException("Unable to attach to a second frame.",*this);

        // revoke as focus listener from the old container window
        stopContainerWindowListening();

        m_xFrame = _rxFrame;
        if (!m_xFrame.is())
            return;

        // TODO: this construction perhaps should be done outside. Don't know the exact meaning of attachFrame.
        // Maybe it is intended to only announce the frame to the controller, and the instance doing this
        // announcement is responsible for calling setComponent, too.
        Reference< XWindow > xContainerWindow = m_xFrame->getContainerWindow();
        VCLXWindow* pContainerWindow = VCLXWindow::GetImplementation(xContainerWindow);
        VclPtr<vcl::Window> pParentWin = pContainerWindow ? pContainerWindow->GetWindow() : VclPtr<vcl::Window>();
        if (!pParentWin)
            throw RuntimeException("The frame is invalid. Unable to extract the container window.",*this);

        Construct( pParentWin );
        try
        {
            m_xFrame->setComponent( VCLUnoHelper::GetInterface( m_pView ), this );
        }
        catch( const Exception& )
        {
            OSL_FAIL( "OPropertyBrowserController::attachFrame: caught an exception!" );
        }

        startContainerWindowListening();

        UpdateUI();
    }


    sal_Bool SAL_CALL OPropertyBrowserController::attachModel( const Reference< XModel >& _rxModel )
    {
        Reference< XObjectInspectorModel > xModel( _rxModel, UNO_QUERY );
        if ( !xModel.is() )
            return false;

        setInspectorModel( xModel );
        return getInspectorModel() == _rxModel;
    }


    bool OPropertyBrowserController::suspendAll_nothrow()
    {
        // if there is a handle inside its "onInteractivePropertySelection" method,
        // then veto
        // Normally, we could expect every handler to do this itself, but being
        // realistic, it's safer to handle this here in general.
        if ( m_xInteractiveHandler.is() )
            return false;

        m_bSuspendingPropertyHandlers = true;
        bool bHandlerVeto = !suspendPropertyHandlers_nothrow( true );
        m_bSuspendingPropertyHandlers = false;
        return !bHandlerVeto;
    }


    bool OPropertyBrowserController::suspendPropertyHandlers_nothrow( bool _bSuspend )
    {
        PropertyHandlerArray aAllHandlers;  // will contain every handler exactly once
        for (auto const& propertyHandler : m_aPropertyHandlers)
        {
            if ( std::find( aAllHandlers.begin(), aAllHandlers.end(), propertyHandler.second ) != aAllHandlers.end() )
                // already visited this particular handler (m_aPropertyHandlers usually contains
                // the same handler more than once)
                continue;
            aAllHandlers.push_back(propertyHandler.second);
        }

        for (auto const& handler : aAllHandlers)
        {
            try
            {
                if ( !handler->suspend( _bSuspend ) )
                    if ( _bSuspend )
                        // if we're not suspending, but reactivating, ignore the error
                        return false;
            }
            catch( const Exception& )
            {
                OSL_FAIL( "OPropertyBrowserController::suspendPropertyHandlers_nothrow: caught an exception!" );
            }
        }
        return true;
    }


    sal_Bool SAL_CALL OPropertyBrowserController::suspend( sal_Bool _bSuspend )
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        OSL_ENSURE( haveView(), "OPropertyBrowserController::suspend: don't have a view anymore!" );

        if ( !_bSuspend )
        {   // this means a "suspend" is to be "revoked"
            suspendPropertyHandlers_nothrow( false );
            // we ourself cannot revoke our suspend
            return false;
        }

        if ( !suspendAll_nothrow() )
            return false;

        // commit the editor's content
        if ( haveView() )
            getPropertyBox().CommitModified();

        // stop listening
        stopContainerWindowListening();

        // outta here
        return true;
    }


    Any SAL_CALL OPropertyBrowserController::getViewData(  )
    {
        return makeAny( m_sPageSelection );
    }


    void SAL_CALL OPropertyBrowserController::restoreViewData( const Any& Data )
    {
        OUString sPageSelection;
        if ( ( Data >>= sPageSelection ) && !sPageSelection.isEmpty() )
        {
            m_sPageSelection = sPageSelection;
            selectPageFromViewData();
        }
    }


    Reference< XModel > SAL_CALL OPropertyBrowserController::getModel(  )
    {
        // have no model
        return Reference< XModel >();
    }


    Reference< XFrame > SAL_CALL OPropertyBrowserController::getFrame(  )
    {
        return m_xFrame;
    }


    void SAL_CALL OPropertyBrowserController::dispose(  )
    {
        SolarMutexGuard aSolarGuard;

        // stop inspecting the current object
        stopInspection( false );

        // say our dispose listeners goodbye
        css::lang::EventObject aEvt;
        aEvt.Source = static_cast< ::cppu::OWeakObject* >(this);
        m_aDisposeListeners.disposeAndClear(aEvt);
        m_aControlObservers.disposeAndClear(aEvt);

        // don't delete explicitly (this is done by the frame we reside in)
        m_pView = nullptr;

        Reference< XComponent > xViewAsComp( m_xView, UNO_QUERY );
        if ( xViewAsComp.is() )
            xViewAsComp->removeEventListener( static_cast< XPropertyChangeListener* >( this ) );
        m_xView.clear( );

        m_aInspectedObjects.clear();
        impl_bindToNewModel_nothrow( nullptr );
    }


    void SAL_CALL OPropertyBrowserController::addEventListener( const Reference< XEventListener >& _rxListener )
    {
        m_aDisposeListeners.addInterface(_rxListener);
    }


    void SAL_CALL OPropertyBrowserController::removeEventListener( const Reference< XEventListener >& _rxListener )
    {
        m_aDisposeListeners.removeInterface(_rxListener);
    }


    OUString SAL_CALL OPropertyBrowserController::getImplementationName(  )
    {
        return getImplementationName_static();
    }

    sal_Bool SAL_CALL OPropertyBrowserController::supportsService( const OUString& ServiceName )
    {
        return cppu::supportsService(this, ServiceName);
    }


    Sequence< OUString > SAL_CALL OPropertyBrowserController::getSupportedServiceNames(  )
    {
        return getSupportedServiceNames_static();
    }


    OUString OPropertyBrowserController::getImplementationName_static(  )
    {
        return OUString("org.openoffice.comp.extensions.ObjectInspector");
    }


    Sequence< OUString > OPropertyBrowserController::getSupportedServiceNames_static(  )
    {
        Sequence< OUString > aSupported { "com.sun.star.inspection.ObjectInspector" };
        return aSupported;
    }


    Reference< XInterface > OPropertyBrowserController::Create(const Reference< XComponentContext >& _rxContext)
    {
        return *(new OPropertyBrowserController( _rxContext ) );
    }


    void SAL_CALL OPropertyBrowserController::focusGained( const FocusEvent& _rSource )
    {
        Reference< XWindow > xSourceWindow(_rSource.Source, UNO_QUERY);
        Reference< XWindow > xContainerWindow;
        if (m_xFrame.is())
            xContainerWindow = m_xFrame->getContainerWindow();

        if ( xContainerWindow.get() == xSourceWindow.get() )
        {   // our container window got the focus
            if ( haveView() )
                getPropertyBox().GrabFocus();
        }
    }


    void SAL_CALL OPropertyBrowserController::focusLost( const FocusEvent& /*_rSource*/ )
    {
        // not interested in
    }


    void SAL_CALL OPropertyBrowserController::disposing( const EventObject& _rSource )
    {
        if ( m_xView.is() && ( m_xView == _rSource.Source ) )
        {
            m_xView = nullptr;
            m_pView = nullptr;
        }

        auto it = std::find_if(m_aInspectedObjects.begin(), m_aInspectedObjects.end(),
            [&_rSource](const InterfaceArray::value_type& rxObj) { return rxObj == _rSource.Source; });
        if (it != m_aInspectedObjects.end())
            m_aInspectedObjects.erase(it);
    }


    IMPL_LINK_NOARG(OPropertyBrowserController, OnPageActivation, LinkParamNone*, void)
    {
        updateViewDataFromActivePage();
    }


    void OPropertyBrowserController::updateViewDataFromActivePage()
    {
        if (!haveView())
            return;

        OUString sOldSelection = m_sPageSelection;
        m_sPageSelection.clear();

        const sal_uInt16 nCurrentPage = m_pView->getActivaPage();
        if ( sal_uInt16(-1) != nCurrentPage )
        {
            for (auto const& pageId : m_aPageIds)
            {
                if ( nCurrentPage == pageId.second )
                {
                    m_sPageSelection = pageId.first;
                    break;
                }
            }
        }

        if ( !m_sPageSelection.isEmpty() )
            m_sLastValidPageSelection = m_sPageSelection;
        else if ( !sOldSelection.isEmpty() )
            m_sLastValidPageSelection = sOldSelection;
    }


    sal_uInt16 OPropertyBrowserController::impl_getPageIdForCategory_nothrow( const OUString& _rCategoryName ) const
    {
        sal_uInt16 nPageId = sal_uInt16(-1);
        HashString2Int16::const_iterator pagePos = m_aPageIds.find( _rCategoryName );
        if ( pagePos != m_aPageIds.end() )
            nPageId = pagePos->second;
        return nPageId;
    }


    void OPropertyBrowserController::selectPageFromViewData()
    {
        sal_uInt16 nNewPage = impl_getPageIdForCategory_nothrow( m_sPageSelection );

        if ( haveView() && ( nNewPage != sal_uInt16(-1) ) )
            m_pView->activatePage( nNewPage );

        // just in case ...
        updateViewDataFromActivePage();
    }


    void OPropertyBrowserController::Construct(vcl::Window* _pParentWin)
    {
        DBG_ASSERT(!haveView(), "OPropertyBrowserController::Construct: already have a view!");
        DBG_ASSERT(_pParentWin, "OPropertyBrowserController::Construct: invalid parent window!");

        m_pView = VclPtr<OPropertyBrowserView>::Create(_pParentWin);
        m_pView->setPageActivationHandler(LINK(this, OPropertyBrowserController, OnPageActivation));

        // add as dispose listener for our view. The view is disposed by the frame we're plugged into,
        // and this disposal _deletes_ the view, so it would be deadly if we use our m_pView member
        // after that
        m_xView = VCLUnoHelper::GetInterface(m_pView);
        Reference< XComponent > xViewAsComp(m_xView, UNO_QUERY);
        if (xViewAsComp.is())
            xViewAsComp->addEventListener( static_cast< XPropertyChangeListener* >( this ) );

        getPropertyBox().SetLineListener(this);
        getPropertyBox().SetControlObserver(this);
        impl_initializeView_nothrow();

        m_pView->Show();
    }


    void SAL_CALL OPropertyBrowserController::propertyChange( const PropertyChangeEvent& _rEvent )
    {
        if ( _rEvent.Source == m_xModel )
        {
            if ( _rEvent.PropertyName == "IsReadOnly" )
               // this is a huge cudgel, admitted.
                // The problem is that in case we were previously read-only, all our controls
                // were created read-only, too. We cannot simply switch them to not-read-only.
                // Even if they had an API for this, we do not know whether they were
                // originally created read-only, or if they are read-only just because
                // the model was.
                impl_rebindToInspectee_nothrow( m_aInspectedObjects );
            return;
        }

        if ( m_sCommittingProperty == _rEvent.PropertyName )
            return;

        if ( !haveView() )
            return;

        Any aNewValue( _rEvent.NewValue );
        if ( impl_hasPropertyHandlerFor_nothrow( _rEvent.PropertyName ) )
        {
            // forward the new value to the property box, to reflect the change in the UI
            aNewValue = impl_getPropertyValue_throw( _rEvent.PropertyName );

            // check whether the state is ambiguous. This is interesting in case we display the properties
            // for multiple objects at once: In this case, we'll get a notification from one of the objects,
            // but need to care for the "composed" value, which can be "ambiguous".
            PropertyHandlerRef xHandler( impl_getHandlerForProperty_throw( _rEvent.PropertyName ), UNO_SET_THROW );
            PropertyState ePropertyState( xHandler->getPropertyState( _rEvent.PropertyName ) );
            bool bAmbiguousValue = ( PropertyState_AMBIGUOUS_VALUE == ePropertyState );

            getPropertyBox().SetPropertyValue( _rEvent.PropertyName, aNewValue, bAmbiguousValue );
        }

        // if it's a actuating property, then update the UI for any dependent
        // properties
        if ( impl_isActuatingProperty_nothrow( _rEvent.PropertyName ) )
            impl_broadcastPropertyChange_nothrow( _rEvent.PropertyName, aNewValue, _rEvent.OldValue, false );
    }


    Reference< XPropertyControl > SAL_CALL OPropertyBrowserController::createPropertyControl( ::sal_Int16 ControlType, sal_Bool CreateReadOnly )
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        Reference< XPropertyControl > xControl;

        // default winbits: a border only
        WinBits nWinBits = WB_BORDER;

        // read-only-ness
        CreateReadOnly |= impl_isReadOnlyModel_throw() ? 1 : 0;
        if ( CreateReadOnly )
            nWinBits |= WB_READONLY;

        switch ( ControlType )
        {
            case PropertyControlType::StringListField:
                xControl = new OMultilineEditControl( &getPropertyBox(), eStringList, nWinBits | WB_DROPDOWN | WB_TABSTOP );
                break;

            case PropertyControlType::MultiLineTextField:
                xControl = new OMultilineEditControl( &getPropertyBox(), eMultiLineText, nWinBits | WB_DROPDOWN | WB_TABSTOP );
                break;

            case PropertyControlType::ListBox:
                xControl = new OListboxControl( &getPropertyBox(), nWinBits | WB_TABSTOP | WB_DROPDOWN);
                break;

            case PropertyControlType::ComboBox:
                xControl = new OComboboxControl( &getPropertyBox(), nWinBits | WB_TABSTOP | WB_DROPDOWN);
                break;

            case PropertyControlType::TextField:
                xControl = new OEditControl( &getPropertyBox(), false, nWinBits | WB_TABSTOP );
                break;

            case PropertyControlType::CharacterField:
                xControl = new OEditControl( &getPropertyBox(), true, nWinBits | WB_TABSTOP );
                break;

            case PropertyControlType::NumericField:
                xControl = new ONumericControl( &getPropertyBox(), nWinBits | WB_TABSTOP | WB_SPIN | WB_REPEAT );
                break;

            case PropertyControlType::DateTimeField:
                xControl = new ODateTimeControl( &getPropertyBox(), nWinBits | WB_TABSTOP );
                break;

            case PropertyControlType::DateField:
                xControl = new ODateControl( &getPropertyBox(), nWinBits | WB_TABSTOP | WB_SPIN | WB_REPEAT );
                break;

            case PropertyControlType::TimeField:
                xControl = new OTimeControl( &getPropertyBox(), nWinBits | WB_TABSTOP | WB_SPIN | WB_REPEAT );
                break;

            case PropertyControlType::ColorListBox:
                xControl = new OColorControl( &getPropertyBox(), nWinBits | WB_TABSTOP | WB_DROPDOWN );
                break;

            case PropertyControlType::HyperlinkField:
                xControl = new OHyperlinkControl( &getPropertyBox(), nWinBits | WB_TABSTOP | WB_DROPDOWN );
                break;

            default:
                throw IllegalArgumentException( OUString(), *this, 1 );
        }

        return xControl;
    }


    void OPropertyBrowserController::impl_toggleInspecteeListening_nothrow( bool _bOn )
    {
        for (auto const& inspectedObject : m_aInspectedObjects)
        {
            try
            {
                Reference< XComponent > xComp( inspectedObject, UNO_QUERY );
                if ( xComp.is() )
                {
                    if ( _bOn )
                        xComp->addEventListener( static_cast< XPropertyChangeListener* >( this ) );
                    else
                        xComp->removeEventListener( static_cast< XPropertyChangeListener* >( this ) );
                }
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION("extensions.propctrlr");
            }
        }
    }


    void OPropertyBrowserController::stopInspection( bool _bCommitModified )
    {
        if ( haveView() )
        {
            if ( _bCommitModified )
                // commit the editor's content
                getPropertyBox().CommitModified();

            // hide the property box so that it does not flicker
            getPropertyBox().Hide();

            // clear the property box
            getPropertyBox().ClearAll();
        }

        // destroy the view first
        if ( haveView() )
        {
            // remove the pages
            for (auto const& pageId : m_aPageIds)
                getPropertyBox().RemovePage( pageId.second );
            clearContainer( m_aPageIds );
        }

        clearContainer( m_aProperties );

        // de-register as dispose-listener from our inspected objects
        impl_toggleInspecteeListening_nothrow( false );

        // handlers are obsolete, so is our "composer" for their UI requests
        if (m_pUIRequestComposer)
            m_pUIRequestComposer->dispose();
        m_pUIRequestComposer.reset();

        // clean up the property handlers
        PropertyHandlerArray aAllHandlers;  // will contain every handler exactly once
        for (auto const& propertyHandler : m_aPropertyHandlers)
            if ( std::find( aAllHandlers.begin(), aAllHandlers.end(), propertyHandler.second ) == aAllHandlers.end() )
                aAllHandlers.push_back( propertyHandler.second );

        for (auto const& handler : aAllHandlers)
        {
            try
            {
                handler->removePropertyChangeListener( this );
                handler->dispose();
            }
            catch( const DisposedException& )
            {
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION("extensions.propctrlr");
            }
        }

        clearContainer( m_aPropertyHandlers );
        clearContainer( m_aDependencyHandlers );
    }


    bool OPropertyBrowserController::impl_hasPropertyHandlerFor_nothrow( const OUString& _rPropertyName ) const
    {
        PropertyHandlerRepository::const_iterator handlerPos = m_aPropertyHandlers.find( _rPropertyName );
        return ( handlerPos != m_aPropertyHandlers.end() );
    }


    OPropertyBrowserController::PropertyHandlerRef const & OPropertyBrowserController::impl_getHandlerForProperty_throw( const OUString& _rPropertyName ) const
    {
        PropertyHandlerRepository::const_iterator handlerPos = m_aPropertyHandlers.find( _rPropertyName );
        if ( handlerPos == m_aPropertyHandlers.end() )
            throw RuntimeException();
        return handlerPos->second;
    }


    Any OPropertyBrowserController::impl_getPropertyValue_throw( const OUString& _rPropertyName )
    {
        PropertyHandlerRef handler = impl_getHandlerForProperty_throw( _rPropertyName );
        return handler->getPropertyValue( _rPropertyName );
    }


    void OPropertyBrowserController::impl_rebindToInspectee_nothrow( const InterfaceArray& _rObjects )
    {
        try
        {
            // stop inspecting the old object(s)
            stopInspection( true );

            // inspect the new object(s)
            m_aInspectedObjects = _rObjects;
            doInspection();

            // update the user interface
            UpdateUI();
        }

        catch(const Exception&)
        {
            OSL_FAIL("OPropertyBrowserController::impl_rebindToInspectee_nothrow: caught an exception !");
        }
    }


    void OPropertyBrowserController::doInspection()
    {
        try
        {

            // obtain the properties of the object
            std::vector< Property > aProperties;

            PropertyHandlerArray aPropertyHandlers;
            getPropertyHandlers( m_aInspectedObjects, aPropertyHandlers );

            PropertyHandlerArray::iterator aHandler( aPropertyHandlers.begin() );
            while ( aHandler != aPropertyHandlers.end() )
            {
                DBG_ASSERT( aHandler->get(), "OPropertyBrowserController::doInspection: invalid handler!" );

                StlSyntaxSequence< Property > aThisHandlersProperties(  (*aHandler)->getSupportedProperties() );

                if ( aThisHandlersProperties.empty() )
                {
                    // this handler doesn't know anything about the current inspectee -> ignore it
                    (*aHandler)->dispose();
                    aHandler = aPropertyHandlers.erase( aHandler );
                    continue;
                }

                // append these properties to our "all properties" array
                aProperties.reserve( aProperties.size() + aThisHandlersProperties.size() );
                for (const auto & aThisHandlersPropertie : aThisHandlersProperties)
                {
                    auto noPrevious = std::none_of(
                        aProperties.begin(),
                        aProperties.end(),
                        FindPropertyByName( aThisHandlersPropertie.Name )
                    );
                    if ( noPrevious )
                    {
                        aProperties.push_back( aThisHandlersPropertie );
                        continue;
                    }

                    // there already was another (previous) handler which supported this property.
                    // Don't add it to aProperties, again.

                    // Also, ensure that handlers which previously expressed interest in *changes*
                    // of this property are not notified.
                    // This is 'cause we have a new handler which is responsible for this property,
                    // which means it can give it a completely different meaning than the previous
                    // handler for this property is prepared for.
                    std::pair< PropertyHandlerMultiRepository::iterator, PropertyHandlerMultiRepository::iterator >
                        aDepHandlers = m_aDependencyHandlers.equal_range( aThisHandlersPropertie.Name );
                    m_aDependencyHandlers.erase( aDepHandlers.first, aDepHandlers.second );
                }

                // determine the superseded properties
                StlSyntaxSequence< OUString > aSupersededByThisHandler( (*aHandler)->getSupersededProperties() );
                for (const auto & superseded : aSupersededByThisHandler)
                {
                    std::vector< Property >::iterator existent = std::find_if(
                        aProperties.begin(),
                        aProperties.end(),
                        FindPropertyByName( superseded )
                    );
                    if ( existent != aProperties.end() )
                        // one of the properties superseded by this handler was supported by a previous
                        // one -> erase
                        aProperties.erase( existent );
                }

                // be notified of changes which this handler is responsible for
                (*aHandler)->addPropertyChangeListener( this );

                // remember this handler for every of the properties which it is responsible
                // for
                for (const auto & aThisHandlersPropertie : aThisHandlersProperties)
                {
                    m_aPropertyHandlers[ aThisHandlersPropertie.Name ] = *aHandler;
                    // note that this implies that if two handlers support the same property,
                    // the latter wins
                }

                // see if the handler expresses interest in any actuating properties
                StlSyntaxSequence< OUString > aInterestingActuations( (*aHandler)->getActuatingProperties() );
                for (const auto & aInterestingActuation : aInterestingActuations)
                {
                    m_aDependencyHandlers.emplace( aInterestingActuation, *aHandler );
                }

                ++aHandler;
            }

            // create a new composer for UI requests coming from the handlers
            m_pUIRequestComposer.reset( new ComposedPropertyUIUpdate( getInspectorUI(), this ) );

            // sort the properties by relative position, as indicated by the model
            sal_Int32 nPos = 0;
            for (auto const& sourceProps : aProperties)
            {
                sal_Int32 nRelativePropertyOrder = nPos;
                if ( m_xModel.is() )
                    nRelativePropertyOrder = m_xModel->getPropertyOrderIndex( sourceProps.Name );
                m_aProperties.emplace(nRelativePropertyOrder, sourceProps);
                ++nPos;
            }

            // be notified when one of our inspectees dies
            impl_toggleInspecteeListening_nothrow( true );
        }
        catch(const Exception&)
        {
            OSL_FAIL("OPropertyBrowserController::doInspection : caught an exception !");
        }
    }


    css::awt::Size SAL_CALL OPropertyBrowserController::getMinimumSize()
    {
        css::awt::Size aSize;
        if( m_pView )
            return m_pView->getMinimumSize();
        else
            return aSize;
    }


    css::awt::Size SAL_CALL OPropertyBrowserController::getPreferredSize()
    {
        return getMinimumSize();
    }


    css::awt::Size SAL_CALL OPropertyBrowserController::calcAdjustedSize( const css::awt::Size& _rNewSize )
    {
        awt::Size aMinSize = getMinimumSize( );
        awt::Size aAdjustedSize( _rNewSize );
        if ( aAdjustedSize.Width < aMinSize.Width )
            aAdjustedSize.Width = aMinSize.Width;
        if ( aAdjustedSize.Height < aMinSize.Height )
            aAdjustedSize.Height = aMinSize.Height;
        return aAdjustedSize;
    }


    void OPropertyBrowserController::describePropertyLine( const Property& _rProperty, OLineDescriptor& _rDescriptor )
    {
        try
        {
            PropertyHandlerRepository::const_iterator handler = m_aPropertyHandlers.find( _rProperty.Name );
            if ( handler == m_aPropertyHandlers.end() )
                throw RuntimeException();   // caught below

            _rDescriptor.assignFrom( handler->second->describePropertyLine( _rProperty.Name, this ) );


            _rDescriptor.xPropertyHandler = handler->second;
            _rDescriptor.sName = _rProperty.Name;
            _rDescriptor.aValue = _rDescriptor.xPropertyHandler->getPropertyValue( _rProperty.Name );

            if ( _rDescriptor.DisplayName.isEmpty() )
            {
            #ifdef DBG_UTIL
                SAL_WARN( "extensions.propctrlr", "OPropertyBrowserController::describePropertyLine: handler did not provide a display name for '"
                        <<_rProperty.Name << "'!" );
            #endif
                _rDescriptor.DisplayName = _rProperty.Name;
            }

            PropertyState   ePropertyState( _rDescriptor.xPropertyHandler->getPropertyState( _rProperty.Name ) );
            if ( PropertyState_AMBIGUOUS_VALUE == ePropertyState )
            {
                _rDescriptor.bUnknownValue = true;
                _rDescriptor.aValue.clear();
            }

            _rDescriptor.bReadOnly = impl_isReadOnlyModel_throw();
        }
        catch( const Exception& )
        {
            OSL_FAIL( "OPropertyBrowserController::describePropertyLine: caught an exception!" );
        }
    }


    void OPropertyBrowserController::impl_buildCategories_throw()
    {
        OSL_PRECOND( m_aPageIds.empty(), "OPropertyBrowserController::impl_buildCategories_throw: duplicate call!" );

        StlSyntaxSequence< PropertyCategoryDescriptor > aCategories;
        if ( m_xModel.is() )
            aCategories = StlSyntaxSequence< PropertyCategoryDescriptor >(m_xModel->describeCategories());

        for (auto const& category : aCategories)
        {
            OSL_ENSURE( m_aPageIds.find( category.ProgrammaticName ) == m_aPageIds.end(),
                "OPropertyBrowserController::impl_buildCategories_throw: duplicate programmatic name!" );

            m_aPageIds[ category.ProgrammaticName ] =
                getPropertyBox().AppendPage( category.UIName, HelpIdUrl::getHelpId( category.HelpURL ) );
        }
    }


    void OPropertyBrowserController::UpdateUI()
    {
        try
        {
            if ( !haveView() )
                // too early, will return later
                return;

            getPropertyBox().DisableUpdate();

            bool bHaveFocus = getPropertyBox().HasChildPathFocus();

            // create our tab pages
            impl_buildCategories_throw();
            // (and allow for pages to be actually unused)
            std::set< sal_uInt16 > aUsedPages;

            // when building the UI below, remember which properties are actuating,
            // to allow for a initial actuatingPropertyChanged call
            std::vector< OUString > aActuatingProperties;
            std::vector< Any > aActuatingPropertyValues;

            // ask the handlers to describe the property UI, and insert the resulting
            // entries into our list boxes
            for (auto const& property : m_aProperties)
            {
                OLineDescriptor aDescriptor;
                describePropertyLine( property.second, aDescriptor );

                bool bIsActuatingProperty = impl_isActuatingProperty_nothrow( property.second.Name );

                SAL_WARN_IF( aDescriptor.Category.isEmpty(), "extensions.propctrlr",
                        "OPropertyBrowserController::UpdateUI: empty category provided for property '"
                        << property.second.Name << "'!");
                // finally insert this property control
                sal_uInt16 nTargetPageId = impl_getPageIdForCategory_nothrow( aDescriptor.Category );
                if ( nTargetPageId == sal_uInt16(-1) )
                {
                    // this category does not yet exist. This is allowed, as an inspector model might be lazy, and not provide
                    // any category information of its own. In this case, we have a fallback ...
                    m_aPageIds[ aDescriptor.Category ] =
                        getPropertyBox().AppendPage( aDescriptor.Category, OString() );
                    nTargetPageId = impl_getPageIdForCategory_nothrow( aDescriptor.Category );
                }

                getPropertyBox().InsertEntry( aDescriptor, nTargetPageId );
                aUsedPages.insert( nTargetPageId );

                // if it's an actuating property, remember it
                if ( bIsActuatingProperty )
                {
                    aActuatingProperties.push_back( property.second.Name );
                    aActuatingPropertyValues.push_back( impl_getPropertyValue_throw( property.second.Name ) );
                }
            }

            // update any dependencies for the actuating properties which we encountered
            {
                std::vector< Any >::const_iterator aPropertyValue = aActuatingPropertyValues.begin();
                for (auto const& actuatingProperty : aActuatingProperties)
                {
                    impl_broadcastPropertyChange_nothrow( actuatingProperty, *aPropertyValue, *aPropertyValue, true );
                    ++aPropertyValue;
                }
            }

            // remove any unused pages (which we did not encounter properties for)
            HashString2Int16 aSurvivingPageIds;
            for (auto const& pageId : m_aPageIds)
            {
                if ( aUsedPages.find( pageId.second ) == aUsedPages.end() )
                    getPropertyBox().RemovePage( pageId.second );
                else
                    aSurvivingPageIds.insert(pageId);
            }
            m_aPageIds.swap( aSurvivingPageIds );


            getPropertyBox().Show();
            getPropertyBox().EnableUpdate();
            if ( bHaveFocus )
                getPropertyBox().GrabFocus();

            // activate the first page
            if ( !m_aPageIds.empty() )
            {
                Sequence< PropertyCategoryDescriptor > aCategories( m_xModel->describeCategories() );
                if ( aCategories.getLength() )
                    m_pView->activatePage( m_aPageIds[ aCategories[0].ProgrammaticName ] );
                else
                    // allowed: if we default-created the pages ...
                    m_pView->activatePage( m_aPageIds.begin()->second );
            }

            // activate the previously active page (if possible)
            if ( !m_sLastValidPageSelection.isEmpty() )
                m_sPageSelection = m_sLastValidPageSelection;
            selectPageFromViewData();
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION("extensions.propctrlr");
        }
    }


    void OPropertyBrowserController::Clicked( const OUString& _rName, bool _bPrimary )
    {
        try
        {
            // since the browse buttons do not get the focus when clicked with the mouse,
            // we need to commit the changes in the current property field
            getPropertyBox().CommitModified();

            PropertyHandlerRepository::const_iterator handler = m_aPropertyHandlers.find( _rName );
            DBG_ASSERT( handler != m_aPropertyHandlers.end(), "OPropertyBrowserController::Clicked: a property without handler? This will crash!" );

            ComposedUIAutoFireGuard aAutoFireGuard( *m_pUIRequestComposer );

            Any aData;
            m_xInteractiveHandler = handler->second;
            InteractiveSelectionResult eResult =
                handler->second->onInteractivePropertySelection( _rName, _bPrimary, aData,
                    m_pUIRequestComposer->getUIForPropertyHandler( handler->second ) );

            switch ( eResult )
            {
            case InteractiveSelectionResult_Cancelled:
            case InteractiveSelectionResult_Success:
                // okay, nothing to do
                break;
            case InteractiveSelectionResult_ObtainedValue:
                handler->second->setPropertyValue( _rName, aData );
                break;
            case InteractiveSelectionResult_Pending:
                // also okay, we expect that the handler has disabled the UI as necessary
                break;
            default:
                OSL_FAIL( "OPropertyBrowserController::Clicked: unknown result value!" );
                break;
            }
        }
        catch (const Exception&)
        {
            DBG_UNHANDLED_EXCEPTION("extensions.propctrlr");
        }
        m_xInteractiveHandler = nullptr;
    }


    bool OPropertyBrowserController::hasPropertyByName( const OUString& _rName )
    {
        for (auto const& property : m_aProperties)
            if ( property.second.Name == _rName )
                return true;
        return false;
    }


    void OPropertyBrowserController::Commit( const OUString& rName, const Any& _rValue )
    {
        try
        {
            OUString sPlcHolder = PcrRes(RID_EMBED_IMAGE_PLACEHOLDER);
            bool bIsPlaceHolderValue = false;

            if ( rName == PROPERTY_IMAGE_URL )
            {
                // if the prop value is the PlaceHolder
                // can ignore it
                OUString sVal;
                _rValue >>= sVal;
                if ( sVal == sPlcHolder )
                    bIsPlaceHolderValue = true;
            }
            m_sCommittingProperty = rName;

            bool bIsActuatingProperty = impl_isActuatingProperty_nothrow( rName );

            Any aOldValue;
            if ( bIsActuatingProperty )
                aOldValue = impl_getPropertyValue_throw( rName );

            // do we have a dedicated handler for this property, which we can delegate some tasks to?
            PropertyHandlerRef handler = impl_getHandlerForProperty_throw( rName );


            // set the value ( only if it's not a placeholder )
            if ( !bIsPlaceHolderValue )
                handler->setPropertyValue( rName, _rValue );


            // re-retrieve the value
            Any aNormalizedValue = handler->getPropertyValue( rName );

            // care for any inter-property dependencies
            if ( bIsActuatingProperty )
                impl_broadcastPropertyChange_nothrow( rName, aNormalizedValue, aOldValue, false );

            // and display it again. This ensures proper formatting
            getPropertyBox().SetPropertyValue( rName, aNormalizedValue, false );
        }
        catch(const PropertyVetoException& eVetoException)
        {
            std::unique_ptr<weld::MessageDialog> xInfoBox(Application::CreateMessageDialog(m_pView ? m_pView->GetFrameWeld() : nullptr,
                                                          VclMessageType::Info, VclButtonsType::Ok,
                                                          eVetoException.Message));
            xInfoBox->run();
            PropertyHandlerRef handler = impl_getHandlerForProperty_throw( rName );
            Any aNormalizedValue = handler->getPropertyValue( rName );
            getPropertyBox().SetPropertyValue( rName, aNormalizedValue, false );
        }
        catch(const Exception&)
        {
            OSL_FAIL("OPropertyBrowserController::Commit : caught an exception !");
        }

        m_sCommittingProperty.clear();
    }


    void OPropertyBrowserController::focusGained( const Reference< XPropertyControl >& Control )
    {
        m_aControlObservers.notifyEach( &XPropertyControlObserver::focusGained, Control );
    }


    void OPropertyBrowserController::valueChanged( const Reference< XPropertyControl >& Control )
    {
        m_aControlObservers.notifyEach( &XPropertyControlObserver::valueChanged, Control );
    }


    namespace
    {
        Reference< XPropertyHandler > lcl_createHandler( const Reference<XComponentContext>& _rContext, const Any& _rFactoryDescriptor )
        {
            Reference< XPropertyHandler > xHandler;

            OUString sServiceName;
            Reference< XSingleServiceFactory > xServiceFac;
            Reference< XSingleComponentFactory > xComponentFac;

            if ( _rFactoryDescriptor >>= sServiceName )
                xHandler.set( _rContext->getServiceManager()->createInstanceWithContext( sServiceName, _rContext ), UNO_QUERY );
            else if ( _rFactoryDescriptor >>= xServiceFac )
                xHandler.set(xServiceFac->createInstance(), css::uno::UNO_QUERY);
            else if ( _rFactoryDescriptor >>= xComponentFac )
                xHandler.set(xComponentFac->createInstanceWithContext( _rContext ), css::uno::UNO_QUERY);
            OSL_ENSURE(xHandler.is(),"lcl_createHandler: Can not create handler");
            return xHandler;
        }
    }


    void OPropertyBrowserController::getPropertyHandlers( const InterfaceArray& _rObjects, PropertyHandlerArray& _rHandlers )
    {
        _rHandlers.resize( 0 );
        if ( _rObjects.empty() )
            return;

        // create a component context for the handlers, containing some information about where
        // they live
        Reference< XComponentContext > xHandlerContext( m_xContext );

        // if our own creator did not pass a dialog parent window, use our own view for this
        Reference< XWindow > xParentWindow;
        Any any = m_xContext->getValueByName( "DialogParentWindow" );
        any >>= xParentWindow;
        if ( !xParentWindow.is() )
        {
            ::cppu::ContextEntry_Init aHandlerContextInfo[] =
            {
                ::cppu::ContextEntry_Init( "DialogParentWindow", makeAny( VCLUnoHelper::GetInterface( m_pView ) ) )
            };
            xHandlerContext = ::cppu::createComponentContext(
                aHandlerContextInfo, SAL_N_ELEMENTS( aHandlerContextInfo ),
                m_xContext );
        }

        Sequence< Any > aHandlerFactories;
        if ( m_xModel.is() )
            aHandlerFactories = m_xModel->getHandlerFactories();

        for ( auto const & handlerFactory : aHandlerFactories )
        {
            if ( _rObjects.size() == 1 )
            {   // we're inspecting only one object -> one handler
                Reference< XPropertyHandler > xHandler( lcl_createHandler( m_xContext, handlerFactory ) );
                if ( xHandler.is() )
                {
                    xHandler->inspect( _rObjects[0] );
                    _rHandlers.push_back( xHandler );
                }
            }
            else
            {
                // create a single handler for every single object
                std::vector< Reference< XPropertyHandler > > aSingleHandlers( _rObjects.size() );
                std::vector< Reference< XPropertyHandler > >::iterator pHandler = aSingleHandlers.begin();

                for (auto const& elem : _rObjects)
                {
                    *pHandler = lcl_createHandler( m_xContext, handlerFactory );
                    if ( pHandler->is() )
                    {
                        (*pHandler)->inspect(elem);
                        ++pHandler;
                    }
                }
                aSingleHandlers.resize( pHandler - aSingleHandlers.begin() );

                // then create a handler which composes information out of those single handlers
                if ( !aSingleHandlers.empty() )
                    _rHandlers.push_back( new PropertyComposer( aSingleHandlers ) );
            }
        }

        // note that the handlers will not be used by our caller, if they indicate that there are no
        // properties they feel responsible for
    }


    bool OPropertyBrowserController::impl_findObjectProperty_nothrow( const OUString& _rName, OrderedPropertyMap::const_iterator* _pProperty )
    {
        OrderedPropertyMap::const_iterator search = std::find_if(m_aProperties.begin(), m_aProperties.end(),
            [&_rName](const OrderedPropertyMap::value_type& rEntry) { return rEntry.second.Name == _rName; });
        if ( _pProperty )
            *_pProperty = search;
        return ( search != m_aProperties.end() );
    }


    void OPropertyBrowserController::rebuildPropertyUI( const OUString& _rPropertyName )
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        if ( !haveView() )
            throw RuntimeException();

        OrderedPropertyMap::const_iterator propertyPos;
        if ( !impl_findObjectProperty_nothrow( _rPropertyName, &propertyPos ) )
            return;

        OLineDescriptor aDescriptor;
        try
        {
            describePropertyLine( propertyPos->second, aDescriptor );
        }
        catch( const Exception& )
        {
            OSL_FAIL( "OPropertyBrowserController::rebuildPropertyUI: caught an exception!" );
        }

        getPropertyBox().ChangeEntry( aDescriptor );
   }


    void OPropertyBrowserController::enablePropertyUI( const OUString& _rPropertyName, sal_Bool _bEnable )
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        if ( !haveView() )
            throw RuntimeException();

        if ( !impl_findObjectProperty_nothrow( _rPropertyName ) )
            return;

        getPropertyBox().EnablePropertyLine( _rPropertyName, _bEnable );
    }


    void OPropertyBrowserController::enablePropertyUIElements( const OUString& _rPropertyName, sal_Int16 _nElements, sal_Bool _bEnable )
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        if ( !haveView() )
            throw RuntimeException();

        if ( !impl_findObjectProperty_nothrow( _rPropertyName ) )
            return;

        getPropertyBox().EnablePropertyControls( _rPropertyName, _nElements, _bEnable );
    }


    void OPropertyBrowserController::showPropertyUI( const OUString& _rPropertyName )
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        if ( !haveView() )
            throw RuntimeException();

        // look up the property in our object properties
        OrderedPropertyMap::const_iterator propertyPos;
        if ( !impl_findObjectProperty_nothrow( _rPropertyName, &propertyPos ) )
            return;

        if ( getPropertyBox().GetPropertyPos( _rPropertyName ) != EDITOR_LIST_ENTRY_NOTFOUND )
        {
            rebuildPropertyUI( _rPropertyName );
            return;
        }

        OLineDescriptor aDescriptor;
        describePropertyLine( propertyPos->second, aDescriptor );

        // look for the position to insert the property

        // side note: The methods GetPropertyPos and InsertEntry of the OPropertyEditor work
        // only on the current page. This implies that it's impossible to use this method here
        // to show property lines which are *not* on the current page.
        // This is sufficient for now, but should be changed in the future.

        // by definition, the properties in m_aProperties are in the order in which they appear in the UI
        // So all we need is a predecessor of pProperty in m_aProperties
        sal_uInt16 nUIPos = EDITOR_LIST_ENTRY_NOTFOUND;
        do
        {
            if ( propertyPos != m_aProperties.begin() )
                --propertyPos;
            nUIPos = getPropertyBox().GetPropertyPos( propertyPos->second.Name );
        }
        while ( ( nUIPos == EDITOR_LIST_ENTRY_NOTFOUND ) && ( propertyPos != m_aProperties.begin() ) );

        if ( nUIPos == EDITOR_LIST_ENTRY_NOTFOUND )
            // insert at the very top
            nUIPos = 0;
        else
            // insert right after the predecessor we found
            ++nUIPos;

        getPropertyBox().InsertEntry(
            aDescriptor, impl_getPageIdForCategory_nothrow( aDescriptor.Category ), nUIPos );
    }


    void OPropertyBrowserController::hidePropertyUI( const OUString& _rPropertyName )
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        if ( !haveView() )
            throw RuntimeException();

        if ( !impl_findObjectProperty_nothrow( _rPropertyName ) )
            return;

        getPropertyBox().RemoveEntry( _rPropertyName );
    }


    void OPropertyBrowserController::showCategory( const OUString& _rCategory, sal_Bool _bShow )
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        if ( !haveView() )
            throw RuntimeException();

        sal_uInt16 nPageId = impl_getPageIdForCategory_nothrow( _rCategory );
        OSL_ENSURE( nPageId != sal_uInt16(-1), "OPropertyBrowserController::showCategory: invalid category!" );

        getPropertyBox().ShowPropertyPage( nPageId, _bShow );
    }


    Reference< XPropertyControl > SAL_CALL OPropertyBrowserController::getPropertyControl( const OUString& _rPropertyName )
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        if ( !haveView() )
            throw RuntimeException();

        Reference< XPropertyControl > xControl( getPropertyBox().GetPropertyControl( _rPropertyName ) );
        return xControl;
    }


    void SAL_CALL OPropertyBrowserController::registerControlObserver( const Reference< XPropertyControlObserver >& Observer )
    {
        m_aControlObservers.addInterface( Observer );
    }


    void SAL_CALL OPropertyBrowserController::revokeControlObserver( const Reference< XPropertyControlObserver >& Observer )
    {
        m_aControlObservers.removeInterface( Observer );
    }


    void SAL_CALL OPropertyBrowserController::setHelpSectionText( const OUString& _rHelpText )
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );

        if ( !haveView() )
            throw DisposedException();

        if ( !getPropertyBox().HasHelpSection() )
            throw NoSupportException();

        getPropertyBox().SetHelpText( _rHelpText );
    }


    void OPropertyBrowserController::impl_broadcastPropertyChange_nothrow( const OUString& _rPropertyName, const Any& _rNewValue, const Any& _rOldValue, bool _bFirstTimeInit ) const
    {
        // are there one or more handlers which are interested in the actuation?
        std::pair< PropertyHandlerMultiRepository::const_iterator, PropertyHandlerMultiRepository::const_iterator > aInterestedHandlers =
            m_aDependencyHandlers.equal_range( _rPropertyName );
        if ( aInterestedHandlers.first == aInterestedHandlers.second )
            // none of our handlers is interested in this
            return;

        ComposedUIAutoFireGuard aAutoFireGuard( *m_pUIRequestComposer );
        try
        {
            // collect the responses from all interested handlers
            PropertyHandlerMultiRepository::const_iterator handler = aInterestedHandlers.first;
            while ( handler != aInterestedHandlers.second )
            {
                handler->second->actuatingPropertyChanged( _rPropertyName, _rNewValue, _rOldValue,
                    m_pUIRequestComposer->getUIForPropertyHandler( handler->second ),
                    _bFirstTimeInit );
                ++handler;
            }
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION("extensions.propctrlr");
        }
    }


} // namespace pcr


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
