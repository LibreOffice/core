/*************************************************************************
 *
 *  $RCSfile: tabletree.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: fs $ $Date: 2000-10-13 16:02:21 $
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

#ifndef _DBAUI_TABLETREE_HXX_
#include "tabletree.hxx"
#endif
#ifndef _DBAUI_TABLETREE_HRC_
#include "tabletree.hrc"
#endif
#ifndef _DBAUI_MODULE_DBU_HXX_
#include "moduledbu.hxx"
#endif
#ifndef _DBU_RESOURCE_HRC_
#include "dbu_resource.hrc"
#endif
#ifndef _SV_MENU_HXX
#include <vcl/menu.hxx>
#endif
#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include <connectivity/dbtools.hxx>
#endif
#ifndef _DBASHARED_STRINGCONSTANTS_HRC_
#include "stringconstants.hrc"
#endif

#ifndef _COM_SUN_STAR_SDBC_XDRIVERACCESS_HPP_
#include <com/sun/star/sdbc/XDriverAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XDATADEFINITIONSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XDataDefinitionSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XVIEWSSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XViewsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XTABLESSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#endif

//.........................................................................
namespace dbaui
{
//.........................................................................

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;

using namespace ::dbtools;
using namespace ::dbaccess;

//========================================================================
//= OTableTreeListBox
//========================================================================
OTableTreeListBox::OTableTreeListBox( Window* pParent, WinBits nWinStyle )
    :OMarkableTreeListBox(pParent,nWinStyle)
    ,m_aTableImage(ResId(TABLE_TREE_ICON))
    ,m_aViewImage(ResId(VIEW_TREE_ICON))
{
    SetDefaultExpandedEntryBmp(Image(ModuleRes(TABLEFOLDER_TREE_ICON)));
    SetDefaultCollapsedEntryBmp(Image(ModuleRes(TABLEFOLDER_TREE_ICON)));
}
//------------------------------------------------------------------------
OTableTreeListBox::OTableTreeListBox( Window* pParent, const ResId& rResId )
    :OMarkableTreeListBox(pParent,rResId)
    ,m_aTableImage(ModuleRes(TABLE_TREE_ICON))
    ,m_aViewImage(ModuleRes(VIEW_TREE_ICON))
{
    SetDefaultExpandedEntryBmp(Image(ModuleRes(TABLEFOLDER_TREE_ICON)));
    SetDefaultCollapsedEntryBmp(Image(ModuleRes(TABLEFOLDER_TREE_ICON)));
}
//------------------------------------------------------------------------
void OTableTreeListBox::Command( const CommandEvent& rEvt )
{
    sal_Bool bHandled = sal_False;
    switch( rEvt.GetCommand() )
    {
        case COMMAND_CONTEXTMENU:
        {
            // die Stelle, an der geklickt wurde
            Point ptWhere;
            if (rEvt.IsMouseEvent())
            {
                ptWhere = rEvt.GetMousePosPixel();
                SvLBoxEntry* ptClickedOn = GetEntry(ptWhere);
                if (ptClickedOn == NULL)
                    break;
                if ( !IsSelected(ptClickedOn) )
                {
                    SelectAll(sal_False);
                    Select(ptClickedOn, sal_True);
                    SetCurEntry(ptClickedOn);
                }
            }
            else
                ptWhere = GetEntryPos(GetCurEntry());


            SvLBoxEntry* pCurrent = GetCurEntry();
            if (!pCurrent)
                break;

            SvSortMode eSortMode = GetModel()->GetSortMode();
            PopupMenu aContextMenu(ModuleRes(MENU_TABLETREE_POPUP));
            switch( aContextMenu.Execute( this, ptWhere ) )
            {
                case MID_SORT_ASCENDING:
                    GetModel()->SetSortMode(SortAscending);
                    break;
                case MID_SORT_DECENDING:
                    GetModel()->SetSortMode(SortDescending);
                    break;
            }
            if(eSortMode != GetModel()->GetSortMode())
                GetModel()->Resort();
        }
        break;
        default:
            SvTreeListBox::Command( rEvt );
    }
}

//------------------------------------------------------------------------
Reference< XConnection > OTableTreeListBox::UpdateTableList(const ::rtl::OUString& _rConnectionURL, const Sequence< PropertyValue > _rProperties) throw(SQLException)
{
    Reference< XDatabaseMetaData > xMetaData;
    Reference< XConnection > xConnection;

    Reference< XNameAccess > xTables, xViews;
    DBG_ASSERT(m_xORB.is(), "OTableTreeListBox::UpdateTableList : please use setServiceFactory to give me a service factory !");

    String sCurrentActionError;
    String sCurrentActionDetails;
    try
    {
        if (m_xORB.is())
        {
            // get the global DriverManager
            Reference< XDriverAccess > xDriverManager;
            sCurrentActionError = String(ModuleRes(STR_COULDNOTCREATE_DRIVERMANAGER));
                // in case an error occures
            sCurrentActionError.SearchAndReplaceAscii("#servicename#", (::rtl::OUString)SERVICE_SDBC_DRIVERMANAGER);
            try
            {
                xDriverManager = Reference< XDriverAccess >(m_xORB->createInstance(SERVICE_SDBC_DRIVERMANAGER), UNO_QUERY);
                DBG_ASSERT(xDriverManager.is(), "OTableTreeListBox::UpdateTableList : could not instantiate the driver manager, or it does not provide the necessary interface!");
            }
            catch (Exception& e)
            {
                // wrap the exception into an SQLException
                SQLException aSQLWrapper(e.Message, m_xORB, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("S1000")), 0, Any());
                throw aSQLWrapper;
            }
            if (!xDriverManager.is())
                throw Exception();


            sCurrentActionError = String(ModuleRes(STR_NOREGISTEREDDRIVER));
            Reference< XDriver > xDriver = xDriverManager->getDriverByURL(_rConnectionURL);
            if (!xDriver.is())
                // will be caught and translated into an SQLContext exception
                throw Exception();

            // get the (very necessary) interface XDataDefinitionSupplier
            sCurrentActionError = String(ModuleRes(STR_INVALIDREGISTEREDDRIVER));
            Reference< XDataDefinitionSupplier > xDefinitionAccess;
            xDefinitionAccess = Reference< XDataDefinitionSupplier >(xDriver, UNO_QUERY);
            if (!xDefinitionAccess.is())
                // will be caught and translated into an SQLContext exception
                throw Exception();

            sCurrentActionError = String(ModuleRes(STR_COULDNOTCONNECT));
            sCurrentActionDetails = String(ModuleRes(STR_COULDNOTCONNECT_PLEASECHECK));
            xConnection = xDriver->connect(_rConnectionURL, _rProperties);
                // exceptions thrown by connect will be caught and re-routed
            DBG_ASSERT(xConnection.is(), "OTableTreeListBox::UpdateTableList : got an invalid connection!");
                // if no exception was thrown, the connection should be no-NULL)
            if (!xConnection.is())
                throw Exception();
            sCurrentActionDetails = String();

            Reference< XTablesSupplier > xTableSupp;
            Reference< XViewsSupplier > xViewSupp;
            sCurrentActionError = String(ModuleRes(STR_NOTABLEINFO));
            xMetaData = xConnection->getMetaData();

            // get the table supplier and the tables
            xTableSupp = xDefinitionAccess->getDataDefinitionByConnection(xConnection);
            if (!xTableSupp.is())
                throw Exception();

            xTables = xTableSupp->getTables();

            // get the views supplier and the views
            xViewSupp = Reference< XViewsSupplier >(xTableSupp, UNO_QUERY);
            if (xViewSupp.is())
                xViews = xViewSupp->getViews();
        }
    }
    catch(RuntimeException&)
    {
        DBG_ERROR("OTableTreeListBox::UpdateTableList : caught an RuntimeException!");
    }
    catch(SQLException& e)
    {
        sCurrentActionError.SearchAndReplaceAscii("#connurl#", _rConnectionURL);
        // prepend a string stating what we were doing and throw again
        SQLContext aExtendedInfo = prependContextInfo(e, NULL, sCurrentActionError.GetBuffer(), sCurrentActionDetails.GetBuffer());
        throw aExtendedInfo;
    }
    catch(Exception&)
    {
        sCurrentActionError.SearchAndReplaceAscii("#connurl#", _rConnectionURL);
        // a non-SQLException exception occured ... simply throw an SQLContext
        SQLContext aExtendedInfo;
        aExtendedInfo.Message = sCurrentActionError.GetBuffer();
        throw aExtendedInfo;
    }

    UpdateTableList(xMetaData, xTables, xViews);
    return xConnection;
}

//------------------------------------------------------------------------
void OTableTreeListBox::UpdateTableList(const Reference< XDatabaseMetaData >& _rxConnMetaData, const Reference< XNameAccess > _rxTables, const Reference< XNameAccess > _rxViews)
{
    // throw away all the old stuff
    Clear();

    if (!_rxTables.is() && !_rxViews.is())
        // nothing to do
        return;

    try
    {
        // get the table/view names
        Sequence< ::rtl::OUString > aTables, aViews;
        const ::rtl::OUString* pTables = NULL;
        const ::rtl::OUString* pViews = NULL;
        if (_rxTables.is())
        {
            aTables = _rxTables->getElementNames();
            pTables = aTables.getConstArray();
        }
        if (_rxViews.is())
        {
            aViews = _rxViews->getElementNames();
            pViews = aViews.getConstArray();
        }

        ::rtl::OUString sCatalog, sSchema, sName;
        SvLBoxEntry* pCat = NULL;
        SvLBoxEntry* pSchema = NULL;

        // loop through both sequences
        const ::rtl::OUString* pSwitchSequences = (pTables && pViews) ? pTables + aTables.getLength() - 1 : NULL;

        sal_Int32 nOverallLen = aTables.getLength() + aViews.getLength();
        const ::rtl::OUString* pCurrentTable = pTables ? pTables : pViews;  // currently handled table or view name
        sal_Bool bIsView = pTables ? sal_True : sal_False;  // pCurrentTable points to a view name ?
        for (   sal_Int32 i = 0;
                i < nOverallLen;
                ++i                                                                 // inc the counter
                ,   (   pSwitchSequences == pCurrentTable                           // did we reached the last table ?
                    ?   bIsView = ((pCurrentTable = pViews) != NULL)                    // yes -> continue with the views, and set bIsView to sal_True
                    :   ++pCurrentTable != NULL                                         // no -> next table
                    )                                                                   //  (!= NULL is to make this a boolean expression, so it should work under SUNPRO5, too)
            )
        {
            pCat = pSchema = NULL;
            // the image : table or view
            Image& aImage = bIsView ? m_aViewImage : m_aTableImage;
            // split the complete name into it's components
            qualifiedNameComponents(_rxConnMetaData, *pCurrentTable, sCatalog, sSchema, sName);

            if (sCatalog.getLength())
            {
                pCat = GetEntryPosByName(sCatalog);
                if(!pCat)
                    pCat = InsertEntry(sCatalog);
            }

            if (sSchema.getLength())
            {
                pSchema = GetEntryPosByName(sSchema);
                if(!pSchema)
                    pSchema = InsertEntry(sSchema, pCat);
            }

            InsertEntry(sName, aImage, aImage, pSchema ? pSchema : pCat);
        }
    }
    catch(RuntimeException&)
    {
        DBG_ERROR("OTableTreeListBox::UpdateTableList : caught a RuntimeException!");
    }
}

//.........................................................................
}   // namespace dbaui
//.........................................................................

/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *  Revision 1.2  2000/10/09 12:36:50  fs
 *  reinserted the code for connecting in UpdateTableList
 *
 *  Revision 1.1  2000/10/05 10:00:59  fs
 *  initial checkin
 *
 *
 *  Revision 1.0 28.09.00 13:32:32  fs
 ************************************************************************/

