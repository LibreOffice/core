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
#include <osl/mutex.hxx>
#include <sal/log.hxx>
#include <tools/diagnose_ex.h>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/io/XPersistObject.hpp>
#include <com/sun/star/sdbc/ResultSetType.hpp>
#include <com/sun/star/sdbc/ResultSetConcurrency.hpp>
#include <com/sun/star/sdbc/XResultSetUpdate.hpp>
#include <com/sun/star/sdbcx/XRowLocate.hpp>
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdb/XSingleSelectQueryComposer.hpp>
#include <com/sun/star/sdbc/XDatabaseMetaData.hpp>
#include <com/sun/star/sdb/XDatabaseEnvironment.hpp>
#include <com/sun/star/uno/XNamingService.hpp>
#include <com/sun/star/sdbc/XDataSource.hpp>
#include <com/sun/star/sdb/CommandType.hpp>
#include <com/sun/star/sdb/DatabaseContext.hpp>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/sdb/XCompletedConnection.hpp>
#include <com/sun/star/task/InteractionHandler.hpp>
#include <com/sun/star/form/ListSourceType.hpp>
#include <com/sun/star/form/XLoadable.hpp>
#include <com/sun/star/form/runtime/FormController.hpp>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/form/XGridColumnFactory.hpp>
#include <com/sun/star/io/XDataInputStream.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <comphelper/container.hxx>
#include <svl/urihelper.hxx>
#include <svtools/svtabbx.hxx>
#include <svtools/headbar.hxx>
#include <vcl/dialog.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <vcl/group.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/edit.hxx>
#include <vcl/msgbox.hxx>
#include <tools/debug.hxx>
#include "datman.hxx"
#include "bibresid.hxx"
#include "bibmod.hxx"
#include "bibview.hxx"
#include "bibprop.hrc"
#include "toolbar.hxx"
#include "bibconfig.hxx"
#include "bibbeam.hxx"
#include "general.hxx"
#include "bib.hrc"
#include "bibliography.hrc"
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

Reference< XConnection > getConnection(const OUString& _rURL)
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
            OSL_FAIL("Exception caught in ODatabaseContext::getRegisteredObject()");
        }
    }
    // build the connection from the data source
    Reference< XConnection > xConn;
    if (xDataSource.is())
    {
        // need user/pwd for this
        Reference< XPropertySet >  xDataSourceProps(xDataSource, UNO_QUERY);
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

Reference< XConnection >    getConnection(const Reference< XInterface > & xRowSet)
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
        OSL_FAIL("exception in getConnection");
    }

    return xConn;
}

Reference< XNameAccess >  getColumns(const Reference< XForm > & _rxForm)
{
    Reference< XNameAccess >  xReturn;
    // check if the form is alive
    Reference< XColumnsSupplier >  xSupplyCols( _rxForm, UNO_QUERY );
    if (xSupplyCols.is())
        xReturn = xSupplyCols->getColumns();

    if (!xReturn.is() || (xReturn->getElementNames().getLength() == 0))
    {   // no ....
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
            catch (const Exception& e)
            {
#ifdef DBG_UTIL
                OUString sMsg( "::getColumns : catched an exception (" + e.Message + ") ..." );

                OSL_FAIL(OUStringToOString(sMsg, RTL_TEXTENCODING_ASCII_US ).getStr());
#else
                (void)e;
#endif
            }

        }
    }
    return xReturn;
}

class MappingDialog_Impl : public ModalDialog
{
    BibDataManager* pDatMan;
    VclPtr<OKButton>       pOKBT;
    VclPtr<ListBox>        pIdentifierLB;
    VclPtr<ListBox>        pAuthorityTypeLB;
    VclPtr<ListBox>        pAuthorLB;
    VclPtr<ListBox>        pTitleLB;
    VclPtr<ListBox>        pMonthLB;
    VclPtr<ListBox>        pYearLB;
    VclPtr<ListBox>        pISBNLB;
    VclPtr<ListBox>        pBooktitleLB;
    VclPtr<ListBox>        pChapterLB;
    VclPtr<ListBox>        pEditionLB;
    VclPtr<ListBox>        pEditorLB;
    VclPtr<ListBox>        pHowpublishedLB;
    VclPtr<ListBox>        pInstitutionLB;
    VclPtr<ListBox>        pJournalLB;
    VclPtr<ListBox>        pNoteLB;
    VclPtr<ListBox>        pAnnoteLB;
    VclPtr<ListBox>        pNumberLB;
    VclPtr<ListBox>        pOrganizationsLB;
    VclPtr<ListBox>        pPagesLB;
    VclPtr<ListBox>        pPublisherLB;
    VclPtr<ListBox>        pAddressLB;
    VclPtr<ListBox>        pSchoolLB;
    VclPtr<ListBox>        pSeriesLB;
    VclPtr<ListBox>        pReportTypeLB;
    VclPtr<ListBox>        pVolumeLB;
    VclPtr<ListBox>        pURLLB;
    VclPtr<ListBox>        pCustom1LB;
    VclPtr<ListBox>        pCustom2LB;
    VclPtr<ListBox>        pCustom3LB;
    VclPtr<ListBox>        pCustom4LB;
    VclPtr<ListBox>        pCustom5LB;
    VclPtr<ListBox>        aListBoxes[COLUMN_COUNT];
    OUString        sNone;

    bool        bModified;


    DECL_LINK_TYPED(OkHdl, Button*, void);
    DECL_LINK_TYPED(ListBoxSelectHdl, ListBox&, void);

public:
    MappingDialog_Impl(vcl::Window* pParent, BibDataManager* pDatMan);
    virtual ~MappingDialog_Impl();
    virtual void dispose() override;

    void    SetModified() {bModified = true;}

};

static sal_uInt16 lcl_FindLogicalName(BibConfig* pConfig ,
                                    const OUString& rLogicalColumnName)
{
    for(sal_uInt16 i = 0; i < COLUMN_COUNT; i++)
    {
        if(rLogicalColumnName == pConfig->GetDefColumnName(i))
            return i;
    }
    return USHRT_MAX;
}

MappingDialog_Impl::MappingDialog_Impl(vcl::Window* pParent, BibDataManager* pMan)
    : ModalDialog(pParent, "MappingDialog", "modules/sbibliography/ui/mappingdialog.ui")
    , pDatMan(pMan)
    , sNone(BIB_RESSTR(RID_BIB_STR_NONE))
    , bModified(false)
{
    get(pOKBT, "ok");
    get(pIdentifierLB, "identifierCombobox");
    get(pAuthorityTypeLB, "authorityTypeCombobox");
    get(pAuthorLB, "authorCombobox");
    get(pTitleLB, "titleCombobox");
    get(pMonthLB, "monthCombobox");
    get(pYearLB, "yearCombobox");
    get(pISBNLB, "ISBNCombobox");
    get(pBooktitleLB, "bookTitleCombobox");
    get(pChapterLB, "chapterCombobox");
    get(pEditionLB, "editionCombobox");
    get(pEditorLB, "editorCombobox");
    get(pHowpublishedLB, "howPublishedCombobox");
    get(pInstitutionLB, "institutionCombobox");
    get(pJournalLB, "journalCombobox");
    get(pNoteLB, "noteCombobox");
    get(pAnnoteLB, "annoteCombobox");
    get(pNumberLB, "numberCombobox");
    get(pOrganizationsLB, "organizationCombobox");
    get(pPagesLB, "pagesCombobox");
    get(pPublisherLB, "publisherCombobox");
    get(pAddressLB, "addressCombobox");
    get(pSchoolLB, "schoolCombobox");
    get(pSeriesLB, "seriesCombobox");
    get(pReportTypeLB, "reportTypeCombobox");
    get(pVolumeLB, "volumeCombobox");
    get(pURLLB, "URLCombobox");
    get(pCustom1LB, "custom1Combobox");
    get(pCustom2LB, "custom2Combobox");
    get(pCustom3LB, "custom3Combobox");
    get(pCustom4LB, "custom4Combobox");
    get(pCustom5LB, "custom5Combobox");

    pOKBT->SetClickHdl(LINK(this, MappingDialog_Impl, OkHdl));
    OUString sTitle = GetText();
    sTitle = sTitle.replaceFirst("%1", pDatMan->getActiveDataTable());
    SetText(sTitle);

    aListBoxes[0] = pIdentifierLB;
    aListBoxes[1] = pAuthorityTypeLB;
    aListBoxes[2] = pAuthorLB;
    aListBoxes[3] = pTitleLB;
    aListBoxes[4] = pYearLB;
    aListBoxes[5] = pISBNLB;
    aListBoxes[6] = pBooktitleLB;
    aListBoxes[7] = pChapterLB;
    aListBoxes[8] = pEditionLB;
    aListBoxes[9] = pEditorLB;
    aListBoxes[10] = pHowpublishedLB;
    aListBoxes[11] = pInstitutionLB;
    aListBoxes[12] = pJournalLB;
    aListBoxes[13] = pMonthLB;
    aListBoxes[14] = pNoteLB;
    aListBoxes[15] = pAnnoteLB;
    aListBoxes[16] = pNumberLB;
    aListBoxes[17] = pOrganizationsLB;
    aListBoxes[18] = pPagesLB;
    aListBoxes[19] = pPublisherLB;
    aListBoxes[20] = pAddressLB;
    aListBoxes[21] = pSchoolLB;
    aListBoxes[22] = pSeriesLB;
    aListBoxes[23] = pReportTypeLB;
    aListBoxes[24] = pVolumeLB;
    aListBoxes[25] = pURLLB;
    aListBoxes[26] = pCustom1LB;
    aListBoxes[27] = pCustom2LB;
    aListBoxes[28] = pCustom3LB;
    aListBoxes[29] = pCustom4LB;
    aListBoxes[30] = pCustom5LB;

    aListBoxes[0]->InsertEntry(sNone);
    Reference< XNameAccess >  xFields = getColumns( pDatMan->getForm() );
    DBG_ASSERT(xFields.is(), "MappingDialog_Impl::MappingDialog_Impl : gave me an invalid form !");
    if(xFields.is())
    {
        Sequence< OUString > aNames = xFields->getElementNames();
        sal_Int32 nFieldsCount = aNames.getLength();
        const OUString* pNames = aNames.getConstArray();

        for(sal_Int32 nField = 0; nField < nFieldsCount; nField++)
            aListBoxes[0]->InsertEntry(pNames[nField]);
    }

    Link<ListBox&,void> aLnk = LINK(this, MappingDialog_Impl, ListBoxSelectHdl);

    aListBoxes[0]->SelectEntryPos(0);
    aListBoxes[0]->SetSelectHdl(aLnk);
    for(sal_uInt16 i = 1; i < COLUMN_COUNT; i++)
    {
        for(sal_Int32 j = 0; j < aListBoxes[0]->GetEntryCount();j++)
            aListBoxes[i]->InsertEntry(aListBoxes[0]->GetEntry(j));
        aListBoxes[i]->SelectEntryPos(0);
        aListBoxes[i]->SetSelectHdl(aLnk);
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
                aListBoxes[nListBoxIndex]->SelectEntry(aColumnPair.sRealColumnName);
            }
        }
    }
}

MappingDialog_Impl::~MappingDialog_Impl()
{
    disposeOnce();
}

void MappingDialog_Impl::dispose()
{
    pOKBT.clear();
    pIdentifierLB.clear();
    pAuthorityTypeLB.clear();
    pAuthorLB.clear();
    pTitleLB.clear();
    pMonthLB.clear();
    pYearLB.clear();
    pISBNLB.clear();
    pBooktitleLB.clear();
    pChapterLB.clear();
    pEditionLB.clear();
    pEditorLB.clear();
    pHowpublishedLB.clear();
    pInstitutionLB.clear();
    pJournalLB.clear();
    pNoteLB.clear();
    pAnnoteLB.clear();
    pNumberLB.clear();
    pOrganizationsLB.clear();
    pPagesLB.clear();
    pPublisherLB.clear();
    pAddressLB.clear();
    pSchoolLB.clear();
    pSeriesLB.clear();
    pReportTypeLB.clear();
    pVolumeLB.clear();
    pURLLB.clear();
    pCustom1LB.clear();
    pCustom2LB.clear();
    pCustom3LB.clear();
    pCustom4LB.clear();
    pCustom5LB.clear();
    for(auto & a : aListBoxes)
        a.clear();
    ModalDialog::dispose();
}

IMPL_LINK_TYPED(MappingDialog_Impl, ListBoxSelectHdl, ListBox&, rListBox, void)
{
    const sal_Int32 nEntryPos = rListBox.GetSelectEntryPos();
    if(0 < nEntryPos)
    {
        for(VclPtr<ListBox> & aListBoxe : aListBoxes)
        {
            if(&rListBox != aListBoxe && aListBoxe->GetSelectEntryPos() == nEntryPos)
                aListBoxe->SelectEntryPos(0);
        }
    }
    SetModified();
}

IMPL_LINK_NOARG_TYPED(MappingDialog_Impl, OkHdl, Button*, void)
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
            OUString sSel = aListBoxes[nEntry]->GetSelectEntry();
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
    EndDialog(bModified ? RET_OK : RET_CANCEL);
}

class DBChangeDialog_Impl : public ModalDialog
{
    VclPtr<ListBox>    m_pSelectionLB;
    DBChangeDialogConfig_Impl   aConfig;

    BibDataManager* pDatMan;

    DECL_LINK_TYPED(DoubleClickHdl, ListBox&, void);
public:
    DBChangeDialog_Impl(vcl::Window* pParent, BibDataManager* pMan );
    virtual ~DBChangeDialog_Impl();
    virtual void dispose() override;

    OUString     GetCurrentURL()const;
};

DBChangeDialog_Impl::DBChangeDialog_Impl(vcl::Window* pParent, BibDataManager* pMan )
    : ModalDialog(pParent, "ChooseDataSourceDialog",
        "modules/sbibliography/ui/choosedatasourcedialog.ui")
    ,
    pDatMan(pMan)
{
    get(m_pSelectionLB, "treeview");
    m_pSelectionLB->set_height_request(m_pSelectionLB->GetTextHeight() * 6);

    m_pSelectionLB->SetStyle(m_pSelectionLB->GetStyle() | WB_SORT);
    m_pSelectionLB->SetDoubleClickHdl( LINK(this, DBChangeDialog_Impl, DoubleClickHdl));

    try
    {
        OUString sActiveSource = pDatMan->getActiveDataSource();
        const Sequence< OUString >& rSources = aConfig.GetDataSourceNames();
        const OUString* pSourceNames = rSources.getConstArray();
        for (sal_Int32 i = 0; i < rSources.getLength(); ++i)
            m_pSelectionLB->InsertEntry(pSourceNames[i]);

        m_pSelectionLB->SelectEntry(sActiveSource);
    }
    catch (const Exception& e)
    {
        SAL_WARN("extensions.biblio",
            "Exception in BibDataManager::DBChangeDialog_Impl::DBChangeDialog_Impl "
            << e.Message);
    }
}

IMPL_LINK_NOARG_TYPED(DBChangeDialog_Impl, DoubleClickHdl, ListBox&, void)
{
    EndDialog(RET_OK);
}

DBChangeDialog_Impl::~DBChangeDialog_Impl()
{
    disposeOnce();
}

void DBChangeDialog_Impl::dispose()
{
    m_pSelectionLB.clear();
    ModalDialog::dispose();
}

OUString  DBChangeDialog_Impl::GetCurrentURL()const
{
    return m_pSelectionLB->GetSelectEntry();
}

// XDispatchProvider
BibInterceptorHelper::BibInterceptorHelper( ::bib::BibBeamer* pBibBeamer, css::uno::Reference< css::frame::XDispatch > const & xDispatch)
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
    BibInterceptorHelper::queryDispatch( const css::util::URL& aURL, const OUString& aTargetFrameName, sal_Int32 nSearchFlags ) throw (css::uno::RuntimeException, std::exception)
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
    BibInterceptorHelper::queryDispatches( const css::uno::Sequence< css::frame::DispatchDescriptor >& aDescripts ) throw (css::uno::RuntimeException, std::exception)
{
    Sequence< Reference< XDispatch> > aReturn( aDescripts.getLength() );
    Reference< XDispatch >* pReturn = aReturn.getArray();
    const DispatchDescriptor* pDescripts = aDescripts.getConstArray();
    for ( sal_Int32 i=0; i<aDescripts.getLength(); ++i, ++pReturn, ++pDescripts )
    {
        *pReturn = queryDispatch( pDescripts->FeatureURL, pDescripts->FrameName, pDescripts->SearchFlags );
    }
    return aReturn;
}

// XDispatchProviderInterceptor
css::uno::Reference< css::frame::XDispatchProvider > SAL_CALL
    BibInterceptorHelper::getSlaveDispatchProvider(  ) throw (css::uno::RuntimeException, std::exception)
{
    return xSlaveDispatchProvider;
}

void SAL_CALL BibInterceptorHelper::setSlaveDispatchProvider( const css::uno::Reference< css::frame::XDispatchProvider >& xNewSlaveDispatchProvider ) throw (css::uno::RuntimeException, std::exception)
{
    xSlaveDispatchProvider = xNewSlaveDispatchProvider;
}

css::uno::Reference< css::frame::XDispatchProvider > SAL_CALL
    BibInterceptorHelper::getMasterDispatchProvider(  ) throw (css::uno::RuntimeException, std::exception)
{
    return xMasterDispatchProvider;
}

void SAL_CALL BibInterceptorHelper::setMasterDispatchProvider( const css::uno::Reference< css::frame::XDispatchProvider >& xNewMasterDispatchProvider ) throw (css::uno::RuntimeException, std::exception)
{
    xMasterDispatchProvider = xNewMasterDispatchProvider;
}


#define STR_UID "uid"
OUString gGridName("theGrid");
OUString gViewName("theView");
OUString gGlobalName("theGlobals");
OUString gBeamerSize("theBeamerSize");
OUString gViewSize("theViewSize");

BibDataManager::BibDataManager()
    :BibDataManager_Base( GetMutex() )
    ,m_pInterceptorHelper( nullptr )
    ,m_aLoadListeners(m_aMutex)
    ,pBibView( nullptr )
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
        RemoveMeAsUidListener();
        if (xLoad.is())
            xLoad->unload();
        if (xComp.is())
            xComp->dispose();
        if(xConnection.is())
            xConnection->dispose();
        m_xForm = nullptr;
    }
    if( m_pInterceptorHelper )
    {
        m_pInterceptorHelper->ReleaseInterceptor();
        m_pInterceptorHelper->release();
        m_pInterceptorHelper = nullptr;
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
            Sequence< OUString > aNames = xColContainer->getElementNames();
            const OUString* pNames = aNames.getConstArray();
            const OUString* pNamesEnd = pNames + aNames.getLength();
            for ( ; pNames != pNamesEnd; ++pNames )
                xColContainer->removeByName( *pNames );
        }

        Reference< XNameAccess >  xFields = getColumns( m_xForm );
        if (!xFields.is())
            return;

        Reference< XGridColumnFactory > xColFactory( _rxGrid, UNO_QUERY );

        Reference< XPropertySet >  xField;

        Sequence< OUString > aFields( xFields->getElementNames() );
        const OUString* pFields = aFields.getConstArray();
        const OUString* pFieldsEnd = pFields + aFields.getLength();

        for ( ; pFields != pFieldsEnd; ++pFields )
        {
            xFields->getByName( *pFields ) >>= xField;

            OUString sCurrentModelType;
            const OUString sType("Type");
            sal_Int32 nType = 0;
            bool bIsFormatted           = false;
            bool bFormattedIsNumeric    = true;
            xField->getPropertyValue(sType) >>= nType;
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
                    SAL_FALLTHROUGH;
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
            Any aColName = makeAny( *pFields );
            xCurrentCol->setPropertyValue(FM_PROP_CONTROLSOURCE,    aColName);
            xCurrentCol->setPropertyValue(FM_PROP_LABEL, aColName);

            xColContainer->insertByName( *pFields, makeAny( xCurrentCol ) );
        }
    }
    catch (const Exception&)
    {
        OSL_FAIL("Exception in BibDataManager::InsertFields");
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
            xNameCont->insertByName( sName, makeAny( m_xGridModel ) );
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
            aVal <<= (sal_Int32)ResultSetType::SCROLL_INSENSITIVE;
            aPropertySet->setPropertyValue("ResultSetType",aVal );
            aVal <<= (sal_Int32)ResultSetConcurrency::READ_ONLY;
            aPropertySet->setPropertyValue("ResultSetConcurrency", aVal);

            //Caching for Performance
            aVal <<= (sal_Int32)50;
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

            if(aTableNameSeq.getLength() > 0)
            {
                const OUString* pTableNames = aTableNameSeq.getConstArray();
                if(!rDesc.sTableOrQuery.isEmpty())
                    aActiveDataTable = rDesc.sTableOrQuery;
                else
                {
                    rDesc.sTableOrQuery = aActiveDataTable = pTableNames[0];
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

Sequence< OUString > BibDataManager::getDataSources()
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
        xFormProps->setPropertyValue( "Filter", makeAny( aQuery ) );
        xFormProps->setPropertyValue( "ApplyFilter", makeAny( true ) );
        reload();
    }
    catch (const Exception&)
    {
        DBG_UNHANDLED_EXCEPTION();
    }


}

OUString BibDataManager::getFilter()
{

    OUString aQueryString;
    try
    {
        Reference< XPropertySet > xFormProps( m_xForm, UNO_QUERY_THROW );
        OSL_VERIFY( xFormProps->getPropertyValue( "Filter"  ) >>= aQueryString );
    }
    catch (const Exception&)
    {
        DBG_UNHANDLED_EXCEPTION();
    }


    return aQueryString;

}

Sequence< OUString > BibDataManager::getQueryFields()
{
    Sequence< OUString > aFieldSeq;
    Reference< XNameAccess >  xFields = getColumns( m_xForm );
    if (xFields.is())
        aFieldSeq = xFields->getElementNames();
    return aFieldSeq;
}

OUString BibDataManager::getQueryField()
{
    BibConfig* pConfig = BibModul::GetConfig();
    OUString aFieldString = pConfig->getQueryField();
    if(aFieldString.isEmpty())
    {
        Sequence< OUString > aSeq = getQueryFields();
        const OUString* pFields = aSeq.getConstArray();
        if(aSeq.getLength()>0)
        {
            aFieldString=pFields[0];
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
        aQueryString=aQuoteChar;
        aQueryString+=getQueryField();
        aQueryString+=aQuoteChar;
        aQueryString+=" like '";
        OUString sQuery(rQuery);
        sQuery = sQuery.replaceAll("?","_");
        sQuery = sQuery.replaceAll("*","%");
        aQueryString += sQuery;
        aQueryString+="%'";
    }
    setFilter(aQueryString);
}

void BibDataManager::setActiveDataSource(const OUString& rURL)
{
    OUString sTmp(aDataSourceURL);
    aDataSourceURL = rURL;

    Reference< XPropertySet >  aPropertySet( m_xForm, UNO_QUERY );
    if(aPropertySet.is())
    {
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
        if(aTableNameSeq.getLength() > 0)
        {
            const OUString* pTableNames = aTableNameSeq.getConstArray();
            aActiveDataTable = pTableNames[0];
            aVal <<= aActiveDataTable;
            aPropertySet->setPropertyValue("Command", aVal);
            aPropertySet->setPropertyValue("CommandType", makeAny(CommandType::TABLE));
            //Caching for Performance
            aVal <<= (sal_Int32)50;
            aPropertySet->setPropertyValue("FetchSize", aVal);
            OUString aString("SELECT * FROM ");
            // quote the table name which may contain catalog.schema.table
            Reference<XDatabaseMetaData> xMetaData(xConnection->getMetaData(),UNO_QUERY);
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

        aEvent.State = makeAny( getDataSources() );

        if(pToolbar)
        {
            aURL.Complete =".uno:Bib/source";
            aEvent.FeatureURL = aURL;
            pToolbar->statusChanged( aEvent );
        }

        updateGridModel();
        load();
    }
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
            Sequence< OUString > aTableNameSeq = xAccess->getElementNames();
            sal_uInt32 nCount = aTableNameSeq.getLength();

            const OUString* pTableNames = aTableNameSeq.getConstArray();
            const OUString* pTableNamesEnd = pTableNames + nCount;

            for ( ; pTableNames != pTableNamesEnd; ++pTableNames )
            {
                if ( rTable == *pTableNames )
                {
                    aActiveDataTable = rTable;
                    Any aVal; aVal <<= rTable;
                    aPropertySet->setPropertyValue( "Command", aVal );
                    break;
                }
            }
            if (pTableNames != pTableNamesEnd)
            {
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

                BibDBDescriptor aDesc;
                aDesc.sDataSource = aDataSourceURL;
                aDesc.sTableOrQuery = aActiveDataTable;
                aDesc.nCommandType = CommandType::TABLE;
                BibModul::GetConfig()->SetBibliographyURL(aDesc);
            }
        }
    }
    catch (const Exception&)
    {
        OSL_FAIL("::setActiveDataTable: something went wrong !");
    }
}


void SAL_CALL BibDataManager::load(  ) throw (RuntimeException, std::exception)
{
    if ( isLoaded() )
        // nothing to do
        return;

    Reference< XLoadable > xFormAsLoadable( m_xForm, UNO_QUERY );
    DBG_ASSERT( xFormAsLoadable.is() || !m_xForm.is(), "BibDataManager::load: invalid form!");
    if ( xFormAsLoadable.is() )
    {
        xFormAsLoadable->load();
        SetMeAsUidListener();

        EventObject aEvt( static_cast< XWeak* >( this ) );
        m_aLoadListeners.notifyEach( &XLoadListener::loaded, aEvt );
    }
}


void SAL_CALL BibDataManager::unload(  ) throw (RuntimeException, std::exception)
{
    if ( !isLoaded() )
        // nothing to do
        return;

    Reference< XLoadable >xFormAsLoadable( m_xForm, UNO_QUERY );
    DBG_ASSERT( xFormAsLoadable.is() || !m_xForm.is(), "BibDataManager::unload: invalid form!");
    if ( xFormAsLoadable.is() )
    {
        EventObject aEvt( static_cast< XWeak* >( this ) );

        {
            m_aLoadListeners.notifyEach( &XLoadListener::unloading, aEvt );
        }

        RemoveMeAsUidListener();
        xFormAsLoadable->unload();

        {
            m_aLoadListeners.notifyEach( &XLoadListener::unloaded, aEvt );
        }
    }
}


void SAL_CALL BibDataManager::reload(  ) throw (RuntimeException, std::exception)
{
    if ( !isLoaded() )
        // nothing to do
        return;

    Reference< XLoadable >xFormAsLoadable( m_xForm, UNO_QUERY );
    DBG_ASSERT( xFormAsLoadable.is() || !m_xForm.is(), "BibDataManager::unload: invalid form!");
    if ( xFormAsLoadable.is() )
    {
        EventObject aEvt( static_cast< XWeak* >( this ) );

        {
            m_aLoadListeners.notifyEach( &XLoadListener::reloading, aEvt );
        }

        xFormAsLoadable->reload();

        {
            m_aLoadListeners.notifyEach( &XLoadListener::reloaded, aEvt );
        }
    }
}


sal_Bool SAL_CALL BibDataManager::isLoaded(  ) throw (RuntimeException, std::exception)
{
    Reference< XLoadable >xFormAsLoadable( m_xForm, UNO_QUERY );
    DBG_ASSERT( xFormAsLoadable.is() || !m_xForm.is(), "BibDataManager::isLoaded: invalid form!");

    bool bLoaded = false;
    if ( xFormAsLoadable.is() )
        bLoaded = xFormAsLoadable->isLoaded();
    return bLoaded;
}


void SAL_CALL BibDataManager::addLoadListener( const Reference< XLoadListener >& aListener ) throw (RuntimeException, std::exception)
{
    m_aLoadListeners.addInterface( aListener );
}


void SAL_CALL BibDataManager::removeLoadListener( const Reference< XLoadListener >& aListener ) throw (RuntimeException, std::exception)
{
    m_aLoadListeners.removeInterface( aListener );
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
        xPropSet->setPropertyValue( "Name", makeAny( rName ) );

        // set the name of the to-be-created control
        OUString aControlName("com.sun.star.form.control.InteractionGridControl");
        Any aAny; aAny <<= aControlName;
        xPropSet->setPropertyValue( "DefaultControl",aAny );

        // the helpURL
        OUString uProp("HelpURL");
        Reference< XPropertySetInfo > xPropInfo = xPropSet->getPropertySetInfo();
        if (xPropInfo->hasPropertyByName(uProp))
        {
            xPropSet->setPropertyValue(
                uProp, makeAny<OUString>(INET_HID_SCHEME HID_BIB_DB_GRIDCTRL));
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
    OUString aName("View_");
    aName += rName;

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
            Reference< XPropertySetInfo >  xInfo = xField.is() ? xField->getPropertySetInfo() : Reference< XPropertySetInfo > ();

            const OUString sType("Type");
            sal_Int32 nFormatKey = 0;
            xField->getPropertyValue(sType) >>= nFormatKey;

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
            xPropSet->setPropertyValue( FM_PROP_CONTROLSOURCE, makeAny( rName ) );
            xPropSet->setPropertyValue("NativeWidgetLook", makeAny( true ) );

            if (bForceListBox)
            {
                uno::Any aAny;

                //uno::Reference< beans::XPropertySet >  xPropSet(xControl, UNO_QUERY);
                aAny <<= (sal_Int16)1;
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
                pValuesArr[0] = BIB_RESSTR(ST_TYPE_ARTICLE);
                pValuesArr[1] = BIB_RESSTR(ST_TYPE_BOOK);
                pValuesArr[2] = BIB_RESSTR(ST_TYPE_BOOKLET);
                pValuesArr[3] = BIB_RESSTR(ST_TYPE_CONFERENCE);
                pValuesArr[4] = BIB_RESSTR(ST_TYPE_INBOOK );
                pValuesArr[5] = BIB_RESSTR(ST_TYPE_INCOLLECTION);
                pValuesArr[6] = BIB_RESSTR(ST_TYPE_INPROCEEDINGS);
                pValuesArr[7] = BIB_RESSTR(ST_TYPE_JOURNAL       );
                pValuesArr[8] = BIB_RESSTR(ST_TYPE_MANUAL    );
                pValuesArr[9] = BIB_RESSTR(ST_TYPE_MASTERSTHESIS);
                pValuesArr[10] = BIB_RESSTR(ST_TYPE_MISC      );
                pValuesArr[11] = BIB_RESSTR(ST_TYPE_PHDTHESIS );
                pValuesArr[12] = BIB_RESSTR(ST_TYPE_PROCEEDINGS   );
                pValuesArr[13] = BIB_RESSTR(ST_TYPE_TECHREPORT    );
                pValuesArr[14] = BIB_RESSTR(ST_TYPE_UNPUBLISHED   );
                pValuesArr[15] = BIB_RESSTR(ST_TYPE_EMAIL     );
                pValuesArr[16] = BIB_RESSTR(ST_TYPE_WWW           );
                pValuesArr[17] = BIB_RESSTR(ST_TYPE_CUSTOM1       );
                pValuesArr[18] = BIB_RESSTR(ST_TYPE_CUSTOM2       );
                pValuesArr[19] = BIB_RESSTR(ST_TYPE_CUSTOM3       );
                pValuesArr[20] = BIB_RESSTR(ST_TYPE_CUSTOM4       );
                pValuesArr[21] = BIB_RESSTR(ST_TYPE_CUSTOM5       );
                // empty string if an invalid value no values is set
                pValuesArr[TYPE_COUNT].clear();

                aAny <<= aValues;

                xPropSet->setPropertyValue("StringItemList", aAny);

                xPropSet->setPropertyValue( "Dropdown", Any(true) );
            }

            Reference< XFormComponent >  aFormComp(xModel,UNO_QUERY );

            Reference< XNameContainer >  xNameCont( m_xForm, UNO_QUERY );
            xNameCont->insertByName(aName, makeAny( aFormComp ) );

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

void SAL_CALL BibDataManager::disposing()
{
    BibDataManager_Base::WeakComponentImplHelperBase::disposing();
}


void BibDataManager::disposing( const EventObject& /*Source*/ ) throw( css::uno::RuntimeException, std::exception )
{
    // not interested in
}


void BibDataManager::propertyChange(const beans::PropertyChangeEvent& evt) throw( RuntimeException, std::exception )
{
    try
    {
        if(evt.PropertyName == FM_PROP_VALUE)
        {
            if( evt.NewValue.getValueType() == cppu::UnoType<io::XInputStream>::get())
            {
                Reference< io::XDataInputStream >  xStream(
                    evt.NewValue, UNO_QUERY );
                aUID <<= xStream->readUTF();
            }
            else
                aUID = evt.NewValue;

            Reference< XRowLocate > xLocate(xBibCursor, UNO_QUERY);
            DBG_ASSERT(xLocate.is(), "BibDataManager::propertyChange : invalid cursor !");
            xLocate->moveToBookmark(aUID);
        }
    }
    catch (const Exception&)
    {
        OSL_FAIL("::propertyChange: something went wrong !");
    }
}


void BibDataManager::SetMeAsUidListener()
{
    try
    {
        Reference< XNameAccess >  xFields = getColumns( m_xForm );
        if (!xFields.is())
            return;

        Sequence< OUString > aFields(xFields->getElementNames());
        const OUString* pFields = aFields.getConstArray();
        sal_Int32 nCount=aFields.getLength();
        OUString StrUID(STR_UID);
        OUString theFieldName;
        for( sal_Int32 i=0; i<nCount; i++ )
        {
            const OUString& rName = pFields[i];

            if (rName.equalsIgnoreAsciiCase(StrUID))
            {
                theFieldName=pFields[i];
                break;
            }
        }

        if(!theFieldName.isEmpty())
        {
            Any aElement;

            aElement = xFields->getByName(theFieldName);
            auto xPropSet = o3tl::doAccess<Reference<XPropertySet>>(aElement);

            (*xPropSet)->addPropertyChangeListener(FM_PROP_VALUE, this);
        }

    }
    catch (const Exception&)
    {
        OSL_FAIL("Exception in BibDataManager::SetMeAsUidListener");
    }
}


void BibDataManager::RemoveMeAsUidListener()
{
    try
    {
        Reference< XNameAccess >  xFields = getColumns( m_xForm );
        if (!xFields.is())
            return;


        Sequence< OUString > aFields(xFields->getElementNames());
        const OUString* pFields = aFields.getConstArray();
        sal_Int32 nCount=aFields.getLength();
        OUString StrUID(STR_UID);
        OUString theFieldName;
        for( sal_Int32 i=0; i<nCount; i++ )
        {
            const OUString& rName = pFields[i];

            if (rName.equalsIgnoreAsciiCase(StrUID))
            {
                theFieldName=pFields[i];
                break;
            }
        }

        if(!theFieldName.isEmpty())
        {
            Any aElement;

            aElement = xFields->getByName(theFieldName);
            auto xPropSet = o3tl::doAccess<Reference<XPropertySet>>(aElement);

            (*xPropSet)->removePropertyChangeListener(FM_PROP_VALUE, this);
        }

    }
    catch (const Exception&)
    {
        OSL_FAIL("Exception in BibDataManager::RemoveMeAsUidListener");
    }
}

void BibDataManager::CreateMappingDialog(vcl::Window* pParent)
{
    VclPtrInstance< MappingDialog_Impl > pDlg(pParent, this);
    if(RET_OK == pDlg->Execute() && pBibView)
    {
        reload();
    }
}

OUString BibDataManager::CreateDBChangeDialog(vcl::Window* pParent)
{
    OUString uRet;
    VclPtrInstance< DBChangeDialog_Impl > pDlg(pParent, this );
    if(RET_OK == pDlg->Execute())
    {
        OUString sNewURL = pDlg->GetCurrentURL();
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

void BibDataManager::RegisterInterceptor( ::bib::BibBeamer* pBibBeamer)
{
    DBG_ASSERT( !m_pInterceptorHelper, "BibDataManager::RegisterInterceptor: called twice!" );

    if( pBibBeamer )
        m_pInterceptorHelper = new BibInterceptorHelper( pBibBeamer, m_xFormDispatch);
    if( m_pInterceptorHelper )
        m_pInterceptorHelper->acquire();
}


bool BibDataManager::HasActiveConnection()
{
    return getConnection( m_xForm ).is();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
