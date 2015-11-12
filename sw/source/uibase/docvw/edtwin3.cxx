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

#include <hintids.hxx>
#include <vcl/settings.hxx>
#include <svx/ruler.hxx>
#include <viewopt.hxx>
#include "view.hxx"
#include "wrtsh.hxx"
#include "basesh.hxx"
#include "pview.hxx"
#include "mdiexp.hxx"
#include "edtwin.hxx"
#include "swmodule.hxx"
#include "modcfg.hxx"
#include "swtable.hxx"
#include "docsh.hxx"
#include "pagedesc.hxx"
#include <frmatr.hxx>
#include <editeng/frmdiritem.hxx>

// Core-Notify
void ScrollMDI( SwViewShell* pVwSh, const SwRect &rRect,
                sal_uInt16 nRangeX, sal_uInt16 nRangeY)
{
    SfxViewShell *pSfxVwSh = pVwSh->GetSfxViewShell();
    if (pSfxVwSh && dynamic_cast< const SwView *>( pSfxVwSh ) !=  nullptr)
        static_cast<SwView *>(pSfxVwSh)->Scroll( rRect.SVRect(), nRangeX, nRangeY );
}

// Docmdi - movable
bool IsScrollMDI( SwViewShell* pVwSh, const SwRect &rRect )
{
    SfxViewShell *pSfxVwSh = pVwSh->GetSfxViewShell();
    if (pSfxVwSh && dynamic_cast< const SwView *>( pSfxVwSh ) !=  nullptr)
        return static_cast<SwView *>(pSfxVwSh)->IsScroll(rRect.SVRect());
    return false;
}

// Notify for size change
void SizeNotify(SwViewShell* pVwSh, const Size &rSize)
{
    SfxViewShell *pSfxVwSh = pVwSh->GetSfxViewShell();
    if (pSfxVwSh)
    {
        if (dynamic_cast< const SwView *>( pSfxVwSh ) !=  nullptr)
            static_cast<SwView *>(pSfxVwSh)->DocSzChgd(rSize);
        else if (dynamic_cast< const SwPagePreview *>( pSfxVwSh ) !=  nullptr)
            static_cast<SwPagePreview *>(pSfxVwSh)->DocSzChgd( rSize );
    }
}

// Notify for page number update
void PageNumNotify( SwViewShell* pVwSh, sal_uInt16 nPhyNum, sal_uInt16 nVirtNum,
                                                    const OUString& rPgStr)
{
    SfxViewShell *pSfxVwSh = pVwSh->GetSfxViewShell();
    if ( pSfxVwSh && dynamic_cast< const SwView *>( pSfxVwSh ) !=  nullptr &&
         static_cast<SwView*>(pSfxVwSh)->GetCurShell() )
            static_cast<SwView *>(pSfxVwSh)->UpdatePageNums(nPhyNum, nVirtNum, rPgStr);
}

void FrameNotify( SwViewShell* pVwSh, FlyMode eMode )
{
    if ( dynamic_cast< const SwCrsrShell *>( pVwSh ) !=  nullptr )
        SwBaseShell::SetFrmMode( eMode, static_cast<SwWrtShell*>(pVwSh) );
}

// Notify for page number update
bool SwEditWin::RulerColumnDrag( const MouseEvent& rMEvt, bool bVerticalMode)
{
    SvxRuler& rRuler = bVerticalMode ?  m_rView.GetVRuler() : m_rView.GetHRuler();
    return (!rRuler.StartDocDrag( rMEvt, RULER_TYPE_BORDER ) &&
            !rRuler.StartDocDrag( rMEvt, RULER_TYPE_MARGIN1) &&
            !rRuler.StartDocDrag( rMEvt, RULER_TYPE_MARGIN2));
}

// #i23726#
// #i42921# - add 3rd parameter <bVerticalMode> in order
// to consider vertical layout
bool SwEditWin::RulerMarginDrag( const MouseEvent& rMEvt,
                                 const bool bVerticalMode )
{
    SvxRuler& rRuler = bVerticalMode ?  m_rView.GetVRuler() : m_rView.GetHRuler();
    return !rRuler.StartDocDrag( rMEvt, RULER_TYPE_INDENT);
}

TableChgMode GetTableChgDefaultMode()
{
    SwModuleOptions* pOpt = SW_MOD()->GetModuleConfig();
    return pOpt ? pOpt->GetTableMode() : TBLVAR_CHGABS;
}

void RepaintPagePreview( SwViewShell* pVwSh, const SwRect& rRect )
{
    SfxViewShell *pSfxVwSh = pVwSh->GetSfxViewShell();
    if (pSfxVwSh && dynamic_cast< const SwPagePreview *>( pSfxVwSh ) !=  nullptr)
        static_cast<SwPagePreview *>(pSfxVwSh)->RepaintCoreRect( rRect );
}

bool JumpToSwMark( SwViewShell* pVwSh, const OUString& rMark )
{
    SfxViewShell *pSfxVwSh = pVwSh->GetSfxViewShell();
    if( pSfxVwSh && dynamic_cast< const SwView *>( pSfxVwSh ) !=  nullptr )
        return static_cast<SwView *>(pSfxVwSh)->JumpToSwMark( rMark );
    return false;
}

void SwEditWin::DataChanged( const DataChangedEvent& rDCEvt )
{
    Window::DataChanged( rDCEvt );

    SwWrtShell* pSh = GetView().GetWrtShellPtr();
    // DataChanged() is sometimes called prior to creating
    // the SwWrtShell
    if(!pSh)
        return;
    bool bViewWasLocked = pSh->IsViewLocked(), bUnlockPaint = false;
    pSh->LockView( true );
    switch( rDCEvt.GetType() )
    {
    case DataChangedEventType::SETTINGS:
        // rearrange ScrollBars, respectively trigger resize, because
        // the ScrollBar size can have change. For that, in the reset
        // handler, the size of the ScrollBars also has to be queried
        // from the settings.
        if( rDCEvt.GetFlags() & AllSettingsFlags::STYLE )
        {
            pSh->LockPaint();
            bUnlockPaint = true;
            pSh->DeleteReplacementBitmaps();
            GetView().InvalidateBorder();               //Scrollbar work
        }
        break;

    case DataChangedEventType::PRINTER:
    case DataChangedEventType::DISPLAY:
    case DataChangedEventType::FONTS:
    case DataChangedEventType::FONTSUBSTITUTION:
        pSh->LockPaint();
        bUnlockPaint = true;
        GetView().GetDocShell()->UpdateFontList();  //e.g. printer change
        break;
    default: break;
    }
    pSh->LockView( bViewWasLocked );
    if( bUnlockPaint )
        pSh->UnlockPaint();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
