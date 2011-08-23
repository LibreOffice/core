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

#include <cstdarg>

#include <stdio.h>

#include <com/sun/star/text/NotePrintMode.hpp>

#ifndef _COM_SUN_STAR_SDB_COMMANDTYPE_HPP_
#include <com/sun/star/sdb/CommandType.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XCOMPONENTLOADER_HPP_
#include <com/sun/star/frame/XComponentLoader.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_iXNUMBERFORMATTER_HPP_
#include <com/sun/star/util/XNumberFormatter.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XCOMPLETEDCONNECTION_HPP_
#include <com/sun/star/sdb/XCompletedConnection.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XCOMPLETEDEXECUTION_HPP_
#include <com/sun/star/sdb/XCompletedExecution.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCHILD_HPP_
#include <com/sun/star/container/XChild.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_MAILMERGEEVENT_
#include <com/sun/star/text/MailMergeEvent.hpp>
#endif
#ifndef _DBCONFIG_HXX
#include <dbconfig.hxx>
#endif
#ifndef _LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif
#ifndef _UNOTOOLS_TEMPFILE_HXX
#include <unotools/tempfile.hxx>
#endif
#ifndef SVTOOLS_URIHELPER_HXX
#include <bf_svtools/urihelper.hxx>
#endif
#ifndef _SVSTDARR_HXX
#define _SVSTDARR_STRINGSDTOR
#include <bf_svtools/svstdarr.hxx>
#endif
#ifndef _ZFORLIST_HXX //autogen
#include <bf_svtools/zforlist.hxx>
#endif
#ifndef _ZFORMAT_HXX //autogen
#include <bf_svtools/zformat.hxx>
#endif
#ifndef _SFXSTRITEM_HXX //autogen
#include <bf_svtools/stritem.hxx>
#endif
#ifndef _SFXENUMITEM_HXX //autogen
#include <bf_svtools/eitem.hxx>
#endif
#ifndef _SFX_PRINTER_HXX //autogen
#include <bf_sfx2/printer.hxx>
#endif
#ifndef _SFXDOCFILE_HXX //autogen
#include <bf_sfx2/docfile.hxx>
#endif
#ifndef _SFX_PROGRESS_HXX //autogen
#include <bf_sfx2/progress.hxx>
#endif
#ifndef _SFX_DOCFILT_HACK_HXX //autogen
#include <bf_sfx2/docfilt.hxx>
#endif
#ifndef _OFF_APP_HXX //autogen
#include <bf_offmgr/app.hxx>
#endif
#ifndef _MAILENUM_HXX //autogen
//#include <goodies/mailenum.hxx>
#endif
#include <cmdid.h>
#ifndef _SWTYPES_HXX
#include <swtypes.hxx>
#endif
#ifndef _SWMODULE_HXX
#include <swmodule.hxx>
#endif
#ifndef _DOCSH_HXX
#include <docsh.hxx>
#endif

#ifndef _HORIORNT_HXX
#include <horiornt.hxx>
#endif

#ifndef _WRTSH_HXX
#include <wrtsh.hxx>
#endif
#ifndef _FLDBAS_HXX
#include <fldbas.hxx>
#endif
#ifndef _FLDDAT_HXX
#include <flddat.hxx>
#endif
#ifndef _SWMODULE_HXX
#include <swmodule.hxx>
#endif
#ifndef _MODCFG_HXX
#include <modcfg.hxx>
#endif
#ifndef _SWPRTOPT_HXX
#include <swprtopt.hxx>
#endif
#ifndef _SHELLIO_HXX
#include <shellio.hxx>
#endif
#ifndef _DBMGR_HXX
#include <dbmgr.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _SWWAIT_HXX
#include <swwait.hxx>
#endif
#ifndef _SWUNOHELPER_HXX
#include <swunohelper.hxx>
#endif

#ifndef _GLOBALS_HRC
#include <globals.hrc>
#endif
#ifndef _STATSTR_HRC
#include <statstr.hrc>
#endif

#ifndef _SFXREQUEST_HXX
#include <bf_sfx2/request.hxx>
#endif
#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XDATASOURCE_HPP_
#include <com/sun/star/sdbc/XDataSource.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XROWSET_HPP_
#include <com/sun/star/sdbc/XRowSet.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XTABLESSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XCOLUMNSSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XQUERIESSUPPLIER_HPP_
#include <com/sun/star/sdb/XQueriesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XCOLUMN_HPP_
#include <com/sun/star/sdb/XColumn.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_DATATYPE_HPP_
#include <com/sun/star/sdbc/DataType.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XSTATEMENT_HPP_
#include <com/sun/star/sdbc/XStatement.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_RESULTSETTYPE_HPP_
#include <com/sun/star/sdbc/ResultSetType.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _COM_SUN_STAR_UTIL_XNUMBERFORMATTYPES_HPP_
#include <com/sun/star/util/XNumberFormatTypes.hpp>
#endif
#ifndef _SVX_LANGITEM_HXX
#include <bf_svx/langitem.hxx>
#endif
#ifndef _SVX_UNOMID_HXX
#include <bf_svx/unomid.hxx>
#endif
#ifndef _NUMUNO_HXX
#include <bf_svtools/numuno.hxx>
#endif

#include <unomailmerge.hxx>

#ifndef _SFXEVENT_HXX
#include <bf_sfx2/event.hxx>
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef _SVX_DATACCESSDESCRIPTOR_HXX_
#include <bf_svx/dataaccessdescriptor.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif
#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif
#ifndef _LEGACYBINFILTERMGR_HXX
#include <legacysmgr/legacy_binfilters_smgr.hxx>	//STRIP002
#endif
#include "bf_so3/staticbaseurl.hxx"
namespace binfilter {
using namespace svx;
using namespace ::com::sun::star;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::task;

#define C2S(cChar) String::CreateFromAscii(cChar)
#define C2U(cChar) ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(cChar))

#define DB_SEP_SPACE  	0
#define DB_SEP_TAB  	1
#define DB_SEP_RETURN   2
#define DB_SEP_NEWLINE  3

/*N*/ SV_IMPL_PTRARR(SwDSParamArr, SwDSParamPtr);
const sal_Char cCursor[] = "Cursor";
const sal_Char cCommand[] = "Command";
const sal_Char cCommandType[] = "CommandType";
const sal_Char cDataSourceName[] = "DataSourceName";
const sal_Char cSelection[] = "Selection";
const sal_Char cActiveConnection[] = "ActiveConnection";

/* -----------------09.12.2002 12:35-----------------
 *
 * --------------------------------------------------*/

/*N*/ class SwConnectionDisposedListener_Impl : public cppu::WeakImplHelper1
/*N*/ < ::com::sun::star::lang::XEventListener >
/*N*/ {
/*N*/     SwNewDBMgr&     rDBMgr;
/*N*/
/*N*/     virtual void SAL_CALL disposing( const EventObject& Source ) throw (RuntimeException);
/*N*/ public:
/*N*/     SwConnectionDisposedListener_Impl(SwNewDBMgr& rMgr);
/*N*/     ~SwConnectionDisposedListener_Impl();
/*N*/
/*N*/ };
// -----------------------------------------------------------------------------
/*N*/ struct SwNewDBMgr_Impl
/*N*/ {
/*N*/     SwDSParam*          pMergeData;
/*N*/     SwMailMergeDlg*     pMergeDialog;
/*N*/     Reference<XEventListener> xDisposeListener;
/*N*/
/*N*/     SwNewDBMgr_Impl(SwNewDBMgr& rDBMgr)
/*N*/        :pMergeData(0)
/*N*/        ,pMergeDialog(0)
/*N*/        ,xDisposeListener(new SwConnectionDisposedListener_Impl(rDBMgr))
/*N*/         {}
/*N*/ };

/*--------------------------------------------------------------------
    Beschreibung: CTOR
 --------------------------------------------------------------------*/

/*N*/ SwNewDBMgr::SwNewDBMgr() :
/*N*/             pImpl(new SwNewDBMgr_Impl(*this)),
/*N*/             pMergeEvtSrc(NULL),
/*N*/             bInMerge(FALSE),
/*N*/             bMergeLock(FALSE),
/*N*/             bMergeSilent(FALSE),
/*N*/ 			nMergeType(DBMGR_INSERT),
/*N*/ 			bInitDBFields(FALSE)
/*N*/ {
/*N*/ }
/* -----------------------------18.07.00 08:56--------------------------------

 ---------------------------------------------------------------------------*/
/*M*/ SwNewDBMgr::~SwNewDBMgr()
/*M*/ {
/*M*/     for(USHORT nPos = 0; nPos < aDataSourceParams.Count(); nPos++)
/*M*/     {
/*M*/         SwDSParam* pParam = aDataSourceParams[nPos];
/*M*/         if(pParam->xConnection.is())
/*M*/         {
/*M*/             try
/*M*/             {
/*M*/                 Reference<XComponent> xComp(pParam->xConnection, UNO_QUERY);
/*M*/                 if(xComp.is())
/*M*/                     xComp->dispose();
/*M*/             }
/*M*/             catch(const RuntimeException& )
/*M*/             {
/*M*/                 //may be disposed already since multiple entries may have used the same connection
/*M*/             }
/*M*/         }
/*M*/     }
/*M*/     delete pImpl;
/*M*/ }
/* -----------------------------03.07.00 17:12--------------------------------

 ---------------------------------------------------------------------------*/
/*?*/ Reference< sdbc::XConnection> SwNewDBMgr::GetConnection(const String& rDataSource,
/*?*/ 													Reference<XDataSource>& rxSource)
/*?*/ {
/*?*/ 	Reference< sdbc::XConnection> xConnection;
/*?*/ 	Reference<XNameAccess> xDBContext;
/*?*/ 	Reference< XMultiServiceFactory > xMgr( ::legacy_binfilters::getLegacyProcessServiceFactory() );
/*?*/ 	if( xMgr.is() )
/*?*/ 	{
/*?*/ 		Reference<XInterface> xInstance = xMgr->createInstance( C2U( "com.sun.star.sdb.DatabaseContext" ));
/*?*/ 		xDBContext = Reference<XNameAccess>(xInstance, UNO_QUERY) ;
/*?*/ 	}
/*?*/ 	DBG_ASSERT(xDBContext.is(), "com.sun.star.sdb.DataBaseContext: service not available");
/*?*/ 	if(xDBContext.is())
/*?*/ 	{
/*?*/ 		try
/*?*/ 		{
/*?*/ 			if(xDBContext->hasByName(rDataSource))
/*?*/ 			{
/*?*/ 				Reference<XCompletedConnection> xComplConnection;
/*?*/ 				xDBContext->getByName(rDataSource) >>= xComplConnection;
/*?*/ 				rxSource = Reference<XDataSource>(xComplConnection, UNO_QUERY);
/*?*/
/*?*/ 				Reference< XInteractionHandler > xHandler(
/*?*/ 					xMgr->createInstance( C2U( "com.sun.star.sdb.InteractionHandler" )), UNO_QUERY);
/*?*/ 				xConnection = xComplConnection->connectWithCompletion( xHandler );
/*?*/ 			}
/*?*/ 		}
/*?*/ 		catch(Exception&) {}
/*?*/ 	}
/*?*/ 	return xConnection;
/*?*/ }
/* -----------------------------06.07.00 14:28--------------------------------
    checks if a desired data source table or query is open
 ---------------------------------------------------------------------------*/
/*N*/ BOOL    SwNewDBMgr::IsDataSourceOpen(const String& rDataSource,
/*N*/             const String& rTableOrQuery, sal_Bool bMergeOnly)
/*N*/ {
/*N*/     if(pImpl->pMergeData)
/*N*/ 	{
/*N*/         return !bMergeLock &&
/*N*/                 rDataSource == (String)pImpl->pMergeData->sDataSource &&
/*N*/                     rTableOrQuery == (String)pImpl->pMergeData->sCommand &&
/*N*/                     pImpl->pMergeData->xResultSet.is();
/*N*/ 	}
/*N*/     else if(!bMergeOnly)
/*N*/     {
/*N*/         SwDBData aData;
/*N*/         aData.sDataSource = rDataSource;
/*N*/         aData.sCommand = rTableOrQuery;
/*N*/         aData.nCommandType = -1;
/*N*/         SwDSParam* pFound = FindDSData(aData, FALSE);
/*N*/         return (pFound && pFound->xResultSet.is());
/*N*/     }
/*N*/     return sal_False;
/*N*/ }
/* -----------------------------10.07.01 14:28--------------------------------

 ---------------------------------------------------------------------------*/
/*N*/ BOOL SwNewDBMgr::ToNextRecord(
/*N*/     const String& rDataSource, const String& rCommand, sal_Int32 nCommandType)
/*N*/ {
/*N*/     SwDSParam* pFound = 0;
/*N*/     BOOL bRet = TRUE;
/*N*/     if(pImpl->pMergeData &&
/*N*/         rDataSource == (String)pImpl->pMergeData->sDataSource &&
/*N*/         rCommand == (String)pImpl->pMergeData->sCommand)
/*N*/         pFound = pImpl->pMergeData;
/*N*/     else
/*N*/     {
/*N*/         SwDBData aData;
/*N*/         aData.sDataSource = rDataSource;
/*N*/         aData.sCommand = rCommand;
/*N*/         aData.nCommandType = -1;
/*N*/         pFound = FindDSData(aData, FALSE);
/*N*/     }
/*N*/     return ToNextRecord(pFound);
/*N*/ }
/* -----------------------------10.07.01 14:38--------------------------------

 ---------------------------------------------------------------------------*/
/*N*/ BOOL SwNewDBMgr::ToNextRecord(SwDSParam* pParam)
/*N*/ {
/*N*/     BOOL bRet = TRUE;
/*N*/     if(!pParam || !pParam->xResultSet.is() || pParam->bEndOfDB ||
/*N*/ 			(pParam->aSelection.getLength() && pParam->aSelection.getLength() <= pParam->nSelectionIndex))
/*N*/ 	{
/*N*/         if(pParam)
/*N*/             pParam->CheckEndOfDB();
/*N*/ 		return FALSE;
/*N*/ 	}
/*N*/ 	try
/*N*/ 	{
/*N*/         if(pParam->aSelection.getLength())
/*N*/ 		{
/*N*/ 			sal_Int32 nPos = 0;
/*N*/ 			pParam->aSelection.getConstArray()[ pParam->nSelectionIndex++ ] >>= nPos;
/*N*/             pParam->bEndOfDB = !pParam->xResultSet->absolute( nPos );
/*N*/             pParam->CheckEndOfDB();
/*N*/             bRet = !pParam->bEndOfDB;
/*N*/             if(pParam->nSelectionIndex >= pParam->aSelection.getLength())
/*N*/                 pParam->bEndOfDB = TRUE;
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/             pParam->bEndOfDB = !pParam->xResultSet->next();
/*N*/             pParam->CheckEndOfDB();
/*N*/             bRet = !pParam->bEndOfDB;
/*N*/             ++pParam->nSelectionIndex;
/*N*/ 		}
/*N*/ 	}
/*N*/     catch(Exception&)
/*N*/ 	{
/*N*/ 	}
/*N*/     return bRet;
/*N*/ }

/* -----------------------------13.07.00 10:41--------------------------------

 ---------------------------------------------------------------------------*/
/*N*/ sal_uInt32 	SwNewDBMgr::GetSelectedRecordId()
/*N*/ {
/*N*/ 	sal_uInt32 	nRet = 0;
/*N*/     DBG_ASSERT(pImpl->pMergeData && pImpl->pMergeData->xResultSet.is(), "no data source in merge");
/*N*/     if(!pImpl->pMergeData || !pImpl->pMergeData->xResultSet.is())
/*N*/ 		return FALSE;
/*N*/ 	try
/*N*/ 	{
/*N*/         nRet = pImpl->pMergeData->xResultSet->getRow();
/*N*/ 	}
/*N*/     catch(Exception& )
/*N*/ 	{
/*N*/ 	}
/*N*/ 	return nRet;
/*N*/ }
/* -----------------------------13.07.00 10:58--------------------------------

 ---------------------------------------------------------------------------*/
/*N*/ sal_Bool SwNewDBMgr::ToRecordId(sal_Int32 nSet)
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); return FALSE;//STRIP001
/*N*/ }
/* -----------------------------17.07.00 14:17--------------------------------

 ---------------------------------------------------------------------------*/
/*N*/ BOOL SwNewDBMgr::OpenDataSource(const String& rDataSource, const String& rTableOrQuery, sal_Int32 nCommandType)
/*N*/ {
/*?*/     SwDBData aData;
/*?*/     aData.sDataSource = rDataSource;
/*?*/     aData.sCommand = rTableOrQuery;
/*?*/     aData.nCommandType = nCommandType;
/*?*/
/*?*/     SwDSParam* pFound = FindDSData(aData, TRUE);
/*?*/ 	Reference< XDataSource> xSource;
/*?*/ 	if(pFound->xResultSet.is())
/*?*/ 		return TRUE;
/*?*/     SwDSParam* pParam = FindDSConnection(rDataSource, FALSE);
/*?*/     Reference< XConnection> xConnection;
/*?*/     if(pParam && pParam->xConnection.is())
/*?*/         pFound->xConnection = pParam->xConnection;
/*?*/     else
/*?*/     {
/*?*/         ::rtl::OUString sDataSource(rDataSource);
/*?*/         pFound->xConnection = RegisterConnection( sDataSource );
/*?*/     }
/*?*/ 	if(pFound->xConnection.is())
/*?*/ 	{
/*?*/ 		try
/*?*/ 		{
/*?*/ 			Reference< sdbc::XDatabaseMetaData >  xMetaData = pFound->xConnection->getMetaData();
/*?*/             try
/*?*/             {
/*?*/                 pFound->bScrollable = xMetaData
/*?*/ 						->supportsResultSetType((sal_Int32)ResultSetType::SCROLL_INSENSITIVE);
/*?*/             }
/*?*/             catch(Exception&)
/*?*/             {
/*?*/                 //#98373# DB driver may not be ODBC 3.0 compliant
/*?*/                 pFound->bScrollable = TRUE;
/*?*/             }
/*?*/ 			pFound->xStatement = pFound->xConnection->createStatement();
/*?*/             ::rtl::OUString aQuoteChar = xMetaData->getIdentifierQuoteString();
/*?*/             ::rtl::OUString sStatement(C2U("SELECT * FROM "));
/*?*/             sStatement = C2U("SELECT * FROM ");
/*?*/             sStatement += aQuoteChar;
/*?*/             sStatement += rTableOrQuery;
/*?*/             sStatement += aQuoteChar;
/*?*/             pFound->xResultSet = pFound->xStatement->executeQuery( sStatement );
/*?*/
/*?*/ 			//after executeQuery the cursor must be positioned
/*?*/             pFound->bEndOfDB = !pFound->xResultSet->next();
/*?*/             pFound->bAfterSelection = sal_False;
/*?*/             pFound->CheckEndOfDB();
/*?*/             ++pFound->nSelectionIndex;
/*?*/ 		}
/*?*/ 		catch (Exception&)
/*?*/ 		{
/*?*/ 			pFound->xResultSet = 0;
/*?*/ 			pFound->xStatement = 0;
/*?*/ 			pFound->xConnection = 0;
/*?*/ 		}
/*?*/ 	}
/*?*/ 	return pFound->xResultSet.is();
/*N*/ }
/* -----------------------------14.08.2001 10:26------------------------------

 ---------------------------------------------------------------------------*/
/*?*/ Reference< XConnection> SwNewDBMgr::RegisterConnection(::rtl::OUString& rDataSource)
/*?*/ {
/*?*/     SwDSParam* pFound = SwNewDBMgr::FindDSConnection(rDataSource, TRUE);
/*?*/     Reference< XDataSource> xSource;
/*?*/     if(!pFound->xConnection.is())
/*?*/ 	{
/*?*/         pFound->xConnection = SwNewDBMgr::GetConnection(rDataSource, xSource );
/*?*/         try
/*?*/         {
/*?*/             Reference<XComponent> xComponent(pFound->xConnection, UNO_QUERY);
/*?*/             if(xComponent.is())
/*?*/                 xComponent->addEventListener(pImpl->xDisposeListener);
/*?*/         }
/*?*/         catch(Exception&)
/*?*/         {
/*?*/         }
/*?*/ 	}
/*?*/     return pFound->xConnection;
/*?*/ }
/* -----------------------------17.07.00 15:55--------------------------------

 ---------------------------------------------------------------------------*/
/*N*/ sal_uInt32      SwNewDBMgr::GetSelectedRecordId(
/*N*/     const String& rDataSource, const String& rTableOrQuery, sal_Int32 nCommandType)
/*N*/ {
/*N*/ 	sal_uInt32 nRet = -1;
/*N*/ 	//check for merge data source first
/*N*/     if(pImpl->pMergeData && rDataSource == (String)pImpl->pMergeData->sDataSource &&
/*N*/                     rTableOrQuery == (String)pImpl->pMergeData->sCommand &&
/*N*/                     (nCommandType == -1 || nCommandType == pImpl->pMergeData->nCommandType) &&
/*N*/                     pImpl->pMergeData->xResultSet.is())
/*N*/ 		nRet = GetSelectedRecordId();
/*N*/ 	else
/*N*/ 	{
/*N*/         SwDBData aData;
/*N*/         aData.sDataSource = rDataSource;
/*N*/         aData.sCommand = rTableOrQuery;
/*N*/         aData.nCommandType = nCommandType;
/*N*/         SwDSParam* pFound = FindDSData(aData, FALSE);
/*N*/ 		if(pFound && pFound->xResultSet.is())
/*N*/ 		{
/*N*/ 			try
/*N*/ 			{	//if a selection array is set the current row at the result set may not be set yet
/*N*/ 				if(pFound->aSelection.getLength())
/*N*/ 				{
/*N*/ 					sal_Int32 nSelIndex = pFound->nSelectionIndex;
/*N*/                     if(nSelIndex >= pFound->aSelection.getLength())
/*N*/ 						nSelIndex = pFound->aSelection.getLength() -1;
/*N*/ 					pFound->aSelection.getConstArray()[nSelIndex] >>= nRet;
/*N*/
/*N*/ 				}
/*N*/ 				else
/*N*/ 					nRet = pFound->xResultSet->getRow();
/*N*/ 			}
/*N*/ 			catch(Exception&){}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return nRet;
/*N*/ }

/* -----------------------------17.07.00 14:18--------------------------------
    close all data sources - after fields were updated
 ---------------------------------------------------------------------------*/
/*N*/ void	SwNewDBMgr::CloseAll(BOOL bIncludingMerge)
/*N*/ {
/*N*/     //the only thing done here is to reset the selection index
/*N*/ 	//all connections stay open
/*N*/     for(USHORT nPos = 0; nPos < aDataSourceParams.Count(); nPos++)
/*N*/     {
/*N*/         SwDSParam* pParam = aDataSourceParams[nPos];
/*N*/         if(bIncludingMerge || pParam != pImpl->pMergeData)
/*N*/         {
/*N*/ 			pParam->nSelectionIndex = 0;
/*N*/ 			pParam->bAfterSelection = sal_False;
/*N*/             try
/*N*/             {
/*N*/                 if(!bInMerge && pParam->xResultSet.is())
/*N*/                     pParam->xResultSet->first();
/*N*/             }
/*N*/             catch(Exception& )
/*N*/             {}
/*N*/         }
/*N*/     }
/*N*/ }
/* -----------------------------17.07.00 14:54--------------------------------

 ---------------------------------------------------------------------------*/
/*N*/ SwDSParam* SwNewDBMgr::FindDSData(const SwDBData& rData, BOOL bCreate)
/*N*/ {
/*N*/ 	SwDSParam* pFound = 0;
/*N*/ 	for(USHORT nPos = aDataSourceParams.Count(); nPos; nPos--)
/*N*/ 	{
/*N*/ 		SwDSParam* pParam = aDataSourceParams[nPos - 1];
/*N*/         if(rData.sDataSource == pParam->sDataSource &&
/*N*/             rData.sCommand == pParam->sCommand &&
/*N*/             (rData.nCommandType == -1 || rData.nCommandType == pParam->nCommandType ||
/*N*/             (bCreate && pParam->nCommandType == -1)))
/*N*/ 			{
/*N*/                 //#94779# calls from the calculator may add a connection with an invalid commandtype
/*N*/                 //later added "real" data base connections have to re-use the already available
/*N*/                 //DSData and set the correct CommandType
/*N*/                 if(bCreate && pParam->nCommandType == -1)
/*N*/                     pParam->nCommandType = rData.nCommandType;
/*N*/ 				pFound = pParam;
/*N*/ 				break;
/*N*/ 			}
/*N*/ 	}
/*N*/ 	if(bCreate)
/*N*/ 	{
/*N*/ 		if(!pFound)
/*N*/ 		{
/*N*/             pFound = new SwDSParam(rData);
/*N*/ 			aDataSourceParams.Insert(pFound, aDataSourceParams.Count());
/*N*/             try
/*N*/             {
/*N*/                 Reference<XComponent> xComponent(pFound->xConnection, UNO_QUERY);
/*N*/                 if(xComponent.is())
/*N*/                     xComponent->addEventListener(pImpl->xDisposeListener);
/*N*/             }
/*N*/             catch(Exception&)
/*N*/             {
/*N*/             }
/*N*/         }
/*N*/ 	}
/*N*/ 	return pFound;
/*N*/ }
/* -----------------------------14.08.2001 10:27------------------------------

 ---------------------------------------------------------------------------*/
/*N*/ SwDSParam*  SwNewDBMgr::FindDSConnection(const ::rtl::OUString& rDataSource, BOOL bCreate)
/*N*/ {
/*N*/ 	SwDSParam* pFound = 0;
/*N*/ 	for(USHORT nPos = 0; nPos < aDataSourceParams.Count(); nPos++)
/*N*/ 	{
/*N*/ 		SwDSParam* pParam = aDataSourceParams[nPos];
/*N*/         if(rDataSource == pParam->sDataSource)
/*N*/         {
/*N*/             pFound = pParam;
/*N*/             break;
/*N*/         }
/*N*/ 	}
/*N*/ 	if(bCreate && !pFound)
/*N*/ 	{
/*?*/         SwDBData aData;
/*?*/         aData.sDataSource = rDataSource;
/*?*/         pFound = new SwDSParam(aData);
/*?*/ 		aDataSourceParams.Insert(pFound, aDataSourceParams.Count());
/*?*/         try
/*?*/         {
/*?*/             Reference<XComponent> xComponent(pFound->xConnection, UNO_QUERY);
/*?*/             if(xComponent.is())
/*?*/                 xComponent->addEventListener(pImpl->xDisposeListener);
/*?*/         }
/*?*/         catch(Exception&)
/*?*/         {
/*?*/         }
/*N*/     }
/*N*/ 	return pFound;
/*N*/ }

/* -----------------------------17.07.00 14:31--------------------------------
    rDBName: <Source> + DB_DELIM + <Table>; + <Statement>
 ---------------------------------------------------------------------------*/
/*M*/ void    SwNewDBMgr::AddDSData(const SwDBData& rData, long nSelStart, long nSelEnd)
/*M*/ {
/*M*/     SwDSParam* pFound = FindDSData(rData, TRUE);
/*M*/     if(nSelStart > 0)
/*M*/ 	{
/*M*/         if(nSelEnd < nSelStart)
/*M*/ 		{
/*M*/ 			sal_uInt32 nZw = nSelEnd;
/*M*/ 			nSelEnd = nSelStart;
/*M*/ 			nSelStart = nZw;
/*M*/ 		}
/*M*/
/*M*/         pFound->aSelection.realloc(nSelEnd - nSelStart + 1);
/*M*/         Any* pSelection = pFound->aSelection.getArray();
/*M*/         for (long i = nSelStart; i <= nSelEnd; i++, ++pSelection)
/*M*/             *pSelection <<= i;
/*M*/ 	}
/*M*/     else
/*M*/         pFound->aSelection.realloc(0);
/*M*/ }
/* -----------------------------17.07.00 14:31--------------------------------

 ---------------------------------------------------------------------------*/
/*M*/ void    SwNewDBMgr::GetDSSelection(const SwDBData& rData, long& rSelStart, long& rSelEnd)
/*M*/ {
/*M*/     SwDSParam* pFound = FindDSData(rData, FALSE);
/*M*/     if(!pFound || !pFound->aSelection.getLength())
/*M*/ 		rSelStart = -1L;
/*M*/ 	else
/*M*/ 	{
/*M*/         pFound->aSelection.getConstArray()[0] >>= rSelStart;
/*M*/         pFound->aSelection.getConstArray()[pFound->aSelection.getLength() - 1] >>= rSelEnd;
/*M*/     }
/*M*/ }
/* -----------------------------17.07.00 14:34--------------------------------

 ---------------------------------------------------------------------------*/
/*N*/ const SwDBData&	SwNewDBMgr::GetAddressDBName()
/*N*/ {
/*N*/ 	return SW_MOD()->GetDBConfig()->GetAddressSource();
/*N*/ }
/* -----------------------------30.08.2001 12:00------------------------------

 ---------------------------------------------------------------------------*/
SwDbtoolsClient* SwNewDBMgr::pDbtoolsClient = NULL;

/* -----------------09.12.2002 12:38-----------------
 *
 * --------------------------------------------------*/
/*N*/ SwConnectionDisposedListener_Impl::SwConnectionDisposedListener_Impl(SwNewDBMgr& rMgr) :
/*N*/     rDBMgr(rMgr)
/*N*/ {};
/* -----------------09.12.2002 12:39-----------------
 *
 * --------------------------------------------------*/
/*N*/ SwConnectionDisposedListener_Impl::~SwConnectionDisposedListener_Impl()
/*N*/ {};
/* -----------------09.12.2002 12:39-----------------
 *
 * --------------------------------------------------*/
/*N*/ void SwConnectionDisposedListener_Impl::disposing( const EventObject& rSource )
/*N*/         throw (RuntimeException)
/*N*/ {
/*N*/     ::vos::OGuard aGuard(Application::GetSolarMutex());
/*N*/     Reference<XConnection> xSource(rSource.Source, UNO_QUERY);
/*N*/     for(USHORT nPos = rDBMgr.aDataSourceParams.Count(); nPos; nPos--)
/*N*/     {
/*N*/         SwDSParam* pParam = rDBMgr.aDataSourceParams[nPos - 1];
/*N*/         if(pParam->xConnection.is() &&
/*N*/                 (xSource == pParam->xConnection))
/*N*/         {
/*N*/             rDBMgr.aDataSourceParams.DeleteAndDestroy(nPos - 1);
/*N*/         }
/*N*/     }
/*N*/ }

}
