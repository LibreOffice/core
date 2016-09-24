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

FmInputRecordNoDialog::FmInputRecordNoDialog(vcl::Window * pParent)
    : ModalDialog( pParent, "RecordNumberDialog", "cui/ui/recordnumberdialog.ui")
{
    get(m_pRecordNo, "entry-nospin");

    m_pRecordNo->SetMin(1);
    m_pRecordNo->SetMax(0x7FFFFFFF);
    m_pRecordNo->SetStrictFormat(true);
    m_pRecordNo->SetDecimalDigits(0);
}

FmInputRecordNoDialog::~FmInputRecordNoDialog()
{
    disposeOnce();
}

void FmInputRecordNoDialog::dispose()
{
    m_pRecordNo.clear();
    ModalDialog::dispose();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
