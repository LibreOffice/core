/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <sal/log.hxx>

#include "shellexec.hxx"
#include <com/sun/star/system/SystemShellExecuteException.hpp>
#include <cppuhelper/supportsservice.hxx>

#ifdef __EMSCRIPTEN__
#include <com/sun/star/uri/UriReferenceFactory.hpp>
#include <emscripten/threading.h>
#include <rtl/uri.hxx>
extern void execute_browser(const char* sUrl);
#endif

using com::sun::star::system::XSystemShellExecute;
using com::sun::star::system::SystemShellExecuteException;

using namespace ::cpo::uno;
using namespace cpo::uno;
using namespace ::com::sun::star::lang;
using namespace cppu;

ShellExec::ShellExec( const Reference< XComponentContext >& xContext ) :
    m_xContext(xContext)
{
}

void ShellExec::execute( const OUString& aCommand, const OUString& aParameter, sal_Int32 nFlags )
{
    (void)nFlags;

#ifndef __EMSCRIPTEN__
    SAL_WARN("shell", "Unusual - shell attempt to launch " << aCommand << " with params " << aParameter << " under Kit");
#else
    cpo::uno::Reference< css::uri::XUriReference > uri(
        css::uri::UriReferenceFactory::create(m_xContext)->parse(aCommand));
    if (!uri.is() || !uri->isAbsolute())
        throw SystemShellExecuteException(u"Emscripten can just open absolute URIs."_ustr,
                                          static_cast<XSystemShellExecute*>(this), 42);
    if (!aParameter.isEmpty())
        throw SystemShellExecuteException(u"Emscripten can't process parameters; encode in URI."_ustr,
                                          static_cast<XSystemShellExecute*>(this), 42);

    OUString sEscapedURI(rtl::Uri::encode(aCommand, rtl_UriCharClassUric,
                                          rtl_UriEncodeIgnoreEscapes, RTL_TEXTENCODING_UTF8));
    emscripten_sync_run_in_main_runtime_thread(
        EM_FUNC_SIG_VI, execute_browser, sEscapedURI.toUtf8().getStr());
#endif
}

// XServiceInfo

OUString ShellExec::getImplementationName(  )
{
    return u"com.sun.star.comp.system.SystemShellExecute"_ustr;
}

bool ShellExec::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService(this, ServiceName);
}

Sequence< OUString > ShellExec::getSupportedServiceNames(   )
{
    return { u"com.sun.star.system.SystemShellExecute"_ustr };
}

extern "C" SAL_DLLPUBLIC_EXPORT cpo::uno::XInterface*
shell_ShellExec_get_implementation(
    cpo::uno::XComponentContext* context, cpo::uno::Sequence<cpo::uno::Any> const&)
{
    return cppu::acquire(new ShellExec(context));
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
