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

#ifndef _XMLOFF_FILLSTYLECONTEXTS_HXX_
#define _XMLOFF_FILLSTYLECONTEXTS_HXX_

#include <com/sun/star/io/XOutputStream.hpp>
#include <xmloff/xmlstyle.hxx>
#include<rtl/ustring.hxx>

// draw:gardient context

class XMLGradientStyleContext: public SvXMLStyleContext
{
private:
    ::com::sun::star::uno::Any  maAny;
    OUString               maStrName;

public:
    TYPEINFO();

    XMLGradientStyleContext( SvXMLImport& rImport,  sal_uInt16 nPrfx,  const OUString& rLName,
                           const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList >& xAttrList );
    virtual ~XMLGradientStyleContext();

    virtual void EndElement();

    virtual sal_Bool IsTransient() const;
};

// draw:hatch context

class XMLHatchStyleContext: public SvXMLStyleContext
{
private:
    ::com::sun::star::uno::Any  maAny;
    OUString               maStrName;

public:
    TYPEINFO();

    XMLHatchStyleContext( SvXMLImport& rImport,  sal_uInt16 nPrfx,  const OUString& rLName,
                           const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList >& xAttrList );
    virtual ~XMLHatchStyleContext();

    virtual void EndElement();

    virtual sal_Bool IsTransient() const;
};

// draw:fill-image context

class XMLBitmapStyleContext: public SvXMLStyleContext
{
private:
    ::com::sun::star::uno::Any  maAny;
    OUString               maStrName;
    ::com::sun::star::uno::Reference < ::com::sun::star::io::XOutputStream > mxBase64Stream;

public:
    TYPEINFO();

    XMLBitmapStyleContext( SvXMLImport& rImport,  sal_uInt16 nPrfx,  const OUString& rLName,
                           const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList >& xAttrList );
    virtual ~XMLBitmapStyleContext();

    virtual SvXMLImportContext *CreateChildContext(
            sal_uInt16 nPrefix,
            const OUString& rLocalName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttrList );

    virtual void EndElement();

    virtual sal_Bool IsTransient() const;
};

// draw:transparency context

class XMLTransGradientStyleContext: public SvXMLStyleContext
{
private:
    ::com::sun::star::uno::Any  maAny;
    OUString               maStrName;

public:
    TYPEINFO();

    XMLTransGradientStyleContext( SvXMLImport& rImport,  sal_uInt16 nPrfx,  const OUString& rLName,
                           const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList >& xAttrList );
    virtual ~XMLTransGradientStyleContext();

    virtual void EndElement();

    virtual sal_Bool IsTransient() const;
};

// draw:marker context

class XMLMarkerStyleContext: public SvXMLStyleContext
{
private:
    ::com::sun::star::uno::Any  maAny;
    OUString               maStrName;

public:
    TYPEINFO();

    XMLMarkerStyleContext( SvXMLImport& rImport,  sal_uInt16 nPrfx,  const OUString& rLName,
                           const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList >& xAttrList );
    virtual ~XMLMarkerStyleContext();

    virtual void EndElement();

    virtual sal_Bool IsTransient() const;
};

// draw:marker context

class XMLDashStyleContext: public SvXMLStyleContext
{
private:
    ::com::sun::star::uno::Any  maAny;
    OUString               maStrName;

public:
    TYPEINFO();

    XMLDashStyleContext( SvXMLImport& rImport,  sal_uInt16 nPrfx,  const OUString& rLName,
                           const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList >& xAttrList );
    virtual ~XMLDashStyleContext();

    virtual void EndElement();

    virtual sal_Bool IsTransient() const;
};

#endif  // _XMLOFF_FILLSTYLECONTEXTS_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
