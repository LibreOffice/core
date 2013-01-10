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
#include "precompiled_vcl.hxx"

#include "vcl/bitmap.hxx"
#include "vcl/svapp.hxx"
#include "vcl/salctype.hxx"
#include "vos/mutex.hxx"
#include "tools/stream.hxx"
#include "com/sun/star/script/XInvocation.hpp"
#include "com/sun/star/awt/XBitmap.hpp"
#include "cppuhelper/compbase1.hxx"
#include <vcl/dibtools.hxx>

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

void SAL_CALL BmpConverter::setValue( const OUString&, const Any& ) throw( UnknownPropertyException )
{
    throw UnknownPropertyException();
}

Any SAL_CALL BmpConverter::getValue( const OUString& ) throw( UnknownPropertyException )
{
    throw UnknownPropertyException();
}

sal_Bool SAL_CALL BmpConverter::hasMethod( const OUString& rName ) throw()
{
    return rName.equalsIgnoreAsciiCase( OUString::createFromAscii( "convert-bitmap-depth" ) );
}

sal_Bool SAL_CALL BmpConverter::hasProperty( const OUString& ) throw()
{
    return sal_False;
}

Any SAL_CALL BmpConverter::invoke(
                                  const OUString& rFunction,
                                  const Sequence< Any >& rParams,
                                  Sequence< sal_Int16 >&,
                                  Sequence< Any >& )
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

        ReadDIB(aBM, aStream, true);

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

    WriteDIB(rBM, aStream, false, true);

    m_aBM = Sequence<sal_Int8>(static_cast<const sal_Int8*>(aStream.GetData()),
                aStream.GetEndOfData());
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
