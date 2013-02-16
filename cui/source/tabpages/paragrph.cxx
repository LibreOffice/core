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

#include <svl/style.hxx>
#include <sfx2/app.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/module.hxx>
#include <vcl/mnemonic.hxx>
#include <svx/dialogs.hrc>

#define _SVX_PARAGRPH_CXX   0

#include <svl/languageoptions.hxx>
#include <svl/cjkoptions.hxx>
#include <editeng/pgrditem.hxx>
#include <cuires.hrc>
#include "paragrph.hrc"
#include "paragrph.hxx"
#include <editeng/frmdiritem.hxx>
#include <editeng/lspcitem.hxx>
#include <editeng/adjustitem.hxx>
#include <editeng/orphitem.hxx>
#include <editeng/widwitem.hxx>
#include <editeng/tstpitem.hxx>
#include <editeng/pmdlitem.hxx>
#include <editeng/spltitem.hxx>
#include <editeng/hyphenzoneitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/formatbreakitem.hxx>
#include <editeng/keepitem.hxx>
#include "svx/dlgutil.hxx"
#include <dialmgr.hxx>
#include "svx/htmlmode.hxx"
#include <editeng/paravertalignitem.hxx>
#include <svl/eitem.hxx>
#include <sfx2/request.hxx>
#include <svl/intitem.hxx>

// static ----------------------------------------------------------------

static sal_uInt16 pStdRanges[] =
{
    SID_ATTR_PARA_LINESPACE,        // 10033
    SID_ATTR_PARA_LINESPACE,
    SID_ATTR_LRSPACE,               // 10048 -
    SID_ATTR_ULSPACE,               // 10049
    SID_ATTR_PARA_REGISTER,         // 10413
    SID_ATTR_PARA_REGISTER,
    0
};

static sal_uInt16 pAlignRanges[] =
{
    SID_ATTR_PARA_ADJUST,           // 10027
    SID_ATTR_PARA_ADJUST,
    0
};

static sal_uInt16 pExtRanges[] =
{
    SID_ATTR_PARA_PAGEBREAK,        // 10037 -
    SID_ATTR_PARA_WIDOWS,           // 10041
    SID_ATTR_PARA_MODEL,            // 10065 -
    SID_ATTR_PARA_KEEP,             // 10066
    0
};

// define ----------------------------------------------------------------

#define MAX_DURCH 5670      // 10 cm makes sense as maximum interline lead
                            // according to BP
#define FIX_DIST_DEF 283    // standard fix distance 0,5 cm

// enum ------------------------------------------------------------------

enum LineSpaceList
{
    LLINESPACE_1    = 0,
    LLINESPACE_15   = 1,
    LLINESPACE_2    = 2,
    LLINESPACE_PROP = 3,
    LLINESPACE_MIN  = 4,
    LLINESPACE_DURCH= 5,
    LLINESPACE_FIX  = 6,
    LLINESPACE_END
};

// C-Function ------------------------------------------------------------

void SetLineSpace_Impl( SvxLineSpacingItem&, int, long lValue = 0 );

void SetLineSpace_Impl( SvxLineSpacingItem& rLineSpace,
                        int eSpace, long lValue )
{
    switch ( eSpace )
    {
        case LLINESPACE_1:
            rLineSpace.GetLineSpaceRule() = SVX_LINE_SPACE_AUTO;
            rLineSpace.GetInterLineSpaceRule() = SVX_INTER_LINE_SPACE_OFF;
            break;

        case LLINESPACE_15:
            rLineSpace.GetLineSpaceRule() = SVX_LINE_SPACE_AUTO;
            rLineSpace.SetPropLineSpace( 150 );
            break;

        case LLINESPACE_2:
            rLineSpace.GetLineSpaceRule() = SVX_LINE_SPACE_AUTO;
            rLineSpace.SetPropLineSpace( 200 );
            break;

        case LLINESPACE_PROP:
            rLineSpace.GetLineSpaceRule() = SVX_LINE_SPACE_AUTO;
            rLineSpace.SetPropLineSpace( (sal_uInt8)lValue );
            break;

        case LLINESPACE_MIN:
            rLineSpace.SetLineHeight( (sal_uInt16)lValue );
            rLineSpace.GetInterLineSpaceRule() = SVX_INTER_LINE_SPACE_OFF;
            break;

        case LLINESPACE_DURCH:
            rLineSpace.GetLineSpaceRule() = SVX_LINE_SPACE_AUTO;
            rLineSpace.SetInterLineSpace( (sal_uInt16)lValue );
            break;

        case LLINESPACE_FIX:
            rLineSpace.SetLineHeight((sal_uInt16)lValue);
            rLineSpace.GetLineSpaceRule() = SVX_LINE_SPACE_FIX;
            rLineSpace.GetInterLineSpaceRule() = SVX_INTER_LINE_SPACE_OFF;
        break;
    }
}


sal_uInt16 GetHtmlMode_Impl(const SfxItemSet& rSet)
{
    sal_uInt16 nHtmlMode = 0;
    const SfxPoolItem* pItem = 0;
    SfxObjectShell* pShell;
    if(SFX_ITEM_SET == rSet.GetItemState(SID_HTML_MODE, sal_False, &pItem) ||
        ( 0 != (pShell = SfxObjectShell::Current()) &&
                    0 != (pItem = pShell->GetItem(SID_HTML_MODE))))
    {
        nHtmlMode = ((SfxUInt16Item*)pItem)->GetValue();
    }
    return nHtmlMode;

}

// class SvxStdParagraphTabPage ------------------------------------------

IMPL_LINK_NOARG(SvxStdParagraphTabPage, ELRLoseFocusHdl)
{
    SfxItemPool* pPool = GetItemSet().GetPool();
    DBG_ASSERT( pPool, "Wo ist der Pool" );
    FieldUnit eUnit =
        MapToFieldUnit( pPool->GetMetric( GetWhich( SID_ATTR_LRSPACE ) ) );

    sal_Int64 nL = aLeftIndent.Denormalize( aLeftIndent.GetValue( eUnit ) );
    sal_Int64 nR = aRightIndent.Denormalize( aRightIndent.GetValue( eUnit ) );
    String aTmp = aFLineIndent.GetText();

    if( aLeftIndent.GetMin() < 0 )
        aFLineIndent.SetMin( -99999, FUNIT_MM );
    else
        aFLineIndent.SetMin( aFLineIndent.Normalize( -nL ), eUnit );

    // Check only for concrete width (Shell)
    sal_Int64 nTmp = nWidth - nL - nR - MM50;
    aFLineIndent.SetMax( aFLineIndent.Normalize( nTmp ), eUnit );

    if ( !aTmp.Len() )
        aFLineIndent.SetEmptyFieldValue();
    // maximum left right
    aTmp = aLeftIndent.GetText();
    nTmp = nWidth - nR - MM50;
    aLeftIndent.SetMax( aLeftIndent.Normalize( nTmp ), eUnit );

    if ( !aTmp.Len() )
        aLeftIndent.SetEmptyFieldValue();
    aTmp = aRightIndent.GetText();
    nTmp = nWidth - nL - MM50;
    aRightIndent.SetMax( aRightIndent.Normalize( nTmp ), eUnit );

    if ( !aTmp.Len() )
        aRightIndent.SetEmptyFieldValue();
    return 0;
}

// -----------------------------------------------------------------------

SfxTabPage* SvxStdParagraphTabPage::Create( Window* pParent,
                                            const SfxItemSet& rSet)
{
    return new SvxStdParagraphTabPage( pParent, rSet );
}

// -----------------------------------------------------------------------

sal_Bool SvxStdParagraphTabPage::FillItemSet( SfxItemSet& rOutSet )
{
    SfxItemState eState = SFX_ITEM_UNKNOWN;
    const SfxPoolItem* pOld = 0;
    SfxItemPool* pPool = rOutSet.GetPool();
    DBG_ASSERT( pPool, "Wo ist der Pool" );

    sal_Bool bModified = sal_False;
    sal_uInt16 nWhich;
    sal_uInt16 nPos = aLineDist.GetSelectEntryPos();

    if ( LISTBOX_ENTRY_NOTFOUND != nPos &&
         ( nPos != aLineDist.GetSavedValue() ||
           aLineDistAtPercentBox.IsValueModified() ||
           aLineDistAtMetricBox.IsValueModified() ) )
    {
        nWhich = GetWhich( SID_ATTR_PARA_LINESPACE );
        SfxMapUnit eUnit = pPool->GetMetric( nWhich );
        SvxLineSpacingItem aSpacing(
            (const SvxLineSpacingItem&)GetItemSet().Get( nWhich ) );

        switch ( nPos )
        {
            case LLINESPACE_1:
            case LLINESPACE_15:
            case LLINESPACE_2:
                SetLineSpace_Impl( aSpacing, nPos );
                break;

            case LLINESPACE_PROP:
                SetLineSpace_Impl( aSpacing, nPos,
                                   static_cast<long>(aLineDistAtPercentBox.Denormalize(
                                   aLineDistAtPercentBox.GetValue() )) );
                break;

            case LLINESPACE_MIN:
            case LLINESPACE_DURCH:
            case LLINESPACE_FIX:
                SetLineSpace_Impl( aSpacing, nPos,
                    GetCoreValue( aLineDistAtMetricBox, eUnit ) );
            break;

            default:
                OSL_FAIL( "unbekannter Type fuer Zeilenabstand." );
                break;
        }
        eState = GetItemSet().GetItemState( nWhich );
        pOld = GetOldItem( rOutSet, SID_ATTR_PARA_LINESPACE );

        if ( !pOld || !( *(const SvxLineSpacingItem*)pOld == aSpacing ) ||
             SFX_ITEM_DONTCARE == eState )
        {
            rOutSet.Put( aSpacing );
            bModified = sal_True;
        }
    }

    if ( aTopDist.IsValueModified() || aBottomDist.IsValueModified()
            || aContextualCB.GetSavedValue() != aContextualCB.IsChecked())
    {
        nWhich = GetWhich( SID_ATTR_ULSPACE );
        SfxMapUnit eUnit = pPool->GetMetric( nWhich );
        pOld = GetOldItem( rOutSet, SID_ATTR_ULSPACE );
        SvxULSpaceItem aMargin( nWhich );

        if ( bRelativeMode )
        {
            DBG_ASSERT( GetItemSet().GetParent(), "No ParentSet" );

            const SvxULSpaceItem& rOldItem =
                (const SvxULSpaceItem&)GetItemSet().GetParent()->Get( nWhich );

            if ( aTopDist.IsRelative() )
                aMargin.SetUpper( rOldItem.GetUpper(),
                                  (sal_uInt16)aTopDist.GetValue() );
            else
                aMargin.SetUpper( (sal_uInt16)GetCoreValue( aTopDist, eUnit ) );

            if ( aBottomDist.IsRelative() )
                aMargin.SetLower( rOldItem.GetLower(),
                                  (sal_uInt16)aBottomDist.GetValue() );
            else
                aMargin.SetLower( (sal_uInt16)GetCoreValue( aBottomDist, eUnit ) );

        }
        else
        {
            aMargin.SetUpper( (sal_uInt16)GetCoreValue( aTopDist, eUnit ) );
            aMargin.SetLower( (sal_uInt16)GetCoreValue( aBottomDist, eUnit ) );
        }
        aMargin.SetContextValue(aContextualCB.IsChecked());
        eState = GetItemSet().GetItemState( nWhich );

        if ( !pOld || !( *(const SvxULSpaceItem*)pOld == aMargin ) ||
             SFX_ITEM_DONTCARE == eState )
        {
            rOutSet.Put( aMargin );
            bModified = sal_True;
        }
    }
    bool bNullTab = false;

    if ( aLeftIndent.IsValueModified() ||
         aFLineIndent.IsValueModified() ||
         aRightIndent.IsValueModified()
         ||  aAutoCB.GetSavedValue() != aAutoCB.IsChecked() )
    {
        nWhich = GetWhich( SID_ATTR_LRSPACE );
        SfxMapUnit eUnit = pPool->GetMetric( nWhich );
        SvxLRSpaceItem aMargin( nWhich );
        pOld = GetOldItem( rOutSet, SID_ATTR_LRSPACE );

        if ( bRelativeMode )
        {
            DBG_ASSERT( GetItemSet().GetParent(), "No ParentSet" );

            const SvxLRSpaceItem& rOldItem =
                (const SvxLRSpaceItem&)GetItemSet().GetParent()->Get( nWhich );

            if ( aLeftIndent.IsRelative() )
                aMargin.SetTxtLeft( rOldItem.GetTxtLeft(),
                                    (sal_uInt16)aLeftIndent.GetValue() );
            else
                aMargin.SetTxtLeft( GetCoreValue( aLeftIndent, eUnit ) );

            if ( aRightIndent.IsRelative() )
                aMargin.SetRight( rOldItem.GetRight(),
                                  (sal_uInt16)aRightIndent.GetValue() );
            else
                aMargin.SetRight( GetCoreValue( aRightIndent, eUnit ) );

            if ( aFLineIndent.IsRelative() )
                aMargin.SetTxtFirstLineOfst( rOldItem.GetTxtFirstLineOfst(),
                                             (sal_uInt16)aFLineIndent.GetValue() );
            else
                aMargin.SetTxtFirstLineOfst(
                    (sal_uInt16)GetCoreValue( aFLineIndent, eUnit ) );
        }
        else
        {
            aMargin.SetTxtLeft( GetCoreValue( aLeftIndent, eUnit ) );
            aMargin.SetRight( GetCoreValue( aRightIndent, eUnit ) );
            aMargin.SetTxtFirstLineOfst(
                (sal_uInt16)GetCoreValue( aFLineIndent, eUnit ) );
        }
        aMargin.SetAutoFirst(aAutoCB.IsChecked());
        if ( aMargin.GetTxtFirstLineOfst() < 0 )
            bNullTab = true;
        eState = GetItemSet().GetItemState( nWhich );

        if ( !pOld || !( *(const SvxLRSpaceItem*)pOld == aMargin ) ||
             SFX_ITEM_DONTCARE == eState )
        {
            rOutSet.Put( aMargin );
            bModified = sal_True;
        }
    }

    if ( bNullTab )
    {
        MapUnit eUnit = (MapUnit)pPool->GetMetric( GetWhich( SID_ATTR_TABSTOP ) );
        if ( MAP_100TH_MM != eUnit )
        {

            // negative first line indent -> set null default tabstob if applicable
            sal_uInt16 _nWhich = GetWhich( SID_ATTR_TABSTOP );
            const SfxItemSet& rInSet = GetItemSet();

            if ( rInSet.GetItemState( _nWhich ) >= SFX_ITEM_AVAILABLE )
            {
                const SvxTabStopItem& rTabItem =
                    (const SvxTabStopItem&)rInSet.Get( _nWhich );
                SvxTabStopItem aNullTab( rTabItem );
                SvxTabStop aNull( 0, SVX_TAB_ADJUST_DEFAULT );
                aNullTab.Insert( aNull );
                rOutSet.Put( aNullTab );
            }
        }
    }
    if( aRegisterCB.IsVisible())
    {
        const SfxBoolItem* pBoolItem = (SfxBoolItem*)GetOldItem(
                            rOutSet, SID_ATTR_PARA_REGISTER);
        SfxBoolItem* pRegItem = (SfxBoolItem*)pBoolItem->Clone();
        sal_uInt16 _nWhich = GetWhich( SID_ATTR_PARA_REGISTER );
        sal_Bool bSet = pRegItem->GetValue();

        if(aRegisterCB.IsChecked() != bSet )
        {
            pRegItem->SetValue(!bSet);
            rOutSet.Put(*pRegItem);
            bModified = sal_True;
        }
        else if ( SFX_ITEM_DEFAULT == GetItemSet().GetItemState( _nWhich, sal_False ) )
            rOutSet.ClearItem(_nWhich);
        delete pRegItem;
    }

    return bModified;
}

// -----------------------------------------------------------------------

void SvxStdParagraphTabPage::Reset( const SfxItemSet& rSet )
{
    SfxItemPool* pPool = rSet.GetPool();
    DBG_ASSERT( pPool, "Wo ist der Pool?" );

    // adjust metric
    FieldUnit eFUnit = GetModuleFieldUnit( rSet );

    bool bApplyCharUnit = GetApplyCharUnit( rSet );

    SvtCJKOptions aCJKOptions;
    if(aCJKOptions.IsAsianTypographyEnabled() && bApplyCharUnit )
        eFUnit = FUNIT_CHAR;

    SetFieldUnit( aLeftIndent, eFUnit );
    SetFieldUnit( aRightIndent, eFUnit );
    SetFieldUnit( aFLineIndent, eFUnit );
    if ( eFUnit == FUNIT_CHAR )
    {
        SetFieldUnit( aTopDist, FUNIT_LINE );
        SetFieldUnit( aBottomDist, FUNIT_LINE );
        SetFieldUnit( aLineDistAtMetricBox, FUNIT_POINT );
    }
    else
    {
        SetFieldUnit( aTopDist, eFUnit );
        SetFieldUnit( aBottomDist, eFUnit );
        SetFieldUnit( aLineDistAtMetricBox, eFUnit );
    }

    sal_uInt16 _nWhich = GetWhich( SID_ATTR_LRSPACE );
    SfxItemState eItemState = rSet.GetItemState( _nWhich );

    if ( eItemState >= SFX_ITEM_AVAILABLE )
    {
        SfxMapUnit eUnit = pPool->GetMetric( _nWhich );

        if ( bRelativeMode )
        {
            const SvxLRSpaceItem& rOldItem =
                (const SvxLRSpaceItem&)rSet.Get( _nWhich );

            if ( rOldItem.GetPropLeft() != 100 )
            {
                aLeftIndent.SetRelative( sal_True );
                aLeftIndent.SetValue( rOldItem.GetPropLeft() );
            }
            else
            {
                aLeftIndent.SetRelative();
                SetFieldUnit( aLeftIndent, eFUnit );
                SetMetricValue( aLeftIndent, rOldItem.GetTxtLeft(), eUnit );
            }

            if ( rOldItem.GetPropRight() != 100 )
            {
                aRightIndent.SetRelative( sal_True );
                aRightIndent.SetValue( rOldItem.GetPropRight() );
            }
            else
            {
                aRightIndent.SetRelative();
                SetFieldUnit( aRightIndent, eFUnit );
                SetMetricValue( aRightIndent, rOldItem.GetRight(), eUnit );
            }

            if ( rOldItem.GetPropTxtFirstLineOfst() != 100 )
            {
                aFLineIndent.SetRelative( sal_True );
                aFLineIndent.SetValue( rOldItem.GetPropTxtFirstLineOfst() );
            }
            else
            {
                aFLineIndent.SetRelative();
                aFLineIndent.SetMin(-9999);
                SetFieldUnit( aFLineIndent, eFUnit );
                SetMetricValue( aFLineIndent, rOldItem.GetTxtFirstLineOfst(),
                                eUnit );
            }
            aAutoCB.Check(rOldItem.IsAutoFirst());
        }
        else
        {
            const SvxLRSpaceItem& rSpace =
                (const SvxLRSpaceItem&)rSet.Get( _nWhich );

            SetMetricValue( aLeftIndent, rSpace.GetTxtLeft(), eUnit );
            SetMetricValue( aRightIndent, rSpace.GetRight(), eUnit );
            SetMetricValue( aFLineIndent, rSpace.GetTxtFirstLineOfst(), eUnit );
            aAutoCB.Check(rSpace.IsAutoFirst());
        }
        AutoHdl_Impl(&aAutoCB);
    }
    else
    {
        aLeftIndent.SetEmptyFieldValue();
        aRightIndent.SetEmptyFieldValue();
        aFLineIndent.SetEmptyFieldValue();
    }

    _nWhich = GetWhich( SID_ATTR_ULSPACE );
    eItemState = rSet.GetItemState( _nWhich );

    if ( eItemState >= SFX_ITEM_AVAILABLE )
    {
        SfxMapUnit eUnit = pPool->GetMetric( _nWhich );

        const SvxULSpaceItem& rOldItem =
            (const SvxULSpaceItem&)rSet.Get( _nWhich );
        if ( bRelativeMode )
        {

            if ( rOldItem.GetPropUpper() != 100 )
            {
                aTopDist.SetRelative( sal_True );
                aTopDist.SetValue( rOldItem.GetPropUpper() );
            }
            else
            {
                aTopDist.SetRelative();
                if ( eFUnit == FUNIT_CHAR )
                    SetFieldUnit( aTopDist, FUNIT_LINE );
                else
                    SetFieldUnit( aTopDist, eFUnit );
                SetMetricValue( aTopDist, rOldItem.GetUpper(), eUnit );
            }

            if ( rOldItem.GetPropLower() != 100 )
            {
                aBottomDist.SetRelative( sal_True );
                aBottomDist.SetValue( rOldItem.GetPropLower() );
            }
            else
            {
                aBottomDist.SetRelative();
                if ( eFUnit == FUNIT_CHAR )
                    SetFieldUnit( aBottomDist, FUNIT_LINE );
                else
                    SetFieldUnit( aBottomDist, eFUnit );
                SetMetricValue( aBottomDist, rOldItem.GetLower(), eUnit );
            }
        }
        else
        {
            SetMetricValue( aTopDist, rOldItem.GetUpper(), eUnit );
            SetMetricValue( aBottomDist, rOldItem.GetLower(), eUnit );
        }
        aContextualCB.Check(rOldItem.GetContext());
    }
    else
    {
        aTopDist.SetEmptyFieldValue();
        aBottomDist.SetEmptyFieldValue();
    }

    _nWhich = GetWhich( SID_ATTR_PARA_LINESPACE );
    eItemState = rSet.GetItemState( _nWhich );

    if ( eItemState >= SFX_ITEM_AVAILABLE )
        SetLineSpacing_Impl( (const SvxLineSpacingItem &)rSet.Get( _nWhich ) );
    else
        aLineDist.SetNoSelection();


    _nWhich = GetWhich( SID_ATTR_PARA_REGISTER );
    eItemState = rSet.GetItemState( _nWhich );

    if ( eItemState >= SFX_ITEM_AVAILABLE )
        aRegisterCB.Check( ((const SfxBoolItem &)rSet.Get( _nWhich )).GetValue());
    aRegisterCB.SaveValue();
    sal_uInt16 nHtmlMode = GetHtmlMode_Impl(rSet);
    if(nHtmlMode & HTMLMODE_ON)
    {
        aRegisterCB.Hide();
        aRegisterFL.Hide();
        aAutoCB.Hide();
        if(!(nHtmlMode & HTMLMODE_SOME_STYLES)) // IE or SW
        {
            aRightLabel.Disable();
            aRightIndent.Disable();
            aTopDist.Disable();  //HTML3.2 and NS 3.0
            aBottomDist.Disable();
            aFLineIndent.Disable();
            aFLineLabel.Disable();
        }
    }

    ELRLoseFocusHdl( NULL );
    aAutoCB.SaveValue();
    aContextualCB.SaveValue();
    aLineDist.SaveValue();
}

// -----------------------------------------------------------------------

void SvxStdParagraphTabPage::EnableRelativeMode()
{
    DBG_ASSERT( GetItemSet().GetParent(), "RelativeMode, but no parent-set!" );

    aLeftIndent.EnableRelativeMode( 0, 999 );
    aFLineIndent.EnableRelativeMode( 0, 999 );
    aRightIndent.EnableRelativeMode( 0, 999 );
    aTopDist.EnableRelativeMode( 0, 999 );
    aBottomDist.EnableRelativeMode( 0, 999 );
    bRelativeMode = sal_True;
}

// -----------------------------------------------------------------------

int SvxStdParagraphTabPage::DeactivatePage( SfxItemSet* _pSet )
{
    ELRLoseFocusHdl( NULL );

    if ( _pSet )
        FillItemSet( *_pSet );
    return LEAVE_PAGE;
}

// -----------------------------------------------------------------------

SvxStdParagraphTabPage::SvxStdParagraphTabPage( Window* pParent,
                                                const SfxItemSet& rAttr ) :

    SfxTabPage( pParent, CUI_RES( RID_SVXPAGE_STD_PARAGRAPH ), rAttr ),

    aIndentFrm              ( this, CUI_RES( FL_INDENT ) ),
    aLeftLabel              ( this, CUI_RES( FT_LEFTINDENT ) ),
    aLeftIndent             ( this, CUI_RES( ED_LEFTINDENT ) ),
    aRightLabel             ( this, CUI_RES( FT_RIGHTINDENT ) ),
    aRightIndent            ( this, CUI_RES( ED_RIGHTINDENT ) ),

    aFLineLabel             ( this, CUI_RES( FT_FLINEINDENT ) ),
    aFLineIndent            ( this, CUI_RES( ED_FLINEINDENT ) ),
    aAutoCB                 ( this, CUI_RES( CB_AUTO ) ),
    aDistFrm                ( this, CUI_RES( FL_DIST ) ),
    aTopLabel               ( this, CUI_RES( FT_TOPDIST ) ),
    aTopDist                ( this, CUI_RES( ED_TOPDIST ) ),
    aBottomLabel            ( this, CUI_RES( FT_BOTTOMDIST ) ),
    aBottomDist             ( this, CUI_RES( ED_BOTTOMDIST ) ),
    aContextualCB           ( this, CUI_RES( CB_CONTEXTUALSPACING ) ),

    aLineDistFrm            ( this, CUI_RES( FL_LINEDIST ) ),
    aLineDist               ( this, CUI_RES( LB_LINEDIST ) ),
    aLineDistAtLabel        ( this, CUI_RES( FT_LINEDIST ) ),
    aLineDistAtPercentBox   ( this, CUI_RES( ED_LINEDISTPERCENT ) ),
    aLineDistAtMetricBox    ( this, CUI_RES( ED_LINEDISTMETRIC ) ),
    sAbsDist                ( CUI_RES(ST_LINEDIST_ABS) ),
    aExampleWin             ( this, CUI_RES( WN_EXAMPLE ) ),
    aRegisterFL             ( this, CUI_RES( FL_REGISTER ) ),
    aRegisterCB             ( this, CUI_RES( CB_REGISTER ) ),
    pActLineDistFld ( &aLineDistAtPercentBox ),
    nAbst           ( MAX_DURCH ),
    nWidth          ( 11905 /*567 * 50*/ ),
    nMinFixDist(0L),

    bRelativeMode   ( sal_False ),
    bNegativeIndents(sal_False)

{
    // this page needs ExchangeSupport
    SetExchangeSupport();

    aLineDistAtMetricBox.Hide();
    FreeResource();
    Init_Impl();
    aFLineIndent.SetMin(-9999);    // is set to 0 on default

    aExampleWin.SetAccessibleName(String(CUI_RES(STR_EXAMPLE)));

}


// -----------------------------------------------------------------------

void SvxStdParagraphTabPage::EnableNegativeMode()
{
    aLeftIndent.SetMin(-9999);
    aRightIndent.SetMin(-9999);
    aRightIndent.EnableNegativeMode();
    aLeftIndent.EnableNegativeMode();
    bNegativeIndents = sal_True;
}

// -----------------------------------------------------------------------

sal_uInt16* SvxStdParagraphTabPage::GetRanges()
{
    return pStdRanges;
}

// -----------------------------------------------------------------------

void SvxStdParagraphTabPage::SetLineSpacing_Impl
(
    const SvxLineSpacingItem &rAttr
)
{
    SfxMapUnit eUnit = GetItemSet().GetPool()->GetMetric( rAttr.Which() );

    switch( rAttr.GetLineSpaceRule() )
    {
        case SVX_LINE_SPACE_AUTO:
        {
            SvxInterLineSpace eInter = rAttr.GetInterLineSpaceRule();

            switch( eInter )
            {
                // Default single line spacing
                case SVX_INTER_LINE_SPACE_OFF:
                    aLineDist.SelectEntryPos( LLINESPACE_1 );
                    break;

                // Default single line spacing
                case SVX_INTER_LINE_SPACE_PROP:
                    if ( 100 == rAttr.GetPropLineSpace() )
                    {
                        aLineDist.SelectEntryPos( LLINESPACE_1 );
                        break;
                    }
                    // 1.5 line spacing
                    if ( 150 == rAttr.GetPropLineSpace() )
                    {
                        aLineDist.SelectEntryPos( LLINESPACE_15 );
                        break;
                    }
                    // double line spacing
                    if ( 200 == rAttr.GetPropLineSpace() )
                    {
                        aLineDist.SelectEntryPos( LLINESPACE_2 );
                        break;
                    }
                    // the set per cent value
                    aLineDistAtPercentBox.
                        SetValue( aLineDistAtPercentBox.Normalize(
                                        rAttr.GetPropLineSpace() ) );
                    aLineDist.SelectEntryPos( LLINESPACE_PROP );
                    break;

                case SVX_INTER_LINE_SPACE_FIX:
                    SetMetricValue( aLineDistAtMetricBox,
                                    rAttr.GetInterLineSpace(), eUnit );
                    aLineDist.SelectEntryPos( LLINESPACE_DURCH );
                    break;
                default: ;//prevent warning
            }
        }
        break;
        case SVX_LINE_SPACE_FIX:
            SetMetricValue(aLineDistAtMetricBox, rAttr.GetLineHeight(), eUnit);
            aLineDist.SelectEntryPos( LLINESPACE_FIX );
        break;

        case SVX_LINE_SPACE_MIN:
            SetMetricValue(aLineDistAtMetricBox, rAttr.GetLineHeight(), eUnit);
            aLineDist.SelectEntryPos( LLINESPACE_MIN );
            break;
        default: ;//prevent warning
    }
    LineDistHdl_Impl( &aLineDist );
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxStdParagraphTabPage, LineDistHdl_Impl, ListBox *, pBox )
{
    switch( pBox->GetSelectEntryPos() )
    {
        case LLINESPACE_1:
        case LLINESPACE_15:
        case LLINESPACE_2:
            aLineDistAtLabel.Enable(sal_False);
            pActLineDistFld->Enable(sal_False);
            pActLineDistFld->SetText( String() );
            break;

        case LLINESPACE_DURCH:
            // setting a sensible default?
            // limit MS min(10, aPageSize)
            aLineDistAtPercentBox.Hide();
            pActLineDistFld = &aLineDistAtMetricBox;
            aLineDistAtMetricBox.SetMin(0);


            if ( aLineDistAtMetricBox.GetText().isEmpty() )
                aLineDistAtMetricBox.SetValue(
                    aLineDistAtMetricBox.Normalize( 1 ) );
            aLineDistAtPercentBox.Hide();
            pActLineDistFld->Show();
            pActLineDistFld->Enable();
            aLineDistAtLabel.Enable();
            break;

        case LLINESPACE_MIN:
            aLineDistAtPercentBox.Hide();
            pActLineDistFld = &aLineDistAtMetricBox;
            aLineDistAtMetricBox.SetMin(0);

            if ( aLineDistAtMetricBox.GetText().isEmpty() )
                aLineDistAtMetricBox.SetValue(
                    aLineDistAtMetricBox.Normalize( 10 ), FUNIT_TWIP );
            aLineDistAtPercentBox.Hide();
            pActLineDistFld->Show();
            pActLineDistFld->Enable();
            aLineDistAtLabel.Enable();
            break;

        case LLINESPACE_PROP:
            aLineDistAtMetricBox.Hide();
            pActLineDistFld = &aLineDistAtPercentBox;

            if ( aLineDistAtPercentBox.GetText().isEmpty() )
                aLineDistAtPercentBox.SetValue(
                    aLineDistAtPercentBox.Normalize( 100 ), FUNIT_TWIP );
            aLineDistAtMetricBox.Hide();
            pActLineDistFld->Show();
            pActLineDistFld->Enable();
            aLineDistAtLabel.Enable();
            break;
        case LLINESPACE_FIX:
        {
            aLineDistAtPercentBox.Hide();
            pActLineDistFld = &aLineDistAtMetricBox;
            sal_Int64 nTemp = aLineDistAtMetricBox.GetValue();
            aLineDistAtMetricBox.SetMin(aLineDistAtMetricBox.Normalize(nMinFixDist), FUNIT_TWIP);

            // if the value has been changed at SetMin,
            // it is time for the default
            if ( aLineDistAtMetricBox.GetValue() != nTemp )
                SetMetricValue( aLineDistAtMetricBox,
                                    FIX_DIST_DEF, SFX_MAPUNIT_TWIP ); // fix is only in Writer
            aLineDistAtPercentBox.Hide();
            pActLineDistFld->Show();
            pActLineDistFld->Enable();
            aLineDistAtLabel.Enable();
        }
        break;
    }
    UpdateExample_Impl( sal_True );
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK_NOARG_INLINE_START(SvxStdParagraphTabPage, ModifyHdl_Impl)
{
    UpdateExample_Impl();
    return 0;
}
IMPL_LINK_NOARG_INLINE_END(SvxStdParagraphTabPage, ModifyHdl_Impl)

// -----------------------------------------------------------------------

void SvxStdParagraphTabPage::Init_Impl()
{
    aLineDist.SetSelectHdl(
        LINK( this, SvxStdParagraphTabPage, LineDistHdl_Impl ) );

    Link aLink = LINK( this, SvxStdParagraphTabPage, ELRLoseFocusHdl );
    aFLineIndent.SetLoseFocusHdl( aLink );
    aLeftIndent.SetLoseFocusHdl( aLink );
    aRightIndent.SetLoseFocusHdl( aLink );

    aLink = LINK( this, SvxStdParagraphTabPage, ModifyHdl_Impl );
    aFLineIndent.SetModifyHdl( aLink );
    aLeftIndent.SetModifyHdl( aLink );
    aRightIndent.SetModifyHdl( aLink );
    aTopDist.SetModifyHdl( aLink );
    aBottomDist.SetModifyHdl( aLink );

    aAutoCB.SetClickHdl( LINK( this, SvxStdParagraphTabPage, AutoHdl_Impl ));
    SfxItemPool* pPool = GetItemSet().GetPool();
    DBG_ASSERT( pPool, "Wo ist der Pool" );
    FieldUnit eUnit =
        MapToFieldUnit( pPool->GetMetric( GetWhich( SID_ATTR_LRSPACE ) ) );

    aTopDist.SetMax( aTopDist.Normalize( nAbst ), eUnit );
    aBottomDist.SetMax( aBottomDist.Normalize( nAbst ), eUnit );
    aLineDistAtMetricBox.SetMax(
        aLineDistAtMetricBox.Normalize( nAbst ), eUnit );
}

// -----------------------------------------------------------------------

void SvxStdParagraphTabPage::UpdateExample_Impl( sal_Bool bAll )
{
    aExampleWin.SetFirstLineOfst(
        (short)aFLineIndent.Denormalize( aFLineIndent.GetValue( FUNIT_TWIP ) ) );
    aExampleWin.SetLeftMargin(
        static_cast<long>(aLeftIndent.Denormalize( aLeftIndent.GetValue( FUNIT_TWIP ) ) ) );
    aExampleWin.SetRightMargin(
        static_cast<long>(aRightIndent.Denormalize( aRightIndent.GetValue( FUNIT_TWIP ) ) ) );
    aExampleWin.SetUpper(
        (sal_uInt16)aTopDist.Denormalize( aTopDist.GetValue( FUNIT_TWIP ) ) );
    aExampleWin.SetLower(
        (sal_uInt16)aBottomDist.Denormalize( aBottomDist.GetValue( FUNIT_TWIP ) ) );



    sal_uInt16 nPos = aLineDist.GetSelectEntryPos();

    switch ( nPos )
    {
        case LLINESPACE_1:
        case LLINESPACE_15:
        case LLINESPACE_2:
            aExampleWin.SetLineSpace( (SvxPrevLineSpace)nPos );
            break;

        case LLINESPACE_PROP:
            aExampleWin.SetLineSpace( (SvxPrevLineSpace)nPos,
                (sal_uInt16)aLineDistAtPercentBox.Denormalize(
                aLineDistAtPercentBox.GetValue() ) );
            break;

        case LLINESPACE_MIN:
        case LLINESPACE_DURCH:
        case LLINESPACE_FIX:
            aExampleWin.SetLineSpace( (SvxPrevLineSpace)nPos,
                (sal_uInt16)GetCoreValue( aLineDistAtMetricBox, SFX_MAPUNIT_TWIP ) );
            break;
    }
    aExampleWin.Draw( bAll );
}

// -----------------------------------------------------------------------

void SvxStdParagraphTabPage::EnableRegisterMode()
{
    aRegisterCB.Show();
    aRegisterFL.Show();
}

void SvxStdParagraphTabPage::EnableContextualMode()
{
    aContextualCB.Show();
}

IMPL_LINK( SvxStdParagraphTabPage, AutoHdl_Impl, CheckBox*, pBox )
{
    sal_Bool bEnable = !pBox->IsChecked();
    aFLineLabel .Enable(bEnable);
    aFLineIndent.Enable(bEnable);
    return 0;
}

void SvxStdParagraphTabPage::SetPageWidth( sal_uInt16 nPageWidth )
{
    nWidth = nPageWidth;
}


void SvxStdParagraphTabPage::EnableAutoFirstLine()
{
    aAutoCB.Show();
}


void    SvxStdParagraphTabPage::EnableAbsLineDist(long nMinTwip)
{
    aLineDist.InsertEntry(sAbsDist);
    nMinFixDist = nMinTwip;
}


void    SvxStdParagraphTabPage::PageCreated(SfxAllItemSet aSet)
{

/* different bit represent call to different method of SvxStdParagraphTabPage
                        0x0001 --->EnableRelativeMode()
                        0x0002 --->EnableRegisterMode()
                        0x0004 --->EnableAutoFirstLine()
                        0x0008 --->EnableNegativeMode()
                        0x0010 --->EnableContextualMode()
            */
    SFX_ITEMSET_ARG (&aSet,pPageWidthItem,SfxUInt16Item,SID_SVXSTDPARAGRAPHTABPAGE_PAGEWIDTH,sal_False);
    SFX_ITEMSET_ARG (&aSet,pFlagSetItem,SfxUInt32Item,SID_SVXSTDPARAGRAPHTABPAGE_FLAGSET,sal_False);
    SFX_ITEMSET_ARG (&aSet,pLineDistItem,SfxUInt32Item,SID_SVXSTDPARAGRAPHTABPAGE_ABSLINEDIST,sal_False);

    if (pPageWidthItem)
        SetPageWidth(pPageWidthItem->GetValue());

    if (pFlagSetItem )
        if (( 0x0001 & pFlagSetItem->GetValue())== 0x0001 )
            EnableRelativeMode();

    if (pFlagSetItem)
        if (( 0x0002 & pFlagSetItem->GetValue())== 0x0002 )
                EnableRegisterMode();

    if (pFlagSetItem)
        if ( ( 0x0004 & pFlagSetItem->GetValue())== 0x0004 )
            EnableAutoFirstLine();

    if(pLineDistItem)
        EnableAbsLineDist(pLineDistItem->GetValue());

    if (pFlagSetItem)
        if  (( 0x0008 & pFlagSetItem->GetValue()) == 0x0008 )
                EnableNegativeMode();

    if (pFlagSetItem)
        if  (( 0x0010 & pFlagSetItem->GetValue()) == 0x0010 )
                EnableContextualMode();
}


#define LASTLINEPOS_DEFAULT     0
#define LASTLINEPOS_LEFT        1

#define LASTLINECOUNT_OLD       3
#define LASTLINECOUNT_NEW       4

// class SvxParaAlignTabPage ------------------------------------------------

SvxParaAlignTabPage::SvxParaAlignTabPage( Window* pParent, const SfxItemSet& rSet )
    : SfxTabPage(pParent, CUI_RES( RID_SVXPAGE_ALIGN_PARAGRAPH ),rSet),
    aAlignFrm           ( this, CUI_RES( FL_ALIGN ) ),
    aLeft               ( this, CUI_RES( BTN_LEFTALIGN ) ),
    aRight              ( this, CUI_RES( BTN_RIGHTALIGN ) ),
    aCenter             ( this, CUI_RES( BTN_CENTERALIGN ) ),
    aJustify            ( this, CUI_RES( BTN_JUSTIFYALIGN ) ),
    aLastLineFT         ( this, CUI_RES( FT_LASTLINE ) ),
    aLastLineLB         ( this, CUI_RES( LB_LASTLINE ) ),
    aExpandCB           ( this, CUI_RES( CB_EXPAND ) ),
    aSnapToGridCB       ( this, CUI_RES( CB_SNAP ) ),
    aExampleWin         ( this, CUI_RES( WN_EXAMPLE ) ),

    aVertAlignFL        ( this, CUI_RES( FL_VERTALIGN ) ),
    aVertAlignFT        ( this, CUI_RES( FT_VERTALIGN ) ),
    aVertAlignLB        ( this, CUI_RES( LB_VERTALIGN ) ),

    aPropertiesFL       ( this, CUI_RES( FL_PROPERTIES    )),
    aTextDirectionFT    ( this, CUI_RES( FT_TEXTDIRECTION )),
    aTextDirectionLB    ( this, CUI_RES( LB_TEXTDIRECTION ))
{
    SvtLanguageOptions aLangOptions;
    sal_uInt16 nLastLinePos = LASTLINEPOS_DEFAULT;

    if ( aLangOptions.IsAsianTypographyEnabled() )
    {
        String sLeft(CUI_RES(ST_LEFTALIGN_ASIAN));
        aLeft.SetText(sLeft);
        aRight.SetText(String(CUI_RES(ST_RIGHTALIGN_ASIAN)));
        sLeft = MnemonicGenerator::EraseAllMnemonicChars( sLeft );

        if ( aLastLineLB.GetEntryCount() == LASTLINECOUNT_OLD )
        {
            aLastLineLB.RemoveEntry( 0 );
            aLastLineLB.InsertEntry( sLeft, 0 );
        }
        else
            nLastLinePos = LASTLINEPOS_LEFT;
    }
    // remove "Default" or "Left" entry, depends on CJKOptions
    if ( aLastLineLB.GetEntryCount() == LASTLINECOUNT_NEW )
        aLastLineLB.RemoveEntry( nLastLinePos );

    FreeResource();
    Link aLink = LINK( this, SvxParaAlignTabPage, AlignHdl_Impl );
    aLeft.SetClickHdl( aLink );
    aRight.SetClickHdl( aLink );
    aCenter.SetClickHdl( aLink );
    aJustify.SetClickHdl( aLink );
    aLastLineLB.SetSelectHdl( LINK( this, SvxParaAlignTabPage, LastLineHdl_Impl ) );
    aTextDirectionLB.SetSelectHdl( LINK( this, SvxParaAlignTabPage, TextDirectionHdl_Impl ) );

    sal_uInt16 nHtmlMode = GetHtmlMode_Impl(rSet);
    if(!(nHtmlMode & HTMLMODE_ON) || (0 != (nHtmlMode & HTMLMODE_SOME_STYLES)) )
    {
        if( aLangOptions.IsCTLFontEnabled() )
        {
            aTextDirectionLB.InsertEntryValue( CUI_RESSTR( RID_SVXSTR_FRAMEDIR_LTR ), FRMDIR_HORI_LEFT_TOP );
            aTextDirectionLB.InsertEntryValue( CUI_RESSTR( RID_SVXSTR_FRAMEDIR_RTL ), FRMDIR_HORI_RIGHT_TOP );
            aTextDirectionLB.InsertEntryValue( CUI_RESSTR( RID_SVXSTR_FRAMEDIR_SUPER ), FRMDIR_ENVIRONMENT );

            aPropertiesFL.Show();
            aTextDirectionFT.Show();
            aTextDirectionLB.Show();
        }
    }

    aExampleWin.SetAccessibleName(String(CUI_RES(STR_EXAMPLE)));

}

SvxParaAlignTabPage::~SvxParaAlignTabPage()
{
}

int SvxParaAlignTabPage::DeactivatePage( SfxItemSet* _pSet )
{
    if ( _pSet )
        FillItemSet( *_pSet );
    return LEAVE_PAGE;
}

SfxTabPage* SvxParaAlignTabPage::Create( Window* pParent, const SfxItemSet& rSet )
{
    return new SvxParaAlignTabPage(pParent, rSet);
}

sal_uInt16* SvxParaAlignTabPage::GetRanges()
{
    return pAlignRanges;

}

sal_Bool SvxParaAlignTabPage::FillItemSet( SfxItemSet& rOutSet )
{
    sal_Bool bModified = sal_False;

    bool bAdj = false, bChecked = false;
    SvxAdjust eAdjust = SVX_ADJUST_LEFT;

    if ( aLeft.IsChecked() )
    {
        eAdjust = SVX_ADJUST_LEFT;
        bAdj = !aLeft.GetSavedValue();
        bChecked = true;
    }
    else if ( aRight.IsChecked() )
    {
        eAdjust = SVX_ADJUST_RIGHT;
        bAdj = !aRight.GetSavedValue();
        bChecked = true;
    }
    else if ( aCenter.IsChecked() )
    {
        eAdjust = SVX_ADJUST_CENTER;
        bAdj = !aCenter.GetSavedValue();
        bChecked = true;
    }
    else if ( aJustify.IsChecked() )
    {
        eAdjust = SVX_ADJUST_BLOCK;
        bAdj = !aJustify.GetSavedValue() ||
            aExpandCB.IsChecked() != aExpandCB.GetSavedValue() ||
            aLastLineLB.GetSelectEntryPos() != aLastLineLB.GetSavedValue();
        bChecked = true;
    }
    sal_uInt16 _nWhich = GetWhich( SID_ATTR_PARA_ADJUST );

    if ( bAdj )
    {
        const SvxAdjustItem* pOld =
            (const SvxAdjustItem*)GetOldItem( rOutSet, SID_ATTR_PARA_ADJUST );
        SvxAdjust eOneWord = aExpandCB.IsChecked() ? SVX_ADJUST_BLOCK
                                                   : SVX_ADJUST_LEFT;
        sal_uInt16 nLBPos = aLastLineLB.GetSelectEntryPos();
        SvxAdjust eLastBlock = SVX_ADJUST_LEFT;

        if ( 1 == nLBPos )
            eLastBlock = SVX_ADJUST_CENTER;
        else if ( 2 == nLBPos )
            eLastBlock = SVX_ADJUST_BLOCK;

        bool bNothingWasChecked =
            !aLeft.GetSavedValue() && !aRight.GetSavedValue() &&
            !aCenter.GetSavedValue() && !aJustify.GetSavedValue();

        if ( !pOld || pOld->GetAdjust() != eAdjust ||
             pOld->GetOneWord() != eOneWord ||
             pOld->GetLastBlock() != eLastBlock ||
             ( bChecked && bNothingWasChecked ) )
        {
            bModified = sal_True;
            SvxAdjustItem aAdj(
                (const SvxAdjustItem&)GetItemSet().Get( _nWhich ) );
            aAdj.SetAdjust( eAdjust );
            aAdj.SetOneWord( eOneWord );
            aAdj.SetLastBlock( eLastBlock );
            rOutSet.Put( aAdj );
        }
    }
    if(aSnapToGridCB.IsChecked() != aSnapToGridCB.GetSavedValue())
    {
        rOutSet.Put(SvxParaGridItem(aSnapToGridCB.IsChecked(), GetWhich( SID_ATTR_PARA_SNAPTOGRID )));
        bModified = sal_True;
    }
    if(aVertAlignLB.GetSavedValue() != aVertAlignLB.GetSelectEntryPos())
    {
        rOutSet.Put(SvxParaVertAlignItem(aVertAlignLB.GetSelectEntryPos(), GetWhich( SID_PARA_VERTALIGN )));
        bModified = sal_True;
    }

    if( aTextDirectionLB.IsVisible() )
    {
        SvxFrameDirection eDir = aTextDirectionLB.GetSelectEntryValue();
        if( eDir != aTextDirectionLB.GetSavedValue() )
        {
            rOutSet.Put( SvxFrameDirectionItem( eDir, GetWhich( SID_ATTR_FRAMEDIRECTION ) ) );
            bModified = sal_True;
        }
    }

    return bModified;
}

void SvxParaAlignTabPage::Reset( const SfxItemSet& rSet )
{
    sal_uInt16 _nWhich = GetWhich( SID_ATTR_PARA_ADJUST );
    SfxItemState eItemState = rSet.GetItemState( _nWhich );

    sal_uInt16 nLBSelect = 0;
    if ( eItemState >= SFX_ITEM_AVAILABLE )
    {
        const SvxAdjustItem& rAdj = (const SvxAdjustItem&)rSet.Get( _nWhich );

        switch ( rAdj.GetAdjust() /*!!! ask VB rAdj.GetLastBlock()*/ )
        {
            case SVX_ADJUST_LEFT: aLeft.Check(); break;

            case SVX_ADJUST_RIGHT: aRight.Check(); break;

            case SVX_ADJUST_CENTER: aCenter.Check(); break;

            case SVX_ADJUST_BLOCK: aJustify.Check(); break;
            default: ; //prevent warning
        }
        sal_Bool bEnable = aJustify.IsChecked();
        aLastLineFT.Enable(bEnable);
        aLastLineLB.Enable(bEnable);
        aExpandCB  .Enable(bEnable);

        aExpandCB.Check(SVX_ADJUST_BLOCK == rAdj.GetOneWord());
        switch(rAdj.GetLastBlock())
        {
            case SVX_ADJUST_LEFT:  nLBSelect = 0; break;

            case SVX_ADJUST_CENTER: nLBSelect = 1;  break;

            case SVX_ADJUST_BLOCK: nLBSelect = 2;  break;
            default: ; //prevent warning
        }
    }
    else
    {
        aLeft.Check( sal_False );
        aRight.Check( sal_False );
        aCenter.Check( sal_False );
        aJustify.Check( sal_False );
    }
    aLastLineLB.SelectEntryPos(nLBSelect);

    sal_uInt16 nHtmlMode = GetHtmlMode_Impl(rSet);
    if(nHtmlMode & HTMLMODE_ON)
    {
        aLastLineLB.Hide();
        aLastLineFT.Hide();
        aExpandCB.Hide();
        if(!(nHtmlMode & HTMLMODE_FULL_STYLES) )
            aJustify.Disable();
        aSnapToGridCB.Show(sal_False);
    }
    _nWhich = GetWhich(SID_ATTR_PARA_SNAPTOGRID);
    eItemState = rSet.GetItemState( _nWhich );
    if ( eItemState >= SFX_ITEM_AVAILABLE )
    {
        const SvxParaGridItem& rSnap = (const SvxParaGridItem&)rSet.Get( _nWhich );
        aSnapToGridCB.Check(rSnap.GetValue());
    }

    _nWhich = GetWhich( SID_PARA_VERTALIGN );
    eItemState = rSet.GetItemState( _nWhich );

    if ( eItemState >= SFX_ITEM_AVAILABLE )
    {
        aVertAlignLB.Show();
        aVertAlignFL.Show();
        aVertAlignFT.Show();

        const SvxParaVertAlignItem& rAlign = (const SvxParaVertAlignItem&)rSet.Get( _nWhich );
        aVertAlignLB.SelectEntryPos(rAlign.GetValue());
    }

    _nWhich = GetWhich( SID_ATTR_FRAMEDIRECTION );
    //text direction
    if( SFX_ITEM_AVAILABLE <= rSet.GetItemState( _nWhich ) )
    {
        const SvxFrameDirectionItem& rFrameDirItem = ( const SvxFrameDirectionItem& ) rSet.Get( _nWhich );
        aTextDirectionLB.SelectEntryValue( (SvxFrameDirection)rFrameDirItem.GetValue() );
        aTextDirectionLB.SaveValue();
    }

    aSnapToGridCB.SaveValue();
    aVertAlignLB.SaveValue();
    aLeft.SaveValue();
    aRight.SaveValue();
    aCenter.SaveValue();
    aJustify.SaveValue();
    aLastLineLB.SaveValue();
    aExpandCB.SaveValue();

    UpdateExample_Impl(sal_True);
}

IMPL_LINK_NOARG(SvxParaAlignTabPage, AlignHdl_Impl)
{
    sal_Bool bJustify = aJustify.IsChecked();
    aLastLineFT.Enable(bJustify);
    aLastLineLB.Enable(bJustify);
    aExpandCB.Enable(bJustify);
    UpdateExample_Impl(sal_False);
    return 0;
}

IMPL_LINK_NOARG(SvxParaAlignTabPage, LastLineHdl_Impl)
{
    UpdateExample_Impl(sal_False);
    return 0;
}

IMPL_LINK_NOARG(SvxParaAlignTabPage, TextDirectionHdl_Impl)
{
    SvxFrameDirection eDir = aTextDirectionLB.GetSelectEntryValue();
    switch ( eDir )
    {
        // check the default alignment for this text direction
        case FRMDIR_HORI_LEFT_TOP :     aLeft.Check( sal_True ); break;
        case FRMDIR_HORI_RIGHT_TOP :    aRight.Check( sal_True ); break;
        case FRMDIR_ENVIRONMENT :       /* do nothing */ break;
        default:
        {
            SAL_WARN( "cui.tabpages", "SvxParaAlignTabPage::TextDirectionHdl_Impl(): other directions not supported" );
        }
    }

    return 0;
}

void    SvxParaAlignTabPage::UpdateExample_Impl( sal_Bool bAll )
{
    if ( aLeft.IsChecked() )
        aExampleWin.SetAdjust( SVX_ADJUST_LEFT );
    else if ( aRight.IsChecked() )
        aExampleWin.SetAdjust( SVX_ADJUST_RIGHT );
    else if ( aCenter.IsChecked() )
        aExampleWin.SetAdjust( SVX_ADJUST_CENTER );
    else if ( aJustify.IsChecked() )
    {
        aExampleWin.SetAdjust( SVX_ADJUST_BLOCK );
        SvxAdjust eLastBlock = SVX_ADJUST_LEFT;
        sal_uInt16 nLBPos = aLastLineLB.GetSelectEntryPos();
        if(nLBPos == 1)
            eLastBlock = SVX_ADJUST_CENTER;
        else if(nLBPos == 2)
            eLastBlock = SVX_ADJUST_BLOCK;
        aExampleWin.SetLastLine( eLastBlock );
    }

    aExampleWin.Draw( bAll );
}

void SvxParaAlignTabPage::EnableJustifyExt()
{
    aLastLineFT.Show();
    aLastLineLB.Show();
    aExpandCB  .Show();
    SvtLanguageOptions aCJKOptions;
    if(aCJKOptions.IsAsianTypographyEnabled())
        aSnapToGridCB.Show();

}

void SvxParaAlignTabPage::PageCreated (SfxAllItemSet aSet)
{
    SFX_ITEMSET_ARG (&aSet,pBoolItem,SfxBoolItem,SID_SVXPARAALIGNTABPAGE_ENABLEJUSTIFYEXT,sal_False);
    if (pBoolItem)
        if(pBoolItem->GetValue())
            EnableJustifyExt();
}

SfxTabPage* SvxExtParagraphTabPage::Create( Window* pParent,
                                            const SfxItemSet& rSet )
{
    return new SvxExtParagraphTabPage( pParent, rSet );
}

// -----------------------------------------------------------------------

sal_Bool SvxExtParagraphTabPage::FillItemSet( SfxItemSet& rOutSet )
{
    sal_Bool bModified = sal_False;
    sal_uInt16 _nWhich = GetWhich( SID_ATTR_PARA_HYPHENZONE );
    const TriState eHyphenState = m_pHyphenBox->GetState();
    const SfxPoolItem* pOld = GetOldItem( rOutSet, SID_ATTR_PARA_HYPHENZONE );

    if ( eHyphenState != m_pHyphenBox->GetSavedValue()     ||
         m_pExtHyphenBeforeBox->IsValueModified()          ||
         m_pExtHyphenAfterBox->IsValueModified()           ||
         m_pMaxHyphenEdit->IsValueModified() )
    {
        SvxHyphenZoneItem aHyphen(
            (const SvxHyphenZoneItem&)GetItemSet().Get( _nWhich ) );
        aHyphen.SetHyphen( eHyphenState == STATE_CHECK );

        if ( eHyphenState == STATE_CHECK )
        {
            aHyphen.GetMinLead() = (sal_uInt8)m_pExtHyphenBeforeBox->GetValue();
            aHyphen.GetMinTrail() = (sal_uInt8)m_pExtHyphenAfterBox->GetValue();
        }
        aHyphen.GetMaxHyphens() = (sal_uInt8)m_pMaxHyphenEdit->GetValue();

        if ( !pOld ||
            !( *(SvxHyphenZoneItem*)pOld == aHyphen ) ||
                eHyphenState != m_pHyphenBox->GetSavedValue())
        {
            rOutSet.Put( aHyphen );
            bModified = sal_True;
        }
    }

    if (m_pPagenumEdit->IsEnabled() && m_pPagenumEdit->IsValueModified())
    {
        SfxUInt16Item aPageNum( SID_ATTR_PARA_PAGENUM,
                                (sal_uInt16)m_pPagenumEdit->GetValue() );

        pOld = GetOldItem( rOutSet, SID_ATTR_PARA_PAGENUM );

        if ( !pOld || ( (const SfxUInt16Item*)pOld )->GetValue() != aPageNum.GetValue() )
        {
            rOutSet.Put( aPageNum );
            bModified = sal_True;
        }
    }

    // pagebreak

    TriState eState = m_pApplyCollBtn->GetState();
    bool bIsPageModel = false;

    _nWhich = GetWhich( SID_ATTR_PARA_MODEL );
    String sPage;
    if ( eState != m_pApplyCollBtn->GetSavedValue() ||
         ( STATE_CHECK == eState &&
           m_pApplyCollBox->GetSelectEntryPos() != m_pApplyCollBox->GetSavedValue() ) )
    {
        if ( eState == STATE_CHECK )
        {
            sPage = m_pApplyCollBox->GetSelectEntry();
            bIsPageModel = 0 != sPage.Len();
        }
        pOld = GetOldItem( rOutSet, SID_ATTR_PARA_MODEL );

        if ( !pOld || ( (const SvxPageModelItem*)pOld )->GetValue() != sPage )
        {
            rOutSet.Put( SvxPageModelItem( sPage, sal_False, _nWhich ) );
            bModified = sal_True;
        }
        else
            bIsPageModel = false;
    }
    else if(STATE_CHECK == eState && m_pApplyCollBtn->IsEnabled())
        bIsPageModel = true;
    else
        rOutSet.Put( SvxPageModelItem( sPage, sal_False, _nWhich ) );

    _nWhich = GetWhich( SID_ATTR_PARA_PAGEBREAK );

    if ( bIsPageModel )
        // if PageModel is turned on, always turn off PageBreak
        rOutSet.Put( SvxFmtBreakItem( SVX_BREAK_NONE, _nWhich ) );
    else
    {
        eState = m_pPageBreakBox->GetState();
        SfxItemState eModelState = GetItemSet().GetItemState(SID_ATTR_PARA_MODEL, sal_False);

        if ( (eModelState == SFX_ITEM_SET && STATE_CHECK == m_pPageBreakBox->GetState()) ||
             eState != m_pPageBreakBox->GetSavedValue()                ||
             m_pBreakTypeLB->GetSelectEntryPos() != m_pBreakTypeLB->GetSavedValue()   ||
             m_pBreakPositionLB->GetSelectEntryPos() != m_pBreakPositionLB->GetSavedValue() )
        {
            const SvxFmtBreakItem rOldBreak(
                    (const SvxFmtBreakItem&)GetItemSet().Get( _nWhich ));
            SvxFmtBreakItem aBreak(rOldBreak.GetBreak(), rOldBreak.Which());

            switch ( eState )
            {
                case STATE_CHECK:
                {
                    sal_Bool bBefore = m_pBreakPositionLB->GetSelectEntryPos() == 0;

                    if ( m_pBreakTypeLB->GetSelectEntryPos() == 0 )
                    {
                        if ( bBefore )
                            aBreak.SetValue( SVX_BREAK_PAGE_BEFORE );
                        else
                            aBreak.SetValue( SVX_BREAK_PAGE_AFTER );
                    }
                    else
                    {
                        if ( bBefore )
                            aBreak.SetValue( SVX_BREAK_COLUMN_BEFORE );
                        else
                            aBreak.SetValue( SVX_BREAK_COLUMN_AFTER );
                    }
                    break;
                }

                case STATE_NOCHECK:
                    aBreak.SetValue( SVX_BREAK_NONE );
                    break;
                default: ; //prevent warning
            }
            pOld = GetOldItem( rOutSet, SID_ATTR_PARA_PAGEBREAK );

            if ( eState != m_pPageBreakBox->GetSavedValue()                ||
                    !pOld || !( *(const SvxFmtBreakItem*)pOld == aBreak ) )
            {
                bModified = sal_True;
                rOutSet.Put( aBreak );
            }
        }
    }


    // paragraph split
    _nWhich = GetWhich( SID_ATTR_PARA_SPLIT );
    eState = m_pKeepTogetherBox->GetState();

    if ( eState != m_pKeepTogetherBox->GetSavedValue() )
    {
        pOld = GetOldItem( rOutSet, SID_ATTR_PARA_SPLIT );

        if ( !pOld || ( (const SvxFmtSplitItem*)pOld )->GetValue() !=
                      ( eState == STATE_NOCHECK ) )
        {
            rOutSet.Put( SvxFmtSplitItem( eState == STATE_NOCHECK, _nWhich ) );
            bModified = sal_True;
        }
    }

    // keep paragraphs
    _nWhich = GetWhich( SID_ATTR_PARA_KEEP );
    eState = m_pKeepParaBox->GetState();

    if ( eState != m_pKeepParaBox->GetSavedValue() )
    {
        pOld = GetOldItem( rOutSet, SID_ATTR_PARA_KEEP );

        // if the status has changed, putting is necessary
        rOutSet.Put( SvxFmtKeepItem( eState == STATE_CHECK, _nWhich ) );
        bModified = sal_True;
    }

    // widows and orphans
    _nWhich = GetWhich( SID_ATTR_PARA_WIDOWS );
    eState = m_pWidowBox->GetState();

    if ( eState != m_pWidowBox->GetSavedValue() ||
         m_pWidowRowNo->IsValueModified() )
    {
        SvxWidowsItem rItem( eState == STATE_CHECK ?
                             (sal_uInt8)m_pWidowRowNo->GetValue() : 0, _nWhich );
        pOld = GetOldItem( rOutSet, SID_ATTR_PARA_WIDOWS );

        if ( eState != m_pWidowBox->GetSavedValue() || !pOld || !( *(const SvxWidowsItem*)pOld == rItem ) )
        {
            rOutSet.Put( rItem );
            bModified = sal_True;
        }
    }

    _nWhich = GetWhich( SID_ATTR_PARA_ORPHANS );
    eState = m_pOrphanBox->GetState();

    if ( eState != m_pOrphanBox->GetSavedValue() ||
         m_pOrphanRowNo->IsValueModified() )
    {
        SvxOrphansItem rItem( eState == STATE_CHECK ?
                             (sal_uInt8)m_pOrphanRowNo->GetValue() : 0, _nWhich );
        pOld = GetOldItem( rOutSet, SID_ATTR_PARA_ORPHANS );

        if ( eState != m_pOrphanBox->GetSavedValue() ||
                !pOld ||
                    !( *(const SvxOrphansItem*)pOld == rItem ) )
        {
            rOutSet.Put( rItem );
            bModified = sal_True;
        }
    }

    return bModified;
}

// -----------------------------------------------------------------------

void SvxExtParagraphTabPage::Reset( const SfxItemSet& rSet )
{
    sal_uInt16 _nWhich = GetWhich( SID_ATTR_PARA_HYPHENZONE );
    SfxItemState eItemState = rSet.GetItemState( _nWhich );

    sal_Bool bItemAvailable = eItemState >= SFX_ITEM_AVAILABLE;
    sal_Bool bIsHyphen = sal_False;
    if( !bHtmlMode && bItemAvailable )
    {
        const SvxHyphenZoneItem& rHyphen =
            (const SvxHyphenZoneItem&)rSet.Get( _nWhich );
        m_pHyphenBox->EnableTriState( sal_False );

        bIsHyphen = rHyphen.IsHyphen();
        m_pHyphenBox->SetState( bIsHyphen ? STATE_CHECK : STATE_NOCHECK );

        m_pExtHyphenBeforeBox->SetValue( rHyphen.GetMinLead() );
        m_pExtHyphenAfterBox->SetValue( rHyphen.GetMinTrail() );
        m_pMaxHyphenEdit->SetValue( rHyphen.GetMaxHyphens() );
    }
    else
    {
        m_pHyphenBox->SetState( STATE_DONTKNOW );
    }
    sal_Bool bEnable = bItemAvailable && bIsHyphen;
    m_pExtHyphenBeforeBox->Enable(bEnable);
    m_pExtHyphenAfterBox->Enable(bEnable);
    m_pBeforeText->Enable(bEnable);
    m_pAfterText->Enable(bEnable);
    m_pMaxHyphenLabel->Enable(bEnable);
    m_pMaxHyphenEdit->Enable(bEnable);

    _nWhich = GetWhich( SID_ATTR_PARA_PAGENUM );

    if ( rSet.GetItemState(_nWhich) >= SFX_ITEM_AVAILABLE )
    {
        const sal_uInt16 nPageNum =
            ( (const SfxUInt16Item&)rSet.Get( _nWhich ) ).GetValue();
        m_pPagenumEdit->SetValue( nPageNum );
    }

    if ( bPageBreak )
    {
        // first handle PageModel
        _nWhich = GetWhich( SID_ATTR_PARA_MODEL );
        sal_Bool bIsPageModel = sal_False;
        eItemState = rSet.GetItemState( _nWhich );

        if ( eItemState >= SFX_ITEM_SET )
        {
            m_pApplyCollBtn->EnableTriState( sal_False );

            const SvxPageModelItem& rModel =
                (const SvxPageModelItem&)rSet.Get( _nWhich );
            String aStr( rModel.GetValue() );

            if ( aStr.Len() &&
                 m_pApplyCollBox->GetEntryPos( aStr ) != LISTBOX_ENTRY_NOTFOUND )
            {
                m_pApplyCollBox->SelectEntry( aStr );
                m_pApplyCollBtn->SetState( STATE_CHECK );
                bIsPageModel = sal_True;

                m_pPageBreakBox->Enable();
                m_pPageBreakBox->EnableTriState( sal_False );
                m_pBreakTypeFT->Enable();
                m_pBreakTypeLB->Enable();
                m_pBreakPositionFT->Enable();
                m_pBreakPositionLB->Enable();
                m_pApplyCollBtn->Enable();
                m_pPageBreakBox->SetState( STATE_CHECK );

                //select page break
                m_pBreakTypeLB->SelectEntryPos(0);
                //select break before
                m_pBreakPositionLB->SelectEntryPos(0);
            }
            else
            {
                m_pApplyCollBox->SetNoSelection();
                m_pApplyCollBtn->SetState( STATE_NOCHECK );
            }
        }
        else if ( SFX_ITEM_DONTCARE == eItemState )
        {
            m_pApplyCollBtn->EnableTriState( sal_True );
            m_pApplyCollBtn->SetState( STATE_DONTKNOW );
            m_pApplyCollBox->SetNoSelection();
        }
        else
        {
            m_pApplyCollBtn->Enable(sal_False);
            m_pApplyCollBox->Enable(sal_False);
            m_pPagenumEdit->Enable(sal_False);
            m_pPagenumText->Enable(sal_False);
        }

        if ( !bIsPageModel )
        {
            _nWhich = GetWhich( SID_ATTR_PARA_PAGEBREAK );
            eItemState = rSet.GetItemState( _nWhich );

            if ( eItemState >= SFX_ITEM_AVAILABLE )
            {
                const SvxFmtBreakItem& rPageBreak =
                    (const SvxFmtBreakItem&)rSet.Get( _nWhich );

                SvxBreak eBreak = (SvxBreak)rPageBreak.GetValue();

                // PageBreak not via CTRL-RETURN,
                // then CheckBox can be freed
                m_pPageBreakBox->Enable();
                m_pPageBreakBox->EnableTriState( sal_False );
                m_pBreakTypeFT->Enable();
                m_pBreakTypeLB->Enable();
                m_pBreakPositionFT->Enable();
                m_pBreakPositionLB->Enable();

                m_pPageBreakBox->SetState( STATE_CHECK );

                sal_Bool _bEnable =     eBreak != SVX_BREAK_NONE &&
                                eBreak != SVX_BREAK_COLUMN_BEFORE &&
                                eBreak != SVX_BREAK_COLUMN_AFTER;
                m_pApplyCollBtn->Enable(_bEnable);
                if(!_bEnable)
                {
                    m_pApplyCollBox->Enable(_bEnable);
                    m_pPagenumEdit->Enable(_bEnable);
                }

                if ( eBreak == SVX_BREAK_NONE )
                    m_pPageBreakBox->SetState( STATE_NOCHECK );

                sal_uInt16 nType = 0; // selection position in break type ListBox : Page
                sal_uInt16 nPosition = 0; //  selection position in break position ListBox : Before
                switch ( eBreak )
                {
                    case SVX_BREAK_PAGE_BEFORE:
                        break;
                    case SVX_BREAK_PAGE_AFTER:
                        nPosition = 1;
                        break;
                    case SVX_BREAK_COLUMN_BEFORE:
                        nType = 1;
                        break;
                    case SVX_BREAK_COLUMN_AFTER:
                        nType = 1;
                        nPosition = 1;
                        break;
                    default: ;//prevent warning
                }
                m_pBreakTypeLB->SelectEntryPos(nType);
                m_pBreakPositionLB->SelectEntryPos(nPosition);
            }
            else if ( SFX_ITEM_DONTCARE == eItemState )
                m_pPageBreakBox->SetState( STATE_DONTKNOW );
            else
            {
                m_pPageBreakBox->Enable(sal_False);
                m_pBreakTypeFT->Enable(sal_False);
                m_pBreakTypeLB->Enable(sal_False);
                m_pBreakPositionFT->Enable(sal_False);
                m_pBreakPositionLB->Enable(sal_False);
            }
        }

        PageBreakPosHdl_Impl( m_pBreakPositionLB );
        PageBreakHdl_Impl( m_pPageBreakBox );
    }

    _nWhich = GetWhich( SID_ATTR_PARA_KEEP );
    eItemState = rSet.GetItemState( _nWhich );

    if ( eItemState >= SFX_ITEM_AVAILABLE )
    {
        m_pKeepParaBox->EnableTriState( sal_False );
        const SvxFmtKeepItem& rKeep =
            (const SvxFmtKeepItem&)rSet.Get( _nWhich );

        if ( rKeep.GetValue() )
            m_pKeepParaBox->SetState( STATE_CHECK );
        else
            m_pKeepParaBox->SetState( STATE_NOCHECK );
    }
    else if ( SFX_ITEM_DONTCARE == eItemState )
        m_pKeepParaBox->SetState( STATE_DONTKNOW );
    else
        m_pKeepParaBox->Enable(sal_False);

    _nWhich = GetWhich( SID_ATTR_PARA_SPLIT );
    eItemState = rSet.GetItemState( _nWhich );

    if ( eItemState >= SFX_ITEM_AVAILABLE )
    {
        const SvxFmtSplitItem& rSplit =
            (const SvxFmtSplitItem&)rSet.Get( _nWhich );
        m_pKeepTogetherBox->EnableTriState( sal_False );

        if ( !rSplit.GetValue() )
            m_pKeepTogetherBox->SetState( STATE_CHECK );
        else
        {
            m_pKeepTogetherBox->SetState( STATE_NOCHECK );

            // widows and orphans
            m_pWidowBox->Enable();
            _nWhich = GetWhich( SID_ATTR_PARA_WIDOWS );
            SfxItemState eTmpState = rSet.GetItemState( _nWhich );

            if ( eTmpState >= SFX_ITEM_AVAILABLE )
            {
                const SvxWidowsItem& rWidow =
                    (const SvxWidowsItem&)rSet.Get( _nWhich );
                m_pWidowBox->EnableTriState( sal_False );
                const sal_uInt16 nLines = rWidow.GetValue();

                sal_Bool _bEnable = nLines > 0;
                m_pWidowRowNo->SetValue( m_pWidowRowNo->Normalize( nLines ) );
                m_pWidowBox->SetState( _bEnable ? STATE_CHECK : STATE_NOCHECK);
                m_pWidowRowNo->Enable(_bEnable);
                //m_pWidowRowLabel->Enable(_bEnable);

            }
            else if ( SFX_ITEM_DONTCARE == eTmpState )
                m_pWidowBox->SetState( STATE_DONTKNOW );
            else
                m_pWidowBox->Enable(sal_False);

            m_pOrphanBox->Enable();
            _nWhich = GetWhich( SID_ATTR_PARA_ORPHANS );
            eTmpState = rSet.GetItemState( _nWhich );

            if ( eTmpState >= SFX_ITEM_AVAILABLE )
            {
                const SvxOrphansItem& rOrphan =
                    (const SvxOrphansItem&)rSet.Get( _nWhich );
                const sal_uInt16 nLines = rOrphan.GetValue();
                m_pOrphanBox->EnableTriState( sal_False );

                sal_Bool _bEnable = nLines > 0;
                m_pOrphanBox->SetState( _bEnable ? STATE_CHECK : STATE_NOCHECK);
                m_pOrphanRowNo->SetValue( m_pOrphanRowNo->Normalize( nLines ) );
                m_pOrphanRowNo->Enable(_bEnable);
                m_pOrphanRowLabel->Enable(_bEnable);

            }
            else if ( SFX_ITEM_DONTCARE == eTmpState )
                m_pOrphanBox->SetState( STATE_DONTKNOW );
            else
                m_pOrphanBox->Enable(sal_False);
        }
    }
    else if ( SFX_ITEM_DONTCARE == eItemState )
        m_pKeepTogetherBox->SetState( STATE_DONTKNOW );
    else
        m_pKeepTogetherBox->Enable(sal_False);

    // so that everything is enabled correctly
    KeepTogetherHdl_Impl( 0 );
    WidowHdl_Impl( 0 );
    OrphanHdl_Impl( 0 );

    m_pHyphenBox->SaveValue();
    m_pExtHyphenBeforeBox->SaveValue();
    m_pExtHyphenAfterBox->SaveValue();
    m_pMaxHyphenEdit->SaveValue();
    m_pPageBreakBox->SaveValue();
    m_pBreakPositionLB->SaveValue();
    m_pBreakTypeLB->SaveValue();
    m_pApplyCollBtn->SaveValue();
    m_pApplyCollBox->SaveValue();
    m_pPagenumEdit->SaveValue();
    m_pKeepTogetherBox->SaveValue();
    m_pKeepParaBox->SaveValue();
    m_pWidowBox->SaveValue();
    m_pOrphanBox->SaveValue();
}

// -----------------------------------------------------------------------

int SvxExtParagraphTabPage::DeactivatePage( SfxItemSet* _pSet )
{
    if ( _pSet )
        FillItemSet( *_pSet );
    return LEAVE_PAGE;
}

// -----------------------------------------------------------------------

void SvxExtParagraphTabPage::DisablePageBreak()
{
    bPageBreak = sal_False;
    m_pPageBreakBox->Enable(sal_False);
    m_pBreakTypeLB->RemoveEntry(0);
    m_pBreakPositionFT->Enable(sal_False);
    m_pBreakPositionLB->Enable(sal_False);
    m_pApplyCollBtn->Enable(sal_False);
    m_pApplyCollBox->Enable(sal_False);
    m_pPagenumEdit->Enable(sal_False);
}

// -----------------------------------------------------------------------

SvxExtParagraphTabPage::SvxExtParagraphTabPage( Window* pParent, const SfxItemSet& rAttr ) :
    SfxTabPage( pParent, "TextFlowPage","cui/ui/textflowpage.ui", rAttr ),

    bPageBreak  ( sal_True ),
    bHtmlMode   ( sal_False ),
    nStdPos     ( 0 )
{
    // Hyphenation
    get(m_pHyphenBox,"checkAuto");
    get(m_pExtHyphenBeforeBox,"spinLineEnd");
    get(m_pExtHyphenAfterBox,"spinLineBegin");
    get(m_pMaxHyphenEdit,"spinMaxNum");
    get(m_pBeforeText,"labelLineBegin");
    get(m_pAfterText,"labelLineEnd");
    get(m_pMaxHyphenLabel,"labelMaxNum");

    //Page break
    get(m_pPageBreakBox,"checkInsert");
    get(m_pBreakTypeLB,"comboBreakType");
    get(m_pBreakPositionLB,"comboBreakPosition");
    get(m_pApplyCollBtn,"checkPageStyle");
    get(m_pApplyCollBox,"comboPageStyle");
    get(m_pPagenumEdit,"spinPageNumber");
    get(m_pBreakTypeFT,"labelType");
    get(m_pBreakPositionFT,"labelPosition");
    get(m_pPagenumText,"labelPageNum");


    // Options
    get(m_pKeepTogetherBox,"checkSplitPara");
    get(m_pKeepParaBox,"checkKeepPara");

    get(m_pOrphanBox,"checkOrphan");
    get(m_pOrphanRowNo,"spinOrphan");
    get(m_pOrphanRowLabel,"labelOrphan");

    get(m_pWidowBox,"checkWidow");
    get(m_pWidowRowNo,"spinWidow");
    get(m_pWidowRowLabel,"labelWidow");

    m_pApplyCollBox->SetAccessibleRelationLabeledBy(m_pApplyCollBtn);
    m_pApplyCollBox->SetAccessibleName(String(CUI_RES(STR_PAGE_STYLE)));

    m_pOrphanRowNo->SetAccessibleRelationLabeledBy(m_pOrphanBox);
    m_pWidowRowNo->SetAccessibleRelationLabeledBy(m_pWidowBox);

    // this page needs ExchangeSupport
    SetExchangeSupport();

    m_pHyphenBox->SetClickHdl(         LINK( this, SvxExtParagraphTabPage, HyphenClickHdl_Impl ) );
    m_pPageBreakBox->SetClickHdl(      LINK( this, SvxExtParagraphTabPage, PageBreakHdl_Impl ) );
    m_pKeepTogetherBox->SetClickHdl(   LINK( this, SvxExtParagraphTabPage, KeepTogetherHdl_Impl ) );
    m_pWidowBox->SetClickHdl(          LINK( this, SvxExtParagraphTabPage, WidowHdl_Impl ) );
    m_pOrphanBox->SetClickHdl(         LINK( this, SvxExtParagraphTabPage, OrphanHdl_Impl ) );
    m_pApplyCollBtn->SetClickHdl(      LINK( this, SvxExtParagraphTabPage, ApplyCollClickHdl_Impl ) );
    m_pBreakTypeLB->SetSelectHdl(      LINK( this, SvxExtParagraphTabPage, PageBreakTypeHdl_Impl ) );
    m_pBreakPositionLB->SetSelectHdl(  LINK( this, SvxExtParagraphTabPage, PageBreakPosHdl_Impl ) );

    SfxObjectShell* pSh = SfxObjectShell::Current();
    if ( pSh )
    {
        SfxStyleSheetBasePool* pPool = pSh->GetStyleSheetPool();
        pPool->SetSearchMask( SFX_STYLE_FAMILY_PAGE );
        SfxStyleSheetBase* pStyle = pPool->First();
        String aStdName;

        while( pStyle )
        {
            if ( aStdName.Len() == 0 )
                // first style == standard style
                aStdName = pStyle->GetName();
            m_pApplyCollBox->InsertEntry( pStyle->GetName() );
            pStyle = pPool->Next();
        }
        nStdPos = m_pApplyCollBox->GetEntryPos( aStdName );
    }

    sal_uInt16 nHtmlMode = GetHtmlMode_Impl( rAttr );
    if ( nHtmlMode & HTMLMODE_ON )
    {
        bHtmlMode = sal_True;
        m_pHyphenBox           ->Enable(sal_False);
        m_pBeforeText          ->Enable(sal_False);
        m_pExtHyphenBeforeBox  ->Enable(sal_False);
        m_pAfterText           ->Enable(sal_False);
        m_pExtHyphenAfterBox   ->Enable(sal_False);
        m_pMaxHyphenLabel      ->Enable(sal_False);
        m_pMaxHyphenEdit       ->Enable(sal_False);
        m_pPagenumText         ->Enable(sal_False);
        m_pPagenumEdit         ->Enable(sal_False);
        // no column break in HTML
        m_pBreakTypeLB->RemoveEntry(1);
    }
}

// -----------------------------------------------------------------------

SvxExtParagraphTabPage::~SvxExtParagraphTabPage()
{
}

// -----------------------------------------------------------------------

sal_uInt16* SvxExtParagraphTabPage::GetRanges()
{
    return pExtRanges;
}

// -----------------------------------------------------------------------

IMPL_LINK_NOARG(SvxExtParagraphTabPage, PageBreakHdl_Impl)
{
    switch ( m_pPageBreakBox->GetState() )
    {
        case STATE_CHECK:
            m_pBreakTypeFT->Enable();
            m_pBreakTypeLB->Enable();
            m_pBreakPositionFT->Enable();
            m_pBreakPositionLB->Enable();

            if ( 0 == m_pBreakTypeLB->GetSelectEntryPos()&&
                0 == m_pBreakPositionLB->GetSelectEntryPos() )
            {
                m_pApplyCollBtn->Enable();

                sal_Bool bEnable = STATE_CHECK == m_pApplyCollBtn->GetState() &&
                                            m_pApplyCollBox->GetEntryCount();
                m_pApplyCollBox->Enable(bEnable);
                if(!bHtmlMode)
                {
                    m_pPagenumText->Enable(bEnable);
                    m_pPagenumEdit->Enable(bEnable);
                }
            }
            break;

        case STATE_NOCHECK:
        case STATE_DONTKNOW:
            m_pApplyCollBtn->SetState( STATE_NOCHECK );
            m_pApplyCollBtn->Enable(sal_False);
            m_pApplyCollBox->Enable(sal_False);
            m_pPagenumText->Enable(sal_False);
            m_pPagenumEdit->Enable(sal_False);
            m_pBreakTypeFT->Enable(sal_False);
            m_pBreakTypeLB->Enable(sal_False);
            m_pBreakPositionFT->Enable(sal_False);
            m_pBreakPositionLB->Enable(sal_False);
            break;
    }
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK_NOARG(SvxExtParagraphTabPage, KeepTogetherHdl_Impl)
{
    sal_Bool bEnable = m_pKeepTogetherBox->GetState() == STATE_NOCHECK;
    m_pWidowBox->Enable(bEnable);
    m_pOrphanBox->Enable(bEnable);

    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK_NOARG(SvxExtParagraphTabPage, WidowHdl_Impl)
{
    switch ( m_pWidowBox->GetState() )
    {
        case STATE_CHECK:
            m_pWidowRowNo->Enable();
            m_pWidowRowLabel->Enable();
            m_pKeepTogetherBox->Enable(sal_False);
            break;

        case STATE_NOCHECK:
            if ( m_pOrphanBox->GetState() == STATE_NOCHECK )
                m_pKeepTogetherBox->Enable();

        // no break
        case STATE_DONTKNOW:
            m_pWidowRowNo->Enable(sal_False);
            m_pWidowRowLabel->Enable(sal_False);
            break;
    }
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK_NOARG(SvxExtParagraphTabPage, OrphanHdl_Impl)
{
    switch( m_pOrphanBox->GetState() )
    {
        case STATE_CHECK:
            m_pOrphanRowNo->Enable();
            m_pOrphanRowLabel->Enable();
            m_pKeepTogetherBox->Enable(sal_False);
            break;

        case STATE_NOCHECK:
            if ( m_pWidowBox->GetState() == STATE_NOCHECK )
                m_pKeepTogetherBox->Enable();

        // kein break
        case STATE_DONTKNOW:
            m_pOrphanRowNo->Enable(sal_False);
            m_pOrphanRowLabel->Enable(sal_False);
            break;
    }
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK_NOARG(SvxExtParagraphTabPage, HyphenClickHdl_Impl)
{

    sal_Bool bEnable = m_pHyphenBox->GetState() == STATE_CHECK;
    m_pBeforeText->Enable(bEnable);
    m_pExtHyphenBeforeBox->Enable(bEnable);
    m_pAfterText->Enable(bEnable);
    m_pExtHyphenAfterBox->Enable(bEnable);
    m_pMaxHyphenLabel->Enable(bEnable);
    m_pMaxHyphenEdit->Enable(bEnable);
    m_pHyphenBox->SetState( bEnable ? STATE_CHECK : STATE_NOCHECK);

    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK_NOARG(SvxExtParagraphTabPage, ApplyCollClickHdl_Impl)
{
    sal_Bool bEnable = sal_False;
    if ( m_pApplyCollBtn->GetState() == STATE_CHECK &&
         m_pApplyCollBox->GetEntryCount() )
    {
        bEnable = sal_True;
        m_pApplyCollBox->SelectEntryPos( nStdPos );
    }
    else
    {
        m_pApplyCollBox->SetNoSelection();
    }
    m_pApplyCollBox->Enable(bEnable);
    if(!bHtmlMode)
    {
        m_pPagenumText->Enable(bEnable);
        m_pPagenumEdit->Enable(bEnable);
    }
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxExtParagraphTabPage, PageBreakPosHdl_Impl, ListBox *, pListBox )
{
    if ( 0 == pListBox->GetSelectEntryPos() )
    {
        m_pApplyCollBtn->Enable();

        sal_Bool bEnable = m_pApplyCollBtn->GetState() == STATE_CHECK &&
                                    m_pApplyCollBox->GetEntryCount();

        m_pApplyCollBox->Enable(bEnable);
        if(!bHtmlMode)
        {
            m_pPagenumText->Enable(bEnable);
            m_pPagenumEdit->Enable(bEnable);
        }
    }
    else if ( 1 == pListBox->GetSelectEntryPos() )
    {
        m_pApplyCollBtn->SetState( STATE_NOCHECK );
        m_pApplyCollBtn->Enable(sal_False);
        m_pApplyCollBox->Enable(sal_False);
        m_pPagenumText->Enable(sal_False);
        m_pPagenumEdit->Enable(sal_False);
    }
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxExtParagraphTabPage, PageBreakTypeHdl_Impl, ListBox *, pListBox )
{
    //column break or break break after
    sal_uInt16 nBreakPos = m_pBreakPositionLB->GetSelectEntryPos();
    if ( pListBox->GetSelectEntryPos() == 1 || 1 == nBreakPos)
    {
        m_pApplyCollBtn->SetState( STATE_NOCHECK );
        m_pApplyCollBtn->Enable(sal_False);
        m_pApplyCollBox->Enable(sal_False);
        m_pPagenumText->Enable(sal_False);
        m_pPagenumEdit->Enable(sal_False);
    }
    else
        PageBreakPosHdl_Impl( m_pBreakPositionLB );
    return 0;
}

void SvxExtParagraphTabPage::PageCreated(SfxAllItemSet aSet)
{


    SFX_ITEMSET_ARG (&aSet,pDisablePageBreakItem,SfxBoolItem,SID_DISABLE_SVXEXTPARAGRAPHTABPAGE_PAGEBREAK,sal_False);

    if (pDisablePageBreakItem)
        if ( pDisablePageBreakItem->GetValue())
                    DisablePageBreak();


}

SvxAsianTabPage::SvxAsianTabPage( Window* pParent, const SfxItemSet& rSet ) :
    SfxTabPage(pParent, "AsianTypography","cui/ui/asiantypography.ui", rSet)

{
    get(m_pForbiddenRulesCB,"checkForbidList");
    get(m_pHangingPunctCB,"checkHangPunct");
    get(m_pScriptSpaceCB,"checkApplySpacing");

    Link aLink = LINK( this, SvxAsianTabPage, ClickHdl_Impl );
    m_pHangingPunctCB->SetClickHdl( aLink );
    m_pScriptSpaceCB->SetClickHdl( aLink );
    m_pForbiddenRulesCB->SetClickHdl( aLink );

}

SvxAsianTabPage::~SvxAsianTabPage()
{
}

SfxTabPage* SvxAsianTabPage::Create(    Window* pParent, const SfxItemSet& rSet )
{
    return new SvxAsianTabPage(pParent, rSet);
}

sal_uInt16*     SvxAsianTabPage::GetRanges()
{
    static sal_uInt16 pRanges[] =
    {
        SID_ATTR_PARA_SCRIPTSPACE, SID_ATTR_PARA_FORBIDDEN_RULES,
        0
    };
    return pRanges;
}

sal_Bool SvxAsianTabPage::FillItemSet( SfxItemSet& rSet )
{
    sal_Bool bRet = sal_False;
    SfxItemPool* pPool = rSet.GetPool();
    if (m_pScriptSpaceCB->IsEnabled() && m_pScriptSpaceCB->IsChecked() != m_pScriptSpaceCB->GetSavedValue())
    {
        SfxBoolItem* pNewItem = (SfxBoolItem*)rSet.Get(
            pPool->GetWhich(SID_ATTR_PARA_SCRIPTSPACE)).Clone();
        pNewItem->SetValue(m_pScriptSpaceCB->IsChecked());
        rSet.Put(*pNewItem);
        delete pNewItem;
        bRet = sal_True;
    }
    if (m_pHangingPunctCB->IsEnabled() && m_pHangingPunctCB->IsChecked() != m_pHangingPunctCB->GetSavedValue())
    {
        SfxBoolItem* pNewItem = (SfxBoolItem*)rSet.Get(
            pPool->GetWhich(SID_ATTR_PARA_HANGPUNCTUATION)).Clone();
        pNewItem->SetValue(m_pHangingPunctCB->IsChecked());
        rSet.Put(*pNewItem);
        delete pNewItem;
        bRet = sal_True;
    }
    if (m_pForbiddenRulesCB->IsEnabled() && m_pForbiddenRulesCB->IsChecked() != m_pForbiddenRulesCB->GetSavedValue())
    {
        SfxBoolItem* pNewItem = (SfxBoolItem*)rSet.Get(
            pPool->GetWhich(SID_ATTR_PARA_FORBIDDEN_RULES)).Clone();
        pNewItem->SetValue(m_pForbiddenRulesCB->IsChecked());
        rSet.Put(*pNewItem);
        delete pNewItem;
        bRet = sal_True;
    }
    return bRet;
}

static void lcl_SetBox(const SfxItemSet& rSet, sal_uInt16 nSlotId, CheckBox& rBox)
{
    sal_uInt16 _nWhich = rSet.GetPool()->GetWhich(nSlotId);
    SfxItemState eState = rSet.GetItemState(_nWhich, sal_True);
    if(!eState || eState == SFX_ITEM_DISABLED )
        rBox.Enable(sal_False);
    else if(eState >= SFX_ITEM_AVAILABLE)
    {
        rBox.EnableTriState( sal_False );
        rBox.Check(((const SfxBoolItem&)rSet.Get(_nWhich)).GetValue());
    }
    else
        rBox.SetState( STATE_DONTKNOW );
    rBox.SaveValue();
}


void SvxAsianTabPage::Reset( const SfxItemSet& rSet )
{
    lcl_SetBox(rSet, SID_ATTR_PARA_FORBIDDEN_RULES, *m_pForbiddenRulesCB );
    lcl_SetBox(rSet, SID_ATTR_PARA_HANGPUNCTUATION, *m_pHangingPunctCB );

    //character distance not yet available
    lcl_SetBox(rSet, SID_ATTR_PARA_SCRIPTSPACE, *m_pScriptSpaceCB );
}

IMPL_LINK( SvxAsianTabPage, ClickHdl_Impl, CheckBox*, pBox )
{
    pBox->EnableTriState( sal_False );
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
