/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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
