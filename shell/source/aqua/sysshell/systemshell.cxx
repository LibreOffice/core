/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: systemshell.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:34:25 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_shell.hxx"

#ifndef _SYSTEMSHELL_HXX_
#include "systemshell.hxx"
#endif

#include <comphelper/processfactory.hxx>

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_Hpp_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

#ifndef _COM_SUN_STAR_URI_XEXTERNALURIREFERENCETRANSLATOR_HPP_
#include <com/sun/star/uri/XExternalUriReferenceTranslator.hpp>
#endif

#ifndef _COM_SUN_STAR_URI_EXTERNALURIREFERENCETRANSLATOR_HPP_
#include <com/sun/star/uri/ExternalUriReferenceTranslator.hpp>
#endif

#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif

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

namespace SystemShell {

    //##############################
    void AddToRecentDocumentList(const rtl::OUString& aFileUrl, const rtl::OUString& aMimeType)
    {
        // Convert file URL for external use (see above)
        rtl::OUString externalUrl = translateToExternalUrl(aFileUrl);
        if( 0 == externalUrl.getLength() )
            externalUrl = aFileUrl;

        if( externalUrl.getLength() )
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

