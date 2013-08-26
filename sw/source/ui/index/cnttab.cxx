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
#include <com/sun/star/ui/dialogs/XFilePicker.hpp>
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
#include <cnttab.hrc>
#include <globals.hrc>
#include <SwStyleNameMapper.hxx>
#include <sfx2/filedlghelper.hxx>
#include <toxwrap.hxx>
#include <chpfld.hxx>

#include "utlui.hrc"

#include <sfx2/app.hxx>

#include <unomid.h>
using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace com::sun::star::ui::dialogs;
using namespace ::sfx2;
#include <svtools/editbrowsebox.hxx>

static const sal_Unicode aDeliStart = '['; // for the form
static const sal_Unicode aDeliEnd    = ']'; // for the form

#define IDX_FILE_EXTENSION OUString("*.sdi")

static String lcl_CreateAutoMarkFileDlg( const String& rURL,
                                const String& rFileString, bool bOpen )
{
    String sRet;

    FileDialogHelper aDlgHelper( bOpen ?
                TemplateDescription::FILEOPEN_SIMPLE : TemplateDescription::FILESAVE_AUTOEXTENSION, 0 );
    uno::Reference < XFilePicker > xFP = aDlgHelper.GetFilePicker();

    uno::Reference<XFilterManager> xFltMgr(xFP, UNO_QUERY);
    String sCurFltr( IDX_FILE_EXTENSION );
    xFltMgr->appendFilter( rFileString, sCurFltr );
    xFltMgr->setCurrentFilter( rFileString ) ;

    String& rLastSaveDir = (String&)SFX_APP()->GetLastSaveDirectory();
    String sSaveDir = rLastSaveDir;

    if( rURL.Len() )
        xFP->setDisplayDirectory( rURL );
    else
    {
        SvtPathOptions aPathOpt;
        xFP->setDisplayDirectory( aPathOpt.GetUserConfigPath() );
    }

    if( aDlgHelper.Execute() == ERRCODE_NONE )
    {
        sRet = xFP->getFiles().getConstArray()[0];
    }
    rLastSaveDir = sSaveDir;
    return sRet;
}

struct AutoMarkEntry
{
    String sSearch;
    String sAlternative;
    String sPrimKey;
    String sSecKey;
    String sComment;
    sal_Bool    bCase;
    sal_Bool    bWord;

    AutoMarkEntry() :
        bCase(sal_False),
        bWord(sal_False){}
};
typedef boost::ptr_vector<AutoMarkEntry> AutoMarkEntryArr;

typedef ::svt::EditBrowseBox SwEntryBrowseBox_Base;

class SwEntryBrowseBox : public SwEntryBrowseBox_Base
{
    Edit                    aCellEdit;
    ::svt::CheckBoxControl  aCellCheckBox;

    String  sSearch;
    String  sAlternative;
    String  sPrimKey;
    String  sSecKey;
    String  sComment;
    String  sCaseSensitive;
    String  sWordOnly;
    String  sYes;
    String  sNo;

    AutoMarkEntryArr    aEntryArr;

    ::svt::CellControllerRef    xController;
    ::svt::CellControllerRef    xCheckController;

    long    nCurrentRow;
    bool    bModified;


    void                        SetModified() {bModified = true;}

protected:
    virtual sal_Bool                SeekRow( long nRow );
    virtual void                    PaintCell(OutputDevice& rDev, const Rectangle& rRect, sal_uInt16 nColId) const;
    virtual void                    InitController(::svt::CellControllerRef& rController, long nRow, sal_uInt16 nCol);
    virtual ::svt::CellController*  GetController(long nRow, sal_uInt16 nCol);
    virtual sal_Bool                SaveModified();

    std::vector<long>               GetOptimalColWidths() const;

public:
    SwEntryBrowseBox(Window* pParent, VclBuilderContainer* pBuilder);
    void    ReadEntries(SvStream& rInStr);
    void    WriteEntries(SvStream& rOutStr);

    sal_Bool                        IsModified()const;

    virtual OUString GetCellText( long nRow, sal_uInt16 nColumn ) const;
    virtual void Resize();
    virtual Size GetOptimalSize() const;
};

class SwAutoMarkDlg_Impl : public ModalDialog
{
    OKButton*           m_pOKPB;

    SwEntryBrowseBox*   m_pEntriesBB;

    String              sAutoMarkURL;
    const String        sAutoMarkType;

    bool                bCreateMode;

    DECL_LINK(OkHdl, void *);
public:
    SwAutoMarkDlg_Impl(Window* pParent, const String& rAutoMarkURL,
                        const String& rAutoMarkType, bool bCreate);
    ~SwAutoMarkDlg_Impl();

};

sal_uInt16 CurTOXType::GetFlatIndex() const
{
    sal_uInt16 nRet = static_cast< sal_uInt16 >(eType);
    if(eType == TOX_USER && nIndex)
    {
        nRet = static_cast< sal_uInt16 >(TOX_AUTHORITIES + nIndex);
    }
    return nRet;
}

#define EDIT_MINWIDTH 15

SwMultiTOXTabDialog::SwMultiTOXTabDialog(Window* pParent, const SfxItemSet& rSet,
                    SwWrtShell &rShell,
                    SwTOXBase* pCurTOX,
                    sal_uInt16 nToxType, sal_Bool bGlobal)
    : SfxTabDialog(pParent, "TocDialog",
        "modules/swriter/ui/tocdialog.ui", &rSet)
    , pMgr( new SwTOXMgr( &rShell ) )
    , rSh(rShell)
    , pExampleFrame(0)
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

    sal_uInt16 nUserTypeCount = rSh.GetTOXTypeCount(TOX_USER);
    nTypeCount = nUserTypeCount + 6;
    pFormArr = new SwForm*[nTypeCount];
    pDescArr = new SwTOXDescription*[nTypeCount];
    pxIndexSectionsArr = new SwIndexSections_Impl*[nTypeCount];
    //the standard user index is on position TOX_USER
    //all user user indexes follow after position TOX_AUTHORITIES
    if(pCurTOX)
    {
        bEditTOX = sal_True;
    }
    for(int i = nTypeCount - 1; i > -1; i--)
    {
        pFormArr[i] = 0;
        pDescArr[i] = 0;
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
                const SwAuthorityFieldType* pFType = (const SwAuthorityFieldType*)
                                                rSh.GetFldType(RES_AUTHORITY, aEmptyStr);
                if(pFType)
                {
                    String sBrackets;
                    if(pFType->GetPrefix())
                        sBrackets += pFType->GetPrefix();
                    if(pFType->GetSuffix())
                        sBrackets += pFType->GetSuffix();
                    pDescArr[nArrayIndex]->SetAuthBrackets(sBrackets);
                    pDescArr[nArrayIndex]->SetAuthSequence(pFType->IsSequence());
                }
                else
                {
                    pDescArr[nArrayIndex]->SetAuthBrackets(OUString("[]"));
                }
            }
        }
    }
    SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();
    OSL_ENSURE(pFact, "Dialogdiet fail!");
    m_nSelectId = AddTabPage("index", SwTOXSelectTabPage::Create, 0);
    m_nStylesId = AddTabPage("styles", SwTOXStylesTabPage::Create, 0);
    m_nColumnId = AddTabPage("columns", SwColumnPage::Create, 0);
    m_nBackGroundId = AddTabPage("background", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BACKGROUND ), 0);
    m_nEntriesId = AddTabPage("entries", SwTOXEntryTabPage::Create, 0);
    if(!pCurTOX)
        SetCurPageId(m_nSelectId);

    m_pShowExampleCB->SetClickHdl(LINK(this, SwMultiTOXTabDialog, ShowPreviewHdl));

    m_pShowExampleCB->Check( SW_MOD()->GetModuleConfig()->IsShowIndexPreview());

    m_pExampleContainerWIN->SetAccessibleName(m_pShowExampleCB->GetText());
    SetViewAlign( WINDOWALIGN_LEFT );
    // SetViewWindow does not work if the dialog is visible!

    if(!m_pShowExampleCB->IsChecked())
        SetViewWindow(m_pExampleContainerWIN);

    ShowPreviewHdl(0);
}

SwMultiTOXTabDialog::~SwMultiTOXTabDialog()
{
    SW_MOD()->GetModuleConfig()->SetShowIndexPreview(m_pShowExampleCB->IsChecked());

    // fdo#38515 Avoid setting focus on deleted controls in the destructors
    EnableInput( sal_False );

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
}

void    SwMultiTOXTabDialog::PageCreated( sal_uInt16 nId, SfxTabPage &rPage )
{
    if (nId == m_nBackGroundId)
    {
        SfxAllItemSet aSet(*(GetInputSetImpl()->GetPool()));
        aSet.Put (SfxUInt32Item(SID_FLAG_TYPE, SVX_SHOW_SELECTOR));
        rPage.PageCreated(aSet);
    }
    else if(nId == m_nColumnId)
    {
        const SwFmtFrmSize& rSize = (const SwFmtFrmSize&)GetInputSetImpl()->Get(RES_FRM_SIZE);

        ((SwColumnPage&)rPage).SetPageWidth(rSize.GetWidth());
    }
    else if (nId == m_nEntriesId)
        ((SwTOXEntryTabPage&)rPage).SetWrtShell(rSh);
    else if (nId == m_nSelectId)
    {
        ((SwTOXSelectTabPage&)rPage).SetWrtShell(rSh);
        if(USHRT_MAX != nInitialTOXType)
            ((SwTOXSelectTabPage&)rPage).SelectType((TOXTypes)nInitialTOXType);
    }
}

short   SwMultiTOXTabDialog::Ok()
{
    short nRet = SfxTabDialog::Ok();
    SwTOXDescription& rDesc = GetTOXDescription(eCurrentTOXType);
    SwTOXBase aNewDef(*rSh.GetDefaultTOXBase( eCurrentTOXType.eType, true ));

    sal_uInt16 nIndex = static_cast< sal_uInt16 >(eCurrentTOXType.eType);
    if(eCurrentTOXType.eType == TOX_USER && eCurrentTOXType.nIndex)
    {
        nIndex =  static_cast< sal_uInt16 >(TOX_AUTHORITIES + eCurrentTOXType.nIndex);
    }

    if(pFormArr[nIndex])
    {
        rDesc.SetForm(*pFormArr[nIndex]);
        aNewDef.SetTOXForm(*pFormArr[nIndex]);
    }
    rDesc.ApplyTo(aNewDef);
    if(!bGlobalFlag)
        pMgr->UpdateOrInsertTOX(
                rDesc, 0, GetOutputItemSet());
    else if(bEditTOX)
        pMgr->UpdateOrInsertTOX(
                rDesc, &pParamTOXBase, GetOutputItemSet());

    if(!eCurrentTOXType.nIndex)
        rSh.SetDefaultTOXBase(aNewDef);

    return nRet;
}

SwForm* SwMultiTOXTabDialog::GetForm(CurTOXType eType)
{
    sal_uInt16 nIndex = eType.GetFlatIndex();
    if(!pFormArr[nIndex])
        pFormArr[nIndex] = new SwForm(eType.eType);
    return pFormArr[nIndex];
}

SwTOXDescription&   SwMultiTOXTabDialog::GetTOXDescription(CurTOXType eType)
{
    sal_uInt16 nIndex = eType.GetFlatIndex();
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
            const SwAuthorityFieldType* pFType = (const SwAuthorityFieldType*)
                                            rSh.GetFldType(RES_AUTHORITY, aEmptyStr);
            if(pFType)
            {
                String sBrackets = OUString(pFType->GetPrefix());
                sBrackets += pFType->GetSuffix();
                pDescArr[nIndex]->SetAuthBrackets(sBrackets);
                pDescArr[nIndex]->SetAuthSequence(pFType->IsSequence());
            }
            else
            {
                pDescArr[nIndex]->SetAuthBrackets(OUString("[]"));
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

IMPL_LINK_NOARG( SwMultiTOXTabDialog, ShowPreviewHdl )
{
    if(m_pShowExampleCB->IsChecked())
    {
        if(!pExampleFrame && !bExampleCreated)
        {
            bExampleCreated = sal_True;
            OUString sTemplate("internal/idxexample");

            OUString sTemplateWithoutExt( sTemplate );
            sTemplate += ".odt";

            SvtPathOptions aOpt;
            aOpt.SetTemplatePath(String(LIBO_SHARE_FOLDER "/template/common"));
            // 6.0 (extension .sxw)
            bool bExist = aOpt.SearchFile( sTemplate, SvtPathOptions::PATH_TEMPLATE );

            if( !bExist )
            {
                // 6.0 (extension .sxw)
                sTemplate = sTemplateWithoutExt;
                sTemplate += ".sxw";
                bExist = aOpt.SearchFile( sTemplate, SvtPathOptions::PATH_TEMPLATE );
            }
            if( !bExist )
            {
                // 5.0 (extension .vor)
                sTemplate = sTemplateWithoutExt;
                sTemplate += ".sdw";
                bExist = aOpt.SearchFile( sTemplate, SvtPathOptions::PATH_TEMPLATE );
            }

            if(!bExist)
            {
                String sInfo(SW_RES(STR_FILE_NOT_FOUND));
                sInfo.SearchAndReplaceAscii( "%1", sTemplate );
                sInfo.SearchAndReplaceAscii( "%2", aOpt.GetTemplatePath() );
                InfoBox aInfo(GetParent(), sInfo);
                aInfo.Execute();
            }
            else
            {
                Link aLink(LINK(this, SwMultiTOXTabDialog, CreateExample_Hdl));
                String sTemp(sTemplate);
                pExampleFrame = new SwOneExampleFrame(
                        *m_pExampleContainerWIN, EX_SHOW_ONLINE_LAYOUT, &aLink, &sTemp);

                if(!pExampleFrame->IsServiceAvailable())
                {
                    pExampleFrame->CreateErrorMessage(0);
                }
            }
            m_pShowExampleCB->Show(pExampleFrame && pExampleFrame->IsServiceAvailable());
        }
    }
    sal_Bool bSetViewWindow = m_pShowExampleCB->IsChecked()
        && pExampleFrame && pExampleFrame->IsServiceAvailable();

    m_pExampleContainerWIN->Show( bSetViewWindow );
    SetViewWindow( bSetViewWindow ? m_pExampleContainerWIN : 0 );

    setOptimalLayoutSize();

    return 0;
}

sal_Bool SwMultiTOXTabDialog::IsNoNum(SwWrtShell& rSh, const String& rName)
{
    SwTxtFmtColl* pColl = rSh.GetParaStyle(rName);
    if(pColl && ! pColl->IsAssignedToListLevelOfOutlineStyle()) //<-end,zhaojianwei
        return sal_True;

    sal_uInt16 nId = SwStyleNameMapper::GetPoolIdFromUIName(rName, nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL);
    if(nId != USHRT_MAX &&
        ! rSh.GetTxtCollFromPool(nId)->IsAssignedToListLevelOfOutlineStyle())   //<-end,zhaojianwei
        return sal_True;

    return sal_False;
}

class SwIndexTreeLB : public SvTreeListBox
{
    const HeaderBar* pHeaderBar;
public:
    SwIndexTreeLB(Window* pWin, const ResId& rResId) :
        SvTreeListBox(pWin, rResId), pHeaderBar(0){}

    virtual void    KeyInput( const KeyEvent& rKEvt );
    virtual sal_IntPtr GetTabPos( SvTreeListEntry*, SvLBoxTab* );
    void            SetHeaderBar(const HeaderBar* pHB) {pHeaderBar = pHB;}
};

sal_IntPtr SwIndexTreeLB::GetTabPos( SvTreeListEntry* pEntry, SvLBoxTab* pTab)
{
    sal_IntPtr nData = (sal_IntPtr)pEntry->GetUserData();
    if(nData != USHRT_MAX)
    {
        sal_IntPtr  nPos = pHeaderBar->GetItemRect( static_cast< sal_uInt16 >(101 + nData) ).TopLeft().X();
        nData = nPos;
    }
    else
        nData = 0;
    nData += pTab->GetPos();
    return nData;
}

void    SwIndexTreeLB::KeyInput( const KeyEvent& rKEvt )
{
    SvTreeListEntry* pEntry = FirstSelected();
    KeyCode aCode = rKEvt.GetKeyCode();
    sal_Bool bChanged = sal_False;
    if(pEntry)
    {
        sal_IntPtr nLevel = (sal_IntPtr)pEntry->GetUserData();
        if(aCode.GetCode() == KEY_ADD )
        {
            if(nLevel < MAXLEVEL - 1)
                nLevel++;
            else if(nLevel == USHRT_MAX)
                nLevel = 0;
            bChanged = sal_True;
        }
        else if(aCode.GetCode() == KEY_SUBTRACT)
        {
            if(!nLevel)
                nLevel = USHRT_MAX;
            else if(nLevel != USHRT_MAX)
                nLevel--;
            bChanged = sal_True;
        }
        if(bChanged)
        {
            pEntry->SetUserData((void*)nLevel);
            Invalidate();
        }
    }
    if(!bChanged)
        SvTreeListBox::KeyInput(rKEvt);
}

class SwHeaderTree : public Control
{
    HeaderBar       aStylesHB;
    SwIndexTreeLB   aStylesTLB;

public:
    SwHeaderTree(Window* pParent, const ResId rResId);

    HeaderBar&      GetHeaderBar() {return aStylesHB;}
    SwIndexTreeLB&  GetTreeListBox() { return aStylesTLB;}

    virtual void    GetFocus();
};

SwHeaderTree::SwHeaderTree(Window* pParent, const ResId rResId) :
        Control(pParent, rResId),
        aStylesHB(  this, ResId(HB_STYLES, *rResId.GetResMgr())),
        aStylesTLB( this, ResId(TLB_STYLES, *rResId.GetResMgr()))
{
    FreeResource();
    aStylesHB.SetStyle(aStylesHB.GetStyle()|WB_BUTTONSTYLE|WB_TABSTOP|WB_BORDER);
     Size aHBSize(aStylesHB.GetSizePixel());
    aHBSize.Height() = aStylesHB.CalcWindowSizePixel().Height();
    aStylesHB.SetSizePixel(aHBSize);
    aStylesTLB.SetPosPixel(Point(0, aHBSize.Height()));
     Size aTLBSize(aStylesHB.GetSizePixel());
    aTLBSize.Height() = GetOutputSizePixel().Height() - aHBSize.Height();
    aStylesTLB.SetSizePixel(aTLBSize);
    aStylesTLB.SetHeaderBar(&aStylesHB);
}

void    SwHeaderTree::GetFocus()
{
    Control::GetFocus();
    aStylesTLB.GrabFocus();
}

class SwAddStylesDlg_Impl : public SfxModalDialog
{
    OKButton        aOk;
    CancelButton    aCancel;
    HelpButton      aHelp;

    FixedLine       aStylesFL;
    SwHeaderTree    aHeaderTree;
    ImageButton     aLeftPB;
    ImageButton      aRightPB;

    String          sHBFirst;
    String*         pStyleArr;

    DECL_LINK(OkHdl, void *);
    DECL_LINK(LeftRightHdl, PushButton*);
    DECL_LINK(HeaderDragHdl, void *);

public:
    SwAddStylesDlg_Impl(Window* pParent, SwWrtShell& rWrtSh, String rStringArr[]);
    ~SwAddStylesDlg_Impl();
};

SwAddStylesDlg_Impl::SwAddStylesDlg_Impl(Window* pParent,
            SwWrtShell& rWrtSh, String rStringArr[]) :
    SfxModalDialog(pParent, SW_RES(DLG_ADD_IDX_STYLES)),
    aOk(        this, SW_RES(PB_OK      )),
    aCancel(    this, SW_RES(PB_CANCEL  )),
    aHelp(      this, SW_RES(PB_HELP        )),

    aStylesFL(  this, SW_RES(FL_STYLES   )),
    aHeaderTree(this, SW_RES(TR_HEADER   )),
    aLeftPB(    this, SW_RES(PB_LEFT     )),
    aRightPB(   this, SW_RES(PB_RIGHT    )),

    sHBFirst(   SW_RES(ST_HB_FIRST)),
    pStyleArr(rStringArr)
{
    FreeResource();

    aHeaderTree.SetAccessibleRelationMemberOf(&aStylesFL);
    aLeftPB.SetAccessibleRelationMemberOf(&aStylesFL);
    aRightPB.SetAccessibleRelationMemberOf(&aStylesFL);

    aOk.SetClickHdl(LINK(this, SwAddStylesDlg_Impl, OkHdl));
    aLeftPB.SetClickHdl(LINK(this, SwAddStylesDlg_Impl, LeftRightHdl));
    aRightPB.SetClickHdl(LINK(this, SwAddStylesDlg_Impl, LeftRightHdl));

    HeaderBar& rHB = aHeaderTree.GetHeaderBar();
    rHB.SetEndDragHdl(LINK(this, SwAddStylesDlg_Impl, HeaderDragHdl));

    long nWidth = rHB.GetSizePixel().Width();
    sal_uInt16 i;

    nWidth /= 14;
    nWidth--;
    rHB.InsertItem( 100, sHBFirst,  4 * nWidth );
    for( i = 1; i <= MAXLEVEL; i++)
        rHB.InsertItem( 100 + i, OUString::number(i), nWidth );
    rHB.Show();

    SwIndexTreeLB& rTLB = aHeaderTree.GetTreeListBox();
    rTLB.SetStyle(rTLB.GetStyle()|WB_CLIPCHILDREN|WB_SORT);
    rTLB.GetModel()->SetSortMode(SortAscending);
    for(i = 0; i < MAXLEVEL; ++i)
    {
        String sStyles(rStringArr[i]);
        for(sal_uInt16 nToken = 0; nToken < comphelper::string::getTokenCount(sStyles, TOX_STYLE_DELIMITER); nToken++)
        {
            String sTmp(sStyles.GetToken(nToken, TOX_STYLE_DELIMITER));
            SvTreeListEntry* pEntry = rTLB.InsertEntry(sTmp);
            pEntry->SetUserData(reinterpret_cast<void*>(i));
        }
    }
    // now the other styles
    //
    const SwTxtFmtColl *pColl   = 0;
    const sal_uInt16 nSz = rWrtSh.GetTxtFmtCollCount();

    for ( sal_uInt16 j = 0;j < nSz; ++j )
    {
        pColl = &rWrtSh.GetTxtFmtColl(j);
        if(pColl->IsDefault())
            continue;

        const OUString aName = pColl->GetName();
        if (!aName.isEmpty())
        {
            SvTreeListEntry* pEntry = rTLB.First();
            while (pEntry && rTLB.GetEntryText(pEntry)!=aName)
            {
                pEntry = rTLB.Next(pEntry);
            }
            if (!pEntry)
            {
                rTLB.InsertEntry(aName)->SetUserData((void*)USHRT_MAX);
            }
        }
    }
    rTLB.GetModel()->Resort();
}

SwAddStylesDlg_Impl::~SwAddStylesDlg_Impl()
{
}

IMPL_LINK_NOARG(SwAddStylesDlg_Impl, OkHdl)
{
    for(sal_uInt16 i = 0; i < MAXLEVEL; i++)
        pStyleArr[i].Erase();

    SwIndexTreeLB& rTLB = aHeaderTree.GetTreeListBox();
    SvTreeListEntry* pEntry = rTLB.First();
    while(pEntry)
    {
        sal_IntPtr nLevel = (sal_IntPtr)pEntry->GetUserData();
        if(nLevel != USHRT_MAX)
        {
            String sName(rTLB.GetEntryText(pEntry));
            if(pStyleArr[nLevel].Len())
                pStyleArr[nLevel] += TOX_STYLE_DELIMITER;
            pStyleArr[nLevel] += sName;
        }
        pEntry = rTLB.Next(pEntry);
    }

    //TODO write back style names
    EndDialog(RET_OK);
    return 0;
}

IMPL_LINK_NOARG(SwAddStylesDlg_Impl, HeaderDragHdl)
{
    aHeaderTree.GetTreeListBox().Invalidate();
    return 0;
}

IMPL_LINK(SwAddStylesDlg_Impl, LeftRightHdl, PushButton*, pBtn)
{
    sal_Bool bLeft = pBtn == &aLeftPB;
    SvTreeListEntry* pEntry = aHeaderTree.GetTreeListBox().FirstSelected();
    if(pEntry)
    {
        sal_IntPtr nLevel = (sal_IntPtr)pEntry->GetUserData();
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
        pEntry->SetUserData((void*)nLevel);
        aHeaderTree.GetTreeListBox().Invalidate();
    }
    return 0;
}

SwTOXSelectTabPage::SwTOXSelectTabPage(Window* pParent, const SfxItemSet& rAttrSet) :
    SfxTabPage(pParent, SW_RES(TP_TOX_SELECT), rAttrSet),

    aTypeTitleFL(       this, SW_RES(FL_TYPETITLE        )),
    aTitleFT(           this, SW_RES(FT_TITLE            )),
    aTitleED(           this, SW_RES(ED_TITLE           )),
    aTypeFT(            this, SW_RES(FT_TYPE                )),
    aTypeLB(            this, SW_RES(LB_TYPE                )),
    aReadOnlyCB(        this, SW_RES(CB_READONLY            )),

    aAreaFL(            this, SW_RES(FL_AREA             )),
    aAreaFT(            this, SW_RES(FT_AREA             )),
    aAreaLB(            this, SW_RES(LB_AREA                )),
    aLevelFT(           this, SW_RES(FT_LEVEL           )),
    aLevelNF(           this, SW_RES(NF_LEVEL           )),

    aCreateFromFL(      this, SW_RES(FL_CREATEFROM       )),
    aFromHeadingsCB(    this, SW_RES(CB_FROMHEADINGS     )),
    aAddStylesCB(       this, SW_RES(CB_ADDSTYLES       )),
    aAddStylesPB(       this, SW_RES(PB_ADDSTYLES       )),

    aFromTablesCB(      this, SW_RES(CB_FROMTABLES       )),
    aFromFramesCB(      this, SW_RES(CB_FROMFRAMES      )),
    aFromGraphicsCB(    this, SW_RES(CB_FROMGRAPHICS        )),
    aFromOLECB(         this, SW_RES(CB_FROMOLE         )),
    aLevelFromChapterCB(this, SW_RES(CB_LEVELFROMCHAPTER    )),

    aFromCaptionsRB(    this, SW_RES(RB_FROMCAPTIONS     )),
    aFromObjectNamesRB( this, SW_RES(RB_FROMOBJECTNAMES )),

    aCaptionSequenceFT( this, SW_RES(FT_CAPTIONSEQUENCE  )),
    aCaptionSequenceLB( this, SW_RES(LB_CAPTIONSEQUENCE  )),
    aDisplayTypeFT(     this, SW_RES(FT_DISPLAYTYPE     )),
    aDisplayTypeLB(     this, SW_RES(LB_DISPLAYTYPE     )),

    aTOXMarksCB(        this, SW_RES(CB_TOXMARKS         )),

    aIdxOptionsFL(      this, SW_RES(FL_IDXOPTIONS       )),
    aCollectSameCB(     this, SW_RES(CB_COLLECTSAME      )),
    aUseFFCB(           this, SW_RES(CB_USEFF           )),
    aUseDashCB(         this, SW_RES(CB_USE_DASH            )),
    aCaseSensitiveCB(   this, SW_RES(CB_CASESENSITIVE   )),
    aInitialCapsCB(     this, SW_RES(CB_INITIALCAPS     )),
    aKeyAsEntryCB(      this, SW_RES(CB_KEYASENTRY      )),
    aFromFileCB(        this, SW_RES(CB_FROMFILE            )),
    aAutoMarkPB(        this, SW_RES(MB_AUTOMARK            )),

    aFromNames(         SW_RES(RES_SRCTYPES              )),
    aFromObjCLB(        this, SW_RES(CLB_FROMOBJ            )),
    aFromObjFL(         this, SW_RES(FL_FROMOBJ          )),

    aSequenceCB(        this, SW_RES(CB_SEQUENCE         )),
    aBracketFT(         this, SW_RES(FT_BRACKET         )),
    aBracketLB(         this, SW_RES(LB_BRACKET         )),
    aAuthorityFormatFL( this, SW_RES(FL_AUTHORITY        )),

    aSortOptionsFL(     this, SW_RES(FL_SORTOPTIONS      )),
    aLanguageFT(        this, SW_RES(FT_LANGUAGE         )),
    aLanguageLB(        this, SW_RES(LB_LANGUAGE         )),
    aSortAlgorithmFT(   this, SW_RES(FT_SORTALG          )),
    aSortAlgorithmLB(   this, SW_RES(LB_SORTALG          )),

    pIndexRes(0),

    sAutoMarkType(SW_RES(ST_AUTOMARK_TYPE)),
    sAddStyleUser(SW_RES(ST_USER_ADDSTYLE)),
    bFirstCall(sal_True)
{
    aBracketLB.InsertEntry(String(SW_RES(ST_NO_BRACKET)), 0);

    aAddStylesPB.SetAccessibleRelationMemberOf(&aCreateFromFL);
    aAddStylesPB.SetAccessibleRelationLabeledBy(&aAddStylesCB);
    aAddStylesPB.SetAccessibleName(aAddStylesCB.GetText());

    FreeResource();

    pIndexEntryWrapper = new IndexEntrySupplierWrapper();

    aLanguageLB.SetLanguageList( LANG_LIST_ALL | LANG_LIST_ONLY_KNOWN,
                                 sal_False, sal_False, sal_False );

    sAddStyleContent = aAddStylesCB.GetText();

    aCBLeftPos1 = aFromHeadingsCB.GetPosPixel();
    aCBLeftPos2 = aAddStylesCB.GetPosPixel();
    aCBLeftPos3 = aTOXMarksCB.GetPosPixel();

    ResStringArray& rNames = aFromNames.GetNames();
    for(sal_uInt16 i = 0; i < rNames.Count(); i++)
    {
        aFromObjCLB.InsertEntry(rNames.GetString(i));
        aFromObjCLB.SetEntryData( i, (void*)rNames.GetValue(i) );
    }
    aFromObjCLB.SetHelpId(HID_OLE_CHECKLB);

    SetExchangeSupport();
    aTypeLB.SetSelectHdl(LINK(this, SwTOXSelectTabPage, TOXTypeHdl));

    aAddStylesPB.SetClickHdl(LINK(this, SwTOXSelectTabPage, AddStylesHdl));

    PopupMenu*  pMenu = aAutoMarkPB.GetPopupMenu();
    pMenu->SetActivateHdl(LINK(this, SwTOXSelectTabPage, MenuEnableHdl));
    pMenu->SetSelectHdl(LINK(this, SwTOXSelectTabPage, MenuExecuteHdl));

    Link aLk =  LINK(this, SwTOXSelectTabPage, CheckBoxHdl);
    aAddStylesCB    .SetClickHdl(aLk);
    aFromHeadingsCB .SetClickHdl(aLk);
    aTOXMarksCB     .SetClickHdl(aLk);
    aFromFileCB     .SetClickHdl(aLk);
    aCollectSameCB  .SetClickHdl(aLk);
    aUseFFCB        .SetClickHdl(aLk);
    aUseDashCB      .SetClickHdl(aLk);
    aInitialCapsCB  .SetClickHdl(aLk);
    aKeyAsEntryCB   .SetClickHdl(aLk);

    Link aModifyLk = LINK(this, SwTOXSelectTabPage, ModifyHdl);
    aTitleED.SetModifyHdl(aModifyLk);
    aLevelNF.SetModifyHdl(aModifyLk);
    aSortAlgorithmLB.SetSelectHdl(aModifyLk);

    aLk =  LINK(this, SwTOXSelectTabPage, RadioButtonHdl);
    aFromCaptionsRB.SetClickHdl(aLk);
    aFromObjectNamesRB.SetClickHdl(aLk);
    RadioButtonHdl(&aFromCaptionsRB);

    aLanguageLB.SetSelectHdl(LINK(this, SwTOXSelectTabPage, LanguageHdl));
    aTypeLB.SelectEntryPos(0);
    aTitleED.SaveValue();
}

SwTOXSelectTabPage::~SwTOXSelectTabPage()
{
    delete pIndexRes;
    delete pIndexEntryWrapper;
}

void SwTOXSelectTabPage::SetWrtShell(SwWrtShell& rSh)
{
    sal_uInt16 nUserTypeCount = rSh.GetTOXTypeCount(TOX_USER);
    if(nUserTypeCount > 1)
    {
        //insert all new user indexes names after the standard user index
        sal_uInt16 nPos = aTypeLB.GetEntryPos((void*)(sal_uInt32)TO_USER);
        nPos++;
        for(sal_uInt16 nUser = 1; nUser < nUserTypeCount; nUser++)
        {
            nPos = aTypeLB.InsertEntry(rSh.GetTOXType(TOX_USER, nUser)->GetTypeName(), nPos);
            sal_uIntPtr nEntryData = nUser << 8;
            nEntryData |= TO_USER;
            aTypeLB.SetEntryData(nPos, (void*)nEntryData);
        }
    }
}

sal_Bool SwTOXSelectTabPage::FillItemSet( SfxItemSet& )
{
    return sal_True;
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
    }
    return nRet;
}

void SwTOXSelectTabPage::SelectType(TOXTypes eSet)
{
    CurTOXType eCurType (eSet, 0);

    sal_IntPtr nData = lcl_TOXTypesToUserData(eCurType);
    aTypeLB.SelectEntryPos(aTypeLB.GetEntryPos((void*)nData));
    aTypeFT.Enable(sal_False);
    aTypeLB.Enable(sal_False);
    TOXTypeHdl(&aTypeLB);
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
        default: OSL_FAIL("what a type?");
    }
    return eRet;
}

void    SwTOXSelectTabPage::ApplyTOXDescription()
{
    SwMultiTOXTabDialog* pTOXDlg = (SwMultiTOXTabDialog*)GetTabDialog();
    const CurTOXType aCurType = pTOXDlg->GetCurrentTOXType();
    SwTOXDescription& rDesc = pTOXDlg->GetTOXDescription(aCurType);
    aReadOnlyCB.Check(rDesc.IsReadonly());
    if(aTitleED.GetText() == aTitleED.GetSavedValue())
    {
        if(rDesc.GetTitle())
            aTitleED.SetText(*rDesc.GetTitle());
        else
            aTitleED.SetText(aEmptyStr);
        aTitleED.SaveValue();
    }

    aAreaLB.SelectEntryPos(rDesc.IsFromChapter() ? 1 : 0);

    if(aCurType.eType != TOX_INDEX)
        aLevelNF.SetValue(rDesc.GetLevel());   //content, user

    sal_uInt16 nCreateType = rDesc.GetContentOptions();

    //user + content
    sal_Bool bHasStyleNames = sal_False;
    sal_uInt16 i;

    for( i = 0; i < MAXLEVEL; i++)
        if(rDesc.GetStyleNames(i).Len())
        {
            bHasStyleNames = sal_True;
            break;
        }
    aAddStylesCB.Check(bHasStyleNames && (nCreateType & nsSwTOXElement::TOX_TEMPLATE));

    aFromOLECB.     Check( 0 != (nCreateType & nsSwTOXElement::TOX_OLE) );
    aFromTablesCB.  Check( 0 != (nCreateType & nsSwTOXElement::TOX_TABLE) );
    aFromGraphicsCB.Check( 0 != (nCreateType & nsSwTOXElement::TOX_GRAPHIC) );
    aFromFramesCB.  Check( 0 != (nCreateType & nsSwTOXElement::TOX_FRAME) );

    aLevelFromChapterCB.Check(rDesc.IsLevelFromChapter());

    //all but illustration and table
    aTOXMarksCB.Check( 0 != (nCreateType & nsSwTOXElement::TOX_MARK) );

    //content
    if(TOX_CONTENT == aCurType.eType)
    {
        aFromHeadingsCB.Check( 0 != (nCreateType & nsSwTOXElement::TOX_OUTLINELEVEL) );
        aAddStylesCB.SetText(sAddStyleContent);
        aAddStylesPB.Enable(aAddStylesCB.IsChecked());
    }
    //index only
    else if(TOX_INDEX == aCurType.eType)
    {
        sal_uInt16 nIndexOptions = rDesc.GetIndexOptions();
        aCollectSameCB.     Check( 0 != (nIndexOptions & nsSwTOIOptions::TOI_SAME_ENTRY) );
        aUseFFCB.           Check( 0 != (nIndexOptions & nsSwTOIOptions::TOI_FF) );
        aUseDashCB.         Check( 0 != (nIndexOptions & nsSwTOIOptions::TOI_DASH) );
        if(aUseFFCB.IsChecked())
            aUseDashCB.Enable(sal_False);
        else if(aUseDashCB.IsChecked())
            aUseFFCB.Enable(sal_False);

        aCaseSensitiveCB.   Check( 0 != (nIndexOptions & nsSwTOIOptions::TOI_CASE_SENSITIVE) );
        aInitialCapsCB.     Check( 0 != (nIndexOptions & nsSwTOIOptions::TOI_INITIAL_CAPS) );
        aKeyAsEntryCB.      Check( 0 != (nIndexOptions & nsSwTOIOptions::TOI_KEY_AS_ENTRY) );
    }
    else if(TOX_ILLUSTRATIONS == aCurType.eType ||
        TOX_TABLES == aCurType.eType)
    {
        aFromObjectNamesRB.Check(rDesc.IsCreateFromObjectNames());
        aFromCaptionsRB.Check(!rDesc.IsCreateFromObjectNames());
        aCaptionSequenceLB.SelectEntry(rDesc.GetSequenceName());
        aDisplayTypeLB.SelectEntryPos( static_cast< sal_uInt16 >(rDesc.GetCaptionDisplay()) );
        RadioButtonHdl(&aFromCaptionsRB);

    }
    else if(TOX_OBJECTS == aCurType.eType)
    {
        long nOLEData = rDesc.GetOLEOptions();
        for(sal_uInt16 nFromObj = 0; nFromObj < aFromObjCLB.GetEntryCount(); nFromObj++)
        {
            sal_IntPtr nData = (sal_IntPtr)aFromObjCLB.GetEntryData(nFromObj);
            aFromObjCLB.CheckEntryPos(nFromObj, 0 != (nData & nOLEData));
        }
    }
    else if(TOX_AUTHORITIES == aCurType.eType)
    {
        String sBrackets(rDesc.GetAuthBrackets());
        if(!sBrackets.Len() || sBrackets.EqualsAscii("  "))
            aBracketLB.SelectEntryPos(0);
        else
            aBracketLB.SelectEntry(sBrackets);
        aSequenceCB.Check(rDesc.IsAuthSequence());
    }
    aAutoMarkPB.Enable(aFromFileCB.IsChecked());

    for(i = 0; i < MAXLEVEL; i++)
        aStyleArr[i] = rDesc.GetStyleNames(i);

    aLanguageLB.SelectLanguage(rDesc.GetLanguage());
    LanguageHdl(0);
    for( long nCnt = 0; nCnt < aSortAlgorithmLB.GetEntryCount(); ++nCnt )
    {
        const String* pEntryData = (const String*)aSortAlgorithmLB.GetEntryData( (sal_uInt16)nCnt );
        OSL_ENSURE(pEntryData, "no entry data available");
        if( pEntryData && *pEntryData == rDesc.GetSortAlgorithm())
        {
            aSortAlgorithmLB.SelectEntryPos( (sal_uInt16)nCnt );
            break;
        }
    }
}

void SwTOXSelectTabPage::FillTOXDescription()
{
    SwMultiTOXTabDialog* pTOXDlg = (SwMultiTOXTabDialog*)GetTabDialog();
    CurTOXType aCurType = pTOXDlg->GetCurrentTOXType();
    SwTOXDescription& rDesc = pTOXDlg->GetTOXDescription(aCurType);
    rDesc.SetTitle(aTitleED.GetText());
    rDesc.SetFromChapter(1 == aAreaLB.GetSelectEntryPos());
    sal_uInt16 nContentOptions = 0;
    if(aTOXMarksCB.IsVisible() && aTOXMarksCB.IsChecked())
        nContentOptions |= nsSwTOXElement::TOX_MARK;

    sal_uInt16 nIndexOptions = rDesc.GetIndexOptions()&nsSwTOIOptions::TOI_ALPHA_DELIMITTER;
    switch(rDesc.GetTOXType())
    {
        case TOX_CONTENT:
            if(aFromHeadingsCB.IsChecked())
                nContentOptions |= nsSwTOXElement::TOX_OUTLINELEVEL;
        break;
        case TOX_USER:
        {
            rDesc.SetTOUName(aTypeLB.GetSelectEntry());

            if(aFromOLECB.IsChecked())
                nContentOptions |= nsSwTOXElement::TOX_OLE;
            if(aFromTablesCB.IsChecked())
                nContentOptions |= nsSwTOXElement::TOX_TABLE;
            if(aFromFramesCB.IsChecked())
                nContentOptions |= nsSwTOXElement::TOX_FRAME;
            if(aFromGraphicsCB.IsChecked())
                nContentOptions |= nsSwTOXElement::TOX_GRAPHIC;
        }
        break;
        case  TOX_INDEX:
        {
            nContentOptions = nsSwTOXElement::TOX_MARK;

            if(aCollectSameCB.IsChecked())
                nIndexOptions |= nsSwTOIOptions::TOI_SAME_ENTRY;
            if(aUseFFCB.IsChecked())
                nIndexOptions |= nsSwTOIOptions::TOI_FF;
            if(aUseDashCB.IsChecked())
                nIndexOptions |= nsSwTOIOptions::TOI_DASH;
            if(aCaseSensitiveCB.IsChecked())
                nIndexOptions |= nsSwTOIOptions::TOI_CASE_SENSITIVE;
            if(aInitialCapsCB.IsChecked())
                nIndexOptions |= nsSwTOIOptions::TOI_INITIAL_CAPS;
            if(aKeyAsEntryCB.IsChecked())
                nIndexOptions |= nsSwTOIOptions::TOI_KEY_AS_ENTRY;
            if(aFromFileCB.IsChecked())
                rDesc.SetAutoMarkURL(sAutoMarkURL);
            else
                rDesc.SetAutoMarkURL(aEmptyStr);
        }
        break;
        case TOX_ILLUSTRATIONS:
        case TOX_TABLES :
            rDesc.SetCreateFromObjectNames(aFromObjectNamesRB.IsChecked());
            rDesc.SetSequenceName(aCaptionSequenceLB.GetSelectEntry());
            rDesc.SetCaptionDisplay((SwCaptionDisplay)aDisplayTypeLB.GetSelectEntryPos());
        break;
        case TOX_OBJECTS:
        {
            long nOLEData = 0;
            for(sal_uInt16 i = 0; i < aFromObjCLB.GetEntryCount(); i++)
            {
                if(aFromObjCLB.IsChecked(i))
                {
                    sal_IntPtr nData = (sal_IntPtr)aFromObjCLB.GetEntryData(i);
                    nOLEData |= nData;
                }
            }
            rDesc.SetOLEOptions((sal_uInt16)nOLEData);
        }
        break;
        case TOX_AUTHORITIES:
        {
            if(aBracketLB.GetSelectEntryPos())
                rDesc.SetAuthBrackets(aBracketLB.GetSelectEntry());
            else
                rDesc.SetAuthBrackets(aEmptyStr);
            rDesc.SetAuthSequence(aSequenceCB.IsChecked());
        }
        break;
    }

    rDesc.SetLevelFromChapter(  aLevelFromChapterCB.IsVisible() &&
                                aLevelFromChapterCB.IsChecked());
    if(aTOXMarksCB.IsChecked() && aTOXMarksCB.IsVisible())
        nContentOptions |= nsSwTOXElement::TOX_MARK;
    if(aFromHeadingsCB.IsChecked() && aFromHeadingsCB.IsVisible())
        nContentOptions |= nsSwTOXElement::TOX_OUTLINELEVEL;
    if(aAddStylesCB.IsChecked() && aAddStylesCB.IsVisible())
        nContentOptions |= nsSwTOXElement::TOX_TEMPLATE;

    rDesc.SetContentOptions(nContentOptions);
    rDesc.SetIndexOptions(nIndexOptions);
    rDesc.SetLevel( static_cast< sal_uInt8 >(aLevelNF.GetValue()) );

    rDesc.SetReadonly(aReadOnlyCB.IsChecked());

    for(sal_uInt16 i = 0; i < MAXLEVEL; i++)
        rDesc.SetStyleNames(aStyleArr[i], i);

    rDesc.SetLanguage(aLanguageLB.GetSelectLanguage());
    const String* pEntryData = (const String*)aSortAlgorithmLB.GetEntryData(
                                            aSortAlgorithmLB.GetSelectEntryPos() );
    OSL_ENSURE(pEntryData, "no entry data available");
    if(pEntryData)
        rDesc.SetSortAlgorithm(*pEntryData);
}

void SwTOXSelectTabPage::Reset( const SfxItemSet& )
{
    SwMultiTOXTabDialog* pTOXDlg = (SwMultiTOXTabDialog*)GetTabDialog();
    SwWrtShell& rSh = pTOXDlg->GetWrtShell();
    const CurTOXType aCurType = pTOXDlg->GetCurrentTOXType();
    sal_IntPtr nData = lcl_TOXTypesToUserData(aCurType);
    aTypeLB.SelectEntryPos(aTypeLB.GetEntryPos((void*)nData));

    sAutoMarkURL = INetURLObject::decode( rSh.GetTOIAutoMarkURL(),
                                        INET_HEX_ESCAPE,
                                           INetURLObject::DECODE_UNAMBIGUOUS,
                                        RTL_TEXTENCODING_UTF8 );
    aFromFileCB.Check( 0 != sAutoMarkURL.Len() );

    aCaptionSequenceLB.Clear();
    sal_uInt16 i, nCount = rSh.GetFldTypeCount(RES_SETEXPFLD);
    for (i = 0; i < nCount; i++)
    {
        SwFieldType *pType = rSh.GetFldType( i, RES_SETEXPFLD );
        if( pType->Which() == RES_SETEXPFLD &&
            ((SwSetExpFieldType *) pType)->GetType() & nsSwGetSetExpType::GSE_SEQ )
            aCaptionSequenceLB.InsertEntry(pType->GetName());
    }

    if(pTOXDlg->IsTOXEditMode())
    {
        aTypeFT.Enable(sal_False);
        aTypeLB.Enable(sal_False);
    }
    TOXTypeHdl(&aTypeLB);
    CheckBoxHdl(&aAddStylesCB);
}

void SwTOXSelectTabPage::ActivatePage( const SfxItemSet& )
{
    //nothing to do
}

int SwTOXSelectTabPage::DeactivatePage( SfxItemSet* _pSet )
{
    if(_pSet)
        _pSet->Put(SfxUInt16Item(FN_PARAM_TOX_TYPE,
            (sal_uInt16)(sal_IntPtr)aTypeLB.GetEntryData( aTypeLB.GetSelectEntryPos() )));
    FillTOXDescription();
    return LEAVE_PAGE;
}

SfxTabPage* SwTOXSelectTabPage::Create( Window* pParent, const SfxItemSet& rAttrSet)
{
    return new SwTOXSelectTabPage(pParent, rAttrSet);
}

IMPL_LINK(SwTOXSelectTabPage, TOXTypeHdl,   ListBox*, pBox)
{
    SwMultiTOXTabDialog* pTOXDlg = (SwMultiTOXTabDialog*)GetTabDialog();
    if(!bFirstCall)
    {
        // save current values into the proper TOXDescription
        FillTOXDescription();
    }
    bFirstCall = sal_False;
    const sal_uInt16 nType =  sal::static_int_cast< sal_uInt16 >(reinterpret_cast< sal_uIntPtr >(
                                pBox->GetEntryData( pBox->GetSelectEntryPos() )));
    CurTOXType eCurType = lcl_UserData2TOXTypes(nType);
    pTOXDlg->SetCurrentTOXType(eCurType);

    aAreaLB.Show( 0 != (nType & (TO_CONTENT|TO_ILLUSTRATION|TO_USER|TO_INDEX|TO_TABLE|TO_OBJECT)) );
    aLevelFT.Show( 0 != (nType & (TO_CONTENT)) );
    aLevelNF.Show( 0 != (nType & (TO_CONTENT)) );
    aLevelFromChapterCB.Show( 0 != (nType & (TO_USER)) );
    aAreaFT.Show( 0 != (nType & (TO_CONTENT|TO_ILLUSTRATION|TO_USER|TO_INDEX|TO_TABLE|TO_OBJECT)) );
    aAreaFL.Show( 0 != (nType & (TO_CONTENT|TO_ILLUSTRATION|TO_USER|TO_INDEX|TO_TABLE|TO_OBJECT)) );

    aFromHeadingsCB.Show( 0 != (nType & (TO_CONTENT)) );
    aAddStylesCB.Show( 0 != (nType & (TO_CONTENT|TO_USER)) );
    aAddStylesPB.Show( 0 != (nType & (TO_CONTENT|TO_USER)) );

    aFromTablesCB.Show( 0 != (nType & (TO_USER)) );
    aFromFramesCB.Show( 0 != (nType & (TO_USER)) );
    aFromGraphicsCB.Show( 0 != (nType & (TO_USER)) );
    aFromOLECB.Show( 0 != (nType & (TO_USER)) );

    aFromCaptionsRB.Show( 0 != (nType & (TO_ILLUSTRATION|TO_TABLE)) );
    aFromObjectNamesRB.Show( 0 != (nType & (TO_ILLUSTRATION|TO_TABLE)) );

    aTOXMarksCB.Show( 0 != (nType & (TO_CONTENT|TO_USER)) );

    aCreateFromFL.Show( 0 != (nType & (TO_CONTENT|TO_ILLUSTRATION|TO_USER|TO_TABLE)) );
    aCaptionSequenceFT.Show( 0 != (nType & (TO_ILLUSTRATION|TO_TABLE)) );
    aCaptionSequenceLB.Show( 0 != (nType & (TO_ILLUSTRATION|TO_TABLE)) );
    aDisplayTypeFT.Show( 0 != (nType & (TO_ILLUSTRATION|TO_TABLE)) );
    aDisplayTypeLB.Show( 0 != (nType & (TO_ILLUSTRATION|TO_TABLE)) );

    aSequenceCB.Show( 0 != (nType & TO_AUTHORITIES) );
    aBracketFT.Show( 0 != (nType & TO_AUTHORITIES) );
    aBracketLB.Show( 0 != (nType & TO_AUTHORITIES) );
    aAuthorityFormatFL.Show( 0 != (nType & TO_AUTHORITIES) );

    sal_Bool bEnableSortLanguage = 0 != (nType & (TO_INDEX|TO_AUTHORITIES));
    aSortOptionsFL.Show(bEnableSortLanguage);
    aLanguageFT.Show(bEnableSortLanguage);
    aLanguageLB.Show(bEnableSortLanguage);
    aSortAlgorithmFT.Show(bEnableSortLanguage);
    aSortAlgorithmLB.Show(bEnableSortLanguage);

        // initialize button positions

    //#i111993# add styles button has two different positions
    if( !aAddStylesPosDef.X() )
    {
        aAddStylesPosDef = ( aAddStylesPB.GetPosPixel() );
        // move left!
        Point aPos(aAddStylesPosDef);
        aPos.X() -= 2 * aAddStylesPB.GetSizePixel().Width();
        aAddStylesPosUser = aPos;
    }

    if( nType & TO_ILLUSTRATION )       //add by zhaojianwei
        aCaptionSequenceLB.SelectEntry( SwStyleNameMapper::GetUIName(
                                    RES_POOLCOLL_LABEL_ABB, aEmptyStr ));
    else if( nType & TO_TABLE )
        aCaptionSequenceLB.SelectEntry( SwStyleNameMapper::GetUIName(
                                    RES_POOLCOLL_LABEL_TABLE, aEmptyStr ));
    else if( nType & TO_USER )
    {
        aAddStylesCB.SetText(sAddStyleUser);
        aAddStylesPB.SetPosPixel(aAddStylesPosUser);
    }
    else if( nType & TO_CONTENT )
    {
        aAddStylesPB.SetPosPixel(aAddStylesPosDef);
    }

    aCollectSameCB.Show( 0 != (nType & TO_INDEX) );
    aUseFFCB.Show( 0 != (nType & TO_INDEX) );
    aUseDashCB.Show( 0 != (nType & TO_INDEX) );
    aCaseSensitiveCB.Show( 0 != (nType & TO_INDEX) );
    aInitialCapsCB.Show( 0 != (nType & TO_INDEX) );
    aKeyAsEntryCB.Show( 0 != (nType & TO_INDEX) );
    aFromFileCB.Show( 0 != (nType & TO_INDEX) );
    aAutoMarkPB.Show( 0 != (nType & TO_INDEX) );

    aIdxOptionsFL.Show( 0 != (nType & TO_INDEX) );

    //object index
    aFromObjCLB.Show( 0 != (nType & TO_OBJECT) );
    aFromObjFL.Show( 0 != (nType & TO_OBJECT) );

    //move controls
    aAddStylesCB.SetPosPixel(nType & TO_USER ? aCBLeftPos1 : aCBLeftPos2);
 Point aPBPos(aAddStylesPB.GetPosPixel());
    aPBPos.Y() = nType & TO_USER ? aCBLeftPos1.Y() : aCBLeftPos2.Y();
    aAddStylesPB.SetPosPixel(aPBPos);
    aTOXMarksCB.SetPosPixel(nType & TO_USER ? aCBLeftPos2 : aCBLeftPos3);
    //set control values from the proper TOXDescription
    {
        ApplyTOXDescription();
    }
    ModifyHdl(0);
    return 0;
}

IMPL_LINK_NOARG(SwTOXSelectTabPage, ModifyHdl)
{
    SwMultiTOXTabDialog* pTOXDlg = (SwMultiTOXTabDialog*)GetTabDialog();
    if(pTOXDlg)
    {
        FillTOXDescription();
        pTOXDlg->CreateOrUpdateExample(pTOXDlg->GetCurrentTOXType().eType, TOX_PAGE_SELECT);
    }
    return 0;
}

IMPL_LINK(SwTOXSelectTabPage, CheckBoxHdl,  CheckBox*, pBox )
{
    SwMultiTOXTabDialog* pTOXDlg = (SwMultiTOXTabDialog*)GetTabDialog();
    const CurTOXType aCurType = pTOXDlg->GetCurrentTOXType();
    if(TOX_CONTENT == aCurType.eType)
    {
        //at least one of the three CheckBoxes must be checked
        if(!aAddStylesCB.IsChecked() && !aFromHeadingsCB.IsChecked() && !aTOXMarksCB.IsChecked())
        {
            //TODO: InfoBox?
            pBox->Check(sal_True);
        }
        aAddStylesPB.Enable(aAddStylesCB.IsChecked());
    }
    if(TOX_USER == aCurType.eType)
    {
        aAddStylesPB.Enable(aAddStylesCB.IsChecked());
    }
    else if(TOX_INDEX == aCurType.eType)
    {
        aAutoMarkPB.Enable(aFromFileCB.IsChecked());
        aUseFFCB.Enable(aCollectSameCB.IsChecked() && !aUseDashCB.IsChecked());
        aUseDashCB.Enable(aCollectSameCB.IsChecked() && !aUseFFCB.IsChecked());
        aCaseSensitiveCB.Enable(aCollectSameCB.IsChecked());
    }
    ModifyHdl(0);
    return 0;
};

IMPL_LINK_NOARG(SwTOXSelectTabPage, RadioButtonHdl)
{
    sal_Bool bEnable = aFromCaptionsRB.IsChecked();
    aCaptionSequenceFT.Enable(bEnable);
    aCaptionSequenceLB.Enable(bEnable);
    aDisplayTypeFT.Enable(bEnable);
    aDisplayTypeLB.Enable(bEnable);
    ModifyHdl(0);
    return 0;
}

IMPL_LINK(SwTOXSelectTabPage, LanguageHdl, ListBox*, pBox)
{
    lang::Locale aLcl( LanguageTag( aLanguageLB.GetSelectLanguage() ).getLocale() );
    Sequence< OUString > aSeq = pIndexEntryWrapper->GetAlgorithmList( aLcl );

    if( !pIndexRes )
        pIndexRes = new IndexEntryResource();

    String sOldString;
    void* pUserData;
    if( 0 != (pUserData = aSortAlgorithmLB.GetEntryData( aSortAlgorithmLB.GetSelectEntryPos())) )
        sOldString = *(String*)pUserData;
    void* pDel;
    sal_uInt16 nEnd = aSortAlgorithmLB.GetEntryCount();
    for( sal_uInt16 n = 0; n < nEnd; ++n )
        if( 0 != ( pDel = aSortAlgorithmLB.GetEntryData( n )) )
            delete (String*)pDel;
    aSortAlgorithmLB.Clear();

    sal_uInt16 nInsPos;
    String sAlg, sUINm;
    nEnd = static_cast< sal_uInt16 >(aSeq.getLength());
    for( sal_uInt16 nCnt = 0; nCnt < nEnd; ++nCnt )
    {
        sUINm = pIndexRes->GetTranslation( sAlg = aSeq[ nCnt ] );
        nInsPos = aSortAlgorithmLB.InsertEntry( sUINm );
        aSortAlgorithmLB.SetEntryData( nInsPos, new String( sAlg ));
        if( sAlg == sOldString )
            aSortAlgorithmLB.SelectEntryPos( nInsPos );
    }

    if( LISTBOX_ENTRY_NOTFOUND == aSortAlgorithmLB.GetSelectEntryPos() )
        aSortAlgorithmLB.SelectEntryPos( 0 );

    if(pBox)
        ModifyHdl(0);
    return 0;
};

IMPL_LINK(SwTOXSelectTabPage, AddStylesHdl, PushButton*, pButton)
{
    SwAddStylesDlg_Impl* pDlg = new SwAddStylesDlg_Impl(pButton,
        ((SwMultiTOXTabDialog*)GetTabDialog())->GetWrtShell(),
        aStyleArr);
    pDlg->Execute();
    delete pDlg;
    ModifyHdl(0);
    return 0;
}

IMPL_LINK(SwTOXSelectTabPage, MenuEnableHdl, Menu*, pMenu)
{
    pMenu->EnableItem(MN_AUTOMARK_EDIT, sAutoMarkURL.Len() > 0);
    return 0;
}

IMPL_LINK(SwTOXSelectTabPage, MenuExecuteHdl, Menu*, pMenu)
{
    const String sSaveAutoMarkURL = sAutoMarkURL;
    switch(pMenu->GetCurItemId())
    {
        case  MN_AUTOMARK_OPEN:
            sAutoMarkURL = lcl_CreateAutoMarkFileDlg(
                                    sAutoMarkURL, sAutoMarkType, true);
        break;
        case  MN_AUTOMARK_NEW :
            sAutoMarkURL = lcl_CreateAutoMarkFileDlg(
                                    sAutoMarkURL, sAutoMarkType, false);
            if( !sAutoMarkURL.Len() )
                break;
        //no break
        case  MN_AUTOMARK_EDIT:
        {
            bool bNew = pMenu->GetCurItemId()== MN_AUTOMARK_NEW;
            SwAutoMarkDlg_Impl* pAutoMarkDlg = new SwAutoMarkDlg_Impl(
                    &aAutoMarkPB, sAutoMarkURL, sAutoMarkType, bNew );

            if( RET_OK != pAutoMarkDlg->Execute() && bNew )
                sAutoMarkURL = sSaveAutoMarkURL;
            delete pAutoMarkDlg;

        }
        break;
    }
    return 0;
}

class SwTOXEdit : public Edit
{
    SwFormToken aFormToken;
    Link        aPrevNextControlLink;
    bool     bNextControl;
    SwTokenWindow* m_pParent;
public:
    SwTOXEdit( Window* pParent, SwTokenWindow* pTokenWin,
                const SwFormToken& aToken)
        : Edit( pParent, WB_BORDER|WB_TABSTOP|WB_CENTER),
        aFormToken(aToken),
        bNextControl(false),
        m_pParent( pTokenWin )
    {
    }

    virtual void    KeyInput( const KeyEvent& rKEvt );
    virtual void    RequestHelp( const HelpEvent& rHEvt );

    bool    IsNextControl() const {return bNextControl;}
    void SetPrevNextLink( const Link& rLink )   {aPrevNextControlLink = rLink;}

    const SwFormToken&  GetFormToken()
        {
            aFormToken.sText = GetText();
            return aFormToken;
        }

    void    SetCharStyleName(const String& rSet, sal_uInt16 nPoolId)
        {
            aFormToken.sCharStyleName = rSet;
            aFormToken.nPoolId = nPoolId;
        }

    void    AdjustSize();
};

void    SwTOXEdit::RequestHelp( const HelpEvent& rHEvt )
{
    if(!m_pParent->CreateQuickHelp(this, aFormToken, rHEvt))
        Edit::RequestHelp(rHEvt);
}

void    SwTOXEdit::KeyInput( const KeyEvent& rKEvt )
{
    const Selection& rSel = GetSelection();
    sal_Int32 nTextLen = GetText().getLength();
    if((rSel.A() == rSel.B() &&
        !rSel.A()) || rSel.A() == (sal_uInt16)nTextLen )
    {
        bool bCall = false;
        KeyCode aCode = rKEvt.GetKeyCode();
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

        if(bCall && aPrevNextControlLink.IsSet())
            aPrevNextControlLink.Call(this);

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
    SwFormToken aFormToken;
    Link        aPrevNextControlLink;
    bool        bNextControl;
    SwTokenWindow* m_pParent;
public:
    SwTOXButton( Window* pParent, SwTokenWindow* pTokenWin,
                const SwFormToken& rToken)
        : PushButton(pParent, WB_BORDER|WB_TABSTOP),
        aFormToken(rToken),
        bNextControl(false),
        m_pParent(pTokenWin)
    {
    }

    virtual void    KeyInput( const KeyEvent& rKEvt );
    virtual void    RequestHelp( const HelpEvent& rHEvt );

    bool IsNextControl() const          {return bNextControl;}
    void SetPrevNextLink(const Link& rLink) {aPrevNextControlLink = rLink;}
    const SwFormToken& GetFormToken() const {return aFormToken;}

    void SetCharStyleName(const String& rSet, sal_uInt16 nPoolId)
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
    sal_uInt16 GetChapterInfo() const{ return aFormToken.nChapterFormat;}

    void SetOutlineLevel( sal_uInt16 nSet ) { aFormToken.nOutlineLevel = nSet;}//i53420
    sal_uInt16 GetOutlineLevel() const{ return aFormToken.nOutlineLevel;}

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

void    SwTOXButton::KeyInput( const KeyEvent& rKEvt )
{
    bool bCall = false;
    KeyCode aCode = rKEvt.GetKeyCode();
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
        m_pParent->RemoveControl(this, sal_True);
        //this is invalid here
        return;
    }
    if(bCall && aPrevNextControlLink.IsSet())
            aPrevNextControlLink.Call(this);
    else
        PushButton::KeyInput(rKEvt);
}

void    SwTOXButton::RequestHelp( const HelpEvent& rHEvt )
{
    if(!m_pParent->CreateQuickHelp(this, aFormToken, rHEvt))
        Button::RequestHelp(rHEvt);
}

SwIdxTreeListBox::SwIdxTreeListBox(Window* pPar, WinBits nStyle)
    : SvTreeListBox(pPar, nStyle)
    , pParent(NULL)
{
}

extern "C" SAL_DLLPUBLIC_EXPORT Window* SAL_CALL makeSwIdxTreeListBox(Window *pParent, VclBuilder::stringmap &rMap)
{
    WinBits nWinStyle = WB_TABSTOP;
    OString sBorder = VclBuilder::extractCustomProperty(rMap);
    if (!sBorder.isEmpty())
        nWinStyle |= WB_BORDER;
    return new SwIdxTreeListBox(pParent, nWinStyle);
}

void    SwIdxTreeListBox::RequestHelp( const HelpEvent& rHEvt )
{
    if( rHEvt.GetMode() & HELPMODE_QUICK )
    {
     Point aPos( ScreenToOutputPixel( rHEvt.GetMousePosPixel() ));
        SvTreeListEntry* pEntry = GetEntry( aPos );
        if( pEntry )
        {
            sal_uInt16 nLevel = static_cast< sal_uInt16 >(GetModel()->GetAbsPos(pEntry));
            String sEntry = pParent->GetLevelHelp(++nLevel);
            if (comphelper::string::equals(sEntry, '*'))
                sEntry = GetEntryText(pEntry);
            if(sEntry.Len())
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
                            QUICKHELP_LEFT|QUICKHELP_VCENTER );
                }
            }
        }
    }
    else
        SvTreeListBox::RequestHelp(rHEvt);
}

SwTOXEntryTabPage::SwTOXEntryTabPage(Window* pParent, const SfxItemSet& rAttrSet)
    : SfxTabPage(pParent, "TocEntriesPage",
        "modules/swriter/ui/tocentriespage.ui", rAttrSet)
    , sDelimStr(SW_RESSTR(STR_DELIM))
    , sNoCharStyle(SW_RESSTR(STR_NO_CHAR_STYLE))
    , sNoCharSortKey(SW_RESSTR(STR_NOSORTKEY))
    , m_pCurrentForm(0)
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

    m_pAlphaDelimCB->SetClickHdl(LINK(this, SwTOXEntryTabPage, ModifyHdl));
    m_pCommaSeparatedCB->SetClickHdl(LINK(this, SwTOXEntryTabPage, ModifyHdl));
    m_pRelToStyleCB->SetClickHdl(LINK(this, SwTOXEntryTabPage, ModifyHdl));

    FieldUnit aMetric = ::GetDfltMetric(sal_False);
    SetMetric(*m_pTabPosMF, aMetric);

    m_pSortDocPosRB->Check();

    m_pFillCharCB->SetMaxTextLen(1);
    m_pFillCharCB->InsertEntry(OUString(' '));
    m_pFillCharCB->InsertEntry(OUString('.'));
    m_pFillCharCB->InsertEntry(OUString('-'));
    m_pFillCharCB->InsertEntry(OUString('_'));

    m_pEditStylePB->Enable(sal_False);

    //fill the types in
    for (sal_uInt16 i = 0; i < AUTH_FIELD_END; ++i)
    {
        String sTmp(SW_RES(STR_AUTH_FIELD_START + i));
        sal_uInt16 nPos = m_pAuthFieldsLB->InsertEntry(sTmp);
        m_pAuthFieldsLB->SetEntryData(nPos, reinterpret_cast< void * >(sal::static_int_cast< sal_uIntPtr >(i)));
    }
    sal_uInt16 nPos = m_pFirstKeyLB->InsertEntry(sNoCharSortKey);
    m_pFirstKeyLB->SetEntryData(nPos, reinterpret_cast< void * >(sal::static_int_cast< sal_uIntPtr >(USHRT_MAX)));
    nPos = m_pSecondKeyLB->InsertEntry(sNoCharSortKey);
    m_pSecondKeyLB->SetEntryData(nPos, reinterpret_cast< void * >(sal::static_int_cast< sal_uIntPtr >(USHRT_MAX)));
    nPos = m_pThirdKeyLB->InsertEntry(sNoCharSortKey);
    m_pThirdKeyLB->SetEntryData(nPos, reinterpret_cast< void * >(sal::static_int_cast< sal_uIntPtr >(USHRT_MAX)));

    for (sal_uInt16 i = 0; i < AUTH_FIELD_END; ++i)
    {
        String sTmp(m_pAuthFieldsLB->GetEntry(i));
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
/* --------------------------------------------------
    pVoid is used as signal to change all levels of the example
 --------------------------------------------------*/
IMPL_LINK(SwTOXEntryTabPage, ModifyHdl, void*, pVoid)
{
    UpdateDescriptor();
    SwMultiTOXTabDialog* pTOXDlg = (SwMultiTOXTabDialog*)GetTabDialog();

    if(pTOXDlg)
    {
        sal_uInt16 nCurLevel = static_cast< sal_uInt16 >(m_pLevelLB->GetModel()->GetAbsPos(m_pLevelLB->FirstSelected()) + 1);
        if(aLastTOXType.eType == TOX_CONTENT && pVoid)
            nCurLevel = USHRT_MAX;
        pTOXDlg->CreateOrUpdateExample(
            pTOXDlg->GetCurrentTOXType().eType, TOX_PAGE_ENTRY, nCurLevel);
    }
    return 0;
}

SwTOXEntryTabPage::~SwTOXEntryTabPage()
{
}

sal_Bool    SwTOXEntryTabPage::FillItemSet( SfxItemSet& )
{
    // nothing to do
    return sal_True;
}

void SwTOXEntryTabPage::Reset( const SfxItemSet& )
{
    SwMultiTOXTabDialog* pTOXDlg = (SwMultiTOXTabDialog*)GetTabDialog();
    const CurTOXType aCurType = pTOXDlg->GetCurrentTOXType();
    m_pCurrentForm = pTOXDlg->GetForm(aCurType);
    if(TOX_INDEX == aCurType.eType)
    {
        SwTOXDescription& rDesc = pTOXDlg->GetTOXDescription(aCurType);
        String sMainEntryCharStyle = rDesc.GetMainEntryCharStyle();
        if(sMainEntryCharStyle.Len())
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
    SwMultiTOXTabDialog* pTOXDlg = (SwMultiTOXTabDialog*)GetTabDialog();
    const CurTOXType aCurType = pTOXDlg->GetCurrentTOXType();

    m_pCurrentForm = pTOXDlg->GetForm(aCurType);
    if( !( aLastTOXType == aCurType ))
    {
        sal_Bool bToxIsAuthorities = TOX_AUTHORITIES == aCurType.eType;
        sal_Bool bToxIsIndex =       TOX_INDEX == aCurType.eType;
        sal_Bool bToxIsContent =     TOX_CONTENT == aCurType.eType;
        sal_Bool bToxIsSequence =    TOX_ILLUSTRATIONS == aCurType.eType;

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
            //
            SwWrtShell& rSh = pTOXDlg->GetWrtShell();
            const SwAuthorityFieldType* pFType = (const SwAuthorityFieldType*)
                                    rSh.GetFldType(RES_AUTHORITY, aEmptyStr);
            if(pFType)
            {
                if(pFType->IsSortByDocument())
                    m_pSortDocPosRB->Check();
                else
                {
                    m_pSortContentRB->Check();
                    sal_uInt16 nKeyCount = pFType->GetSortKeyCount();
                    if(0 < nKeyCount)
                    {
                        const SwTOXSortKey* pKey = pFType->GetSortKey(0);
                        m_pFirstKeyLB->SelectEntryPos(
                            m_pFirstKeyLB->GetEntryPos((void*)(sal_uIntPtr)pKey->eField));
                        m_pFirstSortUpRB->Check(pKey->bSortAscending);
                        m_pFirstSortDownRB->Check(!pKey->bSortAscending);
                    }
                    if(1 < nKeyCount)
                    {
                        const SwTOXSortKey* pKey = pFType->GetSortKey(1);
                        m_pSecondKeyLB->SelectEntryPos(
                            m_pSecondKeyLB->GetEntryPos((void*)(sal_uIntPtr)pKey->eField));
                        m_pSecondSortUpRB->Check(pKey->bSortAscending);
                        m_pSecondSortDownRB->Check(!pKey->bSortAscending);
                    }
                    if(2 < nKeyCount)
                    {
                        const SwTOXSortKey* pKey = pFType->GetSortKey(2);
                        m_pThirdKeyLB->SelectEntryPos(
                            m_pThirdKeyLB->GetEntryPos((void*)(sal_uIntPtr)pKey->eField));
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

        Link aLink = m_pLevelLB->GetSelectHdl();
        m_pLevelLB->SetSelectHdl(Link());
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
    SwMultiTOXTabDialog* pTOXDlg = (SwMultiTOXTabDialog*)GetTabDialog();
    SwTOXDescription& rDesc = pTOXDlg->GetTOXDescription(aLastTOXType);
    if(TOX_INDEX == aLastTOXType.eType)
    {
        String sTemp(m_pMainEntryStyleLB->GetSelectEntry());
        rDesc.SetMainEntryCharStyle(sNoCharStyle == sTemp ? aEmptyStr : sTemp);
        sal_uInt16 nIdxOptions = rDesc.GetIndexOptions() & ~nsSwTOIOptions::TOI_ALPHA_DELIMITTER;
        if(m_pAlphaDelimCB->IsChecked())
            nIdxOptions |= nsSwTOIOptions::TOI_ALPHA_DELIMITTER;
        rDesc.SetIndexOptions(nIdxOptions);
    }
    else if(TOX_AUTHORITIES == aLastTOXType.eType)
    {
        rDesc.SetSortByDocument(m_pSortDocPosRB->IsChecked());
        SwTOXSortKey aKey1, aKey2, aKey3;
        aKey1.eField = (ToxAuthorityField)(sal_uIntPtr)m_pFirstKeyLB->GetEntryData(
                                    m_pFirstKeyLB->GetSelectEntryPos());
        aKey1.bSortAscending = m_pFirstSortUpRB->IsChecked();
        aKey2.eField = (ToxAuthorityField)(sal_uIntPtr)m_pSecondKeyLB->GetEntryData(
                                    m_pSecondKeyLB->GetSelectEntryPos());
        aKey2.bSortAscending = m_pSecondSortUpRB->IsChecked();
        aKey3.eField = (ToxAuthorityField)(sal_uIntPtr)m_pThirdKeyLB->GetEntryData(
                                m_pThirdKeyLB->GetSelectEntryPos());
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

int SwTOXEntryTabPage::DeactivatePage( SfxItemSet* /*pSet*/)
{
    UpdateDescriptor();
    return LEAVE_PAGE;
}

SfxTabPage* SwTOXEntryTabPage::Create( Window* pParent,     const SfxItemSet& rAttrSet)
{
    return new SwTOXEntryTabPage(pParent, rAttrSet);
}

IMPL_LINK(SwTOXEntryTabPage, EditStyleHdl, PushButton*, pBtn)
{
    if( LISTBOX_ENTRY_NOTFOUND != m_pCharStyleLB->GetSelectEntryPos())
    {
        SfxStringItem aStyle(SID_STYLE_EDIT, m_pCharStyleLB->GetSelectEntry());
        SfxUInt16Item aFamily(SID_STYLE_FAMILY, SFX_STYLE_FAMILY_CHAR);
        // TODO: WrtShell?
//      SwPtrItem aShell(FN_PARAM_WRTSHELL, pWrtShell);
        Window* pDefDlgParent = Application::GetDefDialogParent();
        Application::SetDefDialogParent( pBtn );
        ((SwMultiTOXTabDialog*)GetTabDialog())->GetWrtShell().
        GetView().GetViewFrame()->GetDispatcher()->Execute(
        SID_STYLE_EDIT, SFX_CALLMODE_SYNCHRON|SFX_CALLMODE_MODAL,
            &aStyle, &aFamily/*, &aShell*/, 0L);
        Application::SetDefDialogParent( pDefDlgParent );
    }
    return 0;
}

IMPL_LINK(SwTOXEntryTabPage, RemoveInsertAuthHdl, PushButton*, pButton)
{
    bool bInsert = pButton == m_pAuthInsertPB;
    if(bInsert)
    {
        sal_uInt16 nSelPos = m_pAuthFieldsLB->GetSelectEntryPos();
        String sToInsert(m_pAuthFieldsLB->GetSelectEntry());
        SwFormToken aInsert(TOKEN_AUTHORITY);
        aInsert.nAuthorityField = (sal_uInt16)(sal_uIntPtr)m_pAuthFieldsLB->GetEntryData(nSelPos);
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
            const SwFormToken& rToken = ((SwTOXButton*)pCtrl)->GetFormToken();
            PreTokenButtonRemoved(rToken);
            m_pTokenWIN->RemoveControl((SwTOXButton*)pCtrl);
        }
    }
    ModifyHdl(0);
    return 0;
}

void SwTOXEntryTabPage::PreTokenButtonRemoved(const SwFormToken& rToken)
{
    //fill it into the ListBox
    sal_uInt32 nData = rToken.nAuthorityField;
    String sTemp(SW_RES(STR_AUTH_FIELD_START + nData));
    sal_uInt16 nPos = m_pAuthFieldsLB->InsertEntry(sTemp);
    m_pAuthFieldsLB->SetEntryData(nPos, (void*)(sal_uIntPtr)(nData));
}
/*-----------------------------------------------------------------------

This function inizializes the default value in the Token
put here the UI dependent initializations
  -----------------------------------------------------------------------*/
IMPL_LINK(SwTOXEntryTabPage, InsertTokenHdl, PushButton*, pBtn)
{
    String sText;
    FormTokenType eTokenType = TOKEN_ENTRY_NO;
    String sCharStyle;
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
            sText = SwForm::GetFormEntryTxt();
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
        sCharStyle = String(SW_RES(STR_POOLCHR_TOXJUMP));
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
    ModifyHdl(0);
    return 0;
}

IMPL_LINK_NOARG(SwTOXEntryTabPage, AllLevelsHdl)
{
    //get current level
    //write it into all levels
    if(m_pTokenWIN->IsValid())
    {
        String sNewToken = m_pTokenWIN->GetPattern();
        for(sal_uInt16 i = 1; i < m_pCurrentForm->GetFormMax(); i++)
            m_pCurrentForm->SetPattern(i, sNewToken);
        //
        ModifyHdl(this);
    }
    return 0;
}

void SwTOXEntryTabPage::WriteBackLevel()
{
    if(m_pTokenWIN->IsValid())
    {
        String sNewToken = m_pTokenWIN->GetPattern();
        sal_uInt16 nLastLevel = m_pTokenWIN->GetLastLevel();
        if(nLastLevel != USHRT_MAX)
            m_pCurrentForm->SetPattern(nLastLevel + 1, sNewToken );
    }
}

IMPL_LINK(SwTOXEntryTabPage, LevelHdl, SvTreeListBox*, pBox)
{
    if(bInLevelHdl)
        return 0;
    bInLevelHdl = sal_True;
    WriteBackLevel();

    sal_uInt16 nLevel = static_cast< sal_uInt16 >(pBox->GetModel()->GetAbsPos(pBox->FirstSelected()));
    m_pTokenWIN->SetForm(*m_pCurrentForm, nLevel);
    if(TOX_AUTHORITIES == m_pCurrentForm->GetTOXType())
    {
        //fill the types in
        m_pAuthFieldsLB->Clear();
        for( sal_uInt32 i = 0; i < AUTH_FIELD_END; i++)
        {
            String sTmp(SW_RES(STR_AUTH_FIELD_START + i));
            sal_uInt16 nPos = m_pAuthFieldsLB->InsertEntry(sTmp);
            m_pAuthFieldsLB->SetEntryData(nPos, (void*)(sal_uIntPtr)(i));
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
                sal_uInt16  nLstBoxPos = m_pAuthFieldsLB->GetEntryPos( (void*)(sal_uIntPtr)nSearch );
                OSL_ENSURE(LISTBOX_ENTRY_NOTFOUND != nLstBoxPos, "Entry not found?");
                m_pAuthFieldsLB->RemoveEntry(nLstBoxPos);
            }

            aIt++; // #i21237#
        }
        m_pAuthFieldsLB->SelectEntryPos(0);
    }
    bInLevelHdl = sal_False;
    pBox->GrabFocus();
    return 0;
}

IMPL_LINK(SwTOXEntryTabPage, SortKeyHdl, RadioButton*, pButton)
{
    bool bEnable = m_pSortContentRB == pButton;
    m_pSortKeyFrame->Enable(bEnable);
    return 0;
}

IMPL_LINK(SwTOXEntryTabPage, TokenSelectedHdl, SwFormToken*, pToken)
{
    if (!pToken->sCharStyleName.isEmpty())
        m_pCharStyleLB->SelectEntry(pToken->sCharStyleName);
    else
        m_pCharStyleLB->SelectEntry(sNoCharStyle);

    String sEntry = m_pCharStyleLB->GetSelectEntry();
    m_pEditStylePB->Enable(sEntry != sNoCharStyle);

    if(pToken->eTokenType == TOKEN_CHAPTER_INFO)
    {
//---> i89791
        switch(pToken->nChapterFormat)
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

        m_pEntryOutlineLevelNF->SetValue(pToken->nOutlineLevel);
    }

//i53420
    if(pToken->eTokenType == TOKEN_ENTRY_NO)
    {
        m_pEntryOutlineLevelNF->SetValue(pToken->nOutlineLevel);
        sal_uInt16 nFormat = 0;
        if( pToken->nChapterFormat == CF_NUM_NOPREPST_TITLE )
            nFormat = 1;
        m_pNumberFormatLB->SelectEntryPos(nFormat);
    }

    sal_Bool bTabStop = TOKEN_TAB_STOP == pToken->eTokenType;
    m_pFillCharFT->Show(bTabStop);
    m_pFillCharCB->Show(bTabStop);
    m_pTabPosFT->Show(bTabStop);
    m_pTabPosMF->Show(bTabStop);
    m_pAutoRightCB->Show(bTabStop);
    m_pAutoRightCB->Enable(bTabStop);
    if(bTabStop)
    {
        m_pTabPosMF->SetValue(m_pTabPosMF->Normalize(pToken->nTabStopPosition), FUNIT_TWIP);
        m_pAutoRightCB->Check(SVX_TAB_ADJUST_END == pToken->eTabAlign);
        m_pFillCharCB->SetText(OUString(pToken->cTabFillChar));
        m_pTabPosFT->Enable(!m_pAutoRightCB->IsChecked());
        m_pTabPosMF->Enable(!m_pAutoRightCB->IsChecked());
    }
    else
    {
        m_pTabPosMF->Enable(sal_False);
    }

    sal_Bool bIsChapterInfo = pToken->eTokenType == TOKEN_CHAPTER_INFO;
    sal_Bool bIsEntryNumber = pToken->eTokenType == TOKEN_ENTRY_NO;
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
        m_pEntryNoPB->Enable(TOKEN_ENTRY_NO != pToken->eTokenType );
    }
    if(m_pEntryPB->IsVisible())
    {
        m_pEntryPB->Enable(TOKEN_ENTRY_TEXT != pToken->eTokenType &&
                                !m_pTokenWIN->Contains(TOKEN_ENTRY_TEXT)
                                && !m_pTokenWIN->Contains(TOKEN_ENTRY));
    }

    if(m_pChapterInfoPB->IsVisible())
    {
        m_pChapterInfoPB->Enable(TOKEN_CHAPTER_INFO != pToken->eTokenType);
    }
    if(m_pPageNoPB->IsVisible())
    {
        m_pPageNoPB->Enable(TOKEN_PAGE_NUMS != pToken->eTokenType &&
                                !m_pTokenWIN->Contains(TOKEN_PAGE_NUMS));
    }
    if(m_pTabPB->IsVisible())
    {
        m_pTabPB->Enable(!bTabStop);
    }
    if(m_pHyperLinkPB->IsVisible())
    {
        m_pHyperLinkPB->Enable(TOKEN_LINK_START != pToken->eTokenType &&
                            TOKEN_LINK_END != pToken->eTokenType);
    }
    //table of authorities
    if(m_pAuthInsertPB->IsVisible())
    {
        bool bText = TOKEN_TEXT == pToken->eTokenType;
        m_pAuthInsertPB->Enable(bText && !m_pAuthFieldsLB->GetSelectEntry().isEmpty());
        m_pAuthRemovePB->Enable(!bText);
    }

    return 0;
}

IMPL_LINK(SwTOXEntryTabPage, StyleSelectHdl, ListBox*, pBox)
{
    String sEntry = pBox->GetSelectEntry();
    sal_uInt16 nId = (sal_uInt16)(sal_IntPtr)pBox->GetEntryData(pBox->GetSelectEntryPos());
    m_pEditStylePB->Enable(sEntry != sNoCharStyle);
    if(sEntry == sNoCharStyle)
        sEntry.Erase();
    Control* pCtrl = m_pTokenWIN->GetActiveControl();
    OSL_ENSURE(pCtrl, "no active control?");
    if(pCtrl)
    {
        if(WINDOW_EDIT == pCtrl->GetType())
            ((SwTOXEdit*)pCtrl)->SetCharStyleName(sEntry, nId);
        else
            ((SwTOXButton*)pCtrl)->SetCharStyleName(sEntry, nId);

    }
    ModifyHdl(0);
    return 0;
}

IMPL_LINK(SwTOXEntryTabPage, ChapterInfoHdl, ListBox*, pBox)
{
    sal_uInt16 nPos = pBox->GetSelectEntryPos();
    if(LISTBOX_ENTRY_NOTFOUND != nPos)
    {
        Control* pCtrl = m_pTokenWIN->GetActiveControl();
        OSL_ENSURE(pCtrl, "no active control?");
        if(pCtrl && WINDOW_EDIT != pCtrl->GetType())
            ((SwTOXButton*)pCtrl)->SetChapterInfo(nPos);

        ModifyHdl(0);
    }
    return 0;
}

IMPL_LINK(SwTOXEntryTabPage, ChapterInfoOutlineHdl, NumericField*, pField)
{
    const sal_uInt16 nLevel = static_cast<sal_uInt8>(pField->GetValue());

    Control* pCtrl = m_pTokenWIN->GetActiveControl();
    OSL_ENSURE(pCtrl, "no active control?");
    if(pCtrl && WINDOW_EDIT != pCtrl->GetType())
        ((SwTOXButton*)pCtrl)->SetOutlineLevel(nLevel);

    ModifyHdl(0);
    return 0;
}

IMPL_LINK(SwTOXEntryTabPage, NumberFormatHdl, ListBox*, pBox)
{
    const sal_uInt16 nPos = pBox->GetSelectEntryPos();

    if(LISTBOX_ENTRY_NOTFOUND != nPos)
    {
        Control* pCtrl = m_pTokenWIN->GetActiveControl();
        OSL_ENSURE(pCtrl, "no active control?");
        if(pCtrl && WINDOW_EDIT != pCtrl->GetType())
        {
           ((SwTOXButton*)pCtrl)->SetEntryNumberFormat(nPos);//i89791
        }
        ModifyHdl(0);
    }
    return 0;
}

IMPL_LINK(SwTOXEntryTabPage, TabPosHdl, MetricField*, pField)
{
    Control* pCtrl = m_pTokenWIN->GetActiveControl();
    OSL_ENSURE(pCtrl && WINDOW_EDIT != pCtrl->GetType() &&
        TOKEN_TAB_STOP == ((SwTOXButton*)pCtrl)->GetFormToken().eTokenType,
                "no active style::TabStop control?");
    if( pCtrl && WINDOW_EDIT != pCtrl->GetType() )
    {
        ((SwTOXButton*)pCtrl)->SetTabPosition( static_cast< SwTwips >(
                pField->Denormalize( pField->GetValue( FUNIT_TWIP ))));
    }
    ModifyHdl(0);
    return 0;
}

IMPL_LINK(SwTOXEntryTabPage, FillCharHdl, ComboBox*, pBox)
{
    Control* pCtrl = m_pTokenWIN->GetActiveControl();
    OSL_ENSURE(pCtrl && WINDOW_EDIT != pCtrl->GetType() &&
        TOKEN_TAB_STOP == ((SwTOXButton*)pCtrl)->GetFormToken().eTokenType,
                "no active style::TabStop control?");
    if(pCtrl && WINDOW_EDIT != pCtrl->GetType())
    {
        sal_Unicode cSet;
        if( !pBox->GetText().isEmpty() )
            cSet = pBox->GetText()[0];
        else
            cSet = ' ';
        ((SwTOXButton*)pCtrl)->SetFillChar( cSet );
    }
    ModifyHdl(0);
    return 0;
}

IMPL_LINK(SwTOXEntryTabPage, AutoRightHdl, CheckBox*, pBox)
{
    //the most right style::TabStop is usually right aligned
    Control* pCurCtrl = m_pTokenWIN->GetActiveControl();
    OSL_ENSURE(WINDOW_EDIT != pCurCtrl->GetType() &&
            ((SwTOXButton*)pCurCtrl)->GetFormToken().eTokenType == TOKEN_TAB_STOP,
            "no style::TabStop selected!");

    const SwFormToken& rToken = ((SwTOXButton*)pCurCtrl)->GetFormToken();
    sal_Bool bChecked = pBox->IsChecked();
    if(rToken.eTokenType == TOKEN_TAB_STOP)
        ((SwTOXButton*)pCurCtrl)->SetTabAlign(
            bChecked ? SVX_TAB_ADJUST_END : SVX_TAB_ADJUST_LEFT);
    m_pTabPosFT->Enable(!bChecked);
    m_pTabPosMF->Enable(!bChecked);
    ModifyHdl(0);
    return 0;
}

void SwTOXEntryTabPage::SetWrtShell(SwWrtShell& rSh)
{
    SwDocShell* pDocSh = rSh.GetView().GetDocShell();
    ::FillCharStyleListBox(*m_pCharStyleLB, pDocSh, true, true);
    const String sDefault(SW_RES(STR_POOLCOLL_STANDARD));
    for(sal_uInt16 i = 0; i < m_pCharStyleLB->GetEntryCount(); i++)
    {
        String sEntry = m_pCharStyleLB->GetEntry(i);
        if(sDefault != sEntry)
        {
            m_pMainEntryStyleLB->InsertEntry( sEntry );
            m_pMainEntryStyleLB->SetEntryData(i, m_pCharStyleLB->GetEntryData(i));
        }
    }
    m_pMainEntryStyleLB->SelectEntry( SwStyleNameMapper::GetUIName(
                                RES_POOLCHR_IDX_MAIN_ENTRY, aEmptyStr ));
}

String  SwTOXEntryTabPage::GetLevelHelp(sal_uInt16 nLevel) const
{
    OUString sRet;
    SwMultiTOXTabDialog* pTOXDlg = (SwMultiTOXTabDialog*)GetTabDialog();
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

SwTokenWindow::SwTokenWindow(Window* pParent)
    : VclHBox(pParent)
    , pForm(0)
    , nLevel(0)
    , bValid(false)
    , sCharStyle(SW_RESSTR(STR_CHARSTYLE))
    , pActiveCtrl(0)
    , m_pParent(NULL)
{
    m_pUIBuilder = new VclBuilder(this, getUIRootDir(),
        "modules/swriter/ui/tokenwidget.ui", "TokenWidget");
    get(m_pLeftScrollWin, "left");
    get(m_pCtrlParentWin, "ctrl");
    m_pCtrlParentWin->set_height_request(Edit::GetMinimumEditSize().Height());
    get(m_pRightScrollWin, "right");

    for (sal_uInt16 i = 0; i < TOKEN_END; ++i)
    {
        sal_uInt16 nTextId = STR_BUTTON_TEXT_START + i;
        if( STR_TOKEN_ENTRY_TEXT == nTextId )
            nTextId = STR_TOKEN_ENTRY;
        aButtonTexts[i] = SW_RESSTR(nTextId);

        sal_uInt16 nHelpId = STR_BUTTON_HELP_TEXT_START + i;
        if(STR_TOKEN_HELP_ENTRY_TEXT == nHelpId)
            nHelpId = STR_TOKEN_HELP_ENTRY;
        aButtonHelpTexts[i] = SW_RESSTR(nHelpId);
    }

    Link aLink(LINK(this, SwTokenWindow, ScrollHdl));
    m_pLeftScrollWin->SetClickHdl(aLink);
    m_pRightScrollWin->SetClickHdl(aLink);
}

extern "C" SAL_DLLPUBLIC_EXPORT Window* SAL_CALL makeSwTokenWindow(Window *pParent, VclBuilder::stringmap &)
{
    return new SwTokenWindow(pParent);
}

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
    for (ctrl_iterator it = aControlList.begin(); it != aControlList.end(); ++it)
    {
        Control* pControl = (*it);
        pControl->SetGetFocusHdl( Link() );
        pControl->SetLoseFocusHdl( Link() );
        delete pControl;
    }
}

void    SwTokenWindow::SetForm(SwForm& rForm, sal_uInt16 nL)
{
    SetActiveControl(0);
    bValid = sal_True;

    if(pForm)
    {
        //apply current level settings to the form
        for (ctrl_iterator iter = aControlList.begin(); iter != aControlList.end(); ++iter)
            delete (*iter);

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

        Control* pSetActiveControl = 0;
        while(aIt != aPattern.end()) // #i21237#
        {
            SwFormToken aToken(*aIt); // #i21237#

            if(TOKEN_TEXT == aToken.eTokenType)
            {
                OSL_ENSURE(!bLastWasText, "text following text is invalid");
                Control* pCtrl = InsertItem(aToken.sText, aToken);
                bLastWasText = true;
                if(!GetActiveControl())
                    SetActiveControl(pCtrl);
            }
            else
            {
                if( !bLastWasText )
                {
                    bLastWasText = true;
                    SwFormToken aTemp(TOKEN_TEXT);
                    Control* pCtrl = InsertItem(aEmptyStr, aTemp);
                    if(!pSetActiveControl)
                        pSetActiveControl = pCtrl;
                }

                OUString sForm;
                switch( aToken.eTokenType )
                {
                case TOKEN_ENTRY_NO:     sForm = SwForm::GetFormEntryNum(); break;
                case TOKEN_ENTRY_TEXT:   sForm = SwForm::GetFormEntryTxt(); break;
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
            bLastWasText = true;
            SwFormToken aTemp(TOKEN_TEXT);
            Control* pCtrl = InsertItem(aEmptyStr, aTemp);
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
                pFToken = &((SwTOXEdit*)pActiveCtrl)->GetFormToken();
            else
                pFToken = &((SwTOXButton*)pActiveCtrl)->GetFormToken();

            SwFormToken aTemp( *pFToken );
            aButtonSelectedHdl.Call( &aTemp );
        }
    }
}

Control*    SwTokenWindow::InsertItem(const String& rText, const SwFormToken& rToken)
{
    Control* pRet = 0;
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
        SwTOXEdit* pEdit = new SwTOXEdit(m_pCtrlParentWin, this, rToken);
        pEdit->SetPosPixel(aControlPos);

        aControlList.push_back(pEdit);

        pEdit->SetText(rText);
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
        SwTOXButton* pButton = new SwTOXButton(m_pCtrlParentWin, this, rToken);
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

        pButton->Show();
        pRet = pButton;
    }

    return pRet;
}

void    SwTokenWindow::InsertAtSelection(
            const String& rText,
            const SwFormToken& rToken)
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

        const Control* pControl = 0;
        const Control* pExchange = 0;

        ctrl_const_iterator it = aControlList.begin();
        for( ; it != aControlList.end() && pActiveCtrl != (*it); ++it )
        {
            pControl = *it;

            if( WINDOW_EDIT != pControl->GetType())
            {
                const SwFormToken& rNewToken =
                                ((SwTOXButton*)pControl)->GetFormToken();

                if( TOKEN_LINK_START == rNewToken.eTokenType )
                {
                    bPreStartLinkFound = true;
                    pExchange = 0;
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
                                    ((SwTOXButton*)pControl)->GetFormToken();

                    if( TOKEN_LINK_START == rNewToken.eTokenType )
                    {
                        if(bPostLinkStartFound)
                            break;
                        bPostLinkStartFound = sal_True;
                        pExchange = pControl;
                    }
                    else if(TOKEN_LINK_END == rNewToken.eTokenType )
                    {
                        if(bPostLinkStartFound)
                        {
                            bPostLinkStartFound = sal_False;
                            pExchange = 0;
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
                ((SwTOXButton*)pExchange)->SetLinkEnd();
                ((SwTOXButton*)pExchange)->SetText(aButtonTexts[TOKEN_LINK_END]);
            }
        }

        if(bPreEndLinkFound)
        {
            OSL_ENSURE(pExchange, "no control to exchange?");

            if(pExchange)
            {
                ((SwTOXButton*)pExchange)->SetLinkStart();
                ((SwTOXButton*)pExchange)->SetText(aButtonTexts[TOKEN_LINK_START]);
            }
        }
    }

    //if the active control is text then insert a new button at the selection
    //else replace the button
    ctrl_iterator iterActive = std::find(aControlList.begin(),
                                         aControlList.end(), pActiveCtrl);

    Size aControlSize(GetOutputSizePixel());

    if( WINDOW_EDIT == pActiveCtrl->GetType())
    {
        ++iterActive;

        Selection aSel = ((SwTOXEdit*)pActiveCtrl)->GetSelection();
        aSel.Justify();

        String sEditText = ((SwTOXEdit*)pActiveCtrl)->GetText();
        String sLeft = sEditText.Copy( 0, static_cast< sal_uInt16 >(aSel.A()) );
        String sRight = sEditText.Copy( static_cast< sal_uInt16 >(aSel.B()),
                                        static_cast< sal_uInt16 >(sEditText.Len() - aSel.B()));

        ((SwTOXEdit*)pActiveCtrl)->SetText(sLeft);
        ((SwTOXEdit*)pActiveCtrl)->AdjustSize();

        SwFormToken aTmpToken(TOKEN_TEXT);
        SwTOXEdit* pEdit = new SwTOXEdit(m_pCtrlParentWin, this, aTmpToken);

        iterActive = aControlList.insert(iterActive, pEdit);

        pEdit->SetText(sRight);
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
        delete pActiveCtrl;
    }

    //now the new button
    SwTOXButton* pButton = new SwTOXButton(m_pCtrlParentWin, this, aToInsertToken);

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
        String sTmp(SwAuthorityFieldType::GetAuthFieldName(
                    (ToxAuthorityField)aToInsertToken.nAuthorityField));
        pButton->SetText(sTmp.Copy(0, 2));
    }

    Size aEditSize(GetOutputSizePixel());
    aEditSize.Width() = pButton->GetTextWidth(rText) + 5;
    pButton->SetSizePixel(aEditSize);
    pButton->Check(sal_True);
    pButton->Show();
    SetActiveControl(pButton);

    AdjustPositions();
}

void SwTokenWindow::RemoveControl(SwTOXButton* pDel, sal_Bool bInternalCall )
{
    if(bInternalCall && TOX_AUTHORITIES == pForm->GetTOXType())
        m_pParent->PreTokenButtonRemoved(pDel->GetFormToken());

    ctrl_iterator it = std::find(aControlList.begin(), aControlList.end(), pDel);

    OSL_ENSURE(it != aControlList.end(), "Control does not exist!");

    // the two neighbours of the box must be merged
    // the properties of the right one will be lost
    OSL_ENSURE(it != aControlList.begin() && it != aControlList.end() - 1,
        "Button at first or last position?");

    ctrl_iterator itLeft = it, itRight = it;
    --itLeft;
    ++itRight;
    Control *pLeftEdit = *itLeft;
    Control *pRightEdit = *itRight;

    String sTemp(((SwTOXEdit*)pLeftEdit)->GetText());
    sTemp += ((SwTOXEdit*)pRightEdit)->GetText();
    ((SwTOXEdit*)pLeftEdit)->SetText(sTemp);
    ((SwTOXEdit*)pLeftEdit)->AdjustSize();

    aControlList.erase(itRight);
    delete pRightEdit;

    aControlList.erase(it);
    pActiveCtrl->Hide();
    delete pActiveCtrl;

    SetActiveControl(pLeftEdit);
    AdjustPositions();
    if(aModifyHdl.IsSet())
        aModifyHdl.Call(0);
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
            if(pFirstCtrl)
            {
                //if the control fits into the space then the first control must be at postion 0
                long nFirstPos = pFirstCtrl->GetPosPixel().X();

                if(nFirstPos != 0)
                    MoveControls(-nFirstPos);
            }

            m_pRightScrollWin->Enable(false);
            m_pLeftScrollWin->Enable(false);
        }
    }
}

IMPL_LINK(SwTokenWindow, ScrollHdl, ImageButton*, pBtn )
{
    if(aControlList.empty())
        return 0;

    const long nSpace = m_pCtrlParentWin->GetSizePixel().Width();
#if OSL_DEBUG_LEVEL > 1
    //find all start/end positions and print it
    String sMessage(OUString("Space: "));
    sMessage += OUString::number(nSpace);
    sMessage += OUString(" | ");

    for (ctrl_const_iterator it = aControlList.begin(); it != aControlList.end(); ++it)
    {
        Control *pDebugCtrl = *it;

        long nDebugXPos = pDebugCtrl->GetPosPixel().X();
        long nDebugWidth = pDebugCtrl->GetSizePixel().Width();

        sMessage += OUString::number( nDebugXPos );
        sMessage += OUString(" ");
        sMessage += OUString::number(nDebugXPos + nDebugWidth);
        sMessage += OUString(" | ");
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

        Control *pCtrl = 0;

        pCtrl = *(aControlList.begin());
        m_pLeftScrollWin->Enable(pCtrl->GetPosPixel().X() < 0);

        pCtrl = *(aControlList.rbegin());
        m_pRightScrollWin->Enable((pCtrl->GetPosPixel().X() + pCtrl->GetSizePixel().Width()) > nSpace);
    }

    return 0;
}

String  SwTokenWindow::GetPattern() const
{
    String sRet;

    for (ctrl_const_iterator it = aControlList.begin(); it != aControlList.end(); ++it)
    {
        const Control *pCtrl = *it;

        const SwFormToken &rNewToken = pCtrl->GetType() == WINDOW_EDIT
                ? ((SwTOXEdit*)pCtrl)->GetFormToken()
                : ((SwTOXButton*)pCtrl)->GetFormToken();

        //TODO: prevent input of TOX_STYLE_DELIMITER in KeyInput
        sRet += rNewToken.GetString();
    }

    return sRet;
}
/* --------------------------------------------------
    Description: Check if a control of the specified
                    TokenType is already contained in the list
 --------------------------------------------------*/
sal_Bool SwTokenWindow::Contains(FormTokenType eSearchFor) const
{
    bool bRet = false;

    for (ctrl_const_iterator it = aControlList.begin(); it != aControlList.end(); ++it)
    {
        const Control *pCtrl = *it;

        const SwFormToken &rNewToken = pCtrl->GetType() == WINDOW_EDIT
                ? ((SwTOXEdit*)pCtrl)->GetFormToken()
                : ((SwTOXButton*)pCtrl)->GetFormToken();

        if (eSearchFor == rNewToken.eTokenType)
        {
            bRet = true;
            break;
        }
    }

    return bRet;
}

sal_Bool SwTokenWindow::CreateQuickHelp(Control* pCtrl,
            const SwFormToken& rToken,
            const HelpEvent& rHEvt)
{
    sal_Bool bRet = sal_False;
    if( rHEvt.GetMode() & HELPMODE_QUICK )
    {
        sal_Bool bBalloon = Help::IsBalloonHelpEnabled();
        String sEntry;
        if(bBalloon || rToken.eTokenType != TOKEN_AUTHORITY)
            sEntry = (aButtonHelpTexts[rToken.eTokenType]);
        if(rToken.eTokenType == TOKEN_AUTHORITY )
        {
             sEntry += SwAuthorityFieldType::GetAuthFieldName(
                                (ToxAuthorityField) rToken.nAuthorityField);
        }

     Point aPos = OutputToScreenPixel(pCtrl->GetPosPixel());
     Rectangle aItemRect( aPos, pCtrl->GetSizePixel() );
        if(rToken.eTokenType == TOKEN_TAB_STOP )
        {
        }
        else
        {
            if (!rToken.sCharStyleName.isEmpty())
            {
                if(bBalloon)
                    sEntry += '\n';
                else
                    sEntry += ' ';
                sEntry += sCharStyle;
                  sEntry += rToken.sCharStyleName;
            }
        }
        if(bBalloon)
        {
            Help::ShowBalloon( this, aPos, aItemRect, sEntry );
        }
        else
            Help::ShowQuickHelp( this, aItemRect, sEntry,
                QUICKHELP_LEFT|QUICKHELP_VCENTER );
        bRet = sal_True;
    }
    return bRet;
}

IMPL_LINK(SwTokenWindow, EditResize, Edit*, pEdit)
{
    ((SwTOXEdit*)pEdit)->AdjustSize();
    AdjustPositions();
    if(aModifyHdl.IsSet())
        aModifyHdl.Call(0);
    return 0;
}

IMPL_LINK(SwTokenWindow, NextItemHdl, SwTOXEdit*,  pEdit)
{
    ctrl_iterator it = std::find(aControlList.begin(),aControlList.end(),pEdit);

    if (it == aControlList.end())
        return 0;

    ctrl_iterator itTest = it;
    ++itTest;

    if ((it != aControlList.begin() && !pEdit->IsNextControl()) ||
        (itTest != aControlList.end() && pEdit->IsNextControl()))
    {
        ctrl_iterator iterFocus = it;
        pEdit->IsNextControl() ? ++iterFocus : --iterFocus;

        Control *pCtrlFocus = *iterFocus;
        pCtrlFocus->GrabFocus();
        static_cast<SwTOXButton*>(pCtrlFocus)->Check();

        AdjustScrolling();
    }

    return 0;
}

IMPL_LINK(SwTokenWindow, TbxFocusHdl, SwTOXEdit*, pEdit)
{
    for (ctrl_iterator it = aControlList.begin(); it != aControlList.end(); ++it)
    {
        Control *pCtrl = *it;

        if (pCtrl && pCtrl->GetType() != WINDOW_EDIT)
            static_cast<SwTOXButton*>(pCtrl)->Check(false);
    }

    SetActiveControl(pEdit);

    return 0;
}

IMPL_LINK(SwTokenWindow, NextItemBtnHdl, SwTOXButton*, pBtn )
{
    ctrl_iterator it = std::find(aControlList.begin(),aControlList.end(),pBtn);

    if (it == aControlList.end())
        return 0;

    ctrl_iterator itTest = it;
    ++itTest;

    if (!pBtn->IsNextControl() || (itTest != aControlList.end() && pBtn->IsNextControl()))
    {
        bool isNext = pBtn->IsNextControl();

        ctrl_iterator iterFocus = it;
        isNext ? ++iterFocus : --iterFocus;

        Control *pCtrlFocus = *iterFocus;
        pCtrlFocus->GrabFocus();
        Selection aSel(0,0);

        if (!isNext)
        {
            sal_Int32 nLen = static_cast<SwTOXEdit*>(pCtrlFocus)->GetText().getLength();

            aSel.A() = (sal_uInt16)nLen;
            aSel.B() = (sal_uInt16)nLen;
        }

        static_cast<SwTOXEdit*>(pCtrlFocus)->SetSelection(aSel);

        pBtn->Check(false);

        AdjustScrolling();
    }

    return 0;
}

IMPL_LINK(SwTokenWindow, TbxFocusBtnHdl, SwTOXButton*, pBtn )
{
    for (ctrl_iterator it = aControlList.begin(); it != aControlList.end(); ++it)
    {
        Control *pControl = *it;

        if (pControl && WINDOW_EDIT != pControl->GetType())
            static_cast<SwTOXButton*>(pControl)->Check(pBtn == pControl);
    }

    SetActiveControl(pBtn);

    return 0;
}

void SwTokenWindow::GetFocus()
{
    if(GETFOCUS_TAB & GetGetFocusFlags())
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

SwTOXStylesTabPage::SwTOXStylesTabPage(Window* pParent, const SfxItemSet& rAttrSet )
    : SfxTabPage(pParent, "TocStylesPage",
        "modules/swriter/ui/tocstylespage.ui", rAttrSet)
    , m_pCurrentForm(0)
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

    SetExchangeSupport( sal_True );

    m_pEditStyleBT->SetClickHdl   (LINK(   this, SwTOXStylesTabPage, EditStyleHdl));
    m_pAssignBT->SetClickHdl      (LINK(   this, SwTOXStylesTabPage, AssignHdl));
    m_pStdBT->SetClickHdl         (LINK(   this, SwTOXStylesTabPage, StdHdl));
    m_pParaLayLB->SetSelectHdl    (LINK(   this, SwTOXStylesTabPage, EnableSelectHdl));
    m_pLevelLB->SetSelectHdl(LINK(this, SwTOXStylesTabPage, EnableSelectHdl));
    m_pParaLayLB->SetDoubleClickHdl(LINK(  this, SwTOXStylesTabPage, DoubleClickHdl));
}

SwTOXStylesTabPage::~SwTOXStylesTabPage()
{
    delete m_pCurrentForm;
}

sal_Bool SwTOXStylesTabPage::FillItemSet( SfxItemSet& )
{
    return sal_True;
}

void SwTOXStylesTabPage::Reset( const SfxItemSet& rSet )
{
    ActivatePage(rSet);
}

void SwTOXStylesTabPage::ActivatePage( const SfxItemSet& )
{
    m_pCurrentForm = new SwForm(GetForm());
    m_pParaLayLB->Clear();
    m_pLevelLB->Clear();

    // not hyperlink for user directories

    sal_uInt16 i, nSize = m_pCurrentForm->GetFormMax();

    // display form pattern without title

    // display 1st TemplateEntry
    String aStr( SW_RES( STR_TITLE ));
    if( !m_pCurrentForm->GetTemplate( 0 ).isEmpty() )
    {
        aStr += ' ';
        aStr += aDeliStart;
        aStr += m_pCurrentForm->GetTemplate( 0 );
        aStr += aDeliEnd;
    }
    m_pLevelLB->InsertEntry(aStr);

    for( i=1; i < nSize; ++i )
    {
        if( TOX_INDEX == m_pCurrentForm->GetTOXType() &&
            FORM_ALPHA_DELIMITTER == i )
            aStr = SW_RESSTR(STR_ALPHA);
        else
        {
            aStr  = SW_RESSTR(STR_LEVEL);
            aStr += OUString::number(
                    TOX_INDEX == m_pCurrentForm->GetTOXType() ? i - 1 : i );
        }
        String aCpy( aStr );

        if( !m_pCurrentForm->GetTemplate( i ).isEmpty() )
        {
            aCpy += ' ';
            aCpy += aDeliStart;
            aCpy += m_pCurrentForm->GetTemplate( i );
            aCpy += aDeliEnd;
        }
        m_pLevelLB->InsertEntry( aCpy );
    }

    // initialise templates
    const SwTxtFmtColl *pColl;
    SwWrtShell& rSh = ((SwMultiTOXTabDialog*)GetTabDialog())->GetWrtShell();
    const sal_uInt16 nSz = rSh.GetTxtFmtCollCount();

    for( i = 0; i < nSz; ++i )
        if( !(pColl = &rSh.GetTxtFmtColl( i ))->IsDefault() )
            m_pParaLayLB->InsertEntry( pColl->GetName() );

    // query pool collections and set them for the directory
    for( i = 0; i < m_pCurrentForm->GetFormMax(); ++i )
    {
        aStr = m_pCurrentForm->GetTemplate( i );
        if( aStr.Len() &&
            LISTBOX_ENTRY_NOTFOUND == m_pParaLayLB->GetEntryPos( aStr ))
            m_pParaLayLB->InsertEntry( aStr );
    }

    EnableSelectHdl(m_pParaLayLB);
}

int     SwTOXStylesTabPage::DeactivatePage( SfxItemSet* /*pSet*/  )
{
    GetForm() = *m_pCurrentForm;
    return LEAVE_PAGE;
}

SfxTabPage* SwTOXStylesTabPage::Create( Window* pParent,
                                const SfxItemSet& rAttrSet)
{
    return new SwTOXStylesTabPage(pParent, rAttrSet);
}

IMPL_LINK( SwTOXStylesTabPage, EditStyleHdl, Button *, pBtn )
{
    if( LISTBOX_ENTRY_NOTFOUND != m_pParaLayLB->GetSelectEntryPos())
    {
        SfxStringItem aStyle(SID_STYLE_EDIT, m_pParaLayLB->GetSelectEntry());
        SfxUInt16Item aFamily(SID_STYLE_FAMILY, SFX_STYLE_FAMILY_PARA);
        Window* pDefDlgParent = Application::GetDefDialogParent();
        Application::SetDefDialogParent( pBtn );
        SwWrtShell& rSh = ((SwMultiTOXTabDialog*)GetTabDialog())->GetWrtShell();
        rSh.GetView().GetViewFrame()->GetDispatcher()->Execute(
        SID_STYLE_EDIT, SFX_CALLMODE_SYNCHRON|SFX_CALLMODE_MODAL,
            &aStyle, &aFamily, 0L);
        Application::SetDefDialogParent( pDefDlgParent );
    }
    return 0;
}
/*--------------------------------------------------------------------
     Description: allocate templates
 --------------------------------------------------------------------*/
IMPL_LINK_NOARG(SwTOXStylesTabPage, AssignHdl)
{
    sal_uInt16 nLevPos   = m_pLevelLB->GetSelectEntryPos();
    sal_uInt16 nTemplPos = m_pParaLayLB->GetSelectEntryPos();
    if(nLevPos   != LISTBOX_ENTRY_NOTFOUND &&
       nTemplPos != LISTBOX_ENTRY_NOTFOUND)
    {
        String aStr(m_pLevelLB->GetEntry(nLevPos));
        sal_uInt16 nDelPos = aStr.Search(aDeliStart);
        if(nDelPos != STRING_NOTFOUND)
            aStr.Erase(nDelPos-1);
        aStr += ' ';
        aStr += aDeliStart;
        aStr += m_pParaLayLB->GetSelectEntry();

        m_pCurrentForm->SetTemplate(nLevPos, m_pParaLayLB->GetSelectEntry());

        aStr += aDeliEnd;

        m_pLevelLB->RemoveEntry(nLevPos);
        m_pLevelLB->InsertEntry(aStr, nLevPos);
        m_pLevelLB->SelectEntry(aStr);
        Modify();
    }
    return 0;
}

IMPL_LINK_NOARG(SwTOXStylesTabPage, StdHdl)
{
    sal_uInt16 nPos = m_pLevelLB->GetSelectEntryPos();
    if(nPos != LISTBOX_ENTRY_NOTFOUND)
    {   String aStr(m_pLevelLB->GetEntry(nPos));
        sal_uInt16 nDelPos = aStr.Search(aDeliStart);
        if(nDelPos != STRING_NOTFOUND)
            aStr.Erase(nDelPos-1);
        m_pLevelLB->RemoveEntry(nPos);
        m_pLevelLB->InsertEntry(aStr, nPos);
        m_pLevelLB->SelectEntry(aStr);
        m_pCurrentForm->SetTemplate(nPos, aEmptyStr);
        Modify();
    }
    return 0;
}

IMPL_LINK_NOARG_INLINE_START(SwTOXStylesTabPage, DoubleClickHdl)
{
    String aTmpName( m_pParaLayLB->GetSelectEntry() );
    SwWrtShell& rSh = ((SwMultiTOXTabDialog*)GetTabDialog())->GetWrtShell();

    if(m_pParaLayLB->GetSelectEntryPos() != LISTBOX_ENTRY_NOTFOUND &&
       (m_pLevelLB->GetSelectEntryPos() == 0 || SwMultiTOXTabDialog::IsNoNum(rSh, aTmpName)))
        AssignHdl(m_pAssignBT);
    return 0;
}
IMPL_LINK_NOARG_INLINE_END(SwTOXStylesTabPage, DoubleClickHdl)

/*--------------------------------------------------------------------
     Description: enable only when selected
 --------------------------------------------------------------------*/
IMPL_LINK_NOARG(SwTOXStylesTabPage, EnableSelectHdl)
{
    m_pStdBT->Enable(m_pLevelLB->GetSelectEntryPos()  != LISTBOX_ENTRY_NOTFOUND);

    SwWrtShell& rSh = ((SwMultiTOXTabDialog*)GetTabDialog())->GetWrtShell();
    String aTmpName(m_pParaLayLB->GetSelectEntry());
    m_pAssignBT->Enable(m_pParaLayLB->GetSelectEntryPos() != LISTBOX_ENTRY_NOTFOUND &&
                     LISTBOX_ENTRY_NOTFOUND != m_pLevelLB->GetSelectEntryPos() &&
       (m_pLevelLB->GetSelectEntryPos() == 0 || SwMultiTOXTabDialog::IsNoNum(rSh, aTmpName)));
    m_pEditStyleBT->Enable(m_pParaLayLB->GetSelectEntryPos() != LISTBOX_ENTRY_NOTFOUND );
    return 0;
}

void SwTOXStylesTabPage::Modify()
{
    SwMultiTOXTabDialog* pTOXDlg = (SwMultiTOXTabDialog*)GetTabDialog();
    if(pTOXDlg)
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


SwEntryBrowseBox::SwEntryBrowseBox(Window* pParent, VclBuilderContainer* pBuilder)
    : SwEntryBrowseBox_Base( pParent, EBBF_NONE, WB_TABSTOP | WB_BORDER,
                           BROWSER_KEEPSELECTION |
                           BROWSER_COLUMNSELECTION |
                           BROWSER_MULTISELECTION |
                           BROWSER_TRACKING_TIPS |
                           BROWSER_HLINESFULL |
                           BROWSER_VLINESFULL |
                           BROWSER_AUTO_VSCROLL|
                           BROWSER_HIDECURSOR   )
    , aCellEdit(&GetDataWindow(), 0)
    , aCellCheckBox(&GetDataWindow())
    , nCurrentRow(0)
    , bModified(false)
{
    sSearch = pBuilder->get<Window>("searchterm")->GetText();
    sAlternative = pBuilder->get<Window>("alternative")->GetText();
    sPrimKey = pBuilder->get<Window>("key1")->GetText();
    sSecKey = pBuilder->get<Window>("key2")->GetText();
    sComment = pBuilder->get<Window>("comment")->GetText();
    sCaseSensitive = pBuilder->get<Window>("casesensitive")->GetText();
    sWordOnly = pBuilder->get<Window>("wordonly")->GetText();
    sYes = pBuilder->get<Window>("yes")->GetText();
    sNo = pBuilder->get<Window>("no")->GetText();

    aCellCheckBox.GetBox().EnableTriState(sal_False);
    xController = new ::svt::EditCellController(&aCellEdit);
    xCheckController = new ::svt::CheckBoxCellController(&aCellCheckBox);

    //////////////////////////////////////////////////////////////////////
    // HACK: BrowseBox doesn't invalidate its children, how it should be.
    // That's why WB_CLIPCHILDREN is reset in order to enforce the
    // children' invalidation
    WinBits aStyle = GetStyle();
    if( aStyle & WB_CLIPCHILDREN )
    {
        aStyle &= ~WB_CLIPCHILDREN;
        SetStyle( aStyle );
    }
    const String* aTitles[7] =
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
        InsertDataColumn( i, *aTitles[i - 1], nWidth,
                          HIB_STDSTYLE, HEADERBAR_APPEND );

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
        OUString sTitle = GetColumnTitle(i);
        long nColWidth = std::max(nStandardColMinWidth, GetTextWidth(sTitle));
        nColWidth += 12;
        aWidths.push_back(nColWidth);
    }

    for (sal_uInt16 i = 6; i < 8; i++)
    {
        OUString sTitle = GetColumnTitle(i);
        long nColWidth = std::max(nYesNoWidth, GetTextWidth(sTitle));
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

sal_Bool    SwEntryBrowseBox::SeekRow( long nRow )
{
    nCurrentRow = nRow;
    return sal_True;
}

OUString SwEntryBrowseBox::GetCellText(long nRow, sal_uInt16 nColumn) const
{
    const String* pRet = &aEmptyStr;
    if(static_cast<sal_uInt16>( aEntryArr.size() ) > nRow)
    {
        const AutoMarkEntry* pEntry = &aEntryArr[ nRow ];
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

void    SwEntryBrowseBox::PaintCell(OutputDevice& rDev,
                                const Rectangle& rRect, sal_uInt16 nColumnId) const
{
    String sPaint = GetCellText( nCurrentRow, nColumnId );
    sal_uInt16 nStyle = TEXT_DRAW_CLIP | TEXT_DRAW_CENTER;
    rDev.DrawText( rRect, sPaint, nStyle );
}

::svt::CellController* SwEntryBrowseBox::GetController(long /*nRow*/, sal_uInt16 nCol)
{
    return nCol < ITEM_CASE ? xController : xCheckController;
}

sal_Bool SwEntryBrowseBox::SaveModified()
{
    SetModified();
    sal_uInt16 nRow = static_cast< sal_uInt16 >(GetCurRow());
    sal_uInt16 nCol = GetCurColumnId();

    String sNew;
    sal_Bool bVal = sal_False;
    ::svt::CellController* pController = 0;
    if(nCol < ITEM_CASE)
    {
        pController = xController;
        sNew = ((::svt::EditCellController*)pController)->GetEditImplementation()->GetText( LINEEND_LF );
    }
    else
    {
        pController = xCheckController;
        bVal = ((::svt::CheckBoxCellController*)pController)->GetCheckBox().IsChecked();
    }
    AutoMarkEntry* pEntry = nRow >= aEntryArr.size() ? new AutoMarkEntry
                                                      : &aEntryArr[nRow];
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
    if(nRow >= aEntryArr.size())
    {
        aEntryArr.push_back( pEntry );
        RowInserted(nRow, 1, sal_True, sal_True);
        if(nCol < ITEM_WORDONLY)
        {
            pController->ClearModified();
            GoToRow( nRow );
        }
    }
    return sal_True;
}

void    SwEntryBrowseBox::InitController(
                ::svt::CellControllerRef& rController, long nRow, sal_uInt16 nCol)
{
    String rTxt = GetCellText( nRow, nCol );
    if(nCol < ITEM_CASE)
    {
        rController = xController;
        ::svt::CellController* pController = xController;
        ((::svt::EditCellController*)pController)->GetEditImplementation()->SetText( rTxt );
    }
    else
    {
        rController = xCheckController;
        ::svt::CellController* pController = xCheckController;
        ((::svt::CheckBoxCellController*)pController)->GetCheckBox().Check(
                                                            rTxt == sYes );
     }
}

void    SwEntryBrowseBox::ReadEntries(SvStream& rInStr)
{
    AutoMarkEntry* pToInsert = 0;
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

                String sStr = sLine.getToken(0, ';', nSttPos );
                pToInsert->bCase = sStr.Len() && !comphelper::string::equals(sStr, '0');

                sStr = sLine.getToken(0, ';', nSttPos );
                pToInsert->bWord = sStr.Len() && !comphelper::string::equals(sStr, '0');

                aEntryArr.push_back( pToInsert );
                pToInsert = 0;
            }
            else
            {
                if(pToInsert)
                    aEntryArr.push_back(pToInsert);
                pToInsert = new AutoMarkEntry;
                pToInsert->sComment = sLine;
                pToInsert->sComment.Erase(0, 1);
            }
        }
    }
    if( pToInsert )
        aEntryArr.push_back(pToInsert);
    RowInserted(0, aEntryArr.size() + 1, sal_True);
}

void    SwEntryBrowseBox::WriteEntries(SvStream& rOutStr)
{
    //check if the current controller is modified
    sal_uInt16 nCol = GetCurColumnId();
    ::svt::CellController* pController;
    if(nCol < ITEM_CASE)
        pController = xController;
    else
        pController = xCheckController;
    if(pController ->IsModified())
        GoToColumnId(nCol < ITEM_CASE ? ++nCol : --nCol );

    rtl_TextEncoding  eTEnc = osl_getThreadTextEncoding();
    for(sal_uInt16 i = 0; i < aEntryArr.size(); i++)
    {
        AutoMarkEntry* pEntry = &aEntryArr[i];
        if(pEntry->sComment.Len())
        {
            String sWrite = OUString('#');
            sWrite += pEntry->sComment;
            rOutStr.WriteByteStringLine( sWrite, eTEnc );
        }

        String sWrite( pEntry->sSearch );
        sWrite += ';';
        sWrite += pEntry->sAlternative;
        sWrite += ';';
        sWrite += pEntry->sPrimKey;
        sWrite += ';';
        sWrite += pEntry->sSecKey;
        sWrite += ';';
        sWrite += pEntry->bCase ? '1' : '0';
        sWrite += ';';
        sWrite += pEntry->bWord ? '1' : '0';

        if( sWrite.Len() > 5 )
            rOutStr.WriteByteStringLine( sWrite, eTEnc );
    }
}

sal_Bool SwEntryBrowseBox::IsModified()const
{
    if(bModified)
        return sal_True;


    //check if the current controller is modified
    sal_uInt16 nCol = GetCurColumnId();
    ::svt::CellController* pController;
    if(nCol < ITEM_CASE)
        pController = xController;
    else
        pController = xCheckController;
    return pController ->IsModified();
}

SwAutoMarkDlg_Impl::SwAutoMarkDlg_Impl(Window* pParent, const String& rAutoMarkURL,
        const String& rAutoMarkType, bool bCreate)
    : ModalDialog(pParent, "CreateAutomarkDialog",
        "modules/swriter/ui/createautomarkdialog.ui")
    , sAutoMarkURL(rAutoMarkURL)
    , sAutoMarkType(rAutoMarkType)
    , bCreateMode(bCreate)
{
    get(m_pOKPB, "ok");
    m_pEntriesBB = new SwEntryBrowseBox(get<VclContainer>("area"), this);
    m_pEntriesBB->set_expand(true);
    m_pEntriesBB->Show();
    m_pOKPB->SetClickHdl(LINK(this, SwAutoMarkDlg_Impl, OkHdl));

    String sTitle = GetText();
    sTitle.AppendAscii( RTL_CONSTASCII_STRINGPARAM(": "));
    sTitle += sAutoMarkURL;
    SetText(sTitle);
    bool bError = false;
    if( bCreateMode )
        m_pEntriesBB->RowInserted(0, 1, sal_True);
    else
    {
        SfxMedium aMed( sAutoMarkURL, STREAM_STD_READ );
        if( aMed.GetInStream() && !aMed.GetInStream()->GetError() )
            m_pEntriesBB->ReadEntries( *aMed.GetInStream() );
        else
            bError = true;
    }

    if(bError)
        EndDialog(RET_CANCEL);
}

SwAutoMarkDlg_Impl::~SwAutoMarkDlg_Impl()
{
    delete m_pEntriesBB;
}

IMPL_LINK_NOARG(SwAutoMarkDlg_Impl, OkHdl)
{
    bool bError = false;
    if(m_pEntriesBB->IsModified() || bCreateMode)
    {
        SfxMedium aMed( sAutoMarkURL,
                        bCreateMode ? STREAM_WRITE
                                    : STREAM_WRITE| STREAM_TRUNC );
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
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
