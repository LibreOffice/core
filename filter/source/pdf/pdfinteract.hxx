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

