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
#include <sfx2/dialoghelper.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/module.hxx>
#include <vcl/mnemonic.hxx>
#include <svx/dialogs.hrc>

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
#include <sfx2/htmlmode.hxx>
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

    sal_Int64 nL = m_pLeftIndent->Denormalize( m_pLeftIndent->GetValue( eUnit ) );
    sal_Int64 nR = m_pRightIndent->Denormalize( m_pRightIndent->GetValue( eUnit ) );
    OUString aTmp = m_pFLineIndent->GetText();

    if( m_pLeftIndent->GetMin() < 0 )
        m_pFLineIndent->SetMin( -99999, FUNIT_MM );
    else
        m_pFLineIndent->SetMin( m_pFLineIndent->Normalize( -nL ), eUnit );

    // Check only for concrete width (Shell)
    sal_Int64 nTmp = nWidth - nL - nR - MM50;
    m_pFLineIndent->SetMax( m_pFLineIndent->Normalize( nTmp ), eUnit );

    if ( aTmp.isEmpty() )
        m_pFLineIndent->SetEmptyFieldValue();
    // maximum left right
    aTmp = m_pLeftIndent->GetText();
    nTmp = nWidth - nR - MM50;
    m_pLeftIndent->SetMax( m_pLeftIndent->Normalize( nTmp ), eUnit );

    if ( aTmp.isEmpty() )
        m_pLeftIndent->SetEmptyFieldValue();
    aTmp = m_pRightIndent->GetText();
    nTmp = nWidth - nL - MM50;
    m_pRightIndent->SetMax( m_pRightIndent->Normalize( nTmp ), eUnit );

    if ( aTmp.isEmpty() )
        m_pRightIndent->SetEmptyFieldValue();
    return 0;
}

// -----------------------------------------------------------------------

SfxTabPage* SvxStdParagraphTabPage::Create( Window* pParent, const SfxItemSet& rSet)
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
    sal_uInt16 nPos = m_pLineDist->GetSelectEntryPos();

    if ( LISTBOX_ENTRY_NOTFOUND != nPos &&
         ( nPos != m_pLineDist->GetSavedValue() ||
           m_pLineDistAtPercentBox->IsValueModified() ||
           m_pLineDistAtMetricBox->IsValueModified() ) )
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
                                   static_cast<long>(m_pLineDistAtPercentBox->Denormalize(
                                   m_pLineDistAtPercentBox->GetValue() )) );
                break;

            case LLINESPACE_MIN:
            case LLINESPACE_DURCH:
            case LLINESPACE_FIX:
                SetLineSpace_Impl( aSpacing, nPos,
                    GetCoreValue( *m_pLineDistAtMetricBox, eUnit ) );
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

    if ( m_pTopDist->IsValueModified() || m_pBottomDist->IsValueModified()
            || m_pContextualCB->GetSavedValue() != m_pContextualCB->IsChecked())
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

            if ( m_pTopDist->IsRelative() )
                aMargin.SetUpper( rOldItem.GetUpper(),
                                  (sal_uInt16)m_pTopDist->GetValue() );
            else
                aMargin.SetUpper( (sal_uInt16)GetCoreValue( *m_pTopDist, eUnit ) );

            if ( m_pBottomDist->IsRelative() )
                aMargin.SetLower( rOldItem.GetLower(),
                                  (sal_uInt16)m_pBottomDist->GetValue() );
            else
                aMargin.SetLower( (sal_uInt16)GetCoreValue( *m_pBottomDist, eUnit ) );

        }
        else
        {
            aMargin.SetUpper( (sal_uInt16)GetCoreValue( *m_pTopDist, eUnit ) );
            aMargin.SetLower( (sal_uInt16)GetCoreValue( *m_pBottomDist, eUnit ) );
        }
        aMargin.SetContextValue(m_pContextualCB->IsChecked());
        eState = GetItemSet().GetItemState( nWhich );

        if ( !pOld || !( *(const SvxULSpaceItem*)pOld == aMargin ) ||
             SFX_ITEM_DONTCARE == eState )
        {
            rOutSet.Put( aMargin );
            bModified = sal_True;
        }
    }
    bool bNullTab = false;

    if ( m_pLeftIndent->IsValueModified() ||
         m_pFLineIndent->IsValueModified() ||
         m_pRightIndent->IsValueModified()
         ||  m_pAutoCB->GetSavedValue() != m_pAutoCB->IsChecked() )
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

            if ( m_pLeftIndent->IsRelative() )
                aMargin.SetTxtLeft( rOldItem.GetTxtLeft(),
                                    (sal_uInt16)m_pLeftIndent->GetValue() );
            else
                aMargin.SetTxtLeft( GetCoreValue( *m_pLeftIndent, eUnit ) );

            if ( m_pRightIndent->IsRelative() )
                aMargin.SetRight( rOldItem.GetRight(),
                                  (sal_uInt16)m_pRightIndent->GetValue() );
            else
                aMargin.SetRight( GetCoreValue( *m_pRightIndent, eUnit ) );

            if ( m_pFLineIndent->IsRelative() )
                aMargin.SetTxtFirstLineOfst( rOldItem.GetTxtFirstLineOfst(),
                                             (sal_uInt16)m_pFLineIndent->GetValue() );
            else
                aMargin.SetTxtFirstLineOfst(
                    (sal_uInt16)GetCoreValue( *m_pFLineIndent, eUnit ) );
        }
        else
        {
            aMargin.SetTxtLeft( GetCoreValue( *m_pLeftIndent, eUnit ) );
            aMargin.SetRight( GetCoreValue( *m_pRightIndent, eUnit ) );
            aMargin.SetTxtFirstLineOfst(
                (sal_uInt16)GetCoreValue( *m_pFLineIndent, eUnit ) );
        }
        aMargin.SetAutoFirst(m_pAutoCB->IsChecked());
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
    if( m_pRegisterCB->IsVisible())
    {
        const SfxBoolItem* pBoolItem = (SfxBoolItem*)GetOldItem(
                            rOutSet, SID_ATTR_PARA_REGISTER);
        SfxBoolItem* pRegItem = (SfxBoolItem*)pBoolItem->Clone();
        sal_uInt16 _nWhich = GetWhich( SID_ATTR_PARA_REGISTER );
        sal_Bool bSet = pRegItem->GetValue();

        if(m_pRegisterCB->IsChecked() != bSet )
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

    SetFieldUnit( *m_pLeftIndent, eFUnit );
    SetFieldUnit( *m_pRightIndent, eFUnit );
    SetFieldUnit( *m_pFLineIndent, eFUnit );
    if ( eFUnit == FUNIT_CHAR )
    {
        SetFieldUnit( *m_pTopDist, FUNIT_LINE );
        SetFieldUnit( *m_pBottomDist, FUNIT_LINE );
        SetFieldUnit( *m_pLineDistAtMetricBox, FUNIT_POINT );
    }
    else
    {
        SetFieldUnit( *m_pTopDist, eFUnit );
        SetFieldUnit( *m_pBottomDist, eFUnit );
        SetFieldUnit( *m_pLineDistAtMetricBox, eFUnit );
    }

    ELRLoseFocusHdl( NULL ); // reset min/max values

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
                m_pLeftIndent->SetRelative( sal_True );
                m_pLeftIndent->SetValue( rOldItem.GetPropLeft() );
            }
            else
            {
                m_pLeftIndent->SetRelative();
                SetFieldUnit( *m_pLeftIndent, eFUnit );
                SetMetricValue( *m_pLeftIndent, rOldItem.GetTxtLeft(), eUnit );
            }

            if ( rOldItem.GetPropRight() != 100 )
            {
                m_pRightIndent->SetRelative( sal_True );
                m_pRightIndent->SetValue( rOldItem.GetPropRight() );
            }
            else
            {
                m_pRightIndent->SetRelative();
                SetFieldUnit( *m_pRightIndent, eFUnit );
                SetMetricValue( *m_pRightIndent, rOldItem.GetRight(), eUnit );
            }

            if ( rOldItem.GetPropTxtFirstLineOfst() != 100 )
            {
                m_pFLineIndent->SetRelative( sal_True );
                m_pFLineIndent->SetValue( rOldItem.GetPropTxtFirstLineOfst() );
            }
            else
            {
                m_pFLineIndent->SetRelative();
                m_pFLineIndent->SetMin(-9999);
                SetFieldUnit( *m_pFLineIndent, eFUnit );
                SetMetricValue( *m_pFLineIndent, rOldItem.GetTxtFirstLineOfst(),
                                eUnit );
            }
            m_pAutoCB->Check(rOldItem.IsAutoFirst());
        }
        else
        {
            const SvxLRSpaceItem& rSpace =
                (const SvxLRSpaceItem&)rSet.Get( _nWhich );

            SetMetricValue( *m_pLeftIndent, rSpace.GetTxtLeft(), eUnit );
            SetMetricValue( *m_pRightIndent, rSpace.GetRight(), eUnit );
            SetMetricValue( *m_pFLineIndent, rSpace.GetTxtFirstLineOfst(), eUnit );
            m_pAutoCB->Check(rSpace.IsAutoFirst());
        }
        AutoHdl_Impl(m_pAutoCB);
    }
    else
    {
        m_pLeftIndent->SetEmptyFieldValue();
        m_pRightIndent->SetEmptyFieldValue();
        m_pFLineIndent->SetEmptyFieldValue();
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
                m_pTopDist->SetRelative( sal_True );
                m_pTopDist->SetValue( rOldItem.GetPropUpper() );
            }
            else
            {
                m_pTopDist->SetRelative();
                if ( eFUnit == FUNIT_CHAR )
                    SetFieldUnit( *m_pTopDist, FUNIT_LINE );
                else
                    SetFieldUnit( *m_pTopDist, eFUnit );
                SetMetricValue( *m_pTopDist, rOldItem.GetUpper(), eUnit );
            }

            if ( rOldItem.GetPropLower() != 100 )
            {
                m_pBottomDist->SetRelative( sal_True );
                m_pBottomDist->SetValue( rOldItem.GetPropLower() );
            }
            else
            {
                m_pBottomDist->SetRelative();
                if ( eFUnit == FUNIT_CHAR )
                    SetFieldUnit( *m_pBottomDist, FUNIT_LINE );
                else
                    SetFieldUnit( *m_pBottomDist, eFUnit );
                SetMetricValue( *m_pBottomDist, rOldItem.GetLower(), eUnit );
            }
        }
        else
        {
            SetMetricValue( *m_pTopDist, rOldItem.GetUpper(), eUnit );
            SetMetricValue( *m_pBottomDist, rOldItem.GetLower(), eUnit );
        }
        m_pContextualCB->Check(rOldItem.GetContext());
    }
    else
    {
        m_pTopDist->SetEmptyFieldValue();
        m_pBottomDist->SetEmptyFieldValue();
    }

    _nWhich = GetWhich( SID_ATTR_PARA_LINESPACE );
    eItemState = rSet.GetItemState( _nWhich );

    if ( eItemState >= SFX_ITEM_AVAILABLE )
        SetLineSpacing_Impl( (const SvxLineSpacingItem &)rSet.Get( _nWhich ) );
    else
        m_pLineDist->SetNoSelection();


    _nWhich = GetWhich( SID_ATTR_PARA_REGISTER );
    eItemState = rSet.GetItemState( _nWhich );

    if ( eItemState >= SFX_ITEM_AVAILABLE )
        m_pRegisterCB->Check( ((const SfxBoolItem &)rSet.Get( _nWhich )).GetValue());
    m_pRegisterCB->SaveValue();
    sal_uInt16 nHtmlMode = GetHtmlMode_Impl(rSet);
    if(nHtmlMode & HTMLMODE_ON)
    {
        m_pRegisterFL->Hide();
        m_pRegisterCB->Hide();
        m_pAutoCB->Hide();

        if(!(nHtmlMode & HTMLMODE_SOME_STYLES)) // IE or SW
        {
            m_pRightLabel->Disable();
            m_pRightIndent->Disable();
            m_pTopDist->Disable();  //HTML3.2 and NS 3.0
            m_pBottomDist->Disable();
            m_pFLineIndent->Disable();
            m_pFLineLabel->Disable();
        }
    }

    m_pAutoCB->SaveValue();
    m_pContextualCB->SaveValue();
    m_pLineDist->SaveValue();
}

// -----------------------------------------------------------------------

void SvxStdParagraphTabPage::EnableRelativeMode()
{
    DBG_ASSERT( GetItemSet().GetParent(), "RelativeMode, but no parent-set!" );

    m_pLeftIndent->EnableRelativeMode( 0, 999 );
    m_pFLineIndent->EnableRelativeMode( 0, 999 );
    m_pRightIndent->EnableRelativeMode( 0, 999 );
    m_pTopDist->EnableRelativeMode( 0, 999 );
    m_pBottomDist->EnableRelativeMode( 0, 999 );
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

SvxStdParagraphTabPage::SvxStdParagraphTabPage( Window* pParent,  const SfxItemSet& rAttr ) :
    SfxTabPage( pParent, "ParaIndentSpacing","cui/ui/paraindentspacing.ui", rAttr ),

    nAbst           ( MAX_DURCH ),
    nWidth          ( 11905 /*567 * 50*/ ),
    nMinFixDist(0L),

    bRelativeMode   ( sal_False ),
    bNegativeIndents(sal_False)

{
    get(m_pLeftIndent,"spinED_LEFTINDENT");
    get(m_pRightIndent,"spinED_RIGHTINDENT");
    get(m_pRightLabel,"labelFT_RIGHTINDENT");
    get(m_pFLineLabel,"labelFT_FLINEINDENT");
    get(m_pFLineIndent,"spinED_FLINEINDENT");
    get(m_pAutoCB,"checkCB_AUTO");

    get(m_pTopDist,"spinED_TOPDIST");
    get(m_pBottomDist,"spinED_BOTTOMDIST");
    get(m_pContextualCB,"checkCB_CONTEXTUALSPACING");

    get(m_pLineDist,"comboLB_LINEDIST");
    get(m_pLineDistAtPercentBox,"spinED_LINEDISTPERCENT");
    get(m_pLineDistAtMetricBox,"spinED_LINEDISTMETRIC");
    get(m_pLineDistAtLabel,"labelFT_LINEDIST");

    get(m_pAbsDist,"labelST_LINEDIST_ABS");
    sAbsDist = m_pAbsDist->GetText();

    get(m_pRegisterFL,"frameFL_REGISTER");
    get(m_pRegisterCB,"checkCB_REGISTER");

    get(m_pExampleWin,"drawingareaWN_EXAMPLE");

    // this page needs ExchangeSupport
    SetExchangeSupport();

    m_pLineDistAtMetricBox->Hide();

    Init_Impl();
    m_pFLineIndent->SetMin(-9999);    // is set to 0 on default

    setPreviewsToSamePlace(pParent, this);
}

SvxStdParagraphTabPage::~SvxStdParagraphTabPage()
{}

// -----------------------------------------------------------------------

void SvxStdParagraphTabPage::EnableNegativeMode()
{
    m_pLeftIndent->SetMin(-9999);
    m_pRightIndent->SetMin(-9999);
    m_pRightIndent->EnableNegativeMode();
    m_pLeftIndent->EnableNegativeMode();
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
                    m_pLineDist->SelectEntryPos( LLINESPACE_1 );
                    break;

                // Default single line spacing
                case SVX_INTER_LINE_SPACE_PROP:
                    if ( 100 == rAttr.GetPropLineSpace() )
                    {
                        m_pLineDist->SelectEntryPos( LLINESPACE_1 );
                        break;
                    }
                    // 1.5 line spacing
                    if ( 150 == rAttr.GetPropLineSpace() )
                    {
                        m_pLineDist->SelectEntryPos( LLINESPACE_15 );
                        break;
                    }
                    // double line spacing
                    if ( 200 == rAttr.GetPropLineSpace() )
                    {
                        m_pLineDist->SelectEntryPos( LLINESPACE_2 );
                        break;
                    }
                    // the set per cent value
                    m_pLineDistAtPercentBox->SetValue( m_pLineDistAtPercentBox->Normalize(rAttr.GetPropLineSpace() ) );
                    m_pLineDist->SelectEntryPos( LLINESPACE_PROP );
                    break;

                case SVX_INTER_LINE_SPACE_FIX:
                    SetMetricValue( *m_pLineDistAtMetricBox, rAttr.GetInterLineSpace(), eUnit );
                    m_pLineDist->SelectEntryPos( LLINESPACE_DURCH );
                    break;
                default: ;//prevent warning
            }
        }
        break;
        case SVX_LINE_SPACE_FIX:
            SetMetricValue(*m_pLineDistAtMetricBox, rAttr.GetLineHeight(), eUnit);
            m_pLineDist->SelectEntryPos( LLINESPACE_FIX );
        break;

        case SVX_LINE_SPACE_MIN:
            SetMetricValue(*m_pLineDistAtMetricBox, rAttr.GetLineHeight(), eUnit);
            m_pLineDist->SelectEntryPos( LLINESPACE_MIN );
            break;
        default: ;//prevent warning
    }
    LineDistHdl_Impl( m_pLineDist );
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxStdParagraphTabPage, LineDistHdl_Impl, ListBox *, pBox )
{
    switch( pBox->GetSelectEntryPos() )
    {
        case LLINESPACE_1:
        case LLINESPACE_15:
        case LLINESPACE_2:
            m_pLineDistAtLabel->Enable(sal_False);
            m_pLineDistAtPercentBox->Enable(sal_False);
            m_pLineDistAtPercentBox->SetText( OUString() );
            m_pLineDistAtMetricBox->Enable(sal_False);
            m_pLineDistAtMetricBox->SetText( OUString() );
            break;

        case LLINESPACE_DURCH:
            // setting a sensible default?
            // limit MS min(10, aPageSize)
            m_pLineDistAtMetricBox->SetMin(0);

            if ( m_pLineDistAtMetricBox->GetText().isEmpty() )
                m_pLineDistAtMetricBox->SetValue( m_pLineDistAtMetricBox->Normalize( 1 ) );
            m_pLineDistAtPercentBox->Hide();
            m_pLineDistAtMetricBox->Show();
            m_pLineDistAtMetricBox->Enable();
            m_pLineDistAtLabel->Enable();
            break;

        case LLINESPACE_MIN:
            m_pLineDistAtMetricBox->SetMin(0);

            if ( m_pLineDistAtMetricBox->GetText().isEmpty() )
                m_pLineDistAtMetricBox->SetValue( m_pLineDistAtMetricBox->Normalize( 10 ), FUNIT_TWIP );
            m_pLineDistAtPercentBox->Hide();
            m_pLineDistAtMetricBox->Show();
            m_pLineDistAtMetricBox->Enable();
            m_pLineDistAtLabel->Enable();
            break;

        case LLINESPACE_PROP:

            if ( m_pLineDistAtPercentBox->GetText().isEmpty() )
                m_pLineDistAtPercentBox->SetValue( m_pLineDistAtPercentBox->Normalize( 100 ), FUNIT_TWIP );
            m_pLineDistAtMetricBox->Hide();
            m_pLineDistAtPercentBox->Show();
            m_pLineDistAtPercentBox->Enable();
            m_pLineDistAtLabel->Enable();
            break;
        case LLINESPACE_FIX:
        {
            sal_Int64 nTemp = m_pLineDistAtMetricBox->GetValue();
            m_pLineDistAtMetricBox->SetMin(m_pLineDistAtMetricBox->Normalize(nMinFixDist), FUNIT_TWIP);

            // if the value has been changed at SetMin,
            // it is time for the default
            if ( m_pLineDistAtMetricBox->GetValue() != nTemp )
                SetMetricValue( *m_pLineDistAtMetricBox, FIX_DIST_DEF, SFX_MAPUNIT_TWIP ); // fix is only in Writer
            m_pLineDistAtPercentBox->Hide();
            m_pLineDistAtMetricBox->Show();
            m_pLineDistAtMetricBox->Enable();
            m_pLineDistAtLabel->Enable();
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
    m_pLineDist->SetSelectHdl(
        LINK( this, SvxStdParagraphTabPage, LineDistHdl_Impl ) );

    Link aLink = LINK( this, SvxStdParagraphTabPage, ELRLoseFocusHdl );
    m_pFLineIndent->SetLoseFocusHdl( aLink );
    m_pLeftIndent->SetLoseFocusHdl( aLink );
    m_pRightIndent->SetLoseFocusHdl( aLink );

    aLink = LINK( this, SvxStdParagraphTabPage, ModifyHdl_Impl );
    m_pFLineIndent->SetModifyHdl( aLink );
    m_pLeftIndent->SetModifyHdl( aLink );
    m_pRightIndent->SetModifyHdl( aLink );
    m_pTopDist->SetModifyHdl( aLink );
    m_pBottomDist->SetModifyHdl( aLink );

    m_pAutoCB->SetClickHdl( LINK( this, SvxStdParagraphTabPage, AutoHdl_Impl ));
    SfxItemPool* pPool = GetItemSet().GetPool();
    DBG_ASSERT( pPool, "Wo ist der Pool" );
    FieldUnit eUnit = MapToFieldUnit( pPool->GetMetric( GetWhich( SID_ATTR_LRSPACE ) ) );

    m_pTopDist->SetMax( m_pTopDist->Normalize( nAbst ), eUnit );
    m_pBottomDist->SetMax( m_pBottomDist->Normalize( nAbst ), eUnit );
    m_pLineDistAtMetricBox->SetMax( m_pLineDistAtMetricBox->Normalize( nAbst ), eUnit );
}

// -----------------------------------------------------------------------

void SvxStdParagraphTabPage::UpdateExample_Impl( sal_Bool bAll )
{
    m_pExampleWin->SetFirstLineOfst( (short)m_pFLineIndent->Denormalize( m_pFLineIndent->GetValue( FUNIT_TWIP ) ) );
    m_pExampleWin->SetLeftMargin( static_cast<long>(m_pLeftIndent->Denormalize( m_pLeftIndent->GetValue( FUNIT_TWIP ) ) ) );
    m_pExampleWin->SetRightMargin( static_cast<long>(m_pRightIndent->Denormalize( m_pRightIndent->GetValue( FUNIT_TWIP ) ) ) );
    m_pExampleWin->SetUpper( (sal_uInt16)m_pTopDist->Denormalize( m_pTopDist->GetValue( FUNIT_TWIP ) ) );
    m_pExampleWin->SetLower( (sal_uInt16)m_pBottomDist->Denormalize( m_pBottomDist->GetValue( FUNIT_TWIP ) ) );

    sal_uInt16 nPos = m_pLineDist->GetSelectEntryPos();

    switch ( nPos )
    {
        case LLINESPACE_1:
        case LLINESPACE_15:
        case LLINESPACE_2:
            m_pExampleWin->SetLineSpace( (SvxPrevLineSpace)nPos );
            break;

        case LLINESPACE_PROP:
            m_pExampleWin->SetLineSpace( (SvxPrevLineSpace)nPos,
                (sal_uInt16)m_pLineDistAtPercentBox->Denormalize( m_pLineDistAtPercentBox->GetValue() ) );
            break;

        case LLINESPACE_MIN:
        case LLINESPACE_DURCH:
        case LLINESPACE_FIX:
            m_pExampleWin->SetLineSpace( (SvxPrevLineSpace)nPos,
                (sal_uInt16)GetCoreValue( *m_pLineDistAtMetricBox, SFX_MAPUNIT_TWIP ) );
            break;
    }
    m_pExampleWin->Draw( bAll );
}

// -----------------------------------------------------------------------

void SvxStdParagraphTabPage::EnableRegisterMode()
{
    m_pRegisterCB->Show();
    m_pRegisterFL->Show();
}

void SvxStdParagraphTabPage::EnableContextualMode()
{
    m_pContextualCB->Show();
}

IMPL_LINK( SvxStdParagraphTabPage, AutoHdl_Impl, CheckBox*, pBox )
{
    sal_Bool bEnable = !pBox->IsChecked();
    m_pFLineLabel->Enable(bEnable);
    m_pFLineIndent->Enable(bEnable);
    return 0;
}

void SvxStdParagraphTabPage::SetPageWidth( sal_uInt16 nPageWidth )
{
    nWidth = nPageWidth;
}


void SvxStdParagraphTabPage::EnableAutoFirstLine()
{
    m_pAutoCB->Show();
}


void    SvxStdParagraphTabPage::EnableAbsLineDist(long nMinTwip)
{
    m_pLineDist->InsertEntry(sAbsDist);
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

    : SfxTabPage(pParent, "ParaAlignPage", "cui/ui/paragalignpage.ui",rSet)
{
    get(m_pLeft,"radioBTN_LEFTALIGN");
    get(m_pRight,"radioBTN_RIGHTALIGN");
    get(m_pCenter,"radioBTN_CENTERALIGN");
    get(m_pJustify,"radioBTN_JUSTIFYALIGN");
    get(m_pLeftBottom,"labelST_LEFTALIGN_ASIAN");
    get(m_pRightTop,"labelST_RIGHTALIGN_ASIAN");

    get(m_pLastLineFT,"labelLB_LASTLINE");
    get(m_pLastLineLB,"comboLB_LASTLINE");
    get(m_pExpandCB,"checkCB_EXPAND");
    get(m_pSnapToGridCB,"checkCB_SNAP");
    get(m_pExampleWin,"drawingareaWN_EXAMPLE");

    get(m_pVertAlignLB,"comboLB_VERTALIGN");
    get(m_pVertAlignFL,"frameFL_VERTALIGN");

    get(m_pPropertiesFL,"framePROPERTIES");
    get(m_pTextDirectionLB,"comboLB_TEXTDIRECTION");

    SvtLanguageOptions aLangOptions;
    sal_uInt16 nLastLinePos = LASTLINEPOS_DEFAULT;

    if ( aLangOptions.IsAsianTypographyEnabled() )
    {
        m_pLeft->SetText(m_pLeftBottom->GetText());
        m_pRight->SetText(m_pRightTop->GetText());

        OUString sLeft(m_pLeft->GetText());
        sLeft = MnemonicGenerator::EraseAllMnemonicChars( sLeft );

        if ( m_pLastLineLB->GetEntryCount() == LASTLINECOUNT_OLD )
        {
            m_pLastLineLB->RemoveEntry( 0 );
            m_pLastLineLB->InsertEntry( sLeft, 0 );
        }
        else
            nLastLinePos = LASTLINEPOS_LEFT;
    }

    // remove "Default" or "Left" entry, depends on CJKOptions
    if ( m_pLastLineLB->GetEntryCount() == LASTLINECOUNT_NEW )
        m_pLastLineLB->RemoveEntry( nLastLinePos );

    Link aLink = LINK( this, SvxParaAlignTabPage, AlignHdl_Impl );
    m_pLeft->SetClickHdl( aLink );
    m_pRight->SetClickHdl( aLink );
    m_pCenter->SetClickHdl( aLink );
    m_pJustify->SetClickHdl( aLink );
    m_pLastLineLB->SetSelectHdl( LINK( this, SvxParaAlignTabPage, LastLineHdl_Impl ) );
    m_pTextDirectionLB->SetSelectHdl( LINK( this, SvxParaAlignTabPage, TextDirectionHdl_Impl ) );

    sal_uInt16 nHtmlMode = GetHtmlMode_Impl(rSet);
    if(!(nHtmlMode & HTMLMODE_ON) || (0 != (nHtmlMode & HTMLMODE_SOME_STYLES)) )
    {
        if( aLangOptions.IsCTLFontEnabled() )
        {
            m_pTextDirectionLB->InsertEntryValue( CUI_RESSTR( RID_SVXSTR_FRAMEDIR_LTR ), FRMDIR_HORI_LEFT_TOP );
            m_pTextDirectionLB->InsertEntryValue( CUI_RESSTR( RID_SVXSTR_FRAMEDIR_RTL ), FRMDIR_HORI_RIGHT_TOP );
            m_pTextDirectionLB->InsertEntryValue( CUI_RESSTR( RID_SVXSTR_FRAMEDIR_SUPER ), FRMDIR_ENVIRONMENT );

            m_pPropertiesFL->Show();
        }
    }

    setPreviewsToSamePlace(pParent, this);
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

    if ( m_pLeft->IsChecked() )
    {
        eAdjust = SVX_ADJUST_LEFT;
        bAdj = !m_pLeft->GetSavedValue();
        bChecked = true;
    }
    else if ( m_pRight->IsChecked() )
    {
        eAdjust = SVX_ADJUST_RIGHT;
        bAdj = !m_pRight->GetSavedValue();
        bChecked = true;
    }
    else if ( m_pCenter->IsChecked() )
    {
        eAdjust = SVX_ADJUST_CENTER;
        bAdj = !m_pCenter->GetSavedValue();
        bChecked = true;
    }
    else if ( m_pJustify->IsChecked() )
    {
        eAdjust = SVX_ADJUST_BLOCK;
        bAdj = !m_pJustify->GetSavedValue() ||
            m_pExpandCB->IsChecked() != m_pExpandCB->GetSavedValue() ||
            m_pLastLineLB->GetSelectEntryPos() != m_pLastLineLB->GetSavedValue();
        bChecked = true;
    }
    sal_uInt16 _nWhich = GetWhich( SID_ATTR_PARA_ADJUST );

    if ( bAdj )
    {
        const SvxAdjustItem* pOld = (const SvxAdjustItem*)GetOldItem( rOutSet, SID_ATTR_PARA_ADJUST );

        SvxAdjust eOneWord = m_pExpandCB->IsChecked() ? SVX_ADJUST_BLOCK : SVX_ADJUST_LEFT;

        sal_uInt16 nLBPos = m_pLastLineLB->GetSelectEntryPos();
        SvxAdjust eLastBlock = SVX_ADJUST_LEFT;

        if ( 1 == nLBPos )
            eLastBlock = SVX_ADJUST_CENTER;
        else if ( 2 == nLBPos )
            eLastBlock = SVX_ADJUST_BLOCK;

        bool bNothingWasChecked =
            !m_pLeft->GetSavedValue() && !m_pRight->GetSavedValue() &&
            !m_pCenter->GetSavedValue() && !m_pJustify->GetSavedValue();

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
    if(m_pSnapToGridCB->IsChecked() != m_pSnapToGridCB->GetSavedValue())
    {
        rOutSet.Put(SvxParaGridItem(m_pSnapToGridCB->IsChecked(), GetWhich( SID_ATTR_PARA_SNAPTOGRID )));
        bModified = sal_True;
    }
    if(m_pVertAlignLB->GetSavedValue() != m_pVertAlignLB->GetSelectEntryPos())
    {
        rOutSet.Put(SvxParaVertAlignItem(m_pVertAlignLB->GetSelectEntryPos(), GetWhich( SID_PARA_VERTALIGN )));
        bModified = sal_True;
    }

    if( m_pTextDirectionLB->IsVisible() )
    {
        SvxFrameDirection eDir = m_pTextDirectionLB->GetSelectEntryValue();
        if( eDir != m_pTextDirectionLB->GetSavedValue() )
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
            case SVX_ADJUST_LEFT: m_pLeft->Check(); break;

            case SVX_ADJUST_RIGHT: m_pRight->Check(); break;

            case SVX_ADJUST_CENTER: m_pCenter->Check(); break;

            case SVX_ADJUST_BLOCK: m_pJustify->Check(); break;
            default: ; //prevent warning
        }
        sal_Bool bEnable = m_pJustify->IsChecked();
        m_pLastLineFT->Enable(bEnable);
        m_pLastLineLB->Enable(bEnable);
        m_pExpandCB->Enable(bEnable);

        m_pExpandCB->Check(SVX_ADJUST_BLOCK == rAdj.GetOneWord());
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
        m_pLeft->Check( sal_False );
        m_pRight->Check( sal_False );
        m_pCenter->Check( sal_False );
        m_pJustify->Check( sal_False );
    }
    m_pLastLineLB->SelectEntryPos(nLBSelect);

    sal_uInt16 nHtmlMode = GetHtmlMode_Impl(rSet);
    if(nHtmlMode & HTMLMODE_ON)
    {
        m_pLastLineLB->Hide();
        m_pLastLineFT->Hide();
        m_pExpandCB->Hide();
        if(!(nHtmlMode & HTMLMODE_FULL_STYLES) )
            m_pJustify->Disable();
        m_pSnapToGridCB->Show(sal_False);
    }
    _nWhich = GetWhich(SID_ATTR_PARA_SNAPTOGRID);
    eItemState = rSet.GetItemState( _nWhich );
    if ( eItemState >= SFX_ITEM_AVAILABLE )
    {
        const SvxParaGridItem& rSnap = (const SvxParaGridItem&)rSet.Get( _nWhich );
        m_pSnapToGridCB->Check(rSnap.GetValue());
    }

    _nWhich = GetWhich( SID_PARA_VERTALIGN );
    eItemState = rSet.GetItemState( _nWhich );

    if ( eItemState >= SFX_ITEM_AVAILABLE )
    {
        m_pVertAlignFL->Show();

        const SvxParaVertAlignItem& rAlign = (const SvxParaVertAlignItem&)rSet.Get( _nWhich );

        m_pVertAlignLB->SelectEntryPos(rAlign.GetValue());
    }

    _nWhich = GetWhich( SID_ATTR_FRAMEDIRECTION );
    //text direction
    if( SFX_ITEM_AVAILABLE <= rSet.GetItemState( _nWhich ) )
    {
        const SvxFrameDirectionItem& rFrameDirItem = ( const SvxFrameDirectionItem& ) rSet.Get( _nWhich );
        m_pTextDirectionLB->SelectEntryValue( (SvxFrameDirection)rFrameDirItem.GetValue() );
        m_pTextDirectionLB->SaveValue();
    }

    m_pSnapToGridCB->SaveValue();
    m_pVertAlignLB->SaveValue();
    m_pLeft->SaveValue();
    m_pRight->SaveValue();
    m_pCenter->SaveValue();
    m_pJustify->SaveValue();
    m_pLastLineLB->SaveValue();
    m_pExpandCB->SaveValue();

    UpdateExample_Impl(sal_True);
}

IMPL_LINK_NOARG(SvxParaAlignTabPage, AlignHdl_Impl)
{
    sal_Bool bJustify = m_pJustify->IsChecked();
    m_pLastLineFT->Enable(bJustify);
    m_pLastLineLB->Enable(bJustify);
    m_pExpandCB->Enable(bJustify);
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
    SvxFrameDirection eDir = m_pTextDirectionLB->GetSelectEntryValue();
    switch ( eDir )
    {
        // check the default alignment for this text direction
        case FRMDIR_HORI_LEFT_TOP :     m_pLeft->Check( sal_True ); break;
        case FRMDIR_HORI_RIGHT_TOP :    m_pRight->Check( sal_True ); break;
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
    if ( m_pLeft->IsChecked() )
        m_pExampleWin->SetAdjust( SVX_ADJUST_LEFT );
    else if ( m_pRight->IsChecked() )
        m_pExampleWin->SetAdjust( SVX_ADJUST_RIGHT );
    else if ( m_pCenter->IsChecked() )
        m_pExampleWin->SetAdjust( SVX_ADJUST_CENTER );
    else if ( m_pJustify->IsChecked() )
    {
        m_pExampleWin->SetAdjust( SVX_ADJUST_BLOCK );
        SvxAdjust eLastBlock = SVX_ADJUST_LEFT;
        sal_uInt16 nLBPos = m_pLastLineLB->GetSelectEntryPos();
        if(nLBPos == 1)
            eLastBlock = SVX_ADJUST_CENTER;
        else if(nLBPos == 2)
            eLastBlock = SVX_ADJUST_BLOCK;
        m_pExampleWin->SetLastLine( eLastBlock );
    }

    m_pExampleWin->Draw( bAll );
}

void SvxParaAlignTabPage::EnableJustifyExt()
{
    m_pLastLineFT->Show();
    m_pLastLineLB->Show();
    m_pExpandCB->Show();
    SvtLanguageOptions aCJKOptions;
    if(aCJKOptions.IsAsianTypographyEnabled())
        m_pSnapToGridCB->Show();

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
        SfxStyleSheetIterator iter(pPool,
            SFX_STYLE_FAMILY_PAGE, SFXSTYLEBIT_ALL);
        SfxStyleSheetBase* pStyle = iter.First();
        String aStdName;

        while( pStyle )
        {
            if ( aStdName.Len() == 0 )
                // first style == standard style
                aStdName = pStyle->GetName();
            m_pApplyCollBox->InsertEntry( pStyle->GetName() );
            pStyle = iter.Next();
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
