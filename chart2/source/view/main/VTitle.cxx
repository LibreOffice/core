/*************************************************************************
 *
 *  $RCSfile: VTitle.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: bm $ $Date: 2003-10-17 14:50:29 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2003 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#include "VTitle.hxx"
#include "CommonConverters.hxx"
#include "macros.hxx"
#include "PropertyMapper.hxx"
#include "ShapeFactory.hxx"
#include "chartview/ObjectIdentifier.hxx"

#ifndef _DRAFTS_COM_SUN_STAR_CHART2_XFORMATTEDSTRING_HPP_
#include <drafts/com/sun/star/chart2/XFormattedString.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_CHART2_XIDENTIFIABLE_HPP_
#include <drafts/com/sun/star/chart2/XIdentifiable.hpp>
#endif

// header for class Matrix3D
#ifndef _B2D_MATRIX3D_HXX
#include <goodies/matrix3d.hxx>
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_TEXTVERTICALADJUST_HPP_
#include <com/sun/star/drawing/TextVerticalAdjust.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_TEXTHORIZONTALADJUST_HPP_
#include <com/sun/star/drawing/TextHorizontalAdjust.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_CONTROLCHARACTER_HPP_
#include <com/sun/star/text/ControlCharacter.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXT_HPP_
#include <com/sun/star/text/XText.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXTCURSOR_HPP_
#include <com/sun/star/text/XTextCursor.hpp>
#endif

//.............................................................................
namespace chart
{
//.............................................................................
using namespace ::com::sun::star;
using namespace ::drafts::com::sun::star::chart2;

VTitle::VTitle( const uno::Reference< XTitle > & xTitle )
                : m_xTarget(NULL)
                , m_xShapeFactory(NULL)
                , m_xTitle(xTitle)
                , m_xShape(NULL)
                , m_fRotationAngleDegree(0.0)
                , m_nXPos(0)
                , m_nYPos(0)
{
}

VTitle::~VTitle()
{
}

void SAL_CALL VTitle::init(
              const uno::Reference< drawing::XShapes >& xTargetPage
            , const uno::Reference< lang::XMultiServiceFactory >& xFactory )
{
    m_xTarget = xTargetPage;
    m_xShapeFactory = xFactory;
}


awt::Size VTitle::getSize() const
{
    awt::Size aRet;
    if(m_xShape.is())
    {
        aRet = m_xShape->getSize();
    }
    return aRet;
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
        Matrix3D aM3;
        aM3.Scale( 1, 1 );
        aM3.Rotate( m_fRotationAngleDegree );
        aM3.Translate( m_nXPos, m_nYPos);
        xShapeProp->setPropertyValue( C2U( "Transformation" ), uno::makeAny( Matrix3DToHomogenMatrix3(aM3) ) );
    }
    catch( uno::Exception& e )
    {
        ASSERT_EXCEPTION( e );
    }
}

void VTitle::createShapes( const awt::Point& rPos )
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
        uno::Reference< text::XTextRange > xTextRange( xShape, uno::UNO_QUERY );
        uno::Reference< text::XTextCursor > xTextCursor( xText->createTextCursor() );
        uno::Reference< beans::XPropertySet > xShapeProp( xShape, uno::UNO_QUERY );
        uno::Reference< beans::XPropertySet > xTitleProperties( m_xTitle, uno::UNO_QUERY );
        if( !xText.is() || !xTextRange.is() || !xTextCursor.is() || !xShapeProp.is() || !xTitleProperties.is() )
            return;

        tPropertyNameValueMap aValueMap;
        //fill line-, fill- and paragraph-properties into the ValueMap
        {
            tMakePropertyNameMap aNameMap = PropertyMapper::getPropertyNameMapForParagraphProperties();
            const tMakePropertyNameMap& rFillPropMap = PropertyMapper::getPropertyNameMapForFillProperties();
            const tMakePropertyNameMap& rLinePropMap = PropertyMapper::getPropertyNameMapForLineProperties();
            aNameMap.insert(rFillPropMap.begin(),rFillPropMap.end());
            aNameMap.insert(rLinePropMap.begin(),rLinePropMap.end());

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
            uno::Reference< XIdentifiable > xIdent( m_xTitle, uno::UNO_QUERY );
            if( xIdent.is())
            {
                rtl::OUString aCID = ObjectIdentifier::createClassifiedIdentifier( OBJECTTYPE_TITLE, xIdent->getIdentifier() );
                aValueMap.insert( tPropertyNameValueMap::value_type( C2U("Name"), uno::makeAny( aCID ) ) ); //CID rtl::OUString
            }
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
                uno::Reference< beans::XPropertySet > xTargetProps( xTextCursor, uno::UNO_QUERY );
                uno::Reference< beans::XPropertySet > xSourceProps( aStringList[0], uno::UNO_QUERY );

                PropertyMapper::setMappedProperties( xTargetProps, xSourceProps
                    , PropertyMapper::getPropertyNameMapForCharacterProperties() );
            }
        }
        else
        {
            uno::Sequence< uno::Reference< text::XTextCursor > > aCursorList( aStringList.getLength() );
            for( sal_Int32 nN=0; nN<aStringList.getLength();nN++ )
            {
                xTextCursor->gotoEnd(false);
                xText->insertString( xTextRange, aStringList[nN]->getString(), false );
                xTextCursor->gotoEnd(true);
                aCursorList[nN] = xText->createTextCursorByRange( uno::Reference< text::XTextRange >(xTextCursor,uno::UNO_QUERY) );
            }
            for( nN=0; nN<aStringList.getLength();nN++ )
            {
                uno::Reference< beans::XPropertySet > xTargetProps( aCursorList[nN], uno::UNO_QUERY );
                uno::Reference< beans::XPropertySet > xSourceProps( aStringList[nN], uno::UNO_QUERY );
                PropertyMapper::setMappedProperties( xTargetProps, xSourceProps, PropertyMapper::getPropertyNameMapForCharacterProperties() );
            }
        }

        double fRotationAngleDegree( 0.0 );
        try
        {
            xTitleProperties->getPropertyValue( C2U( "TextRotation" ) ) >>= fRotationAngleDegree;
            // convert DEG to RAD
            fRotationAngleDegree *= ( F_PI / 180.0 );
        }
        catch( uno::Exception& e )
        {
            ASSERT_EXCEPTION( e );
        }
        m_fRotationAngleDegree = fRotationAngleDegree;
        m_nXPos = rPos.X;
        m_nYPos = rPos.Y;

        //set position matrix
        //the matrix needs to be set at the end behind autogrow and such position influencing properties
        Matrix3D aM3;
        aM3.Scale( 1, 1 );
        aM3.Rotate( m_fRotationAngleDegree );
        aM3.Translate( m_nXPos, m_nYPos );
        xShapeProp->setPropertyValue( C2U( "Transformation" ), uno::makeAny( Matrix3DToHomogenMatrix3(aM3) ) );
    }
    catch( uno::Exception& e )
    {
        ASSERT_EXCEPTION( e );
    }
}

//.............................................................................
} //namespace chart
//.............................................................................
