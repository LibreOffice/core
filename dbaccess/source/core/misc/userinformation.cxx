/*************************************************************************
 *
 *  $RCSfile: userinformation.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:15:40 $
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

#ifndef _DBA_CORE_USERINFORMATION_HXX_
#include "userinformation.hxx"
#endif

#ifndef _COM_SUN_STAR_REGISTRY_XSIMPLEREGISTRY_HPP_
#include <com/sun/star/registry/XSimpleRegistry.hpp>
#endif
#ifndef _COM_SUN_STAR_REGISTRY_XREGISTRYKEY_HPP_
#include <com/sun/star/registry/XRegistryKey.hpp>
#endif

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _ISOLANG_HXX
#include <tools/isolang.hxx>
#endif

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;

//--------------------------------------------------------------------------
UserInformation::UserInformation(::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxFactory)
    :m_xServiceFactory(_rxFactory)
{
    OSL_ENSHURE(m_xServiceFactory.is(), "UserInformation::UserInformation : invalid service factory !");

    if (!m_xServiceFactory.is())
        return;

    // get the ConfigManager wrapping the soffice.ini
    Reference< XSimpleRegistry > xSofficeIni(
        m_xServiceFactory->createInstance(rtl::OUString::createFromAscii("com.sun.star.config.SpecialConfigManager")),
        UNO_QUERY);

    OSL_ENSHURE(xSofficeIni.is(), "UserInformation::UserInformation : could not get access to the SpecialConfigManager !");
    if (!xSofficeIni.is())
        return;

    try
    {
        Reference< XRegistryKey > xIniRoot = xSofficeIni->getRootKey();
        m_xUserConfigKey = xIniRoot.is() ? xIniRoot->openKey(::rtl::OUString::createFromAscii("User")) : Reference< XRegistryKey > ();
    }
    catch(InvalidRegistryException&)
    {
    }
}

//--------------------------------------------------------------------------
Locale UserInformation::getUserLanguage() const
{
    Locale aReturn;
    if (!isValid())
        return aReturn;

    LanguageType eLanguage = LANGUAGE_ENGLISH_US;       // default
    try
    {
        Reference< XRegistryKey > xLanguage = m_xUserConfigKey->openKey(::rtl::OUString::createFromAscii("Language"));
        if (xLanguage.is())
        {
            String sLanguage = xLanguage->getStringValue();
            eLanguage = (LanguageType)sLanguage.ToInt32();
        }
    }
    catch(InvalidRegistryException&)
    {
    }
    catch(InvalidValueException&)
    {
    }

    UniString sLanguage, sCountry;
    ConvertLanguageToIsoNames(eLanguage, sLanguage, sCountry);
    aReturn.Language = sLanguage;
    aReturn.Country = sCountry;

    return aReturn;
}


