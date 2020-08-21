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

#ifndef INCLUDED_XMLOFF_SOURCE_STYLE_FILLSTYLECONTEXT_HXX
#define INCLUDED_XMLOFF_SOURCE_STYLE_FILLSTYLECONTEXT_HXX

#include <com/sun/star/io/XOutputStream.hpp>
#include <xmloff/xmlstyle.hxx>
#include <rtl/ustring.hxx>

// draw:gradient context

class XMLGradientStyleContext: public SvXMLStyleContext
{
private:
    css::uno::Any          maAny;
    OUString               maStrName;

public:

    XMLGradientStyleContext( SvXMLImport& rImport, sal_Int32 nElement,
                           const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList );
    virtual ~XMLGradientStyleContext() override;

    virtual void SAL_CALL startFastElement( sal_Int32 /*nElement*/,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& ) override {}
    virtual void SAL_CALL endFastElement(sal_Int32 nElement) override;

    virtual bool IsTransient() const override;
};

// draw:hatch context

class XMLHatchStyleContext: public SvXMLStyleContext
{
private:
    css::uno::Any          maAny;
    OUString               maStrName;

public:

    XMLHatchStyleContext( SvXMLImport& rImport, sal_Int32 nElement,
                           const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList );
    virtual ~XMLHatchStyleContext() override;

    virtual void SAL_CALL startFastElement( sal_Int32 /*nElement*/,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& ) override {}
    virtual void SAL_CALL endFastElement(sal_Int32 nElement) override;

    virtual bool IsTransient() const override;
};

// draw:fill-image context

class XMLBitmapStyleContext: public SvXMLStyleContext
{
private:
    css::uno::Any          maAny;
    OUString               maStrName;
    css::uno::Reference < css::io::XOutputStream > mxBase64Stream;

public:

    XMLBitmapStyleContext( SvXMLImport& rImport, sal_Int32 nElement,
                           const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList );
    virtual ~XMLBitmapStyleContext() override;

    virtual SvXMLImportContextRef CreateChildContext(
            sal_uInt16 nPrefix,
            const OUString& rLocalName,
            const css::uno::Reference< css::xml::sax::XAttributeList > & xAttrList ) override;

    virtual void SAL_CALL startFastElement( sal_Int32 /*nElement*/,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& ) override {}
    virtual void SAL_CALL endFastElement(sal_Int32 nElement) override;

    virtual bool IsTransient() const override;
};

// draw:transparency context

class XMLTransGradientStyleContext: public SvXMLStyleContext
{
private:
    css::uno::Any          maAny;
    OUString               maStrName;

public:

    XMLTransGradientStyleContext( SvXMLImport& rImport, sal_Int32 nElement,
                           const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList );
    virtual ~XMLTransGradientStyleContext() override;

    virtual void SAL_CALL startFastElement( sal_Int32 /*nElement*/,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& ) override {}
    virtual void SAL_CALL endFastElement(sal_Int32 nElement) override;

    virtual bool IsTransient() const override;
};

// draw:marker context

class XMLMarkerStyleContext: public SvXMLStyleContext
{
private:
    css::uno::Any          maAny;
    OUString               maStrName;

public:

    XMLMarkerStyleContext( SvXMLImport& rImport, sal_Int32 nElement,
                           const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList );
    virtual ~XMLMarkerStyleContext() override;

    virtual void SAL_CALL startFastElement( sal_Int32 /*nElement*/,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& ) override {}
    virtual void SAL_CALL endFastElement(sal_Int32 nElement) override;

    virtual bool IsTransient() const override;
};

// draw:marker context

class XMLDashStyleContext: public SvXMLStyleContext
{
private:
    css::uno::Any          maAny;
    OUString               maStrName;

public:

    XMLDashStyleContext( SvXMLImport& rImport, sal_Int32 nElement,
                           const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList );
    virtual ~XMLDashStyleContext() override;

    virtual void SAL_CALL startFastElement( sal_Int32 /*nElement*/,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& ) override {}
    virtual void SAL_CALL endFastElement(sal_Int32 nElement) override;

    virtual bool IsTransient() const override;
};

#endif // INCLUDED_XMLOFF_SOURCE_STYLE_FILLSTYLECONTEXT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
