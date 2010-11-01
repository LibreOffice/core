/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#include "precompiled_tools.hxx"

#include <tools/svlibrary.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/util/XMacroExpander.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <comphelper/processfactory.hxx>
#include <tools/string.hxx>
#include <rtl/uri.hxx>

using namespace com::sun::star;

static uno::Sequence< rtl::OUString > GetMultiPaths_Impl()
{
    uno::Sequence< rtl::OUString >   aRes;
    uno::Sequence< rtl::OUString >   aInternalPaths;
    uno::Sequence< rtl::OUString >   aUserPaths;

    bool bSuccess = true;
    uno::Reference< lang::XMultiServiceFactory >  xMgr( comphelper::getProcessServiceFactory() );
    if (xMgr.is())
    {
        try
        {
            String aInternal;
            aInternal.AppendAscii("Libraries");
            String aUser;
            aUser.AppendAscii("Libraries");
            aInternal .AppendAscii( "_internal" );
            aUser     .AppendAscii( "_user" );

            uno::Reference< beans::XPropertySet > xPathSettings( xMgr->createInstance(
                rtl::OUString::createFromAscii( "com.sun.star.util.PathSettings" ) ), uno::UNO_QUERY_THROW );
            xPathSettings->getPropertyValue( aInternal )  >>= aInternalPaths;
            xPathSettings->getPropertyValue( aUser )      >>= aUserPaths;
        }
        catch (uno::Exception &)
        {
            bSuccess = false;
        }
    }
    if (bSuccess)
    {
        sal_Int32 nMaxEntries = aInternalPaths.getLength() + aUserPaths.getLength();
        aRes.realloc( nMaxEntries );
        rtl::OUString *pRes = aRes.getArray();
        sal_Int32 nCount = 0;   // number of actually added entries
        for (int i = 0;  i < 2;  ++i)
        {
            const uno::Sequence< rtl::OUString > &rPathSeq = i == 0 ? aUserPaths : aInternalPaths;
            const rtl::OUString *pPathSeq = rPathSeq.getConstArray();
            for (sal_Int32 k = 0;  k < rPathSeq.getLength();  ++k)
            {
                const bool bAddUser     = (&rPathSeq == &aUserPaths);
                const bool bAddInternal = (&rPathSeq == &aInternalPaths);
                if ((bAddUser || bAddInternal) && pPathSeq[k].getLength() > 0)
                    pRes[ nCount++ ] = pPathSeq[k];
            }
        }
        aRes.realloc( nCount );
    }

    return aRes;
}

bool SvLibrary::LoadModule( osl::Module& rModule, const rtl::OUString& rLibName, ::oslGenericFunction baseModule, ::sal_Int32 mode )
{
    static uno::Sequence < rtl::OUString > aPaths = GetMultiPaths_Impl();
    bool bLoaded = false;

    for (sal_Int32 n=0; n<aPaths.getLength(); n++)
    {
        rtl::OUString aMod = aPaths[n];
        if ( aPaths[n].indexOfAsciiL("vnd.sun.star.expand",19) == 0)
        {
            uno::Reference< uno::XComponentContext > xComponentContext = comphelper::getProcessComponentContext();
            uno::Reference< util::XMacroExpander > xMacroExpander;
            xComponentContext->getValueByName(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("/singletons/com.sun.star.util.theMacroExpander") ) )
                    >>= xMacroExpander;

            aMod = aMod.copy( sizeof("vnd.sun.star.expand:") -1 );
            aMod = ::rtl::Uri::decode( aMod, rtl_UriDecodeWithCharset, RTL_TEXTENCODING_UTF8 );
            aMod = xMacroExpander->expandMacros( aMod );
        }

        aMod += ::rtl::OUString( sal_Unicode('/') );
        aMod += rLibName;
        bLoaded = rModule.load( aMod, mode );
        if ( bLoaded )
            break;
    }

    if (!bLoaded )
        bLoaded = rModule.loadRelative( baseModule, rLibName, mode );

    return bLoaded;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
