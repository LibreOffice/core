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
#include <sfx2/viewfrm.hxx>
#include <sfx2/docfac.hxx>
#include <com/sun/star/awt/XTopWindow.hpp>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/uno/Exception.hpp>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>
#include <tools/urlobj.hxx>
#include <unotools/pathoptions.hxx>
#include <unotools/dynamicmenuoptions.hxx>
#include <sfx2/filedlghelper.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <unotools/ucbhelper.hxx>

#include <comphelper/processfactory.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/ui/dialogs/FolderPicker.hpp>
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>

#include <bitmaps.hlst>
#include <dialmgr.hxx>
#include <strings.hrc>

using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::ui::dialogs;
using namespace ::com::sun::star::uno;

using namespace ::com::sun::star;

/*************************************************************************
|*
|* Data-struct for documenttypes in listbox
|*
|************************************************************************/

struct DocumentTypeData
{
    OUString aStrURL;
    OUString aStrExt;
    DocumentTypeData (const OUString& aURL, const OUString& aExt) : aStrURL(aURL), aStrExt(aExt)
    {}
};

bool SvxHyperlinkNewDocTp::ImplGetURLObject( const OUString& rPath, const OUString& rBase, INetURLObject& aURLObject ) const
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
            sal_Int32 nPos = m_pLbDocTypes->GetSelectedEntryPos();
            if ( nPos != LISTBOX_ENTRY_NOTFOUND )
                aURLObject.SetExtension( static_cast<DocumentTypeData*>(m_pLbDocTypes->GetEntryData( nPos ))->aStrExt );
        }

    }
    return bIsValidURL;
}

/*************************************************************************
|*
|* Constructor / Destructor
|*
|************************************************************************/

SvxHyperlinkNewDocTp::SvxHyperlinkNewDocTp ( vcl::Window *pParent, IconChoiceDialog* pDlg, const SfxItemSet* pItemSet)
:   SvxHyperlinkTabPageBase ( pParent, pDlg, "HyperlinkNewDocPage", "cui/ui/hyperlinknewdocpage.ui", pItemSet )
{
    get(m_pRbtEditNow, "editnow");
    get(m_pRbtEditLater, "editlater");
    get(m_pCbbPath, "path");
    m_pCbbPath->SetSmartProtocol(INetProtocol::File);
    get(m_pBtCreate, "create");
    m_pBtCreate->SetModeImage(Image(StockImage::Yes, RID_SVXBMP_NEWDOC));
    get(m_pLbDocTypes, "types");
    m_pLbDocTypes->set_height_request(m_pLbDocTypes->GetTextHeight() * 5);

    // Set HC bitmaps and disable display of bitmap names.
    m_pBtCreate->EnableTextDisplay (false);

    InitStdControls();

    SetExchangeSupport ();

    m_pCbbPath->Show();
    m_pCbbPath->SetBaseURL(SvtPathOptions().GetWorkPath());

    // set defaults
    m_pRbtEditNow->Check();

    m_pBtCreate->SetClickHdl        ( LINK ( this, SvxHyperlinkNewDocTp, ClickNewHdl_Impl ) );

    FillDocumentList ();
}

SvxHyperlinkNewDocTp::~SvxHyperlinkNewDocTp ()
{
    disposeOnce();
}

void SvxHyperlinkNewDocTp::dispose()
{
    if (m_pLbDocTypes)
    {
        for ( sal_Int32 n=0; n<m_pLbDocTypes->GetEntryCount(); n++ )
            delete static_cast<DocumentTypeData*>(m_pLbDocTypes->GetEntryData ( n ));
        m_pLbDocTypes = nullptr;
    }
    m_pRbtEditNow.clear();
    m_pRbtEditLater.clear();
    m_pCbbPath.clear();
    m_pBtCreate.clear();
    m_pLbDocTypes.clear();
    SvxHyperlinkTabPageBase::dispose();
}

/*************************************************************************
|*
|* Fill the all dialog-controls except controls in groupbox "more..."
|*
|************************************************************************/


void SvxHyperlinkNewDocTp::FillDlgFields(const OUString& /*rStrURL*/)
{
}

void SvxHyperlinkNewDocTp::FillDocumentList ()
{
    EnterWait();

    uno::Sequence< uno::Sequence< beans::PropertyValue > >
        aDynamicMenuEntries( SvtDynamicMenuOptions().GetMenu( EDynamicMenuType::NewMenu ) );

    sal_uInt32 i, nCount = aDynamicMenuEntries.getLength();
    for ( i = 0; i < nCount; i++ )
    {
        uno::Sequence< beans::PropertyValue >& rDynamicMenuEntry = aDynamicMenuEntries[ i ];

        OUString aDocumentUrl, aTitle;

        for ( int e = 0; e < rDynamicMenuEntry.getLength(); e++ )
        {
            if ( rDynamicMenuEntry[ e ].Name == DYNAMICMENU_PROPERTYNAME_URL )
                rDynamicMenuEntry[ e ].Value >>= aDocumentUrl;
            else if ( rDynamicMenuEntry[e].Name == DYNAMICMENU_PROPERTYNAME_TITLE )
                rDynamicMenuEntry[e].Value >>= aTitle;
        }
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
                OUString aTitleName( aTitle );
                aTitleName = aTitleName.replaceFirst( "~", "" );

                sal_Int16 nPos = m_pLbDocTypes->InsertEntry ( aTitleName );
                OUString aStrDefExt( pFilter->GetDefaultExtension () );
                DocumentTypeData *pTypeData = new DocumentTypeData ( aDocumentUrl, aStrDefExt.copy( 2 ) );
                m_pLbDocTypes->SetEntryData ( nPos, pTypeData );
            }
        }
    }
    m_pLbDocTypes->SelectEntryPos ( 0 );

    LeaveWait();
}

/*************************************************************************
|*
|* retrieve and prepare data from dialog-fields
|*
|************************************************************************/

void SvxHyperlinkNewDocTp::GetCurentItemData ( OUString& rStrURL, OUString& aStrName,
                                               OUString& aStrIntName, OUString& aStrFrame,
                                               SvxLinkInsertMode& eMode )
{
    // get data from dialog-controls
    rStrURL = m_pCbbPath->GetText();
    INetURLObject aURL;
    if ( ImplGetURLObject( rStrURL, m_pCbbPath->GetBaseURL(), aURL ) )
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

VclPtr<IconChoicePage> SvxHyperlinkNewDocTp::Create( vcl::Window* pWindow, IconChoiceDialog* pDlg, const SfxItemSet* pItemSet )
{
    return VclPtr<SvxHyperlinkNewDocTp>::Create( pWindow, pDlg, pItemSet );
}

/*************************************************************************
|*
|* Set initial focus
|*
|************************************************************************/

void SvxHyperlinkNewDocTp::SetInitFocus()
{
    m_pCbbPath->GrabFocus();
}

/*************************************************************************
|*
|* Ask page whether an insert is possible
|*
\************************************************************************/

bool SvxHyperlinkNewDocTp::AskApply()
{
    INetURLObject aINetURLObject;
    bool bRet = ImplGetURLObject( m_pCbbPath->GetText(), m_pCbbPath->GetBaseURL(), aINetURLObject );
    if ( !bRet )
    {
        std::unique_ptr<weld::MessageDialog> xWarn(Application::CreateMessageDialog(GetFrameWeld(),
                                                   VclMessageType::Warning, VclButtonsType::Ok,
                                                   CuiResId(RID_SVXSTR_HYPDLG_NOVALIDFILENAME)));
        xWarn->run();
    }
    return bRet;
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
        SfxStringItem aReferer( SID_REFERER, OUString("private:user") );
        SfxStringItem aFrame( SID_TARGETNAME, OUString("_blank") );

        OUString aStrFlags('S');
        if (xExecuteInfo->bRbtEditLater)
        {
            aStrFlags += "H";
        }
        SfxStringItem aFlags (SID_OPTIONS, aStrFlags);

        // open url
        const SfxPoolItem* pReturn = xExecuteInfo->pDispatcher->ExecuteList(
                SID_OPENDOC, SfxCallMode::SYNCHRON,
                { &aName, &aFlags, &aFrame, &aReferer });

        // save new doc
        const SfxViewFrameItem *pItem = dynamic_cast<const SfxViewFrameItem*>( pReturn  );  // SJ: pReturn is NULL if the Hyperlink
        if ( pItem )                                                            // creation is cancelled #106216#
        {
            pViewFrame = pItem->GetFrame();
            if (pViewFrame)
            {
                SfxStringItem aNewName( SID_FILE_NAME, xExecuteInfo->aURL.GetMainURL( INetURLObject::DecodeMechanism::NONE ) );
                SfxUnoFrameItem aDocFrame( SID_FILLFRAME, pViewFrame->GetFrame().GetFrameInterface() );
                fprintf(stderr, "is there a frame int %p\n", pViewFrame->GetFrame().GetFrameInterface().get() );
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

void SvxHyperlinkNewDocTp::DoApply ()
{
    EnterWait();

    // get data from dialog-controls
    OUString aStrNewName = m_pCbbPath->GetText();

    if ( aStrNewName.isEmpty() )
        aStrNewName = maStrInitURL;

    // create a real URL-String
    INetURLObject aURL;
    if ( ImplGetURLObject( aStrNewName, m_pCbbPath->GetBaseURL(), aURL ) )
    {
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
                std::unique_ptr<weld::MessageDialog> xWarn(Application::CreateMessageDialog(GetFrameWeld(),
                                                           VclMessageType::Warning, VclButtonsType::YesNo,
                                                           CuiResId(RID_SVXSTR_HYPERDLG_QUERYOVERWRITE)));
                bCreate = xWarn->run() == RET_YES;
            }
        }
        catch (const uno::Exception&)
        {
        }

        if (bCreate && !aStrNewName.isEmpty())
        {
            ExecuteInfo* pExecuteInfo = new ExecuteInfo;

            pExecuteInfo->bRbtEditLater = m_pRbtEditLater->IsChecked();
            pExecuteInfo->bRbtEditNow = m_pRbtEditNow->IsChecked();
            // get private-url
            sal_Int32 nPos = m_pLbDocTypes->GetSelectedEntryPos();
            if( nPos == LISTBOX_ENTRY_NOTFOUND )
                nPos=0;
            pExecuteInfo->aURL = aURL;
            pExecuteInfo->aStrDocName = static_cast<DocumentTypeData*>(
                                 m_pLbDocTypes->GetEntryData( nPos ))->aStrURL;

            // current document
            pExecuteInfo->xFrame = GetDispatcher()->GetFrame()->GetFrame().GetFrameInterface();
            pExecuteInfo->pDispatcher = GetDispatcher();

            Application::PostUserEvent(LINK(nullptr, SvxHyperlinkNewDocTp, DispatchDocument), pExecuteInfo);
        }
    }

    LeaveWait();
}

/*************************************************************************
|*
|* Click on imagebutton : new
|*
|************************************************************************/

IMPL_LINK_NOARG(SvxHyperlinkNewDocTp, ClickNewHdl_Impl, Button*, void)
{
    uno::Reference < XComponentContext > xContext( ::comphelper::getProcessComponentContext() );
    uno::Reference < XFolderPicker2 >  xFolderPicker = FolderPicker::create(xContext);

    OUString            aStrURL;
    OUString            aTempStrURL( m_pCbbPath->GetText() );
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
    DisableClose( true );
    sal_Int16 nResult = xFolderPicker->execute();
    DisableClose( false );
    if( ExecutableDialogResults::OK == nResult )
    {
        sal_Char const  sSlash[] = "/";

        INetURLObject   aURL( aStrURL, INetProtocol::File );
        OUString        aStrName;
        if( bHandleFileName )
            aStrName = bZeroPath? aTempStrURL : aURL.getName();

        m_pCbbPath->SetBaseURL( xFolderPicker->getDirectory() );
        OUString          aStrTmp( xFolderPicker->getDirectory() );

        if( aStrTmp[ aStrTmp.getLength() - 1 ] != sSlash[0] )
            aStrTmp += sSlash;

        // append old file name
        if( bHandleFileName )
            aStrTmp += aStrName;

        INetURLObject   aNewURL( aStrTmp );

        if( !aStrName.isEmpty() && !aNewURL.getExtension().isEmpty() &&
            m_pLbDocTypes->GetSelectedEntryPos() != LISTBOX_ENTRY_NOTFOUND )
        {
            // get private-url
            const sal_Int32 nPos = m_pLbDocTypes->GetSelectedEntryPos();
            aNewURL.setExtension( static_cast<DocumentTypeData*>(m_pLbDocTypes->GetEntryData( nPos ))->aStrExt );
        }

        if( aNewURL.GetProtocol() == INetProtocol::File )
        {
            osl::FileBase::getSystemPathFromFileURL(aNewURL.GetMainURL( INetURLObject::DecodeMechanism::NONE ), aStrTmp);
        }
        else
        {
            aStrTmp = aNewURL.GetMainURL( INetURLObject::DecodeMechanism::Unambiguous );
        }

        m_pCbbPath->SetText ( aStrTmp );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
