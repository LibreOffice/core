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


#include "app.hxx"
#include "langselect.hxx"
#include "cmdlineargs.hxx"
#include <stdio.h>

#include <rtl/string.hxx>
#include <i18nlangtag/mslangid.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/util/XChangesBatch.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/lang/XLocalizable.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include "com/sun/star/util/XFlushable.hpp"
#include <rtl/instance.hxx>

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::container;
using namespace com::sun::star::beans;
using namespace com::sun::star::util;


namespace desktop {

sal_Bool LanguageSelection::bFoundLanguage = sal_False;
OUString LanguageSelection::aFoundLanguage;
LanguageSelection::LanguageSelectionStatus LanguageSelection::m_eStatus = LS_STATUS_OK;

bool LanguageSelection::prepareLanguage()
{
    m_eStatus = LS_STATUS_OK;
    Reference< XLocalizable > theConfigProvider(
        com::sun::star::configuration::theDefaultProvider::get(
            comphelper::getProcessComponentContext() ),
        UNO_QUERY_THROW );

    sal_Bool bSuccess = sal_False;

    // #i42730#get the windows 16Bit locale - it should be preferred over the UI language
    try
    {
        Reference< XPropertySet > xProp(getConfigAccess("org.openoffice.System/L10N/", sal_False), UNO_QUERY_THROW);
        Any aWin16SysLocale = xProp->getPropertyValue("SystemLocale");
        OUString sWin16SysLocale;
        aWin16SysLocale >>= sWin16SysLocale;
        if( !sWin16SysLocale.isEmpty())
            setDefaultLanguage(sWin16SysLocale);
    }
    catch(const Exception&)
    {
        m_eStatus = LS_STATUS_CONFIGURATIONACCESS_BROKEN;
    }

    // #i32939# use system locale to set document default locale
    try
    {
        OUString usLocale;
        Reference< XPropertySet > xLocaleProp(getConfigAccess(
            "org.openoffice.System/L10N", sal_True), UNO_QUERY_THROW);
        xLocaleProp->getPropertyValue("Locale") >>= usLocale;
            setDefaultLanguage(usLocale);
    }
    catch (const Exception&)
    {
        m_eStatus = LS_STATUS_CONFIGURATIONACCESS_BROKEN;
    }

    // get the selected UI language as string
    bool     bCmdLanguage( false );
    OUString aLocaleString = getUserUILanguage();

    if ( aLocaleString.isEmpty() )
    {
        OUString aEmpty;

        const CommandLineArgs& rCmdLineArgs = Desktop::GetCommandLineArgs();
        aLocaleString = rCmdLineArgs.GetLanguage();
        if (isInstalledLanguage(aLocaleString, sal_False))
        {
            bCmdLanguage   = true;
            bFoundLanguage = true;
            aFoundLanguage = aLocaleString;
        }
        else
            aLocaleString = aEmpty;
    }

    // user further fallbacks for the UI language
    if ( aLocaleString.isEmpty() )
        aLocaleString = getLanguageString();

    if ( !aLocaleString.isEmpty() )
    {
        try
        {
            // prepare default config provider by localizing it to the selected
            // locale this will ensure localized configuration settings to be
            // selected according to the UI language.
            LanguageTag aUILanguageTag(aLocaleString);
            theConfigProvider->setLocale(aUILanguageTag.getLocale( false));

            Reference< XPropertySet > xProp(getConfigAccess("org.openoffice.Setup/L10N/", sal_True), UNO_QUERY_THROW);
            if ( !bCmdLanguage )
            {
                // Store language only
                xProp->setPropertyValue("ooLocale", makeAny(aLocaleString));
                Reference< XChangesBatch >(xProp, UNO_QUERY_THROW)->commitChanges();
            }

            MsLangId::setConfiguredSystemUILanguage( aUILanguageTag.getLanguageType( false) );

            OUString sLocale;
            xProp->getPropertyValue("ooSetupSystemLocale") >>= sLocale;
            if ( !sLocale.isEmpty() )
            {
                LanguageTag aLocaleLanguageTag(sLocale);
                MsLangId::setConfiguredSystemLanguage( aLocaleLanguageTag.getLanguageType( false) );
            }
            else
                MsLangId::setConfiguredSystemLanguage( MsLangId::getSystemLanguage() );

            bSuccess = sal_True;
        }
        catch ( const PropertyVetoException& )
        {
            // we are not allowed to change this
        }
        catch (const Exception& e)
        {
            OString aMsg = OUStringToOString(e.Message, RTL_TEXTENCODING_ASCII_US);
            OSL_FAIL(aMsg.getStr());

        }
    }

    // #i32939# setting of default document locale
    // #i32939# this should not be based on the UI language
    setDefaultLanguage(aLocaleString);

    return bSuccess;
}

void LanguageSelection::setDefaultLanguage(const OUString& sLocale)
{
    // #i32939# setting of default document language
    // See #i42730# for rules for determining source of settings

    // determine script type of locale
    LanguageType nLang = LanguageTag::convertToLanguageType(sLocale);
    sal_uInt16 nScriptType = SvtLanguageOptions::GetScriptTypeOfLanguage(nLang);

    switch (nScriptType)
    {
        case SCRIPTTYPE_ASIAN:
            MsLangId::setConfiguredAsianFallback( nLang );
            break;
        case SCRIPTTYPE_COMPLEX:
            MsLangId::setConfiguredComplexFallback( nLang );
            break;
        default:
            MsLangId::setConfiguredWesternFallback( nLang );
            break;
    }
}

OUString LanguageSelection::getUserUILanguage()
{
    // check whether the user has selected a specific language
    OUString aUserLanguage = getUserLanguage();
    if (!aUserLanguage.isEmpty() )
    {
        if (isInstalledLanguage(aUserLanguage))
        {
            // all is well
            bFoundLanguage = sal_True;
            aFoundLanguage = aUserLanguage;
            return aFoundLanguage;
        }
        else
        {
            // selected language is not/no longer installed
            resetUserLanguage();
        }
    }

    return aUserLanguage;
}

OUString LanguageSelection::getLanguageString()
{
    // did we already find a language?
    if (bFoundLanguage)
        return aFoundLanguage;

    // check whether the user has selected a specific language
    OUString aUserLanguage = getUserUILanguage();
    if (!aUserLanguage.isEmpty() )
        return aUserLanguage ;

    // try to use system default
    aUserLanguage = getSystemLanguage();
    if (!aUserLanguage.isEmpty() )
    {
        if (isInstalledLanguage(aUserLanguage, sal_False))
        {
            // great, system default language is available
            bFoundLanguage = sal_True;
            aFoundLanguage = aUserLanguage;
            return aFoundLanguage;
        }
    }
    // fallback 1: en-US
    OUString usFB("en-US");
    if (isInstalledLanguage(usFB))
    {
        bFoundLanguage = sal_True;
        aFoundLanguage = "en-US";
        return aFoundLanguage;
    }

    // fallback didn't work use first installed language
    aUserLanguage = getFirstInstalledLanguage();

    bFoundLanguage = sal_True;
    aFoundLanguage = aUserLanguage;
    return aFoundLanguage;
}

Reference< XNameAccess > LanguageSelection::getConfigAccess(const sal_Char* pPath, sal_Bool bUpdate)
{
    Reference< XNameAccess > xNameAccess;
    try{
        OUString sAccessSrvc;
        if (bUpdate)
            sAccessSrvc = "com.sun.star.configuration.ConfigurationUpdateAccess";
        else
            sAccessSrvc = "com.sun.star.configuration.ConfigurationAccess";

        OUString sConfigURL = OUString::createFromAscii(pPath);

        Reference< XMultiServiceFactory > theConfigProvider(
            com::sun::star::configuration::theDefaultProvider::get(
                comphelper::getProcessComponentContext() ) );

        // access the provider
        Sequence< Any > theArgs(1);
        theArgs[ 0 ] <<= sConfigURL;
        xNameAccess = Reference< XNameAccess > (
            theConfigProvider->createInstanceWithArguments(
                sAccessSrvc, theArgs ), UNO_QUERY_THROW );
    } catch (const com::sun::star::uno::Exception& e)
    {
        OString aMsg = OUStringToOString(e.Message, RTL_TEXTENCODING_ASCII_US);
        OSL_FAIL(aMsg.getStr());
    }
    return xNameAccess;
}

Sequence< OUString > LanguageSelection::getInstalledLanguages()
{
    Sequence< OUString > seqLanguages;
    Reference< XNameAccess > xAccess = getConfigAccess("org.openoffice.Setup/Office/InstalledLocales", sal_False);
    if (!xAccess.is()) return seqLanguages;
    seqLanguages = xAccess->getElementNames();
    return seqLanguages;
}

// FIXME
// it's not very clever to handle language fallbacks here, but
// right now, there is no place that handles those fallbacks globally
static Sequence< OUString > _getFallbackLocales(const OUString& aIsoLang)
{
    Sequence< OUString > seqFallbacks;
    if ( aIsoLang == "zh-HK" ) {
        seqFallbacks = Sequence< OUString >(1);
        seqFallbacks[0] = "zh-TW";
    }
    return seqFallbacks;
}

sal_Bool LanguageSelection::isInstalledLanguage(OUString& usLocale, sal_Bool bExact)
{
    sal_Bool bInstalled = sal_False;
    Sequence< OUString > seqLanguages = getInstalledLanguages();
    for (sal_Int32 i=0; i<seqLanguages.getLength(); i++)
    {
        if (usLocale.equals(seqLanguages[i]))
        {
            bInstalled = sal_True;
            break;
        }
    }

    if (!bInstalled && !bExact)
    {
        // try fallback locales
        Sequence< OUString > seqFallbacks = _getFallbackLocales(usLocale);
        for (sal_Int32 j=0; j<seqFallbacks.getLength(); j++)
        {
            for (sal_Int32 i=0; i<seqLanguages.getLength(); i++)
            {
                if (seqFallbacks[j].equals(seqLanguages[i]))
                {
                    bInstalled = sal_True;
                    usLocale = seqFallbacks[j];
                    break;
                }
            }
        }
    }

    if (!bInstalled && !bExact)
    {
        // no exact match was found, well try to find a substitute
        for (sal_Int32 i=0; i<seqLanguages.getLength(); i++)
        {
            if (usLocale.indexOf(seqLanguages[i]) == 0)
            {
                // requested locale starts with the installed locale
                // (i.e. installed locale has index 0 in requested locale)
                bInstalled = sal_True;
                usLocale   = seqLanguages[i];
                break;
            }
        }
    }
    return bInstalled;
}

OUString LanguageSelection::getFirstInstalledLanguage()
{
    OUString aLanguage;
    Sequence< OUString > seqLanguages = getInstalledLanguages();
    if (seqLanguages.getLength() > 0)
        aLanguage = seqLanguages[0];
    return aLanguage;
}

OUString LanguageSelection::getUserLanguage()
{
    OUString aUserLanguage;
    Reference< XNameAccess > xAccess(getConfigAccess("org.openoffice.Office.Linguistic/General", sal_False));
    if (xAccess.is())
    {
        try
        {
            xAccess->getByName("UILocale") >>= aUserLanguage;
        }
        catch ( NoSuchElementException const & )
        {
            m_eStatus = LS_STATUS_CONFIGURATIONACCESS_BROKEN;
            return OUString();
        }
        catch ( WrappedTargetException const & )
        {
            m_eStatus = LS_STATUS_CONFIGURATIONACCESS_BROKEN;
            return OUString();
        }
    }
    return aUserLanguage;
}

OUString LanguageSelection::getSystemLanguage()
{
    OUString aUserLanguage;
    Reference< XNameAccess > xAccess(getConfigAccess("org.openoffice.System/L10N", sal_False));
    if (xAccess.is())
    {
        try
        {
            xAccess->getByName("UILocale") >>= aUserLanguage;
        }
        catch ( NoSuchElementException const & )
        {
            m_eStatus = LS_STATUS_CONFIGURATIONACCESS_BROKEN;
            return OUString();
        }
        catch ( WrappedTargetException const & )
        {
            m_eStatus = LS_STATUS_CONFIGURATIONACCESS_BROKEN;
            return OUString();
        }
    }
    return aUserLanguage;
}


void LanguageSelection::resetUserLanguage()
{
    try
    {
        Reference< XPropertySet > xProp(getConfigAccess("org.openoffice.Office.Linguistic/General", sal_True), UNO_QUERY_THROW);
        xProp->setPropertyValue("UILocale", makeAny(OUString()));
        Reference< XChangesBatch >(xProp, UNO_QUERY_THROW)->commitChanges();
    }
    catch ( const PropertyVetoException& )
    {
        // we are not allowed to change this
    }
    catch (const Exception& e)
    {
        OString aMsg = OUStringToOString(e.Message, RTL_TEXTENCODING_ASCII_US);
        OSL_FAIL(aMsg.getStr());
        m_eStatus = LS_STATUS_CONFIGURATIONACCESS_BROKEN;
    }
}

LanguageSelection::LanguageSelectionStatus LanguageSelection::getStatus()
{
    return m_eStatus;
}

} // namespace desktop

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
