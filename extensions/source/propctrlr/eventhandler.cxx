/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: eventhandler.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-07-14 07:21:10 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef EXTENSIONS_SOURCE_PROPCTRLR_EVENTHANDLER_HXX
#include "eventhandler.hxx"
#endif
#ifndef _EXTENSIONS_FORMSCTRLR_FORMBROWSERTOOLS_HXX_
#include "formbrowsertools.hxx"
#endif
#ifndef _EXTENSIONS_FORMSCTRLR_FORMSTRINGS_HXX_
#include "formstrings.hxx"
#endif
#ifndef _EXTENSIONS_FORMSCTRLR_PCRSTRINGS_HXX_
#include "pcrstrings.hxx"
#endif
#ifndef _EXTENSIONS_PROPCTRLR_MODULEPRC_HXX_
#include "modulepcr.hxx"
#endif
#ifndef _EXTENSIONS_FORMCTRLR_PROPRESID_HRC_
#include "formresid.hrc"
#endif
#ifndef EXTENSIONS_INC_EXTENSIO_HRC
#include "extensio.hrc"
#endif
#ifndef EXTENSIONS_SOURCE_PROPCTRLR_HANDLERHELPER_HXX
#include "handlerhelper.hxx"
#endif
#ifndef _EXTENSIONS_PROPCTRLR_PCRCOMMON_HXX_
#include "pcrcommon.hxx"
#endif

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_LANG_NULLPOINTEREXCEPTION_HPP_
#include <com/sun/star/lang/NullPointerException.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCHILD_HPP_
#include <com/sun/star/container/XChild.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XINDEXACCESS_HPP_
#include <com/sun/star/container/XIndexAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_XEVENTATTACHERMANAGER_HPP_
#include <com/sun/star/script/XEventAttacherManager.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XINTROSPECTION_HPP_
#include <com/sun/star/beans/XIntrospection.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XINTROSPECTIONACCESS_HPP_
#include <com/sun/star/beans/XIntrospectionAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_XSCRIPTEVENTSSUPPLIER_HPP_
#include <com/sun/star/script/XScriptEventsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XTABCONTROLLERMODEL_HPP_
#include <com/sun/star/awt/XTabControllerModel.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XFORM_HPP_
#include <com/sun/star/form/XForm.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XFORMCONTROLLER_HPP_
#include <com/sun/star/form/XFormController.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_UNKNOWNPROPERTYEXCEPTION_HPP_
#include <com/sun/star/beans/UnknownPropertyException.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_NOSUCHELEMENTEXCEPTION_HPP_
#include <com/sun/star/container/NoSuchElementException.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEREPLACE_HPP_
#include <com/sun/star/container/XNameReplace.hpp>
#endif
#ifndef _COM_SUN_STAR_INSPECTION_PROPERTYCONTROLTYPE_HPP_
#include <com/sun/star/inspection/PropertyControlType.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XMODIFIABLE_HPP_
#include <com/sun/star/util/XModifiable.hpp>
#endif
/** === end UNO includes === **/

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif
#ifndef _COMPHELPER_SEQUENCEASHASHMAP_HXX_
#include <comphelper/sequenceashashmap.hxx>
#endif
#ifndef _SVX_DIALOG_HXX
#include <svx/svxdlg.hxx>
#endif
#ifndef _SFXAPP_HXX
#include <sfx2/app.hxx>
#endif
#ifndef _SFXITEMSET_HXX
#include <svtools/itemset.hxx>
#endif
#ifndef _EITEM_HXX
#include <svtools/eitem.hxx>
#endif
#ifndef _SVX_SVXIDS_HRC
#include <svx/svxids.hrc>
#endif
#ifndef _RTL_REF_HXX_
#include <rtl/ref.hxx>
#endif

//------------------------------------------------------------------------
extern "C" void SAL_CALL createRegistryInfo_EventHandler()
{
    ::pcr::OAutoRegistration< ::pcr::EventHandler > aAutoRegistration;
}

//........................................................................
namespace pcr
{
//........................................................................

    /** === begin UNO using === **/
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::XComponentContext;
    using ::com::sun::star::beans::XPropertySet;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::TypeClass_STRING;
    using ::com::sun::star::uno::Type;
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
    using ::com::sun::star::form::XFormController;
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
    /** === end UNO using === **/
    namespace PropertyControlType = ::com::sun::star::inspection::PropertyControlType;
    namespace PropertyAttribute = ::com::sun::star::beans::PropertyAttribute;

    //====================================================================
    //= EventDescription
    //====================================================================
    EventDescription::EventDescription( EventId _nId, const sal_Char* _pListenerNamespaceAscii, const sal_Char* _pListenerClassAsciiName,
            const sal_Char* _pListenerMethodAsciiName, sal_uInt16 _nDisplayNameResId, sal_Int32 _nHelpId, sal_Int32 _nUniqueBrowseId )
        :nId( _nId )
        ,sListenerMethodName( ::rtl::OUString::createFromAscii( _pListenerMethodAsciiName ) )
        ,sDisplayName( String( PcrRes( _nDisplayNameResId ) ) )
        ,nHelpId( _nHelpId )
        ,nUniqueBrowseId( _nUniqueBrowseId )
    {
        ::rtl::OUStringBuffer aQualifiedListenerClass;
        aQualifiedListenerClass.appendAscii( "com.sun.star." );
        aQualifiedListenerClass.appendAscii( _pListenerNamespaceAscii );
        aQualifiedListenerClass.appendAscii( "." );
        aQualifiedListenerClass.appendAscii( _pListenerClassAsciiName );
        sListenerClassName = aQualifiedListenerClass.makeStringAndClear();
    }

    //========================================================================
    //= helper
    //========================================================================
    namespace
    {
        //....................................................................
        Sequence< ::rtl::OUString > lcl_getListenerMethodsForType( const Type& type )
        {
            typelib_InterfaceTypeDescription *pType=0;
            type.getDescription( (typelib_TypeDescription**)&pType);

            if ( !pType )
                return Sequence< ::rtl::OUString>();

            Sequence< ::rtl::OUString > aNames( pType->nMembers );
            ::rtl::OUString* pNames = aNames.getArray();
            for ( sal_Int32 i = 0; i < pType->nMembers; ++i, ++pNames)
            {
                // the decription reference
                typelib_TypeDescriptionReference* pMemberDescriptionReference = pType->ppMembers[i];
                // the description for the reference
                typelib_TypeDescription* pMemberDescription = NULL;
                typelib_typedescriptionreference_getDescription( &pMemberDescription, pMemberDescriptionReference );
                if ( pMemberDescription )
                {
                    typelib_InterfaceMemberTypeDescription* pRealMemberDescription =
                        reinterpret_cast<typelib_InterfaceMemberTypeDescription*>(pMemberDescription);
                    *pNames = pRealMemberDescription->pMemberName;
                }
            }

            typelib_typedescription_release( (typelib_TypeDescription*)pType );
            return aNames;
        }

        //....................................................................
        #define DESCRIBE_EVENT( asciinamespace, asciilistener, asciimethod, id_postfix ) \
            s_aKnownEvents.insert( EventMap::value_type( \
                ::rtl::OUString::createFromAscii( asciimethod ), \
                EventDescription( ++nEventId, asciinamespace, asciilistener, asciimethod, RID_STR_EVT_##id_postfix, HID_EVT_##id_postfix, UID_BRWEVT_##id_postfix ) ) )

        //....................................................................
        bool lcl_getEventDescriptionForMethod( const ::rtl::OUString& _rMethodName, EventDescription& _out_rDescription )
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
                    DESCRIBE_EVENT( "sdb",  "XSQLErrorListener",            "errorOccured",             ERROROCCURED );
                    DESCRIBE_EVENT( "awt",  "XAdjustmentListener",          "adjustmentValueChanged",   ADJUSTMENTVALUECHANGED );
                }
            }

            EventMap::const_iterator pos = s_aKnownEvents.find( _rMethodName );
            if ( pos == s_aKnownEvents.end() )
                return false;

            _out_rDescription = pos->second;
            return true;
        }

        //....................................................................
        ::rtl::OUString lcl_getEventPropertyName( const ::rtl::OUString& _rListenerClassName, const ::rtl::OUString& _rMethodName )
        {
            ::rtl::OUStringBuffer aPropertyName;
            aPropertyName.append( _rListenerClassName );
            aPropertyName.append( (sal_Unicode)';' );
            aPropertyName.append( _rMethodName.getStr() );
            return aPropertyName.makeStringAndClear();
        }
    }

    //====================================================================
    //= EventHandler
    //====================================================================
    typedef ::cppu::WeakImplHelper1 <   ::com::sun::star::container::XNameReplace
                                    >   EventHolder_Base;
    /** a UNO component holding assigned event descriptions, for use with a SvxMacroAssignDlg
    */
    class EventHolder : public EventHolder_Base
    {
    private:
        typedef ::std::hash_map< ::rtl::OUString, ::rtl::OUString, ::rtl::OUStringHash >    EventMap;
        typedef ::std::map< EventId, EventMap::iterator >                                   EventMapIndexAccess;

        EventMap            m_aEvents;
        EventMapIndexAccess m_aEventIndexAccess;

    public:
        EventHolder( );

        void AddEvent( EventId _nId, const ::rtl::OUString& _rEventName, const ::rtl::OUString& _rMacroURL );

        // XNameReplace
        virtual void SAL_CALL replaceByName( const ::rtl::OUString& _rName, const Any& aElement ) throw (IllegalArgumentException, NoSuchElementException, WrappedTargetException, RuntimeException);
        virtual Any SAL_CALL getByName( const ::rtl::OUString& _rName ) throw (NoSuchElementException, WrappedTargetException, RuntimeException);
        virtual Sequence< ::rtl::OUString > SAL_CALL getElementNames(  ) throw (RuntimeException);
        virtual ::sal_Bool SAL_CALL hasByName( const ::rtl::OUString& _rName ) throw (RuntimeException);
        virtual Type SAL_CALL getElementType(  ) throw (RuntimeException);
        virtual ::sal_Bool SAL_CALL hasElements(  ) throw (RuntimeException);

    protected:
        ~EventHolder( );
    };

    DBG_NAME( EventHolder )
    //------------------------------------------------------------------------
    EventHolder::EventHolder()
    {
        DBG_CTOR( EventHolder, NULL );
    }

    //------------------------------------------------------------------------
    EventHolder::~EventHolder()
    {
        m_aEvents.clear();
        m_aEventIndexAccess.clear();
        DBG_DTOR( EventHolder, NULL );
    }

    //------------------------------------------------------------------------
    void EventHolder::AddEvent( EventId _nId, const ::rtl::OUString& _rEventName, const ::rtl::OUString&_rMacroURL )
    {
        ::std::pair< EventMap::iterator, bool > insertionResult =
            m_aEvents.insert( EventMap::value_type( _rEventName, _rMacroURL ) );
        DBG_ASSERT( insertionResult.second, "EventHolder::AddEvent: there already was a MacroURL for this event!" );
        m_aEventIndexAccess[ _nId ] = insertionResult.first;
    }

    //------------------------------------------------------------------------
    void SAL_CALL EventHolder::replaceByName( const ::rtl::OUString& _rName, const Any& _rElement ) throw (IllegalArgumentException, NoSuchElementException, WrappedTargetException, RuntimeException)
    {
        EventMap::iterator pos = m_aEvents.find( _rName );
        if ( pos == m_aEvents.end() )
            throw NoSuchElementException( ::rtl::OUString(), *this );

        Sequence< PropertyValue > aScriptDescriptor;
        OSL_VERIFY( _rElement >>= aScriptDescriptor );
        ::comphelper::SequenceAsHashMap aExtractor( aScriptDescriptor );

        ::rtl::OUString sMacroURL;
        OSL_VERIFY( aExtractor[ ::rtl::OUString::createFromAscii( "Script" ) ] >>= sMacroURL );
        pos->second = sMacroURL;
    }

    //------------------------------------------------------------------------
    Any SAL_CALL EventHolder::getByName( const ::rtl::OUString& _rName ) throw (NoSuchElementException, WrappedTargetException, RuntimeException)
    {
        EventMap::const_iterator pos = m_aEvents.find( _rName );
        if ( pos == m_aEvents.end() )
            throw NoSuchElementException( ::rtl::OUString(), *this );

        Any aRet;
        Sequence< PropertyValue > aScriptDescriptor( 2 );
        aScriptDescriptor[0].Name = ::rtl::OUString::createFromAscii( "EventType" );
        aScriptDescriptor[0].Value <<= ::rtl::OUString::createFromAscii( "Script" );
        aScriptDescriptor[1].Name = ::rtl::OUString::createFromAscii( "Script" );
        aScriptDescriptor[1].Value <<= pos->second;

        return makeAny( aScriptDescriptor );
    }

    //------------------------------------------------------------------------
    Sequence< ::rtl::OUString > SAL_CALL EventHolder::getElementNames(  ) throw (RuntimeException)
    {
        Sequence< ::rtl::OUString > aReturn( m_aEventIndexAccess.size() );
        ::rtl::OUString* pReturn = aReturn.getArray();

        // SvxMacroAssignDlg has a weird API: It expects a XNameReplace, means a container whose
        // main access method is by name. In it's UI, it shows the possible events in exactly the
        // order in which XNameAccess::getElementNames returns them.
        // However, SvxMacroAssignDlg *also* takes an index for the initial selection, which is
        // relative to the sequence returned by XNameAccess::getElementNames.
        // This is IMO weird, since it mixes index access with name access, which decreases efficiency
        // of the implementation.
        // Well, it means we're forced to return the events in getElementNames in exactly the same as they
        // appear in the property browser UI.
        for (   EventMapIndexAccess::const_iterator loop = m_aEventIndexAccess.begin();
                loop != m_aEventIndexAccess.end();
                ++loop, ++pReturn
            )
            *pReturn = loop->second->first;
        return aReturn;
    }

     //------------------------------------------------------------------------
    sal_Bool SAL_CALL EventHolder::hasByName( const ::rtl::OUString& _rName ) throw (RuntimeException)
    {
        EventMap::const_iterator pos = m_aEvents.find( _rName );
        return pos != m_aEvents.end();
    }

    //------------------------------------------------------------------------
    Type SAL_CALL EventHolder::getElementType(  ) throw (RuntimeException)
    {
        return ::getCppuType( static_cast< Sequence< PropertyValue >* >( NULL ) );
    }

    //------------------------------------------------------------------------
    sal_Bool SAL_CALL EventHolder::hasElements(  ) throw (RuntimeException)
    {
        return !m_aEvents.empty();
    }


    //====================================================================
    //= EventHandler
    //====================================================================
    DBG_NAME( EventHandler )
    //--------------------------------------------------------------------
    EventHandler::EventHandler( const Reference< XComponentContext >& _rxContext )
        :EventHandler_Base( m_aMutex )
        ,m_aContext( _rxContext )
        ,m_bEventsMapInitialized( false )
        ,m_bIsDialogElement( false )
        ,m_aPropertyListeners( m_aMutex )
    {
        DBG_CTOR( EventHandler, NULL );
    }

    //--------------------------------------------------------------------
    EventHandler::~EventHandler()
    {
        DBG_DTOR( EventHandler, NULL );
    }

    //--------------------------------------------------------------------
    ::rtl::OUString SAL_CALL EventHandler::getImplementationName(  ) throw (RuntimeException)
    {
        return getImplementationName_static();
    }

    //--------------------------------------------------------------------
    ::sal_Bool SAL_CALL EventHandler::supportsService( const ::rtl::OUString& ServiceName ) throw (RuntimeException)
    {
        StlSyntaxSequence< ::rtl::OUString > aAllServices( getSupportedServiceNames() );
        return ::std::find( aAllServices.begin(), aAllServices.end(), ServiceName ) != aAllServices.end();
    }

    //--------------------------------------------------------------------
    Sequence< ::rtl::OUString > SAL_CALL EventHandler::getSupportedServiceNames(  ) throw (RuntimeException)
    {
        return getSupportedServiceNames_static();
    }

    //--------------------------------------------------------------------
    ::rtl::OUString SAL_CALL EventHandler::getImplementationName_static(  ) throw (RuntimeException)
    {
        return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.extensions.EventHandler" ) );
    }

    //--------------------------------------------------------------------
    Sequence< ::rtl::OUString > SAL_CALL EventHandler::getSupportedServiceNames_static(  ) throw (RuntimeException)
    {
        Sequence< ::rtl::OUString > aSupported( 1 );
        aSupported[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.form.inspection.EventHandler" ) );
        return aSupported;
    }

    //--------------------------------------------------------------------
    Reference< XInterface > SAL_CALL EventHandler::Create( const Reference< XComponentContext >& _rxContext )
    {
        return *( new EventHandler( _rxContext ) );
    }

    //--------------------------------------------------------------------
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
        try
        {
            Reference< XPropertySetInfo > xPSI( m_xComponent->getPropertySetInfo() );
            m_bIsDialogElement = xPSI.is()
                              && xPSI->hasPropertyByName( PROPERTY_WIDTH )
                              && xPSI->hasPropertyByName( PROPERTY_HEIGHT )
                              && xPSI->hasPropertyByName( PROPERTY_POSITIONX )
                              && xPSI->hasPropertyByName( PROPERTY_POSITIONY );
        }
        catch( const Exception& )
        {
            OSL_ENSURE( false, "EventHandler::EventHandler: caught an exception while classifying the component!" );
        }
    }

    //--------------------------------------------------------------------
    Any SAL_CALL EventHandler::getPropertyValue( const ::rtl::OUString& _rPropertyName ) throw (UnknownPropertyException, RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        const EventDescription& rEvent = impl_getEventForName_throw( _rPropertyName );

        Sequence< ScriptEventDescriptor > aEvents;
        impl_getComponentScriptEvents_nothrow( aEvents );

        sal_Int32 nEventCount = aEvents.getLength();
        const ScriptEventDescriptor* pEvents = aEvents.getConstArray();

        ::rtl::OUString sPropertyValue;
        for ( sal_Int32 event = 0; event < nEventCount; ++event, ++pEvents )
        {
            if  (   rEvent.sListenerClassName == pEvents->ListenerType
                &&  rEvent.sListenerMethodName == pEvents->EventMethod
                )
            {
                sPropertyValue = pEvents->ScriptCode;
                break;
            }
        }

        return makeAny( sPropertyValue );
    }

    //--------------------------------------------------------------------
    void SAL_CALL EventHandler::setPropertyValue( const ::rtl::OUString& _rPropertyName, const Any& _rValue ) throw (UnknownPropertyException, RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        const EventDescription& rEvent = impl_getEventForName_throw( _rPropertyName );
        ::rtl::OUString sNewScriptCode;
        OSL_VERIFY( _rValue >>= sNewScriptCode );

        ::rtl::OUString sOldScriptCode;
        OSL_VERIFY( getPropertyValue( _rPropertyName ) >>= sOldScriptCode );
        if ( sOldScriptCode == sNewScriptCode )
            return;

        if ( m_bIsDialogElement )
            impl_setDialogElementScriptEvent_nothrow( rEvent, sNewScriptCode );
        else
            impl_setFormComponentScriptEvent_nothrow( rEvent, sNewScriptCode );

        Reference< XModifiable > xDoc( m_aContext.getContextValueByAsciiName( "ContextDocument" ), UNO_QUERY );
        if ( xDoc.is() )
            xDoc->setModified( sal_True );

        PropertyChangeEvent aEvent;
        aEvent.Source = m_xComponent;
        aEvent.PropertyHandle = rEvent.nId;
        aEvent.PropertyName = _rPropertyName;
        aEvent.OldValue <<= sOldScriptCode;
        aEvent.NewValue <<= sNewScriptCode;
        m_aPropertyListeners.notify( aEvent, &XPropertyChangeListener::propertyChange );
    }

    //--------------------------------------------------------------------
    Any SAL_CALL EventHandler::convertToPropertyValue( const ::rtl::OUString& _rPropertyName, const Any& _rControlValue ) throw (UnknownPropertyException, RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        DBG_ASSERT( !_rControlValue.hasValue() || _rControlValue.getValueTypeClass() == TypeClass_STRING,
            "EventHandler::convertToPropertyValue: not NULL and not STRING?" );
        return _rControlValue;
    }

    //--------------------------------------------------------------------
    Any SAL_CALL EventHandler::convertToControlValue( const ::rtl::OUString& _rPropertyName, const Any& _rPropertyValue, const Type& _rControlValueType ) throw (UnknownPropertyException, RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        DBG_ASSERT( !_rPropertyValue.hasValue() || _rPropertyValue.getValueTypeClass() == TypeClass_STRING,
            "EventHandler::convertToControlValue: not NULL and not STRING?" );
        DBG_ASSERT( _rControlValueType.getTypeClass() == TypeClass_STRING,
            "EventHandler::convertToControlValue: unexpected ControlValue type class!" );
        return _rPropertyValue;
    }

    //--------------------------------------------------------------------
    PropertyState SAL_CALL EventHandler::getPropertyState( const ::rtl::OUString& _rPropertyName ) throw (UnknownPropertyException, RuntimeException)
    {
        return PropertyState_DIRECT_VALUE;
    }

    //--------------------------------------------------------------------
    void SAL_CALL EventHandler::addPropertyChangeListener( const Reference< XPropertyChangeListener >& _rxListener ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        if ( !_rxListener.is() )
            throw NullPointerException();
        m_aPropertyListeners.addListener( _rxListener );
    }

    //--------------------------------------------------------------------
    void SAL_CALL EventHandler::removePropertyChangeListener( const Reference< XPropertyChangeListener >& _rxListener ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        m_aPropertyListeners.removeListener( _rxListener );
    }

    //--------------------------------------------------------------------
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
                ::rtl::OUString sListenerClassName;

                // loop through all listeners and all methods, and see which we can present at the UI
                const Type* pListeners = aListeners.getConstArray();
                for ( sal_Int32 listener = 0; listener < listenerCount; ++listener, ++pListeners )
                {
                    aCurrentProperty = Property();

                    // the programmatic name of the listener, to be used as "property" name
                    sListenerClassName = pListeners->getTypeName();
                    OSL_ENSURE( sListenerClassName.getLength(), "EventHandler::getSupportedProperties: strange - no listener name ..." );
                    if ( !sListenerClassName.getLength() )
                        continue;

                    // loop through all methods
                    Sequence< ::rtl::OUString > aMethods( lcl_getListenerMethodsForType( *pListeners ) );

                    const ::rtl::OUString* pMethods = aMethods.getConstArray();
                    sal_uInt32 methodCount = aMethods.getLength();

                    for (sal_uInt32 method = 0 ; method < methodCount ; method++,++pMethods )
                    {
                        EventDescription aEvent;
                        if ( !lcl_getEventDescriptionForMethod( *pMethods, aEvent ) )
                            continue;

                        const_cast< EventHandler* >( this )->m_aEvents.insert( EventMap::value_type(
                            lcl_getEventPropertyName( sListenerClassName, *pMethods ), aEvent ) );
                    }
                }

            }
            catch( const Exception& )
            {
                DBG_ERROR( "EventHandler::getSupportedProperties: caught an exception !" )
            }
        }

        // sort them by ID - this is the relative ordering in the UI
        ::std::map< EventId, Property > aOrderedProperties;
        for (   EventMap::const_iterator loop = m_aEvents.begin();
                loop != m_aEvents.end();
                ++loop
            )
        {
            aOrderedProperties[ loop->second.nId ] = Property(
                loop->first, loop->second.nId,
                ::getCppuType( static_cast< const ::rtl::OUString* >( NULL ) ),
                PropertyAttribute::BOUND );
        }

        StlSyntaxSequence< Property > aReturn( aOrderedProperties.size() );
        ::std::transform( aOrderedProperties.begin(), aOrderedProperties.end(), aReturn.begin(),
            ::std::select2nd< ::std::map< EventId, Property >::value_type >() );
        return aReturn;
    }

    //--------------------------------------------------------------------
    Sequence< ::rtl::OUString > SAL_CALL EventHandler::getSupersededProperties( ) throw (RuntimeException)
    {
        // none
        return Sequence< ::rtl::OUString >( );
    }

    //--------------------------------------------------------------------
    Sequence< ::rtl::OUString > SAL_CALL EventHandler::getActuatingProperties( ) throw (RuntimeException)
    {
        // none
        return Sequence< ::rtl::OUString >( );
    }

    //--------------------------------------------------------------------
    void SAL_CALL EventHandler::describePropertyLine( const ::rtl::OUString& _rPropertyName,
        LineDescriptor& /* [out] */ _out_rDescriptor, const Reference< XPropertyControlFactory >& _rxControlFactory )
        throw (UnknownPropertyException, NullPointerException, RuntimeException)
    {
        if ( !_rxControlFactory.is() )
            throw NullPointerException();

        ::osl::MutexGuard aGuard( m_aMutex );

        _out_rDescriptor.Control = _rxControlFactory->createPropertyControl( PropertyControlType::TextField, sal_True );
        const EventDescription& rEvent = impl_getEventForName_throw( _rPropertyName );
        _out_rDescriptor.DisplayName = rEvent.sDisplayName;
        _out_rDescriptor.HelpURL = HelpIdUrl::getHelpURL( rEvent.nHelpId );
        _out_rDescriptor.PrimaryButtonId = rEvent.nUniqueBrowseId;
        _out_rDescriptor.HasPrimaryButton = sal_True;
        _out_rDescriptor.Category = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Events" ) );
    }

    //--------------------------------------------------------------------
    namespace
    {
        //................................................................
        ::rtl::OUString lcl_getAssignedMacroURL( const EventDescription& _rEvent, const Sequence< ScriptEventDescriptor >& _rAllAssignedMacros )
        {
            ::rtl::OUString sScriptCode;

            const ScriptEventDescriptor* pAssignedEvent = _rAllAssignedMacros.getConstArray();
            sal_Int32 assignedEventCount( _rAllAssignedMacros.getLength() );
            for ( sal_Int32 assignedEvent = 0; assignedEvent < assignedEventCount; ++assignedEvent, ++pAssignedEvent )
            {
                if  (   ( pAssignedEvent->ListenerType != _rEvent.sListenerClassName )
                    ||  ( pAssignedEvent->EventMethod != _rEvent.sListenerMethodName )
                    )
                    continue;

                if  (   ( pAssignedEvent->ScriptCode.getLength() == 0 )
                    ||  ( pAssignedEvent->ScriptType.getLength() == 0 )
                    )
                {
                    DBG_ERROR( "lcl_getAssignedMacroURL: me thinks this should not happen!" );
                    continue;
                }

                sScriptCode = pAssignedEvent->ScriptCode;

                if ( 0 == pAssignedEvent->ScriptType.compareToAscii( "StarBasic" ) )
                {   // it's a StarBasic macro
                    sal_Int32 nPrefixLen = sScriptCode.indexOf( ':' );
                    if ( nPrefixLen >= 0 )
                    {
                        // strip the prefix from the macro name

                        // (This is from the times where Basic macro names were stored
                        // as "document:<macro_name>" or "application:<macro_name>". Since
                        // the new scripting framework, they have another format ...)

                        sScriptCode = sScriptCode.copy( nPrefixLen + 1 );
                    }
                }
            }
            return sScriptCode;
        }
    }

    //--------------------------------------------------------------------
    ::sal_Bool SAL_CALL EventHandler::isComposable( const ::rtl::OUString& _rPropertyName ) throw (UnknownPropertyException, RuntimeException)
    {
        return sal_False;
    }

    //--------------------------------------------------------------------
    InteractiveSelectionResult SAL_CALL EventHandler::onInteractivePropertySelection( const ::rtl::OUString& _rPropertyName, sal_Bool _bPrimary, Any& _rData, const Reference< XObjectInspectorUI >& _rxInspectorUI ) throw (UnknownPropertyException, NullPointerException, RuntimeException)
    {
        if ( !_rxInspectorUI.is() )
            throw NullPointerException();

        ::osl::MutexGuard aGuard( m_aMutex );
        const EventDescription& rForEvent = impl_getEventForName_throw( _rPropertyName );

        Sequence< ScriptEventDescriptor > aAssignedEvents;
        impl_getComponentScriptEvents_nothrow( aAssignedEvents );

        // SvxMacroAssignDlg-compatible structure holding all event/assignments
        ::rtl::Reference< EventHolder >  pEventHolder( new EventHolder );

        for (   EventMap::const_iterator event = m_aEvents.begin();
                event != m_aEvents.end();
                ++event
            )
        {
            // the script which is assigned to the current event (if any)
            ::rtl::OUString sAssignedMacroURL = lcl_getAssignedMacroURL( event->second, aAssignedEvents );

            pEventHolder->AddEvent( event->second.nId, event->second.sListenerMethodName, sAssignedMacroURL );
        }

        // the inital selection in the dialog
        Sequence< ::rtl::OUString > aNames( pEventHolder->getElementNames() );
        const ::rtl::OUString* pChosenEvent = ::std::find( aNames.getConstArray(), aNames.getConstArray() + aNames.getLength(), rForEvent.sListenerMethodName );
        sal_uInt16 nInitialSelection = pChosenEvent - aNames.getConstArray();

        // the dialog
        SvxAbstractDialogFactory* pFactory = SvxAbstractDialogFactory::Create();
        SfxItemSet aItems( SFX_APP()->GetPool(), SID_ATTR_MACROITEM, SID_ATTR_MACROITEM );
        aItems.Put( SfxBoolItem( SID_ATTR_MACROITEM, m_bIsDialogElement ) );
        if ( !pFactory )
            return InteractiveSelectionResult_Cancelled;
        ::std::auto_ptr< VclAbstractDialog > pDialog( pFactory->CreateSvxMacroAssignDlg(
            PropertyHandlerHelper::getDialogParentWindow( m_aContext ), aItems, pEventHolder.get(), nInitialSelection ) );
        if ( !pDialog.get() )
            return InteractiveSelectionResult_Cancelled;

        // DF definite problem here
        // OK & Cancel seem to be both returning 0
        if ( pDialog->Execute() != 0 )
            return InteractiveSelectionResult_Cancelled;

        try
        {
            ::rtl::OUString sScriptCode;
            for (   EventMap::const_iterator event = m_aEvents.begin();
                    event != m_aEvents.end();
                    ++event
                )
            {
                Sequence< PropertyValue > aScriptDescriptor;
                OSL_VERIFY( pEventHolder->getByName( event->second.sListenerMethodName ) >>= aScriptDescriptor );
                ::comphelper::SequenceAsHashMap aExtractor( aScriptDescriptor );
                OSL_VERIFY( aExtractor[ ::rtl::OUString::createFromAscii( "Script" ) ] >>= sScriptCode );

                // set the new "property value"
                ::rtl::OUString sPropertyName( lcl_getEventPropertyName( event->second.sListenerClassName, event->second.sListenerMethodName ) );
                setPropertyValue( sPropertyName, makeAny( sScriptCode ) );
            }
        }
        catch( const Exception& e )
        {
        #if OSL_DEBUG_LEVEL > 0
            ::rtl::OString sMessage( "EventHandler::onInteractivePropertySelection: caught an exception!\n" );
            sMessage += "message:\n";
            sMessage += ::rtl::OString( e.Message.getStr(), e.Message.getLength(), osl_getThreadTextEncoding() );
            OSL_ENSURE( false, sMessage );
        #else
            e; // make compiler happy
        #endif
        }

        return InteractiveSelectionResult_Success;
    }

    //--------------------------------------------------------------------
    void SAL_CALL EventHandler::actuatingPropertyChanged( const ::rtl::OUString& _rActuatingPropertyName, const Any& _rNewValue, const Any& _rOldValue, const Reference< XObjectInspectorUI >& _rxInspectorUI, sal_Bool _bFirstTimeInit ) throw (NullPointerException, RuntimeException)
    {
        DBG_ERROR( "EventHandler::actuatingPropertyChanged: no actuating properties -> no callback (well, this is how it *should* be!)" );
    }

    //--------------------------------------------------------------------
    IMPLEMENT_FORWARD_XCOMPONENT( EventHandler, EventHandler_Base )

    //--------------------------------------------------------------------
    void SAL_CALL EventHandler::disposing()
    {
        EventMap aEmpty;
        m_aEvents.swap( aEmpty );
        m_xComponent.clear();
    }

    //--------------------------------------------------------------------
    sal_Bool SAL_CALL EventHandler::suspend( sal_Bool _bSuspend ) throw (RuntimeException)
    {
        return sal_True;
    }

    //--------------------------------------------------------------------
    sal_Int32 EventHandler::impl_getComponentIndexInParent_throw() const
    {
        Reference< XChild > xChild( m_xComponent, UNO_QUERY_THROW );
        Reference< XIndexAccess > xParentAsIndexAccess( xChild->getParent(), UNO_QUERY_THROW );

        // get the index of the inspected object within it's parent container
        sal_Int32 nElements = xParentAsIndexAccess->getCount();
        for ( sal_Int32 i=0; i<nElements; ++i )
        {
            Reference< XInterface > xElement( xParentAsIndexAccess->getByIndex( i ), UNO_QUERY_THROW );
            if ( xElement == m_xComponent )
                return i;
        }
        throw NoSuchElementException();
    }

    //--------------------------------------------------------------------
    namespace
    {
        ::rtl::OUString lcl_getQualifiedKnownListenerName( const ScriptEventDescriptor& _rFormComponentEventDescriptor )
        {
            EventDescription aKnownEvent;
            if ( lcl_getEventDescriptionForMethod( _rFormComponentEventDescriptor.EventMethod, aKnownEvent ) )
                return aKnownEvent.sListenerClassName;
            DBG_ERROR( "lcl_getQualifiedKnownListenerName: unknown method name!" );
                // somebody assigned an script to a form component event which we don't know
                // Speaking strictly, this is not really an error - it is possible to do
                // this programmatically -, but it should rarely happen, since it's not possible
                // via UI
            return _rFormComponentEventDescriptor.ListenerType;
        }
    }

    //--------------------------------------------------------------------
    void EventHandler::impl_getFormComponentScriptEvents_nothrow( Sequence < ScriptEventDescriptor >& _out_rEvents ) const
    {
        _out_rEvents = Sequence < ScriptEventDescriptor >();
        try
        {
            Reference< XChild > xChild( m_xComponent, UNO_QUERY_THROW );
            Reference< XEventAttacherManager > xEventManager( xChild->getParent(), UNO_QUERY_THROW );
            _out_rEvents = xEventManager->getScriptEvents( impl_getComponentIndexInParent_throw() );

            // the form component script API has unqualified listener names, but for normalization
            // purpose, we want fully qualified ones
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
            OSL_ENSURE( false, "EventHandler::impl_getFormComponentScriptEvents_nothrow: caught an exception!" );
        }
    }

    //--------------------------------------------------------------------
    namespace
    {
        typedef ::std::set< Type, TypeLessByName > TypeBag;

        //----------------------------------------------------------------
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
    }

    //--------------------------------------------------------------------
    void EventHandler::impl_getCopmonentListenerTypes_nothrow( Sequence< Type >& _out_rTypes ) const
    {
        _out_rTypes.realloc( 0 );
        try
        {
            // we use a set to avoid duplicates
            TypeBag aListeners;

            Reference< XIntrospection > xIntrospection( m_aContext.createComponent( "com.sun.star.beans.Introspection" ), UNO_QUERY_THROW );

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
            _out_rTypes.realloc( aListeners.size() );
            ::std::copy( aListeners.begin(), aListeners.end(), _out_rTypes.getArray() );
        }
        catch( const Exception& )
        {
            OSL_ENSURE( false, "EventHandler::impl_getCopmonentListenerTypes_nothrow: caught an exception!" );
        }
    }

    //--------------------------------------------------------------------
    void EventHandler::impl_getDialogElementScriptEvents_nothrow( Sequence < ScriptEventDescriptor >& _out_rEvents ) const
    {
        _out_rEvents = Sequence < ScriptEventDescriptor >();
        try
        {
            Reference< XScriptEventsSupplier > xEventsSupplier( m_xComponent, UNO_QUERY_THROW );
            Reference< XNameContainer > xEvents( xEventsSupplier->getEvents(), UNO_QUERY_THROW );
            Sequence< ::rtl::OUString > aEventNames( xEvents->getElementNames() );

            sal_Int32 nEventCount = aEventNames.getLength();
            _out_rEvents.realloc( nEventCount );

            const ::rtl::OUString* pNames = aEventNames.getConstArray();
            ScriptEventDescriptor* pDescs = _out_rEvents.getArray();

            for( sal_Int32 i = 0 ; i < nEventCount ; ++i, ++pNames, ++pDescs )
                OSL_VERIFY( xEvents->getByName( *pNames ) >>= *pDescs );
        }
        catch( const Exception& )
        {
            OSL_ENSURE( false, "EventHandler::impl_getDialogElementScriptEvents_nothrow: caught an exception!" );
        }
    }

    //--------------------------------------------------------------------
    Reference< XInterface > EventHandler::impl_getSecondaryComponentForEventInspection_throw( ) const
    {
        Reference< XInterface > xReturn;

        // if it's a form, create a form controller for the additional events
        Reference< XForm > xComponentAsForm( m_xComponent, UNO_QUERY );
        if ( xComponentAsForm.is() )
        {
            Reference< XTabControllerModel > xComponentAsTCModel( m_xComponent, UNO_QUERY_THROW );
            Reference< XFormController > xController(
                m_aContext.createComponent( (const rtl::OUString&)SERVICE_FORMCONTROLLER ), UNO_QUERY_THROW );
            xController->setModel( xComponentAsTCModel );

            xReturn = xController.get();
        }
        else
        {
            ::rtl::OUString sControlService;
            OSL_VERIFY( m_xComponent->getPropertyValue( PROPERTY_DEFAULTCONTROL ) >>= sControlService );

            xReturn = m_aContext.createComponent( sControlService );
        }
        return xReturn;
    }

    //--------------------------------------------------------------------
    const EventDescription& EventHandler::impl_getEventForName_throw( const ::rtl::OUString& _rPropertyName ) const
    {
        EventMap::const_iterator pos = m_aEvents.find( _rPropertyName );
        if ( pos == m_aEvents.end() )
            throw UnknownPropertyException();
        return pos->second;
    }

    //--------------------------------------------------------------------
    void EventHandler::impl_setFormComponentScriptEvent_nothrow( const EventDescription& _rForEvent, const ::rtl::OUString& _rScriptCode )
    {
        try
        {
            sal_Int32 nObjectIndex = impl_getComponentIndexInParent_throw();
            Reference< XChild > xChild( m_xComponent, UNO_QUERY_THROW );
            Reference< XEventAttacherManager > xEventManager( xChild->getParent(), UNO_QUERY_THROW );
            Sequence< ScriptEventDescriptor > aEvents( xEventManager->getScriptEvents( nObjectIndex ) );

            // is there already a registered script for this event?
            ScriptEventDescriptor* pEvent = aEvents.getArray();
            sal_Int32 eventCount = aEvents.getLength(), event = 0;
            for ( event = 0; event < eventCount; ++event, ++pEvent )
            {
                if ( pEvent->EventMethod == _rForEvent.sListenerMethodName )
                {
                    // yes
                    if ( _rScriptCode.getLength() )
                        // set to something non-empty -> overwrite
                        pEvent->ScriptCode = _rScriptCode;
                    else
                    {
                        // set to empty -> remove from sequence
                        ::std::copy( pEvent + 1, aEvents.getArray() + eventCount, pEvent );
                        aEvents.realloc( eventCount - 1 );
                        --eventCount;
                    }
                    break;
                }
            }
            if ( ( event >= eventCount ) && _rScriptCode.getLength() )
            {
                // no, did not find it -> append
                aEvents.realloc( eventCount + 1 );
                ScriptEventDescriptor& rEvent = aEvents[ eventCount ];

                rEvent.ListenerType = _rForEvent.sListenerClassName;
                rEvent.ScriptCode = _rScriptCode;
                rEvent.EventMethod = _rForEvent.sListenerMethodName;
                rEvent.ScriptType = ::rtl::OUString::createFromAscii( "Script" );
            }

            xEventManager->revokeScriptEvents( nObjectIndex );
            xEventManager->registerScriptEvents( nObjectIndex, aEvents );

            PropertyHandlerHelper::setContextDocumentModified( m_aContext );
        }
        catch( const Exception& )
        {
            OSL_ENSURE( false, "EventHandler::impl_setFormComponentScriptEvent_nothrow: caught an exception!" );
        }
    }

    //--------------------------------------------------------------------
    void EventHandler::impl_setDialogElementScriptEvent_nothrow( const EventDescription& _rForEvent, const ::rtl::OUString& _rScriptCode )
    {
        try
        {
            Reference< XScriptEventsSupplier > xEventsSupplier( m_xComponent, UNO_QUERY_THROW );
            Reference< XNameContainer > xEvents( xEventsSupplier->getEvents(), UNO_QUERY_THROW );

            ::rtl::OUStringBuffer aCompleteName;
            aCompleteName.append( _rForEvent.sListenerClassName );
            aCompleteName.appendAscii( "::" );
            aCompleteName.append( _rForEvent.sListenerMethodName );
            ::rtl::OUString sCompleteName( aCompleteName.makeStringAndClear() );

            bool bExists = xEvents->hasByName( sCompleteName );

            if ( !_rScriptCode.getLength() )
            {
                if ( bExists )
                    xEvents->removeByName( sCompleteName );
            }
            else
            {
                ScriptEventDescriptor aScriptEvent;
                aScriptEvent.ListenerType = _rForEvent.sListenerClassName;
                aScriptEvent.ScriptCode = _rScriptCode;
                aScriptEvent.EventMethod = _rForEvent.sListenerMethodName;
                aScriptEvent.ScriptType = ::rtl::OUString::createFromAscii( "Script" );
                Any aNewValue; aNewValue <<= aScriptEvent;

                if ( bExists )
                    xEvents->replaceByName( sCompleteName, aNewValue );
                else
                    xEvents->insertByName( sCompleteName, aNewValue );
            }
        }
        catch( const Exception& e )
        {
        #if OSL_DEBUG_LEVEL > 0
            ::rtl::OString sMessage( "EventHandler::impl_setDialogElementScriptEvent_nothrow: caught an exception!\n" );
            sMessage += "message:\n";
            sMessage += ::rtl::OString( e.Message.getStr(), e.Message.getLength(), osl_getThreadTextEncoding() );
            OSL_ENSURE( false, sMessage );
        #else
            e; // make compiler happy
        #endif
        }
    }

//........................................................................
} // namespace pcr
//........................................................................

