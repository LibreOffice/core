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

#include <sal/config.h>

#include <memory>

#include <hintids.hxx>
#include <sfx2/request.hxx>
#include <unotools/useroptions.hxx>
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <svx/colorwindow.hxx>
#include <svx/dataaccessdescriptor.hxx>
#include <editeng/editids.hrc>
#include <editeng/wghtitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/crossedoutitem.hxx>
#include <editeng/cmapitem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/brushitem.hxx>
#include <svl/cjkoptions.hxx>
#include <osl/diagnose.h>
#include <swmodule.hxx>
#include <swtypes.hxx>
#include <usrpref.hxx>
#include <modcfg.hxx>
#include <view.hxx>
#include <pview.hxx>
#include <wview.hxx>
#include <wrtsh.hxx>
#include <docsh.hxx>
#include <uinums.hxx>
#include <prtopt.hxx>
#include <navicfg.hxx>
#include <doc.hxx>
#include <strings.hrc>
#include <IDocumentLayoutAccess.hxx>

#include <tools/color.hxx>
#include <tools/json_writer.hxx>
#include <PostItMgr.hxx>

using namespace ::svx;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::view;
using namespace ::com::sun::star::lang;

static void lcl_SetUIPrefs(const SwViewOption &rPref, SwView* pView, SwViewShell* pSh )
{
    // in FrameSets the actual visibility can differ from the ViewOption's setting
    bool bVScrollChanged = rPref.IsViewVScrollBar() != pSh->GetViewOptions()->IsViewVScrollBar();
    bool bHScrollChanged = rPref.IsViewHScrollBar() != pSh->GetViewOptions()->IsViewHScrollBar();
    bool bVAlignChanged = rPref.IsVRulerRight() != pSh->GetViewOptions()->IsVRulerRight();

    pSh->SetUIOptions(rPref);
    const SwViewOption* pNewPref = pSh->GetViewOptions();

    // Scrollbars on / off
    if(bVScrollChanged)
    {
        pView->EnableVScrollbar(pNewPref->IsViewVScrollBar());
    }
    if(bHScrollChanged)
    {
        pView->EnableHScrollbar( pNewPref->IsViewHScrollBar() || pNewPref->getBrowseMode() );
    }
    //if only the position of the vertical ruler has been changed initiate an update
    if(bVAlignChanged && !bHScrollChanged && !bVScrollChanged)
        pView->InvalidateBorder();

    // Rulers on / off
    if(pNewPref->IsViewVRuler())
        pView->CreateVRuler();
    else
        pView->KillVRuler();

    // TabWindow on / off
    if(pNewPref->IsViewHRuler())
        pView->CreateTab();
    else
        pView->KillTab();

    pView->GetPostItMgr()->PrepareView(true);
}

SwWrtShell* GetActiveWrtShell()
{
    SwView *pActive = ::GetActiveView();
    if( pActive )
        return &pActive->GetWrtShell();
    return nullptr;
}

SwView* GetActiveView()
{
    SfxViewShell* pView = SfxViewShell::Current();
    return dynamic_cast<SwView*>( pView  );
}

SwView* SwModule::GetFirstView()
{
    // returns only visible SwView
    SwView* pView = static_cast<SwView*>(SfxViewShell::GetFirst(true, checkSfxViewShell<SwView>));
    return pView;
}

SwView* SwModule::GetNextView(SwView const * pView)
{
    OSL_ENSURE( pView,"return no SwView" );
    SwView* pNView = static_cast<SwView*>(SfxViewShell::GetNext(*pView, true, checkSfxViewShell<SwView>));
    return pNView;
}

// New Master for the settings is set; this affects the current view and all following.
void SwModule::ApplyUsrPref(const SwViewOption &rUsrPref, SwView* pActView,
                            SvViewOpt nDest )
{
    SwView* pCurrView = pActView;
    SwViewShell* pSh = pCurrView ? &pCurrView->GetWrtShell() : nullptr;

    SwMasterUsrPref* pPref = const_cast<SwMasterUsrPref*>(GetUsrPref(
                                         nDest == SvViewOpt::DestWeb
                                         || (nDest != SvViewOpt::DestText
                                             && dynamic_cast< const SwWebView *>( pCurrView )) ));

    // with Uno, only sdbcx::View, but not the Module should be changed
    bool bViewOnly = SvViewOpt::DestViewOnly == nDest;
    // fob Preview off
    SwPagePreview* pPPView;
    if( !pCurrView && nullptr != (pPPView = dynamic_cast<SwPagePreview*>( SfxViewShell::Current()))  )
    {
        if(!bViewOnly)
            pPref->SetUIOptions( rUsrPref );
        pPPView->EnableVScrollbar(pPref->IsViewVScrollBar());
        pPPView->EnableHScrollbar(pPref->IsViewHScrollBar());
        if(!bViewOnly)
        {
            pPref->SetPagePrevRow(rUsrPref.GetPagePrevRow());
            pPref->SetPagePrevCol(rUsrPref.GetPagePrevCol());
        }
        return;
    }

    if(!bViewOnly)
    {
        pPref->SetUsrPref( rUsrPref );
        pPref->SetModified();
    }

    if( !pCurrView )
        return;

    // Passing on to CORE
    bool bReadonly;
    const SwDocShell* pDocSh = pCurrView->GetDocShell();
    if (pDocSh)
        bReadonly = pDocSh->IsReadOnly();
    else //Use existing option if DocShell missing
        bReadonly = pSh->GetViewOptions()->IsReadonly();
    std::unique_ptr<SwViewOption> xViewOpt;
    if (!bViewOnly)
        xViewOpt.reset(new SwViewOption(*pPref));
    else
        xViewOpt.reset(new SwViewOption(rUsrPref));
    xViewOpt->SetReadonly( bReadonly );
    if( !(*pSh->GetViewOptions() == *xViewOpt) )
    {
        //is maybe only a SwViewShell
        pSh->StartAction();
        pSh->ApplyViewOptions( *xViewOpt );
        static_cast<SwWrtShell*>(pSh)->SetReadOnlyAvailable(xViewOpt->IsCursorInProtectedArea());
        pSh->EndAction();
    }
    if ( pSh->GetViewOptions()->IsReadonly() != bReadonly )
        pSh->SetReadonlyOption(bReadonly);

    lcl_SetUIPrefs(*xViewOpt, pCurrView, pSh);

    // in the end the Idle-Flag is set again
    pPref->SetIdle(true);
}

void SwModule::ApplyUserMetric( FieldUnit eMetric, bool bWeb )
{
        SwMasterUsrPref* pPref;
        if(bWeb)
        {
            if(!m_pWebUsrPref)
                GetUsrPref(true);
            pPref = m_pWebUsrPref.get();
        }
        else
        {
            if(!m_pUsrPref)
                GetUsrPref(false);
            pPref = m_pUsrPref.get();
        }
        FieldUnit eOldMetric = pPref->GetMetric();
        if(eOldMetric != eMetric)
            pPref->SetMetric(eMetric);

        FieldUnit eHScrollMetric = pPref->IsHScrollMetric() ? pPref->GetHScrollMetric() : eMetric;
        FieldUnit eVScrollMetric = pPref->IsVScrollMetric() ? pPref->GetVScrollMetric() : eMetric;

        SwView* pTmpView = SwModule::GetFirstView();
        // switch the ruler for all MDI-Windows
        while(pTmpView)
        {
            if(bWeb == (dynamic_cast<SwWebView*>( pTmpView) !=  nullptr) )
            {
                pTmpView->ChangeVRulerMetric(eVScrollMetric);
                pTmpView->ChangeTabMetric(eHScrollMetric);
            }

            pTmpView = SwModule::GetNextView(pTmpView);
        }
}

void SwModule::ApplyRulerMetric( FieldUnit eMetric, bool bHorizontal, bool bWeb )
{
    SwMasterUsrPref* pPref;
    if(bWeb)
    {
        if(!m_pWebUsrPref)
            GetUsrPref(true);
        pPref = m_pWebUsrPref.get();
    }
    else
    {
        if(!m_pUsrPref)
            GetUsrPref(false);
        pPref = m_pUsrPref.get();
    }
    if( bHorizontal )
        pPref->SetHScrollMetric(eMetric);
    else
        pPref->SetVScrollMetric(eMetric);

    SwView* pTmpView = SwModule::GetFirstView();
    // switch metric at the appropriate rulers
    while(pTmpView)
    {
        if(bWeb == (dynamic_cast<SwWebView *>( pTmpView ) !=  nullptr))
        {
            if( bHorizontal )
                pTmpView->ChangeTabMetric(eMetric);
            else
                pTmpView->ChangeVRulerMetric(eMetric);
        }
        pTmpView = SwModule::GetNextView(pTmpView);
    }
}

//set the usrpref 's char unit attribute and set rulers unit as char if the "apply char unit" is checked
void SwModule::ApplyUserCharUnit(bool bApplyChar, bool bWeb)
{
    SwMasterUsrPref* pPref;
    if(bWeb)
    {
        if(!m_pWebUsrPref)
            GetUsrPref(true);
        pPref = m_pWebUsrPref.get();
    }
    else
    {
        if(!m_pUsrPref)
            GetUsrPref(false);
        pPref = m_pUsrPref.get();
    }
    bool bOldApplyCharUnit = pPref->IsApplyCharUnit();
    bool bHasChanged = false;
    if(bOldApplyCharUnit != bApplyChar)
    {
        pPref->SetApplyCharUnit(bApplyChar);
        bHasChanged = true;
    }

    if( !bHasChanged )
        return;

    FieldUnit eHScrollMetric = pPref->IsHScrollMetric() ? pPref->GetHScrollMetric() : pPref->GetMetric();
    FieldUnit eVScrollMetric = pPref->IsVScrollMetric() ? pPref->GetVScrollMetric() : pPref->GetMetric();
    if(bApplyChar)
    {
        eHScrollMetric = FieldUnit::CHAR;
        eVScrollMetric = FieldUnit::LINE;
    }
    else
    {
        SvtCJKOptions aCJKOptions;
        if ( !aCJKOptions.IsAsianTypographyEnabled() && ( eHScrollMetric == FieldUnit::CHAR ))
            eHScrollMetric = FieldUnit::INCH;
        else if ( eHScrollMetric == FieldUnit::CHAR )
            eHScrollMetric = FieldUnit::CM;
        if ( !aCJKOptions.IsAsianTypographyEnabled() && ( eVScrollMetric == FieldUnit::LINE ))
            eVScrollMetric = FieldUnit::INCH;
        else if ( eVScrollMetric == FieldUnit::LINE )
            eVScrollMetric = FieldUnit::CM;
    }
    SwView* pTmpView = SwModule::GetFirstView();
    // switch rulers for all MDI-Windows
    while(pTmpView)
    {
        if(bWeb == (dynamic_cast<SwWebView*>( pTmpView) !=  nullptr) )
        {
            pTmpView->ChangeVRulerMetric(eVScrollMetric);
            pTmpView->ChangeTabMetric(eHScrollMetric);
        }

        pTmpView = SwModule::GetNextView(pTmpView);
    }
}

SwNavigationConfig*  SwModule::GetNavigationConfig()
{
    if(!m_pNavigationConfig)
    {
        m_pNavigationConfig.reset( new SwNavigationConfig );
    }
    return m_pNavigationConfig.get();
}

SwPrintOptions*     SwModule::GetPrtOptions(bool bWeb)
{
    if(bWeb && !m_pWebPrintOptions)
    {
        m_pWebPrintOptions.reset(new SwPrintOptions(true));
    }
    else if(!bWeb && !m_pPrintOptions)
    {
        m_pPrintOptions.reset(new SwPrintOptions(false));
    }

    return bWeb ? m_pWebPrintOptions.get() : m_pPrintOptions.get();
}

SwChapterNumRules*  SwModule::GetChapterNumRules()
{
    if(!m_pChapterNumRules)
        m_pChapterNumRules.reset(new SwChapterNumRules);
    return m_pChapterNumRules.get();
}

void SwModule::ShowDBObj(SwView const & rView, const SwDBData& rData)
{
    Reference<XFrame> xFrame = rView.GetViewFrame()->GetFrame().GetFrameInterface();

    uno::Reference<XFrame> xBeamerFrame = xFrame->findFrame("_beamer", FrameSearchFlag::CHILDREN);
    if (!xBeamerFrame.is())
        return;

// the beamer has been opened by the SfxViewFrame
    Reference<XController> xController = xBeamerFrame->getController();
    Reference<XSelectionSupplier> xControllerSelection(xController, UNO_QUERY);
    if (xControllerSelection.is())
    {

        ODataAccessDescriptor aSelection;
        aSelection.setDataSource(rData.sDataSource);
        aSelection[DataAccessDescriptorProperty::Command]       <<= rData.sCommand;
        aSelection[DataAccessDescriptorProperty::CommandType]   <<= rData.nCommandType;
        xControllerSelection->select(makeAny(aSelection.createPropertyValueSequence()));
    }
    else {
        OSL_FAIL("no selection supplier in the beamer!");
    }
}

std::size_t SwModule::GetRedlineAuthor()
{
    if (!m_bAuthorInitialised)
    {
        const SvtUserOptions& rOpt = GetUserOptions();
        m_sActAuthor = rOpt.GetFullName();
        if (m_sActAuthor.isEmpty())
        {
            m_sActAuthor = rOpt.GetID();
            if (m_sActAuthor.isEmpty())
                m_sActAuthor = SwResId( STR_REDLINE_UNKNOWN_AUTHOR );
        }
        m_bAuthorInitialised = true;
    }
    return InsertRedlineAuthor( m_sActAuthor );
}

void SwModule::SetRedlineAuthor(const OUString &rAuthor)
{
    m_bAuthorInitialised = true;
    m_sActAuthor = rAuthor;
    InsertRedlineAuthor( m_sActAuthor );
}

OUString const & SwModule::GetRedlineAuthor(std::size_t nPos)
{
    OSL_ENSURE(nPos < m_pAuthorNames.size(), "author not found!"); //#i45342# RTF doc with no author table caused reader to crash
    while(nPos >= m_pAuthorNames.size())
    {
        InsertRedlineAuthor("nn");
    }
    return m_pAuthorNames[nPos];
}

static Color lcl_GetAuthorColor(std::size_t nPos)
{
    static const Color aColArr[] =
    {
        COL_AUTHOR1_DARK, COL_AUTHOR2_DARK, COL_AUTHOR3_DARK,
        COL_AUTHOR4_DARK, COL_AUTHOR5_DARK, COL_AUTHOR6_DARK,
        COL_AUTHOR7_DARK, COL_AUTHOR8_DARK, COL_AUTHOR9_DARK
    };

    return aColArr[nPos % SAL_N_ELEMENTS(aColArr)];
}

/// Returns a JSON representation of a redline author.
void SwModule::GetRedlineAuthorInfo(tools::JsonWriter& rJsonWriter)
{
    auto authorsNode = rJsonWriter.startArray("authors");
    for (std::size_t nAuthor = 0; nAuthor < m_pAuthorNames.size(); ++nAuthor)
    {
        auto authorNode = rJsonWriter.startStruct();
        rJsonWriter.put("index", static_cast<sal_Int64>(nAuthor));
        rJsonWriter.put("name", m_pAuthorNames[nAuthor]);
        rJsonWriter.put("color", sal_uInt32(lcl_GetAuthorColor(nAuthor)));
    }
}

std::size_t SwModule::InsertRedlineAuthor(const OUString& rAuthor)
{
    std::size_t nPos = 0;

    while(nPos < m_pAuthorNames.size() && m_pAuthorNames[nPos] != rAuthor)
        ++nPos;

    if (nPos == m_pAuthorNames.size())
        m_pAuthorNames.push_back(rAuthor);

    return nPos;
}

static void lcl_FillAuthorAttr( std::size_t nAuthor, SfxItemSet &rSet,
                        const AuthorCharAttr &rAttr )
{
    Color aCol( rAttr.m_nColor );

    if( rAttr.m_nColor == COL_TRANSPARENT )
        aCol = lcl_GetAuthorColor(nAuthor);

    bool bBackGr = rAttr.m_nColor == COL_NONE_COLOR;

    switch (rAttr.m_nItemId)
    {
    case SID_ATTR_CHAR_WEIGHT:
        {
            SvxWeightItem aW( static_cast<FontWeight>(rAttr.m_nAttr), RES_CHRATR_WEIGHT );
            rSet.Put( aW );
            aW.SetWhich( RES_CHRATR_CJK_WEIGHT );
            rSet.Put( aW );
            aW.SetWhich( RES_CHRATR_CTL_WEIGHT );
            rSet.Put( aW );
        }
        break;

    case SID_ATTR_CHAR_POSTURE:
        {
            SvxPostureItem aP( static_cast<FontItalic>(rAttr.m_nAttr), RES_CHRATR_POSTURE );
            rSet.Put( aP );
            aP.SetWhich( RES_CHRATR_CJK_POSTURE );
            rSet.Put( aP );
            aP.SetWhich( RES_CHRATR_CTL_POSTURE );
            rSet.Put( aP );
        }
        break;

    case SID_ATTR_CHAR_UNDERLINE:
        rSet.Put( SvxUnderlineItem( static_cast<FontLineStyle>(rAttr.m_nAttr),
                                    RES_CHRATR_UNDERLINE));
        break;

    case SID_ATTR_CHAR_STRIKEOUT:
        rSet.Put(SvxCrossedOutItem( static_cast<FontStrikeout>(rAttr.m_nAttr),
                                    RES_CHRATR_CROSSEDOUT));
        break;

    case SID_ATTR_CHAR_CASEMAP:
        rSet.Put( SvxCaseMapItem( static_cast<SvxCaseMap>(rAttr.m_nAttr),
                                    RES_CHRATR_CASEMAP));
        break;

    case SID_ATTR_BRUSH:
        rSet.Put( SvxBrushItem( aCol, RES_CHRATR_BACKGROUND ));
        bBackGr = true;
        break;
    }

    if( !bBackGr )
        rSet.Put( SvxColorItem( aCol, RES_CHRATR_COLOR ) );
}

void SwModule::GetInsertAuthorAttr(std::size_t nAuthor, SfxItemSet &rSet)
{
    lcl_FillAuthorAttr(nAuthor, rSet, m_pModuleConfig->GetInsertAuthorAttr());
}

void SwModule::GetDeletedAuthorAttr(std::size_t nAuthor, SfxItemSet &rSet)
{
    lcl_FillAuthorAttr(nAuthor, rSet, m_pModuleConfig->GetDeletedAuthorAttr());
}

// For future extension:
void SwModule::GetFormatAuthorAttr( std::size_t nAuthor, SfxItemSet &rSet )
{
    lcl_FillAuthorAttr( nAuthor, rSet, m_pModuleConfig->GetFormatAuthorAttr() );
}

sal_uInt16 SwModule::GetRedlineMarkPos() const
{
    return m_pModuleConfig->GetMarkAlignMode();
}

bool SwModule::IsInsTableFormatNum(bool bHTML) const
{
    return m_pModuleConfig->IsInsTableFormatNum(bHTML);
}

bool SwModule::IsInsTableChangeNumFormat(bool bHTML) const
{
    return m_pModuleConfig->IsInsTableChangeNumFormat(bHTML);
}

bool SwModule::IsInsTableAlignNum(bool bHTML) const
{
    return m_pModuleConfig->IsInsTableAlignNum(bHTML);
}

bool SwModule::IsSplitVerticalByDefault(bool bHTML) const
{
    return m_pModuleConfig->IsSplitVerticalByDefault(bHTML);
}

void SwModule::SetSplitVerticalByDefault(bool bHTML, bool value)
{
    m_pModuleConfig->SetSplitVerticalByDefault(bHTML, value);
}

const Color &SwModule::GetRedlineMarkColor() const
{
    return m_pModuleConfig->GetMarkAlignColor();
}

const SwViewOption* SwModule::GetViewOption(bool bWeb)
{
    return GetUsrPref( bWeb );
}

OUString const & SwModule::GetDocStatWordDelim() const
{
    return m_pModuleConfig->GetWordDelimiter();
}

// Passing-through of the ModuleConfig's Metric (for HTML-Export)
FieldUnit SwModule::GetMetric( bool bWeb ) const
{
    SwMasterUsrPref* pPref;
    if(bWeb)
    {
        if(!m_pWebUsrPref)
            GetUsrPref(true);
        pPref = m_pWebUsrPref.get();
    }
    else
    {
        if(!m_pUsrPref)
            GetUsrPref(false);
        pPref = m_pUsrPref.get();
    }
    return pPref->GetMetric();
}

// Pass-through Update-Status
sal_uInt16 SwModule::GetLinkUpdMode() const
{
    if(!m_pUsrPref)
        GetUsrPref(false);
    return static_cast<sal_uInt16>(m_pUsrPref->GetUpdateLinkMode());
}

SwFieldUpdateFlags SwModule::GetFieldUpdateFlags() const
{
    if(!m_pUsrPref)
        GetUsrPref(false);
    return m_pUsrPref->GetFieldUpdateFlags();
}

void SwModule::ApplyFieldUpdateFlags(SwFieldUpdateFlags eFieldFlags)
{
    if(!m_pUsrPref)
        GetUsrPref(false);
    m_pUsrPref->SetFieldUpdateFlags(eFieldFlags);
}

void SwModule::ApplyLinkMode(sal_Int32 nNewLinkMode)
{
    if(!m_pUsrPref)
        GetUsrPref(false);
    m_pUsrPref->SetUpdateLinkMode(nNewLinkMode);
}

void SwModule::CheckSpellChanges( bool bOnlineSpelling,
        bool bIsSpellWrongAgain, bool bIsSpellAllAgain, bool bSmartTags )
{
    bool bOnlyWrong = bIsSpellWrongAgain && !bIsSpellAllAgain;
    bool bInvalid = bOnlyWrong || bIsSpellAllAgain;
    if( !(bOnlineSpelling || bInvalid) )
        return;

    for( SwDocShell *pDocSh = static_cast<SwDocShell*>(SfxObjectShell::GetFirst(checkSfxObjectShell<SwDocShell>));
         pDocSh;
         pDocSh = static_cast<SwDocShell*>(SfxObjectShell::GetNext( *pDocSh, checkSfxObjectShell<SwDocShell> ) ) )
    {
        SwDoc* pTmp = pDocSh->GetDoc();
        if ( pTmp->getIDocumentLayoutAccess().GetCurrentViewShell() )
        {
            pTmp->SpellItAgainSam( bInvalid, bOnlyWrong, bSmartTags );
            SwViewShell* pViewShell = pTmp->getIDocumentLayoutAccess().GetCurrentViewShell();
            if ( bSmartTags && pViewShell && pViewShell->GetWin() )
                pViewShell->GetWin()->Invalidate();
        }
    }
}

void SwModule::ApplyDefaultPageMode(bool bIsSquaredPageMode)
{
    if(!m_pUsrPref)
        GetUsrPref(false);
    m_pUsrPref->SetDefaultPageMode(bIsSquaredPageMode);
}

SwCompareMode SwModule::GetCompareMode() const
{
    return m_pModuleConfig->GetCompareMode();
}

bool SwModule::IsUseRsid() const
{
    return m_pModuleConfig->IsUseRsid();
}

bool SwModule::IsIgnorePieces() const
{
    return m_pModuleConfig->IsIgnorePieces();
}

sal_uInt16 SwModule::GetPieceLen() const
{
    return m_pModuleConfig->GetPieceLen();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
