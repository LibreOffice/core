/*************************************************************************
 *
 *  $RCSfile: dbtreemodel.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: oj $ $Date: 2000-10-26 14:41:05 $
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

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::beans;
using namespace dbaui;
// -------------------------------------------------------------------------

DBTreeListModel::DBTreeListModel(const Reference< ::com::sun::star::lang::XMultiServiceFactory >& _xMultiServiceFacatory)
{
    DBG_ASSERT(_xMultiServiceFacatory.is(), "DBTreeListModel::DBTreeListModel : need a service factory !");
    try
    {
        m_xDatabaseContext = Reference< XNameAccess >(_xMultiServiceFacatory->createInstance(SERVICE_SDB_DATABASECONTEXT), UNO_QUERY);
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
            sal_Bool bComplete = sal_False;
            Reference<XPropertySet> xProp;
            aValue >>= xProp;
            Any aPwd    = xProp->getPropertyValue(::rtl::OUString::createFromAscii("Password"));
            Any aPwdReq = xProp->getPropertyValue(::rtl::OUString::createFromAscii("IsPasswordRequired"));
            Any aUser   = xProp->getPropertyValue(::rtl::OUString::createFromAscii("User"));
            // only complete when password is requiered and password is empty
            if(cppu::any2bool(aPwdReq) && !comphelper::getString(aPwd).getLength())
                bComplete = sal_True;

            Reference<XConnection> xConnection;  // supports the service sdb::connection
            if(bComplete)
            {
                Reference<XCompletedConnection> xCompletedConnection;
                aValue >>= xCompletedConnection;
               //   xConnection->connectWithCompletion();
            }
            else
            {
                Reference<XDataSource> xDataSource(xProp,UNO_QUERY);
                xConnection = xDataSource->getConnection(comphelper::getString(aUser),comphelper::getString(aPwd));
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
