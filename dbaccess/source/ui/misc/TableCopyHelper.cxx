/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: TableCopyHelper.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-26 14:51:09 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbaccess.hxx"
#ifndef DBUI_TABLECOPYHELPER_HXX
#include "TableCopyHelper.hxx"
#endif
#ifndef DBACCESS_SHARED_DBUSTRINGS_HRC
#include "dbustrings.hrc"
#endif
#ifndef _DBAUI_SQLMESSAGE_HXX_
#include "sqlmessage.hxx"
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef DBAUI_WIZ_COPYTABLEDIALOG_HXX
#include "WCopyTable.hxx"
#endif
#ifndef DBAUI_GENERICCONTROLLER_HXX
#include "genericcontroller.hxx"
#endif
#ifndef DBAUI_WIZARD_CPAGE_HXX
#include "WCPage.hxx"
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
#ifndef _COM_SUN_STAR_TASK_XINTERACTIONHANDLER_HPP_
#include <com/sun/star/task/XInteractionHandler.hpp>
#endif
#include <com/sun/star/sdb/XSingleSelectQueryComposer.hpp>

#ifndef DBAUI_RTFREADER_HXX
#include "RtfReader.hxx"
#endif
#ifndef DBAUI_HTMLREADER_HXX
#include "HtmlReader.hxx"
#endif
#ifndef DBAUI_TOKENWRITER_HXX
#include "TokenWriter.hxx"
#endif
#ifndef DBAUI_TOOLS_HXX
#include "UITools.hxx"
#endif
#ifndef DBAUI_DATAVIEW_HXX
#include "dataview.hxx"
#endif
#ifndef _DBU_RESOURCE_HRC_
#include "dbu_resource.hrc"
#endif
#ifndef _UNOTOOLS_UCBHELPER_HXX
#include <unotools/ucbhelper.hxx>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XTABLESSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XVIEWSSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XViewsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XQUERYDEFINITIONSSUPPLIER_HPP_
#include <com/sun/star/sdb/XQueryDefinitionsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_SQLCONTEXT_HPP_
#include <com/sun/star/sdb/SQLContext.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XPARAMETERS_HPP_
#include <com/sun/star/sdbc/XParameters.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XRESULTSETMETADATASUPPLIER_HPP_
#include <com/sun/star/sdbc/XResultSetMetaDataSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XQUERIESSUPPLIER_HPP_
#include <com/sun/star/sdb/XQueriesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XCOLUMNLOCATE_HPP_
#include <com/sun/star/sdbc/XColumnLocate.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XROWLOCATE_HPP_
#include <com/sun/star/sdbcx/XRowLocate.hpp>
#endif
#ifndef _SV_WAITOBJ_HXX
#include <vcl/waitobj.hxx>
#endif
#ifndef _COM_SUN_STAR_SDB_XSQLQUERYCOMPOSERFACTORY_HPP_
#include <com/sun/star/sdb/XSQLQueryComposerFactory.hpp>
#endif
#ifndef _UNOTOOLS_TEMPFILE_HXX
#include <unotools/tempfile.hxx>
#endif

#ifndef DBAUI_DBEXCHANGE_HXX
#include "dbexchange.hxx"
#endif
//........................................................................
namespace dbaui
{
//........................................................................
using namespace ::dbtools;
using namespace ::svx;
//  using namespace ::svtools;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::task;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::ucb;

    // -----------------------------------------------------------------------------
#define FILL_PARAM(type,method)                         \
{                                                   \
    type nValue = xRow->g##method(column);          \
    if ( !xRow->wasNull() )                         \
        xParameter->s##method(nPos,nValue);         \
    else                                            \
        xParameter->setNull(nPos,aColumnTypes[column]); \
}
// -----------------------------------------------------------------------------
namespace
{
void insertRows(const Reference<XResultSet>& xSrcRs,
                const ODatabaseExport::TPositions& _rvColumns,
                const Reference<XPropertySet>& _xDestTable,
                const Reference<XDatabaseMetaData>& _xMetaData,
                sal_Bool bIsAutoIncrement,
                const Sequence<Any>& _aSelection,
                sal_Bool _bBookmarkSelection,
                Window* _pParent
                ) throw(SQLException, RuntimeException)
{
    Reference< XResultSetMetaDataSupplier> xSrcMetaSup(xSrcRs,UNO_QUERY);
    Reference<XRow> xRow(xSrcRs,UNO_QUERY);
    Reference< XRowLocate > xRowLocate( xSrcRs, UNO_QUERY );
    sal_Bool bUseSelection  = _aSelection.getLength() > 0;

    if ( !xRow.is() || ( bUseSelection && _bBookmarkSelection && !xRowLocate.is() ) )
    {
        DBG_ERROR( "insertRows: bad arguments!" );
        return;
    }

    ::rtl::OUString aSql(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("INSERT INTO ")));
    ::rtl::OUString sComposedTableName = ::dbtools::composeTableName( _xMetaData, _xDestTable, ::dbtools::eInDataManipulation, false, false, true );

    aSql += sComposedTableName;
    aSql += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" ( "));
    // set values and column names
    ::rtl::OUString aValues(RTL_CONSTASCII_USTRINGPARAM(" VALUES ( "));
    static ::rtl::OUString aPara(RTL_CONSTASCII_USTRINGPARAM("?,"));
    static ::rtl::OUString aComma(RTL_CONSTASCII_USTRINGPARAM(","));

    ::rtl::OUString aQuote;
    if ( _xMetaData.is() )
        aQuote = _xMetaData->getIdentifierQuoteString();

    Reference<XColumnsSupplier> xColsSup(_xDestTable,UNO_QUERY);
    OSL_ENSURE(xColsSup.is(),"OTableCopyHelper::insertRows: No columnsSupplier!");
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

    // now create,fill and execute the prepared statement
    Reference< XPreparedStatement > xPrep(ODatabaseExport::createPreparedStatment(_xMetaData,_xDestTable,_rvColumns));
    Reference< XParameters > xParameter(xPrep,UNO_QUERY);

    sal_Int32 nRowCount = 0;
    const Any* pSelBegin    = _aSelection.getConstArray();
    const Any* pSelEnd      = pSelBegin + _aSelection.getLength();
    sal_Bool bNext = sal_True;
    do // loop as long as there are more rows or the selection ends
    {
        if ( bUseSelection )
        {
            if ( pSelBegin != pSelEnd )
            {
                if ( _bBookmarkSelection )
                {
                    xRowLocate->moveToBookmark( *pSelBegin );
                }
                else
                {
                    sal_Int32 nPos = 0;
                    *pSelBegin >>= nPos;
                    bNext = xSrcRs->absolute( nPos );
                }
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
            SQLExceptionInfo aInfo;
            try
            {
                for( sal_Int32 column = 1; aPosIter != _rvColumns.end(); ++aPosIter)
                {
                    sal_Int32 nPos = aPosIter->first;
                    if ( nPos == COLUMN_POSITION_NOT_FOUND )
                    {
                        ++column; // otherwise we don't get the correct value when only the 2nd source column was selected
                        continue;
                    }
                    if ( bIsAutoIncrement && bInsertAutoIncrement )
                    {
                        xParameter->setInt(1,nRowCount);
                        bInsertAutoIncrement = sal_False;
                        continue;
                    }
                    // we have to check here against 1 because the parameters are 1 based
                    OSL_ENSURE( column >= 1 && column < (sal_Int32)aColumnTypes.size(),"Index out of range for column types!");
                    switch(aColumnTypes[column])
                    {
                        case DataType::CHAR:
                        case DataType::VARCHAR:
                        case DataType::LONGVARCHAR:
                        case DataType::DECIMAL:
                        case DataType::NUMERIC:
                            FILL_PARAM( ::rtl::OUString, etString)
                            break;
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
                        case DataType::BOOLEAN:
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
                    ++column;
                }
                xPrep->executeUpdate();
            }
            catch(SQLContext& e) { aInfo = e; }
            catch(SQLWarning& e) { aInfo = e; }
            catch(SQLException& e) { aInfo = e; }

            if ( aInfo.isValid() )
            {
                String sAskIfContinue = String(ModuleRes(STR_ERROR_OCCURED_WHILE_COPYING));
                String sTitle = String(ModuleRes(STR_STAT_WARNING));
                OSQLMessageBox aDlg(_pParent,sTitle,sAskIfContinue,WB_YES_NO|WB_DEF_YES,OSQLMessageBox::Warning,&aInfo);
                if ( aDlg.Execute() != RET_YES )
                {
                    SQLException e;
                    switch( aInfo.getType() )
                    {
                        case SQLExceptionInfo::SQL_EXCEPTION:
                            throw *(const SQLException*)aInfo;
                        case SQLExceptionInfo::SQL_WARNING:
                            throw *(const SQLWarning*)aInfo;
                        case SQLExceptionInfo::SQL_CONTEXT:
                            throw *(const SQLContext*)aInfo;
                        default:
                            break;
                    }
                }
            }
        }
    }
    while( bNext );
}
// -----------------------------------------------------------------------------
Reference<XResultSet> createResultSet(  OGenericUnoController* _pBrowser,sal_Bool bDispose,
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
        ::rtl::OUString sComposedName = ::dbtools::composeTableNameForSelect( _xSrcConnection, xSourceObject );
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
            Reference< XMultiServiceFactory > xFactory( _xSrcConnection, UNO_QUERY );
            Reference< XSingleSelectQueryComposer > xComposer;
            if ( xFactory.is() )
                xComposer.set( xFactory->createInstance( SERVICE_NAME_SINGLESELECTQUERYCOMPOSER ), UNO_QUERY );
            if ( xComposer.is() )
            {
                try
                {
                    xComposer->setQuery(sSql);
                    Reference< XInteractionHandler > xHandler(_pBrowser->getORB()->createInstance(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.sdb.InteractionHandler"))), UNO_QUERY);
                    ::dbtools::askForParameters(xComposer,Reference<XParameters>(xPrepStmt,UNO_QUERY),_xSrcConnection,xHandler);
                    xSrcRs = xPrepStmt->executeQuery();
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
}
// -----------------------------------------------------------------------------
OTableCopyHelper::OTableCopyHelper(OGenericUnoController* _pControler)
    :m_pController(_pControler)
    ,m_bSelectCopyTable(sal_False)
{
}
// -----------------------------------------------------------------------------
void OTableCopyHelper::pasteTable( SotFormatStringId _nFormatId
                                  ,const TransferableDataHelper& _rTransData
                                  ,const ::rtl::OUString& _sDestDataSourceName
                                  ,const SharedConnection& _xConnection)
{
    if ( _nFormatId == SOT_FORMATSTR_ID_DBACCESS_TABLE || _nFormatId == SOT_FORMATSTR_ID_DBACCESS_QUERY )
    {
        if ( ODataAccessObjectTransferable::canExtractObjectDescriptor(_rTransData.GetDataFlavorExVector()) )
        {
            ::svx::ODataAccessDescriptor aPasteData = ODataAccessObjectTransferable::extractObjectDescriptor(_rTransData);
            pasteTable( aPasteData,_sDestDataSourceName,_xConnection);
        }
    }
    else if ( _rTransData.HasFormat(_nFormatId) )
    {
        try
        {
            DropDescriptor aTrans;
            if ( _nFormatId != SOT_FORMAT_RTF )
                const_cast<TransferableDataHelper&>(_rTransData).GetSotStorageStream(_rTransData.HasFormat(SOT_FORMATSTR_ID_HTML) ? SOT_FORMATSTR_ID_HTML : SOT_FORMATSTR_ID_HTML_SIMPLE,aTrans.aHtmlRtfStorage);
            else
                const_cast<TransferableDataHelper&>(_rTransData).GetSotStorageStream(SOT_FORMAT_RTF,aTrans.aHtmlRtfStorage);

            aTrans.nType            = E_TABLE;
            aTrans.bHtml            = SOT_FORMATSTR_ID_HTML == _nFormatId || SOT_FORMATSTR_ID_HTML_SIMPLE == _nFormatId;
            ///dyf add 20070601
            //add for transfor the selected tablename
            aTrans.sDefaultTableName       = GetDefaultTableName();
            //dyf add end
            if ( !copyTagTable(aTrans,sal_False,_xConnection) )
                m_pController->showError(SQLException(String(ModuleRes(STR_NO_TABLE_FORMAT_INSIDE)),*m_pController,::rtl::OUString::createFromAscii("S1000") ,0,Any()));
        }
        catch(SQLContext& e) { m_pController->showError(SQLExceptionInfo(e)); }
        catch(SQLWarning& e) { m_pController->showError(SQLExceptionInfo(e)); }
        catch(SQLException& e) { m_pController->showError(SQLExceptionInfo(e)); }
        catch(Exception& )
        {
            OSL_ENSURE(sal_False, "OTableCopyHelper::pasteTable: caught a generic exception!");
        }
    }
    else
        m_pController->showError(SQLException(String(ModuleRes(STR_NO_TABLE_FORMAT_INSIDE)),*m_pController,::rtl::OUString::createFromAscii("S1000") ,0,Any()));
}
// -----------------------------------------------------------------------------
void OTableCopyHelper::pasteTable( const TransferableDataHelper& _rTransData
                                  ,const ::rtl::OUString& _sDestDataSourceName
                                  ,const SharedConnection& _xConnection)
{
    if ( _rTransData.HasFormat(SOT_FORMATSTR_ID_DBACCESS_TABLE) || _rTransData.HasFormat(SOT_FORMATSTR_ID_DBACCESS_QUERY) )
        pasteTable( SOT_FORMATSTR_ID_DBACCESS_TABLE,_rTransData,_sDestDataSourceName,_xConnection);
    else if ( _rTransData.HasFormat(SOT_FORMATSTR_ID_HTML) )
        pasteTable( SOT_FORMATSTR_ID_HTML,_rTransData,_sDestDataSourceName,_xConnection);
    else if ( _rTransData.HasFormat(SOT_FORMATSTR_ID_HTML_SIMPLE) )
        pasteTable( SOT_FORMATSTR_ID_HTML_SIMPLE,_rTransData,_sDestDataSourceName,_xConnection);
    else if ( _rTransData.HasFormat(SOT_FORMAT_RTF) )
        pasteTable( SOT_FORMAT_RTF,_rTransData,_sDestDataSourceName,_xConnection);
}
// -----------------------------------------------------------------------------
void OTableCopyHelper::pasteTable( ::svx::ODataAccessDescriptor& _rPasteData
                                  ,const ::rtl::OUString& _sDestDataSourceName
                                  ,const SharedConnection& _xDestConnection)
{
    Reference<XConnection> xSrcConnection;
    Reference<XResultSet>   xSrcRs;         // the source resultset may be empty
    Sequence< Any > aSelection;
    sal_Bool bBookmarkSelection( sal_False );
    ::rtl::OUString sCommand,
        sSrcDataSourceName = _rPasteData.getDataSource();

    _rPasteData[daCommand]          >>= sCommand;
    if ( _rPasteData.has(daConnection) )
        _rPasteData[daConnection]   >>= xSrcConnection;
    if ( _rPasteData.has(daSelection) )
        _rPasteData[daSelection]    >>= aSelection;
    if ( _rPasteData.has(daBookmarkSelection) )
        _rPasteData[daBookmarkSelection]    >>= bBookmarkSelection;
    if ( _rPasteData.has(daCursor) )
        _rPasteData[daCursor]       >>= xSrcRs;

    // paste into the tables
    sal_Int32 nCommandType = CommandType::COMMAND;
    if ( _rPasteData.has(daCommandType) )
        _rPasteData[daCommandType] >>= nCommandType;

    insertTable( nCommandType
                ,xSrcConnection
                ,xSrcRs
                ,aSelection
                ,bBookmarkSelection
                ,sCommand
                ,sSrcDataSourceName
                ,_sDestDataSourceName
                ,_xDestConnection);
}
// -----------------------------------------------------------------------------
void OTableCopyHelper::insertTable(sal_Int32 _nCommandType
                                        ,const Reference<XConnection>& _xSrcConnection
                                        ,const Reference<XResultSet>&   _xSrcRs         // the source resultset may be empty
                                        ,const Sequence< Any >& _aSelection
                                        ,sal_Bool _bBookmarkSelection
                                        ,const ::rtl::OUString& _sCommand
                                        ,const ::rtl::OUString& _sSrcDataSourceName
                                        ,const ::rtl::OUString& _sDestDataSourceName
                                        ,const Reference<XConnection>& _xDestConnection)
{
    try
    {
        if ( CommandType::QUERY == _nCommandType || CommandType::TABLE == _nCommandType )
        {
            // first get the dest connection
            Reference<XConnection> xSrcConnection;   // supports the service sdb::connection
            if ( !_xDestConnection.is() )
                return;

            xSrcConnection = _xSrcConnection;
            Reference<XResultSet> xSrcRs = _xSrcRs;


            // get the source connection
            sal_Bool bDispose = sal_False;
            if ( _sSrcDataSourceName == _sDestDataSourceName )
                xSrcConnection = _xDestConnection;
            else if ( !xSrcConnection.is() )
            {
                OSL_ENSURE(0,"Error: connection should not be null!");
                /*
                Reference< XEventListener> xEvt(static_cast< ::cppu::OWeakObject*>(m_pController), UNO_QUERY);
                m_pController->showError(::dbaui::createConnection(m_xDataSource,m_pController->getORB(),xEvt,xSrcConnection));
                bDispose = sal_True;
                */
            }
            Reference<XNameAccess> xNameAccess;
            sal_Bool bTable = sal_True;
            switch(_nCommandType)
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
            if ( xNameAccess.is() && xNameAccess->hasByName( _sCommand ) )
            {
                Reference<XPropertySet> xSourceObject(xNameAccess->getByName( _sCommand ),UNO_QUERY);

                sal_Bool bIsView = sal_False;
                if ( bTable ) // we only have to check if this table has as type VIEW
                {
                    static ::rtl::OUString sVIEW = ::rtl::OUString::createFromAscii("VIEW");
                    bIsView = ::comphelper::getString(xSourceObject->getPropertyValue(PROPERTY_TYPE)) == sVIEW;
                }

                OCopyTableWizard aWizard(m_pController->getView(),
                                            xSourceObject,
                                            xSrcConnection,
                                            _xDestConnection,
                                            getNumberFormatter(_xDestConnection,m_pController->getORB()),
                                            m_pController->getORB());

                //--------dyf modify 2006/6/27
                aWizard.fillTypeInfo();
                aWizard.loadData();
                if(GetIsSelectCopytable())
                {
                    aWizard.ResetsName( GetDefaultTableName());
                    aWizard.setCreateStyle(OCopyTableWizard::WIZARD_APPEND_DATA);
                }

                OCopyTable*         pPage1;
                pPage1 = new OCopyTable(&aWizard,COPY, bIsView);
                pPage1->setCreateStyleAction();

                //--------dyf modify end
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
                    WaitObject aWO(m_pController->getView());
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
                            } // run through
                        case OCopyTableWizard::WIZARD_APPEND_DATA:
                            {

                                if(!xTable.is())
                                    xTable = aWizard.createTable();
                                if(!xTable.is())
                                    break;

                                Reference<XStatement> xStmt; // needed to hold a reference to the statement
                                Reference<XPreparedStatement> xPrepStmt;// needed to hold a reference to the statement

                                ODatabaseExport::TPositions aColumnMapping = aWizard.GetColumnPositions();
                                // create the sql stmt
                                if ( !xSrcRs.is() ) // if not already exists
                                    xSrcRs = createResultSet(m_pController,
                                                            bDispose,
                                                            _nCommandType,
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
                                    Reference<XNameAccess> xSourceColumns = xSrcColsSup->getColumns();
                                    Sequence< ::rtl::OUString> aSeq = xSourceColumns->getElementNames();
                                    const ::rtl::OUString* pBegin = aSeq.getConstArray();
                                    const ::rtl::OUString* pEnd   = pBegin + aSeq.getLength();

                                    ODatabaseExport::TPositions aNewColMapping;
                                    aNewColMapping.resize( aColumnMapping.size() ,ODatabaseExport::TPositions::value_type( COLUMN_POSITION_NOT_FOUND, COLUMN_POSITION_NOT_FOUND ) );

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
                                            _xDestConnection->getMetaData(),
                                            aWizard.isAutoincrementEnabled(),
                                            _aSelection,
                                            _bBookmarkSelection,
                                            m_pController->getView());
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
                switch(_nCommandType)
                {
                    case CommandType::TABLE:
                        break;
                    case CommandType::QUERY:
                        break;
                }
                m_pController->showError(SQLException(String(ModuleRes(STR_NO_TABLE_FORMAT_INSIDE)),*m_pController,::rtl::OUString::createFromAscii("S1000") ,0,Any()));
            }
            if ( bDispose )
                ::comphelper::disposeComponent(xSrcConnection);
        }
        else
            DBG_ERROR("OTableCopyHelper::pasteTable: invalid call (no supported format found)!");
    }
    catch(SQLContext& e) { m_pController->showError(SQLExceptionInfo(e)); }
    catch(SQLWarning& e) { m_pController->showError(SQLExceptionInfo(e)); }
    catch(SQLException& e) { m_pController->showError(SQLExceptionInfo(e)); }
    catch(Exception& )
    {
        OSL_ENSURE(sal_False, "OTableCopyHelper::pasteTable: caught a generic exception!");
    }
}
// -----------------------------------------------------------------------------
sal_Bool OTableCopyHelper::copyTagTable(OTableCopyHelper::DropDescriptor& _rDesc, sal_Bool _bCheck,const SharedConnection& _xConnection)
{
    Reference<XEventListener> xEvt;
    ODatabaseImportExport* pImport = NULL;
    if ( _rDesc.bHtml )
        pImport = new OHTMLImportExport(_xConnection,getNumberFormatter(_xConnection,m_pController->getORB()),m_pController->getORB());
    else
        pImport = new ORTFImportExport(_xConnection,getNumberFormatter(_xConnection,m_pController->getORB()),m_pController->getORB());

    xEvt = pImport;
    SvStream* pStream = (SvStream*)(SotStorageStream*)_rDesc.aHtmlRtfStorage;
    if ( _bCheck )
        pImport->enableCheckOnly();

    //dyf add 20070601
    //set the selected tablename
    pImport->setSTableName(_rDesc.sDefaultTableName);
    //dyf add end
    pImport->setStream(pStream);
    return pImport->Read();
}
// -----------------------------------------------------------------------------
sal_Bool OTableCopyHelper::isTableFormat(const TransferableDataHelper& _rClipboard)  const
{
    sal_Bool bTableFormat   =   _rClipboard.HasFormat(SOT_FORMATSTR_ID_DBACCESS_TABLE)
                ||  _rClipboard.HasFormat(SOT_FORMATSTR_ID_DBACCESS_QUERY)
                ||  _rClipboard.HasFormat(SOT_FORMAT_RTF)
                ||  _rClipboard.HasFormat(SOT_FORMATSTR_ID_HTML)
                ||  _rClipboard.HasFormat(SOT_FORMATSTR_ID_HTML_SIMPLE);

    return bTableFormat;
}
// -----------------------------------------------------------------------------
sal_Bool OTableCopyHelper::copyTagTable(const TransferableDataHelper& _aDroppedData
                                        ,DropDescriptor& _rAsyncDrop
                                        ,const SharedConnection& _xConnection)
{
    sal_Bool bRet = sal_False;
    sal_Bool bHtml = _aDroppedData.HasFormat(SOT_FORMATSTR_ID_HTML) || _aDroppedData.HasFormat(SOT_FORMATSTR_ID_HTML_SIMPLE);
    if ( bHtml || _aDroppedData.HasFormat(SOT_FORMAT_RTF) )
    {
        if ( bHtml )
            const_cast<TransferableDataHelper&>(_aDroppedData).GetSotStorageStream(_aDroppedData.HasFormat(SOT_FORMATSTR_ID_HTML) ? SOT_FORMATSTR_ID_HTML : SOT_FORMATSTR_ID_HTML_SIMPLE,_rAsyncDrop.aHtmlRtfStorage);
        else
            const_cast<TransferableDataHelper&>(_aDroppedData).GetSotStorageStream(SOT_FORMAT_RTF,_rAsyncDrop.aHtmlRtfStorage);

        _rAsyncDrop.bHtml           = bHtml;
        _rAsyncDrop.bError          = !copyTagTable(_rAsyncDrop,sal_True,_xConnection);

        bRet = ( !_rAsyncDrop.bError && _rAsyncDrop.aHtmlRtfStorage.Is() );
        if ( bRet )
        {
            // now we need to copy the stream
            ::utl::TempFile aTmp;
            aTmp.EnableKillingFile(sal_False);
            _rAsyncDrop.aUrl = aTmp.GetURL();
            SotStorageStreamRef aNew = new SotStorageStream( aTmp.GetFileName() );
            _rAsyncDrop.aHtmlRtfStorage->Seek(STREAM_SEEK_TO_BEGIN);
            _rAsyncDrop.aHtmlRtfStorage->CopyTo( aNew );
            aNew->Commit();
            _rAsyncDrop.aHtmlRtfStorage = aNew;
        }
        else
            _rAsyncDrop.aHtmlRtfStorage = NULL;
    }
    return bRet;
}
// -----------------------------------------------------------------------------
void OTableCopyHelper::asyncCopyTagTable(  DropDescriptor& _rDesc
                                ,const ::rtl::OUString& _sDestDataSourceName
                                ,const SharedConnection& _xConnection)
{
    if ( _rDesc.aHtmlRtfStorage.Is() )
    {
        copyTagTable(_rDesc,sal_False,_xConnection);
        _rDesc.aHtmlRtfStorage = NULL;
        // we now have to delete the temp file created in executeDrop
        INetURLObject aURL;
        aURL.SetURL(_rDesc.aUrl);
        ::utl::UCBContentHelper::Kill(aURL.GetMainURL(INetURLObject::NO_DECODE));
    }
    else if ( !_rDesc.bError )
        pasteTable(_rDesc.aDroppedData,_sDestDataSourceName,_xConnection);
    else
        m_pController->showError(SQLException(String(ModuleRes(STR_NO_TABLE_FORMAT_INSIDE)),*m_pController,::rtl::OUString::createFromAscii("S1000") ,0,Any()));
}
// -----------------------------------------------------------------------------
//........................................................................
}   // namespace dbaui
//........................................................................

