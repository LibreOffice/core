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

#include <swtypes.hxx>
#include "addresslistdialog.hxx"
#include "selectdbtabledialog.hxx"
#include "createaddresslistdialog.hxx"
#include <mailmergewizard.hxx>
#include <mmconfigitem.hxx>
#include "mmaddressblockpage.hxx"
#include <dbmgr.hxx>
#include <dbconfig.hxx>
#include <unotools/tempfile.hxx>
#include <vcl/svapp.hxx>
#include <tools/urlobj.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/types.hxx>
#include <com/sun/star/sdbc/XCloseable.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/uno/XNamingService.hpp>
#include <com/sun/star/sdb/DatabaseContext.hpp>
#include <com/sun/star/sdb/XCompletedConnection.hpp>
#include <com/sun/star/sdb/CommandType.hpp>
#include <com/sun/star/sdb/FilterDialog.hpp>
#include <com/sun/star/sdb/XDocumentDataSource.hpp>
#include <com/sun/star/sdbc/XRowSet.hpp>
#include <com/sun/star/sdb/XSingleSelectQueryComposer.hpp>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <com/sun/star/sdb/XQueriesSupplier.hpp>
#include <com/sun/star/task/InteractionHandler.hpp>
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <swunohelper.hxx>
#include <unotools/pathoptions.hxx>
#include <svl/urihelper.hxx>
#include <dbui.hrc>
#include <strings.hrc>
#include <view.hxx>

#include <unomid.h>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::task;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::ui::dialogs;

struct AddressUserData_Impl
{
    uno::Reference<XDataSource>             xSource;
    SharedConnection                        xConnection;
    uno::Reference< XColumnsSupplier>       xColumnsSupplier;
    uno::Reference< sdbc::XResultSet>       xResultSet;
    OUString                    sFilter;
    OUString                    sURL; // data is editable
    sal_Int32                          nCommandType;
    sal_Int32                          nTableAndQueryCount;
    AddressUserData_Impl() :
        nCommandType(0),
        nTableAndQueryCount(-1)
        {}
};

static OUString lcl_getFlatURL( uno::Reference<beans::XPropertySet> const & xSourceProperties )
{
    if(xSourceProperties.is())
    {
        OUString sDBURL;
        xSourceProperties->getPropertyValue("URL") >>= sDBURL;
        if (sDBURL.startsWith("sdbc:flat:"))
        {
            uno::Sequence<OUString> aFilters;
            xSourceProperties->getPropertyValue("TableFilter") >>= aFilters;
            uno::Sequence<PropertyValue> aInfo;
            xSourceProperties->getPropertyValue("Info") >>= aInfo;
            if(aFilters.getLength() == 1 && aInfo.hasElements() )
            {
                OUString sExtension;
                OUString sCharSet;
                for(const auto& rInfo : std::as_const(aInfo))
                {
                    if(rInfo.Name == "Extension")
                        rInfo.Value >>= sExtension;
                    else if(rInfo.Name == "CharSet")
                        rInfo.Value >>= sCharSet;
                }
                if (sCharSet=="UTF-8")
                {
                    //#i97577# at this point the 'URL' can also be a file name!
                    return URIHelper::SmartRel2Abs( INetURLObject(), sDBURL.copy(10) )
                        + "/" + aFilters[0] + "." + sExtension;
                }
            }
        }
    }
    return OUString();
}

SwAddressListDialog::SwAddressListDialog(SwMailMergeAddressBlockPage* pParent)
    : SfxDialogController(pParent->GetWizard()->getDialog(), "modules/swriter/ui/selectaddressdialog.ui", "SelectAddressDialog")
    , m_bInSelectHdl(false)
    , m_pAddressPage(pParent)
    , m_xDescriptionFI(m_xBuilder->weld_label("desc"))
    , m_xConnecting(m_xBuilder->weld_label("connecting"))
    , m_xListLB(m_xBuilder->weld_tree_view("sources"))
    , m_xLoadListPB(m_xBuilder->weld_button("add"))
    , m_xRemovePB(m_xBuilder->weld_button("remove"))
    , m_xCreateListPB(m_xBuilder->weld_button("create"))
    , m_xFilterPB(m_xBuilder->weld_button("filter"))
    , m_xEditPB(m_xBuilder->weld_button("edit"))
    , m_xTablePB(m_xBuilder->weld_button("changetable"))
    , m_xOK(m_xBuilder->weld_button("ok"))
    , m_xIter(m_xListLB->make_iterator())
{
    m_sConnecting = m_xConnecting->get_label();

    const OUString sTemp(m_xDescriptionFI->get_label()
        .replaceFirst("%1", m_xLoadListPB->strip_mnemonic(m_xLoadListPB->get_label()))
        .replaceFirst("%2", m_xCreateListPB->strip_mnemonic(m_xCreateListPB->get_label())));
    m_xDescriptionFI->set_label(sTemp);
    m_xFilterPB->connect_clicked( LINK( this, SwAddressListDialog,    FilterHdl_Impl ));
    m_xLoadListPB->connect_clicked( LINK( this, SwAddressListDialog,  LoadHdl_Impl ));
    m_xRemovePB->connect_clicked( LINK(this, SwAddressListDialog,   RemoveHdl_Impl ));
    m_xCreateListPB->connect_clicked( LINK( this, SwAddressListDialog,CreateHdl_Impl ));
    m_xEditPB->connect_clicked(LINK( this, SwAddressListDialog, EditHdl_Impl));
    m_xTablePB->connect_clicked(LINK( this, SwAddressListDialog, TableSelectHdl_Impl));

    m_xListLB->set_size_request(m_xListLB->get_approximate_digit_width() * 52,
                                m_xListLB->get_height_rows(9));

    std::vector<int> aWidths;
    aWidths.push_back(m_xListLB->get_approximate_digit_width() * 26);
    m_xListLB->set_column_fixed_widths(aWidths);

    m_xListLB->make_sorted();
    m_xOK->connect_clicked(LINK(this, SwAddressListDialog, OKHdl_Impl));

    uno::Reference<XComponentContext> xContext( ::comphelper::getProcessComponentContext() );
    m_xDBContext = DatabaseContext::create(xContext);

    SwMailMergeConfigItem& rConfigItem = m_pAddressPage->GetWizard()->GetConfigItem();
    const SwDBData& rCurrentData = rConfigItem.GetCurrentDBData();

    bool bEnableEdit = false;
    bool bEnableOK = true;
    bool bSelected = false;
    m_xListLB->unselect_all();

    SwDBConfig aDb;
    const OUString sBibliography = aDb.GetBibliographySource().sDataSource;
    const uno::Sequence< OUString> aNames = m_xDBContext->getElementNames();
    for(const OUString& rName : aNames)
    {
        if ( rName == sBibliography )
            continue;
        m_xListLB->append(m_xIter.get());
        m_xListLB->set_text(*m_xIter, rName, 0);
        m_aUserData.emplace_back(new AddressUserData_Impl);
        AddressUserData_Impl* pUserData = m_aUserData.back().get();
        m_xListLB->set_id(*m_xIter, OUString::number(reinterpret_cast<sal_Int64>(pUserData)));
        if (rName == rCurrentData.sDataSource)
        {
            m_xListLB->select(*m_xIter);
            bSelected = true;
            m_xListLB->set_text(*m_xIter, rCurrentData.sCommand, 1);
            pUserData->nCommandType = rCurrentData.nCommandType;
            pUserData->xSource = rConfigItem.GetSource();
            pUserData->xConnection = rConfigItem.GetConnection();
            pUserData->xColumnsSupplier = rConfigItem.GetColumnsSupplier();
            pUserData->xResultSet = rConfigItem.GetResultSet();
            pUserData->sFilter = rConfigItem.GetFilter();
            //is the data source editable (csv, Unicode, single table)
            uno::Reference<beans::XPropertySet> xSourceProperties;
            try
            {
                m_xDBContext->getByName(rName) >>= xSourceProperties;
                pUserData->sURL = lcl_getFlatURL( xSourceProperties );
                bEnableEdit = !pUserData->sURL.isEmpty() &&
                    SWUnoHelper::UCB_IsFile( pUserData->sURL ) && //#i97577#
                    !SWUnoHelper::UCB_IsReadOnlyFileName( pUserData->sURL );
            }
            catch (const uno::Exception&)
            {
                bEnableOK = false;
            }
            m_aDBData = rCurrentData;
        }
    }

    bool bHasChildren = m_xListLB->n_children() > 0;
    if (bHasChildren && !bSelected)
        m_xListLB->select(0); // select the first entry if nothing else selected
    m_xOK->set_sensitive(bHasChildren && bEnableOK);
    m_xEditPB->set_sensitive(bEnableEdit);
    m_xRemovePB->set_sensitive(m_xListLB->n_children() > 0);
    m_xFilterPB->set_sensitive(m_xListLB->n_children() > 0);
    m_xTablePB->set_sensitive(m_xListLB->n_children() > 0);
    m_xListLB->connect_changed(LINK(this, SwAddressListDialog, ListBoxSelectHdl_Impl));
    TableSelectHdl(nullptr);
}

SwAddressListDialog::~SwAddressListDialog()
{
}

IMPL_LINK_NOARG(SwAddressListDialog, FilterHdl_Impl, weld::Button&, void)
{
    int nSelect = m_xListLB->get_selected_index();
    uno::Reference< XMultiServiceFactory > xMgr( ::comphelper::getProcessServiceFactory() );
    if (nSelect != -1)
    {
        const OUString sCommand = m_xListLB->get_text(nSelect, 1);
        if (sCommand.isEmpty())
            return;

        AddressUserData_Impl* pUserData = reinterpret_cast<AddressUserData_Impl*>(m_xListLB->get_id(nSelect).toInt64());
        if (pUserData->xConnection.is() )
        {
            try
            {
                uno::Reference<lang::XMultiServiceFactory> xConnectFactory(pUserData->xConnection, UNO_QUERY_THROW);
                uno::Reference<XSingleSelectQueryComposer> xComposer(
                        xConnectFactory->createInstance("com.sun.star.sdb.SingleSelectQueryComposer"), UNO_QUERY_THROW);

                uno::Reference<XRowSet> xRowSet(
                        xMgr->createInstance("com.sun.star.sdb.RowSet"), UNO_QUERY);
                uno::Reference<XPropertySet> xRowProperties(xRowSet, UNO_QUERY);
                xRowProperties->setPropertyValue("DataSourceName",
                        makeAny(m_xListLB->get_text(nSelect, 0)));
                xRowProperties->setPropertyValue("Command", makeAny(sCommand));
                xRowProperties->setPropertyValue("CommandType", makeAny(pUserData->nCommandType));
                xRowProperties->setPropertyValue("ActiveConnection", makeAny(pUserData->xConnection.getTyped()));
                xRowSet->execute();

                OUString sQuery;
                xRowProperties->getPropertyValue("ActiveCommand")>>= sQuery;
                xComposer->setQuery(sQuery);
                if(!pUserData->sFilter.isEmpty())
                    xComposer->setFilter(pUserData->sFilter);

                uno::Reference< XExecutableDialog> xDialog = sdb::FilterDialog::createWithQuery( comphelper::getComponentContext(xMgr),
                   xComposer,xRowSet, uno::Reference<awt::XWindow>() );

                if ( RET_OK == xDialog->execute() )
                {
                    weld::WaitObject aWait(m_xDialog.get());
                    pUserData->sFilter = xComposer->getFilter();
                }
                ::comphelper::disposeComponent(xRowSet);
            }
            catch (const Exception&)
            {
                OSL_FAIL("exception caught in SwAddressListDialog::FilterHdl_Impl");
            }
        }
    }
}

IMPL_LINK_NOARG(SwAddressListDialog, LoadHdl_Impl, weld::Button&, void)
{
    SwView* pView = m_pAddressPage->GetWizard()->GetSwView();

    const OUString sNewSource = SwDBManager::LoadAndRegisterDataSource(m_xDialog.get(), pView ? pView->GetDocShell() : nullptr);
    if(!sNewSource.isEmpty())
    {
        m_xListLB->append(m_xIter.get());
        m_xListLB->set_text(*m_xIter, sNewSource, 0);
        m_aUserData.emplace_back(new AddressUserData_Impl);
        AddressUserData_Impl* pUserData = m_aUserData.back().get();
        m_xListLB->set_id(*m_xIter, OUString::number(reinterpret_cast<sal_Int64>(pUserData)));
        m_xListLB->select(*m_xIter);
        ListBoxSelectHdl_Impl(*m_xListLB);
        m_xRemovePB->set_sensitive(true);
    }
}

IMPL_LINK_NOARG(SwAddressListDialog, RemoveHdl_Impl, weld::Button&, void)
{
    int nEntry = m_xListLB->get_selected_index();
    if (nEntry != -1)
    {
        std::unique_ptr<weld::MessageDialog> xQuery(Application::CreateMessageDialog(getDialog(),
                                                    VclMessageType::Question, VclButtonsType::YesNo, SwResId(ST_DELETE_CONFIRM)));
        if (xQuery->run() == RET_YES)
        {   // Remove data source connection
            SwDBManager::RevokeDataSource(m_xListLB->get_selected_text());
            // Remove item from the list
            m_xListLB->remove(nEntry);
            // If this was the last item, disable the Remove & Edit buttons and enable Create
            if (m_xListLB->n_children() < 1 )
                {
                m_xRemovePB->set_sensitive(false);
                m_xEditPB->set_sensitive(false);
                m_xFilterPB->set_sensitive(false);
                m_xCreateListPB->set_sensitive(true);
                }
        }
    }


}

IMPL_LINK_NOARG(SwAddressListDialog, CreateHdl_Impl, weld::Button&, void)
{
    SwCreateAddressListDialog aDlg(m_xDialog.get(), /*sInputURL*/OUString(), m_pAddressPage->GetWizard()->GetConfigItem());
    if (RET_OK == aDlg.run())
    {
        //register the URL a new datasource
        const OUString sURL = aDlg.GetURL();
        try
        {
            uno::Reference<XInterface> xNewInstance = m_xDBContext->createInstance();
            INetURLObject aURL( sURL );
            const OUString sNewName = aURL.getBase();
            //find a unique name if sNewName already exists
            OUString sFind(sNewName);
            sal_Int32 nIndex = 0;
            while(m_xDBContext->hasByName(sFind))
            {
                sFind = sNewName + OUString::number(++nIndex);
            }
            uno::Reference<XPropertySet> xDataProperties(xNewInstance, UNO_QUERY);

            //only the 'path' has to be added
            INetURLObject aTempURL(aURL);
            aTempURL.removeSegment();
            aTempURL.removeFinalSlash();
            const OUString sDBURL("sdbc:flat:" + aTempURL.GetMainURL(INetURLObject::DecodeMechanism::NONE));
            xDataProperties->setPropertyValue("URL", Any(sDBURL));
            //set the filter to the file name without extension
            uno::Sequence<OUString> aFilters { sNewName };
            xDataProperties->setPropertyValue("TableFilter", Any(aFilters));

            uno::Sequence<PropertyValue> aInfo(4);
            PropertyValue* pInfo = aInfo.getArray();
            pInfo[0].Name = "FieldDelimiter";
            pInfo[0].Value <<= OUString('\t');
            pInfo[1].Name = "StringDelimiter";
            pInfo[1].Value <<= OUString('"');
            pInfo[2].Name = "Extension";
            pInfo[2].Value <<= aURL.getExtension();//"csv";
            pInfo[3].Name = "CharSet";
            pInfo[3].Value <<= OUString("UTF-8");
            xDataProperties->setPropertyValue("Info", Any(aInfo));

            uno::Reference<sdb::XDocumentDataSource> xDS(xNewInstance, UNO_QUERY_THROW);
            uno::Reference<frame::XStorable> xStore(xDS->getDatabaseDocument(), UNO_QUERY_THROW);
            OUString const sExt(".odb");
            OUString sTmpName;
            {
                OUString sHomePath(SvtPathOptions().GetWorkPath());
                utl::TempFile aTempFile(sFind, true, &sExt, &sHomePath);
                aTempFile.EnableKillingFile();
                sTmpName = aTempFile.GetURL();
            }
            xStore->storeAsURL(sTmpName, Sequence< PropertyValue >());

            m_xDBContext->registerObject( sFind, xNewInstance );
            //now insert the new source into the ListBox
            m_xListLB->append(m_xIter.get());
            m_xListLB->set_text(*m_xIter, sFind, 0);
            m_xListLB->set_text(*m_xIter, aFilters[0], 1);
            m_aUserData.emplace_back(new AddressUserData_Impl);
            AddressUserData_Impl* pUserData = m_aUserData.back().get();
            m_xListLB->set_id(*m_xIter, OUString::number(reinterpret_cast<sal_Int64>(pUserData)));
            m_xListLB->select(*m_xIter);
            ListBoxSelectHdl_Impl(*m_xListLB);
            m_xCreateListPB->set_sensitive(false);
            m_xRemovePB->set_sensitive(true);
        }
        catch (const Exception&)
        {
        }
    }
}

IMPL_LINK_NOARG(SwAddressListDialog, EditHdl_Impl, weld::Button&, void)
{
    int nEntry = m_xListLB->get_selected_index();
    AddressUserData_Impl* pUserData = nEntry != -1 ? reinterpret_cast<AddressUserData_Impl*>(m_xListLB->get_id(nEntry).toInt64()) : nullptr;
    if (pUserData && !pUserData->sURL.isEmpty())
    {
        if(pUserData->xResultSet.is())
        {
            SwMailMergeConfigItem& rConfigItem = m_pAddressPage->GetWizard()->GetConfigItem();
            if(rConfigItem.GetResultSet() != pUserData->xResultSet)
                ::comphelper::disposeComponent( pUserData->xResultSet );
            pUserData->xResultSet = nullptr;

            rConfigItem.DisposeResultSet();
        }
        pUserData->xSource.clear();
        pUserData->xColumnsSupplier.clear();
        pUserData->xConnection.clear();
            // will automatically close if it was the las reference
        SwCreateAddressListDialog aDlg(m_xDialog.get(), pUserData->sURL,
                                       m_pAddressPage->GetWizard()->GetConfigItem());
        aDlg.run();
    }
};

IMPL_LINK_NOARG(SwAddressListDialog, ListBoxSelectHdl_Impl, weld::TreeView&, void)
{
    int nSelect = m_xListLB->get_selected_index();
    Application::PostUserEvent( LINK( this, SwAddressListDialog,
                                      StaticListBoxSelectHdl_Impl ), reinterpret_cast<void*>(nSelect) );
}

IMPL_LINK(SwAddressListDialog, StaticListBoxSelectHdl_Impl, void*, p, void)
{
    int nSelect = reinterpret_cast<sal_IntPtr>(p);
    //prevent nested calls of the select handler
    if (m_bInSelectHdl)
        return;
    weld::WaitObject aWait(m_xDialog.get());
    m_bInSelectHdl = true;
    AddressUserData_Impl* pUserData = nullptr;
    if (nSelect != -1)
    {
        const OUString sTable(m_xListLB->get_text(nSelect, 1));
        if (sTable.isEmpty())
        {
            m_xListLB->set_text(nSelect, m_sConnecting, 1);
        }

        pUserData = reinterpret_cast<AddressUserData_Impl*>(m_xListLB->get_id(nSelect).toInt64());
        if(pUserData->nTableAndQueryCount > 1 || pUserData->nTableAndQueryCount == -1)
        {
            DetectTablesAndQueries(nSelect, sTable.isEmpty());
        }
        else
        {
            //otherwise set the selected db-data
            m_aDBData.sDataSource = m_xListLB->get_text(nSelect, 0);
            m_aDBData.sCommand = m_xListLB->get_text(nSelect, 1);
            m_aDBData.nCommandType = pUserData->nCommandType;
            m_xOK->set_sensitive(true);
        }
        if (m_xListLB->get_text(nSelect, 1) == m_sConnecting)
            m_xListLB->set_text(nSelect, OUString(), 1);
    }
    m_xEditPB->set_sensitive(pUserData && !pUserData->sURL.isEmpty() &&
                    SWUnoHelper::UCB_IsFile( pUserData->sURL ) && //#i97577#
                    !SWUnoHelper::UCB_IsReadOnlyFileName( pUserData->sURL ) );
    m_bInSelectHdl = false;
}

// detect the number of tables for a data source
// if only one is available then set it at the entry
void SwAddressListDialog::DetectTablesAndQueries(
        int nSelect,
        bool bWidthDialog)
{
    try
    {
        AddressUserData_Impl* pUserData = reinterpret_cast<AddressUserData_Impl*>(m_xListLB->get_id(nSelect).toInt64());
        uno::Reference<XCompletedConnection> xComplConnection;
        if(!pUserData->xConnection.is())
        {
            m_aDBData.sDataSource = m_xListLB->get_text(nSelect, 0);
            m_xDBContext->getByName(m_aDBData.sDataSource) >>= xComplConnection;
            pUserData->xSource.set(xComplConnection, UNO_QUERY);

            uno::Reference< XComponentContext > xContext( ::comphelper::getProcessComponentContext() );
            uno::Reference< XInteractionHandler > xHandler = InteractionHandler::createWithParent(xContext, nullptr);
            pUserData->xConnection = SharedConnection( xComplConnection->connectWithCompletion( xHandler ) );
        }
        if(pUserData->xConnection.is())
        {
            sal_Int32 nTables = 0;
            uno::Sequence<OUString> aTables;
            uno::Sequence<OUString> aQueries;
            uno::Reference<XTablesSupplier> xTSupplier(pUserData->xConnection, UNO_QUERY);
            if(xTSupplier.is())
            {
                uno::Reference<XNameAccess> xTables = xTSupplier->getTables();
                aTables = xTables->getElementNames();
                nTables += aTables.getLength();
            }
            uno::Reference<XQueriesSupplier> xQSupplier(pUserData->xConnection, UNO_QUERY);
            if(xQSupplier.is())
            {
                uno::Reference<XNameAccess> xQueries = xQSupplier->getQueries();
                aQueries = xQueries->getElementNames();
                nTables += aQueries.getLength();
            }
            pUserData->nTableAndQueryCount = nTables;
            if(nTables > 1 && bWidthDialog)
            {
                //now call the table select dialog - if more than one table exists
                SwSelectDBTableDialog aDlg(m_xDialog.get(), pUserData->xConnection);
                const OUString sTable = m_xListLB->get_text(nSelect, 1);
                if(!sTable.isEmpty())
                    aDlg.SetSelectedTable(sTable, pUserData->nCommandType == CommandType::TABLE);
                if(RET_OK == aDlg.run())
                {
                    bool bIsTable;
                    m_aDBData.sCommand = aDlg.GetSelectedTable(bIsTable);
                    m_aDBData.nCommandType = bIsTable ? CommandType::TABLE : CommandType::QUERY;
                    pUserData->nCommandType = m_aDBData.nCommandType;
                }
            }
            else if(nTables == 1)
            {
                if(aTables.hasElements())
                {
                    m_aDBData.sCommand = aTables[0];
                    m_aDBData.nCommandType = CommandType::TABLE;
                }
                else
                {
                    m_aDBData.sCommand = aQueries[0];
                    m_aDBData.nCommandType = CommandType::QUERY;
                }
            }
        }
        if ( !m_aDBData.sCommand.isEmpty() )
        {
            uno::Reference<beans::XPropertySet> xSourceProperties;
            m_xDBContext->getByName(m_aDBData.sDataSource) >>= xSourceProperties;
            pUserData->sURL = lcl_getFlatURL( xSourceProperties );

            pUserData->xColumnsSupplier = SwDBManager::GetColumnSupplier(pUserData->xConnection,
                                    m_aDBData.sCommand,
                                    m_aDBData.nCommandType == CommandType::TABLE ?
                                            SwDBSelect::TABLE : SwDBSelect::QUERY );
            //#i97577#
            if( pUserData->xColumnsSupplier.is() )
                m_xListLB->set_text(nSelect, m_aDBData.sCommand, 1);
            else
                m_xListLB->set_text(nSelect, OUString(), 1);
        }
        const OUString sCommand = m_xListLB->get_text(nSelect, 1);
        m_xOK->set_sensitive(!sCommand.isEmpty());
        m_xFilterPB->set_sensitive( pUserData->xConnection.is() && !sCommand.isEmpty() );
        m_xTablePB->set_sensitive( pUserData->nTableAndQueryCount > 1 );
    }
    catch (const Exception&)
    {
        OSL_FAIL("exception caught in SwAddressListDialog::DetectTablesAndQueries");
        m_xOK->set_sensitive(false);
    }
}

IMPL_LINK(SwAddressListDialog, TableSelectHdl_Impl, weld::Button&, rButton, void)
{
    TableSelectHdl(&rButton);
}

void SwAddressListDialog::TableSelectHdl(const weld::Button* pButton)
{
    weld::WaitObject aWait(m_xDialog.get());

    int nSelect = m_xListLB->get_selected_index();
    if (nSelect != -1)
    {
        AddressUserData_Impl* pUserData = reinterpret_cast<AddressUserData_Impl*>(m_xListLB->get_id(nSelect).toInt64());
        //only call the table select dialog if tables have not been searched for or there
        //are more than 1
        const OUString sTable = m_xListLB->get_text(nSelect, 1);
        if( pUserData->nTableAndQueryCount > 1 || pUserData->nTableAndQueryCount == -1)
        {
            DetectTablesAndQueries(nSelect, (pButton != nullptr) || sTable.isEmpty());
        }
    }
}

IMPL_LINK_NOARG(SwAddressListDialog, OKHdl_Impl, weld::Button&, void)
{
    m_xDialog->response(RET_OK);
}

uno::Reference< XDataSource>  SwAddressListDialog::GetSource() const
{
    uno::Reference< XDataSource>  xRet;
    int nSelect = m_xListLB->get_selected_index();
    if (nSelect != -1)
    {
        AddressUserData_Impl* pUserData = reinterpret_cast<AddressUserData_Impl*>(m_xListLB->get_id(nSelect).toInt64());
        xRet = pUserData->xSource;
    }
    return xRet;

}

SharedConnection    SwAddressListDialog::GetConnection() const
{
    SharedConnection xRet;
    int nSelect = m_xListLB->get_selected_index();
    if (nSelect != -1)
    {
        AddressUserData_Impl* pUserData = reinterpret_cast<AddressUserData_Impl*>(m_xListLB->get_id(nSelect).toInt64());
        xRet = pUserData->xConnection;
    }
    return xRet;
}

uno::Reference< XColumnsSupplier> SwAddressListDialog::GetColumnsSupplier() const
{
    uno::Reference< XColumnsSupplier> xRet;
    int nSelect = m_xListLB->get_selected_index();
    if (nSelect != -1)
    {
        AddressUserData_Impl* pUserData = reinterpret_cast<AddressUserData_Impl*>(m_xListLB->get_id(nSelect).toInt64());
        xRet = pUserData->xColumnsSupplier;
    }
    return xRet;
}

OUString SwAddressListDialog::GetFilter() const
{
    int nSelect = m_xListLB->get_selected_index();
    if (nSelect != -1)
    {
        AddressUserData_Impl* pUserData = reinterpret_cast<AddressUserData_Impl*>(m_xListLB->get_id(nSelect).toInt64());
        return pUserData->sFilter;
    }
    return OUString();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
