/*************************************************************************
 *
 *  $RCSfile: bmpconv.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-27 17:57:56 $
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

#include <bitmap.hxx>
#include <impbmpconv.hxx>
#include <svapp.hxx>
#include <vos/mutex.hxx>

#ifndef _STREAM_HXX
#include <tools/stream.hxx>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_XINVOCATION_HPP_
#include <com/sun/star/script/XInvocation.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XBITMAP_HPP_
#include <com/sun/star/awt/XBitmap.hpp>
#endif

#ifndef _CPPUHELPER_COMPBASE1_HXX_
#include <cppuhelper/compbase1.hxx>
#endif


using namespace com::sun::star::uno;
using namespace com::sun::star::script;
using namespace com::sun::star::beans;
using namespace com::sun::star::reflection;
using namespace com::sun::star::awt;
using namespace rtl;

namespace vcl {

class BmpTransporter :
        public cppu::WeakImplHelper1< com::sun::star::awt::XBitmap >
{
    Sequence<sal_Int8>          m_aBM;
    com::sun::star::awt::Size   m_aSize;
public:
    BmpTransporter( const Bitmap& rBM );
    virtual  ~BmpTransporter();

    virtual com::sun::star::awt::Size SAL_CALL getSize() throw();
    virtual Sequence< sal_Int8 > SAL_CALL getDIB() throw();
    virtual Sequence< sal_Int8 > SAL_CALL getMaskDIB() throw();
};

class BmpConverter :
        public cppu::WeakImplHelper1< com::sun::star::script::XInvocation >
{
public:
    BmpConverter();
    virtual ~BmpConverter();

    virtual Reference< XIntrospectionAccess > SAL_CALL getIntrospection() throw();
    virtual void SAL_CALL setValue( const OUString& rProperty, const Any& rValue )
        throw( UnknownPropertyException );
    virtual Any SAL_CALL getValue( const OUString& rProperty )
        throw( UnknownPropertyException );
    virtual sal_Bool SAL_CALL hasMethod( const OUString& rName ) throw();
    virtual sal_Bool SAL_CALL hasProperty( const OUString& rProp ) throw();

    virtual Any SAL_CALL invoke( const OUString& rFunction,
                                 const Sequence< Any >& rParams,
                                 Sequence< sal_Int16 >& rOutParamIndex,
                                 Sequence< Any >& rOutParam
                                 )
        throw( CannotConvertException, InvocationTargetException );
};

}

using namespace vcl;

Reference< XInvocation > vcl::createBmpConverter()
{
    return static_cast<XInvocation*>(new BmpConverter());
}

BmpConverter::BmpConverter()
{
}

BmpConverter::~BmpConverter()
{
}

Reference< XIntrospectionAccess > SAL_CALL BmpConverter::getIntrospection() throw()
{
    return Reference< XIntrospectionAccess >();
}

void SAL_CALL BmpConverter::setValue( const OUString& rProperty, const Any& rValue ) throw( UnknownPropertyException )
{
    throw UnknownPropertyException();
}

Any SAL_CALL BmpConverter::getValue( const OUString& rProperty ) throw( UnknownPropertyException )
{
    throw UnknownPropertyException();
}

sal_Bool SAL_CALL BmpConverter::hasMethod( const OUString& rName ) throw()
{
    return rName.equalsIgnoreAsciiCase( OUString::createFromAscii( "convert-bitmap-depth" ) );
}

sal_Bool SAL_CALL BmpConverter::hasProperty( const OUString& rName ) throw()
{
    return sal_False;
}

Any SAL_CALL BmpConverter::invoke(
                                  const OUString& rFunction,
                                  const Sequence< Any >& rParams,
                                  Sequence< sal_Int16 >& rOutParamIndex,
                                  Sequence< Any >& rOutParam )
    throw( CannotConvertException, InvocationTargetException )
{
    Any aRet;

    if( rFunction.equalsIgnoreAsciiCase( OUString::createFromAscii( "convert-bitmap-depth" ) ) )
    {
        Reference< XBitmap > xBM;
        sal_uInt16 nTargetDepth = 0;
        if( rParams.getLength() != 2 )
            throw CannotConvertException();

        if( ! (rParams.getConstArray()[0] >>= xBM ) ||
            ! ( rParams.getConstArray()[1] >>= nTargetDepth ) )
            throw CannotConvertException();

        Sequence< sal_Int8 > aDIB = xBM->getDIB();

        // call into vcl not thread safe
        vos::OGuard aGuard( Application::GetSolarMutex() );

        SvMemoryStream aStream( aDIB.getArray(), aDIB.getLength(), STREAM_READ | STREAM_WRITE );
        Bitmap aBM;
        aBM.Read( aStream, TRUE );
        if( nTargetDepth < 4 )
            nTargetDepth = 1;
        else if( nTargetDepth < 8 )
            nTargetDepth = 4;
        else if( nTargetDepth >8 && nTargetDepth < 24 )
            nTargetDepth = 24;

        if( aBM.GetBitCount() == 24 && nTargetDepth <= 8 )
            aBM.Dither( BMP_DITHER_FLOYD );

        if( aBM.GetBitCount() != nTargetDepth )
        {
            switch( nTargetDepth )
            {
                case 1:     aBM.Convert( BMP_CONVERSION_1BIT_THRESHOLD );break;
                case 4:     aBM.ReduceColors( BMP_CONVERSION_4BIT_COLORS );break;
                case 8:     aBM.ReduceColors( BMP_CONVERSION_8BIT_COLORS );break;
                case 24:    aBM.Convert( BMP_CONVERSION_24BIT );break;
            }
        }
        xBM = new BmpTransporter( aBM );
        aRet <<= xBM;
    }
    else
        throw InvocationTargetException();

    return aRet;
}

BmpTransporter::BmpTransporter( const Bitmap& rBM )
{
    m_aSize.Width = rBM.GetSizePixel().Width();
    m_aSize.Height = rBM.GetSizePixel().Height();
    SvMemoryStream aStream;
    rBM.Write( aStream, FALSE, TRUE );
    m_aBM = Sequence<sal_Int8>((const sal_Int8*)aStream.GetData(), aStream.GetSize() );
}

BmpTransporter::~BmpTransporter()
{
}

com::sun::star::awt::Size SAL_CALL BmpTransporter::getSize() throw()
{
    return m_aSize;
}

Sequence< sal_Int8 > SAL_CALL BmpTransporter::getDIB() throw()
{
    return m_aBM;
}

Sequence< sal_Int8 > SAL_CALL BmpTransporter::getMaskDIB() throw()
{
    return Sequence< sal_Int8 >();
}
