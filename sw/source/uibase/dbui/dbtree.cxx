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
#include <cppuhelper/implbase.hxx>
#include <svx/dbaexchange.hxx>

#include <dbmgr.hxx>
#include <swmodule.hxx>
#include <view.hxx>
#include <wrtsh.hxx>
#include <dbtree.hxx>
#include <osl/mutex.hxx>
#include <vcl/builderfactory.hxx>
#include <vcl/svapp.hxx>
#include <svtools/treelistentry.hxx>

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

class SwDBTreeList_Impl : public cppu::WeakImplHelper < XContainerListener >
{
    Reference< XDatabaseContext > m_xDatabaseContext;
    SwConnectionArr m_aConnections;
    SwWrtShell* m_pWrtShell;

    public:
        explicit SwDBTreeList_Impl(SwWrtShell* pShell)
            : m_pWrtShell(pShell)
        {
        }
        virtual ~SwDBTreeList_Impl();

    virtual void SAL_CALL elementInserted( const ContainerEvent& Event ) throw (RuntimeException, std::exception) override;
    virtual void SAL_CALL elementRemoved( const ContainerEvent& Event ) throw (RuntimeException, std::exception) override;
    virtual void SAL_CALL elementReplaced( const ContainerEvent& Event ) throw (RuntimeException, std::exception) override;
    virtual void SAL_CALL disposing( const EventObject& Source ) throw (RuntimeException, std::exception) override;

    bool                        HasContext();
    SwWrtShell*                 GetWrtShell() { return m_pWrtShell;}
    void                        SetWrtShell(SwWrtShell& rSh) { m_pWrtShell = &rSh;}
    Reference<XDatabaseContext>    GetContext() const {return m_xDatabaseContext;}
    Reference<XConnection>      GetConnection(const OUString& rSourceName);
};

SwDBTreeList_Impl::~SwDBTreeList_Impl()
{
    if(m_xDatabaseContext.is())
    {
        m_refCount++;
        //block necessary due to solaris' compiler behaviour to
        //remove temporaries at the block's end
        {
            m_xDatabaseContext->removeContainerListener( this );
        }
        m_refCount--;
    }
}

void SwDBTreeList_Impl::elementInserted( const ContainerEvent&  ) throw (RuntimeException, std::exception)
{
    // information not needed
}

void SwDBTreeList_Impl::elementRemoved( const ContainerEvent& rEvent ) throw (RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    OUString sSource;
    rEvent.Accessor >>= sSource;
    for(SwConnectionArr::iterator i = m_aConnections.begin(); i != m_aConnections.end(); ++i)
    {
        if(i->sSourceName == sSource)
        {
            m_aConnections.erase(i);
            break;
        }
    }
}

void SwDBTreeList_Impl::disposing( const EventObject&  ) throw (RuntimeException, std::exception)
{
    m_xDatabaseContext = 0;
}

void SwDBTreeList_Impl::elementReplaced( const ContainerEvent& rEvent ) throw (RuntimeException, std::exception)
{
    elementRemoved(rEvent);
}

bool SwDBTreeList_Impl::HasContext()
{
    if(!m_xDatabaseContext.is())
    {
        Reference< XComponentContext > xContext( ::comphelper::getProcessComponentContext() );
        m_xDatabaseContext = DatabaseContext::create(xContext);
        m_xDatabaseContext->addContainerListener( this );
    }
    return m_xDatabaseContext.is();
}

Reference<XConnection>  SwDBTreeList_Impl::GetConnection(const OUString& rSourceName)
{
    Reference<XConnection> xRet;
    for(SwConnectionArr::const_iterator i = m_aConnections.begin(); i != m_aConnections.end(); ++i)
    {
        if(i->sSourceName == rSourceName)
        {
            xRet = i->xConnection;
            break;
        }
    }
    if(!xRet.is() && m_xDatabaseContext.is() && m_pWrtShell)
    {
        SwConnectionData* pPtr = new SwConnectionData();
        pPtr->sSourceName = rSourceName;
        xRet = m_pWrtShell->GetDBManager()->RegisterConnection(pPtr->sSourceName);
        m_aConnections.push_back(pPtr);
    }
    return xRet;
}

SwDBTreeList::SwDBTreeList(vcl::Window *pParent, WinBits nStyle)
    : SvTreeListBox(pParent, nStyle)
    , aImageList(SW_RES(ILIST_DB_DLG))
    , bInitialized(false)
    , bShowColumns(false)
    , pImpl(new SwDBTreeList_Impl(NULL))
{
    if (IsVisible())
        InitTreeList();
}

VCL_BUILDER_DECL_FACTORY(SwDBTreeList)
{
    WinBits nStyle = WB_TABSTOP;
    OString sBorder = VclBuilder::extractCustomProperty(rMap);
    if (!sBorder.isEmpty())
        nStyle |= WB_BORDER;
    rRet = VclPtr<SwDBTreeList>::Create(pParent, nStyle);
}

Size SwDBTreeList::GetOptimalSize() const
{
    return LogicToPixel(Size(100, 62), MapMode(MAP_APPFONT));
}

SwDBTreeList::~SwDBTreeList()
{
    disposeOnce();
}

void SwDBTreeList::dispose()
{
    delete pImpl;
    pImpl = NULL;
    SvTreeListBox::dispose();
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

    SetDragDropMode(DragDropMode::APP_COPY);

    GetModel()->SetCompareHdl(LINK(this, SwDBTreeList, DBCompare));

    Sequence< OUString > aDBNames = pImpl->GetContext()->getElementNames();
    const OUString* pDBNames = aDBNames.getConstArray();
    long nCount = aDBNames.getLength();

    Image aImg = aImageList.GetImage(IMG_DB);
    for(long i = 0; i < nCount; i++)
    {
        OUString sDBName(pDBNames[i]);
        InsertEntry(sDBName, aImg, aImg, NULL, true);
    }
    OUString sDBName(sDefDBName.getToken(0, DB_DELIM));
    OUString sTableName(sDefDBName.getToken(1, DB_DELIM));
    OUString sColumnName(sDefDBName.getToken(2, DB_DELIM));
    Select(sDBName, sTableName, sColumnName);

    bInitialized = true;
}

void    SwDBTreeList::AddDataSource(const OUString& rSource)
{
    Image aImg = aImageList.GetImage(IMG_DB);
    SvTreeListEntry* pEntry = InsertEntry(rSource, aImg, aImg, NULL, true);
    SvTreeListBox::Select(pEntry);
}

void SwDBTreeList::ShowColumns(bool bShowCol)
{
    if (bShowCol != bShowColumns)
    {
        bShowColumns = bShowCol;
        OUString sTableName;
        OUString sColumnName;
        const OUString sDBName(GetDBName(sTableName, sColumnName));

        SetUpdateMode(false);

        SvTreeListEntry* pEntry = First();

        while (pEntry)
        {
            pEntry = GetRootLevelParent( pEntry );
            Collapse(pEntry);       // zuklappen

            SvTreeListEntry* pChild;
            while ((pChild = FirstChild(pEntry)) != 0L)
                GetModel()->Remove(pChild);

            pEntry = Next(pEntry);
        }

        if (!sDBName.isEmpty())
        {
            Select(sDBName, sTableName, sColumnName);   // force RequestingChildren
        }
        SetUpdateMode(true);
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

                OUString sSourceName = GetEntryText(GetParent(pParent));
                OUString sTableName = GetEntryText(pParent);

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
                        Reference<XNameAccess> xTables = xTSupplier->getTables();
                        OSL_ENSURE(xTables->hasByName(sTableName), "table not available anymore?");
                        try
                        {
                            Any aTable = xTables->getByName(sTableName);
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
                        OUString sName = pColNames[i];
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
                OUString sSourceName = GetEntryText(pParent);
                if(!pImpl->GetContext()->hasByName(sSourceName))
                    return;
                Reference<XConnection> xConnection = pImpl->GetConnection(sSourceName);
                if (xConnection.is())
                {
                    Reference<XTablesSupplier> xTSupplier = Reference<XTablesSupplier>(xConnection, UNO_QUERY);
                    if(xTSupplier.is())
                    {
                        Reference<XNameAccess> xTables = xTSupplier->getTables();
                        Sequence< OUString> aTableNames = xTables->getElementNames();
                        OUString sTableName;
                        long nCount = aTableNames.getLength();
                        const OUString* pTableNames = aTableNames.getConstArray();
                        Image aImg = aImageList.GetImage(IMG_DBTABLE);
                        for (long i = 0; i < nCount; i++)
                        {
                            sTableName = pTableNames[i];
                            SvTreeListEntry* pTableEntry = InsertEntry(sTableName, aImg, aImg, pParent, bShowColumns);
                            //to discriminate between queries and tables the user data of table entries is set
                            pTableEntry->SetUserData(nullptr);
                        }
                    }

                    Reference<XQueriesSupplier> xQSupplier = Reference<XQueriesSupplier>(xConnection, UNO_QUERY);
                    if(xQSupplier.is())
                    {
                        Reference<XNameAccess> xQueries = xQSupplier->getQueries();
                        Sequence< OUString> aQueryNames = xQueries->getElementNames();
                        OUString sQueryName;
                        long nCount = aQueryNames.getLength();
                        const OUString* pQueryNames = aQueryNames.getConstArray();
                        Image aImg = aImageList.GetImage(IMG_DBQUERY);
                        for (long i = 0; i < nCount; i++)
                        {
                            sQueryName = pQueryNames[i];
                            SvTreeListEntry* pQueryEntry = InsertEntry(sQueryName, aImg, aImg, pParent, bShowColumns);
                            pQueryEntry->SetUserData(reinterpret_cast<void*>(1));
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

IMPL_LINK_TYPED( SwDBTreeList, DBCompare, const SvSortData&, rData, sal_Int32 )
{
    SvTreeListEntry* pRight = const_cast<SvTreeListEntry*>(rData.pRight);

    if (GetParent(pRight) && GetParent(GetParent(pRight)))
        return 1; // don't sort column names

    return DefaultCompare(rData);   // otherwise call base class
}

OUString SwDBTreeList::GetDBName(OUString& rTableName, OUString& rColumnName, sal_Bool* pbIsTable)
{
    OUString sDBName;
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

// Format: database.table
void SwDBTreeList::Select(const OUString& rDBName, const OUString& rTableName, const OUString& rColumnName)
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

                    if (bShowColumns && !rColumnName.isEmpty())
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
    OUString sTableName;
    OUString sColumnName;
    OUString sDBName( GetDBName( sTableName, sColumnName ));
    if (!sDBName.isEmpty())
    {
        TransferDataContainer* pContainer = new TransferDataContainer;
        ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable > xRef( pContainer );
        if( !sColumnName.isEmpty() )
        {
            // drag database field
            uno::Reference< svx::OColumnTransferable > xColTransfer( new svx::OColumnTransferable(
                            sDBName,
                            OUString(),
                            sdb::CommandType::TABLE,
                            sTableName,
                            sColumnName,
                            (ColumnTransferFormatFlags::FIELD_DESCRIPTOR|ColumnTransferFormatFlags::COLUMN_DESCRIPTOR) ) );
            xColTransfer->addDataToContainer( pContainer );
        }

        sDBName += "." + sTableName;
        if (!sColumnName.isEmpty())
        {
            sDBName += "." + sColumnName;
        }

        pContainer->CopyString( SotClipboardFormatId::STRING, sDBName );
        pContainer->StartDrag( this, DND_ACTION_COPY | DND_ACTION_LINK,
                                Link<sal_Int8,void>() );
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
