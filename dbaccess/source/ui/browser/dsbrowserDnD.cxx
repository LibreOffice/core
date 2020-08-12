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

#include <dbexchange.hxx>
#include <dbtreelistbox.hxx>
#include "dbtreemodel.hxx"
#include <UITools.hxx>
#include <unodatbr.hxx>

#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/sdb/CommandType.hpp>
#include <com/sun/star/sdbc/SQLException.hpp>

#include <connectivity/dbexception.hxx>
#include <connectivity/dbtools.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <svx/dataaccessdescriptor.hxx>
#include <tools/diagnose_ex.h>
#include <osl/diagnose.h>
#include <vcl/svapp.hxx>

#include <algorithm>

namespace dbaui
{

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::sdb;
    using namespace ::com::sun::star::sdbc;
    using namespace ::com::sun::star::sdbcx;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::util;
    using namespace ::com::sun::star::frame;
    using namespace ::com::sun::star::container;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::form;
    using namespace ::com::sun::star::io;
    using namespace ::com::sun::star::i18n;
    using namespace ::com::sun::star::task;
    using namespace ::com::sun::star::datatransfer;
    using namespace ::dbtools;
    using namespace ::svx;

    bool SbaTableQueryBrowser::implCopyObject(ODataClipboard& rExchange, const weld::TreeIter& rApplyTo, sal_Int32 nCommandType)
    {
        try
        {
            OUString aName = GetEntryText(rApplyTo);
            std::unique_ptr<weld::TreeIter> xRootEntry(m_pTreeView->GetRootLevelParent(&rApplyTo));
            OUString aDSName = getDataSourceAccessor(*xRootEntry);

            SharedConnection xConnection;
            if ( CommandType::QUERY != nCommandType )
            {
                if (!ensureConnection(&rApplyTo, xConnection))
                    return false;
                rExchange.Update(aDSName, nCommandType, aName, xConnection, getNumberFormatter(), getORB());
            }
            else
                rExchange.Update(aDSName, nCommandType, aName, getNumberFormatter(), getORB());

            // the ownership goes to ODataClipboards
            return true;
        }
        catch(const SQLException& )
        {
            showError( SQLExceptionInfo( ::cppu::getCaughtException() ) );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION("dbaccess");
        }
        return false;
    }

    sal_Int8 SbaTableQueryBrowser::queryDrop( const AcceptDropEvent& _rEvt, const DataFlavorExVector& _rFlavors )
    {
        // check if we're a table or query container
        weld::TreeView& rTreeView = m_pTreeView->GetWidget();
        std::unique_ptr<weld::TreeIter> xHitEntry(rTreeView.make_iterator());
        // get_dest_row_at_pos with false cause no drop if no entry was hit exactly
        if (rTreeView.get_dest_row_at_pos(_rEvt.maPosPixel, xHitEntry.get(), false))
        {
            // it must be a container
            EntryType eEntryType = getEntryType(*xHitEntry);
            SharedConnection xConnection;
            if ( eEntryType == etTableContainer && ensureConnection(xHitEntry.get(), xConnection ) && xConnection.is())
            {
                Reference<XChild> xChild(xConnection,UNO_QUERY);
                Reference<XStorable> xStore;
                if ( xChild.is() )
                    xStore.set( getDataSourceOrModel(xChild->getParent()), UNO_QUERY );
                // check for the concrete type
                if ( xStore.is() && !xStore->isReadonly() && std::any_of(_rFlavors.begin(),_rFlavors.end(),TAppSupportedSotFunctor(E_TABLE)) )
                    return DND_ACTION_COPY;
            }
        }

        return DND_ACTION_NONE;
    }
    sal_Int8 SbaTableQueryBrowser::executeDrop( const ExecuteDropEvent& _rEvt )
    {
        weld::TreeView& rTreeView = m_pTreeView->GetWidget();
        std::unique_ptr<weld::TreeIter> xHitEntry(rTreeView.make_iterator());
        // get_dest_row_at_pos with false cause no drop if no entry was hit exactly
        if (!rTreeView.get_dest_row_at_pos(_rEvt.maPosPixel, xHitEntry.get(), false))
            return DND_ACTION_NONE;
        EntryType eEntryType = getEntryType(*xHitEntry);
        if (!isContainer(eEntryType))
        {
            OSL_FAIL("SbaTableQueryBrowser::executeDrop: what the hell did queryDrop do?");
                // queryDrop should not have allowed us to reach this situation...
            return DND_ACTION_NONE;
        }
        // a TransferableDataHelper for accessing the dropped data
        TransferableDataHelper aDroppedData(_rEvt.maDropEvent.Transferable);

        // reset the data of the previous async drop (if any)
        if ( m_nAsyncDrop )
            Application::RemoveUserEvent(m_nAsyncDrop);

        m_nAsyncDrop = nullptr;
        m_aAsyncDrop.aDroppedData.clear();
        m_aAsyncDrop.nType          = E_TABLE;
        m_aAsyncDrop.nAction        = _rEvt.mnAction;
        m_aAsyncDrop.bError         = false;
        m_aAsyncDrop.bHtml          = false;
        m_aAsyncDrop.xDroppedAt.reset();
        m_aAsyncDrop.aUrl.clear();

        // loop through the available formats and see what we can do ...
        // first we have to check if it is our own format, if not we have to copy the stream :-(
        if ( ODataAccessObjectTransferable::canExtractObjectDescriptor(aDroppedData.GetDataFlavorExVector()) )
        {
            m_aAsyncDrop.aDroppedData   = ODataAccessObjectTransferable::extractObjectDescriptor(aDroppedData);
            m_aAsyncDrop.xDroppedAt     = std::move(xHitEntry);

            // asynchron because we some dialogs and we aren't allowed to show them while in D&D
            m_nAsyncDrop = Application::PostUserEvent(LINK(this, SbaTableQueryBrowser, OnAsyncDrop));
            return DND_ACTION_COPY;
        }
        else
        {
            SharedConnection xDestConnection;
            if (  ensureConnection( xHitEntry.get(), xDestConnection )
               && xDestConnection.is()
               && m_aTableCopyHelper.copyTagTable( aDroppedData, m_aAsyncDrop, xDestConnection )
               )
            {
                m_aAsyncDrop.xDroppedAt = std::move(xHitEntry);

                // asynchron because we some dialogs and we aren't allowed to show them while in D&D
                m_nAsyncDrop = Application::PostUserEvent(LINK(this, SbaTableQueryBrowser, OnAsyncDrop));
                return DND_ACTION_COPY;
            }
        }

        return DND_ACTION_NONE;
    }

    bool SbaTableQueryBrowser::requestDrag(const weld::TreeIter& rEntry)
    {
        // it must be a query/table
        EntryType eEntryType = getEntryType(rEntry);
        if (!isObject(eEntryType))
            return false;

        ODataClipboard& rExchange = m_pTreeView->GetDataTransfer();
        return implCopyObject(rExchange, rEntry, (etTableOrView == eEntryType) ? CommandType::TABLE : CommandType::QUERY);
    }

    IMPL_LINK_NOARG(SbaTableQueryBrowser, OnCopyEntry, LinkParamNone*, void)
    {
        weld::TreeView& rTreeView = m_pTreeView->GetWidget();
        std::unique_ptr<weld::TreeIter> xSelected = rTreeView.make_iterator();
        if (rTreeView.get_selected(xSelected.get()) && isEntryCopyAllowed(*xSelected))
            copyEntry(*xSelected);
    }

    bool SbaTableQueryBrowser::isEntryCopyAllowed(const weld::TreeIter& rEntry) const
    {
        EntryType eType = getEntryType(rEntry);
        return  ( eType == etTableOrView || eType == etQuery );
    }

    void SbaTableQueryBrowser::copyEntry(weld::TreeIter& rEntry)
    {
        EntryType eType = getEntryType(rEntry);
        rtl::Reference<ODataClipboard> xTransfer(new ODataClipboard);
        if (implCopyObject(*xTransfer, rEntry, eType == etQuery ? CommandType::QUERY : CommandType::TABLE))
            xTransfer->CopyToClipboard(getView());
    }

    IMPL_LINK_NOARG( SbaTableQueryBrowser, OnAsyncDrop, void*, void )
    {
        m_nAsyncDrop = nullptr;
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( getMutex() );

        if ( m_aAsyncDrop.nType == E_TABLE )
        {
            SharedConnection xDestConnection;
            if ( ensureConnection(m_aAsyncDrop.xDroppedAt.get(), xDestConnection) && xDestConnection.is())
            {
                std::unique_ptr<weld::TreeIter> xDataSourceEntry =
                    m_pTreeView->GetRootLevelParent(m_aAsyncDrop.xDroppedAt.get());
                m_aTableCopyHelper.asyncCopyTagTable(m_aAsyncDrop, getDataSourceAccessor(*xDataSourceEntry), xDestConnection);
            }
        }

        m_aAsyncDrop.aDroppedData.clear();
    }

    void SbaTableQueryBrowser::clearTreeModel()
    {
        weld::TreeView& rTreeView = m_pTreeView->GetWidget();
        rTreeView.all_foreach([this, &rTreeView](weld::TreeIter& rEntryLoop){
            // clear the user data of the tree model
            DBTreeListUserData* pData = reinterpret_cast<DBTreeListUserData*>(rTreeView.get_id(rEntryLoop).toUInt64());
            if (pData)
            {
                rTreeView.set_id(rEntryLoop, OUString());
                Reference<XContainer> xContainer(pData->xContainer, UNO_QUERY);
                if (xContainer.is())
                    xContainer->removeContainerListener(this);

                if (pData->xConnection.is())
                {
                    // connections are to be stored *only* at the data source entries
                    impl_releaseConnection(pData->xConnection);
                }

                delete pData;
            }
            return false;
        });

        m_xCurrentlyDisplayed.reset();
    }
}   // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
