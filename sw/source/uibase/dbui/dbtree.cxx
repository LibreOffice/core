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
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/sdb/DatabaseContext.hpp>
#include <com/sun/star/sdb/XQueriesSupplier.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>
#include <com/sun/star/container/XContainerListener.hpp>
#include <com/sun/star/container/XContainer.hpp>
#include <cppuhelper/implbase.hxx>
#include <svx/dbaexchange.hxx>

#include <dbmgr.hxx>
#include <swmodule.hxx>
#include <view.hxx>
#include <wrtsh.hxx>
#include <dbtree.hxx>
#include <vcl/builderfactory.hxx>
#include <vcl/svapp.hxx>
#include <vcl/treelistentry.hxx>

#include <bitmaps.hlst>

#include <unomid.h>


using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::task;
using namespace ::com::sun::star::beans;

class SwDBTreeList_Impl : public cppu::WeakImplHelper < XContainerListener >
{
    Reference< XDatabaseContext > m_xDatabaseContext;
    SwWrtShell* m_pWrtShell;

    public:
        explicit SwDBTreeList_Impl()
            : m_pWrtShell(nullptr)
        {
        }
        virtual ~SwDBTreeList_Impl() override;

    virtual void SAL_CALL elementInserted( const ContainerEvent& Event ) override;
    virtual void SAL_CALL elementRemoved( const ContainerEvent& Event ) override;
    virtual void SAL_CALL elementReplaced( const ContainerEvent& Event ) override;
    virtual void SAL_CALL disposing( const EventObject& Source ) override;

    bool                        HasContext();
    SwWrtShell*                 GetWrtShell() { return m_pWrtShell;}
    void                        SetWrtShell(SwWrtShell& rSh) { m_pWrtShell = &rSh;}
    const Reference<XDatabaseContext>& GetContext() const {return m_xDatabaseContext;}
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

void SwDBTreeList_Impl::elementInserted( const ContainerEvent&  )
{
    // information not needed
}

void SwDBTreeList_Impl::elementRemoved( const ContainerEvent& )
{
}

void SwDBTreeList_Impl::disposing( const EventObject&  )
{
    m_xDatabaseContext = nullptr;
}

void SwDBTreeList_Impl::elementReplaced( const ContainerEvent& rEvent )
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
    if (m_xDatabaseContext.is() && m_pWrtShell)
    {
        xRet = m_pWrtShell->GetDBManager()->RegisterConnection(rSourceName);
    }
    return xRet;
}

SwDBTreeList::SwDBTreeList(std::unique_ptr<weld::TreeView> xTreeView)
    : bInitialized(false)
    , bShowColumns(false)
    , pImpl(new SwDBTreeList_Impl)
    , m_xTreeView(std::move(xTreeView))
{
    m_xTreeView->connect_expanding(LINK(this, SwDBTreeList, RequestingChildrenHdl));
}

SwDBTreeList::~SwDBTreeList()
{
}

void SwDBTreeList::InitTreeList()
{
    if (!pImpl->HasContext() && pImpl->GetWrtShell())
        return;

    Sequence< OUString > aDBNames = pImpl->GetContext()->getElementNames();
    auto const sort = comphelper::string::NaturalStringSorter(
        comphelper::getProcessComponentContext(),
        Application::GetSettings().GetUILanguageTag().getLocale());
    std::sort(
        aDBNames.begin(), aDBNames.end(),
        [&sort](OUString const & x, OUString const & y)
        { return sort.compare(x, y) < 0; });
    const OUString* pDBNames = aDBNames.getConstArray();
    sal_Int32 nCount = aDBNames.getLength();

    OUString aImg(RID_BMP_DB);
    for (sal_Int32 i = 0; i < nCount; ++i)
    {
        OUString sDBName(pDBNames[i]);
        Reference<XConnection> xConnection = pImpl->GetConnection(sDBName);
        if (xConnection.is())
        {
            m_xTreeView->insert(nullptr, -1, &sDBName, nullptr, nullptr, nullptr, &aImg, true, nullptr);
        }
    }
    Select(OUString(), OUString(), OUString());

    bInitialized = true;
}

void SwDBTreeList::AddDataSource(const OUString& rSource)
{
    OUString aImg(RID_BMP_DB);
    std::unique_ptr<weld::TreeIter> xIter(m_xTreeView->make_iterator());
    m_xTreeView->insert(nullptr, -1, &rSource, nullptr, nullptr, nullptr, &aImg, true, xIter.get());
    m_xTreeView->select(*xIter);
}

IMPL_LINK(SwDBTreeList, RequestingChildrenHdl, const weld::TreeIter&, rParent, bool)
{
    if (!m_xTreeView->iter_has_child(rParent))
    {
        if (m_xTreeView->get_iter_depth(rParent)) // column names
        {
            try
            {
                std::unique_ptr<weld::TreeIter> xGrandParent(m_xTreeView->make_iterator(&rParent));
                m_xTreeView->iter_parent(*xGrandParent);
                OUString sSourceName = m_xTreeView->get_text(*xGrandParent);
                OUString sTableName = m_xTreeView->get_text(rParent);

                if(!pImpl->GetContext()->hasByName(sSourceName))
                    return true;
                Reference<XConnection> xConnection = pImpl->GetConnection(sSourceName);
                bool bTable = m_xTreeView->get_id(rParent).isEmpty();
                Reference<XColumnsSupplier> xColsSupplier;
                if(bTable)
                {
                    Reference<XTablesSupplier> xTSupplier(xConnection, UNO_QUERY);
                    if(xTSupplier.is())
                    {
                        Reference<XNameAccess> xTables = xTSupplier->getTables();
                        OSL_ENSURE(xTables->hasByName(sTableName), "table not available anymore?");
                        try
                        {
                            Any aTable = xTables->getByName(sTableName);
                            Reference<XPropertySet> xPropSet;
                            aTable >>= xPropSet;
                            xColsSupplier.set(xPropSet, UNO_QUERY);
                        }
                        catch (const Exception&)
                        {
                        }
                    }
                }
                else
                {
                    Reference<XQueriesSupplier> xQSupplier(xConnection, UNO_QUERY);
                    if(xQSupplier.is())
                    {
                        Reference<XNameAccess> xQueries = xQSupplier->getQueries();
                        OSL_ENSURE(xQueries->hasByName(sTableName), "table not available anymore?");
                        try
                        {
                            Any aQuery = xQueries->getByName(sTableName);
                            Reference<XPropertySet> xPropSet;
                            aQuery >>= xPropSet;
                            xColsSupplier.set(xPropSet, UNO_QUERY);
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
                        m_xTreeView->append(&rParent, sName);
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
                OUString sSourceName = m_xTreeView->get_text(rParent);
                if (!pImpl->GetContext()->hasByName(sSourceName))
                    return true;
                Reference<XConnection> xConnection = pImpl->GetConnection(sSourceName);
                if (xConnection.is())
                {
                    Reference<XTablesSupplier> xTSupplier(xConnection, UNO_QUERY);
                    if(xTSupplier.is())
                    {
                        Reference<XNameAccess> xTables = xTSupplier->getTables();
                        Sequence< OUString> aTableNames = xTables->getElementNames();
                        OUString sTableName;
                        long nCount = aTableNames.getLength();
                        const OUString* pTableNames = aTableNames.getConstArray();
                        OUString aImg(RID_BMP_DBTABLE);
                        for (long i = 0; i < nCount; i++)
                        {
                            sTableName = pTableNames[i];
                            m_xTreeView->insert(&rParent, -1, &sTableName, nullptr,
                                                nullptr, nullptr, &aImg, bShowColumns, nullptr);
                        }
                    }

                    Reference<XQueriesSupplier> xQSupplier(xConnection, UNO_QUERY);
                    if(xQSupplier.is())
                    {
                        Reference<XNameAccess> xQueries = xQSupplier->getQueries();
                        Sequence< OUString> aQueryNames = xQueries->getElementNames();
                        OUString sQueryName;
                        long nCount = aQueryNames.getLength();
                        const OUString* pQueryNames = aQueryNames.getConstArray();
                        OUString aImg(RID_BMP_DBQUERY);
                        for (long i = 0; i < nCount; i++)
                        {
                            sQueryName = pQueryNames[i];
                            //to discriminate between queries and tables the user data of query entries is set
                            OUString sId(OUString::number(1));
                            m_xTreeView->insert(&rParent, -1, &sQueryName, &sId,
                                                nullptr, nullptr, &aImg, bShowColumns, nullptr);
                        }
                    }
                }
            }
            catch (const Exception&)
            {
            }
        }
    }
    return true;
}

OUString SwDBTreeList::GetDBName(OUString& rTableName, OUString& rColumnName, sal_Bool* pbIsTable)
{
    OUString sDBName;
    std::unique_ptr<weld::TreeIter> xIter(m_xTreeView->make_iterator());
    if (m_xTreeView->get_selected(xIter.get()) && m_xTreeView->get_iter_depth(*xIter))
    {
        if (m_xTreeView->get_iter_depth(*xIter) > 1)
        {
            rColumnName = m_xTreeView->get_text(*xIter);
            m_xTreeView->iter_parent(*xIter); // column name was selected
        }
        if (pbIsTable)
        {
            *pbIsTable = m_xTreeView->get_id(*xIter).isEmpty();
        }
        rTableName = m_xTreeView->get_text(*xIter);
        m_xTreeView->iter_parent(*xIter);
        sDBName = m_xTreeView->get_text(*xIter);
    }
    return sDBName;
}

// Format: database.table
void SwDBTreeList::Select(const OUString& rDBName, const OUString& rTableName, const OUString& rColumnName)
{
    std::unique_ptr<weld::TreeIter> xParent(m_xTreeView->make_iterator());
    if (!m_xTreeView->get_iter_first(*xParent))
        return;

    do
    {
        if (rDBName == m_xTreeView->get_text(*xParent))
        {
            if (!m_xTreeView->iter_has_child(*xParent))
                m_xTreeView->expand_row(*xParent);
            std::unique_ptr<weld::TreeIter> xChild(m_xTreeView->make_iterator(xParent.get()));
            if (!m_xTreeView->iter_children(*xChild))
                continue;
            do
            {
                if (rTableName == m_xTreeView->get_text(*xChild))
                {
                    m_xTreeView->copy_iterator(*xChild, *xParent);

                    bool bNoChild = false;
                    if (bShowColumns && !rColumnName.isEmpty())
                    {
                        if (!m_xTreeView->iter_has_child(*xParent))
                            m_xTreeView->expand_row(*xParent);

                        bNoChild = true;
                        if (m_xTreeView->iter_children(*xChild))
                        {
                            do
                            {
                                if (rColumnName == m_xTreeView->get_text(*xChild))
                                {
                                    bNoChild = false;
                                    break;
                                }
                            }
                            while (m_xTreeView->iter_next_sibling(*xChild));
                        }
                    }

                    if (bNoChild)
                        m_xTreeView->copy_iterator(*xParent, *xChild);

                    m_xTreeView->scroll_to_row(*xChild);
                    m_xTreeView->select(*xChild);
                    return;
                }
            }
            while (m_xTreeView->iter_next_sibling(*xChild));
        }
    } while (m_xTreeView->iter_next_sibling(*xParent));
}

void SwDBTreeList::SetWrtShell(SwWrtShell& rSh)
{
    pImpl->SetWrtShell(rSh);
    if (m_xTreeView->get_visible() && !bInitialized)
        InitTreeList();
}

namespace
{
    void GotoRootLevelParent(weld::TreeView& rTreeView, weld::TreeIter& rEntry)
    {
        while (rTreeView.get_iter_depth(rEntry))
            rTreeView.iter_parent(rEntry);
    }
}

void SwDBTreeList::ShowColumns(bool bShowCol)
{
    if (bShowCol != bShowColumns)
    {
        bShowColumns = bShowCol;
        OUString sTableName;
        OUString sColumnName;
        const OUString sDBName(GetDBName(sTableName, sColumnName));

        m_xTreeView->freeze();

        std::unique_ptr<weld::TreeIter> xIter(m_xTreeView->make_iterator());
        std::unique_ptr<weld::TreeIter> xChild(m_xTreeView->make_iterator());
        if (m_xTreeView->get_iter_first(*xIter))
        {
            do
            {
                GotoRootLevelParent(*m_xTreeView, *xIter);
                m_xTreeView->collapse_row(*xIter);
                while (m_xTreeView->iter_has_child(*xIter))
                {
                    m_xTreeView->copy_iterator(*xIter, *xChild);
                    m_xTreeView->iter_children(*xChild);
                    m_xTreeView->remove(*xChild);
                }
            } while (m_xTreeView->iter_next(*xIter));
        }

        m_xTreeView->thaw();

        if (!sDBName.isEmpty())
        {
            Select(sDBName, sTableName, sColumnName);   // force RequestingChildren
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
