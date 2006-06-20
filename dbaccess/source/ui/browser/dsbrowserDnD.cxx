/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dsbrowserDnD.cxx,v $
 *
 *  $Revision: 1.73 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 02:57:27 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _SBA_UNODATBR_HXX_
#include "unodatbr.hxx"
#endif

#ifndef _COM_SUN_STAR_SDB_COMMANDTYPE_HPP_
#include <com/sun/star/sdb/CommandType.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XCONNECTION_HPP_
#include <com/sun/star/sdbc/XConnection.hpp>
#endif
#ifndef DBAUI_DBTREEMODEL_HXX
#include "dbtreemodel.hxx"
#endif
#ifndef DBACCESS_UI_DBTREEVIEW_HXX
#include "dbtreeview.hxx"
#endif
#ifndef DBACCESS_SHARED_DBUSTRINGS_HRC
#include "dbustrings.hrc"
#endif
#ifndef _DBU_BRW_HRC_
#include "dbu_brw.hrc"
#endif
#ifndef _DBAUI_MODULE_DBU_HXX_
#include "moduledbu.hxx"
#endif
#ifndef _DBHELPER_DBEXCEPTION_HXX_
#include <connectivity/dbexception.hxx>
#endif
#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include <connectivity/dbtools.hxx>
#endif
#ifndef DBAUI_DBEXCHANGE_HXX
#include "dbexchange.hxx"
#endif
#ifndef DBAUI_ENUMTYPES_HXX
#include "QEnumTypes.hxx"
#endif
#ifndef DBAUI_TOOLS_HXX
#include "UITools.hxx"
#endif
#ifndef _SVX_DATACCESSDESCRIPTOR_HXX_
#include <svx/dataaccessdescriptor.hxx>
#endif
#ifndef DBAUI_DBTREELISTBOX_HXX
#include "dbtreelistbox.hxx"
#endif
#ifndef _COM_SUN_STAR_FRAME_XSTORABLE_HPP_
#include <com/sun/star/frame/XStorable.hpp>
#endif
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
            ::osl::MutexGuard aGuard(m_aEntryMutex);

            ::rtl::OUString aName = GetEntryText( _pApplyTo );
            ::rtl::OUString aDSName = getDataSourceAcessor( m_pTreeView->getListBox()->GetRootLevelParent( _pApplyTo ) );

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
        catch(SQLException& e)
        {
            showError(SQLExceptionInfo(e));
        }
        catch(Exception&)
        {
            DBG_ERROR("SbaTableQueryBrowser::implCopyObject: caught a generic exception!");
        }
        return NULL;
    }
    // -----------------------------------------------------------------------------
    sal_Int8 SbaTableQueryBrowser::queryDrop( const AcceptDropEvent& _rEvt, const DataFlavorExVector& _rFlavors )
    {
        // check if we're a table or query container
        SvLBoxEntry* pHitEntry = m_pTreeView->getListBox()->GetEntry( _rEvt.maPosPixel );

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
        SvLBoxEntry* pHitEntry = m_pTreeView->getListBox()->GetEntry( _rEvt.maPosPixel );
        EntryType eEntryType = getEntryType( pHitEntry );
        if (!isContainer(eEntryType))
        {
            DBG_ERROR("SbaTableQueryBrowser::executeDrop: what the hell did queryDrop do?");
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
        SvLBoxEntry* pHitEntry = m_pTreeView->getListBox()->GetEntry( _rPosPixel );
        if (!pHitEntry)
            // no drag of no entry was hit ....
            return sal_False;

        // it must be a query/table
        EntryType eEntryType = getEntryType( pHitEntry );
        if (!isObject(eEntryType))
            return DND_ACTION_NONE;

        TransferableHelper* pTransfer = implCopyObject( pHitEntry, (etTable == eEntryType || etView == eEntryType) ? CommandType::TABLE : CommandType::QUERY);
        Reference< XTransferable> xEnsureDelete = pTransfer;

        if (pTransfer)
            pTransfer->StartDrag( m_pTreeView->getListBox(), DND_ACTION_COPY );

        return NULL != pTransfer;
    }
    // -----------------------------------------------------------------------------
    IMPL_LINK(SbaTableQueryBrowser, OnCopyEntry, SvLBoxEntry*, _pEntry)
    {
        if( isEntryCopyAllowed(_pEntry) )
            copyEntry(_pEntry);
        return 0;
    }
    // -----------------------------------------------------------------------------
    sal_Bool SbaTableQueryBrowser::isEntryCutAllowed(SvLBoxEntry* /*_pEntry*/) const
    {
        // at the momoent this isn't allowed
        return sal_False;
    }
    // -----------------------------------------------------------------------------
    sal_Bool SbaTableQueryBrowser::isEntryCopyAllowed(SvLBoxEntry* _pEntry) const
    {
        EntryType eType = getEntryType(_pEntry);
        return  (eType == etTable || eType == etQuery || eType == etView);
    }
    // -----------------------------------------------------------------------------
    sal_Bool SbaTableQueryBrowser::isEntryPasteAllowed(SvLBoxEntry* /*_pEntry*/) const
    {
        return sal_False;
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
    sal_Bool SbaTableQueryBrowser::copyTagTable(OTableCopyHelper::DropDescriptor& _rDesc, sal_Bool _bCheck)
    {
        // first get the dest connection
        ::osl::MutexGuard aGuard(m_aMutex);

        SharedConnection xDestConnection;
        if ( !ensureConnection( _rDesc.pDroppedAt, xDestConnection ) )
            return sal_False;

        return m_aTableCopyHelper.copyTagTable( _rDesc, _bCheck, xDestConnection );
    }
    // -----------------------------------------------------------------------------
    IMPL_LINK( SbaTableQueryBrowser, OnAsyncDrop, void*, /*NOTINTERESTEDIN*/ )
    {
        m_nAsyncDrop = 0;
        ::vos::OGuard aSolarGuard( Application::GetSolarMutex() );
        ::osl::MutexGuard aGuard(m_aMutex);


        if ( m_aAsyncDrop.nType == E_TABLE )
        {
            SharedConnection xDestConnection;
            if ( ensureConnection( m_aAsyncDrop.pDroppedAt, xDestConnection ) && xDestConnection.is() )
            {
                SvLBoxEntry* pDataSourceEntry = m_pTreeView->getListBox()->GetRootLevelParent(m_aAsyncDrop.pDroppedAt);
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
                DBTreeListModel::DBTreeListUserData* pData = static_cast<DBTreeListModel::DBTreeListUserData*>(pEntryLoop->GetUserData());
                if(pData)
                {
                    pEntryLoop->SetUserData(NULL);
                    Reference< XContainer > xContainer(pData->xContainer, UNO_QUERY);
                    if (xContainer.is())
                        xContainer->removeContainerListener(this);

                    if ( pData->xConnection.is() )
                    {
                        DBG_ASSERT( impl_isDataSourceEntry( pEntryLoop ), "SbaTableQueryBrowser::clearTreeModel: no data source entry, but a connection?" );
                        // without this, pData->aController might not really be a valid ModelControllerConnector
                        impl_releaseConnection( pData->xConnection );
                    }

                    pData->aController.clear();
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

