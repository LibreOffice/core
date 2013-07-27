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

#include "swtypes.hxx"

#include "selglos.hxx"

#include "selglos.hrc"
#include "dochdl.hrc"
#include <vcl/msgbox.hxx>

// STATIC DATA -----------------------------------------------------------


// CTOR / DTOR -----------------------------------------------------------

SwSelGlossaryDlg::SwSelGlossaryDlg(Window * pParent, const OUString &rShortName)
    : ModalDialog(pParent, SW_RES(DLG_SEL_GLOS)),
    aGlosBox(this, SW_RES( LB_GLOS)),
    aGlosFL(this, SW_RES( FL_GLOS)),
    aOKBtn(this, SW_RES( BT_OK)),
    aCancelBtn(this, SW_RES( BT_CANCEL)),
    aHelpBtn(this, SW_RES(BT_HELP))
{
    aGlosFL.SetText(aGlosFL.GetText() + rShortName);
    FreeResource();

    aGlosBox.SetDoubleClickHdl(LINK(this, SwSelGlossaryDlg, DoubleClickHdl));
}

// overload dtor
SwSelGlossaryDlg::~SwSelGlossaryDlg() {}

IMPL_LINK(SwSelGlossaryDlg, DoubleClickHdl, ListBox*, /*pBox*/)
{
    EndDialog(RET_OK);
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
