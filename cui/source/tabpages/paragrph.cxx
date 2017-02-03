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

const sal_uInt16 SvxStdParagraphTabPage::pStdRanges[] =
{
    SID_ATTR_PARA_LINESPACE,        // 10033
    SID_ATTR_PARA_LINESPACE,
    SID_ATTR_LRSPACE,               // 10048 -
    SID_ATTR_ULSPACE,               // 10049
    SID_ATTR_PARA_REGISTER,         // 10413
    SID_ATTR_PARA_REGISTER,
    0
};

const sal_uInt16 SvxParaAlignTabPage::pAlignRanges[] =
{
    SID_ATTR_PARA_ADJUST,           // 10027
    SID_ATTR_PARA_ADJUST,
    0
};

const sal_uInt16 SvxExtParagraphTabPage::pExtRanges[] =
{
    SID_ATTR_PARA_PAGEBREAK,        // 10037 -
    SID_ATTR_PARA_WIDOWS,           // 10041
    SID_ATTR_PARA_MODEL,            // 10065 -
    SID_ATTR_PARA_KEEP,             // 10066
    0
};

#define MAX_DURCH 5670      // 10 cm makes sense as maximum interline lead
                            // according to BP
#define FIX_DIST_DEF 283    // standard fix distance 0,5 cm

enum LineSpaceList
{
    LLINESPACE_1    = 0,
    LLINESPACE_115  = 1,
    LLINESPACE_15   = 2,
    LLINESPACE_2    = 3,
    LLINESPACE_PROP = 4,
    LLINESPACE_MIN  = 5,
    LLINESPACE_DURCH= 6,
    LLINESPACE_FIX  = 7
};

void SetLineSpace_Impl( SvxLineSpacingItem&, int, long lValue = 0 );

void SetLineSpace_Impl( SvxLineSpacingItem& rLineSpace,
                        int eSpace, long lValue )
{
    switch ( eSpace )
    {
        case LLINESPACE_1:
            rLineSpace.SetLineSpaceRule( SvxLineSpaceRule::Auto );
            rLineSpace.SetInterLineSpaceRule( SvxInterLineSpaceRule::Off );
            break;

        case LLINESPACE_115:
            rLineSpace.SetLineSpaceRule( SvxLineSpaceRule::Auto );
            rLineSpace.SetPropLineSpace( 115 );
            break;

        case LLINESPACE_15:
            rLineSpace.SetLineSpaceRule( SvxLineSpaceRule::Auto );
            rLineSpace.SetPropLineSpace( 150 );
            break;

        case LLINESPACE_2:
            rLineSpace.SetLineSpaceRule( SvxLineSpaceRule::Auto );
            rLineSpace.SetPropLineSpace( 200 );
            break;

        case LLINESPACE_PROP:
            rLineSpace.SetLineSpaceRule( SvxLineSpaceRule::Auto );
            rLineSpace.SetPropLineSpace( (sal_uInt8)lValue );
            break;

        case LLINESPACE_MIN:
            rLineSpace.SetLineHeight( (sal_uInt16)lValue );
            rLineSpace.SetInterLineSpaceRule( SvxInterLineSpaceRule::Off );
            break;

        case LLINESPACE_DURCH:
            rLineSpace.SetLineSpaceRule( SvxLineSpaceRule::Auto );
            rLineSpace.SetInterLineSpace( (sal_uInt16)lValue );
            break;

        case LLINESPACE_FIX:
            rLineSpace.SetLineHeight((sal_uInt16)lValue);
            rLineSpace.SetLineSpaceRule( SvxLineSpaceRule::Fix );
            rLineSpace.SetInterLineSpaceRule( SvxInterLineSpaceRule::Off );
        break;
    }
}

sal_uInt16 GetHtmlMode_Impl(const SfxItemSet& rSet)
{
    sal_uInt16 nHtmlMode = 0;
    const SfxPoolItem* pItem = nullptr;
    SfxObjectShell* pShell;
    if(SfxItemState::SET == rSet.GetItemState(SID_HTML_MODE, false, &pItem) ||
        ( nullptr != (pShell = SfxObjectShell::Current()) &&
                    nullptr != (pItem = pShell->GetItem(SID_HTML_MODE))))
    {
        nHtmlMode = static_cast<const SfxUInt16Item*>(pItem)->GetValue();
    }
    return nHtmlMode;

}

IMPL_LINK_NOARG(SvxStdParagraphTabPage, ELRLoseFocusHdl, Control&, void)
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
}

VclPtr<SfxTabPage> SvxStdParagraphTabPage::Create( vcl::Window* pParent, const SfxItemSet* rSet)
{
    return VclPtr<SvxStdParagraphTabPage>::Create( pParent, *rSet );
}

bool SvxStdParagraphTabPage::FillItemSet( SfxItemSet* rOutSet )
{
    SfxItemState eState = SfxItemState::UNKNOWN;
    const SfxPoolItem* pOld = nullptr;
    SfxItemPool* pPool = rOutSet->GetPool();
    DBG_ASSERT( pPool, "Wo ist der Pool" );

    bool bModified = false;
    sal_uInt16 nWhich;
    sal_Int32 nPos = m_pLineDist->GetSelectEntryPos();

    if ( LISTBOX_ENTRY_NOTFOUND != nPos &&
         ( m_pLineDist->IsValueChangedFromSaved() ||
           m_pLineDistAtPercentBox->IsValueModified() ||
           m_pLineDistAtMetricBox->IsValueModified() ) )
    {
        nWhich = GetWhich( SID_ATTR_PARA_LINESPACE );
        MapUnit eUnit = pPool->GetMetric( nWhich );
        SvxLineSpacingItem aSpacing(
            static_cast<const SvxLineSpacingItem&>(GetItemSet().Get( nWhich )) );

        switch ( nPos )
        {
            case LLINESPACE_1:
            case LLINESPACE_115:
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
                OSL_FAIL( "unknown LineDist entry" );
                break;
        }
        eState = GetItemSet().GetItemState( nWhich );
        pOld = GetOldItem( *rOutSet, SID_ATTR_PARA_LINESPACE );

        if ( !pOld || !( *static_cast<const SvxLineSpacingItem*>(pOld) == aSpacing ) ||
             SfxItemState::DONTCARE == eState )
        {
            rOutSet->Put( aSpacing );
            bModified = true;
        }
    }

    if ( m_pTopDist->IsValueModified() || m_pBottomDist->IsValueModified()
         || m_pContextualCB->IsValueChangedFromSaved())
    {
        nWhich = GetWhich( SID_ATTR_ULSPACE );
        MapUnit eUnit = pPool->GetMetric( nWhich );
        pOld = GetOldItem( *rOutSet, SID_ATTR_ULSPACE );
        SvxULSpaceItem aMargin( nWhich );

        if ( bRelativeMode )
        {
            DBG_ASSERT( GetItemSet().GetParent(), "No ParentSet" );

            const SvxULSpaceItem& rOldItem =
                static_cast<const SvxULSpaceItem&>(GetItemSet().GetParent()->Get( nWhich ));

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

        if ( !pOld || !( *static_cast<const SvxULSpaceItem*>(pOld) == aMargin ) ||
             SfxItemState::DONTCARE == eState )
        {
            rOutSet->Put( aMargin );
            bModified = true;
        }
    }
    bool bNullTab = false;

    if ( m_pLeftIndent->IsValueModified() ||
         m_pFLineIndent->IsValueModified() ||
         m_pRightIndent->IsValueModified() ||
         m_pAutoCB->IsValueChangedFromSaved() )
    {
        nWhich = GetWhich( SID_ATTR_LRSPACE );
        MapUnit eUnit = pPool->GetMetric( nWhich );
        SvxLRSpaceItem aMargin( nWhich );
        pOld = GetOldItem( *rOutSet, SID_ATTR_LRSPACE );

        if ( bRelativeMode )
        {
            DBG_ASSERT( GetItemSet().GetParent(), "No ParentSet" );

            const SvxLRSpaceItem& rOldItem =
                static_cast<const SvxLRSpaceItem&>(GetItemSet().GetParent()->Get( nWhich ));

            if ( m_pLeftIndent->IsRelative() )
                aMargin.SetTextLeft( rOldItem.GetTextLeft(),
                                    (sal_uInt16)m_pLeftIndent->GetValue() );
            else
                aMargin.SetTextLeft( GetCoreValue( *m_pLeftIndent, eUnit ) );

            if ( m_pRightIndent->IsRelative() )
                aMargin.SetRight( rOldItem.GetRight(),
                                  (sal_uInt16)m_pRightIndent->GetValue() );
            else
                aMargin.SetRight( GetCoreValue( *m_pRightIndent, eUnit ) );

            if ( m_pFLineIndent->IsRelative() )
                aMargin.SetTextFirstLineOfst( rOldItem.GetTextFirstLineOfst(),
                                             (sal_uInt16)m_pFLineIndent->GetValue() );
            else
                aMargin.SetTextFirstLineOfst(
                    (sal_uInt16)GetCoreValue( *m_pFLineIndent, eUnit ) );
        }
        else
        {
            aMargin.SetTextLeft( GetCoreValue( *m_pLeftIndent, eUnit ) );
            aMargin.SetRight( GetCoreValue( *m_pRightIndent, eUnit ) );
            aMargin.SetTextFirstLineOfst(
                (sal_uInt16)GetCoreValue( *m_pFLineIndent, eUnit ) );
        }
        aMargin.SetAutoFirst(m_pAutoCB->IsChecked());
        if ( aMargin.GetTextFirstLineOfst() < 0 )
            bNullTab = true;
        eState = GetItemSet().GetItemState( nWhich );

        if ( !pOld || !( *static_cast<const SvxLRSpaceItem*>(pOld) == aMargin ) ||
             SfxItemState::DONTCARE == eState )
        {
            rOutSet->Put( aMargin );
            bModified = true;
        }
    }

    if ( bNullTab )
    {
        MapUnit eUnit = (MapUnit)pPool->GetMetric( GetWhich( SID_ATTR_TABSTOP ) );
        if ( MapUnit::Map100thMM != eUnit )
        {

            // negative first line indent -> set null default tabstob if applicable
            sal_uInt16 _nWhich = GetWhich( SID_ATTR_TABSTOP );
            const SfxItemSet& rInSet = GetItemSet();

            if ( rInSet.GetItemState( _nWhich ) >= SfxItemState::DEFAULT )
            {
                const SvxTabStopItem& rTabItem =
                    static_cast<const SvxTabStopItem&>(rInSet.Get( _nWhich ));
                SvxTabStopItem aNullTab( rTabItem );
                SvxTabStop aNull( 0, SvxTabAdjust::Default );
                aNullTab.Insert( aNull );
                rOutSet->Put( aNullTab );
            }
        }
    }
    if( m_pRegisterCB->IsVisible())
    {
        const SfxBoolItem* pBoolItem = static_cast<const SfxBoolItem*>(GetOldItem(
                            *rOutSet, SID_ATTR_PARA_REGISTER));
        if (!pBoolItem)
            return bModified;
        std::unique_ptr<SfxBoolItem> pRegItem(static_cast<SfxBoolItem*>(pBoolItem->Clone()));
        sal_uInt16 _nWhich = GetWhich( SID_ATTR_PARA_REGISTER );
        bool bSet = pRegItem->GetValue();

        if(m_pRegisterCB->IsChecked() != bSet )
        {
            pRegItem->SetValue(!bSet);
            rOutSet->Put(*pRegItem);
            bModified = true;
        }
        else if ( SfxItemState::DEFAULT == GetItemSet().GetItemState( _nWhich, false ) )
            rOutSet->ClearItem(_nWhich);
    }

    return bModified;
}

void SvxStdParagraphTabPage::Reset( const SfxItemSet* rSet )
{
    SfxItemPool* pPool = rSet->GetPool();
    DBG_ASSERT( pPool, "Wo ist der Pool?" );

    // adjust metric
    FieldUnit eFUnit = GetModuleFieldUnit( *rSet );

    bool bApplyCharUnit = GetApplyCharUnit( *rSet );

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

    sal_uInt16 _nWhich = GetWhich( SID_ATTR_LRSPACE );
    SfxItemState eItemState = rSet->GetItemState( _nWhich );

    if ( eItemState >= SfxItemState::DEFAULT )
    {
        MapUnit eUnit = pPool->GetMetric( _nWhich );

        if ( bRelativeMode )
        {
            const SvxLRSpaceItem& rOldItem =
                static_cast<const SvxLRSpaceItem&>(rSet->Get( _nWhich ));

            if ( rOldItem.GetPropLeft() != 100 )
            {
                m_pLeftIndent->SetRelative( true );
                m_pLeftIndent->SetValue( rOldItem.GetPropLeft() );
            }
            else
            {
                m_pLeftIndent->SetRelative(false);
                SetFieldUnit( *m_pLeftIndent, eFUnit );
                SetMetricValue( *m_pLeftIndent, rOldItem.GetTextLeft(), eUnit );
            }

            if ( rOldItem.GetPropRight() != 100 )
            {
                m_pRightIndent->SetRelative( true );
                m_pRightIndent->SetValue( rOldItem.GetPropRight() );
            }
            else
            {
                m_pRightIndent->SetRelative(false);
                SetFieldUnit( *m_pRightIndent, eFUnit );
                SetMetricValue( *m_pRightIndent, rOldItem.GetRight(), eUnit );
            }

            if ( rOldItem.GetPropTextFirstLineOfst() != 100 )
            {
                m_pFLineIndent->SetRelative( true );
                m_pFLineIndent->SetValue( rOldItem.GetPropTextFirstLineOfst() );
            }
            else
            {
                m_pFLineIndent->SetRelative(false);
                m_pFLineIndent->SetMin(-9999);
                SetFieldUnit( *m_pFLineIndent, eFUnit );
                SetMetricValue( *m_pFLineIndent, rOldItem.GetTextFirstLineOfst(),
                                eUnit );
            }
            m_pAutoCB->Check(rOldItem.IsAutoFirst());
        }
        else
        {
            const SvxLRSpaceItem& rSpace =
                static_cast<const SvxLRSpaceItem&>(rSet->Get( _nWhich ));

            SetMetricValue( *m_pLeftIndent, rSpace.GetTextLeft(), eUnit );
            SetMetricValue( *m_pRightIndent, rSpace.GetRight(), eUnit );
            SetMetricValue( *m_pFLineIndent, rSpace.GetTextFirstLineOfst(), eUnit );
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
    eItemState = rSet->GetItemState( _nWhich );

    if ( eItemState >= SfxItemState::DEFAULT )
    {
        MapUnit eUnit = pPool->GetMetric( _nWhich );

        const SvxULSpaceItem& rOldItem =
            static_cast<const SvxULSpaceItem&>(rSet->Get( _nWhich ));
        if ( bRelativeMode )
        {

            if ( rOldItem.GetPropUpper() != 100 )
            {
                m_pTopDist->SetRelative( true );
                m_pTopDist->SetValue( rOldItem.GetPropUpper() );
            }
            else
            {
                m_pTopDist->SetRelative(false);
                if ( eFUnit == FUNIT_CHAR )
                    SetFieldUnit( *m_pTopDist, FUNIT_LINE );
                else
                    SetFieldUnit( *m_pTopDist, eFUnit );
                SetMetricValue( *m_pTopDist, rOldItem.GetUpper(), eUnit );
            }

            if ( rOldItem.GetPropLower() != 100 )
            {
                m_pBottomDist->SetRelative( true );
                m_pBottomDist->SetValue( rOldItem.GetPropLower() );
            }
            else
            {
                m_pBottomDist->SetRelative(false);
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
    eItemState = rSet->GetItemState( _nWhich );

    if ( eItemState >= SfxItemState::DEFAULT )
        SetLineSpacing_Impl( static_cast<const SvxLineSpacingItem &>(rSet->Get( _nWhich )) );
    else
        m_pLineDist->SetNoSelection();

    _nWhich = GetWhich( SID_ATTR_PARA_REGISTER );
    eItemState = rSet->GetItemState( _nWhich );

    if ( eItemState >= SfxItemState::DEFAULT )
        m_pRegisterCB->Check( static_cast<const SfxBoolItem &>(rSet->Get( _nWhich )).GetValue());
    m_pRegisterCB->SaveValue();
    sal_uInt16 nHtmlMode = GetHtmlMode_Impl(*rSet);
    if(nHtmlMode & HTMLMODE_ON)
    {
        m_pRegisterFL->Hide();
        m_pRegisterCB->Hide();
        m_pAutoCB->Hide();
    }

    // this sets the min/max limits; do this _after_ setting the values,
    // because for Impress the min of first-line indent depends on value of
    // left-indent!
    ELRLoseFocusHdl( *m_pFLineIndent );
    m_pAutoCB->SaveValue();
    m_pContextualCB->SaveValue();
    m_pLineDist->SaveValue();
}
void SvxStdParagraphTabPage::ChangesApplied()
{
    m_pLeftIndent->SetValue(m_pLeftIndent->GetValue());
    m_pRightIndent->SetValue(m_pRightIndent->GetValue());
    m_pFLineIndent->SetValue(m_pFLineIndent->GetValue());
    m_pLineDist->SaveValue();
    m_pLineDistAtPercentBox->SaveValue();
    m_pLineDistAtMetricBox->SaveValue();
    m_pRegisterCB->SaveValue();
    m_pTopDist->SetValue(m_pTopDist->GetValue());
    m_pBottomDist->SetValue(m_pBottomDist->GetValue());
    m_pContextualCB->SaveValue();
    m_pAutoCB->SaveValue();
}

void SvxStdParagraphTabPage::EnableRelativeMode()
{
    DBG_ASSERT( GetItemSet().GetParent(), "RelativeMode, but no parent-set!" );

    m_pLeftIndent->EnableRelativeMode( 0, 999, 5 );
    m_pFLineIndent->EnableRelativeMode( 0, 999, 5 );
    m_pRightIndent->EnableRelativeMode( 0, 999, 5 );
    m_pTopDist->EnableRelativeMode( 0, 999, 5 );
    m_pBottomDist->EnableRelativeMode( 0, 999, 5 );
    bRelativeMode = true;
}

DeactivateRC SvxStdParagraphTabPage::DeactivatePage( SfxItemSet* _pSet )
{
    ELRLoseFocusHdl( *m_pFLineIndent );

    if ( _pSet )
        FillItemSet( _pSet );
    return DeactivateRC::LeavePage;
}

SvxStdParagraphTabPage::SvxStdParagraphTabPage( vcl::Window* pParent,  const SfxItemSet& rAttr ) :
    SfxTabPage( pParent, "ParaIndentSpacing","cui/ui/paraindentspacing.ui", &rAttr ),

    nAbst           ( MAX_DURCH ),
    nWidth          ( 11905 /*567 * 50*/ ),
    nMinFixDist(0L),

    bRelativeMode   ( false ),
    bNegativeIndents(false)

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
{
    disposeOnce();
}

void SvxStdParagraphTabPage::dispose()
{
    m_pLeftIndent.clear();
    m_pRightLabel.clear();
    m_pRightIndent.clear();
    m_pFLineLabel.clear();
    m_pFLineIndent.clear();
    m_pAutoCB.clear();
    m_pTopDist.clear();
    m_pBottomDist.clear();
    m_pContextualCB.clear();
    m_pLineDist.clear();
    m_pLineDistAtLabel.clear();
    m_pLineDistAtPercentBox.clear();
    m_pLineDistAtMetricBox.clear();
    m_pAbsDist.clear();
    m_pExampleWin.clear();
    m_pRegisterFL.clear();
    m_pRegisterCB.clear();
    SfxTabPage::dispose();
}

void SvxStdParagraphTabPage::EnableNegativeMode()
{
    m_pLeftIndent->SetMin(-9999);
    m_pRightIndent->SetMin(-9999);
    m_pRightIndent->EnableNegativeMode();
    m_pLeftIndent->EnableNegativeMode();
    bNegativeIndents = true;
}

void SvxStdParagraphTabPage::SetLineSpacing_Impl
(
    const SvxLineSpacingItem &rAttr
)
{
    MapUnit eUnit = GetItemSet().GetPool()->GetMetric( rAttr.Which() );

    switch( rAttr.GetLineSpaceRule() )
    {
        case SvxLineSpaceRule::Auto:
        {
            SvxInterLineSpaceRule eInter = rAttr.GetInterLineSpaceRule();

            switch( eInter )
            {
                // Default single line spacing
                case SvxInterLineSpaceRule::Off:
                    m_pLineDist->SelectEntryPos( LLINESPACE_1 );
                    break;

                // Default single line spacing
                case SvxInterLineSpaceRule::Prop:
                    if ( 100 == rAttr.GetPropLineSpace() )
                    {
                        m_pLineDist->SelectEntryPos( LLINESPACE_1 );
                        break;
                    }
                    // 1.15 line spacing
                    if ( 115 == rAttr.GetPropLineSpace() )
                    {
                        m_pLineDist->SelectEntryPos( LLINESPACE_115 );
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

                case SvxInterLineSpaceRule::Fix:
                    SetMetricValue( *m_pLineDistAtMetricBox, rAttr.GetInterLineSpace(), eUnit );
                    m_pLineDist->SelectEntryPos( LLINESPACE_DURCH );
                    break;
                default: ;//prevent warning
            }
        }
        break;
        case SvxLineSpaceRule::Fix:
            SetMetricValue(*m_pLineDistAtMetricBox, rAttr.GetLineHeight(), eUnit);
            m_pLineDist->SelectEntryPos( LLINESPACE_FIX );
        break;

        case SvxLineSpaceRule::Min:
            SetMetricValue(*m_pLineDistAtMetricBox, rAttr.GetLineHeight(), eUnit);
            m_pLineDist->SelectEntryPos( LLINESPACE_MIN );
            break;
        default: ;//prevent warning
    }
    LineDistHdl_Impl( *m_pLineDist );
}

IMPL_LINK( SvxStdParagraphTabPage, LineDistHdl_Impl, ListBox&, rBox, void )
{
    switch( rBox.GetSelectEntryPos() )
    {
        case LLINESPACE_1:
        case LLINESPACE_115:
        case LLINESPACE_15:
        case LLINESPACE_2:
            m_pLineDistAtLabel->Enable(false);
            m_pLineDistAtPercentBox->Enable(false);
            m_pLineDistAtPercentBox->SetText( OUString() );
            m_pLineDistAtMetricBox->Enable(false);
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
                SetMetricValue( *m_pLineDistAtMetricBox, FIX_DIST_DEF, MapUnit::MapTwip ); // fix is only in Writer
            m_pLineDistAtPercentBox->Hide();
            m_pLineDistAtMetricBox->Show();
            m_pLineDistAtMetricBox->Enable();
            m_pLineDistAtLabel->Enable();
        }
        break;
    }
    UpdateExample_Impl();
}

IMPL_LINK_NOARG(SvxStdParagraphTabPage, ModifyHdl_Impl, Edit&, void)
{
    UpdateExample_Impl();
}

void SvxStdParagraphTabPage::Init_Impl()
{
    m_pLineDist->SetSelectHdl(
        LINK( this, SvxStdParagraphTabPage, LineDistHdl_Impl ) );

    Link<Control&,void> aLink2 = LINK( this, SvxStdParagraphTabPage, ELRLoseFocusHdl );
    m_pFLineIndent->SetLoseFocusHdl( aLink2 );
    m_pLeftIndent->SetLoseFocusHdl( aLink2 );
    m_pRightIndent->SetLoseFocusHdl( aLink2 );

    Link<Edit&,void> aLink = LINK( this, SvxStdParagraphTabPage, ModifyHdl_Impl );
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

void SvxStdParagraphTabPage::UpdateExample_Impl()
{
    m_pExampleWin->SetFirstLineOfst( (short)m_pFLineIndent->Denormalize( m_pFLineIndent->GetValue( FUNIT_TWIP ) ) );
    m_pExampleWin->SetLeftMargin( static_cast<long>(m_pLeftIndent->Denormalize( m_pLeftIndent->GetValue( FUNIT_TWIP ) ) ) );
    m_pExampleWin->SetRightMargin( static_cast<long>(m_pRightIndent->Denormalize( m_pRightIndent->GetValue( FUNIT_TWIP ) ) ) );
    m_pExampleWin->SetUpper( (sal_uInt16)m_pTopDist->Denormalize( m_pTopDist->GetValue( FUNIT_TWIP ) ) );
    m_pExampleWin->SetLower( (sal_uInt16)m_pBottomDist->Denormalize( m_pBottomDist->GetValue( FUNIT_TWIP ) ) );

    sal_Int32 nPos = m_pLineDist->GetSelectEntryPos();

    switch ( nPos )
    {
        case LLINESPACE_1:
        case LLINESPACE_115:
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
                (sal_uInt16)GetCoreValue( *m_pLineDistAtMetricBox, MapUnit::MapTwip ) );
            break;
    }
    m_pExampleWin->Invalidate();
}

void SvxStdParagraphTabPage::EnableRegisterMode()
{
    m_pRegisterCB->Show();
    m_pRegisterFL->Show();
}

void SvxStdParagraphTabPage::EnableContextualMode()
{
    m_pContextualCB->Show();
}

IMPL_LINK( SvxStdParagraphTabPage, AutoHdl_Impl, Button*, pBox, void )
{
    bool bEnable = !static_cast<CheckBox*>(pBox)->IsChecked();
    m_pFLineLabel->Enable(bEnable);
    m_pFLineIndent->Enable(bEnable);
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

void    SvxStdParagraphTabPage::PageCreated(const SfxAllItemSet& aSet)
{

/* different bit represent call to different method of SvxStdParagraphTabPage
                        0x0001 --->EnableRelativeMode()
                        0x0002 --->EnableRegisterMode()
                        0x0004 --->EnableAutoFirstLine()
                        0x0008 --->EnableNegativeMode()
                        0x0010 --->EnableContextualMode()
            */
    const SfxUInt16Item* pPageWidthItem = aSet.GetItem<SfxUInt16Item>(SID_SVXSTDPARAGRAPHTABPAGE_PAGEWIDTH, false);
    const SfxUInt32Item* pFlagSetItem = aSet.GetItem<SfxUInt32Item>(SID_SVXSTDPARAGRAPHTABPAGE_FLAGSET, false);
    const SfxUInt32Item* pLineDistItem = aSet.GetItem<SfxUInt32Item>(SID_SVXSTDPARAGRAPHTABPAGE_ABSLINEDIST, false);

    if (pPageWidthItem)
        nWidth = pPageWidthItem->GetValue();

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

SvxParaAlignTabPage::SvxParaAlignTabPage( vcl::Window* pParent, const SfxItemSet& rSet )

    : SfxTabPage(pParent, "ParaAlignPage", "cui/ui/paragalignpage.ui",&rSet)
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

    Link<Button*,void> aLink = LINK( this, SvxParaAlignTabPage, AlignHdl_Impl );
    m_pLeft->SetClickHdl( aLink );
    m_pRight->SetClickHdl( aLink );
    m_pCenter->SetClickHdl( aLink );
    m_pJustify->SetClickHdl( aLink );
    m_pLastLineLB->SetSelectHdl( LINK( this, SvxParaAlignTabPage, LastLineHdl_Impl ) );
    m_pTextDirectionLB->SetSelectHdl( LINK( this, SvxParaAlignTabPage, TextDirectionHdl_Impl ) );

    m_pTextDirectionLB->InsertEntryValue( CUI_RESSTR( RID_SVXSTR_FRAMEDIR_SUPER ), FRMDIR_ENVIRONMENT );
    m_pTextDirectionLB->InsertEntryValue( CUI_RESSTR( RID_SVXSTR_FRAMEDIR_LTR ), FRMDIR_HORI_LEFT_TOP );
    m_pTextDirectionLB->InsertEntryValue( CUI_RESSTR( RID_SVXSTR_FRAMEDIR_RTL ), FRMDIR_HORI_RIGHT_TOP );

    setPreviewsToSamePlace(pParent, this);
}

SvxParaAlignTabPage::~SvxParaAlignTabPage()
{
    disposeOnce();
}

void SvxParaAlignTabPage::dispose()
{
    m_pLeft.clear();
    m_pRight.clear();
    m_pCenter.clear();
    m_pJustify.clear();
    m_pLeftBottom.clear();
    m_pRightTop.clear();
    m_pLastLineFT.clear();
    m_pLastLineLB.clear();
    m_pExpandCB.clear();
    m_pSnapToGridCB.clear();
    m_pExampleWin.clear();
    m_pVertAlignFL.clear();
    m_pVertAlignLB.clear();
    m_pPropertiesFL.clear();
    m_pTextDirectionLB.clear();
    SfxTabPage::dispose();
}

DeactivateRC SvxParaAlignTabPage::DeactivatePage( SfxItemSet* _pSet )
{
    if ( _pSet )
        FillItemSet( _pSet );
    return DeactivateRC::LeavePage;
}

VclPtr<SfxTabPage> SvxParaAlignTabPage::Create( vcl::Window* pParent, const SfxItemSet* rSet )
{
    return VclPtr<SvxParaAlignTabPage>::Create(pParent, *rSet);
}

bool SvxParaAlignTabPage::FillItemSet( SfxItemSet* rOutSet )
{
    bool bModified = false;

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
            m_pExpandCB->IsValueChangedFromSaved() ||
            m_pLastLineLB->IsValueChangedFromSaved();
        bChecked = true;
    }
    sal_uInt16 _nWhich = GetWhich( SID_ATTR_PARA_ADJUST );

    if ( bAdj )
    {
        const SvxAdjustItem* pOld = static_cast<const SvxAdjustItem*>(GetOldItem( *rOutSet, SID_ATTR_PARA_ADJUST ));

        SvxAdjust eOneWord = m_pExpandCB->IsChecked() ? SVX_ADJUST_BLOCK : SVX_ADJUST_LEFT;

        sal_Int32 nLBPos = m_pLastLineLB->GetSelectEntryPos();
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
            bModified = true;
            SvxAdjustItem aAdj(
                static_cast<const SvxAdjustItem&>(GetItemSet().Get( _nWhich )) );
            aAdj.SetAdjust( eAdjust );
            aAdj.SetOneWord( eOneWord );
            aAdj.SetLastBlock( eLastBlock );
            rOutSet->Put( aAdj );
        }
    }
    if(m_pSnapToGridCB->IsValueChangedFromSaved())
    {
        rOutSet->Put(SvxParaGridItem(m_pSnapToGridCB->IsChecked(), GetWhich( SID_ATTR_PARA_SNAPTOGRID )));
        bModified = true;
    }
    if(m_pVertAlignLB->IsValueChangedFromSaved())
    {
        rOutSet->Put(SvxParaVertAlignItem((SvxParaVertAlignItem::Align)m_pVertAlignLB->GetSelectEntryPos(), GetWhich( SID_PARA_VERTALIGN )));
        bModified = true;
    }

    if( m_pTextDirectionLB->IsVisible() )
    {
        SvxFrameDirection eDir = m_pTextDirectionLB->GetSelectEntryValue();
        if( m_pTextDirectionLB->IsValueChangedFromSaved() )
        {
            rOutSet->Put( SvxFrameDirectionItem( eDir, GetWhich( SID_ATTR_FRAMEDIRECTION ) ) );
            bModified = true;
        }
    }

    return bModified;
}
void SvxParaAlignTabPage::Reset( const SfxItemSet* rSet )
{
    sal_uInt16 _nWhich = GetWhich( SID_ATTR_PARA_ADJUST );
    SfxItemState eItemState = rSet->GetItemState( _nWhich );

    sal_Int32 nLBSelect = 0;
    if ( eItemState >= SfxItemState::DEFAULT )
    {
        const SvxAdjustItem& rAdj = static_cast<const SvxAdjustItem&>(rSet->Get( _nWhich ));

        switch ( rAdj.GetAdjust() /*!!! ask VB rAdj.GetLastBlock()*/ )
        {
            case SVX_ADJUST_LEFT: m_pLeft->Check(); break;

            case SVX_ADJUST_RIGHT: m_pRight->Check(); break;

            case SVX_ADJUST_CENTER: m_pCenter->Check(); break;

            case SVX_ADJUST_BLOCK: m_pJustify->Check(); break;
            default: ; //prevent warning
        }
        bool bEnable = m_pJustify->IsChecked();
        m_pLastLineFT->Enable(bEnable);
        m_pLastLineLB->Enable(bEnable);

        switch(rAdj.GetLastBlock())
        {
            case SVX_ADJUST_LEFT:  nLBSelect = 0; break;

            case SVX_ADJUST_CENTER: nLBSelect = 1;  break;

            case SVX_ADJUST_BLOCK: nLBSelect = 2;  break;
            default: ; //prevent warning
        }
        m_pExpandCB->Enable(bEnable && nLBSelect == 2);
        m_pExpandCB->Check(SVX_ADJUST_BLOCK == rAdj.GetOneWord());
    }
    else
    {
        m_pLeft->Check( false );
        m_pRight->Check( false );
        m_pCenter->Check( false );
        m_pJustify->Check( false );
    }
    m_pLastLineLB->SelectEntryPos(nLBSelect);

    sal_uInt16 nHtmlMode = GetHtmlMode_Impl(*rSet);
    if(nHtmlMode & HTMLMODE_ON)
    {
        m_pLastLineLB->Hide();
        m_pLastLineFT->Hide();
        m_pExpandCB->Hide();
        if(!(nHtmlMode & HTMLMODE_FULL_STYLES) )
            m_pJustify->Disable();
        m_pSnapToGridCB->Show(false);
    }
    _nWhich = GetWhich(SID_ATTR_PARA_SNAPTOGRID);
    eItemState = rSet->GetItemState( _nWhich );
    if ( eItemState >= SfxItemState::DEFAULT )
    {
        const SvxParaGridItem& rSnap = static_cast<const SvxParaGridItem&>(rSet->Get( _nWhich ));
        m_pSnapToGridCB->Check(rSnap.GetValue());
    }

    _nWhich = GetWhich( SID_PARA_VERTALIGN );
    eItemState = rSet->GetItemState( _nWhich );

    if ( eItemState >= SfxItemState::DEFAULT )
    {
        m_pVertAlignFL->Show();

        const SvxParaVertAlignItem& rAlign = static_cast<const SvxParaVertAlignItem&>(rSet->Get( _nWhich ));

        m_pVertAlignLB->SelectEntryPos((sal_Int32)rAlign.GetValue());
    }

    _nWhich = GetWhich( SID_ATTR_FRAMEDIRECTION );
    //text direction
    if( SfxItemState::DEFAULT <= rSet->GetItemState( _nWhich ) )
    {
        const SvxFrameDirectionItem& rFrameDirItem = static_cast<const SvxFrameDirectionItem&>( rSet->Get( _nWhich ) );
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

    UpdateExample_Impl();
}
void SvxParaAlignTabPage::ChangesApplied()
{
    m_pTextDirectionLB->SaveValue();
    m_pSnapToGridCB->SaveValue();
    m_pVertAlignLB->SaveValue();
    m_pLeft->SaveValue();
    m_pRight->SaveValue();
    m_pCenter->SaveValue();
    m_pJustify->SaveValue();
    m_pLastLineLB->SaveValue();
    m_pExpandCB->SaveValue();
}

IMPL_LINK_NOARG(SvxParaAlignTabPage, AlignHdl_Impl, Button*, void)
{
    bool bJustify = m_pJustify->IsChecked();
    m_pLastLineFT->Enable(bJustify);
    m_pLastLineLB->Enable(bJustify);
    bool bLastLineIsBlock = m_pLastLineLB->GetSelectEntryPos() == 2;
    m_pExpandCB->Enable(bJustify && bLastLineIsBlock);
    UpdateExample_Impl();
}

IMPL_LINK_NOARG(SvxParaAlignTabPage, LastLineHdl_Impl, ListBox&, void)
{
    //fdo#41350 only enable 'Expand last word' if last line is also justified
    bool bLastLineIsBlock = m_pLastLineLB->GetSelectEntryPos() == 2;
    m_pExpandCB->Enable(bLastLineIsBlock);
    UpdateExample_Impl();
}

IMPL_LINK_NOARG(SvxParaAlignTabPage, TextDirectionHdl_Impl, ListBox&, void)
{
    SvxFrameDirection eDir = m_pTextDirectionLB->GetSelectEntryValue();
    switch ( eDir )
    {
        // check the default alignment for this text direction
        case FRMDIR_HORI_LEFT_TOP :     m_pLeft->Check(); break;
        case FRMDIR_HORI_RIGHT_TOP :    m_pRight->Check(); break;
        case FRMDIR_ENVIRONMENT :       /* do nothing */ break;
        default:
        {
            SAL_WARN( "cui.tabpages", "SvxParaAlignTabPage::TextDirectionHdl_Impl(): other directions not supported" );
        }
    }
}

void SvxParaAlignTabPage::UpdateExample_Impl()
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
        sal_Int32 nLBPos = m_pLastLineLB->GetSelectEntryPos();
        if(nLBPos == 1)
            eLastBlock = SVX_ADJUST_CENTER;
        else if(nLBPos == 2)
            eLastBlock = SVX_ADJUST_BLOCK;
        m_pExampleWin->SetLastLine( eLastBlock );
    }

    m_pExampleWin->Invalidate();
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

void SvxParaAlignTabPage::PageCreated (const SfxAllItemSet& aSet)
{
    const SfxBoolItem* pBoolItem = aSet.GetItem<SfxBoolItem>(SID_SVXPARAALIGNTABPAGE_ENABLEJUSTIFYEXT, false);
    if (pBoolItem)
        if(pBoolItem->GetValue())
            EnableJustifyExt();
}

VclPtr<SfxTabPage> SvxExtParagraphTabPage::Create( vcl::Window* pParent,
                                            const SfxItemSet* rSet )
{
    return VclPtr<SvxExtParagraphTabPage>::Create( pParent, *rSet );
}

bool SvxExtParagraphTabPage::FillItemSet( SfxItemSet* rOutSet )
{
    bool bModified = false;
    sal_uInt16 _nWhich = GetWhich( SID_ATTR_PARA_HYPHENZONE );
    const TriState eHyphenState = m_pHyphenBox->GetState();
    const SfxPoolItem* pOld = GetOldItem( *rOutSet, SID_ATTR_PARA_HYPHENZONE );

    if ( m_pHyphenBox->IsValueChangedFromSaved()     ||
         m_pExtHyphenBeforeBox->IsValueModified()          ||
         m_pExtHyphenAfterBox->IsValueModified()           ||
         m_pMaxHyphenEdit->IsValueModified() )
    {
        SvxHyphenZoneItem aHyphen(
            static_cast<const SvxHyphenZoneItem&>(GetItemSet().Get( _nWhich )) );
        aHyphen.SetHyphen( eHyphenState == TRISTATE_TRUE );

        if ( eHyphenState == TRISTATE_TRUE )
        {
            aHyphen.GetMinLead() = (sal_uInt8)m_pExtHyphenBeforeBox->GetValue();
            aHyphen.GetMinTrail() = (sal_uInt8)m_pExtHyphenAfterBox->GetValue();
        }
        aHyphen.GetMaxHyphens() = (sal_uInt8)m_pMaxHyphenEdit->GetValue();

        if ( !pOld ||
            !( *static_cast<const SvxHyphenZoneItem*>(pOld) == aHyphen ) ||
                m_pHyphenBox->IsValueChangedFromSaved())
        {
            rOutSet->Put( aHyphen );
            bModified = true;
        }
    }

    if (m_pPagenumEdit->IsEnabled() && m_pPagenumEdit->IsValueModified())
    {
        SfxUInt16Item aPageNum( SID_ATTR_PARA_PAGENUM,
                                (sal_uInt16)m_pPagenumEdit->GetValue() );

        pOld = GetOldItem( *rOutSet, SID_ATTR_PARA_PAGENUM );

        if ( !pOld || static_cast<const SfxUInt16Item*>(pOld)->GetValue() != aPageNum.GetValue() )
        {
            rOutSet->Put( aPageNum );
            bModified = true;
        }
    }

    // pagebreak

    TriState eState = m_pApplyCollBtn->GetState();
    bool bIsPageModel = false;

    _nWhich = GetWhich( SID_ATTR_PARA_MODEL );
    OUString sPage;
    if ( m_pApplyCollBtn->IsValueChangedFromSaved() ||
         ( TRISTATE_TRUE == eState &&
           m_pApplyCollBox->IsValueChangedFromSaved() ) )
    {
        if ( eState == TRISTATE_TRUE )
        {
            sPage = m_pApplyCollBox->GetSelectEntry();
            bIsPageModel = !sPage.isEmpty();
        }
        pOld = GetOldItem( *rOutSet, SID_ATTR_PARA_MODEL );

        if ( !pOld || static_cast<const SvxPageModelItem*>(pOld)->GetValue() != sPage )
        {
            rOutSet->Put( SvxPageModelItem( sPage, false, _nWhich ) );
            bModified = true;
        }
        else
            bIsPageModel = false;
    }
    else if(TRISTATE_TRUE == eState && m_pApplyCollBtn->IsEnabled())
        bIsPageModel = true;
    else
        rOutSet->Put( SvxPageModelItem( sPage, false, _nWhich ) );

    _nWhich = GetWhich( SID_ATTR_PARA_PAGEBREAK );

    if ( bIsPageModel )
        // if PageModel is turned on, always turn off PageBreak
        rOutSet->Put( SvxFormatBreakItem( SvxBreak::NONE, _nWhich ) );
    else
    {
        eState = m_pPageBreakBox->GetState();
        SfxItemState eModelState = GetItemSet().GetItemState(SID_ATTR_PARA_MODEL, false);

        if ( (eModelState == SfxItemState::SET && TRISTATE_TRUE == m_pPageBreakBox->GetState()) ||
             m_pPageBreakBox->IsValueChangedFromSaved()                ||
             m_pBreakTypeLB->IsValueChangedFromSaved()   ||
             m_pBreakPositionLB->IsValueChangedFromSaved() )
        {
            const SvxFormatBreakItem rOldBreak(
                    static_cast<const SvxFormatBreakItem&>(GetItemSet().Get( _nWhich )));
            SvxFormatBreakItem aBreak(rOldBreak.GetBreak(), rOldBreak.Which());

            switch ( eState )
            {
                case TRISTATE_TRUE:
                {
                    bool bBefore = m_pBreakPositionLB->GetSelectEntryPos() == 0;

                    if ( m_pBreakTypeLB->GetSelectEntryPos() == 0 )
                    {
                        if ( bBefore )
                            aBreak.SetValue( SvxBreak::PageBefore );
                        else
                            aBreak.SetValue( SvxBreak::PageAfter );
                    }
                    else
                    {
                        if ( bBefore )
                            aBreak.SetValue( SvxBreak::ColumnBefore );
                        else
                            aBreak.SetValue( SvxBreak::ColumnAfter );
                    }
                    break;
                }

                case TRISTATE_FALSE:
                    aBreak.SetValue( SvxBreak::NONE );
                    break;
                default: ; //prevent warning
            }
            pOld = GetOldItem( *rOutSet, SID_ATTR_PARA_PAGEBREAK );

            if ( eState != m_pPageBreakBox->GetSavedValue()                ||
                    !pOld || !( *static_cast<const SvxFormatBreakItem*>(pOld) == aBreak ) )
            {
                bModified = true;
                rOutSet->Put( aBreak );
            }
        }
    }

    // paragraph split
    _nWhich = GetWhich( SID_ATTR_PARA_SPLIT );
    eState = m_pKeepTogetherBox->GetState();

    if ( m_pKeepTogetherBox->IsValueChangedFromSaved() )
    {
        pOld = GetOldItem( *rOutSet, SID_ATTR_PARA_SPLIT );

        if ( !pOld || static_cast<const SvxFormatSplitItem*>(pOld)->GetValue() !=
                      ( eState == TRISTATE_FALSE ) )
        {
            rOutSet->Put( SvxFormatSplitItem( eState == TRISTATE_FALSE, _nWhich ) );
            bModified = true;
        }
    }

    // keep paragraphs
    _nWhich = GetWhich( SID_ATTR_PARA_KEEP );
    eState = m_pKeepParaBox->GetState();

    if ( m_pKeepParaBox->IsValueChangedFromSaved() )
    {
        // if the status has changed, putting is necessary
        rOutSet->Put( SvxFormatKeepItem( eState == TRISTATE_TRUE, _nWhich ) );
        bModified = true;
    }

    // widows and orphans
    _nWhich = GetWhich( SID_ATTR_PARA_WIDOWS );
    eState = m_pWidowBox->GetState();

    if ( m_pWidowBox->IsValueChangedFromSaved() ||
         m_pWidowRowNo->IsValueModified() )
    {
        SvxWidowsItem rItem( eState == TRISTATE_TRUE ?
                             (sal_uInt8)m_pWidowRowNo->GetValue() : 0, _nWhich );
        pOld = GetOldItem( *rOutSet, SID_ATTR_PARA_WIDOWS );

        if ( m_pWidowBox->IsValueChangedFromSaved() || !pOld || !( *static_cast<const SvxWidowsItem*>(pOld) == rItem ) )
        {
            rOutSet->Put( rItem );
            bModified = true;
        }
    }

    _nWhich = GetWhich( SID_ATTR_PARA_ORPHANS );
    eState = m_pOrphanBox->GetState();

    if ( m_pOrphanBox->IsValueChangedFromSaved() ||
         m_pOrphanRowNo->IsValueModified() )
    {
        SvxOrphansItem rItem( eState == TRISTATE_TRUE ?
                             (sal_uInt8)m_pOrphanRowNo->GetValue() : 0, _nWhich );
        pOld = GetOldItem( *rOutSet, SID_ATTR_PARA_ORPHANS );

        if ( m_pOrphanBox->IsValueChangedFromSaved() ||
                !pOld ||
                    !( *static_cast<const SvxOrphansItem*>(pOld) == rItem ) )
        {
            rOutSet->Put( rItem );
            bModified = true;
        }
    }

    return bModified;
}
void SvxExtParagraphTabPage::Reset( const SfxItemSet* rSet )
{
    sal_uInt16 _nWhich = GetWhich( SID_ATTR_PARA_HYPHENZONE );
    SfxItemState eItemState = rSet->GetItemState( _nWhich );

    bool bItemAvailable = eItemState >= SfxItemState::DEFAULT;
    bool bIsHyphen = false;
    if( !bHtmlMode && bItemAvailable )
    {
        const SvxHyphenZoneItem& rHyphen =
            static_cast<const SvxHyphenZoneItem&>(rSet->Get( _nWhich ));
        m_pHyphenBox->EnableTriState( false );

        bIsHyphen = rHyphen.IsHyphen();
        m_pHyphenBox->SetState( bIsHyphen ? TRISTATE_TRUE : TRISTATE_FALSE );

        m_pExtHyphenBeforeBox->SetValue( rHyphen.GetMinLead() );
        m_pExtHyphenAfterBox->SetValue( rHyphen.GetMinTrail() );
        m_pMaxHyphenEdit->SetValue( rHyphen.GetMaxHyphens() );
    }
    else
    {
        m_pHyphenBox->SetState( TRISTATE_INDET );
    }
    bool bEnable = bItemAvailable && bIsHyphen;
    m_pExtHyphenBeforeBox->Enable(bEnable);
    m_pExtHyphenAfterBox->Enable(bEnable);
    m_pBeforeText->Enable(bEnable);
    m_pAfterText->Enable(bEnable);
    m_pMaxHyphenLabel->Enable(bEnable);
    m_pMaxHyphenEdit->Enable(bEnable);

    _nWhich = GetWhich( SID_ATTR_PARA_PAGENUM );

    if (rSet->GetItemState(_nWhich) >= SfxItemState::SET)
    {
        const sal_uInt16 nPageNum =
            static_cast<const SfxUInt16Item&>(rSet->Get( _nWhich ) ).GetValue();
        m_pPagenumEdit->SetValue( nPageNum );
    }

    if ( bPageBreak )
    {
        // first handle PageModel
        _nWhich = GetWhich( SID_ATTR_PARA_MODEL );
        bool bIsPageModel = false;
        eItemState = rSet->GetItemState( _nWhich );

        if ( eItemState >= SfxItemState::SET )
        {
            m_pApplyCollBtn->EnableTriState( false );

            const SvxPageModelItem& rModel =
                static_cast<const SvxPageModelItem&>(rSet->Get( _nWhich ));
            const OUString& aStr( rModel.GetValue() );

            if ( !aStr.isEmpty() &&
                 m_pApplyCollBox->GetEntryPos( aStr ) != LISTBOX_ENTRY_NOTFOUND )
            {
                m_pApplyCollBox->SelectEntry( aStr );
                m_pApplyCollBtn->SetState( TRISTATE_TRUE );
                bIsPageModel = true;

                m_pPageBreakBox->Enable();
                m_pPageBreakBox->EnableTriState( false );
                m_pBreakTypeFT->Enable();
                m_pBreakTypeLB->Enable();
                m_pBreakPositionFT->Enable();
                m_pBreakPositionLB->Enable();
                m_pApplyCollBtn->Enable();
                m_pPageBreakBox->SetState( TRISTATE_TRUE );

                //select page break
                m_pBreakTypeLB->SelectEntryPos(0);
                //select break before
                m_pBreakPositionLB->SelectEntryPos(0);
            }
            else
            {
                m_pApplyCollBox->SetNoSelection();
                m_pApplyCollBtn->SetState( TRISTATE_FALSE );
            }
        }
        else if ( SfxItemState::DONTCARE == eItemState )
        {
            m_pApplyCollBtn->EnableTriState();
            m_pApplyCollBtn->SetState( TRISTATE_INDET );
            m_pApplyCollBox->SetNoSelection();
        }
        else
        {
            m_pApplyCollBtn->Enable(false);
            m_pApplyCollBox->Enable(false);
            m_pPagenumEdit->Enable(false);
            m_pPagenumText->Enable(false);
        }

        if ( !bIsPageModel )
        {
            _nWhich = GetWhich( SID_ATTR_PARA_PAGEBREAK );
            eItemState = rSet->GetItemState( _nWhich );

            if ( eItemState >= SfxItemState::DEFAULT )
            {
                const SvxFormatBreakItem& rPageBreak =
                    static_cast<const SvxFormatBreakItem&>(rSet->Get( _nWhich ));

                SvxBreak eBreak = rPageBreak.GetBreak();

                // PageBreak not via CTRL-RETURN,
                // then CheckBox can be freed
                m_pPageBreakBox->Enable();
                m_pPageBreakBox->EnableTriState( false );
                m_pBreakTypeFT->Enable();
                m_pBreakTypeLB->Enable();
                m_pBreakPositionFT->Enable();
                m_pBreakPositionLB->Enable();

                m_pPageBreakBox->SetState( TRISTATE_TRUE );

                bool _bEnable =     eBreak != SvxBreak::NONE &&
                                eBreak != SvxBreak::ColumnBefore &&
                                eBreak != SvxBreak::ColumnAfter;
                m_pApplyCollBtn->Enable(_bEnable);
                if(!_bEnable)
                {
                    m_pApplyCollBox->Enable(_bEnable);
                    m_pPagenumEdit->Enable(_bEnable);
                }

                if ( eBreak == SvxBreak::NONE )
                    m_pPageBreakBox->SetState( TRISTATE_FALSE );

                sal_Int32 nType = 0; // selection position in break type ListBox : Page
                sal_Int32 nPosition = 0; //  selection position in break position ListBox : Before
                switch ( eBreak )
                {
                    case SvxBreak::PageBefore:
                        break;
                    case SvxBreak::PageAfter:
                        nPosition = 1;
                        break;
                    case SvxBreak::ColumnBefore:
                        nType = 1;
                        break;
                    case SvxBreak::ColumnAfter:
                        nType = 1;
                        nPosition = 1;
                        break;
                    default: ;//prevent warning
                }
                m_pBreakTypeLB->SelectEntryPos(nType);
                m_pBreakPositionLB->SelectEntryPos(nPosition);
            }
            else if ( SfxItemState::DONTCARE == eItemState )
                m_pPageBreakBox->SetState( TRISTATE_INDET );
            else
            {
                m_pPageBreakBox->Enable(false);
                m_pBreakTypeFT->Enable(false);
                m_pBreakTypeLB->Enable(false);
                m_pBreakPositionFT->Enable(false);
                m_pBreakPositionLB->Enable(false);
            }
        }

        PageBreakPosHdl_Impl( *m_pBreakPositionLB );
        PageBreakHdl_Impl( m_pPageBreakBox );
    }

    _nWhich = GetWhich( SID_ATTR_PARA_KEEP );
    eItemState = rSet->GetItemState( _nWhich );

    if ( eItemState >= SfxItemState::DEFAULT )
    {
        m_pKeepParaBox->EnableTriState( false );
        const SvxFormatKeepItem& rKeep =
            static_cast<const SvxFormatKeepItem&>(rSet->Get( _nWhich ));

        if ( rKeep.GetValue() )
            m_pKeepParaBox->SetState( TRISTATE_TRUE );
        else
            m_pKeepParaBox->SetState( TRISTATE_FALSE );
    }
    else if ( SfxItemState::DONTCARE == eItemState )
        m_pKeepParaBox->SetState( TRISTATE_INDET );
    else
        m_pKeepParaBox->Enable(false);

    _nWhich = GetWhich( SID_ATTR_PARA_SPLIT );
    eItemState = rSet->GetItemState( _nWhich );

    if ( eItemState >= SfxItemState::DEFAULT )
    {
        const SvxFormatSplitItem& rSplit =
            static_cast<const SvxFormatSplitItem&>(rSet->Get( _nWhich ));
        m_pKeepTogetherBox->EnableTriState( false );

        if ( !rSplit.GetValue() )
            m_pKeepTogetherBox->SetState( TRISTATE_TRUE );
        else
        {
            m_pKeepTogetherBox->SetState( TRISTATE_FALSE );

            // widows and orphans
            m_pWidowBox->Enable();
            _nWhich = GetWhich( SID_ATTR_PARA_WIDOWS );
            SfxItemState eTmpState = rSet->GetItemState( _nWhich );

            if ( eTmpState >= SfxItemState::DEFAULT )
            {
                const SvxWidowsItem& rWidow =
                    static_cast<const SvxWidowsItem&>(rSet->Get( _nWhich ));
                m_pWidowBox->EnableTriState( false );
                const sal_uInt16 nLines = rWidow.GetValue();

                bool _bEnable = nLines > 0;
                m_pWidowRowNo->SetValue( m_pWidowRowNo->Normalize( nLines ) );
                m_pWidowBox->SetState( _bEnable ? TRISTATE_TRUE : TRISTATE_FALSE);
                m_pWidowRowNo->Enable(_bEnable);
                //m_pWidowRowLabel->Enable(_bEnable);

            }
            else if ( SfxItemState::DONTCARE == eTmpState )
                m_pWidowBox->SetState( TRISTATE_INDET );
            else
                m_pWidowBox->Enable(false);

            m_pOrphanBox->Enable();
            _nWhich = GetWhich( SID_ATTR_PARA_ORPHANS );
            eTmpState = rSet->GetItemState( _nWhich );

            if ( eTmpState >= SfxItemState::DEFAULT )
            {
                const SvxOrphansItem& rOrphan =
                    static_cast<const SvxOrphansItem&>(rSet->Get( _nWhich ));
                const sal_uInt16 nLines = rOrphan.GetValue();
                m_pOrphanBox->EnableTriState( false );

                bool _bEnable = nLines > 0;
                m_pOrphanBox->SetState( _bEnable ? TRISTATE_TRUE : TRISTATE_FALSE);
                m_pOrphanRowNo->SetValue( m_pOrphanRowNo->Normalize( nLines ) );
                m_pOrphanRowNo->Enable(_bEnable);
                m_pOrphanRowLabel->Enable(_bEnable);

            }
            else if ( SfxItemState::DONTCARE == eTmpState )
                m_pOrphanBox->SetState( TRISTATE_INDET );
            else
                m_pOrphanBox->Enable(false);
        }
    }
    else if ( SfxItemState::DONTCARE == eItemState )
        m_pKeepTogetherBox->SetState( TRISTATE_INDET );
    else
        m_pKeepTogetherBox->Enable(false);

    // so that everything is enabled correctly
    KeepTogetherHdl_Impl( nullptr );
    WidowHdl_Impl( nullptr );
    OrphanHdl_Impl( nullptr );
    ChangesApplied();
}
void SvxExtParagraphTabPage::ChangesApplied()
{
    m_pHyphenBox->SaveValue();
    m_pExtHyphenBeforeBox->SetValue(m_pExtHyphenBeforeBox->GetValue());
    m_pExtHyphenAfterBox->SetValue(m_pExtHyphenAfterBox->GetValue());
    m_pMaxHyphenEdit->SetValue(m_pMaxHyphenEdit->GetValue());
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
    m_pOrphanRowNo->SetValue(m_pOrphanRowNo->GetValue());
    m_pWidowRowNo->SetValue(m_pWidowRowNo->GetValue());
}

DeactivateRC SvxExtParagraphTabPage::DeactivatePage( SfxItemSet* _pSet )
{
    if ( _pSet )
        FillItemSet( _pSet );
    return DeactivateRC::LeavePage;
}

void SvxExtParagraphTabPage::DisablePageBreak()
{
    bPageBreak = false;
    m_pPageBreakBox->Enable(false);
    m_pBreakTypeLB->RemoveEntry(0);
    m_pBreakPositionFT->Enable(false);
    m_pBreakPositionLB->Enable(false);
    m_pApplyCollBtn->Enable(false);
    m_pApplyCollBox->Enable(false);
    m_pPagenumEdit->Enable(false);
}

SvxExtParagraphTabPage::SvxExtParagraphTabPage( vcl::Window* pParent, const SfxItemSet& rAttr ) :
    SfxTabPage( pParent, "TextFlowPage","cui/ui/textflowpage.ui", &rAttr ),

    bPageBreak  ( true ),
    bHtmlMode   ( false ),
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
        pPool->SetSearchMask( SfxStyleFamily::Page );
        SfxStyleSheetBase* pStyle = pPool->First();
        OUString aStdName;

        while( pStyle )
        {
            if ( aStdName.isEmpty() )
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
        bHtmlMode = true;
        m_pHyphenBox           ->Enable(false);
        m_pBeforeText          ->Enable(false);
        m_pExtHyphenBeforeBox  ->Enable(false);
        m_pAfterText           ->Enable(false);
        m_pExtHyphenAfterBox   ->Enable(false);
        m_pMaxHyphenLabel      ->Enable(false);
        m_pMaxHyphenEdit       ->Enable(false);
        m_pPagenumText         ->Enable(false);
        m_pPagenumEdit         ->Enable(false);
        // no column break in HTML
        m_pBreakTypeLB->RemoveEntry(1);
    }
}

SvxExtParagraphTabPage::~SvxExtParagraphTabPage()
{
    disposeOnce();
}

void SvxExtParagraphTabPage::dispose()
{
    m_pHyphenBox.clear();
    m_pBeforeText.clear();
    m_pExtHyphenBeforeBox.clear();
    m_pAfterText.clear();
    m_pExtHyphenAfterBox.clear();
    m_pMaxHyphenLabel.clear();
    m_pMaxHyphenEdit.clear();
    m_pPageBreakBox.clear();
    m_pBreakTypeFT.clear();
    m_pBreakTypeLB.clear();
    m_pBreakPositionFT.clear();
    m_pBreakPositionLB.clear();
    m_pApplyCollBtn.clear();
    m_pApplyCollBox.clear();
    m_pPagenumText.clear();
    m_pPagenumEdit.clear();
    m_pKeepTogetherBox.clear();
    m_pKeepParaBox.clear();
    m_pOrphanBox.clear();
    m_pOrphanRowNo.clear();
    m_pOrphanRowLabel.clear();
    m_pWidowBox.clear();
    m_pWidowRowNo.clear();
    m_pWidowRowLabel.clear();
    SfxTabPage::dispose();
}

IMPL_LINK_NOARG(SvxExtParagraphTabPage, PageBreakHdl_Impl, Button*, void)
{
    switch ( m_pPageBreakBox->GetState() )
    {
        case TRISTATE_TRUE:
            m_pBreakTypeFT->Enable();
            m_pBreakTypeLB->Enable();
            m_pBreakPositionFT->Enable();
            m_pBreakPositionLB->Enable();

            if ( 0 == m_pBreakTypeLB->GetSelectEntryPos()&&
                0 == m_pBreakPositionLB->GetSelectEntryPos() )
            {
                m_pApplyCollBtn->Enable();

                bool bEnable = TRISTATE_TRUE == m_pApplyCollBtn->GetState() &&
                                            m_pApplyCollBox->GetEntryCount();
                m_pApplyCollBox->Enable(bEnable);
                if(!bHtmlMode)
                {
                    m_pPagenumText->Enable(bEnable);
                    m_pPagenumEdit->Enable(bEnable);
                }
            }
            break;

        case TRISTATE_FALSE:
        case TRISTATE_INDET:
            m_pApplyCollBtn->SetState( TRISTATE_FALSE );
            m_pApplyCollBtn->Enable(false);
            m_pApplyCollBox->Enable(false);
            m_pPagenumText->Enable(false);
            m_pPagenumEdit->Enable(false);
            m_pBreakTypeFT->Enable(false);
            m_pBreakTypeLB->Enable(false);
            m_pBreakPositionFT->Enable(false);
            m_pBreakPositionLB->Enable(false);
            break;
    }
}

IMPL_LINK_NOARG(SvxExtParagraphTabPage, KeepTogetherHdl_Impl, Button*, void)
{
    bool bEnable = m_pKeepTogetherBox->GetState() == TRISTATE_FALSE;
    m_pWidowBox->Enable(bEnable);
    m_pOrphanBox->Enable(bEnable);
}

IMPL_LINK_NOARG(SvxExtParagraphTabPage, WidowHdl_Impl, Button*, void)
{
    switch ( m_pWidowBox->GetState() )
    {
        case TRISTATE_TRUE:
            m_pWidowRowNo->Enable();
            m_pWidowRowLabel->Enable();
            m_pKeepTogetherBox->Enable(false);
            break;

        case TRISTATE_FALSE:
            if ( m_pOrphanBox->GetState() == TRISTATE_FALSE )
                m_pKeepTogetherBox->Enable();

            SAL_FALLTHROUGH;
        case TRISTATE_INDET:
            m_pWidowRowNo->Enable(false);
            m_pWidowRowLabel->Enable(false);
            break;
    }
}

IMPL_LINK_NOARG(SvxExtParagraphTabPage, OrphanHdl_Impl, Button*, void)
{
    switch( m_pOrphanBox->GetState() )
    {
        case TRISTATE_TRUE:
            m_pOrphanRowNo->Enable();
            m_pOrphanRowLabel->Enable();
            m_pKeepTogetherBox->Enable(false);
            break;

        case TRISTATE_FALSE:
            if ( m_pWidowBox->GetState() == TRISTATE_FALSE )
                m_pKeepTogetherBox->Enable();

            SAL_FALLTHROUGH;
        case TRISTATE_INDET:
            m_pOrphanRowNo->Enable(false);
            m_pOrphanRowLabel->Enable(false);
            break;
    }
}

IMPL_LINK_NOARG(SvxExtParagraphTabPage, HyphenClickHdl_Impl, Button*, void)
{

    bool bEnable = m_pHyphenBox->GetState() == TRISTATE_TRUE;
    m_pBeforeText->Enable(bEnable);
    m_pExtHyphenBeforeBox->Enable(bEnable);
    m_pAfterText->Enable(bEnable);
    m_pExtHyphenAfterBox->Enable(bEnable);
    m_pMaxHyphenLabel->Enable(bEnable);
    m_pMaxHyphenEdit->Enable(bEnable);
    m_pHyphenBox->SetState( bEnable ? TRISTATE_TRUE : TRISTATE_FALSE);
}

IMPL_LINK_NOARG(SvxExtParagraphTabPage, ApplyCollClickHdl_Impl, Button*, void)
{
    bool bEnable = false;
    if ( m_pApplyCollBtn->GetState() == TRISTATE_TRUE &&
         m_pApplyCollBox->GetEntryCount() )
    {
        bEnable = true;
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
}

IMPL_LINK( SvxExtParagraphTabPage, PageBreakPosHdl_Impl, ListBox&, rListBox, void )
{
    if ( 0 == rListBox.GetSelectEntryPos() )
    {
        m_pApplyCollBtn->Enable();

        bool bEnable = m_pApplyCollBtn->GetState() == TRISTATE_TRUE &&
                                    m_pApplyCollBox->GetEntryCount();

        m_pApplyCollBox->Enable(bEnable);
        if(!bHtmlMode)
        {
            m_pPagenumText->Enable(bEnable);
            m_pPagenumEdit->Enable(bEnable);
        }
    }
    else if ( 1 == rListBox.GetSelectEntryPos() )
    {
        m_pApplyCollBtn->SetState( TRISTATE_FALSE );
        m_pApplyCollBtn->Enable(false);
        m_pApplyCollBox->Enable(false);
        m_pPagenumText->Enable(false);
        m_pPagenumEdit->Enable(false);
    }
}

IMPL_LINK( SvxExtParagraphTabPage, PageBreakTypeHdl_Impl, ListBox&, rListBox, void )
{
    //column break or break break after
    sal_Int32 nBreakPos = m_pBreakPositionLB->GetSelectEntryPos();
    if ( rListBox.GetSelectEntryPos() == 1 || 1 == nBreakPos)
    {
        m_pApplyCollBtn->SetState( TRISTATE_FALSE );
        m_pApplyCollBtn->Enable(false);
        m_pApplyCollBox->Enable(false);
        m_pPagenumText->Enable(false);
        m_pPagenumEdit->Enable(false);
    }
    else
        PageBreakPosHdl_Impl( *m_pBreakPositionLB );
}

void SvxExtParagraphTabPage::PageCreated(const SfxAllItemSet& aSet)
{
    const SfxBoolItem* pDisablePageBreakItem = aSet.GetItem<SfxBoolItem>(SID_DISABLE_SVXEXTPARAGRAPHTABPAGE_PAGEBREAK, false);

    if (pDisablePageBreakItem)
        if ( pDisablePageBreakItem->GetValue())
                    DisablePageBreak();
}

SvxAsianTabPage::SvxAsianTabPage( vcl::Window* pParent, const SfxItemSet& rSet ) :
    SfxTabPage(pParent, "AsianTypography","cui/ui/asiantypography.ui", &rSet)

{
    get(m_pForbiddenRulesCB,"checkForbidList");
    get(m_pHangingPunctCB,"checkHangPunct");
    get(m_pScriptSpaceCB,"checkApplySpacing");

    Link<Button*,void> aLink = LINK( this, SvxAsianTabPage, ClickHdl_Impl );
    m_pHangingPunctCB->SetClickHdl( aLink );
    m_pScriptSpaceCB->SetClickHdl( aLink );
    m_pForbiddenRulesCB->SetClickHdl( aLink );

}

SvxAsianTabPage::~SvxAsianTabPage()
{
    disposeOnce();
}

void SvxAsianTabPage::dispose()
{
    m_pForbiddenRulesCB.clear();
    m_pHangingPunctCB.clear();
    m_pScriptSpaceCB.clear();
    SfxTabPage::dispose();
}

VclPtr<SfxTabPage> SvxAsianTabPage::Create( vcl::Window* pParent, const SfxItemSet* rSet )
{
    return VclPtr<SvxAsianTabPage>::Create(pParent, *rSet);
}

const sal_uInt16*     SvxAsianTabPage::GetRanges()
{
    static const sal_uInt16 pRanges[] =
    {
        SID_ATTR_PARA_SCRIPTSPACE, SID_ATTR_PARA_FORBIDDEN_RULES,
        0
    };
    return pRanges;
}

bool SvxAsianTabPage::FillItemSet( SfxItemSet* rSet )
{
    bool bRet = false;
    SfxItemPool* pPool = rSet->GetPool();
    if (m_pScriptSpaceCB->IsEnabled() && m_pScriptSpaceCB->IsValueChangedFromSaved())
    {
        std::unique_ptr<SfxBoolItem> pNewItem(static_cast<SfxBoolItem*>(rSet->Get(
            pPool->GetWhich(SID_ATTR_PARA_SCRIPTSPACE)).Clone()));
        pNewItem->SetValue(m_pScriptSpaceCB->IsChecked());
        rSet->Put(*pNewItem);
        bRet = true;
    }
    if (m_pHangingPunctCB->IsEnabled() && m_pHangingPunctCB->IsValueChangedFromSaved())
    {
        std::unique_ptr<SfxBoolItem> pNewItem(static_cast<SfxBoolItem*>(rSet->Get(
            pPool->GetWhich(SID_ATTR_PARA_HANGPUNCTUATION)).Clone()));
        pNewItem->SetValue(m_pHangingPunctCB->IsChecked());
        rSet->Put(*pNewItem);
        bRet = true;
    }
    if (m_pForbiddenRulesCB->IsEnabled() && m_pForbiddenRulesCB->IsValueChangedFromSaved())
    {
        std::unique_ptr<SfxBoolItem> pNewItem(static_cast<SfxBoolItem*>(rSet->Get(
            pPool->GetWhich(SID_ATTR_PARA_FORBIDDEN_RULES)).Clone()));
        pNewItem->SetValue(m_pForbiddenRulesCB->IsChecked());
        rSet->Put(*pNewItem);
        bRet = true;
    }
    return bRet;
}

static void lcl_SetBox(const SfxItemSet& rSet, sal_uInt16 nSlotId, CheckBox& rBox)
{
    sal_uInt16 _nWhich = rSet.GetPool()->GetWhich(nSlotId);
    SfxItemState eState = rSet.GetItemState(_nWhich);
    if( eState == SfxItemState::UNKNOWN || eState == SfxItemState::DISABLED )
        rBox.Enable(false);
    else if(eState >= SfxItemState::DEFAULT)
    {
        rBox.EnableTriState( false );
        rBox.Check(static_cast<const SfxBoolItem&>(rSet.Get(_nWhich)).GetValue());
    }
    else
        rBox.SetState( TRISTATE_INDET );
    rBox.SaveValue();
}

void SvxAsianTabPage::Reset( const SfxItemSet* rSet )
{
    lcl_SetBox(*rSet, SID_ATTR_PARA_FORBIDDEN_RULES, *m_pForbiddenRulesCB );
    lcl_SetBox(*rSet, SID_ATTR_PARA_HANGPUNCTUATION, *m_pHangingPunctCB );

    //character distance not yet available
    lcl_SetBox(*rSet, SID_ATTR_PARA_SCRIPTSPACE, *m_pScriptSpaceCB );
}
void SvxAsianTabPage::ChangesApplied()
{
    m_pForbiddenRulesCB->SaveValue();
    m_pHangingPunctCB->SaveValue();
    m_pScriptSpaceCB->SaveValue();
}
IMPL_STATIC_LINK( SvxAsianTabPage, ClickHdl_Impl, Button*, pBox, void )
{
    static_cast<CheckBox*>(pBox)->EnableTriState( false );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
