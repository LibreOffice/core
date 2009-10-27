/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: docpasswordhelper.cxx,v $
 * $Revision: 1.1 $
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
#include "precompiled_comphelper.hxx"

#include "comphelper/docpasswordhelper.hxx"
#include <com/sun/star/task/XInteractionHandler.hpp>
#include "comphelper/mediadescriptor.hxx"

using ::rtl::OUString;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::UNO_SET_THROW;
using ::com::sun::star::task::PasswordRequestMode;
using ::com::sun::star::task::PasswordRequestMode_PASSWORD_ENTER;
using ::com::sun::star::task::PasswordRequestMode_PASSWORD_REENTER;
using ::com::sun::star::task::XInteractionHandler;
using ::com::sun::star::task::XInteractionRequest;

namespace comphelper {

// ============================================================================

IDocPasswordVerifier::~IDocPasswordVerifier()
{
}

// ============================================================================

/*static*/ OUString DocPasswordHelper::requestAndVerifyDocPassword(
        IDocPasswordVerifier& rVerifier,
        const OUString& rMediaPassword,
        const Reference< XInteractionHandler >& rxInteractHandler,
        const OUString& rDocumentName,
        DocPasswordRequestType eRequestType,
        const ::std::vector< OUString >* pDefaultPasswords,
        bool* pbIsDefaultPassword )
{
    OUString aPassword;
    DocPasswordVerifierResult eResult = DocPasswordVerifierResult_WRONG_PASSWORD;

    // first, try provided default passwords
    if( pbIsDefaultPassword )
        *pbIsDefaultPassword = false;
    if( pDefaultPasswords )
    {
        for( ::std::vector< OUString >::const_iterator aIt = pDefaultPasswords->begin(), aEnd = pDefaultPasswords->end(); (eResult == DocPasswordVerifierResult_WRONG_PASSWORD) && (aIt != aEnd); ++aIt )
        {
            aPassword = *aIt;
            OSL_ENSURE( aPassword.getLength() > 0, "DocPasswordHelper::requestAndVerifyDocPassword - unexpected empty default password" );
            if( aPassword.getLength() > 0 )
            {
                eResult = rVerifier.verifyPassword( aPassword );
                if( pbIsDefaultPassword )
                    *pbIsDefaultPassword = eResult == DocPasswordVerifierResult_OK;
            }
        }
    }

    // try media password (skip, if result is OK or ABORT)
    if( eResult == DocPasswordVerifierResult_WRONG_PASSWORD )
    {
        aPassword = rMediaPassword;
        if( aPassword.getLength() > 0 )
            eResult = rVerifier.verifyPassword( aPassword );
    }

    // request a password (skip, if result is OK or ABORT)
    if( (eResult == DocPasswordVerifierResult_WRONG_PASSWORD) && rxInteractHandler.is() ) try
    {
        PasswordRequestMode eRequestMode = PasswordRequestMode_PASSWORD_ENTER;
        while( eResult == DocPasswordVerifierResult_WRONG_PASSWORD )
        {
            DocPasswordRequest* pRequest = new DocPasswordRequest( eRequestType, eRequestMode, rDocumentName );
            Reference< XInteractionRequest > xRequest( pRequest );
            rxInteractHandler->handle( xRequest );
            if( pRequest->isPassword() )
            {
                aPassword = pRequest->getPassword();
                if( aPassword.getLength() > 0 )
                    eResult = rVerifier.verifyPassword( aPassword );
            }
            else
            {
                eResult = DocPasswordVerifierResult_ABORT;
            }
            eRequestMode = PasswordRequestMode_PASSWORD_REENTER;
        }
    }
    catch( Exception& )
    {
    }

    return (eResult == DocPasswordVerifierResult_OK) ? aPassword : OUString();
}

/*static*/ OUString DocPasswordHelper::requestAndVerifyDocPassword(
        IDocPasswordVerifier& rVerifier,
        MediaDescriptor& rMediaDesc,
        DocPasswordRequestType eRequestType,
        const ::std::vector< OUString >* pDefaultPasswords )
{
    OUString aMediaPassword = rMediaDesc.getUnpackedValueOrDefault(
        MediaDescriptor::PROP_PASSWORD(), OUString() );
    Reference< XInteractionHandler > xInteractHandler = rMediaDesc.getUnpackedValueOrDefault(
        MediaDescriptor::PROP_INTERACTIONHANDLER(), Reference< XInteractionHandler >() );
    OUString aDocumentName = rMediaDesc.getUnpackedValueOrDefault(
        MediaDescriptor::PROP_URL(), OUString() );

    bool bIsDefaultPassword = false;
    OUString aPassword = requestAndVerifyDocPassword(
        rVerifier, aMediaPassword, xInteractHandler, aDocumentName, eRequestType, pDefaultPasswords, &bIsDefaultPassword );

    // insert valid password into media descriptor (but not a default password)
    if( (aPassword.getLength() > 0) && !bIsDefaultPassword )
        rMediaDesc[ MediaDescriptor::PROP_PASSWORD() ] <<= aPassword;

    return aPassword;
}

// ============================================================================

} // namespace comphelper

