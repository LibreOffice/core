/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: storagehelper.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: obo $ $Date: 2006-10-13 11:40:22 $
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
#ifndef _COMPHELPER_STORAGEHELPER_HXX
#define _COMPHELPER_STORAGEHELPER_HXX

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSINGLESERVICEFACTORY_HPP_
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#endif

#ifndef _COM_SUN_STAR_EMBED_XSTORAGE_HPP_
#include <com/sun/star/embed/XStorage.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_ELEMENTMODES_HPP_
#include <com/sun/star/embed/ElementModes.hpp>
#endif

#ifndef _COM_SUN_STAR_IO_XINPUTSTREAM_HPP_
#include <com/sun/star/io/XInputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XOUTPUTSTREAM_HPP_
#include <com/sun/star/io/XOutputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XSTREAM_HPP_
#include <com/sun/star/io/XStream.hpp>
#endif
#ifndef INCLUDED_COMPHELPERDLLAPI_H
#include "comphelper/comphelperdllapi.h"
#endif


#define PACKAGE_STORAGE_FORMAT_STRING   ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "PackageFormat" ) )
#define ZIP_STORAGE_FORMAT_STRING       ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ZipFormat" ) )
#define OFOPXML_STORAGE_FORMAT_STRING   ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "OFOPXMLFormat" ) )

namespace comphelper {

class COMPHELPER_DLLPUBLIC OStorageHelper
{
public:
    static ::com::sun::star::uno::Reference< ::com::sun::star::lang::XSingleServiceFactory >
        GetStorageFactory(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xSF
                            = ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >() )
        throw ( ::com::sun::star::uno::Exception );

    static ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >
        GetTemporaryStorage(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xFactory
                            = ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >() )
        throw ( ::com::sun::star::uno::Exception );

    static ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >
        GetStorageFromURL(
            const ::rtl::OUString& aURL,
            sal_Int32 nStorageMode,
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xFactory
                            = ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >() )
        throw ( ::com::sun::star::uno::Exception );

    static ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >
        GetStorageFromInputStream(
            const ::com::sun::star::uno::Reference < ::com::sun::star::io::XInputStream >& xStream,
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xFactory
                            = ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >() )
        throw ( ::com::sun::star::uno::Exception );

    static ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >
        GetStorageFromStream(
            const ::com::sun::star::uno::Reference < ::com::sun::star::io::XStream >& xStream,
            sal_Int32 nStorageMode = ::com::sun::star::embed::ElementModes::READWRITE,
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xFactory
                            = ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >() )
        throw ( ::com::sun::star::uno::Exception );

    static void CopyInputToOutput(
            const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& xInput,
            const ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream >& xOutput )
        throw ( ::com::sun::star::uno::Exception );

    static ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >
        GetInputStreamFromURL(
            const ::rtl::OUString& aURL,
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xFactory
                            = ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >() )
        throw ( ::com::sun::star::uno::Exception );

    static void SetCommonStoragePassword(
            const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage,
            const ::rtl::OUString& aPass )
        throw ( ::com::sun::star::uno::Exception );

    // the following method supports only storages of OOo formats
    static sal_Int32 GetXStorageFormat(
            const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage )
        throw ( ::com::sun::star::uno::Exception );

    // The followin methods are related to creation of a storage of specified format
    static ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >
        GetTemporaryStorageOfFormat(
            const ::rtl::OUString& aFormat,
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xFactory
                            = ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >() )
        throw ( ::com::sun::star::uno::Exception );

    static ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >
        GetStorageOfFormatFromURL(
            const ::rtl::OUString& aFormat,
            const ::rtl::OUString& aURL,
            sal_Int32 nStorageMode,
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xFactory
                            = ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >() )
        throw ( ::com::sun::star::uno::Exception );

    static ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >
        GetStorageOfFormatFromInputStream(
            const ::rtl::OUString& aFormat,
            const ::com::sun::star::uno::Reference < ::com::sun::star::io::XInputStream >& xStream,
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xFactory
                            = ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >() )
        throw ( ::com::sun::star::uno::Exception );

    static ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >
        GetStorageOfFormatFromStream(
            const ::rtl::OUString& aFormat,
            const ::com::sun::star::uno::Reference < ::com::sun::star::io::XStream >& xStream,
            sal_Int32 nStorageMode = ::com::sun::star::embed::ElementModes::READWRITE,
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xFactory
                            = ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >() )
        throw ( ::com::sun::star::uno::Exception );

};

}

#endif

