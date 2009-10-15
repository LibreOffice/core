/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: cuitbxform.cxx,v $
 * $Revision: 1.10 $
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
#include "precompiled_svx.hxx"

#ifdef SVX_DLLIMPLEMENTATION
#undef SVX_DLLIMPLEMENTATION
#endif

#include <string> // HACK: prevent conflict between STLPORT and Workshop headers
#include <tools/ref.hxx>
#include <tools/shl.hxx>
#include <svl/intitem.hxx>
#include <svl/eitem.hxx>
#include <svl/stritem.hxx>
#include <sfx2/dispatch.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/fixed.hxx>
#include "fmitems.hxx"
#include <vcl/sound.hxx>
#include <svx/dialmgr.hxx>
#ifndef _SVX_DIALOGS_HRC
#include <svx/dialogs.hrc>
#endif
/*
#include "tbxctl.hxx"
#include "tbxdraw.hxx"
*/
#include "cuitbxform.hxx"
#ifndef _SVX_FMRESIDS_HRC
#include "fmresids.hrc"
#endif
#include "fmitems.hxx"
#ifndef _SVX_FMHELP_HRC
#include "fmhelp.hrc"
#endif
#include <sfx2/viewfrm.hxx>
#include <sfx2/viewsh.hxx>
//========================================================================
// class FmInputRecordNoDialog
//========================================================================

FmInputRecordNoDialog::FmInputRecordNoDialog(Window * pParent)
    :ModalDialog( pParent, SVX_RES(RID_SVX_DLG_INPUTRECORDNO))
    ,m_aLabel(this, SVX_RES(1))
    ,m_aRecordNo(this, SVX_RES(1))
    ,m_aOk(this, SVX_RES(1))
    ,m_aCancel(this, SVX_RES(1))
{
    m_aRecordNo.SetMin(1);
    m_aRecordNo.SetMax(0x7FFFFFFF);
    m_aRecordNo.SetStrictFormat(TRUE);
    m_aRecordNo.SetDecimalDigits(0);

    FreeResource();
}
