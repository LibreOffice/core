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
#include "precompiled_dbaccess.hxx"


#include "dlgattr.hxx"

#include <sfx2/tabdlg.hxx>

#include <svx/numinf.hxx>

#include <svx/numinf.hxx>

#include <svx/dialogs.hrc>
#include "dbu_dlg.hrc"
#include <svl/itemset.hxx>
#define _ZFORLIST_DECLARE_TABLE
#include <svl/zforlist.hxx>
#include <svx/svxids.hrc>
#include <svx/flagsdef.hxx>
#include <svl/intitem.hxx>
#include "moduledbu.hxx"
using namespace dbaui;

DBG_NAME(SbaSbAttrDlg)
//==================================================================
SbaSbAttrDlg::SbaSbAttrDlg(Window* pParent, const SfxItemSet* pCellAttrs, SvNumberFormatter* pFormatter, sal_uInt16 nFlags, sal_Bool bRow)
             : SfxTabDialog(pParent, ModuleRes( DLG_ATTR ), pCellAttrs )
             ,aTitle(ModuleRes(ST_ROW))
{
    DBG_CTOR(SbaSbAttrDlg,NULL);

    pNumberInfoItem = new SvxNumberInfoItem( pFormatter, 0 );

    if (bRow)
        SetText(aTitle);
    if( nFlags & TP_ATTR_CHAR )
    {
        OSL_FAIL( "found flag TP_ATTR_CHAR" );
    }
    if( nFlags & TP_ATTR_NUMBER )
        AddTabPage( RID_SVXPAGE_NUMBERFORMAT,String(ModuleRes(TP_ATTR_NUMBER)) );
    if( nFlags & TP_ATTR_ALIGN )
        AddTabPage( RID_SVXPAGE_ALIGNMENT,String(ModuleRes(TP_ATTR_ALIGN)) );
    FreeResource();
}

// -----------------------------------------------------------------------
SbaSbAttrDlg::~SbaSbAttrDlg()
{
    delete pNumberInfoItem;

    DBG_DTOR(SbaSbAttrDlg,NULL);
}

// -----------------------------------------------------------------------
void SbaSbAttrDlg::PageCreated( sal_uInt16 nPageId, SfxTabPage& rTabPage )
{
    SfxAllItemSet aSet(*(GetInputSetImpl()->GetPool()));
    switch ( nPageId )
    {
        case RID_SVXPAGE_NUMBERFORMAT:
        {
            aSet.Put (SvxNumberInfoItem( pNumberInfoItem->GetNumberFormatter(), (const sal_uInt16)SID_ATTR_NUMBERFORMAT_INFO));
            rTabPage.PageCreated(aSet);
        }
        break;

        case RID_SVXPAGE_CHAR_STD:
        {
        }
        break;

        case RID_SVXPAGE_ALIGNMENT:
        {
        }
        break;

        default:
        break;
    }
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
