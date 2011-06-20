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
    ScVbaStyle( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext > & xContext, const rtl::OUString& sStyleName, const css::uno::Reference< css::frame::XModel >& _xModel ) throw ( css::script::BasicErrorException, css::uno::RuntimeException );
    ScVbaStyle( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext > & xContext, const css::uno::Reference< css::beans::XPropertySet >& _xPropertySet, const css::uno::Reference< css::frame::XModel >& _xModel ) throw ( css::script::BasicErrorException, css::uno::RuntimeException );
    virtual ~ScVbaStyle(){}
    static css::uno::Reference< css::container::XNameAccess > getStylesNameContainer( const css::uno::Reference< css::frame::XModel >& xModel ) throw( css::uno::RuntimeException );
    virtual css::uno::Reference< ov::XHelperInterface > thisHelperIface() { return this; };
    // XStyle Methods
    virtual ::sal_Bool SAL_CALL BuiltIn() throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual void SAL_CALL setName( const ::rtl::OUString& Name ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getName() throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual void SAL_CALL setNameLocal( const ::rtl::OUString& NameLocal ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getNameLocal() throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual void SAL_CALL Delete() throw (css::script::BasicErrorException, css::uno::RuntimeException);
    // XFormat
    virtual void SAL_CALL setMergeCells( const css::uno::Any& MergeCells ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getMergeCells(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    // XHelperInterface
    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();
};

#endif //SC_VBA_AXIS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
