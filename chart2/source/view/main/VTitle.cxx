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

#include "VTitle.hxx"
#include "CommonConverters.hxx"
#include "macros.hxx"
#include "PropertyMapper.hxx"
#include "ShapeFactory.hxx"
#include "RelativeSizeHelper.hxx"
#include <com/sun/star/chart2/XFormattedString.hpp>
#include <rtl/math.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/TextVerticalAdjust.hpp>
#include <com/sun/star/drawing/TextHorizontalAdjust.hpp>
#include <com/sun/star/text/ControlCharacter.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/text/XTextCursor.hpp>

namespace chart
{
using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

VTitle::VTitle( const uno::Reference< XTitle > & xTitle )
                : m_xTarget(NULL)
                , m_xShapeFactory(NULL)
                , m_xTitle(xTitle)
                , m_xShape(NULL)
                , m_aCID()
                , m_fRotationAngleDegree(0.0)
                , m_nXPos(0)
                , m_nYPos(0)
{
}

VTitle::~VTitle()
{
}

void VTitle::init(
              const uno::Reference< drawing::XShapes >& xTargetPage
            , const uno::Reference< lang::XMultiServiceFactory >& xFactory
            , const OUString& rCID )
{
    m_xTarget = xTargetPage;
    m_xShapeFactory = xFactory;
    m_aCID = rCID;
}

double VTitle::getRotationAnglePi() const
{
    return m_fRotationAngleDegree*F_PI/180.0;
}

awt::Size VTitle::getUnrotatedSize() const //size before rotation
{
    awt::Size aRet;
    if(m_xShape.is())
        aRet = m_xShape->getSize();
    return aRet;
}

awt::Size VTitle::getFinalSize() const //size after rotation
{
    return ShapeFactory::getSizeAfterRotation(
         m_xShape, m_fRotationAngleDegree );
}

void VTitle::changePosition( const awt::Point& rPos )
{
    if(!m_xShape.is())
        return;
    uno::Reference< beans::XPropertySet > xShapeProp( m_xShape, uno::UNO_QUERY );
    if(!xShapeProp.is())
        return;
    try
    {
        m_nXPos = rPos.X;
        m_nYPos = rPos.Y;

        //set position matrix
        //the matrix needs to be set at the end behind autogrow and such position influencing properties
        ::basegfx::B2DHomMatrix aM;
        aM.rotate( -m_fRotationAngleDegree*F_PI/180.0 );//#i78696#->#i80521#
        aM.translate( m_nXPos, m_nYPos);
        xShapeProp->setPropertyValue( "Transformation", uno::makeAny( B2DHomMatrixToHomogenMatrix3(aM) ) );
    }
    catch( const uno::Exception& e )
    {
        ASSERT_EXCEPTION( e );
    }
}

void VTitle::createShapes(
      const awt::Point& rPos
    , const awt::Size& rReferenceSize )
{
    try
    {
        if(!m_xTitle.is())
            return;

        uno::Sequence< uno::Reference< XFormattedString > > aStringList = m_xTitle->getText();
        if(aStringList.getLength()<=0)
            return;

        //create shape and add to page
        uno::Reference< drawing::XShape > xShape(
                m_xShapeFactory->createInstance(
                "com.sun.star.drawing.TextShape" ), uno::UNO_QUERY );
        m_xTarget->add(xShape);
        m_xShape = xShape;

        //set text and text properties
        uno::Reference< text::XText > xText( xShape, uno::UNO_QUERY );
        uno::Reference< text::XTextCursor > xTextCursor( xText->createTextCursor() );
        uno::Reference< text::XTextRange > xTextRange( xTextCursor, uno::UNO_QUERY );
        uno::Reference< beans::XPropertySet > xShapeProp( xShape, uno::UNO_QUERY );
        uno::Reference< beans::XPropertySet > xTitleProperties( m_xTitle, uno::UNO_QUERY );
        if( !xText.is() || !xTextRange.is() || !xTextCursor.is() || !xShapeProp.is() || !xTitleProperties.is() )
            return;

        tPropertyNameValueMap aValueMap;
        //fill line-, fill- and paragraph-properties into the ValueMap
        {
            tMakePropertyNameMap aNameMap = PropertyMapper::getPropertyNameMapForParagraphProperties();
            aNameMap( PropertyMapper::getPropertyNameMapForFillAndLineProperties() );

            PropertyMapper::getValueMap( aValueMap, aNameMap, xTitleProperties );
        }

        //fill some more shape properties into the ValueMap
        {
            drawing::TextHorizontalAdjust eHorizontalAdjust = drawing::TextHorizontalAdjust_CENTER;
            drawing::TextVerticalAdjust eVerticalAdjust = drawing::TextVerticalAdjust_CENTER;

            aValueMap.insert( tPropertyNameValueMap::value_type( "TextHorizontalAdjust", uno::makeAny(eHorizontalAdjust) ) ); // drawing::TextHorizontalAdjust
            aValueMap.insert( tPropertyNameValueMap::value_type( "TextVerticalAdjust", uno::makeAny(eVerticalAdjust) ) ); //drawing::TextVerticalAdjust
            aValueMap.insert( tPropertyNameValueMap::value_type( "TextAutoGrowHeight", uno::makeAny(sal_True) ) ); // sal_Bool
            aValueMap.insert( tPropertyNameValueMap::value_type( "TextAutoGrowWidth", uno::makeAny(sal_True) ) ); // sal_Bool

            //set name/classified ObjectID (CID)
            if( !m_aCID.isEmpty() )
                aValueMap.insert( tPropertyNameValueMap::value_type( "Name", uno::makeAny( m_aCID ) ) ); //CID OUString
        }

        //set global title properties
        {
            tNameSequence aPropNames;
            tAnySequence aPropValues;
            PropertyMapper::getMultiPropertyListsFromValueMap( aPropNames, aPropValues, aValueMap );
            PropertyMapper::setMultiProperties( aPropNames, aPropValues, xShapeProp );
        }

        sal_Bool bStackCharacters(sal_False);
        try
        {
            xTitleProperties->getPropertyValue( "StackCharacters" ) >>= bStackCharacters;
        }
        catch( const uno::Exception& e )
        {
            ASSERT_EXCEPTION( e );
        }
        if(bStackCharacters)
        {
            //if the characters should be stacked we use only the first character properties for code simplicity
            if( aStringList.getLength()>0 )
            {
                OUString aLabel;
                for( sal_Int32 nN=0; nN<aStringList.getLength();nN++ )
                    aLabel += aStringList[nN]->getString();
                aLabel = ShapeFactory::getStackedString( aLabel, bStackCharacters );

                xTextCursor->gotoEnd(false);
                xText->insertString( xTextRange, aLabel, false );
                xTextCursor->gotoEnd(true);
                uno::Reference< beans::XPropertySet > xTargetProps( xShape, uno::UNO_QUERY );
                uno::Reference< beans::XPropertySet > xSourceProps( aStringList[0], uno::UNO_QUERY );

                PropertyMapper::setMappedProperties( xTargetProps, xSourceProps
                    , PropertyMapper::getPropertyNameMapForCharacterProperties() );

                // adapt font size according to page size
                awt::Size aOldRefSize;
                if( xTitleProperties->getPropertyValue( "ReferencePageSize") >>= aOldRefSize )
                {
                    RelativeSizeHelper::adaptFontSizes( xTargetProps, aOldRefSize, rReferenceSize );
                }
            }
        }
        else
        {
            uno::Sequence< uno::Reference< text::XTextCursor > > aCursorList( aStringList.getLength() );
            sal_Int32 nN = 0;
            for( nN=0; nN<aStringList.getLength();nN++ )
            {
                xTextCursor->gotoEnd(false);
                xText->insertString( xTextRange, aStringList[nN]->getString(), false );
                xTextCursor->gotoEnd(true);
                aCursorList[nN] = xText->createTextCursorByRange( uno::Reference< text::XTextRange >(xTextCursor,uno::UNO_QUERY) );
            }
            awt::Size aOldRefSize;
            bool bHasRefPageSize =
                ( xTitleProperties->getPropertyValue( "ReferencePageSize") >>= aOldRefSize );

            if( aStringList.getLength()>0 )
            {
                uno::Reference< beans::XPropertySet > xTargetProps( xShape, uno::UNO_QUERY );
                uno::Reference< beans::XPropertySet > xSourceProps( aStringList[0], uno::UNO_QUERY );
                PropertyMapper::setMappedProperties( xTargetProps, xSourceProps, PropertyMapper::getPropertyNameMapForCharacterProperties() );

                // adapt font size according to page size
                if( bHasRefPageSize )
                {
                    RelativeSizeHelper::adaptFontSizes( xTargetProps, aOldRefSize, rReferenceSize );
                }
            }
        }

        // #i109336# Improve auto positioning in chart
        float fFontHeight = 0.0;
        if ( xShapeProp.is() && ( xShapeProp->getPropertyValue( "CharHeight" ) >>= fFontHeight ) )
        {
            fFontHeight *= ( 2540.0f / 72.0f );  // pt -> 1/100 mm
            float fXFraction = 0.18f;
            sal_Int32 nXDistance = static_cast< sal_Int32 >( ::rtl::math::round( fFontHeight * fXFraction ) );
            float fYFraction = 0.30f;
            sal_Int32 nYDistance = static_cast< sal_Int32 >( ::rtl::math::round( fFontHeight * fYFraction ) );
            xShapeProp->setPropertyValue( "TextLeftDistance", uno::makeAny( nXDistance ) );
            xShapeProp->setPropertyValue( "TextRightDistance", uno::makeAny( nXDistance ) );
            xShapeProp->setPropertyValue( "TextUpperDistance", uno::makeAny( nYDistance ) );
            xShapeProp->setPropertyValue( "TextLowerDistance", uno::makeAny( nYDistance ) );
        }

        try
        {
            double fAngleDegree = 0;
            xTitleProperties->getPropertyValue( "TextRotation" ) >>= fAngleDegree;
            m_fRotationAngleDegree += fAngleDegree;
        }
        catch( const uno::Exception& e )
        {
            ASSERT_EXCEPTION( e );
        }
        m_nXPos = rPos.X;
        m_nYPos = rPos.Y;

        //set position matrix
        //the matrix needs to be set at the end behind autogrow and such position influencing properties
        ::basegfx::B2DHomMatrix aM;
        aM.rotate( -m_fRotationAngleDegree*F_PI/180.0 );//#i78696#->#i80521#
        aM.translate( m_nXPos, m_nYPos );
        xShapeProp->setPropertyValue( "Transformation", uno::makeAny( B2DHomMatrixToHomogenMatrix3(aM) ) );
    }
    catch( const uno::Exception& e )
    {
        ASSERT_EXCEPTION( e );
    }
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
