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
#include "dbtreeview.hxx"
#include <stringconstants.hxx>
#include <QEnumTypes.hxx>
#include <UITools.hxx>
#include <unodatbr.hxx>

#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/sdb/CommandType.hpp>
#include <com/sun/star/sdbc/SQLException.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>

#include <connectivity/dbexception.hxx>
#include <connectivity/dbtools.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <svx/dataaccessdescriptor.hxx>
#include <tools/diagnose_ex.h>
#include <osl/diagnose.h>
#include <vcl/treelistentry.hxx>
#include <vcl/svapp.hxx>

#include <algorithm>
#include <functional>
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

    TransferableHelper* SbaTableQueryBrowser::implCopyObject( SvTreeListEntry* _pApplyTo, sal_Int32 _nCommandType )
    {
        try
        {
            OUString aName = GetEntryText( _pApplyTo );
            OUString aDSName = getDataSourceAccessor( m_pTreeView->getListBox().GetRootLevelParent( _pApplyTo ) );

            ODataClipboard* pData = nullptr;
            SharedConnection xConnection;
            if ( CommandType::QUERY != _nCommandType )
            {
                if ( !ensureConnection( _pApplyTo, xConnection) )
                    return nullptr;
                pData = new ODataClipboard(aDSName, _nCommandType, aName, xConnection, getNumberFormatter(), getORB());
            }
            else
                pData = new ODataClipboard(aDSName, _nCommandType, aName, getNumberFormatter(), getORB());

            // the ownership goes to ODataClipboards
            return pData;
        }
        catch(const SQLException& )
        {
            showError( SQLExceptionInfo( ::cppu::getCaughtException() ) );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION("dbaccess");
        }
        return nullptr;
    }
    sal_Int8 SbaTableQueryBrowser::queryDrop( const AcceptDropEvent& _rEvt, const DataFlavorExVector& _rFlavors )
    {
        // check if we're a table or query container
        SvTreeListEntry* pHitEntry = m_pTreeView->getListBox().GetEntry( _rEvt.maPosPixel );

        if ( pHitEntry ) // no drop if no entry was hit ....
        {
            // it must be a container
            EntryType eEntryType = getEntryType( pHitEntry );
            SharedConnection xConnection;
            if ( eEntryType == etTableContainer && ensureConnection( pHitEntry, xConnection ) && xConnection.is() )
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
        SvTreeListEntry* pHitEntry = m_pTreeView->getListBox().GetEntry( _rEvt.maPosPixel );
        EntryType eEntryType = getEntryType( pHitEntry );
        if (!isContainer(eEntryType))
        {
            OSL_FAIL("SbaTableQueryBrowser::executeDrop: what the hell did queryDrop do?");
                // queryDrop should not have allowed us to reach this situation ....
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
        m_aAsyncDrop.pDroppedAt     = nullptr;
        m_aAsyncDrop.aUrl.clear();

        // loop through the available formats and see what we can do ...
        // first we have to check if it is our own format, if not we have to copy the stream :-(
        if ( ODataAccessObjectTransferable::canExtractObjectDescriptor(aDroppedData.GetDataFlavorExVector()) )
        {
            m_aAsyncDrop.aDroppedData   = ODataAccessObjectTransferable::extractObjectDescriptor(aDroppedData);
            m_aAsyncDrop.pDroppedAt     = pHitEntry;

            // asynchron because we some dialogs and we aren't allowed to show them while in D&D
            m_nAsyncDrop = Application::PostUserEvent(LINK(this, SbaTableQueryBrowser, OnAsyncDrop));
            return DND_ACTION_COPY;
        }
        else
        {
            SharedConnection xDestConnection;
            if (  ensureConnection( pHitEntry, xDestConnection )
               && xDestConnection.is()
               && m_aTableCopyHelper.copyTagTable( aDroppedData, m_aAsyncDrop, xDestConnection )
               )
            {
                m_aAsyncDrop.pDroppedAt = pHitEntry;

                // asynchron because we some dialogs and we aren't allowed to show them while in D&D
                m_nAsyncDrop = Application::PostUserEvent(LINK(this, SbaTableQueryBrowser, OnAsyncDrop));
                return DND_ACTION_COPY;
            }
        }

        return DND_ACTION_NONE;
    }

    bool SbaTableQueryBrowser::requestDrag( sal_Int8 /*_nAction*/, const Point& _rPosPixel )
    {
        // get the affected list entry
        // ensure that the entry which the user clicked at is selected
        SvTreeListEntry* pHitEntry = m_pTreeView->getListBox().GetEntry( _rPosPixel );
        if (!pHitEntry)
            // no drag of no entry was hit ....
            return false;

        // it must be a query/table
        EntryType eEntryType = getEntryType( pHitEntry );
        if (!isObject(eEntryType))
            return false;

        rtl::Reference<TransferableHelper> pTransfer = implCopyObject( pHitEntry, ( etTableOrView == eEntryType ) ? CommandType::TABLE : CommandType::QUERY);

        if (pTransfer)
            pTransfer->StartDrag( &m_pTreeView->getListBox(), DND_ACTION_COPY );

        return pTransfer.is();
    }
    IMPL_LINK_NOARG(SbaTableQueryBrowser, OnCopyEntry, LinkParamNone*, void)
    {
        SvTreeListEntry* pSelected = m_pTreeView->getListBox().FirstSelected();
        if( isEntryCopyAllowed( pSelected ) )
            copyEntry( pSelected );
    }
    bool SbaTableQueryBrowser::isEntryCopyAllowed(SvTreeListEntry const * _pEntry) const
    {
        EntryType eType = getEntryType(_pEntry);
        return  ( eType == etTableOrView || eType == etQuery );
    }
    void SbaTableQueryBrowser::copyEntry(SvTreeListEntry* _pEntry)
    {
        TransferableHelper* pTransfer = nullptr;
        Reference< XTransferable> aEnsureDelete;
        EntryType eType = getEntryType(_pEntry);
        pTransfer       = implCopyObject( _pEntry, eType == etQuery ? CommandType::QUERY : CommandType::TABLE);
        aEnsureDelete   = pTransfer;
        if (pTransfer)
            pTransfer->CopyToClipboard(getView());
    }
    IMPL_LINK_NOARG( SbaTableQueryBrowser, OnAsyncDrop, void*, void )
    {
        m_nAsyncDrop = nullptr;
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( getMutex() );

        if ( m_aAsyncDrop.nType == E_TABLE )
        {
            SharedConnection xDestConnection;
            if ( ensureConnection( m_aAsyncDrop.pDroppedAt, xDestConnection ) && xDestConnection.is() )
            {
                SvTreeListEntry* pDataSourceEntry = m_pTreeView->getListBox().GetRootLevelParent(m_aAsyncDrop.pDroppedAt);
                m_aTableCopyHelper.asyncCopyTagTable( m_aAsyncDrop, getDataSourceAccessor( pDataSourceEntry ), xDestConnection );
            }
        }

        m_aAsyncDrop.aDroppedData.clear();
    }
    void SbaTableQueryBrowser::clearTreeModel()
    {
        if (m_pTreeView)
        {
            auto pTreeModel = m_pTreeView->GetTreeModel();
            // clear the user data of the tree model
            SvTreeListEntry* pEntryLoop = pTreeModel->First();
            while (pEntryLoop)
            {
                DBTreeListUserData* pData = static_cast<DBTreeListUserData*>(pEntryLoop->GetUserData());
                if(pData)
                {
                    pEntryLoop->SetUserData(nullptr);
                    Reference< XContainer > xContainer(pData->xContainer, UNO_QUERY);
                    if (xContainer.is())
                        xContainer->removeContainerListener(this);

                    if ( pData->xConnection.is() )
                    {
                        OSL_ENSURE( impl_isDataSourceEntry( pEntryLoop ), "SbaTableQueryBrowser::clearTreeModel: no data source entry, but a connection?" );
                        // connections are to be stored *only* at the data source entries
                        impl_releaseConnection( pData->xConnection );
                    }

                    delete pData;
                }
                pEntryLoop = pTreeModel->Next(pEntryLoop);
            }
        }
        m_pCurrentlyDisplayed = nullptr;
    }
}   // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
