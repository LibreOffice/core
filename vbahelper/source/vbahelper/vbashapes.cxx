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

#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <com/sun/star/text/WrapTextMode.hpp>
#include <ooo/vba/msforms/XShapeRange.hpp>
#include <ooo/vba/office/MsoAutoShapeType.hpp>
#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/script/BasicErrorException.hpp>
#include <com/sun/star/script/XTypeConverter.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/text/VertOrientation.hpp>
#include <com/sun/star/text/RelOrientation.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/drawing/XShapes.hpp>

#include <utility>
#include <vbahelper/vbahelper.hxx>
#include <vbahelper/vbashape.hxx>
#include <vbahelper/vbashapes.hxx>
#include <vbahelper/vbashaperange.hxx>
#include <rtl/ref.hxx>

using namespace ::ooo::vba;
using namespace ::com::sun::star;

namespace {

class VbShapeEnumHelper : public EnumerationHelper_BASE
{
        rtl::Reference<ScVbaShapes> m_xParent;
        uno::Reference<container::XIndexAccess > m_xIndexAccess;
        sal_Int32 nIndex;
public:
    VbShapeEnumHelper( rtl::Reference< ScVbaShapes > xParent, uno::Reference< container::XIndexAccess > xIndexAccess ) : m_xParent(std::move( xParent )), m_xIndexAccess(std::move( xIndexAccess )), nIndex( 0 ) {}
        virtual sal_Bool SAL_CALL hasMoreElements(  ) override
        {
                return ( nIndex < m_xIndexAccess->getCount() );
        }
        virtual uno::Any SAL_CALL nextElement(  ) override
        {
                ScVbaShapes* pShapes = m_xParent.get();
                if ( pShapes && hasMoreElements() )
                    return pShapes->createCollectionObject(  m_xIndexAccess->getByIndex( nIndex++ ) );
                throw container::NoSuchElementException();
        }

};

}

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
    uno::Reference< container::XIndexAccess > xShapes( new XNamedObjectCollectionHelper< drawing::XShape >( std::move(aShapes) ) );
    m_xIndexAccess = xShapes;
    m_xNameAccess.set( xShapes, uno::UNO_QUERY );
}

ScVbaShapes::ScVbaShapes( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext >& xContext, const css::uno::Reference< css::container::XIndexAccess >& xShapes, uno::Reference< frame::XModel> xModel ): ScVbaShapes_BASE( xParent, xContext, xShapes, true ), m_nNewShapeCount(0), m_xModel(std::move( xModel ))
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
        return uno::Any( uno::Reference< msforms::XShape >( new ScVbaShape( getParent(), mxContext, xShape, m_xShapes, m_xModel, ScVbaShape::getType( xShape ) ) ) );
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
    return u"ScVbaShapes"_ustr;
}

uno::Sequence< OUString >
ScVbaShapes::getServiceNames()
{
    static uno::Sequence< OUString > const aServiceNames
    {
        u"ooo.vba.msform.Shapes"_ustr
    };
    return aServiceNames;
}

css::uno::Reference< css::container::XIndexAccess >
ScVbaShapes::getShapesByArrayIndices( const uno::Any& Index  )
{
    if ( Index.getValueTypeClass() != uno::TypeClass_SEQUENCE )
        throw uno::RuntimeException();

    const uno::Reference< script::XTypeConverter >& xConverter = getTypeConverter(mxContext);
    uno::Any aConverted = xConverter->convertTo( Index, cppu::UnoType<uno::Sequence< uno::Any >>::get() );

    uno::Sequence< uno::Any > sIndices;
    aConverted >>= sIndices;
    XNamedObjectCollectionHelper< drawing::XShape >::XNamedVec aShapes;
    for (const auto& rIndex : sIndices)
    {
        uno::Reference< drawing::XShape > xShape;
        if ( rIndex.getValueTypeClass() == uno::TypeClass_STRING )
        {
            OUString sName;
            rIndex >>= sName;
            xShape.set( m_xNameAccess->getByName( sName ), uno::UNO_QUERY );
        }
        else
        {
            sal_Int32 nIndex = 0;
            rIndex >>= nIndex;
            // adjust for 1 based mso indexing
            xShape.set( m_xIndexAccess->getByIndex( nIndex - 1 ), uno::UNO_QUERY );

        }
        // populate map with drawing::XShapes
        if ( xShape.is() )
            aShapes.push_back( xShape );
    }
    uno::Reference< container::XIndexAccess > xIndexAccess( new XNamedObjectCollectionHelper< drawing::XShape >( std::move(aShapes) ) );
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
        uno::Sequence< uno::Any > sIndices { shapes };
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
        xSelectSupp->select( uno::Any( m_xShapes ) );
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

    uno::Reference< drawing::XShape > xShape( createShape( u"com.sun.star.drawing.RectangleShape"_ustr ), uno::UNO_SET_THROW );
    m_xShapes->add( xShape );

    OUString sName(createName( u"Rectangle" ));
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

    rtl::Reference<ScVbaShape> pScVbaShape = new ScVbaShape( getParent(), mxContext, xShape, m_xShapes, m_xModel, ScVbaShape::getType( xShape ) );
    return uno::Any( uno::Reference< msforms::XShape > ( pScVbaShape ) );
}

uno::Any
ScVbaShapes::AddEllipse(sal_Int32 startX, sal_Int32 startY, sal_Int32 nLineWidth, sal_Int32 nLineHeight)
{
    sal_Int32 nXPos = Millimeter::getInHundredthsOfOneMillimeter( startX );
    sal_Int32 nYPos = Millimeter::getInHundredthsOfOneMillimeter( startY );
    sal_Int32 nWidth = Millimeter::getInHundredthsOfOneMillimeter( nLineWidth );
    sal_Int32 nHeight = Millimeter::getInHundredthsOfOneMillimeter( nLineHeight );

    uno::Reference< drawing::XShape > xShape( createShape( u"com.sun.star.drawing.EllipseShape"_ustr ), uno::UNO_SET_THROW );
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
    OUString name(createName( u"Oval" ));
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

    rtl::Reference<ScVbaShape> pScVbaShape = new ScVbaShape( getParent(), mxContext, xShape, m_xShapes, m_xModel, ScVbaShape::getType( xShape ) );
    return uno::Any( uno::Reference< msforms::XShape > ( pScVbaShape ) );
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

    uno::Reference< drawing::XShape > xShape( createShape( u"com.sun.star.drawing.LineShape"_ustr ), uno::UNO_SET_THROW );
    m_xShapes->add( xShape );

    awt::Point aMovePositionIfRange( 0, 0 );

    OUString name(createName( u"Line" ));
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

    rtl::Reference<ScVbaShape> pScVbaShape = new ScVbaShape( getParent(), mxContext, xShape, m_xShapes, m_xModel, ScVbaShape::getType( xShape ) );
    return uno::Any( uno::Reference< msforms::XShape > ( pScVbaShape ) );
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
    if( xServiceInfo->supportsService( u"com.sun.star.text.TextDocument"_ustr ) )
    {
        return AddTextboxInWriter( _nLeft, _nTop, _nWidth, _nHeight );
    }
    throw uno::RuntimeException( u"Not implemented"_ustr );
}

uno::Any
ScVbaShapes::AddTextboxInWriter( sal_Int32 _nLeft, sal_Int32 _nTop, sal_Int32 _nWidth, sal_Int32 _nHeight )
{
    sal_Int32 nXPos = Millimeter::getInHundredthsOfOneMillimeter( _nLeft );
    sal_Int32 nYPos = Millimeter::getInHundredthsOfOneMillimeter( _nTop );
    sal_Int32 nWidth = Millimeter::getInHundredthsOfOneMillimeter( _nWidth );
    sal_Int32 nHeight = Millimeter::getInHundredthsOfOneMillimeter( _nHeight );

    uno::Reference< drawing::XShape > xShape( createShape( u"com.sun.star.drawing.TextShape"_ustr ), uno::UNO_SET_THROW );
    m_xShapes->add( xShape );

    setDefaultShapeProperties(xShape);

    OUString sName(createName( u"Text Box" ));
    setShape_NameProperty( xShape, sName );

    awt::Size size;
    size.Height = nHeight;
    size.Width = nWidth;
    xShape->setSize(size);

    uno::Reference< beans::XPropertySet > xShapeProps( xShape, uno::UNO_QUERY_THROW );
    xShapeProps->setPropertyValue( u"AnchorType"_ustr, uno::Any( text::TextContentAnchorType_AT_PAGE ) );
    xShapeProps->setPropertyValue( u"HoriOrientRelation"_ustr, uno::Any( text::RelOrientation::PAGE_LEFT ) );
    xShapeProps->setPropertyValue( u"HoriOrient"_ustr, uno::Any( text::HoriOrientation::NONE ) );
    xShapeProps->setPropertyValue( u"HoriOrientPosition"_ustr, uno::Any( nXPos ) );

    xShapeProps->setPropertyValue( u"VertOrientRelation"_ustr, uno::Any( text::RelOrientation::PAGE_FRAME ) );
    xShapeProps->setPropertyValue( u"VertOrient"_ustr, uno::Any( text::VertOrientation::NONE ) );
    xShapeProps->setPropertyValue( u"VertOrientPosition"_ustr, uno::Any( nYPos ) );

    // set to visible
    xShapeProps->setPropertyValue( u"LineStyle"_ustr, uno::Any( drawing::LineStyle_SOLID ) );
    // set to font
    xShapeProps->setPropertyValue( u"LayerID"_ustr, uno::Any( sal_Int16(1) ) );
    xShapeProps->setPropertyValue( u"LayerName"_ustr, uno::Any( u"Heaven"_ustr ) );


    rtl::Reference<ScVbaShape> pScVbaShape = new ScVbaShape( getParent(), mxContext, xShape, m_xShapes, m_xModel, ScVbaShape::getType( xShape ) );
    return uno::Any( uno::Reference< msforms::XShape > ( pScVbaShape ) );
}

void
ScVbaShapes::setDefaultShapeProperties( const uno::Reference< drawing::XShape >& xShape )
{
    uno::Reference< beans::XPropertySet > xPropertySet( xShape, uno::UNO_QUERY_THROW );
    xPropertySet->setPropertyValue( u"FillStyle"_ustr, uno::Any( u"SOLID"_ustr ) );
    xPropertySet->setPropertyValue( u"FillColor"_ustr, uno::Any( sal_Int32(0xFFFFFF) )  );
    xPropertySet->setPropertyValue( u"TextWordWrap"_ustr, uno::Any( text::WrapTextMode_THROUGH )  );
    //not find in OOo2.3
    //xPropertySet->setPropertyValue("Opaque", uno::makeAny( sal_True )  );
}

void
ScVbaShapes::setShape_NameProperty( const uno::Reference< css::drawing::XShape >& xShape, const OUString& sName )
{
    uno::Reference< beans::XPropertySet > xPropertySet( xShape, uno::UNO_QUERY_THROW );
    try
    {
        xPropertySet->setPropertyValue( u"Name"_ustr, uno::Any( sName ) );
    }
    catch(const script::BasicErrorException&)
    {
    }
}

OUString
ScVbaShapes::createName( std::u16string_view sName )
{
    sal_Int32 nActNumber = 1 + m_nNewShapeCount;
    m_nNewShapeCount++;
    return sName + OUString::number( nActNumber );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
