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
#include "AbstractShapeFactory.hxx"
#include <com/sun/star/chart2/XFormattedString.hpp>
#include <rtl/math.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/text/ControlCharacter.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/text/XTextCursor.hpp>

namespace chart
{
using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

VTitle::VTitle( const uno::Reference< XTitle > & xTitle )
                : m_xTarget(nullptr)
                , m_xShapeFactory(nullptr)
                , m_xTitle(xTitle)
                , m_xShape(nullptr)
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
    return AbstractShapeFactory::getSizeAfterRotation(
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
        xShapeProp->setPropertyValue( "Transformation", uno::Any( B2DHomMatrixToHomogenMatrix3(aM) ) );
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
    if(!m_xTitle.is())
        return;

    uno::Sequence< uno::Reference< XFormattedString > > aStringList = m_xTitle->getText();
    if(aStringList.getLength()<=0)
        return;

    m_nXPos = rPos.X;
    m_nYPos = rPos.Y;

    uno::Reference< beans::XPropertySet > xTitleProperties( m_xTitle, uno::UNO_QUERY );

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

    AbstractShapeFactory* pShapeFactory = AbstractShapeFactory::getOrCreateShapeFactory(m_xShapeFactory);
    m_xShape =pShapeFactory->createText( m_xTarget, rReferenceSize, rPos, aStringList,
            xTitleProperties, m_fRotationAngleDegree, m_aCID );
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
