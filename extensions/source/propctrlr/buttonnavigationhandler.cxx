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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_extensions.hxx"
#include "buttonnavigationhandler.hxx"
#include "formstrings.hxx"
#include "formmetadata.hxx"
#include "pushbuttonnavigation.hxx"

/** === begin UNO includes === **/
/** === end UNO includes === **/
#include <tools/debug.hxx>

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
            OSL_FAIL( "ButtonNavigationHandler::getPropertyState: cannot handle this property!" );
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
            OSL_FAIL( "ButtonNavigationHandler::getPropertyValue: cannot handle this property!" );
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
            OSL_FAIL( "ButtonNavigationHandler::setPropertyValue: cannot handle this id!" );
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
            OSL_FAIL( "ButtonNavigationHandler::actuatingPropertyChanged: cannot handle this id!" );
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
