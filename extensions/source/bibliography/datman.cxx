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

#include <sal/config.h>

#include <o3tl/any.hxx>
#include <sal/log.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/sdbc/ResultSetType.hpp>
#include <com/sun/star/sdbc/ResultSetConcurrency.hpp>
#include <com/sun/star/sdbcx/XRowLocate.hpp>
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdb/XSingleSelectQueryComposer.hpp>
#include <com/sun/star/sdbc/XDatabaseMetaData.hpp>
#include <com/sun/star/sdbc/XDataSource.hpp>
#include <com/sun/star/sdb/CommandType.hpp>
#include <com/sun/star/sdb/DatabaseContext.hpp>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/sdb/XCompletedConnection.hpp>
#include <com/sun/star/sdbc/SQLException.hpp>
#include <com/sun/star/task/InteractionHandler.hpp>
#include <com/sun/star/form/ListSourceType.hpp>
#include <com/sun/star/form/XLoadable.hpp>
#include <com/sun/star/form/runtime/FormController.hpp>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/form/XGridColumnFactory.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <tools/debug.hxx>
#include <tools/urlobj.hxx>
#include <vcl/weld.hxx>
#include "datman.hxx"
#include "bibresid.hxx"
#include "bibmod.hxx"
#include "bibview.hxx"
#include "toolbar.hxx"
#include "bibconfig.hxx"
#include "bibbeam.hxx"
#include "general.hxx"
#include <strings.hrc>
#include <helpids.h>
#include <connectivity/dbtools.hxx>
#include <memory>

using namespace ::com::sun::star;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::form;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::lang;

// PropertyNames
constexpr OUStringLiteral FM_PROP_LABEL = u"Label";
constexpr OUString FM_PROP_CONTROLSOURCE = u"DataField"_ustr;
constexpr OUStringLiteral FM_PROP_NAME = u"Name";

static Reference< XConnection > getConnection(const OUString& _rURL)
{
    // first get the sdb::DataSource corresponding to the url
    Reference< XDataSource >    xDataSource;
    // is it a favorite title ?
    Reference<XComponentContext>  xContext = comphelper::getProcessComponentContext();
    Reference< XDatabaseContext >  xNamingContext = DatabaseContext::create(xContext);
    if (xNamingContext->hasByName(_rURL))
    {
        DBG_ASSERT(xNamingContext.is(), "::getDataSource : no NamingService interface on the sdb::DatabaseAccessContext !");
        try
        {
            xDataSource.set(xNamingContext->getRegisteredObject(_rURL), UNO_QUERY);
        }
        catch (const Exception&)
        {
            TOOLS_WARN_EXCEPTION("extensions.biblio", "");
        }
    }
    // build the connection from the data source
    Reference< XConnection > xConn;
    if (xDataSource.is())
    {
        // need user/pwd for this
        Reference< XCompletedConnection > xComplConn(xDataSource, UNO_QUERY);
        try
        {
            Reference<task::XInteractionHandler> xIHdl( task::InteractionHandler::createWithParent(xContext, nullptr), UNO_QUERY_THROW);
            xConn = xComplConn->connectWithCompletion(xIHdl);
        }
        catch (const SQLException&)
        {
            // TODO : a real error handling
        }
        catch (const Exception&)
        {
        }
    }
    return xConn;
}

static Reference< XConnection >    getConnection(const Reference< XInterface > & xRowSet)
{
    Reference< XConnection >    xConn;
    try
    {
        Reference< XPropertySet >  xFormProps(xRowSet, UNO_QUERY);
        if (!xFormProps.is())
            return xConn;

        xConn.set(xFormProps->getPropertyValue("ActiveConnection"), UNO_QUERY);
        if (!xConn.is())
        {
            SAL_INFO("extensions.biblio", "no active connection");
        }
    }
    catch (const Exception&)
    {
        TOOLS_WARN_EXCEPTION("extensions.biblio", "");
    }

    return xConn;
}

static Reference< XNameAccess >  getColumns(const Reference< XForm > & _rxForm)
{
    Reference< XNameAccess >  xReturn;
    // check if the form is alive
    Reference< XColumnsSupplier >  xSupplyCols( _rxForm, UNO_QUERY );
    if (xSupplyCols.is())
        xReturn = xSupplyCols->getColumns();

    if (!xReturn.is() || !xReturn->getElementNames().hasElements())
    {   // no...
        xReturn = nullptr;
        // -> get the table the form is bound to and ask it for their columns
        Reference< XTablesSupplier >  xSupplyTables( getConnection( _rxForm ), UNO_QUERY );
        Reference< XPropertySet >  xFormProps( _rxForm, UNO_QUERY );
        if (xFormProps.is() && xSupplyTables.is())
        {
            try
            {
                DBG_ASSERT(*o3tl::forceAccess<sal_Int32>(xFormProps->getPropertyValue("CommandType")) == CommandType::TABLE,
                    "::getColumns : invalid form (has no table as data source) !");
                OUString sTable;
                xFormProps->getPropertyValue("Command") >>= sTable;
                Reference< XNameAccess >  xTables = xSupplyTables->getTables();
                if (xTables.is() && xTables->hasByName(sTable))
                    xSupplyCols.set(xTables->getByName(sTable), UNO_QUERY);
                if (xSupplyCols.is())
                    xReturn = xSupplyCols->getColumns();
            }
            catch (const Exception&)
            {
                TOOLS_WARN_EXCEPTION( "extensions.biblio", "::getColumns");
            }

        }
    }
    return xReturn;
}

namespace {

class MappingDialog_Impl : public weld::GenericDialogController
{
    BibDataManager* pDatMan;

    OUString        sNone;
    bool        bModified;

    std::unique_ptr<weld::Button> m_xOKBT;
    std::unique_ptr<weld::ComboBox> m_xIdentifierLB;
    std::unique_ptr<weld::ComboBox> m_xAuthorityTypeLB;
    std::unique_ptr<weld::ComboBox> m_xAuthorLB;
    std::unique_ptr<weld::ComboBox> m_xTitleLB;
    std::unique_ptr<weld::ComboBox> m_xMonthLB;
    std::unique_ptr<weld::ComboBox> m_xYearLB;
    std::unique_ptr<weld::ComboBox> m_xISBNLB;
    std::unique_ptr<weld::ComboBox> m_xBooktitleLB;
    std::unique_ptr<weld::ComboBox> m_xChapterLB;
    std::unique_ptr<weld::ComboBox> m_xEditionLB;
    std::unique_ptr<weld::ComboBox> m_xEditorLB;
    std::unique_ptr<weld::ComboBox> m_xHowpublishedLB;
    std::unique_ptr<weld::ComboBox> m_xInstitutionLB;
    std::unique_ptr<weld::ComboBox> m_xJournalLB;
    std::unique_ptr<weld::ComboBox> m_xNoteLB;
    std::unique_ptr<weld::ComboBox> m_xAnnoteLB;
    std::unique_ptr<weld::ComboBox> m_xNumberLB;
    std::unique_ptr<weld::ComboBox> m_xOrganizationsLB;
    std::unique_ptr<weld::ComboBox> m_xPagesLB;
    std::unique_ptr<weld::ComboBox> m_xPublisherLB;
    std::unique_ptr<weld::ComboBox> m_xAddressLB;
    std::unique_ptr<weld::ComboBox> m_xSchoolLB;
    std::unique_ptr<weld::ComboBox> m_xSeriesLB;
    std::unique_ptr<weld::ComboBox> m_xReportTypeLB;
    std::unique_ptr<weld::ComboBox> m_xVolumeLB;
    std::unique_ptr<weld::ComboBox> m_xURLLB;
    std::unique_ptr<weld::ComboBox> m_xCustom1LB;
    std::unique_ptr<weld::ComboBox> m_xCustom2LB;
    std::unique_ptr<weld::ComboBox> m_xCustom3LB;
    std::unique_ptr<weld::ComboBox> m_xCustom4LB;
    std::unique_ptr<weld::ComboBox> m_xCustom5LB;
    std::unique_ptr<weld::ComboBox> m_xLocalURLLB;
    weld::ComboBox* aListBoxes[COLUMN_COUNT];

    DECL_LINK(OkHdl, weld::Button&, void);
    DECL_LINK(ListBoxSelectHdl, weld::ComboBox&, void);

public:
    MappingDialog_Impl(weld::Window* pParent, BibDataManager* pDatMan);
};

}

static sal_uInt16 lcl_FindLogicalName(BibConfig const * pConfig ,
                                      std::u16string_view rLogicalColumnName)
{
    for(sal_uInt16 i = 0; i < COLUMN_COUNT; i++)
    {
        if(rLogicalColumnName == pConfig->GetDefColumnName(i))
            return i;
    }
    return USHRT_MAX;
}

MappingDialog_Impl::MappingDialog_Impl(weld::Window* pParent, BibDataManager* pMan)
    : GenericDialogController(pParent, "modules/sbibliography/ui/mappingdialog.ui", "MappingDialog")
    , pDatMan(pMan)
    , sNone(BibResId(RID_BIB_STR_NONE))
    , bModified(false)
    , m_xOKBT(m_xBuilder->weld_button("ok"))
    , m_xIdentifierLB(m_xBuilder->weld_combo_box("identifierCombobox"))
    , m_xAuthorityTypeLB(m_xBuilder->weld_combo_box("authorityTypeCombobox"))
    , m_xAuthorLB(m_xBuilder->weld_combo_box("authorCombobox"))
    , m_xTitleLB(m_xBuilder->weld_combo_box("titleCombobox"))
    , m_xMonthLB(m_xBuilder->weld_combo_box("monthCombobox"))
    , m_xYearLB(m_xBuilder->weld_combo_box("yearCombobox"))
    , m_xISBNLB(m_xBuilder->weld_combo_box("ISBNCombobox"))
    , m_xBooktitleLB(m_xBuilder->weld_combo_box("bookTitleCombobox"))
    , m_xChapterLB(m_xBuilder->weld_combo_box("chapterCombobox"))
    , m_xEditionLB(m_xBuilder->weld_combo_box("editionCombobox"))
    , m_xEditorLB(m_xBuilder->weld_combo_box("editorCombobox"))
    , m_xHowpublishedLB(m_xBuilder->weld_combo_box("howPublishedCombobox"))
    , m_xInstitutionLB(m_xBuilder->weld_combo_box("institutionCombobox"))
    , m_xJournalLB(m_xBuilder->weld_combo_box("journalCombobox"))
    , m_xNoteLB(m_xBuilder->weld_combo_box("noteCombobox"))
    , m_xAnnoteLB(m_xBuilder->weld_combo_box("annoteCombobox"))
    , m_xNumberLB(m_xBuilder->weld_combo_box("numberCombobox"))
    , m_xOrganizationsLB(m_xBuilder->weld_combo_box("organizationCombobox"))
    , m_xPagesLB(m_xBuilder->weld_combo_box("pagesCombobox"))
    , m_xPublisherLB(m_xBuilder->weld_combo_box("publisherCombobox"))
    , m_xAddressLB(m_xBuilder->weld_combo_box("addressCombobox"))
    , m_xSchoolLB(m_xBuilder->weld_combo_box("schoolCombobox"))
    , m_xSeriesLB(m_xBuilder->weld_combo_box("seriesCombobox"))
    , m_xReportTypeLB(m_xBuilder->weld_combo_box("reportTypeCombobox"))
    , m_xVolumeLB(m_xBuilder->weld_combo_box("volumeCombobox"))
    , m_xURLLB(m_xBuilder->weld_combo_box("URLCombobox"))
    , m_xCustom1LB(m_xBuilder->weld_combo_box("custom1Combobox"))
    , m_xCustom2LB(m_xBuilder->weld_combo_box("custom2Combobox"))
    , m_xCustom3LB(m_xBuilder->weld_combo_box("custom3Combobox"))
    , m_xCustom4LB(m_xBuilder->weld_combo_box("custom4Combobox"))
    , m_xCustom5LB(m_xBuilder->weld_combo_box("custom5Combobox"))
    , m_xLocalURLLB(m_xBuilder->weld_combo_box("LocalURLCombobox"))
{
    m_xOKBT->connect_clicked(LINK(this, MappingDialog_Impl, OkHdl));
    OUString sTitle = m_xDialog->get_title();
    sTitle = sTitle.replaceFirst("%1", pDatMan->getActiveDataTable());
    m_xDialog->set_title(sTitle);

    aListBoxes[0] = m_xIdentifierLB.get();
    aListBoxes[1] = m_xAuthorityTypeLB.get();
    aListBoxes[2] = m_xAuthorLB.get();
    aListBoxes[3] = m_xTitleLB.get();
    aListBoxes[4] = m_xYearLB.get();
    aListBoxes[5] = m_xISBNLB.get();
    aListBoxes[6] = m_xBooktitleLB.get();
    aListBoxes[7] = m_xChapterLB.get();
    aListBoxes[8] = m_xEditionLB.get();
    aListBoxes[9] = m_xEditorLB.get();
    aListBoxes[10] = m_xHowpublishedLB.get();
    aListBoxes[11] = m_xInstitutionLB.get();
    aListBoxes[12] = m_xJournalLB.get();
    aListBoxes[13] = m_xMonthLB.get();
    aListBoxes[14] = m_xNoteLB.get();
    aListBoxes[15] = m_xAnnoteLB.get();
    aListBoxes[16] = m_xNumberLB.get();
    aListBoxes[17] = m_xOrganizationsLB.get();
    aListBoxes[18] = m_xPagesLB.get();
    aListBoxes[19] = m_xPublisherLB.get();
    aListBoxes[20] = m_xAddressLB.get();
    aListBoxes[21] = m_xSchoolLB.get();
    aListBoxes[22] = m_xSeriesLB.get();
    aListBoxes[23] = m_xReportTypeLB.get();
    aListBoxes[24] = m_xVolumeLB.get();
    aListBoxes[25] = m_xURLLB.get();
    aListBoxes[26] = m_xCustom1LB.get();
    aListBoxes[27] = m_xCustom2LB.get();
    aListBoxes[28] = m_xCustom3LB.get();
    aListBoxes[29] = m_xCustom4LB.get();
    aListBoxes[30] = m_xCustom5LB.get();
    aListBoxes[31] = m_xLocalURLLB.get();

    aListBoxes[0]->append_text(sNone);
    Reference< XNameAccess >  xFields = getColumns( pDatMan->getForm() );
    DBG_ASSERT(xFields.is(), "MappingDialog_Impl::MappingDialog_Impl : gave me an invalid form !");
    if (xFields.is())
    {
        const Sequence<OUString> aFieldNames = xFields->getElementNames();
        for(const OUString& rName : aFieldNames)
            aListBoxes[0]->append_text(rName);
    }

    Link<weld::ComboBox&,void> aLnk = LINK(this, MappingDialog_Impl, ListBoxSelectHdl);

    aListBoxes[0]->set_active(0);
    aListBoxes[0]->connect_changed(aLnk);
    for(sal_uInt16 i = 1; i < COLUMN_COUNT; i++)
    {
        for(sal_Int32 j = 0, nEntryCount = aListBoxes[0]->get_count(); j < nEntryCount; ++j)
            aListBoxes[i]->append_text(aListBoxes[0]->get_text(j));
        aListBoxes[i]->set_active(0);
        aListBoxes[i]->connect_changed(aLnk);
    }
    BibConfig* pConfig = BibModul::GetConfig();
    BibDBDescriptor aDesc;
    aDesc.sDataSource = pDatMan->getActiveDataSource();
    aDesc.sTableOrQuery = pDatMan->getActiveDataTable();
    aDesc.nCommandType = CommandType::TABLE;
    const Mapping* pMapping = pConfig->GetMapping(aDesc);
    if(pMapping)
    {
        for(const auto & aColumnPair : pMapping->aColumnPairs)
        {
            sal_uInt16 nListBoxIndex = lcl_FindLogicalName( pConfig, aColumnPair.sLogicalColumnName);
            if(nListBoxIndex < COLUMN_COUNT)
            {
                aListBoxes[nListBoxIndex]->set_active_text(aColumnPair.sRealColumnName);
            }
        }
    }
}

IMPL_LINK(MappingDialog_Impl, ListBoxSelectHdl, weld::ComboBox&, rListBox, void)
{
    const sal_Int32 nEntryPos = rListBox.get_active();
    if (0 < nEntryPos)
    {
        for(auto & pListBoxe : aListBoxes)
        {
            if (&rListBox != pListBoxe && pListBoxe->get_active() == nEntryPos)
                pListBoxe->set_active(0);
        }
    }
    bModified = true;
}

IMPL_LINK_NOARG(MappingDialog_Impl, OkHdl, weld::Button&, void)
{
    if(bModified)
    {
        Mapping aNew;
        aNew.sTableName = pDatMan->getActiveDataTable();
        aNew.sURL = pDatMan->getActiveDataSource();

        sal_uInt16 nWriteIndex = 0;
        BibConfig* pConfig = BibModul::GetConfig();
        for(sal_uInt16 nEntry = 0; nEntry < COLUMN_COUNT; nEntry++)
        {
            OUString sSel = aListBoxes[nEntry]->get_active_text();
            if(sSel != sNone)
            {
                aNew.aColumnPairs[nWriteIndex].sRealColumnName = sSel;
                aNew.aColumnPairs[nWriteIndex].sLogicalColumnName = pConfig->GetDefColumnName(nEntry);
                nWriteIndex++;
            }
        }
        BibDBDescriptor aDesc;
        aDesc.sDataSource = pDatMan->getActiveDataSource();
        aDesc.sTableOrQuery = pDatMan->getActiveDataTable();
        aDesc.nCommandType = CommandType::TABLE;
        pDatMan->ResetIdentifierMapping();
        pConfig->SetMapping(aDesc, &aNew);
    }
    m_xDialog->response(bModified ? RET_OK : RET_CANCEL);
}

namespace {

class DBChangeDialog_Impl : public weld::GenericDialogController
{
    DBChangeDialogConfig_Impl aConfig;

    std::unique_ptr<weld::TreeView> m_xSelectionLB;

    DECL_LINK(DoubleClickHdl, weld::TreeView&, bool);
public:
    DBChangeDialog_Impl(weld::Window* pParent, const BibDataManager* pMan);

    OUString     GetCurrentURL()const;
};

}

DBChangeDialog_Impl::DBChangeDialog_Impl(weld::Window* pParent, const BibDataManager* pDatMan )
    : GenericDialogController(pParent, "modules/sbibliography/ui/choosedatasourcedialog.ui", "ChooseDataSourceDialog")
    , m_xSelectionLB(m_xBuilder->weld_tree_view("treeview"))
{
    m_xSelectionLB->set_size_request(-1, m_xSelectionLB->get_height_rows(6));
    m_xSelectionLB->connect_row_activated(LINK(this, DBChangeDialog_Impl, DoubleClickHdl));
    m_xSelectionLB->make_sorted();

    try
    {
        OUString sActiveSource = pDatMan->getActiveDataSource();
        for (const OUString& rSourceName : aConfig.GetDataSourceNames())
            m_xSelectionLB->append_text(rSourceName);
        m_xSelectionLB->select_text(sActiveSource);
    }
    catch (const Exception&)
    {
        TOOLS_WARN_EXCEPTION("extensions.biblio", "");
    }
}

IMPL_LINK_NOARG(DBChangeDialog_Impl, DoubleClickHdl, weld::TreeView&, bool)
{
    m_xDialog->response(RET_OK);
    return true;
}

OUString  DBChangeDialog_Impl::GetCurrentURL()const
{
    return m_xSelectionLB->get_selected_text();
}

// XDispatchProvider
BibInterceptorHelper::BibInterceptorHelper( const ::bib::BibBeamer* pBibBeamer, css::uno::Reference< css::frame::XDispatch > const & xDispatch)
{
    if( pBibBeamer )
    {
        xInterception = pBibBeamer->getDispatchProviderInterception();
        if( xInterception.is() )
            xInterception->registerDispatchProviderInterceptor( this );
    }
    if( xDispatch.is() )
        xFormDispatch = xDispatch;
}

BibInterceptorHelper::~BibInterceptorHelper( )
{
}

void BibInterceptorHelper::ReleaseInterceptor()
{
    if ( xInterception.is() )
        xInterception->releaseDispatchProviderInterceptor( this );
    xInterception.clear();
}

css::uno::Reference< css::frame::XDispatch > SAL_CALL
    BibInterceptorHelper::queryDispatch( const css::util::URL& aURL, const OUString& aTargetFrameName, sal_Int32 nSearchFlags )
{
    Reference< XDispatch > xReturn;

    OUString aCommand( aURL.Path );
    if ( aCommand == "FormSlots/ConfirmDeletion" )
        xReturn = xFormDispatch;
    else
        if ( xSlaveDispatchProvider.is() )
            xReturn = xSlaveDispatchProvider->queryDispatch( aURL, aTargetFrameName, nSearchFlags);

    return xReturn;
}

css::uno::Sequence< css::uno::Reference< css::frame::XDispatch > > SAL_CALL
    BibInterceptorHelper::queryDispatches( const css::uno::Sequence< css::frame::DispatchDescriptor >& aDescripts )
{
    Sequence< Reference< XDispatch> > aReturn( aDescripts.getLength() );
    Reference< XDispatch >* pReturn = aReturn.getArray();
    for ( const DispatchDescriptor& rDescript : aDescripts )
    {
        *pReturn++ = queryDispatch( rDescript.FeatureURL, rDescript.FrameName, rDescript.SearchFlags );
    }
    return aReturn;
}

// XDispatchProviderInterceptor
css::uno::Reference< css::frame::XDispatchProvider > SAL_CALL
    BibInterceptorHelper::getSlaveDispatchProvider(  )
{
    return xSlaveDispatchProvider;
}

void SAL_CALL BibInterceptorHelper::setSlaveDispatchProvider( const css::uno::Reference< css::frame::XDispatchProvider >& xNewSlaveDispatchProvider )
{
    xSlaveDispatchProvider = xNewSlaveDispatchProvider;
}

css::uno::Reference< css::frame::XDispatchProvider > SAL_CALL
    BibInterceptorHelper::getMasterDispatchProvider(  )
{
    return xMasterDispatchProvider;
}

void SAL_CALL BibInterceptorHelper::setMasterDispatchProvider( const css::uno::Reference< css::frame::XDispatchProvider >& xNewMasterDispatchProvider )
{
    xMasterDispatchProvider = xNewMasterDispatchProvider;
}


constexpr OUStringLiteral gGridName(u"theGrid");

BibDataManager::BibDataManager()
    :pBibView( nullptr )
    ,pToolbar(nullptr)
{
}


BibDataManager::~BibDataManager()
{
    Reference< XLoadable >      xLoad( m_xForm, UNO_QUERY );
    Reference< XPropertySet >   xPrSet( m_xForm, UNO_QUERY );
    Reference< XComponent >     xComp( m_xForm, UNO_QUERY );
    if ( m_xForm.is() )
    {
        Reference< XComponent >  xConnection;
        xPrSet->getPropertyValue("ActiveConnection") >>= xConnection;
        if (xLoad.is())
            xLoad->unload();
        if (xComp.is())
            xComp->dispose();
        if(xConnection.is())
            xConnection->dispose();
        m_xForm = nullptr;
    }
    if( m_xInterceptorHelper.is() )
    {
        m_xInterceptorHelper->ReleaseInterceptor();
        m_xInterceptorHelper.clear();
    }
}

void BibDataManager::InsertFields(const Reference< XFormComponent > & _rxGrid)
{
    if ( !_rxGrid.is() )
        return;

    try
    {
        Reference< XNameContainer > xColContainer( _rxGrid, UNO_QUERY );
        // remove the old fields
        if ( xColContainer->hasElements() )
        {
            const Sequence<OUString> aOldNames = xColContainer->getElementNames();
            for ( const OUString& rName : aOldNames )
                xColContainer->removeByName( rName );
        }

        Reference< XNameAccess >  xFields = getColumns( m_xForm );
        if (!xFields.is())
            return;

        Reference< XGridColumnFactory > xColFactory( _rxGrid, UNO_QUERY );

        Reference< XPropertySet >  xField;

        const Sequence<OUString> aFieldNames = xFields->getElementNames();
        for ( const OUString& rField : aFieldNames )
        {
            xFields->getByName( rField ) >>= xField;

            OUString sCurrentModelType;
            sal_Int32 nType = 0;
            bool bIsFormatted           = false;
            bool bFormattedIsNumeric    = true;
            xField->getPropertyValue("Type") >>= nType;
            switch(nType)
            {
                case DataType::BIT:
                case DataType::BOOLEAN:
                    sCurrentModelType = "CheckBox";
                    break;

                case DataType::BINARY:
                case DataType::VARBINARY:
                case DataType::LONGVARBINARY:
                case DataType::BLOB:
                    sCurrentModelType = "TextField";
                    break;

                case DataType::VARCHAR:
                case DataType::LONGVARCHAR:
                case DataType::CHAR:
                case DataType::CLOB:
                    bFormattedIsNumeric = false;
                    [[fallthrough]];
                default:
                    sCurrentModelType = "FormattedField";
                    bIsFormatted = true;
                    break;
            }

            Reference< XPropertySet >  xCurrentCol = xColFactory->createColumn(sCurrentModelType);
            if (bIsFormatted)
            {
                OUString sFormatKey("FormatKey");
                xCurrentCol->setPropertyValue(sFormatKey, xField->getPropertyValue(sFormatKey));
                Any aFormatted(bFormattedIsNumeric);
                xCurrentCol->setPropertyValue("TreatAsNumber", aFormatted);
            }
            Any aColName( rField );
            xCurrentCol->setPropertyValue(FM_PROP_CONTROLSOURCE,    aColName);
            xCurrentCol->setPropertyValue(FM_PROP_LABEL, aColName);

            xColContainer->insertByName( rField, Any( xCurrentCol ) );
        }
    }
    catch (const Exception&)
    {
        TOOLS_WARN_EXCEPTION("extensions.biblio", "");
    }
}

Reference< awt::XControlModel > BibDataManager::updateGridModel()
{
    return updateGridModel( m_xForm );
}

Reference< awt::XControlModel > const & BibDataManager::updateGridModel(const Reference< XForm > & xDbForm)
{
    try
    {
        Reference< XPropertySet >  aFormPropSet( xDbForm, UNO_QUERY );
        OUString sName;
        aFormPropSet->getPropertyValue("Command") >>= sName;

        if ( !m_xGridModel.is() )
        {
            m_xGridModel = createGridModel( gGridName );

            Reference< XNameContainer >  xNameCont(xDbForm, UNO_QUERY);
            xNameCont->insertByName( sName, Any( m_xGridModel ) );
        }

        // insert the fields
        Reference< XFormComponent > xFormComp( m_xGridModel, UNO_QUERY );
        InsertFields( xFormComp );
    }
    catch (const Exception&)
    {
        OSL_FAIL("::updateGridModel: something went wrong !");
    }

    return m_xGridModel;
}

Reference< XForm >  BibDataManager::createDatabaseForm(BibDBDescriptor& rDesc)
{
    Reference< XForm >  xResult;
    try
    {
        Reference< XMultiServiceFactory >  xMgr = comphelper::getProcessServiceFactory();
        m_xForm.set( xMgr->createInstance( "com.sun.star.form.component.Form" ), UNO_QUERY );

        Reference< XPropertySet >  aPropertySet( m_xForm, UNO_QUERY );

        aDataSourceURL = rDesc.sDataSource;
        if(aPropertySet.is())
        {
            Any aVal;
            aVal <<= sal_Int32(ResultSetType::SCROLL_INSENSITIVE);
            aPropertySet->setPropertyValue("ResultSetType",aVal );
            aVal <<= sal_Int32(ResultSetConcurrency::READ_ONLY);
            aPropertySet->setPropertyValue("ResultSetConcurrency", aVal);

            //Caching for Performance
            aVal <<= sal_Int32(50);
            aPropertySet->setPropertyValue("FetchSize", aVal);

            Reference< XConnection >    xConnection = getConnection(rDesc.sDataSource);
            aVal <<= xConnection;
            aPropertySet->setPropertyValue("ActiveConnection", aVal);

            Reference< XTablesSupplier >  xSupplyTables(xConnection, UNO_QUERY);
            Reference< XNameAccess >  xTables = xSupplyTables.is() ?
                                xSupplyTables->getTables() : Reference< XNameAccess > ();

            Sequence< OUString > aTableNameSeq;
            if (xTables.is())
                aTableNameSeq = xTables->getElementNames();

            if(aTableNameSeq.hasElements())
            {
                if(!rDesc.sTableOrQuery.isEmpty())
                    aActiveDataTable = rDesc.sTableOrQuery;
                else
                {
                    rDesc.sTableOrQuery = aActiveDataTable = aTableNameSeq[0];
                    rDesc.nCommandType = CommandType::TABLE;
                }

                aVal <<= aActiveDataTable;
                aPropertySet->setPropertyValue("Command", aVal);
                aVal <<= rDesc.nCommandType;
                aPropertySet->setPropertyValue("CommandType", aVal);


                Reference< XDatabaseMetaData >  xMetaData = xConnection->getMetaData();
                aQuoteChar = xMetaData->getIdentifierQuoteString();

                Reference< XMultiServiceFactory > xFactory(xConnection, UNO_QUERY);
                if ( xFactory.is() )
                    m_xParser.set( xFactory->createInstance("com.sun.star.sdb.SingleSelectQueryComposer"), UNO_QUERY );

                OUString aString("SELECT * FROM ");

                OUString sCatalog, sSchema, sName;
                ::dbtools::qualifiedNameComponents( xMetaData, aActiveDataTable, sCatalog, sSchema, sName, ::dbtools::EComposeRule::InDataManipulation );
                aString += ::dbtools::composeTableNameForSelect( xConnection, sCatalog, sSchema, sName );

                m_xParser->setElementaryQuery(aString);
                BibConfig* pConfig = BibModul::GetConfig();
                pConfig->setQueryField(getQueryField());
                startQueryWith(pConfig->getQueryText());

                xResult = m_xForm;
            }
        }
    }
    catch (const Exception&)
    {
        OSL_FAIL("::createDatabaseForm: something went wrong !");
    }

    return xResult;
}

Sequence< OUString > BibDataManager::getDataSources() const
{
    Sequence< OUString > aTableNameSeq;

    try
    {
        Reference< XTablesSupplier >  xSupplyTables( getConnection( m_xForm ), UNO_QUERY );
        Reference< XNameAccess >  xTables;
        if (xSupplyTables.is())
            xTables = xSupplyTables->getTables();
        if (xTables.is())
            aTableNameSeq = xTables->getElementNames();
    }
    catch (const Exception&)
    {
        OSL_FAIL("::getDataSources: something went wrong !");
    }

    return aTableNameSeq;
}


void BibDataManager::setFilter(const OUString& rQuery)
{
    if(!m_xParser.is())
        return;
    try
    {
        m_xParser->setFilter( rQuery );
        OUString aQuery = m_xParser->getFilter();
        Reference< XPropertySet >  xFormProps( m_xForm, UNO_QUERY_THROW );
        xFormProps->setPropertyValue( "Filter", Any( aQuery ) );
        xFormProps->setPropertyValue( "ApplyFilter", Any( true ) );
        reload();
    }
    catch (const Exception&)
    {
        DBG_UNHANDLED_EXCEPTION("extensions.biblio");
    }


}

OUString BibDataManager::getFilter() const
{

    OUString aQueryString;
    try
    {
        Reference< XPropertySet > xFormProps( m_xForm, UNO_QUERY_THROW );
        OSL_VERIFY( xFormProps->getPropertyValue( "Filter"  ) >>= aQueryString );
    }
    catch (const Exception&)
    {
        DBG_UNHANDLED_EXCEPTION("extensions.biblio");
    }


    return aQueryString;

}

Sequence< OUString > BibDataManager::getQueryFields() const
{
    Sequence< OUString > aFieldSeq;
    Reference< XNameAccess >  xFields = getColumns( m_xForm );
    if (xFields.is())
        aFieldSeq = xFields->getElementNames();
    return aFieldSeq;
}

OUString BibDataManager::getQueryField() const
{
    BibConfig* pConfig = BibModul::GetConfig();
    OUString aFieldString = pConfig->getQueryField();
    if(aFieldString.isEmpty())
    {
        const Sequence< OUString > aSeq = getQueryFields();
        if(aSeq.hasElements())
        {
            aFieldString=aSeq[0];
        }
    }
    return aFieldString;
}

void BibDataManager::startQueryWith(const OUString& rQuery)
{
    BibConfig* pConfig = BibModul::GetConfig();
    pConfig->setQueryText( rQuery );

    OUString aQueryString;
    if(!rQuery.isEmpty())
    {
        aQueryString=aQuoteChar + getQueryField() + aQuoteChar + " like '";
        OUString sQuery = rQuery.replaceAll("?","_").replaceAll("*","%");
        aQueryString += sQuery + "%'";
    }
    setFilter(aQueryString);
}

void BibDataManager::setActiveDataSource(const OUString& rURL)
{
    OUString sTmp(aDataSourceURL);
    aDataSourceURL = rURL;

    Reference< XPropertySet >  aPropertySet( m_xForm, UNO_QUERY );
    if(!aPropertySet.is())
        return;

    unload();

    Reference< XComponent >  xOldConnection;
    aPropertySet->getPropertyValue("ActiveConnection") >>= xOldConnection;

    Reference< XConnection >    xConnection = getConnection(rURL);
    if(!xConnection.is())
    {
        aDataSourceURL = sTmp;
        return;
    }
    Any aVal; aVal <<= xConnection;
    aPropertySet->setPropertyValue("ActiveConnection", aVal);
    Reference< XMultiServiceFactory >   xFactory(xConnection, UNO_QUERY);
    if ( xFactory.is() )
        m_xParser.set( xFactory->createInstance("com.sun.star.sdb.SingleSelectQueryComposer"), UNO_QUERY );

    if(xOldConnection.is())
        xOldConnection->dispose();

    Sequence< OUString > aTableNameSeq;
    Reference< XTablesSupplier >  xSupplyTables(xConnection, UNO_QUERY);
    if(xSupplyTables.is())
    {
        Reference< XNameAccess >  xAccess = xSupplyTables->getTables();
        aTableNameSeq = xAccess->getElementNames();
    }
    if(aTableNameSeq.hasElements())
    {
        aActiveDataTable = aTableNameSeq[0];
        aVal <<= aActiveDataTable;
        aPropertySet->setPropertyValue("Command", aVal);
        aPropertySet->setPropertyValue("CommandType", Any(CommandType::TABLE));
        //Caching for Performance
        aVal <<= sal_Int32(50);
        aPropertySet->setPropertyValue("FetchSize", aVal);
        OUString aString("SELECT * FROM ");
        // quote the table name which may contain catalog.schema.table
        Reference<XDatabaseMetaData> xMetaData = xConnection->getMetaData();
        aQuoteChar = xMetaData->getIdentifierQuoteString();

        OUString sCatalog, sSchema, sName;
        ::dbtools::qualifiedNameComponents( xMetaData, aActiveDataTable, sCatalog, sSchema, sName, ::dbtools::EComposeRule::InDataManipulation );
        aString += ::dbtools::composeTableNameForSelect( xConnection, sCatalog, sSchema, sName );

        m_xParser->setElementaryQuery(aString);
        BibConfig* pConfig = BibModul::GetConfig();
        pConfig->setQueryField(getQueryField());
        startQueryWith(pConfig->getQueryText());
        setActiveDataTable(aActiveDataTable);
    }
    FeatureStateEvent aEvent;
    util::URL aURL;
    aEvent.IsEnabled  = true;
    aEvent.Requery    = false;
    aEvent.FeatureDescriptor = getActiveDataTable();

    aEvent.State <<= getDataSources();

    if(pToolbar)
    {
        aURL.Complete =".uno:Bib/source";
        aEvent.FeatureURL = aURL;
        pToolbar->statusChanged( aEvent );
    }

    updateGridModel();
    load();
}


void BibDataManager::setActiveDataTable(const OUString& rTable)
{
    ResetIdentifierMapping();
    try
    {
        Reference< XPropertySet >  aPropertySet( m_xForm, UNO_QUERY );

        if(aPropertySet.is())
        {
            Reference< XConnection >    xConnection = getConnection( m_xForm );
            Reference< XTablesSupplier >  xSupplyTables(xConnection, UNO_QUERY);
            Reference< XNameAccess > xAccess = xSupplyTables->getTables();

            for (auto& rTableName: xAccess->getElementNames())
            {
                if (rTable == rTableName)
                {
                    aActiveDataTable = rTable;
                    Any aVal; aVal <<= rTable;
                    aPropertySet->setPropertyValue( "Command", aVal );

                    Reference<XDatabaseMetaData> xMetaData = xConnection->getMetaData();
                    aQuoteChar = xMetaData->getIdentifierQuoteString();

                    Reference<XMultiServiceFactory> xFactory(xConnection, UNO_QUERY);
                    if (xFactory.is())
                        m_xParser.set( xFactory->createInstance("com.sun.star.sdb.SingleSelectQueryComposer"), UNO_QUERY );

                    OUString aString("SELECT * FROM ");

                    OUString sCatalog, sSchema, sName;
                    ::dbtools::qualifiedNameComponents( xMetaData, aActiveDataTable, sCatalog, sSchema, sName, ::dbtools::EComposeRule::InDataManipulation );
                    aString += ::dbtools::composeTableNameForSelect( xConnection, sCatalog, sSchema, sName );

                    m_xParser->setElementaryQuery(aString);

                    BibConfig* pConfig = BibModul::GetConfig();
                    pConfig->setQueryField(getQueryField());
                    startQueryWith(pConfig->getQueryText());

                    BibDBDescriptor aDesc;
                    aDesc.sDataSource = aDataSourceURL;
                    aDesc.sTableOrQuery = aActiveDataTable;
                    aDesc.nCommandType = CommandType::TABLE;
                    BibModul::GetConfig()->SetBibliographyURL(aDesc);

                    break;
                }
            }
        }
    }
    catch (const Exception&)
    {
        OSL_FAIL("::setActiveDataTable: something went wrong !");
    }
}


void SAL_CALL BibDataManager::load(  )
{
    if ( isLoaded() )
        // nothing to do
        return;

    Reference< XLoadable > xFormAsLoadable( m_xForm, UNO_QUERY );
    DBG_ASSERT( xFormAsLoadable.is() || !m_xForm.is(), "BibDataManager::load: invalid form!");
    if ( xFormAsLoadable.is() )
    {
        xFormAsLoadable->load();

        std::unique_lock g(m_aMutex);
        EventObject aEvt( static_cast< XWeak* >( this ) );
        m_aLoadListeners.notifyEach( g, &XLoadListener::loaded, aEvt );
    }
}


void SAL_CALL BibDataManager::unload(  )
{
    if ( !isLoaded() )
        // nothing to do
        return;

    Reference< XLoadable >xFormAsLoadable( m_xForm, UNO_QUERY );
    DBG_ASSERT( xFormAsLoadable.is() || !m_xForm.is(), "BibDataManager::unload: invalid form!");
    if ( !xFormAsLoadable.is() )
        return;

    EventObject aEvt( static_cast< XWeak* >( this ) );

    {
        std::unique_lock g(m_aMutex);
        m_aLoadListeners.notifyEach( g, &XLoadListener::unloading, aEvt );
    }

    xFormAsLoadable->unload();

    {
        std::unique_lock g(m_aMutex);
        m_aLoadListeners.notifyEach( g, &XLoadListener::unloaded, aEvt );
    }
}


void SAL_CALL BibDataManager::reload(  )
{
    if ( !isLoaded() )
        // nothing to do
        return;

    Reference< XLoadable >xFormAsLoadable( m_xForm, UNO_QUERY );
    DBG_ASSERT( xFormAsLoadable.is() || !m_xForm.is(), "BibDataManager::unload: invalid form!");
    if ( !xFormAsLoadable.is() )
        return;

    EventObject aEvt( static_cast< XWeak* >( this ) );

    {
        std::unique_lock g(m_aMutex);
        m_aLoadListeners.notifyEach( g, &XLoadListener::reloading, aEvt );
    }

    xFormAsLoadable->reload();

    {
        std::unique_lock g(m_aMutex);
        m_aLoadListeners.notifyEach( g, &XLoadListener::reloaded, aEvt );
    }
}


sal_Bool SAL_CALL BibDataManager::isLoaded(  )
{
    Reference< XLoadable >xFormAsLoadable( m_xForm, UNO_QUERY );
    DBG_ASSERT( xFormAsLoadable.is() || !m_xForm.is(), "BibDataManager::isLoaded: invalid form!");

    bool bLoaded = false;
    if ( xFormAsLoadable.is() )
        bLoaded = xFormAsLoadable->isLoaded();
    return bLoaded;
}


void SAL_CALL BibDataManager::addLoadListener( const Reference< XLoadListener >& aListener )
{
    std::unique_lock g(m_aMutex);
    m_aLoadListeners.addInterface( g, aListener );
}


void SAL_CALL BibDataManager::removeLoadListener( const Reference< XLoadListener >& aListener )
{
    std::unique_lock g(m_aMutex);
    m_aLoadListeners.removeInterface( g, aListener );
}


Reference< awt::XControlModel > BibDataManager::createGridModel(const OUString& rName)
{
    Reference< awt::XControlModel > xModel;

    try
    {
        // create the control model
        Reference< XMultiServiceFactory >  xMgr = ::comphelper::getProcessServiceFactory();
        Reference< XInterface >  xObject = xMgr->createInstance("com.sun.star.form.component.GridControl");
        xModel.set( xObject, UNO_QUERY );

        // set the
        Reference< XPropertySet > xPropSet( xModel, UNO_QUERY );
        xPropSet->setPropertyValue( "Name", Any( rName ) );

        // set the name of the to-be-created control
        Any aAny(OUString("com.sun.star.form.control.InteractionGridControl"));
        xPropSet->setPropertyValue( "DefaultControl",aAny );

        // the helpURL
        OUString uProp("HelpURL");
        Reference< XPropertySetInfo > xPropInfo = xPropSet->getPropertySetInfo();
        if (xPropInfo->hasPropertyByName(uProp))
        {
            xPropSet->setPropertyValue(
                uProp, Any(OUString(INET_HID_SCHEME + HID_BIB_DB_GRIDCTRL)));
        }
    }
    catch (const Exception&)
    {
        OSL_FAIL("::createGridModel: something went wrong !");
    }

    return xModel;
}

OUString BibDataManager::getControlName(sal_Int32 nFormatKey )
{
    OUString aResStr;
    switch (nFormatKey)
    {
        case DataType::BIT:
        case DataType::BOOLEAN:
            aResStr="CheckBox";
            break;
        case DataType::TINYINT:
        case DataType::SMALLINT:
        case DataType::INTEGER:
            aResStr="NumericField";
            break;
        case DataType::REAL:
        case DataType::DOUBLE:
        case DataType::NUMERIC:
        case DataType::DECIMAL:
            aResStr="FormattedField";
            break;
        case DataType::TIMESTAMP:
            aResStr="FormattedField";
            break;
        case DataType::DATE:
            aResStr="DateField";
            break;
        case DataType::TIME:
            aResStr="TimeField";
            break;
        case DataType::CHAR:
        case DataType::VARCHAR:
        case DataType::LONGVARCHAR:
        default:
            aResStr="TextField";
            break;
    }
    return aResStr;
}

Reference< awt::XControlModel > BibDataManager::loadControlModel(
                    const OUString& rName, bool bForceListBox)
{
    Reference< awt::XControlModel > xModel;
    OUString aName = "View_" + rName;

    try
    {
        Reference< XNameAccess >  xFields = getColumns( m_xForm );
        if (!xFields.is())
            return xModel;
        Reference< XPropertySet >  xField;

        Any aElement;

        if(xFields->hasByName(rName))
        {
            aElement = xFields->getByName(rName);
            aElement >>= xField;

            sal_Int32 nFormatKey = 0;
            xField->getPropertyValue("Type") >>= nFormatKey;

            OUString aInstanceName("com.sun.star.form.component.");

            if (bForceListBox)
                aInstanceName += "ListBox";
            else
                aInstanceName += getControlName(nFormatKey);

            Reference< XComponentContext >  xContext = comphelper::getProcessComponentContext();
            Reference< XInterface >  xObject = xContext->getServiceManager()->createInstanceWithContext(aInstanceName, xContext);
            xModel.set( xObject, UNO_QUERY );
            Reference< XPropertySet >  xPropSet( xModel, UNO_QUERY );
            Any aFieldName; aFieldName <<= aName;

            xPropSet->setPropertyValue( FM_PROP_NAME,aFieldName);
            xPropSet->setPropertyValue( FM_PROP_CONTROLSOURCE, Any( rName ) );
            xPropSet->setPropertyValue("NativeWidgetLook", Any( true ) );

            if (bForceListBox)
            {
                uno::Any aAny;

                //uno::Reference< beans::XPropertySet >  xPropSet(xControl, UNO_QUERY);
                aAny <<= sal_Int16(1);
                xPropSet->setPropertyValue("BoundColumn", aAny);
                aAny <<= ListSourceType_VALUELIST;
                xPropSet->setPropertyValue("ListSourceType", aAny);

                uno::Sequence<OUString> aListSource(TYPE_COUNT);
                OUString* pListSourceArr = aListSource.getArray();
                //pListSourceArr[0] = "select TypeName, TypeIndex from TypeNms";
                for(sal_Int32 i = 0; i < TYPE_COUNT; ++i)
                    pListSourceArr[i] = OUString::number(i);
                aAny <<= aListSource;

                xPropSet->setPropertyValue("ListSource", aAny);

                uno::Sequence<OUString> aValues(TYPE_COUNT + 1);
                OUString* pValuesArr = aValues.getArray();
                pValuesArr[0] = BibResId(ST_TYPE_ARTICLE);
                pValuesArr[1] = BibResId(ST_TYPE_BOOK);
                pValuesArr[2] = BibResId(ST_TYPE_BOOKLET);
                pValuesArr[3] = BibResId(ST_TYPE_CONFERENCE);
                pValuesArr[4] = BibResId(ST_TYPE_INBOOK );
                pValuesArr[5] = BibResId(ST_TYPE_INCOLLECTION);
                pValuesArr[6] = BibResId(ST_TYPE_INPROCEEDINGS);
                pValuesArr[7] = BibResId(ST_TYPE_JOURNAL       );
                pValuesArr[8] = BibResId(ST_TYPE_MANUAL    );
                pValuesArr[9] = BibResId(ST_TYPE_MASTERSTHESIS);
                pValuesArr[10] = BibResId(ST_TYPE_MISC      );
                pValuesArr[11] = BibResId(ST_TYPE_PHDTHESIS );
                pValuesArr[12] = BibResId(ST_TYPE_PROCEEDINGS   );
                pValuesArr[13] = BibResId(ST_TYPE_TECHREPORT    );
                pValuesArr[14] = BibResId(ST_TYPE_UNPUBLISHED   );
                pValuesArr[15] = BibResId(ST_TYPE_EMAIL     );
                pValuesArr[16] = BibResId(ST_TYPE_WWW           );
                pValuesArr[17] = BibResId(ST_TYPE_CUSTOM1       );
                pValuesArr[18] = BibResId(ST_TYPE_CUSTOM2       );
                pValuesArr[19] = BibResId(ST_TYPE_CUSTOM3       );
                pValuesArr[20] = BibResId(ST_TYPE_CUSTOM4       );
                pValuesArr[21] = BibResId(ST_TYPE_CUSTOM5       );
                // empty string if an invalid value no values is set
                pValuesArr[TYPE_COUNT].clear();

                aAny <<= aValues;

                xPropSet->setPropertyValue("StringItemList", aAny);

                xPropSet->setPropertyValue( "Dropdown", Any(true) );
            }

            Reference< XFormComponent >  aFormComp(xModel,UNO_QUERY );

            Reference< XNameContainer >  xNameCont( m_xForm, UNO_QUERY );
            xNameCont->insertByName(aName, Any( aFormComp ) );

            // now if the form where we inserted the new model is already loaded, notify the model of this
            // Note that this implementation below is a HACK as it relies on the fact that the model adds itself
            // as load listener to its parent, which is an implementation detail of the model.
            //
            // the better solution would be the following:
            // in the current scenario, we insert a control model into a form. This results in the control model
            // adding itself as load listener to the form. Now, the form should realize that it's already loaded
            // and notify the model (which it knows as XLoadListener only) immediately. This seems to make sense.
            // (as an analogon to the XStatusListener semantics).
            //
            // But this would be way too risky for this last-day fix here.
            Reference< XLoadable > xLoad( m_xForm, UNO_QUERY );
            if ( xLoad.is() && xLoad->isLoaded() )
            {
                Reference< XLoadListener > xListener( aFormComp, UNO_QUERY );
                if ( xListener.is() )
                {
                    EventObject aLoadSource;
                    aLoadSource.Source = xLoad;
                    xListener->loaded( aLoadSource );
                }
            }
        }
    }
    catch (const Exception&)
    {
        OSL_FAIL("::loadControlModel: something went wrong !");
    }
    return xModel;
}

void BibDataManager::CreateMappingDialog(weld::Window* pParent)
{
    MappingDialog_Impl aDlg(pParent, this);
    if (RET_OK == aDlg.run() && pBibView)
    {
        reload();
    }
}

OUString BibDataManager::CreateDBChangeDialog(weld::Window* pParent)
{
    OUString uRet;
    DBChangeDialog_Impl aDlg(pParent, this);
    if (aDlg.run() == RET_OK)
    {
        OUString sNewURL = aDlg.GetCurrentURL();
        if(sNewURL != getActiveDataSource())
        {
            uRet = sNewURL;
        }
    }
    return uRet;
}

void BibDataManager::DispatchDBChangeDialog()
{
    if (pToolbar)
        pToolbar->SendDispatch(pToolbar->GetChangeSourceId(), Sequence< PropertyValue >());
}

const OUString& BibDataManager::GetIdentifierMapping()
{
    if(sIdentifierMapping.isEmpty())
    {
        BibConfig* pConfig = BibModul::GetConfig();
        BibDBDescriptor aDesc;
        aDesc.sDataSource = getActiveDataSource();
        aDesc.sTableOrQuery = getActiveDataTable();
        aDesc.nCommandType = CommandType::TABLE;
        const Mapping* pMapping = pConfig->GetMapping(aDesc);
        sIdentifierMapping = pConfig->GetDefColumnName(IDENTIFIER_POS);
        if(pMapping)
        {
            for(const auto & aColumnPair : pMapping->aColumnPairs)
            {
                if(aColumnPair.sLogicalColumnName == sIdentifierMapping)
                {
                    sIdentifierMapping = aColumnPair.sRealColumnName;
                    break;
                }
            }
        }
    }
    return sIdentifierMapping;
}

void BibDataManager::SetToolbar(BibToolBar* pSet)
{
    pToolbar = pSet;
    if(pToolbar)
        pToolbar->SetDatMan(*this);
}

uno::Reference< form::runtime::XFormController > const & BibDataManager::GetFormController()
{
    if(!m_xFormCtrl.is())
    {
        Reference< uno::XComponentContext > xContext = comphelper::getProcessComponentContext();
        m_xFormCtrl = form::runtime::FormController::create(xContext);
        m_xFormCtrl->setModel(uno::Reference< awt::XTabControllerModel > (getForm(), UNO_QUERY));
        m_xFormDispatch.set( m_xFormCtrl, UNO_QUERY);
    }
    return m_xFormCtrl;
}

void BibDataManager::RegisterInterceptor( const ::bib::BibBeamer* pBibBeamer)
{
    DBG_ASSERT( !m_xInterceptorHelper.is(), "BibDataManager::RegisterInterceptor: called twice!" );

    if( pBibBeamer )
        m_xInterceptorHelper = new BibInterceptorHelper( pBibBeamer, m_xFormDispatch);
}


bool BibDataManager::HasActiveConnection() const
{
    return getConnection( m_xForm ).is();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
