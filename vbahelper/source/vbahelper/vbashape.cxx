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
#include <ooo/vba/office/MsoZOrderCmd.hpp>
#include <ooo/vba/office/MsoScaleFrom.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/drawing/ConnectorType.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/text/RelOrientation.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <ooo/vba/office/MsoShapeType.hpp>
#include <ooo/vba/office/MsoAutoShapeType.hpp>
#include <ooo/vba/word/WdRelativeHorizontalPosition.hpp>
#include <ooo/vba/word/WdRelativeVerticalPosition.hpp>

#include <basic/sberrors.hxx>
#include <comphelper/processfactory.hxx>
#include <utility>
#include <vcl/svapp.hxx>
#include <sal/log.hxx>

#include <vbahelper/vbashape.hxx>
#include <vbahelper/vbatextframe.hxx>
#include "vbalineformat.hxx"
#include "vbafillformat.hxx"
#include "vbapictureformat.hxx"
#include <vbahelper/vbashaperange.hxx>

using namespace ::ooo::vba;
using namespace ::com::sun::star;

ScVbaShape::ScVbaShape( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, uno::Reference< drawing::XShape > xShape, uno::Reference< drawing::XShapes > xShapes, uno::Reference< frame::XModel > xModel, sal_Int32 nType )
    : ScVbaShape_BASE( xParent, xContext )
    , m_aShapeHelper( xShape )
    , m_xShape(std::move( xShape ))
    , m_xShapes(std::move( xShapes ))
    , m_nType( nType )
    , m_xModel(std::move( xModel ))
{
    m_xPropertySet.set( m_xShape, uno::UNO_QUERY_THROW );
    addListeners();
}

ScVbaShape::~ScVbaShape()
{
}

void SAL_CALL
ScVbaShape::disposing( const lang::EventObject& rEventObject )
{
    try
    {
    uno::Reference< drawing::XShapes > xShapes( rEventObject.Source, uno::UNO_QUERY );
    uno::Reference< drawing::XShape > xShape( rEventObject.Source, uno::UNO_QUERY );
    if ( xShapes.is() )
        removeShapesListener();
    if ( xShape.is() )
        removeShapeListener();
    }
    catch( uno::Exception& )
    {
    }
}


void ScVbaShape::addListeners()
{
    uno::Reference< lang::XComponent > xComponent( m_xShape, uno::UNO_QUERY );
    if ( xComponent.is() )
        xComponent->addEventListener( this );

    xComponent.set( m_xShapes, uno::UNO_QUERY );
    if ( xComponent.is() )
        xComponent->addEventListener( this );
}

void
ScVbaShape::removeShapeListener()
{
    if( m_xShape.is() )
    {
        uno::Reference< lang::XComponent > xComponent( m_xShape, uno::UNO_QUERY_THROW );
        xComponent->removeEventListener( this );
    }
    m_xShape = nullptr;
    m_xPropertySet = nullptr;
}

void
ScVbaShape::removeShapesListener()
{
    if( m_xShapes.is() )
    {
        uno::Reference< lang::XComponent > xComponent( m_xShapes, uno::UNO_QUERY_THROW );
        xComponent->removeEventListener( this );
    }
    m_xShapes = nullptr;
}

sal_Int32
ScVbaShape::getType( const css::uno::Reference< drawing::XShape >& xShape )
{
    OUString sShapeType;
    uno::Reference< drawing::XShapeDescriptor > xShapeDescriptor( xShape, uno::UNO_QUERY_THROW );
    sShapeType = xShapeDescriptor->getShapeType();
    SAL_INFO("vbahelper", "ScVbaShape::getType: " << sShapeType);
    // office::MsoShapeType::msoDiagram to "com.sun.star.drawing.GroupShape"
    if( sShapeType == "com.sun.star.drawing.GroupShape" )
        return office::MsoShapeType::msoGroup;
    else if( sShapeType == "com.sun.star.drawing.GraphicObjectShape" )
        return office::MsoShapeType::msoPicture;
    else if( sShapeType == "com.sun.star.drawing.ControlShape" || sShapeType == "FrameShape" )
        return office::MsoShapeType::msoOLEControlObject;
    // OOo don't support office::MsoShapeType::msoComment as a Shape.
    else if( sShapeType == "com.sun.star.drawing.OLE2Shape" )
        return office::MsoShapeType::msoChart;
    // Art characters office::MsoShapeType::msoTextEffect, in OOo corresponding to "com.sun.star.drawing.CustomShape"
    else if( sShapeType == "com.sun.star.drawing.ConnectorShape" )
    {
        drawing::ConnectorType connectorType;
        uno::Reference< beans::XPropertySet > xPropertySet( xShape, uno::UNO_QUERY_THROW );
        xPropertySet->getPropertyValue( u"EdgeKind"_ustr ) >>= connectorType;
        if( connectorType == drawing::ConnectorType_CURVE )
            return office::MsoShapeType::msoFreeform;
        else if( connectorType == drawing::ConnectorType_LINE )
            return office::MsoShapeType::msoLine;
        else
            return office::MsoShapeType::msoAutoShape;
    }
    else if( sShapeType == "com.sun.star.drawing.LineShape" )
        return office::MsoShapeType::msoLine;
    else if( sShapeType == "com.sun.star.drawing.CustomShape"
            || sShapeType == "com.sun.star.drawing.RectangleShape"
            || sShapeType == "com.sun.star.drawing.EllipseShape" )
        return office::MsoShapeType::msoAutoShape;
    else if( sShapeType == "com.sun.star.drawing.TextShape" )
        return office::MsoShapeType::msoTextBox;
    else
        throw uno::RuntimeException("the shape type do not be supported: " + sShapeType );
}

sal_Int32 ScVbaShape::getAutoShapeType(const css::uno::Reference< drawing::XShape >& xShape)
{
    assert( ScVbaShape::getType( xShape ) == office::MsoShapeType::msoAutoShape );

    OUString sShapeType;
    uno::Reference< drawing::XShapeDescriptor > xShapeDescriptor( xShape, uno::UNO_QUERY_THROW );
    sShapeType = xShapeDescriptor->getShapeType();
    SAL_INFO("vbahelper", "ScVbaShape::getAutoShapeType: " << sShapeType);

    if( sShapeType == "com.sun.star.drawing.EllipseShape" )
        return office::MsoAutoShapeType::msoShapeOval;
    else if ( sShapeType == "com.sun.star.drawing.RectangleShape" )
        return office::MsoAutoShapeType::msoShapeRectangle;
    else if ( sShapeType == "com.sun.star.drawing.CustomShape" )
    {
        uno::Reference< beans::XPropertySet > aXPropSet( xShape, uno::UNO_QUERY );
        uno::Any aGeoPropSet = aXPropSet->getPropertyValue( u"CustomShapeGeometry"_ustr );
        uno::Sequence< beans::PropertyValue > aGeoPropSeq;
        if ( aGeoPropSet >>= aGeoPropSeq )
        {
            for (const auto& rProp : aGeoPropSeq)
            {
                if( rProp.Name == "Type" )
                {
                    OUString sType;
                    if( rProp.Value >>= sType )
                    {
                        if( sType.endsWith( "ellipse" ) )
                            return office::MsoAutoShapeType::msoShapeOval;
                        // TODO other custom shapes here
                    }
                }
            }
        }
    }

    SAL_WARN( "vbahelper", "ScVbaShape::getAutoShapeType: unknown auto type" );
    return -1; // could not decide

}

// Attributes
OUString SAL_CALL
ScVbaShape::getName()
{
    OUString sName;
    uno::Reference< container::XNamed > xNamed( m_xShape, uno::UNO_QUERY_THROW );
    sName = xNamed->getName();
    return sName;
}

void SAL_CALL
ScVbaShape::setName( const OUString& _name )
{
    uno::Reference< container::XNamed > xNamed( m_xShape, uno::UNO_QUERY_THROW );
    xNamed->setName( _name );
}

OUString SAL_CALL
ScVbaShape::getAlternativeText()
{
    OUString sAltText;
    uno::Reference< beans::XPropertySet > xProps( m_xShape, uno::UNO_QUERY_THROW );
    xProps->getPropertyValue(u"Title"_ustr) >>= sAltText;
    return sAltText;
}

void SAL_CALL
ScVbaShape::setAlternativeText( const OUString& sAltText )
{
    uno::Reference< beans::XPropertySet > xProps( m_xShape, uno::UNO_QUERY_THROW );
    xProps->setPropertyValue(u"Title"_ustr, uno::Any( sAltText ) );
}

double SAL_CALL
ScVbaShape::getHeight()
{
    return m_aShapeHelper.getHeight();
}

void SAL_CALL
ScVbaShape::setHeight(double _height)
{
    m_aShapeHelper.setHeight( _height );
}

double SAL_CALL
ScVbaShape::getWidth()
{
    return m_aShapeHelper.getWidth();
}

void SAL_CALL
ScVbaShape::setWidth(double _width)
{
    m_aShapeHelper.setWidth( _width );
}

double SAL_CALL
ScVbaShape::getLeft()
{
    double left = 0;
    try
    {
        left = m_aShapeHelper.getLeft();
    }
    catch( uno::Exception& )
    {
        // fail to get position by using XShape::getPosition()
        sal_Int32 nLeft = 0;
        m_xPropertySet->getPropertyValue( u"HoriOrientPosition"_ustr ) >>= nLeft;
        left = Millimeter::getInPoints( nLeft );
    }
    return left;
}

void SAL_CALL
ScVbaShape::setLeft( double _left )
{
    try
    {
        m_aShapeHelper.setLeft( _left );
    }
    catch( uno::Exception& )
    {
        sal_Int32 nLeft = Millimeter::getInHundredthsOfOneMillimeter( _left );
        m_xPropertySet->setPropertyValue( u"HoriOrientPosition"_ustr , uno::Any( nLeft ) );
    }
}

double SAL_CALL
ScVbaShape::getTop()
{
    double top = 0;
    try
    {
        top = m_aShapeHelper.getTop();
    }
    catch( uno::Exception& )
    {
        sal_Int32 nTop = 0;
        m_xPropertySet->getPropertyValue( u"VertOrientPosition"_ustr ) >>= nTop;
        top = Millimeter::getInPoints( nTop );
    }
    return top;
}

void SAL_CALL
ScVbaShape::setTop( double _top )
{
    try
    {
        m_aShapeHelper.setTop( _top );
    }
    catch( uno::Exception& )
    {
        sal_Int32 nTop = Millimeter::getInHundredthsOfOneMillimeter( _top );
        m_xPropertySet->setPropertyValue( u"VertOrientPosition"_ustr , uno::Any( nTop ) );
    }
}

sal_Bool SAL_CALL
ScVbaShape::getVisible()
{
    // #STUB
    //UNO Shapes are always visible
    return true;
}

void SAL_CALL
ScVbaShape::setVisible( sal_Bool /*_visible*/ )
{
    // #STUB
    //UNO Shapes are always visible
}

sal_Int32 SAL_CALL
ScVbaShape::getZOrderPosition()
{
    sal_Int32 nZOrderPosition = 0;
    uno::Any aZOrderPosition =  m_xPropertySet->getPropertyValue( u"ZOrder"_ustr );
    aZOrderPosition >>= nZOrderPosition;
    return nZOrderPosition + 1;
}

sal_Int32 SAL_CALL
ScVbaShape::getType()
{
    return m_nType;
}

double SAL_CALL
ScVbaShape::getRotation()
{
    double dRotation = 0;
    sal_Int32 nRotation = 0;
    m_xPropertySet->getPropertyValue( u"RotateAngle"_ustr ) >>= nRotation;
    dRotation = static_cast< double >( nRotation /100 );
    return dRotation;
}

void SAL_CALL
ScVbaShape::setRotation( double _rotation )
{
    sal_Int32 nRotation = static_cast < sal_Int32 > ( _rotation * 100 );
    m_xPropertySet->setPropertyValue( u"RotateAngle"_ustr , uno::Any( nRotation ) );
}

uno::Reference< msforms::XLineFormat > SAL_CALL
ScVbaShape::getLine()
{
    // TODO should only return line
    return uno::Reference< msforms::XLineFormat >( new ScVbaLineFormat( this, mxContext, m_xShape ) );
}

uno::Reference< msforms::XFillFormat > SAL_CALL
ScVbaShape::getFill()
{
    return uno::Reference< msforms::XFillFormat >( new ScVbaFillFormat( this, mxContext, m_xShape ) );
}

uno::Reference<  msforms::XPictureFormat > SAL_CALL
ScVbaShape::getPictureFormat()
{
    return uno::Reference< msforms::XPictureFormat >( new ScVbaPictureFormat( this, mxContext, m_xShape ) );
}

// Methods
uno::Any SAL_CALL
ScVbaShape::TextFrame()
{
    uno::Reference< lang::XServiceInfo > xServiceInfo( m_xModel, uno::UNO_QUERY_THROW );
    if( xServiceInfo->supportsService( u"com.sun.star.sheet.SpreadsheetDocument"_ustr ) )
    {
        uno::Reference< uno::XComponentContext > xContext = comphelper::getProcessComponentContext();
        uno::Reference< uno::XInterface > xTextFrame = xContext->getServiceManager()->createInstanceWithArgumentsAndContext( u"ooo.vba.excel.TextFrame"_ustr , { uno::Any(getParent()), uno::Any(m_xShape) }, xContext );
        return uno::Any( xTextFrame );
    }

    return uno::Any( uno::Reference< msforms::XTextFrame >( new VbaTextFrame( this, mxContext, m_xShape ) ) );
}

void SAL_CALL
ScVbaShape::Delete()
{
    SolarMutexGuard aGuard;
    m_xShapes->remove( m_xShape );
}

void SAL_CALL
ScVbaShape::ZOrder( sal_Int32 ZOrderCmd )
{
    sal_Int32 nOrderPosition;
    uno::Any aOrderPosition = m_xPropertySet->getPropertyValue( u"ZOrder"_ustr );
    aOrderPosition >>= nOrderPosition;
    switch( ZOrderCmd )
    {
    case office::MsoZOrderCmd::msoBringToFront:
        m_xPropertySet->setPropertyValue( u"ZOrder"_ustr , uno::Any( SAL_MAX_INT32 ) );
        break;
    case office::MsoZOrderCmd::msoSendToBack:
        m_xPropertySet->setPropertyValue( u"ZOrder"_ustr , uno::Any( sal_Int32(0) ) );
        break;
    case office::MsoZOrderCmd::msoBringForward:
        nOrderPosition += 1;
        m_xPropertySet->setPropertyValue( u"ZOrder"_ustr , uno::Any( nOrderPosition ) );
        break;
    case office::MsoZOrderCmd::msoSendBackward:
        if( nOrderPosition > 0 )
        {
            nOrderPosition -= 1;
            m_xPropertySet->setPropertyValue( u"ZOrder"_ustr , uno::Any( nOrderPosition ) );
        }
        break;
    // below two commands use with Writer for text and image object.
    case office::MsoZOrderCmd::msoBringInFrontOfText:
    case office::MsoZOrderCmd::msoSendBehindText:
        throw uno::RuntimeException( u"This ZOrderCmd is not implemented, it is use with writer."_ustr );
    default:
        throw uno::RuntimeException( u"Invalid Parameter."_ustr );
    }
}

void SAL_CALL
ScVbaShape::IncrementRotation( double Increment )
{
    double nCurrentRotation = getRotation();
    nCurrentRotation += Increment;
    setRotation(nCurrentRotation);
}

void SAL_CALL
ScVbaShape::IncrementLeft( double Increment )
{
    double nCurrentLeft = getLeft();
    nCurrentLeft += Increment;
    setLeft(nCurrentLeft);
}

void SAL_CALL
ScVbaShape::IncrementTop( double Increment )
{
    double nCurrentTop = getTop();
    nCurrentTop += Increment;
    setTop(nCurrentTop);
}

void SAL_CALL
ScVbaShape::ScaleHeight( double Factor, sal_Bool /*RelativeToOriginalSize*/, sal_Int32 Scale )
{
    double nHeight = getHeight();
    double nNewHeight = nHeight * Factor;
    if( Scale == office::MsoScaleFrom::msoScaleFromTopLeft )
    {
        setHeight(nNewHeight);
    }
    else if( Scale == office::MsoScaleFrom::msoScaleFromBottomRight )
    {
        double nDeltaHeight = nNewHeight - nHeight;
        double nNewTop = getTop() - nDeltaHeight;
        setTop(nNewTop);
        setHeight(nNewHeight);
    }
    else if( Scale == office::MsoScaleFrom::msoScaleFromMiddle )
    {
        double nDeltaHeight = (nNewHeight - nHeight) / 2;
        double nNewTop = getTop() - nDeltaHeight;
        setTop(nNewTop);
        setHeight(nNewHeight);
    }
    else
    {
        throw uno::RuntimeException( u"ScaleHeight.Scale wrong value is given."_ustr );
    }
}

void SAL_CALL
ScVbaShape::ScaleWidth( double Factor, sal_Bool /*RelativeToOriginalSize*/, sal_Int32 Scale )
{
    double nWidth = getWidth();
    double nNewWidth = nWidth * Factor;
    if( Scale == office::MsoScaleFrom::msoScaleFromTopLeft )
    {
        setWidth(nNewWidth);
    }
    else if( Scale == office::MsoScaleFrom::msoScaleFromBottomRight )
    {
        double nDeltaWidth = nNewWidth - nWidth;
        double nNewLeft = getLeft() - nDeltaWidth;
        setLeft(nNewLeft);
        setWidth(nNewWidth);
    }
    else if( Scale == office::MsoScaleFrom::msoScaleFromMiddle )
    {
        double nDeltaWidth = (nNewWidth - nWidth) / 2;
        double nNewLeft = getLeft() - nDeltaWidth;
        setLeft(nNewLeft);
        setWidth(nNewWidth);
    }
    else
    {
        throw uno::RuntimeException( u"ScaleHeight.Scale wrong value is given."_ustr );
    }
}

void SAL_CALL
ScVbaShape::Select( const uno::Any& /*Replace*/ )
{
    uno::Reference< view::XSelectionSupplier > xSelectSupp( m_xModel->getCurrentController(), uno::UNO_QUERY_THROW );
    xSelectSupp->select( uno::Any( m_xShape ) );
}

// This method should not be part of Shape, what we really need to do is...
// dynamically create the appropriate objects e.g. TextBox, Oval, Picture etc.
// ( e.g. the ones that really do have ShapeRange as an attribute )
uno::Any SAL_CALL
ScVbaShape::ShapeRange( const uno::Any& index )
{
    // perhaps we should store a reference to the Shapes Collection
    // in this class
    // but anyway this method should not even be in this class
    // #TODO not sure what the parent of the Shapes collection should be

    XNamedObjectCollectionHelper< drawing::XShape >::XNamedVec aVec;
    aVec.push_back( m_xShape );
    uno::Reference< container::XIndexAccess > xIndexAccess( new XNamedObjectCollectionHelper< drawing::XShape >( std::move(aVec) ) );
    uno::Reference< container::XChild > xChild( m_xShape, uno::UNO_QUERY_THROW );
    // #FIXME for want of a better parent, setting this
    uno::Reference< msforms::XShapeRange > xShapeRange( new ScVbaShapeRange( mxParent, mxContext, xIndexAccess,  uno::Reference< drawing::XDrawPage >( xChild->getParent(), uno::UNO_QUERY_THROW ), m_xModel ) );
    if ( index.hasValue() )
        return xShapeRange->Item( index, uno::Any() );
    return uno::Any( xShapeRange );
}

sal_Bool SAL_CALL
ScVbaShape::getLockAspectRatio()
{
    // #STUB
    return false;
}

void SAL_CALL
ScVbaShape::setLockAspectRatio( sal_Bool /*_lockaspectratio*/ )
{
    // #STUB
}

sal_Bool SAL_CALL
ScVbaShape::getLockAnchor()
{
    // #STUB
    return true;
}

void SAL_CALL
ScVbaShape::setLockAnchor( sal_Bool /*_lockanchor*/ )
{
    // #STUB
}

sal_Int32 SAL_CALL
ScVbaShape::getRelativeHorizontalPosition()
{
    sal_Int32 nRelativeHorizontalPosition = word::WdRelativeHorizontalPosition::wdRelativeHorizontalPositionMargin;
    sal_Int16 nType = text::RelOrientation::PAGE_LEFT;
    m_xPropertySet->getPropertyValue( u"HoriOrientRelation"_ustr ) >>= nType;

    switch( nType )
    {
        case text::RelOrientation::FRAME:
        {
            nRelativeHorizontalPosition = word::WdRelativeHorizontalPosition::wdRelativeHorizontalPositionColumn;
            break;
        }
        case text::RelOrientation::PAGE_FRAME:
        {
            nRelativeHorizontalPosition = word::WdRelativeHorizontalPosition::wdRelativeHorizontalPositionPage;
            break;
        }
        case text::RelOrientation::CHAR:
        {
            nRelativeHorizontalPosition = word::WdRelativeHorizontalPosition::wdRelativeHorizontalPositionCharacter;
            break;
        }
        case text::RelOrientation::PAGE_PRINT_AREA:
        {
            nRelativeHorizontalPosition = word::WdRelativeHorizontalPosition::wdRelativeHorizontalPositionMargin;
            break;
        }
        default:
        {
            throw  uno::RuntimeException( u"Shape::RelativeHorizontalPosition: not implemented"_ustr );
        }
    }
    return nRelativeHorizontalPosition;
}

void SAL_CALL
ScVbaShape::setRelativeHorizontalPosition(::sal_Int32 _relativehorizontalposition)
{
    sal_Int16 nType = text::RelOrientation::PAGE_FRAME;
    switch( _relativehorizontalposition )
    {
        case word::WdRelativeHorizontalPosition::wdRelativeHorizontalPositionCharacter:
        {
            nType = text::RelOrientation::CHAR;
            break;
        }
        case word::WdRelativeHorizontalPosition::wdRelativeHorizontalPositionColumn:
        {
            nType = text::RelOrientation::PAGE_FRAME;
            break;
        }
        case word::WdRelativeHorizontalPosition::wdRelativeHorizontalPositionMargin:
        {
            nType = text::RelOrientation::PAGE_PRINT_AREA;
            break;
        }
        case word::WdRelativeHorizontalPosition::wdRelativeHorizontalPositionPage:
        {
            nType = text::RelOrientation::PAGE_FRAME;
            break;
        }
        default:
        {
            DebugHelper::runtimeexception(ERRCODE_BASIC_BAD_ARGUMENT);
        }
    }
    m_xPropertySet->setPropertyValue( u"HoriOrientRelation"_ustr , uno::Any( nType ) );
}

sal_Int32 SAL_CALL
ScVbaShape::getRelativeVerticalPosition()
{
    sal_Int32 nRelativeVerticalPosition = word::WdRelativeVerticalPosition::wdRelativeVerticalPositionMargin;
    sal_Int16 nType = text::RelOrientation::PAGE_FRAME;
    m_xPropertySet->getPropertyValue( u"VertOrientRelation"_ustr ) >>= nType;

    switch( nType )
    {
        case text::RelOrientation::FRAME:
        {
            nRelativeVerticalPosition = word::WdRelativeVerticalPosition::wdRelativeVerticalPositionParagraph;
            break;
        }
        case text::RelOrientation::PAGE_FRAME:
        {
            nRelativeVerticalPosition = word::WdRelativeVerticalPosition::wdRelativeVerticalPositionPage;
            break;
        }
        case text::RelOrientation::TEXT_LINE:
        {
            nRelativeVerticalPosition = word::WdRelativeVerticalPosition::wdRelativeVerticalPositionLine;
            break;
        }
        case text::RelOrientation::PAGE_PRINT_AREA:
        {
            nRelativeVerticalPosition = word::WdRelativeVerticalPosition::wdRelativeVerticalPositionMargin;
            break;
        }
        default:
        {
            throw  uno::RuntimeException( u"Shape::RelativeVerticalPosition: not implemented"_ustr );
        }
    }
    return nRelativeVerticalPosition;
}

void SAL_CALL
ScVbaShape::setRelativeVerticalPosition(::sal_Int32 _relativeverticalposition)
{
    sal_Int16 nType = text::RelOrientation::PAGE_FRAME;
    switch( _relativeverticalposition )
    {
        case word::WdRelativeVerticalPosition::wdRelativeVerticalPositionLine:
        {
            nType = text::RelOrientation::TEXT_LINE;
            break;
        }
        case word::WdRelativeVerticalPosition::wdRelativeVerticalPositionParagraph:
        {
            nType = text::RelOrientation::FRAME;
            break;
        }
        case word::WdRelativeVerticalPosition::wdRelativeVerticalPositionMargin:
        {
            nType = text::RelOrientation::PAGE_PRINT_AREA;
            break;
        }
        case word::WdRelativeVerticalPosition::wdRelativeVerticalPositionPage:
        {
            nType = text::RelOrientation::PAGE_FRAME;
            break;
        }
        default:
        {
            DebugHelper::runtimeexception(ERRCODE_BASIC_BAD_ARGUMENT);
        }
    }
    m_xPropertySet->setPropertyValue( u"VertOrientRelation"_ustr , uno::Any( nType ) );
}

uno::Any SAL_CALL
ScVbaShape::WrapFormat()
{
    uno::Reference< lang::XServiceInfo > xServiceInfo( m_xModel, uno::UNO_QUERY_THROW );
    if( xServiceInfo->supportsService( u"com.sun.star.text.TextDocument"_ustr ))
    {
        uno::Reference< uno::XComponentContext > xContext = comphelper::getProcessComponentContext();
        uno::Reference< uno::XInterface > xWrapFormat = xContext->getServiceManager()->createInstanceWithArgumentsAndContext( u"ooo.vba.word.WrapFormat"_ustr ,{ uno::Any(getParent()), uno::Any(m_xShape) }, xContext );
        return uno::Any( xWrapFormat );
    }
    throw uno::RuntimeException( u"Not implemented"_ustr );
}

OUString
ScVbaShape::getServiceImplName()
{
    return u"ScVbaShape"_ustr;
}

uno::Sequence< OUString >
ScVbaShape::getServiceNames()
{
    static uno::Sequence< OUString > const aServiceNames
    {
        u"ooo.vba.msform.Shape"_ustr
    };
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
