/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <utility>

#include "gdrive_datasupplier.hxx"
#include "gdrive_resultset.hxx"
#include <ucbhelper/authenticationfallback.hxx>
#include <com/sun/star/task/XInteractionHandler.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <comphelper/processfactory.hxx>

using namespace com::sun::star;
using namespace ucp::gdrive;

DynamicResultSet::DynamicResultSet(
    const uno::Reference< uno::XComponentContext >& rxContext,
    rtl::Reference< Content > xContent,
    const ucb::OpenCommandArgument2& rCommand,
    const uno::Reference< ucb::XCommandEnvironment >& rxEnv )
    : ucbhelper::ResultSetImplHelper( rxContext, rCommand ),
      m_xContent(std::move( xContent )),
      m_xEnv( rxEnv )
{
}

void DynamicResultSet::initStatic()
{
    // DEBUG: Force show dialog even without command environment for testing
    try {
        uno::Reference<uno::XComponentContext> xContext = ::comphelper::getProcessComponentContext();
        if (xContext.is()) {
            uno::Reference<task::XInteractionHandler> xIH(
                xContext->getServiceManager()->createInstanceWithContext(
                    u"com.sun.star.task.InteractionHandler"_ustr, xContext),
                uno::UNO_QUERY);
            if (xIH.is()) {
                rtl::Reference<ucbhelper::AuthenticationFallbackRequest> xRequest
                    = new ucbhelper::AuthenticationFallbackRequest(
                        u"DEBUG: DynamicResultSet::initStatic() called! Environment available: "_ustr +
                        (m_xEnv.is() ? u"YES" : u"NO"),
                        u"test://debug-resultset"_ustr);
                xIH->handle(xRequest);
            }
        }
    } catch (...) {
        // Ignore errors in debug code
    }

    m_xResultSet1 = new ::ucbhelper::ResultSet(
        m_xContext, m_aCommand.Properties,
        new DataSupplier( m_xContext, m_xContent, m_aCommand.Mode ), m_xEnv );
}

void DynamicResultSet::initDynamic()
{
    initStatic();
    m_xResultSet2 = m_xResultSet1;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */