/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: storages.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 01:33:37 $
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

#ifndef __FRAMEWORK_PATTERN_STORAGES_HXX_
#define __FRAMEWORK_PATTERN_STORAGES_HXX_

//_______________________________________________
// own includes

#ifndef __FRAMEWORK_SERVICES_H_
#include <services.h>
#endif

#ifndef __FRAMEWORK_GENERAL_H_
#include <general.h>
#endif

//_______________________________________________
// interface includes

#ifndef _COM_SUN_STAR_IO_XOUTPUTSTREAM_HPP_
#include <com/sun/star/io/XOutputStream.hpp>
#endif

#ifndef _COM_SUN_STAR_IO_XSEEKABLE_HPP_
#include <com/sun/star/io/XSeekable.hpp>
#endif

#ifndef _COM_SUN_STAR_EMBED_ELEMENTMODES_HPP_
#include <com/sun/star/embed/ElementModes.hpp>
#endif

#ifndef _COM_SUN_STAR_EMBED_XSTORAGE_HPP_
#include <com/sun/star/embed/XStorage.hpp>
#endif

#ifndef _COM_SUN_STAR_EMBED_XPACKAGESTRUCTURECREATOR_HPP_
#include <com/sun/star/embed/XPackageStructureCreator.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XSINGLESERVICEFACTORY_HPP_
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

//_______________________________________________
// other includes

//_______________________________________________
// namespaces

#ifndef css
namespace css = ::com::sun::star;
#endif

namespace framework{
    namespace pattern{
        namespace storages{

//_______________________________________________
// definitions

//-----------------------------------------------
css::uno::Reference< css::embed::XStorage > createTempStorageBasedOnFolder(const ::rtl::OUString&                                        sFolder  ,
                                                                           const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR    ,
                                                                                 sal_Bool                                                bReadOnly)
{
    // error during opening the temp file isnt realy a runtime error -> handle it gracefully
    css::uno::Reference< css::io::XOutputStream > xTempFile(xSMGR->createInstance(SERVICENAME_TEMPFILE), css::uno::UNO_QUERY);
    if (!xTempFile.is())
        return css::uno::Reference< css::embed::XStorage >();

    // creation of needed resources is mandatory -> error = runtime error
    css::uno::Reference< css::embed::XPackageStructureCreator > xPackageCreator(xSMGR->createInstance(SERVICENAME_PACKAGESTRUCTURECREATOR), css::uno::UNO_QUERY_THROW);
    css::uno::Reference< css::lang::XSingleServiceFactory >     xStorageFactory(xSMGR->createInstance(SERVICENAME_STORAGEFACTORY)         , css::uno::UNO_QUERY_THROW);

    // create zip package
    xPackageCreator->convertToPackage(sFolder, xTempFile);

    // seek it back - so it can be used in a defined way.
    css::uno::Reference< css::io::XSeekable > xSeekable(xTempFile, css::uno::UNO_QUERY_THROW);
    xSeekable->seek(0);

    // open the temp. zip package - using the right open mode
    sal_Int32 nOpenMode = css::embed::ElementModes::ELEMENT_READWRITE;
    if (bReadOnly)
        nOpenMode = css::embed::ElementModes::ELEMENT_READ;

    css::uno::Sequence< css::uno::Any > lArgs(2);
    lArgs[0] <<= xTempFile;
    lArgs[1] <<= nOpenMode;

    css::uno::Reference< css::embed::XStorage > xStorage(xStorageFactory->createInstanceWithArguments(lArgs), css::uno::UNO_QUERY_THROW);
    return xStorage;
}

        } // namespace storages
    } // namespace pattern
} // namespace framework

#endif // __FRAMEWORK_PATTERN_STORAGES_HXX_
