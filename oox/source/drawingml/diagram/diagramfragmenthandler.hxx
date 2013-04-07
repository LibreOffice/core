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

#ifndef OOX_DRAWINGML_DIAGRAMFRAGMENTHANDLER
#define OOX_DRAWINGML_DIAGRAMFRAGMENTHANDLER

#include "oox/core/fragmenthandler.hxx"
#include "oox/core/fragmenthandler2.hxx"
#include "oox/drawingml/diagram/diagram.hxx"

namespace oox { namespace drawingml {


class DiagramDataFragmentHandler : public ::oox::core::FragmentHandler
{
public:
    DiagramDataFragmentHandler( oox::core::XmlFilterBase& rFilter, const OUString& rFragmentPath, const DiagramDataPtr pDataPtr ) throw();
    virtual ~DiagramDataFragmentHandler() throw();

    virtual void SAL_CALL endDocument() throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext( ::sal_Int32 Element, const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& Attribs ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);

private:

    DiagramDataPtr	mpDataPtr;
};



class DiagramLayoutFragmentHandler : public ::oox::core::FragmentHandler
{
public:
    DiagramLayoutFragmentHandler( oox::core::XmlFilterBase& rFilter, const OUString& rFragmentPath, const DiagramLayoutPtr pDataPtr ) throw();
    virtual ~DiagramLayoutFragmentHandler() throw();

    virtual void SAL_CALL endDocument() throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext( ::sal_Int32 Element, const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& Attribs ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);

private:

    DiagramLayoutPtr	mpDataPtr;
};

class DiagramQStylesFragmentHandler : public ::oox::core::FragmentHandler2
{
public:
    DiagramQStylesFragmentHandler(
        oox::core::XmlFilterBase& rFilter,
        const OUString& rFragmentPath,
        DiagramQStyleMap& rStylesMap );

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );

    virtual void onStartElement( const AttributeList& rAttribs );
    virtual void onEndElement();

private:
    ::oox::core::ContextHandlerRef createStyleMatrixContext(sal_Int32 nElement,
                                                            const AttributeList& rAttribs,
                                                            ShapeStyleRef& o_rStyle);

    OUString   maStyleName;
    DiagramStyle      maStyleEntry;
    DiagramQStyleMap& mrStylesMap;
};

class ColorFragmentHandler : public ::oox::core::FragmentHandler2
{
public:
    ColorFragmentHandler(
        ::oox::core::XmlFilterBase& rFilter,
        const OUString& rFragmentPath,
        DiagramColorMap& rColorMap );

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );

    virtual void onStartElement( const AttributeList& rAttribs );
    virtual void onEndElement();

private:
    OUString   maColorName;
    DiagramColor      maColorEntry;
    DiagramColorMap&  mrColorsMap;
};

} }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
