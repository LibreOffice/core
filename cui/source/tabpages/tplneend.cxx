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

#include <tools/urlobj.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>
#include <unotools/pathoptions.hxx>
#include <sfx2/filedlghelper.hxx>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>

#include <strings.hrc>
#include <svx/dialmgr.hxx>
#include <svx/dlgctrl.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdopath.hxx>
#include <svx/xtable.hxx>
#include <svx/xlineit0.hxx>
#include <svx/xlnwtit.hxx>
#include <svx/xlnclit.hxx>
#include <svx/xlnstwit.hxx>
#include <svx/xlnedwit.hxx>
#include <svx/xlnstit.hxx>
#include <svx/xlnedit.hxx>
#include <cuitabline.hxx>
#include <cuitabarea.hxx>
#include <svx/svxdlg.hxx>
#include <dialmgr.hxx>
#include <basegfx/range/b2drange.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <svx/strings.hrc>
#include <osl/diagnose.h>

#define XOUT_WIDTH    150

SvxLineEndDefTabPage::SvxLineEndDefTabPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rInAttrs)
    : SfxTabPage(pPage, pController, u"cui/ui/lineendstabpage.ui"_ustr, u"LineEndPage"_ustr, &rInAttrs)
    , rOutAttrs(rInAttrs)
    , pPolyObj(nullptr)
    , aXLineAttr(rInAttrs.GetPool())
    , rXLSet(aXLineAttr.GetItemSet())
    , pnLineEndListState(nullptr)
    , pPageType(nullptr)
    , nDlgType(0)
    , pPosLineEndLb(nullptr)
    , m_xEdtName(m_xBuilder->weld_entry(u"EDT_NAME"_ustr))
    , m_xLbLineEnds(new SvxLineEndLB(m_xBuilder->weld_combo_box(u"LB_LINEENDS"_ustr)))
    , m_xBtnAdd(m_xBuilder->weld_button(u"BTN_ADD"_ustr))
    , m_xBtnModify(m_xBuilder->weld_button(u"BTN_MODIFY"_ustr))
    , m_xBtnDelete(m_xBuilder->weld_button(u"BTN_DELETE"_ustr))
    , m_xBtnLoad(m_xBuilder->weld_button(u"BTN_LOAD"_ustr))
    , m_xBtnSave(m_xBuilder->weld_button(u"BTN_SAVE"_ustr))
    , m_xCtlPreview(new weld::CustomWeld(*m_xBuilder, u"CTL_PREVIEW"_ustr, m_aCtlPreview))
{
    // this page needs ExchangeSupport
    SetExchangeSupport();

    rXLSet.Put( XLineStyleItem(css::drawing::LineStyle_SOLID) );
    rXLSet.Put( XLineWidthItem(XOUT_WIDTH) );
    rXLSet.Put( XLineColorItem( OUString(), COL_BLACK ) );
    rXLSet.Put( XLineStartWidthItem( m_aCtlPreview.GetOutputSize().Height()  / 2 ) );
    rXLSet.Put( XLineEndWidthItem( m_aCtlPreview.GetOutputSize().Height() / 2 ) );

    // #i34740#
    m_aCtlPreview.SetLineAttributes(aXLineAttr.GetItemSet());

    m_xBtnAdd->connect_clicked(LINK(this, SvxLineEndDefTabPage, ClickAddHdl_Impl));
    m_xBtnModify->connect_clicked(LINK( this, SvxLineEndDefTabPage, ClickModifyHdl_Impl));
    m_xBtnDelete->connect_clicked(LINK( this, SvxLineEndDefTabPage, ClickDeleteHdl_Impl));
    m_xBtnLoad->connect_clicked(LINK( this, SvxLineEndDefTabPage, ClickLoadHdl_Impl));
    m_xBtnSave->connect_clicked(LINK( this, SvxLineEndDefTabPage, ClickSaveHdl_Impl));

    m_xLbLineEnds->connect_changed(LINK(this, SvxLineEndDefTabPage, SelectLineEndHdl_Impl));
}

SvxLineEndDefTabPage::~SvxLineEndDefTabPage()
{
    m_xCtlPreview.reset();
    m_xLbLineEnds.reset();
}

void SvxLineEndDefTabPage::Construct()
{
    m_xLbLineEnds->Fill( pLineEndList );

    bool bCreateArrowPossible = true;

    if( !pPolyObj )
    {
        bCreateArrowPossible = false;
    }
    else if( nullptr == dynamic_cast<const SdrPathObj*>( pPolyObj) )
    {
        SdrObjTransformInfoRec aInfoRec;
        pPolyObj->TakeObjInfo( aInfoRec );
        rtl::Reference<SdrObject> pNewObj;
        if( aInfoRec.bCanConvToPath )
            pNewObj = pPolyObj->ConvertToPolyObj( true, false );

        bCreateArrowPossible = nullptr != dynamic_cast<const SdrPathObj*>( pNewObj.get());
    }

    if( !bCreateArrowPossible )
        m_xBtnAdd->set_sensitive(false);
}

void SvxLineEndDefTabPage::ActivatePage( const SfxItemSet& )
{
    if( nDlgType != 0 ) // area dialog
        return;

    // ActivatePage() is called before the dialog receives PageCreated() !!!
    if( !pLineEndList.is() )
        return;

    if( *pPosLineEndLb != -1)
    {
        m_xLbLineEnds->set_active(*pPosLineEndLb);
        SelectLineEndHdl_Impl();
    }
    INetURLObject   aURL( pLineEndList->GetPath() );

    aURL.Append( pLineEndList->GetName() );
    DBG_ASSERT( aURL.GetProtocol() != INetProtocol::NotValid, "invalid URL" );
    *pPageType = PageType::Area; // 3
    *pPosLineEndLb = -1;
}


DeactivateRC SvxLineEndDefTabPage::DeactivatePage( SfxItemSet* _pSet )
{
    CheckChanges_Impl();

    if( _pSet )
        FillItemSet( _pSet );

    return DeactivateRC::LeavePage;
}


void SvxLineEndDefTabPage::CheckChanges_Impl()
{
    int nPos = m_xLbLineEnds->get_active();

    if (nPos != -1)
    {
        OUString aString = m_xEdtName->get_text();

        if( aString != m_xLbLineEnds->get_active_text() )
        {
            std::unique_ptr<weld::Builder> xBuilder(Application::CreateBuilder(GetFrameWeld(), u"cui/ui/querychangelineenddialog.ui"_ustr));
            std::unique_ptr<weld::MessageDialog> xQueryBox(xBuilder->weld_message_dialog(u"AskChangeLineEndDialog"_ustr));
            if (xQueryBox->run() == RET_YES)
                ClickModifyHdl_Impl(*m_xBtnModify);
        }
    }
    nPos = m_xLbLineEnds->get_active();

    if (nPos != -1)
        *pPosLineEndLb = nPos;
}


bool SvxLineEndDefTabPage::FillItemSet( SfxItemSet* rSet )
{
    if( nDlgType == 0 ) // line dialog
    {
        if( *pPageType == PageType::Bitmap )
        {
            CheckChanges_Impl();

            int nPos = m_xLbLineEnds->get_active();
            const XLineEndEntry* pEntry = pLineEndList->GetLineEnd(nPos);

            rSet->Put( XLineStartItem( pEntry->GetName(), pEntry->GetLineEnd() ) );
            rSet->Put( XLineEndItem( pEntry->GetName(), pEntry->GetLineEnd() ) );
        }
    }
    return true;
}

void SvxLineEndDefTabPage::Reset( const SfxItemSet* )
{
    m_xLbLineEnds->set_active(0);

    // Update lineend
    if( pLineEndList->Count() > 0 )
    {
        int nPos = m_xLbLineEnds->get_active();

        const XLineEndEntry* pEntry = pLineEndList->GetLineEnd(nPos);

        m_xEdtName->set_text(m_xLbLineEnds->get_active_text());

        rXLSet.Put( XLineStartItem( OUString(), pEntry->GetLineEnd() ) );
        rXLSet.Put( XLineEndItem( OUString(), pEntry->GetLineEnd() ) );

        // #i34740#
        m_aCtlPreview.SetLineAttributes(aXLineAttr.GetItemSet());
        m_aCtlPreview.Invalidate();
    }

    // determine button state
    if( pLineEndList->Count() )
    {
        m_xBtnModify->set_sensitive(true);
        m_xBtnDelete->set_sensitive(true);
        m_xBtnSave->set_sensitive(true);
    }
    else
    {
        m_xBtnModify->set_sensitive(false);
        m_xBtnDelete->set_sensitive(false);
        m_xBtnSave->set_sensitive(false);
    }
}

std::unique_ptr<SfxTabPage> SvxLineEndDefTabPage::Create(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rSet)
{
    return std::make_unique<SvxLineEndDefTabPage>(pPage, pController, *rSet );
}

void SvxLineEndDefTabPage::SelectLineEndHdl_Impl()
{
    if( pLineEndList->Count() <= 0 )
        return;

    int nPos = m_xLbLineEnds->get_active();

    const XLineEndEntry* pEntry = pLineEndList->GetLineEnd(nPos);

    m_xEdtName->set_text(m_xLbLineEnds->get_active_text());

    rXLSet.Put( XLineStartItem( OUString(), pEntry->GetLineEnd() ) );
    rXLSet.Put( XLineEndItem( OUString(), pEntry->GetLineEnd() ) );

    // #i34740#
    m_aCtlPreview.SetLineAttributes(aXLineAttr.GetItemSet());
    m_aCtlPreview.Invalidate();

    // Is not set before, in order to only take the new style,
    // if there is an entry selected in the ListBox
    *pPageType = PageType::Bitmap;
}

IMPL_LINK_NOARG(SvxLineEndDefTabPage, SelectLineEndHdl_Impl, weld::ComboBox&, void)
{
    SelectLineEndHdl_Impl();
}

IMPL_LINK_NOARG(SvxLineEndDefTabPage, ClickModifyHdl_Impl, weld::Button&, void)
{
    int nPos = m_xLbLineEnds->get_active();
    if (nPos == -1)
        return;

    OUString aDesc(CuiResId(RID_CUISTR_DESC_LINEEND));
    OUString aName(m_xEdtName->get_text());
    tools::Long nCount = pLineEndList->Count();
    bool bDifferent = true;

    // check whether the name is existing already
    for ( tools::Long i = 0; i < nCount && bDifferent; i++ )
        if ( aName == pLineEndList->GetLineEnd( i )->GetName() )
            bDifferent = false;

    // if yes, repeat and demand a new name
    if ( !bDifferent )
    {
        std::unique_ptr<weld::Builder> xBuilder(Application::CreateBuilder(GetFrameWeld(), u"cui/ui/queryduplicatedialog.ui"_ustr));
        std::unique_ptr<weld::MessageDialog> xWarningBox(xBuilder->weld_message_dialog(u"DuplicateNameDialog"_ustr));
        xWarningBox->run();

        SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
        ScopedVclPtr<AbstractSvxNameDialog> pDlg(pFact->CreateSvxNameDialog(GetFrameWeld(), aName, aDesc));
        bool bLoop = true;

        while( !bDifferent && bLoop && pDlg->Execute() == RET_OK )
        {
            aName = pDlg->GetName();
            bDifferent = true;

            for( tools::Long i = 0; i < nCount && bDifferent; i++ )
            {
                if( aName == pLineEndList->GetLineEnd( i )->GetName() )
                    bDifferent = false;
            }

            if( bDifferent )
                bLoop = false;
            else
                xWarningBox->run();
        }
    }

    // if not existing, enter the entry
    if( !bDifferent )
        return;

    const XLineEndEntry* pOldEntry = pLineEndList->GetLineEnd(nPos);

    if(pOldEntry)
    {
        // #123497# Need to replace the existing entry with a new one
        pLineEndList->Replace(std::make_unique<XLineEndEntry>(pOldEntry->GetLineEnd(), aName), nPos);

        m_xEdtName->set_text(aName);

        m_xLbLineEnds->Modify(*pLineEndList->GetLineEnd(nPos), nPos, pLineEndList->GetUiBitmap(nPos));
        m_xLbLineEnds->set_active(nPos);

        // set flag for modified
        *pnLineEndListState |= ChangeType::MODIFIED;

        *pPageType = PageType::Bitmap;
    }
    else
    {
        OSL_ENSURE(false, "LineEnd to be modified not existing (!)");
    }
}

IMPL_LINK_NOARG(SvxLineEndDefTabPage, ClickAddHdl_Impl, weld::Button&, void)
{
    if( pPolyObj )
    {
        const SdrObject* pNewObj;
        rtl::Reference<SdrObject> pConvPolyObj;

        if( nullptr != dynamic_cast<const SdrPathObj*>( pPolyObj) )
        {
            pNewObj = pPolyObj;
        }
        else
        {
            SdrObjTransformInfoRec aInfoRec;
            pPolyObj->TakeObjInfo( aInfoRec );

            if( aInfoRec.bCanConvToPath )
            {
                pConvPolyObj = pPolyObj->ConvertToPolyObj( true, false );
                pNewObj = pConvPolyObj.get();

                if( !pNewObj || nullptr == dynamic_cast<const SdrPathObj*>( pNewObj) )
                    return; // cancel, additional safety, which
                            // has no use for group objects though.
            }
            else return; // cancel
        }

        basegfx::B2DPolyPolygon aNewPolyPolygon(static_cast<const SdrPathObj*>(pNewObj)->GetPathPoly());
        basegfx::B2DRange aNewRange(basegfx::utils::getRange(aNewPolyPolygon));

        // normalize
        aNewPolyPolygon.transform(basegfx::utils::createTranslateB2DHomMatrix( -aNewRange.getMinX(), -aNewRange.getMinY()));

        pConvPolyObj.clear();

        OUString aNewName(SvxResId(RID_SVXSTR_LINEEND));
        OUString aDesc(CuiResId(RID_CUISTR_DESC_LINEEND));
        OUString aName;

        tools::Long nCount = pLineEndList->Count();
        tools::Long j = 1;
        bool bDifferent = false;

        while ( !bDifferent )
        {
            aName = aNewName + " " + OUString::number( j++ );
            bDifferent = true;

            for( tools::Long i = 0; i < nCount && bDifferent; i++ )
                if ( aName == pLineEndList->GetLineEnd( i )->GetName() )
                    bDifferent = false;
        }

        SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
        ScopedVclPtr<AbstractSvxNameDialog> pDlg(pFact->CreateSvxNameDialog(GetFrameWeld(), aName, aDesc ));
        bool bLoop = true;

        while ( bLoop && pDlg->Execute() == RET_OK )
        {
            aName = pDlg->GetName();
            bDifferent = true;

            for( tools::Long i = 0; i < nCount && bDifferent; i++ )
            {
                if( aName == pLineEndList->GetLineEnd( i )->GetName() )
                    bDifferent = false;
            }

            if( bDifferent )
            {
                bLoop = false;

                auto nLineEndCount = pLineEndList->Count();
                pLineEndList->Insert(std::make_unique<XLineEndEntry>(aNewPolyPolygon, aName), nLineEndCount);

                // add to the ListBox
                m_xLbLineEnds->Append(*pLineEndList->GetLineEnd(nLineEndCount), pLineEndList->GetUiBitmap(nLineEndCount));
                m_xLbLineEnds->set_active(m_xLbLineEnds->get_count() - 1);

                *pnLineEndListState |= ChangeType::MODIFIED;

                SelectLineEndHdl_Impl();
            }
            else
            {
                std::unique_ptr<weld::Builder> xBuilder(Application::CreateBuilder(GetFrameWeld(), u"cui/ui/queryduplicatedialog.ui"_ustr));
                std::unique_ptr<weld::MessageDialog> xWarningBox(xBuilder->weld_message_dialog(u"DuplicateNameDialog"_ustr));
                xWarningBox->run();
            }
        }
    }
    else
        m_xBtnAdd->set_sensitive(false);

    // determine button state
    if ( pLineEndList->Count() )
    {
        m_xBtnModify->set_sensitive(true);
        m_xBtnDelete->set_sensitive(true);
        m_xBtnSave->set_sensitive(true);
    }
}

IMPL_LINK_NOARG(SvxLineEndDefTabPage, ClickDeleteHdl_Impl, weld::Button&, void)
{
    int nPos = m_xLbLineEnds->get_active();

    if (nPos != -1)
    {
        std::unique_ptr<weld::Builder> xBuilder(Application::CreateBuilder(GetFrameWeld(), u"cui/ui/querydeletelineenddialog.ui"_ustr));
        std::unique_ptr<weld::MessageDialog> xQueryBox(xBuilder->weld_message_dialog(u"AskDelLineEndDialog"_ustr));

        if (xQueryBox->run() == RET_YES)
        {
            pLineEndList->Remove(nPos);
            m_xLbLineEnds->remove(nPos);
            m_xLbLineEnds->set_active(0);

            SelectLineEndHdl_Impl();
            *pPageType = PageType::Area; // LineEnd shall not be taken over

            *pnLineEndListState |= ChangeType::MODIFIED;

            m_aCtlPreview.Invalidate();
        }
    }
    // determine button state
    if( !pLineEndList->Count() )
    {
        m_xBtnModify->set_sensitive(false);
        m_xBtnDelete->set_sensitive(false);
        m_xBtnSave->set_sensitive(false);
    }
}

IMPL_LINK_NOARG(SvxLineEndDefTabPage, ClickLoadHdl_Impl, weld::Button&, void)
{
    sal_uInt16 nReturn = RET_YES;

    if ( *pnLineEndListState & ChangeType::MODIFIED )
    {
        std::unique_ptr<weld::Builder> xBuilder(Application::CreateBuilder(GetFrameWeld(), u"cui/ui/querysavelistdialog.ui"_ustr));
        std::unique_ptr<weld::MessageDialog> xBox(xBuilder->weld_message_dialog(u"AskSaveList"_ustr));

        nReturn = xBox->run();

        if ( nReturn == RET_YES )
            pLineEndList->Save();
    }

    if ( nReturn != RET_CANCEL )
    {
        ::sfx2::FileDialogHelper aDlg(css::ui::dialogs::TemplateDescription::FILEOPEN_SIMPLE,
                                      FileDialogFlags::NONE, GetFrameWeld());
        OUString aStrFilterType( u"*.soe"_ustr );
        aDlg.AddFilter( aStrFilterType, aStrFilterType );

        OUString aPalettePath(SvtPathOptions().GetPalettePath());
        OUString aLastDir;
        sal_Int32 nIndex = 0;
        do
        {
            aLastDir = aPalettePath.getToken(0, ';', nIndex);
        }
        while (nIndex >= 0);

        INetURLObject aFile(aLastDir);
        aDlg.SetDisplayDirectory( aFile.GetMainURL( INetURLObject::DecodeMechanism::NONE ) );

        if( aDlg.Execute() == ERRCODE_NONE )
        {
            INetURLObject aURL( aDlg.GetPath() );
            INetURLObject aPathURL( aURL );

            aPathURL.removeSegment();
            aPathURL.removeFinalSlash();

            XLineEndListRef pLeList = XPropertyList::AsLineEndList(
                XPropertyList::CreatePropertyList(
                    XPropertyListType::LineEnd,
                    aPathURL.GetMainURL(INetURLObject::DecodeMechanism::NONE), u""_ustr));
            pLeList->SetName( aURL.getName() );
            if( pLeList->Load() )
            {
                pLineEndList = pLeList;
                static_cast<SvxLineTabDialog*>(GetDialogController())->SetNewLineEndList( pLineEndList );
                m_xLbLineEnds->clear();
                m_xLbLineEnds->Fill( pLineEndList );
                Reset( &rOutAttrs );

                pLineEndList->SetName( aURL.getName() );

                *pnLineEndListState |= ChangeType::CHANGED;
                *pnLineEndListState &= ~ChangeType::MODIFIED;
            }
            else
            {
                std::unique_ptr<weld::Builder> xBuilder(Application::CreateBuilder(GetFrameWeld(), u"cui/ui/querynoloadedfiledialog.ui"_ustr));
                std::unique_ptr<weld::MessageDialog> xBox(xBuilder->weld_message_dialog(u"NoLoadedFileDialog"_ustr));
                xBox->run();
            }
        }
    }

    // determine button state
    if ( pLineEndList->Count() )
    {
        m_xBtnModify->set_sensitive(true);
        m_xBtnDelete->set_sensitive(true);
        m_xBtnSave->set_sensitive(true);
    }
    else
    {
        m_xBtnModify->set_sensitive(false);
        m_xBtnDelete->set_sensitive(false);
        m_xBtnSave->set_sensitive(false);
    }
}

IMPL_LINK_NOARG(SvxLineEndDefTabPage, ClickSaveHdl_Impl, weld::Button&, void)
{
    ::sfx2::FileDialogHelper aDlg(css::ui::dialogs::TemplateDescription::FILESAVE_SIMPLE, FileDialogFlags::NONE, GetFrameWeld());
    OUString aStrFilterType( u"*.soe"_ustr );
    aDlg.AddFilter( aStrFilterType, aStrFilterType );

    OUString aPalettePath(SvtPathOptions().GetPalettePath());
    OUString aLastDir;
    sal_Int32 nIndex = 0;
    do
    {
        aLastDir = aPalettePath.getToken(0, ';', nIndex);
    }
    while (nIndex >= 0);

    INetURLObject aFile(aLastDir);
    DBG_ASSERT( aFile.GetProtocol() != INetProtocol::NotValid, "invalid URL" );

    if( !pLineEndList->GetName().isEmpty() )
    {
        aFile.Append( pLineEndList->GetName() );

        if( aFile.getExtension().isEmpty() )
            aFile.SetExtension( u"soe" );
    }

    aDlg.SetDisplayDirectory( aFile.GetMainURL( INetURLObject::DecodeMechanism::NONE ) );
    if ( aDlg.Execute() != ERRCODE_NONE )
        return;

    INetURLObject   aURL( aDlg.GetPath() );
    INetURLObject   aPathURL( aURL );

    aPathURL.removeSegment();
    aPathURL.removeFinalSlash();

    pLineEndList->SetName( aURL.getName() );
    pLineEndList->SetPath( aPathURL.GetMainURL( INetURLObject::DecodeMechanism::NONE ) );

    if( pLineEndList->Save() )
    {
        *pnLineEndListState &= ~ChangeType::MODIFIED;
    }
    else
    {
        std::unique_ptr<weld::Builder> xBuilder(Application::CreateBuilder(GetFrameWeld(), u"cui/ui/querynosavefiledialog.ui"_ustr));
        std::unique_ptr<weld::MessageDialog> xBox(xBuilder->weld_message_dialog(u"NoSaveFileDialog"_ustr));
        xBox->run();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
