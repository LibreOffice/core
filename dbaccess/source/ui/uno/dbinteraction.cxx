/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dbinteraction.cxx,v $
 *
 *  $Revision: 1.16 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-30 08:56:49 $
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

#ifndef _DBAUI_INTERACTION_HXX_
#include "dbinteraction.hxx"
#endif
#ifndef _DBU_REGHELPER_HXX_
#include "dbu_reghelper.hxx"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef _DBHELPER_DBEXCEPTION_HXX_
#include <connectivity/dbexception.hxx>
#endif
#ifndef _DBAUI_SQLMESSAGE_HXX_
#include "sqlmessage.hxx"
#endif
#ifndef _COM_SUN_STAR_TASK_XINTERACTIONAPPROVE_HPP_
#include <com/sun/star/task/XInteractionApprove.hpp>
#endif
#ifndef _COM_SUN_STAR_TASK_XINTERACTIONDISAPPROVE_HPP_
#include <com/sun/star/task/XInteractionDisapprove.hpp>
#endif
#ifndef _COM_SUN_STAR_TASK_XINTERACTIONRETRY_HPP_
#include <com/sun/star/task/XInteractionRetry.hpp>
#endif
#ifndef _COM_SUN_STAR_TASK_XINTERACTIONABORT_HPP_
#include <com/sun/star/task/XInteractionAbort.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XINTERACTIONSUPPLYAUTHENTICATION_HPP_
#include <com/sun/star/ucb/XInteractionSupplyAuthentication.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XINTERACTIONSUPPLYPARAMETERS_HPP_
#include <com/sun/star/sdb/XInteractionSupplyParameters.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XINTERACTIONDOCUMENTSAVE_HPP_
#include <com/sun/star/sdb/XInteractionDocumentSave.hpp>
#endif
#ifndef _SVTOOLS_LOGINDLG_HXX_
#include <svtools/logindlg.hxx>
#endif
#ifndef SFX_QUERYSAVEDOCUMENT_HXX
#include <sfx2/QuerySaveDocument.hxx>
#endif
#ifndef _DBU_UNO_HRC_
#include "dbu_uno.hrc"
#endif
#ifndef _DBAUI_PARAMDIALOG_HXX_
#include "paramdialog.hxx"
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif
#ifndef DBAUI_COLLECTIONVIEW_HXX
#include "CollectionView.hxx"
#endif
#ifndef DBAUI_TOOLS_HXX
#include "UITools.hxx"
#endif


//==========================================================================

extern "C" void SAL_CALL createRegistryInfo_OInteractionHandler()
{
    static ::dbaui::OMultiInstanceAutoRegistration< ::dbaui::OInteractionHandler > aOInteractionHandler_AutoRegistration;
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
    //= OInteractionHandler
    //=========================================================================
    //-------------------------------------------------------------------------
    OInteractionHandler::OInteractionHandler(const Reference< XMultiServiceFactory >& _rxORB)
        :m_xORB(_rxORB)
    {
    }

    //-------------------------------------------------------------------------
    IMPLEMENT_SERVICE_INFO1_STATIC(OInteractionHandler, "com.sun.star.comp.dbu.OInteractionHandler", "com.sun.star.sdb.InteractionHandler");

    //-------------------------------------------------------------------------
    void SAL_CALL OInteractionHandler::handle(const Reference< XInteractionRequest >& _rxRequest) throw(RuntimeException)
    {
        Any aRequest;
        if (_rxRequest.is())
        {
            try { aRequest = _rxRequest->getRequest(); }
            catch(RuntimeException&) { }
        }
        DBG_ASSERT(aRequest.hasValue(), "OInteractionHandler::handle: invalid request!");
        if (!aRequest.hasValue())
            // no request -> no handling
            return;

        Sequence< Reference< XInteractionContinuation > > aContinuations;
        try { aContinuations = _rxRequest->getContinuations(); }
        catch(RuntimeException&) { }

        // try to extract an SQLException (or one of it's derived members
        SQLExceptionInfo aInfo(aRequest);
        if (aInfo.isValid())
        {
            implHandle(aInfo, aContinuations);
            return;
        }

        AuthenticationRequest aAuthentRequest;
        if (aRequest >>= aAuthentRequest)
        {   // it's an authentification request
            implHandle(aAuthentRequest, aContinuations);
            return;
        }

        ParametersRequest aParamRequest;
        if (aRequest >>= aParamRequest)
        {   // it's an authentification request
            implHandle(aParamRequest, aContinuations);
            return;
        }

        DocumentSaveRequest aDocuRequest;
        if (aRequest >>= aDocuRequest)
        {   // it's an document request
            implHandle(aDocuRequest, aContinuations);
            return;
        }

        OSL_VERIFY( implHandleUnknown( _rxRequest ) );
    }

    //-------------------------------------------------------------------------
    void OInteractionHandler::implHandle(const ParametersRequest& _rParamRequest, const Sequence< Reference< XInteractionContinuation > >& _rContinuations)
    {
        ::vos::OGuard aGuard(Application::GetSolarMutex());
            // want to open a dialog ....

        sal_Int32 nAbortPos = getContinuation(ABORT, _rContinuations);
        sal_Int32 nParamPos = getContinuation(SUPPLY_PARAMETERS, _rContinuations);

        Reference< XInteractionSupplyParameters > xParamCallback;
        if (-1 != nParamPos)
            xParamCallback = Reference< XInteractionSupplyParameters >(_rContinuations[nParamPos], UNO_QUERY);
        DBG_ASSERT(xParamCallback.is(), "OInteractionHandler::implHandle(ParametersRequest): can't set the parameters without an appropriate interaction handler!s");

        // determine the style of the dialog, dependent on the present continuation types
        WinBits nDialogStyle = WB_OK | WB_DEF_OK;
        if (-1 != nAbortPos)
            nDialogStyle = WB_OK_CANCEL;

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
        catch(RuntimeException&)
        {
            DBG_ERROR("OInteractionHandler::implHandle(ParametersRequest): caught a RuntimeException while calling the continuation callback!");
        }
    }

    //-------------------------------------------------------------------------
    void OInteractionHandler::implHandle(const AuthenticationRequest& _rAuthRequest, const Sequence< Reference< XInteractionContinuation > >& _rContinuations)
    {
        ::vos::OGuard aGuard(Application::GetSolarMutex());
            // want to open a dialog ....

        // search the continuations we can handle
        sal_Int32 nAbortPos = getContinuation(ABORT, _rContinuations);
        sal_Int32 nRetryPos = getContinuation(RETRY, _rContinuations);
        sal_Int32 nAuthentPos = getContinuation(SUPPLY_AUTHENTICATION, _rContinuations);

        // we strongly need an XInteractionSupplyAuthentication (else we can't return the input given by the user)
        Reference< XInteractionSupplyAuthentication > xSuppAuthent;
        if (-1 != nAuthentPos)
            xSuppAuthent = Reference< XInteractionSupplyAuthentication >(_rContinuations[nAuthentPos], UNO_QUERY);
        DBG_ASSERT(xSuppAuthent.is(), "OInteractionHandler::implHandle: need an XInteractionSupplyAuthentication to return the results!");

        // check which "remember password" modes are allowed
        sal_Bool bRemember(sal_False);
        sal_Bool bRememberPersistent(sal_False);
        if (xSuppAuthent.is())
        {
            RememberAuthentication eDefault;
            Sequence< RememberAuthentication > aModes(xSuppAuthent->getRememberPasswordModes(eDefault));
            const RememberAuthentication* pModes = aModes.getConstArray();
            bRemember = eDefault != RememberAuthentication_NO;
            for (sal_Int32 i=0; i<aModes.getLength(); ++i, ++pModes)
                if (*pModes == RememberAuthentication_PERSISTENT)
                {
                    bRememberPersistent = sal_True;
                    break;
                }
        }

        // extract some infor from the authentication request
        // use the account as realm
        String sRealm;
        if (_rAuthRequest.HasRealm)
            sRealm = _rAuthRequest.Realm.getStr();

        // determine the flags for
        sal_uInt16 nFlags = 0;
        nFlags |= LF_NO_PATH;   // the AuthenticationRequest does not support a path

        if (0 == _rAuthRequest.Diagnostic.getLength())
            nFlags |= LF_NO_ERRORTEXT;
        if (!_rAuthRequest.HasAccount)
            nFlags |= LF_NO_ACCOUNT;
        if (!_rAuthRequest.HasUserName || !xSuppAuthent.is() || !xSuppAuthent->canSetUserName())
            nFlags |= LF_USERNAME_READONLY;

        // create the dialog
        ::rtl::OUString sName = _rAuthRequest.ServerName;
        sName = ::dbaui::getStrippedDatabaseName(NULL,sName);
        ::svt::LoginDialog aLogin(NULL, nFlags, sName, sRealm.Len() ? &sRealm : NULL);

        // initialize it
        aLogin.SetErrorText(_rAuthRequest.Diagnostic.getStr());
        aLogin.SetName(_rAuthRequest.UserName);
        if (_rAuthRequest.HasAccount)
            aLogin.ClearAccount();
        else
            aLogin.ClearPassword();

        aLogin.SetPassword(_rAuthRequest.Password.getStr());

        aLogin.SetSavePassword(bRemember);
        aLogin.SetSavePasswordText(ModuleRes(bRememberPersistent ? STR_REMEMBERPASSWORD_PERSISTENT : STR_REMEMBERPASSWORD_SESSION));

        String sLoginRequest(ModuleRes(STR_ENTER_CONNECTION_PASSWORD));
        if (sName.getLength())
               sLoginRequest.SearchAndReplaceAscii("$name$", sName.getStr());
        else
        {
            sLoginRequest.SearchAndReplaceAscii("\"$name$\"", String());
            sLoginRequest.SearchAndReplaceAscii("$name$", String()); // just to be sure that in other languages the string will be deleted
        }
        aLogin.SetLoginRequestText(sLoginRequest);

        // execute
        sal_Int32 nResult = aLogin.Execute();

        // dispatch the result
        try
        {
            switch (nResult)
            {
                case RET_OK:
                    if (xSuppAuthent.is())
                    {
                        xSuppAuthent->setUserName(aLogin.GetName());
                        xSuppAuthent->setPassword(aLogin.GetPassword());
                        xSuppAuthent->setRememberPassword(
                                    aLogin.IsSavePassword()
                                    ?
                                            bRememberPersistent
                                        ?   RememberAuthentication_PERSISTENT
                                        :   RememberAuthentication_SESSION
                                    :   RememberAuthentication_NO);
                        if (_rAuthRequest.HasAccount)
                            xSuppAuthent->setAccount(aLogin.GetAccount());
                        xSuppAuthent->select();
                    }
                    break;
                case RET_RETRY:
                    if (-1 != nRetryPos)
                        _rContinuations[nRetryPos]->select();
                    break;
                default:
                    if (-1 != nAbortPos)
                        _rContinuations[nAbortPos]->select();
                    break;
            }
        }
        catch(Exception&)
        {
            DBG_ERROR("OInteractionHandler::implHandle(AuthenticationRequest): error while calling back into the InteractionContinuation!");
        }
    }

    //-------------------------------------------------------------------------
    void OInteractionHandler::implHandle(const SQLExceptionInfo& _rSqlInfo, const Sequence< Reference< XInteractionContinuation > >& _rContinuations)
    {
        ::vos::OGuard aGuard(Application::GetSolarMutex());
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

        // excute the dialog
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
                        OSL_ENSURE( nResult != RET_YES, "OInteractionHandler::implHandle: no handler for YES!" );
                    break;

                case RET_NO:
                    if ( nDisapprovePos != -1 )
                        _rContinuations[ nDisapprovePos ]->select();
                    else
                        OSL_ENSURE( false, "OInteractionHandler::implHandle: no handler for NO!" );
                    break;

                case RET_CANCEL:
                    if ( nAbortPos != -1 )
                        _rContinuations[ nAbortPos ]->select();
                    else if ( nDisapprovePos != -1 )
                        _rContinuations[ nDisapprovePos ]->select();
                    else
                        OSL_ENSURE( false, "OInteractionHandler::implHandle: no handler for CANCEL!" );
                    break;
                case RET_RETRY:
                    if ( nRetryPos != -1 )
                        _rContinuations[ nRetryPos ]->select();
                    else
                        OSL_ENSURE( false, "OInteractionHandler::implHandle: where does the RETRY come from?" );
                    break;
            }
        }
        catch(RuntimeException&)
        {
            DBG_ERROR("OInteractionHandler::implHandle(SQLExceptionInfo): caught a RuntimeException while calling the continuation callback!");
        }
    }
    //-------------------------------------------------------------------------
    void OInteractionHandler::implHandle(const DocumentSaveRequest& _rDocuRequest, const Sequence< Reference< XInteractionContinuation > >& _rContinuations)
    {
        ::vos::OGuard aGuard(Application::GetSolarMutex());
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
                DBG_ASSERT(xCallback.is(), "OInteractionHandler::implHandle(DocumentSaveRequest): can't save document without an appropriate interaction handler!s");

                // determine the style of the dialog, dependent on the present continuation types
                WinBits nDialogStyle = WB_OK | WB_DEF_OK;
                if (-1 != nAbortPos)
                    nDialogStyle = WB_OK_CANCEL;

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
                catch(RuntimeException&)
                {
                    DBG_ERROR("OInteractionHandler::implHandle(DocumentSaveRequest): caught a RuntimeException while calling the continuation callback!");
                }
            }
            else if ( -1 != nApprovePos )
                _rContinuations[nApprovePos]->select();
        }
        else if ( -1 != nDisApprovePos )
                _rContinuations[nDisApprovePos]->select();
    }

    //-------------------------------------------------------------------------
    bool OInteractionHandler::implHandleUnknown( const Reference< XInteractionRequest >& _rxRequest )
    {
        Reference< XInteractionHandler > xFallbackHandler;
        if ( m_xORB.is() )
            xFallbackHandler = xFallbackHandler.query( m_xORB->createInstance( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.task.InteractionHandler" ) ) ) );
        if ( xFallbackHandler.is() )
        {
            xFallbackHandler->handle( _rxRequest );
            return true;
        }
        return false;
    }

    //-------------------------------------------------------------------------
    sal_Int32 OInteractionHandler::getContinuation(Continuation _eCont, const Sequence< Reference< XInteractionContinuation > >& _rContinuations)
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
                case SUPPLY_AUTHENTICATION:
                    if (Reference< XInteractionSupplyAuthentication >(*pContinuations, UNO_QUERY).is())
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


//.........................................................................
}   // namespace dbaui
//.........................................................................

