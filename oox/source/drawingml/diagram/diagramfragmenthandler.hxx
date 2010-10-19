/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: diagramfragmenthandler.hxx,v $
 * $Revision: 1.4 $
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


#ifndef OOX_DRAWINGML_DIAGRAMFRAGMENTHANDLER
#define OOX_DRAWINGML_DIAGRAMFRAGMENTHANDLER

#include "oox/core/fragmenthandler.hxx"
#include "oox/core/fragmenthandler2.hxx"
#include "oox/drawingml/diagram/diagram.hxx"

namespace oox { namespace drawingml {


class DiagramDataFragmentHandler : public ::oox::core::FragmentHandler
{
public:
    DiagramDataFragmentHandler( oox::core::XmlFilterBase& rFilter, const ::rtl::OUString& rFragmentPath, const DiagramDataPtr pDataPtr ) throw();
    virtual ~DiagramDataFragmentHandler() throw();

    virtual void SAL_CALL endDocument() throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext( ::sal_Int32 Element, const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& Attribs ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);

private:

    DiagramDataPtr  mpDataPtr;
};



class DiagramLayoutFragmentHandler : public ::oox::core::FragmentHandler
{
public:
    DiagramLayoutFragmentHandler( oox::core::XmlFilterBase& rFilter, const ::rtl::OUString& rFragmentPath, const DiagramLayoutPtr pDataPtr ) throw();
    virtual ~DiagramLayoutFragmentHandler() throw();

    virtual void SAL_CALL endDocument() throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext( ::sal_Int32 Element, const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& Attribs ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);

private:

    DiagramLayoutPtr    mpDataPtr;
};

class DiagramQStylesFragmentHandler : public ::oox::core::FragmentHandler2
{
public:
    DiagramQStylesFragmentHandler(
        oox::core::XmlFilterBase& rFilter,
        const ::rtl::OUString& rFragmentPath,
        DiagramQStyleMap& rStylesMap );

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );

    virtual void onStartElement( const AttributeList& rAttribs );
    virtual void onEndElement( const ::rtl::OUString& rChars );

private:
    ::oox::core::ContextHandlerRef createStyleMatrixContext(sal_Int32 nElement,
                                                            const AttributeList& rAttribs,
                                                            ShapeStyleRef& o_rStyle);

    ::rtl::OUString   maStyleName;
    DiagramStyle      maStyleEntry;
    DiagramQStyleMap& mrStylesMap;
};

class ColorFragmentHandler : public ::oox::core::FragmentHandler2
{
public:
    ColorFragmentHandler(
        ::oox::core::XmlFilterBase& rFilter,
        const ::rtl::OUString& rFragmentPath,
        DiagramColorMap& rColorMap );

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );

    virtual void onStartElement( const AttributeList& rAttribs );
    virtual void onEndElement( const ::rtl::OUString& rChars );

private:
    ::rtl::OUString   maColorName;
    DiagramColor      maColorEntry;
    DiagramColorMap&  mrColorsMap;
};

} }


#endif
