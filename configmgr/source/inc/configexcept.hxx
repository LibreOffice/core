/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: configexcept.hxx,v $
 * $Revision: 1.6.10.2 $
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

#ifndef CONFIGMGR_CONFIGEXCEPT_HXX_
#define CONFIGMGR_CONFIGEXCEPT_HXX_

#include <rtl/ustring.hxx>
#include <rtl/string.hxx>
#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>

namespace configmgr
{
//-------------------------------------------------------------------------
    namespace uno  = ::com::sun::star::uno;
    namespace lang = ::com::sun::star::lang;
//-----------------------------------------------------------------------------
    namespace configuration
    {
    //-------------------------------------------------------------------------

        class Exception
        {
            rtl::OString m_sAsciiMessage;
        public:
            Exception(char const* sAsciiMessage);
            Exception(rtl::OString const& sAsciiMessage);
            virtual ~Exception() {}

            virtual rtl::OUString message() const;
            virtual char const* what() const;
        };
    //-------------------------------------------------------------------------

        class InvalidName : public Exception
        {
            rtl::OUString m_sName;
        public:
            InvalidName(rtl::OUString const& sName, char const* sAsciiDescription);

            virtual rtl::OUString message() const;
        };
    //-------------------------------------------------------------------------

        class ConstraintViolation
        : public Exception
        {
        public:
            ConstraintViolation(char const* sConstraint);
        };
    //-------------------------------------------------------------------------

        class TypeMismatch : public Exception
        {
            rtl::OUString m_sTypes;
            static rtl::OUString describe(rtl::OUString const& sFoundType, rtl::OUString const& sExpectedType);
        public:
            TypeMismatch(rtl::OUString const& sFoundType, rtl::OUString const& sExpectedType);
            TypeMismatch(rtl::OUString const& sFoundType, rtl::OUString const& sExpectedType, char const* sAsciiDescription);

            virtual rtl::OUString message() const;
        };
    //-------------------------------------------------------------------------
    }

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
    namespace configapi
    {
    //-------------------------------------------------------------------------
        class ExceptionMapper
        {
            configuration::Exception& m_eOriginal;
            uno::Reference<uno::XInterface> m_xContext;
            rtl::OUString m_sMessage;
        public:
            ExceptionMapper(configuration::Exception& e);
            ~ExceptionMapper();

            void setContext(uno::XInterface* pContext);

            rtl::OUString message() const;
            uno::Reference<uno::XInterface> context() const;

            void illegalArgument(sal_Int16 nArgument = -1) throw(lang::IllegalArgumentException);
            void unhandled() throw(uno::RuntimeException);
        };
    //-------------------------------------------------------------------------
    }
//-----------------------------------------------------------------------------
}

#endif // CONFIGMGR_CONFIGEXCEPT_HXX_
