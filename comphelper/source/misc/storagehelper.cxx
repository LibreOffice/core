/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: storagehelper.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 18:29:01 $
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
#include "precompiled_comphelper.hxx"

#ifndef _COM_SUN_STAR_EMBED_ELEMENTMODES_HPP_
#include <com/sun/star/embed/ElementModes.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_XENCRYPTIONPROTECTEDSOURCE_HPP_
#include <com/sun/star/embed/XEncryptionProtectedSource.hpp>
#endif

#ifndef _COM_SUN_STAR_UCB_XSIMPLEFILEACCESS_HPP_
#include <com/sun/star/ucb/XSimpleFileAccess.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_ILLEGALTYPEEXCEPTION_HPP_
#include <com/sun/star/beans/IllegalTypeException.hpp>
#endif

#include <comphelper/fileformat.h>
#include <comphelper/processfactory.hxx>
#include <comphelper/documentconstants.hxx>

#include <comphelper/storagehelper.hxx>

using namespace ::com::sun::star;

namespace comphelper {

// ----------------------------------------------------------------------
uno::Reference< lang::XSingleServiceFactory > OStorageHelper::GetStorageFactory(
                            const uno::Reference< lang::XMultiServiceFactory >& xSF )
        throw ( uno::Exception )
{
    uno::Reference< lang::XMultiServiceFactory > xFactory = xSF.is() ? xSF : ::comphelper::getProcessServiceFactory();
    if ( !xFactory.is() )
        throw uno::RuntimeException();

    uno::Reference < lang::XSingleServiceFactory > xStorageFactory(
                    xFactory->createInstance ( ::rtl::OUString::createFromAscii( "com.sun.star.embed.StorageFactory" ) ),
                    uno::UNO_QUERY );

    if ( !xStorageFactory.is() )
        throw uno::RuntimeException();

    return xStorageFactory;
}

// ----------------------------------------------------------------------
uno::Reference< embed::XStorage > OStorageHelper::GetTemporaryStorage(
            const uno::Reference< lang::XMultiServiceFactory >& xFactory )
    throw ( uno::Exception )
{
    uno::Reference< embed::XStorage > xTempStorage( GetStorageFactory( xFactory )->createInstance(),
                                                    uno::UNO_QUERY );
    if ( !xTempStorage.is() )
        throw uno::RuntimeException();

    return xTempStorage;
}

// ----------------------------------------------------------------------
uno::Reference< embed::XStorage > OStorageHelper::GetStorageFromURL(
            const ::rtl::OUString& aURL,
            sal_Int32 nStorageMode,
            const uno::Reference< lang::XMultiServiceFactory >& xFactory )
    throw ( uno::Exception )
{
    uno::Sequence< uno::Any > aArgs( 2 );
    aArgs[0] <<= aURL;
    aArgs[1] <<= nStorageMode;

    uno::Reference< embed::XStorage > xTempStorage( GetStorageFactory( xFactory )->createInstanceWithArguments( aArgs ),
                                                    uno::UNO_QUERY );
    if ( !xTempStorage.is() )
        throw uno::RuntimeException();

    return xTempStorage;
}

// ----------------------------------------------------------------------
uno::Reference< embed::XStorage > OStorageHelper::GetStorageFromInputStream(
            const uno::Reference < io::XInputStream >& xStream,
            const uno::Reference< lang::XMultiServiceFactory >& xFactory )
        throw ( uno::Exception )
{
    uno::Sequence< uno::Any > aArgs( 2 );
    aArgs[0] <<= xStream;
    aArgs[1] <<= embed::ElementModes::READ;

    uno::Reference< embed::XStorage > xTempStorage( GetStorageFactory( xFactory )->createInstanceWithArguments( aArgs ),
                                                    uno::UNO_QUERY );
    if ( !xTempStorage.is() )
        throw uno::RuntimeException();

    return xTempStorage;
}

// ----------------------------------------------------------------------
uno::Reference< embed::XStorage > OStorageHelper::GetStorageFromStream(
            const uno::Reference < io::XStream >& xStream,
            sal_Int32 nStorageMode,
            const uno::Reference< lang::XMultiServiceFactory >& xFactory )
        throw ( uno::Exception )
{
    uno::Sequence< uno::Any > aArgs( 2 );
    aArgs[0] <<= xStream;
    aArgs[1] <<= nStorageMode;

    uno::Reference< embed::XStorage > xTempStorage( GetStorageFactory( xFactory )->createInstanceWithArguments( aArgs ),
                                                    uno::UNO_QUERY );
    if ( !xTempStorage.is() )
        throw uno::RuntimeException();

    return xTempStorage;
}

// ----------------------------------------------------------------------
void OStorageHelper::CopyInputToOutput(
            const uno::Reference< io::XInputStream >& xInput,
            const uno::Reference< io::XOutputStream >& xOutput )
    throw ( uno::Exception )
{
    static const sal_Int32 nConstBufferSize = 32000;

    sal_Int32 nRead;
    uno::Sequence < sal_Int8 > aSequence ( nConstBufferSize );

    do
    {
        nRead = xInput->readBytes ( aSequence, nConstBufferSize );
        if ( nRead < nConstBufferSize )
        {
            uno::Sequence < sal_Int8 > aTempBuf ( aSequence.getConstArray(), nRead );
            xOutput->writeBytes ( aTempBuf );
        }
        else
            xOutput->writeBytes ( aSequence );
    }
    while ( nRead == nConstBufferSize );
}

// ----------------------------------------------------------------------
uno::Reference< io::XInputStream > OStorageHelper::GetInputStreamFromURL(
            const ::rtl::OUString& aURL,
            const uno::Reference< lang::XMultiServiceFactory >& xSF )
    throw ( uno::Exception )
{
    uno::Reference< lang::XMultiServiceFactory > xFactory = xSF.is() ? xSF : ::comphelper::getProcessServiceFactory();
    if ( !xFactory.is() )
        throw uno::RuntimeException();

    uno::Reference < ::com::sun::star::ucb::XSimpleFileAccess > xTempAccess(
            xFactory->createInstance ( ::rtl::OUString::createFromAscii( "com.sun.star.ucb.SimpleFileAccess" ) ),
            uno::UNO_QUERY );

    if ( !xTempAccess.is() )
        throw uno::RuntimeException();

    uno::Reference< io::XInputStream > xInputStream = xTempAccess->openFileRead( aURL );
    if ( !xInputStream.is() )
        throw uno::RuntimeException();

    return xInputStream;
}

// ----------------------------------------------------------------------
void OStorageHelper::SetCommonStoragePassword(
            const uno::Reference< embed::XStorage >& xStorage,
            const ::rtl::OUString& aPass )
    throw ( uno::Exception )
{
    uno::Reference< embed::XEncryptionProtectedSource > xEncrSet( xStorage, uno::UNO_QUERY );
    if ( !xEncrSet.is() )
        throw io::IOException(); // TODO

    xEncrSet->setEncryptionPassword( aPass );
}

// ----------------------------------------------------------------------
sal_Int32 OStorageHelper::GetXStorageFormat(
            const uno::Reference< embed::XStorage >& xStorage )
        throw ( uno::Exception )
{
    uno::Reference< beans::XPropertySet > xStorProps( xStorage, uno::UNO_QUERY_THROW );

    ::rtl::OUString aMediaType;
    xStorProps->getPropertyValue( ::rtl::OUString::createFromAscii( "MediaType" ) ) >>= aMediaType;

    sal_Int32 nResult = 0;

    // TODO/LATER: the filter configuration could be used to detect it later, or batter a special service
    if (
        aMediaType.equalsIgnoreAsciiCaseAscii(MIMETYPE_VND_SUN_XML_WRITER_ASCII       ) ||
        aMediaType.equalsIgnoreAsciiCaseAscii(MIMETYPE_VND_SUN_XML_WRITER_WEB_ASCII   ) ||
        aMediaType.equalsIgnoreAsciiCaseAscii(MIMETYPE_VND_SUN_XML_WRITER_GLOBAL_ASCII) ||
        aMediaType.equalsIgnoreAsciiCaseAscii(MIMETYPE_VND_SUN_XML_DRAW_ASCII         ) ||
        aMediaType.equalsIgnoreAsciiCaseAscii(MIMETYPE_VND_SUN_XML_IMPRESS_ASCII      ) ||
        aMediaType.equalsIgnoreAsciiCaseAscii(MIMETYPE_VND_SUN_XML_CALC_ASCII         ) ||
        aMediaType.equalsIgnoreAsciiCaseAscii(MIMETYPE_VND_SUN_XML_CHART_ASCII        ) ||
        aMediaType.equalsIgnoreAsciiCaseAscii(MIMETYPE_VND_SUN_XML_MATH_ASCII         )
       )
    {
        nResult = SOFFICE_FILEFORMAT_60;
    }
    else
    if (
        aMediaType.equalsIgnoreAsciiCaseAscii(MIMETYPE_OASIS_OPENDOCUMENT_TEXT_ASCII        ) ||
        aMediaType.equalsIgnoreAsciiCaseAscii(MIMETYPE_OASIS_OPENDOCUMENT_TEXT_WEB_ASCII    ) ||
        aMediaType.equalsIgnoreAsciiCaseAscii(MIMETYPE_OASIS_OPENDOCUMENT_TEXT_GLOBAL_ASCII ) ||
        aMediaType.equalsIgnoreAsciiCaseAscii(MIMETYPE_OASIS_OPENDOCUMENT_DRAWING_ASCII     ) ||
        aMediaType.equalsIgnoreAsciiCaseAscii(MIMETYPE_OASIS_OPENDOCUMENT_PRESENTATION_ASCII) ||
        aMediaType.equalsIgnoreAsciiCaseAscii(MIMETYPE_OASIS_OPENDOCUMENT_SPREADSHEET_ASCII ) ||
        aMediaType.equalsIgnoreAsciiCaseAscii(MIMETYPE_OASIS_OPENDOCUMENT_CHART_ASCII       ) ||
        aMediaType.equalsIgnoreAsciiCaseAscii(MIMETYPE_OASIS_OPENDOCUMENT_FORMULA_ASCII     ) ||
        aMediaType.equalsIgnoreAsciiCaseAscii(MIMETYPE_OASIS_OPENDOCUMENT_DATABASE_ASCII    ) ||
        aMediaType.equalsIgnoreAsciiCaseAscii(MIMETYPE_OASIS_OPENDOCUMENT_REPORT_ASCII    ) ||
        aMediaType.equalsIgnoreAsciiCaseAscii(MIMETYPE_OASIS_OPENDOCUMENT_REPORT_CHART_ASCII    ) ||
        aMediaType.equalsIgnoreAsciiCaseAscii(MIMETYPE_OASIS_OPENDOCUMENT_TEXT_TEMPLATE_ASCII        ) ||
        aMediaType.equalsIgnoreAsciiCaseAscii(MIMETYPE_OASIS_OPENDOCUMENT_DRAWING_TEMPLATE_ASCII     ) ||
        aMediaType.equalsIgnoreAsciiCaseAscii(MIMETYPE_OASIS_OPENDOCUMENT_PRESENTATION_TEMPLATE_ASCII) ||
        aMediaType.equalsIgnoreAsciiCaseAscii(MIMETYPE_OASIS_OPENDOCUMENT_SPREADSHEET_TEMPLATE_ASCII ) ||
        aMediaType.equalsIgnoreAsciiCaseAscii(MIMETYPE_OASIS_OPENDOCUMENT_CHART_TEMPLATE_ASCII       ) ||
        aMediaType.equalsIgnoreAsciiCaseAscii(MIMETYPE_OASIS_OPENDOCUMENT_FORMULA_TEMPLATE_ASCII     )
       )
    {
        nResult = SOFFICE_FILEFORMAT_8;
    }
    else
    {
        // the mediatype is not known
        throw beans::IllegalTypeException();
    }

    return nResult;
}

// ----------------------------------------------------------------------
uno::Reference< embed::XStorage > OStorageHelper::GetTemporaryStorageOfFormat(
            const ::rtl::OUString& aFormat,
            const uno::Reference< lang::XMultiServiceFactory >& xFactory )
    throw ( uno::Exception )
{
    uno::Reference< lang::XMultiServiceFactory > xFactoryToUse = xFactory.is() ? xFactory : ::comphelper::getProcessServiceFactory();
    if ( !xFactoryToUse.is() )
        throw uno::RuntimeException();

    uno::Reference< io::XStream > xTmpStream(
        xFactoryToUse->createInstance( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.io.TempFile" ) ) ),
        uno::UNO_QUERY_THROW );

    return GetStorageOfFormatFromStream( aFormat, xTmpStream, embed::ElementModes::READWRITE, xFactoryToUse );
}

// ----------------------------------------------------------------------
uno::Reference< embed::XStorage > OStorageHelper::GetStorageOfFormatFromURL(
            const ::rtl::OUString& aFormat,
            const ::rtl::OUString& aURL,
            sal_Int32 nStorageMode,
            const uno::Reference< lang::XMultiServiceFactory >& xFactory )
    throw ( uno::Exception )
{
    uno::Sequence< beans::PropertyValue > aProps( 1 );
    aProps[0].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "StorageFormat" ) );
    aProps[0].Value <<= aFormat;

    uno::Sequence< uno::Any > aArgs( 3 );
    aArgs[0] <<= aURL;
    aArgs[1] <<= nStorageMode;
    aArgs[2] <<= aProps;

    uno::Reference< embed::XStorage > xTempStorage( GetStorageFactory( xFactory )->createInstanceWithArguments( aArgs ),
                                                    uno::UNO_QUERY );
    if ( !xTempStorage.is() )
        throw uno::RuntimeException();

    return xTempStorage;
}

// ----------------------------------------------------------------------
uno::Reference< embed::XStorage > OStorageHelper::GetStorageOfFormatFromInputStream(
            const ::rtl::OUString& aFormat,
            const uno::Reference < io::XInputStream >& xStream,
            const uno::Reference< lang::XMultiServiceFactory >& xFactory )
        throw ( uno::Exception )
{
    uno::Sequence< beans::PropertyValue > aProps( 1 );
    aProps[0].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "StorageFormat" ) );
    aProps[0].Value <<= aFormat;

    uno::Sequence< uno::Any > aArgs( 3 );
    aArgs[0] <<= xStream;
    aArgs[1] <<= embed::ElementModes::READ;
    aArgs[2] <<= aProps;

    uno::Reference< embed::XStorage > xTempStorage( GetStorageFactory( xFactory )->createInstanceWithArguments( aArgs ),
                                                    uno::UNO_QUERY );
    if ( !xTempStorage.is() )
        throw uno::RuntimeException();

    return xTempStorage;
}

// ----------------------------------------------------------------------
uno::Reference< embed::XStorage > OStorageHelper::GetStorageOfFormatFromStream(
            const ::rtl::OUString& aFormat,
            const uno::Reference < io::XStream >& xStream,
            sal_Int32 nStorageMode,
            const uno::Reference< lang::XMultiServiceFactory >& xFactory )
        throw ( uno::Exception )
{
    uno::Sequence< beans::PropertyValue > aProps( 1 );
    aProps[0].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "StorageFormat" ) );
    aProps[0].Value <<= aFormat;

    uno::Sequence< uno::Any > aArgs( 3 );
    aArgs[0] <<= xStream;
    aArgs[1] <<= nStorageMode;
    aArgs[2] <<= aProps;

    uno::Reference< embed::XStorage > xTempStorage( GetStorageFactory( xFactory )->createInstanceWithArguments( aArgs ),
                                                    uno::UNO_QUERY );
    if ( !xTempStorage.is() )
        throw uno::RuntimeException();

    return xTempStorage;
}

}

