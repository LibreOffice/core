/*************************************************************************
 *
 *  $RCSfile: configexcept.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: jb $ $Date: 2000-11-07 14:40:31 $
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

#ifndef CONFIGMGR_CONFIGEXCEPT_HXX_
#define CONFIGMGR_CONFIGEXCEPT_HXX_

#include "apitypes.hxx"
#include <rtl/string.hxx>

#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>

namespace configmgr
{
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

            void illegalArgument(sal_Int16 nArgument = -1) throw(css::lang::IllegalArgumentException);
            void unhandled() throw(uno::RuntimeException);
        };
    //-------------------------------------------------------------------------
    }
//-----------------------------------------------------------------------------
}

#endif // CONFIGMGR_CONFIGEXCEPT_HXX_
