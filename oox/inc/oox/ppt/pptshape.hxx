/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: pptshape.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:54:58 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef OOX_PPT_PRESENTATION_PPTSHAPE_HXX
#define OOX_PPT_PRESENTATION_PPTSHAPE_HXX

#include "oox/drawingml/shape.hxx"
#include "oox/ppt/slidepersist.hxx"

namespace oox { namespace ppt {

class PPTShape : public oox::drawingml::Shape
{
    ShapeLocation               meShapeLocation;        // placeholdershapes (mnSubType != 0) on Master are never displayed
    sal_Bool                    mbReferenced;           // placeholdershapes on Layout are displayed only, if they are not referenced
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
            const oox::drawingml::ThemePtr& rxTheme,
            const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& rxShapes,
            const com::sun::star::awt::Rectangle* pShapeRect = 0,
            ::oox::drawingml::ShapeIdMap* pShapeMap = 0 );

    virtual void applyShapeReference( const oox::drawingml::Shape& rReferencedShape );

    void setShapeLocation( const oox::ppt::ShapeLocation eShapeLocation ) { meShapeLocation = eShapeLocation; };
    ShapeLocation getShapeLocation() const { return meShapeLocation; };
    sal_Bool isReferenced() const { return mbReferenced; };
    void setReferenced( sal_Bool bReferenced ){ mbReferenced = bReferenced; };

protected:

};

} }

#endif  //  OOX_PPT_PRESENTATION_PPTSHAPE_HXX
