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

#include <string>
#include <tools/ref.hxx>
#include <tools/shl.hxx>
#include <svl/intitem.hxx>
#include <svl/eitem.hxx>
#include <svl/stritem.hxx>
#include <sfx2/dispatch.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/fixed.hxx>
#include <vcl/sound.hxx>
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
    m_aRecordNo.SetStrictFormat(TRUE);
    m_aRecordNo.SetDecimalDigits(0);

    FreeResource();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
