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

#if ! defined INCLUDED_CLI_BRIDGE_H
#define INCLUDED_CLI_BRIDGE_H
#include <vcclr.h>
#include "osl/interlck.h"
#include "uno/mapping.h"
#include "uno/environment.h"
#include "uno/dispatcher.h"
#include "cli_base.h"
#include "cli_environment.h"
#using <mscorlib.dll>
//#using <cli_uretypes.dll>
#using <cli_basetypes.dll>
#using <system.dll>

namespace sr = System::Reflection;

namespace cli_uno
{


//==== holds environments and mappings =============================================================
struct Bridge;
struct Mapping : public uno_Mapping
{
    Bridge* m_bridge;
};

// The environment will be created in uno_initEnvironment. See also the remarks there
//Managed cli environment for cli objects an UNO proxies (which are cli
//objects. The uno_Environment is not used for cli objects.
__gc struct CliEnvHolder {
static Cli_environment * g_cli_env = NULL;
};

//==================================================================================================
/** An instance of Bridge represents exactly one mapping therefore either
    m_cli2uno or m_uno2cli is valid.
*/
struct Bridge
{
    mutable oslInterlockedCount m_ref;
    uno_ExtEnvironment *        m_uno_env;
    uno_Environment *           m_uno_cli_env;

    Mapping                     m_cli2uno;
    Mapping                     m_uno2cli;
    bool                        m_registered_cli2uno;

    ~Bridge() SAL_THROW( () );
    Bridge( uno_Environment * java_env, uno_ExtEnvironment * uno_env, bool registered_java2uno );

    void acquire() const;
    void release() const;

    void  map_to_uno(
        void * uno_data, System::Object* cli_data,
        typelib_TypeDescriptionReference * type,
        bool assign) const;

    /**
       @param info
       the type of the converted data. It may be a byref type.
     */
    void map_to_cli(
        System::Object* *cli_data, void const * uno_data,
        typelib_TypeDescriptionReference * type, System::Type* info /* maybe 0 */,
        bool bDontCreateObj) const;

    System::Object* map_uno2cli(uno_Interface * pUnoI, typelib_InterfaceTypeDescription* pTD) const;

    System::Object* Bridge::call_uno(uno_Interface * pUnoI,
                      typelib_TypeDescription* member_td,
                      typelib_TypeDescriptionReference * return_type,
                      sal_Int32 nParams, typelib_MethodParameter const * pParams,
                      System::Object * args[], System::Type* argTypes[],
                      System::Object** pException) const;


    void call_cli(
        System::Object* cliI, sr::MethodInfo* method,
        typelib_TypeDescriptionReference * return_type,
        typelib_MethodParameter * params, int nParams,
        void * uno_ret, void * uno_args [], uno_Any ** uno_exc ) const;

    uno_Interface * map_cli2uno(
        System::Object* cliI, typelib_TypeDescription* pTD) const;

};

} //namespace cli_uno


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
