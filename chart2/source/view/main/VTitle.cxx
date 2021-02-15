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
#include <CommonConverters.hxx>
#include <ShapeFactory.hxx>
#include <com/sun/star/chart2/XTitle.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <tools/diagnose_ex.h>

namespace chart
{
using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

VTitle::VTitle( const uno::Reference< XTitle > & xTitle )
                : m_xTitle(xTitle)
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
    return basegfx::deg2rad(m_fRotationAngleDegree);
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
        xShapeProp->setPropertyValue( "Transformation", uno::Any( B2DHomMatrixToHomogenMatrix3(aM) ) );
    }
    catch( const uno::Exception& )
    {
        TOOLS_WARN_EXCEPTION("chart2", "" );
    }
}

bool VTitle::isVisible(const uno::Reference< XTitle >& xTitle) {
    if (!xTitle.is()) {
        return false;
    }
    bool bShow = true;
    try {
        uno::Reference< beans::XPropertySet > xTitleProps(xTitle, uno::UNO_QUERY_THROW);
        xTitleProps->getPropertyValue("Visible") >>= bShow;
    } catch (const uno::Exception &) {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }
    return bShow;
}


void VTitle::createShapes(
      const awt::Point& rPos
    , const awt::Size& rReferenceSize
    , const awt::Size& rTextMaxWidth
    , bool bYAxisTitle )
{
    if(!m_xTitle.is())
        return;

    uno::Sequence< uno::Reference< XFormattedString > > aStringList = m_xTitle->getText();
    if(!aStringList.hasElements())
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
    catch( const uno::Exception& )
    {
        TOOLS_WARN_EXCEPTION("chart2", "" );
    }

    sal_Int32 nTextMaxWidth;
    if (bYAxisTitle)
    {
        if (m_fRotationAngleDegree < 75.0 || m_fRotationAngleDegree > 285.0
            || (m_fRotationAngleDegree > 105.0 && m_fRotationAngleDegree < 255.0))
            nTextMaxWidth = rTextMaxWidth.Width;
        else
            nTextMaxWidth = rTextMaxWidth.Height;
    }
    else if (m_fRotationAngleDegree <= 15.0 || m_fRotationAngleDegree >= 345.0
             || (m_fRotationAngleDegree >= 165.0 && m_fRotationAngleDegree <= 195.0))
        nTextMaxWidth = rTextMaxWidth.Width;
    else
        nTextMaxWidth = rTextMaxWidth.Height;

    ShapeFactory* pShapeFactory = ShapeFactory::getOrCreateShapeFactory(m_xShapeFactory);
    m_xShape =pShapeFactory->createText( m_xTarget, rReferenceSize, rPos, aStringList, xTitleProperties,
                                    m_fRotationAngleDegree, m_aCID, nTextMaxWidth );
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
