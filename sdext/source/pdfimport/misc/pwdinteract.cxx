/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sdext.hxx"

#include "pdfihelper.hxx"

#include <com/sun/star/task/XInteractionHandler.hpp>
#include <com/sun/star/task/XInteractionRequest.hpp>
#include <com/sun/star/task/XInteractionPassword.hpp>
#include <com/sun/star/task/DocumentPasswordRequest.hpp>

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

    OSL_TRACE( "request %s selected\n", pRequest->isSelected() ? "was" : "was not" );
    if( pRequest->isSelected() )
    {
        bSuccess = true;
        rOutPwd = pRequest->getPassword();
    }

    return bSuccess;
}

}
