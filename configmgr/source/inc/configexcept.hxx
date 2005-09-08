/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: configexcept.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 03:45:25 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef CONFIGMGR_CONFIGEXCEPT_HXX_
#define CONFIGMGR_CONFIGEXCEPT_HXX_

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#ifndef _RTL_STRING_HXX_
#include <rtl/string.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_EXCEPTION_HPP_
#include <com/sun/star/uno/Exception.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_ILLEGALARGUMENTEXCEPTION_HPP_
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#endif

namespace configmgr
{
//-------------------------------------------------------------------------
    namespace uno  = ::com::sun::star::uno;
    namespace lang = ::com::sun::star::lang;
    using rtl::OUString;
//-----------------------------------------------------------------------------
    namespace configuration
    {
    //-------------------------------------------------------------------------

        class Exception
        {
            rtl::OString m_sAsciiMessage;
        public:
            Exception();
            Exception(char const* sAsciiMessage);
            Exception(rtl::OString const& sAsciiMessage);
            virtual ~Exception() {}

            virtual OUString message() const;
            virtual char const* what() const;
        };
    //-------------------------------------------------------------------------

        class InvalidName : public Exception
        {
            rtl::OUString m_sName;
        public:
            InvalidName(OUString const& sName);
            InvalidName(OUString const& sName, char const* sAsciiDescription);

            virtual OUString message() const;
        };
    //-------------------------------------------------------------------------

        class ConstraintViolation
        : public Exception
        {
        public:
            ConstraintViolation();
            ConstraintViolation(char const* sConstraint);
        };
    //-------------------------------------------------------------------------

        class TypeMismatch : public Exception
        {
            OUString m_sTypes;
            static OUString describe(OUString const& sFoundType, OUString const& sExpectedType);
        public:
            TypeMismatch();
            TypeMismatch(OUString const& sFoundType);
            TypeMismatch(OUString const& sFoundType, OUString const& sExpectedType);
            TypeMismatch(OUString const& sFoundType, OUString const& sExpectedType, char const* sAsciiDescription);

            virtual OUString message() const;
        };
    //-------------------------------------------------------------------------

        class WrappedUnoException : public Exception
        {
            uno::Any m_aUnoException;
        public:
            WrappedUnoException(uno::Any const& aUnoException);

            OUString extractMessage() const;
            uno::Exception extractUnoException() const;
            uno::Any const& getAnyUnoException() const;

            virtual OUString message() const;
        };
        template <class Except>
        WrappedUnoException rethrowWrapped(Except const& anException)
        {
            throw WrappedUnoException( uno::makeAny(anException) );
        }
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
            OUString m_sMessage;
        public:
            ExceptionMapper(configuration::Exception& e);
            ~ExceptionMapper();

            void setContext(uno::XInterface* pContext);

            OUString message() const;
            uno::Reference<uno::XInterface> context() const;

            void illegalArgument(sal_Int16 nArgument = -1) throw(lang::IllegalArgumentException);
            void unhandled() throw(uno::RuntimeException);
        };
    //-------------------------------------------------------------------------
    }
//-----------------------------------------------------------------------------
}

#endif // CONFIGMGR_CONFIGEXCEPT_HXX_
