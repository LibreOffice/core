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


#include <sax/tools/converter.hxx>

#include <xmloff/SettingsExportHelper.hxx>
#include "xmloff/xmlnmspe.hxx"
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
#include <xmloff/XMLSettingsExportContext.hxx>
#include <xmlenums.hxx>

using namespace ::com::sun::star;
using namespace ::xmloff::token;

XMLSettingsExportHelper::XMLSettingsExportHelper( ::xmloff::XMLSettingsExportContext& i_rContext )
: m_rContext( i_rContext )
, msPrinterIndependentLayout( RTL_CONSTASCII_USTRINGPARAM( "PrinterIndependentLayout" ) )
, msColorTableURL( RTL_CONSTASCII_USTRINGPARAM( "ColorTableURL" ) )
, msLineEndTableURL( RTL_CONSTASCII_USTRINGPARAM( "LineEndTableURL" ) )
, msHatchTableURL( RTL_CONSTASCII_USTRINGPARAM( "HatchTableURL" ) )
, msDashTableURL( RTL_CONSTASCII_USTRINGPARAM( "DashTableURL" ) )
, msGradientTableURL( RTL_CONSTASCII_USTRINGPARAM( "GradientTableURL" ) )
, msBitmapTableURL( RTL_CONSTASCII_USTRINGPARAM( "BitmapTableURL" ) )
{
}

XMLSettingsExportHelper::~XMLSettingsExportHelper()
{
}

void XMLSettingsExportHelper::CallTypeFunction(const uno::Any& rAny,
                                            const rtl::OUString& rName) const
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
            rtl::OUString sString;
            aAny >>= sString;
            exportString(sString, rName);
        }
        break;
        default:
        {
            uno::Type aType = aAny.getValueType();
            if (aType.equals(getCppuType( (uno::Sequence<beans::PropertyValue> *)0 ) ) )
            {
                uno::Sequence< beans::PropertyValue> aProps;
                aAny >>= aProps;
                exportSequencePropertyValue(aProps, rName);
            }
            else if( aType.equals(getCppuType( (uno::Sequence<sal_Int8> *)0 ) ) )
            {
                uno::Sequence< sal_Int8 > aProps;
                aAny >>= aProps;
                exportbase64Binary(aProps, rName);
            }
            else if (aType.equals(getCppuType( (uno::Reference<container::XNameContainer> *)0 ) ) ||
                    aType.equals(getCppuType( (uno::Reference<container::XNameAccess> *)0 ) ))
            {
                uno::Reference< container::XNameAccess> aNamed;
                aAny >>= aNamed;
                exportNameAccess(aNamed, rName);
            }
            else if (aType.equals(getCppuType( (uno::Reference<container::XIndexAccess> *)0 ) ) ||
                    aType.equals(getCppuType( (uno::Reference<container::XIndexContainer> *)0 ) ) )
            {
                uno::Reference<container::XIndexAccess> aIndexed;
                aAny >>= aIndexed;
                exportIndexAccess(aIndexed, rName);
            }
            else if (aType.equals(getCppuType( (util::DateTime *)0 ) ) )
            {
                util::DateTime aDateTime;
                aAny >>= aDateTime;
                exportDateTime(aDateTime, rName);
            }
            else if( aType.equals(getCppuType( (uno::Reference<i18n::XForbiddenCharacters> *)0 ) ) )
            {
                exportForbiddenCharacters( aAny, rName );
            }
            else if( aType.equals(getCppuType( (uno::Sequence<formula::SymbolDescriptor> *)0 ) ) )
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

void XMLSettingsExportHelper::exportBool(const sal_Bool bValue, const rtl::OUString& rName) const
{
    DBG_ASSERT(!rName.isEmpty(), "no name");
    m_rContext.AddAttribute( XML_NAME, rName );
    m_rContext.AddAttribute( XML_TYPE, XML_BOOLEAN );
    m_rContext.StartElement( XML_CONFIG_ITEM, sal_True );
    rtl::OUString sValue;
    if (bValue)
        sValue = GetXMLToken(XML_TRUE);
    else
        sValue = GetXMLToken(XML_FALSE);
    m_rContext.Characters( sValue );
    m_rContext.EndElement( sal_False );
}

void XMLSettingsExportHelper::exportByte(const sal_Int8 nValue, const rtl::OUString& rName) const
{
#if 0
    DBG_ASSERT(!rName.isEmpty(), "no name");
    m_rContext.AddAttribute( XML_NAME, rName );
    m_rContext.AddAttribute( XML_TYPE, XML_BYTE );
    m_rContext.StartElement( XML_CONFIG_ITEM, sal_True );
    rtl::OUStringBuffer sBuffer;
	::sax::Converter::convertNumber(sBuffer, sal_Int32(nValue));
    m_rContext.Characters( sBuffer.makeStringAndClear() );
    m_rContext.EndElement( sal_False );
#else
    (void) nValue; (void) rName;
    OSL_ENSURE(false, "XMLSettingsExportHelper::exportByte(): #i114162#:\n"
        "config-items of type \"byte\" are not valid ODF, "
        "so storing them is disabled!\n"
        "Use a different type instead (e.g. \"short\").");
#endif
}
void XMLSettingsExportHelper::exportShort(const sal_Int16 nValue, const rtl::OUString& rName) const
{
    DBG_ASSERT(!rName.isEmpty(), "no name");
    m_rContext.AddAttribute( XML_NAME, rName );
    m_rContext.AddAttribute( XML_TYPE, XML_SHORT );
    m_rContext.StartElement( XML_CONFIG_ITEM, sal_True );
    rtl::OUStringBuffer sBuffer;
	::sax::Converter::convertNumber(sBuffer, sal_Int32(nValue));
    m_rContext.Characters( sBuffer.makeStringAndClear() );
    m_rContext.EndElement( sal_False );
}

void XMLSettingsExportHelper::exportInt(const sal_Int32 nValue, const rtl::OUString& rName) const
{
    DBG_ASSERT(!rName.isEmpty(), "no name");
    m_rContext.AddAttribute( XML_NAME, rName );
    m_rContext.AddAttribute( XML_TYPE, XML_INT );
    m_rContext.StartElement( XML_CONFIG_ITEM, sal_True );
    rtl::OUStringBuffer sBuffer;
	::sax::Converter::convertNumber(sBuffer, nValue);
    m_rContext.Characters( sBuffer.makeStringAndClear() );
    m_rContext.EndElement( sal_False );
}

void XMLSettingsExportHelper::exportLong(const sal_Int64 nValue, const rtl::OUString& rName) const
{
    DBG_ASSERT(!rName.isEmpty(), "no name");
    m_rContext.AddAttribute( XML_NAME, rName );
    m_rContext.AddAttribute( XML_TYPE, XML_LONG );
    m_rContext.StartElement( XML_CONFIG_ITEM, sal_True );
    rtl::OUString sValue(rtl::OUString::valueOf(nValue));
    m_rContext.Characters( sValue );
    m_rContext.EndElement( sal_False );
}

void XMLSettingsExportHelper::exportDouble(const double fValue, const rtl::OUString& rName) const
{
    DBG_ASSERT(!rName.isEmpty(), "no name");
    m_rContext.AddAttribute( XML_NAME, rName );
    m_rContext.AddAttribute( XML_TYPE, XML_DOUBLE );
    m_rContext.StartElement( XML_CONFIG_ITEM, sal_True );
    rtl::OUStringBuffer sBuffer;
	::sax::Converter::convertDouble(sBuffer, fValue);
    m_rContext.Characters( sBuffer.makeStringAndClear() );
    m_rContext.EndElement( sal_False );
}

void XMLSettingsExportHelper::exportString(const rtl::OUString& sValue, const rtl::OUString& rName) const
{
    DBG_ASSERT(!rName.isEmpty(), "no name");
    m_rContext.AddAttribute( XML_NAME, rName );
    m_rContext.AddAttribute( XML_TYPE, XML_STRING );
    m_rContext.StartElement( XML_CONFIG_ITEM, sal_True );
    if (!sValue.isEmpty())
        m_rContext.Characters( sValue );
    m_rContext.EndElement( sal_False );
}

void XMLSettingsExportHelper::exportDateTime(const util::DateTime& aValue, const rtl::OUString& rName) const
{
    DBG_ASSERT(!rName.isEmpty(), "no name");
    m_rContext.AddAttribute( XML_NAME, rName );
    m_rContext.AddAttribute( XML_TYPE, XML_DATETIME );
    rtl::OUStringBuffer sBuffer;
	::sax::Converter::convertDateTime(sBuffer, aValue);
    m_rContext.StartElement( XML_CONFIG_ITEM, sal_True );
    m_rContext.Characters( sBuffer.makeStringAndClear() );
    m_rContext.EndElement( sal_False );
}

void XMLSettingsExportHelper::exportSequencePropertyValue(
                    const uno::Sequence<beans::PropertyValue>& aProps,
                    const rtl::OUString& rName) const
{
    DBG_ASSERT(!rName.isEmpty(), "no name");
    sal_Int32 nLength(aProps.getLength());
    if(nLength)
    {
        m_rContext.AddAttribute( XML_NAME, rName );
        m_rContext.StartElement( XML_CONFIG_ITEM_SET, sal_True );
        for (sal_Int32 i = 0; i < nLength; i++)
            CallTypeFunction(aProps[i].Value, aProps[i].Name);
        m_rContext.EndElement( sal_True );
    }
}
void XMLSettingsExportHelper::exportSymbolDescriptors(
                    const uno::Sequence < formula::SymbolDescriptor > &rProps,
                    const rtl::OUString rName) const
{
    // #110680#
    uno::Reference< lang::XMultiServiceFactory > xServiceFactory( m_rContext.GetServiceFactory() );
    DBG_ASSERT( xServiceFactory.is(), "XMLSettingsExportHelper::exportSymbolDescriptors: got no service manager" );

    if( xServiceFactory.is() )
    {
        uno::Reference< container::XIndexContainer > xBox(xServiceFactory->createInstance(rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ("com.sun.star.document.IndexedPropertyValues") ) ), uno::UNO_QUERY);
        DBG_ASSERT( xBox.is(), "could not create service com.sun.star.document.IndexedPropertyValues" );
        if (xBox.is() )
        {
            const rtl::OUString sName     ( RTL_CONSTASCII_USTRINGPARAM ( "Name" ) );
            const rtl::OUString sExportName ( RTL_CONSTASCII_USTRINGPARAM ( "ExportName" ) );
            const rtl::OUString sSymbolSet ( RTL_CONSTASCII_USTRINGPARAM ( "SymbolSet" ) );
            const rtl::OUString sCharacter ( RTL_CONSTASCII_USTRINGPARAM ( "Character" ) );
            const rtl::OUString sFontName ( RTL_CONSTASCII_USTRINGPARAM ( "FontName" ) );
            const rtl::OUString sCharSet  ( RTL_CONSTASCII_USTRINGPARAM ( "CharSet" ) );
            const rtl::OUString sFamily   ( RTL_CONSTASCII_USTRINGPARAM ( "Family" ) );
            const rtl::OUString sPitch    ( RTL_CONSTASCII_USTRINGPARAM ( "Pitch" ) );
            const rtl::OUString sWeight   ( RTL_CONSTASCII_USTRINGPARAM ( "Weight" ) );
            const rtl::OUString sItalic   ( RTL_CONSTASCII_USTRINGPARAM ( "Italic" ) );

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
    }
}
void XMLSettingsExportHelper::exportbase64Binary(
                    const uno::Sequence<sal_Int8>& aProps,
                    const rtl::OUString& rName) const
{
    DBG_ASSERT(!rName.isEmpty(), "no name");
    sal_Int32 nLength(aProps.getLength());
    m_rContext.AddAttribute( XML_NAME, rName );
    m_rContext.AddAttribute( XML_TYPE, XML_BASE64BINARY );
    m_rContext.StartElement( XML_CONFIG_ITEM, sal_True );
    if(nLength)
    {
        rtl::OUStringBuffer sBuffer;
		::sax::Converter::encodeBase64(sBuffer, aProps);
        m_rContext.Characters( sBuffer.makeStringAndClear() );
    }
    m_rContext.EndElement( sal_False );
}

void XMLSettingsExportHelper::exportMapEntry(const uno::Any& rAny,
                                        const rtl::OUString& rName,
                                        const sal_Bool bNameAccess) const
{
    DBG_ASSERT((bNameAccess && !rName.isEmpty()) || !bNameAccess, "no name");
    uno::Sequence<beans::PropertyValue> aProps;
    rAny >>= aProps;
    sal_Int32 nLength = aProps.getLength();
    if (nLength)
    {
        if (bNameAccess)
            m_rContext.AddAttribute( XML_NAME, rName );
        m_rContext.StartElement( XML_CONFIG_ITEM_MAP_ENTRY, sal_True );
        for (sal_Int32 i = 0; i < nLength; i++)
            CallTypeFunction(aProps[i].Value, aProps[i].Name);
        m_rContext.EndElement( sal_True );
    }
}

void XMLSettingsExportHelper::exportNameAccess(
                    const uno::Reference<container::XNameAccess>& aNamed,
                    const rtl::OUString& rName) const
{
    DBG_ASSERT(!rName.isEmpty(), "no name");
    DBG_ASSERT(aNamed->getElementType().equals(getCppuType( (uno::Sequence<beans::PropertyValue> *)0 ) ),
                "wrong NameAccess" );
    if(aNamed->hasElements())
    {
        m_rContext.AddAttribute( XML_NAME, rName );
        m_rContext.StartElement( XML_CONFIG_ITEM_MAP_NAMED, sal_True );
        uno::Sequence< rtl::OUString > aNames(aNamed->getElementNames());
        for (sal_Int32 i = 0; i < aNames.getLength(); i++)
            exportMapEntry(aNamed->getByName(aNames[i]), aNames[i], sal_True);
        m_rContext.EndElement( sal_True );
    }
}

void XMLSettingsExportHelper::exportIndexAccess(
                    const uno::Reference<container::XIndexAccess> aIndexed,
                    const rtl::OUString rName) const
{
    DBG_ASSERT(!rName.isEmpty(), "no name");
    DBG_ASSERT(aIndexed->getElementType().equals(getCppuType( (uno::Sequence<beans::PropertyValue> *)0 ) ),
                "wrong IndexAccess" );
    rtl::OUString sEmpty;
    if(aIndexed->hasElements())
    {
        m_rContext.AddAttribute( XML_NAME, rName );
        m_rContext.StartElement( XML_CONFIG_ITEM_MAP_INDEXED, sal_True );
        sal_Int32 nCount = aIndexed->getCount();
        for (sal_Int32 i = 0; i < nCount; i++)
        {
            exportMapEntry(aIndexed->getByIndex(i), sEmpty, sal_False);
        }
        m_rContext.EndElement( sal_True );
    }
}

void XMLSettingsExportHelper::exportForbiddenCharacters(
                    const uno::Any &rAny,
                    const rtl::OUString rName) const
{
    uno::Reference<i18n::XForbiddenCharacters> xForbChars;
    uno::Reference<linguistic2::XSupportedLocales> xLocales;

    rAny >>= xForbChars;
    rAny >>= xLocales;

    DBG_ASSERT( xForbChars.is() && xLocales.is(),"XMLSettingsExportHelper::exportForbiddenCharacters: got illegal forbidden characters!" );

    if( !xForbChars.is() || !xLocales.is() )
        return;

    // #110680#
    uno::Reference< lang::XMultiServiceFactory > xServiceFactory( m_rContext.GetServiceFactory() );
    DBG_ASSERT( xServiceFactory.is(), "XMLSettingsExportHelper::exportForbiddenCharacters: got no service manager" );

    if( xServiceFactory.is() )
    {
        uno::Reference< container::XIndexContainer > xBox(xServiceFactory->createInstance(rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ("com.sun.star.document.IndexedPropertyValues") ) ), uno::UNO_QUERY);
        DBG_ASSERT( xBox.is(), "could not create service com.sun.star.document.IndexedPropertyValues" );
        if (xBox.is() )
        {
            const uno::Sequence< lang::Locale > aLocales( xLocales->getLocales() );
            const lang::Locale* pLocales = aLocales.getConstArray();

            const sal_Int32 nCount = aLocales.getLength();

            const rtl::OUString sLanguage  ( RTL_CONSTASCII_USTRINGPARAM ( "Language" ) );
            const rtl::OUString sCountry   ( RTL_CONSTASCII_USTRINGPARAM ( "Country" ) );
            const rtl::OUString sVariant   ( RTL_CONSTASCII_USTRINGPARAM ( "Variant" ) );
            const rtl::OUString sBeginLine ( RTL_CONSTASCII_USTRINGPARAM ( "BeginLine" ) );
            const rtl::OUString sEndLine   ( RTL_CONSTASCII_USTRINGPARAM ( "EndLine" ) );

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
    }
}

void XMLSettingsExportHelper::exportAllSettings(
                    const uno::Sequence<beans::PropertyValue>& aProps,
                    const rtl::OUString& rName) const
{
    DBG_ASSERT(!rName.isEmpty(), "no name");
    exportSequencePropertyValue(aProps, rName);
}


/** For some settings we may want to change their API representation
 * from their XML settings representation. This is your chance to do
 * so!
 */
void XMLSettingsExportHelper::ManipulateSetting( uno::Any& rAny, const rtl::OUString& rName ) const
{
    if( rName == msPrinterIndependentLayout )
    {
        sal_Int16 nTmp = sal_Int16();
        if( rAny >>= nTmp )
        {
            if( nTmp == document::PrinterIndependentLayout::LOW_RESOLUTION )
                rAny <<= rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("low-resolution"));
            else if( nTmp == document::PrinterIndependentLayout::DISABLED )
                rAny <<= rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("disabled"));
            else if( nTmp == document::PrinterIndependentLayout::HIGH_RESOLUTION )
                rAny <<= rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("high-resolution"));
        }
    }
    else if( (rName == msColorTableURL) || (rName == msLineEndTableURL) || (rName == msHatchTableURL) ||
             (rName == msDashTableURL) || (rName == msGradientTableURL) || (rName == msBitmapTableURL ) )
    {
        if( !mxStringSubsitution.is() )
        {
            if( m_rContext.GetServiceFactory().is() ) try
            {
                uno::Reference< uno::XComponentContext > xContext( comphelper::getComponentContext(m_rContext.GetServiceFactory()) );
                const_cast< XMLSettingsExportHelper* >(this)->mxStringSubsitution =
                    util::PathSubstitution::create(xContext);
            }
            catch( uno::Exception& )
            {
                DBG_UNHANDLED_EXCEPTION();
            }
        }

        if( mxStringSubsitution.is() )
        {
            ::rtl::OUString aURL;
            rAny >>= aURL;
            aURL = mxStringSubsitution->reSubstituteVariables( aURL );
            rAny <<= aURL;
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
