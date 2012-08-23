/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
#include <com/sun/star/sdbc/XCloseable.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/uno/XNamingService.hpp>
#include <com/sun/star/sdb/XCompletedConnection.hpp>
#include <com/sun/star/sdb/CommandType.hpp>
#include <com/sun/star/sdb/XDocumentDataSource.hpp>
#include <com/sun/star/sdbc/XRowSet.hpp>
#include <com/sun/star/sdb/XSingleSelectQueryComposer.hpp>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <com/sun/star/sdb/XQueriesSupplier.hpp>
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <swunohelper.hxx>
#include <vcl/waitobj.hxx>
#include <unotools/pathoptions.hxx>
#include <svl/urihelper.hxx>
#include <addresslistdialog.hrc>
#include <dbui.hrc>

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
using namespace ::rtl;

#define ITEMID_NAME         1
#define ITEMID_TABLE        2

static const char* cUTF8 = "UTF-8";

struct AddressUserData_Impl
{
    uno::Reference<XDataSource>             xSource;
    SharedConnection                        xConnection;
    uno::Reference< XColumnsSupplier>       xColumnsSupplier;
    uno::Reference< sdbc::XResultSet>       xResultSet;
    ::rtl::OUString                    sFilter;
    ::rtl::OUString                    sURL; // data is editable
    sal_Int32                          nCommandType;
    sal_Int32                          nTableAndQueryCount;
    AddressUserData_Impl() :
        nCommandType(0),
        nTableAndQueryCount(-1)
        {}
};

::rtl::OUString lcl_getFlatURL( uno::Reference<beans::XPropertySet>& xSourceProperties )
{
    ::rtl::OUString sURL;
    if(xSourceProperties.is())
    {
        rtl::OUString sDBURL;
        xSourceProperties->getPropertyValue(C2U("URL")) >>= sDBURL;
        if(String(sDBURL).SearchAscii("sdbc:flat:") == 0)
        {
            uno::Sequence<OUString> aFilters;
            xSourceProperties->getPropertyValue(C2U("TableFilter")) >>= aFilters;
            uno::Sequence<PropertyValue> aInfo;
            xSourceProperties->getPropertyValue(C2U("Info")) >>= aInfo;
            if(aFilters.getLength() == 1 && aInfo.getLength() )
            {
                ::rtl::OUString sFieldDelim;
                ::rtl::OUString sStringDelim;
                ::rtl::OUString sExtension;
                ::rtl::OUString sCharSet;
                for(sal_Int32 nInfo = 0; nInfo < aInfo.getLength(); ++nInfo)
                {
                    if(aInfo[nInfo].Name == C2U("FieldDelimiter"))
                        aInfo[nInfo].Value >>= sFieldDelim;
                    else if(aInfo[nInfo].Name == C2U("StringDelimiter"))
                        aInfo[nInfo].Value >>= sStringDelim;
                    else if(aInfo[nInfo].Name == C2U("Extension"))
                        aInfo[nInfo].Value >>= sExtension;
                    else if(aInfo[nInfo].Name == C2U("CharSet"))
                        aInfo[nInfo].Value >>= sCharSet;
                }
                if(!sCharSet.compareToAscii( cUTF8 ))
                {
                    sURL = String(sDBURL).Copy( 10 );
                    //#i97577# at this point the 'URL' can also be a file name!
                    sURL = URIHelper::SmartRel2Abs( INetURLObject(), sURL );
                    sURL += C2U("/");
                    sURL += aFilters[0];
                    sURL += C2U(".");
                    sURL += sExtension;
                }
            }
        }
    }
    return sURL;
}

SwAddressListDialog::SwAddressListDialog(SwMailMergeAddressBlockPage* pParent) :
    SfxModalDialog(pParent, SW_RES(DLG_MM_ADDRESSLISTDIALOG)),
#ifdef MSC
#pragma warning (disable : 4355)
#endif
    m_aDescriptionFI( this, SW_RES(  FI_DESCRIPTION    )),
    m_aListFT( this, SW_RES(         FT_LIST           )),
    m_aListHB( this, WB_BUTTONSTYLE | WB_BOTTOMBORDER),
    m_aListLB( this, SW_RES(         LB_LIST           )),
    m_aLoadListPB( this, SW_RES(     PB_LOADLIST       )),
    m_aCreateListPB(this, SW_RES(    PB_CREATELIST     )),
    m_aFilterPB( this, SW_RES(       PB_FILTER         )),
    m_aEditPB(this, SW_RES(          PB_EDIT           )),
    m_aTablePB(this, SW_RES(         PB_TABLE          )),
    m_aSeparatorFL(this, SW_RES(    FL_SEPARATOR      )),
    m_aOK( this, SW_RES(             PB_OK             )),
    m_aCancel( this, SW_RES(         PB_CANCEL         )),
    m_aHelp( this, SW_RES(           PB_HELP           )),
#ifdef MSC
#pragma warning (default : 4355)
#endif
    m_sName(        SW_RES( ST_NAME )),
    m_sTable(       SW_RES( ST_TABLE )),
    m_sConnecting(  SW_RES( ST_CONNECTING )),
    m_pCreatedDataSource(0),
    m_bInSelectHdl(false),
    m_pAddressPage(pParent)
{
    FreeResource();
    String sTemp(m_aDescriptionFI.GetText());
    sTemp.SearchAndReplaceAscii("%1", m_aLoadListPB.GetText());
    sTemp.SearchAndReplaceAscii("%2", m_aCreateListPB.GetText());
    m_aDescriptionFI.SetText(sTemp);
    m_aFilterPB.SetClickHdl( LINK( this, SwAddressListDialog,    FilterHdl_Impl ));
    m_aLoadListPB.SetClickHdl( LINK( this, SwAddressListDialog,  LoadHdl_Impl ));
    m_aCreateListPB.SetClickHdl( LINK( this, SwAddressListDialog,CreateHdl_Impl ));
    m_aEditPB.SetClickHdl(LINK( this, SwAddressListDialog, EditHdl_Impl));
    m_aTablePB.SetClickHdl(LINK( this, SwAddressListDialog, TableSelectHdl_Impl));

    Size aLBSize(m_aListLB.GetSizePixel());
    m_aListHB.SetSizePixel(aLBSize);
    Size aHeadSize(m_aListHB.CalcWindowSizePixel());
    aHeadSize.Width() = aLBSize.Width();
    m_aListHB.SetSizePixel(aHeadSize);
    Point aLBPos(m_aListLB.GetPosPixel());
    m_aListHB.SetPosPixel(aLBPos);
    aLBPos.Y() += aHeadSize.Height();
    aLBSize.Height() -= aHeadSize.Height();
    m_aListLB.SetPosSizePixel(aLBPos, aLBSize);

    Size aSz(m_aListHB.GetOutputSizePixel());
    m_aListHB.InsertItem( ITEMID_NAME, m_sName,
                            aSz.Width()/2,
                            HIB_LEFT | HIB_VCENTER | HIB_FIXED | HIB_FIXEDPOS/*| HIB_CLICKABLE | HIB_UPARROW */);
    m_aListHB.InsertItem( ITEMID_TABLE, m_sTable,
                            aSz.Width()/2,
                            HIB_LEFT | HIB_VCENTER | HIB_FIXED | HIB_FIXEDPOS /*| HIB_CLICKABLE | HIB_UPARROW */);
    m_aListHB.SetHelpId(HID_MM_ADDRESSLIST_HB );
    m_aListHB.Show();

    m_aListLB.SetHelpId(HID_MM_ADDRESSLIST_TLB);
    static long nTabs[] = {2, 0, aSz.Width()/2 };
    m_aListLB.SetStyle( m_aListLB.GetStyle() | WB_SORT | WB_HSCROLL | WB_CLIPCHILDREN | WB_TABSTOP );
    m_aListLB.SetSelectionMode( SINGLE_SELECTION );
    m_aListLB.SetTabs(&nTabs[0], MAP_PIXEL);
    m_aOK.SetClickHdl( LINK( this, SwAddressListDialog, OKHdl_Impl));

    uno::Reference< XMultiServiceFactory > xMgr( ::comphelper::getProcessServiceFactory() );
    if( xMgr.is() )
    {
        uno::Reference<XInterface> xInstance = xMgr->createInstance( C2U( "com.sun.star.sdb.DatabaseContext" ));
        m_xDBContext = uno::Reference<XNameAccess>(xInstance, UNO_QUERY) ;
    }
    SwMailMergeConfigItem& rConfigItem = m_pAddressPage->GetWizard()->GetConfigItem();
    const SwDBData& rCurrentData = rConfigItem.GetCurrentDBData();

    OSL_ENSURE(m_xDBContext.is(), "service 'com.sun.star.sdb.DatabaseContext' not found!");
    sal_Bool bEnableEdit = sal_False;
    sal_Bool bEnableOK = sal_True;
    m_aListLB.SelectAll( sal_False );

    if(m_xDBContext.is())
    {
        SwDBConfig aDb;
        ::rtl::OUString sBibliography = aDb.GetBibliographySource().sDataSource;
        uno::Sequence< ::rtl::OUString> aNames = m_xDBContext->getElementNames();
        const ::rtl::OUString* pNames = aNames.getConstArray();
        for(sal_Int32 nName = 0; nName < aNames.getLength(); ++nName)
        {
            if ( pNames[nName] == sBibliography )
                continue;
            SvLBoxEntry* pEntry = m_aListLB.InsertEntry(pNames[nName]);
            AddressUserData_Impl* pUserData = new AddressUserData_Impl();
            pEntry->SetUserData(pUserData);
            if(pNames[nName] == rCurrentData.sDataSource)
            {
                m_aListLB.Select(pEntry);
                m_aListLB.SetEntryText(rCurrentData.sCommand, pEntry, ITEMID_TABLE - 1);
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
                    bEnableOK = sal_False;
                }
                m_aDBData = rCurrentData;
            }
        }
    }
    m_aOK.Enable(m_aListLB.GetEntryCount()>0 && bEnableOK);
    m_aEditPB.Enable(bEnableEdit);
    m_aListLB.SetSelectHdl(LINK(this, SwAddressListDialog, ListBoxSelectHdl_Impl));
    TableSelectHdl_Impl(NULL);
}

SwAddressListDialog::~SwAddressListDialog()
{
    SvLBoxEntry* pEntry = m_aListLB.First();
    while(pEntry)
    {
        AddressUserData_Impl* pUserData = static_cast<AddressUserData_Impl*>(pEntry->GetUserData());
        delete pUserData;
        pEntry = m_aListLB.Next( pEntry );
    }
}

IMPL_LINK_NOARG(SwAddressListDialog, FilterHdl_Impl)
{
    SvLBoxEntry* pSelect = m_aListLB.FirstSelected();
    uno::Reference< XMultiServiceFactory > xMgr( ::comphelper::getProcessServiceFactory() );
    if(pSelect && xMgr.is())
    {
        String sCommand = m_aListLB.GetEntryText(pSelect, ITEMID_TABLE - 1);
        if ( !sCommand.Len() )
            return 0;

        AddressUserData_Impl* pUserData = static_cast<AddressUserData_Impl*>(pSelect->GetUserData());
        if(pUserData->xConnection.is() )
        {
            try
            {
                uno::Reference<lang::XMultiServiceFactory> xConnectFactory(pUserData->xConnection, UNO_QUERY_THROW);
                uno::Reference<XSingleSelectQueryComposer> xComposer(
                        xConnectFactory->createInstance(C2U("com.sun.star.sdb.SingleSelectQueryComposer")), UNO_QUERY_THROW);

                PropertyValue aSecond;
                aSecond.Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "RowSet" ) );
                uno::Reference<XRowSet> xRowSet(
                        xMgr->createInstance(C2U("com.sun.star.sdb.RowSet")), UNO_QUERY);
                uno::Reference<XPropertySet> xRowProperties(xRowSet, UNO_QUERY);
                xRowProperties->setPropertyValue(C2U("DataSourceName"),
                        makeAny(OUString(m_aListLB.GetEntryText(pSelect, ITEMID_NAME - 1))));
                xRowProperties->setPropertyValue(C2U("Command"), makeAny(
                        OUString(sCommand)));
                xRowProperties->setPropertyValue(C2U("CommandType"), makeAny(pUserData->nCommandType));
                xRowProperties->setPropertyValue(C2U("ActiveConnection"), makeAny(pUserData->xConnection.getTyped()));
                xRowSet->execute();
                aSecond.Value <<= xRowSet;

                PropertyValue aFirst;
                aFirst.Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "QueryComposer" ) );
                ::rtl::OUString sQuery;
                xRowProperties->getPropertyValue(C2U("ActiveCommand"))>>= sQuery;
                xComposer->setQuery(sQuery);
                if(!pUserData->sFilter.isEmpty())
                    xComposer->setFilter(pUserData->sFilter);
                aFirst.Value <<= xComposer;

                uno::Sequence<Any> aInit(2);
                aInit[0] <<= aFirst;
                aInit[1] <<= aSecond;

                ::rtl::OUString sDialogServiceName( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.sdb.FilterDialog" ) );
                uno::Reference< XExecutableDialog> xDialog(
                        xMgr->createInstanceWithArguments( sDialogServiceName, aInit ), UNO_QUERY);

                if ( RET_OK == xDialog->execute() )
                {
                    WaitObject aWO( NULL );
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
    return 0;
}

IMPL_LINK_NOARG(SwAddressListDialog, LoadHdl_Impl)
{
    String sNewSource = SwNewDBMgr::LoadAndRegisterDataSource();
    if(sNewSource.Len())
    {
        SvLBoxEntry* pNewSource = m_aListLB.InsertEntry(sNewSource);
        pNewSource->SetUserData(new AddressUserData_Impl());
        m_aListLB.Select(pNewSource);
    }
    return 0;
}

IMPL_LINK(SwAddressListDialog, CreateHdl_Impl, PushButton*, pButton)
{
    String sInputURL;
    SwCreateAddressListDialog* pDlg =
            new SwCreateAddressListDialog(
                    pButton,
                    sInputURL,
                    m_pAddressPage->GetWizard()->GetConfigItem());
    if(RET_OK == pDlg->Execute())
    {
        //register the URL a new datasource
        OUString sURL = pDlg->GetURL();
        try
        {
            uno::Reference<XSingleServiceFactory> xFact( m_xDBContext, UNO_QUERY);
            uno::Reference<XInterface> xNewInstance = xFact->createInstance();
            INetURLObject aURL( sURL );
            OUString sNewName = aURL.getBase();
            //find a unique name if sNewName already exists
            OUString sFind(sNewName);
            sal_Int32 nIndex = 0;
            while(m_xDBContext->hasByName(sFind))
            {
                sFind = sNewName;
                sFind += OUString::valueOf(++nIndex);
            }
            uno::Reference<XPropertySet> xDataProperties(xNewInstance, UNO_QUERY);

            OUString sDBURL(C2U("sdbc:flat:"));
            //only the 'path' has to be added
            INetURLObject aTempURL(aURL);
            aTempURL.removeSegment();
            aTempURL.removeFinalSlash();
            sDBURL += aTempURL.GetMainURL(INetURLObject::NO_DECODE);
            Any aAny(&sDBURL, ::getCppuType(&sDBURL));
            xDataProperties->setPropertyValue(C2U("URL"), aAny);
            //set the filter to the file name without extension
            uno::Sequence<OUString> aFilters(1);
            aFilters[0] = sNewName;
            aAny <<= aFilters;
            xDataProperties->setPropertyValue(C2U("TableFilter"), aAny);

            uno::Sequence<PropertyValue> aInfo(4);
            PropertyValue* pInfo = aInfo.getArray();
            pInfo[0].Name = C2U("FieldDelimiter");
            pInfo[0].Value <<= OUString('\t');
            pInfo[1].Name = C2U("StringDelimiter");
            pInfo[1].Value <<= OUString('"');
            pInfo[2].Name = C2U("Extension");
            pInfo[2].Value <<= ::rtl::OUString(aURL.getExtension());//C2U("csv");
            pInfo[3].Name = C2U("CharSet");
            pInfo[3].Value <<= rtl::OUString::createFromAscii(cUTF8);
            aAny <<= aInfo;
            xDataProperties->setPropertyValue(C2U("Info"), aAny);

            uno::Reference<sdb::XDocumentDataSource> xDS(xNewInstance, UNO_QUERY_THROW);
            uno::Reference<frame::XStorable> xStore(xDS->getDatabaseDocument(), UNO_QUERY_THROW);
            String sExt = rtl::OUString(".odb");
            String sTmpName;
            {
                String sHomePath(SvtPathOptions().GetWorkPath());
                utl::TempFile aTempFile(sFind , &sExt, &sHomePath);
                aTempFile.EnableKillingFile(sal_True);
                sTmpName = aTempFile.GetURL();
            }
            xStore->storeAsURL(sTmpName, Sequence< PropertyValue >());


            uno::Reference<XNamingService> xNaming(m_xDBContext, UNO_QUERY);
            xNaming->registerObject( sFind, xNewInstance );
            //now insert the new source into the ListBox
            String sEntry(sFind);
            sEntry += '\t';
            sEntry += String(aFilters[0]);
            m_pCreatedDataSource = m_aListLB.InsertEntry(sEntry);
            AddressUserData_Impl* pUserData = new AddressUserData_Impl();
            pUserData->sURL = sURL;
            m_pCreatedDataSource->SetUserData(pUserData);
            m_aListLB.Select(m_pCreatedDataSource);
            m_aCreateListPB.Enable(sal_False);

        }
        catch (const Exception&)
        {
        }
    }
    delete pDlg;
    return 0;
}

IMPL_LINK(SwAddressListDialog, EditHdl_Impl, PushButton*, pButton)
{
    SvLBoxEntry* pEntry = m_aListLB.FirstSelected();
    AddressUserData_Impl* pUserData = pEntry ? static_cast<AddressUserData_Impl*>(pEntry->GetUserData()) : 0;
    if(pUserData && !pUserData->sURL.isEmpty())
    {
        if(pUserData->xResultSet.is())
        {
            SwMailMergeConfigItem& rConfigItem = m_pAddressPage->GetWizard()->GetConfigItem();
            if(rConfigItem.GetResultSet() != pUserData->xResultSet)
                ::comphelper::disposeComponent( pUserData->xResultSet );
            pUserData->xResultSet = 0;

            rConfigItem.DisposeResultSet();
        }
        pUserData->xSource.clear();
        pUserData->xColumnsSupplier.clear();
        pUserData->xConnection.clear();
            // will automatically close if it was the las reference
        SwCreateAddressListDialog* pDlg =
                new SwCreateAddressListDialog(
                        pButton,
                        pUserData->sURL,
                        m_pAddressPage->GetWizard()->GetConfigItem());
        if(RET_OK == pDlg->Execute())
        {
        }
        delete pDlg;
    }
    return 0;
};

IMPL_LINK_NOARG(SwAddressListDialog, ListBoxSelectHdl_Impl)
{
    SvLBoxEntry* pSelect = m_aListLB.FirstSelected();
    Application::PostUserEvent( STATIC_LINK( this, SwAddressListDialog,
                                                StaticListBoxSelectHdl_Impl ), pSelect );
    return 0;
}

IMPL_STATIC_LINK(SwAddressListDialog, StaticListBoxSelectHdl_Impl, SvLBoxEntry*, pSelect)
{
    //prevent nested calls of the select handler
    if(pThis->m_bInSelectHdl)
        return 0;
    pThis->EnterWait();
    pThis->m_bInSelectHdl = true;
    AddressUserData_Impl* pUserData = 0;
    if(pSelect)
    {
        String sTable = pThis->m_aListLB.GetEntryText(pSelect, ITEMID_TABLE - 1);
        if(!sTable.Len())
        {
            pThis->m_aListLB.SetEntryText(pThis->m_sConnecting, pSelect, ITEMID_TABLE - 1);
            // allow painting of the new entry
            pThis->m_aListLB.Window::Invalidate(INVALIDATE_UPDATE);
            for (sal_uInt16 i = 0; i < 10; i++)
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
            pThis->m_aListLB.EndSelection();
            pThis->DetectTablesAndQueries(pSelect, !sTable.Len());
        }
        else
        {
            //otherwise set the selected db-data
            pThis->m_aDBData.sDataSource = pThis->m_aListLB.GetEntryText(pSelect, ITEMID_NAME - 1);
            pThis->m_aDBData.sCommand = pThis->m_aListLB.GetEntryText(pSelect, ITEMID_TABLE - 1);
            pThis->m_aDBData.nCommandType = pUserData->nCommandType;
            pThis->m_aOK.Enable(sal_True);
        }
        sTable = pThis->m_aListLB.GetEntryText(pSelect, ITEMID_TABLE - 1);
        if(sTable == pThis->m_sConnecting)
           pThis->m_aListLB.SetEntryText(String(), pSelect, ITEMID_TABLE - 1);
    }
    pThis->m_aEditPB.Enable(pUserData && !pUserData->sURL.isEmpty() &&
                    SWUnoHelper::UCB_IsFile( pUserData->sURL ) && //#i97577#
                    !SWUnoHelper::UCB_IsReadOnlyFileName( pUserData->sURL ) );
    pThis->m_bInSelectHdl = false;
    pThis->LeaveWait();
    return 0;
}

// detect the number of tables for a data source
// if only one is available then set it at the entry
void SwAddressListDialog::DetectTablesAndQueries(
        SvLBoxEntry* pSelect,
        bool bWidthDialog)
{
    try
    {
        AddressUserData_Impl* pUserData = static_cast<AddressUserData_Impl*>(pSelect->GetUserData());
        uno::Reference<XCompletedConnection> xComplConnection;
        if(!pUserData->xConnection.is())
        {
            m_aDBData.sDataSource = m_aListLB.GetEntryText(pSelect, ITEMID_NAME - 1);
            m_xDBContext->getByName(m_aDBData.sDataSource) >>= xComplConnection;
            pUserData->xSource = uno::Reference<XDataSource>(xComplConnection, UNO_QUERY);

            uno::Reference< XMultiServiceFactory > xMgr( ::comphelper::getProcessServiceFactory() );
            uno::Reference< XInteractionHandler > xHandler(
                xMgr->createInstance( C2U( "com.sun.star.task.InteractionHandler" )), UNO_QUERY);
            pUserData->xConnection = SharedConnection( xComplConnection->connectWithCompletion( xHandler ) );
        }
        if(pUserData->xConnection.is())
        {
            sal_Int32 nTables = 0;
            uno::Sequence<rtl::OUString> aTables;
            uno::Sequence<rtl::OUString> aQueries;
            uno::Reference<XTablesSupplier> xTSupplier(pUserData->xConnection, UNO_QUERY);
            if(xTSupplier.is())
            {
                uno::Reference<XNameAccess> xTbls = xTSupplier->getTables();
                aTables = xTbls->getElementNames();
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
                SwSelectDBTableDialog* pDlg = new SwSelectDBTableDialog(this, pUserData->xConnection);
                String sTable = m_aListLB.GetEntryText(pSelect, ITEMID_TABLE - 1);
                if(sTable.Len())
                    pDlg->SetSelectedTable(sTable, pUserData->nCommandType == CommandType::TABLE);
                if(RET_OK == pDlg->Execute())
                {
                    bool bIsTable;
                    m_aDBData.sCommand = pDlg->GetSelectedTable(bIsTable);
                    m_aDBData.nCommandType = bIsTable ? CommandType::TABLE : CommandType::QUERY;
                    pUserData->nCommandType = m_aDBData.nCommandType;
                }
                delete pDlg;
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

            pUserData->xColumnsSupplier = SwNewDBMgr::GetColumnSupplier(pUserData->xConnection,
                                    m_aDBData.sCommand,
                                    m_aDBData.nCommandType == CommandType::TABLE ?
                                            SW_DB_SELECT_TABLE : SW_DB_SELECT_QUERY );
            //#i97577#
            if( pUserData->xColumnsSupplier.is() )
                m_aListLB.SetEntryText(m_aDBData.sCommand, pSelect, ITEMID_TABLE - 1);
            else
                m_aListLB.SetEntryText(String(), pSelect, ITEMID_TABLE - 1);
        }
        String sCommand = m_aListLB.GetEntryText(pSelect, ITEMID_TABLE - 1);
        m_aOK.Enable(pSelect && sCommand.Len());
        m_aFilterPB.Enable( pUserData->xConnection.is() && sCommand.Len() );
        m_aTablePB.Enable( pUserData->nTableAndQueryCount > 1 );
    }
    catch (const Exception&)
    {
        OSL_FAIL("exception caught in SwAddressListDialog::DetectTablesAndQueries");
        m_aOK.Enable( sal_False );
    }
}

IMPL_LINK(SwAddressListDialog, TableSelectHdl_Impl, PushButton*, pButton)
{
    EnterWait();
    SvLBoxEntry* pSelect = m_aListLB.FirstSelected();
    if(pSelect)
    {
        AddressUserData_Impl* pUserData = static_cast<AddressUserData_Impl*>(pSelect->GetUserData());
        //only call the table select dialog if tables have not been searched for or there
        //are more than 1
        String sTable = m_aListLB.GetEntryText(pSelect, ITEMID_TABLE - 1);
        if( pUserData->nTableAndQueryCount > 1 || pUserData->nTableAndQueryCount == -1)
        {
            DetectTablesAndQueries(pSelect, (pButton != 0) || (!sTable.Len()));
        }
    }

    LeaveWait();
    return 0;
}

IMPL_LINK_NOARG(SwAddressListDialog, OKHdl_Impl)
{
    EndDialog(sal_True);
    return 0;
}

uno::Reference< XDataSource>  SwAddressListDialog::GetSource()
{
    uno::Reference< XDataSource>  xRet;
    SvLBoxEntry* pSelect = m_aListLB.FirstSelected();
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
    SvLBoxEntry* pSelect = m_aListLB.FirstSelected();
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
    SvLBoxEntry* pSelect = m_aListLB.FirstSelected();
    if(pSelect)
    {
        AddressUserData_Impl* pUserData = static_cast<AddressUserData_Impl*>(pSelect->GetUserData());
        xRet = pUserData->xColumnsSupplier;
    }
    return xRet;
}

::rtl::OUString     SwAddressListDialog::GetFilter()
{
    ::rtl::OUString sRet;
    SvLBoxEntry* pSelect = m_aListLB.FirstSelected();
    if(pSelect)
    {
        AddressUserData_Impl* pUserData = static_cast<AddressUserData_Impl*>(pSelect->GetUserData());
        sRet = pUserData->sFilter;
    }
    return sRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
