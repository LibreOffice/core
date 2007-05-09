/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cli_bridge.h,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2007-05-09 13:30:57 $
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
#using <cli_types.dll>
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
