/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef __FRAMEWORK_PATTERN_STORAGES_HXX_
#define __FRAMEWORK_PATTERN_STORAGES_HXX_

//_______________________________________________
// own includes

#include <services.h>
#include <general.h>

//_______________________________________________
// interface includes
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/embed/XPackageStructureCreator.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
