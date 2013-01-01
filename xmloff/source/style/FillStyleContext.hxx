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
#include <com/sun/star/util/Color.hpp>
#include <xmloff/xmlstyle.hxx>
#include <rtl/ustring.hxx>

//////////////////////////////////////////////////////////////////////////////
// draw:gradient context

class XMLGradientStyleContext: public SvXMLStyleContext
{
private:
    ::com::sun::star::uno::Any  maAny;
    rtl::OUString               maStrName;

public:
    TYPEINFO();

    XMLGradientStyleContext( SvXMLImport& rImport,  sal_uInt16 nPrfx,  const rtl::OUString& rLName,
                           const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList >& xAttrList );
    virtual ~XMLGradientStyleContext();

    virtual void EndElement();

    virtual sal_Bool IsTransient() const;
};

//////////////////////////////////////////////////////////////////////////////
// svg:linearGradient context
class XMLLinearGradientStyleContext: public SvXMLStyleContext
{
private:
    ::com::sun::star::uno::Any  maAny;
    rtl::OUString               maStrName;
    ::std::vector< ::com::sun::star::util::Color > mStopColors;
    ::std::vector< double > mStopOffsets;

public:
    TYPEINFO();

    XMLLinearGradientStyleContext( SvXMLImport& rImport,  sal_uInt16 nPrfx,  const rtl::OUString& rLName,
         const com::sun::star::uno::Reference<
             com::sun::star::xml::sax::XAttributeList> & xAttrList );
    virtual ~XMLLinearGradientStyleContext();

    virtual void EndElement();

    virtual sal_Bool IsTransient() const;

    void SetGradientStop( ::com::sun::star::util::Color aColor, double aOffset);

protected:
    virtual SvXMLImportContext *CreateChildContext(
        sal_uInt16 nPrefix,
        const ::rtl::OUString& rLocalName,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::sax::XAttributeList> & xAttrList );
};

//////////////////////////////////////////////////////////////////////////////
// import svg:stop elements
class XMLSvgGradientStopImportContext : public SvXMLImportContext
{
    XMLLinearGradientStyleContext& rGradientStyleContext;

private:
    ::com::sun::star::uno::Any  maAny;

public:

    TYPEINFO();

    XMLSvgGradientStopImportContext(
        SvXMLImport& rImport,
        sal_uInt16 nPrfx,
        const ::rtl::OUString& rLocalName,
        XMLLinearGradientStyleContext& rGradientStyle);

    ~XMLSvgGradientStopImportContext();

protected:

    virtual void StartElement(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::sax::XAttributeList> & xAttrList);

    virtual void EndElement();
};

//////////////////////////////////////////////////////////////////////////////
// draw:hatch context

class XMLHatchStyleContext: public SvXMLStyleContext
{
private:
    ::com::sun::star::uno::Any  maAny;
    rtl::OUString               maStrName;

public:
    TYPEINFO();

    XMLHatchStyleContext( SvXMLImport& rImport,  sal_uInt16 nPrfx,  const rtl::OUString& rLName,
                           const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList >& xAttrList );
    virtual ~XMLHatchStyleContext();

    virtual void EndElement();

    virtual sal_Bool IsTransient() const;
};

//////////////////////////////////////////////////////////////////////////////
// draw:fill-image context

class XMLBitmapStyleContext: public SvXMLStyleContext
{
private:
    ::com::sun::star::uno::Any  maAny;
    rtl::OUString               maStrName;
    ::com::sun::star::uno::Reference < ::com::sun::star::io::XOutputStream > mxBase64Stream;

public:
    TYPEINFO();

    XMLBitmapStyleContext( SvXMLImport& rImport,  sal_uInt16 nPrfx,  const rtl::OUString& rLName,
                           const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList >& xAttrList );
    virtual ~XMLBitmapStyleContext();

    virtual SvXMLImportContext *CreateChildContext(
            sal_uInt16 nPrefix,
            const ::rtl::OUString& rLocalName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttrList );

    virtual void EndElement();

    virtual sal_Bool IsTransient() const;
};

//////////////////////////////////////////////////////////////////////////////
// draw:transparency context

class XMLTransGradientStyleContext: public SvXMLStyleContext
{
private:
    ::com::sun::star::uno::Any  maAny;
    rtl::OUString               maStrName;

public:
    TYPEINFO();

    XMLTransGradientStyleContext( SvXMLImport& rImport,  sal_uInt16 nPrfx,  const rtl::OUString& rLName,
                           const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList >& xAttrList );
    virtual ~XMLTransGradientStyleContext();

    virtual void EndElement();

    virtual sal_Bool IsTransient() const;
};

//////////////////////////////////////////////////////////////////////////////
// draw:marker context

class XMLMarkerStyleContext: public SvXMLStyleContext
{
private:
    ::com::sun::star::uno::Any  maAny;
    rtl::OUString               maStrName;

public:
    TYPEINFO();

    XMLMarkerStyleContext( SvXMLImport& rImport,  sal_uInt16 nPrfx,  const rtl::OUString& rLName,
                           const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList >& xAttrList );
    virtual ~XMLMarkerStyleContext();

    virtual void EndElement();

    virtual sal_Bool IsTransient() const;
};

//////////////////////////////////////////////////////////////////////////////
// draw:marker context

class XMLDashStyleContext: public SvXMLStyleContext
{
private:
    ::com::sun::star::uno::Any  maAny;
    rtl::OUString               maStrName;

public:
    TYPEINFO();

    XMLDashStyleContext( SvXMLImport& rImport,  sal_uInt16 nPrfx,  const rtl::OUString& rLName,
                           const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList >& xAttrList );
    virtual ~XMLDashStyleContext();

    virtual void EndElement();

    virtual sal_Bool IsTransient() const;
};

#endif  // _XMLOFF_FILLSTYLECONTEXTS_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
