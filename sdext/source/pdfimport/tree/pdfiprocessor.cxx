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
    m_bHaveTextOnDocLevel(false),
    m_bMirrorMapperTried(false)
{
    FontAttributes aDefFont;
    aDefFont.familyName = USTR("Helvetica");
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
    if( m_GlyphsList.size()<1 )
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

    CharGlyph aGlyph;

    aGlyph.setGlyph ( rGlyphs );
    aGlyph.setRect  ( rRect );
    aGlyph.setFontMatrix ( rFontMatrix );
    aGlyph.setGraphicsContext ( getCurrentContext() );
    getGCId(getCurrentContext());
    aGlyph.setCurElement( m_pCurElement );

    aGlyph.setYPrevGlyphPosition( fYPrevTextPosition );
    aGlyph.setXPrevGlyphPosition( fXPrevTextPosition );
    aGlyph.setPrevGlyphHeight  ( fPrevTextHeight );
    aGlyph.setPrevGlyphWidth   ( fPrevTextWidth );

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

    rtl::OUString tempStr( 32 );

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
                                    GraphicsContext aGC,
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
            startIndicator( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( " " ) ) );
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
    startIndicator( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( " " ) ) );
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
    aProps[ USTR( "xmlns:office" ) ]      = USTR( OASIS_STR "office:1.0" );
    aProps[ USTR( "xmlns:style" ) ]       = USTR( OASIS_STR "style:1.0" );
    aProps[ USTR( "xmlns:text" ) ]        = USTR( OASIS_STR "text:1.0" );
    aProps[ USTR( "xmlns:svg" ) ]         = USTR( OASIS_STR "svg-compatible:1.0" );
    aProps[ USTR( "xmlns:table" ) ]       = USTR( OASIS_STR "table:1.0" );
    aProps[ USTR( "xmlns:draw" ) ]        = USTR( OASIS_STR "drawing:1.0" );
    aProps[ USTR( "xmlns:fo" ) ]          = USTR( OASIS_STR "xsl-fo-compatible:1.0" );
    aProps[ USTR( "xmlns:xlink" )]        = USTR( "http://www.w3.org/1999/xlink" );
    aProps[ USTR( "xmlns:dc" )]           = USTR( "http://purl.org/dc/elements/1.1/" );
    aProps[ USTR( "xmlns:number" )]       = USTR( OASIS_STR "datastyle:1.0" );
    aProps[ USTR( "xmlns:presentation" )] = USTR( OASIS_STR "presentation:1.0" );
    aProps[ USTR( "xmlns:math" )]         = USTR( "http://www.w3.org/1998/Math/MathML" );
    aProps[ USTR( "xmlns:form" )]         = USTR( OASIS_STR "form:1.0" );
    aProps[ USTR( "xmlns:script" )]       = USTR( OASIS_STR "script:1.0" );
    aProps[ USTR( "xmlns:dom" )]          = USTR( "http://www.w3.org/2001/xml-events" );
    aProps[ USTR( "xmlns:xforms" )]       = USTR( "http://www.w3.org/2002/xforms" );
    aProps[ USTR( "xmlns:xsd" )]          = USTR( "http://www.w3.org/2001/XMLSchema" );
    aProps[ USTR( "xmlns:xsi" )]          = USTR( "http://www.w3.org/2001/XMLSchema-instance" );
    aProps[ USTR( "office:version" ) ]    = USTR( "1.0" );
    aProps[ USTR( "office:version" ) ]    = USTR( "1.0" );

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
rtl::OUString PDFIProcessor::mirrorString( const rtl::OUString& i_rString )
{
    if( ! m_xMirrorMapper.is() && ! m_bMirrorMapperTried )
    {
        m_bMirrorMapperTried = true;
        uno::Reference< lang::XMultiComponentFactory > xMSF(  m_xContext->getServiceManager(), uno::UNO_SET_THROW );
        uno::Reference < uno::XInterface > xInterface = xMSF->createInstanceWithContext(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.awt.StringMirror")), m_xContext);
        m_xMirrorMapper = uno::Reference< util::XStringMapping >( xInterface, uno::UNO_QUERY );
        #if OSL_DEBUG_LEVEL > 1
        if( m_xMirrorMapper.is() )
            fprintf( stderr, "using mirror mapper service\n" );
        #endif
    }
    if( m_xMirrorMapper.is() )
    {
        uno::Sequence< rtl::OUString > aSeq( 1 );
        aSeq.getArray()[0] = i_rString;
        m_xMirrorMapper->mapStrings( aSeq );
        return  aSeq[0];
    }

    prepareMirrorMap();
    sal_Int32 nLen = i_rString.getLength();
    rtl::OUStringBuffer aRet( nLen );
    for(int i = nLen - 1; i >= 0; i--)
    {
        sal_Unicode cChar = i_rString[ i ];
        aRet.append( m_aMirrorMap[cChar] );
    }
    return aRet.makeStringAndClear();
}

void PDFIProcessor::prepareMirrorMap()
{
    if( m_aMirrorMap.empty() )
    {
        #if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, "falling back to static mirror list\n" );
        #endif

        m_aMirrorMap.reserve( 0x10000 );
        for( int i = 0; i < 0x10000; i++ )
            m_aMirrorMap.push_back( sal_Unicode(i) );

        m_aMirrorMap[ 0x0028 ] = 0x0029; // LEFT PARENTHESIS
        m_aMirrorMap[ 0x0029 ] = 0x0028; // RIGHT PARENTHESIS
        m_aMirrorMap[ 0x003C ] = 0x003E; // LESS-THAN SIGN
        m_aMirrorMap[ 0x003E ] = 0x003C; // GREATER-THAN SIGN
        m_aMirrorMap[ 0x005B ] = 0x005D; // LEFT SQUARE BRACKET
        m_aMirrorMap[ 0x005D ] = 0x005B; // RIGHT SQUARE BRACKET
        m_aMirrorMap[ 0x007B ] = 0x007D; // LEFT CURLY BRACKET
        m_aMirrorMap[ 0x007D ] = 0x007B; // RIGHT CURLY BRACKET
        m_aMirrorMap[ 0x00AB ] = 0x00BB; // LEFT-POINTING DOUBLE ANGLE QUOTATION MARK
        m_aMirrorMap[ 0x00BB ] = 0x00AB; // RIGHT-POINTING DOUBLE ANGLE QUOTATION MARK
        m_aMirrorMap[ 0x0F3A ] = 0x0F3B; // TIBETAN MARK GUG RTAGS GYON
        m_aMirrorMap[ 0x0F3B ] = 0x0F3A; // TIBETAN MARK GUG RTAGS GYAS
        m_aMirrorMap[ 0x0F3C ] = 0x0F3D; // TIBETAN MARK ANG KHANG GYON
        m_aMirrorMap[ 0x0F3D ] = 0x0F3C; // TIBETAN MARK ANG KHANG GYAS
        m_aMirrorMap[ 0x169B ] = 0x169C; // OGHAM FEATHER MARK
        m_aMirrorMap[ 0x169C ] = 0x169B; // OGHAM REVERSED FEATHER MARK
        m_aMirrorMap[ 0x2039 ] = 0x203A; // SINGLE LEFT-POINTING ANGLE QUOTATION MARK
        m_aMirrorMap[ 0x203A ] = 0x2039; // SINGLE RIGHT-POINTING ANGLE QUOTATION MARK
        m_aMirrorMap[ 0x2045 ] = 0x2046; // LEFT SQUARE BRACKET WITH QUILL
        m_aMirrorMap[ 0x2046 ] = 0x2045; // RIGHT SQUARE BRACKET WITH QUILL
        m_aMirrorMap[ 0x207D ] = 0x207E; // SUPERSCRIPT LEFT PARENTHESIS
        m_aMirrorMap[ 0x207E ] = 0x207D; // SUPERSCRIPT RIGHT PARENTHESIS
        m_aMirrorMap[ 0x208D ] = 0x208E; // SUBSCRIPT LEFT PARENTHESIS
        m_aMirrorMap[ 0x208E ] = 0x208D; // SUBSCRIPT RIGHT PARENTHESIS
        m_aMirrorMap[ 0x2208 ] = 0x220B; // ELEMENT OF
        m_aMirrorMap[ 0x2209 ] = 0x220C; // NOT AN ELEMENT OF
        m_aMirrorMap[ 0x220A ] = 0x220D; // SMALL ELEMENT OF
        m_aMirrorMap[ 0x220B ] = 0x2208; // CONTAINS AS MEMBER
        m_aMirrorMap[ 0x220C ] = 0x2209; // DOES NOT CONTAIN AS MEMBER
        m_aMirrorMap[ 0x220D ] = 0x220A; // SMALL CONTAINS AS MEMBER
        m_aMirrorMap[ 0x2215 ] = 0x29F5; // DIVISION SLASH
        m_aMirrorMap[ 0x223C ] = 0x223D; // TILDE OPERATOR
        m_aMirrorMap[ 0x223D ] = 0x223C; // REVERSED TILDE
        m_aMirrorMap[ 0x2243 ] = 0x22CD; // ASYMPTOTICALLY EQUAL TO
        m_aMirrorMap[ 0x2252 ] = 0x2253; // APPROXIMATELY EQUAL TO OR THE IMAGE OF
        m_aMirrorMap[ 0x2253 ] = 0x2252; // IMAGE OF OR APPROXIMATELY EQUAL TO
        m_aMirrorMap[ 0x2254 ] = 0x2255; // COLON EQUALS
        m_aMirrorMap[ 0x2255 ] = 0x2254; // EQUALS COLON
        m_aMirrorMap[ 0x2264 ] = 0x2265; // LESS-THAN OR EQUAL TO
        m_aMirrorMap[ 0x2265 ] = 0x2264; // GREATER-THAN OR EQUAL TO
        m_aMirrorMap[ 0x2266 ] = 0x2267; // LESS-THAN OVER EQUAL TO
        m_aMirrorMap[ 0x2267 ] = 0x2266; // GREATER-THAN OVER EQUAL TO
        m_aMirrorMap[ 0x2268 ] = 0x2269; // [BEST FIT] LESS-THAN BUT NOT EQUAL TO
        m_aMirrorMap[ 0x2269 ] = 0x2268; // [BEST FIT] GREATER-THAN BUT NOT EQUAL TO
        m_aMirrorMap[ 0x226A ] = 0x226B; // MUCH LESS-THAN
        m_aMirrorMap[ 0x226B ] = 0x226A; // MUCH GREATER-THAN
        m_aMirrorMap[ 0x226E ] = 0x226F; // [BEST FIT] NOT LESS-THAN
        m_aMirrorMap[ 0x226F ] = 0x226E; // [BEST FIT] NOT GREATER-THAN
        m_aMirrorMap[ 0x2270 ] = 0x2271; // [BEST FIT] NEITHER LESS-THAN NOR EQUAL TO
        m_aMirrorMap[ 0x2271 ] = 0x2270; // [BEST FIT] NEITHER GREATER-THAN NOR EQUAL TO
        m_aMirrorMap[ 0x2272 ] = 0x2273; // [BEST FIT] LESS-THAN OR EQUIVALENT TO
        m_aMirrorMap[ 0x2273 ] = 0x2272; // [BEST FIT] GREATER-THAN OR EQUIVALENT TO
        m_aMirrorMap[ 0x2274 ] = 0x2275; // [BEST FIT] NEITHER LESS-THAN NOR EQUIVALENT TO
        m_aMirrorMap[ 0x2275 ] = 0x2274; // [BEST FIT] NEITHER GREATER-THAN NOR EQUIVALENT TO
        m_aMirrorMap[ 0x2276 ] = 0x2277; // LESS-THAN OR GREATER-THAN
        m_aMirrorMap[ 0x2277 ] = 0x2276; // GREATER-THAN OR LESS-THAN
        m_aMirrorMap[ 0x2278 ] = 0x2279; // [BEST FIT] NEITHER LESS-THAN NOR GREATER-THAN
        m_aMirrorMap[ 0x2279 ] = 0x2278; // [BEST FIT] NEITHER GREATER-THAN NOR LESS-THAN
        m_aMirrorMap[ 0x227A ] = 0x227B; // PRECEDES
        m_aMirrorMap[ 0x227B ] = 0x227A; // SUCCEEDS
        m_aMirrorMap[ 0x227C ] = 0x227D; // PRECEDES OR EQUAL TO
        m_aMirrorMap[ 0x227D ] = 0x227C; // SUCCEEDS OR EQUAL TO
        m_aMirrorMap[ 0x227E ] = 0x227F; // [BEST FIT] PRECEDES OR EQUIVALENT TO
        m_aMirrorMap[ 0x227F ] = 0x227E; // [BEST FIT] SUCCEEDS OR EQUIVALENT TO
        m_aMirrorMap[ 0x2280 ] = 0x2281; // [BEST FIT] DOES NOT PRECEDE
        m_aMirrorMap[ 0x2281 ] = 0x2280; // [BEST FIT] DOES NOT SUCCEED
        m_aMirrorMap[ 0x2282 ] = 0x2283; // SUBSET OF
        m_aMirrorMap[ 0x2283 ] = 0x2282; // SUPERSET OF
        m_aMirrorMap[ 0x2284 ] = 0x2285; // [BEST FIT] NOT A SUBSET OF
        m_aMirrorMap[ 0x2285 ] = 0x2284; // [BEST FIT] NOT A SUPERSET OF
        m_aMirrorMap[ 0x2286 ] = 0x2287; // SUBSET OF OR EQUAL TO
        m_aMirrorMap[ 0x2287 ] = 0x2286; // SUPERSET OF OR EQUAL TO
        m_aMirrorMap[ 0x2288 ] = 0x2289; // [BEST FIT] NEITHER A SUBSET OF NOR EQUAL TO
        m_aMirrorMap[ 0x2289 ] = 0x2288; // [BEST FIT] NEITHER A SUPERSET OF NOR EQUAL TO
        m_aMirrorMap[ 0x228A ] = 0x228B; // [BEST FIT] SUBSET OF WITH NOT EQUAL TO
        m_aMirrorMap[ 0x228B ] = 0x228A; // [BEST FIT] SUPERSET OF WITH NOT EQUAL TO
        m_aMirrorMap[ 0x228F ] = 0x2290; // SQUARE IMAGE OF
        m_aMirrorMap[ 0x2290 ] = 0x228F; // SQUARE ORIGINAL OF
        m_aMirrorMap[ 0x2291 ] = 0x2292; // SQUARE IMAGE OF OR EQUAL TO
        m_aMirrorMap[ 0x2292 ] = 0x2291; // SQUARE ORIGINAL OF OR EQUAL TO
        m_aMirrorMap[ 0x2298 ] = 0x29B8; // CIRCLED DIVISION SLASH
        m_aMirrorMap[ 0x22A2 ] = 0x22A3; // RIGHT TACK
        m_aMirrorMap[ 0x22A3 ] = 0x22A2; // LEFT TACK
        m_aMirrorMap[ 0x22A6 ] = 0x2ADE; // ASSERTION
        m_aMirrorMap[ 0x22A8 ] = 0x2AE4; // TRUE
        m_aMirrorMap[ 0x22A9 ] = 0x2AE3; // FORCES
        m_aMirrorMap[ 0x22AB ] = 0x2AE5; // DOUBLE VERTICAL BAR DOUBLE RIGHT TURNSTILE
        m_aMirrorMap[ 0x22B0 ] = 0x22B1; // PRECEDES UNDER RELATION
        m_aMirrorMap[ 0x22B1 ] = 0x22B0; // SUCCEEDS UNDER RELATION
        m_aMirrorMap[ 0x22B2 ] = 0x22B3; // NORMAL SUBGROUP OF
        m_aMirrorMap[ 0x22B3 ] = 0x22B2; // CONTAINS AS NORMAL SUBGROUP
        m_aMirrorMap[ 0x22B4 ] = 0x22B5; // NORMAL SUBGROUP OF OR EQUAL TO
        m_aMirrorMap[ 0x22B5 ] = 0x22B4; // CONTAINS AS NORMAL SUBGROUP OR EQUAL TO
        m_aMirrorMap[ 0x22B6 ] = 0x22B7; // ORIGINAL OF
        m_aMirrorMap[ 0x22B7 ] = 0x22B6; // IMAGE OF
        m_aMirrorMap[ 0x22C9 ] = 0x22CA; // LEFT NORMAL FACTOR SEMIDIRECT PRODUCT
        m_aMirrorMap[ 0x22CA ] = 0x22C9; // RIGHT NORMAL FACTOR SEMIDIRECT PRODUCT
        m_aMirrorMap[ 0x22CB ] = 0x22CC; // LEFT SEMIDIRECT PRODUCT
        m_aMirrorMap[ 0x22CC ] = 0x22CB; // RIGHT SEMIDIRECT PRODUCT
        m_aMirrorMap[ 0x22CD ] = 0x2243; // REVERSED TILDE EQUALS
        m_aMirrorMap[ 0x22D0 ] = 0x22D1; // DOUBLE SUBSET
        m_aMirrorMap[ 0x22D1 ] = 0x22D0; // DOUBLE SUPERSET
        m_aMirrorMap[ 0x22D6 ] = 0x22D7; // LESS-THAN WITH DOT
        m_aMirrorMap[ 0x22D7 ] = 0x22D6; // GREATER-THAN WITH DOT
        m_aMirrorMap[ 0x22D8 ] = 0x22D9; // VERY MUCH LESS-THAN
        m_aMirrorMap[ 0x22D9 ] = 0x22D8; // VERY MUCH GREATER-THAN
        m_aMirrorMap[ 0x22DA ] = 0x22DB; // LESS-THAN EQUAL TO OR GREATER-THAN
        m_aMirrorMap[ 0x22DB ] = 0x22DA; // GREATER-THAN EQUAL TO OR LESS-THAN
        m_aMirrorMap[ 0x22DC ] = 0x22DD; // EQUAL TO OR LESS-THAN
        m_aMirrorMap[ 0x22DD ] = 0x22DC; // EQUAL TO OR GREATER-THAN
        m_aMirrorMap[ 0x22DE ] = 0x22DF; // EQUAL TO OR PRECEDES
        m_aMirrorMap[ 0x22DF ] = 0x22DE; // EQUAL TO OR SUCCEEDS
        m_aMirrorMap[ 0x22E0 ] = 0x22E1; // [BEST FIT] DOES NOT PRECEDE OR EQUAL
        m_aMirrorMap[ 0x22E1 ] = 0x22E0; // [BEST FIT] DOES NOT SUCCEED OR EQUAL
        m_aMirrorMap[ 0x22E2 ] = 0x22E3; // [BEST FIT] NOT SQUARE IMAGE OF OR EQUAL TO
        m_aMirrorMap[ 0x22E3 ] = 0x22E2; // [BEST FIT] NOT SQUARE ORIGINAL OF OR EQUAL TO
        m_aMirrorMap[ 0x22E4 ] = 0x22E5; // [BEST FIT] SQUARE IMAGE OF OR NOT EQUAL TO
        m_aMirrorMap[ 0x22E5 ] = 0x22E4; // [BEST FIT] SQUARE ORIGINAL OF OR NOT EQUAL TO
        m_aMirrorMap[ 0x22E6 ] = 0x22E7; // [BEST FIT] LESS-THAN BUT NOT EQUIVALENT TO
        m_aMirrorMap[ 0x22E7 ] = 0x22E6; // [BEST FIT] GREATER-THAN BUT NOT EQUIVALENT TO
        m_aMirrorMap[ 0x22E8 ] = 0x22E9; // [BEST FIT] PRECEDES BUT NOT EQUIVALENT TO
        m_aMirrorMap[ 0x22E9 ] = 0x22E8; // [BEST FIT] SUCCEEDS BUT NOT EQUIVALENT TO
        m_aMirrorMap[ 0x22EA ] = 0x22EB; // [BEST FIT] NOT NORMAL SUBGROUP OF
        m_aMirrorMap[ 0x22EB ] = 0x22EA; // [BEST FIT] DOES NOT CONTAIN AS NORMAL SUBGROUP
        m_aMirrorMap[ 0x22EC ] = 0x22ED; // [BEST FIT] NOT NORMAL SUBGROUP OF OR EQUAL TO
        m_aMirrorMap[ 0x22ED ] = 0x22EC; // [BEST FIT] DOES NOT CONTAIN AS NORMAL SUBGROUP OR EQUAL
        m_aMirrorMap[ 0x22F0 ] = 0x22F1; // UP RIGHT DIAGONAL ELLIPSIS
        m_aMirrorMap[ 0x22F1 ] = 0x22F0; // DOWN RIGHT DIAGONAL ELLIPSIS
        m_aMirrorMap[ 0x22F2 ] = 0x22FA; // ELEMENT OF WITH LONG HORIZONTAL STROKE
        m_aMirrorMap[ 0x22F3 ] = 0x22FB; // ELEMENT OF WITH VERTICAL BAR AT END OF HORIZONTAL STROKE
        m_aMirrorMap[ 0x22F4 ] = 0x22FC; // SMALL ELEMENT OF WITH VERTICAL BAR AT END OF HORIZONTAL STROKE
        m_aMirrorMap[ 0x22F6 ] = 0x22FD; // ELEMENT OF WITH OVERBAR
        m_aMirrorMap[ 0x22F7 ] = 0x22FE; // SMALL ELEMENT OF WITH OVERBAR
        m_aMirrorMap[ 0x22FA ] = 0x22F2; // CONTAINS WITH LONG HORIZONTAL STROKE
        m_aMirrorMap[ 0x22FB ] = 0x22F3; // CONTAINS WITH VERTICAL BAR AT END OF HORIZONTAL STROKE
        m_aMirrorMap[ 0x22FC ] = 0x22F4; // SMALL CONTAINS WITH VERTICAL BAR AT END OF HORIZONTAL STROKE
        m_aMirrorMap[ 0x22FD ] = 0x22F6; // CONTAINS WITH OVERBAR
        m_aMirrorMap[ 0x22FE ] = 0x22F7; // SMALL CONTAINS WITH OVERBAR
        m_aMirrorMap[ 0x2308 ] = 0x2309; // LEFT CEILING
        m_aMirrorMap[ 0x2309 ] = 0x2308; // RIGHT CEILING
        m_aMirrorMap[ 0x230A ] = 0x230B; // LEFT FLOOR
        m_aMirrorMap[ 0x230B ] = 0x230A; // RIGHT FLOOR
        m_aMirrorMap[ 0x2329 ] = 0x232A; // LEFT-POINTING ANGLE BRACKET
        m_aMirrorMap[ 0x232A ] = 0x2329; // RIGHT-POINTING ANGLE BRACKET
        m_aMirrorMap[ 0x2768 ] = 0x2769; // MEDIUM LEFT PARENTHESIS ORNAMENT
        m_aMirrorMap[ 0x2769 ] = 0x2768; // MEDIUM RIGHT PARENTHESIS ORNAMENT
        m_aMirrorMap[ 0x276A ] = 0x276B; // MEDIUM FLATTENED LEFT PARENTHESIS ORNAMENT
        m_aMirrorMap[ 0x276B ] = 0x276A; // MEDIUM FLATTENED RIGHT PARENTHESIS ORNAMENT
        m_aMirrorMap[ 0x276C ] = 0x276D; // MEDIUM LEFT-POINTING ANGLE BRACKET ORNAMENT
        m_aMirrorMap[ 0x276D ] = 0x276C; // MEDIUM RIGHT-POINTING ANGLE BRACKET ORNAMENT
        m_aMirrorMap[ 0x276E ] = 0x276F; // HEAVY LEFT-POINTING ANGLE QUOTATION MARK ORNAMENT
        m_aMirrorMap[ 0x276F ] = 0x276E; // HEAVY RIGHT-POINTING ANGLE QUOTATION MARK ORNAMENT
        m_aMirrorMap[ 0x2770 ] = 0x2771; // HEAVY LEFT-POINTING ANGLE BRACKET ORNAMENT
        m_aMirrorMap[ 0x2771 ] = 0x2770; // HEAVY RIGHT-POINTING ANGLE BRACKET ORNAMENT
        m_aMirrorMap[ 0x2772 ] = 0x2773; // LIGHT LEFT TORTOISE SHELL BRACKET
        m_aMirrorMap[ 0x2773 ] = 0x2772; // LIGHT RIGHT TORTOISE SHELL BRACKET
        m_aMirrorMap[ 0x2774 ] = 0x2775; // MEDIUM LEFT CURLY BRACKET ORNAMENT
        m_aMirrorMap[ 0x2775 ] = 0x2774; // MEDIUM RIGHT CURLY BRACKET ORNAMENT
        m_aMirrorMap[ 0x27C3 ] = 0x27C4; // OPEN SUBSET
        m_aMirrorMap[ 0x27C4 ] = 0x27C3; // OPEN SUPERSET
        m_aMirrorMap[ 0x27C5 ] = 0x27C6; // LEFT S-SHAPED BAG DELIMITER
        m_aMirrorMap[ 0x27C6 ] = 0x27C5; // RIGHT S-SHAPED BAG DELIMITER
        m_aMirrorMap[ 0x27C8 ] = 0x27C9; // REVERSE SOLIDUS PRECEDING SUBSET
        m_aMirrorMap[ 0x27C9 ] = 0x27C8; // SUPERSET PRECEDING SOLIDUS
        m_aMirrorMap[ 0x27D5 ] = 0x27D6; // LEFT OUTER JOIN
        m_aMirrorMap[ 0x27D6 ] = 0x27D5; // RIGHT OUTER JOIN
        m_aMirrorMap[ 0x27DD ] = 0x27DE; // LONG RIGHT TACK
        m_aMirrorMap[ 0x27DE ] = 0x27DD; // LONG LEFT TACK
        m_aMirrorMap[ 0x27E2 ] = 0x27E3; // WHITE CONCAVE-SIDED DIAMOND WITH LEFTWARDS TICK
        m_aMirrorMap[ 0x27E3 ] = 0x27E2; // WHITE CONCAVE-SIDED DIAMOND WITH RIGHTWARDS TICK
        m_aMirrorMap[ 0x27E4 ] = 0x27E5; // WHITE SQUARE WITH LEFTWARDS TICK
        m_aMirrorMap[ 0x27E5 ] = 0x27E4; // WHITE SQUARE WITH RIGHTWARDS TICK
        m_aMirrorMap[ 0x27E6 ] = 0x27E7; // MATHEMATICAL LEFT WHITE SQUARE BRACKET
        m_aMirrorMap[ 0x27E7 ] = 0x27E6; // MATHEMATICAL RIGHT WHITE SQUARE BRACKET
        m_aMirrorMap[ 0x27E8 ] = 0x27E9; // MATHEMATICAL LEFT ANGLE BRACKET
        m_aMirrorMap[ 0x27E9 ] = 0x27E8; // MATHEMATICAL RIGHT ANGLE BRACKET
        m_aMirrorMap[ 0x27EA ] = 0x27EB; // MATHEMATICAL LEFT DOUBLE ANGLE BRACKET
        m_aMirrorMap[ 0x27EB ] = 0x27EA; // MATHEMATICAL RIGHT DOUBLE ANGLE BRACKET
        m_aMirrorMap[ 0x27EC ] = 0x27ED; // MATHEMATICAL LEFT WHITE TORTOISE SHELL BRACKET
        m_aMirrorMap[ 0x27ED ] = 0x27EC; // MATHEMATICAL RIGHT WHITE TORTOISE SHELL BRACKET
        m_aMirrorMap[ 0x27EE ] = 0x27EF; // MATHEMATICAL LEFT FLATTENED PARENTHESIS
        m_aMirrorMap[ 0x27EF ] = 0x27EE; // MATHEMATICAL RIGHT FLATTENED PARENTHESIS
        m_aMirrorMap[ 0x2983 ] = 0x2984; // LEFT WHITE CURLY BRACKET
        m_aMirrorMap[ 0x2984 ] = 0x2983; // RIGHT WHITE CURLY BRACKET
        m_aMirrorMap[ 0x2985 ] = 0x2986; // LEFT WHITE PARENTHESIS
        m_aMirrorMap[ 0x2986 ] = 0x2985; // RIGHT WHITE PARENTHESIS
        m_aMirrorMap[ 0x2987 ] = 0x2988; // Z NOTATION LEFT IMAGE BRACKET
        m_aMirrorMap[ 0x2988 ] = 0x2987; // Z NOTATION RIGHT IMAGE BRACKET
        m_aMirrorMap[ 0x2989 ] = 0x298A; // Z NOTATION LEFT BINDING BRACKET
        m_aMirrorMap[ 0x298A ] = 0x2989; // Z NOTATION RIGHT BINDING BRACKET
        m_aMirrorMap[ 0x298B ] = 0x298C; // LEFT SQUARE BRACKET WITH UNDERBAR
        m_aMirrorMap[ 0x298C ] = 0x298B; // RIGHT SQUARE BRACKET WITH UNDERBAR
        m_aMirrorMap[ 0x298D ] = 0x2990; // LEFT SQUARE BRACKET WITH TICK IN TOP CORNER
        m_aMirrorMap[ 0x298E ] = 0x298F; // RIGHT SQUARE BRACKET WITH TICK IN BOTTOM CORNER
        m_aMirrorMap[ 0x298F ] = 0x298E; // LEFT SQUARE BRACKET WITH TICK IN BOTTOM CORNER
        m_aMirrorMap[ 0x2990 ] = 0x298D; // RIGHT SQUARE BRACKET WITH TICK IN TOP CORNER
        m_aMirrorMap[ 0x2991 ] = 0x2992; // LEFT ANGLE BRACKET WITH DOT
        m_aMirrorMap[ 0x2992 ] = 0x2991; // RIGHT ANGLE BRACKET WITH DOT
        m_aMirrorMap[ 0x2993 ] = 0x2994; // LEFT ARC LESS-THAN BRACKET
        m_aMirrorMap[ 0x2994 ] = 0x2993; // RIGHT ARC GREATER-THAN BRACKET
        m_aMirrorMap[ 0x2995 ] = 0x2996; // DOUBLE LEFT ARC GREATER-THAN BRACKET
        m_aMirrorMap[ 0x2996 ] = 0x2995; // DOUBLE RIGHT ARC LESS-THAN BRACKET
        m_aMirrorMap[ 0x2997 ] = 0x2998; // LEFT BLACK TORTOISE SHELL BRACKET
        m_aMirrorMap[ 0x2998 ] = 0x2997; // RIGHT BLACK TORTOISE SHELL BRACKET
        m_aMirrorMap[ 0x29B8 ] = 0x2298; // CIRCLED REVERSE SOLIDUS
        m_aMirrorMap[ 0x29C0 ] = 0x29C1; // CIRCLED LESS-THAN
        m_aMirrorMap[ 0x29C1 ] = 0x29C0; // CIRCLED GREATER-THAN
        m_aMirrorMap[ 0x29C4 ] = 0x29C5; // SQUARED RISING DIAGONAL SLASH
        m_aMirrorMap[ 0x29C5 ] = 0x29C4; // SQUARED FALLING DIAGONAL SLASH
        m_aMirrorMap[ 0x29CF ] = 0x29D0; // LEFT TRIANGLE BESIDE VERTICAL BAR
        m_aMirrorMap[ 0x29D0 ] = 0x29CF; // VERTICAL BAR BESIDE RIGHT TRIANGLE
        m_aMirrorMap[ 0x29D1 ] = 0x29D2; // BOWTIE WITH LEFT HALF BLACK
        m_aMirrorMap[ 0x29D2 ] = 0x29D1; // BOWTIE WITH RIGHT HALF BLACK
        m_aMirrorMap[ 0x29D4 ] = 0x29D5; // TIMES WITH LEFT HALF BLACK
        m_aMirrorMap[ 0x29D5 ] = 0x29D4; // TIMES WITH RIGHT HALF BLACK
        m_aMirrorMap[ 0x29D8 ] = 0x29D9; // LEFT WIGGLY FENCE
        m_aMirrorMap[ 0x29D9 ] = 0x29D8; // RIGHT WIGGLY FENCE
        m_aMirrorMap[ 0x29DA ] = 0x29DB; // LEFT DOUBLE WIGGLY FENCE
        m_aMirrorMap[ 0x29DB ] = 0x29DA; // RIGHT DOUBLE WIGGLY FENCE
        m_aMirrorMap[ 0x29F5 ] = 0x2215; // REVERSE SOLIDUS OPERATOR
        m_aMirrorMap[ 0x29F8 ] = 0x29F9; // BIG SOLIDUS
        m_aMirrorMap[ 0x29F9 ] = 0x29F8; // BIG REVERSE SOLIDUS
        m_aMirrorMap[ 0x29FC ] = 0x29FD; // LEFT-POINTING CURVED ANGLE BRACKET
        m_aMirrorMap[ 0x29FD ] = 0x29FC; // RIGHT-POINTING CURVED ANGLE BRACKET
        m_aMirrorMap[ 0x2A2B ] = 0x2A2C; // MINUS SIGN WITH FALLING DOTS
        m_aMirrorMap[ 0x2A2C ] = 0x2A2B; // MINUS SIGN WITH RISING DOTS
        m_aMirrorMap[ 0x2A2D ] = 0x2A2E; // PLUS SIGN IN LEFT HALF CIRCLE
        m_aMirrorMap[ 0x2A2E ] = 0x2A2D; // PLUS SIGN IN RIGHT HALF CIRCLE
        m_aMirrorMap[ 0x2A34 ] = 0x2A35; // MULTIPLICATION SIGN IN LEFT HALF CIRCLE
        m_aMirrorMap[ 0x2A35 ] = 0x2A34; // MULTIPLICATION SIGN IN RIGHT HALF CIRCLE
        m_aMirrorMap[ 0x2A3C ] = 0x2A3D; // INTERIOR PRODUCT
        m_aMirrorMap[ 0x2A3D ] = 0x2A3C; // RIGHTHAND INTERIOR PRODUCT
        m_aMirrorMap[ 0x2A64 ] = 0x2A65; // Z NOTATION DOMAIN ANTIRESTRICTION
        m_aMirrorMap[ 0x2A65 ] = 0x2A64; // Z NOTATION RANGE ANTIRESTRICTION
        m_aMirrorMap[ 0x2A79 ] = 0x2A7A; // LESS-THAN WITH CIRCLE INSIDE
        m_aMirrorMap[ 0x2A7A ] = 0x2A79; // GREATER-THAN WITH CIRCLE INSIDE
        m_aMirrorMap[ 0x2A7D ] = 0x2A7E; // LESS-THAN OR SLANTED EQUAL TO
        m_aMirrorMap[ 0x2A7E ] = 0x2A7D; // GREATER-THAN OR SLANTED EQUAL TO
        m_aMirrorMap[ 0x2A7F ] = 0x2A80; // LESS-THAN OR SLANTED EQUAL TO WITH DOT INSIDE
        m_aMirrorMap[ 0x2A80 ] = 0x2A7F; // GREATER-THAN OR SLANTED EQUAL TO WITH DOT INSIDE
        m_aMirrorMap[ 0x2A81 ] = 0x2A82; // LESS-THAN OR SLANTED EQUAL TO WITH DOT ABOVE
        m_aMirrorMap[ 0x2A82 ] = 0x2A81; // GREATER-THAN OR SLANTED EQUAL TO WITH DOT ABOVE
        m_aMirrorMap[ 0x2A83 ] = 0x2A84; // LESS-THAN OR SLANTED EQUAL TO WITH DOT ABOVE RIGHT
        m_aMirrorMap[ 0x2A84 ] = 0x2A83; // GREATER-THAN OR SLANTED EQUAL TO WITH DOT ABOVE LEFT
        m_aMirrorMap[ 0x2A8B ] = 0x2A8C; // LESS-THAN ABOVE DOUBLE-LINE EQUAL ABOVE GREATER-THAN
        m_aMirrorMap[ 0x2A8C ] = 0x2A8B; // GREATER-THAN ABOVE DOUBLE-LINE EQUAL ABOVE LESS-THAN
        m_aMirrorMap[ 0x2A91 ] = 0x2A92; // LESS-THAN ABOVE GREATER-THAN ABOVE DOUBLE-LINE EQUAL
        m_aMirrorMap[ 0x2A92 ] = 0x2A91; // GREATER-THAN ABOVE LESS-THAN ABOVE DOUBLE-LINE EQUAL
        m_aMirrorMap[ 0x2A93 ] = 0x2A94; // LESS-THAN ABOVE SLANTED EQUAL ABOVE GREATER-THAN ABOVE SLANTED EQUAL
        m_aMirrorMap[ 0x2A94 ] = 0x2A93; // GREATER-THAN ABOVE SLANTED EQUAL ABOVE LESS-THAN ABOVE SLANTED EQUAL
        m_aMirrorMap[ 0x2A95 ] = 0x2A96; // SLANTED EQUAL TO OR LESS-THAN
        m_aMirrorMap[ 0x2A96 ] = 0x2A95; // SLANTED EQUAL TO OR GREATER-THAN
        m_aMirrorMap[ 0x2A97 ] = 0x2A98; // SLANTED EQUAL TO OR LESS-THAN WITH DOT INSIDE
        m_aMirrorMap[ 0x2A98 ] = 0x2A97; // SLANTED EQUAL TO OR GREATER-THAN WITH DOT INSIDE
        m_aMirrorMap[ 0x2A99 ] = 0x2A9A; // DOUBLE-LINE EQUAL TO OR LESS-THAN
        m_aMirrorMap[ 0x2A9A ] = 0x2A99; // DOUBLE-LINE EQUAL TO OR GREATER-THAN
        m_aMirrorMap[ 0x2A9B ] = 0x2A9C; // DOUBLE-LINE SLANTED EQUAL TO OR LESS-THAN
        m_aMirrorMap[ 0x2A9C ] = 0x2A9B; // DOUBLE-LINE SLANTED EQUAL TO OR GREATER-THAN
        m_aMirrorMap[ 0x2AA1 ] = 0x2AA2; // DOUBLE NESTED LESS-THAN
        m_aMirrorMap[ 0x2AA2 ] = 0x2AA1; // DOUBLE NESTED GREATER-THAN
        m_aMirrorMap[ 0x2AA6 ] = 0x2AA7; // LESS-THAN CLOSED BY CURVE
        m_aMirrorMap[ 0x2AA7 ] = 0x2AA6; // GREATER-THAN CLOSED BY CURVE
        m_aMirrorMap[ 0x2AA8 ] = 0x2AA9; // LESS-THAN CLOSED BY CURVE ABOVE SLANTED EQUAL
        m_aMirrorMap[ 0x2AA9 ] = 0x2AA8; // GREATER-THAN CLOSED BY CURVE ABOVE SLANTED EQUAL
        m_aMirrorMap[ 0x2AAA ] = 0x2AAB; // SMALLER THAN
        m_aMirrorMap[ 0x2AAB ] = 0x2AAA; // LARGER THAN
        m_aMirrorMap[ 0x2AAC ] = 0x2AAD; // SMALLER THAN OR EQUAL TO
        m_aMirrorMap[ 0x2AAD ] = 0x2AAC; // LARGER THAN OR EQUAL TO
        m_aMirrorMap[ 0x2AAF ] = 0x2AB0; // PRECEDES ABOVE SINGLE-LINE EQUALS SIGN
        m_aMirrorMap[ 0x2AB0 ] = 0x2AAF; // SUCCEEDS ABOVE SINGLE-LINE EQUALS SIGN
        m_aMirrorMap[ 0x2AB3 ] = 0x2AB4; // PRECEDES ABOVE EQUALS SIGN
        m_aMirrorMap[ 0x2AB4 ] = 0x2AB3; // SUCCEEDS ABOVE EQUALS SIGN
        m_aMirrorMap[ 0x2ABB ] = 0x2ABC; // DOUBLE PRECEDES
        m_aMirrorMap[ 0x2ABC ] = 0x2ABB; // DOUBLE SUCCEEDS
        m_aMirrorMap[ 0x2ABD ] = 0x2ABE; // SUBSET WITH DOT
        m_aMirrorMap[ 0x2ABE ] = 0x2ABD; // SUPERSET WITH DOT
        m_aMirrorMap[ 0x2ABF ] = 0x2AC0; // SUBSET WITH PLUS SIGN BELOW
        m_aMirrorMap[ 0x2AC0 ] = 0x2ABF; // SUPERSET WITH PLUS SIGN BELOW
        m_aMirrorMap[ 0x2AC1 ] = 0x2AC2; // SUBSET WITH MULTIPLICATION SIGN BELOW
        m_aMirrorMap[ 0x2AC2 ] = 0x2AC1; // SUPERSET WITH MULTIPLICATION SIGN BELOW
        m_aMirrorMap[ 0x2AC3 ] = 0x2AC4; // SUBSET OF OR EQUAL TO WITH DOT ABOVE
        m_aMirrorMap[ 0x2AC4 ] = 0x2AC3; // SUPERSET OF OR EQUAL TO WITH DOT ABOVE
        m_aMirrorMap[ 0x2AC5 ] = 0x2AC6; // SUBSET OF ABOVE EQUALS SIGN
        m_aMirrorMap[ 0x2AC6 ] = 0x2AC5; // SUPERSET OF ABOVE EQUALS SIGN
        m_aMirrorMap[ 0x2ACD ] = 0x2ACE; // SQUARE LEFT OPEN BOX OPERATOR
        m_aMirrorMap[ 0x2ACE ] = 0x2ACD; // SQUARE RIGHT OPEN BOX OPERATOR
        m_aMirrorMap[ 0x2ACF ] = 0x2AD0; // CLOSED SUBSET
        m_aMirrorMap[ 0x2AD0 ] = 0x2ACF; // CLOSED SUPERSET
        m_aMirrorMap[ 0x2AD1 ] = 0x2AD2; // CLOSED SUBSET OR EQUAL TO
        m_aMirrorMap[ 0x2AD2 ] = 0x2AD1; // CLOSED SUPERSET OR EQUAL TO
        m_aMirrorMap[ 0x2AD3 ] = 0x2AD4; // SUBSET ABOVE SUPERSET
        m_aMirrorMap[ 0x2AD4 ] = 0x2AD3; // SUPERSET ABOVE SUBSET
        m_aMirrorMap[ 0x2AD5 ] = 0x2AD6; // SUBSET ABOVE SUBSET
        m_aMirrorMap[ 0x2AD6 ] = 0x2AD5; // SUPERSET ABOVE SUPERSET
        m_aMirrorMap[ 0x2ADE ] = 0x22A6; // SHORT LEFT TACK
        m_aMirrorMap[ 0x2AE3 ] = 0x22A9; // DOUBLE VERTICAL BAR LEFT TURNSTILE
        m_aMirrorMap[ 0x2AE4 ] = 0x22A8; // VERTICAL BAR DOUBLE LEFT TURNSTILE
        m_aMirrorMap[ 0x2AE5 ] = 0x22AB; // DOUBLE VERTICAL BAR DOUBLE LEFT TURNSTILE
        m_aMirrorMap[ 0x2AEC ] = 0x2AED; // DOUBLE STROKE NOT SIGN
        m_aMirrorMap[ 0x2AED ] = 0x2AEC; // REVERSED DOUBLE STROKE NOT SIGN
        m_aMirrorMap[ 0x2AF7 ] = 0x2AF8; // TRIPLE NESTED LESS-THAN
        m_aMirrorMap[ 0x2AF8 ] = 0x2AF7; // TRIPLE NESTED GREATER-THAN
        m_aMirrorMap[ 0x2AF9 ] = 0x2AFA; // DOUBLE-LINE SLANTED LESS-THAN OR EQUAL TO
        m_aMirrorMap[ 0x2AFA ] = 0x2AF9; // DOUBLE-LINE SLANTED GREATER-THAN OR EQUAL TO
        m_aMirrorMap[ 0x2E02 ] = 0x2E03; // LEFT SUBSTITUTION BRACKET
        m_aMirrorMap[ 0x2E03 ] = 0x2E02; // RIGHT SUBSTITUTION BRACKET
        m_aMirrorMap[ 0x2E04 ] = 0x2E05; // LEFT DOTTED SUBSTITUTION BRACKET
        m_aMirrorMap[ 0x2E05 ] = 0x2E04; // RIGHT DOTTED SUBSTITUTION BRACKET
        m_aMirrorMap[ 0x2E09 ] = 0x2E0A; // LEFT TRANSPOSITION BRACKET
        m_aMirrorMap[ 0x2E0A ] = 0x2E09; // RIGHT TRANSPOSITION BRACKET
        m_aMirrorMap[ 0x2E0C ] = 0x2E0D; // LEFT RAISED OMISSION BRACKET
        m_aMirrorMap[ 0x2E0D ] = 0x2E0C; // RIGHT RAISED OMISSION BRACKET
        m_aMirrorMap[ 0x2E1C ] = 0x2E1D; // LEFT LOW PARAPHRASE BRACKET
        m_aMirrorMap[ 0x2E1D ] = 0x2E1C; // RIGHT LOW PARAPHRASE BRACKET
        m_aMirrorMap[ 0x2E20 ] = 0x2E21; // LEFT VERTICAL BAR WITH QUILL
        m_aMirrorMap[ 0x2E21 ] = 0x2E20; // RIGHT VERTICAL BAR WITH QUILL
        m_aMirrorMap[ 0x2E22 ] = 0x2E23; // TOP LEFT HALF BRACKET
        m_aMirrorMap[ 0x2E23 ] = 0x2E22; // TOP RIGHT HALF BRACKET
        m_aMirrorMap[ 0x2E24 ] = 0x2E25; // BOTTOM LEFT HALF BRACKET
        m_aMirrorMap[ 0x2E25 ] = 0x2E24; // BOTTOM RIGHT HALF BRACKET
        m_aMirrorMap[ 0x2E26 ] = 0x2E27; // LEFT SIDEWAYS U BRACKET
        m_aMirrorMap[ 0x2E27 ] = 0x2E26; // RIGHT SIDEWAYS U BRACKET
        m_aMirrorMap[ 0x2E28 ] = 0x2E29; // LEFT DOUBLE PARENTHESIS
        m_aMirrorMap[ 0x2E29 ] = 0x2E28; // RIGHT DOUBLE PARENTHESIS
        m_aMirrorMap[ 0x3008 ] = 0x3009; // LEFT ANGLE BRACKET
        m_aMirrorMap[ 0x3009 ] = 0x3008; // RIGHT ANGLE BRACKET
        m_aMirrorMap[ 0x300A ] = 0x300B; // LEFT DOUBLE ANGLE BRACKET
        m_aMirrorMap[ 0x300B ] = 0x300A; // RIGHT DOUBLE ANGLE BRACKET
        m_aMirrorMap[ 0x300C ] = 0x300D; // [BEST FIT] LEFT CORNER BRACKET
        m_aMirrorMap[ 0x300D ] = 0x300C; // [BEST FIT] RIGHT CORNER BRACKET
        m_aMirrorMap[ 0x300E ] = 0x300F; // [BEST FIT] LEFT WHITE CORNER BRACKET
        m_aMirrorMap[ 0x300F ] = 0x300E; // [BEST FIT] RIGHT WHITE CORNER BRACKET
        m_aMirrorMap[ 0x3010 ] = 0x3011; // LEFT BLACK LENTICULAR BRACKET
        m_aMirrorMap[ 0x3011 ] = 0x3010; // RIGHT BLACK LENTICULAR BRACKET
        m_aMirrorMap[ 0x3014 ] = 0x3015; // LEFT TORTOISE SHELL BRACKET
        m_aMirrorMap[ 0x3015 ] = 0x3014; // RIGHT TORTOISE SHELL BRACKET
        m_aMirrorMap[ 0x3016 ] = 0x3017; // LEFT WHITE LENTICULAR BRACKET
        m_aMirrorMap[ 0x3017 ] = 0x3016; // RIGHT WHITE LENTICULAR BRACKET
        m_aMirrorMap[ 0x3018 ] = 0x3019; // LEFT WHITE TORTOISE SHELL BRACKET
        m_aMirrorMap[ 0x3019 ] = 0x3018; // RIGHT WHITE TORTOISE SHELL BRACKET
        m_aMirrorMap[ 0x301A ] = 0x301B; // LEFT WHITE SQUARE BRACKET
        m_aMirrorMap[ 0x301B ] = 0x301A; // RIGHT WHITE SQUARE BRACKET
        m_aMirrorMap[ 0xFE59 ] = 0xFE5A; // SMALL LEFT PARENTHESIS
        m_aMirrorMap[ 0xFE5A ] = 0xFE59; // SMALL RIGHT PARENTHESIS
        m_aMirrorMap[ 0xFE5B ] = 0xFE5C; // SMALL LEFT CURLY BRACKET
        m_aMirrorMap[ 0xFE5C ] = 0xFE5B; // SMALL RIGHT CURLY BRACKET
        m_aMirrorMap[ 0xFE5D ] = 0xFE5E; // SMALL LEFT TORTOISE SHELL BRACKET
        m_aMirrorMap[ 0xFE5E ] = 0xFE5D; // SMALL RIGHT TORTOISE SHELL BRACKET
        m_aMirrorMap[ 0xFE64 ] = 0xFE65; // SMALL LESS-THAN SIGN
        m_aMirrorMap[ 0xFE65 ] = 0xFE64; // SMALL GREATER-THAN SIGN
        m_aMirrorMap[ 0xFF08 ] = 0xFF09; // FULLWIDTH LEFT PARENTHESIS
        m_aMirrorMap[ 0xFF09 ] = 0xFF08; // FULLWIDTH RIGHT PARENTHESIS
        m_aMirrorMap[ 0xFF1C ] = 0xFF1E; // FULLWIDTH LESS-THAN SIGN
        m_aMirrorMap[ 0xFF1E ] = 0xFF1C; // FULLWIDTH GREATER-THAN SIGN
        m_aMirrorMap[ 0xFF3B ] = 0xFF3D; // FULLWIDTH LEFT SQUARE BRACKET
        m_aMirrorMap[ 0xFF3D ] = 0xFF3B; // FULLWIDTH RIGHT SQUARE BRACKET
        m_aMirrorMap[ 0xFF5B ] = 0xFF5D; // FULLWIDTH LEFT CURLY BRACKET
        m_aMirrorMap[ 0xFF5D ] = 0xFF5B; // FULLWIDTH RIGHT CURLY BRACKET
        m_aMirrorMap[ 0xFF5F ] = 0xFF60; // FULLWIDTH LEFT WHITE PARENTHESIS
        m_aMirrorMap[ 0xFF60 ] = 0xFF5F; // FULLWIDTH RIGHT WHITE PARENTHESIS
        m_aMirrorMap[ 0xFF62 ] = 0xFF63; // [BEST FIT] HALFWIDTH LEFT CORNER BRACKET
        m_aMirrorMap[ 0xFF63 ] = 0xFF62; // [BEST FIT] HALFWIDTH RIGHT CORNER BRACKET
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
