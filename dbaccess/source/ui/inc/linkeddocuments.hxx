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

#pragma once

#include "AppElementType.hxx"

#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/sdb/application/XDatabaseDocumentUI.hpp>
#include <comphelper/namedvaluecollection.hxx>

namespace weld { class Window; }
namespace dbaui
{

    // OLinkedDocumentsAccess
    class OLinkedDocumentsAccess final
    {
        css::uno::Reference< css::uno::XComponentContext >
                    m_xContext;
        css::uno::Reference< css::container::XNameAccess >
                    m_xDocumentContainer;
        css::uno::Reference< css::sdbc::XConnection>
                    m_xConnection;
        css::uno::Reference< css::sdb::application::XDatabaseDocumentUI >
                    m_xDocumentUI;
        weld::Window* m_pDialogParent;
        OUString    m_sDataSourceName;

    public:
        OLinkedDocumentsAccess(
            weld::Window* pDialogParent,
            const css::uno::Reference< css::sdb::application::XDatabaseDocumentUI >& i_rDocumentUI,
            const css::uno::Reference< css::uno::XComponentContext >& _rxContext,
            const css::uno::Reference< css::container::XNameAccess >& _rxContainer,
            const css::uno::Reference< css::sdbc::XConnection>& _xConnection,
            const OUString& _sDataSourceName
        );
        ~OLinkedDocumentsAccess();

        bool isConnected() const { return m_xConnection.is(); }

        css::uno::Reference< css::lang::XComponent>
                open(
                    const OUString& _rLinkName,
                    css::uno::Reference< css::lang::XComponent>& _xDefinition,
                    ElementOpenMode _eOpenMode,
                    const ::comphelper::NamedValueCollection& _rAdditionalArgs
                );

        css::uno::Reference< css::lang::XComponent >
                newDocument(
                          sal_Int32 i_nActionID,
                    const ::comphelper::NamedValueCollection& i_rCreationArgs,
                          css::uno::Reference< css::lang::XComponent >& o_rDefinition
                );

        void    newFormWithPilot(
                    const sal_Int32 _nCommandType,
                    const OUString& _rObjectName
                );
        void    newReportWithPilot(
                    const sal_Int32 _nCommandType,
                    const OUString& _rObjectName
                );
        void    newQueryWithPilot();
        void    newTableWithPilot();

    private:
        css::uno::Reference< css::lang::XComponent >
            impl_open(
                const OUString& _rLinkName,
                css::uno::Reference< css::lang::XComponent >& _xDefinition,
                ElementOpenMode _eOpenMode,
                const ::comphelper::NamedValueCollection& _rAdditionalArgs
            );

        void
            impl_newWithPilot(
                const char* _pWizardService,
                const sal_Int32 _nCommandType,
                const OUString& _rObjectName
            );

    };

}   // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
