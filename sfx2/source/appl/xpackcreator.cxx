/*************************************************************************
 *
 *  $RCSfile: xpackcreator.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2004-02-25 15:43:04 $
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

#ifndef _COM_SUN_STAR_UCB_XCOMMANDENVIRONMENT_HPP_
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#endif

#ifndef _COM_SUN_STAR_IO_XOUTPUTSTREAM_HPP_
#include <com/sun/star/io/XOutputStream.hpp>
#endif


#include "xpackcreator.hxx"

#include <sot/stg.hxx>
#include <sot/storage.hxx>
#include <tools/stream.hxx>
#include <unotools/tempfile.hxx>
#include <unotools/ucbhelper.hxx>
#include <ucbhelper/content.hxx>
#include <ucbhelper/commandenvironment.hxx>

using namespace ::com::sun::star;

//-------------------------------------------------------------------------
uno::Sequence< ::rtl::OUString > SAL_CALL OPackageStructureCreator::impl_getStaticSupportedServiceNames()
{
    uno::Sequence< ::rtl::OUString > aRet(2);
    aRet[0] = ::rtl::OUString::createFromAscii("com.sun.star.embed.PackageStructureCreator");
    aRet[1] = ::rtl::OUString::createFromAscii("com.sun.star.comp.embed.PackageStructureCreator");
    return aRet;
}

//-------------------------------------------------------------------------
::rtl::OUString SAL_CALL OPackageStructureCreator::impl_getStaticImplementationName()
{
    return ::rtl::OUString::createFromAscii("com.sun.star.comp.embed.PackageStructureCreator");
}

//-------------------------------------------------------------------------
uno::Reference< lang::XSingleServiceFactory > SAL_CALL OPackageStructureCreator::impl_createFactory(
            const uno::Reference< lang::XMultiServiceFactory >& xServiceManager )
{
    return ::cppu::createOneInstanceFactory( xServiceManager,
                                OPackageStructureCreator::impl_getStaticImplementationName(),
                                OPackageStructureCreator::impl_staticCreateSelfInstance,
                                OPackageStructureCreator::impl_getStaticSupportedServiceNames() );
}

//-------------------------------------------------------------------------
uno::Reference< uno::XInterface > SAL_CALL OPackageStructureCreator::impl_staticCreateSelfInstance(
            const uno::Reference< lang::XMultiServiceFactory >& xServiceManager )
{
    return uno::Reference< uno::XInterface >( *new OPackageStructureCreator( xServiceManager ) );
}


//-------------------------------------------------------------------------
void SAL_CALL OPackageStructureCreator::convertToPackage( const ::rtl::OUString& aFolderUrl,
                                                          const uno::Reference< io::XOutputStream >& xTargetStream )
        throw ( io::IOException,
                uno::RuntimeException )
{
    uno::Reference< ::com::sun::star::ucb::XCommandEnvironment > xComEnv;

    if ( !xTargetStream.is() )
        throw io::IOException(); // TODO

    sal_Bool bSuccess = sal_False;
    ::ucb::Content aContent;
    if( ::ucb::Content::create( aFolderUrl, xComEnv, aContent ) )
    {
        SvStream* pTempStream = NULL;

        ::rtl::OUString aTempURL = ::utl::TempFile().GetURL();
        try {
            if ( aContent.isFolder() )
            {
                UCBStorage* pUCBStorage = new UCBStorage( aContent,
                                                          aFolderUrl,
                                                          STREAM_READ,
                                                          sal_False,
                                                          sal_True );
                SotStorageRef aStorage = new SotStorage( pUCBStorage );

                if ( aTempURL.getLength() )
                {
                    pTempStream = new SvFileStream( aTempURL, STREAM_STD_READWRITE );
                    SotStorageRef aTargetStorage = new SotStorage( sal_True, *pTempStream );
                    aStorage->CopyTo( aTargetStorage );
                    aTargetStorage->Commit();

                    if ( aStorage->GetError() || aTargetStorage->GetError() || pTempStream->GetError() )
                        throw io::IOException();

                    aTargetStorage = NULL;
                    aStorage = NULL;

                    pTempStream->Seek( 0 );

                    uno::Sequence< sal_Int8 > aSeq( 32000 );
                    sal_uInt32 nRead = 0;
                    do {
                        if ( aSeq.getLength() < 32000 )
                            aSeq.realloc( 32000 );

                        nRead = pTempStream->Read( aSeq.getArray(), 32000 );
                        if ( nRead < 32000 )
                            aSeq.realloc( nRead );
                        xTargetStream->writeBytes( aSeq );
                    } while( !pTempStream->IsEof() && !pTempStream->GetError() && nRead );

                    if ( pTempStream->GetError() )
                        throw io::IOException();

                    bSuccess = sal_True;
                }
            }
        }
        catch ( uno::RuntimeException& )
        {
            if ( pTempStream )
                delete pTempStream;

            if ( aTempURL.getLength() )
                ::utl::UCBContentHelper::Kill( aTempURL );

            throw;
        }
        catch ( io::IOException& )
        {
            if ( pTempStream )
                delete pTempStream;

            if ( aTempURL.getLength() )
                ::utl::UCBContentHelper::Kill( aTempURL );

            throw;
        }
        catch ( uno::Exception& )
        {
        }

        if ( pTempStream )
            delete pTempStream;

        if ( aTempURL.getLength() )
            ::utl::UCBContentHelper::Kill( aTempURL );
    }

    if ( !bSuccess )
        throw io::IOException(); // TODO: can't proceed with creation
}

//-------------------------------------------------------------------------
::rtl::OUString SAL_CALL OPackageStructureCreator::getImplementationName()
    throw ( uno::RuntimeException )
{
    return impl_getStaticImplementationName();
}

//-------------------------------------------------------------------------
sal_Bool SAL_CALL OPackageStructureCreator::supportsService( const ::rtl::OUString& ServiceName )
    throw ( uno::RuntimeException )
{
    uno::Sequence< ::rtl::OUString > aSeq = impl_getStaticSupportedServiceNames();

    for ( sal_Int32 nInd = 0; nInd < aSeq.getLength(); nInd++ )
        if ( ServiceName.compareTo( aSeq[nInd] ) == 0 )
            return sal_True;

    return sal_False;
}

//-------------------------------------------------------------------------
uno::Sequence< ::rtl::OUString > SAL_CALL OPackageStructureCreator::getSupportedServiceNames()
    throw ( uno::RuntimeException )
{
    return impl_getStaticSupportedServiceNames();
}

