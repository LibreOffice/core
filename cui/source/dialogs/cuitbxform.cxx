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

#include <string>
#include <tools/shl.hxx>
#include <svl/intitem.hxx>
#include <svl/eitem.hxx>
#include <svl/stritem.hxx>
#include <sfx2/dispatch.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/fixed.hxx>
#include <dialmgr.hxx>
#include <cuires.hrc>
#include "cuitbxform.hxx"
#include <sfx2/viewfrm.hxx>
#include <sfx2/viewsh.hxx>
#include "fmsearch.hrc"

//========================================================================
// class FmInputRecordNoDialog
//========================================================================

FmInputRecordNoDialog::FmInputRecordNoDialog(Window * pParent)
    :ModalDialog( pParent, CUI_RES(RID_SVX_DLG_INPUTRECORDNO))
    ,m_aLabel(this, CUI_RES(1))
    ,m_aRecordNo(this, CUI_RES(1))
    ,m_aOk(this, CUI_RES(1))
    ,m_aCancel(this, CUI_RES(1))
{
    m_aRecordNo.SetMin(1);
    m_aRecordNo.SetMax(0x7FFFFFFF);
    m_aRecordNo.SetStrictFormat(sal_True);
    m_aRecordNo.SetDecimalDigits(0);

    FreeResource();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
