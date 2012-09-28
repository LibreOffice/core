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


#ifdef SD_DLLIMPLEMENTATION
#undef SD_DLLIMPLEMENTATION
#endif

#include <vcl/field.hxx>

#include <svl/cjkoptions.hxx>
#include <svl/eitem.hxx>
#include <svl/intitem.hxx>

#include <editeng/editdata.hxx>
#include <svx/dialogs.hrc>
#include <editeng/eeitem.hxx>
#include <svx/flagsdef.hxx>

#include "eetext.hxx"
#include "paragr.hxx"
#include "sdresid.hxx"
#include "glob.hrc"
#include "sdattr.hrc"

class SdParagraphNumTabPage : public SfxTabPage
{
public:
    SdParagraphNumTabPage(Window* pParent, const SfxItemSet& rSet );
    ~SdParagraphNumTabPage();

    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& rSet );
    static sal_uInt16*      GetRanges();

    virtual sal_Bool        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );

private:
    TriStateBox     maNewStartCB;
    TriStateBox     maNewStartNumberCB;
    NumericField    maNewStartNF;
    bool            mbModified;

    DECL_LINK( ImplNewStartHdl, void* );
};

SdParagraphNumTabPage::SdParagraphNumTabPage(Window* pParent, const SfxItemSet& rAttr )
: SfxTabPage(pParent, SdResId(RID_TABPAGE_PARA_NUMBERING), rAttr)
, maNewStartCB( this, SdResId( CB_NEW_START ) )
, maNewStartNumberCB( this, SdResId( CB_NUMBER_NEW_START ) )
, maNewStartNF( this, SdResId( NF_NEW_START ) )
, mbModified(false)
{
    FreeResource();

    maNewStartCB.SetClickHdl(LINK(this, SdParagraphNumTabPage, ImplNewStartHdl));
    maNewStartNumberCB.SetClickHdl(LINK(this, SdParagraphNumTabPage, ImplNewStartHdl));
}

SdParagraphNumTabPage::~SdParagraphNumTabPage()
{
}

SfxTabPage* SdParagraphNumTabPage::Create(Window *pParent, const SfxItemSet & rAttrSet)
{
    return new SdParagraphNumTabPage( pParent, rAttrSet );
}

sal_uInt16* SdParagraphNumTabPage::GetRanges()
{
    static sal_uInt16 aRange[] =
    {
        ATTR_PARANUMBERING_START, ATTR_PARANUMBERING_END,
        0
    };

    return aRange;
}

sal_Bool SdParagraphNumTabPage::FillItemSet( SfxItemSet& rSet )
{
    if(maNewStartCB.GetState() != maNewStartCB.GetSavedValue() ||
        maNewStartNumberCB.GetState() != maNewStartNumberCB.GetSavedValue()||
        maNewStartNF.GetText() != maNewStartNF.GetSavedValue())
    {
        mbModified = true;
        sal_Bool bNewStartChecked = STATE_CHECK == maNewStartCB.GetState();
        sal_Bool bNumberNewStartChecked = STATE_CHECK == maNewStartNumberCB.GetState();
        rSet.Put(SfxBoolItem(ATTR_NUMBER_NEWSTART, bNewStartChecked));

        const sal_Int16 nStartAt = (sal_Int16)maNewStartNF.GetValue();
        rSet.Put(SfxInt16Item(ATTR_NUMBER_NEWSTART_AT, bNumberNewStartChecked && bNewStartChecked ? nStartAt : -1));
    }

    return mbModified;
}

void SdParagraphNumTabPage::Reset( const SfxItemSet& rSet )
{
    SfxItemState eItemState = rSet.GetItemState( ATTR_NUMBER_NEWSTART );
    if(eItemState > SFX_ITEM_AVAILABLE )
    {
        const SfxBoolItem& rStart = (const SfxBoolItem&)rSet.Get(ATTR_NUMBER_NEWSTART);
        maNewStartCB.SetState( rStart.GetValue() ? STATE_CHECK : STATE_NOCHECK );
        maNewStartCB.EnableTriState(sal_False);
    }
    else
    {
        maNewStartCB.SetState(STATE_DONTKNOW);
        maNewStartCB.Disable();
    }
    maNewStartCB.SaveValue();

    eItemState = rSet.GetItemState( ATTR_NUMBER_NEWSTART_AT);
    if( eItemState > SFX_ITEM_AVAILABLE )
    {
        sal_Int16 nNewStart = ((const SfxInt16Item&)rSet.Get(ATTR_NUMBER_NEWSTART_AT)).GetValue();
        maNewStartNumberCB.Check(-1 != nNewStart);
        if(-1 == nNewStart)
            nNewStart = 1;

        maNewStartNF.SetValue(nNewStart);
        maNewStartNumberCB.EnableTriState(sal_False);
    }
    else
    {
        maNewStartCB.SetState(STATE_DONTKNOW);
    }
    ImplNewStartHdl(&maNewStartCB);
    maNewStartNF.SaveValue();
    maNewStartNumberCB.SaveValue();
    mbModified = sal_False;
}

IMPL_LINK_NOARG(SdParagraphNumTabPage, ImplNewStartHdl)
{
    sal_Bool bEnable = maNewStartCB.IsChecked();
    maNewStartNumberCB.Enable(bEnable);
    maNewStartNF.Enable(bEnable && maNewStartNumberCB.IsChecked());
    return 0;
}

SdParagraphDlg::SdParagraphDlg( Window* pParent, const SfxItemSet* pAttr )
: SfxTabDialog( pParent, SdResId( TAB_PARAGRAPH ), pAttr )
{
    FreeResource();

    AddTabPage( RID_SVXPAGE_STD_PARAGRAPH );

    SvtCJKOptions aCJKOptions;
    if( aCJKOptions.IsAsianTypographyEnabled() )
        AddTabPage( RID_SVXPAGE_PARA_ASIAN);
    else
        RemoveTabPage( RID_SVXPAGE_PARA_ASIAN );

    AddTabPage( RID_SVXPAGE_ALIGN_PARAGRAPH );

    static const sal_Bool bShowParaNumbering = ( getenv( "SD_SHOW_NUMBERING_PAGE" ) != NULL );
    if( bShowParaNumbering )
        AddTabPage( RID_TABPAGE_PARA_NUMBERING, SdParagraphNumTabPage::Create, SdParagraphNumTabPage::GetRanges );
    else
        RemoveTabPage( RID_TABPAGE_PARA_NUMBERING );

       AddTabPage( RID_SVXPAGE_TABULATOR );
}

void SdParagraphDlg::PageCreated( sal_uInt16 nId, SfxTabPage &rPage )
{
    SfxAllItemSet aSet(*(GetInputSetImpl()->GetPool()));
    switch( nId )
    {
    case RID_SVXPAGE_STD_PARAGRAPH:
        aSet.Put(SfxUInt32Item(SID_SVXSTDPARAGRAPHTABPAGE_ABSLINEDIST, MM50/2));
        rPage.PageCreated(aSet);
        break;
    default:
        break;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
