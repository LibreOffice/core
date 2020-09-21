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

#include <com/sun/star/text/VertOrientation.hpp>
#include <hintids.hxx>
#include <svtools/htmltokn.h>
#include <svtools/htmlkywd.hxx>
#include <svl/urihelper.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/lrspitem.hxx>
#include <vcl/svapp.hxx>
#include <sal/log.hxx>
#include <osl/diagnose.h>
#include <numrule.hxx>
#include <doc.hxx>
#include <docary.hxx>
#include <poolfmt.hxx>
#include <ndtxt.hxx>
#include <paratr.hxx>

#include "htmlnum.hxx"
#include "swcss1.hxx"
#include "swhtml.hxx"

using namespace css;

// <UL TYPE=...>
HTMLOptionEnum<sal_UCS4> const aHTMLULTypeTable[] =
{
    { OOO_STRING_SVTOOLS_HTML_ULTYPE_disc,    HTML_BULLETCHAR_DISC   },
    { OOO_STRING_SVTOOLS_HTML_ULTYPE_circle,  HTML_BULLETCHAR_CIRCLE },
    { OOO_STRING_SVTOOLS_HTML_ULTYPE_square,  HTML_BULLETCHAR_SQUARE },
    { nullptr,                                0                      }
};


void SwHTMLParser::NewNumberBulletList( HtmlTokenId nToken )
{
    SwHTMLNumRuleInfo& rInfo = GetNumInfo();

    // Create a new paragraph
    bool bSpace = (rInfo.GetDepth() + m_nDefListDeep) == 0;
    if( m_pPam->GetPoint()->nContent.GetIndex() )
        AppendTextNode( bSpace ? AM_SPACE : AM_NOSPACE, false );
    else if( bSpace )
        AddParSpace();

    // Increment the numbering depth
    rInfo.IncDepth();
    sal_uInt8 nLevel = static_cast<sal_uInt8>( (rInfo.GetDepth() <= MAXLEVEL ? rInfo.GetDepth()
                                                        : MAXLEVEL) - 1 );

    // Create rules if needed
    if( !rInfo.GetNumRule() )
    {
        sal_uInt16 nPos = m_xDoc->MakeNumRule( m_xDoc->GetUniqueNumRuleName() );
        rInfo.SetNumRule( m_xDoc->GetNumRuleTable()[nPos] );
    }

    // Change the format for this level if that hasn't happened yet for this level
    bool bNewNumFormat = rInfo.GetNumRule()->GetNumFormat( nLevel ) == nullptr;
    bool bChangeNumFormat = false;

    // Create the default numbering format
    SwNumFormat aNumFormat( rInfo.GetNumRule()->Get(nLevel) );
    rInfo.SetNodeStartValue( nLevel );
    if( bNewNumFormat )
    {
        sal_uInt16 nChrFormatPoolId = 0;
        if( HtmlTokenId::ORDERLIST_ON == nToken )
        {
            aNumFormat.SetNumberingType(SVX_NUM_ARABIC);
            nChrFormatPoolId = RES_POOLCHR_NUM_LEVEL;
        }
        else
        {
            // We'll set a default style because the UI does the same. This meant a 9pt font, which
            // was not the case in Netscape. That didn't bother anyone so far
            // #i63395# - Only apply user defined default bullet font
            if ( numfunc::IsDefBulletFontUserDefined() )
            {
                aNumFormat.SetBulletFont( &numfunc::GetDefBulletFont() );
            }
            aNumFormat.SetNumberingType(SVX_NUM_CHAR_SPECIAL);
            aNumFormat.SetBulletChar( cBulletChar );
            nChrFormatPoolId = RES_POOLCHR_BULLET_LEVEL;
        }

        sal_Int32 nAbsLSpace = HTML_NUMBER_BULLET_MARGINLEFT;

        sal_Int32 nFirstLineIndent  = HTML_NUMBER_BULLET_INDENT;
        if( nLevel > 0 )
        {
            const SwNumFormat& rPrevNumFormat = rInfo.GetNumRule()->Get( nLevel-1 );
            nAbsLSpace = nAbsLSpace + rPrevNumFormat.GetAbsLSpace();
            nFirstLineIndent = rPrevNumFormat.GetFirstLineOffset();
        }
        aNumFormat.SetAbsLSpace( nAbsLSpace );
        aNumFormat.SetFirstLineOffset( nFirstLineIndent );
        aNumFormat.SetCharFormat( m_pCSS1Parser->GetCharFormatFromPool(nChrFormatPoolId) );

        bChangeNumFormat = true;
    }
    else if( 1 != aNumFormat.GetStart() )
    {
        // If the layer has already been used, the start value may need to be set hard to the paragraph.
        rInfo.SetNodeStartValue( nLevel, 1 );
    }

    // and set that in the options
    OUString aId, aStyle, aClass, aLang, aDir;
    OUString aBulletSrc;
    sal_Int16 eVertOri = text::VertOrientation::NONE;
    sal_uInt16 nWidth=USHRT_MAX, nHeight=USHRT_MAX;
    const HTMLOptions& rHTMLOptions = GetOptions();
    for (size_t i = rHTMLOptions.size(); i; )
    {
        const HTMLOption& rOption = rHTMLOptions[--i];
        switch( rOption.GetToken() )
        {
        case HtmlOptionId::ID:
            aId = rOption.GetString();
            break;
        case HtmlOptionId::TYPE:
            if( bNewNumFormat && !rOption.GetString().isEmpty() )
            {
                switch( nToken )
                {
                case HtmlTokenId::ORDERLIST_ON:
                    bChangeNumFormat = true;
                    switch( rOption.GetString()[0] )
                    {
                    case 'A':   aNumFormat.SetNumberingType(SVX_NUM_CHARS_UPPER_LETTER); break;
                    case 'a':   aNumFormat.SetNumberingType(SVX_NUM_CHARS_LOWER_LETTER); break;
                    case 'I':   aNumFormat.SetNumberingType(SVX_NUM_ROMAN_UPPER);        break;
                    case 'i':   aNumFormat.SetNumberingType(SVX_NUM_ROMAN_LOWER);        break;
                    default:    bChangeNumFormat = false;
                    }
                    break;

                case HtmlTokenId::UNORDERLIST_ON:
                    aNumFormat.SetBulletChar( rOption.GetEnum(
                                    aHTMLULTypeTable,aNumFormat.GetBulletChar() ) );
                    bChangeNumFormat = true;
                    break;
                default: break;
                }
            }
            break;
        case HtmlOptionId::START:
            {
                sal_uInt16 nStart = static_cast<sal_uInt16>(rOption.GetNumber());
                if( bNewNumFormat )
                {
                    aNumFormat.SetStart( nStart );
                    bChangeNumFormat = true;
                }
                else
                {
                    rInfo.SetNodeStartValue( nLevel, nStart );
                }
            }
            break;
        case HtmlOptionId::STYLE:
            aStyle = rOption.GetString();
            break;
        case HtmlOptionId::CLASS:
            aClass = rOption.GetString();
            break;
        case HtmlOptionId::LANG:
            aLang = rOption.GetString();
            break;
        case HtmlOptionId::DIR:
            aDir = rOption.GetString();
            break;
        case HtmlOptionId::SRC:
            if( bNewNumFormat )
            {
                aBulletSrc = rOption.GetString();
                if( !InternalImgToPrivateURL(aBulletSrc) )
                    aBulletSrc = URIHelper::SmartRel2Abs( INetURLObject( m_sBaseURL ), aBulletSrc, Link<OUString *, bool>(), false );
            }
            break;
        case HtmlOptionId::WIDTH:
            nWidth = static_cast<sal_uInt16>(rOption.GetNumber());
            break;
        case HtmlOptionId::HEIGHT:
            nHeight = static_cast<sal_uInt16>(rOption.GetNumber());
            break;
        case HtmlOptionId::ALIGN:
            eVertOri = rOption.GetEnum( aHTMLImgVAlignTable, eVertOri );
            break;
        default: break;
        }
    }

    if( !aBulletSrc.isEmpty() )
    {
        // A bullet list with graphics
        aNumFormat.SetNumberingType(SVX_NUM_BITMAP);

        // Create the graphic as a brush
        SvxBrushItem aBrushItem( RES_BACKGROUND );
        aBrushItem.SetGraphicLink( aBulletSrc );
        aBrushItem.SetGraphicPos( GPOS_AREA );

        // Only set size if given a width and a height
        Size aTwipSz( nWidth, nHeight), *pTwipSz=nullptr;
        if( nWidth!=USHRT_MAX && nHeight!=USHRT_MAX )
        {
            aTwipSz =
                Application::GetDefaultDevice()->PixelToLogic( aTwipSz,
                                                    MapMode(MapUnit::MapTwip) );
            pTwipSz = &aTwipSz;
        }

        // Only set orientation if given one
        aNumFormat.SetGraphicBrush( &aBrushItem, pTwipSz,
                            text::VertOrientation::NONE!=eVertOri ? &eVertOri : nullptr);

        // Remember the graphic to not put it into the paragraph
        m_aBulletGrfs[nLevel] = aBulletSrc;
        bChangeNumFormat = true;
    }
    else
        m_aBulletGrfs[nLevel].clear();

    // don't number the current paragraph (for now)
    {
        sal_uInt8 nLvl = nLevel;
        SetNodeNum( nLvl );
    }

    // create a new context
    std::unique_ptr<HTMLAttrContext> xCntxt(new HTMLAttrContext(nToken));

    // Parse styles
    if( HasStyleOptions( aStyle, aId, aClass, &aLang, &aDir ) )
    {
        SfxItemSet aItemSet( m_xDoc->GetAttrPool(), m_pCSS1Parser->GetWhichMap() );
        SvxCSS1PropertyInfo aPropInfo;

        if( ParseStyleOptions( aStyle, aId, aClass, aItemSet, aPropInfo, &aLang, &aDir ) )
        {
            if( bNewNumFormat )
            {
                if( aPropInfo.m_bLeftMargin )
                {
                    // Default indent has already been added
                    long nAbsLSpace =
                        aNumFormat.GetAbsLSpace() - HTML_NUMBER_BULLET_MARGINLEFT;
                    if( aPropInfo.m_nLeftMargin < 0 &&
                        nAbsLSpace < -aPropInfo.m_nLeftMargin )
                        nAbsLSpace = 0U;
                    else if( aPropInfo.m_nLeftMargin > SHRT_MAX ||
                             nAbsLSpace + aPropInfo.m_nLeftMargin > SHRT_MAX )
                        nAbsLSpace = SHRT_MAX;
                    else
                        nAbsLSpace = nAbsLSpace + aPropInfo.m_nLeftMargin;

                    aNumFormat.SetAbsLSpace( nAbsLSpace );
                    bChangeNumFormat = true;
                }
                if( aPropInfo.m_bTextIndent )
                {
                    short nTextIndent =
                        aItemSet.Get( RES_LR_SPACE ).GetTextFirstLineOffset();
                    aNumFormat.SetFirstLineOffset( nTextIndent );
                    bChangeNumFormat = true;
                }
                if( aPropInfo.m_bNumbering )
                {
                    aNumFormat.SetNumberingType(aPropInfo.m_nNumberingType);
                    bChangeNumFormat = true;
                }
                if( aPropInfo.m_bBullet )
                {
                    aNumFormat.SetBulletChar( aPropInfo.m_cBulletChar );
                    bChangeNumFormat = true;
                }
            }
            aPropInfo.m_bLeftMargin = aPropInfo.m_bTextIndent = false;
            if( !aPropInfo.m_bRightMargin )
                aItemSet.ClearItem( RES_LR_SPACE );

            // #i89812# - Perform change to list style before calling <DoPositioning(..)>,
            // because <DoPositioning(..)> may open a new context and thus may
            // clear the <SwHTMLNumRuleInfo> instance hold by local variable <rInfo>.
            if( bChangeNumFormat )
            {
                rInfo.GetNumRule()->Set( nLevel, aNumFormat );
                m_xDoc->ChgNumRuleFormats( *rInfo.GetNumRule() );
                bChangeNumFormat = false;
            }

            DoPositioning(aItemSet, aPropInfo, xCntxt.get());

            InsertAttrs(aItemSet, aPropInfo, xCntxt.get());
        }
    }

    if( bChangeNumFormat )
    {
        rInfo.GetNumRule()->Set( nLevel, aNumFormat );
        m_xDoc->ChgNumRuleFormats( *rInfo.GetNumRule() );
    }

    PushContext(xCntxt);

    // set attributes to the current template
    SetTextCollAttrs(m_aContexts.back().get());
}

void SwHTMLParser::EndNumberBulletList( HtmlTokenId nToken )
{
    SwHTMLNumRuleInfo& rInfo = GetNumInfo();

    // A new paragraph needs to be created, when
    // - the current one isn't empty (it contains text or paragraph-bound objects)
    // - the current one is numbered
    bool bAppend = m_pPam->GetPoint()->nContent.GetIndex() > 0;
    if( !bAppend )
    {
        SwTextNode* pTextNode = m_pPam->GetNode().GetTextNode();

        bAppend = (pTextNode && ! pTextNode->IsOutline() && pTextNode->IsCountedInList()) ||

            HasCurrentParaFlys();
    }

    bool bSpace = (rInfo.GetDepth() + m_nDefListDeep) == 1;
    if( bAppend )
        AppendTextNode( bSpace ? AM_SPACE : AM_NOSPACE, false );
    else if( bSpace )
        AddParSpace();

    // get current context from stack
    std::unique_ptr<HTMLAttrContext> xCntxt(nToken != HtmlTokenId::NONE ? PopContext(getOnToken(nToken)) : nullptr);

    // Don't end a list because of a token, if the context wasn't created or mustn't be ended
    if( rInfo.GetDepth()>0 && (nToken == HtmlTokenId::NONE || xCntxt) )
    {
        rInfo.DecDepth();
        if( !rInfo.GetDepth() )     // was that the last level?
        {
            // The formats not yet modified are now modified, to ease editing
            const SwNumFormat *pRefNumFormat = nullptr;
            bool bChanged = false;
            for( sal_uInt16 i=0; i<MAXLEVEL; i++ )
            {
                const SwNumFormat *pNumFormat = rInfo.GetNumRule()->GetNumFormat(i);
                if( pNumFormat )
                {
                    pRefNumFormat = pNumFormat;
                }
                else if( pRefNumFormat )
                {
                    SwNumFormat aNumFormat( rInfo.GetNumRule()->Get(i) );
                    aNumFormat.SetNumberingType(pRefNumFormat->GetNumberingType() != SVX_NUM_BITMAP
                                                ? pRefNumFormat->GetNumberingType() : SVX_NUM_CHAR_SPECIAL);
                    if( SVX_NUM_CHAR_SPECIAL == aNumFormat.GetNumberingType() )
                    {
                        // #i63395# - Only apply user defined default bullet font
                        if ( numfunc::IsDefBulletFontUserDefined() )
                        {
                            aNumFormat.SetBulletFont( &numfunc::GetDefBulletFont() );
                        }
                        aNumFormat.SetBulletChar( cBulletChar );
                    }
                    aNumFormat.SetAbsLSpace( (i+1) * HTML_NUMBER_BULLET_MARGINLEFT );
                    aNumFormat.SetFirstLineOffset( HTML_NUMBER_BULLET_INDENT );
                    aNumFormat.SetCharFormat( pRefNumFormat->GetCharFormat() );
                    rInfo.GetNumRule()->Set( i, aNumFormat );
                    bChanged = true;
                }
            }
            if( bChanged )
                m_xDoc->ChgNumRuleFormats( *rInfo.GetNumRule() );

            // On the last append, the NumRule item and NodeNum object were copied.
            // Now we need to delete them. ResetAttr deletes the NodeNum object as well
            if (SwTextNode *pTextNode = m_pPam->GetNode().GetTextNode())
                pTextNode->ResetAttr(RES_PARATR_NUMRULE);

            rInfo.Clear();
        }
        else
        {
            // the next paragraph not numbered first
            SetNodeNum( rInfo.GetLevel() );
        }
    }

    // end attributes
    bool bSetAttrs = false;
    if (xCntxt)
    {
        EndContext(xCntxt.get());
        xCntxt.reset();
        bSetAttrs = true;
    }

    if( nToken != HtmlTokenId::NONE )
        SetTextCollAttrs();

    if( bSetAttrs )
        SetAttr();  // Set paragraph attributes asap because of Javascript

}

void SwHTMLParser::NewNumberBulletListItem( HtmlTokenId nToken )
{
    sal_uInt8 nLevel = GetNumInfo().GetLevel();
    OUString aId, aStyle, aClass, aLang, aDir;
    sal_uInt16 nStart = HtmlTokenId::LISTHEADER_ON != nToken
                        ? GetNumInfo().GetNodeStartValue( nLevel )
                        : USHRT_MAX;
    if( USHRT_MAX != nStart )
        GetNumInfo().SetNodeStartValue( nLevel );

    const HTMLOptions& rHTMLOptions = GetOptions();
    for (size_t i = rHTMLOptions.size(); i; )
    {
        const HTMLOption& rOption = rHTMLOptions[--i];
        switch( rOption.GetToken() )
        {
            case HtmlOptionId::VALUE:
                nStart = static_cast<sal_uInt16>(rOption.GetNumber());
                break;
            case HtmlOptionId::ID:
                aId = rOption.GetString();
                break;
            case HtmlOptionId::STYLE:
                aStyle = rOption.GetString();
                break;
            case HtmlOptionId::CLASS:
                aClass = rOption.GetString();
                break;
            case HtmlOptionId::LANG:
                aLang = rOption.GetString();
                break;
            case HtmlOptionId::DIR:
                aDir = rOption.GetString();
                break;
            default: break;
        }
    }

    // create a new paragraph
    if( m_pPam->GetPoint()->nContent.GetIndex() )
        AppendTextNode( AM_NOSPACE, false );
    m_bNoParSpace = false;    // no space in <LI>!

    SwTextNode* pTextNode = m_pPam->GetNode().GetTextNode();
    if (!pTextNode)
    {
        SAL_WARN("sw.html", "No Text-Node at PaM-Position");
        return;
    }

    const bool bCountedInList = nToken != HtmlTokenId::LISTHEADER_ON;

    std::unique_ptr<HTMLAttrContext> xCntxt(new HTMLAttrContext(nToken));

    OUString aNumRuleName;
    if( GetNumInfo().GetNumRule() )
    {
        aNumRuleName = GetNumInfo().GetNumRule()->GetName();
    }
    else
    {
        aNumRuleName = m_xDoc->GetUniqueNumRuleName();
        SwNumRule aNumRule( aNumRuleName,
                            SvxNumberFormat::LABEL_WIDTH_AND_POSITION );
        SwNumFormat aNumFormat( aNumRule.Get( 0 ) );
        // #i63395# - Only apply user defined default bullet font
        if ( numfunc::IsDefBulletFontUserDefined() )
        {
            aNumFormat.SetBulletFont( &numfunc::GetDefBulletFont() );
        }
        aNumFormat.SetNumberingType(SVX_NUM_CHAR_SPECIAL);
        aNumFormat.SetBulletChar( cBulletChar );   // the bullet character !!
        aNumFormat.SetCharFormat( m_pCSS1Parser->GetCharFormatFromPool(RES_POOLCHR_BULLET_LEVEL) );
        aNumFormat.SetFirstLineOffset( HTML_NUMBER_BULLET_INDENT );
        aNumRule.Set( 0, aNumFormat );

        m_xDoc->MakeNumRule( aNumRuleName, &aNumRule );

        OSL_ENSURE( m_nOpenParaToken == HtmlTokenId::NONE,
                "Now an open paragraph element is lost" );
        // We'll act like we're in a paragraph. On the next paragraph, at least numbering is gone,
        // that's gonna be taken over by the next AppendTextNode
        m_nOpenParaToken = nToken;
    }

    static_cast<SwContentNode *>(pTextNode)->SetAttr( SwNumRuleItem(aNumRuleName) );
    pTextNode->SetAttrListLevel(nLevel);
    // #i57656# - <IsCounted()> state of text node has to be adjusted accordingly.
    if ( nLevel < MAXLEVEL )
    {
        pTextNode->SetCountedInList( bCountedInList );
    }
    // #i57919#
    // correction of refactoring done by cws swnumtree
    // - <nStart> contains the start value, if the numbering has to be restarted
    //   at this text node. Value <USHRT_MAX> indicates, that numbering isn't
    //   restarted at this text node
    if ( nStart != USHRT_MAX )
    {
        pTextNode->SetListRestart( true );
        pTextNode->SetAttrListRestartValue( nStart );
    }

    if( GetNumInfo().GetNumRule() )
        GetNumInfo().GetNumRule()->SetInvalidRule( true );

    // parse styles
    if( HasStyleOptions( aStyle, aId, aClass, &aLang, &aDir ) )
    {
        SfxItemSet aItemSet( m_xDoc->GetAttrPool(), m_pCSS1Parser->GetWhichMap() );
        SvxCSS1PropertyInfo aPropInfo;

        if( ParseStyleOptions( aStyle, aId, aClass, aItemSet, aPropInfo, &aLang, &aDir ) )
        {
            DoPositioning(aItemSet, aPropInfo, xCntxt.get());
            InsertAttrs(aItemSet, aPropInfo, xCntxt.get());
        }
    }

    PushContext(xCntxt);

    // set the new template
    SetTextCollAttrs(m_aContexts.back().get());

    // Refresh scroll bar
    ShowStatline();
}

void SwHTMLParser::EndNumberBulletListItem( HtmlTokenId nToken, bool bSetColl )
{
    // Create a new paragraph
    if( nToken == HtmlTokenId::NONE && m_pPam->GetPoint()->nContent.GetIndex() )
        AppendTextNode( AM_NOSPACE );

    // Get context to that token and pop it from stack
    std::unique_ptr<HTMLAttrContext> xCntxt;
    auto nPos = m_aContexts.size();
    nToken = getOnToken(nToken);
    while (!xCntxt && nPos>m_nContextStMin)
    {
        HtmlTokenId nCntxtToken = m_aContexts[--nPos]->GetToken();
        switch( nCntxtToken )
        {
        case HtmlTokenId::LI_ON:
        case HtmlTokenId::LISTHEADER_ON:
            if( nToken == HtmlTokenId::NONE || nToken == nCntxtToken  )
            {
                xCntxt = std::move(m_aContexts[nPos]);
                m_aContexts.erase( m_aContexts.begin() + nPos );
            }
            break;
        case HtmlTokenId::ORDERLIST_ON:
        case HtmlTokenId::UNORDERLIST_ON:
        case HtmlTokenId::MENULIST_ON:
        case HtmlTokenId::DIRLIST_ON:
            // Don't care about LI/LH outside the current list
            nPos = m_nContextStMin;
            break;
        default: break;
        }
    }

    // end attributes
    if (xCntxt)
    {
        EndContext(xCntxt.get());
        SetAttr();  // set paragraph attributes asap because of Javascript
        xCntxt.reset();
    }

    // set current template
    if( bSetColl )
        SetTextCollAttrs();
}

void SwHTMLParser::SetNodeNum( sal_uInt8 nLevel )
{
    SwTextNode* pTextNode = m_pPam->GetNode().GetTextNode();
    if (!pTextNode)
    {
        SAL_WARN("sw.html", "No Text-Node at PaM-Position");
        return;
    }

    OSL_ENSURE( GetNumInfo().GetNumRule(), "No numbering rule" );
    const OUString& rName = GetNumInfo().GetNumRule()->GetName();
    static_cast<SwContentNode *>(pTextNode)->SetAttr( SwNumRuleItem(rName) );

    pTextNode->SetAttrListLevel( nLevel );
    pTextNode->SetCountedInList( false );

    // Invalidate NumRule, it may have been set valid because of an EndAction
    GetNumInfo().GetNumRule()->SetInvalidRule( false );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
