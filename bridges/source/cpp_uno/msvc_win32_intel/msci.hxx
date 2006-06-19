/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: msci.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-19 23:45:05 $
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

#pragma warning(push, 1)
#include <windows.h>
#pragma warning(pop)

#include "rtl/ustring.hxx"


class type_info;
typedef struct _uno_Any uno_Any;
typedef struct _uno_Mapping uno_Mapping;

namespace CPPU_CURRENT_NAMESPACE
{

const DWORD MSVC_ExceptionCode = 0xe06d7363;
const long MSVC_magic_number = 0x19930520L;

//==============================================================================
type_info * msci_getRTTI( ::rtl::OUString const & rUNOname );

//==============================================================================
int msci_filterCppException(
    EXCEPTION_POINTERS * pPointers, uno_Any * pUnoExc, uno_Mapping * pCpp2Uno );

//==============================================================================
void msci_raiseException(
    uno_Any * pUnoExc, uno_Mapping * pUno2Cpp );

}

