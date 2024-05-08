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

#include "commonpagesdbp.hxx"
#include <strings.hrc>
#include <bitmaps.hlst>
#include <componentmodule.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/sdb/XCompletedConnection.hpp>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <com/sun/star/sdb/XQueriesSupplier.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/sdb/SQLContext.hpp>
#include <com/sun/star/sdbc/SQLWarning.hpp>
#include <com/sun/star/sdb/CommandType.hpp>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <tools/debug.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <comphelper/interaction.hxx>
#include <connectivity/dbtools.hxx>
#include <sfx2/docfilt.hxx>
#include <unotools/pathoptions.hxx>
#include <sfx2/filedlghelper.hxx>
#include <svl/filenotation.hxx>
#include <osl/diagnose.h>

namespace dbp
{


    using namespace ::com::sun::star;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::container;
    using namespace ::com::sun::star::sdb;
    using namespace ::com::sun::star::sdbc;
    using namespace ::com::sun::star::sdbcx;
    using namespace ::com::sun::star::task;
    using namespace ::comphelper;

    OTableSelectionPage::OTableSelectionPage(weld::Container* pPage, OControlWizard* pWizard)
        : OControlWizardPage(pPage, pWizard, u"modules/sabpilot/ui/tableselectionpage.ui"_ustr, u"TableSelectionPage"_ustr)
        , m_xTable(m_xBuilder->weld_tree_view(u"table"_ustr))
        , m_xDatasource(m_xBuilder->weld_tree_view(u"datasource"_ustr))
        , m_xSearchDatabase(m_xBuilder->weld_button(u"search"_ustr))
        , m_xSourceBox(m_xBuilder->weld_container(u"sourcebox"_ustr))
    {
        try
        {
            m_xDSContext = getContext().xDatasourceContext;
            if (m_xDSContext.is())
                fillListBox(*m_xDatasource, m_xDSContext->getElementNames());
        }
        catch (const Exception&)
        {
            OSL_FAIL("OTableSelectionPage::OTableSelectionPage: could not collect the data source names!");
        }

        m_xDatasource->connect_changed(LINK(this, OTableSelectionPage, OnListboxSelection));
        m_xTable->connect_changed(LINK(this, OTableSelectionPage, OnListboxSelection));
        m_xTable->connect_row_activated(LINK(this, OTableSelectionPage, OnListboxDoubleClicked));
        m_xSearchDatabase->connect_clicked(LINK(this, OTableSelectionPage, OnSearchClicked));
    }

    OTableSelectionPage::~OTableSelectionPage()
    {
    }

    void OTableSelectionPage::Activate()
    {
        OControlWizardPage::Activate();
        m_xDatasource->grab_focus();
    }

    bool OTableSelectionPage::canAdvance() const
    {
        if (!OControlWizardPage::canAdvance())
            return false;

        if (0 == m_xDatasource->count_selected_rows())
            return false;

        if (0 == m_xTable->count_selected_rows())
            return false;

        return true;
    }

    void OTableSelectionPage::initializePage()
    {
        OControlWizardPage::initializePage();

        const OControlWizardContext& rContext = getContext();
        try
        {
            OUString sDataSourceName;
            rContext.xForm->getPropertyValue(u"DataSourceName"_ustr) >>= sDataSourceName;

            Reference< XConnection > xConnection;
            bool bEmbedded = ::dbtools::isEmbeddedInDatabase( rContext.xForm, xConnection );
            if ( bEmbedded )
            {
                m_xSourceBox->hide();
                m_xDatasource->append_text(sDataSourceName);
            }
            m_xDatasource->select_text(sDataSourceName);

            implFillTables(xConnection);

            OUString sCommand;
            OSL_VERIFY( rContext.xForm->getPropertyValue(u"Command"_ustr) >>= sCommand );
            sal_Int32 nCommandType = CommandType::TABLE;
            OSL_VERIFY( rContext.xForm->getPropertyValue(u"CommandType"_ustr) >>= nCommandType );

            // search the entry of the given type with the given name
            for (sal_Int32 nLookup = 0; nLookup < m_xTable->n_children(); ++nLookup)
            {
                if (sCommand == m_xTable->get_text(nLookup))
                {
                    if (m_xTable->get_id(nLookup).toInt32() == nCommandType)
                    {
                        m_xTable->select( nLookup );
                        break;
                    }
                }
            }
        }
        catch(const Exception&)
        {
            TOOLS_WARN_EXCEPTION("extensions.abpilot", "OTableSelectionPage::initializePage");
        }
    }

    bool OTableSelectionPage::commitPage( ::vcl::WizardTypes::CommitPageReason _eReason )
    {
        if (!OControlWizardPage::commitPage(_eReason))
            return false;

        const OControlWizardContext& rContext = getContext();
        try
        {
            Reference< XConnection > xOldConn;
            if ( !rContext.bEmbedded )
            {
                xOldConn = getFormConnection();

                OUString sDataSource = m_xDatasource->get_selected_text();
                rContext.xForm->setPropertyValue(u"DataSourceName"_ustr, Any( sDataSource ) );
            }
            OUString sCommand = m_xTable->get_selected_text();
            sal_Int32 nCommandType = m_xTable->get_selected_id().toInt32();

            rContext.xForm->setPropertyValue(u"Command"_ustr, Any( sCommand ) );
            rContext.xForm->setPropertyValue(u"CommandType"_ustr, Any( nCommandType ) );

            if ( !rContext.bEmbedded )
                setFormConnection( xOldConn, false );

            if (!updateContext())
                return false;
        }
        catch(const Exception&)
        {
            TOOLS_WARN_EXCEPTION("extensions.dbpilots", "OTableSelectionPage::commitPage");
        }

        return true;
    }

    IMPL_LINK_NOARG( OTableSelectionPage, OnSearchClicked, weld::Button&, void )
    {
        ::sfx2::FileDialogHelper aFileDlg(
                ui::dialogs::TemplateDescription::FILEOPEN_READONLY_VERSION,
                FileDialogFlags::NONE, getDialog()->getDialog());
        aFileDlg.SetDisplayDirectory( SvtPathOptions().GetWorkPath() );

        std::shared_ptr<const SfxFilter> pFilter = SfxFilter::GetFilterByName(u"StarOffice XML (Base)"_ustr);
        OSL_ENSURE(pFilter,"Filter: StarOffice XML (Base) could not be found!");
        if ( pFilter )
        {
            aFileDlg.AddFilter(pFilter->GetUIName(),pFilter->GetDefaultExtension());
        }

        if (ERRCODE_NONE == aFileDlg.Execute())
        {
            OUString sDataSourceName = aFileDlg.GetPath();
            ::svt::OFileNotation aFileNotation(sDataSourceName);
            sDataSourceName = aFileNotation.get(::svt::OFileNotation::N_SYSTEM);
            m_xDatasource->append_text(sDataSourceName);
            m_xDatasource->select_text(sDataSourceName);
            LINK(this, OTableSelectionPage, OnListboxSelection).Call(*m_xDatasource);
        }
    }

    IMPL_LINK(OTableSelectionPage, OnListboxDoubleClicked, weld::TreeView&, _rBox, bool)
    {
        if (_rBox.count_selected_rows())
            getDialog()->travelNext();
        return true;
    }

    IMPL_LINK(OTableSelectionPage, OnListboxSelection, weld::TreeView&, _rBox, void)
    {
        if (m_xDatasource.get() == &_rBox)
        {   // new data source selected
            implFillTables();
        }

        updateDialogTravelUI();
    }

    namespace
    {
        void lcl_fillEntries(weld::TreeView& rListBox, const Sequence<OUString>& rNames, const OUString& rImage, sal_Int32 nCommandType)
        {
            for (auto const & name : rNames)
            {
                rListBox.append(OUString::number(nCommandType), name, rImage);
            }
        }
    }

    void OTableSelectionPage::implFillTables(const Reference< XConnection >& _rxConn)
    {
        m_xTable->clear();

        weld::WaitObject aWaitCursor(getDialog()->getDialog());

        // will be the table tables of the selected data source
        Sequence< OUString > aTableNames;
        Sequence< OUString > aQueryNames;

        // connect to the data source
        Any aSQLException;
        Reference< XConnection > xConn = _rxConn;
        if ( !xConn.is() )
        {
            if (!m_xDSContext.is())
                return;
            // connect to the data source
            try
            {
                OUString sCurrentDatasource = m_xDatasource->get_selected_text();
                if (!sCurrentDatasource.isEmpty())
                {
                    // obtain the DS object
                    Reference< XCompletedConnection > xDatasource;
                    // check if I know this one otherwise transform it into a file URL
                    if ( !m_xDSContext->hasByName(sCurrentDatasource) )
                    {
                        ::svt::OFileNotation aFileNotation(sCurrentDatasource);
                        sCurrentDatasource = aFileNotation.get(::svt::OFileNotation::N_URL);
                    }

                    if (m_xDSContext->getByName(sCurrentDatasource) >>= xDatasource)
                    {   // connect
                        // get the default SDB interaction handler
                        Reference< XInteractionHandler > xHandler = getDialog()->getInteractionHandler(getDialog()->getDialog());
                        if (!xHandler.is() )
                            return;
                        xConn = xDatasource->connectWithCompletion(xHandler);
                        setFormConnection( xConn );
                    }
                    else
                    {
                        OSL_FAIL("OTableSelectionPage::implFillTables: invalid data source object returned by the context");
                    }
                }
            }
            catch(const SQLContext& e) { aSQLException <<= e; }
            catch(const SQLWarning& e) { aSQLException <<= e; }
            catch(const SQLException& e) { aSQLException <<= e; }
            catch (const Exception&)
            {
                OSL_FAIL("OTableSelectionPage::implFillTables: could not fill the table list!");
            }
        }

        // will be the table tables of the selected data source
        if ( xConn.is() )
        {
            try
            {
                // get the tables
                Reference< XTablesSupplier > xSupplTables(xConn, UNO_QUERY);
                if ( xSupplTables.is() )
                {
                    Reference< XNameAccess > xTables = xSupplTables->getTables();
                    if (xTables.is())
                        aTableNames = xTables->getElementNames();
                }

                // and the queries
                Reference< XQueriesSupplier > xSuppQueries( xConn, UNO_QUERY );
                if ( xSuppQueries.is() )
                {
                    Reference< XNameAccess > xQueries = xSuppQueries->getQueries();
                    if ( xQueries.is() )
                        aQueryNames = xQueries->getElementNames();
                }
            }
            catch(const SQLContext& e) { aSQLException <<= e; }
            catch(const SQLWarning& e) { aSQLException <<= e; }
            catch(const SQLException& e) { aSQLException <<= e; }
            catch (const Exception&)
            {
                OSL_FAIL("OTableSelectionPage::implFillTables: could not fill the table list!");
            }
        }


        if ( aSQLException.hasValue() )
        {   // an SQLException (or derivee) was thrown ...
            Reference< XInteractionRequest > xRequest = new OInteractionRequest(aSQLException);
            try
            {
                // get the default SDB interaction handler
                Reference< XInteractionHandler > xHandler = getDialog()->getInteractionHandler(getDialog()->getDialog());
                if ( xHandler.is() )
                    xHandler->handle(xRequest);
            }
            catch(const Exception&) { }
            return;
        }

        lcl_fillEntries(*m_xTable, aTableNames, BMP_TABLE, CommandType::TABLE);
        lcl_fillEntries(*m_xTable, aQueryNames, BMP_QUERY, CommandType::QUERY);
    }

    OMaybeListSelectionPage::OMaybeListSelectionPage(weld::Container* pPage, OControlWizard* pWizard, const OUString& rUIXMLDescription, const OUString& rID)
        : OControlWizardPage(pPage, pWizard, rUIXMLDescription, rID)
        , m_pYes(nullptr)
        , m_pNo(nullptr)
        , m_pList(nullptr)
    {
    }

    OMaybeListSelectionPage::~OMaybeListSelectionPage()
    {
    }

    void OMaybeListSelectionPage::announceControls(weld::RadioButton& _rYesButton, weld::RadioButton& _rNoButton, weld::ComboBox& _rSelection)
    {
        m_pYes = &_rYesButton;
        m_pNo = &_rNoButton;
        m_pList = &_rSelection;

        m_pYes->connect_toggled(LINK(this, OMaybeListSelectionPage, OnRadioSelected));
        m_pNo->connect_toggled(LINK(this, OMaybeListSelectionPage, OnRadioSelected));
        implEnableWindows();
    }

    IMPL_LINK(OMaybeListSelectionPage, OnRadioSelected, weld::Toggleable&, rButton, void)
    {
        if (!rButton.get_active())
            return;
        implEnableWindows();
    }

    void OMaybeListSelectionPage::implInitialize(const OUString& _rSelection)
    {
        DBG_ASSERT(m_pYes, "OMaybeListSelectionPage::implInitialize: no controls announced!");
        bool bIsSelection = ! _rSelection.isEmpty();
        m_pYes->set_active(bIsSelection);
        m_pNo->set_active(!bIsSelection);
        m_pList->set_sensitive(bIsSelection);

        m_pList->set_active_text(bIsSelection ? _rSelection : OUString());
    }

    void OMaybeListSelectionPage::implCommit(OUString& _rSelection)
    {
        _rSelection = m_pYes->get_active() ? m_pList->get_active_text() : OUString();
    }

    void OMaybeListSelectionPage::implEnableWindows()
    {
        m_pList->set_sensitive(m_pYes->get_active());
    }

    void OMaybeListSelectionPage::Activate()
    {
        OControlWizardPage::Activate();

        assert(m_pYes && "OMaybeListSelectionPage::Activate: no controls announced!");
        if (m_pYes->get_active())
            m_pList->grab_focus();
        else
            m_pNo->grab_focus();
    }

    ODBFieldPage::ODBFieldPage(weld::Container* pPage, OControlWizard* pWizard)
        : OMaybeListSelectionPage(pPage, pWizard, u"modules/sabpilot/ui/optiondbfieldpage.ui"_ustr, u"OptionDBField"_ustr)
        , m_xDescription(m_xBuilder->weld_label(u"explLabel"_ustr))
        , m_xStoreYes(m_xBuilder->weld_radio_button(u"yesRadiobutton"_ustr))
        , m_xStoreNo(m_xBuilder->weld_radio_button(u"noRadiobutton"_ustr))
        , m_xStoreWhere(m_xBuilder->weld_combo_box(u"storeInFieldCombobox"_ustr))
    {
        SetPageTitle(compmodule::ModuleRes(RID_STR_OPTION_DB_FIELD_TITLE));

        announceControls(*m_xStoreYes, *m_xStoreNo, *m_xStoreWhere);
    }

    ODBFieldPage::~ODBFieldPage()
    {
    }

    void ODBFieldPage::initializePage()
    {
        OMaybeListSelectionPage::initializePage();

        // fill the fields page
        fillListBox(*m_xStoreWhere, getContext().aFieldNames);

        implInitialize(getDBFieldSetting());
    }

    bool ODBFieldPage::commitPage( ::vcl::WizardTypes::CommitPageReason _eReason )
    {
        if (!OMaybeListSelectionPage::commitPage(_eReason))
            return false;

        implCommit(getDBFieldSetting());

        return true;
    }


}   // namespace dbp


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
