/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef _SMPLMAILMSG_HXX_
#define _SMPLMAILMSG_HXX_

#include <cppuhelper/compbase1.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>

#include <com/sun/star/system/XSimpleMailMessage.hpp>

//----------------------------------------------------------
// class declaration
//----------------------------------------------------------

class CSmplMailMsg :
    public  cppu::WeakImplHelper1< com::sun::star::system::XSimpleMailMessage >
{
public:
    CSmplMailMsg( );

    //------------------------------------------------

    virtual void SAL_CALL setRecipient( const ::rtl::OUString& aRecipient )
        throw (::com::sun::star::uno::RuntimeException);

    virtual ::rtl::OUString SAL_CALL getRecipient(  )
        throw (::com::sun::star::uno::RuntimeException);

    //------------------------------------------------

    virtual void SAL_CALL setCcRecipient( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aCcRecipient )
        throw (::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getCcRecipient(  )
        throw (::com::sun::star::uno::RuntimeException);

    //------------------------------------------------

    virtual void SAL_CALL setBccRecipient( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aBccRecipient )
        throw (::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getBccRecipient(  )
        throw (::com::sun::star::uno::RuntimeException);

    //------------------------------------------------

    virtual void SAL_CALL setOriginator( const ::rtl::OUString& aOriginator )
        throw (::com::sun::star::uno::RuntimeException);

    virtual ::rtl::OUString SAL_CALL getOriginator(  )
        throw (::com::sun::star::uno::RuntimeException);

    //------------------------------------------------

    virtual void SAL_CALL setSubject( const ::rtl::OUString& aSubject )
        throw (::com::sun::star::uno::RuntimeException);

    virtual ::rtl::OUString SAL_CALL getSubject(  )
        throw (::com::sun::star::uno::RuntimeException);

    //------------------------------------------------

    virtual void SAL_CALL setAttachement( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aAttachement )
        throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getAttachement(  )
        throw (::com::sun::star::uno::RuntimeException);

private:
    rtl::OUString                                   m_aRecipient;
    rtl::OUString                                   m_aOriginator;
    rtl::OUString                                   m_aSubject;
    com::sun::star::uno::Sequence< rtl::OUString >  m_CcRecipients;
    com::sun::star::uno::Sequence< rtl::OUString >  m_BccRecipients;
    com::sun::star::uno::Sequence< rtl::OUString >  m_Attachements;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
