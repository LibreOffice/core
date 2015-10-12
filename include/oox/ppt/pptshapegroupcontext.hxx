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

#ifndef INCLUDED_OOX_PPT_PPTSHAPEGROUPCONTEXT_HXX
#define INCLUDED_OOX_PPT_PPTSHAPEGROUPCONTEXT_HXX

#include <oox/drawingml/shapegroupcontext.hxx>
#include <oox/drawingml/color.hxx>
#include <oox/ppt/slidepersist.hxx>

namespace oox { namespace ppt {

class PPTShapeGroupContext : public ::oox::drawingml::ShapeGroupContext
{
    SlidePersistPtr     mpSlidePersistPtr;
    ShapeLocation       meShapeLocation;
    oox::drawingml::ShapePtr    pGraphicShape;

    void                applyFontRefColor(oox::drawingml::ShapePtr pShape, const oox::drawingml::Color& rFontRefColor);

public:
    PPTShapeGroupContext(
        ::oox::core::ContextHandler2Helper& rParent,
        const oox::ppt::SlidePersistPtr& rSlidePersistPtr,
        const oox::ppt::ShapeLocation eShapeLocation,
        oox::drawingml::ShapePtr pMasterShapePtr,
        oox::drawingml::ShapePtr pGroupShapePtr );

    virtual ::oox::core::ContextHandlerRef
        onCreateContext( ::sal_Int32 Element, const ::oox::AttributeList& rAttribs ) override;

    void importExtDrawings();

protected:

};

} }

#endif // INCLUDED_OOX_PPT_PPTSHAPEGROUPCONTEXT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
