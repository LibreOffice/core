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

#include <sal/config.h>

#include <cassert>

#include <pdfihelper.hxx>

#include <com/sun/star/task/ErrorCodeRequest.hpp>
#include <com/sun/star/task/XInteractionHandler.hpp>
#include <com/sun/star/task/XInteractionRequest.hpp>
#include <com/sun/star/task/XInteractionPassword.hpp>
#include <com/sun/star/task/DocumentPasswordRequest.hpp>

#include <cppuhelper/implbase.hxx>
#include <osl/mutex.hxx>
#include <rtl/ref.hxx>
#include <vcl/errcode.hxx>

using namespace com::sun::star;

namespace
{

class PDFPasswordRequest:
    public cppu::WeakImplHelper<
        task::XInteractionRequest, task::XInteractionPassword >
{
private:
    mutable osl::Mutex            m_aMutex;
    uno::Any                      m_aRequest;
    OUString                 m_aPassword;
    bool                          m_bSelected;

public:
    explicit PDFPasswordRequest(bool bFirstTry, const OUString& rName);
    PDFPasswordRequest(const PDFPasswordRequest&) = delete;
    PDFPasswordRequest& operator=(const PDFPasswordRequest&) = delete;

    // XInteractionRequest
    virtual uno::Any SAL_CALL getRequest(  ) override;
    virtual uno::Sequence< uno::Reference< task::XInteractionContinuation > > SAL_CALL getContinuations(  ) override;

    // XInteractionPassword
    virtual void SAL_CALL setPassword( const OUString& rPwd ) override;
    virtual OUString SAL_CALL getPassword() override;

    // XInteractionContinuation
    virtual void SAL_CALL select() override;

    bool isSelected() const { osl::MutexGuard const guard( m_aMutex ); return m_bSelected; }

private:
    virtual ~PDFPasswordRequest() override {}
};

PDFPasswordRequest::PDFPasswordRequest( bool bFirstTry, const OUString& rName ) :
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

uno::Any PDFPasswordRequest::getRequest()
{
    return m_aRequest;
}

uno::Sequence< uno::Reference< task::XInteractionContinuation > > PDFPasswordRequest::getContinuations()
{
    uno::Sequence< uno::Reference< task::XInteractionContinuation > > aRet( 1 );
    aRet[0] = this;
    return aRet;
}

void PDFPasswordRequest::setPassword( const OUString& rPwd )
{
    osl::MutexGuard const guard( m_aMutex );

    m_aPassword = rPwd;
}

OUString PDFPasswordRequest::getPassword()
{
    osl::MutexGuard const guard( m_aMutex );

    return m_aPassword;
}

void PDFPasswordRequest::select()
{
    osl::MutexGuard const guard( m_aMutex );

    m_bSelected = true;
}

class UnsupportedEncryptionFormatRequest:
    public cppu::WeakImplHelper< task::XInteractionRequest >
{
public:
    UnsupportedEncryptionFormatRequest() {}
    UnsupportedEncryptionFormatRequest(const UnsupportedEncryptionFormatRequest&) = delete;
    UnsupportedEncryptionFormatRequest& operator=(const UnsupportedEncryptionFormatRequest&) = delete;

private:
    virtual ~UnsupportedEncryptionFormatRequest() override {}

    virtual uno::Any SAL_CALL getRequest() override {
        return uno::makeAny(
            task::ErrorCodeRequest(
                OUString(), uno::Reference< uno::XInterface >(),
                sal_uInt32(ERRCODE_IO_WRONGVERSION)));
            //TODO: should be something more informative than crudely reused
            // ERRCODE_IO_WRONGVERSION
    }

    virtual uno::Sequence< uno::Reference< task::XInteractionContinuation > >
    SAL_CALL getContinuations() override {
        return
            uno::Sequence< uno::Reference< task::XInteractionContinuation > >();
    }
};

} // namespace

namespace pdfi
{

bool getPassword( const uno::Reference< task::XInteractionHandler >& xHandler,
                  OUString&                                     rOutPwd,
                  bool                                               bFirstTry,
                  const OUString&                               rDocName
                  )
{
    bool bSuccess = false;

    rtl::Reference< PDFPasswordRequest > xReq(
        new PDFPasswordRequest( bFirstTry, rDocName ) );
    try
    {
        xHandler->handle( xReq );
    }
    catch( uno::Exception& )
    {
    }

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
