/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"
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

//.............................................................................
namespace chart
{
//.............................................................................
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
            , const rtl::OUString& rCID )
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
        xShapeProp->setPropertyValue( C2U( "Transformation" ), uno::makeAny( B2DHomMatrixToHomogenMatrix3(aM) ) );
    }
    catch( uno::Exception& e )
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
                m_xShapeFactory->createInstance( C2U(
                "com.sun.star.drawing.TextShape" ) ), uno::UNO_QUERY );
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
            //text::WritingMode eWritingMode = text::WritingMode_LR_TB;//@todo get correct one

            aValueMap.insert( tPropertyNameValueMap::value_type( C2U("TextHorizontalAdjust"), uno::makeAny(eHorizontalAdjust) ) ); // drawing::TextHorizontalAdjust
            aValueMap.insert( tPropertyNameValueMap::value_type( C2U("TextVerticalAdjust"), uno::makeAny(eVerticalAdjust) ) ); //drawing::TextVerticalAdjust
            //aValueMap.insert( tPropertyNameValueMap::value_type( C2U("TextWritingMode"), uno::makeAny(eWritingMode) ) ); //text::WritingMode
            aValueMap.insert( tPropertyNameValueMap::value_type( C2U("TextAutoGrowHeight"), uno::makeAny(sal_True) ) ); // sal_Bool
            aValueMap.insert( tPropertyNameValueMap::value_type( C2U("TextAutoGrowWidth"), uno::makeAny(sal_True) ) ); // sal_Bool

            ////aValueMap.insert( tPropertyNameValueMap::value_type( C2U("TextMaximumFrameWidth"), uno::makeAny(rSize.Width) ) ); //sal_Int32
            ////aValueMap.insert( tPropertyNameValueMap::value_type( C2U("TextMaximumFrameHeight"), uno::makeAny(rSize.Height) ) ); //sal_Int32

            //set name/classified ObjectID (CID)
            if( m_aCID.getLength() )
                aValueMap.insert( tPropertyNameValueMap::value_type( C2U("Name"), uno::makeAny( m_aCID ) ) ); //CID rtl::OUString
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
            xTitleProperties->getPropertyValue( C2U( "StackCharacters" ) ) >>= bStackCharacters;
        }
        catch( uno::Exception& e )
        {
            ASSERT_EXCEPTION( e );
        }
        if(bStackCharacters)
        {
            //if the characters should be stacked we use only the first character properties for code simplicity
            if( aStringList.getLength()>0 )
            {
                rtl::OUString aLabel;
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
                if( xTitleProperties->getPropertyValue( C2U("ReferencePageSize")) >>= aOldRefSize )
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
                ( xTitleProperties->getPropertyValue( C2U("ReferencePageSize")) >>= aOldRefSize );

            //for( nN=0; nN<aStringList.getLength();nN++ ) //portion wise fromatting does not work still
            if( aStringList.getLength()>0 )
            {
                //uno::Reference< beans::XPropertySet > xTargetProps( aCursorList[nN], uno::UNO_QUERY );
                //uno::Reference< beans::XPropertySet > xSourceProps( aStringList[nN], uno::UNO_QUERY );
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
        if ( xShapeProp.is() && ( xShapeProp->getPropertyValue( C2U( "CharHeight" ) ) >>= fFontHeight ) )
        {
            fFontHeight *= ( 2540. / 72. );  // pt -> 1/100 mm
            float fXFraction = 0.18;
            sal_Int32 nXDistance = static_cast< sal_Int32 >( ::rtl::math::round( fFontHeight * fXFraction ) );
            float fYFraction = 0.30;
            sal_Int32 nYDistance = static_cast< sal_Int32 >( ::rtl::math::round( fFontHeight * fYFraction ) );
            xShapeProp->setPropertyValue( C2U( "TextLeftDistance" ), uno::makeAny( nXDistance ) );
            xShapeProp->setPropertyValue( C2U( "TextRightDistance" ), uno::makeAny( nXDistance ) );
            xShapeProp->setPropertyValue( C2U( "TextUpperDistance" ), uno::makeAny( nYDistance ) );
            xShapeProp->setPropertyValue( C2U( "TextLowerDistance" ), uno::makeAny( nYDistance ) );
        }

        try
        {
            double fAngleDegree = 0;
            xTitleProperties->getPropertyValue( C2U( "TextRotation" ) ) >>= fAngleDegree;
            m_fRotationAngleDegree += fAngleDegree;
        }
        catch( uno::Exception& e )
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
        xShapeProp->setPropertyValue( C2U( "Transformation" ), uno::makeAny( B2DHomMatrixToHomogenMatrix3(aM) ) );
    }
    catch( uno::Exception& e )
    {
        ASSERT_EXCEPTION( e );
    }
}

//.............................................................................
} //namespace chart
//.............................................................................
