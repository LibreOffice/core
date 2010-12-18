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

#ifndef _DBAUI_LINKEDDOCUMENTS_HXX_
#define _DBAUI_LINKEDDOCUMENTS_HXX_

#include "AppElementType.hxx"

#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/ucb/XContent.hpp>
#include <com/sun/star/sdb/application/XDatabaseDocumentUI.hpp>
#include <tools/link.hxx>
#include <tools/string.hxx>
#include <comphelper/namedvaluecollection.hxx>

class Window;
//......................................................................
namespace dbaui
{
//......................................................................

    //==================================================================
    //= OLinkedDocumentsAccess
    //==================================================================
    class OLinkedDocumentsAccess
    {
    protected:
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >
                    m_xORB;
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >
                    m_xDocumentContainer;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>
                    m_xConnection;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdb::application::XDatabaseDocumentUI >
                    m_xDocumentUI;
        Window*     m_pDialogParent;
        String      m_sCurrentlyEditing;
        ::rtl::OUString
                    m_sDataSourceName;

    public:
        OLinkedDocumentsAccess(
            Window* _pDialogParent,
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::application::XDatabaseDocumentUI >& i_rDocumentUI,
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB,
            const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& _rxContainer,
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>& _xConnection,
            const ::rtl::OUString& _sDataSourceName
        );
        ~OLinkedDocumentsAccess();

        inline sal_Bool isConnected() const { return m_xConnection.is(); }

        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent>
                open(
                    const ::rtl::OUString& _rLinkName,
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
                    const ::rtl::OUString& _rObjectName = ::rtl::OUString()
                );
        void    newReportWithPilot(
                    const sal_Int32 _nCommandType = -1,
                    const ::rtl::OUString& _rObjectName = ::rtl::OUString()
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
                const ::rtl::OUString& _rLinkName,
                ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >& _xDefinition,
                ElementOpenMode _eOpenMode,
                const ::comphelper::NamedValueCollection& _rAdditionalArgs
            );

        void
            impl_newWithPilot(
                const char* _pWizardService,
                const sal_Int32 _nCommandType,
                const ::rtl::OUString& _rObjectName
            );

    };

//......................................................................
}   // namespace dbaui
//......................................................................

#endif // _DBAUI_LINKEDDOCUMENTS_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
