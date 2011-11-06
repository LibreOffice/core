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



#ifndef PDFINTERACT_HXX
#define PDFINTERACT_HXX

#include "com/sun/star/lang/XServiceInfo.hpp"
#include "cppuhelper/implbase2.hxx"
#include "com/sun/star/lang/XMultiServiceFactory.hpp"
#include "com/sun/star/task/XInteractionHandler2.hpp"

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;

// -------------
// - PDFFilter -
// -------------

class PDFInteractionHandler : public cppu::WeakImplHelper2 < task::XInteractionHandler2,
                                                             XServiceInfo >
{
private:

    Reference< XMultiServiceFactory >   mxMSF;

protected:
    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw(RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(RuntimeException);
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames()  throw(RuntimeException);

    // XIniteractionHandler
    virtual void SAL_CALL handle( const Reference< task::XInteractionRequest >& ) throw(RuntimeException);

    // XIniteractionHandler2
    virtual sal_Bool SAL_CALL handleInteractionRequest( const Reference< task::XInteractionRequest >& ) throw(RuntimeException);
public:

                PDFInteractionHandler( const Reference< XMultiServiceFactory >& rxMSF );
    virtual     ~PDFInteractionHandler();
};

// -----------------------------------------------------------------------------

OUString PDFInteractionHandler_getImplementationName ()
    throw ( RuntimeException );

// -----------------------------------------------------------------------------

sal_Bool SAL_CALL PDFInteractionHandler_supportsService( const OUString& ServiceName )
    throw ( RuntimeException );

// -----------------------------------------------------------------------------

Sequence< OUString > SAL_CALL PDFInteractionHandler_getSupportedServiceNames(  )
    throw ( RuntimeException );

// -----------------------------------------------------------------------------

Reference< XInterface >
SAL_CALL PDFInteractionHandler_createInstance( const Reference< XMultiServiceFactory > & rSMgr)
    throw ( Exception );

#endif // PDFINTERACT_HXX

