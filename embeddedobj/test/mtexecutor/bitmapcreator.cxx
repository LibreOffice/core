/*************************************************************************
 *
 *  $RCSfile: bitmapcreator.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2004-10-04 19:58:28 $
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

