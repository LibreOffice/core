/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_DBACCESS_SOURCE_UI_INC_QUERYPROPERTIESDIALOG_HXX
#define INCLUDED_DBACCESS_SOURCE_UI_INC_QUERYPROPERTIESDIALOG_HXX

#include <vcl/dialog.hxx>
#include <vcl/button.hxx>
#include <rtl/ustring.hxx>
#include "LimitBox.hxx"

namespace dbaui
{

/**
 * Dialog to set such properties of a query as distinct values and limit
 * It can be opened form Edit menu in Query Design View
 */
class QueryPropertiesDialog : public ModalDialog
{

public:

    QueryPropertiesDialog(
        Window* pParent, const sal_Bool bDistinct, const sal_Int64 nLimit );
    virtual ~QueryPropertiesDialog();

    sal_Bool getDistinct() const;
    sal_Int64 getLimit() const;

private:

    RadioButton* m_pRB_Distinct;
    RadioButton* m_pRB_NonDistinct;

    LimitBox* m_pLB_Limit;

};


inline sal_Bool QueryPropertiesDialog::getDistinct() const
{
    return m_pRB_Distinct->IsChecked();
}

inline sal_Int64 QueryPropertiesDialog::getLimit() const
{
    return m_pLB_Limit->GetValue();
}


} ///dbaui namespace

#endif /// INCLUDED_DBACCESS_SOURCE_UI_INC_QUERYPROPERTIESDIALOG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
