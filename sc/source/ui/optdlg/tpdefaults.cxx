/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */


#undef SC_DLLIMPLEMENTATION

#include "tpdefaults.hxx"
#include "optdlg.hrc"
#include "scresid.hxx"
#include "scmod.hxx"
#include "defaultsoptions.hxx"
#include "document.hxx"


ScTpDefaultsOptions::ScTpDefaultsOptions(Window *pParent, const SfxItemSet &rCoreSet) :
    SfxTabPage(pParent, ScResId(RID_SCPAGE_DEFAULTS), rCoreSet),
    aFLInitSpreadSheet ( this, ScResId( FL_INIT_SPREADSHEET ) ),
    aFtNSheets         ( this, ScResId( FT_NSHEETS ) ),
    aEdNSheets         ( this, ScResId( ED_NSHEETS ) ),
    aFtSheetPrefix     ( this, ScResId( FT_SHEETPREFIX ) ),
    aEdSheetPrefix     ( this, ScResId( ED_SHEETPREFIX ) )
{
    FreeResource();

    // the following computation must be modified accordingly if a third line is added to this dialog
    long nTxtW1 = aFtNSheets.GetCtrlTextWidth( aFtNSheets.GetText() );
    long nCtrlW1 = aFtNSheets.GetSizePixel().Width();
    long nTxtW2 = aFtSheetPrefix.GetCtrlTextWidth(aFtSheetPrefix.GetText() );
    long nCtrlW2 = aFtSheetPrefix.GetSizePixel().Width();
    if ( nTxtW1 >= nCtrlW1 || nTxtW2 >= nCtrlW2)
    {
        long nTxtW = std::max(nTxtW1,nTxtW2);
        Size aNewSize = aFtNSheets.GetSizePixel();
        aNewSize.Width() = nTxtW;
        aFtNSheets.SetSizePixel( aNewSize );
        aFtSheetPrefix.SetSizePixel( aNewSize );
        Point aNewPoint = aEdNSheets.GetPosPixel();
        aNewPoint.X() += (nTxtW - nCtrlW1);
        aEdNSheets.SetPosPixel( aNewPoint );
        aNewPoint.Y() = aEdSheetPrefix.GetPosPixel().Y();
        aEdSheetPrefix.SetPosPixel( aNewPoint );
    }
    aEdNSheets.SetModifyHdl( LINK(this, ScTpDefaultsOptions, NumModifiedHdl) );
    aEdSheetPrefix.SetModifyHdl( LINK(this, ScTpDefaultsOptions, PrefixModifiedHdl) );
    aEdSheetPrefix.SetGetFocusHdl( LINK(this, ScTpDefaultsOptions, PrefixEditOnFocusHdl) );
}

ScTpDefaultsOptions::~ScTpDefaultsOptions()
{
}

SfxTabPage* ScTpDefaultsOptions::Create(Window *pParent, const SfxItemSet &rCoreAttrs)
{
    return new ScTpDefaultsOptions(pParent, rCoreAttrs);
}

sal_Bool ScTpDefaultsOptions::FillItemSet(SfxItemSet &rCoreSet)
{
    sal_Bool bRet = false;
    ScDefaultsOptions aOpt;

    SCTAB nTabCount = static_cast<SCTAB>(aEdNSheets.GetValue());
    OUString aSheetPrefix = aEdSheetPrefix.GetText();


    if ( aEdNSheets.GetSavedValue() != aEdNSheets.GetText()
         || aEdSheetPrefix.GetSavedValue() != aSheetPrefix )
    {
        aOpt.SetInitTabCount( nTabCount );
        aOpt.SetInitTabPrefix( aSheetPrefix );

        rCoreSet.Put( ScTpDefaultsItem( SID_SCDEFAULTSOPTIONS, aOpt ) );
        bRet = true;
    }
    return bRet;
}

void ScTpDefaultsOptions::Reset(const SfxItemSet& rCoreSet)
{
    ScDefaultsOptions aOpt;
    const SfxPoolItem* pItem = NULL;

    if(SFX_ITEM_SET == rCoreSet.GetItemState(SID_SCDEFAULTSOPTIONS, false , &pItem))
        aOpt = ((const ScTpDefaultsItem*)pItem)->GetDefaultsOptions();

    aEdNSheets.SetValue( static_cast<sal_uInt16>( aOpt.GetInitTabCount()) );
    aEdSheetPrefix.SetText( aOpt.GetInitTabPrefix() );
    aEdNSheets.SaveValue();
    aEdSheetPrefix.SaveValue();
}

int ScTpDefaultsOptions::DeactivatePage(SfxItemSet* /*pSet*/)
{
    return KEEP_PAGE;
}

void ScTpDefaultsOptions::CheckNumSheets()
{
    sal_Int64 nVal = aEdNSheets.GetValue();
    if (nVal > MAXINITTAB)
        aEdNSheets.SetValue(MAXINITTAB);
    if (nVal < MININITTAB)
        aEdNSheets.SetValue(MININITTAB);
}

void ScTpDefaultsOptions::CheckPrefix(Edit* pEdit)
{
    if (!pEdit)
        return;

    OUString aSheetPrefix = pEdit->GetText();

    if ( !aSheetPrefix.isEmpty() && !ScDocument::ValidTabName( aSheetPrefix ) )
    {
        // Revert to last good Prefix and also select it to
        // indicate something illegal was typed
        Selection aSel( 0,  maOldPrefixValue.getLength() );
        pEdit->SetText( maOldPrefixValue, aSel );
    }
    else
    {
        OnFocusPrefixInput(pEdit);
    }
}

void ScTpDefaultsOptions::OnFocusPrefixInput(Edit* pEdit)
{
    if (!pEdit)
        return;

    // Store Prefix in case we need to revert
    maOldPrefixValue = pEdit->GetText();
}


IMPL_LINK_NOARG(ScTpDefaultsOptions, NumModifiedHdl)
{
    CheckNumSheets();
    return 0;
}

IMPL_LINK( ScTpDefaultsOptions, PrefixModifiedHdl, Edit*, pEdit )
{
    CheckPrefix(pEdit);
    return 0;
}

IMPL_LINK( ScTpDefaultsOptions, PrefixEditOnFocusHdl, Edit*, pEdit )
{
    OnFocusPrefixInput(pEdit);
    return 0;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
