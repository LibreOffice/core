/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: configexcept.cxx,v $
 * $Revision: 1.7.10.3 $
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
#include "precompiled_configmgr.hxx"

#include "configexcept.hxx"
#include <osl/diagnose.h>

namespace configmgr
{
    namespace configuration
    {
//-----------------------------------------------------------------------------

        //---------------------------------------------------------------------
        Exception::Exception(char const* sAsciiMessage)
        : m_sAsciiMessage(sAsciiMessage)
        {
        }
        //---------------------------------------------------------------------
        Exception::Exception(rtl::OString const& sAsciiMessage)
        : m_sAsciiMessage(sAsciiMessage)
        {
        }
        //---------------------------------------------------------------------

        rtl::OUString Exception::message() const
        {

            return rtl::OStringToOUString( m_sAsciiMessage, RTL_TEXTENCODING_ASCII_US );
        }
        //---------------------------------------------------------------------
        char const* Exception::what() const
        {
            return m_sAsciiMessage.getLength() ? m_sAsciiMessage.getStr() : "FAILURE in CONFIGURATION: No description available";
        }
        //---------------------------------------------------------------------

        static const char c_sInvalidNamePre[] = "CONFIGURATION: Invalid path or name: ";
        static const char c_sInvalidName[] = "CONFIGURATION: <Invalid path or name>";
//-----------------------------------------------------------------------------

        //---------------------------------------------------------------------

        InvalidName::InvalidName(rtl::OUString const& sName, char const* sAsciiDescription)
        : Exception( rtl::OString(RTL_CONSTASCII_STRINGPARAM(c_sInvalidName)) += sAsciiDescription )
        , m_sName( sName.concat(rtl::OUString::createFromAscii(sAsciiDescription)) )
        {
        }
        //---------------------------------------------------------------------

        rtl::OUString InvalidName::message() const
        {
            return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(c_sInvalidNamePre)).concat( m_sName );
        }
//-----------------------------------------------------------------------------

        static const char c_sViolation[] = "CONFIGURATION: Update Violates Constraint: ";
        //---------------------------------------------------------------------

        ConstraintViolation::ConstraintViolation(char const* sConstraint)
        : Exception( rtl::OString(RTL_CONSTASCII_STRINGPARAM(c_sViolation)) += sConstraint)
        {
        }

//-----------------------------------------------------------------------------

        static const char c_sTypeMismatch[] = "CONFIGURATION: Data Types do not match: ";
    //---------------------------------------------------------------------
        rtl::OUString TypeMismatch::describe(rtl::OUString const& sFoundType, rtl::OUString const& sExpectedType)
        {
            rtl::OUString sRet = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Found Type: '"));
            sRet += sFoundType;
            if (sExpectedType.getLength() != 0)
            {
                sRet += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("' - Expected Type: '"));
                sRet += sExpectedType;
                sRet += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("'"));
            }
            else
            {
                sRet += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("' is not valid in this context"));
            }
            return sRet;
        }
    //---------------------------------------------------------------------

    //---------------------------------------------------------------------

        TypeMismatch::TypeMismatch(rtl::OUString const& sType1, rtl::OUString const& sType2)
        : Exception( rtl::OString(RTL_CONSTASCII_STRINGPARAM(c_sTypeMismatch)) )
        , m_sTypes( describe(sType1,sType2) )
        {
        }
    //---------------------------------------------------------------------
        TypeMismatch::TypeMismatch(rtl::OUString const& sType1, rtl::OUString const& sType2, char const* sAsciiDescription)
        : Exception( rtl::OString(RTL_CONSTASCII_STRINGPARAM(c_sTypeMismatch)) += sAsciiDescription)
        , m_sTypes( describe(sType1,sType2).concat(rtl::OUString::createFromAscii(sAsciiDescription)) )
        {
        }
     //---------------------------------------------------------------------

        rtl::OUString TypeMismatch::message() const
        {
            return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(c_sTypeMismatch)).concat( m_sTypes );
        }
//-----------------------------------------------------------------------------
    }

    namespace configapi
    {
//-----------------------------------------------------------------------------
        ExceptionMapper::ExceptionMapper(configuration::Exception& e)
        : m_eOriginal(e)
        , m_xContext()
        , m_sMessage(e.message())
        {
        }
        //---------------------------------------------------------------------

        ExceptionMapper::~ExceptionMapper()
        {
        }
        //---------------------------------------------------------------------

        void ExceptionMapper::setContext(uno::XInterface* pContext)
        {
            m_xContext = pContext;
        }
        //---------------------------------------------------------------------

        rtl::OUString ExceptionMapper::message() const
        {
            return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("FAILURE in CONFIGMGR: ")).concat( m_sMessage );
        }
        //---------------------------------------------------------------------

        uno::Reference<uno::XInterface> ExceptionMapper::context() const
        {
            return m_xContext;
        }
        //---------------------------------------------------------------------

        void ExceptionMapper::illegalArgument(sal_Int16 nArgument) throw(lang::IllegalArgumentException)
        {
            throw lang::IllegalArgumentException(message(),context(),nArgument);
        }
        //---------------------------------------------------------------------

        void ExceptionMapper::unhandled() throw(uno::RuntimeException)
        {
            throw uno::RuntimeException(message(),context());
        }
//-----------------------------------------------------------------------------
    }
}
