/*************************************************************************
 *
 *  $RCSfile: langselect.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2003-11-07 14:51:46 $
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


using namespace rtl;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::container;

namespace desktop {

LanguageSelection::LanguageSelection()
:m_pText(NULL)
,m_pListBox(NULL)
,m_pButton(NULL)
,m_pDialog(NULL)
{

}

LanguageSelection::~LanguageSelection()
{
    delete m_pText;
    delete m_pListBox;
    delete m_pButton;
    delete m_pDialog;
}

// execute the language selection
// display a dialog if more than one language is installed
// XXX this is a temporary solution
LanguageType LanguageSelection::Execute()
{

    static sal_Bool bFoundLanguage = sal_False;
    static LanguageType aFoundLanguageType = LANGUAGE_DONTKNOW;
    if (bFoundLanguage)
        return aFoundLanguageType;

    // check whether there was aleady a language selected by the user
    LanguageType aUserLanguage = getUserLanguage();
    if (aUserLanguage != LANGUAGE_DONTKNOW)
    {
        bFoundLanguage = sal_True;
        aFoundLanguageType = aUserLanguage;
        return aFoundLanguageType;
    }

    // get resource
    rtl::OString aMgrName = OString("langselect") + OString::valueOf((sal_Int32)SUPD, 10);
    LanguageType aLanguageType = LANGUAGE_DONTKNOW;
    ResMgr* pResMgr = ResMgr::SearchCreateResMgr( aMgrName, aLanguageType );
    ResId aResId(DLG_LANGSELECT, pResMgr);
    m_pDialog = new ModalDialog(NULL, aResId);
    aResId = ResId(TXT_DLG_LANGSELECT, pResMgr);
    m_pText = new FixedText(m_pDialog, aResId);
    aResId = ResId(LST_DLG_LANGSELECT, pResMgr);
    m_pListBox = new ListBox(m_pDialog, aResId);
    aResId = ResId(BTN_DLG_LANGSELECT_OK, pResMgr);
    m_pButton = new OKButton(m_pDialog, aResId);

    // fill list
    m_lLanguages = getInstalledLanguages();
    StrList languages(getLanguageStrings(m_lLanguages));
    for (StrList::iterator str_iter = languages.begin(); str_iter != languages.end(); str_iter++)
    {
        m_pListBox->InsertEntry(*str_iter);
    }

    if (m_lLanguages.size() > 1) {
        // are there multiple languages installed?
        m_pDialog->Execute();
        short nSelected = m_pListBox->GetSelectEntryPos();
        LangList::const_iterator i = m_lLanguages.begin();
        for (sal_Int32 n=0; n<nSelected; n++) i++;
        bFoundLanguage = sal_True;
        aFoundLanguageType = *i;
        return aFoundLanguageType;
    } else {
        // if there is only one language, use it
        if (m_lLanguages.size() == 1) {
            bFoundLanguage = sal_True;
            aFoundLanguageType = *(m_lLanguages.begin());
            return aFoundLanguageType;
        } else {
            // last resort
            // don't save
            return (LanguageType) SvtPathOptions().SubstituteVariable(
                String::CreateFromAscii("$(langid)")).ToInt32();
        }
    }
}

// Get the localized selection strings for the list of languages
StrList LanguageSelection::getLanguageStrings(const LangList& langLst) const
{
    StrList aList;
    rtl::OString aMgrName = OString("langselect") + OString::valueOf((sal_Int32)SUPD, 10);
    for (LangList::const_iterator lang_iter = langLst.begin(); lang_iter != langLst.end(); lang_iter++)
    {
        LanguageType lang = static_cast<LanguageType>(*lang_iter);
        ResMgr* pResMgr = ResMgr::SearchCreateResMgr( aMgrName, lang);
        if (pResMgr != NULL) {
            String aString(ResId(STR_LANGSELECT, pResMgr));
            aList.push_back(aString);
            delete pResMgr;
        }
    }
    return aList;
}

// get a language choosen by the user
LanguageType LanguageSelection::getUserLanguage() const
{
    LanguageType aLanguageType = LANGUAGE_DONTKNOW;
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
        if (!theConfigProvider.is()) return LANGUAGE_DONTKNOW;

        // access the provider
        Sequence< Any > theArgs(1);
        theArgs[ 0 ] <<= sConfigURL;
        Reference< XNameAccess > theNameAccess = Reference< XNameAccess > (
                theConfigProvider->createInstanceWithArguments(
                sAccessSrvc, theArgs ), UNO_QUERY );
        // check access
        if (!theNameAccess.is()) return LANGUAGE_DONTKNOW;
        // run query
        Any aResult = theNameAccess->getByName( sLocales );
        OUString aLangString;
        if (aResult >>= aLangString)
        {
            aLanguageType = ConvertIsoStringToLanguage(aLangString);
        }
    } catch (com::sun::star::uno::RuntimeException)
    {
        // didn't work - return dontknow
        return LANGUAGE_DONTKNOW;
    }
    return aLanguageType;
}

// get a list with the languages that are installed
LangList LanguageSelection::getInstalledLanguages() const
{
    LangList aList;
    // read language list from org.openoffice.Setup/Office/ooSetupLocales
    try{
        OUString sConfigSrvc = OUString::createFromAscii("com.sun.star.configuration.ConfigurationProvider");
        OUString sAccessSrvc = OUString::createFromAscii("com.sun.star.configuration.ConfigurationAccess");
        OUString sConfigURL = OUString::createFromAscii("org.openoffice.Setup/Office/");
        OUString sLocales = OUString::createFromAscii("ooSetupLocales");

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

        Any aResult = theNameAccess->getByName( sLocales );
        Sequence< OUString > aLangSeq;

        // unpack result from Any type
        if (aResult >>= aLangSeq)
        {
            for (int i=0; i<aLangSeq.getLength(); i++)
                aList.push_back(ConvertIsoStringToLanguage(aLangSeq[i]));
        }
    } catch (com::sun::star::uno::RuntimeException)
    {
        // didn't work - return empty list
    }
    return aList;
}

} // namespace desktop
