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

#include "AppController.hxx"
#include <comphelper/sequence.hxx>
#include <comphelper/property.hxx>
#include <comphelper/processfactory.hxx>
#include "dbustrings.hrc"
#include <com/sun/star/sdbcx/XDataDescriptorFactory.hpp>
#include <com/sun/star/sdbcx/XAppend.hpp>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/sdb/XSingleSelectQueryComposer.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/uno/XNamingService.hpp>
#include <com/sun/star/sdbc/XDataSource.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/container/XHierarchicalNameContainer.hpp>
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdb/CommandType.hpp>
#include <com/sun/star/sdb/XBookmarksSupplier.hpp>
#include <com/sun/star/sdb/SQLContext.hpp>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <com/sun/star/sdbcx/XViewsSupplier.hpp>
#include <com/sun/star/sdb/XQueryDefinitionsSupplier.hpp>
#include <com/sun/star/sdbcx/XDrop.hpp>
#include <unotools/ucbhelper.hxx>
#include "dlgsave.hxx"
#include <comphelper/types.hxx>
#include <vcl/layout.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <connectivity/dbexception.hxx>
#include <vcl/waitobj.hxx>
#include <rtl/ustrbuf.hxx>
#include "AppView.hxx"
#include <svx/dataaccessdescriptor.hxx>
#include <svx/dbaobjectex.hxx>
#include "browserids.hxx"
#include "dbu_reghelper.hxx"
#include "dbu_app.hrc"
#include <vcl/menu.hxx>
#include <comphelper/uno3.hxx>
#include <vcl/svapp.hxx>
#include <svtools/svlbitm.hxx>
#include "listviewitems.hxx"
#include "AppDetailView.hxx"
#include "linkeddocuments.hxx"
#include <vcl/lstbox.hxx>
#include <connectivity/dbtools.hxx>
#include "sqlmessage.hxx"
#include "dbexchange.hxx"
#include "UITools.hxx"
#include <algorithm>
#include <iterator>
#include <svtools/treelistbox.hxx>
#include <com/sun/star/sdb/XReportDocumentsSupplier.hpp>
#include <com/sun/star/sdb/XFormDocumentsSupplier.hpp>
#include <unotools/pathoptions.hxx>
#include <sfx2/docfilt.hxx>
#include <svtools/fileview.hxx>
#include <tools/diagnose_ex.h>
#include <osl/diagnose.h>
#include "defaultobjectnamecheck.hxx"
#include <osl/mutex.hxx>
#include "subcomponentmanager.hxx"

namespace dbaui
{
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
using namespace ::com::sun::star::util;

void OApplicationController::deleteTables(const ::std::vector< OUString>& _rList)
{
    SharedConnection xConnection( ensureConnection() );

    Reference<XTablesSupplier> xSup(xConnection,UNO_QUERY);
    OSL_ENSURE(xSup.is(),"OApplicationController::deleteTable: no XTablesSuppier!");
    if ( xSup.is() )
    {
        Reference<XNameAccess> xTables = xSup->getTables();
        Reference<XDrop> xDrop(xTables,UNO_QUERY);
        if ( xDrop.is() )
        {
            bool bConfirm = true;
            ::std::vector< OUString>::const_iterator aEnd = _rList.end();
            for (::std::vector< OUString>::const_iterator aIter = _rList.begin(); aIter != aEnd; ++aIter)
            {
                OUString sTableName = *aIter;

                sal_Int32 nResult = RET_YES;
                if ( bConfirm )
                    nResult = ::dbaui::askForUserAction(getView(),STR_TITLE_CONFIRM_DELETION ,STR_QUERY_DELETE_TABLE,_rList.size() > 1 && (aIter+1) != _rList.end(),sTableName);

                bool bUserConfirmedDelete =
                            ( RET_YES == nResult )
                        ||  ( RET_ALL == nResult );
                if ( bUserConfirmedDelete && m_pSubComponentManager->closeSubFrames( sTableName, E_TABLE ) )
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
                            OSL_FAIL("OApplicationController::implDropTable: something strange happened!");
                    }
                    catch( const Exception& )
                    {
                        DBG_UNHANDLED_EXCEPTION();
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
            OUString sMessage(ModuleRes(STR_MISSING_TABLES_XDROP));
            ScopedVclPtrInstance< MessageDialog > aError(getView(), sMessage);
            aError->Execute();
        }
    }
}

void OApplicationController::deleteObjects( ElementType _eType, const ::std::vector< OUString>& _rList, bool _bConfirm )
{
    Reference< XNameContainer > xNames( getElements( _eType ), UNO_QUERY );
    Reference< XHierarchicalNameContainer > xHierarchyName( xNames, UNO_QUERY );
    if ( xNames.is() )
    {
        OString sDialogPosition;
        short eResult = _bConfirm ? svtools::QUERYDELETE_YES : svtools::QUERYDELETE_ALL;

        // The list of elements to delete is allowed to contain related elements: A given element may
        // be the ancestor or child of another element from the list.
        // We want to ensure that ancestors get deleted first, so we normalize the list in this respect.
        // #i33353#
        ::std::set< OUString > aDeleteNames;
            // Note that this implicitly uses ::std::less< OUString > a comparison operation, which
            // results in lexicographical order, which is exactly what we need, because "foo" is *before*
            // any "foo/bar" in this order.
        ::std::copy(
            _rList.begin(), _rList.end(),
            ::std::insert_iterator< ::std::set< OUString > >( aDeleteNames, aDeleteNames.begin() )
        );

        ::std::set< OUString >::size_type nCount = aDeleteNames.size();
        for ( ::std::set< OUString >::size_type nObjectsLeft = nCount; !aDeleteNames.empty(); )
        {
            ::std::set< OUString >::iterator  aThisRound = aDeleteNames.begin();

            if ( eResult != svtools::QUERYDELETE_ALL )
            {
                ScopedVclPtrInstance< svtools::QueryDeleteDlg_Impl > aDlg(getView(), *aThisRound);

                if ( !sDialogPosition.isEmpty() )
                    aDlg->SetWindowState( sDialogPosition );

                if ( nObjectsLeft > 1 )
                    aDlg->EnableAllButton();

                eResult = aDlg->Execute();
                if (eResult == svtools::QUERYDELETE_CANCEL)
                    return;

                sDialogPosition = aDlg->GetWindowState( );
            }

            bool bSuccess = false;

            bool bUserConfirmedDelete =
                        ( eResult == svtools::QUERYDELETE_ALL )
                    ||  ( eResult == svtools::QUERYDELETE_YES );

            if  (   bUserConfirmedDelete
                &&  (   _eType != E_QUERY || m_pSubComponentManager->closeSubFrames( *aThisRound, _eType ) )
                )
            {
                try
                {
                    if ( xHierarchyName.is() )
                        xHierarchyName->removeByHierarchicalName( *aThisRound );
                    else
                        xNames->removeByName( *aThisRound );

                    bSuccess = true;

                    // now that we removed the element, care for all its child elements
                    // which may also be a part of the list
                    // #i33353#
                    OSL_ENSURE( aThisRound->getLength() - 1 >= 0, "OApplicationController::deleteObjects: empty name?" );
                    OUStringBuffer sSmallestSiblingName( *aThisRound );
                    sSmallestSiblingName.append( (sal_Unicode)( '/' + 1) );

                    ::std::set< OUString >::iterator aUpperChildrenBound = aDeleteNames.lower_bound( sSmallestSiblingName.makeStringAndClear() );
                    for ( ::std::set< OUString >::iterator aObsolete = aThisRound;
                          aObsolete != aUpperChildrenBound;
                        )
                    {
                        ::std::set< OUString >::iterator aNextObsolete = aObsolete; ++aNextObsolete;
                        aDeleteNames.erase( aObsolete );
                        --nObjectsLeft;
                        aObsolete = aNextObsolete;
                    }
                }
                catch(const SQLException&)
                {
                    showError( SQLExceptionInfo( ::cppu::getCaughtException() ) );
                }
                catch(const WrappedTargetException& e)
                {
                    SQLException aSql;
                    if ( e.TargetException >>= aSql )
                        showError( SQLExceptionInfo( e.TargetException ) );
                    else
                        OSL_FAIL( "OApplicationController::deleteObjects: something strange happened!" );
                }
                catch( const Exception& )
                {
                    DBG_UNHANDLED_EXCEPTION();
                }
            }

            if ( !bSuccess )
            {
                // okay, this object could not be deleted (or the user did not want to delete it),
                // but continue with the rest
                aDeleteNames.erase( aThisRound );
                --nObjectsLeft;
            }
        }
    }
}

void OApplicationController::deleteEntries()
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( getMutex() );

    if ( getContainer() )
    {
        ::std::vector< OUString> aList;
        getSelectionElementNames(aList);
        ElementType eType = getContainer()->getElementType();
        switch(eType)
        {
        case E_TABLE:
            deleteTables(aList);
            break;
        case E_QUERY:
            deleteObjects( E_QUERY, aList, true );
            break;
        case E_FORM:
            deleteObjects( E_FORM, aList, true );
            break;
        case E_REPORT:
            deleteObjects( E_REPORT, aList, true );
            break;
        case E_NONE:
            break;
        }
    }
}

// DO NOT CALL with getMutex() held!!
const SharedConnection& OApplicationController::ensureConnection( ::dbtools::SQLExceptionInfo* _pErrorInfo )
{

    // This looks like double checked locking, but it is not,
    // because every access (read *or* write) to  m_xDataSourceConnection
    // is mutexed.
    // See http://www.cs.umd.edu/~pugh/java/memoryModel/DoubleCheckedLocking.html
    // for what I'm referring to.
    // We cannot use the TLS (thread-local storage) solution
    // since support for TLS is not up to the snuff on Windows :-(

    {
        ::osl::MutexGuard aGuard( getMutex() );

        if ( m_xDataSourceConnection.is() )
            return m_xDataSourceConnection;
    }

    WaitObject aWO(getView());
    Reference<XConnection> conn;
    {
        SolarMutexGuard aSolarGuard;

        OUString sConnectingContext( ModuleRes( STR_COULDNOTCONNECT_DATASOURCE ) );
        sConnectingContext = sConnectingContext.replaceFirst("$name$", getStrippedDatabaseName());

        // do the connection *without* holding getMutex() to avoid deadlock
        // when we are not in the main thread and we need username/password
        // (and thus to display a dialog, which will be done by the main thread)
        // and there is an event that needs getMutex() *before* us in the main thread's queue
        // See fdo#63391
        conn.set( connect( getDatabaseName(), sConnectingContext, _pErrorInfo ) );
    }

    if (conn.is())
    {
        ::osl::MutexGuard aGuard( getMutex() );
        if ( m_xDataSourceConnection.is() )
        {
            Reference< XComponent > comp (conn, UNO_QUERY);
            if(comp.is())
            {
                try
                {
                    comp->dispose();
                }
                catch( const Exception& )
                {
                    OSL_FAIL( "dbaui::OApplicationController::ensureConnection could not dispose of temporary unused connection" );
                }
            }
            conn.clear();
        }
        else
        {
            m_xDataSourceConnection.reset(conn);
            SQLExceptionInfo aError;
            try
            {
                m_xMetaData = m_xDataSourceConnection->getMetaData();
            }
            catch( const SQLException& )
            {
                aError = ::cppu::getCaughtException();
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION();
            }
            if ( aError.isValid() )
            {
                if ( _pErrorInfo )
                {
                    *_pErrorInfo = aError;
                }
                else
                {
                    SolarMutexGuard aSolarGuard;
                    showError( aError );
                }
            }
        }
    }

    return m_xDataSourceConnection;
}

bool OApplicationController::isDataSourceReadOnly() const
{
    Reference<XStorable> xStore(m_xModel,UNO_QUERY);
    return !xStore.is() || xStore->isReadonly();
}

bool OApplicationController::isConnectionReadOnly() const
{
    bool bIsConnectionReadOnly = true;
    if ( m_xMetaData.is() )
    {
        try
        {
            bIsConnectionReadOnly = m_xMetaData->isReadOnly();
        }
        catch(const SQLException&)
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }
    // TODO check configuration
    return bIsConnectionReadOnly;
}

Reference< XNameAccess > OApplicationController::getElements( ElementType _eType )
{
    Reference< XNameAccess > xElements;
    try
    {
        switch ( _eType )
        {
        case E_REPORT:
        {
            Reference< XReportDocumentsSupplier > xSupp( m_xModel, UNO_QUERY_THROW );
            xElements.set( xSupp->getReportDocuments(), UNO_SET_THROW );
        }
        break;

        case E_FORM:
        {
            Reference< XFormDocumentsSupplier > xSupp( m_xModel, UNO_QUERY_THROW );
            xElements.set( xSupp->getFormDocuments(), UNO_SET_THROW );
        }
        break;

        case E_QUERY:
        {
            xElements.set( getQueryDefinitions(), UNO_QUERY_THROW );
        }
        break;

        case E_TABLE:
        {
            if ( m_xDataSourceConnection.is() )
            {
                Reference< XTablesSupplier > xSup( getConnection(), UNO_QUERY_THROW );
                xElements.set( xSup->getTables(), UNO_SET_THROW );
            }
        }
        break;

        default:
            break;
        }
    }
    catch(const Exception&)
    {
        DBG_UNHANDLED_EXCEPTION();
    }

    return xElements;
}

void OApplicationController::getSelectionElementNames(::std::vector< OUString>& _rNames) const
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( getMutex() );

    OSL_ENSURE(getContainer(),"View isn't valid! -> GPF");

    getContainer()->getSelectionElementNames( _rNames );
}

::std::unique_ptr< OLinkedDocumentsAccess > OApplicationController::getDocumentsAccess( ElementType _eType )
{
    OSL_ENSURE( ( _eType == E_TABLE ) || ( _eType == E_QUERY ) || ( _eType == E_FORM ) || ( _eType == E_REPORT ),
        "OApplicationController::getDocumentsAccess: only forms and reports are supported here!" );

    SharedConnection xConnection( ensureConnection() );
    Reference< XNameAccess > xDocContainer;

    if ( ( _eType == E_FORM ) || ( _eType == E_REPORT ) )
    {
        xDocContainer.set( getElements( _eType ) );
        OSL_ENSURE( xDocContainer.is(), "OApplicationController::getDocumentsAccess: invalid container!" );
    }

    ::std::unique_ptr< OLinkedDocumentsAccess > pDocuments( new OLinkedDocumentsAccess(
        getView(), this, getORB(), xDocContainer, xConnection, getDatabaseName()
    ) );
    return pDocuments;
}

TransferableHelper* OApplicationController::copyObject()
{
    try
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( getMutex() );

        ElementType eType = getContainer()->getElementType();
        TransferableHelper* pData = nullptr;
        switch( eType )
        {
            case E_TABLE:
            case E_QUERY:
            {
                SharedConnection xConnection( ensureConnection() );
                Reference< XDatabaseMetaData> xMetaData;
                if ( xConnection.is() )
                    xMetaData = xConnection->getMetaData();

                OUString sName = getContainer()->getQualifiedName( nullptr );
                if ( !sName.isEmpty() )
                {
                    OUString sDataSource = getDatabaseName();

                    if ( eType == E_TABLE )
                    {
                        pData = new ODataClipboard(sDataSource, CommandType::TABLE, sName, xConnection, getNumberFormatter(xConnection, getORB()), getORB());
                    }
                    else
                    {
                        pData = new ODataClipboard(sDataSource, CommandType::QUERY, sName, getNumberFormatter(xConnection, getORB()), getORB());
                    }
                }
            }
                break;
            case E_FORM:
            case E_REPORT:
            {
                ::std::vector< OUString> aList;
                getSelectionElementNames(aList);
                Reference< XHierarchicalNameAccess > xElements(getElements(eType),UNO_QUERY);
                if ( xElements.is() && !aList.empty() )
                {
                    Reference< XContent> xContent(xElements->getByHierarchicalName(*aList.begin()),UNO_QUERY);
                    pData = new OComponentTransferable( getDatabaseName(), xContent );
                }
            }
            break;
            default:
                break;
        }

        // the ownership goes to ODataClipboards
        return pData;
    }
    catch(const SQLException&)
    {
        showError( SQLExceptionInfo( ::cppu::getCaughtException() ) );
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
    return nullptr;
}

bool OApplicationController::paste( ElementType _eType, const svx::ODataAccessDescriptor& _rPasteData, const OUString& _sParentFolder, bool _bMove)
{
    try
    {
        if ( _eType == E_QUERY )
        {
            sal_Int32 nCommandType = CommandType::TABLE;
            if ( _rPasteData.has(daCommandType) )
                _rPasteData[daCommandType]      >>= nCommandType;

            if ( CommandType::QUERY == nCommandType || CommandType::COMMAND == nCommandType )
            {
                // read all necessary data

                OUString sCommand;
                bool bEscapeProcessing = true;

                _rPasteData[daCommand] >>= sCommand;
                if ( _rPasteData.has(daEscapeProcessing) )
                    _rPasteData[daEscapeProcessing] >>= bEscapeProcessing;

                // plausibility check
                bool bValidDescriptor = false;
                OUString sDataSourceName = _rPasteData.getDataSource();
                if (CommandType::QUERY == nCommandType)
                    bValidDescriptor = sDataSourceName.getLength() && sCommand.getLength();
                else if (CommandType::COMMAND == nCommandType)
                    bValidDescriptor = !sCommand.isEmpty();
                if (!bValidDescriptor)
                {
                    OSL_FAIL("OApplicationController::paste: invalid descriptor!");
                    return false;
                }

                // the target object name (as we'll suggest it to the user)
                OUString sTargetName;
                try
                {
                    if ( CommandType::QUERY == nCommandType )
                        sTargetName = sCommand;

                    if ( sTargetName.isEmpty() )
                    {
                        OUString sDefaultName = OUString( ModuleRes( STR_QRY_TITLE ) );
                        sDefaultName = sDefaultName.getToken( 0, ' ' );

                        Reference< XNameAccess > xQueries( getQueryDefinitions(), UNO_QUERY_THROW );
                        sTargetName = ::dbtools::createUniqueName( xQueries, sDefaultName, false );
                    }
                }
                catch(const Exception&)
                {
                    DBG_UNHANDLED_EXCEPTION();
                }

                Reference< XPropertySet > xQuery;
                if (CommandType::QUERY == nCommandType)
                {
                    // need to extract the statement and the escape processing flag from the query object
                    bool bSuccess = false;
                    try
                    {
                        // the concrete query
                        Reference< XQueryDefinitionsSupplier > xSourceQuerySup(
                            getDataSourceByName( sDataSourceName, getView(), getORB(), nullptr ),
                            UNO_QUERY_THROW );
                        Reference< XNameAccess > xQueries( xSourceQuerySup->getQueryDefinitions(), UNO_SET_THROW );
                        if ( xQueries->hasByName( sCommand ) )
                        {
                            xQuery.set( xQueries->getByName(sCommand), UNO_QUERY_THROW );
                            bSuccess = true;
                        }
                    }
                    catch(SQLException&) { throw; } // caught and handled by the outer catch
                    catch( const Exception& )
                    {
                        DBG_UNHANDLED_EXCEPTION();
                    }

                    if (!bSuccess)
                    {
                        OSL_FAIL("OApplicationController::paste: could not extract the source query object!");
                        // TODO: maybe this is worth an error message to be displayed to the user ....
                        return false;
                    }
                }

                Reference< XNameContainer > xDestQueries(getQueryDefinitions(), UNO_QUERY);
                Reference< XSingleServiceFactory > xQueryFactory(xDestQueries, UNO_QUERY);
                if (!xQueryFactory.is())
                {
                    OSL_FAIL("OApplicationController::paste: invalid destination query container!");
                    return false;
                }

                // here we have everything needed to create a new query object ...
                // ... ehm, except a new name
                ensureConnection();

                DynamicTableOrQueryNameCheck aNameChecker( getConnection(), CommandType::QUERY );
                ::dbtools::SQLExceptionInfo aDummy;
                bool bNeedAskForName =  ( sCommand.isEmpty() )
                                            /* we did not have a source name, so the target name was auto-generated */
                                    ||  ( !aNameChecker.isNameValid( sTargetName, aDummy ) );
                                            /*  name is invalid in the target DB (e.g. because it already
                                                has a /table/ with that name) */
                if ( bNeedAskForName )
                {
                    ScopedVclPtrInstance<OSaveAsDlg> aAskForName(
                                            getView(),
                                            CommandType::QUERY,
                                            getORB(),
                                            getConnection(),
                                            sTargetName,
                                            aNameChecker,
                                            SAD_ADDITIONAL_DESCRIPTION | SAD_TITLE_PASTE_AS );
                    if ( RET_OK != aAskForName->Execute() )
                        // cancelled by the user
                        return false;
                    sTargetName = aAskForName->getName();
                }

                // create a new object
                Reference< XPropertySet > xNewQuery(xQueryFactory->createInstance(), UNO_QUERY);
                OSL_ENSURE(xNewQuery.is(), "OApplicationController::paste: invalid object created by factory!");
                if (xNewQuery.is())
                {
                    // initialize
                    if ( xQuery.is() )
                        ::comphelper::copyProperties(xQuery,xNewQuery);
                    else
                    {
                        xNewQuery->setPropertyValue(PROPERTY_COMMAND,makeAny(sCommand));
                        xNewQuery->setPropertyValue(PROPERTY_ESCAPE_PROCESSING,makeAny(bEscapeProcessing));
                    }
                    // insert
                    xDestQueries->insertByName( sTargetName, makeAny(xNewQuery) );
                    xNewQuery.set(xDestQueries->getByName( sTargetName),UNO_QUERY);
                    if ( xQuery.is() && xNewQuery.is() )
                    {
                        Reference<XColumnsSupplier> xSrcColSup(xQuery,UNO_QUERY);
                        Reference<XColumnsSupplier> xDstColSup(xNewQuery,UNO_QUERY);
                        if ( xSrcColSup.is() && xDstColSup.is() )
                        {
                            Reference<XNameAccess> xSrcNameAccess = xSrcColSup->getColumns();
                            Reference<XNameAccess> xDstNameAccess = xDstColSup->getColumns();
                            Reference<XDataDescriptorFactory> xFac(xDstNameAccess,UNO_QUERY);
                            Reference<XAppend> xAppend(xFac,UNO_QUERY);
                            if ( xSrcNameAccess.is() && xDstNameAccess.is() && xSrcNameAccess->hasElements() && xAppend.is() )
                            {
                                Reference<XPropertySet> xDstProp(xFac->createDataDescriptor());

                                Sequence< OUString> aSeq = xSrcNameAccess->getElementNames();
                                const OUString* pIter = aSeq.getConstArray();
                                const OUString* pEnd   = pIter + aSeq.getLength();
                                for( ; pIter != pEnd ; ++pIter)
                                {
                                    Reference<XPropertySet> xSrcProp(xSrcNameAccess->getByName(*pIter),UNO_QUERY);
                                    ::comphelper::copyProperties(xSrcProp,xDstProp);
                                    xAppend->appendByDescriptor(xDstProp);
                                }
                            }
                        }
                    }
                }
            }
            else
                OSL_TRACE("There should be a sequence in it!");
            return true;
        }
        else if ( _rPasteData.has(daComponent) ) // forms or reports
        {
            Reference<XContent> xContent;
            _rPasteData[daComponent] >>= xContent;
            return insertHierachyElement(_eType,_sParentFolder,Reference<XNameAccess>(xContent,UNO_QUERY).is(),xContent,_bMove);
        }
    }
    catch(const SQLException&) { showError( SQLExceptionInfo( ::cppu::getCaughtException() ) ); }
    catch(const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
    return false;
}

Reference<XNameContainer> OApplicationController::getQueryDefinitions() const
{
    Reference<XQueryDefinitionsSupplier> xSet(m_xDataSource,UNO_QUERY);
    Reference<XNameContainer> xNames;
    if ( xSet.is() )
    {
        xNames.set(xSet->getQueryDefinitions(),UNO_QUERY);
    }
    return xNames;
}

void OApplicationController::getSupportedFormats(ElementType _eType,::std::vector<SotClipboardFormatId>& _rFormatIds)
{
    switch( _eType )
    {
        case E_TABLE:
            _rFormatIds.push_back(SotClipboardFormatId::DBACCESS_TABLE);
            _rFormatIds.push_back(SotClipboardFormatId::RTF);
            _rFormatIds.push_back(SotClipboardFormatId::HTML);
            SAL_FALLTHROUGH;
        case E_QUERY:
            _rFormatIds.push_back(SotClipboardFormatId::DBACCESS_QUERY);
            break;
        default:
            break;
    }
}

bool OApplicationController::isTableFormat()  const
{
    return OTableCopyHelper::isTableFormat(getViewClipboard());
}

IMPL_LINK_NOARG_TYPED( OApplicationController, OnAsyncDrop, void*, void )
{
    m_nAsyncDrop = nullptr;
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( getMutex() );

    if ( m_aAsyncDrop.nType == E_TABLE )
    {
        SharedConnection xConnection( ensureConnection() );
        if ( xConnection.is() )
            m_aTableCopyHelper.asyncCopyTagTable( m_aAsyncDrop, getDatabaseName(), xConnection );
    }
    else
    {
        if ( paste(m_aAsyncDrop.nType,m_aAsyncDrop.aDroppedData,m_aAsyncDrop.aUrl,m_aAsyncDrop.nAction == DND_ACTION_MOVE)
            && m_aAsyncDrop.nAction == DND_ACTION_MOVE )
        {
            Reference<XContent> xContent;
            m_aAsyncDrop.aDroppedData[daComponent] >>= xContent;
            ::std::vector< OUString> aList;
            sal_Int32 nIndex = 0;
            OUString sName = xContent->getIdentifier()->getContentIdentifier();
            OUString sErase = sName.getToken(0,'/',nIndex); // we don't want to have the "private:forms" part
            if ( nIndex != -1 )
            {
                aList.push_back(sName.copy(sErase.getLength() + 1));
                deleteObjects( m_aAsyncDrop.nType, aList, false );
            }
        }
    }

    m_aAsyncDrop.aDroppedData.clear();
}

}   // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
