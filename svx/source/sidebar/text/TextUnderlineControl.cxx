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
#include "TextUnderlineControl.hxx"
#include "TextPropertyPanel.hrc"
#include <sfx2/sidebar/ResourceDefinitions.hrc>
#include <svx/dialogs.hrc>
#include <svx/dialmgr.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/sidebar/Theme.hxx>
#include <editeng/udlnitem.hxx>
#include <vcl/settings.hxx>

namespace svx { namespace sidebar {

TextUnderlineControl::TextUnderlineControl (
    vcl::Window* pParent,
    svx::sidebar::TextPropertyPanel& rPanel,
    SfxBindings* pBindings)
:   svx::sidebar::PopupControl( pParent,SVX_RES(RID_POPUPPANEL_TEXTPAGE_UNDERLINE))
,   mrTextPropertyPanel(rPanel)
,   mpBindings(pBindings)
,   maVSUnderline(VclPtr<ValueSet>::Create(this, SVX_RES(VS_UNDERLINE)))
,   maPBOptions (VclPtr<PushButton>::Create(this, SVX_RES(PB_OPTIONS)))

,   maIMGSingle     (SVX_RES(IMG_SINGLE))
,   maIMGDouble     (SVX_RES(IMG_DOUBLE))
,   maIMGBold       (SVX_RES(IMG_BOLD2))
,   maIMGDot        (SVX_RES(IMG_DOT))
,   maIMGDotBold    (SVX_RES(IMG_DOT_BOLD))
,   maIMGDash       (SVX_RES(IMG_DASH))
,   maIMGDashLong   (SVX_RES(IMG_DASH_LONG))
,   maIMGDashDot    (SVX_RES(IMG_DASH_DOT))
,   maIMGDashDotDot (SVX_RES(IMG_DASH_DOT_DOT))
,   maIMGWave       (SVX_RES(IMG_WAVE))

,   maIMGSingleSel      (SVX_RES(IMG_SINGLE_SEL))
,   maIMGDoubleSel      (SVX_RES(IMG_DOUBLE_SEL))
,   maIMGBoldSel        (SVX_RES(IMG_BOLD2_SEL))
,   maIMGDotSel         (SVX_RES(IMG_DOT_SEL))
,   maIMGDotBoldSel     (SVX_RES(IMG_DOT_BOLD_SEL))
,   maIMGDashSel        (SVX_RES(IMG_DASH_SEL))
,   maIMGDashLongSel    (SVX_RES(IMG_DASH_LONG_SEL))
,   maIMGDashDotSel     (SVX_RES(IMG_DASH_DOT_SEL))
,   maIMGDashDotDotSel  (SVX_RES(IMG_DASH_DOT_DOT_SEL))
,   maIMGWaveSel        (SVX_RES(IMG_WAVE_SEL))

{
    initial();
    FreeResource();
}

TextUnderlineControl::~TextUnderlineControl()
{
    disposeOnce();
}

void TextUnderlineControl::dispose()
{
    maVSUnderline.disposeAndClear();
    maPBOptions.disposeAndClear();
    svx::sidebar::PopupControl::dispose();
}

void TextUnderlineControl::initial()
{
    maVSUnderline->SetColor(GetSettings().GetStyleSettings().GetHighContrastMode() ?
        GetSettings().GetStyleSettings().GetMenuColor() :
        sfx2::sidebar::Theme::GetColor( sfx2::sidebar::Theme::Paint_PanelBackground ));
    maVSUnderline->SetBackground(GetSettings().GetStyleSettings().GetHighContrastMode() ?
        GetSettings().GetStyleSettings().GetMenuColor() :
        sfx2::sidebar::Theme::GetColor( sfx2::sidebar::Theme::Paint_PanelBackground ));

    Link<Button*,void> aLink = LINK( this, TextUnderlineControl, PBClickHdl ) ;
    maPBOptions->SetClickHdl(aLink);

    maVSUnderline->SetStyle( maVSUnderline->GetStyle()| WB_3DLOOK | WB_NO_DIRECTSELECT );

    // 'none' item
    maVSUnderline->SetStyle( maVSUnderline->GetStyle()| WB_NONEFIELD | WB_MENUSTYLEVALUESET );
    maVSUnderline->SetText( SVX_RESSTR(STR_WITHOUT) );

    maVSUnderline->InsertItem(1, maIMGSingle, SVX_RESSTR(STR_SINGLE));
    maVSUnderline->SetItemData(1, reinterpret_cast<void*>(LINESTYLE_SINGLE));

    maVSUnderline->InsertItem(2, maIMGDouble, SVX_RESSTR(STR_DOUBLE));
    maVSUnderline->SetItemData(2, reinterpret_cast<void*>(LINESTYLE_DOUBLE));

    maVSUnderline->InsertItem(3, maIMGBold, SVX_RESSTR(STR_BOLD));
    maVSUnderline->SetItemData(3, reinterpret_cast<void*>(LINESTYLE_BOLD));

    maVSUnderline->InsertItem(4, maIMGDot, SVX_RESSTR(STR_DOT));
    maVSUnderline->SetItemData(4, reinterpret_cast<void*>(LINESTYLE_DOTTED));

    maVSUnderline->InsertItem(5, maIMGDotBold, SVX_RESSTR(STR_DOT_BOLD));
    maVSUnderline->SetItemData(5, reinterpret_cast<void*>(LINESTYLE_BOLDDOTTED));

    maVSUnderline->InsertItem(6, maIMGDash, SVX_RESSTR(STR_DASH));
    maVSUnderline->SetItemData(6, reinterpret_cast<void*>(LINESTYLE_DASH));

    maVSUnderline->InsertItem(7, maIMGDashLong, SVX_RESSTR(STR_DASH_LONG));
    maVSUnderline->SetItemData(7, reinterpret_cast<void*>(LINESTYLE_LONGDASH));

    maVSUnderline->InsertItem(8, maIMGDashDot, SVX_RESSTR(STR_DASH_DOT));
    maVSUnderline->SetItemData(8, reinterpret_cast<void*>(LINESTYLE_DASHDOT));

    maVSUnderline->InsertItem(9, maIMGDashDotDot, SVX_RESSTR(STR_DASH_DOT_DOT));
    maVSUnderline->SetItemData(9, reinterpret_cast<void*>(LINESTYLE_DASHDOTDOT));

    maVSUnderline->InsertItem(10, maIMGWave, SVX_RESSTR(STR_WAVE));
    maVSUnderline->SetItemData(10, reinterpret_cast<void*>(LINESTYLE_WAVE));

    maVSUnderline->SetColCount();
    maVSUnderline->SetSelectHdl(LINK( this, TextUnderlineControl, VSSelectHdl ));

    maVSUnderline->StartSelection();
    maVSUnderline->Show();
}

void TextUnderlineControl::Rearrange(FontLineStyle eLine)
{
    maVSUnderline->SetItemImage(1, maIMGSingle);
    maVSUnderline->SetItemImage(2, maIMGDouble );
    maVSUnderline->SetItemImage(3, maIMGBold);
    maVSUnderline->SetItemImage(4, maIMGDot);
    maVSUnderline->SetItemImage(5, maIMGDotBold);
    maVSUnderline->SetItemImage(6, maIMGDash);
    maVSUnderline->SetItemImage(7, maIMGDashLong);
    maVSUnderline->SetItemImage(8, maIMGDashDot);
    maVSUnderline->SetItemImage(9, maIMGDashDotDot);
    maVSUnderline->SetItemImage(10, maIMGWave);

    switch(eLine)
    {
    case LINESTYLE_SINGLE:
        maVSUnderline->SetItemImage(1, maIMGSingleSel);
        maVSUnderline->SelectItem(1);
        maVSUnderline->GrabFocus();
        break;
    case LINESTYLE_DOUBLE:
        maVSUnderline->SetItemImage(2, maIMGDoubleSel);
        maVSUnderline->SelectItem(2);
        maVSUnderline->GrabFocus();
        break;
    case LINESTYLE_BOLD:
        maVSUnderline->SetItemImage(3, maIMGBoldSel);
        maVSUnderline->SelectItem(3);
        maVSUnderline->GrabFocus();
        break;
    case LINESTYLE_DOTTED:
        maVSUnderline->SetItemImage(4, maIMGDotSel);
        maVSUnderline->SelectItem(4);
        maVSUnderline->GrabFocus();
        break;
    case LINESTYLE_BOLDDOTTED:
        maVSUnderline->SetItemImage(5, maIMGDotBoldSel);
        maVSUnderline->SelectItem(5);
        maVSUnderline->GrabFocus();
        break;
    case LINESTYLE_DASH:
        maVSUnderline->SetItemImage(6, maIMGDashSel);
        maVSUnderline->SelectItem(6);
        maVSUnderline->GrabFocus();
        break;
    case LINESTYLE_LONGDASH:
        maVSUnderline->SetItemImage(7, maIMGDashLongSel);
        maVSUnderline->SelectItem(7);
        maVSUnderline->GrabFocus();
        break;
    case LINESTYLE_DASHDOT:
        maVSUnderline->SetItemImage(8, maIMGDashDotSel);
        maVSUnderline->SelectItem(8);
        maVSUnderline->GrabFocus();
        break;
    case LINESTYLE_DASHDOTDOT:
        maVSUnderline->SetItemImage(9, maIMGDashDotDotSel);
        maVSUnderline->SelectItem(9);
        maVSUnderline->GrabFocus();
        break;
    case LINESTYLE_WAVE:
        maVSUnderline->SetItemImage(10, maIMGWaveSel);
        maVSUnderline->SelectItem(10);
        maVSUnderline->GrabFocus();
        break;
    case LINESTYLE_NONE:
        maVSUnderline->SelectItem(0);
        maVSUnderline->GrabFocus();
        break;
    default:
        maVSUnderline->SelectItem(1);
        maVSUnderline->SetNoSelection();
        maPBOptions->GrabFocus();
    }
    maVSUnderline->StartSelection();
}

IMPL_LINK_TYPED(TextUnderlineControl, VSSelectHdl, ValueSet*, pControl, void)
{
    if ( pControl == maVSUnderline.get() )
    {
        const sal_uInt16 iPos = maVSUnderline->GetSelectItemId();
        const FontLineStyle eUnderline = ( iPos == 0 )
                                         ? LINESTYLE_NONE
                                         : static_cast<FontLineStyle>(reinterpret_cast<sal_uInt64>(maVSUnderline->GetItemData( iPos )));

        SvxUnderlineItem aLineItem(eUnderline, SID_ATTR_CHAR_UNDERLINE);

        aLineItem.SetColor(mrTextPropertyPanel.GetUnderlineColor());
        mpBindings->GetDispatcher()->ExecuteList(SID_ATTR_CHAR_UNDERLINE,
                SfxCallMode::RECORD, { &aLineItem });
        mrTextPropertyPanel.EndUnderlinePopupMode();
    }
}

IMPL_LINK_TYPED(TextUnderlineControl, PBClickHdl, Button *, pPBtn, void)
{
    if(pPBtn == maPBOptions.get())
    {
        if (mpBindings)
        {
            SfxDispatcher* pDisp = mpBindings->GetDispatcher();
            pDisp->Execute( SID_CHAR_DLG_EFFECT, SfxCallMode::ASYNCHRON );
        }
        mrTextPropertyPanel.EndUnderlinePopupMode();
    }
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
