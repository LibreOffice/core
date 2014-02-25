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
#ifndef SC_VBA_STYLE_HXX
#define SC_VBA_STYLE_HXX
#include <ooo/vba/excel/XStyle.hpp>
#include <com/sun/star/style/XStyle.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include "vbaformat.hxx"


typedef ScVbaFormat< ov::excel::XStyle > ScVbaStyle_BASE;

class ScVbaStyle : public ScVbaStyle_BASE
{
protected:
    css::uno::Reference< css::style::XStyle > mxStyle;
    css::uno::Reference< css::frame::XModel > mxModel;
    css::uno::Reference< css::container::XNameContainer > mxStyleFamilyNameContainer;
    void initialise() throw ( css::uno::RuntimeException );
public:
    ScVbaStyle( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext > & xContext, const OUString& sStyleName, const css::uno::Reference< css::frame::XModel >& _xModel ) throw ( css::script::BasicErrorException, css::uno::RuntimeException );
    ScVbaStyle( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext > & xContext, const css::uno::Reference< css::beans::XPropertySet >& _xPropertySet, const css::uno::Reference< css::frame::XModel >& _xModel ) throw ( css::script::BasicErrorException, css::uno::RuntimeException );
    virtual ~ScVbaStyle(){}
    static css::uno::Reference< css::container::XNameAccess > getStylesNameContainer( const css::uno::Reference< css::frame::XModel >& xModel ) throw( css::uno::RuntimeException );
    virtual css::uno::Reference< ov::XHelperInterface > thisHelperIface() { return this; };
    // XStyle Methods
    virtual ::sal_Bool SAL_CALL BuiltIn() throw (css::script::BasicErrorException, css::uno::RuntimeException, std::exception);
    virtual void SAL_CALL setName( const OUString& Name ) throw (css::script::BasicErrorException, css::uno::RuntimeException, std::exception);
    virtual OUString SAL_CALL getName() throw (css::script::BasicErrorException, css::uno::RuntimeException, std::exception);
    virtual void SAL_CALL setNameLocal( const OUString& NameLocal ) throw (css::script::BasicErrorException, css::uno::RuntimeException, std::exception);
    virtual OUString SAL_CALL getNameLocal() throw (css::script::BasicErrorException, css::uno::RuntimeException, std::exception);
    virtual void SAL_CALL Delete() throw (css::script::BasicErrorException, css::uno::RuntimeException, std::exception);
    // XFormat
    virtual void SAL_CALL setMergeCells( const css::uno::Any& MergeCells ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getMergeCells(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    // XHelperInterface
    virtual OUString getServiceImplName();
    virtual css::uno::Sequence<OUString> getServiceNames();
};

#endif //SC_VBA_AXIS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
