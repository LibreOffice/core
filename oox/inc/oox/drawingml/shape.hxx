/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

namespace oox { namespace vml {
    struct OleObjectInfo;
} }

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

/** Additional information for a chart embedded in a drawing shape. */
struct ChartShapeInfo
{
    ::rtl::OUString     maFragmentPath;     /// Path to related XML stream, e.g. for charts.
    bool                mbEmbedShapes;      /// True = load chart shapes into chart, false = load into parent drawpage.

    inline explicit     ChartShapeInfo( bool bEmbedShapes ) : mbEmbedShapes( bEmbedShapes ) {}
};

// ============================================================================

class Shape
    : public boost::enable_shared_from_this< Shape >
{
public:

    explicit Shape( const sal_Char* pServiceType = 0 );
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

    table::TablePropertiesPtr       getTableProperties();

    void                            setPosition( com::sun::star::awt::Point nPosition ){ maPosition = nPosition; }
    void                            setSize( com::sun::star::awt::Size aSize ){ maSize = aSize; }
    void                            setRotation( sal_Int32 nRotation ) { mnRotation = nRotation; }
    //  TTTT: MirrorX/Y removed
    //void                            setFlip( sal_Bool bFlipH, sal_Bool bFlipV ) { mbFlipH = bFlipH; mbFlipV = bFlipV; }
    void                            addChild( const ShapePtr pChildPtr ) { maChildren.push_back( pChildPtr ); }
    std::vector< ShapePtr >&        getChildren() { return maChildren; }

    void                            setName( const rtl::OUString& rName ) { msName = rName; }
    ::rtl::OUString                 getName( ) { return msName; }
    void                            setId( const rtl::OUString& rId ) { msId = rId; }
    void                            setHidden( sal_Bool bHidden ) { mbHidden = bHidden; }
    sal_Bool                        getHidden() const { return mbHidden; };
    void                            setSubType( sal_Int32 nSubType ) { mnSubType = nSubType; }
    sal_Int32                       getSubType() const { return mnSubType; }
    void                            setSubTypeIndex( sal_uInt32 nSubTypeIndex ) { mnSubTypeIndex = nSubTypeIndex; }
    sal_Int32                       getSubTypeIndex() const { return mnSubTypeIndex; }

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
                            const ::com::sun::star::awt::Rectangle* pShapeRect = 0,
                            ShapeIdMap* pShapeMap = 0 );

    void                setXShape( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& rXShape )
                            { mxShape = rXShape; };
    const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > &
                        getXShape() const { return mxShape; }

    virtual void        applyShapeReference( const Shape& rReferencedShape );

protected:

    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
                        createAndInsert(
                            ::oox::core::XmlFilterBase& rFilterBase,
                            const ::rtl::OUString& rServiceName,
                            const Theme* pTheme,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& rxShapes,
                            const ::com::sun::star::awt::Rectangle* pShapeRect,
                            sal_Bool bClearText );

    void                addChildren(
                            ::oox::core::XmlFilterBase& rFilterBase,
                            Shape& rMaster,
                            const Theme* pTheme,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& rxShapes,
                            const ::com::sun::star::awt::Rectangle& rClientRect,
                            ShapeIdMap* pShapeMap );

    virtual ::rtl::OUString finalizeServiceName(
                            ::oox::core::XmlFilterBase& rFilter,
                            const ::rtl::OUString& rServiceName,
                            const ::com::sun::star::awt::Rectangle& rShapeRect );

    virtual void        finalizeXShape(
                            ::oox::core::XmlFilterBase& rFilter,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& rxShapes );

    std::vector< ShapePtr >     maChildren;               // only used for group shapes
    TextBodyPtr                 mpTextBody;
    LinePropertiesPtr           mpLinePropertiesPtr;
    FillPropertiesPtr           mpFillPropertiesPtr;
    GraphicPropertiesPtr        mpGraphicPropertiesPtr;
    CustomShapePropertiesPtr    mpCustomShapePropertiesPtr;
    table::TablePropertiesPtr   mpTablePropertiesPtr;
    PropertyMap                 maShapeProperties;
    TextListStylePtr            mpMasterTextListStyle;
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > mxShape;

    rtl::OUString       msServiceName;
    rtl::OUString       msName;
    rtl::OUString       msId;
    sal_Int32           mnSubType;      // if this type is not zero, then the shape is a placeholder
    sal_Int32           mnSubTypeIndex;

    ShapeStyleRefMap   maShapeStyleRefs;

    com::sun::star::awt::Size       maSize;
    com::sun::star::awt::Point      maPosition;

private:
    enum FrameType
    {
        FRAMETYPE_GENERIC,          /// Generic shape, no special type.
        FRAMETYPE_OLEOBJECT,        /// OLE object embedded in a shape.
        FRAMETYPE_CHART,            /// Chart embedded in a shape.
        FRAMETYPE_DIAGRAM,          /// Complex diagram drawing shape.
        FRAMETYPE_TABLE             /// A table embedded in a shape.
    };

    typedef ::boost::shared_ptr< ::oox::vml::OleObjectInfo >    OleObjectInfoRef;
    typedef ::boost::shared_ptr< ChartShapeInfo >               ChartShapeInfoRef;

    FrameType           meFrameType;        /// Type for graphic frame shapes.
    OleObjectInfoRef    mxOleObjectInfo;    /// Additional data for OLE objects.
    ChartShapeInfoRef   mxChartShapeInfo;   /// Additional data for chart shapes.

    sal_Int32                       mnRotation;
    //  TTTT: MirrorX/Y removed
    //sal_Bool                        mbFlipH;
    //sal_Bool                        mbFlipV;
    sal_Bool                        mbHidden;
};

::rtl::OUString GetShapeType( sal_Int32 nType );

// ============================================================================

} }

#endif  //  OOX_DRAWINGML_SHAPE_HXX
