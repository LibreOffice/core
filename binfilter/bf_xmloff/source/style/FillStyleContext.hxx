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

#ifndef _XMLOFF_FILLSTYLECONTEXTS_HXX_
#define _XMLOFF_FILLSTYLECONTEXTS_HXX_

#include <com/sun/star/io/XOutputStream.hpp>

#include <xmlstyle.hxx>

#include "rtl/ustring.hxx"
namespace binfilter {

//////////////////////////////////////////////////////////////////////////////
// draw:gardient context

class XMLGradientStyleContext: public SvXMLStyleContext
{
private:
    ::com::sun::star::uno::Any	maAny;
    ::rtl::OUString				maStrName;

public:
    TYPEINFO();

    XMLGradientStyleContext( SvXMLImport& rImport,  sal_uInt16 nPrfx,  const ::rtl::OUString& rLName, 
                           const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );
    virtual ~XMLGradientStyleContext();

    virtual void EndElement();

    virtual BOOL IsTransient() const;
};

//////////////////////////////////////////////////////////////////////////////
// draw:hatch context

class XMLHatchStyleContext: public SvXMLStyleContext
{
private:
    ::com::sun::star::uno::Any	maAny;
    ::rtl::OUString				maStrName;

public:
    TYPEINFO();

    XMLHatchStyleContext( SvXMLImport& rImport,  sal_uInt16 nPrfx,  const ::rtl::OUString& rLName, 
                           const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );
    virtual ~XMLHatchStyleContext();

    virtual void EndElement();

    virtual BOOL IsTransient() const;
};

//////////////////////////////////////////////////////////////////////////////
// draw:fill-image context

class XMLBitmapStyleContext: public SvXMLStyleContext
{
private:
    ::com::sun::star::uno::Any	maAny;
    ::rtl::OUString				maStrName;
    ::com::sun::star::uno::Reference < ::com::sun::star::io::XOutputStream > mxBase64Stream;

public:
    TYPEINFO();

    XMLBitmapStyleContext( SvXMLImport& rImport,  sal_uInt16 nPrfx,  const ::rtl::OUString& rLName, 
                           const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );
    virtual ~XMLBitmapStyleContext();

    virtual SvXMLImportContext *CreateChildContext(
            sal_uInt16 nPrefix,
            const ::rtl::OUString& rLocalName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttrList );

    virtual void EndElement();

    virtual BOOL IsTransient() const;
};

//////////////////////////////////////////////////////////////////////////////
// draw:transparency context

class XMLTransGradientStyleContext: public SvXMLStyleContext
{
private:
    ::com::sun::star::uno::Any	maAny;
    ::rtl::OUString				maStrName;

public:
    TYPEINFO();

    XMLTransGradientStyleContext( SvXMLImport& rImport,  sal_uInt16 nPrfx,  const ::rtl::OUString& rLName, 
                           const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );
    virtual ~XMLTransGradientStyleContext();

    virtual void EndElement();

    virtual BOOL IsTransient() const;
};

//////////////////////////////////////////////////////////////////////////////
// draw:marker context

class XMLMarkerStyleContext: public SvXMLStyleContext
{
private:
    ::com::sun::star::uno::Any	maAny;
    ::rtl::OUString				maStrName;

public:
    TYPEINFO();

    XMLMarkerStyleContext( SvXMLImport& rImport,  sal_uInt16 nPrfx,  const ::rtl::OUString& rLName, 
                           const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );
    virtual ~XMLMarkerStyleContext();

    virtual void EndElement();

    virtual BOOL IsTransient() const;
};

//////////////////////////////////////////////////////////////////////////////
// draw:marker context

class XMLDashStyleContext: public SvXMLStyleContext
{
private:
    ::com::sun::star::uno::Any	maAny;
    ::rtl::OUString				maStrName;

public:
    TYPEINFO();

    XMLDashStyleContext( SvXMLImport& rImport,  sal_uInt16 nPrfx,  const ::rtl::OUString& rLName, 
                           const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );
    virtual ~XMLDashStyleContext();

    virtual void EndElement();

    virtual BOOL IsTransient() const;
};

}//end of namespace binfilter
#endif	// _XMLOFF_FILLSTYLECONTEXTS_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
