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

#include <svx/sdr/overlay/overlayselection.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <drawinglayer/primitive2d/polypolygonprimitive2d.hxx>
#include <svtools/optionsdrawinglayer.hxx>
#include <vcl/svapp.hxx>
#include <vcl/outdev.hxx>
#include <vcl/settings.hxx>
#include <drawinglayer/primitive2d/invertprimitive2d.hxx>
#include <drawinglayer/primitive2d/unifiedtransparenceprimitive2d.hxx>
#include <basegfx/polygon/b2dpolypolygoncutter.hxx>
#include <svx/sdr/overlay/overlaymanager.hxx>



namespace sdr
{
    namespace overlay
    {
        
        basegfx::B2DPolyPolygon impCombineRangesToPolyPolygon(const std::vector< basegfx::B2DRange >& rRanges)
        {
            const sal_uInt32 nCount(rRanges.size());
            basegfx::B2DPolyPolygon aRetval;

            for(sal_uInt32 a(0); a < nCount; a++)
            {
                const basegfx::B2DPolygon aDiscretePolygon(basegfx::tools::createPolygonFromRect(rRanges[a]));

                if(0 == a)
                {
                    aRetval.append(aDiscretePolygon);
                }
                else
                {
                    aRetval = basegfx::tools::solvePolygonOperationOr(aRetval, basegfx::B2DPolyPolygon(aDiscretePolygon));
                }
            }

            return aRetval;
        }

        
        
        OverlayType impCheckPossibleOverlayType(OverlayType aOverlayType)
        {
            if(OVERLAY_INVERT != aOverlayType)
            {
                const SvtOptionsDrawinglayer aSvtOptionsDrawinglayer;

                if(!aSvtOptionsDrawinglayer.IsTransparentSelection())
                {
                    
                    return OVERLAY_INVERT;
                }
                else
                {
                    const OutputDevice *pOut = Application::GetDefaultDevice();

                    if(pOut->GetSettings().GetStyleSettings().GetHighContrastMode())
                    {
                        
                        return  OVERLAY_INVERT;
                    }

                    if(!pOut->supportsOperation(OutDevSupport_TransparentRect))
                    {
                        
                        return OVERLAY_INVERT;
                    }
                }
            }

            return aOverlayType;
        }

        drawinglayer::primitive2d::Primitive2DSequence OverlaySelection::createOverlayObjectPrimitive2DSequence()
        {
            drawinglayer::primitive2d::Primitive2DSequence aRetval;
            const sal_uInt32 nCount(getRanges().size());

            if(nCount)
            {
                
                const bool bInvert(OVERLAY_INVERT == maLastOverlayType);
                basegfx::BColor aRGBColor(getBaseColor().getBColor());
                aRetval.realloc(nCount);

                if(bInvert)
                {
                    
                    aRGBColor = basegfx::BColor(1.0, 1.0, 1.0);
                }

                for(sal_uInt32 a(0);a < nCount; a++)
                {
                    const basegfx::B2DPolygon aPolygon(basegfx::tools::createPolygonFromRect(maRanges[a]));
                    aRetval[a] = drawinglayer::primitive2d::Primitive2DReference(
                        new drawinglayer::primitive2d::PolyPolygonColorPrimitive2D(
                            basegfx::B2DPolyPolygon(aPolygon),
                            aRGBColor));
                }

                if(bInvert)
                {
                    
                    const drawinglayer::primitive2d::Primitive2DReference aInvert(
                        new drawinglayer::primitive2d::InvertPrimitive2D(
                            aRetval));
                    aRetval = drawinglayer::primitive2d::Primitive2DSequence(&aInvert, 1);
                }
                else if(OVERLAY_TRANSPARENT == maLastOverlayType)
                {
                    
                    const double fTransparence(mnLastTransparence / 100.0);
                    const drawinglayer::primitive2d::Primitive2DReference aUnifiedTransparence(
                        new drawinglayer::primitive2d::UnifiedTransparencePrimitive2D(
                            aRetval,
                            fTransparence));

                    if(getBorder())
                    {
                        const basegfx::B2DPolyPolygon aPolyPolygon(impCombineRangesToPolyPolygon(getRanges()));
                        const drawinglayer::primitive2d::Primitive2DReference aSelectionOutline(
                            new drawinglayer::primitive2d::PolyPolygonHairlinePrimitive2D(
                                aPolyPolygon,
                                aRGBColor));

                        
                        aRetval.realloc(2);
                        aRetval[0] = aUnifiedTransparence;
                        aRetval[1] = aSelectionOutline;
                    }
                    else
                    {
                        
                        aRetval = drawinglayer::primitive2d::Primitive2DSequence(&aUnifiedTransparence, 1);
                    }
                }
            }

            return aRetval;
        }

        OverlaySelection::OverlaySelection(
            OverlayType eType,
            const Color& rColor,
            const std::vector< basegfx::B2DRange >& rRanges,
            bool bBorder)
        :   OverlayObject(rColor),
            meOverlayType(eType),
            maRanges(rRanges),
            maLastOverlayType(eType),
            mnLastTransparence(0),
            mbBorder(bBorder)
        {
            
            allowAntiAliase(false);
        }

        OverlaySelection::~OverlaySelection()
        {
            if(getOverlayManager())
            {
                getOverlayManager()->remove(*this);
            }
        }

        drawinglayer::primitive2d::Primitive2DSequence OverlaySelection::getOverlayObjectPrimitive2DSequence() const
        {
            
               const OverlayType aNewOverlayType(impCheckPossibleOverlayType(meOverlayType));
            const SvtOptionsDrawinglayer aSvtOptionsDrawinglayer;
            const sal_uInt16 nNewTransparence(aSvtOptionsDrawinglayer.GetTransparentSelectionPercent());

            if(getPrimitive2DSequence().hasElements())
            {
                if(aNewOverlayType != maLastOverlayType
                    || nNewTransparence != mnLastTransparence)
                {
                    
                    const_cast< OverlaySelection* >(this)->setPrimitive2DSequence(drawinglayer::primitive2d::Primitive2DSequence());
                }
            }

            if(!getPrimitive2DSequence().hasElements())
            {
                
                const_cast< OverlaySelection* >(this)->maLastOverlayType = aNewOverlayType;
                const_cast< OverlaySelection* >(this)->mnLastTransparence = nNewTransparence;
            }

            
            return OverlayObject::getOverlayObjectPrimitive2DSequence();
        }

        void OverlaySelection::setRanges(const std::vector< basegfx::B2DRange >& rNew)
        {
            if(rNew != maRanges)
            {
                maRanges = rNew;
                objectChange();
            }
        }
    } 
} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
