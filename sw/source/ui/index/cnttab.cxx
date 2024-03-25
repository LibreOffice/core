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

#include <osl/diagnose.h>
#include <o3tl/safeint.hxx>
#include <sal/log.hxx>
#include <svl/style.hxx>
#include <utility>
#include <vcl/weld.hxx>
#include <svl/stritem.hxx>
#include <unotools/pathoptions.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/sfxdlg.hxx>
#include <svx/dialogs.hrc>
#include <svx/flagsdef.hxx>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <com/sun/star/ui/dialogs/XFilePicker3.hpp>
#include <svtools/indexentryres.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <column.hxx>
#include <fmtfsize.hxx>
#include <authfld.hxx>
#include <swtypes.hxx>
#include <wrtsh.hxx>
#include <view.hxx>
#include <cnttab.hxx>
#include <swuicnttab.hxx>
#include <poolfmt.hxx>
#include <strings.hrc>
#include <uitool.hxx>
#include <fmtcol.hxx>
#include <fldbas.hxx>
#include <expfld.hxx>
#include <unotools.hxx>
#include <docsh.hxx>
#include <swmodule.hxx>
#include <modcfg.hxx>
#include <iodetect.hxx>

#include <cmdid.h>
#include <cnttab.hrc>
#include <SwStyleNameMapper.hxx>
#include <sfx2/filedlghelper.hxx>
#include <toxwrap.hxx>
#include <chpfld.hxx>

#include <cmath>
#include <memory>
#include <string_view>
#include <vector>
#include <numeric>


using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace com::sun::star::ui::dialogs;
using namespace ::sfx2;
#include <svtools/editbrowsebox.hxx>

const sal_Unicode aDeliStart = '['; // for the form
const sal_Unicode aDeliEnd    = ']'; // for the form

static OUString lcl_CreateAutoMarkFileDlg(weld::Window* pParent, const OUString& rURL,
                                const OUString& rFileString, bool bOpen)
{
    OUString sRet;

    FileDialogHelper aDlgHelper( bOpen ?
                TemplateDescription::FILEOPEN_SIMPLE : TemplateDescription::FILESAVE_AUTOEXTENSION,
                FileDialogFlags::NONE, pParent);
    uno::Reference < XFilePicker3 > xFP = aDlgHelper.GetFilePicker();

    xFP->appendFilter( rFileString, "*.sdi" );
    xFP->setCurrentFilter( rFileString ) ;

    if( !rURL.isEmpty() )
        xFP->setDisplayDirectory( rURL );
    else
    {
        SvtPathOptions aPathOpt;
        xFP->setDisplayDirectory( aPathOpt.GetUserConfigPath() );
    }

    const ErrCode aErrCode = aDlgHelper.Execute();
    if (aErrCode == ERRCODE_NONE)
    {
        sRet = xFP->getSelectedFiles().getConstArray()[0];
    }
    // tdf#120405 - use previously selected file, if selection is aborted
    else if (aErrCode == ERRCODE_ABORT && !rURL.isEmpty())
    {
        sRet = rURL;
    }

    return sRet;
}

namespace {

struct AutoMarkEntry
{
    OUString sSearch;
    OUString sAlternative;
    OUString sPrimKey;
    OUString sSecKey;
    OUString sComment;
    bool     bCase;
    bool     bWord;

    AutoMarkEntry() :
        bCase(false),
        bWord(false){}
};

}

typedef ::svt::EditBrowseBox SwEntryBrowseBox_Base;

namespace {

class SwEntryBrowseBox : public SwEntryBrowseBox_Base
{
    VclPtr<svt::EditControl> m_aCellEdit;
    VclPtr<svt::CheckBoxControl> m_aCellCheckBox;

    OUString  m_sYes;
    OUString  m_sNo;

    std::vector<std::unique_ptr<AutoMarkEntry>> m_Entries;

    ::svt::CellControllerRef    m_xController;
    ::svt::CellControllerRef    m_xCheckController;

    sal_Int32    m_nCurrentRow;
    bool    m_bModified;

protected:
    virtual bool                    SeekRow( sal_Int32 nRow ) override;
    virtual void                    PaintCell(OutputDevice& rDev, const tools::Rectangle& rRect, sal_uInt16 nColId) const override;
    virtual void                    InitController(::svt::CellControllerRef& rController, sal_Int32 nRow, sal_uInt16 nCol) override;
    virtual ::svt::CellController*  GetController(sal_Int32 nRow, sal_uInt16 nCol) override;
    virtual bool                    SaveModified() override;

    std::vector<tools::Long>               GetOptimalColWidths() const;

public:
    SwEntryBrowseBox(const css::uno::Reference<css::awt::XWindow> &rParent);
    virtual ~SwEntryBrowseBox() override;
    virtual void                    dispose() override;
    void                            ReadEntries(SvStream& rInStr);
    void                            WriteEntries(SvStream& rOutStr);

    bool                            IsModified()const override;

    virtual OUString GetCellText( sal_Int32 nRow, sal_uInt16 nColumn ) const override;
    virtual void                    Resize() override;
    virtual Size                    GetOptimalSize() const override;
};

class SwAutoMarkDlg_Impl : public weld::GenericDialogController
{
    OUString            m_sAutoMarkURL;
    bool                m_bCreateMode;

    std::unique_ptr<weld::Button> m_xOKPB;
    std::unique_ptr<weld::Container> m_xTable;
    css::uno::Reference<css::awt::XWindow> m_xTableCtrlParent;
    VclPtr<SwEntryBrowseBox> m_xEntriesBB;

    DECL_LINK(OkHdl, weld::Button&, void);
public:
    SwAutoMarkDlg_Impl(weld::Window* pParent, OUString aAutoMarkURL,
                       bool bCreate);
    virtual ~SwAutoMarkDlg_Impl() override;
};

}

sal_uInt16 CurTOXType::GetFlatIndex() const
{
    return static_cast< sal_uInt16 >( (eType == TOX_USER && nIndex)
        ? TOX_AUTHORITIES + nIndex : eType );
}

SwMultiTOXTabDialog::SwMultiTOXTabDialog(weld::Widget* pParent, const SfxItemSet& rSet,
                                         SwWrtShell &rShell, SwTOXBase* pCurTOX,
                                         sal_uInt16 nToxType, bool bGlobal)
    : SfxTabDialogController(pParent, "modules/swriter/ui/tocdialog.ui", "TocDialog", &rSet)
    , m_pMgr( new SwTOXMgr( &rShell ) )
    , m_rWrtShell(rShell)
    , m_pParamTOXBase(pCurTOX)
    , m_sUserDefinedIndex(SwResId(STR_USER_DEFINED_INDEX))
    , m_nInitialTOXType(nToxType)
    , m_bEditTOX(false)
    , m_bExampleCreated(false)
    , m_bGlobalFlag(bGlobal)
    , m_xShowExampleCB(m_xBuilder->weld_check_button("showexample"))
{
    m_eCurrentTOXType.eType = TOX_CONTENT;
    m_eCurrentTOXType.nIndex = 0;

    const sal_uInt16 nUserTypeCount = m_rWrtShell.GetTOXTypeCount(TOX_USER);
    m_vTypeData.resize(nUserTypeCount + 6);
    //the standard user index is on position TOX_USER
    //all user indexes follow after position TOX_AUTHORITIES
    if(pCurTOX)
    {
        m_bEditTOX = true;
    }
    for(int i = m_vTypeData.size() - 1; i > -1; i--)
    {
        m_vTypeData[i].m_oIndexSections.emplace();
        if(pCurTOX)
        {
            m_eCurrentTOXType.eType = pCurTOX->GetType();
            sal_uInt16 nArrayIndex = static_cast< sal_uInt16 >(m_eCurrentTOXType.eType);
            if(m_eCurrentTOXType.eType == TOX_USER)
            {
                //which user type is it?
                for(sal_uInt16 nUser = 0; nUser < nUserTypeCount; nUser++)
                {
                    const SwTOXType* pTemp = m_rWrtShell.GetTOXType(TOX_USER, nUser);
                    if(pCurTOX->GetTOXType() == pTemp)
                    {
                        m_eCurrentTOXType.nIndex = nUser;
                        nArrayIndex = static_cast< sal_uInt16 >(nUser > 0 ? TOX_AUTHORITIES + nUser : TOX_USER);
                        break;
                    }
                }
            }
            m_vTypeData[nArrayIndex].m_pForm.reset(new SwForm(pCurTOX->GetTOXForm()));
            m_vTypeData[nArrayIndex].m_pDescription = CreateTOXDescFromTOXBase(pCurTOX);
            if(TOX_AUTHORITIES == m_eCurrentTOXType.eType)
            {
                const SwAuthorityFieldType* pFType = static_cast<const SwAuthorityFieldType*>(
                                                m_rWrtShell.GetFieldType(SwFieldIds::TableOfAuthorities, OUString()));
                if(pFType)
                {
                    OUString sBrackets;
                    if(pFType->GetPrefix())
                        sBrackets += OUStringChar(pFType->GetPrefix());
                    if(pFType->GetSuffix())
                        sBrackets += OUStringChar(pFType->GetSuffix());
                    m_vTypeData[nArrayIndex].m_pDescription->SetAuthBrackets(sBrackets);
                    m_vTypeData[nArrayIndex].m_pDescription->SetAuthSequence(pFType->IsSequence());
                }
                else
                {
                    m_vTypeData[nArrayIndex].m_pDescription->SetAuthBrackets("[]");
                }
            }
        }
    }
    SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();
    AddTabPage("index", SwTOXSelectTabPage::Create, nullptr);
    AddTabPage("styles", SwTOXStylesTabPage::Create, nullptr);
    AddTabPage("columns", SwColumnPage::Create, nullptr);
    AddTabPage("background", pFact->GetTabPageCreatorFunc(RID_SVXPAGE_BKG), nullptr);
    AddTabPage("entries", SwTOXEntryTabPage::Create, nullptr);
    if (!pCurTOX)
        SetCurPageId("index");

    m_xShowExampleCB->connect_toggled(LINK(this, SwMultiTOXTabDialog, ShowPreviewHdl));
    m_xShowExampleCB->set_active(SW_MOD()->GetModuleConfig()->IsShowIndexPreview());

    ShowPreview();
}

SwMultiTOXTabDialog::~SwMultiTOXTabDialog()
{
    SW_MOD()->GetModuleConfig()->SetShowIndexPreview(m_xShowExampleCB->get_active());
}

void SwMultiTOXTabDialog::PageCreated(const OUString& rId, SfxTabPage &rPage)
{
    if (rId == "background")
    {
        SfxAllItemSet aSet(*(GetInputSetImpl()->GetPool()));
        aSet.Put (SfxUInt32Item(SID_FLAG_TYPE, static_cast<sal_uInt32>(SvxBackgroundTabFlags::SHOW_SELECTOR)));
        rPage.PageCreated(aSet);
    }
    else if (rId == "columns")
    {
        const SwFormatFrameSize& rSize = GetInputSetImpl()->Get(RES_FRM_SIZE);

        static_cast<SwColumnPage&>(rPage).SetPageWidth(rSize.GetWidth());
    }
    else if (rId == "entries")
        static_cast<SwTOXEntryTabPage&>(rPage).SetWrtShell(m_rWrtShell);
    else if (rId == "index")
    {
        static_cast<SwTOXSelectTabPage&>(rPage).SetWrtShell(m_rWrtShell);
        if(USHRT_MAX != m_nInitialTOXType)
            static_cast<SwTOXSelectTabPage&>(rPage).SelectType(static_cast<TOXTypes>(m_nInitialTOXType));
    }
}

short SwMultiTOXTabDialog::Ok()
{
    short nRet = SfxTabDialogController::Ok();
    SwTOXDescription& rDesc = GetTOXDescription(m_eCurrentTOXType);
    SwTOXBase aNewDef(*m_rWrtShell.GetDefaultTOXBase( m_eCurrentTOXType.eType, true ));

    const sal_uInt16 nIndex = m_eCurrentTOXType.GetFlatIndex();
    if(m_vTypeData[nIndex].m_pForm)
    {
        rDesc.SetForm(*m_vTypeData[nIndex].m_pForm);
        aNewDef.SetTOXForm(*m_vTypeData[nIndex].m_pForm);
    }
    rDesc.ApplyTo(aNewDef);
    if(!m_bGlobalFlag)
        m_pMgr->UpdateOrInsertTOX(
                rDesc, nullptr, GetOutputItemSet());
    else if(m_bEditTOX)
        m_pMgr->UpdateOrInsertTOX(
                rDesc, &m_pParamTOXBase, GetOutputItemSet());

    if(!m_eCurrentTOXType.nIndex)
        m_rWrtShell.SetDefaultTOXBase(aNewDef);

    return nRet;
}

SwForm* SwMultiTOXTabDialog::GetForm(CurTOXType eType)
{
    const sal_uInt16 nIndex = eType.GetFlatIndex();
    if(!m_vTypeData[nIndex].m_pForm)
        m_vTypeData[nIndex].m_pForm.reset(new SwForm(eType.eType));
    return m_vTypeData[nIndex].m_pForm.get();
}

SwTOXDescription& SwMultiTOXTabDialog::GetTOXDescription(CurTOXType eType)
{
    const sal_uInt16 nIndex = eType.GetFlatIndex();
    if(!m_vTypeData[nIndex].m_pDescription)
    {
        const SwTOXBase* pDef = m_rWrtShell.GetDefaultTOXBase( eType.eType );
        if(pDef)
            m_vTypeData[nIndex].m_pDescription = CreateTOXDescFromTOXBase(pDef);
        else
        {
            m_vTypeData[nIndex].m_pDescription.reset(new SwTOXDescription(eType.eType));
            if(eType.eType == TOX_USER)
                m_vTypeData[nIndex].m_pDescription->SetTitle(m_sUserDefinedIndex);
            else
                m_vTypeData[nIndex].m_pDescription->SetTitle(
                    m_rWrtShell.GetTOXType(eType.eType, 0)->GetTypeName());
        }
        if(TOX_AUTHORITIES == eType.eType)
        {
            const SwAuthorityFieldType* pFType = static_cast<const SwAuthorityFieldType*>(
                                            m_rWrtShell.GetFieldType(SwFieldIds::TableOfAuthorities, OUString()));
            if(pFType)
            {
                m_vTypeData[nIndex].m_pDescription->SetAuthBrackets(OUStringChar(pFType->GetPrefix()) +
                                                  OUStringChar(pFType->GetSuffix()));
                m_vTypeData[nIndex].m_pDescription->SetAuthSequence(pFType->IsSequence());
            }
            else
            {
                m_vTypeData[nIndex].m_pDescription->SetAuthBrackets("[]");
            }
        }
        else if(TOX_INDEX == eType.eType)
            m_vTypeData[nIndex].m_pDescription->SetMainEntryCharStyle(SwResId(STR_POOLCHR_IDX_MAIN_ENTRY));

    }
    return *m_vTypeData[nIndex].m_pDescription;
}

std::unique_ptr<SwTOXDescription> SwMultiTOXTabDialog::CreateTOXDescFromTOXBase(
            const SwTOXBase*pCurTOX)
{
    std::unique_ptr<SwTOXDescription> pDesc(new SwTOXDescription(pCurTOX->GetType()));
    for(sal_uInt16 i = 0; i < MAXLEVEL; i++)
        pDesc->SetStyleNames(pCurTOX->GetStyleNames(i), i);
    pDesc->SetAutoMarkURL(m_rWrtShell.GetTOIAutoMarkURL());
    pDesc->SetTitle(pCurTOX->GetTitle());

    pDesc->SetContentOptions(pCurTOX->GetCreateType());
    if(pDesc->GetTOXType() == TOX_INDEX)
        pDesc->SetIndexOptions(pCurTOX->GetOptions());
    pDesc->SetMainEntryCharStyle(pCurTOX->GetMainEntryCharStyle());
    if(pDesc->GetTOXType() != TOX_INDEX)
        pDesc->SetLevel(static_cast<sal_uInt8>(pCurTOX->GetLevel()));
    pDesc->SetCreateFromObjectNames(pCurTOX->IsFromObjectNames());
    pDesc->SetSequenceName(pCurTOX->GetSequenceName());
    pDesc->SetCaptionDisplay(pCurTOX->GetCaptionDisplay());
    pDesc->SetFromChapter(pCurTOX->IsFromChapter());
    pDesc->SetReadonly(pCurTOX->IsProtected());
    pDesc->SetOLEOptions(pCurTOX->GetOLEOptions());
    pDesc->SetLevelFromChapter(pCurTOX->IsLevelFromChapter());
    pDesc->SetLanguage(pCurTOX->GetLanguage());
    pDesc->SetSortAlgorithm(pCurTOX->GetSortAlgorithm());
    return pDesc;
}

void SwMultiTOXTabDialog::ShowPreview()
{
    if (m_xShowExampleCB->get_active())
    {
        if(!m_xExampleFrame && !m_bExampleCreated)
        {
            m_bExampleCreated = true;
            OUString sTemplate("internal/idxexample.odt");

            SvtPathOptions aOpt;
            bool bExist = aOpt.SearchFile( sTemplate, SvtPathOptions::Paths::Template );

            if(!bExist)
            {
                OUString sInfo(SwResId(STR_FILE_NOT_FOUND));
                sInfo = sInfo.replaceFirst( "%1", sTemplate );
                sInfo = sInfo.replaceFirst( "%2", aOpt.GetTemplatePath() );
                std::unique_ptr<weld::MessageDialog> xInfoBox(Application::CreateMessageDialog(m_xDialog.get(),
                                                              VclMessageType::Info, VclButtonsType::Ok,
                                                              sInfo));
                xInfoBox->run();
            }
            else
            {
                Link<SwOneExampleFrame&,void> aLink(LINK(this, SwMultiTOXTabDialog, CreateExample_Hdl));
                m_xExampleFrame.reset(new SwOneExampleFrame(EX_SHOW_ONLINE_LAYOUT | EX_LOCALIZE_TOC_STRINGS, &aLink, &sTemplate));
                m_xExampleFrameWin.reset(new weld::CustomWeld(*m_xBuilder, "example", *m_xExampleFrame));
            }
            m_xShowExampleCB->set_visible(m_xExampleFrame != nullptr);
        }
    }

    if (m_xExampleFrame)
    {
        const bool bSetViewWindow = m_xShowExampleCB->get_active();
        if (bSetViewWindow)
            m_xExampleFrame->Show();
        else
            m_xExampleFrame->Hide();
    }
}

IMPL_LINK_NOARG(SwMultiTOXTabDialog, ShowPreviewHdl, weld::Toggleable&, void)
{
    ShowPreview();
    m_xDialog->resize_to_request();
}

bool SwMultiTOXTabDialog::IsNoNum(SwWrtShell& rSh, const OUString& rName)
{
    SwTextFormatColl* pColl = rSh.GetParaStyle(rName);
    if(pColl && ! pColl->IsAssignedToListLevelOfOutlineStyle())
        return true;

    const sal_uInt16 nId = SwStyleNameMapper::GetPoolIdFromUIName(
        rName, SwGetPoolIdFromName::TxtColl);
    return nId != USHRT_MAX &&
        ! rSh.GetTextCollFromPool(nId)->IsAssignedToListLevelOfOutlineStyle();
}

namespace {

class SwAddStylesDlg_Impl : public SfxDialogController
{
    OUString*       m_pStyleArr;

    std::unique_ptr<weld::Button> m_xOk;
    std::unique_ptr<weld::Button> m_xLeftPB;
    std::unique_ptr<weld::Button> m_xRightPB;
    std::unique_ptr<weld::TreeView> m_xHeaderTree;

    void ToggleOn(int nEntry, int nToggleColumn);

    DECL_LINK(OkHdl, weld::Button&, void);
    DECL_LINK(LeftRightHdl, weld::Button&, void);
    DECL_LINK(KeyInput, const KeyEvent&, bool);
    DECL_LINK(TreeSizeAllocHdl, const Size&, void);
    DECL_LINK(RadioToggleOnHdl, const weld::TreeView::iter_col&, void);
    DECL_LINK(HeaderBarClick, int, void);

public:
    SwAddStylesDlg_Impl(weld::Window* pParent, SwWrtShell const & rWrtSh, OUString rStringArr[]);
};

}

SwAddStylesDlg_Impl::SwAddStylesDlg_Impl(weld::Window* pParent,
            SwWrtShell const & rWrtSh, OUString rStringArr[])
    : SfxDialogController(pParent, "modules/swriter/ui/assignstylesdialog.ui", "AssignStylesDialog")
    , m_pStyleArr(rStringArr)
    , m_xOk(m_xBuilder->weld_button("ok"))
    , m_xLeftPB(m_xBuilder->weld_button("left"))
    , m_xRightPB(m_xBuilder->weld_button("right"))
    , m_xHeaderTree(m_xBuilder->weld_tree_view("styles"))
{
    m_xOk->connect_clicked(LINK(this, SwAddStylesDlg_Impl, OkHdl));
    m_xLeftPB->connect_clicked(LINK(this, SwAddStylesDlg_Impl, LeftRightHdl));
    m_xRightPB->connect_clicked(LINK(this, SwAddStylesDlg_Impl, LeftRightHdl));

    m_xHeaderTree->connect_size_allocate(LINK(this, SwAddStylesDlg_Impl, TreeSizeAllocHdl));
    m_xHeaderTree->enable_toggle_buttons(weld::ColumnToggleType::Radio);
    m_xHeaderTree->connect_toggled(LINK(this, SwAddStylesDlg_Impl, RadioToggleOnHdl));
    m_xHeaderTree->connect_column_clicked(LINK(this, SwAddStylesDlg_Impl, HeaderBarClick));

    std::vector<int> aWidths
    {
        o3tl::narrowing<int>(m_xHeaderTree->get_approximate_digit_width() * 30)
    };
    int nPadding = m_xHeaderTree->get_approximate_digit_width() * 2;
    OUString sTitle(m_xHeaderTree->get_column_title(1));
    for (sal_uInt16 i = 0; i <= MAXLEVEL; ++i)
    {
        sTitle = OUString::number(i);
        m_xHeaderTree->set_column_title(i + 1, sTitle);
        aWidths.push_back(m_xHeaderTree->get_pixel_size(sTitle).Width() + nPadding);
    }
    m_xHeaderTree->set_column_fixed_widths(aWidths);
    auto nWidth = std::accumulate(aWidths.begin(), aWidths.end(),
                                  Application::GetSettings().GetStyleSettings().GetScrollBarSize());
    m_xHeaderTree->set_size_request(nWidth, m_xHeaderTree->get_height_rows(15));

    int nRow(0);
    for (sal_uInt16 i = 0; i < MAXLEVEL; ++i)
    {
        const OUString &rStyles{rStringArr[i]};
        if (rStyles.isEmpty())
            continue;
        sal_Int32 nPos(0);
        do
        {
            OUString sEntry = rStyles.getToken(0, TOX_STYLE_DELIMITER, nPos);
            m_xHeaderTree->append_text(sEntry);
            for (sal_uInt16 j = 0; j <= MAXLEVEL; ++j)
            {
                TriState eState = i == j - 1 ? TRISTATE_TRUE : TRISTATE_FALSE;
                m_xHeaderTree->set_toggle(nRow, eState, j + 1);
            }
            ++nRow;
        } while (nPos>=0);
    }
    // now the other styles

    const sal_uInt16 nSz = rWrtSh.GetTextFormatCollCount();
    for (sal_uInt16 j = 0; j < nSz; ++j)
    {
        const SwTextFormatColl& rColl = rWrtSh.GetTextFormatColl(j);
        if (rColl.IsDefault())
            continue;

        const OUString aName = rColl.GetName();
        if (!aName.isEmpty())
        {
            bool bEntry = false;
            int nChildren = m_xHeaderTree->n_children();
            for (int i = 0; i < nChildren; ++i)
            {
                if (m_xHeaderTree->get_text(i, 0) == aName)
                {
                    bEntry = true;
                    break;
                }
            }
            if (!bEntry)
            {
                m_xHeaderTree->append_text(aName);
                for (sal_uInt16 k = 0; k <= MAXLEVEL; ++k)
                {
                    TriState eState = k == 0 ? TRISTATE_TRUE : TRISTATE_FALSE;
                    m_xHeaderTree->set_toggle(nRow, eState, k + 1);
                }
                ++nRow;
            }
        }
    }

    m_xHeaderTree->make_sorted();
    m_xHeaderTree->set_sort_column(0);
    m_xHeaderTree->set_sort_order(true);
    m_xHeaderTree->set_sort_indicator(TRISTATE_TRUE, 0);

    m_xHeaderTree->select(0);
    m_xHeaderTree->connect_key_release(LINK(this, SwAddStylesDlg_Impl, KeyInput));
}

IMPL_LINK(SwAddStylesDlg_Impl, HeaderBarClick, int, nColumn, void)
{
    bool bSortAtoZ = m_xHeaderTree->get_sort_order();

    //set new arrow positions in headerbar
    if (nColumn == m_xHeaderTree->get_sort_column())
    {
        bSortAtoZ = !bSortAtoZ;
        m_xHeaderTree->set_sort_order(bSortAtoZ);
    }

    if (nColumn != -1)
    {
        //sort lists
        m_xHeaderTree->set_sort_indicator(bSortAtoZ ? TRISTATE_TRUE : TRISTATE_FALSE, nColumn);
    }
}

IMPL_LINK(SwAddStylesDlg_Impl, TreeSizeAllocHdl, const Size&, rSize, void)
{
    auto nWidth = rSize.Width() - Application::GetSettings().GetStyleSettings().GetScrollBarSize();

    std::vector<int> aWidths { 0 };
    int nPadding = m_xHeaderTree->get_approximate_digit_width() * 2;
    for (sal_uInt16 i = 0; i <= MAXLEVEL; ++i)
    {
        OUString sTitle(m_xHeaderTree->get_column_title(i + 1));
        aWidths.push_back(m_xHeaderTree->get_pixel_size(sTitle).Width() + nPadding);
    }
    auto nOtherWidth = std::accumulate(aWidths.begin(), aWidths.end(), 0);
    aWidths[0] = nWidth - nOtherWidth;
    m_xHeaderTree->set_column_fixed_widths(aWidths);
}

IMPL_LINK(SwAddStylesDlg_Impl, RadioToggleOnHdl, const weld::TreeView::iter_col&, rRowCol, void)
{
    for (sal_uInt16 i = 0; i <= MAXLEVEL; ++i)
    {
        TriState eState = rRowCol.second == i + 1 ? TRISTATE_TRUE : TRISTATE_FALSE;
        m_xHeaderTree->set_toggle(rRowCol.first, eState, i + 1);
    }
}

IMPL_LINK(SwAddStylesDlg_Impl, KeyInput, const KeyEvent&, rKEvt, bool)
{
    vcl::KeyCode aCode = rKEvt.GetKeyCode();
    bool bHandled = false;

    sal_uInt16 nCode = aCode.GetCode();
    switch (nCode)
    {
        case KEY_ADD:
            LeftRightHdl(*m_xRightPB);
            bHandled = true;
            break;
        case KEY_SUBTRACT:
            LeftRightHdl(*m_xLeftPB);
            bHandled = true;
            break;
        case KEY_0:
        case KEY_1:
        case KEY_2:
        case KEY_3:
        case KEY_4:
        case KEY_5:
        case KEY_6:
        case KEY_7:
        case KEY_8:
        case KEY_9:
        case KEY_A:
        {
            int nEntry = m_xHeaderTree->get_selected_index();
            if (nEntry != -1)
            {
                ToggleOn(nEntry, nCode != KEY_A ? nCode - KEY_0 : 10);
                bHandled = true;
            }
            break;
        }
    }

    return bHandled;
}

IMPL_LINK_NOARG(SwAddStylesDlg_Impl, OkHdl, weld::Button&, void)
{
    for(sal_uInt16 i = 0; i < MAXLEVEL; i++)
        m_pStyleArr[i].clear();

    int nChildren = m_xHeaderTree->n_children();
    for (int i = 0; i < nChildren; ++i)
    {
        int nToggleColumn = 0;
        for (sal_uInt16 j = 0; j <= MAXLEVEL; ++j)
        {
            if (m_xHeaderTree->get_toggle(i, j + 1) == TRISTATE_TRUE)
            {
                nToggleColumn = j;
                break;
            }
        }
        if (nToggleColumn)
        {
            int nLevel = nToggleColumn - 1;
            if(!m_pStyleArr[nLevel].isEmpty())
                m_pStyleArr[nLevel] += OUStringChar(TOX_STYLE_DELIMITER);
            m_pStyleArr[nLevel] += m_xHeaderTree->get_text(i, 0);
        }
    }

    //TODO write back style names
    m_xDialog->response(RET_OK);
}

IMPL_LINK(SwAddStylesDlg_Impl, LeftRightHdl, weld::Button&, rBtn, void)
{
    bool bLeft = &rBtn == m_xLeftPB.get();
    int nEntry = m_xHeaderTree->get_selected_index();
    if (nEntry == -1)
        return;

    int nToggleColumn = 0;
    for (sal_uInt16 j = 0; j <= MAXLEVEL; ++j)
    {
        if (m_xHeaderTree->get_toggle(nEntry, j + 1) == TRISTATE_TRUE)
        {
            nToggleColumn = j;
            break;
        }
    }

    if (bLeft)
    {
        if (nToggleColumn)
            --nToggleColumn;
    }
    else
    {
        if (nToggleColumn < MAXLEVEL)
            ++nToggleColumn;
    }

    ToggleOn(nEntry, nToggleColumn);
}

void SwAddStylesDlg_Impl::ToggleOn(int nEntry, int nToggleColumn)
{
    for (sal_uInt16 j = 0; j <= MAXLEVEL; ++j)
    {
        m_xHeaderTree->set_toggle(nEntry, j == nToggleColumn ? TRISTATE_TRUE : TRISTATE_FALSE, j + 1);
    }
}

SwTOXSelectTabPage::SwTOXSelectTabPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rAttrSet)
    : SfxTabPage(pPage, pController, "modules/swriter/ui/tocindexpage.ui", "TocIndexPage", &rAttrSet)
    , m_sAutoMarkType(SwResId(STR_AUTOMARK_TYPE))
    , m_bWaitingInitialSettings(true)
    , m_xTitleED(m_xBuilder->weld_entry("title"))
    , m_xTypeFT(m_xBuilder->weld_label("typeft"))
    , m_xTypeLB(m_xBuilder->weld_combo_box("type"))
    , m_xReadOnlyCB(m_xBuilder->weld_check_button("readonly"))
    , m_xAreaFrame(m_xBuilder->weld_widget("areaframe"))
    , m_xAreaLB(m_xBuilder->weld_combo_box("scope"))
    , m_xLevelFT(m_xBuilder->weld_label("levelft"))
    , m_xLevelNF(m_xBuilder->weld_spin_button("level"))
    , m_xCreateFrame(m_xBuilder->weld_widget("createframe"))
    , m_xFromHeadingsCB(m_xBuilder->weld_check_button("fromheadings"))
    , m_xStylesCB(m_xBuilder->weld_check_button("stylescb"))
    , m_xAddStylesCB(m_xBuilder->weld_check_button("addstylescb"))
    , m_xAddStylesPB(m_xBuilder->weld_button("styles"))
    , m_xFromTablesCB(m_xBuilder->weld_check_button("fromtables"))
    , m_xFromFramesCB(m_xBuilder->weld_check_button("fromframes"))
    , m_xFromGraphicsCB(m_xBuilder->weld_check_button("fromgraphics"))
    , m_xFromOLECB(m_xBuilder->weld_check_button("fromoles"))
    , m_xLevelFromChapterCB(m_xBuilder->weld_check_button("uselevel"))
    , m_xFromCaptionsRB(m_xBuilder->weld_radio_button("captions"))
    , m_xFromObjectNamesRB(m_xBuilder->weld_radio_button("objnames"))
    , m_xCaptionSequenceFT(m_xBuilder->weld_label("categoryft"))
    , m_xCaptionSequenceLB(m_xBuilder->weld_combo_box("category"))
    , m_xDisplayTypeFT(m_xBuilder->weld_label("displayft"))
    , m_xDisplayTypeLB(m_xBuilder->weld_combo_box("display"))
    , m_xParaStyleCB(m_xBuilder->weld_check_button("useparastyle"))
    , m_xParaStyleLB(m_xBuilder->weld_combo_box("parastyle"))
    , m_xTOXMarksCB(m_xBuilder->weld_check_button("indexmarks"))
    , m_xIdxOptionsFrame(m_xBuilder->weld_widget("optionsframe"))
    , m_xCollectSameCB(m_xBuilder->weld_check_button("combinesame"))
    , m_xUseFFCB(m_xBuilder->weld_check_button("useff"))
    , m_xUseDashCB(m_xBuilder->weld_check_button("usedash"))
    , m_xCaseSensitiveCB(m_xBuilder->weld_check_button("casesens"))
    , m_xInitialCapsCB(m_xBuilder->weld_check_button("initcaps"))
    , m_xKeyAsEntryCB(m_xBuilder->weld_check_button("keyasentry"))
    , m_xFromFileCB(m_xBuilder->weld_check_button("fromfile"))
    , m_xAutoMarkPB(m_xBuilder->weld_menu_button("file"))
    , m_xFromObjCLB(m_xBuilder->weld_tree_view("objects"))
    , m_xFromObjFrame(m_xBuilder->weld_widget("objectframe"))
    , m_xSequenceCB(m_xBuilder->weld_check_button("numberentries"))
    , m_xBracketLB(m_xBuilder->weld_combo_box("brackets"))
    , m_xAuthorityFrame(m_xBuilder->weld_widget("authframe"))
    , m_xSortFrame(m_xBuilder->weld_widget("sortframe"))
    , m_xLanguageLB(new SvxLanguageBox(m_xBuilder->weld_combo_box("lang")))
    , m_xSortAlgorithmLB(m_xBuilder->weld_combo_box("keytype"))
{
    m_sAddStyleUser = m_xStylesCB->get_label();
    m_pIndexEntryWrapper.reset(new IndexEntrySupplierWrapper());

    m_xLanguageLB->SetLanguageList( SvxLanguageListFlags::ALL | SvxLanguageListFlags::ONLY_KNOWN,
                                 false );

    //Default mode is arranged to be the tallest mode
    //of alphabetical index, lock that size in now
    LanguageHdl(nullptr); //fill sort algorithm list
    Size aPrefSize(m_xContainer->get_preferred_size());
    m_xContainer->set_size_request(aPrefSize.Width(), aPrefSize.Height());

    m_sAddStyleContent = m_xAddStylesCB->get_label();

    m_xFromObjCLB->enable_toggle_buttons(weld::ColumnToggleType::Check);

    for (size_t i = 0; i < SAL_N_ELEMENTS(RES_SRCTYPES); ++i)
    {
        OUString sId(OUString::number(static_cast<sal_uInt32>(RES_SRCTYPES[i].second)));
        m_xFromObjCLB->append();
        m_xFromObjCLB->set_toggle(i, TRISTATE_FALSE);
        m_xFromObjCLB->set_text(i, SwResId(RES_SRCTYPES[i].first), 0);
        m_xFromObjCLB->set_id(i, sId);
    }
    m_xFromObjCLB->set_size_request(-1, std::max<int>(m_xFromObjCLB->get_preferred_size().Height(),
                                                      m_xFromObjCLB->get_height_rows(SAL_N_ELEMENTS(RES_SRCTYPES))) + 2);

    SetExchangeSupport();
    m_xTypeLB->connect_changed(LINK(this, SwTOXSelectTabPage, TOXTypeHdl));

    m_xAddStylesPB->connect_clicked(LINK(this, SwTOXSelectTabPage, AddStylesHdl));

    m_xAutoMarkPB->connect_toggled(LINK(this, SwTOXSelectTabPage, MenuEnableHdl));
    m_xAutoMarkPB->connect_selected(LINK(this, SwTOXSelectTabPage, MenuExecuteHdl));

    Link<weld::Toggleable&,void> aLk =  LINK(this, SwTOXSelectTabPage, CheckBoxHdl);
    m_xAddStylesCB->connect_toggled(aLk);
    m_xFromHeadingsCB->connect_toggled(aLk);
    m_xTOXMarksCB->connect_toggled(aLk);
    m_xFromFileCB->connect_toggled(aLk);
    m_xCollectSameCB->connect_toggled(aLk);
    m_xUseFFCB->connect_toggled(aLk);
    m_xUseDashCB->connect_toggled(aLk);
    m_xInitialCapsCB->connect_toggled(aLk);
    m_xKeyAsEntryCB->connect_toggled(aLk);
    m_xParaStyleCB->connect_toggled(aLk);

    m_xTitleED->connect_changed(LINK(this, SwTOXSelectTabPage, ModifyEntryHdl));
    m_xLevelNF->connect_value_changed(LINK(this, SwTOXSelectTabPage, ModifySpinHdl));
    m_xSortAlgorithmLB->connect_changed(LINK(this, SwTOXSelectTabPage, ModifyListBoxHdl));
    m_xParaStyleLB->connect_changed(LINK(this, SwTOXSelectTabPage, ModifyListBoxHdl));

    aLk = LINK(this, SwTOXSelectTabPage, RadioButtonHdl);
    m_xFromCaptionsRB->connect_toggled(aLk);
    m_xFromObjectNamesRB->connect_toggled(aLk);
    RadioButtonHdl(*m_xFromCaptionsRB);

    m_xLanguageLB->connect_changed(LINK(this, SwTOXSelectTabPage, LanguageListBoxHdl));
    m_xTypeLB->set_active(0);
    m_xTitleED->save_value();
}

SwTOXSelectTabPage::~SwTOXSelectTabPage()
{
    m_pIndexRes.reset();
    m_pIndexEntryWrapper.reset();
    m_xLanguageLB.reset();
}

void SwTOXSelectTabPage::SetWrtShell(SwWrtShell const & rSh)
{
    const sal_uInt16 nUserTypeCount = rSh.GetTOXTypeCount(TOX_USER);
    if(nUserTypeCount <= 1)
        return;

    //insert all new user indexes names after the standard user index
    sal_Int32 nPos = m_xTypeLB->find_id(OUString::number(sal_uInt32(TO_USER))) + 1;
    for (sal_uInt16 nUser = 1; nUser < nUserTypeCount; nUser++)
    {
        sal_uInt32 nEntryData = nUser << 8;
        nEntryData |= TO_USER;
        OUString sId(OUString::number(nEntryData));
        m_xTypeLB->insert(nPos++, rSh.GetTOXType(TOX_USER, nUser)->GetTypeName(),
                          &sId, nullptr, nullptr);
    }
}

bool SwTOXSelectTabPage::FillItemSet( SfxItemSet* )
{
    return true;
}

static tools::Long lcl_TOXTypesToUserData(CurTOXType eType)
{
    sal_uInt16 nRet = TOX_INDEX;
    switch(eType.eType)
    {
        case TOX_INDEX       : nRet = TO_INDEX;     break;
        case TOX_USER        :
        {
            nRet = eType.nIndex << 8;
            nRet |= TO_USER;
        }
        break;
        case TOX_CONTENT     : nRet = TO_CONTENT;   break;
        case TOX_ILLUSTRATIONS:nRet = TO_ILLUSTRATION; break;
        case TOX_OBJECTS     : nRet = TO_OBJECT;    break;
        case TOX_TABLES      : nRet = TO_TABLE;     break;
        case TOX_AUTHORITIES : nRet = TO_AUTHORITIES; break;
        case TOX_BIBLIOGRAPHY : nRet = TO_BIBLIOGRAPHY; break;
        case TOX_CITATION :break;
    }
    return nRet;
}

void SwTOXSelectTabPage::SelectType(TOXTypes eSet)
{
    CurTOXType eCurType (eSet);

    sal_uInt32 nData = lcl_TOXTypesToUserData(eCurType);
    m_xTypeLB->set_active_id(OUString::number(nData));
    m_xTypeFT->set_sensitive(false);
    m_xTypeLB->set_sensitive(false);
    TOXTypeHdl(*m_xTypeLB);
}

static CurTOXType lcl_UserData2TOXTypes(sal_uInt16 nData)
{
    CurTOXType eRet;

    switch(nData&0xff)
    {
        case TO_INDEX       : eRet.eType = TOX_INDEX;       break;
        case TO_USER        :
        {
            eRet.eType = TOX_USER;
            eRet.nIndex  = (nData&0xff00) >> 8;
        }
        break;
        case TO_CONTENT     : eRet.eType = TOX_CONTENT;     break;
        case TO_ILLUSTRATION: eRet.eType = TOX_ILLUSTRATIONS; break;
        case TO_OBJECT      : eRet.eType = TOX_OBJECTS;     break;
        case TO_TABLE       : eRet.eType = TOX_TABLES;      break;
        case TO_AUTHORITIES : eRet.eType = TOX_AUTHORITIES; break;
        case TO_BIBLIOGRAPHY : eRet.eType = TOX_BIBLIOGRAPHY; break;
        default: OSL_FAIL("what a type?");
    }
    return eRet;
}

void SwTOXSelectTabPage::ApplyTOXDescription()
{
    SwMultiTOXTabDialog* pTOXDlg = static_cast<SwMultiTOXTabDialog*>(GetDialogController());
    const CurTOXType aCurType = pTOXDlg->GetCurrentTOXType();
    SwTOXDescription& rDesc = pTOXDlg->GetTOXDescription(aCurType);

    m_xReadOnlyCB->set_active(rDesc.IsReadonly());
    if (!m_xTitleED->get_value_changed_from_saved())
    {
        if (rDesc.GetTitle())
            m_xTitleED->set_text(*rDesc.GetTitle());
        else
            m_xTitleED->set_text(OUString());
        m_xTitleED->save_value();
    }

    m_xAreaLB->set_active(rDesc.IsFromChapter() ? 1 : 0);

    if (aCurType.eType != TOX_INDEX)
        m_xLevelNF->set_value(rDesc.GetLevel());   //content, user

    SwTOXElement nCreateType = rDesc.GetContentOptions();

    //user + content
    bool bHasStyleNames = false;

    for( sal_uInt16 i = 0; i < MAXLEVEL; i++)
        if(!rDesc.GetStyleNames(i).isEmpty())
        {
            bHasStyleNames = true;
            break;
        }
    m_xAddStylesCB->set_active(bHasStyleNames && (nCreateType & SwTOXElement::Template));

    m_xFromOLECB->set_active( bool(nCreateType & SwTOXElement::Ole) );
    m_xFromTablesCB->set_active( bool(nCreateType & SwTOXElement::Table) );
    m_xFromGraphicsCB->set_active( bool(nCreateType & SwTOXElement::Graphic) );
    m_xFromFramesCB->set_active( bool(nCreateType & SwTOXElement::Frame) );

    m_xLevelFromChapterCB->set_active(rDesc.IsLevelFromChapter());

    //all but illustration and table
    m_xTOXMarksCB->set_active( bool(nCreateType & SwTOXElement::Mark) );

    if (TOX_ILLUSTRATIONS == aCurType.eType || TOX_TABLES == aCurType.eType
        || TOX_OBJECTS== aCurType.eType)
    {
        // load all para styles...
        m_xParaStyleLB->clear();
        SwWrtShell const& rWrtSh(static_cast<SwMultiTOXTabDialog*>(GetDialogController())->GetWrtShell());
        const sal_uInt16 nSz = rWrtSh.GetTextFormatCollCount();
        for (sal_uInt16 j = 0; j < nSz; ++j)
        {
            SwTextFormatColl const& rColl = rWrtSh.GetTextFormatColl(j);
            if (rColl.IsDefault())
                continue;

            OUString const name(rColl.GetName());
            if (!name.isEmpty())
            {
                m_xParaStyleLB->append_text(name);
            }
        }
        // first, init ParaStyle - because any later init (e.g. m_xFromCaptionsRB)
        // ends up calling FillTOXDescription() resetting rDesc!
        OUString const& rStyle(rDesc.GetStyleNames(0));
        assert(rStyle.indexOf(TOX_STYLE_DELIMITER) == -1);
        if (rStyle.isEmpty())
        {
            m_xParaStyleCB->set_active(false);
            m_xParaStyleLB->set_sensitive(false);
        }
        else
        {
            m_xParaStyleCB->set_active(true);
            m_xParaStyleLB->set_sensitive(true);
            m_xParaStyleLB->set_active_text(rStyle);
        }
    }

    //content
    if(TOX_CONTENT == aCurType.eType)
    {
        m_xFromHeadingsCB->set_active( bool(nCreateType & SwTOXElement::OutlineLevel) );
        m_xAddStylesCB->set_label(m_sAddStyleContent);
        m_xAddStylesPB->set_sensitive(m_xAddStylesCB->get_active());
    }
    //index only
    else if(TOX_INDEX == aCurType.eType)
    {
        const SwTOIOptions nIndexOptions = rDesc.GetIndexOptions();
        m_xCollectSameCB->set_active( bool(nIndexOptions & SwTOIOptions::SameEntry) );
        m_xUseFFCB->set_active( bool(nIndexOptions & SwTOIOptions::FF) );
        m_xUseDashCB->set_active( bool(nIndexOptions & SwTOIOptions::Dash) );
        if (m_xUseFFCB->get_active())
            m_xUseDashCB->set_sensitive(false);
        else if (m_xUseDashCB->get_active())
            m_xUseFFCB->set_sensitive(false);

        m_xCaseSensitiveCB->set_active( bool(nIndexOptions & SwTOIOptions::CaseSensitive) );
        m_xInitialCapsCB->set_active( bool(nIndexOptions & SwTOIOptions::InitialCaps) );
        m_xKeyAsEntryCB->set_active( bool(nIndexOptions & SwTOIOptions::KeyAsEntry) );
    }
    else if (TOX_ILLUSTRATIONS == aCurType.eType || TOX_TABLES == aCurType.eType)
    {
        OUString sName(rDesc.GetSequenceName());
        int nIndex = m_xCaptionSequenceLB->find_text(sName);
        if (nIndex != -1)
            m_xCaptionSequenceLB->set_active(nIndex);
        m_xDisplayTypeLB->set_active(static_cast<sal_Int32>(rDesc.GetCaptionDisplay()));
        if (m_xDisplayTypeLB->get_active() == -1)
            m_xDisplayTypeLB->set_active(0);
        m_xFromObjectNamesRB->set_active(rDesc.IsCreateFromObjectNames());
        m_xFromCaptionsRB->set_active(!rDesc.IsCreateFromObjectNames());
        RadioButtonHdl(*m_xFromCaptionsRB);
    }
    else if(TOX_OBJECTS == aCurType.eType)
    {
        SwTOOElements nOLEData = rDesc.GetOLEOptions();
        for (int nFromObj = 0, nCount = m_xFromObjCLB->n_children(); nFromObj < nCount; ++nFromObj)
        {
            SwTOOElements nData = static_cast<SwTOOElements>(m_xFromObjCLB->get_id(nFromObj).toInt32());
            m_xFromObjCLB->set_toggle(nFromObj, bool(nData & nOLEData) ? TRISTATE_TRUE : TRISTATE_FALSE);
        }
    }
    else if(TOX_AUTHORITIES == aCurType.eType)
    {
        const OUString& sBrackets(rDesc.GetAuthBrackets());
        if(sBrackets.isEmpty() || sBrackets == "  ")
            m_xBracketLB->set_active(0);
        else
            m_xBracketLB->set_active_text(sBrackets);
        m_xSequenceCB->set_active(rDesc.IsAuthSequence());
    }
    m_xAutoMarkPB->set_sensitive(m_xFromFileCB->get_active());

    for(sal_uInt16 i = 0; i < MAXLEVEL; i++)
        m_aStyleArr[i] = rDesc.GetStyleNames(i);

    m_xLanguageLB->set_active_id(rDesc.GetLanguage());
    LanguageHdl(nullptr);
    for (int nCnt = 0, nEntryCount = m_xSortAlgorithmLB->get_count(); nCnt < nEntryCount; ++nCnt)
    {
        const OUString& rEntryData = m_xSortAlgorithmLB->get_id(nCnt);
        if (rEntryData == rDesc.GetSortAlgorithm())
        {
            m_xSortAlgorithmLB->set_active(nCnt);
            break;
        }
    }
}

void SwTOXSelectTabPage::FillTOXDescription()
{
    SwMultiTOXTabDialog* pTOXDlg = static_cast<SwMultiTOXTabDialog*>(GetDialogController());
    CurTOXType aCurType = pTOXDlg->GetCurrentTOXType();
    SwTOXDescription& rDesc = pTOXDlg->GetTOXDescription(aCurType);
    rDesc.SetTitle(m_xTitleED->get_text());
    rDesc.SetFromChapter(1 == m_xAreaLB->get_active());
    SwTOXElement nContentOptions = SwTOXElement::NONE;
    if (m_xTOXMarksCB->get_visible() && m_xTOXMarksCB->get_active())
        nContentOptions |= SwTOXElement::Mark;

    SwTOIOptions nIndexOptions = rDesc.GetIndexOptions()&SwTOIOptions::AlphaDelimiter;
    switch(rDesc.GetTOXType())
    {
        case TOX_CONTENT:
            if(m_xFromHeadingsCB->get_active())
                nContentOptions |= SwTOXElement::OutlineLevel;
        break;
        case TOX_USER:
        {
            rDesc.SetTOUName(m_xTypeLB->get_active_text());

            if(m_xFromOLECB->get_active())
                nContentOptions |= SwTOXElement::Ole;
            if(m_xFromTablesCB->get_active())
                nContentOptions |= SwTOXElement::Table;
            if(m_xFromFramesCB->get_active())
                nContentOptions |= SwTOXElement::Frame;
            if(m_xFromGraphicsCB->get_active())
                nContentOptions |= SwTOXElement::Graphic;
        }
        break;
        case TOX_INDEX:
        {
            nContentOptions = SwTOXElement::Mark;

            if(m_xCollectSameCB->get_active())
                nIndexOptions |= SwTOIOptions::SameEntry;
            if(m_xUseFFCB->get_active())
                nIndexOptions |= SwTOIOptions::FF;
            if(m_xUseDashCB->get_active())
                nIndexOptions |= SwTOIOptions::Dash;
            if(m_xCaseSensitiveCB->get_active())
                nIndexOptions |= SwTOIOptions::CaseSensitive;
            if(m_xInitialCapsCB->get_active())
                nIndexOptions |= SwTOIOptions::InitialCaps;
            if(m_xKeyAsEntryCB->get_active())
                nIndexOptions |= SwTOIOptions::KeyAsEntry;
            if(m_xFromFileCB->get_active())
                rDesc.SetAutoMarkURL(m_sAutoMarkURL);
            else
                rDesc.SetAutoMarkURL(OUString());
        }
        break;
        case TOX_ILLUSTRATIONS:
        case TOX_TABLES :
            rDesc.SetCreateFromObjectNames(m_xFromObjectNamesRB->get_active());
            rDesc.SetSequenceName(m_xCaptionSequenceLB->get_active_text());
            rDesc.SetCaptionDisplay(static_cast<SwCaptionDisplay>(m_xDisplayTypeLB->get_active()));
            if (m_xParaStyleCB->get_active())
            {
                m_aStyleArr[0] = m_xParaStyleLB->get_active_text();
            }
            else
            {
                m_aStyleArr[0] = OUString();
            }
        break;
        case TOX_OBJECTS:
        {
            SwTOOElements nOLEData = SwTOOElements::NONE;
            for (int i = 0, nCount = m_xFromObjCLB->n_children(); i < nCount; ++i)
            {
                if (m_xFromObjCLB->get_toggle(i) == TRISTATE_TRUE)
                {
                    SwTOOElements nData = static_cast<SwTOOElements>(m_xFromObjCLB->get_id(i).toInt32());
                    nOLEData |= nData;
                }
            }
            rDesc.SetOLEOptions(nOLEData);
            if (m_xParaStyleCB->get_active())
            {
                m_aStyleArr[0] = m_xParaStyleLB->get_active_text();
            }
            else
            {
                m_aStyleArr[0] = OUString();
            }
        }
        break;
        case TOX_AUTHORITIES:
        case TOX_BIBLIOGRAPHY :
        {
            if (m_xBracketLB->get_active())
                rDesc.SetAuthBrackets(m_xBracketLB->get_active_text());
            else
                rDesc.SetAuthBrackets(OUString());
            rDesc.SetAuthSequence(m_xSequenceCB->get_active());
        }
        break;
        case TOX_CITATION :
        break;
    }

    rDesc.SetLevelFromChapter(  m_xLevelFromChapterCB->get_visible() &&
                                m_xLevelFromChapterCB->get_active());
    if (m_xTOXMarksCB->get_active() && m_xTOXMarksCB->get_visible())
        nContentOptions |= SwTOXElement::Mark;
    if (m_xFromHeadingsCB->get_active() && m_xFromHeadingsCB->get_visible())
        nContentOptions |= SwTOXElement::OutlineLevel;
    if ((m_xAddStylesCB->get_active() && m_xAddStylesCB->get_visible())
        || (m_xParaStyleCB->get_active() && m_xParaStyleCB->get_visible()))
    {
        nContentOptions |= SwTOXElement::Template;
    }

    rDesc.SetContentOptions(nContentOptions);
    rDesc.SetIndexOptions(nIndexOptions);
    rDesc.SetLevel(m_xLevelNF->get_value());

    rDesc.SetReadonly(m_xReadOnlyCB->get_active());

    for(sal_uInt16 i = 0; i < MAXLEVEL; i++)
        rDesc.SetStyleNames(m_aStyleArr[i], i);

    rDesc.SetLanguage(m_xLanguageLB->get_active_id());
    const OUString& rEntryData = m_xSortAlgorithmLB->get_active_id();
    rDesc.SetSortAlgorithm(rEntryData);
}

void SwTOXSelectTabPage::Reset( const SfxItemSet* )
{
    SwMultiTOXTabDialog* pTOXDlg = static_cast<SwMultiTOXTabDialog*>(GetDialogController());
    SwWrtShell& rSh = pTOXDlg->GetWrtShell();
    const CurTOXType aCurType = pTOXDlg->GetCurrentTOXType();
    sal_uInt32 nData = lcl_TOXTypesToUserData(aCurType);
    m_xTypeLB->set_active_id(OUString::number(nData));

    m_sAutoMarkURL = INetURLObject::decode( rSh.GetTOIAutoMarkURL(),
                                           INetURLObject::DecodeMechanism::Unambiguous );
    m_xFromFileCB->set_active(!m_sAutoMarkURL.isEmpty());

    m_xCaptionSequenceLB->clear();
    const size_t nCount = rSh.GetFieldTypeCount(SwFieldIds::SetExp);
    for (size_t i = 0; i < nCount; ++i)
    {
        SwFieldType *pType = rSh.GetFieldType( i, SwFieldIds::SetExp );
        if( pType->Which() == SwFieldIds::SetExp &&
            static_cast<SwSetExpFieldType *>( pType)->GetType() & nsSwGetSetExpType::GSE_SEQ )
            m_xCaptionSequenceLB->append_text(pType->GetName());
    }

    if(pTOXDlg->IsTOXEditMode())
    {
        m_xTypeFT->set_sensitive(false);
        m_xTypeLB->set_sensitive(false);
    }

    if(!m_bWaitingInitialSettings)
    {
        // save current values into the proper TOXDescription
        FillTOXDescription();
    }
    m_bWaitingInitialSettings = false;

    TOXTypeHdl(*m_xTypeLB);
    CheckBoxHdl(*m_xAddStylesCB);
}

void SwTOXSelectTabPage::ActivatePage( const SfxItemSet& )
{
    //nothing to do
}

DeactivateRC SwTOXSelectTabPage::DeactivatePage(SfxItemSet* _pSet)
{
    if (_pSet)
        _pSet->Put(SfxUInt16Item(FN_PARAM_TOX_TYPE, m_xTypeLB->get_active_id().toUInt32()));
    FillTOXDescription();
    return DeactivateRC::LeavePage;
}

std::unique_ptr<SfxTabPage> SwTOXSelectTabPage::Create(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rAttrSet)
{
    return std::make_unique<SwTOXSelectTabPage>(pPage, pController, *rAttrSet);
}

IMPL_LINK(SwTOXSelectTabPage, TOXTypeHdl, weld::ComboBox&, rBox, void)
{
    SwMultiTOXTabDialog* pTOXDlg = static_cast<SwMultiTOXTabDialog*>(GetDialogController());
    const sal_uInt16 nType = rBox.get_active_id().toUInt32();
    CurTOXType eCurType = lcl_UserData2TOXTypes(nType);
    pTOXDlg->SetCurrentTOXType(eCurType);

    m_xAreaLB->set_visible( 0 != (nType & (TO_CONTENT|TO_ILLUSTRATION|TO_USER|TO_INDEX|TO_TABLE|TO_OBJECT)) );
    m_xLevelFT->set_visible( 0 != (nType & (TO_CONTENT)) );
    m_xLevelNF->set_visible( 0 != (nType & (TO_CONTENT)) );
    m_xLevelFromChapterCB->set_visible( 0 != (nType & (TO_USER)) );
    m_xAreaFrame->set_visible( 0 != (nType & (TO_CONTENT|TO_ILLUSTRATION|TO_USER|TO_INDEX|TO_TABLE|TO_OBJECT)) );

    m_xFromHeadingsCB->set_visible( 0 != (nType & (TO_CONTENT)) );
    m_xAddStylesCB->set_visible( 0 != (nType & (TO_CONTENT|TO_USER)) );
    m_xAddStylesPB->set_visible( 0 != (nType & (TO_CONTENT|TO_USER)) );

    m_xFromTablesCB->set_visible( 0 != (nType & (TO_USER)) );
    m_xFromFramesCB->set_visible( 0 != (nType & (TO_USER)) );
    m_xFromGraphicsCB->set_visible( 0 != (nType & (TO_USER)) );
    m_xFromOLECB->set_visible( 0 != (nType & (TO_USER)) );

    m_xFromCaptionsRB->set_visible( 0 != (nType & (TO_ILLUSTRATION|TO_TABLE)) );
    m_xFromObjectNamesRB->set_visible( 0 != (nType & (TO_ILLUSTRATION|TO_TABLE)) );

    m_xTOXMarksCB->set_visible( 0 != (nType & (TO_CONTENT|TO_USER)) );

    m_xCreateFrame->set_visible( 0 != (nType & (TO_CONTENT|TO_ILLUSTRATION|TO_USER|TO_TABLE)) );
    m_xCaptionSequenceFT->set_visible( 0 != (nType & (TO_ILLUSTRATION|TO_TABLE)) );
    m_xCaptionSequenceLB->set_visible( 0 != (nType & (TO_ILLUSTRATION|TO_TABLE)) );
    m_xDisplayTypeFT->set_visible( 0 != (nType & (TO_ILLUSTRATION|TO_TABLE)) );
    m_xDisplayTypeLB->set_visible( 0 != (nType & (TO_ILLUSTRATION|TO_TABLE)) );
    m_xParaStyleCB->set_visible(0 != (nType & (TO_ILLUSTRATION|TO_TABLE|TO_OBJECT)));
    m_xParaStyleLB->set_visible(0 != (nType & (TO_ILLUSTRATION|TO_TABLE|TO_OBJECT)));

    m_xAuthorityFrame->set_visible( 0 != (nType & TO_AUTHORITIES) );

    bool bEnableSortLanguage = 0 != (nType & (TO_INDEX|TO_AUTHORITIES));
    m_xSortFrame->set_visible(bEnableSortLanguage);

    if( nType & TO_ILLUSTRATION )
    {
        OUString sName(SwStyleNameMapper::GetUIName(RES_POOLCOLL_LABEL_FIGURE, OUString()));
        m_xCaptionSequenceLB->set_active_text(sName);
    }
    else if( nType & TO_TABLE )
    {
        OUString sName(SwStyleNameMapper::GetUIName(RES_POOLCOLL_LABEL_TABLE, OUString()));
        m_xCaptionSequenceLB->set_active_text(sName);
    }
    else if( nType & TO_USER )
    {
        m_xAddStylesCB->set_label(m_sAddStyleUser);
    }

    m_xIdxOptionsFrame->set_visible( 0 != (nType & TO_INDEX) );

    //object index
    m_xFromObjFrame->set_visible( 0 != (nType & TO_OBJECT) );

    //set control values from the proper TOXDescription
    {
        ApplyTOXDescription();
    }
    ModifyHdl();
}

void SwTOXSelectTabPage::ModifyHdl()
{
    if(!m_bWaitingInitialSettings)
    {
        FillTOXDescription();
        SwMultiTOXTabDialog* pTOXDlg = static_cast<SwMultiTOXTabDialog*>(GetDialogController());
        pTOXDlg->CreateOrUpdateExample(pTOXDlg->GetCurrentTOXType().eType, TOX_PAGE_SELECT);
    }
}

IMPL_LINK_NOARG(SwTOXSelectTabPage, ModifyListBoxHdl, weld::ComboBox&, void)
{
    ModifyHdl();
}

IMPL_LINK_NOARG(SwTOXSelectTabPage, ModifyEntryHdl, weld::Entry&, void)
{
    ModifyHdl();
}

IMPL_LINK_NOARG(SwTOXSelectTabPage, ModifySpinHdl, weld::SpinButton&, void)
{
    ModifyHdl();
}

IMPL_LINK(SwTOXSelectTabPage, CheckBoxHdl, weld::Toggleable&, rButton, void)
{
    SwMultiTOXTabDialog* pTOXDlg = static_cast<SwMultiTOXTabDialog*>(GetDialogController());
    const CurTOXType aCurType = pTOXDlg->GetCurrentTOXType();
    if(TOX_CONTENT == aCurType.eType)
    {
        //at least one of the three CheckBoxes must be checked
        if (!m_xAddStylesCB->get_active() && !m_xFromHeadingsCB->get_active() && !m_xTOXMarksCB->get_active())
        {
            //TODO: InfoBox?
            rButton.set_active(true);
        }
        m_xAddStylesPB->set_sensitive(m_xAddStylesCB->get_active());
    }
    if (TOX_USER == aCurType.eType)
    {
        m_xAddStylesPB->set_sensitive(m_xAddStylesCB->get_active());
    }
    else if (TOX_INDEX == aCurType.eType)
    {
        m_xAutoMarkPB->set_sensitive(m_xFromFileCB->get_active());
        m_xUseFFCB->set_sensitive(m_xCollectSameCB->get_active() && !m_xUseDashCB->get_active());
        m_xUseDashCB->set_sensitive(m_xCollectSameCB->get_active() && !m_xUseFFCB->get_active());
        m_xCaseSensitiveCB->set_sensitive(m_xCollectSameCB->get_active());
    }
    else if (TOX_ILLUSTRATIONS == aCurType.eType
            || TOX_TABLES == aCurType.eType
            || TOX_OBJECTS == aCurType.eType)
    {
        bool const bEnable(m_xParaStyleCB->get_active());
        m_xParaStyleLB->set_sensitive(bEnable);
    }
    ModifyHdl();
};

IMPL_LINK_NOARG(SwTOXSelectTabPage, RadioButtonHdl, weld::Toggleable&, void)
{
    bool bEnable = m_xFromCaptionsRB->get_active();
    m_xCaptionSequenceFT->set_sensitive(bEnable);
    m_xCaptionSequenceLB->set_sensitive(bEnable);
    m_xDisplayTypeFT->set_sensitive(bEnable);
    m_xDisplayTypeLB->set_sensitive(bEnable);
    ModifyHdl();
}

IMPL_LINK(SwTOXSelectTabPage, LanguageListBoxHdl, weld::ComboBox&, rBox, void)
{
    LanguageHdl(&rBox);
}

void SwTOXSelectTabPage::LanguageHdl(const weld::ComboBox* pBox)
{
    lang::Locale aLcl( LanguageTag( m_xLanguageLB->get_active_id() ).getLocale() );
    Sequence< OUString > aSeq = m_pIndexEntryWrapper->GetAlgorithmList( aLcl );

    if( !m_pIndexRes )
        m_pIndexRes.reset(new IndexEntryResource());

    OUString sOldString = m_xSortAlgorithmLB->get_active_id();
    m_xSortAlgorithmLB->clear();

    sal_Int32 nEnd = aSeq.getLength();
    for( sal_Int32 nCnt = 0; nCnt < nEnd; ++nCnt )
    {
        const OUString sAlg(aSeq[ nCnt ]);
        const OUString sUINm = m_pIndexRes->GetTranslation( sAlg );
        m_xSortAlgorithmLB->append(sAlg, sUINm);
        if( sAlg == sOldString )
            m_xSortAlgorithmLB->set_active(nCnt);
    }

    if (m_xSortAlgorithmLB->get_active() == -1)
        m_xSortAlgorithmLB->set_active(0);

    if (pBox)
        ModifyHdl();
};

IMPL_LINK_NOARG(SwTOXSelectTabPage, AddStylesHdl, weld::Button&, void)
{
    SwAddStylesDlg_Impl aDlg(GetFrameWeld(), static_cast<SwMultiTOXTabDialog*>(GetDialogController())->GetWrtShell(),
        m_aStyleArr);
    aDlg.run();
    ModifyHdl();
}

IMPL_LINK_NOARG(SwTOXSelectTabPage, MenuEnableHdl, weld::Toggleable&, void)
{
    m_xAutoMarkPB->set_item_sensitive("edit", !m_sAutoMarkURL.isEmpty());
}

IMPL_LINK(SwTOXSelectTabPage, MenuExecuteHdl, const OUString&, rIdent, void)
{
    const OUString sSaveAutoMarkURL = m_sAutoMarkURL;

    if (rIdent == "open")
    {
        m_sAutoMarkURL = lcl_CreateAutoMarkFileDlg(GetFrameWeld(),
                                m_sAutoMarkURL, m_sAutoMarkType, true);
    }
    else if (rIdent == "new" || rIdent == "edit")
    {
        bool bNew = (rIdent == "new");
        if (bNew)
        {
            m_sAutoMarkURL = lcl_CreateAutoMarkFileDlg(GetFrameWeld(),
                                    m_sAutoMarkURL, m_sAutoMarkType, false);
            if (m_sAutoMarkURL.isEmpty())
                return;
        }

        SwAutoMarkDlg_Impl aAutoMarkDlg(GetFrameWeld(), m_sAutoMarkURL, bNew);
        if (RET_OK != aAutoMarkDlg.run() && bNew)
            m_sAutoMarkURL = sSaveAutoMarkURL;
    }
}

class SwTOXWidget
{
protected:
    Link<SwTOXWidget&,void> m_aGetFocusLink;
public:
    virtual WindowType GetType() const = 0;
    virtual void GrabFocus() = 0;
    virtual void Hide() = 0;
    virtual void set_grid_left_attach(int nPos) = 0;
    virtual void get_extents_relative_to(weld::Widget& rRelative, int& x, int& y, int& width, int& height) = 0;
    void SetGetFocusHdl(const Link<SwTOXWidget&,void>& rLink) { m_aGetFocusLink = rLink; }
    virtual ~SwTOXWidget() {}
};

class SwTOXEdit : public SwTOXWidget
{
    std::unique_ptr<weld::Builder> m_xBuilder;
    SwFormToken           m_aFormToken;
    Link<SwTOXEdit&,void> m_aModifiedLink;
    Link<SwTOXEdit&,void> m_aPrevNextControlLink;
    bool                  m_bNextControl;
    SwTokenWindow*        m_pParent;
    std::unique_ptr<weld::Entry> m_xEntry;

    DECL_LINK(ModifyHdl, weld::Entry&, void);
public:
    SwTOXEdit(SwTokenWindow* pTokenWin, const SwFormToken& rToken)
        : m_xBuilder(Application::CreateBuilder(pTokenWin->get_child_container(), "modules/swriter/ui/toxentrywidget.ui"))
        , m_aFormToken(rToken)
        , m_bNextControl(false)
        , m_pParent(pTokenWin)
        , m_xEntry(m_xBuilder->weld_entry("entry"))
    {
        m_xEntry->connect_changed(LINK(this, SwTOXEdit, ModifyHdl));
        m_xEntry->connect_key_press(LINK(this, SwTOXEdit, KeyInputHdl));
        m_xEntry->connect_focus_in(LINK(this, SwTOXEdit, FocusInHdl));
        m_xEntry->set_tooltip_text(m_pParent->CreateQuickHelp(rToken));
    }

    virtual ~SwTOXEdit() override
    {
        m_pParent->get_child_container()->move(m_xEntry.get(), nullptr);
    }

    virtual WindowType GetType() const override
    {
        return WindowType::EDIT;
    }

    virtual void GrabFocus() override
    {
        m_xEntry->grab_focus();
    }

    virtual void Hide() override
    {
        m_xEntry->hide();
    }

    void Show()
    {
        m_xEntry->show();
    }

    void SetAccessibleName(const OUString& rName)
    {
        m_xEntry->set_accessible_name(rName);
    }

    virtual void set_grid_left_attach(int nPos) override
    {
        m_xEntry->set_grid_left_attach(nPos);
    }

    virtual void get_extents_relative_to(weld::Widget& rRelative, int& x, int& y, int& width, int& height) override
    {
        m_xEntry->get_extents_relative_to(rRelative, x, y, width, height);
    }

    OUString GetText() const
    {
        return m_xEntry->get_text();
    }

    void SetText(const OUString& rText)
    {
        m_xEntry->set_text(rText);
    }

    void get_selection_bounds(int& rStartPos, int& rEndPos)
    {
        m_xEntry->get_selection_bounds(rStartPos, rEndPos);
    }

    void select_region(int nStartPos, int nEndPos)
    {
        m_xEntry->select_region(nStartPos, nEndPos);
    }

    void SetModifyHdl(const Link<SwTOXEdit&,void>& rLink)
    {
        m_aModifiedLink = rLink;
    }

    DECL_LINK(KeyInputHdl, const KeyEvent&, bool);
    DECL_LINK(FocusInHdl, weld::Widget&, void);

    bool    IsNextControl() const { return m_bNextControl; }
    void SetPrevNextLink(const Link<SwTOXEdit&,void>& rLink) { m_aPrevNextControlLink = rLink; }

    const SwFormToken& GetFormToken()
    {
        m_aFormToken.sText = m_xEntry->get_text();
        return m_aFormToken;
    }

    void SetCharStyleName(const OUString& rSet, sal_uInt16 nPoolId)
    {
        m_aFormToken.sCharStyleName = rSet;
        m_aFormToken.nPoolId = nPoolId;
    }

    void AdjustSize();
};

IMPL_LINK_NOARG(SwTOXEdit, ModifyHdl, weld::Entry&, void)
{
    m_aModifiedLink.Call(*this);
}

IMPL_LINK(SwTOXEdit, KeyInputHdl, const KeyEvent&, rKEvt, bool)
{
    bool bCall = false;
    int nStartPos, nEndPos;
    bool bStartIsEnd = !m_xEntry->get_selection_bounds(nStartPos, nEndPos);
    int nMin = std::min(nStartPos, nEndPos);
    const sal_Int32 nTextLen = GetText().getLength();
    if ((bStartIsEnd && !nMin) || nMin == nTextLen)
    {
        vcl::KeyCode aCode = rKEvt.GetKeyCode();
        if (aCode.GetCode() == KEY_RIGHT && nMin == nTextLen)
        {
            m_bNextControl = true;
            bCall = true;
        }
        else if (aCode.GetCode() == KEY_LEFT && !nMin)
        {
            m_bNextControl = false;
            bCall = true;
        }
        else if ( (aCode.GetCode() == KEY_F3) && aCode.IsShift() && !aCode.IsMod1() && !aCode.IsMod2() )
        {
            if (m_pParent)
            {
                m_pParent->SetFocus2theAllBtn();
            }
        }
        if (bCall && m_aPrevNextControlLink.IsSet())
            m_aPrevNextControlLink.Call(*this);
        else
            bCall = false;

    }
    return bCall;
}

IMPL_LINK_NOARG(SwTOXEdit, FocusInHdl, weld::Widget&, void)
{
    m_aGetFocusLink.Call(*this);
}

void SwTOXEdit::AdjustSize()
{
    auto nWidth = m_xEntry->get_pixel_size(GetText()).Width();
    float fChars = nWidth / m_xEntry->get_approximate_digit_width();
    m_xEntry->set_width_chars(std::max(1.0f, std::ceil(fChars)));
}

class SwTOXButton : public SwTOXWidget
{
    std::unique_ptr<weld::Builder> m_xBuilder;
    SwFormToken             m_aFormToken;
    Link<SwTOXButton&,void> m_aPrevNextControlLink;
    bool                    m_bNextControl;
    SwTokenWindow*          m_pParent;
    std::unique_ptr<weld::ToggleButton> m_xButton;
public:
    SwTOXButton(SwTokenWindow* pTokenWin, const SwFormToken& rToken)
        : m_xBuilder(Application::CreateBuilder(pTokenWin->get_child_container(), "modules/swriter/ui/toxbuttonwidget.ui"))
        , m_aFormToken(rToken)
        , m_bNextControl(false)
        , m_pParent(pTokenWin)
        , m_xButton(m_xBuilder->weld_toggle_button("button"))
    {
        m_xButton->connect_key_press(LINK(this, SwTOXButton, KeyInputHdl));
        m_xButton->connect_focus_in(LINK(this, SwTOXButton, FocusInHdl));
        m_xButton->set_tooltip_text(m_pParent->CreateQuickHelp(rToken));
    }

    virtual ~SwTOXButton() override
    {
        m_pParent->get_child_container()->move(m_xButton.get(), nullptr);
    }

    virtual WindowType GetType() const override
    {
        return WindowType::PUSHBUTTON;
    }

    virtual void GrabFocus() override
    {
        m_xButton->grab_focus();
    }

    virtual void Hide() override
    {
        m_xButton->hide();
    }

    void Show()
    {
        m_xButton->show();
    }

    void SetAccessibleName(const OUString& rName)
    {
        m_xButton->set_accessible_name(rName);
    }

    virtual void set_grid_left_attach(int nPos) override
    {
        m_xButton->set_grid_left_attach(nPos);
    }

    void get_extents_relative_to(weld::Widget& rRelative, int& x, int& y, int& width, int& height) override
    {
        m_xButton->get_extents_relative_to(rRelative, x, y, width, height);
    }

    void Check(bool bCheck = true)
    {
        m_xButton->set_active(bCheck);
    }

    DECL_LINK(KeyInputHdl, const KeyEvent&, bool);
    DECL_LINK(FocusInHdl, weld::Widget&, void);

    bool IsNextControl() const          {return m_bNextControl;}
    void SetPrevNextLink(const Link<SwTOXButton&,void>& rLink) {m_aPrevNextControlLink = rLink;}
    const SwFormToken& GetFormToken() const {return m_aFormToken;}

    void SetCharStyleName(const OUString& rSet, sal_uInt16 nPoolId)
        {
            m_aFormToken.sCharStyleName = rSet;
            m_aFormToken.nPoolId = nPoolId;
        }

    void SetTabPosition(SwTwips nSet)
        { m_aFormToken.nTabStopPosition = nSet; }

    void SetFillChar( sal_Unicode cSet )
        { m_aFormToken.cTabFillChar = cSet; }

    void SetTabAlign(SvxTabAdjust eAlign)
         {  m_aFormToken.eTabAlign = eAlign;}

//---> i89791
    //used for entry number format, in TOC only
    //needed for different UI dialog position
    void SetEntryNumberFormat(sal_uInt16 nSet) {
        switch(nSet)
        {
        default:
        case 0:
            m_aFormToken.nChapterFormat = CF_NUMBER;
            break;
        case 1:
            m_aFormToken.nChapterFormat = CF_NUM_NOPREPST_TITLE;
            break;
        }
    }

    void SetChapterInfo(sal_uInt16 nSet) {
        switch(nSet)
        {
        default:
        case 0:
            m_aFormToken.nChapterFormat = CF_NUM_NOPREPST_TITLE;
            break;
        case 1:
            m_aFormToken.nChapterFormat = CF_TITLE;
            break;
        case 2:
            m_aFormToken.nChapterFormat = CF_NUMBER_NOPREPST;
            break;
        }
    }

    void SetOutlineLevel( sal_uInt16 nSet ) { m_aFormToken.nOutlineLevel = nSet;}//i53420

    void SetText(const OUString& rText)
    {
        m_xButton->set_label(rText);
    }

    void SetLinkEnd()
    {
        OSL_ENSURE(TOKEN_LINK_START == m_aFormToken.eTokenType,
                                    "call SetLinkEnd for link start only!");
        m_aFormToken.eTokenType = TOKEN_LINK_END;
        m_aFormToken.sText = SwForm::GetFormLinkEnd();
        SetText(m_aFormToken.sText);
    }

    void SetLinkStart()
    {
        OSL_ENSURE(TOKEN_LINK_END == m_aFormToken.eTokenType,
                                    "call SetLinkStart for link start only!");
        m_aFormToken.eTokenType = TOKEN_LINK_START;
        m_aFormToken.sText = SwForm::GetFormLinkStt();
        SetText(m_aFormToken.sText);
    }
};

IMPL_LINK(SwTOXButton, KeyInputHdl, const KeyEvent&, rKEvt, bool)
{
    bool bCall = false;
    vcl::KeyCode aCode = rKEvt.GetKeyCode();
    if (aCode.GetCode() == KEY_RIGHT)
    {
        m_bNextControl = true;
        bCall = true;
    }
    else if (aCode.GetCode() == KEY_LEFT)
    {
        m_bNextControl = false;
        bCall = true;
    }
    else if (aCode.GetCode() == KEY_DELETE)
    {
        m_pParent->RemoveControl(this, true);
        //this is invalid here
        return true;
    }
    else if ( (aCode.GetCode() == KEY_F3) && aCode.IsShift() && !aCode.IsMod1() && !aCode.IsMod2() )
    {
        if (m_pParent)
        {
            m_pParent->SetFocus2theAllBtn();
        }
    }
    if (bCall && m_aPrevNextControlLink.IsSet())
        m_aPrevNextControlLink.Call(*this);
    else
        bCall = false;
    return bCall;
}

IMPL_LINK_NOARG(SwTOXButton, FocusInHdl, weld::Widget&, void)
{
    m_aGetFocusLink.Call(*this);
}

namespace
{
    const TranslateId STR_AUTH_FIELD_ARY[] =
    {
        STR_AUTH_FIELD_IDENTIFIER,
        STR_AUTH_FIELD_AUTHORITY_TYPE,
        STR_AUTH_FIELD_ADDRESS,
        STR_AUTH_FIELD_ANNOTE,
        STR_AUTH_FIELD_AUTHOR,
        STR_AUTH_FIELD_BOOKTITLE,
        STR_AUTH_FIELD_CHAPTER,
        STR_AUTH_FIELD_EDITION,
        STR_AUTH_FIELD_EDITOR,
        STR_AUTH_FIELD_HOWPUBLISHED,
        STR_AUTH_FIELD_INSTITUTION,
        STR_AUTH_FIELD_JOURNAL,
        STR_AUTH_FIELD_MONTH,
        STR_AUTH_FIELD_NOTE,
        STR_AUTH_FIELD_NUMBER,
        STR_AUTH_FIELD_ORGANIZATIONS,
        STR_AUTH_FIELD_PAGES,
        STR_AUTH_FIELD_PUBLISHER,
        STR_AUTH_FIELD_SCHOOL,
        STR_AUTH_FIELD_SERIES,
        STR_AUTH_FIELD_TITLE,
        STR_AUTH_FIELD_TYPE,
        STR_AUTH_FIELD_VOLUME,
        STR_AUTH_FIELD_YEAR,
        STR_AUTH_FIELD_URL,
        STR_AUTH_FIELD_CUSTOM1,
        STR_AUTH_FIELD_CUSTOM2,
        STR_AUTH_FIELD_CUSTOM3,
        STR_AUTH_FIELD_CUSTOM4,
        STR_AUTH_FIELD_CUSTOM5,
        STR_AUTH_FIELD_ISBN,
        STR_AUTH_FIELD_LOCAL_URL,
        STR_AUTH_FIELD_TARGET_TYPE,
        STR_AUTH_FIELD_TARGET_URL,
    };
}

SwTOXEntryTabPage::SwTOXEntryTabPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rAttrSet)
    : SfxTabPage(pPage, pController, "modules/swriter/ui/tocentriespage.ui", "TocEntriesPage", &rAttrSet)
    , m_sDelimStr(SwResId(STR_DELIM))
    , m_sNoCharStyle(SwResId(STR_NO_CHAR_STYLE))
    , m_pCurrentForm(nullptr)
    , m_bInLevelHdl(false)
    , m_xTypeFT(m_xBuilder->weld_label("typeft"))
    , m_xLevelFT(m_xBuilder->weld_label("levelft"))
    , m_xLevelLB(m_xBuilder->weld_tree_view("level"))
    , m_xAllLevelsPB(m_xBuilder->weld_button("all"))
    , m_xEntryNoPB(m_xBuilder->weld_button("chapterno"))
    , m_xEntryPB(m_xBuilder->weld_button("entrytext"))
    , m_xTabPB(m_xBuilder->weld_button("tabstop"))
    , m_xChapterInfoPB(m_xBuilder->weld_button("chapterinfo"))
    , m_xPageNoPB(m_xBuilder->weld_button("pageno"))
    , m_xHyperLinkPB(m_xBuilder->weld_button("hyperlink"))
    , m_xFieldBox(m_xBuilder->weld_widget("fieldbox"))
    , m_xAuthFieldsLB(m_xBuilder->weld_combo_box("authfield"))
    , m_xAuthInsertPB(m_xBuilder->weld_button("insert"))
    , m_xAuthRemovePB(m_xBuilder->weld_button("remove"))
    , m_xCharStyleLB(m_xBuilder->weld_combo_box("charstyle"))
    , m_xEditStylePB(m_xBuilder->weld_button("edit"))
    , m_xChapterEntryFT(m_xBuilder->weld_label("chapterentryft"))
    , m_xChapterEntryLB(m_xBuilder->weld_combo_box("chapterentry"))
    , m_xNumberFormatFT(m_xBuilder->weld_label("numberformatft"))
    , m_xNumberFormatLB(m_xBuilder->weld_combo_box("numberformat"))
    , m_xEntryOutlineLevelFT(m_xBuilder->weld_label("entryoutlinelevelft"))
    , m_xEntryOutlineLevelNF(m_xBuilder->weld_spin_button("entryoutlinelevel"))
    , m_xFillCharFT(m_xBuilder->weld_label("fillcharft"))
    , m_xFillCharCB(m_xBuilder->weld_combo_box("fillchar"))
    , m_xTabPosFT(m_xBuilder->weld_label("tabstopposft"))
    , m_xTabPosMF(m_xBuilder->weld_metric_spin_button("tabstoppos", FieldUnit::CM))
    , m_xAutoRightCB(m_xBuilder->weld_check_button("alignright"))
    , m_xFormatFrame(m_xBuilder->weld_widget("formatframe"))
    , m_xMainEntryStyleFT(m_xBuilder->weld_label("mainstyleft"))
    , m_xMainEntryStyleLB(m_xBuilder->weld_combo_box("mainstyle"))
    , m_xAlphaDelimCB(m_xBuilder->weld_check_button("alphadelim"))
    , m_xCommaSeparatedCB(m_xBuilder->weld_check_button("commasep"))
    , m_xRelToStyleCB(m_xBuilder->weld_check_button("reltostyle"))
    , m_xSortingFrame(m_xBuilder->weld_widget("sortingframe"))
    , m_xSortDocPosRB(m_xBuilder->weld_radio_button("sortpos"))
    , m_xSortContentRB(m_xBuilder->weld_radio_button("sortcontents"))
    , m_xSortKeyFrame(m_xBuilder->weld_widget("sortkeyframe"))
    , m_xFirstKeyLB(m_xBuilder->weld_combo_box("key1lb"))
    , m_xFirstSortUpRB(m_xBuilder->weld_toggle_button("up1cb"))
    , m_xFirstSortDownRB(m_xBuilder->weld_toggle_button("down1cb"))
    , m_xSecondKeyLB(m_xBuilder->weld_combo_box("key2lb"))
    , m_xSecondSortUpRB(m_xBuilder->weld_toggle_button("up2cb"))
    , m_xSecondSortDownRB(m_xBuilder->weld_toggle_button("down2cb"))
    , m_xThirdKeyLB(m_xBuilder->weld_combo_box("key3lb"))
    , m_xThirdSortUpRB(m_xBuilder->weld_toggle_button("up3cb"))
    , m_xThirdSortDownRB(m_xBuilder->weld_toggle_button("down3cb"))
    , m_xTokenWIN(new SwTokenWindow(m_xBuilder->weld_container("token")))
{
    const OUString sNoCharSortKey(SwResId(STR_NOSORTKEY));

    m_sAuthTypeStr = m_xTypeFT->get_label();
    m_sLevelStr = m_xLevelFT->get_label();
    m_xAuthFieldsLB->make_sorted();
    m_xTokenWIN->SetTabPage(this);

    m_aLastTOXType.eType = TOXTypes(USHRT_MAX);
    m_aLastTOXType.nIndex = 0;

    SetExchangeSupport();
    m_xEntryNoPB->connect_clicked(LINK(this, SwTOXEntryTabPage, InsertTokenHdl));
    m_xEntryPB->connect_clicked(LINK(this, SwTOXEntryTabPage, InsertTokenHdl));
    m_xChapterInfoPB->connect_clicked(LINK(this, SwTOXEntryTabPage, InsertTokenHdl));
    m_xPageNoPB->connect_clicked(LINK(this, SwTOXEntryTabPage, InsertTokenHdl));
    m_xTabPB->connect_clicked(LINK(this, SwTOXEntryTabPage, InsertTokenHdl));
    m_xHyperLinkPB->connect_clicked(LINK(this, SwTOXEntryTabPage, InsertTokenHdl));
    m_xEditStylePB->connect_clicked(LINK(this, SwTOXEntryTabPage, EditStyleHdl));
    m_xLevelLB->connect_changed(LINK(this, SwTOXEntryTabPage, LevelHdl));
    m_xTokenWIN->SetButtonSelectedHdl(LINK(this, SwTOXEntryTabPage, TokenSelectedHdl));
    m_xTokenWIN->SetModifyHdl(LINK(this, SwTOXEntryTabPage, ModifyHdl));
    m_xCharStyleLB->connect_changed(LINK(this, SwTOXEntryTabPage, StyleSelectHdl));
    m_xCharStyleLB->append_text(m_sNoCharStyle);
    m_xChapterEntryLB->connect_changed(LINK(this, SwTOXEntryTabPage, ChapterInfoHdl));
    m_xEntryOutlineLevelNF->connect_value_changed(LINK(this, SwTOXEntryTabPage, ChapterInfoOutlineHdl));
    m_xNumberFormatLB->connect_changed(LINK(this, SwTOXEntryTabPage, NumberFormatHdl));

    m_xTabPosMF->connect_value_changed(LINK(this, SwTOXEntryTabPage, TabPosHdl));
    m_xFillCharCB->connect_changed(LINK(this, SwTOXEntryTabPage, FillCharHdl));
    m_xAutoRightCB->connect_toggled(LINK(this, SwTOXEntryTabPage, AutoRightHdl));
    m_xAuthInsertPB->connect_clicked(LINK(this, SwTOXEntryTabPage, RemoveInsertAuthHdl));
    m_xAuthRemovePB->connect_clicked(LINK(this, SwTOXEntryTabPage, RemoveInsertAuthHdl));
    m_xSortDocPosRB->connect_toggled(LINK(this, SwTOXEntryTabPage, SortKeyHdl));
    m_xSortContentRB->connect_toggled(LINK(this, SwTOXEntryTabPage, SortKeyHdl));
    m_xAllLevelsPB->connect_clicked(LINK(this, SwTOXEntryTabPage, AllLevelsHdl));

    m_xAlphaDelimCB->connect_toggled(LINK(this, SwTOXEntryTabPage, ModifyClickHdl));
    m_xCommaSeparatedCB->connect_toggled(LINK(this, SwTOXEntryTabPage, ModifyClickHdl));
    m_xRelToStyleCB->connect_toggled(LINK(this, SwTOXEntryTabPage, ModifyClickHdl));

    m_xFirstSortUpRB->set_active(true);
    m_xSecondSortUpRB->set_active(true);
    m_xThirdSortUpRB->set_active(true);

    m_xFirstSortUpRB->connect_toggled(LINK(this, SwTOXEntryTabPage, ToggleHdl));
    m_xFirstSortDownRB->connect_toggled(LINK(this, SwTOXEntryTabPage, ToggleHdl));
    m_xSecondSortUpRB->connect_toggled(LINK(this, SwTOXEntryTabPage, ToggleHdl));
    m_xSecondSortDownRB->connect_toggled(LINK(this, SwTOXEntryTabPage, ToggleHdl));
    m_xThirdSortUpRB->connect_toggled(LINK(this, SwTOXEntryTabPage, ToggleHdl));
    m_xThirdSortDownRB->connect_toggled(LINK(this, SwTOXEntryTabPage, ToggleHdl));

    FieldUnit aMetric = ::GetDfltMetric(false);
    ::SetFieldUnit(*m_xTabPosMF, aMetric);

    m_xSortDocPosRB->set_active(true);

    m_xFillCharCB->set_entry_max_length(1);
    m_xFillCharCB->append_text(OUString(' '));
    m_xFillCharCB->append_text(OUString('.'));
    m_xFillCharCB->append_text(OUString('-'));
    m_xFillCharCB->append_text(OUString('_'));
    m_xFillCharCB->append_text(OUString(u'\x2024')); // ONE DOT LEADER
    m_xFillCharCB->append_text(OUString(u'\x2025')); // TWO DOT LEADER
    m_xFillCharCB->append_text(OUString(u'\x2026')); // HORIZONTAL ELLIPSIS

    m_xEditStylePB->set_sensitive(false);

    //fill the types in
    for (sal_uInt16 i = 0; i < AUTH_FIELD_END; ++i)
    {
        OUString sId(OUString::number(i));
        m_xAuthFieldsLB->append(sId, SwResId(STR_AUTH_FIELD_ARY[i]));
    }

    m_xFirstKeyLB->append(OUString::number(USHRT_MAX), sNoCharSortKey);
    m_xSecondKeyLB->append(OUString::number(USHRT_MAX), sNoCharSortKey);
    m_xThirdKeyLB->append(OUString::number(USHRT_MAX), sNoCharSortKey);

    for (sal_uInt16 i = 0; i < AUTH_FIELD_END; ++i)
    {
        const OUString sTmp(m_xAuthFieldsLB->get_text(i));
        const OUString sEntryData(m_xAuthFieldsLB->get_id(i));
        m_xFirstKeyLB->append(sEntryData, sTmp);
        m_xSecondKeyLB->append(sEntryData, sTmp);
        m_xThirdKeyLB->append(sEntryData, sTmp);
    }
    m_xFirstKeyLB->set_active(0);
    m_xSecondKeyLB->set_active(0);
    m_xThirdKeyLB->set_active(0);

    // lock size of dialog. Determine the field box's widest possible
    // configuration (tdf#149186) before doing so.
    int nFieldBoxWidth = 0;
    for (int eType = TOX_CITATION; eType >= TOX_INDEX; --eType)
    {
        ShowHideControls(eType);
        nFieldBoxWidth = std::max<int>(m_xFieldBox->get_preferred_size().Width(), nFieldBoxWidth);
    }
    m_xFieldBox->set_size_request(nFieldBoxWidth, -1);
    Size aPrefSize(m_xContainer->get_preferred_size());
    m_xFieldBox->set_size_request(-1, -1);
    m_xContainer->set_size_request(aPrefSize.Width(), aPrefSize.Height());
}

SwTOXEntryTabPage::~SwTOXEntryTabPage()
{
    m_xTokenWIN.reset();
}

IMPL_LINK_NOARG(SwTOXEntryTabPage, ModifyClickHdl, weld::Toggleable&, void)
{
    OnModify(true);
}

IMPL_LINK_NOARG(SwTOXEntryTabPage, ModifyHdl, LinkParamNone*, void)
{
    OnModify(false);
}

IMPL_LINK(SwTOXEntryTabPage, ToggleHdl, weld::Toggleable&, rToggle, void)
{
    if (&rToggle == m_xFirstSortUpRB.get())
        m_xFirstSortDownRB->set_active(!m_xFirstSortUpRB->get_active());
    else if (&rToggle == m_xFirstSortDownRB.get())
        m_xFirstSortUpRB->set_active(!m_xFirstSortDownRB->get_active());
    else if (&rToggle == m_xSecondSortUpRB.get())
        m_xSecondSortDownRB->set_active(!m_xSecondSortUpRB->get_active());
    else if (&rToggle == m_xSecondSortDownRB.get())
        m_xSecondSortUpRB->set_active(!m_xSecondSortDownRB->get_active());
    else if (&rToggle == m_xThirdSortUpRB.get())
        m_xThirdSortDownRB->set_active(!m_xThirdSortUpRB->get_active());
    else if (&rToggle == m_xThirdSortDownRB.get())
        m_xThirdSortUpRB->set_active(!m_xThirdSortDownRB->get_active());
}

// bAllLevels is used as signal to change all levels of the example
void SwTOXEntryTabPage::OnModify(bool bAllLevels)
{
    UpdateDescriptor();

    SwMultiTOXTabDialog* pTOXDlg = static_cast<SwMultiTOXTabDialog*>(GetDialogController());
    if (pTOXDlg)
    {
        sal_uInt16 nCurLevel = m_xLevelLB->get_selected_index() + 1;
        if (m_aLastTOXType.eType == TOX_CONTENT && bAllLevels)
            nCurLevel = USHRT_MAX;
        pTOXDlg->CreateOrUpdateExample(
            pTOXDlg->GetCurrentTOXType().eType, TOX_PAGE_ENTRY, nCurLevel);
    }
}

bool SwTOXEntryTabPage::FillItemSet( SfxItemSet* )
{
    // nothing to do
    return true;
}

void SwTOXEntryTabPage::Reset( const SfxItemSet* )
{
    SwMultiTOXTabDialog* pTOXDlg = static_cast<SwMultiTOXTabDialog*>(GetDialogController());
    const CurTOXType aCurType = pTOXDlg->GetCurrentTOXType();
    m_pCurrentForm = pTOXDlg->GetForm(aCurType);
    if(TOX_INDEX == aCurType.eType)
    {
        SwTOXDescription& rDesc = pTOXDlg->GetTOXDescription(aCurType);
        const OUString& sMainEntryCharStyle = rDesc.GetMainEntryCharStyle();
        if(!sMainEntryCharStyle.isEmpty())
        {
            if (m_xMainEntryStyleLB->find_text(sMainEntryCharStyle) == -1)
                m_xMainEntryStyleLB->append_text(sMainEntryCharStyle);
            m_xMainEntryStyleLB->set_active_text(sMainEntryCharStyle);
        }
        else
            m_xMainEntryStyleLB->set_active_text(m_sNoCharStyle);
        m_xAlphaDelimCB->set_active( bool(rDesc.GetIndexOptions() & SwTOIOptions::AlphaDelimiter) );
    }
    m_xRelToStyleCB->set_active(m_pCurrentForm->IsRelTabPos());
    m_xCommaSeparatedCB->set_active(m_pCurrentForm->IsCommaSeparated());
}

void SwTOXEntryTabPage::ShowHideControls(int eType)
{
    bool bToxIsAuthorities = TOX_AUTHORITIES == eType;
    bool bToxIsIndex =       TOX_INDEX == eType;
    bool bToxIsContent =     TOX_CONTENT == eType;
    bool bToxSupportsLinks = TOX_CONTENT == eType ||
                             TOX_ILLUSTRATIONS == eType ||
                             TOX_TABLES == eType ||
                             TOX_OBJECTS == eType ||
                             TOX_USER == eType;

    //show or hide controls
    m_xEntryNoPB->set_visible(bToxIsContent);
    m_xHyperLinkPB->set_visible(bToxSupportsLinks);
    m_xRelToStyleCB->set_visible(!bToxIsAuthorities);
    m_xChapterInfoPB->set_visible(!bToxIsContent && !bToxIsAuthorities);
    m_xEntryPB->set_visible(!bToxIsAuthorities);
    m_xPageNoPB->set_visible(!bToxIsAuthorities);
    m_xAuthFieldsLB->set_visible(bToxIsAuthorities);
    m_xAuthInsertPB->set_visible(bToxIsAuthorities);
    m_xAuthRemovePB->set_visible(bToxIsAuthorities);

    m_xFormatFrame->set_visible(!bToxIsAuthorities);

    m_xSortingFrame->set_visible(bToxIsAuthorities);
    m_xSortKeyFrame->set_visible(bToxIsAuthorities);

    m_xMainEntryStyleFT->set_visible(bToxIsIndex);
    m_xMainEntryStyleLB->set_visible(bToxIsIndex);
    m_xAlphaDelimCB->set_visible(bToxIsIndex);
    m_xCommaSeparatedCB->set_visible(bToxIsIndex);
}

void SwTOXEntryTabPage::ActivatePage( const SfxItemSet& /*rSet*/)
{
    SwMultiTOXTabDialog* pTOXDlg = static_cast<SwMultiTOXTabDialog*>(GetDialogController());
    const CurTOXType aCurType = pTOXDlg->GetCurrentTOXType();

    m_pCurrentForm = pTOXDlg->GetForm(aCurType);
    if( !( m_aLastTOXType == aCurType ))
    {
        bool bToxIsAuthorities = TOX_AUTHORITIES == aCurType.eType;
        bool bToxIsIndex =       TOX_INDEX == aCurType.eType;

        m_xLevelLB->clear();
        for(sal_uInt16 i = 1; i < m_pCurrentForm->GetFormMax(); i++)
        {
            if(bToxIsAuthorities)
                m_xLevelLB->append_text( SwAuthorityFieldType::GetAuthTypeName(
                                            static_cast<ToxAuthorityType>(i - 1)) );
            else if( bToxIsIndex )
            {
                if(i == 1)
                    m_xLevelLB->append_text( m_sDelimStr );
                else
                    m_xLevelLB->append_text( OUString::number(i - 1) );
            }
            else
                m_xLevelLB->append_text(OUString::number(i));
        }
        if(bToxIsAuthorities)
        {
            SwWrtShell& rSh = pTOXDlg->GetWrtShell();
            const SwAuthorityFieldType* pFType = static_cast<const SwAuthorityFieldType*>(
                                    rSh.GetFieldType(SwFieldIds::TableOfAuthorities, OUString()));
            if(pFType)
            {
                if(pFType->IsSortByDocument())
                    m_xSortDocPosRB->set_active(true);
                else
                {
                    m_xSortContentRB->set_active(true);
                    const sal_uInt16 nKeyCount = pFType->GetSortKeyCount();
                    if(0 < nKeyCount)
                    {
                        const SwTOXSortKey* pKey = pFType->GetSortKey(0);
                        m_xFirstKeyLB->set_active_id(OUString::number(pKey->eField));
                        m_xFirstSortUpRB->set_active(pKey->bSortAscending);
                        m_xFirstSortDownRB->set_active(!pKey->bSortAscending);
                    }
                    if(1 < nKeyCount)
                    {
                        const SwTOXSortKey* pKey = pFType->GetSortKey(1);
                        m_xSecondKeyLB->set_active_id(OUString::number(pKey->eField));
                        m_xSecondSortUpRB->set_active(pKey->bSortAscending);
                        m_xSecondSortDownRB->set_active(!pKey->bSortAscending);
                    }
                    if(2 < nKeyCount)
                    {
                        const SwTOXSortKey* pKey = pFType->GetSortKey(2);
                        m_xThirdKeyLB->set_active_id(OUString::number(pKey->eField));
                        m_xThirdSortUpRB->set_active(pKey->bSortAscending);
                        m_xThirdSortDownRB->set_active(!pKey->bSortAscending);
                    }
                }
            }
            SortKeyHdl(m_xSortDocPosRB->get_active() ? *m_xSortDocPosRB : *m_xSortContentRB);
            m_xLevelFT->set_label(m_sAuthTypeStr);
        }
        else
            m_xLevelFT->set_label(m_sLevelStr);

        m_xLevelLB->select(bToxIsIndex ? 1 : 0);

        //show or hide controls
        ShowHideControls(aCurType.eType);
    }
    m_aLastTOXType = aCurType;

    //invalidate PatternWindow
    m_xTokenWIN->SetInvalid();
    LevelHdl(*m_xLevelLB);
}

void SwTOXEntryTabPage::UpdateDescriptor()
{
    WriteBackLevel();
    SwMultiTOXTabDialog* pTOXDlg = static_cast<SwMultiTOXTabDialog*>(GetDialogController());
    SwTOXDescription& rDesc = pTOXDlg->GetTOXDescription(m_aLastTOXType);
    if(TOX_INDEX == m_aLastTOXType.eType)
    {
        const OUString sTemp(m_xMainEntryStyleLB->get_active_text());
        rDesc.SetMainEntryCharStyle(m_sNoCharStyle == sTemp ? OUString(): sTemp);
        SwTOIOptions nIdxOptions = rDesc.GetIndexOptions() & ~SwTOIOptions::AlphaDelimiter;
        if (m_xAlphaDelimCB->get_active())
            nIdxOptions |= SwTOIOptions::AlphaDelimiter;
        rDesc.SetIndexOptions(nIdxOptions);
    }
    else if (TOX_AUTHORITIES == m_aLastTOXType.eType)
    {
        rDesc.SetSortByDocument(m_xSortDocPosRB->get_active());
        SwTOXSortKey aKey1, aKey2, aKey3;
        aKey1.eField = static_cast<ToxAuthorityField>(m_xFirstKeyLB->get_active_id().toInt32());
        aKey1.bSortAscending = m_xFirstSortUpRB->get_active();
        aKey2.eField = static_cast<ToxAuthorityField>(m_xSecondKeyLB->get_active_id().toInt32());
        aKey2.bSortAscending = m_xSecondSortUpRB->get_active();
        aKey3.eField = static_cast<ToxAuthorityField>(m_xThirdKeyLB->get_active_id().toInt32());
        aKey3.bSortAscending = m_xThirdSortUpRB->get_active();

        rDesc.SetSortKeys(aKey1, aKey2, aKey3);
    }
    SwForm* pCurrentForm = pTOXDlg->GetForm(m_aLastTOXType);
    if (m_xRelToStyleCB->get_visible())
        pCurrentForm->SetRelTabPos(m_xRelToStyleCB->get_active());
    if (m_xCommaSeparatedCB->get_visible())
        pCurrentForm->SetCommaSeparated(m_xCommaSeparatedCB->get_active());
}

DeactivateRC SwTOXEntryTabPage::DeactivatePage( SfxItemSet* /*pSet*/)
{
    UpdateDescriptor();
    return DeactivateRC::LeavePage;
}

std::unique_ptr<SfxTabPage> SwTOXEntryTabPage::Create(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rAttrSet)
{
    return std::make_unique<SwTOXEntryTabPage>(pPage, pController, *rAttrSet);
}

IMPL_LINK_NOARG(SwTOXEntryTabPage, EditStyleHdl, weld::Button&, void)
{
    if (m_xCharStyleLB->get_active() != -1)
    {
        SfxStringItem aStyle(SID_STYLE_EDIT, m_xCharStyleLB->get_active_text());
        SfxUInt16Item aFamily(SID_STYLE_FAMILY, sal_uInt16(SfxStyleFamily::Char));
        static_cast<SwMultiTOXTabDialog*>(GetDialogController())->GetWrtShell().
        GetView().GetViewFrame().GetDispatcher()->ExecuteList(SID_STYLE_EDIT,
                SfxCallMode::SYNCHRON,
                { &aStyle, &aFamily });
    }
}

IMPL_LINK(SwTOXEntryTabPage, RemoveInsertAuthHdl, weld::Button&, rButton, void)
{
    bool bInsert = &rButton == m_xAuthInsertPB.get();
    if(bInsert)
    {
        sal_Int32 nSelPos = m_xAuthFieldsLB->get_active();
        const OUString sToInsert(m_xAuthFieldsLB->get_active_text());
        SwFormToken aInsert(TOKEN_AUTHORITY);
        aInsert.nAuthorityField = m_xAuthFieldsLB->get_id(nSelPos).toUInt32();
        m_xTokenWIN->InsertAtSelection(aInsert);
        m_xAuthFieldsLB->remove_text(sToInsert);
        m_xAuthFieldsLB->set_active(nSelPos ? nSelPos - 1 : 0);
    }
    else
    {
        SwTOXWidget* pCtrl = m_xTokenWIN->GetActiveControl();
        OSL_ENSURE(WindowType::EDIT != pCtrl->GetType(), "Remove should be disabled");
        if (WindowType::EDIT != pCtrl->GetType())
        {
            //fill it into the ListBox
            const SwFormToken& rToken = static_cast<SwTOXButton*>(pCtrl)->GetFormToken();
            PreTokenButtonRemoved(rToken);
            m_xTokenWIN->RemoveControl(static_cast<SwTOXButton*>(pCtrl));
        }
    }
    ModifyHdl(nullptr);
}

void SwTOXEntryTabPage::PreTokenButtonRemoved(const SwFormToken& rToken)
{
    //fill it into the ListBox
    sal_uInt32 nData = rToken.nAuthorityField;
    m_xAuthFieldsLB->append(OUString::number(nData), SwResId(STR_AUTH_FIELD_ARY[nData]));
}

void SwTOXEntryTabPage::SetFocus2theAllBtn()
{
    m_xAllLevelsPB->grab_focus();
}

// This function initializes the default value in the Token
// put here the UI dependent initializations
IMPL_LINK(SwTOXEntryTabPage, InsertTokenHdl, weld::Button&, rBtn, void)
{
    FormTokenType eTokenType = TOKEN_ENTRY_NO;
    OUString sCharStyle;
    sal_uInt16  nChapterFormat = CF_NUMBER; // i89791
    if (&rBtn == m_xEntryNoPB.get())
    {
        eTokenType = TOKEN_ENTRY_NO;
    }
    else if (&rBtn == m_xEntryPB.get())
    {
        if( TOX_CONTENT == m_pCurrentForm->GetTOXType() )
        {
            eTokenType = TOKEN_ENTRY_TEXT;
        }
        else
        {
            eTokenType = TOKEN_ENTRY;
        }
    }
    else if (&rBtn == m_xChapterInfoPB.get())
    {
        eTokenType = TOKEN_CHAPTER_INFO;
        nChapterFormat = CF_NUM_NOPREPST_TITLE; // i89791
    }
    else if (&rBtn == m_xPageNoPB.get())
    {
        eTokenType = TOKEN_PAGE_NUMS;
    }
    else if (&rBtn == m_xHyperLinkPB.get())
    {
        eTokenType = TOKEN_LINK_START;
        sCharStyle = SwResId(STR_POOLCHR_TOXJUMP);
    }
    else if (&rBtn == m_xTabPB.get())
    {
        eTokenType = TOKEN_TAB_STOP;
    }
    SwFormToken aInsert(eTokenType);
    aInsert.sCharStyleName = sCharStyle;
    aInsert.nTabStopPosition = 0;
    aInsert.nChapterFormat = nChapterFormat; // i89791
    m_xTokenWIN->InsertAtSelection(aInsert);
    ModifyHdl(nullptr);
}

IMPL_LINK_NOARG(SwTOXEntryTabPage, AllLevelsHdl, weld::Button&, void)
{
    //get current level
    //write it into all levels
    if(m_xTokenWIN->IsValid())
    {
        const OUString sNewToken = m_xTokenWIN->GetPattern();
        for(sal_uInt16 i = 1; i < m_pCurrentForm->GetFormMax(); i++)
            m_pCurrentForm->SetPattern(i, sNewToken);

        OnModify(true);
    }
}

void SwTOXEntryTabPage::WriteBackLevel()
{
    if(m_xTokenWIN->IsValid())
    {
        const OUString sNewToken = m_xTokenWIN->GetPattern();
        const sal_uInt16 nLastLevel = m_xTokenWIN->GetLastLevel();
        if(nLastLevel != USHRT_MAX)
            m_pCurrentForm->SetPattern(nLastLevel + 1, sNewToken);
    }
}

IMPL_LINK(SwTOXEntryTabPage, LevelHdl, weld::TreeView&, rBox, void)
{
    if(m_bInLevelHdl)
        return;
    m_bInLevelHdl = true;
    WriteBackLevel();

    const sal_uInt16 nLevel = rBox.get_selected_index();
    m_xTokenWIN->SetForm(*m_pCurrentForm, nLevel);
    if(TOX_AUTHORITIES == m_pCurrentForm->GetTOXType())
    {
        //fill the types in
        m_xAuthFieldsLB->clear();
        for( sal_uInt32 i = 0; i < AUTH_FIELD_END; i++)
        {
            m_xAuthFieldsLB->append(OUString::number(i), SwResId(STR_AUTH_FIELD_ARY[i]));
        }

        // #i21237#
        SwFormTokens aPattern = m_pCurrentForm->GetPattern(nLevel + 1);

        for(const auto& aToken : aPattern)
        {
            if(TOKEN_AUTHORITY == aToken.eTokenType)
            {
                sal_uInt32 nSearch = aToken.nAuthorityField;
                int nLstBoxPos = m_xAuthFieldsLB->find_id(OUString::number(nSearch));
                OSL_ENSURE(nLstBoxPos != -1, "Entry not found?");
                m_xAuthFieldsLB->remove(nLstBoxPos);
            }
        }
        m_xAuthFieldsLB->set_active(0);
    }
    m_bInLevelHdl = false;
    rBox.grab_focus();
}

IMPL_LINK_NOARG(SwTOXEntryTabPage, SortKeyHdl, weld::Toggleable&, void)
{
    bool bEnable = m_xSortContentRB->get_active();
    m_xSortKeyFrame->set_sensitive(bEnable);
}

IMPL_LINK(SwTOXEntryTabPage, TokenSelectedHdl, SwFormToken&, rToken, void)
{
    if (!rToken.sCharStyleName.isEmpty())
        m_xCharStyleLB->set_active_text(rToken.sCharStyleName);
    else
        m_xCharStyleLB->set_active_text(m_sNoCharStyle);

    const OUString sEntry = m_xCharStyleLB->get_active_text();
    m_xEditStylePB->set_sensitive(sEntry != m_sNoCharStyle);

    if(rToken.eTokenType == TOKEN_CHAPTER_INFO)
    {
//---> i89791
        switch(rToken.nChapterFormat)
        {
        default:
            m_xChapterEntryLB->set_active(-1);//to alert the user
            break;
        case CF_NUM_NOPREPST_TITLE:
            m_xChapterEntryLB->set_active(0);
            break;
        case CF_TITLE:
            m_xChapterEntryLB->set_active(1);
           break;
        case CF_NUMBER_NOPREPST:
            m_xChapterEntryLB->set_active(2);
            break;
        }
//i53420

        m_xEntryOutlineLevelNF->set_value(rToken.nOutlineLevel);
    }

//i53420
    if(rToken.eTokenType == TOKEN_ENTRY_NO)
    {
        m_xEntryOutlineLevelNF->set_value(rToken.nOutlineLevel);
        const sal_uInt16 nFormat =
            rToken.nChapterFormat == CF_NUM_NOPREPST_TITLE ? 1 : 0;
        m_xNumberFormatLB->set_active(nFormat);
    }

    bool bTabStop = TOKEN_TAB_STOP == rToken.eTokenType;
    m_xFillCharFT->set_visible(bTabStop);
    m_xFillCharCB->set_visible(bTabStop);
    m_xTabPosFT->set_visible(bTabStop);
    m_xTabPosMF->set_visible(bTabStop);
    m_xAutoRightCB->set_visible(bTabStop);
    m_xAutoRightCB->set_sensitive(bTabStop);
    if(bTabStop)
    {
        m_xTabPosMF->set_value(m_xTabPosMF->normalize(rToken.nTabStopPosition), FieldUnit::TWIP);
        m_xAutoRightCB->set_active(SvxTabAdjust::End == rToken.eTabAlign);
        m_xFillCharCB->set_entry_text(OUString(rToken.cTabFillChar));
        m_xTabPosFT->set_sensitive(!m_xAutoRightCB->get_active());
        m_xTabPosMF->set_sensitive(!m_xAutoRightCB->get_active());
    }
    else
    {
        m_xTabPosMF->set_sensitive(false);
    }

    bool bIsChapterInfo = rToken.eTokenType == TOKEN_CHAPTER_INFO;
    bool bIsEntryNumber = rToken.eTokenType == TOKEN_ENTRY_NO;
    m_xChapterEntryFT->set_visible( bIsChapterInfo );
    m_xChapterEntryLB->set_visible( bIsChapterInfo );
    m_xEntryOutlineLevelFT->set_visible( bIsChapterInfo || bIsEntryNumber );
    m_xEntryOutlineLevelNF->set_visible( bIsChapterInfo || bIsEntryNumber );
    m_xNumberFormatFT->set_visible( bIsEntryNumber );
    m_xNumberFormatLB->set_visible( bIsEntryNumber );

    //now enable the visible buttons
    //- inserting the same type of control is not allowed
    //- some types of controls can only appear once (EntryText EntryNumber)

    if (m_xEntryNoPB->get_visible())
    {
        m_xEntryNoPB->set_sensitive(TOKEN_ENTRY_NO != rToken.eTokenType );
    }
    if (m_xEntryPB->get_visible())
    {
        m_xEntryPB->set_sensitive(TOKEN_ENTRY_TEXT != rToken.eTokenType &&
                                  !m_xTokenWIN->Contains(TOKEN_ENTRY_TEXT)
                                  && !m_xTokenWIN->Contains(TOKEN_ENTRY));
    }

    if (m_xChapterInfoPB->get_visible())
    {
        m_xChapterInfoPB->set_sensitive(TOKEN_CHAPTER_INFO != rToken.eTokenType);
    }
    if (m_xPageNoPB->get_visible())
    {
        m_xPageNoPB->set_sensitive(TOKEN_PAGE_NUMS != rToken.eTokenType &&
                                   !m_xTokenWIN->Contains(TOKEN_PAGE_NUMS));
    }
    if (m_xTabPB->get_visible())
    {
        m_xTabPB->set_sensitive(!bTabStop);
    }
    if (m_xHyperLinkPB->get_visible())
    {
        m_xHyperLinkPB->set_sensitive(TOKEN_LINK_START != rToken.eTokenType &&
                                      TOKEN_LINK_END != rToken.eTokenType);
    }
    //table of authorities
    if (m_xAuthInsertPB->get_visible())
    {
        bool bText = TOKEN_TEXT == rToken.eTokenType;
        m_xAuthInsertPB->set_sensitive(bText && !m_xAuthFieldsLB->get_active_text().isEmpty());
        m_xAuthRemovePB->set_sensitive(!bText);
    }
}

IMPL_LINK(SwTOXEntryTabPage, StyleSelectHdl, weld::ComboBox&, rBox, void)
{
    OUString sEntry = rBox.get_active_text();
    const sal_uInt16 nId = rBox.get_active_id().toUInt32();
    const bool bEqualsNoCharStyle = sEntry == m_sNoCharStyle;
    m_xEditStylePB->set_sensitive(!bEqualsNoCharStyle);
    if (bEqualsNoCharStyle)
        sEntry.clear();
    SwTOXWidget* pCtrl = m_xTokenWIN->GetActiveControl();
    OSL_ENSURE(pCtrl, "no active control?");
    if(pCtrl)
    {
        if(WindowType::EDIT == pCtrl->GetType())
            static_cast<SwTOXEdit*>(pCtrl)->SetCharStyleName(sEntry, nId);
        else
            static_cast<SwTOXButton*>(pCtrl)->SetCharStyleName(sEntry, nId);

    }
    ModifyHdl(nullptr);
}

IMPL_LINK(SwTOXEntryTabPage, ChapterInfoHdl, weld::ComboBox&, rBox, void)
{
    int nPos = rBox.get_active();
    if (nPos != -1)
    {
        SwTOXWidget* pCtrl = m_xTokenWIN->GetActiveControl();
        OSL_ENSURE(pCtrl, "no active control?");
        if(pCtrl && WindowType::EDIT != pCtrl->GetType())
            static_cast<SwTOXButton*>(pCtrl)->SetChapterInfo(nPos);
        ModifyHdl(nullptr);
    }
}

IMPL_LINK(SwTOXEntryTabPage, ChapterInfoOutlineHdl, weld::SpinButton&, rEdit, void)
{
    const sal_uInt16 nLevel = rEdit.get_value();

    SwTOXWidget* pCtrl = m_xTokenWIN->GetActiveControl();
    OSL_ENSURE(pCtrl, "no active control?");
    if(pCtrl && WindowType::EDIT != pCtrl->GetType())
        static_cast<SwTOXButton*>(pCtrl)->SetOutlineLevel(nLevel);

    ModifyHdl(nullptr);
}

IMPL_LINK(SwTOXEntryTabPage, NumberFormatHdl, weld::ComboBox&, rBox, void)
{
    const sal_Int32 nPos = rBox.get_active();
    if (nPos != -1)
    {
        SwTOXWidget* pCtrl = m_xTokenWIN->GetActiveControl();
        OSL_ENSURE(pCtrl, "no active control?");
        if(pCtrl && WindowType::EDIT != pCtrl->GetType())
        {
           static_cast<SwTOXButton*>(pCtrl)->SetEntryNumberFormat(nPos);//i89791
        }
        ModifyHdl(nullptr);
    }
}

IMPL_LINK(SwTOXEntryTabPage, TabPosHdl, weld::MetricSpinButton&, rEdit, void)
{
    SwTOXWidget* pCtrl = m_xTokenWIN->GetActiveControl();
    OSL_ENSURE(pCtrl && WindowType::EDIT != pCtrl->GetType() &&
        TOKEN_TAB_STOP == static_cast<SwTOXButton*>(pCtrl)->GetFormToken().eTokenType,
                "no active style::TabStop control?");
    if( pCtrl && WindowType::EDIT != pCtrl->GetType() )
    {
        static_cast<SwTOXButton*>(pCtrl)->SetTabPosition( static_cast< SwTwips >(
                rEdit.denormalize(rEdit.get_value(FieldUnit::TWIP))));
    }
    ModifyHdl(nullptr);
}

IMPL_LINK(SwTOXEntryTabPage, FillCharHdl, weld::ComboBox&, rBox, void)
{
    SwTOXWidget* pCtrl = m_xTokenWIN->GetActiveControl();
    OSL_ENSURE(pCtrl && WindowType::EDIT != pCtrl->GetType() &&
        TOKEN_TAB_STOP == static_cast<SwTOXButton*>(pCtrl)->GetFormToken().eTokenType,
                "no active style::TabStop control?");
    if (pCtrl && WindowType::EDIT != pCtrl->GetType())
    {
        sal_Unicode cSet;
        if (!rBox.get_active_text().isEmpty())
            cSet = rBox.get_active_text()[0];
        else
            cSet = ' ';
        static_cast<SwTOXButton*>(pCtrl)->SetFillChar( cSet );
    }
    ModifyHdl(nullptr);
}

IMPL_LINK(SwTOXEntryTabPage, AutoRightHdl, weld::Toggleable&, rBox, void)
{
    //the most right style::TabStop is usually right aligned
    SwTOXWidget* pCurCtrl = m_xTokenWIN->GetActiveControl();
    OSL_ENSURE(WindowType::EDIT != pCurCtrl->GetType() &&
            static_cast<SwTOXButton*>(pCurCtrl)->GetFormToken().eTokenType == TOKEN_TAB_STOP,
            "no style::TabStop selected!");

    const SwFormToken& rToken = static_cast<SwTOXButton*>(pCurCtrl)->GetFormToken();
    bool bChecked = rBox.get_active();
    if(rToken.eTokenType == TOKEN_TAB_STOP)
        static_cast<SwTOXButton*>(pCurCtrl)->SetTabAlign(
            bChecked ? SvxTabAdjust::End : SvxTabAdjust::Left);
    m_xTabPosFT->set_sensitive(!bChecked);
    m_xTabPosMF->set_sensitive(!bChecked);
    ModifyHdl(nullptr);
}

void SwTOXEntryTabPage::SetWrtShell(SwWrtShell& rSh)
{
    SwDocShell* pDocSh = rSh.GetView().GetDocShell();
    ::FillCharStyleListBox(*m_xCharStyleLB, pDocSh, true, true);
    const OUString sDefault(SwResId(STR_POOLCHR_STANDARD));
    for (int i = 0, nCount = m_xCharStyleLB->get_count(); i < nCount; ++i)
    {
        const OUString sEntry = m_xCharStyleLB->get_text(i);
        if(sDefault != sEntry)
        {
            m_xMainEntryStyleLB->append(m_xCharStyleLB->get_id(i), sEntry);
        }
    }
    m_xMainEntryStyleLB->set_active_text(SwStyleNameMapper::GetUIName(
                                           RES_POOLCHR_IDX_MAIN_ENTRY, OUString()));
}

 const TranslateId STR_TOKEN_ARY[] =
{
    STR_TOKEN_ENTRY_NO,
    STR_TOKEN_ENTRY,    //mapped from original STR_TOKEN_ENTRY_TEXT,
    STR_TOKEN_ENTRY,
    STR_TOKEN_TAB_STOP,
    {},
    STR_TOKEN_PAGE_NUMS,
    STR_TOKEN_CHAPTER_INFO,
    STR_TOKEN_LINK_START,
    STR_TOKEN_LINK_END,
    STR_TOKEN_AUTHORITY
};

 const TranslateId STR_TOKEN_HELP_ARY[] =
{
    STR_TOKEN_HELP_ENTRY_NO,
    STR_TOKEN_HELP_ENTRY,   // mapped from original STR_TOKEN_HELP_ENTRY_TEXT,
    STR_TOKEN_HELP_ENTRY,
    STR_TOKEN_HELP_TAB_STOP,
    STR_TOKEN_HELP_TEXT,
    STR_TOKEN_HELP_PAGE_NUMS,
    STR_TOKEN_HELP_CHAPTER_INFO,
    STR_TOKEN_HELP_LINK_START,
    STR_TOKEN_HELP_LINK_END,
    STR_TOKEN_HELP_AUTHORITY
};

SwTokenWindow::SwTokenWindow(std::unique_ptr<weld::Container> xParent)
    : m_pForm(nullptr)
    , m_nLevel(0)
    , m_bValid(false)
    , m_sCharStyle(SwResId(STR_CHARSTYLE))
    , m_pActiveCtrl(nullptr)
    , m_aAdjustPositionsIdle("SwTokenWindow m_aAdjustPositionsIdle")
    , m_pParent(nullptr)
    , m_xParentWidget(std::move(xParent))
    , m_xBuilder(Application::CreateBuilder(m_xParentWidget.get(), "modules/swriter/ui/tokenwidget.ui"))
    , m_xContainer(m_xBuilder->weld_container("TokenWidget"))
    , m_xLeftScrollWin(m_xBuilder->weld_button("left"))
    , m_xCtrlParentWin(m_xBuilder->weld_container("ctrl"))
    , m_xScrollWin(m_xBuilder->weld_scrolled_window("scrollwin"))
    , m_xRightScrollWin(m_xBuilder->weld_button("right"))
{
    m_xScrollWin->connect_hadjustment_changed(LINK(this, SwTokenWindow, ScrollHdl));
    m_xCtrlParentWin->connect_size_allocate(LINK(this, SwTokenWindow, AdjustPositionsHdl));

    for (sal_uInt32 i = 0; i < TOKEN_END; ++i)
    {
        TranslateId pTextId = STR_TOKEN_ARY[i];
        if (pTextId)
            m_aButtonTexts[i] = SwResId(pTextId);

        TranslateId pHelpId = STR_TOKEN_HELP_ARY[i];
        m_aButtonHelpTexts[i] = SwResId(pHelpId);
    }

    m_sAccessibleName = SwResId(STR_STRUCTURE);
    m_sAdditionalAccnameString1 = SwResId(STR_ADDITIONAL_ACCNAME_STRING1);
    m_sAdditionalAccnameString2 = SwResId(STR_ADDITIONAL_ACCNAME_STRING2);
    m_sAdditionalAccnameString3 = SwResId(STR_ADDITIONAL_ACCNAME_STRING3);

    Link<weld::Button&,void> aLink(LINK(this, SwTokenWindow, ScrollBtnHdl));
    m_xLeftScrollWin->connect_clicked(aLink);
    m_xRightScrollWin->connect_clicked(aLink);
}

SwTokenWindow::~SwTokenWindow()
{
}

void SwTokenWindow::SetForm(SwForm& rForm, sal_uInt16 nL)
{
    SetActiveControl(nullptr);
    m_bValid = true;

    if (m_pForm)
    {
        //apply current level settings to the form
        m_aControlList.clear();
    }

    m_nLevel = nL;
    m_pForm = &rForm;
    //now the display
    if(m_nLevel < MAXLEVEL || rForm.GetTOXType() == TOX_AUTHORITIES)
    {
        // #i21237#
        SwFormTokens aPattern = m_pForm->GetPattern(m_nLevel + 1);
        bool bLastWasText = false; //assure alternating text - code - text

        SwTOXWidget* pSetActiveControl = nullptr;
        for (const auto& aToken : aPattern) // #i21237#
        {
            if(TOKEN_TEXT == aToken.eTokenType)
            {
                SAL_WARN_IF(bLastWasText, "sw", "text following text is invalid");
                SwTOXWidget* pCtrl = InsertItem(aToken.sText, aToken);
                bLastWasText = true;
                if (!GetActiveControl())
                    SetActiveControl(pCtrl);
            }
            else
            {
                if( !bLastWasText )
                {
                    SwFormToken aTemp(TOKEN_TEXT);
                    SwTOXWidget* pCtrl = InsertItem(OUString(), aTemp);
                    if(!pSetActiveControl)
                        pSetActiveControl = pCtrl;
                }

                OUString sForm;
                switch( aToken.eTokenType )
                {
                case TOKEN_ENTRY_NO:     sForm = SwForm::GetFormEntryNum(); break;
                case TOKEN_ENTRY_TEXT:   sForm = SwForm::GetFormEntryText(); break;
                case TOKEN_ENTRY:        sForm = SwForm::GetFormEntry(); break;
                case TOKEN_TAB_STOP:     sForm = SwForm::GetFormTab(); break;
                case TOKEN_PAGE_NUMS:    sForm = SwForm::GetFormPageNums(); break;
                case TOKEN_CHAPTER_INFO: sForm = SwForm::GetFormChapterMark(); break;
                case TOKEN_LINK_START:   sForm = SwForm::GetFormLinkStt(); break;
                case TOKEN_LINK_END:     sForm = SwForm::GetFormLinkEnd(); break;
                case TOKEN_AUTHORITY:    sForm = SwForm::GetFormAuth(); break;
                default:; //prevent warning
                }

                InsertItem( sForm, aToken );
                bLastWasText = false;
            }
        }
        if(!bLastWasText)
        {
            SwFormToken aTemp(TOKEN_TEXT);
            SwTOXWidget* pCtrl = InsertItem(OUString(), aTemp);
            if(!pSetActiveControl)
                pSetActiveControl = pCtrl;
        }
        SetActiveControl(pSetActiveControl);
    }
    AdjustScrolling();
}

void SwTokenWindow::SetActiveControl(SwTOXWidget* pSet)
{
    if (pSet == m_pActiveCtrl)
        return;

    m_pActiveCtrl = pSet;
    if( !m_pActiveCtrl )
        return;

    m_pActiveCtrl->GrabFocus();
    //it must be a SwTOXEdit
    const SwFormToken* pFToken;
    if( WindowType::EDIT == m_pActiveCtrl->GetType() )
        pFToken = &static_cast<SwTOXEdit*>(m_pActiveCtrl)->GetFormToken();
    else
        pFToken = &static_cast<SwTOXButton*>(m_pActiveCtrl)->GetFormToken();

    SwFormToken aTemp( *pFToken );
    m_aButtonSelectedHdl.Call( aTemp );
}

SwTOXWidget* SwTokenWindow::InsertItem(const OUString& rText, const SwFormToken& rToken)
{
    SwTOXWidget* pRet = nullptr;

    if (TOKEN_TEXT == rToken.eTokenType)
    {
        SwTOXEdit* pEdit = new SwTOXEdit(this, rToken);
        pEdit->set_grid_left_attach(m_aControlList.size());

        m_aControlList.emplace_back(pEdit);

        pEdit->SetText(rText);
        sal_uInt32 nIndex = GetControlIndex( TOKEN_TEXT );
        OUString strName(m_sAccessibleName + OUString::number(nIndex));
        if ( nIndex == 1 )
        {
            /*Press left or right arrow to choose the structure controls*/
            strName += " (" + m_sAdditionalAccnameString2 + ", "
            /*Press Ctrl+Alt+A to move focus for more operations*/
                     + m_sAdditionalAccnameString1 + ", "
            /*Press Ctrl+Alt+B to move focus back to the current structure control*/
                     + m_sAdditionalAccnameString3 + ")";
        }
        pEdit->SetAccessibleName(strName);
        pEdit->AdjustSize();
        pEdit->SetModifyHdl(LINK(this, SwTokenWindow, EditResize ));
        pEdit->SetPrevNextLink(LINK(this, SwTokenWindow, NextItemHdl));
        pEdit->SetGetFocusHdl(LINK(this, SwTokenWindow, TbxFocusHdl));
        pEdit->Show();
        pRet = pEdit;
    }
    else
    {
        SwTOXButton* pButton = new SwTOXButton(this, rToken);
        pButton->set_grid_left_attach(m_aControlList.size());

        m_aControlList.emplace_back(pButton);

        pButton->SetPrevNextLink(LINK(this, SwTokenWindow, NextItemBtnHdl));
        pButton->SetGetFocusHdl(LINK(this, SwTokenWindow, TbxFocusBtnHdl));

        if(TOKEN_AUTHORITY != rToken.eTokenType)
            pButton->SetText(m_aButtonTexts[rToken.eTokenType]);
        else
        {
            //use the first two chars as symbol
            OUString sTmp(SwAuthorityFieldType::GetAuthFieldName(
                        static_cast<ToxAuthorityField>(rToken.nAuthorityField)));
            pButton->SetText(sTmp.copy(0, std::min(sTmp.getLength(), sal_Int32(2))));
        }

        sal_uInt32 nIndex = GetControlIndex( rToken.eTokenType );
        OUString sAccName = m_aButtonHelpTexts[rToken.eTokenType];
        if ( nIndex )
        {
            sAccName += " " + OUString::number(nIndex);
        }
        pButton->SetAccessibleName( sAccName );

        pButton->Show();
        pRet = pButton;
    }

    return pRet;
}

void SwTokenWindow::InsertAtSelection(const SwFormToken& rToken)
{
    OSL_ENSURE(m_pActiveCtrl, "no active control!");

    if(!m_pActiveCtrl)
        return;

    SwFormToken aToInsertToken(rToken);

    if(TOKEN_LINK_START == aToInsertToken.eTokenType)
    {
        //determine if start or end of hyperlink is appropriate
        //eventually change a following link start into a link end
        // groups of LS LE should be ignored
        // <insert>
        //LS <insert>
        //LE <insert>
        //<insert> LS
        //<insert> LE
        //<insert>
        bool bPreStartLinkFound = false;
        bool bPreEndLinkFound = false;

        const SwTOXWidget* pControl = nullptr;
        const SwTOXWidget* pExchange = nullptr;

        auto it = m_aControlList.cbegin();
        for( ; it != m_aControlList.cend() && m_pActiveCtrl != it->get(); ++it )
        {
            pControl = it->get();

            if( WindowType::EDIT != pControl->GetType())
            {
                const SwFormToken& rNewToken =
                                static_cast<const SwTOXButton*>(pControl)->GetFormToken();

                if( TOKEN_LINK_START == rNewToken.eTokenType )
                {
                    bPreStartLinkFound = true;
                    pExchange = nullptr;
                }
                else if(TOKEN_LINK_END == rNewToken.eTokenType)
                {
                    if( bPreStartLinkFound )
                        bPreStartLinkFound = false;
                    else
                    {
                        bPreEndLinkFound = false;
                        pExchange = pControl;
                    }
                }
            }
        }

        bool bPostLinkStartFound = false;

        if(!bPreStartLinkFound && !bPreEndLinkFound)
        {
            for( ; it != m_aControlList.cend(); ++it )
            {
                pControl = it->get();

                if( pControl != m_pActiveCtrl &&
                    WindowType::EDIT != pControl->GetType())
                {
                    const SwFormToken& rNewToken =
                                    static_cast<const SwTOXButton*>(pControl)->GetFormToken();

                    if( TOKEN_LINK_START == rNewToken.eTokenType )
                    {
                        if(bPostLinkStartFound)
                            break;
                        bPostLinkStartFound = true;
                        pExchange = pControl;
                    }
                    else if(TOKEN_LINK_END == rNewToken.eTokenType )
                    {
                        if(bPostLinkStartFound)
                        {
                            bPostLinkStartFound = false;
                            pExchange = nullptr;
                        }
                        break;
                    }
                }
            }
        }

        if(bPreStartLinkFound)
        {
            aToInsertToken.eTokenType = TOKEN_LINK_END;
            aToInsertToken.sText =  m_aButtonTexts[TOKEN_LINK_END];
        }

        if(bPostLinkStartFound)
        {
            OSL_ENSURE(pExchange, "no control to exchange?");
            if(pExchange)
            {
                const_cast<SwTOXButton*>(static_cast<const SwTOXButton*>(pExchange))->SetLinkEnd();
                const_cast<SwTOXButton*>(static_cast<const SwTOXButton*>(pExchange))->SetText(m_aButtonTexts[TOKEN_LINK_END]);
            }
        }

        if(bPreEndLinkFound)
        {
            OSL_ENSURE(pExchange, "no control to exchange?");

            if(pExchange)
            {
                const_cast<SwTOXButton*>(static_cast<const SwTOXButton*>(pExchange))->SetLinkStart();
                const_cast<SwTOXButton*>(static_cast<const SwTOXButton*>(pExchange))->SetText(m_aButtonTexts[TOKEN_LINK_START]);
            }
        }
    }

    //if the active control is text then insert a new button at the selection
    //else replace the button
    auto iterActive = std::find_if(m_aControlList.begin(), m_aControlList.end(),
                               [this](const auto& rControl)
                               {
                                   SwTOXWidget* pCtrl = rControl.get();
                                   return pCtrl == m_pActiveCtrl;
                               });

    assert(iterActive != m_aControlList.end());
    if (iterActive == m_aControlList.end())
        return;

    if (WindowType::EDIT == m_pActiveCtrl->GetType())
    {
        ++iterActive;

        int nStartPos, nEndPos;
        static_cast<SwTOXEdit*>(m_pActiveCtrl)->get_selection_bounds(nStartPos, nEndPos);

        const OUString sEditText = static_cast<SwTOXEdit*>(m_pActiveCtrl)->GetText();
        const OUString sLeft = sEditText.copy( 0, std::min(nStartPos, nEndPos) );
        const OUString sRight = sEditText.copy( std::max(nStartPos, nEndPos) );

        static_cast<SwTOXEdit*>(m_pActiveCtrl)->SetText(sLeft);
        static_cast<SwTOXEdit*>(m_pActiveCtrl)->AdjustSize();

        SwFormToken aTmpToken(TOKEN_TEXT);
        SwTOXEdit* pEdit = new SwTOXEdit(this, aTmpToken);
        iterActive = m_aControlList.emplace(iterActive, pEdit);

        pEdit->SetText(sRight);
        sal_uInt32 nIndex = GetControlIndex( TOKEN_TEXT );
        OUString strName(m_sAccessibleName + OUString::number(nIndex));
        if ( nIndex == 1)
        {
            /*Press left or right arrow to choose the structure controls*/
            strName += " (" + m_sAdditionalAccnameString2 + ", "
            /*Press Ctrl+Alt+A to move focus for more operations*/
                     + m_sAdditionalAccnameString1 + ", "
            /*Press Ctrl+Alt+B to move focus back to the current structure control*/
                     + m_sAdditionalAccnameString3 + ")";
        }
        pEdit->SetAccessibleName(strName);
        pEdit->AdjustSize();
        pEdit->SetModifyHdl(LINK(this, SwTokenWindow, EditResize ));
        pEdit->SetPrevNextLink(LINK(this, SwTokenWindow, NextItemHdl));
        pEdit->SetGetFocusHdl(LINK(this, SwTokenWindow, TbxFocusHdl));
        pEdit->Show();
    }
    else
    {
        m_pActiveCtrl->Hide();
        m_pActiveCtrl = nullptr;
        iterActive = m_aControlList.erase(iterActive);
    }

    //now the new button
    SwTOXButton* pButton = new SwTOXButton(this, aToInsertToken);
    m_aControlList.emplace(iterActive, pButton);

    pButton->SetPrevNextLink(LINK(this, SwTokenWindow, NextItemBtnHdl));
    pButton->SetGetFocusHdl(LINK(this, SwTokenWindow, TbxFocusBtnHdl));

    if (TOKEN_AUTHORITY != aToInsertToken.eTokenType)
    {
        pButton->SetText(m_aButtonTexts[aToInsertToken.eTokenType]);
    }
    else
    {
        //use the first two chars as symbol
        OUString sTmp(SwAuthorityFieldType::GetAuthFieldName(
                    static_cast<ToxAuthorityField>(aToInsertToken.nAuthorityField)));
        pButton->SetText(sTmp.copy(0, std::min(sTmp.getLength(), sal_Int32(2))));
    }

    pButton->Check();
    pButton->Show();
    SetActiveControl(pButton);

    AdjustPositions();
}

void SwTokenWindow::RemoveControl(const SwTOXButton* pDel, bool bInternalCall)
{
    if (bInternalCall && TOX_AUTHORITIES == m_pForm->GetTOXType())
        m_pParent->PreTokenButtonRemoved(pDel->GetFormToken());

    auto it = std::find_if(m_aControlList.begin(), m_aControlList.end(),
                               [pDel](const auto& rControl)
                               {
                                   SwTOXWidget* pCtrl = rControl.get();
                                   return pCtrl == pDel;
                               });
    assert(it != m_aControlList.end()); //Control does not exist!
    if (it == m_aControlList.end())
        return;

    // the two neighbours of the box must be merged
    // the properties of the right one will be lost
    assert(it != m_aControlList.begin() && it != m_aControlList.end() - 1); //Button at first or last position?
    if (it == m_aControlList.begin() || it == m_aControlList.end() - 1)
        return;

    auto itLeft = it, itRight = it;
    --itLeft;
    ++itRight;
    SwTOXWidget* pLeftEdit = itLeft->get();
    SwTOXWidget* pRightEdit = itRight->get();

    static_cast<SwTOXEdit*>(pLeftEdit)->SetText(static_cast<SwTOXEdit*>(pLeftEdit)->GetText() +
                                     static_cast<SwTOXEdit*>(pRightEdit)->GetText());
    static_cast<SwTOXEdit*>(pLeftEdit)->AdjustSize();

    m_pActiveCtrl->Hide();
    m_pActiveCtrl = nullptr;

    m_aControlList.erase(itRight);
    m_aControlList.erase(it);

    SetActiveControl(pLeftEdit);
    AdjustPositions();
    m_aModifyHdl.Call(nullptr);
}

IMPL_LINK_NOARG(SwTokenWindow, AdjustPositionsHdl, const Size&, void)
{
    AdjustScrolling();
}

void SwTokenWindow::AdjustPositions()
{
    for (size_t i = 0; i < m_aControlList.size(); ++i)
        m_aControlList[i]->set_grid_left_attach(i);
    AdjustScrolling();
}

void SwTokenWindow::MoveControls(tools::Long nOffset)
{
    m_xScrollWin->hadjustment_set_value(nOffset);
}

IMPL_LINK_NOARG(SwTokenWindow, ScrollHdl, weld::ScrolledWindow&, void)
{
    AdjustScrolling();
}

void SwTokenWindow::AdjustScrolling()
{
    if (m_aControlList.size() <= 1)
        return;

    //validate scroll buttons

    auto nLeft = m_xScrollWin->hadjustment_get_value();
    auto nSpace = m_xScrollWin->hadjustment_get_page_size();
    auto nWidth = m_xScrollWin->hadjustment_get_upper();

    bool bEnable = nWidth > nSpace;

    //the active control must be visible
    if (bEnable && m_pActiveCtrl)
    {
        int x, y, width, height;
        m_pActiveCtrl->get_extents_relative_to(*m_xCtrlParentWin, x, y, width, height);

        if (x < nLeft || x + width > nLeft + nSpace)
        {
            MoveControls(x);
            nLeft = x;
        }

        m_xLeftScrollWin->set_sensitive(nLeft > 0);
        m_xRightScrollWin->set_sensitive(nLeft + nSpace < nWidth);
    }
    else
    {
        //if the control fits into the space then the first control must be at position 0
        m_xRightScrollWin->set_sensitive(false);
        m_xLeftScrollWin->set_sensitive(false);
    }
}

IMPL_LINK(SwTokenWindow, ScrollBtnHdl, weld::Button&, rBtn, void)
{
    if (m_aControlList.empty())
        return;

    const auto nSpace = m_xScrollWin->hadjustment_get_page_size();
    const auto nWidth = m_xScrollWin->hadjustment_get_upper();
    const auto nLeft = m_xScrollWin->hadjustment_get_value();

    tools::Long nMove = nLeft;
    if (&rBtn == m_xLeftScrollWin.get())
    {
        //find the first completely visible control (left edge visible)
        auto it = std::find_if(m_aControlList.begin(), m_aControlList.end(),
            [this, nLeft](const auto& rControl)
            {
                SwTOXWidget* pCtrl = rControl.get();

                int x, y, width, height;
                pCtrl->get_extents_relative_to(*m_xCtrlParentWin, x, y, width, height);

                return x >= nLeft;
            });
        if (it != m_aControlList.end())
        {
            if (it == m_aControlList.begin())
            {
                nMove = 0;
            }
            else
            {
                //move the left neighbor to the start position
                auto itLeft = it;
                --itLeft;
                SwTOXWidget* pLeft = itLeft->get();

                int x, y, width, height;
                pLeft->get_extents_relative_to(*m_xCtrlParentWin, x, y, width, height);

                nMove = x;
            }
        }
    }
    else
    {
        //find the first completely visible control (right edge visible)
        auto it = std::find_if(m_aControlList.rbegin(), m_aControlList.rend(),
            [this, nLeft, nSpace](const auto& rControl) {
                SwTOXWidget* pCtrl = rControl.get();

                int x, y, width, height;
                pCtrl->get_extents_relative_to(*m_xCtrlParentWin, x, y, width, height);

                auto nXPos = x + width;
                return nXPos <= nLeft + nSpace;
            });
        if (it != m_aControlList.rend() && it != m_aControlList.rbegin())
        {
            //move the right neighbor  to the right edge right aligned
            auto itRight = it;
            --itRight;
            SwTOXWidget* pRight = itRight->get();

            int x, y, width, height;
            pRight->get_extents_relative_to(*m_xCtrlParentWin, x, y, width, height);

            nMove = x + width - nSpace;
        }

        //move it left until it's completely visible
    }

    if (nMove != nLeft)
    {
        // move the complete list
        MoveControls(nMove);
        m_xLeftScrollWin->set_sensitive(nMove > 0);
        m_xRightScrollWin->set_sensitive(nMove + nSpace < nWidth);
    }
}

OUString SwTokenWindow::GetPattern() const
{
    OUStringBuffer sRet;

    for (const auto& elem : m_aControlList)
    {
        const SwTOXWidget* pCtrl = elem.get();

        const SwFormToken &rNewToken = pCtrl->GetType() == WindowType::EDIT
                ? const_cast<SwTOXEdit*>(static_cast<const SwTOXEdit*>(pCtrl))->GetFormToken()
                : static_cast<const SwTOXButton*>(pCtrl)->GetFormToken();

        //TODO: prevent input of TOX_STYLE_DELIMITER in KeyInput
        sRet.append(rNewToken.GetString());
    }

    return sRet.makeStringAndClear();
}

// Check if a control of the specified TokenType is already contained in the list
bool SwTokenWindow::Contains(FormTokenType eSearchFor) const
{
    bool bRet = false;

    for (const auto& elem : m_aControlList)
    {
        const SwTOXWidget* pCtrl = elem.get();
        const SwFormToken &rNewToken = pCtrl->GetType() == WindowType::EDIT
                ? const_cast<SwTOXEdit*>(static_cast<const SwTOXEdit*>(pCtrl))->GetFormToken()
                : static_cast<const SwTOXButton*>(pCtrl)->GetFormToken();

        if (eSearchFor == rNewToken.eTokenType)
        {
            bRet = true;
            break;
        }
    }

    return bRet;
}

OUString SwTokenWindow::CreateQuickHelp(const SwFormToken& rToken)
{
    OUString sEntry;
    if (rToken.eTokenType != TOKEN_AUTHORITY)
        sEntry = m_aButtonHelpTexts[rToken.eTokenType];
    else
    {
        sEntry += SwAuthorityFieldType::GetAuthFieldName(
                            static_cast<ToxAuthorityField>(rToken.nAuthorityField));
    }

    if (rToken.eTokenType != TOKEN_TAB_STOP)
    {
        if (!rToken.sCharStyleName.isEmpty())
        {
            sEntry += " " + m_sCharStyle + rToken.sCharStyleName;
        }
    }

    return sEntry;
}

IMPL_LINK(SwTokenWindow, EditResize, SwTOXEdit&, rEdit, void)
{
    rEdit.AdjustSize();
    AdjustPositions();
    m_aModifyHdl.Call(nullptr);
}

IMPL_LINK(SwTokenWindow, NextItemHdl, SwTOXEdit&, rEdit, void)
{
    auto it = std::find_if(m_aControlList.begin(), m_aControlList.end(),
                               [&rEdit](const auto& rControl)
                               {
                                   SwTOXWidget* pCtrl = rControl.get();
                                   return pCtrl == &rEdit;
                               });

    if (it == m_aControlList.end())
        return;

    auto itTest = it;
    ++itTest;

    if ((it != m_aControlList.begin() && !rEdit.IsNextControl()) ||
        (itTest != m_aControlList.end() && rEdit.IsNextControl()))
    {
        auto iterFocus = it;
        rEdit.IsNextControl() ? ++iterFocus : --iterFocus;

        SwTOXWidget *pCtrlFocus = iterFocus->get();
        pCtrlFocus->GrabFocus();
        static_cast<SwTOXButton*>(pCtrlFocus)->Check();

        AdjustScrolling();
    }
}

IMPL_LINK(SwTokenWindow, TbxFocusHdl, SwTOXWidget&, rControl, void)
{
    SwTOXEdit* pEdit = static_cast<SwTOXEdit*>(&rControl);
    for (const auto& aControl : m_aControlList)
    {
        SwTOXWidget* pCtrl = aControl.get();
        if (pCtrl && pCtrl->GetType() != WindowType::EDIT)
            static_cast<SwTOXButton*>(pCtrl)->Check(false);
    }

    SetActiveControl(pEdit);
}

IMPL_LINK(SwTokenWindow, NextItemBtnHdl, SwTOXButton&, rBtn, void )
{
    auto it = std::find_if(m_aControlList.begin(), m_aControlList.end(),
                               [&rBtn](const auto& rControl)
                               {
                                   SwTOXWidget* pCtrl = rControl.get();
                                   return pCtrl == &rBtn;
                               });

    if (it == m_aControlList.end())
        return;

    auto itTest = it;
    ++itTest;

    if (rBtn.IsNextControl() && (itTest == m_aControlList.end() || !rBtn.IsNextControl()))
        return;

    bool isNext = rBtn.IsNextControl();

    auto iterFocus = it;
    isNext ? ++iterFocus : --iterFocus;

    SwTOXWidget* pCtrlFocus = iterFocus->get();
    pCtrlFocus->GrabFocus();
    int nStartPos(0), nEndPos(0);

    if (!isNext)
    {
        const sal_Int32 nLen = static_cast<SwTOXEdit*>(pCtrlFocus)->GetText().getLength();

        nStartPos = nLen;
        nEndPos = nLen;
    }

    static_cast<SwTOXEdit*>(pCtrlFocus)->select_region(nStartPos, nEndPos);

    rBtn.Check(false);

    AdjustScrolling();
}

IMPL_LINK(SwTokenWindow, TbxFocusBtnHdl, SwTOXWidget&, rControl, void)
{
    SwTOXButton* pBtn = static_cast<SwTOXButton*>(&rControl);
    for (const auto& aControl : m_aControlList)
    {
        SwTOXWidget* pControl = aControl.get();

        if (pControl && WindowType::EDIT != pControl->GetType())
            static_cast<SwTOXButton*>(pControl)->Check(pBtn == pControl);
    }

    SetActiveControl(pBtn);
}

void SwTokenWindow::SetFocus2theAllBtn()
{
    if (m_pParent)
    {
        m_pParent->SetFocus2theAllBtn();
    }
}

sal_uInt32 SwTokenWindow::GetControlIndex(FormTokenType eType) const
{
    //there are only one entry-text button and only one page-number button,
    //so we need not add index for these two buttons.
    if ( eType == TOKEN_ENTRY_TEXT || eType == TOKEN_PAGE_NUMS )
    {
        return 0;
    }

    sal_uInt32 nIndex = 0;
    for (const auto& elem : m_aControlList)
    {
        const SwTOXWidget* pControl = elem.get();

        const SwFormToken& rNewToken = WindowType::EDIT == pControl->GetType()
            ? const_cast<SwTOXEdit*>(static_cast<const SwTOXEdit*>(pControl))->GetFormToken()
            : static_cast<const SwTOXButton*>(pControl)->GetFormToken();

        if(eType == rNewToken.eTokenType)
        {
            ++nIndex;
        }
    }

    return nIndex;
}

SwTOXStylesTabPage::SwTOXStylesTabPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rAttrSet)
    : SfxTabPage(pPage, pController, "modules/swriter/ui/tocstylespage.ui", "TocStylesPage", &rAttrSet)
    , m_xLevelLB(m_xBuilder->weld_tree_view("levels"))
    , m_xAssignBT(m_xBuilder->weld_button("assign"))
    , m_xParaLayLB(m_xBuilder->weld_tree_view("styles"))
    , m_xStdBT(m_xBuilder->weld_button("default"))
    , m_xEditStyleBT(m_xBuilder->weld_button("edit"))
{
    m_xParaLayLB->make_sorted();
    auto nHeight = m_xLevelLB->get_height_rows(16);
    m_xLevelLB->set_size_request(-1, nHeight);
    m_xParaLayLB->set_size_request(-1, nHeight);

    SetExchangeSupport();

    m_xEditStyleBT->connect_clicked(LINK(this, SwTOXStylesTabPage, EditStyleHdl));
    m_xAssignBT->connect_clicked(LINK(this, SwTOXStylesTabPage, AssignHdl));
    m_xStdBT->connect_clicked(LINK(this, SwTOXStylesTabPage, StdHdl));
    m_xParaLayLB->connect_changed(LINK(this, SwTOXStylesTabPage, EnableSelectHdl));
    m_xLevelLB->connect_changed(LINK(this, SwTOXStylesTabPage, EnableSelectHdl));
    m_xParaLayLB->connect_row_activated(LINK(this, SwTOXStylesTabPage, DoubleClickHdl));
}

SwTOXStylesTabPage::~SwTOXStylesTabPage()
{
}

bool SwTOXStylesTabPage::FillItemSet( SfxItemSet* )
{
    return true;
}

void SwTOXStylesTabPage::Reset( const SfxItemSet* rSet )
{
    ActivatePage(*rSet);
}

void SwTOXStylesTabPage::ActivatePage( const SfxItemSet& )
{
    m_pCurrentForm.reset(new SwForm(GetForm()));

    // not hyperlink for user directories
    const sal_uInt16 nSize = m_pCurrentForm->GetFormMax();

    // display form pattern without title

    m_xLevelLB->freeze();
    m_xLevelLB->clear();
    // display 1st TemplateEntry
    OUString aStr( SwResId( STR_TITLE ));
    if( !m_pCurrentForm->GetTemplate( 0 ).isEmpty() )
    {
        aStr += " " + OUStringChar(aDeliStart)
              + m_pCurrentForm->GetTemplate( 0 )
              + OUStringChar(aDeliEnd);
    }
    m_xLevelLB->append_text(aStr);

    for( sal_uInt16 i=1; i < nSize; ++i )
    {
        if( TOX_INDEX == m_pCurrentForm->GetTOXType() &&
            FORM_ALPHA_DELIMITER == i )
        {
            aStr = SwResId(STR_ALPHA);
        }
        else
        {
            aStr = SwResId(STR_LEVEL) + OUString::number(
                    TOX_INDEX == m_pCurrentForm->GetTOXType() ? i - 1 : i );
        }
        if( !m_pCurrentForm->GetTemplate( i ).isEmpty() )
        {
            aStr += " " + OUStringChar(aDeliStart)
                  + m_pCurrentForm->GetTemplate( i )
                  + OUStringChar(aDeliEnd);
        }
        m_xLevelLB->append_text(aStr);
    }
    m_xLevelLB->thaw();

    // initialise templates
    SwWrtShell& rSh = static_cast<SwMultiTOXTabDialog*>(GetDialogController())->GetWrtShell();
    const sal_uInt16 nSz = rSh.GetTextFormatCollCount();

    m_xParaLayLB->freeze();
    m_xParaLayLB->clear();
    for( sal_uInt16 i = 0; i < nSz; ++i )
    {
        const SwTextFormatColl *pColl = &rSh.GetTextFormatColl( i );
        if( !pColl->IsDefault() )
            m_xParaLayLB->append_text( pColl->GetName() );
    }

    // query pool collections and set them for the directory
    for( sal_uInt16 i = 0; i < m_pCurrentForm->GetFormMax(); ++i )
    {
        aStr = m_pCurrentForm->GetTemplate( i );
        if (!aStr.isEmpty() && m_xParaLayLB->find_text(aStr) == -1)
            m_xParaLayLB->append_text(aStr);
    }
    m_xParaLayLB->thaw();

    EnableSelectHdl(*m_xParaLayLB);
}

DeactivateRC SwTOXStylesTabPage::DeactivatePage( SfxItemSet* /*pSet*/  )
{
    GetForm() = *m_pCurrentForm;
    return DeactivateRC::LeavePage;
}

std::unique_ptr<SfxTabPage> SwTOXStylesTabPage::Create(weld::Container* pPage, weld::DialogController* pController,
                                              const SfxItemSet* rAttrSet)
{
    return std::make_unique<SwTOXStylesTabPage>(pPage, pController, *rAttrSet);
}

IMPL_LINK_NOARG(SwTOXStylesTabPage, EditStyleHdl, weld::Button&, void)
{
    if (m_xParaLayLB->get_selected_index() != -1)
    {
        SfxStringItem aStyle(SID_STYLE_EDIT, m_xParaLayLB->get_selected_text());
        SfxUInt16Item aFamily(SID_STYLE_FAMILY, sal_uInt16(SfxStyleFamily::Para));
        SwWrtShell& rSh = static_cast<SwMultiTOXTabDialog*>(GetDialogController())->GetWrtShell();
        rSh.GetView().GetViewFrame().GetDispatcher()->ExecuteList(SID_STYLE_EDIT,
                SfxCallMode::SYNCHRON,
                { &aStyle, &aFamily });
    }
}

// allocate templates
IMPL_LINK_NOARG(SwTOXStylesTabPage, AssignHdl, weld::Button&, void)
{
    auto nLevPos = m_xLevelLB->get_selected_index();
    auto nTemplPos = m_xParaLayLB->get_selected_index();
    if (nLevPos == -1 || nTemplPos == -1)
        return;

    const OUString aStr(o3tl::getToken(m_xLevelLB->get_text(nLevPos), 0, aDeliStart)
        + OUStringChar(aDeliStart)
        + m_xParaLayLB->get_selected_text()
        + OUStringChar(aDeliEnd));

    m_pCurrentForm->SetTemplate(nLevPos, m_xParaLayLB->get_selected_text());

    m_xLevelLB->remove(nLevPos);
    m_xLevelLB->insert_text(nLevPos, aStr);
    m_xLevelLB->select_text(aStr);
    Modify();
}

IMPL_LINK_NOARG(SwTOXStylesTabPage, StdHdl, weld::Button&, void)
{
    const auto nPos = m_xLevelLB->get_selected_index();
    if (nPos != -1)
    {
        const OUString aStr(m_xLevelLB->get_text(nPos).getToken(0, aDeliStart));
        m_xLevelLB->remove(nPos);
        m_xLevelLB->insert_text(nPos, aStr);
        m_xLevelLB->select_text(aStr);
        m_pCurrentForm->SetTemplate(nPos, OUString());
        Modify();
    }
}

IMPL_LINK_NOARG(SwTOXStylesTabPage, DoubleClickHdl, weld::TreeView&, bool)
{
    const OUString aTmpName(m_xParaLayLB->get_selected_text());
    SwWrtShell& rSh = static_cast<SwMultiTOXTabDialog*>(GetDialogController())->GetWrtShell();

    if(m_xParaLayLB->get_selected_index() != -1 &&
       (m_xLevelLB->get_selected_index() == 0 || SwMultiTOXTabDialog::IsNoNum(rSh, aTmpName)))
        AssignHdl(*m_xAssignBT);

    return true;
}

// enable only when selected
IMPL_LINK_NOARG(SwTOXStylesTabPage, EnableSelectHdl, weld::TreeView&, void)
{
    m_xStdBT->set_sensitive(m_xLevelLB->get_selected_index() != -1);

    SwWrtShell& rSh = static_cast<SwMultiTOXTabDialog*>(GetDialogController())->GetWrtShell();
    const OUString aTmpName(m_xParaLayLB->get_selected_text());
    m_xAssignBT->set_sensitive(m_xParaLayLB->get_selected_index() != -1 &&
                               m_xLevelLB->get_selected_index() != -1 &&
       (m_xLevelLB->get_selected_index() == 0 || SwMultiTOXTabDialog::IsNoNum(rSh, aTmpName)));
    m_xEditStyleBT->set_sensitive(m_xParaLayLB->get_selected_index() != -1);
}

void SwTOXStylesTabPage::Modify()
{
    SwMultiTOXTabDialog* pTOXDlg = static_cast<SwMultiTOXTabDialog*>(GetDialogController());
    if (pTOXDlg)
    {
        GetForm() = *m_pCurrentForm;
        pTOXDlg->CreateOrUpdateExample(pTOXDlg->GetCurrentTOXType().eType, TOX_PAGE_STYLES);
    }
}

#define ITEM_SEARCH         1
#define ITEM_ALTERNATIVE    2
#define ITEM_PRIM_KEY       3
#define ITEM_SEC_KEY        4
#define ITEM_COMMENT        5
#define ITEM_CASE           6
#define ITEM_WORDONLY       7

SwEntryBrowseBox::SwEntryBrowseBox(const css::uno::Reference<css::awt::XWindow> &rParent)
    : SwEntryBrowseBox_Base(VCLUnoHelper::GetWindow(rParent), EditBrowseBoxFlags::NONE, WB_TABSTOP | WB_BORDER,
                           BrowserMode::KEEPHIGHLIGHT |
                           BrowserMode::COLUMNSELECTION |
                           BrowserMode::MULTISELECTION |
                           BrowserMode::TRACKING_TIPS |
                           BrowserMode::HLINES |
                           BrowserMode::VLINES |
                           BrowserMode::AUTO_VSCROLL|
                           BrowserMode::HIDECURSOR   )
    , m_aCellEdit(VclPtr<svt::EditControl>::Create(&GetDataWindow()))
    , m_aCellCheckBox(VclPtr<svt::CheckBoxControl>::Create(&GetDataWindow()))
    , m_nCurrentRow(0)
    , m_bModified(false)
{
    OUString sSearch = SwResId(STR_AUTOMARK_SEARCHTERM);
    OUString sAlternative = SwResId(STR_AUTOMARK_ALTERNATIVE);
    OUString sPrimKey = SwResId(STR_AUTOMARK_KEY1);
    OUString sSecKey = SwResId(STR_AUTOMARK_KEY2);
    OUString sComment = SwResId(STR_AUTOMARK_COMMENT);
    OUString sCaseSensitive = SwResId(STR_AUTOMARK_CASESENSITIVE);
    OUString sWordOnly = SwResId(STR_AUTOMARK_WORDONLY);
    m_sYes = SwResId(STR_AUTOMARK_YES);
    m_sNo = SwResId(STR_AUTOMARK_NO);

    m_aCellCheckBox->EnableTriState(false);
    m_xController = new ::svt::EditCellController(m_aCellEdit.get());
    m_xCheckController = new ::svt::CheckBoxCellController(m_aCellCheckBox.get());

    // HACK: BrowseBox doesn't invalidate its children, how it should be.
    // That's why WB_CLIPCHILDREN is reset in order to enforce the
    // children' invalidation
    WinBits aStyle = GetStyle();
    if( aStyle & WB_CLIPCHILDREN )
    {
        aStyle &= ~WB_CLIPCHILDREN;
        SetStyle( aStyle );
    }

    const OUString* aTitles[7] =
    {
        &sSearch,
        &sAlternative,
        &sPrimKey,
        &sSecKey,
        &sComment,
        &sCaseSensitive,
        &sWordOnly
    };

    tools::Long nWidth = GetSizePixel().Width();
    nWidth /=7;
    --nWidth;
    for(sal_uInt16 i = 1; i < 8; i++)
        InsertDataColumn( i, *aTitles[i - 1], nWidth );
}

SwEntryBrowseBox::~SwEntryBrowseBox()
{
    disposeOnce();
}

void SwEntryBrowseBox::dispose()
{
    m_aCellEdit.disposeAndClear();
    m_aCellCheckBox.disposeAndClear();
    SwEntryBrowseBox_Base::dispose();
}

void SwEntryBrowseBox::Resize()
{
    SwEntryBrowseBox_Base::Resize();

    tools::Long nWidth = GetSizePixel().Width();
    std::vector<tools::Long> aWidths = GetOptimalColWidths();
    tools::Long nNaturalWidth(std::accumulate(aWidths.begin(), aWidths.end(), 0));
    tools::Long nExcess = ((nWidth - nNaturalWidth) / aWidths.size()) - 1;

    for (size_t i = 0; i < aWidths.size(); ++i)
        SetColumnWidth(i+1, aWidths[i] + nExcess);
}

std::vector<tools::Long> SwEntryBrowseBox::GetOptimalColWidths() const
{
    std::vector<tools::Long> aWidths;

    tools::Long nStandardColMinWidth = approximate_digit_width() * 15;
    tools::Long nYesNoWidth = approximate_digit_width() * 5;
    nYesNoWidth = std::max(nYesNoWidth, GetTextWidth(m_sYes));
    nYesNoWidth = std::max(nYesNoWidth, GetTextWidth(m_sNo));
    for (sal_uInt16 i = 1; i < 6; i++)
    {
        tools::Long nColWidth = std::max(nStandardColMinWidth,
                                  GetTextWidth(GetColumnTitle(i)));
        nColWidth += 12;
        aWidths.push_back(nColWidth);
    }

    for (sal_uInt16 i = 6; i < 8; i++)
    {
        tools::Long nColWidth = std::max(nYesNoWidth,
                                  GetTextWidth(GetColumnTitle(i)));
        nColWidth += 12;
        aWidths.push_back(nColWidth);
    }

    return aWidths;
}

Size SwEntryBrowseBox::GetOptimalSize() const
{
    Size aSize = LogicToPixel(Size(276 , 175), MapMode(MapUnit::MapAppFont));

    std::vector<tools::Long> aWidths = GetOptimalColWidths();

    tools::Long nWidth(std::accumulate(aWidths.begin(), aWidths.end(), 0));

    aSize.setWidth( std::max(aSize.Width(), nWidth) );

    return aSize;
}

bool SwEntryBrowseBox::SeekRow( sal_Int32 nRow )
{
    m_nCurrentRow = nRow;
    return true;
}

OUString SwEntryBrowseBox::GetCellText(sal_Int32 nRow, sal_uInt16 nColumn) const
{
    OUString pRet;
    if (o3tl::make_unsigned(nRow) < m_Entries.size())
    {
        const AutoMarkEntry* pEntry = m_Entries[ nRow ].get();
        switch(nColumn)
        {
            case ITEM_SEARCH      : pRet = pEntry->sSearch;      break;
            case ITEM_ALTERNATIVE : pRet = pEntry->sAlternative; break;
            case ITEM_PRIM_KEY    : pRet = pEntry->sPrimKey;     break;
            case ITEM_SEC_KEY     : pRet = pEntry->sSecKey;      break;
            case ITEM_COMMENT     : pRet = pEntry->sComment;     break;
            case ITEM_CASE        : pRet = pEntry->bCase ? m_sYes : m_sNo; break;
            case ITEM_WORDONLY    : pRet = pEntry->bWord ? m_sYes : m_sNo; break;
        }
    }
    return pRet;
}

void SwEntryBrowseBox::PaintCell(OutputDevice& rDev,
                                const tools::Rectangle& rRect, sal_uInt16 nColumnId) const
{
    const DrawTextFlags nStyle = DrawTextFlags::Clip | DrawTextFlags::Center;
    rDev.DrawText( rRect, GetCellText( m_nCurrentRow, nColumnId ), nStyle );
}

::svt::CellController* SwEntryBrowseBox::GetController(sal_Int32 /*nRow*/, sal_uInt16 nCol)
{
    return nCol < ITEM_CASE ? m_xController.get() : m_xCheckController.get();
}

bool SwEntryBrowseBox::SaveModified()
{
    m_bModified = true;
    const size_t nRow = GetCurRow();
    const sal_uInt16 nCol = GetCurColumnId();

    OUString sNew;
    bool bVal = false;
    ::svt::CellController* pController = nullptr;
    if(nCol < ITEM_CASE)
    {
        pController = m_xController.get();
        sNew = static_cast< ::svt::EditCellController*>(pController)->GetEditImplementation()->GetText( LINEEND_LF );
    }
    else
    {
        pController = m_xCheckController.get();
        bVal = static_cast< ::svt::CheckBoxCellController*>(pController)->GetCheckBox().get_active();
    }
    const bool bAddEntry = nRow >= m_Entries.size();
    std::unique_ptr<AutoMarkEntry> xNewEntry(bAddEntry ? new AutoMarkEntry : nullptr);
    AutoMarkEntry* pEntry = bAddEntry ? xNewEntry.get() : m_Entries[nRow].get();
    switch(nCol)
    {
        case  ITEM_SEARCH       : pEntry->sSearch = sNew; break;
        case  ITEM_ALTERNATIVE  : pEntry->sAlternative = sNew; break;
        case  ITEM_PRIM_KEY     : pEntry->sPrimKey   = sNew; break;
        case  ITEM_SEC_KEY      : pEntry->sSecKey    = sNew; break;
        case  ITEM_COMMENT      : pEntry->sComment   = sNew; break;
        case  ITEM_CASE         : pEntry->bCase = bVal; break;
        case  ITEM_WORDONLY     : pEntry->bWord = bVal; break;
    }
    if (bAddEntry)
    {
        m_Entries.push_back(std::move(xNewEntry));
        RowInserted(nRow, 1, true, true);
        if(nCol < ITEM_WORDONLY)
        {
            pController->SaveValue();
            GoToRow( nRow );
        }
    }
    return true;
}

void SwEntryBrowseBox::InitController(
                ::svt::CellControllerRef& rController, sal_Int32 nRow, sal_uInt16 nCol)
{
    const OUString rText = GetCellText( nRow, nCol );
    if(nCol < ITEM_CASE)
    {
        rController = m_xController;
        ::svt::CellController* pController = m_xController.get();
        static_cast< ::svt::EditCellController*>(pController)->GetEditImplementation()->SetText( rText );
    }
    else
    {
        rController = m_xCheckController;
        ::svt::CellController* pController = m_xCheckController.get();
        static_cast< ::svt::CheckBoxCellController*>(pController)->GetCheckBox().set_active(
                                                            rText == m_sYes );
     }
}

void SwEntryBrowseBox::ReadEntries(SvStream& rInStr)
{
    AutoMarkEntry* pToInsert = nullptr;
    // tdf#108910, tdf#125496 - read index entries using the appropriate character set
    rtl_TextEncoding eTEnc = SwIoSystem::GetTextEncoding(rInStr);
    if (eTEnc == RTL_TEXTENCODING_DONTKNOW)
        eTEnc = osl_getThreadTextEncoding();
    while (rInStr.good())
    {
        OUString sLine;
        rInStr.ReadByteStringLine( sLine, eTEnc );

        // # -> comment
        // ; -> delimiter between entries ->
        // Format: TextToSearchFor;AlternativeString;PrimaryKey;SecondaryKey
        // Leading and trailing blanks are ignored
        if( !sLine.isEmpty() )
        {
            //comments are contained in separate lines but are put into the struct of the following data
            //line (if available)
            if( '#' != sLine[0] )
            {
                if( !pToInsert )
                    pToInsert = new AutoMarkEntry;

                sal_Int32 nSttPos = 0;
                pToInsert->sSearch      = sLine.getToken(0, ';', nSttPos );
                pToInsert->sAlternative = sLine.getToken(0, ';', nSttPos );
                pToInsert->sPrimKey     = sLine.getToken(0, ';', nSttPos );
                pToInsert->sSecKey      = sLine.getToken(0, ';', nSttPos );

                std::u16string_view sStr = o3tl::getToken(sLine, 0, ';', nSttPos );
                pToInsert->bCase = !sStr.empty() && sStr != u"0";

                sStr = o3tl::getToken(sLine, 0, ';', nSttPos );
                pToInsert->bWord = !sStr.empty() && sStr != u"0";

                m_Entries.push_back(std::unique_ptr<AutoMarkEntry>(pToInsert));
                pToInsert = nullptr;
            }
            else
            {
                if(pToInsert)
                    m_Entries.push_back(std::unique_ptr<AutoMarkEntry>(pToInsert));
                pToInsert = new AutoMarkEntry;
                pToInsert->sComment = sLine.copy(1);
            }
        }
    }
    if( pToInsert )
        m_Entries.push_back(std::unique_ptr<AutoMarkEntry>(pToInsert));
    RowInserted(0, m_Entries.size() + 1);
}

void SwEntryBrowseBox::WriteEntries(SvStream& rOutStr)
{
    //check if the current controller is modified
    const sal_uInt16 nCol = GetCurColumnId();
    ::svt::CellController* pController;
    if(nCol < ITEM_CASE)
        pController = m_xController.get();
    else
        pController = m_xCheckController.get();
    if (pController->IsValueChangedFromSaved())
        GoToColumnId(nCol + (nCol < ITEM_CASE ? 1 : -1 ));

    for(const std::unique_ptr<AutoMarkEntry> & rpEntry : m_Entries)
    {
        AutoMarkEntry* pEntry = rpEntry.get();
        if(!pEntry->sComment.isEmpty())
        {
            // tdf#108910, tdf#125496 - write index entries using the utf8 text encoding
            rOutStr.WriteByteStringLine( Concat2View("#" + pEntry->sComment), RTL_TEXTENCODING_UTF8 );
        }

        OUString sWrite( pEntry->sSearch + ";" +
                         pEntry->sAlternative + ";" +
                         pEntry->sPrimKey  + ";" +
                         pEntry->sSecKey + ";" +
                         (pEntry->bCase ? std::u16string_view(u"1") : std::u16string_view(u"0")) +
                         ";" +
                         (pEntry->bWord ? std::u16string_view(u"1") : std::u16string_view(u"0")) );

        if( sWrite.getLength() > 5 )
            // tdf#108910, tdf#125496 - write index entries using the utf8 text encoding
            rOutStr.WriteByteStringLine( sWrite, RTL_TEXTENCODING_UTF8 );
    }
}

bool SwEntryBrowseBox::IsModified()const
{
    if(m_bModified)
        return true;

    //check if the current controller is modified
    const sal_uInt16 nCol = GetCurColumnId();
    ::svt::CellController* pController;
    if(nCol < ITEM_CASE)
        pController = m_xController.get();
    else
        pController = m_xCheckController.get();
    return pController->IsValueChangedFromSaved();
}

SwAutoMarkDlg_Impl::SwAutoMarkDlg_Impl(weld::Window* pParent, OUString aAutoMarkURL,
        bool bCreate)
    : GenericDialogController(pParent, "modules/swriter/ui/createautomarkdialog.ui", "CreateAutomarkDialog")
    , m_sAutoMarkURL(std::move(aAutoMarkURL))
    , m_bCreateMode(bCreate)
    , m_xOKPB(m_xBuilder->weld_button("ok"))
    , m_xTable(m_xBuilder->weld_container("area"))
    , m_xTableCtrlParent(m_xTable->CreateChildFrame())
    , m_xEntriesBB(VclPtr<SwEntryBrowseBox>::Create(m_xTableCtrlParent))
{
    m_xEntriesBB->Show();
    m_xOKPB->connect_clicked(LINK(this, SwAutoMarkDlg_Impl, OkHdl));

    m_xDialog->set_title(m_xDialog->get_title() + ": " + m_sAutoMarkURL);
    bool bError = false;
    if( m_bCreateMode )
        m_xEntriesBB->RowInserted(0);
    else
    {
        SfxMedium aMed( m_sAutoMarkURL, StreamMode::STD_READ );
        if( aMed.GetInStream() && !aMed.GetInStream()->GetError() )
            m_xEntriesBB->ReadEntries( *aMed.GetInStream() );
        else
            bError = true;
    }

    Size aPrefSize = m_xEntriesBB->GetOptimalSize();
    m_xTable->set_size_request(aPrefSize.Width(), aPrefSize.Height());

    if (bError)
        m_xDialog->response(RET_CANCEL);
}

SwAutoMarkDlg_Impl::~SwAutoMarkDlg_Impl()
{
    m_xEntriesBB.disposeAndClear();
    m_xTableCtrlParent->dispose();
    m_xTableCtrlParent.clear();
}

IMPL_LINK_NOARG(SwAutoMarkDlg_Impl, OkHdl, weld::Button&, void)
{
    bool bError = false;
    if (m_xEntriesBB->IsModified() || m_bCreateMode)
    {
        SfxMedium aMed( m_sAutoMarkURL,
                        m_bCreateMode ? StreamMode::WRITE
                                    : StreamMode::WRITE| StreamMode::TRUNC );
        SvStream* pStrm = aMed.GetOutStream();
        // tdf#108910, tdf#125496 - write index entries using the utf8 text encoding
        pStrm->SetStreamCharSet( RTL_TEXTENCODING_UTF8 );
        if( !pStrm->GetError() )
        {
            m_xEntriesBB->WriteEntries( *pStrm );
            aMed.Commit();
        }
        else
            bError = true;
    }
    if (!bError)
        m_xDialog->response(RET_OK);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
