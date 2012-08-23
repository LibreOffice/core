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
#include "hintids.hxx"

#include "dialog.hrc"
#include "abstract.hxx"
#include "abstract.hrc"


SwInsertAbstractDlg::SwInsertAbstractDlg( Window* pParent ) :
    SfxModalDialog(pParent, SW_RES(DLG_INSERT_ABSTRACT)),
    aFL     (this, SW_RES(FL_1       )),
    aLevelFT(this, SW_RES(FT_LEVEL   )),
    aLevelNF(this, SW_RES(NF_LEVEL   )),
    aParaFT (this, SW_RES(FT_PARA   )),
    aParaNF (this, SW_RES(NF_PARA   )),
    aDescFT (this, SW_RES(FT_DESC   )),
    aOkPB   (this, SW_RES(PB_OK     )),
    aCancelPB (this, SW_RES(PB_CANCEL   )),
    aHelpPB (this, SW_RES(PB_HELP   ))
{
    FreeResource();
}

SwInsertAbstractDlg::~SwInsertAbstractDlg()
{
}

sal_uInt8 SwInsertAbstractDlg::GetLevel() const
{
    return static_cast< sal_uInt8 >(aLevelNF.GetValue() - 1);
}

sal_uInt8 SwInsertAbstractDlg::GetPara() const
{
    return (sal_uInt8) aParaNF.GetValue();
}






/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
