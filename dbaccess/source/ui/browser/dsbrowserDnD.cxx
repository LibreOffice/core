/*************************************************************************
 *
 *  $RCSfile: dsbrowserDnD.cxx,v $
 *
 *  $Revision: 1.64 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 15:33:59 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
            ::rtl::OUString aDSName = GetEntryText( m_pTreeView->getListBox()->GetRootLevelParent( _pApplyTo ) );

            ODataClipboard* pData = NULL;
            Reference<XConnection> xConnection;  // supports the service sdb::connection
            if ( CommandType::QUERY != _nCommandType )
            {
                if (_bAllowConnection && !ensureConnection(_pApplyTo, xConnection))
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
        return DND_ACTION_NONE;
    }
    // -----------------------------------------------------------------------------
    sal_Int8 SbaTableQueryBrowser::executeDrop( const ExecuteDropEvent& _rEvt )
    {
        return DND_ACTION_NONE;
    }

    // -----------------------------------------------------------------------------
    sal_Bool SbaTableQueryBrowser::requestDrag( sal_Int8 _nAction, const Point& _rPosPixel )
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
    sal_Bool SbaTableQueryBrowser::isEntryCutAllowed(SvLBoxEntry* _pEntry) const
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
    sal_Bool SbaTableQueryBrowser::isEntryPasteAllowed(SvLBoxEntry* _pEntry) const
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
// .........................................................................
}   // namespace dbaui
// .........................................................................

