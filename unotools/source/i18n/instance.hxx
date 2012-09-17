/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 * Copyright (C) 2011 Novell, Inc <michael.meeks@novell.com> (initial developer)
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */
#ifndef _INTL_INSTANCE_HXX
#define _INTL_INSTANCE_HXX

#include <comphelper/processfactory.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <rtl/strbuf.hxx>

namespace css = ::com::sun::star;

// Helper to share code between activators with a fallback MSF.
// Potentially this may also needs to find the library the component
// is implemented inside, but at least centralises this.
inline css::uno::Reference<css::uno::XInterface>
	intl_createInstance( const css::uno::Reference< css::lang::XMultiServiceFactory > & xOptSF,
			     const char *serviceName, const char *context )
{
    css::uno::Reference<css::uno::XInterface> xRet;
    css::uno::Reference<css::lang::XMultiServiceFactory > xSMgr( xOptSF );
    try
    {
        if (!xSMgr.is())
            xSMgr = ::comphelper::getProcessServiceFactory();
        xRet = xSMgr->createInstance( rtl::OUString::createFromAscii( serviceName ) );
    }
    catch (const css::uno::Exception &e)
    {
        SAL_WARN( "unotools.i18n", context << "ctor:Exception caught " << e.Message );
        xRet = css::uno::Reference<css::uno::XInterface>();
    }
    return xRet;
}

#endif // _INTL_INSTANCE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
