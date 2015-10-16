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
#include "TextCharacterSpacingControl.hxx"
#include "TextPropertyPanel.hrc"
#include <sfx2/sidebar/ResourceDefinitions.hrc>
#include <svx/dialogs.hrc>
#include <svx/dialmgr.hxx>
#include <unotools/viewoptions.hxx>
#include <editeng/kernitem.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/sidebar/Theme.hxx>
#include <vcl/settings.hxx>

namespace svx { namespace sidebar {

TextCharacterSpacingControl::TextCharacterSpacingControl (
    vcl::Window* pParent,
    svx::sidebar::TextPropertyPanel& rPanel,
    SfxBindings* pBindings)
:   PopupControl( pParent,SVX_RES(RID_POPUPPANEL_TEXTPAGE_SPACING))
,   mrTextPropertyPanel(rPanel)
,   mpBindings(pBindings)
,   maVSSpacing     (VclPtr<ValueSetWithTextControl>::Create(ValueSetWithTextControl::IMAGE_TEXT,this, SVX_RES(VS_SPACING)))
,   maLastCus       (VclPtr<FixedText>::Create(this, SVX_RES(FT_LASTCUSTOM)))
//, maBorder        (this, SVX_RES(CT_BORDER))
,   maFTSpacing     (VclPtr<FixedText>::Create(this, SVX_RES(FT_SPACING)))
,   maLBKerning     (VclPtr<ListBox>::Create(this, SVX_RES(LB_KERNING)))
,   maFTBy          (VclPtr<FixedText>::Create(this, SVX_RES(FT_BY)))
,   maEditKerning   (VclPtr<MetricField>::Create(this, SVX_RES(ED_KERNING)))

,   mpImg           (NULL)
,   mpImgSel        (NULL)
,   mpStr           (NULL)
,   mpStrTip        (NULL)

,   maImgCus        (SVX_RES(IMG_CUSTOM))
,   maImgCusGrey    (SVX_RES(IMG_CUSTOM_GRAY))
,   maStrCus        (SVX_RESSTR(STR_CUSTOM))
,   maStrCusE       (SVX_RESSTR(STR_CUSTOM_E_TIP)) //add
,   maStrCusC       (SVX_RESSTR(STR_CUSTOM_C_TIP)) //add
,   maStrCusN       (SVX_RESSTR(STR_NORMAL_TIP))   //add
,   maStrUnit       (SVX_RESSTR(STR_PT))           //add

,   mnCustomKern(0)
,   mnLastCus ( SPACING_NOCUSTOM )
,   mbCusEnable(false)
,   mbVS(true)
{
    initial();
    FreeResource();
    maLBKerning->SetSelectHdl(LINK(this, TextCharacterSpacingControl, KerningSelectHdl));
    maEditKerning->SetModifyHdl(LINK(this, TextCharacterSpacingControl, KerningModifyHdl));

}

TextCharacterSpacingControl::~TextCharacterSpacingControl()
{
    disposeOnce();
}

void TextCharacterSpacingControl::dispose()
{
    delete[] mpImg;
    delete[] mpImgSel;
    delete[] mpStr;
    delete[] mpStrTip;
    maVSSpacing.disposeAndClear();
    maLastCus.disposeAndClear();
    maFTSpacing.disposeAndClear();
    maLBKerning.disposeAndClear();
    maFTBy.disposeAndClear();
    maEditKerning.disposeAndClear();
    svx::sidebar::PopupControl::dispose();
}

void TextCharacterSpacingControl::initial()
{
    maVSSpacing->SetStyle( maVSSpacing->GetStyle()| WB_3DLOOK |  WB_NO_DIRECTSELECT  );
    {
        maVSSpacing->SetControlBackground(GetSettings().GetStyleSettings().GetHighContrastMode()?
        GetSettings().GetStyleSettings().GetMenuColor():
        sfx2::sidebar::Theme::GetColor( sfx2::sidebar::Theme::Paint_PanelBackground ));
        maVSSpacing->SetColor(GetSettings().GetStyleSettings().GetHighContrastMode()?
        GetSettings().GetStyleSettings().GetMenuColor():
        sfx2::sidebar::Theme::GetColor( sfx2::sidebar::Theme::Paint_PanelBackground ));
        maVSSpacing->SetBackground(GetSettings().GetStyleSettings().GetHighContrastMode()?
        GetSettings().GetStyleSettings().GetMenuColor():
        sfx2::sidebar::Theme::GetColor( sfx2::sidebar::Theme::Paint_PanelBackground ));
        maFTSpacing->SetBackground(GetSettings().GetStyleSettings().GetHighContrastMode()?
        GetSettings().GetStyleSettings().GetMenuColor():
        sfx2::sidebar::Theme::GetColor( sfx2::sidebar::Theme::Paint_PanelBackground ));
        maFTBy->SetBackground(GetSettings().GetStyleSettings().GetHighContrastMode()?
        GetSettings().GetStyleSettings().GetMenuColor():
        sfx2::sidebar::Theme::GetColor( sfx2::sidebar::Theme::Paint_PanelBackground ));
    }
    mpImg = new Image[5];
    mpImg[0] = Image(SVX_RES(IMG_VERY_TIGHT));
    mpImg[1] = Image(SVX_RES(IMG_TIGHT));
    mpImg[2] = Image(SVX_RES(IMG_NORMAL));
    mpImg[3] = Image(SVX_RES(IMG_LOOSE));
    mpImg[4] = Image(SVX_RES(IMG_VERY_LOOSE));

    mpImgSel = new Image[5];
    mpImgSel[0] = Image(SVX_RES(IMG_VERY_TIGHT_S));
    mpImgSel[1] = Image(SVX_RES(IMG_TIGHT_S));
    mpImgSel[2] = Image(SVX_RES(IMG_NORMAL_S));
    mpImgSel[3] = Image(SVX_RES(IMG_LOOSE_S));
    mpImgSel[4] = Image(SVX_RES(IMG_VERY_LOOSE_S));

    mpStr = new OUString[5];
    mpStr[0] = SVX_RESSTR(STR_VERY_TIGHT);
    mpStr[1] = SVX_RESSTR(STR_TIGHT);
    mpStr[2] = SVX_RESSTR(STR_NORMAL);
    mpStr[3] = SVX_RESSTR(STR_LOOSE);
    mpStr[4] = SVX_RESSTR(STR_VERY_LOOSE);


    mpStrTip = new OUString[5];
    mpStrTip[0] = SVX_RESSTR(STR_VERY_TIGHT_TIP);
    mpStrTip[1] = SVX_RESSTR(STR_TIGHT_TIP);
    mpStrTip[2] = SVX_RESSTR(STR_NORMAL_TIP);
    mpStrTip[3] = SVX_RESSTR(STR_LOOSE_TIP);
    mpStrTip[4] = SVX_RESSTR(STR_VERY_LOOSE_TIP);

    for (int i=0;i<5;i++)
        maVSSpacing->AddItem(mpImg[i], &mpImgSel[i],mpStr[i],&mpStrTip[i]);

    maVSSpacing->AddItem( maImgCus, 0, maStrCus, 0 );

    maVSSpacing->SetNoSelection();
    maVSSpacing->SetSelectHdl(LINK(this, TextCharacterSpacingControl,VSSelHdl ));
    maVSSpacing->StartSelection();
    maVSSpacing->Show();
}

void TextCharacterSpacingControl::Rearrange(bool bLBAvailable,bool bAvailable, long nKerning)
{
    mbVS = true;
    maVSSpacing->SetNoSelection();
    SvtViewOptions aWinOpt( E_WINDOW, SIDEBAR_SPACING_GLOBAL_VALUE );
    if ( aWinOpt.Exists() )
    {
        ::com::sun::star::uno::Sequence < ::com::sun::star::beans::NamedValue > aSeq = aWinOpt.GetUserData();
        ::rtl::OUString aTmp;
        if ( aSeq.getLength())
            aSeq[0].Value >>= aTmp;

        OUString aWinData( aTmp );
        mnCustomKern = aWinData.toInt32();
        mnLastCus = SPACING_CLOSE_BY_CUS_EDIT;
        mbCusEnable = true;
    }
    else
    {
        mnLastCus = SPACING_NOCUSTOM;
        mbCusEnable = false;
    }

    if( !mnLastCus )
    {
        maVSSpacing->ReplaceItemImages(6, maImgCusGrey,0);
    }
    else
    {
        //set custom tips
        maVSSpacing->ReplaceItemImages(6, maImgCus,0);
        if(mnCustomKern > 0)
        {
            OUString aStrTip( maStrCusE);   //LAST CUSTOM no tip defect //add
            aStrTip += OUString::number( (double)mnCustomKern / 10);
            aStrTip += " " + maStrUnit;      // modify
            maVSSpacing->SetItemText(6,aStrTip);
        }
        else if(mnCustomKern < 0)
        {
            OUString aStrTip(maStrCusC) ;     //LAST CUSTOM no tip defect //add
            aStrTip += OUString::number( (double)-mnCustomKern / 10);
            aStrTip += " " + maStrUnit;      // modify
            maVSSpacing->SetItemText( 6, aStrTip );
        }
        else
        {
            OUString aStrTip(maStrCusN) ;     //LAST CUSTOM no tip defect //add
            maVSSpacing->SetItemText( 6, aStrTip );
        }

    }

    if(bLBAvailable && bAvailable)
    {
        maLBKerning->Enable();
        maFTSpacing->Enable();

        SfxMapUnit eUnit = mrTextPropertyPanel.GetSpaceController().GetCoreMetric();
        MapUnit eOrgUnit = (MapUnit)eUnit;
        MapUnit ePntUnit( MAP_POINT );
        long nBig = maEditKerning->Normalize(nKerning);
        nKerning = LogicToLogic( nBig, eOrgUnit, ePntUnit );

        if ( nKerning > 0 )
        {
            maFTBy->Enable();
            maEditKerning->Enable();
            maEditKerning->SetMax( 9999 );
            maEditKerning->SetLast( 9999 );
            maEditKerning->SetValue( nKerning );
            maLBKerning->SelectEntryPos( SIDEBAR_SPACE_EXPAND );
            if(nKerning == 30)
            {
                maVSSpacing->SelectItem(4);
            }
            else if(nKerning == 60)
            {
                maVSSpacing->SelectItem(5);
            }
            else
            {
                maVSSpacing->SetNoSelection();
                maVSSpacing->SelectItem(0);
                mbVS = false;
            }
        }
        else if ( nKerning < 0 )
        {
            maFTBy->Enable();
            maEditKerning->Enable();
            maEditKerning->SetValue( -nKerning );
            maLBKerning->SelectEntryPos( SIDEBAR_SPACE_CONDENSED );
            long nMax = mrTextPropertyPanel.GetSelFontSize()/6;
            maEditKerning->SetMax( maEditKerning->Normalize( nMax ), FUNIT_POINT );
            maEditKerning->SetLast( maEditKerning->GetMax( maEditKerning->GetUnit() ) );
            if( nKerning == -30 )
            {
                maVSSpacing->SelectItem(1);
            }
            else if( nKerning == -15 )
            {
                maVSSpacing->SelectItem(2);
            }
            else
            {
                maVSSpacing->SetNoSelection();
                maVSSpacing->SelectItem(0);
                mbVS = false;
            }
        }
        else
        {
            maVSSpacing->SelectItem(3);
            maLBKerning->SelectEntryPos( SIDEBAR_SPACE_NORMAL );
            maFTBy->Disable();
            maEditKerning->Disable();
            maEditKerning->SetValue( 0 );
            maEditKerning->SetMax( 9999 );
            maEditKerning->SetLast( 9999 );
        }
    }
    else if(bLBAvailable && !bAvailable)
    {
        //modified
        maVSSpacing->SetNoSelection();
        maVSSpacing->SelectItem(0);
        mbVS = false;
        maLBKerning->Enable();
        maFTSpacing->Enable();
        maLBKerning->SetNoSelection();
        maEditKerning->SetText(OUString());
        maEditKerning->Disable();
        maFTBy->Disable();
    }
    else
    {
        maVSSpacing->SetNoSelection();
        maVSSpacing->SelectItem(0);
        mbVS = false;
        maEditKerning->SetText(OUString());
        maLBKerning->SetNoSelection();
        maLBKerning->Disable();
        maFTSpacing->Disable();
        maEditKerning->Disable();
        maFTBy->Disable();
    }
    GetFocus();
    maVSSpacing->SetFormat();
    maVSSpacing->Invalidate();
    maVSSpacing->StartSelection();
}

IMPL_LINK_TYPED(TextCharacterSpacingControl, VSSelHdl, ValueSet*, pControl, void)
{
    mnLastCus = SPACING_CLOSE_BY_CLICK_ICON;

    if(pControl == maVSSpacing.get())
    {
        sal_uInt16 iPos = maVSSpacing->GetSelectItemId();
        short nKern = 0;
        SfxMapUnit eUnit = mrTextPropertyPanel.GetSpaceController().GetCoreMetric();
        long nVal = 0;
        if(iPos == 1)
        {
            nVal = LogicToLogic(30, MAP_POINT, (MapUnit)eUnit);
            nKern = (short)maEditKerning->Denormalize(nVal);
            SvxKerningItem aKernItem(-nKern, SID_ATTR_CHAR_KERNING);
            mpBindings->GetDispatcher()->Execute(SID_ATTR_CHAR_KERNING, SfxCallMode::RECORD, &aKernItem, 0L);
            mnLastCus = SPACING_CLOSE_BY_CLICK_ICON;
        }
        else if(iPos == 2)
        {
            nVal = LogicToLogic(15, MAP_POINT, (MapUnit)eUnit);
            nKern = (short)maEditKerning->Denormalize(nVal);
            SvxKerningItem aKernItem(-nKern, SID_ATTR_CHAR_KERNING);
            mpBindings->GetDispatcher()->Execute(SID_ATTR_CHAR_KERNING, SfxCallMode::RECORD, &aKernItem, 0L);
            mnLastCus = SPACING_CLOSE_BY_CLICK_ICON;
        }
        else if(iPos == 3)
        {
            SvxKerningItem aKernItem(0, SID_ATTR_CHAR_KERNING);
            mpBindings->GetDispatcher()->Execute(SID_ATTR_CHAR_KERNING, SfxCallMode::RECORD, &aKernItem, 0L);
            mnLastCus = SPACING_CLOSE_BY_CLICK_ICON;
        }
        else if(iPos == 4)
        {
            nVal = LogicToLogic(30, MAP_POINT, (MapUnit)eUnit);
            nKern = (short)maEditKerning->Denormalize(nVal);
            SvxKerningItem aKernItem(nKern, SID_ATTR_CHAR_KERNING);
            mpBindings->GetDispatcher()->Execute(SID_ATTR_CHAR_KERNING, SfxCallMode::RECORD, &aKernItem, 0L);
            mnLastCus = SPACING_CLOSE_BY_CLICK_ICON;
        }
        else if(iPos == 5)
        {
            nVal = LogicToLogic(60, MAP_POINT, (MapUnit)eUnit);
            nKern = (short)maEditKerning->Denormalize(nVal);
            SvxKerningItem aKernItem(nKern, SID_ATTR_CHAR_KERNING);
            mpBindings->GetDispatcher()->Execute(SID_ATTR_CHAR_KERNING, SfxCallMode::RECORD, &aKernItem, 0L);
            mnLastCus = SPACING_CLOSE_BY_CLICK_ICON;
        }
        else if(iPos == 6)
        {
            //modified
            if(mbCusEnable)
            {
                nVal = LogicToLogic(mnCustomKern, MAP_POINT, (MapUnit)eUnit);
                nKern = (short)maEditKerning->Denormalize(nVal);
                SvxKerningItem aKernItem(nKern , SID_ATTR_CHAR_KERNING);
                mpBindings->GetDispatcher()->Execute(SID_ATTR_CHAR_KERNING, SfxCallMode::RECORD, &aKernItem, 0L);
                mnLastCus = SPACING_CLOSE_BY_CLICK_ICON;
            }
            else
            {
                maVSSpacing->SetNoSelection();       //add , set no selection and keep the last select item
                maVSSpacing->SetFormat();
                maVSSpacing->Invalidate();
                Invalidate();
                maVSSpacing->StartSelection();
            }
            //modify end
        }

        if(iPos < 6 || (iPos == 6 && mbCusEnable)) //add
            mrTextPropertyPanel.EndSpacingPopupMode();
    }
}

IMPL_LINK_NOARG_TYPED(TextCharacterSpacingControl, KerningSelectHdl, ListBox&, void)
{
    if ( maLBKerning->GetSelectEntryPos() > 0 )
    {
        maFTBy->Enable();
        maEditKerning->Enable();
    }
    else
    {
        maEditKerning->SetValue( 0 );
        maFTBy->Disable();
        maEditKerning->Disable();
    }

    if ( maVSSpacing->GetSelectItemId() > 0 )
    {
        maVSSpacing->SetNoSelection();
        maVSSpacing->SelectItem(0);
        maVSSpacing->SetFormat();
        maVSSpacing->Invalidate();
        Invalidate();
        maVSSpacing->StartSelection();
    }
    KerningModifyHdl( *maEditKerning );
}

IMPL_LINK_NOARG_TYPED(TextCharacterSpacingControl, KerningModifyHdl, Edit&, void)
{
    if ( maVSSpacing->GetSelectItemId() > 0 )
    {
        maVSSpacing->SetNoSelection();
        maVSSpacing->SelectItem(0);
        maVSSpacing->SetFormat();
        maVSSpacing->Invalidate();
        Invalidate();
        maVSSpacing->StartSelection();
    }
    const sal_Int32 nPos = maLBKerning->GetSelectEntryPos();
    short nKern = 0;
    SfxMapUnit eUnit = mrTextPropertyPanel.GetSpaceController().GetCoreMetric();
    mnLastCus = SPACING_CLOSE_BY_CUS_EDIT;
    if ( nPos == SIDEBAR_SPACE_EXPAND || nPos == SIDEBAR_SPACE_CONDENSED )
    {
        long nTmp = static_cast<long>(maEditKerning->GetValue());
        if ( nPos == SIDEBAR_SPACE_CONDENSED )
        {
            long nMax =  mrTextPropertyPanel.GetSelFontSize()/6;
            maEditKerning->SetMax( maEditKerning->Normalize( nMax ), FUNIT_TWIP );
            maEditKerning->SetLast( maEditKerning->GetMax( maEditKerning->GetUnit() ) );
            if(nTmp > maEditKerning->GetMax())
                nTmp = maEditKerning->GetMax();
            mnCustomKern = -nTmp;
            long nVal = LogicToLogic( nTmp, MAP_POINT, (MapUnit)eUnit );
            nKern = (short)maEditKerning->Denormalize( nVal );
            nKern *= - 1;
        }
        else
        {
            maEditKerning->SetMax( 9999 );
            maEditKerning->SetLast( 9999 );
            if(nTmp > maEditKerning->GetMax(FUNIT_TWIP))
                nTmp = maEditKerning->GetMax(FUNIT_TWIP);
            mnCustomKern = nTmp;
            long nVal = LogicToLogic( nTmp, MAP_POINT, (MapUnit)eUnit );
            nKern = (short)maEditKerning->Denormalize( nVal );
        }
    }
    else
    {
        mnCustomKern = 0;
    }
    SvxKerningItem aKernItem(nKern, SID_ATTR_CHAR_KERNING);
    mpBindings->GetDispatcher()->Execute(SID_ATTR_CHAR_KERNING, SfxCallMode::RECORD, &aKernItem, 0L);
}

}} // end of namespace sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
