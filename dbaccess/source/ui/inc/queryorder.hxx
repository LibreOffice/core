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
#ifndef DBAUI_QUERYORDER_HXX
#define DBAUI_QUERYORDER_HXX

#include <vcl/dialog.hxx>

#include <vcl/lstbox.hxx>

#include <vcl/edit.hxx>

#include <vcl/fixed.hxx>

#include <vcl/button.hxx>

#define DOG_ROWS    3

namespace rtl
{
    class OUString;
}
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
            namespace beans
            {
                struct PropertyValue;
                class XPropertySet;
            }
        }
    }
}

//==================================================================
// DlgOrderCrit
//==================================================================
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
        ::rtl::OUString m_sOrgOrder;

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

        ::rtl::OUString GetOrderList( ) const;
        ::rtl::OUString GetOrignalOrder() const { return m_sOrgOrder; }

    private:
        void            impl_initializeOrderList_nothrow();
    };
}
#endif // DBAUI_QUERYORDER_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
