/*************************************************************************
 *
 *  $RCSfile: module.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: jl $ $Date: 2001-03-19 17:11:38 $
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

#ifndef _RTL_USTRING_
#include <rtl/ustring>
#endif

#ifndef _OSL_MODULE_H_
#include <osl/module.h>
#endif

namespace osl
{

class Module
{
    Module( const Module&);
    Module& operator = ( const Module&);

public:

    Module(): m_Module(0){}

    Module( const ::rtl::OUString& strModuleName, sal_Int32 nRtldMode = SAL_LOADMODULE_DEFAULT)
    {
        load( strModuleName, nRtldMode);
    }

    ~Module()
    {
        osl_unloadModule(m_Module);
    }

    sal_Bool SAL_CALL load( const ::rtl::OUString& strModuleName,
        sal_Int32 nRtldMode = SAL_LOADMODULE_DEFAULT)
    {
        unload();
        m_Module= osl_loadModule( strModuleName.pData, nRtldMode );
        return is();
    }

    void SAL_CALL unload()
    {
       osl_unloadModule(m_Module);
       m_Module = 0;
    }

    sal_Bool SAL_CALL is() const
    {
           return m_Module != NULL;
    }

    void* SAL_CALL getSymbol( const ::rtl::OUString& strSymbolName)
    {
        return ( osl_getSymbol( m_Module, strSymbolName.pData ) );
    }

    operator oslModule() const
    {
        return m_Module;
    }

private:
    oslModule m_Module;

};

}

#endif
