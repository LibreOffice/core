/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/




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
