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

#include <comphelper/string.hxx>
#include <rsc/rscsfx.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/help.hxx>
#include <vcl/builderfactory.hxx>
#include <svl/stritem.hxx>
#include <svl/urihelper.hxx>
#include <unotools/pathoptions.hxx>
#include <sfx2/request.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/docfile.hxx>
#include <svtools/simptabl.hxx>
#include <svtools/treelistentry.hxx>
#include <svx/dialogs.hrc>
#include <svx/svxdlg.hxx>
#include <svx/flagsdef.hxx>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <com/sun/star/ui/dialogs/XFilePicker2.hpp>
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
#include <formedt.hxx>
#include <poolfmt.hxx>
#include <poolfmt.hrc>
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
#include <helpid.h>
#include <utlui.hrc>
#include <index.hrc>
#include <globals.hrc>
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

static OUString lcl_CreateAutoMarkFileDlg( const OUString& rURL,
                                const OUString& rFileString, bool bOpen )
{
    OUString sRet;

    FileDialogHelper aDlgHelper( bOpen ?
                TemplateDescription::FILEOPEN_SIMPLE : TemplateDescription::FILESAVE_AUTOEXTENSION, 0 );
    uno::Reference < XFilePicker2 > xFP = aDlgHelper.GetFilePicker();

    uno::Reference<XFilterManager> xFltMgr(xFP, UNO_QUERY);
    xFltMgr->appendFilter( rFileString, "*.sdi" );
    xFltMgr->setCurrentFilter( rFileString ) ;

    OUString& rLastSaveDir = (OUString&)SfxGetpApp()->GetLastSaveDirectory();
    OUString sSaveDir = rLastSaveDir;

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
    rLastSaveDir = sSaveDir;
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
    VclPtr<Edit>                    aCellEdit;
    VclPtr< ::svt::CheckBoxControl>  aCellCheckBox;

    OUString  sSearch;
    OUString  sAlternative;
    OUString  sPrimKey;
    OUString  sSecKey;
    OUString  sComment;
    OUString  sCaseSensitive;
    OUString  sWordOnly;
    OUString  sYes;
    OUString  sNo;

    std::vector<std::unique_ptr<AutoMarkEntry>> m_Entries;

    ::svt::CellControllerRef    xController;
    ::svt::CellControllerRef    xCheckController;

    long    nCurrentRow;
    bool    bModified;

    void                            SetModified() {bModified = true;}

protected:
    virtual bool                    SeekRow( long nRow ) override;
    virtual void                    PaintCell(OutputDevice& rDev, const Rectangle& rRect, sal_uInt16 nColId) const override;
    virtual void                    InitController(::svt::CellControllerRef& rController, long nRow, sal_uInt16 nCol) override;
    virtual ::svt::CellController*  GetController(long nRow, sal_uInt16 nCol) override;
    virtual bool                    SaveModified() override;

    std::vector<long>               GetOptimalColWidths() const;

public:
    SwEntryBrowseBox(vcl::Window* pParent, VclBuilderContainer* pBuilder);
    virtual ~SwEntryBrowseBox();
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

    DECL_LINK_TYPED(OkHdl, Button*, void);
public:
    SwAutoMarkDlg_Impl(vcl::Window* pParent, const OUString& rAutoMarkURL,
                       bool bCreate);
    virtual ~SwAutoMarkDlg_Impl();
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
    , pMgr( new SwTOXMgr( &rShell ) )
    , rSh(rShell)
    , pExampleFrame(nullptr)
    , pParamTOXBase(pCurTOX)
    , sUserDefinedIndex(SW_RESSTR(STR_USER_DEFINED_INDEX))
    , nInitialTOXType(nToxType)
    , bEditTOX(false)
    , bExampleCreated(false)
    , bGlobalFlag(bGlobal)
{
    get(m_pShowExampleCB, "showexample");
    get(m_pExampleContainerWIN, "example");
    Size aWinSize(LogicToPixel(Size(150, 188), MapMode(MAP_APPFONT)));
    m_pExampleContainerWIN->set_width_request(aWinSize.Width());
    m_pExampleContainerWIN->set_height_request(aWinSize.Height());
    m_pExampleContainerWIN->SetSizePixel(aWinSize);

    eCurrentTOXType.eType = TOX_CONTENT;
    eCurrentTOXType.nIndex = 0;

    const sal_uInt16 nUserTypeCount = rSh.GetTOXTypeCount(TOX_USER);
    nTypeCount = nUserTypeCount + 6;
    pFormArr = new SwForm*[nTypeCount];
    pDescArr = new SwTOXDescription*[nTypeCount];
    pxIndexSectionsArr = new SwIndexSections_Impl*[nTypeCount];
    //the standard user index is on position TOX_USER
    //all user indexes follow after position TOX_AUTHORITIES
    if(pCurTOX)
    {
        bEditTOX = true;
    }
    for(int i = nTypeCount - 1; i > -1; i--)
    {
        pFormArr[i] = nullptr;
        pDescArr[i] = nullptr;
        pxIndexSectionsArr[i] = new SwIndexSections_Impl;
        if(pCurTOX)
        {
            eCurrentTOXType.eType = pCurTOX->GetType();
            sal_uInt16 nArrayIndex = static_cast< sal_uInt16 >(eCurrentTOXType.eType);
            if(eCurrentTOXType.eType == TOX_USER)
            {
                //which user type is it?
                for(sal_uInt16 nUser = 0; nUser < nUserTypeCount; nUser++)
                {
                    const SwTOXType* pTemp = rSh.GetTOXType(TOX_USER, nUser);
                    if(pCurTOX->GetTOXType() == pTemp)
                    {
                        eCurrentTOXType.nIndex = nUser;
                        nArrayIndex = static_cast< sal_uInt16 >(nUser > 0 ? TOX_AUTHORITIES + nUser : TOX_USER);
                        break;
                    }
                }
            }
            pFormArr[nArrayIndex] = new SwForm(pCurTOX->GetTOXForm());
            pDescArr[nArrayIndex] = CreateTOXDescFromTOXBase(pCurTOX);
            if(TOX_AUTHORITIES == eCurrentTOXType.eType)
            {
                const SwAuthorityFieldType* pFType = static_cast<const SwAuthorityFieldType*>(
                                                rSh.GetFieldType(RES_AUTHORITY, aEmptyOUStr));
                if(pFType)
                {
                    OUString sBrackets;
                    if(pFType->GetPrefix())
                        sBrackets += OUString(pFType->GetPrefix());
                    if(pFType->GetSuffix())
                        sBrackets += OUString(pFType->GetSuffix());
                    pDescArr[nArrayIndex]->SetAuthBrackets(sBrackets);
                    pDescArr[nArrayIndex]->SetAuthSequence(pFType->IsSequence());
                }
                else
                {
                    pDescArr[nArrayIndex]->SetAuthBrackets("[]");
                }
            }
        }
    }
    SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();
    OSL_ENSURE(pFact, "Dialog creation failed!");
    m_nSelectId = AddTabPage("index", SwTOXSelectTabPage::Create, nullptr);
    m_nStylesId = AddTabPage("styles", SwTOXStylesTabPage::Create, nullptr);
    m_nColumnId = AddTabPage("columns", SwColumnPage::Create, nullptr);
    m_nBackGroundId = AddTabPage("background", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BACKGROUND ), nullptr);
    m_nEntriesId = AddTabPage("entries", SwTOXEntryTabPage::Create, nullptr);
    if(!pCurTOX)
        SetCurPageId(m_nSelectId);

    m_pShowExampleCB->SetClickHdl(LINK(this, SwMultiTOXTabDialog, ShowPreviewHdl));

    m_pShowExampleCB->Check( SW_MOD()->GetModuleConfig()->IsShowIndexPreview());

    m_pExampleContainerWIN->SetAccessibleName(m_pShowExampleCB->GetText());
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

    for(sal_uInt16 i = 0; i < nTypeCount; i++)
    {
        delete pFormArr[i];
        delete pDescArr[i];
        delete pxIndexSectionsArr[i];
    }
    delete[] pxIndexSectionsArr;

    delete[] pFormArr;
    delete[] pDescArr;
    delete pMgr;
    delete pExampleFrame;
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
        const SwFormatFrameSize& rSize = static_cast<const SwFormatFrameSize&>(GetInputSetImpl()->Get(RES_FRM_SIZE));

        static_cast<SwColumnPage&>(rPage).SetPageWidth(rSize.GetWidth());
    }
    else if (nId == m_nEntriesId)
        static_cast<SwTOXEntryTabPage&>(rPage).SetWrtShell(rSh);
    else if (nId == m_nSelectId)
    {
        static_cast<SwTOXSelectTabPage&>(rPage).SetWrtShell(rSh);
        if(USHRT_MAX != nInitialTOXType)
            static_cast<SwTOXSelectTabPage&>(rPage).SelectType((TOXTypes)nInitialTOXType);
    }
}

short SwMultiTOXTabDialog::Ok()
{
    short nRet = SfxTabDialog::Ok();
    SwTOXDescription& rDesc = GetTOXDescription(eCurrentTOXType);
    SwTOXBase aNewDef(*rSh.GetDefaultTOXBase( eCurrentTOXType.eType, true ));

    const sal_uInt16 nIndex = eCurrentTOXType.GetFlatIndex();
    if(pFormArr[nIndex])
    {
        rDesc.SetForm(*pFormArr[nIndex]);
        aNewDef.SetTOXForm(*pFormArr[nIndex]);
    }
    rDesc.ApplyTo(aNewDef);
    if(!bGlobalFlag)
        pMgr->UpdateOrInsertTOX(
                rDesc, nullptr, GetOutputItemSet());
    else if(bEditTOX)
        pMgr->UpdateOrInsertTOX(
                rDesc, &pParamTOXBase, GetOutputItemSet());

    if(!eCurrentTOXType.nIndex)
        rSh.SetDefaultTOXBase(aNewDef);

    return nRet;
}

SwForm* SwMultiTOXTabDialog::GetForm(CurTOXType eType)
{
    const sal_uInt16 nIndex = eType.GetFlatIndex();
    if(!pFormArr[nIndex])
        pFormArr[nIndex] = new SwForm(eType.eType);
    return pFormArr[nIndex];
}

SwTOXDescription& SwMultiTOXTabDialog::GetTOXDescription(CurTOXType eType)
{
    const sal_uInt16 nIndex = eType.GetFlatIndex();
    if(!pDescArr[nIndex])
    {
        const SwTOXBase* pDef = rSh.GetDefaultTOXBase( eType.eType );
        if(pDef)
            pDescArr[nIndex] = CreateTOXDescFromTOXBase(pDef);
        else
        {
            pDescArr[nIndex] = new SwTOXDescription(eType.eType);
            if(eType.eType == TOX_USER)
                pDescArr[nIndex]->SetTitle(sUserDefinedIndex);
            else
                pDescArr[nIndex]->SetTitle(
                    rSh.GetTOXType(eType.eType, 0)->GetTypeName());
        }
        if(TOX_AUTHORITIES == eType.eType)
        {
            const SwAuthorityFieldType* pFType = static_cast<const SwAuthorityFieldType*>(
                                            rSh.GetFieldType(RES_AUTHORITY, aEmptyOUStr));
            if(pFType)
            {
                pDescArr[nIndex]->SetAuthBrackets(OUString(pFType->GetPrefix()) +
                                                  OUString(pFType->GetSuffix()));
                pDescArr[nIndex]->SetAuthSequence(pFType->IsSequence());
            }
            else
            {
                pDescArr[nIndex]->SetAuthBrackets("[]");
            }
        }
        else if(TOX_INDEX == eType.eType)
            pDescArr[nIndex]->SetMainEntryCharStyle(SW_RESSTR(STR_POOLCHR_IDX_MAIN_ENTRY));

    }
    return *pDescArr[nIndex];
}

SwTOXDescription* SwMultiTOXTabDialog::CreateTOXDescFromTOXBase(
            const SwTOXBase*pCurTOX)
{
    SwTOXDescription * pDesc = new SwTOXDescription(pCurTOX->GetType());
    for(sal_uInt16 i = 0; i < MAXLEVEL; i++)
        pDesc->SetStyleNames(pCurTOX->GetStyleNames(i), i);
    pDesc->SetAutoMarkURL(rSh.GetTOIAutoMarkURL());
    pDesc->SetTitle(pCurTOX->GetTitle());

    pDesc->SetContentOptions(pCurTOX->GetCreateType());
    if(pDesc->GetTOXType() == TOX_INDEX)
        pDesc->SetIndexOptions(pCurTOX->GetOptions());
    pDesc->SetMainEntryCharStyle(pCurTOX->GetMainEntryCharStyle());
    if(pDesc->GetTOXType() != TOX_INDEX)
        pDesc->SetLevel((sal_uInt8)pCurTOX->GetLevel());
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

IMPL_LINK_NOARG_TYPED( SwMultiTOXTabDialog, ShowPreviewHdl, Button*, void )
{
    if(m_pShowExampleCB->IsChecked())
    {
        if(!pExampleFrame && !bExampleCreated)
        {
            bExampleCreated = true;
            OUString sTemplate("internal/idxexample.odt");

            SvtPathOptions aOpt;
            bool bExist = aOpt.SearchFile( sTemplate, SvtPathOptions::PATH_TEMPLATE );

            if(!bExist)
            {
                OUString sInfo(SW_RESSTR(STR_FILE_NOT_FOUND));
                sInfo = sInfo.replaceFirst( "%1", sTemplate );
                sInfo = sInfo.replaceFirst( "%2", aOpt.GetTemplatePath() );
                ScopedVclPtrInstance< InfoBox > aInfo(GetParent(), sInfo);
                aInfo->Execute();
            }
            else
            {
                Link<SwOneExampleFrame&,void> aLink(LINK(this, SwMultiTOXTabDialog, CreateExample_Hdl));
                pExampleFrame = new SwOneExampleFrame(
                        *m_pExampleContainerWIN, EX_SHOW_ONLINE_LAYOUT, &aLink, &sTemplate);

                if(!pExampleFrame->IsServiceAvailable())
                {
                    SwOneExampleFrame::CreateErrorMessage(nullptr);
                }
            }
            m_pShowExampleCB->Show(pExampleFrame && pExampleFrame->IsServiceAvailable());
        }
    }
    bool bSetViewWindow = m_pShowExampleCB->IsChecked()
        && pExampleFrame && pExampleFrame->IsServiceAvailable();

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
        rName, nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL);
    if(nId != USHRT_MAX &&
        ! rSh.GetTextCollFromPool(nId)->IsAssignedToListLevelOfOutlineStyle())
        return true;

    return false;
}

class SwIndexTreeLB : public SvSimpleTable
{
public:
    SwIndexTreeLB(SvSimpleTableContainer& rParent, WinBits nBits = 0);
    virtual void KeyInput( const KeyEvent& rKEvt ) override;
    virtual void Resize() override;
    virtual sal_IntPtr GetTabPos( SvTreeListEntry*, SvLBoxTab* ) override;
    void setColSizes();
};

SwIndexTreeLB::SwIndexTreeLB(SvSimpleTableContainer& rParent, WinBits nBits)
    : SvSimpleTable(rParent, nBits)
{
    HeaderBar& rStylesHB = GetTheHeaderBar();
    rStylesHB.SetStyle(rStylesHB.GetStyle()|WB_BUTTONSTYLE);
    SetStyle(GetStyle() & ~(WB_AUTOHSCROLL|WB_HSCROLL));
}

sal_IntPtr SwIndexTreeLB::GetTabPos( SvTreeListEntry* pEntry, SvLBoxTab* pTab)
{
    sal_IntPtr nData = reinterpret_cast<sal_IntPtr>(pEntry->GetUserData());
    if(nData != USHRT_MAX)
    {
        HeaderBar& rStylesHB = GetTheHeaderBar();
        sal_IntPtr  nPos = rStylesHB.GetItemRect( static_cast< sal_uInt16 >(2 + nData) ).TopLeft().X();
        nData = nPos;
    }
    else
        nData = 0;
    nData += pTab->GetPos();
    return nData;
}

void SwIndexTreeLB::KeyInput( const KeyEvent& rKEvt )
{
    SvTreeListEntry* pEntry = FirstSelected();
    vcl::KeyCode aCode = rKEvt.GetKeyCode();
    bool bChanged = false;
    if(pEntry)
    {
        sal_IntPtr nLevel = reinterpret_cast<sal_IntPtr>(pEntry->GetUserData());
        if(aCode.GetCode() == KEY_ADD )
        {
            if(nLevel < MAXLEVEL - 1)
                nLevel++;
            else if(nLevel == USHRT_MAX)
                nLevel = 0;
            bChanged = true;
        }
        else if(aCode.GetCode() == KEY_SUBTRACT)
        {
            if(!nLevel)
                nLevel = USHRT_MAX;
            else if(nLevel != USHRT_MAX)
                nLevel--;
            bChanged = true;
        }
        if(bChanged)
        {
            pEntry->SetUserData(reinterpret_cast<void*>(nLevel));
            Invalidate();
        }
    }
    if(!bChanged)
        SvTreeListBox::KeyInput(rKEvt);
}

void SwIndexTreeLB::Resize()
{
    SvSimpleTable::Resize();
    setColSizes();
}

void SwIndexTreeLB::setColSizes()
{
    HeaderBar &rHB = GetTheHeaderBar();
    if (rHB.GetItemCount() < MAXLEVEL+1)
        return;

    long nWidth = rHB.GetSizePixel().Width();
    nWidth /= 14;
    nWidth--;

    long nTabs_Impl[MAXLEVEL+2];

    nTabs_Impl[0] = MAXLEVEL+1;
    nTabs_Impl[1] = 3 * nWidth;

    for(sal_uInt16 i = 1; i <= MAXLEVEL; ++i)
        nTabs_Impl[i+1] = nTabs_Impl[i] + nWidth;
    SvSimpleTable::SetTabs(&nTabs_Impl[0], MAP_PIXEL);
}

class SwAddStylesDlg_Impl : public SfxModalDialog
{
    VclPtr<OKButton>       m_pOk;

    VclPtr<SwIndexTreeLB>  m_pHeaderTree;
    VclPtr<PushButton>     m_pLeftPB;
    VclPtr<PushButton>     m_pRightPB;

    OUString*       pStyleArr;

    DECL_LINK_TYPED(OkHdl, Button*, void);
    DECL_LINK_TYPED(LeftRightHdl, Button*, void);
    DECL_LINK_TYPED(HeaderDragHdl, HeaderBar*, void);

public:
    SwAddStylesDlg_Impl(vcl::Window* pParent, SwWrtShell& rWrtSh, OUString rStringArr[]);
    virtual ~SwAddStylesDlg_Impl();
    virtual void dispose() override;
};

SwAddStylesDlg_Impl::SwAddStylesDlg_Impl(vcl::Window* pParent,
            SwWrtShell& rWrtSh, OUString rStringArr[])
    : SfxModalDialog(pParent, "AssignStylesDialog",
        "modules/swriter/ui/assignstylesdialog.ui")
    , pStyleArr(rStringArr)
{
    get(m_pOk, "ok");
    get(m_pLeftPB, "left");
    get(m_pRightPB, "right");
    OUString sHBFirst = get<FixedText>("notapplied")->GetText();
    SvSimpleTableContainer *pHeaderTreeContainer = get<SvSimpleTableContainer>("styles");
    Size aSize = pHeaderTreeContainer->LogicToPixel(Size(273, 164), MAP_APPFONT);
    pHeaderTreeContainer->set_width_request(aSize.Width());
    pHeaderTreeContainer->set_height_request(aSize.Height());
    m_pHeaderTree = VclPtr<SwIndexTreeLB>::Create(*pHeaderTreeContainer);

    m_pOk->SetClickHdl(LINK(this, SwAddStylesDlg_Impl, OkHdl));
    m_pLeftPB->SetClickHdl(LINK(this, SwAddStylesDlg_Impl, LeftRightHdl));
    m_pRightPB->SetClickHdl(LINK(this, SwAddStylesDlg_Impl, LeftRightHdl));

    HeaderBar& rHB = m_pHeaderTree->GetTheHeaderBar();
    rHB.SetEndDragHdl(LINK(this, SwAddStylesDlg_Impl, HeaderDragHdl));

    for(sal_uInt16 i = 1; i <= MAXLEVEL; ++i)
        sHBFirst += "\t" + OUString::number(i);
    m_pHeaderTree->InsertHeaderEntry(sHBFirst);
    m_pHeaderTree->setColSizes();

    m_pHeaderTree->SetStyle(m_pHeaderTree->GetStyle()|WB_CLIPCHILDREN|WB_SORT);
    m_pHeaderTree->GetModel()->SetSortMode(SortAscending);
    for (sal_uInt16 i = 0; i < MAXLEVEL; ++i)
    {
        OUString sStyles(rStringArr[i]);
        for(sal_Int32 nToken = 0;
            nToken < comphelper::string::getTokenCount(sStyles, TOX_STYLE_DELIMITER);
            ++nToken)
        {
            const OUString sTmp(sStyles.getToken(nToken, TOX_STYLE_DELIMITER));
            SvTreeListEntry* pEntry = m_pHeaderTree->InsertEntry(sTmp);
            pEntry->SetUserData(reinterpret_cast<void*>(i));
        }
    }
    // now the other styles

    const SwTextFormatColl *pColl   = nullptr;
    const sal_uInt16 nSz = rWrtSh.GetTextFormatCollCount();

    for ( sal_uInt16 j = 0;j < nSz; ++j )
    {
        pColl = &rWrtSh.GetTextFormatColl(j);
        if(pColl->IsDefault())
            continue;

        const OUString aName = pColl->GetName();
        if (!aName.isEmpty())
        {
            SvTreeListEntry* pEntry = m_pHeaderTree->First();
            while (pEntry && SvTabListBox::GetEntryText(pEntry, 0) != aName)
            {
                pEntry = m_pHeaderTree->Next(pEntry);
            }
            if (!pEntry)
            {
                m_pHeaderTree->InsertEntry(aName)->SetUserData(reinterpret_cast<void*>(USHRT_MAX));
            }
        }
    }
    m_pHeaderTree->GetModel()->Resort();
}

SwAddStylesDlg_Impl::~SwAddStylesDlg_Impl()
{
    disposeOnce();
}

void SwAddStylesDlg_Impl::dispose()
{
    m_pHeaderTree.disposeAndClear();
    m_pOk.clear();
    m_pLeftPB.clear();
    m_pRightPB.clear();
    SfxModalDialog::dispose();
}

IMPL_LINK_NOARG_TYPED(SwAddStylesDlg_Impl, OkHdl, Button*, void)
{
    for(sal_uInt16 i = 0; i < MAXLEVEL; i++)
        pStyleArr[i].clear();

    SvTreeListEntry* pEntry = m_pHeaderTree->First();
    while(pEntry)
    {
        sal_IntPtr nLevel = reinterpret_cast<sal_IntPtr>(pEntry->GetUserData());
        if(nLevel != USHRT_MAX)
        {
            if(!pStyleArr[nLevel].isEmpty())
                pStyleArr[nLevel] += OUStringLiteral1<TOX_STYLE_DELIMITER>();
            pStyleArr[nLevel] += SvTabListBox::GetEntryText(pEntry, 0);
        }
        pEntry = m_pHeaderTree->Next(pEntry);
    }

    //TODO write back style names
    EndDialog(RET_OK);
}

IMPL_LINK_NOARG_TYPED(SwAddStylesDlg_Impl, HeaderDragHdl, HeaderBar*, void)
{
    m_pHeaderTree->Invalidate();
}

IMPL_LINK_TYPED(SwAddStylesDlg_Impl, LeftRightHdl, Button*, pBtn, void)
{
    bool bLeft = pBtn == m_pLeftPB;
    SvTreeListEntry* pEntry = m_pHeaderTree->FirstSelected();
    if(pEntry)
    {
        sal_IntPtr nLevel = reinterpret_cast<sal_IntPtr>(pEntry->GetUserData());
        if(bLeft)
        {
            if(!nLevel)
                nLevel = USHRT_MAX;
            else if(nLevel != USHRT_MAX)
                nLevel--;
        }
        else
        {
            if(nLevel < MAXLEVEL - 1)
                nLevel++;
            else if(nLevel == USHRT_MAX)
                nLevel = 0;
        }
        pEntry->SetUserData(reinterpret_cast<void*>(nLevel));
        m_pHeaderTree->Invalidate();
    }
}

SwTOXSelectTabPage::SwTOXSelectTabPage(vcl::Window* pParent, const SfxItemSet& rAttrSet)
    : SfxTabPage(pParent, "TocIndexPage",
        "modules/swriter/ui/tocindexpage.ui", &rAttrSet)
    , aFromNames(SW_RES(RES_SRCTYPES))
    , pIndexRes(nullptr)
    , sAutoMarkType(SW_RESSTR(STR_AUTOMARK_TYPE))
    , m_bWaitingInitialSettings(true)
{
    get(m_pTitleED, "title");
    get(m_pTypeFT, "typeft");
    get(m_pTypeLB, "type");
    get(m_pReadOnlyCB, "readonly");

    get(m_pAreaFrame, "areaframe");
    get(m_pAreaLB, "scope");
    get(m_pLevelFT, "levelft");
    get(m_pLevelNF, "level");

    get(m_pCreateFrame, "createframe");
    get(m_pFromHeadingsCB, "fromheadings");
    get(m_pAddStylesCB, "addstylescb");
    sAddStyleUser = get<vcl::Window>("stylescb")->GetText();
    get(m_pAddStylesPB, "styles");
    get(m_pFromTablesCB, "fromtables");
    get(m_pFromFramesCB, "fromframes");
    get(m_pFromGraphicsCB, "fromgraphics");
    get(m_pFromOLECB, "fromoles");
    get(m_pLevelFromChapterCB, "uselevel");

    get(m_pFromCaptionsRB, "captions");
    get(m_pFromObjectNamesRB, "objnames");

    get(m_pCaptionSequenceFT, "categoryft");
    get(m_pCaptionSequenceLB, "category");
    get(m_pDisplayTypeFT, "displayft");
    get(m_pDisplayTypeLB, "display");
    get(m_pTOXMarksCB, "indexmarks");

    get(m_pIdxOptionsFrame, "optionsframe");
    get(m_pCollectSameCB, "combinesame");
    get(m_pUseFFCB, "useff");
    get(m_pUseDashCB, "usedash");
    get(m_pCaseSensitiveCB, "casesens");
    get(m_pInitialCapsCB, "initcaps");
    get(m_pKeyAsEntryCB, "keyasentry");
    get(m_pFromFileCB, "fromfile");
    get(m_pAutoMarkPB, "file");

    get(m_pFromObjFrame, "objectframe");
    get(m_pFromObjCLB, "objects");

    get(m_pAuthorityFrame, "authframe");
    get(m_pSequenceCB, "numberentries");
    get(m_pBracketLB, "brackets");

    get(m_pSortFrame, "sortframe");
    get(m_pLanguageLB, "lang");
    get(m_pSortAlgorithmLB, "keytype");

    pIndexEntryWrapper = new IndexEntrySupplierWrapper();

    m_pLanguageLB->SetLanguageList( SvxLanguageListFlags::ALL | SvxLanguageListFlags::ONLY_KNOWN,
                                 false );

    //Default mode is arranged to be the tallest mode
    //of alphabetical index, lock that height in now
    LanguageHdl(nullptr); //fill sort algorithm list
    Size aPrefSize(get_preferred_size());
    set_height_request(aPrefSize.Height());

    sAddStyleContent = m_pAddStylesCB->GetText();

    ResStringArray& rNames = aFromNames.GetNames();
    for(sal_uInt32 i = 0; i < rNames.Count(); i++)
    {
        m_pFromObjCLB->InsertEntry(rNames.GetString(i));
        m_pFromObjCLB->SetEntryData( i, reinterpret_cast<void*>(rNames.GetValue(i)) );
    }

    SetExchangeSupport();
    m_pTypeLB->SetSelectHdl(LINK(this, SwTOXSelectTabPage, TOXTypeHdl));

    m_pAddStylesPB->SetClickHdl(LINK(this, SwTOXSelectTabPage, AddStylesHdl));

    PopupMenu*  pMenu = m_pAutoMarkPB->GetPopupMenu();
    pMenu->SetActivateHdl(LINK(this, SwTOXSelectTabPage, MenuEnableHdl));
    pMenu->SetSelectHdl(LINK(this, SwTOXSelectTabPage, MenuExecuteHdl));

    Link<Button*,void> aLk =  LINK(this, SwTOXSelectTabPage, CheckBoxHdl);
    m_pAddStylesCB->SetClickHdl(aLk);
    m_pFromHeadingsCB->SetClickHdl(aLk);
    m_pTOXMarksCB->SetClickHdl(aLk);
    m_pFromFileCB->SetClickHdl(aLk);
    m_pCollectSameCB->SetClickHdl(aLk);
    m_pUseFFCB->SetClickHdl(aLk);
    m_pUseDashCB->SetClickHdl(aLk);
    m_pInitialCapsCB->SetClickHdl(aLk);
    m_pKeyAsEntryCB->SetClickHdl(aLk);

    Link<Edit&,void> aModifyLk = LINK(this, SwTOXSelectTabPage, ModifyHdl);
    m_pTitleED->SetModifyHdl(aModifyLk);
    m_pLevelNF->SetModifyHdl(aModifyLk);
    m_pSortAlgorithmLB->SetSelectHdl(LINK(this, SwTOXSelectTabPage, ModifyListBoxHdl));

    aLk =  LINK(this, SwTOXSelectTabPage, RadioButtonHdl);
    m_pFromCaptionsRB->SetClickHdl(aLk);
    m_pFromObjectNamesRB->SetClickHdl(aLk);
    RadioButtonHdl(m_pFromCaptionsRB);

    m_pLanguageLB->SetSelectHdl(LINK(this, SwTOXSelectTabPage, LanguageListBoxHdl));
    m_pTypeLB->SelectEntryPos(0);
    m_pTitleED->SaveValue();
}

SwTOXSelectTabPage::~SwTOXSelectTabPage()
{
    disposeOnce();
}

void SwTOXSelectTabPage::dispose()
{
    delete pIndexRes;
    delete pIndexEntryWrapper;
    m_pTitleED.clear();
    m_pTypeFT.clear();
    m_pTypeLB.clear();
    m_pReadOnlyCB.clear();
    m_pAreaFrame.clear();
    m_pAreaLB.clear();
    m_pLevelFT.clear();
    m_pLevelNF.clear();
    m_pCreateFrame.clear();
    m_pFromHeadingsCB.clear();
    m_pAddStylesCB.clear();
    m_pAddStylesPB.clear();
    m_pFromTablesCB.clear();
    m_pFromFramesCB.clear();
    m_pFromGraphicsCB.clear();
    m_pFromOLECB.clear();
    m_pLevelFromChapterCB.clear();
    m_pFromCaptionsRB.clear();
    m_pFromObjectNamesRB.clear();
    m_pCaptionSequenceFT.clear();
    m_pCaptionSequenceLB.clear();
    m_pDisplayTypeFT.clear();
    m_pDisplayTypeLB.clear();
    m_pTOXMarksCB.clear();
    m_pIdxOptionsFrame.clear();
    m_pCollectSameCB.clear();
    m_pUseFFCB.clear();
    m_pUseDashCB.clear();
    m_pCaseSensitiveCB.clear();
    m_pInitialCapsCB.clear();
    m_pKeyAsEntryCB.clear();
    m_pFromFileCB.clear();
    m_pAutoMarkPB.clear();
    m_pFromObjCLB.clear();
    m_pFromObjFrame.clear();
    m_pSequenceCB.clear();
    m_pBracketLB.clear();
    m_pAuthorityFrame.clear();
    m_pSortFrame.clear();
    m_pLanguageLB.clear();
    m_pSortAlgorithmLB.clear();
    SfxTabPage::dispose();
}

void SwTOXSelectTabPage::SetWrtShell(SwWrtShell& rSh)
{
    const sal_uInt16 nUserTypeCount = rSh.GetTOXTypeCount(TOX_USER);
    if(nUserTypeCount > 1)
    {
        //insert all new user indexes names after the standard user index
        sal_Int32 nPos = m_pTypeLB->GetEntryPos(reinterpret_cast<void*>((sal_uInt32)TO_USER)) + 1;
        for(sal_uInt16 nUser = 1; nUser < nUserTypeCount; nUser++)
        {
            nPos = m_pTypeLB->InsertEntry(rSh.GetTOXType(TOX_USER, nUser)->GetTypeName(), nPos);
            sal_uIntPtr nEntryData = nUser << 8;
            nEntryData |= TO_USER;
            m_pTypeLB->SetEntryData(nPos, reinterpret_cast<void*>(nEntryData));
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
    CurTOXType eCurType (eSet, 0);

    sal_IntPtr nData = lcl_TOXTypesToUserData(eCurType);
    m_pTypeLB->SelectEntryPos(m_pTypeLB->GetEntryPos(reinterpret_cast<void*>(nData)));
    m_pTypeFT->Enable(false);
    m_pTypeLB->Enable(false);
    TOXTypeHdl(*m_pTypeLB);
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
    m_pReadOnlyCB->Check(rDesc.IsReadonly());
    if(m_pTitleED->GetText() == m_pTitleED->GetSavedValue())
    {
        if(rDesc.GetTitle())
            m_pTitleED->SetText(*rDesc.GetTitle());
        else
            m_pTitleED->SetText(aEmptyOUStr);
        m_pTitleED->SaveValue();
    }

    m_pAreaLB->SelectEntryPos(rDesc.IsFromChapter() ? 1 : 0);

    if(aCurType.eType != TOX_INDEX)
        m_pLevelNF->SetValue(rDesc.GetLevel());   //content, user

    sal_uInt16 nCreateType = rDesc.GetContentOptions();

    //user + content
    bool bHasStyleNames = false;

    for( sal_uInt16 i = 0; i < MAXLEVEL; i++)
        if(!rDesc.GetStyleNames(i).isEmpty())
        {
            bHasStyleNames = true;
            break;
        }
    m_pAddStylesCB->Check(bHasStyleNames && (nCreateType & nsSwTOXElement::TOX_TEMPLATE));

    m_pFromOLECB->     Check( 0 != (nCreateType & nsSwTOXElement::TOX_OLE) );
    m_pFromTablesCB->  Check( 0 != (nCreateType & nsSwTOXElement::TOX_TABLE) );
    m_pFromGraphicsCB->Check( 0 != (nCreateType & nsSwTOXElement::TOX_GRAPHIC) );
    m_pFromFramesCB->  Check( 0 != (nCreateType & nsSwTOXElement::TOX_FRAME) );

    m_pLevelFromChapterCB->Check(rDesc.IsLevelFromChapter());

    //all but illustration and table
    m_pTOXMarksCB->Check( 0 != (nCreateType & nsSwTOXElement::TOX_MARK) );

    //content
    if(TOX_CONTENT == aCurType.eType)
    {
        m_pFromHeadingsCB->Check( 0 != (nCreateType & nsSwTOXElement::TOX_OUTLINELEVEL) );
        m_pAddStylesCB->SetText(sAddStyleContent);
        m_pAddStylesPB->Enable(m_pAddStylesCB->IsChecked());
    }
    //index only
    else if(TOX_INDEX == aCurType.eType)
    {
        const sal_uInt16 nIndexOptions = rDesc.GetIndexOptions();
        m_pCollectSameCB->     Check( 0 != (nIndexOptions & nsSwTOIOptions::TOI_SAME_ENTRY) );
        m_pUseFFCB->           Check( 0 != (nIndexOptions & nsSwTOIOptions::TOI_FF) );
        m_pUseDashCB->         Check( 0 != (nIndexOptions & nsSwTOIOptions::TOI_DASH) );
        if(m_pUseFFCB->IsChecked())
            m_pUseDashCB->Enable(false);
        else if(m_pUseDashCB->IsChecked())
            m_pUseFFCB->Enable(false);

        m_pCaseSensitiveCB->   Check( 0 != (nIndexOptions & nsSwTOIOptions::TOI_CASE_SENSITIVE) );
        m_pInitialCapsCB->     Check( 0 != (nIndexOptions & nsSwTOIOptions::TOI_INITIAL_CAPS) );
        m_pKeyAsEntryCB->      Check( 0 != (nIndexOptions & nsSwTOIOptions::TOI_KEY_AS_ENTRY) );
    }
    else if(TOX_ILLUSTRATIONS == aCurType.eType ||
        TOX_TABLES == aCurType.eType)
    {
        m_pFromObjectNamesRB->Check(rDesc.IsCreateFromObjectNames());
        m_pFromCaptionsRB->Check(!rDesc.IsCreateFromObjectNames());
        m_pCaptionSequenceLB->SelectEntry(rDesc.GetSequenceName());
        m_pDisplayTypeLB->SelectEntryPos( static_cast< sal_Int32 >(rDesc.GetCaptionDisplay()) );
        RadioButtonHdl(m_pFromCaptionsRB);

    }
    else if(TOX_OBJECTS == aCurType.eType)
    {
        long nOLEData = rDesc.GetOLEOptions();
        for(sal_uLong nFromObj = 0; nFromObj < m_pFromObjCLB->GetEntryCount(); nFromObj++)
        {
            sal_IntPtr nData = reinterpret_cast<sal_IntPtr>(m_pFromObjCLB->GetEntryData(nFromObj));
            m_pFromObjCLB->CheckEntryPos(nFromObj, 0 != (nData & nOLEData));
        }
    }
    else if(TOX_AUTHORITIES == aCurType.eType)
    {
        const OUString sBrackets(rDesc.GetAuthBrackets());
        if(sBrackets.isEmpty() || sBrackets == "  ")
            m_pBracketLB->SelectEntryPos(0);
        else
            m_pBracketLB->SelectEntry(sBrackets);
        m_pSequenceCB->Check(rDesc.IsAuthSequence());
    }
    m_pAutoMarkPB->Enable(m_pFromFileCB->IsChecked());

    for(sal_uInt16 i = 0; i < MAXLEVEL; i++)
        aStyleArr[i] = rDesc.GetStyleNames(i);

    m_pLanguageLB->SelectLanguage(rDesc.GetLanguage());
    LanguageHdl(nullptr);
    for( sal_Int32 nCnt = 0; nCnt < m_pSortAlgorithmLB->GetEntryCount(); ++nCnt )
    {
        const OUString* pEntryData = static_cast<const OUString*>(m_pSortAlgorithmLB->GetEntryData( nCnt ));
        OSL_ENSURE(pEntryData, "no entry data available");
        if( pEntryData && *pEntryData == rDesc.GetSortAlgorithm())
        {
            m_pSortAlgorithmLB->SelectEntryPos( nCnt );
            break;
        }
    }
}

void SwTOXSelectTabPage::FillTOXDescription()
{
    SwMultiTOXTabDialog* pTOXDlg = static_cast<SwMultiTOXTabDialog*>(GetTabDialog());
    CurTOXType aCurType = pTOXDlg->GetCurrentTOXType();
    SwTOXDescription& rDesc = pTOXDlg->GetTOXDescription(aCurType);
    rDesc.SetTitle(m_pTitleED->GetText());
    rDesc.SetFromChapter(1 == m_pAreaLB->GetSelectEntryPos());
    sal_uInt16 nContentOptions = 0;
    if(m_pTOXMarksCB->IsVisible() && m_pTOXMarksCB->IsChecked())
        nContentOptions |= nsSwTOXElement::TOX_MARK;

    sal_uInt16 nIndexOptions = rDesc.GetIndexOptions()&nsSwTOIOptions::TOI_ALPHA_DELIMITTER;
    switch(rDesc.GetTOXType())
    {
        case TOX_CONTENT:
            if(m_pFromHeadingsCB->IsChecked())
                nContentOptions |= nsSwTOXElement::TOX_OUTLINELEVEL;
        break;
        case TOX_USER:
        {
            rDesc.SetTOUName(m_pTypeLB->GetSelectEntry());

            if(m_pFromOLECB->IsChecked())
                nContentOptions |= nsSwTOXElement::TOX_OLE;
            if(m_pFromTablesCB->IsChecked())
                nContentOptions |= nsSwTOXElement::TOX_TABLE;
            if(m_pFromFramesCB->IsChecked())
                nContentOptions |= nsSwTOXElement::TOX_FRAME;
            if(m_pFromGraphicsCB->IsChecked())
                nContentOptions |= nsSwTOXElement::TOX_GRAPHIC;
        }
        break;
        case TOX_INDEX:
        {
            nContentOptions = nsSwTOXElement::TOX_MARK;

            if(m_pCollectSameCB->IsChecked())
                nIndexOptions |= nsSwTOIOptions::TOI_SAME_ENTRY;
            if(m_pUseFFCB->IsChecked())
                nIndexOptions |= nsSwTOIOptions::TOI_FF;
            if(m_pUseDashCB->IsChecked())
                nIndexOptions |= nsSwTOIOptions::TOI_DASH;
            if(m_pCaseSensitiveCB->IsChecked())
                nIndexOptions |= nsSwTOIOptions::TOI_CASE_SENSITIVE;
            if(m_pInitialCapsCB->IsChecked())
                nIndexOptions |= nsSwTOIOptions::TOI_INITIAL_CAPS;
            if(m_pKeyAsEntryCB->IsChecked())
                nIndexOptions |= nsSwTOIOptions::TOI_KEY_AS_ENTRY;
            if(m_pFromFileCB->IsChecked())
                rDesc.SetAutoMarkURL(sAutoMarkURL);
            else
                rDesc.SetAutoMarkURL(aEmptyOUStr);
        }
        break;
        case TOX_ILLUSTRATIONS:
        case TOX_TABLES :
            rDesc.SetCreateFromObjectNames(m_pFromObjectNamesRB->IsChecked());
            rDesc.SetSequenceName(m_pCaptionSequenceLB->GetSelectEntry());
            rDesc.SetCaptionDisplay((SwCaptionDisplay)m_pDisplayTypeLB->GetSelectEntryPos());
        break;
        case TOX_OBJECTS:
        {
            long nOLEData = 0;
            for(sal_uLong i = 0; i < m_pFromObjCLB->GetEntryCount(); i++)
            {
                if(m_pFromObjCLB->IsChecked(i))
                {
                    sal_IntPtr nData = reinterpret_cast<sal_IntPtr>(m_pFromObjCLB->GetEntryData(i));
                    nOLEData |= nData;
                }
            }
            rDesc.SetOLEOptions((sal_uInt16)nOLEData);
        }
        break;
        case TOX_AUTHORITIES:
        case TOX_BIBLIOGRAPHY :
        {
            if(m_pBracketLB->GetSelectEntryPos())
                rDesc.SetAuthBrackets(m_pBracketLB->GetSelectEntry());
            else
                rDesc.SetAuthBrackets(aEmptyOUStr);
            rDesc.SetAuthSequence(m_pSequenceCB->IsChecked());
        }
        break;
        case TOX_CITATION :
        break;
    }

    rDesc.SetLevelFromChapter(  m_pLevelFromChapterCB->IsVisible() &&
                                m_pLevelFromChapterCB->IsChecked());
    if(m_pTOXMarksCB->IsChecked() && m_pTOXMarksCB->IsVisible())
        nContentOptions |= nsSwTOXElement::TOX_MARK;
    if(m_pFromHeadingsCB->IsChecked() && m_pFromHeadingsCB->IsVisible())
        nContentOptions |= nsSwTOXElement::TOX_OUTLINELEVEL;
    if(m_pAddStylesCB->IsChecked() && m_pAddStylesCB->IsVisible())
        nContentOptions |= nsSwTOXElement::TOX_TEMPLATE;

    rDesc.SetContentOptions(nContentOptions);
    rDesc.SetIndexOptions(nIndexOptions);
    rDesc.SetLevel( static_cast< sal_uInt8 >(m_pLevelNF->GetValue()) );

    rDesc.SetReadonly(m_pReadOnlyCB->IsChecked());

    for(sal_uInt16 i = 0; i < MAXLEVEL; i++)
        rDesc.SetStyleNames(aStyleArr[i], i);

    rDesc.SetLanguage(m_pLanguageLB->GetSelectLanguage());
    const OUString* pEntryData = static_cast<const OUString*>(m_pSortAlgorithmLB->GetSelectEntryData());
    OSL_ENSURE(pEntryData, "no entry data available");
    if(pEntryData)
        rDesc.SetSortAlgorithm(*pEntryData);
}

void SwTOXSelectTabPage::Reset( const SfxItemSet* )
{
    SwMultiTOXTabDialog* pTOXDlg = static_cast<SwMultiTOXTabDialog*>(GetTabDialog());
    SwWrtShell& rSh = pTOXDlg->GetWrtShell();
    const CurTOXType aCurType = pTOXDlg->GetCurrentTOXType();
    sal_IntPtr nData = lcl_TOXTypesToUserData(aCurType);
    m_pTypeLB->SelectEntryPos(m_pTypeLB->GetEntryPos(reinterpret_cast<void*>(nData)));

    sAutoMarkURL = INetURLObject::decode( rSh.GetTOIAutoMarkURL(),
                                           INetURLObject::DECODE_UNAMBIGUOUS );
    m_pFromFileCB->Check( !sAutoMarkURL.isEmpty() );

    m_pCaptionSequenceLB->Clear();
    const size_t nCount = rSh.GetFieldTypeCount(RES_SETEXPFLD);
    for (size_t i = 0; i < nCount; ++i)
    {
        SwFieldType *pType = rSh.GetFieldType( i, RES_SETEXPFLD );
        if( pType->Which() == RES_SETEXPFLD &&
            static_cast<SwSetExpFieldType *>( pType)->GetType() & nsSwGetSetExpType::GSE_SEQ )
            m_pCaptionSequenceLB->InsertEntry(pType->GetName());
    }

    if(pTOXDlg->IsTOXEditMode())
    {
        m_pTypeFT->Enable(false);
        m_pTypeLB->Enable(false);
    }

    if(!m_bWaitingInitialSettings)
    {
        // save current values into the proper TOXDescription
        FillTOXDescription();
    }
    m_bWaitingInitialSettings = false;

    TOXTypeHdl(*m_pTypeLB);
    CheckBoxHdl(m_pAddStylesCB);
}

void SwTOXSelectTabPage::ActivatePage( const SfxItemSet& )
{
    //nothing to do
}

SfxTabPage::sfxpg SwTOXSelectTabPage::DeactivatePage( SfxItemSet* _pSet )
{
    if(_pSet)
        _pSet->Put(SfxUInt16Item(FN_PARAM_TOX_TYPE,
            (sal_uInt16)reinterpret_cast<sal_IntPtr>(m_pTypeLB->GetSelectEntryData())));
    FillTOXDescription();
    return LEAVE_PAGE;
}

VclPtr<SfxTabPage> SwTOXSelectTabPage::Create( vcl::Window* pParent, const SfxItemSet* rAttrSet)
{
    return VclPtr<SwTOXSelectTabPage>::Create(pParent, *rAttrSet);
}

IMPL_LINK_TYPED(SwTOXSelectTabPage, TOXTypeHdl, ListBox&, rBox, void)
{
    SwMultiTOXTabDialog* pTOXDlg = static_cast<SwMultiTOXTabDialog*>(GetTabDialog());
    const sal_uInt16 nType =  sal::static_int_cast< sal_uInt16 >(reinterpret_cast< sal_uIntPtr >(
                                rBox.GetSelectEntryData()));
    CurTOXType eCurType = lcl_UserData2TOXTypes(nType);
    pTOXDlg->SetCurrentTOXType(eCurType);

    m_pAreaLB->Show( 0 != (nType & (TO_CONTENT|TO_ILLUSTRATION|TO_USER|TO_INDEX|TO_TABLE|TO_OBJECT)) );
    m_pLevelFT->Show( 0 != (nType & (TO_CONTENT)) );
    m_pLevelNF->Show( 0 != (nType & (TO_CONTENT)) );
    m_pLevelFromChapterCB->Show( 0 != (nType & (TO_USER)) );
    m_pAreaFrame->Show( 0 != (nType & (TO_CONTENT|TO_ILLUSTRATION|TO_USER|TO_INDEX|TO_TABLE|TO_OBJECT)) );

    m_pFromHeadingsCB->Show( 0 != (nType & (TO_CONTENT)) );
    m_pAddStylesCB->Show( 0 != (nType & (TO_CONTENT|TO_USER)) );
    m_pAddStylesPB->Show( 0 != (nType & (TO_CONTENT|TO_USER)) );

    m_pFromTablesCB->Show( 0 != (nType & (TO_USER)) );
    m_pFromFramesCB->Show( 0 != (nType & (TO_USER)) );
    m_pFromGraphicsCB->Show( 0 != (nType & (TO_USER)) );
    m_pFromOLECB->Show( 0 != (nType & (TO_USER)) );

    m_pFromCaptionsRB->Show( 0 != (nType & (TO_ILLUSTRATION|TO_TABLE)) );
    m_pFromObjectNamesRB->Show( 0 != (nType & (TO_ILLUSTRATION|TO_TABLE)) );

    m_pTOXMarksCB->Show( 0 != (nType & (TO_CONTENT|TO_USER)) );

    m_pCreateFrame->Show( 0 != (nType & (TO_CONTENT|TO_ILLUSTRATION|TO_USER|TO_TABLE)) );
    m_pCaptionSequenceFT->Show( 0 != (nType & (TO_ILLUSTRATION|TO_TABLE)) );
    m_pCaptionSequenceLB->Show( 0 != (nType & (TO_ILLUSTRATION|TO_TABLE)) );
    m_pDisplayTypeFT->Show( 0 != (nType & (TO_ILLUSTRATION|TO_TABLE)) );
    m_pDisplayTypeLB->Show( 0 != (nType & (TO_ILLUSTRATION|TO_TABLE)) );

    m_pAuthorityFrame->Show( 0 != (nType & TO_AUTHORITIES) );

    bool bEnableSortLanguage = 0 != (nType & (TO_INDEX|TO_AUTHORITIES));
    m_pSortFrame->Show(bEnableSortLanguage);

    if( nType & TO_ILLUSTRATION )
    {
        m_pCaptionSequenceLB->SelectEntry( SwStyleNameMapper::GetUIName(
                                    RES_POOLCOLL_LABEL_ABB, OUString() ));
    }
    else if( nType & TO_TABLE )
    {
        m_pCaptionSequenceLB->SelectEntry( SwStyleNameMapper::GetUIName(
                                    RES_POOLCOLL_LABEL_TABLE, OUString() ));
    }
    else if( nType & TO_USER )
    {
        m_pAddStylesCB->SetText(sAddStyleUser);
    }

    m_pIdxOptionsFrame->Show( 0 != (nType & TO_INDEX) );

    //object index
    m_pFromObjFrame->Show( 0 != (nType & TO_OBJECT) );

    //set control values from the proper TOXDescription
    {
        ApplyTOXDescription();
    }
    ModifyHdl(*m_pTitleED);
}

IMPL_LINK_NOARG_TYPED(SwTOXSelectTabPage, ModifyListBoxHdl, ListBox&, void)
{
    ModifyHdl(*m_pTitleED);
}
IMPL_LINK_NOARG_TYPED(SwTOXSelectTabPage, ModifyHdl, Edit&, void)
{
    if(!m_bWaitingInitialSettings)
    {
        FillTOXDescription();
        SwMultiTOXTabDialog* pTOXDlg = static_cast<SwMultiTOXTabDialog*>(GetTabDialog());
        pTOXDlg->CreateOrUpdateExample(pTOXDlg->GetCurrentTOXType().eType, TOX_PAGE_SELECT);
    }
}

IMPL_LINK_TYPED(SwTOXSelectTabPage, CheckBoxHdl, Button*, pButton, void )
{
    CheckBox* pBox = static_cast<CheckBox*>(pButton);
    SwMultiTOXTabDialog* pTOXDlg = static_cast<SwMultiTOXTabDialog*>(GetTabDialog());
    const CurTOXType aCurType = pTOXDlg->GetCurrentTOXType();
    if(TOX_CONTENT == aCurType.eType)
    {
        //at least one of the three CheckBoxes must be checked
        if(!m_pAddStylesCB->IsChecked() && !m_pFromHeadingsCB->IsChecked() && !m_pTOXMarksCB->IsChecked())
        {
            //TODO: InfoBox?
            pBox->Check();
        }
        m_pAddStylesPB->Enable(m_pAddStylesCB->IsChecked());
    }
    if(TOX_USER == aCurType.eType)
    {
        m_pAddStylesPB->Enable(m_pAddStylesCB->IsChecked());
    }
    else if(TOX_INDEX == aCurType.eType)
    {
        m_pAutoMarkPB->Enable(m_pFromFileCB->IsChecked());
        m_pUseFFCB->Enable(m_pCollectSameCB->IsChecked() && !m_pUseDashCB->IsChecked());
        m_pUseDashCB->Enable(m_pCollectSameCB->IsChecked() && !m_pUseFFCB->IsChecked());
        m_pCaseSensitiveCB->Enable(m_pCollectSameCB->IsChecked());
    }
    ModifyHdl(*m_pTitleED);
};

IMPL_LINK_NOARG_TYPED(SwTOXSelectTabPage, RadioButtonHdl, Button*, void)
{
    bool bEnable = m_pFromCaptionsRB->IsChecked();
    m_pCaptionSequenceFT->Enable(bEnable);
    m_pCaptionSequenceLB->Enable(bEnable);
    m_pDisplayTypeFT->Enable(bEnable);
    m_pDisplayTypeLB->Enable(bEnable);
    ModifyHdl(*m_pTitleED);
}

IMPL_LINK_TYPED(SwTOXSelectTabPage, LanguageListBoxHdl, ListBox&, rBox, void)
{
    LanguageHdl(&rBox);
}
void SwTOXSelectTabPage::LanguageHdl( ListBox* pBox )
{
    lang::Locale aLcl( LanguageTag( m_pLanguageLB->GetSelectLanguage() ).getLocale() );
    Sequence< OUString > aSeq = pIndexEntryWrapper->GetAlgorithmList( aLcl );

    if( !pIndexRes )
        pIndexRes = new IndexEntryResource();

    OUString sOldString;
    void* pUserData;
    if( nullptr != (pUserData = m_pSortAlgorithmLB->GetSelectEntryData()) )
        sOldString = *static_cast<OUString*>(pUserData);
    sal_Int32 nEnd = m_pSortAlgorithmLB->GetEntryCount();
    for( sal_Int32 n = 0; n < nEnd; ++n )
    {
        void* pDel = m_pSortAlgorithmLB->GetEntryData( n );
        if( nullptr != pDel )
            delete static_cast<OUString*>(pDel);
    }
    m_pSortAlgorithmLB->Clear();

    nEnd = aSeq.getLength();
    for( sal_Int32 nCnt = 0; nCnt < nEnd; ++nCnt )
    {
        const OUString sAlg(aSeq[ nCnt ]);
        const OUString sUINm = pIndexRes->GetTranslation( sAlg );
        sal_Int32 nInsPos = m_pSortAlgorithmLB->InsertEntry( sUINm );
        m_pSortAlgorithmLB->SetEntryData( nInsPos, new OUString( sAlg ));
        if( sAlg == sOldString )
            m_pSortAlgorithmLB->SelectEntryPos( nInsPos );
    }

    if( LISTBOX_ENTRY_NOTFOUND == m_pSortAlgorithmLB->GetSelectEntryPos() )
        m_pSortAlgorithmLB->SelectEntryPos( 0 );

    if(pBox)
        ModifyHdl(*m_pTitleED);
};

IMPL_LINK_TYPED(SwTOXSelectTabPage, AddStylesHdl, Button*, pButton, void)
{
    ScopedVclPtrInstance<SwAddStylesDlg_Impl> pDlg(
        pButton, static_cast<SwMultiTOXTabDialog*>(GetTabDialog())->GetWrtShell(),
        aStyleArr);
    pDlg->Execute();
    pDlg.disposeAndClear();
    ModifyHdl(*m_pTitleED);
}

IMPL_LINK_TYPED(SwTOXSelectTabPage, MenuEnableHdl, Menu*, pMenu, bool)
{
    pMenu->EnableItem("edit", !sAutoMarkURL.isEmpty());
    return false;
}

IMPL_LINK_TYPED(SwTOXSelectTabPage, MenuExecuteHdl, Menu*, pMenu, bool)
{
    const OUString sSaveAutoMarkURL = sAutoMarkURL;
    OString sIdent(pMenu->GetCurItemIdent());

    if (sIdent == "open")
    {
        sAutoMarkURL = lcl_CreateAutoMarkFileDlg(
                                sAutoMarkURL, sAutoMarkType, true);
    }
    else if ((sIdent == "new") || (sIdent == "edit"))
    {
        bool bNew = (sIdent == "new");
        if (bNew)
        {
            sAutoMarkURL = lcl_CreateAutoMarkFileDlg(
                                    sAutoMarkURL, sAutoMarkType, false);
            if( sAutoMarkURL.isEmpty() )
                return false;
        }

        VclPtrInstance<SwAutoMarkDlg_Impl> pAutoMarkDlg(
                m_pAutoMarkPB, sAutoMarkURL, bNew );

        if( RET_OK != pAutoMarkDlg->Execute() && bNew )
            sAutoMarkURL = sSaveAutoMarkURL;
    }
    return false;
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
    virtual ~SwTOXEdit() { disposeOnce(); }
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
    if((rSel.A() == rSel.B() &&
        !rSel.A()) || rSel.A() == nTextLen )
    {
        bool bCall = false;
        vcl::KeyCode aCode = rKEvt.GetKeyCode();
        if(aCode.GetCode() == KEY_RIGHT && rSel.A() == nTextLen)
        {
            bNextControl = true;
            bCall = true;
        }
        else if(aCode.GetCode() == KEY_LEFT && !rSel.A() )
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
    aSize.Width() = aTextSize.Width() + EDIT_MINWIDTH;
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
    virtual ~SwTOXButton() { disposeOnce(); }
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

VCL_BUILDER_DECL_FACTORY(SwIdxTreeListBox)
{
    WinBits nWinStyle = WB_TABSTOP;
    OString sBorder = VclBuilder::extractCustomProperty(rMap);
    if (!sBorder.isEmpty())
        nWinStyle |= WB_BORDER;
    rRet = VclPtr<SwIdxTreeListBox>::Create(pParent, nWinStyle);
}

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
                if (pItem && SV_ITEM_ID_LBOXSTRING == pItem->GetType())
                {
                    aPos = GetEntryPosition( pEntry );

                    aPos.X() = GetTabPos( pEntry, pTab );
                 Size aSize( pItem->GetSize( this, pEntry ) );

                    if((aPos.X() + aSize.Width()) > GetSizePixel().Width())
                        aSize.Width() = GetSizePixel().Width() - aPos.X();

                    aPos = OutputToScreenPixel(aPos);
                     Rectangle aItemRect( aPos, aSize );
                    Help::ShowQuickHelp( this, aItemRect, sEntry,
                            QuickHelpFlags::Left|QuickHelpFlags::VCenter );
                }
            }
        }
    }
    else
        SvTreeListBox::RequestHelp(rHEvt);
}

SwTOXEntryTabPage::SwTOXEntryTabPage(vcl::Window* pParent, const SfxItemSet& rAttrSet)
    : SfxTabPage(pParent, "TocEntriesPage",
        "modules/swriter/ui/tocentriespage.ui", &rAttrSet)
    , sDelimStr(SW_RESSTR(STR_DELIM))
    , sNoCharStyle(SW_RESSTR(STR_NO_CHAR_STYLE))
    , sNoCharSortKey(SW_RESSTR(STR_NOSORTKEY))
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
    m_pLevelLB->SetSelectionMode( SINGLE_SELECTION );
    m_pLevelLB->SetHighlightRange();   // select full width
    m_pLevelLB->Show();

    aLastTOXType.eType = (TOXTypes)USHRT_MAX;
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

    m_pEditStylePB->Enable(false);

    //fill the types in
    for (sal_uInt16 i = 0; i < AUTH_FIELD_END; ++i)
    {
        sal_Int32 nPos = m_pAuthFieldsLB->InsertEntry(SW_RESSTR(STR_AUTH_FIELD_START + i));
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
IMPL_LINK_TYPED(SwTOXEntryTabPage, ModifyClickHdl, Button*, pVoid, void)
{
    OnModify(pVoid);
}
IMPL_LINK_NOARG_TYPED(SwTOXEntryTabPage, ModifyHdl, LinkParamNone*, void)
{
    OnModify(nullptr);
}
void SwTOXEntryTabPage::OnModify(void* pVoid)
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
        const OUString sMainEntryCharStyle = rDesc.GetMainEntryCharStyle();
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
        m_pAlphaDelimCB->Check( 0 != (rDesc.GetIndexOptions() & nsSwTOIOptions::TOI_ALPHA_DELIMITTER) );
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
        bool bToxIsSequence =    TOX_ILLUSTRATIONS == aCurType.eType;

        m_pLevelLB->Clear();
        for(sal_uInt16 i = 1; i < m_pCurrentForm->GetFormMax(); i++)
        {
            if(bToxIsAuthorities)
                m_pLevelLB->InsertEntry( SwAuthorityFieldType::GetAuthTypeName(
                                            (ToxAuthorityType) (i - 1)) );
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
                                    rSh.GetFieldType(RES_AUTHORITY, aEmptyOUStr));
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
                            m_pFirstKeyLB->GetEntryPos(reinterpret_cast<void*>((sal_uIntPtr)pKey->eField)));
                        m_pFirstSortUpRB->Check(pKey->bSortAscending);
                        m_pFirstSortDownRB->Check(!pKey->bSortAscending);
                    }
                    if(1 < nKeyCount)
                    {
                        const SwTOXSortKey* pKey = pFType->GetSortKey(1);
                        m_pSecondKeyLB->SelectEntryPos(
                            m_pSecondKeyLB->GetEntryPos(reinterpret_cast<void*>((sal_uIntPtr)pKey->eField)));
                        m_pSecondSortUpRB->Check(pKey->bSortAscending);
                        m_pSecondSortDownRB->Check(!pKey->bSortAscending);
                    }
                    if(2 < nKeyCount)
                    {
                        const SwTOXSortKey* pKey = pFType->GetSortKey(2);
                        m_pThirdKeyLB->SelectEntryPos(
                            m_pThirdKeyLB->GetEntryPos(reinterpret_cast<void*>((sal_uIntPtr)pKey->eField)));
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
        m_pHyperLinkPB->Show(bToxIsContent || bToxIsSequence);
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
        const OUString sTemp(m_pMainEntryStyleLB->GetSelectEntry());
        rDesc.SetMainEntryCharStyle(sNoCharStyle == sTemp ? aEmptyOUStr : sTemp);
        sal_uInt16 nIdxOptions = rDesc.GetIndexOptions() & ~nsSwTOIOptions::TOI_ALPHA_DELIMITTER;
        if(m_pAlphaDelimCB->IsChecked())
            nIdxOptions |= nsSwTOIOptions::TOI_ALPHA_DELIMITTER;
        rDesc.SetIndexOptions(nIdxOptions);
    }
    else if(TOX_AUTHORITIES == aLastTOXType.eType)
    {
        rDesc.SetSortByDocument(m_pSortDocPosRB->IsChecked());
        SwTOXSortKey aKey1, aKey2, aKey3;
        aKey1.eField = (ToxAuthorityField)reinterpret_cast<sal_uIntPtr>(m_pFirstKeyLB->GetSelectEntryData());
        aKey1.bSortAscending = m_pFirstSortUpRB->IsChecked();
        aKey2.eField = (ToxAuthorityField)reinterpret_cast<sal_uIntPtr>(m_pSecondKeyLB->GetSelectEntryData());
        aKey2.bSortAscending = m_pSecondSortUpRB->IsChecked();
        aKey3.eField = (ToxAuthorityField)reinterpret_cast<sal_uIntPtr>(m_pThirdKeyLB->GetSelectEntryData());
        aKey3.bSortAscending = m_pThirdSortUpRB->IsChecked();

        rDesc.SetSortKeys(aKey1, aKey2, aKey3);
    }
    SwForm* pCurrentForm = pTOXDlg->GetForm(aLastTOXType);
    if(m_pRelToStyleCB->IsVisible())
    {
        pCurrentForm->SetRelTabPos(m_pRelToStyleCB->IsChecked());
    }
    if(m_pCommaSeparatedCB->IsVisible())
        pCurrentForm->SetCommaSeparated(m_pCommaSeparatedCB->IsChecked());
}

SfxTabPage::sfxpg SwTOXEntryTabPage::DeactivatePage( SfxItemSet* /*pSet*/)
{
    UpdateDescriptor();
    return LEAVE_PAGE;
}

VclPtr<SfxTabPage> SwTOXEntryTabPage::Create( vcl::Window* pParent,     const SfxItemSet* rAttrSet)
{
    return VclPtr<SwTOXEntryTabPage>::Create(pParent, *rAttrSet);
}

IMPL_LINK_TYPED(SwTOXEntryTabPage, EditStyleHdl, Button*, pBtn, void)
{
    if( LISTBOX_ENTRY_NOTFOUND != m_pCharStyleLB->GetSelectEntryPos())
    {
        SfxStringItem aStyle(SID_STYLE_EDIT, m_pCharStyleLB->GetSelectEntry());
        SfxUInt16Item aFamily(SID_STYLE_FAMILY, SFX_STYLE_FAMILY_CHAR);
        vcl::Window* pDefDlgParent = Application::GetDefDialogParent();
        Application::SetDefDialogParent( pBtn );
        static_cast<SwMultiTOXTabDialog*>(GetTabDialog())->GetWrtShell().
        GetView().GetViewFrame()->GetDispatcher()->Execute(
        SID_STYLE_EDIT, SfxCallMode::SYNCHRON|SfxCallMode::MODAL,
            &aStyle, &aFamily, 0L);
        Application::SetDefDialogParent( pDefDlgParent );
    }
}

IMPL_LINK_TYPED(SwTOXEntryTabPage, RemoveInsertAuthHdl, Button*, pButton, void)
{
    bool bInsert = pButton == m_pAuthInsertPB;
    if(bInsert)
    {
        sal_Int32 nSelPos = m_pAuthFieldsLB->GetSelectEntryPos();
        const OUString sToInsert(m_pAuthFieldsLB->GetSelectEntry());
        SwFormToken aInsert(TOKEN_AUTHORITY);
        aInsert.nAuthorityField = (sal_uInt16)reinterpret_cast<sal_uIntPtr>(m_pAuthFieldsLB->GetEntryData(nSelPos));
        m_pTokenWIN->InsertAtSelection(SwForm::GetFormAuth(), aInsert);
        m_pAuthFieldsLB->RemoveEntry(sToInsert);
        m_pAuthFieldsLB->SelectEntryPos( nSelPos ? nSelPos - 1 : 0);
    }
    else
    {
        Control* pCtrl = m_pTokenWIN->GetActiveControl();
        OSL_ENSURE(WINDOW_EDIT != pCtrl->GetType(), "Remove should be disabled");
        if( WINDOW_EDIT != pCtrl->GetType() )
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
    sal_Int32 nPos = m_pAuthFieldsLB->InsertEntry(SW_RESSTR(STR_AUTH_FIELD_START + nData));
    m_pAuthFieldsLB->SetEntryData(nPos, reinterpret_cast<void*>((sal_uIntPtr)nData));
}

void SwTOXEntryTabPage::SetFocus2theAllBtn()
{
    m_pAllLevelsPB->GrabFocus();
}

bool SwTOXEntryTabPage::Notify( NotifyEvent& rNEvt )
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

    return SfxTabPage::Notify( rNEvt );
}

// This function initializes the default value in the Token
// put here the UI dependent initializations
IMPL_LINK_TYPED(SwTOXEntryTabPage, InsertTokenHdl, Button*, pBtn, void)
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
        sCharStyle = SW_RES(STR_POOLCHR_TOXJUMP);
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

IMPL_LINK_NOARG_TYPED(SwTOXEntryTabPage, AllLevelsHdl, Button*, void)
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

IMPL_LINK_TYPED(SwTOXEntryTabPage, LevelHdl, SvTreeListBox*, pBox, void)
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
            sal_Int32 nPos = m_pAuthFieldsLB->InsertEntry(SW_RESSTR(STR_AUTH_FIELD_START + i));
            m_pAuthFieldsLB->SetEntryData(nPos, reinterpret_cast<void*>((sal_uIntPtr)i));
        }

        // #i21237#
        SwFormTokens aPattern = m_pCurrentForm->GetPattern(nLevel + 1);
        SwFormTokens::iterator aIt = aPattern.begin();

        while(aIt != aPattern.end())
        {
            SwFormToken aToken = *aIt; // #i21237#
            if(TOKEN_AUTHORITY == aToken.eTokenType)
            {
                sal_uInt32 nSearch = aToken.nAuthorityField;
                sal_Int32  nLstBoxPos = m_pAuthFieldsLB->GetEntryPos( reinterpret_cast<void*>((sal_uIntPtr)nSearch) );
                OSL_ENSURE(LISTBOX_ENTRY_NOTFOUND != nLstBoxPos, "Entry not found?");
                m_pAuthFieldsLB->RemoveEntry(nLstBoxPos);
            }

            ++aIt; // #i21237#
        }
        m_pAuthFieldsLB->SelectEntryPos(0);
    }
    bInLevelHdl = false;
    pBox->GrabFocus();
}

IMPL_LINK_TYPED(SwTOXEntryTabPage, SortKeyHdl, Button*, pButton, void)
{
    bool bEnable = m_pSortContentRB == pButton;
    m_pSortKeyFrame->Enable(bEnable);
}

IMPL_LINK_TYPED(SwTOXEntryTabPage, TokenSelectedHdl, SwFormToken&, rToken, void)
{
    if (!rToken.sCharStyleName.isEmpty())
        m_pCharStyleLB->SelectEntry(rToken.sCharStyleName);
    else
        m_pCharStyleLB->SelectEntry(sNoCharStyle);

    const OUString sEntry = m_pCharStyleLB->GetSelectEntry();
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
        m_pTabPosMF->SetValue(m_pTabPosMF->Normalize(rToken.nTabStopPosition), FUNIT_TWIP);
        m_pAutoRightCB->Check(SVX_TAB_ADJUST_END == rToken.eTabAlign);
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
        m_pAuthInsertPB->Enable(bText && !m_pAuthFieldsLB->GetSelectEntry().isEmpty());
        m_pAuthRemovePB->Enable(!bText);
    }
}

IMPL_LINK_TYPED(SwTOXEntryTabPage, StyleSelectHdl, ListBox&, rBox, void)
{
    OUString sEntry = rBox.GetSelectEntry();
    const sal_uInt16 nId = (sal_uInt16)reinterpret_cast<sal_IntPtr>(rBox.GetSelectEntryData());
    const bool bEqualsNoCharStyle = sEntry == sNoCharStyle;
    m_pEditStylePB->Enable(!bEqualsNoCharStyle);
    if (bEqualsNoCharStyle)
        sEntry.clear();
    Control* pCtrl = m_pTokenWIN->GetActiveControl();
    OSL_ENSURE(pCtrl, "no active control?");
    if(pCtrl)
    {
        if(WINDOW_EDIT == pCtrl->GetType())
            static_cast<SwTOXEdit*>(pCtrl)->SetCharStyleName(sEntry, nId);
        else
            static_cast<SwTOXButton*>(pCtrl)->SetCharStyleName(sEntry, nId);

    }
    ModifyHdl(nullptr);
}

IMPL_LINK_TYPED(SwTOXEntryTabPage, ChapterInfoHdl, ListBox&, rBox, void)
{
    sal_Int32 nPos = rBox.GetSelectEntryPos();
    if(LISTBOX_ENTRY_NOTFOUND != nPos)
    {
        Control* pCtrl = m_pTokenWIN->GetActiveControl();
        OSL_ENSURE(pCtrl, "no active control?");
        if(pCtrl && WINDOW_EDIT != pCtrl->GetType())
            static_cast<SwTOXButton*>(pCtrl)->SetChapterInfo(nPos);

        ModifyHdl(nullptr);
    }
}

IMPL_LINK_TYPED(SwTOXEntryTabPage, ChapterInfoOutlineHdl, Edit&, rEdit, void)
{
    const sal_uInt16 nLevel = static_cast<sal_uInt8>(static_cast<NumericField&>(rEdit).GetValue());

    Control* pCtrl = m_pTokenWIN->GetActiveControl();
    OSL_ENSURE(pCtrl, "no active control?");
    if(pCtrl && WINDOW_EDIT != pCtrl->GetType())
        static_cast<SwTOXButton*>(pCtrl)->SetOutlineLevel(nLevel);

    ModifyHdl(nullptr);
}

IMPL_LINK_TYPED(SwTOXEntryTabPage, NumberFormatHdl, ListBox&, rBox, void)
{
    const sal_Int32 nPos = rBox.GetSelectEntryPos();

    if(LISTBOX_ENTRY_NOTFOUND != nPos)
    {
        Control* pCtrl = m_pTokenWIN->GetActiveControl();
        OSL_ENSURE(pCtrl, "no active control?");
        if(pCtrl && WINDOW_EDIT != pCtrl->GetType())
        {
           static_cast<SwTOXButton*>(pCtrl)->SetEntryNumberFormat(nPos);//i89791
        }
        ModifyHdl(nullptr);
    }
}

IMPL_LINK_TYPED(SwTOXEntryTabPage, TabPosHdl, Edit&, rEdit, void)
{
    MetricField* pField = static_cast<MetricField*>(&rEdit);
    Control* pCtrl = m_pTokenWIN->GetActiveControl();
    OSL_ENSURE(pCtrl && WINDOW_EDIT != pCtrl->GetType() &&
        TOKEN_TAB_STOP == static_cast<SwTOXButton*>(pCtrl)->GetFormToken().eTokenType,
                "no active style::TabStop control?");
    if( pCtrl && WINDOW_EDIT != pCtrl->GetType() )
    {
        static_cast<SwTOXButton*>(pCtrl)->SetTabPosition( static_cast< SwTwips >(
                pField->Denormalize( pField->GetValue( FUNIT_TWIP ))));
    }
    ModifyHdl(nullptr);
}

IMPL_LINK_TYPED(SwTOXEntryTabPage, FillCharHdl, Edit&, rBox, void)
{
    Control* pCtrl = m_pTokenWIN->GetActiveControl();
    OSL_ENSURE(pCtrl && WINDOW_EDIT != pCtrl->GetType() &&
        TOKEN_TAB_STOP == static_cast<SwTOXButton*>(pCtrl)->GetFormToken().eTokenType,
                "no active style::TabStop control?");
    if(pCtrl && WINDOW_EDIT != pCtrl->GetType())
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

IMPL_LINK_TYPED(SwTOXEntryTabPage, AutoRightHdl, Button*, pBox, void)
{
    //the most right style::TabStop is usually right aligned
    Control* pCurCtrl = m_pTokenWIN->GetActiveControl();
    OSL_ENSURE(WINDOW_EDIT != pCurCtrl->GetType() &&
            static_cast<SwTOXButton*>(pCurCtrl)->GetFormToken().eTokenType == TOKEN_TAB_STOP,
            "no style::TabStop selected!");

    const SwFormToken& rToken = static_cast<SwTOXButton*>(pCurCtrl)->GetFormToken();
    bool bChecked = static_cast<CheckBox*>(pBox)->IsChecked();
    if(rToken.eTokenType == TOKEN_TAB_STOP)
        static_cast<SwTOXButton*>(pCurCtrl)->SetTabAlign(
            bChecked ? SVX_TAB_ADJUST_END : SVX_TAB_ADJUST_LEFT);
    m_pTabPosFT->Enable(!bChecked);
    m_pTabPosMF->Enable(!bChecked);
    ModifyHdl(nullptr);
}

void SwTOXEntryTabPage::SetWrtShell(SwWrtShell& rSh)
{
    SwDocShell* pDocSh = rSh.GetView().GetDocShell();
    ::FillCharStyleListBox(*m_pCharStyleLB, pDocSh, true, true);
    const OUString sDefault(SW_RES(STR_POOLCOLL_STANDARD));
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

SwTokenWindow::SwTokenWindow(vcl::Window* pParent)
    : VclHBox(pParent)
    , pForm(nullptr)
    , nLevel(0)
    , bValid(false)
    , sCharStyle(SW_RESSTR(STR_CHARSTYLE))
    , pActiveCtrl(nullptr)
    , m_pParent(nullptr)
{
    m_pUIBuilder = new VclBuilder(this, getUIRootDir(),
        "modules/swriter/ui/tokenwidget.ui", "TokenWidget");
    get(m_pLeftScrollWin, "left");
    get(m_pCtrlParentWin, "ctrl");
    m_pCtrlParentWin->set_height_request(Edit::GetMinimumEditSize().Height());
    get(m_pRightScrollWin, "right");

    for (sal_uInt32 i = 0; i < TOKEN_END; ++i)
    {
        sal_uInt32 nTextId = STR_BUTTON_TEXT_START + i;
        if( STR_TOKEN_ENTRY_TEXT == nTextId )
            nTextId = STR_TOKEN_ENTRY;
        aButtonTexts[i] = SW_RESSTR(nTextId);

        sal_uInt32 nHelpId = STR_BUTTON_HELP_TEXT_START + i;
        if(STR_TOKEN_HELP_ENTRY_TEXT == nHelpId)
            nHelpId = STR_TOKEN_HELP_ENTRY;
        aButtonHelpTexts[i] = SW_RESSTR(nHelpId);
    }

    accessibleName = SW_RESSTR(STR_STRUCTURE);
    sAdditionalAccnameString1 = SW_RESSTR(STR_ADDITIONAL_ACCNAME_STRING1);
    sAdditionalAccnameString2 = SW_RESSTR(STR_ADDITIONAL_ACCNAME_STRING2);
    sAdditionalAccnameString3 = SW_RESSTR(STR_ADDITIONAL_ACCNAME_STRING3);

    Link<Button*,void> aLink(LINK(this, SwTokenWindow, ScrollHdl));
    m_pLeftScrollWin->SetClickHdl(aLink);
    m_pRightScrollWin->SetClickHdl(aLink);
}

VCL_BUILDER_FACTORY(SwTokenWindow)

void SwTokenWindow::setAllocation(const Size &rAllocation)
{
    VclHBox::setAllocation(rAllocation);

    if (aControlList.empty())
        return;

    Size aControlSize(m_pCtrlParentWin->GetSizePixel());
    for (ctrl_iterator it = aControlList.begin(); it != aControlList.end(); ++it)
    {
        Control* pControl = (*it);
        Size aSize(pControl->GetSizePixel());
        aSize.Height() = aControlSize.Height();
        pControl->SetSizePixel(aSize);
    }
}

SwTokenWindow::~SwTokenWindow()
{
    disposeOnce();
}

void SwTokenWindow::dispose()
{
    for (ctrl_iterator it = aControlList.begin(); it != aControlList.end(); ++it)
    {
        VclPtr<Control> pControl = (*it);
        pControl->SetGetFocusHdl( Link<Control&,void>() );
        pControl->SetLoseFocusHdl( Link<Control&,void>() );
        pControl.disposeAndClear();
    }
    aControlList.clear();
    disposeBuilder();
    m_pLeftScrollWin.clear();
    m_pCtrlParentWin.clear();
    m_pRightScrollWin.clear();
    pActiveCtrl.clear();
    m_pParent.clear();
    VclHBox::dispose();
}

void SwTokenWindow::SetForm(SwForm& rForm, sal_uInt16 nL)
{
    SetActiveControl(nullptr);
    bValid = true;

    if(pForm)
    {
        for (ctrl_iterator iter = aControlList.begin(); iter != aControlList.end(); ++iter)
            iter->disposeAndClear();
        //apply current level settings to the form
        for (auto it = aControlList.begin(); it != aControlList.end(); ++it)
             it->disposeAndClear();
        aControlList.clear();
    }

    nLevel = nL;
    pForm = &rForm;
    //now the display
    if(nLevel < MAXLEVEL || rForm.GetTOXType() == TOX_AUTHORITIES)
    {
        // #i21237#
        SwFormTokens aPattern = pForm->GetPattern(nLevel + 1);
        SwFormTokens::iterator aIt = aPattern.begin();
        bool bLastWasText = false; //assure alternating text - code - text

        Control* pSetActiveControl = nullptr;
        while(aIt != aPattern.end()) // #i21237#
        {
            SwFormToken aToken(*aIt); // #i21237#

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

            ++aIt; // #i21237#
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
    if( pSet != pActiveCtrl )
    {
        pActiveCtrl = pSet;
        if( pActiveCtrl )
        {
            pActiveCtrl->GrabFocus();
            //it must be a SwTOXEdit
            const SwFormToken* pFToken;
            if( WINDOW_EDIT == pActiveCtrl->GetType() )
                pFToken = &static_cast<SwTOXEdit*>(pActiveCtrl.get())->GetFormToken();
            else
                pFToken = &static_cast<SwTOXButton*>(pActiveCtrl.get())->GetFormToken();

            SwFormToken aTemp( *pFToken );
            aButtonSelectedHdl.Call( aTemp );
        }
    }
}

Control*    SwTokenWindow::InsertItem(const OUString& rText, const SwFormToken& rToken)
{
    Control* pRet = nullptr;
    Size aControlSize(m_pCtrlParentWin->GetSizePixel());
    Point aControlPos;

    if(!aControlList.empty())
    {
        Control* pLast = *(aControlList.rbegin());

        aControlSize = pLast->GetSizePixel();
        aControlPos = pLast->GetPosPixel();
        aControlPos.X() += aControlSize.Width();
    }

    if(TOKEN_TEXT == rToken.eTokenType)
    {
        VclPtr<SwTOXEdit> pEdit = VclPtr<SwTOXEdit>::Create(m_pCtrlParentWin, this, rToken);
        pEdit->SetPosPixel(aControlPos);

        aControlList.push_back(pEdit);

        pEdit->SetText(rText);
        sal_uInt32 nIndex = GetControlIndex( TOKEN_TEXT );
        OUString strName(accessibleName + OUString::number(nIndex));
        if ( nIndex == 1 )
        {
            /*Press left or right arrow to choose the structure controls*/
            strName += " (" + sAdditionalAccnameString2 + ", "
            /*Press Ctrl+Alt+A to move focus for more operations*/
                     + sAdditionalAccnameString1 + ", "
            /*Press Ctrl+Alt+B to move focus back to the current structure control*/
                     + sAdditionalAccnameString3 + ")";
        }
        pEdit->SetAccessibleName(strName);
        Size aEditSize(aControlSize);
        aEditSize.Width() = pEdit->GetTextWidth(rText) + EDIT_MINWIDTH;
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

        aControlList.push_back(pButton);

        Size aEditSize(aControlSize);
        aEditSize.Width() = pButton->GetTextWidth(rText) + 5;
        pButton->SetSizePixel(aEditSize);
        pButton->SetPrevNextLink(LINK(this, SwTokenWindow, NextItemBtnHdl));
        pButton->SetGetFocusHdl(LINK(this, SwTokenWindow, TbxFocusBtnHdl));

        if(TOKEN_AUTHORITY != rToken.eTokenType)
            pButton->SetText(aButtonTexts[rToken.eTokenType]);
        else
        {
            //use the first two chars as symbol
            OUString sTmp(SwAuthorityFieldType::GetAuthFieldName(
                        (ToxAuthorityField)rToken.nAuthorityField));
            pButton->SetText(sTmp.copy(0, 2));
        }

        sal_uInt32 nIndex = GetControlIndex( rToken.eTokenType );
        OUString sAccName = aButtonHelpTexts[rToken.eTokenType];
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
    OSL_ENSURE(pActiveCtrl, "no active control!");

    if(!pActiveCtrl)
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

        ctrl_const_iterator it = aControlList.begin();
        for( ; it != aControlList.end() && pActiveCtrl != (*it); ++it )
        {
            pControl = *it;

            if( WINDOW_EDIT != pControl->GetType())
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
            for( ; it != aControlList.end(); ++it )
            {
                pControl = *it;

                if( pControl != pActiveCtrl &&
                    WINDOW_EDIT != pControl->GetType())
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
            aToInsertToken.sText =  aButtonTexts[TOKEN_LINK_END];
        }

        if(bPostLinkStartFound)
        {
            OSL_ENSURE(pExchange, "no control to exchange?");
            if(pExchange)
            {
                const_cast<SwTOXButton*>(static_cast<const SwTOXButton*>(pExchange))->SetLinkEnd();
                const_cast<SwTOXButton*>(static_cast<const SwTOXButton*>(pExchange))->SetText(aButtonTexts[TOKEN_LINK_END]);
            }
        }

        if(bPreEndLinkFound)
        {
            OSL_ENSURE(pExchange, "no control to exchange?");

            if(pExchange)
            {
                const_cast<SwTOXButton*>(static_cast<const SwTOXButton*>(pExchange))->SetLinkStart();
                const_cast<SwTOXButton*>(static_cast<const SwTOXButton*>(pExchange))->SetText(aButtonTexts[TOKEN_LINK_START]);
            }
        }
    }

    //if the active control is text then insert a new button at the selection
    //else replace the button
    ctrl_iterator iterActive = std::find(aControlList.begin(),
                                         aControlList.end(), pActiveCtrl);

    assert(iterActive != aControlList.end());
    if (iterActive == aControlList.end())
        return;

    Size aControlSize(GetOutputSizePixel());

    if( WINDOW_EDIT == pActiveCtrl->GetType())
    {
        ++iterActive;

        Selection aSel = static_cast<SwTOXEdit*>(pActiveCtrl.get())->GetSelection();
        aSel.Justify();

        const OUString sEditText = static_cast<SwTOXEdit*>(pActiveCtrl.get())->GetText();
        const OUString sLeft = sEditText.copy( 0, aSel.A() );
        const OUString sRight = sEditText.copy( aSel.B() );

        static_cast<SwTOXEdit*>(pActiveCtrl.get())->SetText(sLeft);
        static_cast<SwTOXEdit*>(pActiveCtrl.get())->AdjustSize();

        SwFormToken aTmpToken(TOKEN_TEXT);
        VclPtr<SwTOXEdit> pEdit = VclPtr<SwTOXEdit>::Create(m_pCtrlParentWin, this, aTmpToken);

        iterActive = aControlList.insert(iterActive, pEdit);

        pEdit->SetText(sRight);
        sal_uInt32 nIndex = GetControlIndex( TOKEN_TEXT );
        OUString strName(accessibleName + OUString::number(nIndex));
        if ( nIndex == 1)
        {
            /*Press left or right arrow to choose the structure controls*/
            strName += " (" + sAdditionalAccnameString2 + ", "
            /*Press Ctrl+Alt+A to move focus for more operations*/
                     + sAdditionalAccnameString1 + ", "
            /*Press Ctrl+Alt+B to move focus back to the current structure control*/
                     + sAdditionalAccnameString3 + ")";
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
        iterActive = aControlList.erase(iterActive);
        pActiveCtrl->Hide();
        pActiveCtrl.disposeAndClear();
    }

    //now the new button
    VclPtr<SwTOXButton> pButton = VclPtr<SwTOXButton>::Create(m_pCtrlParentWin, this, aToInsertToken);

    aControlList.insert(iterActive, pButton);

    pButton->SetPrevNextLink(LINK(this, SwTokenWindow, NextItemBtnHdl));
    pButton->SetGetFocusHdl(LINK(this, SwTokenWindow, TbxFocusBtnHdl));

    if(TOKEN_AUTHORITY != aToInsertToken.eTokenType)
    {
        pButton->SetText(aButtonTexts[aToInsertToken.eTokenType]);
    }
    else
    {
        //use the first two chars as symbol
        OUString sTmp(SwAuthorityFieldType::GetAuthFieldName(
                    (ToxAuthorityField)aToInsertToken.nAuthorityField));
        pButton->SetText(sTmp.copy(0, 2));
    }

    Size aEditSize(GetOutputSizePixel());
    aEditSize.Width() = pButton->GetTextWidth(rText) + 5;
    pButton->SetSizePixel(aEditSize);
    pButton->Check();
    pButton->Show();
    SetActiveControl(pButton);

    AdjustPositions();
}

void SwTokenWindow::RemoveControl(SwTOXButton* pDel, bool bInternalCall )
{
    if(bInternalCall && TOX_AUTHORITIES == pForm->GetTOXType())
        m_pParent->PreTokenButtonRemoved(pDel->GetFormToken());

    ctrl_iterator it = std::find(aControlList.begin(), aControlList.end(), pDel);

    assert(it != aControlList.end()); //Control does not exist!
    if (it == aControlList.end())
        return;

    // the two neighbours of the box must be merged
    // the properties of the right one will be lost
    assert(it != aControlList.begin() && it != aControlList.end() - 1); //Button at first or last position?
    if (it == aControlList.begin() || it == aControlList.end() - 1)
        return;

    ctrl_iterator itLeft = it, itRight = it;
    --itLeft;
    ++itRight;
    VclPtr<Control> pLeftEdit = *itLeft;
    VclPtr<Control> pRightEdit = *itRight;

    static_cast<SwTOXEdit*>(pLeftEdit.get())->SetText(static_cast<SwTOXEdit*>(pLeftEdit.get())->GetText() +
                                     static_cast<SwTOXEdit*>(pRightEdit.get())->GetText());
    static_cast<SwTOXEdit*>(pLeftEdit.get())->AdjustSize();

    aControlList.erase(itRight);
    pRightEdit.disposeAndClear();

    aControlList.erase(it);
    pActiveCtrl->Hide();
    pActiveCtrl.disposeAndClear();

    SetActiveControl(pLeftEdit);
    AdjustPositions();
    aModifyHdl.Call(nullptr);
}

void SwTokenWindow::AdjustPositions()
{
    if(aControlList.size() > 1)
    {
        ctrl_iterator it = aControlList.begin();
        Control* pCtrl = *it;
        ++it;

        Point aNextPos = pCtrl->GetPosPixel();
        aNextPos.X() += pCtrl->GetSizePixel().Width();

        for(; it != aControlList.end(); ++it)
        {
            pCtrl = *it;
            pCtrl->SetPosPixel(aNextPos);
            aNextPos.X() += pCtrl->GetSizePixel().Width();
        }

        AdjustScrolling();
    }
};

void SwTokenWindow::MoveControls(long nOffset)
{
    // move the complete list
    for (ctrl_iterator it = aControlList.begin(); it != aControlList.end(); ++it)
    {
        Control *pCtrl = *it;

        Point aPos = pCtrl->GetPosPixel();
        aPos.X() += nOffset;

        pCtrl->SetPosPixel(aPos);
    }
}

void SwTokenWindow::AdjustScrolling()
{
    if(aControlList.size() > 1)
    {
        //validate scroll buttons
        Control* pFirstCtrl = *(aControlList.begin());
        Control* pLastCtrl = *(aControlList.rbegin());

        long nSpace = m_pCtrlParentWin->GetSizePixel().Width();
        long nWidth = pLastCtrl->GetPosPixel().X() - pFirstCtrl->GetPosPixel().X()
                                                    + pLastCtrl->GetSizePixel().Width();
        bool bEnable = nWidth > nSpace;

        //the active control must be visible
        if(bEnable && pActiveCtrl)
        {
            Point aActivePos(pActiveCtrl->GetPosPixel());

            long nMove = 0;

            if(aActivePos.X() < 0)
                nMove = -aActivePos.X();
            else if((aActivePos.X() + pActiveCtrl->GetSizePixel().Width())  > nSpace)
                nMove = -(aActivePos.X() + pActiveCtrl->GetSizePixel().Width() - nSpace);

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

IMPL_LINK_TYPED(SwTokenWindow, ScrollHdl, Button*, pBtn, void )
{
    if(aControlList.empty())
        return;

    const long nSpace = m_pCtrlParentWin->GetSizePixel().Width();
#if OSL_DEBUG_LEVEL > 1
    //find all start/end positions and print it
    OUString sMessage("Space: " + OUString::number(nSpace) + " | ");

    for (ctrl_const_iterator it = aControlList.begin(); it != aControlList.end(); ++it)
    {
        Control *pDebugCtrl = *it;

        long nDebugXPos = pDebugCtrl->GetPosPixel().X();
        long nDebugWidth = pDebugCtrl->GetSizePixel().Width();

        sMessage += OUString::number(nDebugXPos) + " "
                  + OUString::number(nDebugXPos + nDebugWidth) + " | ";
    }

#endif

    long nMove = 0;
    if(pBtn == m_pLeftScrollWin)
    {
        //find the first completely visible control (left edge visible)
        for (ctrl_iterator it = aControlList.begin(); it != aControlList.end(); ++it)
        {
            Control *pCtrl = *it;

            long nXPos = pCtrl->GetPosPixel().X();

            if (nXPos >= 0)
            {
                if (it == aControlList.begin())
                {
                    //move the current control to the left edge
                    nMove = -nXPos;
                }
                else
                {
                    //move the left neighbor to the start position
                    ctrl_iterator itLeft = it;
                    --itLeft;
                    Control *pLeft = *itLeft;

                    nMove = -pLeft->GetPosPixel().X();
                }

                break;
            }
        }
    }
    else
    {
        //find the first completely visible control (right edge visible)
        for (ctrl_reverse_iterator it = aControlList.rbegin(); it != aControlList.rend(); ++it)
        {
            Control *pCtrl = *it;

            long nCtrlWidth = pCtrl->GetSizePixel().Width();
            long nXPos = pCtrl->GetPosPixel().X() + nCtrlWidth;

            if (nXPos <= nSpace)
            {
                if (it != aControlList.rbegin())
                {
                    //move the right neighbor  to the right edge right aligned
                    ctrl_reverse_iterator itRight = it;
                    --itRight;
                    Control *pRight = *itRight;
                    nMove = nSpace - pRight->GetPosPixel().X() - pRight->GetSizePixel().Width();
                }

                break;
            }
        }

        //move it left until it's completely visible
    }

    if(nMove)
    {
        // move the complete list
        MoveControls(nMove);

        Control *pCtrl = nullptr;

        pCtrl = *(aControlList.begin());
        m_pLeftScrollWin->Enable(pCtrl->GetPosPixel().X() < 0);

        pCtrl = *(aControlList.rbegin());
        m_pRightScrollWin->Enable((pCtrl->GetPosPixel().X() + pCtrl->GetSizePixel().Width()) > nSpace);
    }
}

OUString SwTokenWindow::GetPattern() const
{
    OUString sRet;

    for (ctrl_const_iterator it = aControlList.begin(); it != aControlList.end(); ++it)
    {
        const Control *pCtrl = *it;

        const SwFormToken &rNewToken = pCtrl->GetType() == WINDOW_EDIT
                ? const_cast<SwTOXEdit*>(static_cast<const SwTOXEdit*>(pCtrl))->GetFormToken()
                : static_cast<const SwTOXButton*>(pCtrl)->GetFormToken();

        //TODO: prevent input of TOX_STYLE_DELIMITER in KeyInput
        sRet += rNewToken.GetString();
    }

    return sRet;
}

// Check if a control of the specified TokenType is already contained in the list
bool SwTokenWindow::Contains(FormTokenType eSearchFor) const
{
    bool bRet = false;

    for (ctrl_const_iterator it = aControlList.begin(); it != aControlList.end(); ++it)
    {
        const Control *pCtrl = *it;

        const SwFormToken &rNewToken = pCtrl->GetType() == WINDOW_EDIT
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

bool SwTokenWindow::CreateQuickHelp(Control* pCtrl,
            const SwFormToken& rToken,
            const HelpEvent& rHEvt)
{
    bool bRet = false;
    if( rHEvt.GetMode() & HelpEventMode::QUICK )
    {
        bool bBalloon = Help::IsBalloonHelpEnabled();
        OUString sEntry;
        if(bBalloon || rToken.eTokenType != TOKEN_AUTHORITY)
            sEntry = (aButtonHelpTexts[rToken.eTokenType]);
        if(rToken.eTokenType == TOKEN_AUTHORITY )
        {
             sEntry += SwAuthorityFieldType::GetAuthFieldName(
                                (ToxAuthorityField) rToken.nAuthorityField);
        }

        Point aPos = OutputToScreenPixel(pCtrl->GetPosPixel());
        Rectangle aItemRect( aPos, pCtrl->GetSizePixel() );
        if ( rToken.eTokenType != TOKEN_TAB_STOP )
        {
            if (!rToken.sCharStyleName.isEmpty())
            {
                sEntry += OUString(bBalloon ? '\n' : ' ')
                        + sCharStyle + rToken.sCharStyleName;
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

IMPL_LINK_TYPED(SwTokenWindow, EditResize, Edit&, rEdit, void)
{
    static_cast<SwTOXEdit*>(&rEdit)->AdjustSize();
    AdjustPositions();
    aModifyHdl.Call(nullptr);
}

IMPL_LINK_TYPED(SwTokenWindow, NextItemHdl, SwTOXEdit&, rEdit, void)
{
    ctrl_iterator it = std::find(aControlList.begin(),aControlList.end(),&rEdit);

    if (it == aControlList.end())
        return;

    ctrl_iterator itTest = it;
    ++itTest;

    if ((it != aControlList.begin() && !rEdit.IsNextControl()) ||
        (itTest != aControlList.end() && rEdit.IsNextControl()))
    {
        ctrl_iterator iterFocus = it;
        rEdit.IsNextControl() ? ++iterFocus : --iterFocus;

        Control *pCtrlFocus = *iterFocus;
        pCtrlFocus->GrabFocus();
        static_cast<SwTOXButton*>(pCtrlFocus)->Check();

        AdjustScrolling();
    }
}

IMPL_LINK_TYPED(SwTokenWindow, TbxFocusHdl, Control&, rControl, void)
{
    SwTOXEdit* pEdit = static_cast<SwTOXEdit*>(&rControl);
    for (ctrl_iterator it = aControlList.begin(); it != aControlList.end(); ++it)
    {
        Control *pCtrl = *it;

        if (pCtrl && pCtrl->GetType() != WINDOW_EDIT)
            static_cast<SwTOXButton*>(pCtrl)->Check(false);
    }

    SetActiveControl(pEdit);
}

IMPL_LINK_TYPED(SwTokenWindow, NextItemBtnHdl, SwTOXButton&, rBtn, void )
{
    ctrl_iterator it = std::find(aControlList.begin(),aControlList.end(),&rBtn);

    if (it == aControlList.end())
        return;

    ctrl_iterator itTest = it;
    ++itTest;

    if (!rBtn.IsNextControl() || (itTest != aControlList.end() && rBtn.IsNextControl()))
    {
        bool isNext = rBtn.IsNextControl();

        ctrl_iterator iterFocus = it;
        isNext ? ++iterFocus : --iterFocus;

        Control *pCtrlFocus = *iterFocus;
        pCtrlFocus->GrabFocus();
        Selection aSel(0,0);

        if (!isNext)
        {
            const sal_Int32 nLen = static_cast<SwTOXEdit*>(pCtrlFocus)->GetText().getLength();

            aSel.A() = nLen;
            aSel.B() = nLen;
        }

        static_cast<SwTOXEdit*>(pCtrlFocus)->SetSelection(aSel);

        rBtn.Check(false);

        AdjustScrolling();
    }
}

IMPL_LINK_TYPED(SwTokenWindow, TbxFocusBtnHdl, Control&, rControl, void )
{
    SwTOXButton* pBtn = static_cast<SwTOXButton*>(&rControl);
    for (ctrl_iterator it = aControlList.begin(); it != aControlList.end(); ++it)
    {
        Control *pControl = *it;

        if (pControl && WINDOW_EDIT != pControl->GetType())
            static_cast<SwTOXButton*>(pControl)->Check(pBtn == pControl);
    }

    SetActiveControl(pBtn);
}

void SwTokenWindow::GetFocus()
{
    if(GetFocusFlags::Tab & GetGetFocusFlags())
    {
        if (!aControlList.empty())
        {
            Control *pFirst = *aControlList.begin();

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
    for (ctrl_const_iterator it = aControlList.begin(); it != aControlList.end(); ++it)
    {
        const Control* pControl = *it;
        const SwFormToken& rNewToken = WINDOW_EDIT == pControl->GetType()
            ? const_cast<SwTOXEdit*>(static_cast<const SwTOXEdit*>(pControl))->GetFormToken()
            : static_cast<const SwTOXButton*>(pControl)->GetFormToken();

        if(eType == rNewToken.eTokenType)
        {
            ++nIndex;
        }
    }

    return nIndex;
}

SwTOXStylesTabPage::SwTOXStylesTabPage(vcl::Window* pParent, const SfxItemSet& rAttrSet )
    : SfxTabPage(pParent, "TocStylesPage",
        "modules/swriter/ui/tocstylespage.ui", &rAttrSet)
    , m_pCurrentForm(nullptr)
{
    get(m_pLevelLB, "levels");
    get(m_pAssignBT, "assign");
    get(m_pParaLayLB, "styles");
    m_pParaLayLB->SetStyle(m_pParaLayLB->GetStyle() | WB_SORT);
    get(m_pStdBT, "default");
    get(m_pEditStyleBT, "edit");
    long nHeight = m_pLevelLB->GetTextHeight() * 16;
    m_pLevelLB->set_height_request(nHeight);
    m_pParaLayLB->set_height_request(nHeight);

    SetExchangeSupport();

    m_pEditStyleBT->SetClickHdl   (LINK(   this, SwTOXStylesTabPage, EditStyleHdl));
    m_pAssignBT->SetClickHdl      (LINK(   this, SwTOXStylesTabPage, AssignHdl));
    m_pStdBT->SetClickHdl         (LINK(   this, SwTOXStylesTabPage, StdHdl));
    m_pParaLayLB->SetSelectHdl    (LINK(   this, SwTOXStylesTabPage, EnableSelectHdl));
    m_pLevelLB->SetSelectHdl(LINK(this, SwTOXStylesTabPage, EnableSelectHdl));
    m_pParaLayLB->SetDoubleClickHdl(LINK(  this, SwTOXStylesTabPage, DoubleClickHdl));
}

SwTOXStylesTabPage::~SwTOXStylesTabPage()
{
    disposeOnce();
}

void SwTOXStylesTabPage::dispose()
{
    delete m_pCurrentForm;
    m_pLevelLB.clear();
    m_pAssignBT.clear();
    m_pParaLayLB.clear();
    m_pStdBT.clear();
    m_pEditStyleBT.clear();
    SfxTabPage::dispose();
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
    m_pCurrentForm = new SwForm(GetForm());
    m_pParaLayLB->Clear();
    m_pLevelLB->Clear();

    // not hyperlink for user directories

    const sal_uInt16 nSize = m_pCurrentForm->GetFormMax();

    // display form pattern without title

    // display 1st TemplateEntry
    OUString aStr( SW_RES( STR_TITLE ));
    if( !m_pCurrentForm->GetTemplate( 0 ).isEmpty() )
    {
        aStr += " " + OUStringLiteral1<aDeliStart>()
              + m_pCurrentForm->GetTemplate( 0 )
              + OUStringLiteral1<aDeliEnd>();
    }
    m_pLevelLB->InsertEntry(aStr);

    for( sal_uInt16 i=1; i < nSize; ++i )
    {
        if( TOX_INDEX == m_pCurrentForm->GetTOXType() &&
            FORM_ALPHA_DELIMITTER == i )
        {
            aStr = SW_RESSTR(STR_ALPHA);
        }
        else
        {
            aStr = SW_RESSTR(STR_LEVEL) + OUString::number(
                    TOX_INDEX == m_pCurrentForm->GetTOXType() ? i - 1 : i );
        }
        if( !m_pCurrentForm->GetTemplate( i ).isEmpty() )
        {
            aStr += " " + OUStringLiteral1<aDeliStart>()
                  + m_pCurrentForm->GetTemplate( i )
                  + OUStringLiteral1<aDeliEnd>();
        }
        m_pLevelLB->InsertEntry( aStr );
    }

    // initialise templates
    SwWrtShell& rSh = static_cast<SwMultiTOXTabDialog*>(GetTabDialog())->GetWrtShell();
    const sal_uInt16 nSz = rSh.GetTextFormatCollCount();

    for( sal_uInt16 i = 0; i < nSz; ++i )
    {
        const SwTextFormatColl *pColl = &rSh.GetTextFormatColl( i );
        if( !pColl->IsDefault() )
            m_pParaLayLB->InsertEntry( pColl->GetName() );
    }

    // query pool collections and set them for the directory
    for( sal_uInt16 i = 0; i < m_pCurrentForm->GetFormMax(); ++i )
    {
        aStr = m_pCurrentForm->GetTemplate( i );
        if( !aStr.isEmpty() &&
            LISTBOX_ENTRY_NOTFOUND == m_pParaLayLB->GetEntryPos( aStr ))
            m_pParaLayLB->InsertEntry( aStr );
    }

    EnableSelectHdl(*m_pParaLayLB);
}

SfxTabPage::sfxpg SwTOXStylesTabPage::DeactivatePage( SfxItemSet* /*pSet*/  )
{
    GetForm() = *m_pCurrentForm;
    return LEAVE_PAGE;
}

VclPtr<SfxTabPage> SwTOXStylesTabPage::Create( vcl::Window* pParent,
                                               const SfxItemSet* rAttrSet)
{
    return VclPtr<SwTOXStylesTabPage>::Create(pParent, *rAttrSet);
}

IMPL_LINK_TYPED( SwTOXStylesTabPage, EditStyleHdl, Button *, pBtn, void )
{
    if( LISTBOX_ENTRY_NOTFOUND != m_pParaLayLB->GetSelectEntryPos())
    {
        SfxStringItem aStyle(SID_STYLE_EDIT, m_pParaLayLB->GetSelectEntry());
        SfxUInt16Item aFamily(SID_STYLE_FAMILY, SFX_STYLE_FAMILY_PARA);
        vcl::Window* pDefDlgParent = Application::GetDefDialogParent();
        Application::SetDefDialogParent( pBtn );
        SwWrtShell& rSh = static_cast<SwMultiTOXTabDialog*>(GetTabDialog())->GetWrtShell();
        rSh.GetView().GetViewFrame()->GetDispatcher()->Execute(
        SID_STYLE_EDIT, SfxCallMode::SYNCHRON|SfxCallMode::MODAL,
            &aStyle, &aFamily, 0L);
        Application::SetDefDialogParent( pDefDlgParent );
    }
}

// allocate templates
IMPL_LINK_NOARG_TYPED(SwTOXStylesTabPage, AssignHdl, Button*, void)
{
    sal_Int32 nLevPos   = m_pLevelLB->GetSelectEntryPos();
    sal_Int32 nTemplPos = m_pParaLayLB->GetSelectEntryPos();
    if(nLevPos   != LISTBOX_ENTRY_NOTFOUND &&
       nTemplPos != LISTBOX_ENTRY_NOTFOUND)
    {
        const OUString aStr(m_pLevelLB->GetEntry(nLevPos).getToken(0, aDeliStart)
            + OUStringLiteral1<aDeliStart>()
            + m_pParaLayLB->GetSelectEntry()
            + OUStringLiteral1<aDeliEnd>());

        m_pCurrentForm->SetTemplate(nLevPos, m_pParaLayLB->GetSelectEntry());

        m_pLevelLB->RemoveEntry(nLevPos);
        m_pLevelLB->InsertEntry(aStr, nLevPos);
        m_pLevelLB->SelectEntry(aStr);
        Modify();
    }
}

IMPL_LINK_NOARG_TYPED(SwTOXStylesTabPage, StdHdl, Button*, void)
{
    const sal_Int32 nPos = m_pLevelLB->GetSelectEntryPos();
    if(nPos != LISTBOX_ENTRY_NOTFOUND)
    {
        const OUString aStr(m_pLevelLB->GetEntry(nPos).getToken(0, aDeliStart));
        m_pLevelLB->RemoveEntry(nPos);
        m_pLevelLB->InsertEntry(aStr, nPos);
        m_pLevelLB->SelectEntry(aStr);
        m_pCurrentForm->SetTemplate(nPos, aEmptyOUStr);
        Modify();
    }
}

IMPL_LINK_NOARG_TYPED(SwTOXStylesTabPage, DoubleClickHdl, ListBox&, void)
{
    const OUString aTmpName( m_pParaLayLB->GetSelectEntry() );
    SwWrtShell& rSh = static_cast<SwMultiTOXTabDialog*>(GetTabDialog())->GetWrtShell();

    if(m_pParaLayLB->GetSelectEntryPos() != LISTBOX_ENTRY_NOTFOUND &&
       (m_pLevelLB->GetSelectEntryPos() == 0 || SwMultiTOXTabDialog::IsNoNum(rSh, aTmpName)))
        AssignHdl(m_pAssignBT);
}

// enable only when selected
IMPL_LINK_NOARG_TYPED(SwTOXStylesTabPage, EnableSelectHdl, ListBox&, void)
{
    m_pStdBT->Enable(m_pLevelLB->GetSelectEntryPos()  != LISTBOX_ENTRY_NOTFOUND);

    SwWrtShell& rSh = static_cast<SwMultiTOXTabDialog*>(GetTabDialog())->GetWrtShell();
    const OUString aTmpName(m_pParaLayLB->GetSelectEntry());
    m_pAssignBT->Enable(m_pParaLayLB->GetSelectEntryPos() != LISTBOX_ENTRY_NOTFOUND &&
                     LISTBOX_ENTRY_NOTFOUND != m_pLevelLB->GetSelectEntryPos() &&
       (m_pLevelLB->GetSelectEntryPos() == 0 || SwMultiTOXTabDialog::IsNoNum(rSh, aTmpName)));
    m_pEditStyleBT->Enable(m_pParaLayLB->GetSelectEntryPos() != LISTBOX_ENTRY_NOTFOUND );
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
    , aCellEdit(VclPtr<Edit>::Create(&GetDataWindow(), 0))
    , aCellCheckBox(VclPtr< ::svt::CheckBoxControl>::Create(&GetDataWindow()))
    , nCurrentRow(0)
    , bModified(false)
{
    sSearch = pBuilder->get<vcl::Window>("searchterm")->GetText();
    sAlternative = pBuilder->get<vcl::Window>("alternative")->GetText();
    sPrimKey = pBuilder->get<vcl::Window>("key1")->GetText();
    sSecKey = pBuilder->get<vcl::Window>("key2")->GetText();
    sComment = pBuilder->get<vcl::Window>("comment")->GetText();
    sCaseSensitive = pBuilder->get<vcl::Window>("casesensitive")->GetText();
    sWordOnly = pBuilder->get<vcl::Window>("wordonly")->GetText();
    sYes = pBuilder->get<vcl::Window>("yes")->GetText();
    sNo = pBuilder->get<vcl::Window>("no")->GetText();

    aCellCheckBox->GetBox().EnableTriState(false);
    xController = new ::svt::EditCellController(aCellEdit.get());
    xCheckController = new ::svt::CheckBoxCellController(aCellCheckBox.get());

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
    aCellEdit.disposeAndClear();
    aCellCheckBox.disposeAndClear();
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
        long nNaturalWidth(::std::accumulate(aWidths.begin(), aWidths.end(), 0));
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
    nYesNoWidth = std::max(nYesNoWidth, GetTextWidth(sYes));
    nYesNoWidth = std::max(nYesNoWidth, GetTextWidth(sNo));
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
    Size aSize = LogicToPixel(Size(276 , 175), MapMode(MAP_APPFONT));

    std::vector<long> aWidths = GetOptimalColWidths();

    long nWidth(::std::accumulate(aWidths.begin(), aWidths.end(), 0));

    aSize.Width() = std::max(aSize.Width(), nWidth);

    return aSize;
}

bool SwEntryBrowseBox::SeekRow( long nRow )
{
    nCurrentRow = nRow;
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
            case  ITEM_CASE         :pRet = pEntry->bCase ? &sYes : &sNo; break;
            case  ITEM_WORDONLY     :pRet = pEntry->bWord ? &sYes : &sNo; break;
        }
    }
    return *pRet;
}

void SwEntryBrowseBox::PaintCell(OutputDevice& rDev,
                                const Rectangle& rRect, sal_uInt16 nColumnId) const
{
    const DrawTextFlags nStyle = DrawTextFlags::Clip | DrawTextFlags::Center;
    rDev.DrawText( rRect, GetCellText( nCurrentRow, nColumnId ), nStyle );
}

::svt::CellController* SwEntryBrowseBox::GetController(long /*nRow*/, sal_uInt16 nCol)
{
    return nCol < ITEM_CASE ? xController : xCheckController;
}

bool SwEntryBrowseBox::SaveModified()
{
    SetModified();
    const size_t nRow = GetCurRow();
    const sal_uInt16 nCol = GetCurColumnId();

    OUString sNew;
    bool bVal = false;
    ::svt::CellController* pController = nullptr;
    if(nCol < ITEM_CASE)
    {
        pController = xController;
        sNew = static_cast< ::svt::EditCellController*>(pController)->GetEditImplementation()->GetText( LINEEND_LF );
    }
    else
    {
        pController = xCheckController;
        bVal = static_cast< ::svt::CheckBoxCellController*>(pController)->GetCheckBox().IsChecked();
    }
    AutoMarkEntry* pEntry = (nRow >= m_Entries.size()) ? new AutoMarkEntry
                                                       : m_Entries[nRow].get();
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
    if (nRow >= m_Entries.size())
    {
        m_Entries.push_back(std::unique_ptr<AutoMarkEntry>(pEntry));
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
        rController = xController;
        ::svt::CellController* pController = xController;
        static_cast< ::svt::EditCellController*>(pController)->GetEditImplementation()->SetText( rText );
    }
    else
    {
        rController = xCheckController;
        ::svt::CellController* pController = xCheckController;
        static_cast< ::svt::CheckBoxCellController*>(pController)->GetCheckBox().Check(
                                                            rText == sYes );
     }
}

void SwEntryBrowseBox::ReadEntries(SvStream& rInStr)
{
    AutoMarkEntry* pToInsert = nullptr;
    rtl_TextEncoding  eTEnc = osl_getThreadTextEncoding();
    while( !rInStr.GetError() && !rInStr.IsEof() )
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
        pController = xController;
    else
        pController = xCheckController;
    if(pController ->IsModified())
        GoToColumnId(nCol + (nCol < ITEM_CASE ? 1 : -1 ));

    rtl_TextEncoding  eTEnc = osl_getThreadTextEncoding();
    for(size_t i = 0; i < m_Entries.size(); i++)
    {
        AutoMarkEntry* pEntry = m_Entries[i].get();
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
    if(bModified)
        return true;

    //check if the current controller is modified
    const sal_uInt16 nCol = GetCurColumnId();
    ::svt::CellController* pController;
    if(nCol < ITEM_CASE)
        pController = xController;
    else
        pController = xCheckController;
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
        SfxMedium aMed( sAutoMarkURL, STREAM_STD_READ );
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

IMPL_LINK_NOARG_TYPED(SwAutoMarkDlg_Impl, OkHdl, Button*, void)
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
