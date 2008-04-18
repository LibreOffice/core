/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: shape.hxx,v $
 * $Revision: 1.5 $
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

#ifndef OOX_DRAWINGML_SHAPE_HXX
#define OOX_DRAWINGML_SHAPE_HXX

#include "oox/helper/propertymap.hxx"
#include "oox/drawingml/theme.hxx"
#include "oox/drawingml/lineproperties.hxx"
#include "oox/drawingml/fillproperties.hxx"
#include "oox/drawingml/customshapeproperties.hxx"
#include "oox/drawingml/textbody.hxx"
#include "oox/drawingml/textliststyle.hxx"

#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <vector>
#include <map>

namespace oox { namespace drawingml {

class Shape;

typedef boost::shared_ptr< Shape > ShapePtr;

typedef ::std::map< ::rtl::OUString, ShapePtr > ShapeIdMap;

enum ShapeStyle
{
    SHAPESTYLE_ln,
    SHAPESTYLE_fill,
    SHAPESTYLE_effect,
    SHAPESTYLE_font
};
typedef std::map< ShapeStyle, ColorPtr > ShapeStylesColorMap;
typedef std::map< ShapeStyle, rtl::OUString > ShapeStylesIndexMap;

// ============================================================================

/** A callback that will be called after the ::com::sun::drawing::XShape has been
    created from the imported shape and it has been inserted into the draw page.

    An instance of a derived class of this callback can be set at every
    ::oox::drawingml::Shape instance to implement anything that needs a created
    and inserted XShape.
 */
class CreateShapeCallback
{
public:
    virtual             ~CreateShapeCallback();
    virtual void        onCreateXShape(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& rxShape ) = 0;
};
typedef ::boost::shared_ptr< CreateShapeCallback > CreateShapeCallbackRef;

// ============================================================================

class Shape
    : public boost::enable_shared_from_this< Shape >
{
public:

    Shape( const sal_Char* pServiceType = NULL );
    virtual ~Shape();

    rtl::OUString&                  getServiceName(){ return msServiceName; }
    void                            setServiceName( const sal_Char* pServiceName );
    PropertyMap&                    getShapeProperties(){ return maShapeProperties; }
    LinePropertiesPtr               getLineProperties(){ return mpLinePropertiesPtr; }
    FillPropertiesPtr               getFillProperties(){ return mpFillPropertiesPtr; }
    FillPropertiesPtr               getGraphicProperties() { return mpGraphicPropertiesPtr; }
    CustomShapePropertiesPtr        getCustomShapeProperties(){ return mpCustomShapePropertiesPtr; }

    void                            setPosition( com::sun::star::awt::Point nPosition ){ maPosition = nPosition; }
    void                            setSize( com::sun::star::awt::Size aSize ){ maSize = aSize; }
    void                            setRotation( sal_Int32 nRotation ) { mnRotation = nRotation; }
    void                            setFlip( sal_Bool bFlipH, sal_Bool bFlipV ) { mbFlipH = bFlipH; mbFlipV = bFlipV; }
    void                            addChild( const ShapePtr pChildPtr ) { maChilds.push_back( pChildPtr ); }
    std::vector< ShapePtr >&        getChilds() { return maChilds; }

    void                            setName( const rtl::OUString& rName ) { msName = rName; }
    ::rtl::OUString                 getName( ) { return msName; }
    void                            setId( const rtl::OUString& rId ) { msId = rId; }
    void                            setSubType( sal_uInt32 nSubType ) { mnSubType = nSubType; }
    sal_Int32                       getSubType() const { return mnSubType; }
    void                            setIndex( sal_uInt32 nIndex ) { mnIndex = nIndex; }

    // setDefaults has to be called if styles are imported (OfficeXML is not storing properties having the default value)
    void                            setDefaults();

    void                setTextBody(const TextBodyPtr & pTextBody);
    TextBodyPtr         getTextBody();
    void                setMasterTextListStyle( const TextListStylePtr& pMasterTextListStyle );
    TextListStylePtr    getMasterTextListStyle() const { return mpMasterTextListStyle; }


    ShapeStylesColorMap&    getShapeStylesColor(){ return maShapeStylesColorMap; }
    ShapeStylesIndexMap&    getShapeStylesIndex(){ return maShapeStylesIndexMap; }

    inline void         setCreateShapeCallback( CreateShapeCallbackRef xCallback ) { mxCreateCallback = xCallback; }

    // addShape is creating and inserting the corresponding XShape.
    void                addShape(
                            const oox::core::XmlFilterBase& rFilterBase,
                            const ThemePtr& rxTheme,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& rxShapes,
                            const ::com::sun::star::awt::Rectangle* pShapeRect = 0,
                            ShapeIdMap* pShapeMap = 0 );

    const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > &
                        getXShape() const { return mxShape; }

    virtual void        applyShapeReference( const Shape& rReferencedShape );

protected:

    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
                        createAndInsert(
                            const ::oox::core::XmlFilterBase& rFilterBase,
                            const ::rtl::OUString& rServiceName,
                            const ThemePtr& rxTheme,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& rxShapes,
                            const ::com::sun::star::awt::Rectangle* pShapeRect );

    void                addChilds(
                            const ::oox::core::XmlFilterBase& rFilterBase,
                            Shape& rMaster,
                            const ThemePtr& rxTheme,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& rxShapes,
                            const ::com::sun::star::awt::Rectangle& rClientRect,
                            ShapeIdMap* pShapeMap );

    std::vector< ShapePtr >     maChilds;               // only used for group shapes
    TextBodyPtr                 mpTextBody;
    LinePropertiesPtr           mpLinePropertiesPtr;
    FillPropertiesPtr           mpFillPropertiesPtr;
    FillPropertiesPtr           mpGraphicPropertiesPtr;
    CustomShapePropertiesPtr    mpCustomShapePropertiesPtr;
    PropertyMap                 maShapeProperties;
    TextListStylePtr            mpMasterTextListStyle;
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > mxShape;

    rtl::OUString   msServiceName;
    rtl::OUString    msName;
    rtl::OUString    msId;
    sal_uInt32      mnSubType;      // if this type is not zero, then the shape is a placeholder
    sal_uInt32      mnIndex;

    ShapeStylesColorMap maShapeStylesColorMap;
    ShapeStylesIndexMap maShapeStylesIndexMap;

    com::sun::star::awt::Size       maSize;
    com::sun::star::awt::Point      maPosition;

private:

    void setShapeStyles( const ThemePtr& rxTheme, LineProperties& rLineProperties, FillProperties& rFillProperties );
    void setShapeStyleColors( const ::oox::core::XmlFilterBase& rFilterBase,
            LineProperties& rLineProperties, FillProperties& rFillProperties, PropertyMap& rShapeProperties );

    CreateShapeCallbackRef          mxCreateCallback;
    sal_Int32                       mnRotation;
    sal_Bool                        mbFlipH;
    sal_Bool                        mbFlipV;
};

::rtl::OUString GetShapeType( sal_Int32 nType );

} }

#endif  //  OOX_DRAWINGML_SHAPE_HXX
