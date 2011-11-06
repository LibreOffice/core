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
#include "precompiled_embeddedobj.hxx"

#include "bitmapcreator.hxx"

#include <vcl/bitmapex.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <tools/stream.hxx>

using namespace ::com::sun::star;

//-------------------------------------------------------------------------
uno::Sequence< ::rtl::OUString > SAL_CALL VCLBitmapCreator::impl_staticGetSupportedServiceNames()
{
    uno::Sequence< ::rtl::OUString > aRet(2);
    aRet[0] = ::rtl::OUString::createFromAscii("com.sun.star.embed.BitmapCreator");
    aRet[1] = ::rtl::OUString::createFromAscii("com.sun.star.comp.embed.BitmapCreator");
    return aRet;
}

//-------------------------------------------------------------------------
::rtl::OUString SAL_CALL VCLBitmapCreator::impl_staticGetImplementationName()
{
    return ::rtl::OUString::createFromAscii("com.sun.star.comp.embed.BitmapCreator");
}

//-------------------------------------------------------------------------
uno::Reference< uno::XInterface > SAL_CALL VCLBitmapCreator::impl_staticCreateSelfInstance(
            const uno::Reference< lang::XMultiServiceFactory >& xServiceManager )
{
    return uno::Reference< uno::XInterface >( *new VCLBitmapCreator( xServiceManager ) );
}

//-------------------------------------------------------------------------

uno::Reference< uno::XInterface > SAL_CALL VCLBitmapCreator::createInstance()
        throw ( uno::Exception,
                uno::RuntimeException)
{
    BitmapEx aBitmap;
    uno::Reference< uno::XInterface> aResult( VCLUnoHelper::CreateBitmap( aBitmap ), uno::UNO_QUERY );

    return aResult;
}

//-------------------------------------------------------------------------
uno::Reference< uno::XInterface > SAL_CALL VCLBitmapCreator::createInstanceWithArguments(
                                                const uno::Sequence< uno::Any >& aArguments )
        throw ( uno::Exception,
                uno::RuntimeException)
{
    if ( aArguments.getLength() != 1 )
        throw uno::Exception(); // TODO

    uno::Sequence< sal_Int8 > aOrigBitmap;
    if ( !( aArguments[0] >>= aOrigBitmap ) )
        throw uno::Exception(); // TODO

    BitmapEx aBitmap;
    SvMemoryStream aStream( aOrigBitmap.getArray(), aOrigBitmap.getLength(), STREAM_READ );
    aStream >> aBitmap;
    if ( aStream.GetError() )
        throw uno::Exception(); // TODO

    uno::Reference< uno::XInterface > aResult( VCLUnoHelper::CreateBitmap( aBitmap ), uno::UNO_QUERY );

    return aResult;
}

//-------------------------------------------------------------------------
::rtl::OUString SAL_CALL VCLBitmapCreator::getImplementationName()
        throw ( uno::RuntimeException )
{
    return impl_staticGetImplementationName();
}

//-------------------------------------------------------------------------
sal_Bool SAL_CALL VCLBitmapCreator::supportsService( const ::rtl::OUString& ServiceName )
        throw ( uno::RuntimeException )
{
    uno::Sequence< ::rtl::OUString > aSeq = impl_staticGetSupportedServiceNames();

    for ( sal_Int32 nInd = 0; nInd < aSeq.getLength(); nInd++ )
        if ( ServiceName.compareTo( aSeq[nInd] ) == 0 )
            return sal_True;

    return sal_False;
}

//-------------------------------------------------------------------------
uno::Sequence< ::rtl::OUString > SAL_CALL VCLBitmapCreator::getSupportedServiceNames()
        throw ( uno::RuntimeException )
{
    return impl_staticGetSupportedServiceNames();
}

