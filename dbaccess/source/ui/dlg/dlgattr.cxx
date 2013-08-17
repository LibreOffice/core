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

#include "dlgattr.hxx"

#include <sfx2/tabdlg.hxx>

#include <svx/numinf.hxx>

#include <svx/dialogs.hrc>
#include "dbu_dlg.hrc"
#include <svl/itemset.hxx>
#include <svl/zforlist.hxx>
#include <svx/svxids.hrc>
#include <svx/flagsdef.hxx>
#include <svl/intitem.hxx>
#include "moduledbu.hxx"
using namespace dbaui;

DBG_NAME(SbaSbAttrDlg)
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

SbaSbAttrDlg::~SbaSbAttrDlg()
{
    delete pNumberInfoItem;

    DBG_DTOR(SbaSbAttrDlg,NULL);
}

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

        default:
        break;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
