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
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <com/sun/star/text/WrapTextMode.hpp>
#include <ooo/vba/msforms/XShapeRange.hpp>
#include <ooo/vba/office/MsoAutoShapeType.hpp>
#include <ooo/vba/office/MsoTextOrientation.hpp>
#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/script/XTypeConverter.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XTextContent.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/text/VertOrientation.hpp>
#include <com/sun/star/text/RelOrientation.hpp>
#include <com/sun/star/text/SizeType.hpp>
#include <com/sun/star/text/WritingMode.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/drawing/XShapes.hpp>

#include <vbahelper/vbahelper.hxx>
#include <vbahelper/vbashape.hxx>
#include <vbahelper/vbashapes.hxx>
#include <vbahelper/vbashaperange.hxx>

using namespace ::ooo::vba;
using namespace ::com::sun::star;

class VbShapeEnumHelper : public EnumerationHelper_BASE
{
        uno::Reference<msforms::XShapes > m_xParent;
        uno::Reference<container::XIndexAccess > m_xIndexAccess;
        sal_Int32 nIndex;
public:
    VbShapeEnumHelper( const uno::Reference< msforms::XShapes >& xParent,  const uno::Reference< container::XIndexAccess >& xIndexAccess ) : m_xParent( xParent ), m_xIndexAccess( xIndexAccess ), nIndex( 0 ) {}
        virtual sal_Bool SAL_CALL hasMoreElements(  ) override
        {
                return ( nIndex < m_xIndexAccess->getCount() );
        }
        virtual uno::Any SAL_CALL nextElement(  ) override
        {
                ScVbaShapes* pShapes = dynamic_cast< ScVbaShapes* >(m_xParent.get());
                if ( pShapes && hasMoreElements() )
                    return pShapes->createCollectionObject(  m_xIndexAccess->getByIndex( nIndex++ ) );
                throw container::NoSuchElementException();
        }

};

void ScVbaShapes::initBaseCollection()
{
    if ( m_xNameAccess.is() ) // already has NameAccess
        return;
    // no NameAccess then use ShapeCollectionHelper
    XNamedObjectCollectionHelper< drawing::XShape >::XNamedVec aShapes;
    sal_Int32 nLen = m_xIndexAccess->getCount();
    aShapes.reserve( nLen );
    for ( sal_Int32 index=0; index<nLen; ++index )
        aShapes.emplace_back( m_xIndexAccess->getByIndex( index ) , uno::UNO_QUERY );
    uno::Reference< container::XIndexAccess > xShapes( new XNamedObjectCollectionHelper< drawing::XShape >( aShapes ) );
    m_xIndexAccess.set( xShapes, uno::UNO_QUERY );
    m_xNameAccess.set( xShapes, uno::UNO_QUERY );
}

ScVbaShapes::ScVbaShapes( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext >& xContext, const css::uno::Reference< css::container::XIndexAccess >& xShapes, const uno::Reference< frame::XModel>& xModel ): ScVbaShapes_BASE( xParent, xContext, xShapes, true ), m_nNewShapeCount(0), m_xModel( xModel )
{
    m_xShapes.set( xShapes, uno::UNO_QUERY_THROW );
    m_xDrawPage.set( xShapes, uno::UNO_QUERY_THROW );
    initBaseCollection();
}

uno::Reference< container::XEnumeration >
ScVbaShapes::createEnumeration()
{
    return new VbShapeEnumHelper( this,  m_xIndexAccess );
}

uno::Any
ScVbaShapes::createCollectionObject( const css::uno::Any& aSource )
{
    if( aSource.hasValue() )
    {
        uno::Reference< drawing::XShape > xShape( aSource, uno::UNO_QUERY_THROW );
        return uno::makeAny( uno::Reference< msforms::XShape >( new ScVbaShape( getParent(), mxContext, xShape, m_xShapes, m_xModel, ScVbaShape::getType( xShape ) ) ) );
    }
    return uno::Any();
}

uno::Type
ScVbaShapes::getElementType()
{
    return cppu::UnoType<ooo::vba::msforms::XShape>::get();
}

OUString
ScVbaShapes::getServiceImplName()
{
    return OUString("ScVbaShapes");
}

uno::Sequence< OUString >
ScVbaShapes::getServiceNames()
{
    static uno::Sequence< OUString > const aServiceNames
    {
        "ooo.vba.msform.Shapes"
    };
    return aServiceNames;
}

css::uno::Reference< css::container::XIndexAccess >
ScVbaShapes::getShapesByArrayIndices( const uno::Any& Index  )
{
    if ( Index.getValueTypeClass() != uno::TypeClass_SEQUENCE )
        throw uno::RuntimeException();

    const uno::Reference< script::XTypeConverter >& xConverter = getTypeConverter(mxContext);
    uno::Any aConverted;
    aConverted = xConverter->convertTo( Index, cppu::UnoType<uno::Sequence< uno::Any >>::get() );

    uno::Sequence< uno::Any > sIndices;
    aConverted >>= sIndices;
    XNamedObjectCollectionHelper< drawing::XShape >::XNamedVec aShapes;
    sal_Int32 nElems = sIndices.getLength();
    for( sal_Int32 index = 0; index < nElems; ++index )
    {
        uno::Reference< drawing::XShape > xShape;
        if ( sIndices[ index ].getValueTypeClass() == uno::TypeClass_STRING )
        {
            OUString sName;
            sIndices[ index ] >>= sName;
            xShape.set( m_xNameAccess->getByName( sName ), uno::UNO_QUERY );
        }
        else
        {
            sal_Int32 nIndex = 0;
            sIndices[ index ] >>= nIndex;
            // adjust for 1 based mso indexing
            xShape.set( m_xIndexAccess->getByIndex( nIndex - 1 ), uno::UNO_QUERY );

        }
        // populate map with drawing::XShapes
        if ( xShape.is() )
            aShapes.push_back( xShape );
    }
    uno::Reference< container::XIndexAccess > xIndexAccess( new XNamedObjectCollectionHelper< drawing::XShape >( aShapes ) );
    return xIndexAccess;
}

uno::Reference< msforms::XShapeRange > SAL_CALL
ScVbaShapes::Range( const uno::Any& shapes )
{
    // shapes, can be an index or an array of indices
    uno::Reference< container::XIndexAccess > xShapes;
    if ( shapes.getValueTypeClass() == uno::TypeClass_SEQUENCE )
        xShapes = getShapesByArrayIndices( shapes );
    else
    {
        // wrap single index into a sequence
        uno::Sequence< uno::Any > sIndices(1);
        sIndices[ 0 ] = shapes;
        uno::Any aIndex;
        aIndex <<= sIndices;
        xShapes = getShapesByArrayIndices( aIndex );
    }
    return new ScVbaShapeRange(  getParent(), mxContext, xShapes, m_xDrawPage, m_xModel );
}

void SAL_CALL
ScVbaShapes::SelectAll()
{
    uno::Reference< view::XSelectionSupplier > xSelectSupp( m_xModel->getCurrentController(), uno::UNO_QUERY_THROW );
    try
    {
        xSelectSupp->select( uno::makeAny( m_xShapes ) );
    }
    // viewuno.cxx ScTabViewObj::select will throw IllegalArgumentException
    // if one of the shapes is no 'markable' e.g. a button
    // the method still works
    catch(const lang::IllegalArgumentException&)
    {
    }
}

uno::Reference< drawing::XShape >
ScVbaShapes::createShape( const OUString& service )
{
    uno::Reference< lang::XMultiServiceFactory > xMSF( m_xModel, uno::UNO_QUERY_THROW );
    uno::Reference< drawing::XShape > xShape( xMSF->createInstance( service ), uno::UNO_QUERY_THROW );
    return xShape;
}

uno::Any
ScVbaShapes::AddRectangle(sal_Int32 startX, sal_Int32 startY, sal_Int32 nLineWidth, sal_Int32 nLineHeight)
{
    sal_Int32 nXPos = Millimeter::getInHundredthsOfOneMillimeter( startX );
    sal_Int32 nYPos = Millimeter::getInHundredthsOfOneMillimeter( startY );
    sal_Int32 nWidth = Millimeter::getInHundredthsOfOneMillimeter( nLineWidth );
    sal_Int32 nHeight = Millimeter::getInHundredthsOfOneMillimeter( nLineHeight );

    uno::Reference< drawing::XShape > xShape( createShape( "com.sun.star.drawing.RectangleShape" ), uno::UNO_QUERY_THROW );
    m_xShapes->add( xShape );

    OUString sName(createName( "Rectangle" ));
    setDefaultShapeProperties( xShape );
    setShape_NameProperty( xShape, sName );

    awt::Point aMovePositionIfRange(0, 0);
    awt::Point position;
    position.X = nXPos - aMovePositionIfRange.X;
    position.Y = nYPos - aMovePositionIfRange.Y;
    xShape->setPosition( position );

    awt::Size size;
    size.Height = nHeight;
    size.Width = nWidth;
    xShape->setSize( size );

    ScVbaShape *pScVbaShape = new ScVbaShape( getParent(), mxContext, xShape, m_xShapes, m_xModel, ScVbaShape::getType( xShape ) );
    return uno::makeAny( uno::Reference< msforms::XShape > ( pScVbaShape ) );
}

uno::Any
ScVbaShapes::AddEllipse(sal_Int32 startX, sal_Int32 startY, sal_Int32 nLineWidth, sal_Int32 nLineHeight)
{
    sal_Int32 nXPos = Millimeter::getInHundredthsOfOneMillimeter( startX );
    sal_Int32 nYPos = Millimeter::getInHundredthsOfOneMillimeter( startY );
    sal_Int32 nWidth = Millimeter::getInHundredthsOfOneMillimeter( nLineWidth );
    sal_Int32 nHeight = Millimeter::getInHundredthsOfOneMillimeter( nLineHeight );

    uno::Reference< drawing::XShape > xShape( createShape( "com.sun.star.drawing.EllipseShape" ), uno::UNO_QUERY_THROW );
    m_xShapes->add( xShape );

    awt::Point aMovePositionIfRange( 0, 0 );
    //TODO helperapi using a writer document
    /*
    XDocument xDocument = (XDocument)getParent();
    if (AnyConverter.isVoid(_aRange))
    {
        _aRange = xDocument.Range(new Integer(0), new Integer(1));
        // Top&Left in Word is Top&Left of the paper and not the writeable area.
        aMovePositionIfRange = calculateTopLeftMargin((HelperInterfaceAdaptor)xDocument);
    }

    setShape_AnchorTypeAndRangeProperty(xShape, _aRange);
    */
    OUString name(createName( "Oval" ));
    setDefaultShapeProperties(xShape);
    setShape_NameProperty(xShape, name);

    awt::Point position;
    position.X = nXPos - aMovePositionIfRange.X;
    position.Y = nYPos - aMovePositionIfRange.Y;
    xShape->setPosition(position);

    awt::Size size;
    size.Height = nHeight;
    size.Width = nWidth;
    xShape->setSize(size);

    ScVbaShape *pScVbaShape = new ScVbaShape( getParent(), mxContext, xShape, m_xShapes, m_xModel, ScVbaShape::getType( xShape ) );
    return uno::makeAny( uno::Reference< msforms::XShape > ( pScVbaShape ) );
}

//helperapi calc
uno::Any SAL_CALL
ScVbaShapes::AddLine( sal_Int32 StartX, sal_Int32 StartY, sal_Int32 endX, sal_Int32 endY )
{
    sal_Int32 nLineWidth = endX - StartX;
    sal_Int32 nLineHeight = endY - StartY;

    sal_Int32 nHeight = Millimeter::getInHundredthsOfOneMillimeter( nLineHeight );
    sal_Int32 nWidth = Millimeter::getInHundredthsOfOneMillimeter( nLineWidth );
    sal_Int32 nXPos = Millimeter::getInHundredthsOfOneMillimeter( StartX );
    sal_Int32 nYPos = Millimeter::getInHundredthsOfOneMillimeter( StartY );

    uno::Reference< drawing::XShape > xShape( createShape( "com.sun.star.drawing.LineShape" ), uno::UNO_QUERY_THROW );
    m_xShapes->add( xShape );

    awt::Point aMovePositionIfRange( 0, 0 );

    OUString name(createName( "Line" ));
    setDefaultShapeProperties(xShape);
    setShape_NameProperty(xShape, name);

    awt::Point position;
    position.X = nXPos - aMovePositionIfRange.X;
    position.Y = nYPos - aMovePositionIfRange.Y;
    xShape->setPosition(position);

    awt::Size size;
    size.Height = nHeight;
    size.Width = nWidth;
    xShape->setSize(size);

    ScVbaShape *pScVbaShape = new ScVbaShape( getParent(), mxContext, xShape, m_xShapes, m_xModel, ScVbaShape::getType( xShape ) );
    return uno::makeAny( uno::Reference< msforms::XShape > ( pScVbaShape ) );
}

uno::Any SAL_CALL
ScVbaShapes::AddShape( sal_Int32 _nType, sal_Int32 _nLeft, sal_Int32 _nTop, sal_Int32 _nWidth, sal_Int32 _nHeight )
{
    if (_nType == office::MsoAutoShapeType::msoShapeRectangle)
    {
        return AddRectangle(_nLeft, _nTop, _nWidth, _nHeight);
    }
    else if (_nType == office::MsoAutoShapeType::msoShapeOval)
    {
        return AddEllipse(_nLeft, _nTop, _nWidth, _nHeight);
    }
    return uno::Any();
}

uno::Any SAL_CALL
ScVbaShapes::AddTextbox( sal_Int32 /*_nOrientation*/, sal_Int32 _nLeft, sal_Int32 _nTop, sal_Int32 _nWidth, sal_Int32 _nHeight )
{
    uno::Reference< lang::XServiceInfo > xServiceInfo( m_xModel, uno::UNO_QUERY_THROW );
    if( xServiceInfo->supportsService( "com.sun.star.text.TextDocument" ) )
    {
        return AddTextboxInWriter( _nLeft, _nTop, _nWidth, _nHeight );
    }
    throw uno::RuntimeException( "Not implemented" );
}

uno::Any
ScVbaShapes::AddTextboxInWriter( sal_Int32 _nLeft, sal_Int32 _nTop, sal_Int32 _nWidth, sal_Int32 _nHeight )
{
    sal_Int32 nXPos = Millimeter::getInHundredthsOfOneMillimeter( _nLeft );
    sal_Int32 nYPos = Millimeter::getInHundredthsOfOneMillimeter( _nTop );
    sal_Int32 nWidth = Millimeter::getInHundredthsOfOneMillimeter( _nWidth );
    sal_Int32 nHeight = Millimeter::getInHundredthsOfOneMillimeter( _nHeight );

    uno::Reference< drawing::XShape > xShape( createShape( "com.sun.star.drawing.TextShape" ), uno::UNO_QUERY_THROW );
    m_xShapes->add( xShape );

    setDefaultShapeProperties(xShape);

    OUString sName(createName( "Text Box" ));
    setShape_NameProperty( xShape, sName );

    awt::Size size;
    size.Height = nHeight;
    size.Width = nWidth;
    xShape->setSize(size);

    uno::Reference< beans::XPropertySet > xShapeProps( xShape, uno::UNO_QUERY_THROW );
    xShapeProps->setPropertyValue( "AnchorType", uno::makeAny( text::TextContentAnchorType_AT_PAGE ) );
    xShapeProps->setPropertyValue( "HoriOrientRelation", uno::makeAny( text::RelOrientation::PAGE_LEFT ) );
    xShapeProps->setPropertyValue( "HoriOrient", uno::makeAny( text::HoriOrientation::NONE ) );
    xShapeProps->setPropertyValue( "HoriOrientPosition", uno::makeAny( nXPos ) );

    xShapeProps->setPropertyValue( "VertOrientRelation", uno::makeAny( text::RelOrientation::PAGE_FRAME ) );
    xShapeProps->setPropertyValue( "VertOrient", uno::makeAny( text::VertOrientation::NONE ) );
    xShapeProps->setPropertyValue( "VertOrientPosition", uno::makeAny( nYPos ) );

    // set to visible
    xShapeProps->setPropertyValue( "LineStyle", uno::makeAny( drawing::LineStyle_SOLID ) );
    // set to font
    xShapeProps->setPropertyValue( "LayerID", uno::makeAny( sal_Int16(1) ) );
    xShapeProps->setPropertyValue( "LayerName", uno::makeAny( OUString("Heaven") ) );


    ScVbaShape *pScVbaShape = new ScVbaShape( getParent(), mxContext, xShape, m_xShapes, m_xModel, ScVbaShape::getType( xShape ) );
    return uno::makeAny( uno::Reference< msforms::XShape > ( pScVbaShape ) );
}

void
ScVbaShapes::setDefaultShapeProperties( const uno::Reference< drawing::XShape >& xShape )
{
    uno::Reference< beans::XPropertySet > xPropertySet( xShape, uno::UNO_QUERY_THROW );
    xPropertySet->setPropertyValue( "FillStyle", uno::makeAny( OUString("SOLID") ) );
    xPropertySet->setPropertyValue( "FillColor", uno::makeAny( sal_Int32(0xFFFFFF) )  );
    xPropertySet->setPropertyValue( "TextWordWrap", uno::makeAny( text::WrapTextMode_THROUGH )  );
    //not find in OOo2.3
    //xPropertySet->setPropertyValue("Opaque", uno::makeAny( sal_True )  );
}

void
ScVbaShapes::setShape_NameProperty( const uno::Reference< css::drawing::XShape >& xShape, const OUString& sName )
{
    uno::Reference< beans::XPropertySet > xPropertySet( xShape, uno::UNO_QUERY_THROW );
    try
    {
        xPropertySet->setPropertyValue( "Name", uno::makeAny( sName ) );
    }
    catch(const script::BasicErrorException&)
    {
    }
}

OUString
ScVbaShapes::createName( const OUString& sName )
{
    sal_Int32 nActNumber = 1 + m_nNewShapeCount;
    m_nNewShapeCount++;
    return sName + OUString::number( nActNumber );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
