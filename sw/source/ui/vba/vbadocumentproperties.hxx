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
#ifndef SW_VBA_DOCUMENTPROPERTIES_HXX
#define SW_VBA_DOCUMENTPROPERTIES_HXX

#include <ooo/vba/XDocumentProperties.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <vbahelper/vbacollectionimpl.hxx>
#include <boost/unordered_map.hpp>

typedef CollTestImplHelper< ov::XDocumentProperties > SwVbaDocumentproperties_BASE;

class SwVbaBuiltinDocumentProperties : public SwVbaDocumentproperties_BASE
{
protected:
    css::uno::Reference< css::frame::XModel > m_xModel;
public:
    SwVbaBuiltinDocumentProperties( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext > & xContext, const css::uno::Reference< css::frame::XModel >& xDocument );

    // XDocumentProperties
    virtual css::uno::Reference< ::ooo::vba::XDocumentProperty > SAL_CALL Add( const ::rtl::OUString& Name, ::sal_Bool LinkToContent, ::sal_Int8 Type, const css::uno::Any& Value, const css::uno::Any& LinkSource ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    // XEnumerationAccess
    virtual css::uno::Type SAL_CALL getElementType() throw (css::uno::RuntimeException);
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createEnumeration() throw (css::uno::RuntimeException);
    // ScVbaCollectionBaseImpl
    virtual css::uno::Any createCollectionObject( const css::uno::Any& aSource );

    // XHelperInterface
    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();
};

class SwVbaCustomDocumentProperties : public SwVbaBuiltinDocumentProperties
{
public:
    SwVbaCustomDocumentProperties( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext > & xContext, const css::uno::Reference< css::frame::XModel >& xDocument );
// XDocumentProperties
    virtual css::uno::Reference< ::ooo::vba::XDocumentProperty > SAL_CALL Add( const ::rtl::OUString& Name, ::sal_Bool LinkToContent, ::sal_Int8 Type, const css::uno::Any& Value, const css::uno::Any& LinkSource ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    // XHelperInterface
    virtual rtl::OUString& getServiceImplName();
};

#endif /* SW_VBA_DOCUMENTPROPERTY_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
