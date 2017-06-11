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

#include "showcols.hxx"
#include "strings.hrc"

#include <dialmgr.hxx>
#include <vcl/msgbox.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <comphelper/types.hxx>

#define CUIFM_PROP_HIDDEN "Hidden"
#define CUIFM_PROP_LABEL  "Label"

FmShowColsDialog::FmShowColsDialog(vcl::Window* pParent)
    : ModalDialog(pParent, "ShowColDialog", "cui/ui/showcoldialog.ui")
{
    get(m_pOK, "ok");
    get(m_pList, "treeview");
    m_pList->set_height_request(m_pList->GetTextHeight() * 8);
    m_pList->set_width_request(m_pList->approximate_char_width() * 56);
    m_pList->EnableMultiSelection(true);
    m_pOK->SetClickHdl( LINK( this, FmShowColsDialog, OnClickedOk ) );
}

FmShowColsDialog::~FmShowColsDialog()
{
    disposeOnce();
}

void FmShowColsDialog::dispose()
{
    m_pList.clear();
    m_pOK.clear();
    ModalDialog::dispose();
}

IMPL_LINK_NOARG(FmShowColsDialog, OnClickedOk, Button*, void)
{
    DBG_ASSERT(m_xColumns.is(), "FmShowColsDialog::OnClickedOk : you should call SetColumns before executing the dialog !");
    if (m_xColumns.is())
    {
        css::uno::Reference< css::beans::XPropertySet > xCol;
        for (sal_Int32 i=0; i < m_pList->GetSelectEntryCount(); ++i)
        {
            m_xColumns->getByIndex(sal::static_int_cast<sal_Int32>(reinterpret_cast<sal_uIntPtr>(m_pList->GetEntryData(m_pList->GetSelectEntryPos(i))))) >>= xCol;
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

    EndDialog(RET_OK);
}


void FmShowColsDialog::SetColumns(const css::uno::Reference< css::container::XIndexContainer>& xCols)
{
    DBG_ASSERT(xCols.is(), "FmShowColsDialog::SetColumns : invalid columns !");
    if (!xCols.is())
        return;
    m_xColumns = xCols.get();

    m_pList->Clear();

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
            m_pList->SetEntryData( m_pList->InsertEntry(sCurName), reinterpret_cast<void*>((sal_Int64)i) );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
