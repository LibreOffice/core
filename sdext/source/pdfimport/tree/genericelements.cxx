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


#include <genericelements.hxx>
#include <pdfiprocessor.hxx>
#include <pdfihelper.hxx>


#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/range/b2drange.hxx>
#include <sal/log.hxx>

namespace pdfi
{

Element::~Element()
{
}

void Element::applyToChildren( ElementTreeVisitor& rVisitor )
{
    for( auto it = Children.begin(); it != Children.end(); ++it )
        (*it)->visitedBy( rVisitor, it );
}

void Element::setParent( std::list<std::unique_ptr<Element>>::iterator const & el, Element* pNewParent )
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


#if OSL_DEBUG_LEVEL > 0
#include <typeinfo>
void Element::emitStructure( int nLevel)
{
    SAL_INFO( "sdext", std::string(nLevel, ' ') << "<" << typeid( *this ).name() << " " << this << "> ("
                << std::setprecision(1) << x << "," << y << ")+(" << w << "x" << h << ")" );
    for (auto const& child : Children)
        child->emitStructure(nLevel+1);
    SAL_INFO( "sdext", std::string(nLevel, ' ') << "</" << typeid( *this ).name() << ">"  );
}
#endif

void ListElement::visitedBy( ElementTreeVisitor& visitor, const std::list< std::unique_ptr<Element> >::const_iterator& )
{
    // this is only an inner node
    applyToChildren(visitor);
}

void HyperlinkElement::visitedBy( ElementTreeVisitor&                          rVisitor,
                                  const std::list< std::unique_ptr<Element> >::const_iterator& rParentIt )
{
    rVisitor.visit(*this,rParentIt);
}

void TextElement::visitedBy( ElementTreeVisitor&                          rVisitor,
                             const std::list< std::unique_ptr<Element> >::const_iterator& rParentIt )
{
    rVisitor.visit(*this,rParentIt);
}

void FrameElement::visitedBy( ElementTreeVisitor&                          rVisitor,
                              const std::list< std::unique_ptr<Element> >::const_iterator& rParentIt )
{
    rVisitor.visit(*this,rParentIt);
}

void ImageElement::visitedBy( ElementTreeVisitor&                          rVisitor,
                              const std::list< std::unique_ptr<Element> >::const_iterator& rParentIt)
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
        aRange = basegfx::utils::getRange( basegfx::utils::adaptiveSubdivideByAngle( PolyPoly ) );
    else
        aRange = basegfx::utils::getRange( PolyPoly );
    x = aRange.getMinX();
    y = aRange.getMinY();
    w = aRange.getWidth();
    h = aRange.getHeight();

    // fdo#32330 - non-closed paths will not show up filled in LibO
    if( Action & (PATH_FILL | PATH_EOFILL) )
        PolyPoly.setClosed(true);
}

void PolyPolyElement::visitedBy( ElementTreeVisitor&                          rVisitor,
                                 const std::list< std::unique_ptr<Element> >::const_iterator& rParentIt)
{
    rVisitor.visit( *this, rParentIt);
}

#if OSL_DEBUG_LEVEL > 0
void PolyPolyElement::emitStructure( int nLevel)
{
    SAL_WARN( "sdext", std::string(nLevel, ' ') << "<" << typeid( *this ).name() << " " << this << ">" );
    SAL_WARN( "sdext", "path=" );
    int nPoly = PolyPoly.count();
    for( int i = 0; i < nPoly; i++ )
    {
        OUStringBuffer buff;
        basegfx::B2DPolygon aPoly = PolyPoly.getB2DPolygon( i );
        int nPoints = aPoly.count();
        for( int n = 0; n < nPoints; n++ )
        {
            basegfx::B2DPoint aPoint = aPoly.getB2DPoint( n );
            buff.append( " (").append(aPoint.getX()).append(",").append(aPoint.getY()).append(")");
        }
        SAL_WARN( "sdext", "    " << buff.makeStringAndClear() );
    }
    for (auto const& child : Children)
        child->emitStructure( nLevel+1 );
    SAL_WARN( "sdext", std::string(nLevel, ' ') << "</" << typeid( *this ).name() << ">");
}
#endif

void ParagraphElement::visitedBy( ElementTreeVisitor&                          rVisitor,
                                  const std::list< std::unique_ptr<Element> >::const_iterator& rParentIt )
{
    rVisitor.visit(*this,rParentIt);
}

bool ParagraphElement::isSingleLined( PDFIProcessor const & rProc ) const
{
    TextElement* pText = nullptr, *pLastText = nullptr;
    for( auto& rxChild : Children )
    {
        // a paragraph containing subparagraphs cannot be single lined
        if( dynamic_cast< ParagraphElement* >(rxChild.get()) != nullptr )
            return false;

        pText = dynamic_cast< TextElement* >(rxChild.get());
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
    }

    // a paragraph without a single text is not considered single lined
    return pLastText != nullptr;
}

double ParagraphElement::getLineHeight( PDFIProcessor& rProc ) const
{
    double line_h = 0;
    for( auto& rxChild : Children )
    {
        ParagraphElement* pPara = dynamic_cast< ParagraphElement* >(rxChild.get());
        TextElement* pText = nullptr;
        if( pPara )
        {
            double lh = pPara->getLineHeight( rProc );
            if( lh > line_h )
                line_h = lh;
        }
        else if( (pText = dynamic_cast< TextElement* >( rxChild.get() )) != nullptr )
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
    TextElement* pText = nullptr;
    auto it = std::find_if(Children.begin(), Children.end(),
        [](const std::unique_ptr<Element>& rxElem) { return dynamic_cast<TextElement*>(rxElem.get()) != nullptr; });
    if (it != Children.end())
        pText = dynamic_cast<TextElement*>(it->get());
    return pText;
}

PageElement::~PageElement()
{
}

void PageElement::visitedBy( ElementTreeVisitor&                          rVisitor,
                             const std::list< std::unique_ptr<Element> >::const_iterator& rParentIt )
{
     rVisitor.visit(*this, rParentIt);
}

bool PageElement::resolveHyperlink( const std::list<std::unique_ptr<Element>>::iterator& link_it, std::list<std::unique_ptr<Element>>& rElements )
{
    HyperlinkElement* pLink = dynamic_cast<HyperlinkElement*>(link_it->get());
    if( ! pLink ) // sanity check
        return false;

    for( auto it = rElements.begin(); it != rElements.end(); ++it )
    {
        if( (*it)->x >= pLink->x && (*it)->x + (*it)->w <= pLink->x + pLink->w &&
            (*it)->y >= pLink->y && (*it)->y + (*it)->h <= pLink->y + pLink->h )
        {
            TextElement* pText = dynamic_cast<TextElement*>(it->get());
            if( pText )
            {
                if( pLink->Children.empty() )
                {
                    // insert the hyperlink before the frame
                    rElements.splice( it, Hyperlinks.Children, link_it );
                    pLink->Parent = (*it)->Parent;
                }
                // move text element into hyperlink
                auto next = it;
                ++next;
                Element::setParent( it, pLink );
                it = next;
                --it;
                continue;
            }
            // a link can contain multiple text elements or a single frame
            if( ! pLink->Children.empty() )
                continue;
            if( dynamic_cast<ParagraphElement*>(it->get())  )
            {
                if( resolveHyperlink( link_it, (*it)->Children ) )
                    break;
                continue;
            }
            FrameElement* pFrame = dynamic_cast<FrameElement*>(it->get());
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
            Hyperlinks.Children.pop_front();
        }
    }
}

void PageElement::resolveFontStyles( PDFIProcessor const & rProc )
{
    resolveUnderlines(rProc);
}

void PageElement::resolveUnderlines( PDFIProcessor const & rProc )
{
    // FIXME: currently the algorithm used is quadratic
    // this could be solved by some sorting beforehand

    auto poly_it = Children.begin();
    while( poly_it != Children.end() )
    {
        PolyPolyElement* pPoly = dynamic_cast< PolyPolyElement* >(poly_it->get());
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
        for( const auto& rxChild : Children )
        {
            Element* pEle = rxChild.get();
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
                else if( dynamic_cast< HyperlinkElement* >(pEle) != nullptr &&
                         l_x >= pEle->x && r_x <= pEle->x+pEle->w )
                {
                    bRemovePoly = true;
                }
            }
        }
        if( bRemovePoly )
        {
            auto next_it = poly_it;
            ++next_it;
            Children.erase( poly_it );
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
                                 const std::list< std::unique_ptr<Element> >::const_iterator& rParentIt)
{
    rVisitor.visit(*this, rParentIt);
}


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
