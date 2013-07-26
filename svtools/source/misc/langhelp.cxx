/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */


#include <svtools/langhelp.hxx>

#include <vcl/svapp.hxx>
#include <rtl/ustring.hxx>

void localizeWebserviceURI( OUString& rURI )
{
    OUString aLang = Application::GetSettings().GetUILanguageTag().getLanguage();
    if ( aLang.equalsIgnoreAsciiCase("pt")
         && Application::GetSettings().GetUILanguageTag().getCountry().equalsIgnoreAsciiCase("br") )
    {
        aLang = OUString("pt-br");
    }
    if ( aLang.equalsIgnoreAsciiCase("zh") )
    {
        if ( Application::GetSettings().GetUILanguageTag().getCountry().equalsIgnoreAsciiCase("cn") )
            aLang = OUString("zh-cn");
        if ( Application::GetSettings().GetUILanguageTag().getCountry().equalsIgnoreAsciiCase("tw") )
            aLang = OUString("zh-tw");
    }

    rURI += aLang;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
