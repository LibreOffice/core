/*************************************************************************
 *
 *  $RCSfile: docpasswdrequest.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: mav $ $Date: 2002-10-31 11:23:40 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _COM_SUN_STAR_TASK_DOCUMENTPASSWORDREQUEST_HPP_
#include <com/sun/star/task/DocumentPasswordRequest.hpp>
#endif
#ifndef _COM_SUN_STAR_TASK_XINTERACTIONPASSWORD_HPP_
#include <com/sun/star/task/XInteractionPassword.hpp>
#endif
#ifndef _COM_SUN_STAR_TASK_XINTERACTIONREQUEST_HPP_
#include <com/sun/star/task/XInteractionRequest.hpp>
#endif
#ifndef _COM_SUN_STAR_TASK_XINTERACTIONABORT_HPP_
#include <com/sun/star/task/XInteractionAbort.hpp>
#endif

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif


class AbortContinuation : public ::cppu::WeakImplHelper1< ::com::sun::star::task::XInteractionAbort >
{
    sal_Bool mbSelected;

public:
    AbortContinuation() : mbSelected( sal_False ) {}

    sal_Bool isSelected() { return mbSelected; }

    void reset() { mbSelected = sal_False; }

    virtual void SAL_CALL select() throw(::com::sun::star::uno::RuntimeException) { mbSelected = sal_True; }
};

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

class RequestDocumentPassword : public ::cppu::WeakImplHelper1< ::com::sun::star::task::XInteractionRequest >
{
    ::com::sun::star::uno::Any m_aRequest;

    ::com::sun::star::uno::Sequence<
                    ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionContinuation >
                > m_lContinuations;

    AbortContinuation*  m_pAbort;

    PasswordContinuation*   m_pPassword;

public:
    RequestDocumentPassword( ::com::sun::star::task::PasswordRequestMode nMode, ::rtl::OUString aName );

    sal_Bool    isAbort() { return m_pAbort->isSelected(); }
    sal_Bool    isPassword() { return m_pPassword->isSelected(); }

    ::rtl::OUString getPassword()
    {
        return m_pPassword->getPassword();
    }

    virtual ::com::sun::star::uno::Any SAL_CALL getRequest()
        throw( ::com::sun::star::uno::RuntimeException );

    virtual ::com::sun::star::uno::Sequence<
                ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionContinuation >
            > SAL_CALL getContinuations()
        throw( ::com::sun::star::uno::RuntimeException );
};

