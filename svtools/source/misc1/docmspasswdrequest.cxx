/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright IBM Corporation 2009.
 * Copyright 2009 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: docmspasswdrequest.cxx,v $
 * $Revision: 1.0 $
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
#include "precompiled_svtools.hxx"

#include "docmspasswdrequest.hxx"
#include <com/sun/star/task/XInteractionAbort.hpp>
#include <com/sun/star/task/XInteractionPassword.hpp>

//==========================================================================

class MSAbortContinuation : public ::cppu::WeakImplHelper1< ::com::sun::star::task::XInteractionAbort >
{
    sal_Bool mbSelected;

public:
    MSAbortContinuation() : mbSelected( sal_False ) {}

    sal_Bool isSelected() { return mbSelected; }

    void reset() { mbSelected = sal_False; }

    virtual void SAL_CALL select() throw(::com::sun::star::uno::RuntimeException) { mbSelected = sal_True; }
};

//==========================================================================

class MSPasswordContinuation : public ::cppu::WeakImplHelper1< ::com::sun::star::task::XInteractionPassword >
{
    sal_Bool mbSelected;
    ::rtl::OUString maPassword;

public:
    MSPasswordContinuation() : mbSelected( sal_False ) {}

    sal_Bool isSelected() { return mbSelected; }

    void reset() { mbSelected = sal_False; }

    virtual void SAL_CALL select() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setPassword( const ::rtl::OUString& aPass ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getPassword(  ) throw (::com::sun::star::uno::RuntimeException);
};

void SAL_CALL MSPasswordContinuation::select()
    throw(::com::sun::star::uno::RuntimeException)
{
    mbSelected = sal_True;
}

void SAL_CALL MSPasswordContinuation::setPassword( const ::rtl::OUString& aPass )
    throw (::com::sun::star::uno::RuntimeException)
{
    maPassword = aPass;
}

::rtl::OUString SAL_CALL MSPasswordContinuation::getPassword()
    throw (::com::sun::star::uno::RuntimeException)
{
    return maPassword;
}

//==========================================================================

RequestMSDocumentPassword::RequestMSDocumentPassword( ::com::sun::star::task::PasswordRequestMode nMode, ::rtl::OUString aName )
{
    ::rtl::OUString temp;
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > temp2;
    ::com::sun::star::task::DocumentMSPasswordRequest
        aDocumentMSPasswordRequest( temp,
                            temp2,
                          ::com::sun::star::task::InteractionClassification_QUERY,
                          nMode,
                          aName );

       m_aRequest <<= aDocumentMSPasswordRequest;

       m_pAbort  = new MSAbortContinuation;
       m_pPassword = new MSPasswordContinuation;

       m_lContinuations.realloc( 2 );
       m_lContinuations[0] = ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionContinuation >( m_pAbort  );
       m_lContinuations[1] = ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionContinuation >( m_pPassword );
}

sal_Bool RequestMSDocumentPassword::isAbort()
{
    return m_pAbort->isSelected();
}

sal_Bool RequestMSDocumentPassword::isPassword()
{
    return m_pPassword->isSelected();
}

::rtl::OUString RequestMSDocumentPassword::getPassword()
{
    return m_pPassword->getPassword();
}

::com::sun::star::uno::Any SAL_CALL RequestMSDocumentPassword::getRequest()
        throw( ::com::sun::star::uno::RuntimeException )
{
    return m_aRequest;
}

::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionContinuation > >
    SAL_CALL RequestMSDocumentPassword::getContinuations()
        throw( ::com::sun::star::uno::RuntimeException )
{
    return m_lContinuations;
}


