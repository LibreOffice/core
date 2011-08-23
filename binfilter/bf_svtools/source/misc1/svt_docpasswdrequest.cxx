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

// MARKER(update_precomp.py): autogen include statement, do not remove


#include "docpasswdrequest.hxx"

#ifndef _COM_SUN_STAR_TASK_XINTERACTIONABORT_HPP_
#include <com/sun/star/task/XInteractionAbort.hpp>
#endif
#ifndef _COM_SUN_STAR_TASK_XINTERACTIONPASSWORD_HPP_
#include <com/sun/star/task/XInteractionPassword.hpp>
#endif

namespace binfilter
{

//==========================================================================

class AbortContinuation : public ::cppu::WeakImplHelper1< ::com::sun::star::task::XInteractionAbort >
{
    sal_Bool mbSelected;

public:
    AbortContinuation() : mbSelected( sal_False ) {}
    
    sal_Bool isSelected() { return mbSelected; }
    
    void reset() { mbSelected = sal_False; }

    virtual void SAL_CALL select() throw(::com::sun::star::uno::RuntimeException) { mbSelected = sal_True; }
};

//==========================================================================

class PasswordContinuation : public ::cppu::WeakImplHelper1< ::com::sun::star::task::XInteractionPassword >
{
    sal_Bool mbSelected;
    ::rtl::OUString maPassword;

public:
    PasswordContinuation() : mbSelected( sal_False ) {}
    
    sal_Bool isSelected() { return mbSelected; }
    
    void reset() { mbSelected = sal_False; }

    virtual void SAL_CALL select() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setPassword( const ::rtl::OUString& aPass ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getPassword(  ) throw (::com::sun::star::uno::RuntimeException);
};

void SAL_CALL PasswordContinuation::select()
    throw(::com::sun::star::uno::RuntimeException)
{
    mbSelected = sal_True;
}

void SAL_CALL PasswordContinuation::setPassword( const ::rtl::OUString& aPass )
    throw (::com::sun::star::uno::RuntimeException)
{
    maPassword = aPass;
}

::rtl::OUString SAL_CALL PasswordContinuation::getPassword() 
    throw (::com::sun::star::uno::RuntimeException)
{
    return maPassword;
}

//==========================================================================

RequestDocumentPassword::RequestDocumentPassword( ::com::sun::star::task::PasswordRequestMode nMode, ::rtl::OUString aName )
{
    ::rtl::OUString temp;
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > temp2;
    ::com::sun::star::task::DocumentPasswordRequest 
        aDocumentPasswordRequest( temp,
                            temp2,
                          ::com::sun::star::task::InteractionClassification_QUERY,
                          nMode,
                          aName );

       m_aRequest <<= aDocumentPasswordRequest;

       m_pAbort  = new AbortContinuation;
       m_pPassword = new PasswordContinuation;

       m_lContinuations.realloc( 2 );
       m_lContinuations[0] = ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionContinuation >( m_pAbort  );
       m_lContinuations[1] = ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionContinuation >( m_pPassword );
}

sal_Bool RequestDocumentPassword::isPassword()
{
    return m_pPassword->isSelected();
}
        
::rtl::OUString RequestDocumentPassword::getPassword()
{
    return m_pPassword->getPassword();
}
        
::com::sun::star::uno::Any SAL_CALL RequestDocumentPassword::getRequest()
        throw( ::com::sun::star::uno::RuntimeException )
{
    return m_aRequest;
}

::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionContinuation > >
    SAL_CALL RequestDocumentPassword::getContinuations()
        throw( ::com::sun::star::uno::RuntimeException )
{
    return m_lContinuations;
}

}

