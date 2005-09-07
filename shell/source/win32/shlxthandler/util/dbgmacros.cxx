/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dbgmacros.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:05:43 $
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

#include <windows.h>
#include <stdio.h>

#ifndef DBGMACROS_HXX_INCLUDED
#include "internal/dbgmacros.hxx"
#endif

void DbgAssert(bool /*condition*/, const char* /*message*/)
{

    //if (!condition)
    //{
    //  char msg[1024];
    //
    //  sprintf(msg, "Assertion in file %s at line %d\n%s", __FILE__, __LINE__, message);

    //  int nRet = MessageBoxA(
    //      0,
    //      msg,
    //      "Assertion violation",
    //      MB_ICONEXCLAMATION | MB_ABORTRETRYIGNORE);
    //}

}

