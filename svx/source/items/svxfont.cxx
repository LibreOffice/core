/*************************************************************************
 *
 *  $RCSfile: svxfont.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: er $ $Date: 2000-10-29 17:11:30 $
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

// include ----------------------------------------------------------------

#ifndef _OUTDEV_HXX //autogen
#include <vcl/outdev.hxx>
#endif
#ifndef _PRINT_HXX //autogen
#include <vcl/print.hxx>
#endif
#ifndef _SV_POLY_HXX //autogen
#include <vcl/poly.hxx>
#endif
#ifndef _UNOTOOLS_CHARCLASS_HXX
#include <unotools/charclass.hxx>
#endif
#ifndef _UNO_LINGU_HXX
#include <unolingu.hxx>
#endif
#ifndef _COM_SUN_STAR_I18N_KCHARACTERTYPE_HPP_
#include <com/sun/star/i18n/KCharacterType.hpp>
#endif
#pragma hdrstop

#define ITEMID_ESCAPEMENT   0

#define _SVX_SVXFONT_CXX

#include "svxfont.hxx"
#include "escpitem.hxx"

// Minimum: Prozentwert fuers kernen
#define MINKERNPERCENT 5

// prop. Groesse der Kleinbuchstaben bei Kapitaelchen
#define KAPITAELCHENPROP 66

#ifndef REDUCEDSVXFONT
    const sal_Unicode CH_BLANK = sal_Unicode(' ');      // ' ' Leerzeichen
    const XubString aDoubleSpace = (sal_Unicode*)"  ";
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
    const Size& rSize, const Color& rCol, BOOL bLeft )
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
            BOOL bBlank = TRUE;

            for( UINT32 i = 0; i < aTxt.Len(); ++i )
            {
                if( sal_Unicode(' ') == aTxt.GetChar(i) || sal_Unicode('\t') == aTxt.GetChar(i) )
                    bBlank = TRUE;
                else
                {
                    if( bBlank )
                    {
                        String aTemp( aTxt.GetChar( i ) );
                        aCharClass.toUpper( aTemp );
                        aTxt.Replace( i, 1, aTemp );
                    }
                    bBlank = FALSE;
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
    SvxDoCapitals( OutputDevice *pOut, const XubString &rTxt,
                   const xub_StrLen nIdx, const xub_StrLen nLen )
        : pOut(pOut), rTxt(rTxt), nIdx(nIdx), nLen(nLen)
        { }

    virtual void DoSpace( const BOOL bDraw );
    virtual void SetSpace();
    virtual void Do( const XubString &rTxt,
                     const xub_StrLen nIdx, const xub_StrLen nLen,
                     const BOOL bUpper ) = 0;

    inline OutputDevice *GetOut() { return pOut; }
    inline const XubString &GetTxt() const { return rTxt; }
    inline const xub_StrLen GetIdx() const { return nIdx; }
    inline const xub_StrLen GetLen() const { return nLen; }
};

void SvxDoCapitals::DoSpace( const BOOL bDraw ) { }

void SvxDoCapitals::SetSpace() { }

void SvxDoCapitals::Do( const XubString &rTxt, const xub_StrLen nIdx,
    const xub_StrLen nLen, const BOOL bUpper ) { }

/*************************************************************************
 *                  SvxFont::DoOnCapitals() const
 * zerlegt den String in Gross- und Kleinbuchstaben und ruft jeweils die
 * Methode SvxDoCapitals::Do( ) auf.
 *************************************************************************/

void SvxFont::DoOnCapitals(SvxDoCapitals &rDo, const xub_StrLen nPartLen) const
{
    OutputDevice *pOut = rDo.GetOut();
    const XubString &rTxt = rDo.GetTxt();
    const xub_StrLen nIdx = rDo.GetIdx();
    const xub_StrLen nLen = STRING_LEN == nPartLen ? rDo.GetLen() : nPartLen;

    const XubString aTxt( CalcCaseMap( rTxt ) );
    const USHORT nTxtLen = Min( rTxt.Len(), nLen );
    USHORT nPos = 0;
    USHORT nOldPos = nPos;

    const LanguageType eLng = LANGUAGE_DONTKNOW == eLang
                            ? LANGUAGE_SYSTEM : eLang;

    CharClass   aCharClass( SvxCreateLocale( eLng ) );
    String      aCharString;

    while( nPos < nTxtLen )
    {
        // Erst kommen die Upper-Chars dran
        aCharString = rTxt.GetChar( nPos + nIdx );

        // 4251: Es gibt Zeichen, die Upper _und_ Lower sind (z.B. das Blank).
        // Solche Zweideutigkeiten fuehren ins Chaos, deswegen werden diese
        // Zeichen der Menge Lower zugeordnet !

        while( nPos < nTxtLen )
        {
            sal_Int32 nCharacterType = aCharClass.getCharacterType( aCharString, 0 );
            if ( nCharacterType & ::com::sun::star::i18n::KCharacterType::LOWER )
                break;
            if ( ! ( nCharacterType & ::com::sun::star::i18n::KCharacterType::UPPER ) )
                break;
            aCharString = rTxt.GetChar( ++nPos + nIdx );
        }
        if( nOldPos != nPos )
        {
            rDo.Do( aTxt, nIdx + nOldPos, nPos-nOldPos, TRUE );
            nOldPos = nPos;
        }
        // Nun werden die Lower-Chars verarbeitet (ohne Blanks)
        while( nPos < nTxtLen )
        {
            sal_uInt32  nCharacterType = aCharClass.getCharacterType( aCharString, 0 );
            if ( ! ( nCharacterType & ::com::sun::star::i18n::KCharacterType::LOWER ) )
                break;
            if ( CH_BLANK == aCharString )
                break;
            aCharString = rTxt.GetChar( ++nPos + nIdx );
        }
        if( nOldPos != nPos )
        {
            rDo.Do( aTxt, nIdx + nOldPos, nPos-nOldPos, FALSE );
            nOldPos = nPos;
        }
        // Nun werden die Blanks verarbeitet
        while( nPos < nTxtLen && CH_BLANK == aCharString )
            aCharString = rTxt.GetChar( ++nPos + nIdx );

        if( nOldPos != nPos )
        {
            rDo.DoSpace( FALSE );
            rDo.Do( aTxt, nIdx + nOldPos, nPos - nOldPos, FALSE );
            nOldPos = nPos;
            rDo.SetSpace();
        }
    }
    rDo.DoSpace( TRUE );
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
        aTxtSize.setWidth(pOut->GetTextWidth( CalcCaseMap(rTxt), nIdx, nLen ));

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
                         const USHORT nIdx, const USHORT nLen, long* pDXArray ) const
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
    const xub_StrLen nIdx, const xub_StrLen nLen, const long* pDXArray ) const
{
    // Font muss ins OutputDevice selektiert sein...
    if ( !IsCaseMap() && !IsCapital() && !IsKern() && !IsEsc() )
    {
        pOut->DrawTextArray( rPos, rTxt, pDXArray, nIdx, nLen );
        return;
    }

    Point aPos( rPos );
    if ( nEsc )
        aPos.Y() -= nEsc * GetSize().Height() / 100L ;

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
            pOut->DrawStretchText( aPos, aSize.Width(), CalcCaseMap( rTxt ),
                                   nIdx, nTmp );
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
      SvxDoGetCapitalSize( SvxFont *pFnt, const OutputDevice *pOut,
                           const XubString &rTxt, const xub_StrLen nIdx,
                           const xub_StrLen nLen, const short nKrn )
            : SvxDoCapitals( (OutputDevice*)pOut, rTxt, nIdx, nLen ),
              pFont( pFnt ),
              nKern( nKrn )
            { }

    virtual void Do( const XubString &rTxt, const xub_StrLen nIdx,
                     const xub_StrLen nLen, const BOOL bUpper );

    inline const Size &GetSize() const { return aTxtSize; };
};

void SvxDoGetCapitalSize::Do( const XubString &rTxt, const xub_StrLen nIdx,
                              const xub_StrLen nLen, const BOOL bUpper )
{
    Size aPartSize;
    if ( !bUpper )
    {
        BYTE nProp = pFont->GetPropr();
        pFont->SetProprRel( KAPITAELCHENPROP );
        pFont->SetPhysFont( pOut );
        aPartSize.setWidth( pOut->GetTextWidth( rTxt, nIdx, nLen ) );
        aPartSize.setHeight( pOut->GetTextHeight() );
        aTxtSize.Height() = aPartSize.Height();
        pFont->SetPropr( nProp );
        pFont->SetPhysFont( pOut );
    }
    else
    {
        aPartSize.setWidth( pOut->GetTextWidth( rTxt, nIdx, nLen ) );
        aPartSize.setHeight( pOut->GetTextHeight() );
    }
    aTxtSize.Width() += aPartSize.Width();
    aTxtSize.Width() += ( nLen * long( nKern ) );
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
    SvxDoDrawCapital( SvxFont *pFnt, OutputDevice *pOut, const XubString &rTxt,
                      const xub_StrLen nIdx, const xub_StrLen nLen,
                      const Point &rPos, const short nKrn )
        : SvxDoCapitals( pOut, rTxt, nIdx, nLen ),
          pFont( pFnt ),
          aPos( rPos ),
          aSpacePos( rPos ),
          nKern( nKrn )
        { }
    virtual void DoSpace( const BOOL bDraw );
    virtual void SetSpace();
    virtual void Do( const XubString &rTxt, const xub_StrLen nIdx,
                     const xub_StrLen nLen, const BOOL bUpper );
};

void SvxDoDrawCapital::DoSpace( const BOOL bDraw )
{
    if ( bDraw || pFont->IsWordLineMode() )
    {
        USHORT nDiff = (USHORT)(aPos.X() - aSpacePos.X());
        if ( nDiff )
        {
            BOOL bWordWise = pFont->IsWordLineMode();
            BOOL bTrans = pFont->IsTransparent();
            pFont->SetWordLineMode( FALSE );
            pFont->SetTransparent( TRUE );
            pFont->SetPhysFont( pOut );
            pOut->DrawStretchText( aSpacePos, nDiff, aDoubleSpace, 0, 2 );
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

void SvxDoDrawCapital::Do( const XubString &rTxt, const xub_StrLen nIdx,
                           const xub_StrLen nLen, const BOOL bUpper)
{
    BYTE nProp;
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

    aPartSize.setWidth( pOut->GetTextWidth( rTxt, nIdx, nLen ) );
    aPartSize.setHeight( pOut->GetTextHeight() );
    long nWidth = aPartSize.Width();
    if ( nKern )
    {
        aPos.X() += (nKern/2);
        if ( nLen ) nWidth += (nLen*long(nKern));
    }
    pOut->DrawStretchText(aPos,nWidth-nKern,rTxt,nIdx,nLen);

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


