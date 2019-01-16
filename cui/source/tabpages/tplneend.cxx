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
#include <vcl/settings.hxx>
#include <vcl/weld.hxx>
#include <unotools/pathoptions.hxx>
#include <sfx2/app.hxx>
#include <sfx2/filedlghelper.hxx>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>

#include <strings.hrc>
#include <svx/dialmgr.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdopath.hxx>
#include <svx/drawitem.hxx>
#include <svx/xpool.hxx>
#include <svx/xtable.hxx>
#include <cuitabline.hxx>
#include <cuitabarea.hxx>
#include <svx/svxdlg.hxx>
#include <dialmgr.hxx>
#include <svx/dlgutil.hxx>
#include <basegfx/range/b2drange.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <svx/dialogs.hrc>
#include <svx/strings.hrc>

#define XOUT_WIDTH    150

SvxLineEndDefTabPage::SvxLineEndDefTabPage(TabPageParent pParent, const SfxItemSet& rInAttrs)
    : SfxTabPage(pParent , "cui/ui/lineendstabpage.ui", "LineEndPage", &rInAttrs)
    , rOutAttrs(rInAttrs)
    , pPolyObj(nullptr)
    , aXLineAttr(rInAttrs.GetPool())
    , rXLSet(aXLineAttr.GetItemSet())
    , pnLineEndListState(nullptr)
    , pPageType(nullptr)
    , nDlgType(0)
    , pPosLineEndLb(nullptr)
    , m_xEdtName(m_xBuilder->weld_entry("EDT_NAME"))
    , m_xLbLineEnds(new SvxLineEndLB(m_xBuilder->weld_combo_box("LB_LINEENDS")))
    , m_xBtnAdd(m_xBuilder->weld_button("BTN_ADD"))
    , m_xBtnModify(m_xBuilder->weld_button("BTN_MODIFY"))
    , m_xBtnDelete(m_xBuilder->weld_button("BTN_DELETE"))
    , m_xBtnLoad(m_xBuilder->weld_button("BTN_LOAD"))
    , m_xBtnSave(m_xBuilder->weld_button("BTN_SAVE"))
    , m_xCtlPreview(new weld::CustomWeld(*m_xBuilder, "CTL_PREVIEW", m_aCtlPreview))
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
    disposeOnce();
}

void SvxLineEndDefTabPage::dispose()
{
    m_xCtlPreview.reset();
    m_xLbLineEnds.reset();
    SfxTabPage::dispose();
}

void SvxLineEndDefTabPage::Resize()
{
    rXLSet.Put(XLineStartWidthItem(m_aCtlPreview.GetOutputSize().Height()  / 2));
    rXLSet.Put(XLineEndWidthItem(m_aCtlPreview.GetOutputSize().Height() / 2));
    SfxTabPage::Resize();
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
        SdrObject* pNewObj = nullptr;
        if( aInfoRec.bCanConvToPath )
            pNewObj = pPolyObj->ConvertToPolyObj( true, false );

        bCreateArrowPossible = nullptr != dynamic_cast<const SdrPathObj*>( pNewObj);
        SdrObject::Free( pNewObj );
    }

    if( !bCreateArrowPossible )
        m_xBtnAdd->set_sensitive(false);
}

void SvxLineEndDefTabPage::ActivatePage( const SfxItemSet& )
{
    if( nDlgType == 0 ) // area dialog
    {
        // ActivatePage() is called before the dialog receives PageCreated() !!!
        if( pLineEndList.is() )
        {
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
    }
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
            std::unique_ptr<weld::Builder> xBuilder(Application::CreateBuilder(GetDialogFrameWeld(), "cui/ui/querychangelineenddialog.ui"));
            std::unique_ptr<weld::MessageDialog> xQueryBox(xBuilder->weld_message_dialog("AskChangeLineEndDialog"));
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

VclPtr<SfxTabPage> SvxLineEndDefTabPage::Create(TabPageParent pParent, const SfxItemSet* rSet)
{
    return VclPtr<SvxLineEndDefTabPage>::Create(pParent, *rSet );
}

void SvxLineEndDefTabPage::SelectLineEndHdl_Impl()
{
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

        // Is not set before, in order to only take the new style,
        // if there is an entry selected in the ListBox
        *pPageType = PageType::Bitmap;
    }
}

IMPL_LINK_NOARG(SvxLineEndDefTabPage, SelectLineEndHdl_Impl, weld::ComboBox&, void)
{
    SelectLineEndHdl_Impl();
}

IMPL_LINK_NOARG(SvxLineEndDefTabPage, ClickModifyHdl_Impl, weld::Button&, void)
{
    int nPos = m_xLbLineEnds->get_active();
    if (nPos != -1)
    {
        OUString aDesc(CuiResId(RID_SVXSTR_DESC_LINEEND));
        OUString aName(m_xEdtName->get_text());
        long nCount = pLineEndList->Count();
        bool bDifferent = true;

        // check whether the name is existing already
        for ( long i = 0; i < nCount && bDifferent; i++ )
            if ( aName == pLineEndList->GetLineEnd( i )->GetName() )
                bDifferent = false;

        // if yes, repeat and demand a new name
        if ( !bDifferent )
        {
            std::unique_ptr<weld::Builder> xBuilder(Application::CreateBuilder(GetDialogFrameWeld(), "cui/ui/queryduplicatedialog.ui"));
            std::unique_ptr<weld::MessageDialog> xWarningBox(xBuilder->weld_message_dialog("DuplicateNameDialog"));
            xWarningBox->run();

            SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
            ScopedVclPtr<AbstractSvxNameDialog> pDlg(pFact->CreateSvxNameDialog(GetDialogFrameWeld(), aName, aDesc));
            bool bLoop = true;

            while( !bDifferent && bLoop && pDlg->Execute() == RET_OK )
            {
                pDlg->GetName( aName );
                bDifferent = true;

                for( long i = 0; i < nCount && bDifferent; i++ )
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
        if( bDifferent )
        {
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
    }
}

IMPL_LINK_NOARG(SvxLineEndDefTabPage, ClickAddHdl_Impl, weld::Button&, void)
{
    if( pPolyObj )
    {
        const SdrObject* pNewObj;
        SdrObject* pConvPolyObj = nullptr;

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
                pNewObj = pConvPolyObj = pPolyObj->ConvertToPolyObj( true, false );

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

        SdrObject::Free( pConvPolyObj );

        OUString aNewName(SvxResId(RID_SVXSTR_LINEEND));
        OUString aDesc(CuiResId(RID_SVXSTR_DESC_LINEEND));
        OUString aName;

        long nCount = pLineEndList->Count();
        long j = 1;
        bool bDifferent = false;

        while ( !bDifferent )
        {
            aName = aNewName + " " + OUString::number( j++ );
            bDifferent = true;

            for( long i = 0; i < nCount && bDifferent; i++ )
                if ( aName == pLineEndList->GetLineEnd( i )->GetName() )
                    bDifferent = false;
        }

        SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
        ScopedVclPtr<AbstractSvxNameDialog> pDlg(pFact->CreateSvxNameDialog(GetDialogFrameWeld(), aName, aDesc ));
        bool bLoop = true;

        while ( bLoop && pDlg->Execute() == RET_OK )
        {
            pDlg->GetName( aName );
            bDifferent = true;

            for( long i = 0; i < nCount && bDifferent; i++ )
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
                std::unique_ptr<weld::Builder> xBuilder(Application::CreateBuilder(GetDialogFrameWeld(), "cui/ui/queryduplicatedialog.ui"));
                std::unique_ptr<weld::MessageDialog> xWarningBox(xBuilder->weld_message_dialog("DuplicateNameDialog"));
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
        std::unique_ptr<weld::Builder> xBuilder(Application::CreateBuilder(GetDialogFrameWeld(), "cui/ui/querydeletelineenddialog.ui"));
        std::unique_ptr<weld::MessageDialog> xQueryBox(xBuilder->weld_message_dialog("AskDelLineEndDialog"));

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
        std::unique_ptr<weld::Builder> xBuilder(Application::CreateBuilder(GetDialogFrameWeld(), "cui/ui/querysavelistdialog.ui"));
        std::unique_ptr<weld::MessageDialog> xBox(xBuilder->weld_message_dialog("AskSaveList"));

        nReturn = xBox->run();

        if ( nReturn == RET_YES )
            pLineEndList->Save();
    }

    if ( nReturn != RET_CANCEL )
    {
        ::sfx2::FileDialogHelper aDlg(css::ui::dialogs::TemplateDescription::FILEOPEN_SIMPLE,
                                      FileDialogFlags::NONE, GetDialogFrameWeld());
        OUString aStrFilterType( "*.soe" );
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
                    aPathURL.GetMainURL(INetURLObject::DecodeMechanism::NONE), ""));
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
                std::unique_ptr<weld::Builder> xBuilder(Application::CreateBuilder(GetDialogFrameWeld(), "cui/ui/querynoloadedfiledialog.ui"));
                std::unique_ptr<weld::MessageDialog> xBox(xBuilder->weld_message_dialog("NoLoadedFileDialog"));
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
    ::sfx2::FileDialogHelper aDlg(css::ui::dialogs::TemplateDescription::FILESAVE_SIMPLE, FileDialogFlags::NONE, GetDialogFrameWeld());
    OUString aStrFilterType( "*.soe" );
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
            aFile.SetExtension( "soe" );
    }

    aDlg.SetDisplayDirectory( aFile.GetMainURL( INetURLObject::DecodeMechanism::NONE ) );
    if ( aDlg.Execute() == ERRCODE_NONE )
    {
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
            std::unique_ptr<weld::Builder> xBuilder(Application::CreateBuilder(GetDialogFrameWeld(), "cui/ui/querynosavefiledialog.ui"));
            std::unique_ptr<weld::MessageDialog> xBox(xBuilder->weld_message_dialog("NoSaveFileDialog"));
            xBox->run();
        }
    }
}

void SvxLineEndDefTabPage::DataChanged( const DataChangedEvent& rDCEvt )
{
    SfxTabPage::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DataChangedEventType::SETTINGS) && (rDCEvt.GetFlags() & AllSettingsFlags::STYLE) )
    {
        auto nOldSelect = m_xLbLineEnds->get_active();
        m_xLbLineEnds->clear();
        m_xLbLineEnds->Fill(pLineEndList);
        m_xLbLineEnds->set_active(nOldSelect);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
