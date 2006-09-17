/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: StyleMap.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 10:47:00 $
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
#include "precompiled_xmloff.hxx"

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#include <rtl/uuid.h>
#include <rtl/memory.h>

#include "StyleMap.hxx"

using namespace ::osl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;


StyleMap::StyleMap()
{
}



StyleMap::~StyleMap()
{
}


// XUnoTunnel & co
const Sequence< sal_Int8 > & StyleMap::getUnoTunnelId() throw()
{
    static Sequence< sal_Int8 > * pSeq = 0;
    if( !pSeq )
    {
        Guard< Mutex > aGuard( Mutex::getGlobalMutex() );
        if( !pSeq )
        {
            static Sequence< sal_Int8 > aSeq( 16 );
            rtl_createUuid( reinterpret_cast<sal_uInt8*>( aSeq.getArray() ),
                            0, sal_True );
            pSeq = &aSeq;
        }
    }
    return *pSeq;
}

StyleMap* StyleMap::getImplementation( Reference< XInterface > xInt ) throw()
{
    Reference< XUnoTunnel > xUT( xInt, UNO_QUERY );
    if( xUT.is() )
        return reinterpret_cast<StyleMap *>(
                xUT->getSomething( StyleMap::getUnoTunnelId() ) );
    else
        return 0;
}

// XUnoTunnel
sal_Int64 SAL_CALL StyleMap::getSomething(
        const Sequence< sal_Int8 >& rId )
    throw( RuntimeException )
{
    if( rId.getLength() == 16 &&
        0 == rtl_compareMemory( getUnoTunnelId().getConstArray(),
                                             rId.getConstArray(), 16 ) )
    {
        return reinterpret_cast<sal_Int64>( this );
    }
    return 0;
}


