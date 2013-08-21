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


#include <sot/formats.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/sdbc/XDataSource.hpp>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/sdb/DatabaseContext.hpp>
#include <com/sun/star/sdb/XQueriesSupplier.hpp>
#include <com/sun/star/sdb/XDatabaseAccess.hpp>
#include <com/sun/star/sdb/CommandType.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/sdb/XCompletedConnection.hpp>
#include <com/sun/star/container/XContainerListener.hpp>
#include <com/sun/star/container/XContainer.hpp>
#include <cppuhelper/implbase1.hxx>
#include <svx/dbaexchange.hxx>

#include <dbmgr.hxx>
#include <swmodule.hxx>
#include <view.hxx>
#include <wrtsh.hxx>
#include <dbtree.hxx>
#include <osl/mutex.hxx>
#include <vcl/builder.hxx>
#include <vcl/svapp.hxx>
#include "svtools/treelistentry.hxx"

#include <helpid.h>
#include <utlui.hrc>

#include <unomid.h>

#include <boost/ptr_container/ptr_vector.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::task;
using namespace ::com::sun::star::beans;

struct SwConnectionData
{
    OUString                sSourceName;
    Reference<XConnection>  xConnection;
};

typedef boost::ptr_vector<SwConnectionData> SwConnectionArr;

class SwDBTreeList_Impl : public cppu::WeakImplHelper1 < XContainerListener >
{
    Reference< XDatabaseContext > xDBContext;
    SwConnectionArr aConnections;
    SwWrtShell* pWrtSh;

    public:
        SwDBTreeList_Impl(SwWrtShell* pShell) :
            pWrtSh(pShell) {}
        ~SwDBTreeList_Impl();

    virtual void SAL_CALL elementInserted( const ContainerEvent& Event ) throw (RuntimeException);
    virtual void SAL_CALL elementRemoved( const ContainerEvent& Event ) throw (RuntimeException);
    virtual void SAL_CALL elementReplaced( const ContainerEvent& Event ) throw (RuntimeException);
    virtual void SAL_CALL disposing( const EventObject& Source ) throw (RuntimeException);

    sal_Bool                        HasContext();
    SwWrtShell*                 GetWrtShell() { return pWrtSh;}
    void                        SetWrtShell(SwWrtShell& rSh) { pWrtSh = &rSh;}
    Reference<XDatabaseContext>    GetContext() const {return xDBContext;}
    Reference<XConnection>      GetConnection(const OUString& rSourceName);
};

SwDBTreeList_Impl::~SwDBTreeList_Impl()
{
    if(xDBContext.is())
    {
        m_refCount++;
        //block necessary due to solaris' compiler behaviour to
        //remove temporaries at the block's end
        {
            xDBContext->removeContainerListener( this );
        }
        m_refCount--;
    }
}

void SwDBTreeList_Impl::elementInserted( const ContainerEvent&  ) throw (RuntimeException)
{
    // information not needed
}

void SwDBTreeList_Impl::elementRemoved( const ContainerEvent& rEvent ) throw (RuntimeException)
{
    SolarMutexGuard aGuard;
    OUString sSource;
    rEvent.Accessor >>= sSource;
    for(SwConnectionArr::iterator i = aConnections.begin(); i != aConnections.end(); ++i)
    {
        if(i->sSourceName == sSource)
        {
            aConnections.erase(i);
            break;
        }
    }
}

void SwDBTreeList_Impl::disposing( const EventObject&  ) throw (RuntimeException)
{
    xDBContext = 0;
}

void SwDBTreeList_Impl::elementReplaced( const ContainerEvent& rEvent ) throw (RuntimeException)
{
    elementRemoved(rEvent);
}

sal_Bool SwDBTreeList_Impl::HasContext()
{
    if(!xDBContext.is())
    {
        Reference< XComponentContext > xContext( ::comphelper::getProcessComponentContext() );
        xDBContext = DatabaseContext::create(xContext);
        xDBContext->addContainerListener( this );
    }
    return xDBContext.is();
}

Reference<XConnection>  SwDBTreeList_Impl::GetConnection(const OUString& rSourceName)
{
    Reference<XConnection> xRet;
    for(SwConnectionArr::const_iterator i = aConnections.begin(); i != aConnections.end(); ++i)
    {
        if(i->sSourceName == rSourceName)
        {
            xRet = i->xConnection;
            break;
        }
    }
    if(!xRet.is() && xDBContext.is() && pWrtSh)
    {
        SwConnectionData* pPtr = new SwConnectionData();
        pPtr->sSourceName = rSourceName;
        xRet = pWrtSh->GetNewDBMgr()->RegisterConnection(pPtr->sSourceName);
        aConnections.push_back(pPtr);
    }
    return xRet;
}

SwDBTreeList::SwDBTreeList(Window *pParent, WinBits nStyle)
    : SvTreeListBox(pParent, nStyle)
    , aImageList(SW_RES(ILIST_DB_DLG))
    , bInitialized(false)
    , bShowColumns(false)
    , pImpl(new SwDBTreeList_Impl(NULL))
{
    if (IsVisible())
        InitTreeList();
}

extern "C" SAL_DLLPUBLIC_EXPORT Window* SAL_CALL makeSwDBTreeList(Window *pParent, VclBuilder::stringmap &rMap)
{
    WinBits nStyle = WB_TABSTOP;
    OString sBorder = VclBuilder::extractCustomProperty(rMap);
    if (!sBorder.isEmpty())
        nStyle |= WB_BORDER;
    return new SwDBTreeList(pParent, nStyle);
}

Size SwDBTreeList::GetOptimalSize() const
{
    return LogicToPixel(Size(100, 62), MapMode(MAP_APPFONT));
}

SwDBTreeList::~SwDBTreeList()
{
    delete pImpl;
}

void SwDBTreeList::InitTreeList()
{
    if(!pImpl->HasContext() && pImpl->GetWrtShell())
        return;
    SetSelectionMode(SINGLE_SELECTION);
    SetStyle(GetStyle()|WB_HASLINES|WB_CLIPCHILDREN|WB_HASBUTTONS|WB_HASBUTTONSATROOT|WB_HSCROLL);
    // don't set font, so that the Control's font is being applied!
    SetSpaceBetweenEntries(0);
    SetNodeBitmaps( aImageList.GetImage(IMG_COLLAPSE),
                    aImageList.GetImage(IMG_EXPAND  ) );

    SetDragDropMode(SV_DRAGDROP_APP_COPY);

    GetModel()->SetCompareHdl(LINK(this, SwDBTreeList, DBCompare));

    Sequence< OUString > aDBNames = pImpl->GetContext()->getElementNames();
    const OUString* pDBNames = aDBNames.getConstArray();
    long nCount = aDBNames.getLength();

    Image aImg = aImageList.GetImage(IMG_DB);
    for(long i = 0; i < nCount; i++)
    {
        String sDBName(pDBNames[i]);
        InsertEntry(sDBName, aImg, aImg, NULL, sal_True);
    }
    String sDBName(sDefDBName.GetToken(0, DB_DELIM));
    String sTableName(sDefDBName.GetToken(1, DB_DELIM));
    String sColumnName(sDefDBName.GetToken(2, DB_DELIM));
    Select(sDBName, sTableName, sColumnName);

    bInitialized = true;
}

void    SwDBTreeList::AddDataSource(const String& rSource)
{
    Image aImg = aImageList.GetImage(IMG_DB);
    SvTreeListEntry* pEntry = InsertEntry(rSource, aImg, aImg, NULL, sal_True);
    SvTreeListBox::Select(pEntry);
}

void SwDBTreeList::ShowColumns(sal_Bool bShowCol)
{
    if (bShowCol != bShowColumns)
    {
        bShowColumns = bShowCol;
        String sTableName, sColumnName;
        String  sDBName(GetDBName(sTableName, sColumnName));

        SetUpdateMode(sal_False);

        SvTreeListEntry* pEntry = First();

        while (pEntry)
        {
            pEntry = (SvTreeListEntry*)GetRootLevelParent( pEntry );
            Collapse(pEntry);       // zuklappen

            SvTreeListEntry* pChild;
            while ((pChild = FirstChild(pEntry)) != 0L)
                GetModel()->Remove(pChild);

            pEntry = Next(pEntry);
        }

        if (sDBName.Len())
        {
            Select(sDBName, sTableName, sColumnName);   // force RequestingChildren
        }
        SetUpdateMode(sal_True);
    }
}

void  SwDBTreeList::RequestingChildren(SvTreeListEntry* pParent)
{
    if (!pParent->HasChildren())
    {
        if (GetParent(pParent)) // column names
        {
            try
            {

                String sSourceName = GetEntryText(GetParent(pParent));
                String sTableName = GetEntryText(pParent);

                if(!pImpl->GetContext()->hasByName(sSourceName))
                    return;
                Reference<XConnection> xConnection = pImpl->GetConnection(sSourceName);
                bool bTable = pParent->GetUserData() == 0;
                Reference<XColumnsSupplier> xColsSupplier;
                if(bTable)
                {
                    Reference<XTablesSupplier> xTSupplier = Reference<XTablesSupplier>(xConnection, UNO_QUERY);
                    if(xTSupplier.is())
                    {
                        Reference<XNameAccess> xTbls = xTSupplier->getTables();
                        OSL_ENSURE(xTbls->hasByName(sTableName), "table not available anymore?");
                        try
                        {
                            Any aTable = xTbls->getByName(sTableName);
                            Reference<XPropertySet> xPropSet;
                            aTable >>= xPropSet;
                            xColsSupplier = Reference<XColumnsSupplier>(xPropSet, UNO_QUERY);
                        }
                        catch (const Exception&)
                        {
                        }
                    }
                }
                else
                {
                    Reference<XQueriesSupplier> xQSupplier = Reference<XQueriesSupplier>(xConnection, UNO_QUERY);
                    if(xQSupplier.is())
                    {
                        Reference<XNameAccess> xQueries = xQSupplier->getQueries();
                        OSL_ENSURE(xQueries->hasByName(sTableName), "table not available anymore?");
                        try
                        {
                            Any aQuery = xQueries->getByName(sTableName);
                            Reference<XPropertySet> xPropSet;
                            aQuery >>= xPropSet;
                            xColsSupplier = Reference<XColumnsSupplier>(xPropSet, UNO_QUERY);
                        }
                        catch (const Exception&)
                        {
                        }
                    }
                }

                if(xColsSupplier.is())
                {
                    Reference <XNameAccess> xCols = xColsSupplier->getColumns();
                    Sequence< OUString> aColNames = xCols->getElementNames();
                    const OUString* pColNames = aColNames.getConstArray();
                    long nCount = aColNames.getLength();
                    for (long i = 0; i < nCount; i++)
                    {
                        String sName = pColNames[i];
                        InsertEntry(sName, pParent);
                    }
                }
            }
            catch (const Exception&)
            {
            }
        }
        else    // table names
        {
            try
            {
                String sSourceName = GetEntryText(pParent);
                if(!pImpl->GetContext()->hasByName(sSourceName))
                    return;
                Reference<XConnection> xConnection = pImpl->GetConnection(sSourceName);
                if (xConnection.is())
                {
                    Reference<XTablesSupplier> xTSupplier = Reference<XTablesSupplier>(xConnection, UNO_QUERY);
                    if(xTSupplier.is())
                    {
                        Reference<XNameAccess> xTbls = xTSupplier->getTables();
                        Sequence< OUString> aTblNames = xTbls->getElementNames();
                        String sTableName;
                        long nCount = aTblNames.getLength();
                        const OUString* pTblNames = aTblNames.getConstArray();
                        Image aImg = aImageList.GetImage(IMG_DBTABLE);
                        for (long i = 0; i < nCount; i++)
                        {
                            sTableName = pTblNames[i];
                            SvTreeListEntry* pTableEntry = InsertEntry(sTableName, aImg, aImg, pParent, bShowColumns);
                            //to discriminate between queries and tables the user data of table entries is set
                            pTableEntry->SetUserData((void*)0);
                        }
                    }

                    Reference<XQueriesSupplier> xQSupplier = Reference<XQueriesSupplier>(xConnection, UNO_QUERY);
                    if(xQSupplier.is())
                    {
                        Reference<XNameAccess> xQueries = xQSupplier->getQueries();
                        Sequence< OUString> aQueryNames = xQueries->getElementNames();
                        String sQueryName;
                        long nCount = aQueryNames.getLength();
                        const OUString* pQueryNames = aQueryNames.getConstArray();
                        Image aImg = aImageList.GetImage(IMG_DBQUERY);
                        for (long i = 0; i < nCount; i++)
                        {
                            sQueryName = pQueryNames[i];
                            SvTreeListEntry* pQueryEntry = InsertEntry(sQueryName, aImg, aImg, pParent, bShowColumns);
                            pQueryEntry->SetUserData((void*)1);
                        }
                    }
                }
            }
            catch (const Exception&)
            {
            }
        }
    }
}

IMPL_LINK( SwDBTreeList, DBCompare, SvSortData*, pData )
{
    SvTreeListEntry* pRight = (SvTreeListEntry*)(pData->pRight );

    if (GetParent(pRight) && GetParent(GetParent(pRight)))
        return COMPARE_GREATER; // don't sort column names

    return DefaultCompare(pData);   // otherwise call base class
}

String  SwDBTreeList::GetDBName(String& rTableName, String& rColumnName, sal_Bool* pbIsTable)
{
    String sDBName;
    SvTreeListEntry* pEntry = FirstSelected();

    if (pEntry && GetParent(pEntry))
    {
        if (GetParent(GetParent(pEntry)))
        {
            rColumnName = GetEntryText(pEntry);
            pEntry = GetParent(pEntry); // column name was selected
        }
        sDBName = GetEntryText(GetParent(pEntry));
        if(pbIsTable)
        {
            *pbIsTable = pEntry->GetUserData() == 0;
        }
        rTableName = GetEntryText(pEntry);
    }
    return sDBName;
}

/*------------------------------------------------------------------------
 Description:   Format: database.table
------------------------------------------------------------------------*/
void SwDBTreeList::Select(const String& rDBName, const String& rTableName, const String& rColumnName)
{
    SvTreeListEntry* pParent;
    SvTreeListEntry* pChild;
    sal_uInt16 nParent = 0;
    sal_uInt16 nChild = 0;

    while ((pParent = GetEntry(nParent++)) != NULL)
    {
        if (rDBName == GetEntryText(pParent))
        {
            if (!pParent->HasChildren())
                RequestingChildren(pParent);
            while ((pChild = GetEntry(pParent, nChild++)) != NULL)
            {
                if (rTableName == GetEntryText(pChild))
                {
                    pParent = pChild;

                    if (bShowColumns && rColumnName.Len())
                    {
                        nChild = 0;

                        if (!pParent->HasChildren())
                            RequestingChildren(pParent);

                        while ((pChild = GetEntry(pParent, nChild++)) != NULL)
                            if (rColumnName == GetEntryText(pChild))
                                break;
                    }
                    if (!pChild)
                        pChild = pParent;

                    MakeVisible(pChild);
                    SvTreeListBox::Select(pChild);
                    return;
                }
            }
        }
    }
}

void SwDBTreeList::StartDrag( sal_Int8 /*nAction*/, const Point& /*rPosPixel*/ )
{
    String sTableName, sColumnName;
    String  sDBName( GetDBName( sTableName, sColumnName ));
    if( sDBName.Len() )
    {
        TransferDataContainer* pContainer = new TransferDataContainer;
        ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable > xRef( pContainer );
        if( sColumnName.Len() )
        {
            // drag database field
            svx::OColumnTransferable aColTransfer(
                            sDBName
                            ,OUString()
                            , sdb::CommandType::TABLE
                            ,sTableName
                            , sColumnName
                            ,(CTF_FIELD_DESCRIPTOR |CTF_COLUMN_DESCRIPTOR ));
            aColTransfer.addDataToContainer( pContainer );
        }

        sDBName += '.';
        sDBName += sTableName;
        if( sColumnName.Len() )
        {
            sDBName += '.';
            sDBName += sColumnName;
        }

        pContainer->CopyString( FORMAT_STRING, sDBName );
        pContainer->StartDrag( this, DND_ACTION_COPY | DND_ACTION_LINK,
                                Link() );
    }
}

sal_Int8 SwDBTreeList::AcceptDrop( const AcceptDropEvent& /*rEvt*/ )
{
    return DND_ACTION_NONE;
}

void SwDBTreeList::SetWrtShell(SwWrtShell& rSh)
{
    pImpl->SetWrtShell(rSh);
    if (IsVisible() && !bInitialized)
        InitTreeList();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
