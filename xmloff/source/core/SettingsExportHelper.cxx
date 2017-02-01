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


#include <sax/tools/converter.hxx>

#include <xmloff/SettingsExportHelper.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmltoken.hxx>
#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>
#include <comphelper/extract.hxx>
#include <comphelper/processfactory.hxx>

#include <com/sun/star/linguistic2/XSupportedLocales.hpp>
#include <com/sun/star/i18n/XForbiddenCharacters.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/util/PathSubstitution.hpp>
#include <com/sun/star/util/DateTime.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/formula/SymbolDescriptor.hpp>
#include <com/sun/star/document/PrinterIndependentLayout.hpp>
#include <com/sun/star/document/IndexedPropertyValues.hpp>
#include <xmloff/XMLSettingsExportContext.hxx>
#include <xmlenums.hxx>

using namespace ::com::sun::star;
using namespace ::xmloff::token;

XMLSettingsExportHelper::XMLSettingsExportHelper( ::xmloff::XMLSettingsExportContext& i_rContext )
: m_rContext( i_rContext )
, msPrinterIndependentLayout( "PrinterIndependentLayout" )
, msColorTableURL( "ColorTableURL" )
, msLineEndTableURL( "LineEndTableURL" )
, msHatchTableURL( "HatchTableURL" )
, msDashTableURL( "DashTableURL" )
, msGradientTableURL( "GradientTableURL" )
, msBitmapTableURL( "BitmapTableURL" )
{
}

XMLSettingsExportHelper::~XMLSettingsExportHelper()
{
}

void XMLSettingsExportHelper::CallTypeFunction(const uno::Any& rAny,
                                            const OUString& rName) const
{
    uno::Any aAny( rAny );
    ManipulateSetting( aAny, rName );

    uno::TypeClass eClass = aAny.getValueTypeClass();
    switch (eClass)
    {
        case uno::TypeClass_VOID:
        {
            /*
             * This assertion pops up when exporting values which are set to:
             * PropertyAttribute::MAYBEVOID, and thus are _supposed_ to have
             * a VOID value...so I'm removing it ...mtg
             * OSL_FAIL("no type");
             */
        }
        break;
        case uno::TypeClass_BOOLEAN:
        {
            exportBool(::cppu::any2bool(aAny), rName);
        }
        break;
        case uno::TypeClass_BYTE:
        {
            sal_Int8 nInt8 = 0;
            aAny >>= nInt8;
            exportByte(nInt8, rName);
        }
        break;
        case uno::TypeClass_SHORT:
        {
            sal_Int16 nInt16 = 0;
            aAny >>= nInt16;
            exportShort(nInt16, rName);
        }
        break;
        case uno::TypeClass_LONG:
        {
            sal_Int32 nInt32 = 0;
            aAny >>= nInt32;
            exportInt(nInt32, rName);
        }
        break;
        case uno::TypeClass_HYPER:
        {
            sal_Int64 nInt64 = 0;
            aAny >>= nInt64;
            exportLong(nInt64, rName);
        }
        break;
        case uno::TypeClass_DOUBLE:
        {
            double fDouble = 0.0;
            aAny >>= fDouble;
            exportDouble(fDouble, rName);
        }
        break;
        case uno::TypeClass_STRING:
        {
            OUString sString;
            aAny >>= sString;
            exportString(sString, rName);
        }
        break;
        default:
        {
            uno::Type aType = aAny.getValueType();
            if (aType.equals(cppu::UnoType<uno::Sequence<beans::PropertyValue>>::get() ) )
            {
                uno::Sequence< beans::PropertyValue> aProps;
                aAny >>= aProps;
                exportSequencePropertyValue(aProps, rName);
            }
            else if( aType.equals(cppu::UnoType<uno::Sequence<sal_Int8>>::get() ) )
            {
                uno::Sequence< sal_Int8 > aProps;
                aAny >>= aProps;
                exportbase64Binary(aProps, rName);
            }
            else if (aType.equals(cppu::UnoType<container::XNameContainer>::get()) ||
                    aType.equals(cppu::UnoType<container::XNameAccess>::get()))
            {
                uno::Reference< container::XNameAccess> aNamed;
                aAny >>= aNamed;
                exportNameAccess(aNamed, rName);
            }
            else if (aType.equals(cppu::UnoType<container::XIndexAccess>::get()) ||
                    aType.equals(cppu::UnoType<container::XIndexContainer>::get()) )
            {
                uno::Reference<container::XIndexAccess> aIndexed;
                aAny >>= aIndexed;
                exportIndexAccess(aIndexed, rName);
            }
            else if (aType.equals(cppu::UnoType<util::DateTime>::get()) )
            {
                util::DateTime aDateTime;
                aAny >>= aDateTime;
                exportDateTime(aDateTime, rName);
            }
            else if( aType.equals(cppu::UnoType<i18n::XForbiddenCharacters>::get()) )
            {
                exportForbiddenCharacters( aAny, rName );
            }
            else if( aType.equals(cppu::UnoType<uno::Sequence<formula::SymbolDescriptor>>::get() ) )
            {
                uno::Sequence< formula::SymbolDescriptor > aProps;
                aAny >>= aProps;
                exportSymbolDescriptors(aProps, rName);
            }
            else {
                OSL_FAIL("this type is not implemented now");
            }
        }
        break;
    }
}

void XMLSettingsExportHelper::exportBool(const bool bValue, const OUString& rName) const
{
    DBG_ASSERT(!rName.isEmpty(), "no name");
    m_rContext.AddAttribute( XML_NAME, rName );
    m_rContext.AddAttribute( XML_TYPE, XML_BOOLEAN );
    m_rContext.StartElement( XML_CONFIG_ITEM );
    OUString sValue;
    if (bValue)
        sValue = GetXMLToken(XML_TRUE);
    else
        sValue = GetXMLToken(XML_FALSE);
    m_rContext.Characters( sValue );
    m_rContext.EndElement( false );
}

void XMLSettingsExportHelper::exportByte(const sal_Int8 nValue, const OUString& rName)
{
    (void) nValue; (void) rName;
    OSL_ENSURE(false, "XMLSettingsExportHelper::exportByte(): #i114162#:\n"
        "config-items of type \"byte\" are not valid ODF, "
        "so storing them is disabled!\n"
        "Use a different type instead (e.g. \"short\").");
}
void XMLSettingsExportHelper::exportShort(const sal_Int16 nValue, const OUString& rName) const
{
    DBG_ASSERT(!rName.isEmpty(), "no name");
    m_rContext.AddAttribute( XML_NAME, rName );
    m_rContext.AddAttribute( XML_TYPE, XML_SHORT );
    m_rContext.StartElement( XML_CONFIG_ITEM );
    m_rContext.Characters( OUString::number(nValue) );
    m_rContext.EndElement( false );
}

void XMLSettingsExportHelper::exportInt(const sal_Int32 nValue, const OUString& rName) const
{
    DBG_ASSERT(!rName.isEmpty(), "no name");
    m_rContext.AddAttribute( XML_NAME, rName );
    m_rContext.AddAttribute( XML_TYPE, XML_INT );
    m_rContext.StartElement( XML_CONFIG_ITEM );
    m_rContext.Characters( OUString::number(nValue) );
    m_rContext.EndElement( false );
}

void XMLSettingsExportHelper::exportLong(const sal_Int64 nValue, const OUString& rName) const
{
    DBG_ASSERT(!rName.isEmpty(), "no name");
    m_rContext.AddAttribute( XML_NAME, rName );
    m_rContext.AddAttribute( XML_TYPE, XML_LONG );
    m_rContext.StartElement( XML_CONFIG_ITEM );
    m_rContext.Characters( OUString::number(nValue) );
    m_rContext.EndElement( false );
}

void XMLSettingsExportHelper::exportDouble(const double fValue, const OUString& rName) const
{
    DBG_ASSERT(!rName.isEmpty(), "no name");
    m_rContext.AddAttribute( XML_NAME, rName );
    m_rContext.AddAttribute( XML_TYPE, XML_DOUBLE );
    m_rContext.StartElement( XML_CONFIG_ITEM );
    OUStringBuffer sBuffer;
    ::sax::Converter::convertDouble(sBuffer, fValue);
    m_rContext.Characters( sBuffer.makeStringAndClear() );
    m_rContext.EndElement( false );
}

void XMLSettingsExportHelper::exportString(const OUString& sValue, const OUString& rName) const
{
    DBG_ASSERT(!rName.isEmpty(), "no name");
    m_rContext.AddAttribute( XML_NAME, rName );
    m_rContext.AddAttribute( XML_TYPE, XML_STRING );
    m_rContext.StartElement( XML_CONFIG_ITEM );
    if (!sValue.isEmpty())
        m_rContext.Characters( sValue );
    m_rContext.EndElement( false );
}

void XMLSettingsExportHelper::exportDateTime(const util::DateTime& aValue, const OUString& rName) const
{
    DBG_ASSERT(!rName.isEmpty(), "no name");
    m_rContext.AddAttribute( XML_NAME, rName );
    m_rContext.AddAttribute( XML_TYPE, XML_DATETIME );
    OUStringBuffer sBuffer;
    ::sax::Converter::convertDateTime(sBuffer, aValue, nullptr);
    m_rContext.StartElement( XML_CONFIG_ITEM );
    m_rContext.Characters( sBuffer.makeStringAndClear() );
    m_rContext.EndElement( false );
}

void XMLSettingsExportHelper::exportSequencePropertyValue(
                    const uno::Sequence<beans::PropertyValue>& aProps,
                    const OUString& rName) const
{
    DBG_ASSERT(!rName.isEmpty(), "no name");
    sal_Int32 nLength(aProps.getLength());
    if(nLength)
    {
        m_rContext.AddAttribute( XML_NAME, rName );
        m_rContext.StartElement( XML_CONFIG_ITEM_SET );
        for (sal_Int32 i = 0; i < nLength; i++)
            CallTypeFunction(aProps[i].Value, aProps[i].Name);
        m_rContext.EndElement( true );
    }
}
void XMLSettingsExportHelper::exportSymbolDescriptors(
                    const uno::Sequence < formula::SymbolDescriptor > &rProps,
                    const OUString& rName) const
{
    uno::Reference< container::XIndexContainer > xBox = document::IndexedPropertyValues::create(m_rContext.GetComponentContext());

    const OUString sName     ( "Name" );
    const OUString sExportName ( "ExportName" );
    const OUString sSymbolSet ( "SymbolSet" );
    const OUString sCharacter ( "Character" );
    const OUString sFontName ( "FontName" );
    const OUString sCharSet  ( "CharSet" );
    const OUString sFamily   ( "Family" );
    const OUString sPitch    ( "Pitch" );
    const OUString sWeight   ( "Weight" );
    const OUString sItalic   ( "Italic" );

    sal_Int32 nCount = rProps.getLength();
    const formula::SymbolDescriptor *pDescriptor = rProps.getConstArray();

    for( sal_Int32 nIndex = 0; nIndex < nCount; nIndex++, pDescriptor++ )
    {
        uno::Sequence < beans::PropertyValue > aSequence ( XML_SYMBOL_DESCRIPTOR_MAX );
        beans::PropertyValue *pSymbol = aSequence.getArray();

        pSymbol[XML_SYMBOL_DESCRIPTOR_NAME].Name         = sName;
        pSymbol[XML_SYMBOL_DESCRIPTOR_NAME].Value       <<= pDescriptor->sName;
        pSymbol[XML_SYMBOL_DESCRIPTOR_EXPORT_NAME].Name  = sExportName;
        pSymbol[XML_SYMBOL_DESCRIPTOR_EXPORT_NAME].Value<<= pDescriptor->sExportName;
        pSymbol[XML_SYMBOL_DESCRIPTOR_FONT_NAME].Name    = sFontName;
        pSymbol[XML_SYMBOL_DESCRIPTOR_FONT_NAME].Value  <<= pDescriptor->sFontName;
        pSymbol[XML_SYMBOL_DESCRIPTOR_CHAR_SET].Name      = sCharSet;
        pSymbol[XML_SYMBOL_DESCRIPTOR_CHAR_SET].Value   <<= pDescriptor->nCharSet;
        pSymbol[XML_SYMBOL_DESCRIPTOR_FAMILY].Name       = sFamily;
        pSymbol[XML_SYMBOL_DESCRIPTOR_FAMILY].Value <<= pDescriptor->nFamily;
        pSymbol[XML_SYMBOL_DESCRIPTOR_PITCH].Name        = sPitch;
        pSymbol[XML_SYMBOL_DESCRIPTOR_PITCH].Value      <<= pDescriptor->nPitch;
        pSymbol[XML_SYMBOL_DESCRIPTOR_WEIGHT].Name       = sWeight;
        pSymbol[XML_SYMBOL_DESCRIPTOR_WEIGHT].Value <<= pDescriptor->nWeight;
        pSymbol[XML_SYMBOL_DESCRIPTOR_ITALIC].Name       = sItalic;
        pSymbol[XML_SYMBOL_DESCRIPTOR_ITALIC].Value <<= pDescriptor->nItalic;
        pSymbol[XML_SYMBOL_DESCRIPTOR_SYMBOL_SET].Name       = sSymbolSet;
        pSymbol[XML_SYMBOL_DESCRIPTOR_SYMBOL_SET].Value <<= pDescriptor->sSymbolSet;
        pSymbol[XML_SYMBOL_DESCRIPTOR_CHARACTER].Name       = sCharacter;
        pSymbol[XML_SYMBOL_DESCRIPTOR_CHARACTER].Value  <<= pDescriptor->nCharacter;

        xBox->insertByIndex(nIndex, uno::makeAny( aSequence ));
    }

    uno::Reference< container::XIndexAccess > xIA( xBox, uno::UNO_QUERY );
    exportIndexAccess( xIA, rName );
}
void XMLSettingsExportHelper::exportbase64Binary(
                    const uno::Sequence<sal_Int8>& aProps,
                    const OUString& rName) const
{
    DBG_ASSERT(!rName.isEmpty(), "no name");
    sal_Int32 nLength(aProps.getLength());
    m_rContext.AddAttribute( XML_NAME, rName );
    m_rContext.AddAttribute( XML_TYPE, XML_BASE64BINARY );
    m_rContext.StartElement( XML_CONFIG_ITEM );
    if(nLength)
    {
        OUStringBuffer sBuffer;
        ::sax::Converter::encodeBase64(sBuffer, aProps);
        m_rContext.Characters( sBuffer.makeStringAndClear() );
    }
    m_rContext.EndElement( false );
}

void XMLSettingsExportHelper::exportMapEntry(const uno::Any& rAny,
                                        const OUString& rName,
                                        const bool bNameAccess) const
{
    DBG_ASSERT((bNameAccess && !rName.isEmpty()) || !bNameAccess, "no name");
    uno::Sequence<beans::PropertyValue> aProps;
    rAny >>= aProps;
    sal_Int32 nLength = aProps.getLength();
    if (nLength)
    {
        if (bNameAccess)
            m_rContext.AddAttribute( XML_NAME, rName );
        m_rContext.StartElement( XML_CONFIG_ITEM_MAP_ENTRY );
        for (sal_Int32 i = 0; i < nLength; i++)
            CallTypeFunction(aProps[i].Value, aProps[i].Name);
        m_rContext.EndElement( true );
    }
}

void XMLSettingsExportHelper::exportNameAccess(
                    const uno::Reference<container::XNameAccess>& aNamed,
                    const OUString& rName) const
{
    DBG_ASSERT(!rName.isEmpty(), "no name");
    DBG_ASSERT(aNamed->getElementType().equals(cppu::UnoType<uno::Sequence<beans::PropertyValue>>::get() ),
                "wrong NameAccess" );
    if(aNamed->hasElements())
    {
        m_rContext.AddAttribute( XML_NAME, rName );
        m_rContext.StartElement( XML_CONFIG_ITEM_MAP_NAMED );
        uno::Sequence< OUString > aNames(aNamed->getElementNames());
        for (sal_Int32 i = 0; i < aNames.getLength(); i++)
            exportMapEntry(aNamed->getByName(aNames[i]), aNames[i], true);
        m_rContext.EndElement( true );
    }
}

void XMLSettingsExportHelper::exportIndexAccess(
                    const uno::Reference<container::XIndexAccess>& rIndexed,
                    const OUString& rName) const
{
    DBG_ASSERT(!rName.isEmpty(), "no name");
    DBG_ASSERT(rIndexed->getElementType().equals(cppu::UnoType<uno::Sequence<beans::PropertyValue>>::get() ),
                "wrong IndexAccess" );
    if (rIndexed->hasElements())
    {
        m_rContext.AddAttribute( XML_NAME, rName );
        m_rContext.StartElement( XML_CONFIG_ITEM_MAP_INDEXED );
        sal_Int32 nCount = rIndexed->getCount();
        for (sal_Int32 i = 0; i < nCount; i++)
        {
            exportMapEntry(rIndexed->getByIndex(i), "", false);
        }
        m_rContext.EndElement( true );
    }
}

void XMLSettingsExportHelper::exportForbiddenCharacters(
                    const uno::Any &rAny,
                    const OUString& rName) const
{
    uno::Reference<i18n::XForbiddenCharacters> xForbChars;
    uno::Reference<linguistic2::XSupportedLocales> xLocales;

    rAny >>= xForbChars;
    rAny >>= xLocales;

    SAL_WARN_IF( !(xForbChars.is() && xLocales.is()), "xmloff","XMLSettingsExportHelper::exportForbiddenCharacters: got illegal forbidden characters!" );

    if( !xForbChars.is() || !xLocales.is() )
        return;

    uno::Reference< container::XIndexContainer > xBox = document::IndexedPropertyValues::create(m_rContext.GetComponentContext());
    const uno::Sequence< lang::Locale > aLocales( xLocales->getLocales() );
    const lang::Locale* pLocales = aLocales.getConstArray();

    const sal_Int32 nCount = aLocales.getLength();

    /* FIXME-BCP47: this stupid and counterpart in
     * xmloff/source/core/DocumentSettingsContext.cxx
     * XMLConfigItemMapIndexedContext::EndElement() */

    const OUString sLanguage  ( "Language" );
    const OUString sCountry   ( "Country" );
    const OUString sVariant   ( "Variant" );
    const OUString sBeginLine ( "BeginLine" );
    const OUString sEndLine   ( "EndLine" );

    sal_Int32 nPos = 0;
    for( sal_Int32 nIndex = 0; nIndex < nCount; nIndex++, pLocales++ )
    {
        if( xForbChars->hasForbiddenCharacters( *pLocales ) )
        {
            const i18n::ForbiddenCharacters aChars( xForbChars->getForbiddenCharacters( *pLocales ) );


            uno::Sequence < beans::PropertyValue > aSequence ( XML_FORBIDDEN_CHARACTER_MAX );
            beans::PropertyValue *pForChar = aSequence.getArray();

            pForChar[XML_FORBIDDEN_CHARACTER_LANGUAGE].Name    = sLanguage;
            pForChar[XML_FORBIDDEN_CHARACTER_LANGUAGE].Value <<= pLocales->Language;
            pForChar[XML_FORBIDDEN_CHARACTER_COUNTRY].Name    = sCountry;
            pForChar[XML_FORBIDDEN_CHARACTER_COUNTRY].Value <<= pLocales->Country;
            pForChar[XML_FORBIDDEN_CHARACTER_VARIANT].Name    = sVariant;
            pForChar[XML_FORBIDDEN_CHARACTER_VARIANT].Value <<= pLocales->Variant;
            pForChar[XML_FORBIDDEN_CHARACTER_BEGIN_LINE].Name    = sBeginLine;
            pForChar[XML_FORBIDDEN_CHARACTER_BEGIN_LINE].Value <<= aChars.beginLine;
            pForChar[XML_FORBIDDEN_CHARACTER_END_LINE].Name    = sEndLine;
            pForChar[XML_FORBIDDEN_CHARACTER_END_LINE].Value <<= aChars.endLine;
            xBox->insertByIndex(nPos++, uno::makeAny( aSequence ));
        }
    }

    uno::Reference< container::XIndexAccess > xIA( xBox, uno::UNO_QUERY );
    exportIndexAccess( xIA, rName );
}

void XMLSettingsExportHelper::exportAllSettings(
                    const uno::Sequence<beans::PropertyValue>& aProps,
                    const OUString& rName) const
{
    DBG_ASSERT(!rName.isEmpty(), "no name");
    exportSequencePropertyValue(aProps, rName);
}


/** For some settings we may want to change their API representation
 * from their XML settings representation. This is your chance to do
 * so!
 */
void XMLSettingsExportHelper::ManipulateSetting( uno::Any& rAny, const OUString& rName ) const
{
    if( rName == msPrinterIndependentLayout )
    {
        sal_Int16 nTmp = sal_Int16();
        if( rAny >>= nTmp )
        {
            if( nTmp == document::PrinterIndependentLayout::LOW_RESOLUTION )
                rAny <<= OUString("low-resolution");
            else if( nTmp == document::PrinterIndependentLayout::DISABLED )
                rAny <<= OUString("disabled");
            else if( nTmp == document::PrinterIndependentLayout::HIGH_RESOLUTION )
                rAny <<= OUString("high-resolution");
        }
    }
    else if( (rName == msColorTableURL) || (rName == msLineEndTableURL) || (rName == msHatchTableURL) ||
             (rName == msDashTableURL) || (rName == msGradientTableURL) || (rName == msBitmapTableURL ) )
    {
        if( !mxStringSubsitution.is() )
        {
            try
            {
                const_cast< XMLSettingsExportHelper* >(this)->mxStringSubsitution =
                    util::PathSubstitution::create( m_rContext.GetComponentContext() );
            }
            catch( uno::Exception& )
            {
                DBG_UNHANDLED_EXCEPTION();
            }
        }

        if( mxStringSubsitution.is() )
        {
            OUString aURL;
            rAny >>= aURL;
            aURL = mxStringSubsitution->reSubstituteVariables( aURL );
            rAny <<= aURL;
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
