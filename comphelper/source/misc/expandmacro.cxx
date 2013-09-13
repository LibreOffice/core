/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <comphelper/expandmacro.hxx>

#include <com/sun/star/uno/Reference.hxx>
#include <rtl/ustring.hxx>
#include <rtl/uri.hxx>
#include <rtl/bootstrap.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace comphelper
{
    rtl::OUString getExpandedFilePath(const rtl::OUString& filepath)
    {
        rtl::OUString aFilename = filepath;

        if( aFilename.startsWith( "vnd.sun.star.expand:" ) )
        {
            // cut protocol
            rtl::OUString aMacro( aFilename.copy( sizeof ( "vnd.sun.star.expand:" ) -1 ) );

            // decode uric class chars
            aMacro = rtl::Uri::decode( aMacro, rtl_UriDecodeWithCharset, RTL_TEXTENCODING_UTF8 );

            // expand macro string
            rtl::Bootstrap::expandMacros( aMacro);

            aFilename = aMacro;
        }
        return aFilename;
    }
}
