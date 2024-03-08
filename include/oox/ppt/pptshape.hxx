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

#include <vector>

#include <com/sun/star/uno/Reference.hxx>
#include <oox/drawingml/shape.hxx>
#include <oox/drawingml/drawingmltypes.hxx>
#include <oox/ppt/slidepersist.hxx>
#include <rtl/ustring.hxx>
#include <sal/types.h>

namespace basegfx { class B2DHomMatrix; }

namespace com::sun::star {
    namespace awt { struct Rectangle; }
    namespace drawing { class XShapes; }
}

namespace oox {
    namespace core { class XmlFilterBase; }
    namespace drawingml { class Theme; }
}

namespace oox::ppt {

class PPTShape final : public oox::drawingml::Shape
{
    OUString                    msModelId;              // fallback dgs smartart shape reference
    ShapeLocation               meShapeLocation;        // placeholdershapes (mnSubType != 0) on Master are never displayed
    bool                        mbReferenced;           // placeholdershapes on Layout are displayed only, if they are not referenced
                                                        // placeholdershapes on Slide are displayed always
    oox::drawingml::ShapePtr mpPlaceholder;
    /// Set if spPr tag is non empty for the shape
    bool mbHasNoninheritedShapeProperties;

    bool IsPlaceHolderCandidate(const SlidePersist& rSlidePersist) const;

public:

    PPTShape( const oox::ppt::ShapeLocation eShapeLocation,
                const OUString& rServiceType );
    virtual ~PPTShape() override;

    using oox::drawingml::Shape::addShape;
    // addShape is creating and inserting the corresponding XShape.
    void addShape(
            oox::core::XmlFilterBase& rFilterBase,
            const SlidePersist& rPersist,
            const oox::drawingml::Theme* pTheme,
            const css::uno::Reference< css::drawing::XShapes >& rxShapes,
            basegfx::B2DHomMatrix& aTransformation,
            ::oox::drawingml::ShapeIdMap* pShapeMap );

    ShapeLocation getShapeLocation() const { return meShapeLocation; };
    void setReferenced( bool bReferenced ){ mbReferenced = bReferenced; };
    void setPlaceholder(const oox::drawingml::ShapePtr& pPlaceholder) { mpPlaceholder = pPlaceholder; }
    void setModelId( const OUString& rId ) { msModelId = rId; }

    /// Flags shape as having a non-empty spPr tag
    void setHasNoninheritedShapeProperties() { mbHasNoninheritedShapeProperties = true; }
    /// Returns whether or not the shape had a non-empty spPr tag
    bool hasNonInheritedShapeProperties() const { return mbHasNoninheritedShapeProperties; }

    static oox::drawingml::ShapePtr findPlaceholder( const sal_Int32 nFirstSubType,
            const sal_Int32 nSecondSubType, const std::optional< sal_Int32 >& oSubTypeIndex,
            std::vector< oox::drawingml::ShapePtr >& rShapes, bool bMasterOnly = false );
    static oox::drawingml::ShapePtr findPlaceholderByIndex( const sal_Int32 nIdx,
            std::vector< oox::drawingml::ShapePtr >& rShapes, bool bMasterOnly = false );

    static oox::drawingml::TextListStylePtr getSubTypeTextListStyle( const SlidePersist& rSlidePersist, sal_Int32 nSubType );
};

}

#endif // INCLUDED_OOX_PPT_PPTSHAPE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
