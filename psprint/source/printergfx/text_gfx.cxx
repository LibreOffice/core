/*************************************************************************
  *
  *  $RCSfile: text_gfx.cxx,v $
  *
  *  $Revision: 1.10 $
  *
  *  last change: $Author: pl $ $Date: 2001-07-27 07:58:33 $
  *
  *  The Contents of this file are made available subject to the terms of
  *  either of the following licenses
  *
  *         - GNU Lesser General Public License Version 2.1
  *         - Sun Industry Standards Source License Version 1.1
  *
  *  Sun Microsystems Inc., October, 2000
  *
  *  GNU Lesser General Public License Version 2.1
  *  =============================================
  *  Copyright 2000 by Sun Microsystems, Inc.
  *  901 San Antonio Road, Palo Alto, CA 94303, USA
  *
  *  This library is free software; you can redistribute it and/or
  *  modify it under the terms of the GNU Lesser General Public
  *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _PSPRINT_PRINTERGFX_HXX_
#include <psprint/printergfx.hxx>
#endif
#ifndef _PSPRINT_FONTMANAGER_HXX_
#include <psprint/fontmanager.hxx>
#endif
#ifndef _PSPRINT_HELPER_HXX_
#include <psprint/helper.hxx>
#endif
#ifndef _PSPRINT_PRINTERUTIL_HXX_
#include "psputil.hxx"
#endif
#ifndef _PSPRINT_GLYPHSET_HXX_
#include <glyphset.hxx>
#endif
#ifndef _OSL_THREAD_H_
#include <osl/thread.h>
#endif
#ifdef SOLARIS
#include <alloca.h>
#endif
#include <math.h>

using namespace psp ;

namespace psp {
/*
 container for a font and its helper fonts:
 1st font is the font substitute e.g. helvetica substitutes arial on the printer
 2nd is the font itself
 3rd is a fallback font, usually a font with unicode glyph repertoir (e.g. andale)
 symbol fonts (adobe-fontspecific) may need special glyphmapping
 (symbol page vc. latin page)
*/
class Font3
{
    private:

        #define Font3Size 3

        fontID  mpFont [Font3Size];
        bool    mbSymbol;

    public:

        fontID  GetFont (int nIdx) const
                    { return nIdx < Font3Size ? mpFont[nIdx] : -1 ; }
        bool    IsSymbolFont () const
                    { return mbSymbol; }

        Font3 (const PrinterGfx &rGfx);
        ~Font3 () {}
};

Font3::Font3(const PrinterGfx &rGfx)
{
    mpFont[0] = rGfx.getFontSubstitute();
    mpFont[1] = rGfx.GetFontID();
    mpFont[2] = rGfx.getFallbackID();
    // mpFont[2] = rGfx.GetFontID();

       PrintFontManager &rMgr = PrintFontManager::get();
    mbSymbol = mpFont[1] != -1 ?
                rMgr.getFontEncoding(mpFont[1]) == RTL_TEXTENCODING_SYMBOL : false;
}

} // namespace psp

static int getVerticalDeltaAngle( sal_Unicode nChar )
{
    int nAngle = 0;
    if( ( nChar >= 0x1100 && nChar < 0x11fa ) ||
        ( nChar >= 0x3000 && nChar < 0xfb00 ) )
    {
        if( nChar == 0x2010 || nChar == 0x2015 ||
            nChar == 0x2016 || nChar == 0x2026 ||
            ( nChar >= 0x3008 && nChar < 0x3018 ) ||
            nChar >= 0xFF00 )
            nAngle = 0;
        else if( nChar == 0x30fc )
            nAngle = -900;
        else
            nAngle = 900;
    }
    return nAngle;
}

void
PrinterGfx::PSUploadPS1Font (sal_Int32 nFontID)
{
    std::list< sal_Int32 >::iterator aFont;
    // already in the document header ?
    for (aFont = maPS1Font.begin();
         aFont != maPS1Font.end() && *aFont != nFontID;
         aFont++)
        ;
    // not yet downloaded, do now
    if (aFont == maPS1Font.end())
    {
        const rtl::OString& rSysPath (mrFontMgr.getFontFileSysPath(nFontID) );
        rtl::OUString aUNCPath;
        osl::File::getFileURLFromSystemPath (OStringToOUString (rSysPath, osl_getThreadTextEncoding()), aUNCPath);
        osl::File aFontFile (aUNCPath);

        // provide the pfb or pfa font as a (pfa-)font resource
        rtl::OString aPostScriptName = rtl::OUStringToOString (
                                                               mrFontMgr.getPSName(mnFontID),
                                                               RTL_TEXTENCODING_ASCII_US);

        sal_Char  pFontResource [256];
        sal_Int32 nChar = 0;

        nChar  = psp::appendStr ("%%BeginResource: font ", pFontResource);
        nChar += psp::appendStr (aPostScriptName.getStr(), pFontResource + nChar);
        nChar += psp::appendStr ("\n",                     pFontResource + nChar);

        WritePS (mpPageHeader, pFontResource);

        aFontFile.open (OpenFlag_Read);
        convertPfbToPfa (aFontFile, *mpPageHeader);
        aFontFile.close ();

        WritePS (mpPageHeader, "%%EndResource\n");

        // add the fontid to the list
        maPS1Font.push_back (nFontID);
    }
}

/*
 * implement text handling printer routines,
 */

sal_uInt16
PrinterGfx::SetFont(
                    sal_Int32 nFontID,
                    sal_Int32 nHeight,
                    sal_Int32 nWidth,
                    sal_Int32 nAngle,
                    bool bVertical
                    )
{
    // font and encoding will be set by drawText again immediately
    // before PSShowText
    mnFontID                          = nFontID;
    maVirtualStatus.maFont            = rtl::OString();
    maVirtualStatus.maEncoding        = RTL_TEXTENCODING_DONTKNOW;
    maVirtualStatus.mnTextHeight      = nHeight;
    maVirtualStatus.mnTextWidth       = nWidth;
    mnTextAngle                       = nAngle;
    mbTextVertical                    = bVertical;

    return 0;
}

sal_uInt16
PrinterGfx::SetFallbackFont ( sal_Int32 nFontID )
{
    mnFallbackID = nFontID;
    return 0;
}

void
PrinterGfx::DrawText (
                      const Point& rPoint,
                      const sal_Unicode* pStr,
                      sal_Int16 nLen,
                      const sal_Int32* pDeltaArray
                      )
{
    fontID nRestoreFont = mnFontID;

    // setup font[substitutes] and map the string into the symbol area in case of
    // symbol font
    Font3 aFont(*this);
    sal_Unicode *pEffectiveStr;
    if ( aFont.IsSymbolFont() )
    {
        pEffectiveStr = (sal_Unicode*)alloca(nLen * sizeof(pStr[0]));
        for (int i = 0; i < nLen; i++)
            pEffectiveStr[i] = pStr[i] < 256 ? pStr[i] + 0xF000 : pStr[i];
    }
    else
    {
        pEffectiveStr = const_cast<sal_Unicode*>(pStr);
    }

    fontID    *pFontMap   = (fontID*)    alloca(nLen * sizeof(fontID));
    sal_Int32 *pCharWidth = (sal_Int32*) alloca(nLen * sizeof(sal_Int32));

    int nScale = maVirtualStatus.mnTextWidth ? maVirtualStatus.mnTextWidth : maVirtualStatus.mnTextHeight;
    for( int n = 0; n < nLen; n++ )
    {
        CharacterMetric aBBox;
        pFontMap[n]   = getCharMetric (aFont, pEffectiveStr[n], &aBBox);
        pCharWidth[n] = getCharWidth  (mbTextVertical, pEffectiveStr[n], &aBBox) * nScale;
    }

    // setup a new delta array, use virtual resolution of 1000
    sal_Int32* pNewDeltaArray = (sal_Int32*)alloca( sizeof( sal_Int32 )*nLen );
    if ( pDeltaArray != 0)
    {
        for (int i = 0; i < nLen - 1; i++)
            pNewDeltaArray[i] = 1000 * pDeltaArray[i];
        pNewDeltaArray[nLen - 1] = 0;
    }
    else
    {
        pNewDeltaArray[0] = pCharWidth[0];
        for (int i = 1; i < nLen; i++)
            pNewDeltaArray[i] = pNewDeltaArray[i-1] + pCharWidth[i];
    }

    // move and rotate the user coordinate system
    // avoid the gsave/grestore for the simple cases since it allows
    // reuse of the current font if it hasn't changed
    sal_Int32 nCurrentTextAngle = mnTextAngle;
    sal_Int32 nCurrentPointX;
    sal_Int32 nCurrentPointY;

    if (nCurrentTextAngle != 0)
    {
        PSGSave ();
        PSTranslate (rPoint);
        PSRotate (nCurrentTextAngle);
        mnTextAngle = 0;

        nCurrentPointX = 0;
        nCurrentPointY = 0;
    }
    else
    {
        nCurrentPointX = rPoint.X();
        nCurrentPointY = rPoint.Y();
    }

    // draw the string
    sal_Int32 nDelta = 0;
    for (int nTo = 0; nTo < nLen; )
    {
        int    nFrom = nTo;
        fontID nFont = pFontMap[ nFrom ];

        while ((nTo < nLen) && (nFont == pFontMap[nTo]))
        {
            pNewDeltaArray[ nTo ] = ((0.5 + pNewDeltaArray[ nTo ]) / 1000.0) - nDelta;
            nTo++ ;
        }

        SetFont( nFont, maVirtualStatus.mnTextHeight, maVirtualStatus.mnTextWidth,
                mnTextAngle, mbTextVertical );

        if (mbTextVertical)
        {
            drawVerticalizedText(
                    Point(nCurrentPointX + nDelta, nCurrentPointY),
                    pEffectiveStr + nFrom, nTo - nFrom,
                    pNewDeltaArray + nFrom );
        }
        else
        {
            drawText(
                    Point(nCurrentPointX + nDelta, nCurrentPointY),
                    pEffectiveStr + nFrom, nTo - nFrom,
                    pDeltaArray == NULL ? NULL : pNewDeltaArray + nFrom );
        }
        nDelta += pNewDeltaArray[ nTo - 1 ];
    }

    // restore the user coordinate system
    if (nCurrentTextAngle != 0)
    {
        PSGRestore ();
        mnTextAngle = nCurrentTextAngle;
    }

    // restore the original font settings
    SetFont( nRestoreFont, maVirtualStatus.mnTextHeight, maVirtualStatus.mnTextWidth,
            mnTextAngle, mbTextVertical );
}

void PrinterGfx::drawVerticalizedText(
                                      const Point& rPoint,
                                      const sal_Unicode* pStr,
                                      sal_Int16 nLen,
                                      const sal_Int32* pDeltaArray
                                      )
{
    sal_Int32* pDelta = (sal_Int32*)alloca( nLen * sizeof(sal_Int32) );

    int nTextScale   = maVirtualStatus.mnTextWidth ? maVirtualStatus.mnTextWidth : maVirtualStatus.mnTextHeight;
    int nNormalAngle = mnTextAngle;
    int nDeltaAngle, nLastPos = 0;

    double fSin = sin( -2.0*M_PI*nNormalAngle/3600 );
    double fCos = cos( -2.0*M_PI*nNormalAngle/3600 );

    Point aPoint( rPoint );
    for( int i = 0; i < nLen; )
    {
        while( ( nDeltaAngle = getVerticalDeltaAngle( pStr[i] ) ) == 0 && i < nLen )
            i++;
        if( i <= nLen && i > nLastPos )
        {
            int nWidth = 0;
            for( int n = nLastPos; n < i; n++ )
                pDelta[n] = pDeltaArray[n] - (aPoint.X() - rPoint.X() );

            SetFont( mnFontID, maVirtualStatus.mnTextHeight, maVirtualStatus.mnTextWidth, nNormalAngle, mbTextVertical );
            drawText( aPoint, pStr + nLastPos, i - nLastPos, pDelta + nLastPos );

            aPoint.X() = rPoint.X() + ((double)pDeltaArray[i-1] * fCos);
            aPoint.Y() = rPoint.Y() + ((double)pDeltaArray[i-1] * fSin);
        }
        if( i < nLen )
        {
            SetFont( mnFontID, maVirtualStatus.mnTextHeight, maVirtualStatus.mnTextWidth, nNormalAngle + nDeltaAngle,
                     mbTextVertical );

            CharacterMetric aMetric;
            PrintFontManager &rMgr = PrintFontManager::get();
            rMgr.getMetrics( mnFontID, pStr + i, 1, &aMetric /* mbVertical */);
            double nH = nTextScale * aMetric.height / 1000.0;

            Point aPos( aPoint );
            switch( nDeltaAngle )
            {
                case 900:
                    aPos.X() += nH * fCos;
                    aPos.Y() += nH * fSin;
                    break;
                case -900:
                    aPos.X() += nH * fSin;
                    aPos.Y() += nH * fCos;
                    break;
            }
            drawText( aPos, pStr+i, 1, NULL );
            if( i < nLen-1 && pDeltaArray )
            {
                aPoint.X() = rPoint.X() + ((double)pDeltaArray[i] * fCos);
                aPoint.Y() = rPoint.Y() + ((double)pDeltaArray[i] * fSin);
            }
        }
        i++;
        nLastPos = i;
    }
    mnTextAngle = nNormalAngle;
}

void
PrinterGfx::LicenceWarning(const Point& rPoint, const sal_Unicode* pStr,
                           sal_Int16 nLen, const sal_Int32* pDeltaArray)
{
    // treat it like a builtin font in case a user has that font also in the
    // printer. This is not so unlikely as it may seem; no print embedding
    // licensed fonts are often used (or so they say) in companies:
    // they are installed on displays and printers, but get not embedded in
    // print files or documents because they are not licensed for use outside
    // the company.
    rtl::OString aMessage( "The font " );
    aMessage += rtl::OUStringToOString( mrFontMgr.getPSName(mnFontID),
            RTL_TEXTENCODING_ASCII_US );
    aMessage += " could not be downloaded\nbecause its license does not allow for that";
    PSComment( aMessage.getStr() );

    rtl::OString aFontName = rtl::OUStringToOString(
            mrFontMgr.getPSName(mnFontID),
            RTL_TEXTENCODING_ASCII_US);
    PSSetFont (aFontName, RTL_TEXTENCODING_ISO_8859_1);

    sal_Size  nSize    = 4 * nLen;
    sal_uChar* pBuffer = (sal_uChar*)alloca (nSize* sizeof(sal_uChar));

    ConverterFactory* pCvt = GetConverterFactory ();
    nSize = pCvt->Convert (pStr, nLen, pBuffer, nSize, RTL_TEXTENCODING_ISO_8859_1);

    PSMoveTo (rPoint);
    PSShowText (pBuffer, nLen, nSize, pDeltaArray);
}

void
PrinterGfx::drawText(
                     const Point& rPoint,
                     const sal_Unicode* pStr,
                     sal_Int16 nLen,
                     const sal_Int32* pDeltaArray
                     )
{
    if (!(nLen > 0))
        return;

    fonttype::type   eType          = mrFontMgr.getFontType (mnFontID);
    fonttype::type   eEffectiveType = fonttype::Unknown;
    rtl_TextEncoding nEncoding      = mrFontMgr.getFontEncoding (mnFontID);

    if (eType == fonttype::Type1)
        PSUploadPS1Font (mnFontID);

    if (   eType == fonttype::TrueType
        && !mrFontMgr.isFontDownloadingAllowed(mnFontID))
    {
        LicenceWarning(rPoint, pStr, nLen, pDeltaArray);
        return;
    }

    // search for a glyph set matching the set font
    std::list< GlyphSet >::iterator aIter;
    for (aIter = maPS3Font.begin(); aIter != maPS3Font.end(); aIter++)
        if (   ((*aIter).GetFontID()  == mnFontID)
            && ((*aIter).IsVertical() == mbTextVertical))
        {
            (*aIter).DrawText (*this, rPoint, pStr, nLen, pDeltaArray);
            break;
        }

    // not found ? create a new one
    if (aIter == maPS3Font.end())
    {
        maPS3Font.push_back (GlyphSet(mnFontID, mbTextVertical));
        maPS3Font.back().DrawText (*this, rPoint, pStr, nLen, pDeltaArray);
    }
}

int
PrinterGfx::getCharWidth (sal_Bool b_vert, sal_Unicode n_char, CharacterMetric *p_bbox)
{
    b_vert = b_vert && (getVerticalDeltaAngle(n_char) != 0);
    return b_vert ? p_bbox->height : p_bbox->width;
}

fontID
PrinterGfx::getCharMetric (const Font3 &rFont, sal_Unicode n_char, CharacterMetric *p_bbox)
{
    p_bbox->width  = -1;
    p_bbox->height = -1;

    for (fontID n = 0; n < 3; n++)
    {
        fontID n_font = rFont.GetFont(n);
        if (n_font != -1)
            mrFontMgr.getMetrics( n_font, n_char, n_char, p_bbox );
        if (p_bbox->width >= 0 && p_bbox->height >= 0)
            return n_font;
    }
    if (n_char != '?')
        return getCharMetric (rFont, '?', p_bbox);

    return rFont.GetFont(0) != -1 ? rFont.GetFont(0) : rFont.GetFont(1);
}

fontID
PrinterGfx::getFontSubstitute () const
{
    if( mpFontSubstitutes )
    {
        ::std::hash_map< fontID, fontID >::const_iterator it =
              mpFontSubstitutes->find( mnFontID );
        if( it != mpFontSubstitutes->end() )
            return it->second;
    }

    return -1;
}

sal_Int32
PrinterGfx::GetCharWidth (sal_Unicode nFrom, sal_Unicode nTo, long *pWidthArray)
{
    Font3 aFont(*this);
    if (aFont.IsSymbolFont() && (nFrom < 256) && (nTo < 256))
    {
        nFrom += 0xF000;
        nTo   += 0xF000;
    }

    int nScale = maVirtualStatus.mnTextWidth ? maVirtualStatus.mnTextWidth : maVirtualStatus.mnTextHeight;
    for( int n = 0; n < (nTo - nFrom + 1); n++ )
    {
        CharacterMetric aBBox;
        getCharMetric (aFont, n + nFrom, &aBBox);
        pWidthArray[n] = getCharWidth (mbTextVertical, n + nFrom, &aBBox) * nScale;
    }

    // returned metrics have postscript precision
    return 1000;
}

const ::std::list< KernPair >& PrinterGfx::getKernPairs( bool bVertical ) const
{
    /*
     *  Note: this is only a 80% solution: if a font is only
     *  partially substituted in a string due to missing glyphs
     *  the results may not be perfect; the more so the more the
     *  substitution differs from the original metricwise. But
     *  vcl only asks for KernPairs for each font once and NOT
     *  in a string context this is the best we can do.
     *  In future the kerning should be done on a per string basis.
     */
    fontID nFont = mnFontID;
    if( mpFontSubstitutes )
    {
        ::std::hash_map< fontID, fontID >::const_iterator it =
              mpFontSubstitutes->find( mnFontID );
        if( it != mpFontSubstitutes->end() )
            nFont = it->second;
    }
    return mrFontMgr.getKernPairs( nFont, bVertical );
}

/*
 * advanced glyph handling
 */

sal_Bool
PrinterGfx::GetGlyphBoundRect (sal_Unicode c, Rectangle& rOutRect)
{
    return 0;
}

sal_uInt32
PrinterGfx::GetGlyphOutline (sal_Unicode c,
                             sal_uInt16 **ppPolySizes, Point **ppPoints, sal_uInt8 **ppFlags)
{
    return 0;
}

/*
 * spool the converted truetype fonts to the page header after the page body is
 * complete
 * for Type1 fonts spool additional reencoding vectors that are necessary to access the
 * whole font
 */

void
PrinterGfx::OnEndPage ()
{
    std::list< GlyphSet >::iterator aIter;
    for (aIter = maPS3Font.begin(); aIter != maPS3Font.end(); ++aIter)
    {
        if (aIter->GetFontType() == fonttype::TrueType)
        {
            aIter->PSUploadFont (*mpPageHeader, *this, mbUploadPS42Fonts ? true : false );
        }
        else
        // (   aIter->GetFontType() == fonttype::Type1
        //  || aIter->GetFontType() == fonttype::Builtin )
        {
            aIter->PSUploadEncoding (mpPageHeader, *this);
        }
    }
}

