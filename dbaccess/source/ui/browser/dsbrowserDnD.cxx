/*************************************************************************
 *
 *  $RCSfile: dsbrowserDnD.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: oj $ $Date: 2001-03-27 07:09:19 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _SBA_UNODATBR_HXX_
#include "unodatbr.hxx"
#endif

#ifndef _COM_SUN_STAR_SDB_XQUERIESSUPPLIER_HPP_
#include <com/sun/star/sdb/XQueriesSupplier.hpp>
#endif

#ifndef _COM_SUN_STAR_SDB_COMMANDTYPE_HPP_
#include <com/sun/star/sdb/CommandType.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_DATATYPE_HPP_
#include <com/sun/star/sdbc/DataType.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XCONNECTION_HPP_
#include <com/sun/star/sdbc/XConnection.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XTABLESSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XRESULTSETMETADATASUPPLIER_HPP_
#include <com/sun/star/sdbc/XResultSetMetaDataSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XQUERYDEFINITIONSSUPPLIER_HPP_
#include <com/sun/star/sdb/XQueryDefinitionsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSINGLESERVICEFACTORY_HPP_
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_SQLCONTEXT_HPP_
#include <com/sun/star/sdb/SQLContext.hpp>
#endif
#ifndef DBAUI_DBTREEMODEL_HXX
#include "dbtreemodel.hxx"
#endif
#ifndef DBACCESS_UI_DBTREEVIEW_HXX
#include "dbtreeview.hxx"
#endif
#ifndef DBACCESS_SHARED_DBUSTRINGS_HRC
#include "dbustrings.hrc"
#endif
#ifndef _DBU_RESOURCE_HRC_
#include "dbu_resource.hrc"
#endif
#ifndef _DBAUI_MODULE_DBU_HXX_
#include "moduledbu.hxx"
#endif
#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif
#ifndef _DBHELPER_DBEXCEPTION_HXX_
#include <connectivity/dbexception.hxx>
#endif
#ifndef DBAUI_DBTREELISTBOX_HXX
#include "dbtreelistbox.hxx"
#endif
#ifndef DBAUI_TOKENWRITER_HXX
#include "TokenWriter.hxx"
#endif
#ifndef DBAUI_DBEXCHANGE_HXX
#include "dbexchange.hxx"
#endif
#ifndef _DBACCESS_DBATOOLS_HXX_
#include "dbatools.hxx"
#endif
#ifndef DBAUI_WIZ_COPYTABLEDIALOG_HXX
#include "WCopyTable.hxx"
#endif
#ifndef DBAUI_WIZ_EXTENDPAGES_HXX
#include "WExtendPages.hxx"
#endif
#ifndef DBAUI_WIZ_NAMEMATCHING_HXX
#include "WNameMatch.hxx"
#endif
#ifndef DBAUI_WIZ_COLUMNSELECT_HXX
#include "WColumnSelect.hxx"
#endif
#ifndef DBAUI_ENUMTYPES_HXX
#include "QEnumTypes.hxx"
#endif
#ifndef DBAUI_WIZARD_CPAGE_HXX
#include "WCPage.hxx"
#endif
#ifndef DBAUI_TOOLS_HXX
#include "UITools.hxx"
#endif
#ifndef DBAUI_RTFREADER_HXX
#include "RtfReader.hxx"
#endif
#ifndef DBAUI_HTMLREADER_HXX
#include "HtmlReader.hxx"
#endif
#ifndef DBAUI_DLGSAVE_HXX
#include "dlgsave.hxx"
#endif
#ifndef _SOT_STORAGE_HXX
#include <sot/storage.hxx>
#endif

// .........................................................................
namespace dbaui
{
// .........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::sdb;
    using namespace ::com::sun::star::sdbc;
    using namespace ::com::sun::star::sdbcx;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::util;
    using namespace ::com::sun::star::frame;
    using namespace ::com::sun::star::container;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::form;
    using namespace ::com::sun::star::io;
    using namespace ::com::sun::star::i18n;
    using namespace ::com::sun::star::datatransfer;
    using namespace ::dbtools;

    // -----------------------------------------------------------------------------
    void SbaTableQueryBrowser::implPasteQuery( SvLBoxEntry* _pApplyTo, const TransferableDataHelper& _rPasteData )
    {
        DBG_ASSERT(ET_QUERY_CONTAINER == getEntryType(_pApplyTo), "SbaTableQueryBrowser::implPasteQuery: invalid target entry!");
        try
        {
            sal_Bool bQueryDescriptor = _rPasteData.HasFormat(SOT_FORMATSTR_ID_DBACCESS_QUERY);
            sal_Bool bCommandDescriptor = _rPasteData.HasFormat(SOT_FORMATSTR_ID_DBACCESS_COMMAND);
            if (bQueryDescriptor || bCommandDescriptor)
            {
                DataFlavor aFlavor;
                SotExchange::GetFormatDataFlavor(bQueryDescriptor ? SOT_FORMATSTR_ID_DBACCESS_QUERY : SOT_FORMATSTR_ID_DBACCESS_COMMAND, aFlavor);
                Sequence<PropertyValue> aDescriptor;
    #ifdef DBG_UTIL
                sal_Bool bCorrectFormat =
    #endif
                _rPasteData.GetAny(aFlavor) >>= aDescriptor;
                DBG_ASSERT(bCorrectFormat, "SbaTableQueryBrowser::implPasteQuery: invalid DnD or clipboard format!");

    #if defined(DBG_UTIL) && defined(PRIV_DEBUG)
                // temporary - remove this !!!
                if (!bCorrectFormat)
                {
                    createObjectDescriptor(aDescriptor,
                        ::rtl::OUString::createFromAscii("Bibliography"),
                        CommandType::QUERY,
                        ::rtl::OUString::createFromAscii("Query1")
                        );
                }
    #endif

                ::rtl::OUString sDataSourceName;
                sal_Int32       nCommandType = CommandType::QUERY;
                ::rtl::OUString sCommand;
                sal_Bool        bEscapeProcessing = sal_True;
                extractObjectDescription(aDescriptor, &sDataSourceName, &nCommandType, &sCommand, &bEscapeProcessing);

                // plausibility check
                sal_Bool bValidDescriptor = sal_False;
                if (CommandType::QUERY == nCommandType)
                    bValidDescriptor = sDataSourceName.getLength() && sCommand.getLength();
                else if (CommandType::COMMAND == nCommandType)
                    bValidDescriptor = (0 != sCommand.getLength());
                if (!bValidDescriptor)
                {
                    DBG_ERROR("SbaTableQueryBrowser::implPasteQuery: invalid descriptor!");
                    return;
                }

                // three properties we can set only if we have a query object as source
                ::rtl::OUString sUpdateTableName;
                ::rtl::OUString sUpdateSchemaName;
                ::rtl::OUString sUpdateCatalogName;

                // the target object name (as we'll suggest it to the user)
                String sTargetName;
                if (CommandType::QUERY == nCommandType)
                    sTargetName = sCommand;

                if (CommandType::QUERY == nCommandType)
                {
                    // need to extract the statement and the escape processing flag from the query object
                    sal_Bool bSuccess = sal_False;
                    try
                    {
                        // the data source to extract the query from
                        Reference< XQueryDefinitionsSupplier > xSourceDSQuerySupp;
                        m_xDatabaseContext->getByName(sDataSourceName) >>= xSourceDSQuerySupp;

                        // the query container
                        Reference< XNameAccess > xQueries;
                        if (xSourceDSQuerySupp.is())
                            xQueries = xSourceDSQuerySupp->getQueryDefinitions();

                        // the concrete query
                        Reference< XPropertySet > xQuery;
                        if (xQueries.is())
                            xQueries->getByName(sCommand) >>= xQuery;

                        if (xQuery.is())
                        {
                            // extract all the properties we're interested in
                            xQuery->getPropertyValue(PROPERTY_COMMAND) >>= sCommand;
                            bEscapeProcessing = ::cppu::any2bool(xQuery->getPropertyValue(PROPERTY_USE_ESCAPE_PROCESSING));

                            xQuery->getPropertyValue(PROPERTY_UPDATE_TABLENAME) >>= sUpdateTableName;
                            xQuery->getPropertyValue(PROPERTY_UPDATE_SCHEMANAME) >>= sUpdateSchemaName;
                            xQuery->getPropertyValue(PROPERTY_UPDATE_CATALOGNAME) >>= sUpdateCatalogName;
                            bSuccess = sal_True;
                        }
                    }
                    catch(SQLException&) { throw; } // caught and handled by the outer catch
                    catch(Exception&) { }

                    if (!bSuccess)
                    {
                        DBG_ERROR("SbaTableQueryBrowser::implPasteQuery: could not extract the source query object!");
                        // TODO: maybe this is worth an error message to be displayed to the user ....
                        return;
                    }
                }

                // get the queries container of the destination data source
                if (!ensureEntryObject(_pApplyTo))
                    // this is a heavy error ... the name container for the queries could not ne obtained
                    return;

                DBTreeListModel::DBTreeListUserData* pQueriesData = static_cast<DBTreeListModel::DBTreeListUserData*>(_pApplyTo->GetUserData());
                Reference< XNameContainer > xDestQueries(pQueriesData->xObject, UNO_QUERY);
                Reference< XSingleServiceFactory > xQueryFactory(xDestQueries, UNO_QUERY);
                if (!xQueryFactory.is())
                {
                    DBG_ERROR("SbaTableQueryBrowser::implPasteQuery: invalid destination query container!");
                    return;
                }

                // here we have everything needed to create a new query object ...
                // ... ehm, except a new name
                OSaveAsDlg aAskForName(getView(), CommandType::QUERY, xDestQueries.get(), Reference< XDatabaseMetaData>(), sTargetName, SAD_ADDITIONAL_DESCRIPTION | SAD_TITLE_PASTE_AS);
                if (RET_OK != aAskForName.Execute())
                    // cancelled by the user
                    return;

                sTargetName = aAskForName.getName();

                // create a new object
                Reference< XPropertySet > xNewQuery(xQueryFactory->createInstance(), UNO_QUERY);
                DBG_ASSERT(xNewQuery.is(), "SbaTableQueryBrowser::implPasteQuery: invalid object created by factory!");
                if (xNewQuery.is())
                {
                    // initialize
                    xNewQuery->setPropertyValue( PROPERTY_COMMAND, makeAny(sCommand) );
                    xNewQuery->setPropertyValue( PROPERTY_USE_ESCAPE_PROCESSING, ::cppu::bool2any(bEscapeProcessing) );
                    xNewQuery->setPropertyValue( PROPERTY_UPDATE_TABLENAME, makeAny(sUpdateTableName) );
                    xNewQuery->setPropertyValue( PROPERTY_UPDATE_SCHEMANAME, makeAny(sUpdateSchemaName) );
                    xNewQuery->setPropertyValue( PROPERTY_UPDATE_CATALOGNAME, makeAny(sUpdateCatalogName) );
                    // insert
                    xDestQueries->insertByName( sTargetName, makeAny(xNewQuery) );
                }
            }
        }
        catch(SQLContext& e) { showError(SQLExceptionInfo(e)); }
        catch(SQLWarning& e) { showError(SQLExceptionInfo(e)); }
        catch(SQLException& e) { showError(SQLExceptionInfo(e)); }
        catch(Exception& )
        {
            DBG_ERROR("SbaTableQueryBrowser::implPasteQuery: caught a strange exception!");
        }
    }

    // -----------------------------------------------------------------------------
    void SbaTableQueryBrowser::implPasteTable( SvLBoxEntry* _pApplyTo, const TransferableDataHelper& _rPasteData )
    {
        try
        {
            // paste into the tables
            sal_Bool bTableDescriptor = _rPasteData.HasFormat(SOT_FORMATSTR_ID_DBACCESS_TABLE);
            sal_Bool bQueryDescriptor = _rPasteData.HasFormat(SOT_FORMATSTR_ID_DBACCESS_QUERY);
            OSL_ENSURE(!bTableDescriptor || !bQueryDescriptor, "SbaTableQueryBrowser::implPasteTable: suspicious formats (both a table and a query descriptor)!");
            if  (bTableDescriptor || bQueryDescriptor)
            {
                DataFlavor aFlavor;
                SotExchange::GetFormatDataFlavor(bTableDescriptor ? SOT_FORMATSTR_ID_DBACCESS_TABLE : SOT_FORMATSTR_ID_DBACCESS_QUERY, aFlavor);
                Sequence<PropertyValue> aSeq;
    #ifdef DBG_UTIL
                sal_Bool bCorrectFormat =
    #endif
                _rPasteData.GetAny(aFlavor) >>= aSeq;
                DBG_ASSERT(bCorrectFormat, "SbaTableQueryBrowser::implPasteTable: invalid DnD or clipboard format!");

                if(aSeq.getLength())
                {
                    ::rtl::OUString aDSName = getEntryText( m_pTreeView->getListBox()->GetRootLevelParent( _pApplyTo ) );

                    const PropertyValue* pBegin = aSeq.getConstArray();
                    const PropertyValue* pEnd   = pBegin + aSeq.getLength();

                    // first get the dest connection
                    Reference<XConnection> xDestConnection;  // supports the service sdb::connection
                    if(!ensureConnection(_pApplyTo, xDestConnection))
                        return;

                    Reference<XConnection> xSrcConnection;
                    ::rtl::OUString sName,sSrcDataSourceName;
                    sal_Int32 nCommandType = CommandType::TABLE;
                    for(;pBegin != pEnd;++pBegin)
                    {
                        if(pBegin->Name == PROPERTY_DATASOURCENAME)
                            pBegin->Value >>= sSrcDataSourceName;
                        else if(pBegin->Name == PROPERTY_COMMANDTYPE)
                            pBegin->Value >>= nCommandType;
                        else if(pBegin->Name == PROPERTY_COMMAND)
                            pBegin->Value >>= sName;
                        else if(pBegin->Name == PROPERTY_ACTIVECONNECTION)
                            pBegin->Value >>= xSrcConnection;
                    }

                    // get the source connection

                    sal_Bool bDispose = sal_False;
                    if(sSrcDataSourceName == aDSName)
                        xSrcConnection = xDestConnection;
                    else if(!xSrcConnection.is())
                    {
                        Reference< XEventListener> xEvt(static_cast<::cppu::OWeakObject*>(this), UNO_QUERY);
                        showError(::dbaui::createConnection(sSrcDataSourceName,m_xDatabaseContext,getORB(),xEvt,xSrcConnection));
                        bDispose = sal_True;
                    }
                    Reference<XNameAccess> xNameAccess;
                    switch(nCommandType)
                    {
                        case CommandType::TABLE:
                            {
                                // only for tables
                                Reference<XTablesSupplier> xSup(xSrcConnection,UNO_QUERY);
                                if(xSup.is())
                                    xNameAccess = xSup->getTables();
                            }
                            break;
                        case CommandType::QUERY:
                            {
                                Reference<XQueriesSupplier> xSup(xSrcConnection,UNO_QUERY);
                                if(xSup.is())
                                    xNameAccess = xSup->getQueries();
                            }
                            break;
                    }

                    // check if this name really exists in the name access
                    if(xNameAccess.is() && xNameAccess->hasByName(sName))
                    {
                        Reference<XPropertySet> xSourceObject;
                        xNameAccess->getByName(sName) >>= xSourceObject;
                        OCopyTableWizard aWizard(getView(),
                                                 xSourceObject,
                                                 xDestConnection,
                                                 getNumberFormatter(),
                                                 getORB());
                        OCopyTable*         pPage1 = new OCopyTable(&aWizard,COPY, sal_False,OCopyTableWizard::WIZARD_DEF_DATA);
                        OWizNameMatching*   pPage2 = new OWizNameMatching(&aWizard);
                        OWizColumnSelect*   pPage3 = new OWizColumnSelect(&aWizard);
                        OWizNormalExtend*   pPage4 = new OWizNormalExtend(&aWizard);

                        aWizard.AddWizardPage(pPage1);
                        aWizard.AddWizardPage(pPage2);
                        aWizard.AddWizardPage(pPage3);
                        aWizard.AddWizardPage(pPage4);
                        aWizard.ActivatePage();

                        if (aWizard.Execute())
                        {
                            Reference<XPropertySet> xTable;
                            switch(aWizard.GetCreateStyle())
                            {
                                case OCopyTableWizard::WIZARD_DEF:
                                case OCopyTableWizard::WIZARD_DEF_DATA:
                                    {
                                        xTable = aWizard.createTable();
                                        if(!xTable.is())
                                            break;
                                        if(OCopyTableWizard::WIZARD_DEF == aWizard.GetCreateStyle())
                                            break;
                                    }
                                case OCopyTableWizard::WIZARD_APPEND_DATA:
                                    {
                                        Reference<XStatement> xStmt = xSrcConnection->createStatement();
                                        if(!xStmt.is())
                                            break;
                                        ::rtl::OUString sSql,sDestName;
                                        ::dbaui::composeTableName(xDestConnection->getMetaData(),xTable,sDestName,sal_False);
                                        // create the sql stmt
                                        if(nCommandType == CommandType::TABLE)
                                        {
                                            sSql = ::rtl::OUString::createFromAscii("SELECT * FROM ");
                                            ::rtl::OUString sComposedName;
                                            ::dbaui::composeTableName(xSrcConnection->getMetaData(),xSourceObject,sComposedName,sal_True);
                                            sSql += sComposedName;
                                        }
                                        else
                                            xSourceObject->getPropertyValue(PROPERTY_COMMAND) >>= sSql;

                                        Reference<XResultSet> xSrcRs = xStmt->executeQuery(sSql);
                                        Reference<XRow> xRow(xSrcRs,UNO_QUERY);
                                        if(!xSrcRs.is() || !xRow.is())
                                            break;

                                        Reference<XResultSet> xDestSet = Reference< XResultSet >(getORB()->createInstance(::rtl::OUString::createFromAscii("com.sun.star.sdb.RowSet")),UNO_QUERY);
                                        Reference<XPropertySet > xProp(xDestSet,UNO_QUERY);
                                        if(xProp.is())
                                        {
                                            xProp->setPropertyValue(PROPERTY_ACTIVECONNECTION,makeAny(xDestConnection));
                                            xProp->setPropertyValue(PROPERTY_COMMANDTYPE,makeAny(CommandType::TABLE));
                                            xProp->setPropertyValue(PROPERTY_COMMAND,makeAny(sDestName));
                                            xProp->setPropertyValue(PROPERTY_IGNORERESULT,::cppu::bool2any(sal_True));
                                            Reference<XRowSet> xRowSet(xProp,UNO_QUERY);
                                            xRowSet->execute();
                                        }
                                        Reference< XResultSetUpdate> xDestRsUpd(xDestSet,UNO_QUERY);
                                        Reference< XRowUpdate>       xDestRowUpd(xDestSet,UNO_QUERY);

                                        Reference< XResultSetMetaDataSupplier> xSrcMetaSup(xSrcRs,UNO_QUERY);
                                        Reference< XResultSetMetaData> xMeta = xSrcMetaSup->getMetaData();
                                        sal_Int32 nCount = xMeta->getColumnCount();

                                        sal_Bool bIsAutoIncrement           = aWizard.SetAutoincrement();
                                        ::std::vector<sal_Int32> vColumns   = aWizard.GetColumnPositions();
                                        OSL_ENSURE(sal_Int32(vColumns.size()) == nCount,"Column count isn't correct!");

                                        sal_Int32 nRowCount = 0;
                                        while(xSrcRs->next())
                                        {
                                            ++nRowCount;
                                            xDestRsUpd->moveToInsertRow();
                                            for(sal_Int32 i=1;i<=nCount;++i)
                                            {
                                                sal_Int32 nPos = vColumns[i-1];
                                                if(nPos == CONTAINER_ENTRY_NOTFOUND)
                                                    continue;
                                                if(i == 1 && bIsAutoIncrement)
                                                {
                                                    xDestRowUpd->updateInt(1,nRowCount);
                                                    continue;
                                                }

                                                switch(xMeta->getColumnType(i))
                                                {
                                                    case DataType::CHAR:
                                                    case DataType::VARCHAR:
                                                        xDestRowUpd->updateString(vColumns[i-1],xRow->getString(i));
                                                        break;
                                                    case DataType::DECIMAL:
                                                    case DataType::NUMERIC:
                                                    case DataType::BIGINT:
                                                        xDestRowUpd->updateDouble(vColumns[i-1],xRow->getDouble(i));
                                                        break;
                                                    case DataType::FLOAT:
                                                        xDestRowUpd->updateFloat(vColumns[i-1],xRow->getFloat(i));
                                                        break;
                                                    case DataType::DOUBLE:
                                                        xDestRowUpd->updateDouble(vColumns[i-1],xRow->getDouble(i));
                                                        break;
                                                    case DataType::LONGVARCHAR:
                                                        xDestRowUpd->updateString(vColumns[i-1],xRow->getString(i));
                                                        break;
                                                    case DataType::LONGVARBINARY:
                                                        xDestRowUpd->updateBytes(vColumns[i-1],xRow->getBytes(i));
                                                        break;
                                                    case DataType::DATE:
                                                        xDestRowUpd->updateDate(vColumns[i-1],xRow->getDate(i));
                                                        break;
                                                    case DataType::TIME:
                                                        xDestRowUpd->updateTime(vColumns[i-1],xRow->getTime(i));
                                                        break;
                                                    case DataType::TIMESTAMP:
                                                        xDestRowUpd->updateTimestamp(vColumns[i-1],xRow->getTimestamp(i));
                                                        break;
                                                    case DataType::BIT:
                                                        xDestRowUpd->updateBoolean(vColumns[i-1],xRow->getBoolean(i));
                                                        break;
                                                    case DataType::TINYINT:
                                                        xDestRowUpd->updateByte(vColumns[i-1],xRow->getByte(i));
                                                        break;
                                                    case DataType::SMALLINT:
                                                        xDestRowUpd->updateShort(vColumns[i-1],xRow->getShort(i));
                                                        break;
                                                    case DataType::INTEGER:
                                                        xDestRowUpd->updateInt(vColumns[i-1],xRow->getInt(i));
                                                        break;
                                                    case DataType::REAL:
                                                        xDestRowUpd->updateDouble(vColumns[i-1],xRow->getDouble(i));
                                                        break;
                                                    case DataType::BINARY:
                                                    case DataType::VARBINARY:
                                                        xDestRowUpd->updateBytes(vColumns[i-1],xRow->getBytes(i));
                                                        break;
                                                    default:
                                                        OSL_ENSURE(0,"Unknown type");
                                                }
                                                if(xRow->wasNull())
                                                    xDestRowUpd->updateNull(vColumns[i-1]);
                                            }
                                            xDestRsUpd->insertRow();
                                        }
                                        ::comphelper::disposeComponent(xDestRsUpd);
                                    }
                                    break;
                                default:
                                    break;
                            }
                        }
                    }
                    if(bDispose)
                        ::comphelper::disposeComponent(xSrcConnection);
                }
            }
            else if(_rPasteData.HasFormat(SOT_FORMATSTR_ID_HTML) || _rPasteData.HasFormat(SOT_FORMAT_RTF))
            {
                // first get the dest connection
                Reference<XConnection> xDestConnection;  // supports the service sdb::connection
                if(!ensureConnection(_pApplyTo, xDestConnection))
                    return;

                SotStorageStreamRef aStream;
                Reference<XEventListener> xEvt;
                ODatabaseImportExport* pImport = NULL;
                if(_rPasteData.HasFormat(SOT_FORMATSTR_ID_HTML))
                {
                    const_cast<TransferableDataHelper&>(_rPasteData).GetSotStorageStream(SOT_FORMATSTR_ID_HTML,aStream);
                        // TODO: why are the GetXXX methods not const???

                    pImport = new OHTMLImportExport(xDestConnection,getNumberFormatter(),getORB());
                }
                else
                {
                    const_cast<TransferableDataHelper&>(_rPasteData).GetSotStorageStream(SOT_FORMAT_RTF,aStream);
                        // TODO: why are the GetXXX methods not const???
                    pImport = new ORTFImportExport(xDestConnection,getNumberFormatter(),getORB());
                }
                xEvt = pImport;
                SvStream* pStream = (SvStream*)(SotStorageStream*)aStream;
                pImport->setStream(pStream);
                pImport->Read();
            }
            else
                DBG_ERROR("SbaTableQueryBrowser::implPasteTable: invalid call (no supported format found)!");
        }
        catch(SQLContext& e) { showError(SQLExceptionInfo(e)); }
        catch(SQLWarning& e) { showError(SQLExceptionInfo(e)); }
        catch(SQLException& e) { showError(SQLExceptionInfo(e)); }
        catch(Exception& )
        {
            OSL_ASSERT(0);
        }
    }

    // -----------------------------------------------------------------------------
    TransferableHelper* SbaTableQueryBrowser::implCopyObject( SvLBoxEntry* _pApplyTo, sal_Int32 _nCommandType, sal_Bool _bAllowConnection )
    {
        try
        {
            ::osl::MutexGuard aGuard(m_aEntryMutex);
            Reference<XConnection> xConnection;  // supports the service sdb::connection
            if (_bAllowConnection && !ensureConnection(_pApplyTo, xConnection))
                return NULL;

            ::rtl::OUString aName = getEntryText( _pApplyTo );
            ::rtl::OUString aDSName = getEntryText( m_pTreeView->getListBox()->GetRootLevelParent( _pApplyTo ) );

            Sequence<PropertyValue> aSeq(4);
            aSeq[0].Name    = PROPERTY_DATASOURCENAME;
            aSeq[0].Value <<= aDSName;
            aSeq[1].Name    = PROPERTY_ACTIVECONNECTION;;
            aSeq[1].Value <<= xConnection;
            aSeq[2].Name    = PROPERTY_COMMANDTYPE;
            aSeq[2].Value <<= _nCommandType;
            aSeq[3].Name    = PROPERTY_COMMAND;
            aSeq[3].Value <<= aName;

            // the rtf format
            ORTFImportExport* pRtf = new ORTFImportExport(aSeq,getORB(),getNumberFormatter());
            Reference< XInterface> xRTF = *pRtf;
            pRtf->initialize();
            // the html format
            OHTMLImportExport* pHtml = new OHTMLImportExport(aSeq,getORB(),getNumberFormatter());
            Reference< XInterface> xHTML = *pHtml;
            pHtml->initialize();
            // the sdbc format
            // the owner ship goes to ODataClipboard
            ODataClipboard* pData = new ODataClipboard(aSeq,pHtml,pRtf);
            return pData;
        }
        catch(SQLException& e)
        {
            showError(SQLExceptionInfo(e));
        }
        catch(Exception&)
        {
            DBG_ERROR("SbaTableQueryBrowser::implCopyObject: caught a generic exception!");
        }
        return NULL;
    }

    // -----------------------------------------------------------------------------
    sal_Int8 SbaTableQueryBrowser::queryDrop( const AcceptDropEvent& _rEvt, const DataFlavorExVector& _rFlavors )
    {
        // check if we're a table or query container
        SvLBoxEntry* pHitEntry = m_pTreeView->getListBox()->GetEntry( _rEvt.maPosPixel );
        if (!pHitEntry)
            // no drop of no entry was hit ....
            return DND_ACTION_NONE;

        // it must be a container
        EntryType eEntryType = getEntryType( pHitEntry );
        if (!isContainer(eEntryType))
            return DND_ACTION_NONE;

        // TODO: check if the data source is readonly

        // check for the concrete type
        for (   DataFlavorExVector::const_iterator aSearch = _rFlavors.begin();
                aSearch != _rFlavors.end();
                ++aSearch
            )
        {
            switch (aSearch->mnSotId)
            {
                case SOT_FORMAT_RTF:                    // RTF data descriptions
                case SOT_FORMATSTR_ID_HTML:             // HTML data descriptions
                case SOT_FORMATSTR_ID_DBACCESS_TABLE:   // table descriptor
                    if (ET_TABLE_CONTAINER == eEntryType)
                        return DND_ACTION_COPY;
                    break;

                case SOT_FORMATSTR_ID_DBACCESS_QUERY:   // query descriptor
                case SOT_FORMATSTR_ID_DBACCESS_COMMAND: // SQL command
                    if (ET_QUERY_CONTAINER == eEntryType)
                        return DND_ACTION_COPY;
                    break;
            }
        }
        return DND_ACTION_NONE;
    }

    // -----------------------------------------------------------------------------
    IMPL_LINK( SbaTableQueryBrowser, OnAsyncDrop, void*, NOTINTERESTEDIN )
    {
        m_nAsyncDrop = 0;

        if (m_aAsyncDrop.bTable)
            implPasteTable(m_aAsyncDrop.pDroppedAt, m_aAsyncDrop.aDroppedData);
        else
            implPasteQuery(m_aAsyncDrop.pDroppedAt, m_aAsyncDrop.aDroppedData);

        m_aAsyncDrop.aDroppedData   = TransferableDataHelper();
        m_aAsyncDrop.pDroppedAt     = NULL;

        return 0L;
    }

    // -----------------------------------------------------------------------------
    sal_Int8 SbaTableQueryBrowser::executeDrop( const ExecuteDropEvent& _rEvt )
    {
        SvLBoxEntry* pHitEntry = m_pTreeView->getListBox()->GetEntry( _rEvt.maPosPixel );
        EntryType eEntryType = getEntryType( pHitEntry );
        if (!isContainer(eEntryType))
        {
            DBG_ERROR("SbaTableQueryBrowser::executeDrop: what the hell did queryDrop do?");
                // queryDrop shoud not have allowed us to reach this situation ....
            return DND_ACTION_NONE;
        }

        // a TransferableDataHelper for accessing the dropped data
        TransferableDataHelper aDroppedData(_rEvt.maDropEvent.Transferable);


        // reset the data of the previous async drop (if any)
        if (m_nAsyncDrop)
            Application::RemoveUserEvent(m_nAsyncDrop);
        m_nAsyncDrop = 0;
        m_aAsyncDrop.aDroppedData   = TransferableDataHelper();
        m_aAsyncDrop.pDroppedAt     = NULL;
        m_aAsyncDrop.bTable         = sal_False;


        sal_Bool bDoDrop = sal_False;

        // loop through the available formats and see what we can do ...
        DataFlavorExVector::const_iterator aFormats = aDroppedData.GetDataFlavorExVector().begin();
        DataFlavorExVector::const_iterator aFormatsEnd = aDroppedData.GetDataFlavorExVector().end();
        for (;aFormats != aFormatsEnd; ++aFormats)
        {
            switch (aFormats->mnSotId)
            {
                case SOT_FORMAT_RTF:                    // RTF data descriptions
                case SOT_FORMATSTR_ID_HTML:             // HTML data descriptions
                case SOT_FORMATSTR_ID_DBACCESS_TABLE:   // table descriptor
                    bDoDrop = (ET_TABLE_CONTAINER == eEntryType);
                    break;

                case SOT_FORMATSTR_ID_DBACCESS_COMMAND: // SQL command
                case SOT_FORMATSTR_ID_DBACCESS_QUERY:   // query descriptor
                    bDoDrop =   (ET_QUERY_CONTAINER == eEntryType)
                            ||  (ET_TABLE_CONTAINER == eEntryType);
                    break;
            }
            if (bDoDrop)
                break;
        }

        if (bDoDrop)
        {
            m_aAsyncDrop.aDroppedData   = aDroppedData;
            m_aAsyncDrop.pDroppedAt     = pHitEntry;
            m_aAsyncDrop.bTable         = (ET_TABLE_CONTAINER == eEntryType);

            m_nAsyncDrop = Application::PostUserEvent(LINK(this, SbaTableQueryBrowser, OnAsyncDrop));
            return DND_ACTION_COPY;
        }

        return DND_ACTION_NONE;
    }

    // -----------------------------------------------------------------------------
    sal_Bool SbaTableQueryBrowser::requestDrag( sal_Int8 _nAction, const Point& _rPosPixel )
    {
        // get the affected list entry
        // ensure that the entry which the user clicked at is selected
        SvLBoxEntry* pHitEntry = m_pTreeView->getListBox()->GetEntry( _rPosPixel );
        if (!pHitEntry)
            // no drag of no entry was hit ....
            return sal_False;

        // it must be a query/table
        EntryType eEntryType = getEntryType( pHitEntry );
        if (!isObject(eEntryType))
            return DND_ACTION_NONE;

        TransferableHelper* pTransfer = implCopyObject( pHitEntry, (ET_TABLE == eEntryType) ? CommandType::TABLE : CommandType::QUERY);
        Reference< XTransferable> xEnsureDelete = pTransfer;

        if (pTransfer)
            pTransfer->StartDrag( m_pTreeView->getListBox(), DND_ACTION_COPY );

        return NULL != pTransfer;
    }

// .........................................................................
}   // namespace dbaui
// .........................................................................

/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *  Revision 1.1  2001/03/23 10:59:09  fs
 *  initial checkin - DnD related implementations for the data source browser controller
 *
 *
 *  Revision 1.0 23.03.01 09:03:17  fs
 ************************************************************************/
