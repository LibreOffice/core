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

#include <showcols.hxx>

#include <osl/diagnose.h>
#include <vcl/msgbox.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <comphelper/types.hxx>
#include <tools/debug.hxx>

#define CUIFM_PROP_HIDDEN "Hidden"
#define CUIFM_PROP_LABEL  "Label"

FmShowColsDialog::FmShowColsDialog(weld::Window* pParent)
    : GenericDialogController(pParent, "cui/ui/showcoldialog.ui", "ShowColDialog")
    , m_xList(m_xBuilder->weld_tree_view("treeview"))
    , m_xOK(m_xBuilder->weld_button("ok"))
{
    m_xList->set_size_request(m_xList->get_approximate_digit_width() * 40, m_xList->get_height_rows(8));
    m_xList->set_selection_mode(SelectionMode::Multiple);
    m_xOK->connect_clicked(LINK(this, FmShowColsDialog, OnClickedOk));
}

FmShowColsDialog::~FmShowColsDialog()
{
}

IMPL_LINK_NOARG(FmShowColsDialog, OnClickedOk, weld::Button&, void)
{
    DBG_ASSERT(m_xColumns.is(), "FmShowColsDialog::OnClickedOk : you should call SetColumns before executing the dialog !");
    if (m_xColumns.is())
    {
        css::uno::Reference< css::beans::XPropertySet > xCol;
        auto nSelectedRows = m_xList->get_selected_rows();
        for (auto i : nSelectedRows)
        {
            m_xColumns->getByIndex(m_xList->get_id(i).toInt32()) >>= xCol;
            if (xCol.is())
            {
                try
                {
                    xCol->setPropertyValue(CUIFM_PROP_HIDDEN, css::uno::Any(false));
                }
                catch(...)
                {
                    OSL_FAIL("FmShowColsDialog::OnClickedOk Exception occurred!");
                }
            }
        }
    }

    m_xDialog->response(RET_OK);
}

void FmShowColsDialog::SetColumns(const css::uno::Reference< css::container::XIndexContainer>& xCols)
{
    DBG_ASSERT(xCols.is(), "FmShowColsDialog::SetColumns : invalid columns !");
    if (!xCols.is())
        return;
    m_xColumns = xCols.get();

    m_xList->clear();

    css::uno::Reference< css::beans::XPropertySet>  xCurCol;
    OUString sCurName;
    for (sal_Int32 i=0; i<xCols->getCount(); ++i)
    {
        sCurName.clear();
        xCurCol.set(xCols->getByIndex(i), css::uno::UNO_QUERY);
        bool bIsHidden = false;
        try
        {
            css::uno::Any aHidden = xCurCol->getPropertyValue(CUIFM_PROP_HIDDEN);
            bIsHidden = ::comphelper::getBOOL(aHidden);

            OUString sName;
            xCurCol->getPropertyValue(CUIFM_PROP_LABEL) >>= sName;
            sCurName = sName;
        }
        catch(...)
        {
            OSL_FAIL("FmShowColsDialog::SetColumns Exception occurred!");
        }

        // if the col is hidden, put it into the list
        if (bIsHidden)
            m_xList->append(OUString::number(i), sCurName);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
