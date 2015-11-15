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
#include "dbu_dlg.hrc"
#include "dsitems.hxx"
#include "browserids.hxx"
#include "datasourceconnector.hxx"
#include <comphelper/types.hxx>
#include <connectivity/dbtools.hxx>
#include <connectivity/dbexception.hxx>
#include "stringlistitem.hxx"
#include <svl/eitem.hxx>
#include <svl/stritem.hxx>
#include "dbustrings.hrc"
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <vcl/waitobj.hxx>
#include <com/sun/star/i18n/Collator.hpp>
#include <com/sun/star/sdb/SQLContext.hpp>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <com/sun/star/sdbcx/XAppend.hpp>
#include <com/sun/star/util/XModifiable.hpp>
#include <com/sun/star/sdbcx/XDrop.hpp>
#include <com/sun/star/sdbcx/XDataDefinitionSupplier.hpp>
#include "sqlmessage.hxx"
#include <vcl/msgbox.hxx>
#include "dbaccess_helpid.hrc"
#include "UITools.hxx"
#include <osl/mutex.hxx>
#include <osl/diagnose.h>
#include <svtools/imgdef.hxx>
#include "svtools/treelistentry.hxx"
#include "TablesSingleDlg.hxx"
#include <tools/diagnose_ex.h>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/exc_hlp.hxx>

namespace dbaui
{

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::sdbc;
    using namespace ::com::sun::star::sdbcx;
    using namespace ::com::sun::star::sdb;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::i18n;
    using namespace ::com::sun::star::container;
    using namespace ::com::sun::star::frame;
    using namespace ::com::sun::star::util;
    using namespace ::dbtools;
    using namespace ::comphelper;

    // OTableSubscriptionPage
    OTableSubscriptionPage::OTableSubscriptionPage(vcl::Window* pParent, const SfxItemSet& _rCoreAttrs,
        OTableSubscriptionDialog* _pTablesDlg)
        : OGenericAdministrationPage(pParent, "TablesFilterPage",
            "dbaccess/ui/tablesfilterpage.ui", _rCoreAttrs)
        , m_bCatalogAtStart(true)
        , m_pTablesDlg(_pTablesDlg)
    {
        get(m_pTables, "TablesFilterPage");

        get(m_pTablesList, "treeview");
        m_pTablesList->init(true);
        m_pTablesList->set_width_request(56 * m_pTablesList->approximate_char_width());
        m_pTablesList->set_height_request(12 * m_pTablesList->GetTextHeight());

        m_pTablesList->SetCheckHandler(LINK(this,OGenericAdministrationPage,OnControlModified));

        // initialize the TabListBox
        m_pTablesList->SetSelectionMode( MULTIPLE_SELECTION );
        m_pTablesList->SetDragDropMode( DragDropMode::NONE );
        m_pTablesList->EnableInplaceEditing( false );
        m_pTablesList->SetStyle(m_pTablesList->GetStyle() | WB_BORDER | WB_HASLINES | WB_HASLINESATROOT | WB_SORT | WB_HASBUTTONS | WB_HSCROLL |WB_HASBUTTONSATROOT);

        m_pTablesList->Clear();

        m_pTablesList->SetCheckButtonHdl(LINK(this, OTableSubscriptionPage, OnTreeEntryButtonChecked));
        m_pTablesList->SetCheckHandler(LINK(this, OTableSubscriptionPage, OnTreeEntryChecked));
    }

    OTableSubscriptionPage::~OTableSubscriptionPage()
    {
        disposeOnce();
    }

    void OTableSubscriptionPage::dispose()
    {
        // just to make sure that our connection will be removed
        try
        {
            ::comphelper::disposeComponent(m_xCurrentConnection);
        }
        catch (RuntimeException&) { }
        m_pTables.clear();
        m_pTablesList.clear();
        m_pTablesDlg.clear();
        OGenericAdministrationPage::dispose();
    }

    void OTableSubscriptionPage::StateChanged( StateChangedType nType )
    {
        OGenericAdministrationPage::StateChanged( nType );

        if ( nType == StateChangedType::ControlBackground )
        {
            // Check if we need to get new images for normal/high contrast mode
            m_pTablesList->notifyHiContrastChanged();
        }
    }
    void OTableSubscriptionPage::DataChanged( const DataChangedEvent& rDCEvt )
    {
        OGenericAdministrationPage::DataChanged( rDCEvt );

        if ((( rDCEvt.GetType() == DataChangedEventType::SETTINGS )   ||
            ( rDCEvt.GetType() == DataChangedEventType::DISPLAY   ))  &&
            ( rDCEvt.GetFlags() & AllSettingsFlags::STYLE        ))
        {
            // Check if we need to get new images for normal/high contrast mode
            m_pTablesList->notifyHiContrastChanged();
        }
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

        SvTreeListEntry* pRootEntry = m_pTablesList->getAllObjectsEntry();

        const OUString* pIncludeTable = _rTables.getConstArray();
        for (sal_Int32 i=0; i<_rTables.getLength(); ++i, ++pIncludeTable)
        {
            if (xMeta.is())
                qualifiedNameComponents(xMeta, pIncludeTable->getStr(), sCatalog, sSchema, sName,::dbtools::eInDataManipulation);
            else
                sName = pIncludeTable->getStr();

            bool bAllTables = (1 == sName.getLength()) && ('%' == sName[0]);
            bool bAllSchemas = (1 == sSchema.getLength()) && ('%' == sSchema[0]);

            // the catalog entry
            SvTreeListEntry* pCatalog = m_pTablesList->GetEntryPosByName(sCatalog, pRootEntry);
            if (!(pCatalog || sCatalog.isEmpty()))
                // the table (resp. its catalog) referred in this filter entry does not exist anymore
                continue;

            if (bAllSchemas && pCatalog)
            {
                m_pTablesList->checkWildcard(pCatalog);
                continue;
            }

            // the schema entry
            SvTreeListEntry* pSchema = m_pTablesList->GetEntryPosByName(sSchema, (pCatalog ? pCatalog : pRootEntry));
            if (!(pSchema || sSchema.isEmpty()))
                // the table (resp. its schema) referred in this filter entry does not exist anymore
                continue;

            if (bAllTables && pSchema)
            {
                m_pTablesList->checkWildcard(pSchema);
                continue;
            }

            SvTreeListEntry* pEntry = m_pTablesList->GetEntryPosByName(sName, pSchema ? pSchema : (pCatalog ? pCatalog : pRootEntry) );
            if (pEntry)
                m_pTablesList->SetCheckButtonState(pEntry, SV_BUTTON_CHECKED);
        }
        m_pTablesList->CheckButtons();
    }

    void OTableSubscriptionPage::implCompleteTablesCheck( const css::uno::Sequence< OUString >& _rTableFilter )
    {
        if (!_rTableFilter.getLength())
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
                    m_pTablesList->Clear();
                    m_pTablesDlg->endExecution();
                    return;
                }
            }

            if (!m_xCollator.is())
            {
                // the collator for the string compares
                try
                {
                    m_xCollator = Collator::create(m_xORB);
                    m_xCollator->loadDefaultCollator(Application::GetSettings().GetLanguageTag().getLocale(), 0);
                }
                catch(const Exception&)
                {
                    DBG_UNHANDLED_EXCEPTION();
                }
            }

            // fill the table list with this connection information
            SQLExceptionInfo aErrorInfo;
            // the current DSN
            OUString sURL;
            if ( m_pTablesDlg )
                sURL = m_pTablesDlg->getConnectionURL();

            try
            {
                WaitObject aWaitCursor(this);
                m_pTablesList->GetModel()->SetSortMode(SortAscending);
                m_pTablesList->GetModel()->SetCompareHdl(LINK(this, OTableSubscriptionPage, OnTreeEntryCompare));

                Reference< XDriver > xDriver;
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
                        xModi->setModified(sal_False);

                }

                if ( m_xCurrentConnection.is() )
                {
                    m_pTablesList->UpdateTableList( m_xCurrentConnection );
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
                ScopedVclPtrInstance< OSQLMessageBox > aMessageBox( GetParentDialog(), aErrorInfo );
                aMessageBox->Execute();
                m_pTables->Enable(false);
                m_pTablesList->Clear();

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
                    DBG_UNHANDLED_EXCEPTION();
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
        SvTreeListEntry* pExpand = m_pTablesList->getAllObjectsEntry();
        while (pExpand)
        {
            m_pTablesList->Expand(pExpand);
            pExpand = m_pTablesList->FirstChild(pExpand);
            if (pExpand && SvTreeList::NextSibling(pExpand))
                pExpand = nullptr;
        }

        // update the toolbox according the current selection and check state
        OGenericAdministrationPage::implInitControls(_rSet, _bSaveValue);
    }

    void OTableSubscriptionPage::CheckAll( bool _bCheck )
    {
        SvButtonState eState = _bCheck ? SV_BUTTON_CHECKED : SV_BUTTON_UNCHECKED;
        SvTreeListEntry* pEntry = m_pTablesList->First();
        while (pEntry)
        {
            m_pTablesList->SetCheckButtonState( pEntry, eState);
            pEntry = m_pTablesList->Next(pEntry);
        }

        if (_bCheck && m_pTablesList->getAllObjectsEntry())
            m_pTablesList->checkWildcard(m_pTablesList->getAllObjectsEntry());
    }

    SfxTabPage::sfxpg OTableSubscriptionPage::DeactivatePage(SfxItemSet* _pSet)
    {
        sfxpg nResult = OGenericAdministrationPage::DeactivatePage(_pSet);

        // dispose the connection, we don't need it anymore, so we're not wasting resources
        try
        {
            ::comphelper::disposeComponent(m_xCurrentConnection);
        }
        catch (RuntimeException&) { }

        return nResult;
    }
    IMPL_LINK_NOARG_TYPED( OTableSubscriptionPage, OnTreeEntryButtonChecked, SvTreeListBox*, void )
    {
        callModifiedHdl();
    }
    IMPL_LINK_TYPED( OTableSubscriptionPage, OnTreeEntryChecked, void*, _pControl, void )
    {
        OnControlModified(_pControl);
    }
    IMPL_LINK_TYPED( OTableSubscriptionPage, OnTreeEntryCompare, const SvSortData&, _rSortData, sal_Int32 )
    {
        const SvTreeListEntry* pLHS = static_cast<const SvTreeListEntry*>(_rSortData.pLeft);
        const SvTreeListEntry* pRHS = static_cast<const SvTreeListEntry*>(_rSortData.pRight);
        OSL_ENSURE(pLHS && pRHS, "SbaTableQueryBrowser::OnTreeEntryCompare: invalid tree entries!");

        const SvLBoxString* pLeftTextItem = static_cast<const SvLBoxString*>(pLHS->GetFirstItem(SV_ITEM_ID_LBOXSTRING));
        const SvLBoxString* pRightTextItem = static_cast<const SvLBoxString*>(pRHS->GetFirstItem(SV_ITEM_ID_LBOXSTRING));
        OSL_ENSURE(pLeftTextItem && pRightTextItem, "SbaTableQueryBrowser::OnTreeEntryCompare: invalid text items!");

        OUString sLeftText = pLeftTextItem->GetText();
        OUString sRightText = pRightTextItem->GetText();

        sal_Int32 nCompareResult = 0;   // equal by default

        if (m_xCollator.is())
        {
            try
            {
                nCompareResult = m_xCollator->compareString(sLeftText, sRightText);
            }
            catch(Exception&)
            {
            }
        }
        else
            // default behaviour if we do not have a collator -> do the simple string compare
            nCompareResult = sLeftText.compareTo(sRightText);

        return nCompareResult;
    }

    Sequence< OUString > OTableSubscriptionPage::collectDetailedSelection() const
    {
        Sequence< OUString > aTableFilter;
        static const char sDot[] = ".";
        static const char sWildcard[] = "%";

        OUString sComposedName;
        const SvTreeListEntry* pAllObjectsEntry = m_pTablesList->getAllObjectsEntry();
        if (!pAllObjectsEntry)
            return aTableFilter;
        SvTreeListEntry* pEntry = m_pTablesList->GetModel()->Next(const_cast<SvTreeListEntry*>(pAllObjectsEntry));
        while(pEntry)
        {
            bool bCatalogWildcard = false;
            bool bSchemaWildcard =  false;
            SvTreeListEntry* pSchema = nullptr;
            SvTreeListEntry* pCatalog = nullptr;

            if (m_pTablesList->GetCheckButtonState(pEntry) == SV_BUTTON_CHECKED && !m_pTablesList->GetModel()->HasChildren(pEntry))
            {   // checked and a leaf, which means it's no catalog, no schema, but a real table
                OUString sCatalog;
                if(m_pTablesList->GetModel()->HasParent(pEntry))
                {
                    pSchema = m_pTablesList->GetModel()->GetParent(pEntry);
                    if (pAllObjectsEntry == pSchema)
                        // do not want to have the root entry
                        pSchema = nullptr;

                    if (pSchema)
                    {   // it's a real schema entry, not the "all objects" root
                        if(m_pTablesList->GetModel()->HasParent(pSchema))
                        {
                            pCatalog = m_pTablesList->GetModel()->GetParent(pSchema);
                            if (pAllObjectsEntry == pCatalog)
                                // do not want to have the root entry
                                pCatalog = nullptr;

                            if (pCatalog)
                            {   // it's a real catalog entry, not the "all objects" root
                                bCatalogWildcard = OTableTreeListBox::isWildcardChecked(pCatalog);
                                if (m_bCatalogAtStart)
                                {
                                    sComposedName += m_pTablesList->GetEntryText( pCatalog );
                                    sComposedName += m_sCatalogSeparator;
                                    if (bCatalogWildcard)
                                        sComposedName += sWildcard;
                                }
                                else
                                {
                                    if (bCatalogWildcard)
                                        sCatalog = sWildcard;
                                    else
                                        sCatalog.clear();
                                    sCatalog += m_sCatalogSeparator;
                                    sCatalog += m_pTablesList->GetEntryText( pCatalog );
                                }
                            }
                        }
                        bSchemaWildcard = OTableTreeListBox::isWildcardChecked(pSchema);
                        sComposedName += m_pTablesList->GetEntryText( pSchema );
                        sComposedName += sDot;
                    }

                    if (bSchemaWildcard)
                        sComposedName += sWildcard;
                }
                if (!bSchemaWildcard && !bCatalogWildcard)
                    sComposedName += m_pTablesList->GetEntryText( pEntry );

                if (!m_bCatalogAtStart && !bCatalogWildcard)
                    sComposedName += sCatalog;

                // need some space
                sal_Int32 nOldLen = aTableFilter.getLength();
                aTableFilter.realloc(nOldLen + 1);
                // add the new name
                aTableFilter[nOldLen] = sComposedName;

                // reset the composed name
                sComposedName.clear();
            }

            if (bCatalogWildcard)
                pEntry = implNextSibling(pCatalog);
            else if (bSchemaWildcard)
                pEntry = implNextSibling(pSchema);
            else
                pEntry = m_pTablesList->GetModel()->Next(pEntry);
        }

        return aTableFilter;
    }

    SvTreeListEntry* OTableSubscriptionPage::implNextSibling(SvTreeListEntry* _pEntry) const
    {
        SvTreeListEntry* pReturn = nullptr;
        if (_pEntry)
        {
            pReturn = SvTreeList::NextSibling(_pEntry);
            if (!pReturn)
                pReturn = implNextSibling(m_pTablesList->GetParent(_pEntry));
        }
        return pReturn;
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
            if (dbaui::OTableTreeListBox::isWildcardChecked(m_pTablesList->getAllObjectsEntry()))
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

    void OTableSubscriptionPage::fillControls(::std::vector< ISaveValueWrapper* >& /*_rControlList*/)
    {
    }

    void OTableSubscriptionPage::fillWindows(::std::vector< ISaveValueWrapper* >& _rControlList)
    {
        _rControlList.push_back(new ODisableWrapper<VclContainer>(m_pTables));
    }
}   // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
