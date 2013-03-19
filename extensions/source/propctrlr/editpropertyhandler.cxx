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

#include "editpropertyhandler.hxx"
#include "formstrings.hxx"
#include "formmetadata.hxx"

#include <com/sun/star/inspection/XObjectInspectorUI.hpp>
#include <tools/debug.hxx>

#define TEXTTYPE_SINGLELINE     0
#define TEXTTYPE_MULTILINE      1
#define TEXTTYPE_RICHTEXT       2

//------------------------------------------------------------------------
extern "C" void SAL_CALL createRegistryInfo_EditPropertyHandler()
{
    ::pcr::EditPropertyHandler::registerImplementation();
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
    //= EditPropertyHandler
    //====================================================================
    DBG_NAME( EditPropertyHandler )
    //--------------------------------------------------------------------
    EditPropertyHandler::EditPropertyHandler( const Reference< XComponentContext >& _rxContext )
        :EditPropertyHandler_Base( _rxContext )
    {
        DBG_CTOR( EditPropertyHandler, NULL );
    }

    //--------------------------------------------------------------------
    EditPropertyHandler::~EditPropertyHandler( )
    {
        DBG_DTOR( EditPropertyHandler, NULL );
    }

    //--------------------------------------------------------------------
    ::rtl::OUString SAL_CALL EditPropertyHandler::getImplementationName_static(  ) throw (RuntimeException)
    {
        return ::rtl::OUString( "com.sun.star.comp.extensions.EditPropertyHandler" );
    }

    //--------------------------------------------------------------------
    Sequence< ::rtl::OUString > SAL_CALL EditPropertyHandler::getSupportedServiceNames_static(  ) throw (RuntimeException)
    {
        Sequence< ::rtl::OUString > aSupported( 1 );
        aSupported[0] = ::rtl::OUString( "com.sun.star.form.inspection.EditPropertyHandler" );
        return aSupported;
    }

    //--------------------------------------------------------------------
    Any SAL_CALL EditPropertyHandler::getPropertyValue( const ::rtl::OUString& _rPropertyName ) throw (UnknownPropertyException, RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        PropertyId nPropId( impl_getPropertyId_throw( _rPropertyName ) );

        Any aReturn;
        try
        {
            switch ( nPropId )
            {
            case PROPERTY_ID_SHOW_SCROLLBARS:
            {
                sal_Bool bHasVScroll = sal_False;
                m_xComponent->getPropertyValue( PROPERTY_VSCROLL ) >>= bHasVScroll;
                sal_Bool bHasHScroll = sal_False;
                m_xComponent->getPropertyValue( PROPERTY_HSCROLL ) >>= bHasHScroll;

                aReturn <<= (sal_Int32)( ( bHasVScroll ? 2 : 0 ) + ( bHasHScroll ? 1 : 0 ) );
            }
            break;

            case PROPERTY_ID_TEXTTYPE:
            {
                sal_Int32 nTextType = TEXTTYPE_SINGLELINE;
                sal_Bool bRichText = sal_False;
                OSL_VERIFY( m_xComponent->getPropertyValue( PROPERTY_RICHTEXT ) >>= bRichText );
                if ( bRichText )
                    nTextType = TEXTTYPE_RICHTEXT;
                else
                {
                    sal_Bool bMultiLine = sal_False;
                    OSL_VERIFY( m_xComponent->getPropertyValue( PROPERTY_MULTILINE ) >>= bMultiLine );
                    if ( bMultiLine )
                        nTextType = TEXTTYPE_MULTILINE;
                    else
                        nTextType = TEXTTYPE_SINGLELINE;
                }
                aReturn <<= nTextType;
            }
            break;


            default:
                OSL_FAIL( "EditPropertyHandler::getPropertyValue: cannot handle this property!" );
                break;
            }
        }
        catch( const Exception& )
        {
            OSL_FAIL( "EditPropertyHandler::getPropertyValue: caught an exception!" );
        }

        return aReturn;
    }

    //--------------------------------------------------------------------
    void SAL_CALL EditPropertyHandler::setPropertyValue( const ::rtl::OUString& _rPropertyName, const Any& _rValue ) throw (UnknownPropertyException, RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        PropertyId nPropId( impl_getPropertyId_throw( _rPropertyName ) );

        try
        {
            switch ( nPropId )
            {
            case PROPERTY_ID_SHOW_SCROLLBARS:
            {
                sal_Int32 nScrollbars = 0;
                _rValue >>= nScrollbars;

                sal_Bool bHasVScroll = 0 != ( nScrollbars & 2 );
                sal_Bool bHasHScroll = 0 != ( nScrollbars & 1 );

                m_xComponent->setPropertyValue( PROPERTY_VSCROLL, makeAny( (sal_Bool)bHasVScroll ) );
                m_xComponent->setPropertyValue( PROPERTY_HSCROLL, makeAny( (sal_Bool)bHasHScroll ) );
            }
            break;

            case PROPERTY_ID_TEXTTYPE:
            {
                sal_Bool bMultiLine = sal_False;
                sal_Bool bRichText = sal_False;
                sal_Int32 nTextType = TEXTTYPE_SINGLELINE;
                OSL_VERIFY( _rValue >>= nTextType );
                switch ( nTextType )
                {
                case TEXTTYPE_SINGLELINE: bMultiLine = bRichText = sal_False; break;
                case TEXTTYPE_MULTILINE:  bMultiLine = sal_True; bRichText = sal_False; break;
                case TEXTTYPE_RICHTEXT:   bMultiLine = sal_True; bRichText = sal_True; break;
                default:
                    OSL_FAIL( "EditPropertyHandler::setPropertyValue: invalid text type!" );
                }

                m_xComponent->setPropertyValue( PROPERTY_MULTILINE, makeAny( bMultiLine ) );
                m_xComponent->setPropertyValue( PROPERTY_RICHTEXT, makeAny( bRichText ) );
            }
            break;

            default:
                OSL_FAIL( "EditPropertyHandler::setPropertyValue: cannot handle this id!" );
            }
        }
        catch( const Exception& )
        {
            OSL_FAIL( "EditPropertyHandler::setPropertyValue: caught an exception!" );
        }
    }

    //--------------------------------------------------------------------
    bool EditPropertyHandler::implHaveBothScrollBarProperties() const
    {
        // have a "Scrollbars" property if the object supports both "HScroll" and "VScroll"
        Reference< XPropertySetInfo > xPSI;
        if ( m_xComponent.is() )
            xPSI = m_xComponent->getPropertySetInfo();

        return xPSI.is()
            && xPSI->hasPropertyByName( PROPERTY_HSCROLL )
            && xPSI->hasPropertyByName( PROPERTY_VSCROLL );
    }

    //--------------------------------------------------------------------
    bool EditPropertyHandler::implHaveTextTypeProperty() const
    {
        // have a "Scrollbars" property if the object supports both "HScroll" and "VScroll"
        Reference< XPropertySetInfo > xPSI;
        if ( m_xComponent.is() )
            xPSI = m_xComponent->getPropertySetInfo();

        return xPSI.is()
            && xPSI->hasPropertyByName( PROPERTY_RICHTEXT )
            && xPSI->hasPropertyByName( PROPERTY_MULTILINE );
    }

    //--------------------------------------------------------------------
    Sequence< Property > SAL_CALL EditPropertyHandler::doDescribeSupportedProperties() const
    {
        ::std::vector< Property > aProperties;

        if ( implHaveBothScrollBarProperties() )
            addInt32PropertyDescription( aProperties, PROPERTY_SHOW_SCROLLBARS );

        if ( implHaveTextTypeProperty() )
            addInt32PropertyDescription( aProperties, PROPERTY_TEXTTYPE );

        if ( aProperties.empty() )
            return Sequence< Property >();
        return Sequence< Property >( &(*aProperties.begin()), aProperties.size() );
    }

    //--------------------------------------------------------------------
    Sequence< ::rtl::OUString > SAL_CALL EditPropertyHandler::getSupersededProperties( ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        ::std::vector< ::rtl::OUString > aSuperseded;
        if ( implHaveBothScrollBarProperties() )
        {
            aSuperseded.push_back( static_cast<const rtl::OUString&>(PROPERTY_HSCROLL) );
            aSuperseded.push_back( static_cast<const rtl::OUString&>(PROPERTY_VSCROLL) );
        }
        if ( implHaveTextTypeProperty() )
        {
            aSuperseded.push_back(  static_cast<const rtl::OUString&>(PROPERTY_RICHTEXT) );
            aSuperseded.push_back(  static_cast<const rtl::OUString&>(PROPERTY_MULTILINE) );
        }
        if ( aSuperseded.empty() )
            return Sequence< ::rtl::OUString >();
        return Sequence< ::rtl::OUString >( &(*aSuperseded.begin()), aSuperseded.size() );
    }

    //--------------------------------------------------------------------
    Sequence< ::rtl::OUString > SAL_CALL EditPropertyHandler::getActuatingProperties( ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        ::std::vector< ::rtl::OUString > aInterestingActuatingProps;
        if ( implHaveTextTypeProperty() )
            aInterestingActuatingProps.push_back(  static_cast<const rtl::OUString&>(PROPERTY_TEXTTYPE) );
        aInterestingActuatingProps.push_back( static_cast<const rtl::OUString&>(PROPERTY_MULTILINE) );
        return Sequence< ::rtl::OUString >( &(*aInterestingActuatingProps.begin()), aInterestingActuatingProps.size() );
    }

    //--------------------------------------------------------------------
    void SAL_CALL EditPropertyHandler::actuatingPropertyChanged( const ::rtl::OUString& _rActuatingPropertyName, const Any& _rNewValue, const Any& /*_rOldValue*/, const Reference< XObjectInspectorUI >& _rxInspectorUI, sal_Bool ) throw (NullPointerException, RuntimeException)
    {
        if ( !_rxInspectorUI.is() )
            throw NullPointerException();

        ::osl::MutexGuard aGuard( m_aMutex );
        PropertyId nActuatingPropId( impl_getPropertyId_throw( _rActuatingPropertyName ) );
        switch ( nActuatingPropId )
        {
        case PROPERTY_ID_TEXTTYPE:
        {
            sal_Int32 nTextType = TEXTTYPE_SINGLELINE;
            getPropertyValue( PROPERTY_TEXTTYPE ) >>= nTextType;

            if ( impl_isSupportedProperty_nothrow( PROPERTY_ID_WORDBREAK ) )
                _rxInspectorUI->enablePropertyUI( PROPERTY_WORDBREAK,   nTextType == TEXTTYPE_RICHTEXT );
            _rxInspectorUI->enablePropertyUI( PROPERTY_MAXTEXTLEN,      nTextType != TEXTTYPE_RICHTEXT );
            _rxInspectorUI->enablePropertyUI( PROPERTY_ECHO_CHAR,       nTextType == TEXTTYPE_SINGLELINE );
            _rxInspectorUI->enablePropertyUI( PROPERTY_FONT,            nTextType != TEXTTYPE_RICHTEXT );
            _rxInspectorUI->enablePropertyUI( PROPERTY_ALIGN,           nTextType != TEXTTYPE_RICHTEXT );
            _rxInspectorUI->enablePropertyUI( PROPERTY_DEFAULT_TEXT,    nTextType != TEXTTYPE_RICHTEXT );
            _rxInspectorUI->enablePropertyUI( PROPERTY_SHOW_SCROLLBARS, nTextType != TEXTTYPE_SINGLELINE );
            _rxInspectorUI->enablePropertyUI( PROPERTY_LINEEND_FORMAT,  nTextType != TEXTTYPE_SINGLELINE );
            _rxInspectorUI->enablePropertyUI( PROPERTY_VERTICAL_ALIGN,  nTextType == TEXTTYPE_SINGLELINE );

            _rxInspectorUI->showCategory( ::rtl::OUString( "Data" ), nTextType != TEXTTYPE_RICHTEXT );
        }
        break;

        case PROPERTY_ID_MULTILINE:
        {
            sal_Bool bIsMultiline = sal_False;
            _rNewValue >>= bIsMultiline;

            _rxInspectorUI->enablePropertyUI( PROPERTY_SHOW_SCROLLBARS, bIsMultiline );
            _rxInspectorUI->enablePropertyUI( PROPERTY_ECHO_CHAR, !bIsMultiline );
            _rxInspectorUI->enablePropertyUI( PROPERTY_LINEEND_FORMAT, bIsMultiline );
        }
        break;

        default:
            OSL_FAIL( "EditPropertyHandler::actuatingPropertyChanged: cannot handle this id!" );
        }
    }

//........................................................................
}   // namespace pcr
//........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
