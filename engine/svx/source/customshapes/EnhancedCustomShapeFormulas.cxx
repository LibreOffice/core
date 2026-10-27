/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
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

#include <sal/config.h>

#include <svx/EnhancedCustomShapeFormulas.hxx>
#include <svx/EnhancedCustomShapeTypeNames.hxx>
#include <svx/sdasitm.hxx>
#include <comphelper/configuration.hxx>
#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeParameterType.hpp>
#include <o3tl/safeint.hxx>
#include <sal/log.hxx>

#include <cmath>

using namespace ::com::sun::star;
using namespace ::cpo::uno;
using namespace ::com::sun::star::drawing;

using EnhancedCustomShape::ExpressionFunct;

EnhancedCustomShapeFormulas::EnhancedCustomShapeFormulas(
    const SdrCustomShapeGeometryItem& rGeometryItem, const tools::Rectangle& rLogicRect,
    bool bFilled, bool bStroked)
    : m_eSpType(mso_sptNil)
    , m_nCoordLeft(0)
    , m_nCoordTop(0)
    , m_nCoordWidthG(21600)
    , m_nCoordHeightG(21600)
    , m_nCoordWidth(0)
    , m_nCoordHeight(0)
    , m_aLogicRect(rLogicRect)
    , m_fXScale(1.0)
    , m_fYScale(1.0)
    , m_fXRatio(1.0)
    , m_fYRatio(1.0)
    , m_bOOXMLShape(false)
    , m_nXRef(0x80000000)
    , m_nYRef(0x80000000)
    , m_bFilled(bFilled)
    , m_bStroked(bStroked)
    , m_bFlipH(false)
    , m_bFlipV(false)
{
    OUString sShapeType;
    const Any* pAny = rGeometryItem.GetPropertyValueByName(u"Type"_ustr);
    if (pAny)
    {
        *pAny >>= sShapeType;
        m_bOOXMLShape = sShapeType.startsWith("ooxml-");
        SAL_INFO("svx", "shape type: " << sShapeType << " " << m_bOOXMLShape);
    }
    m_eSpType = EnhancedCustomShapeTypeNames::Get(sShapeType);

    pAny = rGeometryItem.GetPropertyValueByName(u"MirroredX"_ustr);
    if (pAny)
        *pAny >>= m_bFlipH;
    pAny = rGeometryItem.GetPropertyValueByName(u"MirroredY"_ustr);
    if (pAny)
        *pAny >>= m_bFlipV;

    ApplyShapeAttributes(rGeometryItem);
    SetPathSize();

    if (m_seqEquations.hasElements())
        m_vEquationResults.resize(m_seqEquations.getLength());
}

tools::Rectangle
EnhancedCustomShapeFormulas::GetTextRectFor(const Sequence<beans::PropertyValue>& rGeometry,
                                            const Size& rSize)
{
    const EnhancedCustomShapeFormulas aFormulas(SdrCustomShapeGeometryItem(rGeometry),
                                                tools::Rectangle(Point(0, 0), rSize), true, true);
    return aFormulas.GetTextRect();
}

void EnhancedCustomShapeFormulas::ApplyShapeAttributes( const SdrCustomShapeGeometryItem& rGeometryItem )
{
    // AdjustmentValues
    const Any* pAny = rGeometryItem.GetPropertyValueByName( u"AdjustmentValues"_ustr );
    if ( pAny )
        *pAny >>= m_seqAdjustmentValues;


    // Coordsize
    const Any* pViewBox = rGeometryItem.GetPropertyValueByName( u"ViewBox"_ustr );
    css::awt::Rectangle aViewBox;
    if ( pViewBox && (*pViewBox >>= aViewBox ) )
    {
        m_nCoordLeft    = aViewBox.X;
        m_nCoordTop     = aViewBox.Y;
        m_nCoordWidthG  = aViewBox.Width;
        if (m_nCoordWidthG < 0)
            m_nCoordWidthG = o3tl::saturating_toggle_sign(m_nCoordWidthG);
        m_nCoordHeightG = aViewBox.Height;
        if (m_nCoordHeightG < 0)
            m_nCoordHeightG = o3tl::saturating_toggle_sign(m_nCoordHeightG);
    }
    static constexpr OUString sPath( u"Path"_ustr );

    // Path/Coordinates
    pAny = rGeometryItem.GetPropertyValueByName( sPath, u"Coordinates"_ustr );
    if ( pAny )
        *pAny >>= m_seqCoordinates;


    // Path/GluePoints
    pAny = rGeometryItem.GetPropertyValueByName( sPath, u"GluePoints"_ustr );
    if ( pAny )
        *pAny >>= m_seqGluePoints;

    // Path/GluePointLeavingDirections
    pAny = rGeometryItem.GetPropertyValueByName(sPath, u"GluePointLeavingDirections"_ustr);
    if (pAny)
        *pAny >>= m_seqGluePointLeavingDirections;

    // Path/Segments
    pAny = rGeometryItem.GetPropertyValueByName( sPath, u"Segments"_ustr );
    if ( pAny )
        *pAny >>= m_seqSegments;


    // Path/SubViewSize
    pAny = rGeometryItem.GetPropertyValueByName( sPath, u"SubViewSize"_ustr );
    if ( pAny )
        *pAny >>= m_seqSubViewSize;


    // Path/StretchX
    pAny = rGeometryItem.GetPropertyValueByName( sPath, u"StretchX"_ustr );
    if ( pAny )
    {
        sal_Int32 nStretchX = 0;
        if ( *pAny >>= nStretchX )
            m_nXRef = nStretchX;
    }


    // Path/StretchY
    pAny = rGeometryItem.GetPropertyValueByName( sPath, u"StretchY"_ustr );
    if ( pAny )
    {
        sal_Int32 nStretchY = 0;
        if ( *pAny >>= nStretchY )
            m_nYRef = nStretchY;
    }


    // Path/TextFrames
    pAny = rGeometryItem.GetPropertyValueByName( sPath, u"TextFrames"_ustr );
    if ( pAny )
        *pAny >>= m_seqTextFrames;


    // Equations
    pAny = rGeometryItem.GetPropertyValueByName( u"Equations"_ustr );
    if ( pAny )
        *pAny >>= m_seqEquations;


    // Handles
    pAny = rGeometryItem.GetPropertyValueByName( u"Handles"_ustr );
    if ( pAny )
        *pAny >>= m_seqHandles;
}

void EnhancedCustomShapeFormulas::SetPathSize( sal_Int32 nIndex )
{
    sal_Int32 nWidth = 0;
    sal_Int32 nHeight = 0;

    if ( m_seqSubViewSize.hasElements() && nIndex < m_seqSubViewSize.getLength() ) {
        nWidth = m_seqSubViewSize[ nIndex ].Width;
        nHeight = m_seqSubViewSize[ nIndex ].Height;
        SAL_INFO(
            "svx",
            "set subpath " << nIndex << " size: " << nWidth << " x "
                << nHeight);
    }

    if ( nWidth && nHeight ) {
        m_nCoordWidth = nWidth;
        m_nCoordHeight = nHeight;
    } else {
        m_nCoordWidth = m_nCoordWidthG;
        m_nCoordHeight = m_nCoordHeightG;
    }

    m_fXScale = m_nCoordWidth == 0 ? 0.0 : static_cast<double>(m_aLogicRect.GetWidth()) / static_cast<double>(m_nCoordWidth);
    m_fYScale = m_nCoordHeight == 0 ? 0.0 : static_cast<double>(m_aLogicRect.GetHeight()) / static_cast<double>(m_nCoordHeight);
    if ( m_bOOXMLShape )
    {
        SAL_INFO(
            "svx",
            "ooxml shape, path width: " << m_nCoordWidth << " height: "
                << m_nCoordHeight);

        // Try to set up scale separately, if given only width or height
        // This is possible case in OOXML when only width or height is non-zero
        if ( m_nCoordWidth == 0 )
        {
            if ( nWidth )
                m_fXScale = static_cast<double>(m_aLogicRect.GetWidth()) / static_cast<double>(nWidth);
            else
                m_fXScale = 1.0;
        }
        if ( m_nCoordHeight == 0 )
        {
            if ( nHeight )
                m_fYScale = static_cast<double>(m_aLogicRect.GetHeight()) / static_cast<double>(nHeight);
            else
                m_fYScale = 1.0;
        }
    }
    if ( static_cast<sal_uInt32>(m_nXRef) != 0x80000000 && m_aLogicRect.GetHeight() )
    {
        m_fXRatio = static_cast<double>(m_aLogicRect.GetWidth()) / static_cast<double>(m_aLogicRect.GetHeight());
        if ( m_fXRatio > 1 )
            m_fXScale /= m_fXRatio;
        else
            m_fXRatio = 1.0;
    }
    else
        m_fXRatio = 1.0;
    if ( static_cast<sal_uInt32>(m_nYRef) != 0x80000000 && m_aLogicRect.GetWidth() )
    {
        m_fYRatio = static_cast<double>(m_aLogicRect.GetHeight()) / static_cast<double>(m_aLogicRect.GetWidth());
        if ( m_fYRatio > 1 )
            m_fYScale /= m_fYRatio;
        else
            m_fYRatio = 1.0;
    }
    else
        m_fYRatio = 1.0;

    if (comphelper::IsFuzzing())
    {
        if (fabs(m_fXScale) > 100000)
        {
            SAL_WARN("svx", "unreasonable X Scale of: " << m_fXScale);
            m_fXScale = 1.0;
        }
        if (fabs(m_fYScale) > 100000)
        {
            SAL_WARN("svx", "unreasonable Y Scale of: " << m_fYScale);
            m_fYScale = 1.0;
        }
    }
}

double EnhancedCustomShapeFormulas::GetEnumFunc( const ExpressionFunct eFunc ) const
{
    double fRet = 0.0;
    switch( eFunc )
    {
        case ExpressionFunct::EnumPi :         fRet = M_PI; break;
        case ExpressionFunct::EnumLeft :       fRet = static_cast<double>(m_nCoordLeft); break;
        case ExpressionFunct::EnumTop :        fRet = static_cast<double>(m_nCoordTop); break;
        case ExpressionFunct::EnumRight :      fRet = (static_cast<double>(m_nCoordLeft) + static_cast<double>(m_nCoordWidth)) * m_fXRatio; break;
        case ExpressionFunct::EnumBottom :     fRet = (static_cast<double>(m_nCoordTop) + static_cast<double>(m_nCoordHeight)) * m_fYRatio; break;
        case ExpressionFunct::EnumXStretch :   fRet = m_nXRef; break;
        case ExpressionFunct::EnumYStretch :   fRet = m_nYRef; break;
        case ExpressionFunct::EnumHasStroke :  fRet = m_bStroked ? 1.0 : 0.0; break;
        case ExpressionFunct::EnumHasFill :    fRet = m_bFilled ? 1.0 : 0.0; break;
        case ExpressionFunct::EnumWidth :      fRet = m_nCoordWidth; break;
        case ExpressionFunct::EnumHeight :     fRet = m_nCoordHeight; break;
        case ExpressionFunct::EnumLogWidth :   fRet = m_aLogicRect.GetWidth(); break;
        case ExpressionFunct::EnumLogHeight :  fRet = m_aLogicRect.GetHeight(); break;
        default: break;
    }
    return fRet;
}
double EnhancedCustomShapeFormulas::GetAdjustValueAsDouble( const sal_Int32 nIndex ) const
{
    double fNumber = 0.0;
    if ( nIndex < m_seqAdjustmentValues.getLength() )
    {
        if ( m_seqAdjustmentValues[ nIndex ].Value.getValueTypeClass() == TypeClass_DOUBLE )
            m_seqAdjustmentValues[ nIndex ].Value >>= fNumber;
        else
        {
            sal_Int32 nNumber = 0;
            m_seqAdjustmentValues[ nIndex ].Value >>= nNumber;
            fNumber = static_cast<double>(nNumber);
        }
    }
    return fNumber;
}
double EnhancedCustomShapeFormulas::GetEquationValueAsDouble( const sal_Int32 nIndex ) const
{
    double fNumber = 0.0;
    static sal_uInt32 nLevel = 0;
    if ( nIndex >= static_cast<sal_Int32>(m_vEquationResults.size()) )
        return fNumber;

    if (!m_vEquationResults[nIndex].bParsed)
    {
        m_vEquationResults[nIndex].bParsed = true;
        try
        {
            m_vEquationResults[nIndex].xNode = EnhancedCustomShape::FunctionParser::parseFunction( m_seqEquations[ nIndex ], *this );
        }
        catch ( EnhancedCustomShape::ParseError& )
        {
            SAL_INFO(
                "svx",
                "error: equation number: " << nIndex << ", parser failed ("
                    << m_seqEquations[nIndex] << ")");
        }
    }
    if ( m_vEquationResults[ nIndex ].xNode )
    {
        nLevel ++;
        try
        {
            if ( m_vEquationResults[ nIndex ].bReady )
                fNumber = m_vEquationResults[ nIndex ].fValue;
            else {
                // cast to non const, so that we can optimize by caching
                // equation results, without changing all the const in the stack
                struct EquationResult &aResult = const_cast<EnhancedCustomShapeFormulas*>(this)->m_vEquationResults[ nIndex ];

                fNumber = aResult.fValue = (*m_vEquationResults[ nIndex ].xNode)();
                aResult.bReady = true;

                SAL_INFO("svx", "equation " << nLevel << " (level: " << m_seqEquations[nIndex] << "): "
                         << fNumber << " --> " << 180.0*fNumber/10800000.0);
            }
            if ( !std::isfinite( fNumber ) )
                fNumber = 0.0;
        }
        catch ( ... )
        {
            SAL_WARN("svx", "EnhancedCustomShapeFormulas::GetEquationValueAsDouble failed");
        }
        nLevel --;
    }
    SAL_INFO(
        "svx",
        "?" << nIndex << " --> " << fNumber << " (angle: "
            << 180.0*fNumber/10800000.0 << ")");

    return fNumber;
}

bool EnhancedCustomShapeFormulas::SetAdjustValueAsDouble( const double& rValue, const sal_Int32 nIndex )
{
    bool bRetValue = false;
    if ( nIndex < m_seqAdjustmentValues.getLength() )
    {
        // updating our local adjustment sequence
        auto pseqAdjustmentValues = m_seqAdjustmentValues.getArray();
        pseqAdjustmentValues[ nIndex ].Value <<= rValue;
        pseqAdjustmentValues[ nIndex ].State = css::beans::PropertyState_DIRECT_VALUE;
        bRetValue = true;
    }
    return bRetValue;
}

basegfx::B2DPoint EnhancedCustomShapeFormulas::GetPointAsB2DPoint( const css::drawing::EnhancedCustomShapeParameterPair& rPair,
                                        const bool bScale, const bool bReplaceGeoSize ) const
{
    double fValX, fValY;
    // width
    GetParameter(fValX, rPair.First, bReplaceGeoSize, false);
    fValX -= m_nCoordLeft;
    if (bScale)
    {
        fValX *= m_fXScale;
    }
    // height
    GetParameter(fValY, rPair.Second, false, bReplaceGeoSize);
    fValY -= m_nCoordTop;
    if (bScale)
    {
        fValY *= m_fYScale;
    }
    return basegfx::B2DPoint(fValX,fValY);
}

Point EnhancedCustomShapeFormulas::GetPoint( const css::drawing::EnhancedCustomShapeParameterPair& rPair,
                                        const bool bScale, const bool bReplaceGeoSize ) const
{
    basegfx::B2DPoint aPoint(GetPointAsB2DPoint(rPair, bScale, bReplaceGeoSize));
    return Point(static_cast<tools::Long>(aPoint.getX()), static_cast<tools::Long>(aPoint.getY()));
}

void EnhancedCustomShapeFormulas::GetParameter( double& rRetValue, const EnhancedCustomShapeParameter& rParameter,
                                              const bool bReplaceGeoWidth, const bool bReplaceGeoHeight ) const
{
    rRetValue = 0.0;
    switch ( rParameter.Type )
    {
        case EnhancedCustomShapeParameterType::ADJUSTMENT :
        {
            sal_Int32 nAdjustmentIndex = 0;
            if ( rParameter.Value >>= nAdjustmentIndex )
            {
                rRetValue = GetAdjustValueAsDouble( nAdjustmentIndex );
            }
        }
        break;
        case EnhancedCustomShapeParameterType::EQUATION :
        {
            sal_Int32 nEquationIndex = 0;
            if ( rParameter.Value >>= nEquationIndex )
            {
                rRetValue = GetEquationValueAsDouble( nEquationIndex );
            }
        }
        break;
        case EnhancedCustomShapeParameterType::NORMAL :
        {
            if ( rParameter.Value.getValueTypeClass() == TypeClass_DOUBLE )
            {
                double fValue(0.0);
                if ( rParameter.Value >>= fValue )
                {
                    rRetValue = fValue;
                }
            }
            else
            {
                sal_Int32 nValue = 0;
                if ( rParameter.Value >>= nValue )
                {
                    rRetValue = nValue;
                    if ( bReplaceGeoWidth && ( nValue == m_nCoordWidth ) )
                        rRetValue *= m_fXRatio;
                    else if ( bReplaceGeoHeight && ( nValue == m_nCoordHeight ) )
                        rRetValue *= m_fYRatio;
                }
            }
        }
        break;
        case EnhancedCustomShapeParameterType::LEFT :
        case EnhancedCustomShapeParameterType::TOP :
        {
            rRetValue  = 0.0;
        }
        break;
        case EnhancedCustomShapeParameterType::RIGHT :
        {
            rRetValue = m_nCoordWidth;
        }
        break;
        case EnhancedCustomShapeParameterType::BOTTOM :
        {
            rRetValue = m_nCoordHeight;
        }
        break;
    }
}

tools::Rectangle EnhancedCustomShapeFormulas::GetTextRect() const
{
    if ( !m_seqTextFrames.hasElements() )
        return m_aLogicRect;
    sal_Int32 nIndex = 0;
    Point aTopLeft( GetPoint( m_seqTextFrames[ nIndex ].TopLeft, !m_bOOXMLShape, true ) );
    Point aBottomRight( GetPoint( m_seqTextFrames[ nIndex ].BottomRight, !m_bOOXMLShape, true ) );
    tools::Rectangle aRect( aTopLeft, aBottomRight );
    if ( m_bFlipH )
    {
        aRect.SetLeft(m_aLogicRect.GetWidth() - 1 - aBottomRight.X());
        aRect.SetRight( m_aLogicRect.GetWidth() - 1 - aTopLeft.X());
    }
    if ( m_bFlipV )
    {
        aRect.SetTop(m_aLogicRect.GetHeight() - 1 - aBottomRight.Y());
        aRect.SetBottom(m_aLogicRect.GetHeight() - 1 - aTopLeft.Y());
    }
    SAL_INFO("svx", aRect.GetWidth() << " x " << aRect.GetHeight());
    if( aRect.GetWidth() <= 1 || aRect.GetHeight() <= 1 )
        return m_aLogicRect;
    aRect.Move( m_aLogicRect.Left(), m_aLogicRect.Top() );
    aRect.Normalize();
    return aRect;
}

void EnhancedCustomShapeFormulas::ForgetTheFormulaResults()
{
    for (EquationResult& rResult : m_vEquationResults)
        rResult.bReady = false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
