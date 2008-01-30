/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: TableCopyHelper.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-30 08:50:47 $
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
#ifndef _COM_SUN_STAR_TASK_XINTERACTIONHANDLER_HPP_
#include <com/sun/star/task/XInteractionHandler.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XSINGLESELECTQUERYCOMPOSER_HPP_
#include <com/sun/star/sdb/XSingleSelectQueryComposer.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_APPLICATION_COPYTABLEOPERATION_HPP_
#include <com/sun/star/sdb/application/CopyTableOperation.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_APPLICATION_COPYTABLEWIZARD_HPP_
#include <com/sun/star/sdb/application/CopyTableWizard.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_DATAACCESSDESCRIPTORFACTORY_HPP_
#include <com/sun/star/sdb/DataAccessDescriptorFactory.hpp>
#endif

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
#ifndef TOOLS_DIAGNOSE_EX_H
#include <tools/diagnose_ex.h>
#endif
#ifndef COMPHELPER_COMPONENTCONTEXT_HXX
#include <comphelper/componentcontext.hxx>
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
#ifndef _CPPUHELPER_EXC_HLP_HXX_
#include <cppuhelper/exc_hlp.hxx>
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

// -----------------------------------------------------------------------------
OTableCopyHelper::OTableCopyHelper(OGenericUnoController* _pControler)
    :m_pController(_pControler)
{
}

// -----------------------------------------------------------------------------
void OTableCopyHelper::insertTable(sal_Int32 _nCommandType
                                        ,const Reference<XConnection>& _xSrcConnection
                                        ,const Sequence< Any >& _aSelection
                                        ,sal_Bool _bBookmarkSelection
                                        ,const ::rtl::OUString& _sCommand
                                        ,const ::rtl::OUString& _sSrcDataSourceName
                                        ,const ::rtl::OUString& _sDestDataSourceName
                                        ,const Reference<XConnection>& _xDestConnection)
{
    if ( CommandType::QUERY != _nCommandType && CommandType::TABLE != _nCommandType )
    {
        DBG_ERROR( "OTableCopyHelper::insertTable: invalid call (no supported format found)!" );
        return;
    }

    try
    {
        Reference<XConnection> xSrcConnection( _xSrcConnection );
        if ( _sSrcDataSourceName == _sDestDataSourceName )
            xSrcConnection = _xDestConnection;

        if ( !xSrcConnection.is() || !_xDestConnection.is() )
        {
            OSL_ENSURE( false, "OTableCopyHelper::insertTable: no connection/s!" );
            return;
        }

        ::comphelper::ComponentContext aContext( m_pController->getORB() );

        Reference< XDataAccessDescriptorFactory > xFactory( DataAccessDescriptorFactory::get( aContext.getUNOContext() ) );

        Reference< XPropertySet > xSource( xFactory->createDataAccessDescriptor(), UNO_SET_THROW );
        xSource->setPropertyValue( PROPERTY_COMMAND_TYPE, makeAny( _nCommandType ) );
        xSource->setPropertyValue( PROPERTY_COMMAND, makeAny( _sCommand ) );
        xSource->setPropertyValue( PROPERTY_ACTIVE_CONNECTION, makeAny( xSrcConnection ) );
        xSource->setPropertyValue( PROPERTY_SELECTION, makeAny( _aSelection ) );
        xSource->setPropertyValue( PROPERTY_BOOKMARK_SELECTION, makeAny( _bBookmarkSelection ) );

        Reference< XPropertySet > xDest( xFactory->createDataAccessDescriptor(), UNO_SET_THROW );
        xDest->setPropertyValue( PROPERTY_ACTIVE_CONNECTION, makeAny( _xDestConnection ) );

        Reference< XCopyTableWizard > xWizard( CopyTableWizard::create( aContext.getUNOContext(), xSource, xDest ), UNO_SET_THROW );

        ::rtl::OUString sTableNameForAppend( GetTableNameForAppend() );
        xWizard->setDestinationTableName( GetTableNameForAppend() );

        bool bAppendToExisting = ( sTableNameForAppend.getLength() != 0 );
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

// -----------------------------------------------------------------------------
void OTableCopyHelper::pasteTable( const ::svx::ODataAccessDescriptor& _rPasteData, const ::rtl::OUString& _sDestDataSourceName,
                                  const SharedConnection& _xDestConnection )
{
    Reference<XConnection> xSrcConnection;
    ::rtl::OUString sCommand,
        sSrcDataSourceName = _rPasteData.getDataSource();

    _rPasteData[daCommand]          >>= sCommand;
    if ( _rPasteData.has(daConnection) )
        _rPasteData[daConnection]   >>= xSrcConnection;
#if OSL_DEBUG_LEVEL > 0
    if ( _rPasteData.has(daCursor) )
    {
        Reference< XResultSet > xSrcRs;
        _rPasteData[daCursor] >>= xSrcRs;
        OSL_ENSURE( !xSrcRs.is(), "OTableCopyHelper::pasteTable: source result set not supported anymore!" );
        // There was a time where we supported passing a result set as shortcut to the source
        // object. That is, we do not need to create an own result set we already have one.
        // Since we UNOized the Copy Table Wizard (#i81658#), we removed this support, since it
        // contradicted the semantics of DataAccessDescriptor.ResultSet.
        //
        // This shouldn't be a problem, since there seems to be no client which actually
        // passed a result set here.
        // However, if there still is, we probably need to introduce an (undocumented?) property
        // at the DataAccessDescriptor, which takes this "source result set".
    }

    if ( _rPasteData.has( daSelection ) || _rPasteData.has( daBookmarkSelection ) )
    {
        OSL_ENSURE( false, "OTableCopyHelper::pasteTable: bookmark/selection not supported anymore!" );
        // similar notes here: Selection and BookmarkSelection are not supported in the UNOized
        // copy table wizard anymore (it doesn't make sense without support for a source result set),
        // and there seem to be no clients which actually use it. So, instead of implementing an
        // unused case, we dropped this here.
    }
#endif

    // paste into the tables
    sal_Int32 nCommandType = CommandType::COMMAND;
    if ( _rPasteData.has(daCommandType) )
        _rPasteData[daCommandType] >>= nCommandType;

    insertTable( nCommandType
                ,xSrcConnection
                ,Sequence< Any >()
                ,sal_False
                ,sCommand
                ,sSrcDataSourceName
                ,_sDestDataSourceName
                ,_xDestConnection);
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
            aTrans.sDefaultTableName = GetTableNameForAppend();
            if ( !copyTagTable(aTrans,sal_False,_xConnection) )
                m_pController->showError(SQLException(String(ModuleRes(STR_NO_TABLE_FORMAT_INSIDE)),*m_pController,::rtl::OUString::createFromAscii("S1000") ,0,Any()));
        }
        catch(const SQLException&)
        {
            m_pController->showError( SQLExceptionInfo( ::cppu::getCaughtException() ) );
        }
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

