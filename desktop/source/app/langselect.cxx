/*************************************************************************
 *
 *  $RCSfile: langselect.cxx,v $
 *
 *  $Revision: 1.9 $
 *  last change: $Author: kz $ $Date: 2004-07-26 15:11:22 $
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

#include "app.hxx"
#include "langselect.hxx"
#include "langselect.hrc"
#include <stdio.h>

#ifndef _RTL_STRING_HXX
#include <rtl/string.hxx>
#endif
#ifndef _SVTOOLS_PATHOPTIONS_HXX
#include <svtools/pathoptions.hxx>
#endif
#ifndef _TOOLS_RESID_HXX
#include <tools/resid.hxx>
#endif
#ifndef _TOOLS_ISOLANG_HXX
#include <tools/isolang.hxx>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/lang/XLocalizable.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <rtl/locale.hxx>
#ifndef INCLUDED_RTL_INSTANCE_HXX
#include <rtl/instance.hxx>
#endif
#include <osl/process.h>

using namespace rtl;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::container;
using namespace com::sun::star::beans;

namespace desktop {

LanguageSelectionDialog::LanguageSelectionDialog(ResMgr *pResMgr) :
    ModalDialog(NULL,ResId(DLG_LANGSELECT, pResMgr)),
    m_aText(this, ResId(TXT_DLG_LANGSELECT, pResMgr)),
    m_aListBox(this, ResId(LST_DLG_LANGSELECT, pResMgr)),
    m_aButton(this, ResId(BTN_DLG_LANGSELECT_OK, pResMgr))
{
    FreeResource();
}

namespace { struct lLanguages : public rtl::Static<IsoList, lLanguages> {}; }

// execute the language selection
// display a dialog if more than one language is installed
// XXX this is a temporary solution
static sal_Bool bFoundLanguage = sal_False;
//static LanguageType aFoundLanguageType = LANGUAGE_DONTKNOW;
static OUString aFoundLanguage;

Locale LanguageSelection::IsoStringToLocale(const OUString& str)
{
    Locale l;
    sal_Int32 index=0;
    l.Language = str.getToken(0, '-', index);
    if (index >= 0) l.Country = str.getToken(0, '-', index);
    if (index >= 0) l.Variant = str.getToken(0, '-', index);
    return l;
}

void LanguageSelection::prepareLanguage()
{
    // XXX make everything works without assertions in first run
    // in multi-language installations, some things might fail in first run
    if (getUserLanguage().getLength() > 0) return;
    IsoList l = getInstalledIsoLanguages();
    if (l.size() >= 1)
    {
        // throw any away existing default config
        OUString sConfigSrvc = OUString::createFromAscii("com.sun.star.configuration.ConfigurationProvider");
        Reference< XMultiServiceFactory > theMSF = comphelper::getProcessServiceFactory();
        Reference< XLocalizable > theConfigProvider(
            theMSF->createInstance( sConfigSrvc ),UNO_QUERY );
        OSL_ENSURE(theConfigProvider.is(), "cannot localize config manager.");
        if (theConfigProvider.is())
        {
            OUString aLocaleString = *l.begin();
            Locale loc = LanguageSelection::IsoStringToLocale(aLocaleString);
            theConfigProvider->setLocale(loc);
        }
    }
}

OUString LanguageSelection::getLanguageString()
{

    if (bFoundLanguage)
        return aFoundLanguage;

    // check whether there was aleady a language selected by the user
    OUString aUserLanguage = getUserLanguage();
    if (aUserLanguage.getLength() > 0 )
    {
        bFoundLanguage = sal_True;
        aFoundLanguage = aUserLanguage;
        // return aFoundLanguageType;
        // we can not return yet, we need to check whether this instance supports
        // the language that was previously chosen by the user...
    }

    // fill list
    IsoList &rLanguages = lLanguages::get();
    if (rLanguages.size() < 1)
        rLanguages = getInstalledIsoLanguages();
    // check whether found language is available
    if (bFoundLanguage)
    {
        IsoList::const_iterator iLang = rLanguages.begin();
        while (iLang != rLanguages.end())
        {
            if (iLang->equals(aFoundLanguage))
                return aFoundLanguage;
            else
                iLang++;
        }
    }
    if (rLanguages.size() > 1) {
        // are there multiple languages installed?
        // get resource
        rtl::OString aMgrName = OString("langselect") + OString::valueOf((sal_Int32)SUPD, 10);
        ::com::sun::star::lang::Locale aLocale;
        ResMgr* pResMgr = ResMgr::SearchCreateResMgr( aMgrName, aLocale );
        LanguageSelectionDialog lsd(pResMgr);
        StrList languages(getLanguageStrings(rLanguages));
        for (StrList::iterator str_iter = languages.begin(); str_iter != languages.end(); str_iter++)
        {
            lsd.m_aListBox.InsertEntry(*str_iter);
        }

        lsd.Execute();
        short nSelected = lsd.m_aListBox.GetSelectEntryPos();
        IsoList::const_iterator i = rLanguages.begin();
        for (sal_Int32 n=0; n<nSelected; n++) i++;
        bFoundLanguage = sal_True;
        aFoundLanguage = *i;
        return aFoundLanguage;
    } else {
        // if there is only one language, use it
        if (rLanguages.size() == 1) {
            bFoundLanguage = sal_True;
            aFoundLanguage = *(rLanguages.begin());
            return aFoundLanguage;
        } else {
            // last resort
            // don't save
            return ConvertLanguageToIsoString((LanguageType) SvtPathOptions().SubstituteVariable(
                String::CreateFromAscii("$(langid)")).ToInt32()) ;
            /*
            ::com::sun::star::lang::Locale aLocale;
            OUString aLocString( aLocale.Language );
            if ( aLocale.Country.getLength() != 0 )
            {
                aLocString += OUString::createFromAscii("-")
                                + aLocale.Country;
                if ( aLocale.Variant.getLength() != 0 )
                {
                    aLocString += OUString::createFromAscii("-")
                                    + aLocale.Variant;
                }
            }
            return aLocString;
            */
        }
    }
}


// Get the localized selection strings for the list of languages
StrList LanguageSelection::getLanguageStrings(const IsoList& langLst)
{
    StrList aList;
    rtl::OString aMgrName = OString("langselect") + OString::valueOf((sal_Int32)SUPD, 10);
    for (IsoList::const_iterator lang_iter = langLst.begin(); lang_iter != langLst.end(); lang_iter++)
    {
        rtl::OUString lang = static_cast<OUString>(*lang_iter);
        ::com::sun::star::lang::Locale aLocale = LanguageSelection::IsoStringToLocale(lang);
        ResMgr* pResMgr = ResMgr::SearchCreateResMgr( aMgrName, aLocale );
        if (pResMgr != NULL) {
            String aString(ResId(STR_LANGSELECT, pResMgr));
            aList.push_back(aString);
            delete pResMgr;
        }
    }
    return aList;
}

// get a language choosen by the user
OUString LanguageSelection::getUserLanguage()
{
    OUString aLanguage;
    try{

        OUString sConfigSrvc = OUString::createFromAscii("com.sun.star.configuration.ConfigurationProvider");
        OUString sAccessSrvc = OUString::createFromAscii("com.sun.star.configuration.ConfigurationAccess");
        OUString sConfigURL = OUString::createFromAscii("org.openoffice.Setup/L10N/");
        OUString sLocales = OUString::createFromAscii("ooLocale");

        // get configuration provider
        Reference< XMultiServiceFactory > theMSF = comphelper::getProcessServiceFactory();
        Reference< XMultiServiceFactory > theConfigProvider = Reference< XMultiServiceFactory > (
                theMSF->createInstance( sConfigSrvc ),UNO_QUERY );
        // check provider
        if (!theConfigProvider.is()) return aLanguage;

        // access the provider
        Sequence< Any > theArgs(1);
        theArgs[ 0 ] <<= sConfigURL;
        Reference< XNameAccess > theNameAccess = Reference< XNameAccess > (
                theConfigProvider->createInstanceWithArguments(
                sAccessSrvc, theArgs ), UNO_QUERY );
        // check access
        if (!theNameAccess.is()) return aLanguage;
        // run query
        Any aResult = theNameAccess->getByName( sLocales );
        OUString aLangString;
        if (aResult >>= aLangString)
        {
            aLanguage = aLangString;
        }
    } catch (com::sun::star::uno::RuntimeException)
    {
        // didn't work - return dontknow
        return aLanguage;
    }
    return aLanguage;
}

// get a list with the languages that are installed
IsoList LanguageSelection::getInstalledIsoLanguages()
{
    IsoList aList;
    // read language list from org.openoffice.Setup/Office/ooSetupLocales
    try{
        OUString sConfigSrvc = OUString::createFromAscii("com.sun.star.configuration.ConfigurationProvider");
        OUString sAccessSrvc = OUString::createFromAscii("com.sun.star.configuration.ConfigurationAccess");
        OUString sConfigURL = OUString::createFromAscii("org.openoffice.Setup/Office/InstalledLocales");

        // get configuration provider
        Reference< XMultiServiceFactory > theMSF = comphelper::getProcessServiceFactory();
        Reference< XMultiServiceFactory > theConfigProvider = Reference< XMultiServiceFactory > (
                theMSF->createInstance( sConfigSrvc ),UNO_QUERY );
        // check provider
        if (!theConfigProvider.is()) return aList;

        // access the provider
        Sequence< Any > theArgs(1);
        theArgs[ 0 ] <<= sConfigURL;
        Reference< XNameAccess > theNameAccess = Reference< XNameAccess > (
                theConfigProvider->createInstanceWithArguments(
                sAccessSrvc, theArgs ), UNO_QUERY );
        //check access
        if (!theNameAccess.is()) return aList;

        Sequence< OUString > aLangSeq = theNameAccess->getElementNames();

        for (int i=0; i<aLangSeq.getLength(); i++)
               aList.push_back(aLangSeq[i]);
    } catch (com::sun::star::uno::RuntimeException)
    {
        // didn't work - return empty list
    }
    return aList;
}

/*
// get a list with the languages that are installed
LangList LanguageSelection::getInstalledLanguages()
{
    LangList aList;
    // read language list from org.openoffice.Setup/Office/ooSetupLocales
    try{
        OUString sConfigSrvc = OUString::createFromAscii("com.sun.star.configuration.ConfigurationProvider");
        OUString sAccessSrvc = OUString::createFromAscii("com.sun.star.configuration.ConfigurationAccess");
        OUString sConfigURL = OUString::createFromAscii("org.openoffice.Setup/Office/InstalledLocales");

        // get configuration provider
        Reference< XMultiServiceFactory > theMSF = comphelper::getProcessServiceFactory();
        Reference< XMultiServiceFactory > theConfigProvider = Reference< XMultiServiceFactory > (
                theMSF->createInstance( sConfigSrvc ),UNO_QUERY );
        // check provider
        if (!theConfigProvider.is()) return aList;

        // access the provider
        Sequence< Any > theArgs(1);
        theArgs[ 0 ] <<= sConfigURL;
        Reference< XNameAccess > theNameAccess = Reference< XNameAccess > (
                theConfigProvider->createInstanceWithArguments(
                sAccessSrvc, theArgs ), UNO_QUERY );
        //check access
        if (!theNameAccess.is()) return aList;

        Sequence< OUString > aLangSeq = theNameAccess->getElementNames();

        for (int i=0; i<aLangSeq.getLength(); i++)
            aList.push_back(ConvertIsoStringToLanguage(aLangSeq[i]));

    } catch (com::sun::star::uno::Exception&)
    {
        // didn't work - return empty list
    }
    return aList;
}
*/
} // namespace desktop
