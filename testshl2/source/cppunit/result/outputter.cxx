/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile$
 * $Revision$
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
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
// #include "precompiled_cppunit.hxx"

//
#include <string>
#include <sal/types.h>
#include "testshl/result/outputter.hxx"

// -----------------------------------------------------------------------------

Outputter::~Outputter()
{
    // delete m_pLog;
}

void Outputter::writeToAll(const sal_Char* _sCharStr)
{
    // std::cout << _sCharStr;
    if (m_pStream)
    {
        *m_pStream << _sCharStr;
    }
    if (m_pLog.get())
    {
        m_pLog->write(_sCharStr);
    }
}

void Outputter::write(const sal_Char* _sCharStr)
{
    writeToAll(_sCharStr);
}

void Outputter::write(std::string const& _sStr)
{
    writeToAll(_sStr.c_str());
}

void Outputter::write(sal_Int32 _nValue)
{
    sal_Char cBuf[20];
    sal_Char* pBuf = cBuf;
    sprintf(pBuf, "%d", SAL_STATIC_CAST(int, _nValue));
    writeToAll(pBuf);
}

// -----------------------------------------------------------------------------

Outputter& operator <<( Outputter &_aStreamWrapper, const sal_Char* _sValue)
{
    _aStreamWrapper.write(_sValue);
    return _aStreamWrapper;
}

Outputter& operator <<( Outputter &_aStreamWrapper, std::string const& _sValue)
{
    _aStreamWrapper.write(_sValue);
    return _aStreamWrapper;
}

Outputter& operator <<( Outputter &_aStreamWrapper, sal_Int32 _nValue)
{
    _aStreamWrapper.write(_nValue);
    return _aStreamWrapper;
}

//# Outputter& operator <<( Outputter &_aStreamWrapper, double )
//# {
//#     return _aStreamWrapper;
//# }

Outputter& operator <<( Outputter &_aStreamWrapper, Outputter::endl const&)
{
    _aStreamWrapper.write("\n");
    return _aStreamWrapper;
}
