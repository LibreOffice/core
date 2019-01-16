/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
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

#ifndef INCLUDED_CONNECTIVITY_SOURCE_INC_WRITER_WCONNECTION_HXX
#define INCLUDED_CONNECTIVITY_SOURCE_INC_WRITER_WCONNECTION_HXX

#include <memory>
#include <file/FConnection.hxx>
#include <com/sun/star/frame/XDesktop2.hpp>
#include <com/sun/star/frame/XTerminateListener.hpp>
#include <unotools/closeveto.hxx>

namespace com
{
namespace sun
{
namespace star
{
namespace text
{
class XTextDocument;
}
}
}
}

namespace utl
{
class CloseVeto;
}

namespace connectivity
{
namespace writer
{
class ODriver;
class OWriterConnection : public file::OConnection
{
    // the spreadsheet document:
    css::uno::Reference<css::text::XTextDocument> m_xDoc;
    OUString m_sPassword;
    OUString m_aFileName;
    oslInterlockedCount m_nDocCount;

    class CloseVetoButTerminateListener
        : public cppu::WeakComponentImplHelper<css::frame::XTerminateListener>
    {
    private:
        /// close listener that vetoes so nobody else disposes m_xDoc
        std::unique_ptr<utl::CloseVeto> m_pCloseListener;
        /// but also listen to XDesktop and if app is terminating anyway, dispose m_xDoc while
        /// its still possible to do so properly
        css::uno::Reference<css::frame::XDesktop2> m_xDesktop;
        osl::Mutex m_aMutex;

    public:
        CloseVetoButTerminateListener()
            : cppu::WeakComponentImplHelper<css::frame::XTerminateListener>(m_aMutex)
        {
        }

        void start(const css::uno::Reference<css::uno::XInterface>& rCloseable,
                   const css::uno::Reference<css::frame::XDesktop2>& rDesktop)
        {
            m_xDesktop = rDesktop;
            m_xDesktop->addTerminateListener(this);
            m_pCloseListener = std::make_unique<utl::CloseVeto>(rCloseable, true);
        }

        void stop()
        {
            m_pCloseListener.reset();
            if (!m_xDesktop.is())
                return;
            m_xDesktop->removeTerminateListener(this);
            m_xDesktop.clear();
        }

        // XTerminateListener
        void SAL_CALL queryTermination(const css::lang::EventObject& /*rEvent*/) override {}

        void SAL_CALL notifyTermination(const css::lang::EventObject& /*rEvent*/) override
        {
            stop();
        }

        void SAL_CALL disposing() override
        {
            stop();
            cppu::WeakComponentImplHelperBase::disposing();
        }

        void SAL_CALL disposing(const css::lang::EventObject& rEvent) override
        {
            const bool bShutDown = (rEvent.Source == m_xDesktop);
            if (bShutDown)
                stop();
        }
    };

    rtl::Reference<CloseVetoButTerminateListener> m_xCloseVetoButTerminateListener;

public:
    OWriterConnection(ODriver* _pDriver);
    ~OWriterConnection() override;

    void construct(const OUString& rURL,
                   const css::uno::Sequence<css::beans::PropertyValue>& rInfo) override;

    // XServiceInfo
    DECLARE_SERVICE_INFO();

    // OComponentHelper
    void SAL_CALL disposing() override;

    // XConnection
    css::uno::Reference<css::sdbc::XDatabaseMetaData> SAL_CALL getMetaData() override;
    css::uno::Reference<css::sdbcx::XTablesSupplier> createCatalog() override;
    css::uno::Reference<css::sdbc::XStatement> SAL_CALL createStatement() override;
    css::uno::Reference<css::sdbc::XPreparedStatement>
        SAL_CALL prepareStatement(const OUString& sql) override;
    css::uno::Reference<css::sdbc::XPreparedStatement>
        SAL_CALL prepareCall(const OUString& sql) override;

    // no interface methods
    css::uno::Reference<css::text::XTextDocument> const& acquireDoc();
    void releaseDoc();

    class ODocHolder
    {
        OWriterConnection* m_pConnection;
        css::uno::Reference<css::text::XTextDocument> m_xDoc;

    public:
        ODocHolder(OWriterConnection* _pConnection)
            : m_pConnection(_pConnection)
        {
            m_xDoc = m_pConnection->acquireDoc();
        }
        ~ODocHolder()
        {
            m_xDoc.clear();
            m_pConnection->releaseDoc();
        }
        const css::uno::Reference<css::text::XTextDocument>& getDoc() const { return m_xDoc; }
    };
};
}
}

#endif // INCLUDED_CONNECTIVITY_SOURCE_INC_WRITER_WCONNECTION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
