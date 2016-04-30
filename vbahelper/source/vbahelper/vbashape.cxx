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
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/drawing/ConnectorType.hpp>
#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/drawing/XDrawPages.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <com/sun/star/text/RelOrientation.hpp>
#include <ooo/vba/word/WdRelativeHorizontalPosition.hpp>
#include <ooo/vba/word/WdRelativeVerticalPosition.hpp>

#include <comphelper/processfactory.hxx>
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>
#include <svx/unopage.hxx>
#include <svx/unoshape.hxx>

#include <vbahelper/vbashape.hxx>
#include <vbahelper/vbatextframe.hxx>
#include "vbalineformat.hxx"
#include "vbafillformat.hxx"
#include "vbapictureformat.hxx"
#include <vbahelper/vbashaperange.hxx>

using namespace ::ooo::vba;
using namespace ::com::sun::star;

ScVbaShape::ScVbaShape( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< drawing::XShape >& xShape, const uno::Reference< drawing::XShapes >& xShapes, const uno::Reference< frame::XModel >& xModel, sal_Int32 nType )
    throw (lang::IllegalArgumentException, uno::RuntimeException)
    : ScVbaShape_BASE( xParent, xContext ), m_xShape( xShape ), m_xShapes( xShapes ), m_nType( nType ), m_xModel( xModel )
{
    m_xPropertySet.set( m_xShape, uno::UNO_QUERY_THROW );
    m_pShapeHelper.reset( new ShapeHelper( m_xShape ) );
    addListeners();
}

ScVbaShape::~ScVbaShape()
{
}

void SAL_CALL
ScVbaShape::disposing( const lang::EventObject& rEventObject ) throw( uno::RuntimeException, std::exception )
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
ScVbaShape::removeShapeListener() throw( uno::RuntimeException )
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
ScVbaShape::removeShapesListener() throw( uno::RuntimeException )
{
    if( m_xShapes.is() )
    {
        uno::Reference< lang::XComponent > xComponent( m_xShapes, uno::UNO_QUERY_THROW );
        xComponent->removeEventListener( this );
    }
    m_xShapes = nullptr;
}

sal_Int32
ScVbaShape::getType( const css::uno::Reference< drawing::XShape >& xShape ) throw (uno::RuntimeException)
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
        enum drawing::ConnectorType connectorType;
        uno::Reference< beans::XPropertySet > xPropertySet( xShape, uno::UNO_QUERY_THROW );
        xPropertySet->getPropertyValue( "EdgeKind" ) >>= connectorType;
        if( connectorType == drawing::ConnectorType_CURVE )
            return office::MsoShapeType::msoFreeform;
        else if( connectorType == drawing::ConnectorType_LINE )
            return office::MsoShapeType::msoLine;
        else
            return office::MsoShapeType::msoAutoShape;
    }
    else if( sShapeType == "com.sun.star.drawing.LineShape" )
        return office::MsoShapeType::msoLine;
    else if( sShapeType == "com.sun.star.drawing.CustomShape" || sShapeType == "com.sun.star.drawing.RectangleShape" )
        return office::MsoShapeType::msoAutoShape;
    else if( sShapeType == "com.sun.star.drawing.TextShape" )
        return office::MsoShapeType::msoTextBox;
    else
        throw uno::RuntimeException("the shape type do not be supported: " + sShapeType );
}

// Attributes
OUString SAL_CALL
ScVbaShape::getName() throw (uno::RuntimeException, std::exception)
{
    OUString sName;
    uno::Reference< container::XNamed > xNamed( m_xShape, uno::UNO_QUERY_THROW );
    sName = xNamed->getName();
    return sName;
}

void SAL_CALL
ScVbaShape::setName( const OUString& _name ) throw (uno::RuntimeException, std::exception)
{
    uno::Reference< container::XNamed > xNamed( m_xShape, uno::UNO_QUERY_THROW );
    xNamed->setName( _name );
}

OUString SAL_CALL
ScVbaShape::getAlternativeText() throw (uno::RuntimeException, std::exception)
{
    OUString sAltText;
    uno::Reference< beans::XPropertySet > xProps( m_xShape, uno::UNO_QUERY_THROW );
    xProps->getPropertyValue("Title") >>= sAltText;
    return sAltText;
}

void SAL_CALL
ScVbaShape::setAlternativeText( const OUString& sAltText ) throw (uno::RuntimeException, std::exception)
{
    uno::Reference< beans::XPropertySet > xProps( m_xShape, uno::UNO_QUERY_THROW );
    xProps->setPropertyValue("Title", uno::Any( sAltText ) );
}

double SAL_CALL
ScVbaShape::getHeight() throw (uno::RuntimeException, std::exception)
{
    return m_pShapeHelper->getHeight();
}

void SAL_CALL
ScVbaShape::setHeight(double _height)
    throw (uno::RuntimeException, std::exception)
{
    m_pShapeHelper->setHeight( _height );
}

double SAL_CALL
ScVbaShape::getWidth() throw (uno::RuntimeException, std::exception)
{
    return m_pShapeHelper->getWidth();
}

void SAL_CALL
ScVbaShape::setWidth(double _width)
    throw (uno::RuntimeException, std::exception)
{
    m_pShapeHelper->setWidth( _width );
}

double SAL_CALL
ScVbaShape::getLeft() throw (uno::RuntimeException, std::exception)
{
    double left = 0;
    try
    {
        left = m_pShapeHelper->getLeft();
    }
    catch( uno::Exception& )
    {
        // fail to get position by using XShape::getPosition()
        sal_Int32 nLeft = 0;
        m_xPropertySet->getPropertyValue( "HoriOrientPosition" ) >>= nLeft;
        left = Millimeter::getInPoints( nLeft );
    }
    return left;
}

void SAL_CALL
ScVbaShape::setLeft( double _left ) throw (uno::RuntimeException, std::exception)
{
    try
    {
        m_pShapeHelper->setLeft( _left );
    }
    catch( uno::Exception& )
    {
        sal_Int32 nLeft = 0;
        nLeft = Millimeter::getInHundredthsOfOneMillimeter( _left );
        m_xPropertySet->setPropertyValue( "HoriOrientPosition" , uno::makeAny( nLeft ) );
    }
}

double SAL_CALL
ScVbaShape::getTop() throw (uno::RuntimeException, std::exception)
{
    double top = 0;
    try
    {
        top = m_pShapeHelper->getTop();
    }
    catch( uno::Exception& )
    {
        sal_Int32 nTop = 0;
        m_xPropertySet->getPropertyValue( "VertOrientPosition" ) >>= nTop;
        top = Millimeter::getInPoints( nTop );
    }
    return top;
}

void SAL_CALL
ScVbaShape::setTop( double _top ) throw (uno::RuntimeException, std::exception)
{
    try
    {
        m_pShapeHelper->setTop( _top );
    }
    catch( uno::Exception& )
    {
        sal_Int32 nTop = 0;
        nTop = Millimeter::getInHundredthsOfOneMillimeter( _top );
        m_xPropertySet->setPropertyValue( "VertOrientPosition" , uno::makeAny( nTop ) );
    }
}

sal_Bool SAL_CALL
ScVbaShape::getVisible() throw (uno::RuntimeException, std::exception)
{
    // #STUB
    //UNO Shapes are always visible
    return true;
}

void SAL_CALL
ScVbaShape::setVisible( sal_Bool /*_visible*/ ) throw (uno::RuntimeException, std::exception)
{
    // #STUB
    //UNO Shapes are always visible
}

sal_Int32 SAL_CALL
ScVbaShape::getZOrderPosition() throw (uno::RuntimeException, std::exception)
{
    sal_Int32 nZOrderPosition = 0;
    uno::Any aZOrderPosition =  m_xPropertySet->getPropertyValue( "ZOrder" );
    aZOrderPosition >>= nZOrderPosition;
    return nZOrderPosition + 1;
}

sal_Int32 SAL_CALL
ScVbaShape::getType() throw (uno::RuntimeException, std::exception)
{
    return m_nType;
}

double SAL_CALL
ScVbaShape::getRotation() throw (uno::RuntimeException, std::exception)
{
    double dRotation = 0;
    sal_Int32 nRotation = 0;
    m_xPropertySet->getPropertyValue( "RotateAngle" ) >>= nRotation;
    dRotation = static_cast< double >( nRotation /100 );
    return dRotation;
}

void SAL_CALL
ScVbaShape::setRotation( double _rotation ) throw (uno::RuntimeException, std::exception)
{
    sal_Int32 nRotation = static_cast < sal_Int32 > ( _rotation * 100 );
    m_xPropertySet->setPropertyValue( "RotateAngle" , uno::makeAny( nRotation ) );
}

uno::Reference< msforms::XLineFormat > SAL_CALL
ScVbaShape::getLine() throw (uno::RuntimeException, std::exception)
{
    // TODO should only return line
    return uno::Reference< msforms::XLineFormat >( new ScVbaLineFormat( this, mxContext, m_xShape ) );
}

uno::Reference< msforms::XFillFormat > SAL_CALL
ScVbaShape::getFill() throw (uno::RuntimeException, std::exception)
{
    return uno::Reference< msforms::XFillFormat >( new ScVbaFillFormat( this, mxContext, m_xShape ) );
}

uno::Reference<  msforms::XPictureFormat > SAL_CALL
ScVbaShape::getPictureFormat() throw (uno::RuntimeException, std::exception)
{
    return uno::Reference< msforms::XPictureFormat >( new ScVbaPictureFormat( this, mxContext, m_xShape ) );
}

// Methods
uno::Any SAL_CALL
ScVbaShape::TextFrame() throw (uno::RuntimeException, std::exception)
{
    uno::Reference< lang::XServiceInfo > xServiceInfo( m_xModel, uno::UNO_QUERY_THROW );
    if( xServiceInfo->supportsService( "com.sun.star.sheet.SpreadsheetDocument" ) )
    {
        uno::Reference< uno::XComponentContext > xContext = comphelper::getProcessComponentContext();
        uno::Sequence< uno::Any > aArgs(2);
        aArgs[0] = uno::makeAny( getParent() );
        aArgs[1] <<= m_xShape;
        uno::Reference< uno::XInterface > xTextFrame = xContext->getServiceManager()->createInstanceWithArgumentsAndContext( "ooo.vba.excel.TextFrame" , aArgs, xContext );
        return uno::makeAny( xTextFrame );
    }

    return uno::makeAny( uno::Reference< msforms::XTextFrame >( new VbaTextFrame( this, mxContext, m_xShape ) ) );
}

void SAL_CALL
ScVbaShape::Delete() throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    m_xShapes->remove( m_xShape );
}

void SAL_CALL
ScVbaShape::ZOrder( sal_Int32 ZOrderCmd ) throw (uno::RuntimeException, std::exception)
{
    sal_Int32 nOrderPosition;
    uno::Any aOrderPostion = m_xPropertySet->getPropertyValue( "ZOrder" );
    aOrderPostion >>= nOrderPosition;
    switch( ZOrderCmd )
    {
    case office::MsoZOrderCmd::msoBringToFront:
        m_xPropertySet->setPropertyValue( "ZOrder" , uno::makeAny( SAL_MAX_INT32 ) );
        break;
    case office::MsoZOrderCmd::msoSendToBack:
        m_xPropertySet->setPropertyValue( "ZOrder" , uno::makeAny( (sal_Int32)0 ) );
        break;
    case office::MsoZOrderCmd::msoBringForward:
        nOrderPosition += 1;
        m_xPropertySet->setPropertyValue( "ZOrder" , uno::makeAny( nOrderPosition ) );
        break;
    case office::MsoZOrderCmd::msoSendBackward:
        if( nOrderPosition > 0 )
        {
            nOrderPosition -= 1;
            m_xPropertySet->setPropertyValue( "ZOrder" , uno::makeAny( nOrderPosition ) );
        }
        break;
    // below two commands use with Writer for text and image object.
    case office::MsoZOrderCmd::msoBringInFrontOfText:
    case office::MsoZOrderCmd::msoSendBehindText:
        throw uno::RuntimeException( "This ZOrderCmd is not implemented, it is use with writer." );
    default:
        throw uno::RuntimeException( "Invalid Parameter." );
    }
}

void SAL_CALL
ScVbaShape::IncrementRotation( double Increment ) throw (uno::RuntimeException, std::exception)
{
    double nCurrentRotation = getRotation();
    nCurrentRotation += Increment;
    setRotation(nCurrentRotation);
}

void SAL_CALL
ScVbaShape::IncrementLeft( double Increment ) throw (uno::RuntimeException, std::exception)
{
    double nCurrentLeft = getLeft();
    nCurrentLeft += Increment;
    setLeft(nCurrentLeft);
}

void SAL_CALL
ScVbaShape::IncrementTop( double Increment ) throw (uno::RuntimeException, std::exception)
{
    double nCurrentTop = getTop();
    nCurrentTop += Increment;
    setTop(nCurrentTop);
}

void SAL_CALL
ScVbaShape::ScaleHeight( double Factor, sal_Bool /*RelativeToOriginalSize*/, sal_Int32 Scale ) throw (uno::RuntimeException, std::exception)
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
        throw uno::RuntimeException( "ScaleHeight.Scale wrong value is given." );
    }
}

void SAL_CALL
ScVbaShape::ScaleWidth( double Factor, sal_Bool /*RelativeToOriginalSize*/, sal_Int32 Scale ) throw (uno::RuntimeException, std::exception)
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
        throw uno::RuntimeException( "ScaleHeight.Scale wrong value is given." );
    }
}

void SAL_CALL
ScVbaShape::Select( const uno::Any& /*Replace*/ ) throw ( uno::RuntimeException, std::exception )
{
    uno::Reference< view::XSelectionSupplier > xSelectSupp( m_xModel->getCurrentController(), uno::UNO_QUERY_THROW );
    xSelectSupp->select( uno::makeAny( m_xShape ) );
}

// This method should not be part of Shape, what we reall need to do is...
// dynamically create the appropriate objects e.g. TextBox, Oval, Picture etc.
// ( e.g. the ones that really do have ShapeRange as an attribute )
uno::Any SAL_CALL
ScVbaShape::ShapeRange( const uno::Any& index ) throw ( uno::RuntimeException, std::exception )
{
    // perhaps we should store a reference to the Shapes Collection
    // in this class
    // but anyway this method should not even be in this class
    // #TODO not sure what the parent of the Shapes collection should be

    XNamedObjectCollectionHelper< drawing::XShape >::XNamedVec aVec;
    aVec.push_back( m_xShape );
    uno::Reference< container::XIndexAccess > xIndexAccess( new XNamedObjectCollectionHelper< drawing::XShape >( aVec ) );
    uno::Reference< container::XChild > xChild( m_xShape, uno::UNO_QUERY_THROW );
    // #FIXME for want of a better parent, setting this
    uno::Reference< msforms::XShapeRange > xShapeRange( new ScVbaShapeRange( mxParent, mxContext, xIndexAccess,  uno::Reference< drawing::XDrawPage >( xChild->getParent(), uno::UNO_QUERY_THROW ), m_xModel ) );
    if ( index.hasValue() )
        return xShapeRange->Item( index, uno::Any() );
    return uno::makeAny( xShapeRange );
}

sal_Bool SAL_CALL
ScVbaShape::getLockAspectRatio() throw (uno::RuntimeException, std::exception)
{
    // #STUB
    return false;
}

void SAL_CALL
ScVbaShape::setLockAspectRatio( sal_Bool /*_lockaspectratio*/ ) throw (uno::RuntimeException, std::exception)
{
    // #STUB
}

sal_Bool SAL_CALL
ScVbaShape::getLockAnchor() throw (uno::RuntimeException, std::exception)
{
    // #STUB
    return true;
}

void SAL_CALL
ScVbaShape::setLockAnchor( sal_Bool /*_lockanchor*/ ) throw (uno::RuntimeException, std::exception)
{
    // #STUB
}

sal_Int32 SAL_CALL
ScVbaShape::getRelativeHorizontalPosition() throw (uno::RuntimeException, std::exception)
{
    sal_Int32 nRelativeHorizontalPosition = word::WdRelativeHorizontalPosition::wdRelativeHorizontalPositionMargin;
    sal_Int16 nType = text::RelOrientation::PAGE_LEFT;
    m_xPropertySet->getPropertyValue( "HoriOrientRelation" ) >>= nType;

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
            throw  uno::RuntimeException( "Shape::RelativeHorizontalPosition: not implemented" );
        }
    }
    return nRelativeHorizontalPosition;
}

void SAL_CALL
ScVbaShape::setRelativeHorizontalPosition(::sal_Int32 _relativehorizontalposition)
    throw (uno::RuntimeException, std::exception)
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
            DebugHelper::runtimeexception(ERRCODE_BASIC_BAD_ARGUMENT, OUString());
        }
    }
    m_xPropertySet->setPropertyValue( "HoriOrientRelation" , uno::makeAny( nType ) );
}

sal_Int32 SAL_CALL
ScVbaShape::getRelativeVerticalPosition() throw (uno::RuntimeException, std::exception)
{
    sal_Int32 nRelativeVerticalPosition = word::WdRelativeVerticalPosition::wdRelativeVerticalPositionMargin;
    sal_Int16 nType = text::RelOrientation::PAGE_FRAME;
    m_xPropertySet->getPropertyValue( "VertOrientRelation" ) >>= nType;

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
            throw  uno::RuntimeException( "Shape::RelativeVerticalPosition: not implemented" );
        }
    }
    return nRelativeVerticalPosition;
}

void SAL_CALL
ScVbaShape::setRelativeVerticalPosition(::sal_Int32 _relativeverticalposition)
    throw (uno::RuntimeException, std::exception)
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
            DebugHelper::runtimeexception(ERRCODE_BASIC_BAD_ARGUMENT, OUString());
        }
    }
    m_xPropertySet->setPropertyValue( "VertOrientRelation" , uno::makeAny( nType ) );
}

uno::Any SAL_CALL
ScVbaShape::WrapFormat() throw (uno::RuntimeException, std::exception)
{
    uno::Reference< lang::XServiceInfo > xServiceInfo( m_xModel, uno::UNO_QUERY_THROW );
    if( xServiceInfo->supportsService( "com.sun.star.text.TextDocument" ))
    {
        uno::Reference< uno::XComponentContext > xContext = comphelper::getProcessComponentContext();
        uno::Sequence< uno::Any > aArgs(2);
        aArgs[0] = uno::makeAny( getParent() );
        aArgs[1] <<= m_xShape;
        uno::Reference< uno::XInterface > xWrapFormat = xContext->getServiceManager()->createInstanceWithArgumentsAndContext( "ooo.vba.word.WrapFormat" , aArgs, xContext );
        return uno::makeAny( xWrapFormat );
    }
    throw uno::RuntimeException( "Not implemented" );
}

OUString
ScVbaShape::getServiceImplName()
{
    return OUString("ScVbaShape");
}

uno::Sequence< OUString >
ScVbaShape::getServiceNames()
{
    static uno::Sequence< OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = "ooo.vba.msform.Shape";
    }
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
