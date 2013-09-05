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


#include <com/sun/star/lang/DisposedException.hpp>
#include <editeng/eeitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/contouritem.hxx>
#include <editeng/shdditem.hxx>
#include <editeng/crossedoutitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/fontitem.hxx>
#include <svl/poolitem.hxx>
#include <svx/xfillit0.hxx>
#include <svx/xlineit0.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/numitem.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/editeng.hxx>
#include <svl/smplhint.hxx>
#include <editeng/langitem.hxx>
#include <editeng/charreliefitem.hxx>
#include <editeng/emphasismarkitem.hxx>
#include <svx/sdr/table/tabledesign.hxx>
#include <editeng/autokernitem.hxx>

#include <svx/svdattr.hxx>
#include <editeng/outliner.hxx>
#include <svx/xtable.hxx>           // for RGB_Color
#include <editeng/bulletitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/adjustitem.hxx>
#include <editeng/numdef.hxx>
#include <svl/itempool.hxx>

#include "stlpool.hxx"
#include "sdresid.hxx"
#include "stlsheet.hxx"
#include "glob.hrc"
#include "glob.hxx"
#include "drawdoc.hxx"
#include "sdmod.hxx"
#include "sdpage.hxx"
#include "helpids.h"
#include <svl/itemset.hxx>
#include "app.hrc"

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::style;
using namespace ::com::sun::star::container;

// ----------------------------------------------------------

SdStyleSheetPool::SdStyleSheetPool(SfxItemPool const& _rPool, SdDrawDocument* pDocument)
:   SdStyleSheetPoolBase( _rPool )
,   mpActualStyleSheet(NULL)
,   mpDoc(pDocument)
{
    if( mpDoc )
    {
        rtl::Reference< SfxStyleSheetPool > xPool( this );

        // create graphics family
        mxGraphicFamily = new SdStyleFamily( xPool, SD_STYLE_FAMILY_GRAPHICS );
        mxCellFamily = new SdStyleFamily( xPool, SD_STYLE_FAMILY_CELL );

        mxTableFamily = sdr::table::CreateTableDesignFamily();
        Reference< XNamed > xNamed( mxTableFamily, UNO_QUERY );
        if( xNamed.is() )
            msTableFamilyName = xNamed->getName();

        // create presentation families, one for each master page
        const sal_uInt16 nCount = mpDoc->GetMasterSdPageCount(PK_STANDARD);
        for( sal_uInt16 nPage = 0; nPage < nCount; ++nPage )
            AddStyleFamily( mpDoc->GetMasterSdPage(nPage,PK_STANDARD) );

    }
}

// ----------------------------------------------------------

SdStyleSheetPool::~SdStyleSheetPool()
{
    DBG_ASSERT( mpDoc == NULL, "sd::SdStyleSheetPool::~SdStyleSheetPool(), dispose me first!" );
}

// ----------------------------------------------------------

SfxStyleSheetBase* SdStyleSheetPool::Create(const OUString& rName, SfxStyleFamily eFamily, sal_uInt16 _nMask )
{
    return new SdStyleSheet(rName, *this, eFamily, _nMask);
}

// ----------------------------------------------------------

SfxStyleSheetBase* SdStyleSheetPool::Create(const SdStyleSheet& rStyle)
{
    return new SdStyleSheet( rStyle );
}

// ----------------------------------------------------------

SfxStyleSheetBase* SdStyleSheetPool::GetTitleSheet(const OUString& rLayoutName)
{
    OUString aName(rLayoutName);
    aName += SD_LT_SEPARATOR;
    aName += SD_RESSTR(STR_LAYOUT_TITLE);
    SfxStyleSheetBase* pResult = Find(aName, SD_STYLE_FAMILY_MASTERPAGE);
    return pResult;
}

/*************************************************************************
|*
|* Create a list of outline text templates for a presentation layout.
|* The caller has to delete the list.
|*
\************************************************************************/

void SdStyleSheetPool::CreateOutlineSheetList (const OUString& rLayoutName, std::vector<SfxStyleSheetBase*> &rOutlineStyles)
{
    OUString aName(rLayoutName);
    aName += SD_LT_SEPARATOR;
    aName += SD_RESSTR(STR_LAYOUT_OUTLINE);

    for (sal_Int32 nSheet = 1; nSheet < 10; nSheet++)
    {
        OUString aFullName(aName + " " + OUString::number( nSheet ) );
        SfxStyleSheetBase* pSheet = Find(aFullName, SD_STYLE_FAMILY_MASTERPAGE);

        if (pSheet)
            rOutlineStyles.push_back(pSheet);
    }
}

/*************************************************************************
|*
|* Create style sheets with default values for the named presentation layout
|*
\************************************************************************/

void SdStyleSheetPool::CreateLayoutStyleSheets(const OUString& rLayoutName, sal_Bool bCheck /*= sal_False*/ )
{
    const sal_uInt16 nUsedMask = SFXSTYLEBIT_ALL & ~SFXSTYLEBIT_USERDEF;

    (void)bCheck;
    sal_Bool bCreated = sal_False;

    SfxStyleSheetBase* pSheet = NULL;

    OUString aPrefix(rLayoutName + SD_LT_SEPARATOR);

    Font aLatinFont, aCJKFont, aCTLFont;

    mpDoc->getDefaultFonts( aLatinFont, aCJKFont, aCTLFont );

    // Font for title and outline
    SvxFontItem aSvxFontItem( aLatinFont.GetFamily(), aLatinFont.GetName(), aLatinFont.GetStyleName(), aLatinFont.GetPitch(),
                              aLatinFont.GetCharSet(), EE_CHAR_FONTINFO );

    SvxFontItem aSvxFontItemCJK( aCJKFont.GetFamily(), aCJKFont.GetName(), aCJKFont.GetStyleName(), aCJKFont.GetPitch(),
                                 aCJKFont.GetCharSet(), EE_CHAR_FONTINFO_CJK );

    SvxFontItem aSvxFontItemCTL( aCTLFont.GetFamily(), aCTLFont.GetName(), aCTLFont.GetStyleName(), aCTLFont.GetPitch(),
                                 aCTLFont.GetCharSet(), EE_CHAR_FONTINFO_CTL );

    Font aBulletFont( GetBulletFont() );

    /**************************************************************************
    * outline levels
    **************************************************************************/
    OUString aName(SD_RESSTR(STR_LAYOUT_OUTLINE));
    OUString aHelpFile;

    SfxStyleSheetBase* pParent = NULL;
    SvxLRSpaceItem aSvxLRSpaceItem( EE_PARA_LRSPACE );
    SvxULSpaceItem aSvxULSpaceItem( EE_PARA_ULSPACE );

    for( sal_Int32 nLevel = 1; nLevel < 10; nLevel++)
    {
        OUString aLevelName( aPrefix + aName + " " + OUString::number( nLevel ) ) ;

        if (!Find(aLevelName, SD_STYLE_FAMILY_MASTERPAGE))
        {
            bCreated = sal_True;
            pSheet = &Make(aLevelName, SD_STYLE_FAMILY_MASTERPAGE,nUsedMask);
            pSheet->SetHelpId( aHelpFile, HID_PSEUDOSHEET_OUTLINE + nLevel );

            pSheet->SetParent( OUString() );

            // attributing for level 1, the others levels inherit
            if (nLevel == 1)
            {
                SfxItemSet&     rSet = pSheet->GetItemSet();

                rSet.Put(aSvxFontItem);
                rSet.Put(aSvxFontItemCJK);
                rSet.Put(aSvxFontItemCTL);
                rSet.Put( SvxPostureItem( ITALIC_NONE, EE_CHAR_ITALIC ) );
                rSet.Put( SvxPostureItem( ITALIC_NONE, EE_CHAR_ITALIC_CJK ) );
                rSet.Put( SvxPostureItem( ITALIC_NONE, EE_CHAR_ITALIC_CTL ) );
                rSet.Put( SvxWeightItem( WEIGHT_NORMAL, EE_CHAR_WEIGHT ) );
                rSet.Put( SvxWeightItem( WEIGHT_NORMAL, EE_CHAR_WEIGHT_CJK ) );
                rSet.Put( SvxWeightItem( WEIGHT_NORMAL, EE_CHAR_WEIGHT_CTL ) );
                rSet.Put( SvxUnderlineItem(UNDERLINE_NONE, EE_CHAR_UNDERLINE ) );
                rSet.Put( SvxOverlineItem(UNDERLINE_NONE, EE_CHAR_OVERLINE ) );
                rSet.Put( SvxCrossedOutItem(STRIKEOUT_NONE, EE_CHAR_STRIKEOUT ) );
                rSet.Put( SvxShadowedItem(sal_False, EE_CHAR_SHADOW ) );
                rSet.Put( SvxContourItem(sal_False, EE_CHAR_OUTLINE ) );
                rSet.Put( SvxEmphasisMarkItem(EMPHASISMARK_NONE, EE_CHAR_EMPHASISMARK ) );
                rSet.Put( SvxCharReliefItem(RELIEF_NONE, EE_CHAR_RELIEF) );
                rSet.Put( SvxColorItem( Color(COL_AUTO), EE_CHAR_COLOR) );
                rSet.Put( XLineStyleItem(XLINE_NONE) );
                rSet.Put( XFillStyleItem(XFILL_NONE) );
                rSet.Put( SdrTextFitToSizeTypeItem(SDRTEXTFIT_AUTOFIT) );
                rSet.Put( SdrTextAutoGrowHeightItem(sal_False) );
                // #i16874# enable kerning by default but only for new documents
                rSet.Put( SvxAutoKernItem( sal_True, EE_CHAR_PAIRKERNING ) );

                if( nLevel == 1 )
                {
                    Font f( GetBulletFont() );
                    PutNumBulletItem( pSheet, f );
                }
            }

            sal_uLong nFontSize = 20;
            sal_uInt16 nLower = 100;

            switch (nLevel)
            {
                case 1:
                {
                    nFontSize = 32;
                    nLower = 500;
                }
                break;

                case 2:
                {
                    nFontSize = 28;
                    nLower = 400;
                }
                break;

                case 3:
                {
                    nFontSize = 24;
                    nLower = 300;
                }
                break;

                case 4:
                {
                    nLower = 200;
                }
                break;
            }

            // FontSize
            nFontSize = (sal_uInt16)((nFontSize * 2540L) / 72);  // Pt --> 1/100 mm
            SfxItemSet& rOutlineSet = pSheet->GetItemSet();
            rOutlineSet.Put( SvxFontHeightItem( nFontSize, 100, EE_CHAR_FONTHEIGHT ) );
            rOutlineSet.Put( SvxFontHeightItem( nFontSize, 100, EE_CHAR_FONTHEIGHT_CJK ) );
            rOutlineSet.Put( SvxFontHeightItem( SdDrawDocument::convertFontHeightToCTL( nFontSize ), 100, EE_CHAR_FONTHEIGHT_CTL ) );

            // Line distance (downwards). Stuff around here cleaned up in i35937
            aSvxULSpaceItem.SetLower(nLower);
            pSheet->GetItemSet().Put(aSvxULSpaceItem);
        }
    }

    // if we created outline styles, we need to chain them
    if( bCreated )
    {
        pParent = NULL;
        for (sal_Int32 nLevel = 1; nLevel < 10; nLevel++)
        {
            OUString aLevelName( aPrefix + aName + " " + OUString::number( nLevel ) );

            pSheet = Find(aLevelName, SD_STYLE_FAMILY_MASTERPAGE);

            DBG_ASSERT( pSheet, "missing layout style!");

            if( pSheet )
            {
                if (pParent)
                    pSheet->SetParent(pParent->GetName());
                pParent = pSheet;
            }
        }
    }

    /**************************************************************************
    * Title
    **************************************************************************/
    aName = aPrefix + SD_RESSTR(STR_LAYOUT_TITLE);

    if (!Find(aName, SD_STYLE_FAMILY_MASTERPAGE))
    {
        bCreated = sal_True;

        pSheet = &Make(aName, SD_STYLE_FAMILY_MASTERPAGE,nUsedMask);
        pSheet->SetHelpId( aHelpFile, HID_PSEUDOSHEET_TITLE );
        pSheet->SetParent( OUString() );
        SfxItemSet& rTitleSet = pSheet->GetItemSet();
        rTitleSet.Put(XLineStyleItem(XLINE_NONE));
        rTitleSet.Put(XFillStyleItem(XFILL_NONE));
        rTitleSet.Put(aSvxFontItem);
        rTitleSet.Put(aSvxFontItemCJK);
        rTitleSet.Put(aSvxFontItemCTL);
        rTitleSet.Put(SvxPostureItem( ITALIC_NONE, EE_CHAR_ITALIC ) );
        rTitleSet.Put(SvxPostureItem( ITALIC_NONE, EE_CHAR_ITALIC_CJK ) );
        rTitleSet.Put(SvxPostureItem( ITALIC_NONE, EE_CHAR_ITALIC_CTL ) );
        rTitleSet.Put(SvxWeightItem( WEIGHT_NORMAL, EE_CHAR_WEIGHT ) );
        rTitleSet.Put(SvxWeightItem( WEIGHT_NORMAL, EE_CHAR_WEIGHT_CJK ) );
        rTitleSet.Put(SvxWeightItem( WEIGHT_NORMAL, EE_CHAR_WEIGHT_CTL ) );
        rTitleSet.Put(SvxFontHeightItem( 1552, 100, EE_CHAR_FONTHEIGHT ) );                 // 44 pt
        rTitleSet.Put(SvxFontHeightItem( 1552, 100, EE_CHAR_FONTHEIGHT_CJK ) );                 // 44 pt
        rTitleSet.Put(SvxFontHeightItem( SdDrawDocument::convertFontHeightToCTL( 1552 ), 100, EE_CHAR_FONTHEIGHT_CTL ) );                   // 44 pt
        rTitleSet.Put(SvxUnderlineItem(UNDERLINE_NONE, EE_CHAR_UNDERLINE ));
        rTitleSet.Put(SvxOverlineItem(UNDERLINE_NONE, EE_CHAR_OVERLINE ));
        rTitleSet.Put(SvxCrossedOutItem(STRIKEOUT_NONE, EE_CHAR_STRIKEOUT ));
        rTitleSet.Put(SvxShadowedItem(sal_False, EE_CHAR_SHADOW ));
        rTitleSet.Put(SvxContourItem(sal_False, EE_CHAR_OUTLINE ));
        rTitleSet.Put( SvxEmphasisMarkItem(EMPHASISMARK_NONE, EE_CHAR_EMPHASISMARK ) );
        rTitleSet.Put( SvxCharReliefItem(RELIEF_NONE, EE_CHAR_RELIEF ) );
        rTitleSet.Put(SvxColorItem( Color(COL_AUTO), EE_CHAR_COLOR ));
        rTitleSet.Put(SvxAdjustItem(SVX_ADJUST_CENTER, EE_PARA_JUST ));
        rTitleSet.Put( SdrTextVertAdjustItem( SDRTEXTVERTADJUST_CENTER ) );
        // #i16874# enable kerning by default but only for new documents
        rTitleSet.Put( SvxAutoKernItem( sal_True, EE_CHAR_PAIRKERNING ) );

        aBulletFont.SetSize(Size(0,1552));                  // 44 pt
        PutNumBulletItem( pSheet, aBulletFont );
    }

    /**************************************************************************
    * Subtitle
    **************************************************************************/
    aName = aPrefix + SD_RESSTR(STR_LAYOUT_SUBTITLE);

    if (!Find(aName, SD_STYLE_FAMILY_MASTERPAGE))
    {
        bCreated = sal_True;

        pSheet = &Make(aName, SD_STYLE_FAMILY_MASTERPAGE,nUsedMask);
        pSheet->SetHelpId( aHelpFile, HID_PSEUDOSHEET_SUBTITLE );
        pSheet->SetParent( OUString() );
        SfxItemSet& rSubtitleSet = pSheet->GetItemSet();
        rSubtitleSet.Put(XLineStyleItem(XLINE_NONE));
        rSubtitleSet.Put(XFillStyleItem(XFILL_NONE));
        rSubtitleSet.Put(aSvxFontItem);
        rSubtitleSet.Put(aSvxFontItemCJK);
        rSubtitleSet.Put(aSvxFontItemCTL);
        rSubtitleSet.Put(SvxPostureItem( ITALIC_NONE, EE_CHAR_ITALIC ) );
        rSubtitleSet.Put(SvxPostureItem( ITALIC_NONE, EE_CHAR_ITALIC_CJK ) );
        rSubtitleSet.Put(SvxPostureItem( ITALIC_NONE, EE_CHAR_ITALIC_CTL ) );
        rSubtitleSet.Put(SvxWeightItem( WEIGHT_NORMAL, EE_CHAR_WEIGHT ) );
        rSubtitleSet.Put(SvxWeightItem( WEIGHT_NORMAL, EE_CHAR_WEIGHT_CJK ) );
        rSubtitleSet.Put(SvxWeightItem( WEIGHT_NORMAL, EE_CHAR_WEIGHT_CTL ) );
        rSubtitleSet.Put( SvxFontHeightItem( 1129, 100, EE_CHAR_FONTHEIGHT ) );     // 32 pt
        rSubtitleSet.Put( SvxFontHeightItem( 1129, 100, EE_CHAR_FONTHEIGHT_CJK ) ); // 32 pt
        rSubtitleSet.Put( SvxFontHeightItem( SdDrawDocument::convertFontHeightToCTL( 1129 ), 100, EE_CHAR_FONTHEIGHT_CTL ) ); // 32 pt
        rSubtitleSet.Put(SvxUnderlineItem(UNDERLINE_NONE, EE_CHAR_UNDERLINE ));
        rSubtitleSet.Put(SvxOverlineItem(UNDERLINE_NONE, EE_CHAR_OVERLINE ));
        rSubtitleSet.Put(SvxCrossedOutItem(STRIKEOUT_NONE, EE_CHAR_STRIKEOUT ));
        rSubtitleSet.Put(SvxShadowedItem(sal_False, EE_CHAR_SHADOW ));
        rSubtitleSet.Put(SvxContourItem(sal_False, EE_CHAR_OUTLINE ));
        rSubtitleSet.Put( SvxEmphasisMarkItem(EMPHASISMARK_NONE, EE_CHAR_EMPHASISMARK ) );
        rSubtitleSet.Put( SvxCharReliefItem(RELIEF_NONE, EE_CHAR_RELIEF ) );
        rSubtitleSet.Put(SvxColorItem( Color(COL_AUTO), EE_CHAR_COLOR ));
        rSubtitleSet.Put(SvxAdjustItem(SVX_ADJUST_CENTER, EE_PARA_JUST ));
        rSubtitleSet.Put( SdrTextVertAdjustItem( SDRTEXTVERTADJUST_CENTER ) );
        // #i16874# enable kerning by default but only for new documents
        rSubtitleSet.Put( SvxAutoKernItem( sal_True, EE_CHAR_PAIRKERNING ) );
        aSvxLRSpaceItem.SetTxtLeft(0);
        rSubtitleSet.Put(aSvxLRSpaceItem);

        Font aTmpFont( GetBulletFont() );
        aTmpFont.SetSize(Size(0, 1129));        // 32 pt
        PutNumBulletItem( pSheet, aTmpFont );
    }

    /**************************************************************************
    * Notes
    **************************************************************************/
    aName = aPrefix + SD_RESSTR(STR_LAYOUT_NOTES);

    if (!Find(aName, SD_STYLE_FAMILY_MASTERPAGE))
    {
        bCreated = sal_True;

        pSheet = &Make(aName, SD_STYLE_FAMILY_MASTERPAGE,nUsedMask);
        pSheet->SetHelpId( aHelpFile, HID_PSEUDOSHEET_NOTES );
        pSheet->SetParent( OUString() );
        SfxItemSet& rNotesSet = pSheet->GetItemSet();
        rNotesSet.Put(XLineStyleItem(XLINE_NONE));
        rNotesSet.Put(XFillStyleItem(XFILL_NONE));
        rNotesSet.Put(aSvxFontItem);
        rNotesSet.Put(aSvxFontItemCJK);
        rNotesSet.Put(aSvxFontItemCTL);
        rNotesSet.Put( SvxPostureItem( ITALIC_NONE, EE_CHAR_ITALIC ) );
        rNotesSet.Put( SvxPostureItem( ITALIC_NONE, EE_CHAR_ITALIC_CJK ) );
        rNotesSet.Put( SvxPostureItem( ITALIC_NONE, EE_CHAR_ITALIC_CTL ) );
        rNotesSet.Put( SvxWeightItem( WEIGHT_NORMAL, EE_CHAR_WEIGHT ) );
        rNotesSet.Put( SvxWeightItem( WEIGHT_NORMAL, EE_CHAR_WEIGHT_CJK ) );
        rNotesSet.Put( SvxWeightItem( WEIGHT_NORMAL, EE_CHAR_WEIGHT_CTL ) );
        rNotesSet.Put( SvxFontHeightItem( 705, 100, EE_CHAR_FONTHEIGHT ) );     // 20 pt
        rNotesSet.Put( SvxFontHeightItem( 705, 100, EE_CHAR_FONTHEIGHT_CJK ) ); // 20 pt
        rNotesSet.Put( SvxFontHeightItem( SdDrawDocument::convertFontHeightToCTL( 705 ), 100, EE_CHAR_FONTHEIGHT_CTL ) ); // 20 pt
        rNotesSet.Put( SvxUnderlineItem(UNDERLINE_NONE, EE_CHAR_UNDERLINE ) );
        rNotesSet.Put( SvxOverlineItem(UNDERLINE_NONE, EE_CHAR_OVERLINE ) );
        rNotesSet.Put( SvxCrossedOutItem(STRIKEOUT_NONE, EE_CHAR_STRIKEOUT ) );
        rNotesSet.Put( SvxShadowedItem(sal_False, EE_CHAR_SHADOW ) );
        rNotesSet.Put( SvxContourItem(sal_False, EE_CHAR_OUTLINE ) );
        rNotesSet.Put( SvxEmphasisMarkItem(EMPHASISMARK_NONE, EE_CHAR_EMPHASISMARK ) );
        rNotesSet.Put( SvxCharReliefItem(RELIEF_NONE, EE_CHAR_RELIEF) );
        rNotesSet.Put( SvxColorItem( Color(COL_AUTO), EE_CHAR_COLOR ) );
        rNotesSet.Put( SvxLRSpaceItem( 0, 0, 600, -600, EE_PARA_LRSPACE  ) );
        // #i16874# enable kerning by default but only for new documents
        rNotesSet.Put( SvxAutoKernItem( sal_True, EE_CHAR_PAIRKERNING ) );

/* #i35937# */

    }

    /**************************************************************************
    * Background objects
    **************************************************************************/
    aName = aPrefix + SD_RESSTR(STR_LAYOUT_BACKGROUNDOBJECTS);

    if (!Find(aName, SD_STYLE_FAMILY_MASTERPAGE))
    {
        bCreated = sal_True;

        pSheet = &Make(aName, SD_STYLE_FAMILY_MASTERPAGE,nUsedMask);
        pSheet->SetHelpId( aHelpFile, HID_PSEUDOSHEET_BACKGROUNDOBJECTS );
        pSheet->SetParent( OUString() );
        SfxItemSet& rBackgroundObjectsSet = pSheet->GetItemSet();
        rBackgroundObjectsSet.Put(SdrShadowItem(sal_False));
        rBackgroundObjectsSet.Put(SdrShadowColorItem(Color(COL_GRAY)));
        rBackgroundObjectsSet.Put(SdrShadowXDistItem(200)); // 3 mm shadow distance
        rBackgroundObjectsSet.Put(SdrShadowYDistItem(200));
        // #i16874# enable kerning by default but only for new documents
        rBackgroundObjectsSet.Put( SvxAutoKernItem( sal_True, EE_CHAR_PAIRKERNING ) );
        rBackgroundObjectsSet.Put(SdrTextHorzAdjustItem(SDRTEXTHORZADJUST_BLOCK));
    }

    /**************************************************************************
    * Background
    **************************************************************************/
    aName = aPrefix + SD_RESSTR(STR_LAYOUT_BACKGROUND);

    if (!Find(aName, SD_STYLE_FAMILY_MASTERPAGE))
    {
        bCreated = sal_True;

        pSheet = &Make(aName, SD_STYLE_FAMILY_MASTERPAGE,nUsedMask);
        pSheet->SetHelpId( aHelpFile, HID_PSEUDOSHEET_BACKGROUND );
        pSheet->SetParent( OUString() );
        SfxItemSet& rBackgroundSet = pSheet->GetItemSet();
        rBackgroundSet.Put(XLineStyleItem(XLINE_NONE));
        rBackgroundSet.Put(XFillStyleItem(XFILL_NONE));
        // #i16874# enable kerning by default but only for new documents
        rBackgroundSet.Put( SvxAutoKernItem( sal_True, EE_CHAR_PAIRKERNING ) );
    }

    DBG_ASSERT( !bCheck || !bCreated, "missing layout style sheets detected!" );
}

/*************************************************************************
|*
|* Copy graphic style sheets from source pool into this pool
|*
|* (rSourcePool can not be const since SfxStyleSheetPoolBase::Find isn't const)
|*
\************************************************************************/

void SdStyleSheetPool::CopyGraphicSheets(SdStyleSheetPool& rSourcePool)
{
    CopySheets( rSourcePool, SD_STYLE_FAMILY_GRAPHICS );
}

void SdStyleSheetPool::CopyCellSheets(SdStyleSheetPool& rSourcePool)
{
    CopySheets( rSourcePool, SD_STYLE_FAMILY_CELL );
}

void SdStyleSheetPool::CopyTableStyles(SdStyleSheetPool& rSourcePool)
{
    Reference< XIndexAccess > xSource( rSourcePool.mxTableFamily, UNO_QUERY );
    Reference< XNameContainer > xTarget( mxTableFamily, UNO_QUERY );
    Reference< XSingleServiceFactory > xFactory( mxTableFamily, UNO_QUERY );

    if( xSource.is() && xFactory.is() && mxTableFamily.is() )
    {
        for( sal_Int32 nIndex = 0; nIndex < xSource->getCount(); nIndex++ ) try
        {
            Reference< XStyle > xSourceTableStyle( xSource->getByIndex( nIndex ), UNO_QUERY );
            if( xSourceTableStyle.is() )
            {
                Reference< XStyle > xNewTableStyle( xFactory->createInstance(), UNO_QUERY );
                if( xNewTableStyle.is() )
                {
                    Reference< XNameAccess> xSourceNames( xSourceTableStyle, UNO_QUERY_THROW );

                    Sequence< OUString > aStyleNames( xSourceNames->getElementNames() );
                    OUString* pStyleNames( aStyleNames.getArray() );

                    Reference< XNameReplace > xTargetNames( xNewTableStyle, UNO_QUERY );

                    sal_Int32 nNames = aStyleNames.getLength();
                    while( nNames-- )
                    {
                        const OUString aName( *pStyleNames++ );
                        Reference< XStyle > xSourceStyle( xSourceNames->getByName( aName ), UNO_QUERY );
                        Reference< XStyle > xTargetStyle;
                        if( xSourceStyle.is() ) try
                        {
                            mxCellFamily->getByName( xSourceStyle->getName() ) >>= xTargetStyle;
                        }
                        catch( Exception& )
                        {
                            OSL_FAIL( "sd::SdStyleSheetPool::CopyTableStyles(), exception caught!" );
                        }

                        if( xTargetStyle.is() )
                            xTargetNames->replaceByName( aName, Any( xTargetStyle ) );
                    }
                }

                OUString sName( xSourceTableStyle->getName() );
                if( xTarget->hasByName( sName ) )
                    xTarget->replaceByName( sName, Any( xNewTableStyle ) );
                else
                    xTarget->insertByName( sName, Any( xNewTableStyle ) );
            }
        }
        catch( Exception& )
        {
            OSL_FAIL("sd::SdStyleSheetPool::CopyTableStyles(), exception caught!");
        }
    }
}

void SdStyleSheetPool::CopyGraphicSheets(SdStyleSheetPool& rSourcePool, SdStyleSheetVector& rCreatedSheets)
{
    CopySheets( rSourcePool, SD_STYLE_FAMILY_GRAPHICS, rCreatedSheets );
}

void SdStyleSheetPool::CopyCellSheets(SdStyleSheetPool& rSourcePool, SdStyleSheetVector& rCreatedSheets)
{
    CopySheets( rSourcePool, SD_STYLE_FAMILY_CELL, rCreatedSheets );
}

void SdStyleSheetPool::CopySheets(SdStyleSheetPool& rSourcePool, SfxStyleFamily eFamily )
{
    SdStyleSheetVector aTmpSheets;
    CopySheets(rSourcePool, eFamily, aTmpSheets);
}

void SdStyleSheetPool::CopySheets(SdStyleSheetPool& rSourcePool, SfxStyleFamily eFamily, SdStyleSheetVector& rCreatedSheets)
{
    OUString aHelpFile;

    sal_uInt32 nCount = rSourcePool.aStyles.size();

    std::vector< std::pair< rtl::Reference< SfxStyleSheetBase >, OUString > > aNewStyles;

    for (sal_uInt32 n = 0; n < nCount; n++)
    {
        rtl::Reference< SfxStyleSheetBase > xSheet( rSourcePool.aStyles[sal::static_int_cast<sal_uInt16>(n)] );

        if( xSheet->GetFamily() == eFamily )
        {
            OUString aName( xSheet->GetName() );
            if ( !Find( aName, eFamily ) )
            {
                rtl::Reference< SfxStyleSheetBase > xNewSheet( &Make( aName, eFamily ) );

                xNewSheet->SetMask( xSheet->GetMask() );

                // Also set parent relation for copied style sheets
                OUString aParent( xSheet->GetParent() );
                if( !aParent.isEmpty() )
                    aNewStyles.push_back( std::pair< rtl::Reference< SfxStyleSheetBase >, OUString >( xNewSheet, aParent ) );

                xNewSheet->SetHelpId( aHelpFile, xSheet->GetHelpId( aHelpFile ) );
                xNewSheet->GetItemSet().Put( xSheet->GetItemSet() );

                rCreatedSheets.push_back( SdStyleSheetRef( static_cast< SdStyleSheet* >( xNewSheet.get() ) ) );
            }
        }
    }

    // set parents on newly added stylesheets
    std::vector< std::pair< rtl::Reference< SfxStyleSheetBase >, OUString > >::iterator aIter;
    for( aIter = aNewStyles.begin(); aIter != aNewStyles.end(); ++aIter )
    {
        DBG_ASSERT( rSourcePool.Find( (*aIter).second, eFamily ), "StyleSheet has invalid parent: Family mismatch" );
        (*aIter).first->SetParent( (*aIter).second );
    }
}


/*************************************************************************
|*
|* Copy style sheets of the named presentation layout from the source pool into
|* this pool. Copies only the style sheets which aren't yet in this pool.
|* If not NULL, pCreatedSheets is filled with pointers to the created style
|* sheets.
|*
|* (rSourcePool can not be const since SfxStyleSheetPoolBase::Find isn't const)
|*
\************************************************************************/

void SdStyleSheetPool::CopyLayoutSheets(const OUString& rLayoutName, SdStyleSheetPool& rSourcePool, SdStyleSheetVector& rCreatedSheets)
{
    SfxStyleSheetBase* pSheet = NULL;

    std::vector<OUString> aNameList;
    CreateLayoutSheetNames(rLayoutName,aNameList);

    OUString sEmpty;
    for (std::vector<OUString>::const_iterator it = aNameList.begin(); it != aNameList.end(); ++it)
    {
        pSheet = Find(*it, SD_STYLE_FAMILY_MASTERPAGE);
        if (!pSheet)
        {
            SfxStyleSheetBase* pSourceSheet = rSourcePool.Find(*it, SD_STYLE_FAMILY_MASTERPAGE);
            DBG_ASSERT(pSourceSheet, "CopyLayoutSheets: Style sheet missing");
            if (pSourceSheet)
            {
                // In the case one comes with Methusalem-Docs.
                SfxStyleSheetBase& rNewSheet = Make(*it, SD_STYLE_FAMILY_MASTERPAGE);
                rNewSheet.SetHelpId( sEmpty, pSourceSheet->GetHelpId( sEmpty ) );
                rNewSheet.GetItemSet().Put(pSourceSheet->GetItemSet());
                rCreatedSheets.push_back( SdStyleSheetRef( static_cast< SdStyleSheet* >( &rNewSheet ) ) );
            }
        }
    }

    // Special treatment for outline templates: create parent relation
    std::vector<SfxStyleSheetBase*> aOutlineSheets;
    CreateOutlineSheetList(rLayoutName,aOutlineSheets);

    if( !aOutlineSheets.empty() )
    {
        std::vector<SfxStyleSheetBase*>::iterator it = aOutlineSheets.begin();
        SfxStyleSheetBase* pParent = *it;
        ++it;

        while (it != aOutlineSheets.end())
        {
            pSheet = *it;

            if (!pSheet)
                break;

            if (pSheet->GetParent().isEmpty())
                pSheet->SetParent(pParent->GetName());

            pParent = pSheet;

            ++it;
        }
    }
}

/*************************************************************************
|*
|* Create list with names of the presentation templates of a layout.
|* The list and the containing strings are owned by the caller!
|*
\************************************************************************/

void SdStyleSheetPool::CreateLayoutSheetNames(const OUString& rLayoutName, std::vector<OUString> &aNameList) const
{
    OUString aPrefix(rLayoutName + SD_LT_SEPARATOR);
    OUString aName(SD_RESSTR(STR_LAYOUT_OUTLINE));

    for (sal_Int32 nLevel = 1; nLevel < 10; nLevel++)
        aNameList.push_back( aPrefix + aName + " " + OUString::number( nLevel ) );

    aNameList.push_back( aPrefix + SD_RESSTR(STR_LAYOUT_TITLE) );
    aNameList.push_back( aPrefix + SD_RESSTR(STR_LAYOUT_SUBTITLE) );
    aNameList.push_back( aPrefix + SD_RESSTR(STR_LAYOUT_NOTES) );
    aNameList.push_back( aPrefix + SD_RESSTR(STR_LAYOUT_BACKGROUNDOBJECTS) );
    aNameList.push_back( aPrefix + SD_RESSTR(STR_LAYOUT_BACKGROUND) );
}

/*************************************************************************
|*
|* Create a list with pointer to presentation templates of a layout.
|* The list is owned by the caller!
|*
\************************************************************************/

void SdStyleSheetPool::CreateLayoutSheetList(const OUString& rLayoutName, SdStyleSheetVector& rLayoutSheets )
{
    OUString aLayoutNameWithSep(rLayoutName + OUString(SD_LT_SEPARATOR ));

    SfxStyleSheetIterator aIter(this, SD_STYLE_FAMILY_MASTERPAGE);
    SfxStyleSheetBase* pSheet = aIter.First();

    while (pSheet)
    {
        if (pSheet->GetName().startsWith(aLayoutNameWithSep))
            rLayoutSheets.push_back( SdStyleSheetRef( static_cast< SdStyleSheet* >( pSheet ) ) );
        pSheet = aIter.Next();
    }
}

/*************************************************************************
|*
|* Create pseudo style sheets if necessary
|*
\************************************************************************/

void SdStyleSheetPool::CreatePseudosIfNecessary()
{
    OUString aName;
    OUString aHelpFile;
    SfxStyleSheetBase* pSheet = NULL;
    SfxStyleSheetBase* pParent = NULL;

    sal_uInt16 nUsedMask = SFXSTYLEBIT_USED;

    aName = SD_RESSTR(STR_PSEUDOSHEET_TITLE);
    if( (pSheet = Find(aName, SD_STYLE_FAMILY_PSEUDO)) == 0 )
    {
        pSheet = &Make(aName, SD_STYLE_FAMILY_PSEUDO, nUsedMask);
        pSheet->SetParent( OUString() );
        ((SfxStyleSheet*)pSheet)->StartListening(*this);
    }
    pSheet->SetHelpId( aHelpFile, HID_PSEUDOSHEET_TITLE );

    aName = SD_RESSTR(STR_PSEUDOSHEET_SUBTITLE);
    if( (pSheet = Find(aName, SD_STYLE_FAMILY_PSEUDO)) == 0 )
    {
        pSheet = &Make(aName, SD_STYLE_FAMILY_PSEUDO, nUsedMask);
        pSheet->SetParent( OUString() );
        ((SfxStyleSheet*)pSheet)->StartListening(*this);
    }
    pSheet->SetHelpId( aHelpFile, HID_PSEUDOSHEET_SUBTITLE );

    aName = SD_RESSTR(STR_PSEUDOSHEET_BACKGROUNDOBJECTS);
    if( (pSheet = Find(aName, SD_STYLE_FAMILY_PSEUDO)) == 0 )
    {
        pSheet = &Make(aName, SD_STYLE_FAMILY_PSEUDO, nUsedMask);
        pSheet->SetParent( OUString() );
        ((SfxStyleSheet*)pSheet)->StartListening(*this);
    }
    pSheet->SetHelpId( aHelpFile, HID_PSEUDOSHEET_BACKGROUNDOBJECTS );

    aName = SD_RESSTR(STR_PSEUDOSHEET_BACKGROUND);
    if( (pSheet = Find(aName, SD_STYLE_FAMILY_PSEUDO)) == 0 )
    {
        pSheet = &Make(aName, SD_STYLE_FAMILY_PSEUDO, nUsedMask);
        pSheet->SetParent( OUString() );
        ((SfxStyleSheet*)pSheet)->StartListening(*this);
    }
    pSheet->SetHelpId( aHelpFile, HID_PSEUDOSHEET_BACKGROUND );

    aName = SD_RESSTR(STR_PSEUDOSHEET_NOTES);
    if( (pSheet = Find(aName, SD_STYLE_FAMILY_PSEUDO)) == 0 )
    {
        pSheet = &Make(aName, SD_STYLE_FAMILY_PSEUDO, nUsedMask);
        pSheet->SetParent( OUString() );
        ((SfxStyleSheet*)pSheet)->StartListening(*this);
    }
    pSheet->SetHelpId( aHelpFile, HID_PSEUDOSHEET_NOTES );

    pParent = NULL;
    SetSearchMask(SD_STYLE_FAMILY_PSEUDO);
    aName = SD_RESSTR(STR_PSEUDOSHEET_OUTLINE);
    for (sal_Int32 nLevel = 1; nLevel < 10; nLevel++)
    {
        OUString aLevelName( aName + " " + OUString::number( nLevel ) );

        if( (pSheet = Find(aLevelName, SD_STYLE_FAMILY_PSEUDO)) == 0 )
        {
            pSheet = &Make(aLevelName, SD_STYLE_FAMILY_PSEUDO, nUsedMask);

            if (pSheet)
            {
                if (pParent)
                    pSheet->SetParent(pParent->GetName());
                pParent = pSheet;
                ((SfxStyleSheet*)pSheet)->StartListening(*this);
            }
        }
        pSheet->SetHelpId( aHelpFile, HID_PSEUDOSHEET_OUTLINE + nLevel );
    }
}


/*************************************************************************
|*
|* Set the correct name in the program language to the standard styles
|*
\************************************************************************/

void SdStyleSheetPool::UpdateStdNames()
{
    OUString aHelpFile;
    sal_uInt32  nCount = aStyles.size();
    std::vector<SfxStyleSheetBase*> aEraseList;

    for( sal_uInt32 n=0; n < nCount; n++ )
    {
        SfxStyleSheetBase* pStyle = aStyles[ n ].get();

        if( !pStyle->IsUserDefined() )
        {
            OUString aOldName   = pStyle->GetName();
            sal_uLong nHelpId   = pStyle->GetHelpId( aHelpFile );
            SfxStyleFamily eFam = pStyle->GetFamily();

            sal_Bool bHelpKnown = sal_True;
            OUString aNewName;
            sal_uInt16 nNameId = 0;
            switch( nHelpId )
            {
                case HID_STANDARD_STYLESHEET_NAME:  nNameId = STR_STANDARD_STYLESHEET_NAME; break;
                case HID_POOLSHEET_OBJWITHARROW:    nNameId = STR_POOLSHEET_OBJWITHARROW;   break;
                case HID_POOLSHEET_OBJWITHSHADOW:   nNameId = STR_POOLSHEET_OBJWITHSHADOW;  break;
                case HID_POOLSHEET_OBJWITHOUTFILL:  nNameId = STR_POOLSHEET_OBJWITHOUTFILL; break;
                case HID_POOLSHEET_OBJNOLINENOFILL: nNameId = STR_POOLSHEET_OBJNOLINENOFILL;break;
                case HID_POOLSHEET_TEXT:            nNameId = STR_POOLSHEET_TEXT;           break;
                case HID_POOLSHEET_TEXTBODY:        nNameId = STR_POOLSHEET_TEXTBODY;       break;
                case HID_POOLSHEET_TEXTBODY_JUSTIFY:nNameId = STR_POOLSHEET_TEXTBODY_JUSTIFY;break;
                case HID_POOLSHEET_TEXTBODY_INDENT: nNameId = STR_POOLSHEET_TEXTBODY_INDENT;break;
                case HID_POOLSHEET_TITLE:           nNameId = STR_POOLSHEET_TITLE;          break;
                case HID_POOLSHEET_TITLE1:          nNameId = STR_POOLSHEET_TITLE1;         break;
                case HID_POOLSHEET_TITLE2:          nNameId = STR_POOLSHEET_TITLE2;         break;
                case HID_POOLSHEET_HEADLINE:        nNameId = STR_POOLSHEET_HEADLINE;       break;
                case HID_POOLSHEET_HEADLINE1:       nNameId = STR_POOLSHEET_HEADLINE1;      break;
                case HID_POOLSHEET_HEADLINE2:       nNameId = STR_POOLSHEET_HEADLINE2;      break;
                case HID_POOLSHEET_MEASURE:         nNameId = STR_POOLSHEET_MEASURE;        break;

                case HID_PSEUDOSHEET_TITLE:         nNameId = STR_PSEUDOSHEET_TITLE;        break;
                case HID_PSEUDOSHEET_SUBTITLE:      nNameId = STR_PSEUDOSHEET_SUBTITLE;     break;
                case HID_PSEUDOSHEET_OUTLINE1:
                case HID_PSEUDOSHEET_OUTLINE2:
                case HID_PSEUDOSHEET_OUTLINE3:
                case HID_PSEUDOSHEET_OUTLINE4:
                case HID_PSEUDOSHEET_OUTLINE5:
                case HID_PSEUDOSHEET_OUTLINE6:
                case HID_PSEUDOSHEET_OUTLINE7:
                case HID_PSEUDOSHEET_OUTLINE8:
                case HID_PSEUDOSHEET_OUTLINE9:      nNameId = STR_PSEUDOSHEET_OUTLINE;      break;
                case HID_PSEUDOSHEET_BACKGROUNDOBJECTS: nNameId = STR_PSEUDOSHEET_BACKGROUNDOBJECTS; break;
                case HID_PSEUDOSHEET_BACKGROUND:    nNameId = STR_PSEUDOSHEET_BACKGROUND;   break;
                case HID_PSEUDOSHEET_NOTES:         nNameId = STR_PSEUDOSHEET_NOTES;        break;

                case HID_SD_CELL_STYLE_DEFAULT:         nNameId = STR_STANDARD_STYLESHEET_NAME; break;
                case HID_SD_CELL_STYLE_BANDED:          nNameId = STR_POOLSHEET_BANDED_CELL; break;
                case HID_SD_CELL_STYLE_HEADER:          nNameId = STR_POOLSHEET_HEADER; break;
                case HID_SD_CELL_STYLE_TOTAL:           nNameId = STR_POOLSHEET_TOTAL; break;
                case HID_SD_CELL_STYLE_FIRST_COLUMN:    nNameId = STR_POOLSHEET_FIRST_COLUMN; break;
                case HID_SD_CELL_STYLE_LAST_COLUMN:     nNameId = STR_POOLSHEET_LAST_COLUMN; break;

                default:
                    // 0 or wrong (old) HelpId
                    bHelpKnown = sal_False;
            }
            if( bHelpKnown )
            {
                if( nNameId )
                {
                    aNewName = SD_RESSTR( nNameId );
                    if( nNameId == STR_PSEUDOSHEET_OUTLINE )
                    {
                        aNewName += " " + OUString::number( sal_Int32( nHelpId - HID_PSEUDOSHEET_OUTLINE ) );
                    }
                }

                if( !aNewName.isEmpty() && aNewName != aOldName )
                {
                    SfxStyleSheetBase* pSheetFound = Find( aNewName, eFam );

                    if ( !pSheetFound )
                    {
                        // Sheet does not yet exist: rename old sheet
                        pStyle->SetName( aNewName );    // transform also parents
                    }
                    else
                    {
                        // Sheet does exist: old sheet has to be removed
                        aEraseList.push_back( pStyle );
                    }
                }
            }
        }
    }

    // styles that could not be renamed, must be removed
    for ( size_t i = 0, n = aEraseList.size(); i < n; ++i )
        Remove( aEraseList[ i ] );
}
// --------------------------------------------------------------------
// Set new SvxNumBulletItem for the respective style sheet
// --------------------------------------------------------------------

void SdStyleSheetPool::PutNumBulletItem( SfxStyleSheetBase* pSheet,
                                         Font& rBulletFont )
{
    OUString aHelpFile;
    sal_uLong nHelpId = pSheet->GetHelpId( aHelpFile );
    SfxItemSet& rSet = pSheet->GetItemSet();

    switch ( nHelpId )
    {
        case HID_STANDARD_STYLESHEET_NAME :
        {
            // Standard template
            SvxNumberFormat aNumberFormat(SVX_NUM_CHAR_SPECIAL);
            aNumberFormat.SetBulletFont(&rBulletFont);
            aNumberFormat.SetBulletChar( 0x25CF ); // StarBats: 0xF000 + 34
            aNumberFormat.SetBulletRelSize(45);
            aNumberFormat.SetBulletColor(Color(COL_AUTO));
            aNumberFormat.SetStart(1);
            aNumberFormat.SetNumAdjust(SVX_ADJUST_LEFT);

            SvxNumRule aNumRule( NUM_BULLET_REL_SIZE|NUM_BULLET_COLOR|NUM_CHAR_TEXT_DISTANCE, SVX_MAX_NUM , sal_False);

            for( sal_uInt16 i = 0; i < aNumRule.GetLevelCount(); i++ )
            {
                const short nLSpace = (i + 1) * 600;
                aNumberFormat.SetLSpace(nLSpace);
                aNumberFormat.SetAbsLSpace(nLSpace);
                aNumberFormat.SetFirstLineOffset(-600);
                aNumRule.SetLevel( i, aNumberFormat );
            }

            rSet.Put( SvxNumBulletItem( aNumRule, EE_PARA_NUMBULLET ) );
            ((SfxStyleSheet*)pSheet)->Broadcast(SfxSimpleHint( SFX_HINT_DATACHANGED ) );
        }
        break;

        case HID_PSEUDOSHEET_TITLE:
            /* title gets same bullet as subtitle and not that page symbol anymore */
        case HID_PSEUDOSHEET_SUBTITLE :
        {
            // Subtitle template
            SvxNumRule* pDefaultRule = ((SvxNumBulletItem*) rSet.GetPool()->GetSecondaryPool()->GetPoolDefaultItem(EE_PARA_NUMBULLET))->GetNumRule();
            DBG_ASSERT( pDefaultRule, "Where is my default template? [CL]" );

            if(pDefaultRule)
            {
                SvxNumRule aNumRule(pDefaultRule->GetFeatureFlags(), 10, sal_False);
                for(sal_uInt16 i=0; i < aNumRule.GetLevelCount(); i++)
                {
                    SvxNumberFormat aFrmt( pDefaultRule->GetLevel(i) );
                    aFrmt.SetNumberingType(SVX_NUM_CHAR_SPECIAL);
                    // #i93908# clear suffix for bullet lists
                    aFrmt.SetPrefix(OUString());
                    aFrmt.SetSuffix(OUString());
                    aFrmt.SetStart(1);
                    aFrmt.SetBulletRelSize(45);
                    aFrmt.SetBulletChar( 0x25CF );  // StarBats: 0xF000 + 34
                    aFrmt.SetBulletFont(&rBulletFont);
                    aNumRule.SetLevel(i, aFrmt);
                }

                rSet.Put( SvxNumBulletItem( aNumRule, EE_PARA_NUMBULLET ) );
                ((SfxStyleSheet*)pSheet)->Broadcast(SfxSimpleHint( SFX_HINT_DATACHANGED ) );
            }
        }
        break;

        case HID_PSEUDOSHEET_OUTLINE + 1 :
        {
            // Outline template
            SvxNumberFormat aNumberFormat(SVX_NUM_CHAR_SPECIAL);
            aNumberFormat.SetBulletColor(Color(COL_AUTO));
            aNumberFormat.SetStart(1);
            aNumberFormat.SetNumAdjust(SVX_ADJUST_LEFT);

            SvxNumRule aNumRule( NUM_BULLET_REL_SIZE|NUM_BULLET_COLOR|NUM_CHAR_TEXT_DISTANCE|NUM_SYMBOL_ALIGNMENT,
                                 SVX_MAX_NUM, sal_False );
            for( sal_uInt16 i = 0; i < aNumRule.GetLevelCount(); i++ )
            {
                aNumberFormat.SetBulletChar( 0x25CF );  // StarBats: 0xF000 + 34
                aNumberFormat.SetBulletRelSize(45);
                const short nLSpace = (i + 1) * 1200;
                aNumberFormat.SetLSpace(nLSpace);
                aNumberFormat.SetAbsLSpace(nLSpace);
                short nFirstLineOffset = -600;

                sal_uLong nFontSize = 20;
                switch(i)
                {
                    case 0:
                    {
                        nFontSize = 32;
                        nFirstLineOffset = -900;
                    }
                    break;

                    case 1:
                    {
                        aNumberFormat.SetBulletChar( 0x2013 );  // StarBats: 0xF000 + 150
                        aNumberFormat.SetBulletRelSize(75);
                        nFontSize = 32;
                        nFirstLineOffset = -900;
                    }
                    break;

                    case 2:
                    {
                        nFontSize = 28;
                        nFirstLineOffset = -800;
                    }
                    break;

                    case 3:
                    {
                        aNumberFormat.SetBulletChar( 0x2013 );  // StarBats: 0xF000 + 150
                        aNumberFormat.SetBulletRelSize(75);
                        nFontSize = 24;
                    }
                    break;
                }

                aNumberFormat.SetFirstLineOffset(nFirstLineOffset);
                nFontSize = (sal_uInt16)((nFontSize * 2540L) / 72);  // Pt --> 1/100 mm
                rBulletFont.SetSize(Size(0,846));       // 24 pt
                aNumberFormat.SetBulletFont(&rBulletFont);
                aNumRule.SetLevel( i, aNumberFormat );
            }

            rSet.Put( SvxNumBulletItem( aNumRule, EE_PARA_NUMBULLET ) );
            ((SfxStyleSheet*)pSheet)->Broadcast(SfxSimpleHint( SFX_HINT_DATACHANGED ) );
        }
        break;
    }
}

/*************************************************************************
|*
|* Create standard bullet font (without size)
|*
\************************************************************************/

Font SdStyleSheetPool::GetBulletFont() const
{
    Font aBulletFont( OUString( "StarSymbol" ), Size(0, 1000) );
    aBulletFont.SetCharSet(RTL_TEXTENCODING_UNICODE);
    aBulletFont.SetWeight(WEIGHT_NORMAL);
    aBulletFont.SetUnderline(UNDERLINE_NONE);
    aBulletFont.SetOverline(UNDERLINE_NONE);
    aBulletFont.SetStrikeout(STRIKEOUT_NONE);
    aBulletFont.SetItalic(ITALIC_NONE);
    aBulletFont.SetOutline(sal_False);
    aBulletFont.SetShadow(sal_False);
    aBulletFont.SetColor(Color(COL_AUTO));
    aBulletFont.SetTransparent(sal_True);

    return aBulletFont;
}

// --------------------------------------------------------------------

void SdStyleSheetPool::AddStyleFamily( const SdPage* pPage )
{
    rtl::Reference< SfxStyleSheetPool > xPool( this );
    maStyleFamilyMap[pPage] = new SdStyleFamily( xPool, pPage );
}

// --------------------------------------------------------------------

void SdStyleSheetPool::RemoveStyleFamily( const SdPage* pPage )
{
    SdStyleFamilyMap::iterator iter( maStyleFamilyMap.find( pPage ) );
    if( iter != maStyleFamilyMap.end() )
    {
        SdStyleFamilyRef xStyle( (*iter).second );
        maStyleFamilyMap.erase( iter );

        if( xStyle.is() ) try
        {
            xStyle->dispose();
        }
        catch( Exception& )
        {
        }
    }
}

// --------------------------------------------------------------------

void SdStyleSheetPool::throwIfDisposed() throw(::com::sun::star::uno::RuntimeException)
{
    if( mpDoc == NULL )
        throw DisposedException();
}

// --------------------------------------------------------------------
// XServiceInfo
// --------------------------------------------------------------------

OUString SAL_CALL SdStyleSheetPool::getImplementationName() throw(RuntimeException)
{
    return OUString( "SdStyleSheetPool" );
}

// --------------------------------------------------------------------

static const sal_Char* gpServiceName = "com.sun.star.style.StyleFamilies";

sal_Bool SAL_CALL SdStyleSheetPool::supportsService( const OUString& ServiceName ) throw(RuntimeException)
{
    return ServiceName.equalsAscii( gpServiceName );
}

// --------------------------------------------------------------------

Sequence< OUString > SAL_CALL SdStyleSheetPool::getSupportedServiceNames() throw(RuntimeException)
{
    OUString aStr( OUString::createFromAscii( gpServiceName ) );
    return Sequence< OUString >( &aStr, 1 );
}

// --------------------------------------------------------------------
// XNameAccess
// --------------------------------------------------------------------

Any SAL_CALL SdStyleSheetPool::getByName( const OUString& aName ) throw(NoSuchElementException, WrappedTargetException, RuntimeException)
{
    throwIfDisposed();

    if( mxGraphicFamily->getName() == aName )
        return Any( Reference< XNameAccess >( static_cast< XNameAccess* >( mxGraphicFamily.get() ) ) );

    if( mxCellFamily->getName() == aName )
        return Any( Reference< XNameAccess >( static_cast< XNameAccess* >( mxCellFamily.get() ) ) );

    if( msTableFamilyName == aName )
        return Any( mxTableFamily );

    for( SdStyleFamilyMap::iterator iter( maStyleFamilyMap.begin() ); iter != maStyleFamilyMap.end(); ++iter )
    {
        if( (*iter).second->getName() == aName )
            return Any( Reference< XNameAccess >( static_cast< XNameAccess* >( (*iter).second.get() ) ) );
    }

    throw NoSuchElementException();
}

// --------------------------------------------------------------------

Sequence< OUString > SAL_CALL SdStyleSheetPool::getElementNames() throw(RuntimeException)
{
    throwIfDisposed();

    Sequence< OUString > aNames( maStyleFamilyMap.size() + 3 );
    OUString* pNames = aNames.getArray();

    *pNames++ = mxGraphicFamily->getName();
    *pNames++ = mxCellFamily->getName();
    *pNames++ = msTableFamilyName;

    for( SdStyleFamilyMap::iterator iter( maStyleFamilyMap.begin() ); iter != maStyleFamilyMap.end(); ++iter )
    {
        *pNames++ = (*iter).second->getName();
    }

    return aNames;
}

// --------------------------------------------------------------------

sal_Bool SAL_CALL SdStyleSheetPool::hasByName( const OUString& aName ) throw(RuntimeException)
{
    throwIfDisposed();

    if( mxGraphicFamily->getName() == aName )
        return sal_True;

    if( mxCellFamily->getName() == aName )
        return sal_True;

    if( msTableFamilyName == aName )
        return sal_True;

    for( SdStyleFamilyMap::iterator iter( maStyleFamilyMap.begin() ); iter != maStyleFamilyMap.end(); ++iter )
    {
        if( (*iter).second->getName() == aName )
            return sal_True;
    }

    return sal_False;
}

// --------------------------------------------------------------------
// XElementAccess
// --------------------------------------------------------------------

Type SAL_CALL SdStyleSheetPool::getElementType() throw(RuntimeException)
{
    throwIfDisposed();

    return XNameAccess::static_type();
}

// --------------------------------------------------------------------

sal_Bool SAL_CALL SdStyleSheetPool::hasElements() throw(RuntimeException)
{
    return sal_True;
}

// --------------------------------------------------------------------
// XIndexAccess
// --------------------------------------------------------------------

sal_Int32 SAL_CALL SdStyleSheetPool::getCount() throw(RuntimeException)
{
    throwIfDisposed();

    return maStyleFamilyMap.size() + 3;
}

// --------------------------------------------------------------------

Any SAL_CALL SdStyleSheetPool::getByIndex( sal_Int32 Index ) throw(IndexOutOfBoundsException, WrappedTargetException, RuntimeException)
{
    switch( Index )
    {
    case 0:
        return Any( Reference< XNameAccess >( static_cast< XNameAccess* >( mxGraphicFamily.get() ) ) );

    case 1:
        return Any( Reference< XNameAccess >( static_cast< XNameAccess* >( mxCellFamily.get() ) ) );

    case 2:
        return Any( mxTableFamily );

    default:
        {
            Index -= 3;
            if( (Index < 0) || (Index >= sal::static_int_cast<sal_Int32>(maStyleFamilyMap.size())) )
                throw IndexOutOfBoundsException();
            SdStyleFamilyMap::iterator iter( maStyleFamilyMap.begin() );
            while( Index-- )
                ++iter;

            return Any( Reference< XNameAccess >( static_cast< XNameAccess* >( (*iter).second.get() ) ) );
        }
    }
}

// --------------------------------------------------------------------
// XComponent
// --------------------------------------------------------------------

void SAL_CALL SdStyleSheetPool::dispose() throw (RuntimeException)
{
    if( mpDoc )
    {
        mxGraphicFamily->dispose();
        mxGraphicFamily.clear();
        mxCellFamily->dispose();
        mxCellFamily.clear();

        Reference< XComponent > xComp( mxTableFamily, UNO_QUERY );
        if( xComp.is() )
            xComp->dispose();
        mxTableFamily = 0;

        SdStyleFamilyMap aTempMap;
        aTempMap.swap( maStyleFamilyMap );

        for( SdStyleFamilyMap::iterator iter( aTempMap.begin() ); iter != aTempMap.end(); ++iter ) try
        {
            (*iter).second->dispose();
        }
        catch( Exception& )
        {
        }

        mpDoc = 0;

        Clear();
    }
}

// --------------------------------------------------------------------

void SAL_CALL SdStyleSheetPool::addEventListener( const Reference< XEventListener >& /*xListener*/ ) throw (RuntimeException)
{
}

// --------------------------------------------------------------------

void SAL_CALL SdStyleSheetPool::removeEventListener( const Reference< XEventListener >& /*aListener*/ ) throw (RuntimeException)
{
}

// --------------------------------------------------------------------

SdStyleSheetVector SdStyleSheetPool::CreateChildList( SdStyleSheet* pSheet )
{
    SdStyleSheetVector aResult;

    sal_uInt16 nListenerCount = pSheet->GetListenerCount();
    if (nListenerCount > 0)
    {
        for (sal_uInt16 n = 0; n < nListenerCount; n++)
        {
            SdStyleSheet* pChild = dynamic_cast< SdStyleSheet* >( pSheet->GetListener(n) );
            if(pChild && pChild->GetParent() == pSheet->GetName())
            {
                aResult.push_back( SdStyleSheetRef( pChild ) );
            }
        }
    }

    return aResult;
}

// --------------------------------------------------------------------

void SAL_CALL SdStyleSheetPool::acquire (void) throw ()
{
    SdStyleSheetPoolBase::acquire();
}

void SAL_CALL SdStyleSheetPool::release (void) throw ()
{
    SdStyleSheetPoolBase::release();
}

// --------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
