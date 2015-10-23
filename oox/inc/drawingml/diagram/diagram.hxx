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

#ifndef INCLUDED_OOX_DRAWINGML_DIAGRAM_DIAGRAM_HXX
#define INCLUDED_OOX_DRAWINGML_DIAGRAM_DIAGRAM_HXX

#include <rtl/ustring.hxx>
#include <oox/drawingml/shape.hxx>
#include <oox/core/xmlfilterbase.hxx>

#include <com/sun/star/xml/dom/XDocument.hpp>

namespace oox { namespace drawingml {

/** load diagram data, and put resulting graphic into shape

    This method loads the diagram data fragments from the given paths,
    generate and layout the shapes, and push it as children into the
    referenced shape.
 */
void loadDiagram( ShapePtr& pShape,
                  core::XmlFilterBase& rFilter,
                  const OUString& rDataModelPath,
                  const OUString& rLayoutPath,
                  const OUString& rQStylePath,
                  const OUString& rColorStylePath );

void loadDiagram( const ShapePtr& pShape,
                  core::XmlFilterBase& rFilter,
                  const css::uno::Reference< css::xml::dom::XDocument>& rXDataModelDom,
                  const css::uno::Reference< css::xml::dom::XDocument>& rXLayoutDom,
                  const css::uno::Reference< css::xml::dom::XDocument>& rXQStyleDom,
                  const css::uno::Reference< css::xml::dom::XDocument>& rXColorStyleDom );
} }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
