/*************************************************************************
 *
 *  $RCSfile: AppControllerDnD.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 15:28:29 $
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
 *  Contributor(s): Ocke Janssen
 *
 *
 ************************************************************************/

#ifndef DBAUI_APPCONTROLLER_HXX
#include "AppController.hxx"
#endif
#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif
#ifndef DBACCESS_SHARED_DBUSTRINGS_HRC
#include "dbustrings.hrc"
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_XNAMINGSERVICE_HPP_
#include <com/sun/star/uno/XNamingService.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XDATASOURCE_HPP_
#include <com/sun/star/sdbc/XDataSource.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XSTORABLE_HPP_
#include <com/sun/star/frame/XStorable.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XPARAMETERS_HPP_
#include <com/sun/star/sdbc/XParameters.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCHILD_HPP_
#include <com/sun/star/container/XChild.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XHIERARCHICALNAMECONTAINER_HPP_
#include <com/sun/star/container/XHierarchicalNameContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_DATATYPE_HPP_
#include <com/sun/star/sdbc/DataType.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XBOOKMARKSSUPPLIER_HPP_
#include <com/sun/star/sdb/XBookmarksSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XQUERIESSUPPLIER_HPP_
#include <com/sun/star/sdb/XQueriesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XRESULTSETMETADATASUPPLIER_HPP_
#include <com/sun/star/sdbc/XResultSetMetaDataSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XSQLQUERYCOMPOSERFACTORY_HPP_
#include <com/sun/star/sdb/XSQLQueryComposerFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_SQLCONTEXT_HPP_
#include <com/sun/star/sdb/SQLContext.hpp>
#endif
#ifndef _COM_SUN_STAR_TASK_XINTERACTIONHANDLER_HPP_
#include <com/sun/star/task/XInteractionHandler.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XCOLUMNLOCATE_HPP_
#include <com/sun/star/sdbc/XColumnLocate.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XROWLOCATE_HPP_
#include <com/sun/star/sdbcx/XRowLocate.hpp>
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
#ifndef _COM_SUN_STAR_SDBCX_XDROP_HPP_
#include <com/sun/star/sdbcx/XDrop.hpp>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _UNOTOOLS_UCBHELPER_HXX
#include <unotools/ucbhelper.hxx>
#endif
#ifndef DBAUI_DLGSAVE_HXX
#include "dlgsave.hxx"
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif
#ifndef _DBHELPER_DBEXCEPTION_HXX_
#include <connectivity/dbexception.hxx>
#endif
#ifndef _SV_WAITOBJ_HXX
#include <vcl/waitobj.hxx>
#endif
#ifndef DBAUI_APPVIEW_HXX
#include "AppView.hxx"
#endif
#ifndef _SVX_DATACCESSDESCRIPTOR_HXX_
#include <svx/dataaccessdescriptor.hxx>
#endif
#ifndef SVX_DBAOBJECTEX_HXX
#include <svx/dbaobjectex.hxx>
#endif
#ifndef DBACCESS_UI_BROWSER_ID_HXX
#include "browserids.hxx"
#endif
#ifndef _DBAU_REGHELPER_HXX_
#include "dbu_reghelper.hxx"
#endif
#ifndef _DBU_APP_HRC_
#include "dbu_app.hrc"
#endif
#ifndef _SV_TOOLBOX_HXX //autogen wg. ToolBox
#include <vcl/toolbox.hxx>
#endif
#ifndef _SV_MENU_HXX
#include <vcl/menu.hxx>
#endif
#ifndef _COMPHELPER_UNO3_HXX_
#include <comphelper/uno3.hxx>
#endif
#ifndef _DBAUI_QUERYDESIGNACCESS_HXX_
#include "querydesignaccess.hxx"
#endif
#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#ifndef _SVLBOXITM_HXX
#include <svtools/svlbitm.hxx>
#endif
#ifndef _DBAUI_LISTVIEWITEMS_HXX_
#include "listviewitems.hxx"
#endif
#ifndef DBAUI_APPDETAILVIEW_HXX
#include "AppDetailView.hxx"
#endif
#ifndef _DBAUI_LINKEDDOCUMENTS_HXX_
#include "linkeddocuments.hxx"
#endif
#ifndef _SV_LSTBOX_HXX
#include <vcl/lstbox.hxx>
#endif
#ifndef _DBHELPER_DBEXCEPTION_HXX_
#include <connectivity/dbexception.hxx>
#endif
#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include <connectivity/dbtools.hxx>
#endif
#ifndef _DBAUI_SQLMESSAGE_HXX_
#include "sqlmessage.hxx"
#endif
#ifndef _STRING_HXX
#include <tools/string.hxx>
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
#ifndef DBAUI_RTFREADER_HXX
#include "RtfReader.hxx"
#endif
#ifndef DBAUI_HTMLREADER_HXX
#include "HtmlReader.hxx"
#endif
#ifndef DBAUI_TOOLS_HXX
#include "UITools.hxx"
#endif
#include <algorithm>
#ifndef _SVTREEBOX_HXX
#include <svtools/svtreebx.hxx>
#endif
#ifndef _COM_SUN_STAR_SDB_XREPORTDOCUMENTSSUPPLIER_HPP_
#include <com/sun/star/sdb/XReportDocumentsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XFORMDOCUMENTSSUPPLIER_HPP_
#include <com/sun/star/sdb/XFormDocumentsSupplier.hpp>
#endif
#ifndef _FILEDLGHELPER_HXX
#include <sfx2/filedlghelper.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_PATHOPTIONS_HXX
#include <svtools/pathoptions.hxx>
#endif
#ifndef _SFX_DOCFILT_HACK_HXX
#include <sfx2/docfilt.hxx>
#endif
#ifndef _SVT_FILEVIEW_HXX
#include <svtools/fileview.hxx>
#endif

//........................................................................
namespace dbaui
{
//........................................................................
using namespace ::dbtools;
using namespace ::svx;
using namespace ::svtools;
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
    type nValue = xRow->g##method(i);               \
    if ( !xRow->wasNull() )                         \
        xParameter->s##method(nPos,nValue);         \
    else                                            \
        xParameter->setNull(nPos,aColumnTypes[i]);  \
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
    ::rtl::OUString sComposedTableName = ::dbtools::composeTableName(_xMetaData,_xDestTable,sal_True,::dbtools::eInDataManipulation);

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
    OSL_ENSURE(xColsSup.is(),"OApplicationController::insertRows: No columnsSupplier!");
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
    ::std::vector< ::rtl::OUString> aInsertList;
    aInsertList.resize(aSeq.getLength()+1);
    sal_Int32 i = 0;
    for(sal_Int32 j=0; j < aInsertList.size() ;++i,++j)
    {
        ODatabaseExport::TPositions::const_iterator aFind = ::std::find_if(_rvColumns.begin(),_rvColumns.end(),
            ::std::compose1(::std::bind2nd(::std::equal_to<sal_Int32>(),i+1),::std::select2nd<ODatabaseExport::TPositions::value_type>()));
        if ( _rvColumns.end() != aFind && aFind->second != CONTAINER_ENTRY_NOTFOUND && aFind->first != CONTAINER_ENTRY_NOTFOUND )
        {
            aInsertList[aFind->first] = ::dbtools::quoteName( aQuote,*(pBegin+i));
        }
    }

    i = 1;
    // create the sql string
    for (::std::vector< ::rtl::OUString>::iterator aInsertIter = aInsertList.begin(); aInsertIter != aInsertList.end(); ++aInsertIter)
    {
        if ( aInsertIter->getLength() )
        {
            aSql += *aInsertIter;
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
    sal_Bool bNext = sal_True;
    sal_Bool bAlreadyAsked = sal_False;
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
                for(sal_Int32 i = 1;aPosIter != _rvColumns.end();++aPosIter)
                {
                    sal_Int32 nPos = aPosIter->first;
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
                    OSL_ENSURE( i >= 1 && i < (sal_Int32)aColumnTypes.size(),"Index out of range for column types!");
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
            catch(SQLContext& e) { aInfo = e; }
            catch(SQLWarning& e) { aInfo = e; }
            catch(SQLException& e) { aInfo = e; }

            if ( aInfo.isValid() && !bAlreadyAsked )
            {
                String sAskIfContinue = String(ModuleRes(STR_ERROR_OCCURED_WHILE_COPYING));
                String sTitle = String(ModuleRes(STR_STAT_WARNING));
                OSQLMessageBox aDlg(_pParent,sTitle,sAskIfContinue,WB_YES_NO|WB_DEF_YES,OSQLMessageBox::Warning);
                if ( aDlg.Execute() == RET_YES )
                    bAlreadyAsked = sal_True;
                else
                {
                    SQLException e;
                    switch( aInfo.getType() )
                    {
                        case SQLExceptionInfo::SQL_EXCEPTION:
                            throw *(const SQLException*)aInfo;
                            break;
                        case SQLExceptionInfo::SQL_WARNING:
                            throw *(const SQLWarning*)aInfo;
                            break;
                        case SQLExceptionInfo::SQL_CONTEXT:
                            throw *(const SQLContext*)aInfo;
                            break;
                    }
                }
            }
        }
    }
    while( bNext );
}
// -----------------------------------------------------------------------------
Reference<XResultSet> createResultSet(  OApplicationController* _pBrowser,sal_Bool bDispose,
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
        sal_Bool bUseCatalogInSelect = ::dbtools::isDataSourcePropertyEnabled(_xSrcConnection,PROPERTY_USECATALOGINSELECT,sal_True);
        sal_Bool bUseSchemaInSelect = ::dbtools::isDataSourcePropertyEnabled(_xSrcConnection,PROPERTY_USESCHEMAINSELECT,sal_True);
        ::rtl::OUString sComposedName = ::dbtools::composeTableName(_xSrcConnection->getMetaData(),xSourceObject,sal_True,::dbtools::eInDataManipulation,bUseCatalogInSelect,bUseSchemaInSelect);
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
}
// -----------------------------------------------------------------------------
void OApplicationController::deleteTables(const ::std::vector< ::rtl::OUString>& _rList)
{
    Reference<XConnection> xConnection;
    ensureConnection(xConnection);

    Reference<XTablesSupplier> xSup(xConnection,UNO_QUERY);
    OSL_ENSURE(xSup.is(),"OApplicationController::deleteTable: no XTablesSuppier!");
    if ( xSup.is() )
    {
        Reference<XNameAccess> xTables = xSup->getTables();
        Reference<XDrop> xDrop(xTables,UNO_QUERY);
        if ( xDrop.is() )
        {
            bool bConfirm = true;
            ::std::vector< ::rtl::OUString>::const_iterator aEnd = _rList.end();
            for (::std::vector< ::rtl::OUString>::const_iterator aIter = _rList.begin(); aIter != aEnd; ++aIter)
            {
                ::rtl::OUString sTableName = *aIter;

                sal_Int32 nResult = RET_YES;
                if ( bConfirm )
                    nResult = ::dbaui::askForUserAction(getView(),STR_TITLE_CONFIRM_DELETION ,STR_QUERY_DELETE_TABLE,_rList.size() > 1 && (aIter+1) != _rList.end(),sTableName);
                if ( (RET_YES == nResult) || (RET_ALL == nResult) )
                {
                    SQLExceptionInfo aErrorInfo;
                    try
                    {
                        if ( xTables->hasByName(sTableName) )
                            xDrop->dropByName(sTableName);
                        else
                        {// could be a view
                            Reference<XViewsSupplier> xViewsSup(xConnection,UNO_QUERY);

                            Reference<XNameAccess> xViews;
                            if ( xViewsSup.is() )
                            {
                                xViews = xViewsSup->getViews();
                                if ( xViews.is() && xViews->hasByName(sTableName) )
                                {
                                    xDrop.set(xViews,UNO_QUERY);
                                    if ( xDrop.is() )
                                        xDrop->dropByName(sTableName);
                                }
                            }
                        }
                    }
                    catch(SQLContext& e) { aErrorInfo = e; }
                    catch(SQLWarning& e) { aErrorInfo = e; }
                    catch(SQLException& e) { aErrorInfo = e; }
                    catch(WrappedTargetException& e)
                    {
                        SQLException aSql;
                        if(e.TargetException >>= aSql)
                            aErrorInfo = aSql;
                        else
                            OSL_ENSURE(sal_False, "OApplicationController::implDropTable: something strange happended!");
                    }
                    catch(Exception&)
                    {
                        DBG_ERROR("OApplicationController::implDropTable: suspicious exception caught!");
                    }

                    if ( aErrorInfo.isValid() )
                        showError(aErrorInfo);

                    if ( RET_ALL == nResult )
                        bConfirm = false;
                }
                else
                    break;
            }
        }
        else
        {
            String sMessage(ModuleRes(STR_MISSING_TABLES_XDROP));
            ErrorBox aError(getView(), WB_OK, sMessage);
            aError.Execute();
        }
    }
}
// -----------------------------------------------------------------------------
void OApplicationController::deleteObjects(ElementType _eType
                                           ,const ::std::vector< ::rtl::OUString>& _rList
                                           ,sal_uInt16 _nTextResource)
{
    Reference<XNameContainer> xNames(getElements(_eType), UNO_QUERY);
    dbaui::deleteObjects(getView(),getORB(),xNames,_rList,_nTextResource);
}
// -----------------------------------------------------------------------------
void OApplicationController::deleteEntries()
{
    ::vos::OGuard aSolarGuard(Application::GetSolarMutex());
    ::osl::MutexGuard aGuard(m_aMutex);

    if ( getContainer() )
    {
        ::std::vector< ::rtl::OUString> aList;
        getSelectionElementNames(aList);
        ElementType eType = getContainer()->getElementType();
        switch(eType)
        {
        case E_TABLE:
            deleteTables(aList);
            break;
        case E_QUERY:
            deleteObjects(E_QUERY,aList,STR_QUERY_DELETE_QUERY);
            break;
        case E_FORM:
            deleteObjects(E_FORM,aList,STR_QUERY_DELETE_FORM);
            break;
        case E_REPORT:
            deleteObjects(E_REPORT,aList,STR_QUERY_DELETE_REPORT);
            break;
        }
    }
}
// -----------------------------------------------------------------------------
Reference<XConnection> OApplicationController::getActiveConnection() const
{
    Reference<XConnection> xConnection;
    if ( getContainer() )
    {
        ::rtl::OUString sDataSourceName = getDatabaseName();
        TDataSourceConnections::const_iterator aFind = m_aDataSourceConnections.find(sDataSourceName);
        if ( aFind != m_aDataSourceConnections.end() )
            xConnection = aFind->second;
    }

    return xConnection;
}
// -----------------------------------------------------------------------------
void OApplicationController::ensureConnection(Reference<XConnection>& _xConnection,sal_Bool _bCreate)
{
    ::vos::OGuard aSolarGuard(Application::GetSolarMutex());
    ::osl::MutexGuard aGuard(m_aMutex);

    ::rtl::OUString sDataSourceName = getDatabaseName();
    TDataSourceConnections::iterator aFind = m_aDataSourceConnections.find(sDataSourceName);
    if ( aFind == m_aDataSourceConnections.end() )
        aFind = m_aDataSourceConnections.insert(TDataSourceConnections::value_type(sDataSourceName,Reference<XConnection>())).first;

    if ( !aFind->second.is() && _bCreate )
    {
        WaitObject aWO(getView());
        String sConnectingContext( ModuleRes( STR_COULDNOTCONNECT_DATASOURCE ) );
        sConnectingContext.SearchAndReplaceAscii("$name$", sDataSourceName);

        aFind->second = connect(sDataSourceName, sConnectingContext, rtl::OUString(), sal_True);
        // otherwise we got a loop when connecting to db throws an error
//      if ( !aFind->second.is() )
//          getContainer()->clearSelection();
    }
    _xConnection = aFind->second;
}
// -----------------------------------------------------------------------------
sal_Bool OApplicationController::isDataSourceReadOnly() const
{
    Reference<XStorable> xStore(m_xDataSource,UNO_QUERY);
    return !xStore.is() || xStore->isReadonly();
}
// -----------------------------------------------------------------------------
sal_Bool OApplicationController::isConnectionReadOnly() const
{
    sal_Bool bIsConnectionReadOnly = sal_True;
    Reference<XConnection> xConnection = getActiveConnection();

    if ( xConnection.is() )
    {
        try
        {
            bIsConnectionReadOnly = xConnection->getMetaData()->isReadOnly();
        }
        catch(SQLException&)
        {
        }
    }
    // TODO check configuration
    return bIsConnectionReadOnly;
}
// -----------------------------------------------------------------------------
sal_Bool OApplicationController::isRelationDesignAllowed() const
{
    return sal_True;
}
// -----------------------------------------------------------------------------
Reference< XNameAccess > OApplicationController::getElements(ElementType _eType)
{
    OSL_ENSURE(getContainer(),"View is NULL! -> GPF");
    // TODO get a list for all object
    ::rtl::OUString sDataSource = getDatabaseName();

    Reference< XNameAccess > xElements;
    try
    {
        switch ( _eType )
        {
            case E_REPORT: // TODO: seperate handling of forms and reports
                {
                    Reference< XReportDocumentsSupplier > xSupp(m_xDataSource,UNO_QUERY);
                    OSL_ENSURE(xSupp.is(),"Data source doesn't return a XReportDocumentsSupplier -> GPF");
                    if ( xSupp.is() )
                        xElements = xSupp->getReportDocuments();
                }
                break;
            case E_FORM:
                {
                    Reference< XFormDocumentsSupplier > xSupp(m_xDataSource,UNO_QUERY);
                    OSL_ENSURE(xSupp.is(),"Data source doesn't return a XFormDocumentsSupplier -> GPF");
                    if ( xSupp.is() )
                        xElements = xSupp->getFormDocuments();
                }
                break;
            case E_QUERY:
                {
                    xElements.set(getQueryDefintions(),UNO_QUERY);
                }
                break;
            case E_TABLE:
                {
                    Reference<XConnection> xConnection;
                    ensureConnection(xConnection,sal_False);
                    if ( xConnection.is() )
                    {
                        Reference<XTablesSupplier> xSup(xConnection,UNO_QUERY);
                        OSL_ENSURE(xSup.is(),"OApplicationController::getElements: no XTablesSuppier!");
                        if ( xSup.is() )
                            xElements = xSup->getTables();
                    }
                }
                break;
        }
    }
    catch(const Exception&)
    {
        OSL_ENSURE(0,"Could not get element container!");
    }
    return xElements;
}
// -----------------------------------------------------------------------------
void OApplicationController::getElements(ElementType _eType,::std::vector< ::rtl::OUString>& _rList)
{
    OSL_ENSURE(getContainer(),"View is NULL! -> GPF");
    // TODO get a list for all object
    Reference< XNameAccess > xElements = getElements(_eType);
    if ( xElements.is() )
    {
        Sequence< ::rtl::OUString> aSeq = xElements->getElementNames();
        _rList.reserve(aSeq.getLength());

        const ::rtl::OUString* pBegin = aSeq.getConstArray();
        const ::rtl::OUString* pEnd   = pBegin + aSeq.getLength();
        _rList.assign(pBegin,pEnd);
    }
}
// -----------------------------------------------------------------------------
void OApplicationController::impl_initialize( const Sequence< Any >& aArguments )
{
    sal_Bool bInteractive = sal_False;
    const Any* pIter = aArguments.getConstArray();
    const Any* pEnd   = pIter + aArguments.getLength();
    PropertyValue aProp;
    for(;pIter != pEnd;++pIter)
    {
        if ( (*pIter >>= aProp) && aProp.Name == URL_INTERACTIVE )
        {
            aProp.Value >>= bInteractive;
            break;
        }
    }

    sal_Bool bNew = sal_False;
    Reference<XModel> xModel(m_xDataSource,UNO_QUERY);
    if ( bInteractive && xModel.is() && !xModel->getURL().getLength() && getView() )
    {
        WinBits nBits(WB_STDMODAL|WB_SAVEAS);
        ::sfx2::FileDialogHelper aFileDlg( ::sfx2::FILESAVE_AUTOEXTENSION,static_cast<sal_uInt32>(nBits) ,getView());
        aFileDlg.SetDisplayDirectory( SvtPathOptions().GetWorkPath() );

        const SfxFilter* pFilter = getStandardFilter();
        if ( pFilter )
        {
            aFileDlg.AddFilter(pFilter->GetFilterName(),pFilter->GetDefaultExtension());
            aFileDlg.SetCurrentFilter(pFilter->GetFilterName());
        }

        Reference< XNameAccess > xDatabaseContext(getORB()->createInstance(SERVICE_SDB_DATABASECONTEXT), UNO_QUERY);
        if ( xDatabaseContext.is() )
        {
            if ( aFileDlg.Execute() == ERRCODE_NONE )
            {
                INetURLObject aURL( aFileDlg.GetPath() );
                if( aURL.GetProtocol() != INET_PROT_NOT_VALID )
                {
                    ::rtl::OUString sFileName = aURL.GetMainURL( INetURLObject::NO_DECODE );
                    if ( ::utl::UCBContentHelper::IsDocument(sFileName) )
                        ::utl::UCBContentHelper::Kill(sFileName);
                    try
                    {
                        Sequence< PropertyValue > aArgs;
                        xModel->attachResource(sFileName,aArgs);
                        attachModel(xModel);
                        Reference<XStorable> xStr(xModel,UNO_QUERY);
                        if ( xStr.is() )
                            xStr->store();

                        Execute(SID_DB_APP_DSCONNECTION_TYPE);
                        getContainer()->disableControls(isDataSourceReadOnly());
                    }
                    catch(Exception)
                    {
                    }
                }
            }
            else
                throw Exception();
        }

    }

    Execute(SID_DB_APP_VIEW_FORMS);

    InvalidateAll();
}
// -----------------------------------------------------------------------------
void OApplicationController::getSelectionElementNames(::std::vector< ::rtl::OUString>& _rNames)
{
    ::vos::OGuard aSolarGuard( Application::GetSolarMutex() );
    ::osl::MutexGuard aGuard(m_aMutex);

    OSL_ENSURE(getContainer(),"View isn't valid! -> GPF");

    Reference< XDatabaseMetaData> xMetaData;
    if ( getContainer()->getElementType() == E_TABLE )
    {
        Reference<XConnection> xConnection;
        ensureConnection(xConnection,sal_False);
        if ( xConnection.is() )
            xMetaData = xConnection->getMetaData();
    }

    getContainer()->getSelectionElementNames(_rNames,xMetaData);
}
// -----------------------------------------------------------------------------
::std::auto_ptr<OLinkedDocumentsAccess> OApplicationController::getDocumentsAccess(ElementType _eType)
{
    OSL_ENSURE(_eType == E_FORM || _eType == E_REPORT || _eType == E_QUERY,"Illegal type for call!");
    Reference< XNameAccess > xNameAccess;
    switch( _eType )
    {
        case E_FORM:
        {
            Reference< XFormDocumentsSupplier > xSupp(m_xDataSource,UNO_QUERY);
            if ( xSupp.is() )
                xNameAccess = xSupp->getFormDocuments();
            break;
        }
        case E_REPORT:
        {
            Reference< XReportDocumentsSupplier > xSupp(m_xDataSource,UNO_QUERY);
            if ( xSupp.is() )
                xNameAccess = xSupp->getReportDocuments();
            break;
        }
        case E_QUERY:
        {
            Reference< XQueryDefinitionsSupplier > xSupp(m_xDataSource,UNO_QUERY);
            if ( xSupp.is() )
                xNameAccess = xSupp->getQueryDefinitions();
            break;
        }
    }

    Reference<XConnection> xConnection;
    ensureConnection(xConnection);
    OSL_ENSURE(xNameAccess.is(),"Data source doesn't return a name access -> GPF");
    ::std::auto_ptr<OLinkedDocumentsAccess> aHelper( new OLinkedDocumentsAccess(getView(), getORB(), xNameAccess,xConnection));
    return aHelper;
}
// -----------------------------------------------------------------------------
TransferableHelper* OApplicationController::copyObject()
{
    try
    {
        ::vos::OGuard aSolarGuard( Application::GetSolarMutex() );
        ::osl::MutexGuard aGuard(m_aMutex);

        ElementType eType = getContainer()->getElementType();
        TransferableHelper* pData = NULL;
        switch( eType )
        {
            case E_TABLE:
            case E_QUERY:
            {
                Reference<XConnection> xConnection;
                ensureConnection(xConnection,sal_False);
                Reference< XDatabaseMetaData> xMetaData;
                if ( xConnection.is() )
                    xMetaData = xConnection->getMetaData();

                ::rtl::OUString sName = getContainer()->getQualifiedName(NULL,xMetaData);
                OSL_ENSURE(sName.getLength(),"NO name given!");
                ::rtl::OUString sDataSource = getDatabaseName();

                if ( eType == E_TABLE )
                {
                    pData = new ODataClipboard(sDataSource, CommandType::TABLE, sName, xConnection, getNumberFormatter(xConnection), getORB());
                }
                else
                {
                    pData = new ODataClipboard(sDataSource, CommandType::QUERY, sName, getNumberFormatter(xConnection), getORB());
                }

            }
                break;
            case E_FORM:
            case E_REPORT:
            {
                ::std::vector< ::rtl::OUString> aList;
                getSelectionElementNames(aList);
                Reference< XHierarchicalNameAccess > xElements(getElements(eType),UNO_QUERY);
                if ( xElements.is() )
                {
                    Reference< XContent> xContent(xElements->getByHierarchicalName(*aList.begin()),UNO_QUERY);
                    pData = new OComponentTransferable(m_sDatabaseName,xContent);
                }
            }
                break;
        }

        // the owner ship goes to ODataClipboards
        return pData;
    }
    catch(SQLException& e)
    {
        showError(SQLExceptionInfo(e));
    }
    catch(Exception&)
    {
        DBG_ERROR("OApplicationController::copyObject: caught a generic exception!");
    }
    return NULL;
}
// -----------------------------------------------------------------------------
sal_Bool OApplicationController::paste( ElementType _eType,const ::svx::ODataAccessDescriptor& _rPasteData,const String& _sParentFolder ,sal_Bool _bMove)
{
    try
    {
        ::rtl::OUString sDataSourceName = _rPasteData.getDataSource();
        if ( _eType == E_QUERY )
        {
            sal_Int32 nCommandType = CommandType::TABLE;
            if ( _rPasteData.has(daCommandType) )
                _rPasteData[daCommandType]      >>= nCommandType;

            if ( CommandType::QUERY == nCommandType || CommandType::COMMAND == nCommandType )
            {
                // read all nescessary data

                ::rtl::OUString sCommand;
                sal_Bool        bEscapeProcessing = sal_True;

                _rPasteData[daCommand] >>= sCommand;
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
                    DBG_ERROR("OApplicationController::pasteQuery: invalid descriptor!");
                    return sal_False;
                }

                // three properties we can set only if we have a query object as source
                ::rtl::OUString sUpdateTableName;
                ::rtl::OUString sUpdateSchemaName;
                ::rtl::OUString sUpdateCatalogName;

                // the target object name (as we'll suggest it to the user)
                String sTargetName;
                Reference< XNameAccess > xQueries;
                try
                {
                    // the query container
                    xQueries.set(getQueryDefintions(),UNO_QUERY);
                    String aQueryDefaultName = String(ModuleRes(STR_QRY_TITLE));
                    aQueryDefaultName = aQueryDefaultName.GetToken(0,' ');
                    sTargetName = ::dbtools::createUniqueName(xQueries,aQueryDefaultName);
                }
                catch(Exception)
                {
                    OSL_ENSURE(0,"could not create query default name!");
                }

                if (CommandType::QUERY == nCommandType)
                {
                    // need to extract the statement and the escape processing flag from the query object
                    sal_Bool bSuccess = sal_False;
                    try
                    {
                        // the concrete query
                        Reference< XPropertySet > xQuery;
                        if ( xQueries.is() && xQueries->hasByName(sCommand) )
                        {
                            xQuery.set(xQueries->getByName(sCommand),UNO_QUERY);

                            if (xQuery.is())
                            {
                                // extract all the properties we're interested in
                                xQuery->getPropertyValue(PROPERTY_COMMAND) >>= sCommand;
                                xQuery->getPropertyValue(PROPERTY_USE_ESCAPE_PROCESSING) >>= bEscapeProcessing;

                                xQuery->getPropertyValue(PROPERTY_UPDATE_TABLENAME) >>= sUpdateTableName;
                                xQuery->getPropertyValue(PROPERTY_UPDATE_SCHEMANAME) >>= sUpdateSchemaName;
                                xQuery->getPropertyValue(PROPERTY_UPDATE_CATALOGNAME) >>= sUpdateCatalogName;
                                bSuccess = sal_True;
                            }
                        }
                    }
                    catch(SQLException&) { throw; } // caught and handled by the outer catch
                    catch(Exception&) { }

                    if (!bSuccess)
                    {
                        DBG_ERROR("OApplicationController::pasteQuery: could not extract the source query object!");
                        // TODO: maybe this is worth an error message to be displayed to the user ....
                        return sal_False;
                    }
                }


                Reference< XNameContainer > xDestQueries(getQueryDefintions(), UNO_QUERY);
                Reference< XSingleServiceFactory > xQueryFactory(xDestQueries, UNO_QUERY);
                if (!xQueryFactory.is())
                {
                    DBG_ERROR("OApplicationController::pasteQuery: invalid destination query container!");
                    return sal_False;
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
                if ( RET_OK != aAskForName.Execute() )
                    // cancelled by the user
                    return sal_False;

                sTargetName = aAskForName.getName();

                // create a new object
                Reference< XPropertySet > xNewQuery(xQueryFactory->createInstance(), UNO_QUERY);
                DBG_ASSERT(xNewQuery.is(), "OApplicationController::pasteQuery: invalid object created by factory!");
                if (xNewQuery.is())
                {
                    // initialize
                    xNewQuery->setPropertyValue( PROPERTY_COMMAND, makeAny(sCommand) );
                    xNewQuery->setPropertyValue( PROPERTY_USE_ESCAPE_PROCESSING, makeAny(bEscapeProcessing) );
                    xNewQuery->setPropertyValue( PROPERTY_UPDATE_TABLENAME, makeAny(sUpdateTableName) );
                    xNewQuery->setPropertyValue( PROPERTY_UPDATE_SCHEMANAME, makeAny(sUpdateSchemaName) );
                    xNewQuery->setPropertyValue( PROPERTY_UPDATE_CATALOGNAME, makeAny(sUpdateCatalogName) );
                    // insert
                    xDestQueries->insertByName( sTargetName, makeAny(xNewQuery) );
                }
            }
            else
                OSL_TRACE("There should be a sequence in it!");
            return sal_True;
        }
        else // forms or reports
        {
            Reference<XContent> xContent;
            _rPasteData[daComponent] >>= xContent;
            return insertHierachyElement(_eType,_sParentFolder,Reference<XNameAccess>(xContent,UNO_QUERY).is(),xContent,_bMove);
        }
    }
    catch(SQLContext& e) { showError(SQLExceptionInfo(e)); }
    catch(SQLWarning& e) { showError(SQLExceptionInfo(e)); }
    catch(SQLException& e) { showError(SQLExceptionInfo(e)); }
    catch(Exception& )
    {
        DBG_ERROR("OApplicationController::paste: caught a strange exception!");
    }
    return sal_False;
}
// -----------------------------------------------------------------------------
void OApplicationController::pasteTable( SotFormatStringId _nFormatId,const TransferableDataHelper& _rTransData )
{
    if ( _nFormatId == SOT_FORMATSTR_ID_DBACCESS_TABLE || _nFormatId == SOT_FORMATSTR_ID_DBACCESS_QUERY )
    {
        if ( ODataAccessObjectTransferable::canExtractObjectDescriptor(_rTransData.GetDataFlavorExVector()) )
            pasteTable( ODataAccessObjectTransferable::extractObjectDescriptor(_rTransData) );
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
            if ( !copyTagTable(aTrans,sal_False) )
                showError(SQLException(String(ModuleRes(STR_NO_TABLE_FORMAT_INSIDE)),*this,::rtl::OUString::createFromAscii("S1000") ,0,Any()));
        }
        catch(SQLContext& e) { showError(SQLExceptionInfo(e)); }
        catch(SQLWarning& e) { showError(SQLExceptionInfo(e)); }
        catch(SQLException& e) { showError(SQLExceptionInfo(e)); }
        catch(Exception& )
        {
            OSL_ENSURE(sal_False, "OApplicationController::pasteTable: caught a generic exception!");
        }
    }
    else
        showError(SQLException(String(ModuleRes(STR_NO_TABLE_FORMAT_INSIDE)),*this,::rtl::OUString::createFromAscii("S1000") ,0,Any()));
}
// -----------------------------------------------------------------------------
void OApplicationController::pasteTable( const TransferableDataHelper& _rTransData )
{
    if ( _rTransData.HasFormat(SOT_FORMATSTR_ID_DBACCESS_TABLE) || _rTransData.HasFormat(SOT_FORMATSTR_ID_DBACCESS_QUERY) )
        pasteTable( SOT_FORMATSTR_ID_DBACCESS_TABLE,_rTransData);
    else if ( _rTransData.HasFormat(SOT_FORMATSTR_ID_HTML) )
        pasteTable( SOT_FORMATSTR_ID_HTML,_rTransData);
    else if ( _rTransData.HasFormat(SOT_FORMATSTR_ID_HTML_SIMPLE) )
        pasteTable( SOT_FORMATSTR_ID_HTML_SIMPLE,_rTransData);
    else if ( _rTransData.HasFormat(SOT_FORMAT_RTF) )
        pasteTable( SOT_FORMAT_RTF,_rTransData);
}
// -----------------------------------------------------------------------------
void OApplicationController::pasteTable( const ::svx::ODataAccessDescriptor& _rPasteData )
{
    Reference<XConnection> xSrcConnection;
    Reference<XResultSet>   xSrcRs;         // the source resultset may be empty
    Sequence< Any > aSelection;
    sal_Bool bBookmarkSelection;
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
                ,sSrcDataSourceName);
}
// -----------------------------------------------------------------------------
void OApplicationController::insertTable(sal_Int32 _nCommandType
                                        ,const Reference<XConnection>& _xSrcConnection
                                        ,const Reference<XResultSet>&   _xSrcRs         // the source resultset may be empty
                                        ,const Sequence< Any >& _aSelection
                                        ,sal_Bool _bBookmarkSelection
                                        ,const ::rtl::OUString& _sCommand
                                        ,const ::rtl::OUString& _sSrcDataSourceName)
{
    try
    {
        if ( CommandType::QUERY == _nCommandType || CommandType::TABLE == _nCommandType )
        {
            ::vos::OGuard aSolarGuard( Application::GetSolarMutex() );
            ::osl::MutexGuard aGuard(m_aMutex);

            ::rtl::OUString sDataSourceName = getDatabaseName();

            // first get the dest connection
            Reference<XConnection> xDestConnection,xSrcConnection;   // supports the service sdb::connection
            ensureConnection(xDestConnection);
            if ( !xDestConnection.is() )
                return;

            xSrcConnection = _xSrcConnection;
            Reference<XResultSet> xSrcRs = _xSrcRs;


            // get the source connection
            sal_Bool bDispose = sal_False;
            if ( _sSrcDataSourceName == sDataSourceName )
                xSrcConnection = xDestConnection;
            else if ( !xSrcConnection.is() )
            {
                Reference< XEventListener> xEvt(static_cast< ::cppu::OWeakObject*>(this), UNO_QUERY);
                showError(::dbaui::createConnection(m_xDataSource,getORB(),xEvt,xSrcConnection));
                bDispose = sal_True;
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

                OCopyTableWizard aWizard(getView(),
                                            xSourceObject,
                                            xSrcConnection,
                                            xDestConnection,
                                            getNumberFormatter(xDestConnection),
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
                    WaitObject aWO(getView());
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
                                    xSrcRs = createResultSet(this,
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
                                            _aSelection,
                                            _bBookmarkSelection,
                                            getView());
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
                showError(SQLException(String(ModuleRes(STR_NO_TABLE_FORMAT_INSIDE)),*this,::rtl::OUString::createFromAscii("S1000") ,0,Any()));
            }
            if ( bDispose )
                ::comphelper::disposeComponent(xSrcConnection);
        }
        else
            DBG_ERROR("OApplicationController::pasteTable: invalid call (no supported format found)!");
    }
    catch(SQLContext& e) { showError(SQLExceptionInfo(e)); }
    catch(SQLWarning& e) { showError(SQLExceptionInfo(e)); }
    catch(SQLException& e) { showError(SQLExceptionInfo(e)); }
    catch(Exception& )
    {
        OSL_ENSURE(sal_False, "OApplicationController::pasteTable: caught a generic exception!");
    }
}
// -----------------------------------------------------------------------------
Reference<XNameContainer> OApplicationController::getQueryDefintions() const
{
    Reference<XQueryDefinitionsSupplier> xSet(m_xDataSource,UNO_QUERY);
    Reference<XNameContainer> xNames;
    if ( xSet.is() )
    {
        xNames.set(xSet->getQueryDefinitions(),UNO_QUERY);
    }
    return xNames;
}
// -----------------------------------------------------------------------------
void OApplicationController::getSupportedFormats(ElementType _eType,::std::vector<SotFormatStringId>& _rFormatIds) const
{
    switch( _eType )
    {
        case E_TABLE:
            _rFormatIds.push_back(SOT_FORMATSTR_ID_DBACCESS_TABLE);
            _rFormatIds.push_back(SOT_FORMAT_RTF);
            _rFormatIds.push_back(SOT_FORMATSTR_ID_HTML);
            _rFormatIds.push_back(SOT_FORMATSTR_ID_HTML_SIMPLE);
            // run through
        case E_QUERY:
            _rFormatIds.push_back(SOT_FORMATSTR_ID_DBACCESS_QUERY);
            break;
    }
}
// -----------------------------------------------------------------------------
sal_Bool OApplicationController::isTableFormat()  const
{
    sal_Bool bTableFormat   =   getViewClipboard().HasFormat(SOT_FORMATSTR_ID_DBACCESS_TABLE)
                ||  getViewClipboard().HasFormat(SOT_FORMATSTR_ID_DBACCESS_QUERY)
                ||  getViewClipboard().HasFormat(SOT_FORMAT_RTF)
                ||  getViewClipboard().HasFormat(SOT_FORMATSTR_ID_HTML)
                ||  getViewClipboard().HasFormat(SOT_FORMATSTR_ID_HTML_SIMPLE);

    return bTableFormat;
}
// -----------------------------------------------------------------------------
sal_Bool OApplicationController::copyTagTable(DropDescriptor& _rDesc, sal_Bool _bCheck)
{
    // first get the dest connection
    ::osl::MutexGuard aGuard(m_aMutex);

    Reference<XConnection> xDestConnection;  // supports the service sdb::connection
    ensureConnection( xDestConnection);
    if ( !xDestConnection.is() )
        return sal_False;

    Reference<XEventListener> xEvt;
    ODatabaseImportExport* pImport = NULL;
    if ( _rDesc.bHtml )
        pImport = new OHTMLImportExport(xDestConnection,getNumberFormatter(xDestConnection),getORB());
    else
        pImport = new ORTFImportExport(xDestConnection,getNumberFormatter(xDestConnection),getORB());

    xEvt = pImport;
    SvStream* pStream = (SvStream*)(SotStorageStream*)_rDesc.aHtmlRtfStorage;
    if ( _bCheck )
        pImport->enableCheckOnly();

    pImport->setStream(pStream);
    return pImport->Read();
}
// -----------------------------------------------------------------------------
IMPL_LINK( OApplicationController, OnAsyncDrop, void*, NOTINTERESTEDIN )
{
    m_nAsyncDrop = 0;
    ::vos::OGuard aSolarGuard( Application::GetSolarMutex() );
    ::osl::MutexGuard aGuard(m_aMutex);


    if ( m_aAsyncDrop.nType == E_TABLE )
    {
        if ( m_aAsyncDrop.aHtmlRtfStorage.Is() )
        {
            copyTagTable(m_aAsyncDrop,sal_False);
            m_aAsyncDrop.aHtmlRtfStorage = NULL;
            // we now have to delete the temp file created in executeDrop
            INetURLObject aURL;
            aURL.SetURL(m_aAsyncDrop.aUrl);
            ::utl::UCBContentHelper::Kill(aURL.GetMainURL(INetURLObject::NO_DECODE));
        }
        else if ( !m_aAsyncDrop.bError )
            pasteTable(m_aAsyncDrop.aDroppedData);
        else
            showError(SQLException(String(ModuleRes(STR_NO_TABLE_FORMAT_INSIDE)),*this,::rtl::OUString::createFromAscii("S1000") ,0,Any()));
    }
    else
    {
        if ( paste(m_aAsyncDrop.nType,m_aAsyncDrop.aDroppedData,m_aAsyncDrop.aUrl,m_aAsyncDrop.nAction == DND_ACTION_MOVE)
            && m_aAsyncDrop.nAction == DND_ACTION_MOVE )
        {
            Reference<XContent> xContent;
            m_aAsyncDrop.aDroppedData[daComponent] >>= xContent;
            ::std::vector< ::rtl::OUString> aList;
            sal_Int32 nIndex = 0;
            ::rtl::OUString sName = xContent->getIdentifier()->getContentIdentifier();
            ::rtl::OUString sErase = sName.getToken(0,'/',nIndex); // we don't want to have the "private:forms" part
            if ( nIndex != -1 )
            {
                aList.push_back(sName.copy(sErase.getLength() + 1));
                Reference<XNameContainer> xNames(getElements(m_aAsyncDrop.nType), UNO_QUERY);
                dbaui::deleteObjects(getView(),getORB(),xNames,aList,0,sal_False);
            }
        }
    }

    m_aAsyncDrop.aDroppedData.clear();

    return 0L;
}
//........................................................................
}   // namespace dbaui
//........................................................................


