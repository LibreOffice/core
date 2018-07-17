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

#include <memory>
#include <hintids.hxx>
#include <comphelper/string.hxx>
#include <vcl/svapp.hxx>
#include <vcl/wrkwin.hxx>
#include <svx/svxids.hrc>
#include <i18nlangtag/languagetag.hxx>
#include <svl/stritem.hxx>
#include <svl/urihelper.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/adjustitem.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/langitem.hxx>
#include <editeng/scripttypeitem.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/event.hxx>
#include <svtools/imap.hxx>
#include <svtools/htmltokn.h>
#include <svtools/htmlkywd.hxx>
#include <unotools/eventcfg.hxx>

#include <fmtornt.hxx>
#include <fmturl.hxx>
#include <fmtsrnd.hxx>
#include <fmtinfmt.hxx>
#include <fmtcntnt.hxx>
#include <fmtanchr.hxx>
#include <fmtfsize.hxx>
#include <frmatr.hxx>
#include <charatr.hxx>
#include <frmfmt.hxx>
#include <charfmt.hxx>
#include <docary.hxx>
#include <docsh.hxx>
#include <pam.hxx>
#include <doc.hxx>
#include <ndtxt.hxx>
#include <shellio.hxx>
#include <poolfmt.hxx>
#include <IMark.hxx>
#include <ndgrf.hxx>
#include "htmlnum.hxx"
#include "swcss1.hxx"
#include "swhtml.hxx"
#include <numrule.hxx>
#include <fmtflcnt.hxx>
#include <IDocumentMarkAccess.hxx>

#include <vcl/graphicfilter.hxx>
#include <tools/urlobj.hxx>

using namespace ::com::sun::star;

HTMLOptionEnum<sal_Int16> aHTMLImgHAlignTable[] =
{
    { OOO_STRING_SVTOOLS_HTML_AL_left,    text::HoriOrientation::LEFT       },
    { OOO_STRING_SVTOOLS_HTML_AL_right,   text::HoriOrientation::RIGHT      },
    { nullptr,                            0               }
};

HTMLOptionEnum<sal_Int16> aHTMLImgVAlignTable[] =
{
    { OOO_STRING_SVTOOLS_HTML_VA_top,         text::VertOrientation::LINE_TOP       },
    { OOO_STRING_SVTOOLS_HTML_VA_texttop,     text::VertOrientation::CHAR_TOP       },
    { OOO_STRING_SVTOOLS_HTML_VA_middle,      text::VertOrientation::CENTER         },
    { OOO_STRING_SVTOOLS_HTML_AL_center,      text::VertOrientation::CENTER         },
    { OOO_STRING_SVTOOLS_HTML_VA_absmiddle,   text::VertOrientation::LINE_CENTER    },
    { OOO_STRING_SVTOOLS_HTML_VA_bottom,      text::VertOrientation::TOP            },
    { OOO_STRING_SVTOOLS_HTML_VA_baseline,    text::VertOrientation::TOP            },
    { OOO_STRING_SVTOOLS_HTML_VA_absbottom,   text::VertOrientation::LINE_BOTTOM    },
    { nullptr,                                0                   }
};

ImageMap *SwHTMLParser::FindImageMap( const OUString& rName ) const
{
    OSL_ENSURE( rName[0] != '#', "FindImageMap: name begins with '#'!" );

    if (m_pImageMaps)
    {
        for (auto &rpIMap : *m_pImageMaps)
        {
            if (rName.equalsIgnoreAsciiCase(rpIMap->GetName()))
            {
                return rpIMap.get();
            }
        }
    }
    return nullptr;
}

void SwHTMLParser::ConnectImageMaps()
{
    SwNodes& rNds = m_xDoc->GetNodes();
    // on the first node of section #1
    sal_uLong nIdx = rNds.GetEndOfAutotext().StartOfSectionIndex() + 1;
    sal_uLong nEndIdx = rNds.GetEndOfAutotext().GetIndex();

    SwGrfNode* pGrfNd;
    while( m_nMissingImgMaps > 0 && nIdx < nEndIdx )
    {
        SwNode *pNd = rNds[nIdx + 1];
        if( nullptr != (pGrfNd = pNd->GetGrfNode()) )
        {
            SwFrameFormat *pFormat = pGrfNd->GetFlyFormat();
            SwFormatURL aURL( pFormat->GetURL() );
            const ImageMap *pIMap = aURL.GetMap();
            if( pIMap && pIMap->GetIMapObjectCount()==0 )
            {
                // The (empty) image map of the node will be either
                // replaced with found image map or deleted.
                ImageMap *pNewIMap =
                    FindImageMap( pIMap->GetName() );
                aURL.SetMap( pNewIMap );
                pFormat->SetFormatAttr( aURL );
                if( !pGrfNd->IsScaleImageMap() )
                {
                    // meanwhile the graphic size is known or the
                    // graphic don't need scaling
                    pGrfNd->ScaleImageMap();
                }
                m_nMissingImgMaps--;  // search a map less
            }
        }
        nIdx = rNds[nIdx]->EndOfSectionIndex() + 1;
    }
}

void SwHTMLParser::SetAnchorAndAdjustment( sal_Int16 eVertOri,
                                           sal_Int16 eHoriOri,
                                           const SvxCSS1PropertyInfo &rCSS1PropInfo,
                                           SfxItemSet& rFrameItemSet )
{
    const SfxItemSet *pCntnrItemSet = nullptr;
    auto i = m_aContexts.size();
    while( !pCntnrItemSet && i > m_nContextStMin )
        pCntnrItemSet = m_aContexts[--i]->GetFrameItemSet();

    if( pCntnrItemSet )
    {
        // If we are in a container then the anchoring of the container is used.
        rFrameItemSet.Put( *pCntnrItemSet );
    }
    else if( SwCSS1Parser::MayBePositioned( rCSS1PropInfo, true ) )
    {
        // If the alignment can be set via CSS1 options we use them.
        SetAnchorAndAdjustment( rCSS1PropInfo, rFrameItemSet );
    }
    else
    {
        // Otherwise the alignment is set correspondingly the normal HTML options.
        SetAnchorAndAdjustment( eVertOri, eHoriOri, rFrameItemSet );
    }
}

void SwHTMLParser::SetAnchorAndAdjustment( sal_Int16 eVertOri,
                                           sal_Int16 eHoriOri,
                                           SfxItemSet& rFrameSet,
                                           bool bDontAppend )
{
    bool bMoveBackward = false;
    SwFormatAnchor aAnchor( RndStdIds::FLY_AS_CHAR );
    sal_Int16 eVertRel = text::RelOrientation::FRAME;

    if( text::HoriOrientation::NONE != eHoriOri )
    {
        // determine paragraph indent
        sal_uInt16 nLeftSpace = 0, nRightSpace = 0;
        short nIndent = 0;
        GetMarginsFromContextWithNumBul( nLeftSpace, nRightSpace, nIndent );

        // determine horizontal alignment and wrapping
        sal_Int16 eHoriRel;
        css::text::WrapTextMode eSurround;
        switch( eHoriOri )
        {
        case text::HoriOrientation::LEFT:
            eHoriRel = nLeftSpace ? text::RelOrientation::PRINT_AREA : text::RelOrientation::FRAME;
            eSurround = css::text::WrapTextMode_RIGHT;
            break;
        case text::HoriOrientation::RIGHT:
            eHoriRel = nRightSpace ? text::RelOrientation::PRINT_AREA : text::RelOrientation::FRAME;
            eSurround = css::text::WrapTextMode_LEFT;
            break;
        case text::HoriOrientation::CENTER:   // for tables
            eHoriRel = text::RelOrientation::FRAME;
            eSurround = css::text::WrapTextMode_NONE;
            break;
        default:
            eHoriRel = text::RelOrientation::FRAME;
            eSurround = css::text::WrapTextMode_PARALLEL;
            break;
        }

        // Create a new paragraph, if the current one has frames
        // anchored at paragraph/at char without wrapping.
        if( !bDontAppend && HasCurrentParaFlys( true ) )
        {
            // When the paragraph only contains graphics then there
            // is no need for bottom margin. Since here also with use of
            // styles no margin should be created, set attributes to
            // override!
            sal_uInt16 nUpper=0, nLower=0;
            GetULSpaceFromContext( nUpper, nLower );
            InsertAttr( SvxULSpaceItem( nUpper, 0, RES_UL_SPACE ), true );

            AppendTextNode( AM_NOSPACE );

            if( nUpper )
            {
                NewAttr(m_xAttrTab, &m_xAttrTab->pULSpace, SvxULSpaceItem(0, nLower, RES_UL_SPACE));
                m_aParaAttrs.push_back( m_xAttrTab->pULSpace );
                EndAttr( m_xAttrTab->pULSpace, false );
            }
        }

        // determine vertical alignment and anchoring
        const sal_Int32 nContent = m_pPam->GetPoint()->nContent.GetIndex();
        if( nContent )
        {
            aAnchor.SetType( RndStdIds::FLY_AT_CHAR );
            bMoveBackward = true;
            eVertOri = text::VertOrientation::CHAR_BOTTOM;
            eVertRel = text::RelOrientation::CHAR;
        }
        else
        {
            aAnchor.SetType( RndStdIds::FLY_AT_PARA );
            eVertOri = text::VertOrientation::TOP;
            eVertRel = text::RelOrientation::PRINT_AREA;
        }

        rFrameSet.Put( SwFormatHoriOrient( 0, eHoriOri, eHoriRel) );

        rFrameSet.Put( SwFormatSurround( eSurround ) );
    }
    rFrameSet.Put( SwFormatVertOrient( 0, eVertOri, eVertRel) );

    if( bMoveBackward )
        m_pPam->Move( fnMoveBackward );

    if (aAnchor.GetAnchorId() == RndStdIds::FLY_AS_CHAR && !m_pPam->GetNode().GetTextNode())
    {
        eState = SvParserState::Error;
        return;
    }

    aAnchor.SetAnchor( m_pPam->GetPoint() );

    if( bMoveBackward )
        m_pPam->Move( fnMoveForward );

    rFrameSet.Put( aAnchor );
}

void SwHTMLParser::RegisterFlyFrame( SwFrameFormat *pFlyFormat )
{
    // automatically anchored frames must be moved forward by one position
    if( RES_DRAWFRMFMT != pFlyFormat->Which() &&
        (RndStdIds::FLY_AT_PARA == pFlyFormat->GetAnchor().GetAnchorId()) &&
        css::text::WrapTextMode_THROUGH == pFlyFormat->GetSurround().GetSurround() )
    {
        m_aMoveFlyFrames.push_back( pFlyFormat );
        m_aMoveFlyCnts.push_back( m_pPam->GetPoint()->nContent.GetIndex() );
    }
}

/*  */

void SwHTMLParser::GetDefaultScriptType( ScriptType& rType,
                                         OUString& rTypeStr ) const
{
    SwDocShell *pDocSh = m_xDoc->GetDocShell();
    SvKeyValueIterator* pHeaderAttrs = pDocSh ? pDocSh->GetHeaderAttributes()
                                              : nullptr;
    rType = GetScriptType( pHeaderAttrs );
    rTypeStr = GetScriptTypeString( pHeaderAttrs );
}

namespace
{
    bool allowAccessLink(SwDoc& rDoc)
    {
        OUString sReferer;
        SfxObjectShell * sh = rDoc.GetPersist();
        if (sh != nullptr && sh->HasName())
        {
            sReferer = sh->GetMedium()->GetName();
        }
        return !SvtSecurityOptions().isUntrustedReferer(sReferer);
    }
}

/*  */

void SwHTMLParser::InsertImage()
{
    // and now analyze
    OUString sAltNm, aId, aClass, aStyle, aMap, sHTMLGrfName;
    OUString sGrfNm;
    OUString aGraphicData;
    sal_Int16 eVertOri = text::VertOrientation::TOP;
    sal_Int16 eHoriOri = text::HoriOrientation::NONE;
    bool bWidthProvided=false, bHeightProvided=false;
    long nWidth=0, nHeight=0;
    long nVSpace=0, nHSpace=0;

    sal_uInt16 nBorder = (m_xAttrTab->pINetFormat ? 1 : 0);
    bool bIsMap = false;
    bool bPrcWidth = false;
    bool bPrcHeight = false;
    OUString sWidthAsString, sHeightAsString;
    SvxMacroItem aMacroItem(RES_FRMMACRO);

    ScriptType eDfltScriptType;
    OUString sDfltScriptType;
    GetDefaultScriptType( eDfltScriptType, sDfltScriptType );

    const HTMLOptions& rHTMLOptions = GetOptions();
    for (size_t i = rHTMLOptions.size(); i; )
    {
        SvMacroItemId nEvent = SvMacroItemId::NONE;
        ScriptType eScriptType2 = eDfltScriptType;
        const HTMLOption& rOption = rHTMLOptions[--i];
        switch( rOption.GetToken() )
        {
            case HtmlOptionId::ID:
                aId = rOption.GetString();
                break;
            case HtmlOptionId::STYLE:
                aStyle = rOption.GetString();
                break;
            case HtmlOptionId::CLASS:
                aClass = rOption.GetString();
                break;
            case HtmlOptionId::SRC:
                sGrfNm = rOption.GetString();
                if( !InternalImgToPrivateURL(sGrfNm) )
                    sGrfNm = INetURLObject::GetAbsURL( m_sBaseURL, sGrfNm );
                break;
            case HtmlOptionId::DATA:
                aGraphicData = rOption.GetString();
                if (!InternalImgToPrivateURL(aGraphicData))
                    aGraphicData = INetURLObject::GetAbsURL(m_sBaseURL, aGraphicData);
                break;
            case HtmlOptionId::ALIGN:
                eVertOri =
                    rOption.GetEnum( aHTMLImgVAlignTable,
                                                    text::VertOrientation::TOP );
                eHoriOri =
                    rOption.GetEnum( aHTMLImgHAlignTable );
                break;
            case HtmlOptionId::WIDTH:
                // for now only store as pixel value!
                nWidth = rOption.GetNumber();
                sWidthAsString = rOption.GetString();
                bPrcWidth = (sWidthAsString.indexOf('%') != -1);
                if( bPrcWidth && nWidth>100 )
                    nWidth = 100;
                // width|height = "auto" means viewing app decides the size
                // i.e. proceed as if no particular size was provided
                bWidthProvided = (sWidthAsString != "auto");
                break;
            case HtmlOptionId::HEIGHT:
                // for now only store as pixel value!
                nHeight = rOption.GetNumber();
                sHeightAsString = rOption.GetString();
                bPrcHeight = (sHeightAsString.indexOf('%') != -1);
                if( bPrcHeight && nHeight>100 )
                    nHeight = 100;
                // the same as above w/ HtmlOptionId::WIDTH
                bHeightProvided = (sHeightAsString != "auto");
                break;
            case HtmlOptionId::VSPACE:
                nVSpace = rOption.GetNumber();
                break;
            case HtmlOptionId::HSPACE:
                nHSpace = rOption.GetNumber();
                break;
            case HtmlOptionId::ALT:
                sAltNm = rOption.GetString();
                break;
            case HtmlOptionId::BORDER:
                nBorder = static_cast<sal_uInt16>(rOption.GetNumber());
                break;
            case HtmlOptionId::ISMAP:
                bIsMap = true;
                break;
            case HtmlOptionId::USEMAP:
                aMap = rOption.GetString();
                break;
            case HtmlOptionId::NAME:
                sHTMLGrfName = rOption.GetString();
                break;

            case HtmlOptionId::SDONLOAD:
                eScriptType2 = STARBASIC;
                SAL_FALLTHROUGH;
            case HtmlOptionId::ONLOAD:
                nEvent = SvMacroItemId::OnImageLoadDone;
                goto IMAGE_SETEVENT;

            case HtmlOptionId::SDONABORT:
                eScriptType2 = STARBASIC;
                SAL_FALLTHROUGH;
            case HtmlOptionId::ONABORT:
                nEvent = SvMacroItemId::OnImageLoadCancel;
                goto IMAGE_SETEVENT;

            case HtmlOptionId::SDONERROR:
                eScriptType2 = STARBASIC;
                SAL_FALLTHROUGH;
            case HtmlOptionId::ONERROR:
                nEvent = SvMacroItemId::OnImageLoadError;
                goto IMAGE_SETEVENT;
IMAGE_SETEVENT:
                {
                    OUString sTmp( rOption.GetString() );
                    if( !sTmp.isEmpty() )
                    {
                        sTmp = convertLineEnd(sTmp, GetSystemLineEnd());
                        OUString sScriptType;
                        if( EXTENDED_STYPE == eScriptType2 )
                            sScriptType = sDfltScriptType;
                        aMacroItem.SetMacro( nEvent,
                            SvxMacro( sTmp, sScriptType, eScriptType2 ));
                    }
                }
                break;
            default: break;
        }
    }

    if (sGrfNm.isEmpty() && !aGraphicData.isEmpty())
        sGrfNm = aGraphicData;

    if( sGrfNm.isEmpty() )
        return;

    // When we are in a ordered list and the paragraph is still empty and not
    // numbered, it may be a graphic for a bullet list.
    if( !m_pPam->GetPoint()->nContent.GetIndex() &&
        GetNumInfo().GetDepth() > 0 && GetNumInfo().GetDepth() <= MAXLEVEL &&
        !m_aBulletGrfs[GetNumInfo().GetDepth()-1].isEmpty() &&
        m_aBulletGrfs[GetNumInfo().GetDepth()-1]==sGrfNm )
    {
        SwTextNode* pTextNode = m_pPam->GetNode().GetTextNode();

        if( pTextNode && ! pTextNode->IsCountedInList())
        {
            OSL_ENSURE( pTextNode->GetActualListLevel() == GetNumInfo().GetLevel(),
                    "Numbering level is wrong" );

            pTextNode->SetCountedInList( true );

            // It's necessary to invalidate the rule, because between the reading
            // of LI and the graphic an EndAction could be called.
            if( GetNumInfo().GetNumRule() )
                GetNumInfo().GetNumRule()->SetInvalidRule( true );

            // Set the style again, so that indent of the first line is correct.
            SetTextCollAttrs();

            return;
        }
    }

    Graphic aGraphic;
    INetURLObject aGraphicURL( sGrfNm );
    if( aGraphicURL.GetProtocol() == INetProtocol::Data )
    {
        std::unique_ptr<SvMemoryStream> const pStream(aGraphicURL.getData());
        if (pStream)
        {
            GraphicFilter& rFilter = GraphicFilter::GetGraphicFilter();
            aGraphic = rFilter.ImportUnloadedGraphic(*pStream);
                sGrfNm.clear();

            if (!sGrfNm.isEmpty())
            {
                if (ERRCODE_NONE == rFilter.ImportGraphic(aGraphic, "", *pStream))
                    sGrfNm.clear();
            }
        }
    }
    else if (m_sBaseURL.isEmpty() || !aGraphicData.isEmpty())
    {
        // sBaseURL is empty if the source is clipboard
        // aGraphicData is non-empty for <object data="..."> -> not a linked graphic.
        if (ERRCODE_NONE == GraphicFilter::GetGraphicFilter().ImportGraphic(aGraphic, aGraphicURL))
            sGrfNm.clear();
    }

    if (!sGrfNm.isEmpty())
    {
        aGraphic.SetDefaultType();
    }

    if (!nHeight || !nWidth)
    {
        Size aPixelSize = aGraphic.GetSizePixel(Application::GetDefaultDevice());
        if (!bWidthProvided)
            nWidth = aPixelSize.Width();
        if (!bHeightProvided)
            nHeight = aPixelSize.Height();
    }

    SfxItemSet aItemSet( m_xDoc->GetAttrPool(), m_pCSS1Parser->GetWhichMap() );
    SvxCSS1PropertyInfo aPropInfo;
    if( HasStyleOptions( aStyle, aId, aClass ) )
        ParseStyleOptions( aStyle, aId, aClass, aItemSet, aPropInfo );

    SfxItemSet aFrameSet( m_xDoc->GetAttrPool(),
                        svl::Items<RES_FRMATR_BEGIN, RES_FRMATR_END-1>{} );
    if( !IsNewDoc() )
        Reader::ResetFrameFormatAttrs( aFrameSet );

    // set the border
    long nHBorderWidth = 0, nVBorderWidth = 0;
    if( nBorder )
    {
        nHBorderWidth = static_cast<long>(nBorder);
        nVBorderWidth = static_cast<long>(nBorder);
        SvxCSS1Parser::PixelToTwip( nVBorderWidth, nHBorderWidth );

        ::editeng::SvxBorderLine aHBorderLine( nullptr, nHBorderWidth );
        ::editeng::SvxBorderLine aVBorderLine( nullptr, nVBorderWidth );

        if( m_xAttrTab->pINetFormat )
        {
            const OUString& rURL =
                static_cast<const SwFormatINetFormat&>(m_xAttrTab->pINetFormat->GetItem()).GetValue();

            m_pCSS1Parser->SetATagStyles();
            sal_uInt16 nPoolId =  static_cast< sal_uInt16 >(m_xDoc->IsVisitedURL( rURL )
                                    ? RES_POOLCHR_INET_VISIT
                                    : RES_POOLCHR_INET_NORMAL);
            const SwCharFormat *pCharFormat = m_pCSS1Parser->GetCharFormatFromPool( nPoolId );
            aHBorderLine.SetColor( pCharFormat->GetColor().GetValue() );
            aVBorderLine.SetColor( aHBorderLine.GetColor() );
        }
        else
        {
            const SvxColorItem& rColorItem = m_xAttrTab->pFontColor ?
              static_cast<const SvxColorItem &>(m_xAttrTab->pFontColor->GetItem()) :
              m_xDoc->GetDefault(RES_CHRATR_COLOR);
            aHBorderLine.SetColor( rColorItem.GetValue() );
            aVBorderLine.SetColor( aHBorderLine.GetColor() );
        }

        SvxBoxItem aBoxItem( RES_BOX );
        aBoxItem.SetLine( &aHBorderLine, SvxBoxItemLine::TOP );
        aBoxItem.SetLine( &aHBorderLine, SvxBoxItemLine::BOTTOM );
        aBoxItem.SetLine( &aVBorderLine, SvxBoxItemLine::LEFT );
        aBoxItem.SetLine( &aVBorderLine, SvxBoxItemLine::RIGHT );
        aFrameSet.Put( aBoxItem );
    }

    SetAnchorAndAdjustment( eVertOri, eHoriOri, aPropInfo, aFrameSet );

    SetSpace( Size( nHSpace, nVSpace), aItemSet, aPropInfo, aFrameSet );

    // set other CSS1 attributes
    SetFrameFormatAttrs( aItemSet, HtmlFrameFormatFlags::Box, aFrameSet );

    Size aTwipSz( bPrcWidth ? 0 : nWidth, bPrcHeight ? 0 : nHeight );
    if( (aTwipSz.Width() || aTwipSz.Height()) && Application::GetDefaultDevice() )
    {
        if (bWidthProvided || bHeightProvided || // attributes imply pixel!
            aGraphic.GetPrefMapMode().GetMapUnit() == MapUnit::MapPixel)
        {
            aTwipSz = Application::GetDefaultDevice()
                    ->PixelToLogic( aTwipSz, MapMode( MapUnit::MapTwip ) );
        }
        else
        {   // some bitmaps may have a size in metric units (e.g. PNG); use that
            assert(aGraphic.GetPrefMapMode().GetMapUnit() < MapUnit::MapPixel);
            aTwipSz = OutputDevice::LogicToLogic(aGraphic.GetPrefSize(),
                    aGraphic.GetPrefMapMode(), MapMode(MapUnit::MapTwip));
        }
    }

    // convert CSS1 size to "normal" size
    switch( aPropInfo.m_eWidthType )
    {
        case SVX_CSS1_LTYPE_TWIP:
            aTwipSz.setWidth( aPropInfo.m_nWidth );
            nWidth = 1; // != 0
            bPrcWidth = false;
            break;
        case SVX_CSS1_LTYPE_PERCENTAGE:
            aTwipSz.setWidth( 0 );
            nWidth = aPropInfo.m_nWidth;
            bPrcWidth = true;
            break;
        default:
            ;
    }
    switch( aPropInfo.m_eHeightType )
    {
        case SVX_CSS1_LTYPE_TWIP:
            aTwipSz.setHeight( aPropInfo.m_nHeight );
            nHeight = 1;    // != 0
            bPrcHeight = false;
            break;
        case SVX_CSS1_LTYPE_PERCENTAGE:
            aTwipSz.setHeight( 0 );
            nHeight = aPropInfo.m_nHeight;
            bPrcHeight = true;
            break;
        default:
            ;
    }

    Size aGrfSz( 0, 0 );
    bool bSetTwipSize = true;       // Set Twip-Size on Node?
    bool bChangeFrameSize = false;    // Change frame format later?
    bool bRequestGrfNow = false;
    bool bSetScaleImageMap = false;
    sal_uInt8 nPrcWidth = 0, nPrcHeight = 0;

    if ((!nWidth || !nHeight) && allowAccessLink(*m_xDoc))
    {
        GraphicDescriptor aDescriptor(aGraphicURL);
        if (aDescriptor.Detect(/*bExtendedInfo=*/true))
        {
            // Try to use size info from the image header before defaulting to
            // HTML_DFLT_IMG_WIDTH/HEIGHT.
            aTwipSz = Application::GetDefaultDevice()->PixelToLogic(aDescriptor.GetSizePixel(),
                                                                    MapMode(MapUnit::MapTwip));
            nWidth = aTwipSz.getWidth();
            nHeight = aTwipSz.getHeight();
        }
    }

    if( !nWidth || !nHeight )
    {
        // When the graphic is in a table, it will be requested immediately,
        // so that it is available before the table is layouted.
        if (m_xTable && !nWidth)
        {
            bRequestGrfNow = true;
            IncGrfsThatResizeTable();
        }

        // The frame size is set later
        bChangeFrameSize = true;
        aGrfSz = aTwipSz;
        if( !nWidth && !nHeight )
        {
            aTwipSz.setWidth( HTML_DFLT_IMG_WIDTH );
            aTwipSz.setHeight( HTML_DFLT_IMG_HEIGHT );
        }
        else if( nWidth )
        {
            // a percentage value
            if( bPrcWidth )
            {
                nPrcWidth = static_cast<sal_uInt8>(nWidth);
                nPrcHeight = 255;
            }
            else
            {
                aTwipSz.setHeight( HTML_DFLT_IMG_HEIGHT );
            }
        }
        else if( nHeight )
        {
            if( bPrcHeight )
            {
                nPrcHeight = static_cast<sal_uInt8>(nHeight);
                nPrcWidth = 255;
            }
            else
            {
                aTwipSz.setWidth( HTML_DFLT_IMG_WIDTH );
            }
        }
    }
    else
    {
        // Width and height were given and don't need to be set
        bSetTwipSize = false;

        if( bPrcWidth )
            nPrcWidth = static_cast<sal_uInt8>(nWidth);

        if( bPrcHeight )
            nPrcHeight = static_cast<sal_uInt8>(nHeight);
    }

    // set image map
    aMap = comphelper::string::stripEnd(aMap, ' ');
    if( !aMap.isEmpty() )
    {
        // Since we only know local image maps we just use everything
        // after # as name
        sal_Int32 nPos = aMap.indexOf( '#' );
        OUString aName;
        if ( -1 == nPos )
            aName = aMap ;
        else
            aName = aMap.copy(nPos+1);

        ImageMap *pImgMap = FindImageMap( aName );
        if( pImgMap )
        {
            SwFormatURL aURL; aURL.SetMap( pImgMap );// is copied

            bSetScaleImageMap = !nPrcWidth || !nPrcHeight;
            aFrameSet.Put( aURL );
        }
        else
        {
            ImageMap aEmptyImgMap( aName );
            SwFormatURL aURL; aURL.SetMap( &aEmptyImgMap );// is copied
            aFrameSet.Put( aURL );
            m_nMissingImgMaps++;          // image maps are missing

            // the graphic has to scaled during SetTwipSize, if we didn't
            // set a size on the node or the size doesn't match the graphic size.
            bSetScaleImageMap = true;
        }
    }

    // observe minimum values !!
    if( nPrcWidth )
    {
        OSL_ENSURE( !aTwipSz.Width(),
                "Why is a width set if we already have percentage value?" );
        aTwipSz.setWidth( aGrfSz.Width() ? aGrfSz.Width()
                                         : HTML_DFLT_IMG_WIDTH );
    }
    else
    {
        aTwipSz.AdjustWidth(2*nVBorderWidth );
        if( aTwipSz.Width() < MINFLY )
            aTwipSz.setWidth( MINFLY );
    }
    if( nPrcHeight )
    {
        OSL_ENSURE( !aTwipSz.Height(),
                "Why is a height set if we already have percentage value?" );
        aTwipSz.setHeight( aGrfSz.Height() ? aGrfSz.Height()
                                           : HTML_DFLT_IMG_HEIGHT );
    }
    else
    {
        aTwipSz.AdjustHeight(2*nHBorderWidth );
        if( aTwipSz.Height() < MINFLY )
            aTwipSz.setHeight( MINFLY );
    }

    SwFormatFrameSize aFrameSize( ATT_FIX_SIZE, aTwipSz.Width(), aTwipSz.Height() );
    aFrameSize.SetWidthPercent( nPrcWidth );
    aFrameSize.SetHeightPercent( nPrcHeight );
    aFrameSet.Put( aFrameSize );

    const SwNodeType eNodeType = m_pPam->GetNode().GetNodeType();
    if (eNodeType != SwNodeType::Text && eNodeType != SwNodeType::Table)
        return;

    // passing empty sGrfNm here, means we don't want the graphic to be linked
    SwFrameFormat *const pFlyFormat =
        m_xDoc->getIDocumentContentOperations().InsertGraphic(
            *m_pPam, sGrfNm, aEmptyOUStr, &aGraphic,
            &aFrameSet, nullptr, nullptr);
    SwGrfNode *pGrfNd = m_xDoc->GetNodes()[ pFlyFormat->GetContent().GetContentIdx()
                                  ->GetIndex()+1 ]->GetGrfNode();

    if( !sHTMLGrfName.isEmpty() )
    {
        pFlyFormat->SetName( sHTMLGrfName );

        // maybe jump to graphic
        if( JUMPTO_GRAPHIC == m_eJumpTo && sHTMLGrfName == m_sJmpMark )
        {
            m_bChkJumpMark = true;
            m_eJumpTo = JUMPTO_NONE;
        }
    }

    if (pGrfNd)
    {
        if( !sAltNm.isEmpty() )
            pGrfNd->SetTitle( sAltNm );

        if( bSetTwipSize )
            pGrfNd->SetTwipSize( aGrfSz );

        pGrfNd->SetChgTwipSize( bChangeFrameSize );

        if( bSetScaleImageMap )
            pGrfNd->SetScaleImageMap( true );
    }

    if( m_xAttrTab->pINetFormat )
    {
        const SwFormatINetFormat &rINetFormat =
            static_cast<const SwFormatINetFormat&>(m_xAttrTab->pINetFormat->GetItem());

        SwFormatURL aURL( pFlyFormat->GetURL() );

        aURL.SetURL( rINetFormat.GetValue(), bIsMap );
        aURL.SetTargetFrameName( rINetFormat.GetTargetFrame() );
        aURL.SetName( rINetFormat.GetName() );
        pFlyFormat->SetFormatAttr( aURL );

        {
            static const SvMacroItemId aEvents[] = {
                SvMacroItemId::OnMouseOver,
                SvMacroItemId::OnClick,
                SvMacroItemId::OnMouseOut };

            for( SvMacroItemId id : aEvents )
            {
                const SvxMacro *pMacro = rINetFormat.GetMacro( id );
                if( nullptr != pMacro )
                    aMacroItem.SetMacro( id, *pMacro );
            }
        }

        if ((RndStdIds::FLY_AS_CHAR == pFlyFormat->GetAnchor().GetAnchorId()) &&
            m_xAttrTab->pINetFormat->GetSttPara() ==
                        m_pPam->GetPoint()->nNode &&
            m_xAttrTab->pINetFormat->GetSttCnt() ==
                        m_pPam->GetPoint()->nContent.GetIndex() - 1 )
        {
            // the attribute was insert right before as-character anchored
            // graphic, therefore we move it
            m_xAttrTab->pINetFormat->SetStart( *m_pPam->GetPoint() );

            // When the attribute is also an anchor, we'll insert
            // a bookmark before the graphic, because SwFormatURL
            // isn't an anchor.
            if( !rINetFormat.GetName().isEmpty() )
            {
                m_pPam->Move( fnMoveBackward );
                InsertBookmark( rINetFormat.GetName() );
                m_pPam->Move( fnMoveForward );
            }
        }

    }

    if( !aMacroItem.GetMacroTable().empty() )
        pFlyFormat->SetFormatAttr( aMacroItem );

    // tdf#87083 If the graphic has not been loaded yet, then load it now.
    // Otherwise it may be loaded during the first paint of the object and it
    // will be too late to adapt the size of the graphic at that point.
    if (bRequestGrfNow && pGrfNd)
    {
        Size aUpdatedSize = pGrfNd->GetTwipSize();  //trigger a swap-in
        SAL_WARN_IF(!aUpdatedSize.Width() || !aUpdatedSize.Height(), "sw.html", "html image with no width or height");
    }

    // maybe create frames and register auto bound frames
    RegisterFlyFrame( pFlyFormat );

    if( !aId.isEmpty() )
        InsertBookmark( aId );
}

/*  */

void SwHTMLParser::InsertBodyOptions()
{
    m_xDoc->SetTextFormatColl( *m_pPam,
                         m_pCSS1Parser->GetTextCollFromPool( RES_POOLCOLL_TEXT ) );

    OUString aBackGround, aId, aStyle, aLang, aDir;
    Color aBGColor, aTextColor, aLinkColor, aVLinkColor;
    bool bBGColor=false, bTextColor=false;
    bool bLinkColor=false, bVLinkColor=false;

    ScriptType eDfltScriptType;
    OUString sDfltScriptType;
    GetDefaultScriptType( eDfltScriptType, sDfltScriptType );

    const HTMLOptions& rHTMLOptions = GetOptions();
    for (size_t i = rHTMLOptions.size(); i; )
    {
        const HTMLOption& rOption = rHTMLOptions[--i];
        ScriptType eScriptType2 = eDfltScriptType;
        OUString aEvent;
        bool bSetEvent = false;

        switch( rOption.GetToken() )
        {
            case HtmlOptionId::ID:
                aId = rOption.GetString();
                break;
            case HtmlOptionId::BACKGROUND:
                aBackGround = rOption.GetString();
                break;
            case HtmlOptionId::BGCOLOR:
                rOption.GetColor( aBGColor );
                bBGColor = true;
                break;
            case HtmlOptionId::TEXT:
                rOption.GetColor( aTextColor );
                bTextColor = true;
                break;
            case HtmlOptionId::LINK:
                rOption.GetColor( aLinkColor );
                bLinkColor = true;
                break;
            case HtmlOptionId::VLINK:
                rOption.GetColor( aVLinkColor );
                bVLinkColor = true;
                break;

            case HtmlOptionId::SDONLOAD:
                eScriptType2 = STARBASIC;
                SAL_FALLTHROUGH;
            case HtmlOptionId::ONLOAD:
                aEvent = GlobalEventConfig::GetEventName( GlobalEventId::OPENDOC );
                bSetEvent = true;
                break;

            case HtmlOptionId::SDONUNLOAD:
                eScriptType2 = STARBASIC;
                SAL_FALLTHROUGH;
            case HtmlOptionId::ONUNLOAD:
                aEvent = GlobalEventConfig::GetEventName( GlobalEventId::PREPARECLOSEDOC );
                bSetEvent = true;
                break;

            case HtmlOptionId::SDONFOCUS:
                eScriptType2 = STARBASIC;
                SAL_FALLTHROUGH;
            case HtmlOptionId::ONFOCUS:
                aEvent = GlobalEventConfig::GetEventName( GlobalEventId::ACTIVATEDOC );
                bSetEvent = true;
                break;

            case HtmlOptionId::SDONBLUR:
                eScriptType2 = STARBASIC;
                SAL_FALLTHROUGH;
            case HtmlOptionId::ONBLUR:
                aEvent = GlobalEventConfig::GetEventName( GlobalEventId::DEACTIVATEDOC );
                bSetEvent = true;
                break;

            case HtmlOptionId::ONERROR:
                break;

            case HtmlOptionId::STYLE:
                aStyle = rOption.GetString();
                bTextColor = true;
                break;
            case HtmlOptionId::LANG:
                aLang = rOption.GetString();
                break;
            case HtmlOptionId::DIR:
                aDir = rOption.GetString();
                break;
            default: break;
        }

        if( bSetEvent )
        {
            const OUString& rEvent = rOption.GetString();
            if( !rEvent.isEmpty() )
                InsertBasicDocEvent( aEvent, rEvent, eScriptType2,
                                     sDfltScriptType );
        }
    }

    if( bTextColor && !m_pCSS1Parser->IsBodyTextSet() )
    {
        // The font colour is set in the default style
        m_pCSS1Parser->GetTextCollFromPool( RES_POOLCOLL_STANDARD )
            ->SetFormatAttr( SvxColorItem(aTextColor, RES_CHRATR_COLOR) );
        m_pCSS1Parser->SetBodyTextSet();
    }

    // Prepare the items for the page style (background, frame)
    // If BrushItem already set values must remain!
    SvxBrushItem aBrushItem( m_pCSS1Parser->makePageDescBackground() );
    bool bSetBrush = false;

    if( bBGColor && !m_pCSS1Parser->IsBodyBGColorSet() )
    {
        // background colour from "BGCOLOR"
        OUString aLink;
        if( !aBrushItem.GetGraphicLink().isEmpty() )
            aLink = aBrushItem.GetGraphicLink();
        SvxGraphicPosition ePos = aBrushItem.GetGraphicPos();

        aBrushItem.SetColor( aBGColor );

        if( !aLink.isEmpty() )
        {
            aBrushItem.SetGraphicLink( aLink );
            aBrushItem.SetGraphicPos( ePos );
        }
        bSetBrush = true;
        m_pCSS1Parser->SetBodyBGColorSet();
    }

    if( !aBackGround.isEmpty() && !m_pCSS1Parser->IsBodyBackgroundSet() )
    {
        // background graphic from "BACKGROUND"
        aBrushItem.SetGraphicLink( INetURLObject::GetAbsURL( m_sBaseURL, aBackGround ) );
        aBrushItem.SetGraphicPos( GPOS_TILED );
        bSetBrush = true;
        m_pCSS1Parser->SetBodyBackgroundSet();
    }

    if( !aStyle.isEmpty() || !aDir.isEmpty() )
    {
        SfxItemSet aItemSet( m_xDoc->GetAttrPool(), m_pCSS1Parser->GetWhichMap() );
        SvxCSS1PropertyInfo aPropInfo;
        OUString aDummy;
        ParseStyleOptions( aStyle, aDummy, aDummy, aItemSet, aPropInfo, nullptr, &aDir );

        // Some attributes have to set on the page style, in fact the ones
        // which aren't inherited
        m_pCSS1Parser->SetPageDescAttrs( bSetBrush ? &aBrushItem : nullptr,
                                       &aItemSet );

        const SfxPoolItem *pItem;
        static const sal_uInt16 aWhichIds[3] = { RES_CHRATR_FONTSIZE,
                                       RES_CHRATR_CJK_FONTSIZE,
                                       RES_CHRATR_CTL_FONTSIZE };
        for(sal_uInt16 i : aWhichIds)
        {
            if( SfxItemState::SET == aItemSet.GetItemState( i, false,
                                                       &pItem ) &&
                static_cast <const SvxFontHeightItem * >(pItem)->GetProp() != 100)
            {
                sal_uInt32 nHeight =
                    ( m_aFontHeights[2] *
                     static_cast <const SvxFontHeightItem * >(pItem)->GetProp() ) / 100;
                SvxFontHeightItem aNewItem( nHeight, 100, i );
                aItemSet.Put( aNewItem );
            }
        }

        // all remaining options can be set on the default style
        m_pCSS1Parser->GetTextCollFromPool( RES_POOLCOLL_STANDARD )
            ->SetFormatAttr( aItemSet );
    }
    else if( bSetBrush )
    {
        m_pCSS1Parser->SetPageDescAttrs( &aBrushItem );
    }

    if( bLinkColor && !m_pCSS1Parser->IsBodyLinkSet() )
    {
        SwCharFormat *pCharFormat =
            m_pCSS1Parser->GetCharFormatFromPool(RES_POOLCHR_INET_NORMAL);
        pCharFormat->SetFormatAttr( SvxColorItem(aLinkColor, RES_CHRATR_COLOR) );
        m_pCSS1Parser->SetBodyLinkSet();
    }
    if( bVLinkColor && !m_pCSS1Parser->IsBodyVLinkSet() )
    {
        SwCharFormat *pCharFormat =
            m_pCSS1Parser->GetCharFormatFromPool(RES_POOLCHR_INET_VISIT);
        pCharFormat->SetFormatAttr( SvxColorItem(aVLinkColor, RES_CHRATR_COLOR) );
        m_pCSS1Parser->SetBodyVLinkSet();
    }
    if( !aLang.isEmpty() )
    {
        LanguageType eLang = LanguageTag::convertToLanguageTypeWithFallback( aLang );
        if( LANGUAGE_DONTKNOW != eLang )
        {
            sal_uInt16 nWhich = 0;
            switch( SvtLanguageOptions::GetScriptTypeOfLanguage( eLang ) )
            {
            case SvtScriptType::LATIN:
                nWhich = RES_CHRATR_LANGUAGE;
                break;
            case SvtScriptType::ASIAN:
                nWhich = RES_CHRATR_CJK_LANGUAGE;
                break;
            case SvtScriptType::COMPLEX:
                nWhich = RES_CHRATR_CTL_LANGUAGE;
                break;
            default: break;
            }
            if( nWhich )
            {
                SvxLanguageItem aLanguage( eLang, nWhich );
                aLanguage.SetWhich( nWhich );
                m_xDoc->SetDefault( aLanguage );
            }
        }
    }

    if( !aId.isEmpty() )
        InsertBookmark( aId );
}

/*  */

void SwHTMLParser::NewAnchor()
{
    // end previous link if there was one
    std::unique_ptr<HTMLAttrContext> xOldCntxt(PopContext(HtmlTokenId::ANCHOR_ON));
    if (xOldCntxt)
    {
        // and maybe end attributes
        EndContext(xOldCntxt.get());
    }

    SvxMacroTableDtor aMacroTable;
    OUString sHRef, aName, sTarget;
    OUString aId, aStyle, aClass, aLang, aDir;
    bool bHasHRef = false, bFixed = false;

    ScriptType eDfltScriptType;
    OUString sDfltScriptType;
    GetDefaultScriptType( eDfltScriptType, sDfltScriptType );

    const HTMLOptions& rHTMLOptions = GetOptions();
    for (size_t i = rHTMLOptions.size(); i; )
    {
        SvMacroItemId nEvent = SvMacroItemId::NONE;
        ScriptType eScriptType2 = eDfltScriptType;
        const HTMLOption& rOption = rHTMLOptions[--i];
        switch( rOption.GetToken() )
        {
            case HtmlOptionId::NAME:
                aName = rOption.GetString();
                break;

            case HtmlOptionId::HREF:
                sHRef = rOption.GetString();
                bHasHRef = true;
                break;
            case HtmlOptionId::TARGET:
                sTarget = rOption.GetString();
                break;

            case HtmlOptionId::STYLE:
                aStyle = rOption.GetString();
                break;
            case HtmlOptionId::ID:
                aId = rOption.GetString();
                break;
            case HtmlOptionId::CLASS:
                aClass = rOption.GetString();
                break;
            case HtmlOptionId::SDFIXED:
                bFixed = true;
                break;
            case HtmlOptionId::LANG:
                aLang = rOption.GetString();
                break;
            case HtmlOptionId::DIR:
                aDir = rOption.GetString();
                break;

            case HtmlOptionId::SDONCLICK:
                eScriptType2 = STARBASIC;
                SAL_FALLTHROUGH;
            case HtmlOptionId::ONCLICK:
                nEvent = SvMacroItemId::OnClick;
                goto ANCHOR_SETEVENT;

            case HtmlOptionId::SDONMOUSEOVER:
                eScriptType2 = STARBASIC;
                SAL_FALLTHROUGH;
            case HtmlOptionId::ONMOUSEOVER:
                nEvent = SvMacroItemId::OnMouseOver;
                goto ANCHOR_SETEVENT;

            case HtmlOptionId::SDONMOUSEOUT:
                eScriptType2 = STARBASIC;
                SAL_FALLTHROUGH;
            case HtmlOptionId::ONMOUSEOUT:
                nEvent = SvMacroItemId::OnMouseOut;
                goto ANCHOR_SETEVENT;
ANCHOR_SETEVENT:
                {
                    OUString sTmp( rOption.GetString() );
                    if( !sTmp.isEmpty() )
                    {
                        sTmp = convertLineEnd(sTmp, GetSystemLineEnd());
                        OUString sScriptType;
                        if( EXTENDED_STYPE == eScriptType2 )
                            sScriptType = sDfltScriptType;
                        aMacroTable.Insert( nEvent, SvxMacro( sTmp, sScriptType, eScriptType2 ));
                    }
                }
                break;
            default: break;
        }
    }

    // Jump targets, which match our implicit targets,
    // here we throw out rigorously.
    if( !aName.isEmpty() )
    {
        OUString sDecoded( INetURLObject::decode( aName,
                                           INetURLObject::DecodeMechanism::Unambiguous ));
        sal_Int32 nPos = sDecoded.lastIndexOf( cMarkSeparator );
        if( nPos != -1 )
        {
            OUString sCmp= sDecoded.copy(nPos+1).replaceAll(" ","");
            if( !sCmp.isEmpty() )
            {
                sCmp = sCmp.toAsciiLowerCase();
                if( sCmp == "region" ||
                    sCmp == "frame" ||
                    sCmp == "graphic" ||
                    sCmp == "ole" ||
                    sCmp == "table" ||
                    sCmp == "outline" ||
                    sCmp == "text" )
                {
                    aName.clear();
                }
            }
        }
    }

    // create a new context
    std::unique_ptr<HTMLAttrContext> xCntxt(new HTMLAttrContext(HtmlTokenId::ANCHOR_ON));

    bool bEnAnchor = false, bFootnoteAnchor = false, bFootnoteEnSymbol = false;
    OUString aFootnoteName;
    OUString aStrippedClass( aClass );
    SwCSS1Parser::GetScriptFromClass( aStrippedClass, false );
    if( aStrippedClass.getLength() >=9  && bHasHRef && sHRef.getLength() > 1 &&
        ('s' == aStrippedClass[0] || 'S' == aStrippedClass[0]) &&
        ('d' == aStrippedClass[1] || 'D' == aStrippedClass[1]) )
    {
        if( aStrippedClass.equalsIgnoreAsciiCase( OOO_STRING_SVTOOLS_HTML_sdendnote_anc ) )
            bEnAnchor = true;
        else if( aStrippedClass.equalsIgnoreAsciiCase( OOO_STRING_SVTOOLS_HTML_sdfootnote_anc ) )
            bFootnoteAnchor = true;
        else if( aStrippedClass.equalsIgnoreAsciiCase( OOO_STRING_SVTOOLS_HTML_sdendnote_sym ) ||
                 aStrippedClass.equalsIgnoreAsciiCase( OOO_STRING_SVTOOLS_HTML_sdfootnote_sym ) )
            bFootnoteEnSymbol = true;
        if( bEnAnchor || bFootnoteAnchor || bFootnoteEnSymbol )
        {
            aFootnoteName = sHRef.copy( 1 );
            aClass = aStrippedClass = aName = aEmptyOUStr;
            bHasHRef = false;
        }
    }

    // Styles parsen
    if( HasStyleOptions( aStyle, aId, aStrippedClass, &aLang, &aDir ) )
    {
        SfxItemSet aItemSet( m_xDoc->GetAttrPool(), m_pCSS1Parser->GetWhichMap() );
        SvxCSS1PropertyInfo aPropInfo;

        if( ParseStyleOptions( aStyle, aId, aClass, aItemSet, aPropInfo, &aLang, &aDir ) )
        {
            DoPositioning(aItemSet, aPropInfo, xCntxt.get());
            InsertAttrs(aItemSet, aPropInfo, xCntxt.get(), true);
        }
    }

    if( bHasHRef )
    {
        if( !sHRef.isEmpty() )
        {
            sHRef = URIHelper::SmartRel2Abs( INetURLObject(m_sBaseURL), sHRef, Link<OUString *, bool>(), false );
        }
        else
        {
            // use directory if empty URL
            INetURLObject aURLObj( m_aPathToFile );
            sHRef = aURLObj.GetPartBeforeLastName();
        }

        m_pCSS1Parser->SetATagStyles();
        SwFormatINetFormat aINetFormat( sHRef, sTarget );
        aINetFormat.SetName( aName );

        if( !aMacroTable.empty() )
            aINetFormat.SetMacroTable( &aMacroTable );

        // set the default attribute
        InsertAttr(&m_xAttrTab->pINetFormat, aINetFormat, xCntxt.get());
    }
    else if( !aName.isEmpty() )
    {
        InsertBookmark( aName );
    }

    if( bEnAnchor || bFootnoteAnchor )
    {
        InsertFootEndNote( aFootnoteName, bEnAnchor, bFixed );
        m_bInFootEndNoteAnchor = m_bCallNextToken = true;
    }
    else if( bFootnoteEnSymbol )
    {
        m_bInFootEndNoteSymbol = m_bCallNextToken = true;
    }

    // save context
    PushContext(xCntxt);
}

void SwHTMLParser::EndAnchor()
{
    if( m_bInFootEndNoteAnchor )
    {
        FinishFootEndNote();
        m_bInFootEndNoteAnchor = false;
    }
    else if( m_bInFootEndNoteSymbol )
    {
        m_bInFootEndNoteSymbol = false;
    }

    EndTag( HtmlTokenId::ANCHOR_OFF );
}

/*  */

void SwHTMLParser::InsertBookmark( const OUString& rName )
{
    HTMLAttr* pTmp = new HTMLAttr( *m_pPam->GetPoint(),
            SfxStringItem(RES_FLTR_BOOKMARK, rName), nullptr, std::shared_ptr<HTMLAttrTable>());
    m_aSetAttrTab.push_back( pTmp );
}

bool SwHTMLParser::HasCurrentParaBookmarks( bool bIgnoreStack ) const
{
    bool bHasMarks = false;
    sal_uLong nNodeIdx = m_pPam->GetPoint()->nNode.GetIndex();

    // first step: are there still bookmark in the attribute-stack?
    // bookmarks are added to the end of the stack - thus we only have
    // to check the last bookmark
    if( !bIgnoreStack )
    {
        for( auto i = m_aSetAttrTab.size(); i; )
        {
            HTMLAttr* pAttr = m_aSetAttrTab[ --i ];
            if( RES_FLTR_BOOKMARK == pAttr->pItem->Which() )
            {
                if( pAttr->GetSttParaIdx() == nNodeIdx )
                    bHasMarks = true;
                break;
            }
        }
    }

    if( !bHasMarks )
    {
        // second step: when we didn't find a bookmark, check if there is one set already
        IDocumentMarkAccess* const pMarkAccess = m_xDoc->getIDocumentMarkAccess();
        for(IDocumentMarkAccess::const_iterator_t ppMark = pMarkAccess->getAllMarksBegin();
            ppMark != pMarkAccess->getAllMarksEnd();
            ++ppMark)
        {
            const ::sw::mark::IMark* pBookmark = ppMark->get();

            const sal_uLong nBookNdIdx = pBookmark->GetMarkPos().nNode.GetIndex();
            if( nBookNdIdx==nNodeIdx )
            {
                bHasMarks = true;
                break;
            }
            else if( nBookNdIdx > nNodeIdx )
                break;
        }
    }

    return bHasMarks;
}

/*  */

void SwHTMLParser::StripTrailingPara()
{
    bool bSetSmallFont = false;

    SwContentNode* pCNd = m_pPam->GetContentNode();
    sal_uLong nNodeIdx = m_pPam->GetPoint()->nNode.GetIndex();
    if( !m_pPam->GetPoint()->nContent.GetIndex() )
    {
        if( pCNd && pCNd->StartOfSectionIndex() + 2 <
            pCNd->EndOfSectionIndex() && CanRemoveNode(nNodeIdx))
        {
            const SwFrameFormats& rFrameFormatTable = *m_xDoc->GetSpzFrameFormats();

            for( auto pFormat : rFrameFormatTable )
            {
                SwFormatAnchor const*const pAnchor = &pFormat->GetAnchor();
                SwPosition const*const pAPos = pAnchor->GetContentAnchor();
                if (pAPos &&
                    ((RndStdIds::FLY_AT_PARA == pAnchor->GetAnchorId()) ||
                     (RndStdIds::FLY_AT_CHAR == pAnchor->GetAnchorId())) &&
                    pAPos->nNode == nNodeIdx )

                    return;     // we can't delete the node
            }

            SetAttr( false );   // the still open attributes must be
                                // closed before the node is deleted,
                                // otherwise the last index is dangling

            if( pCNd->Len() && pCNd->IsTextNode() )
            {
                // fields were inserted into the node, now they have
                // to be moved
                SwTextNode *pPrvNd = m_xDoc->GetNodes()[nNodeIdx-1]->GetTextNode();
                if( pPrvNd )
                {
                    SwIndex aSrc( pCNd, 0 );
                    pCNd->GetTextNode()->CutText( pPrvNd, aSrc, pCNd->Len() );
                }
            }

            // now we have to move maybe existing bookmarks
            IDocumentMarkAccess* const pMarkAccess = m_xDoc->getIDocumentMarkAccess();
            for(IDocumentMarkAccess::const_iterator_t ppMark = pMarkAccess->getAllMarksBegin();
                ppMark != pMarkAccess->getAllMarksEnd();
                ++ppMark)
            {
                ::sw::mark::IMark* pMark = ppMark->get();

                sal_uLong nBookNdIdx = pMark->GetMarkPos().nNode.GetIndex();
                if(nBookNdIdx==nNodeIdx)
                {
                    SwNodeIndex nNewNdIdx(m_pPam->GetPoint()->nNode);
                    SwContentNode* pNd = SwNodes::GoPrevious(&nNewNdIdx);
                    if(!pNd)
                    {
                        OSL_ENSURE(false, "Oops, where is my predecessor node?");
                        return;
                    }
                    // #i81002# - refactoring
                    // Do not directly manipulate member of <SwBookmark>
                    {
                        SwPosition aNewPos(*pNd);
                        aNewPos.nContent.Assign(pNd, pNd->Len());
                        const SwPaM aPaM(aNewPos);
                        pMarkAccess->repositionMark(ppMark->get(), aPaM);
                    }
                }
                else if( nBookNdIdx > nNodeIdx )
                    break;
            }

            m_pPam->GetPoint()->nContent.Assign( nullptr, 0 );
            m_pPam->SetMark();
            m_pPam->DeleteMark();
            m_xDoc->GetNodes().Delete( m_pPam->GetPoint()->nNode );
            m_pPam->Move( fnMoveBackward, GoInNode );
        }
        else if (pCNd && pCNd->IsTextNode() && m_xTable)
        {
            // In empty cells we set a small font, so that the cell doesn't
            // get higher than the graphic resp. as low as possible.
            bSetSmallFont = true;
        }
    }
    else if( pCNd && pCNd->IsTextNode() && m_xTable &&
             pCNd->StartOfSectionIndex()+2 ==
             pCNd->EndOfSectionIndex() )
    {
        // When the cell contains only as-character anchored graphics/frames,
        // then we also set a small font.
        bSetSmallFont = true;
        SwTextNode* pTextNd = pCNd->GetTextNode();

        sal_Int32 nPos = m_pPam->GetPoint()->nContent.GetIndex();
        while( bSetSmallFont && nPos>0 )
        {
            --nPos;
            bSetSmallFont =
                (CH_TXTATR_BREAKWORD == pTextNd->GetText()[nPos]) &&
                (nullptr != pTextNd->GetTextAttrForCharAt( nPos, RES_TXTATR_FLYCNT ));
        }
    }

    if( bSetSmallFont )
    {
        // Added default to CJK and CTL
        SvxFontHeightItem aFontHeight( 40, 100, RES_CHRATR_FONTSIZE );
        pCNd->SetAttr( aFontHeight );
        SvxFontHeightItem aFontHeightCJK( 40, 100, RES_CHRATR_CJK_FONTSIZE );
        pCNd->SetAttr( aFontHeightCJK );
        SvxFontHeightItem aFontHeightCTL( 40, 100, RES_CHRATR_CTL_FONTSIZE );
        pCNd->SetAttr( aFontHeightCTL );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
