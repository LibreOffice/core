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
#ifndef INCLUDED_SC_SOURCE_UI_VBA_VBASTYLES_HXX
#define INCLUDED_SC_SOURCE_UI_VBA_VBASTYLES_HXX
#include <ooo/vba/excel/XStyles.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <vbahelper/vbacollectionimpl.hxx>

typedef CollTestImplHelper< ov::excel::XStyles > ScVbaStyles_BASE;
class ScVbaStyles: public ScVbaStyles_BASE
{
    css::uno::Reference< css::frame::XModel > mxModel;
    // hard ref to parent ( perhaps we should try this in the
    // XHelperInterface itself
    css::uno::Reference< ov::XHelperInterface > mxParent;
    css::uno::Reference< css::lang::XMultiServiceFactory > mxMSF;
    css::uno::Reference< css::container::XNameContainer > mxNameContainerCellStyles;
public:
    ScVbaStyles( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext > & xContext, const css::uno::Reference< css::frame::XModel >& xModel ) throw ( css::script::BasicErrorException );
    css::uno::Sequence< OUString > getStyleNames() throw ( css::uno::RuntimeException );
    void Delete(const OUString& _sStyleName) throw ( css::script::BasicErrorException );
    // XStyles
     virtual css::uno::Reference< ov::excel::XStyle > SAL_CALL Add( const OUString& Name, const css::uno::Any& BasedOn ) throw (css::script::BasicErrorException, css::uno::RuntimeException, std::exception) override;
    // XEnumerationAccess
    virtual css::uno::Type SAL_CALL getElementType() throw (css::uno::RuntimeException) override;
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createEnumeration() throw (css::uno::RuntimeException) override;
    virtual css::uno::Any createCollectionObject(const css::uno::Any&) override;
    // XHelperInterface
    virtual OUString getServiceImplName() override;
    virtual css::uno::Sequence<OUString> getServiceNames() override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
