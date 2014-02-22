/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <chrlohdl.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmluconv.hxx>
#include <unotools/saveopt.hxx>
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







XMLCharLanguageHdl::~XMLCharLanguageHdl()
{
    
}

bool XMLCharLanguageHdl::equals( const ::com::sun::star::uno::Any& r1, const ::com::sun::star::uno::Any& r2 ) const
{
    bool bRet = false;
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

bool XMLCharLanguageHdl::importXML( const OUString& rStrImpValue, uno::Any& rValue, const SvXMLUnitConverter& ) const
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
            {
                SAL_WARN_IF( aLocale.Language != I18NLANGTAG_QLT, "xmloff.style",
                        "XMLCharLanguageHdl::importXML - attempt to import language twice");
            }
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
    return true;
}

bool XMLCharLanguageHdl::exportXML( OUString& rStrExpValue, const uno::Any& rValue, const SvXMLUnitConverter& ) const
{
    lang::Locale aLocale;
    if(!(rValue >>= aLocale))
        return false;

    if (aLocale.Variant.isEmpty())
        rStrExpValue = aLocale.Language;
    else
    {
        LanguageTag aLanguageTag( aLocale);
        OUString aScript, aCountry;
        aLanguageTag.getIsoLanguageScriptCountry( rStrExpValue, aScript, aCountry);
        
        
        
        if (rStrExpValue.isEmpty())
            return false;
    }

    if( rStrExpValue.isEmpty() )
        rStrExpValue = GetXMLToken( XML_NONE );

    return true;
}

XMLCharScriptHdl::~XMLCharScriptHdl()
{
    
}

bool XMLCharScriptHdl::equals( const ::com::sun::star::uno::Any& r1, const ::com::sun::star::uno::Any& r2 ) const
{
    bool bRet = false;
    lang::Locale aLocale1, aLocale2;

    if( ( r1 >>= aLocale1 ) && ( r2 >>= aLocale2 ) )
    {
        bool bEmptyVariant1 = aLocale1.Variant.isEmpty();
        bool bEmptyVariant2 = aLocale2.Variant.isEmpty();
        if (bEmptyVariant1 && bEmptyVariant2)
            bRet = true;
        else if ((bEmptyVariant1 && !bEmptyVariant2) || (!bEmptyVariant1 && bEmptyVariant2))
            ;   
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

bool XMLCharScriptHdl::importXML( const OUString& rStrImpValue, uno::Any& rValue, const SvXMLUnitConverter& ) const
{
    lang::Locale aLocale;
    rValue >>= aLocale;

    if( !IsXMLToken( rStrImpValue, XML_NONE ) )
    {
        
        
        if (aLocale.Variant.isEmpty())
        {
            if (aLocale.Language.isEmpty())
            {
                SAL_INFO( "xmloff.style", "XMLCharScriptHdl::importXML - script but no language yet");
                
                
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
    return true;
}

bool XMLCharScriptHdl::exportXML( OUString& rStrExpValue, const uno::Any& rValue, const SvXMLUnitConverter& ) const
{
    lang::Locale aLocale;
    if(!(rValue >>= aLocale))
        return false;

    

    if (aLocale.Variant.isEmpty())
        return false;

    LanguageTag aLanguageTag( aLocale);
    if (!aLanguageTag.hasScript())
        return false;

    if (SvtSaveOptions().GetODFDefaultVersion() < SvtSaveOptions::ODFVER_012)
        return false;

    OUString aLanguage, aCountry;
    aLanguageTag.getIsoLanguageScriptCountry( aLanguage, rStrExpValue, aCountry);
    
    
    
    if (aLanguage.isEmpty() || rStrExpValue.isEmpty())
        return false;

    return true;
}

XMLCharCountryHdl::~XMLCharCountryHdl()
{
    
}

bool XMLCharCountryHdl::equals( const ::com::sun::star::uno::Any& r1, const ::com::sun::star::uno::Any& r2 ) const
{
    bool bRet = false;
    lang::Locale aLocale1, aLocale2;

    if( ( r1 >>= aLocale1 ) && ( r2 >>= aLocale2 ) )
        bRet = ( aLocale1.Country == aLocale2.Country );

    return bRet;
}

bool XMLCharCountryHdl::importXML( const OUString& rStrImpValue, uno::Any& rValue, const SvXMLUnitConverter& ) const
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
                
                
                sal_Int32 i = aLocale.Variant.indexOf('-');     
                if (2 <= i && i < aLocale.Variant.getLength())
                {
                    i = aLocale.Variant.indexOf( '-', i+1);
                    if (i < 0)                                  
                        aLocale.Variant += "-" + rStrImpValue;  
                }
            }
        }
    }

    rValue <<= aLocale;
    return true;
}

bool XMLCharCountryHdl::exportXML( OUString& rStrExpValue, const uno::Any& rValue, const SvXMLUnitConverter& ) const
{
    lang::Locale aLocale;
    if(!(rValue >>= aLocale))
        return false;

    if (aLocale.Variant.isEmpty())
        rStrExpValue = aLocale.Country;
    else
    {
        LanguageTag aLanguageTag( aLocale);
        OUString aLanguage, aScript;
        aLanguageTag.getIsoLanguageScriptCountry( aLanguage, aScript, rStrExpValue);
        
        
        
        if (rStrExpValue.isEmpty())
            return false;
    }

    if( rStrExpValue.isEmpty() )
        rStrExpValue = GetXMLToken( XML_NONE );

    return true;
}

XMLCharRfcLanguageTagHdl::~XMLCharRfcLanguageTagHdl()
{
    
}

bool XMLCharRfcLanguageTagHdl::equals( const ::com::sun::star::uno::Any& r1, const ::com::sun::star::uno::Any& r2 ) const
{
    bool bRet = false;
    lang::Locale aLocale1, aLocale2;

    if( ( r1 >>= aLocale1 ) && ( r2 >>= aLocale2 ) )
        bRet = ( aLocale1.Variant == aLocale2.Variant );

    return bRet;
}

bool XMLCharRfcLanguageTagHdl::importXML( const OUString& rStrImpValue, uno::Any& rValue, const SvXMLUnitConverter& ) const
{
    lang::Locale aLocale;
    rValue >>= aLocale;

    if( !IsXMLToken( rStrImpValue, XML_NONE ) )
    {
        aLocale.Variant = rStrImpValue;
        aLocale.Language = I18NLANGTAG_QLT;
    }

    rValue <<= aLocale;
    return true;
}

bool XMLCharRfcLanguageTagHdl::exportXML( OUString& rStrExpValue, const uno::Any& rValue, const SvXMLUnitConverter& ) const
{
    lang::Locale aLocale;
    if(!(rValue >>= aLocale))
        return false;

    
    if (aLocale.Variant.isEmpty())
        return false;

    if (SvtSaveOptions().GetODFDefaultVersion() < SvtSaveOptions::ODFVER_012)
        return false;

    rStrExpValue = aLocale.Variant;

    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
