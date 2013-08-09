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

#include <chrlohdl.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmluconv.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <rtl/ustrbuf.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/lang/Locale.hpp>

using namespace ::com::sun::star;
using namespace ::xmloff::token;

/* TODO-BCP47: this fiddling with Locale is quite ugly and fragile, especially
 * for the fo:script temporarily stored in Variant, it would be better to use
 * LanguageTagODF but we have that nasty UNO API requirement here.
 * => make LanguageTagODF (unpublished) API? */

// For runtime performance, instead of converting back and forth between
// com::sun::star::Locale and LanguageTag to decide if script or tag are
// needed, this code takes advantage of knowledge about the internal
// representation of BCP 47 language tags in a Locale if present as done in a
// LanguageTag.

XMLCharLanguageHdl::~XMLCharLanguageHdl()
{
    // nothing to do
}

bool XMLCharLanguageHdl::equals( const ::com::sun::star::uno::Any& r1, const ::com::sun::star::uno::Any& r2 ) const
{
    sal_Bool bRet = sal_False;
    lang::Locale aLocale1, aLocale2;

    if( ( r1 >>= aLocale1 ) && ( r2 >>= aLocale2 ) )
    {
        bool bEmptyOrScriptVariant1 = (aLocale1.Variant.isEmpty() || aLocale1.Variant[0] == '-');
        bool bEmptyOrScriptVariant2 = (aLocale2.Variant.isEmpty() || aLocale2.Variant[0] == '-');
        if (bEmptyOrScriptVariant1 && bEmptyOrScriptVariant2)
            bRet = ( aLocale1.Language == aLocale2.Language );
        else
        {
            OUString aLanguage1, aLanguage2;
            if (bEmptyOrScriptVariant1)
                aLanguage1 = aLocale1.Language;
            else
                aLanguage1 = LanguageTag( aLocale1).getLanguage();
            if (bEmptyOrScriptVariant2)
                aLanguage2 = aLocale2.Language;
            else
                aLanguage2 = LanguageTag( aLocale2).getLanguage();
            bRet = ( aLanguage1 == aLanguage2 );
        }
    }

    return bRet;
}

sal_Bool XMLCharLanguageHdl::importXML( const OUString& rStrImpValue, uno::Any& rValue, const SvXMLUnitConverter& ) const
{
    lang::Locale aLocale;
    rValue >>= aLocale;

    if( !IsXMLToken(rStrImpValue, XML_NONE) )
    {
        if (aLocale.Variant.isEmpty())
            aLocale.Language = rStrImpValue;
        else
        {
            if (!aLocale.Language.isEmpty() || aLocale.Variant[0] != '-')
                SAL_WARN( "xmloff.style", "XMLCharLanguageHdl::importXML - attempt to import language twice");
            else
            {
                aLocale.Variant = rStrImpValue + aLocale.Variant;
                if (!aLocale.Country.isEmpty())
                    aLocale.Variant += "-" + aLocale.Country;
                aLocale.Language = I18NLANGTAG_QLT;
            }
        }
    }

    rValue <<= aLocale;
    return sal_True;
}

sal_Bool XMLCharLanguageHdl::exportXML( OUString& rStrExpValue, const uno::Any& rValue, const SvXMLUnitConverter& ) const
{
    lang::Locale aLocale;
    if(!(rValue >>= aLocale))
        return sal_False;

    if (aLocale.Variant.isEmpty())
        rStrExpValue = aLocale.Language;
    else
    {
        LanguageTag aLanguageTag( aLocale);
        OUString aScript, aCountry;
        aLanguageTag.getIsoLanguageScriptCountry( rStrExpValue, aScript, aCountry);
        // Do not write *:language='none' for a non-ISO language with
        // *:rfc-language-tag that is written if Variant is not empty. If there
        // is no match do not write this attribute at all.
        if (rStrExpValue.isEmpty())
            return sal_False;
    }

    if( rStrExpValue.isEmpty() )
        rStrExpValue = GetXMLToken( XML_NONE );

    return sal_True;
}

XMLCharScriptHdl::~XMLCharScriptHdl()
{
    // nothing to do
}

bool XMLCharScriptHdl::equals( const ::com::sun::star::uno::Any& r1, const ::com::sun::star::uno::Any& r2 ) const
{
    sal_Bool bRet = sal_False;
    lang::Locale aLocale1, aLocale2;

    if( ( r1 >>= aLocale1 ) && ( r2 >>= aLocale2 ) )
    {
        bool bEmptyVariant1 = aLocale1.Variant.isEmpty();
        bool bEmptyVariant2 = aLocale2.Variant.isEmpty();
        if (bEmptyVariant1 && bEmptyVariant2)
            bRet = sal_True;
        else if ((bEmptyVariant1 && !bEmptyVariant2) || (!bEmptyVariant1 && bEmptyVariant2))
            ;   // stays false
        else
        {
            OUString aScript1, aScript2;
            if (aLocale1.Variant[0] == '-')
                aScript1 = aLocale1.Variant.copy(1);
            else
                aScript1 = LanguageTag( aLocale1).getScript();
            if (aLocale2.Variant[0] == '-')
                aScript2 = aLocale2.Variant.copy(1);
            else
                aScript2 = LanguageTag( aLocale2).getScript();
            bRet = ( aScript1 == aScript2 );
        }
    }

    return bRet;
}

sal_Bool XMLCharScriptHdl::importXML( const OUString& rStrImpValue, uno::Any& rValue, const SvXMLUnitConverter& ) const
{
    lang::Locale aLocale;
    rValue >>= aLocale;

    if( !IsXMLToken( rStrImpValue, XML_NONE ) )
    {
        // Import the script only if we don't have a full BCP 47 language tag
        // in Variant yet.
        if (aLocale.Variant.isEmpty())
        {
            if (aLocale.Language.isEmpty())
            {
                SAL_INFO( "xmloff.style", "XMLCharScriptHdl::importXML - script but no language yet");
                // Temporarily store in Variant and hope the best (we will get
                // a language later, yes?)
                aLocale.Variant = "-" + rStrImpValue;
            }
            else
            {
                aLocale.Variant = aLocale.Language + "-" + rStrImpValue;
                if (!aLocale.Country.isEmpty())
                    aLocale.Variant += "-" + aLocale.Country;
                aLocale.Language = I18NLANGTAG_QLT;
            }
        }
        else if (aLocale.Variant[0] == '-')
        {
            SAL_WARN( "xmloff.style", "XMLCharScriptHdl::importXML - attempt to insert script twice: "
                    << rStrImpValue << " -> " << aLocale.Variant);
        }
        else
        {
            // Assume that if there already is a script or anything else BCP 47
            // it was read by XMLCharRfcLanguageTagHdl() and takes precedence.
            // On the other hand, an *:rfc-language-tag without script and a
            // *:script ?!?
#if OSL_DEBUG_LEVEL > 0 || defined(DBG_UTIL)
            LanguageTag aLanguageTag( aLocale);
            if (!aLanguageTag.hasScript())
            {
                SAL_WARN( "xmloff.style", "XMLCharScriptHdl::importXML - attempt to insert script over bcp47: "
                        << rStrImpValue << " -> " << aLanguageTag.getBcp47());
            }
#endif
        }
    }

    rValue <<= aLocale;
    return sal_True;
}

sal_Bool XMLCharScriptHdl::exportXML( OUString& rStrExpValue, const uno::Any& rValue, const SvXMLUnitConverter& ) const
{
    lang::Locale aLocale;
    if(!(rValue >>= aLocale))
        return sal_False;

    // Do not write script='none' for default script.

    if (aLocale.Variant.isEmpty())
        return sal_False;

    LanguageTag aLanguageTag( aLocale);
    if (!aLanguageTag.hasScript())
        return sal_False;

    OUString aLanguage, aCountry;
    aLanguageTag.getIsoLanguageScriptCountry( aLanguage, rStrExpValue, aCountry);
    // For non-ISO language it does not make sense to write *:script if
    // *:language is not written either, does it? It's all in
    // *:rfc-language-tag
    if (aLanguage.isEmpty() || rStrExpValue.isEmpty())
        return sal_False;

    return sal_True;
}

XMLCharCountryHdl::~XMLCharCountryHdl()
{
    // nothing to do
}

bool XMLCharCountryHdl::equals( const ::com::sun::star::uno::Any& r1, const ::com::sun::star::uno::Any& r2 ) const
{
    sal_Bool bRet = sal_False;
    lang::Locale aLocale1, aLocale2;

    if( ( r1 >>= aLocale1 ) && ( r2 >>= aLocale2 ) )
        bRet = ( aLocale1.Country == aLocale2.Country );

    return bRet;
}

sal_Bool XMLCharCountryHdl::importXML( const OUString& rStrImpValue, uno::Any& rValue, const SvXMLUnitConverter& ) const
{
    lang::Locale aLocale;
    rValue >>= aLocale;

    if( !IsXMLToken( rStrImpValue, XML_NONE ) )
    {
        if (aLocale.Country.isEmpty())
        {
            aLocale.Country = rStrImpValue;
            if (aLocale.Variant.getLength() >= 7 && aLocale.Language == I18NLANGTAG_QLT)
            {
                // already assembled language tag, at least ll-Ssss and not
                // ll-CC or lll-CC
                sal_Int32 i = aLocale.Variant.indexOf('-');     // separator to script
                if (2 <= i && i < aLocale.Variant.getLength())
                {
                    i = aLocale.Variant.indexOf( '-', i+1);
                    if (i < 0)                                  // no other separator
                        aLocale.Variant += "-" + rStrImpValue;  // append country
                }
            }
        }
    }

    rValue <<= aLocale;
    return sal_True;
}

sal_Bool XMLCharCountryHdl::exportXML( OUString& rStrExpValue, const uno::Any& rValue, const SvXMLUnitConverter& ) const
{
    lang::Locale aLocale;
    if(!(rValue >>= aLocale))
        return sal_False;

    if (aLocale.Variant.isEmpty())
        rStrExpValue = aLocale.Country;
    else
    {
        LanguageTag aLanguageTag( aLocale);
        OUString aLanguage, aScript;
        aLanguageTag.getIsoLanguageScriptCountry( aLanguage, aScript, rStrExpValue);
        // Do not write *:country='none' for a non-ISO country with
        // *:rfc-language-tag that is written if Variant is not empty. If there
        // is no match do not write this attribute at all.
        if (rStrExpValue.isEmpty())
            return sal_False;
    }

    if( rStrExpValue.isEmpty() )
        rStrExpValue = GetXMLToken( XML_NONE );

    return sal_True;
}

XMLCharRfcLanguageTagHdl::~XMLCharRfcLanguageTagHdl()
{
    // nothing to do
}

bool XMLCharRfcLanguageTagHdl::equals( const ::com::sun::star::uno::Any& r1, const ::com::sun::star::uno::Any& r2 ) const
{
    sal_Bool bRet = sal_False;
    lang::Locale aLocale1, aLocale2;

    if( ( r1 >>= aLocale1 ) && ( r2 >>= aLocale2 ) )
        bRet = ( aLocale1.Variant == aLocale2.Variant );

    return bRet;
}

sal_Bool XMLCharRfcLanguageTagHdl::importXML( const OUString& rStrImpValue, uno::Any& rValue, const SvXMLUnitConverter& ) const
{
    lang::Locale aLocale;
    rValue >>= aLocale;

    if( !IsXMLToken( rStrImpValue, XML_NONE ) )
    {
        aLocale.Variant = rStrImpValue;
        aLocale.Language = I18NLANGTAG_QLT;
    }

    rValue <<= aLocale;
    return sal_True;
}

sal_Bool XMLCharRfcLanguageTagHdl::exportXML( OUString& rStrExpValue, const uno::Any& rValue, const SvXMLUnitConverter& ) const
{
    lang::Locale aLocale;
    if(!(rValue >>= aLocale))
        return sal_False;

    // Do not write rfc-language-tag='none' if BCP 47 is not needed.
    if (aLocale.Variant.isEmpty())
        return sal_False;

    rStrExpValue = aLocale.Variant;

    return sal_True;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
