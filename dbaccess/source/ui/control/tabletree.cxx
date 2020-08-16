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

#include <core_resource.hxx>
#include <tabletree.hxx>
#include <imageprovider.hxx>
#include <strings.hrc>
#include <connectivity/dbtools.hxx>
#include <com/sun/star/sdb/application/DatabaseObject.hpp>
#include <com/sun/star/sdb/application/DatabaseObjectContainer.hpp>
#include <com/sun/star/sdbcx/XViewsSupplier.hpp>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <com/sun/star/sdbc/SQLException.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <tools/diagnose_ex.h>
#include <osl/diagnose.h>
#include <connectivity/dbmetadata.hxx>
#include <vcl/event.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>

#include <algorithm>

namespace dbaui
{

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::sdb::application;

using namespace ::dbtools;
using namespace ::comphelper;

namespace DatabaseObject = ::com::sun::star::sdb::application::DatabaseObject;
namespace DatabaseObjectContainer = ::com::sun::star::sdb::application::DatabaseObjectContainer;

// OTableTreeListBox
OTableTreeListBox::OTableTreeListBox(vcl::Window* pParent, bool bShowToggles)
    : InterimDBTreeListBox(pParent)
    , m_xImageProvider(new ImageProvider)
    , m_bVirtualRoot(false)
    , m_bNoEmptyFolders(false)
    , m_bShowToggles(bShowToggles)
{
    if (m_bShowToggles)
        m_xTreeView->enable_toggle_buttons(weld::ColumnToggleType::Check);
}

OTableTreeListBox::~OTableTreeListBox()
{
}

TableTreeListBox::TableTreeListBox(std::unique_ptr<weld::TreeView> xTreeView, bool bShowToggles)
    : TreeListBox(std::move(xTreeView))
    , m_xImageProvider(new ImageProvider)
    , m_bVirtualRoot(false)
    , m_bNoEmptyFolders(false)
    , m_bShowToggles(bShowToggles)
{
    if (m_bShowToggles)
        m_xTreeView->enable_toggle_buttons(weld::ColumnToggleType::Check);
}

bool OTableTreeListBox::isFolderEntry(const weld::TreeIter& rEntry) const
{
    sal_Int32 nEntryType = m_xTreeView->get_id(rEntry).toInt32();
    return ( nEntryType == DatabaseObjectContainer::TABLES )
        ||  ( nEntryType == DatabaseObjectContainer::CATALOG )
        ||  ( nEntryType == DatabaseObjectContainer::SCHEMA );
}

void OTableTreeListBox::implOnNewConnection( const Reference< XConnection >& _rxConnection )
{
    m_xConnection = _rxConnection;
    m_xImageProvider.reset( new ImageProvider( m_xConnection  ) );
}

void TableTreeListBox::implOnNewConnection( const Reference< XConnection >& _rxConnection )
{
    m_xConnection = _rxConnection;
    m_xImageProvider.reset( new ImageProvider( m_xConnection  ) );
}

void OTableTreeListBox::UpdateTableList(const Reference<XConnection>& _rxConnection)
{
    Sequence< OUString > sTables, sViews;

    OUString sCurrentActionError;
    try
    {
        Reference< XTablesSupplier > xTableSupp( _rxConnection, UNO_QUERY_THROW );
        sCurrentActionError = DBA_RES(STR_NOTABLEINFO);

        Reference< XNameAccess > xTables,xViews;

        Reference< XViewsSupplier > xViewSupp( _rxConnection, UNO_QUERY );
        if ( xViewSupp.is() )
        {
            xViews = xViewSupp->getViews();
            if (xViews.is())
                sViews = xViews->getElementNames();
        }

        xTables = xTableSupp->getTables();
        if (xTables.is())
            sTables = xTables->getElementNames();
    }
    catch(RuntimeException&)
    {
        OSL_FAIL("OTableTreeListBox::UpdateTableList : caught a RuntimeException!");
    }
    catch ( const SQLException& )
    {
        throw;
    }
    catch(Exception&)
    {
        css::uno::Any anyEx = cppu::getCaughtException();
        // a non-SQLException exception occurred ... simply throw an SQLException
        throw SQLException(sCurrentActionError, nullptr, "", 0, anyEx);
    }

    UpdateTableList( _rxConnection, sTables, sViews );
}

void TableTreeListBox::UpdateTableList( const Reference< XConnection >& _rxConnection )
{
    Sequence< OUString > sTables, sViews;

    OUString sCurrentActionError;
    try
    {
        Reference< XTablesSupplier > xTableSupp( _rxConnection, UNO_QUERY_THROW );
        sCurrentActionError = DBA_RES(STR_NOTABLEINFO);

        Reference< XNameAccess > xTables,xViews;

        Reference< XViewsSupplier > xViewSupp( _rxConnection, UNO_QUERY );
        if ( xViewSupp.is() )
        {
            xViews = xViewSupp->getViews();
            if (xViews.is())
                sViews = xViews->getElementNames();
        }

        xTables = xTableSupp->getTables();
        if (xTables.is())
            sTables = xTables->getElementNames();
    }
    catch(RuntimeException&)
    {
        OSL_FAIL("OTableTreeListBox::UpdateTableList : caught a RuntimeException!");
    }
    catch ( const SQLException& )
    {
        throw;
    }
    catch(Exception&)
    {
        css::uno::Any anyEx = cppu::getCaughtException();
        // a non-SQLException exception occurred ... simply throw an SQLException
        throw SQLException(sCurrentActionError, nullptr, "", 0, anyEx);
    }

    UpdateTableList( _rxConnection, sTables, sViews );
}

namespace
{
    struct OViewSetter
    {
        const Sequence< OUString> m_aViews;
        ::comphelper::UStringMixEqual m_aEqualFunctor;

        OViewSetter(const Sequence< OUString>& _rViews,bool _bCase) : m_aViews(_rViews),m_aEqualFunctor(_bCase){}
        OTableTreeListBox::TNames::value_type operator() (const OUString& name)
        {
            OTableTreeListBox::TNames::value_type aRet;
            aRet.first = name;
            aRet.second = std::any_of(m_aViews.begin(), m_aViews.end(),
                                      [this, &name](const OUString& lhs)
                                      { return m_aEqualFunctor(lhs, name); } );

            return aRet;
        }
    };

}

void OTableTreeListBox::UpdateTableList(
                const Reference< XConnection >& _rxConnection,
                const Sequence< OUString>& _rTables,
                const Sequence< OUString>& _rViews
            )
{
    TNames aTables;
    aTables.resize(_rTables.getLength());
    try
    {
        Reference< XDatabaseMetaData > xMeta( _rxConnection->getMetaData(), UNO_SET_THROW );
        std::transform( _rTables.begin(), _rTables.end(),
            aTables.begin(), OViewSetter( _rViews, xMeta->supportsMixedCaseQuotedIdentifiers() ) );
    }
    catch(Exception&)
    {
        DBG_UNHANDLED_EXCEPTION("dbaccess");
    }
    UpdateTableList( _rxConnection, aTables );
}

void TableTreeListBox::UpdateTableList(
                const Reference< XConnection >& _rxConnection,
                const Sequence< OUString>& _rTables,
                const Sequence< OUString>& _rViews
            )
{
    TNames aTables;
    aTables.resize(_rTables.getLength());
    try
    {
        Reference< XDatabaseMetaData > xMeta( _rxConnection->getMetaData(), UNO_SET_THROW );
        std::transform( _rTables.begin(), _rTables.end(),
            aTables.begin(), OViewSetter( _rViews, xMeta->supportsMixedCaseQuotedIdentifiers() ) );
    }
    catch(Exception&)
    {
        DBG_UNHANDLED_EXCEPTION("dbaccess");
    }
    UpdateTableList( _rxConnection, aTables );
}

namespace
{
    std::vector< OUString > lcl_getMetaDataStrings_throw( const Reference< XResultSet >& _rxMetaDataResult, sal_Int32 _nColumnIndex )
    {
        std::vector< OUString > aStrings;
        Reference< XRow > xRow( _rxMetaDataResult, UNO_QUERY_THROW );
        while ( _rxMetaDataResult->next() )
            aStrings.push_back( xRow->getString( _nColumnIndex ) );
        return aStrings;
    }

    bool lcl_shouldDisplayEmptySchemasAndCatalogs( const Reference< XConnection >& _rxConnection )
    {
        ::dbtools::DatabaseMetaData aMetaData( _rxConnection );
        return aMetaData.displayEmptyTableFolders();
    }
}

void OTableTreeListBox::UpdateTableList( const Reference< XConnection >& _rxConnection, const TNames& _rTables )
{
    implOnNewConnection( _rxConnection );

    // throw away all the old stuff
    m_xTreeView->clear();
    m_xTreeView->make_unsorted();

    try
    {
        if (haveVirtualRoot())
        {
            OUString sRootEntryText;
            if ( std::none_of(_rTables.begin(),_rTables.end(),
                                [] (const TNames::value_type& name) { return !name.second; }) )
                sRootEntryText  = DBA_RES(STR_ALL_TABLES);
            else if ( std::none_of(_rTables.begin(),_rTables.end(),
                                     [] (const TNames::value_type& name) { return name.second; }) )
                sRootEntryText  = DBA_RES(STR_ALL_VIEWS);
            else
                sRootEntryText  = DBA_RES(STR_ALL_TABLES_AND_VIEWS);
            OUString sId(OUString::number(DatabaseObjectContainer::TABLES));
            OUString sImageId = ImageProvider::getFolderImageId(DatabaseObject::TABLE);
            std::unique_ptr<weld::TreeIter> xRet(m_xTreeView->make_iterator());
            m_xTreeView->insert(nullptr, -1, nullptr, &sId, nullptr, nullptr, false, xRet.get());
            m_xTreeView->set_image(*xRet, sImageId, -1);
            if (m_bShowToggles)
                m_xTreeView->set_toggle(*xRet, TRISTATE_FALSE);
            m_xTreeView->set_text(*xRet, sRootEntryText, 0);
            m_xTreeView->set_text_emphasis(*xRet, false, 0);
        }

        if ( _rTables.empty() )
            // nothing to do (besides inserting the root entry)
            return;

        // get the table/view names
        Reference< XDatabaseMetaData > xMeta( _rxConnection->getMetaData(), UNO_SET_THROW );
        for (auto const& table : _rTables)
        {
            // add the entry
            implAddEntry(xMeta, table.first, false);
        }

        if ( !m_bNoEmptyFolders && lcl_shouldDisplayEmptySchemasAndCatalogs( _rxConnection ) )
        {
            bool bSupportsCatalogs = xMeta->supportsCatalogsInDataManipulation();
            bool bSupportsSchemas = xMeta->supportsSchemasInDataManipulation();

            if ( bSupportsCatalogs || bSupportsSchemas )
            {
                // we display empty catalogs if the DB supports catalogs, and they're noted at the beginning of a
                // composed name. Otherwise, we display empty schematas. (also see the tree structure explained in
                // implAddEntry)
                bool bCatalogs = bSupportsCatalogs && xMeta->isCatalogAtStart();

                std::vector< OUString > aFolderNames( lcl_getMetaDataStrings_throw(
                    bCatalogs ? xMeta->getCatalogs() : xMeta->getSchemas(), 1 ) );
                sal_Int32 nFolderType = bCatalogs ? DatabaseObjectContainer::CATALOG : DatabaseObjectContainer::SCHEMA;

                OUString sImageId = ImageProvider::getFolderImageId(DatabaseObject::TABLE);

                std::unique_ptr<weld::TreeIter> xRootEntry(getAllObjectsEntry());
                std::unique_ptr<weld::TreeIter> xRet(m_xTreeView->make_iterator());
                for (auto const& folderName : aFolderNames)
                {
                    std::unique_ptr<weld::TreeIter> xFolder(GetEntryPosByName(folderName, xRootEntry.get()));
                    if (!xFolder)
                    {
                        OUString sId(OUString::number(nFolderType));
                        m_xTreeView->insert(xRootEntry.get(), -1, nullptr, &sId, nullptr, nullptr, false, xRet.get());
                        m_xTreeView->set_image(*xRet, sImageId, -1);
                        if (m_bShowToggles)
                            m_xTreeView->set_toggle(*xRet, TRISTATE_FALSE);
                        m_xTreeView->set_text(*xRet, folderName, 0);
                        m_xTreeView->set_text_emphasis(*xRet, false, 0);
                    }
                }
            }
        }
    }
    catch ( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("dbaccess");
    }

    m_xTreeView->make_sorted();
}

void TableTreeListBox::UpdateTableList( const Reference< XConnection >& _rxConnection, const TNames& _rTables )
{
    implOnNewConnection( _rxConnection );

    // throw away all the old stuff
    m_xTreeView->clear();
    m_xTreeView->make_unsorted();

    try
    {
        if (haveVirtualRoot())
        {
            OUString sRootEntryText;
            if ( std::none_of(_rTables.begin(),_rTables.end(),
                                [] (const TNames::value_type& name) { return !name.second; }) )
                sRootEntryText  = DBA_RES(STR_ALL_TABLES);
            else if ( std::none_of(_rTables.begin(),_rTables.end(),
                                     [] (const TNames::value_type& name) { return name.second; }) )
                sRootEntryText  = DBA_RES(STR_ALL_VIEWS);
            else
                sRootEntryText  = DBA_RES(STR_ALL_TABLES_AND_VIEWS);
            OUString sId(OUString::number(DatabaseObjectContainer::TABLES));
            OUString sImageId = ImageProvider::getFolderImageId(DatabaseObject::TABLE);
            std::unique_ptr<weld::TreeIter> xRet(m_xTreeView->make_iterator());
            m_xTreeView->insert(nullptr, -1, nullptr, &sId, nullptr, nullptr, false, xRet.get());
            m_xTreeView->set_image(*xRet, sImageId, -1);
            if (m_bShowToggles)
                m_xTreeView->set_toggle(*xRet, TRISTATE_FALSE);
            m_xTreeView->set_text(*xRet, sRootEntryText, 0);
            m_xTreeView->set_text_emphasis(*xRet, false, 0);
        }

        if ( _rTables.empty() )
            // nothing to do (besides inserting the root entry)
            return;

        // get the table/view names
        Reference< XDatabaseMetaData > xMeta( _rxConnection->getMetaData(), UNO_SET_THROW );
        for (auto const& table : _rTables)
        {
            // add the entry
            implAddEntry(xMeta, table.first, false);
        }

        if ( !m_bNoEmptyFolders && lcl_shouldDisplayEmptySchemasAndCatalogs( _rxConnection ) )
        {
            bool bSupportsCatalogs = xMeta->supportsCatalogsInDataManipulation();
            bool bSupportsSchemas = xMeta->supportsSchemasInDataManipulation();

            if ( bSupportsCatalogs || bSupportsSchemas )
            {
                // we display empty catalogs if the DB supports catalogs, and they're noted at the beginning of a
                // composed name. Otherwise, we display empty schematas. (also see the tree structure explained in
                // implAddEntry)
                bool bCatalogs = bSupportsCatalogs && xMeta->isCatalogAtStart();

                std::vector< OUString > aFolderNames( lcl_getMetaDataStrings_throw(
                    bCatalogs ? xMeta->getCatalogs() : xMeta->getSchemas(), 1 ) );
                sal_Int32 nFolderType = bCatalogs ? DatabaseObjectContainer::CATALOG : DatabaseObjectContainer::SCHEMA;

                OUString sImageId = ImageProvider::getFolderImageId(DatabaseObject::TABLE);

                std::unique_ptr<weld::TreeIter> xRootEntry(getAllObjectsEntry());
                std::unique_ptr<weld::TreeIter> xRet(m_xTreeView->make_iterator());
                for (auto const& folderName : aFolderNames)
                {
                    std::unique_ptr<weld::TreeIter> xFolder(GetEntryPosByName(folderName, xRootEntry.get()));
                    if (!xFolder)
                    {
                        OUString sId(OUString::number(nFolderType));
                        m_xTreeView->insert(xRootEntry.get(), -1, nullptr, &sId, nullptr, nullptr, false, xRet.get());
                        m_xTreeView->set_image(*xRet, sImageId, -1);
                        if (m_bShowToggles)
                            m_xTreeView->set_toggle(*xRet, TRISTATE_FALSE);
                        m_xTreeView->set_text(*xRet, folderName, 0);
                        m_xTreeView->set_text_emphasis(*xRet, false, 0);
                    }
                }
            }
        }
    }
    catch ( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("dbaccess");
    }

    m_xTreeView->make_sorted();
}

bool TableTreeListBox::isWildcardChecked(const weld::TreeIter& rEntry)
{
    return m_xTreeView->get_text_emphasis(rEntry, 0);
}

void TableTreeListBox::checkWildcard(weld::TreeIter& rEntry)
{
    if (!m_bShowToggles)
        return;
    m_xTreeView->set_toggle(rEntry, TRISTATE_TRUE);
    checkedButton_noBroadcast(rEntry);
}

std::unique_ptr<weld::TreeIter> OTableTreeListBox::getAllObjectsEntry() const
{
    if (!haveVirtualRoot())
        return nullptr;
    auto xRet = m_xTreeView->make_iterator();
    if (!m_xTreeView->get_iter_first(*xRet))
        return nullptr;
    return xRet;
}

std::unique_ptr<weld::TreeIter> TableTreeListBox::getAllObjectsEntry() const
{
    if (!haveVirtualRoot())
        return nullptr;
    auto xRet = m_xTreeView->make_iterator();
    if (!m_xTreeView->get_iter_first(*xRet))
        return nullptr;
    return xRet;
}

void OTableTreeListBox::checkedButton_noBroadcast(const weld::TreeIter& rEntry)
{
    if (!m_bShowToggles)
        return;
    TriState eState = m_xTreeView->get_toggle(rEntry);
    OSL_ENSURE(TRISTATE_INDET != eState, "OTableTreeListBox::CheckButtonHdl: user action which lead to TRISTATE?");

    if (m_xTreeView->iter_has_child(rEntry)) // if it has children, check those too
    {
        std::unique_ptr<weld::TreeIter> xChildEntry(m_xTreeView->make_iterator(&rEntry));
        std::unique_ptr<weld::TreeIter> xSiblingEntry(m_xTreeView->make_iterator(&rEntry));
        bool bChildEntry = m_xTreeView->iter_next(*xChildEntry);
        bool bSiblingEntry = m_xTreeView->iter_next_sibling(*xSiblingEntry);
        while (bChildEntry && (!bSiblingEntry || !xChildEntry->equal(*xSiblingEntry)))
        {
            m_xTreeView->set_toggle(*xChildEntry, eState);
            bChildEntry = m_xTreeView->iter_next(*xChildEntry);
        }
    }

    if (m_xTreeView->is_selected(rEntry))
    {
        m_xTreeView->selected_foreach([this, eState](weld::TreeIter& rSelected){
            m_xTreeView->set_toggle(rSelected, eState);
            if (m_xTreeView->iter_has_child(rSelected)) // if it has children, check those too
            {
                std::unique_ptr<weld::TreeIter> xChildEntry(m_xTreeView->make_iterator(&rSelected));
                std::unique_ptr<weld::TreeIter> xSiblingEntry(m_xTreeView->make_iterator(&rSelected));
                bool bChildEntry = m_xTreeView->iter_next(*xChildEntry);
                bool bSiblingEntry = m_xTreeView->iter_next_sibling(*xSiblingEntry);
                while (bChildEntry && (!bSiblingEntry || !xChildEntry->equal(*xSiblingEntry)))
                {
                    m_xTreeView->set_toggle(*xChildEntry, eState);
                    bChildEntry = m_xTreeView->iter_next(*xChildEntry);
                }
            }
            return false;
        });
    }

    CheckButtons();

    // if an entry has children, it makes a difference if the entry is checked
    // because all children are checked or if the user checked it explicitly.
    // So we track explicit (un)checking
    implEmphasize(rEntry, eState == TRISTATE_TRUE);
}

TriState OTableTreeListBox::implDetermineState(weld::TreeIter& rEntry)
{
    if (!m_bShowToggles)
        return TRISTATE_FALSE;

    TriState eState = m_xTreeView->get_toggle(rEntry);
    if (!m_xTreeView->iter_has_child(rEntry))
        // nothing to do in this bottom-up routine if there are no children ...
        return eState;

    // loop through the children and check their states
    sal_uInt16 nCheckedChildren = 0;
    sal_uInt16 nChildrenOverall = 0;

    std::unique_ptr<weld::TreeIter> xChild(m_xTreeView->make_iterator(&rEntry));
    bool bChildLoop = m_xTreeView->iter_children(*xChild);
    while (bChildLoop)
    {
        TriState eChildState = implDetermineState(*xChild);
        if (eChildState == TRISTATE_INDET)
            break;
        if (eChildState == TRISTATE_TRUE)
            ++nCheckedChildren;
        ++nChildrenOverall;
        bChildLoop = m_xTreeView->iter_next_sibling(*xChild);
    }

    if (bChildLoop)
    {
        // we did not finish the loop because at least one of the children is in tristate
        eState = TRISTATE_INDET;

        // but this means that we did not finish all the siblings of pChildLoop,
        // so their checking may be incorrect at the moment
        // -> correct this
        while (bChildLoop)
        {
            implDetermineState(*xChild);
            bChildLoop = m_xTreeView->iter_next_sibling(*xChild);
        }
    }
    else
    {
        // none if the children are in tristate
        if (nCheckedChildren)
        {
            // we have at least one child checked
            if (nCheckedChildren != nChildrenOverall)
            {
                // not all children are checked
                eState = TRISTATE_INDET;
            }
            else
            {
                // all children are checked
                eState = TRISTATE_TRUE;
            }
        }
        else
        {
            // no children are checked
            eState = TRISTATE_FALSE;
        }
    }

    // finally set the entry to the state we just determined
    m_xTreeView->set_toggle(rEntry, eState);

    return eState;
}

void OTableTreeListBox::CheckButtons()
{
    if (!m_bShowToggles)
        return;

    auto xEntry(m_xTreeView->make_iterator());
    if (!m_xTreeView->get_iter_first(*xEntry))
        return;

    do
    {
        implDetermineState(*xEntry);
    } while (m_xTreeView->iter_next_sibling(*xEntry));
}

void TableTreeListBox::checkedButton_noBroadcast(const weld::TreeIter& rEntry)
{
    if (!m_bShowToggles)
        return;
    TriState eState = m_xTreeView->get_toggle(rEntry);
    OSL_ENSURE(TRISTATE_INDET != eState, "OTableTreeListBox::CheckButtonHdl: user action which lead to TRISTATE?");

    if (m_xTreeView->iter_has_child(rEntry)) // if it has children, check those too
    {
        std::unique_ptr<weld::TreeIter> xChildEntry(m_xTreeView->make_iterator(&rEntry));
        std::unique_ptr<weld::TreeIter> xSiblingEntry(m_xTreeView->make_iterator(&rEntry));
        bool bChildEntry = m_xTreeView->iter_next(*xChildEntry);
        bool bSiblingEntry = m_xTreeView->iter_next_sibling(*xSiblingEntry);
        while (bChildEntry && (!bSiblingEntry || !xChildEntry->equal(*xSiblingEntry)))
        {
            m_xTreeView->set_toggle(*xChildEntry, eState);
            bChildEntry = m_xTreeView->iter_next(*xChildEntry);
        }
    }

    if (m_xTreeView->is_selected(rEntry))
    {
        m_xTreeView->selected_foreach([this, eState](weld::TreeIter& rSelected){
            m_xTreeView->set_toggle(rSelected, eState);
            if (m_xTreeView->iter_has_child(rSelected)) // if it has children, check those too
            {
                std::unique_ptr<weld::TreeIter> xChildEntry(m_xTreeView->make_iterator(&rSelected));
                std::unique_ptr<weld::TreeIter> xSiblingEntry(m_xTreeView->make_iterator(&rSelected));
                bool bChildEntry = m_xTreeView->iter_next(*xChildEntry);
                bool bSiblingEntry = m_xTreeView->iter_next_sibling(*xSiblingEntry);
                while (bChildEntry && (!bSiblingEntry || !xChildEntry->equal(*xSiblingEntry)))
                {
                    m_xTreeView->set_toggle(*xChildEntry, eState);
                    bChildEntry = m_xTreeView->iter_next(*xChildEntry);
                }
            }
            return false;
        });
    }

    CheckButtons();

    // if an entry has children, it makes a difference if the entry is checked
    // because all children are checked or if the user checked it explicitly.
    // So we track explicit (un)checking
    implEmphasize(rEntry, eState == TRISTATE_TRUE);
}

void OTableTreeListBox::implEmphasize(const weld::TreeIter& rEntry, bool _bChecked, bool _bUpdateDescendants, bool _bUpdateAncestors)
{
    // special emphasizing handling for the "all objects" entry
    bool bAllObjectsEntryAffected = haveVirtualRoot() && (getAllObjectsEntry()->equal(rEntry));
    if  (   m_xTreeView->iter_has_child(rEntry) // the entry has children
        ||  bAllObjectsEntryAffected            // or it is the "all objects" entry
        )
    {
        m_xTreeView->set_text_emphasis(rEntry, _bChecked, 0);
    }

    if (_bUpdateDescendants)
    {
        std::unique_ptr<weld::TreeIter> xChild(m_xTreeView->make_iterator(&rEntry));
        // remove the mark for all children of the checked entry
        bool bChildLoop = m_xTreeView->iter_children(*xChild);
        while (bChildLoop)
        {
            if (m_xTreeView->iter_has_child(*xChild))
                implEmphasize(*xChild, false, true, false);
            bChildLoop = m_xTreeView->iter_next_sibling(*xChild);
        }
    }

    if (_bUpdateAncestors)
    {
        std::unique_ptr<weld::TreeIter> xParent(m_xTreeView->make_iterator(&rEntry));
        // remove the mark for all ancestors of the entry
        if (m_xTreeView->iter_parent(*xParent))
            implEmphasize(*xParent, false, false);
    }
}

void TableTreeListBox::implEmphasize(const weld::TreeIter& rEntry, bool _bChecked, bool _bUpdateDescendants, bool _bUpdateAncestors)
{
    // special emphasizing handling for the "all objects" entry
    bool bAllObjectsEntryAffected = haveVirtualRoot() && (getAllObjectsEntry()->equal(rEntry));
    if  (   m_xTreeView->iter_has_child(rEntry) // the entry has children
        ||  bAllObjectsEntryAffected            // or it is the "all objects" entry
        )
    {
        m_xTreeView->set_text_emphasis(rEntry, _bChecked, 0);
    }

    if (_bUpdateDescendants)
    {
        std::unique_ptr<weld::TreeIter> xChild(m_xTreeView->make_iterator(&rEntry));
        // remove the mark for all children of the checked entry
        bool bChildLoop = m_xTreeView->iter_children(*xChild);
        while (bChildLoop)
        {
            if (m_xTreeView->iter_has_child(*xChild))
                implEmphasize(*xChild, false, true, false);
            bChildLoop = m_xTreeView->iter_next_sibling(*xChild);
        }
    }

    if (_bUpdateAncestors)
    {
        std::unique_ptr<weld::TreeIter> xParent(m_xTreeView->make_iterator(&rEntry));
        // remove the mark for all ancestors of the entry
        if (m_xTreeView->iter_parent(*xParent))
            implEmphasize(*xParent, false, false);
    }
}

std::unique_ptr<weld::TreeIter> OTableTreeListBox::implAddEntry(
        const Reference< XDatabaseMetaData >& _rxMeta,
        const OUString& _rTableName,
        bool _bCheckName
    )
{
    OSL_PRECOND( _rxMeta.is(), "OTableTreeListBox::implAddEntry: invalid meta data!" );
    if ( !_rxMeta.is() )
        return nullptr;

    // split the complete name into its components
    OUString sCatalog, sSchema, sName;
    qualifiedNameComponents( _rxMeta, _rTableName, sCatalog, sSchema, sName, ::dbtools::EComposeRule::InDataManipulation );

    std::unique_ptr<weld::TreeIter> xParentEntry(getAllObjectsEntry());

    // if the DB uses catalog at the start of identifiers, then our hierarchy is
    //   catalog
    //   +- schema
    //      +- table
    // else it is
    //   schema
    //   +- catalog
    //      +- table
    bool bCatalogAtStart = _rxMeta->isCatalogAtStart();
    const OUString& rFirstName  = bCatalogAtStart ? sCatalog : sSchema;
    const sal_Int32 nFirstFolderType   = bCatalogAtStart ? DatabaseObjectContainer::CATALOG : DatabaseObjectContainer::SCHEMA;
    const OUString& rSecondName = bCatalogAtStart ? sSchema : sCatalog;
    const sal_Int32 nSecondFolderType  = bCatalogAtStart ? DatabaseObjectContainer::SCHEMA : DatabaseObjectContainer::CATALOG;

    if ( !rFirstName.isEmpty() )
    {
        std::unique_ptr<weld::TreeIter> xFolder(GetEntryPosByName(rFirstName, xParentEntry.get()));
        if (!xFolder)
        {
            xFolder = m_xTreeView->make_iterator();
            OUString sId(OUString::number(nFirstFolderType));
            OUString sImageId = ImageProvider::getFolderImageId(DatabaseObject::TABLE);
            m_xTreeView->insert(xParentEntry.get(), -1, nullptr, &sId, nullptr, nullptr, false, xFolder.get());
            m_xTreeView->set_image(*xFolder, sImageId, -1);
            if (m_bShowToggles)
                m_xTreeView->set_toggle(*xFolder, TRISTATE_FALSE);
            m_xTreeView->set_text(*xFolder, rFirstName, 0);
            m_xTreeView->set_text_emphasis(*xFolder, false, 0);
        }
        xParentEntry = std::move(xFolder);
    }

    if ( !rSecondName.isEmpty() )
    {
        std::unique_ptr<weld::TreeIter> xFolder(GetEntryPosByName(rSecondName, xParentEntry.get()));
        if (!xFolder)
        {
            xFolder = m_xTreeView->make_iterator();
            OUString sId(OUString::number(nSecondFolderType));
            OUString sImageId = ImageProvider::getFolderImageId(DatabaseObject::TABLE);
            m_xTreeView->insert(xParentEntry.get(), -1, nullptr, &sId, nullptr, nullptr, false, xFolder.get());
            m_xTreeView->set_image(*xFolder, sImageId, -1);
            if (m_bShowToggles)
                m_xTreeView->set_toggle(*xFolder, TRISTATE_FALSE);
            m_xTreeView->set_text(*xFolder, rSecondName, 0);
            m_xTreeView->set_text_emphasis(*xFolder, false, 0);
        }
        xParentEntry = std::move(xFolder);
    }

    if (!_bCheckName || !GetEntryPosByName(sName, xParentEntry.get()))
    {
        std::unique_ptr<weld::TreeIter> xEntry = m_xTreeView->make_iterator();
        m_xTreeView->insert(xParentEntry.get(), -1, nullptr, nullptr, nullptr, nullptr, false, xEntry.get());

        auto xGraphic = m_xImageProvider->getXGraphic(_rTableName, DatabaseObject::TABLE);
        if (xGraphic.is())
            m_xTreeView->set_image(*xEntry, xGraphic, -1);
        else
        {
            OUString sImageId(m_xImageProvider->getImageId(_rTableName, DatabaseObject::TABLE));
            m_xTreeView->set_image(*xEntry, sImageId, -1);
        }
        if (m_bShowToggles)
            m_xTreeView->set_toggle(*xEntry, TRISTATE_FALSE);
        m_xTreeView->set_text(*xEntry, sName, 0);
        m_xTreeView->set_text_emphasis(*xEntry, false, 0);

        return xEntry;
    }

    return nullptr;
}

void TableTreeListBox::implAddEntry(
        const Reference< XDatabaseMetaData >& _rxMeta,
        const OUString& _rTableName,
        bool _bCheckName
    )
{
    OSL_PRECOND( _rxMeta.is(), "OTableTreeListBox::implAddEntry: invalid meta data!" );
    if ( !_rxMeta.is() )
        return;

    // split the complete name into its components
    OUString sCatalog, sSchema, sName;
    qualifiedNameComponents( _rxMeta, _rTableName, sCatalog, sSchema, sName, ::dbtools::EComposeRule::InDataManipulation );

    std::unique_ptr<weld::TreeIter> xParentEntry(getAllObjectsEntry());

    // if the DB uses catalog at the start of identifiers, then our hierarchy is
    //   catalog
    //   +- schema
    //      +- table
    // else it is
    //   schema
    //   +- catalog
    //      +- table
    bool bCatalogAtStart = _rxMeta->isCatalogAtStart();
    const OUString& rFirstName  = bCatalogAtStart ? sCatalog : sSchema;
    const sal_Int32 nFirstFolderType   = bCatalogAtStart ? DatabaseObjectContainer::CATALOG : DatabaseObjectContainer::SCHEMA;
    const OUString& rSecondName = bCatalogAtStart ? sSchema : sCatalog;
    const sal_Int32 nSecondFolderType  = bCatalogAtStart ? DatabaseObjectContainer::SCHEMA : DatabaseObjectContainer::CATALOG;

    if ( !rFirstName.isEmpty() )
    {
        std::unique_ptr<weld::TreeIter> xFolder(GetEntryPosByName(rFirstName, xParentEntry.get()));
        if (!xFolder)
        {
            xFolder = m_xTreeView->make_iterator();
            OUString sId(OUString::number(nFirstFolderType));
            OUString sImageId = ImageProvider::getFolderImageId(DatabaseObject::TABLE);
            m_xTreeView->insert(xParentEntry.get(), -1, nullptr, &sId, nullptr, nullptr, false, xFolder.get());
            m_xTreeView->set_image(*xFolder, sImageId, -1);
            if (m_bShowToggles)
                m_xTreeView->set_toggle(*xFolder, TRISTATE_FALSE);
            m_xTreeView->set_text(*xFolder, rFirstName, 0);
            m_xTreeView->set_text_emphasis(*xFolder, false, 0);
        }
        xParentEntry = std::move(xFolder);
    }

    if ( !rSecondName.isEmpty() )
    {
        std::unique_ptr<weld::TreeIter> xFolder(GetEntryPosByName(rSecondName, xParentEntry.get()));
        if (!xFolder)
        {
            xFolder = m_xTreeView->make_iterator();
            OUString sId(OUString::number(nSecondFolderType));
            OUString sImageId = ImageProvider::getFolderImageId(DatabaseObject::TABLE);
            m_xTreeView->insert(xParentEntry.get(), -1, nullptr, &sId, nullptr, nullptr, false, xFolder.get());
            m_xTreeView->set_image(*xFolder, sImageId, -1);
            if (m_bShowToggles)
                m_xTreeView->set_toggle(*xFolder, TRISTATE_FALSE);
            m_xTreeView->set_text(*xFolder, rSecondName, 0);
            m_xTreeView->set_text_emphasis(*xFolder, false, 0);
        }
        xParentEntry = std::move(xFolder);
    }

    if (!_bCheckName || !GetEntryPosByName(sName, xParentEntry.get()))
    {
        std::unique_ptr<weld::TreeIter> xEntry = m_xTreeView->make_iterator();
        m_xTreeView->insert(xParentEntry.get(), -1, nullptr, nullptr, nullptr, nullptr, false, xEntry.get());

        auto xGraphic = m_xImageProvider->getXGraphic(_rTableName, DatabaseObject::TABLE);
        if (xGraphic.is())
            m_xTreeView->set_image(*xEntry, xGraphic, -1);
        else
        {
            OUString sImageId(m_xImageProvider->getImageId(_rTableName, DatabaseObject::TABLE));
            m_xTreeView->set_image(*xEntry, sImageId, -1);
        }
        if (m_bShowToggles)
            m_xTreeView->set_toggle(*xEntry, TRISTATE_FALSE);
        m_xTreeView->set_text(*xEntry, sName, 0);
        m_xTreeView->set_text_emphasis(*xEntry, false, 0);
    }
}

NamedDatabaseObject OTableTreeListBox::describeObject(weld::TreeIter& rEntry)
{
    NamedDatabaseObject aObject;

    sal_Int32 nEntryType = m_xTreeView->get_id(rEntry).toInt32();

    if  ( nEntryType == DatabaseObjectContainer::TABLES )
    {
        aObject.Type = DatabaseObjectContainer::TABLES;
    }
    else if (   ( nEntryType == DatabaseObjectContainer::CATALOG )
            ||  ( nEntryType == DatabaseObjectContainer::SCHEMA )
            )
    {
        // nothing useful to be done
    }
    else
    {
        aObject.Type = DatabaseObject::TABLE;
        aObject.Name = getQualifiedTableName(rEntry);
    }

    return aObject;
}

std::unique_ptr<weld::TreeIter> OTableTreeListBox::addedTable(const OUString& rName)
{
    try
    {
        Reference< XDatabaseMetaData > xMeta;
        if ( impl_getAndAssertMetaData( xMeta ) )
            return implAddEntry( xMeta, rName );
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("dbaccess");
    }
    return nullptr;
}

bool OTableTreeListBox::impl_getAndAssertMetaData( Reference< XDatabaseMetaData >& _out_rMetaData ) const
{
    if ( m_xConnection.is() )
        _out_rMetaData = m_xConnection->getMetaData();
    OSL_PRECOND( _out_rMetaData.is(), "OTableTreeListBox::impl_getAndAssertMetaData: invalid current connection!" );
    return _out_rMetaData.is();
}

OUString OTableTreeListBox::getQualifiedTableName(weld::TreeIter& rEntry) const
{
    OSL_PRECOND( !isFolderEntry(rEntry), "OTableTreeListBox::getQualifiedTableName: folder entries not allowed here!" );

    try
    {
        Reference< XDatabaseMetaData > xMeta;
        if ( !impl_getAndAssertMetaData( xMeta ) )
            return OUString();

        OUString sCatalog;
        OUString sSchema;
        OUString sTable;

        std::unique_ptr<weld::TreeIter> xSchema(m_xTreeView->make_iterator(&rEntry));
        bool bSchema = m_xTreeView->iter_parent(*xSchema);
        if (bSchema)
        {
            std::unique_ptr<weld::TreeIter> xCatalog(m_xTreeView->make_iterator(xSchema.get()));
            bool bCatalog = m_xTreeView->iter_parent(*xCatalog);
            if  (   bCatalog
                ||  (   xMeta->supportsCatalogsInDataManipulation()
                    &&  !xMeta->supportsSchemasInDataManipulation()
                    )   // here we support catalog but no schema
                )
            {
                if (!bCatalog)
                {
                    xCatalog = std::move(xSchema);
                    bSchema = false;
                }
                sCatalog = m_xTreeView->get_text(*xCatalog);
            }
            if (bSchema)
                sSchema = m_xTreeView->get_text(*xSchema);
        }
        sTable = m_xTreeView->get_text(rEntry);

        return ::dbtools::composeTableName( xMeta, sCatalog, sSchema, sTable, false, ::dbtools::EComposeRule::InDataManipulation );
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("dbaccess");
    }
    return OUString();
}

std::unique_ptr<weld::TreeIter> OTableTreeListBox::getEntryByQualifiedName(const OUString& rName)
{
    try
    {
        Reference< XDatabaseMetaData > xMeta;
        if ( !impl_getAndAssertMetaData( xMeta ) )
            return nullptr;

        // split the complete name into its components
        OUString sCatalog, sSchema, sName;
        qualifiedNameComponents(xMeta, rName, sCatalog, sSchema, sName,::dbtools::EComposeRule::InDataManipulation);

        std::unique_ptr<weld::TreeIter> xParent(getAllObjectsEntry());
        std::unique_ptr<weld::TreeIter> xCat;
        std::unique_ptr<weld::TreeIter> xSchema;
        if (!sCatalog.isEmpty())
        {
            xCat = GetEntryPosByName(sCatalog);
            if (xCat)
                xParent = std::move(xCat);
        }

        if (!sSchema.isEmpty())
        {
            xSchema = GetEntryPosByName(sSchema, xParent.get());
            if (xSchema)
                xParent = std::move(xSchema);
        }

        return GetEntryPosByName(sName, xParent.get());
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("dbaccess");
    }
    return nullptr;
}

void OTableTreeListBox::removedTable(const OUString& rName)
{
    try
    {
        std::unique_ptr<weld::TreeIter> xEntry = getEntryByQualifiedName(rName);
        if (xEntry)
            m_xTreeView->remove(*xEntry);
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("dbaccess");
    }
}

void TableTreeListBox::CheckButtons()
{
    if (!m_bShowToggles)
        return;

    auto xEntry(m_xTreeView->make_iterator());
    if (!m_xTreeView->get_iter_first(*xEntry))
        return;

    do
    {
        implDetermineState(*xEntry);
    } while (m_xTreeView->iter_next_sibling(*xEntry));
}

TriState TableTreeListBox::implDetermineState(weld::TreeIter& rEntry)
{
    if (!m_bShowToggles)
        return TRISTATE_FALSE;

    TriState eState = m_xTreeView->get_toggle(rEntry);
    if (!m_xTreeView->iter_has_child(rEntry))
        // nothing to do in this bottom-up routine if there are no children ...
        return eState;

    // loop through the children and check their states
    sal_uInt16 nCheckedChildren = 0;
    sal_uInt16 nChildrenOverall = 0;

    std::unique_ptr<weld::TreeIter> xChild(m_xTreeView->make_iterator(&rEntry));
    bool bChildLoop = m_xTreeView->iter_children(*xChild);
    while (bChildLoop)
    {
        TriState eChildState = implDetermineState(*xChild);
        if (eChildState == TRISTATE_INDET)
            break;
        if (eChildState == TRISTATE_TRUE)
            ++nCheckedChildren;
        ++nChildrenOverall;
        bChildLoop = m_xTreeView->iter_next_sibling(*xChild);
    }

    if (bChildLoop)
    {
        // we did not finish the loop because at least one of the children is in tristate
        eState = TRISTATE_INDET;

        // but this means that we did not finish all the siblings of pChildLoop,
        // so their checking may be incorrect at the moment
        // -> correct this
        while (bChildLoop)
        {
            implDetermineState(*xChild);
            bChildLoop = m_xTreeView->iter_next_sibling(*xChild);
        }
    }
    else
    {
        // none if the children are in tristate
        if (nCheckedChildren)
        {
            // we have at least one child checked
            if (nCheckedChildren != nChildrenOverall)
            {
                // not all children are checked
                eState = TRISTATE_INDET;
            }
            else
            {
                // all children are checked
                eState = TRISTATE_TRUE;
            }
        }
        else
        {
            // no children are checked
            eState = TRISTATE_FALSE;
        }
    }

    // finally set the entry to the state we just determined
    m_xTreeView->set_toggle(rEntry, eState);

    return eState;
}

}   // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
