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


#include "dbinteraction.hxx"
#include "dbu_reghelper.hxx"
#include <tools/diagnose_ex.h>
#include <osl/diagnose.h>
#include <vcl/msgbox.hxx>
#include <connectivity/dbexception.hxx>
#include "sqlmessage.hxx"
#include <com/sun/star/task/InteractionHandler.hpp>
#include <com/sun/star/task/XInteractionApprove.hpp>
#include <com/sun/star/task/XInteractionDisapprove.hpp>
#include <com/sun/star/task/XInteractionRetry.hpp>
#include <com/sun/star/task/XInteractionAbort.hpp>
#include <com/sun/star/sdb/XInteractionSupplyParameters.hpp>
#include <com/sun/star/sdb/XInteractionDocumentSave.hpp>
#include <sfx2/QuerySaveDocument.hxx>
#include "dbu_uno.hrc"
#include "paramdialog.hxx"
#include <vcl/svapp.hxx>
#include <osl/mutex.hxx>
#include "CollectionView.hxx"
#include "UITools.hxx"
#include <comphelper/processfactory.hxx>


//==========================================================================

extern "C" void SAL_CALL createRegistryInfo_OInteractionHandler()
{
    static ::dbaui::OMultiInstanceAutoRegistration< ::dbaui::SQLExceptionInteractionHandler > aSQLExceptionInteractionHandler_AutoRegistration;
    static ::dbaui::OMultiInstanceAutoRegistration< ::dbaui::LegacyInteractionHandler > aLegacyInteractionHandler_AutoRegistration;
}

//.........................................................................
namespace dbaui
{
//.........................................................................
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::ucb;
    using namespace ::com::sun::star::sdb;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::task;
    using namespace ::com::sun::star::beans;
    using namespace ::dbtools;

    //=========================================================================
    //= BasicInteractionHandler
    //=========================================================================
    //-------------------------------------------------------------------------
    BasicInteractionHandler::BasicInteractionHandler( const Reference< XMultiServiceFactory >& _rxORB, const bool i_bFallbackToGeneric )
        :m_xORB( _rxORB )
        ,m_bFallbackToGeneric( i_bFallbackToGeneric )
    {
        OSL_ENSURE( !m_bFallbackToGeneric,
            "BasicInteractionHandler::BasicInteractionHandler: enabling legacy behavior, there should be no clients of this anymore!" );
    }

    //-------------------------------------------------------------------------
    ::sal_Bool SAL_CALL BasicInteractionHandler::handleInteractionRequest( const Reference< XInteractionRequest >& i_rRequest ) throw (RuntimeException)
    {
        return impl_handle_throw( i_rRequest );
    }

    //-------------------------------------------------------------------------
    void SAL_CALL BasicInteractionHandler::handle( const Reference< XInteractionRequest >& i_rRequest ) throw(RuntimeException)
    {
        impl_handle_throw( i_rRequest );
    }

    //-------------------------------------------------------------------------
    sal_Bool BasicInteractionHandler::impl_handle_throw( const Reference< XInteractionRequest >& i_Request )
    {
        Any aRequest( i_Request->getRequest() );
        OSL_ENSURE(aRequest.hasValue(), "BasicInteractionHandler::handle: invalid request!");
        if ( !aRequest.hasValue() )
            // no request -> no handling
            return sal_False;

        Sequence< Reference< XInteractionContinuation > > aContinuations( i_Request->getContinuations() );

        // try to extract an SQLException (or one of it's derived members
        SQLExceptionInfo aInfo( aRequest );
        if ( aInfo.isValid() )
        {
            implHandle( aInfo, aContinuations );
            return sal_True;
        }

        ParametersRequest aParamRequest;
        if ( aRequest >>= aParamRequest )
        {
            implHandle( aParamRequest, aContinuations );
            return sal_True;
        }

        DocumentSaveRequest aDocuRequest;
        if ( aRequest >>= aDocuRequest )
        {
            implHandle( aDocuRequest, aContinuations );
            return sal_True;
        }

        if ( m_bFallbackToGeneric )
            return implHandleUnknown( i_Request );

        return sal_False;
    }

    //-------------------------------------------------------------------------
    void BasicInteractionHandler::implHandle(const ParametersRequest& _rParamRequest, const Sequence< Reference< XInteractionContinuation > >& _rContinuations)
    {
        SolarMutexGuard aGuard;
            // want to open a dialog ....

        sal_Int32 nAbortPos = getContinuation(ABORT, _rContinuations);
        sal_Int32 nParamPos = getContinuation(SUPPLY_PARAMETERS, _rContinuations);

        Reference< XInteractionSupplyParameters > xParamCallback;
        if (-1 != nParamPos)
            xParamCallback = Reference< XInteractionSupplyParameters >(_rContinuations[nParamPos], UNO_QUERY);
        OSL_ENSURE(xParamCallback.is(), "BasicInteractionHandler::implHandle(ParametersRequest): can't set the parameters without an appropriate interaction handler!s");

        OParameterDialog aDlg(NULL, _rParamRequest.Parameters, _rParamRequest.Connection, m_xORB);
        sal_Int16 nResult = aDlg.Execute();
        try
        {
            switch (nResult)
            {
                case RET_OK:
                    if (xParamCallback.is())
                    {
                        xParamCallback->setParameters(aDlg.getValues());
                        xParamCallback->select();
                    }
                    break;
                default:
                    if (-1 != nAbortPos)
                        _rContinuations[nAbortPos]->select();
                    break;
            }
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }

    //-------------------------------------------------------------------------
    void BasicInteractionHandler::implHandle(const SQLExceptionInfo& _rSqlInfo, const Sequence< Reference< XInteractionContinuation > >& _rContinuations)
    {
        SolarMutexGuard aGuard;
            // want to open a dialog ....

        sal_Int32 nApprovePos = getContinuation(APPROVE, _rContinuations);
        sal_Int32 nDisapprovePos = getContinuation(DISAPPROVE, _rContinuations);
        sal_Int32 nAbortPos = getContinuation(ABORT, _rContinuations);
        sal_Int32 nRetryPos = getContinuation(RETRY, _rContinuations);

        // determine the style of the dialog, dependent on the present continuation types
        WinBits nDialogStyle = 0;
        bool bHaveCancel = nAbortPos != -1;
        // "approve" means "Yes", "disapprove" means "No"
        // VCL only supports having both (which makes sense ...)
        if ( ( nApprovePos != -1 ) || ( nDisapprovePos != -1 ) )
            nDialogStyle = ( bHaveCancel ? WB_YES_NO_CANCEL : WB_YES_NO ) | WB_DEF_YES;
        else
        {
            // if there's no yes/no, then use a default OK button
            nDialogStyle = ( bHaveCancel ? WB_OK_CANCEL : WB_OK ) | WB_DEF_OK;
        }

        // If there's a "Retry" continuation, have a "Retry" button
        if ( nRetryPos != -1 )
        {
            nDialogStyle = WB_RETRY_CANCEL | WB_DEF_RETRY;
        }

        // execute the dialog
        OSQLMessageBox aDialog(NULL, _rSqlInfo, nDialogStyle);
            // TODO: need a way to specify the parent window
        sal_Int16 nResult = aDialog.Execute();
        try
        {
            switch (nResult)
            {
                case RET_YES:
                case RET_OK:
                    if ( nApprovePos != -1 )
                        _rContinuations[ nApprovePos ]->select();
                    else
                        OSL_ENSURE( nResult != RET_YES, "BasicInteractionHandler::implHandle: no handler for YES!" );
                    break;

                case RET_NO:
                    if ( nDisapprovePos != -1 )
                        _rContinuations[ nDisapprovePos ]->select();
                    else
                        OSL_FAIL( "BasicInteractionHandler::implHandle: no handler for NO!" );
                    break;

                case RET_CANCEL:
                    if ( nAbortPos != -1 )
                        _rContinuations[ nAbortPos ]->select();
                    else if ( nDisapprovePos != -1 )
                        _rContinuations[ nDisapprovePos ]->select();
                    else
                        OSL_FAIL( "BasicInteractionHandler::implHandle: no handler for CANCEL!" );
                    break;
                case RET_RETRY:
                    if ( nRetryPos != -1 )
                        _rContinuations[ nRetryPos ]->select();
                    else
                        OSL_FAIL( "BasicInteractionHandler::implHandle: where does the RETRY come from?" );
                    break;
            }
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }
    //-------------------------------------------------------------------------
    void BasicInteractionHandler::implHandle(const DocumentSaveRequest& _rDocuRequest, const Sequence< Reference< XInteractionContinuation > >& _rContinuations)
    {
        SolarMutexGuard aGuard;
            // want to open a dialog ....

        sal_Int32 nApprovePos = getContinuation(APPROVE, _rContinuations);
        sal_Int32 nDisApprovePos = getContinuation(DISAPPROVE, _rContinuations);
        sal_Int32 nAbortPos = getContinuation(ABORT, _rContinuations);

        short nRet = RET_YES;
        if ( -1 != nApprovePos )
        {
            // fragen, ob gespeichert werden soll
            nRet = ExecuteQuerySaveDocument(NULL,_rDocuRequest.Name);
        }

        if ( RET_CANCEL == nRet )
        {
            if (-1 != nAbortPos)
                _rContinuations[nAbortPos]->select();
            return;
        }
        else if ( RET_YES == nRet )
        {
            sal_Int32 nDocuPos = getContinuation(SUPPLY_DOCUMENTSAVE, _rContinuations);

            if (-1 != nDocuPos)
            {
                Reference< XInteractionDocumentSave > xCallback(_rContinuations[nDocuPos], UNO_QUERY);
                OSL_ENSURE(xCallback.is(), "BasicInteractionHandler::implHandle(DocumentSaveRequest): can't save document without an appropriate interaction handler!s");

                OCollectionView aDlg(NULL,_rDocuRequest.Content,_rDocuRequest.Name,m_xORB);
                sal_Int16 nResult = aDlg.Execute();
                try
                {
                    switch (nResult)
                    {
                        case RET_OK:
                            if (xCallback.is())
                            {
                                xCallback->setName(aDlg.getName(),aDlg.getSelectedFolder());
                                xCallback->select();
                            }
                            break;
                        default:
                            if (-1 != nAbortPos)
                                _rContinuations[nAbortPos]->select();
                            break;
                    }
                }
                catch( const Exception& )
                {
                    DBG_UNHANDLED_EXCEPTION();
                }
            }
            else if ( -1 != nApprovePos )
                _rContinuations[nApprovePos]->select();
        }
        else if ( -1 != nDisApprovePos )
                _rContinuations[nDisApprovePos]->select();
    }

    //-------------------------------------------------------------------------
    bool BasicInteractionHandler::implHandleUnknown( const Reference< XInteractionRequest >& _rxRequest )
    {
        if ( m_xORB.is() )
        {
            Reference< XInteractionHandler2 > xFallbackHandler(
                InteractionHandler::createWithParent(comphelper::getComponentContext(m_xORB), 0) );
            xFallbackHandler->handle( _rxRequest );
            return true;
        }
        return false;
    }

    //-------------------------------------------------------------------------
    sal_Int32 BasicInteractionHandler::getContinuation(Continuation _eCont, const Sequence< Reference< XInteractionContinuation > >& _rContinuations)
    {
        const Reference< XInteractionContinuation >* pContinuations = _rContinuations.getConstArray();
        for (sal_Int32 i=0; i<_rContinuations.getLength(); ++i, ++pContinuations)
        {
            switch (_eCont)
            {
                case APPROVE:
                    if (Reference< XInteractionApprove >(*pContinuations, UNO_QUERY).is())
                        return i;
                    break;
                case DISAPPROVE:
                    if (Reference< XInteractionDisapprove >(*pContinuations, UNO_QUERY).is())
                        return i;
                    break;
                case RETRY:
                    if (Reference< XInteractionRetry >(*pContinuations, UNO_QUERY).is())
                        return i;
                    break;
                case ABORT:
                    if (Reference< XInteractionAbort >(*pContinuations, UNO_QUERY).is())
                        return i;
                    break;
                case SUPPLY_PARAMETERS:
                    if (Reference< XInteractionSupplyParameters >(*pContinuations, UNO_QUERY).is())
                        return i;
                    break;
                case SUPPLY_DOCUMENTSAVE:
                    if (Reference< XInteractionDocumentSave >(*pContinuations, UNO_QUERY).is())
                        return i;
                    break;
            }
        }

        return -1;
    }

    //==========================================================================
    //= SQLExceptionInteractionHandler
    //==========================================================================
    IMPLEMENT_SERVICE_INFO1_STATIC( SQLExceptionInteractionHandler, "com.sun.star.comp.dbaccess.DatabaseInteractionHandler", "com.sun.star.sdb.DatabaseInteractionHandler" );

    //==========================================================================
    //= LegacyInteractionHandler
    //==========================================================================
    IMPLEMENT_SERVICE_INFO1_STATIC( LegacyInteractionHandler, "com.sun.star.comp.dbaccess.LegacyInteractionHandler", "com.sun.star.sdb.InteractionHandler" );

//.........................................................................
}   // namespace dbaui
//.........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
