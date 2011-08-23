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

#ifndef OOX_DRAWINGML_TEXTPARAGRAPHPROPERTIESCONTEXT_HXX
#define OOX_DRAWINGML_TEXTPARAGRAPHPROPERTIESCONTEXT_HXX

#include <list>

#include <com/sun/star/style/TabStop.hpp>
#include "oox/drawingml/drawingmltypes.hxx"
#include "oox/drawingml/textparagraphproperties.hxx"
#include "oox/drawingml/textspacing.hxx"
#include "oox/core/contexthandler.hxx"

namespace oox { namespace drawingml {

class TextParagraphPropertiesContext : public ::oox::core::ContextHandler
{
public:
    TextParagraphPropertiesContext( ::oox::core::ContextHandler& rParent,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XFastAttributeList >& rXAttributes,
            TextParagraphProperties& rTextParagraphProperties );
    ~TextParagraphPropertiesContext();

    virtual void SAL_CALL endFastElement( ::sal_Int32 Element ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext( ::sal_Int32 Element, const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& Attribs ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);

protected:
    TextParagraphProperties& mrTextParagraphProperties;
    TextSpacing		maLineSpacing;
    TextSpacing&    mrSpaceBefore;
    TextSpacing&    mrSpaceAfter;
    BulletList&		mrBulletList;
    ::std::list< ::com::sun::star::style::TabStop >  maTabList;
    ::boost::shared_ptr< BlipFillProperties > mxBlipProps;
};

} }

#endif  //  OOX_DRAWINGML_TEXTPARAGRAPHPROPERTIESCONTEXT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
