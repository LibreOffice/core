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

#include <map>
#include <memory>
#include <vector>

#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <oox/dllapi.h>
#include <oox/drawingml/color.hxx>
#include <oox/drawingml/drawingmltypes.hxx>
#include <oox/helper/helper.hxx>
#include <oox/helper/propertymap.hxx>
#include <rtl/ustring.hxx>
#include <sal/types.h>

namespace basegfx { class B2DHomMatrix; }

namespace com { namespace sun { namespace star {
    namespace awt { struct Rectangle; }
    namespace drawing { class XShape; }
    namespace drawing { class XShapes; }
    namespace uno { class Any; }
} } }

namespace oox { namespace core {
    class XmlFilterBase;
} }

namespace oox { namespace vml {
    struct OleObjectInfo;
} }

namespace oox { namespace drawingml {

class Theme;
struct EffectProperties;
struct FillProperties;
struct GraphicProperties;
struct LineProperties;
struct Shape3DProperties;

class CustomShapeProperties;
typedef std::shared_ptr< CustomShapeProperties > CustomShapePropertiesPtr;

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
    bool const   mbEmbedShapes;      ///< True = load chart shapes into chart, false = load into parent drawpage.

    explicit     ChartShapeInfo( bool bEmbedShapes ) : mbEmbedShapes( bEmbedShapes ) {}
};

/// Attributes for a linked textbox.
struct LinkedTxbxAttr
{
    sal_Int32 id;
    sal_Int32 seq;
    LinkedTxbxAttr(): id(0),seq(0){};
};

class OOX_DLLPUBLIC Shape
    : public std::enable_shared_from_this< Shape >
{
public:

    explicit Shape( const sal_Char* pServiceType = nullptr, bool bDefaultHeight = true );
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

    CustomShapePropertiesPtr&       getCustomShapeProperties(){ return mpCustomShapePropertiesPtr; }

    Shape3DProperties&              get3DProperties() { return *mp3DPropertiesPtr; }
    const Shape3DProperties&        get3DProperties() const { return *mp3DPropertiesPtr; }

    table::TablePropertiesPtr const & getTableProperties();

    EffectProperties&               getEffectProperties() const { return *mpEffectPropertiesPtr; }

    void                            setChildPosition( css::awt::Point nPosition ){ maChPosition = nPosition; }
    void                            setChildSize( css::awt::Size aSize ){ maChSize = aSize; }

    void                            setPosition( css::awt::Point nPosition ){ maPosition = nPosition; }
    const css::awt::Point&          getPosition() const { return maPosition; }

    void                            setSize( css::awt::Size aSize ){ maSize = aSize; }
    const css::awt::Size&           getSize() const { return maSize; }

    void                            setRotation( sal_Int32 nRotation ) { mnRotation = nRotation; }
    sal_Int32                       getRotation() const { return mnRotation; }
    void                            setDiagramRotation( sal_Int32 nRotation ) { mnDiagramRotation = nRotation; }
    void                            setFlip( bool bFlipH, bool bFlipV ) { mbFlipH = bFlipH; mbFlipV = bFlipV; }
    void                            addChild( const ShapePtr& rChildPtr ) { maChildren.push_back( rChildPtr ); }
    std::vector< ShapePtr >&        getChildren() { return maChildren; }

    void                            setName( const OUString& rName ) { msName = rName; }
    const OUString&                 getName( ) { return msName; }
    void                            setInternalName( const OUString& rInternalName ) { msInternalName = rInternalName; }
    const OUString&                 getInternalName() const { return msInternalName; }
    void                            setId( const OUString& rId ) { msId = rId; }
    const OUString&                 getId() { return msId; }
    void                            setDescription( const OUString& rDescr ) { msDescription = rDescr; }
    void                            setHidden( bool bHidden ) { mbHidden = bHidden; }
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
    const TextBodyPtr&   getTextBody() { return mpTextBody;}
    void                setMasterTextListStyle( const TextListStylePtr& pMasterTextListStyle );
    const TextListStylePtr&  getMasterTextListStyle() const { return mpMasterTextListStyle; }

    ShapeStyleRefMap&        getShapeStyleRefs() { return maShapeStyleRefs; }
    const ShapeStyleRefMap&  getShapeStyleRefs() const { return maShapeStyleRefs; }
    const ShapeStyleRef*            getShapeStyleRef( sal_Int32 nRefType ) const;

    // addShape is creating and inserting the corresponding XShape.
    void                addShape(
                            ::oox::core::XmlFilterBase& rFilterBase,
                            const Theme* pTheme,
                            const css::uno::Reference< css::drawing::XShapes >& rxShapes,
                            const basegfx::B2DHomMatrix& aTransformation,
                            FillProperties& rShapeOrParentShapeFillProps,
                            ShapeIdMap* pShapeMap = nullptr,
                            bool bInGroup = false);

    const css::uno::Reference< css::drawing::XShape > &
                        getXShape() const { return mxShape; }

    void                applyShapeReference( const Shape& rReferencedShape, bool bUseText = true );
    const ::std::vector<OUString>&
                        getExtDrawings() { return maExtDrawings; }
    void                addExtDrawingRelId( const OUString &rRelId ) { maExtDrawings.push_back( rRelId ); }
    // Set font color only for extdrawings.
    void                setFontRefColorForNodes(const Color& rColor) { maFontRefColorForNodes = rColor; }
    const Color&        getFontRefColorForNodes() const { return maFontRefColorForNodes; }
    void                setLockedCanvas(bool bLockedCanvas);
    bool                getLockedCanvas() { return mbLockedCanvas;}
    void                setWps(bool bWps);
    bool                getWps() { return mbWps;}
    void                setTextBox(bool bTextBox);
    const css::uno::Sequence<css::beans::PropertyValue> &
                        getDiagramDoms() { return maDiagramDoms; }
    void                setDiagramDoms(const css::uno::Sequence<css::beans::PropertyValue>& rDiagramDoms) { maDiagramDoms = rDiagramDoms; }
    css::uno::Sequence< css::uno::Sequence< css::uno::Any > >resolveRelationshipsOfTypeFromOfficeDoc(
                                                                          core::XmlFilterBase& rFilter, const OUString& sFragment, const OUString& sType );
    void                setLinkedTxbxAttributes(const LinkedTxbxAttr& rhs){ maLinkedTxbxAttr = rhs; };
    void                setTxbxHasLinkedTxtBox( const bool rhs){ mbHasLinkedTxbx = rhs; };
    const LinkedTxbxAttr&     getLinkedTxbxAttributes() { return maLinkedTxbxAttr; };
    bool                isLinkedTxbx() { return mbHasLinkedTxbx; };

    void setZOrder(sal_Int32 nZOrder) { mnZOrder = nZOrder; }

    sal_Int32 getZOrder() const { return mnZOrder; }

    void setZOrderOff(sal_Int32 nZOrderOff) { mnZOrderOff = nZOrderOff; }

    sal_Int32 getZOrderOff() const { return mnZOrderOff; }

    void setDataNodeType(sal_Int32 nDataNodeType) { mnDataNodeType = nDataNodeType; }

    sal_Int32 getDataNodeType() const { return mnDataNodeType; }

    void setAspectRatio(double fAspectRatio) { mfAspectRatio = fAspectRatio; }

    double getAspectRatio() const { return mfAspectRatio; }

    /// Changes reference semantics to value semantics for fill properties.
    void cloneFillProperties();

    void keepDiagramDrawing(::oox::core::XmlFilterBase& rFilterBase, const OUString& rFragmentPath);

protected:

    enum FrameType
    {
        FRAMETYPE_GENERIC, ///< Generic shape, no special type.
        FRAMETYPE_OLEOBJECT, ///< OLE object embedded in a shape.
        FRAMETYPE_CHART, ///< Chart embedded in a shape.
        FRAMETYPE_DIAGRAM, ///< Complex diagram drawing shape.
        FRAMETYPE_TABLE ///< A table embedded in a shape.
    };

    css::uno::Reference< css::drawing::XShape > const &
                        createAndInsert(
                            ::oox::core::XmlFilterBase& rFilterBase,
                            const OUString& rServiceName,
                            const Theme* pTheme,
                            const css::uno::Reference< css::drawing::XShapes >& rxShapes,
                            bool bClearText,
                            bool bDoNotInsertEmptyTextBody,
                            basegfx::B2DHomMatrix& aTransformation,
                            FillProperties& rShapeOrParentShapeFillProps,
                            bool bInGroup = false
                             );

    void                addChildren(
                            ::oox::core::XmlFilterBase& rFilterBase,
                            Shape& rMaster,
                            const Theme* pTheme,
                            const css::uno::Reference< css::drawing::XShapes >& rxShapes,
                            ShapeIdMap* pShapeMap,
                            const basegfx::B2DHomMatrix& aTransformation );

    void                keepDiagramCompatibilityInfo();
    void                convertSmartArtToMetafile( ::oox::core::XmlFilterBase const& rFilterBase );

    css::uno::Reference< css::drawing::XShape >
                        renderDiagramToGraphic( ::oox::core::XmlFilterBase const & rFilterBase );

    OUString finalizeServiceName(
                            ::oox::core::XmlFilterBase& rFilter,
                            const OUString& rServiceName,
                            const css::awt::Rectangle& rShapeRect );

    virtual void        finalizeXShape(
                            ::oox::core::XmlFilterBase& rFilter,
                            const css::uno::Reference< css::drawing::XShapes >& rxShapes );

    void                putPropertyToGrabBag(
                            const OUString& sPropertyName, const css::uno::Any& aPropertyValue );
    void                putPropertyToGrabBag(
                            const css::beans::PropertyValue& pProperty );
    void                putPropertiesToGrabBag(
                            const css::uno::Sequence< css::beans::PropertyValue >& aProperties );

    FillProperties      getActualFillProperties(const Theme* pTheme, const FillProperties* pParentShapeFillProps) const;
    LineProperties      getActualLineProperties(const Theme* pTheme) const;
    EffectProperties    getActualEffectProperties(const Theme* pTheme) const;

    std::vector< ShapePtr >     maChildren;               // only used for group shapes
    css::awt::Size   maChSize;                 // only used for group shapes
    css::awt::Point  maChPosition;             // only used for group shapes

    TextBodyPtr                 mpTextBody;
    LinePropertiesPtr           mpLinePropertiesPtr;
    LinePropertiesPtr           mpShapeRefLinePropPtr;
    FillPropertiesPtr           mpFillPropertiesPtr;
    FillPropertiesPtr           mpShapeRefFillPropPtr;
    GraphicPropertiesPtr        mpGraphicPropertiesPtr;
    CustomShapePropertiesPtr    mpCustomShapePropertiesPtr;
    table::TablePropertiesPtr   mpTablePropertiesPtr;
    Shape3DPropertiesPtr        mp3DPropertiesPtr;
    EffectPropertiesPtr         mpEffectPropertiesPtr;
    EffectPropertiesPtr         mpShapeRefEffectPropPtr;
    PropertyMap                 maShapeProperties;
    PropertyMap                 maDefaultShapeProperties;
    TextListStylePtr            mpMasterTextListStyle;
    css::uno::Reference< css::drawing::XShape > mxShape;

    OUString                    msServiceName;
    OUString                    msName;
    OUString                    msInternalName; // used by diagram; not displayed in UI
    OUString                    msId;
    OUString                    msDescription;
    sal_Int32                   mnSubType;      // if this type is not zero, then the shape is a placeholder
    OptValue< sal_Int32 >       moSubTypeIndex;

    ShapeStyleRefMap            maShapeStyleRefs;

    css::awt::Size              maSize;
    css::awt::Point             maPosition;
    ::std::vector<OUString>     maExtDrawings;
    Color                       maFontRefColorForNodes;

    FrameType                   meFrameType; ///< Type for graphic frame shapes.

private:

    typedef std::shared_ptr< ::oox::vml::OleObjectInfo >    OleObjectInfoRef;
    typedef std::shared_ptr< ChartShapeInfo >               ChartShapeInfoRef;

    OleObjectInfoRef    mxOleObjectInfo;    ///< Additional data for OLE objects.
    ChartShapeInfoRef   mxChartShapeInfo;   ///< Additional data for chart shapes.

    sal_Int32                       mnRotation;
    sal_Int32                       mnDiagramRotation; // rotates shape prior to sizing, does not affect text rotation
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

    css::uno::Sequence<css::beans::PropertyValue> maDiagramDoms;

    /// Z-Order.
    sal_Int32 mnZOrder = 0;

    /// Z-Order offset.
    sal_Int32 mnZOrderOff = 0;

    /// Type of data node for an in-diagram shape.
    sal_Int32 mnDataNodeType = 0;

    /// Aspect ratio for an in-diagram shape.
    double mfAspectRatio = 0;
};

} }

#endif // INCLUDED_OOX_DRAWINGML_SHAPE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
