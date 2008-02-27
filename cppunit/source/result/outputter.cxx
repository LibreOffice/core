/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: outputter.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: lla $ $Date: 2008-02-27 16:26:59 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_cppunit.hxx"

//
#include <string>
#include <sal/types.h>
#include "cppunit/result/outputter.hxx"

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
