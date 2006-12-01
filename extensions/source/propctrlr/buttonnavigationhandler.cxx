/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: buttonnavigationhandler.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: rt $ $Date: 2006-12-01 17:35:08 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_extensions.hxx"

#ifndef EXTENSIONS_SOURCE_PROPCTRLR_BUTTONNAVIGATIONHANDLER_HXX
#include "buttonnavigationhandler.hxx"
#endif

#ifndef _EXTENSIONS_FORMSCTRLR_FORMSTRINGS_HXX_
#include "formstrings.hxx"
#endif
#ifndef _EXTENSIONS_PROPCTRLR_FORMMETADATA_HXX_
#include "formmetadata.hxx"
#endif
#ifndef EXTENSIONS_PROPCTRLR_PUSHBUTTONNAVIGATION_HXX
#include "pushbuttonnavigation.hxx"
#endif

/** === begin UNO includes === **/
/** === end UNO includes === **/

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

//------------------------------------------------------------------------
extern "C" void SAL_CALL createRegistryInfo_ButtonNavigationHandler()
{
    ::pcr::ButtonNavigationHandler::registerImplementation();
}

//........................................................................
namespace pcr
{
//........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::script;
    using namespace ::com::sun::star::frame;
    using namespace ::com::sun::star::inspection;

    //====================================================================
    //= ButtonNavigationHandler
    //====================================================================
    DBG_NAME( ButtonNavigationHandler )
    //--------------------------------------------------------------------
    ButtonNavigationHandler::ButtonNavigationHandler( const Reference< XComponentContext >& _rxContext )
        :ButtonNavigationHandler_Base( _rxContext )
    {
        DBG_CTOR( ButtonNavigationHandler, NULL );

        m_aContext.createComponent(
            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.form.inspection.FormComponentPropertyHandler" ) ),
            m_xSlaveHandler );
        if ( !m_xSlaveHandler.is() )
            throw RuntimeException();
    }

    //--------------------------------------------------------------------
    ButtonNavigationHandler::~ButtonNavigationHandler( )
    {
        DBG_DTOR( ButtonNavigationHandler, NULL );
    }

    //--------------------------------------------------------------------
    ::rtl::OUString SAL_CALL ButtonNavigationHandler::getImplementationName_static(  ) throw (RuntimeException)
    {
        return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.extensions.ButtonNavigationHandler" ) );
    }

    //--------------------------------------------------------------------
    Sequence< ::rtl::OUString > SAL_CALL ButtonNavigationHandler::getSupportedServiceNames_static(  ) throw (RuntimeException)
    {
        Sequence< ::rtl::OUString > aSupported( 1 );
        aSupported[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.form.inspection.ButtonNavigationHandler" ) );
        return aSupported;
    }

    //--------------------------------------------------------------------
    void SAL_CALL ButtonNavigationHandler::inspect( const Reference< XInterface >& _rxIntrospectee ) throw (RuntimeException, NullPointerException)
    {
        ButtonNavigationHandler_Base::inspect( _rxIntrospectee );
        m_xSlaveHandler->inspect( _rxIntrospectee );
    }

    //--------------------------------------------------------------------
    PropertyState  SAL_CALL ButtonNavigationHandler::getPropertyState( const ::rtl::OUString& _rPropertyName ) throw (UnknownPropertyException, RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        PropertyId nPropId( impl_getPropertyId_throw( _rPropertyName ) );
        PropertyState eState = PropertyState_DIRECT_VALUE;
        switch ( nPropId )
        {
        case PROPERTY_ID_BUTTONTYPE:
        {
            PushButtonNavigation aHelper( m_xComponent );
            eState = aHelper.getCurrentButtonTypeState();
        }
        break;
        case PROPERTY_ID_TARGET_URL:
        {
            PushButtonNavigation aHelper( m_xComponent );
            eState = aHelper.getCurrentTargetURLState();
        }
        break;

        default:
            DBG_ERROR( "ButtonNavigationHandler::getPropertyState: cannot handle this property!" );
            break;
        }

        return eState;
    }

    //--------------------------------------------------------------------
    Any SAL_CALL ButtonNavigationHandler::getPropertyValue( const ::rtl::OUString& _rPropertyName ) throw (UnknownPropertyException, RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        PropertyId nPropId( impl_getPropertyId_throw( _rPropertyName ) );

        Any aReturn;
        switch ( nPropId )
        {
        case PROPERTY_ID_BUTTONTYPE:
        {
            PushButtonNavigation aHelper( m_xComponent );
            aReturn = aHelper.getCurrentButtonType();
        }
        break;

        case PROPERTY_ID_TARGET_URL:
        {
            PushButtonNavigation aHelper( m_xComponent );
            aReturn = aHelper.getCurrentTargetURL();
        }
        break;

        default:
            DBG_ERROR( "ButtonNavigationHandler::getPropertyValue: cannot handle this property!" );
            break;
        }

        return aReturn;
    }

    //--------------------------------------------------------------------
    void SAL_CALL ButtonNavigationHandler::setPropertyValue( const ::rtl::OUString& _rPropertyName, const Any& _rValue ) throw (UnknownPropertyException, RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        PropertyId nPropId( impl_getPropertyId_throw( _rPropertyName ) );
        switch ( nPropId )
        {
        case PROPERTY_ID_BUTTONTYPE:
        {
            PushButtonNavigation aHelper( m_xComponent );
            aHelper.setCurrentButtonType( _rValue );
        }
        break;

        case PROPERTY_ID_TARGET_URL:
        {
            PushButtonNavigation aHelper( m_xComponent );
            aHelper.setCurrentTargetURL( _rValue );
        }
        break;

        default:
            OSL_ENSURE( sal_False, "ButtonNavigationHandler::setPropertyValue: cannot handle this id!" );
        }
    }

    //--------------------------------------------------------------------
    bool ButtonNavigationHandler::isNavigationCapableButton( const Reference< XPropertySet >& _rxComponent )
    {
        Reference< XPropertySetInfo > xPSI;
        if ( _rxComponent.is() )
            xPSI = _rxComponent->getPropertySetInfo();

        return xPSI.is()
            && xPSI->hasPropertyByName( PROPERTY_TARGET_URL )
            && xPSI->hasPropertyByName( PROPERTY_BUTTONTYPE );
    }

    //--------------------------------------------------------------------
    Sequence< Property > SAL_CALL ButtonNavigationHandler::doDescribeSupportedProperties() const
    {
        ::std::vector< Property > aProperties;

        if ( isNavigationCapableButton( m_xComponent ) )
        {
            addStringPropertyDescription( aProperties, PROPERTY_TARGET_URL );
            implAddPropertyDescription( aProperties, PROPERTY_BUTTONTYPE, ::getCppuType( static_cast< sal_Int32* >( NULL ) ) );
        }

        if ( aProperties.empty() )
            return Sequence< Property >();
        return Sequence< Property >( &(*aProperties.begin()), aProperties.size() );
    }

    //--------------------------------------------------------------------
    Sequence< ::rtl::OUString > SAL_CALL ButtonNavigationHandler::getActuatingProperties( ) throw (RuntimeException)
    {
        Sequence< ::rtl::OUString > aActuating( 2 );
        aActuating[0] = PROPERTY_BUTTONTYPE;
        aActuating[1] = PROPERTY_TARGET_URL;
        return aActuating;
    }

    //--------------------------------------------------------------------
    InteractiveSelectionResult SAL_CALL ButtonNavigationHandler::onInteractivePropertySelection( const ::rtl::OUString& _rPropertyName, sal_Bool _bPrimary, Any& _rData, const Reference< XObjectInspectorUI >& _rxInspectorUI ) throw (UnknownPropertyException, NullPointerException, RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        PropertyId nPropId( impl_getPropertyId_throw( _rPropertyName ) );

        InteractiveSelectionResult eReturn( InteractiveSelectionResult_Cancelled );

        switch ( nPropId )
        {
        case PROPERTY_ID_TARGET_URL:
            eReturn = m_xSlaveHandler->onInteractivePropertySelection( _rPropertyName, _bPrimary, _rData, _rxInspectorUI );
            break;
        default:
            eReturn = ButtonNavigationHandler_Base::onInteractivePropertySelection( _rPropertyName, _bPrimary, _rData, _rxInspectorUI );
            break;
        }

        return eReturn;
    }

    //--------------------------------------------------------------------
    void SAL_CALL ButtonNavigationHandler::actuatingPropertyChanged( const ::rtl::OUString& _rActuatingPropertyName, const Any& /*_rNewValue*/, const Any& /*_rOldValue*/, const Reference< XObjectInspectorUI >& _rxInspectorUI, sal_Bool /*_bFirstTimeInit*/ ) throw (NullPointerException, RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        PropertyId nPropId( impl_getPropertyId_throw( _rActuatingPropertyName ) );
        switch ( nPropId )
        {
        case PROPERTY_ID_BUTTONTYPE:
        {
            PushButtonNavigation aHelper( m_xComponent );
            _rxInspectorUI->enablePropertyUI( PROPERTY_TARGET_URL, aHelper.currentButtonTypeIsOpenURL() );
        }
        break;

        case PROPERTY_ID_TARGET_URL:
        {
            PushButtonNavigation aHelper( m_xComponent );
            _rxInspectorUI->enablePropertyUI( PROPERTY_TARGET_FRAME, aHelper.hasNonEmptyCurrentTargetURL() );
        }
        break;

        default:
            OSL_ENSURE( sal_False, "ButtonNavigationHandler::actuatingPropertyChanged: cannot handle this id!" );
        }
    }

    //--------------------------------------------------------------------
    LineDescriptor SAL_CALL ButtonNavigationHandler::describePropertyLine( const ::rtl::OUString& _rPropertyName, const Reference< XPropertyControlFactory >& _rxControlFactory ) throw (UnknownPropertyException, NullPointerException, RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        PropertyId nPropId( impl_getPropertyId_throw( _rPropertyName ) );

        LineDescriptor aReturn;

        switch ( nPropId )
        {
        case PROPERTY_ID_TARGET_URL:
            aReturn = m_xSlaveHandler->describePropertyLine( _rPropertyName, _rxControlFactory );
            break;
        default:
            aReturn = ButtonNavigationHandler_Base::describePropertyLine( _rPropertyName, _rxControlFactory );
            break;
        }

        return aReturn;
    }

//........................................................................
}   // namespace pcr
//........................................................................

