/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: systemshell.cxx,v $
 * $Revision: 1.9 $
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

#include "osl/module.hxx"

using namespace ::com::sun::star;

#define UNISTRING(s) rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(s))

const rtl::OUString SYM_ADD_TO_RECENTLY_USED_FILE_LIST = UNISTRING("add_to_recently_used_file_list");
const rtl::OUString LIB_RECENT_FILE = UNISTRING("librecentfile.so");
const rtl::OUString DEFAULT_CONTEXT = UNISTRING("DefaultContext");

void * (* sym_gtk_recent_manager_get_default) () = NULL;
void   (* sym_gtk_recent_manager_add_item) (void *, const char *) = NULL;

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
            pset->getPropertyValue(DEFAULT_CONTEXT) >>= context;
            if (context.is())
                extUrl = uri::ExternalUriReferenceTranslator::create(context)->translateToExternal(internalUrl);
        }
    }
    return extUrl;
}

namespace SystemShell {

    typedef void (*PFUNC_ADD_TO_RECENTLY_USED_LIST)(const rtl::OUString&, const rtl::OUString&);

    //##############################
    rtl::OUString get_absolute_library_url(const rtl::OUString& /*lib_name*/)
    {
        rtl::OUString url;
        if (osl::Module::getUrlFromAddress(reinterpret_cast<oslGenericFunction>(AddToRecentDocumentList), url))
        {
            sal_Int32 index = url.lastIndexOf('/');
            url = url.copy(0, index + 1);
            url += LIB_RECENT_FILE;
        }
        return url;
    }

    bool init_recent_manager_api()
    {
        oslModule hDefault;
        if( osl_getModuleHandle( NULL, &hDefault ) )
        {
            sym_gtk_recent_manager_get_default = (void * (*)())
                osl_getAsciiFunctionSymbol(hDefault, "gtk_recent_manager_get_default");
            sym_gtk_recent_manager_add_item = (void (*)(void *, const char *))
                osl_getAsciiFunctionSymbol(hDefault, "gtk_recent_manager_add_item");

        }
        bool ret = (NULL != sym_gtk_recent_manager_get_default) && (NULL != sym_gtk_recent_manager_add_item);
        return ret;
    }


    //##############################
    void AddToRecentDocumentList(const rtl::OUString& aFileUrl, const rtl::OUString& aMimeType)
    {
        static bool bIsRecentManagerPresent = init_recent_manager_api();

        // Convert file URL for external use (see above)
        rtl::OUString externalUrl = translateToExternalUrl(aFileUrl);
        if( 0 == externalUrl.getLength() )
            externalUrl = aFileUrl;

        if( bIsRecentManagerPresent )
        {
            void * recent_manager = sym_gtk_recent_manager_get_default();
            sym_gtk_recent_manager_add_item(recent_manager, rtl::OUStringToOString(aFileUrl, RTL_TEXTENCODING_UTF8).getStr());
        }
        else
        {
            rtl::OUString librecentfile_url = get_absolute_library_url(LIB_RECENT_FILE);

            if (librecentfile_url.getLength())
            {
                osl::Module module(librecentfile_url);

                if (module.is())
                {
                    // convert from reinterpret_cast<PFUNC_ADD_TO_RECENTLY_USED_LIST>
                    // not allowed in gcc 3.3 without permissive.
                    PFUNC_ADD_TO_RECENTLY_USED_LIST add_to_recently_used_file_list =
                        reinterpret_cast<PFUNC_ADD_TO_RECENTLY_USED_LIST>(module.getFunctionSymbol(SYM_ADD_TO_RECENTLY_USED_FILE_LIST));

                    if (add_to_recently_used_file_list)
                        add_to_recently_used_file_list(aFileUrl, aMimeType);
                }
            }
        }
    }

} // namespace SystemShell

