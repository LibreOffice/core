/*************************************************************************
 *
 *  $RCSfile: dsbrowserDnD.cxx,v $
 *
 *  $Revision: 1.60 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-19 17:52:12 $
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
#ifndef _COM_SUN_STAR_SDBC_XDATASOURCE_HPP_
#include <com/sun/star/sdbc/XDataSource.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XSQLQUERYCOMPOSERFACTORY_HPP_
#include <com/sun/star/sdb/XSQLQueryComposerFactory.hpp>
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
#ifndef _COM_SUN_STAR_SDBCX_XRENAME_HPP_
#include <com/sun/star/sdbcx/XRename.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XRESULTSETMETADATASUPPLIER_HPP_
#include <com/sun/star/sdbc/XResultSetMetaDataSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XCOLUMNLOCATE_HPP_
#include <com/sun/star/sdbc/XColumnLocate.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XQUERYDEFINITIONSSUPPLIER_HPP_
#include <com/sun/star/sdb/XQueryDefinitionsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XPARAMETERS_HPP_
#include <com/sun/star/sdbc/XParameters.hpp>
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
#ifndef _DBU_BRW_HRC_
#include "dbu_brw.hrc"
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
#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include <connectivity/dbtools.hxx>
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
#ifndef _SVX_DATACCESSDESCRIPTOR_HXX_
#include <svx/dataaccessdescriptor.hxx>
#endif
#ifndef _COM_SUN_STAR_TASK_XINTERACTIONHANDLER_HPP_
#include <com/sun/star/task/XInteractionHandler.hpp>
#endif
#ifndef _DBAUI_LINKEDDOCUMENTS_HXX_
#include "linkeddocuments.hxx"
#endif

#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _UNOTOOLS_UCBHELPER_HXX
#include <unotools/ucbhelper.hxx>
#endif
#ifndef _UNOTOOLS_TEMPFILE_HXX
#include <unotools/tempfile.hxx>
#endif
#ifndef _SV_WAITOBJ_HXX
#include <vcl/waitobj.hxx>
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
    using namespace ::com::sun::star::task;
    using namespace ::com::sun::star::datatransfer;
    using namespace ::dbtools;
    using namespace ::svx;

    // -----------------------------------------------------------------------------
#define FILL_PARAM(type,method)                         \
    {                                                   \
        type nValue = xRow->g##method(i);               \
        if ( !xRow->wasNull() )                         \
            xParameter->s##method(nPos,nValue);         \
        else                                            \
            xParameter->setNull(nPos,aColumnTypes[i]);  \
    }
    // -----------------------------------------------------------------------------
    void insertRows(const Reference<XResultSet>& xSrcRs,
                   const ODatabaseExport::TPositions& _rvColumns,
                   const Reference<XPropertySet>& _xDestTable,
                   const Reference<XDatabaseMetaData>& _xMetaData,
                   sal_Bool bIsAutoIncrement,
                   const Sequence<Any>& _aSelection) throw(SQLException, RuntimeException)
    {
        Reference< XResultSetMetaDataSupplier> xSrcMetaSup(xSrcRs,UNO_QUERY);
        Reference<XRow> xRow(xSrcRs,UNO_QUERY);
        if(!xSrcRs.is() || !xRow.is())
            return;

        ::rtl::OUString aSql(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("INSERT INTO ")));
        ::rtl::OUString sComposedTableName;
        ::dbaui::composeTableName(_xMetaData,_xDestTable,sComposedTableName,sal_True);

        aSql += sComposedTableName;
        aSql += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" ( "));
        // set values and column names
        ::rtl::OUString aValues(RTL_CONSTASCII_USTRINGPARAM(" VALUES ( "));
        static ::rtl::OUString aPara(RTL_CONSTASCII_USTRINGPARAM("?,"));
        static ::rtl::OUString aComma(RTL_CONSTASCII_USTRINGPARAM(","));

        ::rtl::OUString aQuote = _xMetaData->getIdentifierQuoteString();

        Reference<XColumnsSupplier> xColsSup(_xDestTable,UNO_QUERY);
        OSL_ENSURE(xColsSup.is(),"SbaTableQueryBrowser::insertRows: No columnsSupplier!");
        if(!xColsSup.is())
            return;

        // we a vector which all types
        Reference< XResultSetMetaData> xMeta = xSrcMetaSup->getMetaData();
        sal_Int32 nCount = xMeta->getColumnCount();
        ::std::vector<sal_Int32> aColumnTypes;
        aColumnTypes.reserve(nCount+1);
        aColumnTypes.push_back(-1); // just to avoid a everytime i-1 call

        for(sal_Int32 k=1;k <= nCount;++k)
            aColumnTypes.push_back(xMeta->getColumnType(k));

        // create sql string and set column types
        Reference<XNameAccess> xNameAccess = xColsSup->getColumns();
        Sequence< ::rtl::OUString> aSeq = xNameAccess->getElementNames();
        const ::rtl::OUString* pBegin = aSeq.getConstArray();
        const ::rtl::OUString* pEnd   = pBegin + aSeq.getLength();
        for(sal_Int32 i=1;pBegin != pEnd;++pBegin,++i)
        {
            // create the sql string
            if ( _rvColumns.end() != ::std::find_if(_rvColumns.begin(),_rvColumns.end(),
                ::std::compose1(::std::bind2nd(::std::equal_to<sal_Int32>(),i),::std::select1st<ODatabaseExport::TPositions::value_type>())) )
            {
                aSql += ::dbtools::quoteName( aQuote,*pBegin);
                aSql += aComma;
                aValues += aPara;
            }
        }

        aSql = aSql.replaceAt(aSql.getLength()-1,1,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(")")));
        aValues = aValues.replaceAt(aValues.getLength()-1,1,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(")")));

        aSql += aValues;
        // now create,fill and execute the prepared statement
        Reference< XPreparedStatement > xPrep(_xMetaData->getConnection()->prepareStatement(aSql));
        Reference< XParameters > xParameter(xPrep,UNO_QUERY);


        sal_Int32 nRowCount = 0;
        const Any* pSelBegin    = _aSelection.getConstArray();
        const Any* pSelEnd      = pSelBegin + _aSelection.getLength();
        sal_Bool bUseSelection  = _aSelection.getLength() > 0;
        sal_Bool bNext = sal_True;
        do // loop as long as there are more rows or the selection ends
        {
            if ( bUseSelection )
            {
                if ( pSelBegin != pSelEnd )
                {
                    sal_Int32 nPos = 0;
                    *pSelBegin >>= nPos;
                    bNext = xSrcRs->absolute( nPos );
                    ++pSelBegin;
                }
                else
                    bNext = sal_False;
            }
            else
                bNext = xSrcRs->next();
            if ( bNext )
            {
                ++nRowCount;
                sal_Bool bInsertAutoIncrement = sal_True;
                ODatabaseExport::TPositions::const_iterator aPosIter = _rvColumns.begin();
                for(sal_Int32 i = 1;aPosIter != _rvColumns.end();++aPosIter)
                {
                    sal_Int32 nPos = aPosIter->second;
                    if ( nPos == CONTAINER_ENTRY_NOTFOUND )
                    {
                        ++i; // otherwise we don't get the correct value when only the 2nd source column was selected
                        continue;
                    }
                    if ( bIsAutoIncrement && bInsertAutoIncrement )
                    {
                        xParameter->setInt(1,nRowCount);
                        bInsertAutoIncrement = sal_False;
                        continue;
                    }
                    // we have to check here against 1 because the parameters are 1 based
                    OSL_ENSURE( i >= 1 && i < aColumnTypes.size(),"Index out of range for column types!");
                    switch(aColumnTypes[i])
                    {
                        case DataType::CHAR:
                        case DataType::VARCHAR:
                        case DataType::LONGVARCHAR:
                            FILL_PARAM( ::rtl::OUString, etString)
                            break;
                        case DataType::DECIMAL:
                        case DataType::NUMERIC:
                        case DataType::DOUBLE:
                        case DataType::REAL:
                            FILL_PARAM( double, etDouble)
                            break;
                        case DataType::BIGINT:
                            FILL_PARAM( sal_Int64, etLong)
                            break;
                        case DataType::FLOAT:
                            FILL_PARAM( float, etFloat)
                            break;
                        case DataType::LONGVARBINARY:
                        case DataType::BINARY:
                        case DataType::VARBINARY:
                            FILL_PARAM( Sequence< sal_Int8 >, etBytes)
                            break;
                        case DataType::DATE:
                            FILL_PARAM( ::com::sun::star::util::Date, etDate)
                            break;
                        case DataType::TIME:
                            FILL_PARAM( ::com::sun::star::util::Time, etTime)
                            break;
                        case DataType::TIMESTAMP:
                            FILL_PARAM( ::com::sun::star::util::DateTime, etTimestamp)
                            break;
                        case DataType::BIT:
                            FILL_PARAM( sal_Bool, etBoolean)
                            break;
                        case DataType::TINYINT:
                            FILL_PARAM( sal_Int8, etByte)
                            break;
                        case DataType::SMALLINT:
                            FILL_PARAM( sal_Int16, etShort)
                            break;
                        case DataType::INTEGER:
                            FILL_PARAM( sal_Int32, etInt)
                            break;
                        default:
                            OSL_ENSURE(0,"Unknown type");
                    }
                    ++i;
                }
                xPrep->executeUpdate();
            }
        }
        while( bNext );
    }
    // -----------------------------------------------------------------------------
    Reference<XResultSet> createResultSet(  SbaTableQueryBrowser* _pBrowser,sal_Bool bDispose,
                                            sal_Int32 _nCommandType,Reference<XConnection>& _xSrcConnection,
                                            const Reference<XPropertySet>& xSourceObject,
                                            Reference<XStatement> &xStmt,Reference<XPreparedStatement> &xPrepStmt)
    {
        Reference<XResultSet> xSrcRs;
        ::rtl::OUString sSql;
        if(_nCommandType == CommandType::TABLE)
        {
            sSql = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SELECT "));
            // we need to create the sql stmt with column names
            // otherwise it is possible that names don't match
            ::rtl::OUString sQuote = _xSrcConnection->getMetaData()->getIdentifierQuoteString();
            static ::rtl::OUString sComma = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(","));

            Reference<XColumnsSupplier> xSrcColsSup(xSourceObject,UNO_QUERY);
            OSL_ENSURE(xSrcColsSup.is(),"No source columns!");
            Reference<XNameAccess> xNameAccess = xSrcColsSup->getColumns();
            Sequence< ::rtl::OUString> aSeq = xNameAccess->getElementNames();
            const ::rtl::OUString* pBegin = aSeq.getConstArray();
            const ::rtl::OUString* pEnd   = pBegin + aSeq.getLength();
            for(;pBegin != pEnd;++pBegin)
            {
                sSql += ::dbtools::quoteName( sQuote,*pBegin);
                sSql += sComma;
            }
            sSql = sSql.replaceAt(sSql.getLength()-1,1,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" ")));
            sSql += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("FROM "));
            ::rtl::OUString sComposedName;
            ::dbaui::composeTableName(_xSrcConnection->getMetaData(),xSourceObject,sComposedName,sal_True);
            sSql += sComposedName;
            xStmt = _xSrcConnection->createStatement();
            if( xStmt.is() )
                xSrcRs = xStmt->executeQuery(sSql);
        }
        else
        {
            xSourceObject->getPropertyValue(PROPERTY_COMMAND) >>= sSql;
            xPrepStmt = _xSrcConnection->prepareStatement(sSql);
            if( xPrepStmt.is() )
            {
                // look if we have to fill in some parameters
                // create and fill a composer
                Reference< XSQLQueryComposerFactory >  xFactory(_xSrcConnection, UNO_QUERY);
                Reference< XSQLQueryComposer> xComposer;
                if (xFactory.is())
                {
                    try
                    {
                        xComposer = xFactory->createQueryComposer();
                        if(xComposer.is())
                        {
                            xComposer->setQuery(sSql);
                            Reference< XInteractionHandler > xHandler(_pBrowser->getORB()->createInstance(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.sdb.InteractionHandler"))), UNO_QUERY);
                            ::dbtools::askForParameters(xComposer,Reference<XParameters>(xPrepStmt,UNO_QUERY),_xSrcConnection,xHandler);
                            xSrcRs = xPrepStmt->executeQuery();
                        }
                    }
                    catch(SQLContext&)
                    {
                        if(bDispose)
                            ::comphelper::disposeComponent(_xSrcConnection);
                        throw;
                    }
                    catch(SQLWarning&)
                    {
                        if(bDispose)
                            ::comphelper::disposeComponent(_xSrcConnection);
                        throw;
                    }
                    catch(SQLException&)
                    {
                        if(bDispose)
                            ::comphelper::disposeComponent(_xSrcConnection);
                        throw;
                    }
                    catch (Exception&)
                    {
                        xComposer = NULL;
                    }
                }
            }
        }
        return xSrcRs;
    }

    // -----------------------------------------------------------------------------
    void SbaTableQueryBrowser::implPasteQuery( SvLBoxEntry* _pApplyTo, const ::svx::ODataAccessDescriptor& _rPasteData )
    {
        DBG_ASSERT(etQueryContainer == getEntryType(_pApplyTo) || etQuery == getEntryType(_pApplyTo), "SbaTableQueryBrowser::implPasteQuery: invalid target entry!");
        try
        {
            sal_Int32 nCommandType = CommandType::TABLE;
            if ( _rPasteData.has(daCommandType) )
                _rPasteData[daCommandType]      >>= nCommandType;

            if ( CommandType::QUERY == nCommandType || CommandType::COMMAND == nCommandType )
            {
                // read all nescessary data
                ::rtl::OUString sDataSourceName;
                ::rtl::OUString sCommand;
                sal_Bool        bEscapeProcessing = sal_True;
                _rPasteData[daDataSource]       >>= sDataSourceName;
                _rPasteData[daCommand]          >>= sCommand;
                if ( _rPasteData.has(daEscapeProcessing) )
                    _rPasteData[daEscapeProcessing] >>= bEscapeProcessing;

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
                if ( !ensureEntryObject( isContainer(_pApplyTo) ? _pApplyTo : m_pTreeView->getListBox()->GetParent( _pApplyTo )) )
                    // this is a heavy error ... the name container for the queries could not ne obtained
                    return;

                // check if the entry is a container else get the parent
                DBTreeListModel::DBTreeListUserData* pQueriesData = NULL;
                if ( !isContainer(_pApplyTo) )
                    pQueriesData = static_cast<DBTreeListModel::DBTreeListUserData*>(m_pTreeView->getListBox()->GetParent(_pApplyTo)->GetUserData());
                else
                    pQueriesData = static_cast<DBTreeListModel::DBTreeListUserData*>(_pApplyTo->GetUserData());

                OSL_ENSURE(pQueriesData,"No query data!");
                Reference< XNameContainer > xDestQueries(pQueriesData->xObject, UNO_QUERY);
                Reference< XSingleServiceFactory > xQueryFactory(xDestQueries, UNO_QUERY);
                if (!xQueryFactory.is())
                {
                    DBG_ERROR("SbaTableQueryBrowser::implPasteQuery: invalid destination query container!");
                    return;
                }

                // here we have everything needed to create a new query object ...
                // ... ehm, except a new name
                OSaveAsDlg aAskForName( getView(),
                                        CommandType::QUERY,
                                        xDestQueries.get(),
                                        Reference< XDatabaseMetaData>(),
                                        Reference< XConnection>(),
                                        sTargetName,
                                        SAD_ADDITIONAL_DESCRIPTION | SAD_TITLE_PASTE_AS);
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
            else
                OSL_TRACE("There should be a sequence in it!");
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
    void SbaTableQueryBrowser::implPasteTable( SvLBoxEntry* _pApplyTo, const TransferableDataHelper& _rTransData )
    {
        if ( ODataAccessObjectTransferable::canExtractObjectDescriptor(_rTransData.GetDataFlavorExVector()) )
            implPasteTable( _pApplyTo, ODataAccessObjectTransferable::extractObjectDescriptor(_rTransData) );
        else
        {
            try
            {
                sal_Bool bHtml = _rTransData.HasFormat(SOT_FORMATSTR_ID_HTML) || _rTransData.HasFormat(SOT_FORMATSTR_ID_HTML_SIMPLE);
                if ( bHtml || _rTransData.HasFormat(SOT_FORMAT_RTF))
                {
                    DropDescriptor aTrans;
                    if ( bHtml )
                        const_cast<TransferableDataHelper&>(_rTransData).GetSotStorageStream(_rTransData.HasFormat(SOT_FORMATSTR_ID_HTML) ? SOT_FORMATSTR_ID_HTML : SOT_FORMATSTR_ID_HTML_SIMPLE,aTrans.aHtmlRtfStorage);
                    else
                        const_cast<TransferableDataHelper&>(_rTransData).GetSotStorageStream(SOT_FORMAT_RTF,aTrans.aHtmlRtfStorage);

                    aTrans.pDroppedAt       = _pApplyTo;
                    aTrans.bTable           = etTableContainer;
                    aTrans.bHtml            = bHtml;
                    if ( !copyHtmlRtfTable(aTrans,sal_False) )
                        showError(SQLException(String(ModuleRes(STR_NO_TABLE_FORMAT_INSIDE)),*this,::rtl::OUString::createFromAscii("S1000") ,0,Any()));
                }
            }
            catch(SQLContext& e) { showError(SQLExceptionInfo(e)); }
            catch(SQLWarning& e) { showError(SQLExceptionInfo(e)); }
            catch(SQLException& e) { showError(SQLExceptionInfo(e)); }
            catch(Exception& )
            {
                OSL_ENSURE(sal_False, "SbaTableQueryBrowser::implPasteTable: caught a generic exception!");
            }
        }
    }
    // -----------------------------------------------------------------------------
    void SbaTableQueryBrowser::implPasteTable( SvLBoxEntry* _pApplyTo, const ::svx::ODataAccessDescriptor& _rPasteData )
    {
        try
        {
            // paste into the tables
            sal_Int32 nCommandType = CommandType::COMMAND;
            if ( _rPasteData.has(daCommandType) )
                _rPasteData[daCommandType] >>= nCommandType;

            if ( CommandType::QUERY == nCommandType || CommandType::TABLE == nCommandType )
            {
                ::rtl::OUString aDSName = GetEntryText( m_pTreeView->getListBox()->GetRootLevelParent( _pApplyTo ) );

                // first get the dest connection
                Reference<XConnection> xDestConnection;  // supports the service sdb::connection
                if ( !ensureConnection(_pApplyTo, xDestConnection) )
                    return;

                Reference<XConnection> xSrcConnection;
                Reference<XResultSet>   xSrcRs;         // the source resultset may be empty
                Sequence< Any > aSelection;
                ::rtl::OUString sCommand,sSrcDataSourceName;
                _rPasteData[daDataSource]       >>= sSrcDataSourceName;
                _rPasteData[daCommand]          >>= sCommand;
                if ( _rPasteData.has(daConnection) )
                    _rPasteData[daConnection]   >>= xSrcConnection;
                if ( _rPasteData.has(daSelection) )
                    _rPasteData[daSelection]    >>= aSelection;
                if ( _rPasteData.has(daCursor) )
                    _rPasteData[daCursor]       >>= xSrcRs;

                // get the source connection
                sal_Bool bDispose = sal_False;
                if ( sSrcDataSourceName == aDSName )
                    xSrcConnection = xDestConnection;
                else if ( !xSrcConnection.is() )
                {
                    Reference< XEventListener> xEvt(static_cast< ::cppu::OWeakObject*>(this), UNO_QUERY);
                    showError(::dbaui::createConnection(sSrcDataSourceName,m_xDatabaseContext,getORB(),xEvt,xSrcConnection));
                    bDispose = sal_True;
                }
                Reference<XNameAccess> xNameAccess;
                sal_Bool bTable = sal_True;
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
                            if ( xSup.is() )
                                xNameAccess = xSup->getQueries();
                            bTable = sal_False;
                        }
                        break;
                }

                // check if this name really exists in the name access
                if ( xNameAccess.is() && xNameAccess->hasByName( sCommand ) )
                {
                    Reference<XPropertySet> xSourceObject;

                    xNameAccess->getByName( sCommand ) >>= xSourceObject;
                    sal_Bool bIsView = sal_False;
                    if ( bTable ) // we only have to check if this table has as type VIEW
                    {
                        static ::rtl::OUString sVIEW = ::rtl::OUString::createFromAscii("VIEW");
                        bIsView = ::comphelper::getString(xSourceObject->getPropertyValue(PROPERTY_TYPE)) == sVIEW;
                    }

                    OCopyTableWizard aWizard(getView(),
                                             xSourceObject,
                                             xSrcConnection,
                                             xDestConnection,
                                             getNumberFormatter(),
                                             getORB());
                    aWizard.fillTypeInfo();
                    aWizard.loadData();
                    OCopyTable*         pPage1 = new OCopyTable(&aWizard,COPY, bIsView,OCopyTableWizard::WIZARD_DEF_DATA);
                    OWizNameMatching*   pPage2 = new OWizNameMatching(&aWizard);
                    OWizColumnSelect*   pPage3 = new OWizColumnSelect(&aWizard);
                    OWizNormalExtend*   pPage4 = new OWizNormalExtend(&aWizard);

                    aWizard.AddWizardPage(pPage1);
                    aWizard.AddWizardPage(pPage2);
                    aWizard.AddWizardPage(pPage3);
                    aWizard.AddWizardPage(pPage4);
                    aWizard.ActivatePage();

                    if (aWizard.Execute() == RET_OK)
                    {
                        WaitObject aWO(getBrowserView());
                        Reference<XPropertySet> xTable;
                        switch(aWizard.getCreateStyle())
                        {
                            case OCopyTableWizard::WIZARD_DEF:
                            case OCopyTableWizard::WIZARD_DEF_DATA:
                                {
                                    xTable = aWizard.createTable();
                                    if(!xTable.is())
                                        break;
                                    if(OCopyTableWizard::WIZARD_DEF == aWizard.getCreateStyle())
                                        break;
                                }
                            case OCopyTableWizard::WIZARD_APPEND_DATA:
                                {

                                    if(!xTable.is())
                                        xTable = aWizard.createTable();
                                    if(!xTable.is())
                                        break;

                                    Reference<XStatement> xStmt; // needed to hold a reference to the statement
                                    Reference<XPreparedStatement> xPrepStmt;// needed to hold a reference to the statement
                                    ::rtl::OUString sDestName;
                                    ::dbaui::composeTableName(xDestConnection->getMetaData(),xTable,sDestName,sal_False);

                                    ODatabaseExport::TPositions aColumnMapping = aWizard.GetColumnPositions();
                                    // create the sql stmt
                                    if ( !xSrcRs.is() ) // if not already exists
                                        xSrcRs = createResultSet(this,
                                                                bDispose,
                                                                nCommandType,
                                                                xSrcConnection,
                                                                xSourceObject,
                                                                xStmt,
                                                                xPrepStmt);
                                    else
                                    {
                                        // here I use the ResultSet directly so I have to adjust
                                        // the column mapping given by the wizard, because the resultset could use
                                        // another order of the columns
                                        Reference< XColumnLocate> xLocate(xSrcRs,UNO_QUERY);
                                        Reference<XColumnsSupplier> xSrcColsSup(xSourceObject,UNO_QUERY);
                                        OSL_ENSURE(xSrcColsSup.is(),"No source columns!");
                                        Reference<XNameAccess> xNameAccess = xSrcColsSup->getColumns();
                                        Sequence< ::rtl::OUString> aSeq = xNameAccess->getElementNames();
                                        const ::rtl::OUString* pBegin = aSeq.getConstArray();
                                        const ::rtl::OUString* pEnd   = pBegin + aSeq.getLength();

                                        ODatabaseExport::TPositions aNewColMapping;
                                        aNewColMapping.resize( aColumnMapping.size() ,ODatabaseExport::TPositions::value_type(CONTAINER_ENTRY_NOTFOUND,CONTAINER_ENTRY_NOTFOUND) );

                                        for(sal_Int32 k = 0;pBegin != pEnd;++pBegin,++k)
                                        {
                                            sal_Int32 nColPos = xLocate->findColumn(*pBegin) -1;
                                            if ( nColPos >= 0 )
                                                //  aNewColMapping[k] = aColumnMapping[nColPos];
                                                aNewColMapping[nColPos] = aColumnMapping[k];
                                        }
                                        aColumnMapping = aNewColMapping;
                                        // position the resultset before the first row
                                        if ( !xSrcRs->isBeforeFirst() )
                                            xSrcRs->beforeFirst();
                                    }
                                    // now insert the rows into the new table
                                    // we couldn't use the rowset here because it could happen that we haven't a primary key
                                    insertRows( xSrcRs,
                                                aColumnMapping,
                                                xTable,
                                                xDestConnection->getMetaData(),
                                                aWizard.isAutoincrementEnabled(),
                                                aSelection);
                                }
                                break;
                            case OCopyTableWizard::WIZARD_DEF_VIEW:
                                xTable = aWizard.createView();
                                break;
                            default:
                                break;
                        }
                    }
                }
                else
                {
                    switch(nCommandType)
                    {
                        case CommandType::TABLE:
                            break;
                        case CommandType::QUERY:
                            break;
                    }
                    showError(SQLException(String(ModuleRes(STR_NO_TABLE_FORMAT_INSIDE)),*this,::rtl::OUString::createFromAscii("S1000") ,0,Any()));
                }
                if(bDispose)
                    ::comphelper::disposeComponent(xSrcConnection);
            }
            else
                DBG_ERROR("SbaTableQueryBrowser::implPasteTable: invalid call (no supported format found)!");
        }
        catch(SQLContext& e) { showError(SQLExceptionInfo(e)); }
        catch(SQLWarning& e) { showError(SQLExceptionInfo(e)); }
        catch(SQLException& e) { showError(SQLExceptionInfo(e)); }
        catch(Exception& )
        {
            OSL_ENSURE(sal_False, "SbaTableQueryBrowser::implPasteTable: caught a generic exception!");
        }
    }
    // -----------------------------------------------------------------------------
    sal_Bool SbaTableQueryBrowser::copyHtmlRtfTable(DropDescriptor& _rDesc, sal_Bool _bCheck)
    {
        // first get the dest connection
        Reference<XConnection> xDestConnection;  // supports the service sdb::connection
        if(!ensureConnection(_rDesc.pDroppedAt, xDestConnection))
            return sal_False;

        Reference<XEventListener> xEvt;
        ODatabaseImportExport* pImport = NULL;
        if ( _rDesc.bHtml )
            pImport = new OHTMLImportExport(xDestConnection,getNumberFormatter(),getORB());
        else
            pImport = new ORTFImportExport(xDestConnection,getNumberFormatter(),getORB());

        xEvt = pImport;
        SvStream* pStream = (SvStream*)(SotStorageStream*)_rDesc.aHtmlRtfStorage;
        if ( _bCheck )
            pImport->enableCheckOnly();

        pImport->setStream(pStream);
        return pImport->Read();
    }
    // -----------------------------------------------------------------------------
    TransferableHelper* SbaTableQueryBrowser::implCopyObject( SvLBoxEntry* _pApplyTo, sal_Int32 _nCommandType, sal_Bool _bAllowConnection )
    {
        try
        {
            ::osl::MutexGuard aGuard(m_aEntryMutex);

            ::rtl::OUString aName = GetEntryText( _pApplyTo );
            ::rtl::OUString aDSName = GetEntryText( m_pTreeView->getListBox()->GetRootLevelParent( _pApplyTo ) );

            ODataClipboard* pData = NULL;
            Reference<XConnection> xConnection;  // supports the service sdb::connection
            if ( CommandType::QUERY != _nCommandType )
            {
                if (_bAllowConnection && !ensureConnection(_pApplyTo, xConnection))
                    return NULL;
                pData = new ODataClipboard(aDSName, _nCommandType, aName, xConnection, getNumberFormatter(), getORB());
            }
            else
                pData = new ODataClipboard(aDSName, _nCommandType, aName, getNumberFormatter(), getORB());

            // the owner ship goes to ODataClipboards
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
    /// unary_function Functor object for class DataFlavorExVector::value_type returntype is bool
    struct SupportedSotFunctor : ::std::unary_function<DataFlavorExVector::value_type,bool>
    {
        SbaTableQueryBrowser::EntryType eEntryType;
        sal_Bool    bQueryDrop;
        SupportedSotFunctor(const SbaTableQueryBrowser::EntryType &_eEntryType,sal_Bool _bQueryDrop)
            : eEntryType(_eEntryType)
            , bQueryDrop(_bQueryDrop)
        {
        }

        inline bool operator()(const DataFlavorExVector::value_type& _aType)
        {
            switch (_aType.mnSotId)
            {
                case SOT_FORMAT_RTF:                    // RTF data descriptions
                case SOT_FORMATSTR_ID_HTML:             // HTML data descriptions
                case SOT_FORMATSTR_ID_HTML_SIMPLE:      // HTML data descriptions
                case SOT_FORMATSTR_ID_DBACCESS_TABLE:   // table descriptor
                    return (SbaTableQueryBrowser::etTableContainer == eEntryType);
                    break;
                case SOT_FORMATSTR_ID_DBACCESS_QUERY:   // query descriptor
                case SOT_FORMATSTR_ID_DBACCESS_COMMAND: // SQL command
                    return ((SbaTableQueryBrowser::etQueryContainer == eEntryType) || ( !bQueryDrop && SbaTableQueryBrowser::etTableContainer == eEntryType));
                    break;
            }
            return false;
        }
    };


    // -----------------------------------------------------------------------------
    sal_Int8 SbaTableQueryBrowser::queryDrop( const AcceptDropEvent& _rEvt, const DataFlavorExVector& _rFlavors )
    {
        // check if we're a table or query container
        SvLBoxEntry* pHitEntry = m_pTreeView->getListBox()->GetEntry( _rEvt.maPosPixel );

        if ( pHitEntry ) // no drop if no entry was hit ....
        {
            // it must be a container
            EntryType eEntryType = getEntryType( pHitEntry );
            if ( isContainer(eEntryType) )
            {
                // TODO: check if the data source is readonly
                // check for the concrete type
                if(::std::find_if(_rFlavors.begin(),_rFlavors.end(),SupportedSotFunctor(eEntryType,sal_True)) != _rFlavors.end())
                    return DND_ACTION_COPY;
            }
        }

        return DND_ACTION_NONE;
    }

    // -----------------------------------------------------------------------------
    IMPL_LINK( SbaTableQueryBrowser, OnAsyncDrop, void*, NOTINTERESTEDIN )
    {
        m_nAsyncDrop = 0;

        if (m_aAsyncDrop.bTable)
        {
            if ( m_aAsyncDrop.aHtmlRtfStorage.Is() )
            {
                copyHtmlRtfTable(m_aAsyncDrop,sal_False);
                m_aAsyncDrop.aHtmlRtfStorage = NULL;
                // we now have to delete the temp file created in executeDrop
                INetURLObject aURL;
                aURL.SetURL(m_aAsyncDrop.aUrl);
                ::utl::UCBContentHelper::Kill(aURL.GetMainURL(INetURLObject::NO_DECODE));
            }
            else if ( !m_aAsyncDrop.bError )
                implPasteTable(m_aAsyncDrop.pDroppedAt, m_aAsyncDrop.aDroppedData);
            else
                showError(SQLException(String(ModuleRes(STR_NO_TABLE_FORMAT_INSIDE)),*this,::rtl::OUString::createFromAscii("S1000") ,0,Any()));
        }
        else
            implPasteQuery(m_aAsyncDrop.pDroppedAt, m_aAsyncDrop.aDroppedData);

        m_aAsyncDrop.aDroppedData.clear();
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
        m_aAsyncDrop.aDroppedData.clear();
        m_aAsyncDrop.pDroppedAt     = NULL;
        m_aAsyncDrop.bTable         = sal_False;
        m_aAsyncDrop.bError         = sal_False;
        m_aAsyncDrop.bHtml          = sal_False;


        // loop through the available formats and see what we can do ...
        // first we have to check if it is our own format, if not we have to copy the stream :-(
        if ( ODataAccessObjectTransferable::canExtractObjectDescriptor(aDroppedData.GetDataFlavorExVector()) )
        {
            m_aAsyncDrop.aDroppedData   = ODataAccessObjectTransferable::extractObjectDescriptor(aDroppedData);
            m_aAsyncDrop.pDroppedAt     = pHitEntry;
            m_aAsyncDrop.bTable         = (etTableContainer == eEntryType);

            // asyncron because we some dialogs and we aren't allowed to show them while in D&D
            m_nAsyncDrop = Application::PostUserEvent(LINK(this, SbaTableQueryBrowser, OnAsyncDrop));
            return DND_ACTION_COPY;
        }
        else
        {
            sal_Bool bHtml = aDroppedData.HasFormat(SOT_FORMATSTR_ID_HTML) || aDroppedData.HasFormat(SOT_FORMATSTR_ID_HTML_SIMPLE);
            if ( bHtml || aDroppedData.HasFormat(SOT_FORMAT_RTF))
            {
                if ( bHtml )
                    const_cast<TransferableDataHelper&>(aDroppedData).GetSotStorageStream(aDroppedData.HasFormat(SOT_FORMATSTR_ID_HTML) ? SOT_FORMATSTR_ID_HTML : SOT_FORMATSTR_ID_HTML_SIMPLE,m_aAsyncDrop.aHtmlRtfStorage);
                else
                    const_cast<TransferableDataHelper&>(aDroppedData).GetSotStorageStream(SOT_FORMAT_RTF,m_aAsyncDrop.aHtmlRtfStorage);

                m_aAsyncDrop.pDroppedAt     = pHitEntry;
                m_aAsyncDrop.bTable         = (etTableContainer == eEntryType);
                m_aAsyncDrop.bHtml          = bHtml;
                m_aAsyncDrop.bError         = !copyHtmlRtfTable(m_aAsyncDrop,sal_True);

                if ( !m_aAsyncDrop.bError && m_aAsyncDrop.aHtmlRtfStorage.Is() )
                {
                    // now we need to copy the stream
                    ::utl::TempFile aTmp;
                    aTmp.EnableKillingFile(sal_False);
                    m_aAsyncDrop.aUrl = aTmp.GetURL();
                    SotStorageStreamRef aNew = new SotStorageStream( aTmp.GetFileName() );
                    m_aAsyncDrop.aHtmlRtfStorage->Seek(STREAM_SEEK_TO_BEGIN);
                    m_aAsyncDrop.aHtmlRtfStorage->CopyTo( aNew );
                    aNew->Commit();
                    m_aAsyncDrop.aHtmlRtfStorage = aNew;
                }
                else
                    m_aAsyncDrop.aHtmlRtfStorage = NULL;

                // asyncron because we some dialogs and we aren't allowed to show them while in D&D
                m_nAsyncDrop = Application::PostUserEvent(LINK(this, SbaTableQueryBrowser, OnAsyncDrop));
                return DND_ACTION_COPY;
            }
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

        if (etBookmark == eEntryType)
            return DND_ACTION_NONE;

        TransferableHelper* pTransfer = implCopyObject( pHitEntry, (etTable == eEntryType || etView == eEntryType) ? CommandType::TABLE : CommandType::QUERY);
        Reference< XTransferable> xEnsureDelete = pTransfer;

        if (pTransfer)
            pTransfer->StartDrag( m_pTreeView->getListBox(), DND_ACTION_COPY );

        return NULL != pTransfer;
    }
    // -----------------------------------------------------------------------------
    sal_Bool SbaTableQueryBrowser::isTableFormat()  const
    {
        sal_Bool bTableFormat = sal_False;
        bTableFormat    =   getViewClipboard().HasFormat(SOT_FORMATSTR_ID_DBACCESS_TABLE)
                    ||  getViewClipboard().HasFormat(SOT_FORMATSTR_ID_DBACCESS_QUERY)
                    ||  getViewClipboard().HasFormat(SOT_FORMAT_RTF)
                    ||  getViewClipboard().HasFormat(SOT_FORMATSTR_ID_HTML)
                    ||  getViewClipboard().HasFormat(SOT_FORMATSTR_ID_HTML_SIMPLE);

        return bTableFormat;
    }
    // -----------------------------------------------------------------------------
    IMPL_LINK(SbaTableQueryBrowser, OnCutEntry, SvLBoxEntry*, _pEntry)
    {
        return 0;
    }
    // -----------------------------------------------------------------------------
    IMPL_LINK(SbaTableQueryBrowser, OnCopyEntry, SvLBoxEntry*, _pEntry)
    {
        if( isEntryCopyAllowed(_pEntry) )
            copyEntry(_pEntry);
        return 0;
    }
    // -----------------------------------------------------------------------------
    IMPL_LINK(SbaTableQueryBrowser, OnPasteEntry, SvLBoxEntry*, _pEntry)
    {
        if ( isEntryPasteAllowed(_pEntry) )
            pasteEntry(_pEntry);
        return 0;
    }
    // -----------------------------------------------------------------------------
    IMPL_LINK(SbaTableQueryBrowser, OnEditingEntry, SvLBoxEntry*, _pEntry)
    {
        EntryType eType = getEntryType(_pEntry);
        long nRet = 0;
        switch(eType)
        {
            case etQuery:
            case etView:
            case etTable:
                try
                {
                    if(eType == etQuery || isConnectionWriteAble(_pEntry))
                    {
                        ensureObjectExists(_pEntry);
                        DBTreeListModel::DBTreeListUserData* pData = static_cast<DBTreeListModel::DBTreeListUserData*>(_pEntry->GetUserData());
                        if(pData && pData->xObject.is())
                        {
                            Reference<XRename> xRename(pData->xObject,UNO_QUERY);
                            if(xRename.is())
                            {
                                ::rtl::OUString sName;
                                Reference<XPropertySet> xProp(pData->xObject,UNO_QUERY);
                                xProp->getPropertyValue(PROPERTY_NAME) >>= sName;
                                m_pTreeView->getListBox()->SetEntryText(_pEntry,sName);
                                nRet =  1;
                            }
                        }
                    }
                }
                catch(const Exception&)
                {
                    OSL_ENSURE(0,"Exception catched!");
                }
                break;
        }
        return nRet;
    }
    // -----------------------------------------------------------------------------
    IMPL_LINK(SbaTableQueryBrowser, OnEditedEntry, DBTreeEditedEntry*, _aEntry)
    {
        EntryType eType = getEntryType(_aEntry->pEntry);
        ::rtl::OUString sOldName;
        long nRet = 0;
        try
        {
            switch(eType)
            {
                case etView:
                case etTable:
                case etQuery:

                    if ( etQuery == eType || isConnectionWriteAble(_aEntry->pEntry) )
                    {
                        DBTreeListModel::DBTreeListUserData* pData = static_cast<DBTreeListModel::DBTreeListUserData*>(_aEntry->pEntry->GetUserData());
                        OSL_ENSURE(pData && pData->xObject.is(),"Error in editing!");
                        if(pData && pData->xObject.is())
                        {
                            ::rtl::OUString sName,sSchema,sCatalog;
                            ::rtl::OUString sNewName = _aEntry->aNewText;
                            Reference<XPropertySet> xProp(pData->xObject,UNO_QUERY);
                            xProp->getPropertyValue(PROPERTY_NAME) >>= sName;
                            Reference<XConnection> xConnection = getConnectionFromEntry(_aEntry->pEntry);
                            Reference<XDatabaseMetaData> xMeta = xConnection.is() ? xConnection->getMetaData() : Reference<XDatabaseMetaData>();
                            if(etQuery == eType)
                                sOldName = sName;
                            else
                                ::dbaui::composeTableName(xMeta,xProp,sOldName,sal_False);

                            if ( !_aEntry->aNewText.Len() )
                            {
                                String sError = ErrorBox(getView(), ModuleRes(ERROR_INVALID_TABLE_NAME)).GetMessText();
                                dbtools::throwGenericSQLException(sError, NULL);
                            }

                            // check if the new name is allowed
                            if ( etQuery != eType && isSQL92CheckEnabled(xConnection) && xMeta.is() )
                            {
                                ::rtl::OUString sAlias = ::dbtools::convertName2SQLName(sNewName,xMeta->getExtraNameCharacters());
                                if ( (xMeta.is() && xMeta->storesMixedCaseQuotedIdentifiers())
                                            ?
                                            sAlias != sNewName
                                            :
                                    !sNewName.equalsIgnoreAsciiCase(sAlias))
                                    {
                                        String sError = ErrorBox(getView(), ModuleRes(ERROR_INVALID_TABLE_NAME)).GetMessText();
                                        dbtools::throwGenericSQLException(sError, NULL);
                                    }
                            }

                            if ( (etQuery == eType || (xMeta.is() && xMeta->storesMixedCaseQuotedIdentifiers()))
                                            ?
                                            sName != sNewName
                                            :
                                    !sNewName.equalsIgnoreAsciiCase(sName)
                                )
                            {
                                Reference<XRename> xRename(pData->xObject,UNO_QUERY);
                                OSL_ENSURE(xRename.is(),"No Xrename interface!");
                                if(xRename.is())
                                {
                                    xRename->rename(sNewName);
                                     nRet = 1;
                                    if(etQuery != eType)
                                    {// special handling for tables and views
                                         xProp->getPropertyValue(PROPERTY_SCHEMANAME)  >>= sSchema;
                                        xProp->getPropertyValue(PROPERTY_CATALOGNAME) >>= sCatalog;
                                        ::dbtools::composeTableName(xMeta,sCatalog,sSchema,sNewName,sName,sal_False,::dbtools::eInDataManipulation);
                                        sOldName = sName;
                                        // now check if our datasource has set a tablefilter and if append the new table name to it
                                        ::dbaui::appendToFilter(xConnection,sOldName,getORB(),getView()); // we are not interessted in the return value
                                    }
                                    else
                                        sOldName = sNewName;

                                }
                            }
                        }
                    }
                    break;
            }
        }
        catch(const SQLException& e)
        {
            showError(SQLExceptionInfo(e));
        }
        catch(const ElementExistException& e)
        {
            static ::rtl::OUString sStatus = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("S1000"));
            String sMsg = String(ModuleRes(STR_OBJECT_ALREADY_EXISTS));
            sMsg.SearchAndReplace('#',e.Message);
            showError(SQLExceptionInfo(SQLException(sMsg, e.Context, sStatus, 0, Any())));
        }
        catch(const Exception& )
        {
            OSL_ENSURE(0,"Exception catched!");
        }
        _aEntry->aNewText = sOldName;

        return nRet;
    }
    // -----------------------------------------------------------------------------
    IMPL_LINK(SbaTableQueryBrowser, OnDeleteEntry, SvLBoxEntry*, _pEntry)
    {
        EntryType eType = getEntryType(_pEntry);
        switch(eType)
        {
            case etQuery:
                implRemoveQuery(_pEntry);
                break;
            case etView:
            case etTable:
                {
                    // check if connection is readonly
                    if(isConnectionWriteAble(_pEntry))
                        implDropTable(_pEntry);
                }
                break;
            case etBookmark:
                {
                    // get the container of the bookmarks
                    SvLBoxEntry* pContainer = isContainer(_pEntry) ? _pEntry : m_pTreeView->getListBox()->GetParent(_pEntry);
                    if (!ensureEntryObject(pContainer))
                        break;

                    String sSelectedObject = GetEntryText(_pEntry);

                    DBTreeListModel::DBTreeListUserData* pContainerData = static_cast<DBTreeListModel::DBTreeListUserData*>(pContainer->GetUserData());
                    Reference< XNameAccess > xBookmarks(pContainerData->xObject, UNO_QUERY);

                    OLinkedDocumentsAccess aHelper(getView(), getORB(), xBookmarks);
                    aHelper.drop(sSelectedObject);
                }
                break;
        }

        return 0;
    }
    // -----------------------------------------------------------------------------
    sal_Bool SbaTableQueryBrowser::isEntryCutAllowed(SvLBoxEntry* _pEntry) const
    {
        // at the momoent this isn't allowed
        return sal_False;
    }
    // -----------------------------------------------------------------------------
    sal_Bool SbaTableQueryBrowser::isEntryCopyAllowed(SvLBoxEntry* _pEntry) const
    {
        EntryType eType = getEntryType(_pEntry);
        return  (eType == etTable || eType == etQuery || eType == etView);
    }
    // -----------------------------------------------------------------------------
    sal_Bool SbaTableQueryBrowser::isEntryPasteAllowed(SvLBoxEntry* _pEntry) const
    {
        sal_Bool bAllowed = sal_False;
        EntryType eType = getEntryType(_pEntry);
        switch(eType)
        {
            case etQuery:
            case etQueryContainer:
            {
                bAllowed = getViewClipboard().HasFormat(SOT_FORMATSTR_ID_DBACCESS_QUERY);
                break;
            }
            case etView:
            case etTable:
            case etTableContainer:
                {
                    // check if connection is readonly
                    bAllowed = isConnectionWriteAble(_pEntry) && isTableFormat();
                }
                break;
        }
        return bAllowed;
    }
    // -----------------------------------------------------------------------------
    void SbaTableQueryBrowser::cutEntry(SvLBoxEntry* _pEntry)
    {
    }
    // -----------------------------------------------------------------------------
    void SbaTableQueryBrowser::copyEntry(SvLBoxEntry* _pEntry)
    {
        TransferableHelper* pTransfer = NULL;
        Reference< XTransferable> aEnsureDelete;
        EntryType eType = getEntryType(_pEntry);
        pTransfer       = implCopyObject( _pEntry, eType == etQuery ? CommandType::QUERY : CommandType::TABLE);
        aEnsureDelete   = pTransfer;
        if (pTransfer)
            pTransfer->CopyToClipboard(getView());
    }
    // -----------------------------------------------------------------------------
    void SbaTableQueryBrowser::pasteEntry(SvLBoxEntry* _pEntry)
    {
        EntryType eType = getEntryType(_pEntry);
        switch(eType)
        {
            case etQuery:
            case etQueryContainer:
                implPasteQuery(_pEntry, ODataAccessObjectTransferable::extractObjectDescriptor(getViewClipboard()));
                break;

            case etView:
            case etTable:
            case etTableContainer:
                implPasteTable( _pEntry, getViewClipboard() );
            default:
                ;
        }
    }
    // -----------------------------------------------------------------------------
    Reference<XConnection> SbaTableQueryBrowser::getConnectionFromEntry(SvLBoxEntry* _pEntry) const
    {
        DBTreeListModel::DBTreeListUserData* pDSData = NULL;
        DBTreeListModel::DBTreeListUserData* pEntryData = NULL;
        SvLBoxEntry* pDSEntry = NULL;
        pDSEntry = m_pTreeView->getListBox()->GetRootLevelParent(_pEntry);
        pDSData =   pDSEntry
                ?   static_cast<DBTreeListModel::DBTreeListUserData*>(pDSEntry->GetUserData())
                :   NULL;

        sal_Bool bIsConnectionWriteAble = sal_False;
        Reference<XConnection> xCon;
        if(pDSData && pDSData->xObject.is())
            xCon = Reference<XConnection>(pDSData->xObject,UNO_QUERY);
        return xCon;
    }
    // -----------------------------------------------------------------------------
    sal_Bool SbaTableQueryBrowser::isConnectionWriteAble(SvLBoxEntry* _pEntry) const
    {
        // check if connection is readonly
        sal_Bool bIsConnectionWriteAble = sal_False;
        Reference<XConnection> xCon = getConnectionFromEntry(_pEntry);
        if(xCon.is())
        {
            try
            {
                bIsConnectionWriteAble = !xCon->getMetaData()->isReadOnly();
            }
            catch(SQLException&)
            {
            }
        }
        return bIsConnectionWriteAble;
    }
// .........................................................................
}   // namespace dbaui
// .........................................................................

