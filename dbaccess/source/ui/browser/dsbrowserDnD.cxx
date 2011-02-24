/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbaccess.hxx"

#include "dbexchange.hxx"
#include "dbtreelistbox.hxx"
#include "dbtreemodel.hxx"
#include "dbtreeview.hxx"
#include "dbu_brw.hrc"
#include "dbustrings.hrc"
#include "QEnumTypes.hxx"
#include "UITools.hxx"
#include "unodatbr.hxx"

/** === begin UNO includes === **/
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/sdb/CommandType.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
/** === end UNO includes === **/

#include <connectivity/dbexception.hxx>
#include <connectivity/dbtools.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <svtools/treelist.hxx>
#include <svx/dataaccessdescriptor.hxx>
#include <tools/diagnose_ex.h>
#include <osl/diagnose.h>

#include <functional>
// .........................................................................
namespace dbaui
{
// .........................................................................

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

    // -----------------------------------------------------------------------------
    TransferableHelper* SbaTableQueryBrowser::implCopyObject( SvLBoxEntry* _pApplyTo, sal_Int32 _nCommandType, sal_Bool _bAllowConnection )
    {
        try
        {
            ::rtl::OUString aName = GetEntryText( _pApplyTo );
            ::rtl::OUString aDSName = getDataSourceAcessor( m_pTreeView->getListBox().GetRootLevelParent( _pApplyTo ) );

            ODataClipboard* pData = NULL;
            SharedConnection xConnection;
            if ( CommandType::QUERY != _nCommandType )
            {
                if ( _bAllowConnection && !ensureConnection( _pApplyTo, xConnection) )
                    return NULL;
                pData = new ODataClipboard(aDSName, _nCommandType, aName, xConnection, getNumberFormatter(), getORB());
            }
            else
                pData = new ODataClipboard(aDSName, _nCommandType, aName, getNumberFormatter(), getORB());

            // the owner ship goes to ODataClipboards
            return pData;
        }
        catch(const SQLException& )
        {
            showError( SQLExceptionInfo( ::cppu::getCaughtException() ) );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
        return NULL;
    }
    // -----------------------------------------------------------------------------
    sal_Int8 SbaTableQueryBrowser::queryDrop( const AcceptDropEvent& _rEvt, const DataFlavorExVector& _rFlavors )
    {
        // check if we're a table or query container
        SvLBoxEntry* pHitEntry = m_pTreeView->getListBox().GetEntry( _rEvt.maPosPixel );

        if ( pHitEntry ) // no drop if no entry was hit ....
        {
            // it must be a container
            EntryType eEntryType = getEntryType( pHitEntry );
            SharedConnection xConnection;
            if ( eEntryType == etTableContainer && ensureConnection( pHitEntry, xConnection ) && xConnection.is() )
            {
                Reference<XChild> xChild(xConnection,UNO_QUERY);
                Reference<XStorable> xStore(xChild.is() ? getDataSourceOrModel(xChild->getParent()) : Reference<XInterface>(),UNO_QUERY);
                // check for the concrete type
                if ( xStore.is() && !xStore->isReadonly() && ::std::find_if(_rFlavors.begin(),_rFlavors.end(),TAppSupportedSotFunctor(E_TABLE,sal_True)) != _rFlavors.end())
                    return DND_ACTION_COPY;
            }
        }

        return DND_ACTION_NONE;
    }
    // -----------------------------------------------------------------------------
    sal_Int8 SbaTableQueryBrowser::executeDrop( const ExecuteDropEvent& _rEvt )
    {
        SvLBoxEntry* pHitEntry = m_pTreeView->getListBox().GetEntry( _rEvt.maPosPixel );
        EntryType eEntryType = getEntryType( pHitEntry );
        if (!isContainer(eEntryType))
        {
            OSL_ASSERT("SbaTableQueryBrowser::executeDrop: what the hell did queryDrop do?");
                // queryDrop shoud not have allowed us to reach this situation ....
            return DND_ACTION_NONE;
        }
        // a TransferableDataHelper for accessing the dropped data
        TransferableDataHelper aDroppedData(_rEvt.maDropEvent.Transferable);


        // reset the data of the previous async drop (if any)
        if ( m_nAsyncDrop )
            Application::RemoveUserEvent(m_nAsyncDrop);


        m_nAsyncDrop = 0;
        m_aAsyncDrop.aDroppedData.clear();
        m_aAsyncDrop.nType          = E_TABLE;
        m_aAsyncDrop.nAction        = _rEvt.mnAction;
        m_aAsyncDrop.bError         = sal_False;
        m_aAsyncDrop.bHtml          = sal_False;
        m_aAsyncDrop.pDroppedAt     = NULL;
        m_aAsyncDrop.aUrl           = ::rtl::OUString();


        // loop through the available formats and see what we can do ...
        // first we have to check if it is our own format, if not we have to copy the stream :-(
        if ( ODataAccessObjectTransferable::canExtractObjectDescriptor(aDroppedData.GetDataFlavorExVector()) )
        {
            m_aAsyncDrop.aDroppedData   = ODataAccessObjectTransferable::extractObjectDescriptor(aDroppedData);
            m_aAsyncDrop.pDroppedAt     = pHitEntry;

            // asyncron because we some dialogs and we aren't allowed to show them while in D&D
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

                // asyncron because we some dialogs and we aren't allowed to show them while in D&D
                m_nAsyncDrop = Application::PostUserEvent(LINK(this, SbaTableQueryBrowser, OnAsyncDrop));
                return DND_ACTION_COPY;
            }
        }

        return DND_ACTION_NONE;
    }

    // -----------------------------------------------------------------------------
    sal_Bool SbaTableQueryBrowser::requestDrag( sal_Int8 /*_nAction*/, const Point& _rPosPixel )
    {
        // get the affected list entry
        // ensure that the entry which the user clicked at is selected
        SvLBoxEntry* pHitEntry = m_pTreeView->getListBox().GetEntry( _rPosPixel );
        if (!pHitEntry)
            // no drag of no entry was hit ....
            return sal_False;

        // it must be a query/table
        EntryType eEntryType = getEntryType( pHitEntry );
        if (!isObject(eEntryType))
            return DND_ACTION_NONE;

        TransferableHelper* pTransfer = implCopyObject( pHitEntry, ( etTableOrView == eEntryType ) ? CommandType::TABLE : CommandType::QUERY);
        Reference< XTransferable> xEnsureDelete = pTransfer;

        if (pTransfer)
            pTransfer->StartDrag( &m_pTreeView->getListBox(), DND_ACTION_COPY );

        return NULL != pTransfer;
    }
    // -----------------------------------------------------------------------------
    IMPL_LINK(SbaTableQueryBrowser, OnCopyEntry, void*, /*NOTINTERESIN*/)
    {
        SvLBoxEntry* pSelected = m_pTreeView->getListBox().FirstSelected();
        if( isEntryCopyAllowed( pSelected ) )
            copyEntry( pSelected );
        return 0;
    }
    // -----------------------------------------------------------------------------
    sal_Bool SbaTableQueryBrowser::isEntryCopyAllowed(SvLBoxEntry* _pEntry) const
    {
        EntryType eType = getEntryType(_pEntry);
        return  ( eType == etTableOrView || eType == etQuery );
    }
    // -----------------------------------------------------------------------------
    void SbaTableQueryBrowser::copyEntry(SvLBoxEntry* _pEntry)
    {
        TransferableHelper* pTransfer = NULL;
        Reference< XTransferable> aEnsureDelete;
        EntryType eType = getEntryType(_pEntry);
        pTransfer       = implCopyObject( _pEntry, eType == etQuery ? CommandType::QUERY : CommandType::TABLE);
        aEnsureDelete   = pTransfer;
        if (pTransfer)
            pTransfer->CopyToClipboard(getView());
    }
    // -----------------------------------------------------------------------------
    IMPL_LINK( SbaTableQueryBrowser, OnAsyncDrop, void*, /*NOTINTERESTEDIN*/ )
    {
        m_nAsyncDrop = 0;
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( getMutex() );

        if ( m_aAsyncDrop.nType == E_TABLE )
        {
            SharedConnection xDestConnection;
            if ( ensureConnection( m_aAsyncDrop.pDroppedAt, xDestConnection ) && xDestConnection.is() )
            {
                SvLBoxEntry* pDataSourceEntry = m_pTreeView->getListBox().GetRootLevelParent(m_aAsyncDrop.pDroppedAt);
                m_aTableCopyHelper.asyncCopyTagTable( m_aAsyncDrop, getDataSourceAcessor( pDataSourceEntry ), xDestConnection );
            }
        }

        m_aAsyncDrop.aDroppedData.clear();

        return 0L;
    }
    // -----------------------------------------------------------------------------
    void SbaTableQueryBrowser::clearTreeModel()
    {
        if (m_pTreeModel)
        {
            // clear the user data of the tree model
            SvLBoxEntry* pEntryLoop = m_pTreeModel->First();
            while (pEntryLoop)
            {
                DBTreeListUserData* pData = static_cast<DBTreeListUserData*>(pEntryLoop->GetUserData());
                if(pData)
                {
                    pEntryLoop->SetUserData(NULL);
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
                pEntryLoop = m_pTreeModel->Next(pEntryLoop);
            }
        }
        m_pCurrentlyDisplayed = NULL;
    }
// .........................................................................
}   // namespace dbaui
// .........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
