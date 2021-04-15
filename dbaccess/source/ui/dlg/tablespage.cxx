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

#include "tablespage.hxx"
#include <dsitems.hxx>
#include <datasourceconnector.hxx>
#include <comphelper/types.hxx>
#include <connectivity/dbtools.hxx>
#include <connectivity/dbexception.hxx>
#include <stringlistitem.hxx>
#include <svl/stritem.hxx>
#include <strings.hxx>
#include <com/sun/star/sdbc/SQLException.hpp>
#include <com/sun/star/util/XModifiable.hpp>
#include <sqlmessage.hxx>
#include <UITools.hxx>
#include <osl/diagnose.h>
#include <TablesSingleDlg.hxx>
#include <tools/diagnose_ex.h>
#include <cppuhelper/exc_hlp.hxx>

namespace dbaui
{

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::sdbc;
    using namespace ::com::sun::star::sdbcx;
    using namespace ::com::sun::star::sdb;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::container;
    using namespace ::com::sun::star::util;
    using namespace ::dbtools;
    using namespace ::comphelper;

    // OTableSubscriptionPage
    OTableSubscriptionPage::OTableSubscriptionPage(weld::Container* pPage, OTableSubscriptionDialog* pTablesDlg, const SfxItemSet& _rCoreAttrs)
        : OGenericAdministrationPage(pPage, pTablesDlg, "dbaccess/ui/tablesfilterpage.ui", "TablesFilterPage", _rCoreAttrs)
        , m_bCatalogAtStart(true)
        , m_pTablesDlg(pTablesDlg)
        , m_xTables(m_xBuilder->weld_widget("TablesFilterPage"))
        , m_xTablesList(new OTableTreeListBox(m_xBuilder->weld_tree_view("treeview"), true))
    {
        m_xTablesList->init();

        weld::TreeView& rWidget = m_xTablesList->GetWidget();

        rWidget.set_size_request(rWidget.get_approximate_digit_width() * 48,
                                 rWidget.get_height_rows(12));

        // initialize the TabListBox
        rWidget.set_selection_mode(SelectionMode::Multiple);

        rWidget.connect_toggled(LINK(this, OTableSubscriptionPage, OnTreeEntryChecked));
    }

    OTableSubscriptionPage::~OTableSubscriptionPage()
    {
        // just to make sure that our connection will be removed
        try
        {
            ::comphelper::disposeComponent(m_xCurrentConnection);
        }
        catch (RuntimeException&) { }
    }

    void OTableSubscriptionPage::implCheckTables(const Sequence< OUString >& _rTables)
    {
        // the meta data for the current connection, used for splitting up table names
        Reference< XDatabaseMetaData > xMeta;
        try
        {
            if (m_xCurrentConnection.is())
                xMeta = m_xCurrentConnection->getMetaData();
        }
        catch(SQLException&)
        {
            OSL_FAIL("OTableSubscriptionPage::implCheckTables : could not retrieve the current connection's meta data!");
        }

        // uncheck all
        CheckAll(false);

        // check the ones which are in the list
        OUString sCatalog, sSchema, sName;

        std::unique_ptr<weld::TreeIter> xRootEntry(m_xTablesList->getAllObjectsEntry());

        for (const OUString& rIncludeTable : _rTables)
        {
            if (xMeta.is())
                qualifiedNameComponents(xMeta, rIncludeTable, sCatalog, sSchema, sName,::dbtools::EComposeRule::InDataManipulation);
            else
                sName = rIncludeTable;

            bool bAllTables = (1 == sName.getLength()) && ('%' == sName[0]);
            bool bAllSchemas = (1 == sSchema.getLength()) && ('%' == sSchema[0]);

            // the catalog entry
            std::unique_ptr<weld::TreeIter> xCatalog(m_xTablesList->GetEntryPosByName(sCatalog, xRootEntry.get()));
            if (!(xCatalog || sCatalog.isEmpty()))
                // the table (resp. its catalog) referred in this filter entry does not exist anymore
                continue;

            if (bAllSchemas && xCatalog)
            {
                m_xTablesList->checkWildcard(*xCatalog);
                continue;
            }

            // the schema entry
            std::unique_ptr<weld::TreeIter> xSchema = m_xTablesList->GetEntryPosByName(sSchema, (xCatalog ? xCatalog.get() : xRootEntry.get()));
            if (!(xSchema || sSchema.isEmpty()))
                // the table (resp. its schema) referred in this filter entry does not exist anymore
                continue;

            if (bAllTables && xSchema)
            {
                m_xTablesList->checkWildcard(*xSchema);
                continue;
            }

            std::unique_ptr<weld::TreeIter> xEntry(m_xTablesList->GetEntryPosByName(sName, xSchema ? xSchema.get() : (xCatalog ? xCatalog.get() : xRootEntry.get())));
            if (xEntry)
                m_xTablesList->GetWidget().set_toggle(*xEntry, TRISTATE_TRUE);
        }
        m_xTablesList->CheckButtons();
    }

    void OTableSubscriptionPage::implCompleteTablesCheck( const css::uno::Sequence< OUString >& _rTableFilter )
    {
        if (!_rTableFilter.hasElements())
        {   // no tables visible
            CheckAll(false);
        }
        else
        {
            if ((1 == _rTableFilter.getLength()) && _rTableFilter[0] == "%")
            {   // all tables visible
                CheckAll();
            }
            else
                implCheckTables( _rTableFilter );
        }
    }

    void OTableSubscriptionPage::implInitControls(const SfxItemSet& _rSet, bool _bSaveValue)
    {
        // check whether or not the selection is invalid or readonly (invalid implies readonly, but not vice versa)
        bool bValid, bReadonly;
        getFlags(_rSet, bValid, bReadonly);

        // get the name of the data source we're working for
        const SfxStringItem* pNameItem = _rSet.GetItem<SfxStringItem>(DSID_NAME);
        OSL_ENSURE(pNameItem, "OTableSubscriptionPage::implInitControls: missing the name attribute!");
        OUString sDSName = pNameItem->GetValue();

        if (bValid && !sDSName.isEmpty() && !m_xCurrentConnection.is() )
        {   // get the current table list from the connection for the current settings

            // the PropertyValues for the current dialog settings
            Sequence< PropertyValue > aConnectionParams;
            OSL_ENSURE(m_pTablesDlg, "OTableSubscriptionPage::implInitControls: need a parent dialog doing the translation!");
            if ( m_pTablesDlg )
            {
                if (!m_pTablesDlg->getCurrentSettings(aConnectionParams))
                {
                    m_xTablesList->GetWidget().clear();
                    m_pTablesDlg->endExecution();
                    return;
                }
            }

            // fill the table list with this connection information
            SQLExceptionInfo aErrorInfo;

            try
            {
                weld::WaitObject aWaitCursor(GetFrameWeld());

                Reference<XPropertySet> xProp = m_pTablesDlg->getCurrentDataSource();
                OSL_ENSURE(xProp.is(),"No data source set!");
                if ( xProp.is() )
                {
                    Any aTableFilter = xProp->getPropertyValue(PROPERTY_TABLEFILTER);
                    Any aTableTypeFilter = xProp->getPropertyValue(PROPERTY_TABLETYPEFILTER);

                    Reference<XModifiable> xModi(getDataSourceOrModel(xProp),UNO_QUERY);
                    bool bModified = ( xModi.is() && xModi->isModified() );

                    Sequence< OUString > aNewTableFilter { "%" };
                    xProp->setPropertyValue(PROPERTY_TABLEFILTER,makeAny(aNewTableFilter));

                    xProp->setPropertyValue( PROPERTY_TABLETYPEFILTER, makeAny( Sequence< OUString >() ) );
                    Reference< css::lang::XEventListener> xEvt;
                    aErrorInfo = ::dbaui::createConnection(xProp, m_xORB, xEvt, m_xCurrentConnection);

                    xProp->setPropertyValue(PROPERTY_TABLEFILTER,aTableFilter);
                    xProp->setPropertyValue(PROPERTY_TABLETYPEFILTER,aTableTypeFilter);

                    if ( xModi.is() && !bModified )
                        xModi->setModified(false);

                }

                if ( m_xCurrentConnection.is() )
                {
                    m_xTablesList->UpdateTableList( m_xCurrentConnection );
                    if (m_pTablesDlg)
                        m_pTablesDlg->successfullyConnected();
                }
            }
            catch (const SQLException&)
            {
                aErrorInfo = ::cppu::getCaughtException();
            }

            if (aErrorInfo.isValid())
            {
                // establishing the connection failed. Show an error window and exit.
                OSQLMessageBox aMessageBox(GetFrameWeld(), aErrorInfo);
                aMessageBox.run();
                m_xTables->set_sensitive(false);
                m_xTablesList->GetWidget().clear();

                if ( m_pTablesDlg )
                {
                    m_pTablesDlg->clearPassword();
                    m_pTablesDlg->endExecution();
                }
            }
            else
            {
                // in addition, we need some infos about the connection used
                m_sCatalogSeparator = ".";    // (default)
                m_bCatalogAtStart = true;   // (default)
                try
                {
                    Reference< XDatabaseMetaData > xMeta;
                    if (m_xCurrentConnection.is())
                        xMeta = m_xCurrentConnection->getMetaData();
                    if (xMeta.is() && xMeta->supportsCatalogsInDataManipulation())
                    {
                        m_sCatalogSeparator = xMeta->getCatalogSeparator();
                        m_bCatalogAtStart = xMeta->isCatalogAtStart();
                    }
                }
                catch(Exception&)
                {
                    DBG_UNHANDLED_EXCEPTION("dbaccess");
                }
            }
        }

        // get the current table filter
        const OStringListItem* pTableFilter = _rSet.GetItem<OStringListItem>(DSID_TABLEFILTER);
        Sequence< OUString > aTableFilter;
        if (pTableFilter)
            aTableFilter = pTableFilter->getList();

        implCompleteTablesCheck( aTableFilter );

        // expand the first entry by default
        std::unique_ptr<weld::TreeIter> xExpand = m_xTablesList->getAllObjectsEntry();
        while (xExpand)
        {
            m_xTablesList->GetWidget().expand_row(*xExpand);
            if (!m_xTablesList->GetWidget().iter_children(*xExpand))
                break;
            std::unique_ptr<weld::TreeIter> xSibling(m_xTablesList->GetWidget().make_iterator(xExpand.get()));
            if (m_xTablesList->GetWidget().iter_next_sibling(*xSibling))
                xExpand.reset();
        }

        // update the toolbox according the current selection and check state
        OGenericAdministrationPage::implInitControls(_rSet, _bSaveValue);
    }

    void OTableSubscriptionPage::CheckAll( bool _bCheck )
    {
        std::unique_ptr<weld::TreeIter> xEntry(m_xTablesList->GetWidget().make_iterator());
        if (m_xTablesList->GetWidget().get_iter_first(*xEntry))
        {
            do
            {
                m_xTablesList->GetWidget().set_toggle(*xEntry, _bCheck ? TRISTATE_TRUE : TRISTATE_FALSE);
            }
            while (m_xTablesList->GetWidget().iter_next(*xEntry));
        }

        if (_bCheck)
        {
            auto xRoot = m_xTablesList->getAllObjectsEntry();
            if (xRoot)
                m_xTablesList->checkWildcard(*xRoot);
        }
    }

    DeactivateRC OTableSubscriptionPage::DeactivatePage(SfxItemSet* _pSet)
    {
        DeactivateRC nResult = OGenericAdministrationPage::DeactivatePage(_pSet);

        // dispose the connection, we don't need it anymore, so we're not wasting resources
        try
        {
            ::comphelper::disposeComponent(m_xCurrentConnection);
        }
        catch (RuntimeException&) { }

        return nResult;
    }

    IMPL_LINK(OTableSubscriptionPage, OnTreeEntryChecked, const weld::TreeView::iter_col&, rRowCol, void)
    {
        m_xTablesList->checkedButton_noBroadcast(rRowCol.first);
        callModifiedHdl();
    }

    Sequence< OUString > OTableSubscriptionPage::collectDetailedSelection() const
    {
        Sequence< OUString > aTableFilter;
        constexpr OUStringLiteral sWildcard = u"%";

        std::unique_ptr<weld::TreeIter> xAllObjectsEntry(m_xTablesList->getAllObjectsEntry());
        if (!xAllObjectsEntry)
            return aTableFilter;
        std::unique_ptr<weld::TreeIter> xEntry(m_xTablesList->GetWidget().make_iterator(xAllObjectsEntry.get()));
        if (!m_xTablesList->GetWidget().iter_next(*xEntry))
            xEntry.reset();
        while (xEntry)
        {
            bool bCatalogWildcard = false;
            bool bSchemaWildcard =  false;
            std::unique_ptr<weld::TreeIter> xSchema;
            std::unique_ptr<weld::TreeIter> xCatalog;

            if (m_xTablesList->GetWidget().get_toggle(*xEntry) == TRISTATE_TRUE && !m_xTablesList->GetWidget().iter_has_child(*xEntry))
            {   // checked and a leaf, which means it's no catalog, no schema, but a real table
                OUStringBuffer sComposedName;
                OUString sCatalog;
                if (m_xTablesList->GetWidget().get_iter_depth(*xEntry))
                {
                    xSchema = m_xTablesList->GetWidget().make_iterator(xEntry.get());
                    m_xTablesList->GetWidget().iter_parent(*xSchema);
                    if (xAllObjectsEntry->equal(*xSchema))
                    {
                        // do not want to have the root entry
                        xSchema.reset();
                    }

                    if (xSchema)
                    {   // it's a real schema entry, not the "all objects" root
                        if (m_xTablesList->GetWidget().get_iter_depth(*xSchema))
                        {
                            xCatalog = m_xTablesList->GetWidget().make_iterator(xSchema.get());
                            m_xTablesList->GetWidget().iter_parent(*xCatalog);
                            if (xAllObjectsEntry->equal(*xCatalog))
                            {
                                // do not want to have the root entry
                                xCatalog.reset();
                            }

                            if (xCatalog)
                            {   // it's a real catalog entry, not the "all objects" root
                                bCatalogWildcard = m_xTablesList->isWildcardChecked(*xCatalog);
                                if (m_bCatalogAtStart)
                                {
                                    sComposedName.append(m_xTablesList->GetWidget().get_text(*xCatalog)).append(m_sCatalogSeparator);
                                    if (bCatalogWildcard)
                                        sComposedName.append(sWildcard);
                                }
                                else
                                {
                                    if (bCatalogWildcard)
                                        sCatalog = sWildcard;
                                    else
                                        sCatalog.clear();
                                    sCatalog += m_sCatalogSeparator + m_xTablesList->GetWidget().get_text(*xCatalog) ;
                                }
                            }
                        }
                        bSchemaWildcard = m_xTablesList->isWildcardChecked(*xSchema);
                        sComposedName.append(m_xTablesList->GetWidget().get_text(*xSchema)).append(".");
                    }

                    if (bSchemaWildcard)
                        sComposedName.append(sWildcard);
                }
                if (!bSchemaWildcard && !bCatalogWildcard)
                    sComposedName.append(m_xTablesList->GetWidget().get_text(*xEntry));

                if (!m_bCatalogAtStart && !bCatalogWildcard)
                    sComposedName.append(sCatalog);

                // need some space
                sal_Int32 nOldLen = aTableFilter.getLength();
                aTableFilter.realloc(nOldLen + 1);
                // add the new name
                aTableFilter[nOldLen] = sComposedName.makeStringAndClear();
            }

            if (bCatalogWildcard)
                xEntry = implNextSibling(xCatalog.get());
            else if (bSchemaWildcard)
                xEntry = implNextSibling(xSchema.get());
            else
            {
                if (!m_xTablesList->GetWidget().iter_next(*xEntry))
                    xEntry.reset();
            }
        }

        return aTableFilter;
    }

    std::unique_ptr<weld::TreeIter> OTableSubscriptionPage::implNextSibling(const weld::TreeIter* pEntry) const
    {
        std::unique_ptr<weld::TreeIter> xReturn;
        if (pEntry)
        {
            xReturn = m_xTablesList->GetWidget().make_iterator(pEntry);
            if (!m_xTablesList->GetWidget().iter_next_sibling(*xReturn))
            {
                std::unique_ptr<weld::TreeIter> xParent = m_xTablesList->GetWidget().make_iterator(pEntry);
                if (m_xTablesList->GetWidget().iter_parent(*xParent))
                    xReturn = implNextSibling(xParent.get());
                else
                    xReturn.reset();
            }
        }
        return xReturn;
    }

    bool OTableSubscriptionPage::FillItemSet( SfxItemSet* _rCoreAttrs )
    {
        bool bValid, bReadonly;
        getFlags(*_rCoreAttrs, bValid, bReadonly);

        if (!bValid || bReadonly)
            // don't store anything if the data we're working with is invalid or readonly
            return true;

        // create the output string which contains all the table names
        if ( m_xCurrentConnection.is() )
        {   // collect the table filter data only if we have a connection - else no tables are displayed at all
            Sequence< OUString > aTableFilter;
            auto xRoot = m_xTablesList->getAllObjectsEntry();
            if (xRoot && m_xTablesList->isWildcardChecked(*xRoot))
            {
                aTableFilter.realloc(1);
                aTableFilter[0] = "%";
            }
            else
            {
                aTableFilter = collectDetailedSelection();
            }
            _rCoreAttrs->Put( OStringListItem(DSID_TABLEFILTER, aTableFilter) );
        }

        return true;
    }

    void OTableSubscriptionPage::fillControls(std::vector< std::unique_ptr<ISaveValueWrapper> >& /*_rControlList*/)
    {
    }

    void OTableSubscriptionPage::fillWindows(std::vector< std::unique_ptr<ISaveValueWrapper> >& _rControlList)
    {
        _rControlList.emplace_back(new ODisableWidgetWrapper<weld::Widget>(m_xTables.get()));
    }
}   // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
