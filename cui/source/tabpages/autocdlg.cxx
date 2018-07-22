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

#include <i18nutil/unicode.hxx>
#include <vcl/builderfactory.hxx>
#include <vcl/field.hxx>
#include <vcl/keycodes.hxx>
#include <vcl/settings.hxx>
#include <sot/exchange.hxx>
#include <svtools/transfer.hxx>
#include <unotools/syslocale.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/viewsh.hxx>
#include <unotools/charclass.hxx>
#include <unotools/collatorwrapper.hxx>
#include <com/sun/star/i18n/CollatorOptions.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>
#include <vcl/svapp.hxx>
#include <sfx2/module.hxx>
#include <sfx2/request.hxx>
#include <sfx2/sfxsids.hrc>
#include <svl/eitem.hxx>
#include <svl/languageoptions.hxx>
#include <svtools/svlbitm.hxx>
#include <svx/SmartTagMgr.hxx>
#include <com/sun/star/smarttags/XSmartTagRecognizer.hpp>
#include <com/sun/star/smarttags/XSmartTagAction.hpp>
#include <rtl/strbuf.hxx>
#include <o3tl/make_unique.hxx>

#include <autocdlg.hxx>
#include <editeng/acorrcfg.hxx>
#include <editeng/svxacorr.hxx>
#include <cuicharmap.hxx>
#include <strings.hrc>
#include <editeng/unolingu.hxx>
#include <dialmgr.hxx>
#include <svx/svxids.hrc>

static LanguageType eLastDialogLanguage = LANGUAGE_SYSTEM;

using namespace ::com::sun::star::util;
using namespace ::com::sun::star;

OfaAutoCorrDlg::OfaAutoCorrDlg(vcl::Window* pParent, const SfxItemSet* _pSet )
    : SfxTabDialog(pParent, "AutoCorrectDialog", "cui/ui/autocorrectdialog.ui", _pSet)
{
    get(m_pLanguageBox, "langbox");
    get(m_pLanguageLB, "lang");

    bool bShowSWOptions = false;
    bool bOpenSmartTagOptions = false;

    if ( _pSet )
    {
        const SfxBoolItem* pItem = SfxItemSet::GetItem<SfxBoolItem>(_pSet, SID_AUTO_CORRECT_DLG, false);
        if ( pItem && pItem->GetValue() )
            bShowSWOptions = true;

        const SfxBoolItem* pItem2 = SfxItemSet::GetItem<SfxBoolItem>(_pSet, SID_OPEN_SMARTTAGOPTIONS, false);
        if ( pItem2 && pItem2->GetValue() )
            bOpenSmartTagOptions = true;
    }

    AddTabPage("options", OfaAutocorrOptionsPage::Create, nullptr);
    AddTabPage("apply", OfaSwAutoFmtOptionsPage::Create, nullptr);
    AddTabPage("wordcompletion", OfaAutoCompleteTabPage::Create, nullptr);
    AddTabPage("smarttags", OfaSmartTagOptionsTabPage::Create, nullptr);

    if (!bShowSWOptions)
    {
        RemoveTabPage("apply");
        RemoveTabPage("wordcompletion");
        RemoveTabPage("smarttags");
    }
    else
    {
        // remove smart tag tab page if no extensions are installed
        SvxAutoCorrect* pAutoCorrect = SvxAutoCorrCfg::Get().GetAutoCorrect();
        SvxSwAutoFormatFlags *pOpt = &pAutoCorrect->GetSwFlags();
        if ( !pOpt || !pOpt->pSmartTagMgr || 0 == pOpt->pSmartTagMgr->NumberOfRecognizers() )
            RemoveTabPage("smarttags");

        RemoveTabPage("options");
    }

    m_nReplacePageId = AddTabPage("replace", OfaAutocorrReplacePage::Create, nullptr);
    m_nExceptionsPageId = AddTabPage("exceptions",  OfaAutocorrExceptPage::Create, nullptr);
    AddTabPage("localized", OfaQuoteTabPage::Create, nullptr);

    // initialize languages
    //! LANGUAGE_NONE is displayed as '[All]' and the LanguageType
    //! will be set to LANGUAGE_UNDETERMINED
    SvxLanguageListFlags nLangList = SvxLanguageListFlags::WESTERN;

    if( SvtLanguageOptions().IsCTLFontEnabled() )
        nLangList |= SvxLanguageListFlags::CTL;
    if( SvtLanguageOptions().IsCJKFontEnabled() )
        nLangList |= SvxLanguageListFlags::CJK;
    m_pLanguageLB->SetLanguageList( nLangList, true, true );
    m_pLanguageLB->SelectLanguage( LANGUAGE_NONE );
    sal_Int32 nPos = m_pLanguageLB->GetSelectedEntryPos();
    DBG_ASSERT( LISTBOX_ENTRY_NOTFOUND != nPos, "listbox entry missing" );
    m_pLanguageLB->SetEntryData( nPos, reinterpret_cast<void*>(static_cast<sal_uInt16>(LANGUAGE_UNDETERMINED)) );

    // Initializing doesn't work for static on linux - therefore here
    if( LANGUAGE_SYSTEM == eLastDialogLanguage )
        eLastDialogLanguage = Application::GetSettings().GetLanguageTag().getLanguageType();

    LanguageType nSelectLang = LANGUAGE_UNDETERMINED;
    nPos = m_pLanguageLB->GetEntryPos( reinterpret_cast<void*>(static_cast<sal_uInt16>(eLastDialogLanguage)) );
    if (LISTBOX_ENTRY_NOTFOUND != nPos)
        nSelectLang = eLastDialogLanguage;
    m_pLanguageLB->SelectLanguage( nSelectLang );

    m_pLanguageLB->SetSelectHdl(LINK(this, OfaAutoCorrDlg, SelectLanguageHdl));

    if ( bOpenSmartTagOptions )
        SetCurPageId("smarttags");
}

OfaAutoCorrDlg::~OfaAutoCorrDlg()
{
    disposeOnce();
}

void OfaAutoCorrDlg::dispose()
{
    m_pLanguageBox.clear();
    m_pLanguageLB.clear();
    SfxTabDialog::dispose();
}

void OfaAutoCorrDlg::EnableLanguage(bool bEnable)
{
    m_pLanguageBox->Enable(bEnable);
}

static bool lcl_FindEntry( ListBox& rLB, const OUString& rEntry,
                    CollatorWrapper const & rCmpClass )
{
    sal_Int32 nCount = rLB.GetEntryCount();
    sal_Int32 nSelPos = rLB.GetSelectedEntryPos();
    sal_Int32 i;
    for(i = 0; i < nCount; i++)
    {
        if( 0 == rCmpClass.compareString(rEntry, rLB.GetEntry(i) ))
        {
            rLB.SelectEntryPos(i);
            return true;
        }
    }
    if(LISTBOX_ENTRY_NOTFOUND != nSelPos)
        rLB.SelectEntryPos(nSelPos, false);
    return false;
}

IMPL_LINK(OfaAutoCorrDlg, SelectLanguageHdl, ListBox&, rBox, void)
{
    sal_Int32 nPos = rBox.GetSelectedEntryPos();
    void* pVoid = rBox.GetEntryData(nPos);
    LanguageType eNewLang = LanguageType(reinterpret_cast<sal_IntPtr>(pVoid));
    // save old settings and fill anew
    if(eNewLang != eLastDialogLanguage)
    {
        sal_uInt16  nPageId = GetCurPageId();
        if (m_nReplacePageId == nPageId)
            static_cast<OfaAutocorrReplacePage*>(GetTabPage( nPageId ))->SetLanguage(eNewLang);
        else if (m_nExceptionsPageId == nPageId)
            static_cast<OfaAutocorrExceptPage*>(GetTabPage( nPageId ))->SetLanguage(eNewLang);
    }
}

OfaAutocorrOptionsPage::OfaAutocorrOptionsPage(vcl::Window* pParent, const SfxItemSet& rSet)
    : SfxTabPage(pParent, "AutocorrectOptionsPage", "cui/ui/acoroptionspage.ui", &rSet)
    , m_sInput(CuiResId(RID_SVXSTR_USE_REPLACE))
    , m_sDoubleCaps(CuiResId(RID_SVXSTR_CPTL_STT_WORD))
    , m_sStartCap(CuiResId(RID_SVXSTR_CPTL_STT_SENT))
    , m_sBoldUnderline(CuiResId(RID_SVXSTR_BOLD_UNDER))
    , m_sURL(CuiResId(RID_SVXSTR_DETECT_URL))
    , m_sNoDblSpaces(CuiResId(RID_SVXSTR_NO_DBL_SPACES))
    , m_sDash(CuiResId(RID_SVXSTR_DASH))
    , m_sAccidentalCaps(CuiResId(RID_SVXSTR_CORRECT_ACCIDENTAL_CAPS_LOCK))
{
    get(m_pCheckLB, "checklist");
}

OfaAutocorrOptionsPage::~OfaAutocorrOptionsPage()
{
    disposeOnce();
}

void OfaAutocorrOptionsPage::dispose()
{
    m_pCheckLB.clear();
    SfxTabPage::dispose();
}

VclPtr<SfxTabPage> OfaAutocorrOptionsPage::Create( TabPageParent pParent,
                                                   const SfxItemSet* rSet)
{
    return VclPtr<OfaAutocorrOptionsPage>::Create(pParent.pParent, *rSet);
}

bool OfaAutocorrOptionsPage::FillItemSet( SfxItemSet* )
{
    SvxAutoCorrect* pAutoCorrect = SvxAutoCorrCfg::Get().GetAutoCorrect();
    ACFlags nFlags = pAutoCorrect->GetFlags();

    sal_uLong nPos = 0;
    pAutoCorrect->SetAutoCorrFlag(ACFlags::Autocorrect,          m_pCheckLB->IsChecked(nPos++));
    pAutoCorrect->SetAutoCorrFlag(ACFlags::CapitalStartWord,     m_pCheckLB->IsChecked(nPos++));
    pAutoCorrect->SetAutoCorrFlag(ACFlags::CapitalStartSentence, m_pCheckLB->IsChecked(nPos++));
    pAutoCorrect->SetAutoCorrFlag(ACFlags::ChgWeightUnderl,      m_pCheckLB->IsChecked(nPos++));
    pAutoCorrect->SetAutoCorrFlag(ACFlags::SetINetAttr,          m_pCheckLB->IsChecked(nPos++));
    pAutoCorrect->SetAutoCorrFlag(ACFlags::ChgToEnEmDash,        m_pCheckLB->IsChecked(nPos++));
    pAutoCorrect->SetAutoCorrFlag(ACFlags::IgnoreDoubleSpace,    m_pCheckLB->IsChecked(nPos++));
    pAutoCorrect->SetAutoCorrFlag(ACFlags::CorrectCapsLock,      m_pCheckLB->IsChecked(nPos++));

    bool bReturn = nFlags != pAutoCorrect->GetFlags();
    if(bReturn )
    {
        SvxAutoCorrCfg& rCfg = SvxAutoCorrCfg::Get();
        rCfg.SetModified();
        rCfg.Commit();
    }
    return bReturn;
}

void    OfaAutocorrOptionsPage::ActivatePage( const SfxItemSet& )
{
    static_cast<OfaAutoCorrDlg*>(GetTabDialog())->EnableLanguage(false);
}

void OfaAutocorrOptionsPage::Reset( const SfxItemSet* )
{
    SvxAutoCorrect* pAutoCorrect = SvxAutoCorrCfg::Get().GetAutoCorrect();
    const ACFlags nFlags = pAutoCorrect->GetFlags();

    m_pCheckLB->SetUpdateMode(false);
    m_pCheckLB->Clear();

    m_pCheckLB->InsertEntry(m_sInput);
    m_pCheckLB->InsertEntry(m_sDoubleCaps);
    m_pCheckLB->InsertEntry(m_sStartCap);
    m_pCheckLB->InsertEntry(m_sBoldUnderline);
    m_pCheckLB->InsertEntry(m_sURL);
    m_pCheckLB->InsertEntry(m_sDash);
    m_pCheckLB->InsertEntry(m_sNoDblSpaces);
    m_pCheckLB->InsertEntry(m_sAccidentalCaps);

    sal_uLong nPos = 0;
    m_pCheckLB->CheckEntryPos( nPos++, bool(nFlags & ACFlags::Autocorrect) );
    m_pCheckLB->CheckEntryPos( nPos++, bool(nFlags & ACFlags::CapitalStartWord) );
    m_pCheckLB->CheckEntryPos( nPos++, bool(nFlags & ACFlags::CapitalStartSentence) );
    m_pCheckLB->CheckEntryPos( nPos++, bool(nFlags & ACFlags::ChgWeightUnderl) );
    m_pCheckLB->CheckEntryPos( nPos++, bool(nFlags & ACFlags::SetINetAttr) );
    m_pCheckLB->CheckEntryPos( nPos++, bool(nFlags & ACFlags::ChgToEnEmDash) );
    m_pCheckLB->CheckEntryPos( nPos++, bool(nFlags & ACFlags::IgnoreDoubleSpace) );
    m_pCheckLB->CheckEntryPos( nPos++, bool(nFlags & ACFlags::CorrectCapsLock) );

    m_pCheckLB->SetUpdateMode(true);
}

/*********************************************************************/
/*                                                                   */
/*  helping struct for dUserData of the Checklistbox                 */
/*                                                                   */
/*********************************************************************/

struct ImpUserData
{
    OUString  *pString;
    vcl::Font *pFont;

    ImpUserData(OUString* pText, vcl::Font* pFnt)
        { pString = pText; pFont = pFnt;}
};


/*********************************************************************/
/*                                                                   */
/*  dialog for per cent settings                                     */
/*                                                                   */
/*********************************************************************/

class OfaAutoFmtPrcntSet : public weld::GenericDialogController
{
    std::unique_ptr<weld::MetricSpinButton> m_xPrcntMF;
public:
    explicit OfaAutoFmtPrcntSet(weld::Window* pParent)
        : GenericDialogController(pParent, "cui/ui/percentdialog.ui", "PercentDialog")
        , m_xPrcntMF(m_xBuilder->weld_metric_spin_button("margin", FUNIT_PERCENT))
    {
    }

    weld::MetricSpinButton& GetPrcntFld()
    {
        return *m_xPrcntMF;
    }
};

/*********************************************************************/
/*                                                                   */
/*  changed LBoxString                                               */
/*                                                                   */
/*********************************************************************/

class OfaImpBrwString : public SvLBoxString
{
public:

    explicit OfaImpBrwString( const OUString& rStr ) : SvLBoxString(rStr){}

    virtual void Paint(const Point& rPos, SvTreeListBox& rDev, vcl::RenderContext& rRenderContext,
                       const SvViewDataEntry* pView, const SvTreeListEntry& rEntry) override;
};

void OfaImpBrwString::Paint(const Point& rPos, SvTreeListBox& /*rDev*/, vcl::RenderContext& rRenderContext,
                            const SvViewDataEntry* /*pView*/, const SvTreeListEntry& rEntry)
{
    rRenderContext.DrawText(rPos, GetText());
    if (rEntry.GetUserData())
    {
        ImpUserData* pUserData = static_cast<ImpUserData*>(rEntry.GetUserData());
        Point aNewPos(rPos);
        aNewPos.AdjustX(rRenderContext.GetTextWidth(GetText()) );
        vcl::Font aOldFont(rRenderContext.GetFont());
        vcl::Font aFont(aOldFont);
        if (pUserData->pFont)
        {
            aFont = *pUserData->pFont;
            aFont.SetColor(aOldFont.GetColor());
            aFont.SetFontSize(aOldFont.GetFontSize());
        }
        aFont.SetWeight(WEIGHT_BOLD);

        bool bFett = true;
        sal_Int32 nPos = 0;
        do {
            OUString sTxt(pUserData->pString->getToken(0, 1, nPos));

            if (bFett)
                rRenderContext.SetFont(aFont);

            rRenderContext.DrawText(aNewPos, sTxt);

            if (-1 != nPos)
                aNewPos.AdjustX(rRenderContext.GetTextWidth(sTxt) );

            if (bFett)
                rRenderContext.SetFont(aOldFont);

            bFett = !bFett;
        } while(-1 != nPos);
    }
}

/*********************************************************************/
/*                                                                   */
/*  use TabPage autoformat                                           */
/*                                                                   */
/*********************************************************************/

#define CBCOL_FIRST     0
#define CBCOL_SECOND    1
#define CBCOL_BOTH      2

enum OfaAutoFmtOptions
{
    USE_REPLACE_TABLE,
    CORR_UPPER,
    BEGIN_UPPER,
    BOLD_UNDERLINE,
    DETECT_URL,
    REPLACE_DASHES,
    DEL_SPACES_AT_STT_END,
    DEL_SPACES_BETWEEN_LINES,
    IGNORE_DBLSPACE,
    CORRECT_CAPS_LOCK,
    APPLY_NUMBERING,
    INSERT_BORDER,
    CREATE_TABLE,
    REPLACE_STYLES,
    DEL_EMPTY_NODE,
    REPLACE_USER_COLL,
    REPLACE_BULLETS,
    MERGE_SINGLE_LINE_PARA
};

OfaSwAutoFmtOptionsPage::OfaSwAutoFmtOptionsPage( vcl::Window* pParent,
                                const SfxItemSet& rSet )
    : SfxTabPage(pParent, "ApplyAutoFmtPage", "cui/ui/applyautofmtpage.ui", &rSet)
    , sDeleteEmptyPara(CuiResId(RID_SVXSTR_DEL_EMPTY_PARA))
    , sUseReplaceTbl(CuiResId(RID_SVXSTR_USE_REPLACE))
    , sCapitalStartWord(CuiResId(RID_SVXSTR_CPTL_STT_WORD))
    , sCapitalStartSentence(CuiResId(RID_SVXSTR_CPTL_STT_SENT))
    , sUserStyle(CuiResId(RID_SVXSTR_USER_STYLE))
    , sBullet(CuiResId(RID_SVXSTR_BULLET))
    , sBoldUnder(CuiResId(RID_SVXSTR_BOLD_UNDER))
    , sNoDblSpaces(CuiResId(RID_SVXSTR_NO_DBL_SPACES))
    , sCorrectCapsLock(CuiResId(RID_SVXSTR_CORRECT_ACCIDENTAL_CAPS_LOCK))
    , sDetectURL(CuiResId(RID_SVXSTR_DETECT_URL))
    , sDash(CuiResId(RID_SVXSTR_DASH))
    , sRightMargin(CuiResId(RID_SVXSTR_RIGHT_MARGIN))
    , sNum(CuiResId(RID_SVXSTR_NUM))
    , sBorder(CuiResId(RID_SVXSTR_BORDER))
    , sTable(CuiResId(RID_SVXSTR_CREATE_TABLE))
    , sReplaceTemplates(CuiResId(RID_SVXSTR_REPLACE_TEMPLATES))
    , sDelSpaceAtSttEnd(CuiResId(RID_SVXSTR_DEL_SPACES_AT_STT_END))
    , sDelSpaceBetweenLines(CuiResId(RID_SVXSTR_DEL_SPACES_BETWEEN_LINES))
    , nPercent(50)
{
    get(m_pEditPB, "edit");

    SvSimpleTableContainer* pCheckLBContainer(get<SvSimpleTableContainer>("list"));
    Size aControlSize(248 , 149);
    aControlSize = LogicToPixel(aControlSize, MapMode(MapUnit::MapAppFont));
    pCheckLBContainer->set_width_request(aControlSize.Width());
    pCheckLBContainer->set_height_request(aControlSize.Height());
    m_pCheckLB = VclPtr<OfaACorrCheckListBox>::Create(*pCheckLBContainer);

    m_pCheckLB->SetStyle(m_pCheckLB->GetStyle()|WB_HSCROLL| WB_VSCROLL);

    m_pCheckLB->SetSelectHdl(LINK(this, OfaSwAutoFmtOptionsPage, SelectHdl));
    m_pCheckLB->SetDoubleClickHdl(LINK(this, OfaSwAutoFmtOptionsPage, DoubleClickEditHdl));

    static long const aStaticTabs[]=
    {
        0, 20, 40
    };

    m_pCheckLB->SvSimpleTable::SetTabs(SAL_N_ELEMENTS(aStaticTabs), aStaticTabs);
    OUString sHeader = get<vcl::Window>("m")->GetText() + "\t"
                     + get<vcl::Window>("t")->GetText() + "\t";
    m_pCheckLB->InsertHeaderEntry(sHeader, HEADERBAR_APPEND,
                        HeaderBarItemBits::CENTER | HeaderBarItemBits::VCENTER | HeaderBarItemBits::FIXEDPOS | HeaderBarItemBits::FIXED);

    m_pEditPB->SetClickHdl(LINK(this, OfaSwAutoFmtOptionsPage, EditHdl));
}

SvTreeListEntry* OfaSwAutoFmtOptionsPage::CreateEntry(OUString& rTxt, sal_uInt16 nCol)
{
    SvTreeListEntry* pEntry = new SvTreeListEntry;

    if (!m_xCheckButtonData)
    {
        m_xCheckButtonData.reset(new SvLBoxButtonData(m_pCheckLB));
        m_pCheckLB->SetCheckButtonData(m_xCheckButtonData.get());
    }

    pEntry->AddItem(o3tl::make_unique<SvLBoxContextBmp>(Image(), Image(), false));

    if (nCol == CBCOL_SECOND)
        pEntry->AddItem(o3tl::make_unique<SvLBoxString>(""));
    else
        pEntry->AddItem(o3tl::make_unique<SvLBoxButton>(SvLBoxButtonKind::EnabledCheckbox, m_xCheckButtonData.get()));

    if (nCol == CBCOL_FIRST)
        pEntry->AddItem(o3tl::make_unique<SvLBoxString>(""));
    else
        pEntry->AddItem(o3tl::make_unique<SvLBoxButton>(SvLBoxButtonKind::EnabledCheckbox, m_xCheckButtonData.get()));
    pEntry->AddItem(o3tl::make_unique<OfaImpBrwString>(rTxt));

    return pEntry;
}

OfaSwAutoFmtOptionsPage::~OfaSwAutoFmtOptionsPage()
{
    disposeOnce();
}

void OfaSwAutoFmtOptionsPage::dispose()
{
    if (m_pCheckLB)
    {
        delete static_cast<ImpUserData*>(m_pCheckLB->GetUserData( REPLACE_BULLETS ));
        delete static_cast<ImpUserData*>(m_pCheckLB->GetUserData( APPLY_NUMBERING ));
        delete static_cast<ImpUserData*>(m_pCheckLB->GetUserData( MERGE_SINGLE_LINE_PARA ));
        m_xCheckButtonData.reset();
    }
    m_pCheckLB.disposeAndClear();
    m_pEditPB.clear();
    SfxTabPage::dispose();
}

VclPtr<SfxTabPage> OfaSwAutoFmtOptionsPage::Create( TabPageParent pParent,
                                                    const SfxItemSet* rAttrSet)
{
    return VclPtr<OfaSwAutoFmtOptionsPage>::Create(pParent.pParent, *rAttrSet);
}

bool OfaSwAutoFmtOptionsPage::FillItemSet( SfxItemSet*  )
{
    bool bModified = false;
    SvxAutoCorrect* pAutoCorrect = SvxAutoCorrCfg::Get().GetAutoCorrect();
    SvxSwAutoFormatFlags *pOpt = &pAutoCorrect->GetSwFlags();
    ACFlags nFlags = pAutoCorrect->GetFlags();

    bool bCheck = m_pCheckLB->IsChecked(USE_REPLACE_TABLE);
    bModified |= pOpt->bAutoCorrect != bCheck;
    pOpt->bAutoCorrect = bCheck;
    pAutoCorrect->SetAutoCorrFlag(ACFlags::Autocorrect,
                        m_pCheckLB->IsChecked(USE_REPLACE_TABLE, CBCOL_SECOND));

    bCheck = m_pCheckLB->IsChecked(CORR_UPPER);
    bModified |= pOpt->bCapitalStartWord != bCheck;
    pOpt->bCapitalStartWord = bCheck;
    pAutoCorrect->SetAutoCorrFlag(ACFlags::CapitalStartWord,
                        m_pCheckLB->IsChecked(CORR_UPPER, CBCOL_SECOND));

    bCheck = m_pCheckLB->IsChecked(BEGIN_UPPER);
    bModified |= pOpt->bCapitalStartSentence != bCheck;
    pOpt->bCapitalStartSentence = bCheck;
    pAutoCorrect->SetAutoCorrFlag(ACFlags::CapitalStartSentence,
                        m_pCheckLB->IsChecked(BEGIN_UPPER, CBCOL_SECOND));

    bCheck = m_pCheckLB->IsChecked(BOLD_UNDERLINE);
    bModified |= pOpt->bChgWeightUnderl != bCheck;
    pOpt->bChgWeightUnderl = bCheck;
    pAutoCorrect->SetAutoCorrFlag(ACFlags::ChgWeightUnderl,
                        m_pCheckLB->IsChecked(BOLD_UNDERLINE, CBCOL_SECOND));

    pAutoCorrect->SetAutoCorrFlag(ACFlags::IgnoreDoubleSpace,
                        m_pCheckLB->IsChecked(IGNORE_DBLSPACE, CBCOL_SECOND));

    pAutoCorrect->SetAutoCorrFlag(ACFlags::CorrectCapsLock,
                        m_pCheckLB->IsChecked(CORRECT_CAPS_LOCK, CBCOL_SECOND));

    bCheck = m_pCheckLB->IsChecked(DETECT_URL);
    bModified |= pOpt->bSetINetAttr != bCheck;
    pOpt->bSetINetAttr = bCheck;
    pAutoCorrect->SetAutoCorrFlag(ACFlags::SetINetAttr,
                        m_pCheckLB->IsChecked(DETECT_URL, CBCOL_SECOND));

    bCheck = m_pCheckLB->IsChecked(DEL_EMPTY_NODE);
    bModified |= pOpt->bDelEmptyNode != bCheck;
    pOpt->bDelEmptyNode = bCheck;

    bCheck = m_pCheckLB->IsChecked(REPLACE_USER_COLL);
    bModified |= pOpt->bChgUserColl != bCheck;
    pOpt->bChgUserColl = bCheck;

    bCheck = m_pCheckLB->IsChecked(REPLACE_BULLETS);
    bModified |= pOpt->bChgEnumNum != bCheck;
    pOpt->bChgEnumNum = bCheck;
    bModified |= aBulletFont != pOpt->aBulletFont;
    pOpt->aBulletFont = aBulletFont;
    bModified |= !comphelper::string::equals(sBulletChar, pOpt->cBullet);
    pOpt->cBullet = sBulletChar[0];

    bModified |= aByInputBulletFont != pOpt->aByInputBulletFont;
    bModified |= !comphelper::string::equals(sByInputBulletChar, pOpt->cByInputBullet);
    pOpt->aByInputBulletFont = aByInputBulletFont;
    pOpt->cByInputBullet = sByInputBulletChar[0];

    bCheck = m_pCheckLB->IsChecked(MERGE_SINGLE_LINE_PARA);
    bModified |= pOpt->bRightMargin != bCheck;
    pOpt->bRightMargin = bCheck;
    bModified |= nPercent != pOpt->nRightMargin;
    pOpt->nRightMargin = static_cast<sal_uInt8>(nPercent);

    bCheck = m_pCheckLB->IsChecked(APPLY_NUMBERING, CBCOL_SECOND);
    bModified |= pOpt->bSetNumRule != bCheck;
    pOpt->bSetNumRule = bCheck;

    bCheck = m_pCheckLB->IsChecked(INSERT_BORDER, CBCOL_SECOND);
    bModified |= pOpt->bSetBorder != bCheck;
    pOpt->bSetBorder = bCheck;

    bCheck = m_pCheckLB->IsChecked(CREATE_TABLE, CBCOL_SECOND);
    bModified |= pOpt->bCreateTable != bCheck;
    pOpt->bCreateTable = bCheck;

    bCheck = m_pCheckLB->IsChecked(REPLACE_STYLES, CBCOL_SECOND);
    bModified |= pOpt->bReplaceStyles != bCheck;
    pOpt->bReplaceStyles = bCheck;

    bCheck = m_pCheckLB->IsChecked(REPLACE_DASHES);
    bModified |= pOpt->bChgToEnEmDash != bCheck;
    pOpt->bChgToEnEmDash = bCheck;
    pAutoCorrect->SetAutoCorrFlag(ACFlags::ChgToEnEmDash,
                        m_pCheckLB->IsChecked(REPLACE_DASHES, CBCOL_SECOND));

    bCheck = m_pCheckLB->IsChecked(DEL_SPACES_AT_STT_END);
    bModified |= pOpt->bAFormatDelSpacesAtSttEnd != bCheck;
    pOpt->bAFormatDelSpacesAtSttEnd = bCheck;
    bCheck = m_pCheckLB->IsChecked(DEL_SPACES_AT_STT_END, CBCOL_SECOND);
    bModified |= pOpt->bAFormatByInpDelSpacesAtSttEnd != bCheck;
    pOpt->bAFormatByInpDelSpacesAtSttEnd = bCheck;

    bCheck = m_pCheckLB->IsChecked(DEL_SPACES_BETWEEN_LINES);
    bModified |= pOpt->bAFormatDelSpacesBetweenLines != bCheck;
    pOpt->bAFormatDelSpacesBetweenLines = bCheck;
    bCheck = m_pCheckLB->IsChecked(DEL_SPACES_BETWEEN_LINES, CBCOL_SECOND);
    bModified |= pOpt->bAFormatByInpDelSpacesBetweenLines != bCheck;
    pOpt->bAFormatByInpDelSpacesBetweenLines = bCheck;

    if(bModified || nFlags != pAutoCorrect->GetFlags())
    {
        SvxAutoCorrCfg& rCfg = SvxAutoCorrCfg::Get();
        rCfg.SetModified();
        rCfg.Commit();
    }

    return true;
}

void    OfaSwAutoFmtOptionsPage::ActivatePage( const SfxItemSet& )
{
    static_cast<OfaAutoCorrDlg*>(GetTabDialog())->EnableLanguage(false);
}

void OfaSwAutoFmtOptionsPage::Reset( const SfxItemSet* )
{
    SvxAutoCorrect* pAutoCorrect = SvxAutoCorrCfg::Get().GetAutoCorrect();
    SvxSwAutoFormatFlags *pOpt = &pAutoCorrect->GetSwFlags();
    const ACFlags nFlags = pAutoCorrect->GetFlags();

    m_pCheckLB->SetUpdateMode(false);
    m_pCheckLB->Clear();

    // The following entries have to be inserted in the same order
    // as in the OfaAutoFmtOptions-enum!
    m_pCheckLB->GetModel()->Insert(CreateEntry(sUseReplaceTbl,     CBCOL_BOTH  ));
    m_pCheckLB->GetModel()->Insert(CreateEntry(sCapitalStartWord,  CBCOL_BOTH  ));
    m_pCheckLB->GetModel()->Insert(CreateEntry(sCapitalStartSentence, CBCOL_BOTH  ));
    m_pCheckLB->GetModel()->Insert(CreateEntry(sBoldUnder,         CBCOL_BOTH  ));
    m_pCheckLB->GetModel()->Insert(CreateEntry(sDetectURL,         CBCOL_BOTH  ));
    m_pCheckLB->GetModel()->Insert(CreateEntry(sDash,              CBCOL_BOTH  ));
    m_pCheckLB->GetModel()->Insert(CreateEntry(sDelSpaceAtSttEnd,  CBCOL_BOTH  ));
    m_pCheckLB->GetModel()->Insert(CreateEntry(sDelSpaceBetweenLines, CBCOL_BOTH  ));

    m_pCheckLB->GetModel()->Insert(CreateEntry(sNoDblSpaces,       CBCOL_SECOND));
    m_pCheckLB->GetModel()->Insert(CreateEntry(sCorrectCapsLock,   CBCOL_SECOND));
    m_pCheckLB->GetModel()->Insert(CreateEntry(sNum,               CBCOL_SECOND));
    m_pCheckLB->GetModel()->Insert(CreateEntry(sBorder,            CBCOL_SECOND));
    m_pCheckLB->GetModel()->Insert(CreateEntry(sTable,             CBCOL_SECOND));
    m_pCheckLB->GetModel()->Insert(CreateEntry(sReplaceTemplates,  CBCOL_SECOND));
    m_pCheckLB->GetModel()->Insert(CreateEntry(sDeleteEmptyPara,   CBCOL_FIRST ));
    m_pCheckLB->GetModel()->Insert(CreateEntry(sUserStyle,         CBCOL_FIRST ));
    m_pCheckLB->GetModel()->Insert(CreateEntry(sBullet,            CBCOL_FIRST ));
    m_pCheckLB->GetModel()->Insert(CreateEntry(sRightMargin,       CBCOL_FIRST ));

    m_pCheckLB->CheckEntryPos( USE_REPLACE_TABLE,  CBCOL_FIRST,    pOpt->bAutoCorrect );
    m_pCheckLB->CheckEntryPos( USE_REPLACE_TABLE,  CBCOL_SECOND,   bool(nFlags & ACFlags::Autocorrect));
    m_pCheckLB->CheckEntryPos( CORR_UPPER,         CBCOL_FIRST,    pOpt->bCapitalStartWord );
    m_pCheckLB->CheckEntryPos( CORR_UPPER,         CBCOL_SECOND,   bool(nFlags & ACFlags::CapitalStartWord) );
    m_pCheckLB->CheckEntryPos( BEGIN_UPPER,        CBCOL_FIRST,    pOpt->bCapitalStartSentence );
    m_pCheckLB->CheckEntryPos( BEGIN_UPPER,        CBCOL_SECOND,   bool(nFlags & ACFlags::CapitalStartSentence) );
    m_pCheckLB->CheckEntryPos( BOLD_UNDERLINE,     CBCOL_FIRST,    pOpt->bChgWeightUnderl );
    m_pCheckLB->CheckEntryPos( BOLD_UNDERLINE,     CBCOL_SECOND,   bool(nFlags & ACFlags::ChgWeightUnderl) );
    m_pCheckLB->CheckEntryPos( IGNORE_DBLSPACE,    CBCOL_SECOND,   bool(nFlags & ACFlags::IgnoreDoubleSpace) );
    m_pCheckLB->CheckEntryPos( CORRECT_CAPS_LOCK,  CBCOL_SECOND,   bool(nFlags & ACFlags::CorrectCapsLock) );
    m_pCheckLB->CheckEntryPos( DETECT_URL,         CBCOL_FIRST,    pOpt->bSetINetAttr );
    m_pCheckLB->CheckEntryPos( DETECT_URL,         CBCOL_SECOND,   bool(nFlags & ACFlags::SetINetAttr) );
    m_pCheckLB->CheckEntryPos( REPLACE_DASHES,     CBCOL_FIRST,    pOpt->bChgToEnEmDash );
    m_pCheckLB->CheckEntryPos( REPLACE_DASHES,     CBCOL_SECOND,   bool(nFlags & ACFlags::ChgToEnEmDash) );
    m_pCheckLB->CheckEntryPos( DEL_SPACES_AT_STT_END,      CBCOL_FIRST,    pOpt->bAFormatDelSpacesAtSttEnd );
    m_pCheckLB->CheckEntryPos( DEL_SPACES_AT_STT_END,      CBCOL_SECOND,   pOpt->bAFormatByInpDelSpacesAtSttEnd );
    m_pCheckLB->CheckEntryPos( DEL_SPACES_BETWEEN_LINES,   CBCOL_FIRST,    pOpt->bAFormatDelSpacesBetweenLines );
    m_pCheckLB->CheckEntryPos( DEL_SPACES_BETWEEN_LINES,   CBCOL_SECOND,   pOpt->bAFormatByInpDelSpacesBetweenLines );
    m_pCheckLB->CheckEntryPos( DEL_EMPTY_NODE,     CBCOL_FIRST,    pOpt->bDelEmptyNode );
    m_pCheckLB->CheckEntryPos( REPLACE_USER_COLL,  CBCOL_FIRST,    pOpt->bChgUserColl );
    m_pCheckLB->CheckEntryPos( REPLACE_BULLETS,    CBCOL_FIRST,    pOpt->bChgEnumNum );

    aBulletFont = pOpt->aBulletFont;
    sBulletChar = OUString(pOpt->cBullet);
    ImpUserData* pUserData = new ImpUserData(&sBulletChar, &aBulletFont);
    m_pCheckLB->SetUserData(  REPLACE_BULLETS, pUserData );

    nPercent = pOpt->nRightMargin;
    sMargin = " " +
        unicode::formatPercent(nPercent, Application::GetSettings().GetUILanguageTag());
    pUserData = new ImpUserData(&sMargin, nullptr);
    m_pCheckLB->SetUserData( MERGE_SINGLE_LINE_PARA, pUserData );

    m_pCheckLB->CheckEntryPos( APPLY_NUMBERING,    CBCOL_SECOND,   pOpt->bSetNumRule );

    aByInputBulletFont = pOpt->aByInputBulletFont;
    sByInputBulletChar = OUString( pOpt->cByInputBullet );
    ImpUserData* pUserData2 = new ImpUserData(&sByInputBulletChar, &aByInputBulletFont);
    m_pCheckLB->SetUserData( APPLY_NUMBERING , pUserData2 );

    m_pCheckLB->CheckEntryPos( MERGE_SINGLE_LINE_PARA, CBCOL_FIRST, pOpt->bRightMargin );
    m_pCheckLB->CheckEntryPos( INSERT_BORDER,      CBCOL_SECOND,   pOpt->bSetBorder );
    m_pCheckLB->CheckEntryPos( CREATE_TABLE,       CBCOL_SECOND,   pOpt->bCreateTable );
    m_pCheckLB->CheckEntryPos( REPLACE_STYLES,     CBCOL_SECOND,   pOpt->bReplaceStyles );

    m_pCheckLB->SetUpdateMode(true);
}

IMPL_LINK(OfaSwAutoFmtOptionsPage, SelectHdl, SvTreeListBox*, pBox, void)
{
    m_pEditPB->Enable(nullptr != pBox->FirstSelected()->GetUserData());
}

IMPL_LINK_NOARG(OfaSwAutoFmtOptionsPage, DoubleClickEditHdl, SvTreeListBox*, bool)
{
    EditHdl(nullptr);
    return false;
}

IMPL_LINK_NOARG(OfaSwAutoFmtOptionsPage, EditHdl, Button*, void)
{
    sal_uLong nSelEntryPos = m_pCheckLB->GetSelectedEntryPos();
    if( nSelEntryPos == REPLACE_BULLETS ||
        nSelEntryPos == APPLY_NUMBERING)
    {
        SvxCharacterMap aMapDlg(GetFrameWeld(), nullptr, false);
        ImpUserData* pUserData = static_cast<ImpUserData*>(m_pCheckLB->FirstSelected()->GetUserData());
        aMapDlg.SetCharFont(*pUserData->pFont);
        aMapDlg.SetChar( (*pUserData->pString)[0] );
        if (RET_OK == aMapDlg.execute())
        {
            vcl::Font aFont(aMapDlg.GetCharFont());
            *pUserData->pFont = aFont;
            sal_UCS4 aChar = aMapDlg.GetChar();
            // using the UCS4 constructor
            OUString aOUStr( &aChar, 1 );
            *pUserData->pString = aOUStr;
        }
    }
    else if( MERGE_SINGLE_LINE_PARA == nSelEntryPos )
    {
        // dialog for per cent settings
        OfaAutoFmtPrcntSet aDlg(GetFrameWeld());
        aDlg.GetPrcntFld().set_value(nPercent, FUNIT_PERCENT);
        if (aDlg.run() == RET_OK)
        {
            nPercent = static_cast<sal_uInt16>(aDlg.GetPrcntFld().get_value(FUNIT_PERCENT));
            sMargin = " " +
                unicode::formatPercent(nPercent, Application::GetSettings().GetUILanguageTag());
        }
    }
    m_pCheckLB->Invalidate();
}

void OfaACorrCheckListBox::SetTabs()
{
    SvSimpleTable::SetTabs();
    SvLBoxTabFlags nAdjust = SvLBoxTabFlags::ADJUST_RIGHT|SvLBoxTabFlags::ADJUST_LEFT|SvLBoxTabFlags::ADJUST_CENTER|SvLBoxTabFlags::ADJUST_NUMERIC|SvLBoxTabFlags::FORCE;

    if( aTabs.size() > 1 )
    {
        SvLBoxTab* pTab = aTabs[1];
        pTab->nFlags &= ~nAdjust;
        pTab->nFlags |= SvLBoxTabFlags::PUSHABLE|SvLBoxTabFlags::ADJUST_CENTER|SvLBoxTabFlags::FORCE;
    }
    if( aTabs.size() > 2 )
    {
        SvLBoxTab* pTab = aTabs[2];
        pTab->nFlags &= ~nAdjust;
        pTab->nFlags |= SvLBoxTabFlags::PUSHABLE|SvLBoxTabFlags::ADJUST_CENTER|SvLBoxTabFlags::FORCE;
    }
}

void OfaACorrCheckListBox::CheckEntryPos(sal_uLong nPos, sal_uInt16 nCol, bool bChecked)
{
    if ( nPos < GetEntryCount() )
        SetCheckButtonState(
            GetEntry(nPos),
            nCol,
            bChecked ? SvButtonState::Checked : SvButtonState::Unchecked );
}

bool OfaACorrCheckListBox::IsChecked(sal_uLong nPos, sal_uInt16 nCol)
{
    return GetCheckButtonState( GetEntry(nPos), nCol ) == SvButtonState::Checked;
}

void OfaACorrCheckListBox::SetCheckButtonState( SvTreeListEntry* pEntry, sal_uInt16 nCol, SvButtonState eState)
{
    SvLBoxButton& rItem = static_cast<SvLBoxButton&>(pEntry->GetItem(nCol + 1));

    if (rItem.GetType() == SvLBoxItemType::Button)
    {
        switch( eState )
        {
            case SvButtonState::Checked:
                rItem.SetStateChecked();
                break;

            case SvButtonState::Unchecked:
                rItem.SetStateUnchecked();
                break;

            case SvButtonState::Tristate:
                rItem.SetStateTristate();
                break;
        }
        InvalidateEntry( pEntry );
    }
}

SvButtonState OfaACorrCheckListBox::GetCheckButtonState( SvTreeListEntry* pEntry, sal_uInt16 nCol )
{
    SvButtonState eState = SvButtonState::Unchecked;
    SvLBoxButton& rItem = static_cast<SvLBoxButton&>(pEntry->GetItem(nCol + 1));

    if (rItem.GetType() == SvLBoxItemType::Button)
    {
        SvItemStateFlags nButtonFlags = rItem.GetButtonFlags();
        eState = SvLBoxButtonData::ConvertToButtonState( nButtonFlags );
    }

    return eState;
}

void OfaACorrCheckListBox::HBarClick()
{
    // sorting is stopped by this override
}

void    OfaACorrCheckListBox::KeyInput( const KeyEvent& rKEvt )
{
    if(!rKEvt.GetKeyCode().GetModifier() &&
        KEY_SPACE == rKEvt.GetKeyCode().GetCode())
    {
        sal_uLong nSelPos = GetSelectedEntryPos();
        sal_uInt16 nCol = GetCurrentTabPos() - 1;
        if ( nCol < 2 )
        {
            CheckEntryPos( nSelPos, nCol, !IsChecked( nSelPos, nCol ) );
            CallImplEventListeners( VclEventId::CheckboxToggle, static_cast<void*>(GetEntry( nSelPos )) );
        }
        else
        {
            sal_uInt16 nCheck = IsChecked(nSelPos, 1) ? 1 : 0;
            if(IsChecked(nSelPos))
                nCheck += 2;
            nCheck--;
            nCheck &= 3;
            CheckEntryPos(nSelPos, 1, 0 != (nCheck & 1));
            CheckEntryPos(nSelPos, 0, 0 != (nCheck & 2));
        }
    }
    else
        SvSimpleTable::KeyInput(rKEvt);
}

OfaAutocorrReplacePage::OfaAutocorrReplacePage( vcl::Window* pParent,
                                                const SfxItemSet& rSet )
    : SfxTabPage(pParent, "AcorReplacePage", "cui/ui/acorreplacepage.ui", &rSet)
    , eLang(eLastDialogLanguage)
    , bHasSelectionText(false)
    , bFirstSelect(true)
    , bReplaceEditChanged(false)
    , bSWriter(true)
{
    get(m_pTextOnlyCB, "textonly");
    get(m_pDeleteReplacePB, "delete");
    get(m_pNewReplacePB, "new");
    sNew = m_pNewReplacePB->GetText();
    sModify = get<PushButton>("replace")->GetText();
    get(m_pShortED, "origtext");
    get(m_pReplaceED, "newtext");
    get(m_pReplaceTLB, "tabview");
    m_pReplaceTLB->set_height_request(16 * GetTextHeight());

    SfxModule *pMod = SfxApplication::GetModule(SfxToolsModule::Writer);
    bSWriter = pMod == SfxModule::GetActiveModule();

    LanguageTag aLanguageTag( eLastDialogLanguage );
    pCompareClass.reset( new CollatorWrapper( comphelper::getProcessComponentContext() ) );
    pCompareClass->loadDefaultCollator( aLanguageTag.getLocale(), 0 );
    pCharClass.reset( new CharClass( aLanguageTag ) );

    static long const aTabs[] = { 1, 61 };
    m_pReplaceTLB->SetTabs( SAL_N_ELEMENTS(aTabs), aTabs );

    m_pReplaceTLB->SetStyle( m_pReplaceTLB->GetStyle()|WB_HSCROLL|WB_CLIPCHILDREN );
    m_pReplaceTLB->SetSelectHdl( LINK(this, OfaAutocorrReplacePage, SelectHdl) );
    m_pNewReplacePB->SetClickHdl( LINK(this, OfaAutocorrReplacePage, NewDelButtonHdl) );
    m_pDeleteReplacePB->SetClickHdl( LINK(this, OfaAutocorrReplacePage, NewDelButtonHdl) );
    m_pShortED->SetModifyHdl( LINK(this, OfaAutocorrReplacePage, ModifyHdl) );
    m_pReplaceED->SetModifyHdl( LINK(this, OfaAutocorrReplacePage, ModifyHdl) );
    m_pShortED->SetActionHdl( LINK(this, OfaAutocorrReplacePage, NewDelActionHdl) );
    m_pReplaceED->SetActionHdl( LINK(this, OfaAutocorrReplacePage, NewDelActionHdl) );

    m_pReplaceED->SetSpaces(true);
    m_pShortED->SetSpaces(true);

    m_pShortED->ConnectColumn(m_pReplaceTLB, 0);
    m_pReplaceED->ConnectColumn(m_pReplaceTLB, 1);
}

OfaAutocorrReplacePage::~OfaAutocorrReplacePage()
{
    disposeOnce();
}

void OfaAutocorrReplacePage::dispose()
{
    aDoubleStringTable.clear();
    aChangesTable.clear();

    pCompareClass.reset();
    pCharClass.reset();

    m_pTextOnlyCB.clear();
    m_pShortED.clear();
    m_pReplaceED.clear();
    m_pReplaceTLB.clear();
    m_pNewReplacePB.clear();
    m_pDeleteReplacePB.clear();
    SfxTabPage::dispose();
}

VclPtr<SfxTabPage> OfaAutocorrReplacePage::Create( TabPageParent pParent, const SfxItemSet* rSet)
{
    return VclPtr<OfaAutocorrReplacePage>::Create(pParent.pParent, *rSet);
}

void OfaAutocorrReplacePage::ActivatePage( const SfxItemSet& )
{
    if(eLang != eLastDialogLanguage)
        SetLanguage(eLastDialogLanguage);
    static_cast<OfaAutoCorrDlg*>(GetTabDialog())->EnableLanguage(true);
}

DeactivateRC OfaAutocorrReplacePage::DeactivatePage( SfxItemSet*  )
{
    return DeactivateRC::LeavePage;
}

bool OfaAutocorrReplacePage::FillItemSet( SfxItemSet* )
{
    SvxAutoCorrect* pAutoCorrect = SvxAutoCorrCfg::Get().GetAutoCorrect();

    for (StringChangeTable::reverse_iterator it = aChangesTable.rbegin(); it != aChangesTable.rend(); ++it)
    {
        LanguageType eCurrentLang = it->first;
        StringChangeList& rStringChangeList = it->second;
        std::vector<SvxAutocorrWord> aDeleteWords;
        std::vector<SvxAutocorrWord> aNewWords;

        for (DoubleString & deleteEntry : rStringChangeList.aDeletedEntries)
        {
            SvxAutocorrWord aDeleteWord( deleteEntry.sShort, deleteEntry.sLong );
            aDeleteWords.push_back( aDeleteWord );
        }

        for (DoubleString & newEntry : rStringChangeList.aNewEntries)
        {
            //fdo#67697 if the user data is set then we want to retain the
            //source formatting of the entry, so don't use the optimized
            //text-only MakeCombinedChanges for this entry
            bool bKeepSourceFormatting = newEntry.pUserData == &bHasSelectionText;
            if (bKeepSourceFormatting)
            {
                pAutoCorrect->PutText(newEntry.sShort, *SfxObjectShell::Current(), eCurrentLang);
                continue;
            }

            SvxAutocorrWord aNewWord( newEntry.sShort, newEntry.sLong );
            aNewWords.push_back( aNewWord );
        }
        pAutoCorrect->MakeCombinedChanges( aNewWords, aDeleteWords, eCurrentLang );
    }
    aChangesTable.clear();
    return false;
}

void OfaAutocorrReplacePage::RefillReplaceBox(bool bFromReset,
                                        LanguageType eOldLanguage,
                                        LanguageType eNewLanguage)
{
    eLang = eNewLanguage;
    if(bFromReset)
    {
        aDoubleStringTable.clear();
        aChangesTable.clear();
    }
    else
    {
        DoubleStringArray* pArray;
        if(aDoubleStringTable.find(eOldLanguage) != aDoubleStringTable.end())
        {
            pArray = &aDoubleStringTable[eOldLanguage];
            pArray->clear();
        }
        else
        {
            pArray = &aDoubleStringTable[eOldLanguage]; // create new array
        }

        sal_uInt32 nListBoxCount = static_cast<sal_uInt32>(m_pReplaceTLB->GetEntryCount());
        sal_uInt32 i;
        for(i = 0; i < nListBoxCount; i++)
        {
            pArray->push_back(DoubleString());
            DoubleString& rDouble = (*pArray)[pArray->size() - 1];
            SvTreeListEntry*  pEntry = m_pReplaceTLB->GetEntry( i );
            rDouble.sShort = SvTabListBox::GetEntryText(pEntry, 0);
            rDouble.sLong = SvTabListBox::GetEntryText(pEntry, 1);
            rDouble.pUserData = pEntry->GetUserData();
        }
    }

    m_pReplaceTLB->Clear();
    if( !bSWriter )
        aFormatText.clear();

    if( aDoubleStringTable.find(eLang) != aDoubleStringTable.end() )
    {
        DoubleStringArray& rArray = aDoubleStringTable[eNewLanguage];
        for(DoubleString & rDouble : rArray)
        {
            bool bTextOnly = nullptr == rDouble.pUserData;
            // formatted text is only in Writer
            if(bSWriter || bTextOnly)
            {
                OUString sEntry = rDouble.sShort + "\t" + rDouble.sLong;
                SvTreeListEntry* pEntry = m_pReplaceTLB->InsertEntry(sEntry);
                m_pTextOnlyCB->Check(bTextOnly);
                if(!bTextOnly)
                    pEntry->SetUserData(rDouble.pUserData); // that means: with format info or even with selection text
            }
            else
            {
                aFormatText.insert(rDouble.sShort);
            }
        }
    }
    else
    {
        SvxAutoCorrect* pAutoCorrect = SvxAutoCorrCfg::Get().GetAutoCorrect();
        SvxAutocorrWordList* pWordList = pAutoCorrect->LoadAutocorrWordList(eLang);
        m_pReplaceTLB->SetUpdateMode(false);
        SvxAutocorrWordList::Content aContent = pWordList->getSortedContent();
        for (auto const& elem : aContent)
        {
            bool bTextOnly = elem->IsTextOnly();
            // formatted text is only in Writer
            if(bSWriter || bTextOnly)
            {
                OUString sEntry = elem->GetShort() + "\t" + elem->GetLong();
                SvTreeListEntry* pEntry = m_pReplaceTLB->InsertEntry(sEntry);
                m_pTextOnlyCB->Check(elem->IsTextOnly());
                if(!bTextOnly)
                    pEntry->SetUserData(m_pTextOnlyCB); // that means: with format info
            }
            else
            {
                aFormatText.insert(elem->GetShort());
            }
        }
        m_pNewReplacePB->Enable(false);
        m_pDeleteReplacePB->Enable(false);
        m_pReplaceTLB->SetUpdateMode(true);
    }

    SfxViewShell* pViewShell = SfxViewShell::Current();
    if( pViewShell && pViewShell->HasSelection() )
    {
        bHasSelectionText = true;
        const OUString sSelection( pViewShell->GetSelectionText() );
        m_pReplaceED->SetText( sSelection );
        m_pTextOnlyCB->Check( !bSWriter );
        m_pTextOnlyCB->Enable( bSWriter && !sSelection.isEmpty() );
    }
    else
    {
        m_pTextOnlyCB->Enable( false );
    }
}

void OfaAutocorrReplacePage::Reset( const SfxItemSet* )
{
    RefillReplaceBox(true, eLang, eLang);
    m_pShortED->GrabFocus();
}

void OfaAutocorrReplacePage::SetLanguage(LanguageType eSet)
{
    //save old settings an refill
    if(eSet != eLang)
    {
        RefillReplaceBox(false, eLang, eSet);
        eLastDialogLanguage = eSet;

        LanguageTag aLanguageTag( eLastDialogLanguage );
        pCompareClass.reset( new CollatorWrapper( comphelper::getProcessComponentContext() ) );
        pCompareClass->loadDefaultCollator( aLanguageTag.getLocale(), 0 );
        pCharClass.reset( new CharClass( aLanguageTag ) );
        ModifyHdl(*m_pShortED);
    }
}

IMPL_LINK(OfaAutocorrReplacePage, SelectHdl, SvTreeListBox*, pBox, void)
{
    if(!bFirstSelect || !bHasSelectionText)
    {
        SvTreeListEntry* pEntry = pBox->FirstSelected();
        OUString sTmpShort(SvTabListBox::GetEntryText(pEntry, 0));
        // if the text is set via ModifyHdl, the cursor is always at the beginning
        // of a word, although you're editing here
        bool bSameContent = 0 == pCompareClass->compareString( sTmpShort, m_pShortED->GetText() );
        Selection aSel = m_pShortED->GetSelection();
        if(m_pShortED->GetText() != sTmpShort)
        {
            m_pShortED->SetText(sTmpShort);
            // if it was only a different notation, the selection has to be set again
            if(bSameContent)
            {
                m_pShortED->SetSelection(aSel);
            }
        }
        m_pReplaceED->SetText( SvTabListBox::GetEntryText(pEntry, 1) );
        // with UserData there is a Formatinfo
        m_pTextOnlyCB->Check( pEntry->GetUserData() == nullptr);
    }
    else
    {
        bFirstSelect = false;
    }

    m_pNewReplacePB->Enable(false);
    m_pDeleteReplacePB->Enable();
};

void OfaAutocorrReplacePage::NewEntry(const OUString& sShort, const OUString& sLong, bool bKeepSourceFormatting)
{
    DoubleStringArray& rNewArray = aChangesTable[eLang].aNewEntries;
    for (size_t i = 0; i < rNewArray.size(); i++)
    {
        if (rNewArray[i].sShort == sShort)
        {
            rNewArray.erase(rNewArray.begin() + i);
            break;
        }
    }

    DoubleStringArray& rDeletedArray = aChangesTable[eLang].aDeletedEntries;
    for (size_t i = 0; i < rDeletedArray.size(); i++)
    {
        if (rDeletedArray[i].sShort == sShort)
        {
            rDeletedArray.erase(rDeletedArray.begin() + i);
            break;
        }
    }

    DoubleString aNewString = DoubleString();
    aNewString.sShort = sShort;
    aNewString.sLong = sLong;
    rNewArray.push_back(aNewString);
    if (bKeepSourceFormatting)
        rNewArray.back().pUserData = &bHasSelectionText;
}

void OfaAutocorrReplacePage::DeleteEntry(const OUString& sShort, const OUString& sLong)
{
    DoubleStringArray& rNewArray = aChangesTable[eLang].aNewEntries;
    for (size_t i = 0; i < rNewArray.size(); i++)
    {
        if (rNewArray[i].sShort == sShort)
        {
            rNewArray.erase(rNewArray.begin() + i);
            break;
        }
    }

    DoubleStringArray& rDeletedArray = aChangesTable[eLang].aDeletedEntries;
    for (size_t i = 0; i < rDeletedArray.size(); i++)
    {
        if (rDeletedArray[i].sShort == sShort)
        {
            rDeletedArray.erase(rDeletedArray.begin() + i);
            break;
        }
    }

    DoubleString aDeletedString = DoubleString();
    aDeletedString.sShort = sShort;
    aDeletedString.sLong = sLong;
    rDeletedArray.push_back(aDeletedString);
}

IMPL_LINK(OfaAutocorrReplacePage, NewDelButtonHdl, Button*, pBtn, void)
{
    NewDelHdl(pBtn);
}

IMPL_LINK(OfaAutocorrReplacePage, NewDelActionHdl, AutoCorrEdit&, rEdit, bool)
{
    return NewDelHdl(&rEdit);
}
bool OfaAutocorrReplacePage::NewDelHdl(void const * pBtn)
{
    SvTreeListEntry* pEntry = m_pReplaceTLB->FirstSelected();
    if( pBtn == m_pDeleteReplacePB )
    {
        DBG_ASSERT( pEntry, "no entry selected" );
        if( pEntry )
        {
            DeleteEntry(SvTabListBox::GetEntryText(pEntry, 0), SvTabListBox::GetEntryText(pEntry, 1));
            m_pReplaceTLB->GetModel()->Remove(pEntry);
            ModifyHdl(*m_pShortED);
            return false;
        }
    }
    if(pBtn == m_pNewReplacePB || m_pNewReplacePB->IsEnabled())
    {
        SvTreeListEntry* _pNewEntry = m_pReplaceTLB->FirstSelected();
        OUString sEntry(m_pShortED->GetText());
        if(!sEntry.isEmpty() && ( !m_pReplaceED->GetText().isEmpty() ||
                ( bHasSelectionText && bSWriter ) ))
        {
            bool bKeepSourceFormatting = !bReplaceEditChanged && !m_pTextOnlyCB->IsChecked();

            NewEntry(m_pShortED->GetText(), m_pReplaceED->GetText(), bKeepSourceFormatting);
            m_pReplaceTLB->SetUpdateMode(false);
            sal_uLong nPos = TREELIST_ENTRY_NOTFOUND;
            sEntry += "\t" + m_pReplaceED->GetText();
            if(_pNewEntry)
            {
                nPos = m_pReplaceTLB->GetModel()->GetAbsPos(_pNewEntry);
                m_pReplaceTLB->GetModel()->Remove(_pNewEntry);
            }
            else
            {
                sal_uLong j;
                for( j = 0; j < m_pReplaceTLB->GetEntryCount(); j++ )
                {
                    SvTreeListEntry* pReplaceEntry = m_pReplaceTLB->GetEntry(j);
                    if( 0 >=  pCompareClass->compareString(sEntry, SvTabListBox::GetEntryText(pReplaceEntry, 0) ) )
                        break;
                }
                nPos = j;
            }
            SvTreeListEntry* pInsEntry = m_pReplaceTLB->InsertEntry(
                                        sEntry, static_cast< SvTreeListEntry * >(nullptr), false,
                                        nPos == TREELIST_ENTRY_NOTFOUND ? TREELIST_APPEND : nPos);
            if (bKeepSourceFormatting)
            {
                pInsEntry->SetUserData(&bHasSelectionText); // new formatted text
            }

            m_pReplaceTLB->MakeVisible( pInsEntry );
            m_pReplaceTLB->SetUpdateMode( true );
            // if the request came from the ReplaceEdit, give focus to the ShortEdit
            if(m_pReplaceED->HasFocus())
            {
                m_pShortED->GrabFocus();
            }
        }
    }
    else
    {
        // this can only be an enter in one of the two edit fields
        // which means EndDialog() - has to be evaluated in KeyInput
        return false;
    }
    ModifyHdl(*m_pShortED);
    return true;
}

IMPL_LINK(OfaAutocorrReplacePage, ModifyHdl, Edit&, rEdt, void)
{
    SvTreeListEntry* pFirstSel = m_pReplaceTLB->FirstSelected();
    bool bShort = &rEdt == m_pShortED;
    const OUString rEntry = rEdt.GetText();
    const OUString rRepString = m_pReplaceED->GetText();
    OUString aWordStr( pCharClass->lowercase( rEntry ));

    if(bShort)
    {
        if(!rEntry.isEmpty())
        {
            bool bFound = false;
            bool bTmpSelEntry=false;

            for(sal_uLong i = 0; i < m_pReplaceTLB->GetEntryCount(); i++)
            {
                SvTreeListEntry*  pEntry = m_pReplaceTLB->GetEntry( i );
                OUString aTestStr = SvTabListBox::GetEntryText(pEntry, 0);
                if( pCompareClass->compareString(rEntry, aTestStr ) == 0 )
                {
                    if( !rRepString.isEmpty() )
                    {
                        bFirstSelect = true;
                    }
                    m_pReplaceTLB->SetCurEntry(pEntry);
                    pFirstSel = pEntry;
                    m_pNewReplacePB->SetText(sModify);
                    bFound = true;
                    break;
                }
                else
                {
                    aTestStr = pCharClass->lowercase( aTestStr );
                    if( aTestStr.startsWith(aWordStr) && !bTmpSelEntry )
                    {
                        m_pReplaceTLB->MakeVisible( pEntry );
                        bTmpSelEntry = true;
                    }
                }
            }
            if( !bFound )
            {
                m_pReplaceTLB->SelectAll( false );
                pFirstSel = nullptr;
                m_pNewReplacePB->SetText( sNew );
                if( bReplaceEditChanged )
                    m_pTextOnlyCB->Enable(false);
            }
            m_pDeleteReplacePB->Enable( bFound );
        }
        else if( m_pReplaceTLB->GetEntryCount() > 0 )
        {
            SvTreeListEntry*  pEntry = m_pReplaceTLB->GetEntry( 0 );
            m_pReplaceTLB->MakeVisible( pEntry );
        }

    }
    else if( !bShort )
    {
        bReplaceEditChanged = true;
        if( pFirstSel )
        {
            m_pNewReplacePB->SetText( sModify );
        }
    }

    const OUString& rShortTxt = m_pShortED->GetText();
    bool bEnableNew = !rShortTxt.isEmpty() &&
                        ( !rRepString.isEmpty() ||
                                ( bHasSelectionText && bSWriter )) &&
                        ( !pFirstSel || rRepString !=
                                SvTabListBox::GetEntryText( pFirstSel, 1 ) );
    if( bEnableNew )
    {
        for (auto const& elem : aFormatText)
        {
            if(elem == rShortTxt)
            {
                bEnableNew = false;
                break;
            }
        }
    }
    m_pNewReplacePB->Enable( bEnableNew );
}

static bool lcl_FindInArray(std::vector<OUString>& rStrings, const OUString& rString)
{
    for (auto const& elem : rStrings)
    {
        if(elem == rString)
        {
            return true;
        }
    }
    return false;
}

OfaAutocorrExceptPage::OfaAutocorrExceptPage(vcl::Window* pParent, const SfxItemSet& rSet)
    : SfxTabPage(pParent, "AcorExceptPage", "cui/ui/acorexceptpage.ui", &rSet)
    , eLang(eLastDialogLanguage)
{
    get(m_pAbbrevED, "abbrev");
    get(m_pAbbrevLB, "abbrevlist");
    m_pAbbrevLB->SetStyle(m_pAbbrevLB->GetStyle() | WB_SORT);
    m_pAbbrevLB->SetDropDownLineCount(7);
    get(m_pNewAbbrevPB, "newabbrev");
    get(m_pDelAbbrevPB, "delabbrev");
    get(m_pAutoAbbrevCB, "autoabbrev");

    get(m_pDoubleCapsED, "double");
    get(m_pDoubleCapsLB, "doublelist");
    m_pDoubleCapsLB->SetStyle(m_pDoubleCapsLB->GetStyle() | WB_SORT);
    m_pDoubleCapsLB->SetDropDownLineCount(7);
    get(m_pNewDoublePB, "newdouble");
    get(m_pDelDoublePB, "deldouble");
    get(m_pAutoCapsCB, "autodouble");

    css::lang::Locale aLcl( LanguageTag::convertToLocale(eLastDialogLanguage ));
    pCompareClass.reset( new CollatorWrapper( comphelper::getProcessComponentContext() ) );
    pCompareClass->loadDefaultCollator( aLcl, 0 );

    m_pNewAbbrevPB->SetClickHdl(LINK(this, OfaAutocorrExceptPage, NewDelButtonHdl));
    m_pDelAbbrevPB->SetClickHdl(LINK(this, OfaAutocorrExceptPage, NewDelButtonHdl));
    m_pNewDoublePB->SetClickHdl(LINK(this, OfaAutocorrExceptPage, NewDelButtonHdl));
    m_pDelDoublePB->SetClickHdl(LINK(this, OfaAutocorrExceptPage, NewDelButtonHdl));

    m_pAbbrevLB->SetSelectHdl(LINK(this, OfaAutocorrExceptPage, SelectHdl));
    m_pDoubleCapsLB->SetSelectHdl(LINK(this, OfaAutocorrExceptPage, SelectHdl));
    m_pAbbrevED->SetModifyHdl(LINK(this, OfaAutocorrExceptPage, ModifyHdl));
    m_pDoubleCapsED->SetModifyHdl(LINK(this, OfaAutocorrExceptPage, ModifyHdl));

    m_pAbbrevED->SetActionHdl(LINK(this, OfaAutocorrExceptPage, NewDelActionHdl));
    m_pDoubleCapsED->SetActionHdl(LINK(this, OfaAutocorrExceptPage, NewDelActionHdl));
}

OfaAutocorrExceptPage::~OfaAutocorrExceptPage()
{
    disposeOnce();
}

void OfaAutocorrExceptPage::dispose()
{
    aStringsTable.clear();
    pCompareClass.reset();
    m_pAbbrevED.clear();
    m_pAbbrevLB.clear();
    m_pNewAbbrevPB.clear();
    m_pDelAbbrevPB.clear();
    m_pAutoAbbrevCB.clear();
    m_pDoubleCapsED.clear();
    m_pDoubleCapsLB.clear();
    m_pNewDoublePB.clear();
    m_pDelDoublePB.clear();
    m_pAutoCapsCB.clear();
    SfxTabPage::dispose();
}

VclPtr<SfxTabPage> OfaAutocorrExceptPage::Create( TabPageParent pParent,
                                                  const SfxItemSet* rSet)
{
    return VclPtr<OfaAutocorrExceptPage>::Create(pParent.pParent, *rSet);
}

void    OfaAutocorrExceptPage::ActivatePage( const SfxItemSet& )
{
    if(eLang != eLastDialogLanguage)
        SetLanguage(eLastDialogLanguage);
    static_cast<OfaAutoCorrDlg*>(GetTabDialog())->EnableLanguage(true);
}

DeactivateRC OfaAutocorrExceptPage::DeactivatePage( SfxItemSet* )
{
    return DeactivateRC::LeavePage;
}

bool OfaAutocorrExceptPage::FillItemSet( SfxItemSet*  )
{
    SvxAutoCorrect* pAutoCorrect = SvxAutoCorrCfg::Get().GetAutoCorrect();
    for(StringsTable::reverse_iterator it1 = aStringsTable.rbegin(); it1 != aStringsTable.rend(); ++it1)
    {
        LanguageType eCurLang = it1->first;
        StringsArrays& rArrays = it1->second;
        if(eCurLang != eLang) // current language is treated later
        {
            SvStringsISortDtor* pWrdList = pAutoCorrect->LoadWrdSttExceptList(eCurLang);

            if(pWrdList)
            {
                size_t nCount = pWrdList->size();
                size_t i;
                for( i = nCount; i; )
                {
                    OUString aString = (*pWrdList)[ --i ];

                    if( !lcl_FindInArray(rArrays.aDoubleCapsStrings, aString))
                    {
                      pWrdList->erase(i);
                    }
                }

                for (auto const& elem : rArrays.aDoubleCapsStrings)
                {
                    pWrdList->insert(elem);
                }
                pAutoCorrect->SaveWrdSttExceptList(eCurLang);
            }

            SvStringsISortDtor* pCplList = pAutoCorrect->LoadCplSttExceptList(eCurLang);

            if(pCplList)
            {
                size_t nCount = pCplList->size();
                size_t i;
                for( i = nCount; i; )
                {
                    OUString aString = (*pCplList)[ --i ];
                    if( !lcl_FindInArray(rArrays.aAbbrevStrings, aString))
                    {
                        pCplList->erase(i);
                    }
                }

                for (auto const& elem : rArrays.aAbbrevStrings)
                {
                    pCplList->insert(elem);
                }

                pAutoCorrect->SaveCplSttExceptList(eCurLang);
            }
        }
    }
    aStringsTable.clear();

    SvStringsISortDtor* pWrdList = pAutoCorrect->LoadWrdSttExceptList(eLang);

    if(pWrdList)
    {
        size_t nCount = pWrdList->size();
        size_t i;
        for( i = nCount; i; )
        {
            OUString aString = (*pWrdList)[ --i ];
            if( LISTBOX_ENTRY_NOTFOUND == m_pDoubleCapsLB->GetEntryPos(aString) )
            {
                pWrdList->erase(i);
            }
        }
        nCount = m_pDoubleCapsLB->GetEntryCount();
        for( i = 0; i < nCount; ++i )
        {
            pWrdList->insert( m_pDoubleCapsLB->GetEntry( i ) );
        }
        pAutoCorrect->SaveWrdSttExceptList(eLang);
    }

    SvStringsISortDtor* pCplList = pAutoCorrect->LoadCplSttExceptList(eLang);

    if(pCplList)
    {
        size_t nCount = pCplList->size();
        for( size_t i = nCount; i; )
        {
            OUString aString = (*pCplList)[ --i ];
            if( LISTBOX_ENTRY_NOTFOUND == m_pAbbrevLB->GetEntryPos(aString) )
            {
                pCplList->erase(i);
            }
        }
        sal_Int32 nAbbrevCount = m_pAbbrevLB->GetEntryCount();
        for( sal_Int32 ia = 0; ia < nAbbrevCount; ++ia )
        {
            pCplList->insert( m_pAbbrevLB->GetEntry( ia ) );
        }
        pAutoCorrect->SaveCplSttExceptList(eLang);
    }
    if(m_pAutoAbbrevCB->IsValueChangedFromSaved())
        pAutoCorrect->SetAutoCorrFlag( ACFlags::SaveWordCplSttLst, m_pAutoAbbrevCB->IsChecked());
    if(m_pAutoCapsCB->IsValueChangedFromSaved())
        pAutoCorrect->SetAutoCorrFlag( ACFlags::SaveWordWrdSttLst, m_pAutoCapsCB->IsChecked());
    return false;
}

void OfaAutocorrExceptPage::SetLanguage(LanguageType eSet)
{
    if(eLang != eSet)
    {
        // save old settings and fill anew
        RefillReplaceBoxes(false, eLang, eSet);
        eLastDialogLanguage = eSet;
        pCompareClass.reset( new CollatorWrapper( comphelper::getProcessComponentContext() ) );
        pCompareClass->loadDefaultCollator( LanguageTag::convertToLocale( eLastDialogLanguage ), 0 );
        ModifyHdl(*m_pAbbrevED);
        ModifyHdl(*m_pDoubleCapsED);
    }
}

void OfaAutocorrExceptPage::RefillReplaceBoxes(bool bFromReset,
                                        LanguageType eOldLanguage,
                                        LanguageType eNewLanguage)
{
    eLang = eNewLanguage;
    if(bFromReset)
    {
        aStringsTable.clear();
    }
    else
    {
        StringsArrays* pArrays;
        if(aStringsTable.find(eOldLanguage) != aStringsTable.end())
        {
            pArrays = &aStringsTable[eOldLanguage];
            pArrays->aAbbrevStrings.clear();
            pArrays->aDoubleCapsStrings.clear();
        }
        else
        {
            pArrays = &aStringsTable[eOldLanguage]; // create new array
        }

        sal_Int32 i;
        for(i = 0; i < m_pAbbrevLB->GetEntryCount(); i++)
            pArrays->aAbbrevStrings.push_back(m_pAbbrevLB->GetEntry(i));

        for(i = 0; i < m_pDoubleCapsLB->GetEntryCount(); i++)
            pArrays->aDoubleCapsStrings.push_back(m_pDoubleCapsLB->GetEntry(i));
    }
    m_pDoubleCapsLB->Clear();
    m_pAbbrevLB->Clear();
    OUString sTemp;
    m_pAbbrevED->SetText(sTemp);
    m_pDoubleCapsED->SetText(sTemp);

    if(aStringsTable.find(eLang) != aStringsTable.end())
    {
        StringsArrays& rArrays = aStringsTable[eLang];
        for (auto const& elem : rArrays.aAbbrevStrings)
            m_pAbbrevLB->InsertEntry(elem);

        for (auto const& elem : rArrays.aDoubleCapsStrings)
            m_pDoubleCapsLB->InsertEntry(elem);
    }
    else
    {
        SvxAutoCorrect* pAutoCorrect = SvxAutoCorrCfg::Get().GetAutoCorrect();
        const SvStringsISortDtor* pCplList = pAutoCorrect->GetCplSttExceptList(eLang);
        const SvStringsISortDtor* pWrdList = pAutoCorrect->GetWrdSttExceptList(eLang);
        size_t i;
        for( i = 0; i < pCplList->size(); i++ )
        {
            m_pAbbrevLB->InsertEntry((*pCplList)[i]);
        }
        for( i = 0; i < pWrdList->size(); i++ )
        {
            m_pDoubleCapsLB->InsertEntry((*pWrdList)[i]);
        }
    }
}

void OfaAutocorrExceptPage::Reset( const SfxItemSet* )
{
    SvxAutoCorrect* pAutoCorrect = SvxAutoCorrCfg::Get().GetAutoCorrect();
    RefillReplaceBoxes(true, eLang, eLang);
    m_pAutoAbbrevCB->  Check(  pAutoCorrect->IsAutoCorrFlag( ACFlags::SaveWordCplSttLst ));
    m_pAutoCapsCB->    Check(  pAutoCorrect->IsAutoCorrFlag( ACFlags::SaveWordWrdSttLst ));
    m_pAutoAbbrevCB->SaveValue();
    m_pAutoCapsCB->SaveValue();
}

IMPL_LINK(OfaAutocorrExceptPage, NewDelButtonHdl, Button*, pBtn, void)
{
    NewDelHdl(pBtn);
}

IMPL_LINK(OfaAutocorrExceptPage, NewDelActionHdl, AutoCorrEdit&, rEdit, bool)
{
    return NewDelHdl(&rEdit);
}

bool OfaAutocorrExceptPage::NewDelHdl(void const * pBtn)
{
    if((pBtn == m_pNewAbbrevPB || pBtn == m_pAbbrevED.get() )
        && !m_pAbbrevED->GetText().isEmpty())
    {
        m_pAbbrevLB->InsertEntry(m_pAbbrevED->GetText());
        ModifyHdl(*m_pAbbrevED);
    }
    else if(pBtn == m_pDelAbbrevPB)
    {
        m_pAbbrevLB->RemoveEntry(m_pAbbrevED->GetText());
        ModifyHdl(*m_pAbbrevED);
    }
    else if((pBtn == m_pNewDoublePB || pBtn == m_pDoubleCapsED.get() )
            && !m_pDoubleCapsED->GetText().isEmpty())
    {
        m_pDoubleCapsLB->InsertEntry(m_pDoubleCapsED->GetText());
        ModifyHdl(*m_pDoubleCapsED);
    }
    else if(pBtn == m_pDelDoublePB)
    {
        m_pDoubleCapsLB->RemoveEntry(m_pDoubleCapsED->GetText());
        ModifyHdl(*m_pDoubleCapsED);
    }
    return false;
}

IMPL_LINK(OfaAutocorrExceptPage, SelectHdl, ListBox&, rBox, void)
{
    if (&rBox == m_pAbbrevLB)
    {
        m_pAbbrevED->SetText(rBox.GetSelectedEntry());
        m_pNewAbbrevPB->Enable(false);
        m_pDelAbbrevPB->Enable();
    }
    else
    {
        m_pDoubleCapsED->SetText(rBox.GetSelectedEntry());
        m_pNewDoublePB->Enable(false);
        m_pDelDoublePB->Enable();
    }
}

IMPL_LINK(OfaAutocorrExceptPage, ModifyHdl, Edit&, rEdt, void)
{
//  sal_Bool bSame = pEdt->GetText() == ->GetSelectedEntry();
    const OUString& sEntry = rEdt.GetText();
    bool bEntryLen = !sEntry.isEmpty();
    if(&rEdt == m_pAbbrevED)
    {
        bool bSame = lcl_FindEntry(*m_pAbbrevLB, sEntry, *pCompareClass);
        if(bSame && sEntry != m_pAbbrevLB->GetSelectedEntry())
            rEdt.SetText(m_pAbbrevLB->GetSelectedEntry());
        m_pNewAbbrevPB->Enable(!bSame && bEntryLen);
        m_pDelAbbrevPB->Enable(bSame && bEntryLen);
    }
    else
    {
        bool bSame = lcl_FindEntry(*m_pDoubleCapsLB, sEntry, *pCompareClass);
        if(bSame && sEntry != m_pDoubleCapsLB->GetSelectedEntry())
            rEdt.SetText(m_pDoubleCapsLB->GetSelectedEntry());
        m_pNewDoublePB->Enable(!bSame && bEntryLen);
        m_pDelDoublePB->Enable(bSame && bEntryLen);
    }
}

VCL_BUILDER_FACTORY(AutoCorrEdit)

void AutoCorrEdit::dispose()
{
    m_xReplaceTLB.disposeAndClear();
    Edit::dispose();
}

AutoCorrEdit::~AutoCorrEdit() {
    disposeOnce();
}

void AutoCorrEdit::ConnectColumn(const VclPtr<SvTabListBox>& rTable, sal_Int32 nCol)
{
    m_xReplaceTLB = rTable;
    m_nCol = nCol;
}

void AutoCorrEdit::KeyInput( const KeyEvent& rKEvt )
{
    const vcl::KeyCode aKeyCode = rKEvt.GetKeyCode();
    const sal_uInt16 nModifier = aKeyCode.GetModifier();
    if( aKeyCode.GetCode() == KEY_RETURN )
    {
        // if there's nothing done on enter, call the
        // base class after all to close the dialog
        if(!nModifier && !aActionLink.Call(*this))
                 Edit::KeyInput(rKEvt);
    }
    else if(bSpaces || aKeyCode.GetCode() != KEY_SPACE)
        Edit::KeyInput(rKEvt);
}

void AutoCorrEdit::Resize()
{
    Edit::Resize();
    if (!m_xReplaceTLB)
        return;
    m_xReplaceTLB->SetTab(m_nCol, GetPosPixel().X(), MapUnit::MapPixel);
}

enum OfaQuoteOptions
{
    ADD_NONBRK_SPACE,
    REPLACE_1ST
};

SvTreeListEntry* OfaQuoteTabPage::CreateEntry(OUString& rTxt, sal_uInt16 nCol)
{
    SvTreeListEntry* pEntry = new SvTreeListEntry;

    if (!m_xCheckButtonData)
    {
        m_xCheckButtonData.reset(new SvLBoxButtonData(m_pSwCheckLB));
        m_pSwCheckLB->SetCheckButtonData(m_xCheckButtonData.get());
    }

    pEntry->AddItem(o3tl::make_unique<SvLBoxContextBmp>(Image(), Image(), false));

    if (nCol == CBCOL_SECOND)
        pEntry->AddItem(o3tl::make_unique<SvLBoxString>(""));
    else
        pEntry->AddItem(o3tl::make_unique<SvLBoxButton>(SvLBoxButtonKind::EnabledCheckbox, m_xCheckButtonData.get()));

    if (nCol == CBCOL_FIRST)
        pEntry->AddItem(o3tl::make_unique<SvLBoxString>(""));
    else
        pEntry->AddItem(o3tl::make_unique<SvLBoxButton>(SvLBoxButtonKind::EnabledCheckbox, m_xCheckButtonData.get()));

    pEntry->AddItem(o3tl::make_unique<OfaImpBrwString>(rTxt));

    return pEntry;
}

OfaQuoteTabPage::OfaQuoteTabPage(vcl::Window* pParent, const SfxItemSet& rSet)
    : SfxTabPage(pParent, "ApplyLocalizedPage", "cui/ui/applylocalizedpage.ui", &rSet)
    , sNonBrkSpace(CuiResId(RID_SVXSTR_NON_BREAK_SPACE))
    , sOrdinal(CuiResId(RID_SVXSTR_ORDINAL))
    , cSglStartQuote(0)
    , cSglEndQuote(0)
    , cStartQuote(0)
    , cEndQuote(0)
{
    get(m_pCheckLB, "checklist");

    SvSimpleTableContainer *pListContainer = get<SvSimpleTableContainer>("list");
    Size aControlSize(252 , 85);
    aControlSize = LogicToPixel(aControlSize, MapMode(MapUnit::MapAppFont));
    pListContainer->set_width_request(aControlSize.Width());
    pListContainer->set_height_request(aControlSize.Height());
    m_pSwCheckLB = VclPtr<OfaACorrCheckListBox>::Create(*pListContainer);

    get(m_pSingleTypoCB, "singlereplace");
    get(m_pSglStartQuotePB, "startsingle");
    get(m_pSglStartExFT, "singlestartex");
    get(m_pSglEndQuotePB, "endsingle");
    get(m_pSglEndExFT, "singleendex");
    get(m_pSglStandardPB, "defaultsingle");

    get(m_pDoubleTypoCB, "doublereplace");
    get(m_pDblStartQuotePB, "startdouble");
    get(m_pDblStartExFT, "doublestartex");
    get(m_pDblEndQuotePB, "enddouble");
    get(m_pDblEndExFT, "doubleendex");
    get(m_pDblStandardPB, "defaultdouble");

    m_sStandard = get<FixedText>("singlestartex")->GetText();

    bool bShowSWOptions = false;

    const SfxBoolItem* pItem = rSet.GetItem<SfxBoolItem>(SID_AUTO_CORRECT_DLG, false);
    if ( pItem && pItem->GetValue() )
        bShowSWOptions = true;

    if ( bShowSWOptions )
    {
        static long const aStaticTabs[]=
        {
            0, 20, 40
        };

        m_pSwCheckLB->SetStyle(m_pSwCheckLB->GetStyle() | WB_HSCROLL| WB_VSCROLL);

        m_pSwCheckLB->SvSimpleTable::SetTabs(SAL_N_ELEMENTS(aStaticTabs), aStaticTabs);
        OUString sHeader = get<vcl::Window>("m")->GetText() + "\t" + get<vcl::Window>("t")->GetText() + "\t";
        m_pSwCheckLB->InsertHeaderEntry( sHeader, HEADERBAR_APPEND,
                        HeaderBarItemBits::CENTER | HeaderBarItemBits::VCENTER | HeaderBarItemBits::FIXEDPOS | HeaderBarItemBits::FIXED);
        m_pCheckLB->Hide();
    }
    else
    {
        m_pSwCheckLB->HideTable();
    }

    m_pDblStartQuotePB->SetClickHdl(LINK(this,    OfaQuoteTabPage, QuoteHdl));
    m_pDblEndQuotePB->SetClickHdl(LINK(this,      OfaQuoteTabPage, QuoteHdl));
    m_pSglStartQuotePB->SetClickHdl(LINK(this, OfaQuoteTabPage, QuoteHdl));
    m_pSglEndQuotePB->SetClickHdl(LINK(this,   OfaQuoteTabPage, QuoteHdl));
    m_pDblStandardPB->SetClickHdl(LINK(this,   OfaQuoteTabPage, StdQuoteHdl));
    m_pSglStandardPB->SetClickHdl(LINK(this,   OfaQuoteTabPage, StdQuoteHdl));
}

OfaQuoteTabPage::~OfaQuoteTabPage()
{
    disposeOnce();
}

void OfaQuoteTabPage::dispose()
{
    m_xCheckButtonData.reset();
    m_pSwCheckLB.disposeAndClear();
    m_pCheckLB.disposeAndClear();
    m_pSingleTypoCB.clear();
    m_pSglStartQuotePB.clear();
    m_pSglStartExFT.clear();
    m_pSglEndQuotePB.clear();
    m_pSglEndExFT.clear();
    m_pSglStandardPB.clear();
    m_pDoubleTypoCB.clear();
    m_pDblStartQuotePB.clear();
    m_pDblStartExFT.clear();
    m_pDblEndQuotePB.clear();
    m_pDblEndExFT.clear();
    m_pDblStandardPB.clear();
    SfxTabPage::dispose();
}

VclPtr<SfxTabPage> OfaQuoteTabPage::Create( TabPageParent pParent,
                                            const SfxItemSet* rAttrSet)
{
    return VclPtr<OfaQuoteTabPage>::Create(pParent.pParent, *rAttrSet);
}

bool OfaQuoteTabPage::FillItemSet( SfxItemSet*  )
{
    SvxAutoCorrect* pAutoCorrect = SvxAutoCorrCfg::Get().GetAutoCorrect();

    ACFlags nFlags = pAutoCorrect->GetFlags();

    if (m_pCheckLB->IsVisible())
    {
        sal_uLong nPos = 0;
        pAutoCorrect->SetAutoCorrFlag(ACFlags::AddNonBrkSpace, m_pCheckLB->IsChecked(nPos++));
        pAutoCorrect->SetAutoCorrFlag(ACFlags::ChgOrdinalNumber, m_pCheckLB->IsChecked(nPos++));
    }

    bool bModified = false;
    if (m_pSwCheckLB->IsVisible())
    {
        SvxSwAutoFormatFlags *pOpt = &pAutoCorrect->GetSwFlags();

        bool bCheck = m_pSwCheckLB->IsChecked(ADD_NONBRK_SPACE);
        bModified |= pOpt->bAddNonBrkSpace != bCheck;
        pOpt->bAddNonBrkSpace = bCheck;
        pAutoCorrect->SetAutoCorrFlag(ACFlags::AddNonBrkSpace,
                            m_pSwCheckLB->IsChecked(ADD_NONBRK_SPACE, CBCOL_SECOND));

        bCheck = m_pSwCheckLB->IsChecked(REPLACE_1ST);
        bModified |= pOpt->bChgOrdinalNumber != bCheck;
        pOpt->bChgOrdinalNumber = bCheck;
        pAutoCorrect->SetAutoCorrFlag(ACFlags::ChgOrdinalNumber,
                        m_pSwCheckLB->IsChecked(REPLACE_1ST, CBCOL_SECOND));
    }

    pAutoCorrect->SetAutoCorrFlag(ACFlags::ChgQuotes, m_pDoubleTypoCB->IsChecked());
    pAutoCorrect->SetAutoCorrFlag(ACFlags::ChgSglQuotes, m_pSingleTypoCB->IsChecked());
    bool bReturn = nFlags != pAutoCorrect->GetFlags();
    if(cStartQuote != pAutoCorrect->GetStartDoubleQuote())
    {
        bReturn = true;
        sal_Unicode cUCS2 = static_cast<sal_Unicode>(cStartQuote); //TODO
        pAutoCorrect->SetStartDoubleQuote(cUCS2);
    }
    if(cEndQuote != pAutoCorrect->GetEndDoubleQuote())
    {
        bReturn = true;
        sal_Unicode cUCS2 = static_cast<sal_Unicode>(cEndQuote); //TODO
        pAutoCorrect->SetEndDoubleQuote(cUCS2);
    }
    if(cSglStartQuote != pAutoCorrect->GetStartSingleQuote())
    {
        bReturn = true;
        sal_Unicode cUCS2 = static_cast<sal_Unicode>(cSglStartQuote); //TODO
        pAutoCorrect->SetStartSingleQuote(cUCS2);
    }
    if(cSglEndQuote != pAutoCorrect->GetEndSingleQuote())
    {
        bReturn = true;
        sal_Unicode cUCS2 = static_cast<sal_Unicode>(cSglEndQuote); //TODO
        pAutoCorrect->SetEndSingleQuote(cUCS2);
    }

    if( bModified || bReturn )
    {
        SvxAutoCorrCfg& rCfg = SvxAutoCorrCfg::Get();
        rCfg.SetModified();
        rCfg.Commit();
    }
    return bReturn;
}

void OfaQuoteTabPage::ActivatePage( const SfxItemSet& )
{
    static_cast<OfaAutoCorrDlg*>(GetTabDialog())->EnableLanguage(false);
}

void OfaQuoteTabPage::Reset( const SfxItemSet* )
{
    SvxAutoCorrect* pAutoCorrect = SvxAutoCorrCfg::Get().GetAutoCorrect();
    const ACFlags nFlags = pAutoCorrect->GetFlags();

    // Initialize the Sw options
    if (m_pSwCheckLB->IsVisible())
    {
        SvxSwAutoFormatFlags *pOpt = &pAutoCorrect->GetSwFlags();

        m_pSwCheckLB->SetUpdateMode( false );
        m_pSwCheckLB->Clear();

        m_pSwCheckLB->GetModel()->Insert(CreateEntry(sNonBrkSpace, CBCOL_BOTH ));
        m_pSwCheckLB->GetModel()->Insert(CreateEntry(sOrdinal, CBCOL_BOTH ));

        m_pSwCheckLB->CheckEntryPos( ADD_NONBRK_SPACE, CBCOL_FIRST,    pOpt->bAddNonBrkSpace );
        m_pSwCheckLB->CheckEntryPos( ADD_NONBRK_SPACE, CBCOL_SECOND,   bool(nFlags & ACFlags::AddNonBrkSpace) );
        m_pSwCheckLB->CheckEntryPos( REPLACE_1ST, CBCOL_FIRST,    pOpt->bChgOrdinalNumber );
        m_pSwCheckLB->CheckEntryPos( REPLACE_1ST, CBCOL_SECOND,   bool(nFlags & ACFlags::ChgOrdinalNumber) );

        m_pSwCheckLB->SetUpdateMode( true );
    }

    // Initialize the non Sw options
    if (m_pCheckLB->IsVisible())
    {
        m_pCheckLB->SetUpdateMode(false);
        m_pCheckLB->Clear();

        m_pCheckLB->InsertEntry( sNonBrkSpace );
        m_pCheckLB->InsertEntry( sOrdinal );

        sal_uLong nPos = 0;
        m_pCheckLB->CheckEntryPos( nPos++, bool(nFlags & ACFlags::AddNonBrkSpace) );
        m_pCheckLB->CheckEntryPos( nPos++, bool(nFlags & ACFlags::ChgOrdinalNumber) );

        m_pCheckLB->SetUpdateMode(true);
    }

    // Initialize the quote stuffs
    m_pDoubleTypoCB->Check(bool(nFlags & ACFlags::ChgQuotes));
    m_pSingleTypoCB->Check(bool(nFlags & ACFlags::ChgSglQuotes));
    m_pDoubleTypoCB->SaveValue();
    m_pSingleTypoCB->SaveValue();

    cStartQuote = pAutoCorrect->GetStartDoubleQuote();
    cEndQuote = pAutoCorrect->GetEndDoubleQuote();
    cSglStartQuote = pAutoCorrect->GetStartSingleQuote();
    cSglEndQuote = pAutoCorrect->GetEndSingleQuote();

    m_pSglStartExFT->SetText(ChangeStringExt_Impl(cSglStartQuote));
    m_pSglEndExFT->SetText(ChangeStringExt_Impl(cSglEndQuote));
    m_pDblStartExFT->SetText(ChangeStringExt_Impl(cStartQuote));
    m_pDblEndExFT->SetText(ChangeStringExt_Impl(cEndQuote));
}

#define SGL_START       0
#define DBL_START       1
#define SGL_END         2
#define DBL_END         3


IMPL_LINK( OfaQuoteTabPage, QuoteHdl, Button*, pBtn, void )
{
    sal_uInt16 nMode = SGL_START;
    if (pBtn == m_pSglEndQuotePB)
        nMode = SGL_END;
    else if (pBtn == m_pDblStartQuotePB)
        nMode = DBL_START;
    else if (pBtn == m_pDblEndQuotePB)
        nMode = DBL_END;
    // start character selection dialog
    SvxCharacterMap aMap(GetFrameWeld(), nullptr, false);
    aMap.SetCharFont( OutputDevice::GetDefaultFont(DefaultFontType::LATIN_TEXT,
                        LANGUAGE_ENGLISH_US, GetDefaultFontFlags::OnlyOne ));
    aMap.set_title(nMode < SGL_END ? CuiResId(RID_SVXSTR_STARTQUOTE)  : CuiResId(RID_SVXSTR_ENDQUOTE));
    sal_UCS4 cDlg;
    SvxAutoCorrect* pAutoCorrect = SvxAutoCorrCfg::Get().GetAutoCorrect();
    LanguageType eLang = Application::GetSettings().GetLanguageTag().getLanguageType();
    switch( nMode )
    {
        case SGL_START:
            cDlg = cSglStartQuote;
            if(cDlg == 0)
                cDlg = pAutoCorrect->GetQuote('\'', true, eLang);
        break;
        case SGL_END:
            cDlg = cSglEndQuote;
            if(cDlg == 0)
                cDlg = pAutoCorrect->GetQuote('\'', false, eLang);
        break;
        case DBL_START:
            cDlg = cStartQuote;
            if(cDlg == 0)
                cDlg = pAutoCorrect->GetQuote('\"', true, eLang);
        break;
        case DBL_END:
            cDlg = cEndQuote;
            if(cDlg == 0)
                cDlg = pAutoCorrect->GetQuote('\"', false, eLang);
        break;
        default:
            OSL_FAIL("svx::OfaQuoteTabPage::QuoteHdl(), how to initialize cDlg?" );
            cDlg = 0;
            break;

    }
    aMap.SetChar(  cDlg );
    aMap.DisableFontSelection();
    if (aMap.execute() == RET_OK)
    {
        sal_UCS4 cNewChar = aMap.GetChar();
        switch( nMode )
        {
            case SGL_START:
                cSglStartQuote = cNewChar;
                m_pSglStartExFT->SetText(ChangeStringExt_Impl(cNewChar));
            break;
            case SGL_END:
                cSglEndQuote = cNewChar;
                m_pSglEndExFT->SetText(ChangeStringExt_Impl(cNewChar));
            break;
            case DBL_START:
                cStartQuote = cNewChar;
                m_pDblStartExFT->SetText(ChangeStringExt_Impl(cNewChar));
            break;
            case DBL_END:
                cEndQuote = cNewChar;
                m_pDblEndExFT->SetText(ChangeStringExt_Impl(cNewChar));
            break;
        }
    }
}

IMPL_LINK( OfaQuoteTabPage, StdQuoteHdl, Button*, pBtn, void )
{
    if (pBtn == m_pDblStandardPB)
    {
        cStartQuote = 0;
        m_pDblStartExFT->SetText(ChangeStringExt_Impl(0));
        cEndQuote = 0;
        m_pDblEndExFT->SetText(ChangeStringExt_Impl(0));

    }
    else
    {
        cSglStartQuote = 0;
        m_pSglStartExFT->SetText(ChangeStringExt_Impl(0));
        cSglEndQuote = 0;
        m_pSglEndExFT->SetText(ChangeStringExt_Impl(0));
    }
}


OUString OfaQuoteTabPage::ChangeStringExt_Impl( sal_UCS4 cChar )
{
    if (!cChar)
        return m_sStandard;

    // convert codepoint value to unicode-hex string
    sal_UCS4 aStrCodes[32] = { 0, ' ', '(', 'U', '+', '0' };
    aStrCodes[0] = cChar;
    int nFullLen = 5;
    int nHexLen = 4;
    while( (cChar >> (4*nHexLen)) != 0 )
        ++nHexLen;
    for( int i = nHexLen; --i >= 0;)
    {
        sal_UCS4 cHexDigit = ((cChar >> (4*i)) & 0x0f) + '0';
        if( cHexDigit > '9' )
            cHexDigit += 'A' - ('9' + 1);
        aStrCodes[ nFullLen++ ] = cHexDigit;
    }
    aStrCodes[ nFullLen++ ] = ')';
    // using the new UCS4 constructor
    OUString aOUStr( aStrCodes, nFullLen );
    return aOUStr;
}

OfaAutoCompleteTabPage::OfaAutoCompleteTabPage(vcl::Window* pParent,
    const SfxItemSet& rSet)
    : SfxTabPage(pParent, "WordCompletionPage",
        "cui/ui/wordcompletionpage.ui", &rSet)
    , m_pAutoCompleteList(nullptr)
    , m_nAutoCmpltListCnt(0)
{
    get(m_pCBActiv, "enablewordcomplete");
    get(m_pCBAppendSpace, "appendspace");
    get(m_pCBAsTip, "showastip");
    get(m_pCBCollect, "collectwords");
    get(m_pCBRemoveList, "whenclosing");

    //fdo#65595, we need height-for-width support here, but for now we can
    //bodge it
    Size aPrefSize(m_pCBRemoveList->get_preferred_size());
    Size aSize(m_pCBRemoveList->CalcMinimumSize(32*approximate_char_width()));
    if (aPrefSize.Width() > aSize.Width())
    {
        m_pCBRemoveList->set_width_request(aSize.Width());
        m_pCBRemoveList->set_height_request(aSize.Height());
    }

    get(m_pDCBExpandKey, "acceptwith");
    get(m_pNFMinWordlen, "minwordlen");
    get(m_pNFMaxEntries, "maxentries");
    get(m_pLBEntries, "entries");
    m_pLBEntries->SetPage(this);
    aSize = LogicToPixel(Size(121, 158), MapMode(MapUnit::MapAppFont));
    m_pLBEntries->set_width_request(aSize.Width());
    m_pLBEntries->set_height_request(aSize.Height());
    get(m_pPBEntries, "delete");

    // the defined KEYs
    static const sal_uInt16 aKeyCodes[] = {
        KEY_END,
        KEY_RETURN,
        KEY_SPACE,
        KEY_RIGHT,
        KEY_TAB,
        0
    };

    for( const sal_uInt16* pKeys = aKeyCodes; *pKeys; ++pKeys )
    {
        vcl::KeyCode aKCode( *pKeys );
        sal_Int32 nPos = m_pDCBExpandKey->InsertEntry( aKCode.GetName() );
        m_pDCBExpandKey->SetEntryData( nPos, reinterpret_cast<void*>(*pKeys) );
        if( KEY_RETURN == *pKeys )      // default to RETURN
            m_pDCBExpandKey->SelectEntryPos( nPos );
    }

    m_pPBEntries->SetClickHdl(LINK(this, OfaAutoCompleteTabPage, DeleteHdl));
    m_pCBActiv->SetToggleHdl(LINK(this, OfaAutoCompleteTabPage, CheckHdl));
    m_pCBCollect->SetToggleHdl(LINK(this, OfaAutoCompleteTabPage, CheckHdl));
}

OfaAutoCompleteTabPage::~OfaAutoCompleteTabPage()
{
    disposeOnce();
}

void OfaAutoCompleteTabPage::dispose()
{
    m_pCBActiv.clear();
    m_pCBAppendSpace.clear();
    m_pCBAsTip.clear();
    m_pCBCollect.clear();
    m_pCBRemoveList.clear();
    m_pDCBExpandKey.clear();
    m_pNFMinWordlen.clear();
    m_pNFMaxEntries.clear();
    m_pLBEntries.clear();
    m_pPBEntries.clear();
    SfxTabPage::dispose();
}

VclPtr<SfxTabPage> OfaAutoCompleteTabPage::Create( TabPageParent pParent,
                                                   const SfxItemSet* rSet)
{
    return VclPtr<OfaAutoCompleteTabPage>::Create( pParent.pParent, *rSet );
}

bool OfaAutoCompleteTabPage::FillItemSet( SfxItemSet* )
{
    bool bModified = false, bCheck;
    SvxAutoCorrect* pAutoCorrect = SvxAutoCorrCfg::Get().GetAutoCorrect();
    SvxSwAutoFormatFlags *pOpt = &pAutoCorrect->GetSwFlags();
    sal_uInt16 nVal;

    bCheck = m_pCBActiv->IsChecked();
    bModified |= pOpt->bAutoCompleteWords != bCheck;
    pOpt->bAutoCompleteWords = bCheck;
    bCheck = m_pCBCollect->IsChecked();
    bModified |= pOpt->bAutoCmpltCollectWords != bCheck;
    pOpt->bAutoCmpltCollectWords = bCheck;
    bCheck = !m_pCBRemoveList->IsChecked(); // inverted value!
    bModified |= pOpt->bAutoCmpltKeepList != bCheck;
    pOpt->bAutoCmpltKeepList = bCheck;
    bCheck = m_pCBAppendSpace->IsChecked();
    bModified |= pOpt->bAutoCmpltAppendBlanc != bCheck;
    pOpt->bAutoCmpltAppendBlanc = bCheck;
    bCheck = m_pCBAsTip->IsChecked();
    bModified |= pOpt->bAutoCmpltShowAsTip != bCheck;
    pOpt->bAutoCmpltShowAsTip = bCheck;

    nVal = static_cast<sal_uInt16>(m_pNFMinWordlen->GetValue());
    bModified |= nVal != pOpt->nAutoCmpltWordLen;
    pOpt->nAutoCmpltWordLen = nVal;

    nVal = static_cast<sal_uInt16>(m_pNFMaxEntries->GetValue());
    bModified |= nVal != pOpt->nAutoCmpltListLen;
    pOpt->nAutoCmpltListLen = nVal;

    const sal_Int32 nPos = m_pDCBExpandKey->GetSelectedEntryPos();
    if( nPos < m_pDCBExpandKey->GetEntryCount() )
    {
        sal_uLong nKey = reinterpret_cast<sal_uLong>(m_pDCBExpandKey->GetEntryData( nPos ));
        bModified |= nKey != pOpt->nAutoCmpltExpandKey;
        pOpt->nAutoCmpltExpandKey = static_cast<sal_uInt16>(nKey);
    }

    if (m_pAutoCompleteList && m_nAutoCmpltListCnt != m_pLBEntries->GetEntryCount())
    {
        bModified = true;
        pOpt->m_pAutoCompleteList = m_pAutoCompleteList;
    }
    if( bModified )
    {
        SvxAutoCorrCfg& rCfg = SvxAutoCorrCfg::Get();
        rCfg.SetModified();
        rCfg.Commit();
    }
    return true;
}

void OfaAutoCompleteTabPage::Reset( const SfxItemSet*  )
{
    SvxAutoCorrect* pAutoCorrect = SvxAutoCorrCfg::Get().GetAutoCorrect();
    SvxSwAutoFormatFlags *pOpt = &pAutoCorrect->GetSwFlags();

    m_pCBActiv->Check( pOpt->bAutoCompleteWords );
    m_pCBCollect->Check( pOpt->bAutoCmpltCollectWords );
    m_pCBRemoveList->Check( !pOpt->bAutoCmpltKeepList ); //inverted value!
    m_pCBAppendSpace->Check( pOpt->bAutoCmpltAppendBlanc );
    m_pCBAsTip->Check( pOpt->bAutoCmpltShowAsTip );

    m_pNFMinWordlen->SetValue( pOpt->nAutoCmpltWordLen );
    m_pNFMaxEntries->SetValue( pOpt->nAutoCmpltListLen );

    // select the specific KeyCode:
    {
        sal_uLong nKey = pOpt->nAutoCmpltExpandKey;
        for( sal_Int32 n = 0, nCnt = m_pDCBExpandKey->GetEntryCount(); n < nCnt; ++n )
            if( nKey == reinterpret_cast<sal_uLong>(m_pDCBExpandKey->GetEntryData( n )))
            {
                m_pDCBExpandKey->SelectEntryPos( n );
                break;
            }
    }

    if (pOpt->m_pAutoCompleteList && pOpt->m_pAutoCompleteList->size())
    {
        m_pAutoCompleteList = const_cast<editeng::SortedAutoCompleteStrings*>(
                pOpt->m_pAutoCompleteList);
        pOpt->m_pAutoCompleteList = nullptr;
        m_nAutoCmpltListCnt = m_pAutoCompleteList->size();
        for (size_t n = 0; n < m_nAutoCmpltListCnt; ++n)
        {
            const OUString* pStr =
                &(*m_pAutoCompleteList)[n]->GetAutoCompleteString();
            sal_Int32 nPos = m_pLBEntries->InsertEntry( *pStr );
            m_pLBEntries->SetEntryData( nPos, const_cast<OUString *>(pStr) );
        }
    }
    else
    {
        m_pLBEntries->Disable();
        m_pPBEntries->Disable();
    }

    CheckHdl(*m_pCBActiv);
    CheckHdl(*m_pCBCollect);
}

void OfaAutoCompleteTabPage::ActivatePage( const SfxItemSet& )
{
    static_cast<OfaAutoCorrDlg*>(GetTabDialog())->EnableLanguage( false );
}

IMPL_LINK_NOARG(OfaAutoCompleteTabPage, DeleteHdl, Button*, void)
{
    sal_Int32 nSelCnt =
        (m_pAutoCompleteList) ? m_pLBEntries->GetSelectedEntryCount() : 0;
    while( nSelCnt )
    {
        sal_Int32 nPos = m_pLBEntries->GetSelectedEntryPos( --nSelCnt );
        OUString* pStr = static_cast<OUString*>(m_pLBEntries->GetEntryData(nPos));
        m_pLBEntries->RemoveEntry( nPos );
        editeng::IAutoCompleteString hack(*pStr); // UGLY
        m_pAutoCompleteList->erase(&hack);
    }
}

IMPL_LINK( OfaAutoCompleteTabPage, CheckHdl, CheckBox&, rBox, void )
{
    bool bEnable = rBox.IsChecked();
    if (&rBox == m_pCBActiv)
    {
        m_pCBAppendSpace->Enable( bEnable );
        m_pCBAppendSpace->Enable( bEnable );
        m_pCBAsTip->Enable( bEnable );
        m_pDCBExpandKey->Enable( bEnable );
    }
    else if (m_pCBCollect == &rBox)
        m_pCBRemoveList->Enable( bEnable );
}

void OfaAutoCompleteTabPage::CopyToClipboard() const
{
    sal_Int32 nSelCnt = m_pLBEntries->GetSelectedEntryCount();
    if (m_pAutoCompleteList && nSelCnt)
    {
        rtl::Reference<TransferDataContainer> pCntnr = new TransferDataContainer;

        OStringBuffer sData;

        rtl_TextEncoding nEncode = osl_getThreadTextEncoding();

        for( sal_Int32 n = 0; n < nSelCnt; ++n )
        {
            sData.append(OUStringToOString(m_pLBEntries->GetSelectedEntry(n),
                nEncode));
#if defined(_WIN32)
            sData.append("\015\012");
#else
            sData.append("\012");
#endif
        }
        pCntnr->CopyByteString( SotClipboardFormatId::STRING, sData.makeStringAndClear() );
        pCntnr->CopyToClipboard( static_cast<vcl::Window*>(const_cast<OfaAutoCompleteTabPage *>(this)) );
    }
}

OfaAutoCompleteTabPage::AutoCompleteMultiListBox::~AutoCompleteMultiListBox()
{
    disposeOnce();
}

void OfaAutoCompleteTabPage::AutoCompleteMultiListBox::dispose()
{
    m_pPage.clear();
    MultiListBox::dispose();
}

bool OfaAutoCompleteTabPage::AutoCompleteMultiListBox::PreNotify(
            NotifyEvent& rNEvt )
{
    bool bHandled = MultiListBox::PreNotify( rNEvt );

    if( !bHandled && MouseNotifyEvent::KEYUP == rNEvt.GetType() )
    {
        const vcl::KeyCode& rKeyCode = rNEvt.GetKeyEvent()->GetKeyCode();
        switch( rKeyCode.GetModifier() | rKeyCode.GetCode() )
        {
        case KEY_DELETE:
            m_pPage->DeleteHdl( nullptr );
            bHandled = true;
            break;

        default:
            if( KeyFuncType::COPY == rKeyCode.GetFunction() )
            {
                m_pPage->CopyToClipboard();
                bHandled = true;
            }
            break;
        }
    }
    return bHandled;
}

extern "C" SAL_DLLPUBLIC_EXPORT void makeAutoCompleteMultiListBox(VclPtr<vcl::Window> & rRet, VclPtr<vcl::Window> & pParent, VclBuilder::stringmap & rMap)
{
    WinBits nWinBits = WB_TABSTOP;

    OUString sBorder = BuilderUtils::extractCustomProperty(rMap);
    if (!sBorder.isEmpty())
       nWinBits |= WB_BORDER;

    rRet = VclPtr<OfaAutoCompleteTabPage::AutoCompleteMultiListBox>::Create(pParent, nWinBits);
}

// class OfaSmartTagOptionsTabPage ---------------------------------------------

OfaSmartTagOptionsTabPage::OfaSmartTagOptionsTabPage( vcl::Window* pParent,
                                                      const SfxItemSet& rSet )
    : SfxTabPage(pParent, "SmartTagOptionsPage", "cui/ui/smarttagoptionspage.ui", &rSet)
{
    get(m_pMainCB, "main");
    get(m_pSmartTagTypesLB, "list");
    get(m_pPropertiesPB, "properties");

    // some options for the list box:
    m_pSmartTagTypesLB->SetStyle( m_pSmartTagTypesLB->GetStyle() | WB_HSCROLL | WB_HIDESELECTION );
    m_pSmartTagTypesLB->SetHighlightRange();
    Size aControlSize(LogicToPixel(Size(172, 154), MapMode(MapUnit::MapAppFont)));
    m_pSmartTagTypesLB->set_width_request(aControlSize.Width());
    m_pSmartTagTypesLB->set_height_request(aControlSize.Height());

    // set the handlers:
    m_pMainCB->SetToggleHdl(LINK(this, OfaSmartTagOptionsTabPage, CheckHdl));
    m_pPropertiesPB->SetClickHdl(LINK(this, OfaSmartTagOptionsTabPage, ClickHdl));
    m_pSmartTagTypesLB->SetSelectHdl(LINK(this, OfaSmartTagOptionsTabPage, SelectHdl));
}

OfaSmartTagOptionsTabPage::~OfaSmartTagOptionsTabPage()
{
    disposeOnce();
}

void OfaSmartTagOptionsTabPage::dispose()
{
    m_pMainCB.clear();
    m_pSmartTagTypesLB.clear();
    m_pPropertiesPB.clear();
    SfxTabPage::dispose();
}

VclPtr<SfxTabPage> OfaSmartTagOptionsTabPage::Create( TabPageParent pParent, const SfxItemSet* rSet)
{
    return VclPtr<OfaSmartTagOptionsTabPage>::Create( pParent.pParent, *rSet );
}

/** This struct is used to associate list box entries with smart tag data
*/
struct ImplSmartTagLBUserData
{
    OUString maSmartTagType;
    uno::Reference< smarttags::XSmartTagRecognizer > mxRec;
    sal_Int32 mnSmartTagIdx;

    ImplSmartTagLBUserData( const OUString& rSmartTagType,
                            uno::Reference< smarttags::XSmartTagRecognizer > const & xRec,
                            sal_Int32 nSmartTagIdx ) :
        maSmartTagType( rSmartTagType ),
        mxRec( xRec ),
        mnSmartTagIdx( nSmartTagIdx ) {}
};

/** Clears m_pSmartTagTypesLB
*/
void OfaSmartTagOptionsTabPage::ClearListBox()
{
    const sal_uLong nCount = m_pSmartTagTypesLB->GetEntryCount();
    for ( sal_uLong i = 0; i < nCount; ++i )
    {
        const SvTreeListEntry* pEntry = m_pSmartTagTypesLB->GetEntry(i);
        const ImplSmartTagLBUserData* pUserData = static_cast< ImplSmartTagLBUserData* >(pEntry->GetUserData());
        delete pUserData;
    }

    m_pSmartTagTypesLB->Clear();
}

/** Inserts items into m_pSmartTagTypesLB
*/
void OfaSmartTagOptionsTabPage::FillListBox( const SmartTagMgr& rSmartTagMgr )
{
    // first we have to clear the list box:
    ClearListBox();

    // fill list box:
    const sal_uInt32 nNumberOfRecognizers = rSmartTagMgr.NumberOfRecognizers();
    const lang::Locale aLocale( LanguageTag::convertToLocale( eLastDialogLanguage ) );

    for ( sal_uInt32 i = 0; i < nNumberOfRecognizers; ++i )
    {
        uno::Reference< smarttags::XSmartTagRecognizer > xRec = rSmartTagMgr.GetRecognizer(i);

        const OUString aName = xRec->getName( aLocale );
        const sal_Int32 nNumberOfSupportedSmartTags = xRec->getSmartTagCount();

        for ( sal_Int32 j = 0; j < nNumberOfSupportedSmartTags; ++j )
        {
            const OUString aSmartTagType = xRec->getSmartTagName(j);
            OUString aSmartTagCaption = rSmartTagMgr.GetSmartTagCaption( aSmartTagType, aLocale );

            if ( aSmartTagCaption.isEmpty() )
                aSmartTagCaption = aSmartTagType;

            const OUString aLBEntry = aSmartTagCaption + " (" + aName + ")";

            SvTreeListEntry* pEntry = m_pSmartTagTypesLB->SvTreeListBox::InsertEntry( aLBEntry );
            if ( pEntry )
            {
                const bool bCheck = rSmartTagMgr.IsSmartTagTypeEnabled( aSmartTagType );
                m_pSmartTagTypesLB->SetCheckButtonState( pEntry, bCheck ? SvButtonState::Checked : SvButtonState::Unchecked );
                pEntry->SetUserData(static_cast<void*>(new ImplSmartTagLBUserData( aSmartTagType, xRec, j ) ) );
            }
        }
    }
}

/** Handler for the push button
*/
IMPL_LINK_NOARG(OfaSmartTagOptionsTabPage, ClickHdl, Button*, void)
{
    const sal_uLong nPos = m_pSmartTagTypesLB->GetSelectedEntryPos();
    const SvTreeListEntry* pEntry = m_pSmartTagTypesLB->GetEntry(nPos);
    const ImplSmartTagLBUserData* pUserData = static_cast< ImplSmartTagLBUserData* >(pEntry->GetUserData());
    uno::Reference< smarttags::XSmartTagRecognizer > xRec = pUserData->mxRec;
    const sal_Int32 nSmartTagIdx = pUserData->mnSmartTagIdx;

     const lang::Locale aLocale( LanguageTag::convertToLocale( eLastDialogLanguage ) );
    if ( xRec->hasPropertyPage( nSmartTagIdx, aLocale ) )
        xRec->displayPropertyPage( nSmartTagIdx, aLocale );
}

/** Handler for the check box
*/
IMPL_LINK_NOARG(OfaSmartTagOptionsTabPage, CheckHdl, CheckBox&, void)
{
    const bool bEnable = m_pMainCB->IsChecked();
    m_pSmartTagTypesLB->Enable( bEnable );
    m_pSmartTagTypesLB->Invalidate();
    m_pPropertiesPB->Enable( false );

    // if the controls are currently enabled, we still have to check
    // if the properties button should be disabled because the currently
    // selected smart tag type does not have a properties dialog.
    // We do this by calling SelectHdl:
    if ( bEnable )
        SelectHdl(m_pSmartTagTypesLB);
}

/** Handler for the list box
*/
IMPL_LINK_NOARG(OfaSmartTagOptionsTabPage, SelectHdl, SvTreeListBox*, void)
{
    if ( m_pSmartTagTypesLB->GetEntryCount() < 1 )
        return;

    const sal_uLong nPos = m_pSmartTagTypesLB->GetSelectedEntryPos();
    const SvTreeListEntry* pEntry = m_pSmartTagTypesLB->GetEntry(nPos);
    const ImplSmartTagLBUserData* pUserData = static_cast< ImplSmartTagLBUserData* >(pEntry->GetUserData());
    uno::Reference< smarttags::XSmartTagRecognizer > xRec = pUserData->mxRec;
    const sal_Int32 nSmartTagIdx = pUserData->mnSmartTagIdx;

    const lang::Locale aLocale( LanguageTag::convertToLocale( eLastDialogLanguage ) );
    if ( xRec->hasPropertyPage( nSmartTagIdx, aLocale ) )
        m_pPropertiesPB->Enable();
    else
        m_pPropertiesPB->Enable( false );
}

/** Propagates the current settings to the smart tag manager.
*/
bool OfaSmartTagOptionsTabPage::FillItemSet( SfxItemSet* )
{
    SvxAutoCorrect* pAutoCorrect = SvxAutoCorrCfg::Get().GetAutoCorrect();
    SvxSwAutoFormatFlags *pOpt = &pAutoCorrect->GetSwFlags();
    SmartTagMgr* pSmartTagMgr = pOpt->pSmartTagMgr;

    // robust!
    if ( !pSmartTagMgr )
        return false;

    bool bModifiedSmartTagTypes = false;
    std::vector< OUString > aDisabledSmartTagTypes;

    const sal_uLong nCount = m_pSmartTagTypesLB->GetEntryCount();

    for ( sal_uLong i = 0; i < nCount; ++i )
    {
        const SvTreeListEntry* pEntry = m_pSmartTagTypesLB->GetEntry(i);
        const ImplSmartTagLBUserData* pUserData = static_cast< ImplSmartTagLBUserData* >(pEntry->GetUserData());
        const bool bChecked = m_pSmartTagTypesLB->IsChecked(i);
        const bool bIsCurrentlyEnabled = pSmartTagMgr->IsSmartTagTypeEnabled( pUserData->maSmartTagType );

        bModifiedSmartTagTypes = bModifiedSmartTagTypes || ( !bChecked != !bIsCurrentlyEnabled );

        if ( !bChecked )
            aDisabledSmartTagTypes.push_back( pUserData->maSmartTagType );

        delete pUserData;
    }

    const bool bModifiedRecognize = ( !m_pMainCB->IsChecked() != !pSmartTagMgr->IsLabelTextWithSmartTags() );
    if ( bModifiedSmartTagTypes || bModifiedRecognize )
    {
        bool bLabelTextWithSmartTags = m_pMainCB->IsChecked();
        pSmartTagMgr->WriteConfiguration( bModifiedRecognize     ? &bLabelTextWithSmartTags : nullptr,
                                          bModifiedSmartTagTypes ? &aDisabledSmartTagTypes : nullptr );
    }

    return true;
}

/** Sets the controls based on the current settings at SmartTagMgr.
*/
void OfaSmartTagOptionsTabPage::Reset( const SfxItemSet*  )
{
    SvxAutoCorrect* pAutoCorrect = SvxAutoCorrCfg::Get().GetAutoCorrect();
    SvxSwAutoFormatFlags *pOpt = &pAutoCorrect->GetSwFlags();
    const SmartTagMgr* pSmartTagMgr = pOpt->pSmartTagMgr;

    // robust, should not happen!
    if ( !pSmartTagMgr )
        return;

    FillListBox( *pSmartTagMgr );
    m_pSmartTagTypesLB->SelectEntryPos( 0 );
    m_pMainCB->Check( pSmartTagMgr->IsLabelTextWithSmartTags() );
    CheckHdl(*m_pMainCB);
}

void OfaSmartTagOptionsTabPage::ActivatePage( const SfxItemSet& )
{
    static_cast<OfaAutoCorrDlg*>(GetTabDialog())->EnableLanguage( false );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
