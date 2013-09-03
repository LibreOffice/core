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

#ifndef OOX_DRAWINGML_SHAPE_HXX
#define OOX_DRAWINGML_SHAPE_HXX

#include "oox/helper/propertymap.hxx"
#include "oox/drawingml/drawingmltypes.hxx"
#include "oox/drawingml/customshapeproperties.hxx"
#include "oox/drawingml/textliststyle.hxx"
#include "oox/drawingml/shape3dproperties.hxx"

#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <vector>
#include <map>
#include "oox/dllapi.h"

namespace oox { namespace vml {
    struct OleObjectInfo;
} }

namespace oox { namespace drawingml {

class CustomShapeProperties;
typedef boost::shared_ptr< CustomShapeProperties > CustomShapePropertiesPtr;

typedef ::std::map< OUString, ShapePtr > ShapeIdMap;

struct ShapeStyleRef
{
    Color               maPhClr;
    sal_Int32           mnThemedIdx;
};

typedef ::std::map< sal_Int32, ShapeStyleRef > ShapeStyleRefMap;

// ============================================================================

/** Additional information for a chart embedded in a drawing shape. */
struct ChartShapeInfo
{
    OUString     maFragmentPath;     ///< Path to related XML stream, e.g. for charts.
    bool                mbEmbedShapes;      ///< True = load chart shapes into chart, false = load into parent drawpage.

    inline explicit     ChartShapeInfo( bool bEmbedShapes ) : mbEmbedShapes( bEmbedShapes ) {}
};

// ============================================================================

class OOX_DLLPUBLIC Shape
    : public boost::enable_shared_from_this< Shape >
{
public:

    explicit Shape( const sal_Char* pServiceType = 0 );
    explicit Shape( const ShapePtr& pSourceShape );
    virtual ~Shape();

    OUString&                  getServiceName(){ return msServiceName; }
    void                            setServiceName( const sal_Char* pServiceName );

    PropertyMap&                    getShapeProperties(){ return maShapeProperties; }

    inline LineProperties&          getLineProperties() { return *mpLinePropertiesPtr; }
    inline const LineProperties&    getLineProperties() const { return *mpLinePropertiesPtr; }

    inline FillProperties&          getFillProperties() { return *mpFillPropertiesPtr; }
    inline const FillProperties&    getFillProperties() const { return *mpFillPropertiesPtr; }

    inline GraphicProperties&       getGraphicProperties() { return *mpGraphicPropertiesPtr; }
    inline const GraphicProperties& getGraphicProperties() const { return *mpGraphicPropertiesPtr; }

    CustomShapePropertiesPtr        getCustomShapeProperties(){ return mpCustomShapePropertiesPtr; }

    Shape3DProperties&              get3DProperties() { return *mp3DPropertiesPtr; }
    const Shape3DProperties&        get3DProperties() const { return *mp3DPropertiesPtr; }

    table::TablePropertiesPtr       getTableProperties();

    inline EffectProperties&        getEffectProperties() { return *mpEffectPropertiesPtr; }

    void                              setChildPosition( com::sun::star::awt::Point nPosition ){ maChPosition = nPosition; }
    void                              setChildSize( com::sun::star::awt::Size aSize ){ maChSize = aSize; }
    void                              moveAllToPosition( const com::sun::star::awt::Point &rPoint );

    void                              setPosition( com::sun::star::awt::Point nPosition ){ maPosition = nPosition; }
    const com::sun::star::awt::Point& getPosition() const { return maPosition; }

    void                              setSize( com::sun::star::awt::Size aSize ){ maSize = aSize; }
    const com::sun::star::awt::Size&  getSize() const { return maSize; }

    void                            setRotation( sal_Int32 nRotation ) { mnRotation = nRotation; }
    void                            setFlip( sal_Bool bFlipH, sal_Bool bFlipV ) { mbFlipH = bFlipH; mbFlipV = bFlipV; }
    void                            addChild( const ShapePtr pChildPtr ) { maChildren.push_back( pChildPtr ); }
    std::vector< ShapePtr >&        getChildren() { return maChildren; }

    void                            setName( const OUString& rName ) { msName = rName; }
    OUString                 getName( ) { return msName; }
    void                            setId( const OUString& rId ) { msId = rId; }
    OUString                 getId() { return msId; }
    void                            setHidden( sal_Bool bHidden ) { mbHidden = bHidden; }
    sal_Bool                        getHidden() const { return mbHidden; };
    void                            setHiddenMasterShape( sal_Bool bHiddenMasterShape ) { mbHiddenMasterShape = bHiddenMasterShape; }
    void                            setSubType( sal_Int32 nSubType ) { mnSubType = nSubType; }
    sal_Int32                       getSubType() const { return mnSubType; }
    void                            setSubTypeIndex( sal_Int32 nSubTypeIndex ) { moSubTypeIndex = nSubTypeIndex; }
    const OptValue< sal_Int32 >&    getSubTypeIndex() const { return moSubTypeIndex; }

    // setDefaults has to be called if styles are imported (OfficeXML is not storing properties having the default value)
    void                            setDefaults();

    ::oox::vml::OleObjectInfo&      setOleObjectType();
    ChartShapeInfo&                 setChartType( bool bEmbedShapes );
    void                            setDiagramType();
    void                            setTableType();

    void                setTextBody(const TextBodyPtr & pTextBody);
    TextBodyPtr         getTextBody();
    void                setMasterTextListStyle( const TextListStylePtr& pMasterTextListStyle );
    TextListStylePtr    getMasterTextListStyle() const { return mpMasterTextListStyle; }

    inline ShapeStyleRefMap&        getShapeStyleRefs() { return maShapeStyleRefs; }
    inline const ShapeStyleRefMap&  getShapeStyleRefs() const { return maShapeStyleRefs; }
    const ShapeStyleRef*            getShapeStyleRef( sal_Int32 nRefType ) const;

    // addShape is creating and inserting the corresponding XShape.
    void                addShape(
                            ::oox::core::XmlFilterBase& rFilterBase,
                            const Theme* pTheme,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& rxShapes,
                            basegfx::B2DHomMatrix& aTransformation,
                            FillProperties& rShapeOrParentShapeFillProps,
                            const ::com::sun::star::awt::Rectangle* pShapeRect = 0,
                            ShapeIdMap* pShapeMap = 0 );

    void                dropChildren() { maChildren.clear(); }

    void                addChildren(
                            ::oox::core::XmlFilterBase& rFilterBase,
                            const Theme* pTheme,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& rxShapes,
                            basegfx::B2DHomMatrix& aTransformation,
                            const ::com::sun::star::awt::Rectangle* pShapeRect = 0,
                            ShapeIdMap* pShapeMap = 0 );

    void                setXShape( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& rXShape )
                            { mxShape = rXShape; };
    const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > &
                        getXShape() const { return mxShape; }

    virtual void        applyShapeReference( const Shape& rReferencedShape, bool bUseText = true );
    const ::std::vector<OUString>&
                        getExtDrawings() { return maExtDrawings; }
    void                addExtDrawingRelId( const OUString &rRelId ) { maExtDrawings.push_back( rRelId ); }
    void                setLockedCanvas(bool bLockedCanvas);
    bool                getLockedCanvas();
    const com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue> &
                        getDiagramDoms() { return maDiagramDoms; }
    void                setDiagramDoms(const com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue>& rDiagramDoms) { maDiagramDoms = rDiagramDoms; }

protected:

    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
                        createAndInsert(
                            ::oox::core::XmlFilterBase& rFilterBase,
                            const OUString& rServiceName,
                            const Theme* pTheme,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& rxShapes,
                            const ::com::sun::star::awt::Rectangle* pShapeRect,
                            sal_Bool bClearText,
                            sal_Bool bDoNotInsertEmptyTextBody,
                            basegfx::B2DHomMatrix& aTransformation,
                            FillProperties& rShapeOrParentShapeFillProps
                             );

    void                addChildren(
                            ::oox::core::XmlFilterBase& rFilterBase,
                            Shape& rMaster,
                            const Theme* pTheme,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& rxShapes,
                            const ::com::sun::star::awt::Rectangle& rClientRect,
                            ShapeIdMap* pShapeMap,
                            basegfx::B2DHomMatrix& aTransformation );

    virtual OUString finalizeServiceName(
                            ::oox::core::XmlFilterBase& rFilter,
                            const OUString& rServiceName,
                            const ::com::sun::star::awt::Rectangle& rShapeRect );

    virtual void        finalizeXShape(
                            ::oox::core::XmlFilterBase& rFilter,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& rxShapes );

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
    table::TablePropertiesPtr   mpTablePropertiesPtr;
    Shape3DPropertiesPtr        mp3DPropertiesPtr;
    EffectPropertiesPtr         mpEffectPropertiesPtr;
    PropertyMap                 maShapeProperties;
    PropertyMap                 maDefaultShapeProperties;
    TextListStylePtr            mpMasterTextListStyle;
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > mxShape;

    OUString       msServiceName;
    OUString       msName;
    OUString       msId;
    sal_Int32           mnSubType;      // if this type is not zero, then the shape is a placeholder
    OptValue< sal_Int32 >   moSubTypeIndex;

    ShapeStyleRefMap   maShapeStyleRefs;

    com::sun::star::awt::Size       maSize;
    com::sun::star::awt::Point      maPosition;
    ::std::vector<OUString>    maExtDrawings;

private:
    enum FrameType
    {
        FRAMETYPE_GENERIC,          ///< Generic shape, no special type.
        FRAMETYPE_OLEOBJECT,        ///< OLE object embedded in a shape.
        FRAMETYPE_CHART,            ///< Chart embedded in a shape.
        FRAMETYPE_DIAGRAM,          ///< Complex diagram drawing shape.
        FRAMETYPE_TABLE             ///< A table embedded in a shape.
    };

    typedef ::boost::shared_ptr< ::oox::vml::OleObjectInfo >    OleObjectInfoRef;
    typedef ::boost::shared_ptr< ChartShapeInfo >               ChartShapeInfoRef;

    FrameType           meFrameType;        ///< Type for graphic frame shapes.
    OleObjectInfoRef    mxOleObjectInfo;    ///< Additional data for OLE objects.
    ChartShapeInfoRef   mxChartShapeInfo;   ///< Additional data for chart shapes.

    sal_Int32                       mnRotation;
    sal_Bool                        mbFlipH;
    sal_Bool                        mbFlipV;
    sal_Bool                        mbHidden;
    sal_Bool                        mbHiddenMasterShape; // master shapes can be hidden in layout slides
                                                         // we need separate flag because we don't want
                                                         // to propagate it when applying reference shape
    bool mbLockedCanvas; ///< Is this shape part of a locked canvas?

    com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue> maDiagramDoms;
};

// ============================================================================

} }

#endif  //  OOX_DRAWINGML_SHAPE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
