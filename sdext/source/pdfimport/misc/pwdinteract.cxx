/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include "sal/config.h"

#include <cassert>

#include "pdfihelper.hxx"

#include <boost/noncopyable.hpp>
#include <com/sun/star/task/ErrorCodeRequest.hpp>
#include <com/sun/star/task/XInteractionHandler.hpp>
#include <com/sun/star/task/XInteractionRequest.hpp>
#include <com/sun/star/task/XInteractionPassword.hpp>
#include <com/sun/star/task/DocumentPasswordRequest.hpp>

#include <cppuhelper/exc_hlp.hxx>
#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/implbase2.hxx>
#include <osl/mutex.hxx>
#include <rtl/ref.hxx>
#include <tools/errcode.hxx>

using namespace com::sun::star;

namespace
{

class PDFPasswordRequest:
    public cppu::WeakImplHelper2<
        task::XInteractionRequest, task::XInteractionPassword >,
    private boost::noncopyable
{
private:
    mutable osl::Mutex            m_aMutex;
    uno::Any                      m_aRequest;
    rtl::OUString                 m_aPassword;
    bool                          m_bSelected;

public:
    explicit PDFPasswordRequest(bool bFirstTry, const rtl::OUString& rName);

    // XInteractionRequest
    virtual uno::Any SAL_CALL getRequest(  ) throw (uno::RuntimeException);
    virtual uno::Sequence< uno::Reference< task::XInteractionContinuation > > SAL_CALL getContinuations(  ) throw (uno::RuntimeException);

    // XInteractionPassword
    virtual void SAL_CALL setPassword( const rtl::OUString& rPwd ) throw (uno::RuntimeException);
    virtual rtl::OUString SAL_CALL getPassword() throw (uno::RuntimeException);

    // XInteractionContinuation
    virtual void SAL_CALL select() throw (uno::RuntimeException);

    bool isSelected() const { osl::MutexGuard const guard( m_aMutex ); return m_bSelected; }

private:
    virtual ~PDFPasswordRequest() {}
};

PDFPasswordRequest::PDFPasswordRequest( bool bFirstTry, const rtl::OUString& rName ) :
    m_aRequest(
        uno::makeAny(
            task::DocumentPasswordRequest(
                OUString(), uno::Reference< uno::XInterface >(),
                task::InteractionClassification_QUERY,
                (bFirstTry
                 ? task::PasswordRequestMode_PASSWORD_ENTER
                 : task::PasswordRequestMode_PASSWORD_REENTER),
                rName))),
    m_bSelected(false)
{}

uno::Any PDFPasswordRequest::getRequest() throw (uno::RuntimeException)
{
    return m_aRequest;
}

uno::Sequence< uno::Reference< task::XInteractionContinuation > > PDFPasswordRequest::getContinuations() throw (uno::RuntimeException)
{
    uno::Sequence< uno::Reference< task::XInteractionContinuation > > aRet( 1 );
    aRet[0] = this;
    return aRet;
}

void PDFPasswordRequest::setPassword( const rtl::OUString& rPwd ) throw (uno::RuntimeException)
{
    osl::MutexGuard const guard( m_aMutex );

    m_aPassword = rPwd;
}

rtl::OUString PDFPasswordRequest::getPassword() throw (uno::RuntimeException)
{
    osl::MutexGuard const guard( m_aMutex );

    return m_aPassword;
}

void PDFPasswordRequest::select() throw (uno::RuntimeException)
{
    osl::MutexGuard const guard( m_aMutex );

    m_bSelected = true;
}

class UnsupportedEncryptionFormatRequest:
    public cppu::WeakImplHelper1< task::XInteractionRequest >,
    private boost::noncopyable
{
public:
    UnsupportedEncryptionFormatRequest() {}

private:
    virtual ~UnsupportedEncryptionFormatRequest() {}

    virtual uno::Any SAL_CALL getRequest() throw (uno::RuntimeException) {
        return uno::makeAny(
            task::ErrorCodeRequest(
                OUString(), uno::Reference< uno::XInterface >(),
                ERRCODE_IO_WRONGVERSION));
            //TODO: should be something more informative than crudely reused
            // ERRCODE_IO_WRONGVERSION
    }

    virtual uno::Sequence< uno::Reference< task::XInteractionContinuation > >
    SAL_CALL getContinuations() throw (uno::RuntimeException) {
        return
            uno::Sequence< uno::Reference< task::XInteractionContinuation > >();
    }
};

} // namespace

namespace pdfi
{

bool getPassword( const uno::Reference< task::XInteractionHandler >& xHandler,
                  rtl::OUString&                                     rOutPwd,
                  bool                                               bFirstTry,
                  const rtl::OUString&                               rDocName
                  )
{
    bool bSuccess = false;

    rtl::Reference< PDFPasswordRequest > xReq(
        new PDFPasswordRequest( bFirstTry, rDocName ) );
    try
    {
        xHandler->handle( xReq.get() );
    }
    catch( uno::Exception& )
    {
    }

    OSL_TRACE( "request %s selected", xReq->isSelected() ? "was" : "was not" );
    if( xReq->isSelected() )
    {
        bSuccess = true;
        rOutPwd = xReq->getPassword();
    }

    return bSuccess;
}

void reportUnsupportedEncryptionFormat(
    uno::Reference< task::XInteractionHandler > const & handler)
{
    assert(handler.is());
    handler->handle(new UnsupportedEncryptionFormatRequest);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
