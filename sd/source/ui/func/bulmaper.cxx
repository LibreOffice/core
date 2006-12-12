/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: bulmaper.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-12 17:13:36 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#ifdef SD_DLLIMPLEMENTATION
#undef SD_DLLIMPLEMENTATION
#endif

#ifndef _SVX_SVXIDS_HRC
#include <svx/svxids.hrc>
#endif

#define ITEMID_FONT         SID_ATTR_CHAR_FONT
#define ITEMID_FONTHEIGHT   SID_ATTR_CHAR_FONTHEIGHT
#define ITEMID_COLOR        SID_ATTR_CHAR_COLOR
#define ITEMID_POSTURE      SID_ATTR_CHAR_POSTURE
#define ITEMID_WEIGHT       SID_ATTR_CHAR_WEIGHT
#define ITEMID_SHADOWED     SID_ATTR_CHAR_SHADOWED
#define ITEMID_CONTOUR      SID_ATTR_CHAR_CONTOUR
#define ITEMID_CROSSEDOUT   SID_ATTR_CHAR_STRIKEOUT
#define ITEMID_UNDERLINE    SID_ATTR_CHAR_UNDERLINE

//-> Fonts & Items
#ifndef _SV_FONT_HXX
#include <vcl/font.hxx>
#endif
#ifndef _SVX_FONTITEM_HXX
#include <svx/fontitem.hxx>
#endif
#ifndef _SVX_FHGTITEM_HXX //autogen
#include <svx/fhgtitem.hxx>
#endif
#ifndef _SVX_WGHTITEM_HXX //autogen
#include <svx/wghtitem.hxx>
#endif
#ifndef _SVX_UDLNITEM_HXX //autogen
#include <svx/udlnitem.hxx>
#endif
#ifndef _SVX_CRSDITEM_HXX //autogen
#include <svx/crsditem.hxx>
#endif
#ifndef _SVX_POSTITEM_HXX //autogen
#include <svx/postitem.hxx>
#endif
#ifndef _SVX_ITEM_HXX //autogen
#include <svx/cntritem.hxx>
#endif
#ifndef _SVX_SHDDITEM_HXX //autogen
#include <svx/shdditem.hxx>
#endif

//<- Fonts & Items

#ifndef _SVX_BULITEM_HXX
#include <svx/bulitem.hxx>
#endif
#define ITEMID_BRUSH    0
#ifndef _SVX_BRSHITEM_HXX
#include <svx/brshitem.hxx>
#endif
#ifndef _SV_GRAPH_HXX
#include <vcl/graph.hxx>
#endif
#ifndef _SFXITEMSET_HXX //autogen
#include <svtools/itemset.hxx>
#endif
#ifndef _SFXITEMPOOL_HXX //autogen
#include <svtools/itempool.hxx>
#endif
#include <svx/numitem.hxx>
#ifndef _EEITEM_HXX //autogen
#include <svx/eeitem.hxx>
#endif

#include "bulmaper.hxx"

#define GetWhich(nSlot) rSet.GetPool()->GetWhich( nSlot )

void SdBulletMapper::PreMapNumBulletForDialog( SfxItemSet& rSet )
{
    if( SFX_ITEM_SET == rSet.GetItemState( EE_PARA_NUMBULLET, FALSE ) )
    {
        SvxNumRule* pRule = ((SvxNumBulletItem*)rSet.GetItem( EE_PARA_NUMBULLET ))->GetNumRule();

        if(pRule && pRule->GetNumRuleType() == SVX_RULETYPE_PRESENTATION_NUMBERING)
        {
            // 10er Bullet Item auf 9er Item mappen
            SvxNumRule aNewRule( pRule->GetFeatureFlags(), 9, FALSE, SVX_RULETYPE_PRESENTATION_NUMBERING );

            for( USHORT i = 0; i < 9; i++ )
                aNewRule.SetLevel(i, pRule->GetLevel(i+1));

            rSet.Put( SvxNumBulletItem( aNewRule, EE_PARA_NUMBULLET ) );
        }
    }
}

void SdBulletMapper::PostMapNumBulletForDialog( SfxItemSet& rSet )
{
    if( SFX_ITEM_SET == rSet.GetItemState( EE_PARA_NUMBULLET, FALSE ) )
    {
        SvxNumRule* pRule = ((SvxNumBulletItem*)rSet.GetItem( EE_PARA_NUMBULLET ))->GetNumRule();

        if(pRule)
        {
            pRule->UnLinkGraphics();
            if(pRule->GetNumRuleType() == SVX_RULETYPE_PRESENTATION_NUMBERING)
            {
                // 9er Bullet Item auf 10er Item mappen
                SvxNumRule aNewRule( pRule->GetFeatureFlags(), 10, FALSE, SVX_RULETYPE_PRESENTATION_NUMBERING );

                for( USHORT i = 0; i < 9; i++ )
                    aNewRule.SetLevel(i+1, pRule->GetLevel(i));

                rSet.Put( SvxNumBulletItem( aNewRule, EE_PARA_NUMBULLET ) );
            }
        }
    }
}

void SdBulletMapper::MapFontsInNumRule( SvxNumRule& aNumRule, const SfxItemSet& rSet )
{
    const USHORT nCount = aNumRule.GetLevelCount();
    for( USHORT nLevel = 0; nLevel < nCount; nLevel++ )
    {
        const SvxNumberFormat& rSrcLevel = aNumRule.GetLevel(nLevel);
        SvxNumberFormat aNewLevel( rSrcLevel );

        if(rSrcLevel.GetNumberingType() != com::sun::star::style::NumberingType::CHAR_SPECIAL &&
           rSrcLevel.GetNumberingType() != com::sun::star::style::NumberingType::NUMBER_NONE )
        {
            // wenn Aufzaehlung statt Bullet gewaehlt wurde, wird der Bullet-Font
            // dem Vorlagen-Font angeglichen

            // to be implemented if module supports CJK
            long nFontID = ITEMID_FONT;
            long nFontHeightID = ITEMID_FONTHEIGHT;
            long nWeightID = ITEMID_WEIGHT;
            long nPostureID = ITEMID_POSTURE;

            if( 0 )
            {
                nFontID = EE_CHAR_FONTINFO_CJK;
                 nFontHeightID = EE_CHAR_FONTHEIGHT_CJK;
                nWeightID = EE_CHAR_WEIGHT_CJK;
                nPostureID = EE_CHAR_ITALIC_CJK;
            }
            else if( 0 )
            {
                nFontID = EE_CHAR_FONTINFO_CTL;
                 nFontHeightID = EE_CHAR_FONTHEIGHT_CTL;
                nWeightID = EE_CHAR_WEIGHT_CTL;
                nPostureID = EE_CHAR_ITALIC_CTL;
            }

            Font aMyFont;
            const SvxFontItem& rFItem =
                (SvxFontItem&)rSet.Get(GetWhich( (USHORT)nFontID ));
            aMyFont.SetFamily(rFItem.GetFamily());
            aMyFont.SetName(rFItem.GetFamilyName());
            aMyFont.SetCharSet(rFItem.GetCharSet());
            aMyFont.SetPitch(rFItem.GetPitch());

            const SvxFontHeightItem& rFHItem =
                (SvxFontHeightItem&)rSet.Get(GetWhich( (USHORT)nFontHeightID ));
            aMyFont.SetSize(Size(0, rFHItem.GetHeight()));

            const SvxWeightItem& rWItem =
                (SvxWeightItem&)rSet.Get(GetWhich( (USHORT)nWeightID ));
            aMyFont.SetWeight(rWItem.GetWeight());

            const SvxPostureItem& rPItem =
                (SvxPostureItem&)rSet.Get(GetWhich( (USHORT)nPostureID ));
            aMyFont.SetItalic(rPItem.GetPosture());

            const SvxUnderlineItem& rUItem = (SvxUnderlineItem&)rSet.Get(GetWhich(SID_ATTR_CHAR_UNDERLINE));
            aMyFont.SetUnderline(rUItem.GetUnderline());

            const SvxCrossedOutItem& rCOItem = (SvxCrossedOutItem&)rSet.Get(GetWhich(SID_ATTR_CHAR_STRIKEOUT));
            aMyFont.SetStrikeout(rCOItem.GetStrikeout());

            const SvxContourItem& rCItem = (SvxContourItem&)rSet.Get(GetWhich(SID_ATTR_CHAR_CONTOUR));
            aMyFont.SetOutline(rCItem.GetValue());

            const SvxShadowedItem& rSItem = (SvxShadowedItem&)rSet.Get(GetWhich(SID_ATTR_CHAR_SHADOWED));
            aMyFont.SetShadow(rSItem.GetValue());

            aNewLevel.SetBulletFont(&aMyFont);
//          aNewLevel.SetBulletRelSize( 75 );
            aNumRule.SetLevel(nLevel, aNewLevel );
        }
        else if( rSrcLevel.GetNumberingType() == com::sun::star::style::NumberingType::CHAR_SPECIAL )
        {
            String aEmpty;
            aNewLevel.SetPrefix( aEmpty );
            aNewLevel.SetSuffix( aEmpty );
            aNumRule.SetLevel(nLevel, aNewLevel );
        }
    }
}
