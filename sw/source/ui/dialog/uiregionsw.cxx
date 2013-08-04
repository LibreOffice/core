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

#include <hintids.hxx>
#include <regionsw.hxx>
#include <svl/urihelper.hxx>
#include <svl/PasswordHelper.hxx>
#include <vcl/svapp.hxx>
#include <vcl/msgbox.hxx>
#include <svl/stritem.hxx>
#include <svl/eitem.hxx>
#include <sfx2/passwd.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/request.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/linkmgr.hxx>
#include <sfx2/docinsert.hxx>
#include <sfx2/filedlghelper.hxx>
#include <editeng/sizeitem.hxx>
#include <svtools/htmlcfg.hxx>
#include "svtools/treelistentry.hxx"

#include <comphelper/storagehelper.hxx>
#include <uitool.hxx>
#include <IMark.hxx>
#include <section.hxx>
#include <docary.hxx>
#include <doc.hxx>                      // for the SwSectionFmt-Array
#include <basesh.hxx>
#include <wdocsh.hxx>
#include <view.hxx>
#include <swmodule.hxx>
#include <wrtsh.hxx>
#include <swundo.hxx>                   // for Undo-Ids
#include <column.hxx>
#include <fmtfsize.hxx>
#include <shellio.hxx>

#include <helpid.h>
#include <cmdid.h>
#include <regionsw.hrc>
#include <comcore.hrc>
#include <globals.hrc>
#include <sfx2/bindings.hxx>
#include <sfx2/htmlmode.hxx>
#include <svx/dlgutil.hxx>
#include <svx/dialogs.hrc>
#include <svx/svxdlg.hxx>
#include <svx/flagsdef.hxx>

using namespace ::com::sun::star;

static void   lcl_ReadSections( SfxMedium& rMedium, ComboBox& rBox );

static void lcl_FillList( SwWrtShell& rSh, ComboBox& rSubRegions, ComboBox* pAvailNames, const SwSectionFmt* pNewFmt )
{
    const SwSectionFmt* pFmt;
    if( !pNewFmt )
    {
        sal_uInt16 nCount = rSh.GetSectionFmtCount();
        for(sal_uInt16 i=0;i<nCount;i++)
        {
            SectionType eTmpType;
            if( !(pFmt = &rSh.GetSectionFmt(i))->GetParent() &&
                    pFmt->IsInNodesArr() &&
                    (eTmpType = pFmt->GetSection()->GetType()) != TOX_CONTENT_SECTION
                    && TOX_HEADER_SECTION != eTmpType )
            {
                    String* pString =
                        new String(pFmt->GetSection()->GetSectionName());
                    if(pAvailNames)
                        pAvailNames->InsertEntry(*pString);
                    rSubRegions.InsertEntry(*pString);
                    lcl_FillList( rSh, rSubRegions, pAvailNames, pFmt );
            }
        }
    }
    else
    {
        SwSections aTmpArr;
        sal_uInt16 nCnt = pNewFmt->GetChildSections(aTmpArr,SORTSECT_POS);
        if( nCnt )
        {
            SectionType eTmpType;
            for( sal_uInt16 n = 0; n < nCnt; ++n )
                if( (pFmt = aTmpArr[n]->GetFmt())->IsInNodesArr()&&
                    (eTmpType = pFmt->GetSection()->GetType()) != TOX_CONTENT_SECTION
                    && TOX_HEADER_SECTION != eTmpType )
                {
                    String* pString =
                        new String(pFmt->GetSection()->GetSectionName());
                    if(pAvailNames)
                        pAvailNames->InsertEntry(*pString);
                    rSubRegions.InsertEntry(*pString);
                    lcl_FillList( rSh, rSubRegions, pAvailNames, pFmt );
                }
        }
    }
}

static void lcl_FillSubRegionList( SwWrtShell& rSh, ComboBox& rSubRegions, ComboBox* pAvailNames )
{
    lcl_FillList( rSh, rSubRegions, pAvailNames, 0 );
    IDocumentMarkAccess* const pMarkAccess = rSh.getIDocumentMarkAccess();
    for( IDocumentMarkAccess::const_iterator_t ppMark = pMarkAccess->getMarksBegin();
        ppMark != pMarkAccess->getMarksEnd();
        ++ppMark)
    {
        const ::sw::mark::IMark* pBkmk = ppMark->get();
        if( pBkmk->IsExpanded() )
            rSubRegions.InsertEntry( pBkmk->GetName() );
    }
}

/*----------------------------------------------------------------------------
 Description: user data class for region information
----------------------------------------------------------------------------*/

class SectRepr
{
private:
    SwSectionData           m_SectionData;
    SwFmtCol                m_Col;
    SvxBrushItem            m_Brush;
    SwFmtFtnAtTxtEnd        m_FtnNtAtEnd;
    SwFmtEndAtTxtEnd        m_EndNtAtEnd;
    SwFmtNoBalancedColumns  m_Balance;
    SvxFrameDirectionItem   m_FrmDirItem;
    SvxLRSpaceItem          m_LRSpaceItem;
    sal_uInt16                  m_nArrPos;
    // shows, if maybe textcontent is in the region
    bool                    m_bContent  : 1;
    // for multiselection, mark at first, then work with TreeListBox!
    bool                    m_bSelected : 1;
    uno::Sequence<sal_Int8> m_TempPasswd;

public:
    SectRepr(sal_uInt16 nPos, SwSection& rSect);
    bool    operator==(const SectRepr& rSectRef) const
            { return m_nArrPos == rSectRef.GetArrPos(); }

    bool    operator< (const SectRepr& rSectRef) const
            { return m_nArrPos <  rSectRef.GetArrPos(); }

    SwSectionData &     GetSectionData()        { return m_SectionData; }
    SwSectionData const&GetSectionData() const  { return m_SectionData; }
    SwFmtCol&               GetCol()            { return m_Col; }
    SvxBrushItem&           GetBackground()     { return m_Brush; }
    SwFmtFtnAtTxtEnd&       GetFtnNtAtEnd()     { return m_FtnNtAtEnd; }
    SwFmtEndAtTxtEnd&       GetEndNtAtEnd()     { return m_EndNtAtEnd; }
    SwFmtNoBalancedColumns& GetBalance()        { return m_Balance; }
    SvxFrameDirectionItem&  GetFrmDir()         { return m_FrmDirItem; }
    SvxLRSpaceItem&         GetLRSpace()        { return m_LRSpaceItem; }

    sal_uInt16              GetArrPos() const { return m_nArrPos; }
    String              GetFile() const;
    String              GetSubRegion() const;
    void                SetFile(String const& rFile);
    void                SetFilter(String const& rFilter);
    void                SetSubRegion(String const& rSubRegion);

    bool                IsContent() { return m_bContent; }
    void                SetContent(bool const bValue) { m_bContent = bValue; }

    void                SetSelected() { m_bSelected = true; }
    bool                IsSelected() const { return m_bSelected; }

    uno::Sequence<sal_Int8> & GetTempPasswd() { return m_TempPasswd; }
    void SetTempPasswd(const uno::Sequence<sal_Int8> & rPasswd)
        { m_TempPasswd = rPasswd; }
};


SectRepr::SectRepr( sal_uInt16 nPos, SwSection& rSect )
    : m_SectionData( rSect )
    , m_Brush( RES_BACKGROUND )
    , m_FrmDirItem( FRMDIR_ENVIRONMENT, RES_FRAMEDIR )
    , m_LRSpaceItem( RES_LR_SPACE )
    , m_nArrPos(nPos)
    , m_bContent(m_SectionData.GetLinkFileName().Len() == 0)
    , m_bSelected(false)
{
    SwSectionFmt *pFmt = rSect.GetFmt();
    if( pFmt )
    {
        m_Col = pFmt->GetCol();
        m_Brush = pFmt->GetBackground();
        m_FtnNtAtEnd = pFmt->GetFtnAtTxtEnd();
        m_EndNtAtEnd = pFmt->GetEndAtTxtEnd();
        m_Balance.SetValue(pFmt->GetBalancedColumns().GetValue());
        m_FrmDirItem = pFmt->GetFrmDir();
        m_LRSpaceItem = pFmt->GetLRSpace();
    }
}

void SectRepr::SetFile( const String& rFile )
{
    String sNewFile( INetURLObject::decode( rFile, INET_HEX_ESCAPE,
                                           INetURLObject::DECODE_UNAMBIGUOUS,
                                        RTL_TEXTENCODING_UTF8 ));
    String sOldFileName( m_SectionData.GetLinkFileName() );
    String sSub( sOldFileName.GetToken( 2, sfx2::cTokenSeparator ) );

    if( rFile.Len() || sSub.Len() )
    {
        sNewFile += sfx2::cTokenSeparator;
        if( rFile.Len() ) // Filter only with FileName
            sNewFile += sOldFileName.GetToken( 1, sfx2::cTokenSeparator );

        sNewFile += sfx2::cTokenSeparator;
        sNewFile += sSub;
    }

    m_SectionData.SetLinkFileName( sNewFile );

    if( rFile.Len() || sSub.Len() )
    {
        m_SectionData.SetType( FILE_LINK_SECTION );
    }
    else
    {
        m_SectionData.SetType( CONTENT_SECTION );
    }
}


void SectRepr::SetFilter( const String& rFilter )
{
    String sNewFile;
    String sOldFileName( m_SectionData.GetLinkFileName() );
    String sFile( sOldFileName.GetToken( 0, sfx2::cTokenSeparator ) );
    String sSub( sOldFileName.GetToken( 2, sfx2::cTokenSeparator ) );

    if( sFile.Len() )
        (((( sNewFile = sFile ) += sfx2::cTokenSeparator ) += rFilter )
                                += sfx2::cTokenSeparator ) += sSub;
    else if( sSub.Len() )
        (( sNewFile = sfx2::cTokenSeparator ) += sfx2::cTokenSeparator ) += sSub;

    m_SectionData.SetLinkFileName( sNewFile );

    if( sNewFile.Len() )
    {
        m_SectionData.SetType( FILE_LINK_SECTION );
    }
}

void SectRepr::SetSubRegion(const String& rSubRegion)
{
    String sNewFile;
    String sOldFileName( m_SectionData.GetLinkFileName() );
    String sFilter( sOldFileName.GetToken( 1, sfx2::cTokenSeparator ) );
    sOldFileName = sOldFileName.GetToken( 0, sfx2::cTokenSeparator );

    if( rSubRegion.Len() || sOldFileName.Len() )
        (((( sNewFile = sOldFileName ) += sfx2::cTokenSeparator ) += sFilter )
                                       += sfx2::cTokenSeparator ) += rSubRegion;

    m_SectionData.SetLinkFileName( sNewFile );

    if( rSubRegion.Len() || sOldFileName.Len() )
    {
        m_SectionData.SetType( FILE_LINK_SECTION );
    }
    else
    {
        m_SectionData.SetType( CONTENT_SECTION );
    }
}


String SectRepr::GetFile() const
{
    String sLinkFile( m_SectionData.GetLinkFileName() );
    if( sLinkFile.Len() )
    {
        if (DDE_LINK_SECTION == m_SectionData.GetType())
        {
            sal_uInt16 n = sLinkFile.SearchAndReplace( sfx2::cTokenSeparator, ' ' );
            sLinkFile.SearchAndReplace( sfx2::cTokenSeparator, ' ',  n );
        }
        else
            sLinkFile = INetURLObject::decode( sLinkFile.GetToken( 0,
                                               sfx2::cTokenSeparator ),
                                        INET_HEX_ESCAPE,
                                           INetURLObject::DECODE_UNAMBIGUOUS,
                                        RTL_TEXTENCODING_UTF8 );
    }
    return sLinkFile;
}


String SectRepr::GetSubRegion() const
{
    String sLinkFile( m_SectionData.GetLinkFileName() );
    if( sLinkFile.Len() )
        sLinkFile = sLinkFile.GetToken( 2, sfx2::cTokenSeparator );
    return sLinkFile;
}

/*----------------------------------------------------------------------------
 Description: dialog edit regions
----------------------------------------------------------------------------*/
SwEditRegionDlg::SwEditRegionDlg( Window* pParent, SwWrtShell& rWrtSh )
    : SfxModalDialog(pParent, "EditSectionDialog",
        "modules/swriter/ui/editsectiondialog.ui")
    , m_bSubRegionsFilled(false)
    , aImageIL(SW_RES(IL_SECTION_BITMAPS))
    , rSh(rWrtSh)
    , m_pDocInserter(NULL)
    , m_pOldDefDlgParent(NULL)
    , bDontCheckPasswd(true)
{
    get(m_pCurName, "curname");
    get(m_pTree, "tree");
    m_pTree->set_height_request(m_pTree->GetTextHeight() * 16);
    get(m_pFileCB, "link");
    m_pFileCB->SetState(STATE_NOCHECK);
    get(m_pDDECB, "dde");
    get(m_pDDEFrame, "ddedepend");
    get(m_pFileNameFT, "filenameft");
    get(m_pDDECommandFT, "ddeft");
    get(m_pFileNameED, "filename");
    get(m_pFilePB, "file");
    get(m_pSubRegionFT, "sectionft");
    get(m_pSubRegionED, "section");
    m_pSubRegionED->SetStyle(m_pSubRegionED->GetStyle() | WB_SORT);
    get(m_pProtectCB, "protect");
    m_pProtectCB->SetState(STATE_NOCHECK);
    get(m_pPasswdCB, "withpassword");
    get(m_pPasswdPB, "password");
    get(m_pHideCB, "hide");
    m_pHideCB->SetState(STATE_NOCHECK);
    get(m_pConditionFT, "conditionft");
    get(m_pConditionED, "condition");
    // edit in readonly sections
    get(m_pEditInReadonlyCB, "editinro");
    m_pEditInReadonlyCB->SetState(STATE_NOCHECK);
    get(m_pOptionsPB, "options");
    get(m_pDismiss, "remove");
    get(m_pOK, "ok");

    bWeb = 0 != PTR_CAST( SwWebDocShell, rSh.GetView().GetDocShell() );

    m_pTree->SetSelectHdl(LINK(this, SwEditRegionDlg, GetFirstEntryHdl));
    m_pTree->SetDeselectHdl(LINK(this, SwEditRegionDlg, DeselectHdl));
    m_pCurName->SetModifyHdl(LINK(this, SwEditRegionDlg, NameEditHdl));
    m_pConditionED->SetModifyHdl( LINK( this, SwEditRegionDlg, ConditionEditHdl));
    m_pOK->SetClickHdl         ( LINK( this, SwEditRegionDlg, OkHdl));
    m_pPasswdCB->SetClickHdl(LINK(this, SwEditRegionDlg, ChangePasswdHdl));
    m_pPasswdPB->SetClickHdl(LINK(this, SwEditRegionDlg, ChangePasswdHdl));
    m_pHideCB->SetClickHdl(LINK(this, SwEditRegionDlg, ChangeHideHdl));
    // edit in readonly sections
    m_pEditInReadonlyCB->SetClickHdl(LINK(this, SwEditRegionDlg, ChangeEditInReadonlyHdl));

    m_pOptionsPB->SetClickHdl(LINK(this, SwEditRegionDlg, OptionsHdl));
    m_pProtectCB->SetClickHdl(LINK(this, SwEditRegionDlg, ChangeProtectHdl));
    m_pDismiss->SetClickHdl    ( LINK( this, SwEditRegionDlg, ChangeDismissHdl));
    m_pFileCB->SetClickHdl(LINK(this, SwEditRegionDlg, UseFileHdl));
    m_pFilePB->SetClickHdl(LINK(this, SwEditRegionDlg, FileSearchHdl));
    m_pFileNameED->SetModifyHdl(LINK(this, SwEditRegionDlg, FileNameHdl));
    m_pSubRegionED->SetModifyHdl(LINK(this, SwEditRegionDlg, FileNameHdl));
    m_pSubRegionED->AddEventListener(LINK(this, SwEditRegionDlg, SubRegionEventHdl));
    m_pSubRegionED->EnableAutocomplete(true, true);

    m_pTree->SetSelectionMode( MULTIPLE_SELECTION );
    m_pTree->SetStyle(m_pTree->GetStyle()|WB_HASBUTTONSATROOT|WB_CLIPCHILDREN|WB_HSCROLL);
    m_pTree->SetSpaceBetweenEntries(0);

    if(bWeb)
    {
        m_pDDECB->Hide();
        get<VclContainer>("hideframe")->Hide();
        m_pPasswdCB->Hide();
    }

    m_pDDECB->SetClickHdl(LINK(this, SwEditRegionDlg, DDEHdl));

    pCurrSect = rSh.GetCurrSection();
    RecurseList( 0, 0 );
    // if the cursor is not in a region
    // the first one will always be selected
    if( !m_pTree->FirstSelected() && m_pTree->First() )
        m_pTree->Select( m_pTree->First() );
    m_pTree->Show();
    bDontCheckPasswd = sal_False;
}

sal_Bool SwEditRegionDlg::CheckPasswd(CheckBox* pBox)
{
    if(bDontCheckPasswd)
        return sal_True;
    sal_Bool bRet = sal_True;
    SvTreeListEntry* pEntry = m_pTree->FirstSelected();
    while( pEntry )
    {
        SectReprPtr pRepr = (SectReprPtr)pEntry->GetUserData();
        if (!pRepr->GetTempPasswd().getLength()
            && pRepr->GetSectionData().GetPassword().getLength())
        {
            SfxPasswordDialog aPasswdDlg(this);
            bRet = sal_False;
            if (aPasswdDlg.Execute())
            {
                String sNewPasswd( aPasswdDlg.GetPassword() );
                ::com::sun::star::uno::Sequence <sal_Int8 > aNewPasswd;
                SvPasswordHelper::GetHashPassword( aNewPasswd, sNewPasswd );
                if (SvPasswordHelper::CompareHashPassword(
                        pRepr->GetSectionData().GetPassword(), sNewPasswd))
                {
                    pRepr->SetTempPasswd(aNewPasswd);
                    bRet = sal_True;
                }
                else
                {
                    InfoBox(this, SW_RES(REG_WRONG_PASSWORD)).Execute();
                }
            }
        }
        pEntry = m_pTree->NextSelected(pEntry);
    }
    if(!bRet && pBox)
    {
        //reset old button state
        if(pBox->IsTriStateEnabled())
            pBox->SetState(pBox->IsChecked() ? STATE_NOCHECK : STATE_DONTKNOW);
        else
            pBox->Check(!pBox->IsChecked());
    }

    return bRet;
}

/*---------------------------------------------------------------------
    Description: recursively look for child-sections
---------------------------------------------------------------------*/
void SwEditRegionDlg::RecurseList( const SwSectionFmt* pFmt, SvTreeListEntry* pEntry )
{
    SwSection* pSect = 0;
    SvTreeListEntry* pSelEntry = 0;

    if (!pFmt)
    {
        sal_uInt16 nCount=rSh.GetSectionFmtCount();
        for ( sal_uInt16 n=0; n < nCount; n++ )
        {
            SectionType eTmpType;
            if( !( pFmt = &rSh.GetSectionFmt(n))->GetParent() &&
                pFmt->IsInNodesArr() &&
                (eTmpType = pFmt->GetSection()->GetType()) != TOX_CONTENT_SECTION
                && TOX_HEADER_SECTION != eTmpType )
            {
                SectRepr* pSectRepr = new SectRepr( n,
                                            *(pSect=pFmt->GetSection()) );
                Image aImg = BuildBitmap( pSect->IsProtect(),pSect->IsHidden());
                pEntry = m_pTree->InsertEntry(pSect->GetSectionName(), aImg, aImg);
                pEntry->SetUserData(pSectRepr);
                RecurseList( pFmt, pEntry );
                if (pEntry->HasChildren())
                    m_pTree->Expand(pEntry);
                if (pCurrSect==pSect)
                    m_pTree->Select(pEntry);
            }
        }
    }
    else
    {
        SwSections aTmpArr;
        SvTreeListEntry* pNEntry;
        sal_uInt16 nCnt = pFmt->GetChildSections(aTmpArr,SORTSECT_POS);
        if( nCnt )
        {
            for( sal_uInt16 n = 0; n < nCnt; ++n )
            {
                SectionType eTmpType;
                pFmt = aTmpArr[n]->GetFmt();
                if( pFmt->IsInNodesArr() &&
                    (eTmpType = pFmt->GetSection()->GetType()) != TOX_CONTENT_SECTION
                    && TOX_HEADER_SECTION != eTmpType )
                {
                    pSect=aTmpArr[n];
                    SectRepr* pSectRepr=new SectRepr(
                                    FindArrPos( pSect->GetFmt() ), *pSect );
                    Image aImage = BuildBitmap( pSect->IsProtect(),
                                            pSect->IsHidden());
                    pNEntry = m_pTree->InsertEntry(
                        pSect->GetSectionName(), aImage, aImage, pEntry);
                    pNEntry->SetUserData(pSectRepr);
                    RecurseList( aTmpArr[n]->GetFmt(), pNEntry );
                    if( pNEntry->HasChildren())
                        m_pTree->Expand(pNEntry);
                    if (pCurrSect==pSect)
                        pSelEntry = pNEntry;
                }
            }
        }
    }
    if(0 != pSelEntry)
    {
        m_pTree->MakeVisible(pSelEntry);
        m_pTree->Select(pSelEntry);
    }
}

sal_uInt16 SwEditRegionDlg::FindArrPos(const SwSectionFmt* pFmt )
{
    sal_uInt16 nCount=rSh.GetSectionFmtCount();
    for (sal_uInt16 i=0;i<nCount;i++)
        if (pFmt==&rSh.GetSectionFmt(i))
            return i;

    OSL_FAIL("SectionFormat not on the list" );
    return USHRT_MAX;
}

SwEditRegionDlg::~SwEditRegionDlg( )
{
    SvTreeListEntry* pEntry = m_pTree->First();
    while( pEntry )
    {
        delete (SectRepr*)pEntry->GetUserData();
        pEntry = m_pTree->Next( pEntry );
    }

    delete m_pDocInserter;
}

void    SwEditRegionDlg::SelectSection(const String& rSectionName)
{
    SvTreeListEntry* pEntry = m_pTree->First();
    while(pEntry)
    {
        SectReprPtr pRepr = (SectReprPtr)pEntry->GetUserData();
        if (pRepr->GetSectionData().GetSectionName() == rSectionName)
            break;
        pEntry = m_pTree->Next(pEntry);
    }
    if(pEntry)
    {
        m_pTree->SelectAll(false);
        m_pTree->Select(pEntry);
        m_pTree->MakeVisible(pEntry);
    }
}

/*---------------------------------------------------------------------
    Description:    selected entry in TreeListBox is showed in
                    Edit window
                    in case of multiselection some controls are disabled
---------------------------------------------------------------------*/
IMPL_LINK( SwEditRegionDlg, GetFirstEntryHdl, SvTreeListBox *, pBox )
{
    bDontCheckPasswd = true;
    SvTreeListEntry* pEntry=pBox->FirstSelected();
    m_pHideCB->Enable(true);
    // edit in readonly sections
    m_pEditInReadonlyCB->Enable(true);

    m_pProtectCB->Enable(true);
    m_pFileCB->Enable(true);
    ::com::sun::star::uno::Sequence <sal_Int8> aCurPasswd;
    if( 1 < pBox->GetSelectionCount() )
    {
        m_pHideCB->EnableTriState(true);
        m_pProtectCB->EnableTriState(true);
        // edit in readonly sections
        m_pEditInReadonlyCB->EnableTriState(true);

        m_pFileCB->EnableTriState(true);

        bool bHiddenValid       = true;
        bool bProtectValid      = true;
        bool bConditionValid    = true;
        // edit in readonly sections
        bool bEditInReadonlyValid = true;
        bool bEditInReadonly    = true;

        bool bHidden            = true;
        bool bProtect           = true;
        String sCondition;
        bool bFirst             = true;
        bool bFileValid         = true;
        bool bFile              = true;
        bool bPasswdValid       = true;

        while( pEntry )
        {
            SectRepr* pRepr=(SectRepr*) pEntry->GetUserData();
            SwSectionData const& rData( pRepr->GetSectionData() );
            if(bFirst)
            {
                sCondition      = rData.GetCondition();
                bHidden         = rData.IsHidden();
                bProtect        = rData.IsProtectFlag();
                // edit in readonly sections
                bEditInReadonly = rData.IsEditInReadonlyFlag();

                bFile           = (rData.GetType() != CONTENT_SECTION);
                aCurPasswd      = rData.GetPassword();
            }
            else
            {
                String sTemp(rData.GetCondition());
                if(sCondition != sTemp)
                    bConditionValid = sal_False;
                bHiddenValid      = (bHidden == rData.IsHidden());
                bProtectValid     = (bProtect == rData.IsProtectFlag());
                // edit in readonly sections
                bEditInReadonlyValid =
                    (bEditInReadonly == rData.IsEditInReadonlyFlag());

                bFileValid        = (bFile ==
                    (rData.GetType() != CONTENT_SECTION));
                bPasswdValid      = (aCurPasswd == rData.GetPassword());
            }
            pEntry = pBox->NextSelected(pEntry);
            bFirst = false;
        }

        m_pHideCB->SetState(!bHiddenValid ? STATE_DONTKNOW :
                    bHidden ? STATE_CHECK : STATE_NOCHECK);
        m_pProtectCB->SetState(!bProtectValid ? STATE_DONTKNOW :
                    bProtect ? STATE_CHECK : STATE_NOCHECK);
        // edit in readonly sections
        m_pEditInReadonlyCB->SetState(!bEditInReadonlyValid ? STATE_DONTKNOW :
                    bEditInReadonly ? STATE_CHECK : STATE_NOCHECK);

        m_pFileCB->SetState(!bFileValid ? STATE_DONTKNOW :
                    bFile ? STATE_CHECK : STATE_NOCHECK);

        if (bConditionValid)
            m_pConditionED->SetText(sCondition);
        else
        {
            m_pConditionFT->Enable(false);
            m_pConditionED->Enable(false);
        }

        m_pCurName->Enable(false);
        m_pDDECB->Enable(false);
        m_pDDEFrame->Enable(false);
        m_pOptionsPB->Enable(false);
        bool bPasswdEnabled = m_pProtectCB->GetState() == STATE_CHECK;
        m_pPasswdCB->Enable(bPasswdEnabled);
        m_pPasswdPB->Enable(bPasswdEnabled);
        if(!bPasswdValid)
        {
            pEntry = pBox->FirstSelected();
            pBox->SelectAll( sal_False );
            pBox->Select( pEntry );
            GetFirstEntryHdl(pBox);
            return 0;
        }
        else
            m_pPasswdCB->Check(aCurPasswd.getLength() > 0);
    }
    else if (pEntry )
    {
        m_pCurName->Enable(sal_True);
        m_pOptionsPB->Enable(true);
        SectRepr* pRepr=(SectRepr*) pEntry->GetUserData();
        SwSectionData const& rData( pRepr->GetSectionData() );
        m_pConditionED->SetText(rData.GetCondition());
        m_pHideCB->Enable();
        m_pHideCB->SetState((rData.IsHidden()) ? STATE_CHECK : STATE_NOCHECK);
        bool bHide = STATE_CHECK == m_pHideCB->GetState();
        m_pConditionED->Enable(bHide);
        m_pConditionFT->Enable(bHide);
        m_pPasswdCB->Check(rData.GetPassword().getLength() > 0);

        m_pOK->Enable();
        m_pPasswdCB->Enable();
        m_pCurName->SetText(pBox->GetEntryText(pEntry));
        m_pCurName->Enable();
        m_pDismiss->Enable();
        String aFile = pRepr->GetFile();
        String sSub = pRepr->GetSubRegion();
        m_bSubRegionsFilled = false;
        m_pSubRegionED->Clear();
        if(aFile.Len()||sSub.Len())
        {
            m_pFileCB->Check(true);
            m_pFileNameED->SetText(aFile);
            m_pSubRegionED->SetText(sSub);
            m_pDDECB->Check(rData.GetType() == DDE_LINK_SECTION);
        }
        else
        {
            m_pFileCB->Check(false);
            m_pFileNameED->SetText(aFile);
            m_pDDECB->Enable(false);
            m_pDDECB->Check(false);
        }
        UseFileHdl(m_pFileCB);
        DDEHdl(m_pDDECB);
        m_pProtectCB->SetState((rData.IsProtectFlag())
                ? STATE_CHECK : STATE_NOCHECK);
        m_pProtectCB->Enable();

        // edit in readonly sections
        m_pEditInReadonlyCB->SetState((rData.IsEditInReadonlyFlag())
                ? STATE_CHECK : STATE_NOCHECK);
        m_pEditInReadonlyCB->Enable();

        bool bPasswdEnabled = m_pProtectCB->IsChecked();
        m_pPasswdCB->Enable(bPasswdEnabled);
        m_pPasswdPB->Enable(bPasswdEnabled);
    }
    bDontCheckPasswd = sal_False;
    return 0;
}

IMPL_LINK( SwEditRegionDlg, DeselectHdl, SvTreeListBox *, pBox )
{
    if( !pBox->GetSelectionCount() )
    {
        m_pHideCB->Enable(false);
        m_pProtectCB->Enable(false);
        // edit in readonly sections
        m_pEditInReadonlyCB->Enable(false);

        m_pPasswdCB->Enable(false);
        m_pConditionFT->Enable(false);
        m_pConditionED->Enable(false);
        m_pFileCB->Enable(sal_False);
        m_pDDEFrame->Enable(false);
        m_pDDECB->Enable(false);
        m_pCurName->Enable(false);

        UseFileHdl(m_pFileCB);
        DDEHdl(m_pDDECB);
    }
    return 0;
}

/*---------------------------------------------------------------------
    Description:    in OkHdl the modified settings are being applied
                    and reversed regions are deleted
---------------------------------------------------------------------*/
IMPL_LINK_NOARG(SwEditRegionDlg, OkHdl)
{
    // temp. Array because during changing of a region the position
    // inside of the "Core-Arrays" can be shifted:
    //  - at linked regions, when they have more SubRegions or get
    //    new ones.
    // StartUndo must certainly also happen not before the formats
    // are copied (ClearRedo!)

    const SwSectionFmts& rDocFmts = rSh.GetDoc()->GetSections();
    SwSectionFmts aOrigArray(rDocFmts);

    rSh.StartAllAction();
    rSh.StartUndo();
    rSh.ResetSelect( 0,sal_False );
    SvTreeListEntry* pEntry = m_pTree->First();

    while( pEntry )
    {
        SectReprPtr pRepr = (SectReprPtr) pEntry->GetUserData();
        SwSectionFmt* pFmt = aOrigArray[ pRepr->GetArrPos() ];
        if (!pRepr->GetSectionData().IsProtectFlag())
        {
            pRepr->GetSectionData().SetPassword(uno::Sequence<sal_Int8 >());
        }
        sal_uInt16 nNewPos = rDocFmts.GetPos( pFmt );
        if( USHRT_MAX != nNewPos )
        {
            SfxItemSet* pSet = pFmt->GetAttrSet().Clone( sal_False );
            if( pFmt->GetCol() != pRepr->GetCol() )
                pSet->Put( pRepr->GetCol() );

            if( pFmt->GetBackground(sal_False) != pRepr->GetBackground() )
                pSet->Put( pRepr->GetBackground() );

            if( pFmt->GetFtnAtTxtEnd(sal_False) != pRepr->GetFtnNtAtEnd() )
                pSet->Put( pRepr->GetFtnNtAtEnd() );

            if( pFmt->GetEndAtTxtEnd(sal_False) != pRepr->GetEndNtAtEnd() )
                pSet->Put( pRepr->GetEndNtAtEnd() );

            if( pFmt->GetBalancedColumns() != pRepr->GetBalance() )
                pSet->Put( pRepr->GetBalance() );

            if( pFmt->GetFrmDir() != pRepr->GetFrmDir() )
                pSet->Put( pRepr->GetFrmDir() );

            if( pFmt->GetLRSpace() != pRepr->GetLRSpace())
                pSet->Put( pRepr->GetLRSpace());

            rSh.UpdateSection( nNewPos, pRepr->GetSectionData(),
                            pSet->Count() ? pSet : 0 );
            delete pSet;
        }
        pEntry = m_pTree->Next( pEntry );
    }

    for (SectReprArr::reverse_iterator aI = aSectReprArr.rbegin(), aEnd = aSectReprArr.rend(); aI != aEnd; ++aI)
    {
        SwSectionFmt* pFmt = aOrigArray[ aI->GetArrPos() ];
        sal_uInt16 nNewPos = rDocFmts.GetPos( pFmt );
        if( USHRT_MAX != nNewPos )
            rSh.DelSectionFmt( nNewPos );
    }

    aOrigArray.clear();

    // EndDialog must be called ahead of EndAction's end,
    // otherwise ScrollError can occur.
    EndDialog(RET_OK);

    rSh.EndUndo();
    rSh.EndAllAction();

    return 0;
}

/*---------------------------------------------------------------------
 Description: Toggle protect
---------------------------------------------------------------------*/
IMPL_LINK( SwEditRegionDlg, ChangeProtectHdl, TriStateBox *, pBox )
{
    if(!CheckPasswd(pBox))
        return 0;
    pBox->EnableTriState(false);
    SvTreeListEntry* pEntry = m_pTree->FirstSelected();
    OSL_ENSURE(pEntry,"no entry found");
    sal_Bool bCheck = STATE_CHECK == pBox->GetState();
    while( pEntry )
    {
        SectReprPtr pRepr = (SectReprPtr) pEntry->GetUserData();
        pRepr->GetSectionData().SetProtectFlag(bCheck);
        Image aImage = BuildBitmap(bCheck,
                                   STATE_CHECK == m_pHideCB->GetState());
        m_pTree->SetExpandedEntryBmp(  pEntry, aImage );
        m_pTree->SetCollapsedEntryBmp( pEntry, aImage );
        pEntry = m_pTree->NextSelected(pEntry);
    }
    m_pPasswdCB->Enable(bCheck);
    m_pPasswdPB->Enable(bCheck);
    return 0;
}

/*---------------------------------------------------------------------
 Description: Toggle hide
---------------------------------------------------------------------*/
IMPL_LINK( SwEditRegionDlg, ChangeHideHdl, TriStateBox *, pBox )
{
    if(!CheckPasswd(pBox))
        return 0;
    pBox->EnableTriState(false);
    SvTreeListEntry* pEntry = m_pTree->FirstSelected();
    OSL_ENSURE(pEntry,"no entry found");
    while( pEntry )
    {
        SectReprPtr pRepr = (SectReprPtr) pEntry->GetUserData();
        pRepr->GetSectionData().SetHidden(STATE_CHECK == pBox->GetState());

        Image aImage = BuildBitmap(STATE_CHECK == m_pProtectCB->GetState(),
                                    STATE_CHECK == pBox->GetState());
        m_pTree->SetExpandedEntryBmp(  pEntry, aImage );
        m_pTree->SetCollapsedEntryBmp( pEntry, aImage );

        pEntry = m_pTree->NextSelected(pEntry);
    }

    bool bHide = STATE_CHECK == pBox->GetState();
    m_pConditionED->Enable(bHide);
    m_pConditionFT->Enable(bHide);
    return 0;
}

/*---------------------------------------------------------------------
 Description: Toggle edit in readonly
---------------------------------------------------------------------*/
IMPL_LINK( SwEditRegionDlg, ChangeEditInReadonlyHdl, TriStateBox *, pBox )
{
    if(!CheckPasswd(pBox))
        return 0;
    pBox->EnableTriState(false);
    SvTreeListEntry* pEntry = m_pTree->FirstSelected();
    OSL_ENSURE(pEntry,"no entry found");
    while( pEntry )
    {
        SectReprPtr pRepr = (SectReprPtr) pEntry->GetUserData();
        pRepr->GetSectionData().SetEditInReadonlyFlag(
                STATE_CHECK == pBox->GetState());
        pEntry = m_pTree->NextSelected(pEntry);
    }

    return 0;
}

/*---------------------------------------------------------------------
 Description: clear selected region
---------------------------------------------------------------------*/
IMPL_LINK_NOARG(SwEditRegionDlg, ChangeDismissHdl)
{
    if(!CheckPasswd())
        return 0;
    SvTreeListEntry* pEntry = m_pTree->FirstSelected();
    SvTreeListEntry* pChild;
    SvTreeListEntry* pParent;
    // at first mark all selected
    while(pEntry)
    {
        const SectReprPtr pSectRepr = (SectRepr*)pEntry->GetUserData();
        pSectRepr->SetSelected();
        pEntry = m_pTree->NextSelected(pEntry);
    }
    pEntry = m_pTree->FirstSelected();
    // then delete
    while(pEntry)
    {
        const SectReprPtr pSectRepr = (SectRepr*)pEntry->GetUserData();
        SvTreeListEntry* pRemove = 0;
        bool bRestart = false;
        if(pSectRepr->IsSelected())
        {
            aSectReprArr.insert( pSectRepr );
            while( (pChild = m_pTree->FirstChild(pEntry) )!= 0 )
            {
                // because of the repositioning we have to start at the beginning again
                bRestart = true;
                pParent = m_pTree->GetParent(pEntry);
                m_pTree->GetModel()->Move(pChild, pParent, m_pTree->GetModel()->GetRelPos(pEntry));
            }
            pRemove = pEntry;
        }
        if(bRestart)
            pEntry = m_pTree->First();
        else
            pEntry = m_pTree->Next(pEntry);
        if(pRemove)
            m_pTree->GetModel()->Remove( pRemove );
    }

    if ( m_pTree->FirstSelected() == 0 )
    {
        m_pConditionFT->Enable(false);
        m_pConditionED->Enable(false);
        m_pDismiss->       Enable(sal_False);
        m_pCurName->Enable(false);
        m_pProtectCB->Enable(false);
        m_pPasswdCB->Enable(false);
        m_pHideCB->Enable(false);
        // edit in readonly sections
        m_pEditInReadonlyCB->Enable(false);
        m_pEditInReadonlyCB->SetState(STATE_NOCHECK);
        m_pProtectCB->SetState(STATE_NOCHECK);
        m_pPasswdCB->Check(false);
        m_pHideCB->SetState(STATE_NOCHECK);
        m_pFileCB->Check(false);
        // otherwise the focus would be on HelpButton
        m_pOK->GrabFocus();
        UseFileHdl(m_pFileCB);
    }
    return 0;
}

/*---------------------------------------------------------------------
 Description: link CheckBox to file?
---------------------------------------------------------------------*/
IMPL_LINK( SwEditRegionDlg, UseFileHdl, CheckBox *, pBox )
{
    if(!CheckPasswd(pBox))
        return 0;
    SvTreeListEntry* pEntry = m_pTree->FirstSelected();
    pBox->EnableTriState(sal_False);
    sal_Bool bMulti = 1 < m_pTree->GetSelectionCount();
    sal_Bool bFile = pBox->IsChecked();
    if(pEntry)
    {
        while(pEntry)
        {
            const SectReprPtr pSectRepr = (SectRepr*)pEntry->GetUserData();
            sal_Bool bContent = pSectRepr->IsContent();
            if( pBox->IsChecked() && bContent && rSh.HasSelection() )
            {
                if( RET_NO == QueryBox( this, SW_RES(QB_CONNECT) ).Execute() )
                    pBox->Check( sal_False );
            }
            if( bFile )
                pSectRepr->SetContent(sal_False);
            else
            {
                pSectRepr->SetFile(aEmptyStr);
                pSectRepr->SetSubRegion(aEmptyStr);
                pSectRepr->GetSectionData().SetLinkFilePassword(aEmptyStr);
            }

            pEntry = m_pTree->NextSelected(pEntry);
        }
        m_pDDECB->Enable(bFile && ! bMulti);
        m_pDDEFrame->Enable(bFile && ! bMulti);
        if( bFile )
        {
            m_pProtectCB->SetState(STATE_CHECK);
            m_pFileNameED->GrabFocus();

        }
        else
        {
            m_pDDECB->Check(false);
            DDEHdl(m_pDDECB);
            m_pSubRegionED->SetText(OUString());
        }
    }
    else
    {
        pBox->Check(false);
        pBox->Enable(false);
        m_pDDECB->Check(false);
        m_pDDECB->Enable(false);
        m_pDDEFrame->Enable(false);
    }
    return 0;
}

/*---------------------------------------------------------------------
    Description: call dialog paste file
---------------------------------------------------------------------*/
IMPL_LINK_NOARG(SwEditRegionDlg, FileSearchHdl)
{
    if(!CheckPasswd(0))
        return 0;

    m_pOldDefDlgParent = Application::GetDefDialogParent();
    Application::SetDefDialogParent( this );
    delete m_pDocInserter;
    m_pDocInserter =
        new ::sfx2::DocumentInserter( OUString("swriter") );
    m_pDocInserter->StartExecuteModal( LINK( this, SwEditRegionDlg, DlgClosedHdl ) );
    return 0;
}

IMPL_LINK_NOARG(SwEditRegionDlg, OptionsHdl)
{
    if(!CheckPasswd())
        return 0;
    SvTreeListEntry* pEntry = m_pTree->FirstSelected();

    if(pEntry)
    {
        SectReprPtr pSectRepr = (SectRepr*)pEntry->GetUserData();
        SfxItemSet aSet(rSh.GetView().GetPool(),
                            RES_COL, RES_COL,
                            RES_COLUMNBALANCE, RES_FRAMEDIR,
                            RES_BACKGROUND, RES_BACKGROUND,
                            RES_FRM_SIZE, RES_FRM_SIZE,
                            SID_ATTR_PAGE_SIZE, SID_ATTR_PAGE_SIZE,
                            RES_LR_SPACE, RES_LR_SPACE,
                            RES_FTN_AT_TXTEND, RES_END_AT_TXTEND,
                            0);

        aSet.Put( pSectRepr->GetCol() );
        aSet.Put( pSectRepr->GetBackground() );
        aSet.Put( pSectRepr->GetFtnNtAtEnd() );
        aSet.Put( pSectRepr->GetEndNtAtEnd() );
        aSet.Put( pSectRepr->GetBalance() );
        aSet.Put( pSectRepr->GetFrmDir() );
        aSet.Put( pSectRepr->GetLRSpace() );

        const SwSectionFmts& rDocFmts = rSh.GetDoc()->GetSections();
        SwSectionFmts aOrigArray(rDocFmts);

        SwSectionFmt* pFmt = aOrigArray[pSectRepr->GetArrPos()];
        long nWidth = rSh.GetSectionWidth(*pFmt);
        aOrigArray.clear();
        if (!nWidth)
            nWidth = USHRT_MAX;

        aSet.Put(SwFmtFrmSize(ATT_VAR_SIZE, nWidth));
        aSet.Put(SvxSizeItem(SID_ATTR_PAGE_SIZE, Size(nWidth, nWidth)));

        SwSectionPropertyTabDialog aTabDlg(this, aSet, rSh);
        if(RET_OK == aTabDlg.Execute())
        {
            const SfxItemSet* pOutSet = aTabDlg.GetOutputItemSet();
            if( pOutSet && pOutSet->Count() )
            {
                const SfxPoolItem *pColItem, *pBrushItem,
                                  *pFtnItem, *pEndItem, *pBalanceItem,
                                  *pFrmDirItem, *pLRSpaceItem;
                SfxItemState eColState = pOutSet->GetItemState(
                                        RES_COL, sal_False, &pColItem );
                SfxItemState eBrushState = pOutSet->GetItemState(
                                        RES_BACKGROUND, sal_False, &pBrushItem );
                SfxItemState eFtnState = pOutSet->GetItemState(
                                        RES_FTN_AT_TXTEND, sal_False, &pFtnItem );
                SfxItemState eEndState = pOutSet->GetItemState(
                                        RES_END_AT_TXTEND, sal_False, &pEndItem );
                SfxItemState eBalanceState = pOutSet->GetItemState(
                                        RES_COLUMNBALANCE, sal_False, &pBalanceItem );
                SfxItemState eFrmDirState = pOutSet->GetItemState(
                                        RES_FRAMEDIR, sal_False, &pFrmDirItem );
                SfxItemState eLRState = pOutSet->GetItemState(
                                        RES_LR_SPACE, sal_False, &pLRSpaceItem);

                if( SFX_ITEM_SET == eColState ||
                    SFX_ITEM_SET == eBrushState ||
                    SFX_ITEM_SET == eFtnState ||
                    SFX_ITEM_SET == eEndState ||
                    SFX_ITEM_SET == eBalanceState||
                    SFX_ITEM_SET == eFrmDirState||
                    SFX_ITEM_SET == eLRState)
                {
                    SvTreeListEntry* pSelEntry = m_pTree->FirstSelected();
                    while( pSelEntry )
                    {
                        SectReprPtr pRepr = (SectReprPtr)pSelEntry->GetUserData();
                        if( SFX_ITEM_SET == eColState )
                            pRepr->GetCol() = *(SwFmtCol*)pColItem;
                        if( SFX_ITEM_SET == eBrushState )
                            pRepr->GetBackground() = *(SvxBrushItem*)pBrushItem;
                        if( SFX_ITEM_SET == eFtnState )
                            pRepr->GetFtnNtAtEnd() = *(SwFmtFtnAtTxtEnd*)pFtnItem;
                        if( SFX_ITEM_SET == eEndState )
                            pRepr->GetEndNtAtEnd() = *(SwFmtEndAtTxtEnd*)pEndItem;
                        if( SFX_ITEM_SET == eBalanceState )
                            pRepr->GetBalance().SetValue(((SwFmtNoBalancedColumns*)pBalanceItem)->GetValue());
                        if( SFX_ITEM_SET == eFrmDirState )
                            pRepr->GetFrmDir().SetValue(((SvxFrameDirectionItem*)pFrmDirItem)->GetValue());
                        if( SFX_ITEM_SET == eLRState )
                            pRepr->GetLRSpace() = *(SvxLRSpaceItem*)pLRSpaceItem;

                        pSelEntry = m_pTree->NextSelected(pSelEntry);
                    }
                }
            }
        }
    }

    return 0;
}

/*---------------------------------------------------------------------
    Description:    Applying of the filename or the
                    linked region
---------------------------------------------------------------------*/
IMPL_LINK( SwEditRegionDlg, FileNameHdl, Edit *, pEdit )
{
    Selection aSelect = pEdit->GetSelection();
    if(!CheckPasswd())
        return 0;
    pEdit->SetSelection(aSelect);
    SvTreeListEntry* pEntry = m_pTree->FirstSelected();
    OSL_ENSURE(pEntry,"no entry found");
    SectReprPtr pSectRepr = (SectRepr*)pEntry->GetUserData();
    if (pEdit == m_pFileNameED)
    {
        m_bSubRegionsFilled = false;
        m_pSubRegionED->Clear();
        if (m_pDDECB->IsChecked())
        {
            String sLink( pEdit->GetText() );
            sal_uInt16 nPos = 0;
            while( STRING_NOTFOUND != (nPos = sLink.SearchAscii( "  ", nPos )) )
                sLink.Erase( nPos--, 1 );

            nPos = sLink.SearchAndReplace( ' ', sfx2::cTokenSeparator );
            sLink.SearchAndReplace( ' ', sfx2::cTokenSeparator, nPos );

            pSectRepr->GetSectionData().SetLinkFileName( sLink );
            pSectRepr->GetSectionData().SetType( DDE_LINK_SECTION );
        }
        else
        {
            String sTmp(pEdit->GetText());
            if(sTmp.Len())
            {
                SfxMedium* pMedium = rSh.GetView().GetDocShell()->GetMedium();
                INetURLObject aAbs;
                if( pMedium )
                    aAbs = pMedium->GetURLObject();
                sTmp = URIHelper::SmartRel2Abs(
                    aAbs, sTmp, URIHelper::GetMaybeFileHdl() );
            }
            pSectRepr->SetFile( sTmp );
            pSectRepr->GetSectionData().SetLinkFilePassword( aEmptyStr );
        }
    }
    else
    {
        pSectRepr->SetSubRegion( pEdit->GetText() );
    }
    return 0;
}

IMPL_LINK( SwEditRegionDlg, DDEHdl, CheckBox*, pBox )
{
    if(!CheckPasswd(pBox))
        return 0;
    SvTreeListEntry* pEntry = m_pTree->FirstSelected();
    if(pEntry)
    {
        bool bFile = m_pFileCB->IsChecked();
        SectReprPtr pSectRepr = (SectRepr*)pEntry->GetUserData();
        SwSectionData & rData( pSectRepr->GetSectionData() );
        sal_Bool bDDE = pBox->IsChecked();
        if(bDDE)
        {
            m_pFileNameFT->Hide();
            m_pDDECommandFT->Enable();
            m_pDDECommandFT->Show();
            m_pSubRegionFT->Hide();
            m_pSubRegionED->Hide();
            if (FILE_LINK_SECTION == rData.GetType())
            {
                pSectRepr->SetFile(OUString());
                m_pFileNameED->SetText(OUString());
                rData.SetLinkFilePassword(OUString());
            }
            rData.SetType(DDE_LINK_SECTION);
        }
        else
        {
            m_pDDECommandFT->Hide();
            m_pFileNameFT->Enable(bFile);
            m_pFileNameFT->Show();
            m_pSubRegionED->Show();
            m_pSubRegionFT->Show();
            m_pSubRegionED->Enable(bFile);
            m_pSubRegionFT->Enable(bFile);
            m_pSubRegionED->Enable(bFile);
            if (DDE_LINK_SECTION == rData.GetType())
            {
                rData.SetType(FILE_LINK_SECTION);
                pSectRepr->SetFile(OUString());
                rData.SetLinkFilePassword(OUString());
                m_pFileNameED->SetText(OUString());
            }
        }
        m_pFilePB->Enable(bFile && !bDDE);
    }
    return 0;
}

IMPL_LINK( SwEditRegionDlg, ChangePasswdHdl, Button *, pBox )
{
    bool bChange = pBox == m_pPasswdPB;
    if(!CheckPasswd(0))
    {
        if(!bChange)
            m_pPasswdCB->Check(!m_pPasswdCB->IsChecked());
        return 0;
    }
    SvTreeListEntry* pEntry = m_pTree->FirstSelected();
    bool bSet = bChange ? bChange : m_pPasswdCB->IsChecked();
    OSL_ENSURE(pEntry,"no entry found");
    while( pEntry )
    {
        SectReprPtr pRepr = (SectReprPtr)pEntry->GetUserData();
        if(bSet)
        {
            if(!pRepr->GetTempPasswd().getLength() || bChange)
            {
                SfxPasswordDialog aPasswdDlg(this);
                aPasswdDlg.ShowExtras(SHOWEXTRAS_CONFIRM);
                if(RET_OK == aPasswdDlg.Execute())
                {
                    String sNewPasswd( aPasswdDlg.GetPassword() );
                    if( aPasswdDlg.GetConfirm() == sNewPasswd )
                    {
                        SvPasswordHelper::GetHashPassword( pRepr->GetTempPasswd(), sNewPasswd );
                    }
                    else
                    {
                        InfoBox(pBox, SW_RES(REG_WRONG_PASSWD_REPEAT)).Execute();
                        ChangePasswdHdl(pBox);
                        break;
                    }
                }
                else
                {
                    if(!bChange)
                        m_pPasswdCB->Check(false);
                    break;
                }
            }
            pRepr->GetSectionData().SetPassword(pRepr->GetTempPasswd());
        }
        else
        {
            pRepr->GetSectionData().SetPassword(uno::Sequence<sal_Int8 >());
        }
        pEntry = m_pTree->NextSelected(pEntry);
    }
    return 0;
}

/*---------------------------------------------------------------------
    Description:    the current region name is being added to the
                    TreeListBox immediately during editing, with empty
                    string no Ok()
---------------------------------------------------------------------*/
IMPL_LINK_NOARG(SwEditRegionDlg, NameEditHdl)
{
    if(!CheckPasswd(0))
        return 0;
    SvTreeListEntry* pEntry = m_pTree->FirstSelected();
    OSL_ENSURE(pEntry,"no entry found");
    if (pEntry)
    {
        OUString aName = m_pCurName->GetText();
        m_pTree->SetEntryText(pEntry,aName);
        SectReprPtr pRepr = (SectReprPtr) pEntry->GetUserData();
        pRepr->GetSectionData().SetSectionName(aName);

        m_pOK->Enable(!aName.isEmpty());
    }
    return 0;
}

IMPL_LINK( SwEditRegionDlg, ConditionEditHdl, Edit *, pEdit )
{
    Selection aSelect = pEdit->GetSelection();
    if(!CheckPasswd(0))
        return 0;
    pEdit->SetSelection(aSelect);
    SvTreeListEntry* pEntry = m_pTree->FirstSelected();
    OSL_ENSURE(pEntry,"no entry found");
    while( pEntry )
    {
        SectReprPtr pRepr = (SectReprPtr)pEntry->GetUserData();
        pRepr->GetSectionData().SetCondition(pEdit->GetText());
        pEntry = m_pTree->NextSelected(pEntry);
    }
    return 0;
}

IMPL_LINK( SwEditRegionDlg, DlgClosedHdl, sfx2::FileDialogHelper *, _pFileDlg )
{
    String sFileName, sFilterName, sPassword;
    if ( _pFileDlg->GetError() == ERRCODE_NONE )
    {
        SfxMedium* pMedium = m_pDocInserter->CreateMedium();
        if ( pMedium )
        {
            sFileName = pMedium->GetURLObject().GetMainURL( INetURLObject::NO_DECODE );
            sFilterName = pMedium->GetFilter()->GetFilterName();
            const SfxPoolItem* pItem;
            if ( SFX_ITEM_SET == pMedium->GetItemSet()->GetItemState( SID_PASSWORD, sal_False, &pItem ) )
                sPassword = ( (SfxStringItem*)pItem )->GetValue();
            ::lcl_ReadSections(*pMedium, *m_pSubRegionED);
            delete pMedium;
        }
    }

    SvTreeListEntry* pEntry = m_pTree->FirstSelected();
    OSL_ENSURE( pEntry, "no entry found" );
    if ( pEntry )
    {
        SectReprPtr pSectRepr = (SectRepr*)pEntry->GetUserData();
        pSectRepr->SetFile( sFileName );
        pSectRepr->SetFilter( sFilterName );
        pSectRepr->GetSectionData().SetLinkFilePassword(sPassword);
        m_pFileNameED->SetText(pSectRepr->GetFile());
    }

    Application::SetDefDialogParent( m_pOldDefDlgParent );
    return 0;
}

IMPL_LINK( SwEditRegionDlg, SubRegionEventHdl, VclWindowEvent *, pEvent )
{
    if( !m_bSubRegionsFilled && pEvent && pEvent->GetId() == VCLEVENT_DROPDOWN_PRE_OPEN )
    {
        //if necessary fill the names bookmarks/sections/tables now

        OUString sFileName = m_pFileNameED->GetText();
        if(!sFileName.isEmpty())
        {
            SfxMedium* pMedium = rSh.GetView().GetDocShell()->GetMedium();
            INetURLObject aAbs;
            if( pMedium )
                aAbs = pMedium->GetURLObject();
            sFileName = URIHelper::SmartRel2Abs(
                    aAbs, sFileName, URIHelper::GetMaybeFileHdl() );

            //load file and set the shell
            SfxMedium aMedium( sFileName, STREAM_STD_READ );
            sFileName = aMedium.GetURLObject().GetMainURL( INetURLObject::NO_DECODE );
            ::lcl_ReadSections(aMedium, *m_pSubRegionED);
        }
        else
            lcl_FillSubRegionList(rSh, *m_pSubRegionED, 0);
        m_bSubRegionsFilled = true;
    }
    return 0;
}

Image SwEditRegionDlg::BuildBitmap( sal_Bool bProtect, sal_Bool bHidden )
{
    ImageList& rImgLst = aImageIL;
    return rImgLst.GetImage((!bHidden+(bProtect<<1)) + 1);
}

/*--------------------------------------------------------------------
    Description:    helper function - read region names from medium
 --------------------------------------------------------------------*/
static void lcl_ReadSections( SfxMedium& rMedium, ComboBox& rBox )
{
    rBox.Clear();
    uno::Reference < embed::XStorage > xStg;
    if( rMedium.IsStorage() && (xStg = rMedium.GetStorage()).is() )
    {
        std::vector<String*> aArr;
        sal_uInt32 nFormat = SotStorage::GetFormatID( xStg );
        if ( nFormat == SOT_FORMATSTR_ID_STARWRITER_60 || nFormat == SOT_FORMATSTR_ID_STARWRITERGLOB_60 ||
            nFormat == SOT_FORMATSTR_ID_STARWRITER_8 || nFormat == SOT_FORMATSTR_ID_STARWRITERGLOB_8)
            SwGetReaderXML()->GetSectionList( rMedium, aArr );

        for(std::vector<String*>::const_iterator it(aArr.begin()); it != aArr.end(); ++it) {
            rBox.InsertEntry( **it );
            delete *it;
        }
    }
}

SwInsertSectionTabDialog::SwInsertSectionTabDialog(
            Window* pParent, const SfxItemSet& rSet, SwWrtShell& rSh) :
    SfxTabDialog( pParent, SW_RES(DLG_INSERT_SECTION), &rSet ),
    rWrtSh(rSh)
    , m_pSectionData(0)
{
    String sInsert(SW_RES(ST_INSERT));
    GetOKButton().SetText(sInsert);
    FreeResource();
    SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();
    OSL_ENSURE(pFact, "Dialogdiet fail!");
    AddTabPage(TP_INSERT_SECTION, SwInsertSectionTabPage::Create, 0);
    AddTabPage(TP_COLUMN,   SwColumnPage::Create,    0);
    AddTabPage(TP_BACKGROUND, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BACKGROUND ), 0);
    AddTabPage(TP_SECTION_FTNENDNOTES, SwSectionFtnEndTabPage::Create, 0);
    AddTabPage(TP_SECTION_INDENTS, SwSectionIndentTabPage::Create, 0);

    SvxHtmlOptions& rHtmlOpt = SvxHtmlOptions::Get();
    long nHtmlMode = rHtmlOpt.GetExportMode();

    bool bWeb = 0 != PTR_CAST( SwWebDocShell, rSh.GetView().GetDocShell() );
    if(bWeb)
    {
        RemoveTabPage(TP_SECTION_FTNENDNOTES);
        RemoveTabPage(TP_SECTION_INDENTS);
        if( HTML_CFG_NS40 != nHtmlMode && HTML_CFG_WRITER != nHtmlMode)
            RemoveTabPage(TP_COLUMN);
    }
    SetCurPageId(TP_INSERT_SECTION);
}

SwInsertSectionTabDialog::~SwInsertSectionTabDialog()
{
}

void SwInsertSectionTabDialog::PageCreated( sal_uInt16 nId, SfxTabPage &rPage )
{
    if(TP_INSERT_SECTION == nId)
        ((SwInsertSectionTabPage&)rPage).SetWrtShell(rWrtSh);
    else if( TP_BACKGROUND == nId  )
    {
            SfxAllItemSet aSet(*(GetInputSetImpl()->GetPool()));
            aSet.Put (SfxUInt32Item(SID_FLAG_TYPE, SVX_SHOW_SELECTOR));
            rPage.PageCreated(aSet);
    }
    else if( TP_COLUMN == nId )
    {
        const SwFmtFrmSize& rSize = (const SwFmtFrmSize&)GetInputSetImpl()->Get(RES_FRM_SIZE);
        ((SwColumnPage&)rPage).SetPageWidth(rSize.GetWidth());
        ((SwColumnPage&)rPage).ShowBalance(sal_True);
        ((SwColumnPage&)rPage).SetInSection(sal_True);
    }
    else if(TP_SECTION_INDENTS == nId)
        ((SwSectionIndentTabPage&)rPage).SetWrtShell(rWrtSh);
}

void SwInsertSectionTabDialog::SetSectionData(SwSectionData const& rSect)
{
    m_pSectionData.reset( new SwSectionData(rSect) );
}

short   SwInsertSectionTabDialog::Ok()
{
    short nRet = SfxTabDialog::Ok();
    OSL_ENSURE(m_pSectionData.get(),
            "SwInsertSectionTabDialog: no SectionData?");
    const SfxItemSet* pOutputItemSet = GetOutputItemSet();
    rWrtSh.InsertSection(*m_pSectionData, pOutputItemSet);
    SfxViewFrame* pViewFrm = rWrtSh.GetView().GetViewFrame();
    uno::Reference< frame::XDispatchRecorder > xRecorder =
            pViewFrm->GetBindings().GetRecorder();
    if ( xRecorder.is() )
    {
        SfxRequest aRequest( pViewFrm, FN_INSERT_REGION);
        const SfxPoolItem* pCol;
        if(SFX_ITEM_SET == pOutputItemSet->GetItemState(RES_COL, sal_False, &pCol))
        {
            aRequest.AppendItem(SfxUInt16Item(SID_ATTR_COLUMNS,
                ((const SwFmtCol*)pCol)->GetColumns().size()));
        }
        aRequest.AppendItem(SfxStringItem( FN_PARAM_REGION_NAME,
                    m_pSectionData->GetSectionName()));
        aRequest.AppendItem(SfxStringItem( FN_PARAM_REGION_CONDITION,
                    m_pSectionData->GetCondition()));
        aRequest.AppendItem(SfxBoolItem( FN_PARAM_REGION_HIDDEN,
                    m_pSectionData->IsHidden()));
        aRequest.AppendItem(SfxBoolItem( FN_PARAM_REGION_PROTECT,
                    m_pSectionData->IsProtectFlag()));
        // edit in readonly sections
        aRequest.AppendItem(SfxBoolItem( FN_PARAM_REGION_EDIT_IN_READONLY,
                    m_pSectionData->IsEditInReadonlyFlag()));

        String sLinkFileName( m_pSectionData->GetLinkFileName() );
        aRequest.AppendItem(SfxStringItem( FN_PARAM_1, sLinkFileName.GetToken( 0, sfx2::cTokenSeparator )));
        aRequest.AppendItem(SfxStringItem( FN_PARAM_2, sLinkFileName.GetToken( 1, sfx2::cTokenSeparator )));
        aRequest.AppendItem(SfxStringItem( FN_PARAM_3, sLinkFileName.GetToken( 2, sfx2::cTokenSeparator )));
        aRequest.Done();
    }
    return nRet;
}

SwInsertSectionTabPage::SwInsertSectionTabPage(
                            Window *pParent, const SfxItemSet &rAttrSet)
    : SfxTabPage(pParent, "SectionPage",
        "modules/swriter/ui/sectionpage.ui", rAttrSet)
    , m_pWrtSh(0)
    , m_pDocInserter(NULL)
    , m_pOldDefDlgParent(NULL)
{
    get(m_pCurName, "sectionnames");
    m_pCurName->SetStyle(m_pCurName->GetStyle() | WB_SORT);
    m_pCurName->set_height_request(m_pCurName->GetTextHeight() * 12);
    get(m_pFileCB, "link");
    get(m_pDDECB, "dde");
    get(m_pDDECommandFT, "ddelabel");
    get(m_pFileNameFT, "filelabel");
    get(m_pFileNameED, "filename");
    get(m_pFilePB, "selectfile");
    get(m_pSubRegionFT, "sectionlabel");
    get(m_pSubRegionED, "sectionname");
    m_pSubRegionED->SetStyle(m_pSubRegionED->GetStyle() | WB_SORT);
    get(m_pProtectCB, "protect");
    get(m_pPasswdCB, "withpassword");
    get(m_pPasswdPB, "selectpassword");
    get(m_pHideCB, "hide");
    get(m_pConditionFT, "condlabel");
    get(m_pConditionED, "withcond");
    // edit in readonly sections
    get(m_pEditInReadonlyCB, "editable");



    m_pProtectCB->SetClickHdl  ( LINK( this, SwInsertSectionTabPage, ChangeProtectHdl));
    m_pPasswdCB->SetClickHdl   ( LINK( this, SwInsertSectionTabPage, ChangePasswdHdl));
    m_pPasswdPB->SetClickHdl   ( LINK( this, SwInsertSectionTabPage, ChangePasswdHdl));
    m_pHideCB->SetClickHdl     ( LINK( this, SwInsertSectionTabPage, ChangeHideHdl));
    // edit in readonly sections
    m_pEditInReadonlyCB->SetClickHdl       ( LINK( this, SwInsertSectionTabPage, ChangeEditInReadonlyHdl));
    m_pFileCB->SetClickHdl     ( LINK( this, SwInsertSectionTabPage, UseFileHdl ));
    m_pFilePB->SetClickHdl     ( LINK( this, SwInsertSectionTabPage, FileSearchHdl ));
    m_pCurName->SetModifyHdl   ( LINK( this, SwInsertSectionTabPage, NameEditHdl));
    m_pDDECB->SetClickHdl      ( LINK( this, SwInsertSectionTabPage, DDEHdl ));
    ChangeProtectHdl(m_pProtectCB);
    m_pSubRegionED->EnableAutocomplete( sal_True, sal_True );
}

SwInsertSectionTabPage::~SwInsertSectionTabPage()
{
    delete m_pDocInserter;
}

void    SwInsertSectionTabPage::SetWrtShell(SwWrtShell& rSh)
{
    m_pWrtSh = &rSh;

    bool bWeb = 0 != PTR_CAST(SwWebDocShell, m_pWrtSh->GetView().GetDocShell());
    if(bWeb)
    {
        m_pHideCB->Hide();
        m_pConditionED->Hide();
        m_pConditionFT->Hide();
        m_pDDECB->Hide();
        m_pDDECommandFT->Hide();
    }

    lcl_FillSubRegionList(*m_pWrtSh, *m_pSubRegionED, m_pCurName);

    SwSectionData *const pSectionData =
        static_cast<SwInsertSectionTabDialog*>(GetTabDialog())
            ->GetSectionData();
    if (pSectionData) // something set?
    {
        m_pCurName->SetText(
            rSh.GetUniqueSectionName(& pSectionData->GetSectionName()));
        m_pProtectCB->Check( 0 != pSectionData->IsProtectFlag() );
        m_sFileName = pSectionData->GetLinkFileName();
        m_sFilePasswd = pSectionData->GetLinkFilePassword();
        m_pFileCB->Check( 0 != m_sFileName.Len() );
        m_pFileNameED->SetText( m_sFileName );
        UseFileHdl(m_pFileCB);
    }
    else
    {
        m_pCurName->SetText( rSh.GetUniqueSectionName() );
    }
}

sal_Bool SwInsertSectionTabPage::FillItemSet( SfxItemSet& )
{
    SwSectionData aSection(CONTENT_SECTION, m_pCurName->GetText());
    aSection.SetCondition(m_pConditionED->GetText());
    sal_Bool bProtected = m_pProtectCB->IsChecked();
    aSection.SetProtectFlag(bProtected);
    aSection.SetHidden(m_pHideCB->IsChecked());
    // edit in readonly sections
    aSection.SetEditInReadonlyFlag(m_pEditInReadonlyCB->IsChecked());

    if(bProtected)
    {
        aSection.SetPassword(m_aNewPasswd);
    }
    String sFileName = m_pFileNameED->GetText();
    String sSubRegion = m_pSubRegionED->GetText();
    sal_Bool bDDe = m_pDDECB->IsChecked();
    if(m_pFileCB->IsChecked() && (sFileName.Len() || sSubRegion.Len() || bDDe))
    {
        String aLinkFile;
        if( bDDe )
        {
            aLinkFile = sFileName;

            sal_uInt16 nPos = 0;
            while( STRING_NOTFOUND != (nPos = aLinkFile.SearchAscii( "  ", nPos )) )
                aLinkFile.Erase( nPos--, 1 );

            nPos = aLinkFile.SearchAndReplace( ' ', sfx2::cTokenSeparator );
            aLinkFile.SearchAndReplace( ' ', sfx2::cTokenSeparator, nPos );
        }
        else
        {
            if(sFileName.Len())
            {
                SfxMedium* pMedium = m_pWrtSh->GetView().GetDocShell()->GetMedium();
                INetURLObject aAbs;
                if( pMedium )
                    aAbs = pMedium->GetURLObject();
                aLinkFile = URIHelper::SmartRel2Abs(
                    aAbs, sFileName, URIHelper::GetMaybeFileHdl() );
                aSection.SetLinkFilePassword( m_sFilePasswd );
            }

            aLinkFile += sfx2::cTokenSeparator;
            aLinkFile += m_sFilterName;
            aLinkFile += sfx2::cTokenSeparator;
            aLinkFile += sSubRegion;
        }

        aSection.SetLinkFileName(aLinkFile);
        if(aLinkFile.Len())
        {
            aSection.SetType( m_pDDECB->IsChecked() ?
                                    DDE_LINK_SECTION :
                                        FILE_LINK_SECTION);
        }
    }
    ((SwInsertSectionTabDialog*)GetTabDialog())->SetSectionData(aSection);
    return sal_True;
}

void SwInsertSectionTabPage::Reset( const SfxItemSet& )
{
}

SfxTabPage* SwInsertSectionTabPage::Create( Window* pParent,
                                const SfxItemSet& rAttrSet)
{
    return new SwInsertSectionTabPage(pParent, rAttrSet);
}

IMPL_LINK( SwInsertSectionTabPage, ChangeHideHdl, CheckBox *, pBox )
{
    sal_Bool bHide = pBox->IsChecked();
    m_pConditionED->Enable(bHide);
    m_pConditionFT->Enable(bHide);
    return 0;
}

IMPL_LINK_NOARG(SwInsertSectionTabPage, ChangeEditInReadonlyHdl)
{
    return 0;
}

IMPL_LINK( SwInsertSectionTabPage, ChangeProtectHdl, CheckBox *, pBox )
{
    sal_Bool bCheck = pBox->IsChecked();
    m_pPasswdCB->Enable(bCheck);
    m_pPasswdPB->Enable(bCheck);
    return 0;
}

IMPL_LINK( SwInsertSectionTabPage, ChangePasswdHdl, Button *, pButton )
{
    sal_Bool bChange = pButton == m_pPasswdPB;
    sal_Bool bSet = bChange ? bChange : m_pPasswdCB->IsChecked();
    if(bSet)
    {
        if(!m_aNewPasswd.getLength() || bChange)
        {
            SfxPasswordDialog aPasswdDlg(this);
            aPasswdDlg.ShowExtras(SHOWEXTRAS_CONFIRM);
            if(RET_OK == aPasswdDlg.Execute())
            {
                String sNewPasswd( aPasswdDlg.GetPassword() );
                if( aPasswdDlg.GetConfirm() == sNewPasswd )
                {
                    SvPasswordHelper::GetHashPassword( m_aNewPasswd, sNewPasswd );
                }
                else
                {
                    InfoBox(pButton, SW_RES(REG_WRONG_PASSWD_REPEAT)).Execute();
                }
            }
            else if(!bChange)
                m_pPasswdCB->Check(sal_False);
        }
    }
    else
        m_aNewPasswd.realloc(0);
    return 0;
}

IMPL_LINK_NOARG_INLINE_START(SwInsertSectionTabPage, NameEditHdl)
{
    String  aName=m_pCurName->GetText();
    GetTabDialog()->GetOKButton().Enable(aName.Len() && m_pCurName->GetEntryPos( aName ) == USHRT_MAX);
    return 0;
}
IMPL_LINK_NOARG_INLINE_END(SwInsertSectionTabPage, NameEditHdl)

IMPL_LINK( SwInsertSectionTabPage, UseFileHdl, CheckBox *, pBox )
{
    if( pBox->IsChecked() )
    {
        if( m_pWrtSh->HasSelection() &&
            RET_NO == QueryBox( this, SW_RES(QB_CONNECT) ).Execute() )
            pBox->Check( sal_False );
    }

    sal_Bool bFile = pBox->IsChecked();
    m_pFileNameFT->Enable(bFile);
    m_pFileNameED->Enable(bFile);
    m_pFilePB->Enable(bFile);
    m_pSubRegionFT->Enable(bFile);
    m_pSubRegionED->Enable(bFile);
    m_pDDECommandFT->Enable(bFile);
    m_pDDECB->Enable(bFile);
    if( bFile )
    {
        m_pFileNameED->GrabFocus();
        m_pProtectCB->Check( sal_True );
    }
    else
    {
        m_pDDECB->Check(sal_False);
        DDEHdl(m_pDDECB);
    }
    return 0;
}

IMPL_LINK_NOARG(SwInsertSectionTabPage, FileSearchHdl)
{
    m_pOldDefDlgParent = Application::GetDefDialogParent();
    Application::SetDefDialogParent( this );
    delete m_pDocInserter;
    m_pDocInserter = new ::sfx2::DocumentInserter(
            OUString("swriter") );
    m_pDocInserter->StartExecuteModal( LINK( this, SwInsertSectionTabPage, DlgClosedHdl ) );
    return 0;
}

IMPL_LINK( SwInsertSectionTabPage, DDEHdl, CheckBox*, pBox )
{
    sal_Bool bDDE = pBox->IsChecked();
    sal_Bool bFile = m_pFileCB->IsChecked();
    m_pFilePB->Enable(!bDDE && bFile);
    if(bDDE)
    {
        m_pFileNameFT->Hide();
        m_pDDECommandFT->Enable(bDDE);
        m_pDDECommandFT->Show();
        m_pSubRegionFT->Hide();
        m_pSubRegionED->Hide();
        m_pFileNameED->SetAccessibleName(m_pDDECommandFT->GetText());
    }
    else
    {
        m_pDDECommandFT->Hide();
        m_pFileNameFT->Enable(bFile);
        m_pFileNameFT->Show();
        m_pSubRegionFT->Show();
        m_pSubRegionED->Show();
        m_pSubRegionED->Enable(bFile);
        m_pFileNameED->SetAccessibleName(m_pFileNameFT->GetText());
    }
    return 0;
}

IMPL_LINK( SwInsertSectionTabPage, DlgClosedHdl, sfx2::FileDialogHelper *, _pFileDlg )
{
    if ( _pFileDlg->GetError() == ERRCODE_NONE )
    {
        SfxMedium* pMedium = m_pDocInserter->CreateMedium();
        if ( pMedium )
        {
            m_sFileName = pMedium->GetURLObject().GetMainURL( INetURLObject::NO_DECODE );
            m_sFilterName = pMedium->GetFilter()->GetFilterName();
            const SfxPoolItem* pItem;
            if ( SFX_ITEM_SET == pMedium->GetItemSet()->GetItemState( SID_PASSWORD, sal_False, &pItem ) )
                m_sFilePasswd = ( (SfxStringItem*)pItem )->GetValue();
            m_pFileNameED->SetText( INetURLObject::decode(
                m_sFileName, INET_HEX_ESCAPE, INetURLObject::DECODE_UNAMBIGUOUS, RTL_TEXTENCODING_UTF8 ) );
            ::lcl_ReadSections(*pMedium, *m_pSubRegionED);
            delete pMedium;
        }
    }
    else
        m_sFilterName = m_sFilePasswd = aEmptyStr;

    Application::SetDefDialogParent( m_pOldDefDlgParent );
    return 0;
}

// --------------------------------------------------------------

// numbering format conversion:
// ListBox  - format            - enum-value
// 0        - A, B, C, ...      - 0
// 1        - a, b, c, ...      - 1
// 2        - I, II, III, ...   - 2
// 3        - i, ii, iii, ...   - 3
// 4        - 1, 2, 3, ...      - 4
// 5        - A, .., AA, ..,    - 9
// 6        - a, .., aa, ..,    - 10

inline SvxExtNumType GetNumType( sal_uInt16 n )
{
    return (SvxExtNumType)(4 < n ? n + 4 : n );
}

SwSectionFtnEndTabPage::SwSectionFtnEndTabPage( Window *pParent,
                                                const SfxItemSet &rAttrSet)
    : SfxTabPage( pParent, "FootnotesEndnotesTabPage", "modules/swriter/ui/footnotesendnotestabpage.ui", rAttrSet )

{
    get(pFtnNtAtTextEndCB,"ftnntattextend");

    get(pFtnNtNumCB,"ftnntnum");
    get(pFtnOffsetLbl,"ftnoffset_label");
    get(pFtnOffsetFld,"ftnoffset");

    get(pFtnNtNumFmtCB,"ftnntnumfmt");
    get(pFtnPrefixFT,"ftnprefix_label");
    get(pFtnPrefixED,"ftnprefix");
    get(pFtnNumViewBox,"ftnnumviewbox");
    get(pFtnSuffixFT,"ftnsuffix_label");
    get(pFtnSuffixED,"ftnsuffix");

    get(pEndNtAtTextEndCB,"endntattextend");

    get(pEndNtNumCB,"endntnum");
    get(pEndOffsetLbl,"endoffset_label");
    get(pEndOffsetFld,"endoffset");

    get(pEndNtNumFmtCB,"endntnumfmt");
    get(pEndPrefixFT,"endprefix_label");
    get(pEndPrefixED,"endprefix");
    get(pEndNumViewBox,"endnumviewbox");
    get(pEndSuffixFT,"endsuffix_label");
    get(pEndSuffixED,"endsuffix");

    Link aLk( LINK( this, SwSectionFtnEndTabPage, FootEndHdl));
    pFtnNtAtTextEndCB->SetClickHdl( aLk );
    pFtnNtNumCB->SetClickHdl( aLk );
    pEndNtAtTextEndCB->SetClickHdl( aLk );
    pEndNtNumCB->SetClickHdl( aLk );
    pFtnNtNumFmtCB->SetClickHdl( aLk );
    pEndNtNumFmtCB->SetClickHdl( aLk );
}

SwSectionFtnEndTabPage::~SwSectionFtnEndTabPage()
{
}

sal_Bool SwSectionFtnEndTabPage::FillItemSet( SfxItemSet& rSet )
{
    SwFmtFtnAtTxtEnd aFtn( pFtnNtAtTextEndCB->IsChecked()
                            ? ( pFtnNtNumCB->IsChecked()
                                ? ( pFtnNtNumFmtCB->IsChecked()
                                    ? FTNEND_ATTXTEND_OWNNUMANDFMT
                                    : FTNEND_ATTXTEND_OWNNUMSEQ )
                                : FTNEND_ATTXTEND )
                            : FTNEND_ATPGORDOCEND );

    switch( aFtn.GetValue() )
    {
    case FTNEND_ATTXTEND_OWNNUMANDFMT:
        aFtn.SetNumType( pFtnNumViewBox->GetSelectedNumberingType() );
        aFtn.SetPrefix( pFtnPrefixED->GetText().replaceAll("\\t", "\t") ); // fdo#65666
        aFtn.SetSuffix( pFtnSuffixED->GetText().replaceAll("\\t", "\t") );
        // no break;

    case FTNEND_ATTXTEND_OWNNUMSEQ:
        aFtn.SetOffset( static_cast< sal_uInt16 >( pFtnOffsetFld->GetValue()-1 ) );
        // no break;
    }

    SwFmtEndAtTxtEnd aEnd( pEndNtAtTextEndCB->IsChecked()
                            ? ( pEndNtNumCB->IsChecked()
                                ? ( pEndNtNumFmtCB->IsChecked()
                                    ? FTNEND_ATTXTEND_OWNNUMANDFMT
                                    : FTNEND_ATTXTEND_OWNNUMSEQ )
                                : FTNEND_ATTXTEND )
                            : FTNEND_ATPGORDOCEND );

    switch( aEnd.GetValue() )
    {
    case FTNEND_ATTXTEND_OWNNUMANDFMT:
        aEnd.SetNumType( pEndNumViewBox->GetSelectedNumberingType() );
        aEnd.SetPrefix( pEndPrefixED->GetText().replaceAll("\\t", "\t") );
        aEnd.SetSuffix( pEndSuffixED->GetText().replaceAll("\\t", "\t") );
        // no break;

    case FTNEND_ATTXTEND_OWNNUMSEQ:
        aEnd.SetOffset( static_cast< sal_uInt16 >( pEndOffsetFld->GetValue()-1 ) );
        // no break;
    }

    rSet.Put( aFtn );
    rSet.Put( aEnd );

    return sal_True;
}

void SwSectionFtnEndTabPage::ResetState( sal_Bool bFtn,
                                    const SwFmtFtnEndAtTxtEnd& rAttr )
{
    CheckBox *pNtAtTextEndCB, *pNtNumCB, *pNtNumFmtCB;
    FixedText*pPrefixFT, *pSuffixFT;
    Edit *pPrefixED, *pSuffixED;
    SwNumberingTypeListBox *pNumViewBox;
    FixedText* pOffsetTxt;
    NumericField *pOffsetFld;

    if( bFtn )
    {
        pNtAtTextEndCB = pFtnNtAtTextEndCB;
        pNtNumCB = pFtnNtNumCB;
        pNtNumFmtCB = pFtnNtNumFmtCB;
        pPrefixFT = pFtnPrefixFT;
        pPrefixED = pFtnPrefixED;
        pSuffixFT = pFtnSuffixFT;
        pSuffixED = pFtnSuffixED;
        pNumViewBox = pFtnNumViewBox;
        pOffsetTxt = pFtnOffsetLbl;
        pOffsetFld = pFtnOffsetFld;
    }
    else
    {
        pNtAtTextEndCB = pEndNtAtTextEndCB;
        pNtNumCB = pEndNtNumCB;
        pNtNumFmtCB = pEndNtNumFmtCB;
        pPrefixFT = pEndPrefixFT;
        pPrefixED = pEndPrefixED;
        pSuffixFT = pEndSuffixFT;
        pSuffixED = pEndSuffixED;
        pNumViewBox = pEndNumViewBox;
        pOffsetTxt = pEndOffsetLbl;
        pOffsetFld = pEndOffsetFld;
    }

    sal_uInt16 eState = rAttr.GetValue();
    switch( eState )
    {
    case FTNEND_ATTXTEND_OWNNUMANDFMT:
        pNtNumFmtCB->SetState( STATE_CHECK );
        // no break;

    case FTNEND_ATTXTEND_OWNNUMSEQ:
        pNtNumCB->SetState( STATE_CHECK );
        // no break;

    case FTNEND_ATTXTEND:
        pNtAtTextEndCB->SetState( STATE_CHECK );
        // no break;
    }

    pNumViewBox->SelectNumberingType( rAttr.GetNumType() );
    pOffsetFld->SetValue( rAttr.GetOffset() + 1 );
    pPrefixED->SetText( rAttr.GetPrefix().replaceAll("\t", "\\t") );
    pSuffixED->SetText( rAttr.GetSuffix().replaceAll("\t", "\\t") );

    switch( eState )
    {
    case FTNEND_ATPGORDOCEND:
        pNtNumCB->Enable( sal_False );
        // no break;

    case FTNEND_ATTXTEND:
        pNtNumFmtCB->Enable( sal_False );
        pOffsetFld->Enable( sal_False );
        pOffsetTxt->Enable( sal_False );
        // no break;

    case FTNEND_ATTXTEND_OWNNUMSEQ:
        pNumViewBox->Enable( sal_False );
        pPrefixFT->Enable( sal_False );
        pPrefixED->Enable( sal_False );
        pSuffixFT->Enable( sal_False );
        pSuffixED->Enable( sal_False );
        // no break;
    }
}

void SwSectionFtnEndTabPage::Reset( const SfxItemSet& rSet )
{
    ResetState( sal_True, (const SwFmtFtnAtTxtEnd&)rSet.Get(
                                    RES_FTN_AT_TXTEND, sal_False ));
    ResetState( sal_False, (const SwFmtEndAtTxtEnd&)rSet.Get(
                                    RES_END_AT_TXTEND, sal_False ));
}

SfxTabPage* SwSectionFtnEndTabPage::Create( Window* pParent,
                                const SfxItemSet& rAttrSet)
{
    return new SwSectionFtnEndTabPage(pParent, rAttrSet);
}

IMPL_LINK( SwSectionFtnEndTabPage, FootEndHdl, CheckBox *, pBox )
{
    sal_Bool bFoot = pFtnNtAtTextEndCB == pBox || pFtnNtNumCB == pBox ||
                    pFtnNtNumFmtCB == pBox ;

    CheckBox *pNumBox, *pNumFmtBox, *pEndBox;
    SwNumberingTypeListBox* pNumViewBox;
    FixedText* pOffsetTxt;
    NumericField *pOffsetFld;
    FixedText*pPrefixFT, *pSuffixFT;
    Edit *pPrefixED, *pSuffixED;

    if( bFoot )
    {
        pEndBox = pFtnNtAtTextEndCB;
        pNumBox = pFtnNtNumCB;
        pNumFmtBox = pFtnNtNumFmtCB;
        pNumViewBox = pFtnNumViewBox;
        pOffsetTxt = pFtnOffsetLbl;
        pOffsetFld = pFtnOffsetFld;
        pPrefixFT = pFtnPrefixFT;
        pSuffixFT = pFtnSuffixFT;
        pPrefixED = pFtnPrefixED;
        pSuffixED = pFtnSuffixED;
    }
    else
    {
        pEndBox = pEndNtAtTextEndCB;
        pNumBox = pEndNtNumCB;
        pNumFmtBox = pEndNtNumFmtCB;
        pNumViewBox = pEndNumViewBox;
        pOffsetTxt = pEndOffsetLbl;
        pOffsetFld = pEndOffsetFld;
        pPrefixFT = pEndPrefixFT;
        pSuffixFT = pEndSuffixFT;
        pPrefixED = pEndPrefixED;
        pSuffixED = pEndSuffixED;
    }

    sal_Bool bEnableAtEnd = STATE_CHECK == pEndBox->GetState();
    sal_Bool bEnableNum = bEnableAtEnd && STATE_CHECK == pNumBox->GetState();
    sal_Bool bEnableNumFmt = bEnableNum && STATE_CHECK == pNumFmtBox->GetState();

    pNumBox->Enable( bEnableAtEnd );
    pOffsetTxt->Enable( bEnableNum );
    pOffsetFld->Enable( bEnableNum );
    pNumFmtBox->Enable( bEnableNum );
    pNumViewBox->Enable( bEnableNumFmt );
    pPrefixED->Enable( bEnableNumFmt );
    pSuffixED->Enable( bEnableNumFmt );
    pPrefixFT->Enable( bEnableNumFmt );
    pSuffixFT->Enable( bEnableNumFmt );

    return 0;
}

SwSectionPropertyTabDialog::SwSectionPropertyTabDialog(
    Window* pParent, const SfxItemSet& rSet, SwWrtShell& rSh) :
    SfxTabDialog(pParent, SW_RES(DLG_SECTION_PROPERTIES), &rSet),
    rWrtSh(rSh)
{
    FreeResource();
    SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();
    OSL_ENSURE(pFact, "Dialogdiet fail!");
    AddTabPage(TP_COLUMN,   SwColumnPage::Create,    0);
    AddTabPage(TP_BACKGROUND, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BACKGROUND ), 0 );
    AddTabPage(TP_SECTION_FTNENDNOTES, SwSectionFtnEndTabPage::Create, 0);
    AddTabPage(TP_SECTION_INDENTS, SwSectionIndentTabPage::Create, 0);

    SvxHtmlOptions& rHtmlOpt = SvxHtmlOptions::Get();
    long nHtmlMode = rHtmlOpt.GetExportMode();
    bool bWeb = 0 != PTR_CAST( SwWebDocShell, rSh.GetView().GetDocShell() );
    if(bWeb)
    {
        RemoveTabPage(TP_SECTION_FTNENDNOTES);
        RemoveTabPage(TP_SECTION_INDENTS);
        if( HTML_CFG_NS40 != nHtmlMode && HTML_CFG_WRITER != nHtmlMode)
            RemoveTabPage(TP_COLUMN);
    }
}

SwSectionPropertyTabDialog::~SwSectionPropertyTabDialog()
{
}

void SwSectionPropertyTabDialog::PageCreated( sal_uInt16 nId, SfxTabPage &rPage )
{
    if( TP_BACKGROUND == nId  )
    {
            SfxAllItemSet aSet(*(GetInputSetImpl()->GetPool()));
            aSet.Put (SfxUInt32Item(SID_FLAG_TYPE, SVX_SHOW_SELECTOR));
            rPage.PageCreated(aSet);
    }
    else if( TP_COLUMN == nId )
    {
        ((SwColumnPage&)rPage).ShowBalance(sal_True);
        ((SwColumnPage&)rPage).SetInSection(sal_True);
    }
    else if(TP_SECTION_INDENTS == nId)
        ((SwSectionIndentTabPage&)rPage).SetWrtShell(rWrtSh);
}

SwSectionIndentTabPage::SwSectionIndentTabPage(Window *pParent, const SfxItemSet &rAttrSet)
    : SfxTabPage(pParent, "IndentPage", "modules/swriter/ui/indentpage.ui", rAttrSet)
{
    get(m_pBeforeMF, "before");
    get(m_pAfterMF, "after");
    get(m_pPreviewWin, "preview");
    Link aLk = LINK(this, SwSectionIndentTabPage, IndentModifyHdl);
    m_pBeforeMF->SetModifyHdl(aLk);
    m_pAfterMF->SetModifyHdl(aLk);
}

SwSectionIndentTabPage::~SwSectionIndentTabPage()
{
}

sal_Bool SwSectionIndentTabPage::FillItemSet( SfxItemSet& rSet)
{
    if(m_pBeforeMF->IsValueModified() ||
            m_pAfterMF->IsValueModified())
    {
        SvxLRSpaceItem aLRSpace(
                static_cast< long >(m_pBeforeMF->Denormalize(m_pBeforeMF->GetValue(FUNIT_TWIP))) ,
                static_cast< long >(m_pAfterMF->Denormalize(m_pAfterMF->GetValue(FUNIT_TWIP))), 0, 0, RES_LR_SPACE);
        rSet.Put(aLRSpace);
    }
    return sal_True;
}

void SwSectionIndentTabPage::Reset( const SfxItemSet& rSet)
{
    //this page doesn't show up in HTML mode
    FieldUnit aMetric = ::GetDfltMetric(sal_False);
    SetMetric(*m_pBeforeMF, aMetric);
    SetMetric(*m_pAfterMF , aMetric);

    SfxItemState eItemState = rSet.GetItemState( RES_LR_SPACE );
    if ( eItemState >= SFX_ITEM_AVAILABLE )
    {
        const SvxLRSpaceItem& rSpace =
            (const SvxLRSpaceItem&)rSet.Get( RES_LR_SPACE );

        m_pBeforeMF->SetValue( m_pBeforeMF->Normalize(rSpace.GetLeft()), FUNIT_TWIP );
        m_pAfterMF->SetValue( m_pAfterMF->Normalize(rSpace.GetRight()), FUNIT_TWIP );
    }
    else
    {
        m_pBeforeMF->SetEmptyFieldValue();
        m_pAfterMF->SetEmptyFieldValue();
    }
    m_pBeforeMF->SaveValue();
    m_pAfterMF->SaveValue();
    IndentModifyHdl(0);
}

SfxTabPage*  SwSectionIndentTabPage::Create( Window* pParent, const SfxItemSet& rAttrSet)
{
    return new SwSectionIndentTabPage(pParent, rAttrSet);
}

void SwSectionIndentTabPage::SetWrtShell(SwWrtShell& rSh)
{
    //set sensible values at the preview
    m_pPreviewWin->SetAdjust(SVX_ADJUST_BLOCK);
    m_pPreviewWin->SetLastLine(SVX_ADJUST_BLOCK);
    const SwRect& rPageRect = rSh.GetAnyCurRect( RECT_PAGE, 0 );
    Size aPageSize(rPageRect.Width(), rPageRect.Height());
    m_pPreviewWin->SetSize(aPageSize);
}

IMPL_LINK_NOARG(SwSectionIndentTabPage, IndentModifyHdl)
{
    m_pPreviewWin->SetLeftMargin( static_cast< long >(m_pBeforeMF->Denormalize(m_pBeforeMF->GetValue(FUNIT_TWIP))) );
    m_pPreviewWin->SetRightMargin( static_cast< long >(m_pAfterMF->Denormalize(m_pAfterMF->GetValue(FUNIT_TWIP))) );
    m_pPreviewWin->Draw(sal_True);
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
