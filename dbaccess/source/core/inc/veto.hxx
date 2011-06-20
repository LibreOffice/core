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

#ifndef DBACCESS_VETO_HXX
#define DBACCESS_VETO_HXX

/** === begin UNO includes === **/
#include <com/sun/star/util/XVeto.hpp>
/** === end UNO includes === **/

#include <cppuhelper/implbase1.hxx>

namespace dbaccess
{

    //====================================================================
    //= Veto
    //====================================================================
    typedef ::cppu::WeakImplHelper1 <   ::com::sun::star::util::XVeto
                                    >   Veto_Base;
    /** implements ::com::sun::star::util::XVeto
    */
    class Veto : public Veto_Base
    {
    private:
        const ::rtl::OUString               m_sReason;
        const ::com::sun::star::uno::Any    m_aDetails;

    public:
        Veto( const ::rtl::OUString& _rReason, const ::com::sun::star::uno::Any& _rDetails );

        virtual ::rtl::OUString SAL_CALL getReason() throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Any SAL_CALL getDetails() throw (::com::sun::star::uno::RuntimeException);

    protected:
        ~Veto();

    private:
        Veto();                         // never implemented
        Veto( const Veto& );            // never implemented
        Veto& operator=( const Veto& ); // never implemented
    };

} // namespace dbaccess

#endif // DBACCESS_VETO_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
