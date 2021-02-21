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
#include <tools/diagnose_ex.h>
#include <core_resource.hxx>
#include <strings.hrc>
#include <comphelper/interaction.hxx>
#include <comphelper/propertysequence.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>
#include <UITools.hxx>
#include <com/sun/star/container/XHierarchicalNameContainer.hpp>
#include <com/sun/star/ucb/InteractiveAugmentedIOException.hpp>
#include <com/sun/star/ucb/IOErrorCode.hpp>
#include <com/sun/star/ucb/XDynamicResultSet.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/task/InteractionHandler.hpp>
#include <com/sun/star/task/InteractionClassification.hpp>
#include <com/sun/star/sdbc/SQLException.hpp>
#include <comphelper/processfactory.hxx>
#include <ucbhelper/commandenvironment.hxx>
#include <ucbhelper/content.hxx>
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

OCollectionView::OCollectionView(weld::Window* pParent,
                                 const Reference< XContent>& _xContent,
                                 const OUString& _sDefaultName,
                                 const css::uno::Reference< css::uno::XComponentContext >& _rxContext)
    : GenericDialogController(pParent, "dbaccess/ui/collectionviewdialog.ui", "CollectionView")
    , m_xContent(_xContent)
    , m_xContext(_rxContext)
    , m_bCreateForm(true)
    , m_xFTCurrentPath(m_xBuilder->weld_label("currentPathLabel"))
    , m_xNewFolder(m_xBuilder->weld_button("newFolderButton"))
    , m_xUp(m_xBuilder->weld_button("upButton"))
    , m_xView(m_xBuilder->weld_tree_view("viewTreeview"))
    , m_xName(m_xBuilder->weld_entry("fileNameEntry"))
    , m_xPB_OK(m_xBuilder->weld_button("ok"))
{
    Reference<XInteractionHandler2> xHandler(
        InteractionHandler::createWithParent(m_xContext, m_xDialog->GetXWindow()));
    m_xCmdEnv = new ::ucbhelper::CommandEnvironment(xHandler, nullptr);

    OSL_ENSURE(m_xContent.is(),"No valid content!");
    m_xView->set_size_request(m_xView->get_approximate_digit_width() * 60, m_xView->get_height_rows(8));
    m_xView->make_sorted();
    Initialize();
    initCurrentPath();

    m_xName->set_text(_sDefaultName);
    m_xName->grab_focus();

    m_xView->connect_row_activated( LINK( this, OCollectionView, Dbl_Click_FileView ) );
    m_xUp->connect_clicked( LINK( this, OCollectionView, Up_Click ) );
    m_xNewFolder->connect_clicked( LINK( this, OCollectionView, NewFolder_Click ) );
    m_xPB_OK->connect_clicked( LINK( this, OCollectionView, Save_Click ) );
}

OCollectionView::~OCollectionView()
{
}

IMPL_LINK_NOARG(OCollectionView, Save_Click, weld::Button&, void)
{
    OUString sName = m_xName->get_text();
    if (sName.isEmpty())
        return;
    try
    {
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
                Initialize();
                initCurrentPath();
            }
            OUString sSubFolder = sName.copy(0,nIndex-1);
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
                        InteractionHandler::createWithParent(m_xContext, m_xDialog->GetXWindow()));
                    rtl::Reference<OInteractionRequest> pRequest = new OInteractionRequest(makeAny(aException));

                    rtl::Reference<OInteractionApprove> pApprove = new OInteractionApprove;
                    pRequest->addContinuation(pApprove);
                    xHandler->handle(pRequest);

                    return;
                }
            }
        }
        Reference<XNameContainer> xNameContainer(m_xContent,UNO_QUERY);
        if ( xNameContainer.is() )
        {
            if ( xNameContainer->hasByName(sName) )
            {
                std::unique_ptr<weld::MessageDialog> xQueryBox(Application::CreateMessageDialog(m_xDialog.get(),
                                                               VclMessageType::Question, VclButtonsType::YesNo,
                                                               DBA_RES(STR_ALREADYEXISTOVERWRITE)));
                if (xQueryBox->run() != RET_YES)
                    return;
            }
            m_xName->set_text(sName);
            m_xDialog->response(RET_OK);
        }
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("dbaccess");
    }
}

IMPL_LINK_NOARG(OCollectionView, NewFolder_Click, weld::Button&, void)
{
    try
    {
        Reference<XHierarchicalNameContainer> xNameContainer(m_xContent,UNO_QUERY);
        if ( dbaui::insertHierachyElement(m_xDialog.get(),m_xContext,xNameContainer,OUString(),m_bCreateForm) )
            Initialize();
    }
    catch( const SQLException& )
    {
        showError(::dbtools::SQLExceptionInfo(::cppu::getCaughtException()), m_xDialog->GetXWindow(), m_xContext);
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("dbaccess");
    }
}

IMPL_LINK_NOARG(OCollectionView, Up_Click, weld::Button&, void)
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
                Initialize();
                initCurrentPath();
            }
            else
                m_xUp->set_sensitive(false);
        }
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("dbaccess");
    }
}

IMPL_LINK_NOARG(OCollectionView, Dbl_Click_FileView, weld::TreeView&, bool)
{
    try
    {
        Reference<XNameAccess> xNameAccess(m_xContent,UNO_QUERY);
        if ( xNameAccess.is() )
        {
            OUString sSubFolder = m_xView->get_selected_text();
            if (!sSubFolder.isEmpty())
            {
                Reference< XContent> xContent;
                if ( xNameAccess->hasByName(sSubFolder) )
                    xContent.set(xNameAccess->getByName(sSubFolder),UNO_QUERY);
                if ( xContent.is() )
                {
                    m_xContent = xContent;
                    Initialize();
                    initCurrentPath();
                }
            }
        }
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("dbaccess");
    }

    return true;
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

            m_xFTCurrentPath->set_label(sPath);
            Reference<XChild> xChild(m_xContent,UNO_QUERY);
            bEnable = xChild.is() && Reference<XNameAccess>(xChild->getParent(),UNO_QUERY).is();
        }
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("dbaccess");
    }
    m_xUp->set_sensitive(bEnable);
}

OUString OCollectionView::getName() const
{
    return m_xName->get_text();
}

#define ROW_TITLE 1
#define ROW_IS_FOLDER 2

void OCollectionView::Initialize()
{
    weld::WaitObject aWaitCursor(m_xDialog.get());

    m_xView->clear();

    try
    {
        ::ucbhelper::Content aContent(m_xContent, m_xCmdEnv, comphelper::getProcessComponentContext());
        Sequence<OUString> aProps(2);
        aProps[0] = "Title";
        aProps[1] = "IsFolder";
        auto xDynResultSet = aContent.createDynamicCursor(aProps, ucbhelper::INCLUDE_FOLDERS_ONLY);
        if (!xDynResultSet.is())
            return;

        Reference<XResultSet> xResultSet = xDynResultSet->getStaticResultSet();
        Reference<XRow> xRow(xResultSet, UNO_QUERY);
        while (xResultSet->next())
        {
            if (!xRow->getBoolean(ROW_IS_FOLDER))
                continue;
            m_xView->append_text(xRow->getString(ROW_TITLE));
        }
    }
    catch (const Exception&)
    {
        DBG_UNHANDLED_EXCEPTION("dbaccess");
    }
}

}   // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
