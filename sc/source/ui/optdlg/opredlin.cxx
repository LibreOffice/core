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

#undef SC_DLLIMPLEMENTATION

#include <svx/dlgutil.hxx>
#include <svx/drawitem.hxx>
#include <svx/xtable.hxx>

#include "appoptio.hxx"
#include "scmod.hxx"
#include "scitems.hxx"
#include "tpview.hxx"
#include "global.hxx"
#include "viewopti.hxx"
#include "tabvwsh.hxx"
#include "uiitems.hxx"
#include "scresid.hxx"
#include "docsh.hxx"
#include "sc.hrc"
#include "globstr.hrc"

#include "opredlin.hxx"

ScRedlineOptionsTabPage::ScRedlineOptionsTabPage( vcl::Window* pParent,
                                                    const SfxItemSet& rSet )
    : SfxTabPage(pParent,"OptChangesPage", "modules/scalc/ui/optchangespage.ui", &rSet),
    aAuthorStr      (ScResId(SCSTR_AUTHOR))
{
    get(m_pContentColorLB, "changes");
    get(m_pRemoveColorLB, "deletions");
    get(m_pInsertColorLB, "entries");
    get(m_pMoveColorLB, "insertions");
}

ScRedlineOptionsTabPage::~ScRedlineOptionsTabPage()
{
    disposeOnce();
}

void ScRedlineOptionsTabPage::dispose()
{
    m_pContentColorLB.clear();
    m_pRemoveColorLB.clear();
    m_pInsertColorLB.clear();
    m_pMoveColorLB.clear();
    SfxTabPage::dispose();
}

VclPtr<SfxTabPage> ScRedlineOptionsTabPage::Create( vcl::Window* pParent, const SfxItemSet* rSet )
{
    return VclPtr<ScRedlineOptionsTabPage>::Create( pParent, *rSet );
}

bool ScRedlineOptionsTabPage::FillItemSet( SfxItemSet* /* rSet */ )
{
    ScAppOptions aAppOptions=SC_MOD()->GetAppOptions();

    sal_uLong nNew=0;
    sal_Int32 nPos=0;

    nPos = m_pContentColorLB->GetSelectEntryPos();
    if (nPos != LISTBOX_ENTRY_NOTFOUND)
    {
        if (nPos!=0)
            nNew= m_pContentColorLB->GetEntryColor(nPos).GetColor();
        else
            nNew= COL_TRANSPARENT;

        aAppOptions.SetTrackContentColor(nNew);

    }
    nPos = m_pMoveColorLB->GetSelectEntryPos();
    if (nPos != LISTBOX_ENTRY_NOTFOUND)
    {
        if (nPos!=0)
            nNew= m_pMoveColorLB->GetEntryColor(nPos).GetColor();
        else
            nNew= COL_TRANSPARENT;

        aAppOptions.SetTrackMoveColor(nNew);

    }
    nPos = m_pInsertColorLB->GetSelectEntryPos();
    if (nPos != LISTBOX_ENTRY_NOTFOUND)
    {
        if (nPos!=0)
            nNew= m_pInsertColorLB->GetEntryColor(nPos).GetColor();
        else
            nNew= COL_TRANSPARENT;

        aAppOptions.SetTrackInsertColor(nNew);

    }
    nPos = m_pRemoveColorLB->GetSelectEntryPos();
    if (nPos != LISTBOX_ENTRY_NOTFOUND)
    {
        if (nPos!=0)
            nNew= m_pRemoveColorLB->GetEntryColor(nPos).GetColor();
        else
            nNew= COL_TRANSPARENT;

        aAppOptions.SetTrackDeleteColor(nNew);

    }

    SC_MOD()->SetAppOptions(aAppOptions);

    //  Repaint (wenn alles ueber Items laufen wuerde, wie es sich gehoert,
    //  waere das nicht noetig...)
    ScDocShell* pDocSh = dynamic_cast<ScDocShell*>( SfxObjectShell::Current() );
    if (pDocSh)
        pDocSh->PostPaintGridAll();

    return false;
}

void ScRedlineOptionsTabPage::Reset( const SfxItemSet* /* rSet */ )
{
    m_pContentColorLB->InsertEntry(aAuthorStr);
    m_pMoveColorLB->InsertEntry(aAuthorStr);
    m_pInsertColorLB->InsertEntry(aAuthorStr);
    m_pRemoveColorLB->InsertEntry(aAuthorStr);

    m_pContentColorLB->SetUpdateMode( false);
    m_pMoveColorLB->SetUpdateMode( false);
    m_pInsertColorLB->SetUpdateMode( false);
    m_pRemoveColorLB->SetUpdateMode( false);

    XColorListRef xColorLst = XColorList::GetStdColorList();
    for( long i = 0; i < xColorLst->Count(); ++i )
    {
        XColorEntry* pEntry = xColorLst->GetColor( i );
        Color aColor = pEntry->GetColor();
        OUString sName = pEntry->GetName();

        m_pContentColorLB->InsertEntry( aColor, sName );
        m_pMoveColorLB->InsertEntry( aColor, sName );
        m_pInsertColorLB->InsertEntry( aColor, sName );
        m_pRemoveColorLB->InsertEntry( aColor, sName );
    }
    m_pContentColorLB->SetUpdateMode( true );
    m_pMoveColorLB->SetUpdateMode( true );
    m_pInsertColorLB->SetUpdateMode( true );
    m_pRemoveColorLB->SetUpdateMode( true );

    ScAppOptions aAppOptions=SC_MOD()->GetAppOptions();

    sal_uLong nColor = aAppOptions.GetTrackContentColor();
    if (nColor == COL_TRANSPARENT)
        m_pContentColorLB->SelectEntryPos(0);
    else
        m_pContentColorLB->SelectEntry(Color(nColor));

    nColor = aAppOptions.GetTrackMoveColor();
    if (nColor == COL_TRANSPARENT)
        m_pMoveColorLB->SelectEntryPos(0);
    else
        m_pMoveColorLB->SelectEntry(Color(nColor));

    nColor = aAppOptions.GetTrackInsertColor();
    if (nColor == COL_TRANSPARENT)
        m_pInsertColorLB->SelectEntryPos(0);
    else
        m_pInsertColorLB->SelectEntry(Color(nColor));

    nColor = aAppOptions.GetTrackDeleteColor();
    if (nColor == COL_TRANSPARENT)
        m_pRemoveColorLB->SelectEntryPos(0);
    else
        m_pRemoveColorLB->SelectEntry(Color(nColor));

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
