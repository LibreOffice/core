/*************************************************************************
 *
 *  $RCSfile: graphic.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2004-06-11 09:50:08 $
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

#ifndef _RTL_UUID_H_
#include <rtl/uuid.h>
#endif
#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif
#ifndef _SV_SVAPP_HXX_
#include <vcl/svapp.hxx>
#endif

#ifndef _COM_SUN_STAR_GRAPHIC_GRAPHICTYPE_HPP_
#include <com/sun/star/graphic/GraphicType.hpp>
#endif

#include <vcl/graph.hxx>
#include "graphic.hxx"

using namespace com::sun::star;

namespace unographic {

// -------------------
// - GraphicProvider -
// -------------------

Graphic::Graphic() :
    mpGraphic( NULL )
{
}

// ------------------------------------------------------------------------------

Graphic::~Graphic()
    throw()
{
    delete mpGraphic;
}

// ------------------------------------------------------------------------------

void Graphic::init( const ::Graphic& rGraphic )
    throw()
{
    delete mpGraphic;
    mpGraphic = new ::Graphic( rGraphic );
    ::unographic::GraphicDescriptor::init( *mpGraphic );
}

// ------------------------------------------------------------------------------

uno::Any SAL_CALL Graphic::queryAggregation( const uno::Type& rType )
    throw( uno::RuntimeException )
{
    uno::Any aAny;

    if( rType == ::getCppuType((const uno::Reference< graphic::XGraphic >*)0) )
        aAny <<= uno::Reference< graphic::XGraphic >( this );
    else if( rType == ::getCppuType((const uno::Reference< lang::XUnoTunnel >*)0) )
        aAny <<= uno::Reference< lang::XUnoTunnel >(this);
    else
        aAny <<= ::unographic::GraphicDescriptor::queryAggregation( rType );

    return aAny ;
}

// ------------------------------------------------------------------------------

uno::Any SAL_CALL Graphic::queryInterface( const uno::Type & rType )
    throw( uno::RuntimeException )
{
    return ::unographic::GraphicDescriptor::queryInterface( rType );
}

// ------------------------------------------------------------------------------

void SAL_CALL Graphic::acquire()
    throw()
{
    ::unographic::GraphicDescriptor::acquire();
}

// ------------------------------------------------------------------------------

void SAL_CALL Graphic::release() throw()
{
    ::unographic::GraphicDescriptor::release();
}

// ------------------------------------------------------------------------------

uno::Sequence< sal_Int8 > SAL_CALL Graphic::getImplementationId_Static()
    throw(uno::RuntimeException)
{
    vos::OGuard                         aGuard( Application::GetSolarMutex() );
    static uno::Sequence< sal_Int8 >    aId;

    if( aId.getLength() == 0 )
    {
        aId.realloc( 16 );
        rtl_createUuid( reinterpret_cast< sal_uInt8* >( aId.getArray() ), 0, sal_True );
    }

    return aId;
}

// ------------------------------------------------------------------------------

::rtl::OUString Graphic::getImplementationName_Static()
    throw()
{
    return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.graphic.Graphic" ) );
}

// ------------------------------------------------------------------------------

uno::Sequence< ::rtl::OUString > Graphic::getSupportedServiceNames_Static()
    throw()
{
    uno::Sequence< ::rtl::OUString > aSeq( 1 );

    aSeq.getArray()[ 0 ] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.graphic.Graphic" ) );

    return aSeq;
}

// ------------------------------------------------------------------------------

::rtl::OUString SAL_CALL Graphic::getImplementationName()
    throw( uno::RuntimeException )
{
    return getImplementationName_Static();
}

// ------------------------------------------------------------------------------

sal_Bool SAL_CALL Graphic::supportsService( const ::rtl::OUString& rServiceName )
    throw( uno::RuntimeException )
{
    if( ::unographic::GraphicDescriptor::supportsService( rServiceName ) )
        return true;
    else
    {
        uno::Sequence< ::rtl::OUString >    aSNL( getSupportedServiceNames() );
        const ::rtl::OUString*              pArray = aSNL.getConstArray();

        for( int i = 0; i < aSNL.getLength(); i++ )
            if( pArray[i] == rServiceName )
                return true;

        return false;
    }
}

// ------------------------------------------------------------------------------

uno::Sequence< ::rtl::OUString > SAL_CALL Graphic::getSupportedServiceNames()
    throw( uno::RuntimeException )
{
    uno::Sequence< ::rtl::OUString >    aRet( ::unographic::GraphicDescriptor::getSupportedServiceNames() );
    uno::Sequence< ::rtl::OUString >    aNew( getSupportedServiceNames_Static() );
    sal_Int32                           nOldCount = aRet.getLength();

    aRet.realloc( nOldCount + aNew.getLength() );

    for( sal_Int32 i = 0; i < aNew.getLength(); ++i )
        aRet[ nOldCount++ ] = aNew[ i ];

    return aRet;
}

// ------------------------------------------------------------------------------

uno::Sequence< uno::Type > SAL_CALL Graphic::getTypes()
    throw(uno::RuntimeException)
{
    uno::Sequence< uno::Type >  aRet( ::unographic::GraphicDescriptor::getTypes() );
    sal_Int32                   nOldCount = aRet.getLength();

    aRet.realloc( nOldCount + 1 );
    aRet[ nOldCount ] = ::getCppuType((const uno::Reference< graphic::XGraphic>*)0);

    return aRet;
}

// ------------------------------------------------------------------------------

uno::Sequence< sal_Int8 > SAL_CALL Graphic::getImplementationId()
    throw(uno::RuntimeException)
{
    return getImplementationId_Static();
}

// ------------------------------------------------------------------------------

::sal_Int8 SAL_CALL Graphic::getType()
     throw (uno::RuntimeException)
{
    ::sal_Int8 cRet = graphic::GraphicType::EMPTY;

    if( mpGraphic && ( mpGraphic->GetType() != GRAPHIC_NONE ) )
        cRet = ( ( mpGraphic->GetType() == GRAPHIC_BITMAP ) ? graphic::GraphicType::PIXEL : graphic::GraphicType::VECTOR );

    return cRet;
}

//----------------------------------------------------------------------
const ::Graphic* Graphic::getImplementation( const uno::Reference< uno::XInterface >& rxIFace )
    throw()
{
    uno::Reference< lang::XUnoTunnel > xTunnel( rxIFace, uno::UNO_QUERY );
    return( xTunnel.is() ? reinterpret_cast< ::Graphic* >( xTunnel->getSomething( getImplementationId_Static() ) ) : NULL );
}

//----------------------------------------------------------------------
sal_Int64 SAL_CALL Graphic::getSomething( const uno::Sequence< sal_Int8 >& rId )
    throw( uno::RuntimeException )
{
    return( ( rId.getLength() == 16 && 0 == rtl_compareMemory( getImplementationId().getConstArray(), rId.getConstArray(), 16 ) ) ?
            reinterpret_cast< sal_Int64 >( mpGraphic ) :
            0 );
}

}
