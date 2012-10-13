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


#include "xmlemitter.hxx"
#include "genericelements.hxx"
#include "pdfiprocessor.hxx"
#include "pdfihelper.hxx"
#include "style.hxx"


#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/range/b2drange.hxx>

namespace pdfi
{

ElementFactory::~ElementFactory()
{
}

Element::~Element()
{
    while( !Children.empty() )
    {
        Element* pCurr( Children.front() );
        delete pCurr;
        Children.pop_front();
    }
}

void Element::applyToChildren( ElementTreeVisitor& rVisitor )
{
    for( std::list< Element* >::iterator it = Children.begin(); it != Children.end(); ++it )
        (*it)->visitedBy( rVisitor, it );
}

void Element::setParent( std::list<Element*>::iterator& el, Element* pNewParent )
{
    if( pNewParent )
    {
        pNewParent->Children.splice( pNewParent->Children.end(), (*el)->Parent->Children, el );
        (*el)->Parent = pNewParent;
    }
}

void Element::updateGeometryWith( const Element* pMergeFrom )
{
    if( w == 0 && h == 0 )
    {
        x = pMergeFrom->x;
        y = pMergeFrom->y;
        w = pMergeFrom->w;
        h = pMergeFrom->h;
    }
    else
    {
        if( pMergeFrom->x < x )
        {
            w += x - pMergeFrom->x;
            x = pMergeFrom->x;
        }
        if( pMergeFrom->x+pMergeFrom->w > x+w )
            w = pMergeFrom->w+pMergeFrom->x - x;
        if( pMergeFrom->y < y )
        {
            h += y - pMergeFrom->y;
            y = pMergeFrom->y;
        }
        if( pMergeFrom->y+pMergeFrom->h > y+h )
            h = pMergeFrom->h+pMergeFrom->y - y;
    }
}


#if OSL_DEBUG_LEVEL > 1
#include <typeinfo>
void Element::emitStructure( int nLevel)
{
    OSL_TRACE( "%*s<%s %p> (%.1f,%.1f)+(%.1fx%.1f)\n",
               nLevel, "", typeid( *this ).name(), this,
               x, y, w, h );
    for( std::list< Element* >::iterator it = Children.begin(); it != Children.end(); ++it )
        (*it)->emitStructure(nLevel+1 );
    OSL_TRACE( "%*s</%s>", nLevel, "", typeid( *this ).name() );
}
#endif

void ListElement::visitedBy( ElementTreeVisitor& visitor, const std::list< Element* >::const_iterator& )
{
    // this is only an inner node
    applyToChildren(visitor);
}

void HyperlinkElement::visitedBy( ElementTreeVisitor&                          rVisitor,
                                  const std::list< Element* >::const_iterator& rParentIt )
{
    rVisitor.visit(*this,rParentIt);
}

void TextElement::visitedBy( ElementTreeVisitor&                          rVisitor,
                             const std::list< Element* >::const_iterator& rParentIt )
{
    rVisitor.visit(*this,rParentIt);
}

void FrameElement::visitedBy( ElementTreeVisitor&                          rVisitor,
                              const std::list< Element* >::const_iterator& rParentIt )
{
    rVisitor.visit(*this,rParentIt);
}

void ImageElement::visitedBy( ElementTreeVisitor&                          rVisitor,
                              const std::list< Element* >::const_iterator& rParentIt)
{
    rVisitor.visit( *this, rParentIt);
}

PolyPolyElement::PolyPolyElement( Element*                       pParent,
                                  sal_Int32                      nGCId,
                                  const basegfx::B2DPolyPolygon& rPolyPoly,
                                  sal_Int8                       nAction )
    : DrawElement( pParent, nGCId ),
      PolyPoly( rPolyPoly ),
      Action( nAction )
{
}

void PolyPolyElement::updateGeometry()
{
    basegfx::B2DRange aRange;
    if( PolyPoly.areControlPointsUsed() )
        aRange = basegfx::tools::getRange( basegfx::tools::adaptiveSubdivideByAngle( PolyPoly ) );
    else
        aRange = basegfx::tools::getRange( PolyPoly );
    x = aRange.getMinX();
    y = aRange.getMinY();
    w = aRange.getWidth();
    h = aRange.getHeight();

    // fdo#32330 - non-closed paths will not show up filled in LibO
    if( Action & (PATH_FILL | PATH_EOFILL) )
        PolyPoly.setClosed(true);
}

void PolyPolyElement::visitedBy( ElementTreeVisitor&                          rVisitor,
                                 const std::list< Element* >::const_iterator& rParentIt)
{
    rVisitor.visit( *this, rParentIt);
}

#if OSL_DEBUG_LEVEL > 1
void PolyPolyElement::emitStructure( int nLevel)
{
    OSL_TRACE( "%*s<%s %p>", nLevel, "", typeid( *this ).name(), this  );
    OSL_TRACE( "path=" );
    int nPoly = PolyPoly.count();
    for( int i = 0; i < nPoly; i++ )
    {
        basegfx::B2DPolygon aPoly = PolyPoly.getB2DPolygon( i );
        int nPoints = aPoly.count();
        for( int n = 0; n < nPoints; n++ )
        {
            basegfx::B2DPoint aPoint = aPoly.getB2DPoint( n );
            OSL_TRACE( " (%g,%g)", aPoint.getX(), aPoint.getY() );
        }
        OSL_TRACE( "\n" );
    }
    for( std::list< Element* >::iterator it = Children.begin(); it != Children.end(); ++it )
        (*it)->emitStructure( nLevel+1 );
    OSL_TRACE( "%*s</%s>", nLevel, "", typeid( *this ).name() );
}
#endif

void ParagraphElement::visitedBy( ElementTreeVisitor&                          rVisitor,
                                  const std::list< Element* >::const_iterator& rParentIt )
{
    rVisitor.visit(*this,rParentIt);
}

bool ParagraphElement::isSingleLined( PDFIProcessor& rProc ) const
{
    std::list< Element* >::const_iterator it = Children.begin();
    TextElement* pText = NULL, *pLastText = NULL;
    while( it != Children.end() )
    {
        // a paragraph containing subparagraphs cannot be single lined
        if( dynamic_cast< ParagraphElement* >(*it) != NULL )
            return false;

        pText = dynamic_cast< TextElement* >(*it);
        if( pText )
        {
            const FontAttributes& rFont = rProc.getFont( pText->FontId );
            if( pText->h > rFont.size*1.5 )
                return  false;
            if( pLastText )
            {
                if( pText->y > pLastText->y+pLastText->h ||
                    pLastText->y > pText->y+pText->h )
                    return false;
            }
            else
                pLastText = pText;
        }
        ++it;
    }

    // a paragraph without a single text is not considered single lined
    return pLastText != NULL;
}

double ParagraphElement::getLineHeight( PDFIProcessor& rProc ) const
{
    double line_h = 0;
    for( std::list< Element* >::const_iterator it = Children.begin(); it != Children.end(); ++it )
    {
        ParagraphElement* pPara = dynamic_cast< ParagraphElement* >(*it);
        TextElement* pText = NULL;
        if( pPara )
        {
            double lh = pPara->getLineHeight( rProc );
            if( lh > line_h )
                line_h = lh;
        }
        else if( (pText = dynamic_cast< TextElement* >( *it )) != NULL )
        {
            const FontAttributes& rFont = rProc.getFont( pText->FontId );
            double lh = pText->h;
            if( pText->h > rFont.size*1.5 )
                lh = rFont.size;
            if( lh > line_h )
                line_h = lh;
        }
    }
    return line_h;
}

TextElement* ParagraphElement::getFirstTextChild() const
{
    TextElement* pText = NULL;
    for( std::list< Element* >::const_iterator it = Children.begin();
         it != Children.end() && ! pText; ++it )
    {
        pText = dynamic_cast<TextElement*>(*it);
    }
    return pText;
}

PageElement::~PageElement()
{
    if( HeaderElement )
        delete HeaderElement;
    if( FooterElement )
        delete FooterElement;
}

void PageElement::visitedBy( ElementTreeVisitor&                          rVisitor,
                             const std::list< Element* >::const_iterator& rParentIt )
{
     rVisitor.visit(*this, rParentIt);
}

void PageElement::updateParagraphGeometry( Element* pEle )
{
    // update geometry of children
    for( std::list< Element* >::iterator it = pEle->Children.begin();
         it != pEle->Children.end(); ++it )
    {
        updateParagraphGeometry( *it );
    }
    // if this is a paragraph itself, then update according to children geometry
    if( dynamic_cast<ParagraphElement*>(pEle) )
    {
        for( std::list< Element* >::iterator it = pEle->Children.begin();
             it != pEle->Children.end(); ++it )
        {
            Element* pChild = NULL;
            TextElement* pText = dynamic_cast<TextElement*>(*it);
            if( pText )
                pChild = pText;
            else
            {
                ParagraphElement* pPara = dynamic_cast<ParagraphElement*>(*it);
                if( pPara )
                    pChild = pPara;
            }
            if( pChild )
                pEle->updateGeometryWith( pChild );
        }
    }
}

bool PageElement::resolveHyperlink( std::list<Element*>::iterator link_it, std::list<Element*>& rElements )
{
    HyperlinkElement* pLink = dynamic_cast<HyperlinkElement*>(*link_it);
    if( ! pLink ) // sanity check
        return false;

    for( std::list<Element*>::iterator it = rElements.begin(); it != rElements.end(); ++it )
    {
        if( (*it)->x >= pLink->x && (*it)->x + (*it)->w <= pLink->x + pLink->w &&
            (*it)->y >= pLink->y && (*it)->y + (*it)->h <= pLink->y + pLink->h )
        {
            TextElement* pText = dynamic_cast<TextElement*>(*it);
            if( pText )
            {
                if( pLink->Children.empty() )
                {
                    // insert the hyperlink before the frame
                    rElements.splice( it, Hyperlinks.Children, link_it );
                    pLink->Parent = (*it)->Parent;
                }
                // move text element into hyperlink
                std::list<Element*>::iterator next = it;
                ++next;
                Element::setParent( it, pLink );
                it = next;
                --it;
                continue;
            }
            // a link can contain multiple text elements or a single frame
            if( ! pLink->Children.empty() )
                continue;
            if( dynamic_cast<ParagraphElement*>(*it)  )
            {
                if( resolveHyperlink( link_it, (*it)->Children ) )
                    break;
                continue;
            }
            FrameElement* pFrame = dynamic_cast<FrameElement*>(*it);
            if( pFrame )
            {
                // insert the hyperlink before the frame
                rElements.splice( it, Hyperlinks.Children, link_it );
                pLink->Parent = (*it)->Parent;
                // move frame into hyperlink
                Element::setParent( it, pLink );
                break;
            }
        }
    }
    return ! pLink->Children.empty();
}

void PageElement::resolveHyperlinks()
{
    while( ! Hyperlinks.Children.empty() )
    {
        if( ! resolveHyperlink( Hyperlinks.Children.begin(), Children ) )
        {
            delete Hyperlinks.Children.front();
            Hyperlinks.Children.pop_front();
        }
    }
}

void PageElement::resolveFontStyles( PDFIProcessor& rProc )
{
    resolveUnderlines(rProc);
}

void PageElement::resolveUnderlines( PDFIProcessor& rProc )
{
    // FIXME: currently the algorithm used is quadratic
    // this could be solved by some sorting beforehand

    std::list< Element* >::iterator poly_it = Children.begin();
    while( poly_it != Children.end() )
    {
        PolyPolyElement* pPoly = dynamic_cast< PolyPolyElement* >(*poly_it);
        if( ! pPoly || ! pPoly->Children.empty() )
        {
            ++poly_it;
            continue;
        }
        /* check for: no filling
        *             only two points (FIXME: handle small rectangles, too)
        *             y coordinates of points are equal
        */
        if( pPoly->Action != PATH_STROKE )
        {
            ++poly_it;
            continue;
        }
        if( pPoly->PolyPoly.count() != 1 )
        {
            ++poly_it;
            continue;
        }

        bool bRemovePoly = false;
        basegfx::B2DPolygon aPoly = pPoly->PolyPoly.getB2DPolygon(0);
        if( aPoly.count() != 2 ||
            aPoly.getB2DPoint(0).getY() != aPoly.getB2DPoint(1).getY() )
        {
            ++poly_it;
            continue;
        }
        double l_x = aPoly.getB2DPoint(0).getX();
        double r_x = aPoly.getB2DPoint(1).getX();
        double u_y;
        if( r_x < l_x )
        {
            u_y = r_x; r_x = l_x; l_x = u_y;
        }
        u_y = aPoly.getB2DPoint(0).getY();
        for( std::list< Element*>::iterator it = Children.begin();
             it != Children.end(); ++it )
        {
            Element* pEle = *it;
            if( pEle->y <= u_y && pEle->y + pEle->h*1.1 >= u_y )
            {
                // first: is the element underlined completely ?
                if( pEle->x + pEle->w*0.1 >= l_x &&
                    pEle->x + pEle->w*0.9 <= r_x )
                {
                    TextElement* pText = dynamic_cast< TextElement* >(pEle);
                    if( pText )
                    {
                        const GraphicsContext& rTextGC = rProc.getGraphicsContext( pText->GCId );
                        if( ! rTextGC.isRotatedOrSkewed() )
                        {
                            bRemovePoly = true;
                            // retrieve ID for modified font
                            FontAttributes aAttr = rProc.getFont( pText->FontId );
                            aAttr.isUnderline = true;
                            pText->FontId = rProc.getFontId( aAttr );
                        }
                    }
                    else if( dynamic_cast< HyperlinkElement* >(pEle) )
                        bRemovePoly = true;
                }
                // second: hyperlinks may be larger than their underline
                // since they are just arbitrary rectangles in the action definition
                else if( dynamic_cast< HyperlinkElement* >(pEle) != NULL &&
                         l_x >= pEle->x && r_x <= pEle->x+pEle->w )
                {
                    bRemovePoly = true;
                }
            }
        }
        if( bRemovePoly )
        {
            std::list< Element* >::iterator next_it = poly_it;
            ++next_it;
            Children.erase( poly_it );
            delete pPoly;
            poly_it = next_it;
        }
        else
            ++poly_it;
    }
}

DocumentElement::~DocumentElement()
{
}

void DocumentElement::visitedBy( ElementTreeVisitor&                          rVisitor,
                                 const std::list< Element* >::const_iterator& rParentIt)
{
    rVisitor.visit(*this, rParentIt);
}


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
