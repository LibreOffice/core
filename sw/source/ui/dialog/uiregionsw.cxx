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
#include <vcl/treelistentry.hxx>

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

OUString BuildBitmap(bool bProtect, bool bHidden)
{
    if (bProtect)
        return bHidden ? OUString(RID_BMP_PROT_HIDE) : OUString(RID_BMP_PROT_NO_HIDE);
    return bHidden ? OUString(RID_BMP_HIDE) : OUString(RID_BMP_NO_HIDE);
}

}

static void   lcl_ReadSections( SfxMedium& rMedium, weld::ComboBox& rBox );

static void lcl_FillList( SwWrtShell& rSh, weld::ComboBox& rSubRegions, weld::ComboBox* pAvailNames, const SwSectionFormat* pNewFormat )
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
                    if (pAvailNames)
                        pAvailNames->append_text(sString);
                    rSubRegions.append_text(sString);
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
                    if (pAvailNames)
                        pAvailNames->append_text(sString);
                    rSubRegions.append_text(sString);
                    lcl_FillList( rSh, rSubRegions, pAvailNames, pFormat );
                }
            }
        }
    }
}

static void lcl_FillSubRegionList( SwWrtShell& rSh, weld::ComboBox& rSubRegions, weld::ComboBox* pAvailNames )
{
    rSubRegions.clear();
    lcl_FillList( rSh, rSubRegions, pAvailNames, nullptr );
    IDocumentMarkAccess* const pMarkAccess = rSh.getIDocumentMarkAccess();
    for( IDocumentMarkAccess::const_iterator_t ppMark = pMarkAccess->getBookmarksBegin();
        ppMark != pMarkAccess->getBookmarksEnd();
        ++ppMark)
    {
        const ::sw::mark::IMark* pBkmk = ppMark->get();
        if( pBkmk->IsExpanded() )
            rSubRegions.append_text( pBkmk->GetName() );
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
    sal_Int32 nIdx{ 0 };
    const OUString sFile( sOldFileName.getToken( 0, sfx2::cTokenSeparator, nIdx ) ); // token 0
    const OUString sSub( sOldFileName.getToken( 1, sfx2::cTokenSeparator, nIdx ) );  // token 2

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
SwEditRegionDlg::SwEditRegionDlg(weld::Window* pParent, SwWrtShell& rWrtSh)
    : SfxDialogController(pParent, "modules/swriter/ui/editsectiondialog.ui",
                          "EditSectionDialog")
    , m_bSubRegionsFilled(false)
    , rSh(rWrtSh)
    , bDontCheckPasswd(true)
    , m_xCurName(m_xBuilder->weld_entry("curname"))
    , m_xTree(m_xBuilder->weld_tree_view("tree"))
    , m_xFileCB(m_xBuilder->weld_check_button("link"))
    , m_xDDECB(m_xBuilder->weld_check_button("dde"))
    , m_xDDEFrame(m_xBuilder->weld_widget("ddedepend"))
    , m_xFileNameFT(m_xBuilder->weld_label("filenameft"))
    , m_xDDECommandFT(m_xBuilder->weld_label("ddeft"))
    , m_xFileNameED(m_xBuilder->weld_entry("filename"))
    , m_xFilePB(m_xBuilder->weld_button("file"))
    , m_xSubRegionFT(m_xBuilder->weld_label("sectionft"))
    , m_xSubRegionED(m_xBuilder->weld_combo_box("section"))
    , m_xProtectCB(m_xBuilder->weld_check_button("protect"))
    , m_xPasswdCB(m_xBuilder->weld_check_button("withpassword"))
    , m_xPasswdPB(m_xBuilder->weld_button("password"))
    , m_xHideCB(m_xBuilder->weld_check_button("hide"))
    , m_xConditionFT(m_xBuilder->weld_label("conditionft"))
    , m_xConditionED(new SwConditionEdit(m_xBuilder->weld_entry("condition")))
    , m_xEditInReadonlyCB(m_xBuilder->weld_check_button("editinro"))
    , m_xOK(m_xBuilder->weld_button("ok"))
    , m_xOptionsPB(m_xBuilder->weld_button("options"))
    , m_xDismiss(m_xBuilder->weld_button("remove"))
    , m_xHideFrame(m_xBuilder->weld_widget("hideframe"))
{
    m_xTree->set_size_request(-1, m_xTree->get_height_rows(16));
    m_xFileCB->set_state(TRISTATE_FALSE);
    m_xSubRegionED->make_sorted();
    m_xProtectCB->set_state(TRISTATE_FALSE);
    m_xHideCB->set_state(TRISTATE_FALSE);
    // edit in readonly sections
    m_xEditInReadonlyCB->set_state(TRISTATE_FALSE);

    bool bWeb = dynamic_cast<SwWebDocShell*>( rSh.GetView().GetDocShell() ) != nullptr;

    m_xTree->connect_changed(LINK(this, SwEditRegionDlg, GetFirstEntryHdl));
    m_xCurName->connect_changed(LINK(this, SwEditRegionDlg, NameEditHdl));
    m_xConditionED->connect_changed( LINK( this, SwEditRegionDlg, ConditionEditHdl));
    m_xOK->connect_clicked( LINK( this, SwEditRegionDlg, OkHdl));
    m_xPasswdCB->connect_toggled(LINK(this, SwEditRegionDlg, TogglePasswdHdl));
    m_xPasswdPB->connect_clicked(LINK(this, SwEditRegionDlg, ChangePasswdHdl));
    m_xHideCB->connect_toggled(LINK(this, SwEditRegionDlg, ChangeHideHdl));
    // edit in readonly sections
    m_xEditInReadonlyCB->connect_toggled(LINK(this, SwEditRegionDlg, ChangeEditInReadonlyHdl));

    m_xOptionsPB->connect_clicked(LINK(this, SwEditRegionDlg, OptionsHdl));
    m_xProtectCB->connect_toggled(LINK(this, SwEditRegionDlg, ChangeProtectHdl));
    m_xDismiss->connect_clicked( LINK( this, SwEditRegionDlg, ChangeDismissHdl));
    m_xFileCB->connect_toggled(LINK(this, SwEditRegionDlg, UseFileHdl));
    m_xFilePB->connect_clicked(LINK(this, SwEditRegionDlg, FileSearchHdl));
    m_xFileNameED->connect_changed(LINK(this, SwEditRegionDlg, FileNameEntryHdl));
    m_xSubRegionED->connect_changed(LINK(this, SwEditRegionDlg, FileNameComboBoxHdl));
    m_xSubRegionED->connect_popup_toggled(LINK(this, SwEditRegionDlg, SubRegionEventHdl));
    m_xSubRegionED->set_entry_completion(true, true);

    m_xTree->set_selection_mode(SelectionMode::Multiple);

    if (bWeb)
    {
        m_xDDECB->hide();
        m_xHideFrame->hide();
        m_xPasswdCB->hide();
    }

    m_xDDECB->connect_toggled(LINK(this, SwEditRegionDlg, DDEHdl));

    pCurrSect = rSh.GetCurrSection();
    RecurseList( nullptr, nullptr );

    // if the cursor is not in a region the first one will always be selected
    if (!m_xTree->get_selected(nullptr))
    {
        std::unique_ptr<weld::TreeIter> xIter(m_xTree->make_iterator());
        if (m_xTree->get_iter_first(*xIter))
        {
            m_xTree->select(*xIter);
            GetFirstEntryHdl(*m_xTree);
        }
    }

    m_xTree->show();
    bDontCheckPasswd = false;
}

bool SwEditRegionDlg::CheckPasswd(weld::ToggleButton* pBox)
{
    if (bDontCheckPasswd)
        return true;
    bool bRet = true;

    m_xTree->selected_foreach([this, &bRet](weld::TreeIter& rEntry){
        SectRepr* pRepr = reinterpret_cast<SectRepr*>(m_xTree->get_id(rEntry).toInt64());
        if (!pRepr->GetTempPasswd().getLength()
            && pRepr->GetSectionData().GetPassword().getLength())
        {
            SfxPasswordDialog aPasswdDlg(m_xDialog.get());
            bRet = false;
            if (aPasswdDlg.run())
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
                    std::unique_ptr<weld::MessageDialog> xInfoBox(Application::CreateMessageDialog(m_xDialog.get(),
                                                                  VclMessageType::Info, VclButtonsType::Ok,
                                                                  SwResId(STR_WRONG_PASSWORD)));
                    xInfoBox->run();
                }
            }
        }
        return false;
    });
    if (!bRet && pBox)
    {
        //reset old button state
        if (pBox->get_state() != TRISTATE_INDET)
            pBox->set_active(!pBox->get_active());
    }

    return bRet;
}

// recursively look for child-sections
void SwEditRegionDlg::RecurseList(const SwSectionFormat* pFormat, weld::TreeIter* pEntry)
{
    std::unique_ptr<weld::TreeIter> xIter(m_xTree->make_iterator());
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

                OUString sText(pSect->GetSectionName());
                OUString sImage(BuildBitmap(pSect->IsProtect(),pSect->IsHidden()));
                OUString sId(OUString::number(reinterpret_cast<sal_Int64>(pSectRepr)));
                m_xTree->insert(nullptr, -1, &sText, &sId, nullptr, nullptr, &sImage, false, xIter.get());

                RecurseList(pFormat, xIter.get());
                if (m_xTree->iter_has_child(*xIter))
                    m_xTree->expand_row(*xIter);
                if (pCurrSect==pSect)
                {
                    m_xTree->select(*xIter);
                    m_xTree->scroll_to_row(*xIter);
                    GetFirstEntryHdl(*m_xTree);
                }
            }
        }
    }
    else
    {
        SwSections aTmpArr;
        pFormat->GetChildSections(aTmpArr, SectionSort::Pos);
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

                OUString sText(pSect->GetSectionName());
                OUString sImage = BuildBitmap(pSect->IsProtect(), pSect->IsHidden());
                OUString sId(OUString::number(reinterpret_cast<sal_Int64>(pSectRepr)));
                m_xTree->insert(pEntry, -1, &sText, &sId, nullptr, nullptr, &sImage, false, xIter.get());

                RecurseList(pSect->GetFormat(), xIter.get());
                if (m_xTree->iter_has_child(*xIter))
                    m_xTree->expand_row(*xIter);
                if (pCurrSect==pSect)
                {
                    m_xTree->select(*xIter);
                    m_xTree->scroll_to_row(*xIter);
                    GetFirstEntryHdl(*m_xTree);
                }
            }
        }
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
    std::unique_ptr<weld::TreeIter> xIter(m_xTree->make_iterator());
    if (m_xTree->get_iter_first(*xIter))
    {
        do
        {
            delete reinterpret_cast<SectRepr*>(m_xTree->get_id(*xIter).toInt64());
        } while (m_xTree->iter_next(*xIter));
    }
}

void SwEditRegionDlg::SelectSection(const OUString& rSectionName)
{
    std::unique_ptr<weld::TreeIter> xIter(m_xTree->make_iterator());
    if (m_xTree->get_iter_first(*xIter))
    {
        do
        {
            SectRepr* pRepr = reinterpret_cast<SectRepr*>(m_xTree->get_id(*xIter).toInt64());
            if (pRepr->GetSectionData().GetSectionName() == rSectionName)
            {
                m_xTree->unselect_all();
                m_xTree->select(*xIter);
                m_xTree->scroll_to_row(*xIter);
                GetFirstEntryHdl(*m_xTree);
                break;
            }
        } while (m_xTree->iter_next(*xIter));
    }
}

// selected entry in TreeListBox is showed in Edit window in case of
// multiselection some controls are disabled
IMPL_LINK(SwEditRegionDlg, GetFirstEntryHdl, weld::TreeView&, rBox, void)
{
    bDontCheckPasswd = true;
    std::unique_ptr<weld::TreeIter> xIter(rBox.make_iterator());
    bool bEntry = rBox.get_selected(xIter.get());
    m_xHideCB->set_sensitive(true);
    // edit in readonly sections
    m_xEditInReadonlyCB->set_sensitive(true);

    m_xProtectCB->set_sensitive(true);
    m_xFileCB->set_sensitive(true);
    css::uno::Sequence <sal_Int8> aCurPasswd;
    if (1 < rBox.count_selected_rows())
    {
        m_xHideCB->set_state(TRISTATE_INDET);
        m_xProtectCB->set_state(TRISTATE_INDET);
        // edit in readonly sections
        m_xEditInReadonlyCB->set_state(TRISTATE_INDET);
        m_xFileCB->set_state(TRISTATE_INDET);

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

        m_xTree->selected_foreach([&](weld::TreeIter& rEntry){
            SectRepr* pRepr = reinterpret_cast<SectRepr*>(m_xTree->get_id(rEntry).toInt64());
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
            bFirst = false;
            return false;
        });

        m_xHideCB->set_state(!bHiddenValid ? TRISTATE_INDET :
                    bHidden ? TRISTATE_TRUE : TRISTATE_FALSE);
        m_xProtectCB->set_state(!bProtectValid ? TRISTATE_INDET :
                    bProtect ? TRISTATE_TRUE : TRISTATE_FALSE);
        // edit in readonly sections
        m_xEditInReadonlyCB->set_state(!bEditInReadonlyValid ? TRISTATE_INDET :
                    bEditInReadonly ? TRISTATE_TRUE : TRISTATE_FALSE);

        m_xFileCB->set_state(!bFileValid ? TRISTATE_INDET :
                    bFile ? TRISTATE_TRUE : TRISTATE_FALSE);

        if (bConditionValid)
            m_xConditionED->set_text(sCondition);
        else
        {
            m_xConditionFT->set_sensitive(false);
            m_xConditionED->set_sensitive(false);
        }

        m_xCurName->set_sensitive(false);
        m_xDDECB->set_sensitive(false);
        m_xDDEFrame->set_sensitive(false);
        m_xOptionsPB->set_sensitive(false);
        bool bPasswdEnabled = m_xProtectCB->get_state() == TRISTATE_TRUE;
        m_xPasswdCB->set_sensitive(bPasswdEnabled);
        m_xPasswdPB->set_sensitive(bPasswdEnabled);
        if(!bPasswdValid)
        {
            rBox.get_selected(xIter.get());
            rBox.unselect_all();
            rBox.select(*xIter);
            GetFirstEntryHdl(rBox);
            return;
        }
        else
            m_xPasswdCB->set_active(aCurPasswd.getLength() > 0);
    }
    else if (bEntry )
    {
        m_xCurName->set_sensitive(true);
        m_xOptionsPB->set_sensitive(true);
        SectRepr* pRepr = reinterpret_cast<SectRepr*>(m_xTree->get_id(*xIter).toInt64());
        SwSectionData const& rData( pRepr->GetSectionData() );
        m_xConditionED->set_text(rData.GetCondition());
        m_xHideCB->set_sensitive(true);
        m_xHideCB->set_state((rData.IsHidden()) ? TRISTATE_TRUE : TRISTATE_FALSE);
        bool bHide = TRISTATE_TRUE == m_xHideCB->get_state();
        m_xConditionED->set_sensitive(bHide);
        m_xConditionFT->set_sensitive(bHide);
        m_xPasswdCB->set_active(rData.GetPassword().getLength() > 0);

        m_xOK->set_sensitive(true);
        m_xPasswdCB->set_sensitive(true);
        m_xCurName->set_text(rBox.get_text(*xIter));
        m_xCurName->set_sensitive(true);
        m_xDismiss->set_sensitive(true);
        const OUString aFile = pRepr->GetFile();
        const OUString sSub = pRepr->GetSubRegion();
        m_xSubRegionED->clear();
        m_xSubRegionED->append_text(""); // put in a dummy entry, which is replaced when m_bSubRegionsFilled is set
        m_bSubRegionsFilled = false;
        if( !aFile.isEmpty() || !sSub.isEmpty() )
        {
            m_xFileCB->set_active(true);
            m_xFileNameED->set_text(aFile);
            m_xSubRegionED->set_entry_text(sSub);
            m_xDDECB->set_active(rData.GetType() == DDE_LINK_SECTION);
        }
        else
        {
            m_xFileCB->set_active(false);
            m_xFileNameED->set_text(aFile);
            m_xDDECB->set_sensitive(false);
            m_xDDECB->set_active(false);
        }
        UseFileHdl(*m_xFileCB);
        DDEHdl(*m_xDDECB);
        m_xProtectCB->set_state((rData.IsProtectFlag())
                ? TRISTATE_TRUE : TRISTATE_FALSE);
        m_xProtectCB->set_sensitive(true);

        // edit in readonly sections
        m_xEditInReadonlyCB->set_state((rData.IsEditInReadonlyFlag())
                ? TRISTATE_TRUE : TRISTATE_FALSE);
        m_xEditInReadonlyCB->set_sensitive(true);

        bool bPasswdEnabled = m_xProtectCB->get_active();
        m_xPasswdCB->set_sensitive(bPasswdEnabled);
        m_xPasswdPB->set_sensitive(bPasswdEnabled);
    }
    bDontCheckPasswd = false;
}

#if 0
IMPL_LINK( SwEditRegionDlg, DeselectHdl, SvTreeListBox *, pBox, void )
{
    if( pBox->GetSelectionCount() )
        return;

    m_xHideCB->set_sensitive(false);
    m_xProtectCB->set_sensitive(false);
    // edit in readonly sections
    m_xEditInReadonlyCB->set_sensitive(false);

    m_xPasswdCB->set_sensitive(false);
    m_xConditionFT->set_sensitive(false);
    m_xConditionED->set_sensitive(false);
    m_xFileCB->set_sensitive(false);
    m_xDDEFrame->set_sensitive(false);
    m_xDDECB->set_sensitive(false);
    m_xCurName->set_sensitive(false);

    UseFileHdl(*m_xFileCB);
    DDEHdl(m_xDDECB);

}
#endif

// in OkHdl the modified settings are being applied and reversed regions are deleted
IMPL_LINK_NOARG(SwEditRegionDlg, OkHdl, weld::Button&, void)
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

    std::unique_ptr<weld::TreeIter> xIter(m_xTree->make_iterator());
    if (m_xTree->get_iter_first(*xIter))
    {
        do
        {
            SectRepr* pRepr = reinterpret_cast<SectRepr*>(m_xTree->get_id(*xIter).toInt64());
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
        } while (m_xTree->iter_next(*xIter));
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

    // response must be called ahead of EndAction's end,
    // otherwise ScrollError can occur.
    m_xDialog->response(RET_OK);

    rSh.EndUndo();
    rSh.EndAllAction();
}

// Toggle protect
IMPL_LINK(SwEditRegionDlg, ChangeProtectHdl, weld::ToggleButton&, rButton, void)
{
    if (!CheckPasswd(&rButton))
        return;
    bool bCheck = TRISTATE_TRUE == rButton.get_state();
    m_xTree->selected_foreach([this, bCheck](weld::TreeIter& rEntry){
        SectRepr* pRepr = reinterpret_cast<SectRepr*>(m_xTree->get_id(rEntry).toInt64());
        pRepr->GetSectionData().SetProtectFlag(bCheck);
        OUString aImage = BuildBitmap(bCheck, TRISTATE_TRUE == m_xHideCB->get_state());
        m_xTree->set_image(rEntry, aImage);
        return false;
    });
    m_xPasswdCB->set_sensitive(bCheck);
    m_xPasswdPB->set_sensitive(bCheck);
}

// Toggle hide
IMPL_LINK( SwEditRegionDlg, ChangeHideHdl, weld::ToggleButton&, rButton, void)
{
    if (!CheckPasswd(&rButton))
        return;
    m_xTree->selected_foreach([this, &rButton](weld::TreeIter& rEntry){
        SectRepr* pRepr = reinterpret_cast<SectRepr*>(m_xTree->get_id(rEntry).toInt64());
        pRepr->GetSectionData().SetHidden(TRISTATE_TRUE == rButton.get_state());
        OUString aImage = BuildBitmap(TRISTATE_TRUE == m_xProtectCB->get_state(),
                                      TRISTATE_TRUE == rButton.get_state());
        m_xTree->set_image(rEntry, aImage);
        return false;
    });
    bool bHide = TRISTATE_TRUE == rButton.get_state();
    m_xConditionED->set_sensitive(bHide);
    m_xConditionFT->set_sensitive(bHide);
}

// Toggle edit in readonly
IMPL_LINK(SwEditRegionDlg, ChangeEditInReadonlyHdl, weld::ToggleButton&, rButton, void)
{
    if (!CheckPasswd(&rButton))
        return;
    m_xTree->selected_foreach([this, &rButton](weld::TreeIter& rEntry){
        SectRepr* pRepr = reinterpret_cast<SectRepr*>(m_xTree->get_id(rEntry).toInt64());
        pRepr->GetSectionData().SetEditInReadonlyFlag(
                TRISTATE_TRUE == rButton.get_state());
        return false;
    });
}

// clear selected region
IMPL_LINK_NOARG(SwEditRegionDlg, ChangeDismissHdl, weld::Button&, void)
{
    if(!CheckPasswd())
        return;
    // at first mark all selected
    m_xTree->selected_foreach([this](weld::TreeIter& rEntry){
        SectRepr* const pSectRepr = reinterpret_cast<SectRepr*>(m_xTree->get_id(rEntry).toInt64());
        pSectRepr->SetSelected();
        return false;
    });

    std::unique_ptr<weld::TreeIter> xEntry(m_xTree->make_iterator());
    bool bEntry(m_xTree->get_selected(xEntry.get()));
    // then delete
    while (bEntry)
    {
        SectRepr* const pSectRepr = reinterpret_cast<SectRepr*>(m_xTree->get_id(*xEntry).toInt64());
        std::unique_ptr<weld::TreeIter> xRemove;
        bool bRestart = false;
        if (pSectRepr->IsSelected())
        {
            m_SectReprs.insert(std::make_pair(pSectRepr->GetArrPos(),
                        std::unique_ptr<SectRepr>(pSectRepr)));
            if (m_xTree->iter_has_child(*xEntry))
            {
                std::unique_ptr<weld::TreeIter> xChild(m_xTree->make_iterator(xEntry.get()));
                m_xTree->iter_children(*xChild);
                std::unique_ptr<weld::TreeIter> xParent(m_xTree->make_iterator(xEntry.get()));
                if (!m_xTree->iter_parent(*xParent))
                    xParent.reset();
                bool bChild = true;
                do
                {
                    // because of the repositioning we have to start at the beginning again
                    bRestart = true;
                    std::unique_ptr<weld::TreeIter> xMove(m_xTree->make_iterator(xChild.get()));
                    bChild = m_xTree->iter_next_sibling(*xChild);
                    m_xTree->move_subtree(*xMove, xParent.get(), m_xTree->get_iter_index_in_parent(*xEntry));
                } while (bChild);
            }
            xRemove = m_xTree->make_iterator(xEntry.get());
        }
        if (bRestart)
            bEntry = m_xTree->get_iter_first(*xEntry);
        else
            bEntry = m_xTree->iter_next(*xEntry);
        if (xRemove)
            m_xTree->remove(*xRemove);
    }

    if (m_xTree->get_selected(nullptr))
        return;

    m_xConditionFT->set_sensitive(false);
    m_xConditionED->set_sensitive(false);
    m_xDismiss->set_sensitive(false);
    m_xCurName->set_sensitive(false);
    m_xProtectCB->set_sensitive(false);
    m_xPasswdCB->set_sensitive(false);
    m_xHideCB->set_sensitive(false);
    // edit in readonly sections
    m_xEditInReadonlyCB->set_sensitive(false);
    m_xEditInReadonlyCB->set_state(TRISTATE_FALSE);
    m_xProtectCB->set_state(TRISTATE_FALSE);
    m_xPasswdCB->set_active(false);
    m_xHideCB->set_state(TRISTATE_FALSE);
    m_xFileCB->set_active(false);
    // otherwise the focus would be on HelpButton
    m_xOK->grab_focus();
    UseFileHdl(*m_xFileCB);
}

// link CheckBox to file?
IMPL_LINK(SwEditRegionDlg, UseFileHdl, weld::ToggleButton&, rButton, void)
{
    if (!CheckPasswd(&rButton))
        return;
    bool bMulti = 1 < m_xTree->count_selected_rows();
    bool bFile = rButton.get_active();
    if (m_xTree->get_selected(nullptr))
    {
        m_xTree->selected_foreach([&](weld::TreeIter& rEntry){
            SectRepr* const pSectRepr = reinterpret_cast<SectRepr*>(m_xTree->get_id(rEntry).toInt64());
            bool bContent = pSectRepr->IsContent();
            if( rButton.get_active() && bContent && rSh.HasSelection() )
            {
                std::unique_ptr<weld::MessageDialog> xQueryBox(Application::CreateMessageDialog(m_xDialog.get(),
                                                               VclMessageType::Question, VclButtonsType::YesNo,
                                                               SwResId(STR_QUERY_CONNECT)));
                if (RET_NO == xQueryBox->run())
                    rButton.set_active( false );
            }
            if( bFile )
                pSectRepr->SetContent(false);
            else
            {
                pSectRepr->SetFile(OUString());
                pSectRepr->SetSubRegion(OUString());
                pSectRepr->GetSectionData().SetLinkFilePassword(OUString());
            }
            return false;
        });
        m_xDDECB->set_sensitive(bFile && !bMulti);
        m_xDDEFrame->set_sensitive(bFile && !bMulti);
        if( bFile )
        {
            m_xProtectCB->set_state(TRISTATE_TRUE);
            ChangeProtectHdl(*m_xProtectCB);
            m_xFileNameED->grab_focus();

        }
        else
        {
            m_xDDECB->set_active(false);
            m_xSubRegionED->set_entry_text(OUString());
        }
        DDEHdl(*m_xDDECB);
    }
    else
    {
        rButton.set_active(false);
        rButton.set_sensitive(false);
        m_xDDECB->set_active(false);
        m_xDDECB->set_sensitive(false);
        m_xDDEFrame->set_sensitive(false);
    }
}

// call dialog paste file
IMPL_LINK_NOARG(SwEditRegionDlg, FileSearchHdl, weld::Button&, void)
{
    if(!CheckPasswd())
        return;
    m_pDocInserter.reset(new ::sfx2::DocumentInserter(m_xDialog.get(), "swriter"));
    m_pDocInserter->StartExecuteModal( LINK( this, SwEditRegionDlg, DlgClosedHdl ) );
}

IMPL_LINK_NOARG(SwEditRegionDlg, OptionsHdl, weld::Button&, void)
{
    if(!CheckPasswd())
        return;
    SectRepr* pSectRepr = reinterpret_cast<SectRepr*>(m_xTree->get_selected_id().toInt64());
    if (!pSectRepr)
        return;

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

    SwSectionPropertyTabDialog aTabDlg(m_xDialog.get(), aSet, rSh);
    if (RET_OK == aTabDlg.run())
    {
        const SfxItemSet* pOutSet = aTabDlg.GetOutputItemSet();
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
                m_xTree->selected_foreach([&](weld::TreeIter& rEntry){
                    SectRepr* pRepr = reinterpret_cast<SectRepr*>(m_xTree->get_id(rEntry).toInt64());
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
                    return false;
                });
            }
        }
    }
}

IMPL_LINK(SwEditRegionDlg, FileNameComboBoxHdl, weld::ComboBox&, rEdit, void)
{
    int nStartPos, nEndPos;
    rEdit.get_entry_selection_bounds(nStartPos, nEndPos);
    if (!CheckPasswd())
        return;
    rEdit.select_entry_region(nStartPos, nEndPos);
    SectRepr* pSectRepr = reinterpret_cast<SectRepr*>(m_xTree->get_selected_id().toInt64());
    pSectRepr->SetSubRegion( rEdit.get_active_text() );
}

// Applying of the filename or the linked region
IMPL_LINK(SwEditRegionDlg, FileNameEntryHdl, weld::Entry&, rEdit, void)
{
    int nStartPos, nEndPos;
    rEdit.get_selection_bounds(nStartPos, nEndPos);
    if (!CheckPasswd())
        return;
    rEdit.select_region(nStartPos, nEndPos);
    SectRepr* pSectRepr = reinterpret_cast<SectRepr*>(m_xTree->get_selected_id().toInt64());
    m_xSubRegionED->clear();
    m_xSubRegionED->append_text(""); // put in a dummy entry, which is replaced when m_bSubRegionsFilled is set
    m_bSubRegionsFilled = false;
    if (m_xDDECB->get_active())
    {
        OUString sLink( SwSectionData::CollapseWhiteSpaces(rEdit.get_text()) );
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
        OUString sTmp(rEdit.get_text());
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
        pSectRepr->GetSectionData().SetLinkFilePassword(OUString());
    }
}

IMPL_LINK(SwEditRegionDlg, DDEHdl, weld::ToggleButton&, rButton, void)
{
    if (!CheckPasswd(&rButton))
        return;
    SectRepr* pSectRepr = reinterpret_cast<SectRepr*>(m_xTree->get_selected_id().toInt64());
    if (pSectRepr)
    {
        bool bFile = m_xFileCB->get_active();
        SwSectionData & rData( pSectRepr->GetSectionData() );
        bool bDDE = rButton.get_active();
        if(bDDE)
        {
            m_xFileNameFT->hide();
            m_xDDECommandFT->set_sensitive(true);
            m_xDDECommandFT->show();
            m_xSubRegionFT->hide();
            m_xSubRegionED->hide();
            if (FILE_LINK_SECTION == rData.GetType())
            {
                pSectRepr->SetFile(OUString());
                m_xFileNameED->set_text(OUString());
                rData.SetLinkFilePassword(OUString());
            }
            rData.SetType(DDE_LINK_SECTION);
        }
        else
        {
            m_xDDECommandFT->hide();
            m_xFileNameFT->set_sensitive(bFile);
            m_xFileNameFT->show();
            m_xSubRegionED->show();
            m_xSubRegionFT->show();
            m_xSubRegionED->set_sensitive(bFile);
            m_xSubRegionFT->set_sensitive(bFile);
            m_xSubRegionED->set_sensitive(bFile);
            if (DDE_LINK_SECTION == rData.GetType())
            {
                rData.SetType(FILE_LINK_SECTION);
                pSectRepr->SetFile(OUString());
                rData.SetLinkFilePassword(OUString());
                m_xFileNameED->set_text(OUString());
            }
        }
        m_xFilePB->set_sensitive(bFile && !bDDE);
    }
}

void SwEditRegionDlg::ChangePasswd(bool bChange)
{
    if (!CheckPasswd())
    {
        if (!bChange)
            m_xPasswdCB->set_active(!m_xPasswdCB->get_active());
        return;
    }

    bool bSet = bChange ? bChange : m_xPasswdCB->get_active();

    m_xTree->selected_foreach([this, bChange, bSet](weld::TreeIter& rEntry){
        SectRepr* pRepr = reinterpret_cast<SectRepr*>(m_xTree->get_id(rEntry).toInt64());
        if(bSet)
        {
            if(!pRepr->GetTempPasswd().getLength() || bChange)
            {
                SfxPasswordDialog aPasswdDlg(m_xDialog.get());
                aPasswdDlg.ShowExtras(SfxShowExtras::CONFIRM);
                if (RET_OK == aPasswdDlg.run())
                {
                    const OUString sNewPasswd(aPasswdDlg.GetPassword());
                    if (aPasswdDlg.GetConfirm() == sNewPasswd)
                    {
                        SvPasswordHelper::GetHashPassword( pRepr->GetTempPasswd(), sNewPasswd );
                    }
                    else
                    {
                        std::unique_ptr<weld::MessageDialog> xInfoBox(Application::CreateMessageDialog(m_xDialog.get(),
                                                                      VclMessageType::Info, VclButtonsType::Ok,
                                                                      SwResId(STR_WRONG_PASSWD_REPEAT)));
                        xInfoBox->run();
                        ChangePasswd(bChange);
                        return true;
                    }
                }
                else
                {
                    if(!bChange)
                        m_xPasswdCB->set_active(false);
                    return true;
                }
            }
            pRepr->GetSectionData().SetPassword(pRepr->GetTempPasswd());
        }
        else
        {
            pRepr->GetSectionData().SetPassword(uno::Sequence<sal_Int8 >());
        }
        return false;
    });
}

IMPL_LINK_NOARG(SwEditRegionDlg, TogglePasswdHdl, weld::ToggleButton&, void)
{
    ChangePasswd(false);
}

IMPL_LINK_NOARG(SwEditRegionDlg, ChangePasswdHdl, weld::Button&, void)
{
    ChangePasswd(true);
}

// the current region name is being added to the TreeListBox immediately during
// editing, with empty string no Ok()
IMPL_LINK_NOARG(SwEditRegionDlg, NameEditHdl, weld::Entry&, void)
{
    if(!CheckPasswd())
        return;
    std::unique_ptr<weld::TreeIter> xIter(m_xTree->make_iterator());
    if (m_xTree->get_selected(xIter.get()))
    {
        const OUString aName = m_xCurName->get_text();
        m_xTree->set_text(*xIter, aName);
        SectRepr* pRepr = reinterpret_cast<SectRepr*>(m_xTree->get_id(*xIter).toInt64());
        pRepr->GetSectionData().SetSectionName(aName);

        m_xOK->set_sensitive(!aName.isEmpty());
    }
}

IMPL_LINK( SwEditRegionDlg, ConditionEditHdl, weld::Entry&, rEdit, void )
{
    int nStartPos, nEndPos;
    rEdit.get_selection_bounds(nStartPos, nEndPos);
    if(!CheckPasswd())
        return;
    rEdit.select_region(nStartPos, nEndPos);

    m_xTree->selected_foreach([this, &rEdit](weld::TreeIter& rEntry){
        SectRepr* pRepr = reinterpret_cast<SectRepr*>(m_xTree->get_id(rEntry).toInt64());
        pRepr->GetSectionData().SetCondition(rEdit.get_text());
        return false;
    });
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
            ::lcl_ReadSections(*pMedium, *m_xSubRegionED);
        }
    }

    SectRepr* pSectRepr = reinterpret_cast<SectRepr*>(m_xTree->get_selected_id().toInt64());
    if (pSectRepr)
    {
        pSectRepr->SetFile( sFileName );
        pSectRepr->SetFilter( sFilterName );
        pSectRepr->GetSectionData().SetLinkFilePassword(sPassword);
        m_xFileNameED->set_text(pSectRepr->GetFile());
    }
}

IMPL_LINK_NOARG(SwEditRegionDlg, SubRegionEventHdl, weld::ComboBox&, void)
{
    if (!m_bSubRegionsFilled)
    {
        //if necessary fill the names bookmarks/sections/tables now

        OUString sFileName = m_xFileNameED->get_text();
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
            ::lcl_ReadSections(aMedium, *m_xSubRegionED);
        }
        else
            lcl_FillSubRegionList(rSh, *m_xSubRegionED, nullptr);
        m_bSubRegionsFilled = true;
    }
}

// helper function - read section names from medium
static void lcl_ReadSections( SfxMedium& rMedium, weld::ComboBox& rBox )
{
    rBox.clear();
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
            rBox.append_text(it);
        }
    }
}

SwInsertSectionTabDialog::SwInsertSectionTabDialog(
            weld::Window* pParent, const SfxItemSet& rSet, SwWrtShell& rSh)
    : SfxTabDialogController(pParent, "modules/swriter/ui/insertsectiondialog.ui",
                             "InsertSectionDialog",&rSet)
    , rWrtSh(rSh)
{
    SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();
    AddTabPage("section", SwInsertSectionTabPage::Create, nullptr);
    AddTabPage("columns",   SwColumnPage::Create, nullptr);
    AddTabPage("background", pFact->GetTabPageCreatorFunc(RID_SVXPAGE_BKG), nullptr);
    AddTabPage("notes", SwSectionFootnoteEndTabPage::Create, nullptr);
    AddTabPage("indents", SwSectionIndentTabPage::Create, nullptr);

    SvxHtmlOptions& rHtmlOpt = SvxHtmlOptions::Get();
    long nHtmlMode = rHtmlOpt.GetExportMode();

    bool bWeb = dynamic_cast<SwWebDocShell*>( rSh.GetView().GetDocShell()  ) != nullptr ;
    if(bWeb)
    {
        RemoveTabPage("notes");
        RemoveTabPage("indents");
        if( HTML_CFG_NS40 != nHtmlMode && HTML_CFG_WRITER != nHtmlMode)
            RemoveTabPage("columns");
    }
    SetCurPageId("section");
}

SwInsertSectionTabDialog::~SwInsertSectionTabDialog()
{
}

void SwInsertSectionTabDialog::PageCreated(const OString& rId, SfxTabPage &rPage)
{
    if (rId == "section")
        static_cast<SwInsertSectionTabPage&>(rPage).SetWrtShell(rWrtSh);
    else if (rId == "background")
    {
        SfxAllItemSet aSet(*(GetInputSetImpl()->GetPool()));
        aSet.Put (SfxUInt32Item(SID_FLAG_TYPE, static_cast<sal_uInt32>(SvxBackgroundTabFlags::SHOW_SELECTOR)));
        rPage.PageCreated(aSet);
    }
    else if (rId == "columns")
    {
        const SwFormatFrameSize& rSize = GetInputSetImpl()->Get(RES_FRM_SIZE);
        static_cast<SwColumnPage&>(rPage).SetPageWidth(rSize.GetWidth());
        static_cast<SwColumnPage&>(rPage).ShowBalance(true);
        static_cast<SwColumnPage&>(rPage).SetInSection(true);
    }
    else if (rId == "indents")
        static_cast<SwSectionIndentTabPage&>(rPage).SetWrtShell(rWrtSh);
}

void SwInsertSectionTabDialog::SetSectionData(SwSectionData const& rSect)
{
    m_pSectionData.reset( new SwSectionData(rSect) );
}

short SwInsertSectionTabDialog::Ok()
{
    short nRet = SfxTabDialogController::Ok();
    OSL_ENSURE(m_pSectionData, "SwInsertSectionTabDialog: no SectionData?");
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

SwInsertSectionTabPage::SwInsertSectionTabPage(TabPageParent pParent, const SfxItemSet &rAttrSet)
    : SfxTabPage(pParent, "modules/swriter/ui/sectionpage.ui", "SectionPage", &rAttrSet)
    , m_pWrtSh(nullptr)
    , m_xCurName(m_xBuilder->weld_entry_tree_view("sectionnames", "sectionnames-entry",
                                                  "sectionnames-list"))
    , m_xFileCB(m_xBuilder->weld_check_button("link"))
    , m_xDDECB(m_xBuilder->weld_check_button("dde"))
    , m_xDDECommandFT(m_xBuilder->weld_label("ddelabel"))
    , m_xFileNameFT(m_xBuilder->weld_label("filelabel"))
    , m_xFileNameED(m_xBuilder->weld_entry("filename"))
    , m_xFilePB(m_xBuilder->weld_button("selectfile"))
    , m_xSubRegionFT(m_xBuilder->weld_label("sectionlabel"))
    , m_xSubRegionED(m_xBuilder->weld_combo_box("sectionname"))
    , m_xProtectCB(m_xBuilder->weld_check_button("protect"))
    , m_xPasswdCB(m_xBuilder->weld_check_button("withpassword"))
    , m_xPasswdPB(m_xBuilder->weld_button("selectpassword"))
    , m_xHideCB(m_xBuilder->weld_check_button("hide"))
    , m_xConditionFT(m_xBuilder->weld_label("condlabel"))
    , m_xConditionED(new SwConditionEdit(m_xBuilder->weld_entry("withcond")))
    // edit in readonly sections
    , m_xEditInReadonlyCB(m_xBuilder->weld_check_button("editable"))
{
    m_xCurName->make_sorted();
    m_xCurName->set_height_request_by_rows(12);
    m_xSubRegionED->make_sorted();

    m_xProtectCB->connect_toggled( LINK( this, SwInsertSectionTabPage, ChangeProtectHdl));
    m_xPasswdCB->connect_toggled( LINK( this, SwInsertSectionTabPage, TogglePasswdHdl));
    m_xPasswdPB->connect_clicked( LINK( this, SwInsertSectionTabPage, ChangePasswdHdl));
    m_xHideCB->connect_toggled( LINK( this, SwInsertSectionTabPage, ChangeHideHdl));
    m_xFileCB->connect_toggled( LINK( this, SwInsertSectionTabPage, UseFileHdl ));
    m_xFilePB->connect_clicked( LINK( this, SwInsertSectionTabPage, FileSearchHdl ));
    m_xCurName->connect_changed( LINK( this, SwInsertSectionTabPage, NameEditHdl));
    m_xDDECB->connect_toggled( LINK( this, SwInsertSectionTabPage, DDEHdl ));
    ChangeProtectHdl(*m_xProtectCB);
    m_xSubRegionED->set_entry_completion(true, true);
}

SwInsertSectionTabPage::~SwInsertSectionTabPage()
{
    disposeOnce();
}

void    SwInsertSectionTabPage::SetWrtShell(SwWrtShell& rSh)
{
    m_pWrtSh = &rSh;

    bool bWeb = dynamic_cast<SwWebDocShell*>( m_pWrtSh->GetView().GetDocShell() )!= nullptr;
    if(bWeb)
    {
        m_xHideCB->hide();
        m_xConditionED->hide();
        m_xConditionFT->hide();
        m_xDDECB->hide();
        m_xDDECommandFT->hide();
    }

    lcl_FillSubRegionList(*m_pWrtSh, *m_xSubRegionED, m_xCurName.get());

    SwSectionData *const pSectionData =
        static_cast<SwInsertSectionTabDialog*>(GetDialogController())
            ->GetSectionData();
    if (pSectionData) // something set?
    {
        const OUString sSectionName(pSectionData->GetSectionName());
        m_xCurName->set_entry_text(rSh.GetUniqueSectionName(&sSectionName));
        m_xProtectCB->set_active( pSectionData->IsProtectFlag() );
        ChangeProtectHdl(*m_xProtectCB);
        m_sFileName = pSectionData->GetLinkFileName();
        m_sFilePasswd = pSectionData->GetLinkFilePassword();
        m_xFileCB->set_active( !m_sFileName.isEmpty() );
        m_xFileNameED->set_text( m_sFileName );
        UseFileHdl(*m_xFileCB);
    }
    else
    {
        m_xCurName->set_entry_text(rSh.GetUniqueSectionName());
    }
}

bool SwInsertSectionTabPage::FillItemSet( SfxItemSet* )
{
    SwSectionData aSection(CONTENT_SECTION, m_xCurName->get_active_text());
    aSection.SetCondition(m_xConditionED->get_text());
    bool bProtected = m_xProtectCB->get_active();
    aSection.SetProtectFlag(bProtected);
    aSection.SetHidden(m_xHideCB->get_active());
    // edit in readonly sections
    aSection.SetEditInReadonlyFlag(m_xEditInReadonlyCB->get_active());

    if(bProtected)
    {
        aSection.SetPassword(m_aNewPasswd);
    }
    const OUString sFileName = m_xFileNameED->get_text();
    const OUString sSubRegion = m_xSubRegionED->get_active_text();
    bool bDDe = m_xDDECB->get_active();
    if (m_xFileCB->get_active() && (!sFileName.isEmpty() || !sSubRegion.isEmpty() || bDDe))
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
            aSection.SetType( m_xDDECB->get_active() ?
                                    DDE_LINK_SECTION :
                                        FILE_LINK_SECTION);
        }
    }
    static_cast<SwInsertSectionTabDialog*>(GetDialogController())->SetSectionData(aSection);
    return true;
}

void SwInsertSectionTabPage::Reset( const SfxItemSet* )
{
}

VclPtr<SfxTabPage> SwInsertSectionTabPage::Create(TabPageParent pParent,
                                                  const SfxItemSet* rAttrSet)
{
    return VclPtr<SwInsertSectionTabPage>::Create(pParent, *rAttrSet);
}

IMPL_LINK(SwInsertSectionTabPage, ChangeHideHdl, weld::ToggleButton&, rBox, void)
{
    bool bHide = rBox.get_active();
    m_xConditionED->set_sensitive(bHide);
    m_xConditionFT->set_sensitive(bHide);
}

IMPL_LINK(SwInsertSectionTabPage, ChangeProtectHdl, weld::ToggleButton&, rBox, void)
{
    bool bCheck = rBox.get_active();
    m_xPasswdCB->set_sensitive(bCheck);
    m_xPasswdPB->set_sensitive(bCheck);
}

void SwInsertSectionTabPage::ChangePasswd(bool bChange)
{
    bool bSet = bChange ? bChange : m_xPasswdCB->get_active();
    if (bSet)
    {
        if(!m_aNewPasswd.getLength() || bChange)
        {
            SfxPasswordDialog aPasswdDlg(GetDialogFrameWeld());
            aPasswdDlg.ShowExtras(SfxShowExtras::CONFIRM);
            if (RET_OK == aPasswdDlg.run())
            {
                const OUString sNewPasswd(aPasswdDlg.GetPassword());
                if (aPasswdDlg.GetConfirm() == sNewPasswd)
                {
                    SvPasswordHelper::GetHashPassword( m_aNewPasswd, sNewPasswd );
                }
                else
                {
                    std::unique_ptr<weld::MessageDialog> xInfoBox(Application::CreateMessageDialog(GetDialogFrameWeld(),
                                                                  VclMessageType::Info, VclButtonsType::Ok,
                                                                  SwResId(STR_WRONG_PASSWD_REPEAT)));
                    xInfoBox->run();
                }
            }
            else if(!bChange)
                m_xPasswdCB->set_active(false);
        }
    }
    else
        m_aNewPasswd.realloc(0);
}

IMPL_LINK_NOARG(SwInsertSectionTabPage, TogglePasswdHdl, weld::ToggleButton&, void)
{
    ChangePasswd(false);
}

IMPL_LINK_NOARG(SwInsertSectionTabPage, ChangePasswdHdl, weld::Button&, void)
{
    ChangePasswd(true);
}


IMPL_LINK_NOARG(SwInsertSectionTabPage, NameEditHdl, weld::ComboBox&, void)
{
    const OUString aName = m_xCurName->get_active_text();
    GetDialogController()->GetOKButton().set_sensitive(!aName.isEmpty() &&
            m_xCurName->find_text(aName) == -1);
}

IMPL_LINK(SwInsertSectionTabPage, UseFileHdl, weld::ToggleButton&, rButton, void)
{
    if (rButton.get_active())
    {
        if (m_pWrtSh->HasSelection())
        {
            std::unique_ptr<weld::MessageDialog> xQueryBox(Application::CreateMessageDialog(GetDialogFrameWeld(),
                                                           VclMessageType::Question, VclButtonsType::YesNo,
                                                           SwResId(STR_QUERY_CONNECT)));
            if (RET_NO == xQueryBox->run())
                rButton.set_active(false);
        }
    }

    bool bFile = rButton.get_active();
    m_xFileNameFT->set_sensitive(bFile);
    m_xFileNameED->set_sensitive(bFile);
    m_xFilePB->set_sensitive(bFile);
    m_xSubRegionFT->set_sensitive(bFile);
    m_xSubRegionED->set_sensitive(bFile);
    m_xDDECommandFT->set_sensitive(bFile);
    m_xDDECB->set_sensitive(bFile);
    if (bFile)
    {
        m_xFileNameED->grab_focus();
        m_xProtectCB->set_active(true);
        ChangeProtectHdl(*m_xProtectCB);
    }
    else
    {
        m_xDDECB->set_active(false);
        DDEHdl(*m_xDDECB);
    }
}

IMPL_LINK_NOARG(SwInsertSectionTabPage, FileSearchHdl, weld::Button&, void)
{
    m_pDocInserter.reset(new ::sfx2::DocumentInserter(GetDialogFrameWeld(), "swriter"));
    m_pDocInserter->StartExecuteModal( LINK( this, SwInsertSectionTabPage, DlgClosedHdl ) );
}

IMPL_LINK( SwInsertSectionTabPage, DDEHdl, weld::ToggleButton&, rButton, void )
{
    bool bDDE = rButton.get_active();
    bool bFile = m_xFileCB->get_active();
    m_xFilePB->set_sensitive(!bDDE && bFile);
    if (bDDE)
    {
        m_xFileNameFT->hide();
        m_xDDECommandFT->set_sensitive(bDDE);
        m_xDDECommandFT->show();
        m_xSubRegionFT->hide();
        m_xSubRegionED->hide();
        m_xFileNameED->set_accessible_name(m_xDDECommandFT->get_label());
    }
    else
    {
        m_xDDECommandFT->hide();
        m_xFileNameFT->set_sensitive(bFile);
        m_xFileNameFT->show();
        m_xSubRegionFT->show();
        m_xSubRegionED->show();
        m_xSubRegionED->set_sensitive(bFile);
        m_xFileNameED->set_accessible_name(m_xFileNameFT->get_label());
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
            m_xFileNameED->set_text( INetURLObject::decode(
                m_sFileName, INetURLObject::DecodeMechanism::Unambiguous ) );
            ::lcl_ReadSections(*pMedium, *m_xSubRegionED);
        }
    }
    else
    {
        m_sFilterName.clear();
        m_sFilePasswd.clear();
    }
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
    , m_xFootnoteNumViewBox(new SwNumberingTypeListBox(m_xBuilder->weld_combo_box("ftnnumviewbox")))
    , m_xFootnoteSuffixFT(m_xBuilder->weld_label("ftnsuffix_label"))
    , m_xFootnoteSuffixED(m_xBuilder->weld_entry("ftnsuffix"))
    , m_xEndNtAtTextEndCB(m_xBuilder->weld_check_button("endntattextend"))
    , m_xEndNtNumCB(m_xBuilder->weld_check_button("endntnum"))
    , m_xEndOffsetLbl(m_xBuilder->weld_label("endoffset_label"))
    , m_xEndOffsetField(m_xBuilder->weld_spin_button("endoffset"))
    , m_xEndNtNumFormatCB(m_xBuilder->weld_check_button("endntnumfmt"))
    , m_xEndPrefixFT(m_xBuilder->weld_label("endprefix_label"))
    , m_xEndPrefixED(m_xBuilder->weld_entry("endprefix"))
    , m_xEndNumViewBox(new SwNumberingTypeListBox(m_xBuilder->weld_combo_box("endnumviewbox")))
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
        [[fallthrough]];

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
        [[fallthrough]];

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
        [[fallthrough]];

    case FTNEND_ATTXTEND_OWNNUMSEQ:
        pNtNumCB->set_state( TRISTATE_TRUE );
        [[fallthrough]];

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
        [[fallthrough]];

    case FTNEND_ATTXTEND:
        pNtNumFormatCB->set_sensitive( false );
        pOffsetField->set_sensitive( false );
        pOffsetText->set_sensitive( false );
        [[fallthrough]];

    case FTNEND_ATTXTEND_OWNNUMSEQ:
        pNumViewBox->set_sensitive( false );
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
    pNumViewBox->set_sensitive( bEnableNumFormat );
    pPrefixED->set_sensitive( bEnableNumFormat );
    pSuffixED->set_sensitive( bEnableNumFormat );
    pPrefixFT->set_sensitive( bEnableNumFormat );
    pSuffixFT->set_sensitive( bEnableNumFormat );
}

SwSectionPropertyTabDialog::SwSectionPropertyTabDialog(
    weld::Window* pParent, const SfxItemSet& rSet, SwWrtShell& rSh)
    : SfxTabDialogController(pParent, "modules/swriter/ui/formatsectiondialog.ui",
                             "FormatSectionDialog", &rSet)
    , rWrtSh(rSh)
{
    SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();
    AddTabPage("columns",   SwColumnPage::Create, nullptr);
    AddTabPage("background", pFact->GetTabPageCreatorFunc(RID_SVXPAGE_BKG), nullptr);
    AddTabPage("notes", SwSectionFootnoteEndTabPage::Create, nullptr);
    AddTabPage("indents", SwSectionIndentTabPage::Create, nullptr);

    SvxHtmlOptions& rHtmlOpt = SvxHtmlOptions::Get();
    long nHtmlMode = rHtmlOpt.GetExportMode();
    bool bWeb = dynamic_cast<SwWebDocShell*>( rSh.GetView().GetDocShell()  ) != nullptr ;
    if(bWeb)
    {
        RemoveTabPage("notes");
        RemoveTabPage("indents");
        if( HTML_CFG_NS40 != nHtmlMode && HTML_CFG_WRITER != nHtmlMode)
            RemoveTabPage("columns");
    }
}

SwSectionPropertyTabDialog::~SwSectionPropertyTabDialog()
{
}

void SwSectionPropertyTabDialog::PageCreated(const OString& rId, SfxTabPage &rPage)
{
    if (rId == "background")
    {
        SfxAllItemSet aSet(*(GetInputSetImpl()->GetPool()));
        aSet.Put (SfxUInt32Item(SID_FLAG_TYPE, static_cast<sal_uInt32>(SvxBackgroundTabFlags::SHOW_SELECTOR)));
        rPage.PageCreated(aSet);
    }
    else if (rId == "columns")
    {
        static_cast<SwColumnPage&>(rPage).ShowBalance(true);
        static_cast<SwColumnPage&>(rPage).SetInSection(true);
    }
    else if (rId == "indents")
        static_cast<SwSectionIndentTabPage&>(rPage).SetWrtShell(rWrtSh);
}

SwSectionIndentTabPage::SwSectionIndentTabPage(TabPageParent pParent, const SfxItemSet &rAttrSet)
    : SfxTabPage(pParent, "modules/swriter/ui/indentpage.ui", "IndentPage", &rAttrSet)
    , m_xBeforeMF(m_xBuilder->weld_metric_spin_button("before", FieldUnit::CM))
    , m_xAfterMF(m_xBuilder->weld_metric_spin_button("after", FieldUnit::CM))
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
                m_xBeforeMF->denormalize(m_xBeforeMF->get_value(FieldUnit::TWIP)) ,
                m_xAfterMF->denormalize(m_xAfterMF->get_value(FieldUnit::TWIP)), 0, 0, RES_LR_SPACE);
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

        m_xBeforeMF->set_value(m_xBeforeMF->normalize(rSpace.GetLeft()), FieldUnit::TWIP);
        m_xAfterMF->set_value(m_xAfterMF->normalize(rSpace.GetRight()), FieldUnit::TWIP);
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
    m_aPreviewWin.SetLeftMargin(m_xBeforeMF->denormalize(m_xBeforeMF->get_value(FieldUnit::TWIP)));
    m_aPreviewWin.SetRightMargin(m_xAfterMF->denormalize(m_xAfterMF->get_value(FieldUnit::TWIP)));
    m_aPreviewWin.Invalidate();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
