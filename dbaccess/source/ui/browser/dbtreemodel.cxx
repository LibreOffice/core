/*************************************************************************
 *
 *  $RCSfile: dbtreemodel.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: fs $ $Date: 2000-11-06 17:43:44 $
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
 *  Source License Version 1.1 (the License); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an AS IS basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _COM_SUN_STAR_UTIL_XFLUSHABLE_HPP_
#include <com/sun/star/util/XFlushable.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XDATASOURCE_HPP_
#include <com/sun/star/sdbc/XDataSource.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XCONNECTION_HPP_
#include <com/sun/star/sdbc/XConnection.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XTABLESSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XVIEWSSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XViewsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XCOMPLETEDCONNECTION_HPP_
#include <com/sun/star/sdb/XCompletedConnection.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XQUERYDEFINITIONSSUPPLIER_HPP_
#include <com/sun/star/sdb/XQueryDefinitionsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_TASK_XINTERACTIONHANDLER_HPP_
#include <com/sun/star/task/XInteractionHandler.hpp>
#endif
#ifndef _COM_SUN_STAR_UI_XEXECUTABLEDIALOG_HPP_
#include <com/sun/star/ui/XExecutableDialog.hpp>
#endif
#include <com/sun/star/beans/PropertyValue.hpp>
#ifndef _CPPUHELPER_EXTRACT_HXX_
#include <cppuhelper/extract.hxx>
#endif
#ifndef _DBU_RESOURCE_HRC_
#include "dbu_resource.hrc"
#endif
#ifndef DBACCESS_SHARED_DBUSTRINGS_HRC
#include "dbustrings.hrc"
#endif
#ifndef DBAUI_DBTREEMODEL_HXX
#include "dbtreemodel.hxx"
#endif
#ifndef _SVLBOXITM_HXX
#include <svtools/svlbitm.hxx>
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif
#ifndef _DBHELPER_DBEXCEPTION_HXX_
#include <connectivity/dbexception.hxx>
#endif
#ifndef _VCL_STDTEXT_HXX
#include <vcl/stdtext.hxx>
#endif

using namespace ::com::sun::star::ui;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::task;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::beans;
using namespace ::dbaui;
using namespace ::dbtools;

// -------------------------------------------------------------------------

DBTreeListModel::DBTreeListModel(const Reference< ::com::sun::star::lang::XMultiServiceFactory >& _xMultiServiceFacatory)
    :m_xORB(_xMultiServiceFacatory)
{
    DBG_ASSERT(m_xORB.is(), "DBTreeListModel::DBTreeListModel : need a service factory !");
    try
    {
        m_xDatabaseContext = Reference< XNameAccess >(m_xORB->createInstance(SERVICE_SDB_DATABASECONTEXT), UNO_QUERY);
    }
    catch(Exception&)
    {
    }
    if (m_xDatabaseContext.is())
    {
        String aNames[2];
        aNames[0] = String(ModuleRes(STR_QRY_TITLE)).GetToken(0,' ');
        aNames[1] = String(ModuleRes(STR_TBL_TITLE)).GetToken(0,' ');

        // images for the table and query folders
        Image aFolderImage[2] = {
                    Image(ModuleRes(QUERYFOLDER_TREE_ICON)),
                    Image(ModuleRes(TABLEFOLDER_TREE_ICON))
                };

        Image aDBImage(ModuleRes(IMG_DATABASE));
        // fill the model with the names of the registered datasources
        Sequence< ::rtl::OUString > aDatasources = m_xDatabaseContext->getElementNames();
        const ::rtl::OUString* pBegin   = aDatasources.getConstArray();
        const ::rtl::OUString* pEnd     = pBegin + aDatasources.getLength();
        for (; pBegin != pEnd;++pBegin)
        {
            SvLBoxEntry* pEntry     = new SvLBoxEntry();
            // add items
            {   // just for reuse of the names
                SvLBoxContextBmp* pContextBmp = new SvLBoxContextBmp( pEntry, 0,aDBImage,aDBImage,0 );
                pEntry->AddItem( pContextBmp );

                SvLBoxString* pString   = new SvLBoxString( pEntry, 0, *pBegin );
                pEntry->AddItem( pString );

                // here we safe the connection
                DBTreeListUserData* pData = new DBTreeListUserData;
                pEntry->SetUserData(pData);
            }
            Insert(pEntry);



            for(sal_Int32 i=0;i<2;++i)
            {
                SvLBoxEntry* pChild = new SvLBoxEntry();
                pChild->EnableChildsOnDemand();

                SvLBoxContextBmp* pContextBmp = new SvLBoxContextBmp( pChild, 0,aFolderImage[i],aFolderImage[i],0 );
                pChild->AddItem( pContextBmp );

                SvLBoxString* pString   = new SvLBoxString( pChild, 0, aNames[i] );
                pChild->AddItem( pString );

                DBTreeListUserData* pData = new DBTreeListUserData;
                pData->bTable = i;
                pChild->SetUserData(pData);
                Insert(pChild,pEntry);
            }
        }
    }
}
// -------------------------------------------------------------------------
DBTreeListModel::~DBTreeListModel()
{
    SvLBoxEntry* pNext = (SvLBoxEntry*)First();
    while(pNext)
    {
         DBTreeListUserData* pData = static_cast<DBTreeListUserData*>(pNext->GetUserData());
         if(pData)
         {
             pData->xInterface = NULL;
             delete pData;
         }
         pNext = (SvLBoxEntry*)Next(pNext);
    }
    m_xDatabaseContext = NULL;
}
// -------------------------------------------------------------------------
void DBTreeListModel::fillEntry(SvLBoxEntry* _pParent)
{
    if(!_pParent->HasChilds())
    {
        SvLBoxEntry* pFirstParent = (SvLBoxEntry*)GetRootLevelParent(_pParent);
        OSL_ENSHURE(pFirstParent,"DBTreeListModel::fillEntry: No rootlevelparent!");

        DBTreeListUserData* pData = (DBTreeListUserData*)_pParent->GetUserData();
        OSL_ENSHURE(pData,"DBTreeListModel::fillEntry: No user data!");
        SvLBoxString* pString = (SvLBoxString*)pFirstParent->GetFirstItem(SV_ITEM_ID_LBOXSTRING);
        OSL_ENSHURE(pString,"DBTreeListModel::fillEntry: No string item!");
        String aName(pString->GetText());
        Any aValue(m_xDatabaseContext->getByName(aName));
        if(pData->bTable)
        {
            Reference<XPropertySet> xProp;
            aValue >>= xProp;
            ::rtl::OUString sPwd, sUser;
            sal_Bool bPwdReq = sal_False;
            try
            {
                xProp->getPropertyValue(PROPERTY_PASSWORD) >>= sPwd;
                bPwdReq = cppu::any2bool(xProp->getPropertyValue(PROPERTY_ISPASSWORDREQUIRED));
                xProp->getPropertyValue(PROPERTY_USER) >>= sUser;
            }
            catch(Exception&)
            {
                DBG_ERROR("DBTreeListModel::fillEntry: error while retrieving data source properties!");
            }

            SQLExceptionInfo aInfo;
            try
            {

                Reference<XConnection> xConnection;  // supports the service sdb::connection
                if(bPwdReq && !sPwd.getLength())
                {   // password required, but empty -> connect using an interaction handler
                    Reference<XCompletedConnection> xConnectionCompletion(xProp, UNO_QUERY);
                    if (!xConnectionCompletion.is())
                    {
                        DBG_ERROR("DBTreeListModel::fillEntry: missing an interface ... need an error message here!");
                    }
                    else
                    {   // instantiate the default SDB interaction handler
                        Reference< XInteractionHandler > xHandler(m_xORB->createInstance(SERVICE_SDB_INTERACTION_HANDLER), UNO_QUERY);
                        if (!xHandler.is())
                        {
                            ShowServiceNotAvailableError(NULL, String(SERVICE_SDB_INTERACTION_HANDLER), sal_True);
                                // TODO: a real parent!
                        }
                        else
                        {
                            xConnection = xConnectionCompletion->connectWithCompletion(xHandler);
                        }
                    }
                }
                else
                {
                    Reference<XDataSource> xDataSource(xProp,UNO_QUERY);
                    xConnection = xDataSource->getConnection(sUser, sPwd);
                }
                if(xConnection.is())
                {
                    DBTreeListUserData* pFirstData = (DBTreeListUserData*)pFirstParent->GetUserData();
                    if(!pFirstData->xInterface.is())
                        pFirstData->xInterface = xConnection;

                    Reference<XTablesSupplier> xTabSup(xConnection,UNO_QUERY);
                    if(xTabSup.is())
                    {
                        Image aImage(ModuleRes(TABLE_TREE_ICON));
                        insertEntries(xTabSup->getTables(),_pParent,aImage);
                    }

                    Reference<XViewsSupplier> xViewSup(xConnection,UNO_QUERY);
                    if(xViewSup.is())
                    {
                        Image aImage(ModuleRes(VIEW_TREE_ICON));
                        insertEntries(xViewSup->getViews(),_pParent,aImage);
                    }
                }
            }
            catch(SQLException& e) { aInfo = SQLExceptionInfo(e); }
            catch(SQLWarning& e) { aInfo = SQLExceptionInfo(e); }
            catch(SQLContext& e) { aInfo = SQLExceptionInfo(e); }
            catch(Exception&) { DBG_ERROR("DBTreeListModel::fillEntry: could not connect - unknown exception!"); }
            if (aInfo.isValid())
            {
                try
                {
                    Sequence< Any > aArgs(1);
                    aArgs[0] <<= PropertyValue(PROPERTY_SQLEXCEPTION, 0, aInfo.get(), PropertyState_DIRECT_VALUE);
                    Reference< XExecutableDialog > xErrorDialog(
                        m_xORB->createInstanceWithArguments(::rtl::OUString::createFromAscii("com.sun.star.sdb.ErrorMessageDialog"), aArgs), UNO_QUERY);
                    if (xErrorDialog.is())
                        xErrorDialog->execute();
                }
                catch(Exception&)
                {
                    DBG_ERROR("DBTreeListModel::fillEntry: could not display the error message!");
                }
            }
        }
        else // we have to expand the queries
        {
            Reference<XQueryDefinitionsSupplier> xQuerySup;
            if(aValue >>= xQuerySup)
            {
                Image aImage(ModuleRes(QUERY_TREE_ICON));
                insertEntries(xQuerySup->getQueryDefinitions(),_pParent,aImage);
            }
        }
    }
}
// -------------------------------------------------------------------------
void DBTreeListModel::insertEntries(const Reference<XNameAccess>& _xNameAccess,SvLBoxEntry* _pParent,const Image& _rImage)
{
    DBTreeListUserData* pData = (DBTreeListUserData*)_pParent->GetUserData();
    if(pData)
        pData->xInterface = _xNameAccess;

    Sequence< ::rtl::OUString > aNames = _xNameAccess->getElementNames();
    const ::rtl::OUString* pBegin   = aNames.getConstArray();
    const ::rtl::OUString* pEnd     = pBegin + aNames.getLength();
    for (; pBegin != pEnd;++pBegin)
    {
        SvLBoxEntry* pEntry     = new SvLBoxEntry();
        // add items
        SvLBoxContextBmp* pContextBmp = new SvLBoxContextBmp(pEntry, 0,_rImage,_rImage,0);
        pEntry->AddItem( pContextBmp );

        SvLBoxString* pString   = new SvLBoxString( pEntry, 0, *pBegin );
        pEntry->AddItem( pString );

        Insert(pEntry,_pParent);
    }
}
