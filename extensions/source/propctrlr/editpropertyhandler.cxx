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
#include "pcrservices.hxx"

#include <com/sun/star/inspection/XObjectInspectorUI.hpp>
#include <com/sun/star/lang/NullPointerException.hpp>

#define TEXTTYPE_SINGLELINE     0
#define TEXTTYPE_MULTILINE      1
#define TEXTTYPE_RICHTEXT       2


extern "C" void createRegistryInfo_EditPropertyHandler()
{
    ::pcr::EditPropertyHandler::registerImplementation();
}


namespace pcr
{


    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::script;
    using namespace ::com::sun::star::frame;
    using namespace ::com::sun::star::inspection;


    //= EditPropertyHandler


    EditPropertyHandler::EditPropertyHandler( const Reference< XComponentContext >& _rxContext )
        :EditPropertyHandler_Base( _rxContext )
    {
    }


    EditPropertyHandler::~EditPropertyHandler( )
    {
    }


    OUString EditPropertyHandler::getImplementationName_static(  )
    {
        return OUString( "com.sun.star.comp.extensions.EditPropertyHandler" );
    }


    Sequence< OUString > EditPropertyHandler::getSupportedServiceNames_static(  )
    {
        Sequence<OUString> aSupported { "com.sun.star.form.inspection.EditPropertyHandler" };
        return aSupported;
    }


    Any SAL_CALL EditPropertyHandler::getPropertyValue( const OUString& _rPropertyName )
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        PropertyId nPropId( impl_getPropertyId_throwUnknownProperty( _rPropertyName ) );

        Any aReturn;
        try
        {
            switch ( nPropId )
            {
            case PROPERTY_ID_SHOW_SCROLLBARS:
            {
                bool bHasVScroll = false;
                m_xComponent->getPropertyValue( PROPERTY_VSCROLL ) >>= bHasVScroll;
                bool bHasHScroll = false;
                m_xComponent->getPropertyValue( PROPERTY_HSCROLL ) >>= bHasHScroll;

                aReturn <<= static_cast<sal_Int32>( ( bHasVScroll ? 2 : 0 ) + ( bHasHScroll ? 1 : 0 ) );
            }
            break;

            case PROPERTY_ID_TEXTTYPE:
            {
                sal_Int32 nTextType = TEXTTYPE_SINGLELINE;
                bool bRichText = false;
                OSL_VERIFY( m_xComponent->getPropertyValue( PROPERTY_RICHTEXT ) >>= bRichText );
                if ( bRichText )
                    nTextType = TEXTTYPE_RICHTEXT;
                else
                {
                    bool bMultiLine = false;
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


    void SAL_CALL EditPropertyHandler::setPropertyValue( const OUString& _rPropertyName, const Any& _rValue )
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        PropertyId nPropId( impl_getPropertyId_throwUnknownProperty( _rPropertyName ) );

        try
        {
            switch ( nPropId )
            {
            case PROPERTY_ID_SHOW_SCROLLBARS:
            {
                sal_Int32 nScrollbars = 0;
                _rValue >>= nScrollbars;

                bool bHasVScroll = 0 != ( nScrollbars & 2 );
                bool bHasHScroll = 0 != ( nScrollbars & 1 );

                m_xComponent->setPropertyValue( PROPERTY_VSCROLL, makeAny( bHasVScroll ) );
                m_xComponent->setPropertyValue( PROPERTY_HSCROLL, makeAny( bHasHScroll ) );
            }
            break;

            case PROPERTY_ID_TEXTTYPE:
            {
                bool bMultiLine = false;
                bool bRichText = false;
                sal_Int32 nTextType = TEXTTYPE_SINGLELINE;
                OSL_VERIFY( _rValue >>= nTextType );
                switch ( nTextType )
                {
                case TEXTTYPE_SINGLELINE: bMultiLine = bRichText = false; break;
                case TEXTTYPE_MULTILINE:  bMultiLine = true; bRichText = false; break;
                case TEXTTYPE_RICHTEXT:   bMultiLine = true; bRichText = true; break;
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


    Sequence< Property > EditPropertyHandler::doDescribeSupportedProperties() const
    {
        std::vector< Property > aProperties;

        if ( implHaveBothScrollBarProperties() )
            addInt32PropertyDescription( aProperties, PROPERTY_SHOW_SCROLLBARS );

        if ( implHaveTextTypeProperty() )
            addInt32PropertyDescription( aProperties, PROPERTY_TEXTTYPE );

        if ( aProperties.empty() )
            return Sequence< Property >();
        return comphelper::containerToSequence(aProperties);
    }


    Sequence< OUString > SAL_CALL EditPropertyHandler::getSupersededProperties( )
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        std::vector< OUString > aSuperseded;
        if ( implHaveBothScrollBarProperties() )
        {
            aSuperseded.push_back( PROPERTY_HSCROLL );
            aSuperseded.push_back( PROPERTY_VSCROLL );
        }
        if ( implHaveTextTypeProperty() )
        {
            aSuperseded.push_back(  PROPERTY_RICHTEXT );
            aSuperseded.push_back(  PROPERTY_MULTILINE );
        }
        if ( aSuperseded.empty() )
            return Sequence< OUString >();
        return comphelper::containerToSequence(aSuperseded);
    }


    Sequence< OUString > SAL_CALL EditPropertyHandler::getActuatingProperties( )
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        std::vector< OUString > aInterestingActuatingProps;
        if ( implHaveTextTypeProperty() )
            aInterestingActuatingProps.push_back(  PROPERTY_TEXTTYPE );
        aInterestingActuatingProps.push_back( PROPERTY_MULTILINE );
        return comphelper::containerToSequence(aInterestingActuatingProps);
    }


    void SAL_CALL EditPropertyHandler::actuatingPropertyChanged( const OUString& _rActuatingPropertyName, const Any& _rNewValue, const Any& /*_rOldValue*/, const Reference< XObjectInspectorUI >& _rxInspectorUI, sal_Bool )
    {
        if ( !_rxInspectorUI.is() )
            throw NullPointerException();

        ::osl::MutexGuard aGuard( m_aMutex );
        PropertyId nActuatingPropId( impl_getPropertyId_throwRuntime( _rActuatingPropertyName ) );
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

            _rxInspectorUI->showCategory( "Data", nTextType != TEXTTYPE_RICHTEXT );
        }
        break;

        case PROPERTY_ID_MULTILINE:
        {
            bool bIsMultiline = false;
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


}   // namespace pcr


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
