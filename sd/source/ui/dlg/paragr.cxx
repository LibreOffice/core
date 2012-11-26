/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

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

    DECL_LINK( ImplNewStartHdl, CheckBox* );
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
    static sal_uInt16 __FAR_DATA aRange[] =
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
        bool bNewStartChecked = STATE_CHECK == maNewStartCB.GetState();
        bool bNumberNewStartChecked = STATE_CHECK == maNewStartNumberCB.GetState();
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
        maNewStartCB.EnableTriState(false);
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
        maNewStartNumberCB.EnableTriState(false);
    }
    else
    {
        maNewStartCB.SetState(STATE_DONTKNOW);
    }
    ImplNewStartHdl(&maNewStartCB);
    maNewStartNF.SaveValue();
    maNewStartNumberCB.SaveValue();
    mbModified = false;
}

IMPL_LINK( SdParagraphNumTabPage, ImplNewStartHdl, CheckBox*, EMPTYARG )
{
    bool bEnable = maNewStartCB.IsChecked();
    maNewStartNumberCB.Enable(bEnable);
    maNewStartNF.Enable(bEnable && maNewStartNumberCB.IsChecked());
    return 0;
}

SdParagraphDlg::SdParagraphDlg( Window* pParent, const SfxItemSet* pAttr )
: SfxTabDialog( pParent, SdResId( TAB_PARAGRAPH ), pAttr )
, rOutAttrs( *pAttr )
{
    FreeResource();

    AddTabPage( RID_SVXPAGE_STD_PARAGRAPH );

    SvtCJKOptions aCJKOptions;
    if( aCJKOptions.IsAsianTypographyEnabled() )
        AddTabPage( RID_SVXPAGE_PARA_ASIAN);
    else
        RemoveTabPage( RID_SVXPAGE_PARA_ASIAN );

    AddTabPage( RID_SVXPAGE_ALIGN_PARAGRAPH );

    static const bool bShowParaNumbering = ( getenv( "SD_SHOW_NUMBERING_PAGE" ) != NULL );
    if( bShowParaNumbering )
        AddTabPage( RID_TABPAGE_PARA_NUMBERING, SdParagraphNumTabPage::Create, SdParagraphNumTabPage::GetRanges );
    else
        RemoveTabPage( RID_TABPAGE_PARA_NUMBERING );

       AddTabPage( RID_SVXPAGE_TABULATOR );
}
