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

#include <sal/config.h>
#include <sal/log.hxx>
#include <string_view>

#include <pdfiprocessor.hxx>
#include <xmlemitter.hxx>
#include <pdfihelper.hxx>
#include <imagecontainer.hxx>
#include "style.hxx"
#include "writertreevisiting.hxx"
#include <genericelements.hxx>

#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <osl/diagnose.h>
#include <com/sun/star/i18n/CharacterClassification.hpp>
#include <com/sun/star/i18n/DirectionProperty.hpp>
#include <comphelper/string.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star::uno;

namespace pdfi
{

const Reference<XBreakIterator>& WriterXmlOptimizer::GetBreakIterator()
{
    if (!mxBreakIter.is())
    {
        mxBreakIter = BreakIterator::create(m_rProcessor.m_xContext);
    }
    return mxBreakIter;
}

const Reference< XCharacterClassification >& WriterXmlEmitter::GetCharacterClassification()
{
    if ( !mxCharClass.is() )
    {
        Reference< XComponentContext > xContext( m_rEmitContext.m_xContext, uno::UNO_SET_THROW );
        mxCharClass = CharacterClassification::create(xContext);
    }
    return mxCharClass;
}

void WriterXmlEmitter::visit( HyperlinkElement& elem, const std::list< std::unique_ptr<Element> >::const_iterator&   )
{
    if( elem.Children.empty() )
        return;

    const char* pType = dynamic_cast<DrawElement*>(elem.Children.front().get()) ? "draw:a" : "text:a";

    PropertyMap aProps;
    aProps[ u"xlink:type"_ustr ] = "simple";
    aProps[ u"xlink:href"_ustr ] = elem.URI;
    aProps[ u"office:target-frame-name"_ustr ] = "_blank";
    aProps[ u"xlink:show"_ustr ] = "new";

    m_rEmitContext.rEmitter.beginTag( pType, aProps );
    auto this_it = elem.Children.begin();
    while( this_it != elem.Children.end() && this_it->get() != &elem )
    {
        (*this_it)->visitedBy( *this, this_it );
        ++this_it;
    }
    m_rEmitContext.rEmitter.endTag( pType );
}

void WriterXmlEmitter::visit( TextElement& elem, const std::list< std::unique_ptr<Element> >::const_iterator&   )
{
    if( elem.Text.isEmpty() )
        return;

    PropertyMap aProps = {};
    const sal_Unicode strSpace = 0x0020;
    const sal_Unicode strNbSpace = 0x00A0;
    const sal_Unicode tabSpace = 0x0009;

    if( elem.StyleId != -1 )
    {
        aProps[ u"text:style-name"_ustr ] =
            m_rEmitContext.rStyles.getStyleName( elem.StyleId );
    }

    OUString str(elem.Text.toString());

    // Check for RTL
    bool isRTL = false;
    Reference< i18n::XCharacterClassification > xCC( GetCharacterClassification() );
    if( xCC.is() )
    {
        for(int i=1; i< elem.Text.getLength(); i++)
        {
            i18n::DirectionProperty nType = static_cast<i18n::DirectionProperty>(xCC->getCharacterDirection( str, i ));
            if ( nType == i18n::DirectionProperty_RIGHT_TO_LEFT           ||
                 nType == i18n::DirectionProperty_RIGHT_TO_LEFT_ARABIC    ||
                 nType == i18n::DirectionProperty_RIGHT_TO_LEFT_EMBEDDING ||
                 nType == i18n::DirectionProperty_RIGHT_TO_LEFT_OVERRIDE
                )
                isRTL = true;
        }
    }

    if (isRTL)  // If so, reverse string
    {
        // First, produce mirrored-image for each code point which has the Bidi_Mirrored property.
        str = PDFIProcessor::SubstituteBidiMirrored(str);
        // Then, reverse the code points in the string, in backward order.
        str = ::comphelper::string::reverseCodePoints(str);
    }

    m_rEmitContext.rEmitter.beginTag( "text:span", aProps );

    sal_Unicode strToken;
    for (int i = 0; i < elem.Text.getLength(); i++)
    {
        strToken = str[i];
        if (strToken == strSpace || strToken == strNbSpace)
        {
            aProps[u"text:c"_ustr] = "1";
            m_rEmitContext.rEmitter.beginTag("text:s", aProps);
            m_rEmitContext.rEmitter.endTag("text:s");
        }
        else if (strToken == tabSpace)
        {
            m_rEmitContext.rEmitter.beginTag("text:tab", aProps);
            m_rEmitContext.rEmitter.endTag("text:tab");
        }
        else
            m_rEmitContext.rEmitter.write(OUString(strToken));
    }

    auto this_it = elem.Children.begin();
    while( this_it != elem.Children.end() && this_it->get() != &elem )
    {
        (*this_it)->visitedBy( *this, this_it );
        ++this_it;
    }

    m_rEmitContext.rEmitter.endTag( "text:span" );
}

void WriterXmlEmitter::visit( ParagraphElement& elem, const std::list< std::unique_ptr<Element> >::const_iterator&   )
{
    PropertyMap aProps;
    if( elem.StyleId != -1 )
    {
        aProps[ u"text:style-name"_ustr ] = m_rEmitContext.rStyles.getStyleName( elem.StyleId );
    }
    const char* pTagType = "text:p";
    if( elem.Type == ParagraphElement::Headline )
        pTagType = "text:h";
    m_rEmitContext.rEmitter.beginTag( pTagType, aProps );

    auto this_it = elem.Children.begin();
    while( this_it != elem.Children.end() && this_it->get() != &elem )
    {
        (*this_it)->visitedBy( *this, this_it );
        ++this_it;
    }

    m_rEmitContext.rEmitter.endTag( pTagType );
}

void WriterXmlEmitter::fillFrameProps( DrawElement&       rElem,
                                       PropertyMap&       rProps,
                                       const EmitContext& rEmitContext )
{
    double rel_x = rElem.x, rel_y = rElem.y;

    // find anchor type by recursing though parents
    Element* pAnchor = &rElem;
    ParagraphElement* pParaElt = nullptr;
    PageElement* pPage = nullptr;
    while ((pAnchor = pAnchor->Parent))
    {
        if ((pParaElt = dynamic_cast<ParagraphElement*>(pAnchor)))
            break;
        if ((pPage = dynamic_cast<PageElement*>(pAnchor)))
            break;
    }
    if( pAnchor )
    {
        if (pParaElt)
        {
            rProps[ u"text:anchor-type"_ustr ] = rElem.isCharacter
                ? std::u16string_view(u"character") : std::u16string_view(u"paragraph");
        }
        else
        {
            assert(pPage); // guaranteed by the while loop above
            rProps[ u"text:anchor-type"_ustr ] = "page";
            rProps[ u"text:anchor-page-number"_ustr ] = OUString::number(pPage->PageNumber);
        }
        rel_x -= pAnchor->x;
        rel_y -= pAnchor->y;
    }

    rProps[ u"draw:z-index"_ustr ] = OUString::number( rElem.ZOrder );
    rProps[ u"draw:style-name"_ustr] = rEmitContext.rStyles.getStyleName( rElem.StyleId );
    rProps[ u"svg:width"_ustr ]   = convertPixelToUnitString( rElem.w );
    rProps[ u"svg:height"_ustr ]  = convertPixelToUnitString( rElem.h );

    const GraphicsContext& rGC =
        rEmitContext.rProcessor.getGraphicsContext( rElem.GCId );
    if( rGC.Transformation.isIdentity() )
    {
        if( !rElem.isCharacter )
        {
            rProps[ u"svg:x"_ustr ]       = convertPixelToUnitString( rel_x );
            rProps[ u"svg:y"_ustr ]       = convertPixelToUnitString( rel_y );
        }
    }
    else
    {
        basegfx::B2DTuple aScale, aTranslation;
        double fRotate, fShearX;

        rGC.Transformation.decompose( aScale, aTranslation, fRotate, fShearX );

        OUStringBuffer aBuf( 256 );

        // TODO(F2): general transformation case missing; if implemented, note
        // that ODF rotation is oriented the other way

        // build transformation string
        if (rElem.MirrorVertical)
        {
            // At some point, rElem.h may start arriving positive,
            // so use robust adjusting math
            rel_y -= std::abs(rElem.h);
            if (!aBuf.isEmpty())
                aBuf.append(' ');
            aBuf.append("scale( 1.0 -1.0 )");
        }
        if( fShearX != 0.0 )
        {
            aBuf.append( "skewX( " + OUString::number(fShearX) + " )" );
        }
        if( fRotate != 0.0 )
        {
            if( !aBuf.isEmpty() )
                aBuf.append( ' ' );
            aBuf.append( "rotate( " + OUString::number(-fRotate) + " )" );

        }
        if( ! rElem.isCharacter )
        {
            if( !aBuf.isEmpty() )
                aBuf.append( ' ' );
            aBuf.append( "translate( "
                + convertPixelToUnitString( rel_x )
                + " "
                + convertPixelToUnitString( rel_y )
                + " )" );
        }

        rProps[ u"draw:transform"_ustr ] = aBuf.makeStringAndClear();
    }
}

void WriterXmlEmitter::visit( FrameElement& elem, const std::list< std::unique_ptr<Element> >::const_iterator&   )
{
    if( elem.Children.empty() )
        return;

    bool bTextBox = (dynamic_cast<ParagraphElement*>(elem.Children.front().get()) != nullptr);
    PropertyMap aFrameProps;
    fillFrameProps( elem, aFrameProps, m_rEmitContext );
    m_rEmitContext.rEmitter.beginTag( "draw:frame", aFrameProps );
    if( bTextBox )
        m_rEmitContext.rEmitter.beginTag( "draw:text-box", PropertyMap() );

    auto this_it = elem.Children.begin();
    while( this_it != elem.Children.end() && this_it->get() != &elem )
    {
        (*this_it)->visitedBy( *this, this_it );
        ++this_it;
    }

    if( bTextBox )
        m_rEmitContext.rEmitter.endTag( "draw:text-box" );
    m_rEmitContext.rEmitter.endTag( "draw:frame" );
}

void WriterXmlEmitter::visit( PolyPolyElement& elem, const std::list< std::unique_ptr<Element> >::const_iterator& )
{
    elem.updateGeometry();
    /* note:
     *   aw recommends using 100dth of mm in all respects since the xml import
     *   (a) is buggy (see issue 37213)
     *   (b) is optimized for 100dth of mm and does not scale itself then,
     *       this does not gain us speed but makes for smaller rounding errors since
     *       the xml importer coordinates are integer based
     */
    for (sal_uInt32 i = 0; i< elem.PolyPoly.count(); i++)
    {
        basegfx::B2DPolygon b2dPolygon =  elem.PolyPoly.getB2DPolygon( i );

        for ( sal_uInt32 j = 0; j< b2dPolygon.count(); j++ )
        {
            basegfx::B2DPoint point;
            basegfx::B2DPoint nextPoint;
            point = b2dPolygon.getB2DPoint( j );

            basegfx::B2DPoint prevPoint = b2dPolygon.getPrevControlPoint( j ) ;

            point.setX( convPx2mmPrec2( point.getX() )*100.0 );
            point.setY( convPx2mmPrec2( point.getY() )*100.0 );

            if ( b2dPolygon.isPrevControlPointUsed( j ) )
            {
                prevPoint.setX( convPx2mmPrec2( prevPoint.getX() )*100.0 );
                prevPoint.setY( convPx2mmPrec2( prevPoint.getY() )*100.0 );
            }

            if ( b2dPolygon.isNextControlPointUsed( j ) )
            {
                nextPoint = b2dPolygon.getNextControlPoint( j ) ;
                nextPoint.setX( convPx2mmPrec2( nextPoint.getX() )*100.0 );
                nextPoint.setY( convPx2mmPrec2( nextPoint.getY() )*100.0 );
            }

            b2dPolygon.setB2DPoint( j, point );

            if ( b2dPolygon.isPrevControlPointUsed( j ) )
                b2dPolygon.setPrevControlPoint( j , prevPoint ) ;

            if ( b2dPolygon.isNextControlPointUsed( j ) )
                b2dPolygon.setNextControlPoint( j , nextPoint ) ;
        }

        elem.PolyPoly.setB2DPolygon( i, b2dPolygon );
    }

    PropertyMap aProps;
    fillFrameProps( elem, aProps, m_rEmitContext );
    aProps[ u"svg:viewBox"_ustr ] =
        "0 0 "
        + OUString::number(convPx2mmPrec2(elem.w)*100.0)
        + " "
        + OUString::number( convPx2mmPrec2(elem.h)*100.0 );
    aProps[ u"svg:d"_ustr ]       = basegfx::utils::exportToSvgD( elem.PolyPoly, true, true, false );

    m_rEmitContext.rEmitter.beginTag( "draw:path", aProps );
    m_rEmitContext.rEmitter.endTag( "draw:path" );
}

void WriterXmlEmitter::visit( ImageElement& elem, const std::list< std::unique_ptr<Element> >::const_iterator& )
{
    PropertyMap aImageProps;
    m_rEmitContext.rEmitter.beginTag( "draw:image", aImageProps );
    m_rEmitContext.rEmitter.beginTag( "office:binary-data", PropertyMap() );
    m_rEmitContext.rImages.writeBase64EncodedStream( elem.Image, m_rEmitContext);
    m_rEmitContext.rEmitter.endTag( "office:binary-data" );
    m_rEmitContext.rEmitter.endTag( "draw:image" );
}

void WriterXmlEmitter::visit( PageElement& elem, const std::list< std::unique_ptr<Element> >::const_iterator&   )
{
    if( m_rEmitContext.xStatusIndicator.is() )
        m_rEmitContext.xStatusIndicator->setValue( elem.PageNumber );

    auto this_it =  elem.Children.begin();
    while( this_it != elem.Children.end() && this_it->get() != &elem )
    {
        (*this_it)->visitedBy( *this, this_it );
        ++this_it;
    }
}

void WriterXmlEmitter::visit( DocumentElement& elem, const std::list< std::unique_ptr<Element> >::const_iterator&)
{
    m_rEmitContext.rEmitter.beginTag( "office:body", PropertyMap() );
    m_rEmitContext.rEmitter.beginTag( "office:text", PropertyMap() );

    for( const auto& rxChild : elem.Children )
    {
        PageElement* pPage = dynamic_cast<PageElement*>(rxChild.get());
        if( pPage )
        {
            // emit only page anchored objects
            // currently these are only DrawElement types
            for( auto child_it = pPage->Children.begin(); child_it != pPage->Children.end(); ++child_it )
            {
                if( dynamic_cast<DrawElement*>(child_it->get()) != nullptr )
                    (*child_it)->visitedBy( *this, child_it );
            }
        }
    }

    // do not emit page anchored objects, they are emitted before
    // (must precede all pages in writer document) currently these are
    // only DrawElement types
    for( auto it = elem.Children.begin(); it != elem.Children.end(); ++it )
    {
        if( dynamic_cast<DrawElement*>(it->get()) == nullptr )
            (*it)->visitedBy( *this, it );
    }

    m_rEmitContext.rEmitter.endTag( "office:text" );
    m_rEmitContext.rEmitter.endTag( "office:body" );
}


void WriterXmlOptimizer::visit( HyperlinkElement&, const std::list< std::unique_ptr<Element> >::const_iterator& )
{
}

void WriterXmlOptimizer::visit( TextElement&, const std::list< std::unique_ptr<Element> >::const_iterator&)
{
}

void WriterXmlOptimizer::visit( FrameElement& elem, const std::list< std::unique_ptr<Element> >::const_iterator& )
{
    elem.applyToChildren(*this);
}

void WriterXmlOptimizer::visit( ImageElement&, const std::list< std::unique_ptr<Element> >::const_iterator& )
{
}

void WriterXmlOptimizer::visit( PolyPolyElement& elem, const std::list< std::unique_ptr<Element> >::const_iterator& elemIt )
{
    /* note: optimize two consecutive PolyPolyElements that
     *  have the same path but one of which is a stroke while
     *     the other is a fill
     */
    if( !elem.Parent )
        return;
    // find following PolyPolyElement in parent's children list
    if( elemIt == elem.Parent->Children.end() )
        return;
    auto next_it = elemIt;
    ++next_it;
    if( next_it == elem.Parent->Children.end() )
        return;

    PolyPolyElement* pNext = dynamic_cast<PolyPolyElement*>(next_it->get());
    if( !pNext || pNext->PolyPoly != elem.PolyPoly )
        return;

    const GraphicsContext& rNextGC =
                  m_rProcessor.getGraphicsContext( pNext->GCId );
    const GraphicsContext& rThisGC =
                  m_rProcessor.getGraphicsContext( elem.GCId );

    if( !(rThisGC.BlendMode      == rNextGC.BlendMode &&
        rThisGC.Flatness       == rNextGC.Flatness &&
        rThisGC.Transformation == rNextGC.Transformation &&
        rThisGC.Clip           == rNextGC.Clip &&
        pNext->Action          == PATH_STROKE &&
        (elem.Action == PATH_FILL || elem.Action == PATH_EOFILL)) )
        return;

    GraphicsContext aGC = rThisGC;
    aGC.LineJoin  = rNextGC.LineJoin;
    aGC.LineCap   = rNextGC.LineCap;
    aGC.LineWidth = rNextGC.LineWidth;
    aGC.MiterLimit= rNextGC.MiterLimit;
    aGC.DashArray = rNextGC.DashArray;
    aGC.LineColor = rNextGC.LineColor;
    elem.GCId = m_rProcessor.getGCId( aGC );

    elem.Action |= pNext->Action;

    elem.Children.splice( elem.Children.end(), pNext->Children );
    elem.Parent->Children.erase(next_it);
}

void WriterXmlOptimizer::visit( ParagraphElement& elem, const std::list< std::unique_ptr<Element> >::const_iterator& rParentIt)
{
    optimizeTextElements( elem );

    elem.applyToChildren(*this);

    if( !(elem.Parent && rParentIt != elem.Parent->Children.end()) )
        return;

    // find if there is a previous paragraph that might be a heading for this one
    auto prev = rParentIt;
    ParagraphElement* pPrevPara = nullptr;
    while( prev != elem.Parent->Children.begin() )
    {
        --prev;
        pPrevPara = dynamic_cast< ParagraphElement* >(prev->get());
        if( pPrevPara )
        {
            /* What constitutes a heading ? current hints are:
             * - one line only
             * - not too far away from this paragraph (two heading height max ?)
             * - font larger or bold
             * this is of course incomplete
             * FIXME: improve hints for heading
             */
            // check for single line
            if( pPrevPara->isSingleLined( m_rProcessor ) )
            {
                double head_line_height = pPrevPara->getLineHeight( m_rProcessor );
                if( pPrevPara->y + pPrevPara->h + 2*head_line_height > elem.y )
                {
                    // check for larger font
                    if( head_line_height > elem.getLineHeight( m_rProcessor ) )
                    {
                        pPrevPara->Type = ParagraphElement::Headline;
                    }
                    else
                    {
                        // check whether text of pPrevPara is bold (at least first text element)
                        // and this para is not bold (ditto)
                        TextElement* pPrevText = pPrevPara->getFirstTextChild();
                        TextElement* pThisText = elem.getFirstTextChild();
                        if( pPrevText && pThisText )
                        {
                            const FontAttributes& rPrevFont = m_rProcessor.getFont( pPrevText->FontId );
                            const FontAttributes& rThisFont = m_rProcessor.getFont( pThisText->FontId );
                            if ( (rPrevFont.fontWeight ==  u"600" ||
                                  rPrevFont.fontWeight ==  u"bold" ||
                                  rPrevFont.fontWeight ==  u"800" ||
                                  rPrevFont.fontWeight ==  u"900" )  &&
                                 (rThisFont.fontWeight ==  u"600" ||
                                  rThisFont.fontWeight ==  u"bold" ||
                                  rThisFont.fontWeight ==  u"800" ||
                                  rThisFont.fontWeight ==  u"900" ) )
                            {
                                pPrevPara->Type = ParagraphElement::Headline;
                            }
                        }
                    }
                }
            }
            break;
        }
    }
}

void WriterXmlOptimizer::visit( PageElement& elem, const std::list< std::unique_ptr<Element> >::const_iterator& )
{
    if( m_rProcessor.getStatusIndicator().is() )
        m_rProcessor.getStatusIndicator()->setValue( elem.PageNumber );

    // resolve hyperlinks
    elem.resolveHyperlinks();

    elem.resolveFontStyles( m_rProcessor ); // underlines and such

    // FIXME: until hyperlinks and font effects are adjusted for
    // geometrical search handle them before sorting
    PDFIProcessor::sortElements( &elem );

    // find paragraphs in text
    ParagraphElement* pCurPara = nullptr;
    std::list< std::unique_ptr<Element> >::iterator page_element, next_page_element;
    next_page_element = elem.Children.begin();
    double fCurLineHeight = 0.0; // average height of text items in current para
    int nCurLineElements = 0; // number of line contributing elements in current para
    double line_left = elem.w, line_right = 0.0;
    double column_width = elem.w*0.75; // estimate text width
    // TODO: guess columns
    while( next_page_element != elem.Children.end() )
    {
        page_element = next_page_element++;
        ParagraphElement* pPagePara = dynamic_cast<ParagraphElement*>(page_element->get());
        if( pPagePara )
        {
            pCurPara = pPagePara;
            // adjust line height and text items
            fCurLineHeight = 0.0;
            nCurLineElements = 0;
            for( const auto& rxChild : pCurPara->Children )
            {
                TextElement* pTestText = rxChild->dynCastAsTextElement();
                if( pTestText )
                {
                    fCurLineHeight = (fCurLineHeight*double(nCurLineElements) + pTestText->h)/double(nCurLineElements+1);
                    nCurLineElements++;
                }
            }
            continue;
        }

        HyperlinkElement* pLink = dynamic_cast<HyperlinkElement*>(page_element->get());
        DrawElement* pDraw = dynamic_cast<DrawElement*>(page_element->get());
        if( ! pDraw && pLink && ! pLink->Children.empty() )
            pDraw = dynamic_cast<DrawElement*>(pLink->Children.front().get() );
        if( pDraw )
        {
            // insert small drawing objects as character, else leave them page bound

            bool bInsertToParagraph = false;
            // first check if this is either inside the paragraph
            if( pCurPara && pDraw->y < pCurPara->y + pCurPara->h )
            {
                if( pDraw->h < fCurLineHeight * 1.5 )
                {
                    bInsertToParagraph = true;
                    fCurLineHeight = (fCurLineHeight*double(nCurLineElements) + pDraw->h)/double(nCurLineElements+1);
                    nCurLineElements++;
                    // mark draw element as character
                    pDraw->isCharacter = true;
                }
            }
            // or perhaps the draw element begins a new paragraph
            else if( next_page_element != elem.Children.end() )
            {
                TextElement* pText = (*next_page_element)->dynCastAsTextElement();
                if( ! pText )
                {
                    ParagraphElement* pPara = dynamic_cast<ParagraphElement*>(next_page_element->get());
                    if( pPara && ! pPara->Children.empty() )
                        pText = pPara->Children.front()->dynCastAsTextElement();
                }
                if( pText && // check there is a text
                    pDraw->h < pText->h*1.5 && // and it is approx the same height
                    // and either upper or lower edge of pDraw is inside text's vertical range
                    ( ( pDraw->y >= pText->y && pDraw->y <= pText->y+pText->h ) ||
                      ( pDraw->y+pDraw->h >= pText->y && pDraw->y+pDraw->h <= pText->y+pText->h )
                      )
                    )
                {
                    bInsertToParagraph = true;
                    fCurLineHeight = pDraw->h;
                    nCurLineElements = 1;
                    line_left = pDraw->x;
                    line_right = pDraw->x + pDraw->w;
                    // begin a new paragraph
                    pCurPara = nullptr;
                    // mark draw element as character
                    pDraw->isCharacter = true;
                }
            }

            if( ! bInsertToParagraph )
            {
                pCurPara = nullptr;
                continue;
            }
        }

        TextElement* pText = (*page_element)->dynCastAsTextElement();
        if( ! pText && pLink && ! pLink->Children.empty() )
            pText = pLink->Children.front()->dynCastAsTextElement();
        if( pText )
        {
            Element* pGeo = pLink ? static_cast<Element*>(pLink) :
                                    static_cast<Element*>(pText);
            if( pCurPara )
            {
                // there was already a text element, check for a new paragraph
                if( nCurLineElements > 0 )
                {
                    // if the new text is significantly distant from the paragraph
                    // begin a new paragraph
                    if( pGeo->y > pCurPara->y+pCurPara->h + fCurLineHeight*0.5 )
                        pCurPara = nullptr; // insert new paragraph
                    else if( pGeo->y > (pCurPara->y+pCurPara->h - fCurLineHeight*0.05) )
                    {
                        // new paragraph if either the last line of the paragraph
                        // was significantly shorter than the paragraph as a whole
                        if( (line_right - line_left) < pCurPara->w*0.75 )
                            pCurPara = nullptr;
                        // or the last line was significantly smaller than the column width
                        else if( (line_right - line_left) < column_width*0.75 )
                            pCurPara = nullptr;
                    }
                }
            }
            // update line height/width
            if( pCurPara )
            {
                fCurLineHeight = (fCurLineHeight*double(nCurLineElements) + pGeo->h)/double(nCurLineElements+1);
                nCurLineElements++;
                if( pGeo->x < line_left )
                    line_left = pGeo->x;
                if( pGeo->x+pGeo->w > line_right )
                    line_right = pGeo->x+pGeo->w;
            }
            else
            {
                fCurLineHeight = pGeo->h;
                nCurLineElements = 1;
                line_left = pGeo->x;
                line_right = pGeo->x + pGeo->w;
            }
        }

        // move element to current paragraph
        if( ! pCurPara ) // new paragraph, insert one
        {
            pCurPara = ElementFactory::createParagraphElement( nullptr );
            assert(pCurPara);
            // set parent
            pCurPara->Parent = &elem;
            //insert new paragraph before current element
            page_element = elem.Children.insert( page_element, std::unique_ptr<Element>(pCurPara) );
            // forward iterator to current element again
            ++ page_element;
            // update next_element which is now invalid
            next_page_element = page_element;
            ++ next_page_element;
        }
        Element* pCurEle = page_element->get();
        Element::setParent( page_element, pCurPara );
        OSL_ENSURE( !pText || pCurEle == pText || pCurEle == pLink, "paragraph child list in disorder" );
        if( pText || pDraw )
            pCurPara->updateGeometryWith( pCurEle );
    }

    // process children
    elem.applyToChildren(*this);

    // find possible header and footer
    checkHeaderAndFooter( elem );
}

void WriterXmlOptimizer::checkHeaderAndFooter( PageElement& rElem )
{
    /* indicators for a header:
     *  - single line paragraph at top of page (inside 15% page height)
     *  - at least lineheight above the next paragraph
     *
     *  indicators for a footer likewise:
     *  - single line paragraph at bottom of page (inside 15% page height)
     *  - at least lineheight below the previous paragraph
     */

    auto isParagraphElement = [](std::unique_ptr<Element>& rxChild) -> bool {
        return dynamic_cast<ParagraphElement*>(rxChild.get()) != nullptr;
    };

    // detect header
    // Note: the following assumes that the pages' children have been
    // sorted geometrically
    auto it = std::find_if(rElem.Children.begin(), rElem.Children.end(), isParagraphElement);
    if (it != rElem.Children.end())
    {
        ParagraphElement& rPara = dynamic_cast<ParagraphElement&>(**it);
        if( rPara.y+rPara.h < rElem.h*0.15 && rPara.isSingleLined( m_rProcessor ) )
        {
            auto next_it = it;
            ParagraphElement* pNextPara = nullptr;
            while( ++next_it != rElem.Children.end() && pNextPara == nullptr )
            {
                pNextPara = dynamic_cast<ParagraphElement*>(next_it->get());
            }
            if( pNextPara && pNextPara->y > rPara.y+rPara.h*2 )
            {
                rElem.HeaderElement = std::move(*it);
                rPara.Parent = nullptr;
                rElem.Children.erase( it );
            }
        }
    }

    // detect footer
    auto rit = std::find_if(rElem.Children.rbegin(), rElem.Children.rend(), isParagraphElement);
    if (rit == rElem.Children.rend())
        return;

    ParagraphElement& rPara = dynamic_cast<ParagraphElement&>(**rit);
    if( !(rPara.y > rElem.h*0.85 && rPara.isSingleLined( m_rProcessor )) )
        return;

    std::list< std::unique_ptr<Element> >::reverse_iterator next_it = rit;
    ParagraphElement* pNextPara = nullptr;
    while( ++next_it != rElem.Children.rend() && pNextPara == nullptr )
    {
        pNextPara = dynamic_cast<ParagraphElement*>(next_it->get());
    }
    if( pNextPara && pNextPara->y < rPara.y-rPara.h*2 )
    {
        rElem.FooterElement = std::move(*rit);
        rPara.Parent = nullptr;
        rElem.Children.erase( std::next(rit).base() );
    }
}

void WriterXmlOptimizer::optimizeTextElements(Element& rParent)
{
    if( rParent.Children.empty() ) // this should not happen
    {
        OSL_FAIL( "empty paragraph optimized" );
        return;
    }

    // concatenate child elements with same font id
    auto next = rParent.Children.begin();
    auto it = next++;
    FrameElement* pFrame = dynamic_cast<FrameElement*>(rParent.Parent);
    bool bRotatedFrame = false;
    if( pFrame )
    {
        const GraphicsContext& rFrameGC = m_rProcessor.getGraphicsContext( pFrame->GCId );
        if( rFrameGC.isRotatedOrSkewed() )
            bRotatedFrame = true;
    }
    while( next != rParent.Children.end() )
    {
        bool bConcat = false;
        TextElement* pCur = (*it)->dynCastAsTextElement();
        if( pCur )
        {
            TextElement* pNext = dynamic_cast<TextElement*>(next->get());
            OUString str;
            bool bPara = strspn("ParagraphElement", typeid(rParent).name());
            ParagraphElement* pPara = dynamic_cast<ParagraphElement*>(&rParent);
            if (bPara && pPara && isComplex(GetBreakIterator(), pCur))
                pPara->bRtl = true;
            if( pNext )
            {
                const GraphicsContext& rCurGC = m_rProcessor.getGraphicsContext( pCur->GCId );
                const GraphicsContext& rNextGC = m_rProcessor.getGraphicsContext( pNext->GCId );

                // line and space optimization; works only in strictly horizontal mode

                if( !bRotatedFrame
                    && ! rCurGC.isRotatedOrSkewed()
                    && ! rNextGC.isRotatedOrSkewed()
                    && ! pNext->Text.isEmpty()
                    && pNext->Text[0] != ' '
                    && ! pCur->Text.isEmpty()
                    && pCur->Text[pCur->Text.getLength() - 1] != ' '
                    )
                {
                    // check for new line in paragraph
                    if( pNext->y > pCur->y+pCur->h )
                    {
                        // new line begins
                        // check whether a space would should be inserted or a hyphen removed
                        sal_Unicode aLastCode = pCur->Text[pCur->Text.getLength() - 1];
                        if( aLastCode == '-'
                            || aLastCode == 0x2010
                            || (aLastCode >= 0x2012 && aLastCode <= 0x2015)
                            || aLastCode == 0xff0d
                        )
                        {
                            // cut a hyphen
                            pCur->Text.setLength( pCur->Text.getLength()-1 );
                        }
                        // append a space unless there is a non breaking hyphen
                        else if( aLastCode != 0x2011 )
                        {
                            pCur->Text.append( ' ' );
                        }
                    }
                    else // we're continuing the same line
                    {
                        // check whether a space would should be inserted
                        // check for a small horizontal offset
                        if( pCur->x + pCur->w + pNext->h*0.15 < pNext->x )
                        {
                            pCur->Text.append( ' ' );
                        }
                    }
                }
                // concatenate consecutive text elements unless there is a
                // font or text color change, leave a new span in that case
                if( pCur->FontId == pNext->FontId &&
                    rCurGC.FillColor.Red == rNextGC.FillColor.Red &&
                    rCurGC.FillColor.Green == rNextGC.FillColor.Green &&
                    rCurGC.FillColor.Blue == rNextGC.FillColor.Blue &&
                    rCurGC.FillColor.Alpha == rNextGC.FillColor.Alpha
                    )
                {
                    pCur->updateGeometryWith( pNext );
                    if (pPara && pPara->bRtl)
                    {
                        // Tdf#152083: If RTL, reverse the text in pNext so that its correct order is
                        // restored when the combined text is reversed in WriterXmlEmitter::visit.
                        OUString tempStr;
                        bool bNeedReverse=false;
                        str = pNext->Text.toString();
                        for (sal_Int32 i=0; i < str.getLength(); i++)
                        {
                            if (str[i] == u' ')
                            {   // Space char (e.g. the space as in " م") needs special treatment.
                                //   First, append the space char to pCur.
                                pCur->Text.append(OUStringChar(str[i]));
                                //   Then, check whether the tmpStr needs reverse, if so then reverse and append.
                                if (bNeedReverse)
                                {
                                    tempStr = ::comphelper::string::reverseCodePoints(tempStr);
                                    pCur->Text.append(tempStr);
                                    tempStr = u""_ustr;
                                }
                                bNeedReverse = false;
                            }
                            else
                            {
                                tempStr += OUStringChar(str[i]);
                                bNeedReverse = true;
                            }
                        }
                        // Do the last append
                        if (bNeedReverse)
                        {
                            tempStr = ::comphelper::string::reverseCodePoints(tempStr);
                            pCur->Text.append(tempStr);
                        }
                        else
                        {
                            pCur->Text.append(tempStr);
                        }
                    }
                    else
                    {
                        // append text to current element directly without reverse
                        pCur->Text.append(pNext->Text);
                    }
                    if (bPara && pPara && isComplex(GetBreakIterator(), pCur))
                        pPara->bRtl = true;
                    // append eventual children to current element
                    // and clear children (else the children just
                    // appended to pCur would be destroyed)
                    pCur->Children.splice( pCur->Children.end(), pNext->Children );
                    // get rid of the now useless element
                    rParent.Children.erase( next );
                    bConcat = true;
                }
            }
        }
        else if( dynamic_cast<HyperlinkElement*>(it->get()) )
            optimizeTextElements( **it );
        if( bConcat )
        {
            next = it;
            ++next;
        }
        else
        {
            ++it;
            ++next;
        }
    }
}

void WriterXmlOptimizer::visit( DocumentElement& elem, const std::list< std::unique_ptr<Element> >::const_iterator&)
{
    elem.applyToChildren(*this);
}


void WriterXmlFinalizer::visit( PolyPolyElement& elem, const std::list< std::unique_ptr<Element> >::const_iterator& )
{
    // xxx TODO copied from DrawElement
    const GraphicsContext& rGC = m_rProcessor.getGraphicsContext(elem.GCId );
    PropertyMap aProps;
    aProps[ u"style:family"_ustr ] = "graphic";

    PropertyMap aGCProps;
    if (elem.Action & PATH_STROKE)
    {
        double scale = GetAverageTransformationScale(rGC.Transformation);
        if (rGC.DashArray.size() < 2)
        {
            aGCProps[ u"draw:stroke"_ustr ] = "solid";
        }
        else
        {
            PropertyMap props;
            FillDashStyleProps(props, rGC.DashArray, scale);
            StyleContainer::Style style("draw:stroke-dash"_ostr, std::move(props));

            aGCProps[ u"draw:stroke"_ustr ] = "dash";
            aGCProps[ u"draw:stroke-dash"_ustr ] =
                m_rStyleContainer.getStyleName(
                m_rStyleContainer.getStyleId(style));
        }

        aGCProps[ u"svg:stroke-color"_ustr ] = getColorString(rGC.LineColor);
        aGCProps[ u"svg:stroke-width"_ustr ] = convertPixelToUnitString(rGC.LineWidth * scale);
        aGCProps[ u"draw:stroke-linejoin"_ustr ] = rGC.GetLineJoinString();
        aGCProps[ u"svg:stroke-linecap"_ustr ] = rGC.GetLineCapString();
    }
    else
    {
        aGCProps[ u"draw:stroke"_ustr ] = "none";
    }

    if (elem.FillImage != -1)
    {
        PropertyMap props;
        // The image isn't actually in a prop, it's in an extra chunk inside.
        StyleContainer::Style style("draw:fill-image"_ostr, std::move(props));
        style.Contents = m_rProcessor.getImages().asBase64EncodedString(elem.FillImage);
        aGCProps[ u"draw:fill-image-name"_ustr ] =
            m_rStyleContainer.getStyleName(
            m_rStyleContainer.getStyleId(style));
        aGCProps[ u"draw:fill-image-width"_ustr ] = unitMMString(convPx2mm(elem.TileWidth));
        aGCProps[ u"draw:fill-image-height"_ustr ] = unitMMString(convPx2mm(elem.TileHeight));
    }

    // TODO(F1): check whether stuff could be emulated by gradient/bitmap/hatch
    if( elem.Action & (PATH_FILL | PATH_EOFILL) )
    {
        if (elem.FillImage == -1)
        {
            aGCProps[ u"draw:fill"_ustr ]   = "solid";
        }
        else
        {
            aGCProps[ u"draw:fill"_ustr ]   = "bitmap";
        }
        aGCProps[ u"draw:fill-color"_ustr ] = getColorString( rGC.FillColor );
    }
    else
    {
        aGCProps[ u"draw:fill"_ustr ] = "none";
    }

    StyleContainer::Style aStyle( "style:style"_ostr, std::move(aProps) );
    StyleContainer::Style aSubStyle( "style:graphic-properties"_ostr, std::move(aGCProps) );
    aStyle.SubStyles.push_back( &aSubStyle );

    elem.StyleId = m_rStyleContainer.getStyleId( aStyle );
}

void WriterXmlFinalizer::visit( HyperlinkElement&, const std::list< std::unique_ptr<Element> >::const_iterator& )
{
}

void WriterXmlFinalizer::visit( TextElement& elem, const std::list< std::unique_ptr<Element> >::const_iterator& )
{
    const FontAttributes& rFont = m_rProcessor.getFont( elem.FontId );
    PropertyMap aProps;
    aProps[ u"style:family"_ustr ] = "text";

    PropertyMap aFontProps;

    // family name
    // TODO: tdf#143095: use system font name rather than PSName
    SAL_INFO("sdext.pdfimport", "The font used in xml is: " << rFont.familyName);
    aFontProps[ u"fo:font-family"_ustr ] = rFont.familyName;
    aFontProps[ u"style:font-family-asian"_ustr ] = rFont.familyName;
    aFontProps[ u"style:font-family-complex"_ustr ] = rFont.familyName;

    // bold
    aFontProps[ u"fo:font-weight"_ustr ]            = rFont.fontWeight;
    aFontProps[ u"style:font-weight-asian"_ustr ]   = rFont.fontWeight;
    aFontProps[ u"style:font-weight-complex"_ustr ] = rFont.fontWeight;

    // italic
    if( rFont.isItalic )
    {
        aFontProps[ u"fo:font-style"_ustr ]            = "italic";
        aFontProps[ u"style:font-style-asian"_ustr ]   = "italic";
        aFontProps[ u"style:font-style-complex"_ustr ] = "italic";
    }

    // underline
    if( rFont.isUnderline )
    {
        aFontProps[ u"style:text-underline-style"_ustr ]  = "solid";
        aFontProps[ u"style:text-underline-width"_ustr ]  = "auto";
        aFontProps[ u"style:text-underline-color"_ustr ]  = "font-color";
    }

    // outline
    if( rFont.isOutline )
        aFontProps[ u"style:text-outline"_ustr ]  = "true";

    // size
    OUString aFSize = OUString::number( rFont.size*72/PDFI_OUTDEV_RESOLUTION ) + "pt";
    aFontProps[ u"fo:font-size"_ustr ]            = aFSize;
    aFontProps[ u"style:font-size-asian"_ustr ]   = aFSize;
    aFontProps[ u"style:font-size-complex"_ustr ] = aFSize;

    // color
    const GraphicsContext& rGC = m_rProcessor.getGraphicsContext( elem.GCId );
    aFontProps[ u"fo:color"_ustr ] = getColorString( rFont.isOutline ? rGC.LineColor : rGC.FillColor );

    StyleContainer::Style aStyle( "style:style"_ostr, std::move(aProps) );
    StyleContainer::Style aSubStyle( "style:text-properties"_ostr, std::move(aFontProps) );
    aStyle.SubStyles.push_back( &aSubStyle );
    elem.StyleId = m_rStyleContainer.getStyleId( aStyle );
}

void WriterXmlFinalizer::visit( ParagraphElement& elem, const std::list< std::unique_ptr<Element> >::const_iterator& rParentIt )
{
    PropertyMap aParaProps;

    if( elem.Parent )
    {
        // check for center alignment
        // criterion: paragraph is small relative to parent and distributed around its center
        double p_x = elem.Parent->x;
        double p_w = elem.Parent->w;

        PageElement* pPage = dynamic_cast<PageElement*>(elem.Parent);
        if( pPage )
        {
            p_x += pPage->LeftMargin;
            p_w -= pPage->LeftMargin+pPage->RightMargin;
        }
        bool bIsCenter = false;
        if( elem.w < ( p_w/2) )
        {
            double delta = elem.w/4;
            // allow very small paragraphs to deviate a little more
            // relative to parent's center
            if( elem.w <  p_w/8 )
                delta = elem.w;
            if( fabs( elem.x+elem.w/2 - ( p_x+ p_w/2) ) <  delta ||
                (pPage && fabs( elem.x+elem.w/2 - (pPage->x + pPage->w/2) ) <  delta) )
            {
                bIsCenter = true;
                aParaProps[ u"fo:text-align"_ustr ] = "center";
            }
        }
        if( ! bIsCenter && elem.x > p_x + p_w/10 )
        {
            // indent
            aParaProps[ u"fo:margin-left"_ustr ] = OUString::number(convPx2mm( elem.x - p_x )) + "mm";
        }

        // check whether to leave some space to next paragraph
        // find whether there is a next paragraph
        auto it = rParentIt;
        const ParagraphElement* pNextPara = nullptr;
        while( ++it != elem.Parent->Children.end() && ! pNextPara )
            pNextPara = dynamic_cast< const ParagraphElement* >(it->get());
        if( pNextPara )
        {
            if( pNextPara->y - (elem.y+elem.h) > convmm2Px( 10 ) )
            {
                aParaProps[ u"fo:margin-bottom"_ustr ] =
                    OUString::number( convPx2mm( pNextPara->y - (elem.y+elem.h) ) ) + "mm";
            }
        }
    }

    if( ! aParaProps.empty() )
    {
        PropertyMap aProps;
        aProps[ u"style:family"_ustr ] = "paragraph";
        StyleContainer::Style aStyle( "style:style"_ostr, std::move(aProps) );
        StyleContainer::Style aSubStyle( "style:paragraph-properties"_ostr, std::move(aParaProps) );
        aStyle.SubStyles.push_back( &aSubStyle );
        elem.StyleId = m_rStyleContainer.getStyleId( aStyle );
    }

    elem.applyToChildren(*this);
}

void WriterXmlFinalizer::visit( FrameElement& elem, const std::list< std::unique_ptr<Element> >::const_iterator&)
{
    PropertyMap aProps;
    aProps[ u"style:family"_ustr ] = "graphic";

    PropertyMap aGCProps;

    aGCProps[ u"draw:stroke"_ustr ]                    = "none";
    aGCProps[ u"draw:fill"_ustr ]                      = "none";
    aGCProps[ u"draw:auto-grow-height"_ustr ]          = "true";
    aGCProps[ u"draw:auto-grow-width"_ustr ]           = "true";
    aGCProps[ u"draw:textarea-horizontal-align"_ustr ] = "left";
    aGCProps[ u"draw:textarea-vertical-align"_ustr ]   = "top";
    aGCProps[ u"fo:min-height"_ustr]                   = "0cm";
    aGCProps[ u"fo:min-width"_ustr]                    = "0cm";
    aGCProps[ u"fo:padding-top"_ustr ]                 = "0cm";
    aGCProps[ u"fo:padding-left"_ustr ]                = "0cm";
    aGCProps[ u"fo:padding-right"_ustr ]               = "0cm";
    aGCProps[ u"fo:padding-bottom"_ustr ]              = "0cm";

    StyleContainer::Style aStyle( "style:style"_ostr, std::move(aProps) );
    StyleContainer::Style aSubStyle( "style:graphic-properties"_ostr, std::move(aGCProps) );
    aStyle.SubStyles.push_back( &aSubStyle );

    elem.StyleId = m_rStyleContainer.getStyleId( aStyle );
    elem.applyToChildren(*this);
}

void WriterXmlFinalizer::visit( ImageElement&, const std::list< std::unique_ptr<Element> >::const_iterator& )
{
}

void WriterXmlFinalizer::setFirstOnPage( ParagraphElement&    rElem,
                                         StyleContainer&      rStyles,
                                         const OUString& rMasterPageName )
{
    PropertyMap aProps;
    if( rElem.StyleId != -1 )
    {
        const PropertyMap* pProps = rStyles.getProperties( rElem.StyleId );
        if( pProps )
            aProps = *pProps;
    }

    aProps[ u"style:family"_ustr ] = "paragraph";
    aProps[ u"style:master-page-name"_ustr ] = rMasterPageName;

    if( rElem.StyleId != -1 )
        rElem.StyleId = rStyles.setProperties( rElem.StyleId, std::move(aProps) );
    else
    {
        StyleContainer::Style aStyle( "style:style"_ostr, std::move(aProps) );
        rElem.StyleId = rStyles.getStyleId( aStyle );
    }
}

void WriterXmlFinalizer::visit( PageElement& elem, const std::list< std::unique_ptr<Element> >::const_iterator& )
{
    if( m_rProcessor.getStatusIndicator().is() )
        m_rProcessor.getStatusIndicator()->setValue( elem.PageNumber );

    // transform from pixel to mm
    double page_width = convPx2mm( elem.w ), page_height = convPx2mm( elem.h );

    // calculate page margins out of the relevant children (paragraphs)
    elem.TopMargin = elem.h;
    elem.BottomMargin = 0;
    elem.LeftMargin = elem.w;
    elem.RightMargin = 0;
    // first element should be a paragraph
    ParagraphElement* pFirstPara = nullptr;
    for( const auto& rxChild : elem.Children )
    {
        if( dynamic_cast<ParagraphElement*>( rxChild.get() ) )
        {
            if( rxChild->x < elem.LeftMargin )
                elem.LeftMargin = rxChild->x;
            if( rxChild->y < elem.TopMargin )
                elem.TopMargin = rxChild->y;
            if( rxChild->x + rxChild->w > elem.w - elem.RightMargin )
                elem.RightMargin = elem.w - (rxChild->x + rxChild->w);
            if( rxChild->y + rxChild->h > elem.h - elem.BottomMargin )
                elem.BottomMargin = elem.h - (rxChild->y + rxChild->h);
            if( ! pFirstPara )
                pFirstPara = dynamic_cast<ParagraphElement*>( rxChild.get() );
        }
    }
    if( elem.HeaderElement && elem.HeaderElement->y < elem.TopMargin )
        elem.TopMargin = elem.HeaderElement->y;
    if( elem.FooterElement && elem.FooterElement->y+elem.FooterElement->h > elem.h - elem.BottomMargin )
        elem.BottomMargin = elem.h - (elem.FooterElement->y + elem.FooterElement->h);

    // transform margins to mm
    double left_margin     = convPx2mm( elem.LeftMargin );
    double right_margin    = convPx2mm( elem.RightMargin );
    double top_margin      = convPx2mm( elem.TopMargin );
    double bottom_margin   = convPx2mm( elem.BottomMargin );
    if( ! pFirstPara )
    {
        // use default page margins
        left_margin     = 10;
        right_margin    = 10;
        top_margin      = 10;
        bottom_margin   = 10;
    }

    // round left/top margin to nearest mm
    left_margin     = rtl_math_round( left_margin, 0, rtl_math_RoundingMode_Floor );
    top_margin      = rtl_math_round( top_margin, 0, rtl_math_RoundingMode_Floor );
    // round (fuzzy) right/bottom margin to nearest cm
    right_margin    = rtl_math_round( right_margin, right_margin >= 10 ? -1 : 0, rtl_math_RoundingMode_Floor );
    bottom_margin   = rtl_math_round( bottom_margin, bottom_margin >= 10 ? -1 : 0, rtl_math_RoundingMode_Floor );

    // set reasonable default in case of way too large margins
    // e.g. no paragraph case
    if( left_margin > page_width/2.0 - 10 )
        left_margin = 10;
    if( right_margin > page_width/2.0 - 10 )
        right_margin = 10;
    if( top_margin > page_height/2.0 - 10 )
        top_margin = 10;
    if( bottom_margin > page_height/2.0 - 10 )
        bottom_margin = 10;

    // catch the weird cases
    if( left_margin < 0 )
        left_margin = 0;
    if( right_margin < 0 )
        right_margin = 0;
    if( top_margin < 0 )
        top_margin = 0;
    if( bottom_margin < 0 )
        bottom_margin = 0;

    // widely differing margins are unlikely to be correct
    if( right_margin > left_margin*1.5 )
        right_margin = left_margin;

    elem.LeftMargin      = convmm2Px( left_margin );
    elem.RightMargin     = convmm2Px( right_margin );
    elem.TopMargin       = convmm2Px( top_margin );
    elem.BottomMargin    = convmm2Px( bottom_margin );

    // get styles for paragraphs
    PropertyMap aPageProps;
    PropertyMap aPageLayoutProps;
    aPageLayoutProps[ u"fo:page-width"_ustr ]     = unitMMString( page_width );
    aPageLayoutProps[ u"fo:page-height"_ustr ]    = unitMMString( page_height );
    aPageLayoutProps[ u"style:print-orientation"_ustr ]
        = elem.w < elem.h ? std::u16string_view(u"portrait") : std::u16string_view(u"landscape");
    aPageLayoutProps[ u"fo:margin-top"_ustr ]     = unitMMString( top_margin );
    aPageLayoutProps[ u"fo:margin-bottom"_ustr ]  = unitMMString( bottom_margin );
    aPageLayoutProps[ u"fo:margin-left"_ustr ]    = unitMMString( left_margin );
    aPageLayoutProps[ u"fo:margin-right"_ustr ]   = unitMMString( right_margin );
    aPageLayoutProps[ u"style:writing-mode"_ustr ]= "lr-tb";

    StyleContainer::Style aStyle( "style:page-layout"_ostr, std::move(aPageProps));
    StyleContainer::Style aSubStyle( "style:page-layout-properties"_ostr, std::move(aPageLayoutProps));
    aStyle.SubStyles.push_back(&aSubStyle);
    sal_Int32 nPageStyle = m_rStyleContainer.impl_getStyleId( aStyle, false );

    // create master page
    OUString aMasterPageLayoutName = m_rStyleContainer.getStyleName( nPageStyle );
    aPageProps[ u"style:page-layout-name"_ustr ] = aMasterPageLayoutName;
    StyleContainer::Style aMPStyle( "style:master-page"_ostr, std::move(aPageProps) );
    StyleContainer::Style aHeaderStyle( "style:header"_ostr, PropertyMap() );
    StyleContainer::Style aFooterStyle( "style:footer"_ostr, PropertyMap() );
    if( elem.HeaderElement )
    {
        elem.HeaderElement->visitedBy( *this, std::list<std::unique_ptr<Element>>::iterator() );
        aHeaderStyle.ContainedElement = elem.HeaderElement.get();
        aMPStyle.SubStyles.push_back( &aHeaderStyle );
    }
    if( elem.FooterElement )
    {
        elem.FooterElement->visitedBy( *this, std::list<std::unique_ptr<Element>>::iterator() );
        aFooterStyle.ContainedElement = elem.FooterElement.get();
        aMPStyle.SubStyles.push_back( &aFooterStyle );
    }
    elem.StyleId = m_rStyleContainer.impl_getStyleId( aMPStyle,false );


    OUString aMasterPageName = m_rStyleContainer.getStyleName( elem.StyleId );

    // create styles for children
    elem.applyToChildren(*this);

    // no paragraph or other elements before the first paragraph
    if( ! pFirstPara )
    {
        pFirstPara = ElementFactory::createParagraphElement( nullptr );
        pFirstPara->Parent = &elem;
        elem.Children.push_front( std::unique_ptr<Element>(pFirstPara) );
    }
    setFirstOnPage(*pFirstPara, m_rStyleContainer, aMasterPageName);
}

void WriterXmlFinalizer::visit( DocumentElement& elem, const std::list< std::unique_ptr<Element> >::const_iterator& )
{
    elem.applyToChildren(*this);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
