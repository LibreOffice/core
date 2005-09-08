/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: componentfactory.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 02:54:42 $
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

#ifndef _COMPHELPER_COMPONENTFACTORY_HXX
#include <comphelper/componentfactory.hxx>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

#ifndef _COM_SUN_STAR_REGISTRY_XREGISTRYKEY_HDL_
#include <com/sun/star/registry/XRegistryKey.hpp>
#endif

#ifndef _CPPUHELPER_SHLIB_HXX_
#include <cppuhelper/shlib.hxx>
#endif


#ifndef GCC
#pragma hdrstop
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
