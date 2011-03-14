/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"

#ifdef SW_DLLIMPLEMENTATION
#undef SW_DLLIMPLEMENTATION
#endif


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
