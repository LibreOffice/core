/*************************************************************************
 *
 *  $RCSfile: module.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: obr $ $Date: 2001-11-12 15:51:50 $
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

#ifndef _OSL_MODULE_HXX_
#define _OSL_MODULE_HXX_

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#ifndef _OSL_MODULE_H_
#include <osl/module.h>
#endif

namespace osl
{

/** A class representing a share library module
*/
class Module
{
    Module( const Module&);
    Module& operator = ( const Module&);

public:
    /** Lookup URL of module which is mapped at the specified address.
        @param  addr specifies an address in the process memory space
        @param  pustrURL receives the URL of the module that is mapped at pv
        @return sal_True on success, sal_False if no module can be found at the specified address
    */
    static sal_Bool getUrlFromAddress(void * addr, ::rtl::OUString & libraryUrl) {
        return osl_getModuleURLFromAddress(addr, &libraryUrl.pData);
    }


    /** Constructs an uninitialized module object.
    */
    Module(): m_Module(0){}

    /** Contructs the module object and loads the shared library or module.
        @param strModuleName the module to load
        @seealso ::osl_loadModule()
    */
    Module( const ::rtl::OUString& strModuleName, sal_Int32 nRtldMode = SAL_LOADMODULE_DEFAULT)
    {
        load( strModuleName, nRtldMode);
    }

    /** Unloads the module and destructs the object
        @seealso ::osl_unloadModule()
    */
    ~Module()
    {
        osl_unloadModule(m_Module);
    }

    /** Loads the shared library or module.

        A previously loaded module will be unloaded before the new one gets loaded.
        @param strModuleName the module to load
        @seealso ::osl_loadModule()
    */
    sal_Bool SAL_CALL load( const ::rtl::OUString& strModuleName,
        sal_Int32 nRtldMode = SAL_LOADMODULE_DEFAULT)
    {
        unload();
        m_Module= osl_loadModule( strModuleName.pData, nRtldMode );
        return is();
    }

    /** Unloads the module.
        @seealso ::osl_unloadModule()
    */
    void SAL_CALL unload()
    {
        if (m_Module)
        {
            osl_unloadModule(m_Module);
            m_Module = 0;
        }
    }

    /** Checks if a module is loaded.
        @return sal_True if the module is loaded, sal_False if not
    */
    sal_Bool SAL_CALL is() const
    {
           return m_Module != NULL;
    }

    /** Extract a symbol from the module.
        @param strSymbolName the symbol to extract.
        @return the address of the symbol
        @seealso ::osl_getSymbol()
    */
    void* SAL_CALL getSymbol( const ::rtl::OUString& strSymbolName)
    {
        return ( osl_getSymbol( m_Module, strSymbolName.pData ) );
    }

    /** Cast operator to oslModule.
        @return the oslModule handle
    */
    operator oslModule() const
    {
        return m_Module;
    }

private:
    oslModule m_Module;

};

}

#endif
