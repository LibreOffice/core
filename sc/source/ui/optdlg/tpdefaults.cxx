/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Initial Developer of the Original Code is
 *       Albert Thuswaldner <albert.thuswaldner@gmail.com>
 * Portions created by the Initial Developer are Copyright (C) 2011 the
 * Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */


#undef SC_DLLIMPLEMENTATION

#include "tpdefaults.hxx"
#include "optdlg.hrc"
#include "scresid.hxx"
#include "scmod.hxx"
#include "docoptio.hxx"
#include "document.hxx"
#include "global.hxx"
#include "globstr.hrc"

#define INIT_SHEETS_MIN 1
#define INIT_SHEETS_MAX 1024

using ::rtl::OUString;

ScTpDefaultsOptions::ScTpDefaultsOptions(Window *pParent, const SfxItemSet &rCoreAttrs) :
    SfxTabPage(pParent, ScResId(RID_SCPAGE_DEFAULTS), rCoreAttrs),
    aFLInitSpreadSheet ( this, ScResId( FL_INIT_SPREADSHEET ) ),
    aFtNSheets         ( this, ScResId( FT_NSHEETS ) ),
    aEdNSheets         ( this, ScResId( ED_NSHEETS ) ),
    aFtSheetPrefix     ( this, ScResId( FT_SHEETPREFIX ) ),
    aEdSheetPrefix     ( this, ScResId( ED_SHEETPREFIX ) )
{
    FreeResource();

    const ScTpCalcItem& rItem = static_cast<const ScTpCalcItem&>(
        rCoreAttrs.Get(GetWhich(SID_SCDOCOPTIONS)));
    mpOldOptions.reset(new ScDocOptions(rItem.GetDocOptions()));
    mpNewOptions.reset(new ScDocOptions(rItem.GetDocOptions()));

    long nTxtW = aFtNSheets.GetCtrlTextWidth( aFtNSheets.GetText() );
    long nCtrlW = aFtNSheets.GetSizePixel().Width();
    if ( nTxtW >= nCtrlW )
    {
        Size aNewSize = aFtNSheets.GetSizePixel();
        aNewSize.Width() += ( nTxtW - nCtrlW );
        aFtNSheets.SetSizePixel( aNewSize );
        Point aNewPoint = aEdNSheets.GetPosPixel();
        aNewPoint.X() += ( nTxtW - nCtrlW );
        aEdNSheets.SetPosPixel( aNewPoint );
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

sal_Bool ScTpDefaultsOptions::FillItemSet(SfxItemSet &rCoreAttrs)
{
    SCTAB nTabCount = static_cast<SCTAB>(aEdNSheets.GetValue());
    OUString aSheetPrefix = aEdSheetPrefix.GetText();

    mpNewOptions->SetInitTabCount( nTabCount );
    mpNewOptions->SetInitTabPrefix( aSheetPrefix );

    if (*mpNewOptions != *mpOldOptions)
    {
        rCoreAttrs.Put(ScTpCalcItem(GetWhich(SID_SCDOCOPTIONS), *mpNewOptions));
        return sal_True;
    }
    else
        return sal_False;
}

void ScTpDefaultsOptions::Reset(const SfxItemSet& /*rCoreAttrs*/)
{
    aEdNSheets.SetValue( static_cast<sal_uInt16>(mpOldOptions->GetInitTabCount()) );
    aEdSheetPrefix.SetText( mpOldOptions->GetInitTabPrefix() );
}

int ScTpDefaultsOptions::DeactivatePage(SfxItemSet* /*pSet*/)
{
    return KEEP_PAGE;
}

void ScTpDefaultsOptions::CheckNumSheets()
{
    sal_Int64 nVal = aEdNSheets.GetValue();
    if (nVal > INIT_SHEETS_MAX)
        aEdNSheets.SetValue(INIT_SHEETS_MAX);
    if (nVal < INIT_SHEETS_MIN)
        aEdNSheets.SetValue(INIT_SHEETS_MIN);
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


IMPL_LINK( ScTpDefaultsOptions, NumModifiedHdl, NumericField*, EMPTYARG )
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
