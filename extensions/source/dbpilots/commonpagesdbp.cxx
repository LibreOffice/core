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
#include "dbpresid.hrc"
#include "componentmodule.hxx"
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
#include <svtools/localresaccess.hxx>
#include <comphelper/interaction.hxx>
#include <connectivity/dbtools.hxx>
#include <vcl/stdtext.hxx>
#include <vcl/waitobj.hxx>
#include <sfx2/docfilt.hxx>
#include <unotools/pathoptions.hxx>
#include <sfx2/filedlghelper.hxx>
#include <svl/filenotation.hxx>
//.........................................................................
namespace dbp
{
//.........................................................................

    using namespace ::com::sun::star;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::container;
    using namespace ::com::sun::star::sdb;
    using namespace ::com::sun::star::sdbc;
    using namespace ::com::sun::star::sdbcx;
    using namespace ::com::sun::star::task;
    using namespace ::comphelper;

    //=====================================================================
    //= OTableSelectionPage
    //=====================================================================
    //---------------------------------------------------------------------
    OTableSelectionPage::OTableSelectionPage(OControlWizard* _pParent)
        :OControlWizardPage(_pParent, ModuleRes(RID_PAGE_TABLESELECTION))
        ,m_aData            (this, ModuleRes(FL_DATA))
        ,m_aExplanation     (this, ModuleRes(FT_EXPLANATION))
        ,m_aDatasourceLabel (this, ModuleRes(FT_DATASOURCE))
        ,m_aDatasource      (this, ModuleRes(LB_DATASOURCE))
        ,m_aSearchDatabase  (this, ModuleRes(PB_FORMDATASOURCE))
        ,m_aTableLabel      (this, ModuleRes(FT_TABLE))
        ,m_aTable           (this, ModuleRes(LB_TABLE))
    {
        FreeResource();

        implCollectDatasource();

        m_aDatasource.SetSelectHdl(LINK(this, OTableSelectionPage, OnListboxSelection));
        m_aTable.SetSelectHdl(LINK(this, OTableSelectionPage, OnListboxSelection));
        m_aTable.SetDoubleClickHdl(LINK(this, OTableSelectionPage, OnListboxDoubleClicked));
        m_aSearchDatabase.SetClickHdl(LINK(this, OTableSelectionPage, OnSearchClicked));

        m_aDatasource.SetDropDownLineCount(10);
    }

    //---------------------------------------------------------------------
    void OTableSelectionPage::ActivatePage()
    {
        OControlWizardPage::ActivatePage();
        m_aDatasource.GrabFocus();
    }

    //---------------------------------------------------------------------
    bool OTableSelectionPage::canAdvance() const
    {
        if (!OControlWizardPage::canAdvance())
            return false;

        if (0 == m_aDatasource.GetSelectEntryCount())
            return false;

        if (0 == m_aTable.GetSelectEntryCount())
            return false;

        return sal_True;
    }

    //---------------------------------------------------------------------
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
                m_aDatasource.Hide();
                m_aDatasourceLabel.Hide();
                m_aSearchDatabase.Hide();
                m_aTableLabel.SetPosPixel(m_aDatasourceLabel.GetPosPixel());
                m_aTable.SetPosPixel(m_aDatasource.GetPosPixel());
                m_aDatasource.InsertEntry(sDataSourceName);
            }
            m_aDatasource.SelectEntry(sDataSourceName);

            implFillTables(xConnection);

            OUString sCommand;
            OSL_VERIFY( rContext.xForm->getPropertyValue("Command") >>= sCommand );
            sal_Int32 nCommandType = CommandType::TABLE;
            OSL_VERIFY( rContext.xForm->getPropertyValue("CommandType") >>= nCommandType );

            // search the entry of the given type with the given name
            for ( sal_uInt16 nLookup = 0; nLookup < m_aTable.GetEntryCount(); ++nLookup )
            {
                if (sCommand.equals(m_aTable.GetEntry(nLookup)))
                {
                    if ( reinterpret_cast< sal_IntPtr >( m_aTable.GetEntryData( nLookup ) ) == nCommandType )
                    {
                        m_aTable.SelectEntryPos( nLookup );
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

    //---------------------------------------------------------------------
    sal_Bool OTableSelectionPage::commitPage( ::svt::WizardTypes::CommitPageReason _eReason )
    {
        if (!OControlWizardPage::commitPage(_eReason))
            return sal_False;

        const OControlWizardContext& rContext = getContext();
        try
        {
            Reference< XConnection > xOldConn;
            if ( !rContext.bEmbedded )
            {
                xOldConn = getFormConnection();

                OUString sDataSource = m_aDatasource.GetSelectEntry();
                rContext.xForm->setPropertyValue("DataSourceName", makeAny( sDataSource ) );
            }
            OUString sCommand = m_aTable.GetSelectEntry();
            sal_Int32 nCommandType = reinterpret_cast< sal_IntPtr >( m_aTable.GetEntryData( m_aTable.GetSelectEntryPos() ) );

            rContext.xForm->setPropertyValue("Command", makeAny( sCommand ) );
            rContext.xForm->setPropertyValue("CommandType", makeAny( nCommandType ) );

            if ( !rContext.bEmbedded )
                setFormConnection( xOldConn, sal_False );

            if (!updateContext())
                return sal_False;
        }
        catch(const Exception&)
        {
            OSL_FAIL("OTableSelectionPage::commitPage: caught an exception!");
        }

        return sal_True;
    }

    //---------------------------------------------------------------------
    IMPL_LINK( OTableSelectionPage, OnSearchClicked, PushButton*, /*_pButton*/ )
    {
        ::sfx2::FileDialogHelper aFileDlg(
                ui::dialogs::TemplateDescription::FILEOPEN_READONLY_VERSION, 0);
        aFileDlg.SetDisplayDirectory( SvtPathOptions().GetWorkPath() );

        const SfxFilter* pFilter = SfxFilter::GetFilterByName(OUString("StarOffice XML (Base)"));
        OSL_ENSURE(pFilter,"Filter: StarOffice XML (Base) could not be found!");
        if ( pFilter )
        {
            aFileDlg.AddFilter(pFilter->GetUIName(),pFilter->GetDefaultExtension());
        }

        if (0 == aFileDlg.Execute())
        {
            String sDataSourceName = aFileDlg.GetPath();
            ::svt::OFileNotation aFileNotation(sDataSourceName);
            sDataSourceName = aFileNotation.get(::svt::OFileNotation::N_SYSTEM);
            m_aDatasource.InsertEntry(sDataSourceName);
            m_aDatasource.SelectEntry(sDataSourceName);
            LINK(this, OTableSelectionPage, OnListboxSelection).Call(&m_aDatasource);
        }
        return 0L;
    }
    //---------------------------------------------------------------------
    IMPL_LINK( OTableSelectionPage, OnListboxDoubleClicked, ListBox*, _pBox )
    {
        if (_pBox->GetSelectEntryCount())
            getDialog()->travelNext();
        return 0L;
    }

    //---------------------------------------------------------------------
    IMPL_LINK( OTableSelectionPage, OnListboxSelection, ListBox*, _pBox )
    {
        if (&m_aDatasource == _pBox)
        {   // new data source selected
            implFillTables();
        }
        else
        {
        }

        updateDialogTravelUI();

        return 0L;
    }

    //---------------------------------------------------------------------
    namespace
    {
        void    lcl_fillEntries( ListBox& _rListBox, const Sequence< OUString >& _rNames, const Image& _rImage, sal_Int32 _nCommandType )
        {
            const OUString* pNames = _rNames.getConstArray();
            const OUString* pNamesEnd = _rNames.getConstArray() + _rNames.getLength();
            sal_uInt16 nPos = 0;
            while ( pNames != pNamesEnd )
            {
                nPos = _rListBox.InsertEntry( *pNames++, _rImage );
                _rListBox.SetEntryData( nPos, reinterpret_cast< void* >( _nCommandType ) );
            }
        }
    }

    //---------------------------------------------------------------------
    void OTableSelectionPage::implFillTables(const Reference< XConnection >& _rxConn)
    {
        m_aTable.Clear();

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
                OUString sCurrentDatasource = m_aDatasource.GetSelectEntry();
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
                        Reference< XInteractionHandler > xHandler = getDialog()->getInteractionHandler(this);
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
                Reference< XInteractionHandler > xHandler = getDialog()->getInteractionHandler(this);
                if ( xHandler.is() )
                    xHandler->handle(xRequest);
            }
            catch(const Exception&) { }
            return;
        }

        Image aTableImage, aQueryImage;
        {
            ::svt::OLocalResourceAccess aLocalResAccess( ModuleRes( RID_PAGE_TABLESELECTION ), RSC_TABPAGE );

            aTableImage = Image( ModuleRes( IMG_TABLE ) );
            aQueryImage = Image( ModuleRes( IMG_QUERY ) );
        }
        lcl_fillEntries( m_aTable, aTableNames, aTableImage, CommandType::TABLE );
        lcl_fillEntries( m_aTable, aQueryNames, aQueryImage, CommandType::QUERY );
    }

    //---------------------------------------------------------------------
    void OTableSelectionPage::implCollectDatasource()
    {
        try
        {
            m_xDSContext = getContext().xDatasourceContext;
            if (m_xDSContext.is())
                fillListBox(m_aDatasource, m_xDSContext->getElementNames());
        }
        catch (const Exception&)
        {
            OSL_FAIL("OTableSelectionPage::implCollectDatasource: could not collect the data source names!");
        }
    }

    //=====================================================================
    //= OMaybeListSelectionPage
    //=====================================================================
    //---------------------------------------------------------------------
    OMaybeListSelectionPage::OMaybeListSelectionPage( OControlWizard* _pParent, const ResId& _rId )
        :OControlWizardPage(_pParent, _rId)
        ,m_pYes(NULL)
        ,m_pNo(NULL)
        ,m_pList(NULL)
    {
    }

    //---------------------------------------------------------------------
    void OMaybeListSelectionPage::announceControls(RadioButton& _rYesButton, RadioButton& _rNoButton, ListBox& _rSelection)
    {
        m_pYes = &_rYesButton;
        m_pNo = &_rNoButton;
        m_pList = &_rSelection;

        m_pYes->SetClickHdl(LINK(this, OMaybeListSelectionPage, OnRadioSelected));
        m_pNo->SetClickHdl(LINK(this, OMaybeListSelectionPage, OnRadioSelected));
        implEnableWindows();
    }

    //---------------------------------------------------------------------
    IMPL_LINK( OMaybeListSelectionPage, OnRadioSelected, RadioButton*, /*NOTINTERESTEDIN*/ )
    {
        implEnableWindows();
        return 0L;
    }

    //---------------------------------------------------------------------
    void OMaybeListSelectionPage::implInitialize(const String& _rSelection)
    {
        DBG_ASSERT(m_pYes, "OMaybeListSelectionPage::implInitialize: no controls announced!");
        sal_Bool bIsSelection = (0 != _rSelection.Len());
        m_pYes->Check(bIsSelection);
        m_pNo->Check(!bIsSelection);
        m_pList->Enable(bIsSelection);

        m_pList->SelectEntry(bIsSelection ? _rSelection : String());
    }

    //---------------------------------------------------------------------
    void OMaybeListSelectionPage::implCommit(String& _rSelection)
    {
        _rSelection = m_pYes->IsChecked() ? m_pList->GetSelectEntry() : OUString();
    }

    //---------------------------------------------------------------------
    void OMaybeListSelectionPage::implEnableWindows()
    {
        m_pList->Enable(m_pYes->IsChecked());
    }

    //---------------------------------------------------------------------
    void OMaybeListSelectionPage::ActivatePage()
    {
        OControlWizardPage::ActivatePage();

        DBG_ASSERT(m_pYes, "OMaybeListSelectionPage::ActivatePage: no controls announced!");
        if (m_pYes->IsChecked())
            m_pList->GrabFocus();
        else
            m_pNo->GrabFocus();
    }

    //=====================================================================
    //= ODBFieldPage
    //=====================================================================
    //---------------------------------------------------------------------
    ODBFieldPage::ODBFieldPage( OControlWizard* _pParent )
        :OMaybeListSelectionPage(_pParent, ModuleRes(RID_PAGE_OPTION_DBFIELD))
        ,m_aFrame           (this, ModuleRes(FL_DATABASEFIELD_EXPL))
        ,m_aDescription     (this, ModuleRes(FT_DATABASEFIELD_EXPL))
        ,m_aQuestion        (this, ModuleRes(FT_DATABASEFIELD_QUEST))
        ,m_aStoreYes        (this, ModuleRes(RB_STOREINFIELD_YES))
        ,m_aStoreNo         (this, ModuleRes(LB_STOREINFIELD))
        ,m_aStoreWhere      (this, ModuleRes(RB_STOREINFIELD_NO))
    {
        FreeResource();
        announceControls(m_aStoreYes, m_aStoreNo, m_aStoreWhere);
        m_aStoreWhere.SetDropDownLineCount(10);
    }

    //---------------------------------------------------------------------
    void ODBFieldPage::initializePage()
    {
        OMaybeListSelectionPage::initializePage();

        // fill the fields page
        fillListBox(m_aStoreWhere, getContext().aFieldNames);

        implInitialize(getDBFieldSetting());
    }

    //---------------------------------------------------------------------
    sal_Bool ODBFieldPage::commitPage( ::svt::WizardTypes::CommitPageReason _eReason )
    {
        if (!OMaybeListSelectionPage::commitPage(_eReason))
            return sal_False;

        implCommit(getDBFieldSetting());

        return sal_True;
    }

//.........................................................................
}   // namespace dbp
//.........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
