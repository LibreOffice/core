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

#ifndef DBACCESS_PROGRESSCAPTURE_HXX
#define DBACCESS_PROGRESSCAPTURE_HXX

/** === begin UNO includes === **/
#include <com/sun/star/task/XStatusIndicator.hpp>
/** === end UNO includes === **/

#include <cppuhelper/implbase1.hxx>

#include <memory>

//........................................................................
namespace dbmm
{
//........................................................................

    class IMigrationProgress;

    //====================================================================
    //= ProgressCapture
    //====================================================================
    typedef ::cppu::WeakImplHelper1 <   ::com::sun::star::task::XStatusIndicator
                                    >   ProgressCapture_Base;

    struct ProgressCapture_Data;

    class ProgressCapture : public ProgressCapture_Base
    {
    public:
        ProgressCapture( const ::rtl::OUString& _rObjectName, IMigrationProgress& _rMasterProgress );

        void    dispose();

        // XStatusIndicator
        virtual void SAL_CALL start( const ::rtl::OUString& Text, ::sal_Int32 Range ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL end(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setText( const ::rtl::OUString& Text ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setValue( ::sal_Int32 Value ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL reset(  ) throw (::com::sun::star::uno::RuntimeException);

    protected:
        virtual ~ProgressCapture();

    private:
        ::std::auto_ptr< ProgressCapture_Data > m_pData;
    };

//........................................................................
} // namespace dbmm
//........................................................................

#endif // DBACCESS_PROGRESSCAPTURE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
