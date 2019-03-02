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

#include <CollectionView.hxx>
#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>
#include <core_resource.hxx>
#include <dbu_dlg.hxx>
#include <strings.hrc>
#include <comphelper/interaction.hxx>
#include <comphelper/propertysequence.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>
#include <stringconstants.hxx>
#include <bitmaps.hlst>
#include <UITools.hxx>
#include <com/sun/star/container/XHierarchicalNameContainer.hpp>
#include <com/sun/star/ucb/InteractiveAugmentedIOException.hpp>
#include <com/sun/star/ucb/IOErrorCode.hpp>
#include <com/sun/star/task/InteractionHandler.hpp>
#include <com/sun/star/task/InteractionClassification.hpp>
#include <com/sun/star/sdbc/SQLException.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <unotools/viewoptions.hxx>
#include <osl/thread.h>
#include <connectivity/dbexception.hxx>

namespace dbaui
{

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::task;
using namespace ::com::sun::star::sdbc;
using namespace comphelper;
OCollectionView::OCollectionView( vcl::Window * pParent
                                 ,const Reference< XContent>& _xContent
                                 ,const OUString& _sDefaultName
                                 ,const css::uno::Reference< css::uno::XComponentContext >& _rxContext)
    : ModalDialog( pParent, "CollectionView", "dbaccess/ui/collectionviewdialog.ui")
    , m_xContent(_xContent)
    , m_xContext(_rxContext)
    , m_bCreateForm(true)
{
    get(m_pFTCurrentPath, "currentPathLabel");
    get(m_pNewFolder, "newFolderButton");
    get(m_pUp, "upButton");
    get(m_pView, "viewTreeview");
    get(m_pName, "fileNameEntry");
    get(m_pPB_OK, "ok");

    OSL_ENSURE(m_xContent.is(),"No valid content!");
    m_pView->Initialize(m_xContent);
    m_pFTCurrentPath->SetStyle( m_pFTCurrentPath->GetStyle() | WB_PATHELLIPSIS );
    initCurrentPath();

    m_pName->SetText(_sDefaultName);
    m_pName->GrabFocus();

    m_pUp->SetModeImage(Image(StockImage::Yes, BMP_NAVIGATION_BTN_UP_SC));
    m_pNewFolder->SetModeImage(Image(StockImage::Yes, BMP_NAVIGATION_CREATEFOLDER_SC));

    m_pView->SetDoubleClickHdl( LINK( this, OCollectionView, Dbl_Click_FileView ) );
    m_pView->EnableAutoResize();
    m_pView->EnableDelete(true);
    m_pUp->SetClickHdl( LINK( this, OCollectionView, Up_Click ) );
    m_pNewFolder->SetClickHdl( LINK( this, OCollectionView, NewFolder_Click ) );
    m_pPB_OK->SetClickHdl( LINK( this, OCollectionView, Save_Click ) );
}

OCollectionView::~OCollectionView()
{
    disposeOnce();
}

void OCollectionView::dispose()
{
    m_pFTCurrentPath.clear();
    m_pNewFolder.clear();
    m_pUp.clear();
    m_pView.clear();
    m_pName.clear();
    m_pPB_OK.clear();
    ModalDialog::dispose();
}

IMPL_LINK_NOARG(OCollectionView, Save_Click, Button*, void)
{
    OUString sName = m_pName->GetText();
    if ( sName.isEmpty() )
        return;
    try
    {
        OUString sSubFolder = m_pView->GetCurrentURL();
        sal_Int32 nIndex = sName.lastIndexOf('/') + 1;
        if ( nIndex )
        {
            if ( nIndex == 1 ) // special handling for root
            {
                Reference<XChild> xChild(m_xContent,UNO_QUERY);
                Reference<XNameAccess> xNameAccess(xChild,UNO_QUERY);
                while( xNameAccess.is() )
                {
                    xNameAccess.set(xChild->getParent(),UNO_QUERY);
                    if ( xNameAccess.is() )
                    {
                        m_xContent.set(xNameAccess,UNO_QUERY);
                        xChild.set(m_xContent,UNO_QUERY);
                    }
                }
                m_pView->Initialize(m_xContent);
                initCurrentPath();
            }
            sSubFolder = sName.copy(0,nIndex-1);
            sName = sName.copy(nIndex);
            Reference<XHierarchicalNameContainer> xHier(m_xContent,UNO_QUERY);
            OSL_ENSURE(xHier.is(),"XHierarchicalNameContainer not supported!");
            if ( !sSubFolder.isEmpty() && xHier.is() )
            {
                if ( xHier->hasByHierarchicalName(sSubFolder) )
                {
                    m_xContent.set(xHier->getByHierarchicalName(sSubFolder),UNO_QUERY);
                }
                else // sub folder doesn't exist
                {
                    Sequence<Any> aValues(comphelper::InitAnyPropertySequence(
                    {
                        {"ResourceName", Any(sSubFolder)},
                        {"ResourceType", Any(OUString("folder"))}
                    }));
                    InteractiveAugmentedIOException aException(OUString(),Reference<XInterface>(),
                                                               InteractionClassification_ERROR,
                                                               IOErrorCode_NOT_EXISTING_PATH,aValues);

                    Reference<XInteractionHandler2> xHandler(
                        InteractionHandler::createWithParent(m_xContext, VCLUnoHelper::GetInterface( this )));
                    OInteractionRequest* pRequest = new OInteractionRequest(makeAny(aException));
                    Reference< XInteractionRequest > xRequest(pRequest);

                    OInteractionApprove* pApprove = new OInteractionApprove;
                    pRequest->addContinuation(pApprove);
                    xHandler->handle(xRequest);

                    return;
                }
            }
        }
        Reference<XNameContainer> xNameContainer(m_xContent,UNO_QUERY);
        if ( xNameContainer.is() )
        {
            if ( xNameContainer->hasByName(sName) )
            {
                std::unique_ptr<weld::MessageDialog> xQueryBox(Application::CreateMessageDialog(GetFrameWeld(),
                                                               VclMessageType::Question, VclButtonsType::YesNo,
                                                               DBA_RES(STR_ALREADYEXISTOVERWRITE)));
                if (xQueryBox->run() != RET_YES)
                    return;
            }
            m_pName->SetText(sName);
            EndDialog( RET_OK );
        }
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("dbaccess");
    }
}

IMPL_LINK_NOARG(OCollectionView, NewFolder_Click, Button*, void)
{
    try
    {
        Reference<XHierarchicalNameContainer> xNameContainer(m_xContent,UNO_QUERY);
        if ( dbaui::insertHierachyElement(GetFrameWeld(),m_xContext,xNameContainer,OUString(),m_bCreateForm) )
            m_pView->Initialize(m_xContent);
    }
    catch( const SQLException& )
    {
        showError( ::dbtools::SQLExceptionInfo( ::cppu::getCaughtException() ), VCLUnoHelper::GetInterface(this), m_xContext );
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("dbaccess");
    }
}

IMPL_LINK_NOARG(OCollectionView, Up_Click, Button*, void)
{
    try
    {
        Reference<XChild> xChild(m_xContent,UNO_QUERY);
        if ( xChild.is() )
        {
            Reference<XNameAccess> xNameAccess(xChild->getParent(),UNO_QUERY);
            if ( xNameAccess.is() )
            {
                m_xContent.set(xNameAccess,UNO_QUERY);
                m_pView->Initialize(m_xContent);
                initCurrentPath();
            }
            else
                m_pUp->Disable();
        }
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("dbaccess");
    }
}

IMPL_LINK_NOARG(OCollectionView, Dbl_Click_FileView, SvTreeListBox*, bool)
{
    try
    {
        Reference<XNameAccess> xNameAccess(m_xContent,UNO_QUERY);
        if ( xNameAccess.is() )
        {
            OUString sSubFolder = m_pView->GetCurrentURL();
            sSubFolder = sSubFolder.copy(sSubFolder.lastIndexOf('/') + 1);
            if ( !sSubFolder.isEmpty() )
            {
                Reference< XContent> xContent;
                if ( xNameAccess->hasByName(sSubFolder) )
                    xContent.set(xNameAccess->getByName(sSubFolder),UNO_QUERY);
                if ( xContent.is() )
                {
                    m_xContent = xContent;
                    m_pView->Initialize(m_xContent);
                    initCurrentPath();
                }
            }
        }
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("dbaccess");
    }
    return false;
}

void OCollectionView::initCurrentPath()
{
    bool bEnable = false;
    try
    {
        if ( m_xContent.is() )
        {
            const OUString sCID = m_xContent->getIdentifier()->getContentIdentifier();
            static const char s_sFormsCID[] = "private:forms";
            static const char s_sReportsCID[] = "private:reports";
            m_bCreateForm = s_sFormsCID == sCID;
            OUString sPath("/");
            if ( m_bCreateForm && sCID.getLength() != static_cast<sal_Int32>(strlen(s_sFormsCID)))
                sPath = sCID.copy(strlen(s_sFormsCID));
            else if ( !m_bCreateForm && sCID.getLength() != static_cast<sal_Int32>(strlen(s_sReportsCID)) )
                sPath = sCID.copy(strlen(s_sReportsCID) - 2);

            m_pFTCurrentPath->SetText(sPath);
            Reference<XChild> xChild(m_xContent,UNO_QUERY);
            bEnable = xChild.is() && Reference<XNameAccess>(xChild->getParent(),UNO_QUERY).is();
        }
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("dbaccess");
    }
    m_pUp->Enable(bEnable);
}

OUString OCollectionView::getName() const
{
    return m_pName->GetText();
}

}   // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
