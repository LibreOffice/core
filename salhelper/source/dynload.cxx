/*************************************************************************
 *
 *  $RCSfile: dynload.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: jl $ $Date: 2001-03-28 11:02:29 $
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

#ifndef _SALHELPER_DYNLOAD_HXX_
#include <salhelper/dynload.hxx>
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

namespace salhelper
{

typedef void*   (SAL_CALL *ApiInitFunction) (void);

ORealDynamicLoader::ORealDynamicLoader(ORealDynamicLoader ** ppSetToZeroInDestructor_,
                       const rtl::OUString& moduleName,
                       const rtl::OUString& initFunction,
                       void* pApi,
                       oslModule pModule)
    : m_pApi(pApi)
    , m_refCount(1)
    , m_pModule(pModule)
    , m_strModuleName(moduleName)
    , m_strInitFunction(initFunction)
    , ppSetToZeroInDestructor( ppSetToZeroInDestructor_ )
{
}

ORealDynamicLoader* ORealDynamicLoader::newInstance(ORealDynamicLoader ** ppSetToZeroInDestructor,
                                  const rtl::OUString& moduleName,
                                  const rtl::OUString& initFunction)
{
    ApiInitFunction initFunc;
    oslModule pModule = osl_loadModule(moduleName.pData, SAL_LOADMODULE_DEFAULT);

    if ( !pModule )
    {
        return NULL;
    }

    initFunc = (ApiInitFunction)osl_getSymbol(pModule, initFunction.pData);

    if ( !initFunc )
    {
        osl_unloadModule(pModule);
        return NULL;
    }

    return(new ORealDynamicLoader(ppSetToZeroInDestructor, moduleName,
                                 initFunction,
                                 initFunc(),
                                 pModule));
}

ORealDynamicLoader::~ORealDynamicLoader()
{
    // set the address to zero
    if( ppSetToZeroInDestructor )
        *ppSetToZeroInDestructor = 0;

    if (m_pModule)
    {
        osl_unloadModule(m_pModule);
        m_pModule = NULL;
    }
}

sal_uInt32 ORealDynamicLoader::acquire()
{
    return ++m_refCount;
}

sal_uInt32 ORealDynamicLoader::release()
{
    sal_uInt32 nRet = --m_refCount;
    if( nRet == 0 )
        delete this;
    return nRet;
}


void* ORealDynamicLoader::getApi() const
{
    return m_pApi;
}

} // namespace salhelper

