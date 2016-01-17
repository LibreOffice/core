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
#include <DocumentStylePoolManager.hxx>
#include <doc.hxx>
#include <DocumentSettingManager.hxx>
#include <IDocumentState.hxx>
#include <IDocumentUndoRedo.hxx>
#include <fmtanchr.hxx>
#include <fmtfsize.hxx>
#include <paratr.hxx>
#include <poolfmt.hxx>
#include <fmtornt.hxx>
#include <fmtsrndenum.hxx>
#include <charfmt.hxx>
#include <fmtsrnd.hxx>
#include <docary.hxx>
#include <GetMetricVal.hxx>
#include <pagedesc.hxx>
#include <frmfmt.hxx>
#include <fmtline.hxx>
#include <numrule.hxx>
#include <editeng/paperinf.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/tstpitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/lspcitem.hxx>
#include <editeng/adjustitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/keepitem.hxx>
#include <editeng/opaqitem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/cmapitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/protitem.hxx>
#include <editeng/escapementitem.hxx>
#include <editeng/langitem.hxx>
#include <editeng/charrotateitem.hxx>
#include <editeng/frmdiritem.hxx>
#include <editeng/emphasismarkitem.hxx>
#include <editeng/scriptspaceitem.hxx>
#include <rcid.hrc>
#include <com/sun/star/table/BorderLineStyle.hpp>
#include <com/sun/star/text/VertOrientation.hpp>
#include <com/sun/star/text/RelOrientation.hpp>
#include <com/sun/star/text/HoriOrientation.hpp>

using namespace ::editeng;
using namespace ::com::sun::star;

bool IsConditionalByPoolId(sal_uInt16 nId)
{
    // TODO: why is this style conditional?
    // If it is changed to no longer be conditional, then a style "Text Body"
    // will be imported without its conditions from ODF.
    return RES_POOLCOLL_TEXT == nId;
}

namespace
{
    static const sal_uInt16 PT_3   =  3 * 20;      //  3 pt
    static const sal_uInt16 PT_6   =  6 * 20;      //  6 pt
    static const sal_uInt16 PT_7   =  7 * 20;      //  7 pt
    static const sal_uInt16 PT_10  = 10 * 20;      // 10 pt
    static const sal_uInt16 PT_12  = 12 * 20;      // 12 pt
    static const sal_uInt16 PT_14  = 14 * 20;      // 14 pt
    static const sal_uInt16 PT_16  = 16 * 20;      // 16 pt
    static const sal_uInt16 PT_18  = 18 * 20;      // 18 pt
    static const sal_uInt16 PT_24  = 24 * 20;      // 24 pt
    static const sal_uInt16 PT_28  = 28 * 20;      // 28 pt

    #define HTML_PARSPACE   GetMetricVal( CM_05 )

    static const sal_uInt16 aHeadlineSizes[ 2 * MAXLEVEL ] = {
        // we do everything percentual now:
        130, 115, 101, 95, 85,
        85,   80,  80, 75, 75,  // normal
        PT_24, PT_18, PT_14, PT_12, PT_10,
        PT_7, PT_7, PT_7, PT_7, PT_7            // HTML mode
    };

    static long lcl_GetRightMargin( SwDoc& rDoc )
    {
        // Make sure that the printer settings are taken over to the standard
        // page style
        const SwFrameFormat& rPgDscFormat = rDoc.GetPageDesc( 0 ).GetMaster();
        const SvxLRSpaceItem& rLR = rPgDscFormat.GetLRSpace();
        const long nLeft = rLR.GetLeft();
        const long nRight = rLR.GetRight();
        const long nWidth = rPgDscFormat.GetFrameSize().GetWidth();
        return nWidth - nLeft - nRight;
    }

    static void lcl_SetDfltFont( DefaultFontType nFntType, SfxItemSet& rSet )
    {
        static struct {
            sal_uInt16 nResLngId;
            sal_uInt16 nResFntId;
        } aArr[ 3 ] = {
            { RES_CHRATR_LANGUAGE, RES_CHRATR_FONT },
            { RES_CHRATR_CJK_LANGUAGE, RES_CHRATR_CJK_FONT },
            { RES_CHRATR_CTL_LANGUAGE, RES_CHRATR_CTL_FONT }
        };
        for( sal_uInt16 n = 0; n < 3; ++n )
        {
            sal_uInt16 nLng = static_cast<const SvxLanguageItem&>(rSet.GetPool()->GetDefaultItem(
                                aArr[n].nResLngId )).GetLanguage();
            vcl::Font aFnt( OutputDevice::GetDefaultFont( nFntType,
                                    nLng, GetDefaultFontFlags::OnlyOne ) );

            rSet.Put( SvxFontItem( aFnt.GetFamily(), aFnt.GetFamilyName(),
                                OUString(), aFnt.GetPitch(),
                                aFnt.GetCharSet(), aArr[n].nResFntId ));
        }
    }

    static void lcl_SetDfltFont( DefaultFontType nLatinFntType, DefaultFontType nCJKFntType,
                            DefaultFontType nCTLFntType, SfxItemSet& rSet )
    {
        static struct {
            sal_uInt16 nResLngId;
            sal_uInt16 nResFntId;
            DefaultFontType nFntType;
        } aArr[ 3 ] = {
            { RES_CHRATR_LANGUAGE, RES_CHRATR_FONT, static_cast<DefaultFontType>(0) },
            { RES_CHRATR_CJK_LANGUAGE, RES_CHRATR_CJK_FONT, static_cast<DefaultFontType>(0) },
            { RES_CHRATR_CTL_LANGUAGE, RES_CHRATR_CTL_FONT, static_cast<DefaultFontType>(0) }
        };
        aArr[0].nFntType = nLatinFntType;
        aArr[1].nFntType = nCJKFntType;
        aArr[2].nFntType = nCTLFntType;

        for( sal_uInt16 n = 0; n < 3; ++n )
        {
            sal_uInt16 nLng = static_cast<const SvxLanguageItem&>(rSet.GetPool()->GetDefaultItem(
                                aArr[n].nResLngId )).GetLanguage();
            vcl::Font aFnt( OutputDevice::GetDefaultFont( aArr[n].nFntType,
                                    nLng, GetDefaultFontFlags::OnlyOne ) );

            rSet.Put( SvxFontItem( aFnt.GetFamily(), aFnt.GetFamilyName(),
                                OUString(), aFnt.GetPitch(),
                                aFnt.GetCharSet(), aArr[n].nResFntId ));
        }
    }

    static void lcl_SetHeadline( SwDoc* pDoc, SwTextFormatColl* pColl,
                            SfxItemSet& rSet,
                            sal_uInt16 nOutLvlBits, sal_uInt8 nLevel, bool bItalic )
    {
        SetAllScriptItem( rSet, SvxWeightItem( WEIGHT_BOLD, RES_CHRATR_WEIGHT ) );
        SvxFontHeightItem aHItem(240, 100, RES_CHRATR_FONTSIZE);
        const bool bHTMLMode = pDoc->GetDocumentSettingManager().get(DocumentSettingId::HTML_MODE);
        if( bHTMLMode )
            aHItem.SetHeight( aHeadlineSizes[ MAXLEVEL + nLevel ] );
        else
            aHItem.SetHeight( PT_14, aHeadlineSizes[ nLevel ] );
        SetAllScriptItem( rSet, aHItem );

        if( bItalic && !bHTMLMode )
            SetAllScriptItem( rSet, SvxPostureItem( ITALIC_NORMAL, RES_CHRATR_POSTURE ) );

        if( bHTMLMode )
        {
            lcl_SetDfltFont( DefaultFontType::LATIN_TEXT, DefaultFontType::CJK_TEXT,
                                DefaultFontType::CTL_TEXT, rSet );
        }

        if( !pColl )
            return;

        if( !( nOutLvlBits & ( 1 << nLevel )) )
        {
            pColl->AssignToListLevelOfOutlineStyle(nLevel);
            if( !bHTMLMode )
            {
                SwNumRule * pOutlineRule = pDoc->GetOutlineNumRule();
                const SwNumFormat& rNFormat = pOutlineRule->Get( nLevel );

                SvxLRSpaceItem aLR( static_cast<const SvxLRSpaceItem&>(pColl->GetFormatAttr( RES_LR_SPACE )) );
                if ( rNFormat.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_WIDTH_AND_POSITION &&
                        ( rNFormat.GetAbsLSpace() || rNFormat.GetFirstLineOffset() ) )
                {
                    aLR.SetTextFirstLineOfstValue( rNFormat.GetFirstLineOffset() );
                    aLR.SetTextLeft( rNFormat.GetAbsLSpace() );
                }
                else
                {
                    // tdf#93970 The indent set at the associated outline style also affects this paragraph.
                    // We don't want this here, so override it. This doesn't affect the outline style properties.
                    aLR.SetTextFirstLineOfstValue( 0 );
                    aLR.SetTextLeft( 0 );
                }
                pColl->SetFormatAttr( aLR );

                // All paragraph styles, which are assigned to a level of the
                // outline style has to have the outline style set as its list style.
                SwNumRuleItem aItem(pOutlineRule->GetName());
                pColl->SetFormatAttr(aItem);
            }
        }
        pColl->SetNextTextFormatColl( *pDoc->getIDocumentStylePoolAccess().GetTextCollFromPool( RES_POOLCOLL_TEXT ));
    }

    static void lcl_SetRegister( SwDoc* pDoc, SfxItemSet& rSet, sal_uInt16 nFact,
                            bool bHeader, bool bTab )
    {
        SvxLRSpaceItem aLR( RES_LR_SPACE );
        sal_uInt16 nLeft = nFact ? GetMetricVal( CM_05 ) * nFact : 0;
        aLR.SetTextLeft( nLeft );

        rSet.Put( aLR );
        if( bHeader )
        {
            SetAllScriptItem( rSet, SvxWeightItem( WEIGHT_BOLD, RES_CHRATR_WEIGHT ) );
            SetAllScriptItem( rSet, SvxFontHeightItem( PT_16, 100, RES_CHRATR_FONTSIZE ) );
        }
        if( bTab )
        {
            long nRightMargin = lcl_GetRightMargin( *pDoc );
            SvxTabStopItem aTStops( 0, 0, SVX_TAB_ADJUST_DEFAULT, RES_PARATR_TABSTOP );
            aTStops.Insert( SvxTabStop( nRightMargin - nLeft,
                                        SVX_TAB_ADJUST_RIGHT,
                                        cDfltDecimalChar, '.' ));
            rSet.Put( aTStops );
        }
    }

    static void lcl_SetNumBul( SwDoc* pDoc, SwTextFormatColl* pColl,
                            SfxItemSet& rSet,
                            sal_uInt16 nNxt, SwTwips nEZ, SwTwips nLeft,
                            SwTwips nUpper, SwTwips nLower )
    {

        SvxLRSpaceItem aLR( RES_LR_SPACE );
        SvxULSpaceItem aUL( RES_UL_SPACE );
        aLR.SetTextFirstLineOfst( sal_uInt16(nEZ) );
        aLR.SetTextLeft( sal_uInt16(nLeft) );
        aUL.SetUpper( sal_uInt16(nUpper) );
        aUL.SetLower( sal_uInt16(nLower) );
        rSet.Put( aLR );
        rSet.Put( aUL );

        if( pColl )
            pColl->SetNextTextFormatColl( *pDoc->getIDocumentStylePoolAccess().GetTextCollFromPool( nNxt ));
    }

    static void lcl_PutStdPageSizeIntoItemSet( SwDoc* pDoc, SfxItemSet& rSet )
    {
        SwPageDesc* pStdPgDsc = pDoc->getIDocumentStylePoolAccess().GetPageDescFromPool( RES_POOLPAGE_STANDARD );
        SwFormatFrameSize aFrameSz( pStdPgDsc->GetMaster().GetFrameSize() );
        if( pStdPgDsc->GetLandscape() )
        {
            SwTwips nTmp = aFrameSz.GetHeight();
            aFrameSz.SetHeight( aFrameSz.GetWidth() );
            aFrameSz.SetWidth( nTmp );
        }
        rSet.Put( aFrameSz );
    }
}


namespace sw
{

DocumentStylePoolManager::DocumentStylePoolManager( SwDoc& i_rSwdoc ) : m_rDoc( i_rSwdoc )
{
}

SwTextFormatColl* DocumentStylePoolManager::GetTextCollFromPool( sal_uInt16 nId, bool bRegardLanguage )
{
    OSL_ENSURE(
        (RES_POOLCOLL_TEXT_BEGIN <= nId && nId < RES_POOLCOLL_TEXT_END) ||
        (RES_POOLCOLL_LISTS_BEGIN <= nId && nId < RES_POOLCOLL_LISTS_END) ||
        (RES_POOLCOLL_EXTRA_BEGIN <= nId && nId < RES_POOLCOLL_EXTRA_END) ||
        (RES_POOLCOLL_REGISTER_BEGIN <= nId && nId < RES_POOLCOLL_REGISTER_END) ||
        (RES_POOLCOLL_DOC_BEGIN <= nId && nId < RES_POOLCOLL_DOC_END) ||
        (RES_POOLCOLL_HTML_BEGIN <= nId && nId < RES_POOLCOLL_HTML_END),
            "Wrong  AutoFormat Id" );

    SwTextFormatColl* pNewColl;
    sal_uInt16 nOutLvlBits = 0;
    for( size_t n = 0; n < m_rDoc.GetTextFormatColls()->size(); ++n )
    {
        if( nId == ( pNewColl = (*m_rDoc.GetTextFormatColls())[ n ] )->GetPoolFormatId() )
        {
            return pNewColl;
        }

        if( pNewColl->IsAssignedToListLevelOfOutlineStyle())
            nOutLvlBits |= ( 1 << pNewColl->GetAssignedOutlineStyleLevel() );
    }

    // Didn't find it until here -> create anew
    sal_uInt16 nResId = 0;
    if( RES_POOLCOLL_TEXT_BEGIN <= nId && nId < RES_POOLCOLL_TEXT_END )
        nResId = RC_POOLCOLL_TEXT_BEGIN - RES_POOLCOLL_TEXT_BEGIN;
    else if (RES_POOLCOLL_LISTS_BEGIN <= nId && nId < RES_POOLCOLL_LISTS_END)
        nResId = RC_POOLCOLL_LISTS_BEGIN - RES_POOLCOLL_LISTS_BEGIN;
    else if (RES_POOLCOLL_EXTRA_BEGIN <= nId && nId < RES_POOLCOLL_EXTRA_END)
        nResId = RC_POOLCOLL_EXTRA_BEGIN - RES_POOLCOLL_EXTRA_BEGIN;
    else if (RES_POOLCOLL_REGISTER_BEGIN <= nId && nId < RES_POOLCOLL_REGISTER_END)
        nResId = RC_POOLCOLL_REGISTER_BEGIN - RES_POOLCOLL_REGISTER_BEGIN;
    else if (RES_POOLCOLL_DOC_BEGIN <= nId && nId < RES_POOLCOLL_DOC_END)
        nResId = RC_POOLCOLL_DOC_BEGIN - RES_POOLCOLL_DOC_BEGIN;
    else if (RES_POOLCOLL_HTML_BEGIN <= nId && nId < RES_POOLCOLL_HTML_END)
        nResId = RC_POOLCOLL_HTML_BEGIN - RES_POOLCOLL_HTML_BEGIN;

    OSL_ENSURE( nResId, "Invalid Pool ID" );
    if( !nResId )
        return GetTextCollFromPool( RES_POOLCOLL_STANDARD );

    ResId aResId( nResId + nId, *pSwResMgr );
    OUString aNm( aResId );

    // A Set for all to-be-set Attributes
    SwAttrSet aSet( m_rDoc.GetAttrPool(), aTextFormatCollSetRange );
    sal_uInt16 nParent = GetPoolParent( nId );

    {

//FEATURE::CONDCOLL
        if(::IsConditionalByPoolId( nId ))
            pNewColl = new SwConditionTextFormatColl( m_rDoc.GetAttrPool(), aNm, !nParent
                                                ? m_rDoc.GetDfltTextFormatColl()
                                                : GetTextCollFromPool( nParent ));
        else
//FEATURE::CONDCOLL
            pNewColl = new SwTextFormatColl( m_rDoc.GetAttrPool(), aNm, !nParent
                                            ? m_rDoc.GetDfltTextFormatColl()
                                            : GetTextCollFromPool( nParent ));
        pNewColl->SetPoolFormatId( nId );
        m_rDoc.GetTextFormatColls()->push_back( pNewColl );
    }

    bool bNoDefault = m_rDoc.GetDocumentSettingManager().get( DocumentSettingId::STYLES_NODEFAULT );
    if ( !bNoDefault )
    {
        switch( nId )
        {
        // General content forms
        case RES_POOLCOLL_STANDARD:
            /* koreans do not like SvxScriptItem(TRUE) */
            if (bRegardLanguage)
            {
                sal_uLong nAppLanguage = GetAppLanguage();
                if (GetDefaultFrameDirection(nAppLanguage) ==
                    FRMDIR_HORI_RIGHT_TOP)
                {
                    SvxAdjustItem aAdjust(SVX_ADJUST_RIGHT, RES_PARATR_ADJUST );
                    aSet.Put(aAdjust);
                }
                if (nAppLanguage == LANGUAGE_KOREAN)
                {
                    SvxScriptSpaceItem aScriptSpace(false, RES_PARATR_SCRIPTSPACE);
                    aSet.Put(aScriptSpace);
                }
            }
            break;

        case RES_POOLCOLL_TEXT:                 // Text body
            {
                SvxLineSpacingItem aLSpc( LINE_SPACE_DEFAULT_HEIGHT, RES_PARATR_LINESPACING );
                SvxULSpaceItem aUL( 0, PT_7, RES_UL_SPACE );
                aLSpc.SetPropLineSpace( (const sal_uInt8) 120 );
                if( m_rDoc.GetDocumentSettingManager().get(DocumentSettingId::HTML_MODE) ) aUL.SetLower( HTML_PARSPACE );
                aSet.Put( aUL );
                aSet.Put( aLSpc );
            }
            break;
        case RES_POOLCOLL_TEXT_IDENT:           // Text body indentation
            {
                SvxLRSpaceItem aLR( RES_LR_SPACE );
                aLR.SetTextFirstLineOfst( GetMetricVal( CM_05 ));
                aSet.Put( aLR );
            }
            break;
        case RES_POOLCOLL_TEXT_NEGIDENT:        // Text body neg. indentation
            {
                SvxLRSpaceItem aLR( RES_LR_SPACE );
                aLR.SetTextFirstLineOfst( -(short)GetMetricVal( CM_05 ));
                aLR.SetTextLeft( GetMetricVal( CM_1 ));
                SvxTabStopItem aTStops(RES_PARATR_TABSTOP);
                aTStops.Insert( SvxTabStop( 0 ));

                aSet.Put( aLR );
                aSet.Put( aTStops );
            }
            break;
        case RES_POOLCOLL_TEXT_MOVE:            // Text body move
            {
                SvxLRSpaceItem aLR( RES_LR_SPACE );
                aLR.SetTextLeft( GetMetricVal( CM_05 ));
                aSet.Put( aLR );
            }
            break;

        case RES_POOLCOLL_CONFRONTATION:    // Text body confrontation
            {
                SvxLRSpaceItem aLR( RES_LR_SPACE );
                aLR.SetTextFirstLineOfst( - short( GetMetricVal( CM_1 ) * 4 +
                                                  GetMetricVal( CM_05)) );
                aLR.SetTextLeft( GetMetricVal( CM_1 ) * 5 );
                SvxTabStopItem aTStops( RES_PARATR_TABSTOP );
                aTStops.Insert( SvxTabStop( 0 ));

                aSet.Put( aLR );
                aSet.Put( aTStops );
            }
            break;
        case RES_POOLCOLL_MARGINAL:         // Text body marginal
            {
                SvxLRSpaceItem aLR( RES_LR_SPACE );
                aLR.SetTextLeft( GetMetricVal( CM_1 ) * 4 );
                aSet.Put( aLR );
            }
            break;

        case RES_POOLCOLL_HEADLINE_BASE:            // Base headline
            {
                static const sal_uInt16 aFontWhich[] =
                {   RES_CHRATR_FONT,
                    RES_CHRATR_CJK_FONT,
                    RES_CHRATR_CTL_FONT
                };
                static const sal_uInt16 aLangTypes[] =
                {
                    RES_CHRATR_LANGUAGE,
                    RES_CHRATR_CJK_LANGUAGE,
                    RES_CHRATR_CTL_LANGUAGE
                };
                static const sal_uInt16 aLangs[] =
                {
                    LANGUAGE_ENGLISH_US,
                    LANGUAGE_ENGLISH_US,
                    LANGUAGE_ARABIC_SAUDI_ARABIA
                };
                static const DefaultFontType nFontTypes[] =
                {
                    DefaultFontType::LATIN_HEADING,
                    DefaultFontType::CJK_HEADING,
                    DefaultFontType::CTL_HEADING
                };

                for( int i = 0; i < 3; ++i )
                {
                    sal_uInt16 nLng = static_cast<const SvxLanguageItem&>(m_rDoc.GetDefault( aLangTypes[i] )).GetLanguage();
                    if( LANGUAGE_DONTKNOW == nLng )
                        nLng = aLangs[i];

                    vcl::Font aFnt( OutputDevice::GetDefaultFont( nFontTypes[i],
                                            nLng, GetDefaultFontFlags::OnlyOne ) );

                    aSet.Put( SvxFontItem( aFnt.GetFamily(), aFnt.GetFamilyName(),
                                            OUString(), aFnt.GetPitch(),
                                            aFnt.GetCharSet(), aFontWhich[i] ));
                }

                SvxFontHeightItem aFntSize( PT_14, 100, RES_CHRATR_FONTSIZE );
                SvxULSpaceItem aUL( PT_12, PT_6, RES_UL_SPACE );
                if( m_rDoc.GetDocumentSettingManager().get(DocumentSettingId::HTML_MODE) )
                    aUL.SetLower( HTML_PARSPACE );
                aSet.Put( SvxFormatKeepItem( true, RES_KEEP ));

                pNewColl->SetNextTextFormatColl( *GetTextCollFromPool( RES_POOLCOLL_TEXT ));

                aSet.Put( aUL );
                SetAllScriptItem( aSet, aFntSize );
            }
            break;

        case RES_POOLCOLL_NUMBUL_BASE:          // Base Numbering
            break;

        case RES_POOLCOLL_GREETING:             // Greeting
        case RES_POOLCOLL_REGISTER_BASE:        // Base indexes
        case RES_POOLCOLL_SIGNATURE:            // Signatures
        case RES_POOLCOLL_TABLE:                // Tabele content
            {
                SwFormatLineNumber aLN;
                aLN.SetCountLines( false );
                aSet.Put( aLN );
            }
            break;

        case RES_POOLCOLL_HEADLINE1:        // Headinline 1
            {
                SvxULSpaceItem aUL( PT_12, PT_6, RES_UL_SPACE );
                aSet.Put( aUL );
                lcl_SetHeadline( &m_rDoc, pNewColl, aSet, nOutLvlBits, 0, false );
            }
            break;
        case RES_POOLCOLL_HEADLINE2:        // Headinline 2
            {
                SvxULSpaceItem aUL( PT_10, PT_6, RES_UL_SPACE );
                aSet.Put( aUL );
                lcl_SetHeadline( &m_rDoc, pNewColl, aSet, nOutLvlBits, 1, false );
            }
            break;
        case RES_POOLCOLL_HEADLINE3:        // Headinline 3
            {
                SvxULSpaceItem aUL( PT_7, PT_6, RES_UL_SPACE );
                aSet.Put( aUL );
                lcl_SetHeadline( &m_rDoc, pNewColl, aSet, nOutLvlBits, 2, false );
            }
            break;
        case RES_POOLCOLL_HEADLINE4:        // Headinline 4
            {
                SvxULSpaceItem aUL( PT_6, PT_6, RES_UL_SPACE );
                aSet.Put( aUL );
                lcl_SetHeadline( &m_rDoc, pNewColl, aSet, nOutLvlBits, 3, true );
            }
            break;
        case RES_POOLCOLL_HEADLINE5:        // Headinline 5
            {
                SvxULSpaceItem aUL( PT_6, PT_3, RES_UL_SPACE );
                aSet.Put( aUL );
                lcl_SetHeadline( &m_rDoc, pNewColl, aSet, nOutLvlBits, 4, false );
            }
            break;
        case RES_POOLCOLL_HEADLINE6:        // Headinline 6
            {
                SvxULSpaceItem aUL( PT_3, PT_3, RES_UL_SPACE );
                aSet.Put( aUL );
                lcl_SetHeadline( &m_rDoc, pNewColl, aSet, nOutLvlBits, 5, true );
            }
            break;
        case RES_POOLCOLL_HEADLINE7:        // Headinline 7
            {
                SvxULSpaceItem aUL( PT_3, PT_3, RES_UL_SPACE );
                aSet.Put( aUL );
                lcl_SetHeadline( &m_rDoc, pNewColl, aSet, nOutLvlBits, 6, false );
            }
            break;
        case RES_POOLCOLL_HEADLINE8:        // Headinline 8
            {
                SvxULSpaceItem aUL( PT_3, PT_3, RES_UL_SPACE );
                aSet.Put( aUL );
                lcl_SetHeadline( &m_rDoc, pNewColl, aSet, nOutLvlBits, 7, true );
            }
            break;
        case RES_POOLCOLL_HEADLINE9:        // Headinline 9
            {
                SvxULSpaceItem aUL( PT_3, PT_3, RES_UL_SPACE );
                aSet.Put( aUL );
                lcl_SetHeadline( &m_rDoc, pNewColl, aSet, nOutLvlBits, 8, false );
            }
            break;
        case RES_POOLCOLL_HEADLINE10:       // Headinline 10
            {
                SvxULSpaceItem aUL( PT_3, PT_3, RES_UL_SPACE );
                aSet.Put( aUL );
                lcl_SetHeadline( &m_rDoc, pNewColl, aSet, nOutLvlBits, 9, false );
            }
            break;

        // Special sections:
        // Header
        case RES_POOLCOLL_HEADER:
        case RES_POOLCOLL_HEADERL:
        case RES_POOLCOLL_HEADERR:
        // Footer
        case RES_POOLCOLL_FOOTER:
        case RES_POOLCOLL_FOOTERL:
        case RES_POOLCOLL_FOOTERR:
            {
                SwFormatLineNumber aLN;
                aLN.SetCountLines( false );
                aSet.Put( aLN );

                long nRightMargin = lcl_GetRightMargin( m_rDoc );

                SvxTabStopItem aTStops( 0, 0, SVX_TAB_ADJUST_DEFAULT, RES_PARATR_TABSTOP );
                aTStops.Insert( SvxTabStop( nRightMargin / 2, SVX_TAB_ADJUST_CENTER ) );
                aTStops.Insert( SvxTabStop( nRightMargin, SVX_TAB_ADJUST_RIGHT ) );

                aSet.Put( aTStops );
            }
            break;

        case RES_POOLCOLL_TABLE_HDLN:
            {
                SetAllScriptItem( aSet, SvxWeightItem( WEIGHT_BOLD, RES_CHRATR_WEIGHT ) );
                aSet.Put( SvxAdjustItem( SVX_ADJUST_CENTER, RES_PARATR_ADJUST ) );
                SwFormatLineNumber aLN;
                aLN.SetCountLines( false );
                aSet.Put( aLN );
            }
            break;

        case RES_POOLCOLL_FOOTNOTE:             // paragraph style Footnote
        case RES_POOLCOLL_ENDNOTE:              // paragraph style Endnote
            {
                SvxLRSpaceItem aLR( RES_LR_SPACE );
                aLR.SetTextFirstLineOfst( -(short)( GetMetricVal( CM_05 ) + GetMetricVal( CM_01 ) ) );
                aLR.SetTextLeft( GetMetricVal( CM_05 ) + GetMetricVal( CM_01 ) );
                SetAllScriptItem( aSet, SvxFontHeightItem( PT_10, 100, RES_CHRATR_FONTSIZE ) );
                aSet.Put( aLR );
                SwFormatLineNumber aLN;
                aLN.SetCountLines( false );
                aSet.Put( aLN );
            }
            break;

        case RES_POOLCOLL_LABEL:                // basic caption
            {
                SvxULSpaceItem aUL( RES_UL_SPACE );
                aUL.SetUpper( PT_6 );
                aUL.SetLower( PT_6 );
                aSet.Put( aUL );
                SetAllScriptItem( aSet, SvxPostureItem( ITALIC_NORMAL, RES_CHRATR_POSTURE ) );
                SetAllScriptItem( aSet, SvxFontHeightItem( PT_10, 100, RES_CHRATR_FONTSIZE ) );
                SwFormatLineNumber aLN;
                aLN.SetCountLines( false );
                aSet.Put( aLN );
            }
            break;

        case RES_POOLCOLL_FRAME:                // Frame content
        case RES_POOLCOLL_LABEL_ABB:            // caption image
        case RES_POOLCOLL_LABEL_TABLE:          // caption table
        case RES_POOLCOLL_LABEL_FRAME:          // caption frame
        case RES_POOLCOLL_LABEL_DRAWING:        // caption drawing
            break;

        case RES_POOLCOLL_JAKETADRESS:          // envelope address
            {
                SvxULSpaceItem aUL( RES_UL_SPACE );
                aUL.SetLower( PT_3 );
                aSet.Put( aUL );
                SwFormatLineNumber aLN;
                aLN.SetCountLines( false );
                aSet.Put( aLN );
            }
            break;

        case RES_POOLCOLL_SENDADRESS:           // Sender address
            {
                if( m_rDoc.GetDocumentSettingManager().get(DocumentSettingId::HTML_MODE) )
                    SetAllScriptItem( aSet, SvxPostureItem(ITALIC_NORMAL, RES_CHRATR_POSTURE) );
                else
                {
                    SvxULSpaceItem aUL( RES_UL_SPACE ); aUL.SetLower( PT_3 );
                    aSet.Put( aUL );
                }
                SwFormatLineNumber aLN;
                aLN.SetCountLines( false );
                aSet.Put( aLN );
            }
            break;

        // User defined indexes:
        case RES_POOLCOLL_TOX_USERH:            // Header
            lcl_SetRegister( &m_rDoc, aSet, 0, true, false );
            {
                SwFormatLineNumber aLN;
                aLN.SetCountLines( false );
                aSet.Put( aLN );
            }
            break;
        case RES_POOLCOLL_TOX_USER1:            // 1. Level
            lcl_SetRegister( &m_rDoc, aSet, 0, false, true );
            break;
        case RES_POOLCOLL_TOX_USER2:            // 2. Level
            lcl_SetRegister( &m_rDoc, aSet, 1, false, true );
            break;
        case RES_POOLCOLL_TOX_USER3:            // 3. Level
            lcl_SetRegister( &m_rDoc, aSet, 2, false, true );
            break;
        case RES_POOLCOLL_TOX_USER4:            // 4. Level
            lcl_SetRegister( &m_rDoc, aSet, 3, false, true );
            break;
        case RES_POOLCOLL_TOX_USER5:            // 5. Level
            lcl_SetRegister( &m_rDoc, aSet, 4, false, true );
            break;
        case RES_POOLCOLL_TOX_USER6:            // 6. Level
            lcl_SetRegister( &m_rDoc, aSet, 5, false, true );
            break;
        case RES_POOLCOLL_TOX_USER7:            // 7. Level
            lcl_SetRegister( &m_rDoc, aSet, 6, false, true );
            break;
        case RES_POOLCOLL_TOX_USER8:            // 8. Level
            lcl_SetRegister( &m_rDoc, aSet, 7, false, true );
            break;
        case RES_POOLCOLL_TOX_USER9:            // 9. Level
            lcl_SetRegister( &m_rDoc, aSet, 8, false, true );
            break;
        case RES_POOLCOLL_TOX_USER10:           // 10. Level
            lcl_SetRegister( &m_rDoc, aSet, 9, false, true );
            break;

        // Index
        case RES_POOLCOLL_TOX_IDXH:         // Header
            lcl_SetRegister( &m_rDoc, aSet, 0, true, false );
            {
                SwFormatLineNumber aLN;
                aLN.SetCountLines( false );
                aSet.Put( aLN );
            }
            break;
        case RES_POOLCOLL_TOX_IDX1:         // 1. Level
            lcl_SetRegister( &m_rDoc, aSet, 0, false, false );
            break;
        case RES_POOLCOLL_TOX_IDX2:         // 2. Level
            lcl_SetRegister( &m_rDoc, aSet, 1, false, false );
            break;
        case RES_POOLCOLL_TOX_IDX3:         // 3. Level
            lcl_SetRegister( &m_rDoc, aSet, 2, false, false );
            break;
        case RES_POOLCOLL_TOX_IDXBREAK:     // Trenner
            lcl_SetRegister( &m_rDoc, aSet, 0, false, false );
            break;

        // Table of Content
        case RES_POOLCOLL_TOX_CNTNTH:       // Header
            lcl_SetRegister( &m_rDoc, aSet, 0, true, false );
            {
                SwFormatLineNumber aLN;
                aLN.SetCountLines( false );
                aSet.Put( aLN );
            }
            break;
        case RES_POOLCOLL_TOX_CNTNT1:       // 1. Level
            lcl_SetRegister( &m_rDoc, aSet, 0, false, true );
            break;
        case RES_POOLCOLL_TOX_CNTNT2:       // 2. Level
            lcl_SetRegister( &m_rDoc, aSet, 1, false, true );
            break;
        case RES_POOLCOLL_TOX_CNTNT3:       // 3. Level
            lcl_SetRegister( &m_rDoc, aSet, 2, false, true );
            break;
        case RES_POOLCOLL_TOX_CNTNT4:       // 4. Level
            lcl_SetRegister( &m_rDoc, aSet, 3, false, true );
            break;
        case RES_POOLCOLL_TOX_CNTNT5:       // 5. Level
            lcl_SetRegister( &m_rDoc, aSet, 4, false, true );
            break;
        case RES_POOLCOLL_TOX_CNTNT6:       // 6. Level
            lcl_SetRegister( &m_rDoc, aSet, 5, false, true );
            break;
        case RES_POOLCOLL_TOX_CNTNT7:       // 7. Level
            lcl_SetRegister( &m_rDoc, aSet, 6, false, true );
            break;
        case RES_POOLCOLL_TOX_CNTNT8:       // 8. Level
            lcl_SetRegister( &m_rDoc, aSet, 7, false, true );
            break;
        case RES_POOLCOLL_TOX_CNTNT9:       // 9. Level
            lcl_SetRegister( &m_rDoc, aSet, 8, false, true );
            break;
        case RES_POOLCOLL_TOX_CNTNT10:      // 10. Level
            lcl_SetRegister( &m_rDoc, aSet, 9, false, true );
            break;

        case RES_POOLCOLL_TOX_ILLUSH:
        case RES_POOLCOLL_TOX_OBJECTH:
        case RES_POOLCOLL_TOX_TABLESH:
        case RES_POOLCOLL_TOX_AUTHORITIESH:
            lcl_SetRegister( &m_rDoc, aSet, 0, true, false );
            {
                SwFormatLineNumber aLN;
                aLN.SetCountLines( false );
                aSet.Put( aLN );
            }
            break;
        case RES_POOLCOLL_TOX_ILLUS1:
        case RES_POOLCOLL_TOX_OBJECT1:
        case RES_POOLCOLL_TOX_TABLES1:
        case RES_POOLCOLL_TOX_AUTHORITIES1:
            lcl_SetRegister( &m_rDoc, aSet, 0, false, true );
        break;

        case RES_POOLCOLL_NUM_LEVEL1S:
            lcl_SetNumBul( &m_rDoc, pNewColl, aSet, RES_POOLCOLL_NUM_LEVEL1,
                            lNumFirstLineOffset, SwNumRule::GetNumIndent( 0 ),
                            PT_12, PT_6 );
            break;
        case RES_POOLCOLL_NUM_LEVEL1:
            lcl_SetNumBul( &m_rDoc, pNewColl, aSet, RES_POOLCOLL_NUM_LEVEL1,
                            lNumFirstLineOffset, SwNumRule::GetNumIndent( 0 ),
                            0, PT_6 );
            break;
        case RES_POOLCOLL_NUM_LEVEL1E:
            lcl_SetNumBul( &m_rDoc, pNewColl, aSet, RES_POOLCOLL_NUM_LEVEL1,
                            lNumFirstLineOffset, SwNumRule::GetNumIndent( 0 ),
                            0, PT_12 );
            break;
        case RES_POOLCOLL_NUM_NONUM1:
            lcl_SetNumBul( &m_rDoc, pNewColl, aSet, RES_POOLCOLL_NUM_NONUM1,
                            0, SwNumRule::GetNumIndent( 0 ), 0, PT_6 );
            break;
        case RES_POOLCOLL_NUM_LEVEL2S:
            lcl_SetNumBul( &m_rDoc, pNewColl, aSet, RES_POOLCOLL_NUM_LEVEL2,
                            lNumFirstLineOffset, SwNumRule::GetNumIndent( 1 ),
                            PT_12, PT_6 );
            break;
        case RES_POOLCOLL_NUM_LEVEL2:
            lcl_SetNumBul( &m_rDoc, pNewColl, aSet, RES_POOLCOLL_NUM_LEVEL2,
                            lNumFirstLineOffset, SwNumRule::GetNumIndent( 1 ),
                            0, PT_6 );
            break;
        case RES_POOLCOLL_NUM_LEVEL2E:
            lcl_SetNumBul( &m_rDoc, pNewColl, aSet, RES_POOLCOLL_NUM_LEVEL2,
                            lNumFirstLineOffset, SwNumRule::GetNumIndent( 1 ),
                            0, PT_12 );
            break;
        case RES_POOLCOLL_NUM_NONUM2:
            lcl_SetNumBul( &m_rDoc, pNewColl, aSet, RES_POOLCOLL_NUM_NONUM2,
                            0, SwNumRule::GetNumIndent( 1 ), 0, PT_6 );
            break;
        case RES_POOLCOLL_NUM_LEVEL3S:
            lcl_SetNumBul( &m_rDoc, pNewColl, aSet, RES_POOLCOLL_NUM_LEVEL3,
                            lNumFirstLineOffset, SwNumRule::GetNumIndent( 2 ),
                            PT_12, PT_6 );
            break;
        case RES_POOLCOLL_NUM_LEVEL3:
            lcl_SetNumBul( &m_rDoc, pNewColl, aSet, RES_POOLCOLL_NUM_LEVEL3,
                            lNumFirstLineOffset, SwNumRule::GetNumIndent( 2 ),
                            0, PT_6 );
            break;
        case RES_POOLCOLL_NUM_LEVEL3E:
            lcl_SetNumBul( &m_rDoc, pNewColl, aSet, RES_POOLCOLL_NUM_LEVEL3,
                            lNumFirstLineOffset, SwNumRule::GetNumIndent( 2 ),
                            0, PT_12 );
            break;
        case RES_POOLCOLL_NUM_NONUM3:
            lcl_SetNumBul( &m_rDoc, pNewColl, aSet, RES_POOLCOLL_NUM_NONUM3,
                            0, SwNumRule::GetNumIndent( 2 ), 0, PT_6 );
            break;
        case RES_POOLCOLL_NUM_LEVEL4S:
            lcl_SetNumBul( &m_rDoc, pNewColl, aSet, RES_POOLCOLL_NUM_LEVEL4,
                            lNumFirstLineOffset, SwNumRule::GetNumIndent( 3 ),
                            PT_12, PT_6 );
            break;
        case RES_POOLCOLL_NUM_LEVEL4:
            lcl_SetNumBul( &m_rDoc, pNewColl, aSet, RES_POOLCOLL_NUM_LEVEL4,
                            lNumFirstLineOffset, SwNumRule::GetNumIndent( 3 ),
                            0, PT_6 );
            break;
        case RES_POOLCOLL_NUM_LEVEL4E:
            lcl_SetNumBul( &m_rDoc, pNewColl, aSet, RES_POOLCOLL_NUM_LEVEL4,
                            lNumFirstLineOffset, SwNumRule::GetNumIndent( 3 ),
                            0, PT_12 );
            break;
        case RES_POOLCOLL_NUM_NONUM4:
            lcl_SetNumBul( &m_rDoc, pNewColl, aSet, RES_POOLCOLL_NUM_NONUM4,
                            0, SwNumRule::GetNumIndent( 3 ), 0, PT_6 );
            break;
        case RES_POOLCOLL_NUM_LEVEL5S:
            lcl_SetNumBul( &m_rDoc, pNewColl, aSet, RES_POOLCOLL_NUM_LEVEL5,
                            lNumFirstLineOffset, SwNumRule::GetNumIndent( 4 ),
                            PT_12, PT_6 );
            break;
        case RES_POOLCOLL_NUM_LEVEL5:
            lcl_SetNumBul( &m_rDoc, pNewColl, aSet, RES_POOLCOLL_NUM_LEVEL5,
                            lNumFirstLineOffset, SwNumRule::GetNumIndent( 4 ),
                            0, PT_6 );
            break;
        case RES_POOLCOLL_NUM_LEVEL5E:
            lcl_SetNumBul( &m_rDoc, pNewColl, aSet, RES_POOLCOLL_NUM_LEVEL5,
                            lNumFirstLineOffset, SwNumRule::GetNumIndent( 4 ),
                            0, PT_12 );
            break;
        case RES_POOLCOLL_NUM_NONUM5:
            lcl_SetNumBul( &m_rDoc, pNewColl, aSet, RES_POOLCOLL_NUM_NONUM5,
                            0, SwNumRule::GetNumIndent( 4 ), 0, PT_6 );
            break;

        case RES_POOLCOLL_BUL_LEVEL1S:
            lcl_SetNumBul( &m_rDoc, pNewColl, aSet, RES_POOLCOLL_BUL_LEVEL1,
                            lBullFirstLineOffset, SwNumRule::GetBullIndent( 0 ),
                            PT_12, PT_6 );
            break;
        case RES_POOLCOLL_BUL_LEVEL1:
            lcl_SetNumBul( &m_rDoc, pNewColl, aSet, RES_POOLCOLL_BUL_LEVEL1,
                            lBullFirstLineOffset, SwNumRule::GetBullIndent( 0 ),
                            0, PT_6 );
            break;
        case RES_POOLCOLL_BUL_LEVEL1E:
            lcl_SetNumBul( &m_rDoc, pNewColl, aSet, RES_POOLCOLL_BUL_LEVEL1,
                            lBullFirstLineOffset, SwNumRule::GetBullIndent( 0 ),
                            0, PT_12 );
            break;
        case RES_POOLCOLL_BUL_NONUM1:
            lcl_SetNumBul( &m_rDoc, pNewColl, aSet, RES_POOLCOLL_BUL_NONUM1,
                            0, SwNumRule::GetBullIndent( 0 ), 0, PT_6 );
            break;
        case RES_POOLCOLL_BUL_LEVEL2S:
            lcl_SetNumBul( &m_rDoc, pNewColl, aSet, RES_POOLCOLL_BUL_LEVEL2,
                            lBullFirstLineOffset, SwNumRule::GetBullIndent( 1 ),
                            PT_12, PT_6 );
            break;
        case RES_POOLCOLL_BUL_LEVEL2:
            lcl_SetNumBul( &m_rDoc, pNewColl, aSet, RES_POOLCOLL_BUL_LEVEL2,
                            lBullFirstLineOffset, SwNumRule::GetBullIndent( 1 ),
                            0, PT_6 );
            break;
        case RES_POOLCOLL_BUL_LEVEL2E:
            lcl_SetNumBul( &m_rDoc, pNewColl, aSet, RES_POOLCOLL_BUL_LEVEL2,
                            lBullFirstLineOffset, SwNumRule::GetBullIndent( 1 ),
                            0, PT_12 );
            break;
        case RES_POOLCOLL_BUL_NONUM2:
            lcl_SetNumBul( &m_rDoc, pNewColl, aSet, RES_POOLCOLL_BUL_NONUM2,
                            0, SwNumRule::GetBullIndent( 1 ), 0, PT_6 );
            break;
        case RES_POOLCOLL_BUL_LEVEL3S:
            lcl_SetNumBul( &m_rDoc, pNewColl, aSet, RES_POOLCOLL_BUL_LEVEL3,
                            lBullFirstLineOffset, SwNumRule::GetBullIndent( 2 ),
                            PT_12, PT_6 );
            break;
        case RES_POOLCOLL_BUL_LEVEL3:
            lcl_SetNumBul( &m_rDoc, pNewColl, aSet, RES_POOLCOLL_BUL_LEVEL3,
                            lBullFirstLineOffset, SwNumRule::GetBullIndent( 2 ),
                            0, PT_6 );
            break;
        case RES_POOLCOLL_BUL_LEVEL3E:
            lcl_SetNumBul( &m_rDoc, pNewColl, aSet, RES_POOLCOLL_BUL_LEVEL3,
                            lBullFirstLineOffset, SwNumRule::GetBullIndent( 2 ),
                            0, PT_12 );
            break;
        case RES_POOLCOLL_BUL_NONUM3:
            lcl_SetNumBul( &m_rDoc, pNewColl, aSet, RES_POOLCOLL_BUL_NONUM3,
                            0, SwNumRule::GetBullIndent( 2 ), 0, PT_6 );
            break;
        case RES_POOLCOLL_BUL_LEVEL4S:
            lcl_SetNumBul( &m_rDoc, pNewColl, aSet, RES_POOLCOLL_BUL_LEVEL4,
                            lBullFirstLineOffset, SwNumRule::GetBullIndent( 3 ),
                            PT_12, PT_6 );
            break;
        case RES_POOLCOLL_BUL_LEVEL4:
            lcl_SetNumBul( &m_rDoc, pNewColl, aSet, RES_POOLCOLL_BUL_LEVEL4,
                            lBullFirstLineOffset, SwNumRule::GetBullIndent( 3 ),
                            0, PT_6 );
            break;
        case RES_POOLCOLL_BUL_LEVEL4E:
            lcl_SetNumBul( &m_rDoc, pNewColl, aSet, RES_POOLCOLL_BUL_LEVEL4,
                            lBullFirstLineOffset, SwNumRule::GetBullIndent( 3 ),
                            0, PT_12 );
            break;
        case RES_POOLCOLL_BUL_NONUM4:
            lcl_SetNumBul( &m_rDoc, pNewColl, aSet, RES_POOLCOLL_BUL_NONUM4,
                            0, SwNumRule::GetBullIndent( 3 ), 0, PT_6 );
            break;
        case RES_POOLCOLL_BUL_LEVEL5S:
            lcl_SetNumBul( &m_rDoc, pNewColl, aSet, RES_POOLCOLL_BUL_LEVEL5,
                            lBullFirstLineOffset, SwNumRule::GetBullIndent( 4 ),
                            PT_12, PT_6 );
            break;
        case RES_POOLCOLL_BUL_LEVEL5:
            lcl_SetNumBul( &m_rDoc, pNewColl, aSet, RES_POOLCOLL_BUL_LEVEL5,
                            lBullFirstLineOffset, SwNumRule::GetBullIndent( 4 ),
                            0, PT_6 );
            break;
        case RES_POOLCOLL_BUL_LEVEL5E:
            lcl_SetNumBul( &m_rDoc, pNewColl, aSet, RES_POOLCOLL_BUL_LEVEL5,
                            lBullFirstLineOffset, SwNumRule::GetBullIndent( 4 ),
                            0, PT_12 );
            break;
        case RES_POOLCOLL_BUL_NONUM5:
            lcl_SetNumBul( &m_rDoc, pNewColl, aSet, RES_POOLCOLL_BUL_NONUM5,
                            0, SwNumRule::GetBullIndent( 4 ), 0, PT_6 );
            break;

        case RES_POOLCOLL_DOC_TITEL:            // Document Title
            {
                SetAllScriptItem( aSet, SvxWeightItem( WEIGHT_BOLD, RES_CHRATR_WEIGHT ) );
                SetAllScriptItem( aSet, SvxFontHeightItem( PT_28, 100, RES_CHRATR_FONTSIZE ) );

                aSet.Put( SvxAdjustItem( SVX_ADJUST_CENTER, RES_PARATR_ADJUST ) );

                pNewColl->SetNextTextFormatColl( *GetTextCollFromPool( RES_POOLCOLL_TEXT ));
            }
            break;

        case RES_POOLCOLL_DOC_SUBTITEL:         // Document subtitle
            {
                SvxULSpaceItem aUL( PT_3, PT_6, RES_UL_SPACE );
                aSet.Put( aUL );
                SetAllScriptItem( aSet, SvxFontHeightItem( PT_18, 100, RES_CHRATR_FONTSIZE ));

                aSet.Put( SvxAdjustItem( SVX_ADJUST_CENTER, RES_PARATR_ADJUST ));

                pNewColl->SetNextTextFormatColl( *GetTextCollFromPool( RES_POOLCOLL_TEXT ));
            }
            break;

        case RES_POOLCOLL_HTML_BLOCKQUOTE:
            {
                SvxLRSpaceItem aLR( RES_LR_SPACE );
                aLR.SetLeft( GetMetricVal( CM_1 ));
                aLR.SetRight( GetMetricVal( CM_1 ));
                aSet.Put( aLR );
                SvxULSpaceItem aUL( RES_UL_SPACE );
                aUL = pNewColl->GetULSpace();
                aUL.SetLower( HTML_PARSPACE );
                aSet.Put( aUL);
            }
            break;

        case RES_POOLCOLL_HTML_PRE:
            {
                ::lcl_SetDfltFont( DefaultFontType::FIXED, aSet );

                // WORKAROUND: Set PRE to 10pt
                SetAllScriptItem( aSet, SvxFontHeightItem(PT_10, 100, RES_CHRATR_FONTSIZE) );

                // The lower paragraph distance is set explicitly (makes
                // assigning hard attributes easier)
                SvxULSpaceItem aULSpaceItem( RES_UL_SPACE );
                aULSpaceItem = pNewColl->GetULSpace();
                aULSpaceItem.SetLower( 0 );
                aSet.Put( aULSpaceItem );
            }
            break;

        case RES_POOLCOLL_HTML_HR:
            {
                SvxBoxItem aBox( RES_BOX );
                Color aColor( COL_GRAY );
                SvxBorderLine aNew(&aColor, 1, table::BorderLineStyle::DOUBLE);
                aBox.SetLine( &aNew, SvxBoxItemLine::BOTTOM );

                aSet.Put( aBox );
                aSet.Put( SwParaConnectBorderItem( false ) );
                SetAllScriptItem( aSet, SvxFontHeightItem(120, 100, RES_CHRATR_FONTSIZE) );

                SvxULSpaceItem aUL( RES_UL_SPACE );
                {
                    pNewColl->SetNextTextFormatColl( *GetTextCollFromPool( RES_POOLCOLL_TEXT ));
                    aUL = pNewColl->GetULSpace();
                }
                aUL.SetLower( HTML_PARSPACE );
                aSet.Put( aUL);
                SwFormatLineNumber aLN;
                aLN.SetCountLines( false );
                aSet.Put( aLN );
            }
            break;

        case RES_POOLCOLL_HTML_DD:
            {
                SvxLRSpaceItem aLR( RES_LR_SPACE );
                aLR = pNewColl->GetLRSpace();
                // We indent by 1 cm. The IDs are always 2 away from each other!
                aLR.SetLeft( GetMetricVal( CM_1 ));
                aSet.Put( aLR );
            }
            break;
        case RES_POOLCOLL_HTML_DT:
            {
                SvxLRSpaceItem aLR( RES_LR_SPACE );
                {
                    pNewColl->SetNextTextFormatColl( *GetTextCollFromPool( RES_POOLCOLL_HTML_DD ));
                    aLR = pNewColl->GetLRSpace();
                }
                // We indent by 0 cm. The IDs are always 2 away from each other!
                aLR.SetLeft( 0 );
                aSet.Put( aLR );
            }
            break;
        }
    }

    if( aSet.Count() )
        pNewColl->SetFormatAttr( aSet );
    return pNewColl;
}

/// Return the AutomaticFormat with the supplied Id. If it doesn't
/// exist, create it.
SwFormat* DocumentStylePoolManager::GetFormatFromPool( sal_uInt16 nId )
{
    SwFormat *pNewFormat = nullptr;
    SwFormat *pDeriveFormat = nullptr;

    SwFormatsBase* pArray[ 2 ];
    sal_uInt16 nArrCnt = 1, nRCId = 0;
    sal_uInt16* pWhichRange = nullptr;

    switch( nId & (COLL_GET_RANGE_BITS + POOLGRP_NOCOLLID) )
    {
    case POOLGRP_CHARFMT:
        {
            pArray[0] = m_rDoc.GetCharFormats();
            pDeriveFormat = m_rDoc.GetDfltCharFormat();

            if( nId > RES_POOLCHR_NORMAL_END )
                nRCId = RC_POOLCHRFMT_HTML_BEGIN - RES_POOLCHR_HTML_BEGIN;
            else
                nRCId = RC_POOLCHRFMT_BEGIN - RES_POOLCHR_BEGIN;
            pWhichRange =  aCharFormatSetRange;

            // Fault: unknown Format, but a CharFormat
            //             -> return the first one
            if( RES_POOLCHR_BEGIN > nId || nId >= RES_POOLCHR_END )
            {
                OSL_ENSURE( false, "invalid Id" );
                nId = RES_POOLCHR_BEGIN;
            }
        }
        break;
    case POOLGRP_FRAMEFMT:
        {
            pArray[0] = m_rDoc.GetFrameFormats();
            pArray[1] = m_rDoc.GetSpzFrameFormats();
            pDeriveFormat = m_rDoc.GetDfltFrameFormat();
            nArrCnt = 2;
            nRCId = RC_POOLFRMFMT_BEGIN - RES_POOLFRM_BEGIN;
            pWhichRange = aFrameFormatSetRange;

            // Fault: unknown Format, but a FrameFormat
            //             -> return the first one
            if( RES_POOLFRM_BEGIN > nId || nId >= RES_POOLFRM_END )
            {
                OSL_ENSURE( false, "invalid Id" );
                nId = RES_POOLFRM_BEGIN;
            }
        }
        break;

    default:
        // Fault, unknown Format
        OSL_ENSURE( nId, "invalid Id" );
        return nullptr;
    }
    OSL_ENSURE( nRCId, "invalid Id" );

    while( nArrCnt-- )
        for( size_t n = 0; n < (*pArray[nArrCnt]).GetFormatCount(); ++n )
            if( nId == ( pNewFormat = (*pArray[ nArrCnt ] ).GetFormat( n ) )->
                    GetPoolFormatId() )
            {
                return pNewFormat;
            }

    ResId aResId( nRCId + nId, *pSwResMgr );
    OUString aNm( aResId );
    SwAttrSet aSet( m_rDoc.GetAttrPool(), pWhichRange );

    {
        bool bIsModified = m_rDoc.getIDocumentState().IsModified();

        {
            ::sw::UndoGuard const undoGuard(m_rDoc.GetIDocumentUndoRedo());
            switch (nId & (COLL_GET_RANGE_BITS + POOLGRP_NOCOLLID) )
            {
                case POOLGRP_CHARFMT:
                    pNewFormat = m_rDoc._MakeCharFormat(aNm, pDeriveFormat, false, true);
                    break;
                case POOLGRP_FRAMEFMT:
                    pNewFormat = m_rDoc._MakeFrameFormat(aNm, pDeriveFormat, false, true);
                    break;
                default:
                    break;
            }
        }

        if( !bIsModified )
            m_rDoc.getIDocumentState().ResetModified();
        pNewFormat->SetPoolFormatId( nId );
        pNewFormat->SetAuto(false);      // no AutoFormat
    }

    switch( nId )
    {
    case RES_POOLCHR_FOOTNOTE:              // Footnote
    case RES_POOLCHR_PAGENO:                // Page/Field
    case RES_POOLCHR_LABEL:                 // Label
    case RES_POOLCHR_DROPCAPS:              // Dropcaps
    case RES_POOLCHR_NUM_LEVEL:             // Numbering level
    case RES_POOLCHR_TOXJUMP:               // Table of contents jump
    case RES_POOLCHR_ENDNOTE:               // Endnote
    case RES_POOLCHR_LINENUM:               // Line numbering
        break;

    case RES_POOLCHR_ENDNOTE_ANCHOR:        // Endnote anchor
    case RES_POOLCHR_FOOTNOTE_ANCHOR:       // Footnote anchor
        {
            aSet.Put( SvxEscapementItem( DFLT_ESC_AUTO_SUPER, 58, RES_CHRATR_ESCAPEMENT ) );
        }
        break;

    case RES_POOLCHR_BUL_LEVEL:             // Bullet character
        {
            const vcl::Font& rBulletFont = numfunc::GetDefBulletFont();
            SetAllScriptItem( aSet, SvxFontItem( rBulletFont.GetFamily(),
                        rBulletFont.GetFamilyName(), rBulletFont.GetStyleName(),
                        rBulletFont.GetPitch(), rBulletFont.GetCharSet(), RES_CHRATR_FONT ));
        }
        break;

    case RES_POOLCHR_INET_NORMAL:
        {
            Color aCol( COL_BLUE );
            aSet.Put( SvxColorItem( aCol, RES_CHRATR_COLOR ) );
            aSet.Put( SvxUnderlineItem( UNDERLINE_SINGLE, RES_CHRATR_UNDERLINE ) );
            // i40133: patch submitted by rail: set language to 'none' to prevent spell checking:
            aSet.Put( SvxLanguageItem( LANGUAGE_NONE, RES_CHRATR_LANGUAGE ) );
            aSet.Put( SvxLanguageItem( LANGUAGE_NONE, RES_CHRATR_CJK_LANGUAGE ) );
            aSet.Put( SvxLanguageItem( LANGUAGE_NONE, RES_CHRATR_CTL_LANGUAGE ) );
        }
        break;
    case RES_POOLCHR_INET_VISIT:
        {
            Color aCol( COL_RED );
            aSet.Put( SvxColorItem( aCol, RES_CHRATR_COLOR ) );
            aSet.Put( SvxUnderlineItem( UNDERLINE_SINGLE, RES_CHRATR_UNDERLINE ) );
            aSet.Put( SvxLanguageItem( LANGUAGE_NONE, RES_CHRATR_LANGUAGE ) );
            aSet.Put( SvxLanguageItem( LANGUAGE_NONE, RES_CHRATR_CJK_LANGUAGE ) );
            aSet.Put( SvxLanguageItem( LANGUAGE_NONE, RES_CHRATR_CTL_LANGUAGE ) );
        }
        break;
    case RES_POOLCHR_JUMPEDIT:
        {
            Color aCol( COL_CYAN );
            aSet.Put( SvxColorItem( aCol, RES_CHRATR_COLOR ) );
            aSet.Put( SvxUnderlineItem( UNDERLINE_DOTTED, RES_CHRATR_UNDERLINE ) );
            aSet.Put( SvxCaseMapItem( SVX_CASEMAP_KAPITAELCHEN, RES_CHRATR_CASEMAP ) );
        }
        break;

    case RES_POOLCHR_RUBYTEXT:
        {
            long nH = static_cast<const SvxFontHeightItem*>(GetDfltAttr(
                                RES_CHRATR_CJK_FONTSIZE ))->GetHeight() / 2;
            SetAllScriptItem( aSet, SvxFontHeightItem( nH, 100, RES_CHRATR_FONTSIZE));
            aSet.Put(SvxUnderlineItem( UNDERLINE_NONE, RES_CHRATR_UNDERLINE ));
            aSet.Put(SvxEmphasisMarkItem( EMPHASISMARK_NONE, RES_CHRATR_EMPHASIS_MARK) );
        }
        break;

    case RES_POOLCHR_HTML_EMPHASIS:
    case RES_POOLCHR_HTML_CITIATION:
    case RES_POOLCHR_HTML_VARIABLE:
        {
            SetAllScriptItem( aSet, SvxPostureItem( ITALIC_NORMAL, RES_CHRATR_POSTURE) );
        }
        break;

    case RES_POOLCHR_IDX_MAIN_ENTRY:
    case RES_POOLCHR_HTML_STRONG:
        {
            SetAllScriptItem( aSet, SvxWeightItem( WEIGHT_BOLD, RES_CHRATR_WEIGHT ));
        }
        break;

    case RES_POOLCHR_HTML_CODE:
    case RES_POOLCHR_HTML_SAMPLE:
    case RES_POOLCHR_HTML_KEYBOARD:
    case RES_POOLCHR_HTML_TELETYPE:
        {
            ::lcl_SetDfltFont( DefaultFontType::FIXED, aSet );
        }
        break;
   case RES_POOLCHR_VERT_NUM:
            aSet.Put( SvxCharRotateItem( 900, false, RES_CHRATR_ROTATE ) );
    break;

    case RES_POOLFRM_FRAME:
        {
            if ( m_rDoc.GetDocumentSettingManager().get(DocumentSettingId::HTML_MODE) )
            {
                aSet.Put( SwFormatAnchor( FLY_AS_CHAR ));
                aSet.Put( SwFormatVertOrient( 0, text::VertOrientation::LINE_CENTER, text::RelOrientation::PRINT_AREA ) );
                aSet.Put( SwFormatSurround( SURROUND_NONE ) );
            }
            else
            {
                aSet.Put( SwFormatAnchor( FLY_AT_PARA ));
                aSet.Put( SwFormatSurround( SURROUND_PARALLEL ) );
                aSet.Put( SwFormatHoriOrient( 0, text::HoriOrientation::CENTER, text::RelOrientation::PRINT_AREA ) );
                aSet.Put( SwFormatVertOrient( 0, text::VertOrientation::TOP, text::RelOrientation::PRINT_AREA ) );
                Color aCol( COL_BLACK );
                SvxBorderLine aLine( &aCol, DEF_LINE_WIDTH_0 );
                SvxBoxItem aBox( RES_BOX );
                aBox.SetLine( &aLine, SvxBoxItemLine::TOP );
                aBox.SetLine( &aLine, SvxBoxItemLine::BOTTOM );
                aBox.SetLine( &aLine, SvxBoxItemLine::LEFT );
                aBox.SetLine( &aLine, SvxBoxItemLine::RIGHT );
                aBox.SetDistance( 85 );
                aSet.Put( aBox );
                aSet.Put( SvxLRSpaceItem( 114, 114, 0, 0, RES_LR_SPACE ) );
                aSet.Put( SvxULSpaceItem( 114, 114, RES_UL_SPACE ) );
            }

            //UUUU for styles of FlyFrames do not set the FillStyle to make it a derived attribute
            aSet.ClearItem(XATTR_FILLSTYLE);
        }
        break;
    case RES_POOLFRM_GRAPHIC:
    case RES_POOLFRM_OLE:
        {
            aSet.Put( SwFormatAnchor( FLY_AT_PARA ));
            aSet.Put( SwFormatHoriOrient( 0, text::HoriOrientation::CENTER, text::RelOrientation::FRAME ));
            aSet.Put( SwFormatVertOrient( 0, text::VertOrientation::TOP, text::RelOrientation::FRAME ));
            aSet.Put( SwFormatSurround( SURROUND_IDEAL ));
        }
        break;
    case RES_POOLFRM_FORMEL:
        {
            aSet.Put( SwFormatAnchor( FLY_AS_CHAR ) );
            aSet.Put( SwFormatVertOrient( 0, text::VertOrientation::CHAR_CENTER, text::RelOrientation::FRAME ) );
            aSet.Put( SvxLRSpaceItem( 114, 114, 0, 0, RES_LR_SPACE ) );
        }
        break;
    case RES_POOLFRM_MARGINAL:
        {
            aSet.Put( SwFormatAnchor( FLY_AT_PARA ));
            aSet.Put( SwFormatHoriOrient( 0, text::HoriOrientation::LEFT, text::RelOrientation::FRAME ));
            aSet.Put( SwFormatVertOrient( 0, text::VertOrientation::TOP, text::RelOrientation::FRAME ));
            aSet.Put( SwFormatSurround( SURROUND_PARALLEL ));
            // Set the default width to 3.5 cm, use the minimum value for the height
            aSet.Put( SwFormatFrameSize( ATT_MIN_SIZE,
                    GetMetricVal( CM_1 ) * 3 + GetMetricVal( CM_05 ),
                    MM50 ));
        }
        break;
    case RES_POOLFRM_WATERSIGN:
        {
            aSet.Put( SwFormatAnchor( FLY_AT_PAGE ));
            aSet.Put( SwFormatHoriOrient( 0, text::HoriOrientation::CENTER, text::RelOrientation::FRAME ));
            aSet.Put( SwFormatVertOrient( 0, text::VertOrientation::CENTER, text::RelOrientation::FRAME ));
            aSet.Put( SvxOpaqueItem( RES_OPAQUE, false ));
            aSet.Put( SwFormatSurround( SURROUND_THROUGHT ));
        }
        break;
    case RES_POOLFRM_LABEL:
        {
            aSet.Put( SwFormatAnchor( FLY_AS_CHAR ) );
            aSet.Put( SwFormatVertOrient( 0, text::VertOrientation::TOP, text::RelOrientation::FRAME ) );
            aSet.Put( SvxLRSpaceItem( 114, 114, 0, 0, RES_LR_SPACE ) );

            SvxProtectItem aProtect( RES_PROTECT );
            aProtect.SetSizeProtect( true );
            aProtect.SetPosProtect( true );
            aSet.Put( aProtect );

            pNewFormat->SetAutoUpdateFormat();
        }
        break;
    }
    if( aSet.Count() )
    {
        {
            pNewFormat->SetFormatAttr( aSet );
        }
    }
    return pNewFormat;
}

SwFrameFormat* DocumentStylePoolManager::GetFrameFormatFromPool( sal_uInt16 nId )
{
    return static_cast<SwFrameFormat*>(GetFormatFromPool( nId ));
}

SwCharFormat* DocumentStylePoolManager::GetCharFormatFromPool( sal_uInt16 nId )
{
    return static_cast<SwCharFormat*>(GetFormatFromPool( nId ));
}

SwPageDesc* DocumentStylePoolManager::GetPageDescFromPool( sal_uInt16 nId, bool bRegardLanguage )
{
    OSL_ENSURE( RES_POOLPAGE_BEGIN <= nId && nId < RES_POOLPAGE_END,
            "Wrong AutoFormat Id" );

    for( size_t n = 0; n < m_rDoc.GetPageDescCnt(); ++n )
    {
        if ( nId == m_rDoc.GetPageDesc(n).GetPoolFormatId() )
        {
            return &m_rDoc.GetPageDesc(n);
        }
    }

    if( RES_POOLPAGE_BEGIN > nId ||  nId >= RES_POOLPAGE_END )
    {
        // unknown page pool ID
        OSL_ENSURE( false, "<SwDoc::GetPageDescFromPool(..)> - unknown page pool ID" );
        nId = RES_POOLPAGE_BEGIN;
    }

    SwPageDesc* pNewPgDsc = nullptr;
    {
        const ResId aResId( sal_uInt32(RC_POOLPAGEDESC_BEGIN + nId - RES_POOLPAGE_BEGIN), *pSwResMgr );
        const OUString aNm( aResId );
        const bool bIsModified = m_rDoc.getIDocumentState().IsModified();

        {
            ::sw::UndoGuard const undoGuard(m_rDoc.GetIDocumentUndoRedo());
            pNewPgDsc = m_rDoc.MakePageDesc(aNm, nullptr, bRegardLanguage);
        }

        pNewPgDsc->SetPoolFormatId( nId );
        if ( !bIsModified )
        {
            m_rDoc.getIDocumentState().ResetModified();
        }
    }

    SvxLRSpaceItem aLR( RES_LR_SPACE );
    {
        aLR.SetLeft( GetMetricVal( CM_1 ) * 2 );
        aLR.SetRight( aLR.GetLeft() );
    }
    SvxULSpaceItem aUL( RES_UL_SPACE );
    {
        aUL.SetUpper( (sal_uInt16)aLR.GetLeft() );
        aUL.SetLower( (sal_uInt16)aLR.GetLeft() );
    }

    SwAttrSet aSet( m_rDoc.GetAttrPool(), aPgFrameFormatSetRange );
    bool bSetLeft = true;

    switch( nId )
    {
    case RES_POOLPAGE_STANDARD:     // "Default"
        {
            aSet.Put( aLR );
            aSet.Put( aUL );
            pNewPgDsc->SetUseOn( nsUseOnPage::PD_ALL | nsUseOnPage::PD_FIRSTSHARE );
        }
        break;

    case RES_POOLPAGE_FIRST:        // "First Page"
    case RES_POOLPAGE_REGISTER:     // "Index"
        {
            lcl_PutStdPageSizeIntoItemSet( &m_rDoc, aSet );
            aSet.Put( aLR );
            aSet.Put( aUL );
            pNewPgDsc->SetUseOn( nsUseOnPage::PD_ALL );
            if( RES_POOLPAGE_FIRST == nId )
                pNewPgDsc->SetFollow( GetPageDescFromPool( RES_POOLPAGE_STANDARD ));
        }
        break;

    case RES_POOLPAGE_LEFT:         // "Left Page"
        {
            lcl_PutStdPageSizeIntoItemSet( &m_rDoc, aSet );
            aSet.Put( aLR );
            aSet.Put( aUL );
            bSetLeft = false;
            pNewPgDsc->SetUseOn( nsUseOnPage::PD_LEFT );
            // this relies on GetPageDescFromPool() not going into infinite recursion
            // (by this point RES_POOLPAGE_LEFT will not reach this place again)
            pNewPgDsc->SetFollow( GetPageDescFromPool( RES_POOLPAGE_RIGHT ));
        }
        break;
    case RES_POOLPAGE_RIGHT:        // "Right Page"
        {
            lcl_PutStdPageSizeIntoItemSet( &m_rDoc, aSet );
            aSet.Put( aLR );
            aSet.Put( aUL );
            bSetLeft = false;
            pNewPgDsc->SetUseOn( nsUseOnPage::PD_RIGHT );
            pNewPgDsc->SetFollow( GetPageDescFromPool( RES_POOLPAGE_LEFT ));
        }
        break;

    case RES_POOLPAGE_JAKET:        // "Envelope"
        {
            Size aPSize( SvxPaperInfo::GetPaperSize( PAPER_ENV_C65 ) );
            LandscapeSwap( aPSize );
            aSet.Put( SwFormatFrameSize( ATT_FIX_SIZE, aPSize.Width(), aPSize.Height() ));
            aLR.SetLeft( 0 ); aLR.SetRight( 0 );
            aUL.SetUpper( 0 ); aUL.SetLower( 0 );
            aSet.Put( aLR );
            aSet.Put( aUL );

            pNewPgDsc->SetUseOn( nsUseOnPage::PD_ALL );
            pNewPgDsc->SetLandscape( true );
        }
        break;

    case RES_POOLPAGE_HTML:         // "HTML"
        {
            lcl_PutStdPageSizeIntoItemSet( &m_rDoc, aSet );
            aLR.SetRight( GetMetricVal( CM_1 ));
            aUL.SetUpper( (sal_uInt16)aLR.GetRight() );
            aUL.SetLower( (sal_uInt16)aLR.GetRight() );
            aSet.Put( aLR );
            aSet.Put( aUL );

            pNewPgDsc->SetUseOn( nsUseOnPage::PD_ALL );
        }
        break;

    case RES_POOLPAGE_FOOTNOTE:     // "Footnote"
    case RES_POOLPAGE_ENDNOTE:      // "Endnote"
        {
            lcl_PutStdPageSizeIntoItemSet( &m_rDoc, aSet );
            aSet.Put( aLR );
            aSet.Put( aUL );
            pNewPgDsc->SetUseOn( nsUseOnPage::PD_ALL );
            SwPageFootnoteInfo aInf( pNewPgDsc->GetFootnoteInfo() );
            aInf.SetLineWidth( 0 );
            aInf.SetTopDist( 0 );
            aInf.SetBottomDist( 0 );
            pNewPgDsc->SetFootnoteInfo( aInf );
        }
        break;

    case RES_POOLPAGE_LANDSCAPE:    // "Landscape"
        {
            SwPageDesc* pStdPgDsc = GetPageDescFromPool( RES_POOLPAGE_STANDARD );
            SwFormatFrameSize aFrameSz( pStdPgDsc->GetMaster().GetFrameSize() );
            if ( !pStdPgDsc->GetLandscape() )
            {
                const SwTwips nTmp = aFrameSz.GetHeight();
                aFrameSz.SetHeight( aFrameSz.GetWidth() );
                aFrameSz.SetWidth( nTmp );
            }
            aSet.Put( aFrameSz );
            aSet.Put( aLR );
            aSet.Put( aUL );
            pNewPgDsc->SetUseOn( nsUseOnPage::PD_ALL );
            pNewPgDsc->SetLandscape( true );
        }
        break;

    }

    if( aSet.Count() )
    {
        if( bSetLeft )
        {
            pNewPgDsc->GetLeft().SetFormatAttr( aSet );
            pNewPgDsc->GetFirstLeft().SetFormatAttr( aSet );
        }
        pNewPgDsc->GetMaster().SetFormatAttr( aSet );
        pNewPgDsc->GetFirstMaster().SetFormatAttr( aSet );
    }
    return pNewPgDsc;
}

SwNumRule* DocumentStylePoolManager::GetNumRuleFromPool( sal_uInt16 nId )
{
    OSL_ENSURE( RES_POOLNUMRULE_BEGIN <= nId && nId < RES_POOLNUMRULE_END,
            "Wrong AutoFormat Id" );

    SwNumRule* pNewRule;

    for (size_t n = 0; n < m_rDoc.GetNumRuleTable().size(); ++n )
    {
        if (nId == ( pNewRule = m_rDoc.GetNumRuleTable()[ n ] )->GetPoolFormatId())
        {
            return pNewRule;
        }
    }

    // error: unknown Pool style
    if( RES_POOLNUMRULE_BEGIN > nId ||  nId >= RES_POOLNUMRULE_END )
    {
        OSL_ENSURE( false, "invalid Id" );
        nId = RES_POOLNUMRULE_BEGIN;
    }

    ResId aResId( sal_uInt32(RC_POOLNUMRULE_BEGIN + nId - RES_POOLNUMRULE_BEGIN), *pSwResMgr );
    OUString aNm( aResId );

    SwCharFormat *pNumCFormat = nullptr, *pBullCFormat = nullptr;

    const SvxNumberFormat::SvxNumPositionAndSpaceMode eNumberFormatPositionAndSpaceMode
                                  = numfunc::GetDefaultPositionAndSpaceMode(); //#i89178#
    {
        bool bIsModified = m_rDoc.getIDocumentState().IsModified();

        sal_uInt16 n = m_rDoc.MakeNumRule( aNm, nullptr, false, eNumberFormatPositionAndSpaceMode );

        pNewRule = m_rDoc.GetNumRuleTable()[ n ];
        pNewRule->SetPoolFormatId( nId );
        pNewRule->SetAutoRule( false );

        if( RES_POOLNUMRULE_NUM1 <= nId && nId <= RES_POOLNUMRULE_NUM5 )
            pNumCFormat = GetCharFormatFromPool( RES_POOLCHR_NUM_LEVEL );

        if( ( RES_POOLNUMRULE_BUL1 <= nId && nId <= RES_POOLNUMRULE_BUL5 ) ||
            RES_POOLNUMRULE_NUM5 == nId )
            pBullCFormat = GetCharFormatFromPool( RES_POOLCHR_NUM_LEVEL );

        if( !bIsModified )
            m_rDoc.getIDocumentState().ResetModified();
    }

    switch( nId )
    {
    case RES_POOLNUMRULE_NUM1:
        {
            SwNumFormat aFormat;
            aFormat.SetPositionAndSpaceMode( eNumberFormatPositionAndSpaceMode );
            aFormat.SetNumberingType(SVX_NUM_ARABIC);
            aFormat.SetCharFormat( pNumCFormat );
            aFormat.SetStart( 1 );
            aFormat.SetIncludeUpperLevels( 1 );
            aFormat.SetSuffix( "." );

            static const sal_uInt16 aAbsSpace[ MAXLEVEL ] =
                {
//              cm: 0,5  1,0  1,5  2,0   2,5   3,0   3,5   4,0   4,5   5,0
                    283, 567, 850, 1134, 1417, 1701, 1984, 2268, 2551, 2835
                };
            const sal_uInt16* pArr = aAbsSpace;

            if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_WIDTH_AND_POSITION )
            {
                aFormat.SetFirstLineOffset( - (*pArr) );
            }
            else if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_ALIGNMENT )
            {
                aFormat.SetLabelFollowedBy( SvxNumberFormat::LISTTAB );
                aFormat.SetFirstLineIndent( - (*pArr) );
            }

            for (sal_uInt16 n = 0; n < MAXLEVEL; ++n, ++pArr)
            {
                if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_WIDTH_AND_POSITION )
                {
                    aFormat.SetAbsLSpace( *pArr );
                }
                else if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_ALIGNMENT )
                {
                    aFormat.SetListtabPos( *pArr );
                    aFormat.SetIndentAt( *pArr );
                }

                pNewRule->Set( n, aFormat );
            }
        }
        break;

    case RES_POOLNUMRULE_NUM2:
        {
            static const sal_uInt16 aAbsSpace[ MAXLEVEL ] =
                {
                    283,  283,  567,  709,      // 0.50, 0.50, 1.00, 1.25
                    850, 1021, 1304, 1474,      // 1.50, 1.80, 2.30, 2.60
                   1588, 1758                   // 2.80, 3.10
                };

            const sal_uInt16* pArr = aAbsSpace;
            SwNumFormat aFormat;

            aFormat.SetPositionAndSpaceMode( eNumberFormatPositionAndSpaceMode );
            aFormat.SetNumberingType(SVX_NUM_ARABIC);
            aFormat.SetCharFormat( pNumCFormat );
            aFormat.SetIncludeUpperLevels( 1 );

            if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_ALIGNMENT )
            {
                aFormat.SetLabelFollowedBy( SvxNumberFormat::LISTTAB );
            }

            sal_uInt16 nSpace = 0;
            for (sal_uInt16 n = 0; n < MAXLEVEL; ++n)
            {
                if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_WIDTH_AND_POSITION )
                {
                    aFormat.SetAbsLSpace( nSpace = nSpace + pArr[ n ] );
                    aFormat.SetFirstLineOffset( - pArr[ n ] );
                }
                else if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_ALIGNMENT )
                {
                    aFormat.SetListtabPos( nSpace = nSpace + pArr[ n ] );
                    aFormat.SetIndentAt( nSpace );
                    aFormat.SetFirstLineIndent( - pArr[ n ] );
                }

                aFormat.SetStart( n+1 );
                pNewRule->Set( n, aFormat );
            }
        }
        break;
    case RES_POOLNUMRULE_NUM3:
        {
            SwNumFormat aFormat;

            aFormat.SetPositionAndSpaceMode( eNumberFormatPositionAndSpaceMode );
            aFormat.SetNumberingType(SVX_NUM_ARABIC);
            aFormat.SetCharFormat( pNumCFormat );
            aFormat.SetIncludeUpperLevels( 1 );

            sal_uInt16 nOffs = GetMetricVal( CM_1 ) * 3;

            if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_WIDTH_AND_POSITION )
            {
                aFormat.SetFirstLineOffset( - nOffs );
            }
            else if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_ALIGNMENT )
            {
                aFormat.SetLabelFollowedBy( SvxNumberFormat::LISTTAB );
                aFormat.SetFirstLineIndent( - nOffs );
            }

            for (sal_uInt16 n = 0; n < MAXLEVEL; ++n)
            {
                if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_WIDTH_AND_POSITION )
                {
                    aFormat.SetAbsLSpace( (n+1) * nOffs );
                }
                else if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_ALIGNMENT )
                {
                    long nPos = (n+1) * static_cast<long>(nOffs);
                    aFormat.SetListtabPos(nPos);
                    aFormat.SetIndentAt(nPos);
                }

                aFormat.SetStart( n+1 );
                pNewRule->Set( n, aFormat );
            }
        }
        break;
    case RES_POOLNUMRULE_NUM4:
        {
            SwNumFormat aFormat;

            aFormat.SetPositionAndSpaceMode( eNumberFormatPositionAndSpaceMode );
            aFormat.SetNumberingType(SVX_NUM_ROMAN_UPPER);
            aFormat.SetCharFormat( pNumCFormat );
            aFormat.SetIncludeUpperLevels( 1 );
            aFormat.SetSuffix( "." );

            static const sal_uInt16 aAbsSpace[ MAXLEVEL ] =
                {
//              cm: 0,5  1,0  1,5  2,0   2,5   3,0   3,5   4,0   4,5   5,0
                    283, 567, 850, 1134, 1417, 1701, 1984, 2268, 2551, 2835
                };
            const sal_uInt16* pArr = aAbsSpace;

            if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_WIDTH_AND_POSITION )
            {
                aFormat.SetFirstLineOffset( - (*pArr) );
            }
            else if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_ALIGNMENT )
            {
                aFormat.SetLabelFollowedBy( SvxNumberFormat::SPACE );
                aFormat.SetFirstLineIndent( - (*pArr) );
            }

            for (sal_uInt16 n = 0; n < MAXLEVEL; ++n, ++pArr)
            {
                aFormat.SetStart( n + 1 );

                if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_WIDTH_AND_POSITION )
                {
                    aFormat.SetAbsLSpace( *pArr );
                }
                else if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_ALIGNMENT )
                {
                    aFormat.SetListtabPos( *pArr );
                    aFormat.SetIndentAt( *pArr );
                }

                pNewRule->Set( n, aFormat );
            }
        }
        break;
    case RES_POOLNUMRULE_NUM5:
        {
            // [ First, LSpace ]
            static const sal_uInt16 aAbsSpace0to2[] =
                {
                    227,  227,      // 0.40, 0.40,
                    369,  624,      // 0.65, 1.10,
                    255,  879       // 0.45, 1.55
                };

            const sal_uInt16* pArr0to2 = aAbsSpace0to2;
            SwNumFormat aFormat;

            aFormat.SetPositionAndSpaceMode( eNumberFormatPositionAndSpaceMode );
            aFormat.SetNumberingType(SVX_NUM_ARABIC);
            aFormat.SetStart( 1 );
            aFormat.SetIncludeUpperLevels( 1 );
            aFormat.SetSuffix( "." );

            if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_ALIGNMENT )
            {
                aFormat.SetLabelFollowedBy( SvxNumberFormat::LISTTAB );
            }

            if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_WIDTH_AND_POSITION )
            {
                aFormat.SetFirstLineOffset( -pArr0to2[0] );    // == 0.40 cm
                aFormat.SetAbsLSpace( pArr0to2[1] );           // == 0.40 cm
            }
            else if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_ALIGNMENT )
            {
                aFormat.SetFirstLineIndent( -pArr0to2[0] );
                aFormat.SetListtabPos( pArr0to2[1] );
                aFormat.SetIndentAt( pArr0to2[1] );
            }

            aFormat.SetCharFormat( pNumCFormat );
            pNewRule->Set( 0, aFormat );

            aFormat.SetIncludeUpperLevels( 2 );
            aFormat.SetStart( 2 );

            if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_WIDTH_AND_POSITION )
            {
                aFormat.SetFirstLineOffset( -pArr0to2[2] );    // == 0.65 cm
                aFormat.SetAbsLSpace( pArr0to2[3] );           // == 1.10 cm
            }
            else if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_ALIGNMENT )
            {
                aFormat.SetFirstLineIndent( -pArr0to2[2] );
                aFormat.SetListtabPos( pArr0to2[3] );
                aFormat.SetIndentAt( pArr0to2[3] );
            }

            pNewRule->Set( 1, aFormat );

            aFormat.SetNumberingType(SVX_NUM_CHARS_LOWER_LETTER);
            aFormat.SetSuffix(OUString(static_cast<sal_Unicode>(')')));
            aFormat.SetIncludeUpperLevels( 1 );
            aFormat.SetStart( 3 );

            if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_WIDTH_AND_POSITION )
            {
                aFormat.SetFirstLineOffset( - pArr0to2[4] );   // == 0.45cm
                aFormat.SetAbsLSpace( pArr0to2[5] );           // == 1.55 cm
            }
            else if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_ALIGNMENT )
            {
                aFormat.SetFirstLineIndent( -pArr0to2[4] );
                aFormat.SetListtabPos( pArr0to2[5] );
                aFormat.SetIndentAt( pArr0to2[5] );
            }

            pNewRule->Set( 2, aFormat );

            aFormat.SetNumberingType(SVX_NUM_CHAR_SPECIAL);
            aFormat.SetCharFormat( pBullCFormat );
            aFormat.SetBulletFont(  &numfunc::GetDefBulletFont() );
            aFormat.SetBulletChar( cBulletChar );
            sal_Int16 nOffs = GetMetricVal( CM_01 ) * 4,
                   nOffs2 = GetMetricVal( CM_1 ) * 2;

            if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_WIDTH_AND_POSITION )
            {
                aFormat.SetFirstLineOffset( - nOffs );
            }
            else if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_ALIGNMENT )
            {
                aFormat.SetFirstLineIndent( - nOffs );
            }

            aFormat.SetSuffix( OUString() );
            for (sal_uInt16 n = 3; n < MAXLEVEL; ++n)
            {
                aFormat.SetStart( n+1 );

                if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_WIDTH_AND_POSITION )
                {
                    aFormat.SetAbsLSpace( nOffs2 + ((n-3) * nOffs) );
                }
                else if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_ALIGNMENT )
                {
                    long nPos = nOffs2 + ((n-3) * static_cast<long>(nOffs));
                    aFormat.SetListtabPos(nPos);
                    aFormat.SetIndentAt(nPos);
                }

                pNewRule->Set( n, aFormat );
            }
        }
        break;

    case RES_POOLNUMRULE_BUL1:
        {
            SwNumFormat aFormat;

            aFormat.SetPositionAndSpaceMode( eNumberFormatPositionAndSpaceMode );
            aFormat.SetNumberingType(SVX_NUM_CHAR_SPECIAL);
            aFormat.SetCharFormat( pBullCFormat );
            aFormat.SetStart( 1 );
            aFormat.SetIncludeUpperLevels( 1 );
            aFormat.SetBulletFont( &numfunc::GetDefBulletFont() );
            aFormat.SetBulletChar( cBulletChar );

            static const sal_uInt16 aAbsSpace[ MAXLEVEL ] =
                {
//              cm: 0,4  0,8  1,2  1,6  2,0   2,4   2,8   3,2   3,6   4,0
                    227, 454, 680, 907, 1134, 1361, 1587, 1814, 2041, 2268
                };
            const sal_uInt16* pArr = aAbsSpace;

            if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_WIDTH_AND_POSITION )
            {
                aFormat.SetFirstLineOffset( - (*pArr) );
            }
            else if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_ALIGNMENT )
            {
                aFormat.SetLabelFollowedBy( SvxNumberFormat::LISTTAB );
                aFormat.SetFirstLineIndent( - (*pArr) );
            }

            for (sal_uInt16 n = 0; n < MAXLEVEL; ++n, ++pArr)
            {
                if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_WIDTH_AND_POSITION )
                {
                    aFormat.SetAbsLSpace( *pArr );
                }
                else if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_ALIGNMENT )
                {
                    aFormat.SetListtabPos( *pArr );
                    aFormat.SetIndentAt( *pArr );
                }

                pNewRule->Set( n, aFormat );
            }
        }
        break;
    case RES_POOLNUMRULE_BUL2:
        {
            SwNumFormat aFormat;

            aFormat.SetPositionAndSpaceMode( eNumberFormatPositionAndSpaceMode );
            aFormat.SetNumberingType(SVX_NUM_CHAR_SPECIAL);
            aFormat.SetCharFormat( pBullCFormat );
            aFormat.SetStart( 1 );
            aFormat.SetIncludeUpperLevels( 1 );
            aFormat.SetBulletFont(  &numfunc::GetDefBulletFont() );
            aFormat.SetBulletChar( 0x2013 );

            static const sal_uInt16 aAbsSpace[ MAXLEVEL ] =
                {
//              cm: 0,3  0,6  0,9  1,2  1,5  1,8   2,1   2,4   2,7   3,0
                    170, 340, 510, 680, 850, 1020, 1191, 1361, 1531, 1701
                };
            const sal_uInt16* pArr = aAbsSpace;

            if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_WIDTH_AND_POSITION )
            {
                aFormat.SetFirstLineOffset( - (*pArr) );
            }
            else if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_ALIGNMENT )
            {
                aFormat.SetLabelFollowedBy( SvxNumberFormat::LISTTAB );
                aFormat.SetFirstLineIndent( - (*pArr) );
            }

            for (sal_uInt16 n = 0; n < MAXLEVEL; ++n, ++pArr)
            {
                if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_WIDTH_AND_POSITION )
                {
                    aFormat.SetAbsLSpace( *pArr );
                }
                else if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_ALIGNMENT )
                {
                    aFormat.SetListtabPos( *pArr );
                    aFormat.SetIndentAt( *pArr );
                }

                pNewRule->Set( n, aFormat );
            }
        }
        break;
    case RES_POOLNUMRULE_BUL3:
        {
            SwNumFormat aFormat;

            aFormat.SetPositionAndSpaceMode( eNumberFormatPositionAndSpaceMode );

            aFormat.SetNumberingType(SVX_NUM_CHAR_SPECIAL);
            aFormat.SetCharFormat( pBullCFormat );
            aFormat.SetStart( 1 );
            aFormat.SetIncludeUpperLevels( 1 );
            aFormat.SetBulletFont(  &numfunc::GetDefBulletFont() );

            sal_uInt16 nOffs = GetMetricVal( CM_01 ) * 4;

            if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_WIDTH_AND_POSITION )
            {
                aFormat.SetFirstLineOffset( - nOffs );
            }
            else if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_ALIGNMENT )
            {
                aFormat.SetLabelFollowedBy( SvxNumberFormat::LISTTAB );
                aFormat.SetFirstLineIndent( - nOffs );
            }

            for (sal_uInt16 n = 0; n < MAXLEVEL; ++n)
            {
                aFormat.SetBulletChar( ( n & 1 ? 0x25a1 : 0x2611 ) );

                if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_WIDTH_AND_POSITION )
                {
                    aFormat.SetAbsLSpace( ((n & 1) +1) * nOffs );
                }
                else if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_ALIGNMENT )
                {
                    long nPos = ((n & 1) +1) * static_cast<long>(nOffs);
                    aFormat.SetListtabPos(nPos);
                    aFormat.SetIndentAt(nPos);
                }

                pNewRule->Set( n, aFormat );
            }
        }
        break;
    case RES_POOLNUMRULE_BUL4:
        {
            SwNumFormat aFormat;

            aFormat.SetPositionAndSpaceMode( eNumberFormatPositionAndSpaceMode );
            aFormat.SetNumberingType(SVX_NUM_CHAR_SPECIAL);
            aFormat.SetCharFormat( pBullCFormat );
            aFormat.SetStart( 1 );
            aFormat.SetIncludeUpperLevels( 1 );
            aFormat.SetBulletFont(  &numfunc::GetDefBulletFont() );

            static const sal_uInt16 aAbsSpace[ MAXLEVEL ] =
                {
//              cm: 0,4  0,8  1,2  1,6  2,0   2,4   2,8   3,2   3,6   4,0
                    227, 454, 680, 907, 1134, 1361, 1587, 1814, 2041, 2268
                };

            const sal_uInt16* pArr = aAbsSpace;

            if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_WIDTH_AND_POSITION )
            {
                aFormat.SetFirstLineOffset( - (*pArr) );
            }
            else if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_ALIGNMENT )
            {
                aFormat.SetLabelFollowedBy( SvxNumberFormat::SPACE );
                aFormat.SetFirstLineIndent( - (*pArr) );
            }

            for (sal_uInt16 n = 0; n < MAXLEVEL; ++n, ++pArr)
            {
                switch( n )
                {
                case 0:     aFormat.SetBulletChar( 0x27a2 );   break;
                case 1:     aFormat.SetBulletChar( 0xE006 );   break;
                default:    aFormat.SetBulletChar( 0xE004 );   break;
                }

                if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_WIDTH_AND_POSITION )
                {
                    aFormat.SetAbsLSpace( *pArr );
                }
                else if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_ALIGNMENT )
                {
                    aFormat.SetListtabPos( *pArr );
                    aFormat.SetIndentAt( *pArr );
                }

                pNewRule->Set( n, aFormat );
            }
        }
        break;
    case RES_POOLNUMRULE_BUL5:
        {
            SwNumFormat aFormat;

            aFormat.SetPositionAndSpaceMode( eNumberFormatPositionAndSpaceMode );
            aFormat.SetNumberingType(SVX_NUM_CHAR_SPECIAL);
            aFormat.SetCharFormat( pBullCFormat );
            aFormat.SetStart( 1 );
            aFormat.SetIncludeUpperLevels( 1 );
            aFormat.SetBulletChar( 0x2717 );
            aFormat.SetBulletFont( &numfunc::GetDefBulletFont() );

            static const sal_uInt16 aAbsSpace[ MAXLEVEL ] =
                {
//              cm: 0,4  0,8  1,2  1,6  2,0   2,4   2,8   3,2   3,6   4,0
                    227, 454, 680, 907, 1134, 1361, 1587, 1814, 2041, 2268
                };
            const sal_uInt16* pArr = aAbsSpace;

            if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_WIDTH_AND_POSITION )
            {
                aFormat.SetFirstLineOffset( - (*pArr) );
            }
            else if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_ALIGNMENT )
            {
                aFormat.SetLabelFollowedBy( SvxNumberFormat::LISTTAB );
                aFormat.SetFirstLineIndent( - (*pArr) );
            }

            for (sal_uInt16 n = 0; n < MAXLEVEL; ++n, ++pArr)
            {
                if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_WIDTH_AND_POSITION )
                {
                    aFormat.SetAbsLSpace( *pArr );
                }
                else if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_ALIGNMENT )
                {
                    aFormat.SetListtabPos( *pArr );
                    aFormat.SetIndentAt( *pArr );
                }

                pNewRule->Set( n, aFormat );
            }
        }
        break;
    }

    return pNewRule;
}

/// Check if this AutoCollection is already/still in use in this Document
bool DocumentStylePoolManager::IsPoolTextCollUsed( sal_uInt16 nId ) const
{
    OSL_ENSURE(
        (RES_POOLCOLL_TEXT_BEGIN <= nId && nId < RES_POOLCOLL_TEXT_END) ||
        (RES_POOLCOLL_LISTS_BEGIN <= nId && nId < RES_POOLCOLL_LISTS_END) ||
        (RES_POOLCOLL_EXTRA_BEGIN <= nId && nId < RES_POOLCOLL_EXTRA_END) ||
        (RES_POOLCOLL_REGISTER_BEGIN <= nId && nId < RES_POOLCOLL_REGISTER_END) ||
        (RES_POOLCOLL_DOC_BEGIN <= nId && nId < RES_POOLCOLL_DOC_END) ||
        (RES_POOLCOLL_HTML_BEGIN <= nId && nId < RES_POOLCOLL_HTML_END),
            "Wrong AutoFormat Id" );

    SwTextFormatColl* pNewColl = nullptr;
    bool bFnd = false;
    for( sal_uInt16 n = 0; !bFnd && n < m_rDoc.GetTextFormatColls()->size(); ++n )
    {
        pNewColl = (*m_rDoc.GetTextFormatColls())[ n ];
        if( nId == pNewColl->GetPoolFormatId() )
            bFnd = true;
    }

    if( !bFnd || !pNewColl->HasWriterListeners() )
        return false;

    SwAutoFormatGetDocNode aGetHt( &m_rDoc.GetNodes() );
    return !pNewColl->GetInfo( aGetHt );
}

/// Check if this AutoCollection is already/still in use
bool DocumentStylePoolManager::IsPoolFormatUsed( sal_uInt16 nId ) const
{
    const SwFormat *pNewFormat = nullptr;
    const SwFormatsBase* pArray[ 2 ];
    sal_uInt16 nArrCnt = 1;
    bool bFnd = true;

    if (RES_POOLCHR_BEGIN <= nId && nId < RES_POOLCHR_END)
    {
        pArray[0] = m_rDoc.GetCharFormats();
    }
    else if (RES_POOLFRM_BEGIN <= nId && nId < RES_POOLFRM_END)
    {
        pArray[0] = m_rDoc.GetFrameFormats();
        pArray[1] = m_rDoc.GetSpzFrameFormats();
        nArrCnt = 2;
    }
    else
    {
        SAL_WARN("sw.core", "Invalid Pool Id: " << nId << " should be within "
            "[" << int(RES_POOLCHR_BEGIN) << "," << int(RES_POOLCHR_END) << ") or "
            "[" << int(RES_POOLFRM_BEGIN) << "," << int(RES_POOLFRM_END) << ")");
        bFnd = false;
    }

    if( bFnd )
    {
        bFnd = false;
        while( nArrCnt-- && !bFnd )
            for( size_t n = 0; !bFnd && n < (*pArray[nArrCnt]).GetFormatCount(); ++n )
                if( nId == ( pNewFormat = (*pArray[ nArrCnt ] ).GetFormat( n ) )->
                        GetPoolFormatId() )
                    bFnd = true;
    }

    // Not found or no dependencies?
    if( bFnd && pNewFormat->HasWriterListeners() )
    {
        // Check if we have dependent ContentNodes in the Nodes array
        // (also indirect ones for derived Formats)
        SwAutoFormatGetDocNode aGetHt( &m_rDoc.GetNodes() );
        bFnd = !pNewFormat->GetInfo( aGetHt );
    }
    else
        bFnd = false;

    return bFnd;
}

/// Check if this AutoCollection is already/still in use in this Document
bool DocumentStylePoolManager::IsPoolPageDescUsed( sal_uInt16 nId ) const
{
    OSL_ENSURE( RES_POOLPAGE_BEGIN <= nId && nId < RES_POOLPAGE_END,
            "Wrong AutoFormat Id" );
    SwPageDesc *pNewPgDsc = nullptr;
    bool bFnd = false;
    for( size_t n = 0; !bFnd && n < m_rDoc.GetPageDescCnt(); ++n )
    {
        pNewPgDsc = &m_rDoc.GetPageDesc(n);
        if( nId == pNewPgDsc->GetPoolFormatId() )
            bFnd = true;
    }

    // Not found or no dependencies?
    if( !bFnd || !pNewPgDsc->HasWriterListeners() )     // ??????
        return false;

    // Check if we have dependent ContentNodes in the Nodes array
    // (also indirect ones for derived Formats)
    SwAutoFormatGetDocNode aGetHt( &m_rDoc.GetNodes() );
    return !pNewPgDsc->GetInfo( aGetHt );
}

DocumentStylePoolManager::~DocumentStylePoolManager()
{
}

}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
