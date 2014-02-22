/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <svgio/svgreader/svgtextpathnode.hxx>
#include <svgio/svgreader/svgstyleattributes.hxx>
#include <svgio/svgreader/svgpathnode.hxx>
#include <svgio/svgreader/svgdocument.hxx>
#include <svgio/svgreader/svgtrefnode.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <drawinglayer/primitive2d/textbreakuphelper.hxx>
#include <drawinglayer/primitive2d/groupprimitive2d.hxx>
#include <basegfx/curve/b2dcubicbezier.hxx>
#include <basegfx/curve/b2dbeziertools.hxx>



namespace svgio
{
    namespace svgreader
    {
        class pathTextBreakupHelper : public drawinglayer::primitive2d::TextBreakupHelper
        {
        private:
            const basegfx::B2DPolygon&      mrPolygon;
            const double                    mfBasegfxPathLength;
            double                          mfPosition;
            const basegfx::B2DPoint&        mrTextStart;

            const sal_uInt32                mnMaxIndex;
            sal_uInt32                      mnIndex;
            basegfx::B2DCubicBezier         maCurrentSegment;
            basegfx::B2DCubicBezierHelper*  mpB2DCubicBezierHelper;
            double                          mfCurrentSegmentLength;
            double                          mfSegmentStartPosition;

        protected:
            
            
            virtual bool allowChange(sal_uInt32 nCount, basegfx::B2DHomMatrix& rNewTransform, sal_uInt32 nIndex, sal_uInt32 nLength);

            void freeB2DCubicBezierHelper();
            basegfx::B2DCubicBezierHelper* getB2DCubicBezierHelper();
            void advanceToPosition(double fNewPosition);

        public:
            pathTextBreakupHelper(
                const drawinglayer::primitive2d::TextSimplePortionPrimitive2D& rSource,
                const basegfx::B2DPolygon& rPolygon,
                const double fBasegfxPathLength,
                double fPosition,
                const basegfx::B2DPoint& rTextStart);
            virtual ~pathTextBreakupHelper();

            
            double getPosition() const { return mfPosition; }
        };

        void pathTextBreakupHelper::freeB2DCubicBezierHelper()
        {
            if(mpB2DCubicBezierHelper)
            {
                delete mpB2DCubicBezierHelper;
                mpB2DCubicBezierHelper = 0;
            }
        }

        basegfx::B2DCubicBezierHelper* pathTextBreakupHelper::getB2DCubicBezierHelper()
        {
            if(!mpB2DCubicBezierHelper && maCurrentSegment.isBezier())
            {
                mpB2DCubicBezierHelper = new basegfx::B2DCubicBezierHelper(maCurrentSegment);
            }

            return mpB2DCubicBezierHelper;
        }

        void pathTextBreakupHelper::advanceToPosition(double fNewPosition)
        {
            while(mfSegmentStartPosition + mfCurrentSegmentLength < fNewPosition && mnIndex < mnMaxIndex)
            {
                mfSegmentStartPosition += mfCurrentSegmentLength;
                mnIndex++;

                if(mnIndex < mnMaxIndex)
                {
                    freeB2DCubicBezierHelper();
                    mrPolygon.getBezierSegment(mnIndex % mrPolygon.count(), maCurrentSegment);
                    maCurrentSegment.testAndSolveTrivialBezier();
                    mfCurrentSegmentLength = getB2DCubicBezierHelper()
                        ? getB2DCubicBezierHelper()->getLength()
                        : maCurrentSegment.getLength();
                }
            }

            mfPosition = fNewPosition;
        }

        pathTextBreakupHelper::pathTextBreakupHelper(
            const drawinglayer::primitive2d::TextSimplePortionPrimitive2D& rSource,
            const basegfx::B2DPolygon& rPolygon,
            const double fBasegfxPathLength,
            double fPosition,
            const basegfx::B2DPoint& rTextStart)
        :   drawinglayer::primitive2d::TextBreakupHelper(rSource),
            mrPolygon(rPolygon),
            mfBasegfxPathLength(fBasegfxPathLength),
            mfPosition(0.0),
            mrTextStart(rTextStart),
            mnMaxIndex(rPolygon.isClosed() ? rPolygon.count() : rPolygon.count() - 1),
            mnIndex(0),
            maCurrentSegment(),
            mpB2DCubicBezierHelper(0),
            mfCurrentSegmentLength(0.0),
            mfSegmentStartPosition(0.0)
        {
            mrPolygon.getBezierSegment(mnIndex % mrPolygon.count(), maCurrentSegment);
            mfCurrentSegmentLength = maCurrentSegment.getLength();

            advanceToPosition(fPosition);
        }

        pathTextBreakupHelper::~pathTextBreakupHelper()
        {
            freeB2DCubicBezierHelper();
        }

        bool pathTextBreakupHelper::allowChange(sal_uInt32 /*nCount*/, basegfx::B2DHomMatrix& rNewTransform, sal_uInt32 nIndex, sal_uInt32 nLength)
        {
            bool bRetval(false);

            if(mfPosition < mfBasegfxPathLength && nLength && mnIndex < mnMaxIndex)
            {
                const double fSnippetWidth(
                    getTextLayouter().getTextWidth(
                        getSource().getText(),
                        nIndex,
                        nLength));

                if(basegfx::fTools::more(fSnippetWidth, 0.0))
                {
                    const OUString aText(getSource().getText());
                    const OUString aTrimmedChars(aText.copy(nIndex, nLength).trim());
                    const double fEndPos(mfPosition + fSnippetWidth);

                    if(!aTrimmedChars.isEmpty() && (mfPosition < mfBasegfxPathLength || fEndPos > 0.0))
                    {
                        const double fHalfSnippetWidth(fSnippetWidth * 0.5);

                        advanceToPosition(mfPosition + fHalfSnippetWidth);

                        
                        bRetval = true;

                        
                        basegfx::B2DPoint aPosition(0.0, 0.0);
                        basegfx::B2DVector aTangent(0.0, 1.0);

                        if(mfPosition < 0.0)
                        {
                            
                            aTangent = maCurrentSegment.getTangent(0.0);
                            aTangent.normalize();
                            aPosition = maCurrentSegment.getStartPoint() + (aTangent * (mfPosition - mfSegmentStartPosition));
                        }
                        else if(mfPosition > mfBasegfxPathLength)
                        {
                            
                            aTangent = maCurrentSegment.getTangent(1.0);
                            aTangent.normalize();
                            aPosition = maCurrentSegment.getEndPoint() + (aTangent * (mfPosition - mfSegmentStartPosition));
                        }
                        else
                        {
                            
                            double fBezierDistance(mfPosition - mfSegmentStartPosition);

                            if(getB2DCubicBezierHelper())
                            {
                                
                                
                                fBezierDistance = getB2DCubicBezierHelper()->distanceToRelative(fBezierDistance);
                            }
                            else
                            {
                                
                                fBezierDistance = fBezierDistance / mfCurrentSegmentLength;
                            }

                            aPosition = maCurrentSegment.interpolatePoint(fBezierDistance);
                            aTangent = maCurrentSegment.getTangent(fBezierDistance);
                            aTangent.normalize();
                        }

                        
                        const basegfx::B2DPoint aBasePoint(rNewTransform * basegfx::B2DPoint(0.0, 0.0));
                        const basegfx::B2DVector aOffset(aBasePoint - mrTextStart);

                        if(!basegfx::fTools::equalZero(aOffset.getY()))
                        {
                            
                            aPosition.setY(aPosition.getY() + aOffset.getY());
                        }

                        
                        aPosition -= fHalfSnippetWidth * aTangent;

                        
                        rNewTransform.translate(-aBasePoint.getX(), -aBasePoint.getY());

                        
                        rNewTransform.rotate(atan2(aTangent.getY(), aTangent.getX()));

                        
                        rNewTransform.translate(aPosition.getX(), aPosition.getY());
                    }

                    
                    advanceToPosition(fEndPos);
                }
            }

            return bRetval;
        }

    } 
} 



namespace svgio
{
    namespace svgreader
    {
        SvgTextPathNode::SvgTextPathNode(
            SvgDocument& rDocument,
            SvgNode* pParent)
        :   SvgNode(SVGTokenTextPath, rDocument, pParent),
            maSvgStyleAttributes(*this),
            maXLink(),
            maStartOffset(),
            mbMethod(true),
            mbSpacing(false)
        {
        }

        SvgTextPathNode::~SvgTextPathNode()
        {
        }

        const SvgStyleAttributes* SvgTextPathNode::getSvgStyleAttributes() const
        {
            return &maSvgStyleAttributes;
        }

        void SvgTextPathNode::parseAttribute(const OUString& rTokenName, SVGToken aSVGToken, const OUString& aContent)
        {
            
            SvgNode::parseAttribute(rTokenName, aSVGToken, aContent);

            
            maSvgStyleAttributes.parseStyleAttribute(rTokenName, aSVGToken, aContent);

            
            switch(aSVGToken)
            {
                case SVGTokenStyle:
                {
                    maSvgStyleAttributes.readStyle(aContent);
                    break;
                }
                case SVGTokenStartOffset:
                {
                    SvgNumber aNum;

                    if(readSingleNumber(aContent, aNum))
                    {
                        if(aNum.isPositive())
                        {
                            setStartOffset(aNum);
                        }
                    }
                    break;
                }
                case SVGTokenMethod:
                {
                    if(!aContent.isEmpty())
                    {
                        if(aContent.startsWith("align"))
                        {
                            setMethod(true);
                        }
                        else if(aContent.startsWith("stretch"))
                        {
                            setMethod(false);
                        }
                    }
                    break;
                }
                case SVGTokenSpacing:
                {
                    if(!aContent.isEmpty())
                    {
                        if(aContent.startsWith("auto"))
                        {
                            setSpacing(true);
                        }
                        else if(aContent.startsWith("exact"))
                        {
                            setSpacing(false);
                        }
                    }
                    break;
                }
                case SVGTokenXlinkHref:
                {
                    const sal_Int32 nLen(aContent.getLength());

                    if(nLen && '#' == aContent[0])
                    {
                        maXLink = aContent.copy(1);
                    }
                    break;
                }
                default:
                {
                    break;
                }
            }
        }

        bool SvgTextPathNode::isValid() const
        {
            const SvgPathNode* pSvgPathNode = dynamic_cast< const SvgPathNode* >(getDocument().findSvgNodeById(maXLink));

            if(!pSvgPathNode)
            {
                return false;
            }

            const basegfx::B2DPolyPolygon* pPolyPolyPath = pSvgPathNode->getPath();

            if(!pPolyPolyPath || !pPolyPolyPath->count())
            {
                return false;
            }

            const basegfx::B2DPolygon aPolygon(pPolyPolyPath->getB2DPolygon(0));

            if(!aPolygon.count())
            {
                return false;
            }

            const double fBasegfxPathLength(basegfx::tools::getLength(aPolygon));

            if(basegfx::fTools::equalZero(fBasegfxPathLength))
            {
                return false;
            }

            return true;
        }

        void SvgTextPathNode::decomposePathNode(
            const drawinglayer::primitive2d::Primitive2DSequence& rPathContent,
            drawinglayer::primitive2d::Primitive2DSequence& rTarget,
            const basegfx::B2DPoint& rTextStart) const
        {
            if(rPathContent.hasElements())
            {
                const SvgPathNode* pSvgPathNode = dynamic_cast< const SvgPathNode* >(getDocument().findSvgNodeById(maXLink));

                if(pSvgPathNode)
                {
                    const basegfx::B2DPolyPolygon* pPolyPolyPath = pSvgPathNode->getPath();

                    if(pPolyPolyPath && pPolyPolyPath->count())
                    {
                        basegfx::B2DPolygon aPolygon(pPolyPolyPath->getB2DPolygon(0));

                        if(pSvgPathNode->getTransform())
                        {
                            aPolygon.transform(*pSvgPathNode->getTransform());
                        }

                        const double fBasegfxPathLength(basegfx::tools::getLength(aPolygon));

                        if(!basegfx::fTools::equalZero(fBasegfxPathLength))
                        {
                            double fUserToBasegfx(1.0); 

                            if(pSvgPathNode->getPathLength().isSet())
                            {
                                const double fUserLength(pSvgPathNode->getPathLength().solve(*this, length));

                                if(fUserLength > 0.0 && !basegfx::fTools::equal(fUserLength, fBasegfxPathLength))
                                {
                                    fUserToBasegfx = fUserLength / fBasegfxPathLength;
                                }
                            }

                            double fPosition(0.0);

                            if(getStartOffset().isSet())
                            {
                                if(Unit_percent == getStartOffset().getUnit())
                                {
                                    
                                    fPosition = getStartOffset().getNumber() * 0.01 * fBasegfxPathLength;
                                }
                                else
                                {
                                    fPosition = getStartOffset().solve(*this, length) * fUserToBasegfx;
                                }
                            }

                            if(fPosition >= 0.0)
                            {
                                const sal_Int32 nLength(rPathContent.getLength());
                                sal_Int32 nCurrent(0);

                                while(fPosition < fBasegfxPathLength && nCurrent < nLength)
                                {
                                    const drawinglayer::primitive2d::TextSimplePortionPrimitive2D* pCandidate = 0;
                                    const drawinglayer::primitive2d::Primitive2DReference xReference(rPathContent[nCurrent]);

                                    if(xReference.is())
                                    {
                                        pCandidate = dynamic_cast< const drawinglayer::primitive2d::TextSimplePortionPrimitive2D* >(xReference.get());
                                    }

                                    if(pCandidate)
                                    {
                                        const pathTextBreakupHelper aPathTextBreakupHelper(
                                            *pCandidate,
                                            aPolygon,
                                            fBasegfxPathLength,
                                            fPosition,
                                            rTextStart);

                                        const drawinglayer::primitive2d::Primitive2DSequence aResult(
                                            aPathTextBreakupHelper.getResult(drawinglayer::primitive2d::BreakupUnit_character));

                                        if(aResult.hasElements())
                                        {
                                            drawinglayer::primitive2d::appendPrimitive2DSequenceToPrimitive2DSequence(rTarget, aResult);
                                        }

                                        
                                        fPosition = aPathTextBreakupHelper.getPosition();
                                    }

                                    nCurrent++;
                                }
                            }
                        }
                    }
                }
            }
        }

    } 
} 




/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
