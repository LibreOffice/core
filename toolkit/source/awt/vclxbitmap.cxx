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
#include "precompiled_toolkit.hxx"

#include <toolkit/awt/vclxbitmap.hxx>
#include <toolkit/helper/macros.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <tools/stream.hxx>
#include <rtl/memory.h>
#include <rtl/uuid.h>
#include <vcl/dibtools.hxx>

//  ----------------------------------------------------
//  class VCLXBitmap
//  ----------------------------------------------------

// ::com::sun::star::uno::XInterface
::com::sun::star::uno::Any VCLXBitmap::queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aRet = ::cppu::queryInterface( rType,
                                        SAL_STATIC_CAST( ::com::sun::star::awt::XBitmap*, this ),
                                        SAL_STATIC_CAST( ::com::sun::star::awt::XDisplayBitmap*, this ),
                                        SAL_STATIC_CAST( ::com::sun::star::lang::XUnoTunnel*, this ),
                                        SAL_STATIC_CAST( ::com::sun::star::lang::XTypeProvider*, this ) );
    return (aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType ));
}

// ::com::sun::star::lang::XUnoTunnel
IMPL_XUNOTUNNEL( VCLXBitmap )

// ::com::sun::star::lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( VCLXBitmap )
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XBitmap>* ) NULL ),
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XDisplayBitmap>* ) NULL )
IMPL_XTYPEPROVIDER_END


// ::com::sun::star::awt::XBitmap
::com::sun::star::awt::Size VCLXBitmap::getSize() throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    ::com::sun::star::awt::Size aSize( maBitmap.GetSizePixel().Width(), maBitmap.GetSizePixel().Height() );
    return aSize;
}

::com::sun::star::uno::Sequence< sal_Int8 > VCLXBitmap::getDIB() throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    SvMemoryStream aMem;
    WriteDIB(maBitmap.GetBitmap(), aMem, false, true);
    return ::com::sun::star::uno::Sequence<sal_Int8>( (sal_Int8*) aMem.GetData(), aMem.Tell() );
}

::com::sun::star::uno::Sequence< sal_Int8 > VCLXBitmap::getMaskDIB() throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    SvMemoryStream aMem;
    WriteDIB(maBitmap.GetMask(), aMem, false, true);
    return ::com::sun::star::uno::Sequence<sal_Int8>( (sal_Int8*) aMem.GetData(), aMem.Tell() );
}

// eof
