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

#include <basegfx/numeric/ftools.hxx>

#include "VCartesianAxis.hxx"
#include "PlottingPositionHelper.hxx"
#include "ShapeFactory.hxx"
#include "CommonConverters.hxx"
#include "macros.hxx"
#include "ViewDefines.hxx"
#include "PropertyMapper.hxx"
#include "NumberFormatterWrapper.hxx"
#include "LabelPositionHelper.hxx"
#include "TrueGuard.hxx"
#include "BaseGFXHelper.hxx"
#include "AxisHelper.hxx"
#include "Tickmarks_Equidistant.hxx"

#include <rtl/math.hxx>
#include <tools/color.hxx>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/text/WritingMode2.hpp>
#include <editeng/unoprnms.hxx>
#include <svx/unoshape.hxx>
#include <svx/unoshtxt.hxx>

#include <algorithm>
#include <boost/scoped_ptr.hpp>

//.............................................................................
namespace chart
{
//.............................................................................
using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;
using namespace ::rtl::math;
using ::com::sun::star::uno::Reference;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

VCartesianAxis::VCartesianAxis( const AxisProperties& rAxisProperties
            , const Reference< util::XNumberFormatsSupplier >& xNumberFormatsSupplier
            , sal_Int32 nDimensionIndex, sal_Int32 nDimensionCount
            , PlottingPositionHelper* pPosHelper )//takes ownership
            : VAxisBase( nDimensionIndex, nDimensionCount, rAxisProperties, xNumberFormatsSupplier )
{
    if( pPosHelper )
        m_pPosHelper = pPosHelper;
    else
        m_pPosHelper = new PlottingPositionHelper();
}

VCartesianAxis::~VCartesianAxis()
{
    delete m_pPosHelper;
    m_pPosHelper = NULL;
}

Reference< drawing::XShape > createSingleLabel(
            const Reference< lang::XMultiServiceFactory>& xShapeFactory
          , const Reference< drawing::XShapes >& xTarget
          , const awt::Point& rAnchorScreenPosition2D
          , const rtl::OUString& rLabel
          , const AxisLabelProperties& rAxisLabelProperties
          , const AxisProperties& rAxisProperties
          , const tNameSequence& rPropNames
          , const tAnySequence& rPropValues
          )
{
    if(rLabel.isEmpty())
        return 0;

    // #i78696# use mathematically correct rotation now
    const double fRotationAnglePi(rAxisLabelProperties.fRotationAngleDegree * (F_PI / -180.0));
    uno::Any aATransformation = ShapeFactory::makeTransformation( rAnchorScreenPosition2D, fRotationAnglePi );
    rtl::OUString aLabel = ShapeFactory::getStackedString( rLabel, rAxisLabelProperties.bStackCharacters );

    Reference< drawing::XShape > xShape2DText = ShapeFactory(xShapeFactory)
                    .createText( xTarget, aLabel, rPropNames, rPropValues, aATransformation );

    LabelPositionHelper::correctPositionForRotation( xShape2DText
        , rAxisProperties.m_aLabelAlignment, rAxisLabelProperties.fRotationAngleDegree, rAxisProperties.m_bComplexCategories );

    return xShape2DText;
}

bool lcl_doesShapeOverlapWithTickmark( const Reference< drawing::XShape >& xShape
                       , double fRotationAngleDegree
                       , const basegfx::B2DVector& rTickScreenPosition
                       , bool bIsHorizontalAxis, bool bIsVerticalAxis )
{
    if(!xShape.is())
        return false;

    ::basegfx::B2IRectangle aShapeRect = BaseGFXHelper::makeRectangle(xShape->getPosition(),ShapeFactory::getSizeAfterRotation( xShape, fRotationAngleDegree ));

    if( bIsVerticalAxis )
    {
        return ( (rTickScreenPosition.getY() >= aShapeRect.getMinY())
            && (rTickScreenPosition.getY() <= aShapeRect.getMaxY()) );
    }
    if( bIsHorizontalAxis )
    {
        return ( (rTickScreenPosition.getX() >= aShapeRect.getMinX())
            && (rTickScreenPosition.getX() <= aShapeRect.getMaxX()) );
    }

    basegfx::B2IVector aPosition(
        static_cast<sal_Int32>( rTickScreenPosition.getX() )
        , static_cast<sal_Int32>( rTickScreenPosition.getY() ) );
    return aShapeRect.isInside(aPosition);
}

bool doesOverlap( const Reference< drawing::XShape >& xShape1
                , const Reference< drawing::XShape >& xShape2
                , double fRotationAngleDegree )
{
    if( !xShape1.is() || !xShape2.is() )
        return false;

    ::basegfx::B2IRectangle aRect1( BaseGFXHelper::makeRectangle(xShape1->getPosition(),ShapeFactory::getSizeAfterRotation( xShape1, fRotationAngleDegree )));
    ::basegfx::B2IRectangle aRect2( BaseGFXHelper::makeRectangle(xShape2->getPosition(),ShapeFactory::getSizeAfterRotation( xShape2, fRotationAngleDegree )));
    return aRect1.overlaps(aRect2);
}

void removeShapesAtWrongRhythm( TickIter& rIter
                              , sal_Int32 nCorrectRhythm
                              , sal_Int32 nMaxTickToCheck
                              , const Reference< drawing::XShapes >& xTarget )
{
    sal_Int32 nTick = 0;
    for( TickInfo* pTickInfo = rIter.firstInfo()
        ; pTickInfo && nTick <= nMaxTickToCheck
        ; pTickInfo = rIter.nextInfo(), nTick++ )
    {
        //remove labels which does not fit into the rhythm
        if( nTick%nCorrectRhythm != 0)
        {
            if(pTickInfo->xTextShape.is())
            {
                xTarget->remove(pTickInfo->xTextShape);
                pTickInfo->xTextShape = NULL;
            }
        }
    }
}

class LabelIterator : public TickIter
{
    //this Iterator iterates over existing text labels

    //if the labels are staggered and bInnerLine is true
    //we iterate only through the labels which are lying more inside the diagram

    //if the labels are staggered and bInnerLine is false
    //we iterate only through the labels which are lying more outside the diagram

    //if the labels are not staggered
    //we iterate through all labels

public:
    LabelIterator( ::std::vector< TickInfo >& rTickInfoVector
            , const AxisLabelStaggering eAxisLabelStaggering
            , bool bInnerLine );

    virtual TickInfo*   firstInfo();
    virtual TickInfo*   nextInfo();

private: //methods
    LabelIterator();

private: //member
    PureTickIter m_aPureTickIter;
    const AxisLabelStaggering   m_eAxisLabelStaggering;
    bool m_bInnerLine;
};

LabelIterator::LabelIterator( ::std::vector< TickInfo >& rTickInfoVector
            , const AxisLabelStaggering eAxisLabelStaggering
            , bool bInnerLine )
            : m_aPureTickIter( rTickInfoVector )
            , m_eAxisLabelStaggering(eAxisLabelStaggering)
            , m_bInnerLine(bInnerLine)
{
}

TickInfo* LabelIterator::firstInfo()
{
    TickInfo* pTickInfo = m_aPureTickIter.firstInfo();
    while( pTickInfo && !pTickInfo->xTextShape.is() )
        pTickInfo = m_aPureTickIter.nextInfo();
    if(!pTickInfo)
        return NULL;
    if( (STAGGER_EVEN==m_eAxisLabelStaggering && m_bInnerLine)
        ||
        (STAGGER_ODD==m_eAxisLabelStaggering && !m_bInnerLine)
        )
    {
        //skip first label
        do
            pTickInfo = m_aPureTickIter.nextInfo();
        while( pTickInfo && !pTickInfo->xTextShape.is() );
    }
    if(!pTickInfo)
        return NULL;
    return pTickInfo;
}

TickInfo* LabelIterator::nextInfo()
{
    TickInfo* pTickInfo = NULL;
    //get next label
    do
        pTickInfo = m_aPureTickIter.nextInfo();
    while( pTickInfo && !pTickInfo->xTextShape.is() );

    if(  STAGGER_EVEN==m_eAxisLabelStaggering
      || STAGGER_ODD==m_eAxisLabelStaggering )
    {
        //skip one label
        do
            pTickInfo = m_aPureTickIter.nextInfo();
        while( pTickInfo && !pTickInfo->xTextShape.is() );
    }
    return pTickInfo;
}

B2DVector lcl_getLabelsDistance( TickIter& rIter, const B2DVector& rDistanceTickToText, double fRotationAngleDegree )
{
    //calculates the height or width of a line of labels
    //thus a following line of labels can be shifted for that distance

    B2DVector aRet(0,0);

    sal_Int32 nDistanceTickToText = static_cast<sal_Int32>( rDistanceTickToText.getLength() );
    if( nDistanceTickToText==0.0)
        return aRet;

    B2DVector aStaggerDirection(rDistanceTickToText);
    aStaggerDirection.normalize();

    sal_Int32 nDistance=0;
    Reference< drawing::XShape >  xShape2DText(NULL);
    for( TickInfo* pTickInfo = rIter.firstInfo()
        ; pTickInfo
        ; pTickInfo = rIter.nextInfo() )
    {
        xShape2DText = pTickInfo->xTextShape;
        if( xShape2DText.is() )
        {
            awt::Size aSize = ShapeFactory::getSizeAfterRotation( xShape2DText, fRotationAngleDegree );
            if(fabs(aStaggerDirection.getX())>fabs(aStaggerDirection.getY()))
                nDistance = ::std::max(nDistance,aSize.Width);
            else
                nDistance = ::std::max(nDistance,aSize.Height);
        }
    }

    aRet = aStaggerDirection*nDistance;

    //add extra distance for vertical distance
    if(fabs(aStaggerDirection.getX())>fabs(aStaggerDirection.getY()))
        aRet += rDistanceTickToText;

    return aRet;
}

void lcl_shiftLables( TickIter& rIter, const B2DVector& rStaggerDistance )
{
    if(rStaggerDistance.getLength()==0.0)
        return;
    Reference< drawing::XShape >  xShape2DText(NULL);
    for( TickInfo* pTickInfo = rIter.firstInfo()
        ; pTickInfo
        ; pTickInfo = rIter.nextInfo() )
    {
        xShape2DText = pTickInfo->xTextShape;
        if( xShape2DText.is() )
        {
            awt::Point aPos  = xShape2DText->getPosition();
            aPos.X += static_cast<sal_Int32>(rStaggerDistance.getX());
            aPos.Y += static_cast<sal_Int32>(rStaggerDistance.getY());
            xShape2DText->setPosition( aPos );
        }
    }
}

bool lcl_hasWordBreak( const Reference< drawing::XShape >& rxShape )
{
    if ( rxShape.is() )
    {
        SvxShape* pShape = SvxShape::getImplementation( rxShape );
        SvxShapeText* pShapeText = dynamic_cast< SvxShapeText* >( pShape );
        if ( pShapeText )
        {
            SvxTextEditSource* pTextEditSource = dynamic_cast< SvxTextEditSource* >( pShapeText->GetEditSource() );
            if ( pTextEditSource )
            {
                pTextEditSource->UpdateOutliner();
                SvxTextForwarder* pTextForwarder = pTextEditSource->GetTextForwarder();
                if ( pTextForwarder )
                {
                    sal_uInt16 nParaCount = pTextForwarder->GetParagraphCount();
                    for ( sal_uInt16 nPara = 0; nPara < nParaCount; ++nPara )
                    {
                        sal_uInt16 nLineCount = pTextForwarder->GetLineCount( nPara );
                        for ( sal_uInt16 nLine = 0; nLine < nLineCount; ++nLine )
                        {
                            sal_uInt16 nLineStart = 0;
                            sal_uInt16 nLineEnd = 0;
                            pTextForwarder->GetLineBoundaries( nLineStart, nLineEnd, nPara, nLine );
                            sal_uInt16 nWordStart = 0;
                            sal_uInt16 nWordEnd = 0;
                            if ( pTextForwarder->GetWordIndices( nPara, nLineStart, nWordStart, nWordEnd ) &&
                                 ( nWordStart != nLineStart ) )
                            {
                                return true;
                            }
                        }
                    }
                }
            }
        }
    }

    return false;
}

class MaxLabelTickIter : public TickIter
{
    //iterate over first two and last two labels and the longest label
public:
    MaxLabelTickIter( ::std::vector< TickInfo >& rTickInfoVector
            , sal_Int32 nLongestLabelIndex );
    virtual ~MaxLabelTickIter();

    virtual TickInfo* firstInfo();
    virtual TickInfo* nextInfo();

private:
    ::std::vector< TickInfo >& m_rTickInfoVector;
    ::std::vector< sal_Int32 > m_aValidIndices;
    sal_Int32 m_nCurrentIndex;
};

MaxLabelTickIter::MaxLabelTickIter( ::std::vector< TickInfo >& rTickInfoVector
            , sal_Int32 nLongestLabelIndex )
            : m_rTickInfoVector(rTickInfoVector)
            , m_nCurrentIndex(0)
{
    sal_Int32 nMaxIndex = m_rTickInfoVector.size()-1;
    if( nLongestLabelIndex<0 || nLongestLabelIndex>=nMaxIndex-1 )
        nLongestLabelIndex = 0;

    if( nMaxIndex>=0 )
        m_aValidIndices.push_back(0);
    if( nMaxIndex>=1 )
        m_aValidIndices.push_back(1);
    if( nLongestLabelIndex>1 )
        m_aValidIndices.push_back(nLongestLabelIndex);
    if( nMaxIndex > 2 )
        m_aValidIndices.push_back(nMaxIndex-1);
    if( nMaxIndex > 1 )
        m_aValidIndices.push_back(nMaxIndex);
}
MaxLabelTickIter::~MaxLabelTickIter()
{
}

TickInfo* MaxLabelTickIter::firstInfo()
{
    m_nCurrentIndex = 0;
    if( m_nCurrentIndex < static_cast<sal_Int32>(m_aValidIndices.size()) )
        return &m_rTickInfoVector[m_aValidIndices[m_nCurrentIndex]];
    return 0;
}

TickInfo* MaxLabelTickIter::nextInfo()
{
    m_nCurrentIndex++;
    if( m_nCurrentIndex>=0 && m_nCurrentIndex<static_cast<sal_Int32>(m_aValidIndices.size()) )
        return &m_rTickInfoVector[m_aValidIndices[m_nCurrentIndex]];
    return 0;
}

bool VCartesianAxis::isBreakOfLabelsAllowed( const AxisLabelProperties& rAxisLabelProperties
                                                     , bool bIsHorizontalAxis )
{
    if( m_aTextLabels.getLength() > 100 )
        return false;
    if( !rAxisLabelProperties.bLineBreakAllowed )
        return false;
    if( rAxisLabelProperties.bStackCharacters )
        return false;
    //no break for value axis
    if( !m_bUseTextLabels )
        return false;
    if( !::rtl::math::approxEqual( rAxisLabelProperties.fRotationAngleDegree, 0.0 ) )
        return false;
    //break only for horizontal axis
    return bIsHorizontalAxis;
}

bool VCartesianAxis::isAutoStaggeringOfLabelsAllowed( const AxisLabelProperties& rAxisLabelProperties
                                                     , bool bIsHorizontalAxis, bool bIsVerticalAxis )
{
    if( rAxisLabelProperties.eStaggering != STAGGER_AUTO )
        return false;
    if( rAxisLabelProperties.bOverlapAllowed )
        return false;
    if( rAxisLabelProperties.bLineBreakAllowed ) //auto line break or auto staggering, doing both automatisms they may conflict...
        return false;
    if( !::rtl::math::approxEqual( rAxisLabelProperties.fRotationAngleDegree, 0.0 ) )
        return false;
    //automatic staggering only for horizontal axis with horizontal text
    //or vertical axis with vertical text
    if( bIsHorizontalAxis )
        return !rAxisLabelProperties.bStackCharacters;
    if( bIsVerticalAxis )
        return rAxisLabelProperties.bStackCharacters;
    return false;
}

struct ComplexCategoryPlacement
{
    rtl::OUString Text;
    sal_Int32 Count;
    double TickValue;

    ComplexCategoryPlacement( const rtl::OUString& rText, sal_Int32 nCount, double fTickValue )
        : Text(rText), Count(nCount), TickValue(fTickValue)
    {}
};

void VCartesianAxis::createAllTickInfosFromComplexCategories( ::std::vector< ::std::vector< TickInfo > >& rAllTickInfos, bool bShiftedPosition )
{
    //no minor tickmarks will be generated!
    //order is: inner labels first , outer labels last (that is different to all other TickIter cases)
    if(!bShiftedPosition)
    {
        rAllTickInfos.clear();
        sal_Int32 nLevel=0;
        sal_Int32 nLevelCount = m_aAxisProperties.m_pExplicitCategoriesProvider->getCategoryLevelCount();
        for( ; nLevel<nLevelCount; nLevel++ )
        {
            ::std::vector< TickInfo > aTickInfoVector;
            const std::vector<ComplexCategory>* pComplexCategories =
                m_aAxisProperties.m_pExplicitCategoriesProvider->getCategoriesByLevel(nLevel);

            if (!pComplexCategories)
                continue;

            sal_Int32 nCatIndex = 0;
            std::vector<ComplexCategory>::const_iterator aIt = pComplexCategories->begin();
            std::vector<ComplexCategory>::const_iterator aEnd = pComplexCategories->end();

            for(;aIt!=aEnd;++aIt)
            {
                TickInfo aTickInfo(0);
                ComplexCategory aCat(*aIt);
                sal_Int32 nCount = aCat.Count;
                if( nCatIndex + 1.0 + nCount >= m_aScale.Maximum )
                {
                    nCount = static_cast<sal_Int32>(m_aScale.Maximum - 1.0 - nCatIndex);
                    if( nCount <= 0 )
                        nCount = 1;
                }
                aTickInfo.fScaledTickValue = nCatIndex + 1.0 + nCount/2.0;
                aTickInfo.nFactorForLimitedTextWidth = nCount;
                aTickInfo.aText = aCat.Text;
                aTickInfoVector.push_back(aTickInfo);
                nCatIndex += nCount;
                if( nCatIndex + 1.0 >= m_aScale.Maximum )
                    break;
            }
            rAllTickInfos.push_back(aTickInfoVector);
        }
    }
    else //bShiftedPosition==false
    {
        rAllTickInfos.clear();
        sal_Int32 nLevel=0;
        sal_Int32 nLevelCount = m_aAxisProperties.m_pExplicitCategoriesProvider->getCategoryLevelCount();
        for( ; nLevel<nLevelCount; nLevel++ )
        {
            ::std::vector< TickInfo > aTickInfoVector;
            const std::vector<ComplexCategory>* pComplexCategories =
                m_aAxisProperties.m_pExplicitCategoriesProvider->getCategoriesByLevel(nLevel);
            sal_Int32 nCatIndex = 0;
            if (pComplexCategories)
            {
                std::vector<ComplexCategory>::const_iterator aIt = pComplexCategories->begin();
                std::vector<ComplexCategory>::const_iterator aEnd = pComplexCategories->end();
                for(;aIt!=aEnd;++aIt)
                {
                    TickInfo aTickInfo(0);
                    ComplexCategory aCat(*aIt);
                    aTickInfo.fScaledTickValue = nCatIndex + 1.0;
                    aTickInfoVector.push_back(aTickInfo);
                    nCatIndex += aCat.Count;
                    if( nCatIndex + 1.0 > m_aScale.Maximum )
                        break;
                }
            }

            //fill up with single ticks until maximum scale
            while( nCatIndex + 1.0 < m_aScale.Maximum )
            {
                TickInfo aTickInfo(0);
                aTickInfo.fScaledTickValue = nCatIndex + 1.0;
                aTickInfoVector.push_back(aTickInfo);
                nCatIndex ++;
                if( nLevel>0 )
                    break;
            }
            //add an additional tick at the end
            {
                TickInfo aTickInfo(0);
                aTickInfo.fScaledTickValue = m_aScale.Maximum;
                aTickInfoVector.push_back(aTickInfo);
            }
            rAllTickInfos.push_back(aTickInfoVector);
        }
    }
}

void VCartesianAxis::createAllTickInfos( ::std::vector< ::std::vector< TickInfo > >& rAllTickInfos )
{
    if( isComplexCategoryAxis() )
        createAllTickInfosFromComplexCategories( rAllTickInfos, false );
    else
        VAxisBase::createAllTickInfos(rAllTickInfos);
}

TickIter* VCartesianAxis::createLabelTickIterator( sal_Int32 nTextLevel )
{
    if( nTextLevel>=0 && nTextLevel < static_cast< sal_Int32 >(m_aAllTickInfos.size()) )
        return new PureTickIter( m_aAllTickInfos[nTextLevel] );
    return NULL;
}

TickIter* VCartesianAxis::createMaximumLabelTickIterator( sal_Int32 nTextLevel )
{
    if( isComplexCategoryAxis() || isDateAxis() )
    {
        return createLabelTickIterator( nTextLevel ); //mmmm maybe todo: create less than all texts here
    }
    else
    {
        if(nTextLevel==0)
        {
            if( !m_aAllTickInfos.empty() )
            {
                sal_Int32 nLongestLabelIndex = m_bUseTextLabels ? this->getIndexOfLongestLabel( m_aTextLabels ) : 0;
                return new MaxLabelTickIter( m_aAllTickInfos[0], nLongestLabelIndex );
            }
        }
    }
    return NULL;
}

sal_Int32 VCartesianAxis::getTextLevelCount() const
{
    sal_Int32 nTextLevelCount = 1;
    if( isComplexCategoryAxis() )
        nTextLevelCount = m_aAxisProperties.m_pExplicitCategoriesProvider->getCategoryLevelCount();
    return nTextLevelCount;
}

bool VCartesianAxis::createTextShapes(
                       const Reference< drawing::XShapes >& xTarget
                     , TickIter& rTickIter
                     , AxisLabelProperties& rAxisLabelProperties
                     , TickFactory_2D* pTickFactory
                     , sal_Int32 nScreenDistanceBetweenTicks )
{
    //returns true if the text shapes have been created successfully
    //otherwise false - in this case the AxisLabelProperties have changed
    //and contain new instructions for the next try for text shape creation

    Reference< XScaling > xInverseScaling( NULL );
    if( m_aScale.Scaling.is() )
        xInverseScaling = m_aScale.Scaling->getInverseScaling();

    FixedNumberFormatter aFixedNumberFormatter(
                m_xNumberFormatsSupplier, rAxisLabelProperties.nNumberFormatKey );

    const bool bIsHorizontalAxis = pTickFactory->isHorizontalAxis();
    const bool bIsVerticalAxis = pTickFactory->isVerticalAxis();
    bool bIsStaggered = rAxisLabelProperties.getIsStaggered();
    B2DVector aTextToTickDistance( pTickFactory->getDistanceAxisTickToText( m_aAxisProperties, true ) );
    sal_Int32 nLimitedSpaceForText = -1;
    if( isBreakOfLabelsAllowed( rAxisLabelProperties, bIsHorizontalAxis ) )
    {
        nLimitedSpaceForText = nScreenDistanceBetweenTicks;
        if( bIsStaggered )
            nLimitedSpaceForText *= 2;

        if( nLimitedSpaceForText > 0 )
        { //reduce space for a small amount to have a visible distance between the labels:
            sal_Int32 nReduce = (nLimitedSpaceForText*5)/100;
            if(!nReduce)
                nReduce = 1;
            nLimitedSpaceForText -= nReduce;
        }
    }

    uno::Sequence< rtl::OUString >* pCategories = 0;
    if( m_bUseTextLabels && !m_aAxisProperties.m_bComplexCategories )
        pCategories = &m_aTextLabels;

    TickInfo* pPreviousVisibleTickInfo = NULL;
    TickInfo* pPREPreviousVisibleTickInfo = NULL;
    TickInfo* pLastVisibleNeighbourTickInfo = NULL;

    //------------------------------------------------
    //prepare properties for multipropertyset-interface of shape
    tNameSequence aPropNames;
    tAnySequence aPropValues;

    bool bLimitedHeight = fabs(aTextToTickDistance.getX()) > fabs(aTextToTickDistance.getY());
    Reference< beans::XPropertySet > xProps( m_aAxisProperties.m_xAxisModel, uno::UNO_QUERY );
    PropertyMapper::getTextLabelMultiPropertyLists( xProps, aPropNames, aPropValues, false
        , nLimitedSpaceForText, bLimitedHeight );
    LabelPositionHelper::doDynamicFontResize( aPropValues, aPropNames, xProps
        , m_aAxisLabelProperties.m_aFontReferenceSize );
    LabelPositionHelper::changeTextAdjustment( aPropValues, aPropNames, m_aAxisProperties.m_aLabelAlignment );

    uno::Any* pColorAny = PropertyMapper::getValuePointer(aPropValues,aPropNames,C2U("CharColor"));
    sal_Int32 nColor = Color( COL_AUTO ).GetColor();
    if(pColorAny)
        *pColorAny >>= nColor;

    uno::Any* pLimitedSpaceAny = PropertyMapper::getValuePointerForLimitedSpace(aPropValues,aPropNames,bLimitedHeight);
    //------------------------------------------------

    sal_Int32 nTick = 0;
    for( TickInfo* pTickInfo = rTickIter.firstInfo()
        ; pTickInfo
        ; pTickInfo = rTickIter.nextInfo(), nTick++ )
    {
        pLastVisibleNeighbourTickInfo = bIsStaggered ?
                    pPREPreviousVisibleTickInfo : pPreviousVisibleTickInfo;

        //don't create labels which does not fit into the rhythm
        if( nTick%rAxisLabelProperties.nRhythm != 0)
            continue;

        //don't create labels for invisible ticks
        if( !pTickInfo->bPaintIt )
            continue;

        //if NO OVERLAP -> don't create labels where the tick overlaps
        //with the text of the last neighbour tickmark
        if( pLastVisibleNeighbourTickInfo && !rAxisLabelProperties.bOverlapAllowed )
        {
            if( lcl_doesShapeOverlapWithTickmark( pLastVisibleNeighbourTickInfo->xTextShape
                       , rAxisLabelProperties.fRotationAngleDegree
                       , pTickInfo->aTickScreenPosition
                       , bIsHorizontalAxis, bIsVerticalAxis ) )
            {
                bool bOverlapAlsoAfterSwitchingOnAutoStaggering = true;
                if( !bIsStaggered && isAutoStaggeringOfLabelsAllowed( rAxisLabelProperties, bIsHorizontalAxis, bIsVerticalAxis ) )
                {
                    bIsStaggered = true;
                    rAxisLabelProperties.eStaggering = STAGGER_EVEN;
                    pLastVisibleNeighbourTickInfo = pPREPreviousVisibleTickInfo;
                    if( !pLastVisibleNeighbourTickInfo ||
                        !lcl_doesShapeOverlapWithTickmark( pLastVisibleNeighbourTickInfo->xTextShape
                                , rAxisLabelProperties.fRotationAngleDegree
                                , pTickInfo->aTickScreenPosition
                                , bIsHorizontalAxis, bIsVerticalAxis ) )
                        bOverlapAlsoAfterSwitchingOnAutoStaggering = false;
                }
                if( bOverlapAlsoAfterSwitchingOnAutoStaggering )
                {
                    if( rAxisLabelProperties.bRhythmIsFix )
                        continue;
                    rAxisLabelProperties.nRhythm++;
                    removeShapesAtWrongRhythm( rTickIter, rAxisLabelProperties.nRhythm, nTick, xTarget );
                    return false;
                }
            }
        }

        //xxxxx pTickInfo->updateUnscaledValue( xInverseScaling );

        bool bHasExtraColor=false;
        sal_Int32 nExtraColor=0;

        rtl::OUString aLabel;
        if(pCategories)
        {
            sal_Int32 nIndex = static_cast< sal_Int32 >(pTickInfo->getUnscaledTickValue()) - 1; //first category (index 0) matches with real number 1.0
            if( nIndex>=0 && nIndex<pCategories->getLength() )
                aLabel = (*pCategories)[nIndex];
        }
        else if( m_aAxisProperties.m_bComplexCategories )
        {
            aLabel = pTickInfo->aText;
        }
        else
            aLabel = aFixedNumberFormatter.getFormattedString( pTickInfo->getUnscaledTickValue(), nExtraColor, bHasExtraColor );

        if(pColorAny)
            *pColorAny = uno::makeAny(bHasExtraColor?nExtraColor:nColor);
        if(pLimitedSpaceAny)
            *pLimitedSpaceAny = uno::makeAny(sal_Int32(nLimitedSpaceForText*pTickInfo->nFactorForLimitedTextWidth));

        B2DVector aTickScreenPos2D( pTickInfo->aTickScreenPosition );
        aTickScreenPos2D += aTextToTickDistance;
        awt::Point aAnchorScreenPosition2D(
            static_cast<sal_Int32>(aTickScreenPos2D.getX())
            ,static_cast<sal_Int32>(aTickScreenPos2D.getY()));

        //create single label
        if(!pTickInfo->xTextShape.is())
            pTickInfo->xTextShape = createSingleLabel( m_xShapeFactory, xTarget
                                    , aAnchorScreenPosition2D, aLabel
                                    , rAxisLabelProperties, m_aAxisProperties
                                    , aPropNames, aPropValues );
        if(!pTickInfo->xTextShape.is())
            continue;

        recordMaximumTextSize( pTickInfo->xTextShape, rAxisLabelProperties.fRotationAngleDegree );

         //better rotate if single words are broken apart
        if( nLimitedSpaceForText>0 && !rAxisLabelProperties.bOverlapAllowed
                && ::rtl::math::approxEqual( rAxisLabelProperties.fRotationAngleDegree, 0.0 )
                && m_aAxisProperties.m_bComplexCategories
                && lcl_hasWordBreak( pTickInfo->xTextShape ) )
        {
            rAxisLabelProperties.fRotationAngleDegree = 90;
            rAxisLabelProperties.bLineBreakAllowed = false;
            m_aAxisLabelProperties.fRotationAngleDegree = rAxisLabelProperties.fRotationAngleDegree;
            removeTextShapesFromTicks();
            return false;
        }

        //if NO OVERLAP -> remove overlapping shapes
        if( pLastVisibleNeighbourTickInfo && !rAxisLabelProperties.bOverlapAllowed )
        {
            if( doesOverlap( pLastVisibleNeighbourTickInfo->xTextShape, pTickInfo->xTextShape, rAxisLabelProperties.fRotationAngleDegree ) )
            {
                bool bOverlapAlsoAfterSwitchingOnAutoStaggering = true;
                if( !bIsStaggered && isAutoStaggeringOfLabelsAllowed( rAxisLabelProperties, bIsHorizontalAxis, bIsVerticalAxis ) )
                {
                    bIsStaggered = true;
                    rAxisLabelProperties.eStaggering = STAGGER_EVEN;
                    pLastVisibleNeighbourTickInfo = pPREPreviousVisibleTickInfo;
                    if( !pLastVisibleNeighbourTickInfo ||
                        !lcl_doesShapeOverlapWithTickmark( pLastVisibleNeighbourTickInfo->xTextShape
                            , rAxisLabelProperties.fRotationAngleDegree
                            , pTickInfo->aTickScreenPosition
                            , bIsHorizontalAxis, bIsVerticalAxis ) )
                        bOverlapAlsoAfterSwitchingOnAutoStaggering = false;
                }
                if( bOverlapAlsoAfterSwitchingOnAutoStaggering )
                {
                    if( rAxisLabelProperties.bRhythmIsFix )
                    {
                        xTarget->remove(pTickInfo->xTextShape);
                        pTickInfo->xTextShape = NULL;
                        continue;
                    }
                    rAxisLabelProperties.nRhythm++;
                    removeShapesAtWrongRhythm( rTickIter, rAxisLabelProperties.nRhythm, nTick, xTarget );
                    return false;
                }
            }
        }

        pPREPreviousVisibleTickInfo = pPreviousVisibleTickInfo;
        pPreviousVisibleTickInfo = pTickInfo;
    }
    return true;
}

drawing::PointSequenceSequence lcl_makePointSequence( B2DVector& rStart, B2DVector& rEnd )
{
    drawing::PointSequenceSequence aPoints(1);
    aPoints[0].realloc(2);
    aPoints[0][0].X = static_cast<sal_Int32>(rStart.getX());
    aPoints[0][0].Y = static_cast<sal_Int32>(rStart.getY());
    aPoints[0][1].X = static_cast<sal_Int32>(rEnd.getX());
    aPoints[0][1].Y = static_cast<sal_Int32>(rEnd.getY());
    return aPoints;
}

double VCartesianAxis::getLogicValueWhereMainLineCrossesOtherAxis() const
{
    double fMin = (m_nDimensionIndex==1) ? m_pPosHelper->getLogicMinX() : m_pPosHelper->getLogicMinY();
    double fMax = (m_nDimensionIndex==1) ? m_pPosHelper->getLogicMaxX() : m_pPosHelper->getLogicMaxY();

    double fCrossesOtherAxis;
    if(m_aAxisProperties.m_pfMainLinePositionAtOtherAxis)
        fCrossesOtherAxis = *m_aAxisProperties.m_pfMainLinePositionAtOtherAxis;
    else
    {
        if( ::com::sun::star::chart::ChartAxisPosition_END == m_aAxisProperties.m_eCrossoverType )
            fCrossesOtherAxis = fMax;
        else
            fCrossesOtherAxis = fMin;
    }
    return fCrossesOtherAxis;
}

double VCartesianAxis::getLogicValueWhereLabelLineCrossesOtherAxis() const
{
    double fMin = (m_nDimensionIndex==1) ? m_pPosHelper->getLogicMinX() : m_pPosHelper->getLogicMinY();
    double fMax = (m_nDimensionIndex==1) ? m_pPosHelper->getLogicMaxX() : m_pPosHelper->getLogicMaxY();

    double fCrossesOtherAxis;
    if( ::com::sun::star::chart::ChartAxisLabelPosition_OUTSIDE_START == m_aAxisProperties.m_eLabelPos )
        fCrossesOtherAxis = fMin;
    else if( ::com::sun::star::chart::ChartAxisLabelPosition_OUTSIDE_END == m_aAxisProperties.m_eLabelPos )
        fCrossesOtherAxis = fMax;
    else
        fCrossesOtherAxis = getLogicValueWhereMainLineCrossesOtherAxis();
    return fCrossesOtherAxis;
}

bool VCartesianAxis::getLogicValueWhereExtraLineCrossesOtherAxis( double& fCrossesOtherAxis ) const
{
    if( !m_aAxisProperties.m_pfExrtaLinePositionAtOtherAxis )
        return false;
    double fMin = (m_nDimensionIndex==1) ? m_pPosHelper->getLogicMinX() : m_pPosHelper->getLogicMinY();
    double fMax = (m_nDimensionIndex==1) ? m_pPosHelper->getLogicMaxX() : m_pPosHelper->getLogicMaxY();
    if( *m_aAxisProperties.m_pfExrtaLinePositionAtOtherAxis <= fMin
        || *m_aAxisProperties.m_pfExrtaLinePositionAtOtherAxis >= fMax )
        return false;
    fCrossesOtherAxis = *m_aAxisProperties.m_pfExrtaLinePositionAtOtherAxis;
    return true;
}

B2DVector VCartesianAxis::getScreenPosition( double fLogicX, double fLogicY, double fLogicZ ) const
{
    B2DVector aRet(0,0);

    if( m_pPosHelper )
    {
        drawing::Position3D aScenePos = m_pPosHelper->transformLogicToScene( fLogicX, fLogicY, fLogicZ, true );
        if(3==m_nDimension)
        {
            if( m_xLogicTarget.is() && m_pPosHelper && m_pShapeFactory )
            {
                tPropertyNameMap aDummyPropertyNameMap;
                Reference< drawing::XShape > xShape3DAnchor = m_pShapeFactory->createCube( m_xLogicTarget
                        , aScenePos,drawing::Direction3D(1,1,1), 0, 0, aDummyPropertyNameMap);
                awt::Point a2DPos = xShape3DAnchor->getPosition(); //get 2D position from xShape3DAnchor
                m_xLogicTarget->remove(xShape3DAnchor);
                aRet.setX( a2DPos.X );
                aRet.setY( a2DPos.Y );
            }
            else
            {
                OSL_FAIL("cannot calculate scrren position in VCartesianAxis::getScreenPosition");
            }
        }
        else
        {
            aRet.setX( aScenePos.PositionX );
            aRet.setY( aScenePos.PositionY );
        }
    }

    return aRet;
}

VCartesianAxis::ScreenPosAndLogicPos VCartesianAxis::getScreenPosAndLogicPos( double fLogicX_, double fLogicY_, double fLogicZ_ ) const
{
    ScreenPosAndLogicPos aRet;
    aRet.fLogicX = fLogicX_;
    aRet.fLogicY = fLogicY_;
    aRet.fLogicZ = fLogicZ_;
    aRet.aScreenPos = getScreenPosition( fLogicX_, fLogicY_, fLogicZ_ );
    return aRet;
}

typedef ::std::vector< VCartesianAxis::ScreenPosAndLogicPos > tScreenPosAndLogicPosList;
struct lcl_LessXPos : ::std::binary_function< VCartesianAxis::ScreenPosAndLogicPos, VCartesianAxis::ScreenPosAndLogicPos, bool >
{
    inline bool operator() ( const VCartesianAxis::ScreenPosAndLogicPos& rPos1, const VCartesianAxis::ScreenPosAndLogicPos& rPos2 )
    {
        return ( rPos1.aScreenPos.getX() < rPos2.aScreenPos.getX() );
    }
};

struct lcl_GreaterYPos : ::std::binary_function< VCartesianAxis::ScreenPosAndLogicPos, VCartesianAxis::ScreenPosAndLogicPos, bool >
{
    inline bool operator() ( const VCartesianAxis::ScreenPosAndLogicPos& rPos1, const VCartesianAxis::ScreenPosAndLogicPos& rPos2 )
    {
        return ( rPos1.aScreenPos.getY() > rPos2.aScreenPos.getY() );
    }
};

void VCartesianAxis::get2DAxisMainLine( B2DVector& rStart, B2DVector& rEnd, double fCrossesOtherAxis )
{
    //m_aAxisProperties might get updated and changed here because
    //    the label alignmant and inner direction sign depends exactly of the choice of the axis line position which is made here in this method

    double fMinX = m_pPosHelper->getLogicMinX();
    double fMinY = m_pPosHelper->getLogicMinY();
    double fMinZ = m_pPosHelper->getLogicMinZ();
    double fMaxX = m_pPosHelper->getLogicMaxX();
    double fMaxY = m_pPosHelper->getLogicMaxY();
    double fMaxZ = m_pPosHelper->getLogicMaxZ();

    double fXStart = fMinX;
    double fYStart = fMinY;
    double fZStart = fMinZ;
    double fXEnd = fXStart;
    double fYEnd = fYStart;
    double fZEnd = fZStart;

    double fXOnXPlane = fMinX;
    double fXOther = fMaxX;
    int nDifferentValue = !m_pPosHelper->isMathematicalOrientationX() ? -1 : 1;
    if( !m_pPosHelper->isSwapXAndY() )
        nDifferentValue *= (CuboidPlanePosition_Left != m_eLeftWallPos) ? -1 : 1;
    else
        nDifferentValue *= (CuboidPlanePosition_Bottom != m_eBottomPos) ? -1 : 1;
    if( nDifferentValue<0 )
    {
        fXOnXPlane = fMaxX;
        fXOther = fMinX;
    }

    double fYOnYPlane = fMinY;
    double fYOther = fMaxY;
    nDifferentValue = !m_pPosHelper->isMathematicalOrientationY() ? -1 : 1;
    if( !m_pPosHelper->isSwapXAndY() )
        nDifferentValue *= (CuboidPlanePosition_Bottom != m_eBottomPos) ? -1 : 1;
    else
        nDifferentValue *= (CuboidPlanePosition_Left != m_eLeftWallPos) ? -1 : 1;
    if( nDifferentValue<0 )
    {
        fYOnYPlane = fMaxY;
        fYOther = fMinY;
    }

    double fZOnZPlane = fMaxZ;
    double fZOther = fMinZ;
    nDifferentValue = !m_pPosHelper->isMathematicalOrientationZ() ? -1 : 1;
    nDifferentValue *= (CuboidPlanePosition_Back != m_eBackWallPos) ? -1 : 1;
    if( nDifferentValue<0 )
    {
        fZOnZPlane = fMinZ;
        fZOther = fMaxZ;
    }

    if( 0==m_nDimensionIndex ) //x-axis
    {
        if( fCrossesOtherAxis < fMinY )
            fCrossesOtherAxis = fMinY;
        else if( fCrossesOtherAxis > fMaxY )
            fCrossesOtherAxis = fMaxY;

        fYStart = fYEnd = fCrossesOtherAxis;
        fXEnd=m_pPosHelper->getLogicMaxX();

        if(3==m_nDimension)
        {
            if( AxisHelper::isAxisPositioningEnabled() )
            {
                if( ::rtl::math::approxEqual( fYOther, fYStart) )
                    fZStart = fZEnd = fZOnZPlane;
                else
                    fZStart = fZEnd = fZOther;
            }
            else
            {
                rStart = getScreenPosition( fXStart, fYStart, fZStart );
                rEnd = getScreenPosition( fXEnd, fYEnd, fZEnd );

                double fDeltaX = rEnd.getX() - rStart.getX();
                double fDeltaY = rEnd.getY() - rStart.getY();

                //only those points are candidates which are lying on exactly one wall as these are outer edges
                tScreenPosAndLogicPosList aPosList;
                aPosList.push_back( getScreenPosAndLogicPos( fMinX, fYOnYPlane, fZOther ) );
                aPosList.push_back( getScreenPosAndLogicPos( fMinX, fYOther, fZOnZPlane ) );

                if( fabs(fDeltaY) > fabs(fDeltaX)  )
                {
                    m_aAxisProperties.m_aLabelAlignment = LABEL_ALIGN_LEFT;
                    //choose most left positions
                    ::std::sort( aPosList.begin(), aPosList.end(), lcl_LessXPos() );
                    m_aAxisProperties.m_fLabelDirectionSign = fDeltaY<0 ? -1 : 1;
                }
                else
                {
                    m_aAxisProperties.m_aLabelAlignment = LABEL_ALIGN_BOTTOM;
                    //choose most bottom positions
                    ::std::sort( aPosList.begin(), aPosList.end(), lcl_GreaterYPos() );
                    m_aAxisProperties.m_fLabelDirectionSign = fDeltaX<0 ? -1 : 1;
                }
                ScreenPosAndLogicPos aBestPos( aPosList[0] );
                fYStart = fYEnd = aBestPos.fLogicY;
                fZStart = fZEnd = aBestPos.fLogicZ;
                if( !m_pPosHelper->isMathematicalOrientationX() )
                    m_aAxisProperties.m_fLabelDirectionSign *= -1;
            }
        }//end 3D x axis
    }
    else if( 1==m_nDimensionIndex ) //y-axis
    {
        if( fCrossesOtherAxis < fMinX )
            fCrossesOtherAxis = fMinX;
        else if( fCrossesOtherAxis > fMaxX )
            fCrossesOtherAxis = fMaxX;

        fXStart = fXEnd = fCrossesOtherAxis;
        fYEnd=m_pPosHelper->getLogicMaxY();

        if(3==m_nDimension)
        {
            if( AxisHelper::isAxisPositioningEnabled() )
            {
                if( ::rtl::math::approxEqual( fXOther, fXStart) )
                    fZStart = fZEnd = fZOnZPlane;
                else
                    fZStart = fZEnd = fZOther;
            }
            else
            {
                rStart = getScreenPosition( fXStart, fYStart, fZStart );
                rEnd = getScreenPosition( fXEnd, fYEnd, fZEnd );

                double fDeltaX = rEnd.getX() - rStart.getX();
                double fDeltaY = rEnd.getY() - rStart.getY();

                //only those points are candidates which are lying on exactly one wall as these are outer edges
                tScreenPosAndLogicPosList aPosList;
                aPosList.push_back( getScreenPosAndLogicPos( fXOnXPlane, fMinY, fZOther ) );
                aPosList.push_back( getScreenPosAndLogicPos( fXOther, fMinY, fZOnZPlane ) );

                if( fabs(fDeltaY) > fabs(fDeltaX)  )
                {
                    m_aAxisProperties.m_aLabelAlignment = LABEL_ALIGN_LEFT;
                    //choose most left positions
                    ::std::sort( aPosList.begin(), aPosList.end(), lcl_LessXPos() );
                    m_aAxisProperties.m_fLabelDirectionSign = fDeltaY<0 ? -1 : 1;
                }
                else
                {
                    m_aAxisProperties.m_aLabelAlignment = LABEL_ALIGN_BOTTOM;
                    //choose most bottom positions
                    ::std::sort( aPosList.begin(), aPosList.end(), lcl_GreaterYPos() );
                    m_aAxisProperties.m_fLabelDirectionSign = fDeltaX<0 ? -1 : 1;
                }
                ScreenPosAndLogicPos aBestPos( aPosList[0] );
                fXStart = fXEnd = aBestPos.fLogicX;
                fZStart = fZEnd = aBestPos.fLogicZ;
                if( !m_pPosHelper->isMathematicalOrientationY() )
                    m_aAxisProperties.m_fLabelDirectionSign *= -1;
            }
        }//end 3D y axis
    }
    else //z-axis
    {
        fZEnd = m_pPosHelper->getLogicMaxZ();
        if( AxisHelper::isAxisPositioningEnabled() )
        {
            if( !m_aAxisProperties.m_bSwapXAndY )
            {
                if( fCrossesOtherAxis < fMinY )
                    fCrossesOtherAxis = fMinY;
                else if( fCrossesOtherAxis > fMaxY )
                    fCrossesOtherAxis = fMaxY;
                fYStart = fYEnd = fCrossesOtherAxis;

                if( ::rtl::math::approxEqual( fYOther, fYStart) )
                    fXStart = fXEnd = fXOnXPlane;
                else
                    fXStart = fXEnd = fXOther;
            }
            else
            {
                if( fCrossesOtherAxis < fMinX )
                    fCrossesOtherAxis = fMinX;
                else if( fCrossesOtherAxis > fMaxX )
                    fCrossesOtherAxis = fMaxX;
                fXStart = fXEnd = fCrossesOtherAxis;

                if( ::rtl::math::approxEqual( fXOther, fXStart) )
                    fYStart = fYEnd = fYOnYPlane;
                else
                    fYStart = fYEnd = fYOther;
            }
        }
        else
        {
            if( !m_pPosHelper->isSwapXAndY() )
            {
                fXStart = fXEnd = m_pPosHelper->isMathematicalOrientationX() ? m_pPosHelper->getLogicMaxX() : m_pPosHelper->getLogicMinX();
                fYStart = fYEnd = m_pPosHelper->isMathematicalOrientationY() ? m_pPosHelper->getLogicMinY() : m_pPosHelper->getLogicMaxY();
            }
            else
            {
                fXStart = fXEnd = m_pPosHelper->isMathematicalOrientationX() ? m_pPosHelper->getLogicMinX() : m_pPosHelper->getLogicMaxX();
                fYStart = fYEnd = m_pPosHelper->isMathematicalOrientationY() ? m_pPosHelper->getLogicMaxY() : m_pPosHelper->getLogicMinY();
            }

            if(3==m_nDimension)
            {
                rStart = getScreenPosition( fXStart, fYStart, fZStart );
                rEnd = getScreenPosition( fXEnd, fYEnd, fZEnd );

                double fDeltaX = rEnd.getX() - rStart.getX();

                //only those points are candidates which are lying on exactly one wall as these are outer edges
                tScreenPosAndLogicPosList aPosList;
                aPosList.push_back( getScreenPosAndLogicPos( fXOther, fYOnYPlane, fMinZ ) );
                aPosList.push_back( getScreenPosAndLogicPos( fXOnXPlane, fYOther, fMinZ ) );

                ::std::sort( aPosList.begin(), aPosList.end(), lcl_GreaterYPos() );
                ScreenPosAndLogicPos aBestPos( aPosList[0] );
                ScreenPosAndLogicPos aNotSoGoodPos( aPosList[1] );

                //choose most bottom positions
                if( !::rtl::math::approxEqual( fDeltaX, 0.0 ) ) // prefere left-right algnments
                {
                    if( aBestPos.aScreenPos.getX() > aNotSoGoodPos.aScreenPos.getX() )
                        m_aAxisProperties.m_aLabelAlignment = LABEL_ALIGN_RIGHT;
                    else
                         m_aAxisProperties.m_aLabelAlignment = LABEL_ALIGN_LEFT;
                }
                else
                {
                    if( aBestPos.aScreenPos.getY() > aNotSoGoodPos.aScreenPos.getY() )
                        m_aAxisProperties.m_aLabelAlignment = LABEL_ALIGN_BOTTOM;
                    else
                         m_aAxisProperties.m_aLabelAlignment = LABEL_ALIGN_TOP;
                }

                m_aAxisProperties.m_fLabelDirectionSign = fDeltaX<0 ? -1 : 1;
                if( !m_pPosHelper->isMathematicalOrientationZ() )
                    m_aAxisProperties.m_fLabelDirectionSign *= -1;

                fXStart = fXEnd = aBestPos.fLogicX;
                fYStart = fYEnd = aBestPos.fLogicY;
            }
        }//end 3D z axis
    }

    rStart = getScreenPosition( fXStart, fYStart, fZStart );
    rEnd = getScreenPosition( fXEnd, fYEnd, fZEnd );

    if(3==m_nDimension && !AxisHelper::isAxisPositioningEnabled() )
        m_aAxisProperties.m_fInnerDirectionSign = m_aAxisProperties.m_fLabelDirectionSign;//to behave like before

    if(3==m_nDimension && AxisHelper::isAxisPositioningEnabled() )
    {
        double fDeltaX = rEnd.getX() - rStart.getX();
        double fDeltaY = rEnd.getY() - rStart.getY();

        if( 2==m_nDimensionIndex )
        {
            if( m_eLeftWallPos != CuboidPlanePosition_Left )
            {
                m_aAxisProperties.m_fLabelDirectionSign *= -1.0;
                m_aAxisProperties.m_fInnerDirectionSign *= -1.0;
            }

            m_aAxisProperties.m_aLabelAlignment =
                ( m_aAxisProperties.m_fLabelDirectionSign<0 ) ?
                    LABEL_ALIGN_LEFT :  LABEL_ALIGN_RIGHT;

            if( ( fDeltaY<0 && m_aScale.Orientation == AxisOrientation_REVERSE ) ||
                ( fDeltaY>0 && m_aScale.Orientation == AxisOrientation_MATHEMATICAL ) )
                m_aAxisProperties.m_aLabelAlignment =
                    ( m_aAxisProperties.m_aLabelAlignment==LABEL_ALIGN_RIGHT ) ?
                        LABEL_ALIGN_LEFT :  LABEL_ALIGN_RIGHT;
        }
        else if( fabs(fDeltaY) > fabs(fDeltaX) )
        {
            if( m_eBackWallPos != CuboidPlanePosition_Back )
            {
                m_aAxisProperties.m_fLabelDirectionSign *= -1.0;
                m_aAxisProperties.m_fInnerDirectionSign *= -1.0;
            }

            m_aAxisProperties.m_aLabelAlignment =
                ( m_aAxisProperties.m_fLabelDirectionSign<0 ) ?
                    LABEL_ALIGN_LEFT :  LABEL_ALIGN_RIGHT;

            if( ( fDeltaY<0 && m_aScale.Orientation == AxisOrientation_REVERSE ) ||
                ( fDeltaY>0 && m_aScale.Orientation == AxisOrientation_MATHEMATICAL ) )
                m_aAxisProperties.m_aLabelAlignment =
                    ( m_aAxisProperties.m_aLabelAlignment==LABEL_ALIGN_RIGHT ) ?
                        LABEL_ALIGN_LEFT :  LABEL_ALIGN_RIGHT;
        }
        else
        {
            if( m_eBackWallPos != CuboidPlanePosition_Back )
            {
                m_aAxisProperties.m_fLabelDirectionSign *= -1.0;
                m_aAxisProperties.m_fInnerDirectionSign *= -1.0;
            }

            m_aAxisProperties.m_aLabelAlignment =
                ( m_aAxisProperties.m_fLabelDirectionSign<0 ) ?
                    LABEL_ALIGN_TOP : LABEL_ALIGN_BOTTOM;

            if( ( fDeltaX>0 && m_aScale.Orientation == AxisOrientation_REVERSE ) ||
                ( fDeltaX<0 && m_aScale.Orientation == AxisOrientation_MATHEMATICAL ) )
                m_aAxisProperties.m_aLabelAlignment =
                    ( m_aAxisProperties.m_aLabelAlignment==LABEL_ALIGN_TOP ) ?
                        LABEL_ALIGN_BOTTOM : LABEL_ALIGN_TOP;
        }
    }
}

TickFactory* VCartesianAxis::createTickFactory()
{
    return createTickFactory2D();
}

TickFactory_2D* VCartesianAxis::createTickFactory2D()
{
    B2DVector aStart, aEnd;
    this->get2DAxisMainLine( aStart, aEnd, this->getLogicValueWhereMainLineCrossesOtherAxis() );

    B2DVector aLabelLineStart, aLabelLineEnd;
    this->get2DAxisMainLine( aLabelLineStart, aLabelLineEnd, this->getLogicValueWhereLabelLineCrossesOtherAxis() );

    return new TickFactory_2D( m_aScale, m_aIncrement, aStart, aEnd, aLabelLineStart-aStart );
}

void lcl_hideIdenticalScreenValues( TickIter& rTickIter )
{
    TickInfo* pPreviousTickInfo = rTickIter.firstInfo();
    if(!pPreviousTickInfo)
        return;
    pPreviousTickInfo->bPaintIt = true;
    for( TickInfo* pTickInfo = rTickIter.nextInfo(); pTickInfo; pTickInfo = rTickIter.nextInfo())
    {
        pTickInfo->bPaintIt =
            ( static_cast<sal_Int32>(pTickInfo->aTickScreenPosition.getX())
            != static_cast<sal_Int32>(pPreviousTickInfo->aTickScreenPosition.getX()) )
            ||
            ( static_cast<sal_Int32>(pTickInfo->aTickScreenPosition.getY())
            != static_cast<sal_Int32>(pPreviousTickInfo->aTickScreenPosition.getY()) );
        pPreviousTickInfo = pTickInfo;
    }
}

//'hide' tickmarks with identical screen values in aAllTickInfos
void VCartesianAxis::hideIdenticalScreenValues( ::std::vector< ::std::vector< TickInfo > >& rTickInfos ) const
{
    if( isComplexCategoryAxis() || isDateAxis() )
    {
        sal_Int32 nCount = rTickInfos.size();
        for( sal_Int32 nN=0; nN<nCount; nN++ )
        {
            PureTickIter aTickIter( rTickInfos[nN] );
            lcl_hideIdenticalScreenValues( aTickIter );
        }
    }
    else
    {
        EquidistantTickIter aTickIter( rTickInfos, m_aIncrement, 0, -1 );
        lcl_hideIdenticalScreenValues( aTickIter );
    }
}

sal_Int32 VCartesianAxis::estimateMaximumAutoMainIncrementCount()
{
    sal_Int32 nRet = 10;

    if( m_nMaximumTextWidthSoFar==0 && m_nMaximumTextHeightSoFar==0 )
        return nRet;

    B2DVector aStart, aEnd;
    this->get2DAxisMainLine( aStart, aEnd, this->getLogicValueWhereMainLineCrossesOtherAxis() );

    sal_Int32 nMaxHeight = static_cast<sal_Int32>(fabs(aEnd.getY()-aStart.getY()));
    sal_Int32 nMaxWidth = static_cast<sal_Int32>(fabs(aEnd.getX()-aStart.getX()));

    sal_Int32 nTotalAvailable = nMaxHeight;
    sal_Int32 nSingleNeeded = m_nMaximumTextHeightSoFar;

    //for horizontal axis:
    if( (m_nDimensionIndex == 0 && !m_aAxisProperties.m_bSwapXAndY)
        || (m_nDimensionIndex == 1 && m_aAxisProperties.m_bSwapXAndY) )
    {
        nTotalAvailable = nMaxWidth;
        nSingleNeeded = m_nMaximumTextWidthSoFar;
    }

    if( nSingleNeeded>0 )
        nRet = nTotalAvailable/nSingleNeeded;

    return nRet;
}

void VCartesianAxis::doStaggeringOfLabels( const AxisLabelProperties& rAxisLabelProperties, TickFactory_2D* pTickFactory2D )
{
    if( !pTickFactory2D )
        return;

    if( isComplexCategoryAxis() )
    {
        sal_Int32 nTextLevelCount = getTextLevelCount();
        B2DVector aCummulatedLabelsDistance(0,0);
        for( sal_Int32 nTextLevel=0; nTextLevel<nTextLevelCount; nTextLevel++ )
        {
            boost::scoped_ptr<TickIter> apTickIter(createLabelTickIterator(nTextLevel));
            if (apTickIter)
            {
                double fRotationAngleDegree = m_aAxisLabelProperties.fRotationAngleDegree;
                if( nTextLevel>0 )
                {
                    lcl_shiftLables( *apTickIter.get(), aCummulatedLabelsDistance );
                    fRotationAngleDegree = 0.0;
                }
                aCummulatedLabelsDistance += lcl_getLabelsDistance( *apTickIter.get()
                    , pTickFactory2D->getDistanceAxisTickToText( m_aAxisProperties )
                    , fRotationAngleDegree );
            }
        }
    }
    else if( rAxisLabelProperties.getIsStaggered() )
    {
        if( !m_aAllTickInfos.empty() )
        {
            LabelIterator aInnerIter( m_aAllTickInfos[0], rAxisLabelProperties.eStaggering, true );
            LabelIterator aOuterIter( m_aAllTickInfos[0], rAxisLabelProperties.eStaggering, false );

            lcl_shiftLables( aOuterIter
                , lcl_getLabelsDistance( aInnerIter
                    , pTickFactory2D->getDistanceAxisTickToText( m_aAxisProperties ), 0.0 ) );
        }
    }
}

void VCartesianAxis::createLabels()
{
    if( !prepareShapeCreation() )
        return;

    //-----------------------------------------
    //create labels
    if( m_aAxisProperties.m_bDisplayLabels )
    {
        boost::scoped_ptr< TickFactory_2D > apTickFactory2D( this->createTickFactory2D() );
        TickFactory_2D* pTickFactory2D = apTickFactory2D.get();
        if( !pTickFactory2D )
            return;

        //-----------------------------------------
        //get the transformed screen values for all tickmarks in aAllTickInfos
        pTickFactory2D->updateScreenValues( m_aAllTickInfos );
        //-----------------------------------------
        //'hide' tickmarks with identical screen values in aAllTickInfos
        hideIdenticalScreenValues( m_aAllTickInfos );

        removeTextShapesFromTicks();

        //create tick mark text shapes
        sal_Int32 nTextLevelCount = getTextLevelCount();
        sal_Int32 nScreenDistanceBetweenTicks = -1;
        for( sal_Int32 nTextLevel=0; nTextLevel<nTextLevelCount; nTextLevel++ )
        {
            boost::scoped_ptr< TickIter > apTickIter(createLabelTickIterator( nTextLevel ));
            if(apTickIter)
            {
                if(nTextLevel==0)
                {
                    nScreenDistanceBetweenTicks = TickFactory_2D::getTickScreenDistance( *apTickIter.get() );
                    if( nTextLevelCount>1 )
                        nScreenDistanceBetweenTicks*=2; //the above used tick iter does contain also the sub ticks -> thus the given distance is only the half
                }

                AxisLabelProperties aComplexProps(m_aAxisLabelProperties);
                if( m_aAxisProperties.m_bComplexCategories )
                {
                    aComplexProps.bLineBreakAllowed = true;
                    aComplexProps.bOverlapAllowed = !::rtl::math::approxEqual( aComplexProps.fRotationAngleDegree, 0.0 );

                }
                AxisLabelProperties& rAxisLabelProperties =  m_aAxisProperties.m_bComplexCategories ? aComplexProps : m_aAxisLabelProperties;
                while( !createTextShapes( m_xTextTarget, *apTickIter.get(), rAxisLabelProperties, pTickFactory2D, nScreenDistanceBetweenTicks ) )
                {
                };
            }
        }
        doStaggeringOfLabels( m_aAxisLabelProperties, pTickFactory2D );
    }
}

void VCartesianAxis::createMaximumLabels()
{
    TrueGuard aRecordMaximumTextSize(m_bRecordMaximumTextSize);

    if( !prepareShapeCreation() )
        return;

    //-----------------------------------------
    //create labels
    if( m_aAxisProperties.m_bDisplayLabels )
    {
        boost::scoped_ptr< TickFactory_2D > apTickFactory2D( this->createTickFactory2D() );
        TickFactory_2D* pTickFactory2D = apTickFactory2D.get();
        if( !pTickFactory2D )
            return;

        //-----------------------------------------
        //get the transformed screen values for all tickmarks in aAllTickInfos
        pTickFactory2D->updateScreenValues( m_aAllTickInfos );

        //create tick mark text shapes
        //@todo: iterate through all tick depth wich should be labeled

        AxisLabelProperties aAxisLabelProperties( m_aAxisLabelProperties );
        if( isAutoStaggeringOfLabelsAllowed( aAxisLabelProperties, pTickFactory2D->isHorizontalAxis(), pTickFactory2D->isVerticalAxis() ) )
            aAxisLabelProperties.eStaggering = STAGGER_EVEN;
        aAxisLabelProperties.bOverlapAllowed = true;
        aAxisLabelProperties.bLineBreakAllowed = false;
        sal_Int32 nTextLevelCount = getTextLevelCount();
        for( sal_Int32 nTextLevel=0; nTextLevel<nTextLevelCount; nTextLevel++ )
        {
            boost::scoped_ptr< TickIter > apTickIter(createMaximumLabelTickIterator( nTextLevel ));
            if(apTickIter)
            {
                while( !createTextShapes( m_xTextTarget, *apTickIter.get(), aAxisLabelProperties, pTickFactory2D, -1 ) )
                {
                };
            }
        }
        doStaggeringOfLabels( aAxisLabelProperties, pTickFactory2D );
    }
}

void VCartesianAxis::updatePositions()
{
    //-----------------------------------------
    //update positions of labels
    if( m_aAxisProperties.m_bDisplayLabels )
    {
        boost::scoped_ptr< TickFactory_2D > apTickFactory2D( this->createTickFactory2D() );
        TickFactory_2D* pTickFactory2D = apTickFactory2D.get();
        if( !pTickFactory2D )
            return;

        //-----------------------------------------
        //update positions of all existing text shapes
        pTickFactory2D->updateScreenValues( m_aAllTickInfos );

        ::std::vector< ::std::vector< TickInfo > >::iterator aDepthIter = m_aAllTickInfos.begin();
        const ::std::vector< ::std::vector< TickInfo > >::const_iterator aDepthEnd  = m_aAllTickInfos.end();
        for( sal_Int32 nDepth=0; aDepthIter != aDepthEnd; ++aDepthIter, nDepth++ )
        {
            ::std::vector< TickInfo >::iterator aTickIter = aDepthIter->begin();
            const ::std::vector< TickInfo >::const_iterator aTickEnd  = aDepthIter->end();
            for( ; aTickIter != aTickEnd; ++aTickIter )
            {
                TickInfo& rTickInfo = (*aTickIter);
                Reference< drawing::XShape > xShape2DText( rTickInfo.xTextShape );
                if( xShape2DText.is() )
                {
                    B2DVector aTextToTickDistance( pTickFactory2D->getDistanceAxisTickToText( m_aAxisProperties, true ) );
                    B2DVector aTickScreenPos2D( rTickInfo.aTickScreenPosition );
                    aTickScreenPos2D += aTextToTickDistance;
                    awt::Point aAnchorScreenPosition2D(
                        static_cast<sal_Int32>(aTickScreenPos2D.getX())
                        ,static_cast<sal_Int32>(aTickScreenPos2D.getY()));

                    double fRotationAngleDegree = m_aAxisLabelProperties.fRotationAngleDegree;

                    // #i78696# use mathematically correct rotation now
                    const double fRotationAnglePi(fRotationAngleDegree * (F_PI / -180.0));
                    uno::Any aATransformation = ShapeFactory::makeTransformation(aAnchorScreenPosition2D, fRotationAnglePi);

                    //set new position
                    uno::Reference< beans::XPropertySet > xProp( xShape2DText, uno::UNO_QUERY );
                    if( xProp.is() )
                    {
                        try
                        {
                            xProp->setPropertyValue( C2U( "Transformation" ), aATransformation );
                        }
                        catch( const uno::Exception& e )
                        {
                            ASSERT_EXCEPTION( e );
                        }
                    }

                    //correctPositionForRotation
                    LabelPositionHelper::correctPositionForRotation( xShape2DText
                        , m_aAxisProperties.m_aLabelAlignment, fRotationAngleDegree, m_aAxisProperties.m_bComplexCategories );
                }
            }
        }

        doStaggeringOfLabels( m_aAxisLabelProperties, pTickFactory2D );
    }
}

void VCartesianAxis::createTickMarkLineShapes( ::std::vector< TickInfo >& rTickInfos, const TickmarkProperties& rTickmarkProperties, TickFactory_2D& rTickFactory2D, bool bOnlyAtLabels )
{
    sal_Int32 nPointCount = rTickInfos.size();
    drawing::PointSequenceSequence aPoints(2*nPointCount);

    ::std::vector< TickInfo >::const_iterator       aTickIter = rTickInfos.begin();
    const ::std::vector< TickInfo >::const_iterator aTickEnd  = rTickInfos.end();
    sal_Int32 nN = 0;
    for( ; aTickIter != aTickEnd; ++aTickIter )
    {
        if( !(*aTickIter).bPaintIt )
            continue;

        bool bTicksAtLabels = ( m_aAxisProperties.m_eTickmarkPos != ::com::sun::star::chart::ChartAxisMarkPosition_AT_AXIS );
        double fInnerDirectionSign = m_aAxisProperties.m_fInnerDirectionSign;
        if( bTicksAtLabels && m_aAxisProperties.m_eLabelPos == ::com::sun::star::chart::ChartAxisLabelPosition_OUTSIDE_END )
            fInnerDirectionSign *= -1.0;
        bTicksAtLabels = bTicksAtLabels || bOnlyAtLabels;
        //add ticks at labels:
        rTickFactory2D.addPointSequenceForTickLine( aPoints, nN++, (*aTickIter).fScaledTickValue
            , fInnerDirectionSign , rTickmarkProperties, bTicksAtLabels );
        //add ticks at axis (without lables):
        if( !bOnlyAtLabels && m_aAxisProperties.m_eTickmarkPos == ::com::sun::star::chart::ChartAxisMarkPosition_AT_LABELS_AND_AXIS )
            rTickFactory2D.addPointSequenceForTickLine( aPoints, nN++, (*aTickIter).fScaledTickValue
                , m_aAxisProperties.m_fInnerDirectionSign, rTickmarkProperties, !bTicksAtLabels );
    }
    aPoints.realloc(nN);
    m_pShapeFactory->createLine2D( m_xGroupShape_Shapes, aPoints
                                , &rTickmarkProperties.aLineProperties );
}

void VCartesianAxis::createShapes()
{
    if( !prepareShapeCreation() )
        return;

    boost::scoped_ptr< TickFactory_2D > apTickFactory2D( this->createTickFactory2D() );
    TickFactory_2D* pTickFactory2D = apTickFactory2D.get();
    if( !pTickFactory2D )
        return;

    //-----------------------------------------
    //create line shapes
    if(2==m_nDimension)
    {
        //-----------------------------------------
        //create extra long ticks to separate complex categories (create them only there where the labels are)
        if( isComplexCategoryAxis() )
        {
            ::std::vector< ::std::vector< TickInfo > > aComplexTickInfos;
            createAllTickInfosFromComplexCategories( aComplexTickInfos, true );
            pTickFactory2D->updateScreenValues( aComplexTickInfos );
            hideIdenticalScreenValues( aComplexTickInfos );

            ::std::vector<TickmarkProperties> aTickmarkPropertiesList;
            static bool bIncludeSpaceBetweenTickAndText = false;
            sal_Int32 nOffset = static_cast<sal_Int32>(pTickFactory2D->getDistanceAxisTickToText( m_aAxisProperties, false, bIncludeSpaceBetweenTickAndText ).getLength());
            sal_Int32 nTextLevelCount = getTextLevelCount();
            for( sal_Int32 nTextLevel=0; nTextLevel<nTextLevelCount; nTextLevel++ )
            {
                boost::scoped_ptr< TickIter > apTickIter(createLabelTickIterator( nTextLevel ));
                if( apTickIter )
                {
                    double fRotationAngleDegree = m_aAxisLabelProperties.fRotationAngleDegree;
                    B2DVector aLabelsDistance( lcl_getLabelsDistance( *apTickIter.get(), pTickFactory2D->getDistanceAxisTickToText( m_aAxisProperties, false ), fRotationAngleDegree ) );
                    sal_Int32 nCurrentLength = static_cast<sal_Int32>(aLabelsDistance.getLength());
                    aTickmarkPropertiesList.push_back( m_aAxisProperties.makeTickmarkPropertiesForComplexCategories( nOffset + nCurrentLength, 0, nTextLevel ) );
                    nOffset += nCurrentLength;
                }
            }

            sal_Int32 nTickmarkPropertiesCount = aTickmarkPropertiesList.size();
            ::std::vector< ::std::vector< TickInfo > >::iterator aDepthIter             = aComplexTickInfos.begin();
            const ::std::vector< ::std::vector< TickInfo > >::const_iterator aDepthEnd  = aComplexTickInfos.end();
            for( sal_Int32 nDepth=0; aDepthIter != aDepthEnd && nDepth < nTickmarkPropertiesCount; ++aDepthIter, nDepth++ )
            {
                if(nDepth==0 && !m_aAxisProperties.m_nMajorTickmarks)
                    continue;
                createTickMarkLineShapes( *aDepthIter, aTickmarkPropertiesList[nDepth], *pTickFactory2D, true /*bOnlyAtLabels*/ );
            }
        }
        //-----------------------------------------
        //create normal ticks for major and minor intervals
        {
            ::std::vector< ::std::vector< TickInfo > > aUnshiftedTickInfos;
            if( m_aScale.ShiftedCategoryPosition )// if ShiftedCategoryPosition==true the tickmarks in m_aAllTickInfos are shifted
            {
                pTickFactory2D->getAllTicks( aUnshiftedTickInfos );
                pTickFactory2D->updateScreenValues( aUnshiftedTickInfos );
                hideIdenticalScreenValues( aUnshiftedTickInfos );
            }
            ::std::vector< ::std::vector< TickInfo > >& rAllTickInfos = m_aScale.ShiftedCategoryPosition ? aUnshiftedTickInfos : m_aAllTickInfos;

            ::std::vector< ::std::vector< TickInfo > >::iterator aDepthIter             = rAllTickInfos.begin();
            const ::std::vector< ::std::vector< TickInfo > >::const_iterator aDepthEnd  = rAllTickInfos.end();
            if(aDepthIter == aDepthEnd)//no tickmarks at all
                return;

            sal_Int32 nTickmarkPropertiesCount = m_aAxisProperties.m_aTickmarkPropertiesList.size();
            for( sal_Int32 nDepth=0; aDepthIter != aDepthEnd && nDepth < nTickmarkPropertiesCount; ++aDepthIter, nDepth++ )
                createTickMarkLineShapes( *aDepthIter, m_aAxisProperties.m_aTickmarkPropertiesList[nDepth], *pTickFactory2D, false /*bOnlyAtLabels*/ );
        }
        //-----------------------------------------
        //create axis main lines
        //it serves also as the handle shape for the axis selection
        {
            drawing::PointSequenceSequence aPoints(1);
            apTickFactory2D->createPointSequenceForAxisMainLine( aPoints );
            Reference< drawing::XShape > xShape = m_pShapeFactory->createLine2D(
                    m_xGroupShape_Shapes, aPoints
                    , &m_aAxisProperties.m_aLineProperties );
            //because of this name this line will be used for marking the axis
            m_pShapeFactory->setShapeName( xShape, C2U("MarkHandles") );
        }
        //-----------------------------------------
        //create an additional line at NULL
        if( !AxisHelper::isAxisPositioningEnabled() )
        {
            double fExtraLineCrossesOtherAxis;
            if( getLogicValueWhereExtraLineCrossesOtherAxis(fExtraLineCrossesOtherAxis) )
            {
                B2DVector aStart, aEnd;
                this->get2DAxisMainLine( aStart, aEnd, fExtraLineCrossesOtherAxis );
                drawing::PointSequenceSequence aPoints( lcl_makePointSequence(aStart,aEnd) );
                Reference< drawing::XShape > xShape = m_pShapeFactory->createLine2D(
                        m_xGroupShape_Shapes, aPoints, &m_aAxisProperties.m_aLineProperties );
            }
        }
    }

    //createLabels();
}

//.............................................................................
} //namespace chart
//.............................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
