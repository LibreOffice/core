/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cc50_solaris_sparc.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 22:18:09 $
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

#include <cstddef>

#ifndef _RTL_STRING_HXX_
#include <rtl/string.hxx>
#endif
#include <typeinfo>

typedef struct _uno_Any uno_Any;
typedef struct _uno_Mapping uno_Mapping;

// private C50 structures and functions
namespace __Crun
{
    struct static_type_info
    {
        std::ptrdiff_t m_pClassName;
        int         m_nSkip1; // must be 0
        void*       m_pMagic; // points to some magic data
        int         m_nMagic[ 4 ];
        int         m_nSkip2[2]; // must be 0
    };
    void* ex_alloc(unsigned);
    void ex_throw( void*, const static_type_info*, void(*)(void*));
    void* ex_get();
    void ex_rethrow_q() throw();
}

namespace __Cimpl
{
    const char* ex_name();
}

extern "C" void _ex_register( void*, int );

namespace CPPU_CURRENT_NAMESPACE
{

inline char* adjustPointer( char* pIn, typelib_TypeDescription* pType )
{
    switch( pType->nSize )
    {
        case 1: return pIn + 3;
        case 2: return pIn + 2;
        case 3: return pIn + 1;
            // Huh ? perhaps a char[3] ? Though that would be a pointer
            // well, we have it anyway for symmetry
    }
    return pIn;
}

//##################################################################################################
//#### exceptions ##################################################################################
//##################################################################################################

void cc50_solaris_sparc_raiseException(
    uno_Any * pUnoExc, uno_Mapping * pUno2Cpp );

void cc50_solaris_sparc_fillUnoException(
    void*, const char*,
    uno_Any*, uno_Mapping * pCpp2Uno );

}

