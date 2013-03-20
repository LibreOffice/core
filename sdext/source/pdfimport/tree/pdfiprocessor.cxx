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
#include "genericelements.hxx"
#include "style.hxx"
#include "treevisiting.hxx"

#include <rtl/string.hxx>
#include <rtl/strbuf.hxx>

#include <comphelper/sequence.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/polygon/b2dpolygonclipper.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/tools/canvastools.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/range/b2irange.hxx>
#include <basegfx/range/b2drectangle.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <vcl/svapp.hxx>

#include <com/sun/star/rendering/XVolatileBitmap.hpp>
#include <com/sun/star/geometry/RealSize2D.hpp>
#include <com/sun/star/geometry/RealPoint2D.hpp>
#include <com/sun/star/geometry/RealRectangle2D.hpp>


using namespace com::sun::star;


namespace pdfi
{

 PDFIProcessor::PDFIProcessor( const uno::Reference< task::XStatusIndicator >& xStat ,
            com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >  xContext) :

    m_xContext(xContext),
    fYPrevTextPosition(-10000.0),
    fPrevTextHeight(0.0),
    fXPrevTextPosition(0.0),
    fPrevTextWidth(0.0),
    m_pElFactory( new ElementFactory() ),
    m_pDocument( m_pElFactory->createDocumentElement() ),
    m_pCurPage(0),
    m_pCurElement(0),
    m_nNextFontId( 1 ),
    m_aIdToFont(),
    m_aFontToId(),
    m_aGCStack(),
    m_nNextGCId( 1 ),
    m_aIdToGC(),
    m_aGCToId(),
    m_aImages(),
    m_eTextDirection( LrTb ),
    m_nPages(0),
    m_nNextZOrder( 1 ),
    m_bIsWhiteSpaceInLine( false ),
    m_xStatusIndicator( xStat ),
    m_bHaveTextOnDocLevel(false)
{
    FontAttributes aDefFont;
    aDefFont.familyName = "Helvetica";
    aDefFont.isBold     = false;
    aDefFont.isItalic   = false;
    aDefFont.size       = 10*PDFI_OUTDEV_RESOLUTION/72;
    m_aIdToFont[ 0 ]    = aDefFont;
    m_aFontToId[ aDefFont ] = 0;

    GraphicsContext aDefGC;
    m_aGCStack.push_back( aDefGC );
    m_aIdToGC[ 0 ] = aDefGC;
    m_aGCToId[ aDefGC ] = 0;
}

void PDFIProcessor::enableToplevelText()
{
    m_bHaveTextOnDocLevel = true;
}

void PDFIProcessor::setPageNum( sal_Int32 nPages )
{
    m_nPages = nPages;
}


void PDFIProcessor::pushState()
{
    GraphicsContextStack::value_type const a(m_aGCStack.back());
    m_aGCStack.push_back(a);
}

void PDFIProcessor::popState()
{
    m_aGCStack.pop_back();
}

void PDFIProcessor::setFlatness( double value )
{
    getCurrentContext().Flatness = value;
}

void PDFIProcessor::setTransformation( const geometry::AffineMatrix2D& rMatrix )
{
    basegfx::unotools::homMatrixFromAffineMatrix(
        getCurrentContext().Transformation,
        rMatrix );
}

void PDFIProcessor::setLineDash( const uno::Sequence<double>& dashes,
                                 double                       /*start*/ )
{
    // TODO(F2): factor in start offset
    GraphicsContext& rContext( getCurrentContext() );
    comphelper::sequenceToContainer(rContext.DashArray,dashes);
}

void PDFIProcessor::setLineJoin(sal_Int8 nJoin)
{
    getCurrentContext().LineJoin = nJoin;
}

void PDFIProcessor::setLineCap(sal_Int8 nCap)
{
    getCurrentContext().LineCap = nCap;
}

void PDFIProcessor::setMiterLimit(double)
{
    OSL_TRACE("PDFIProcessor::setMiterLimit(): not supported by ODF");
}

void PDFIProcessor::setLineWidth(double nWidth)
{
    getCurrentContext().LineWidth = nWidth;
}

void PDFIProcessor::setFillColor( const rendering::ARGBColor& rColor )
{
    getCurrentContext().FillColor = rColor;
}

void PDFIProcessor::setStrokeColor( const rendering::ARGBColor& rColor )
{
    getCurrentContext().LineColor = rColor;
}

void PDFIProcessor::setBlendMode(sal_Int8)
{
    OSL_TRACE("PDFIProcessor::setBlendMode(): not supported by ODF");
}

void PDFIProcessor::setFont( const FontAttributes& i_rFont )
{
    FontAttributes aChangedFont( i_rFont );
    GraphicsContext& rGC=getCurrentContext();
    // for text render modes, please see PDF reference manual
    aChangedFont.isOutline = ( (rGC.TextRenderMode == 1) || (rGC. TextRenderMode == 2) );
    FontToIdMap::const_iterator it = m_aFontToId.find( aChangedFont );
    if( it != m_aFontToId.end() )
        rGC.FontId = it->second;
    else
    {
        m_aFontToId[ aChangedFont ] = m_nNextFontId;
        m_aIdToFont[ m_nNextFontId ] = aChangedFont;
        rGC.FontId = m_nNextFontId;
        m_nNextFontId++;
    }
}

void PDFIProcessor::setTextRenderMode( sal_Int32 i_nMode )
{
    GraphicsContext& rGC=getCurrentContext();
    rGC.TextRenderMode = i_nMode;
    IdToFontMap::iterator it = m_aIdToFont.find( rGC.FontId );
    if( it != m_aIdToFont.end() )
        setFont( it->second );
}

sal_Int32 PDFIProcessor::getFontId( const FontAttributes& rAttr ) const
{
    const sal_Int32 nCurFont = getCurrentContext().FontId;
    const_cast<PDFIProcessor*>(this)->setFont( rAttr );
    const sal_Int32 nFont = getCurrentContext().FontId;
    const_cast<PDFIProcessor*>(this)->getCurrentContext().FontId = nCurFont;

    return nFont;
}

// line diagnose block - start
void PDFIProcessor::processGlyphLine()
{
    if( m_GlyphsList.empty() )
        return;

    double fPreAvarageSpaceValue= 0.0;
    double fAvarageDiffCharSpaceValue= 0.0;
    double fMinPreSpaceValue= 0.0;
    double fMaxPreSpaceValue= 0.0;
    double fNullSpaceBreakerAvaregeSpaceValue = 0.0;

    unsigned int    nSpaceCount( 0 );
    unsigned int    nDiffSpaceCount( 0 );
    unsigned int    nNullSpaceBreakerCount=0;
    bool preSpaceNull(true);

    for ( unsigned int i=0; i<m_GlyphsList.size()-1; i++ ) // i=1 because the first glyph doesn't have a prevGlyphSpace value
    {
        if( m_GlyphsList[i].getPrevGlyphsSpace()>0.0 )
        {
           if( fMinPreSpaceValue>m_GlyphsList[i].getPrevGlyphsSpace() )
               fMinPreSpaceValue=m_GlyphsList[i].getPrevGlyphsSpace();

           if( fMaxPreSpaceValue<m_GlyphsList[i].getPrevGlyphsSpace() )
               fMaxPreSpaceValue=m_GlyphsList[i].getPrevGlyphsSpace();

           fPreAvarageSpaceValue+= m_GlyphsList[i].getPrevGlyphsSpace();
           nSpaceCount++;
        }
    }

    if( nSpaceCount!=0 )
     fPreAvarageSpaceValue= fPreAvarageSpaceValue/( nSpaceCount );

    for ( unsigned int i=0; i<m_GlyphsList.size()-1; i++ ) // i=1 because the first glyph doesn't have a prevGlyphSpace value
    {
       if ( m_GlyphsList[i].getPrevGlyphsSpace()==0.0 )
       {
            if (
                 ( m_GlyphsList[i+1].getPrevGlyphsSpace()>0.0)&&
                 ( fPreAvarageSpaceValue>m_GlyphsList[i+1].getPrevGlyphsSpace())
               )
            {
              fNullSpaceBreakerAvaregeSpaceValue+=m_GlyphsList[i+1].getPrevGlyphsSpace();
              nNullSpaceBreakerCount++;
            }
        }
    }

    if( ( fNullSpaceBreakerAvaregeSpaceValue!= 0.0 )&&
        ( fNullSpaceBreakerAvaregeSpaceValue < fPreAvarageSpaceValue )
      )
    {
        fPreAvarageSpaceValue = fNullSpaceBreakerAvaregeSpaceValue;
    }

    for ( unsigned int i=0; i<m_GlyphsList.size()-1; i++ ) // i=1 cose the first Glypth dont have prevGlyphSpace value
    {
        if  ( ( m_GlyphsList[i].getPrevGlyphsSpace()>0.0 )
            )
        {
          if (
              ( m_GlyphsList[i].getPrevGlyphsSpace()  <= fPreAvarageSpaceValue )&&
              ( m_GlyphsList[i+1].getPrevGlyphsSpace()<= fPreAvarageSpaceValue )
             )
          {
               double temp= m_GlyphsList[i].getPrevGlyphsSpace()-m_GlyphsList[i+1].getPrevGlyphsSpace();

               if(temp!=0.0)
               {
                 if( temp< 0.0)
                  temp= temp* -1.0;

                 fAvarageDiffCharSpaceValue+=temp;
                 nDiffSpaceCount++;
               }
          }
        }

    }

    if (
         ( nNullSpaceBreakerCount>0 )
       )
    {
       fNullSpaceBreakerAvaregeSpaceValue=fNullSpaceBreakerAvaregeSpaceValue/nNullSpaceBreakerCount;
    }

    if (
         ( nDiffSpaceCount>0 )&&(fAvarageDiffCharSpaceValue>0)
       )
    {
        fAvarageDiffCharSpaceValue= fAvarageDiffCharSpaceValue/ nDiffSpaceCount;
    }

    ParagraphElement* pPara= NULL ;
    FrameElement* pFrame= NULL ;

    if(!m_GlyphsList.empty())
    {
        pFrame = m_pElFactory->createFrameElement( m_GlyphsList[0].getCurElement(), getGCId( getTransformGlyphContext( m_GlyphsList[0])) );
        pFrame->ZOrder = m_nNextZOrder++;
        pPara = m_pElFactory->createParagraphElement( pFrame );

        processGlyph( 0,
                  m_GlyphsList[0],
                  pPara,
                  pFrame,
                  m_bIsWhiteSpaceInLine );
    }


    preSpaceNull=false;

    for ( unsigned int i=1; i<m_GlyphsList.size()-1; i++ )
    {
        double fPrevDiffCharSpace= m_GlyphsList[i].getPrevGlyphsSpace()-m_GlyphsList[i-1].getPrevGlyphsSpace();
        double fPostDiffCharSpace= m_GlyphsList[i].getPrevGlyphsSpace()-m_GlyphsList[i+1].getPrevGlyphsSpace();


         if(
             preSpaceNull && (m_GlyphsList[i].getPrevGlyphsSpace()!= 0.0)
            )
         {
               preSpaceNull=false;
              if( fNullSpaceBreakerAvaregeSpaceValue > m_GlyphsList[i].getPrevGlyphsSpace() )
              {
                processGlyph( 0,
                                      m_GlyphsList[i],
                              pPara,
                              pFrame,
                              m_bIsWhiteSpaceInLine );

              }
              else
              {
                processGlyph( 1,
                              m_GlyphsList[i],
                              pPara,
                              pFrame,
                              m_bIsWhiteSpaceInLine );

              }

         }
         else
         {
            if (
                ( ( m_GlyphsList[i].getPrevGlyphsSpace()<= fPreAvarageSpaceValue )&&
                  ( fPrevDiffCharSpace<=fAvarageDiffCharSpaceValue )&&
                  ( fPostDiffCharSpace<=fAvarageDiffCharSpaceValue )
                ) ||
                ( m_GlyphsList[i].getPrevGlyphsSpace() == 0.0 )
            )
            {
                preSpaceNull=true;

            processGlyph( 0,
                        m_GlyphsList[i],
                        pPara,
                        pFrame,
                        m_bIsWhiteSpaceInLine );

            }
            else
            {
                processGlyph( 1,
                        m_GlyphsList[i],
                        pPara,
                        pFrame,
                        m_bIsWhiteSpaceInLine );

            }

         }

    }

    if(m_GlyphsList.size()>1)
     processGlyph( 0,
                  m_GlyphsList[m_GlyphsList.size()-1],
                  pPara,
                  pFrame,
                  m_bIsWhiteSpaceInLine );

    m_GlyphsList.clear();
}

void PDFIProcessor::processGlyph( double       fPreAvarageSpaceValue,
                                  CharGlyph&   aGlyph,
                                  ParagraphElement* pPara,
                                  FrameElement* pFrame,
                                  bool         bIsWhiteSpaceInLine
                                      )
{
    if( !bIsWhiteSpaceInLine )
    {
        bool flag=( 0 < fPreAvarageSpaceValue );

        drawCharGlyphs(  aGlyph.getGlyph(),
                         aGlyph.getRect(),
                         aGlyph.getGC(),
                         pPara,
                         pFrame,
                         flag);
    }
    else
    {
        drawCharGlyphs( aGlyph.getGlyph(),
                        aGlyph.getRect(),
                        aGlyph.getGC(),
                        pPara,
                        pFrame,
                        false );
    }
}

void PDFIProcessor::drawGlyphLine( const rtl::OUString&             rGlyphs,
                                   const geometry::RealRectangle2D& rRect,
                                   const geometry::Matrix2D&        rFontMatrix )
{
    double isFirstLine= fYPrevTextPosition+ fXPrevTextPosition+ fPrevTextHeight+ fPrevTextWidth ;
    if(
        (  ( ( fYPrevTextPosition!= rRect.Y1 ) ) ||
           ( ( fXPrevTextPosition > rRect.X2 ) ) ||
           ( ( fXPrevTextPosition+fPrevTextWidth*1.3)<rRect.X1 )
        )  && ( isFirstLine> 0.0 )
    )
    {
        processGlyphLine();
    }

    CharGlyph aGlyph(fXPrevTextPosition, fYPrevTextPosition, fPrevTextHeight, fPrevTextWidth,
               m_pCurElement, getCurrentContext(), rFontMatrix, rRect, rGlyphs);


    getGCId(getCurrentContext());

    m_GlyphsList.push_back( aGlyph );

    fYPrevTextPosition  = rRect.Y1;
    fXPrevTextPosition  = rRect.X2;
    fPrevTextHeight     = rRect.Y2-rRect.Y1;
    fPrevTextWidth      = rRect.X2-rRect.X1;

    if( !m_bIsWhiteSpaceInLine )
    {
        static rtl::OUString tempWhiteSpaceStr( 0x20 );
        static rtl::OUString tempWhiteSpaceNonBreakingStr( 0xa0 );
        m_bIsWhiteSpaceInLine=(rGlyphs.equals( tempWhiteSpaceStr ) || rGlyphs.equals( tempWhiteSpaceNonBreakingStr ));
    }
}

GraphicsContext& PDFIProcessor::getTransformGlyphContext( CharGlyph& rGlyph )
{
    geometry::RealRectangle2D   rRect = rGlyph.getRect();
    geometry::Matrix2D          rFontMatrix = rGlyph.getFontMatrix();

    basegfx::B2DHomMatrix aFontMatrix;
    basegfx::unotools::homMatrixFromMatrix(
        aFontMatrix,
        rFontMatrix );

    FontAttributes aFontAttrs = m_aIdToFont[ rGlyph.getGC().FontId ];

    // add transformation to GC
    basegfx::B2DHomMatrix aFontTransform(basegfx::tools::createTranslateB2DHomMatrix(-rRect.X1, -rRect.Y1));
    aFontTransform *= aFontMatrix;
    aFontTransform.translate( rRect.X1, rRect.Y1 );


    rGlyph.getGC().Transformation = rGlyph.getGC().Transformation * aFontTransform;
    getGCId(rGlyph.getGC());

  return rGlyph.getGC();
}

void PDFIProcessor::drawCharGlyphs( rtl::OUString&             rGlyphs,
                                    geometry::RealRectangle2D& rRect,
                                    const GraphicsContext& aGC,
                                    ParagraphElement* pPara,
                                    FrameElement* pFrame,
                                    bool bSpaceFlag )
{


    rtl::OUString tempStr( 32 );
    geometry::RealRectangle2D aRect(rRect);

    ::basegfx::B2DRange aRect2;
    calcTransformedRectBounds( aRect2,
                                              ::basegfx::unotools::b2DRectangleFromRealRectangle2D(aRect),
                                              aGC.Transformation );
   // check whether there was a previous draw frame

    TextElement* pText = m_pElFactory->createTextElement( pPara,
                                                          getGCId(aGC),
                                                          aGC.FontId );
    if( bSpaceFlag )
        pText->Text.append( tempStr );

    pText->Text.append( rGlyphs );

    pText->x = aRect2.getMinX() ;
    pText->y = aRect2.getMinY() ;
    pText->w = 0.0;  // ToDO P2: 1.1 is a hack for solving of size auto-grow problem
    pText->h = aRect2.getHeight(); // ToDO P2: 1.1 is a hack for solving of size auto-grow problem

    pPara->updateGeometryWith( pText );

    if( pFrame )
      pFrame->updateGeometryWith( pPara );

}

void PDFIProcessor::drawGlyphs( const rtl::OUString&             rGlyphs,
                                const geometry::RealRectangle2D& rRect,
                                const geometry::Matrix2D&        rFontMatrix )
{
     drawGlyphLine( rGlyphs, rRect, rFontMatrix );
}

void PDFIProcessor::endText()
{
    TextElement* pText = dynamic_cast<TextElement*>(m_pCurElement);
    if( pText )
        m_pCurElement = pText->Parent;
}

void PDFIProcessor::setupImage(ImageId nImage)
{
    const GraphicsContext& rGC( getCurrentContext() );

    basegfx::B2DHomMatrix aTrans( rGC.Transformation );

    // check for rotation, which is the other way around in ODF
    basegfx::B2DTuple aScale, aTranslation;
    double fRotate, fShearX;
    rGC.Transformation.decompose( aScale, aTranslation, fRotate, fShearX );
    // TODDO(F4): correcting rotation when fShearX != 0 ?
    if( fRotate != 0.0 )
    {

        // try to create a Transformation that corrects for the wrong rotation
        aTrans.identity();
        aTrans.scale( aScale.getX(), aScale.getY() );
        aTrans.rotate( -fRotate );

        basegfx::B2DRange aRect( 0, 0, 1, 1 );
        aRect.transform( aTrans );

        // TODO(F3) treat translation correctly
        // the corrections below work for multiples of 90 degree
        // which is a common case (landscape/portrait/seascape)
        // we need a general solution here; however this needs to
        // work in sync with DrawXmlEmitter::fillFrameProps and WriterXmlEmitter::fillFrameProps
        // admittedly this is a lame workaround and fails for arbitrary rotation
        double fQuadrant = fmod( fRotate, 2.0*M_PI ) / M_PI_2;
        int nQuadrant = (int)fQuadrant;
        if( nQuadrant < 0 )
            nQuadrant += 4;
        if( nQuadrant == 1 )
        {
            aTranslation.setX( aTranslation.getX() + aRect.getHeight() + aRect.getWidth());
            aTranslation.setY( aTranslation.getY() + aRect.getHeight() );
        }
        if( nQuadrant == 3 )
            aTranslation.setX( aTranslation.getX() - aRect.getHeight() );

        aTrans.translate( aTranslation.getX(),
                          aTranslation.getY() );
    }

    bool bMirrorVertical = aScale.getY() > 0;

    // transform unit rect to determine view box
    basegfx::B2DRange aRect( 0, 0, 1, 1 );
    aRect.transform( aTrans );

    // TODO(F3): Handle clip
    const sal_Int32 nGCId = getGCId(rGC);
    FrameElement* pFrame = m_pElFactory->createFrameElement( m_pCurElement, nGCId );
    ImageElement* pImageElement = m_pElFactory->createImageElement( pFrame, nGCId, nImage );
    pFrame->x = pImageElement->x = aRect.getMinX();
    pFrame->y = pImageElement->y = aRect.getMinY();
    pFrame->w = pImageElement->w = aRect.getWidth();
    pFrame->h = pImageElement->h = aRect.getHeight();
    pFrame->ZOrder = m_nNextZOrder++;

    if( bMirrorVertical )
    {
        pFrame->MirrorVertical = pImageElement->MirrorVertical = true;
        pFrame->x        += aRect.getWidth();
        pImageElement->x += aRect.getWidth();
        pFrame->y        += aRect.getHeight();
        pImageElement->y += aRect.getHeight();
    }
}

void PDFIProcessor::drawMask(const uno::Sequence<beans::PropertyValue>& xBitmap,
                             bool                                       /*bInvert*/ )
{
    // TODO(F3): Handle mask and inversion
    setupImage( m_aImages.addImage(xBitmap) );
}

void PDFIProcessor::drawImage(const uno::Sequence<beans::PropertyValue>& xBitmap )
{
    setupImage( m_aImages.addImage(xBitmap) );
}

void PDFIProcessor::drawColorMaskedImage(const uno::Sequence<beans::PropertyValue>& xBitmap,
                                         const uno::Sequence<uno::Any>&             /*xMaskColors*/ )
{
    // TODO(F3): Handle mask colors
    setupImage( m_aImages.addImage(xBitmap) );
}

void PDFIProcessor::drawMaskedImage(const uno::Sequence<beans::PropertyValue>& xBitmap,
                                    const uno::Sequence<beans::PropertyValue>& /*xMask*/,
                                    bool                                       /*bInvertMask*/)
{
    // TODO(F3): Handle mask and inversion
    setupImage( m_aImages.addImage(xBitmap) );
}

void PDFIProcessor::drawAlphaMaskedImage(const uno::Sequence<beans::PropertyValue>& xBitmap,
                                         const uno::Sequence<beans::PropertyValue>& /*xMask*/)
{
    // TODO(F3): Handle mask

    setupImage( m_aImages.addImage(xBitmap) );

}

void PDFIProcessor::strokePath( const uno::Reference< rendering::XPolyPolygon2D >& rPath )
{
    basegfx::B2DPolyPolygon aPoly=basegfx::unotools::b2DPolyPolygonFromXPolyPolygon2D(rPath);
    aPoly.transform(getCurrentContext().Transformation);

    PolyPolyElement* pPoly = m_pElFactory->createPolyPolyElement(
        m_pCurElement,
        getGCId(getCurrentContext()),
        aPoly,
        PATH_STROKE );
    pPoly->updateGeometry();
    pPoly->ZOrder = m_nNextZOrder++;
}

void PDFIProcessor::fillPath( const uno::Reference< rendering::XPolyPolygon2D >& rPath )
{
    basegfx::B2DPolyPolygon aPoly=basegfx::unotools::b2DPolyPolygonFromXPolyPolygon2D(rPath);
    aPoly.transform(getCurrentContext().Transformation);

    PolyPolyElement* pPoly = m_pElFactory->createPolyPolyElement(
        m_pCurElement,
        getGCId(getCurrentContext()),
        aPoly,
        PATH_FILL );
    pPoly->updateGeometry();
    pPoly->ZOrder = m_nNextZOrder++;
}

void PDFIProcessor::eoFillPath( const uno::Reference< rendering::XPolyPolygon2D >& rPath )
{
    basegfx::B2DPolyPolygon aPoly=basegfx::unotools::b2DPolyPolygonFromXPolyPolygon2D(rPath);
    aPoly.transform(getCurrentContext().Transformation);

    PolyPolyElement* pPoly = m_pElFactory->createPolyPolyElement(
        m_pCurElement,
        getGCId(getCurrentContext()),
        aPoly,
        PATH_EOFILL );
    pPoly->updateGeometry();
    pPoly->ZOrder = m_nNextZOrder++;
}

void PDFIProcessor::intersectClip(const uno::Reference< rendering::XPolyPolygon2D >& rPath)
{
    // TODO(F3): interpret fill mode
    basegfx::B2DPolyPolygon aNewClip = basegfx::unotools::b2DPolyPolygonFromXPolyPolygon2D(rPath);
    aNewClip.transform(getCurrentContext().Transformation);
    basegfx::B2DPolyPolygon aCurClip = getCurrentContext().Clip;

    if( aCurClip.count() )  // #i92985# adapted API from (..., false, false) to (..., true, false)
        aNewClip = basegfx::tools::clipPolyPolygonOnPolyPolygon( aCurClip, aNewClip, true, false );

    getCurrentContext().Clip = aNewClip;
}

void PDFIProcessor::intersectEoClip(const uno::Reference< rendering::XPolyPolygon2D >& rPath)
{
    // TODO(F3): interpret fill mode
    basegfx::B2DPolyPolygon aNewClip = basegfx::unotools::b2DPolyPolygonFromXPolyPolygon2D(rPath);
    aNewClip.transform(getCurrentContext().Transformation);
    basegfx::B2DPolyPolygon aCurClip = getCurrentContext().Clip;

    if( aCurClip.count() )  // #i92985# adapted API from (..., false, false) to (..., true, false)
        aNewClip = basegfx::tools::clipPolyPolygonOnPolyPolygon( aCurClip, aNewClip, true, false );

    getCurrentContext().Clip = aNewClip;
}

void PDFIProcessor::hyperLink( const geometry::RealRectangle2D& rBounds,
                               const ::rtl::OUString&           rURI )
{
    if( !rURI.isEmpty() )
    {
        HyperlinkElement* pLink = m_pElFactory->createHyperlinkElement(
            &m_pCurPage->Hyperlinks,
            rURI );
        pLink->x = rBounds.X1;
        pLink->y = rBounds.Y1;
        pLink->w = rBounds.X2-rBounds.X1;
        pLink->h = rBounds.Y2-rBounds.Y1;
    }
}

const FontAttributes& PDFIProcessor::getFont( sal_Int32 nFontId ) const
{
    IdToFontMap::const_iterator it = m_aIdToFont.find( nFontId );
    if( it == m_aIdToFont.end() )
        it = m_aIdToFont.find( 0 );
    return it->second;
}

sal_Int32 PDFIProcessor::getGCId( const GraphicsContext& rGC )
{
    sal_Int32 nGCId = 0;
    GCToIdMap::const_iterator it = m_aGCToId.find( rGC );
    if( it != m_aGCToId.end() )
        nGCId = it->second;
    else
    {
        m_aGCToId[ rGC ] = m_nNextGCId;
        m_aIdToGC[ m_nNextGCId ] = rGC;
        nGCId = m_nNextGCId;
        m_nNextGCId++;
    }

    return nGCId;
}

const GraphicsContext& PDFIProcessor::getGraphicsContext( sal_Int32 nGCId ) const
{
    IdToGCMap::const_iterator it = m_aIdToGC.find( nGCId );
    if( it == m_aIdToGC.end() )
        it = m_aIdToGC.find( 0 );
    return it->second;
}

void PDFIProcessor::endPage()
{
    processGlyphLine(); // draw last line
    if( m_xStatusIndicator.is()
        && m_pCurPage
        && m_pCurPage->PageNumber == m_nPages
    )
        m_xStatusIndicator->end();
}

void PDFIProcessor::startPage( const geometry::RealSize2D& rSize )
{
    // initial clip is to page bounds
    getCurrentContext().Clip = basegfx::B2DPolyPolygon(
        basegfx::tools::createPolygonFromRect(
            basegfx::B2DRange( 0, 0, rSize.Width, rSize.Height )));

    sal_Int32 nNextPageNr = m_pCurPage ? m_pCurPage->PageNumber+1 : 1;
    if( m_xStatusIndicator.is() )
    {
        if( nNextPageNr == 1 )
            startIndicator( rtl::OUString( " " ) );
        m_xStatusIndicator->setValue( nNextPageNr );
    }
    m_pCurPage = m_pElFactory->createPageElement(m_pDocument.get(), nNextPageNr);
    m_pCurElement = m_pCurPage;
    m_pCurPage->w = rSize.Width;
    m_pCurPage->h = rSize.Height;
    m_nNextZOrder = 1;


}

void PDFIProcessor::emit( XmlEmitter&               rEmitter,
                          const TreeVisitorFactory& rVisitorFactory )
{
#if OSL_DEBUG_LEVEL > 1
    m_pDocument->emitStructure( 0 );
#endif

    ElementTreeVisitorSharedPtr optimizingVisitor(
        rVisitorFactory.createOptimizingVisitor(*this));
    // FIXME: localization
    startIndicator( rtl::OUString( " " ) );
    m_pDocument->visitedBy( *optimizingVisitor, std::list<Element*>::const_iterator());

#if OSL_DEBUG_LEVEL > 1
    m_pDocument->emitStructure( 0 );
#endif

    // get styles
    StyleContainer aStyles;
    ElementTreeVisitorSharedPtr finalizingVisitor(
        rVisitorFactory.createStyleCollectingVisitor(aStyles,*this));
    // FIXME: localization

    m_pDocument->visitedBy( *finalizingVisitor, std::list<Element*>::const_iterator() );

    EmitContext aContext( rEmitter, aStyles, m_aImages, *this, m_xStatusIndicator, m_xContext );
    ElementTreeVisitorSharedPtr aEmittingVisitor(
        rVisitorFactory.createEmittingVisitor(aContext, *this));

    PropertyMap aProps;
    // document prolog
    #define OASIS_STR "urn:oasis:names:tc:opendocument:xmlns:"
    aProps[ "xmlns:office" ]      = OASIS_STR "office:1.0" ;
    aProps[ "xmlns:style" ]       = OASIS_STR "style:1.0" ;
    aProps[ "xmlns:text" ]        = OASIS_STR "text:1.0" ;
    aProps[ "xmlns:svg" ]         = OASIS_STR "svg-compatible:1.0" ;
    aProps[ "xmlns:table" ]       = OASIS_STR "table:1.0" ;
    aProps[ "xmlns:draw" ]        = OASIS_STR "drawing:1.0" ;
    aProps[ "xmlns:fo" ]          = OASIS_STR "xsl-fo-compatible:1.0" ;
    aProps[ "xmlns:xlink"]        = "http://www.w3.org/1999/xlink";
    aProps[ "xmlns:dc"]           = "http://purl.org/dc/elements/1.1/";
    aProps[ "xmlns:number"]       = OASIS_STR "datastyle:1.0" ;
    aProps[ "xmlns:presentation"] = OASIS_STR "presentation:1.0" ;
    aProps[ "xmlns:math"]         = "http://www.w3.org/1998/Math/MathML";
    aProps[ "xmlns:form"]         = OASIS_STR "form:1.0" ;
    aProps[ "xmlns:script"]       = OASIS_STR "script:1.0" ;
    aProps[ "xmlns:dom"]          = "http://www.w3.org/2001/xml-events";
    aProps[ "xmlns:xforms"]       = "http://www.w3.org/2002/xforms";
    aProps[ "xmlns:xsd"]          = "http://www.w3.org/2001/XMLSchema";
    aProps[ "xmlns:xsi"]          = "http://www.w3.org/2001/XMLSchema-instance";
    aProps[ "office:version" ]    = "1.0";
    aProps[ "office:version" ]    = "1.0";

    aContext.rEmitter.beginTag( "office:document", aProps );

    // emit style list
    aStyles.emit( aContext, *aEmittingVisitor );

    m_pDocument->visitedBy( *aEmittingVisitor, std::list<Element*>::const_iterator() );
    aContext.rEmitter.endTag( "office:document" );
    endIndicator();
}

void PDFIProcessor::startIndicator( const rtl::OUString& rText, sal_Int32 nElements )
{
    if( nElements == -1 )
        nElements = m_nPages;
    if( m_xStatusIndicator.is() )
    {
        sal_Int32 nUnicodes = rText.getLength();
        rtl::OUStringBuffer aStr( nUnicodes*2 );
        const sal_Unicode* pText = rText.getStr();
        for( int i = 0; i < nUnicodes; i++ )
        {
            if( nUnicodes-i > 1&&
                pText[i]   == '%' &&
                pText[i+1] == 'd'
            )
            {
                aStr.append( nElements );
                i++;
            }
            else
                aStr.append( pText[i] );
        }
        m_xStatusIndicator->start( aStr.makeStringAndClear(), nElements );
    }
}

void PDFIProcessor::endIndicator()
{
    if( m_xStatusIndicator.is() )
        m_xStatusIndicator->end();
}

static bool lr_tb_sort( Element* pLeft, Element* pRight )
{
    // first: top-bottom sorting

    // Note: allow for 10% overlap on text lines since text lines are usually
    // of the same order as font height whereas the real paint area
    // of text is usually smaller
    double fudge_factor = 1.0;
    if( dynamic_cast< TextElement* >(pLeft) || dynamic_cast< TextElement* >(pRight) )
        fudge_factor = 0.9;

    // if left's lower boundary is above right's upper boundary
    // then left is smaller
    if( pLeft->y+pLeft->h*fudge_factor < pRight->y )
        return true;
    // if right's lower boundary is above left's upper boundary
    // then left is definitely not smaller
    if( pRight->y+pRight->h*fudge_factor < pLeft->y )
        return false;

    // by now we have established that left and right are inside
    // a "line", that is they have vertical overlap
    // second: left-right sorting
    // if left's right boundary is left to right's left boundary
    // then left is smaller
    if( pLeft->x+pLeft->w < pRight->x )
        return true;
    // if right's right boundary is left to left's left boundary
    // then left is definitely not smaller
    if( pRight->x+pRight->w < pLeft->x )
        return false;

    // here we have established vertical and horizontal overlap
    // so sort left first, top second
    if( pLeft->x < pRight->x )
        return true;
    if( pRight->x < pLeft->x )
        return false;
    if( pLeft->y < pRight->y )
        return true;

    return false;
}

void PDFIProcessor::sortElements( Element* pEle, bool bDeep )
{
    if( pEle->Children.empty() )
        return;

    if( bDeep )
    {
        for( std::list< Element* >::iterator it = pEle->Children.begin();
             it != pEle->Children.end(); ++it )
        {
            sortElements( *it, bDeep );
        }
    }
    // HACK: the stable sort member on std::list that takes a
    // strict weak ordering requires member templates - which we
    // do not have on all compilers. so we need to use std::stable_sort
    // here - which does need random access iterators which the
    // list iterators are not.
    // so we need to copy the Element* to an array, stable sort that and
    // copy them back.
    std::vector<Element*> aChildren;
    while( ! pEle->Children.empty() )
    {
        aChildren.push_back( pEle->Children.front() );
        pEle->Children.pop_front();
    }
    switch( m_eTextDirection )
    {
        case LrTb:
        default:
        std::stable_sort( aChildren.begin(), aChildren.end(), lr_tb_sort );
        break;
    }
    int nChildren = aChildren.size();
    for( int i = 0; i < nChildren; i++ )
        pEle->Children.push_back( aChildren[i] );
}


::basegfx::B2DRange& PDFIProcessor::calcTransformedRectBounds( ::basegfx::B2DRange&         outRect,
                                                        const ::basegfx::B2DRange&      inRect,
                                                        const ::basegfx::B2DHomMatrix&  transformation )
        {
            outRect.reset();

            if( inRect.isEmpty() )
                return outRect;

            // transform all four extremal points of the rectangle,
            // take bounding rect of those.

            // transform left-top point
            outRect.expand( transformation * inRect.getMinimum() );

            // transform bottom-right point
            outRect.expand( transformation * inRect.getMaximum() );

            ::basegfx::B2DPoint aPoint;

            // transform top-right point
            aPoint.setX( inRect.getMaxX() );
            aPoint.setY( inRect.getMinY() );

            aPoint *= transformation;
            outRect.expand( aPoint );

            // transform bottom-left point
            aPoint.setX( inRect.getMinX() );
            aPoint.setY( inRect.getMaxY() );

            aPoint *= transformation;
            outRect.expand( aPoint );

            // over and out.
            return outRect;
        }

// helper method: get a mirrored string
rtl::OUString PDFIProcessor::mirrorString( const rtl::OUString& i_rString ) const
{
    const sal_Int32 nLen = i_rString.getLength();
    rtl::OUStringBuffer aMirror( nLen );

    sal_Int32 i = 0;
    while(i < nLen)
    {
        // read one code point
        const sal_uInt32 nCodePoint = i_rString.iterateCodePoints( &i );

        // and append it mirrored
        aMirror.appendUtf32( GetMirroredChar(nCodePoint) );
    }
    return aMirror.makeStringAndClear();
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
