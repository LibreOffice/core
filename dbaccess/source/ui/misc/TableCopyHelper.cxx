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

#include "TableCopyHelper.hxx"
#include "dbustrings.hrc"
#include "sqlmessage.hxx"
#include <vcl/msgbox.hxx>
#include "WCopyTable.hxx"
#include <dbaccess/genericcontroller.hxx>
#include "WCPage.hxx"
#include <com/sun/star/task/XInteractionHandler.hpp>
#include <com/sun/star/sdb/XSingleSelectQueryComposer.hpp>
#include <com/sun/star/sdb/application/CopyTableOperation.hpp>
#include <com/sun/star/sdb/application/CopyTableWizard.hpp>
#include <com/sun/star/sdb/DataAccessDescriptorFactory.hpp>

#include "RtfReader.hxx"
#include "HtmlReader.hxx"
#include "TokenWriter.hxx"
#include "UITools.hxx"
#include <dbaccess/dataview.hxx>
#include "dbu_resource.hrc"
#include <unotools/ucbhelper.hxx>
#include <tools/urlobj.hxx>
#include <tools/diagnose_ex.h>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <com/sun/star/sdbcx/XViewsSupplier.hpp>
#include <com/sun/star/sdb/XQueryDefinitionsSupplier.hpp>
#include <com/sun/star/sdb/SQLContext.hpp>
#include <com/sun/star/sdbc/XParameters.hpp>
#include <com/sun/star/sdbc/XResultSetMetaDataSupplier.hpp>
#include <com/sun/star/sdb/XQueriesSupplier.hpp>
#include <com/sun/star/sdbc/XColumnLocate.hpp>
#include <com/sun/star/sdbcx/XRowLocate.hpp>
#include <vcl/waitobj.hxx>
#include <com/sun/star/sdb/XSQLQueryComposerFactory.hpp>
#include <unotools/tempfile.hxx>
#include <cppuhelper/exc_hlp.hxx>

#include "dbexchange.hxx"
namespace dbaui
{
using namespace ::dbtools;
using namespace ::svx;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::task;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdb::application;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::ucb;

OTableCopyHelper::OTableCopyHelper(OGenericUnoController* _pControler)
    :m_pController(_pControler)
{
}

void OTableCopyHelper::insertTable( const OUString& i_rSourceDataSource, const Reference<XConnection>& i_rSourceConnection,
        const OUString& i_rCommand, const sal_Int32 i_nCommandType,
        const Reference< XResultSet >& i_rSourceRows, const Sequence< Any >& i_rSelection, const sal_Bool i_bBookmarkSelection,
        const OUString& i_rDestDataSource, const Reference<XConnection>& i_rDestConnection)
{
    if ( CommandType::QUERY != i_nCommandType && CommandType::TABLE != i_nCommandType )
    {
        SAL_WARN("dbaccess.ui", "OTableCopyHelper::insertTable: invalid call (no supported format found)!" );
        return;
    }

    try
    {
        Reference<XConnection> xSrcConnection( i_rSourceConnection );
        if ( i_rSourceDataSource == i_rDestDataSource )
            xSrcConnection = i_rDestConnection;

        if ( !xSrcConnection.is() || !i_rDestConnection.is() )
        {
            SAL_WARN("dbaccess.ui", "OTableCopyHelper::insertTable: no connection/s!" );
            return;
        }

        Reference<XComponentContext> aContext( m_pController->getORB() );

        Reference< XDataAccessDescriptorFactory > xFactory( DataAccessDescriptorFactory::get( aContext ) );

        Reference< XPropertySet > xSource( xFactory->createDataAccessDescriptor(), UNO_SET_THROW );
        xSource->setPropertyValue( PROPERTY_COMMAND_TYPE, makeAny( i_nCommandType ) );
        xSource->setPropertyValue( PROPERTY_COMMAND, makeAny( i_rCommand ) );
        xSource->setPropertyValue( PROPERTY_ACTIVE_CONNECTION, makeAny( xSrcConnection ) );
        xSource->setPropertyValue( PROPERTY_RESULT_SET, makeAny( i_rSourceRows ) );
        xSource->setPropertyValue( PROPERTY_SELECTION, makeAny( i_rSelection ) );
        xSource->setPropertyValue( PROPERTY_BOOKMARK_SELECTION, makeAny( i_bBookmarkSelection ) );

        Reference< XPropertySet > xDest( xFactory->createDataAccessDescriptor(), UNO_SET_THROW );
        xDest->setPropertyValue( PROPERTY_ACTIVE_CONNECTION, makeAny( i_rDestConnection ) );

        Reference< XCopyTableWizard > xWizard( CopyTableWizard::create( aContext, xSource, xDest ), UNO_SET_THROW );

        OUString sTableNameForAppend( GetTableNameForAppend() );
        xWizard->setDestinationTableName( GetTableNameForAppend() );

        bool bAppendToExisting = !sTableNameForAppend.isEmpty();
        xWizard->setOperation( bAppendToExisting ? CopyTableOperation::AppendData : CopyTableOperation::CopyDefinitionAndData );

        xWizard->execute();
    }
    catch( const SQLException& )
    {
        m_pController->showError( SQLExceptionInfo( ::cppu::getCaughtException() ) );
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}

void OTableCopyHelper::pasteTable( const ::svx::ODataAccessDescriptor& _rPasteData, const OUString& i_rDestDataSourceName,
                                  const SharedConnection& i_rDestConnection )
{
    OUString sSrcDataSourceName = _rPasteData.getDataSource();

    OUString sCommand;
    _rPasteData[ daCommand ] >>= sCommand;

    Reference<XConnection> xSrcConnection;
    if ( _rPasteData.has(daConnection) )
    {
        OSL_VERIFY( _rPasteData[daConnection] >>= xSrcConnection );
    }

    Reference< XResultSet > xResultSet;
    if ( _rPasteData.has(daCursor) )
    {
        OSL_VERIFY( _rPasteData[ daCursor ] >>= xResultSet );
    }

    Sequence< Any > aSelection;
    if ( _rPasteData.has( daSelection ) )
    {
        OSL_VERIFY( _rPasteData[ daSelection ] >>= aSelection );
        OSL_ENSURE( _rPasteData.has( daBookmarkSelection ), "OTableCopyHelper::pasteTable: you should specify BookmarkSelection, too, to be on the safe side!" );
    }

    sal_Bool bBookmarkSelection( sal_True );
    if ( _rPasteData.has( daBookmarkSelection ) )
    {
        OSL_VERIFY( _rPasteData[ daBookmarkSelection ] >>= bBookmarkSelection );
    }
    OSL_ENSURE( bBookmarkSelection, "OTableCopyHelper::pasteTable: working with selection-indicies (instead of bookmarks) is error-prone, and thus deprecated!" );

    sal_Int32 nCommandType = CommandType::COMMAND;
    if ( _rPasteData.has(daCommandType) )
        _rPasteData[daCommandType] >>= nCommandType;

    insertTable( sSrcDataSourceName, xSrcConnection, sCommand, nCommandType,
                 xResultSet, aSelection, bBookmarkSelection,
                 i_rDestDataSourceName, i_rDestConnection );
}

void OTableCopyHelper::pasteTable( SotFormatStringId _nFormatId
                                  ,const TransferableDataHelper& _rTransData
                                  ,const OUString& i_rDestDataSource
                                  ,const SharedConnection& _xConnection)
{
    if ( _nFormatId == SOT_FORMATSTR_ID_DBACCESS_TABLE || _nFormatId == SOT_FORMATSTR_ID_DBACCESS_QUERY )
    {
        if ( ODataAccessObjectTransferable::canExtractObjectDescriptor(_rTransData.GetDataFlavorExVector()) )
        {
            ::svx::ODataAccessDescriptor aPasteData = ODataAccessObjectTransferable::extractObjectDescriptor(_rTransData);
            pasteTable( aPasteData,i_rDestDataSource,_xConnection);
        }
    }
    else if ( _rTransData.HasFormat(_nFormatId) )
    {
        try
        {
            DropDescriptor aTrans;
            if ( _nFormatId != SOT_FORMAT_RTF )
                const_cast<TransferableDataHelper&>(_rTransData).GetSotStorageStream(SOT_FORMATSTR_ID_HTML ,aTrans.aHtmlRtfStorage);
            else
                const_cast<TransferableDataHelper&>(_rTransData).GetSotStorageStream(SOT_FORMAT_RTF,aTrans.aHtmlRtfStorage);

            aTrans.nType            = E_TABLE;
            aTrans.bHtml            = SOT_FORMATSTR_ID_HTML == _nFormatId;
            aTrans.sDefaultTableName = GetTableNameForAppend();
            if ( !copyTagTable(aTrans,sal_False,_xConnection) )
                m_pController->showError(SQLException(String(ModuleRes(STR_NO_TABLE_FORMAT_INSIDE)),*m_pController,OUString("S1000"),0,Any()));
        }
        catch(const SQLException&)
        {
            m_pController->showError( SQLExceptionInfo( ::cppu::getCaughtException() ) );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }
    else
        m_pController->showError(SQLException(String(ModuleRes(STR_NO_TABLE_FORMAT_INSIDE)),*m_pController,OUString("S1000"),0,Any()));
}

void OTableCopyHelper::pasteTable( const TransferableDataHelper& _rTransData
                                  ,const OUString& i_rDestDataSource
                                  ,const SharedConnection& _xConnection)
{
    if ( _rTransData.HasFormat(SOT_FORMATSTR_ID_DBACCESS_TABLE) || _rTransData.HasFormat(SOT_FORMATSTR_ID_DBACCESS_QUERY) )
        pasteTable( SOT_FORMATSTR_ID_DBACCESS_TABLE,_rTransData,i_rDestDataSource,_xConnection);
    else if ( _rTransData.HasFormat(SOT_FORMATSTR_ID_HTML) )
        pasteTable( SOT_FORMATSTR_ID_HTML,_rTransData,i_rDestDataSource,_xConnection);
    else if ( _rTransData.HasFormat(SOT_FORMAT_RTF) )
        pasteTable( SOT_FORMAT_RTF,_rTransData,i_rDestDataSource,_xConnection);
}

sal_Bool OTableCopyHelper::copyTagTable(OTableCopyHelper::DropDescriptor& _rDesc, sal_Bool _bCheck,const SharedConnection& _xConnection)
{
    Reference<XEventListener> xEvt;
    ODatabaseImportExport* pImport = NULL;
    if ( _rDesc.bHtml )
        pImport = new OHTMLImportExport(_xConnection,getNumberFormatter(_xConnection, m_pController->getORB()),m_pController->getORB());
    else
        pImport = new ORTFImportExport(_xConnection,getNumberFormatter(_xConnection, m_pController->getORB()),m_pController->getORB());

    xEvt = pImport;
    SvStream* pStream = (SvStream*)(SotStorageStream*)_rDesc.aHtmlRtfStorage;
    if ( _bCheck )
        pImport->enableCheckOnly();

    //set the selected tablename
    pImport->setSTableName(_rDesc.sDefaultTableName);

    pImport->setStream(pStream);
    return pImport->Read();
}

sal_Bool OTableCopyHelper::isTableFormat(const TransferableDataHelper& _rClipboard)  const
{
    sal_Bool bTableFormat   =   _rClipboard.HasFormat(SOT_FORMATSTR_ID_DBACCESS_TABLE)
                ||  _rClipboard.HasFormat(SOT_FORMATSTR_ID_DBACCESS_QUERY)
                ||  _rClipboard.HasFormat(SOT_FORMAT_RTF)
                ||  _rClipboard.HasFormat(SOT_FORMATSTR_ID_HTML);

    return bTableFormat;
}

sal_Bool OTableCopyHelper::copyTagTable(const TransferableDataHelper& _aDroppedData
                                        ,DropDescriptor& _rAsyncDrop
                                        ,const SharedConnection& _xConnection)
{
    sal_Bool bRet = sal_False;
    sal_Bool bHtml = _aDroppedData.HasFormat(SOT_FORMATSTR_ID_HTML);
    if ( bHtml || _aDroppedData.HasFormat(SOT_FORMAT_RTF) )
    {
        if ( bHtml )
            const_cast<TransferableDataHelper&>(_aDroppedData).GetSotStorageStream(SOT_FORMATSTR_ID_HTML ,_rAsyncDrop.aHtmlRtfStorage);
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

void OTableCopyHelper::asyncCopyTagTable(  DropDescriptor& _rDesc
                                ,const OUString& i_rDestDataSource
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
        pasteTable(_rDesc.aDroppedData,i_rDestDataSource,_xConnection);
    else
        m_pController->showError(SQLException(String(ModuleRes(STR_NO_TABLE_FORMAT_INSIDE)),*m_pController,OUString("S1000"),0,Any()));
}

}   // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
