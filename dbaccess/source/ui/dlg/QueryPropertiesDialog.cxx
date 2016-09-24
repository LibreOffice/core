/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "QueryPropertiesDialog.hxx"

namespace dbaui
{

QueryPropertiesDialog::QueryPropertiesDialog(
    vcl::Window* pParent, const bool bDistinct, const sal_Int64 nLimit )
    : ModalDialog(pParent, "QueryPropertiesDialog", "dbaccess/ui/querypropertiesdialog.ui")
    , m_pRB_Distinct( nullptr )
    , m_pRB_NonDistinct( nullptr )
    , m_pLB_Limit( nullptr )
{
    get( m_pRB_Distinct, "distinct" );
    get( m_pRB_NonDistinct, "nondistinct" );
    get( m_pLB_Limit, "limitbox" );

    m_pRB_Distinct->Check( bDistinct );
    m_pRB_NonDistinct->Check( !bDistinct );
    m_pLB_Limit->SetValue( nLimit );
}

QueryPropertiesDialog::~QueryPropertiesDialog()
{
    disposeOnce();
}

void QueryPropertiesDialog::dispose()
{
    m_pRB_Distinct.clear();
    m_pRB_NonDistinct.clear();
    m_pLB_Limit.clear();
    ModalDialog::dispose();
}

} ///dbaui namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
