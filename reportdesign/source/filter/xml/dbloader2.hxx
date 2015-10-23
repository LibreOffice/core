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

#ifndef INCLUDED_REPORTDESIGN_SOURCE_FILTER_XML_DBLOADER2_HXX
#define INCLUDED_REPORTDESIGN_SOURCE_FILTER_XML_DBLOADER2_HXX

#include <toolkit/helper/vclunohelper.hxx>
#include <toolkit/awt/vclxwindow.hxx>
#include <com/sun/star/task/XStatusIndicatorFactory.hpp>
#include <com/sun/star/task/XStatusIndicator.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XFrameLoader.hpp>
#include <com/sun/star/document/XEventListener.hpp>
#include <com/sun/star/sdb/XDocumentDataSource.hpp>
#include <com/sun/star/frame/XLoadEventListener.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/container/XSet.hpp>
#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <com/sun/star/document/XExtendedFilterDetection.hpp>
#include <cppuhelper/implbase.hxx>
#include <com/sun/star/lang/XInitialization.hpp>
#include <vcl/svapp.hxx>
#include <ucbhelper/content.hxx>
#include <ucbhelper/commandenvironment.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/documentconstants.hxx>
#include <comphelper/types.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <osl/file.hxx>
#include <sfx2/docfile.hxx>
#include <unotools/moduleoptions.hxx>
#include <comphelper/storagehelper.hxx>
#include <vcl/msgbox.hxx>


namespace rptxml
{

    class ORptTypeDetection : public ::cppu::WeakImplHelper< css::document::XExtendedFilterDetection, css::lang::XServiceInfo>
{
    css::uno::Reference< css::uno::XComponentContext > m_xContext;
public:
    explicit ORptTypeDetection(css::uno::Reference< css::uno::XComponentContext > const & xContext);

    // XServiceInfo
    OUString                                     SAL_CALL getImplementationName() throw(std::exception  ) override;
    sal_Bool                                            SAL_CALL supportsService(const OUString& ServiceName) throw(std::exception  ) override;
    css::uno::Sequence< OUString >  SAL_CALL getSupportedServiceNames() throw(std::exception  ) override;

    // static methods
    static OUString getImplementationName_Static() throw( css::uno::RuntimeException )
    {
        return OUString("com.sun.star.comp.report.ORptTypeDetection");
    }
    static css::uno::Sequence< OUString > getSupportedServiceNames_Static() throw( css::uno::RuntimeException );
    static css::uno::Reference< css::uno::XInterface > SAL_CALL
    create(css::uno::Reference< css::uno::XComponentContext > const & xContext);

    virtual OUString SAL_CALL detect( css::uno::Sequence< css::beans::PropertyValue >& Descriptor ) throw (css::uno::RuntimeException, std::exception) override;
};
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
