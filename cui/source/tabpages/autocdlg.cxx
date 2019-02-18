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
#include <vcl/transfer.hxx>
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
#include <vcl/svlbitm.hxx>
#include <svx/SmartTagMgr.hxx>
#include <com/sun/star/smarttags/XSmartTagRecognizer.hpp>
#include <com/sun/star/smarttags/XSmartTagAction.hpp>
#include <rtl/strbuf.hxx>

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

OfaAutoCorrDlg::OfaAutoCorrDlg(weld::Window* pParent, const SfxItemSet* _pSet )
    : SfxTabDialogController(pParent, "cui/ui/autocorrectdialog.ui", "AutoCorrectDialog", _pSet)
    , m_xLanguageBox(m_xBuilder->weld_widget("langbox"))
    , m_xLanguageLB(new LanguageBox(m_xBuilder->weld_combo_box("lang")))
{
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
    AddTabPage("applypage", OfaSwAutoFmtOptionsPage::Create, nullptr);
    AddTabPage("wordcompletion", OfaAutoCompleteTabPage::Create, nullptr);
    AddTabPage("smarttags", OfaSmartTagOptionsTabPage::Create, nullptr);

    if (!bShowSWOptions)
    {
        RemoveTabPage("applypage");
        RemoveTabPage("wordcompletion");
        RemoveTabPage("smarttags");
    }
    else
    {
        // remove smart tag tab page if no extensions are installed
        SvxAutoCorrect* pAutoCorrect = SvxAutoCorrCfg::Get().GetAutoCorrect();
        SvxSwAutoFormatFlags& rOpt = pAutoCorrect->GetSwFlags();
        if (!rOpt.pSmartTagMgr || 0 == rOpt.pSmartTagMgr->NumberOfRecognizers())
            RemoveTabPage("smarttags");

        RemoveTabPage("options");
    }

    AddTabPage("replace", OfaAutocorrReplacePage::Create, nullptr);
    AddTabPage("exceptions",  OfaAutocorrExceptPage::Create, nullptr);
    AddTabPage("localized", OfaQuoteTabPage::Create, nullptr);

    // initialize languages
    //! LANGUAGE_NONE is displayed as '[All]' and the LanguageType
    //! will be set to LANGUAGE_UNDETERMINED
    SvxLanguageListFlags nLangList = SvxLanguageListFlags::WESTERN;

    if( SvtLanguageOptions().IsCTLFontEnabled() )
        nLangList |= SvxLanguageListFlags::CTL;
    if( SvtLanguageOptions().IsCJKFontEnabled() )
        nLangList |= SvxLanguageListFlags::CJK;
    m_xLanguageLB->SetLanguageList( nLangList, true, true );
    m_xLanguageLB->set_active_id( LANGUAGE_NONE );
    int nPos = m_xLanguageLB->get_active();
    DBG_ASSERT(nPos != -1, "listbox entry missing" );
    m_xLanguageLB->set_id(nPos, LANGUAGE_UNDETERMINED);

    // Initializing doesn't work for static on linux - therefore here
    if (LANGUAGE_SYSTEM == eLastDialogLanguage)
        eLastDialogLanguage = Application::GetSettings().GetLanguageTag().getLanguageType();

    LanguageType nSelectLang = LANGUAGE_UNDETERMINED;
    nPos = m_xLanguageLB->find_id(eLastDialogLanguage);
    if (nPos != -1)
        nSelectLang = eLastDialogLanguage;
    m_xLanguageLB->set_active_id(nSelectLang);

    m_xLanguageLB->connect_changed(LINK(this, OfaAutoCorrDlg, SelectLanguageHdl));

    if ( bOpenSmartTagOptions )
        SetCurPageId("smarttags");
}

OfaAutoCorrDlg::~OfaAutoCorrDlg()
{
}

void OfaAutoCorrDlg::EnableLanguage(bool bEnable)
{
    m_xLanguageBox->set_sensitive(bEnable);
}

static bool lcl_FindEntry(weld::TreeView& rLB, const OUString& rEntry,
                          CollatorWrapper const & rCmpClass)
{
    int nCount = rLB.n_children();
    int nSelPos = rLB.get_selected_index();
    for (int i = 0; i < nCount; i++)
    {
        if (0 == rCmpClass.compareString(rEntry, rLB.get_text(i)))
        {
            rLB.select(i);
            return true;
        }
    }
    if (nSelPos != -1)
        rLB.unselect(nSelPos);
    return false;
}

IMPL_LINK_NOARG(OfaAutoCorrDlg, SelectLanguageHdl, weld::ComboBox&, void)
{
    LanguageType eNewLang = m_xLanguageLB->get_active_id();
    // save old settings and fill anew
    if(eNewLang != eLastDialogLanguage)
    {
        OString sPageId = GetCurPageId();
        if (sPageId == "replace")
            static_cast<OfaAutocorrReplacePage*>(GetTabPage(sPageId))->SetLanguage(eNewLang);
        else if (sPageId == "exceptions")
            static_cast<OfaAutocorrExceptPage*>(GetTabPage(sPageId))->SetLanguage(eNewLang);
    }
}

OfaAutocorrOptionsPage::OfaAutocorrOptionsPage(TabPageParent pParent, const SfxItemSet& rSet)
    : SfxTabPage(pParent, "cui/ui/acoroptionspage.ui", "AutocorrectOptionsPage", &rSet)
    , m_sInput(CuiResId(RID_SVXSTR_USE_REPLACE))
    , m_sDoubleCaps(CuiResId(RID_SVXSTR_CPTL_STT_WORD))
    , m_sStartCap(CuiResId(RID_SVXSTR_CPTL_STT_SENT))
    , m_sBoldUnderline(CuiResId(RID_SVXSTR_BOLD_UNDER))
    , m_sURL(CuiResId(RID_SVXSTR_DETECT_URL))
    , m_sNoDblSpaces(CuiResId(RID_SVXSTR_NO_DBL_SPACES))
    , m_sDash(CuiResId(RID_SVXSTR_DASH))
    , m_sAccidentalCaps(CuiResId(RID_SVXSTR_CORRECT_ACCIDENTAL_CAPS_LOCK))
    , m_xCheckLB(m_xBuilder->weld_tree_view("checklist"))
{
    std::vector<int> aWidths;
    aWidths.push_back(m_xCheckLB->get_checkbox_column_width());
    m_xCheckLB->set_column_fixed_widths(aWidths);
    m_xCheckLB->set_size_request(-1, m_xCheckLB->get_height_rows(10));
}

OfaAutocorrOptionsPage::~OfaAutocorrOptionsPage()
{
    disposeOnce();
}

VclPtr<SfxTabPage> OfaAutocorrOptionsPage::Create(TabPageParent pParent,
                                                  const SfxItemSet* rSet)
{
    return VclPtr<OfaAutocorrOptionsPage>::Create(pParent, *rSet);
}

#define CBCOL_FIRST     0
#define CBCOL_SECOND    1
#define CBCOL_BOTH      2

bool OfaAutocorrOptionsPage::FillItemSet( SfxItemSet* )
{
    SvxAutoCorrect* pAutoCorrect = SvxAutoCorrCfg::Get().GetAutoCorrect();
    ACFlags nFlags = pAutoCorrect->GetFlags();

    int nPos = 0;
    pAutoCorrect->SetAutoCorrFlag(ACFlags::Autocorrect,          m_xCheckLB->get_toggle(nPos++, CBCOL_FIRST));
    pAutoCorrect->SetAutoCorrFlag(ACFlags::CapitalStartWord,     m_xCheckLB->get_toggle(nPos++, CBCOL_FIRST));
    pAutoCorrect->SetAutoCorrFlag(ACFlags::CapitalStartSentence, m_xCheckLB->get_toggle(nPos++, CBCOL_FIRST));
    pAutoCorrect->SetAutoCorrFlag(ACFlags::ChgWeightUnderl,      m_xCheckLB->get_toggle(nPos++, CBCOL_FIRST));
    pAutoCorrect->SetAutoCorrFlag(ACFlags::SetINetAttr,          m_xCheckLB->get_toggle(nPos++, CBCOL_FIRST));
    pAutoCorrect->SetAutoCorrFlag(ACFlags::ChgToEnEmDash,        m_xCheckLB->get_toggle(nPos++, CBCOL_FIRST));
    pAutoCorrect->SetAutoCorrFlag(ACFlags::IgnoreDoubleSpace,    m_xCheckLB->get_toggle(nPos++, CBCOL_FIRST));
    pAutoCorrect->SetAutoCorrFlag(ACFlags::CorrectCapsLock,      m_xCheckLB->get_toggle(nPos++, CBCOL_FIRST));

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
    static_cast<OfaAutoCorrDlg*>(GetDialogController())->EnableLanguage(false);
}

void OfaAutocorrOptionsPage::InsertEntry(const OUString& rTxt)
{
    m_xCheckLB->append();
    const int nRow = m_xCheckLB->n_children() - 1;
    m_xCheckLB->set_toggle(nRow, false, CBCOL_FIRST);
    m_xCheckLB->set_text(nRow, rTxt, 1);
}

void OfaAutocorrOptionsPage::Reset( const SfxItemSet* )
{
    SvxAutoCorrect* pAutoCorrect = SvxAutoCorrCfg::Get().GetAutoCorrect();
    const ACFlags nFlags = pAutoCorrect->GetFlags();

    m_xCheckLB->freeze();
    m_xCheckLB->clear();

    InsertEntry(m_sInput);
    InsertEntry(m_sDoubleCaps);
    InsertEntry(m_sStartCap);
    InsertEntry(m_sBoldUnderline);
    InsertEntry(m_sURL);
    InsertEntry(m_sDash);
    InsertEntry(m_sNoDblSpaces);
    InsertEntry(m_sAccidentalCaps);

    int nPos = 0;
    m_xCheckLB->set_toggle( nPos++, bool(nFlags & ACFlags::Autocorrect), CBCOL_FIRST );
    m_xCheckLB->set_toggle( nPos++, bool(nFlags & ACFlags::CapitalStartWord), CBCOL_FIRST );
    m_xCheckLB->set_toggle( nPos++, bool(nFlags & ACFlags::CapitalStartSentence), CBCOL_FIRST );
    m_xCheckLB->set_toggle( nPos++, bool(nFlags & ACFlags::ChgWeightUnderl), CBCOL_FIRST );
    m_xCheckLB->set_toggle( nPos++, bool(nFlags & ACFlags::SetINetAttr), CBCOL_FIRST );
    m_xCheckLB->set_toggle( nPos++, bool(nFlags & ACFlags::ChgToEnEmDash), CBCOL_FIRST );
    m_xCheckLB->set_toggle( nPos++, bool(nFlags & ACFlags::IgnoreDoubleSpace), CBCOL_FIRST );
    m_xCheckLB->set_toggle( nPos++, bool(nFlags & ACFlags::CorrectCapsLock), CBCOL_FIRST );

    m_xCheckLB->thaw();
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
        , m_xPrcntMF(m_xBuilder->weld_metric_spin_button("margin", FieldUnit::PERCENT))
    {
    }

    weld::MetricSpinButton& GetPrcntFld()
    {
        return *m_xPrcntMF;
    }
};

/*********************************************************************/
/*                                                                   */
/*  use TabPage autoformat                                           */
/*                                                                   */
/*********************************************************************/

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

OfaSwAutoFmtOptionsPage::OfaSwAutoFmtOptionsPage(TabPageParent pParent,
                                                 const SfxItemSet& rSet )
    : SfxTabPage(pParent, "cui/ui/applyautofmtpage.ui", "ApplyAutoFmtPage", &rSet)
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
    , m_xCheckLB(m_xBuilder->weld_tree_view("list"))
    , m_xEditPB(m_xBuilder->weld_button("edit"))
{
    m_xCheckLB->connect_changed(LINK(this, OfaSwAutoFmtOptionsPage, SelectHdl));
    m_xCheckLB->connect_row_activated(LINK(this, OfaSwAutoFmtOptionsPage, DoubleClickEditHdl));

    std::vector<int> aWidths;
    aWidths.push_back(m_xCheckLB->get_pixel_size(m_xCheckLB->get_column_title(0)).Width() * 2);
    aWidths.push_back(m_xCheckLB->get_pixel_size(m_xCheckLB->get_column_title(1)).Width() * 2);
    m_xCheckLB->set_column_fixed_widths(aWidths);

    m_xEditPB->connect_clicked(LINK(this, OfaSwAutoFmtOptionsPage, EditHdl));
}

void OfaSwAutoFmtOptionsPage::CreateEntry(const OUString& rTxt, sal_uInt16 nCol)
{
    m_xCheckLB->append();
    const int nRow = m_xCheckLB->n_children() - 1;
    if (nCol == CBCOL_FIRST || nCol == CBCOL_BOTH)
        m_xCheckLB->set_toggle(nRow, false, CBCOL_FIRST);
    if (nCol == CBCOL_SECOND || nCol == CBCOL_BOTH)
        m_xCheckLB->set_toggle(nRow, false, CBCOL_SECOND);
    m_xCheckLB->set_text(nRow, rTxt, 2);
}

OfaSwAutoFmtOptionsPage::~OfaSwAutoFmtOptionsPage()
{
    disposeOnce();
}

VclPtr<SfxTabPage> OfaSwAutoFmtOptionsPage::Create(TabPageParent pParent,
                                                   const SfxItemSet* rAttrSet)
{
    return VclPtr<OfaSwAutoFmtOptionsPage>::Create(pParent, *rAttrSet);
}

bool OfaSwAutoFmtOptionsPage::FillItemSet( SfxItemSet*  )
{
    bool bModified = false;
    SvxAutoCorrect* pAutoCorrect = SvxAutoCorrCfg::Get().GetAutoCorrect();
    SvxSwAutoFormatFlags *pOpt = &pAutoCorrect->GetSwFlags();
    ACFlags nFlags = pAutoCorrect->GetFlags();

    bool bCheck = m_xCheckLB->get_toggle(USE_REPLACE_TABLE, CBCOL_FIRST);
    bModified |= pOpt->bAutoCorrect != bCheck;
    pOpt->bAutoCorrect = bCheck;
    pAutoCorrect->SetAutoCorrFlag(ACFlags::Autocorrect,
                        m_xCheckLB->get_toggle(USE_REPLACE_TABLE, CBCOL_SECOND));

    bCheck = m_xCheckLB->get_toggle(CORR_UPPER, CBCOL_FIRST);
    bModified |= pOpt->bCapitalStartWord != bCheck;
    pOpt->bCapitalStartWord = bCheck;
    pAutoCorrect->SetAutoCorrFlag(ACFlags::CapitalStartWord,
                        m_xCheckLB->get_toggle(CORR_UPPER, CBCOL_SECOND));

    bCheck = m_xCheckLB->get_toggle(BEGIN_UPPER, CBCOL_FIRST);
    bModified |= pOpt->bCapitalStartSentence != bCheck;
    pOpt->bCapitalStartSentence = bCheck;
    pAutoCorrect->SetAutoCorrFlag(ACFlags::CapitalStartSentence,
                        m_xCheckLB->get_toggle(BEGIN_UPPER, CBCOL_SECOND));

    bCheck = m_xCheckLB->get_toggle(BOLD_UNDERLINE, CBCOL_FIRST);
    bModified |= pOpt->bChgWeightUnderl != bCheck;
    pOpt->bChgWeightUnderl = bCheck;
    pAutoCorrect->SetAutoCorrFlag(ACFlags::ChgWeightUnderl,
                        m_xCheckLB->get_toggle(BOLD_UNDERLINE, CBCOL_SECOND));

    pAutoCorrect->SetAutoCorrFlag(ACFlags::IgnoreDoubleSpace,
                        m_xCheckLB->get_toggle(IGNORE_DBLSPACE, CBCOL_SECOND));

    pAutoCorrect->SetAutoCorrFlag(ACFlags::CorrectCapsLock,
                        m_xCheckLB->get_toggle(CORRECT_CAPS_LOCK, CBCOL_SECOND));

    bCheck = m_xCheckLB->get_toggle(DETECT_URL, CBCOL_FIRST);
    bModified |= pOpt->bSetINetAttr != bCheck;
    pOpt->bSetINetAttr = bCheck;
    pAutoCorrect->SetAutoCorrFlag(ACFlags::SetINetAttr,
                        m_xCheckLB->get_toggle(DETECT_URL, CBCOL_SECOND));

    bCheck = m_xCheckLB->get_toggle(DEL_EMPTY_NODE, CBCOL_FIRST);
    bModified |= pOpt->bDelEmptyNode != bCheck;
    pOpt->bDelEmptyNode = bCheck;

    bCheck = m_xCheckLB->get_toggle(REPLACE_USER_COLL, CBCOL_FIRST);
    bModified |= pOpt->bChgUserColl != bCheck;
    pOpt->bChgUserColl = bCheck;

    bCheck = m_xCheckLB->get_toggle(REPLACE_BULLETS, CBCOL_FIRST);
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

    bCheck = m_xCheckLB->get_toggle(MERGE_SINGLE_LINE_PARA, CBCOL_FIRST);
    bModified |= pOpt->bRightMargin != bCheck;
    pOpt->bRightMargin = bCheck;
    bModified |= nPercent != pOpt->nRightMargin;
    pOpt->nRightMargin = static_cast<sal_uInt8>(nPercent);

    bCheck = m_xCheckLB->get_toggle(APPLY_NUMBERING, CBCOL_SECOND);
    bModified |= pOpt->bSetNumRule != bCheck;
    pOpt->bSetNumRule = bCheck;

    bCheck = m_xCheckLB->get_toggle(INSERT_BORDER, CBCOL_SECOND);
    bModified |= pOpt->bSetBorder != bCheck;
    pOpt->bSetBorder = bCheck;

    bCheck = m_xCheckLB->get_toggle(CREATE_TABLE, CBCOL_SECOND);
    bModified |= pOpt->bCreateTable != bCheck;
    pOpt->bCreateTable = bCheck;

    bCheck = m_xCheckLB->get_toggle(REPLACE_STYLES, CBCOL_SECOND);
    bModified |= pOpt->bReplaceStyles != bCheck;
    pOpt->bReplaceStyles = bCheck;

    bCheck = m_xCheckLB->get_toggle(REPLACE_DASHES, CBCOL_FIRST);
    bModified |= pOpt->bChgToEnEmDash != bCheck;
    pOpt->bChgToEnEmDash = bCheck;
    pAutoCorrect->SetAutoCorrFlag(ACFlags::ChgToEnEmDash,
                        m_xCheckLB->get_toggle(REPLACE_DASHES, CBCOL_SECOND));

    bCheck = m_xCheckLB->get_toggle(DEL_SPACES_AT_STT_END, CBCOL_FIRST);
    bModified |= pOpt->bAFormatDelSpacesAtSttEnd != bCheck;
    pOpt->bAFormatDelSpacesAtSttEnd = bCheck;
    bCheck = m_xCheckLB->get_toggle(DEL_SPACES_AT_STT_END, CBCOL_SECOND);
    bModified |= pOpt->bAFormatByInpDelSpacesAtSttEnd != bCheck;
    pOpt->bAFormatByInpDelSpacesAtSttEnd = bCheck;

    bCheck = m_xCheckLB->get_toggle(DEL_SPACES_BETWEEN_LINES, CBCOL_FIRST);
    bModified |= pOpt->bAFormatDelSpacesBetweenLines != bCheck;
    pOpt->bAFormatDelSpacesBetweenLines = bCheck;
    bCheck = m_xCheckLB->get_toggle(DEL_SPACES_BETWEEN_LINES, CBCOL_SECOND);
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
    static_cast<OfaAutoCorrDlg*>(GetDialogController())->EnableLanguage(false);
}

void OfaSwAutoFmtOptionsPage::Reset( const SfxItemSet* )
{
    SvxAutoCorrect* pAutoCorrect = SvxAutoCorrCfg::Get().GetAutoCorrect();
    SvxSwAutoFormatFlags *pOpt = &pAutoCorrect->GetSwFlags();
    const ACFlags nFlags = pAutoCorrect->GetFlags();

    aBulletFont = pOpt->aBulletFont;
    sBulletChar = OUString(pOpt->cBullet);

    aByInputBulletFont = pOpt->aByInputBulletFont;
    sByInputBulletChar = OUString( pOpt->cByInputBullet );

    nPercent = pOpt->nRightMargin;
    sMargin = unicode::formatPercent(nPercent, Application::GetSettings().GetUILanguageTag());

    m_xCheckLB->freeze();
    m_xCheckLB->clear();

    // The following entries have to be inserted in the same order
    // as in the OfaAutoFmtOptions-enum!
    CreateEntry(sUseReplaceTbl,     CBCOL_BOTH  );
    CreateEntry(sCapitalStartWord,  CBCOL_BOTH  );
    CreateEntry(sCapitalStartSentence, CBCOL_BOTH  );
    CreateEntry(sBoldUnder,         CBCOL_BOTH  );
    CreateEntry(sDetectURL,         CBCOL_BOTH  );
    CreateEntry(sDash,              CBCOL_BOTH  );
    CreateEntry(sDelSpaceAtSttEnd,  CBCOL_BOTH  );
    CreateEntry(sDelSpaceBetweenLines, CBCOL_BOTH  );

    CreateEntry(sNoDblSpaces,       CBCOL_SECOND);
    CreateEntry(sCorrectCapsLock,   CBCOL_SECOND);
    CreateEntry(sNum.replaceFirst("%1", sBulletChar), CBCOL_SECOND);
    CreateEntry(sBorder,            CBCOL_SECOND);
    CreateEntry(sTable,             CBCOL_SECOND);
    CreateEntry(sReplaceTemplates,  CBCOL_SECOND);
    CreateEntry(sDeleteEmptyPara,   CBCOL_FIRST );
    CreateEntry(sUserStyle,         CBCOL_FIRST );
    CreateEntry(sBullet.replaceFirst("%1", sByInputBulletChar), CBCOL_FIRST);
    CreateEntry(sRightMargin.replaceFirst("%1", sMargin), CBCOL_FIRST);

    m_xCheckLB->set_toggle(USE_REPLACE_TABLE, pOpt->bAutoCorrect, CBCOL_FIRST);
    m_xCheckLB->set_toggle(USE_REPLACE_TABLE, bool(nFlags & ACFlags::Autocorrect), CBCOL_SECOND);
    m_xCheckLB->set_toggle(CORR_UPPER, pOpt->bCapitalStartWord, CBCOL_FIRST);
    m_xCheckLB->set_toggle(CORR_UPPER, bool(nFlags & ACFlags::CapitalStartWord), CBCOL_SECOND);
    m_xCheckLB->set_toggle(BEGIN_UPPER, pOpt->bCapitalStartSentence, CBCOL_FIRST);
    m_xCheckLB->set_toggle(BEGIN_UPPER, bool(nFlags & ACFlags::CapitalStartSentence), CBCOL_SECOND);
    m_xCheckLB->set_toggle(BOLD_UNDERLINE, pOpt->bChgWeightUnderl, CBCOL_FIRST);
    m_xCheckLB->set_toggle(BOLD_UNDERLINE, bool(nFlags & ACFlags::ChgWeightUnderl), CBCOL_SECOND);
    m_xCheckLB->set_toggle(DETECT_URL, pOpt->bSetINetAttr, CBCOL_FIRST);
    m_xCheckLB->set_toggle(DETECT_URL, bool(nFlags & ACFlags::SetINetAttr), CBCOL_SECOND);
    m_xCheckLB->set_toggle(REPLACE_DASHES, pOpt->bChgToEnEmDash, CBCOL_FIRST);
    m_xCheckLB->set_toggle(REPLACE_DASHES, bool(nFlags & ACFlags::ChgToEnEmDash), CBCOL_SECOND);
    m_xCheckLB->set_toggle(DEL_SPACES_AT_STT_END, pOpt->bAFormatDelSpacesAtSttEnd, CBCOL_FIRST);
    m_xCheckLB->set_toggle(DEL_SPACES_AT_STT_END, pOpt->bAFormatByInpDelSpacesAtSttEnd, CBCOL_SECOND);
    m_xCheckLB->set_toggle(DEL_SPACES_BETWEEN_LINES, pOpt->bAFormatDelSpacesBetweenLines, CBCOL_FIRST);
    m_xCheckLB->set_toggle(DEL_SPACES_BETWEEN_LINES, pOpt->bAFormatByInpDelSpacesBetweenLines, CBCOL_SECOND);
    m_xCheckLB->set_toggle(IGNORE_DBLSPACE, bool(nFlags & ACFlags::IgnoreDoubleSpace), CBCOL_SECOND);
    m_xCheckLB->set_toggle(CORRECT_CAPS_LOCK, bool(nFlags & ACFlags::CorrectCapsLock), CBCOL_SECOND);
    m_xCheckLB->set_toggle(APPLY_NUMBERING, pOpt->bSetNumRule, CBCOL_SECOND);
    m_xCheckLB->set_toggle(INSERT_BORDER, pOpt->bSetBorder, CBCOL_SECOND);
    m_xCheckLB->set_toggle(CREATE_TABLE, pOpt->bCreateTable, CBCOL_SECOND);
    m_xCheckLB->set_toggle(REPLACE_STYLES, pOpt->bReplaceStyles, CBCOL_SECOND);
    m_xCheckLB->set_toggle(DEL_EMPTY_NODE, pOpt->bDelEmptyNode, CBCOL_FIRST);
    m_xCheckLB->set_toggle(REPLACE_USER_COLL, pOpt->bChgUserColl, CBCOL_FIRST);
    m_xCheckLB->set_toggle(REPLACE_BULLETS, pOpt->bChgEnumNum, CBCOL_FIRST);
    m_xCheckLB->set_toggle(MERGE_SINGLE_LINE_PARA, pOpt->bRightMargin, CBCOL_FIRST);

    ImpUserData* pUserData = new ImpUserData(&sBulletChar, &aBulletFont);
    OUString sId(OUString::number(reinterpret_cast<sal_Int64>(pUserData)));
    m_xCheckLB->set_id(REPLACE_BULLETS, sId);

    pUserData = new ImpUserData(&sMargin, nullptr);
    sId = OUString::number(reinterpret_cast<sal_Int64>(pUserData));
    m_xCheckLB->set_id(MERGE_SINGLE_LINE_PARA, sId);

    ImpUserData* pUserData2 = new ImpUserData(&sByInputBulletChar, &aByInputBulletFont);
    sId = OUString::number(reinterpret_cast<sal_Int64>(pUserData2));
    m_xCheckLB->set_id(APPLY_NUMBERING, sId);

    m_xCheckLB->thaw();
}

IMPL_LINK(OfaSwAutoFmtOptionsPage, SelectHdl, weld::TreeView&, rBox, void)
{
    m_xEditPB->set_sensitive(rBox.get_selected_id().toInt64() != 0);
}

IMPL_LINK_NOARG(OfaSwAutoFmtOptionsPage, DoubleClickEditHdl, weld::TreeView&, void)
{
    EditHdl(*m_xEditPB);
}

IMPL_LINK_NOARG(OfaSwAutoFmtOptionsPage, EditHdl, weld::Button&, void)
{
    int nSelEntryPos = m_xCheckLB->get_selected_index();
    if (nSelEntryPos == REPLACE_BULLETS || nSelEntryPos == APPLY_NUMBERING)
    {
        SvxCharacterMap aMapDlg(GetDialogFrameWeld(), nullptr, false);
        ImpUserData* pUserData = reinterpret_cast<ImpUserData*>(m_xCheckLB->get_id(nSelEntryPos).toInt64());
        aMapDlg.SetCharFont(*pUserData->pFont);
        aMapDlg.SetChar( (*pUserData->pString)[0] );
        if (RET_OK == aMapDlg.run())
        {
            const vcl::Font& aFont(aMapDlg.GetCharFont());
            *pUserData->pFont = aFont;
            sal_UCS4 aChar = aMapDlg.GetChar();
            // using the UCS4 constructor
            OUString aOUStr( &aChar, 1 );
            *pUserData->pString = aOUStr;
            if (nSelEntryPos == REPLACE_BULLETS)
                m_xCheckLB->set_text(nSelEntryPos, sNum.replaceFirst("%1", aOUStr), 2);
            else
                m_xCheckLB->set_text(nSelEntryPos, sBullet.replaceFirst("%1", aOUStr), 2);
        }
    }
    else if( MERGE_SINGLE_LINE_PARA == nSelEntryPos )
    {
        // dialog for per cent settings
        OfaAutoFmtPrcntSet aDlg(GetDialogFrameWeld());
        aDlg.GetPrcntFld().set_value(nPercent, FieldUnit::PERCENT);
        if (aDlg.run() == RET_OK)
        {
            nPercent = static_cast<sal_uInt16>(aDlg.GetPrcntFld().get_value(FieldUnit::PERCENT));
            sMargin = unicode::formatPercent(nPercent, Application::GetSettings().GetUILanguageTag());
            m_xCheckLB->set_text(nSelEntryPos, sRightMargin.replaceFirst("%1", sMargin), 2);
        }
    }
}


OfaAutocorrReplacePage::OfaAutocorrReplacePage(TabPageParent pParent,
                                               const SfxItemSet& rSet)
    : SfxTabPage(pParent, "cui/ui/acorreplacepage.ui", "AcorReplacePage", &rSet)
    , eLang(eLastDialogLanguage)
    , bHasSelectionText(false)
    , bFirstSelect(true)
    , bReplaceEditChanged(false)
    , bSWriter(true)
    , m_xTextOnlyCB(m_xBuilder->weld_check_button("textonly"))
    , m_xShortED(m_xBuilder->weld_entry("origtext"))
    , m_xReplaceED(m_xBuilder->weld_entry("newtext"))
    , m_xReplaceTLB(m_xBuilder->weld_tree_view("tabview"))
    , m_xNewReplacePB(m_xBuilder->weld_button("new"))
    , m_xReplacePB(m_xBuilder->weld_button("replace"))
    , m_xDeleteReplacePB(m_xBuilder->weld_button("delete"))
{
    sNew = m_xNewReplacePB->get_label();
    sModify = m_xReplacePB->get_label();
    m_xReplaceTLB->set_size_request(-1, m_xReplaceTLB->get_height_rows(10));

    SfxModule *pMod = SfxApplication::GetModule(SfxToolsModule::Writer);
    bSWriter = pMod == SfxModule::GetActiveModule();

    LanguageTag aLanguageTag( eLastDialogLanguage );
    pCompareClass.reset( new CollatorWrapper( comphelper::getProcessComponentContext() ) );
    pCompareClass->loadDefaultCollator( aLanguageTag.getLocale(), 0 );
    pCharClass.reset( new CharClass( aLanguageTag ) );

    std::vector<int> aWidths;
    aWidths.push_back(m_xReplaceTLB->get_approximate_digit_width() * 32);
    m_xReplaceTLB->set_column_fixed_widths(aWidths);

    m_xReplaceTLB->connect_changed( LINK(this, OfaAutocorrReplacePage, SelectHdl) );
    m_xNewReplacePB->connect_clicked( LINK(this, OfaAutocorrReplacePage, NewDelButtonHdl) );
    m_xDeleteReplacePB->connect_clicked( LINK(this, OfaAutocorrReplacePage, NewDelButtonHdl) );
    m_xShortED->connect_changed( LINK(this, OfaAutocorrReplacePage, ModifyHdl) );
    m_xReplaceED->connect_changed( LINK(this, OfaAutocorrReplacePage, ModifyHdl) );
    m_xShortED->connect_activate( LINK(this, OfaAutocorrReplacePage, NewDelActionHdl) );
    m_xReplaceED->connect_activate( LINK(this, OfaAutocorrReplacePage, NewDelActionHdl) );
    m_xShortED->connect_size_allocate(LINK(this, OfaAutocorrReplacePage, EntrySizeAllocHdl));
    m_xReplaceED->connect_size_allocate(LINK(this, OfaAutocorrReplacePage, EntrySizeAllocHdl));
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

    SfxTabPage::dispose();
}

VclPtr<SfxTabPage> OfaAutocorrReplacePage::Create(TabPageParent pParent, const SfxItemSet* rSet)
{
    return VclPtr<OfaAutocorrReplacePage>::Create(pParent, *rSet);
}

void OfaAutocorrReplacePage::ActivatePage( const SfxItemSet& )
{
    if(eLang != eLastDialogLanguage)
        SetLanguage(eLastDialogLanguage);
    static_cast<OfaAutoCorrDlg*>(GetDialogController())->EnableLanguage(true);
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

        sal_uInt32 nListBoxCount = m_xReplaceTLB->n_children();
        sal_uInt32 i;
        for(i = 0; i < nListBoxCount; i++)
        {
            pArray->push_back(DoubleString());
            DoubleString& rDouble = (*pArray)[pArray->size() - 1];
            rDouble.sShort = m_xReplaceTLB->get_text(i, 0);
            rDouble.sLong = m_xReplaceTLB->get_text(i, 1);
            rDouble.pUserData = reinterpret_cast<void*>(m_xReplaceTLB->get_id(i).toInt64());
        }
    }

    m_xReplaceTLB->clear();
    if( !bSWriter )
        aFormatText.clear();

    m_xReplaceTLB->freeze();

    if( aDoubleStringTable.find(eLang) != aDoubleStringTable.end() )
    {
        DoubleStringArray& rArray = aDoubleStringTable[eNewLanguage];
        for(DoubleString & rDouble : rArray)
        {
            bool bTextOnly = nullptr == rDouble.pUserData;
            // formatted text is only in Writer
            if (bSWriter || bTextOnly)
            {
                m_xTextOnlyCB->set_active(bTextOnly);
                OUString sId;
                if (!bTextOnly)
                {
                    // that means: with format info or even with selection text
                    sId = OUString::number(reinterpret_cast<sal_Int64>(rDouble.pUserData));
                }
                m_xReplaceTLB->append(sId, rDouble.sShort);
                m_xReplaceTLB->set_text(m_xReplaceTLB->n_children() - 1, rDouble.sLong, 1);
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
        SvxAutocorrWordList::Content aContent = pWordList->getSortedContent();
        for (auto const& elem : aContent)
        {
            bool bTextOnly = elem->IsTextOnly();
            // formatted text is only in Writer
            if (bSWriter || bTextOnly)
            {
                m_xTextOnlyCB->set_active(elem->IsTextOnly());
                OUString sId;
                if (!bTextOnly)
                {
                    // that means: with format info or even with selection text
                    sId = OUString::number(reinterpret_cast<sal_Int64>(m_xTextOnlyCB.get()));
                }
                m_xReplaceTLB->append(sId, elem->GetShort());
                m_xReplaceTLB->set_text(m_xReplaceTLB->n_children() - 1, elem->GetLong(), 1);
            }
            else
            {
                aFormatText.insert(elem->GetShort());
            }
        }
        m_xNewReplacePB->set_sensitive(false);
        m_xDeleteReplacePB->set_sensitive(false);
    }

    m_xReplaceTLB->thaw();

    SfxViewShell* pViewShell = SfxViewShell::Current();
    if (pViewShell && pViewShell->HasSelection())
    {
        bHasSelectionText = true;
        const OUString sSelection( pViewShell->GetSelectionText() );
        m_xReplaceED->set_text(sSelection);
        m_xTextOnlyCB->set_active(!bSWriter);
        m_xTextOnlyCB->set_sensitive(bSWriter && !sSelection.isEmpty());
    }
    else
    {
        m_xTextOnlyCB->set_sensitive(false);
    }
}

void OfaAutocorrReplacePage::Reset( const SfxItemSet* )
{
    RefillReplaceBox(true, eLang, eLang);
    m_xShortED->grab_focus();
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
        ModifyHdl(*m_xShortED);
    }
}

IMPL_LINK(OfaAutocorrReplacePage, SelectHdl, weld::TreeView&, rBox, void)
{
    if(!bFirstSelect || !bHasSelectionText)
    {
        int nEntry = rBox.get_selected_index();
        OUString sTmpShort(rBox.get_text(nEntry, 0));
        // if the text is set via ModifyHdl, the cursor is always at the beginning
        // of a word, although you're editing here
        bool bSameContent = 0 == pCompareClass->compareString(sTmpShort, m_xShortED->get_text());
        int nStartPos, nEndPos;
        m_xShortED->get_selection_bounds(nStartPos, nEndPos);
        if (m_xShortED->get_text() != sTmpShort)
        {
            m_xShortED->set_text(sTmpShort);
            // if it was only a different notation, the selection has to be set again
            if (bSameContent)
            {
                m_xShortED->select_region(nStartPos, nEndPos);
            }
        }
        m_xReplaceED->set_text(rBox.get_text(nEntry, 1));
        // with UserData there is a Formatinfo
        m_xTextOnlyCB->set_active(rBox.get_id(nEntry).isEmpty());
    }
    else
    {
        bFirstSelect = false;
    }

    m_xNewReplacePB->set_sensitive(false);
    m_xDeleteReplacePB->set_sensitive(true);
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

IMPL_LINK(OfaAutocorrReplacePage, NewDelButtonHdl, weld::Button&, rBtn, void)
{
    NewDelHdl(&rBtn);
}

IMPL_LINK(OfaAutocorrReplacePage, NewDelActionHdl, weld::Entry&, rEdit, bool)
{
    return NewDelHdl(&rEdit);
}

IMPL_LINK_NOARG(OfaAutocorrReplacePage, EntrySizeAllocHdl, const Size&, void)
{
    std::vector<int> aWidths;
    int x, y, width, height;
    if (m_xReplaceED->get_extents_relative_to(*m_xReplaceTLB, x, y, width, height))
    {
        aWidths.push_back(x);
        m_xReplaceTLB->set_column_fixed_widths(aWidths);
    }
}

bool OfaAutocorrReplacePage::NewDelHdl(const weld::Widget* pBtn)
{
    int nEntry = m_xReplaceTLB->get_selected_index();
    if (pBtn == m_xDeleteReplacePB.get())
    {
        DBG_ASSERT( nEntry != -1, "no entry selected" );
        if (nEntry != -1)
        {
            DeleteEntry(m_xReplaceTLB->get_text(nEntry, 0), m_xReplaceTLB->get_text(nEntry, 1));
            m_xReplaceTLB->remove(nEntry);
            ModifyHdl(*m_xShortED);
            return false;
        }
    }

    if (pBtn == m_xNewReplacePB.get() || m_xNewReplacePB->get_sensitive())
    {
        OUString sEntry(m_xShortED->get_text());
        if (!sEntry.isEmpty() && (!m_xReplaceED->get_text().isEmpty() ||
                ( bHasSelectionText && bSWriter ) ))
        {
            bool bKeepSourceFormatting = !bReplaceEditChanged && !m_xTextOnlyCB->get_active();

            NewEntry(m_xShortED->get_text(), m_xReplaceED->get_text(), bKeepSourceFormatting);
            m_xReplaceTLB->freeze();
            int nPos = -1;
            if (nEntry != -1)
            {
                nPos = nEntry;
                m_xReplaceTLB->remove(nEntry);
            }
            else
            {
                int j;
                int nCount = m_xReplaceTLB->n_children();
                for (j = 0; j < nCount; ++j)
                {
                    if (0 >= pCompareClass->compareString(sEntry, m_xReplaceTLB->get_text(j, 0)))
                        break;
                }
                nPos = j;
            }

            OUString sId;
            if (bKeepSourceFormatting)
            {
                sId = OUString::number(reinterpret_cast<sal_Int64>(&bHasSelectionText)); // new formatted text
            }

            m_xReplaceTLB->insert(nPos, sEntry, &sId, nullptr, nullptr);
            m_xReplaceTLB->set_text(nPos, m_xReplaceED->get_text(), 1);
            m_xReplaceTLB->thaw();
            m_xReplaceTLB->scroll_to_row(nPos);
            // if the request came from the ReplaceEdit, give focus to the ShortEdit
            if (m_xReplaceED->has_focus())
            {
                m_xShortED->grab_focus();
            }
        }
    }
    else
    {
        // this can only be an enter in one of the two edit fields
        // which means EndDialog() - has to be evaluated in KeyInput
        return false;
    }
    ModifyHdl(*m_xShortED);
    return true;
}

IMPL_LINK(OfaAutocorrReplacePage, ModifyHdl, weld::Entry&, rEdt, void)
{
    int nFirstSel = m_xReplaceTLB->get_selected_index();
    bool bShort = &rEdt == m_xShortED.get();
    const OUString rEntry = rEdt.get_text();
    const OUString rRepString = m_xReplaceED->get_text();
    OUString aWordStr(pCharClass->lowercase(rEntry));

    if(bShort)
    {
        if(!rEntry.isEmpty())
        {
            bool bFound = false;
            bool bTmpSelEntry=false;

            int nCount = m_xReplaceTLB->n_children();
            for (int i = 0; i < nCount; ++i)
            {
                int nEntry = i;
                OUString aTestStr = m_xReplaceTLB->get_text(i, 0);
                if( pCompareClass->compareString(rEntry, aTestStr ) == 0 )
                {
                    if( !rRepString.isEmpty() )
                    {
                        bFirstSelect = true;
                    }
                    m_xReplaceTLB->set_cursor(nEntry);
                    nFirstSel = i;
                    m_xNewReplacePB->set_label(sModify);
                    bFound = true;
                    break;
                }
                else
                {
                    aTestStr = pCharClass->lowercase( aTestStr );
                    if( aTestStr.startsWith(aWordStr) && !bTmpSelEntry )
                    {
                        m_xReplaceTLB->scroll_to_row(nEntry);
                        bTmpSelEntry = true;
                    }
                }
            }
            if( !bFound )
            {
                m_xReplaceTLB->select(-1);
                nFirstSel = -1;
                m_xNewReplacePB->set_label(sNew);
                if( bReplaceEditChanged )
                    m_xTextOnlyCB->set_sensitive(false);
            }
            m_xDeleteReplacePB->set_sensitive(bFound);
        }
        else if (m_xReplaceTLB->n_children() > 0)
        {
            m_xReplaceTLB->scroll_to_row(0);
        }

    }
    else if( !bShort )
    {
        bReplaceEditChanged = true;
        if (nFirstSel != -1)
        {
            m_xNewReplacePB->set_label(sModify);
        }
    }

    const OUString& rShortTxt = m_xShortED->get_text();
    bool bEnableNew = !rShortTxt.isEmpty() &&
                        ( !rRepString.isEmpty() ||
                                ( bHasSelectionText && bSWriter )) &&
                        ( nFirstSel == -1 || rRepString !=
                                m_xReplaceTLB->get_text(nFirstSel, 1) );
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
    m_xNewReplacePB->set_sensitive(bEnableNew);
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

OfaAutocorrExceptPage::OfaAutocorrExceptPage(TabPageParent pParent, const SfxItemSet& rSet)
    : SfxTabPage(pParent, "cui/ui/acorexceptpage.ui", "AcorExceptPage", &rSet)
    , eLang(eLastDialogLanguage)
    , m_xAbbrevED(m_xBuilder->weld_entry("abbrev"))
    , m_xAbbrevLB(m_xBuilder->weld_tree_view("abbrevlist"))
    , m_xNewAbbrevPB(m_xBuilder->weld_button("newabbrev"))
    , m_xDelAbbrevPB(m_xBuilder->weld_button("delabbrev"))
    , m_xAutoAbbrevCB(m_xBuilder->weld_check_button("autoabbrev"))
    , m_xDoubleCapsED(m_xBuilder->weld_entry("double"))
    , m_xDoubleCapsLB(m_xBuilder->weld_tree_view("doublelist"))
    , m_xNewDoublePB(m_xBuilder->weld_button("newdouble"))
    , m_xDelDoublePB(m_xBuilder->weld_button("deldouble"))
    , m_xAutoCapsCB(m_xBuilder->weld_check_button("autodouble"))
{
    m_xAbbrevLB->make_sorted();
    m_xAbbrevLB->set_size_request(-1, m_xAbbrevLB->get_height_rows(6));

    m_xDoubleCapsLB->make_sorted();
    m_xDoubleCapsLB->set_size_request(-1, m_xDoubleCapsLB->get_height_rows(6));

    css::lang::Locale aLcl( LanguageTag::convertToLocale(eLastDialogLanguage ));
    pCompareClass.reset( new CollatorWrapper( comphelper::getProcessComponentContext() ) );
    pCompareClass->loadDefaultCollator( aLcl, 0 );

    m_xNewAbbrevPB->connect_clicked(LINK(this, OfaAutocorrExceptPage, NewDelButtonHdl));
    m_xDelAbbrevPB->connect_clicked(LINK(this, OfaAutocorrExceptPage, NewDelButtonHdl));
    m_xNewDoublePB->connect_clicked(LINK(this, OfaAutocorrExceptPage, NewDelButtonHdl));
    m_xDelDoublePB->connect_clicked(LINK(this, OfaAutocorrExceptPage, NewDelButtonHdl));

    m_xAbbrevLB->connect_changed(LINK(this, OfaAutocorrExceptPage, SelectHdl));
    m_xDoubleCapsLB->connect_changed(LINK(this, OfaAutocorrExceptPage, SelectHdl));
    m_xAbbrevED->connect_changed(LINK(this, OfaAutocorrExceptPage, ModifyHdl));
    m_xDoubleCapsED->connect_changed(LINK(this, OfaAutocorrExceptPage, ModifyHdl));

    m_xAbbrevED->connect_activate(LINK(this, OfaAutocorrExceptPage, NewDelActionHdl));
    m_xDoubleCapsED->connect_activate(LINK(this, OfaAutocorrExceptPage, NewDelActionHdl));
}

OfaAutocorrExceptPage::~OfaAutocorrExceptPage()
{
    disposeOnce();
}

void OfaAutocorrExceptPage::dispose()
{
    aStringsTable.clear();
    pCompareClass.reset();
    SfxTabPage::dispose();
}

VclPtr<SfxTabPage> OfaAutocorrExceptPage::Create(TabPageParent pParent,
                                                 const SfxItemSet* rSet)
{
    return VclPtr<OfaAutocorrExceptPage>::Create(pParent, *rSet);
}

void    OfaAutocorrExceptPage::ActivatePage( const SfxItemSet& )
{
    if(eLang != eLastDialogLanguage)
        SetLanguage(eLastDialogLanguage);
    static_cast<OfaAutoCorrDlg*>(GetDialogController())->EnableLanguage(true);
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
            if (m_xDoubleCapsLB->find_text(aString) == -1)
            {
                pWrdList->erase(i);
            }
        }
        nCount = m_xDoubleCapsLB->n_children();
        for( i = 0; i < nCount; ++i )
        {
            pWrdList->insert(m_xDoubleCapsLB->get_text(i));
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
            if (m_xAbbrevLB->find_text(aString) == -1)
            {
                pCplList->erase(i);
            }
        }
        sal_Int32 nAbbrevCount = m_xAbbrevLB->n_children();
        for( sal_Int32 ia = 0; ia < nAbbrevCount; ++ia )
        {
            pCplList->insert(m_xAbbrevLB->get_text(ia));
        }
        pAutoCorrect->SaveCplSttExceptList(eLang);
    }
    if (m_xAutoAbbrevCB->get_state_changed_from_saved())
        pAutoCorrect->SetAutoCorrFlag( ACFlags::SaveWordCplSttLst, m_xAutoAbbrevCB->get_active());
    if (m_xAutoCapsCB->get_state_changed_from_saved())
        pAutoCorrect->SetAutoCorrFlag( ACFlags::SaveWordWrdSttLst, m_xAutoCapsCB->get_active());
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
        ModifyHdl(*m_xAbbrevED);
        ModifyHdl(*m_xDoubleCapsED);
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

        sal_Int32 i, nCount;
        nCount = m_xAbbrevLB->n_children();
        for(i = 0; i < nCount; i++)
            pArrays->aAbbrevStrings.push_back(m_xAbbrevLB->get_text(i));

        nCount = m_xDoubleCapsLB->n_children();
        for(i = 0; i < nCount; i++)
            pArrays->aDoubleCapsStrings.push_back(m_xDoubleCapsLB->get_text(i));
    }
    m_xDoubleCapsLB->clear();
    m_xAbbrevLB->clear();
    OUString sTemp;
    m_xAbbrevED->set_text(sTemp);
    m_xDoubleCapsED->set_text(sTemp);

    if(aStringsTable.find(eLang) != aStringsTable.end())
    {
        StringsArrays& rArrays = aStringsTable[eLang];
        for (auto const& elem : rArrays.aAbbrevStrings)
            m_xAbbrevLB->append_text(elem);

        for (auto const& elem : rArrays.aDoubleCapsStrings)
            m_xDoubleCapsLB->append_text(elem);
    }
    else
    {
        SvxAutoCorrect* pAutoCorrect = SvxAutoCorrCfg::Get().GetAutoCorrect();
        const SvStringsISortDtor* pCplList = pAutoCorrect->GetCplSttExceptList(eLang);
        const SvStringsISortDtor* pWrdList = pAutoCorrect->GetWrdSttExceptList(eLang);
        size_t i;
        for( i = 0; i < pCplList->size(); i++ )
        {
            m_xAbbrevLB->append_text((*pCplList)[i]);
        }
        for( i = 0; i < pWrdList->size(); i++ )
        {
            m_xDoubleCapsLB->append_text((*pWrdList)[i]);
        }
    }
}

void OfaAutocorrExceptPage::Reset( const SfxItemSet* )
{
    SvxAutoCorrect* pAutoCorrect = SvxAutoCorrCfg::Get().GetAutoCorrect();
    RefillReplaceBoxes(true, eLang, eLang);
    m_xAutoAbbrevCB->set_active(pAutoCorrect->IsAutoCorrFlag( ACFlags::SaveWordCplSttLst));
    m_xAutoCapsCB->set_active(pAutoCorrect->IsAutoCorrFlag( ACFlags::SaveWordWrdSttLst));
    m_xAutoAbbrevCB->save_state();
    m_xAutoCapsCB->save_state();
}

IMPL_LINK(OfaAutocorrExceptPage, NewDelButtonHdl, weld::Button&, rBtn, void)
{
    NewDelHdl(&rBtn);
}

IMPL_LINK(OfaAutocorrExceptPage, NewDelActionHdl, weld::Entry&, rEdit, bool)
{
    NewDelHdl(&rEdit);
    return false;
}

void OfaAutocorrExceptPage::NewDelHdl(const weld::Widget* pBtn)
{
    if ((pBtn == m_xNewAbbrevPB.get() || pBtn == m_xAbbrevED.get())
        && !m_xAbbrevED->get_text().isEmpty())
    {
        m_xAbbrevLB->append_text(m_xAbbrevED->get_text());
        ModifyHdl(*m_xAbbrevED);
    }
    else if(pBtn == m_xDelAbbrevPB.get())
    {
        m_xAbbrevLB->remove_text(m_xAbbrevED->get_text());
        ModifyHdl(*m_xAbbrevED);
    }
    else if((pBtn == m_xNewDoublePB.get() || pBtn == m_xDoubleCapsED.get() )
            && !m_xDoubleCapsED->get_text().isEmpty())
    {
        m_xDoubleCapsLB->append_text(m_xDoubleCapsED->get_text());
        ModifyHdl(*m_xDoubleCapsED);
    }
    else if (pBtn == m_xDelDoublePB.get())
    {
        m_xDoubleCapsLB->remove_text(m_xDoubleCapsED->get_text());
        ModifyHdl(*m_xDoubleCapsED);
    }
}

IMPL_LINK(OfaAutocorrExceptPage, SelectHdl, weld::TreeView&, rBox, void)
{
    if (&rBox == m_xAbbrevLB.get())
    {
        m_xAbbrevED->set_text(rBox.get_selected_text());
        m_xNewAbbrevPB->set_sensitive(false);
        m_xDelAbbrevPB->set_sensitive(true);
    }
    else
    {
        m_xDoubleCapsED->set_text(rBox.get_selected_text());
        m_xNewDoublePB->set_sensitive(false);
        m_xDelDoublePB->set_sensitive(true);
    }
}

IMPL_LINK(OfaAutocorrExceptPage, ModifyHdl, weld::Entry&, rEdt, void)
{
    const OUString& sEntry = rEdt.get_text();
    bool bEntryLen = !sEntry.isEmpty();
    if (&rEdt == m_xAbbrevED.get())
    {
        bool bSame = lcl_FindEntry(*m_xAbbrevLB, sEntry, *pCompareClass);
        if(bSame && sEntry != m_xAbbrevLB->get_selected_text())
            rEdt.set_text(m_xAbbrevLB->get_selected_text());
        m_xNewAbbrevPB->set_sensitive(!bSame && bEntryLen);
        m_xDelAbbrevPB->set_sensitive(bSame && bEntryLen);
    }
    else
    {
        bool bSame = lcl_FindEntry(*m_xDoubleCapsLB, sEntry, *pCompareClass);
        if(bSame && sEntry != m_xDoubleCapsLB->get_selected_text())
            rEdt.set_text(m_xDoubleCapsLB->get_selected_text());
        m_xNewDoublePB->set_sensitive(!bSame && bEntryLen);
        m_xDelDoublePB->set_sensitive(bSame && bEntryLen);
    }
}

enum OfaQuoteOptions
{
    ADD_NONBRK_SPACE,
    REPLACE_1ST
};

void OfaQuoteTabPage::CreateEntry(weld::TreeView& rCheckLB, const OUString& rTxt, sal_uInt16 nCol, sal_uInt16 nTextCol)
{
    rCheckLB.append();
    const int nRow = rCheckLB.n_children() - 1;
    if (nCol == CBCOL_FIRST || nCol == CBCOL_BOTH)
        rCheckLB.set_toggle(nRow, false, CBCOL_FIRST);
    if (nCol == CBCOL_SECOND || nCol == CBCOL_BOTH)
        rCheckLB.set_toggle(nRow, false, CBCOL_SECOND);
    rCheckLB.set_text(nRow, rTxt, nTextCol);
}

OfaQuoteTabPage::OfaQuoteTabPage(TabPageParent pParent, const SfxItemSet& rSet)
    : SfxTabPage(pParent, "cui/ui/applylocalizedpage.ui", "ApplyLocalizedPage", &rSet)
    , sNonBrkSpace(CuiResId(RID_SVXSTR_NON_BREAK_SPACE))
    , sOrdinal(CuiResId(RID_SVXSTR_ORDINAL))
    , cSglStartQuote(0)
    , cSglEndQuote(0)
    , cStartQuote(0)
    , cEndQuote(0)
    , m_xSingleTypoCB(m_xBuilder->weld_check_button("singlereplace"))
    , m_xSglStartQuotePB(m_xBuilder->weld_button("startsingle"))
    , m_xSglStartExFT(m_xBuilder->weld_label("singlestartex"))
    , m_xSglEndQuotePB(m_xBuilder->weld_button("endsingle"))
    , m_xSglEndExFT(m_xBuilder->weld_label("singleendex"))
    , m_xSglStandardPB(m_xBuilder->weld_button("defaultsingle"))
    , m_xDoubleTypoCB(m_xBuilder->weld_check_button("doublereplace"))
    , m_xDblStartQuotePB(m_xBuilder->weld_button("startdouble"))
    , m_xDblStartExFT(m_xBuilder->weld_label("doublestartex"))
    , m_xDblEndQuotePB(m_xBuilder->weld_button("enddouble"))
    , m_xDblEndExFT(m_xBuilder->weld_label("doubleendex"))
    , m_xDblStandardPB(m_xBuilder->weld_button("defaultdouble"))
    , m_xStandard(m_xBuilder->weld_label("singlestartex"))
    , m_xCheckLB(m_xBuilder->weld_tree_view("checklist"))
    , m_xSwCheckLB(m_xBuilder->weld_tree_view("list"))
{
    m_xSwCheckLB->set_size_request(m_xSwCheckLB->get_approximate_digit_width() * 50,
                                   m_xSwCheckLB->get_height_rows(6));

    bool bShowSWOptions = false;

    const SfxBoolItem* pItem = rSet.GetItem<SfxBoolItem>(SID_AUTO_CORRECT_DLG, false);
    if ( pItem && pItem->GetValue() )
        bShowSWOptions = true;

    if ( bShowSWOptions )
    {
        std::vector<int> aWidths;
        aWidths.push_back(m_xSwCheckLB->get_pixel_size(m_xSwCheckLB->get_column_title(0)).Width() * 2);
        aWidths.push_back(m_xSwCheckLB->get_pixel_size(m_xSwCheckLB->get_column_title(1)).Width() * 2);
        m_xSwCheckLB->set_column_fixed_widths(aWidths);
        m_xCheckLB->hide();
    }
    else
    {
        std::vector<int> aWidths;
        aWidths.push_back(m_xSwCheckLB->get_checkbox_column_width());
        m_xCheckLB->set_column_fixed_widths(aWidths);
        m_xSwCheckLB->hide();
    }

    m_xDblStartQuotePB->connect_clicked(LINK(this, OfaQuoteTabPage, QuoteHdl));
    m_xDblEndQuotePB->connect_clicked(LINK(this, OfaQuoteTabPage, QuoteHdl));
    m_xSglStartQuotePB->connect_clicked(LINK(this, OfaQuoteTabPage, QuoteHdl));
    m_xSglEndQuotePB->connect_clicked(LINK(this, OfaQuoteTabPage, QuoteHdl));
    m_xDblStandardPB->connect_clicked(LINK(this, OfaQuoteTabPage, StdQuoteHdl));
    m_xSglStandardPB->connect_clicked(LINK(this, OfaQuoteTabPage, StdQuoteHdl));
}

OfaQuoteTabPage::~OfaQuoteTabPage()
{
    disposeOnce();
}

VclPtr<SfxTabPage> OfaQuoteTabPage::Create(TabPageParent pParent,
                                           const SfxItemSet* rAttrSet)
{
    return VclPtr<OfaQuoteTabPage>::Create(pParent, *rAttrSet);
}

bool OfaQuoteTabPage::FillItemSet( SfxItemSet*  )
{
    SvxAutoCorrect* pAutoCorrect = SvxAutoCorrCfg::Get().GetAutoCorrect();

    ACFlags nFlags = pAutoCorrect->GetFlags();

    if (m_xCheckLB->get_visible())
    {
        int nPos = 0;
        pAutoCorrect->SetAutoCorrFlag(ACFlags::AddNonBrkSpace, m_xCheckLB->get_toggle(nPos++, CBCOL_FIRST));
        pAutoCorrect->SetAutoCorrFlag(ACFlags::ChgOrdinalNumber, m_xCheckLB->get_toggle(nPos++, CBCOL_FIRST));
    }

    bool bModified = false;
    if (m_xSwCheckLB->get_visible())
    {
        SvxSwAutoFormatFlags *pOpt = &pAutoCorrect->GetSwFlags();

        bool bCheck = m_xSwCheckLB->get_toggle(ADD_NONBRK_SPACE, CBCOL_FIRST);
        bModified |= pOpt->bAddNonBrkSpace != bCheck;
        pOpt->bAddNonBrkSpace = bCheck;
        pAutoCorrect->SetAutoCorrFlag(ACFlags::AddNonBrkSpace,
                            m_xSwCheckLB->get_toggle(ADD_NONBRK_SPACE, CBCOL_SECOND));

        bCheck = m_xSwCheckLB->get_toggle(REPLACE_1ST, CBCOL_FIRST);
        bModified |= pOpt->bChgOrdinalNumber != bCheck;
        pOpt->bChgOrdinalNumber = bCheck;
        pAutoCorrect->SetAutoCorrFlag(ACFlags::ChgOrdinalNumber,
                        m_xSwCheckLB->get_toggle(REPLACE_1ST, CBCOL_SECOND));
    }

    pAutoCorrect->SetAutoCorrFlag(ACFlags::ChgQuotes, m_xDoubleTypoCB->get_active());
    pAutoCorrect->SetAutoCorrFlag(ACFlags::ChgSglQuotes, m_xSingleTypoCB->get_active());
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
    static_cast<OfaAutoCorrDlg*>(GetDialogController())->EnableLanguage(false);
}

void OfaQuoteTabPage::Reset( const SfxItemSet* )
{
    SvxAutoCorrect* pAutoCorrect = SvxAutoCorrCfg::Get().GetAutoCorrect();
    const ACFlags nFlags = pAutoCorrect->GetFlags();

    // Initialize the Sw options
    if (m_xSwCheckLB->get_visible())
    {
        SvxSwAutoFormatFlags *pOpt = &pAutoCorrect->GetSwFlags();

        m_xSwCheckLB->freeze();
        m_xSwCheckLB->clear();

        CreateEntry(*m_xSwCheckLB, sNonBrkSpace, CBCOL_BOTH, 2);
        CreateEntry(*m_xSwCheckLB, sOrdinal, CBCOL_BOTH, 2);

        m_xSwCheckLB->set_toggle(ADD_NONBRK_SPACE, pOpt->bAddNonBrkSpace, CBCOL_FIRST);
        m_xSwCheckLB->set_toggle(ADD_NONBRK_SPACE, bool(nFlags & ACFlags::AddNonBrkSpace), CBCOL_SECOND);
        m_xSwCheckLB->set_toggle(REPLACE_1ST, pOpt->bChgOrdinalNumber, CBCOL_FIRST);
        m_xSwCheckLB->set_toggle(REPLACE_1ST, bool(nFlags & ACFlags::ChgOrdinalNumber), CBCOL_SECOND);

        m_xSwCheckLB->thaw();
    }

    // Initialize the non Sw options
    if (m_xCheckLB->get_visible())
    {
        m_xCheckLB->freeze();
        m_xCheckLB->clear();

        CreateEntry(*m_xCheckLB, sNonBrkSpace, CBCOL_FIRST, 1);
        CreateEntry(*m_xCheckLB, sOrdinal, CBCOL_FIRST, 1);

        int nPos = 0;
        m_xCheckLB->set_toggle(nPos++, bool(nFlags & ACFlags::AddNonBrkSpace), CBCOL_FIRST);
        m_xCheckLB->set_toggle(nPos++, bool(nFlags & ACFlags::ChgOrdinalNumber), CBCOL_FIRST);

        m_xCheckLB->thaw();
    }

    // Initialize the quote stuffs
    m_xDoubleTypoCB->set_active(bool(nFlags & ACFlags::ChgQuotes));
    m_xSingleTypoCB->set_active(bool(nFlags & ACFlags::ChgSglQuotes));
    m_xDoubleTypoCB->save_state();
    m_xSingleTypoCB->save_state();

    cStartQuote = pAutoCorrect->GetStartDoubleQuote();
    cEndQuote = pAutoCorrect->GetEndDoubleQuote();
    cSglStartQuote = pAutoCorrect->GetStartSingleQuote();
    cSglEndQuote = pAutoCorrect->GetEndSingleQuote();

    m_xSglStartExFT->set_label(ChangeStringExt_Impl(cSglStartQuote));
    m_xSglEndExFT->set_label(ChangeStringExt_Impl(cSglEndQuote));
    m_xDblStartExFT->set_label(ChangeStringExt_Impl(cStartQuote));
    m_xDblEndExFT->set_label(ChangeStringExt_Impl(cEndQuote));
}

#define SGL_START       0
#define DBL_START       1
#define SGL_END         2
#define DBL_END         3


IMPL_LINK(OfaQuoteTabPage, QuoteHdl, weld::Button&, rBtn, void)
{
    sal_uInt16 nMode = SGL_START;
    if (&rBtn == m_xSglEndQuotePB.get())
        nMode = SGL_END;
    else if (&rBtn == m_xDblStartQuotePB.get())
        nMode = DBL_START;
    else if (&rBtn == m_xDblEndQuotePB.get())
        nMode = DBL_END;
    // start character selection dialog
    SvxCharacterMap aMap(GetDialogFrameWeld(), nullptr, false);
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
    if (aMap.run() == RET_OK)
    {
        sal_UCS4 cNewChar = aMap.GetChar();
        switch( nMode )
        {
            case SGL_START:
                cSglStartQuote = cNewChar;
                m_xSglStartExFT->set_label(ChangeStringExt_Impl(cNewChar));
            break;
            case SGL_END:
                cSglEndQuote = cNewChar;
                m_xSglEndExFT->set_label(ChangeStringExt_Impl(cNewChar));
            break;
            case DBL_START:
                cStartQuote = cNewChar;
                m_xDblStartExFT->set_label(ChangeStringExt_Impl(cNewChar));
            break;
            case DBL_END:
                cEndQuote = cNewChar;
                m_xDblEndExFT->set_label(ChangeStringExt_Impl(cNewChar));
            break;
        }
    }
}

IMPL_LINK(OfaQuoteTabPage, StdQuoteHdl, weld::Button&, rBtn, void)
{
    if (&rBtn == m_xDblStandardPB.get())
    {
        cStartQuote = 0;
        m_xDblStartExFT->set_label(ChangeStringExt_Impl(0));
        cEndQuote = 0;
        m_xDblEndExFT->set_label(ChangeStringExt_Impl(0));

    }
    else
    {
        cSglStartQuote = 0;
        m_xSglStartExFT->set_label(ChangeStringExt_Impl(0));
        cSglEndQuote = 0;
        m_xSglEndExFT->set_label(ChangeStringExt_Impl(0));
    }
}

OUString OfaQuoteTabPage::ChangeStringExt_Impl( sal_UCS4 cChar )
{
    if (!cChar)
        return m_xStandard->get_label();

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

OfaAutoCompleteTabPage::OfaAutoCompleteTabPage(TabPageParent pParent,
    const SfxItemSet& rSet)
    : SfxTabPage(pParent, "cui/ui/wordcompletionpage.ui",
                 "WordCompletionPage", &rSet)
    , m_pAutoCompleteList(nullptr)
    , m_nAutoCmpltListCnt(0)
    , m_xCBActiv(m_xBuilder->weld_check_button("enablewordcomplete"))
    , m_xCBAppendSpace(m_xBuilder->weld_check_button("appendspace"))
    , m_xCBAsTip(m_xBuilder->weld_check_button("showastip"))
    , m_xCBCollect(m_xBuilder->weld_check_button("collectwords"))
    , m_xCBRemoveList(m_xBuilder->weld_check_button("whenclosing"))
    , m_xDCBExpandKey(m_xBuilder->weld_combo_box("acceptwith"))
    , m_xNFMinWordlen(m_xBuilder->weld_spin_button("minwordlen"))
    , m_xNFMaxEntries(m_xBuilder->weld_spin_button("maxentries"))
    , m_xLBEntries(m_xBuilder->weld_tree_view("entries"))
    , m_xPBEntries(m_xBuilder->weld_button("delete"))
{
    //fdo#65595, we need height-for-width support here, but for now we can
    //bodge it
    Size aPrefSize(m_xCBRemoveList->get_preferred_size());
    int nMaxWidth = m_xCBRemoveList->get_approximate_digit_width() * 40;
    if (aPrefSize.Width() > nMaxWidth)
    {
        m_xCBRemoveList->set_label_line_wrap(true);
        m_xCBRemoveList->set_size_request(nMaxWidth, -1);
    }

    m_xLBEntries->set_size_request(m_xLBEntries->get_approximate_digit_width() * 30,
                                   m_xLBEntries->get_height_rows(10));

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
        vcl::KeyCode aKCode(*pKeys);
        m_xDCBExpandKey->append(OUString::number(static_cast<sal_Int32>(*pKeys)), aKCode.GetName());
        if (KEY_RETURN == *pKeys)      // default to RETURN
            m_xDCBExpandKey->set_active(std::distance(aKeyCodes, pKeys));
    }

    m_xPBEntries->connect_clicked(LINK(this, OfaAutoCompleteTabPage, DeleteHdl));
    m_xCBActiv->connect_toggled(LINK(this, OfaAutoCompleteTabPage, CheckHdl));
    m_xCBCollect->connect_toggled(LINK(this, OfaAutoCompleteTabPage, CheckHdl));
    m_xLBEntries->connect_key_release(LINK(this, OfaAutoCompleteTabPage, KeyReleaseHdl));
}

OfaAutoCompleteTabPage::~OfaAutoCompleteTabPage()
{
    disposeOnce();
}

void OfaSwAutoFmtOptionsPage::dispose()
{
    delete reinterpret_cast<ImpUserData*>(m_xCheckLB->get_id(REPLACE_BULLETS).toInt64());
    delete reinterpret_cast<ImpUserData*>(m_xCheckLB->get_id(APPLY_NUMBERING).toInt64());
    delete reinterpret_cast<ImpUserData*>(m_xCheckLB->get_id(MERGE_SINGLE_LINE_PARA).toInt64());
    SfxTabPage::dispose();
}

VclPtr<SfxTabPage> OfaAutoCompleteTabPage::Create(TabPageParent pParent,
                                                  const SfxItemSet* rSet)
{
    return VclPtr<OfaAutoCompleteTabPage>::Create(pParent, *rSet);
}

bool OfaAutoCompleteTabPage::FillItemSet( SfxItemSet* )
{
    bool bModified = false, bCheck;
    SvxAutoCorrect* pAutoCorrect = SvxAutoCorrCfg::Get().GetAutoCorrect();
    SvxSwAutoFormatFlags *pOpt = &pAutoCorrect->GetSwFlags();
    sal_uInt16 nVal;

    bCheck = m_xCBActiv->get_active();
    bModified |= pOpt->bAutoCompleteWords != bCheck;
    pOpt->bAutoCompleteWords = bCheck;
    bCheck = m_xCBCollect->get_active();
    bModified |= pOpt->bAutoCmpltCollectWords != bCheck;
    pOpt->bAutoCmpltCollectWords = bCheck;
    bCheck = !m_xCBRemoveList->get_active(); // inverted value!
    bModified |= pOpt->bAutoCmpltKeepList != bCheck;
    pOpt->bAutoCmpltKeepList = bCheck;
    bCheck = m_xCBAppendSpace->get_active();
    bModified |= pOpt->bAutoCmpltAppendBlanc != bCheck;
    pOpt->bAutoCmpltAppendBlanc = bCheck;
    bCheck = m_xCBAsTip->get_active();
    bModified |= pOpt->bAutoCmpltShowAsTip != bCheck;
    pOpt->bAutoCmpltShowAsTip = bCheck;

    nVal = static_cast<sal_uInt16>(m_xNFMinWordlen->get_value());
    bModified |= nVal != pOpt->nAutoCmpltWordLen;
    pOpt->nAutoCmpltWordLen = nVal;

    nVal = static_cast<sal_uInt16>(m_xNFMaxEntries->get_value());
    bModified |= nVal != pOpt->nAutoCmpltListLen;
    pOpt->nAutoCmpltListLen = nVal;

    const int nPos = m_xDCBExpandKey->get_active();
    if (nPos != -1)
    {
        sal_Int32 nKey = m_xDCBExpandKey->get_id(nPos).toInt32();
        bModified |= nKey != pOpt->nAutoCmpltExpandKey;
        pOpt->nAutoCmpltExpandKey = static_cast<sal_uInt16>(nKey);
    }

    if (m_pAutoCompleteList && m_nAutoCmpltListCnt != m_xLBEntries->n_children())
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

    m_xCBActiv->set_active( pOpt->bAutoCompleteWords );
    m_xCBCollect->set_active( pOpt->bAutoCmpltCollectWords );
    m_xCBRemoveList->set_active( !pOpt->bAutoCmpltKeepList ); //inverted value!
    m_xCBAppendSpace->set_active( pOpt->bAutoCmpltAppendBlanc );
    m_xCBAsTip->set_active( pOpt->bAutoCmpltShowAsTip );

    m_xNFMinWordlen->set_value( pOpt->nAutoCmpltWordLen );
    m_xNFMaxEntries->set_value( pOpt->nAutoCmpltListLen );

    // select the specific KeyCode:
    {
        sal_Int32 nKey = pOpt->nAutoCmpltExpandKey;
        for (int n = 0, nCnt = m_xDCBExpandKey->get_count(); n < nCnt; ++n)
        {
            if (nKey == m_xDCBExpandKey->get_id(n).toInt32())
            {
                m_xDCBExpandKey->set_active(n);
                break;
            }
        }
    }

    if (pOpt->m_pAutoCompleteList && !pOpt->m_pAutoCompleteList->empty())
    {
        m_pAutoCompleteList = const_cast<editeng::SortedAutoCompleteStrings*>(
                pOpt->m_pAutoCompleteList);
        pOpt->m_pAutoCompleteList = nullptr;
        m_nAutoCmpltListCnt = m_pAutoCompleteList->size();
        for (size_t n = 0; n < m_nAutoCmpltListCnt; ++n)
        {
            const OUString* pStr =
                &(*m_pAutoCompleteList)[n]->GetAutoCompleteString();
            OUString sId(OUString::number(reinterpret_cast<sal_Int64>(pStr)));
            m_xLBEntries->append(sId, *pStr);
        }
    }
    else
    {
        m_xLBEntries->set_sensitive(false);
        m_xPBEntries->set_sensitive(false);
    }

    CheckHdl(*m_xCBActiv);
    CheckHdl(*m_xCBCollect);
}

void OfaAutoCompleteTabPage::ActivatePage( const SfxItemSet& )
{
    static_cast<OfaAutoCorrDlg*>(GetDialogController())->EnableLanguage( false );
}

IMPL_LINK_NOARG(OfaAutoCompleteTabPage, DeleteHdl, weld::Button&, void)
{
    auto rows = m_xLBEntries->get_selected_rows();
    std::sort(rows.begin(), rows.end());
    while (!rows.empty())
    {
        sal_Int32 nPos = rows.back();
        OUString* pStr = reinterpret_cast<OUString*>(m_xLBEntries->get_id(nPos).toInt64());
        m_xLBEntries->remove(nPos);
        editeng::IAutoCompleteString hack(*pStr); // UGLY
        m_pAutoCompleteList->erase(&hack);
        rows.pop_back();
    }
}

IMPL_LINK(OfaAutoCompleteTabPage, CheckHdl, weld::ToggleButton&, rBox, void)
{
    bool bEnable = rBox.get_active();
    if (&rBox == m_xCBActiv.get())
    {
        m_xCBAppendSpace->set_sensitive(bEnable);
        m_xCBAppendSpace->set_sensitive(bEnable);
        m_xCBAsTip->set_sensitive(bEnable);
        m_xDCBExpandKey->set_sensitive(bEnable);
    }
    else if (&rBox == m_xCBCollect.get())
        m_xCBRemoveList->set_sensitive(bEnable);
}

void OfaAutoCompleteTabPage::CopyToClipboard() const
{
    auto rows = m_xLBEntries->get_selected_rows();
    if (m_pAutoCompleteList && !rows.empty())
    {
        rtl::Reference<TransferDataContainer> pCntnr = new TransferDataContainer;

        OStringBuffer sData;

        rtl_TextEncoding nEncode = osl_getThreadTextEncoding();

        for (auto a : rows)
        {
            sData.append(OUStringToOString(m_xLBEntries->get_text(a), nEncode));
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

IMPL_LINK(OfaAutoCompleteTabPage, KeyReleaseHdl, const KeyEvent&, rEvent, bool)
{
    bool bHandled = false;
    const vcl::KeyCode& rKeyCode = rEvent.GetKeyCode();
    switch (rKeyCode.GetModifier() | rKeyCode.GetCode())
    {
        case KEY_DELETE:
            DeleteHdl(*m_xPBEntries);
            bHandled = true;
            break;
        default:
            if (KeyFuncType::COPY == rKeyCode.GetFunction())
            {
                CopyToClipboard();
                bHandled = true;
            }
            break;
    }
    return bHandled;
}

// class OfaSmartTagOptionsTabPage ---------------------------------------------

OfaSmartTagOptionsTabPage::OfaSmartTagOptionsTabPage(TabPageParent pParent,
                                                     const SfxItemSet& rSet )
    : SfxTabPage(pParent, "cui/ui/smarttagoptionspage.ui", "SmartTagOptionsPage", &rSet)
    , m_xMainCB(m_xBuilder->weld_check_button("main"))
    , m_xSmartTagTypesLB(m_xBuilder->weld_tree_view("list"))
    , m_xPropertiesPB(m_xBuilder->weld_button("properties"))
{
    m_xSmartTagTypesLB->set_size_request(m_xSmartTagTypesLB->get_approximate_digit_width() * 50,
                                         m_xSmartTagTypesLB->get_height_rows(6));

    std::vector<int> aWidths;
    aWidths.push_back(m_xSmartTagTypesLB->get_checkbox_column_width());
    m_xSmartTagTypesLB->set_column_fixed_widths(aWidths);

    // set the handlers:
    m_xMainCB->connect_toggled(LINK(this, OfaSmartTagOptionsTabPage, CheckHdl));
    m_xPropertiesPB->connect_clicked(LINK(this, OfaSmartTagOptionsTabPage, ClickHdl));
    m_xSmartTagTypesLB->connect_changed(LINK(this, OfaSmartTagOptionsTabPage, SelectHdl));
}

OfaSmartTagOptionsTabPage::~OfaSmartTagOptionsTabPage()
{
    disposeOnce();
}

VclPtr<SfxTabPage> OfaSmartTagOptionsTabPage::Create(TabPageParent pParent, const SfxItemSet* rSet)
{
    return VclPtr<OfaSmartTagOptionsTabPage>::Create(pParent, *rSet);
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

/** Clears m_xSmartTagTypesLB
*/
void OfaSmartTagOptionsTabPage::ClearListBox()
{
    const int nCount = m_xSmartTagTypesLB->n_children();
    for (int i = 0; i < nCount; ++i)
    {
        const ImplSmartTagLBUserData* pUserData = reinterpret_cast<ImplSmartTagLBUserData*>(m_xSmartTagTypesLB->get_id(i).toInt64());
        delete pUserData;
    }

    m_xSmartTagTypesLB->clear();
}

/** Inserts items into m_xSmartTagTypesLB
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
        const uno::Reference< smarttags::XSmartTagRecognizer >& xRec = rSmartTagMgr.GetRecognizer(i);

        const OUString aName = xRec->getName( aLocale );
        const sal_Int32 nNumberOfSupportedSmartTags = xRec->getSmartTagCount();

        for ( sal_Int32 j = 0; j < nNumberOfSupportedSmartTags; ++j )
        {
            const OUString aSmartTagType = xRec->getSmartTagName(j);
            OUString aSmartTagCaption = rSmartTagMgr.GetSmartTagCaption( aSmartTagType, aLocale );

            if ( aSmartTagCaption.isEmpty() )
                aSmartTagCaption = aSmartTagType;

            const OUString aLBEntry = aSmartTagCaption + " (" + aName + ")";

            m_xSmartTagTypesLB->append();
            const int nRow = m_xSmartTagTypesLB->n_children() - 1;
            const bool bCheck = rSmartTagMgr.IsSmartTagTypeEnabled( aSmartTagType );
            m_xSmartTagTypesLB->set_toggle(nRow, bCheck, CBCOL_FIRST);
            m_xSmartTagTypesLB->set_text(nRow, aLBEntry, 1);
            m_xSmartTagTypesLB->set_id(nRow, OUString::number(reinterpret_cast<sal_Int64>(new ImplSmartTagLBUserData(aSmartTagType, xRec, j))));
        }
    }
}

/** Handler for the push button
*/
IMPL_LINK_NOARG(OfaSmartTagOptionsTabPage, ClickHdl, weld::Button&, void)
{
    const int nPos = m_xSmartTagTypesLB->get_selected_index();
    const ImplSmartTagLBUserData* pUserData = reinterpret_cast<ImplSmartTagLBUserData*>(m_xSmartTagTypesLB->get_id(nPos).toInt64());
    uno::Reference< smarttags::XSmartTagRecognizer > xRec = pUserData->mxRec;
    const sal_Int32 nSmartTagIdx = pUserData->mnSmartTagIdx;

    const lang::Locale aLocale( LanguageTag::convertToLocale( eLastDialogLanguage ) );
    if ( xRec->hasPropertyPage( nSmartTagIdx, aLocale ) )
        xRec->displayPropertyPage( nSmartTagIdx, aLocale );
}

/** Handler for the check box
*/
IMPL_LINK_NOARG(OfaSmartTagOptionsTabPage, CheckHdl, weld::ToggleButton&, void)
{
    const bool bEnable = m_xMainCB->get_active();
    m_xSmartTagTypesLB->set_sensitive(bEnable);
    m_xPropertiesPB->set_sensitive(false);

    // if the controls are currently enabled, we still have to check
    // if the properties button should be disabled because the currently
    // selected smart tag type does not have a properties dialog.
    // We do this by calling SelectHdl:
    if (bEnable)
        SelectHdl(*m_xSmartTagTypesLB);
}

/** Handler for the list box
*/
IMPL_LINK_NOARG(OfaSmartTagOptionsTabPage, SelectHdl, weld::TreeView&, void)
{
    const int nPos = m_xSmartTagTypesLB->get_selected_index();
    if (nPos == -1)
        return;
    const ImplSmartTagLBUserData* pUserData = reinterpret_cast<ImplSmartTagLBUserData*>(m_xSmartTagTypesLB->get_id(nPos).toInt64());
    uno::Reference< smarttags::XSmartTagRecognizer > xRec = pUserData->mxRec;
    const sal_Int32 nSmartTagIdx = pUserData->mnSmartTagIdx;

    const lang::Locale aLocale( LanguageTag::convertToLocale( eLastDialogLanguage ) );
    if ( xRec->hasPropertyPage( nSmartTagIdx, aLocale ) )
        m_xPropertiesPB->set_sensitive(true);
    else
        m_xPropertiesPB->set_sensitive(false);
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

    const int nCount = m_xSmartTagTypesLB->n_children();

    for (int i = 0; i < nCount; ++i)
    {
        const ImplSmartTagLBUserData* pUserData = reinterpret_cast<ImplSmartTagLBUserData*>(m_xSmartTagTypesLB->get_id(i).toInt64());
        const bool bChecked = m_xSmartTagTypesLB->get_toggle(i, CBCOL_FIRST);
        const bool bIsCurrentlyEnabled = pSmartTagMgr->IsSmartTagTypeEnabled( pUserData->maSmartTagType );

        bModifiedSmartTagTypes = bModifiedSmartTagTypes || ( !bChecked != !bIsCurrentlyEnabled );

        if ( !bChecked )
            aDisabledSmartTagTypes.push_back( pUserData->maSmartTagType );

        delete pUserData;
    }

    const bool bModifiedRecognize = ( !m_xMainCB->get_active() != !pSmartTagMgr->IsLabelTextWithSmartTags() );
    if ( bModifiedSmartTagTypes || bModifiedRecognize )
    {
        bool bLabelTextWithSmartTags = m_xMainCB->get_active();
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

    FillListBox(*pSmartTagMgr);
    m_xSmartTagTypesLB->select(0);
    m_xMainCB->set_active(pSmartTagMgr->IsLabelTextWithSmartTags());
    CheckHdl(*m_xMainCB);
}

void OfaSmartTagOptionsTabPage::ActivatePage( const SfxItemSet& )
{
    static_cast<OfaAutoCorrDlg*>(GetDialogController())->EnableLanguage( false );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
