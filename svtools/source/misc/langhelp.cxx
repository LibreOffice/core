/*************************************************************************
 *
 * Copyright 2010 Novell, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * or later, as published by the Free Software Foundation.
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
