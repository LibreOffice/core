/*************************************************************************
 *
 *  $RCSfile: addresslistdialog.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: obo $ $Date: 2005-07-08 10:29:42 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef SW_DLLIMPLEMENTATION
#undef SW_DLLIMPLEMENTATION
#endif

#pragma hdrstop

#ifndef _SWTYPES_HXX
#include <swtypes.hxx>
#endif
#ifndef _ADDRESSLISTDIALOG_HXX
#include <addresslistdialog.hxx>
#endif
#ifndef _SELECTDBTABLEDIALOG_HXX
#include <selectdbtabledialog.hxx>
#endif
#ifndef _CREATEADDRESSLISTDIALOG_HXX
#include <createaddresslistdialog.hxx>
#endif
#ifndef _MAILMERGEWIZARD_HXX
#include <mailmergewizard.hxx>
#endif
#ifndef _MMCONFIGITEM_HXX
#include <mmconfigitem.hxx>
#endif
#ifndef _MAILMERGEADDRESSBLOCKPAGE_HXX
#include <mmaddressblockpage.hxx>
#endif
#ifndef _DBMGR_HXX
#include <dbmgr.hxx>
#endif
#ifndef _DBCONFIG_HXX
#include <dbconfig.hxx>
#endif
#ifndef _UNOTOOLS_TEMPFILE_HXX
#include <unotools/tempfile.hxx>
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif
#ifndef _COM_SUN_STAR_SDBC_XCLOSEABLE_HPP_
#include <com/sun/star/sdbc/XCloseable.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSINGLESERVICEFACTORY_HPP_
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_XNAMINGSERVICE_HPP_
#include <com/sun/star/uno/XNamingService.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XCOMPLETEDCONNECTION_HPP_
#include <com/sun/star/sdb/XCompletedConnection.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_COMMANDTYPE_HPP_
#include <com/sun/star/sdb/CommandType.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XDOCUMENTDATASOURCE_HPP_
#include <com/sun/star/sdb/XDocumentDataSource.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XDATASOURCE_HPP_
#include <com/sun/star/sdbc/XDataSource.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XROWSET_HPP_
#include <com/sun/star/sdbc/XRowSet.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XSQLQUERYCOMPOSERFACTORY_HPP_
#include <com/sun/star/sdb/XSQLQueryComposerFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XTABLESSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XQUERIESSUPPLIER_HPP_
#include <com/sun/star/sdb/XQueriesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_UI_DIALOGS_XEXECUTABLEDIALOG_HPP_
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XSTORABLE_HPP_
#include <com/sun/star/frame/XStorable.hpp>
#endif
#ifndef _SWUNOHELPER_HXX
#include <swunohelper.hxx>
#endif
#ifndef _SV_WAITOBJ_HXX
#include <vcl/waitobj.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_PATHOPTIONS_HXX
#include <svtools/pathoptions.hxx>
#endif
#ifndef SW_SHARED_UNO_COMPONENT_HXX
#include "sharedunocomponent.hxx"
#endif

#include <addresslistdialog.hrc>
#include <dbui.hrc>

#include <helpid.h>

using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::container;
using namespace com::sun::star::sdb;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::sdbcx;
using namespace com::sun::star::task;
using namespace com::sun::star::beans;
using namespace ::com::sun::star::ui::dialogs;
using namespace rtl;

#define C2U(cChar) ::rtl::OUString::createFromAscii(cChar)
#define ITEMID_NAME         1
#define ITEMID_TABLE        2

typedef SharedUNOComponent< XConnection >   SharedConnection;

static const char* cUTF8 = "UTF-8";
/*-- 07.05.2004 14:11:34---------------------------------------------------

  -----------------------------------------------------------------------*/
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
/*-- 07.04.2004 16:35:43---------------------------------------------------

  -----------------------------------------------------------------------*/
SwAddressListDialog::SwAddressListDialog(SwMailMergeAddressBlockPage* pParent) :
    SfxModalDialog(pParent, SW_RES(DLG_MM_ADDRESSLISTDIALOG)),
#pragma warning (disable : 4355)
    m_aDescriptionFI( this, ResId(  FI_DESCRIPTION    )),
    m_aListFT( this, ResId(         FT_LIST           )),
    m_aLoadListPB( this, ResId(     PB_LOADLIST       )),
    m_aCreateListPB(this, ResId(    PB_CREATELIST     )),
    m_aListHB( this, WB_BUTTONSTYLE | WB_BOTTOMBORDER),
    m_aListLB( this, ResId(         LB_LIST           )),
    m_aFilterPB( this, ResId(       PB_FILTER         )),
    m_aEditPB(this, ResId(          PB_EDIT           )),
    m_aTablePB(this, ResId(         PB_TABLE          )),
    m_aSeparatorFL(this, ResId(    FL_SEPARATOR      )),
    m_aOK( this, ResId(             PB_OK             )),
    m_aCancel( this, ResId(         PB_CANCEL         )),
    m_aHelp( this, ResId(           PB_HELP           )),
#pragma warning (default : 4355)
    m_sName(        ResId( ST_NAME )),
    m_sTable(       ResId( ST_TABLE )),
    m_pCreatedDataSource(0),
    m_pAddressPage(pParent),
    m_bInSelectHdl(false)
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
    m_aListLB.SetWindowBits( WB_SORT | WB_HSCROLL | WB_CLIPCHILDREN | WB_TABSTOP );
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

    DBG_ASSERT(m_xDBContext.is(), "service 'com.sun.star.sdb.DatabaseContext' not found!")
    sal_Bool bEnableEdit = sal_False;
    sal_Bool bEnableOK = sal_True;
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
                    bEnableEdit = pUserData->sURL.getLength() > 0 &&
                        !SWUnoHelper::UCB_IsReadOnlyFileName( pUserData->sURL );
                }
                catch(const uno::Exception& rEx)
                {
                    rEx;
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
/*-- 07.04.2004 16:35:43---------------------------------------------------

  -----------------------------------------------------------------------*/
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
/*-- 07.04.2004 16:35:44---------------------------------------------------

  -----------------------------------------------------------------------*/
IMPL_LINK(SwAddressListDialog, FilterHdl_Impl, PushButton*, pButton)
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
                uno::Reference<XSQLQueryComposerFactory> xQFact(pUserData->xConnection, UNO_QUERY_THROW);
                uno::Reference<XSQLQueryComposer> xComposer = xQFact->createQueryComposer();


                PropertyValue aSecond;
                aSecond.Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "RowSet" ) );
                uno::Reference<XRowSet> xRowSet(
                        xMgr->createInstance(C2U("com.sun.star.sdb.RowSet")), UNO_QUERY);
                uno::Reference<XPropertySet> xRowProperties(xRowSet, UNO_QUERY);
                xRowProperties->setPropertyValue(C2U("DataSourceName"),
                        makeAny(OUString(m_aListLB.GetEntryText(pSelect, ITEMID_NAME - 1))));
                xRowProperties->setPropertyValue(C2U("Command"), makeAny(
                        OUString()));
                xRowProperties->setPropertyValue(C2U("CommandType"), makeAny(pUserData->nCommandType));
                xRowProperties->setPropertyValue(C2U("ActiveConnection"), makeAny(pUserData->xConnection.getTyped()));
                xRowSet->execute();
                aSecond.Value <<= xRowSet;

                PropertyValue aFirst;
                aFirst.Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "QueryComposer" ) );
                ::rtl::OUString sQuery;
                xRowProperties->getPropertyValue(C2U("ActiveCommand"))>>= sQuery;
                xComposer->setQuery(sQuery);
                if(pUserData->sFilter.getLength())
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
            catch(Exception& rEx)
            {
                DBG_ERROR("exception caught in SwAddressListDialog::FilterHdl_Impl")
                rEx;
            }
        }
    }
    return 0;
}
/*-- 07.04.2004 16:35:44---------------------------------------------------

  -----------------------------------------------------------------------*/
IMPL_LINK(SwAddressListDialog, LoadHdl_Impl,   PushButton*, pButton)
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
/*-- 07.04.2004 16:35:44---------------------------------------------------

  -----------------------------------------------------------------------*/
IMPL_LINK(SwAddressListDialog, CreateHdl_Impl, PushButton*, pButton)
{
    String sURL;
    SwCreateAddressListDialog* pDlg =
            new SwCreateAddressListDialog(
                    pButton,
                    sURL,
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
            pInfo[0].Value <<= OUString(String('\t'));
            pInfo[1].Name = C2U("StringDelimiter");
            pInfo[1].Value <<= OUString('"');
            pInfo[2].Name = C2U("Extension");
            pInfo[2].Value <<= ::rtl::OUString(aURL.getExtension());//C2U("csv");
            pInfo[3].Name = C2U("CharSet");
            pInfo[3].Value <<= C2U(cUTF8);
            aAny <<= aInfo;
            xDataProperties->setPropertyValue(C2U("Info"), aAny);

            uno::Reference<sdb::XDocumentDataSource> xDS(xNewInstance, UNO_QUERY_THROW);
            uno::Reference<frame::XStorable> xStore(xDS->getDatabaseDocument(), UNO_QUERY_THROW);
            String sExt = String::CreateFromAscii(".odb");
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
            m_aCreateListPB.Enable(FALSE);

        }
        catch(Exception& rEx)
        {
            rEx;
        }
    }
    delete pDlg;
    return 0;
}
/*-- 22.04.2004 10:30:40---------------------------------------------------

  -----------------------------------------------------------------------*/
IMPL_LINK(SwAddressListDialog, EditHdl_Impl, PushButton*, pButton)
{
    SvLBoxEntry* pEntry = m_aListLB.FirstSelected();
    AddressUserData_Impl* pUserData = pEntry ? static_cast<AddressUserData_Impl*>(pEntry->GetUserData()) : 0;
    if(pUserData && pUserData->sURL.getLength())
    {
        if(pUserData->xResultSet.is())
        {
            SwMailMergeConfigItem& rConfigItem = m_pAddressPage->GetWizard()->GetConfigItem();
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
/*-- 19.04.2004 09:41:05---------------------------------------------------

  -----------------------------------------------------------------------*/
IMPL_LINK(SwAddressListDialog, ListBoxSelectHdl_Impl, SvTabListBox*, EMPTYARG)
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
        pUserData = static_cast<AddressUserData_Impl*>(pSelect->GetUserData());
        if(pUserData->nTableAndQueryCount > 1 || pUserData->nTableAndQueryCount == -1)
        {
            pThis->DetectTablesAndQueries(pSelect, false);
        }
        else
        {
            //otherwise set the selected db-data
            pThis->m_aDBData.sDataSource = pThis->m_aListLB.GetEntryText(pSelect, ITEMID_NAME - 1);
            pThis->m_aDBData.sCommand = pThis->m_aListLB.GetEntryText(pSelect, ITEMID_TABLE - 1);
            pThis->m_aDBData.nCommandType = pUserData->nCommandType;
        }
    }
    pThis->m_aEditPB.Enable(pUserData && pUserData->sURL.getLength() &&
                    !SWUnoHelper::UCB_IsReadOnlyFileName( pUserData->sURL ) );
    pThis->m_bInSelectHdl = false;
    pThis->LeaveWait();
    return 0;
}

/*-- 13.05.2004 14:59:25---------------------------------------------------
    detect the number of tables for a data source
    if only one is available then set it at the entry
  -----------------------------------------------------------------------*/
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
                xMgr->createInstance( C2U( "com.sun.star.sdb.InteractionHandler" )), UNO_QUERY);
            pUserData->xConnection = xComplConnection->connectWithCompletion( xHandler );
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
        if ( m_aDBData.sCommand.getLength() )
        {
            uno::Reference<beans::XPropertySet> xSourceProperties;
            m_xDBContext->getByName(m_aDBData.sDataSource) >>= xSourceProperties;
            pUserData->sURL = lcl_getFlatURL( xSourceProperties );

            m_aListLB.SetEntryText(m_aDBData.sCommand, pSelect, ITEMID_TABLE - 1);
            pUserData->xColumnsSupplier = SwNewDBMgr::GetColumnSupplier(pUserData->xConnection,
                                    m_aDBData.sCommand,
                                    m_aDBData.nCommandType == CommandType::TABLE ?
                                            SW_DB_SELECT_TABLE : SW_DB_SELECT_QUERY );
        }
        String sCommand = m_aListLB.GetEntryText(pSelect, ITEMID_TABLE - 1);
        m_aOK.Enable(pSelect && sCommand.Len());
        m_aFilterPB.Enable( pUserData->xConnection.is() && sCommand.Len() );
        m_aTablePB.Enable( pUserData->nTableAndQueryCount > 1 );
    }
    catch(Exception& rEx)
    {
        DBG_ERROR("exception caught in SwAddressListDialog::DetectTablesAndQueries")
        m_aOK.Enable( sal_False );
        rEx;
    }
}

/*-- 13.05.2004 12:55:40---------------------------------------------------

  -----------------------------------------------------------------------*/
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
            DetectTablesAndQueries(pSelect, true);
        }
    }

    LeaveWait();
    return 0;
}

/*-- 08.04.2004 14:52:11---------------------------------------------------

  -----------------------------------------------------------------------*/
IMPL_LINK(SwAddressListDialog, OKHdl_Impl, PushButton*, pButton)
{
    EndDialog(TRUE);
    return 0;
}

/*-- 07.05.2004 14:17:47---------------------------------------------------

  -----------------------------------------------------------------------*/
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
/*-- 07.05.2004 14:17:48---------------------------------------------------

  -----------------------------------------------------------------------*/
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
/*-- 07.05.2004 14:17:48---------------------------------------------------

  -----------------------------------------------------------------------*/
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
/*-- 14.05.2004 15:04:09---------------------------------------------------

  -----------------------------------------------------------------------*/
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
