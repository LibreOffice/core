/*************************************************************************
 *
 *  $RCSfile: SettingsExportHelper.cxx,v $
 *
 *  $Revision: 1.22 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 14:12:25 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _XMLOFF_SETTINGSEXPORTHELPER_HXX
#include "SettingsExportHelper.hxx"
#endif

#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif
#ifndef _XMLOFF_XMLTOKEN_HXX
#include "xmltoken.hxx"
#endif
#ifndef _XMLOFF_XMLUCONV_HXX
#include "xmluconv.hxx"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif

// #110680#
//#ifndef _COMPHELPER_PROCESSFACTORYHXX_
//#include <comphelper/processfactory.hxx>
//#endif

#ifndef _COM_SUN_STAR_LINGUISTIC2_XSUPPORTEDLOCALES_HPP_
#include <com/sun/star/linguistic2/XSupportedLocales.hpp>
#endif
#ifndef _COM_SUN_STAR_I18N_XFORBIDDENCHARACTERS_HPP_
#include <com/sun/star/i18n/XForbiddenCharacters.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XIndexCONTAINER_HPP_
#include <com/sun/star/container/XIndexContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_DATETIME_HPP_
#include <com/sun/star/util/DateTime.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XINDEXCONTAINER_HPP_
#include <com/sun/star/container/XIndexContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_FORMULA_SYMBOLDESCRIPTOR_HPP_
#include <com/sun/star/formula/SymbolDescriptor.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_PRINTERINDEPENDENTLAYOUT_HPP_
#include <com/sun/star/document/PrinterIndependentLayout.hpp>
#endif
#ifndef _XMLOFF_NMSPMAP_HXX
#include "nmspmap.hxx"
#endif
#ifndef _XMLENUMS_HXX_
#include <xmlenums.hxx>
#endif

using namespace ::com::sun::star;
using namespace ::xmloff::token;

XMLSettingsExportHelper::XMLSettingsExportHelper(SvXMLExport& rTempExport)
    : rExport(rTempExport)
{
}

XMLSettingsExportHelper::~XMLSettingsExportHelper()
{
}


/** function to allow changing the API and XML representation;
 implementation below */
void lcl_manipulateSetting( uno::Any&, const rtl::OUString& );


void XMLSettingsExportHelper::CallTypeFunction(const uno::Any& rAny,
                                            const rtl::OUString& rName) const
{
    uno::Any aAny( rAny );
    lcl_manipulateSetting( aAny, rName );

    uno::TypeClass eClass = aAny.getValueTypeClass();
    switch (eClass)
    {
        case uno::TypeClass_VOID:
        {
            /*
             * This assertion pops up when exporting values which are set to:
             * PropertyAttribute::MAYBEVOID, and thus are _supposed_ to have
             * a VOID value...so I'm removing it ...mtg
             * DBG_ERROR("no type");
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
            sal_Int8 nInt8;
            aAny >>= nInt8;
            exportByte(nInt8, rName);
        }
        break;
        case uno::TypeClass_SHORT:
        {
            sal_Int16 nInt16;
            aAny >>= nInt16;
            exportShort(nInt16, rName);
        }
        break;
        case uno::TypeClass_LONG:
        {
            sal_Int32 nInt32;
            aAny >>= nInt32;
            exportInt(nInt32, rName);
        }
        break;
        case uno::TypeClass_HYPER:
        {
            sal_Int64 nInt64;
            aAny >>= nInt64;
            exportLong(nInt64, rName);
        }
        break;
        case uno::TypeClass_DOUBLE:
        {
            double fDouble;
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
            else
                DBG_ERROR("this type is not implemented now");
        }
        break;
    }
}

void XMLSettingsExportHelper::exportBool(const sal_Bool bValue, const rtl::OUString& rName) const
{
    DBG_ASSERT(rName.getLength(), "no name");
    rExport.AddAttribute(XML_NAMESPACE_CONFIG, XML_NAME, rName);
    rExport.AddAttribute(XML_NAMESPACE_CONFIG, XML_TYPE, XML_BOOLEAN);
    SvXMLElementExport aBoolElem(rExport, XML_NAMESPACE_CONFIG, XML_CONFIG_ITEM, sal_True, sal_False);
    rtl::OUString sValue;
    if (bValue)
        sValue = GetXMLToken(XML_TRUE);
    else
        sValue = GetXMLToken(XML_FALSE);
    rExport.GetDocHandler()->characters(sValue);
}

void XMLSettingsExportHelper::exportByte(const sal_Int8 nValue, const rtl::OUString& rName) const
{
    DBG_ASSERT(rName.getLength(), "no name");
    rExport.AddAttribute(XML_NAMESPACE_CONFIG, XML_NAME, rName);
    rExport.AddAttribute(XML_NAMESPACE_CONFIG, XML_TYPE, XML_BYTE);
    SvXMLElementExport aShortElem(rExport, XML_NAMESPACE_CONFIG, XML_CONFIG_ITEM, sal_True, sal_False);
    rtl::OUStringBuffer sBuffer;
    SvXMLUnitConverter::convertNumber(sBuffer, sal_Int32(nValue));
    rExport.GetDocHandler()->characters(sBuffer.makeStringAndClear());
}
void XMLSettingsExportHelper::exportShort(const sal_Int16 nValue, const rtl::OUString& rName) const
{
    DBG_ASSERT(rName.getLength(), "no name");
    rExport.AddAttribute(XML_NAMESPACE_CONFIG, XML_NAME, rName);
    rExport.AddAttribute(XML_NAMESPACE_CONFIG, XML_TYPE, XML_SHORT);
    SvXMLElementExport aShortElem(rExport, XML_NAMESPACE_CONFIG, XML_CONFIG_ITEM, sal_True, sal_False);
    rtl::OUStringBuffer sBuffer;
    SvXMLUnitConverter::convertNumber(sBuffer, sal_Int32(nValue));
    rExport.GetDocHandler()->characters(sBuffer.makeStringAndClear());
}

void XMLSettingsExportHelper::exportInt(const sal_Int32 nValue, const rtl::OUString& rName) const
{
    DBG_ASSERT(rName.getLength(), "no name");
    rExport.AddAttribute(XML_NAMESPACE_CONFIG, XML_NAME, rName);
    rExport.AddAttribute(XML_NAMESPACE_CONFIG, XML_TYPE, XML_INT);
    SvXMLElementExport aIntElem(rExport, XML_NAMESPACE_CONFIG, XML_CONFIG_ITEM, sal_True, sal_False);
    rtl::OUStringBuffer sBuffer;
    SvXMLUnitConverter::convertNumber(sBuffer, nValue);
    rExport.GetDocHandler()->characters(sBuffer.makeStringAndClear());
}

void XMLSettingsExportHelper::exportLong(const sal_Int64 nValue, const rtl::OUString& rName) const
{
    DBG_ASSERT(rName.getLength(), "no name");
    rExport.AddAttribute(XML_NAMESPACE_CONFIG, XML_NAME, rName);
    rExport.AddAttribute(XML_NAMESPACE_CONFIG, XML_TYPE, XML_LONG);
    SvXMLElementExport aIntElem(rExport, XML_NAMESPACE_CONFIG, XML_CONFIG_ITEM, sal_True, sal_False);
    rtl::OUString sValue(rtl::OUString::valueOf(nValue));
    rExport.GetDocHandler()->characters(sValue);
}

void XMLSettingsExportHelper::exportDouble(const double fValue, const rtl::OUString& rName) const
{
    DBG_ASSERT(rName.getLength(), "no name");
    rExport.AddAttribute(XML_NAMESPACE_CONFIG, XML_NAME, rName);
    rExport.AddAttribute(XML_NAMESPACE_CONFIG, XML_TYPE, XML_DOUBLE);
    SvXMLElementExport aDoubleElem(rExport, XML_NAMESPACE_CONFIG, XML_CONFIG_ITEM, sal_True, sal_False);
    rtl::OUStringBuffer sBuffer;
    SvXMLUnitConverter::convertDouble(sBuffer, fValue);
    rExport.GetDocHandler()->characters(sBuffer.makeStringAndClear());
}

void XMLSettingsExportHelper::exportString(const rtl::OUString& sValue, const rtl::OUString& rName) const
{
    DBG_ASSERT(rName.getLength(), "no name");
    rExport.AddAttribute(XML_NAMESPACE_CONFIG, XML_NAME, rName);
    rExport.AddAttribute(XML_NAMESPACE_CONFIG, XML_TYPE, XML_STRING);
    SvXMLElementExport aDoubleElem(rExport, XML_NAMESPACE_CONFIG, XML_CONFIG_ITEM, sal_True, sal_False);
    if (sValue.getLength())
        rExport.GetDocHandler()->characters(sValue);
}

void XMLSettingsExportHelper::exportDateTime(const util::DateTime& aValue, const rtl::OUString& rName) const
{
    DBG_ASSERT(rName.getLength(), "no name");
    rExport.AddAttribute(XML_NAMESPACE_CONFIG, XML_NAME, rName);
    rExport.AddAttribute(XML_NAMESPACE_CONFIG, XML_TYPE, XML_DATETIME);
    rtl::OUStringBuffer sBuffer;
    SvXMLUnitConverter::convertDateTime(sBuffer, aValue);
    SvXMLElementExport aDoubleElem(rExport, XML_NAMESPACE_CONFIG, XML_CONFIG_ITEM, sal_True, sal_False);
    rExport.GetDocHandler()->characters(sBuffer.makeStringAndClear());
}

void XMLSettingsExportHelper::exportSequencePropertyValue(
                    const uno::Sequence<beans::PropertyValue>& aProps,
                    const rtl::OUString& rName) const
{
    DBG_ASSERT(rName.getLength(), "no name");
    sal_Int32 nLength(aProps.getLength());
    if(nLength)
    {
        rExport.AddAttribute(XML_NAMESPACE_CONFIG, XML_NAME, rName);
        SvXMLElementExport aSequenceElem(rExport, XML_NAMESPACE_CONFIG, XML_CONFIG_ITEM_SET, sal_True, sal_True);
        for (sal_Int32 i = 0; i < nLength; i++)
            CallTypeFunction(aProps[i].Value, aProps[i].Name);
    }
}
void XMLSettingsExportHelper::exportSymbolDescriptors(
                    const uno::Sequence < formula::SymbolDescriptor > &rProps,
                    const rtl::OUString rName) const
{
    // #110680#
    // uno::Reference< lang::XMultiServiceFactory > xServiceFactory( comphelper::getProcessServiceFactory() );
    uno::Reference< lang::XMultiServiceFactory > xServiceFactory( rExport.getServiceFactory() );
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
    DBG_ASSERT(rName.getLength(), "no name");
    sal_Int32 nLength(aProps.getLength());
    rExport.AddAttribute(XML_NAMESPACE_CONFIG, XML_NAME, rName);
    rExport.AddAttribute(XML_NAMESPACE_CONFIG, XML_TYPE, XML_BASE64BINARY);
    SvXMLElementExport aDoubleElem(rExport, XML_NAMESPACE_CONFIG, XML_CONFIG_ITEM, sal_True, sal_False);
    if(nLength)
    {
        rtl::OUStringBuffer sBuffer;
        SvXMLUnitConverter::encodeBase64(sBuffer, aProps);
        rExport.GetDocHandler()->characters(sBuffer.makeStringAndClear());
    }
}

void XMLSettingsExportHelper::exportMapEntry(const uno::Any& rAny,
                                        const rtl::OUString& rName,
                                        const sal_Bool bNameAccess) const
{
    DBG_ASSERT((bNameAccess && rName.getLength()) || !bNameAccess, "no name");
    uno::Sequence<beans::PropertyValue> aProps;
    rAny >>= aProps;
    sal_Int32 nLength = aProps.getLength();
    if (nLength)
    {
        if (bNameAccess)
            rExport.AddAttribute(XML_NAMESPACE_CONFIG, XML_NAME, rName);
        SvXMLElementExport aEntryElem(rExport, XML_NAMESPACE_CONFIG, XML_CONFIG_ITEM_MAP_ENTRY, sal_True, sal_True);
        for (sal_Int32 i = 0; i < nLength; i++)
            CallTypeFunction(aProps[i].Value, aProps[i].Name);
    }
}

void XMLSettingsExportHelper::exportNameAccess(
                    const uno::Reference<container::XNameAccess>& aNamed,
                    const rtl::OUString& rName) const
{
    DBG_ASSERT(rName.getLength(), "no name");
    DBG_ASSERT(aNamed->getElementType().equals(getCppuType( (uno::Sequence<beans::PropertyValue> *)0 ) ),
                "wrong NameAccess" );
    if(aNamed->hasElements())
    {
        rExport.AddAttribute(XML_NAMESPACE_CONFIG, XML_NAME, rName);
        SvXMLElementExport aNamedElem(rExport, XML_NAMESPACE_CONFIG, XML_CONFIG_ITEM_MAP_NAMED, sal_True, sal_True);
        uno::Sequence< rtl::OUString > aNames(aNamed->getElementNames());
        for (sal_Int32 i = 0; i < aNames.getLength(); i++)
            exportMapEntry(aNamed->getByName(aNames[i]), aNames[i], sal_True);
    }
}

void XMLSettingsExportHelper::exportIndexAccess(
                    const uno::Reference<container::XIndexAccess> aIndexed,
                    const rtl::OUString rName) const
{
    DBG_ASSERT(rName.getLength(), "no name");
    DBG_ASSERT(aIndexed->getElementType().equals(getCppuType( (uno::Sequence<beans::PropertyValue> *)0 ) ),
                "wrong NameAccess" );
    rtl::OUString sEmpty;// ( RTLCONSTASCII_USTRINGPARAM( "View" ) );
    if(aIndexed->hasElements())
    {
        rExport.AddAttribute(XML_NAMESPACE_CONFIG, XML_NAME, rName);
        SvXMLElementExport aIndexedElem(rExport, XML_NAMESPACE_CONFIG, XML_CONFIG_ITEM_MAP_INDEXED, sal_True, sal_True);
        sal_Int32 nCount = aIndexed->getCount();
        for (sal_Int32 i = 0; i < nCount; i++)
        {
            exportMapEntry(aIndexed->getByIndex(i), sEmpty, sal_False);
        }
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
    // uno::Reference< lang::XMultiServiceFactory > xServiceFactory( comphelper::getProcessServiceFactory() );
    uno::Reference< lang::XMultiServiceFactory > xServiceFactory( rExport.getServiceFactory() );
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

void XMLSettingsExportHelper::exportSettings(
                    const uno::Sequence<beans::PropertyValue>& aProps,
                    const rtl::OUString& rName) const
{
    DBG_ASSERT(rName.getLength(), "no name");
    ::rtl::OUString aQName =
        rExport.GetNamespaceMap().GetQNameByKey( XML_NAMESPACE_OOO, rName );
    exportSequencePropertyValue(aProps, aQName);
}


/** For some settings we may want to change their API representation
 * from their XML settings representation. This is your chance to do
 * so!
 */
void lcl_manipulateSetting(
    uno::Any& rAny,
    const rtl::OUString& rName )
{
    if( rName.equalsAsciiL(
            RTL_CONSTASCII_STRINGPARAM( "PrinterIndependentLayout" ) ) )
    {
        sal_Int16 nTmp;
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
}
