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

#ifndef OOX_PPT_PRESENTATION_PPTSHAPE_HXX
#define OOX_PPT_PRESENTATION_PPTSHAPE_HXX

#include "oox/drawingml/shape.hxx"
#include "oox/ppt/slidepersist.hxx"

namespace oox { namespace ppt {

class PPTShape : public oox::drawingml::Shape
{
    ShapeLocation				meShapeLocation;		// placeholdershapes (mnSubType != 0) on Master are never displayed
    sal_Bool					mbReferenced;			// placeholdershapes on Layout are displayed only, if they are not referenced
                                                        // placeholdershapes on Slide are displayed always

public:

    PPTShape( const oox::ppt::ShapeLocation eShapeLocation,
                const sal_Char* pServiceType = NULL );
    virtual ~PPTShape();

    using oox::drawingml::Shape::addShape;
    // addShape is creating and inserting the corresponding XShape.
    void addShape(
            const oox::core::XmlFilterBase& rFilterBase,
            const SlidePersist& rPersist,
            const oox::drawingml::Theme* pTheme,
            const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& rxShapes,
            const com::sun::star::awt::Rectangle* pShapeRect = 0,
            ::oox::drawingml::ShapeIdMap* pShapeMap = 0 );

    virtual void applyShapeReference( const oox::drawingml::Shape& rReferencedShape );

    void setShapeLocation( const oox::ppt::ShapeLocation eShapeLocation ) { meShapeLocation = eShapeLocation; };
    ShapeLocation getShapeLocation() const { return meShapeLocation; };
    sal_Bool isReferenced() const { return mbReferenced; };
    void setReferenced( sal_Bool bReferenced ){ mbReferenced = bReferenced; };

    static oox::drawingml::ShapePtr findPlaceholder( const sal_Int32 nMasterPlaceholder, std::vector< oox::drawingml::ShapePtr >& rShapes );
    static oox::drawingml::ShapePtr findPlaceholderByIndex( const sal_Int32 nIdx, std::vector< oox::drawingml::ShapePtr >& rShapes );
    static oox::drawingml::ShapePtr findPlaceholder( sal_Int32 nFirstPlaceholder, sal_Int32 nSecondPlaceholder, std::vector< oox::drawingml::ShapePtr >& rShapes );
};

} }

#endif  //  OOX_PPT_PRESENTATION_PPTSHAPE_HXX
