/*************************************************************************
 *
 *  $RCSfile: dbinteraction.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: fs $ $Date: 2000-10-26 18:08:16 $
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
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _DBAUI_INTERACTION_HXX_
#include "dbinteraction.hxx"
#endif
#ifndef _DBA_REGISTRATION_HELPER_HXX_
#include "registrationhelper.hxx"
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
#ifndef _SVTOOLS_LOGINDLG_HXX_
#include <svtools/logindlg.hxx>
#endif
#ifndef _DBU_RESOURCE_HRC_
#include "dbu_resource.hrc"
#endif
#ifndef _DBAUI_PARAMDIALOG_HXX_
#include "paramdialog.hxx"
#endif

//==========================================================================

extern "C" void SAL_CALL createRegistryInfo_OInteractionHandler()
{
    static OMultiInstanceAutoRegistration< ::dbaui::OInteractionHandler > aOInteractionHandler_AutoRegistration;
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
    IMPLEMENT_SERVICE_INFO1_STATIC(OInteractionHandler, "com.sun.star.comp.dbaccess.OInteractionHandler", "com.sun.star.sdb.InteractionHandler");

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

        DBG_ERROR("OInteractionHandler::handle: unsupported request type!");
    }

    //-------------------------------------------------------------------------
    void OInteractionHandler::implHandle(const ParametersRequest& _rParamRequest, const Sequence< Reference< XInteractionContinuation > >& _rContinuations)
    {
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
        ::svt::LoginDialog aLogin(NULL, nFlags, _rAuthRequest.ServerName, sRealm.Len() ? &sRealm : NULL);

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

        if (_rAuthRequest.ServerName.getLength())
        {
            String sLoginRequest(ModuleRes(STR_ENTER_CONNECTION_PASSWORD));
            sLoginRequest.SearchAndReplaceAscii("$name$", _rAuthRequest.ServerName.getStr()),
            aLogin.SetLoginRequestText(sLoginRequest);
        }

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
        sal_Int32 nApprovePos = getContinuation(APPROVE, _rContinuations);
        sal_Int32 nAbortPos = getContinuation(ABORT, _rContinuations);
        sal_Int32 nRetryPos = getContinuation(RETRY, _rContinuations);
#ifdef DBG_UTIL
        sal_Int32 nDisapprovePos = getContinuation(DISAPPROVE, _rContinuations);
        sal_Int32 nAuthentPos = getContinuation(SUPPLY_AUTHENTICATION, _rContinuations);
        DBG_ASSERT((-1 == nDisapprovePos) && (-1 == nAuthentPos), "OInteractionHandler::implHandle(SQLExceptionInfo): unsupported continuation type!");
            // "Retry" and "Authenticate" do not make sense if the request refered to an SQLException
#endif
        // determine the style of the dialog, dependent on the present continuation types
        WinBits nDialogStyle = WB_OK | WB_DEF_OK;
        if (-1 != nAbortPos)
            nDialogStyle = WB_OK_CANCEL;
        if (-1 != nRetryPos)
            nDialogStyle |= WB_RETRY_CANCEL | WB_DEF_RETRY;

        // excute the dialog
        OSQLMessageBox aDialog(NULL, _rSqlInfo, nDialogStyle);
            // TODO: need a way to specify the parent window
        sal_Int16 nResult = aDialog.Execute();
        try
        {
            switch (nResult)
            {
                case RET_OK:
                    if (-1 != nApprovePos)
                        _rContinuations[nApprovePos]->select();
                    break;
                case RET_CANCEL:
                    if (-1 != nAbortPos)
                        _rContinuations[nAbortPos]->select();
                    break;
                case RET_RETRY:
                    if (-1 != nRetryPos)
                        _rContinuations[nRetryPos]->select();
                    break;
            }
        }
        catch(RuntimeException&)
        {
            DBG_ERROR("OInteractionHandler::implHandle(SQLExceptionInfo): caught a RuntimeException while calling the continuation callback!");
        }
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
            }
        }

        return -1;
    }


//.........................................................................
}   // namespace dbaui
//.........................................................................

/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *  Revision 1.2  2000/10/26 07:32:32  fs
 *  special login request text, with interpreting the ServerName as data source name
 *
 *  Revision 1.1  2000/10/25 12:59:42  fs
 *  initial checkin - InteractionHandler for common database related interaction requests
 *
 *
 *  Revision 1.0 25.10.00 10:18:21  fs
 ************************************************************************/

