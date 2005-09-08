/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: processfactory.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 02:55:10 $
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

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace osl;

namespace comphelper
{

/*
    This function preserves only that the xProcessFactory variable will not be create when
    the library is loaded.
*/
Reference< XMultiServiceFactory > localProcessFactory( const Reference< XMultiServiceFactory >& xSMgr, sal_Bool bSet )
{
    Guard< Mutex > aGuard( Mutex::getGlobalMutex() );

    static Reference< XMultiServiceFactory > xProcessFactory;
    if ( bSet )
    {
        xProcessFactory = xSMgr;
    }

    return xProcessFactory;
}


void setProcessServiceFactory(const Reference< XMultiServiceFactory >& xSMgr)
{
    localProcessFactory( xSMgr, sal_True );
}

Reference< XMultiServiceFactory > getProcessServiceFactory()
{
    Reference< XMultiServiceFactory> xReturn;
    xReturn = localProcessFactory( xReturn, sal_False );
    return xReturn;
}

Reference< XInterface > createProcessComponent( const ::rtl::OUString& _rServiceSpecifier ) SAL_THROW( ( RuntimeException ) )
{
    Reference< XInterface > xComponent;

    Reference< XMultiServiceFactory > xFactory( getProcessServiceFactory() );
    if ( xFactory.is() )
        xComponent = xFactory->createInstance( _rServiceSpecifier );

    return xComponent;
}

Reference< XInterface > createProcessComponentWithArguments( const ::rtl::OUString& _rServiceSpecifier,
        const Sequence< Any >& _rArgs ) SAL_THROW( ( RuntimeException ) )
{
    Reference< XInterface > xComponent;

    Reference< XMultiServiceFactory > xFactory( getProcessServiceFactory() );
    if ( xFactory.is() )
        xComponent = xFactory->createInstanceWithArguments( _rServiceSpecifier, _rArgs );

    return xComponent;
}

} // namesapce comphelper

