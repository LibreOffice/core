/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_editeng.hxx"

// include ----------------------------------------------------------------

#include <vcl/outdev.hxx>
#include <vcl/print.hxx>
#include <tools/poly.hxx>
#include <unotools/charclass.hxx>
#include <editeng/unolingu.hxx>
#include <com/sun/star/i18n/KCharacterType.hpp>

#define _SVX_SVXFONT_CXX

#include <editeng/svxfont.hxx>
#include <editeng/escpitem.hxx>

// Minimum: Prozentwert fuers kernen
#define MINKERNPERCENT 5

// prop. Groesse der Kleinbuchstaben bei Kapitaelchen
#define KAPITAELCHENPROP 66

#ifndef REDUCEDSVXFONT
    const sal_Unicode CH_BLANK = sal_Unicode(' ');      // ' ' Leerzeichen
    static sal_Char __READONLY_DATA sDoubleSpace[] = "  ";
#endif

/*************************************************************************
 *                      class SvxFont
 *************************************************************************/

SvxFont::SvxFont()
{
    nKern = nEsc = 0;
    nPropr = 100;
    eCaseMap = SVX_CASEMAP_NOT_MAPPED;
    eLang = LANGUAGE_SYSTEM;
}

SvxFont::SvxFont( const Font &rFont )
    : Font( rFont )
{
    nKern = nEsc = 0;
    nPropr = 100;
    eCaseMap = SVX_CASEMAP_NOT_MAPPED;
    eLang = LANGUAGE_SYSTEM;
}

/*************************************************************************
 *                      class SvxFont: Copy-Ctor
 *************************************************************************/

SvxFont::SvxFont( const SvxFont &rFont )
    : Font( rFont )
{
    nKern = rFont.GetFixKerning();
    nEsc  = rFont.GetEscapement();
    nPropr = rFont.GetPropr();
    eCaseMap = rFont.GetCaseMap();
    eLang = rFont.GetLanguage();
}

/*************************************************************************
 *               static SvxFont::DrawArrow
 *************************************************************************/

void SvxFont::DrawArrow( OutputDevice &rOut, const Rectangle& rRect,
    const Size& rSize, const Color& rCol, sal_Bool bLeft )
{
    long nLeft = ( rRect.Left() + rRect.Right() - rSize.Width() )/ 2;
    long nRight = nLeft + rSize.Width();
    long nMid = ( rRect.Top() + rRect.Bottom() ) / 2;
    long nTop = nMid - rSize.Height() / 2;
    long nBottom = nTop + rSize.Height();
    if( nLeft < rRect.Left() )
    {
        nLeft = rRect.Left();
        nRight = rRect.Right();
    }
    if( nTop < rRect.Top() )
    {
        nTop = rRect.Top();
        nBottom = rRect.Bottom();
    }
    Polygon aPoly;
    Point aTmp( bLeft ? nLeft : nRight, nMid );
    Point aNxt( bLeft ? nRight : nLeft, nTop );
    aPoly.Insert( 0, aTmp );
    aPoly.Insert( 0, aNxt );
    aNxt.Y() = nBottom;
    aPoly.Insert( 0, aNxt );
    aPoly.Insert( 0, aTmp );
    Color aOldLineColor = rOut.GetLineColor();
    Color aOldFillColor = rOut.GetFillColor();
    rOut.SetFillColor( rCol );
    rOut.SetLineColor( Color( COL_BLACK ) );
    rOut.DrawPolygon( aPoly );
    rOut.DrawLine( aTmp, aNxt );
    rOut.SetLineColor( aOldLineColor );
    rOut.SetFillColor( aOldFillColor );
}

/*************************************************************************
 *                      SvxFont::CalcCaseMap
 *************************************************************************/

XubString SvxFont::CalcCaseMap( const XubString &rTxt ) const
{
    if( !IsCaseMap() || !rTxt.Len() ) return rTxt;
    XubString aTxt( rTxt );
    // Ich muss mir noch die Sprache besorgen
    const LanguageType eLng = LANGUAGE_DONTKNOW == eLang
                            ? LANGUAGE_SYSTEM : eLang;

    CharClass aCharClass( SvxCreateLocale( eLng ) );

    switch( eCaseMap )
    {
        case SVX_CASEMAP_KAPITAELCHEN:
        case SVX_CASEMAP_VERSALIEN:
        {
            aCharClass.toUpper( aTxt );
            break;
        }

        case SVX_CASEMAP_GEMEINE:
        {
            aCharClass.toLower( aTxt );
            break;
        }
        case SVX_CASEMAP_TITEL:
        {
            // Jeder Wortbeginn wird gross geschrieben,
            // der Rest des Wortes wird unbesehen uebernommen.
            // Bug: wenn das Attribut mitten im Wort beginnt.
            sal_Bool bBlank = sal_True;

            for( sal_uInt16 i = 0; i < aTxt.Len(); ++i )
            {
                if( sal_Unicode(' ') == aTxt.GetChar(i) || sal_Unicode('\t') == aTxt.GetChar(i) )
                    bBlank = sal_True;
                else
                {
                    if( bBlank )
                    {
                        String aTemp( aTxt.GetChar( i ) );
                        aCharClass.toUpper( aTemp );
                        aTxt.Replace( i, 1, aTemp );
                    }
                    bBlank = sal_False;
                }
            }
            break;
        }
        default:
        {
            DBG_ASSERT(!this, "SvxFont::CaseMapTxt: unknown casemap");
            break;
        }
    }
    return aTxt;
}

/*************************************************************************
 * Hier beginnen die Methoden, die im Writer nicht benutzt werden koennen,
 * deshalb kann man diesen Bereich durch setzen von REDUCEDSVXFONT ausklammern.
 *************************************************************************/
#ifndef REDUCEDSVXFONT

/*************************************************************************
 *                      class SvxDoCapitals
 * die virtuelle Methode Do wird von SvxFont::DoOnCapitals abwechselnd mit
 * den "Gross-" und "Kleinbuchstaben"-Teilen aufgerufen.
 * Die Ableitungen von SvxDoCapitals erfuellen diese Methode mit Leben.
 *************************************************************************/

class SvxDoCapitals
{
protected:
    OutputDevice *pOut;
    const XubString &rTxt;
    const xub_StrLen nIdx;
    const xub_StrLen nLen;

public:
    SvxDoCapitals( OutputDevice *_pOut, const XubString &_rTxt,
                   const xub_StrLen _nIdx, const xub_StrLen _nLen )
        : pOut(_pOut), rTxt(_rTxt), nIdx(_nIdx), nLen(_nLen)
        { }

    virtual void DoSpace( const sal_Bool bDraw );
    virtual void SetSpace();
    virtual void Do( const XubString &rTxt,
                     const xub_StrLen nIdx, const xub_StrLen nLen,
                     const sal_Bool bUpper ) = 0;

    inline OutputDevice *GetOut() { return pOut; }
    inline const XubString &GetTxt() const { return rTxt; }
    xub_StrLen GetIdx() const { return nIdx; }
    xub_StrLen GetLen() const { return nLen; }
};

void SvxDoCapitals::DoSpace( const sal_Bool /*bDraw*/ ) { }

void SvxDoCapitals::SetSpace() { }

void SvxDoCapitals::Do( const XubString &/*_rTxt*/, const xub_StrLen /*_nIdx*/,
    const xub_StrLen /*_nLen*/, const sal_Bool /*bUpper*/ ) { }

/*************************************************************************
 *                  SvxFont::DoOnCapitals() const
 * zerlegt den String in Gross- und Kleinbuchstaben und ruft jeweils die
 * Methode SvxDoCapitals::Do( ) auf.
 *************************************************************************/

void SvxFont::DoOnCapitals(SvxDoCapitals &rDo, const xub_StrLen nPartLen) const
{
    const XubString &rTxt = rDo.GetTxt();
    const xub_StrLen nIdx = rDo.GetIdx();
    const xub_StrLen nLen = STRING_LEN == nPartLen ? rDo.GetLen() : nPartLen;

    const XubString aTxt( CalcCaseMap( rTxt ) );
    const sal_uInt16 nTxtLen = Min( rTxt.Len(), nLen );
    sal_uInt16 nPos = 0;
    sal_uInt16 nOldPos = nPos;

    // #108210#
    // Test if string length differ between original and CaseMapped
    sal_Bool bCaseMapLengthDiffers(aTxt.Len() != rTxt.Len());

    const LanguageType eLng = LANGUAGE_DONTKNOW == eLang
                            ? LANGUAGE_SYSTEM : eLang;

    CharClass   aCharClass( SvxCreateLocale( eLng ) );
    String      aCharString;

    while( nPos < nTxtLen )
    {
        // Erst kommen die Upper-Chars dran

        // 4251: Es gibt Zeichen, die Upper _und_ Lower sind (z.B. das Blank).
        // Solche Zweideutigkeiten fuehren ins Chaos, deswegen werden diese
        // Zeichen der Menge Lower zugeordnet !

        while( nPos < nTxtLen )
        {
            aCharString = rTxt.GetChar( nPos + nIdx );
            sal_Int32 nCharacterType = aCharClass.getCharacterType( aCharString, 0 );
            if ( nCharacterType & ::com::sun::star::i18n::KCharacterType::LOWER )
                break;
            if ( ! ( nCharacterType & ::com::sun::star::i18n::KCharacterType::UPPER ) )
                break;
            ++nPos;
        }
        if( nOldPos != nPos )
        {
            if(bCaseMapLengthDiffers)
            {
                // #108210#
                // If strings differ work preparing the necessary snippet to address that
                // potential difference
                const XubString aSnippet(rTxt, nIdx + nOldPos, nPos-nOldPos);
                XubString aNewText = CalcCaseMap(aSnippet);

                rDo.Do( aNewText, 0, aNewText.Len(), sal_True );
            }
            else
            {
                rDo.Do( aTxt, nIdx + nOldPos, nPos-nOldPos, sal_True );
            }

            nOldPos = nPos;
        }
        // Nun werden die Lower-Chars verarbeitet (ohne Blanks)
        while( nPos < nTxtLen )
        {
            sal_uInt32  nCharacterType = aCharClass.getCharacterType( aCharString, 0 );
            if ( ( nCharacterType & ::com::sun::star::i18n::KCharacterType::UPPER ) )
                break;
            if ( CH_BLANK == aCharString )
                break;
            if( ++nPos < nTxtLen )
                aCharString = rTxt.GetChar( nPos + nIdx );
        }
        if( nOldPos != nPos )
        {
            if(bCaseMapLengthDiffers)
            {
                // #108210#
                // If strings differ work preparing the necessary snippet to address that
                // potential difference
                const XubString aSnippet(rTxt, nIdx + nOldPos, nPos - nOldPos);
                XubString aNewText = CalcCaseMap(aSnippet);

                rDo.Do( aNewText, 0, aNewText.Len(), sal_False );
            }
            else
            {
                rDo.Do( aTxt, nIdx + nOldPos, nPos-nOldPos, sal_False );
            }

            nOldPos = nPos;
        }
        // Nun werden die Blanks verarbeitet
        while( nPos < nTxtLen && CH_BLANK == aCharString && ++nPos < nTxtLen )
            aCharString = rTxt.GetChar( nPos + nIdx );

        if( nOldPos != nPos )
        {
            rDo.DoSpace( sal_False );

            if(bCaseMapLengthDiffers)
            {
                // #108210#
                // If strings differ work preparing the necessary snippet to address that
                // potential difference
                const XubString aSnippet(rTxt, nIdx + nOldPos, nPos - nOldPos);
                XubString aNewText = CalcCaseMap(aSnippet);

                rDo.Do( aNewText, 0, aNewText.Len(), sal_False );
            }
            else
            {
                rDo.Do( aTxt, nIdx + nOldPos, nPos - nOldPos, sal_False );
            }

            nOldPos = nPos;
            rDo.SetSpace();
        }
    }
    rDo.DoSpace( sal_True );
}

/**************************************************************************
 *                    SvxFont::SetPhysFont()
 *************************************************************************/

void SvxFont::SetPhysFont( OutputDevice *pOut ) const
{
    const Font& rCurrentFont = pOut->GetFont();
    if ( nPropr == 100 )
    {
        if ( !rCurrentFont.IsSameInstance( *this ) )
            pOut->SetFont( *this );
    }
    else
    {
        Font aNewFont( *this );
        Size aSize( aNewFont.GetSize() );
        aNewFont.SetSize( Size( aSize.Width() * nPropr / 100L,
                                    aSize.Height() * nPropr / 100L ) );
        if ( !rCurrentFont.IsSameInstance( aNewFont ) )
            pOut->SetFont( aNewFont );
    }
}

/*************************************************************************
 *                    SvxFont::ChgPhysFont()
 *************************************************************************/

Font SvxFont::ChgPhysFont( OutputDevice *pOut ) const
{
    Font aOldFont( pOut->GetFont() );
    SetPhysFont( pOut );
    return aOldFont;
}

/*************************************************************************
 *                    SvxFont::GetPhysTxtSize()
 *************************************************************************/

Size SvxFont::GetPhysTxtSize( const OutputDevice *pOut, const XubString &rTxt,
                         const xub_StrLen nIdx, const xub_StrLen nLen ) const
{
    if ( !IsCaseMap() && !IsKern() )
        return Size( pOut->GetTextWidth( rTxt, nIdx, nLen ),
                     pOut->GetTextHeight() );

    Size aTxtSize;
    aTxtSize.setHeight( pOut->GetTextHeight() );
    if ( !IsCaseMap() )
        aTxtSize.setWidth( pOut->GetTextWidth( rTxt, nIdx, nLen ) );
    else
    {
        // #108210#
        const XubString aNewText = CalcCaseMap(rTxt);
        sal_Bool bCaseMapLengthDiffers(aNewText.Len() != rTxt.Len());
        sal_Int32 nWidth(0L);

        if(bCaseMapLengthDiffers)
        {
            // If strings differ work preparing the necessary snippet to address that
            // potential difference
            const XubString aSnippet(rTxt, nIdx, nLen);
            XubString _aNewText = CalcCaseMap(aSnippet);
            nWidth = pOut->GetTextWidth( _aNewText, 0, _aNewText.Len() );
        }
        else
        {
            nWidth = pOut->GetTextWidth( aNewText, nIdx, nLen );
        }

        aTxtSize.setWidth(nWidth);
    }

    if( IsKern() && ( nLen > 1 ) )
        aTxtSize.Width() += ( ( nLen-1 ) * long( nKern ) );

    return aTxtSize;
}

Size SvxFont::GetPhysTxtSize( const OutputDevice *pOut, const XubString &rTxt )
{
    if ( !IsCaseMap() && !IsKern() )
        return Size( pOut->GetTextWidth( rTxt ), pOut->GetTextHeight() );

    Size aTxtSize;
    aTxtSize.setHeight( pOut->GetTextHeight() );
    if ( !IsCaseMap() )
        aTxtSize.setWidth( pOut->GetTextWidth( rTxt ) );
    else
        aTxtSize.setWidth( pOut->GetTextWidth( CalcCaseMap( rTxt ) ) );

    if( IsKern() && ( rTxt.Len() > 1 ) )
        aTxtSize.Width() += ( ( rTxt.Len()-1 ) * long( nKern ) );

    return aTxtSize;
}

Size SvxFont::QuickGetTextSize( const OutputDevice *pOut, const XubString &rTxt,
                         const sal_uInt16 nIdx, const sal_uInt16 nLen, sal_Int32* pDXArray ) const
{
    if ( !IsCaseMap() && !IsKern() )
        return Size( pOut->GetTextArray( rTxt, pDXArray, nIdx, nLen ),
                     pOut->GetTextHeight() );

    Size aTxtSize;
    aTxtSize.setHeight( pOut->GetTextHeight() );
    if ( !IsCaseMap() )
        aTxtSize.setWidth( pOut->GetTextArray( rTxt, pDXArray, nIdx, nLen ) );
    else
        aTxtSize.setWidth( pOut->GetTextArray( CalcCaseMap( rTxt ),
                           pDXArray, nIdx, nLen ) );

    if( IsKern() && ( nLen > 1 ) )
    {
        aTxtSize.Width() += ( ( nLen-1 ) * long( nKern ) );

        if ( pDXArray )
        {
            for ( xub_StrLen i = 0; i < nLen; i++ )
                pDXArray[i] += ( (i+1) * long( nKern ) );
            // Der letzte ist um ein nKern zu gross:
            pDXArray[nLen-1] -= nKern;
        }
    }
    return aTxtSize;
}

/*************************************************************************
 *                    SvxFont::GetTxtSize()
 *************************************************************************/

Size SvxFont::GetTxtSize( const OutputDevice *pOut, const XubString &rTxt,
                         const xub_StrLen nIdx, const xub_StrLen nLen )
{
    xub_StrLen nTmp = nLen;
    if ( nTmp == STRING_LEN )   // schon initialisiert?
        nTmp = rTxt.Len();
    Font aOldFont( ChgPhysFont((OutputDevice *)pOut) );
    Size aTxtSize;
    if( IsCapital() && rTxt.Len() )
    {
        aTxtSize = GetCapitalSize( pOut, rTxt, nIdx, nTmp );
    }
    else aTxtSize = GetPhysTxtSize(pOut,rTxt,nIdx,nTmp);
    ((OutputDevice *)pOut)->SetFont( aOldFont );
    return aTxtSize;
}

/*************************************************************************
 *                    SvxFont::DrawText()
 *************************************************************************/

void SvxFont::DrawText( OutputDevice *pOut,
               const Point &rPos, const XubString &rTxt,
               const xub_StrLen nIdx, const xub_StrLen nLen ) const
{
    if( !nLen || !rTxt.Len() )  return;
    xub_StrLen nTmp = nLen;
    if ( nTmp == STRING_LEN )   // schon initialisiert?
        nTmp = rTxt.Len();
    Point aPos( rPos );
    if ( nEsc )
    {
        Size aSize = (this->GetSize());
        aPos.Y() -= ((nEsc*long(aSize.Height()))/ 100L);
    }
    Font aOldFont( ChgPhysFont( pOut ) );

    if ( IsCapital() )
        DrawCapital( pOut, aPos, rTxt, nIdx, nTmp );
    else
    {
        Size aSize = GetPhysTxtSize( pOut, rTxt, nIdx, nTmp );

        if ( !IsCaseMap() )
            pOut->DrawStretchText( aPos, aSize.Width(), rTxt, nIdx, nTmp );
        else
            pOut->DrawStretchText( aPos, aSize.Width(), CalcCaseMap( rTxt ),
                                   nIdx, nTmp );
    }
    pOut->SetFont(aOldFont);
}

void SvxFont::QuickDrawText( OutputDevice *pOut,
    const Point &rPos, const XubString &rTxt,
    const xub_StrLen nIdx, const xub_StrLen nLen, const sal_Int32* pDXArray ) const
{
    // Font muss ins OutputDevice selektiert sein...
    if ( !IsCaseMap() && !IsCapital() && !IsKern() && !IsEsc() )
    {
        pOut->DrawTextArray( rPos, rTxt, pDXArray, nIdx, nLen );
        return;
    }

    Point aPos( rPos );

    if ( nEsc )
    {
        long nDiff = GetSize().Height();
        nDiff *= nEsc;
        nDiff /= 100;

        if ( !IsVertical() )
            aPos.Y() -= nDiff;
        else
            aPos.X() += nDiff;
    }

    if( IsCapital() )
    {
        DBG_ASSERT( !pDXArray, "DrawCapital nicht fuer TextArray!" );
        DrawCapital( pOut, aPos, rTxt, nIdx, nLen );
    }
    else
    {
        if ( IsKern() && !pDXArray )
        {
            Size aSize = GetPhysTxtSize( pOut, rTxt, nIdx, nLen );

            if ( !IsCaseMap() )
                pOut->DrawStretchText( aPos, aSize.Width(), rTxt, nIdx, nLen );
            else
                pOut->DrawStretchText( aPos, aSize.Width(), CalcCaseMap( rTxt ), nIdx, nLen );
        }
        else
        {
            if ( !IsCaseMap() )
                pOut->DrawTextArray( aPos, rTxt, pDXArray, nIdx, nLen );
            else
                pOut->DrawTextArray( aPos, CalcCaseMap( rTxt ), pDXArray, nIdx, nLen );
        }
    }
}

// -----------------------------------------------------------------------

void SvxFont::DrawPrev( OutputDevice *pOut, Printer* pPrinter,
                        const Point &rPos, const XubString &rTxt,
                        const xub_StrLen nIdx, const xub_StrLen nLen ) const
{
    if ( !nLen || !rTxt.Len() )
        return;
    xub_StrLen nTmp = nLen;

    if ( nTmp == STRING_LEN )   // schon initialisiert?
        nTmp = rTxt.Len();
    Point aPos( rPos );

    if ( nEsc )
    {
        short nTmpEsc;
        if( DFLT_ESC_AUTO_SUPER == nEsc )
            nTmpEsc = 33;
        else if( DFLT_ESC_AUTO_SUB == nEsc )
            nTmpEsc = -20;
        else
            nTmpEsc = nEsc;
        Size aSize = ( this->GetSize() );
        aPos.Y() -= ( ( nTmpEsc * long( aSize.Height() ) ) / 100L );
    }
    Font aOldFont( ChgPhysFont( pOut ) );
    Font aOldPrnFont( ChgPhysFont( pPrinter ) );

    if ( IsCapital() )
        DrawCapital( pOut, aPos, rTxt, nIdx, nTmp );
    else
    {
        Size aSize = GetPhysTxtSize( pPrinter, rTxt, nIdx, nTmp );

        if ( !IsCaseMap() )
            pOut->DrawStretchText( aPos, aSize.Width(), rTxt, nIdx, nTmp );
        else
        {
            // #108210#
            const XubString aNewText = CalcCaseMap(rTxt);
            sal_Bool bCaseMapLengthDiffers(aNewText.Len() != rTxt.Len());

            if(bCaseMapLengthDiffers)
            {
                // If strings differ work preparing the necessary snippet to address that
                // potential difference
                const XubString aSnippet(rTxt, nIdx, nTmp);
                XubString _aNewText = CalcCaseMap(aSnippet);

                pOut->DrawStretchText( aPos, aSize.Width(), _aNewText, 0, _aNewText.Len() );
            }
            else
            {
                pOut->DrawStretchText( aPos, aSize.Width(), CalcCaseMap( rTxt ), nIdx, nTmp );
            }
        }
    }
    pOut->SetFont(aOldFont);
    pPrinter->SetFont( aOldPrnFont );
}

// -----------------------------------------------------------------------

SvxFont& SvxFont::operator=( const Font& rFont )
{
    Font::operator=( rFont );
    return *this;
}

SvxFont& SvxFont::operator=( const SvxFont& rFont )
{
    Font::operator=( rFont );
    eLang = rFont.eLang;
    eCaseMap = rFont.eCaseMap;
    nEsc = rFont.nEsc;
    nPropr = rFont.nPropr;
    nKern = rFont.nKern;
    return *this;
}


/*************************************************************************
 *                    class SvxDoGetCapitalSize
 * wird von SvxFont::GetCapitalSize() zur Berechnung der TxtSize bei
 * eingestellten Kapitaelchen benutzt.
 *************************************************************************/

class SvxDoGetCapitalSize : public SvxDoCapitals
{
protected:
    SvxFont*    pFont;
    Size        aTxtSize;
    short       nKern;
public:
      SvxDoGetCapitalSize( SvxFont *_pFnt, const OutputDevice *_pOut,
                           const XubString &_rTxt, const xub_StrLen _nIdx,
                           const xub_StrLen _nLen, const short _nKrn )
            : SvxDoCapitals( (OutputDevice*)_pOut, _rTxt, _nIdx, _nLen ),
              pFont( _pFnt ),
              nKern( _nKrn )
            { }

    virtual void Do( const XubString &rTxt, const xub_StrLen nIdx,
                     const xub_StrLen nLen, const sal_Bool bUpper );

    inline const Size &GetSize() const { return aTxtSize; };
};

void SvxDoGetCapitalSize::Do( const XubString &_rTxt, const xub_StrLen _nIdx,
                              const xub_StrLen _nLen, const sal_Bool bUpper )
{
    Size aPartSize;
    if ( !bUpper )
    {
        sal_uInt8 nProp = pFont->GetPropr();
        pFont->SetProprRel( KAPITAELCHENPROP );
        pFont->SetPhysFont( pOut );
        aPartSize.setWidth( pOut->GetTextWidth( _rTxt, _nIdx, _nLen ) );
        aPartSize.setHeight( pOut->GetTextHeight() );
        aTxtSize.Height() = aPartSize.Height();
        pFont->SetPropr( nProp );
        pFont->SetPhysFont( pOut );
    }
    else
    {
        aPartSize.setWidth( pOut->GetTextWidth( _rTxt, _nIdx, _nLen ) );
        aPartSize.setHeight( pOut->GetTextHeight() );
    }
    aTxtSize.Width() += aPartSize.Width();
    aTxtSize.Width() += ( _nLen * long( nKern ) );
}

/*************************************************************************
 *                    SvxFont::GetCapitalSize()
 * berechnet TxtSize, wenn Kapitaelchen eingestellt sind.
 *************************************************************************/

Size SvxFont::GetCapitalSize( const OutputDevice *pOut, const XubString &rTxt,
                             const xub_StrLen nIdx, const xub_StrLen nLen) const
{
    // Start:
    SvxDoGetCapitalSize aDo( (SvxFont *)this, pOut, rTxt, nIdx, nLen, nKern );
    DoOnCapitals( aDo );
    Size aTxtSize( aDo.GetSize() );

    // End:
    if( !aTxtSize.Height() )
    {
        aTxtSize.setWidth( 0 );
        aTxtSize.setHeight( pOut->GetTextHeight() );
    }
    return aTxtSize;
}

/*************************************************************************
 *                     class SvxDoDrawCapital
 * wird von SvxFont::DrawCapital zur Ausgabe von Kapitaelchen benutzt.
 *************************************************************************/

class SvxDoDrawCapital : public SvxDoCapitals
{
protected:
    SvxFont *pFont;
    Point aPos;
    Point aSpacePos;
    short nKern;
public:
    SvxDoDrawCapital( SvxFont *pFnt, OutputDevice *_pOut, const XubString &_rTxt,
                      const xub_StrLen _nIdx, const xub_StrLen _nLen,
                      const Point &rPos, const short nKrn )
        : SvxDoCapitals( _pOut, _rTxt, _nIdx, _nLen ),
          pFont( pFnt ),
          aPos( rPos ),
          aSpacePos( rPos ),
          nKern( nKrn )
        { }
    virtual void DoSpace( const sal_Bool bDraw );
    virtual void SetSpace();
    virtual void Do( const XubString &rTxt, const xub_StrLen nIdx,
                     const xub_StrLen nLen, const sal_Bool bUpper );
};

void SvxDoDrawCapital::DoSpace( const sal_Bool bDraw )
{
    if ( bDraw || pFont->IsWordLineMode() )
    {
        sal_uInt16 nDiff = (sal_uInt16)(aPos.X() - aSpacePos.X());
        if ( nDiff )
        {
            sal_Bool bWordWise = pFont->IsWordLineMode();
            sal_Bool bTrans = pFont->IsTransparent();
            pFont->SetWordLineMode( sal_False );
            pFont->SetTransparent( sal_True );
            pFont->SetPhysFont( pOut );
            pOut->DrawStretchText( aSpacePos, nDiff, XubString( sDoubleSpace,
                            RTL_TEXTENCODING_MS_1252 ), 0, 2 );
            pFont->SetWordLineMode( bWordWise );
            pFont->SetTransparent( bTrans );
            pFont->SetPhysFont( pOut );
        }
    }
}

void SvxDoDrawCapital::SetSpace()
{
    if ( pFont->IsWordLineMode() )
        aSpacePos.X() = aPos.X();
}

void SvxDoDrawCapital::Do( const XubString &_rTxt, const xub_StrLen _nIdx,
                           const xub_StrLen _nLen, const sal_Bool bUpper)
{
    sal_uInt8 nProp = 0;
    Size aPartSize;

    // Einstellen der gewuenschten Fonts
    FontUnderline eUnder = pFont->GetUnderline();
    FontStrikeout eStrike = pFont->GetStrikeout();
    pFont->SetUnderline( UNDERLINE_NONE );
    pFont->SetStrikeout( STRIKEOUT_NONE );
    if ( !bUpper )
    {
        nProp = pFont->GetPropr();
        pFont->SetProprRel( KAPITAELCHENPROP );
    }
    pFont->SetPhysFont( pOut );

    aPartSize.setWidth( pOut->GetTextWidth( _rTxt, _nIdx, _nLen ) );
    aPartSize.setHeight( pOut->GetTextHeight() );
    long nWidth = aPartSize.Width();
    if ( nKern )
    {
        aPos.X() += (nKern/2);
        if ( _nLen ) nWidth += (_nLen*long(nKern));
    }
    pOut->DrawStretchText(aPos,nWidth-nKern,_rTxt,_nIdx,_nLen);

    // Font restaurieren
    pFont->SetUnderline( eUnder );
    pFont->SetStrikeout( eStrike );
    if ( !bUpper )
        pFont->SetPropr( nProp );
    pFont->SetPhysFont( pOut );

    aPos.X() += nWidth-(nKern/2);
}

/*************************************************************************
 * SvxFont::DrawCapital() gibt Kapitaelchen aus.
 *************************************************************************/

void SvxFont::DrawCapital( OutputDevice *pOut,
               const Point &rPos, const XubString &rTxt,
               const xub_StrLen nIdx, const xub_StrLen nLen ) const
{
    SvxDoDrawCapital aDo( (SvxFont *)this,pOut,rTxt,nIdx,nLen,rPos,nKern );
    DoOnCapitals( aDo );
}

#endif // !REDUCEDSVXFONT


