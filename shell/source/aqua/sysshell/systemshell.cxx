/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: systemshell.cxx,v $
 * $Revision: 1.3 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
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
#include "precompiled_shell.hxx"
#include "systemshell.hxx"

#include <comphelper/processfactory.hxx>

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_Hpp_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/uri/XExternalUriReferenceTranslator.hpp>
#include <com/sun/star/uri/ExternalUriReferenceTranslator.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <premac.h>
#include <Cocoa/Cocoa.h>
#include <postmac.h>

using namespace ::com::sun::star;

// We need to re-encode file urls because osl_getFileURLFromSystemPath converts
// to UTF-8 before encoding non ascii characters, which is not what other apps expect.
static rtl::OUString translateToExternalUrl(const rtl::OUString& internalUrl)
{
    rtl::OUString extUrl;

    uno::Reference< lang::XMultiServiceFactory > sm = comphelper::getProcessServiceFactory();
    if (sm.is())
    {
        uno::Reference< beans::XPropertySet > pset;
        sm->queryInterface( getCppuType( &pset )) >>= pset;
        if (pset.is())
        {
            uno::Reference< uno::XComponentContext > context;
            static const rtl::OUString DEFAULT_CONTEXT( RTL_CONSTASCII_USTRINGPARAM( "DefaultContext" ) );
            pset->getPropertyValue(DEFAULT_CONTEXT) >>= context;
            if (context.is())
                extUrl = uri::ExternalUriReferenceTranslator::create(context)->translateToExternal(internalUrl);
        }
    }
    return extUrl;
}

// #i104525# many versions of OSX have problems with some URLs:
// when an app requests OSX to add one of these URLs to the "Recent Items" list
// then this app gets killed (TextEdit, Preview, etc. and also OOo)
static bool isDangerousUrl( const rtl::OUString& rUrl)
{
    // use a heuristic that detects all known cases since there is no official comment
    // on the exact impact and root cause of the OSX bug
    const int nLen = rUrl.getLength();
    const sal_Unicode* p = rUrl.getStr();
    for( int i = 0; i < nLen-3; ++i, ++p) {
        if( p[0] != '%')
            continue;
        // escaped percent?
        if( (p[1] == '2') && (p[2] == '5'))
            return true;
        // escapes are considered to be UTF-8 encoded
        // => check for invalid UTF-8 leading byte
        if( (p[1] != 'f') && (p[1] != 'F'))
            continue;
        int cLowNibble = p[2];
        if( (cLowNibble >= '0') && (cLowNibble <= '9'))
            return false;
        if( cLowNibble >= 'a')
            cLowNibble -= 'a' - 'A';
        if( (cLowNibble < 'A') || (cLowNibble >= 'C'))
            return true;
    }

    return false;
}

namespace SystemShell {

    //##############################
    void AddToRecentDocumentList(const rtl::OUString& aFileUrl, const rtl::OUString& /*aMimeType*/)
    {
        // Convert file URL for external use (see above)
        rtl::OUString externalUrl = translateToExternalUrl(aFileUrl);
        if( 0 == externalUrl.getLength() )
            externalUrl = aFileUrl;

        if( externalUrl.getLength()
        &&  !isDangerousUrl( externalUrl))
        {
            NSString* pString = [[NSString alloc] initWithCharacters: externalUrl.getStr() length: externalUrl.getLength()];
            NSURL* pURL = [NSURL URLWithString: pString];

            if( pURL )
            {
                NSDocumentController* pCtrl = [NSDocumentController sharedDocumentController];
                [pCtrl noteNewRecentDocumentURL: pURL];
            }
            if( pString )
                [pString release];

        }
    }

} // namespace SystemShell

