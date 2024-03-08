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

#include <drawingfragment.hxx>

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <comphelper/propertyvalue.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/container/XNameReplace.hpp>
#include <com/sun/star/document/XEventsSupplier.hpp>
#include <com/sun/star/drawing/XControlShape.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/script/ScriptEventDescriptor.hpp>
#include <com/sun/star/script/XEventAttacherManager.hpp>
#include <rtl/strbuf.hxx>
#include <svx/svdobj.hxx>
#include <drwlayer.hxx>
#include <oox/core/filterbase.hxx>
#include <oox/drawingml/connectorshapecontext.hxx>
#include <oox/drawingml/graphicshapecontext.hxx>
#include <oox/helper/attributelist.hxx>
#include <oox/helper/propertyset.hxx>
#include <oox/shape/ShapeDrawingFragmentHandler.hxx>
#include <oox/token/namespaces.hxx>
#include <oox/token/properties.hxx>
#include <oox/token/tokens.hxx>
#include <oox/vml/vmlshape.hxx>
#include <oox/vml/vmlshapecontainer.hxx>
#include <osl/diagnose.h>
#include <o3tl/string_view.hxx>
#include <formulaparser.hxx>
#include <stylesbuffer.hxx>
#include <themebuffer.hxx>
#include <worksheetbuffer.hxx>

namespace oox::xls {

using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::script;
using namespace ::com::sun::star::uno;
using namespace ::oox::core;
using namespace ::oox::drawingml;
using namespace ::oox::ole;

using ::com::sun::star::awt::Size;
using ::com::sun::star::awt::Point;
using ::com::sun::star::awt::Rectangle;
using ::com::sun::star::awt::XControlModel;
// no using's for ::oox::vml, that may clash with ::oox::drawingml types

ShapeMacroAttacher::ShapeMacroAttacher( const OUString& rMacroName, const Reference< XShape >& rxShape ) :
    VbaMacroAttacherBase( rMacroName ),
    mxShape( rxShape )
{
}

void ShapeMacroAttacher::attachMacro( const OUString& rMacroUrl )
{
    try
    {
        Reference< XEventsSupplier > xSupplier( mxShape, UNO_QUERY_THROW );
        Reference< XNameReplace > xEvents( xSupplier->getEvents(), UNO_SET_THROW );
        Sequence aEventProps{ comphelper::makePropertyValue("EventType", OUString( "Script" )),
                              comphelper::makePropertyValue("Script", rMacroUrl) };
        xEvents->replaceByName( "OnClick", Any( aEventProps ) );
    }
    catch( Exception& )
    {
    }
}

Shape::Shape( const WorksheetHelper& rHelper, const AttributeList& rAttribs, const OUString& rServiceName ) :
    ::oox::drawingml::Shape( rServiceName ),
    WorksheetHelper( rHelper )
{
    OUString aMacro = rAttribs.getXString( XML_macro, OUString() );
    if( !aMacro.isEmpty() )
        maMacroName = getFormulaParser().importMacroName( aMacro );
}

void Shape::finalizeXShape( XmlFilterBase& rFilter, const Reference< XShapes >& rxShapes )
{
    OUString sURL;
    getShapeProperties().getProperty( PROP_URL ) >>= sURL;
    getWorksheets().convertSheetNameRef( sURL );
    if( !maMacroName.isEmpty() && mxShape.is() )
    {
        VbaMacroAttacherRef xAttacher = std::make_shared<ShapeMacroAttacher>( maMacroName, mxShape );
        getBaseFilter().getVbaProject().registerMacroAttacher( xAttacher );
    }
    ::oox::drawingml::Shape::finalizeXShape( rFilter, rxShapes );
    if ( !sURL.isEmpty() )
    {
        SdrObject* pObj = SdrObject::getSdrObjectFromXShape( mxShape );
        if ( pObj )
            pObj->setHyperlink(sURL);
    }
}

GroupShapeContext::GroupShapeContext( const FragmentHandler2& rParent,
        const WorksheetHelper& rHelper, const ShapePtr& rxParentShape, const ShapePtr& rxShape ) :
    ShapeGroupContext( rParent, rxParentShape, rxShape ),
    WorksheetHelper( rHelper )
{
}

/*static*/ ContextHandlerRef GroupShapeContext::createShapeContext( FragmentHandler2& rParent,
        const WorksheetHelper& rHelper, sal_Int32 nElement, const AttributeList& rAttribs,
        const ShapePtr& rxParentShape, ShapePtr* pxShape )
{
    switch( nElement )
    {
        case XDR_TOKEN( sp ):
        {
            ShapePtr xShape = std::make_shared<Shape>( rHelper, rAttribs, u"com.sun.star.drawing.CustomShape"_ustr );
            if( pxShape ) *pxShape = xShape;
            return new ShapeContext( rParent, rxParentShape, xShape );
        }
        case XDR_TOKEN( cxnSp ):
        {
            ShapePtr xShape = std::make_shared<Shape>( rHelper, rAttribs, u"com.sun.star.drawing.ConnectorShape"_ustr );
            if( pxShape ) *pxShape = xShape;
            return new ConnectorShapeContext(rParent, rxParentShape, xShape,
                                             xShape->getConnectorShapeProperties());
        }
        case XDR_TOKEN( pic ):
        {
            ShapePtr xShape = std::make_shared<Shape>( rHelper, rAttribs, u"com.sun.star.drawing.GraphicObjectShape"_ustr );
            if( pxShape ) *pxShape = xShape;
            return new GraphicShapeContext( rParent, rxParentShape, xShape );
        }
        case XDR_TOKEN( graphicFrame ):
        {
            ShapePtr xShape = std::make_shared<Shape>( rHelper, rAttribs, u"com.sun.star.drawing.GraphicObjectShape"_ustr );
            if( pxShape ) *pxShape = xShape;
            return new GraphicalObjectFrameContext( rParent, rxParentShape, xShape, rHelper.getSheetType() != WorksheetType::Chart );
        }
        case XDR_TOKEN( grpSp ):
        {
            ShapePtr xShape = std::make_shared<Shape>( rHelper, rAttribs, u"com.sun.star.drawing.GroupShape"_ustr );
            if( pxShape ) *pxShape = xShape;
            return new GroupShapeContext( rParent, rHelper, rxParentShape, xShape );
        }
    }
    return nullptr;
}

ContextHandlerRef GroupShapeContext::onCreateContext(
        sal_Int32 nElement, const AttributeList& rAttribs )
{
    ContextHandlerRef xContext = createShapeContext( *this, *this, nElement, rAttribs, mpGroupShapePtr );
    return xContext ? xContext : ShapeGroupContext::onCreateContext( nElement, rAttribs );
}

DrawingFragment::DrawingFragment( const WorksheetHelper& rHelper, const OUString& rFragmentPath ) :
    WorksheetFragmentBase( rHelper, rFragmentPath ),
    mxDrawPage( rHelper.getDrawPage() )
{
    OSL_ENSURE( mxDrawPage.is(), "DrawingFragment::DrawingFragment - missing drawing page" );
}

ContextHandlerRef DrawingFragment::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    switch( getCurrentElement() )
    {
        case XML_ROOT_CONTEXT:
            if( nElement == XDR_TOKEN( wsDr ) ) return this;
        break;

        case XDR_TOKEN( wsDr ):
            switch( nElement )
            {
                case XDR_TOKEN( absoluteAnchor ):
                case XDR_TOKEN( oneCellAnchor ):
                case XDR_TOKEN( twoCellAnchor ):
                    mxAnchor.reset( new ShapeAnchor( *this ) );
                    mxAnchor->importAnchor( nElement, rAttribs );
                    return this;
            }
        break;

        case XDR_TOKEN( absoluteAnchor ):
        case XDR_TOKEN( oneCellAnchor ):
        case XDR_TOKEN( twoCellAnchor ):
        {
            switch( nElement )
            {
                case XDR_TOKEN( from ):
                case XDR_TOKEN( to ):           return this;

                case XDR_TOKEN( pos ):          if( mxAnchor ) mxAnchor->importPos( rAttribs );           break;
                case XDR_TOKEN( ext ):          if( mxAnchor ) mxAnchor->importExt( rAttribs );           break;
                case XDR_TOKEN( clientData ):   if( mxAnchor ) mxAnchor->importClientData( rAttribs );    break;

                default:                        return GroupShapeContext::createShapeContext( *this, *this, nElement, rAttribs, ShapePtr(), &mxShape );
            }
        }
        break;

        case XDR_TOKEN( from ):
        case XDR_TOKEN( to ):
            switch( nElement )
            {
                case XDR_TOKEN( col ):
                case XDR_TOKEN( row ):
                case XDR_TOKEN( colOff ):
                case XDR_TOKEN( rowOff ):       return this;    // collect index in onCharacters()
            }
        break;
    }
    return nullptr;
}

void DrawingFragment::onCharacters( const OUString& rChars )
{
    switch( getCurrentElement() )
    {
        case XDR_TOKEN( col ):
        case XDR_TOKEN( row ):
        case XDR_TOKEN( colOff ):
        case XDR_TOKEN( rowOff ):
            if( mxAnchor ) mxAnchor->setCellPos( getCurrentElement(), getParentElement(), rChars );
        break;
    }
}

void DrawingFragment::onEndElement()
{
    switch( getCurrentElement() )
    {
        case XDR_TOKEN( absoluteAnchor ):
        case XDR_TOKEN( oneCellAnchor ):
        case XDR_TOKEN( twoCellAnchor ):
            if( mxDrawPage.is() && mxShape && mxAnchor )
            {
                EmuRectangle aShapeRectEmu = mxAnchor->calcAnchorRectEmu( getDrawPageSize() );
                const bool bIsShapeVisible = mxAnchor->isAnchorValid();
                if( (aShapeRectEmu.X >= 0) && (aShapeRectEmu.Y >= 0) && (aShapeRectEmu.Width >= 0) && (aShapeRectEmu.Height >= 0) )
                {
                    const sal_Int32 aRotation = mxShape->getRotation();
                    if ((aRotation >= 45  * PER_DEGREE && aRotation < 135 * PER_DEGREE)
                     || (aRotation >= 225 * PER_DEGREE && aRotation < 315 * PER_DEGREE))
                    {
                        // When rotating any shape in MSO Excel within the range of degrees given above,
                        // Excel changes the cells in which the shape is anchored. The new position of
                        // the anchors are always calculated using a 90 degrees rotation anticlockwise.
                        // There is an important result of this operation: the top left point of the shape changes,
                        // it will be another vertex.
                        // The anchor position is given in the xml file, it is in the xdr:from and xdr:to elements.
                        // Let's see what happens in time order:
                        // We create a shape in Excel, the anchor position is in a given cell, then the rotation happens
                        // as mentioned above, and excel recalculates the cells in which the anchors are positioned.
                        // This new cell is exported into the xml elements xdr:from and xdr:to, when Excel exports the document!
                        // Thus, if we have a 90 degrees rotation and an already rotated point from which we base
                        // our calculations here in LO, the result is an incorrect 180 degrees rotation.
                        // Now, we need to create the bounding rectangle of the shape with this in mind.
                        // (Important to mention that at this point we don't talk about rotations at all, this bounding
                        // rectangle contains the original not-rotated shape. Rotation happens later in the code.)
                        // We get the new (x, y) coords, then swap width with height.
                        // To get the new coords we reflect the rectangle in the line y = x. (This will return the
                        // correct vertex, which is the actual top left one.)
                        // Another fact that appears to be true in Excel is that there are only 2 of possible anchor
                        // positions for a shape that is only rotated (and not resized for example).
                        // The first position happens in the set of degrees {[45, 135) U [225, 315)} and the second
                        // set is all the other angles. The two sets partition the circle (of all rotations: 360 degrees).
                        sal_Int64 nHalfWidth = aShapeRectEmu.Width / 2;
                        sal_Int64 nHalfHeight = aShapeRectEmu.Height / 2;
                        aShapeRectEmu.X = aShapeRectEmu.X + nHalfWidth - nHalfHeight;
                        aShapeRectEmu.Y = aShapeRectEmu.Y + nHalfHeight - nHalfWidth;
                        std::swap(aShapeRectEmu.Width, aShapeRectEmu.Height);
                    }

                    // TODO: DrawingML implementation expects 32-bit coordinates for EMU rectangles (change that to EmuRectangle)
                    // tdf#135918: Negative X,Y position has to be allowed to avoid shape displacement on rotation.
                    // The negative values can exist because of previous lines where the anchor rectangle must be mirrored in some ranges.
                    Rectangle aShapeRectEmu32(
                        getLimitedValue< sal_Int32, sal_Int64 >( aShapeRectEmu.X, SAL_MIN_INT32, SAL_MAX_INT32 ),
                        getLimitedValue< sal_Int32, sal_Int64 >( aShapeRectEmu.Y, SAL_MIN_INT32, SAL_MAX_INT32 ),
                        getLimitedValue< sal_Int32, sal_Int64 >( aShapeRectEmu.Width, 0, SAL_MAX_INT32 ),
                        getLimitedValue< sal_Int32, sal_Int64 >( aShapeRectEmu.Height, 0, SAL_MAX_INT32 ) );

                    // Make sure to set the position and size *before* calling addShape().
                    mxShape->setPosition(Point(aShapeRectEmu32.X, aShapeRectEmu32.Y));
                    mxShape->setSize(Size(aShapeRectEmu32.Width, aShapeRectEmu32.Height));

                    // tdf#83671. Because Excel saves a diagram with zero size in xdr:xfm, the
                    // initial diagram import produces a background shape with zero size and no
                    // diagram shapes at all. Here the size has been determined from the anchor and
                    // thus repeating the import of diagram.xml gives the diagram shapes.
                    if (mxShape->getDiagramDoms().getLength() > 0
                        && mxShape->getChildren().size() == 1
                        && mxShape->getExtDrawings().size() == 1)
                    {
                        mxShape->getChildren()[0]->setSize(mxShape->getSize());
                        OUString sFragmentPath(
                            getFragmentPathFromRelId(mxShape->getExtDrawings()[0]));
                        // Don't know why importFragment looses shape name and id. Rescue them.
                        OUString sBackupName(mxShape->getName());
                        OUString sBackupId(mxShape->getId());
                        getOoxFilter().importFragment(new oox::shape::ShapeDrawingFragmentHandler(
                            getOoxFilter(), sFragmentPath, mxShape));
                        mxShape->setName(sBackupName);
                        mxShape->setId(sBackupId);
                    }

                    if (mxShape->getFontRefColorForNodes().isUsed())
                          applyFontRefColor(mxShape, mxShape->getFontRefColorForNodes());

                    basegfx::B2DHomMatrix aTransformation;
                    if ( !bIsShapeVisible)
                        mxShape->setHidden(true);

                    mxShape->addShape( getOoxFilter(), &getTheme(), mxDrawPage, aTransformation, mxShape->getFillProperties() );

                    /*  Collect all shape positions in the WorksheetHelper base
                        class. But first, scale EMUs to 1/100 mm. */
                    Rectangle aShapeRectHmm(
                        convertEmuToHmm(aShapeRectEmu32.X > 0 ? aShapeRectEmu32.X : 0), convertEmuToHmm(aShapeRectEmu32.Y > 0 ? aShapeRectEmu32.Y : 0),
                        convertEmuToHmm(aShapeRectEmu32.Width ), convertEmuToHmm(aShapeRectEmu32.Height ) );
                    extendShapeBoundingBox( aShapeRectHmm );
                    // set cell Anchoring
                    if ( mxAnchor->getEditAs() != ShapeAnchor::ANCHOR_ABSOLUTE )
                    {
                        SdrObject* pObj = SdrObject::getSdrObjectFromXShape( mxShape->getXShape() );
                        if ( pObj )
                        {
                            bool bResizeWithCell = mxAnchor->getEditAs() == ShapeAnchor::ANCHOR_TWOCELL;
                            ScDrawLayer::SetCellAnchoredFromPosition( *pObj, getScDocument(), getSheetIndex(), bResizeWithCell );
                        }
                    }
                }
            }
            mxShape.reset();
            mxAnchor.reset();
        break;
    }
}

void DrawingFragment::applyFontRefColor(const oox::drawingml::ShapePtr& pShape,
                                        const oox::drawingml::Color& rFontRefColor)
{
    pShape->getShapeStyleRefs()[XML_fontRef].maPhClr = rFontRefColor;
    std::vector<oox::drawingml::ShapePtr>& vChildren = pShape->getChildren();
    for (auto const& child : vChildren)
    {
        applyFontRefColor(child, rFontRefColor);
    }
}

// VML

namespace {

class VmlFindNoteFunc
{
public:
    explicit            VmlFindNoteFunc( const ScAddress& rPos );
    bool                operator()( const ::oox::vml::ShapeBase& rShape ) const;

private:
    sal_Int32           mnCol;
    sal_Int32           mnRow;
};

VmlFindNoteFunc::VmlFindNoteFunc( const ScAddress& rPos ) :
    mnCol( rPos.Col() ),
    mnRow( rPos.Row() )
{
}

bool VmlFindNoteFunc::operator()( const ::oox::vml::ShapeBase& rShape ) const
{
    const ::oox::vml::ClientData* pClientData = rShape.getClientData();
    return pClientData && (pClientData->mnCol == mnCol) && (pClientData->mnRow == mnRow);
}

} // namespace

VmlControlMacroAttacher::VmlControlMacroAttacher( const OUString& rMacroName,
        const Reference< XIndexContainer >& rxCtrlFormIC, sal_Int32 nCtrlIndex, sal_Int32 nCtrlType, sal_Int32 nDropStyle ) :
    VbaMacroAttacherBase( rMacroName ),
    mxCtrlFormIC( rxCtrlFormIC ),
    mnCtrlIndex( nCtrlIndex ),
    mnCtrlType( nCtrlType ),
    mnDropStyle( nDropStyle )
{
}

void VmlControlMacroAttacher::attachMacro( const OUString& rMacroUrl )
{
    ScriptEventDescriptor aEventDesc;
    aEventDesc.ScriptType = "Script";
    aEventDesc.ScriptCode = rMacroUrl;

    // editable drop downs are treated like edit boxes
    bool bEditDropDown = (mnCtrlType == XML_Drop) && (mnDropStyle == XML_ComboEdit);
    sal_Int32 nCtrlType = bEditDropDown ? XML_Edit : mnCtrlType;

    switch( nCtrlType )
    {
        case XML_Button:
        case XML_Checkbox:
        case XML_Radio:
            aEventDesc.ListenerType = "XActionListener";
            aEventDesc.EventMethod = "actionPerformed";
        break;
        case XML_Label:
        case XML_GBox:
        case XML_Dialog:
            aEventDesc.ListenerType = "XMouseListener";
            aEventDesc.EventMethod = "mouseReleased";
        break;
        case XML_Edit:
            aEventDesc.ListenerType = "XTextListener";
            aEventDesc.EventMethod = "textChanged";
        break;
        case XML_Spin:
        case XML_Scroll:
            aEventDesc.ListenerType = "XAdjustmentListener";
            aEventDesc.EventMethod = "adjustmentValueChanged";
        break;
        case XML_List:
        case XML_Drop:
            aEventDesc.ListenerType = "XChangeListener";
            aEventDesc.EventMethod = "changed";
        break;
        default:
            OSL_ENSURE( false, "VmlControlMacroAttacher::attachMacro - unexpected object type" );
            return;
    }

    try
    {
        Reference< XEventAttacherManager > xEventMgr( mxCtrlFormIC, UNO_QUERY_THROW );
        xEventMgr->registerScriptEvent( mnCtrlIndex, aEventDesc );
    }
    catch( Exception& )
    {
    }
}

VmlDrawing::VmlDrawing( const WorksheetHelper& rHelper ) :
    ::oox::vml::Drawing( rHelper.getOoxFilter(), rHelper.getDrawPage(), ::oox::vml::VMLDRAWING_EXCEL ),
    WorksheetHelper( rHelper ),
    maControlConv( rHelper.getBaseFilter().getModel(), rHelper.getBaseFilter().getGraphicHelper() )
{
    // default font for legacy listboxes and dropdowns: Tahoma, 8pt
    maListBoxFont.moName = "Tahoma";
    maListBoxFont.moColor = "auto";
    maListBoxFont.monSize = 160;
}

const ::oox::vml::ShapeBase* VmlDrawing::getNoteShape( const ScAddress& rPos ) const
{
    return getShapes().findShape( VmlFindNoteFunc( rPos ) );
}

bool VmlDrawing::isShapeSupported( const ::oox::vml::ShapeBase& rShape ) const
{
    const ::oox::vml::ClientData* pClientData = rShape.getClientData();
    return !pClientData || (pClientData->mnObjType != XML_Note);
}

OUString VmlDrawing::getShapeBaseName( const ::oox::vml::ShapeBase& rShape ) const
{
    if( const ::oox::vml::ClientData* pClientData = rShape.getClientData() )
    {
        switch( pClientData->mnObjType )
        {
            case XML_Button:    return "Button";
            case XML_Checkbox:  return "Check Box";
            case XML_Dialog:    return "Dialog Frame";
            case XML_Drop:      return "Drop Down";
            case XML_Edit:      return "Edit Box";
            case XML_GBox:      return "Group Box";
            case XML_Label:     return "Label";
            case XML_List:      return "List Box";
            case XML_Note:      return "Comment";
            case XML_Pict:      return (pClientData->mbDde || getOleObjectInfo( rShape.getShapeId() )) ? OUString( "Object" ) : OUString( "Picture" );
            case XML_Radio:     return "Option Button";
            case XML_Scroll:    return "Scroll Bar";
            case XML_Spin:      return "Spinner";
        }
    }
    return ::oox::vml::Drawing::getShapeBaseName( rShape );
}

bool VmlDrawing::convertClientAnchor( Rectangle& orShapeRect, const OUString& rShapeAnchor ) const
{
    if( rShapeAnchor.isEmpty() )
        return false;
    ShapeAnchor aAnchor( *this );
    aAnchor.importVmlAnchor( rShapeAnchor );
    orShapeRect = aAnchor.calcAnchorRectHmm( getDrawPageSize() );
    return (orShapeRect.Width >= 0) && (orShapeRect.Height >= 0);
}

Reference< XShape > VmlDrawing::createAndInsertClientXShape( const ::oox::vml::ShapeBase& rShape,
        const Reference< XShapes >& rxShapes, const Rectangle& rShapeRect ) const
{
    // simulate the legacy drawing controls with OLE form controls
    OUString aShapeName = rShape.getShapeName();
    const ::oox::vml::ClientData* pClientData = rShape.getClientData();
    if( !aShapeName.isEmpty() && pClientData )
    {
        Rectangle aShapeRect = rShapeRect;
        const ::oox::vml::TextBox* pTextBox = rShape.getTextBox();
        EmbeddedControl aControl( aShapeName );
        switch( pClientData->mnObjType )
        {
            case XML_Button:
            {
                AxCommandButtonModel& rAxModel = aControl.createModel< AxCommandButtonModel >();
                convertControlText( rAxModel.maFontData, rAxModel.mnTextColor, rAxModel.maCaption, pTextBox, pClientData->mnTextHAlign );
                rAxModel.mnFlags = AX_FLAGS_ENABLED | AX_FLAGS_OPAQUE | AX_FLAGS_WORDWRAP;
                rAxModel.mnVerticalAlign = pClientData->mnTextVAlign;
            }
            break;

            case XML_Label:
            {
                AxLabelModel& rAxModel = aControl.createModel< AxLabelModel >();
                convertControlText( rAxModel.maFontData, rAxModel.mnTextColor, rAxModel.maCaption, pTextBox, pClientData->mnTextHAlign );
                rAxModel.mnFlags = AX_FLAGS_ENABLED | AX_FLAGS_WORDWRAP;
                rAxModel.mnBorderStyle = AX_BORDERSTYLE_NONE;
                rAxModel.mnSpecialEffect = AX_SPECIALEFFECT_FLAT;
                rAxModel.mnVerticalAlign = pClientData->mnTextVAlign;
            }
            break;

            case XML_Edit:
            {
                bool bNumeric = (pClientData->mnVTEdit == ::oox::vml::VML_CLIENTDATA_INTEGER) || (pClientData->mnVTEdit == ::oox::vml::VML_CLIENTDATA_NUMBER);
                AxMorphDataModelBase& rAxModel = bNumeric ?
                    static_cast< AxMorphDataModelBase& >( aControl.createModel< AxNumericFieldModel >() ) :
                    static_cast< AxMorphDataModelBase& >( aControl.createModel< AxTextBoxModel >() );
                convertControlText( rAxModel.maFontData, rAxModel.mnTextColor, rAxModel.maValue, pTextBox, pClientData->mnTextHAlign );
                setFlag( rAxModel.mnFlags, AX_FLAGS_MULTILINE, pClientData->mbMultiLine );
                setFlag( rAxModel.mnScrollBars, AX_SCROLLBAR_VERTICAL, pClientData->mbVScroll );
                if( pClientData->mbSecretEdit )
                    rAxModel.mnPasswordChar = '*';
            }
            break;

            case XML_GBox:
            {
                AxFrameModel& rAxModel = aControl.createModel< AxFrameModel >();
                convertControlText( rAxModel.maFontData, rAxModel.mnTextColor, rAxModel.maCaption, pTextBox, pClientData->mnTextHAlign );
                rAxModel.mnBorderStyle = pClientData->mbNo3D ? AX_BORDERSTYLE_SINGLE : AX_BORDERSTYLE_NONE;
                rAxModel.mnSpecialEffect = pClientData->mbNo3D ? AX_SPECIALEFFECT_FLAT : AX_SPECIALEFFECT_BUMPED;

                /*  Move top border of groupbox up by half font height, because
                    Excel specifies Y position of the groupbox border line
                    instead the top border of the caption text. */
                if( const ::oox::vml::TextFontModel* pFontModel = pTextBox ? pTextBox->getFirstFont() : nullptr )
                {
                    sal_Int32 nFontHeightHmm = o3tl::convert( pFontModel->monSize.value_or( 160 ), o3tl::Length::twip, o3tl::Length::mm100 );
                    sal_Int32 nYDiff = ::std::min< sal_Int32 >( nFontHeightHmm / 2, aShapeRect.Y );
                    aShapeRect.Y -= nYDiff;
                    aShapeRect.Height += nYDiff;
                }
            }
            break;

            case XML_Checkbox:
            {
                AxCheckBoxModel& rAxModel = aControl.createModel< AxCheckBoxModel >();
                convertControlText( rAxModel.maFontData, rAxModel.mnTextColor, rAxModel.maCaption, pTextBox, pClientData->mnTextHAlign );
                convertControlBackground( rAxModel, rShape );
                rAxModel.maValue = OUString::number( pClientData->mnChecked );
                rAxModel.mnSpecialEffect = pClientData->mbNo3D ? AX_SPECIALEFFECT_FLAT : AX_SPECIALEFFECT_SUNKEN;
                rAxModel.mnVerticalAlign = pClientData->mnTextVAlign;
                bool bTriState = (pClientData->mnChecked != ::oox::vml::VML_CLIENTDATA_UNCHECKED) && (pClientData->mnChecked != ::oox::vml::VML_CLIENTDATA_CHECKED);
                rAxModel.mnMultiSelect = bTriState ? AX_SELECTION_MULTI : AX_SELECTION_SINGLE;
            }
            break;

            case XML_Radio:
            {
                AxOptionButtonModel& rAxModel = aControl.createModel< AxOptionButtonModel >();

                // unique name to prevent autoGrouping with ActiveX controls and which a GroupBox may override - see vmldrawing.cxx.
                rAxModel.maGroupName = "autoGroup_formControl";
                convertControlText( rAxModel.maFontData, rAxModel.mnTextColor, rAxModel.maCaption, pTextBox, pClientData->mnTextHAlign );
                convertControlBackground( rAxModel, rShape );
                rAxModel.maValue = OUString::number( pClientData->mnChecked );
                rAxModel.mnSpecialEffect = pClientData->mbNo3D ? AX_SPECIALEFFECT_FLAT : AX_SPECIALEFFECT_SUNKEN;
                rAxModel.mnVerticalAlign = pClientData->mnTextVAlign;
            }
            break;

            case XML_List:
            {
                AxListBoxModel& rAxModel = aControl.createModel< AxListBoxModel >();
                convertControlFontData( rAxModel.maFontData, rAxModel.mnTextColor, maListBoxFont );
                rAxModel.mnBorderStyle = pClientData->mbNo3D2 ? AX_BORDERSTYLE_SINGLE : AX_BORDERSTYLE_NONE;
                rAxModel.mnSpecialEffect = pClientData->mbNo3D2 ? AX_SPECIALEFFECT_FLAT : AX_SPECIALEFFECT_SUNKEN;
                switch( pClientData->mnSelType )
                {
                    case XML_Single:    rAxModel.mnMultiSelect = AX_SELECTION_SINGLE;    break;
                    case XML_Multi:     rAxModel.mnMultiSelect = AX_SELECTION_MULTI;     break;
                    case XML_Extend:    rAxModel.mnMultiSelect = AX_SELECTION_EXTENDED;  break;
                }
            }
            break;

            case XML_Drop:
            {
                AxComboBoxModel& rAxModel = aControl.createModel< AxComboBoxModel >();
                convertControlFontData( rAxModel.maFontData, rAxModel.mnTextColor, maListBoxFont );
                rAxModel.mnDisplayStyle = AX_DISPLAYSTYLE_DROPDOWN;
                rAxModel.mnShowDropButton = AX_SHOWDROPBUTTON_ALWAYS;
                rAxModel.mnBorderStyle = pClientData->mbNo3D2 ? AX_BORDERSTYLE_SINGLE : AX_BORDERSTYLE_NONE;
                rAxModel.mnSpecialEffect = pClientData->mbNo3D2 ? AX_SPECIALEFFECT_FLAT : AX_SPECIALEFFECT_SUNKEN;
                rAxModel.mnListRows = pClientData->mnDropLines;
            }
            break;

            case XML_Spin:
            {
                AxSpinButtonModel& rAxModel = aControl.createModel< AxSpinButtonModel >();
                rAxModel.mnMin = pClientData->mnMin;
                rAxModel.mnMax = pClientData->mnMax;
                rAxModel.mnPosition = pClientData->mnVal;
                rAxModel.mnSmallChange = pClientData->mnInc;
            }
            break;

            case XML_Scroll:
            {
                AxScrollBarModel& rAxModel = aControl.createModel< AxScrollBarModel >();
                rAxModel.mnMin = pClientData->mnMin;
                rAxModel.mnMax = pClientData->mnMax;
                rAxModel.mnPosition = pClientData->mnVal;
                rAxModel.mnSmallChange = pClientData->mnInc;
                rAxModel.mnLargeChange = pClientData->mnPage;
            }
            break;

            case XML_Dialog:
            {
                // fake with a group box
                AxFrameModel& rAxModel = aControl.createModel< AxFrameModel >();
                convertControlText( rAxModel.maFontData, rAxModel.mnTextColor, rAxModel.maCaption, pTextBox, XML_Left );
                rAxModel.mnBorderStyle = AX_BORDERSTYLE_SINGLE;
                rAxModel.mnSpecialEffect = AX_SPECIALEFFECT_FLAT;
            }
            break;
        }

        if( ControlModelBase* pAxModel = aControl.getModel() )
        {
            // create the control shape
            pAxModel->maSize.first = aShapeRect.Width;
            pAxModel->maSize.second = aShapeRect.Height;
            sal_Int32 nCtrlIndex = -1;
            Reference< XShape > xShape = createAndInsertXControlShape( aControl, rxShapes, aShapeRect, nCtrlIndex );

            // control shape macro
            if( xShape.is() && (nCtrlIndex >= 0) && !pClientData->maFmlaMacro.isEmpty() )
            {
                OUString aMacroName = getFormulaParser().importMacroName( pClientData->maFmlaMacro );
                if( !aMacroName.isEmpty() )
                {
                    Reference< XIndexContainer > xFormIC = getControlForm().getXForm();
                    VbaMacroAttacherRef xAttacher = std::make_shared<VmlControlMacroAttacher>( aMacroName, xFormIC, nCtrlIndex, pClientData->mnObjType, pClientData->mnDropStyle );
                    getBaseFilter().getVbaProject().registerMacroAttacher( xAttacher );
                }
            }

            return xShape;
        }
    }

    return Reference< XShape >();
}

void VmlDrawing::notifyXShapeInserted( const Reference< XShape >& rxShape,
        const Rectangle& rShapeRect, const ::oox::vml::ShapeBase& rShape, bool bGroupChild )
{
    // collect all shape positions in the WorksheetHelper base class (but not children of group shapes)
    if( !bGroupChild )
        extendShapeBoundingBox( rShapeRect );

    // convert settings from VML client data
    const ::oox::vml::ClientData* pClientData = rShape.getClientData();
    if(!pClientData)
        return;

    // specific settings for embedded form controls
    try
    {
        Reference< XControlShape > xCtrlShape( rxShape, UNO_QUERY_THROW );
        Reference< XControlModel > xCtrlModel( xCtrlShape->getControl(), UNO_SET_THROW );
        PropertySet aPropSet( xCtrlModel );

        // printable
        aPropSet.setProperty( PROP_Printable, pClientData->mbPrintObject );

        // control source links
        if( !pClientData->maFmlaLink.isEmpty() || !pClientData->maFmlaRange.isEmpty() )
            maControlConv.bindToSources( xCtrlModel, pClientData->maFmlaLink, pClientData->maFmlaRange, getSheetIndex() );
    }
    catch( Exception& )
    {
    }
}

// private --------------------------------------------------------------------

sal_uInt32 VmlDrawing::convertControlTextColor( std::u16string_view aTextColor ) const
{
    // color attribute not present or 'auto' - use passed default color
    if( aTextColor.empty() || o3tl::equalsIgnoreAsciiCase( aTextColor, u"auto" ) )
        return AX_SYSCOLOR_WINDOWTEXT;

    if( aTextColor[ 0 ] == '#' )
    {
        // RGB colors in the format '#RRGGBB'
        if( aTextColor.size() == 7 )
            return OleHelper::encodeOleColor( o3tl::toUInt32(aTextColor.substr( 1 ), 16) );

        // RGB colors in the format '#RGB'
        if( aTextColor.size() == 4 )
        {
            sal_Int32 nR = o3tl::toUInt32(aTextColor.substr( 1, 1 ), 16) * 0x11;
            sal_Int32 nG = o3tl::toUInt32(aTextColor.substr( 2, 1 ), 16) * 0x11;
            sal_Int32 nB = o3tl::toUInt32(aTextColor.substr( 3, 1 ), 16) * 0x11;
            return OleHelper::encodeOleColor( (nR << 16) | (nG << 8) | nB );
        }

        OSL_ENSURE( false, OStringBuffer( "VmlDrawing::convertControlTextColor - invalid color name '"
                + OUStringToOString( aTextColor, RTL_TEXTENCODING_ASCII_US ) + "\'" ).getStr() );
        return AX_SYSCOLOR_WINDOWTEXT;
    }

    const GraphicHelper& rGraphicHelper = getBaseFilter().getGraphicHelper();

    /*  Predefined color names or system color names (resolve to RGB to detect
        valid color name). */
    sal_Int32 nColorToken = AttributeConversion::decodeToken( aTextColor );
    ::Color nRgbValue = Color::getVmlPresetColor( nColorToken, API_RGB_TRANSPARENT );
    if( nRgbValue == API_RGB_TRANSPARENT )
        nRgbValue = rGraphicHelper.getSystemColor( nColorToken );
    if( nRgbValue != API_RGB_TRANSPARENT )
        return OleHelper::encodeOleColor( nRgbValue );

    // try palette color
    return OleHelper::encodeOleColor( rGraphicHelper.getPaletteColor( o3tl::toInt32(aTextColor) ) );
}

void VmlDrawing::convertControlFontData( AxFontData& rAxFontData, sal_uInt32& rnOleTextColor, const ::oox::vml::TextFontModel& rFontModel ) const
{
    if( rFontModel.moName.has_value() )
        rAxFontData.maFontName = rFontModel.moName.value();

    // font height: convert from twips to points, then to internal representation of AX controls
    rAxFontData.setHeightPoints( static_cast< sal_Int16 >( (rFontModel.monSize.value_or( 200 ) + 10) / 20 ) );

    // font effects
    rAxFontData.mnFontEffects = AxFontFlags::NONE;
    setFlag( rAxFontData.mnFontEffects, AxFontFlags::Bold, rFontModel.mobBold.value_or( false ) );
    setFlag( rAxFontData.mnFontEffects, AxFontFlags::Italic, rFontModel.mobItalic.value_or( false ) );
    setFlag( rAxFontData.mnFontEffects, AxFontFlags::Strikeout, rFontModel.mobStrikeout.value_or( false ) );
    sal_Int32 nUnderline = rFontModel.monUnderline.value_or( XML_none );
    setFlag( rAxFontData.mnFontEffects, AxFontFlags::Underline, nUnderline != XML_none );
    rAxFontData.mbDblUnderline = nUnderline == XML_double;

    // font color
    rnOleTextColor = convertControlTextColor( rFontModel.moColor.value_or( OUString() ) );
}

void VmlDrawing::convertControlText( AxFontData& rAxFontData, sal_uInt32& rnOleTextColor,
        OUString& rCaption, const ::oox::vml::TextBox* pTextBox, sal_Int32 nTextHAlign ) const
{
    if( pTextBox )
    {
        rCaption = pTextBox->getText();
        if( const ::oox::vml::TextFontModel* pFontModel = pTextBox->getFirstFont() )
            convertControlFontData( rAxFontData, rnOleTextColor, *pFontModel );
    }

    switch( nTextHAlign )
    {
        case XML_Left:      rAxFontData.mnHorAlign = AxHorizontalAlign::Left;      break;
        case XML_Center:    rAxFontData.mnHorAlign = AxHorizontalAlign::Center;    break;
        case XML_Right:     rAxFontData.mnHorAlign = AxHorizontalAlign::Right;     break;
        default:            rAxFontData.mnHorAlign = AxHorizontalAlign::Left;
    }
}

void VmlDrawing::convertControlBackground( AxMorphDataModelBase& rAxModel, const ::oox::vml::ShapeBase& rShape ) const
{
    const ::oox::vml::FillModel& rFillModel = rShape.getTypeModel().maFillModel;
    bool bHasFill = rFillModel.moFilled.value_or( true );
    setFlag( rAxModel.mnFlags, AX_FLAGS_OPAQUE, bHasFill );
    if( bHasFill )
    {
        const GraphicHelper& rGraphicHelper = getBaseFilter().getGraphicHelper();
        ::Color nSysWindowColor = rGraphicHelper.getSystemColor( XML_window, API_RGB_WHITE );
        ::oox::drawingml::Color aColor = ::oox::vml::ConversionHelper::decodeColor( rGraphicHelper, rFillModel.moColor, rFillModel.moOpacity, nSysWindowColor );
        ::Color nRgbValue = aColor.getColor( rGraphicHelper );
        rAxModel.mnBackColor = OleHelper::encodeOleColor( nRgbValue );
    }
}

VmlDrawingFragment::VmlDrawingFragment( const WorksheetHelper& rHelper, const OUString& rFragmentPath ) :
    ::oox::vml::DrawingFragment( rHelper.getOoxFilter(), rFragmentPath, rHelper.getVmlDrawing() ),
    WorksheetHelper( rHelper )
{
}

void VmlDrawingFragment::finalizeImport()
{
    ::oox::vml::DrawingFragment::finalizeImport();
    getVmlDrawing().convertAndInsert();
}

} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
