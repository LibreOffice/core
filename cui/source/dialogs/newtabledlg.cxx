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

#include "dialmgr.hxx"
#include "newtabledlg.hxx"

SvxNewTableDialog::SvxNewTableDialog()
    : m_pDialog( VclPtr<ModalDialog>::Create( nullptr, "NewTableDialog", "cui/ui/newtabledialog.ui" ) )
{
    m_pDialog->get(mpNumRows, "rows");
    m_pDialog->get(mpNumColumns, "columns");
}

SvxNewTableDialog::~SvxNewTableDialog()
{
    disposeOnce();
}

void SvxNewTableDialog::dispose()
{
    mpNumColumns.clear();
    mpNumRows.clear();
    m_pDialog.disposeAndClear();
    SvxAbstractNewTableDialog::dispose();
}

short SvxNewTableDialog::Execute()
{
    return m_pDialog->Execute();
}

sal_Int32 SvxNewTableDialog::getRows() const
{
    return sal::static_int_cast< sal_Int32 >( mpNumRows->GetValue() );
}

sal_Int32 SvxNewTableDialog::getColumns() const
{
    return sal::static_int_cast< sal_Int32 >( mpNumColumns->GetValue() );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
