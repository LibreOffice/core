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


#include "eventhandler.hxx"
#include "pcrservices.hxx"
#include <helpids.h>
#include <propctrlr.h>
#include "formbrowsertools.hxx"
#include <strings.hrc>
#include "formstrings.hxx"
#include "handlerhelper.hxx"
#include "modulepcr.hxx"
#include "pcrcommon.hxx"
#include "pcrstrings.hxx"
#include "propertycontrolextender.hxx"

#include <com/sun/star/awt/XTabControllerModel.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/UnknownPropertyException.hpp>
#include <com/sun/star/beans/theIntrospection.hpp>
#include <com/sun/star/beans/XIntrospectionAccess.hpp>
#include <com/sun/star/container/NoSuchElementException.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/container/XNameReplace.hpp>
#include <com/sun/star/form/FormComponentType.hpp>
#include <com/sun/star/form/XForm.hpp>
#include <com/sun/star/form/runtime/FormController.hpp>
#include <com/sun/star/inspection/PropertyControlType.hpp>
#include <com/sun/star/lang/NullPointerException.hpp>
#include <com/sun/star/script/XEventAttacherManager.hpp>
#include <com/sun/star/script/XScriptEventsSupplier.hpp>
#include <com/sun/star/uri/UriReferenceFactory.hpp>
#include <com/sun/star/uri/XVndSunStarScriptUrlReference.hpp>

#include <comphelper/namedvaluecollection.hxx>
#include <comphelper/evtmethodhelper.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/types.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <rtl/ref.hxx>
#include <rtl/ustrbuf.hxx>
#include <sfx2/app.hxx>
#include <svl/eitem.hxx>
#include <svl/itemset.hxx>
#include <svx/svxdlg.hxx>
#include <svx/svxids.hrc>
#include <tools/diagnose_ex.h>

#include <map>
#include <algorithm>
#include <iterator>
#include <o3tl/functional.hxx>

extern "C" void createRegistryInfo_EventHandler()
{
    ::pcr::OAutoRegistration< ::pcr::EventHandler > aAutoRegistration;
}

namespace pcr
{

    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::XComponentContext;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::TypeClass_STRING;
    using ::com::sun::star::uno::Type;
    using ::com::sun::star::beans::theIntrospection;
    using ::com::sun::star::beans::XPropertyChangeListener;
    using ::com::sun::star::beans::Property;
    using ::com::sun::star::beans::PropertyState;
    using ::com::sun::star::beans::PropertyState_DIRECT_VALUE;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::script::ScriptEventDescriptor;
    using ::com::sun::star::script::XScriptEventsSupplier;
    using ::com::sun::star::lang::NullPointerException;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::container::XChild;
    using ::com::sun::star::container::XIndexAccess;
    using ::com::sun::star::script::XEventAttacherManager;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::uno::XInterface;
    using ::com::sun::star::beans::XIntrospection;
    using ::com::sun::star::beans::XIntrospectionAccess;
    using ::com::sun::star::container::XNameContainer;
    using ::com::sun::star::awt::XTabControllerModel;
    using ::com::sun::star::form::XForm;
    using ::com::sun::star::form::runtime::FormController;
    using ::com::sun::star::form::runtime::XFormController;
    using ::com::sun::star::beans::UnknownPropertyException;
    using ::com::sun::star::uno::makeAny;
    using ::com::sun::star::container::NoSuchElementException;
    using ::com::sun::star::beans::XPropertySetInfo;
    using ::com::sun::star::container::XNameReplace;
    using ::com::sun::star::lang::IllegalArgumentException;
    using ::com::sun::star::lang::WrappedTargetException;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::beans::PropertyValue;
    using ::com::sun::star::inspection::LineDescriptor;
    using ::com::sun::star::inspection::XPropertyControlFactory;
    using ::com::sun::star::inspection::InteractiveSelectionResult;
    using ::com::sun::star::inspection::InteractiveSelectionResult_Cancelled;
    using ::com::sun::star::inspection::InteractiveSelectionResult_Success;
    using ::com::sun::star::inspection::XObjectInspectorUI;
    using ::com::sun::star::beans::PropertyChangeEvent;
    using ::com::sun::star::frame::XFrame;
    using ::com::sun::star::frame::XModel;
    using ::com::sun::star::frame::XController;
    using ::com::sun::star::uno::UNO_SET_THROW;
    using com::sun::star::uri::UriReferenceFactory;
    using com::sun::star::uri::XUriReferenceFactory;
    using com::sun::star::uri::XVndSunStarScriptUrlReference;
    using ::com::sun::star::lang::XEventListener;

    namespace PropertyControlType = css::inspection::PropertyControlType;
    namespace PropertyAttribute = css::beans::PropertyAttribute;
    namespace FormComponentType = css::form::FormComponentType;

    EventDescription::EventDescription( EventId _nId, const sal_Char* _pListenerNamespaceAscii, const sal_Char* _pListenerClassAsciiName,
            const sal_Char* _pListenerMethodAsciiName, const char* pDisplayNameResId, const OString& _sHelpId, const OString& _sUniqueBrowseId )
        :sDisplayName(PcrRes( pDisplayNameResId ))
        ,sListenerMethodName( OUString::createFromAscii( _pListenerMethodAsciiName ) )
        ,sHelpId( _sHelpId )
        ,sUniqueBrowseId( _sUniqueBrowseId )
        ,nId( _nId )
    {
        OUStringBuffer aQualifiedListenerClass;
        aQualifiedListenerClass.append( "com.sun.star." );
        aQualifiedListenerClass.appendAscii( _pListenerNamespaceAscii );
        aQualifiedListenerClass.append( "." );
        aQualifiedListenerClass.appendAscii( _pListenerClassAsciiName );
        sListenerClassName = aQualifiedListenerClass.makeStringAndClear();
    }

    namespace
    {
        #define DESCRIBE_EVENT( map, asciinamespace, asciilistener, asciimethod, id_postfix ) \
            map.emplace(  \
                asciimethod, \
                EventDescription( ++nEventId, asciinamespace, asciilistener, asciimethod, RID_STR_EVT_##id_postfix, HID_EVT_##id_postfix, UID_BRWEVT_##id_postfix ) )

        bool lcl_getEventDescriptionForMethod( const OUString& _rMethodName, EventDescription& _out_rDescription )
        {
            static EventMap s_aKnownEvents = []() {
                EventMap aMap;
                sal_Int32 nEventId = 0;

                DESCRIBE_EVENT(aMap, "form", "XApproveActionListener",     "approveAction",          APPROVEACTIONPERFORMED);
                DESCRIBE_EVENT(aMap, "awt",  "XActionListener",            "actionPerformed",        ACTIONPERFORMED);
                DESCRIBE_EVENT(aMap, "form", "XChangeListener",            "changed",                CHANGED);
                DESCRIBE_EVENT(aMap, "awt",  "XTextListener",              "textChanged",            TEXTCHANGED);
                DESCRIBE_EVENT(aMap, "awt",  "XItemListener",              "itemStateChanged",       ITEMSTATECHANGED);
                DESCRIBE_EVENT(aMap, "awt",  "XFocusListener",             "focusGained",            FOCUSGAINED);
                DESCRIBE_EVENT(aMap, "awt",  "XFocusListener",             "focusLost",              FOCUSLOST);
                DESCRIBE_EVENT(aMap, "awt",  "XKeyListener",               "keyPressed",             KEYTYPED);
                DESCRIBE_EVENT(aMap, "awt",  "XKeyListener",               "keyReleased",            KEYUP);
                DESCRIBE_EVENT(aMap, "awt",  "XMouseListener",             "mouseEntered",           MOUSEENTERED);
                DESCRIBE_EVENT(aMap, "awt",  "XMouseMotionListener",       "mouseDragged",           MOUSEDRAGGED);
                DESCRIBE_EVENT(aMap, "awt",  "XMouseMotionListener",       "mouseMoved",             MOUSEMOVED);
                DESCRIBE_EVENT(aMap, "awt",  "XMouseListener",             "mousePressed",           MOUSEPRESSED);
                DESCRIBE_EVENT(aMap, "awt",  "XMouseListener",             "mouseReleased",          MOUSERELEASED);
                DESCRIBE_EVENT(aMap, "awt",  "XMouseListener",             "mouseExited",            MOUSEEXITED);
                DESCRIBE_EVENT(aMap, "form", "XResetListener",             "approveReset",           APPROVERESETTED);
                DESCRIBE_EVENT(aMap, "form", "XResetListener",             "resetted",               RESETTED);
                DESCRIBE_EVENT(aMap, "form", "XSubmitListener",            "approveSubmit",          SUBMITTED);
                DESCRIBE_EVENT(aMap, "form", "XUpdateListener",            "approveUpdate",          BEFOREUPDATE);
                DESCRIBE_EVENT(aMap, "form", "XUpdateListener",            "updated",                AFTERUPDATE);
                DESCRIBE_EVENT(aMap, "form", "XLoadListener",              "loaded",                 LOADED);
                DESCRIBE_EVENT(aMap, "form", "XLoadListener",              "reloading",              RELOADING);
                DESCRIBE_EVENT(aMap, "form", "XLoadListener",              "reloaded",               RELOADED);
                DESCRIBE_EVENT(aMap, "form", "XLoadListener",              "unloading",              UNLOADING);
                DESCRIBE_EVENT(aMap, "form", "XLoadListener",              "unloaded",               UNLOADED);
                DESCRIBE_EVENT(aMap, "form", "XConfirmDeleteListener",     "confirmDelete",          CONFIRMDELETE);
                DESCRIBE_EVENT(aMap, "sdb",  "XRowSetApproveListener",     "approveRowChange",       APPROVEROWCHANGE);
                DESCRIBE_EVENT(aMap, "sdbc", "XRowSetListener",            "rowChanged",             ROWCHANGE);
                DESCRIBE_EVENT(aMap, "sdb",  "XRowSetApproveListener",     "approveCursorMove",      POSITIONING);
                DESCRIBE_EVENT(aMap, "sdbc", "XRowSetListener",            "cursorMoved",            POSITIONED);
                DESCRIBE_EVENT(aMap, "form", "XDatabaseParameterListener", "approveParameter",       APPROVEPARAMETER);
                DESCRIBE_EVENT(aMap, "sdb",  "XSQLErrorListener",          "errorOccured",           ERROROCCURRED);
                DESCRIBE_EVENT(aMap, "awt",  "XAdjustmentListener",        "adjustmentValueChanged", ADJUSTMENTVALUECHANGED);

                return aMap;
            }();

            EventMap::const_iterator pos = s_aKnownEvents.find( _rMethodName );
            if ( pos == s_aKnownEvents.end() )
                return false;

            _out_rDescription = pos->second;
            return true;
        }

        OUString lcl_getEventPropertyName( const OUString& _rListenerClassName, const OUString& _rMethodName )
        {
            return _rListenerClassName + OUStringLiteral1(';') + _rMethodName;
        }

        ScriptEventDescriptor lcl_getAssignedScriptEvent( const EventDescription& _rEvent, const std::vector< ScriptEventDescriptor >& _rAllAssignedMacros )
        {
            ScriptEventDescriptor aScriptEvent;
            // for the case there is actually no event assigned, initialize at least ListenerType and MethodName,
            //  so this ScriptEventDescriptor properly describes the given event
            aScriptEvent.ListenerType = _rEvent.sListenerClassName;
            aScriptEvent.EventMethod = _rEvent.sListenerMethodName;

            for ( const ScriptEventDescriptor& rSED :  _rAllAssignedMacros )
            {
                if  (   rSED.ListenerType != _rEvent.sListenerClassName
                    ||  rSED.EventMethod != _rEvent.sListenerMethodName
                    )
                    continue;

                if  (  rSED.ScriptCode.isEmpty()
                    || rSED.ScriptType.isEmpty()
                    )
                {
                    OSL_FAIL( "lcl_getAssignedScriptEvent: me thinks this should not happen!" );
                    continue;
                }

                aScriptEvent = rSED;

                if ( aScriptEvent.ScriptType != "StarBasic" )
                    continue;

                // this is an old-style macro specification:
                // [document|application]:Library.Module.Function
                // we need to translate this to the new-style macro specification
                // vnd.sun.star.script:Library.Module.Function?language=Basic&location=[document|application]

                sal_Int32 nPrefixLen = aScriptEvent.ScriptCode.indexOf( ':' );
                OSL_ENSURE( nPrefixLen > 0, "lcl_getAssignedScriptEvent: illegal location!" );
                OUString sLocation = aScriptEvent.ScriptCode.copy( 0, nPrefixLen );
                OUString sMacroPath = aScriptEvent.ScriptCode.copy( nPrefixLen + 1 );

                OUStringBuffer aNewStyleSpec;
                aNewStyleSpec.append( "vnd.sun.star.script:" );
                aNewStyleSpec.append     ( sMacroPath );
                aNewStyleSpec.append( "?language=Basic&location=" );
                aNewStyleSpec.append     ( sLocation );

                aScriptEvent.ScriptCode = aNewStyleSpec.makeStringAndClear();

                // also, this new-style spec requires the script code to be "Script" instead of "StarBasic"
                aScriptEvent.ScriptType = "Script";
            }
            return aScriptEvent;
        }

        OUString lcl_getQualifiedKnownListenerName( const ScriptEventDescriptor& _rFormComponentEventDescriptor )
        {
            EventDescription aKnownEvent;
            if ( lcl_getEventDescriptionForMethod( _rFormComponentEventDescriptor.EventMethod, aKnownEvent ) )
                return aKnownEvent.sListenerClassName;
            OSL_FAIL( "lcl_getQualifiedKnownListenerName: unknown method name!" );
                // somebody assigned an script to a form component event which we don't know
                // Speaking strictly, this is not really an error - it is possible to do
                // this programmatically -, but it should rarely happen, since it's not possible
                // via UI
            return _rFormComponentEventDescriptor.ListenerType;
        }

        typedef std::set< Type, TypeLessByName > TypeBag;

        void lcl_addListenerTypesFor_throw( const Reference< XInterface >& _rxComponent,
            const Reference< XIntrospection >& _rxIntrospection, TypeBag& _out_rTypes )
        {
            if ( !_rxComponent.is() )
                return;
            OSL_PRECOND( _rxIntrospection.is(), "lcl_addListenerTypesFor_throw: this will crash!" );

            Reference< XIntrospectionAccess > xIntrospectionAccess(
                _rxIntrospection->inspect( makeAny( _rxComponent ) ), UNO_QUERY_THROW );

            Sequence< Type > aListeners( xIntrospectionAccess->getSupportedListeners() );

            std::copy( aListeners.begin(), aListeners.end(),
                         std::insert_iterator< TypeBag >( _out_rTypes, _out_rTypes.begin() ) );
        }
    }

    typedef ::cppu::WeakImplHelper <   css::container::XNameReplace
                                    >   EventHolder_Base;
    /* An UNO component holding assigned event descriptions, for use with a SvxMacroAssignDlg */
    class EventHolder : public EventHolder_Base
    {
    private:
        typedef std::unordered_map< OUString, ScriptEventDescriptor >  EventMap;
        typedef std::map< EventId, EventMap::iterator >                                       EventMapIndexAccess;

        EventMap            m_aEventNameAccess;
        EventMapIndexAccess m_aEventIndexAccess;

    public:
        EventHolder( );

        void addEvent( EventId _nId, const OUString& _rEventName, const ScriptEventDescriptor& _rScriptEvent );

        /** effectively the same as getByName, but instead of converting the ScriptEventDescriptor to the weird
            format used by the macro assignment dialog, it is returned directly
        */
        ScriptEventDescriptor getNormalizedDescriptorByName( const OUString& _rEventName ) const;

        // XNameReplace
        virtual void SAL_CALL replaceByName( const OUString& _rName, const Any& aElement ) override;
        virtual Any SAL_CALL getByName( const OUString& _rName ) override;
        virtual Sequence< OUString > SAL_CALL getElementNames(  ) override;
        virtual sal_Bool SAL_CALL hasByName( const OUString& _rName ) override;
        virtual Type SAL_CALL getElementType(  ) override;
        virtual sal_Bool SAL_CALL hasElements(  ) override;

    protected:
        virtual ~EventHolder( ) override;

    private:
        ScriptEventDescriptor const & impl_getDescriptor_throw( const OUString& _rEventName ) const;
    };


    EventHolder::EventHolder()
    {
    }

    EventHolder::~EventHolder()
    {
        m_aEventNameAccess.clear();
        m_aEventIndexAccess.clear();
    }

    void EventHolder::addEvent( EventId _nId, const OUString& _rEventName, const ScriptEventDescriptor& _rScriptEvent )
    {
        std::pair< EventMap::iterator, bool > insertionResult =
            m_aEventNameAccess.emplace( _rEventName, _rScriptEvent );
        OSL_ENSURE( insertionResult.second, "EventHolder::addEvent: there already was a MacroURL for this event!" );
        m_aEventIndexAccess[ _nId ] = insertionResult.first;
    }

    ScriptEventDescriptor EventHolder::getNormalizedDescriptorByName( const OUString& _rEventName ) const
    {
        return impl_getDescriptor_throw( _rEventName );
    }

    ScriptEventDescriptor const & EventHolder::impl_getDescriptor_throw( const OUString& _rEventName ) const
    {
        EventMap::const_iterator pos = m_aEventNameAccess.find( _rEventName );
        if ( pos == m_aEventNameAccess.end() )
            throw NoSuchElementException( OUString(), *const_cast< EventHolder* >( this ) );
        return pos->second;
    }

    void SAL_CALL EventHolder::replaceByName( const OUString& _rName, const Any& _rElement )
    {
        EventMap::iterator pos = m_aEventNameAccess.find( _rName );
        if ( pos == m_aEventNameAccess.end() )
            throw NoSuchElementException( OUString(), *this );

        Sequence< PropertyValue > aScriptDescriptor;
        OSL_VERIFY( _rElement >>= aScriptDescriptor );

        ::comphelper::NamedValueCollection aExtractor( aScriptDescriptor );

        pos->second.ScriptType = aExtractor.getOrDefault( "EventType", OUString() );
        pos->second.ScriptCode = aExtractor.getOrDefault( "Script", OUString() );
    }

    Any SAL_CALL EventHolder::getByName( const OUString& _rName )
    {
        ScriptEventDescriptor aDescriptor( impl_getDescriptor_throw( _rName ) );

        Sequence< PropertyValue > aScriptDescriptor( 2 );
        aScriptDescriptor[0].Name = "EventType";
        aScriptDescriptor[0].Value <<= aDescriptor.ScriptType;
        aScriptDescriptor[1].Name = "Script";
        aScriptDescriptor[1].Value <<= aDescriptor.ScriptCode;

        return makeAny( aScriptDescriptor );
    }

    Sequence< OUString > SAL_CALL EventHolder::getElementNames(  )
    {
        Sequence< OUString > aReturn( m_aEventIndexAccess.size() );
        OUString* pReturn = aReturn.getArray();

        // SvxMacroAssignDlg has a weird API: It expects a XNameReplace, means a container whose
        // main access method is by name. In its UI, it shows the possible events in exactly the
        // order in which XNameAccess::getElementNames returns them.
        // However, SvxMacroAssignDlg *also* takes an index for the initial selection, which is
        // relative to the sequence returned by XNameAccess::getElementNames.
        // This is IMO weird, since it mixes index access with name access, which decreases efficiency
        // of the implementation.
        // Well, it means we're forced to return the events in getElementNames in exactly the same as they
        // appear in the property browser UI.
        for (auto const& elem : m_aEventIndexAccess)
        {
            *pReturn = elem.second->first;
            ++pReturn;
        }
        return aReturn;
    }

    sal_Bool SAL_CALL EventHolder::hasByName( const OUString& _rName )
    {
        EventMap::const_iterator pos = m_aEventNameAccess.find( _rName );
        return pos != m_aEventNameAccess.end();
    }

    Type SAL_CALL EventHolder::getElementType(  )
    {
        return cppu::UnoType<Sequence< PropertyValue >>::get();
    }

    sal_Bool SAL_CALL EventHolder::hasElements(  )
    {
        return !m_aEventNameAccess.empty();
    }


    EventHandler::EventHandler( const Reference< XComponentContext >& _rxContext )
        :EventHandler_Base( m_aMutex )
        ,m_xContext( _rxContext )
        ,m_aPropertyListeners( m_aMutex )
        ,m_bEventsMapInitialized( false )
        ,m_bIsDialogElement( false )
        ,m_nGridColumnType( -1 )
    {
    }

    EventHandler::~EventHandler()
    {
    }

    OUString SAL_CALL EventHandler::getImplementationName(  )
    {
        return getImplementationName_static();
    }

    sal_Bool SAL_CALL EventHandler::supportsService( const OUString& ServiceName )
    {
        return cppu::supportsService(this, ServiceName);
    }

    Sequence< OUString > SAL_CALL EventHandler::getSupportedServiceNames(  )
    {
        return getSupportedServiceNames_static();
    }

    OUString EventHandler::getImplementationName_static(  )
    {
        return OUString(  "com.sun.star.comp.extensions.EventHandler"  );
    }

    Sequence< OUString > EventHandler::getSupportedServiceNames_static(  )
    {
        Sequence<OUString> aSupported { "com.sun.star.form.inspection.EventHandler" };
        return aSupported;
    }

    Reference< XInterface > EventHandler::Create( const Reference< XComponentContext >& _rxContext )
    {
        return *( new EventHandler( _rxContext ) );
    }

    void SAL_CALL EventHandler::inspect( const Reference< XInterface >& _rxIntrospectee )
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        if ( !_rxIntrospectee.is() )
            throw NullPointerException();

        m_xComponent.set( _rxIntrospectee, UNO_QUERY_THROW );

        m_bEventsMapInitialized = false;
        EventMap aEmpty;
        m_aEvents.swap( aEmpty );

        m_bIsDialogElement = false;
        m_nGridColumnType = -1;
        try
        {
            Reference< XPropertySetInfo > xPSI( m_xComponent->getPropertySetInfo() );
            m_bIsDialogElement = xPSI.is()
                              && xPSI->hasPropertyByName( PROPERTY_WIDTH )
                              && xPSI->hasPropertyByName( PROPERTY_HEIGHT )
                              && xPSI->hasPropertyByName( PROPERTY_POSITIONX )
                              && xPSI->hasPropertyByName( PROPERTY_POSITIONY );

            Reference< XChild > xAsChild( _rxIntrospectee, UNO_QUERY );
            if ( xAsChild.is() && !Reference< XForm >( _rxIntrospectee, UNO_QUERY ).is() )
            {
                if ( FormComponentType::GRIDCONTROL == classifyComponent( xAsChild->getParent() ) )
                {
                    m_nGridColumnType = classifyComponent( _rxIntrospectee );
                }
            }
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION("extensions.propctrlr");
        }
    }

    Any SAL_CALL EventHandler::getPropertyValue( const OUString& _rPropertyName )
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        const EventDescription& rEvent = impl_getEventForName_throw( _rPropertyName );

        std::vector< ScriptEventDescriptor > aEvents;
        impl_getComponentScriptEvents_nothrow( aEvents );

        ScriptEventDescriptor aPropertyValue;
        for ( const ScriptEventDescriptor& rSCD : aEvents )
        {
            if  (   rEvent.sListenerClassName == rSCD.ListenerType
                &&  rEvent.sListenerMethodName == rSCD.EventMethod
                )
            {
                aPropertyValue = rSCD;
                break;
            }
        }

        return makeAny( aPropertyValue );
    }

    void SAL_CALL EventHandler::setPropertyValue( const OUString& _rPropertyName, const Any& _rValue )
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        const EventDescription& rEvent = impl_getEventForName_throw( _rPropertyName );

        ScriptEventDescriptor aNewScriptEvent;
        OSL_VERIFY( _rValue >>= aNewScriptEvent );

        ScriptEventDescriptor aOldScriptEvent;
        OSL_VERIFY( getPropertyValue( _rPropertyName ) >>= aOldScriptEvent );
        if ( aOldScriptEvent == aNewScriptEvent )
            return;

        if ( m_bIsDialogElement )
            impl_setDialogElementScriptEvent_nothrow( aNewScriptEvent );
        else
            impl_setFormComponentScriptEvent_nothrow( aNewScriptEvent );

        PropertyHandlerHelper::setContextDocumentModified( m_xContext );

        PropertyChangeEvent aEvent;
        aEvent.Source = m_xComponent;
        aEvent.PropertyHandle = rEvent.nId;
        aEvent.PropertyName = _rPropertyName;
        aEvent.OldValue <<= aOldScriptEvent;
        aEvent.NewValue <<= aNewScriptEvent;
        m_aPropertyListeners.notify( aEvent, &XPropertyChangeListener::propertyChange );
    }

    Any SAL_CALL EventHandler::convertToPropertyValue( const OUString& _rPropertyName, const Any& _rControlValue )
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        OUString sNewScriptCode;
        OSL_VERIFY( _rControlValue >>= sNewScriptCode );

        std::vector< ScriptEventDescriptor > aAllAssignedEvents;
        impl_getComponentScriptEvents_nothrow( aAllAssignedEvents );

        const EventDescription& rEvent = impl_getEventForName_throw( _rPropertyName );
        ScriptEventDescriptor aAssignedScript = lcl_getAssignedScriptEvent( rEvent, aAllAssignedEvents );

        OSL_ENSURE( sNewScriptCode.isEmpty(), "EventHandler::convertToPropertyValue: cannot convert a non-empty display name!" );
        // Usually, there is no possibility for the user to change the content of an event binding directly in the
        // input field, this instead is done with the macro assignment dialog.
        // The only exception is the user pressing "DEL" while the control has the focus, in this case, we reset the
        // control content to an empty string. So this is the only scenario where this method is allowed to be called.

        // Strictly, we would be able to convert the display value to a property value,
        // using the "name (location, language)" format we used in convertToControlValue. However,
        // there is no need for this code...

        aAssignedScript.ScriptCode = sNewScriptCode;
        return makeAny( aAssignedScript );
    }

    Any SAL_CALL EventHandler::convertToControlValue( const OUString& /*_rPropertyName*/, const Any& _rPropertyValue, const Type& _rControlValueType )
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        ScriptEventDescriptor aScriptEvent;
        OSL_VERIFY( _rPropertyValue >>= aScriptEvent );

        OSL_ENSURE( _rControlValueType.getTypeClass() == TypeClass_STRING,
            "EventHandler::convertToControlValue: unexpected ControlValue type class!" );

        OUString sScript( aScriptEvent.ScriptCode );
        if ( !sScript.isEmpty() )
        {
            // format is: "name (location, language)"
            try
            {
                // parse
                Reference< XUriReferenceFactory > xUriRefFac = UriReferenceFactory::create( m_xContext );
                Reference< XVndSunStarScriptUrlReference > xScriptUri( xUriRefFac->parse( sScript ), UNO_QUERY_THROW );

                OUStringBuffer aComposeBuffer;

                // name
                aComposeBuffer.append( xScriptUri->getName() );

                // location
                const OUString sLocationParamName(  "location"  );
                const OUString sLocation = xScriptUri->getParameter( sLocationParamName );
                const OUString sLangParamName(  "language"  );
                const OUString sLanguage = xScriptUri->getParameter( sLangParamName );

                if ( !(sLocation.isEmpty() && sLanguage.isEmpty()) )
                {
                    aComposeBuffer.append( " (" );

                    // location
                    OSL_ENSURE( !sLocation.isEmpty(), "EventHandler::convertToControlValue: unexpected: no location!" );
                    if ( !sLocation.isEmpty() )
                    {
                        aComposeBuffer.append( sLocation );
                        aComposeBuffer.append( ", " );
                    }

                    // language
                    if ( !sLanguage.isEmpty() )
                    {
                        aComposeBuffer.append( sLanguage );
                    }

                    aComposeBuffer.append( ')' );
                }

                sScript = aComposeBuffer.makeStringAndClear();
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION("extensions.propctrlr");
            }
        }

        return makeAny( sScript );
    }

    PropertyState SAL_CALL EventHandler::getPropertyState( const OUString& /*_rPropertyName*/ )
    {
        return PropertyState_DIRECT_VALUE;
    }

    void SAL_CALL EventHandler::addPropertyChangeListener( const Reference< XPropertyChangeListener >& _rxListener )
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        if ( !_rxListener.is() )
            throw NullPointerException();
        m_aPropertyListeners.addListener( _rxListener );
    }

    void SAL_CALL EventHandler::removePropertyChangeListener( const Reference< XPropertyChangeListener >& _rxListener )
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        m_aPropertyListeners.removeListener( _rxListener );
    }

    Sequence< Property > SAL_CALL EventHandler::getSupportedProperties()
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        if ( !m_bEventsMapInitialized )
        {
            m_bEventsMapInitialized = true;
            try
            {
                std::vector< Type > aListeners;
                impl_getComponentListenerTypes_nothrow( aListeners );

                OUString sListenerClassName;

                // loop through all listeners and all methods, and see which we can present at the UI
                for ( const Type& rListener : aListeners )
                {
                    // the programmatic name of the listener, to be used as "property" name
                    sListenerClassName = rListener.getTypeName();
                    OSL_ENSURE( !sListenerClassName.isEmpty(), "EventHandler::getSupportedProperties: strange - no listener name ..." );
                    if ( sListenerClassName.isEmpty() )
                        continue;

                    // loop through all methods
                    for (const OUString& rMethod : comphelper::getEventMethodsForType( rListener ))
                    {
                        EventDescription aEvent;
                        if ( !lcl_getEventDescriptionForMethod( rMethod, aEvent ) )
                            continue;

                        if ( !impl_filterMethod_nothrow( aEvent ) )
                            continue;

                        m_aEvents.emplace(
                            lcl_getEventPropertyName( sListenerClassName, rMethod ), aEvent );
                    }
                }

            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION("extensions.propctrlr");
            }
        }

        // sort them by ID - this is the relative ordering in the UI
        std::map< EventId, Property > aOrderedProperties;
        for (auto const& event : m_aEvents)
        {
            aOrderedProperties[ event.second.nId ] = Property(
                event.first, event.second.nId,
                ::cppu::UnoType<OUString>::get(),
                PropertyAttribute::BOUND );
        }

        return comphelper::mapValuesToSequence( aOrderedProperties );
    }

    Sequence< OUString > SAL_CALL EventHandler::getSupersededProperties( )
    {
        // none
        return Sequence< OUString >( );
    }

    Sequence< OUString > SAL_CALL EventHandler::getActuatingProperties( )
    {
        // none
        return Sequence< OUString >( );
    }

    LineDescriptor SAL_CALL EventHandler::describePropertyLine( const OUString& _rPropertyName,
        const Reference< XPropertyControlFactory >& _rxControlFactory )
    {
        if ( !_rxControlFactory.is() )
            throw NullPointerException();

        ::osl::MutexGuard aGuard( m_aMutex );

        LineDescriptor aDescriptor;

        aDescriptor.Control = _rxControlFactory->createPropertyControl( PropertyControlType::TextField, true );
        Reference< XEventListener > xControlExtender = new PropertyControlExtender( aDescriptor.Control );

        const EventDescription& rEvent = impl_getEventForName_throw( _rPropertyName );
        aDescriptor.DisplayName = rEvent.sDisplayName;
        aDescriptor.HelpURL = HelpIdUrl::getHelpURL( rEvent.sHelpId );
        aDescriptor.PrimaryButtonId = OStringToOUString(rEvent.sUniqueBrowseId, RTL_TEXTENCODING_UTF8);
        aDescriptor.HasPrimaryButton = true;
        aDescriptor.Category = "Events";
        return aDescriptor;
    }

    sal_Bool SAL_CALL EventHandler::isComposable( const OUString& /*_rPropertyName*/ )
    {
        return false;
    }

    InteractiveSelectionResult SAL_CALL EventHandler::onInteractivePropertySelection( const OUString& _rPropertyName, sal_Bool /*_bPrimary*/, Any& /*_rData*/, const Reference< XObjectInspectorUI >& _rxInspectorUI )
    {
        if ( !_rxInspectorUI.is() )
            throw NullPointerException();

        ::osl::MutexGuard aGuard( m_aMutex );
        const EventDescription& rForEvent = impl_getEventForName_throw( _rPropertyName );

        std::vector< ScriptEventDescriptor > aAllAssignedEvents;
        impl_getComponentScriptEvents_nothrow( aAllAssignedEvents );

        // SvxMacroAssignDlg-compatible structure holding all event/assignments
        ::rtl::Reference< EventHolder >  pEventHolder( new EventHolder );

        for (auto const& event : m_aEvents)
        {
            // the script which is assigned to the current event (if any)
            ScriptEventDescriptor aAssignedScript = lcl_getAssignedScriptEvent( event.second, aAllAssignedEvents );
            pEventHolder->addEvent( event.second.nId, event.second.sListenerMethodName, aAssignedScript );
        }

        // the initial selection in the dialog
        Sequence< OUString > aNames( pEventHolder->getElementNames() );
        const OUString* pChosenEvent = std::find( aNames.begin(), aNames.end(), rForEvent.sListenerMethodName );
        sal_uInt16 nInitialSelection = static_cast<sal_uInt16>( pChosenEvent - aNames.begin() );

        // the dialog
        SvxAbstractDialogFactory* pFactory = SvxAbstractDialogFactory::Create();

        ScopedVclPtr<VclAbstractDialog> pDialog( pFactory->CreateSvxMacroAssignDlg(
            PropertyHandlerHelper::getDialogParentFrame( m_xContext ),
            impl_getContextFrame_nothrow(),
            m_bIsDialogElement,
            pEventHolder.get(),
            nInitialSelection
        ) );

        if ( !pDialog.get() )
            return InteractiveSelectionResult_Cancelled;

        // DF definite problem here
        // OK & Cancel seem to be both returning 0
        if ( pDialog->Execute() == RET_CANCEL )
            return InteractiveSelectionResult_Cancelled;

        try
        {
            for (auto const& event : m_aEvents)
            {
                ScriptEventDescriptor aScriptDescriptor( pEventHolder->getNormalizedDescriptorByName( event.second.sListenerMethodName ) );

                // set the new "property value"
                setPropertyValue(
                    lcl_getEventPropertyName( event.second.sListenerClassName, event.second.sListenerMethodName ),
                    makeAny( aScriptDescriptor )
                );
            }
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION("extensions.propctrlr");
        }

        return InteractiveSelectionResult_Success;
    }

    void SAL_CALL EventHandler::actuatingPropertyChanged( const OUString& /*_rActuatingPropertyName*/, const Any& /*_rNewValue*/, const Any& /*_rOldValue*/, const Reference< XObjectInspectorUI >& /*_rxInspectorUI*/, sal_Bool /*_bFirstTimeInit*/ )
    {
        OSL_FAIL( "EventHandler::actuatingPropertyChanged: no actuating properties -> no callback (well, this is how it *should* be!)" );
    }

    IMPLEMENT_FORWARD_XCOMPONENT( EventHandler, EventHandler_Base )

    void SAL_CALL EventHandler::disposing()
    {
        EventMap aEmpty;
        m_aEvents.swap( aEmpty );
        m_xComponent.clear();
    }

    sal_Bool SAL_CALL EventHandler::suspend( sal_Bool /*_bSuspend*/ )
    {
        return true;
    }

    Reference< XFrame > EventHandler::impl_getContextFrame_nothrow() const
    {
        Reference< XFrame > xContextFrame;

        try
        {
            Reference< XModel > xContextDocument( PropertyHandlerHelper::getContextDocument(m_xContext), UNO_QUERY_THROW );
            Reference< XController > xController( xContextDocument->getCurrentController(), UNO_SET_THROW );
            xContextFrame.set( xController->getFrame(), UNO_SET_THROW );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION("extensions.propctrlr");
        }

        return xContextFrame;
    }

    sal_Int32 EventHandler::impl_getComponentIndexInParent_throw() const
    {
        Reference< XChild > xChild( m_xComponent, UNO_QUERY_THROW );
        Reference< XIndexAccess > xParentAsIndexAccess( xChild->getParent(), UNO_QUERY_THROW );

        // get the index of the inspected object within its parent container
        sal_Int32 nElements = xParentAsIndexAccess->getCount();
        for ( sal_Int32 i=0; i<nElements; ++i )
        {
            Reference< XInterface > xElement( xParentAsIndexAccess->getByIndex( i ), UNO_QUERY_THROW );
            if ( xElement == m_xComponent )
                return i;
        }
        throw NoSuchElementException();
    }

    void EventHandler::impl_getFormComponentScriptEvents_nothrow( std::vector < ScriptEventDescriptor >& _out_rEvents ) const
    {
        _out_rEvents.clear();
        try
        {
            Reference< XChild > xChild( m_xComponent, UNO_QUERY_THROW );
            Reference< XEventAttacherManager > xEventManager( xChild->getParent(), UNO_QUERY_THROW );
            comphelper::sequenceToContainer(_out_rEvents, xEventManager->getScriptEvents( impl_getComponentIndexInParent_throw() ));

            // the form component script API has unqualified listener names, but for normalization
            // purpose, we want fully qualified ones
            for ( ScriptEventDescriptor& rSED :  _out_rEvents)
            {
                rSED.ListenerType = lcl_getQualifiedKnownListenerName( rSED );
            }
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION("extensions.propctrlr");
        }
    }

    void EventHandler::impl_getComponentListenerTypes_nothrow( std::vector< Type >& _out_rTypes ) const
    {
        _out_rTypes.clear();
        try
        {
            // we use a set to avoid duplicates
            TypeBag aListeners;

            Reference< XIntrospection > xIntrospection = theIntrospection::get( m_xContext );

            // --- model listeners
            lcl_addListenerTypesFor_throw(
                m_xComponent, xIntrospection, aListeners );

            // --- "secondary component" (usually: "control" listeners)
            {
                Reference< XInterface > xSecondaryComponent( impl_getSecondaryComponentForEventInspection_throw() );
                lcl_addListenerTypesFor_throw( xSecondaryComponent, xIntrospection, aListeners );
                ::comphelper::disposeComponent( xSecondaryComponent );
            }

            // now that they're disambiguated, copy these types into our member
            std::copy(aListeners.begin(), aListeners.end(), std::back_inserter(_out_rTypes));
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION("extensions.propctrlr");
        }
    }

    void EventHandler::impl_getDialogElementScriptEvents_nothrow( std::vector < ScriptEventDescriptor >& _out_rEvents ) const
    {
        _out_rEvents.clear();
        try
        {
            Reference< XScriptEventsSupplier > xEventsSupplier( m_xComponent, UNO_QUERY_THROW );
            Reference< XNameContainer > xEvents( xEventsSupplier->getEvents(), UNO_QUERY_THROW );
            Sequence< OUString > aEventNames( xEvents->getElementNames() );

            sal_Int32 nEventCount = aEventNames.getLength();
            _out_rEvents.resize( nEventCount );

            for( sal_Int32 i = 0; i < nEventCount; ++i )
                OSL_VERIFY( xEvents->getByName( aEventNames[i] ) >>= _out_rEvents[i] );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION("extensions.propctrlr");
        }
    }

    Reference< XInterface > EventHandler::impl_getSecondaryComponentForEventInspection_throw( ) const
    {
        Reference< XInterface > xReturn;

        // if it's a form, create a form controller for the additional events
        Reference< XForm > xComponentAsForm( m_xComponent, UNO_QUERY );
        if ( xComponentAsForm.is() )
        {
            Reference< XTabControllerModel > xComponentAsTCModel( m_xComponent, UNO_QUERY_THROW );
            Reference< XFormController > xController = FormController::create( m_xContext );
            xController->setModel( xComponentAsTCModel );

            xReturn = xController;
        }
        else
        {
            OUString sControlService;
            OSL_VERIFY( m_xComponent->getPropertyValue( PROPERTY_DEFAULTCONTROL ) >>= sControlService );

            xReturn = m_xContext->getServiceManager()->createInstanceWithContext( sControlService, m_xContext );
        }
        return xReturn;
    }

    const EventDescription& EventHandler::impl_getEventForName_throw( const OUString& _rPropertyName ) const
    {
        EventMap::const_iterator pos = m_aEvents.find( _rPropertyName );
        if ( pos == m_aEvents.end() )
            throw UnknownPropertyException();
        return pos->second;
    }

    namespace
    {
        bool lcl_endsWith( const OUString& _rText, const OUString& _rCheck )
        {
            sal_Int32 nTextLen = _rText.getLength();
            sal_Int32 nCheckLen = _rCheck.getLength();
            if ( nCheckLen > nTextLen )
                return false;

            return _rText.indexOf( _rCheck ) == ( nTextLen - nCheckLen );
        }
    }

    void EventHandler::impl_setFormComponentScriptEvent_nothrow( const ScriptEventDescriptor& _rScriptEvent )
    {
        try
        {
            OUString sScriptCode( _rScriptEvent.ScriptCode );
            OUString sScriptType( _rScriptEvent.ScriptType );
            bool bResetScript = sScriptCode.isEmpty();

            sal_Int32 nObjectIndex = impl_getComponentIndexInParent_throw();
            Reference< XChild > xChild( m_xComponent, UNO_QUERY_THROW );
            Reference< XEventAttacherManager > xEventManager( xChild->getParent(), UNO_QUERY_THROW );
            std::vector< ScriptEventDescriptor > aEvents;
            comphelper::sequenceToContainer( aEvents, xEventManager->getScriptEvents( nObjectIndex ) );

            // is there already a registered script for this event?
            sal_Int32 eventCount = aEvents.size(), event = 0;
            for ( event = 0; event < eventCount; ++event )
            {
                ScriptEventDescriptor* pEvent = &aEvents[event];
                if  (   ( pEvent->EventMethod == _rScriptEvent.EventMethod )
                    &&  ( lcl_endsWith( _rScriptEvent.ListenerType, pEvent->ListenerType ) )
                          // (strange enough, the events we get from getScriptEvents are not fully qualified)
                    )
                {
                    // yes
                    if ( !bResetScript )
                    {
                        // set to something non-empty -> overwrite
                        pEvent->ScriptCode = sScriptCode;
                        pEvent->ScriptType = sScriptType;
                    }
                    else
                    {
                        // set to empty -> remove from vector
                        aEvents.erase(aEvents.begin() + event );
                        --eventCount;
                    }
                    break;
                }
            }
            if ( ( event >= eventCount ) && !bResetScript )
            {
                // no, did not find it -> append
                aEvents.push_back( _rScriptEvent );
            }

            xEventManager->revokeScriptEvents( nObjectIndex );
            xEventManager->registerScriptEvents( nObjectIndex, comphelper::containerToSequence(aEvents) );

            PropertyHandlerHelper::setContextDocumentModified( m_xContext );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION("extensions.propctrlr");
        }
    }

    void EventHandler::impl_setDialogElementScriptEvent_nothrow( const ScriptEventDescriptor& _rScriptEvent )
    {
        try
        {
            OUString sScriptCode( _rScriptEvent.ScriptCode );
            bool bResetScript =  sScriptCode.isEmpty();

            Reference< XScriptEventsSupplier > xEventsSupplier( m_xComponent, UNO_QUERY_THROW );
            Reference< XNameContainer > xEvents( xEventsSupplier->getEvents(), UNO_QUERY_THROW );

            OUStringBuffer aCompleteName;
            aCompleteName.append( _rScriptEvent.ListenerType );
            aCompleteName.append( "::" );
            aCompleteName.append( _rScriptEvent.EventMethod );
            OUString sCompleteName( aCompleteName.makeStringAndClear() );

            bool bExists = xEvents->hasByName( sCompleteName );

            if ( bResetScript )
            {
                if ( bExists )
                    xEvents->removeByName( sCompleteName );
            }
            else
            {
                Any aNewValue; aNewValue <<= _rScriptEvent;

                if ( bExists )
                    xEvents->replaceByName( sCompleteName, aNewValue );
                else
                    xEvents->insertByName( sCompleteName, aNewValue );
            }
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION("extensions.propctrlr");
        }
    }

    bool EventHandler::impl_filterMethod_nothrow( const EventDescription& _rEvent ) const
    {
        // some (control-triggered) events do not make sense for certain grid control columns. However,
        // our mechanism to retrieve control-triggered events does not know about this, so we do some
        // late filtering here.
        switch ( m_nGridColumnType )
        {
        case FormComponentType::COMBOBOX:
            if ( UID_BRWEVT_ACTIONPERFORMED == _rEvent.sUniqueBrowseId )
                return false;
            break;
        case FormComponentType::LISTBOX:
            if  (   ( UID_BRWEVT_CHANGED == _rEvent.sUniqueBrowseId )
                ||  ( UID_BRWEVT_ACTIONPERFORMED == _rEvent.sUniqueBrowseId )
                )
                return false;
            break;
        }

        return true;
    }

} // namespace pcr

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
