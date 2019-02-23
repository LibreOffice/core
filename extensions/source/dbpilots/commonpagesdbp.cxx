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
#include <comphelper/interaction.hxx>
#include <connectivity/dbtools.hxx>
#include <vcl/stdtext.hxx>
#include <vcl/waitobj.hxx>
#include <vcl/layout.hxx>
#include <sfx2/docfilt.hxx>
#include <unotools/pathoptions.hxx>
#include <sfx2/filedlghelper.hxx>
#include <svl/filenotation.hxx>
#include <osl/diagnose.h>

namespace dbp
{


    using namespace ::com::sun::star;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::container;
    using namespace ::com::sun::star::sdb;
    using namespace ::com::sun::star::sdbc;
    using namespace ::com::sun::star::sdbcx;
    using namespace ::com::sun::star::task;
    using namespace ::comphelper;

    OTableSelectionPage::OTableSelectionPage(OControlWizard* _pParent)
        :OControlWizardPage(_pParent, "TableSelectionPage", "modules/sabpilot/ui/tableselectionpage.ui")
    {
        get(m_pTable,"table");
        get(m_pDatasource, "datasource");
        get(m_pDatasourceLabel, "datasourcelabel");
        get(m_pSearchDatabase, "search");

        try
        {
            m_xDSContext = getContext().xDatasourceContext;
            if (m_xDSContext.is())
                fillListBox(*m_pDatasource, m_xDSContext->getElementNames());
        }
        catch (const Exception&)
        {
            OSL_FAIL("OTableSelectionPage::OTableSelectionPage: could not collect the data source names!");
        }

        m_pDatasource->SetSelectHdl(LINK(this, OTableSelectionPage, OnListboxSelection));
        m_pTable->SetSelectHdl(LINK(this, OTableSelectionPage, OnListboxSelection));
        m_pTable->SetDoubleClickHdl(LINK(this, OTableSelectionPage, OnListboxDoubleClicked));
        m_pSearchDatabase->SetClickHdl(LINK(this, OTableSelectionPage, OnSearchClicked));

        m_pDatasource->SetDropDownLineCount(10);
    }

    OTableSelectionPage::~OTableSelectionPage()
    {
        disposeOnce();
    }

    void OTableSelectionPage::dispose()
    {
        m_pDatasourceLabel.clear();
        m_pDatasource.clear();
        m_pSearchDatabase.clear();
        m_pTable.clear();
        OControlWizardPage::dispose();
    }

    void OTableSelectionPage::ActivatePage()
    {
        OControlWizardPage::ActivatePage();
        m_pDatasource->GrabFocus();
    }


    bool OTableSelectionPage::canAdvance() const
    {
        if (!OControlWizardPage::canAdvance())
            return false;

        if (0 == m_pDatasource->GetSelectedEntryCount())
            return false;

        if (0 == m_pTable->GetSelectedEntryCount())
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
            rContext.xForm->getPropertyValue("DataSourceName") >>= sDataSourceName;

            Reference< XConnection > xConnection;
            bool bEmbedded = ::dbtools::isEmbeddedInDatabase( rContext.xForm, xConnection );
            if ( bEmbedded )
            {
                VclVBox *_pSourceBox = get<VclVBox>("sourcebox");
                _pSourceBox->Hide();
                m_pDatasource->InsertEntry(sDataSourceName);
            }
            m_pDatasource->SelectEntry(sDataSourceName);

            implFillTables(xConnection);

            OUString sCommand;
            OSL_VERIFY( rContext.xForm->getPropertyValue("Command") >>= sCommand );
            sal_Int32 nCommandType = CommandType::TABLE;
            OSL_VERIFY( rContext.xForm->getPropertyValue("CommandType") >>= nCommandType );

            // search the entry of the given type with the given name
            for ( sal_Int32 nLookup = 0; nLookup < m_pTable->GetEntryCount(); ++nLookup )
            {
                if (sCommand == m_pTable->GetEntry(nLookup))
                {
                    if ( reinterpret_cast< sal_IntPtr >( m_pTable->GetEntryData( nLookup ) ) == nCommandType )
                    {
                        m_pTable->SelectEntryPos( nLookup );
                        break;
                    }
                }
            }
        }
        catch(const Exception&)
        {
            OSL_FAIL("OTableSelectionPage::initializePage: caught an exception!");
        }
    }


    bool OTableSelectionPage::commitPage( ::svt::WizardTypes::CommitPageReason _eReason )
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

                OUString sDataSource = m_pDatasource->GetSelectedEntry();
                rContext.xForm->setPropertyValue("DataSourceName", makeAny( sDataSource ) );
            }
            OUString sCommand = m_pTable->GetSelectedEntry();
            sal_Int32 nCommandType = reinterpret_cast< sal_IntPtr >( m_pTable->GetSelectedEntryData() );

            rContext.xForm->setPropertyValue("Command", makeAny( sCommand ) );
            rContext.xForm->setPropertyValue("CommandType", makeAny( nCommandType ) );

            if ( !rContext.bEmbedded )
                setFormConnection( xOldConn, false );

            if (!updateContext())
                return false;
        }
        catch(const Exception&)
        {
            OSL_FAIL("OTableSelectionPage::commitPage: caught an exception!");
        }

        return true;
    }


    IMPL_LINK_NOARG( OTableSelectionPage, OnSearchClicked, Button*, void )
    {
        ::sfx2::FileDialogHelper aFileDlg(
                ui::dialogs::TemplateDescription::FILEOPEN_READONLY_VERSION,
                FileDialogFlags::NONE, GetFrameWeld());
        aFileDlg.SetDisplayDirectory( SvtPathOptions().GetWorkPath() );

        std::shared_ptr<const SfxFilter> pFilter = SfxFilter::GetFilterByName("StarOffice XML (Base)");
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
            m_pDatasource->InsertEntry(sDataSourceName);
            m_pDatasource->SelectEntry(sDataSourceName);
            LINK(this, OTableSelectionPage, OnListboxSelection).Call(*m_pDatasource);
        }
    }

    IMPL_LINK( OTableSelectionPage, OnListboxDoubleClicked, ListBox&, _rBox, void )
    {
        if (_rBox.GetSelectedEntryCount())
            getDialog()->travelNext();
    }


    IMPL_LINK( OTableSelectionPage, OnListboxSelection, ListBox&, _rBox, void )
    {
        if (m_pDatasource == &_rBox)
        {   // new data source selected
            implFillTables();
        }

        updateDialogTravelUI();
    }


    namespace
    {
        void lcl_fillEntries( ListBox& _rListBox, const Sequence< OUString >& _rNames, const Image& _rImage, sal_Int32 _nCommandType )
        {
            for ( auto const & name : _rNames )
            {
                const sal_Int32 nPos = _rListBox.InsertEntry( name, _rImage );
                _rListBox.SetEntryData( nPos, reinterpret_cast< void* >( _nCommandType ) );
            }
        }
    }


    void OTableSelectionPage::implFillTables(const Reference< XConnection >& _rxConn)
    {
        m_pTable->Clear();

        WaitObject aWaitCursor(this);

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
                OUString sCurrentDatasource = m_pDatasource->GetSelectedEntry();
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
                        Reference< XInteractionHandler > xHandler = getDialog()->getInteractionHandler(GetFrameWeld());
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
                    Reference< XNameAccess > xTables(xSupplTables->getTables(), UNO_QUERY);
                    if (xTables.is())
                        aTableNames = xTables->getElementNames();
                }

                // and the queries
                Reference< XQueriesSupplier > xSuppQueries( xConn, UNO_QUERY );
                if ( xSuppQueries.is() )
                {
                    Reference< XNameAccess > xQueries( xSuppQueries->getQueries(), UNO_QUERY );
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
                Reference< XInteractionHandler > xHandler = getDialog()->getInteractionHandler(GetFrameWeld());
                if ( xHandler.is() )
                    xHandler->handle(xRequest);
            }
            catch(const Exception&) { }
            return;
        }

        Image aTableImage(StockImage::Yes, BMP_TABLE);
        Image aQueryImage(StockImage::Yes, BMP_QUERY);

        lcl_fillEntries( *m_pTable, aTableNames, aTableImage, CommandType::TABLE );
        lcl_fillEntries( *m_pTable, aQueryNames, aQueryImage, CommandType::QUERY );
    }


    OMaybeListSelectionPage::OMaybeListSelectionPage( OControlWizard* _pParent, const OString& _rID, const OUString& _rUIXMLDescription )
        :OControlWizardPage(_pParent, _rID, _rUIXMLDescription)
        ,m_pYes(nullptr)
        ,m_pNo(nullptr)
        ,m_pList(nullptr)
    {
    }

    OMaybeListSelectionPage::~OMaybeListSelectionPage()
    {
        disposeOnce();
    }

    void OMaybeListSelectionPage::dispose()
    {
        m_pYes.clear();
        m_pNo.clear();
        m_pList.clear();
        OControlWizardPage::dispose();
    }

    void OMaybeListSelectionPage::announceControls(RadioButton& _rYesButton, RadioButton& _rNoButton, ListBox& _rSelection)
    {
        m_pYes = &_rYesButton;
        m_pNo = &_rNoButton;
        m_pList = &_rSelection;

        m_pYes->SetClickHdl(LINK(this, OMaybeListSelectionPage, OnRadioSelected));
        m_pNo->SetClickHdl(LINK(this, OMaybeListSelectionPage, OnRadioSelected));
        implEnableWindows();
    }

    IMPL_LINK_NOARG( OMaybeListSelectionPage, OnRadioSelected, Button*, void )
    {
        implEnableWindows();
    }


    void OMaybeListSelectionPage::implInitialize(const OUString& _rSelection)
    {
        DBG_ASSERT(m_pYes, "OMaybeListSelectionPage::implInitialize: no controls announced!");
        bool bIsSelection = ! _rSelection.isEmpty();
        m_pYes->Check(bIsSelection);
        m_pNo->Check(!bIsSelection);
        m_pList->Enable(bIsSelection);

        m_pList->SelectEntry(bIsSelection ? _rSelection : OUString());
    }


    void OMaybeListSelectionPage::implCommit(OUString& _rSelection)
    {
        _rSelection = m_pYes->IsChecked() ? m_pList->GetSelectedEntry() : OUString();
    }


    void OMaybeListSelectionPage::implEnableWindows()
    {
        m_pList->Enable(m_pYes->IsChecked());
    }


    void OMaybeListSelectionPage::ActivatePage()
    {
        OControlWizardPage::ActivatePage();

        DBG_ASSERT(m_pYes, "OMaybeListSelectionPage::ActivatePage: no controls announced!");
        if (m_pYes->IsChecked())
            m_pList->GrabFocus();
        else
            m_pNo->GrabFocus();
    }

    ODBFieldPage::ODBFieldPage( OControlWizard* _pParent )
        :OMaybeListSelectionPage(_pParent, "OptionDBField", "modules/sabpilot/ui/optiondbfieldpage.ui")
    {
        get(m_pDescription, "explLabel");
        get(m_pStoreYes, "yesRadiobutton");
        get(m_pStoreNo, "noRadiobutton");
        get(m_pStoreWhere, "storeInFieldCombobox");
        SetText(compmodule::ModuleRes(RID_STR_OPTION_DB_FIELD_TITLE));

        announceControls(*m_pStoreYes, *m_pStoreNo, *m_pStoreWhere);
        m_pStoreWhere->SetDropDownLineCount(10);
    }

    ODBFieldPage::~ODBFieldPage()
    {
        disposeOnce();
    }

    void ODBFieldPage::dispose()
    {
        m_pDescription.clear();
        m_pStoreYes.clear();
        m_pStoreNo.clear();
        m_pStoreWhere.clear();
        OMaybeListSelectionPage::dispose();
    }

    void ODBFieldPage::initializePage()
    {
        OMaybeListSelectionPage::initializePage();

        // fill the fields page
        fillListBox(*m_pStoreWhere, getContext().aFieldNames);

        implInitialize(getDBFieldSetting());
    }


    bool ODBFieldPage::commitPage( ::svt::WizardTypes::CommitPageReason _eReason )
    {
        if (!OMaybeListSelectionPage::commitPage(_eReason))
            return false;

        implCommit(getDBFieldSetting());

        return true;
    }


}   // namespace dbp


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
