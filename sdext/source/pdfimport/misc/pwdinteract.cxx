/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: pwdinteract.cxx,v $
 *
 * $Revision: 1.2 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sdext.hxx"

#include "pdfihelper.hxx"

#include <com/sun/star/task/XInteractionHandler.hpp>
#include <com/sun/star/task/XInteractionRequest.hpp>
#include <com/sun/star/task/XInteractionPassword.hpp>
#include <com/sun/star/task/PasswordRequest.hpp>

#include <cppuhelper/exc_hlp.hxx>
#include <cppuhelper/compbase2.hxx>
#include <cppuhelper/basemutex.hxx>


using namespace com::sun::star;

namespace
{

typedef ::cppu::WeakComponentImplHelper2<
    com::sun::star::task::XInteractionRequest,
    com::sun::star::task::XInteractionPassword > PDFPasswordRequestBase;

class PDFPasswordRequest : private cppu::BaseMutex,
                           public PDFPasswordRequestBase
{
private:
    task::PasswordRequest m_aRequest;
    rtl::OUString         m_aPassword;
    bool                  m_bSelected;

public:
    explicit PDFPasswordRequest(bool bFirstTry);

    // XInteractionRequest
    virtual uno::Any SAL_CALL getRequest(  ) throw (uno::RuntimeException);
    virtual uno::Sequence< uno::Reference< task::XInteractionContinuation > > SAL_CALL getContinuations(  ) throw (uno::RuntimeException);

    // XInteractionPassword
    virtual void SAL_CALL setPassword( const rtl::OUString& rPwd ) throw (uno::RuntimeException);
    virtual rtl::OUString SAL_CALL getPassword() throw (uno::RuntimeException);

    // XInteractionContinuation
    virtual void SAL_CALL select() throw (uno::RuntimeException);

    bool isSelected() const { osl::MutexGuard const guard( m_aMutex ); return m_bSelected; }
};

PDFPasswordRequest::PDFPasswordRequest( bool bFirstTry ) :
    PDFPasswordRequestBase( m_aMutex ),
    m_aRequest(),
    m_aPassword(),
    m_bSelected(false)
{
    m_aRequest.Mode = bFirstTry ?
        task::PasswordRequestMode_PASSWORD_ENTER :
        task::PasswordRequestMode_PASSWORD_REENTER;
    m_aRequest.Classification = task::InteractionClassification_QUERY;
}

uno::Any SAL_CALL PDFPasswordRequest::getRequest() throw (uno::RuntimeException)
{
    osl::MutexGuard const guard( m_aMutex );

    uno::Any aRet;
    aRet <<= m_aRequest;
    return aRet;
}

uno::Sequence< uno::Reference< task::XInteractionContinuation > > SAL_CALL PDFPasswordRequest::getContinuations() throw (uno::RuntimeException)
{
    osl::MutexGuard const guard( m_aMutex );

    uno::Sequence< uno::Reference< task::XInteractionContinuation > > aRet( 1 );
    aRet.getArray()[0] = static_cast<task::XInteractionContinuation*>(this);
    return aRet;
}

void SAL_CALL PDFPasswordRequest::setPassword( const rtl::OUString& rPwd ) throw (uno::RuntimeException)
{
    osl::MutexGuard const guard( m_aMutex );

    m_aPassword = rPwd;
}

rtl::OUString SAL_CALL PDFPasswordRequest::getPassword() throw (uno::RuntimeException)
{
    osl::MutexGuard const guard( m_aMutex );

    return m_aPassword;
}

void SAL_CALL PDFPasswordRequest::select() throw (uno::RuntimeException)
{
    osl::MutexGuard const guard( m_aMutex );

    m_bSelected = true;
}

} // namespace

namespace pdfi
{

bool getPassword( const uno::Reference< task::XInteractionHandler >& xHandler,
                  rtl::OUString&                                     rOutPwd,
                  bool                                               bFirstTry )
{
    bool bSuccess = false;

    PDFPasswordRequest* pRequest;
    uno::Reference< task::XInteractionRequest > xReq(
        pRequest = new PDFPasswordRequest( bFirstTry ) );
    try
    {
        xHandler->handle( xReq );
    }
    catch( uno::Exception& )
    {
    }

    OSL_TRACE( "request %s selected\n", pRequest->isSelected() ? "was" : "was not" );
    if( pRequest->isSelected() )
    {
        bSuccess = true;
        rOutPwd = pRequest->getPassword();
    }

    return bSuccess;
}

}
