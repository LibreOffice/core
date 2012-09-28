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
#include <cppuhelper/compbase2.hxx>
#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/basemutex.hxx>
#include <tools/errcode.hxx>

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
    task::DocumentPasswordRequest m_aRequest;
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
};

PDFPasswordRequest::PDFPasswordRequest( bool bFirstTry, const rtl::OUString& rName ) :
    PDFPasswordRequestBase( m_aMutex ),
    m_aRequest(),
    m_aPassword(),
    m_bSelected(false)
{
    m_aRequest.Mode = bFirstTry ?
        task::PasswordRequestMode_PASSWORD_ENTER :
        task::PasswordRequestMode_PASSWORD_REENTER;
    m_aRequest.Classification = task::InteractionClassification_QUERY;
    m_aRequest.Name = rName;
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

    PDFPasswordRequest* pRequest;
    uno::Reference< task::XInteractionRequest > xReq(
        pRequest = new PDFPasswordRequest( bFirstTry, rDocName ) );
    try
    {
        xHandler->handle( xReq );
    }
    catch( uno::Exception& )
    {
    }

    OSL_TRACE( "request %s selected", pRequest->isSelected() ? "was" : "was not" );
    if( pRequest->isSelected() )
    {
        bSuccess = true;
        rOutPwd = pRequest->getPassword();
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
