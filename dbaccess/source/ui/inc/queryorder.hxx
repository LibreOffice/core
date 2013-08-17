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
#ifndef DBAUI_QUERYORDER_HXX
#define DBAUI_QUERYORDER_HXX

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
        ListBox         aLB_ORDERFIELD1;
        ListBox         aLB_ORDERVALUE1;
        ListBox         aLB_ORDERFIELD2;
        ListBox         aLB_ORDERVALUE2;
        ListBox         aLB_ORDERFIELD3;
        ListBox         aLB_ORDERVALUE3;
        FixedText       aFT_ORDERFIELD;
        FixedText       aFT_ORDERAFTER1;
        FixedText       aFT_ORDERAFTER2;
        FixedText       aFT_ORDEROPER;
        FixedText       aFT_ORDERDIR;
        OKButton        aBT_OK;
        CancelButton    aBT_CANCEL;
        HelpButton      aBT_HELP;
        FixedLine       aFL_ORDER;
        String          aSTR_NOENTRY;
        OUString m_sOrgOrder;

        ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XSingleSelectQueryComposer> m_xQueryComposer;
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess> m_xColumns;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>      m_xConnection;

        ListBox*        m_aColumnList[DOG_ROWS];
        ListBox*        m_aValueList[DOG_ROWS];

        DECL_LINK( FieldListSelectHdl, ListBox * );
        void            EnableLines();

    public:
        DlgOrderCrit(   Window * pParent,
                        const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>& _rxConnection,
                        const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XSingleSelectQueryComposer>& _rxComposer,
                        const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess>& _rxCols);

                        ~DlgOrderCrit();
        void            BuildOrderPart();

        OUString GetOrderList( ) const;
        OUString GetOrignalOrder() const { return m_sOrgOrder; }

    private:
        void            impl_initializeOrderList_nothrow();
    };
}
#endif // DBAUI_QUERYORDER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
