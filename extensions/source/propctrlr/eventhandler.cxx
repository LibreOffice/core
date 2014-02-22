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
#include "propctrlr.hrc"
#include "formbrowsertools.hxx"
#include "formresid.hrc"
#include "formstrings.hxx"
#include "handlerhelper.hxx"
#include "modulepcr.hxx"
#include "pcrcommon.hxx"
#include "pcrstrings.hxx"
#include "propertycontrolextender.hxx"

#include <com/sun/star/awt/XTabControllerModel.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/UnknownPropertyException.hpp>
#include <com/sun/star/beans/Introspection.hpp>
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
#include <com/sun/star/util/XModifiable.hpp>
#include <com/sun/star/uri/UriReferenceFactory.hpp>
#include <com/sun/star/uri/XVndSunStarScriptUrlReference.hpp>

#include <comphelper/namedvaluecollection.hxx>
#include <comphelper/evtmethodhelper.hxx>
#include <comphelper/types.hxx>
#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <rtl/ref.hxx>
#include <rtl/ustrbuf.hxx>
#include <sfx2/app.hxx>
#include <svl/eitem.hxx>
#include <svl/itemset.hxx>
#include <svx/svxdlg.hxx>
#include <svx/svxids.hrc>
#include <tools/diagnose_ex.h>
#include <vcl/msgbox.hxx>

#include <map>
#include <algorithm>
#include <o3tl/compat_functional.hxx>

extern "C" void SAL_CALL createRegistryInfo_EventHandler()
{
    ::pcr::OAutoRegistration< ::pcr::EventHandler > aAutoRegistration;
}

namespace pcr
{

    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::XComponentContext;
    using ::com::sun::star::beans::XPropertySet;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::TypeClass_STRING;
    using ::com::sun::star::uno::Type;
    using ::com::sun::star::beans::Introspection;
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
    using ::com::sun::star::util::XModifiable;
    using ::com::sun::star::beans::PropertyChangeEvent;
    using ::com::sun::star::frame::XFrame;
    using ::com::sun::star::frame::XModel;
    using ::com::sun::star::frame::XController;
    using ::com::sun::star::uno::UNO_SET_THROW;
    using com::sun::star::uri::UriReferenceFactory;
    using com::sun::star::uri::XUriReferenceFactory;
    using com::sun::star::uri::XVndSunStarScriptUrlReference;
    using ::com::sun::star::lang::XEventListener;

    namespace PropertyControlType = ::com::sun::star::inspection::PropertyControlType;
    namespace PropertyAttribute = ::com::sun::star::beans::PropertyAttribute;
    namespace FormComponentType = ::com::sun::star::form::FormComponentType;

    EventDescription::EventDescription( EventId _nId, const sal_Char* _pListenerNamespaceAscii, const sal_Char* _pListenerClassAsciiName,
            const sal_Char* _pListenerMethodAsciiName, sal_uInt16 _nDisplayNameResId, const OString& _sHelpId, const OString& _sUniqueBrowseId )
        :sDisplayName(PcrRes( _nDisplayNameResId ).toString())
        ,sListenerMethodName( OUString::createFromAscii( _pListenerMethodAsciiName ) )
        ,sHelpId( _sHelpId )
        ,sUniqueBrowseId( _sUniqueBrowseId )
        ,nId( _nId )
    {
        OUStringBuffer aQualifiedListenerClass;
        aQualifiedListenerClass.appendAscii( "com.sun.star." );
        aQualifiedListenerClass.appendAscii( _pListenerNamespaceAscii );
        aQualifiedListenerClass.appendAscii( "." );
        aQualifiedListenerClass.appendAscii( _pListenerClassAsciiName );
        sListenerClassName = aQualifiedListenerClass.makeStringAndClear();
    }

    namespace
    {
        #define DESCRIBE_EVENT( asciinamespace, asciilistener, asciimethod, id_postfix ) \
            s_aKnownEvents.insert( EventMap::value_type( \
                OUString::createFromAscii( asciimethod ), \
                EventDescription( ++nEventId, asciinamespace, asciilistener, asciimethod, RID_STR_EVT_##id_postfix, HID_EVT_##id_postfix, UID_BRWEVT_##id_postfix ) ) )

        bool lcl_getEventDescriptionForMethod( const OUString& _rMethodName, EventDescription& _out_rDescription )
        {
            static EventMap s_aKnownEvents;
            if ( s_aKnownEvents.empty() )
            {
                ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
                if ( s_aKnownEvents.empty() )
                {
                    static sal_Int32 nEventId = 0;

                    DESCRIBE_EVENT( "form", "XApproveActionListener",       "approveAction",            APPROVEACTIONPERFORMED );
                    DESCRIBE_EVENT( "awt",  "XActionListener",              "actionPerformed",          ACTIONPERFORMED );
                    DESCRIBE_EVENT( "form", "XChangeListener",              "changed",                  CHANGED );
                    DESCRIBE_EVENT( "awt",  "XTextListener",                "textChanged",              TEXTCHANGED );
                    DESCRIBE_EVENT( "awt",  "XItemListener",                "itemStateChanged",         ITEMSTATECHANGED );
                    DESCRIBE_EVENT( "awt",  "XFocusListener",               "focusGained",              FOCUSGAINED );
                    DESCRIBE_EVENT( "awt",  "XFocusListener",               "focusLost",                FOCUSLOST );
                    DESCRIBE_EVENT( "awt",  "XKeyListener",                 "keyPressed",               KEYTYPED );
                    DESCRIBE_EVENT( "awt",  "XKeyListener",                 "keyReleased",              KEYUP );
                    DESCRIBE_EVENT( "awt",  "XMouseListener",               "mouseEntered",             MOUSEENTERED );
                    DESCRIBE_EVENT( "awt",  "XMouseMotionListener",         "mouseDragged",             MOUSEDRAGGED );
                    DESCRIBE_EVENT( "awt",  "XMouseMotionListener",         "mouseMoved",               MOUSEMOVED );
                    DESCRIBE_EVENT( "awt",  "XMouseListener",               "mousePressed",             MOUSEPRESSED );
                    DESCRIBE_EVENT( "awt",  "XMouseListener",               "mouseReleased",            MOUSERELEASED );
                    DESCRIBE_EVENT( "awt",  "XMouseListener",               "mouseExited",              MOUSEEXITED );
                    DESCRIBE_EVENT( "form", "XResetListener",               "approveReset",             APPROVERESETTED );
                    DESCRIBE_EVENT( "form", "XResetListener",               "resetted",                 RESETTED );
                    DESCRIBE_EVENT( "form", "XSubmitListener",              "approveSubmit",            SUBMITTED );
                    DESCRIBE_EVENT( "form", "XUpdateListener",              "approveUpdate",            BEFOREUPDATE );
                    DESCRIBE_EVENT( "form", "XUpdateListener",              "updated",                  AFTERUPDATE );
                    DESCRIBE_EVENT( "form", "XLoadListener",                "loaded",                   LOADED );
                    DESCRIBE_EVENT( "form", "XLoadListener",                "reloading",                RELOADING );
                    DESCRIBE_EVENT( "form", "XLoadListener",                "reloaded",                 RELOADED );
                    DESCRIBE_EVENT( "form", "XLoadListener",                "unloading",                UNLOADING );
                    DESCRIBE_EVENT( "form", "XLoadListener",                "unloaded",                 UNLOADED );
                    DESCRIBE_EVENT( "form", "XConfirmDeleteListener",       "confirmDelete",            CONFIRMDELETE );
                    DESCRIBE_EVENT( "sdb",  "XRowSetApproveListener",       "approveRowChange",         APPROVEROWCHANGE );
                    DESCRIBE_EVENT( "sdbc", "XRowSetListener",              "rowChanged",               ROWCHANGE );
                    DESCRIBE_EVENT( "sdb",  "XRowSetApproveListener",       "approveCursorMove",        POSITIONING );
                    DESCRIBE_EVENT( "sdbc", "XRowSetListener",              "cursorMoved",              POSITIONED );
                    DESCRIBE_EVENT( "form", "XDatabaseParameterListener",   "approveParameter",         APPROVEPARAMETER );
                    DESCRIBE_EVENT( "sdb",  "XSQLErrorListener",            "errorOccured",             ERROROCCURRED );
                    DESCRIBE_EVENT( "awt",  "XAdjustmentListener",          "adjustmentValueChanged",   ADJUSTMENTVALUECHANGED );
                }
            }

            EventMap::const_iterator pos = s_aKnownEvents.find( _rMethodName );
            if ( pos == s_aKnownEvents.end() )
                return false;

            _out_rDescription = pos->second;
            return true;
        }

        OUString lcl_getEventPropertyName( const OUString& _rListenerClassName, const OUString& _rMethodName )
        {
            OUStringBuffer aPropertyName;
            aPropertyName.append( _rListenerClassName );
            aPropertyName.append( ';' );
            aPropertyName.append( _rMethodName.getStr() );
            return aPropertyName.makeStringAndClear();
        }

        ScriptEventDescriptor lcl_getAssignedScriptEvent( const EventDescription& _rEvent, const Sequence< ScriptEventDescriptor >& _rAllAssignedMacros )
        {
            ScriptEventDescriptor aScriptEvent;
            
            
            aScriptEvent.ListenerType = _rEvent.sListenerClassName;
            aScriptEvent.EventMethod = _rEvent.sListenerMethodName;

            const ScriptEventDescriptor* pAssignedEvent = _rAllAssignedMacros.getConstArray();
            sal_Int32 assignedEventCount( _rAllAssignedMacros.getLength() );
            for ( sal_Int32 assignedEvent = 0; assignedEvent < assignedEventCount; ++assignedEvent, ++pAssignedEvent )
            {
                if  (   ( pAssignedEvent->ListenerType != _rEvent.sListenerClassName )
                    ||  ( pAssignedEvent->EventMethod != _rEvent.sListenerMethodName )
                    )
                    continue;

                if  (   ( pAssignedEvent->ScriptCode.isEmpty() )
                    ||  ( pAssignedEvent->ScriptType.isEmpty() )
                    )
                {
                    OSL_FAIL( "lcl_getAssignedScriptEvent: me thinks this should not happen!" );
                    continue;
                }

                aScriptEvent = *pAssignedEvent;

                if ( aScriptEvent.ScriptType != "StarBasic" )
                    continue;

                
                
                
                

                sal_Int32 nPrefixLen = aScriptEvent.ScriptCode.indexOf( ':' );
                OSL_ENSURE( nPrefixLen > 0, "lcl_getAssignedScriptEvent: illegal location!" );
                OUString sLocation = aScriptEvent.ScriptCode.copy( 0, nPrefixLen );
                OUString sMacroPath = aScriptEvent.ScriptCode.copy( nPrefixLen + 1 );

                OUStringBuffer aNewStyleSpec;
                aNewStyleSpec.appendAscii( "vnd.sun.star.script:" );
                aNewStyleSpec.append     ( sMacroPath );
                aNewStyleSpec.appendAscii( "?language=Basic&location=" );
                aNewStyleSpec.append     ( sLocation );

                aScriptEvent.ScriptCode = aNewStyleSpec.makeStringAndClear();

                
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
                
                
                
                
            return _rFormComponentEventDescriptor.ListenerType;
        }

        typedef ::std::set< Type, TypeLessByName > TypeBag;

        void lcl_addListenerTypesFor_throw( const Reference< XInterface >& _rxComponent,
            const Reference< XIntrospection >& _rxIntrospection, TypeBag& _out_rTypes )
        {
            if ( !_rxComponent.is() )
                return;
            OSL_PRECOND( _rxIntrospection.is(), "lcl_addListenerTypesFor_throw: this will crash!" );

            Reference< XIntrospectionAccess > xIntrospectionAccess(
                _rxIntrospection->inspect( makeAny( _rxComponent ) ), UNO_QUERY_THROW );

            Sequence< Type > aListeners( xIntrospectionAccess->getSupportedListeners() );

            ::std::copy( aListeners.getConstArray(), aListeners.getConstArray() + aListeners.getLength(),
                ::std::insert_iterator< TypeBag >( _out_rTypes, _out_rTypes.begin() ) );
        }

        bool operator ==( const ScriptEventDescriptor _lhs, const ScriptEventDescriptor _rhs )
        {
            return  (   ( _lhs.ListenerType         == _rhs.ListenerType        )
                    &&  ( _lhs.EventMethod          == _rhs.EventMethod         )
                    &&  ( _lhs.AddListenerParam     == _rhs.AddListenerParam    )
                    &&  ( _lhs.ScriptType           == _rhs.ScriptType          )
                    &&  ( _lhs.ScriptCode           == _rhs.ScriptCode          )
                    );
        }
    }

    typedef ::cppu::WeakImplHelper1 <   ::com::sun::star::container::XNameReplace
                                    >   EventHolder_Base;
    /* An UNO component holding assigned event descriptions, for use with a SvxMacroAssignDlg */
    class EventHolder : public EventHolder_Base
    {
    private:
        typedef ::boost::unordered_map< OUString, ScriptEventDescriptor, OUStringHash >  EventMap;
        typedef ::std::map< EventId, EventMap::iterator >                                       EventMapIndexAccess;

        EventMap            m_aEventNameAccess;
        EventMapIndexAccess m_aEventIndexAccess;

    public:
        EventHolder( );

        void addEvent( EventId _nId, const OUString& _rEventName, const ScriptEventDescriptor& _rScriptEvent );

        /** effectively the same as getByName, but instead of converting the ScriptEventDescriptor to the weird
            format used by the macro assignment dialog, it is returned directly
        */
        ScriptEventDescriptor getNormalizedDescriptorByName( const OUString& _rEventName ) const;

        
        virtual void SAL_CALL replaceByName( const OUString& _rName, const Any& aElement ) throw (IllegalArgumentException, NoSuchElementException, WrappedTargetException, RuntimeException);
        virtual Any SAL_CALL getByName( const OUString& _rName ) throw (NoSuchElementException, WrappedTargetException, RuntimeException);
        virtual Sequence< OUString > SAL_CALL getElementNames(  ) throw (RuntimeException);
        virtual ::sal_Bool SAL_CALL hasByName( const OUString& _rName ) throw (RuntimeException);
        virtual Type SAL_CALL getElementType(  ) throw (RuntimeException);
        virtual ::sal_Bool SAL_CALL hasElements(  ) throw (RuntimeException);

    protected:
        ~EventHolder( );

    private:
        ScriptEventDescriptor impl_getDescriptor_throw( const OUString& _rEventName ) const;
    };

    DBG_NAME( EventHolder )

    EventHolder::EventHolder()
    {
        DBG_CTOR( EventHolder, NULL );
    }

    EventHolder::~EventHolder()
    {
        m_aEventNameAccess.clear();
        m_aEventIndexAccess.clear();
        DBG_DTOR( EventHolder, NULL );
    }

    void EventHolder::addEvent( EventId _nId, const OUString& _rEventName, const ScriptEventDescriptor& _rScriptEvent )
    {
        ::std::pair< EventMap::iterator, bool > insertionResult =
            m_aEventNameAccess.insert( EventMap::value_type( _rEventName, _rScriptEvent ) );
        OSL_ENSURE( insertionResult.second, "EventHolder::addEvent: there already was a MacroURL for this event!" );
        m_aEventIndexAccess[ _nId ] = insertionResult.first;
    }

    ScriptEventDescriptor EventHolder::getNormalizedDescriptorByName( const OUString& _rEventName ) const
    {
        return impl_getDescriptor_throw( _rEventName );
    }

    ScriptEventDescriptor EventHolder::impl_getDescriptor_throw( const OUString& _rEventName ) const
    {
        EventMap::const_iterator pos = m_aEventNameAccess.find( _rEventName );
        if ( pos == m_aEventNameAccess.end() )
            throw NoSuchElementException( OUString(), *const_cast< EventHolder* >( this ) );
        return pos->second;
    }

    void SAL_CALL EventHolder::replaceByName( const OUString& _rName, const Any& _rElement ) throw (IllegalArgumentException, NoSuchElementException, WrappedTargetException, RuntimeException)
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

    Any SAL_CALL EventHolder::getByName( const OUString& _rName ) throw (NoSuchElementException, WrappedTargetException, RuntimeException)
    {
        ScriptEventDescriptor aDescriptor( impl_getDescriptor_throw( _rName ) );

        Sequence< PropertyValue > aScriptDescriptor( 2 );
        aScriptDescriptor[0].Name = "EventType";
        aScriptDescriptor[0].Value <<= aDescriptor.ScriptType;
        aScriptDescriptor[1].Name = "Script";
        aScriptDescriptor[1].Value <<= aDescriptor.ScriptCode;

        return makeAny( aScriptDescriptor );
    }

    Sequence< OUString > SAL_CALL EventHolder::getElementNames(  ) throw (RuntimeException)
    {
        Sequence< OUString > aReturn( m_aEventIndexAccess.size() );
        OUString* pReturn = aReturn.getArray();

        
        
        
        
        
        
        
        
        
        for (   EventMapIndexAccess::const_iterator loop = m_aEventIndexAccess.begin();
                loop != m_aEventIndexAccess.end();
                ++loop, ++pReturn
            )
            *pReturn = loop->second->first;
        return aReturn;
    }

    sal_Bool SAL_CALL EventHolder::hasByName( const OUString& _rName ) throw (RuntimeException)
    {
        EventMap::const_iterator pos = m_aEventNameAccess.find( _rName );
        return pos != m_aEventNameAccess.end();
    }

    Type SAL_CALL EventHolder::getElementType(  ) throw (RuntimeException)
    {
        return ::getCppuType( static_cast< Sequence< PropertyValue >* >( NULL ) );
    }

    sal_Bool SAL_CALL EventHolder::hasElements(  ) throw (RuntimeException)
    {
        return !m_aEventNameAccess.empty();
    }

    DBG_NAME( EventHandler )

    EventHandler::EventHandler( const Reference< XComponentContext >& _rxContext )
        :EventHandler_Base( m_aMutex )
        ,m_xContext( _rxContext )
        ,m_aPropertyListeners( m_aMutex )
        ,m_bEventsMapInitialized( false )
        ,m_bIsDialogElement( false )
        ,m_nGridColumnType( -1 )
    {
        DBG_CTOR( EventHandler, NULL );
    }

    EventHandler::~EventHandler()
    {
        DBG_DTOR( EventHandler, NULL );
    }

    OUString SAL_CALL EventHandler::getImplementationName(  ) throw (RuntimeException)
    {
        return getImplementationName_static();
    }

    ::sal_Bool SAL_CALL EventHandler::supportsService( const OUString& ServiceName ) throw (RuntimeException)
    {
        return cppu::supportsService(this, ServiceName);
    }

    Sequence< OUString > SAL_CALL EventHandler::getSupportedServiceNames(  ) throw (RuntimeException)
    {
        return getSupportedServiceNames_static();
    }

    OUString SAL_CALL EventHandler::getImplementationName_static(  ) throw (RuntimeException)
    {
        return OUString(  "com.sun.star.comp.extensions.EventHandler"  );
    }

    Sequence< OUString > SAL_CALL EventHandler::getSupportedServiceNames_static(  ) throw (RuntimeException)
    {
        Sequence< OUString > aSupported( 1 );
        aSupported[0] = "com.sun.star.form.inspection.EventHandler";
        return aSupported;
    }

    Reference< XInterface > SAL_CALL EventHandler::Create( const Reference< XComponentContext >& _rxContext )
    {
        return *( new EventHandler( _rxContext ) );
    }

    void SAL_CALL EventHandler::inspect( const Reference< XInterface >& _rxIntrospectee ) throw (RuntimeException, NullPointerException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        if ( !_rxIntrospectee.is() )
            throw NullPointerException();

        m_xComponent = Reference< XPropertySet >( _rxIntrospectee, UNO_QUERY_THROW );

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
            DBG_UNHANDLED_EXCEPTION();
        }
    }

    Any SAL_CALL EventHandler::getPropertyValue( const OUString& _rPropertyName ) throw (UnknownPropertyException, RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        const EventDescription& rEvent = impl_getEventForName_throw( _rPropertyName );

        Sequence< ScriptEventDescriptor > aEvents;
        impl_getComponentScriptEvents_nothrow( aEvents );

        sal_Int32 nEventCount = aEvents.getLength();
        const ScriptEventDescriptor* pEvents = aEvents.getConstArray();

        ScriptEventDescriptor aPropertyValue;
        for ( sal_Int32 event = 0; event < nEventCount; ++event, ++pEvents )
        {
            if  (   rEvent.sListenerClassName == pEvents->ListenerType
                &&  rEvent.sListenerMethodName == pEvents->EventMethod
                )
            {
                aPropertyValue = *pEvents;
                break;
            }
        }

        return makeAny( aPropertyValue );
    }

    void SAL_CALL EventHandler::setPropertyValue( const OUString& _rPropertyName, const Any& _rValue ) throw (UnknownPropertyException, RuntimeException)
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

    Any SAL_CALL EventHandler::convertToPropertyValue( const OUString& _rPropertyName, const Any& _rControlValue ) throw (UnknownPropertyException, RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        OUString sNewScriptCode;
        OSL_VERIFY( _rControlValue >>= sNewScriptCode );

        Sequence< ScriptEventDescriptor > aAllAssignedEvents;
        impl_getComponentScriptEvents_nothrow( aAllAssignedEvents );

        const EventDescription& rEvent = impl_getEventForName_throw( _rPropertyName );
        ScriptEventDescriptor aAssignedScript = lcl_getAssignedScriptEvent( rEvent, aAllAssignedEvents );

        OSL_ENSURE( sNewScriptCode.isEmpty(), "EventHandler::convertToPropertyValue: cannot convert a non-empty display name!" );
        
        
        
        

        
        
        

        aAssignedScript.ScriptCode = sNewScriptCode;
        return makeAny( aAssignedScript );
    }

    Any SAL_CALL EventHandler::convertToControlValue( const OUString& /*_rPropertyName*/, const Any& _rPropertyValue, const Type& _rControlValueType ) throw (UnknownPropertyException, RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        ScriptEventDescriptor aScriptEvent;
        OSL_VERIFY( _rPropertyValue >>= aScriptEvent );

        OSL_ENSURE( _rControlValueType.getTypeClass() == TypeClass_STRING,
            "EventHandler::convertToControlValue: unexpected ControlValue type class!" );
        (void)_rControlValueType;

        OUString sScript( aScriptEvent.ScriptCode );
        if ( !sScript.isEmpty() )
        {
            
            try
            {
                
                Reference< XUriReferenceFactory > xUriRefFac = UriReferenceFactory::create( m_xContext );
                Reference< XVndSunStarScriptUrlReference > xScriptUri( xUriRefFac->parse( sScript ), UNO_QUERY_THROW );

                OUStringBuffer aComposeBuffer;

                
                aComposeBuffer.append( xScriptUri->getName() );

                
                const OUString sLocationParamName(  "location"  );
                const OUString sLocation = xScriptUri->getParameter( sLocationParamName );
                const OUString sLangParamName(  "language"  );
                const OUString sLanguage = xScriptUri->getParameter( sLangParamName );

                if ( !(sLocation.isEmpty() && sLanguage.isEmpty()) )
                {
                    aComposeBuffer.appendAscii( " (" );

                    
                    OSL_ENSURE( !sLocation.isEmpty(), "EventHandler::convertToControlValue: unexpected: no location!" );
                    if ( !sLocation.isEmpty() )
                    {
                        aComposeBuffer.append( sLocation );
                        aComposeBuffer.appendAscii( ", " );
                    }

                    
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
                DBG_UNHANDLED_EXCEPTION();
            }
        }

        return makeAny( sScript );
    }

    PropertyState SAL_CALL EventHandler::getPropertyState( const OUString& /*_rPropertyName*/ ) throw (UnknownPropertyException, RuntimeException)
    {
        return PropertyState_DIRECT_VALUE;
    }

    void SAL_CALL EventHandler::addPropertyChangeListener( const Reference< XPropertyChangeListener >& _rxListener ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        if ( !_rxListener.is() )
            throw NullPointerException();
        m_aPropertyListeners.addListener( _rxListener );
    }

    void SAL_CALL EventHandler::removePropertyChangeListener( const Reference< XPropertyChangeListener >& _rxListener ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        m_aPropertyListeners.removeListener( _rxListener );
    }

    Sequence< Property > SAL_CALL EventHandler::getSupportedProperties() throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        if ( !m_bEventsMapInitialized )
        {
            const_cast< EventHandler* >( this )->m_bEventsMapInitialized = true;
            try
            {
                Sequence< Type > aListeners;
                impl_getCopmonentListenerTypes_nothrow( aListeners );
                sal_Int32 listenerCount = aListeners.getLength();

                Property aCurrentProperty;
                OUString sListenerClassName;

                
                const Type* pListeners = aListeners.getConstArray();
                for ( sal_Int32 listener = 0; listener < listenerCount; ++listener, ++pListeners )
                {
                    aCurrentProperty = Property();

                    
                    sListenerClassName = pListeners->getTypeName();
                    OSL_ENSURE( !sListenerClassName.isEmpty(), "EventHandler::getSupportedProperties: strange - no listener name ..." );
                    if ( sListenerClassName.isEmpty() )
                        continue;

                    
                    Sequence< OUString > aMethods( comphelper::getEventMethodsForType( *pListeners ) );

                    const OUString* pMethods = aMethods.getConstArray();
                    sal_uInt32 methodCount = aMethods.getLength();

                    for (sal_uInt32 method = 0 ; method < methodCount ; ++method, ++pMethods )
                    {
                        EventDescription aEvent;
                        if ( !lcl_getEventDescriptionForMethod( *pMethods, aEvent ) )
                            continue;

                        if ( !impl_filterMethod_nothrow( aEvent ) )
                            continue;

                        const_cast< EventHandler* >( this )->m_aEvents.insert( EventMap::value_type(
                            lcl_getEventPropertyName( sListenerClassName, *pMethods ), aEvent ) );
                    }
                }

            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION();
            }
        }

        
        ::std::map< EventId, Property > aOrderedProperties;
        for (   EventMap::const_iterator loop = m_aEvents.begin();
                loop != m_aEvents.end();
                ++loop
            )
        {
            aOrderedProperties[ loop->second.nId ] = Property(
                loop->first, loop->second.nId,
                ::getCppuType( static_cast< const OUString* >( NULL ) ),
                PropertyAttribute::BOUND );
        }

        StlSyntaxSequence< Property > aReturn( aOrderedProperties.size() );
        ::std::transform( aOrderedProperties.begin(), aOrderedProperties.end(), aReturn.begin(),
            ::o3tl::select2nd< ::std::map< EventId, Property >::value_type >() );
        return aReturn;
    }

    Sequence< OUString > SAL_CALL EventHandler::getSupersededProperties( ) throw (RuntimeException)
    {
        
        return Sequence< OUString >( );
    }

    Sequence< OUString > SAL_CALL EventHandler::getActuatingProperties( ) throw (RuntimeException)
    {
        
        return Sequence< OUString >( );
    }

    LineDescriptor SAL_CALL EventHandler::describePropertyLine( const OUString& _rPropertyName,
        const Reference< XPropertyControlFactory >& _rxControlFactory )
        throw (UnknownPropertyException, NullPointerException, RuntimeException)
    {
        if ( !_rxControlFactory.is() )
            throw NullPointerException();

        ::osl::MutexGuard aGuard( m_aMutex );

        LineDescriptor aDescriptor;

        aDescriptor.Control = _rxControlFactory->createPropertyControl( PropertyControlType::TextField, sal_True );
        Reference< XEventListener > xControlExtender = new PropertyControlExtender( aDescriptor.Control );

        const EventDescription& rEvent = impl_getEventForName_throw( _rPropertyName );
        aDescriptor.DisplayName = rEvent.sDisplayName;
        aDescriptor.HelpURL = HelpIdUrl::getHelpURL( rEvent.sHelpId );
        aDescriptor.PrimaryButtonId = OStringToOUString(rEvent.sUniqueBrowseId, RTL_TEXTENCODING_UTF8);
        aDescriptor.HasPrimaryButton = sal_True;
        aDescriptor.Category = "Events";
        return aDescriptor;
    }

    ::sal_Bool SAL_CALL EventHandler::isComposable( const OUString& /*_rPropertyName*/ ) throw (UnknownPropertyException, RuntimeException)
    {
        return sal_False;
    }

    InteractiveSelectionResult SAL_CALL EventHandler::onInteractivePropertySelection( const OUString& _rPropertyName, sal_Bool /*_bPrimary*/, Any& /*_rData*/, const Reference< XObjectInspectorUI >& _rxInspectorUI ) throw (UnknownPropertyException, NullPointerException, RuntimeException)
    {
        if ( !_rxInspectorUI.is() )
            throw NullPointerException();

        ::osl::MutexGuard aGuard( m_aMutex );
        const EventDescription& rForEvent = impl_getEventForName_throw( _rPropertyName );

        Sequence< ScriptEventDescriptor > aAllAssignedEvents;
        impl_getComponentScriptEvents_nothrow( aAllAssignedEvents );

        
        ::rtl::Reference< EventHolder >  pEventHolder( new EventHolder );

        for (   EventMap::const_iterator event = m_aEvents.begin();
                event != m_aEvents.end();
                ++event
            )
        {
            
            ScriptEventDescriptor aAssignedScript = lcl_getAssignedScriptEvent( event->second, aAllAssignedEvents );
            pEventHolder->addEvent( event->second.nId, event->second.sListenerMethodName, aAssignedScript );
        }

        
        Sequence< OUString > aNames( pEventHolder->getElementNames() );
        const OUString* pChosenEvent = ::std::find( aNames.getConstArray(), aNames.getConstArray() + aNames.getLength(), rForEvent.sListenerMethodName );
        sal_uInt16 nInitialSelection = (sal_uInt16)( pChosenEvent - aNames.getConstArray() );

        
        SvxAbstractDialogFactory* pFactory = SvxAbstractDialogFactory::Create();
        if ( !pFactory )
            return InteractiveSelectionResult_Cancelled;

        ::std::auto_ptr< VclAbstractDialog > pDialog( pFactory->CreateSvxMacroAssignDlg(
            PropertyHandlerHelper::getDialogParentWindow( m_xContext ),
            impl_getContextFrame_nothrow(),
            m_bIsDialogElement,
            pEventHolder.get(),
            nInitialSelection
        ) );

        if ( !pDialog.get() )
            return InteractiveSelectionResult_Cancelled;

        
        
        if ( pDialog->Execute() == RET_CANCEL )
            return InteractiveSelectionResult_Cancelled;

        try
        {
            for (   EventMap::const_iterator event = m_aEvents.begin();
                    event != m_aEvents.end();
                    ++event
                )
            {
                ScriptEventDescriptor aScriptDescriptor( pEventHolder->getNormalizedDescriptorByName( event->second.sListenerMethodName ) );

                
                setPropertyValue(
                    lcl_getEventPropertyName( event->second.sListenerClassName, event->second.sListenerMethodName ),
                    makeAny( aScriptDescriptor )
                );
            }
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }

        return InteractiveSelectionResult_Success;
    }

    void SAL_CALL EventHandler::actuatingPropertyChanged( const OUString& /*_rActuatingPropertyName*/, const Any& /*_rNewValue*/, const Any& /*_rOldValue*/, const Reference< XObjectInspectorUI >& /*_rxInspectorUI*/, sal_Bool /*_bFirstTimeInit*/ ) throw (NullPointerException, RuntimeException)
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

    sal_Bool SAL_CALL EventHandler::suspend( sal_Bool /*_bSuspend*/ ) throw (RuntimeException)
    {
        return sal_True;
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
            DBG_UNHANDLED_EXCEPTION();
        }

        return xContextFrame;
    }

    sal_Int32 EventHandler::impl_getComponentIndexInParent_throw() const
    {
        Reference< XChild > xChild( m_xComponent, UNO_QUERY_THROW );
        Reference< XIndexAccess > xParentAsIndexAccess( xChild->getParent(), UNO_QUERY_THROW );

        
        sal_Int32 nElements = xParentAsIndexAccess->getCount();
        for ( sal_Int32 i=0; i<nElements; ++i )
        {
            Reference< XInterface > xElement( xParentAsIndexAccess->getByIndex( i ), UNO_QUERY_THROW );
            if ( xElement == m_xComponent )
                return i;
        }
        throw NoSuchElementException();
    }

    void EventHandler::impl_getFormComponentScriptEvents_nothrow( Sequence < ScriptEventDescriptor >& _out_rEvents ) const
    {
        _out_rEvents = Sequence < ScriptEventDescriptor >();
        try
        {
            Reference< XChild > xChild( m_xComponent, UNO_QUERY_THROW );
            Reference< XEventAttacherManager > xEventManager( xChild->getParent(), UNO_QUERY_THROW );
            _out_rEvents = xEventManager->getScriptEvents( impl_getComponentIndexInParent_throw() );

            
            
            ScriptEventDescriptor* pEvents = _out_rEvents.getArray();
            ScriptEventDescriptor* pEventsEnd = _out_rEvents.getArray() + _out_rEvents.getLength();
            while ( pEvents != pEventsEnd )
            {
                pEvents->ListenerType = lcl_getQualifiedKnownListenerName( *pEvents );
                ++pEvents;
            }
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }

    void EventHandler::impl_getCopmonentListenerTypes_nothrow( Sequence< Type >& _out_rTypes ) const
    {
        _out_rTypes.realloc( 0 );
        try
        {
            
            TypeBag aListeners;

            Reference< XIntrospection > xIntrospection = Introspection::create( m_xContext );

            
            lcl_addListenerTypesFor_throw(
                m_xComponent, xIntrospection, aListeners );

            
            {
                Reference< XInterface > xSecondaryComponent( impl_getSecondaryComponentForEventInspection_throw() );
                lcl_addListenerTypesFor_throw( xSecondaryComponent, xIntrospection, aListeners );
                ::comphelper::disposeComponent( xSecondaryComponent );
            }

            
            _out_rTypes.realloc( aListeners.size() );
            ::std::copy( aListeners.begin(), aListeners.end(), _out_rTypes.getArray() );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }

    void EventHandler::impl_getDialogElementScriptEvents_nothrow( Sequence < ScriptEventDescriptor >& _out_rEvents ) const
    {
        _out_rEvents = Sequence < ScriptEventDescriptor >();
        try
        {
            Reference< XScriptEventsSupplier > xEventsSupplier( m_xComponent, UNO_QUERY_THROW );
            Reference< XNameContainer > xEvents( xEventsSupplier->getEvents(), UNO_QUERY_THROW );
            Sequence< OUString > aEventNames( xEvents->getElementNames() );

            sal_Int32 nEventCount = aEventNames.getLength();
            _out_rEvents.realloc( nEventCount );

            const OUString* pNames = aEventNames.getConstArray();
            ScriptEventDescriptor* pDescs = _out_rEvents.getArray();

            for( sal_Int32 i = 0 ; i < nEventCount ; ++i, ++pNames, ++pDescs )
                OSL_VERIFY( xEvents->getByName( *pNames ) >>= *pDescs );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }

    Reference< XInterface > EventHandler::impl_getSecondaryComponentForEventInspection_throw( ) const
    {
        Reference< XInterface > xReturn;

        
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
        static bool lcl_endsWith( const OUString& _rText, const OUString& _rCheck )
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
            Sequence< ScriptEventDescriptor > aEvents( xEventManager->getScriptEvents( nObjectIndex ) );

            
            ScriptEventDescriptor* pEvent = aEvents.getArray();
            sal_Int32 eventCount = aEvents.getLength(), event = 0;
            for ( event = 0; event < eventCount; ++event, ++pEvent )
            {
                if  (   ( pEvent->EventMethod == _rScriptEvent.EventMethod )
                    &&  ( lcl_endsWith( _rScriptEvent.ListenerType, pEvent->ListenerType ) )
                          
                    )
                {
                    
                    if ( !bResetScript )
                    {
                        
                        pEvent->ScriptCode = sScriptCode;
                        pEvent->ScriptType = sScriptType;
                    }
                    else
                    {
                        
                        ::std::copy( pEvent + 1, aEvents.getArray() + eventCount, pEvent );
                        aEvents.realloc( eventCount - 1 );
                        --eventCount;
                    }
                    break;
                }
            }
            if ( ( event >= eventCount ) && !bResetScript )
            {
                
                aEvents.realloc( eventCount + 1 );
                aEvents[ eventCount ] = _rScriptEvent;
            }

            xEventManager->revokeScriptEvents( nObjectIndex );
            xEventManager->registerScriptEvents( nObjectIndex, aEvents );

            PropertyHandlerHelper::setContextDocumentModified( m_xContext );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
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
            aCompleteName.appendAscii( "::" );
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
            DBG_UNHANDLED_EXCEPTION();
        }
    }

    bool EventHandler::impl_filterMethod_nothrow( const EventDescription& _rEvent ) const
    {
        
        
        
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

} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
