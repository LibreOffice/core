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

#include <memory>
#include <osl/diagnose.h>
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
#include <svx/strings.hrc>
#include <svx/dialmgr.hxx>
#include <paragrph.hxx>
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
#include <svx/dlgutil.hxx>
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

static void SetLineSpace_Impl( SvxLineSpacingItem&, int, long lValue = 0 );

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
            rLineSpace.SetPropLineSpace( static_cast<sal_uInt16>(lValue) );
            break;

        case LLINESPACE_MIN:
            rLineSpace.SetLineHeight( static_cast<sal_uInt16>(lValue) );
            rLineSpace.SetInterLineSpaceRule( SvxInterLineSpaceRule::Off );
            break;

        case LLINESPACE_DURCH:
            rLineSpace.SetLineSpaceRule( SvxLineSpaceRule::Auto );
            rLineSpace.SetInterLineSpace( static_cast<sal_uInt16>(lValue) );
            break;

        case LLINESPACE_FIX:
            rLineSpace.SetLineHeight(static_cast<sal_uInt16>(lValue));
            rLineSpace.SetLineSpaceRule( SvxLineSpaceRule::Fix );
            rLineSpace.SetInterLineSpaceRule( SvxInterLineSpaceRule::Off );
        break;
    }
}

static sal_uInt16 GetHtmlMode_Impl(const SfxItemSet& rSet)
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

void SvxStdParagraphTabPage::ELRLoseFocus()
{
    SfxItemPool* pPool = GetItemSet().GetPool();
    DBG_ASSERT( pPool, "Where is the pool?" );
    FieldUnit eUnit =
        MapToFieldUnit( pPool->GetMetric( GetWhich( SID_ATTR_LRSPACE ) ) );

    sal_Int64 nL = m_xLeftIndent->denormalize(m_xLeftIndent->get_value(eUnit));
    sal_Int64 nR = m_xRightIndent->denormalize(m_xRightIndent->get_value(eUnit));
    OUString aTmp = m_xFLineIndent->get_text();

    if (m_xLeftIndent->get_min(FieldUnit::NONE) < 0)
        m_xFLineIndent->set_min(-99999, FieldUnit::MM);
    else
        m_xFLineIndent->set_min(m_xFLineIndent->normalize(-nL), eUnit);

    // Check only for concrete width (Shell)
    sal_Int64 nTmp = nWidth - nL - nR - MM50;
    m_xFLineIndent->set_max(m_xFLineIndent->normalize(nTmp), eUnit);

    if (aTmp.isEmpty())
        m_xFLineIndent->set_text(OUString());
    // maximum left right
    aTmp = m_xLeftIndent->get_text();
    nTmp = nWidth - nR - MM50;
    m_xLeftIndent->set_max(m_xLeftIndent->normalize(nTmp), eUnit);

    if ( aTmp.isEmpty() )
        m_xLeftIndent->set_text(OUString());
    aTmp = m_xRightIndent->get_text();
    nTmp = nWidth - nL - MM50;
    m_xRightIndent->set_max(m_xRightIndent->normalize(nTmp), eUnit);

    if ( aTmp.isEmpty() )
        m_xRightIndent->set_text(OUString());

    UpdateExample_Impl();
}

IMPL_LINK_NOARG(SvxStdParagraphTabPage, ELRLoseFocusHdl, weld::MetricSpinButton&, void)
{
    ELRLoseFocus();
}

VclPtr<SfxTabPage> SvxStdParagraphTabPage::Create( TabPageParent pParent, const SfxItemSet* rSet)
{
    return VclPtr<SvxStdParagraphTabPage>::Create(pParent, *rSet);
}

bool SvxStdParagraphTabPage::FillItemSet( SfxItemSet* rOutSet )
{
    SfxItemState eState = SfxItemState::UNKNOWN;
    const SfxPoolItem* pOld = nullptr;
    SfxItemPool* pPool = rOutSet->GetPool();
    DBG_ASSERT( pPool, "Where is the pool?" );

    bool bModified = false;
    sal_uInt16 nWhich;
    int nPos = m_xLineDist->get_active();

    if ( nPos != -1 &&
         ( m_xLineDist->get_value_changed_from_saved() ||
           m_xLineDistAtPercentBox->get_value_changed_from_saved() ||
           m_xLineDistAtMetricBox->get_value_changed_from_saved() ) )
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
                                   static_cast<long>(m_xLineDistAtPercentBox->denormalize(
                                   m_xLineDistAtPercentBox->get_value(FieldUnit::NONE) )) );
                break;

            case LLINESPACE_MIN:
            case LLINESPACE_DURCH:
            case LLINESPACE_FIX:
                SetLineSpace_Impl( aSpacing, nPos,
                    GetCoreValue( *m_xLineDistAtMetricBox, eUnit ) );
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

    if ( m_xTopDist->get_value_changed_from_saved() || m_xBottomDist->get_value_changed_from_saved()
         || m_xContextualCB->get_state_changed_from_saved())
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

            if ( m_xTopDist->IsRelative() )
                aMargin.SetUpper( rOldItem.GetUpper(),
                                  static_cast<sal_uInt16>(m_xTopDist->get_value(FieldUnit::NONE)) );
            else
                aMargin.SetUpper( static_cast<sal_uInt16>(m_xTopDist->GetCoreValue(eUnit)) );

            if ( m_xBottomDist->IsRelative() )
                aMargin.SetLower( rOldItem.GetLower(),
                                  static_cast<sal_uInt16>(m_xBottomDist->get_value(FieldUnit::NONE)) );
            else
                aMargin.SetLower( static_cast<sal_uInt16>(m_xBottomDist->GetCoreValue(eUnit)) );

        }
        else
        {
            aMargin.SetUpper(static_cast<sal_uInt16>(m_xTopDist->GetCoreValue(eUnit)));
            aMargin.SetLower(static_cast<sal_uInt16>(m_xBottomDist->GetCoreValue(eUnit)));
        }
        aMargin.SetContextValue(m_xContextualCB->get_active());
        eState = GetItemSet().GetItemState( nWhich );

        if ( !pOld || *static_cast<const SvxULSpaceItem*>(pOld) != aMargin ||
             SfxItemState::DONTCARE == eState )
        {
            rOutSet->Put( aMargin );
            bModified = true;
        }
    }
    bool bNullTab = false;

    if ( m_xLeftIndent->get_value_changed_from_saved() ||
         m_xFLineIndent->get_value_changed_from_saved() ||
         m_xRightIndent->get_value_changed_from_saved() ||
         m_xAutoCB->get_state_changed_from_saved() )
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

            if (m_xLeftIndent->IsRelative())
                aMargin.SetTextLeft( rOldItem.GetTextLeft(),
                                    static_cast<sal_uInt16>(m_xLeftIndent->get_value(FieldUnit::NONE)) );
            else
                aMargin.SetTextLeft(m_xLeftIndent->GetCoreValue(eUnit));

            if ( m_xRightIndent->IsRelative() )
                aMargin.SetRight( rOldItem.GetRight(),
                                  static_cast<sal_uInt16>(m_xRightIndent->get_value(FieldUnit::NONE)) );
            else
                aMargin.SetRight(m_xRightIndent->GetCoreValue(eUnit));

            if ( m_xFLineIndent->IsRelative() )
                aMargin.SetTextFirstLineOfst( rOldItem.GetTextFirstLineOfst(),
                                             static_cast<sal_uInt16>(m_xFLineIndent->get_value(FieldUnit::NONE)) );
            else
                aMargin.SetTextFirstLineOfst(static_cast<sal_uInt16>(m_xFLineIndent->GetCoreValue(eUnit)));
        }
        else
        {
            aMargin.SetTextLeft(m_xLeftIndent->GetCoreValue(eUnit));
            aMargin.SetRight(m_xRightIndent->GetCoreValue(eUnit));
            aMargin.SetTextFirstLineOfst(static_cast<sal_uInt16>(m_xFLineIndent->GetCoreValue(eUnit)));
        }
        aMargin.SetAutoFirst(m_xAutoCB->get_active());
        if ( aMargin.GetTextFirstLineOfst() < 0 )
            bNullTab = true;
        eState = GetItemSet().GetItemState( nWhich );

        if ( !pOld || *static_cast<const SvxLRSpaceItem*>(pOld) != aMargin ||
             SfxItemState::DONTCARE == eState )
        {
            rOutSet->Put( aMargin );
            bModified = true;
        }
    }

    if ( bNullTab )
    {
        MapUnit eUnit = pPool->GetMetric( GetWhich( SID_ATTR_TABSTOP ) );
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
    if (m_xRegisterCB->get_visible())
    {
        const SfxBoolItem* pBoolItem = static_cast<const SfxBoolItem*>(GetOldItem(
                            *rOutSet, SID_ATTR_PARA_REGISTER));
        if (!pBoolItem)
            return bModified;
        std::unique_ptr<SfxBoolItem> pRegItem(static_cast<SfxBoolItem*>(pBoolItem->Clone()));
        sal_uInt16 _nWhich = GetWhich( SID_ATTR_PARA_REGISTER );
        bool bSet = pRegItem->GetValue();

        if (m_xRegisterCB->get_active() != bSet)
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
    DBG_ASSERT( pPool, "Where is the pool?" );

    // adjust metric
    FieldUnit eFUnit = GetModuleFieldUnit( *rSet );

    bool bApplyCharUnit = GetApplyCharUnit( *rSet );

    SvtCJKOptions aCJKOptions;
    if(aCJKOptions.IsAsianTypographyEnabled() && bApplyCharUnit )
        eFUnit = FieldUnit::CHAR;

    m_xLeftIndent->SetFieldUnit(eFUnit);
    m_xRightIndent->SetFieldUnit(eFUnit);
    m_xFLineIndent->SetFieldUnit(eFUnit);
    if ( eFUnit == FieldUnit::CHAR )
    {
        m_xTopDist->SetFieldUnit(FieldUnit::LINE);
        m_xBottomDist->SetFieldUnit(FieldUnit::LINE);
        SetFieldUnit(*m_xLineDistAtMetricBox, FieldUnit::POINT);
    }
    else
    {
        m_xTopDist->SetFieldUnit(eFUnit);
        m_xBottomDist->SetFieldUnit(eFUnit);
        SetFieldUnit(*m_xLineDistAtMetricBox, eFUnit);
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
                m_xLeftIndent->SetRelative( true );
                m_xLeftIndent->set_value(rOldItem.GetPropLeft(), FieldUnit::NONE);
            }
            else
            {
                m_xLeftIndent->SetRelative(false);
                m_xLeftIndent->SetFieldUnit(eFUnit);
                m_xLeftIndent->SetMetricValue(rOldItem.GetTextLeft(), eUnit);
            }

            if ( rOldItem.GetPropRight() != 100 )
            {
                m_xRightIndent->SetRelative( true );
                m_xRightIndent->set_value(rOldItem.GetPropRight(), FieldUnit::NONE);
            }
            else
            {
                m_xRightIndent->SetRelative(false);
                m_xRightIndent->SetFieldUnit(eFUnit);
                m_xRightIndent->SetMetricValue(rOldItem.GetRight(), eUnit);
            }

            if ( rOldItem.GetPropTextFirstLineOfst() != 100 )
            {
                m_xFLineIndent->SetRelative(true);
                m_xFLineIndent->set_value(rOldItem.GetPropTextFirstLineOfst(), FieldUnit::NONE);
            }
            else
            {
                m_xFLineIndent->SetRelative(false);
                m_xFLineIndent->set_min(-9999, FieldUnit::NONE);
                m_xFLineIndent->SetFieldUnit(eFUnit);
                m_xFLineIndent->SetMetricValue(rOldItem.GetTextFirstLineOfst(), eUnit);
            }
            m_xAutoCB->set_active(rOldItem.IsAutoFirst());
        }
        else
        {
            const SvxLRSpaceItem& rSpace =
                static_cast<const SvxLRSpaceItem&>(rSet->Get( _nWhich ));

            m_xLeftIndent->SetMetricValue(rSpace.GetTextLeft(), eUnit);
            m_xRightIndent->SetMetricValue(rSpace.GetRight(), eUnit);
            m_xFLineIndent->SetMetricValue(rSpace.GetTextFirstLineOfst(), eUnit);
            m_xAutoCB->set_active(rSpace.IsAutoFirst());
        }
        AutoHdl_Impl(*m_xAutoCB);
    }
    else
    {
        m_xLeftIndent->set_text(OUString());
        m_xRightIndent->set_text(OUString());
        m_xFLineIndent->set_text(OUString());
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
                m_xTopDist->SetRelative( true );
                m_xTopDist->set_value(rOldItem.GetPropUpper(), FieldUnit::NONE);
            }
            else
            {
                m_xTopDist->SetRelative(false);
                if (eFUnit == FieldUnit::CHAR)
                    m_xTopDist->SetFieldUnit(FieldUnit::LINE);
                else
                    m_xTopDist->SetFieldUnit(eFUnit);
                m_xTopDist->SetMetricValue(rOldItem.GetUpper(), eUnit);
            }

            if ( rOldItem.GetPropLower() != 100 )
            {
                m_xBottomDist->SetRelative( true );
                m_xBottomDist->set_value(rOldItem.GetPropLower(), FieldUnit::NONE);
            }
            else
            {
                m_xBottomDist->SetRelative(false);
                if (eFUnit == FieldUnit::CHAR)
                    m_xBottomDist->SetFieldUnit(FieldUnit::LINE);
                else
                    m_xBottomDist->SetFieldUnit(eFUnit);
                m_xBottomDist->SetMetricValue(rOldItem.GetLower(), eUnit);
            }
        }
        else
        {
            m_xTopDist->SetMetricValue(rOldItem.GetUpper(), eUnit);
            m_xBottomDist->SetMetricValue(rOldItem.GetLower(), eUnit);
        }
        m_xContextualCB->set_active(rOldItem.GetContext());
    }
    else
    {
        m_xTopDist->set_text(OUString());
        m_xBottomDist->set_text(OUString());
    }

    _nWhich = GetWhich( SID_ATTR_PARA_LINESPACE );
    eItemState = rSet->GetItemState( _nWhich );

    if ( eItemState >= SfxItemState::DEFAULT )
        SetLineSpacing_Impl( static_cast<const SvxLineSpacingItem &>(rSet->Get( _nWhich )) );
    else
        m_xLineDist->set_active(-1);

    _nWhich = GetWhich( SID_ATTR_PARA_REGISTER );
    eItemState = rSet->GetItemState( _nWhich );

    if ( eItemState >= SfxItemState::DEFAULT )
        m_xRegisterCB->set_active( static_cast<const SfxBoolItem &>(rSet->Get( _nWhich )).GetValue());
    m_xRegisterCB->save_state();
    sal_uInt16 nHtmlMode = GetHtmlMode_Impl(*rSet);
    if(nHtmlMode & HTMLMODE_ON)
    {
        m_xRegisterFL->hide();
        m_xRegisterCB->hide();
        m_xAutoCB->hide();
    }

    // this sets the min/max limits; do this _after_ setting the values,
    // because for Impress the min of first-line indent depends on value of
    // left-indent!
    ELRLoseFocus();
    ChangesApplied();
}

void SvxStdParagraphTabPage::ChangesApplied()
{
    m_xLeftIndent->save_value();
    m_xRightIndent->save_value();
    m_xFLineIndent->save_value();
    m_xLineDist->save_value();
    m_xLineDistAtPercentBox->save_value();
    m_xLineDistAtMetricBox->save_value();
    m_xRegisterCB->save_state();
    m_xTopDist->save_value();
    m_xBottomDist->save_value();
    m_xContextualCB->save_state();
    m_xAutoCB->save_state();
}

void SvxStdParagraphTabPage::EnableRelativeMode()
{
    DBG_ASSERT( GetItemSet().GetParent(), "RelativeMode, but no parent-set!" );

    m_xLeftIndent->EnableRelativeMode( 0, 999 );
    m_xFLineIndent->EnableRelativeMode( 0, 999 );
    m_xRightIndent->EnableRelativeMode( 0, 999 );
    m_xTopDist->EnableRelativeMode( 0, 999 );
    m_xBottomDist->EnableRelativeMode( 0, 999 );
    bRelativeMode = true;
}

void SvxStdParagraphTabPage::ActivatePage( const SfxItemSet& rSet )
{
    sal_uInt16 _nWhich = GetWhich( SID_ATTR_PARA_ADJUST );
    SfxItemState eItemState = rSet.GetItemState( _nWhich );

    if ( eItemState >= SfxItemState::DEFAULT )
    {
        const SvxAdjustItem& rAdj = static_cast<const SvxAdjustItem&>( rSet.Get( _nWhich ) );
        SvxAdjust eAdjust = rAdj.GetAdjust();
        if ( eAdjust == SvxAdjust::Center || eAdjust == SvxAdjust::Block )
        {
            _nWhich = GetWhich( SID_ATTR_FRAMEDIRECTION );
            eItemState = rSet.GetItemState( _nWhich );

            if ( eItemState >= SfxItemState::DEFAULT )
            {
                const SvxFrameDirectionItem& rFrameDirItem = static_cast<const SvxFrameDirectionItem&>( rSet.Get( _nWhich ) );
                SvxFrameDirection eFrameDirection = rFrameDirItem.GetValue();

                m_aExampleWin.EnableRTL( SvxFrameDirection::Horizontal_RL_TB == eFrameDirection );

                if ( eAdjust == SvxAdjust::Block )
                    m_aExampleWin.SetLastLine( rAdj.GetLastBlock() );
            }
        }
        else
        {
            m_aExampleWin.EnableRTL( eAdjust == SvxAdjust::Right );
            eAdjust = SvxAdjust::Left; //required for correct preview display
            m_aExampleWin.SetLastLine( eAdjust );
        }
        m_aExampleWin.SetAdjust( eAdjust );

        UpdateExample_Impl();
    }
}

DeactivateRC SvxStdParagraphTabPage::DeactivatePage( SfxItemSet* _pSet )
{
    ELRLoseFocus();

    if ( _pSet )
        FillItemSet( _pSet );
    return DeactivateRC::LeavePage;
}

SvxStdParagraphTabPage::SvxStdParagraphTabPage(TabPageParent pParent,  const SfxItemSet& rAttr)
    : SfxTabPage(pParent, "cui/ui/paraindentspacing.ui", "ParaIndentSpacing", &rAttr)
    , nWidth(11905 /*567 * 50*/)
    , nMinFixDist(0)
    , bRelativeMode(false)
    , m_xLeftIndent(new RelativeField(m_xBuilder->weld_metric_spin_button("spinED_LEFTINDENT", FieldUnit::CM)))
    , m_xRightLabel(m_xBuilder->weld_label("labelFT_RIGHTINDENT"))
    , m_xRightIndent(new RelativeField(m_xBuilder->weld_metric_spin_button("spinED_RIGHTINDENT", FieldUnit::CM)))
    , m_xFLineLabel(m_xBuilder->weld_label("labelFT_FLINEINDENT"))
    , m_xFLineIndent(new RelativeField(m_xBuilder->weld_metric_spin_button("spinED_FLINEINDENT", FieldUnit::CM)))
    , m_xAutoCB(m_xBuilder->weld_check_button("checkCB_AUTO"))
    , m_xTopDist(new RelativeField(m_xBuilder->weld_metric_spin_button("spinED_TOPDIST", FieldUnit::CM)))
    , m_xBottomDist(new RelativeField(m_xBuilder->weld_metric_spin_button("spinED_BOTTOMDIST", FieldUnit::CM)))
    , m_xContextualCB(m_xBuilder->weld_check_button("checkCB_CONTEXTUALSPACING"))
    , m_xLineDist(m_xBuilder->weld_combo_box("comboLB_LINEDIST"))
    , m_xLineDistAtPercentBox(m_xBuilder->weld_metric_spin_button("spinED_LINEDISTPERCENT", FieldUnit::PERCENT))
    , m_xLineDistAtMetricBox(m_xBuilder->weld_metric_spin_button("spinED_LINEDISTMETRIC", FieldUnit::CM))
    , m_xLineDistAtLabel(m_xBuilder->weld_label("labelFT_LINEDIST"))
    , m_xAbsDist(m_xBuilder->weld_label("labelST_LINEDIST_ABS"))
    , m_xRegisterFL(m_xBuilder->weld_widget("frameFL_REGISTER"))
    , m_xRegisterCB(m_xBuilder->weld_check_button("checkCB_REGISTER"))
    , m_xExampleWin(new weld::CustomWeld(*m_xBuilder, "drawingareaWN_EXAMPLE", m_aExampleWin))
{
    sAbsDist = m_xAbsDist->get_label();

    // this page needs ExchangeSupport
    SetExchangeSupport();

    m_xLineDistAtMetricBox->hide();

    Init_Impl();
    m_xFLineIndent->set_min(-9999, FieldUnit::NONE);    // is set to 0 on default
}

SvxStdParagraphTabPage::~SvxStdParagraphTabPage()
{
}

void SvxStdParagraphTabPage::EnableNegativeMode()
{
    m_xLeftIndent->set_min(-9999, FieldUnit::NONE);
    m_xRightIndent->set_min(-9999, FieldUnit::NONE);
    m_xRightIndent->EnableNegativeMode();
    m_xLeftIndent->EnableNegativeMode();
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
                    m_xLineDist->set_active( LLINESPACE_1 );
                    break;

                // Default single line spacing
                case SvxInterLineSpaceRule::Prop:
                    if ( 100 == rAttr.GetPropLineSpace() )
                    {
                        m_xLineDist->set_active( LLINESPACE_1 );
                        break;
                    }
                    // 1.15 line spacing
                    if ( 115 == rAttr.GetPropLineSpace() )
                    {
                        m_xLineDist->set_active( LLINESPACE_115 );
                        break;
                    }
                    // 1.5 line spacing
                    if ( 150 == rAttr.GetPropLineSpace() )
                    {
                        m_xLineDist->set_active( LLINESPACE_15 );
                        break;
                    }
                    // double line spacing
                    if ( 200 == rAttr.GetPropLineSpace() )
                    {
                        m_xLineDist->set_active( LLINESPACE_2 );
                        break;
                    }
                    // the set per cent value
                    m_xLineDistAtPercentBox->set_value(m_xLineDistAtPercentBox->normalize(rAttr.GetPropLineSpace()), FieldUnit::NONE);
                    m_xLineDist->set_active( LLINESPACE_PROP );
                    break;

                case SvxInterLineSpaceRule::Fix:
                    SetMetricValue( *m_xLineDistAtMetricBox, rAttr.GetInterLineSpace(), eUnit );
                    m_xLineDist->set_active( LLINESPACE_DURCH );
                    break;
                default: ;//prevent warning
            }
        }
        break;
        case SvxLineSpaceRule::Fix:
            SetMetricValue(*m_xLineDistAtMetricBox, rAttr.GetLineHeight(), eUnit);
            m_xLineDist->set_active( LLINESPACE_FIX );
        break;

        case SvxLineSpaceRule::Min:
            SetMetricValue(*m_xLineDistAtMetricBox, rAttr.GetLineHeight(), eUnit);
            m_xLineDist->set_active( LLINESPACE_MIN );
            break;
        default: ;//prevent warning
    }
    LineDistHdl_Impl( *m_xLineDist );
}

IMPL_LINK(SvxStdParagraphTabPage, LineDistHdl_Impl, weld::ComboBox&, rBox, void)
{
    switch (rBox.get_active())
    {
        case LLINESPACE_1:
        case LLINESPACE_115:
        case LLINESPACE_15:
        case LLINESPACE_2:
            m_xLineDistAtLabel->set_sensitive(false);
            m_xLineDistAtPercentBox->set_sensitive(false);
            m_xLineDistAtPercentBox->set_text(OUString());
            m_xLineDistAtMetricBox->set_sensitive(false);
            m_xLineDistAtMetricBox->set_text(OUString());
            break;

        case LLINESPACE_DURCH:
            // setting a sensible default?
            // limit MS min(10, aPageSize)
            m_xLineDistAtMetricBox->set_min(0, FieldUnit::NONE);

            if (m_xLineDistAtMetricBox->get_text().isEmpty())
                m_xLineDistAtMetricBox->set_value(m_xLineDistAtMetricBox->normalize(1), FieldUnit::NONE);
            m_xLineDistAtPercentBox->hide();
            m_xLineDistAtMetricBox->show();
            m_xLineDistAtMetricBox->set_sensitive(true);
            m_xLineDistAtLabel->set_sensitive(true);
            break;

        case LLINESPACE_MIN:
            m_xLineDistAtMetricBox->set_min(0, FieldUnit::NONE);

            if (m_xLineDistAtMetricBox->get_text().isEmpty())
                m_xLineDistAtMetricBox->set_value(m_xLineDistAtMetricBox->normalize(10), FieldUnit::TWIP);
            m_xLineDistAtPercentBox->hide();
            m_xLineDistAtMetricBox->show();
            m_xLineDistAtMetricBox->set_sensitive(true);
            m_xLineDistAtLabel->set_sensitive(true);
            break;

        case LLINESPACE_PROP:

            if (m_xLineDistAtPercentBox->get_text().isEmpty())
                m_xLineDistAtPercentBox->set_value(m_xLineDistAtPercentBox->normalize(100), FieldUnit::TWIP);
            m_xLineDistAtMetricBox->hide();
            m_xLineDistAtPercentBox->show();
            m_xLineDistAtPercentBox->set_sensitive(true);
            m_xLineDistAtLabel->set_sensitive(true);
            break;
        case LLINESPACE_FIX:
        {
            auto nTemp = m_xLineDistAtMetricBox->get_value(FieldUnit::NONE);
            m_xLineDistAtMetricBox->set_min(m_xLineDistAtMetricBox->normalize(nMinFixDist), FieldUnit::TWIP);

            // if the value has been changed at SetMin,
            // it is time for the default
            if (m_xLineDistAtMetricBox->get_value(FieldUnit::NONE) != nTemp)
                SetMetricValue( *m_xLineDistAtMetricBox, FIX_DIST_DEF, MapUnit::MapTwip ); // fix is only in Writer
            m_xLineDistAtPercentBox->hide();
            m_xLineDistAtMetricBox->show();
            m_xLineDistAtMetricBox->set_sensitive(true);
            m_xLineDistAtLabel->set_sensitive(true);
        }
        break;
    }
    UpdateExample_Impl();
}

IMPL_LINK_NOARG(SvxStdParagraphTabPage, ModifyHdl_Impl, weld::MetricSpinButton&, void)
{
    UpdateExample_Impl();
}

void SvxStdParagraphTabPage::Init_Impl()
{
    m_xLineDist->connect_changed(LINK(this, SvxStdParagraphTabPage, LineDistHdl_Impl));

    Link<weld::MetricSpinButton&,void> aLink2 = LINK(this, SvxStdParagraphTabPage, ELRLoseFocusHdl);
    m_xFLineIndent->connect_value_changed(aLink2);
    m_xLeftIndent->connect_value_changed(aLink2);
    m_xRightIndent->connect_value_changed(aLink2);

    Link<weld::MetricSpinButton&,void> aLink = LINK(this, SvxStdParagraphTabPage, ModifyHdl_Impl);
    m_xTopDist->connect_value_changed(aLink);
    m_xBottomDist->connect_value_changed(aLink);

    m_xAutoCB->connect_toggled(LINK(this, SvxStdParagraphTabPage, AutoHdl_Impl));
    SfxItemPool* pPool = GetItemSet().GetPool();
    DBG_ASSERT( pPool, "Where is the pool?" );
    FieldUnit eUnit = MapToFieldUnit( pPool->GetMetric( GetWhich( SID_ATTR_LRSPACE ) ) );

    m_xTopDist->set_max( m_xTopDist->normalize( MAX_DURCH ), eUnit );
    m_xBottomDist->set_max( m_xBottomDist->normalize( MAX_DURCH ), eUnit );
    m_xLineDistAtMetricBox->set_max( m_xLineDistAtMetricBox->normalize( MAX_DURCH ), eUnit );
}

void SvxStdParagraphTabPage::UpdateExample_Impl()
{
    m_aExampleWin.SetFirstLineOfst( static_cast<short>(m_xFLineIndent->denormalize( m_xFLineIndent->get_value( FieldUnit::TWIP ) )) );
    m_aExampleWin.SetLeftMargin( static_cast<long>(m_xLeftIndent->denormalize( m_xLeftIndent->get_value( FieldUnit::TWIP ) ) ) );
    m_aExampleWin.SetRightMargin( static_cast<long>(m_xRightIndent->denormalize( m_xRightIndent->get_value( FieldUnit::TWIP ) ) ) );
    m_aExampleWin.SetUpper( static_cast<sal_uInt16>(m_xTopDist->denormalize( m_xTopDist->get_value( FieldUnit::TWIP ) )) );
    m_aExampleWin.SetLower( static_cast<sal_uInt16>(m_xBottomDist->denormalize( m_xBottomDist->get_value( FieldUnit::TWIP ) )) );

    int nPos = m_xLineDist->get_active();

    switch ( nPos )
    {
        case LLINESPACE_1:
        case LLINESPACE_115:
        case LLINESPACE_15:
        case LLINESPACE_2:
        case LLINESPACE_PROP:
        case LLINESPACE_MIN:
        case LLINESPACE_DURCH:
        case LLINESPACE_FIX:
            m_aExampleWin.SetLineSpace( static_cast<SvxPrevLineSpace>(nPos) );
            break;
    }
    m_aExampleWin.Invalidate();
}

void SvxStdParagraphTabPage::EnableRegisterMode()
{
    m_xRegisterCB->show();
    m_xRegisterFL->show();
}

void SvxStdParagraphTabPage::EnableContextualMode()
{
    m_xContextualCB->show();
}

IMPL_LINK(SvxStdParagraphTabPage, AutoHdl_Impl, weld::ToggleButton&, rBox, void)
{
    bool bEnable = !rBox.get_active();
    m_xFLineLabel->set_sensitive(bEnable);
    m_xFLineIndent->set_sensitive(bEnable);
}

void SvxStdParagraphTabPage::EnableAutoFirstLine()
{
    m_xAutoCB->show();
}

void SvxStdParagraphTabPage::EnableAbsLineDist(long nMinTwip)
{
    m_xLineDist->append_text(sAbsDist);
    nMinFixDist = nMinTwip;
}

void SvxStdParagraphTabPage::PageCreated(const SfxAllItemSet& aSet)
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

SvxParaAlignTabPage::SvxParaAlignTabPage(TabPageParent pParent, const SfxItemSet& rSet)
    : SfxTabPage(pParent, "cui/ui/paragalignpage.ui", "ParaAlignPage", &rSet)
    , m_xLeft(m_xBuilder->weld_radio_button("radioBTN_LEFTALIGN"))
    , m_xRight(m_xBuilder->weld_radio_button("radioBTN_RIGHTALIGN"))
    , m_xCenter(m_xBuilder->weld_radio_button("radioBTN_CENTERALIGN"))
    , m_xJustify(m_xBuilder->weld_radio_button("radioBTN_JUSTIFYALIGN"))
    , m_xLeftBottom(m_xBuilder->weld_label("labelST_LEFTALIGN_ASIAN"))
    , m_xRightTop(m_xBuilder->weld_label("labelST_RIGHTALIGN_ASIAN"))
    , m_xLastLineFT(m_xBuilder->weld_label("labelLB_LASTLINE"))
    , m_xLastLineLB(m_xBuilder->weld_combo_box("comboLB_LASTLINE"))
    , m_xExpandCB(m_xBuilder->weld_check_button("checkCB_EXPAND"))
    , m_xSnapToGridCB(m_xBuilder->weld_check_button("checkCB_SNAP"))
    , m_xExampleWin(new weld::CustomWeld(*m_xBuilder, "drawingareaWN_EXAMPLE", m_aExampleWin))
    , m_xVertAlignFL(m_xBuilder->weld_widget("frameFL_VERTALIGN"))
    , m_xVertAlignLB(m_xBuilder->weld_combo_box("comboLB_VERTALIGN"))
    , m_xPropertiesFL(m_xBuilder->weld_widget("framePROPERTIES"))
    , m_xTextDirectionLB(new svx::FrameDirectionListBox(m_xBuilder->weld_combo_box("comboLB_TEXTDIRECTION")))
{
    SetExchangeSupport();

    SvtLanguageOptions aLangOptions;
    sal_uInt16 nLastLinePos = LASTLINEPOS_DEFAULT;

    if ( aLangOptions.IsAsianTypographyEnabled() )
    {
        m_xLeft->set_label(m_xLeftBottom->get_label());
        m_xRight->set_label(m_xRightTop->get_label());

        OUString sLeft(m_xLeft->get_label());
        sLeft = MnemonicGenerator::EraseAllMnemonicChars( sLeft );

        if (m_xLastLineLB->get_count() == LASTLINECOUNT_OLD)
        {
            m_xLastLineLB->remove(0);
            m_xLastLineLB->insert_text(0, sLeft);
        }
        else
            nLastLinePos = LASTLINEPOS_LEFT;
    }

    // remove "Default" or "Left" entry, depends on CJKOptions
    if (m_xLastLineLB->get_count() == LASTLINECOUNT_NEW)
        m_xLastLineLB->remove(nLastLinePos);

    Link<weld::ToggleButton&, void> aLink = LINK( this, SvxParaAlignTabPage, AlignHdl_Impl );
    m_xLeft->connect_toggled(aLink);
    m_xRight->connect_toggled(aLink);
    m_xCenter->connect_toggled(aLink);
    m_xJustify->connect_toggled(aLink);
    m_xLastLineLB->connect_changed(LINK(this, SvxParaAlignTabPage, LastLineHdl_Impl));
    m_xTextDirectionLB->connect_changed(LINK(this, SvxParaAlignTabPage, TextDirectionHdl_Impl));

    m_xTextDirectionLB->append(SvxFrameDirection::Environment, SvxResId(RID_SVXSTR_FRAMEDIR_SUPER));
    m_xTextDirectionLB->append(SvxFrameDirection::Horizontal_LR_TB, SvxResId(RID_SVXSTR_FRAMEDIR_LTR));
    m_xTextDirectionLB->append(SvxFrameDirection::Horizontal_RL_TB, SvxResId(RID_SVXSTR_FRAMEDIR_RTL));
}

SvxParaAlignTabPage::~SvxParaAlignTabPage()
{
}

DeactivateRC SvxParaAlignTabPage::DeactivatePage( SfxItemSet* _pSet )
{
    if ( _pSet )
        FillItemSet( _pSet );
    return DeactivateRC::LeavePage;
}

VclPtr<SfxTabPage> SvxParaAlignTabPage::Create(TabPageParent pParent, const SfxItemSet* rSet)
{
    return VclPtr<SvxParaAlignTabPage>::Create(pParent, *rSet);
}

bool SvxParaAlignTabPage::FillItemSet( SfxItemSet* rOutSet )
{
    bool bModified = false;

    bool bAdj = false;
    SvxAdjust eAdjust = SvxAdjust::Left;

    if (m_xLeft->get_active())
    {
        eAdjust = SvxAdjust::Left;
        bAdj = m_xLeft->get_saved_state() == TRISTATE_FALSE;
    }
    else if (m_xRight->get_active())
    {
        eAdjust = SvxAdjust::Right;
        bAdj = m_xRight->get_saved_state() == TRISTATE_FALSE;
    }
    else if (m_xCenter->get_active())
    {
        eAdjust = SvxAdjust::Center;
        bAdj = m_xCenter->get_saved_state() == TRISTATE_FALSE;
    }
    else if (m_xJustify->get_active())
    {
        eAdjust = SvxAdjust::Block;
        bAdj = m_xJustify->get_saved_state() == TRISTATE_FALSE ||
            m_xExpandCB->get_state_changed_from_saved() ||
            m_xLastLineLB->get_value_changed_from_saved();
    }

    sal_uInt16 _nWhich = GetWhich( SID_ATTR_PARA_ADJUST );

    if (bAdj)
    {
        SvxAdjust eOneWord = m_xExpandCB->get_active() ? SvxAdjust::Block : SvxAdjust::Left;

        int nLBPos = m_xLastLineLB->get_active();
        SvxAdjust eLastBlock = SvxAdjust::Left;
        if ( 1 == nLBPos )
            eLastBlock = SvxAdjust::Center;
        else if ( 2 == nLBPos )
            eLastBlock = SvxAdjust::Block;

        SvxAdjustItem aAdj( static_cast<const SvxAdjustItem&>(GetItemSet().Get( _nWhich )) );
        aAdj.SetAdjust( eAdjust );
        aAdj.SetOneWord( eOneWord );
        aAdj.SetLastBlock( eLastBlock );
        rOutSet->Put( aAdj );
        bModified = true;
    }

    if (m_xSnapToGridCB->get_state_changed_from_saved())
    {
        rOutSet->Put(SvxParaGridItem(m_xSnapToGridCB->get_active(), GetWhich( SID_ATTR_PARA_SNAPTOGRID )));
        bModified = true;
    }

    if (m_xVertAlignLB->get_value_changed_from_saved())
    {
        rOutSet->Put(SvxParaVertAlignItem(static_cast<SvxParaVertAlignItem::Align>(m_xVertAlignLB->get_active()), GetWhich( SID_PARA_VERTALIGN )));
        bModified = true;
    }

    if (m_xTextDirectionLB->get_visible())
    {
        if (m_xTextDirectionLB->get_value_changed_from_saved())
        {
            SvxFrameDirection eDir = m_xTextDirectionLB->get_active_id();
            rOutSet->Put( SvxFrameDirectionItem( eDir, GetWhich( SID_ATTR_FRAMEDIRECTION ) ) );
            bModified = true;
        }
    }

    return bModified;
}

void SvxParaAlignTabPage::ActivatePage( const SfxItemSet& rSet )
{
    Reset( &rSet );
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
            case SvxAdjust::Left: m_xLeft->set_active(true); break;

            case SvxAdjust::Right: m_xRight->set_active(true); break;

            case SvxAdjust::Center: m_xCenter->set_active(true); break;

            case SvxAdjust::Block: m_xJustify->set_active(true); break;
            default: ; //prevent warning
        }
        bool bEnable = m_xJustify->get_active();
        m_xLastLineFT->set_sensitive(bEnable);
        m_xLastLineLB->set_sensitive(bEnable);

        switch(rAdj.GetLastBlock())
        {
            case SvxAdjust::Left:  nLBSelect = 0; break;

            case SvxAdjust::Center: nLBSelect = 1;  break;

            case SvxAdjust::Block: nLBSelect = 2;  break;
            default: ; //prevent warning
        }
        m_xExpandCB->set_sensitive(bEnable && nLBSelect == 2);
        m_xExpandCB->set_active(SvxAdjust::Block == rAdj.GetOneWord());
    }
    else
    {
        m_xLeft->set_active(false);
        m_xRight->set_active(false);
        m_xCenter->set_active(false);
        m_xJustify->set_active(false);
    }
    m_xLastLineLB->set_active(nLBSelect);

    sal_uInt16 nHtmlMode = GetHtmlMode_Impl(*rSet);
    if(nHtmlMode & HTMLMODE_ON)
    {
        m_xLastLineLB->hide();
        m_xLastLineFT->hide();
        m_xExpandCB->hide();
        if(!(nHtmlMode & HTMLMODE_FULL_STYLES) )
            m_xJustify->set_sensitive(false);
        m_xSnapToGridCB->hide();
    }
    _nWhich = GetWhich(SID_ATTR_PARA_SNAPTOGRID);
    eItemState = rSet->GetItemState( _nWhich );
    if ( eItemState >= SfxItemState::DEFAULT )
    {
        const SvxParaGridItem& rSnap = static_cast<const SvxParaGridItem&>(rSet->Get( _nWhich ));
        m_xSnapToGridCB->set_active(rSnap.GetValue());
    }

    _nWhich = GetWhich( SID_PARA_VERTALIGN );
    eItemState = rSet->GetItemState( _nWhich );

    if ( eItemState >= SfxItemState::DEFAULT )
    {
        m_xVertAlignFL->show();

        const SvxParaVertAlignItem& rAlign = static_cast<const SvxParaVertAlignItem&>(rSet->Get( _nWhich ));

        m_xVertAlignLB->set_active(static_cast<sal_Int32>(rAlign.GetValue()));
    }

    _nWhich = GetWhich( SID_ATTR_FRAMEDIRECTION );
    //text direction
    if( SfxItemState::DEFAULT <= rSet->GetItemState( _nWhich ) )
    {
        const SvxFrameDirectionItem& rFrameDirItem = static_cast<const SvxFrameDirectionItem&>( rSet->Get( _nWhich ) );
        m_xTextDirectionLB->set_active_id(rFrameDirItem.GetValue());
        m_xTextDirectionLB->save_value();
    }

    m_xSnapToGridCB->save_state();
    m_xVertAlignLB->save_value();
    m_xLeft->save_state();
    m_xRight->save_state();
    m_xCenter->save_state();
    m_xJustify->save_state();
    m_xLastLineLB->save_value();
    m_xExpandCB->save_state();

    UpdateExample_Impl();
}

void SvxParaAlignTabPage::ChangesApplied()
{
    m_xTextDirectionLB->save_value();
    m_xSnapToGridCB->save_state();
    m_xVertAlignLB->save_value();
    m_xLeft->save_state();
    m_xRight->save_state();
    m_xCenter->save_state();
    m_xJustify->save_state();
    m_xLastLineLB->save_value();
    m_xExpandCB->save_state();
}

IMPL_LINK_NOARG(SvxParaAlignTabPage, AlignHdl_Impl, weld::ToggleButton&, void)
{
    bool bJustify = m_xJustify->get_active();
    m_xLastLineFT->set_sensitive(bJustify);
    m_xLastLineLB->set_sensitive(bJustify);
    bool bLastLineIsBlock = m_xLastLineLB->get_active() == 2;
    m_xExpandCB->set_sensitive(bJustify && bLastLineIsBlock);
    //set last line listbox to entry position 0 if not enabled
    if (!m_xLastLineLB->get_sensitive())
        m_xLastLineLB->set_active(0);
    //uncheck 'Expand ... word' when check box is not enabled
    if (!m_xExpandCB->get_sensitive())
        m_xExpandCB->set_active(false);
    UpdateExample_Impl();
}

IMPL_LINK_NOARG(SvxParaAlignTabPage, LastLineHdl_Impl, weld::ComboBox&, void)
{
    //fdo#41350 only enable 'Expand last word' if last line is also justified
    bool bLastLineIsBlock = m_xLastLineLB->get_active() == 2;
    m_xExpandCB->set_sensitive(bLastLineIsBlock);
    //uncheck 'Expand ... word' when check box is not enabled
    if (!m_xExpandCB->get_sensitive())
        m_xExpandCB->set_active(false);
    UpdateExample_Impl();
}

IMPL_LINK_NOARG(SvxParaAlignTabPage, TextDirectionHdl_Impl, weld::ComboBox&, void)
{
    UpdateExample_Impl();
}

void SvxParaAlignTabPage::UpdateExample_Impl()
{
    if (m_xLeft->get_active())
    {
        m_aExampleWin.EnableRTL(false);
        m_aExampleWin.SetAdjust(SvxAdjust::Left);
        m_aExampleWin.SetLastLine(SvxAdjust::Left);
    }
    else if (m_xRight->get_active())
    {
        m_aExampleWin.EnableRTL(true);
        m_aExampleWin.SetAdjust(SvxAdjust::Left);
        m_aExampleWin.SetLastLine(SvxAdjust::Left);
    }
    else
    {
        SvxFrameDirection eDir = m_xTextDirectionLB->get_active_id();
        switch ( eDir )
        {
            case SvxFrameDirection::Environment :
                if ( !m_xRight->get_active() )
                    m_aExampleWin.EnableRTL( IsRTLEnabled() /*false*/ );
                break;
            case SvxFrameDirection::Horizontal_RL_TB :
                if ( !m_xLeft->get_active() )
                    m_aExampleWin.EnableRTL( true );
                break;
            case SvxFrameDirection::Horizontal_LR_TB :
                if ( !m_xRight->get_active() )
                    m_aExampleWin.EnableRTL( false );
                break;
            default: ; //prevent warning
        }
        if (m_xCenter->get_active())
            m_aExampleWin.SetAdjust( SvxAdjust::Center );
        else if (m_xJustify->get_active())
        {
            m_aExampleWin.SetAdjust( SvxAdjust::Block );
            int nLBPos = m_xLastLineLB->get_active();
            if (nLBPos == 0)
                m_aExampleWin.SetLastLine(SvxAdjust::Left);
            else if (nLBPos == 1)
                m_aExampleWin.SetLastLine(SvxAdjust::Center);
            else if (nLBPos == 2)
                m_aExampleWin.SetLastLine(SvxAdjust::Block);
        }
    }

    m_aExampleWin.Invalidate();
}

void SvxParaAlignTabPage::EnableJustifyExt()
{
    m_xLastLineFT->show();
    m_xLastLineLB->show();
    m_xExpandCB->show();
    SvtLanguageOptions aCJKOptions;
    if (aCJKOptions.IsAsianTypographyEnabled())
        m_xSnapToGridCB->show();

}

void SvxParaAlignTabPage::PageCreated (const SfxAllItemSet& aSet)
{
    const SfxBoolItem* pBoolItem = aSet.GetItem<SfxBoolItem>(SID_SVXPARAALIGNTABPAGE_ENABLEJUSTIFYEXT, false);
    if (pBoolItem && pBoolItem->GetValue())
        EnableJustifyExt();
}

VclPtr<SfxTabPage> SvxExtParagraphTabPage::Create(TabPageParent pParent, const SfxItemSet* rSet)
{
    return VclPtr<SvxExtParagraphTabPage>::Create(pParent, *rSet);
}

bool SvxExtParagraphTabPage::FillItemSet( SfxItemSet* rOutSet )
{
    bool bModified = false;
    sal_uInt16 _nWhich = GetWhich( SID_ATTR_PARA_HYPHENZONE );
    const TriState eHyphenState = m_xHyphenBox->get_state();
    const SfxPoolItem* pOld = GetOldItem( *rOutSet, SID_ATTR_PARA_HYPHENZONE );

    if ( m_xHyphenBox->get_state_changed_from_saved() ||
         m_xExtHyphenBeforeBox->get_value_changed_from_saved() ||
         m_xExtHyphenAfterBox->get_value_changed_from_saved() ||
         m_xMaxHyphenEdit->get_value_changed_from_saved() )
    {
        SvxHyphenZoneItem aHyphen(
            static_cast<const SvxHyphenZoneItem&>(GetItemSet().Get( _nWhich )) );
        aHyphen.SetHyphen( eHyphenState == TRISTATE_TRUE );

        if ( eHyphenState == TRISTATE_TRUE )
        {
            aHyphen.GetMinLead() = static_cast<sal_uInt8>(m_xExtHyphenBeforeBox->get_value());
            aHyphen.GetMinTrail() = static_cast<sal_uInt8>(m_xExtHyphenAfterBox->get_value());
        }
        aHyphen.GetMaxHyphens() = static_cast<sal_uInt8>(m_xMaxHyphenEdit->get_value());

        if ( !pOld ||
            *static_cast<const SvxHyphenZoneItem*>(pOld) != aHyphen ||
                m_xHyphenBox->get_state_changed_from_saved())
        {
            rOutSet->Put( aHyphen );
            bModified = true;
        }
    }

    if (m_xPageNumBox->get_sensitive()
        && (m_xPageNumBox->get_state_changed_from_saved() || m_xPagenumEdit->get_value_changed_from_saved()))
    {
        pOld = GetOldItem( *rOutSet, SID_ATTR_PARA_PAGENUM );

        if (TRISTATE_TRUE == m_xPageNumBox->get_state()
            && (!pOld || IsInvalidItem(pOld)
                || static_cast<const SfxUInt16Item*>(pOld)->GetValue() != m_xPagenumEdit->get_value()))
        {
            SfxUInt16Item aPageNum(SID_ATTR_PARA_PAGENUM,
                    static_cast<sal_uInt16>(m_xPagenumEdit->get_value()));
            rOutSet->Put( aPageNum );
            bModified = true;
        }
        else if (TRISTATE_FALSE == m_xPageNumBox->get_state()
                && (pOld || IsInvalidItem(pOld)))
        {
            // need to tell sw to remove the item
            rOutSet->DisableItem(SID_ATTR_PARA_PAGENUM);
            bModified = true;
        }
    }

    // pagebreak

    TriState eState = m_xApplyCollBtn->get_state();
    bool bIsPageModel = false;

    _nWhich = GetWhich( SID_ATTR_PARA_MODEL );
    OUString sPage;
    if ( m_xApplyCollBtn->get_state_changed_from_saved() ||
         ( TRISTATE_TRUE == eState &&
           m_xApplyCollBox->get_value_changed_from_saved() ) )
    {
        if ( eState == TRISTATE_TRUE )
        {
            sPage = m_xApplyCollBox->get_active_text();
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
    else if(TRISTATE_TRUE == eState && m_xApplyCollBtn->get_sensitive())
        bIsPageModel = true;
    else
        rOutSet->Put( SvxPageModelItem( sPage, false, _nWhich ) );

    _nWhich = GetWhich( SID_ATTR_PARA_PAGEBREAK );

    if ( bIsPageModel )
        // if PageModel is turned on, always turn off PageBreak
        rOutSet->Put( SvxFormatBreakItem( SvxBreak::NONE, _nWhich ) );
    else
    {
        eState = m_xPageBreakBox->get_state();
        SfxItemState eModelState = GetItemSet().GetItemState(SID_ATTR_PARA_MODEL, false);

        if ( (eModelState == SfxItemState::SET && TRISTATE_TRUE == m_xPageBreakBox->get_state()) ||
             m_xPageBreakBox->get_state_changed_from_saved() ||
             m_xBreakTypeLB->get_value_changed_from_saved() ||
             m_xBreakPositionLB->get_value_changed_from_saved() )
        {
            const SvxFormatBreakItem rOldBreak(
                    static_cast<const SvxFormatBreakItem&>(GetItemSet().Get( _nWhich )));
            SvxFormatBreakItem aBreak(rOldBreak.GetBreak(), rOldBreak.Which());

            switch ( eState )
            {
                case TRISTATE_TRUE:
                {
                    bool bBefore = m_xBreakPositionLB->get_active() == 0;

                    if (m_xBreakTypeLB->get_active() == 0)
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

            if ( eState != m_xPageBreakBox->get_saved_state() ||
                    !pOld || !( *static_cast<const SvxFormatBreakItem*>(pOld) == aBreak ) )
            {
                bModified = true;
                rOutSet->Put( aBreak );
            }
        }
    }

    // paragraph split
    _nWhich = GetWhich( SID_ATTR_PARA_SPLIT );
    eState = m_xKeepTogetherBox->get_state();

    if (m_xKeepTogetherBox->get_state_changed_from_saved())
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
    eState = m_xKeepParaBox->get_state();

    if (m_xKeepParaBox->get_state_changed_from_saved())
    {
        // if the status has changed, putting is necessary
        rOutSet->Put( SvxFormatKeepItem( eState == TRISTATE_TRUE, _nWhich ) );
        bModified = true;
    }

    // widows and orphans
    _nWhich = GetWhich( SID_ATTR_PARA_WIDOWS );
    eState = m_xWidowBox->get_state();

    if ( m_xWidowBox->get_state_changed_from_saved() ||
         m_xWidowRowNo->get_value_changed_from_saved() )
    {
        SvxWidowsItem rItem( eState == TRISTATE_TRUE ?
                             static_cast<sal_uInt8>(m_xWidowRowNo->get_value()) : 0, _nWhich );
        pOld = GetOldItem( *rOutSet, SID_ATTR_PARA_WIDOWS );

        if ( m_xWidowBox->get_state_changed_from_saved() || !pOld || !( *static_cast<const SvxWidowsItem*>(pOld) == rItem ) )
        {
            rOutSet->Put( rItem );
            bModified = true;
        }
    }

    _nWhich = GetWhich( SID_ATTR_PARA_ORPHANS );
    eState = m_xOrphanBox->get_state();

    if ( m_xOrphanBox->get_state_changed_from_saved() ||
         m_xOrphanRowNo->get_value_changed_from_saved() )
    {
        SvxOrphansItem rItem( eState == TRISTATE_TRUE ?
                             static_cast<sal_uInt8>(m_xOrphanRowNo->get_value()) : 0, _nWhich );
        pOld = GetOldItem( *rOutSet, SID_ATTR_PARA_ORPHANS );

        if ( m_xOrphanBox->get_state_changed_from_saved() ||
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

        bIsHyphen = rHyphen.IsHyphen();
        m_xHyphenBox->set_state(bIsHyphen ? TRISTATE_TRUE : TRISTATE_FALSE);

        m_xExtHyphenBeforeBox->set_value(rHyphen.GetMinLead());
        m_xExtHyphenAfterBox->set_value(rHyphen.GetMinTrail());
        m_xMaxHyphenEdit->set_value(rHyphen.GetMaxHyphens());
    }
    else
    {
        m_xHyphenBox->set_state(TRISTATE_INDET);
    }
    bool bEnable = bItemAvailable && bIsHyphen;
    m_xExtHyphenBeforeBox->set_sensitive(bEnable);
    m_xExtHyphenAfterBox->set_sensitive(bEnable);
    m_xBeforeText->set_sensitive(bEnable);
    m_xAfterText->set_sensitive(bEnable);
    m_xMaxHyphenLabel->set_sensitive(bEnable);
    m_xMaxHyphenEdit->set_sensitive(bEnable);

    _nWhich = GetWhich( SID_ATTR_PARA_PAGENUM );

    switch (rSet->GetItemState(_nWhich))
    {
        case SfxItemState::SET:
        {
            m_xPageNumBox->set_state(TRISTATE_TRUE);
            SfxUInt16Item const*const pItem(rSet->GetItem<SfxUInt16Item>(_nWhich));
            const sal_uInt16 nPageNum(pItem->GetValue());
            m_xPagenumEdit->set_value(nPageNum);
            break;
        }
        case SfxItemState::DONTCARE:
        {
            m_xPageNumBox->set_state(TRISTATE_INDET);
            break;
        }
        case SfxItemState::UNKNOWN:
        case SfxItemState::DEFAULT:
        case SfxItemState::DISABLED:
        {
            m_xPageNumBox->set_state(TRISTATE_FALSE);
            break;
        }
        default:
            assert(false); // unexpected
            break;
    }

    if ( bPageBreak )
    {
        // first handle PageModel
        _nWhich = GetWhich( SID_ATTR_PARA_MODEL );
        bool bIsPageModel = false;
        eItemState = rSet->GetItemState( _nWhich );

        if ( eItemState >= SfxItemState::SET )
        {
            const SvxPageModelItem& rModel =
                static_cast<const SvxPageModelItem&>(rSet->Get( _nWhich ));
            const OUString& aStr( rModel.GetValue() );

            if (!aStr.isEmpty() && m_xApplyCollBox->find_text(aStr) != -1)
            {
                m_xApplyCollBox->set_active_text(aStr);
                m_xApplyCollBtn->set_state(TRISTATE_TRUE);
                bIsPageModel = true;

                m_xPageBreakBox->set_sensitive(true);
                m_xBreakTypeFT->set_sensitive(true);
                m_xBreakTypeLB->set_sensitive(true);
                m_xBreakPositionFT->set_sensitive(true);
                m_xBreakPositionLB->set_sensitive(true);
                m_xApplyCollBtn->set_sensitive(false);
                m_xPageBreakBox->set_state(TRISTATE_TRUE);

                //select page break
                m_xBreakTypeLB->set_active(0);
                //select break before
                m_xBreakPositionLB->set_active(0);
            }
            else
            {
                m_xApplyCollBox->set_active(-1);
                m_xApplyCollBtn->set_state(TRISTATE_FALSE);
            }
        }
        else if ( SfxItemState::DONTCARE == eItemState )
        {
            m_xApplyCollBtn->set_state(TRISTATE_INDET);
            m_xApplyCollBox->set_active(-1);
        }
        else
        {
            m_xApplyCollBtn->set_sensitive(false);
            m_xApplyCollBox->set_sensitive(false);
            m_xPagenumEdit->set_sensitive(false);
            m_xPageNumBox->set_sensitive(false);
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
                m_xPageBreakBox->set_sensitive(true);
                m_xBreakTypeFT->set_sensitive(true);
                m_xBreakTypeLB->set_sensitive(true);
                m_xBreakPositionFT->set_sensitive(true);
                m_xBreakPositionLB->set_sensitive(true);

                m_xPageBreakBox->set_state(TRISTATE_TRUE);

                bool _bEnable =     eBreak != SvxBreak::NONE &&
                                eBreak != SvxBreak::ColumnBefore &&
                                eBreak != SvxBreak::ColumnAfter;
                m_xApplyCollBtn->set_sensitive(_bEnable);
                if (!_bEnable)
                {
                    m_xApplyCollBox->set_sensitive(_bEnable);
                    m_xPageNumBox->set_sensitive(false);
                    m_xPagenumEdit->set_sensitive(_bEnable);
                }

                if ( eBreak == SvxBreak::NONE )
                    m_xPageBreakBox->set_state(TRISTATE_FALSE);

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
                m_xBreakTypeLB->set_active(nType);
                m_xBreakPositionLB->set_active(nPosition);
            }
            else if ( SfxItemState::DONTCARE == eItemState )
                m_xPageBreakBox->set_state(TRISTATE_INDET);
            else
            {
                m_xPageBreakBox->set_sensitive(false);
                m_xBreakTypeFT->set_sensitive(false);
                m_xBreakTypeLB->set_sensitive(false);
                m_xBreakPositionFT->set_sensitive(false);
                m_xBreakPositionLB->set_sensitive(false);
            }
        }

        PageBreakPosHdl_Impl(*m_xBreakPositionLB);
        PageBreakHdl_Impl(*m_xPageBreakBox);
    }

    _nWhich = GetWhich( SID_ATTR_PARA_KEEP );
    eItemState = rSet->GetItemState( _nWhich );

    if ( eItemState >= SfxItemState::DEFAULT )
    {
        const SvxFormatKeepItem& rKeep =
            static_cast<const SvxFormatKeepItem&>(rSet->Get( _nWhich ));

        if ( rKeep.GetValue() )
            m_xKeepParaBox->set_state(TRISTATE_TRUE);
        else
            m_xKeepParaBox->set_state(TRISTATE_FALSE);
    }
    else if ( SfxItemState::DONTCARE == eItemState )
        m_xKeepParaBox->set_state(TRISTATE_INDET);
    else
        m_xKeepParaBox->set_sensitive(false);

    _nWhich = GetWhich( SID_ATTR_PARA_SPLIT );
    eItemState = rSet->GetItemState( _nWhich );

    if ( eItemState >= SfxItemState::DEFAULT )
    {
        const SvxFormatSplitItem& rSplit =
            static_cast<const SvxFormatSplitItem&>(rSet->Get( _nWhich ));

        if ( !rSplit.GetValue() )
            m_xKeepTogetherBox->set_state(TRISTATE_TRUE);
        else
        {
            m_xKeepTogetherBox->set_state(TRISTATE_FALSE);

            // widows and orphans
            m_xWidowBox->set_sensitive(true);
            _nWhich = GetWhich( SID_ATTR_PARA_WIDOWS );
            SfxItemState eTmpState = rSet->GetItemState( _nWhich );

            if ( eTmpState >= SfxItemState::DEFAULT )
            {
                const SvxWidowsItem& rWidow =
                    static_cast<const SvxWidowsItem&>(rSet->Get( _nWhich ));
                const sal_uInt16 nLines = rWidow.GetValue();

                bool _bEnable = nLines > 0;
                m_xWidowRowNo->set_value(m_xWidowRowNo->normalize(nLines));
                m_xWidowBox->set_state(_bEnable ? TRISTATE_TRUE : TRISTATE_FALSE);
                m_xWidowRowNo->set_sensitive(_bEnable);
                //m_xWidowRowLabel->set_sensitive(_bEnable);

            }
            else if ( SfxItemState::DONTCARE == eTmpState )
                m_xWidowBox->set_state( TRISTATE_INDET );
            else
                m_xWidowBox->set_sensitive(false);

            m_xOrphanBox->set_sensitive(true);
            _nWhich = GetWhich( SID_ATTR_PARA_ORPHANS );
            eTmpState = rSet->GetItemState( _nWhich );

            if ( eTmpState >= SfxItemState::DEFAULT )
            {
                const SvxOrphansItem& rOrphan =
                    static_cast<const SvxOrphansItem&>(rSet->Get( _nWhich ));
                const sal_uInt16 nLines = rOrphan.GetValue();

                bool _bEnable = nLines > 0;
                m_xOrphanBox->set_state(_bEnable ? TRISTATE_TRUE : TRISTATE_FALSE);
                m_xOrphanRowNo->set_value(m_xOrphanRowNo->normalize(nLines));
                m_xOrphanRowNo->set_sensitive(_bEnable);
                m_xOrphanRowLabel->set_sensitive(_bEnable);

            }
            else if ( SfxItemState::DONTCARE == eTmpState )
                m_xOrphanBox->set_state(TRISTATE_INDET);
            else
                m_xOrphanBox->set_sensitive(false);
        }
    }
    else if ( SfxItemState::DONTCARE == eItemState )
        m_xKeepTogetherBox->set_state(TRISTATE_INDET);
    else
        m_xKeepTogetherBox->set_sensitive(false);

    // so that everything is enabled correctly
    KeepTogetherHdl_Impl(*m_xKeepTogetherBox);
    WidowHdl_Impl(*m_xWidowBox);
    OrphanHdl_Impl(*m_xOrphanBox);
    ChangesApplied();
}
void SvxExtParagraphTabPage::ChangesApplied()
{
    m_xHyphenBox->save_state();
    m_xExtHyphenBeforeBox->set_value(m_xExtHyphenBeforeBox->get_value());
    m_xExtHyphenAfterBox->set_value(m_xExtHyphenAfterBox->get_value());
    m_xMaxHyphenEdit->set_value(m_xMaxHyphenEdit->get_value());
    m_xPageBreakBox->save_state();
    m_xBreakPositionLB->save_value();
    m_xBreakTypeLB->save_value();
    m_xApplyCollBtn->save_state();
    m_xApplyCollBox->save_value();
    m_xPageNumBox->save_state();
    m_xPagenumEdit->save_value();
    m_xKeepTogetherBox->save_state();
    m_xKeepParaBox->save_state();
    m_xWidowBox->save_state();
    m_xOrphanBox->save_state();
    m_xOrphanRowNo->set_value(m_xOrphanRowNo->get_value());
    m_xWidowRowNo->set_value(m_xWidowRowNo->get_value());
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
    m_xPageBreakBox->set_sensitive(false);
    m_xBreakTypeLB->remove(0);
    m_xBreakPositionFT->set_sensitive(false);
    m_xBreakPositionLB->set_sensitive(false);
    m_xApplyCollBtn->set_sensitive(false);
    m_xApplyCollBox->set_sensitive(false);
    m_xPageNumBox->set_sensitive(false);
    m_xPagenumEdit->set_sensitive(false);
}

SvxExtParagraphTabPage::SvxExtParagraphTabPage(TabPageParent pParent, const SfxItemSet& rAttr)
    : SfxTabPage(pParent, "cui/ui/textflowpage.ui", "TextFlowPage", &rAttr)
    , bPageBreak(true)
    , bHtmlMode(false)
    , nStdPos(0)
    // Hyphenation
    , m_xHyphenBox(m_xBuilder->weld_check_button("checkAuto"))
    , m_xBeforeText(m_xBuilder->weld_label("labelLineBegin"))
    , m_xExtHyphenBeforeBox(m_xBuilder->weld_spin_button("spinLineEnd"))
    , m_xAfterText(m_xBuilder->weld_label("labelLineEnd"))
    , m_xExtHyphenAfterBox(m_xBuilder->weld_spin_button("spinLineBegin"))
    , m_xMaxHyphenLabel(m_xBuilder->weld_label("labelMaxNum"))
    , m_xMaxHyphenEdit(m_xBuilder->weld_spin_button("spinMaxNum"))
    //Page break
    , m_xPageBreakBox(m_xBuilder->weld_check_button("checkInsert"))
    , m_xBreakTypeFT(m_xBuilder->weld_label("labelType"))
    , m_xBreakTypeLB(m_xBuilder->weld_combo_box("comboBreakType"))
    , m_xBreakPositionFT(m_xBuilder->weld_label("labelPosition"))
    , m_xBreakPositionLB(m_xBuilder->weld_combo_box("comboBreakPosition"))
    , m_xApplyCollBtn(m_xBuilder->weld_check_button("checkPageStyle"))
    , m_xApplyCollBox(m_xBuilder->weld_combo_box("comboPageStyle"))
    , m_xPageNumBox(m_xBuilder->weld_check_button("labelPageNum"))
    , m_xPagenumEdit(m_xBuilder->weld_spin_button("spinPageNumber"))
    // Options
    , m_xKeepTogetherBox(m_xBuilder->weld_check_button("checkSplitPara"))
    , m_xKeepParaBox(m_xBuilder->weld_check_button("checkKeepPara"))
    , m_xOrphanBox(m_xBuilder->weld_check_button("checkOrphan"))
    , m_xOrphanRowNo(m_xBuilder->weld_spin_button("spinOrphan"))
    , m_xOrphanRowLabel(m_xBuilder->weld_label("labelOrphan"))
    , m_xWidowBox(m_xBuilder->weld_check_button("checkWidow"))
    , m_xWidowRowNo(m_xBuilder->weld_spin_button("spinWidow"))
    , m_xWidowRowLabel(m_xBuilder->weld_label("labelWidow"))
{
    // this page needs ExchangeSupport
    SetExchangeSupport();

    m_xHyphenBox->connect_toggled(LINK(this, SvxExtParagraphTabPage, HyphenClickHdl_Impl));
    m_xPageBreakBox->connect_toggled(LINK(this, SvxExtParagraphTabPage, PageBreakHdl_Impl));
    m_xKeepTogetherBox->connect_toggled(LINK(this, SvxExtParagraphTabPage, KeepTogetherHdl_Impl));
    m_xWidowBox->connect_toggled(LINK(this, SvxExtParagraphTabPage, WidowHdl_Impl));
    m_xOrphanBox->connect_toggled(LINK(this, SvxExtParagraphTabPage, OrphanHdl_Impl));
    m_xApplyCollBtn->connect_toggled(LINK(this, SvxExtParagraphTabPage, ApplyCollClickHdl_Impl));
    m_xBreakTypeLB->connect_changed(LINK(this, SvxExtParagraphTabPage, PageBreakTypeHdl_Impl));
    m_xBreakPositionLB->connect_changed(LINK(this, SvxExtParagraphTabPage, PageBreakPosHdl_Impl));
    m_xPageNumBox->connect_toggled(LINK(this, SvxExtParagraphTabPage, PageNumBoxClickHdl_Impl));

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
            m_xApplyCollBox->append_text(pStyle->GetName());
            pStyle = pPool->Next();
        }
        nStdPos = m_xApplyCollBox->find_text(aStdName);
    }

    sal_uInt16 nHtmlMode = GetHtmlMode_Impl( rAttr );
    if ( nHtmlMode & HTMLMODE_ON )
    {
        bHtmlMode = true;
        m_xHyphenBox->set_sensitive(false);
        m_xBeforeText->set_sensitive(false);
        m_xExtHyphenBeforeBox->set_sensitive(false);
        m_xAfterText->set_sensitive(false);
        m_xExtHyphenAfterBox->set_sensitive(false);
        m_xMaxHyphenLabel->set_sensitive(false);
        m_xMaxHyphenEdit->set_sensitive(false);
        m_xPageNumBox->set_sensitive(false);
        m_xPagenumEdit->set_sensitive(false);
        // no column break in HTML
        m_xBreakTypeLB->remove(1);
    }
}

SvxExtParagraphTabPage::~SvxExtParagraphTabPage()
{
}

IMPL_LINK_NOARG(SvxExtParagraphTabPage, PageBreakHdl_Impl, weld::ToggleButton&, void)
{
    switch (m_xPageBreakBox->get_state())
    {
        case TRISTATE_TRUE:
            m_xBreakTypeFT->set_sensitive(true);
            m_xBreakTypeLB->set_sensitive(true);
            m_xBreakPositionFT->set_sensitive(true);
            m_xBreakPositionLB->set_sensitive(true);

            if (0 == m_xBreakTypeLB->get_active() && 0 == m_xBreakPositionLB->get_active())
            {
                m_xApplyCollBtn->set_sensitive(true);

                bool bEnable = TRISTATE_TRUE == m_xApplyCollBtn->get_state() &&
                                            m_xApplyCollBox->get_count();
                m_xApplyCollBox->set_sensitive(bEnable);
                if(!bHtmlMode)
                {
                    m_xPageNumBox->set_sensitive(bEnable);
                    m_xPagenumEdit->set_sensitive(bEnable && m_xPageNumBox->get_state() == TRISTATE_TRUE);
                }
            }
            break;

        case TRISTATE_FALSE:
        case TRISTATE_INDET:
            m_xApplyCollBtn->set_state(TRISTATE_FALSE);
            m_xApplyCollBtn->set_sensitive(false);
            m_xApplyCollBox->set_sensitive(false);
            m_xPageNumBox->set_sensitive(false);
            m_xPagenumEdit->set_sensitive(false);
            m_xBreakTypeFT->set_sensitive(false);
            m_xBreakTypeLB->set_sensitive(false);
            m_xBreakPositionFT->set_sensitive(false);
            m_xBreakPositionLB->set_sensitive(false);
            break;
    }
}

IMPL_LINK_NOARG(SvxExtParagraphTabPage, KeepTogetherHdl_Impl, weld::ToggleButton&, void)
{
    bool bEnable = m_xKeepTogetherBox->get_state() == TRISTATE_FALSE;
    m_xWidowBox->set_sensitive(bEnable);
    m_xOrphanBox->set_sensitive(bEnable);
}

IMPL_LINK_NOARG(SvxExtParagraphTabPage, WidowHdl_Impl, weld::ToggleButton&, void)
{
    switch (m_xWidowBox->get_state())
    {
        case TRISTATE_TRUE:
            m_xWidowRowNo->set_sensitive(true);
            m_xWidowRowLabel->set_sensitive(true);
            m_xKeepTogetherBox->set_sensitive(false);
            break;
        case TRISTATE_FALSE:
            if (m_xOrphanBox->get_state() == TRISTATE_FALSE)
                m_xKeepTogetherBox->set_sensitive(true);
            [[fallthrough]];
        case TRISTATE_INDET:
            m_xWidowRowNo->set_sensitive(false);
            m_xWidowRowLabel->set_sensitive(false);
            break;
    }
}

IMPL_LINK_NOARG(SvxExtParagraphTabPage, OrphanHdl_Impl, weld::ToggleButton&, void)
{
    switch (m_xOrphanBox->get_state())
    {
        case TRISTATE_TRUE:
            m_xOrphanRowNo->set_sensitive(true);
            m_xOrphanRowLabel->set_sensitive(true);
            m_xKeepTogetherBox->set_sensitive(false);
            break;

        case TRISTATE_FALSE:
            if (m_xWidowBox->get_state() == TRISTATE_FALSE)
                m_xKeepTogetherBox->set_sensitive(true);
            [[fallthrough]];
        case TRISTATE_INDET:
            m_xOrphanRowNo->set_sensitive(false);
            m_xOrphanRowLabel->set_sensitive(false);
            break;
    }
}

IMPL_LINK_NOARG(SvxExtParagraphTabPage, HyphenClickHdl_Impl, weld::ToggleButton&, void)
{
    bool bEnable = m_xHyphenBox->get_state() == TRISTATE_TRUE;
    m_xBeforeText->set_sensitive(bEnable);
    m_xExtHyphenBeforeBox->set_sensitive(bEnable);
    m_xAfterText->set_sensitive(bEnable);
    m_xExtHyphenAfterBox->set_sensitive(bEnable);
    m_xMaxHyphenLabel->set_sensitive(bEnable);
    m_xMaxHyphenEdit->set_sensitive(bEnable);
    m_xHyphenBox->set_state(bEnable ? TRISTATE_TRUE : TRISTATE_FALSE);
}

IMPL_LINK_NOARG(SvxExtParagraphTabPage, ApplyCollClickHdl_Impl, weld::ToggleButton&, void)
{
    bool bEnable = false;
    if (m_xApplyCollBtn->get_state() == TRISTATE_TRUE && m_xApplyCollBox->get_count())
    {
        bEnable = true;
        m_xApplyCollBox->set_active(nStdPos);
    }
    else
    {
        m_xApplyCollBox->set_active(-1);
    }
    m_xApplyCollBox->set_sensitive(bEnable);
    if (!bHtmlMode)
    {
        m_xPageNumBox->set_sensitive(bEnable);
        m_xPagenumEdit->set_sensitive(bEnable && m_xPageNumBox->get_state() == TRISTATE_TRUE);
    }
}

IMPL_LINK(SvxExtParagraphTabPage, PageBreakPosHdl_Impl, weld::ComboBox&, rListBox, void)
{
    if (0 == rListBox.get_active())
    {
        m_xApplyCollBtn->set_sensitive(true);

        bool bEnable = m_xApplyCollBtn->get_state() == TRISTATE_TRUE && m_xApplyCollBox->get_count();

        m_xApplyCollBox->set_sensitive(bEnable);
        if (!bHtmlMode)
        {
            m_xPageNumBox->set_sensitive(bEnable);
            m_xPagenumEdit->set_sensitive(bEnable && m_xPageNumBox->get_state() == TRISTATE_TRUE);
        }
    }
    else if (1 == rListBox.get_active())
    {
        m_xApplyCollBtn->set_state(TRISTATE_FALSE);
        m_xApplyCollBtn->set_sensitive(false);
        m_xApplyCollBox->set_sensitive(false);
        m_xPageNumBox->set_sensitive(false);
        m_xPagenumEdit->set_sensitive(false);
    }
}

IMPL_LINK(SvxExtParagraphTabPage, PageBreakTypeHdl_Impl, weld::ComboBox&, rListBox, void)
{
    //column break or break after
    int nBreakPos = m_xBreakPositionLB->get_active();
    if (rListBox.get_active() == 1 || 1 == nBreakPos)
    {
        m_xApplyCollBtn->set_state(TRISTATE_FALSE);
        m_xApplyCollBtn->set_sensitive(false);
        m_xApplyCollBox->set_sensitive(false);
        m_xPageNumBox->set_sensitive(false);
        m_xPagenumEdit->set_sensitive(false);
    }
    else
        PageBreakPosHdl_Impl(*m_xBreakPositionLB);
}

IMPL_LINK_NOARG(SvxExtParagraphTabPage, PageNumBoxClickHdl_Impl, weld::ToggleButton&, void)
{
    m_xPagenumEdit->set_sensitive(m_xPageNumBox->get_state() == TRISTATE_TRUE);
}

void SvxExtParagraphTabPage::PageCreated(const SfxAllItemSet& aSet)
{
    const SfxBoolItem* pDisablePageBreakItem = aSet.GetItem<SfxBoolItem>(SID_DISABLE_SVXEXTPARAGRAPHTABPAGE_PAGEBREAK, false);

    if (pDisablePageBreakItem)
        if ( pDisablePageBreakItem->GetValue())
                    DisablePageBreak();
}

SvxAsianTabPage::SvxAsianTabPage(TabPageParent pParent, const SfxItemSet& rSet)
    : SfxTabPage(pParent, "cui/ui/asiantypography.ui", "AsianTypography", &rSet)
    , m_xForbiddenRulesCB(m_xBuilder->weld_check_button("checkForbidList"))
    , m_xHangingPunctCB(m_xBuilder->weld_check_button("checkHangPunct"))
    , m_xScriptSpaceCB(m_xBuilder->weld_check_button("checkApplySpacing"))
{
}

SvxAsianTabPage::~SvxAsianTabPage()
{
}

VclPtr<SfxTabPage> SvxAsianTabPage::Create(TabPageParent pParent, const SfxItemSet* rSet)
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
    if (m_xScriptSpaceCB->get_sensitive() && m_xScriptSpaceCB->get_state_changed_from_saved())
    {
        std::unique_ptr<SfxBoolItem> pNewItem(static_cast<SfxBoolItem*>(rSet->Get(
            pPool->GetWhich(SID_ATTR_PARA_SCRIPTSPACE)).Clone()));
        pNewItem->SetValue(m_xScriptSpaceCB->get_active());
        rSet->Put(*pNewItem);
        bRet = true;
    }
    if (m_xHangingPunctCB->get_sensitive() && m_xHangingPunctCB->get_state_changed_from_saved())
    {
        std::unique_ptr<SfxBoolItem> pNewItem(static_cast<SfxBoolItem*>(rSet->Get(
            pPool->GetWhich(SID_ATTR_PARA_HANGPUNCTUATION)).Clone()));
        pNewItem->SetValue(m_xHangingPunctCB->get_active());
        rSet->Put(*pNewItem);
        bRet = true;
    }
    if (m_xForbiddenRulesCB->get_sensitive() && m_xForbiddenRulesCB->get_state_changed_from_saved())
    {
        std::unique_ptr<SfxBoolItem> pNewItem(static_cast<SfxBoolItem*>(rSet->Get(
            pPool->GetWhich(SID_ATTR_PARA_FORBIDDEN_RULES)).Clone()));
        pNewItem->SetValue(m_xForbiddenRulesCB->get_active());
        rSet->Put(*pNewItem);
        bRet = true;
    }
    return bRet;
}

static void lcl_SetBox(const SfxItemSet& rSet, sal_uInt16 nSlotId, weld::CheckButton& rBox)
{
    sal_uInt16 _nWhich = rSet.GetPool()->GetWhich(nSlotId);
    SfxItemState eState = rSet.GetItemState(_nWhich);
    if( eState == SfxItemState::UNKNOWN || eState == SfxItemState::DISABLED )
        rBox.set_sensitive(false);
    else if(eState >= SfxItemState::DEFAULT)
        rBox.set_active(static_cast<const SfxBoolItem&>(rSet.Get(_nWhich)).GetValue());
    else
        rBox.set_state(TRISTATE_INDET);
    rBox.save_state();
}

void SvxAsianTabPage::Reset( const SfxItemSet* rSet )
{
    lcl_SetBox(*rSet, SID_ATTR_PARA_FORBIDDEN_RULES, *m_xForbiddenRulesCB );
    lcl_SetBox(*rSet, SID_ATTR_PARA_HANGPUNCTUATION, *m_xHangingPunctCB );

    //character distance not yet available
    lcl_SetBox(*rSet, SID_ATTR_PARA_SCRIPTSPACE, *m_xScriptSpaceCB );
}

void SvxAsianTabPage::ChangesApplied()
{
    m_xForbiddenRulesCB->save_state();
    m_xHangingPunctCB->save_state();
    m_xScriptSpaceCB->save_state();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
