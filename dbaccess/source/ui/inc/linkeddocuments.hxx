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

#ifndef _DBAUI_LINKEDDOCUMENTS_HXX_
#define _DBAUI_LINKEDDOCUMENTS_HXX_

#include "AppElementType.hxx"

#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/ucb/XContent.hpp>
#include <com/sun/star/sdb/application/XDatabaseDocumentUI.hpp>
#include <tools/string.hxx>
#include <comphelper/namedvaluecollection.hxx>

class Window;
namespace dbaui
{

    // OLinkedDocumentsAccess
    class OLinkedDocumentsAccess
    {
    protected:
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >
                    m_xContext;
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >
                    m_xDocumentContainer;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>
                    m_xConnection;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdb::application::XDatabaseDocumentUI >
                    m_xDocumentUI;
        Window*     m_pDialogParent;
        String      m_sCurrentlyEditing;
        OUString
                    m_sDataSourceName;

    public:
        OLinkedDocumentsAccess(
            Window* _pDialogParent,
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::application::XDatabaseDocumentUI >& i_rDocumentUI,
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _rxContext,
            const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& _rxContainer,
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>& _xConnection,
            const OUString& _sDataSourceName
        );
        ~OLinkedDocumentsAccess();

        inline sal_Bool isConnected() const { return m_xConnection.is(); }

        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent>
                open(
                    const OUString& _rLinkName,
                    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent>& _xDefinition,
                    ElementOpenMode _eOpenMode,
                    const ::comphelper::NamedValueCollection& _rAdditionalArgs
                );

        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >
                newDocument(
                          sal_Int32 i_nActionID,
                    const ::comphelper::NamedValueCollection& i_rCreationArgs,
                          ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >& o_rDefinition
                );

        void    newFormWithPilot(
                    const sal_Int32 _nCommandType = -1,
                    const OUString& _rObjectName = OUString()
                );
        void    newReportWithPilot(
                    const sal_Int32 _nCommandType = -1,
                    const OUString& _rObjectName = OUString()
                );
        void    newQueryWithPilot();
        void    newTableWithPilot();

        enum RESULT
        {
            ERROR,
            SUCCESS,
            CANCEL
        };
    private:
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >
            impl_open(
                const OUString& _rLinkName,
                ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >& _xDefinition,
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

#endif // _DBAUI_LINKEDDOCUMENTS_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
