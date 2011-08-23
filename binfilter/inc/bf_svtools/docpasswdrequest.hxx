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

#ifndef INCLUDED_SVTOOLS_DOCPASSWDREQUEST_HXX
#define INCLUDED_SVTOOLS_DOCPASSWDREQUEST_HXX

#ifndef _COM_SUN_STAR_TASK_DOCUMENTPASSWORDREQUEST_HPP_
#include <com/sun/star/task/DocumentPasswordRequest.hpp>
#endif
#ifndef _COM_SUN_STAR_TASK_XINTERACTIONREQUEST_HPP_
#include <com/sun/star/task/XInteractionRequest.hpp>
#endif

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif

namespace binfilter {

class AbortContinuation;
class PasswordContinuation;

class  RequestDocumentPassword : public ::cppu::WeakImplHelper1< ::com::sun::star::task::XInteractionRequest >
{
    ::com::sun::star::uno::Any m_aRequest;
        
    ::com::sun::star::uno::Sequence< 
                    ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionContinuation > 
                > m_lContinuations;
        
    AbortContinuation*	    m_pAbort;
    PasswordContinuation*	m_pPassword;

public:
    RequestDocumentPassword( ::com::sun::star::task::PasswordRequestMode nMode, ::rtl::OUString aName );
    
    sal_Bool isPassword();
        
    ::rtl::OUString getPassword();
        
    virtual ::com::sun::star::uno::Any SAL_CALL getRequest() 
        throw( ::com::sun::star::uno::RuntimeException );

    virtual ::com::sun::star::uno::Sequence< 
                ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionContinuation > 
            > SAL_CALL getContinuations() 
        throw( ::com::sun::star::uno::RuntimeException );
};  

}

#endif /* INCLUDED_SVTOOLS_DOCPASSWDREQUEST_HXX */
