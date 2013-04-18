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

#ifndef OOX_DRAWINGML_SHAPECONTEXT_HXX
#define OOX_DRAWINGML_SHAPECONTEXT_HXX

#include <com/sun/star/drawing/XShapes.hpp>

#include "oox/core/contexthandler.hxx"
#include "oox/drawingml/shape.hxx"
#include "oox/drawingml/shapepropertiescontext.hxx"
#include "oox/dllapi.h"

namespace oox { namespace drawingml {

class OOX_DLLPUBLIC ShapeContext : public ::oox::core::ContextHandler
{
public:
    ShapeContext( ::oox::core::ContextHandler& rParent, ShapePtr pMasterShapePtr, ShapePtr pShapePtr );
    virtual ~ShapeContext();

    virtual void SAL_CALL endFastElement( ::sal_Int32 Element ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext( ::sal_Int32 Element, const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& Attribs ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);

    ShapePtr getShape();

protected:

    ShapePtr mpMasterShapePtr;
    ShapePtr mpShapePtr;
};

} }

#endif  //  OOX_DRAWINGML_SHAPEGROUPCONTEXT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
