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


#include <svx/svxids.hrc>

//-> Fonts & Items
#include <vcl/font.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/crossedoutitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/contouritem.hxx>
#include <editeng/shdditem.hxx>

//<- Fonts & Items
#include <editeng/bulletitem.hxx>
#include <editeng/brushitem.hxx>
#include <vcl/graph.hxx>
#include <svl/itemset.hxx>
#include <svl/itempool.hxx>
#include <editeng/numitem.hxx>
#include <editeng/eeitem.hxx>

#include "bulmaper.hxx"


#define GetWhich(nSlot) rSet.GetPool()->GetWhich( nSlot )

void SdBulletMapper::MapFontsInNumRule( SvxNumRule& aNumRule, const SfxItemSet& rSet )
{
    const sal_uInt16 nCount = aNumRule.GetLevelCount();
    for( sal_uInt16 nLevel = 0; nLevel < nCount; nLevel++ )
    {
        const SvxNumberFormat& rSrcLevel = aNumRule.GetLevel(nLevel);
        SvxNumberFormat aNewLevel( rSrcLevel );

        if(rSrcLevel.GetNumberingType() != com::sun::star::style::NumberingType::CHAR_SPECIAL &&
           rSrcLevel.GetNumberingType() != com::sun::star::style::NumberingType::NUMBER_NONE )
        {
            // if enumeration instead bullet is chosen, adjust bullet font to template font

            // to be implemented if module supports CJK
            long nFontID = SID_ATTR_CHAR_FONT;
            long nFontHeightID = SID_ATTR_CHAR_FONTHEIGHT;
            long nWeightID = SID_ATTR_CHAR_WEIGHT;
            long nPostureID = SID_ATTR_CHAR_POSTURE;

            Font aMyFont;
            const SvxFontItem& rFItem =
                (SvxFontItem&)rSet.Get(GetWhich( (sal_uInt16)nFontID ));
            aMyFont.SetFamily(rFItem.GetFamily());
            aMyFont.SetName(rFItem.GetFamilyName());
            aMyFont.SetCharSet(rFItem.GetCharSet());
            aMyFont.SetPitch(rFItem.GetPitch());

            const SvxFontHeightItem& rFHItem =
                (SvxFontHeightItem&)rSet.Get(GetWhich( (sal_uInt16)nFontHeightID ));
            aMyFont.SetSize(Size(0, rFHItem.GetHeight()));

            const SvxWeightItem& rWItem =
                (SvxWeightItem&)rSet.Get(GetWhich( (sal_uInt16)nWeightID ));
            aMyFont.SetWeight(rWItem.GetWeight());

            const SvxPostureItem& rPItem =
                (SvxPostureItem&)rSet.Get(GetWhich( (sal_uInt16)nPostureID ));
            aMyFont.SetItalic(rPItem.GetPosture());

            const SvxUnderlineItem& rUItem = (SvxUnderlineItem&)rSet.Get(GetWhich(SID_ATTR_CHAR_UNDERLINE));
            aMyFont.SetUnderline(rUItem.GetLineStyle());

            const SvxOverlineItem& rOItem = (SvxOverlineItem&)rSet.Get(GetWhich(SID_ATTR_CHAR_OVERLINE));
            aMyFont.SetOverline(rOItem.GetLineStyle());

            const SvxCrossedOutItem& rCOItem = (SvxCrossedOutItem&)rSet.Get(GetWhich(SID_ATTR_CHAR_STRIKEOUT));
            aMyFont.SetStrikeout(rCOItem.GetStrikeout());

            const SvxContourItem& rCItem = (SvxContourItem&)rSet.Get(GetWhich(SID_ATTR_CHAR_CONTOUR));
            aMyFont.SetOutline(rCItem.GetValue());

            const SvxShadowedItem& rSItem = (SvxShadowedItem&)rSet.Get(GetWhich(SID_ATTR_CHAR_SHADOWED));
            aMyFont.SetShadow(rSItem.GetValue());

            aNewLevel.SetBulletFont(&aMyFont);
            aNumRule.SetLevel(nLevel, aNewLevel );
        }
        else if( rSrcLevel.GetNumberingType() == com::sun::star::style::NumberingType::CHAR_SPECIAL )
        {
            aNewLevel.SetPrefix("");
            aNewLevel.SetSuffix("");
            aNumRule.SetLevel(nLevel, aNewLevel );
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
