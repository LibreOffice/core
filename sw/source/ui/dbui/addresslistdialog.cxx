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
#include <addresslistdialog.hxx>
#include <selectdbtabledialog.hxx>
#include <createaddresslistdialog.hxx>
#include <mailmergewizard.hxx>
#include <mmconfigitem.hxx>
#include <mmaddressblockpage.hxx>
#include <dbmgr.hxx>
#include <dbconfig.hxx>
#include <unotools/tempfile.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/svapp.hxx>
#include <tools/urlobj.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/types.hxx>
#include <svtools/simptabl.hxx>
#include <svtools/treelistentry.hxx>
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
#include <vcl/waitobj.hxx>
#include <unotools/pathoptions.hxx>
#include <svl/urihelper.hxx>
#include <dbui.hrc>
#include <view.hxx>

#include <helpid.h>
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

#define ITEMID_NAME         1
#define ITEMID_TABLE        2

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

static OUString lcl_getFlatURL( uno::Reference<beans::XPropertySet>& xSourceProperties )
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
            if(aFilters.getLength() == 1 && aInfo.getLength() )
            {
                OUString sFieldDelim;
                OUString sStringDelim;
                OUString sExtension;
                OUString sCharSet;
                for(sal_Int32 nInfo = 0; nInfo < aInfo.getLength(); ++nInfo)
                {
                    if(aInfo[nInfo].Name == "FieldDelimiter")
                        aInfo[nInfo].Value >>= sFieldDelim;
                    else if(aInfo[nInfo].Name == "StringDelimiter")
                        aInfo[nInfo].Value >>= sStringDelim;
                    else if(aInfo[nInfo].Name == "Extension")
                        aInfo[nInfo].Value >>= sExtension;
                    else if(aInfo[nInfo].Name == "CharSet")
                        aInfo[nInfo].Value >>= sCharSet;
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

class SwAddrSourceLB : public SvSimpleTable
{
public:
    SwAddrSourceLB(SvSimpleTableContainer& rParent, WinBits nBits = 0)
        : SvSimpleTable(rParent, nBits)
    {
    }
    virtual void Resize() override;
    void setColSizes();
};

void SwAddrSourceLB::Resize()
{
    SvSimpleTable::Resize();
    setColSizes();
}

void SwAddrSourceLB::setColSizes()
{
    HeaderBar &rHB = GetTheHeaderBar();
    if (rHB.GetItemCount() < 2)
        return;

    long nWidth = rHB.GetSizePixel().Width();

    long nTabs[] = { 2, 0, nWidth/2 };

    SvSimpleTable::SetTabs(&nTabs[0], MAP_PIXEL);
}

SwAddressListDialog::SwAddressListDialog(SwMailMergeAddressBlockPage* pParent)
    : SfxModalDialog(pParent, "SelectAddressDialog",
        "modules/swriter/ui/selectaddressdialog.ui")

    ,

    m_pCreatedDataSource(nullptr),
    m_bInSelectHdl(false),
    m_pAddressPage(pParent)
{
    get(m_pDescriptionFI, "desc");
    get(m_pLoadListPB, "add");
    get(m_pCreateListPB, "create");
    get(m_pFilterPB, "filter");
    get(m_pEditPB, "edit");
    get(m_pTablePB, "changetable");
    get(m_pOK, "ok");

    m_sName = get<FixedText>("name")->GetText();
    m_sTable = get<FixedText>("table")->GetText();
    m_sConnecting = get<FixedText>("connecting")->GetText();

    const OUString sTemp(m_pDescriptionFI->GetText()
        .replaceFirst("%1", m_pLoadListPB->GetText())
        .replaceFirst("%2", m_pCreateListPB->GetText()));
    m_pDescriptionFI->SetText(sTemp);
    m_pFilterPB->SetClickHdl( LINK( this, SwAddressListDialog,    FilterHdl_Impl ));
    m_pLoadListPB->SetClickHdl( LINK( this, SwAddressListDialog,  LoadHdl_Impl ));
    m_pCreateListPB->SetClickHdl( LINK( this, SwAddressListDialog,CreateHdl_Impl ));
    m_pEditPB->SetClickHdl(LINK( this, SwAddressListDialog, EditHdl_Impl));
    m_pTablePB->SetClickHdl(LINK( this, SwAddressListDialog, TableSelectHdl_Impl));

    SvSimpleTableContainer *pHeaderTreeContainer = get<SvSimpleTableContainer>("sources");
    Size aSize = pHeaderTreeContainer->LogicToPixel(Size(182 , 102), MAP_APPFONT);
    pHeaderTreeContainer->set_width_request(aSize.Width());
    pHeaderTreeContainer->set_height_request(aSize.Height());
    m_pListLB = VclPtr<SwAddrSourceLB>::Create(*pHeaderTreeContainer);

    m_pListLB->InsertHeaderEntry(m_sName + "\t" + m_sTable);
    m_pListLB->setColSizes();

    m_pListLB->SetStyle( m_pListLB->GetStyle() | WB_SORT | WB_HSCROLL | WB_CLIPCHILDREN | WB_TABSTOP );
    m_pListLB->SetSelectionMode( SINGLE_SELECTION );
    m_pOK->SetClickHdl( LINK( this, SwAddressListDialog, OKHdl_Impl));

    uno::Reference<XComponentContext> xContext( ::comphelper::getProcessComponentContext() );
    m_xDBContext = DatabaseContext::create(xContext);

    SwMailMergeConfigItem& rConfigItem = m_pAddressPage->GetWizard()->GetConfigItem();
    const SwDBData& rCurrentData = rConfigItem.GetCurrentDBData();

    bool bEnableEdit = false;
    bool bEnableOK = true;
    m_pListLB->SelectAll( false );

    SwDBConfig aDb;
    const OUString sBibliography = aDb.GetBibliographySource().sDataSource;
    uno::Sequence< OUString> aNames = m_xDBContext->getElementNames();
    const OUString* pNames = aNames.getConstArray();
    for(sal_Int32 nName = 0; nName < aNames.getLength(); ++nName)
    {
        if ( pNames[nName] == sBibliography )
            continue;
        SvTreeListEntry* pEntry = m_pListLB->InsertEntry(pNames[nName]);
        AddressUserData_Impl* pUserData = new AddressUserData_Impl();
        pEntry->SetUserData(pUserData);
        if(pNames[nName] == rCurrentData.sDataSource)
        {
            m_pListLB->Select(pEntry);
            m_pListLB->SetEntryText(rCurrentData.sCommand, pEntry, ITEMID_TABLE - 1);
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
                m_xDBContext->getByName(pNames[nName]) >>= xSourceProperties;
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

    m_pOK->Enable(m_pListLB->GetEntryCount()>0 && bEnableOK);
    m_pEditPB->Enable(bEnableEdit);
    m_pListLB->SetSelectHdl(LINK(this, SwAddressListDialog, ListBoxSelectHdl_Impl));
    TableSelectHdl_Impl(nullptr);
}

SwAddressListDialog::~SwAddressListDialog()
{
    disposeOnce();
}

void SwAddressListDialog::dispose()
{
    SvTreeListEntry* pEntry = m_pListLB->First();
    while(pEntry)
    {
        AddressUserData_Impl* pUserData = static_cast<AddressUserData_Impl*>(pEntry->GetUserData());
        delete pUserData;
        pEntry = m_pListLB->Next( pEntry );
    }
    m_pListLB.disposeAndClear();
    m_pAddressPage.clear();
    m_pDescriptionFI.clear();
    m_pLoadListPB.clear();
    m_pCreateListPB.clear();
    m_pFilterPB.clear();
    m_pEditPB.clear();
    m_pTablePB.clear();
    m_pOK.clear();
    SfxModalDialog::dispose();
}

IMPL_LINK_NOARG_TYPED(SwAddressListDialog, FilterHdl_Impl, Button*, void)
{
    SvTreeListEntry* pSelect = m_pListLB->FirstSelected();
    uno::Reference< XMultiServiceFactory > xMgr( ::comphelper::getProcessServiceFactory() );
    if(pSelect)
    {
        const OUString sCommand = SvTabListBox::GetEntryText(pSelect, ITEMID_TABLE - 1);
        if (sCommand.isEmpty())
            return;

        AddressUserData_Impl* pUserData = static_cast<AddressUserData_Impl*>(pSelect->GetUserData());
        if(pUserData->xConnection.is() )
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
                        makeAny(SvTabListBox::GetEntryText(pSelect, ITEMID_NAME - 1)));
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
                    WaitObject aWO( nullptr );
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

IMPL_LINK_NOARG_TYPED(SwAddressListDialog, LoadHdl_Impl, Button*, void)
{
    SwView* pView = m_pAddressPage->GetWizard()->GetSwView();

    const OUString sNewSource = SwDBManager::LoadAndRegisterDataSource(pView ? pView->GetDocShell() : nullptr);
    if(!sNewSource.isEmpty())
    {
        SvTreeListEntry* pNewSource = m_pListLB->InsertEntry(sNewSource);
        pNewSource->SetUserData(new AddressUserData_Impl());
        m_pListLB->Select(pNewSource);
    }
}

IMPL_LINK_TYPED(SwAddressListDialog, CreateHdl_Impl, Button*, pButton, void)
{
    OUString sInputURL;
    VclPtr<SwCreateAddressListDialog> pDlg(
            VclPtr<SwCreateAddressListDialog>::Create(
                    pButton,
                    sInputURL,
                    m_pAddressPage->GetWizard()->GetConfigItem()));
    if(RET_OK == pDlg->Execute())
    {
        //register the URL a new datasource
        const OUString sURL = pDlg->GetURL();
        try
        {
            uno::Reference<XSingleServiceFactory> xFact( m_xDBContext, UNO_QUERY);
            uno::Reference<XInterface> xNewInstance = xFact->createInstance();
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
            const OUString sDBURL("sdbc:flat:" + aTempURL.GetMainURL(INetURLObject::NO_DECODE));
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

            uno::Reference<XNamingService> xNaming(m_xDBContext, UNO_QUERY);
            xNaming->registerObject( sFind, xNewInstance );
            //now insert the new source into the ListBox
            m_pCreatedDataSource = m_pListLB->InsertEntry(sFind + "\t" + aFilters[0]);
            AddressUserData_Impl* pUserData = new AddressUserData_Impl();
            pUserData->sURL = sURL;
            m_pCreatedDataSource->SetUserData(pUserData);
            m_pListLB->Select(m_pCreatedDataSource);
            m_pCreateListPB->Enable(false);

        }
        catch (const Exception&)
        {
        }
    }
}

IMPL_LINK_TYPED(SwAddressListDialog, EditHdl_Impl, Button*, pButton, void)
{
    SvTreeListEntry* pEntry = m_pListLB->FirstSelected();
    AddressUserData_Impl* pUserData = pEntry ? static_cast<AddressUserData_Impl*>(pEntry->GetUserData()) : nullptr;
    if(pUserData && !pUserData->sURL.isEmpty())
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
        VclPtr<SwCreateAddressListDialog> pDlg(
                VclPtr<SwCreateAddressListDialog>::Create(

                        pButton,
                        pUserData->sURL,
                        m_pAddressPage->GetWizard()->GetConfigItem()));
        if(RET_OK == pDlg->Execute())
        {
        }
    }
};

IMPL_LINK_NOARG_TYPED(SwAddressListDialog, ListBoxSelectHdl_Impl, SvTreeListBox*, void)
{
    SvTreeListEntry* pSelect = m_pListLB->FirstSelected();
    Application::PostUserEvent( LINK( this, SwAddressListDialog,
                                      StaticListBoxSelectHdl_Impl ), pSelect, true );
}

IMPL_LINK_TYPED(SwAddressListDialog, StaticListBoxSelectHdl_Impl, void*, p, void)
{
    SvTreeListEntry* pSelect = static_cast<SvTreeListEntry*>(p);
    //prevent nested calls of the select handler
    if(m_bInSelectHdl)
        return;
    EnterWait();
    m_bInSelectHdl = true;
    AddressUserData_Impl* pUserData = nullptr;
    if(pSelect)
    {
        const OUString sTable(SvTabListBox::GetEntryText(pSelect, ITEMID_TABLE - 1));
        if(sTable.isEmpty())
        {
            m_pListLB->SetEntryText(m_sConnecting, pSelect, ITEMID_TABLE - 1);
            // allow painting of the new entry
            m_pListLB->Window::Invalidate(InvalidateFlags::Update);
            for (int i = 0; i < 10; ++i)
                Application::Reschedule();
        }

        pUserData = static_cast<AddressUserData_Impl*>(pSelect->GetUserData());
        if(pUserData->nTableAndQueryCount > 1 || pUserData->nTableAndQueryCount == -1)
        {
            /*
             * We're a callback from a selection from a list box, which takes
             * place on mouse down before mouse up. The next dialog also has a
             * list box. Spawning it means this list box doesn't get the mouse
             * down event. So it sticks on "making selection" mode. So if you
             * cancel the next dialog and just move the mouse out of this entry
             * and back then the dialog pops up again, without requiring a click
             *
             * Most expedient thing to do is to manually end the parent selection
             * here.
             */
            m_pListLB->EndSelection();
            DetectTablesAndQueries(pSelect, sTable.isEmpty());
        }
        else
        {
            //otherwise set the selected db-data
            m_aDBData.sDataSource = SvTabListBox::GetEntryText(pSelect, ITEMID_NAME - 1);
            m_aDBData.sCommand = SvTabListBox::GetEntryText(pSelect, ITEMID_TABLE - 1);
            m_aDBData.nCommandType = pUserData->nCommandType;
            m_pOK->Enable();
        }
        if(SvTabListBox::GetEntryText(pSelect, ITEMID_TABLE - 1) == m_sConnecting)
           m_pListLB->SetEntryText(OUString(), pSelect, ITEMID_TABLE - 1);
    }
    m_pEditPB->Enable(pUserData && !pUserData->sURL.isEmpty() &&
                    SWUnoHelper::UCB_IsFile( pUserData->sURL ) && //#i97577#
                    !SWUnoHelper::UCB_IsReadOnlyFileName( pUserData->sURL ) );
    m_bInSelectHdl = false;
    LeaveWait();
}

// detect the number of tables for a data source
// if only one is available then set it at the entry
void SwAddressListDialog::DetectTablesAndQueries(
        SvTreeListEntry* pSelect,
        bool bWidthDialog)
{
    try
    {
        AddressUserData_Impl* pUserData = static_cast<AddressUserData_Impl*>(pSelect->GetUserData());
        uno::Reference<XCompletedConnection> xComplConnection;
        if(!pUserData->xConnection.is())
        {
            m_aDBData.sDataSource = SvTabListBox::GetEntryText(pSelect, ITEMID_NAME - 1);
            m_xDBContext->getByName(m_aDBData.sDataSource) >>= xComplConnection;
            pUserData->xSource.set(xComplConnection, UNO_QUERY);

            uno::Reference< XComponentContext > xContext( ::comphelper::getProcessComponentContext() );
            uno::Reference< XInteractionHandler > xHandler( InteractionHandler::createWithParent(xContext, nullptr), UNO_QUERY );
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
                VclPtrInstance<SwSelectDBTableDialog> pDlg(this, pUserData->xConnection);
                const OUString sTable = SvTabListBox::GetEntryText(pSelect, ITEMID_TABLE - 1);
                if(!sTable.isEmpty())
                    pDlg->SetSelectedTable(sTable, pUserData->nCommandType == CommandType::TABLE);
                if(RET_OK == pDlg->Execute())
                {
                    bool bIsTable;
                    m_aDBData.sCommand = pDlg->GetSelectedTable(bIsTable);
                    m_aDBData.nCommandType = bIsTable ? CommandType::TABLE : CommandType::QUERY;
                    pUserData->nCommandType = m_aDBData.nCommandType;
                }
            }
            else if(nTables == 1)
            {
                if(aTables.getLength())
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
                m_pListLB->SetEntryText(m_aDBData.sCommand, pSelect, ITEMID_TABLE - 1);
            else
                m_pListLB->SetEntryText(OUString(), pSelect, ITEMID_TABLE - 1);
        }
        const OUString sCommand = SvTabListBox::GetEntryText(pSelect, ITEMID_TABLE - 1);
        m_pOK->Enable(pSelect && !sCommand.isEmpty());
        m_pFilterPB->Enable( pUserData->xConnection.is() && !sCommand.isEmpty() );
        m_pTablePB->Enable( pUserData->nTableAndQueryCount > 1 );
    }
    catch (const Exception&)
    {
        OSL_FAIL("exception caught in SwAddressListDialog::DetectTablesAndQueries");
        m_pOK->Enable( false );
    }
}

IMPL_LINK_TYPED(SwAddressListDialog, TableSelectHdl_Impl, Button*, pButton, void)
{
    EnterWait();
    SvTreeListEntry* pSelect = m_pListLB->FirstSelected();
    if(pSelect)
    {
        AddressUserData_Impl* pUserData = static_cast<AddressUserData_Impl*>(pSelect->GetUserData());
        //only call the table select dialog if tables have not been searched for or there
        //are more than 1
        const OUString sTable = SvTabListBox::GetEntryText(pSelect, ITEMID_TABLE - 1);
        if( pUserData->nTableAndQueryCount > 1 || pUserData->nTableAndQueryCount == -1)
        {
            DetectTablesAndQueries(pSelect, (pButton != nullptr) || sTable.isEmpty());
        }
    }

    LeaveWait();
}

IMPL_LINK_NOARG_TYPED(SwAddressListDialog, OKHdl_Impl, Button*, void)
{
    EndDialog(RET_OK);
}

uno::Reference< XDataSource>  SwAddressListDialog::GetSource()
{
    uno::Reference< XDataSource>  xRet;
    SvTreeListEntry* pSelect = m_pListLB->FirstSelected();
    if(pSelect)
    {
        AddressUserData_Impl* pUserData = static_cast<AddressUserData_Impl*>(pSelect->GetUserData());
        xRet = pUserData->xSource;
    }
    return xRet;

}

SharedConnection    SwAddressListDialog::GetConnection()
{
    SharedConnection xRet;
    SvTreeListEntry* pSelect = m_pListLB->FirstSelected();
    if(pSelect)
    {
        AddressUserData_Impl* pUserData = static_cast<AddressUserData_Impl*>(pSelect->GetUserData());
        xRet = pUserData->xConnection;
    }
    return xRet;
}

uno::Reference< XColumnsSupplier> SwAddressListDialog::GetColumnsSupplier()
{
    uno::Reference< XColumnsSupplier> xRet;
    SvTreeListEntry* pSelect = m_pListLB->FirstSelected();
    if(pSelect)
    {
        AddressUserData_Impl* pUserData = static_cast<AddressUserData_Impl*>(pSelect->GetUserData());
        xRet = pUserData->xColumnsSupplier;
    }
    return xRet;
}

OUString     SwAddressListDialog::GetFilter()
{
    SvTreeListEntry* pSelect = m_pListLB->FirstSelected();
    if(pSelect)
    {
        AddressUserData_Impl* pUserData = static_cast<AddressUserData_Impl*>(pSelect->GetUserData());
        return pUserData->sFilter;
    }
    return OUString();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
