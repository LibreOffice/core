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

#include <vcl/weld.hxx>

#define DOG_ROWS    3

namespace com::sun::star{
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

// DlgOrderCrit
namespace dbaui
{
    class DlgOrderCrit final : public weld::GenericDialogController
    {
        OUString               m_sOrgOrder;

        css::uno::Reference< css::sdb::XSingleSelectQueryComposer> m_xQueryComposer;
        css::uno::Reference< css::container::XNameAccess>          m_xColumns;
        css::uno::Reference< css::sdbc::XConnection>               m_xConnection;

        weld::ComboBox* m_aColumnList[DOG_ROWS];
        weld::ComboBox* m_aValueList[DOG_ROWS];

        std::unique_ptr<weld::ComboBox> m_xLB_ORDERFIELD1;
        std::unique_ptr<weld::ComboBox> m_xLB_ORDERVALUE1;
        std::unique_ptr<weld::ComboBox> m_xLB_ORDERFIELD2;
        std::unique_ptr<weld::ComboBox> m_xLB_ORDERVALUE2;
        std::unique_ptr<weld::ComboBox> m_xLB_ORDERFIELD3;
        std::unique_ptr<weld::ComboBox> m_xLB_ORDERVALUE3;

        DECL_LINK(FieldListSelectHdl, weld::ComboBox&, void);
        void            EnableLines();

    public:
        DlgOrderCrit(weld::Window * pParent,
                     const css::uno::Reference< css::sdbc::XConnection>& _rxConnection,
                     const css::uno::Reference< css::sdb::XSingleSelectQueryComposer>& _rxComposer,
                     const css::uno::Reference< css::container::XNameAccess>& _rxCols);
        virtual ~DlgOrderCrit() override;

        void            BuildOrderPart();

        OUString GetOrderList( ) const;
        const OUString& GetOriginalOrder() const { return m_sOrgOrder; }

    private:
        void            impl_initializeOrderList_nothrow();
    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
