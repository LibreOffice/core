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

#include <comphelper/string.hxx>
#include <vcl/msgbox.hxx>
#include <comphelper/string.hxx>
#include <vcl/msgbox.hxx>
#include "reffact.hxx"
#include "document.hxx"
#include "scresid.hxx"
#include "globstr.hrc"
#include "dbnamdlg.hrc"
#include "rangenam.hxx"     // IsNameValid
#include "globalnames.hxx"
#include "dbformattingdlg.hxx"

ScDbFormattingDlg::ScDbFormattingDlg( Window* pParent )
    :   ModalDialog( pParent, "DbDataFormatting", "modules/scalc/ui/dbdataformatting.ui" )
{
}

ScDbFormattingDlg::~ScDbFormattingDlg()
{
}

void ScDbFormattingDlg::SetReference( const ScRange& rRef, ScDocument* pDocP )
{
}

sal_Bool ScDbFormattingDlg::Close()
{
    return true;
}

void ScDbFormattingDlg::SetActive()
{
    //aEdAssign.GrabFocus();
    //RefInputDone();
}

sal_Bool ScDbFormattingDlg::IsRefInputMode() const
{
    return false;
}
