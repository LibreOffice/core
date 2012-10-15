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
#include "drawtreevisiting.hxx"
#include "genericelements.hxx"

#include "basegfx/polygon/b2dpolypolygontools.hxx"
#include "basegfx/range/b2drange.hxx"

#include "com/sun/star/i18n/BreakIterator.hpp"
#include "com/sun/star/lang/XMultiServiceFactory.hpp"
#include "comphelper/processfactory.hxx"
#include "com/sun/star/i18n/ScriptType.hpp"
#include "com/sun/star/i18n/DirectionProperty.hpp"

#include <string.h>

using namespace ::com::sun::star;
using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star::uno;

namespace pdfi
{

const Reference< XBreakIterator >& DrawXmlOptimizer::GetBreakIterator()
{
    if ( !mxBreakIter.is() )
    {
        Reference< XComponentContext > xContext( this->m_rProcessor.m_xContext, uno::UNO_SET_THROW );
        mxBreakIter = BreakIterator::create(xContext);
    }
    return mxBreakIter;
}

const ::com::sun::star::uno::Reference< ::com::sun::star::i18n::XCharacterClassification >& DrawXmlEmitter::GetCharacterClassification()
{
    if ( !mxCharClass.is() )
    {
        Reference< XComponentContext > xContext( m_rEmitContext.m_xContext, uno::UNO_SET_THROW );
        Reference< XMultiComponentFactory > xMSF(  xContext->getServiceManager(), uno::UNO_SET_THROW );
    Reference < XInterface > xInterface = xMSF->createInstanceWithContext(::rtl::OUString("com.sun.star.i18n.CharacterClassification"), xContext);
        mxCharClass = uno::Reference< i18n::XCharacterClassification >( xInterface, uno::UNO_QUERY );
    }
    return mxCharClass;
}

void DrawXmlEmitter::visit( HyperlinkElement& elem, const std::list< Element* >::const_iterator&   )
{
    if( elem.Children.empty() )
        return;

    const char* pType = dynamic_cast<DrawElement*>(elem.Children.front()) ? "draw:a" : "text:a";

    PropertyMap aProps;
    aProps[ USTR( "xlink:type" ) ] = USTR( "simple" );
    aProps[ USTR( "xlink:href" ) ] = elem.URI;
    aProps[ USTR( "office:target-frame-name" ) ] = USTR( "_blank" );
    aProps[ USTR( "xlink:show" ) ] = USTR( "new" );

    m_rEmitContext.rEmitter.beginTag( pType, aProps );
    std::list< Element* >::iterator this_it =  elem.Children.begin();
    while( this_it !=elem.Children.end() && *this_it != &elem )
    {
        (*this_it)->visitedBy( *this, this_it );
        ++this_it;
    }
    m_rEmitContext.rEmitter.endTag( pType );
}

void DrawXmlEmitter::visit( TextElement& elem, const std::list< Element* >::const_iterator&   )
{
    if( ! elem.Text.getLength() )
        return;

    rtl::OUString strSpace(32);
    rtl::OUString strNbSpace(160);
    rtl::OUString tabSpace(0x09);
    PropertyMap aProps;
    if( elem.StyleId != -1 )
    {
        aProps[ rtl::OUString( "text:style-name"  ) ] =
            m_rEmitContext.rStyles.getStyleName( elem.StyleId );
    }

    rtl::OUString str(elem.Text.getStr());

    // Check for RTL
    bool isRTL = false;
    Reference< i18n::XCharacterClassification > xCC( GetCharacterClassification() );
    if( xCC.is() )
    {
        for(int i=1; i< elem.Text.getLength(); i++)
        {
            sal_Int16 nType = xCC->getCharacterDirection( str, i );
            if ( nType == ::com::sun::star::i18n::DirectionProperty_RIGHT_TO_LEFT           ||
                 nType == ::com::sun::star::i18n::DirectionProperty_RIGHT_TO_LEFT_ARABIC    ||
                 nType == ::com::sun::star::i18n::DirectionProperty_RIGHT_TO_LEFT_EMBEDDING ||
                 nType == ::com::sun::star::i18n::DirectionProperty_RIGHT_TO_LEFT_OVERRIDE
                )
                isRTL = true;
        }
    }

    if (isRTL)  // If so, reverse string
        str = m_rProcessor.mirrorString( str );

    m_rEmitContext.rEmitter.beginTag( "text:span", aProps );

    for(int i=0; i< elem.Text.getLength(); i++)
    {
        rtl::OUString strToken=  str.copy(i,1) ;
        if( strSpace.equals(strToken) || strNbSpace.equals(strToken))
        {
            aProps[ USTR( "text:c" ) ] = USTR( "1" );
            m_rEmitContext.rEmitter.beginTag( "text:s", aProps );
            m_rEmitContext.rEmitter.endTag( "text:s");
        }
        else
        {
            if( tabSpace.equals(strToken) )
            {
                m_rEmitContext.rEmitter.beginTag( "text:tab", aProps );
                m_rEmitContext.rEmitter.endTag( "text:tab");
            }
            else
            {
                m_rEmitContext.rEmitter.write( strToken );
            }
        }
    }

    std::list< Element* >::iterator this_it =  elem.Children.begin();
    while( this_it !=elem.Children.end() && *this_it != &elem )
    {
        (*this_it)->visitedBy( *this, this_it );
        ++this_it;
    }

    m_rEmitContext.rEmitter.endTag( "text:span" );
}

void DrawXmlEmitter::visit( ParagraphElement& elem, const std::list< Element* >::const_iterator&   )
{
    PropertyMap aProps;
    if( elem.StyleId != -1 )
    {
        aProps[ USTR( "text:style-name" ) ] = m_rEmitContext.rStyles.getStyleName( elem.StyleId );
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

void DrawXmlEmitter::fillFrameProps( DrawElement&       rElem,
                                     PropertyMap&       rProps,
                                     const EmitContext& rEmitContext,
                                     bool               bWasTransformed
                                     )
{
    double rel_x = rElem.x, rel_y = rElem.y;

    rProps[ USTR( "draw:z-index" ) ] = rtl::OUString::valueOf( rElem.ZOrder );
    rProps[ USTR( "draw:style-name" )] = rEmitContext.rStyles.getStyleName( rElem.StyleId );
    rProps[ USTR( "svg:width" ) ]   = convertPixelToUnitString( rElem.w );
    rProps[ USTR( "svg:height" ) ]  = convertPixelToUnitString( rElem.h );

    const GraphicsContext& rGC =
        rEmitContext.rProcessor.getGraphicsContext( rElem.GCId );
    if( rGC.Transformation.isIdentity() || bWasTransformed )
    {
        rProps[ USTR( "svg:x" ) ]       = convertPixelToUnitString( rel_x );
        rProps[ USTR( "svg:y" ) ]       = convertPixelToUnitString( rel_y );
    }
    else
    {
        basegfx::B2DTuple aScale, aTranslation;
        double fRotate, fShearX;

        rGC.Transformation.decompose( aScale, aTranslation, fRotate, fShearX );

        rtl::OUStringBuffer aBuf( 256 );

        // TODO(F2): general transformation case missing; if implemented, note
        // that ODF rotation is oriented the other way

        // vertical mirroring is done by horizontally mirroring and rotaing 180 degree
        // quaint !
        if( rElem.MirrorVertical )
            fRotate += M_PI;

        // build transformation string
        if( fShearX != 0.0 )
        {
            aBuf.appendAscii( "skewX( " );
            aBuf.append( fShearX );
            aBuf.appendAscii( " )" );
        }
        if( fRotate != 0.0 )
        {
            if( aBuf.getLength() > 0 )
                aBuf.append( sal_Unicode(' ') );
            aBuf.appendAscii( "rotate( " );
            aBuf.append( -fRotate );
            aBuf.appendAscii( " )" );

        }
        if( aBuf.getLength() > 0 )
            aBuf.append( sal_Unicode(' ') );
        aBuf.appendAscii( "translate( " );
        aBuf.append( convertPixelToUnitString( rel_x ) );
        aBuf.append( sal_Unicode(' ') );
        aBuf.append( convertPixelToUnitString( rel_y ) );
        aBuf.appendAscii( " )" );

        rProps[ USTR( "draw:transform" ) ] = aBuf.makeStringAndClear();
    }
}

void DrawXmlEmitter::visit( FrameElement& elem, const std::list< Element* >::const_iterator&   )
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

void DrawXmlEmitter::visit( PolyPolyElement& elem, const std::list< Element* >::const_iterator& )
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
    // PDFIProcessor transforms geometrical objects, not images and text
    // so we need to tell fillFrameProps here that the transformation for
    // a PolyPolyElement was already applied (aside form translation)
    fillFrameProps( elem, aProps, m_rEmitContext, true );
    rtl::OUStringBuffer aBuf( 64 );
    aBuf.appendAscii( "0 0 " );
    aBuf.append( convPx2mmPrec2(elem.w)*100.0 );
    aBuf.append( sal_Unicode(' ') );
    aBuf.append( convPx2mmPrec2(elem.h)*100.0 );
    aProps[ USTR( "svg:viewBox" ) ] = aBuf.makeStringAndClear();
    aProps[ USTR( "svg:d" ) ]       = basegfx::tools::exportToSvgD( elem.PolyPoly );

    m_rEmitContext.rEmitter.beginTag( "draw:path", aProps );
    m_rEmitContext.rEmitter.endTag( "draw:path" );
}

void DrawXmlEmitter::visit( ImageElement& elem, const std::list< Element* >::const_iterator& )
{
    PropertyMap aImageProps;
    m_rEmitContext.rEmitter.beginTag( "draw:image", aImageProps );
    m_rEmitContext.rEmitter.beginTag( "office:binary-data", PropertyMap() );
    m_rEmitContext.rImages.writeBase64EncodedStream( elem.Image, m_rEmitContext);
    m_rEmitContext.rEmitter.endTag( "office:binary-data" );
    m_rEmitContext.rEmitter.endTag( "draw:image" );
}

void DrawXmlEmitter::visit( PageElement& elem, const std::list< Element* >::const_iterator&   )
{
    PropertyMap aPageProps;
    aPageProps[ USTR( "draw:master-page-name" ) ] = m_rEmitContext.rStyles.getStyleName( elem.StyleId );

    m_rEmitContext.rEmitter.beginTag("draw:page", aPageProps);

    if( m_rEmitContext.xStatusIndicator.is() )
        m_rEmitContext.xStatusIndicator->setValue( elem.PageNumber );

    std::list< Element* >::iterator this_it =  elem.Children.begin();
    while( this_it !=elem.Children.end() && *this_it != &elem )
    {
        (*this_it)->visitedBy( *this, this_it );
        ++this_it;
    }

    m_rEmitContext.rEmitter.endTag("draw:page");
}

void DrawXmlEmitter::visit( DocumentElement& elem, const std::list< Element* >::const_iterator&)
{
    m_rEmitContext.rEmitter.beginTag( "office:body", PropertyMap() );
    m_rEmitContext.rEmitter.beginTag( m_bWriteDrawDocument ? "office:drawing" : "office:presentation",
                                      PropertyMap() );

    std::list< Element* >::iterator this_it =  elem.Children.begin();
    while( this_it !=elem.Children.end() && *this_it != &elem )
    {
        (*this_it)->visitedBy( *this, this_it );
        ++this_it;
    }

    m_rEmitContext.rEmitter.endTag( m_bWriteDrawDocument ? "office:drawing" : "office:presentation" );
    m_rEmitContext.rEmitter.endTag( "office:body" );
}

/////////////////////////////////////////////////////////////////

void DrawXmlOptimizer::visit( HyperlinkElement&, const std::list< Element* >::const_iterator& )
{
}

void DrawXmlOptimizer::visit( TextElement&, const std::list< Element* >::const_iterator&)
{
}

void DrawXmlOptimizer::visit( FrameElement& elem, const std::list< Element* >::const_iterator& )
{
    elem.applyToChildren(*this);
}

void DrawXmlOptimizer::visit( ImageElement&, const std::list< Element* >::const_iterator& )
{
}

void DrawXmlOptimizer::visit( PolyPolyElement& elem, const std::list< Element* >::const_iterator& )
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

                // TODO(F2): this comparison fails for OOo-generated polygons with beziers.
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
                        rThisGC.FillColor.Red  == rNextGC.FillColor.Red &&
                        rThisGC.FillColor.Green== rNextGC.FillColor.Green &&
                        rThisGC.FillColor.Blue == rNextGC.FillColor.Blue &&
                        rThisGC.FillColor.Alpha== rNextGC.FillColor.Alpha &&
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

void DrawXmlOptimizer::visit( ParagraphElement& elem, const std::list< Element* >::const_iterator& )
{
    optimizeTextElements( elem );

    elem.applyToChildren(*this);
}

void DrawXmlOptimizer::visit( PageElement& elem, const std::list< Element* >::const_iterator& )
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
                    if( pGeo->y > pCurPara->y + pCurPara->h + fCurLineHeight*0.5  )
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
       if (! pCurPara )  // new paragraph, insert one
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
}

bool isSpaces(TextElement* pTextElem)
{
    for (sal_Int32 i = 0; i != pTextElem->Text.getLength(); ++i) {
        if (pTextElem->Text[i] != ' ') {
            return false;
        }
    }
    return true;
}

bool notTransformed(GraphicsContext GC)
{
    return (
        GC.Transformation.get(0,0) ==  100.00 &&
        GC.Transformation.get(1,0) ==    0.00 &&
        GC.Transformation.get(0,1) ==    0.00 &&
        GC.Transformation.get(1,1) == -100.00
       );
}

void DrawXmlOptimizer::optimizeTextElements(Element& rParent)
{
    if( rParent.Children.empty() ) // this should not happen
    {
        OSL_FAIL( "empty paragraph optimized" );
        return;
    }

    // concatenate child elements with same font id
    std::list< Element* >::iterator next = rParent.Children.begin();
    std::list< Element* >::iterator it = next++;

    while( next != rParent.Children.end() )
    {
        bool bConcat = false;
        TextElement* pCur = dynamic_cast<TextElement*>(*it);

        if( pCur )
        {
            TextElement* pNext = dynamic_cast<TextElement*>(*next);
            bool isComplex = false;
            rtl::OUString str(pCur->Text.getStr());
            for(int i=0; i< str.getLength(); i++)
            {
                sal_Int16 nType = GetBreakIterator()->getScriptType( str, i );
                if (nType == ::com::sun::star::i18n::ScriptType::COMPLEX)
                    isComplex = true;
            }
            bool bPara = strspn("ParagraphElement", typeid(rParent).name());
            ParagraphElement* pPara = dynamic_cast<ParagraphElement*>(&rParent);
            if (bPara && isComplex)
                pPara->bRtl = true;
            if( pNext )
            {
                const GraphicsContext& rCurGC = m_rProcessor.getGraphicsContext( pCur->GCId );
                const GraphicsContext& rNextGC = m_rProcessor.getGraphicsContext( pNext->GCId );

                // line and space optimization; works only in strictly horizontal mode

                // concatenate consecutive text elements unless there is a
                // font or text color or matrix change, leave a new span in that case
                if( (pCur->FontId == pNext->FontId || isSpaces(pNext)) &&
                    rCurGC.FillColor.Red == rNextGC.FillColor.Red &&
                    rCurGC.FillColor.Green == rNextGC.FillColor.Green &&
                    rCurGC.FillColor.Blue == rNextGC.FillColor.Blue &&
                    rCurGC.FillColor.Alpha == rNextGC.FillColor.Alpha &&
                    (rCurGC.Transformation == rNextGC.Transformation || notTransformed(rNextGC))
                    )
                {
                    pCur->updateGeometryWith( pNext );
                    // append text to current element
                        pCur->Text.append( pNext->Text.getStr(), pNext->Text.getLength() );

                        str = pCur->Text.getStr();
                    for(int i=0; i< str.getLength(); i++)
                    {
                        sal_Int16 nType = GetBreakIterator()->getScriptType( str, i );
                        if (nType == ::com::sun::star::i18n::ScriptType::COMPLEX)
                            isComplex = true;
                    }
                    if (bPara && isComplex)
                        pPara->bRtl = true;
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
        if ( bConcat )
            next = it;
        else
            ++it;
        ++next;
    }
}

void DrawXmlOptimizer::visit( DocumentElement& elem, const std::list< Element* >::const_iterator&)
{
    elem.applyToChildren(*this);
}

//////////////////////////////////////////////////////////////////////////////////


void DrawXmlFinalizer::visit( PolyPolyElement& elem, const std::list< Element* >::const_iterator& )
{
    // xxx TODO copied from DrawElement
    const GraphicsContext& rGC = m_rProcessor.getGraphicsContext(elem.GCId );
    PropertyMap aProps;
    aProps[ USTR( "style:family" ) ] = USTR( "graphic" );
    aProps[ USTR( "style:parent-style-name") ] = USTR( "standard" );
    // generate standard graphic style if necessary
    m_rStyleContainer.getStandardStyleId( "graphic" );

    PropertyMap aGCProps;

    // TODO(F3): proper dash emulation
    if( elem.Action & PATH_STROKE )
    {
        aGCProps[ USTR("draw:stroke") ] = rGC.DashArray.empty() ? USTR("solid") : USTR("dash");
        aGCProps[ USTR("svg:stroke-color") ] = getColorString( rGC.LineColor );
        if( rGC.LineWidth != 0.0 )
        {
            ::basegfx::B2DVector aVec(rGC.LineWidth,0);
            aVec *= rGC.Transformation;

            aVec.setX ( convPx2mmPrec2( aVec.getX() )*100.0 );
            aVec.setY ( convPx2mmPrec2( aVec.getY() )*100.0 );

            aGCProps[ USTR("svg:stroke-width") ] = rtl::OUString::valueOf( aVec.getLength() );
        }
    }
    else
    {
        aGCProps[ USTR("draw:stroke") ] = USTR("none");
    }

    // TODO(F1): check whether stuff could be emulated by gradient/bitmap/hatch
    if( elem.Action & (PATH_FILL | PATH_EOFILL) )
    {
        aGCProps[ USTR("draw:fill") ]   = USTR("solid");
        aGCProps[ USTR("draw:fill-color") ] = getColorString( rGC.FillColor );
    }
    else
    {
        aGCProps[ USTR("draw:fill") ] = USTR("none");
    }

    StyleContainer::Style aStyle( "style:style", aProps );
    StyleContainer::Style aSubStyle( "style:graphic-properties", aGCProps );
    aStyle.SubStyles.push_back( &aSubStyle );

    elem.StyleId = m_rStyleContainer.getStyleId( aStyle );
}

void DrawXmlFinalizer::visit( HyperlinkElement&, const std::list< Element* >::const_iterator& )
{
}

void DrawXmlFinalizer::visit( TextElement& elem, const std::list< Element* >::const_iterator& )
{
    const FontAttributes& rFont = m_rProcessor.getFont( elem.FontId );
    PropertyMap aProps;
    aProps[ USTR( "style:family" ) ] = USTR( "text" );

    PropertyMap aFontProps;

    // family name
    aFontProps[ USTR( "fo:font-family" ) ] = rFont.familyName;
    aFontProps[ USTR( "style:font-family-complex" ) ] = rFont.familyName;

    // bold
    if( rFont.isBold )
    {
        aFontProps[ USTR( "fo:font-weight" ) ]         = USTR( "bold" );
        aFontProps[ USTR( "fo:font-weight-asian" ) ]   = USTR( "bold" );
        aFontProps[ USTR( "style:font-weight-complex" ) ] = USTR( "bold" );
    }
    // italic
    if( rFont.isItalic )
    {
        aFontProps[ USTR( "fo:font-style" ) ]         = USTR( "italic" );
        aFontProps[ USTR( "fo:font-style-asian" ) ]   = USTR( "italic" );
        aFontProps[ USTR( "style:font-style-complex" ) ] = USTR( "italic" );
    }
    // underline
    if( rFont.isUnderline )
    {
        aFontProps[ USTR( "style:text-underline-style" ) ]  = USTR( "solid" );
        aFontProps[ USTR( "style:text-underline-width" ) ]  = USTR( "auto" );
        aFontProps[ USTR( "style:text-underline-color" ) ]  = USTR( "font-color" );
    }
    // outline
    if( rFont.isOutline )
    {
        aFontProps[ USTR( "style:text-outline" ) ]  = USTR( "true" );
    }
    // size
    rtl::OUStringBuffer aBuf( 32 );
    aBuf.append( rFont.size*72/PDFI_OUTDEV_RESOLUTION );
    aBuf.appendAscii( "pt" );
    rtl::OUString aFSize = aBuf.makeStringAndClear();
    aFontProps[ USTR( "fo:font-size" ) ]            = aFSize;
    aFontProps[ USTR( "style:font-size-asian" ) ]   = aFSize;
    aFontProps[ USTR( "style:font-size-complex" ) ] = aFSize;
    // color
    const GraphicsContext& rGC = m_rProcessor.getGraphicsContext( elem.GCId );
    aFontProps[ USTR( "fo:color" ) ]                 =  getColorString( rFont.isOutline ? rGC.LineColor : rGC.FillColor );

    StyleContainer::Style aStyle( "style:style", aProps );
    StyleContainer::Style aSubStyle( "style:text-properties", aFontProps );
    aStyle.SubStyles.push_back( &aSubStyle );
    elem.StyleId = m_rStyleContainer.getStyleId( aStyle );
}

void DrawXmlFinalizer::visit( ParagraphElement& elem, const std::list< Element* >::const_iterator& )
{

    PropertyMap aProps;
    aProps[ USTR( "style:family" ) ] = USTR( "paragraph" );
    // generate standard paragraph style if necessary
    m_rStyleContainer.getStandardStyleId( "paragraph" );

    PropertyMap aParProps;

    aParProps[ USTR("fo:text-align")]                   = USTR("start");
    if (elem.bRtl)
        aParProps[ USTR("style:writing-mode")]                    = USTR("rl-tb");
    else
        aParProps[ USTR("style:writing-mode")]                    = USTR("lr-tb");

    StyleContainer::Style aStyle( "style:style", aProps );
    StyleContainer::Style aSubStyle( "style:paragraph-properties", aParProps );
    aStyle.SubStyles.push_back( &aSubStyle );

    elem.StyleId = m_rStyleContainer.getStyleId( aStyle );

    elem.applyToChildren(*this);
}

void DrawXmlFinalizer::visit( FrameElement& elem, const std::list< Element* >::const_iterator&)
{
    PropertyMap aProps;
    aProps[ USTR( "style:family" ) ] = USTR( "graphic" );
    aProps[ USTR( "style:parent-style-name") ] = USTR( "standard" );
    // generate standard graphic style if necessary
    m_rStyleContainer.getStandardStyleId( "graphic" );

    PropertyMap aGCProps;

    aGCProps[ USTR("draw:stroke") ]                    = USTR("none");
    aGCProps[ USTR("draw:fill") ]                      = USTR("none");
    aGCProps[ USTR("draw:auto-grow-height") ]          = USTR("true");
    aGCProps[ USTR("draw:auto-grow-width") ]           = USTR("true");
    aGCProps[ USTR("draw:textarea-horizontal-align") ] = USTR("left");
    aGCProps[ USTR("draw:textarea-vertical-align") ]   = USTR("top");
    aGCProps[ USTR("fo:min-height")]                   = USTR("0cm");
    aGCProps[ USTR("fo:min-width")]                    = USTR("0cm");
    aGCProps[ USTR("fo:padding-top") ]                 = USTR("0cm");
    aGCProps[ USTR("fo:padding-left") ]                = USTR("0cm");
    aGCProps[ USTR("fo:padding-right") ]               = USTR("0cm");
    aGCProps[ USTR("fo:padding-bottom") ]              = USTR("0cm");

    // remark: vertical mirroring is done in current OOO by
    // mirroring horzontally and rotating 180 degrees
    // this is quaint, but unfortunately it seems
    // mirror=vertical is defined but not implemented in current code
    if( elem.MirrorVertical )
        aGCProps[ USTR("style:mirror") ] = USTR("horizontal");

    StyleContainer::Style aStyle( "style:style", aProps );
    StyleContainer::Style aSubStyle( "style:graphic-properties", aGCProps );
    aStyle.SubStyles.push_back( &aSubStyle );

    elem.StyleId = m_rStyleContainer.getStyleId( aStyle );
    elem.applyToChildren(*this);
}

void DrawXmlFinalizer::visit( ImageElement&, const std::list< Element* >::const_iterator& )
{
}

void DrawXmlFinalizer::visit( PageElement& elem, const std::list< Element* >::const_iterator& )
{
    if( m_rProcessor.getStatusIndicator().is() )
        m_rProcessor.getStatusIndicator()->setValue( elem.PageNumber );

    // transform from pixel to mm
    double page_width = convPx2mm( elem.w ), page_height = convPx2mm( elem.h );

    // calculate page margins out of the relevant children (paragraphs)
    elem.TopMargin = elem.h, elem.BottomMargin = 0, elem.LeftMargin = elem.w, elem.RightMargin = 0;

    for( std::list< Element* >::const_iterator it = elem.Children.begin(); it != elem.Children.end(); ++it )
    {
        if( (*it)->x < elem.LeftMargin )
            elem.LeftMargin = (*it)->x;
        if( (*it)->y < elem.TopMargin )
            elem.TopMargin = (*it)->y;
        if( (*it)->x + (*it)->w > elem.RightMargin )
            elem.RightMargin = ((*it)->x + (*it)->w);
        if( (*it)->y + (*it)->h > elem.BottomMargin )
            elem.BottomMargin = ((*it)->y + (*it)->h);
    }

    // transform margins to mm
    double left_margin     = convPx2mm( elem.LeftMargin );
    double right_margin    = convPx2mm( elem.RightMargin );
    double top_margin      = convPx2mm( elem.TopMargin );
    double bottom_margin   = convPx2mm( elem.BottomMargin );

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
    rtl::OUStringBuffer aBuf( 64 );
    aPageLayoutProps[ USTR( "fo:margin-top" ) ]     =  unitMMString( top_margin );
    aPageLayoutProps[ USTR( "fo:margin-bottom" ) ]  =  unitMMString( bottom_margin );
    aPageLayoutProps[ USTR( "fo:margin-left" ) ]    =  unitMMString( left_margin );
    aPageLayoutProps[ USTR( "fo:margin-right" ) ]   =  unitMMString( right_margin );
    aPageLayoutProps[ USTR( "fo:page-width" ) ]     =  unitMMString( page_width );
    aPageLayoutProps[ USTR( "fo:page-height" ) ]    =  unitMMString( page_height );
    aPageLayoutProps[ USTR( "style:print-orientation" ) ]= elem.w < elem.h ? USTR( "portrait" ) : USTR( "landscape" );
    aPageLayoutProps[ USTR( "style:writing-mode" ) ]= USTR( "lr-tb" );

    StyleContainer::Style aStyle( "style:page-layout", aPageProps);
    StyleContainer::Style aSubStyle( "style:page-layout-properties", aPageLayoutProps);
    aStyle.SubStyles.push_back(&aSubStyle);
    sal_Int32 nPageStyle = m_rStyleContainer.impl_getStyleId( aStyle, false );

    // create master page
    rtl::OUString aMasterPageLayoutName = m_rStyleContainer.getStyleName( nPageStyle );
    aPageProps[ USTR( "style:page-layout-name" ) ] = aMasterPageLayoutName;

    StyleContainer::Style aMPStyle( "style:master-page", aPageProps);

    StyleContainer::Style aHeaderStyle( "style:header", PropertyMap() );
    StyleContainer::Style aFooterStyle( "style:footer", PropertyMap() );

    elem.StyleId = m_rStyleContainer.impl_getStyleId( aMPStyle,false );


    rtl::OUString aMasterPageName = m_rStyleContainer.getStyleName( elem.StyleId );

    // create styles for children
    elem.applyToChildren(*this);
}

void DrawXmlFinalizer::visit( DocumentElement& elem, const std::list< Element* >::const_iterator& )
{
    elem.applyToChildren(*this);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
