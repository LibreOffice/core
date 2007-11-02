/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: zipexcptn.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 13:11:26 $
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
#include "precompiled_shell.hxx"

#ifndef GLOBAL_HXX_INCLUDED
#include "internal/global.hxx"
#endif

#ifndef ZIPEXCPTN_HXX_INCLUDED
#include "zipexcptn.hxx"
#endif

//------------------------------------------
/**
*/
RuntimeException::RuntimeException(int Error) :
    m_Error(Error)
{
}

//------------------------------------------
/**
*/
RuntimeException::~RuntimeException() throw()
{
}

//------------------------------------------
/**
*/
int RuntimeException::GetErrorCode() const
{
    return m_Error;
}

//------------------------------------------
/**
*/
ZipException::ZipException(int Error) :
    RuntimeException(Error)
{
}

//------------------------------------------
/**
*/
const char* ZipException::what() const throw()
{
    return 0;
}

//------------------------------------------
/**
*/
Win32Exception::Win32Exception(int Error) :
    RuntimeException(Error),
    m_MsgBuff(0)
{
}

//------------------------------------------
/**
*/
Win32Exception::~Win32Exception() throw()
{
#ifndef OS2
    if (m_MsgBuff)
        LocalFree(m_MsgBuff);
#endif
}

//------------------------------------------
/**
*/
const char* Win32Exception::what() const throw()
{
#ifdef OS2
    return "Win32Exception!";
#else
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        GetErrorCode(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
        (LPTSTR) &m_MsgBuff,
        0,
        NULL);

    return reinterpret_cast<char*>(m_MsgBuff);
#endif
}

//------------------------------------------
/**
*/
ZipContentMissException::ZipContentMissException(int Error) :
    ZipException(Error)
{
}

//------------------------------------------
/**
*/
AccessViolationException::AccessViolationException(int Error) :
    Win32Exception(Error)
{
}

//------------------------------------------
/**
*/
IOException::IOException(int Error) :
    Win32Exception(Error)
{
}
