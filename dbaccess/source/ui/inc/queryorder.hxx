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
#ifndef INCLUDED_DBACCESS_SOURCE_UI_INC_QUERYORDER_HXX
#define INCLUDED_DBACCESS_SOURCE_UI_INC_QUERYORDER_HXX

#include <vcl/dialog.hxx>

#include <vcl/lstbox.hxx>

#include <vcl/edit.hxx>

#include <vcl/fixed.hxx>

#include <vcl/button.hxx>

#define DOG_ROWS    3

namespace com
{
    namespace sun
    {
        namespace star
        {
            namespace sdb
            {
                class XSingleSelectQueryComposer;
            }
            namespace sdbc
            {
                class XConnection;
            }
            namespace container
            {
                class XNameAccess;
            }
        }
    }
}

// DlgOrderCrit
namespace dbaui
{
    class DlgOrderCrit : public ModalDialog
    {
    protected:
        VclPtr<ListBox>        m_pLB_ORDERFIELD1;
        VclPtr<ListBox>        m_pLB_ORDERVALUE1;
        VclPtr<ListBox>        m_pLB_ORDERFIELD2;
        VclPtr<ListBox>        m_pLB_ORDERVALUE2;
        VclPtr<ListBox>        m_pLB_ORDERFIELD3;
        VclPtr<ListBox>        m_pLB_ORDERVALUE3;
        OUString               aSTR_NOENTRY;
        OUString               m_sOrgOrder;

        css::uno::Reference< css::sdb::XSingleSelectQueryComposer> m_xQueryComposer;
        css::uno::Reference< css::container::XNameAccess>          m_xColumns;
        css::uno::Reference< css::sdbc::XConnection>               m_xConnection;

        VclPtr<ListBox>        m_aColumnList[DOG_ROWS];
        VclPtr<ListBox>        m_aValueList[DOG_ROWS];

        DECL_LINK_TYPED( FieldListSelectHdl, ListBox&, void );
        void            EnableLines();

    public:
        DlgOrderCrit(   vcl::Window * pParent,
                        const css::uno::Reference< css::sdbc::XConnection>& _rxConnection,
                        const css::uno::Reference< css::sdb::XSingleSelectQueryComposer>& _rxComposer,
                        const css::uno::Reference< css::container::XNameAccess>& _rxCols);
        virtual ~DlgOrderCrit();
        virtual void dispose() override;

        void            BuildOrderPart();

        OUString GetOrderList( ) const;
        OUString GetOrignalOrder() const { return m_sOrgOrder; }

    private:
        void            impl_initializeOrderList_nothrow();
    };
}
#endif // INCLUDED_DBACCESS_SOURCE_UI_INC_QUERYORDER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
