/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: module.h,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: obo $ $Date: 2006-01-20 13:30:42 $
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

/** @HTML */

#ifndef _OSL_MODULE_H_
#define _OSL_MODULE_H_

#ifndef _RTL_USTRING_H
#   include <rtl/ustring.h>
#endif

#ifndef _RTL_TENCINFO_H
#   include <rtl/tencinfo.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef SAL_DLLPREFIX
#define SAL_MODULENAME(name) SAL_DLLPREFIX name SAL_DLLEXTENSION
#else
#define SAL_MODULENAME(name) name SAL_DLLEXTENSION
#endif

#ifdef SAL_W32
#define SAL_MODULENAME_WITH_VERSION(name, version) name version SAL_DLLEXTENSION

#elif defined(SAL_UNX)
#if defined(MACOSX)
#define SAL_MODULENAME_WITH_VERSION(name, version) SAL_DLLPREFIX name ".dylib." version
#else
#define SAL_MODULENAME_WITH_VERSION(name, version) SAL_DLLPREFIX name SAL_DLLEXTENSION "." version
#endif

#endif

#if defined (MACOSX)
  #define SAL_LOADMODULE_GLOBAL 0x8
#else
  #define SAL_LOADMODULE_GLOBAL 0x100
#endif

#define SAL_LOADMODULE_DEFAULT    0x00000
#define SAL_LOADMODULE_LAZY       0x00001
#define SAL_LOADMODULE_NOW        0x00002

typedef void* oslModule;

/** Generic Function pointer type that will be used as symbol address.
    @see osl_getFunctionSymbol.
    @see osl_getModuleURLFromFunctionAddress.
*/
typedef void ( SAL_CALL *oslGenericFunction )( void );

/** Load a shared library or module.
    @param strModuleName denotes the name of the module to be loaded.
    @return NULL if the module could not be loaded, otherwise a handle to the module.
*/
oslModule SAL_CALL osl_loadModule(rtl_uString *strModuleName, sal_Int32 nRtldMode);

/** Release the module
*/
void SAL_CALL osl_unloadModule(oslModule Module);

/** lookup the specified symbol name.
    @return address of the symbol or NULL if lookup failed.
*/
void* SAL_CALL osl_getSymbol( oslModule Module, rtl_uString *strSymbolName);

/** Lookup the specified function symbol name.

    osl_getFunctionSymbol is an alternative function for osl_getSymbol.
    Use Function pointer as symbol address to conceal type conversion.

    @param Module
    [in] the handle of the Module.

    @param ustrFunctionSymbolName
    [in] Name of the function that will be looked up.

    @return
    <dl>
    <dt>Function address.</dt>
    <dd>on success</dd>
    <dt>NULL</dt>
    <dd>lookup failed or the parameter are invalid.</dd>
    </dl>

    @see osl_getSymbol
*/
oslGenericFunction SAL_CALL osl_getFunctionSymbol( oslModule Module, rtl_uString *ustrFunctionSymbolName );

/** Lookup URL of module which is mapped at the specified address.
    @param pv specifies an address in the process memory space.
    @param pustrURL receives the URL of the module that is mapped at pv.
    @return sal_True on success, sal_False if no module can be found at the specified address.
*/
sal_Bool SAL_CALL osl_getModuleURLFromAddress( void *pv, rtl_uString **pustrURL );

/** Lookup URL of module which is mapped at the specified function address.

    osl_getModuleURLFromFunctionAddress is an alternative function for osl_getModuleURLFromAddress.
    Use Function pointer as symbol address to conceal type conversion.

    @param pf
    [in] function address in oslGenericFunction format.

    @param pustrFunctionURL
    [out] receives the URL of the module that is mapped at pf.

    @return
    <dl>
    <dt>sal_True</dt>
    <dd>on success</dd>
    <dt>sal_False</dt>
    <dd>no module can be found at the specified function address or parameter is somewhat invalid.</dd>
    </dl>

    @see osl_getModuleURLFromAddress
*/
sal_Bool SAL_CALL osl_getModuleURLFromFunctionAddress( oslGenericFunction pf, rtl_uString **pustrFunctionURL );

#ifdef __cplusplus
}
#endif

#endif  /* _OSL_MODULE_H_  */
