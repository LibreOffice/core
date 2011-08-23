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

#ifndef OOX_DRAWINGML_SHAPE_HXX
#define OOX_DRAWINGML_SHAPE_HXX

#include "oox/helper/propertymap.hxx"
#include "oox/drawingml/drawingmltypes.hxx"
#include "oox/drawingml/customshapeproperties.hxx"
#include "oox/drawingml/textliststyle.hxx"

#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <vector>
#include <map>

namespace oox { namespace drawingml {

class CustomShapeProperties;
typedef boost::shared_ptr< CustomShapeProperties > CustomShapePropertiesPtr;

typedef ::std::map< ::rtl::OUString, ShapePtr > ShapeIdMap;

struct ShapeStyleRef
{
    Color               maPhClr;
    sal_Int32           mnThemedIdx;
};

typedef ::std::map< sal_Int32, ShapeStyleRef > ShapeStyleRefMap;

// ============================================================================

/** A callback that will be called before and after the API shape is created
    from the imported shape.

    An instance of a derived class of this callback can be set at every
    ::oox::drawingml::Shape instance to implement anything that needs a created
    and inserted XShape.
 */
class CreateShapeCallback
{
public:
    virtual ::rtl::OUString onCreateXShape(
                            const ::rtl::OUString& rServiceName,
                            const ::com::sun::star::awt::Rectangle& rShapeRect );

    virtual void        onXShapeCreated(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& rxShape,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& rxShapes ) const;

    inline const PropertyMap& getShapeProperties() const { return maShapeProps; }

protected:
    explicit            CreateShapeCallback( ::oox::core::XmlFilterBase& rFilter );
    virtual             ~CreateShapeCallback();

protected:
    ::oox::core::XmlFilterBase& mrFilter;
    PropertyMap         maShapeProps;
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

    inline LineProperties&          getLineProperties() { return *mpLinePropertiesPtr; }
    inline const LineProperties&    getLineProperties() const { return *mpLinePropertiesPtr; }

    inline FillProperties&          getFillProperties() { return *mpFillPropertiesPtr; }
    inline const FillProperties&    getFillProperties() const { return *mpFillPropertiesPtr; }

    inline GraphicProperties&       getGraphicProperties() { return *mpGraphicPropertiesPtr; }
    inline const GraphicProperties& getGraphicProperties() const { return *mpGraphicPropertiesPtr; }

    CustomShapePropertiesPtr        getCustomShapeProperties(){ return mpCustomShapePropertiesPtr; }

    table::TablePropertiesPtr		getTableProperties();

    void                              setChildPosition( com::sun::star::awt::Point nPosition ){ maChPosition = nPosition; }
    void                              setChildSize( com::sun::star::awt::Size aSize ){ maChSize = aSize; }

    void                            setPosition( com::sun::star::awt::Point nPosition ){ maPosition = nPosition; }
    void                            setSize( com::sun::star::awt::Size aSize ){ maSize = aSize; }
    void                            setRotation( sal_Int32 nRotation ) { mnRotation = nRotation; }
    void                            setFlip( sal_Bool bFlipH, sal_Bool bFlipV ) { mbFlipH = bFlipH; mbFlipV = bFlipV; }
    void                            addChild( const ShapePtr pChildPtr ) { maChildren.push_back( pChildPtr ); }
    std::vector< ShapePtr >&        getChildren() { return maChildren; }

    void                            setName( const rtl::OUString& rName ) { msName = rName; }
    ::rtl::OUString                 getName( ) { return msName; }
    void                            setId( const rtl::OUString& rId ) { msId = rId; }
    void							setHidden( sal_Bool bHidden ) { mbHidden = bHidden; }
    sal_Bool						getHidden() const { return mbHidden; };
    void                            setSubType( sal_Int32 nSubType ) { mnSubType = nSubType; }
    sal_Int32                       getSubType() const { return mnSubType; }
    void                            setSubTypeIndex( sal_uInt32 nSubTypeIndex ) { mnSubTypeIndex = nSubTypeIndex; }
    sal_Int32						getSubTypeIndex() const { return mnSubTypeIndex; }

    // setDefaults has to be called if styles are imported (OfficeXML is not storing properties having the default value)
    void                            setDefaults();

    void                setTextBody(const TextBodyPtr & pTextBody);
    TextBodyPtr         getTextBody();
    void                setMasterTextListStyle( const TextListStylePtr& pMasterTextListStyle );
    TextListStylePtr    getMasterTextListStyle() const { return mpMasterTextListStyle; }

    inline ShapeStyleRefMap&        getShapeStyleRefs() { return maShapeStyleRefs; }
    inline const ShapeStyleRefMap&  getShapeStyleRefs() const { return maShapeStyleRefs; }
    const ShapeStyleRef*            getShapeStyleRef( sal_Int32 nRefType ) const;

    inline void         setCreateShapeCallback( CreateShapeCallbackRef xCallback ) { mxCreateCallback = xCallback; }

    // addShape is creating and inserting the corresponding XShape.
    void                addShape(
                            const oox::core::XmlFilterBase& rFilterBase,
                            const Theme* pTheme,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& rxShapes,
                            const ::com::sun::star::awt::Rectangle* pShapeRect = 0,
                            ShapeIdMap* pShapeMap = 0 );

    void				setXShape( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& rXShape )
                            { mxShape = rXShape; };
    const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > &
                        getXShape() const { return mxShape; }

    virtual void        applyShapeReference( const Shape& rReferencedShape );

protected:

    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
                        createAndInsert(
                            const ::oox::core::XmlFilterBase& rFilterBase,
                            const ::rtl::OUString& rServiceName,
                            const Theme* pTheme,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& rxShapes,
                            const ::com::sun::star::awt::Rectangle* pShapeRect,
                            sal_Bool bClearText );

    void                addChildren(
                            const ::oox::core::XmlFilterBase& rFilterBase,
                            Shape& rMaster,
                            const Theme* pTheme,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& rxShapes,
                            const ::com::sun::star::awt::Rectangle& rClientRect,
                            ShapeIdMap* pShapeMap );

    std::vector< ShapePtr >     maChildren;               // only used for group shapes
    com::sun::star::awt::Size   maChSize;                 // only used for group shapes
    com::sun::star::awt::Point  maChPosition;             // only used for group shapes
    com::sun::star::awt::Size   maAbsoluteSize;           // only used for group shapes
    com::sun::star::awt::Point  maAbsolutePosition;       // only used for group shapes
    sal_Bool                    mbIsChild;

    TextBodyPtr                 mpTextBody;
    LinePropertiesPtr           mpLinePropertiesPtr;
    FillPropertiesPtr           mpFillPropertiesPtr;
    GraphicPropertiesPtr        mpGraphicPropertiesPtr;
    CustomShapePropertiesPtr    mpCustomShapePropertiesPtr;
    table::TablePropertiesPtr	mpTablePropertiesPtr;
    PropertyMap                 maShapeProperties;
    PropertyMap                 maDefaultShapeProperties;
    TextListStylePtr            mpMasterTextListStyle;
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > mxShape;

    rtl::OUString		msServiceName;
    rtl::OUString		msName;
    rtl::OUString		msId;
    sal_Int32			mnSubType;      // if this type is not zero, then the shape is a placeholder
    sal_Int32			mnSubTypeIndex;

    ShapeStyleRefMap   maShapeStyleRefs;

    com::sun::star::awt::Size       maSize;
    com::sun::star::awt::Point      maPosition;

private:
    CreateShapeCallbackRef          mxCreateCallback;
    sal_Int32                       mnRotation;
    sal_Bool                        mbFlipH;
    sal_Bool                        mbFlipV;
    sal_Bool						mbHidden;
};

::rtl::OUString GetShapeType( sal_Int32 nType );

} }

#endif  //  OOX_DRAWINGML_SHAPE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
