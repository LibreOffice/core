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
#include "precompiled_comphelper.hxx"
#include <comphelper/componentfactory.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#ifndef _COM_SUN_STAR_REGISTRY_XREGISTRYKEY_HDL_
#include <com/sun/star/registry/XRegistryKey.hpp>
#endif
#include <cppuhelper/shlib.hxx>


#ifndef GCC
#endif


using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;
using namespace ::rtl;


namespace comphelper
{

Reference< XInterface > getComponentInstance(
            const OUString & rLibraryName,
            const OUString & rImplementationName
            )
{
    Reference< XInterface > xI;
    Reference< XMultiServiceFactory > xMSF = ::comphelper::getProcessServiceFactory();
    if ( xMSF.is() )
        xI = xMSF->createInstance( rImplementationName );
    if( !xI.is() )
    {
        Reference< XSingleServiceFactory > xSSF =
            loadLibComponentFactory( rLibraryName, rImplementationName,
            Reference< XMultiServiceFactory >(), Reference< XRegistryKey >() );
        if (xSSF.is())
            xI = xSSF->createInstance();
    }
    return xI;
}


Reference< XSingleServiceFactory > loadLibComponentFactory(
            const OUString & rLibName,
            const OUString & rImplName,
            const Reference< XMultiServiceFactory > & xSF,
            const Reference< XRegistryKey > & xKey
            )
{
    return Reference< XSingleServiceFactory >( ::cppu::loadSharedLibComponentFactory(
        rLibName, OUString(), rImplName, xSF, xKey ), UNO_QUERY );
}

}   // namespace comphelper
