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
        const Reference< XResultSet >& i_rSourceRows, const Sequence< Any >& i_rSelection, const bool i_bBookmarkSelection,
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

void OTableCopyHelper::pasteTable( const svx::ODataAccessDescriptor& _rPasteData, const OUString& i_rDestDataSourceName,
                                  const SharedConnection& i_rDestConnection )
{
    OUString sSrcDataSourceName = _rPasteData.getDataSource();

    OUString sCommand;
    _rPasteData[ DataAccessDescriptorProperty::Command ] >>= sCommand;

    Reference<XConnection> xSrcConnection;
    if ( _rPasteData.has(DataAccessDescriptorProperty::Connection) )
    {
        OSL_VERIFY( _rPasteData[DataAccessDescriptorProperty::Connection] >>= xSrcConnection );
    }

    Reference< XResultSet > xResultSet;
    if ( _rPasteData.has(DataAccessDescriptorProperty::Cursor) )
    {
        OSL_VERIFY( _rPasteData[ DataAccessDescriptorProperty::Cursor ] >>= xResultSet );
    }

    Sequence< Any > aSelection;
    if ( _rPasteData.has( DataAccessDescriptorProperty::Selection ) )
    {
        OSL_VERIFY( _rPasteData[ DataAccessDescriptorProperty::Selection ] >>= aSelection );
        OSL_ENSURE( _rPasteData.has( DataAccessDescriptorProperty::BookmarkSelection ), "OTableCopyHelper::pasteTable: you should specify BookmarkSelection, too, to be on the safe side!" );
    }

    bool bBookmarkSelection( true );
    if ( _rPasteData.has( DataAccessDescriptorProperty::BookmarkSelection ) )
    {
        OSL_VERIFY( _rPasteData[ DataAccessDescriptorProperty::BookmarkSelection ] >>= bBookmarkSelection );
    }
    OSL_ENSURE( bBookmarkSelection, "OTableCopyHelper::pasteTable: working with selection-indicies (instead of bookmarks) is error-prone, and thus deprecated!" );

    sal_Int32 nCommandType = CommandType::COMMAND;
    if ( _rPasteData.has(DataAccessDescriptorProperty::CommandType) )
        _rPasteData[DataAccessDescriptorProperty::CommandType] >>= nCommandType;

    insertTable( sSrcDataSourceName, xSrcConnection, sCommand, nCommandType,
                 xResultSet, aSelection, bBookmarkSelection,
                 i_rDestDataSourceName, i_rDestConnection );
}

void OTableCopyHelper::pasteTable( SotClipboardFormatId _nFormatId
                                  ,const TransferableDataHelper& _rTransData
                                  ,const OUString& i_rDestDataSource
                                  ,const SharedConnection& _xConnection)
{
    if ( _nFormatId == SotClipboardFormatId::DBACCESS_TABLE || _nFormatId == SotClipboardFormatId::DBACCESS_QUERY )
    {
        if ( ODataAccessObjectTransferable::canExtractObjectDescriptor(_rTransData.GetDataFlavorExVector()) )
        {
            svx::ODataAccessDescriptor aPasteData = ODataAccessObjectTransferable::extractObjectDescriptor(_rTransData);
            pasteTable( aPasteData,i_rDestDataSource,_xConnection);
        }
    }
    else if ( _rTransData.HasFormat(_nFormatId) )
    {
        try
        {
            DropDescriptor aTrans;
            bool bOk;
            if ( _nFormatId != SotClipboardFormatId::RTF )
                bOk = const_cast<TransferableDataHelper&>(_rTransData).GetSotStorageStream(SotClipboardFormatId::HTML ,aTrans.aHtmlRtfStorage);
            else
                bOk = const_cast<TransferableDataHelper&>(_rTransData).GetSotStorageStream(SotClipboardFormatId::RTF,aTrans.aHtmlRtfStorage);

            aTrans.nType            = E_TABLE;
            aTrans.bHtml            = SotClipboardFormatId::HTML == _nFormatId;
            aTrans.sDefaultTableName = GetTableNameForAppend();
            if ( !bOk || !copyTagTable(aTrans,false,_xConnection) )
                m_pController->showError(SQLException(ModuleRes(STR_NO_TABLE_FORMAT_INSIDE), *m_pController, "S1000", 0, Any()));
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
        m_pController->showError(SQLException(ModuleRes(STR_NO_TABLE_FORMAT_INSIDE), *m_pController, "S1000", 0, Any()));
}

void OTableCopyHelper::pasteTable( const TransferableDataHelper& _rTransData
                                  ,const OUString& i_rDestDataSource
                                  ,const SharedConnection& _xConnection)
{
    if ( _rTransData.HasFormat(SotClipboardFormatId::DBACCESS_TABLE) || _rTransData.HasFormat(SotClipboardFormatId::DBACCESS_QUERY) )
        pasteTable( SotClipboardFormatId::DBACCESS_TABLE,_rTransData,i_rDestDataSource,_xConnection);
    else if ( _rTransData.HasFormat(SotClipboardFormatId::HTML) )
        pasteTable( SotClipboardFormatId::HTML,_rTransData,i_rDestDataSource,_xConnection);
    else if ( _rTransData.HasFormat(SotClipboardFormatId::RTF) )
        pasteTable( SotClipboardFormatId::RTF,_rTransData,i_rDestDataSource,_xConnection);
}

bool OTableCopyHelper::copyTagTable(OTableCopyHelper::DropDescriptor& _rDesc, bool _bCheck, const SharedConnection& _xConnection)
{
    Reference<XEventListener> xEvt;
    ODatabaseImportExport* pImport = nullptr;
    if ( _rDesc.bHtml )
        pImport = new OHTMLImportExport(_xConnection,getNumberFormatter(_xConnection, m_pController->getORB()),m_pController->getORB());
    else
        pImport = new ORTFImportExport(_xConnection,getNumberFormatter(_xConnection, m_pController->getORB()),m_pController->getORB());

    xEvt = pImport;
    SvStream* pStream = static_cast<SvStream*>(_rDesc.aHtmlRtfStorage.get());
    if ( _bCheck )
        pImport->enableCheckOnly();

    //set the selected tablename
    pImport->setSTableName(_rDesc.sDefaultTableName);

    pImport->setStream(pStream);
    return pImport->Read();
}

bool OTableCopyHelper::isTableFormat(const TransferableDataHelper& _rClipboard)
{
    bool bTableFormat   =   _rClipboard.HasFormat(SotClipboardFormatId::DBACCESS_TABLE)
                ||  _rClipboard.HasFormat(SotClipboardFormatId::DBACCESS_QUERY)
                ||  _rClipboard.HasFormat(SotClipboardFormatId::RTF)
                ||  _rClipboard.HasFormat(SotClipboardFormatId::HTML);

    return bTableFormat;
}

bool OTableCopyHelper::copyTagTable(const TransferableDataHelper& _aDroppedData
                                   ,DropDescriptor& _rAsyncDrop
                                   ,const SharedConnection& _xConnection)
{
    bool bRet = false;
    bool bHtml = _aDroppedData.HasFormat(SotClipboardFormatId::HTML);
    if ( bHtml || _aDroppedData.HasFormat(SotClipboardFormatId::RTF) )
    {
        bool bOk;
        if ( bHtml )
            bOk = const_cast<TransferableDataHelper&>(_aDroppedData).GetSotStorageStream(SotClipboardFormatId::HTML ,_rAsyncDrop.aHtmlRtfStorage);
        else
            bOk = const_cast<TransferableDataHelper&>(_aDroppedData).GetSotStorageStream(SotClipboardFormatId::RTF,_rAsyncDrop.aHtmlRtfStorage);

        _rAsyncDrop.bHtml           = bHtml;
        _rAsyncDrop.bError          = !copyTagTable(_rAsyncDrop,true,_xConnection);

        bRet = ( !_rAsyncDrop.bError && bOk && _rAsyncDrop.aHtmlRtfStorage.is() );
        if ( bRet )
        {
            // now we need to copy the stream
            ::utl::TempFile aTmp;
            _rAsyncDrop.aUrl = aTmp.GetURL();
            ::tools::SvRef<SotStorageStream> aNew = new SotStorageStream( aTmp.GetFileName() );
            _rAsyncDrop.aHtmlRtfStorage->Seek(STREAM_SEEK_TO_BEGIN);
            _rAsyncDrop.aHtmlRtfStorage->CopyTo( aNew.get() );
            aNew->Commit();
            _rAsyncDrop.aHtmlRtfStorage = aNew;
        }
        else
            _rAsyncDrop.aHtmlRtfStorage = nullptr;
    }
    return bRet;
}

void OTableCopyHelper::asyncCopyTagTable(  DropDescriptor& _rDesc
                                ,const OUString& i_rDestDataSource
                                ,const SharedConnection& _xConnection)
{
    if ( _rDesc.aHtmlRtfStorage.is() )
    {
        copyTagTable(_rDesc,false,_xConnection);
        _rDesc.aHtmlRtfStorage = nullptr;
        // we now have to delete the temp file created in executeDrop
        INetURLObject aURL;
        aURL.SetURL(_rDesc.aUrl);
        ::utl::UCBContentHelper::Kill(aURL.GetMainURL(INetURLObject::DecodeMechanism::NONE));
    }
    else if ( !_rDesc.bError )
        pasteTable(_rDesc.aDroppedData,i_rDestDataSource,_xConnection);
    else
        m_pController->showError(SQLException(ModuleRes(STR_NO_TABLE_FORMAT_INSIDE), *m_pController, "S1000", 0, Any()));
}

}   // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
