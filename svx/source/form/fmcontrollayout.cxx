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


#include "fmcontrollayout.hxx"
#include "fmprop.hrc"

#include <com/sun/star/form/FormComponentType.hpp>
#include <com/sun/star/awt/VisualEffect.hpp>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/awt/FontDescriptor.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/container/XChild.hpp>

#include <comphelper/processfactory.hxx>
#include <i18nlangtag/mslangid.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <unotools/syslocale.hxx>

#include <toolkit/helper/vclunohelper.hxx>
#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>
#include <vcl/outdev.hxx>

//........................................................................
namespace svxform
{
//........................................................................

    using namespace ::utl;
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::XInterface;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::uno::UNO_SET_THROW;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::makeAny;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::uno::Type;
    using ::com::sun::star::beans::XPropertySet;
    using ::com::sun::star::beans::XPropertySetInfo;
    using ::com::sun::star::lang::Locale;
    using ::com::sun::star::awt::FontDescriptor;
    using ::com::sun::star::style::XStyleFamiliesSupplier;
    using ::com::sun::star::lang::XServiceInfo;
    using ::com::sun::star::container::XNameAccess;
    using ::com::sun::star::container::XChild;

    namespace FormComponentType = ::com::sun::star::form::FormComponentType;
    namespace VisualEffect = ::com::sun::star::awt::VisualEffect;
    namespace ScriptType = ::com::sun::star::i18n::ScriptType;

    //--------------------------------------------------------------------
    namespace
    {
        //....................................................................
        template< class INTERFACE_TYPE >
        Reference< INTERFACE_TYPE > getTypedModelNode( const Reference< XInterface >& _rxModelNode )
        {
            Reference< INTERFACE_TYPE > xTypedNode( _rxModelNode, UNO_QUERY );
            if ( xTypedNode.is() )
                return xTypedNode;
            else
            {
                Reference< XChild > xChild( _rxModelNode, UNO_QUERY );
                if ( xChild.is() )
                    return getTypedModelNode< INTERFACE_TYPE >( xChild->getParent() );
                else
                    return NULL;
            }
        }

        //....................................................................
        static bool lcl_getDocumentDefaultStyleAndFamily( const Reference< XInterface >& _rxDocument, OUString& _rFamilyName, OUString& _rStyleName ) SAL_THROW(( Exception ))
        {
            bool bSuccess = true;
            Reference< XServiceInfo > xDocumentSI( _rxDocument, UNO_QUERY );
            if ( xDocumentSI.is() )
            {
                if (  xDocumentSI->supportsService("com.sun.star.text.TextDocument")
                   || xDocumentSI->supportsService("com.sun.star.text.WebDocument")
                   )
                {
                    _rFamilyName = OUString( "ParagraphStyles" );
                    _rStyleName = OUString( "Standard" );
                }
                else if ( xDocumentSI->supportsService("com.sun.star.sheet.SpreadsheetDocument") )
                {
                    _rFamilyName = OUString( "CellStyles" );
                    _rStyleName = OUString( "Default" );
                }
                else if (  xDocumentSI->supportsService("com.sun.star.drawing.DrawingDocument")
                        || xDocumentSI->supportsService("com.sun.star.presentation.PresentationDocument")
                        )
                {
                    _rFamilyName = OUString( "graphics" );
                    _rStyleName = OUString( "standard" );
                }
                else
                    bSuccess = false;
            }
            return bSuccess;
        }

        //....................................................................
        static void lcl_initializeControlFont( const Reference< XPropertySet >& _rxModel )
        {
            try
            {
                Reference< XPropertySet > xStyle( ControlLayouter::getDefaultDocumentTextStyle( _rxModel ), UNO_SET_THROW );
                Reference< XPropertySetInfo > xStylePSI( xStyle->getPropertySetInfo(), UNO_SET_THROW );

                // determine the script type associated with the system locale
                const SvtSysLocale aSysLocale;
                const LocaleDataWrapper& rSysLocaleData = aSysLocale.GetLocaleData();
                const sal_Int16 eSysLocaleScriptType = MsLangId::getScriptType( rSysLocaleData.getLanguageTag().getLanguageType() );

                // depending on this script type, use the right property from the document's style which controls the
                // default locale for document content
                const sal_Char* pCharLocalePropertyName = "CharLocale";
                switch ( eSysLocaleScriptType )
                {
                case ScriptType::LATIN:
                    // already defaulted above
                    break;
                case ScriptType::ASIAN:
                    pCharLocalePropertyName = "CharLocaleAsian";
                    break;
                case ScriptType::COMPLEX:
                    pCharLocalePropertyName = "CharLocaleComplex";
                    break;
                default:
                    OSL_FAIL( "lcl_initializeControlFont: unexpected script type for system locale!" );
                    break;
                }

                OUString sCharLocalePropertyName = OUString::createFromAscii( pCharLocalePropertyName );
                Locale aDocumentCharLocale;
                if ( xStylePSI->hasPropertyByName( sCharLocalePropertyName ) )
                {
                    OSL_VERIFY( xStyle->getPropertyValue( sCharLocalePropertyName ) >>= aDocumentCharLocale );
                }
                // fall back to CharLocale property at the style
                if ( aDocumentCharLocale.Language.isEmpty() )
                {
                    sCharLocalePropertyName = OUString( "CharLocale" );
                    if ( xStylePSI->hasPropertyByName( sCharLocalePropertyName ) )
                    {
                        OSL_VERIFY( xStyle->getPropertyValue( sCharLocalePropertyName ) >>= aDocumentCharLocale );
                    }
                }
                // fall back to the system locale
                if ( aDocumentCharLocale.Language.isEmpty() )
                {
                    aDocumentCharLocale = rSysLocaleData.getLanguageTag().getLocale();
                }

                // retrieve a default font for this locale, and set it at the control
                Font aFont = OutputDevice::GetDefaultFont( DEFAULTFONT_SANS, LanguageTag::convertToLanguageType( aDocumentCharLocale ), DEFAULTFONT_FLAGS_ONLYONE );
                FontDescriptor aFontDesc = VCLUnoHelper::CreateFontDescriptor( aFont );
                _rxModel->setPropertyValue(
                    OUString( "FontDescriptor" ),
                    makeAny( aFontDesc )
                );
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION();
            }
        }
    }

    //====================================================================
    //= ControlLayouter
    //====================================================================
    //--------------------------------------------------------------------
    Reference< XPropertySet > ControlLayouter::getDefaultDocumentTextStyle( const Reference< XPropertySet >& _rxModel )
    {
        // the style family collection
        Reference< XStyleFamiliesSupplier > xSuppStyleFamilies( getTypedModelNode< XStyleFamiliesSupplier >( _rxModel.get() ), UNO_SET_THROW );
        Reference< XNameAccess > xStyleFamilies( xSuppStyleFamilies->getStyleFamilies(), UNO_SET_THROW );

        // the names of the family, and the style - depends on the document type we live in
        OUString sFamilyName, sStyleName;
        if ( !lcl_getDocumentDefaultStyleAndFamily( xSuppStyleFamilies.get(), sFamilyName, sStyleName ) )
            throw RuntimeException("unknown document type!", NULL );

        // the concrete style
        Reference< XNameAccess > xStyleFamily( xStyleFamilies->getByName( sFamilyName ), UNO_QUERY_THROW );
        return Reference< XPropertySet >( xStyleFamily->getByName( sStyleName ), UNO_QUERY_THROW );
    }

    //--------------------------------------------------------------------
    void ControlLayouter::initializeControlLayout( const Reference< XPropertySet >& _rxControlModel, DocumentType _eDocType )
    {
        DBG_ASSERT( _rxControlModel.is(), "ControlLayouter::initializeControlLayout: invalid model!" );
        if ( !_rxControlModel.is() )
            return;

        try
        {
            Reference< XPropertySetInfo > xPSI( _rxControlModel->getPropertySetInfo(), UNO_SET_THROW );

            // the control type
            sal_Int16 nClassId = FormComponentType::CONTROL;
            _rxControlModel->getPropertyValue( FM_PROP_CLASSID ) >>= nClassId;

            // the document type
            if ( _eDocType == eUnknownDocumentType )
                _eDocType = DocumentClassification::classifyHostDocument( _rxControlModel.get() );

            // let's see what the configuration says about the visual effect
            OConfigurationNode  aConfig = getLayoutSettings( _eDocType );
            Any aVisualEffect = aConfig.getNodeValue( OUString( "VisualEffect" ) );
            if ( aVisualEffect.hasValue() )
            {
                OUString sVisualEffect;
                OSL_VERIFY( aVisualEffect >>= sVisualEffect );

                sal_Int16 nVisualEffect = VisualEffect::NONE;
                if ( sVisualEffect == "flat" )
                    nVisualEffect = VisualEffect::FLAT;
                else if ( sVisualEffect == "3D" )
                    nVisualEffect = VisualEffect::LOOK3D;

                if ( xPSI->hasPropertyByName( FM_PROP_BORDER ) )
                {
                    if  (  ( nClassId != FormComponentType::COMMANDBUTTON )
                        && ( nClassId != FormComponentType::RADIOBUTTON )
                        && ( nClassId != FormComponentType::CHECKBOX    )
                        && ( nClassId != FormComponentType::GROUPBOX )
                        && ( nClassId != FormComponentType::FIXEDTEXT )
                        && ( nClassId != FormComponentType::SCROLLBAR )
                        && ( nClassId != FormComponentType::SPINBUTTON )
                        )
                    {
                        _rxControlModel->setPropertyValue( FM_PROP_BORDER, makeAny( nVisualEffect ) );
                        if  (   ( nVisualEffect == VisualEffect::FLAT )
                            &&  ( xPSI->hasPropertyByName( FM_PROP_BORDERCOLOR ) )
                            )
                            // light gray flat border
                            _rxControlModel->setPropertyValue( FM_PROP_BORDERCOLOR, makeAny( (sal_Int32)0x00C0C0C0 ) );
                    }
                }
                if ( xPSI->hasPropertyByName( FM_PROP_VISUALEFFECT ) )
                    _rxControlModel->setPropertyValue( FM_PROP_VISUALEFFECT, makeAny( nVisualEffect ) );
            }

            // the font (only if we use the document's ref devices for rendering control text, otherwise, the
            // default font of VCL controls is assumed to be fine)
            if  (   useDocumentReferenceDevice( _eDocType )
                &&  xPSI->hasPropertyByName( FM_PROP_FONT )
                )
                lcl_initializeControlFont( _rxControlModel );
        }
        catch( const Exception& )
        {
            OSL_FAIL( "ControlLayouter::initializeControlLayout: caught an exception!" );
        }
    }

    //--------------------------------------------------------------------
    ::utl::OConfigurationNode ControlLayouter::getLayoutSettings( DocumentType _eDocType )
    {
        OUString sConfigName = OUString( "/org.openoffice.Office.Common/Forms/ControlLayout/" );
        sConfigName += DocumentClassification::getModuleIdentifierForDocumentType( _eDocType );
        return OConfigurationTreeRoot::createWithComponentContext(
            ::comphelper::getProcessComponentContext(),    // TODO
            sConfigName );
    }

    //--------------------------------------------------------------------
    bool ControlLayouter::useDynamicBorderColor( DocumentType _eDocType )
    {
        OConfigurationNode aConfig = getLayoutSettings( _eDocType );
        Any aDynamicBorderColor = aConfig.getNodeValue( OUString( "DynamicBorderColors" ) );
        bool bDynamicBorderColor = false;
        OSL_VERIFY( aDynamicBorderColor >>= bDynamicBorderColor );
        return bDynamicBorderColor;
    }

    //--------------------------------------------------------------------
    bool ControlLayouter::useDocumentReferenceDevice( DocumentType _eDocType )
    {
        if ( _eDocType == eUnknownDocumentType )
            return false;
        OConfigurationNode aConfig = getLayoutSettings( _eDocType );
        Any aUseRefDevice = aConfig.getNodeValue( OUString( "UseDocumentTextMetrics" ) );
        bool bUseRefDevice = false;
        OSL_VERIFY( aUseRefDevice >>= bUseRefDevice );
        return bUseRefDevice;
    }

//........................................................................
} // namespace svxform
//........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
