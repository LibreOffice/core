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

#ifndef INCLUDED_OOX_PPT_PPTSHAPE_HXX
#define INCLUDED_OOX_PPT_PPTSHAPE_HXX

#include <oox/drawingml/shape.hxx>
#include <oox/ppt/slidepersist.hxx>

namespace oox { namespace ppt {

class PPTShape : public oox::drawingml::Shape
{
    OUString                    msModelId;              // fallback dgs smartart shape reference
    ShapeLocation               meShapeLocation;        // placeholdershapes (mnSubType != 0) on Master are never displayed
    bool                        mbReferenced;           // placeholdershapes on Layout are displayed only, if they are not referenced
                                                        // placeholdershapes on Slide are displayed always

public:

    PPTShape( const oox::ppt::ShapeLocation eShapeLocation,
                const sal_Char* pServiceType = nullptr );
    virtual ~PPTShape();

    using oox::drawingml::Shape::addShape;
    // addShape is creating and inserting the corresponding XShape.
    void addShape(
            oox::core::XmlFilterBase& rFilterBase,
            SlidePersist& rPersist,
            const oox::drawingml::Theme* pTheme,
            const css::uno::Reference< css::drawing::XShapes >& rxShapes,
            basegfx::B2DHomMatrix& aTransformation,
            const css::awt::Rectangle* pShapeRect = nullptr,
            ::oox::drawingml::ShapeIdMap* pShapeMap = nullptr );

    virtual void applyShapeReference( const oox::drawingml::Shape& rReferencedShape, bool bUseText = true ) override;

    ShapeLocation getShapeLocation() const { return meShapeLocation; };
    void setReferenced( bool bReferenced ){ mbReferenced = bReferenced; };
    void setPlaceholder( oox::drawingml::ShapePtr pPlaceholder ) { mpPlaceholder = pPlaceholder; }
    void setModelId( const OUString& rId ) { msModelId = rId; }

    static oox::drawingml::ShapePtr findPlaceholder( const sal_Int32 nFirstSubType,
            const sal_Int32 nSecondSubType, const OptValue< sal_Int32 >& oSubTypeIndex,
            std::vector< oox::drawingml::ShapePtr >& rShapes, bool bMasterOnly = false );
    static oox::drawingml::ShapePtr findPlaceholderByIndex( const sal_Int32 nIdx,
            std::vector< oox::drawingml::ShapePtr >& rShapes, bool bMasterOnly = false );

    static oox::drawingml::TextListStylePtr getSubTypeTextListStyle( const SlidePersist& rSlidePersist, sal_Int32 nSubType );

protected:

    oox::drawingml::ShapePtr mpPlaceholder;
};

} }

#endif // INCLUDED_OOX_PPT_PPTSHAPE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
