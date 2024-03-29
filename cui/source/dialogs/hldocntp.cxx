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

#include <hldocntp.hxx>
#include <osl/file.hxx>
#include <sfx2/filedlghelper.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/docfilt.hxx>
#include <svl/stritem.hxx>
#include <com/sun/star/awt/XTopWindow.hpp>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/Exception.hpp>
#include <utility>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>
#include <tools/urlobj.hxx>
#include <unotools/pathoptions.hxx>
#include <unotools/dynamicmenuoptions.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <unotools/ucbhelper.hxx>

#include <comphelper/processfactory.hxx>
#include <com/sun/star/ui/dialogs/XFolderPicker2.hpp>
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>

#include <cuihyperdlg.hxx>
#include <dialmgr.hxx>
#include <strings.hrc>

using namespace ::com::sun::star::ui::dialogs;
using namespace ::com::sun::star::uno;

using namespace ::com::sun::star;

/*************************************************************************
|*
|* Data-struct for documenttypes in listbox
|*
|************************************************************************/

namespace {

struct DocumentTypeData
{
    OUString aStrURL;
    OUString aStrExt;
    DocumentTypeData (OUString aURL, OUString aExt) : aStrURL(std::move(aURL)), aStrExt(std::move(aExt))
    {}
};

}

bool SvxHyperlinkNewDocTp::ImplGetURLObject( const OUString& rPath, std::u16string_view rBase, INetURLObject& aURLObject ) const
{
    bool bIsValidURL = !rPath.isEmpty();
    if ( bIsValidURL )
    {
        aURLObject.SetURL( rPath );
        if ( aURLObject.GetProtocol() == INetProtocol::NotValid )      // test if the source is already a valid url
        {                                                           // if not we have to create a url from a physical file name
            bool wasAbs;
            INetURLObject base(rBase);
            base.setFinalSlash();
            aURLObject = base.smartRel2Abs(
                rPath, wasAbs, true, INetURLObject::EncodeMechanism::All,
                RTL_TEXTENCODING_UTF8, true);
        }
        bIsValidURL = aURLObject.GetProtocol() != INetProtocol::NotValid;
        if ( bIsValidURL )
        {
            OUString aBase( aURLObject.getName( INetURLObject::LAST_SEGMENT, false ) );
            if ( aBase.isEmpty() || ( aBase[0] == '.' ) )
                bIsValidURL = false;
        }
        if ( bIsValidURL )
        {
            sal_Int32 nPos = m_xLbDocTypes->get_selected_index();
            if (nPos != -1)
                aURLObject.SetExtension(weld::fromId<DocumentTypeData*>(m_xLbDocTypes->get_id(nPos))->aStrExt);
        }

    }
    return bIsValidURL;
}

/*************************************************************************
|*
|* Constructor / Destructor
|*
|************************************************************************/

SvxHyperlinkNewDocTp::SvxHyperlinkNewDocTp(weld::Container* pParent, SvxHpLinkDlg* pDlg, const SfxItemSet* pItemSet)
    : SvxHyperlinkTabPageBase(pParent, pDlg, "cui/ui/hyperlinknewdocpage.ui", "HyperlinkNewDocPage", pItemSet)
    , m_xRbtEditNow(xBuilder->weld_radio_button("editnow"))
    , m_xRbtEditLater(xBuilder->weld_radio_button("editlater"))
    , m_xCbbPath(new SvxHyperURLBox(xBuilder->weld_combo_box("path")))
    , m_xBtCreate(xBuilder->weld_button("create"))
    , m_xLbDocTypes(xBuilder->weld_tree_view("types"))
{
    m_xCbbPath->SetSmartProtocol(INetProtocol::File);
    m_xLbDocTypes->set_size_request(-1, m_xLbDocTypes->get_height_rows(5));

    InitStdControls();

    SetExchangeSupport ();

    m_xCbbPath->show();
    m_xCbbPath->SetBaseURL(SvtPathOptions().GetWorkPath());

    // set defaults
    m_xRbtEditNow->set_active(true);

    m_xBtCreate->connect_clicked(LINK(this, SvxHyperlinkNewDocTp, ClickNewHdl_Impl));

    FillDocumentList ();
}

SvxHyperlinkNewDocTp::~SvxHyperlinkNewDocTp ()
{
    if (m_xLbDocTypes)
    {
        for (sal_Int32 n = 0, nEntryCount = m_xLbDocTypes->n_children(); n < nEntryCount; ++n)
            delete weld::fromId<DocumentTypeData*>(m_xLbDocTypes->get_id(n));
        m_xLbDocTypes = nullptr;
    }
}

/*************************************************************************
|*
|* Fill the all dialog-controls except controls in groupbox "more..."
|*
|************************************************************************/


void SvxHyperlinkNewDocTp::FillDlgFields(const OUString& /*rStrURL*/)
{
}

void SvxHyperlinkNewDocTp::FillDocumentList()
{
    weld::WaitObject aWaitObj(mpDialog->getDialog());

    std::vector<SvtDynMenuEntry> aDynamicMenuEntries( SvtDynamicMenuOptions::GetMenu( EDynamicMenuType::NewMenu ) );

    for ( const SvtDynMenuEntry & rDynamicMenuEntry : aDynamicMenuEntries )
    {
        OUString aDocumentUrl = rDynamicMenuEntry.sURL;
        OUString aTitle = rDynamicMenuEntry.sTitle;

        //#i96822# business cards, labels and database should not be inserted here
        if( aDocumentUrl == "private:factory/swriter?slot=21051" ||
            aDocumentUrl == "private:factory/swriter?slot=21052" ||
            aDocumentUrl == "private:factory/sdatabase?Interactive" )
            continue;

        // Insert into listbox
        if ( !aDocumentUrl.isEmpty() )
        {
            if ( aDocumentUrl == "private:factory/simpress?slot=6686" )              // SJ: #106216# do not start
                aDocumentUrl = "private:factory/simpress"; // the AutoPilot for impress

            // insert private-url and default-extension as user-data
            std::shared_ptr<const SfxFilter> pFilter = SfxFilter::GetDefaultFilterFromFactory( aDocumentUrl );
            if ( pFilter )
            {
                // insert doc-name and image
                OUString aTitleName = aTitle.replaceFirst( "~", "" );

                OUString aStrDefExt(pFilter->GetDefaultExtension());
                DocumentTypeData *pTypeData = new DocumentTypeData(aDocumentUrl, aStrDefExt.copy(2));
                OUString sId(weld::toId(pTypeData));
                m_xLbDocTypes->append(sId, aTitleName);
            }
        }
    }
    m_xLbDocTypes->select(0);
}

/*************************************************************************
|*
|* retrieve and prepare data from dialog-fields
|*
|************************************************************************/

void SvxHyperlinkNewDocTp::GetCurrentItemData ( OUString& rStrURL, OUString& aStrName,
                                               OUString& aStrIntName, OUString& aStrFrame,
                                               SvxLinkInsertMode& eMode )
{
    // get data from dialog-controls
    rStrURL = m_xCbbPath->get_active_text();
    INetURLObject aURL;
    if ( ImplGetURLObject( rStrURL, m_xCbbPath->GetBaseURL(), aURL ) )
    {
        rStrURL = aURL.GetMainURL( INetURLObject::DecodeMechanism::NONE );
    }

    GetDataFromCommonFields( aStrName, aStrIntName, aStrFrame, eMode );
}

/*************************************************************************
|*
|* static method to create Tabpage
|*
|************************************************************************/

std::unique_ptr<IconChoicePage> SvxHyperlinkNewDocTp::Create(weld::Container* pWindow, SvxHpLinkDlg* pDlg, const SfxItemSet* pItemSet)
{
    return std::make_unique<SvxHyperlinkNewDocTp>(pWindow, pDlg, pItemSet);
}

/*************************************************************************
|*
|* Set initial focus
|*
|************************************************************************/
void SvxHyperlinkNewDocTp::SetInitFocus()
{
    m_xCbbPath->grab_focus();
}

namespace
{
    struct ExecuteInfo
    {
        bool bRbtEditLater;
        bool bRbtEditNow;
        INetURLObject aURL;
        OUString aStrDocName;
        // current document
        css::uno::Reference<css::frame::XFrame> xFrame;
        SfxDispatcher* pDispatcher;
    };
}

IMPL_STATIC_LINK(SvxHyperlinkNewDocTp, DispatchDocument, void*, p, void)
{
    std::unique_ptr<ExecuteInfo> xExecuteInfo(static_cast<ExecuteInfo*>(p));
    if (!xExecuteInfo->xFrame.is())
        return;
    try
    {
        //if it throws dispatcher is invalid
        css::uno::Reference<css::awt::XTopWindow>(xExecuteInfo->xFrame->getContainerWindow(), css::uno::UNO_QUERY_THROW);

        SfxViewFrame *pViewFrame = nullptr;

        // create items
        SfxStringItem aName( SID_FILE_NAME, xExecuteInfo->aStrDocName );
        SfxStringItem aReferer( SID_REFERER, "private:user" );
        SfxStringItem aFrame( SID_TARGETNAME, "_blank");

        OUString aStrFlags('S');
        if (xExecuteInfo->bRbtEditLater)
        {
            aStrFlags += "H";
        }
        SfxStringItem aFlags (SID_OPTIONS, aStrFlags);

        // open url
        const SfxPoolItemHolder aResult(xExecuteInfo->pDispatcher->ExecuteList(
            SID_OPENDOC, SfxCallMode::SYNCHRON,
            { &aName, &aFlags, &aFrame, &aReferer }));

        // save new doc
        const SfxViewFrameItem *pItem = dynamic_cast<const SfxViewFrameItem*>(aResult.getItem());  // aResult is NULL if the Hyperlink
        if ( pItem )                                                            // creation is cancelled #106216#
        {
            pViewFrame = pItem->GetFrame();
            if (pViewFrame)
            {
                SfxStringItem aNewName( SID_FILE_NAME, xExecuteInfo->aURL.GetMainURL( INetURLObject::DecodeMechanism::NONE ) );
                SfxUnoFrameItem aDocFrame( SID_FILLFRAME, pViewFrame->GetFrame().GetFrameInterface() );
                pViewFrame->GetDispatcher()->ExecuteList(
                    SID_SAVEASDOC, SfxCallMode::SYNCHRON,
                    { &aNewName }, { &aDocFrame });
            }
        }

        if (xExecuteInfo->bRbtEditNow)
        {
            css::uno::Reference<css::awt::XTopWindow> xWindow(xExecuteInfo->xFrame->getContainerWindow(), css::uno::UNO_QUERY);
            if (xWindow.is()) //will be false if the frame was exited while the document was loading (e.g. we waited for warning dialogs)
                xWindow->toFront();
        }

        if (pViewFrame && xExecuteInfo->bRbtEditLater)
        {
            SfxObjectShell* pObjShell = pViewFrame->GetObjectShell();
            pObjShell->DoClose();
        }
    }
    catch (...)
    {
    }
}

/*************************************************************************
|*
|* Any action to do after apply-button is pressed
|*
\************************************************************************/
void SvxHyperlinkNewDocTp::DoApply()
{
    weld::WaitObject aWait(mpDialog->getDialog());

    // get data from dialog-controls
    OUString aStrNewName = m_xCbbPath->get_active_text();

    if ( aStrNewName.isEmpty() )
        aStrNewName = maStrInitURL;

    // create a real URL-String
    INetURLObject aURL;
    if ( !ImplGetURLObject( aStrNewName, m_xCbbPath->GetBaseURL(), aURL ) )
        return;

    // create Document
    aStrNewName = aURL.GetURLPath( INetURLObject::DecodeMechanism::NONE );
    bool bCreate = true;
    try
    {
        // check if file exists, warn before we overwrite it
        std::unique_ptr<SvStream> pIStm = ::utl::UcbStreamHelper::CreateStream( aURL.GetMainURL( INetURLObject::DecodeMechanism::NONE ), StreamMode::READ );

        bool bOk = pIStm && ( pIStm->GetError() == ERRCODE_NONE);

        pIStm.reset();

        if( bOk )
        {
            std::unique_ptr<weld::MessageDialog> xWarn(Application::CreateMessageDialog(mpDialog->getDialog(),
                                                       VclMessageType::Warning, VclButtonsType::YesNo,
                                                       CuiResId(RID_CUISTR_HYPERDLG_QUERYOVERWRITE)));
            bCreate = xWarn->run() == RET_YES;
        }
    }
    catch (const uno::Exception&)
    {
    }

    if (!bCreate || aStrNewName.isEmpty())
        return;

    ExecuteInfo* pExecuteInfo = new ExecuteInfo;

    pExecuteInfo->bRbtEditLater = m_xRbtEditLater->get_active();
    pExecuteInfo->bRbtEditNow = m_xRbtEditNow->get_active();
    // get private-url
    sal_Int32 nPos = m_xLbDocTypes->get_selected_index();
    if (nPos == -1)
        nPos = 0;
    pExecuteInfo->aURL = aURL;
    pExecuteInfo->aStrDocName = weld::fromId<DocumentTypeData*>(m_xLbDocTypes->get_id(nPos))->aStrURL;

    // current document
    pExecuteInfo->xFrame = GetDispatcher()->GetFrame()->GetFrame().GetFrameInterface();
    pExecuteInfo->pDispatcher = GetDispatcher();

    Application::PostUserEvent(LINK(nullptr, SvxHyperlinkNewDocTp, DispatchDocument), pExecuteInfo);
}

/*************************************************************************
|*
|* Click on imagebutton : new
|*
|************************************************************************/
IMPL_LINK_NOARG(SvxHyperlinkNewDocTp, ClickNewHdl_Impl, weld::Button&, void)
{
    DisableClose( true );
    uno::Reference < XComponentContext > xContext( ::comphelper::getProcessComponentContext() );
    uno::Reference < XFolderPicker2 >  xFolderPicker = sfx2::createFolderPicker(xContext, mpDialog->getDialog());

    OUString            aStrURL;
    OUString            aTempStrURL( m_xCbbPath->get_active_text() );
    osl::FileBase::getFileURLFromSystemPath( aTempStrURL, aStrURL );

    OUString            aStrPath = aStrURL;
    bool            bZeroPath = aStrPath.isEmpty();
    bool            bHandleFileName = bZeroPath;    // when path has length of 0, then the rest should always be handled
                                                        //  as file name, otherwise we do not yet know

    if( bZeroPath )
        aStrPath = SvtPathOptions().GetWorkPath();
    else if( !::utl::UCBContentHelper::IsFolder( aStrURL ) )
        bHandleFileName = true;

    xFolderPicker->setDisplayDirectory( aStrPath );
    sal_Int16 nResult = xFolderPicker->execute();
    DisableClose( false );
    if( ExecutableDialogResults::OK != nResult )
        return;

    char const  sSlash[] = "/";

    INetURLObject   aURL( aStrURL, INetProtocol::File );
    OUString        aStrName;
    if( bHandleFileName )
        aStrName = bZeroPath? aTempStrURL : aURL.getName();

    m_xCbbPath->SetBaseURL( xFolderPicker->getDirectory() );
    OUString          aStrTmp( xFolderPicker->getDirectory() );

    if( aStrTmp[ aStrTmp.getLength() - 1 ] != sSlash[0] )
        aStrTmp += sSlash;

    // append old file name
    if( bHandleFileName )
        aStrTmp += aStrName;

    INetURLObject   aNewURL( aStrTmp );

    if (!aStrName.isEmpty() && !aNewURL.getExtension().isEmpty() &&
       m_xLbDocTypes->get_selected_index() != -1)
    {
        // get private-url
        const sal_Int32 nPos = m_xLbDocTypes->get_selected_index();
        aNewURL.setExtension(weld::fromId<DocumentTypeData*>(m_xLbDocTypes->get_id(nPos))->aStrExt);
    }

    if( aNewURL.GetProtocol() == INetProtocol::File )
    {
        osl::FileBase::getSystemPathFromFileURL(aNewURL.GetMainURL( INetURLObject::DecodeMechanism::NONE ), aStrTmp);
    }
    else
    {
        aStrTmp = aNewURL.GetMainURL( INetURLObject::DecodeMechanism::Unambiguous );
    }

    m_xCbbPath->set_entry_text( aStrTmp );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
