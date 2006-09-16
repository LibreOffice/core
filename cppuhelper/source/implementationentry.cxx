/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: implementationentry.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-16 12:41:06 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_cppuhelper.hxx"
#ifndef _CPPUHELPER_IMPLEMENATIONENTRY_HXX_
#include <cppuhelper/implementationentry.hxx>
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

using namespace ::rtl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;

namespace cppu {

sal_Bool component_writeInfoHelper(
    void *, void *pRegistryKey , const struct ImplementationEntry entries[] )
{
    sal_Bool bRet = sal_False;
    try
    {
        if( pRegistryKey )
        {
            for( sal_Int32 i = 0; entries[i].create ; i ++ )
            {
                OUStringBuffer buf( 124 );
                buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("/") );
                buf.append( entries[i].getImplementationName() );
                buf.appendAscii(RTL_CONSTASCII_STRINGPARAM( "/UNO/SERVICES" ) );
                Reference< XRegistryKey > xNewKey(
                    reinterpret_cast< XRegistryKey * >( pRegistryKey )->createKey( buf.makeStringAndClear()  ) );

                Sequence< OUString > seq = entries[i].getSupportedServiceNames();
                const OUString *pArray = seq.getConstArray();
                for ( sal_Int32 nPos = 0 ; nPos < seq.getLength(); nPos ++ )
                    xNewKey->createKey( pArray[nPos] );
            }
            bRet = sal_True;
        }
    }
    catch ( InvalidRegistryException & )
    {
        OSL_ENSURE( sal_False, "### InvalidRegistryException!" );
    }
    return bRet;
}


void * component_getFactoryHelper(
    const sal_Char * pImplName, void *, void *,
    const struct ImplementationEntry entries[] )
{

      void * pRet = 0;
    Reference< XSingleComponentFactory > xFactory;

    for( sal_Int32 i = 0 ; entries[i].create ; i ++ )
    {
        OUString implName = entries[i].getImplementationName();
        if( 0 == implName.compareToAscii( pImplName ) )
        {
            xFactory = entries[i].createFactory(
                entries[i].create,
                implName,
                entries[i].getSupportedServiceNames(),
                entries[i].moduleCounter );
        }
    }

    if( xFactory.is() )
    {
        xFactory->acquire();
        pRet = xFactory.get();
    }
    return pRet;
}

}
