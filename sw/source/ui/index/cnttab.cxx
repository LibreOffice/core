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

#include <config_folders.h>

#include <sal/log.hxx>
#include <vcl/builderfactory.hxx>
#include <svl/style.hxx>
#include <vcl/help.hxx>
#include <vcl/weld.hxx>
#include <svl/stritem.hxx>
#include <svl/urihelper.hxx>
#include <unotools/pathoptions.hxx>
#include <sfx2/request.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/docfile.hxx>
#include <svtools/simptabl.hxx>
#include <vcl/treelistentry.hxx>
#include <svx/dialogs.hrc>
#include <svx/svxdlg.hxx>
#include <svx/flagsdef.hxx>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <com/sun/star/ui/dialogs/XFilePicker3.hpp>
#include <com/sun/star/ui/dialogs/XFilterManager.hpp>
#include <svtools/indexentryres.hxx>
#include <editeng/unolingu.hxx>
#include <column.hxx>
#include <fmtfsize.hxx>
#include <shellio.hxx>
#include <authfld.hxx>
#include <swtypes.hxx>
#include <wrtsh.hxx>
#include <view.hxx>
#include <basesh.hxx>
#include <outline.hxx>
#include <cnttab.hxx>
#include <swuicnttab.hxx>
#include <poolfmt.hxx>
#include <strings.hrc>
#include <uitool.hxx>
#include <fmtcol.hxx>
#include <fldbas.hxx>
#include <expfld.hxx>
#include <unotools.hxx>
#include <unotxdoc.hxx>
#include <docsh.hxx>
#include <swmodule.hxx>
#include <modcfg.hxx>

#include <cmdid.h>
#include <globals.hrc>
#include <cnttab.hrc>
#include <SwStyleNameMapper.hxx>
#include <sfx2/filedlghelper.hxx>
#include <toxwrap.hxx>
#include <chpfld.hxx>

#include <sfx2/app.hxx>

#include <unomid.h>

#include <memory>
#include <vector>
#include <numeric>


using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace com::sun::star::ui::dialogs;
using namespace ::sfx2;
#include <svtools/editbrowsebox.hxx>

static const sal_Unicode aDeliStart = '['; // for the form
static const sal_Unicode aDeliEnd    = ']'; // for the form

static OUString lcl_CreateAutoMarkFileDlg(weld::Window* pParent, const OUString& rURL,
                                const OUString& rFileString, bool bOpen)
{
    OUString sRet;

    FileDialogHelper aDlgHelper( bOpen ?
                TemplateDescription::FILEOPEN_SIMPLE : TemplateDescription::FILESAVE_AUTOEXTENSION,
                FileDialogFlags::NONE, pParent);
    uno::Reference < XFilePicker3 > xFP = aDlgHelper.GetFilePicker();

    uno::Reference<XFilterManager> xFltMgr(xFP, UNO_QUERY);
    xFltMgr->appendFilter( rFileString, "*.sdi" );
    xFltMgr->setCurrentFilter( rFileString ) ;

    if( !rURL.isEmpty() )
        xFP->setDisplayDirectory( rURL );
    else
    {
        SvtPathOptions aPathOpt;
        xFP->setDisplayDirectory( aPathOpt.GetUserConfigPath() );
    }

    if( aDlgHelper.Execute() == ERRCODE_NONE )
    {
        sRet = xFP->getSelectedFiles().getConstArray()[0];
    }

    return sRet;
}

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

typedef ::svt::EditBrowseBox SwEntryBrowseBox_Base;

class SwEntryBrowseBox : public SwEntryBrowseBox_Base
{
    VclPtr<Edit>                    m_aCellEdit;
    VclPtr< ::svt::CheckBoxControl>  m_aCellCheckBox;

    OUString  m_sSearch;
    OUString  m_sAlternative;
    OUString  m_sPrimKey;
    OUString  m_sSecKey;
    OUString  m_sComment;
    OUString  m_sCaseSensitive;
    OUString  m_sWordOnly;
    OUString  m_sYes;
    OUString  m_sNo;

    std::vector<std::unique_ptr<AutoMarkEntry>> m_Entries;

    ::svt::CellControllerRef    m_xController;
    ::svt::CellControllerRef    m_xCheckController;

    long    m_nCurrentRow;
    bool    m_bModified;

protected:
    virtual bool                    SeekRow( long nRow ) override;
    virtual void                    PaintCell(OutputDevice& rDev, const tools::Rectangle& rRect, sal_uInt16 nColId) const override;
    virtual void                    InitController(::svt::CellControllerRef& rController, long nRow, sal_uInt16 nCol) override;
    virtual ::svt::CellController*  GetController(long nRow, sal_uInt16 nCol) override;
    virtual bool                    SaveModified() override;

    std::vector<long>               GetOptimalColWidths() const;

public:
    SwEntryBrowseBox(vcl::Window* pParent, VclBuilderContainer* pBuilder);
    virtual ~SwEntryBrowseBox() override;
    virtual void                    dispose() override;
    void                            ReadEntries(SvStream& rInStr);
    void                            WriteEntries(SvStream& rOutStr);

    bool                            IsModified()const override;

    virtual OUString GetCellText( long nRow, sal_uInt16 nColumn ) const override;
    virtual void                    Resize() override;
    virtual Size                    GetOptimalSize() const override;
};

class SwAutoMarkDlg_Impl : public ModalDialog
{
    VclPtr<OKButton>           m_pOKPB;

    VclPtr<SwEntryBrowseBox>   m_pEntriesBB;

    OUString            sAutoMarkURL;

    bool                bCreateMode;

    DECL_LINK(OkHdl, Button*, void);
public:
    SwAutoMarkDlg_Impl(vcl::Window* pParent, const OUString& rAutoMarkURL,
                       bool bCreate);
    virtual ~SwAutoMarkDlg_Impl() override;
    virtual void dispose() override;

};

sal_uInt16 CurTOXType::GetFlatIndex() const
{
    return static_cast< sal_uInt16 >( (eType == TOX_USER && nIndex)
        ? TOX_AUTHORITIES + nIndex : eType );
}

#define EDIT_MINWIDTH 15

SwMultiTOXTabDialog::SwMultiTOXTabDialog(vcl::Window* pParent, const SfxItemSet& rSet,
                    SwWrtShell &rShell,
                    SwTOXBase* pCurTOX,
                    sal_uInt16 nToxType, bool bGlobal)
    : SfxTabDialog(pParent, "TocDialog",
        "modules/swriter/ui/tocdialog.ui", &rSet)
    , m_pMgr( new SwTOXMgr( &rShell ) )
    , m_rWrtShell(rShell)
    , m_pParamTOXBase(pCurTOX)
    , m_sUserDefinedIndex(SwResId(STR_USER_DEFINED_INDEX))
    , m_nInitialTOXType(nToxType)
    , m_bEditTOX(false)
    , m_bExampleCreated(false)
    , m_bGlobalFlag(bGlobal)
{
    get(m_pShowExampleCB, "showexample");
    get(m_pExampleContainerWIN, "example");
    Size aWinSize(LogicToPixel(Size(150, 188), MapMode(MapUnit::MapAppFont)));
    m_pExampleContainerWIN->set_width_request(aWinSize.Width());
    m_pExampleContainerWIN->set_height_request(aWinSize.Height());
    m_pExampleContainerWIN->SetSizePixel(aWinSize);

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
        m_vTypeData[i].m_pxIndexSections.reset(new SwIndexSections_Impl);
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
                                                m_rWrtShell.GetFieldType(SwFieldIds::TableOfAuthorities, aEmptyOUStr));
                if(pFType)
                {
                    OUString sBrackets;
                    if(pFType->GetPrefix())
                        sBrackets += OUStringLiteral1(pFType->GetPrefix());
                    if(pFType->GetSuffix())
                        sBrackets += OUStringLiteral1(pFType->GetSuffix());
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
    m_nSelectId = AddTabPage("index", SwTOXSelectTabPage::Create, nullptr);
    AddTabPage("styles", SwTOXStylesTabPage::Create, nullptr);
    m_nColumnId = AddTabPage("columns", SwColumnPage::Create, nullptr);
    m_nBackGroundId = AddTabPage("background", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BKG ), nullptr);
    m_nEntriesId = AddTabPage("entries", SwTOXEntryTabPage::Create, nullptr);
    if(!pCurTOX)
        SetCurPageId(m_nSelectId);

    m_pShowExampleCB->SetClickHdl(LINK(this, SwMultiTOXTabDialog, ShowPreviewHdl));

    m_pShowExampleCB->Check( SW_MOD()->GetModuleConfig()->IsShowIndexPreview());

    SetViewAlign( WindowAlign::Left );
    // SetViewWindow does not work if the dialog is visible!

    if(!m_pShowExampleCB->IsChecked())
        SetViewWindow(m_pExampleContainerWIN);

    ShowPreviewHdl(nullptr);
}

SwMultiTOXTabDialog::~SwMultiTOXTabDialog()
{
    disposeOnce();
}

void SwMultiTOXTabDialog::dispose()
{
    SW_MOD()->GetModuleConfig()->SetShowIndexPreview(m_pShowExampleCB->IsChecked());

    // fdo#38515 Avoid setting focus on deleted controls in the destructors
    EnableInput( false );

    m_vTypeData.clear();
    m_pMgr.reset();
    m_pExampleFrame.reset();
    m_pExampleContainerWIN.clear();
    m_pShowExampleCB.clear();
    SfxTabDialog::dispose();
}

void SwMultiTOXTabDialog::PageCreated( sal_uInt16 nId, SfxTabPage &rPage )
{
    if (nId == m_nBackGroundId)
    {
        SfxAllItemSet aSet(*(GetInputSetImpl()->GetPool()));
        aSet.Put (SfxUInt32Item(SID_FLAG_TYPE, static_cast<sal_uInt32>(SvxBackgroundTabFlags::SHOW_SELECTOR)));
        rPage.PageCreated(aSet);
    }
    else if(nId == m_nColumnId)
    {
        const SwFormatFrameSize& rSize = GetInputSetImpl()->Get(RES_FRM_SIZE);

        static_cast<SwColumnPage&>(rPage).SetPageWidth(rSize.GetWidth());
    }
    else if (nId == m_nEntriesId)
        static_cast<SwTOXEntryTabPage&>(rPage).SetWrtShell(m_rWrtShell);
    else if (nId == m_nSelectId)
    {
        static_cast<SwTOXSelectTabPage&>(rPage).SetWrtShell(m_rWrtShell);
        if(USHRT_MAX != m_nInitialTOXType)
            static_cast<SwTOXSelectTabPage&>(rPage).SelectType(static_cast<TOXTypes>(m_nInitialTOXType));
    }
}

short SwMultiTOXTabDialog::Ok()
{
    short nRet = SfxTabDialog::Ok();
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
                                            m_rWrtShell.GetFieldType(SwFieldIds::TableOfAuthorities, aEmptyOUStr));
            if(pFType)
            {
                m_vTypeData[nIndex].m_pDescription->SetAuthBrackets(OUStringLiteral1(pFType->GetPrefix()) +
                                                  OUStringLiteral1(pFType->GetSuffix()));
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

IMPL_LINK_NOARG( SwMultiTOXTabDialog, ShowPreviewHdl, Button*, void )
{
    if(m_pShowExampleCB->IsChecked())
    {
        if(!m_pExampleFrame && !m_bExampleCreated)
        {
            m_bExampleCreated = true;
            OUString sTemplate("internal/idxexample.odt");

            SvtPathOptions aOpt;
            bool bExist = aOpt.SearchFile( sTemplate, SvtPathOptions::PATH_TEMPLATE );

            if(!bExist)
            {
                OUString sInfo(SwResId(STR_FILE_NOT_FOUND));
                sInfo = sInfo.replaceFirst( "%1", sTemplate );
                sInfo = sInfo.replaceFirst( "%2", aOpt.GetTemplatePath() );
                std::unique_ptr<weld::MessageDialog> xInfoBox(Application::CreateMessageDialog(GetFrameWeld(),
                                                              VclMessageType::Info, VclButtonsType::Ok,
                                                              sInfo));
                xInfoBox->run();
            }
            else
            {
                Link<SwOneExampleFrame&,void> aLink(LINK(this, SwMultiTOXTabDialog, CreateExample_Hdl));
                m_pExampleFrame.reset(new SwOneExampleFrame(
                        *m_pExampleContainerWIN, EX_SHOW_ONLINE_LAYOUT, &aLink, &sTemplate));

                if(!m_pExampleFrame->IsServiceAvailable())
                {
                    SwOneExampleFrame::CreateErrorMessage();
                }
            }
            m_pShowExampleCB->Show(m_pExampleFrame && m_pExampleFrame->IsServiceAvailable());
        }
    }
    bool bSetViewWindow = m_pShowExampleCB->IsChecked()
        && m_pExampleFrame && m_pExampleFrame->IsServiceAvailable();

    m_pExampleContainerWIN->Show( bSetViewWindow );
    SetViewWindow( bSetViewWindow ? m_pExampleContainerWIN.get() : nullptr );

    setOptimalLayoutSize();
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

class SwAddStylesDlg_Impl : public SfxDialogController
{
    OUString*       pStyleArr;

    std::unique_ptr<weld::Button> m_xOk;
    std::unique_ptr<weld::Button> m_xLeftPB;
    std::unique_ptr<weld::Button> m_xRightPB;
    std::unique_ptr<weld::TreeView> m_xHeaderTree;

    DECL_LINK(OkHdl, weld::Button&, void);
    DECL_LINK(LeftRightHdl, weld::Button&, void);
    DECL_LINK(KeyInput, const KeyEvent&, bool);
    DECL_LINK(TreeSizeAllocHdl, const Size&, void);
    typedef std::pair<int, int> row_col;
    DECL_LINK(RadioToggleOnHdl, const row_col&, void);

public:
    SwAddStylesDlg_Impl(weld::Window* pParent, SwWrtShell const & rWrtSh, OUString rStringArr[]);
};

SwAddStylesDlg_Impl::SwAddStylesDlg_Impl(weld::Window* pParent,
            SwWrtShell const & rWrtSh, OUString rStringArr[])
    : SfxDialogController(pParent, "modules/swriter/ui/assignstylesdialog.ui", "AssignStylesDialog")
    , pStyleArr(rStringArr)
    , m_xOk(m_xBuilder->weld_button("ok"))
    , m_xLeftPB(m_xBuilder->weld_button("left"))
    , m_xRightPB(m_xBuilder->weld_button("right"))
    , m_xHeaderTree(m_xBuilder->weld_tree_view("styles"))
{
    m_xOk->connect_clicked(LINK(this, SwAddStylesDlg_Impl, OkHdl));
    m_xLeftPB->connect_clicked(LINK(this, SwAddStylesDlg_Impl, LeftRightHdl));
    m_xRightPB->connect_clicked(LINK(this, SwAddStylesDlg_Impl, LeftRightHdl));
    m_xHeaderTree->connect_size_allocate(LINK(this, SwAddStylesDlg_Impl, TreeSizeAllocHdl));

    std::vector<int> aRadioColumns;
    for (sal_uInt16 i = 0; i <= MAXLEVEL; ++i)
        aRadioColumns.push_back(i + 1);
    m_xHeaderTree->set_toggle_columns_as_radio(aRadioColumns);
    m_xHeaderTree->connect_toggled(LINK(this, SwAddStylesDlg_Impl, RadioToggleOnHdl));

    std::vector<int> aWidths;
    aWidths.push_back(m_xHeaderTree->get_approximate_digit_width() * 30);
    int nPadding = m_xHeaderTree->get_approximate_digit_width() * 2;
    OUString sTitle(m_xHeaderTree->get_column_title(1));
    for (sal_uInt16 i = 0; i <= MAXLEVEL; ++i)
    {
        sTitle = OUString::number(i);
        m_xHeaderTree->set_column_title(i + 1, sTitle);
        aWidths.push_back(m_xHeaderTree->get_pixel_size(sTitle).Width() + nPadding);
    }
    m_xHeaderTree->set_column_fixed_widths(aWidths);
    auto nWidth = std::accumulate(aWidths.begin(), aWidths.end(), 0);
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
                m_xHeaderTree->set_toggle(nRow, i == j - 1, j + 1);
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
                    m_xHeaderTree->set_toggle(nRow, k == 0, k + 1);
                ++nRow;
            }
        }
    }
    m_xHeaderTree->make_sorted();
    m_xHeaderTree->select(0);
    m_xHeaderTree->connect_key_release(LINK(this, SwAddStylesDlg_Impl, KeyInput));
}

IMPL_LINK(SwAddStylesDlg_Impl, TreeSizeAllocHdl, const Size&, rSize, void)
{
    auto nWidth = rSize.Width();

    std::vector<int> aWidths;
    aWidths.push_back(0);
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

IMPL_LINK(SwAddStylesDlg_Impl, RadioToggleOnHdl, const row_col&, rRowCol, void)
{
    for (sal_uInt16 i = 0; i <= MAXLEVEL; ++i)
        m_xHeaderTree->set_toggle(rRowCol.first, rRowCol.second == i + 1, i + 1);
}

IMPL_LINK(SwAddStylesDlg_Impl, KeyInput, const KeyEvent&, rKEvt, bool)
{
    vcl::KeyCode aCode = rKEvt.GetKeyCode();
    bool bHandled = false;

    if (aCode.GetCode() == KEY_ADD || aCode.GetCode() == KEY_RIGHT)
    {
        LeftRightHdl(*m_xRightPB);
        bHandled = true;
    }
    else if (aCode.GetCode() == KEY_SUBTRACT || aCode.GetCode() == KEY_LEFT)
    {
        LeftRightHdl(*m_xLeftPB);
        bHandled = true;
    }

    return bHandled;
}

IMPL_LINK_NOARG(SwAddStylesDlg_Impl, OkHdl, weld::Button&, void)
{
    for(sal_uInt16 i = 0; i < MAXLEVEL; i++)
        pStyleArr[i].clear();

    int nChildren = m_xHeaderTree->n_children();
    for (int i = 0; i < nChildren; ++i)
    {
        int nToggleColumn = 0;
        for (sal_uInt16 j = 0; j <= MAXLEVEL; ++j)
        {
            if (m_xHeaderTree->get_toggle(i, j + 1))
            {
                nToggleColumn = j;
                break;
            }
        }
        if (nToggleColumn)
        {
            int nLevel = nToggleColumn - 1;
            if(!pStyleArr[nLevel].isEmpty())
                pStyleArr[nLevel] += OUStringLiteral1(TOX_STYLE_DELIMITER);
            pStyleArr[nLevel] += m_xHeaderTree->get_text(i, 0);
        }
    }

    //TODO write back style names
    m_xDialog->response(RET_OK);
}

IMPL_LINK(SwAddStylesDlg_Impl, LeftRightHdl, weld::Button&, rBtn, void)
{
    bool bLeft = &rBtn == m_xLeftPB.get();
    int nEntry = m_xHeaderTree->get_selected_index();
    if (nEntry != -1)
    {
        int nToggleColumn = 0;
        for (sal_uInt16 j = 0; j <= MAXLEVEL; ++j)
        {
            if (m_xHeaderTree->get_toggle(nEntry, j + 1))
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

        for (sal_uInt16 j = 0; j <= MAXLEVEL; ++j)
            m_xHeaderTree->set_toggle(nEntry, j == nToggleColumn, j + 1);
    }
}

SwTOXSelectTabPage::SwTOXSelectTabPage(TabPageParent pParent, const SfxItemSet& rAttrSet)
    : SfxTabPage(pParent, "modules/swriter/ui/tocindexpage.ui", "TocIndexPage", &rAttrSet)
    , sAutoMarkType(SwResId(STR_AUTOMARK_TYPE))
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
    , m_xLanguageLB(new LanguageBox(m_xBuilder->weld_combo_box("lang")))
    , m_xSortAlgorithmLB(m_xBuilder->weld_combo_box("keytype"))
{
    sAddStyleUser = m_xStylesCB->get_label();
    pIndexEntryWrapper.reset(new IndexEntrySupplierWrapper());

    m_xLanguageLB->SetLanguageList( SvxLanguageListFlags::ALL | SvxLanguageListFlags::ONLY_KNOWN,
                                 false );

    //Default mode is arranged to be the tallest mode
    //of alphabetical index, lock that height in now
    LanguageHdl(nullptr); //fill sort algorithm list
    Size aPrefSize(get_preferred_size());
    set_height_request(aPrefSize.Height());

    sAddStyleContent = m_xAddStylesCB->get_label();

    std::vector<int> aWidths;
    aWidths.push_back(m_xFromObjCLB->get_approximate_digit_width() * 3 + 6);
    m_xFromObjCLB->set_column_fixed_widths(aWidths);

    for (size_t i = 0; i < SAL_N_ELEMENTS(RES_SRCTYPES); ++i)
    {
        OUString sId(OUString::number(static_cast<sal_uInt32>(RES_SRCTYPES[i].second)));
        m_xFromObjCLB->insert(nullptr, -1, nullptr, &sId,
                              nullptr, nullptr, nullptr, false);
        m_xFromObjCLB->set_toggle(i, false, 0);
        m_xFromObjCLB->set_text(i, SwResId(RES_SRCTYPES[i].first), 1);
    }

    SetExchangeSupport();
    m_xTypeLB->connect_changed(LINK(this, SwTOXSelectTabPage, TOXTypeHdl));

    m_xAddStylesPB->connect_clicked(LINK(this, SwTOXSelectTabPage, AddStylesHdl));

    m_xAutoMarkPB->connect_toggled(LINK(this, SwTOXSelectTabPage, MenuEnableHdl));
    m_xAutoMarkPB->connect_selected(LINK(this, SwTOXSelectTabPage, MenuExecuteHdl));

    Link<weld::ToggleButton&,void> aLk =  LINK(this, SwTOXSelectTabPage, CheckBoxHdl);
    m_xAddStylesCB->connect_toggled(aLk);
    m_xFromHeadingsCB->connect_toggled(aLk);
    m_xTOXMarksCB->connect_toggled(aLk);
    m_xFromFileCB->connect_toggled(aLk);
    m_xCollectSameCB->connect_toggled(aLk);
    m_xUseFFCB->connect_toggled(aLk);
    m_xUseDashCB->connect_toggled(aLk);
    m_xInitialCapsCB->connect_toggled(aLk);
    m_xKeyAsEntryCB->connect_toggled(aLk);

    m_xTitleED->connect_changed(LINK(this, SwTOXSelectTabPage, ModifyEntryHdl));
    m_xLevelNF->connect_value_changed(LINK(this, SwTOXSelectTabPage, ModifySpinHdl));
    m_xSortAlgorithmLB->connect_changed(LINK(this, SwTOXSelectTabPage, ModifyListBoxHdl));

    aLk =  LINK(this, SwTOXSelectTabPage, RadioButtonHdl);
    m_xFromCaptionsRB->connect_toggled(aLk);
    m_xFromObjectNamesRB->connect_toggled(aLk);
    RadioButtonHdl(*m_xFromCaptionsRB);

    m_xLanguageLB->connect_changed(LINK(this, SwTOXSelectTabPage, LanguageListBoxHdl));
    m_xTypeLB->set_active(0);
    m_xTitleED->save_value();
}

SwTOXSelectTabPage::~SwTOXSelectTabPage()
{
    disposeOnce();
}

void SwTOXSelectTabPage::dispose()
{
    pIndexRes.reset();
    pIndexEntryWrapper.reset();
    m_xLanguageLB.reset();
    SfxTabPage::dispose();
}

void SwTOXSelectTabPage::SetWrtShell(SwWrtShell const & rSh)
{
    const sal_uInt16 nUserTypeCount = rSh.GetTOXTypeCount(TOX_USER);
    if(nUserTypeCount > 1)
    {
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
}

bool SwTOXSelectTabPage::FillItemSet( SfxItemSet* )
{
    return true;
}

static long lcl_TOXTypesToUserData(CurTOXType eType)
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
    SwMultiTOXTabDialog* pTOXDlg = static_cast<SwMultiTOXTabDialog*>(GetTabDialog());
    const CurTOXType aCurType = pTOXDlg->GetCurrentTOXType();
    SwTOXDescription& rDesc = pTOXDlg->GetTOXDescription(aCurType);
    m_xReadOnlyCB->set_active(rDesc.IsReadonly());
    if (!m_xTitleED->get_value_changed_from_saved())
    {
        if (rDesc.GetTitle())
            m_xTitleED->set_text(*rDesc.GetTitle());
        else
            m_xTitleED->set_text(aEmptyOUStr);
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

    //content
    if(TOX_CONTENT == aCurType.eType)
    {
        m_xFromHeadingsCB->set_active( bool(nCreateType & SwTOXElement::OutlineLevel) );
        m_xAddStylesCB->set_label(sAddStyleContent);
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
        m_xFromObjectNamesRB->set_active(rDesc.IsCreateFromObjectNames());
        m_xFromCaptionsRB->set_active(!rDesc.IsCreateFromObjectNames());
        OUString sName(rDesc.GetSequenceName());
        int nIndex = m_xCaptionSequenceLB->find_text(sName);
        if (nIndex != -1)
            m_xCaptionSequenceLB->set_active(nIndex);
        m_xDisplayTypeLB->set_active(static_cast<sal_Int32>(rDesc.GetCaptionDisplay()));
        if (m_xDisplayTypeLB->get_active() == -1)
            m_xDisplayTypeLB->set_active(0);
        RadioButtonHdl(*m_xFromCaptionsRB);

    }
    else if(TOX_OBJECTS == aCurType.eType)
    {
        SwTOOElements nOLEData = rDesc.GetOLEOptions();
        for (int nFromObj = 0, nCount = m_xFromObjCLB->n_children(); nFromObj < nCount; ++nFromObj)
        {
            SwTOOElements nData = static_cast<SwTOOElements>(m_xFromObjCLB->get_id(nFromObj).toInt32());
            m_xFromObjCLB->set_toggle(nFromObj, bool(nData & nOLEData), 0);
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
        aStyleArr[i] = rDesc.GetStyleNames(i);

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
    SwMultiTOXTabDialog* pTOXDlg = static_cast<SwMultiTOXTabDialog*>(GetTabDialog());
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
                rDesc.SetAutoMarkURL(sAutoMarkURL);
            else
                rDesc.SetAutoMarkURL(aEmptyOUStr);
        }
        break;
        case TOX_ILLUSTRATIONS:
        case TOX_TABLES :
            rDesc.SetCreateFromObjectNames(m_xFromObjectNamesRB->get_active());
            rDesc.SetSequenceName(m_xCaptionSequenceLB->get_active_text());
            rDesc.SetCaptionDisplay(static_cast<SwCaptionDisplay>(m_xDisplayTypeLB->get_active()));
        break;
        case TOX_OBJECTS:
        {
            SwTOOElements nOLEData = SwTOOElements::NONE;
            for (int i = 0, nCount = m_xFromObjCLB->n_children(); i < nCount; ++i)
            {
                if (m_xFromObjCLB->get_toggle(i, 0))
                {
                    SwTOOElements nData = static_cast<SwTOOElements>(m_xFromObjCLB->get_id(i).toInt32());
                    nOLEData |= nData;
                }
            }
            rDesc.SetOLEOptions(nOLEData);
        }
        break;
        case TOX_AUTHORITIES:
        case TOX_BIBLIOGRAPHY :
        {
            if (m_xBracketLB->get_active() != -1)
                rDesc.SetAuthBrackets(m_xBracketLB->get_active_text());
            else
                rDesc.SetAuthBrackets(aEmptyOUStr);
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
    if (m_xAddStylesCB->get_active() && m_xAddStylesCB->get_visible())
        nContentOptions |= SwTOXElement::Template;

    rDesc.SetContentOptions(nContentOptions);
    rDesc.SetIndexOptions(nIndexOptions);
    rDesc.SetLevel(m_xLevelNF->get_value());

    rDesc.SetReadonly(m_xReadOnlyCB->get_active());

    for(sal_uInt16 i = 0; i < MAXLEVEL; i++)
        rDesc.SetStyleNames(aStyleArr[i], i);

    rDesc.SetLanguage(m_xLanguageLB->get_active_id());
    const OUString& rEntryData = m_xSortAlgorithmLB->get_active_id();
    rDesc.SetSortAlgorithm(rEntryData);
}

void SwTOXSelectTabPage::Reset( const SfxItemSet* )
{
    SwMultiTOXTabDialog* pTOXDlg = static_cast<SwMultiTOXTabDialog*>(GetTabDialog());
    SwWrtShell& rSh = pTOXDlg->GetWrtShell();
    const CurTOXType aCurType = pTOXDlg->GetCurrentTOXType();
    sal_uInt32 nData = lcl_TOXTypesToUserData(aCurType);
    m_xTypeLB->set_active_id(OUString::number(nData));

    sAutoMarkURL = INetURLObject::decode( rSh.GetTOIAutoMarkURL(),
                                           INetURLObject::DecodeMechanism::Unambiguous );
    m_xFromFileCB->set_active(!sAutoMarkURL.isEmpty());

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

VclPtr<SfxTabPage> SwTOXSelectTabPage::Create(TabPageParent pParent, const SfxItemSet* rAttrSet)
{
    return VclPtr<SwTOXSelectTabPage>::Create(pParent, *rAttrSet);
}

IMPL_LINK(SwTOXSelectTabPage, TOXTypeHdl, weld::ComboBox&, rBox, void)
{
    SwMultiTOXTabDialog* pTOXDlg = static_cast<SwMultiTOXTabDialog*>(GetTabDialog());
    const sal_uInt16 nType = rBox.get_active_id().toUInt32();
    CurTOXType eCurType = lcl_UserData2TOXTypes(nType);
    pTOXDlg->SetCurrentTOXType(eCurType);

    m_xAreaLB->show( 0 != (nType & (TO_CONTENT|TO_ILLUSTRATION|TO_USER|TO_INDEX|TO_TABLE|TO_OBJECT)) );
    m_xLevelFT->show( 0 != (nType & (TO_CONTENT)) );
    m_xLevelNF->show( 0 != (nType & (TO_CONTENT)) );
    m_xLevelFromChapterCB->show( 0 != (nType & (TO_USER)) );
    m_xAreaFrame->show( 0 != (nType & (TO_CONTENT|TO_ILLUSTRATION|TO_USER|TO_INDEX|TO_TABLE|TO_OBJECT)) );

    m_xFromHeadingsCB->show( 0 != (nType & (TO_CONTENT)) );
    m_xAddStylesCB->show( 0 != (nType & (TO_CONTENT|TO_USER)) );
    m_xAddStylesPB->show( 0 != (nType & (TO_CONTENT|TO_USER)) );

    m_xFromTablesCB->show( 0 != (nType & (TO_USER)) );
    m_xFromFramesCB->show( 0 != (nType & (TO_USER)) );
    m_xFromGraphicsCB->show( 0 != (nType & (TO_USER)) );
    m_xFromOLECB->show( 0 != (nType & (TO_USER)) );

    m_xFromCaptionsRB->show( 0 != (nType & (TO_ILLUSTRATION|TO_TABLE)) );
    m_xFromObjectNamesRB->show( 0 != (nType & (TO_ILLUSTRATION|TO_TABLE)) );

    m_xTOXMarksCB->show( 0 != (nType & (TO_CONTENT|TO_USER)) );

    m_xCreateFrame->show( 0 != (nType & (TO_CONTENT|TO_ILLUSTRATION|TO_USER|TO_TABLE)) );
    m_xCaptionSequenceFT->show( 0 != (nType & (TO_ILLUSTRATION|TO_TABLE)) );
    m_xCaptionSequenceLB->show( 0 != (nType & (TO_ILLUSTRATION|TO_TABLE)) );
    m_xDisplayTypeFT->show( 0 != (nType & (TO_ILLUSTRATION|TO_TABLE)) );
    m_xDisplayTypeLB->show( 0 != (nType & (TO_ILLUSTRATION|TO_TABLE)) );

    m_xAuthorityFrame->show( 0 != (nType & TO_AUTHORITIES) );

    bool bEnableSortLanguage = 0 != (nType & (TO_INDEX|TO_AUTHORITIES));
    m_xSortFrame->show(bEnableSortLanguage);

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
        m_xAddStylesCB->set_label(sAddStyleUser);
    }

    m_xIdxOptionsFrame->show( 0 != (nType & TO_INDEX) );

    //object index
    m_xFromObjFrame->show( 0 != (nType & TO_OBJECT) );

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
        SwMultiTOXTabDialog* pTOXDlg = static_cast<SwMultiTOXTabDialog*>(GetTabDialog());
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

IMPL_LINK(SwTOXSelectTabPage, CheckBoxHdl, weld::ToggleButton&, rButton, void)
{
    SwMultiTOXTabDialog* pTOXDlg = static_cast<SwMultiTOXTabDialog*>(GetTabDialog());
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
    ModifyHdl();
};

IMPL_LINK_NOARG(SwTOXSelectTabPage, RadioButtonHdl, weld::ToggleButton&, void)
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
    Sequence< OUString > aSeq = pIndexEntryWrapper->GetAlgorithmList( aLcl );

    if( !pIndexRes )
        pIndexRes.reset(new IndexEntryResource());

    OUString sOldString = m_xSortAlgorithmLB->get_active_id();
    m_xSortAlgorithmLB->clear();

    sal_Int32 nEnd = aSeq.getLength();
    for( sal_Int32 nCnt = 0; nCnt < nEnd; ++nCnt )
    {
        const OUString sAlg(aSeq[ nCnt ]);
        const OUString sUINm = pIndexRes->GetTranslation( sAlg );
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
    SwAddStylesDlg_Impl aDlg(GetDialogFrameWeld(), static_cast<SwMultiTOXTabDialog*>(GetTabDialog())->GetWrtShell(),
        aStyleArr);
    aDlg.run();
    ModifyHdl();
}

IMPL_LINK_NOARG(SwTOXSelectTabPage, MenuEnableHdl, weld::ToggleButton&, void)
{
    m_xAutoMarkPB->set_item_sensitive("edit", !sAutoMarkURL.isEmpty());
}

IMPL_LINK(SwTOXSelectTabPage, MenuExecuteHdl, const OString&, rIdent, void)
{
    const OUString sSaveAutoMarkURL = sAutoMarkURL;

    if (rIdent == "open")
    {
        sAutoMarkURL = lcl_CreateAutoMarkFileDlg(GetDialogFrameWeld(),
                                sAutoMarkURL, sAutoMarkType, true);
    }
    else if (rIdent == "new" || rIdent == "edit")
    {
        bool bNew = (rIdent == "new");
        if (bNew)
        {
            sAutoMarkURL = lcl_CreateAutoMarkFileDlg(GetDialogFrameWeld(),
                                    sAutoMarkURL, sAutoMarkType, false);
            if (sAutoMarkURL.isEmpty())
                return;
        }

        VclPtrInstance<SwAutoMarkDlg_Impl> pAutoMarkDlg(
                GetTabDialog(), sAutoMarkURL, bNew);

        if( RET_OK != pAutoMarkDlg->Execute() && bNew )
            sAutoMarkURL = sSaveAutoMarkURL;
    }
}

class SwTOXEdit : public Edit
{
    SwFormToken           aFormToken;
    Link<SwTOXEdit&,void> aPrevNextControlLink;
    bool                  bNextControl;
    VclPtr<SwTokenWindow> m_pParent;
public:
    SwTOXEdit( vcl::Window* pParent, SwTokenWindow* pTokenWin,
                const SwFormToken& aToken)
        : Edit( pParent, WB_BORDER|WB_TABSTOP|WB_CENTER),
        aFormToken(aToken),
        bNextControl(false),
        m_pParent( pTokenWin )
    {
    }
    virtual ~SwTOXEdit() override { disposeOnce(); }
    virtual void dispose() override { m_pParent.clear(); Edit::dispose(); }

    virtual void    KeyInput( const KeyEvent& rKEvt ) override;
    virtual void    RequestHelp( const HelpEvent& rHEvt ) override;

    bool    IsNextControl() const {return bNextControl;}
    void SetPrevNextLink( const Link<SwTOXEdit&,void>& rLink ) {aPrevNextControlLink = rLink;}

    const SwFormToken& GetFormToken()
        {
            aFormToken.sText = GetText();
            return aFormToken;
        }

    void    SetCharStyleName(const OUString& rSet, sal_uInt16 nPoolId)
        {
            aFormToken.sCharStyleName = rSet;
            aFormToken.nPoolId = nPoolId;
        }

    void    AdjustSize();
};

void SwTOXEdit::RequestHelp( const HelpEvent& rHEvt )
{
    if(!m_pParent->CreateQuickHelp(this, aFormToken, rHEvt))
        Edit::RequestHelp(rHEvt);
}

void SwTOXEdit::KeyInput( const KeyEvent& rKEvt )
{
    const Selection& rSel = GetSelection();
    const sal_Int32 nTextLen = GetText().getLength();
    if((rSel.Min() == rSel.Max() &&
        !rSel.Min()) || rSel.Min() == nTextLen )
    {
        bool bCall = false;
        vcl::KeyCode aCode = rKEvt.GetKeyCode();
        if(aCode.GetCode() == KEY_RIGHT && rSel.Min() == nTextLen)
        {
            bNextControl = true;
            bCall = true;
        }
        else if(aCode.GetCode() == KEY_LEFT && !rSel.Min() )
        {
            bNextControl = false;
            bCall = true;
        }
        else if ( (aCode.GetCode() == KEY_F3) && aCode.IsShift() && !aCode.IsMod1() && !aCode.IsMod2() )
        {
            if ( m_pParent )
            {
                m_pParent->SetFocus2theAllBtn();
            }
        }
        if(bCall)
            aPrevNextControlLink.Call(*this);

    }
    Edit::KeyInput(rKEvt);
}

void SwTOXEdit::AdjustSize()
{
     Size aSize(GetSizePixel());
     Size aTextSize(GetTextWidth(GetText()), GetTextHeight());
    aTextSize = LogicToPixel(aTextSize);
    aSize.setWidth( aTextSize.Width() + EDIT_MINWIDTH );
    SetSizePixel(aSize);
}

class SwTOXButton : public PushButton
{
    SwFormToken             aFormToken;
    Link<SwTOXButton&,void> aPrevNextControlLink;
    bool                    bNextControl;
    VclPtr<SwTokenWindow>   m_pParent;
public:
    SwTOXButton( vcl::Window* pParent, SwTokenWindow* pTokenWin,
                const SwFormToken& rToken)
        : PushButton(pParent, WB_BORDER|WB_TABSTOP),
        aFormToken(rToken),
        bNextControl(false),
        m_pParent(pTokenWin)
    {
    }
    virtual ~SwTOXButton() override { disposeOnce(); }
    virtual void dispose() override { m_pParent.clear(); PushButton::dispose(); }

    virtual void KeyInput( const KeyEvent& rKEvt ) override;
    virtual void RequestHelp( const HelpEvent& rHEvt ) override;

    bool IsNextControl() const          {return bNextControl;}
    void SetPrevNextLink(const Link<SwTOXButton&,void>& rLink) {aPrevNextControlLink = rLink;}
    const SwFormToken& GetFormToken() const {return aFormToken;}

    void SetCharStyleName(const OUString& rSet, sal_uInt16 nPoolId)
        {
            aFormToken.sCharStyleName = rSet;
            aFormToken.nPoolId = nPoolId;
        }

    void SetTabPosition(SwTwips nSet)
        { aFormToken.nTabStopPosition = nSet; }

    void SetFillChar( sal_Unicode cSet )
        { aFormToken.cTabFillChar = cSet; }

    void SetTabAlign(SvxTabAdjust eAlign)
         {  aFormToken.eTabAlign = eAlign;}

//---> i89791
    //used for entry number format, in TOC only
    //needed for different UI dialog position
    void SetEntryNumberFormat(sal_uInt16 nSet) {
        switch(nSet)
        {
        default:
        case 0:
            aFormToken.nChapterFormat = CF_NUMBER;
            break;
        case 1:
            aFormToken.nChapterFormat = CF_NUM_NOPREPST_TITLE;
            break;
        }
    }

    void SetChapterInfo(sal_uInt16 nSet) {
        switch(nSet)
        {
        default:
        case 0:
            aFormToken.nChapterFormat = CF_NUM_NOPREPST_TITLE;
            break;
        case 1:
            aFormToken.nChapterFormat = CF_TITLE;
            break;
        case 2:
            aFormToken.nChapterFormat = CF_NUMBER_NOPREPST;
            break;
        }
    }

    void SetOutlineLevel( sal_uInt16 nSet ) { aFormToken.nOutlineLevel = nSet;}//i53420

    void SetLinkEnd()
        {
            OSL_ENSURE(TOKEN_LINK_START == aFormToken.eTokenType,
                                    "call SetLinkEnd for link start only!");
            aFormToken.eTokenType = TOKEN_LINK_END;
            aFormToken.sText = SwForm::GetFormLinkEnd();
            SetText(aFormToken.sText);
        }
    void SetLinkStart()
        {
            OSL_ENSURE(TOKEN_LINK_END == aFormToken.eTokenType,
                                    "call SetLinkStart for link start only!");
            aFormToken.eTokenType = TOKEN_LINK_START;
            aFormToken.sText = SwForm::GetFormLinkStt();
            SetText(aFormToken.sText);
        }
};

void SwTOXButton::KeyInput( const KeyEvent& rKEvt )
{
    bool bCall = false;
    vcl::KeyCode aCode = rKEvt.GetKeyCode();
    if(aCode.GetCode() == KEY_RIGHT)
    {
        bNextControl = true;
        bCall = true;
    }
    else if(aCode.GetCode() == KEY_LEFT  )
    {
        bNextControl = false;
        bCall = true;
    }
    else if(aCode.GetCode() == KEY_DELETE)
    {
        m_pParent->RemoveControl(this, true);
        //this is invalid here
        return;
    }
    else if ( (aCode.GetCode() == KEY_F3) && aCode.IsShift() && !aCode.IsMod1() && !aCode.IsMod2() )
    {
        if ( m_pParent )
        {
            m_pParent->SetFocus2theAllBtn();
        }
    }
    if(bCall && aPrevNextControlLink.IsSet())
            aPrevNextControlLink.Call(*this);
    else
        PushButton::KeyInput(rKEvt);
}

void SwTOXButton::RequestHelp( const HelpEvent& rHEvt )
{
    if(!m_pParent->CreateQuickHelp(this, aFormToken, rHEvt))
        Button::RequestHelp(rHEvt);
}

SwIdxTreeListBox::SwIdxTreeListBox(vcl::Window* pPar, WinBits nStyle)
    : SvTreeListBox(pPar, nStyle)
    , pParent(nullptr)
{
}

SwIdxTreeListBox::~SwIdxTreeListBox()
{
    disposeOnce();
}

void SwIdxTreeListBox::dispose()
{
    pParent.clear();
    SvTreeListBox::dispose();
}

VCL_BUILDER_FACTORY_CONSTRUCTOR(SwIdxTreeListBox, WB_TABSTOP)

void SwIdxTreeListBox::RequestHelp( const HelpEvent& rHEvt )
{
    if( rHEvt.GetMode() & HelpEventMode::QUICK )
    {
     Point aPos( ScreenToOutputPixel( rHEvt.GetMousePosPixel() ));
        SvTreeListEntry* pEntry = GetEntry( aPos );
        if( pEntry )
        {
            sal_uInt16 nLevel = static_cast< sal_uInt16 >(GetModel()->GetAbsPos(pEntry));
            OUString sEntry = pParent->GetLevelHelp(++nLevel);
            if (sEntry == "*")
                sEntry = GetEntryText(pEntry);
            if(!sEntry.isEmpty())
            {
                SvLBoxTab* pTab;
                SvLBoxItem* pItem = GetItem( pEntry, aPos.X(), &pTab );
                if (pItem && SvLBoxItemType::String == pItem->GetType())
                {
                    aPos = GetEntryPosition( pEntry );

                    aPos.setX( GetTabPos( pEntry, pTab ) );
                 Size aSize( pItem->GetSize( this, pEntry ) );

                    if((aPos.X() + aSize.Width()) > GetSizePixel().Width())
                        aSize.setWidth( GetSizePixel().Width() - aPos.X() );

                    aPos = OutputToScreenPixel(aPos);
                     tools::Rectangle aItemRect( aPos, aSize );
                    Help::ShowQuickHelp( this, aItemRect, sEntry,
                            QuickHelpFlags::Left|QuickHelpFlags::VCenter );
                }
            }
        }
    }
    else
        SvTreeListBox::RequestHelp(rHEvt);
}

namespace
{
    const char* STR_AUTH_FIELD_ARY[] =
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
        STR_AUTH_FIELD_ISBN
    };
}

SwTOXEntryTabPage::SwTOXEntryTabPage(vcl::Window* pParent, const SfxItemSet& rAttrSet)
    : SfxTabPage(pParent, "TocEntriesPage",
        "modules/swriter/ui/tocentriespage.ui", &rAttrSet)
    , sDelimStr(SwResId(STR_DELIM))
    , sNoCharStyle(SwResId(STR_NO_CHAR_STYLE))
    , sNoCharSortKey(SwResId(STR_NOSORTKEY))
    , m_pCurrentForm(nullptr)
    , bInLevelHdl(false)
{
    get(m_pLevelFT, "levelft");
    sAuthTypeStr = get<FixedText>("typeft")->GetText();
    get(m_pLevelLB, "level");
    m_pLevelLB->SetTabPage(this);
    get(m_pAllLevelsPB, "all");
    get(m_pEntryNoPB, "chapterno");
    get(m_pEntryPB, "entrytext");
    get(m_pTabPB, "tabstop");
    get(m_pChapterInfoPB, "chapterinfo");
    get(m_pPageNoPB, "pageno");
    get(m_pHyperLinkPB, "hyperlink");

    get(m_pAuthFieldsLB, "authfield");
    m_pAuthFieldsLB->SetStyle(m_pAuthFieldsLB->GetStyle() | WB_SORT);
    get(m_pAuthInsertPB, "insert");
    get(m_pAuthRemovePB, "remove");

    get(m_pCharStyleLB, "charstyle");
    get(m_pEditStylePB, "edit");

    get(m_pChapterEntryFT, "chapterentryft");
    get(m_pChapterEntryLB, "chapterentry");

    get(m_pNumberFormatFT, "numberformatft");
    get(m_pNumberFormatLB, "numberformat");

    get(m_pEntryOutlineLevelFT, "entryoutlinelevelft");
    get(m_pEntryOutlineLevelNF, "entryoutlinelevel");

    get(m_pFillCharFT, "fillcharft");
    get(m_pFillCharCB, "fillchar");

    get(m_pTabPosFT, "tabstopposft");
    get(m_pTabPosMF, "tabstoppos");
    get(m_pAutoRightCB, "alignright");

    get(m_pFormatFrame, "formatframe");
    get(m_pRelToStyleCB, "reltostyle");
    get(m_pMainEntryStyleFT, "mainstyleft");
    get(m_pMainEntryStyleLB, "mainstyle");
    get(m_pAlphaDelimCB, "alphadelim");
    get(m_pCommaSeparatedCB, "commasep");

    get(m_pSortingFrame, "sortingframe");
    get(m_pSortDocPosRB, "sortpos");
    get(m_pSortContentRB, "sortcontents");

    get(m_pSortKeyFrame, "sortkeyframe");
    get(m_pFirstKeyLB, "key1lb");
    get(m_pSecondKeyLB, "key2lb");
    get(m_pThirdKeyLB, "key3lb");
    get(m_pFirstSortUpRB, "up1cb");
    get(m_pSecondSortUpRB, "up2cb");
    get(m_pThirdSortUpRB, "up3cb");
    get(m_pFirstSortDownRB, "down1cb");
    get(m_pSecondSortDownRB, "down2cb");
    get(m_pThirdSortDownRB, "down3cb");

    get(m_pTokenWIN, "token");
    m_pTokenWIN->SetTabPage(this);

    sLevelStr = m_pLevelFT->GetText();
    m_pLevelLB->SetStyle( m_pLevelLB->GetStyle() | WB_HSCROLL );
    m_pLevelLB->SetSpaceBetweenEntries(0);
    m_pLevelLB->SetSelectionMode( SelectionMode::Single );
    m_pLevelLB->SetHighlightRange();   // select full width
    m_pLevelLB->Show();

    aLastTOXType.eType = TOXTypes(USHRT_MAX);
    aLastTOXType.nIndex = 0;

    SetExchangeSupport();
    m_pEntryNoPB->SetClickHdl(LINK(this, SwTOXEntryTabPage, InsertTokenHdl));
    m_pEntryPB->SetClickHdl(LINK(this, SwTOXEntryTabPage, InsertTokenHdl));
    m_pChapterInfoPB->SetClickHdl(LINK(this, SwTOXEntryTabPage, InsertTokenHdl));
    m_pPageNoPB->SetClickHdl(LINK(this, SwTOXEntryTabPage, InsertTokenHdl));
    m_pTabPB->SetClickHdl(LINK(this, SwTOXEntryTabPage, InsertTokenHdl));
    m_pHyperLinkPB->SetClickHdl(LINK(this, SwTOXEntryTabPage, InsertTokenHdl));
    m_pEditStylePB->SetClickHdl(LINK(this, SwTOXEntryTabPage, EditStyleHdl));
    m_pLevelLB->SetSelectHdl(LINK(this, SwTOXEntryTabPage, LevelHdl));
    m_pTokenWIN->SetButtonSelectedHdl(LINK(this, SwTOXEntryTabPage, TokenSelectedHdl));
    m_pTokenWIN->SetModifyHdl(LINK(this, SwTOXEntryTabPage, ModifyHdl));
    m_pCharStyleLB->SetSelectHdl(LINK(this, SwTOXEntryTabPage, StyleSelectHdl));
    m_pCharStyleLB->InsertEntry(sNoCharStyle);
    m_pChapterEntryLB->SetSelectHdl(LINK(this, SwTOXEntryTabPage, ChapterInfoHdl));
    m_pEntryOutlineLevelNF->SetModifyHdl(LINK(this, SwTOXEntryTabPage, ChapterInfoOutlineHdl));
    m_pNumberFormatLB->SetSelectHdl(LINK(this, SwTOXEntryTabPage, NumberFormatHdl));

    m_pTabPosMF->SetModifyHdl(LINK(this, SwTOXEntryTabPage, TabPosHdl));
    m_pFillCharCB->SetModifyHdl(LINK(this, SwTOXEntryTabPage, FillCharHdl));
    m_pAutoRightCB->SetClickHdl(LINK(this, SwTOXEntryTabPage, AutoRightHdl));
    m_pAuthInsertPB->SetClickHdl(LINK(this, SwTOXEntryTabPage, RemoveInsertAuthHdl));
    m_pAuthRemovePB->SetClickHdl(LINK(this, SwTOXEntryTabPage, RemoveInsertAuthHdl));
    m_pSortDocPosRB->SetClickHdl(LINK(this, SwTOXEntryTabPage, SortKeyHdl));
    m_pSortContentRB->SetClickHdl(LINK(this, SwTOXEntryTabPage, SortKeyHdl));
    m_pAllLevelsPB->SetClickHdl(LINK(this, SwTOXEntryTabPage, AllLevelsHdl));

    m_pAlphaDelimCB->SetClickHdl(LINK(this, SwTOXEntryTabPage, ModifyClickHdl));
    m_pCommaSeparatedCB->SetClickHdl(LINK(this, SwTOXEntryTabPage, ModifyClickHdl));
    m_pRelToStyleCB->SetClickHdl(LINK(this, SwTOXEntryTabPage, ModifyClickHdl));

    FieldUnit aMetric = ::GetDfltMetric(false);
    SetMetric(*m_pTabPosMF, aMetric);

    m_pSortDocPosRB->Check();

    m_pFillCharCB->SetMaxTextLen(1);
    m_pFillCharCB->InsertEntry(OUString(' '));
    m_pFillCharCB->InsertEntry(OUString('.'));
    m_pFillCharCB->InsertEntry(OUString('-'));
    m_pFillCharCB->InsertEntry(OUString('_'));
    m_pFillCharCB->InsertEntry(OUString(u'\x2024')); // ONE DOT LEADER
    m_pFillCharCB->InsertEntry(OUString(u'\x2025')); // TWO DOT LEADER
    m_pFillCharCB->InsertEntry(OUString(u'\x2026')); // HORIZONTAL ELLIPSIS

    m_pEditStylePB->Enable(false);

    //fill the types in
    for (sal_uInt16 i = 0; i < AUTH_FIELD_END; ++i)
    {
        sal_Int32 nPos = m_pAuthFieldsLB->InsertEntry(SwResId(STR_AUTH_FIELD_ARY[i]));
        m_pAuthFieldsLB->SetEntryData(nPos, reinterpret_cast< void * >(sal::static_int_cast< sal_uIntPtr >(i)));
    }
    sal_Int32 nPos = m_pFirstKeyLB->InsertEntry(sNoCharSortKey);
    m_pFirstKeyLB->SetEntryData(nPos, reinterpret_cast< void * >(sal::static_int_cast< sal_uIntPtr >(USHRT_MAX)));
    nPos = m_pSecondKeyLB->InsertEntry(sNoCharSortKey);
    m_pSecondKeyLB->SetEntryData(nPos, reinterpret_cast< void * >(sal::static_int_cast< sal_uIntPtr >(USHRT_MAX)));
    nPos = m_pThirdKeyLB->InsertEntry(sNoCharSortKey);
    m_pThirdKeyLB->SetEntryData(nPos, reinterpret_cast< void * >(sal::static_int_cast< sal_uIntPtr >(USHRT_MAX)));

    for (sal_uInt16 i = 0; i < AUTH_FIELD_END; ++i)
    {
        const OUString sTmp(m_pAuthFieldsLB->GetEntry(i));
        void* pEntryData = m_pAuthFieldsLB->GetEntryData(i);
        nPos = m_pFirstKeyLB->InsertEntry(sTmp);
        m_pFirstKeyLB->SetEntryData(nPos, pEntryData);
        nPos = m_pSecondKeyLB->InsertEntry(sTmp);
        m_pSecondKeyLB->SetEntryData(nPos, pEntryData);
        nPos = m_pThirdKeyLB->InsertEntry(sTmp);
        m_pThirdKeyLB->SetEntryData(nPos, pEntryData);
    }
    m_pFirstKeyLB->SelectEntryPos(0);
    m_pSecondKeyLB->SelectEntryPos(0);
    m_pThirdKeyLB->SelectEntryPos(0);
}

SwTOXEntryTabPage::~SwTOXEntryTabPage()
{
    disposeOnce();
}

void SwTOXEntryTabPage::dispose()
{
    m_pLevelFT.clear();
    m_pLevelLB.clear();
    m_pTokenWIN.clear();
    m_pAllLevelsPB.clear();
    m_pEntryNoPB.clear();
    m_pEntryPB.clear();
    m_pTabPB.clear();
    m_pChapterInfoPB.clear();
    m_pPageNoPB.clear();
    m_pHyperLinkPB.clear();
    m_pAuthFieldsLB.clear();
    m_pAuthInsertPB.clear();
    m_pAuthRemovePB.clear();
    m_pCharStyleLB.clear();
    m_pEditStylePB.clear();
    m_pChapterEntryFT.clear();
    m_pChapterEntryLB.clear();
    m_pNumberFormatFT.clear();
    m_pNumberFormatLB.clear();
    m_pEntryOutlineLevelFT.clear();
    m_pEntryOutlineLevelNF.clear();
    m_pFillCharFT.clear();
    m_pFillCharCB.clear();
    m_pTabPosFT.clear();
    m_pTabPosMF.clear();
    m_pAutoRightCB.clear();
    m_pFormatFrame.clear();
    m_pMainEntryStyleFT.clear();
    m_pMainEntryStyleLB.clear();
    m_pAlphaDelimCB.clear();
    m_pCommaSeparatedCB.clear();
    m_pRelToStyleCB.clear();
    m_pSortingFrame.clear();
    m_pSortDocPosRB.clear();
    m_pSortContentRB.clear();
    m_pSortKeyFrame.clear();
    m_pFirstKeyLB.clear();
    m_pFirstSortUpRB.clear();
    m_pFirstSortDownRB.clear();
    m_pSecondKeyLB.clear();
    m_pSecondSortUpRB.clear();
    m_pSecondSortDownRB.clear();
    m_pThirdKeyLB.clear();
    m_pThirdSortUpRB.clear();
    m_pThirdSortDownRB.clear();
    SfxTabPage::dispose();
}


// pVoid is used as signal to change all levels of the example
IMPL_LINK(SwTOXEntryTabPage, ModifyClickHdl, Button*, pVoid, void)
{
    OnModify(pVoid);
}
IMPL_LINK_NOARG(SwTOXEntryTabPage, ModifyHdl, LinkParamNone*, void)
{
    OnModify(nullptr);
}
void SwTOXEntryTabPage::OnModify(void const * pVoid)
{
    UpdateDescriptor();

    SwMultiTOXTabDialog* pTOXDlg = static_cast<SwMultiTOXTabDialog*>(GetTabDialog());
    if (pTOXDlg)
    {
        sal_uInt16 nCurLevel = static_cast< sal_uInt16 >(m_pLevelLB->GetModel()->GetAbsPos(m_pLevelLB->FirstSelected()) + 1);
        if(aLastTOXType.eType == TOX_CONTENT && pVoid)
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
    SwMultiTOXTabDialog* pTOXDlg = static_cast<SwMultiTOXTabDialog*>(GetTabDialog());
    const CurTOXType aCurType = pTOXDlg->GetCurrentTOXType();
    m_pCurrentForm = pTOXDlg->GetForm(aCurType);
    if(TOX_INDEX == aCurType.eType)
    {
        SwTOXDescription& rDesc = pTOXDlg->GetTOXDescription(aCurType);
        const OUString& sMainEntryCharStyle = rDesc.GetMainEntryCharStyle();
        if(!sMainEntryCharStyle.isEmpty())
        {
            if( LISTBOX_ENTRY_NOTFOUND ==
                    m_pMainEntryStyleLB->GetEntryPos(sMainEntryCharStyle))
                m_pMainEntryStyleLB->InsertEntry(
                        sMainEntryCharStyle);
            m_pMainEntryStyleLB->SelectEntry(sMainEntryCharStyle);
        }
        else
            m_pMainEntryStyleLB->SelectEntry(sNoCharStyle);
        m_pAlphaDelimCB->Check( bool(rDesc.GetIndexOptions() & SwTOIOptions::AlphaDelimiter) );
    }
    m_pRelToStyleCB->Check(m_pCurrentForm->IsRelTabPos());
    m_pCommaSeparatedCB->Check(m_pCurrentForm->IsCommaSeparated());
}

void SwTOXEntryTabPage::ActivatePage( const SfxItemSet& /*rSet*/)
{
    SwMultiTOXTabDialog* pTOXDlg = static_cast<SwMultiTOXTabDialog*>(GetTabDialog());
    const CurTOXType aCurType = pTOXDlg->GetCurrentTOXType();

    m_pCurrentForm = pTOXDlg->GetForm(aCurType);
    if( !( aLastTOXType == aCurType ))
    {
        bool bToxIsAuthorities = TOX_AUTHORITIES == aCurType.eType;
        bool bToxIsIndex =       TOX_INDEX == aCurType.eType;
        bool bToxIsContent =     TOX_CONTENT == aCurType.eType;
        bool bToxSupportsLinks = TOX_CONTENT == aCurType.eType ||
                                 TOX_ILLUSTRATIONS == aCurType.eType ||
                                 TOX_TABLES == aCurType.eType ||
                                 TOX_OBJECTS == aCurType.eType ||
                                 TOX_USER == aCurType.eType;

        m_pLevelLB->Clear();
        for(sal_uInt16 i = 1; i < m_pCurrentForm->GetFormMax(); i++)
        {
            if(bToxIsAuthorities)
                m_pLevelLB->InsertEntry( SwAuthorityFieldType::GetAuthTypeName(
                                            static_cast<ToxAuthorityType>(i - 1)) );
            else if( bToxIsIndex )
            {
                if(i == 1)
                    m_pLevelLB->InsertEntry( sDelimStr );
                else
                    m_pLevelLB->InsertEntry( OUString::number(i - 1) );
            }
            else
                m_pLevelLB->InsertEntry(OUString::number(i));
        }
        if(bToxIsAuthorities)
        {
            SwWrtShell& rSh = pTOXDlg->GetWrtShell();
            const SwAuthorityFieldType* pFType = static_cast<const SwAuthorityFieldType*>(
                                    rSh.GetFieldType(SwFieldIds::TableOfAuthorities, aEmptyOUStr));
            if(pFType)
            {
                if(pFType->IsSortByDocument())
                    m_pSortDocPosRB->Check();
                else
                {
                    m_pSortContentRB->Check();
                    const sal_uInt16 nKeyCount = pFType->GetSortKeyCount();
                    if(0 < nKeyCount)
                    {
                        const SwTOXSortKey* pKey = pFType->GetSortKey(0);
                        m_pFirstKeyLB->SelectEntryPos(
                            m_pFirstKeyLB->GetEntryPos(reinterpret_cast<void*>(static_cast<sal_uIntPtr>(pKey->eField))));
                        m_pFirstSortUpRB->Check(pKey->bSortAscending);
                        m_pFirstSortDownRB->Check(!pKey->bSortAscending);
                    }
                    if(1 < nKeyCount)
                    {
                        const SwTOXSortKey* pKey = pFType->GetSortKey(1);
                        m_pSecondKeyLB->SelectEntryPos(
                            m_pSecondKeyLB->GetEntryPos(reinterpret_cast<void*>(static_cast<sal_uIntPtr>(pKey->eField))));
                        m_pSecondSortUpRB->Check(pKey->bSortAscending);
                        m_pSecondSortDownRB->Check(!pKey->bSortAscending);
                    }
                    if(2 < nKeyCount)
                    {
                        const SwTOXSortKey* pKey = pFType->GetSortKey(2);
                        m_pThirdKeyLB->SelectEntryPos(
                            m_pThirdKeyLB->GetEntryPos(reinterpret_cast<void*>(static_cast<sal_uIntPtr>(pKey->eField))));
                        m_pThirdSortUpRB->Check(pKey->bSortAscending);
                        m_pThirdSortDownRB->Check(!pKey->bSortAscending);
                    }
                }
            }
            SortKeyHdl(m_pSortDocPosRB->IsChecked() ? m_pSortDocPosRB : m_pSortContentRB);
            m_pLevelFT->SetText(sAuthTypeStr);
        }
        else
            m_pLevelFT->SetText(sLevelStr);

        Link<SvTreeListBox*,void> aLink = m_pLevelLB->GetSelectHdl();
        m_pLevelLB->SetSelectHdl(Link<SvTreeListBox*,void>());
        m_pLevelLB->Select( m_pLevelLB->GetEntry( bToxIsIndex ? 1 : 0 ) );
        m_pLevelLB->SetSelectHdl(aLink);

        //show or hide controls
        m_pEntryNoPB->Show(bToxIsContent);
        m_pHyperLinkPB->Show(bToxSupportsLinks);
        m_pRelToStyleCB->Show(!bToxIsAuthorities);
        m_pChapterInfoPB->Show(!bToxIsContent && !bToxIsAuthorities);
        m_pEntryPB->Show(!bToxIsAuthorities);
        m_pPageNoPB->Show(!bToxIsAuthorities);
        m_pAuthFieldsLB->Show(bToxIsAuthorities);
        m_pAuthInsertPB->Show(bToxIsAuthorities);
        m_pAuthRemovePB->Show(bToxIsAuthorities);

        m_pFormatFrame->Show(!bToxIsAuthorities);

        m_pSortingFrame->Show(bToxIsAuthorities);
        m_pSortKeyFrame->Show(bToxIsAuthorities);

        m_pMainEntryStyleFT->Show(bToxIsIndex);
        m_pMainEntryStyleLB->Show(bToxIsIndex);
        m_pAlphaDelimCB->Show(bToxIsIndex);
        m_pCommaSeparatedCB->Show(bToxIsIndex);
    }
    aLastTOXType = aCurType;

    //invalidate PatternWindow
    m_pTokenWIN->SetInvalid();
    LevelHdl(m_pLevelLB);
}

void SwTOXEntryTabPage::UpdateDescriptor()
{
    WriteBackLevel();
    SwMultiTOXTabDialog* pTOXDlg = static_cast<SwMultiTOXTabDialog*>(GetTabDialog());
    SwTOXDescription& rDesc = pTOXDlg->GetTOXDescription(aLastTOXType);
    if(TOX_INDEX == aLastTOXType.eType)
    {
        const OUString sTemp(m_pMainEntryStyleLB->GetSelectedEntry());
        rDesc.SetMainEntryCharStyle(sNoCharStyle == sTemp ? aEmptyOUStr : sTemp);
        SwTOIOptions nIdxOptions = rDesc.GetIndexOptions() & ~SwTOIOptions::AlphaDelimiter;
        if(m_pAlphaDelimCB->IsChecked())
            nIdxOptions |= SwTOIOptions::AlphaDelimiter;
        rDesc.SetIndexOptions(nIdxOptions);
    }
    else if(TOX_AUTHORITIES == aLastTOXType.eType)
    {
        rDesc.SetSortByDocument(m_pSortDocPosRB->IsChecked());
        SwTOXSortKey aKey1, aKey2, aKey3;
        aKey1.eField = static_cast<ToxAuthorityField>(reinterpret_cast<sal_uIntPtr>(m_pFirstKeyLB->GetSelectedEntryData()));
        aKey1.bSortAscending = m_pFirstSortUpRB->IsChecked();
        aKey2.eField = static_cast<ToxAuthorityField>(reinterpret_cast<sal_uIntPtr>(m_pSecondKeyLB->GetSelectedEntryData()));
        aKey2.bSortAscending = m_pSecondSortUpRB->IsChecked();
        aKey3.eField = static_cast<ToxAuthorityField>(reinterpret_cast<sal_uIntPtr>(m_pThirdKeyLB->GetSelectedEntryData()));
        aKey3.bSortAscending = m_pThirdSortUpRB->IsChecked();

        rDesc.SetSortKeys(aKey1, aKey2, aKey3);
    }
    SwForm* pCurrentForm = pTOXDlg->GetForm(aLastTOXType);
    if (m_pRelToStyleCB->IsVisible())
        pCurrentForm->SetRelTabPos(m_pRelToStyleCB->IsChecked());
    if (m_pCommaSeparatedCB->IsVisible())
        pCurrentForm->SetCommaSeparated(m_pCommaSeparatedCB->IsChecked());
}

DeactivateRC SwTOXEntryTabPage::DeactivatePage( SfxItemSet* /*pSet*/)
{
    UpdateDescriptor();
    return DeactivateRC::LeavePage;
}

VclPtr<SfxTabPage> SwTOXEntryTabPage::Create( TabPageParent pParent,     const SfxItemSet* rAttrSet)
{
    return VclPtr<SwTOXEntryTabPage>::Create(pParent.pParent, *rAttrSet);
}

IMPL_LINK_NOARG(SwTOXEntryTabPage, EditStyleHdl, Button*, void)
{
    if( LISTBOX_ENTRY_NOTFOUND != m_pCharStyleLB->GetSelectedEntryPos())
    {
        SfxStringItem aStyle(SID_STYLE_EDIT, m_pCharStyleLB->GetSelectedEntry());
        SfxUInt16Item aFamily(SID_STYLE_FAMILY, sal_uInt16(SfxStyleFamily::Char));
        static_cast<SwMultiTOXTabDialog*>(GetTabDialog())->GetWrtShell().
        GetView().GetViewFrame()->GetDispatcher()->ExecuteList(SID_STYLE_EDIT,
                SfxCallMode::SYNCHRON|SfxCallMode::MODAL,
                { &aStyle, &aFamily });
    }
}

IMPL_LINK(SwTOXEntryTabPage, RemoveInsertAuthHdl, Button*, pButton, void)
{
    bool bInsert = pButton == m_pAuthInsertPB;
    if(bInsert)
    {
        sal_Int32 nSelPos = m_pAuthFieldsLB->GetSelectedEntryPos();
        const OUString sToInsert(m_pAuthFieldsLB->GetSelectedEntry());
        SwFormToken aInsert(TOKEN_AUTHORITY);
        aInsert.nAuthorityField = static_cast<sal_uInt16>(reinterpret_cast<sal_uIntPtr>(m_pAuthFieldsLB->GetEntryData(nSelPos)));
        m_pTokenWIN->InsertAtSelection(SwForm::GetFormAuth(), aInsert);
        m_pAuthFieldsLB->RemoveEntry(sToInsert);
        m_pAuthFieldsLB->SelectEntryPos( nSelPos ? nSelPos - 1 : 0);
    }
    else
    {
        Control* pCtrl = m_pTokenWIN->GetActiveControl();
        OSL_ENSURE(WindowType::EDIT != pCtrl->GetType(), "Remove should be disabled");
        if( WindowType::EDIT != pCtrl->GetType() )
        {
            //fill it into the ListBox
            const SwFormToken& rToken = static_cast<SwTOXButton*>(pCtrl)->GetFormToken();
            PreTokenButtonRemoved(rToken);
            m_pTokenWIN->RemoveControl(static_cast<SwTOXButton*>(pCtrl));
        }
    }
    ModifyHdl(nullptr);
}

void SwTOXEntryTabPage::PreTokenButtonRemoved(const SwFormToken& rToken)
{
    //fill it into the ListBox
    sal_uInt32 nData = rToken.nAuthorityField;
    sal_Int32 nPos = m_pAuthFieldsLB->InsertEntry(SwResId(STR_AUTH_FIELD_ARY[nData]));
    m_pAuthFieldsLB->SetEntryData(nPos, reinterpret_cast<void*>(static_cast<sal_uIntPtr>(nData)));
}

void SwTOXEntryTabPage::SetFocus2theAllBtn()
{
    m_pAllLevelsPB->GrabFocus();
}

bool SwTOXEntryTabPage::EventNotify( NotifyEvent& rNEvt )
{
    if ( rNEvt.GetType() == MouseNotifyEvent::KEYINPUT )
    {
        const KeyEvent& rKEvt = *rNEvt.GetKeyEvent();
        vcl::KeyCode aCode = rKEvt.GetKeyCode();
        if ( (aCode.GetCode() == KEY_F4) && aCode.IsShift() && !aCode.IsMod1() && !aCode.IsMod2() )
        {
            if ( m_pTokenWIN->GetActiveControl() )
            {
                m_pTokenWIN->GetActiveControl()->GrabFocus();
            }
        }

    }

    return SfxTabPage::EventNotify(rNEvt);
}

// This function initializes the default value in the Token
// put here the UI dependent initializations
IMPL_LINK(SwTOXEntryTabPage, InsertTokenHdl, Button*, pBtn, void)
{
    OUString sText;
    FormTokenType eTokenType = TOKEN_ENTRY_NO;
    OUString sCharStyle;
    sal_uInt16  nChapterFormat = CF_NUMBER; // i89791
    if(pBtn == m_pEntryNoPB)
    {
        sText = SwForm::GetFormEntryNum();
        eTokenType = TOKEN_ENTRY_NO;
    }
    else if(pBtn == m_pEntryPB)
    {
        if( TOX_CONTENT == m_pCurrentForm->GetTOXType() )
        {
            sText = SwForm::GetFormEntryText();
            eTokenType = TOKEN_ENTRY_TEXT;
        }
        else
        {
            sText = SwForm::GetFormEntry();
            eTokenType = TOKEN_ENTRY;
        }
    }
    else if(pBtn == m_pChapterInfoPB)
    {
        sText = SwForm::GetFormChapterMark();
        eTokenType = TOKEN_CHAPTER_INFO;
        nChapterFormat = CF_NUM_NOPREPST_TITLE; // i89791
    }
    else if(pBtn == m_pPageNoPB)
    {
        sText = SwForm::GetFormPageNums();
        eTokenType = TOKEN_PAGE_NUMS;
    }
    else if(pBtn == m_pHyperLinkPB)
    {
        sText = SwForm::GetFormLinkStt();
        eTokenType = TOKEN_LINK_START;
        sCharStyle = SwResId(STR_POOLCHR_TOXJUMP);
    }
    else if(pBtn == m_pTabPB)
    {
        sText = SwForm::GetFormTab();
        eTokenType = TOKEN_TAB_STOP;
    }
    SwFormToken aInsert(eTokenType);
    aInsert.sCharStyleName = sCharStyle;
    aInsert.nTabStopPosition = 0;
    aInsert.nChapterFormat = nChapterFormat; // i89791
    m_pTokenWIN->InsertAtSelection(sText, aInsert);
    ModifyHdl(nullptr);
}

IMPL_LINK_NOARG(SwTOXEntryTabPage, AllLevelsHdl, Button*, void)
{
    //get current level
    //write it into all levels
    if(m_pTokenWIN->IsValid())
    {
        const OUString sNewToken = m_pTokenWIN->GetPattern();
        for(sal_uInt16 i = 1; i < m_pCurrentForm->GetFormMax(); i++)
            m_pCurrentForm->SetPattern(i, sNewToken);

        OnModify(this);
    }
}

void SwTOXEntryTabPage::WriteBackLevel()
{
    if(m_pTokenWIN->IsValid())
    {
        const OUString sNewToken = m_pTokenWIN->GetPattern();
        const sal_uInt16 nLastLevel = m_pTokenWIN->GetLastLevel();
        if(nLastLevel != USHRT_MAX)
            m_pCurrentForm->SetPattern(nLastLevel + 1, sNewToken);
    }
}

IMPL_LINK(SwTOXEntryTabPage, LevelHdl, SvTreeListBox*, pBox, void)
{
    if(bInLevelHdl)
        return;
    bInLevelHdl = true;
    WriteBackLevel();

    const sal_uInt16 nLevel = static_cast< sal_uInt16 >(
        pBox->GetModel()->GetAbsPos(pBox->FirstSelected()));
    m_pTokenWIN->SetForm(*m_pCurrentForm, nLevel);
    if(TOX_AUTHORITIES == m_pCurrentForm->GetTOXType())
    {
        //fill the types in
        m_pAuthFieldsLB->Clear();
        for( sal_uInt32 i = 0; i < AUTH_FIELD_END; i++)
        {
            sal_Int32 nPos = m_pAuthFieldsLB->InsertEntry(SwResId(STR_AUTH_FIELD_ARY[i]));
            m_pAuthFieldsLB->SetEntryData(nPos, reinterpret_cast<void*>(static_cast<sal_uIntPtr>(i)));
        }

        // #i21237#
        SwFormTokens aPattern = m_pCurrentForm->GetPattern(nLevel + 1);

        for(const auto& aToken : aPattern)
        {
            if(TOKEN_AUTHORITY == aToken.eTokenType)
            {
                sal_uInt32 nSearch = aToken.nAuthorityField;
                sal_Int32  nLstBoxPos = m_pAuthFieldsLB->GetEntryPos( reinterpret_cast<void*>(static_cast<sal_uIntPtr>(nSearch)) );
                OSL_ENSURE(LISTBOX_ENTRY_NOTFOUND != nLstBoxPos, "Entry not found?");
                m_pAuthFieldsLB->RemoveEntry(nLstBoxPos);
            }
        }
        m_pAuthFieldsLB->SelectEntryPos(0);
    }
    bInLevelHdl = false;
    pBox->GrabFocus();
}

IMPL_LINK(SwTOXEntryTabPage, SortKeyHdl, Button*, pButton, void)
{
    bool bEnable = m_pSortContentRB == pButton;
    m_pSortKeyFrame->Enable(bEnable);
}

IMPL_LINK(SwTOXEntryTabPage, TokenSelectedHdl, SwFormToken&, rToken, void)
{
    if (!rToken.sCharStyleName.isEmpty())
        m_pCharStyleLB->SelectEntry(rToken.sCharStyleName);
    else
        m_pCharStyleLB->SelectEntry(sNoCharStyle);

    const OUString sEntry = m_pCharStyleLB->GetSelectedEntry();
    m_pEditStylePB->Enable(sEntry != sNoCharStyle);

    if(rToken.eTokenType == TOKEN_CHAPTER_INFO)
    {
//---> i89791
        switch(rToken.nChapterFormat)
        {
        default:
            m_pChapterEntryLB->SetNoSelection();//to alert the user
            break;
        case CF_NUM_NOPREPST_TITLE:
            m_pChapterEntryLB->SelectEntryPos(0);
            break;
        case CF_TITLE:
            m_pChapterEntryLB->SelectEntryPos(1);
           break;
        case CF_NUMBER_NOPREPST:
            m_pChapterEntryLB->SelectEntryPos(2);
            break;
        }
//i53420

        m_pEntryOutlineLevelNF->SetValue(rToken.nOutlineLevel);
    }

//i53420
    if(rToken.eTokenType == TOKEN_ENTRY_NO)
    {
        m_pEntryOutlineLevelNF->SetValue(rToken.nOutlineLevel);
        const sal_uInt16 nFormat =
            rToken.nChapterFormat == CF_NUM_NOPREPST_TITLE ? 1 : 0;
        m_pNumberFormatLB->SelectEntryPos(nFormat);
    }

    bool bTabStop = TOKEN_TAB_STOP == rToken.eTokenType;
    m_pFillCharFT->Show(bTabStop);
    m_pFillCharCB->Show(bTabStop);
    m_pTabPosFT->Show(bTabStop);
    m_pTabPosMF->Show(bTabStop);
    m_pAutoRightCB->Show(bTabStop);
    m_pAutoRightCB->Enable(bTabStop);
    if(bTabStop)
    {
        m_pTabPosMF->SetValue(m_pTabPosMF->Normalize(rToken.nTabStopPosition), FieldUnit::TWIP);
        m_pAutoRightCB->Check(SvxTabAdjust::End == rToken.eTabAlign);
        m_pFillCharCB->SetText(OUString(rToken.cTabFillChar));
        m_pTabPosFT->Enable(!m_pAutoRightCB->IsChecked());
        m_pTabPosMF->Enable(!m_pAutoRightCB->IsChecked());
    }
    else
    {
        m_pTabPosMF->Enable(false);
    }

    bool bIsChapterInfo = rToken.eTokenType == TOKEN_CHAPTER_INFO;
    bool bIsEntryNumber = rToken.eTokenType == TOKEN_ENTRY_NO;
    m_pChapterEntryFT->Show( bIsChapterInfo );
    m_pChapterEntryLB->Show( bIsChapterInfo );
    m_pEntryOutlineLevelFT->Show( bIsChapterInfo || bIsEntryNumber );
    m_pEntryOutlineLevelNF->Show( bIsChapterInfo || bIsEntryNumber );
    m_pNumberFormatFT->Show( bIsEntryNumber );
    m_pNumberFormatLB->Show( bIsEntryNumber );

    //now enable the visible buttons
    //- inserting the same type of control is not allowed
    //- some types of controls can only appear once (EntryText EntryNumber)

    if(m_pEntryNoPB->IsVisible())
    {
        m_pEntryNoPB->Enable(TOKEN_ENTRY_NO != rToken.eTokenType );
    }
    if(m_pEntryPB->IsVisible())
    {
        m_pEntryPB->Enable(TOKEN_ENTRY_TEXT != rToken.eTokenType &&
                                !m_pTokenWIN->Contains(TOKEN_ENTRY_TEXT)
                                && !m_pTokenWIN->Contains(TOKEN_ENTRY));
    }

    if(m_pChapterInfoPB->IsVisible())
    {
        m_pChapterInfoPB->Enable(TOKEN_CHAPTER_INFO != rToken.eTokenType);
    }
    if(m_pPageNoPB->IsVisible())
    {
        m_pPageNoPB->Enable(TOKEN_PAGE_NUMS != rToken.eTokenType &&
                                !m_pTokenWIN->Contains(TOKEN_PAGE_NUMS));
    }
    if(m_pTabPB->IsVisible())
    {
        m_pTabPB->Enable(!bTabStop);
    }
    if(m_pHyperLinkPB->IsVisible())
    {
        m_pHyperLinkPB->Enable(TOKEN_LINK_START != rToken.eTokenType &&
                            TOKEN_LINK_END != rToken.eTokenType);
    }
    //table of authorities
    if(m_pAuthInsertPB->IsVisible())
    {
        bool bText = TOKEN_TEXT == rToken.eTokenType;
        m_pAuthInsertPB->Enable(bText && !m_pAuthFieldsLB->GetSelectedEntry().isEmpty());
        m_pAuthRemovePB->Enable(!bText);
    }
}

IMPL_LINK(SwTOXEntryTabPage, StyleSelectHdl, ListBox&, rBox, void)
{
    OUString sEntry = rBox.GetSelectedEntry();
    const sal_uInt16 nId = static_cast<sal_uInt16>(reinterpret_cast<sal_IntPtr>(rBox.GetSelectedEntryData()));
    const bool bEqualsNoCharStyle = sEntry == sNoCharStyle;
    m_pEditStylePB->Enable(!bEqualsNoCharStyle);
    if (bEqualsNoCharStyle)
        sEntry.clear();
    Control* pCtrl = m_pTokenWIN->GetActiveControl();
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

IMPL_LINK(SwTOXEntryTabPage, ChapterInfoHdl, ListBox&, rBox, void)
{
    sal_Int32 nPos = rBox.GetSelectedEntryPos();
    if(LISTBOX_ENTRY_NOTFOUND != nPos)
    {
        Control* pCtrl = m_pTokenWIN->GetActiveControl();
        OSL_ENSURE(pCtrl, "no active control?");
        if(pCtrl && WindowType::EDIT != pCtrl->GetType())
            static_cast<SwTOXButton*>(pCtrl)->SetChapterInfo(nPos);

        ModifyHdl(nullptr);
    }
}

IMPL_LINK(SwTOXEntryTabPage, ChapterInfoOutlineHdl, Edit&, rEdit, void)
{
    const sal_uInt16 nLevel = static_cast<sal_uInt8>(static_cast<NumericField&>(rEdit).GetValue());

    Control* pCtrl = m_pTokenWIN->GetActiveControl();
    OSL_ENSURE(pCtrl, "no active control?");
    if(pCtrl && WindowType::EDIT != pCtrl->GetType())
        static_cast<SwTOXButton*>(pCtrl)->SetOutlineLevel(nLevel);

    ModifyHdl(nullptr);
}

IMPL_LINK(SwTOXEntryTabPage, NumberFormatHdl, ListBox&, rBox, void)
{
    const sal_Int32 nPos = rBox.GetSelectedEntryPos();

    if(LISTBOX_ENTRY_NOTFOUND != nPos)
    {
        Control* pCtrl = m_pTokenWIN->GetActiveControl();
        OSL_ENSURE(pCtrl, "no active control?");
        if(pCtrl && WindowType::EDIT != pCtrl->GetType())
        {
           static_cast<SwTOXButton*>(pCtrl)->SetEntryNumberFormat(nPos);//i89791
        }
        ModifyHdl(nullptr);
    }
}

IMPL_LINK(SwTOXEntryTabPage, TabPosHdl, Edit&, rEdit, void)
{
    MetricField* pField = static_cast<MetricField*>(&rEdit);
    Control* pCtrl = m_pTokenWIN->GetActiveControl();
    OSL_ENSURE(pCtrl && WindowType::EDIT != pCtrl->GetType() &&
        TOKEN_TAB_STOP == static_cast<SwTOXButton*>(pCtrl)->GetFormToken().eTokenType,
                "no active style::TabStop control?");
    if( pCtrl && WindowType::EDIT != pCtrl->GetType() )
    {
        static_cast<SwTOXButton*>(pCtrl)->SetTabPosition( static_cast< SwTwips >(
                pField->Denormalize( pField->GetValue( FieldUnit::TWIP ))));
    }
    ModifyHdl(nullptr);
}

IMPL_LINK(SwTOXEntryTabPage, FillCharHdl, Edit&, rBox, void)
{
    Control* pCtrl = m_pTokenWIN->GetActiveControl();
    OSL_ENSURE(pCtrl && WindowType::EDIT != pCtrl->GetType() &&
        TOKEN_TAB_STOP == static_cast<SwTOXButton*>(pCtrl)->GetFormToken().eTokenType,
                "no active style::TabStop control?");
    if(pCtrl && WindowType::EDIT != pCtrl->GetType())
    {
        sal_Unicode cSet;
        if( !rBox.GetText().isEmpty() )
            cSet = rBox.GetText()[0];
        else
            cSet = ' ';
        static_cast<SwTOXButton*>(pCtrl)->SetFillChar( cSet );
    }
    ModifyHdl(nullptr);
}

IMPL_LINK(SwTOXEntryTabPage, AutoRightHdl, Button*, pBox, void)
{
    //the most right style::TabStop is usually right aligned
    Control* pCurCtrl = m_pTokenWIN->GetActiveControl();
    OSL_ENSURE(WindowType::EDIT != pCurCtrl->GetType() &&
            static_cast<SwTOXButton*>(pCurCtrl)->GetFormToken().eTokenType == TOKEN_TAB_STOP,
            "no style::TabStop selected!");

    const SwFormToken& rToken = static_cast<SwTOXButton*>(pCurCtrl)->GetFormToken();
    bool bChecked = static_cast<CheckBox*>(pBox)->IsChecked();
    if(rToken.eTokenType == TOKEN_TAB_STOP)
        static_cast<SwTOXButton*>(pCurCtrl)->SetTabAlign(
            bChecked ? SvxTabAdjust::End : SvxTabAdjust::Left);
    m_pTabPosFT->Enable(!bChecked);
    m_pTabPosMF->Enable(!bChecked);
    ModifyHdl(nullptr);
}

void SwTOXEntryTabPage::SetWrtShell(SwWrtShell& rSh)
{
    SwDocShell* pDocSh = rSh.GetView().GetDocShell();
    ::FillCharStyleListBox(*m_pCharStyleLB, pDocSh, true, true);
    const OUString sDefault(SwResId(STR_POOLCOLL_STANDARD));
    for(sal_Int32 i = 0; i < m_pCharStyleLB->GetEntryCount(); i++)
    {
        const OUString sEntry = m_pCharStyleLB->GetEntry(i);
        if(sDefault != sEntry)
        {
            m_pMainEntryStyleLB->InsertEntry( sEntry );
            m_pMainEntryStyleLB->SetEntryData(i, m_pCharStyleLB->GetEntryData(i));
        }
    }
    m_pMainEntryStyleLB->SelectEntry( SwStyleNameMapper::GetUIName(
                                RES_POOLCHR_IDX_MAIN_ENTRY, aEmptyOUStr ));
}

OUString SwTOXEntryTabPage::GetLevelHelp(sal_uInt16 nLevel) const
{
    OUString sRet;
    SwMultiTOXTabDialog* pTOXDlg = static_cast<SwMultiTOXTabDialog*>(GetTabDialog());
    const CurTOXType aCurType = pTOXDlg->GetCurrentTOXType();
    if( TOX_INDEX == aCurType.eType )
        SwStyleNameMapper::FillUIName( static_cast< sal_uInt16 >(1 == nLevel ? RES_POOLCOLL_TOX_IDXBREAK
                                  : RES_POOLCOLL_TOX_IDX1 + nLevel-2), sRet );

    else if( TOX_AUTHORITIES == aCurType.eType )
    {
        //wildcard -> show entry text
        sRet = "*";
    }
    return sRet;
}

static const char* STR_TOKEN_ARY[] =
{
    STR_TOKEN_ENTRY_NO,
    STR_TOKEN_ENTRY,    //mapped from original STR_TOKEN_ENTRY_TEXT,
    STR_TOKEN_ENTRY,
    STR_TOKEN_TAB_STOP,
    nullptr,
    STR_TOKEN_PAGE_NUMS,
    STR_TOKEN_CHAPTER_INFO,
    STR_TOKEN_LINK_START,
    STR_TOKEN_LINK_END,
    STR_TOKEN_AUTHORITY
};

static const char* STR_TOKEN_HELP_ARY[] =
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

SwTokenWindow::SwTokenWindow(vcl::Window* pParent)
    : VclHBox(pParent)
    , m_pForm(nullptr)
    , m_nLevel(0)
    , m_bValid(false)
    , m_sCharStyle(SwResId(STR_CHARSTYLE))
    , m_pActiveCtrl(nullptr)
    , m_pParent(nullptr)
{
    m_pUIBuilder.reset(new VclBuilder(this, getUIRootDir(),
        "modules/swriter/ui/tokenwidget.ui", "TokenWidget"));
    get(m_pLeftScrollWin, "left");
    get(m_pCtrlParentWin, "ctrl");
    m_pCtrlParentWin->set_height_request(Edit::GetMinimumEditSize().Height());
    get(m_pRightScrollWin, "right");

    for (sal_uInt32 i = 0; i < TOKEN_END; ++i)
    {
        const char* pTextId = STR_TOKEN_ARY[i];
        if (pTextId)
            m_aButtonTexts[i] = SwResId(pTextId);

        const char* pHelpId = STR_TOKEN_HELP_ARY[i];
        m_aButtonHelpTexts[i] = SwResId(pHelpId);
    }

    m_sAccessibleName = SwResId(STR_STRUCTURE);
    m_sAdditionalAccnameString1 = SwResId(STR_ADDITIONAL_ACCNAME_STRING1);
    m_sAdditionalAccnameString2 = SwResId(STR_ADDITIONAL_ACCNAME_STRING2);
    m_sAdditionalAccnameString3 = SwResId(STR_ADDITIONAL_ACCNAME_STRING3);

    Link<Button*,void> aLink(LINK(this, SwTokenWindow, ScrollHdl));
    m_pLeftScrollWin->SetClickHdl(aLink);
    m_pRightScrollWin->SetClickHdl(aLink);
}

VCL_BUILDER_FACTORY(SwTokenWindow)

void SwTokenWindow::setAllocation(const Size &rAllocation)
{
    VclHBox::setAllocation(rAllocation);

    if (m_aControlList.empty())
        return;

    Size aControlSize(m_pCtrlParentWin->GetSizePixel());
    for (VclPtr<Control> const & pControl : m_aControlList)
    {
        Size aSize(pControl->GetSizePixel());
        aSize.setHeight( aControlSize.Height() );
        pControl->SetSizePixel(aSize);
    }
    AdjustPositions();
}

SwTokenWindow::~SwTokenWindow()
{
    disposeOnce();
}

void SwTokenWindow::dispose()
{
    for (VclPtr<Control> & pControl : m_aControlList)
    {
        pControl->SetGetFocusHdl( Link<Control&,void>() );
        pControl->SetLoseFocusHdl( Link<Control&,void>() );
        pControl.disposeAndClear();
    }
    m_aControlList.clear();
    disposeBuilder();
    m_pLeftScrollWin.clear();
    m_pCtrlParentWin.clear();
    m_pRightScrollWin.clear();
    m_pActiveCtrl.clear();
    m_pParent.clear();
    VclHBox::dispose();
}

void SwTokenWindow::SetForm(SwForm& rForm, sal_uInt16 nL)
{
    SetActiveControl(nullptr);
    m_bValid = true;

    if(m_pForm)
    {
        //apply current level settings to the form
        for (auto& aControl : m_aControlList)
            aControl.disposeAndClear();
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

        Control* pSetActiveControl = nullptr;
        for (const auto& aToken : aPattern) // #i21237#
        {
            if(TOKEN_TEXT == aToken.eTokenType)
            {
                SAL_WARN_IF(bLastWasText, "sw", "text following text is invalid");
                Control* pCtrl = InsertItem(aToken.sText, aToken);
                bLastWasText = true;
                if(!GetActiveControl())
                    SetActiveControl(pCtrl);
            }
            else
            {
                if( !bLastWasText )
                {
                    SwFormToken aTemp(TOKEN_TEXT);
                    Control* pCtrl = InsertItem(aEmptyOUStr, aTemp);
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
            Control* pCtrl = InsertItem(aEmptyOUStr, aTemp);
            if(!pSetActiveControl)
                pSetActiveControl = pCtrl;
        }
        SetActiveControl(pSetActiveControl);
    }
    AdjustScrolling();
}

void SwTokenWindow::SetActiveControl(Control* pSet)
{
    if( pSet != m_pActiveCtrl )
    {
        m_pActiveCtrl = pSet;
        if( m_pActiveCtrl )
        {
            m_pActiveCtrl->GrabFocus();
            //it must be a SwTOXEdit
            const SwFormToken* pFToken;
            if( WindowType::EDIT == m_pActiveCtrl->GetType() )
                pFToken = &static_cast<SwTOXEdit*>(m_pActiveCtrl.get())->GetFormToken();
            else
                pFToken = &static_cast<SwTOXButton*>(m_pActiveCtrl.get())->GetFormToken();

            SwFormToken aTemp( *pFToken );
            m_aButtonSelectedHdl.Call( aTemp );
        }
    }
}

Control*    SwTokenWindow::InsertItem(const OUString& rText, const SwFormToken& rToken)
{
    Control* pRet = nullptr;
    Size aControlSize(m_pCtrlParentWin->GetSizePixel());
    Point aControlPos;

    if(!m_aControlList.empty())
    {
        Control* pLast = m_aControlList.rbegin()->get();

        aControlSize = pLast->GetSizePixel();
        aControlPos = pLast->GetPosPixel();
        aControlPos.AdjustX(aControlSize.Width() );
    }

    if(TOKEN_TEXT == rToken.eTokenType)
    {
        VclPtr<SwTOXEdit> pEdit = VclPtr<SwTOXEdit>::Create(m_pCtrlParentWin, this, rToken);
        pEdit->SetPosPixel(aControlPos);

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
        Size aEditSize(aControlSize);
        aEditSize.setWidth( pEdit->GetTextWidth(rText) + EDIT_MINWIDTH );
        pEdit->SetSizePixel(aEditSize);
        pEdit->SetModifyHdl(LINK(this, SwTokenWindow, EditResize ));
        pEdit->SetPrevNextLink(LINK(this, SwTokenWindow, NextItemHdl));
        pEdit->SetGetFocusHdl(LINK(this, SwTokenWindow, TbxFocusHdl));
        pEdit->Show();
        pRet = pEdit;
    }
    else
    {
        VclPtr<SwTOXButton> pButton = VclPtr<SwTOXButton>::Create(m_pCtrlParentWin, this, rToken);
        pButton->SetPosPixel(aControlPos);

        m_aControlList.emplace_back(pButton);

        Size aEditSize(aControlSize);
        aEditSize.setWidth( pButton->GetTextWidth(rText) + 5 );
        pButton->SetSizePixel(aEditSize);
        pButton->SetPrevNextLink(LINK(this, SwTokenWindow, NextItemBtnHdl));
        pButton->SetGetFocusHdl(LINK(this, SwTokenWindow, TbxFocusBtnHdl));

        if(TOKEN_AUTHORITY != rToken.eTokenType)
            pButton->SetText(m_aButtonTexts[rToken.eTokenType]);
        else
        {
            //use the first two chars as symbol
            OUString sTmp(SwAuthorityFieldType::GetAuthFieldName(
                        static_cast<ToxAuthorityField>(rToken.nAuthorityField)));
            pButton->SetText(sTmp.copy(0, 2));
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

void SwTokenWindow::InsertAtSelection(const OUString& rText, const SwFormToken& rToken)
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

        const Control* pControl = nullptr;
        const Control* pExchange = nullptr;

        auto it = m_aControlList.cbegin();
        for( ; it != m_aControlList.cend() && m_pActiveCtrl != (*it); ++it )
        {
            pControl = *it;

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
                pControl = *it;

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
    auto iterActive = std::find(m_aControlList.begin(),
                                         m_aControlList.end(), m_pActiveCtrl);

    assert(iterActive != m_aControlList.end());
    if (iterActive == m_aControlList.end())
        return;

    Size aControlSize(GetOutputSizePixel());

    if( WindowType::EDIT == m_pActiveCtrl->GetType())
    {
        ++iterActive;

        Selection aSel = static_cast<SwTOXEdit*>(m_pActiveCtrl.get())->GetSelection();
        aSel.Justify();

        const OUString sEditText = static_cast<SwTOXEdit*>(m_pActiveCtrl.get())->GetText();
        const OUString sLeft = sEditText.copy( 0, aSel.Min() );
        const OUString sRight = sEditText.copy( aSel.Max() );

        static_cast<SwTOXEdit*>(m_pActiveCtrl.get())->SetText(sLeft);
        static_cast<SwTOXEdit*>(m_pActiveCtrl.get())->AdjustSize();

        SwFormToken aTmpToken(TOKEN_TEXT);
        VclPtr<SwTOXEdit> pEdit = VclPtr<SwTOXEdit>::Create(m_pCtrlParentWin, this, aTmpToken);

        iterActive = m_aControlList.insert(iterActive, pEdit);

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
        pEdit->SetSizePixel(aControlSize);
        pEdit->AdjustSize();
        pEdit->SetModifyHdl(LINK(this, SwTokenWindow, EditResize ));
        pEdit->SetPrevNextLink(LINK(this, SwTokenWindow, NextItemHdl));
        pEdit->SetGetFocusHdl(LINK(this, SwTokenWindow, TbxFocusHdl));
        pEdit->Show();
    }
    else
    {
        iterActive = m_aControlList.erase(iterActive);
        m_pActiveCtrl->Hide();
        m_pActiveCtrl.disposeAndClear();
    }

    //now the new button
    VclPtr<SwTOXButton> pButton = VclPtr<SwTOXButton>::Create(m_pCtrlParentWin, this, aToInsertToken);

    m_aControlList.insert(iterActive, pButton);

    pButton->SetPrevNextLink(LINK(this, SwTokenWindow, NextItemBtnHdl));
    pButton->SetGetFocusHdl(LINK(this, SwTokenWindow, TbxFocusBtnHdl));

    if(TOKEN_AUTHORITY != aToInsertToken.eTokenType)
    {
        pButton->SetText(m_aButtonTexts[aToInsertToken.eTokenType]);
    }
    else
    {
        //use the first two chars as symbol
        OUString sTmp(SwAuthorityFieldType::GetAuthFieldName(
                    static_cast<ToxAuthorityField>(aToInsertToken.nAuthorityField)));
        pButton->SetText(sTmp.copy(0, 2));
    }

    Size aEditSize(GetOutputSizePixel());
    aEditSize.setWidth( pButton->GetTextWidth(rText) + 5 );
    pButton->SetSizePixel(aEditSize);
    pButton->Check();
    pButton->Show();
    SetActiveControl(pButton);

    AdjustPositions();
}

void SwTokenWindow::RemoveControl(SwTOXButton* pDel, bool bInternalCall )
{
    if(bInternalCall && TOX_AUTHORITIES == m_pForm->GetTOXType())
        m_pParent->PreTokenButtonRemoved(pDel->GetFormToken());

    auto it = std::find(m_aControlList.begin(), m_aControlList.end(), pDel);

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
    VclPtr<Control> pLeftEdit = *itLeft;
    VclPtr<Control> pRightEdit = *itRight;

    static_cast<SwTOXEdit*>(pLeftEdit.get())->SetText(static_cast<SwTOXEdit*>(pLeftEdit.get())->GetText() +
                                     static_cast<SwTOXEdit*>(pRightEdit.get())->GetText());
    static_cast<SwTOXEdit*>(pLeftEdit.get())->AdjustSize();

    m_aControlList.erase(itRight);
    pRightEdit.disposeAndClear();

    m_aControlList.erase(it);
    m_pActiveCtrl->Hide();
    m_pActiveCtrl.disposeAndClear();

    SetActiveControl(pLeftEdit);
    AdjustPositions();
    m_aModifyHdl.Call(nullptr);
}

void SwTokenWindow::AdjustPositions()
{
    if(m_aControlList.size() > 1)
    {
        auto it = m_aControlList.begin();
        Control* pCtrl = it->get();
        ++it;

        Point aNextPos = pCtrl->GetPosPixel();
        aNextPos.AdjustX(pCtrl->GetSizePixel().Width() );

        for(; it != m_aControlList.end(); ++it)
        {
            pCtrl = it->get();
            pCtrl->SetPosPixel(aNextPos);
            aNextPos.AdjustX(pCtrl->GetSizePixel().Width() );
        }

        AdjustScrolling();
    }
};

void SwTokenWindow::MoveControls(long nOffset)
{
    // move the complete list
    for (VclPtr<Control> const & pCtrl : m_aControlList)
    {
        Point aPos = pCtrl->GetPosPixel();
        aPos.AdjustX(nOffset );

        pCtrl->SetPosPixel(aPos);
    }
}

void SwTokenWindow::AdjustScrolling()
{
    if(m_aControlList.size() > 1)
    {
        //validate scroll buttons
        Control* pFirstCtrl = m_aControlList.begin()->get();
        Control* pLastCtrl = m_aControlList.rbegin()->get();

        long nSpace = m_pCtrlParentWin->GetSizePixel().Width();
        long nWidth = pLastCtrl->GetPosPixel().X() - pFirstCtrl->GetPosPixel().X()
                                                    + pLastCtrl->GetSizePixel().Width();
        bool bEnable = nWidth > nSpace;

        //the active control must be visible
        if(bEnable && m_pActiveCtrl)
        {
            Point aActivePos(m_pActiveCtrl->GetPosPixel());

            long nMove = 0;

            if(aActivePos.X() < 0)
                nMove = -aActivePos.X();
            else if((aActivePos.X() + m_pActiveCtrl->GetSizePixel().Width())  > nSpace)
                nMove = -(aActivePos.X() + m_pActiveCtrl->GetSizePixel().Width() - nSpace);

            if(nMove)
                MoveControls(nMove);

            m_pLeftScrollWin->Enable(pFirstCtrl->GetPosPixel().X() < 0);

            m_pRightScrollWin->Enable((pLastCtrl->GetPosPixel().X() + pLastCtrl->GetSizePixel().Width()) > nSpace);
        }
        else
        {
            //if the control fits into the space then the first control must be at position 0
            long nFirstPos = pFirstCtrl->GetPosPixel().X();

            if(nFirstPos != 0)
                MoveControls(-nFirstPos);

            m_pRightScrollWin->Enable(false);
            m_pLeftScrollWin->Enable(false);
        }
    }
}

IMPL_LINK(SwTokenWindow, ScrollHdl, Button*, pBtn, void )
{
    if(m_aControlList.empty())
        return;

    const long nSpace = m_pCtrlParentWin->GetSizePixel().Width();

    long nMove = 0;
    if(pBtn == m_pLeftScrollWin)
    {
        //find the first completely visible control (left edge visible)
        auto it = std::find_if(m_aControlList.begin(), m_aControlList.end(),
            [](const VclPtr<Control>& rControl) {
                Control *pCtrl = rControl.get();
                return pCtrl->GetPosPixel().X() >= 0;
            });
        if (it != m_aControlList.end())
        {
            if (it == m_aControlList.begin())
            {
                //move the current control to the left edge
                Control *pCtrl = it->get();

                nMove = -pCtrl->GetPosPixel().X();
            }
            else
            {
                //move the left neighbor to the start position
                auto itLeft = it;
                --itLeft;
                Control *pLeft = itLeft->get();

                nMove = -pLeft->GetPosPixel().X();
            }
        }
    }
    else
    {
        //find the first completely visible control (right edge visible)
        auto it = std::find_if(m_aControlList.rbegin(), m_aControlList.rend(),
            [&nSpace](const VclPtr<Control>& rControl) {
                Control *pCtrl = rControl.get();
                long nCtrlWidth = pCtrl->GetSizePixel().Width();
                long nXPos = pCtrl->GetPosPixel().X() + nCtrlWidth;
                return nXPos <= nSpace;
            });
        if (it != m_aControlList.rend() && it != m_aControlList.rbegin())
        {
            //move the right neighbor  to the right edge right aligned
            auto itRight = it;
            --itRight;
            Control *pRight = itRight->get();
            nMove = nSpace - pRight->GetPosPixel().X() - pRight->GetSizePixel().Width();
        }

        //move it left until it's completely visible
    }

    if(nMove)
    {
        // move the complete list
        MoveControls(nMove);

        Control *pCtrl = nullptr;

        pCtrl = m_aControlList.begin()->get();
        m_pLeftScrollWin->Enable(pCtrl->GetPosPixel().X() < 0);

        pCtrl = m_aControlList.rbegin()->get();
        m_pRightScrollWin->Enable((pCtrl->GetPosPixel().X() + pCtrl->GetSizePixel().Width()) > nSpace);
    }
}

OUString SwTokenWindow::GetPattern() const
{
    OUStringBuffer sRet;

    for (const Control* pCtrl : m_aControlList)
    {
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

    for (const Control* pCtrl : m_aControlList)
    {
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

bool SwTokenWindow::CreateQuickHelp(Control const * pCtrl,
            const SwFormToken& rToken,
            const HelpEvent& rHEvt)
{
    bool bRet = false;
    if( rHEvt.GetMode() & HelpEventMode::QUICK )
    {
        bool bBalloon = Help::IsBalloonHelpEnabled();
        OUString sEntry;
        if(bBalloon || rToken.eTokenType != TOKEN_AUTHORITY)
            sEntry = m_aButtonHelpTexts[rToken.eTokenType];
        if(rToken.eTokenType == TOKEN_AUTHORITY )
        {
             sEntry += SwAuthorityFieldType::GetAuthFieldName(
                                static_cast<ToxAuthorityField>(rToken.nAuthorityField));
        }

        Point aPos = m_pCtrlParentWin->OutputToScreenPixel(pCtrl->GetPosPixel());
        tools::Rectangle aItemRect( aPos, pCtrl->GetSizePixel() );
        if ( rToken.eTokenType != TOKEN_TAB_STOP )
        {
            if (!rToken.sCharStyleName.isEmpty())
            {
                sEntry += OUString(bBalloon ? '\n' : ' ')
                        + m_sCharStyle + rToken.sCharStyleName;
            }
        }
        if(bBalloon)
        {
            Help::ShowBalloon( this, aPos, aItemRect, sEntry );
        }
        else
            Help::ShowQuickHelp( this, aItemRect, sEntry,
                QuickHelpFlags::Left|QuickHelpFlags::VCenter );
        bRet = true;
    }
    return bRet;
}

IMPL_LINK(SwTokenWindow, EditResize, Edit&, rEdit, void)
{
    static_cast<SwTOXEdit*>(&rEdit)->AdjustSize();
    AdjustPositions();
    m_aModifyHdl.Call(nullptr);
}

IMPL_LINK(SwTokenWindow, NextItemHdl, SwTOXEdit&, rEdit, void)
{
    auto it = std::find(m_aControlList.begin(),m_aControlList.end(),&rEdit);

    if (it == m_aControlList.end())
        return;

    auto itTest = it;
    ++itTest;

    if ((it != m_aControlList.begin() && !rEdit.IsNextControl()) ||
        (itTest != m_aControlList.end() && rEdit.IsNextControl()))
    {
        auto iterFocus = it;
        rEdit.IsNextControl() ? ++iterFocus : --iterFocus;

        Control *pCtrlFocus = iterFocus->get();
        pCtrlFocus->GrabFocus();
        static_cast<SwTOXButton*>(pCtrlFocus)->Check();

        AdjustScrolling();
    }
}

IMPL_LINK(SwTokenWindow, TbxFocusHdl, Control&, rControl, void)
{
    SwTOXEdit* pEdit = static_cast<SwTOXEdit*>(&rControl);
    for (VclPtr<Control> const & pCtrl : m_aControlList)
    {
        if (pCtrl && pCtrl->GetType() != WindowType::EDIT)
            static_cast<SwTOXButton*>(pCtrl.get())->Check(false);
    }

    SetActiveControl(pEdit);
}

IMPL_LINK(SwTokenWindow, NextItemBtnHdl, SwTOXButton&, rBtn, void )
{
    auto it = std::find(m_aControlList.begin(),m_aControlList.end(),&rBtn);

    if (it == m_aControlList.end())
        return;

    auto itTest = it;
    ++itTest;

    if (!rBtn.IsNextControl() || (itTest != m_aControlList.end() && rBtn.IsNextControl()))
    {
        bool isNext = rBtn.IsNextControl();

        auto iterFocus = it;
        isNext ? ++iterFocus : --iterFocus;

        Control *pCtrlFocus = iterFocus->get();
        pCtrlFocus->GrabFocus();
        Selection aSel(0,0);

        if (!isNext)
        {
            const sal_Int32 nLen = static_cast<SwTOXEdit*>(pCtrlFocus)->GetText().getLength();

            aSel.Min() = nLen;
            aSel.Max() = nLen;
        }

        static_cast<SwTOXEdit*>(pCtrlFocus)->SetSelection(aSel);

        rBtn.Check(false);

        AdjustScrolling();
    }
}

IMPL_LINK(SwTokenWindow, TbxFocusBtnHdl, Control&, rControl, void )
{
    SwTOXButton* pBtn = static_cast<SwTOXButton*>(&rControl);
    for (auto& aControl : m_aControlList)
    {
        Control *pControl = aControl.get();

        if (pControl && WindowType::EDIT != pControl->GetType())
            static_cast<SwTOXButton*>(pControl)->Check(pBtn == pControl);
    }

    SetActiveControl(pBtn);
}

void SwTokenWindow::GetFocus()
{
    if(GetFocusFlags::Tab & GetGetFocusFlags())
    {
        if (!m_aControlList.empty())
        {
            Control *pFirst = m_aControlList.begin()->get();

            if (pFirst)
            {
                pFirst->GrabFocus();
                SetActiveControl(pFirst);
                AdjustScrolling();
            }
        }
    }
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
    for (const Control* pControl : m_aControlList)
    {
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

SwTOXStylesTabPage::SwTOXStylesTabPage(TabPageParent pParent, const SfxItemSet& rAttrSet)
    : SfxTabPage(pParent, "modules/swriter/ui/tocstylespage.ui", "TocStylesPage", &rAttrSet)
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
        aStr += " " + OUStringLiteral1(aDeliStart)
              + m_pCurrentForm->GetTemplate( 0 )
              + OUStringLiteral1(aDeliEnd);
    }
    m_xLevelLB->append_text(aStr);

    for( sal_uInt16 i=1; i < nSize; ++i )
    {
        if( TOX_INDEX == m_pCurrentForm->GetTOXType() &&
            FORM_ALPHA_DELIMITTER == i )
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
            aStr += " " + OUStringLiteral1(aDeliStart)
                  + m_pCurrentForm->GetTemplate( i )
                  + OUStringLiteral1(aDeliEnd);
        }
        m_xLevelLB->append_text(aStr);
    }
    m_xLevelLB->thaw();

    // initialise templates
    SwWrtShell& rSh = static_cast<SwMultiTOXTabDialog*>(GetTabDialog())->GetWrtShell();
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

VclPtr<SfxTabPage> SwTOXStylesTabPage::Create(TabPageParent pParent,
                                              const SfxItemSet* rAttrSet)
{
    return VclPtr<SwTOXStylesTabPage>::Create(pParent, *rAttrSet);
}

IMPL_LINK_NOARG(SwTOXStylesTabPage, EditStyleHdl, weld::Button&, void)
{
    if (m_xParaLayLB->get_selected_index() != -1)
    {
        SfxStringItem aStyle(SID_STYLE_EDIT, m_xParaLayLB->get_selected_text());
        SfxUInt16Item aFamily(SID_STYLE_FAMILY, sal_uInt16(SfxStyleFamily::Para));
        SwWrtShell& rSh = static_cast<SwMultiTOXTabDialog*>(GetTabDialog())->GetWrtShell();
        rSh.GetView().GetViewFrame()->GetDispatcher()->ExecuteList(SID_STYLE_EDIT,
                SfxCallMode::SYNCHRON|SfxCallMode::MODAL,
                { &aStyle, &aFamily });
    }
}

// allocate templates
IMPL_LINK_NOARG(SwTOXStylesTabPage, AssignHdl, weld::Button&, void)
{
    auto nLevPos = m_xLevelLB->get_selected_index();
    auto nTemplPos = m_xParaLayLB->get_selected_index();
    if (nLevPos != -1 && nTemplPos != -1)
    {
        const OUString aStr(m_xLevelLB->get_text(nLevPos).getToken(0, aDeliStart)
            + OUStringLiteral1(aDeliStart)
            + m_xParaLayLB->get_selected_text()
            + OUStringLiteral1(aDeliEnd));

        m_pCurrentForm->SetTemplate(nLevPos, m_xParaLayLB->get_selected_text());

        m_xLevelLB->remove(nLevPos);
        m_xLevelLB->insert_text(nLevPos, aStr);
        m_xLevelLB->select_text(aStr);
        Modify();
    }
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
        m_pCurrentForm->SetTemplate(nPos, aEmptyOUStr);
        Modify();
    }
}

IMPL_LINK_NOARG(SwTOXStylesTabPage, DoubleClickHdl, weld::TreeView&, void)
{
    const OUString aTmpName(m_xParaLayLB->get_selected_text());
    SwWrtShell& rSh = static_cast<SwMultiTOXTabDialog*>(GetTabDialog())->GetWrtShell();

    if(m_xParaLayLB->get_selected_index() != -1 &&
       (m_xLevelLB->get_selected_index() == 0 || SwMultiTOXTabDialog::IsNoNum(rSh, aTmpName)))
        AssignHdl(*m_xAssignBT);
}

// enable only when selected
IMPL_LINK_NOARG(SwTOXStylesTabPage, EnableSelectHdl, weld::TreeView&, void)
{
    m_xStdBT->set_sensitive(m_xLevelLB->get_selected_index() != -1);

    SwWrtShell& rSh = static_cast<SwMultiTOXTabDialog*>(GetTabDialog())->GetWrtShell();
    const OUString aTmpName(m_xParaLayLB->get_selected_text());
    m_xAssignBT->set_sensitive(m_xParaLayLB->get_selected_index() != -1 &&
                               m_xLevelLB->get_selected_index() != -1 &&
       (m_xLevelLB->get_selected_index() == 0 || SwMultiTOXTabDialog::IsNoNum(rSh, aTmpName)));
    m_xEditStyleBT->set_sensitive(m_xParaLayLB->get_selected_index() != -1);
}

void SwTOXStylesTabPage::Modify()
{
    SwMultiTOXTabDialog* pTOXDlg = static_cast<SwMultiTOXTabDialog*>(GetTabDialog());
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

SwEntryBrowseBox::SwEntryBrowseBox(vcl::Window* pParent, VclBuilderContainer* pBuilder)
    : SwEntryBrowseBox_Base( pParent, EditBrowseBoxFlags::NONE, WB_TABSTOP | WB_BORDER,
                           BrowserMode::KEEPHIGHLIGHT |
                           BrowserMode::COLUMNSELECTION |
                           BrowserMode::MULTISELECTION |
                           BrowserMode::TRACKING_TIPS |
                           BrowserMode::HLINES |
                           BrowserMode::VLINES |
                           BrowserMode::AUTO_VSCROLL|
                           BrowserMode::HIDECURSOR   )
    , m_aCellEdit(VclPtr<Edit>::Create(&GetDataWindow(), 0))
    , m_aCellCheckBox(VclPtr< ::svt::CheckBoxControl>::Create(&GetDataWindow()))
    , m_nCurrentRow(0)
    , m_bModified(false)
{
    m_sSearch = pBuilder->get<vcl::Window>("searchterm")->GetText();
    m_sAlternative = pBuilder->get<vcl::Window>("alternative")->GetText();
    m_sPrimKey = pBuilder->get<vcl::Window>("key1")->GetText();
    m_sSecKey = pBuilder->get<vcl::Window>("key2")->GetText();
    m_sComment = pBuilder->get<vcl::Window>("comment")->GetText();
    m_sCaseSensitive = pBuilder->get<vcl::Window>("casesensitive")->GetText();
    m_sWordOnly = pBuilder->get<vcl::Window>("wordonly")->GetText();
    m_sYes = pBuilder->get<vcl::Window>("yes")->GetText();
    m_sNo = pBuilder->get<vcl::Window>("no")->GetText();

    m_aCellCheckBox->GetBox().EnableTriState(false);
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
        &m_sSearch,
        &m_sAlternative,
        &m_sPrimKey,
        &m_sSecKey,
        &m_sComment,
        &m_sCaseSensitive,
        &m_sWordOnly
    };

    long nWidth = GetSizePixel().Width();
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

    Dialog *pDlg = GetParentDialog();
    if (pDlg && pDlg->isCalculatingInitialLayoutSize())
    {
        long nWidth = GetSizePixel().Width();
        std::vector<long> aWidths = GetOptimalColWidths();
        long nNaturalWidth(std::accumulate(aWidths.begin(), aWidths.end(), 0));
        long nExcess = ((nWidth - nNaturalWidth) / aWidths.size()) - 1;

        for (size_t i = 0; i < aWidths.size(); ++i)
            SetColumnWidth(i+1, aWidths[i] + nExcess);
    }
}

std::vector<long> SwEntryBrowseBox::GetOptimalColWidths() const
{
    std::vector<long> aWidths;

    long nStandardColMinWidth = approximate_char_width() * 16;
    long nYesNoWidth = approximate_char_width() * 5;
    nYesNoWidth = std::max(nYesNoWidth, GetTextWidth(m_sYes));
    nYesNoWidth = std::max(nYesNoWidth, GetTextWidth(m_sNo));
    for (sal_uInt16 i = 1; i < 6; i++)
    {
        long nColWidth = std::max(nStandardColMinWidth,
                                  GetTextWidth(GetColumnTitle(i)));
        nColWidth += 12;
        aWidths.push_back(nColWidth);
    }

    for (sal_uInt16 i = 6; i < 8; i++)
    {
        long nColWidth = std::max(nYesNoWidth,
                                  GetTextWidth(GetColumnTitle(i)));
        nColWidth += 12;
        aWidths.push_back(nColWidth);
    }

    return aWidths;
}

Size SwEntryBrowseBox::GetOptimalSize() const
{
    Size aSize = LogicToPixel(Size(276 , 175), MapMode(MapUnit::MapAppFont));

    std::vector<long> aWidths = GetOptimalColWidths();

    long nWidth(std::accumulate(aWidths.begin(), aWidths.end(), 0));

    aSize.setWidth( std::max(aSize.Width(), nWidth) );

    return aSize;
}

bool SwEntryBrowseBox::SeekRow( long nRow )
{
    m_nCurrentRow = nRow;
    return true;
}

OUString SwEntryBrowseBox::GetCellText(long nRow, sal_uInt16 nColumn) const
{
    const OUString* pRet = &aEmptyOUStr;
    if (static_cast<size_t>(nRow) < m_Entries.size())
    {
        const AutoMarkEntry* pEntry = m_Entries[ nRow ].get();
        switch(nColumn)
        {
            case  ITEM_SEARCH       :pRet = &pEntry->sSearch; break;
            case  ITEM_ALTERNATIVE  :pRet = &pEntry->sAlternative; break;
            case  ITEM_PRIM_KEY     :pRet = &pEntry->sPrimKey   ; break;
            case  ITEM_SEC_KEY      :pRet = &pEntry->sSecKey    ; break;
            case  ITEM_COMMENT      :pRet = &pEntry->sComment   ; break;
            case  ITEM_CASE         :pRet = pEntry->bCase ? &m_sYes : &m_sNo; break;
            case  ITEM_WORDONLY     :pRet = pEntry->bWord ? &m_sYes : &m_sNo; break;
        }
    }
    return *pRet;
}

void SwEntryBrowseBox::PaintCell(OutputDevice& rDev,
                                const tools::Rectangle& rRect, sal_uInt16 nColumnId) const
{
    const DrawTextFlags nStyle = DrawTextFlags::Clip | DrawTextFlags::Center;
    rDev.DrawText( rRect, GetCellText( m_nCurrentRow, nColumnId ), nStyle );
}

::svt::CellController* SwEntryBrowseBox::GetController(long /*nRow*/, sal_uInt16 nCol)
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
        bVal = static_cast< ::svt::CheckBoxCellController*>(pController)->GetCheckBox().IsChecked();
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
            pController->ClearModified();
            GoToRow( nRow );
        }
    }
    return true;
}

void SwEntryBrowseBox::InitController(
                ::svt::CellControllerRef& rController, long nRow, sal_uInt16 nCol)
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
        static_cast< ::svt::CheckBoxCellController*>(pController)->GetCheckBox().Check(
                                                            rText == m_sYes );
     }
}

void SwEntryBrowseBox::ReadEntries(SvStream& rInStr)
{
    AutoMarkEntry* pToInsert = nullptr;
    rtl_TextEncoding  eTEnc = osl_getThreadTextEncoding();
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

                OUString sStr = sLine.getToken(0, ';', nSttPos );
                pToInsert->bCase = !sStr.isEmpty() && sStr != "0";

                sStr = sLine.getToken(0, ';', nSttPos );
                pToInsert->bWord = !sStr.isEmpty() && sStr != "0";

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
    if(pController ->IsModified())
        GoToColumnId(nCol + (nCol < ITEM_CASE ? 1 : -1 ));

    rtl_TextEncoding  eTEnc = osl_getThreadTextEncoding();
    for(std::unique_ptr<AutoMarkEntry> & rpEntry : m_Entries)
    {
        AutoMarkEntry* pEntry = rpEntry.get();
        if(!pEntry->sComment.isEmpty())
        {
            rOutStr.WriteByteStringLine( "#" + pEntry->sComment, eTEnc );
        }

        OUString sWrite( pEntry->sSearch + ";" +
                         pEntry->sAlternative + ";" +
                         pEntry->sPrimKey  + ";" +
                         pEntry->sSecKey + ";" +
                         (pEntry->bCase ? OUString("1") : OUString("0")) + ";" +
                         (pEntry->bWord ? OUString("1") : OUString("0")) );

        if( sWrite.getLength() > 5 )
            rOutStr.WriteByteStringLine( sWrite, eTEnc );
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
    return pController->IsModified();
}

SwAutoMarkDlg_Impl::SwAutoMarkDlg_Impl(vcl::Window* pParent, const OUString& rAutoMarkURL,
        bool bCreate)
    : ModalDialog(pParent, "CreateAutomarkDialog",
        "modules/swriter/ui/createautomarkdialog.ui")
    , sAutoMarkURL(rAutoMarkURL)
    , bCreateMode(bCreate)
{
    get(m_pOKPB, "ok");
    m_pEntriesBB = VclPtr<SwEntryBrowseBox>::Create(get<VclContainer>("area"), this);
    m_pEntriesBB->set_expand(true);
    m_pEntriesBB->Show();
    m_pOKPB->SetClickHdl(LINK(this, SwAutoMarkDlg_Impl, OkHdl));

    SetText(GetText() + ": " + sAutoMarkURL);
    bool bError = false;
    if( bCreateMode )
        m_pEntriesBB->RowInserted(0);
    else
    {
        SfxMedium aMed( sAutoMarkURL, StreamMode::STD_READ );
        if( aMed.GetInStream() && !aMed.GetInStream()->GetError() )
            m_pEntriesBB->ReadEntries( *aMed.GetInStream() );
        else
            bError = true;
    }

    if(bError)
        EndDialog();
}

SwAutoMarkDlg_Impl::~SwAutoMarkDlg_Impl()
{
    disposeOnce();
}

void SwAutoMarkDlg_Impl::dispose()
{
    m_pEntriesBB.disposeAndClear();
    m_pOKPB.clear();
    ModalDialog::dispose();
}

IMPL_LINK_NOARG(SwAutoMarkDlg_Impl, OkHdl, Button*, void)
{
    bool bError = false;
    if(m_pEntriesBB->IsModified() || bCreateMode)
    {
        SfxMedium aMed( sAutoMarkURL,
                        bCreateMode ? StreamMode::WRITE
                                    : StreamMode::WRITE| StreamMode::TRUNC );
        SvStream* pStrm = aMed.GetOutStream();
        pStrm->SetStreamCharSet( RTL_TEXTENCODING_MS_1253 );
        if( !pStrm->GetError() )
        {
            m_pEntriesBB->WriteEntries( *pStrm );
            aMed.Commit();
        }
        else
            bError = true;
    }
    if( !bError )
        EndDialog(RET_OK);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
