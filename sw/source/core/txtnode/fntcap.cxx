/*************************************************************************
 *
 *  $RCSfile: fntcap.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: jp $ $Date: 2000-11-20 16:22:08 $
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

#ifdef PRECOMPILED
#include "core_pch.hxx"
#endif

#pragma hdrstop

#ifndef _OUTDEV_HXX //autogen
#include <vcl/outdev.hxx>
#endif
#ifndef _COM_SUN_STAR_I18N_CHARTYPE_HDL
#include <com/sun/star/i18n/CharType.hdl>
#endif
#ifndef _PRINT_HXX //autogen
#include <vcl/print.hxx>
#endif

#ifndef _ERRHDL_HXX
#include <errhdl.hxx>
#endif
#ifndef _FNTCACHE_HXX
#include <fntcache.hxx>
#endif
#ifndef _SWFONT_HXX
#include <swfont.hxx>
#endif
#ifndef _DRAWFONT_HXX
#include <drawfont.hxx>
#endif
#ifndef _BREAKIT_HXX
#include <breakit.hxx>
#endif

using namespace ::com::sun::star::i18n;


#define KAPITAELCHENPROP 66

/*************************************************************************
 *                      class SwDoCapitals
 *************************************************************************/

class SwDoCapitals
{
protected:
    SwDrawTextInfo &rInf;
public:
    SwDoCapitals ( SwDrawTextInfo &rInfo ) : rInf( rInfo ) { }
    virtual void Init( SwFntObj *pUpperFont, SwFntObj *pLowerFont ) = 0;
    virtual void Do() = 0;
    inline OutputDevice *GetOut() { return rInf.GetpOut(); }
    inline SwDrawTextInfo& GetInf() { return rInf; }
};

/*************************************************************************
 *                    class SwDoGetCapitalSize
 *************************************************************************/

class SwDoGetCapitalSize : public SwDoCapitals
{
protected:
    Size aTxtSize;
public:
    SwDoGetCapitalSize( SwDrawTextInfo &rInfo ) : SwDoCapitals ( rInfo ) { }
    virtual void Init( SwFntObj *pUpperFont, SwFntObj *pLowerFont );
    virtual void Do();
    const Size &GetSize() const { return aTxtSize; }
};

void SwDoGetCapitalSize::Init( SwFntObj *pUpperFont, SwFntObj *pLowerFont )
{
    aTxtSize.Height() = 0;
    aTxtSize.Width() = 0;
}

void SwDoGetCapitalSize::Do()
{
    aTxtSize.Width() += rInf.GetSize().Width();
    if( rInf.GetUpper() )
        aTxtSize.Height() = rInf.GetSize().Height();
}

/*************************************************************************
 *                    SwSubFont::GetCapitalSize()
 *************************************************************************/

Size SwSubFont::GetCapitalSize( ViewShell *pSh,
                             const OutputDevice *pOut, const XubString &rTxt,
                             const xub_StrLen nIdx, const xub_StrLen nLen )
{
    // Start:
    Point aPos( 0, 0 );
    SwDrawTextInfo aInfo( pSh, *(OutputDevice *)pOut, rTxt, nIdx, nLen, 0, FALSE );
    aInfo.SetPos( aPos );
    aInfo.SetWrong( NULL );
    aInfo.SetDrawSpace( FALSE );
    aInfo.SetKern( CheckKerning() );
    aInfo.SetSpace( 0 );
    SwDoGetCapitalSize aDo( aInfo );
    DoOnCapitals( aDo );
    Size aTxtSize( aDo.GetSize() );

    // End:
    if( !aTxtSize.Height() )
    {
        SV_STAT( nGetTextSize );
        aTxtSize.Height() = short ( pOut->GetTextHeight() );
    }
    return aTxtSize;
}

/*************************************************************************
 *                    class SwDoGetCapitalBreak
 *************************************************************************/

class SwDoGetCapitalBreak : public SwDoCapitals
{
protected:
    xub_StrLen *pExtraPos;
    long nTxtWidth;
    xub_StrLen nBreak;
public:
    SwDoGetCapitalBreak( SwDrawTextInfo &rInfo, long nWidth, xub_StrLen *pExtra)
        :   SwDoCapitals ( rInfo ), nTxtWidth( nWidth ),
            nBreak( STRING_LEN ), pExtraPos( pExtra )
        { }
    virtual void Init( SwFntObj *pUpperFont, SwFntObj *pLowerFont );
    virtual void Do();
    const xub_StrLen GetBreak() const { return nBreak; }
};

void SwDoGetCapitalBreak::Init( SwFntObj *pUpperFont, SwFntObj *pLowerFont )
{
}

void SwDoGetCapitalBreak::Do()
{
    if ( nTxtWidth )
    {
        if ( rInf.GetSize().Width() < nTxtWidth )
            nTxtWidth -= rInf.GetSize().Width();
        else
        {
            xub_StrLen nEnd = rInf.GetEnd();
            if( pExtraPos )
            {
                nBreak = GetOut()->GetTextBreak( rInf.GetText(), nTxtWidth, '-',
                     *pExtraPos, rInf.GetIdx(), rInf.GetLen(), rInf.GetKern() );
                if( *pExtraPos > nEnd )
                    *pExtraPos = nEnd;
            }
            else
                nBreak = GetOut()->GetTextBreak( rInf.GetText(), nTxtWidth,
                               rInf.GetIdx(), rInf.GetLen(), rInf.GetKern() );
            if( nBreak > nEnd )
                nBreak = nEnd;
            nTxtWidth = 0;
        }
    }
}

/*************************************************************************
 *                    SwFont::GetCapitalSize()
 *************************************************************************/

xub_StrLen SwFont::GetCapitalBreak( ViewShell *pSh, const OutputDevice *pOut,
        const XubString &rTxt, long nTextWidth, xub_StrLen *pExtra,
        const xub_StrLen nIdx, const xub_StrLen nLen )
{
    // Start:
    Point aPos( 0, 0 );
    SwDrawTextInfo aInfo(pSh, *(OutputDevice*)pOut, rTxt, nIdx, nLen, 0, FALSE);
    aInfo.SetPos( aPos );
    aInfo.SetSpace( 0 );
    aInfo.SetWrong( NULL );
    aInfo.SetDrawSpace( FALSE );
    aInfo.SetKern( CheckKerning() );
    SwDoGetCapitalBreak aDo( aInfo, nTextWidth, pExtra );
    DoOnCapitals( aDo );
    return aDo.GetBreak();
}

/*************************************************************************
 *                     class SwDoDrawCapital
 *************************************************************************/

class SwDoDrawCapital : public SwDoCapitals
{
protected:
    SwFntObj *pUpperFnt;
    SwFntObj *pLowerFnt;
public:
    SwDoDrawCapital( SwDrawTextInfo &rInfo ) :
        SwDoCapitals( rInfo )
        { }
    virtual void Init( SwFntObj *pUpperFont, SwFntObj *pLowerFont );
    virtual void Do();
    void DrawSpace( Point &rPos );
};

void SwDoDrawCapital::Init( SwFntObj *pUpperFont, SwFntObj *pLowerFont )
{
    pUpperFnt = pUpperFont;
    pLowerFnt = pLowerFont;
}

void SwDoDrawCapital::Do()
{
    SV_STAT( nDrawText );
    USHORT nOrgWidth = rInf.GetWidth();
    rInf.SetWidth( USHORT(rInf.GetSize().Width()) );
    if ( rInf.GetUpper() )
        pUpperFnt->DrawText( rInf );
    else
    {
        BOOL bOldBullet = rInf.GetBullet();
        rInf.SetBullet( FALSE );
        pLowerFnt->DrawText( rInf );
        rInf.SetBullet( bOldBullet );
    }
    rInf.ShiftX();
    rInf.SetWidth( nOrgWidth );
}

/*************************************************************************
 *                    SwDoDrawCapital::DrawSpace()
 *************************************************************************/

void SwDoDrawCapital::DrawSpace( Point &rPos )
{
    static sal_Char __READONLY_DATA sDoubleSpace[] = "  ";
    const USHORT nDiff = (USHORT)( rInf.GetPos().X() - rPos.X() );
    if ( nDiff )
        GetOut()->DrawStretchText( rPos, nDiff,
            XubString( sDoubleSpace, RTL_TEXTENCODING_MS_1252 ), 0, 2 );
    rPos.X() = rInf.GetPos().X() + rInf.GetWidth();
}

/*************************************************************************
 *                    SwSubFont::DrawCapital()
 *************************************************************************/

void SwSubFont::DrawCapital( SwDrawTextInfo &rInf )
{
    // Es wird vorausgesetzt, dass rPos bereits kalkuliert ist!
    // hochgezogen in SwFont: const Point aPos( CalcPos(rPos) );
    rInf.SetDrawSpace( GetUnderline() != UNDERLINE_NONE ||
                       GetStrikeout() != STRIKEOUT_NONE );
    SwDoDrawCapital aDo( rInf );
    DoOnCapitals( aDo );
}

/*************************************************************************
 *                     class SwDoDrawCapital
 *************************************************************************/

class SwDoCapitalCrsrOfst : public SwDoCapitals
{
protected:
    SwFntObj *pUpperFnt;
    SwFntObj *pLowerFnt;
    xub_StrLen nCrsr;
    USHORT nOfst;
public:
    SwDoCapitalCrsrOfst( SwDrawTextInfo &rInfo, const USHORT nOfs ) :
        SwDoCapitals( rInfo ), nOfst( nOfs ), nCrsr( 0 )
        { }
    virtual void Init( SwFntObj *pUpperFont, SwFntObj *pLowerFont );
    virtual void Do();

    void DrawSpace( const Point &rPos );
    inline xub_StrLen GetCrsr(){ return nCrsr; }
};

void SwDoCapitalCrsrOfst::Init( SwFntObj *pUpperFont, SwFntObj *pLowerFont )
{
    pUpperFnt = pUpperFont;
    pLowerFnt = pLowerFont;
}

void SwDoCapitalCrsrOfst::Do()
{
    if ( nOfst )
    {
        if ( nOfst > rInf.GetSize().Width() )
        {
            nOfst -= USHORT(rInf.GetSize().Width());
            nCrsr += rInf.GetLen();
        }
        else
        {
            if ( rInf.GetUpper() )
                nCrsr += pUpperFnt->GetCrsrOfst( rInf.GetpOut(), rInf.GetText(),
                         nOfst, rInf.GetIdx(), rInf.GetLen(), rInf.GetKern() );
            else
                nCrsr += pLowerFnt->GetCrsrOfst( rInf.GetpOut(), rInf.GetText(),
                         nOfst, rInf.GetIdx(), rInf.GetLen(), rInf.GetKern(),
                         rInf.GetSpace() );
            nOfst = 0;
        }
    }
}

/*************************************************************************
 *                    SwSubFont::GetCapitalCrsrOfst()
 *************************************************************************/

xub_StrLen SwSubFont::GetCapitalCrsrOfst( ViewShell *pSh,
    OutputDevice *pOut, const XubString &rTxt, const USHORT nOfst,
    const xub_StrLen nIdx, const xub_StrLen nLen, const short nSpaceAdd )
{
    Point aPos( 0, 0 );
    SwDrawTextInfo aInfo(pSh, *(OutputDevice*)pOut, rTxt, nIdx, nLen, 0, FALSE);
    aInfo.SetPos( aPos );
    aInfo.SetWrong( NULL );
    aInfo.SetDrawSpace( FALSE );
    aInfo.SetKern( CheckKerning() );
    aInfo.SetSpace( nSpaceAdd );
    SwDoCapitalCrsrOfst aDo( aInfo, nOfst );
    DoOnCapitals( aDo );
    return aDo.GetCrsr();
}

/*************************************************************************
 *                    class SwDoDrawStretchCapital
 *************************************************************************/

class SwDoDrawStretchCapital : public SwDoDrawCapital
{
    const xub_StrLen nStrLen;
    const USHORT nCapWidth;
    const USHORT nOrgWidth;
public:
    virtual void Do();

    SwDoDrawStretchCapital( SwDrawTextInfo &rInfo, const USHORT nCapWidth )
            : SwDoDrawCapital( rInfo ),
              nCapWidth( nCapWidth ),
              nOrgWidth( rInfo.GetWidth() ),
              nStrLen( rInfo.GetLen() )
        { }
};

/*************************************************************************
 *                    SwDoDrawStretchCapital
 *************************************************************************/

void SwDoDrawStretchCapital::Do()
{
    SV_STAT( nDrawStretchText );
    USHORT nPartWidth = USHORT(rInf.GetSize().Width());

    if( rInf.GetLen() )
    {
        // 4023: Kapitaelchen und Kerning.
        long nDiff = long(nOrgWidth) - long(nCapWidth);
        if( nDiff )
        {
            nDiff *= rInf.GetLen();
            nDiff /= (long) nStrLen;
            nDiff += nPartWidth;
            if( 0 < nDiff )
                nPartWidth = USHORT(nDiff);
        }

        // Optimierung:
        if( 1 >= rInf.GetLen() )
            GetOut()->DrawText( rInf.GetPos(), rInf.GetText(), rInf.GetIdx(),
                rInf.GetLen() );
        else
            GetOut()->DrawStretchText( rInf.GetPos(), nPartWidth,
                                rInf.GetText(), rInf.GetIdx(), rInf.GetLen() );
    }
    ((Point&)rInf.GetPos()).X() += nPartWidth;
}

/*************************************************************************
 *                    SwSubFont::DrawStretchCapital()
 *************************************************************************/

void SwSubFont::DrawStretchCapital( SwDrawTextInfo &rInf )
{
    // Es wird vorausgesetzt, dass rPos bereits kalkuliert ist!
    // hochgezogen in SwFont: const Point aPos( CalcPos(rPos) );

    // 4023: nie das Kerning draufrechnen, weil wir die
    // unmanipulierte Width brauchen.
    const USHORT nCapWidth = (USHORT)( GetCapitalSize( rInf.GetShell(),
        rInf.GetpOut(), rInf.GetText(), rInf.GetIdx(), rInf.GetLen() ) ).Width();
    if( rInf.GetLen() == STRING_LEN )
        rInf.SetLen( rInf.GetText().Len() );
    rInf.SetDrawSpace( GetUnderline() != UNDERLINE_NONE ||
                       GetStrikeout() != STRIKEOUT_NONE );
    SwDoDrawStretchCapital aDo( rInf, nCapWidth );
    DoOnCapitals( aDo );
}

/*************************************************************************
 *                  SwSubFont::DoOnCapitals() const
 *************************************************************************/

void SwSubFont::DoOnCapitals( SwDoCapitals &rDo )
{
    ASSERT( pLastFont, "SwFont::DoOnCapitals: No LastFont?!" );

    Size aPartSize;
    const XubString aTxt( CalcCaseMap( rDo.GetInf().GetText() ) );
    xub_StrLen nMaxPos = Min( USHORT(rDo.GetInf().GetText().Len()
                            - rDo.GetInf().GetIdx()), rDo.GetInf().GetLen() );
    rDo.GetInf().SetLen( nMaxPos );

    const XubString& rOldText = rDo.GetInf().GetText();
    rDo.GetInf().SetText( aTxt );
    rDo.GetInf().SetSize( aPartSize );
    xub_StrLen nPos = rDo.GetInf().GetIdx();
    xub_StrLen nOldPos = nPos;
    nMaxPos += nPos;

    SwFntObj *pOldLast = pLastFont;
    SwFntAccess *pBigFontAccess = NULL;
    SwFntObj *pBigFont;
    SwFntAccess *pSpaceFontAccess = NULL;
    SwFntObj *pSpaceFont = NULL;

    const void *pMagic2 = NULL;
    USHORT nIndex2 = 0;
    SwSubFont aFont( *this );
    Point aStartPos( rDo.GetInf().GetPos() );

    const BOOL bUnderStriked = aFont.GetUnderline() != UNDERLINE_NONE
                            || aFont.GetStrikeout() != STRIKEOUT_NONE;
    const BOOL bWordWise = bUnderStriked && aFont.IsWordLineMode() &&
                           rDo.GetInf().GetDrawSpace();
    const short nKern = rDo.GetInf().GetKern();

    if ( bUnderStriked )
    {
        if ( bWordWise )
        {
            aFont.SetWordLineMode( FALSE );
            pSpaceFontAccess = new SwFntAccess( pMagic2, nIndex2, &aFont,
                                                rDo.GetInf().GetShell() );
            pSpaceFont = pSpaceFontAccess->Get();
        }
        else
            pSpaceFont = pLastFont;

        // Wir basteln uns einen Font fuer die Grossbuchstaben:
        aFont.SetUnderline( UNDERLINE_NONE );
        aFont.SetStrikeout( STRIKEOUT_NONE );
        pMagic2 = NULL;
        nIndex2 = 0;
        pBigFontAccess = new SwFntAccess( pMagic2, nIndex2, &aFont,
                                          rDo.GetInf().GetShell() );
        pBigFont = pBigFontAccess->Get();
    }
    else
        pBigFont = pLastFont;

    // Hier entsteht der Kleinbuchstabenfont:
    aFont.SetProportion( BYTE( (aFont.GetPropr()*KAPITAELCHENPROP) / 100L) );
    pMagic2 = NULL;
    nIndex2 = 0;
    SwFntAccess *pSmallFontAccess = new SwFntAccess( pMagic2, nIndex2, &aFont,
                                                     rDo.GetInf().GetShell() );
    SwFntObj *pSmallFont = pSmallFontAccess->Get();

    rDo.Init( pBigFont, pSmallFont );
    OutputDevice* pOutSize = pSmallFont->GetPrt();
    if( !pOutSize )
        pOutSize = rDo.GetOut();

    const LanguageType eLng = LANGUAGE_DONTKNOW == GetLanguage()
                            ? LANGUAGE_SYSTEM : GetLanguage();

    if( nPos < nMaxPos )
    {
        nPos = pBreakIt->xBreak->endOfCharBlock( rOldText, nPos,
            pBreakIt->GetLocale( eLng ), CharType::LOWER_CASE_CHAR );
        if( nPos == STRING_LEN )
            nPos = nOldPos;
        else if( nPos > nMaxPos )
            nPos = nMaxPos;
    }

    while( nOldPos < nMaxPos )
    {

        //  The lower ones...
        if( nOldPos != nPos )
        {
            SV_STAT( nGetTextSize );
            pLastFont = pSmallFont;
            pLastFont->SetDevFont( rDo.GetInf().GetShell(), rDo.GetOut() );
            rDo.GetInf().SetIdx( nOldPos );
            rDo.GetInf().SetLen( nPos - nOldPos );
            rDo.GetInf().SetUpper( FALSE );
            aPartSize = pSmallFont->GetTextSize( rDo.GetInf().GetShell(),
                 pOutSize, rDo.GetInf().GetText(), rDo.GetInf().GetIdx(),
                 rDo.GetInf().GetLen(), rDo.GetInf().GetKern() );
            if( nKern && nPos < nMaxPos )
                aPartSize.Width() += nKern;
            rDo.Do();
            nOldPos = nPos;
        }
        nPos = pBreakIt->xBreak->nextCharBlock( rOldText, nPos,
               pBreakIt->GetLocale( eLng ), CharType::LOWER_CASE_CHAR );
        if( nPos == STRING_LEN || nPos > nMaxPos )
            nPos = nMaxPos;
        ASSERT( nPos, "nextCharBlock not implemented?" );
#ifndef PRODUCT
        if( !nPos )
            nPos = nMaxPos;
#endif
        // The upper ones...
        if( nOldPos != nPos )
        {
            do
            {
                rDo.GetInf().SetUpper( TRUE );
                pLastFont = pBigFont;
                pLastFont->SetDevFont( rDo.GetInf().GetShell(), rDo.GetOut() );
                xub_StrLen nTmp;
                if( bWordWise )
                {
                    nTmp = nOldPos;
                    while( nTmp < nPos && CH_BLANK == rOldText.GetChar( nTmp ) )
                        ++nTmp;
                    if( nOldPos < nTmp )
                    {
                        pLastFont = pSpaceFont;
                        pLastFont->SetDevFont( rDo.GetInf().GetShell(),
                                               rDo.GetOut() );
                        ((SwDoDrawCapital&)rDo).DrawSpace( aStartPos );
                        pLastFont = pBigFont;
                        pLastFont->SetDevFont( rDo.GetInf().GetShell(),
                                               rDo.GetOut() );
                        rDo.GetInf().SetIdx( nOldPos );
                        rDo.GetInf().SetLen( nTmp - nOldPos );
                        aPartSize = pBigFont->GetTextSize( rDo.GetInf().
                            GetShell(), pOutSize, aTxt, rDo.GetInf().GetIdx(),
                              rDo.GetInf().GetLen(), rDo.GetInf().GetKern() );
                        if( rDo.GetInf().GetSpace() )
                            aPartSize.Width() += rDo.GetInf().GetSpace() *
                                                 ( nTmp - nOldPos );
                        if( nKern && nPos < nMaxPos )
                            aPartSize.Width() += nKern;
                        rDo.Do();
                        aStartPos = rDo.GetInf().GetPos();
                        nOldPos = nTmp;
                    }
                    while( nTmp < nPos && CH_BLANK != rOldText.GetChar( nTmp ) )
                        ++nTmp;
                }
                else
                    nTmp = nPos;
                if( nTmp > nOldPos )
                {
                    rDo.GetInf().SetIdx( nOldPos );
                    rDo.GetInf().SetLen( nTmp - nOldPos );
                    aPartSize = pBigFont->GetTextSize( rDo.GetInf().GetShell(),
                                  pOutSize, aTxt, rDo.GetInf().GetIdx(),
                                  rDo.GetInf().GetLen(), rDo.GetInf().GetKern() );
                    if( !bWordWise && rDo.GetInf().GetSpace() )
                        for( xub_StrLen nI = nOldPos; nI < nPos; ++nI )
                            if( CH_BLANK == rOldText.GetChar( nI ) )
                            aPartSize.Width() += rDo.GetInf().GetSpace();
                    if( nKern && nPos < nMaxPos )
                        aPartSize.Width() += nKern;
                    rDo.Do();
                    nOldPos = nTmp;
                }
            } while( nOldPos != nPos );
        }
        nPos = pBreakIt->xBreak->endOfCharBlock( rOldText, nPos,
               pBreakIt->GetLocale( eLng ), CharType::LOWER_CASE_CHAR );
        if( nPos == STRING_LEN || nPos > nMaxPos )
            nPos = nMaxPos;
        ASSERT( nPos, "endOfCharBlock not implemented?" );
#ifndef PRODUCT
        if( !nPos )
            nPos = nMaxPos;
#endif
    }

    // Aufraeumen:
    if( pBigFont != pOldLast )
        delete pBigFontAccess;

    if( bUnderStriked )
    {
        if( rDo.GetInf().GetDrawSpace() )
        {
            pLastFont = pSpaceFont;
            pLastFont->SetDevFont( rDo.GetInf().GetShell(), rDo.GetOut() );
            ( (SwDoDrawCapital&) rDo ).DrawSpace( aStartPos );
        }
        if ( bWordWise )
            delete pSpaceFontAccess;
    }
    pLastFont = pOldLast;
    pLastFont->SetDevFont( rDo.GetInf().GetShell(), rDo.GetOut() );

    delete pSmallFontAccess;
    rDo.GetInf().SetText( rOldText );
}


