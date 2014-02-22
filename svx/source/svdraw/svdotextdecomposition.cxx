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


#include <svx/svdetc.hxx>
#include <svx/svdoutl.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdotext.hxx>
#include <basegfx/vector/b2dvector.hxx>
#include <svx/sdr/primitive2d/sdrtextprimitive2d.hxx>
#include <drawinglayer/primitive2d/textprimitive2d.hxx>
#include <drawinglayer/primitive2d/textdecoratedprimitive2d.hxx>
#include <basegfx/range/b2drange.hxx>
#include <editeng/editstat.hxx>
#include <tools/helpers.hxx>
#include <svx/sdtfchim.hxx>
#include <svl/itemset.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <drawinglayer/animation/animationtiming.hxx>
#include <basegfx/color/bcolor.hxx>
#include <vcl/svapp.hxx>
#include <editeng/eeitemid.hxx>
#include <editeng/escapementitem.hxx>
#include <editeng/svxenum.hxx>
#include <editeng/flditem.hxx>
#include <drawinglayer/primitive2d/texthierarchyprimitive2d.hxx>
#include <vcl/metaact.hxx>
#include <drawinglayer/primitive2d/wrongspellprimitive2d.hxx>
#include <drawinglayer/primitive2d/graphicprimitive2d.hxx>
#include <drawinglayer/primitive2d/textlayoutdevice.hxx>
#include <svx/unoapi.hxx>
#include <drawinglayer/geometry/viewinformation2d.hxx>
#include <editeng/outlobj.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>




namespace
{
    drawinglayer::primitive2d::Primitive2DSequence impConvertVectorToPrimitive2DSequence(const std::vector< drawinglayer::primitive2d::BasePrimitive2D* >& rPrimitiveVector)
    {
        const sal_Int32 nCount(rPrimitiveVector.size());
        drawinglayer::primitive2d::Primitive2DSequence aRetval(nCount);

        for(sal_Int32 a(0L); a < nCount; a++)
        {
            aRetval[a] = drawinglayer::primitive2d::Primitive2DReference(rPrimitiveVector[a]);
        }

        return aRetval;
    }

    class impTextBreakupHandler
    {
    private:
        std::vector< drawinglayer::primitive2d::BasePrimitive2D* >  maTextPortionPrimitives;
        std::vector< drawinglayer::primitive2d::BasePrimitive2D* >  maLinePrimitives;
        std::vector< drawinglayer::primitive2d::BasePrimitive2D* >  maParagraphPrimitives;

        SdrOutliner&                                                mrOutliner;
        basegfx::B2DHomMatrix                                       maNewTransformA;
        basegfx::B2DHomMatrix                                       maNewTransformB;

        
        basegfx::B2DVector                                          maScale;

        
        
        
        
        basegfx::B2DRange                                           maClipRange;

        DECL_LINK(decomposeContourTextPrimitive, DrawPortionInfo* );
        DECL_LINK(decomposeBlockTextPrimitive, DrawPortionInfo* );
        DECL_LINK(decomposeStretchTextPrimitive, DrawPortionInfo* );

        DECL_LINK(decomposeContourBulletPrimitive, DrawBulletInfo* );
        DECL_LINK(decomposeBlockBulletPrimitive, DrawBulletInfo* );
        DECL_LINK(decomposeStretchBulletPrimitive, DrawBulletInfo* );

        bool impIsUnderlineAbove(const Font& rFont) const;
        void impCreateTextPortionPrimitive(const DrawPortionInfo& rInfo);
        drawinglayer::primitive2d::BasePrimitive2D* impCheckFieldPrimitive(drawinglayer::primitive2d::BasePrimitive2D* pPrimitive, const DrawPortionInfo& rInfo) const;
        void impFlushTextPortionPrimitivesToLinePrimitives();
        void impFlushLinePrimitivesToParagraphPrimitives();
        void impHandleDrawPortionInfo(const DrawPortionInfo& rInfo);
        void impHandleDrawBulletInfo(const DrawBulletInfo& rInfo);

    public:
        impTextBreakupHandler(SdrOutliner& rOutliner)
        :   maTextPortionPrimitives(),
            maLinePrimitives(),
            maParagraphPrimitives(),
            mrOutliner(rOutliner),
            maNewTransformA(),
            maNewTransformB(),
            maScale(),
            maClipRange()
        {
        }

        void decomposeContourTextPrimitive(const basegfx::B2DHomMatrix& rNewTransformA, const basegfx::B2DHomMatrix& rNewTransformB, const basegfx::B2DVector& rScale)
        {
            maScale = rScale;
            maNewTransformA = rNewTransformA;
            maNewTransformB = rNewTransformB;
            mrOutliner.SetDrawPortionHdl(LINK(this, impTextBreakupHandler, decomposeContourTextPrimitive));
            mrOutliner.SetDrawBulletHdl(LINK(this, impTextBreakupHandler, decomposeContourBulletPrimitive));
            mrOutliner.StripPortions();
            mrOutliner.SetDrawPortionHdl(Link());
            mrOutliner.SetDrawBulletHdl(Link());
        }

        void decomposeBlockTextPrimitive(
            const basegfx::B2DHomMatrix& rNewTransformA,
            const basegfx::B2DHomMatrix& rNewTransformB,
            const basegfx::B2DRange& rClipRange)
        {
            maNewTransformA = rNewTransformA;
            maNewTransformB = rNewTransformB;
            maClipRange = rClipRange;
            mrOutliner.SetDrawPortionHdl(LINK(this, impTextBreakupHandler, decomposeBlockTextPrimitive));
            mrOutliner.SetDrawBulletHdl(LINK(this, impTextBreakupHandler, decomposeBlockBulletPrimitive));
            mrOutliner.StripPortions();
            mrOutliner.SetDrawPortionHdl(Link());
            mrOutliner.SetDrawBulletHdl(Link());
        }

        void decomposeStretchTextPrimitive(const basegfx::B2DHomMatrix& rNewTransformA, const basegfx::B2DHomMatrix& rNewTransformB)
        {
            maNewTransformA = rNewTransformA;
            maNewTransformB = rNewTransformB;
            mrOutliner.SetDrawPortionHdl(LINK(this, impTextBreakupHandler, decomposeStretchTextPrimitive));
            mrOutliner.SetDrawBulletHdl(LINK(this, impTextBreakupHandler, decomposeStretchBulletPrimitive));
            mrOutliner.StripPortions();
            mrOutliner.SetDrawPortionHdl(Link());
            mrOutliner.SetDrawBulletHdl(Link());
        }

        drawinglayer::primitive2d::Primitive2DSequence getPrimitive2DSequence();
    };

    bool impTextBreakupHandler::impIsUnderlineAbove(const Font& rFont) const
    {
        if(!rFont.IsVertical())
        {
            return false;
        }

        if((LANGUAGE_JAPANESE == rFont.GetLanguage()) || (LANGUAGE_JAPANESE == rFont.GetCJKContextLanguage()))
        {
            
            return true;
        }

        return false;
    }

    void impTextBreakupHandler::impCreateTextPortionPrimitive(const DrawPortionInfo& rInfo)
    {
        if(!rInfo.maText.isEmpty() && rInfo.mnTextLen)
        {
            OUString caseMappedText = rInfo.mrFont.CalcCaseMap( rInfo.maText );
            basegfx::B2DVector aFontScaling;
            drawinglayer::attribute::FontAttribute aFontAttribute(
                drawinglayer::primitive2d::getFontAttributeFromVclFont(
                    aFontScaling,
                    rInfo.mrFont,
                    rInfo.IsRTL(),
                    false));
            basegfx::B2DHomMatrix aNewTransform;

            
            aNewTransform.scale(aFontScaling.getX(), aFontScaling.getY());

            
            if(100 != rInfo.mrFont.GetPropr())
            {
                const double fFactor(rInfo.mrFont.GetPropr() / 100.0);
                aNewTransform.scale(fFactor, fFactor);
            }

            
            if(rInfo.mrFont.GetOrientation())
            {
                aNewTransform.rotate(-rInfo.mrFont.GetOrientation() * F_PI1800);
            }

            
            if(rInfo.mrFont.GetEscapement())
            {
                sal_Int16 nEsc(rInfo.mrFont.GetEscapement());

                if(DFLT_ESC_AUTO_SUPER == nEsc)
                {
                    nEsc = 33;
                }
                else if(DFLT_ESC_AUTO_SUB == nEsc)
                {
                    nEsc = -20;
                }

                if(nEsc > 100)
                {
                    nEsc = 100;
                }
                else if(nEsc < -100)
                {
                    nEsc = -100;
                }

                const double fEscapement(nEsc / -100.0);
                aNewTransform.translate(0.0, fEscapement * aFontScaling.getY());
            }

            
            aNewTransform *= maNewTransformA;

            
            aNewTransform.translate(rInfo.mrStartPos.X(), rInfo.mrStartPos.Y());

            
            aNewTransform *= maNewTransformB;

            
            
            ::std::vector< double > aDXArray;
            static bool bDisableTextArray(false);

            if(!bDisableTextArray && rInfo.mpDXArray && rInfo.mnTextLen)
            {
                aDXArray.reserve(rInfo.mnTextLen);

                for(sal_Int32 a=0; a < rInfo.mnTextLen; a++)
                {
                    aDXArray.push_back((double)rInfo.mpDXArray[a]);
                }
            }

            
            const Color aFontColor(rInfo.mrFont.GetColor());
            const basegfx::BColor aBFontColor(aFontColor.getBColor());

            
            
            
            const bool bWordLineMode(rInfo.mrFont.IsWordLineMode() && !rInfo.mbEndOfBullet);

            
            drawinglayer::primitive2d::BasePrimitive2D* pNewPrimitive = 0;
            const bool bDecoratedIsNeeded(
                   UNDERLINE_NONE != rInfo.mrFont.GetOverline()
                || UNDERLINE_NONE != rInfo.mrFont.GetUnderline()
                || STRIKEOUT_NONE != rInfo.mrFont.GetStrikeout()
                || EMPHASISMARK_NONE != (rInfo.mrFont.GetEmphasisMark() & EMPHASISMARK_STYLE)
                || RELIEF_NONE != rInfo.mrFont.GetRelief()
                || rInfo.mrFont.IsShadow()
                || bWordLineMode);

            if(bDecoratedIsNeeded)
            {
                
                
                const Color aUnderlineColor(rInfo.maTextLineColor);
                const basegfx::BColor aBUnderlineColor((0xffffffff == aUnderlineColor.GetColor()) ? aBFontColor : aUnderlineColor.getBColor());
                const Color aOverlineColor(rInfo.maOverlineColor);
                const basegfx::BColor aBOverlineColor((0xffffffff == aOverlineColor.GetColor()) ? aBFontColor : aOverlineColor.getBColor());

                
                const drawinglayer::primitive2d::TextLine eFontOverline(
                    drawinglayer::primitive2d::mapFontUnderlineToTextLine(rInfo.mrFont.GetOverline()));
                const drawinglayer::primitive2d::TextLine eFontUnderline(
                    drawinglayer::primitive2d::mapFontUnderlineToTextLine(rInfo.mrFont.GetUnderline()));

                
                const bool bUnderlineAbove(
                    drawinglayer::primitive2d::TEXT_LINE_NONE != eFontUnderline && impIsUnderlineAbove(rInfo.mrFont));

                
                const drawinglayer::primitive2d::TextStrikeout eTextStrikeout(
                    drawinglayer::primitive2d::mapFontStrikeoutToTextStrikeout(rInfo.mrFont.GetStrikeout()));

                
                drawinglayer::primitive2d::TextEmphasisMark eTextEmphasisMark(drawinglayer::primitive2d::TEXT_EMPHASISMARK_NONE);

                switch(rInfo.mrFont.GetEmphasisMark() & EMPHASISMARK_STYLE)
                {
                    case EMPHASISMARK_DOT : eTextEmphasisMark = drawinglayer::primitive2d::TEXT_EMPHASISMARK_DOT; break;
                    case EMPHASISMARK_CIRCLE : eTextEmphasisMark = drawinglayer::primitive2d::TEXT_EMPHASISMARK_CIRCLE; break;
                    case EMPHASISMARK_DISC : eTextEmphasisMark = drawinglayer::primitive2d::TEXT_EMPHASISMARK_DISC; break;
                    case EMPHASISMARK_ACCENT : eTextEmphasisMark = drawinglayer::primitive2d::TEXT_EMPHASISMARK_ACCENT; break;
                }

                const bool bEmphasisMarkAbove(rInfo.mrFont.GetEmphasisMark() & EMPHASISMARK_POS_ABOVE);
                const bool bEmphasisMarkBelow(rInfo.mrFont.GetEmphasisMark() & EMPHASISMARK_POS_BELOW);

                
                drawinglayer::primitive2d::TextRelief eTextRelief(drawinglayer::primitive2d::TEXT_RELIEF_NONE);

                switch(rInfo.mrFont.GetRelief())
                {
                    case RELIEF_EMBOSSED : eTextRelief = drawinglayer::primitive2d::TEXT_RELIEF_EMBOSSED; break;
                    case RELIEF_ENGRAVED : eTextRelief = drawinglayer::primitive2d::TEXT_RELIEF_ENGRAVED; break;
                    default : break; 
                }

                
                const bool bShadow(rInfo.mrFont.IsShadow());

                
                pNewPrimitive = new drawinglayer::primitive2d::TextDecoratedPortionPrimitive2D(

                    
                    aNewTransform,
                    caseMappedText,
                    rInfo.mnTextStart,
                    rInfo.mnTextLen,
                    aDXArray,
                    aFontAttribute,
                    rInfo.mpLocale ? *rInfo.mpLocale : ::com::sun::star::lang::Locale(),
                    aBFontColor,

                    
                    aBOverlineColor,
                    aBUnderlineColor,
                    eFontOverline,
                    eFontUnderline,
                    bUnderlineAbove,
                    eTextStrikeout,
                    bWordLineMode,
                    eTextEmphasisMark,
                    bEmphasisMarkAbove,
                    bEmphasisMarkBelow,
                    eTextRelief,
                    bShadow);
            }
            else
            {
                
                pNewPrimitive = new drawinglayer::primitive2d::TextSimplePortionPrimitive2D(
                    aNewTransform,
                    caseMappedText,
                    rInfo.mnTextStart,
                    rInfo.mnTextLen,
                    aDXArray,
                    aFontAttribute,
                    rInfo.mpLocale ? *rInfo.mpLocale : ::com::sun::star::lang::Locale(),
                    aBFontColor,
                    rInfo.mbFilled,
                    rInfo.mnWidthToFill);
            }

            if(rInfo.mbEndOfBullet)
            {
                
                const drawinglayer::primitive2d::Primitive2DReference aNewReference(pNewPrimitive);
                const drawinglayer::primitive2d::Primitive2DSequence aNewSequence(&aNewReference, 1);
                pNewPrimitive = new drawinglayer::primitive2d::TextHierarchyBulletPrimitive2D(aNewSequence);
            }

            if(rInfo.mpFieldData)
            {
                pNewPrimitive = impCheckFieldPrimitive(pNewPrimitive, rInfo);
            }

            maTextPortionPrimitives.push_back(pNewPrimitive);

            
            if(rInfo.mpWrongSpellVector && !aDXArray.empty())
            {
                const sal_Int32 nSize(rInfo.mpWrongSpellVector->size());
                const sal_Int32 nDXCount(aDXArray.size());
                const basegfx::BColor aSpellColor(1.0, 0.0, 0.0); 

                for(sal_Int32 a(0); a < nSize; a++)
                {
                    const EEngineData::WrongSpellClass& rCandidate = (*rInfo.mpWrongSpellVector)[a];

                    if(rCandidate.nStart >= rInfo.mnTextStart && rCandidate.nEnd >= rInfo.mnTextStart && rCandidate.nEnd > rCandidate.nStart)
                    {
                        const sal_Int32 nStart(rCandidate.nStart - rInfo.mnTextStart);
                        const sal_Int32 nEnd(rCandidate.nEnd - rInfo.mnTextStart);
                        double fStart(0.0);
                        double fEnd(0.0);

                        if(nStart > 0 && nStart - 1 < nDXCount)
                        {
                            fStart = aDXArray[nStart - 1];
                        }

                        if(nEnd > 0 && nEnd - 1 < nDXCount)
                        {
                            fEnd = aDXArray[nEnd - 1];
                        }

                        if(!basegfx::fTools::equal(fStart, fEnd))
                        {
                            if(rInfo.IsRTL())
                            {
                                
                                
                                
                                const double fTextWidth(aDXArray[aDXArray.size() - 1]);

                                fStart = fTextWidth - fStart;
                                fEnd = fTextWidth - fEnd;
                            }

                            
                            
                            const double fFontScaleX(aFontScaling.getX());

                            if(!basegfx::fTools::equal(fFontScaleX, 1.0)
                                && !basegfx::fTools::equalZero(fFontScaleX))
                            {
                                fStart /= fFontScaleX;
                                fEnd /= fFontScaleX;
                            }

                            maTextPortionPrimitives.push_back(new drawinglayer::primitive2d::WrongSpellPrimitive2D(
                                aNewTransform,
                                fStart,
                                fEnd,
                                aSpellColor));
                        }
                    }
                }
            }
        }
    }

    drawinglayer::primitive2d::BasePrimitive2D* impTextBreakupHandler::impCheckFieldPrimitive(drawinglayer::primitive2d::BasePrimitive2D* pPrimitive, const DrawPortionInfo& rInfo) const
    {
        if(rInfo.mpFieldData)
        {
            
            
            const SvxURLField* pURLField = dynamic_cast< const SvxURLField* >(rInfo.mpFieldData);
            const SvxPageField* pPageField = dynamic_cast< const SvxPageField* >(rInfo.mpFieldData);

            
            drawinglayer::primitive2d::Primitive2DSequence aSequence;

            if(pPrimitive)
            {
                aSequence.realloc(1);
                aSequence[0] = drawinglayer::primitive2d::Primitive2DReference(pPrimitive);
            }

            if(pURLField)
            {
                pPrimitive = new drawinglayer::primitive2d::TextHierarchyFieldPrimitive2D(aSequence, drawinglayer::primitive2d::FIELD_TYPE_URL, pURLField->GetURL());
            }
            else if(pPageField)
            {
                pPrimitive = new drawinglayer::primitive2d::TextHierarchyFieldPrimitive2D(aSequence, drawinglayer::primitive2d::FIELD_TYPE_PAGE, "");
            }
            else
            {
                pPrimitive = new drawinglayer::primitive2d::TextHierarchyFieldPrimitive2D(aSequence, drawinglayer::primitive2d::FIELD_TYPE_COMMON, "");
            }
        }

        return pPrimitive;
    }

    void impTextBreakupHandler::impFlushTextPortionPrimitivesToLinePrimitives()
    {
        
        
        if(!maTextPortionPrimitives.empty())
        {
            drawinglayer::primitive2d::Primitive2DSequence aLineSequence(impConvertVectorToPrimitive2DSequence(maTextPortionPrimitives));
            maTextPortionPrimitives.clear();
            maLinePrimitives.push_back(new drawinglayer::primitive2d::TextHierarchyLinePrimitive2D(aLineSequence));
        }
    }

    void impTextBreakupHandler::impFlushLinePrimitivesToParagraphPrimitives()
    {
        
        
        
        drawinglayer::primitive2d::Primitive2DSequence aParagraphSequence(impConvertVectorToPrimitive2DSequence(maLinePrimitives));
        maLinePrimitives.clear();
        maParagraphPrimitives.push_back(new drawinglayer::primitive2d::TextHierarchyParagraphPrimitive2D(aParagraphSequence));
    }

    void impTextBreakupHandler::impHandleDrawPortionInfo(const DrawPortionInfo& rInfo)
    {
        impCreateTextPortionPrimitive(rInfo);

        if(rInfo.mbEndOfLine || rInfo.mbEndOfParagraph)
        {
            impFlushTextPortionPrimitivesToLinePrimitives();
        }

        if(rInfo.mbEndOfParagraph)
        {
            impFlushLinePrimitivesToParagraphPrimitives();
        }
    }

    void impTextBreakupHandler::impHandleDrawBulletInfo(const DrawBulletInfo& rInfo)
    {
        basegfx::B2DHomMatrix aNewTransform;

        
        aNewTransform.scale(rInfo.maBulletSize.getWidth(), rInfo.maBulletSize.getHeight());

        
        aNewTransform *= maNewTransformA;

        
        aNewTransform.translate(rInfo.maBulletPosition.X(), rInfo.maBulletPosition.Y());

        
        aNewTransform *= maNewTransformB;

        
        const GraphicAttr aGraphicAttr;

        
        const drawinglayer::primitive2d::Primitive2DReference aNewReference(new drawinglayer::primitive2d::GraphicPrimitive2D(
            aNewTransform,
            rInfo.maBulletGraphicObject,
            aGraphicAttr));

        
        const drawinglayer::primitive2d::Primitive2DSequence aNewSequence(&aNewReference, 1);
        drawinglayer::primitive2d::BasePrimitive2D* pNewPrimitive = new drawinglayer::primitive2d::TextHierarchyBulletPrimitive2D(aNewSequence);

        
        maTextPortionPrimitives.push_back(pNewPrimitive);
    }

    IMPL_LINK(impTextBreakupHandler, decomposeContourTextPrimitive, DrawPortionInfo*, pInfo)
    {
        
        
        if(pInfo && (double)pInfo->mrStartPos.Y() < maScale.getY())
        {
            impHandleDrawPortionInfo(*pInfo);
        }

        return 0;
    }

    IMPL_LINK(impTextBreakupHandler, decomposeBlockTextPrimitive, DrawPortionInfo*, pInfo)
    {
        if(pInfo)
        {
            
            
            if(!maClipRange.isEmpty())
            {
                
                
                const basegfx::B2DPoint aStartPosition(pInfo->mrStartPos.X(), pInfo->mrStartPos.Y());

                if(!maClipRange.isInside(aStartPosition))
                {
                    return 0;
                }

                
                drawinglayer::primitive2d::TextLayouterDevice aTextLayouterDevice;
                aTextLayouterDevice.setFont(pInfo->mrFont);

                const basegfx::B2DRange aTextBoundRect(
                    aTextLayouterDevice.getTextBoundRect(
                        pInfo->maText, pInfo->mnTextStart, pInfo->mnTextLen));
                const basegfx::B2DPoint aTopLeft(aTextBoundRect.getMinimum() + aStartPosition);

                if(!maClipRange.isInside(aTopLeft))
                {
                    return 0;
                }

                
                const basegfx::B2DPoint aBottomRight(aTextBoundRect.getMaximum() + aStartPosition);

                if(!maClipRange.isInside(aBottomRight))
                {
                    return 0;
                }

                
            }
            impHandleDrawPortionInfo(*pInfo);
        }

        return 0;
    }

    IMPL_LINK(impTextBreakupHandler, decomposeStretchTextPrimitive, DrawPortionInfo*, pInfo)
    {
        if(pInfo)
        {
            impHandleDrawPortionInfo(*pInfo);
        }

        return 0;
    }

    IMPL_LINK(impTextBreakupHandler, decomposeContourBulletPrimitive, DrawBulletInfo*, pInfo)
    {
        if(pInfo)
        {
            impHandleDrawBulletInfo(*pInfo);
        }

        return 0;
    }

    IMPL_LINK(impTextBreakupHandler, decomposeBlockBulletPrimitive, DrawBulletInfo*, pInfo)
    {
        if(pInfo)
        {
            impHandleDrawBulletInfo(*pInfo);
        }

        return 0;
    }

    IMPL_LINK(impTextBreakupHandler, decomposeStretchBulletPrimitive, DrawBulletInfo*, pInfo)
    {
        if(pInfo)
        {
            impHandleDrawBulletInfo(*pInfo);
        }

        return 0;
    }

    drawinglayer::primitive2d::Primitive2DSequence impTextBreakupHandler::getPrimitive2DSequence()
    {
        if(!maTextPortionPrimitives.empty())
        {
            
            impFlushTextPortionPrimitivesToLinePrimitives();
        }

        if(!maLinePrimitives.empty())
        {
            
            impFlushLinePrimitivesToParagraphPrimitives();
        }

        return impConvertVectorToPrimitive2DSequence(maParagraphPrimitives);
    }
} 




void SdrTextObj::impDecomposeContourTextPrimitive(
    drawinglayer::primitive2d::Primitive2DSequence& rTarget,
    const drawinglayer::primitive2d::SdrContourTextPrimitive2D& rSdrContourTextPrimitive,
    const drawinglayer::geometry::ViewInformation2D& aViewInformation) const
{
    
    basegfx::B2DVector aScale, aTranslate;
    double fRotate, fShearX;
    rSdrContourTextPrimitive.getObjectTransform().decompose(aScale, aTranslate, fRotate, fShearX);

    
    basegfx::B2DPolyPolygon aPolyPolygon(rSdrContourTextPrimitive.getUnitPolyPolygon());
    aPolyPolygon.transform(basegfx::tools::createScaleB2DHomMatrix(fabs(aScale.getX()), fabs(aScale.getY())));

    
    SdrOutliner& rOutliner = ImpGetDrawOutliner();
    const Size aNullSize;
    rOutliner.SetPaperSize(aNullSize);
    rOutliner.SetPolygon(aPolyPolygon);
    rOutliner.SetUpdateMode(true);
    rOutliner.SetText(rSdrContourTextPrimitive.getOutlinerParaObject());

    
    rOutliner.setVisualizedPage(GetSdrPageFromXDrawPage(aViewInformation.getVisualizedPage()));

    
    basegfx::B2DHomMatrix aNewTransformA;

    
    
    const bool bMirrorX(basegfx::fTools::less(aScale.getX(), 0.0));
    const bool bMirrorY(basegfx::fTools::less(aScale.getY(), 0.0));

    
    
    const basegfx::B2DHomMatrix aNewTransformB(basegfx::tools::createScaleShearXRotateTranslateB2DHomMatrix(
        bMirrorX ? -1.0 : 1.0, bMirrorY ? -1.0 : 1.0,
        fShearX, fRotate, aTranslate.getX(), aTranslate.getY()));

    
    impTextBreakupHandler aConverter(rOutliner);
    aConverter.decomposeContourTextPrimitive(aNewTransformA, aNewTransformB, aScale);

    
    rOutliner.Clear();
    rOutliner.setVisualizedPage(0);

    rTarget = aConverter.getPrimitive2DSequence();
}

void SdrTextObj::impDecomposeAutoFitTextPrimitive(
    drawinglayer::primitive2d::Primitive2DSequence& rTarget,
    const drawinglayer::primitive2d::SdrAutoFitTextPrimitive2D& rSdrAutofitTextPrimitive,
    const drawinglayer::geometry::ViewInformation2D& aViewInformation) const
{
    
    basegfx::B2DVector aScale, aTranslate;
    double fRotate, fShearX;
    rSdrAutofitTextPrimitive.getTextRangeTransform().decompose(aScale, aTranslate, fRotate, fShearX);

    
    basegfx::B2DRange aAnchorTextRange(aTranslate);
    aAnchorTextRange.expand(aTranslate + aScale);

    
    const SfxItemSet& rTextItemSet = rSdrAutofitTextPrimitive.getSdrText()->GetItemSet();
    SdrOutliner& rOutliner = ImpGetDrawOutliner();
    SdrTextVertAdjust eVAdj = GetTextVerticalAdjust(rTextItemSet);
    SdrTextHorzAdjust eHAdj = GetTextHorizontalAdjust(rTextItemSet);
    const sal_uInt32 nOriginalControlWord(rOutliner.GetControlWord());
    const Size aNullSize;

    
    rOutliner.setVisualizedPage(GetSdrPageFromXDrawPage(aViewInformation.getVisualizedPage()));

    rOutliner.SetControlWord(nOriginalControlWord|EE_CNTRL_AUTOPAGESIZE|EE_CNTRL_STRETCHING);
    rOutliner.SetMinAutoPaperSize(aNullSize);
    rOutliner.SetMaxAutoPaperSize(Size(1000000,1000000));

    
    const sal_uInt32 nAnchorTextWidth(FRound(aAnchorTextRange.getWidth() + 1L));
    const sal_uInt32 nAnchorTextHeight(FRound(aAnchorTextRange.getHeight() + 1L));
    const OutlinerParaObject* pOutlinerParaObject = rSdrAutofitTextPrimitive.getSdrText()->GetOutlinerParaObject();
    OSL_ENSURE(pOutlinerParaObject, "impDecomposeBlockTextPrimitive used with no OutlinerParaObject (!)");
    const bool bVerticalWritintg(pOutlinerParaObject->IsVertical());
    const Size aAnchorTextSize(Size(nAnchorTextWidth, nAnchorTextHeight));

    if((rSdrAutofitTextPrimitive.getWordWrap() || IsTextFrame()))
    {
        rOutliner.SetMaxAutoPaperSize(aAnchorTextSize);
    }

    if(SDRTEXTHORZADJUST_BLOCK == eHAdj && !bVerticalWritintg)
    {
        rOutliner.SetMinAutoPaperSize(Size(nAnchorTextWidth, 0));
    }

    if(SDRTEXTVERTADJUST_BLOCK == eVAdj && bVerticalWritintg)
    {
        rOutliner.SetMinAutoPaperSize(Size(0, nAnchorTextHeight));
    }

    rOutliner.SetPaperSize(aNullSize);
    rOutliner.SetUpdateMode(true);
    rOutliner.SetText(*pOutlinerParaObject);
    ImpAutoFitText(rOutliner,aAnchorTextSize,bVerticalWritintg);

    
    rOutliner.setVisualizedPage(GetSdrPageFromXDrawPage(aViewInformation.getVisualizedPage()));

    
    const Size aOutlinerTextSize(rOutliner.GetPaperSize());
    const basegfx::B2DVector aOutlinerScale(aOutlinerTextSize.Width(), aOutlinerTextSize.Height());
    basegfx::B2DVector aAdjustTranslate(0.0, 0.0);

    
    if(SDRTEXTHORZADJUST_CENTER == eHAdj || SDRTEXTHORZADJUST_RIGHT == eHAdj)
    {
        const double fFree(aAnchorTextRange.getWidth() - aOutlinerScale.getX());

        if(SDRTEXTHORZADJUST_CENTER == eHAdj)
        {
            aAdjustTranslate.setX(fFree / 2.0);
        }

        if(SDRTEXTHORZADJUST_RIGHT == eHAdj)
        {
            aAdjustTranslate.setX(fFree);
        }
    }

    
    if(SDRTEXTVERTADJUST_CENTER == eVAdj || SDRTEXTVERTADJUST_BOTTOM == eVAdj)
    {
        const double fFree(aAnchorTextRange.getHeight() - aOutlinerScale.getY());

        if(SDRTEXTVERTADJUST_CENTER == eVAdj)
        {
            aAdjustTranslate.setY(fFree / 2.0);
        }

        if(SDRTEXTVERTADJUST_BOTTOM == eVAdj)
        {
            aAdjustTranslate.setY(fFree);
        }
    }

    
    
    basegfx::B2DHomMatrix aNewTransformA;
    basegfx::B2DHomMatrix aNewTransformB;

    
    
    
    const double fStartInX(bVerticalWritintg ? aAdjustTranslate.getX() + aOutlinerScale.getX() : aAdjustTranslate.getX());
    aNewTransformA.translate(fStartInX, aAdjustTranslate.getY());

    
    
    const bool bMirrorX(basegfx::fTools::less(aScale.getX(), 0.0));
    const bool bMirrorY(basegfx::fTools::less(aScale.getY(), 0.0));
    aNewTransformB.scale(bMirrorX ? -1.0 : 1.0, bMirrorY ? -1.0 : 1.0);

    
    
    aNewTransformB.shearX(fShearX);
    aNewTransformB.rotate(fRotate);
    aNewTransformB.translate(aTranslate.getX(), aTranslate.getY());

    basegfx::B2DRange aClipRange;

    
    impTextBreakupHandler aConverter(rOutliner);
    aConverter.decomposeBlockTextPrimitive(aNewTransformA, aNewTransformB, aClipRange);

    
    rOutliner.Clear();
    rOutliner.setVisualizedPage(0);
    rOutliner.SetControlWord(nOriginalControlWord);

    rTarget = aConverter.getPrimitive2DSequence();
}

void SdrTextObj::impDecomposeBlockTextPrimitive(
    drawinglayer::primitive2d::Primitive2DSequence& rTarget,
    const drawinglayer::primitive2d::SdrBlockTextPrimitive2D& rSdrBlockTextPrimitive,
    const drawinglayer::geometry::ViewInformation2D& aViewInformation) const
{
    
    basegfx::B2DVector aScale, aTranslate;
    double fRotate, fShearX;
    rSdrBlockTextPrimitive.getTextRangeTransform().decompose(aScale, aTranslate, fRotate, fShearX);

    
    basegfx::B2DRange aAnchorTextRange(aTranslate);
    aAnchorTextRange.expand(aTranslate + aScale);

    
    const bool bIsCell(rSdrBlockTextPrimitive.getCellText());
    SdrOutliner& rOutliner = ImpGetDrawOutliner();
    SdrTextHorzAdjust eHAdj = rSdrBlockTextPrimitive.getSdrTextHorzAdjust();
    SdrTextVertAdjust eVAdj = rSdrBlockTextPrimitive.getSdrTextVertAdjust();
    const sal_uInt32 nOriginalControlWord(rOutliner.GetControlWord());
    const Size aNullSize;

    
    rOutliner.setVisualizedPage(GetSdrPageFromXDrawPage(aViewInformation.getVisualizedPage()));
    rOutliner.SetFixedCellHeight(rSdrBlockTextPrimitive.isFixedCellHeight());
    rOutliner.SetControlWord(nOriginalControlWord|EE_CNTRL_AUTOPAGESIZE);
    rOutliner.SetMinAutoPaperSize(aNullSize);
    rOutliner.SetMaxAutoPaperSize(Size(1000000,1000000));

    
    
    
    Color aOriginalBackColor(rOutliner.GetBackgroundColor());
    const SfxItemSet* pBackgroundFillSet = &GetObjectItemSet();

    if (XFILL_NONE == ((const XFillStyleItem&)pBackgroundFillSet->Get(XATTR_FILLSTYLE)).GetValue())
    {
        SdrPage *pOwnerPage = GetPage();
        if (pOwnerPage)
        {
            pBackgroundFillSet = &pOwnerPage->getSdrPageProperties().GetItemSet();

            if (XFILL_NONE == ((const XFillStyleItem&)pBackgroundFillSet->Get(XATTR_FILLSTYLE)).GetValue())
            {
                if (!pOwnerPage->IsMasterPage() && pOwnerPage->TRG_HasMasterPage())
                {
                    pBackgroundFillSet = &pOwnerPage->TRG_GetMasterPage().getSdrPageProperties().GetItemSet();
                }
            }
        }
    }

    if (XFILL_NONE != ((const XFillStyleItem&)pBackgroundFillSet->Get(XATTR_FILLSTYLE)).GetValue())
    {
        Color aColor(rOutliner.GetBackgroundColor());
        GetDraftFillColor(*pBackgroundFillSet, aColor);
        rOutliner.SetBackgroundColor(aColor);
    }

    
    const sal_uInt32 nAnchorTextWidth(FRound(aAnchorTextRange.getWidth() + 1L));
    const sal_uInt32 nAnchorTextHeight(FRound(aAnchorTextRange.getHeight() + 1L));
    const bool bVerticalWritintg(rSdrBlockTextPrimitive.getOutlinerParaObject().IsVertical());
    const Size aAnchorTextSize(Size(nAnchorTextWidth, nAnchorTextHeight));

    if(bIsCell)
    {
        
        
        rOutliner.SetMaxAutoPaperSize(aAnchorTextSize);

        
        
        
        
        
        if(bVerticalWritintg)
        {
            
            rOutliner.SetMinAutoPaperSize(Size(0, aAnchorTextSize.Height()));
        }
        else
        {
            
            rOutliner.SetMinAutoPaperSize(Size(aAnchorTextSize.Width(), 0));
        }

        rOutliner.SetPaperSize(aAnchorTextSize);
        rOutliner.SetUpdateMode(true);
        rOutliner.SetText(rSdrBlockTextPrimitive.getOutlinerParaObject());
    }
    else
    {
        
        const bool bHorizontalIsBlock(SDRTEXTHORZADJUST_BLOCK == eHAdj && !bVerticalWritintg);
        const bool bVerticalIsBlock(SDRTEXTVERTADJUST_BLOCK == eVAdj && bVerticalWritintg);

        
        if(bHorizontalIsBlock)
        {
            rOutliner.SetMinAutoPaperSize(Size(nAnchorTextWidth, 0));
        }
        else if(bVerticalIsBlock)
        {
            rOutliner.SetMinAutoPaperSize(Size(0, nAnchorTextHeight));
        }

        if((rSdrBlockTextPrimitive.getWordWrap() || IsTextFrame()) && !rSdrBlockTextPrimitive.getUnlimitedPage())
        {
            
            
            
            
            
            
            Size aMaxAutoPaperSize(aAnchorTextSize);

            if(bHorizontalIsBlock)
            {
                
                aMaxAutoPaperSize.setHeight(1000000);
            }
            else if(bVerticalIsBlock)
            {
                
                aMaxAutoPaperSize.setWidth(1000000);
            }

            rOutliner.SetMaxAutoPaperSize(aMaxAutoPaperSize);
        }

        rOutliner.SetPaperSize(aNullSize);
        rOutliner.SetUpdateMode(true);
        rOutliner.SetText(rSdrBlockTextPrimitive.getOutlinerParaObject());
    }

    rOutliner.SetControlWord(nOriginalControlWord);

    
    const Size aOutlinerTextSize(rOutliner.GetPaperSize());
    const basegfx::B2DVector aOutlinerScale(aOutlinerTextSize.Width(), aOutlinerTextSize.Height());
    basegfx::B2DVector aAdjustTranslate(0.0, 0.0);

    
    
    
    if(!IsTextFrame() && !bIsCell)
    {
        if(aAnchorTextRange.getWidth() < aOutlinerScale.getX() && !bVerticalWritintg)
        {
            
            
            if(SDRTEXTHORZADJUST_BLOCK == eHAdj)
            {
                eHAdj = SDRTEXTHORZADJUST_CENTER;
            }
        }

        if(aAnchorTextRange.getHeight() < aOutlinerScale.getY() && bVerticalWritintg)
        {
            
            
            if(SDRTEXTVERTADJUST_BLOCK == eVAdj)
            {
                eVAdj = SDRTEXTVERTADJUST_CENTER;
            }
        }
    }

    
    if(SDRTEXTHORZADJUST_CENTER == eHAdj || SDRTEXTHORZADJUST_RIGHT == eHAdj)
    {
        const double fFree(aAnchorTextRange.getWidth() - aOutlinerScale.getX());

        if(SDRTEXTHORZADJUST_CENTER == eHAdj)
        {
            aAdjustTranslate.setX(fFree / 2.0);
        }

        if(SDRTEXTHORZADJUST_RIGHT == eHAdj)
        {
            aAdjustTranslate.setX(fFree);
        }
    }

    
    if(SDRTEXTVERTADJUST_CENTER == eVAdj || SDRTEXTVERTADJUST_BOTTOM == eVAdj)
    {
        const double fFree(aAnchorTextRange.getHeight() - aOutlinerScale.getY());

        if(SDRTEXTVERTADJUST_CENTER == eVAdj)
        {
            aAdjustTranslate.setY(fFree / 2.0);
        }

        if(SDRTEXTVERTADJUST_BOTTOM == eVAdj)
        {
            aAdjustTranslate.setY(fFree);
        }
    }

    
    
    
    
    
    const double fStartInX(bVerticalWritintg ? aAdjustTranslate.getX() + aOutlinerScale.getX() : aAdjustTranslate.getX());
    const basegfx::B2DTuple aAdjOffset(fStartInX, aAdjustTranslate.getY());
    basegfx::B2DHomMatrix aNewTransformA(basegfx::tools::createTranslateB2DHomMatrix(aAdjOffset.getX(), aAdjOffset.getY()));

    
    
    const bool bMirrorX(basegfx::fTools::less(aScale.getX(), 0.0));
    const bool bMirrorY(basegfx::fTools::less(aScale.getY(), 0.0));

    
    
    const basegfx::B2DHomMatrix aNewTransformB(basegfx::tools::createScaleShearXRotateTranslateB2DHomMatrix(
        bMirrorX ? -1.0 : 1.0, bMirrorY ? -1.0 : 1.0,
        fShearX, fRotate, aTranslate.getX(), aTranslate.getY()));

    
    basegfx::B2DRange aClipRange;

    if(rSdrBlockTextPrimitive.getClipOnBounds())
    {
        aClipRange.expand(-aAdjOffset);
        aClipRange.expand(basegfx::B2DTuple(aAnchorTextSize.Width(), aAnchorTextSize.Height()) - aAdjOffset);
    }

    
    impTextBreakupHandler aConverter(rOutliner);
    aConverter.decomposeBlockTextPrimitive(aNewTransformA, aNewTransformB, aClipRange);

    
    rOutliner.SetBackgroundColor(aOriginalBackColor);
    rOutliner.Clear();
    rOutliner.setVisualizedPage(0);

    rTarget = aConverter.getPrimitive2DSequence();
}

void SdrTextObj::impDecomposeStretchTextPrimitive(
    drawinglayer::primitive2d::Primitive2DSequence& rTarget,
    const drawinglayer::primitive2d::SdrStretchTextPrimitive2D& rSdrStretchTextPrimitive,
    const drawinglayer::geometry::ViewInformation2D& aViewInformation) const
{
    
    basegfx::B2DVector aScale, aTranslate;
    double fRotate, fShearX;
    rSdrStretchTextPrimitive.getTextRangeTransform().decompose(aScale, aTranslate, fRotate, fShearX);

    
    basegfx::B2DRange aAnchorTextRange(aTranslate);
    aAnchorTextRange.expand(aTranslate + aScale);

    
    SdrOutliner& rOutliner = ImpGetDrawOutliner();
    const sal_uInt32 nOriginalControlWord(rOutliner.GetControlWord());
    const Size aNullSize;

    rOutliner.SetControlWord(nOriginalControlWord|EE_CNTRL_STRETCHING|EE_CNTRL_AUTOPAGESIZE);
    rOutliner.SetFixedCellHeight(rSdrStretchTextPrimitive.isFixedCellHeight());
    rOutliner.SetMinAutoPaperSize(aNullSize);
    rOutliner.SetMaxAutoPaperSize(Size(1000000,1000000));
    rOutliner.SetPaperSize(aNullSize);
    rOutliner.SetUpdateMode(true);
    rOutliner.SetText(rSdrStretchTextPrimitive.getOutlinerParaObject());

    
    rOutliner.setVisualizedPage(GetSdrPageFromXDrawPage(aViewInformation.getVisualizedPage()));

    
    const Size aOutlinerTextSize(rOutliner.CalcTextSize());
    const basegfx::B2DVector aOutlinerScale(
        basegfx::fTools::equalZero(aOutlinerTextSize.Width()) ? 1.0 : aOutlinerTextSize.Width(),
        basegfx::fTools::equalZero(aOutlinerTextSize.Height()) ? 1.0 : aOutlinerTextSize.Height());

    
    basegfx::B2DHomMatrix aNewTransformA;

    
    
    
    const bool bVertical(rSdrStretchTextPrimitive.getOutlinerParaObject().IsVertical());

    if(bVertical)
    {
        aNewTransformA.translate(aScale.getX(), 0.0);
    }

    
    
    const double fScaleX(fabs(aScale.getX()) / aOutlinerScale.getX());
    const double fScaleY(fabs(aScale.getY()) / aOutlinerScale.getY());
    rOutliner.SetGlobalCharStretching((sal_Int16)FRound(fScaleX * 100.0), (sal_Int16)FRound(fScaleY * 100.0));

    
    
    const bool bMirrorX(basegfx::fTools::less(aScale.getX(), 0.0));
    const bool bMirrorY(basegfx::fTools::less(aScale.getY(), 0.0));

    
    
    const basegfx::B2DHomMatrix aNewTransformB(basegfx::tools::createScaleShearXRotateTranslateB2DHomMatrix(
        bMirrorX ? -1.0 : 1.0, bMirrorY ? -1.0 : 1.0,
        fShearX, fRotate, aTranslate.getX(), aTranslate.getY()));

    
    impTextBreakupHandler aConverter(rOutliner);
    aConverter.decomposeStretchTextPrimitive(aNewTransformA, aNewTransformB);

    
    rOutliner.SetControlWord(nOriginalControlWord);
    rOutliner.Clear();
    rOutliner.setVisualizedPage(0);

    rTarget = aConverter.getPrimitive2DSequence();
}



#define ENDLESS_LOOP    (0xffffffff)
#define ENDLESS_TIME    ((double)0xffffffff)
#define PIXEL_DPI       (96.0)

void SdrTextObj::impGetBlinkTextTiming(drawinglayer::animation::AnimationEntryList& rAnimList) const
{
    if(SDRTEXTANI_BLINK == GetTextAniKind())
    {
        
        const SfxItemSet& rSet = GetObjectItemSet();
        const sal_uInt32 nRepeat((sal_uInt32)((SdrTextAniCountItem&)rSet.Get(SDRATTR_TEXT_ANICOUNT)).GetValue());
        bool bVisisbleWhenStopped(((SdrTextAniStopInsideItem&)rSet.Get(SDRATTR_TEXT_ANISTOPINSIDE)).GetValue());
        double fDelay((double)((SdrTextAniDelayItem&)rSet.Get(SDRATTR_TEXT_ANIDELAY)).GetValue());

        if(0.0 == fDelay)
        {
            
            fDelay = 250.0;
        }

        
        drawinglayer::animation::AnimationEntryLoop  aLoop(nRepeat ? nRepeat : ENDLESS_LOOP);
        drawinglayer::animation::AnimationEntryFixed aStart(fDelay, 0.0);
        aLoop.append(aStart);
        drawinglayer::animation::AnimationEntryFixed aEnd(fDelay, 1.0);
        aLoop.append(aEnd);
        rAnimList.append(aLoop);

        
        if(0L != nRepeat)
        {
            drawinglayer::animation::AnimationEntryFixed aStop(ENDLESS_TIME, bVisisbleWhenStopped ? 0.0 : 1.0);
            rAnimList.append(aStop);
        }
    }
}

void impCreateScrollTiming(const SfxItemSet& rSet, drawinglayer::animation::AnimationEntryList& rAnimList, bool bForward, double fTimeFullPath, double fFrequency)
{
    bool bVisisbleWhenStopped(((SdrTextAniStopInsideItem&)rSet.Get(SDRATTR_TEXT_ANISTOPINSIDE)).GetValue());
    bool bVisisbleWhenStarted(((SdrTextAniStartInsideItem&)rSet.Get(SDRATTR_TEXT_ANISTOPINSIDE )).GetValue());
    const sal_uInt32 nRepeat(((SdrTextAniCountItem&)rSet.Get(SDRATTR_TEXT_ANICOUNT)).GetValue());

    if(bVisisbleWhenStarted)
    {
        
        drawinglayer::animation::AnimationEntryLinear aInOut(fTimeFullPath * 0.5, fFrequency, 0.5, bForward ? 1.0 : 0.0);
        rAnimList.append(aInOut);
    }

    
    if(nRepeat || 0L == nRepeat)
    {
        drawinglayer::animation::AnimationEntryLoop aLoop(nRepeat ? nRepeat : ENDLESS_LOOP);
        drawinglayer::animation::AnimationEntryLinear aThrough(fTimeFullPath, fFrequency, bForward ? 0.0 : 1.0, bForward ? 1.0 : 0.0);
        aLoop.append(aThrough);
        rAnimList.append(aLoop);
    }

    if(0L != nRepeat && bVisisbleWhenStopped)
    {
        
        drawinglayer::animation::AnimationEntryLinear aOutIn(fTimeFullPath * 0.5, fFrequency, bForward ? 0.0 : 1.0, 0.5);
        rAnimList.append(aOutIn);

        
        drawinglayer::animation::AnimationEntryFixed aEnd(ENDLESS_TIME, 0.5);
        rAnimList.append(aEnd);
    }
}

void impCreateAlternateTiming(const SfxItemSet& rSet, drawinglayer::animation::AnimationEntryList& rAnimList, double fRelativeTextLength, bool bForward, double fTimeFullPath, double fFrequency)
{
    if(basegfx::fTools::more(fRelativeTextLength, 0.5))
    {
        
        
        bForward = !bForward;
    }

    const double fStartPosition(bForward ? fRelativeTextLength : 1.0 - fRelativeTextLength);
    const double fEndPosition(bForward ? 1.0 - fRelativeTextLength : fRelativeTextLength);
    bool bVisisbleWhenStopped(((SdrTextAniStopInsideItem&)rSet.Get(SDRATTR_TEXT_ANISTOPINSIDE)).GetValue());
    bool bVisisbleWhenStarted(((SdrTextAniStartInsideItem&)rSet.Get(SDRATTR_TEXT_ANISTOPINSIDE )).GetValue());
    const sal_uInt32 nRepeat(((SdrTextAniCountItem&)rSet.Get(SDRATTR_TEXT_ANICOUNT)).GetValue());

    if(!bVisisbleWhenStarted)
    {
        
        drawinglayer::animation::AnimationEntryLinear aOutIn(fTimeFullPath * 0.5, fFrequency, bForward ? 0.0 : 1.0, 0.5);
        rAnimList.append(aOutIn);
    }

    
    
    const double fInnerMovePath(fabs(1.0 - (fRelativeTextLength * 2.0)));
    const double fTimeForInnerPath(fTimeFullPath * fInnerMovePath);
    const double fHalfInnerPath(fTimeForInnerPath * 0.5);
    const sal_uInt32 nDoubleRepeat(nRepeat / 2L);

    if(nDoubleRepeat || 0L == nRepeat)
    {
        
        drawinglayer::animation::AnimationEntryLoop aLoop(nDoubleRepeat ? nDoubleRepeat : ENDLESS_LOOP);
        drawinglayer::animation::AnimationEntryLinear aTime0(fHalfInnerPath, fFrequency, 0.5, fEndPosition);
        aLoop.append(aTime0);
        drawinglayer::animation::AnimationEntryLinear aTime1(fTimeForInnerPath, fFrequency, fEndPosition, fStartPosition);
        aLoop.append(aTime1);
        drawinglayer::animation::AnimationEntryLinear aTime2(fHalfInnerPath, fFrequency, fStartPosition, 0.5);
        aLoop.append(aTime2);
        rAnimList.append(aLoop);
    }

    if(nRepeat % 2L)
    {
        
        drawinglayer::animation::AnimationEntryLinear aTime0(fHalfInnerPath, fFrequency, 0.5, fEndPosition);
        rAnimList.append(aTime0);
        drawinglayer::animation::AnimationEntryLinear aTime1(fHalfInnerPath, fFrequency, fEndPosition, 0.5);
        rAnimList.append(aTime1);
    }

    if(0L != nRepeat)
    {
        if(bVisisbleWhenStopped)
        {
            
            drawinglayer::animation::AnimationEntryFixed aEnd(ENDLESS_TIME, 0.5);
            rAnimList.append(aEnd);
        }
        else
        {
            
            drawinglayer::animation::AnimationEntryLinear aInOut(fTimeFullPath * 0.5, fFrequency, 0.5, bForward ? 1.0 : 0.0);
            rAnimList.append(aInOut);
        }
    }
}

void impCreateSlideTiming(const SfxItemSet& rSet, drawinglayer::animation::AnimationEntryList& rAnimList, bool bForward, double fTimeFullPath, double fFrequency)
{
    
    const double fStartPosition(bForward ? 0.0 : 1.0);
    const sal_uInt32 nRepeat(((SdrTextAniCountItem&)rSet.Get(SDRATTR_TEXT_ANICOUNT)).GetValue());

    
    drawinglayer::animation::AnimationEntryLinear aOutIn(fTimeFullPath * 0.5, fFrequency, fStartPosition, 0.5);
    rAnimList.append(aOutIn);

    
    if(nRepeat > 1L || 0L == nRepeat)
    {
        drawinglayer::animation::AnimationEntryLoop aLoop(nRepeat ? nRepeat - 1L : ENDLESS_LOOP);
        drawinglayer::animation::AnimationEntryLinear aTime0(fTimeFullPath * 0.5, fFrequency, 0.5, fStartPosition);
        aLoop.append(aTime0);
        drawinglayer::animation::AnimationEntryLinear aTime1(fTimeFullPath * 0.5, fFrequency, fStartPosition, 0.5);
        aLoop.append(aTime1);
        rAnimList.append(aLoop);
    }

    
    if(0L != nRepeat)
    {
        drawinglayer::animation::AnimationEntryFixed aEnd(ENDLESS_TIME, 0.5);
        rAnimList.append(aEnd);
    }
}

void SdrTextObj::impGetScrollTextTiming(drawinglayer::animation::AnimationEntryList& rAnimList, double fFrameLength, double fTextLength) const
{
    const SdrTextAniKind eAniKind(GetTextAniKind());

    if(SDRTEXTANI_SCROLL == eAniKind || SDRTEXTANI_ALTERNATE == eAniKind || SDRTEXTANI_SLIDE == eAniKind)
    {
        
        
        const SfxItemSet& rSet = GetObjectItemSet();
        double fAnimationDelay((double)((SdrTextAniDelayItem&)rSet.Get(SDRATTR_TEXT_ANIDELAY)).GetValue());
        double fSingleStepWidth((double)((SdrTextAniAmountItem&)rSet.Get(SDRATTR_TEXT_ANIAMOUNT)).GetValue());
        const SdrTextAniDirection eDirection(GetTextAniDirection());
        const bool bForward(SDRTEXTANI_RIGHT == eDirection || SDRTEXTANI_DOWN == eDirection);

        if(basegfx::fTools::equalZero(fAnimationDelay))
        {
            
            fAnimationDelay = 50.0;
        }

        if(basegfx::fTools::less(fSingleStepWidth, 0.0))
        {
            
            
            
            fSingleStepWidth = (-fSingleStepWidth * (2540.0 / PIXEL_DPI));
        }

        if(basegfx::fTools::equalZero(fSingleStepWidth))
        {
            
            fSingleStepWidth = 100.0;
        }

        
        
        const double fFullPathLength(fFrameLength + fTextLength);
        const double fNumberOfSteps(fFullPathLength / fSingleStepWidth);
        double fTimeFullPath(fNumberOfSteps * fAnimationDelay);

        if(fTimeFullPath < fAnimationDelay)
        {
            fTimeFullPath = fAnimationDelay;
        }

        switch(eAniKind)
        {
            case SDRTEXTANI_SCROLL :
            {
                impCreateScrollTiming(rSet, rAnimList, bForward, fTimeFullPath, fAnimationDelay);
                break;
            }
            case SDRTEXTANI_ALTERNATE :
            {
                double fRelativeTextLength(fTextLength / (fFrameLength + fTextLength));
                impCreateAlternateTiming(rSet, rAnimList, fRelativeTextLength, bForward, fTimeFullPath, fAnimationDelay);
                break;
            }
            case SDRTEXTANI_SLIDE :
            {
                impCreateSlideTiming(rSet, rAnimList, bForward, fTimeFullPath, fAnimationDelay);
                break;
            }
            default : break; 
        }
    }
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
