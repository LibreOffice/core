/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Initial Developer of the Original Code is
 *       Thorsten Behrens <tbehrens@novell.com>
 *
 * Contributor(s): Thorsten Behrens <tbehrens@novell.com>
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svtools.hxx"

#include <svtools/langhelp.hxx>

#include <vcl/svapp.hxx>
#include <rtl/ustring.hxx>

void localizeWebserviceURI( ::rtl::OUString& rURI )
{
    ::rtl::OUString aLang = Application::GetSettings().GetUILocale().Language;
    if ( aLang.equalsIgnoreAsciiCaseAscii( "pt" )
         && Application::GetSettings().GetUILocale().Country.equalsIgnoreAsciiCaseAscii("br") )
    {
        aLang = ::rtl::OUString::createFromAscii("pt-br");
    }

    rURI += aLang;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
