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
#include <vcl/weld.hxx>
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
#include <svtools/treelistentry.hxx>

#include <uitool.hxx>
#include <IMark.hxx>
#include <section.hxx>
#include <docary.hxx>
#include <doc.hxx>
#include <basesh.hxx>
#include <wdocsh.hxx>
#include <view.hxx>
#include <swmodule.hxx>
#include <wrtsh.hxx>
#include <swundo.hxx>
#include <column.hxx>
#include <fmtfsize.hxx>
#include <shellio.hxx>

#include <cmdid.h>
#include <strings.hrc>
#include <globals.hrc>
#include <bitmaps.hlst>
#include <sfx2/bindings.hxx>
#include <sfx2/htmlmode.hxx>
#include <svx/dlgutil.hxx>
#include <svx/dialogs.hrc>
#include <svx/svxdlg.hxx>
#include <svx/flagsdef.hxx>
#include <memory>

using namespace ::com::sun::star;

namespace {

Image BuildBitmap(bool bProtect, bool bHidden)
{
    if (bProtect)
        return Image(BitmapEx(bHidden ? OUString(RID_BMP_PROT_HIDE) : OUString(RID_BMP_PROT_NO_HIDE)));
    return Image(BitmapEx(bHidden ? OUString(RID_BMP_HIDE) : OUString(RID_BMP_NO_HIDE)));
}

}

static void   lcl_ReadSections( SfxMedium& rMedium, ComboBox& rBox );

static void lcl_FillList( SwWrtShell& rSh, ComboBox& rSubRegions, ComboBox* pAvailNames, const SwSectionFormat* pNewFormat )
{
    if( !pNewFormat )
    {
        const size_t nCount = rSh.GetSectionFormatCount();
        for (size_t i = 0; i<nCount; i++)
        {
            SectionType eTmpType;
            const SwSectionFormat* pFormat = &rSh.GetSectionFormat(i);
            if( !pFormat->GetParent() &&
                    pFormat->IsInNodesArr() &&
                    (eTmpType = pFormat->GetSection()->GetType()) != TOX_CONTENT_SECTION
                    && TOX_HEADER_SECTION != eTmpType )
            {
                    const OUString sString(pFormat->GetSection()->GetSectionName());
                    if(pAvailNames)
                        pAvailNames->InsertEntry(sString);
                    rSubRegions.InsertEntry(sString);
                    lcl_FillList( rSh, rSubRegions, pAvailNames, pFormat );
            }
        }
    }
    else
    {
        SwSections aTmpArr;
        pNewFormat->GetChildSections(aTmpArr, SectionSort::Pos);
        if( !aTmpArr.empty() )
        {
            SectionType eTmpType;
            for( const auto pSect : aTmpArr )
            {
                const SwSectionFormat* pFormat = pSect->GetFormat();
                if( pFormat->IsInNodesArr()&&
                    (eTmpType = pFormat->GetSection()->GetType()) != TOX_CONTENT_SECTION
                    && TOX_HEADER_SECTION != eTmpType )
                {
                    const OUString sString(pFormat->GetSection()->GetSectionName());
                    if(pAvailNames)
                        pAvailNames->InsertEntry(sString);
                    rSubRegions.InsertEntry(sString);
                    lcl_FillList( rSh, rSubRegions, pAvailNames, pFormat );
                }
            }
        }
    }
}

static void lcl_FillSubRegionList( SwWrtShell& rSh, ComboBox& rSubRegions, ComboBox* pAvailNames )
{
    lcl_FillList( rSh, rSubRegions, pAvailNames, nullptr );
    IDocumentMarkAccess* const pMarkAccess = rSh.getIDocumentMarkAccess();
    for( IDocumentMarkAccess::const_iterator_t ppMark = pMarkAccess->getBookmarksBegin();
        ppMark != pMarkAccess->getBookmarksEnd();
        ++ppMark)
    {
        const ::sw::mark::IMark* pBkmk = ppMark->get();
        if( pBkmk->IsExpanded() )
            rSubRegions.InsertEntry( pBkmk->GetName() );
    }
}

// user data class for region information
class SectRepr
{
private:
    SwSectionData           m_SectionData;
    SwFormatCol                m_Col;
    SvxBrushItem            m_Brush;
    SwFormatFootnoteAtTextEnd        m_FootnoteNtAtEnd;
    SwFormatEndAtTextEnd        m_EndNtAtEnd;
    SwFormatNoBalancedColumns  m_Balance;
    SvxFrameDirectionItem   m_FrameDirItem;
    SvxLRSpaceItem          m_LRSpaceItem;
    const size_t            m_nArrPos;
    // shows, if maybe textcontent is in the region
    bool                    m_bContent  : 1;
    // for multiselection, mark at first, then work with TreeListBox!
    bool                    m_bSelected : 1;
    uno::Sequence<sal_Int8> m_TempPasswd;

public:
    SectRepr(size_t nPos, SwSection& rSect);

    SwSectionData &     GetSectionData()        { return m_SectionData; }
    SwFormatCol&               GetCol()            { return m_Col; }
    SvxBrushItem&           GetBackground()     { return m_Brush; }
    SwFormatFootnoteAtTextEnd&       GetFootnoteNtAtEnd()     { return m_FootnoteNtAtEnd; }
    SwFormatEndAtTextEnd&       GetEndNtAtEnd()     { return m_EndNtAtEnd; }
    SwFormatNoBalancedColumns& GetBalance()        { return m_Balance; }
    SvxFrameDirectionItem&  GetFrameDir()         { return m_FrameDirItem; }
    SvxLRSpaceItem&         GetLRSpace()        { return m_LRSpaceItem; }

    size_t              GetArrPos() const { return m_nArrPos; }
    OUString            GetFile() const;
    OUString            GetSubRegion() const;
    void                SetFile(OUString const& rFile);
    void                SetFilter(OUString const& rFilter);
    void                SetSubRegion(OUString const& rSubRegion);

    bool                IsContent() { return m_bContent; }
    void                SetContent(bool const bValue) { m_bContent = bValue; }

    void                SetSelected() { m_bSelected = true; }
    bool                IsSelected() const { return m_bSelected; }

    uno::Sequence<sal_Int8> & GetTempPasswd() { return m_TempPasswd; }
    void SetTempPasswd(const uno::Sequence<sal_Int8> & rPasswd)
        { m_TempPasswd = rPasswd; }
};

SectRepr::SectRepr( size_t nPos, SwSection& rSect )
    : m_SectionData( rSect )
    , m_Brush( RES_BACKGROUND )
    , m_FrameDirItem( SvxFrameDirection::Environment, RES_FRAMEDIR )
    , m_LRSpaceItem( RES_LR_SPACE )
    , m_nArrPos(nPos)
    , m_bContent(m_SectionData.GetLinkFileName().isEmpty())
    , m_bSelected(false)
{
    SwSectionFormat *pFormat = rSect.GetFormat();
    if( pFormat )
    {
        m_Col = pFormat->GetCol();
        m_Brush = pFormat->makeBackgroundBrushItem();
        m_FootnoteNtAtEnd = pFormat->GetFootnoteAtTextEnd();
        m_EndNtAtEnd = pFormat->GetEndAtTextEnd();
        m_Balance.SetValue(pFormat->GetBalancedColumns().GetValue());
        m_FrameDirItem = pFormat->GetFrameDir();
        m_LRSpaceItem = pFormat->GetLRSpace();
    }
}

void SectRepr::SetFile( const OUString& rFile )
{
    OUString sNewFile( INetURLObject::decode( rFile,
                                           INetURLObject::DecodeMechanism::Unambiguous ));
    const OUString sOldFileName( m_SectionData.GetLinkFileName() );
    const OUString sSub( sOldFileName.getToken( 2, sfx2::cTokenSeparator ) );

    if( !rFile.isEmpty() || !sSub.isEmpty() )
    {
        sNewFile += OUStringLiteral1(sfx2::cTokenSeparator);
        if( !rFile.isEmpty() ) // Filter only with FileName
            sNewFile += sOldFileName.getToken( 1, sfx2::cTokenSeparator );

        sNewFile += OUStringLiteral1(sfx2::cTokenSeparator) + sSub;
    }

    m_SectionData.SetLinkFileName( sNewFile );

    if( !rFile.isEmpty() || !sSub.isEmpty() )
    {
        m_SectionData.SetType( FILE_LINK_SECTION );
    }
    else
    {
        m_SectionData.SetType( CONTENT_SECTION );
    }
}

void SectRepr::SetFilter( const OUString& rFilter )
{
    OUString sNewFile;
    const OUString sOldFileName( m_SectionData.GetLinkFileName() );
    const OUString sFile( sOldFileName.getToken( 0, sfx2::cTokenSeparator ) );
    const OUString sSub( sOldFileName.getToken( 2, sfx2::cTokenSeparator ) );

    if( !sFile.isEmpty() )
        sNewFile = sFile + OUStringLiteral1(sfx2::cTokenSeparator) +
                   rFilter + OUStringLiteral1(sfx2::cTokenSeparator) + sSub;
    else if( !sSub.isEmpty() )
        sNewFile = OUStringLiteral1(sfx2::cTokenSeparator) + OUStringLiteral1(sfx2::cTokenSeparator) + sSub;

    m_SectionData.SetLinkFileName( sNewFile );

    if( !sNewFile.isEmpty() )
    {
        m_SectionData.SetType( FILE_LINK_SECTION );
    }
}

void SectRepr::SetSubRegion(const OUString& rSubRegion)
{
    OUString sNewFile;
    sal_Int32 n(0);
    const OUString sLinkFileName(m_SectionData.GetLinkFileName());
    const OUString sOldFileName( sLinkFileName.getToken( 0, sfx2::cTokenSeparator, n ) );
    const OUString sFilter( sLinkFileName.getToken( 0, sfx2::cTokenSeparator, n ) );

    if( !rSubRegion.isEmpty() || !sOldFileName.isEmpty() )
        sNewFile = sOldFileName + OUStringLiteral1(sfx2::cTokenSeparator) +
                   sFilter + OUStringLiteral1(sfx2::cTokenSeparator) + rSubRegion;

    m_SectionData.SetLinkFileName( sNewFile );

    if( !rSubRegion.isEmpty() || !sOldFileName.isEmpty() )
    {
        m_SectionData.SetType( FILE_LINK_SECTION );
    }
    else
    {
        m_SectionData.SetType( CONTENT_SECTION );
    }
}

OUString SectRepr::GetFile() const
{
    const OUString sLinkFile( m_SectionData.GetLinkFileName() );

    if( sLinkFile.isEmpty() )
    {
        return sLinkFile;
    }
    if (DDE_LINK_SECTION == m_SectionData.GetType())
    {
        sal_Int32 n = 0;
        return sLinkFile.replaceFirst( OUStringLiteral1(sfx2::cTokenSeparator), " ", &n )
                        .replaceFirst( OUStringLiteral1(sfx2::cTokenSeparator), " ", &n );
    }
    return INetURLObject::decode( sLinkFile.getToken( 0, sfx2::cTokenSeparator ),
                                  INetURLObject::DecodeMechanism::Unambiguous );
}

OUString SectRepr::GetSubRegion() const
{
    const OUString sLinkFile( m_SectionData.GetLinkFileName() );
    if( !sLinkFile.isEmpty() )
        return sLinkFile.getToken( 2, sfx2::cTokenSeparator );
    return sLinkFile;
}

// dialog edit regions
SwEditRegionDlg::SwEditRegionDlg( vcl::Window* pParent, SwWrtShell& rWrtSh )
    : SfxModalDialog(pParent, "EditSectionDialog",
        "modules/swriter/ui/editsectiondialog.ui")
    , m_bSubRegionsFilled(false)
    , rSh(rWrtSh)
    , m_pDocInserter(nullptr)
    , bDontCheckPasswd(true)
{
    get(m_pCurName, "curname");
    get(m_pTree, "tree");
    m_pTree->set_height_request(m_pTree->GetTextHeight() * 16);
    get(m_pFileCB, "link");
    m_pFileCB->SetState(TRISTATE_FALSE);
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
    m_pProtectCB->SetState(TRISTATE_FALSE);
    get(m_pPasswdCB, "withpassword");
    get(m_pPasswdPB, "password");
    get(m_pHideCB, "hide");
    m_pHideCB->SetState(TRISTATE_FALSE);
    get(m_pConditionFT, "conditionft");
    get(m_pConditionED, "condition");
    // edit in readonly sections
    get(m_pEditInReadonlyCB, "editinro");
    m_pEditInReadonlyCB->SetState(TRISTATE_FALSE);
    get(m_pOptionsPB, "options");
    get(m_pDismiss, "remove");
    get(m_pOK, "ok");

    bool bWeb = dynamic_cast<SwWebDocShell*>( rSh.GetView().GetDocShell() ) != nullptr;

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

    m_pTree->SetSelectionMode( SelectionMode::Multiple );
    m_pTree->SetStyle(m_pTree->GetStyle()|WB_HASBUTTONSATROOT|WB_CLIPCHILDREN|WB_HSCROLL);
    m_pTree->SetSpaceBetweenEntries(0);
    m_pTree->SetAllEntriesAccessibleRoleType(SvTreeAccRoleType::TREE);

    if(bWeb)
    {
        m_pDDECB->Hide();
        get<VclContainer>("hideframe")->Hide();
        m_pPasswdCB->Hide();
    }

    m_pDDECB->SetClickHdl(LINK(this, SwEditRegionDlg, DDEHdl));

    pCurrSect = rSh.GetCurrSection();
    RecurseList( nullptr, nullptr );
    // if the cursor is not in a region
    // the first one will always be selected
    if( !m_pTree->FirstSelected() && m_pTree->First() )
        m_pTree->Select( m_pTree->First() );
    m_pTree->Show();
    bDontCheckPasswd = false;
}

bool SwEditRegionDlg::CheckPasswd(CheckBox* pBox)
{
    if(bDontCheckPasswd)
        return true;
    bool bRet = true;
    SvTreeListEntry* pEntry = m_pTree->FirstSelected();
    while( pEntry )
    {
        SectRepr* pRepr = static_cast<SectRepr*>(pEntry->GetUserData());
        if (!pRepr->GetTempPasswd().getLength()
            && pRepr->GetSectionData().GetPassword().getLength())
        {
            SfxPasswordDialog aPasswdDlg(GetFrameWeld());
            bRet = false;
            if (aPasswdDlg.execute())
            {
                const OUString sNewPasswd(aPasswdDlg.GetPassword());
                css::uno::Sequence <sal_Int8 > aNewPasswd;
                SvPasswordHelper::GetHashPassword( aNewPasswd, sNewPasswd );
                if (SvPasswordHelper::CompareHashPassword(
                        pRepr->GetSectionData().GetPassword(), sNewPasswd))
                {
                    pRepr->SetTempPasswd(aNewPasswd);
                    bRet = true;
                }
                else
                {
                    std::unique_ptr<weld::MessageDialog> xInfoBox(Application::CreateMessageDialog(GetFrameWeld(),
                                                                  VclMessageType::Info, VclButtonsType::Ok,
                                                                  SwResId(STR_WRONG_PASSWORD)));
                    xInfoBox->run();
                }
            }
        }
        pEntry = m_pTree->NextSelected(pEntry);
    }
    if(!bRet && pBox)
    {
        //reset old button state
        if(pBox->IsTriStateEnabled())
            pBox->SetState(pBox->IsChecked() ? TRISTATE_FALSE : TRISTATE_INDET);
        else
            pBox->Check(!pBox->IsChecked());
    }

    return bRet;
}

// recursively look for child-sections
void SwEditRegionDlg::RecurseList( const SwSectionFormat* pFormat, SvTreeListEntry* pEntry )
{
    SvTreeListEntry* pSelEntry = nullptr;
    if (!pFormat)
    {
        const size_t nCount=rSh.GetSectionFormatCount();
        for ( size_t n = 0; n < nCount; n++ )
        {
            SectionType eTmpType;
            if( !( pFormat = &rSh.GetSectionFormat(n))->GetParent() &&
                pFormat->IsInNodesArr() &&
                (eTmpType = pFormat->GetSection()->GetType()) != TOX_CONTENT_SECTION
                && TOX_HEADER_SECTION != eTmpType )
            {
                SwSection *pSect = pFormat->GetSection();
                SectRepr* pSectRepr = new SectRepr( n, *pSect );
                Image aImg = BuildBitmap( pSect->IsProtect(),pSect->IsHidden());
                pEntry = m_pTree->InsertEntry(pSect->GetSectionName(), aImg, aImg);
                pEntry->SetUserData(pSectRepr);
                RecurseList( pFormat, pEntry );
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
        pFormat->GetChildSections(aTmpArr, SectionSort::Pos);
        if( !aTmpArr.empty() )
        {
            for( const auto pSect : aTmpArr )
            {
                SectionType eTmpType;
                pFormat = pSect->GetFormat();
                if( pFormat->IsInNodesArr() &&
                    (eTmpType = pFormat->GetSection()->GetType()) != TOX_CONTENT_SECTION
                    && TOX_HEADER_SECTION != eTmpType )
                {
                    SectRepr* pSectRepr=new SectRepr(
                                    FindArrPos( pSect->GetFormat() ), *pSect );
                    Image aImage = BuildBitmap( pSect->IsProtect(),
                                            pSect->IsHidden());
                    pNEntry = m_pTree->InsertEntry(
                        pSect->GetSectionName(), aImage, aImage, pEntry);
                    pNEntry->SetUserData(pSectRepr);
                    RecurseList( pSect->GetFormat(), pNEntry );
                    if( pNEntry->HasChildren())
                        m_pTree->Expand(pNEntry);
                    if (pCurrSect==pSect)
                        pSelEntry = pNEntry;
                }
            }
        }
    }
    if(nullptr != pSelEntry)
    {
        m_pTree->MakeVisible(pSelEntry);
        m_pTree->Select(pSelEntry);
    }
}

size_t SwEditRegionDlg::FindArrPos(const SwSectionFormat* pFormat )
{
    const size_t nCount=rSh.GetSectionFormatCount();
    for ( size_t i = 0; i < nCount; i++ )
        if ( pFormat == &rSh.GetSectionFormat(i) )
            return i;

    OSL_FAIL("SectionFormat not on the list" );
    return SIZE_MAX;
}

SwEditRegionDlg::~SwEditRegionDlg( )
{
    disposeOnce();
}

void SwEditRegionDlg::dispose()
{
    SvTreeListEntry* pEntry = m_pTree->First();
    while( pEntry )
    {
        delete static_cast<SectRepr*>(pEntry->GetUserData());
        pEntry = m_pTree->Next( pEntry );
    }

    delete m_pDocInserter;
    m_pCurName.clear();
    m_pTree.clear();
    m_pFileCB.clear();
    m_pDDECB.clear();
    m_pDDEFrame.clear();
    m_pFileNameFT.clear();
    m_pDDECommandFT.clear();
    m_pFileNameED.clear();
    m_pFilePB.clear();
    m_pSubRegionFT.clear();
    m_pSubRegionED.clear();
    m_pProtectCB.clear();
    m_pPasswdCB.clear();
    m_pPasswdPB.clear();
    m_pHideCB.clear();
    m_pConditionFT.clear();
    m_pConditionED.clear();
    m_pEditInReadonlyCB.clear();
    m_pOK.clear();
    m_pOptionsPB.clear();
    m_pDismiss.clear();
    SfxModalDialog::dispose();
}

void    SwEditRegionDlg::SelectSection(const OUString& rSectionName)
{
    SvTreeListEntry* pEntry = m_pTree->First();
    while(pEntry)
    {
        SectRepr* pRepr = static_cast<SectRepr*>(pEntry->GetUserData());
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

// selected entry in TreeListBox is showed in Edit window in case of
// multiselection some controls are disabled
IMPL_LINK( SwEditRegionDlg, GetFirstEntryHdl, SvTreeListBox *, pBox, void )
{
    bDontCheckPasswd = true;
    SvTreeListEntry* pEntry=pBox->FirstSelected();
    m_pHideCB->Enable();
    // edit in readonly sections
    m_pEditInReadonlyCB->Enable();

    m_pProtectCB->Enable();
    m_pFileCB->Enable();
    css::uno::Sequence <sal_Int8> aCurPasswd;
    if( 1 < pBox->GetSelectionCount() )
    {
        m_pHideCB->EnableTriState();
        m_pProtectCB->EnableTriState();
        // edit in readonly sections
        m_pEditInReadonlyCB->EnableTriState();

        m_pFileCB->EnableTriState();

        bool bHiddenValid       = true;
        bool bProtectValid      = true;
        bool bConditionValid    = true;
        // edit in readonly sections
        bool bEditInReadonlyValid = true;
        bool bEditInReadonly    = true;

        bool bHidden            = true;
        bool bProtect           = true;
        OUString sCondition;
        bool bFirst             = true;
        bool bFileValid         = true;
        bool bFile              = true;
        bool bPasswdValid       = true;

        while( pEntry )
        {
            SectRepr* pRepr=static_cast<SectRepr*>(pEntry->GetUserData());
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
                if(sCondition != rData.GetCondition())
                    bConditionValid = false;
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

        m_pHideCB->SetState(!bHiddenValid ? TRISTATE_INDET :
                    bHidden ? TRISTATE_TRUE : TRISTATE_FALSE);
        m_pProtectCB->SetState(!bProtectValid ? TRISTATE_INDET :
                    bProtect ? TRISTATE_TRUE : TRISTATE_FALSE);
        // edit in readonly sections
        m_pEditInReadonlyCB->SetState(!bEditInReadonlyValid ? TRISTATE_INDET :
                    bEditInReadonly ? TRISTATE_TRUE : TRISTATE_FALSE);

        m_pFileCB->SetState(!bFileValid ? TRISTATE_INDET :
                    bFile ? TRISTATE_TRUE : TRISTATE_FALSE);

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
        bool bPasswdEnabled = m_pProtectCB->GetState() == TRISTATE_TRUE;
        m_pPasswdCB->Enable(bPasswdEnabled);
        m_pPasswdPB->Enable(bPasswdEnabled);
        if(!bPasswdValid)
        {
            pEntry = pBox->FirstSelected();
            pBox->SelectAll( false );
            pBox->Select( pEntry );
            GetFirstEntryHdl(pBox);
            return;
        }
        else
            m_pPasswdCB->Check(aCurPasswd.getLength() > 0);
    }
    else if (pEntry )
    {
        m_pCurName->Enable();
        m_pOptionsPB->Enable();
        SectRepr* pRepr=static_cast<SectRepr*>(pEntry->GetUserData());
        SwSectionData const& rData( pRepr->GetSectionData() );
        m_pConditionED->SetText(rData.GetCondition());
        m_pHideCB->Enable();
        m_pHideCB->SetState((rData.IsHidden()) ? TRISTATE_TRUE : TRISTATE_FALSE);
        bool bHide = TRISTATE_TRUE == m_pHideCB->GetState();
        m_pConditionED->Enable(bHide);
        m_pConditionFT->Enable(bHide);
        m_pPasswdCB->Check(rData.GetPassword().getLength() > 0);

        m_pOK->Enable();
        m_pPasswdCB->Enable();
        m_pCurName->SetText(pBox->GetEntryText(pEntry));
        m_pCurName->Enable();
        m_pDismiss->Enable();
        const OUString aFile = pRepr->GetFile();
        const OUString sSub = pRepr->GetSubRegion();
        m_bSubRegionsFilled = false;
        m_pSubRegionED->Clear();
        if( !aFile.isEmpty() || !sSub.isEmpty() )
        {
            m_pFileCB->Check();
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
                ? TRISTATE_TRUE : TRISTATE_FALSE);
        m_pProtectCB->Enable();

        // edit in readonly sections
        m_pEditInReadonlyCB->SetState((rData.IsEditInReadonlyFlag())
                ? TRISTATE_TRUE : TRISTATE_FALSE);
        m_pEditInReadonlyCB->Enable();

        bool bPasswdEnabled = m_pProtectCB->IsChecked();
        m_pPasswdCB->Enable(bPasswdEnabled);
        m_pPasswdPB->Enable(bPasswdEnabled);
    }
    bDontCheckPasswd = false;
}

IMPL_LINK( SwEditRegionDlg, DeselectHdl, SvTreeListBox *, pBox, void )
{
    if( pBox->GetSelectionCount() )
        return;

    m_pHideCB->Enable(false);
    m_pProtectCB->Enable(false);
    // edit in readonly sections
    m_pEditInReadonlyCB->Enable(false);

    m_pPasswdCB->Enable(false);
    m_pConditionFT->Enable(false);
    m_pConditionED->Enable(false);
    m_pFileCB->Enable(false);
    m_pDDEFrame->Enable(false);
    m_pDDECB->Enable(false);
    m_pCurName->Enable(false);

    UseFileHdl(m_pFileCB);
    DDEHdl(m_pDDECB);

}

// in OkHdl the modified settings are being applied and reversed regions are deleted
IMPL_LINK_NOARG(SwEditRegionDlg, OkHdl, Button*, void)
{
    // temp. Array because during changing of a region the position
    // inside of the "Core-Arrays" can be shifted:
    //  - at linked regions, when they have more SubRegions or get
    //    new ones.
    // StartUndo must certainly also happen not before the formats
    // are copied (ClearRedo!)

    const SwSectionFormats& rDocFormats = rSh.GetDoc()->GetSections();
    SwSectionFormats aOrigArray(rDocFormats);

    rSh.StartAllAction();
    rSh.StartUndo();
    rSh.ResetSelect( nullptr,false );
    SvTreeListEntry* pEntry = m_pTree->First();

    while( pEntry )
    {
        SectRepr* pRepr = static_cast<SectRepr*>(pEntry->GetUserData());
        SwSectionFormat* pFormat = aOrigArray[ pRepr->GetArrPos() ];
        if (!pRepr->GetSectionData().IsProtectFlag())
        {
            pRepr->GetSectionData().SetPassword(uno::Sequence<sal_Int8 >());
        }
        size_t nNewPos = rDocFormats.GetPos(pFormat);
        if ( SIZE_MAX != nNewPos )
        {
            std::unique_ptr<SfxItemSet> pSet(pFormat->GetAttrSet().Clone( false ));
            if( pFormat->GetCol() != pRepr->GetCol() )
                pSet->Put( pRepr->GetCol() );

            SvxBrushItem aBrush(pFormat->makeBackgroundBrushItem(false));
            if( aBrush != pRepr->GetBackground() )
                pSet->Put( pRepr->GetBackground() );

            if( pFormat->GetFootnoteAtTextEnd(false) != pRepr->GetFootnoteNtAtEnd() )
                pSet->Put( pRepr->GetFootnoteNtAtEnd() );

            if( pFormat->GetEndAtTextEnd(false) != pRepr->GetEndNtAtEnd() )
                pSet->Put( pRepr->GetEndNtAtEnd() );

            if( pFormat->GetBalancedColumns() != pRepr->GetBalance() )
                pSet->Put( pRepr->GetBalance() );

            if( pFormat->GetFrameDir() != pRepr->GetFrameDir() )
                pSet->Put( pRepr->GetFrameDir() );

            if( pFormat->GetLRSpace() != pRepr->GetLRSpace())
                pSet->Put( pRepr->GetLRSpace());

            rSh.UpdateSection( nNewPos, pRepr->GetSectionData(),
                               pSet->Count() ? pSet.get() : nullptr );
        }
        pEntry = m_pTree->Next( pEntry );
    }

    for (SectReprs_t::reverse_iterator it = m_SectReprs.rbegin(), aEnd = m_SectReprs.rend(); it != aEnd; ++it)
    {
        assert(it->first == it->second->GetArrPos());
        SwSectionFormat* pFormat = aOrigArray[ it->second->GetArrPos() ];
        const size_t nNewPos = rDocFormats.GetPos( pFormat );
        if( SIZE_MAX != nNewPos )
            rSh.DelSectionFormat( nNewPos );
    }

    aOrigArray.clear();

    // EndDialog must be called ahead of EndAction's end,
    // otherwise ScrollError can occur.
    EndDialog(RET_OK);

    rSh.EndUndo();
    rSh.EndAllAction();
}

// Toggle protect
IMPL_LINK( SwEditRegionDlg, ChangeProtectHdl, Button *, pButton, void )
{
    TriStateBox* pBox = static_cast<TriStateBox*>(pButton);
    if(!CheckPasswd(pBox))
        return;
    pBox->EnableTriState(false);
    SvTreeListEntry* pEntry = m_pTree->FirstSelected();
    OSL_ENSURE(pEntry,"no entry found");
    bool bCheck = TRISTATE_TRUE == pBox->GetState();
    while( pEntry )
    {
        SectRepr* pRepr = static_cast<SectRepr*>(pEntry->GetUserData());
        pRepr->GetSectionData().SetProtectFlag(bCheck);
        Image aImage = BuildBitmap(bCheck,
                                   TRISTATE_TRUE == m_pHideCB->GetState());
        m_pTree->SetExpandedEntryBmp(  pEntry, aImage );
        m_pTree->SetCollapsedEntryBmp( pEntry, aImage );
        pEntry = m_pTree->NextSelected(pEntry);
    }
    m_pPasswdCB->Enable(bCheck);
    m_pPasswdPB->Enable(bCheck);
}

// Toggle hide
IMPL_LINK( SwEditRegionDlg, ChangeHideHdl, Button *, pButton, void )
{
    TriStateBox* pBox = static_cast<TriStateBox*>(pButton);
    if(!CheckPasswd(pBox))
        return;
    pBox->EnableTriState(false);
    SvTreeListEntry* pEntry = m_pTree->FirstSelected();
    OSL_ENSURE(pEntry,"no entry found");
    while( pEntry )
    {
        SectRepr* pRepr = static_cast<SectRepr*>(pEntry->GetUserData());
        pRepr->GetSectionData().SetHidden(TRISTATE_TRUE == pBox->GetState());

        Image aImage = BuildBitmap(TRISTATE_TRUE == m_pProtectCB->GetState(),
                                    TRISTATE_TRUE == pBox->GetState());
        m_pTree->SetExpandedEntryBmp(  pEntry, aImage );
        m_pTree->SetCollapsedEntryBmp( pEntry, aImage );

        pEntry = m_pTree->NextSelected(pEntry);
    }

    bool bHide = TRISTATE_TRUE == pBox->GetState();
    m_pConditionED->Enable(bHide);
    m_pConditionFT->Enable(bHide);
}

// Toggle edit in readonly
IMPL_LINK( SwEditRegionDlg, ChangeEditInReadonlyHdl, Button *, pButton, void )
{
    TriStateBox* pBox = static_cast<TriStateBox*>(pButton);
    if(!CheckPasswd(pBox))
        return;
    pBox->EnableTriState(false);
    SvTreeListEntry* pEntry = m_pTree->FirstSelected();
    OSL_ENSURE(pEntry,"no entry found");
    while( pEntry )
    {
        SectRepr* pRepr = static_cast<SectRepr*>(pEntry->GetUserData());
        pRepr->GetSectionData().SetEditInReadonlyFlag(
                TRISTATE_TRUE == pBox->GetState());
        pEntry = m_pTree->NextSelected(pEntry);
    }
}

// clear selected region
IMPL_LINK_NOARG(SwEditRegionDlg, ChangeDismissHdl, Button*, void)
{
    if(!CheckPasswd())
        return;
    SvTreeListEntry* pEntry = m_pTree->FirstSelected();
    SvTreeListEntry* pChild;
    SvTreeListEntry* pParent;
    // at first mark all selected
    while(pEntry)
    {
        SectRepr* const pSectRepr = static_cast<SectRepr*>(pEntry->GetUserData());
        pSectRepr->SetSelected();
        pEntry = m_pTree->NextSelected(pEntry);
    }
    pEntry = m_pTree->FirstSelected();
    // then delete
    while(pEntry)
    {
        SectRepr* const pSectRepr = static_cast<SectRepr*>(pEntry->GetUserData());
        SvTreeListEntry* pRemove = nullptr;
        bool bRestart = false;
        if(pSectRepr->IsSelected())
        {
            m_SectReprs.insert(std::make_pair(pSectRepr->GetArrPos(),
                        std::unique_ptr<SectRepr>(pSectRepr)));
            while( (pChild = m_pTree->FirstChild(pEntry) )!= nullptr )
            {
                // because of the repositioning we have to start at the beginning again
                bRestart = true;
                pParent = m_pTree->GetParent(pEntry);
                m_pTree->GetModel()->Move(pChild, pParent, SvTreeList::GetRelPos(pEntry));
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

    if ( m_pTree->FirstSelected() )
        return;

    m_pConditionFT->Enable(false);
    m_pConditionED->Enable(false);
    m_pDismiss->       Enable(false);
    m_pCurName->Enable(false);
    m_pProtectCB->Enable(false);
    m_pPasswdCB->Enable(false);
    m_pHideCB->Enable(false);
    // edit in readonly sections
    m_pEditInReadonlyCB->Enable(false);
    m_pEditInReadonlyCB->SetState(TRISTATE_FALSE);
    m_pProtectCB->SetState(TRISTATE_FALSE);
    m_pPasswdCB->Check(false);
    m_pHideCB->SetState(TRISTATE_FALSE);
    m_pFileCB->Check(false);
    // otherwise the focus would be on HelpButton
    m_pOK->GrabFocus();
    UseFileHdl(m_pFileCB);

}

// link CheckBox to file?
IMPL_LINK( SwEditRegionDlg, UseFileHdl, Button *, pButton, void )
{
    CheckBox* pBox = static_cast<CheckBox*>(pButton);
    if(!CheckPasswd(pBox))
        return;
    SvTreeListEntry* pEntry = m_pTree->FirstSelected();
    pBox->EnableTriState(false);
    bool bMulti = 1 < m_pTree->GetSelectionCount();
    bool bFile = pBox->IsChecked();
    if(pEntry)
    {
        while(pEntry)
        {
            SectRepr* const pSectRepr = static_cast<SectRepr*>(pEntry->GetUserData());
            bool bContent = pSectRepr->IsContent();
            if( pBox->IsChecked() && bContent && rSh.HasSelection() )
            {
                std::unique_ptr<weld::MessageDialog> xQueryBox(Application::CreateMessageDialog(GetFrameWeld(),
                                                               VclMessageType::Question, VclButtonsType::YesNo,
                                                               SwResId(STR_QUERY_CONNECT)));
                if (RET_NO == xQueryBox->run())
                    pBox->Check( false );
            }
            if( bFile )
                pSectRepr->SetContent(false);
            else
            {
                pSectRepr->SetFile(aEmptyOUStr);
                pSectRepr->SetSubRegion(aEmptyOUStr);
                pSectRepr->GetSectionData().SetLinkFilePassword(aEmptyOUStr);
            }

            pEntry = m_pTree->NextSelected(pEntry);
        }
        m_pDDECB->Enable(bFile && ! bMulti);
        m_pDDEFrame->Enable(bFile && ! bMulti);
        if( bFile )
        {
            m_pProtectCB->SetState(TRISTATE_TRUE);
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
}

// call dialog paste file
IMPL_LINK_NOARG(SwEditRegionDlg, FileSearchHdl, Button*, void)
{
    if(!CheckPasswd())
        return;
    delete m_pDocInserter;
    m_pDocInserter = new ::sfx2::DocumentInserter(GetFrameWeld(), "swriter");
    m_pDocInserter->StartExecuteModal( LINK( this, SwEditRegionDlg, DlgClosedHdl ) );
}

IMPL_LINK_NOARG(SwEditRegionDlg, OptionsHdl, Button*, void)
{
    if(!CheckPasswd())
        return;
    SvTreeListEntry* pEntry = m_pTree->FirstSelected();

    if(!pEntry)
        return;

    SectRepr* pSectRepr = static_cast<SectRepr*>(pEntry->GetUserData());
    SfxItemSet aSet(
        rSh.GetView().GetPool(),
        svl::Items<
            RES_FRM_SIZE, RES_FRM_SIZE,
            RES_LR_SPACE, RES_LR_SPACE,
            RES_BACKGROUND, RES_BACKGROUND,
            RES_COL, RES_COL,
            RES_FTN_AT_TXTEND, RES_FRAMEDIR,
            XATTR_FILL_FIRST, XATTR_FILL_LAST,
            SID_ATTR_PAGE_SIZE, SID_ATTR_PAGE_SIZE>{});

    aSet.Put( pSectRepr->GetCol() );
    aSet.Put( pSectRepr->GetBackground() );
    aSet.Put( pSectRepr->GetFootnoteNtAtEnd() );
    aSet.Put( pSectRepr->GetEndNtAtEnd() );
    aSet.Put( pSectRepr->GetBalance() );
    aSet.Put( pSectRepr->GetFrameDir() );
    aSet.Put( pSectRepr->GetLRSpace() );

    const SwSectionFormats& rDocFormats = rSh.GetDoc()->GetSections();
    SwSectionFormats aOrigArray(rDocFormats);

    SwSectionFormat* pFormat = aOrigArray[pSectRepr->GetArrPos()];
    long nWidth = rSh.GetSectionWidth(*pFormat);
    aOrigArray.clear();
    if (!nWidth)
        nWidth = USHRT_MAX;

    aSet.Put(SwFormatFrameSize(ATT_VAR_SIZE, nWidth));
    aSet.Put(SvxSizeItem(SID_ATTR_PAGE_SIZE, Size(nWidth, nWidth)));

    ScopedVclPtrInstance< SwSectionPropertyTabDialog > aTabDlg(this, aSet, rSh);
    if(RET_OK == aTabDlg->Execute())
    {
        const SfxItemSet* pOutSet = aTabDlg->GetOutputItemSet();
        if( pOutSet && pOutSet->Count() )
        {
            const SfxPoolItem *pColItem, *pBrushItem,
                              *pFootnoteItem, *pEndItem, *pBalanceItem,
                              *pFrameDirItem, *pLRSpaceItem;
            SfxItemState eColState = pOutSet->GetItemState(
                                    RES_COL, false, &pColItem );
            SfxItemState eBrushState = pOutSet->GetItemState(
                                    RES_BACKGROUND, false, &pBrushItem );
            SfxItemState eFootnoteState = pOutSet->GetItemState(
                                    RES_FTN_AT_TXTEND, false, &pFootnoteItem );
            SfxItemState eEndState = pOutSet->GetItemState(
                                    RES_END_AT_TXTEND, false, &pEndItem );
            SfxItemState eBalanceState = pOutSet->GetItemState(
                                    RES_COLUMNBALANCE, false, &pBalanceItem );
            SfxItemState eFrameDirState = pOutSet->GetItemState(
                                    RES_FRAMEDIR, false, &pFrameDirItem );
            SfxItemState eLRState = pOutSet->GetItemState(
                                    RES_LR_SPACE, false, &pLRSpaceItem);

            if( SfxItemState::SET == eColState ||
                SfxItemState::SET == eBrushState ||
                SfxItemState::SET == eFootnoteState ||
                SfxItemState::SET == eEndState ||
                SfxItemState::SET == eBalanceState||
                SfxItemState::SET == eFrameDirState||
                SfxItemState::SET == eLRState)
            {
                SvTreeListEntry* pSelEntry = m_pTree->FirstSelected();
                while( pSelEntry )
                {
                    SectRepr* pRepr = static_cast<SectRepr*>(pSelEntry->GetUserData());
                    if( SfxItemState::SET == eColState )
                        pRepr->GetCol() = *static_cast<const SwFormatCol*>(pColItem);
                    if( SfxItemState::SET == eBrushState )
                        pRepr->GetBackground() = *static_cast<const SvxBrushItem*>(pBrushItem);
                    if( SfxItemState::SET == eFootnoteState )
                        pRepr->GetFootnoteNtAtEnd() = *static_cast<const SwFormatFootnoteAtTextEnd*>(pFootnoteItem);
                    if( SfxItemState::SET == eEndState )
                        pRepr->GetEndNtAtEnd() = *static_cast<const SwFormatEndAtTextEnd*>(pEndItem);
                    if( SfxItemState::SET == eBalanceState )
                        pRepr->GetBalance().SetValue(static_cast<const SwFormatNoBalancedColumns*>(pBalanceItem)->GetValue());
                    if( SfxItemState::SET == eFrameDirState )
                        pRepr->GetFrameDir().SetValue(static_cast<const SvxFrameDirectionItem*>(pFrameDirItem)->GetValue());
                    if( SfxItemState::SET == eLRState )
                        pRepr->GetLRSpace() = *static_cast<const SvxLRSpaceItem*>(pLRSpaceItem);

                    pSelEntry = m_pTree->NextSelected(pSelEntry);
                }
            }
        }
    }

}

// Applying of the filename or the linked region
IMPL_LINK( SwEditRegionDlg, FileNameHdl, Edit&, rEdit, void )
{
    Selection aSelect = rEdit.GetSelection();
    if(!CheckPasswd())
        return;
    rEdit.SetSelection(aSelect);
    SvTreeListEntry* pEntry = m_pTree->FirstSelected();
    OSL_ENSURE(pEntry,"no entry found");
    SectRepr* pSectRepr = static_cast<SectRepr*>(pEntry->GetUserData());
    if (&rEdit == m_pFileNameED)
    {
        m_bSubRegionsFilled = false;
        m_pSubRegionED->Clear();
        if (m_pDDECB->IsChecked())
        {
            OUString sLink( SwSectionData::CollapseWhiteSpaces(rEdit.GetText()) );
            sal_Int32 nPos = 0;
            sLink = sLink.replaceFirst( " ", OUStringLiteral1(sfx2::cTokenSeparator), &nPos );
            if (nPos>=0)
            {
                sLink = sLink.replaceFirst( " ", OUStringLiteral1(sfx2::cTokenSeparator), &nPos );
            }

            pSectRepr->GetSectionData().SetLinkFileName( sLink );
            pSectRepr->GetSectionData().SetType( DDE_LINK_SECTION );
        }
        else
        {
            OUString sTmp(rEdit.GetText());
            if(!sTmp.isEmpty())
            {
                SfxMedium* pMedium = rSh.GetView().GetDocShell()->GetMedium();
                INetURLObject aAbs;
                if( pMedium )
                    aAbs = pMedium->GetURLObject();
                sTmp = URIHelper::SmartRel2Abs(
                    aAbs, sTmp, URIHelper::GetMaybeFileHdl() );
            }
            pSectRepr->SetFile( sTmp );
            pSectRepr->GetSectionData().SetLinkFilePassword( aEmptyOUStr );
        }
    }
    else
    {
        pSectRepr->SetSubRegion( rEdit.GetText() );
    }
}

IMPL_LINK( SwEditRegionDlg, DDEHdl, Button*, pButton, void )
{
    CheckBox* pBox = static_cast<CheckBox*>(pButton);
    if(!CheckPasswd(pBox))
        return;
    SvTreeListEntry* pEntry = m_pTree->FirstSelected();
    if(pEntry)
    {
        bool bFile = m_pFileCB->IsChecked();
        SectRepr* pSectRepr = static_cast<SectRepr*>(pEntry->GetUserData());
        SwSectionData & rData( pSectRepr->GetSectionData() );
        bool bDDE = pBox->IsChecked();
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
}

IMPL_LINK( SwEditRegionDlg, ChangePasswdHdl, Button *, pBox, void )
{
    bool bChange = pBox == m_pPasswdPB;
    if(!CheckPasswd())
    {
        if(!bChange)
            m_pPasswdCB->Check(!m_pPasswdCB->IsChecked());
        return;
    }
    SvTreeListEntry* pEntry = m_pTree->FirstSelected();
    bool bSet = bChange ? bChange : m_pPasswdCB->IsChecked();
    OSL_ENSURE(pEntry,"no entry found");
    while( pEntry )
    {
        SectRepr* pRepr = static_cast<SectRepr*>(pEntry->GetUserData());
        if(bSet)
        {
            if(!pRepr->GetTempPasswd().getLength() || bChange)
            {
                SfxPasswordDialog aPasswdDlg(GetFrameWeld());
                aPasswdDlg.ShowExtras(SfxShowExtras::CONFIRM);
                if (RET_OK == aPasswdDlg.execute())
                {
                    const OUString sNewPasswd(aPasswdDlg.GetPassword());
                    if (aPasswdDlg.GetConfirm() == sNewPasswd)
                    {
                        SvPasswordHelper::GetHashPassword( pRepr->GetTempPasswd(), sNewPasswd );
                    }
                    else
                    {
                        std::unique_ptr<weld::MessageDialog> xInfoBox(Application::CreateMessageDialog(pBox->GetFrameWeld(),
                                                                      VclMessageType::Info, VclButtonsType::Ok,
                                                                      SwResId(STR_WRONG_PASSWD_REPEAT)));
                        xInfoBox->run();
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
}

// the current region name is being added to the TreeListBox immediately during
// editing, with empty string no Ok()
IMPL_LINK_NOARG(SwEditRegionDlg, NameEditHdl, Edit&, void)
{
    if(!CheckPasswd())
        return;
    SvTreeListEntry* pEntry = m_pTree->FirstSelected();
    OSL_ENSURE(pEntry,"no entry found");
    if (pEntry)
    {
        const OUString aName = m_pCurName->GetText();
        m_pTree->SetEntryText(pEntry,aName);
        SectRepr* pRepr = static_cast<SectRepr*>(pEntry->GetUserData());
        pRepr->GetSectionData().SetSectionName(aName);

        m_pOK->Enable(!aName.isEmpty());
    }
}

IMPL_LINK( SwEditRegionDlg, ConditionEditHdl, Edit&, rEdit, void )
{
    Selection aSelect = rEdit.GetSelection();
    if(!CheckPasswd())
        return;
    rEdit.SetSelection(aSelect);
    SvTreeListEntry* pEntry = m_pTree->FirstSelected();
    OSL_ENSURE(pEntry,"no entry found");
    while( pEntry )
    {
        SectRepr* pRepr = static_cast<SectRepr*>(pEntry->GetUserData());
        pRepr->GetSectionData().SetCondition(rEdit.GetText());
        pEntry = m_pTree->NextSelected(pEntry);
    }
}

IMPL_LINK( SwEditRegionDlg, DlgClosedHdl, sfx2::FileDialogHelper *, _pFileDlg, void )
{
    OUString sFileName, sFilterName, sPassword;
    if ( _pFileDlg->GetError() == ERRCODE_NONE )
    {
        std::unique_ptr<SfxMedium> pMedium(m_pDocInserter->CreateMedium("sglobal"));
        if ( pMedium )
        {
            sFileName = pMedium->GetURLObject().GetMainURL( INetURLObject::DecodeMechanism::NONE );
            sFilterName = pMedium->GetFilter()->GetFilterName();
            const SfxPoolItem* pItem;
            if ( SfxItemState::SET == pMedium->GetItemSet()->GetItemState( SID_PASSWORD, false, &pItem ) )
                sPassword = static_cast<const SfxStringItem*>(pItem )->GetValue();
            ::lcl_ReadSections(*pMedium, *m_pSubRegionED);
        }
    }

    SvTreeListEntry* pEntry = m_pTree->FirstSelected();
    OSL_ENSURE( pEntry, "no entry found" );
    if ( pEntry )
    {
        SectRepr* pSectRepr = static_cast<SectRepr*>(pEntry->GetUserData());
        pSectRepr->SetFile( sFileName );
        pSectRepr->SetFilter( sFilterName );
        pSectRepr->GetSectionData().SetLinkFilePassword(sPassword);
        m_pFileNameED->SetText(pSectRepr->GetFile());
    }
}

IMPL_LINK( SwEditRegionDlg, SubRegionEventHdl, VclWindowEvent&, rEvent, void )
{
    if( !m_bSubRegionsFilled && rEvent.GetId() == VclEventId::DropdownPreOpen )
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
            SfxMedium aMedium( sFileName, StreamMode::STD_READ );
            sFileName = aMedium.GetURLObject().GetMainURL( INetURLObject::DecodeMechanism::NONE );
            ::lcl_ReadSections(aMedium, *m_pSubRegionED);
        }
        else
            lcl_FillSubRegionList(rSh, *m_pSubRegionED, nullptr);
        m_bSubRegionsFilled = true;
    }
}

// helper function - read section names from medium
static void lcl_ReadSections( SfxMedium& rMedium, ComboBox& rBox )
{
    rBox.Clear();
    uno::Reference < embed::XStorage > xStg;
    if( rMedium.IsStorage() && (xStg = rMedium.GetStorage()).is() )
    {
        std::vector<OUString> aArr;
        SotClipboardFormatId nFormat = SotStorage::GetFormatID( xStg );
        if ( nFormat == SotClipboardFormatId::STARWRITER_60 || nFormat == SotClipboardFormatId::STARWRITERGLOB_60 ||
            nFormat == SotClipboardFormatId::STARWRITER_8 || nFormat == SotClipboardFormatId::STARWRITERGLOB_8)
            SwGetReaderXML()->GetSectionList( rMedium, aArr );

        for (auto const& it : aArr)
        {
            rBox.InsertEntry(it);
        }
    }
}

SwInsertSectionTabDialog::SwInsertSectionTabDialog(
            vcl::Window* pParent, const SfxItemSet& rSet, SwWrtShell& rSh)
    : SfxTabDialog(pParent, "InsertSectionDialog",
        "modules/swriter/ui/insertsectiondialog.ui", &rSet)
    , rWrtSh(rSh)
{
    SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();
    m_nSectionPageId = AddTabPage("section", SwInsertSectionTabPage::Create, nullptr);
    m_nColumnPageId = AddTabPage("columns",   SwColumnPage::Create,    nullptr);
    m_nBackPageId = AddTabPage("background", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BKG ), nullptr);
    m_nNotePageId = AddTabPage("notes", SwSectionFootnoteEndTabPage::Create, nullptr);
    m_nIndentPage = AddTabPage("indents", SwSectionIndentTabPage::Create, nullptr);

    SvxHtmlOptions& rHtmlOpt = SvxHtmlOptions::Get();
    long nHtmlMode = rHtmlOpt.GetExportMode();

    bool bWeb = dynamic_cast<SwWebDocShell*>( rSh.GetView().GetDocShell()  ) != nullptr ;
    if(bWeb)
    {
        RemoveTabPage(m_nNotePageId);
        RemoveTabPage(m_nIndentPage);
        if( HTML_CFG_NS40 != nHtmlMode && HTML_CFG_WRITER != nHtmlMode)
            RemoveTabPage(m_nColumnPageId);
    }
    SetCurPageId(m_nSectionPageId);
}

SwInsertSectionTabDialog::~SwInsertSectionTabDialog()
{
}

void SwInsertSectionTabDialog::PageCreated( sal_uInt16 nId, SfxTabPage &rPage )
{
    if (nId == m_nSectionPageId)
        static_cast<SwInsertSectionTabPage&>(rPage).SetWrtShell(rWrtSh);
    else if (nId == m_nBackPageId)
    {
            SfxAllItemSet aSet(*(GetInputSetImpl()->GetPool()));
            aSet.Put (SfxUInt32Item(SID_FLAG_TYPE, static_cast<sal_uInt32>(SvxBackgroundTabFlags::SHOW_SELECTOR)));
            rPage.PageCreated(aSet);
    }
    else if (nId == m_nColumnPageId)
    {
        const SwFormatFrameSize& rSize = GetInputSetImpl()->Get(RES_FRM_SIZE);
        static_cast<SwColumnPage&>(rPage).SetPageWidth(rSize.GetWidth());
        static_cast<SwColumnPage&>(rPage).ShowBalance(true);
        static_cast<SwColumnPage&>(rPage).SetInSection(true);
    }
    else if (nId == m_nIndentPage)
        static_cast<SwSectionIndentTabPage&>(rPage).SetWrtShell(rWrtSh);
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
    SfxViewFrame* pViewFrame = rWrtSh.GetView().GetViewFrame();
    uno::Reference< frame::XDispatchRecorder > xRecorder =
            pViewFrame->GetBindings().GetRecorder();
    if ( xRecorder.is() )
    {
        SfxRequest aRequest( pViewFrame, FN_INSERT_REGION);
        const SfxPoolItem* pCol;
        if(SfxItemState::SET == pOutputItemSet->GetItemState(RES_COL, false, &pCol))
        {
            aRequest.AppendItem(SfxUInt16Item(SID_ATTR_COLUMNS,
                static_cast<const SwFormatCol*>(pCol)->GetColumns().size()));
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

        const OUString sLinkFileName( m_pSectionData->GetLinkFileName() );
        sal_Int32 n = 0;
        aRequest.AppendItem(SfxStringItem( FN_PARAM_1, sLinkFileName.getToken( 0, sfx2::cTokenSeparator, n )));
        aRequest.AppendItem(SfxStringItem( FN_PARAM_2, sLinkFileName.getToken( 0, sfx2::cTokenSeparator, n )));
        aRequest.AppendItem(SfxStringItem( FN_PARAM_3, sLinkFileName.getToken( 0, sfx2::cTokenSeparator, n )));
        aRequest.Done();
    }
    return nRet;
}

SwInsertSectionTabPage::SwInsertSectionTabPage(
                            vcl::Window *pParent, const SfxItemSet &rAttrSet)
    : SfxTabPage(pParent, "SectionPage",
        "modules/swriter/ui/sectionpage.ui", &rAttrSet)
    , m_pWrtSh(nullptr)
    , m_pDocInserter(nullptr)
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
    m_pFileCB->SetClickHdl     ( LINK( this, SwInsertSectionTabPage, UseFileHdl ));
    m_pFilePB->SetClickHdl     ( LINK( this, SwInsertSectionTabPage, FileSearchHdl ));
    m_pCurName->SetModifyHdl   ( LINK( this, SwInsertSectionTabPage, NameEditHdl));
    m_pDDECB->SetClickHdl      ( LINK( this, SwInsertSectionTabPage, DDEHdl ));
    ChangeProtectHdl(m_pProtectCB);
    m_pSubRegionED->EnableAutocomplete( true, true );
}

SwInsertSectionTabPage::~SwInsertSectionTabPage()
{
    disposeOnce();
}

void SwInsertSectionTabPage::dispose()
{
    delete m_pDocInserter;
    m_pCurName.clear();
    m_pFileCB.clear();
    m_pDDECB.clear();
    m_pDDECommandFT.clear();
    m_pFileNameFT.clear();
    m_pFileNameED.clear();
    m_pFilePB.clear();
    m_pSubRegionFT.clear();
    m_pSubRegionED.clear();
    m_pProtectCB.clear();
    m_pPasswdCB.clear();
    m_pPasswdPB.clear();
    m_pHideCB.clear();
    m_pConditionFT.clear();
    m_pConditionED.clear();
    m_pEditInReadonlyCB.clear();
    SfxTabPage::dispose();
}

void    SwInsertSectionTabPage::SetWrtShell(SwWrtShell& rSh)
{
    m_pWrtSh = &rSh;

    bool bWeb = dynamic_cast<SwWebDocShell*>( m_pWrtSh->GetView().GetDocShell() )!= nullptr;
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
        const OUString sSectionName(pSectionData->GetSectionName());
        m_pCurName->SetText(rSh.GetUniqueSectionName(&sSectionName));
        m_pProtectCB->Check( pSectionData->IsProtectFlag() );
        m_sFileName = pSectionData->GetLinkFileName();
        m_sFilePasswd = pSectionData->GetLinkFilePassword();
        m_pFileCB->Check( !m_sFileName.isEmpty() );
        m_pFileNameED->SetText( m_sFileName );
        UseFileHdl(m_pFileCB);
    }
    else
    {
        m_pCurName->SetText( rSh.GetUniqueSectionName() );
    }
}

bool SwInsertSectionTabPage::FillItemSet( SfxItemSet* )
{
    SwSectionData aSection(CONTENT_SECTION, m_pCurName->GetText());
    aSection.SetCondition(m_pConditionED->GetText());
    bool bProtected = m_pProtectCB->IsChecked();
    aSection.SetProtectFlag(bProtected);
    aSection.SetHidden(m_pHideCB->IsChecked());
    // edit in readonly sections
    aSection.SetEditInReadonlyFlag(m_pEditInReadonlyCB->IsChecked());

    if(bProtected)
    {
        aSection.SetPassword(m_aNewPasswd);
    }
    const OUString sFileName = m_pFileNameED->GetText();
    const OUString sSubRegion = m_pSubRegionED->GetText();
    bool bDDe = m_pDDECB->IsChecked();
    if(m_pFileCB->IsChecked() && (!sFileName.isEmpty() || !sSubRegion.isEmpty() || bDDe))
    {
        OUString aLinkFile;
        if( bDDe )
        {
            aLinkFile = SwSectionData::CollapseWhiteSpaces(sFileName);
            sal_Int32 nPos = 0;
            aLinkFile = aLinkFile.replaceFirst( " ", OUStringLiteral1(sfx2::cTokenSeparator), &nPos );
            if (nPos>=0)
            {
                aLinkFile = aLinkFile.replaceFirst( " ", OUStringLiteral1(sfx2::cTokenSeparator), &nPos );
            }
        }
        else
        {
            if(!sFileName.isEmpty())
            {
                SfxMedium* pMedium = m_pWrtSh->GetView().GetDocShell()->GetMedium();
                INetURLObject aAbs;
                if( pMedium )
                    aAbs = pMedium->GetURLObject();
                aLinkFile = URIHelper::SmartRel2Abs(
                    aAbs, sFileName, URIHelper::GetMaybeFileHdl() );
                aSection.SetLinkFilePassword( m_sFilePasswd );
            }

            aLinkFile += OUStringLiteral1(sfx2::cTokenSeparator) + m_sFilterName
                      +  OUStringLiteral1(sfx2::cTokenSeparator) + sSubRegion;
        }

        aSection.SetLinkFileName(aLinkFile);
        if (!aLinkFile.isEmpty())
        {
            aSection.SetType( m_pDDECB->IsChecked() ?
                                    DDE_LINK_SECTION :
                                        FILE_LINK_SECTION);
        }
    }
    static_cast<SwInsertSectionTabDialog*>(GetTabDialog())->SetSectionData(aSection);
    return true;
}

void SwInsertSectionTabPage::Reset( const SfxItemSet* )
{
}

VclPtr<SfxTabPage> SwInsertSectionTabPage::Create( TabPageParent pParent,
                                                   const SfxItemSet* rAttrSet)
{
    return VclPtr<SwInsertSectionTabPage>::Create(pParent.pParent, *rAttrSet);
}

IMPL_LINK( SwInsertSectionTabPage, ChangeHideHdl, Button *, pBox, void )
{
    bool bHide = static_cast<CheckBox*>(pBox)->IsChecked();
    m_pConditionED->Enable(bHide);
    m_pConditionFT->Enable(bHide);
}

IMPL_LINK( SwInsertSectionTabPage, ChangeProtectHdl, Button *, pBox, void )
{
    bool bCheck = static_cast<CheckBox*>(pBox)->IsChecked();
    m_pPasswdCB->Enable(bCheck);
    m_pPasswdPB->Enable(bCheck);
}

IMPL_LINK( SwInsertSectionTabPage, ChangePasswdHdl, Button *, pButton, void )
{
    bool bChange = pButton == m_pPasswdPB;
    bool bSet = bChange ? bChange : m_pPasswdCB->IsChecked();
    if(bSet)
    {
        if(!m_aNewPasswd.getLength() || bChange)
        {
            SfxPasswordDialog aPasswdDlg(GetFrameWeld());
            aPasswdDlg.ShowExtras(SfxShowExtras::CONFIRM);
            if (RET_OK == aPasswdDlg.execute())
            {
                const OUString sNewPasswd(aPasswdDlg.GetPassword());
                if (aPasswdDlg.GetConfirm() == sNewPasswd)
                {
                    SvPasswordHelper::GetHashPassword( m_aNewPasswd, sNewPasswd );
                }
                else
                {
                    std::unique_ptr<weld::MessageDialog> xInfoBox(Application::CreateMessageDialog(pButton->GetFrameWeld(),
                                                                  VclMessageType::Info, VclButtonsType::Ok,
                                                                  SwResId(STR_WRONG_PASSWD_REPEAT)));
                    xInfoBox->run();
                }
            }
            else if(!bChange)
                m_pPasswdCB->Check(false);
        }
    }
    else
        m_aNewPasswd.realloc(0);
}

IMPL_LINK_NOARG(SwInsertSectionTabPage, NameEditHdl, Edit&, void)
{
    const OUString aName = m_pCurName->GetText();
    GetTabDialog()->GetOKButton().Enable(!aName.isEmpty() &&
            m_pCurName->GetEntryPos( aName ) == LISTBOX_ENTRY_NOTFOUND);
}

IMPL_LINK( SwInsertSectionTabPage, UseFileHdl, Button *, pButton, void )
{
    CheckBox* pBox = static_cast<CheckBox*>(pButton);
    if( pBox->IsChecked() )
    {
        if (m_pWrtSh->HasSelection())
        {
            std::unique_ptr<weld::MessageDialog> xQueryBox(Application::CreateMessageDialog(GetFrameWeld(),
                                                           VclMessageType::Question, VclButtonsType::YesNo,
                                                           SwResId(STR_QUERY_CONNECT)));
            if (RET_NO == xQueryBox->run())
                pBox->Check( false );
        }
    }

    bool bFile = pBox->IsChecked();
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
        m_pProtectCB->Check();
    }
    else
    {
        m_pDDECB->Check(false);
        DDEHdl(m_pDDECB);
    }
}

IMPL_LINK_NOARG(SwInsertSectionTabPage, FileSearchHdl, Button*, void)
{
    delete m_pDocInserter;
    m_pDocInserter = new ::sfx2::DocumentInserter(GetFrameWeld(), "swriter");
    m_pDocInserter->StartExecuteModal( LINK( this, SwInsertSectionTabPage, DlgClosedHdl ) );
}

IMPL_LINK( SwInsertSectionTabPage, DDEHdl, Button*, pButton, void )
{
    CheckBox* pBox = static_cast<CheckBox*>(pButton);
    bool bDDE = pBox->IsChecked();
    bool bFile = m_pFileCB->IsChecked();
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
}

IMPL_LINK( SwInsertSectionTabPage, DlgClosedHdl, sfx2::FileDialogHelper *, _pFileDlg, void )
{
    if ( _pFileDlg->GetError() == ERRCODE_NONE )
    {
        std::unique_ptr<SfxMedium> pMedium(m_pDocInserter->CreateMedium("sglobal"));
        if ( pMedium )
        {
            m_sFileName = pMedium->GetURLObject().GetMainURL( INetURLObject::DecodeMechanism::NONE );
            m_sFilterName = pMedium->GetFilter()->GetFilterName();
            const SfxPoolItem* pItem;
            if ( SfxItemState::SET == pMedium->GetItemSet()->GetItemState( SID_PASSWORD, false, &pItem ) )
                m_sFilePasswd = static_cast<const SfxStringItem*>(pItem)->GetValue();
            m_pFileNameED->SetText( INetURLObject::decode(
                m_sFileName, INetURLObject::DecodeMechanism::Unambiguous ) );
            ::lcl_ReadSections(*pMedium, *m_pSubRegionED);
        }
    }
    else
        m_sFilterName = m_sFilePasswd = aEmptyOUStr;
}

SwSectionFootnoteEndTabPage::SwSectionFootnoteEndTabPage(TabPageParent pParent, const SfxItemSet &rAttrSet)
    : SfxTabPage(pParent, "modules/swriter/ui/footnotesendnotestabpage.ui", "FootnotesEndnotesTabPage", &rAttrSet)
    , m_xFootnoteNtAtTextEndCB(m_xBuilder->weld_check_button("ftnntattextend"))
    , m_xFootnoteNtNumCB(m_xBuilder->weld_check_button("ftnntnum"))
    , m_xFootnoteOffsetLbl(m_xBuilder->weld_label("ftnoffset_label"))
    , m_xFootnoteOffsetField(m_xBuilder->weld_spin_button("ftnoffset"))
    , m_xFootnoteNtNumFormatCB(m_xBuilder->weld_check_button("ftnntnumfmt"))
    , m_xFootnotePrefixFT(m_xBuilder->weld_label("ftnprefix_label"))
    , m_xFootnotePrefixED(m_xBuilder->weld_entry("ftnprefix"))
    , m_xFootnoteNumViewBox(new SwNumberingTypeListBox(m_xBuilder->weld_combo_box_text("ftnnumviewbox")))
    , m_xFootnoteSuffixFT(m_xBuilder->weld_label("ftnsuffix_label"))
    , m_xFootnoteSuffixED(m_xBuilder->weld_entry("ftnsuffix"))
    , m_xEndNtAtTextEndCB(m_xBuilder->weld_check_button("endntattextend"))
    , m_xEndNtNumCB(m_xBuilder->weld_check_button("endntnum"))
    , m_xEndOffsetLbl(m_xBuilder->weld_label("endoffset_label"))
    , m_xEndOffsetField(m_xBuilder->weld_spin_button("endoffset"))
    , m_xEndNtNumFormatCB(m_xBuilder->weld_check_button("endntnumfmt"))
    , m_xEndPrefixFT(m_xBuilder->weld_label("endprefix_label"))
    , m_xEndPrefixED(m_xBuilder->weld_entry("endprefix"))
    , m_xEndNumViewBox(new SwNumberingTypeListBox(m_xBuilder->weld_combo_box_text("endnumviewbox")))
    , m_xEndSuffixFT(m_xBuilder->weld_label("endsuffix_label"))
    , m_xEndSuffixED(m_xBuilder->weld_entry("endsuffix"))
{
    m_xFootnoteNumViewBox->Reload(SwInsertNumTypes::Extended);
    m_xEndNumViewBox->Reload(SwInsertNumTypes::Extended);

    Link<weld::ToggleButton&,void> aLk( LINK( this, SwSectionFootnoteEndTabPage, FootEndHdl));
    m_xFootnoteNtAtTextEndCB->connect_toggled( aLk );
    m_xFootnoteNtNumCB->connect_toggled( aLk );
    m_xEndNtAtTextEndCB->connect_toggled( aLk );
    m_xEndNtNumCB->connect_toggled( aLk );
    m_xFootnoteNtNumFormatCB->connect_toggled( aLk );
    m_xEndNtNumFormatCB->connect_toggled( aLk );
}

SwSectionFootnoteEndTabPage::~SwSectionFootnoteEndTabPage()
{
}

bool SwSectionFootnoteEndTabPage::FillItemSet( SfxItemSet* rSet )
{
    SwFormatFootnoteAtTextEnd aFootnote( m_xFootnoteNtAtTextEndCB->get_active()
                            ? ( m_xFootnoteNtNumCB->get_active()
                                ? ( m_xFootnoteNtNumFormatCB->get_active()
                                    ? FTNEND_ATTXTEND_OWNNUMANDFMT
                                    : FTNEND_ATTXTEND_OWNNUMSEQ )
                                : FTNEND_ATTXTEND )
                            : FTNEND_ATPGORDOCEND );

    switch( aFootnote.GetValue() )
    {
    case FTNEND_ATTXTEND_OWNNUMANDFMT:
        aFootnote.SetNumType( m_xFootnoteNumViewBox->GetSelectedNumberingType() );
        aFootnote.SetPrefix( m_xFootnotePrefixED->get_text().replaceAll("\\t", "\t") ); // fdo#65666
        aFootnote.SetSuffix( m_xFootnoteSuffixED->get_text().replaceAll("\\t", "\t") );
        SAL_FALLTHROUGH;

    case FTNEND_ATTXTEND_OWNNUMSEQ:
        aFootnote.SetOffset( static_cast< sal_uInt16 >( m_xFootnoteOffsetField->get_value()-1 ) );
        break;
    default: break;
    }

    SwFormatEndAtTextEnd aEnd( m_xEndNtAtTextEndCB->get_active()
                            ? ( m_xEndNtNumCB->get_active()
                                ? ( m_xEndNtNumFormatCB->get_active()
                                    ? FTNEND_ATTXTEND_OWNNUMANDFMT
                                    : FTNEND_ATTXTEND_OWNNUMSEQ )
                                : FTNEND_ATTXTEND )
                            : FTNEND_ATPGORDOCEND );

    switch( aEnd.GetValue() )
    {
    case FTNEND_ATTXTEND_OWNNUMANDFMT:
        aEnd.SetNumType( m_xEndNumViewBox->GetSelectedNumberingType() );
        aEnd.SetPrefix( m_xEndPrefixED->get_text().replaceAll("\\t", "\t") );
        aEnd.SetSuffix( m_xEndSuffixED->get_text().replaceAll("\\t", "\t") );
        SAL_FALLTHROUGH;

    case FTNEND_ATTXTEND_OWNNUMSEQ:
        aEnd.SetOffset( static_cast< sal_uInt16 >( m_xEndOffsetField->get_value()-1 ) );
        break;
    default: break;
    }

    rSet->Put( aFootnote );
    rSet->Put( aEnd );

    return true;
}

void SwSectionFootnoteEndTabPage::ResetState( bool bFootnote,
                                         const SwFormatFootnoteEndAtTextEnd& rAttr )
{
    weld::CheckButton *pNtAtTextEndCB, *pNtNumCB, *pNtNumFormatCB;
    weld::Label *pPrefixFT, *pSuffixFT;
    weld::Entry *pPrefixED, *pSuffixED;
    SwNumberingTypeListBox *pNumViewBox;
    weld::Label *pOffsetText;
    weld::SpinButton *pOffsetField;

    if( bFootnote )
    {
        pNtAtTextEndCB = m_xFootnoteNtAtTextEndCB.get();
        pNtNumCB = m_xFootnoteNtNumCB.get();
        pNtNumFormatCB = m_xFootnoteNtNumFormatCB.get();
        pPrefixFT = m_xFootnotePrefixFT.get();
        pPrefixED = m_xFootnotePrefixED.get();
        pSuffixFT = m_xFootnoteSuffixFT.get();
        pSuffixED = m_xFootnoteSuffixED.get();
        pNumViewBox = m_xFootnoteNumViewBox.get();
        pOffsetText = m_xFootnoteOffsetLbl.get();
        pOffsetField = m_xFootnoteOffsetField.get();
    }
    else
    {
        pNtAtTextEndCB = m_xEndNtAtTextEndCB.get();
        pNtNumCB = m_xEndNtNumCB.get();
        pNtNumFormatCB = m_xEndNtNumFormatCB.get();
        pPrefixFT = m_xEndPrefixFT.get();
        pPrefixED = m_xEndPrefixED.get();
        pSuffixFT = m_xEndSuffixFT.get();
        pSuffixED = m_xEndSuffixED.get();
        pNumViewBox = m_xEndNumViewBox.get();
        pOffsetText = m_xEndOffsetLbl.get();
        pOffsetField = m_xEndOffsetField.get();
    }

    const sal_uInt16 eState = rAttr.GetValue();
    switch( eState )
    {
    case FTNEND_ATTXTEND_OWNNUMANDFMT:
        pNtNumFormatCB->set_state( TRISTATE_TRUE );
        SAL_FALLTHROUGH;

    case FTNEND_ATTXTEND_OWNNUMSEQ:
        pNtNumCB->set_state( TRISTATE_TRUE );
        SAL_FALLTHROUGH;

    case FTNEND_ATTXTEND:
        pNtAtTextEndCB->set_state( TRISTATE_TRUE );
        // no break;
    }

    pNumViewBox->SelectNumberingType( rAttr.GetNumType() );
    pOffsetField->set_value( rAttr.GetOffset() + 1 );
    pPrefixED->set_text( rAttr.GetPrefix().replaceAll("\t", "\\t") );
    pSuffixED->set_text( rAttr.GetSuffix().replaceAll("\t", "\\t") );

    switch( eState )
    {
    case FTNEND_ATPGORDOCEND:
        pNtNumCB->set_sensitive( false );
        SAL_FALLTHROUGH;

    case FTNEND_ATTXTEND:
        pNtNumFormatCB->set_sensitive( false );
        pOffsetField->set_sensitive( false );
        pOffsetText->set_sensitive( false );
        SAL_FALLTHROUGH;

    case FTNEND_ATTXTEND_OWNNUMSEQ:
        pNumViewBox->Enable( false );
        pPrefixFT->set_sensitive( false );
        pPrefixED->set_sensitive( false );
        pSuffixFT->set_sensitive( false );
        pSuffixED->set_sensitive( false );
        // no break;
    }
}

void SwSectionFootnoteEndTabPage::Reset( const SfxItemSet* rSet )
{
    ResetState( true, rSet->Get( RES_FTN_AT_TXTEND, false ));
    ResetState( false, rSet->Get( RES_END_AT_TXTEND, false ));
}

VclPtr<SfxTabPage> SwSectionFootnoteEndTabPage::Create( TabPageParent pParent,
                                                   const SfxItemSet* rAttrSet)
{
    return VclPtr<SwSectionFootnoteEndTabPage>::Create(pParent, *rAttrSet);
}

IMPL_LINK( SwSectionFootnoteEndTabPage, FootEndHdl, weld::ToggleButton&, rBox, void )
{
    bool bFoot = m_xFootnoteNtAtTextEndCB.get() == &rBox || m_xFootnoteNtNumCB.get() == &rBox ||
                    m_xFootnoteNtNumFormatCB.get() == &rBox ;

    weld::CheckButton *pNumBox, *pNumFormatBox, *pEndBox;
    SwNumberingTypeListBox* pNumViewBox;
    weld::Label *pOffsetText;
    weld::SpinButton *pOffsetField;
    weld::Label *pPrefixFT, *pSuffixFT;
    weld::Entry *pPrefixED, *pSuffixED;

    if( bFoot )
    {
        pEndBox = m_xFootnoteNtAtTextEndCB.get();
        pNumBox = m_xFootnoteNtNumCB.get();
        pNumFormatBox = m_xFootnoteNtNumFormatCB.get();
        pNumViewBox = m_xFootnoteNumViewBox.get();
        pOffsetText = m_xFootnoteOffsetLbl.get();
        pOffsetField = m_xFootnoteOffsetField.get();
        pPrefixFT = m_xFootnotePrefixFT.get();
        pSuffixFT = m_xFootnoteSuffixFT.get();
        pPrefixED = m_xFootnotePrefixED.get();
        pSuffixED = m_xFootnoteSuffixED.get();
    }
    else
    {
        pEndBox = m_xEndNtAtTextEndCB.get();
        pNumBox = m_xEndNtNumCB.get();
        pNumFormatBox = m_xEndNtNumFormatCB.get();
        pNumViewBox = m_xEndNumViewBox.get();
        pOffsetText = m_xEndOffsetLbl.get();
        pOffsetField = m_xEndOffsetField.get();
        pPrefixFT = m_xEndPrefixFT.get();
        pSuffixFT = m_xEndSuffixFT.get();
        pPrefixED = m_xEndPrefixED.get();
        pSuffixED = m_xEndSuffixED.get();
    }

    bool bEnableAtEnd = TRISTATE_TRUE == pEndBox->get_state();
    bool bEnableNum = bEnableAtEnd && TRISTATE_TRUE == pNumBox->get_state();
    bool bEnableNumFormat = bEnableNum && TRISTATE_TRUE == pNumFormatBox->get_state();

    pNumBox->set_sensitive( bEnableAtEnd );
    pOffsetText->set_sensitive( bEnableNum );
    pOffsetField->set_sensitive( bEnableNum );
    pNumFormatBox->set_sensitive( bEnableNum );
    pNumViewBox->Enable( bEnableNumFormat );
    pPrefixED->set_sensitive( bEnableNumFormat );
    pSuffixED->set_sensitive( bEnableNumFormat );
    pPrefixFT->set_sensitive( bEnableNumFormat );
    pSuffixFT->set_sensitive( bEnableNumFormat );
}

SwSectionPropertyTabDialog::SwSectionPropertyTabDialog(
    vcl::Window* pParent, const SfxItemSet& rSet, SwWrtShell& rSh)
    : SfxTabDialog(pParent, "FormatSectionDialog",
        "modules/swriter/ui/formatsectiondialog.ui", &rSet)
    , rWrtSh(rSh)
{
    SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();
    m_nColumnPageId = AddTabPage("columns",   SwColumnPage::Create,    nullptr);
    m_nBackPageId = AddTabPage("background", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BKG ), nullptr );
    m_nNotePageId = AddTabPage("notes", SwSectionFootnoteEndTabPage::Create, nullptr);
    m_nIndentPage = AddTabPage("indents", SwSectionIndentTabPage::Create, nullptr);

    SvxHtmlOptions& rHtmlOpt = SvxHtmlOptions::Get();
    long nHtmlMode = rHtmlOpt.GetExportMode();
    bool bWeb = dynamic_cast<SwWebDocShell*>( rSh.GetView().GetDocShell()  ) != nullptr ;
    if(bWeb)
    {
        RemoveTabPage(m_nNotePageId);
        RemoveTabPage(m_nIndentPage);
        if( HTML_CFG_NS40 != nHtmlMode && HTML_CFG_WRITER != nHtmlMode)
            RemoveTabPage(m_nColumnPageId);
    }
}

SwSectionPropertyTabDialog::~SwSectionPropertyTabDialog()
{
}

void SwSectionPropertyTabDialog::PageCreated( sal_uInt16 nId, SfxTabPage &rPage )
{
    if (nId == m_nBackPageId)
    {
            SfxAllItemSet aSet(*(GetInputSetImpl()->GetPool()));
            aSet.Put (SfxUInt32Item(SID_FLAG_TYPE, static_cast<sal_uInt32>(SvxBackgroundTabFlags::SHOW_SELECTOR)));
            rPage.PageCreated(aSet);
    }
    else if (nId == m_nColumnPageId)
    {
        static_cast<SwColumnPage&>(rPage).ShowBalance(true);
        static_cast<SwColumnPage&>(rPage).SetInSection(true);
    }
    else if (nId == m_nIndentPage)
        static_cast<SwSectionIndentTabPage&>(rPage).SetWrtShell(rWrtSh);
}

SwSectionIndentTabPage::SwSectionIndentTabPage(TabPageParent pParent, const SfxItemSet &rAttrSet)
    : SfxTabPage(pParent, "modules/swriter/ui/indentpage.ui", "IndentPage", &rAttrSet)
    , m_xBeforeMF(m_xBuilder->weld_metric_spin_button("before", FUNIT_CM))
    , m_xAfterMF(m_xBuilder->weld_metric_spin_button("after", FUNIT_CM))
    , m_xPreviewWin(new weld::CustomWeld(*m_xBuilder, "preview", m_aPreviewWin))
{
    Link<weld::MetricSpinButton&,void> aLk = LINK(this, SwSectionIndentTabPage, IndentModifyHdl);
    m_xBeforeMF->connect_value_changed(aLk);
    m_xAfterMF->connect_value_changed(aLk);
}

SwSectionIndentTabPage::~SwSectionIndentTabPage()
{
}

bool SwSectionIndentTabPage::FillItemSet(SfxItemSet* rSet)
{
    if (m_xBeforeMF->get_value_changed_from_saved() || m_xAfterMF->get_value_changed_from_saved())
    {
        SvxLRSpaceItem aLRSpace(
                m_xBeforeMF->denormalize(m_xBeforeMF->get_value(FUNIT_TWIP)) ,
                m_xAfterMF->denormalize(m_xAfterMF->get_value(FUNIT_TWIP)), 0, 0, RES_LR_SPACE);
        rSet->Put(aLRSpace);
    }
    return true;
}

void SwSectionIndentTabPage::Reset( const SfxItemSet* rSet)
{
    //this page doesn't show up in HTML mode
    FieldUnit aMetric = ::GetDfltMetric(false);
    SetFieldUnit(*m_xBeforeMF, aMetric);
    SetFieldUnit(*m_xAfterMF , aMetric);

    SfxItemState eItemState = rSet->GetItemState( RES_LR_SPACE );
    if ( eItemState >= SfxItemState::DEFAULT )
    {
        const SvxLRSpaceItem& rSpace =
            rSet->Get( RES_LR_SPACE );

        m_xBeforeMF->set_value(m_xBeforeMF->normalize(rSpace.GetLeft()), FUNIT_TWIP);
        m_xAfterMF->set_value(m_xAfterMF->normalize(rSpace.GetRight()), FUNIT_TWIP);
    }
    else
    {
        m_xBeforeMF->set_text("");
        m_xAfterMF->set_text("");
    }
    m_xBeforeMF->save_value();
    m_xAfterMF->save_value();
    IndentModifyHdl(*m_xBeforeMF);
}

VclPtr<SfxTabPage> SwSectionIndentTabPage::Create(TabPageParent pParent, const SfxItemSet* rAttrSet)
{
    return VclPtr<SwSectionIndentTabPage>::Create(pParent, *rAttrSet);
}

void SwSectionIndentTabPage::SetWrtShell(SwWrtShell const & rSh)
{
    //set sensible values at the preview
    m_aPreviewWin.SetAdjust(SvxAdjust::Block);
    m_aPreviewWin.SetLastLine(SvxAdjust::Block);
    const SwRect& rPageRect = rSh.GetAnyCurRect( CurRectType::Page );
    Size aPageSize(rPageRect.Width(), rPageRect.Height());
    m_aPreviewWin.SetSize(aPageSize);
}

IMPL_LINK_NOARG(SwSectionIndentTabPage, IndentModifyHdl, weld::MetricSpinButton&, void)
{
    m_aPreviewWin.SetLeftMargin(m_xBeforeMF->denormalize(m_xBeforeMF->get_value(FUNIT_TWIP)));
    m_aPreviewWin.SetRightMargin(m_xAfterMF->denormalize(m_xAfterMF->get_value(FUNIT_TWIP)));
    m_aPreviewWin.Invalidate();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
