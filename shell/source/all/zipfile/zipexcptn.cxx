/*************************************************************************
 *
 *  $RCSfile: zipexcptn.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-04-07 10:57:52 $
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
RuntimeException::~RuntimeException()
{
}

//------------------------------------------
/**
*/
RuntimeException::GetErrorCode() const
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
Win32Exception::~Win32Exception()
{
    if (m_MsgBuff)
        LocalFree(m_MsgBuff);
}

//------------------------------------------
/**
*/
const char* Win32Exception::what() const throw()
{
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
