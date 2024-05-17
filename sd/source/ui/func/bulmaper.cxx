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

#include <editeng/editids.hrc>

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
#include <svl/itemset.hxx>
#include <svl/itempool.hxx>
#include <editeng/numitem.hxx>

#include <bulmaper.hxx>

#define GetWhichIDFromSlotID(nSlot) rSet.GetPool()->GetWhichIDFromSlotID( nSlot )

void SdBulletMapper::MapFontsInNumRule( SvxNumRule& aNumRule, const SfxItemSet& rSet )
{
    const sal_uInt16 nCount = aNumRule.GetLevelCount();
    for( sal_uInt16 nLevel = 0; nLevel < nCount; nLevel++ )
    {
        const SvxNumberFormat& rSrcLevel = aNumRule.GetLevel(nLevel);
        SvxNumberFormat aNewLevel( rSrcLevel );

        if(rSrcLevel.GetNumberingType() != css::style::NumberingType::CHAR_SPECIAL &&
           rSrcLevel.GetNumberingType() != css::style::NumberingType::NUMBER_NONE )
        {
            // if enumeration instead bullet is chosen, adjust bullet font to template font

            // to be implemented if module supports CJK

            vcl::Font aMyFont;
            const SvxFontItem& rFItem = rSet.Get(GetWhichIDFromSlotID( SID_ATTR_CHAR_FONT ));
            aMyFont.SetFamily(rFItem.GetFamily());
            aMyFont.SetFamilyName(rFItem.GetFamilyName());
            aMyFont.SetCharSet(rFItem.GetCharSet());
            aMyFont.SetPitch(rFItem.GetPitch());

            const SvxFontHeightItem& rFHItem = rSet.Get(GetWhichIDFromSlotID( SID_ATTR_CHAR_FONTHEIGHT ));
            aMyFont.SetFontSize(Size(0, rFHItem.GetHeight()));

            const SvxWeightItem& rWItem = rSet.Get(GetWhichIDFromSlotID( SID_ATTR_CHAR_WEIGHT ));
            aMyFont.SetWeight(rWItem.GetWeight());

            const SvxPostureItem& rPItem = rSet.Get(GetWhichIDFromSlotID(SID_ATTR_CHAR_POSTURE));
            aMyFont.SetItalic(rPItem.GetPosture());

            const SvxUnderlineItem& rUItem = rSet.Get(GetWhichIDFromSlotID(SID_ATTR_CHAR_UNDERLINE));
            aMyFont.SetUnderline(rUItem.GetLineStyle());

            const SvxOverlineItem& rOItem = rSet.Get(GetWhichIDFromSlotID(SID_ATTR_CHAR_OVERLINE));
            aMyFont.SetOverline(rOItem.GetLineStyle());

            const SvxCrossedOutItem& rCOItem = rSet.Get(GetWhichIDFromSlotID(SID_ATTR_CHAR_STRIKEOUT));
            aMyFont.SetStrikeout(rCOItem.GetStrikeout());

            const SvxContourItem& rCItem = rSet.Get(GetWhichIDFromSlotID(SID_ATTR_CHAR_CONTOUR));
            aMyFont.SetOutline(rCItem.GetValue());

            const SvxShadowedItem& rSItem = rSet.Get(GetWhichIDFromSlotID(SID_ATTR_CHAR_SHADOWED));
            aMyFont.SetShadow(rSItem.GetValue());

            aNewLevel.SetBulletFont(&aMyFont);
            aNumRule.SetLevel(nLevel, aNewLevel );
        }
        else if( rSrcLevel.GetNumberingType() == css::style::NumberingType::CHAR_SPECIAL )
        {
            aNewLevel.SetListFormat(u""_ustr, u""_ustr, nLevel);
            aNumRule.SetLevel(nLevel, aNewLevel );
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
