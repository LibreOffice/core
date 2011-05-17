/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
