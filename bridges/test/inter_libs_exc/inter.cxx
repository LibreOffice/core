/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: inter.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-16 16:04:11 $
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
#include "precompiled_bridges.hxx"

#include <stdio.h>
#include "share.h"

#include <rtl/string.hxx>
#include <osl/module.hxx>


using namespace ::rtl;
using namespace ::osl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

extern "C" int main( int argc, char const * argv [] )
{
#ifdef SAL_W32
#define SAL_DLLPREFIX ""
#endif
    Module mod_starter(
        OUSTR(SAL_DLLPREFIX"starter"SAL_DLLEXTENSION),
        SAL_LOADMODULE_LAZY | SAL_LOADMODULE_GLOBAL );
    Module mod_thrower(
        OUSTR(SAL_DLLPREFIX"thrower"SAL_DLLEXTENSION),
        SAL_LOADMODULE_LAZY | SAL_LOADMODULE_GLOBAL );

    typedef t_throws_exc (SAL_CALL * t_get_thrower)();
    t_get_thrower get_thrower = (t_get_thrower)mod_thrower.getSymbol( OUSTR("get_thrower") );
    t_throws_exc thrower = (*get_thrower)();

    typedef void (SAL_CALL * t_starter)( t_throws_exc );
    t_starter start = (t_starter)mod_starter.getSymbol( OUSTR("start") );

    (*start)( thrower );

    return 0;
}
