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


#include "pdfiprocessor.hxx"
#include "xmlemitter.hxx"
#include "pdfihelper.hxx"
#include "imagecontainer.hxx"
#include "style.hxx"
#include "writertreevisiting.hxx"
#include "genericelements.hxx"

#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/range/b2drange.hxx>


namespace pdfi
{

void WriterXmlEmitter::visit( HyperlinkElement& elem, const std::list< Element* >::const_iterator&   )
{
    if( elem.Children.empty() )
        return;

    const char* pType = dynamic_cast<DrawElement*>(elem.Children.front()) ? "draw:a" : "text:a";

    PropertyMap aProps;
    aProps[ "xlink:type" ] = "simple";
    aProps[ "xlink:href" ] = elem.URI;
    aProps[ "office:target-frame-name" ] = "_blank";
    aProps[ "xlink:show" ] = "new";

    m_rEmitContext.rEmitter.beginTag( pType, aProps );
    std::list< Element* >::iterator this_it =  elem.Children.begin();
    while( this_it !=elem.Children.end() && *this_it != &elem )
    {
        (*this_it)->visitedBy( *this, this_it );
        ++this_it;
    }
    m_rEmitContext.rEmitter.endTag( pType );
}

void WriterXmlEmitter::visit( TextElement& elem, const std::list< Element* >::const_iterator&   )
{
    if( elem.Text.isEmpty() )
        return;

    PropertyMap aProps;
    if( elem.StyleId != -1 )
    {
        aProps[ OUString( "text:style-name" ) ] =
            m_rEmitContext.rStyles.getStyleName( elem.StyleId );
    }

    m_rEmitContext.rEmitter.beginTag( "text:span", aProps );
    m_rEmitContext.rEmitter.write( elem.Text.makeStringAndClear() );
    std::list< Element* >::iterator this_it =  elem.Children.begin();
    while( this_it !=elem.Children.end() && *this_it != &elem )
    {
        (*this_it)->visitedBy( *this, this_it );
        ++this_it;
    }

    m_rEmitContext.rEmitter.endTag( "text:span" );
}

void WriterXmlEmitter::visit( ParagraphElement& elem, const std::list< Element* >::const_iterator&   )
{
    PropertyMap aProps;
    if( elem.StyleId != -1 )
    {
        aProps[ "text:style-name" ] = m_rEmitContext.rStyles.getStyleName( elem.StyleId );
    }
    const char* pTagType = "text:p";
    if( elem.Type == elem.Headline )
        pTagType = "text:h";
    m_rEmitContext.rEmitter.beginTag( pTagType, aProps );

    std::list< Element* >::iterator this_it =  elem.Children.begin();
    while( this_it !=elem.Children.end() && *this_it != &elem )
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
    Element* pAnchor = rElem.Parent;
    while( pAnchor &&
           ! dynamic_cast<ParagraphElement*>(pAnchor) &&
           ! dynamic_cast<PageElement*>(pAnchor) )
    {
        pAnchor = pAnchor->Parent;
    }
    if( pAnchor )
    {
        if( dynamic_cast<ParagraphElement*>(pAnchor) )
        {
            rProps[ "text:anchor-type" ] = rElem.isCharacter
                ? OUString("character") : OUString("paragraph");
        }
        else
        {
            PageElement* pPage = dynamic_cast<PageElement*>(pAnchor);
            rProps[ "text:anchor-type" ] = "page";
            rProps[ "text:anchor-page-number" ] = OUString::number(pPage->PageNumber);
        }
        rel_x -= pAnchor->x;
        rel_y -= pAnchor->y;
    }

    rProps[ "draw:z-index" ] = OUString::number( rElem.ZOrder );
    rProps[ "draw:style-name"] = rEmitContext.rStyles.getStyleName( rElem.StyleId );
    rProps[ "svg:width" ]   = convertPixelToUnitString( rElem.w );
    rProps[ "svg:height" ]  = convertPixelToUnitString( rElem.h );

    const GraphicsContext& rGC =
        rEmitContext.rProcessor.getGraphicsContext( rElem.GCId );
    if( rGC.Transformation.isIdentity() )
    {
        if( !rElem.isCharacter )
        {
            rProps[ "svg:x" ]       = convertPixelToUnitString( rel_x );
            rProps[ "svg:y" ]       = convertPixelToUnitString( rel_y );
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
        if( fShearX != 0.0 )
        {
            aBuf.appendAscii( "skewX( " );
            aBuf.append( fShearX );
            aBuf.appendAscii( " )" );
        }
        if( fRotate != 0.0 )
        {
            if( !aBuf.isEmpty() )
                aBuf.append( sal_Unicode(' ') );
            aBuf.appendAscii( "rotate( " );
            aBuf.append( -fRotate );
            aBuf.appendAscii( " )" );

        }
        if( ! rElem.isCharacter )
        {
            if( !aBuf.isEmpty() )
                aBuf.append( sal_Unicode(' ') );
            aBuf.appendAscii( "translate( " );
            aBuf.append( convertPixelToUnitString( rel_x ) );
            aBuf.append( sal_Unicode(' ') );
            aBuf.append( convertPixelToUnitString( rel_y ) );
            aBuf.appendAscii( " )" );
         }

        rProps[ "draw:transform" ] = aBuf.makeStringAndClear();
    }
}

void WriterXmlEmitter::visit( FrameElement& elem, const std::list< Element* >::const_iterator&   )
{
    if( elem.Children.empty() )
        return;

    bool bTextBox = (dynamic_cast<ParagraphElement*>(elem.Children.front()) != NULL);
    PropertyMap aFrameProps;
    fillFrameProps( elem, aFrameProps, m_rEmitContext );
    m_rEmitContext.rEmitter.beginTag( "draw:frame", aFrameProps );
    if( bTextBox )
        m_rEmitContext.rEmitter.beginTag( "draw:text-box", PropertyMap() );

    std::list< Element* >::iterator this_it =  elem.Children.begin();
    while( this_it !=elem.Children.end() && *this_it != &elem )
    {
        (*this_it)->visitedBy( *this, this_it );
        ++this_it;
    }

    if( bTextBox )
        m_rEmitContext.rEmitter.endTag( "draw:text-box" );
    m_rEmitContext.rEmitter.endTag( "draw:frame" );
}

void WriterXmlEmitter::visit( PolyPolyElement& elem, const std::list< Element* >::const_iterator& )
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
        basegfx::B2DPolygon b2dPolygon;
        b2dPolygon =  elem.PolyPoly.getB2DPolygon( i );

        for ( sal_uInt32 j = 0; j< b2dPolygon.count(); j++ )
        {
            basegfx::B2DPoint point;
            basegfx::B2DPoint nextPoint;
            point = b2dPolygon.getB2DPoint( j );

            basegfx::B2DPoint prevPoint;
            prevPoint = b2dPolygon.getPrevControlPoint( j ) ;

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
    OUStringBuffer aBuf( 64 );
    aBuf.appendAscii( "0 0 " );
    aBuf.append( convPx2mmPrec2(elem.w)*100.0 );
    aBuf.append( sal_Unicode(' ') );
    aBuf.append( convPx2mmPrec2(elem.h)*100.0 );
    aProps[ "svg:viewBox" ] = aBuf.makeStringAndClear();
    aProps[ "svg:d" ]       = basegfx::tools::exportToSvgD( elem.PolyPoly );

    m_rEmitContext.rEmitter.beginTag( "draw:path", aProps );
    m_rEmitContext.rEmitter.endTag( "draw:path" );
}

void WriterXmlEmitter::visit( ImageElement& elem, const std::list< Element* >::const_iterator& )
{
    PropertyMap aImageProps;
    m_rEmitContext.rEmitter.beginTag( "draw:image", aImageProps );
    m_rEmitContext.rEmitter.beginTag( "office:binary-data", PropertyMap() );
    m_rEmitContext.rImages.writeBase64EncodedStream( elem.Image, m_rEmitContext);
    m_rEmitContext.rEmitter.endTag( "office:binary-data" );
    m_rEmitContext.rEmitter.endTag( "draw:image" );
}

void WriterXmlEmitter::visit( PageElement& elem, const std::list< Element* >::const_iterator&   )
{
    if( m_rEmitContext.xStatusIndicator.is() )
        m_rEmitContext.xStatusIndicator->setValue( elem.PageNumber );

    std::list< Element* >::iterator this_it =  elem.Children.begin();
    while( this_it !=elem.Children.end() && *this_it != &elem )
    {
        (*this_it)->visitedBy( *this, this_it );
        ++this_it;
    }
}

void WriterXmlEmitter::visit( DocumentElement& elem, const std::list< Element* >::const_iterator&)
{
    m_rEmitContext.rEmitter.beginTag( "office:body", PropertyMap() );
    m_rEmitContext.rEmitter.beginTag( "office:text", PropertyMap() );

    for( std::list< Element* >::iterator it = elem.Children.begin(); it != elem.Children.end(); ++it )
    {
        PageElement* pPage = dynamic_cast<PageElement*>(*it);
        if( pPage )
        {
            // emit only page anchored objects
            // currently these are only DrawElement types
            for( std::list< Element* >::iterator child_it = pPage->Children.begin(); child_it != pPage->Children.end(); ++child_it )
            {
                if( dynamic_cast<DrawElement*>(*child_it) != NULL )
                    (*child_it)->visitedBy( *this, child_it );
            }
        }
    }

    // do not emit page anchored objects, they are emitted before
    // (must precede all pages in writer document) currently these are
    // only DrawElement types
    for( std::list< Element* >::iterator it = elem.Children.begin(); it != elem.Children.end(); ++it )
    {
        if( dynamic_cast<DrawElement*>(*it) == NULL )
            (*it)->visitedBy( *this, it );
    }

    m_rEmitContext.rEmitter.endTag( "office:text" );
    m_rEmitContext.rEmitter.endTag( "office:body" );
}

/////////////////////////////////////////////////////////////////

void WriterXmlOptimizer::visit( HyperlinkElement&, const std::list< Element* >::const_iterator& )
{
}

void WriterXmlOptimizer::visit( TextElement&, const std::list< Element* >::const_iterator&)
{
}

void WriterXmlOptimizer::visit( FrameElement& elem, const std::list< Element* >::const_iterator& )
{
    elem.applyToChildren(*this);
}

void WriterXmlOptimizer::visit( ImageElement&, const std::list< Element* >::const_iterator& )
{
}

void WriterXmlOptimizer::visit( PolyPolyElement& elem, const std::list< Element* >::const_iterator& )
{
    /* note: optimize two consecutive PolyPolyElements that
     *  have the same path but one of which is a stroke while
     *     the other is a fill
     */
    if( elem.Parent )
    {
        // find following PolyPolyElement in parent's children list
        std::list< Element* >::iterator this_it = elem.Parent->Children.begin();
        while( this_it != elem.Parent->Children.end() && *this_it != &elem )
            ++this_it;

        if( this_it != elem.Parent->Children.end() )
        {
            std::list< Element* >::iterator next_it = this_it;
            if( ++next_it != elem.Parent->Children.end() )
            {
                PolyPolyElement* pNext = dynamic_cast<PolyPolyElement*>(*next_it);
                if( pNext && pNext->PolyPoly == elem.PolyPoly )
                {
                    const GraphicsContext& rNextGC =
                        m_rProcessor.getGraphicsContext( pNext->GCId );
                    const GraphicsContext& rThisGC =
                        m_rProcessor.getGraphicsContext( elem.GCId );

                    if( rThisGC.BlendMode      == rNextGC.BlendMode &&
                        rThisGC.Flatness       == rNextGC.Flatness &&
                        rThisGC.Transformation == rNextGC.Transformation &&
                        rThisGC.Clip           == rNextGC.Clip &&
                        pNext->Action          == PATH_STROKE &&
                        (elem.Action == PATH_FILL || elem.Action == PATH_EOFILL) )
                    {
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
                        elem.Parent->Children.erase( next_it );
                        delete pNext;
                    }
                }
            }
        }
    }
}

void WriterXmlOptimizer::visit( ParagraphElement& elem, const std::list< Element* >::const_iterator& rParentIt)
{
    optimizeTextElements( elem );

    elem.applyToChildren(*this);

    if( elem.Parent && rParentIt != elem.Parent->Children.end() )
    {
        // find if there is a previous paragraph that might be a heading for this one
        std::list<Element*>::const_iterator prev = rParentIt;
        ParagraphElement* pPrevPara = NULL;
        while( prev != elem.Parent->Children.begin() )
        {
            --prev;
            pPrevPara = dynamic_cast< ParagraphElement* >(*prev);
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
                            pPrevPara->Type = elem.Headline;
                        }
                        else
                        {
                            // check whether text of pPrevPara is bold (at least first text element)
                            // and this para is not bold (dito)
                            TextElement* pPrevText = pPrevPara->getFirstTextChild();
                            TextElement* pThisText = elem.getFirstTextChild();
                            if( pPrevText && pThisText )
                            {
                                const FontAttributes& rPrevFont = m_rProcessor.getFont( pPrevText->FontId );
                                const FontAttributes& rThisFont = m_rProcessor.getFont( pThisText->FontId );
                                if( rPrevFont.isBold && ! rThisFont.isBold )
                                    pPrevPara->Type = elem.Headline;
                            }
                        }
                    }
                }
                break;
            }
        }
    }
}

void WriterXmlOptimizer::visit( PageElement& elem, const std::list< Element* >::const_iterator& )
{
    if( m_rProcessor.getStatusIndicator().is() )
        m_rProcessor.getStatusIndicator()->setValue( elem.PageNumber );

    // resolve hyperlinks
    elem.resolveHyperlinks();

    elem.resolveFontStyles( m_rProcessor ); // underlines and such

    // FIXME: until hyperlinks and font effects are adjusted for
    // geometrical search handle them before sorting
    m_rProcessor.sortElements( &elem );

    // find paragraphs in text
    ParagraphElement* pCurPara = NULL;
    std::list< Element* >::iterator page_element, next_page_element;
    next_page_element = elem.Children.begin();
    double fCurLineHeight = 0.0; // average height of text items in current para
    int nCurLineElements = 0; // number of line contributing elements in current para
    double line_left = elem.w, line_right = 0.0;
    double column_width = elem.w*0.75; // estimate text width
    // TODO: guess columns
    while( next_page_element != elem.Children.end() )
    {
        page_element = next_page_element++;
        ParagraphElement* pPagePara = dynamic_cast<ParagraphElement*>(*page_element);
        if( pPagePara )
        {
            pCurPara = pPagePara;
            // adjust line height and text items
            fCurLineHeight = 0.0;
            nCurLineElements = 0;
            for( std::list< Element* >::iterator it = pCurPara->Children.begin();
                 it != pCurPara->Children.end(); ++it )
            {
                TextElement* pTestText = dynamic_cast<TextElement*>(*it);
                if( pTestText )
                {
                    fCurLineHeight = (fCurLineHeight*double(nCurLineElements) + pTestText->h)/double(nCurLineElements+1);
                    nCurLineElements++;
                }
            }
            continue;
        }

        HyperlinkElement* pLink = dynamic_cast<HyperlinkElement*>(*page_element);
        DrawElement* pDraw = dynamic_cast<DrawElement*>(*page_element);
        if( ! pDraw && pLink && ! pLink->Children.empty() )
            pDraw = dynamic_cast<DrawElement*>(pLink->Children.front() );
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
                TextElement* pText = dynamic_cast<TextElement*>(*next_page_element);
                if( ! pText )
                {
                    ParagraphElement* pPara = dynamic_cast<ParagraphElement*>(*next_page_element);
                    if( pPara && ! pPara->Children.empty() )
                        pText = dynamic_cast<TextElement*>(pPara->Children.front());
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
                    pCurPara = NULL;
                    // mark draw element as character
                    pDraw->isCharacter = true;
                }
            }

            if( ! bInsertToParagraph )
            {
                pCurPara = NULL;
                continue;
            }
        }

        TextElement* pText = dynamic_cast<TextElement*>(*page_element);
        if( ! pText && pLink && ! pLink->Children.empty() )
            pText = dynamic_cast<TextElement*>(pLink->Children.front());
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
                        pCurPara = NULL; // insert new paragraph
                    else if( pGeo->y > (pCurPara->y+pCurPara->h - fCurLineHeight*0.05) )
                    {
                        // new paragraph if either the last line of the paragraph
                        // was significantly shorter than the paragraph as a whole
                        if( (line_right - line_left) < pCurPara->w*0.75 )
                            pCurPara = NULL;
                        // or the last line was significantly smaller than the column width
                        else if( (line_right - line_left) < column_width*0.75 )
                            pCurPara = NULL;
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
            pCurPara = m_rProcessor.getElementFactory()->createParagraphElement( NULL );
            // set parent
            pCurPara->Parent = &elem;
            //insert new paragraph before current element
            page_element = elem.Children.insert( page_element, pCurPara );
            // forward iterator to current element again
            ++ page_element;
            // update next_element which is now invalid
            next_page_element = page_element;
            ++ next_page_element;
        }
        Element* pCurEle = *page_element;
        pCurEle->setParent( page_element, pCurPara );
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
     *  - single line paragrah at top of page (  inside 15% page height)
     *  - at least linheight above the next paragr   aph
     *
     *  indicators for a footer likewise:
     *  - single line paragraph at bottom of page (inside 15% page height)
     *  - at least lineheight below the previous paragraph
     */

    // detect header
    // Note: the following assumes that the pages' chiuldren have been
    // sorted geometrically
    std::list< Element* >::iterator it = rElem.Children.begin();
    while( it != rElem.Children.end() )
    {
        ParagraphElement* pPara = dynamic_cast<ParagraphElement*>(*it);
        if( pPara )
        {
            if( pPara->y+pPara->h < rElem.h*0.15 && pPara->isSingleLined( m_rProcessor ) )
            {
                std::list< Element* >::iterator next_it = it;
                ParagraphElement* pNextPara = NULL;
                while( ++next_it != rElem.Children.end() && pNextPara == NULL )
                {
                    pNextPara = dynamic_cast<ParagraphElement*>(*next_it);
                }
                if( pNextPara && pNextPara->y > pPara->y+pPara->h*2 )
                {
                    rElem.HeaderElement = pPara;
                    pPara->Parent = NULL;
                    rElem.Children.remove( pPara );
                }
            }
            break;
        }
        ++it;
    }

    // detect footer
    std::list< Element* >::reverse_iterator rit = rElem.Children.rbegin();
    while( rit != rElem.Children.rend() )
    {
        ParagraphElement* pPara = dynamic_cast<ParagraphElement*>(*rit);
        if( pPara )
        {
            if( pPara->y > rElem.h*0.85 && pPara->isSingleLined( m_rProcessor ) )
            {
                std::list< Element* >::reverse_iterator next_it = rit;
                ParagraphElement* pNextPara = NULL;
                while( ++next_it != rElem.Children.rend() && pNextPara == NULL )
                {
                    pNextPara = dynamic_cast<ParagraphElement*>(*next_it);
                }
                if( pNextPara && pNextPara->y < pPara->y-pPara->h*2 )
                {
                    rElem.FooterElement = pPara;
                    pPara->Parent = NULL;
                    rElem.Children.remove( pPara );
                }
            }
            break;
        }
        ++rit;
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
    std::list< Element* >::iterator next = rParent.Children.begin();
    std::list< Element* >::iterator it = next++;
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
        TextElement* pCur = dynamic_cast<TextElement*>(*it);
        if( pCur )
        {
            TextElement* pNext = dynamic_cast<TextElement*>(*next);
            if( pNext )
            {
                const GraphicsContext& rCurGC = m_rProcessor.getGraphicsContext( pCur->GCId );
                const GraphicsContext& rNextGC = m_rProcessor.getGraphicsContext( pNext->GCId );

                // line and space optimization; works only in strictly horizontal mode

                if( !bRotatedFrame
                    && ! rCurGC.isRotatedOrSkewed()
                    && ! rNextGC.isRotatedOrSkewed()
                    && ! pNext->Text.isEmpty()
                    && pNext->Text[0] != sal_Unicode(' ')
                    && ! pCur->Text.isEmpty()
                    && pCur->Text[pCur->Text.getLength() - 1] != sal_Unicode(' ')
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
                            pCur->Text.append( sal_Unicode( ' ' ) );
                        }
                    }
                    else // we're continuing the same line
                    {
                        // check whether a space would should be inserted
                        // check for a small horizontal offset
                        if( pCur->x + pCur->w + pNext->h*0.15 < pNext->x )
                        {
                            pCur->Text.append( sal_Unicode(' ') );
                        }
                    }
                }
                // concatenate consecutive text elements unless there is a
                // font or text color or matrix change, leave a new span in that case
                if( pCur->FontId == pNext->FontId &&
                    rCurGC.FillColor.Red == rNextGC.FillColor.Red &&
                    rCurGC.FillColor.Green == rNextGC.FillColor.Green &&
                    rCurGC.FillColor.Blue == rNextGC.FillColor.Blue &&
                    rCurGC.FillColor.Alpha == rNextGC.FillColor.Alpha &&
                    rCurGC.Transformation == rNextGC.Transformation
                    )
                {
                    pCur->updateGeometryWith( pNext );
                    // append text to current element
                    pCur->Text.append( pNext->Text.getStr(), pNext->Text.getLength() );
                    // append eventual children to current element
                    // and clear children (else the children just
                    // appended to pCur would be destroyed)
                    pCur->Children.splice( pCur->Children.end(), pNext->Children );
                    // get rid of the now useless element
                    rParent.Children.erase( next );
                    delete pNext;
                    bConcat = true;
                }
            }
        }
        else if( dynamic_cast<HyperlinkElement*>(*it) )
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

void WriterXmlOptimizer::visit( DocumentElement& elem, const std::list< Element* >::const_iterator&)
{
    elem.applyToChildren(*this);
}

//////////////////////////////////////////////////////////////////////////////////


void WriterXmlFinalizer::visit( PolyPolyElement& elem, const std::list< Element* >::const_iterator& )
{
    // xxx TODO copied from DrawElement
    const GraphicsContext& rGC = m_rProcessor.getGraphicsContext(elem.GCId );
    PropertyMap aProps;
    aProps[ "style:family" ] = "graphic";

    PropertyMap aGCProps;

    // TODO(F3): proper dash emulation
    if( elem.Action & PATH_STROKE )
    {
        aGCProps[ "draw:stroke" ] = rGC.DashArray.empty() ? OUString("solid") : OUString("dash");
        aGCProps[ "svg:stroke-color" ] = getColorString( rGC.LineColor );
        if( rGC.LineWidth != 0.0 )
        {
            ::basegfx::B2DVector aVec(rGC.LineWidth,0);
            aVec *= rGC.Transformation;

            aVec.setX ( convPx2mmPrec2( aVec.getX() )*100.0 );
            aVec.setY ( convPx2mmPrec2( aVec.getY() )*100.0 );

            aGCProps[ "svg:stroke-width" ] = OUString::number( aVec.getLength() );
        }
    }
    else
    {
        aGCProps[ "draw:stroke" ] = "none";
    }

    // TODO(F1): check whether stuff could be emulated by gradient/bitmap/hatch
    if( elem.Action & (PATH_FILL | PATH_EOFILL) )
    {
        aGCProps[ "draw:fill" ]   = "solid";
        aGCProps[ "draw:fill-color" ] = getColorString( rGC.FillColor );
    }
    else
    {
        aGCProps[ "draw:fill" ] = "none";
    }

    StyleContainer::Style aStyle( "style:style", aProps );
    StyleContainer::Style aSubStyle( "style:graphic-properties", aGCProps );
    aStyle.SubStyles.push_back( &aSubStyle );

    elem.StyleId = m_rStyleContainer.getStyleId( aStyle );
}

void WriterXmlFinalizer::visit( HyperlinkElement&, const std::list< Element* >::const_iterator& )
{
}

void WriterXmlFinalizer::visit( TextElement& elem, const std::list< Element* >::const_iterator& )
{
    const FontAttributes& rFont = m_rProcessor.getFont( elem.FontId );
    PropertyMap aProps;
    aProps[ "style:family" ] = "text";

    PropertyMap aFontProps;

    // family name
    aFontProps[ "fo:font-family" ] = rFont.familyName;
    // bold
    if( rFont.isBold )
    {
        aFontProps[ "fo:font-weight" ]         = "bold";
        aFontProps[ "fo:font-weight-asian" ]   = "bold";
        aFontProps[ "fo:font-weight-complex" ] = "bold";
    }
    // italic
    if( rFont.isItalic )
    {
        aFontProps[ "fo:font-style" ]         = "italic";
        aFontProps[ "fo:font-style-asian" ]   = "italic";
        aFontProps[ "fo:font-style-complex" ] = "italic";
    }
    // underline
    if( rFont.isUnderline )
    {
        aFontProps[ "style:text-underline-style" ]  = "solid";
        aFontProps[ "style:text-underline-width" ]  = "auto";
        aFontProps[ "style:text-underline-color" ]  = "font-color";
    }
    // outline
    if( rFont.isOutline )
    {
        aFontProps[ "style:text-outline" ]  = "true";
    }
    // size
    OUStringBuffer aBuf( 32 );
    aBuf.append( rFont.size*72/PDFI_OUTDEV_RESOLUTION );
    aBuf.appendAscii( "pt" );
    OUString aFSize = aBuf.makeStringAndClear();
    aFontProps[ "fo:font-size" ]            = aFSize;
    aFontProps[ "style:font-size-asian" ]   = aFSize;
    aFontProps[ "style:font-size-complex" ] = aFSize;
    // color
    const GraphicsContext& rGC = m_rProcessor.getGraphicsContext( elem.GCId );
    aFontProps[ "fo:color" ]                 =  getColorString( rFont.isOutline ? rGC.LineColor : rGC.FillColor );

    StyleContainer::Style aStyle( "style:style", aProps );
    StyleContainer::Style aSubStyle( "style:text-properties", aFontProps );
    aStyle.SubStyles.push_back( &aSubStyle );
    elem.StyleId = m_rStyleContainer.getStyleId( aStyle );
}

void WriterXmlFinalizer::visit( ParagraphElement& elem, const std::list< Element* >::const_iterator& rParentIt )
{
    PropertyMap aParaProps;

    if( elem.Parent )
    {
        // check for center alignement
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
                aParaProps[ "fo:text-align" ] = "center";
            }
        }
        if( ! bIsCenter && elem.x > p_x + p_w/10 )
        {
            // indent
            OUStringBuffer aBuf( 32 );
            aBuf.append( convPx2mm( elem.x - p_x ) );
            aBuf.appendAscii( "mm" );
            aParaProps[ "fo:margin-left" ] = aBuf.makeStringAndClear();
        }

        // check whether to leave some space to next paragraph
        // find whether there is a next paragraph
        std::list< Element* >::const_iterator it = rParentIt;
        const ParagraphElement* pNextPara = NULL;
        while( ++it != elem.Parent->Children.end() && ! pNextPara )
            pNextPara = dynamic_cast< const ParagraphElement* >(*it);
        if( pNextPara )
        {
            if( pNextPara->y - (elem.y+elem.h) > convmm2Px( 10 ) )
            {
                OUStringBuffer aBuf( 32 );
                aBuf.append( convPx2mm( pNextPara->y - (elem.y+elem.h) ) );
                aBuf.appendAscii( "mm" );
                aParaProps[ "fo:margin-bottom" ] = aBuf.makeStringAndClear();
            }
        }
    }

    if( ! aParaProps.empty() )
    {
        PropertyMap aProps;
        aProps[ "style:family" ] = "paragraph";
        StyleContainer::Style aStyle( "style:style", aProps );
        StyleContainer::Style aSubStyle( "style:paragraph-properties", aParaProps );
        aStyle.SubStyles.push_back( &aSubStyle );
        elem.StyleId = m_rStyleContainer.getStyleId( aStyle );
    }

    elem.applyToChildren(*this);
}

void WriterXmlFinalizer::visit( FrameElement& elem, const std::list< Element* >::const_iterator&)
{
    PropertyMap aProps;
    aProps[ "style:family" ] = "graphic";

    PropertyMap aGCProps;

    aGCProps[ "draw:stroke" ] = "none";
    aGCProps[ "draw:fill" ] = "none";

    StyleContainer::Style aStyle( "style:style", aProps );
    StyleContainer::Style aSubStyle( "style:graphic-properties", aGCProps );
    aStyle.SubStyles.push_back( &aSubStyle );

    elem.StyleId = m_rStyleContainer.getStyleId( aStyle );
    elem.applyToChildren(*this);
}

void WriterXmlFinalizer::visit( ImageElement&, const std::list< Element* >::const_iterator& )
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

    aProps[ "style:family" ] = "paragraph";
    aProps[ "style:master-page-name" ] = rMasterPageName;

    if( rElem.StyleId != -1 )
        rElem.StyleId = rStyles.setProperties( rElem.StyleId, aProps );
    else
    {
        StyleContainer::Style aStyle( "style:style", aProps );
        rElem.StyleId = rStyles.getStyleId( aStyle );
    }
}

void WriterXmlFinalizer::visit( PageElement& elem, const std::list< Element* >::const_iterator& )
{
    if( m_rProcessor.getStatusIndicator().is() )
        m_rProcessor.getStatusIndicator()->setValue( elem.PageNumber );

    // transform from pixel to mm
    double page_width = convPx2mm( elem.w ), page_height = convPx2mm( elem.h );

    // calculate page margins out of the relevant children (paragraphs)
    elem.TopMargin = elem.h, elem.BottomMargin = 0, elem.LeftMargin = elem.w, elem.RightMargin = 0;
    // first element should be a paragraphy
    ParagraphElement* pFirstPara = NULL;
    for( std::list< Element* >::const_iterator it = elem.Children.begin(); it != elem.Children.end(); ++it )
    {
        if( dynamic_cast<ParagraphElement*>( *it ) )
        {
            if( (*it)->x < elem.LeftMargin )
                elem.LeftMargin = (*it)->x;
            if( (*it)->y < elem.TopMargin )
                elem.TopMargin = (*it)->y;
            if( (*it)->x + (*it)->w > elem.w - elem.RightMargin )
                elem.RightMargin = elem.w - ((*it)->x + (*it)->w);
            if( (*it)->y + (*it)->h > elem.h - elem.BottomMargin )
                elem.BottomMargin = elem.h - ((*it)->y + (*it)->h);
            if( ! pFirstPara )
                pFirstPara = dynamic_cast<ParagraphElement*>( *it );
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
    aPageLayoutProps[ "fo:page-width" ]     = unitMMString( page_width );
    aPageLayoutProps[ "fo:page-height" ]    = unitMMString( page_height );
    aPageLayoutProps[ "style:print-orientation" ]
        = elem.w < elem.h ? OUString("portrait") : OUString("landscape");
    aPageLayoutProps[ "fo:margin-top" ]     = unitMMString( top_margin );
    aPageLayoutProps[ "fo:margin-bottom" ]  = unitMMString( bottom_margin );
    aPageLayoutProps[ "fo:margin-left" ]    = unitMMString( left_margin );
    aPageLayoutProps[ "fo:margin-right" ]   = unitMMString( right_margin );
    aPageLayoutProps[ "style:writing-mode" ]= "lr-tb";

    StyleContainer::Style aStyle( "style:page-layout", aPageProps);
    StyleContainer::Style aSubStyle( "style:page-layout-properties", aPageLayoutProps);
    aStyle.SubStyles.push_back(&aSubStyle);
    sal_Int32 nPageStyle = m_rStyleContainer.impl_getStyleId( aStyle, false );

    // create master page
    OUString aMasterPageLayoutName = m_rStyleContainer.getStyleName( nPageStyle );
    aPageProps[ "style:page-layout-name" ] = aMasterPageLayoutName;
    StyleContainer::Style aMPStyle( "style:master-page", aPageProps );
    StyleContainer::Style aHeaderStyle( "style:header", PropertyMap() );
    StyleContainer::Style aFooterStyle( "style:footer", PropertyMap() );
    if( elem.HeaderElement )
    {
        elem.HeaderElement->visitedBy( *this, std::list<Element*>::iterator() );
        aHeaderStyle.ContainedElement = elem.HeaderElement;
        aMPStyle.SubStyles.push_back( &aHeaderStyle );
    }
    if( elem.FooterElement )
    {
        elem.FooterElement->visitedBy( *this, std::list<Element*>::iterator() );
        aFooterStyle.ContainedElement = elem.FooterElement;
        aMPStyle.SubStyles.push_back( &aFooterStyle );
    }
    elem.StyleId = m_rStyleContainer.impl_getStyleId( aMPStyle,false );


    OUString aMasterPageName = m_rStyleContainer.getStyleName( elem.StyleId );

    // create styles for children
    elem.applyToChildren(*this);

    // no paragraph or other elements before the first paragraph
    if( ! pFirstPara )
    {
        pFirstPara = m_rProcessor.getElementFactory()->createParagraphElement( NULL );
        pFirstPara->Parent = &elem;
        elem.Children.push_front( pFirstPara );
    }
    setFirstOnPage(*pFirstPara, m_rStyleContainer, aMasterPageName);
}

void WriterXmlFinalizer::visit( DocumentElement& elem, const std::list< Element* >::const_iterator& )
{
    elem.applyToChildren(*this);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
