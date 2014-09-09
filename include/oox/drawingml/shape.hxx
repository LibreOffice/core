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

#ifndef INCLUDED_OOX_DRAWINGML_SHAPE_HXX
#define INCLUDED_OOX_DRAWINGML_SHAPE_HXX

#include <oox/helper/propertymap.hxx>
#include <oox/drawingml/drawingmltypes.hxx>
#include <oox/drawingml/customshapeproperties.hxx>
#include <oox/drawingml/textliststyle.hxx>
#include <oox/drawingml/shape3dproperties.hxx>

#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <vector>
#include <map>
#include <oox/dllapi.h>

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
    ShapeStyleRef() : mnThemedIdx(0) {}
};

typedef ::std::map< sal_Int32, ShapeStyleRef > ShapeStyleRefMap;

/** Additional information for a chart embedded in a drawing shape. */
struct ChartShapeInfo
{
    OUString     maFragmentPath;     ///< Path to related XML stream, e.g. for charts.
    bool                mbEmbedShapes;      ///< True = load chart shapes into chart, false = load into parent drawpage.

    explicit     ChartShapeInfo( bool bEmbedShapes ) : mbEmbedShapes( bEmbedShapes ) {}
};

/// Attributes for a linked textbox.
struct LinkedTxbxAttr
{
    sal_Int32 id;
    sal_Int32 seq;
    LinkedTxbxAttr(): id(0),seq(0){};
    ~LinkedTxbxAttr(){};
};

class OOX_DLLPUBLIC Shape
    : public boost::enable_shared_from_this< Shape >
{
public:

    explicit Shape( const sal_Char* pServiceType = 0, bool bDefaultHeight = true );
    explicit Shape( const ShapePtr& pSourceShape );
    virtual ~Shape();

    OUString&                  getServiceName(){ return msServiceName; }
    void                            setServiceName( const sal_Char* pServiceName );

    PropertyMap&                    getShapeProperties(){ return maShapeProperties; }

    LineProperties&          getLineProperties() { return *mpLinePropertiesPtr; }
    const LineProperties&    getLineProperties() const { return *mpLinePropertiesPtr; }

    FillProperties&          getFillProperties() { return *mpFillPropertiesPtr; }
    const FillProperties&    getFillProperties() const { return *mpFillPropertiesPtr; }

    GraphicProperties&       getGraphicProperties() { return *mpGraphicPropertiesPtr; }
    const GraphicProperties& getGraphicProperties() const { return *mpGraphicPropertiesPtr; }

    CustomShapePropertiesPtr        getCustomShapeProperties(){ return mpCustomShapePropertiesPtr; }

    Shape3DProperties&              get3DProperties() { return *mp3DPropertiesPtr; }
    const Shape3DProperties&        get3DProperties() const { return *mp3DPropertiesPtr; }

    table::TablePropertiesPtr       getTableProperties();

    EffectProperties&               getEffectProperties() { return *mpEffectPropertiesPtr; }

    void                              setChildPosition( com::sun::star::awt::Point nPosition ){ maChPosition = nPosition; }
    void                              setChildSize( com::sun::star::awt::Size aSize ){ maChSize = aSize; }

    void                              setPosition( com::sun::star::awt::Point nPosition ){ maPosition = nPosition; }
    const com::sun::star::awt::Point& getPosition() const { return maPosition; }

    void                              setSize( com::sun::star::awt::Size aSize ){ maSize = aSize; }
    const com::sun::star::awt::Size&  getSize() const { return maSize; }

    void                            setRotation( sal_Int32 nRotation ) { mnRotation = nRotation; }
    void                            setFlip( bool bFlipH, bool bFlipV ) { mbFlipH = bFlipH; mbFlipV = bFlipV; }
    void                            addChild( const ShapePtr pChildPtr ) { maChildren.push_back( pChildPtr ); }
    std::vector< ShapePtr >&        getChildren() { return maChildren; }

    void                            setName( const OUString& rName ) { msName = rName; }
    OUString                       getName( ) { return msName; }
    void                            setId( const OUString& rId ) { msId = rId; }
    OUString                        getId() { return msId; }
    void                            setHidden( bool bHidden ) { mbHidden = bHidden; }
    bool                            getHidden() const { return mbHidden; };
    void                            setHiddenMasterShape( bool bHiddenMasterShape ) { mbHiddenMasterShape = bHiddenMasterShape; }
    void                            setSubType( sal_Int32 nSubType ) { mnSubType = nSubType; }
    sal_Int32                       getSubType() const { return mnSubType; }
    void                            setSubTypeIndex( sal_Int32 nSubTypeIndex ) { moSubTypeIndex = nSubTypeIndex; }
    const OptValue< sal_Int32 >&    getSubTypeIndex() const { return moSubTypeIndex; }

    // setDefaults has to be called if styles are imported (OfficeXML is not storing properties having the default value)
    void                            setDefaults(bool bHeight);

    ::oox::vml::OleObjectInfo&      setOleObjectType();
    ChartShapeInfo&                 setChartType( bool bEmbedShapes );
    void                            setDiagramType();
    void                            setTableType();

    void                setTextBody(const TextBodyPtr & pTextBody);
    TextBodyPtr         getTextBody();
    void                setMasterTextListStyle( const TextListStylePtr& pMasterTextListStyle );
    TextListStylePtr    getMasterTextListStyle() const { return mpMasterTextListStyle; }

    ShapeStyleRefMap&        getShapeStyleRefs() { return maShapeStyleRefs; }
    const ShapeStyleRefMap&  getShapeStyleRefs() const { return maShapeStyleRefs; }
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
    // Set font color only for extdrawings.
    void                setFontRefColorForNodes(const Color& rColor) { maFontRefColorForNodes = rColor; }
    const Color&        getFontRefColorForNodes() const { return maFontRefColorForNodes; }
    void                setLockedCanvas(bool bLockedCanvas);
    bool                getLockedCanvas();
    void                setWps(bool bWps);
    bool                getWps();
    void                setTextBox(bool bTextBox);
    const com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue> &
                        getDiagramDoms() { return maDiagramDoms; }
    void                setDiagramDoms(const com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue>& rDiagramDoms) { maDiagramDoms = rDiagramDoms; }
    com::sun::star::uno::Sequence< com::sun::star::uno::Sequence< com::sun::star::uno::Any > >resolveRelationshipsOfTypeFromOfficeDoc(
                                                                          core::XmlFilterBase& rFilter, const OUString& sFragment, const OUString& sType );
    void                setLinkedTxbxAttributes(const LinkedTxbxAttr& rhs){ maLinkedTxbxAttr = rhs; };
    void                setTxbxHasLinkedTxtBox( const bool rhs){ mbHasLinkedTxbx = rhs; };
    const LinkedTxbxAttr&     getLinkedTxbxAttributes() { return maLinkedTxbxAttr; };
    bool                isLinkedTxbx() { return mbHasLinkedTxbx; };

protected:

    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
                        createAndInsert(
                            ::oox::core::XmlFilterBase& rFilterBase,
                            const OUString& rServiceName,
                            const Theme* pTheme,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& rxShapes,
                            const ::com::sun::star::awt::Rectangle* pShapeRect,
                            bool bClearText,
                            bool bDoNotInsertEmptyTextBody,
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

    void                keepDiagramCompatibilityInfo( ::oox::core::XmlFilterBase& rFilterBase );

    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
                        renderDiagramToGraphic( ::oox::core::XmlFilterBase& rFilterBase );

    virtual OUString finalizeServiceName(
                            ::oox::core::XmlFilterBase& rFilter,
                            const OUString& rServiceName,
                            const ::com::sun::star::awt::Rectangle& rShapeRect );

    virtual void        finalizeXShape(
                            ::oox::core::XmlFilterBase& rFilter,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& rxShapes );

    void                putPropertyToGrabBag(
                            const OUString& sPropertyName, const ::com::sun::star::uno::Any& aPropertyValue );
    void                putPropertyToGrabBag(
                            const ::com::sun::star::beans::PropertyValue& pProperty );
    void                putPropertiesToGrabBag(
                            const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aProperties );

    std::vector< ShapePtr >     maChildren;               // only used for group shapes
    com::sun::star::awt::Size   maChSize;                 // only used for group shapes
    com::sun::star::awt::Point  maChPosition;             // only used for group shapes
    com::sun::star::awt::Size   maAbsoluteSize;           // only used for group shapes
    com::sun::star::awt::Point  maAbsolutePosition;       // only used for group shapes
    bool                        mbIsChild;

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
    Color                           maFontRefColorForNodes;

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
    bool                            mbFlipH;
    bool                            mbFlipV;
    bool                            mbHidden;
    bool                            mbHiddenMasterShape; // master shapes can be hidden in layout slides
                                                         // we need separate flag because we don't want
                                                         // to propagate it when applying reference shape
    bool mbLockedCanvas; ///< Is this shape part of a locked canvas?
    bool mbWps; ///< Is this a wps shape?
    bool mbTextBox; ///< This shape has a textbox.
    LinkedTxbxAttr                  maLinkedTxbxAttr;
    bool                            mbHasLinkedTxbx; // this text box has linked text box ?

    com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue> maDiagramDoms;
};

} }

#endif // INCLUDED_OOX_DRAWINGML_SHAPE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
