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

#include "cuires.hrc"
#include "dialmgr.hxx"
#include "newtabledlg.hxx"
#include "newtabledlg.hrc"

SvxNewTableDialog::SvxNewTableDialog( Window* pParent )
: ModalDialog( pParent, CUI_RES( RID_SVX_NEWTABLE_DLG ) )
, maFtColumns( this, CUI_RES( FT_COLUMNS ) )
, maNumColumns( this, CUI_RES( NF_COLUMNS ) )
, maFtRows( this, CUI_RES( FT_ROWS ) )
, maNumRows( this, CUI_RES( NF_ROWS ) )
, maFlSep( this, CUI_RES( FL_SEP ) )
, maHelpButton( this, CUI_RES( BTN_HELP ) )
, maOkButton( this, CUI_RES( BTN_OK ) )
, maCancelButton( this, CUI_RES( BTN_CANCEL ) )
{
    maNumRows.SetValue(2);
    maNumColumns.SetValue(5);
    FreeResource();
}

short SvxNewTableDialog::Execute(void)
{
    return ModalDialog::Execute();
}

void SvxNewTableDialog::Apply(void)
{
}

sal_Int32 SvxNewTableDialog::getRows() const
{
    return sal::static_int_cast< sal_Int32 >( maNumRows.GetValue() );
}

sal_Int32 SvxNewTableDialog::getColumns() const
{
    return sal::static_int_cast< sal_Int32 >( maNumColumns.GetValue() );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
