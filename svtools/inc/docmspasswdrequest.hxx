/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright IBM Corporation 2009.
 * Copyright 2009 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: docmspasswdrequest.hxx,v $
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

#ifndef INCLUDED_SVTOOLS_DOCMSPASSWDREQUEST_HXX
#define INCLUDED_SVTOOLS_DOCMSPASSWDREQUEST_HXX

#include "svtools/svldllapi.h"
#include <com/sun/star/task/DocumentMSPasswordRequest.hpp>
#include <com/sun/star/task/XInteractionRequest.hpp>
#include <rtl/ustring.hxx>
#include <cppuhelper/implbase1.hxx>

class MSAbortContinuation;
class MSPasswordContinuation;

class SVL_DLLPUBLIC RequestMSDocumentPassword : public ::cppu::WeakImplHelper1< ::com::sun::star::task::XInteractionRequest >
{
    ::com::sun::star::uno::Any m_aRequest;

    ::com::sun::star::uno::Sequence<
                    ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionContinuation >
                > m_lContinuations;

    MSAbortContinuation*        m_pAbort;
    MSPasswordContinuation* m_pPassword;

public:
    RequestMSDocumentPassword( ::com::sun::star::task::PasswordRequestMode nMode, ::rtl::OUString aName );

    sal_Bool isAbort();
    sal_Bool isPassword();

    ::rtl::OUString getPassword();

    virtual ::com::sun::star::uno::Any SAL_CALL getRequest()
        throw( ::com::sun::star::uno::RuntimeException );

    virtual ::com::sun::star::uno::Sequence<
                ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionContinuation >
            > SAL_CALL getContinuations()
        throw( ::com::sun::star::uno::RuntimeException );
};

#endif /* INCLUDED_SVTOOLS_DOCMSPASSWDREQUEST_HXX */
