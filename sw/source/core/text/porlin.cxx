/*************************************************************************
 *
 *  $RCSfile: porlin.cxx,v $
 *
 *  $Revision: 1.20 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-17 14:28:52 $
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


#pragma hdrstop

#ifdef BIDI
#ifndef _OUTDEV_HXX //autogen
#include <vcl/outdev.hxx>
#endif
#endif
#ifndef _SW_PORTIONHANDLER_HXX
#include <SwPortionHandler.hxx>
#endif

#include "errhdl.hxx"   // ASSERT

#include "txtcfg.hxx"
#include "porlin.hxx"
#include "inftxt.hxx"
#include "portxt.hxx"
#include "pormulti.hxx"
#include "porglue.hxx"
#include "inftxt.hxx"
#include "blink.hxx"
#ifndef PRODUCT

sal_Bool ChkChain( SwLinePortion *pStart )
{
    SwLinePortion *pPor = pStart->GetPortion();
    MSHORT nCount = 0;
    while( pPor )
    {
        ++nCount;
        ASSERT( nCount < 200 && pPor != pStart,
                "ChkChain(): lost in chains" );
        if( nCount >= 200 || pPor == pStart )
        {
            // der Lebensretter
            pPor = pStart->GetPortion();
            pStart->SetPortion(0);
            pPor->Truncate();
            pStart->SetPortion( pPor );
            return sal_False;
        }
        pPor = pPor->GetPortion();
    }
    return sal_True;
}
#endif

#if OSL_DEBUG_LEVEL > 1
const sal_Char *GetPortionName( const MSHORT nType );
#endif

SwLinePortion::~SwLinePortion()
{
    if( pBlink )
        pBlink->Delete( this );
}

SwLinePortion *SwLinePortion::Compress()
{
    return GetLen() || Width() ? this : 0;
}

KSHORT SwLinePortion::GetViewWidth( const SwTxtSizeInfo & ) const
{
    return 0;
}

/*************************************************************************
 *               SwLinePortion::SwLinePortion( )
 *************************************************************************/

SwLinePortion::SwLinePortion( ) :
    nLineLength( 0 ),
    nAscent( 0 ),
    pPortion( NULL )
{
}

/*************************************************************************
 *               SwLinePortion::PrePaint()
 *************************************************************************/

void SwLinePortion::PrePaint( const SwTxtPaintInfo& rInf,
                              const SwLinePortion* pLast ) const
{
    ASSERT( rInf.OnWin(), "SwLinePortion::PrePaint: don't prepaint on a printer");
    ASSERT( !Width(), "SwLinePortion::PrePaint: For Width()==0 only!");

    const KSHORT nViewWidth = GetViewWidth( rInf );

    if( ! nViewWidth )
        return;

    const KSHORT nHalfView = nViewWidth / 2;
    USHORT nLastWidth = pLast->Width();

    if ( pLast->InSpaceGrp() && rInf.GetSpaceAdd() )
        nLastWidth += (USHORT)pLast->CalcSpacing( rInf.GetSpaceAdd(), rInf );

    KSHORT nPos;
    SwTxtPaintInfo aInf( rInf );

#ifdef BIDI
    const BOOL bBidiPor = ( rInf.GetTxtFrm()->IsRightToLeft() ) !=
                          ( 0 != ( TEXT_LAYOUT_BIDI_RTL & rInf.GetOut()->GetLayoutMode() ) );

    USHORT nDir = bBidiPor ?
                  1800 :
                  rInf.GetFont()->GetOrientation( rInf.GetTxtFrm()->IsVertical() );
#else
    USHORT nDir = rInf.GetFont()->GetOrientation( rInf.GetTxtFrm()->IsVertical() );
#endif

    switch ( nDir )
    {
    case 0 :
        nPos = KSHORT( rInf.X() );
        if( nLastWidth > nHalfView )
            nPos += nLastWidth - nHalfView;
        aInf.X( nPos );
        break;
    case 900 :
        nPos = KSHORT( rInf.Y() );
        if( nLastWidth > nHalfView )
            nPos -= nLastWidth + nHalfView;
        aInf.Y( nPos );
        break;
#ifdef BIDI
    case 1800 :
        nPos = KSHORT( rInf.X() );
        if( nLastWidth > nHalfView )
            nPos -= nLastWidth + nHalfView;
        aInf.X( nPos );
        break;
#endif
    case 2700 :
        nPos = KSHORT( rInf.Y() );
        if( nLastWidth > nHalfView )
            nPos += nLastWidth - nHalfView;
        aInf.Y( nPos );
        break;
    }

    SwLinePortion *pThis = (SwLinePortion*)this;
    pThis->Width( nViewWidth );
    Paint( aInf );
    pThis->Width(0);
}

/*************************************************************************
 *                  SwLinePortion::CalcTxtSize()
 *************************************************************************/

void SwLinePortion::CalcTxtSize( const SwTxtSizeInfo &rInf )
{
    if( GetLen() == rInf.GetLen()  )
        *((SwPosSize*)this) = GetTxtSize( rInf );
    else
    {
        SwTxtSizeInfo aInf( rInf );
        aInf.SetLen( GetLen() );
        *((SwPosSize*)this) = GetTxtSize( aInf );
    }
}

/*************************************************************************
 *                  SwLinePortion::Truncate()
 *
 * Es werden alle nachfolgenden Portions geloescht.
 *************************************************************************/

void SwLinePortion::_Truncate()
{
    SwLinePortion *pPos = pPortion;
    do
    {   ASSERT( pPos != this, "SwLinePortion::Truncate: loop" );
        SwLinePortion *pLast = pPos;
        pPos = pPos->GetPortion();
        pLast->SetPortion( 0 );
        delete pLast;

    } while( pPos );

    pPortion = 0;
}

/*************************************************************************
 *                virtual SwLinePortion::Insert()
 *
 * Es wird immer hinter uns eingefuegt.
 *************************************************************************/

SwLinePortion *SwLinePortion::Insert( SwLinePortion *pIns )
{
    pIns->FindLastPortion()->SetPortion( pPortion );
    SetPortion( pIns );
#ifndef PRODUCT
    ChkChain( this );
#endif
    return pIns;
}

/*************************************************************************
 *                  SwLinePortion::FindLastPortion()
 *************************************************************************/

SwLinePortion *SwLinePortion::FindLastPortion()
{
    register SwLinePortion *pPos = this;
    // An das Ende wandern und pLinPortion an den letzten haengen ...
    while( pPos->GetPortion() )
    {
        DBG_LOOP;
        pPos = pPos->GetPortion();
    }
    return pPos;
}

/*************************************************************************
 *                virtual SwLinePortion::Append()
 *************************************************************************/

SwLinePortion *SwLinePortion::Append( SwLinePortion *pIns )
{
    SwLinePortion *pPos = FindLastPortion();
    pPos->SetPortion( pIns );
    pIns->SetPortion( 0 );
#ifndef PRODUCT
    ChkChain( this );
#endif
    return pIns;
}

/*************************************************************************
 *                virtual SwLinePortion::Cut()
 *************************************************************************/

SwLinePortion *SwLinePortion::Cut( SwLinePortion *pVictim )
{
    SwLinePortion *pPrev = pVictim->FindPrevPortion( this );
    ASSERT( pPrev, "SwLinePortion::Cut(): can't cut" );
    pPrev->SetPortion( pVictim->GetPortion() );
    pVictim->SetPortion(0);
    return pVictim;
}

/*************************************************************************
 *                SwLinePortion::FindPrevPortion()
 *************************************************************************/

SwLinePortion *SwLinePortion::FindPrevPortion( const SwLinePortion *pRoot )
{
    ASSERT( pRoot != this, "SwLinePortion::FindPrevPortion(): invalid root" );
    SwLinePortion *pPos = (SwLinePortion*)pRoot;
    while( pPos->GetPortion() && pPos->GetPortion() != this )
    {
        DBG_LOOP;
        pPos = pPos->GetPortion();
    }
    ASSERT( pPos->GetPortion(),
            "SwLinePortion::FindPrevPortion: blowing in the wind");
    return pPos;
}

/*************************************************************************
 *                virtual SwLinePortion::GetCrsrOfst()
 *************************************************************************/

xub_StrLen SwLinePortion::GetCrsrOfst( const KSHORT nOfst ) const
{
    if( nOfst > ( PrtWidth() / 2 ) )
        return GetLen();
    else
        return 0;
}

/*************************************************************************
 *                virtual SwLinePortion::GetTxtSize()
 *************************************************************************/

SwPosSize SwLinePortion::GetTxtSize( const SwTxtSizeInfo & ) const
{
    ASSERT( !this, "SwLinePortion::GetTxtSize: don't ask me about sizes, "
                   "I'm only a stupid SwLinePortion" );
    return SwPosSize();
}

#ifndef PRODUCT

/*************************************************************************
 *                virtual SwLinePortion::Check()
 *************************************************************************/

sal_Bool SwLinePortion::Check( SvStream &, SwTxtSizeInfo & ) //$ ostream
{
#ifdef USED
    SwPosSize aSize( GetTxtSize( rInfo ) );
    if( Width() != aSize.Width() )
    {
        CONSTCHAR( pDbgTxt, "Error in width of SwLinePortion: " );
        rOs << pDbgTxt << endl;
        operator<<( rOs );
        rOs << endl;
        rOs << "Had: " << Width() << " And: " << aSize.Width();
        return sal_False;
    }
#endif /* USED */
    return sal_True;
}
#endif

/*************************************************************************
 *                 virtual SwLinePortion::Format()
 *************************************************************************/

sal_Bool SwLinePortion::Format( SwTxtFormatInfo &rInf )
{
    if( rInf.X() > rInf.Width() )
    {
        Truncate();
        rInf.SetUnderFlow( this );
        return sal_True;
    }

    register const SwLinePortion *pLast = rInf.GetLast();
    Height( pLast->Height() );
    SetAscent( pLast->GetAscent() );
    const KSHORT nNewWidth = rInf.X() + PrtWidth();
    // Nur Portions mit echter Breite koennen ein sal_True zurueckliefern
    // Notizen beispielsweise setzen niemals bFull==sal_True
    if( rInf.Width() <= nNewWidth && PrtWidth() && ! IsKernPortion() )
    {
        Truncate();
        if( nNewWidth > rInf.Width() )
            PrtWidth( nNewWidth - rInf.Width() );
        rInf.GetLast()->FormatEOL( rInf );
        return sal_True;
    }
    return sal_False;
}

/*************************************************************************
 *                 virtual SwLinePortion::FormatEOL()
 *************************************************************************/

// Format end of line

void SwLinePortion::FormatEOL( SwTxtFormatInfo &rInf )
{ }

/*************************************************************************
 *                      SwLinePortion::Move()
 *************************************************************************/

void SwLinePortion::Move( SwTxtPaintInfo &rInf )
{
    BOOL bB2T = rInf.GetDirection() == DIR_BOTTOM2TOP;
#ifdef BIDI
    const BOOL bFrmDir = rInf.GetTxtFrm()->IsRightToLeft();
    BOOL bCounterDir = ( ! bFrmDir && DIR_RIGHT2LEFT == rInf.GetDirection() ) ||
                       (   bFrmDir && DIR_LEFT2RIGHT == rInf.GetDirection() );
#endif

    if ( InSpaceGrp() && rInf.GetSpaceAdd() )
    {
        SwTwips nTmp = PrtWidth() + CalcSpacing( rInf.GetSpaceAdd(), rInf );
        if( rInf.IsRotated() )
            rInf.Y( rInf.Y() + ( bB2T ? -nTmp : nTmp ) );
#ifdef BIDI
        else if ( bCounterDir )
            rInf.X( rInf.X() - nTmp );
#endif
        else
            rInf.X( rInf.X() + nTmp );
    }
    else
    {
        if( InFixMargGrp() && !IsMarginPortion() )
        {
            rInf.IncSpaceIdx();
            rInf.IncKanaIdx();
        }
        if( rInf.IsRotated() )
            rInf.Y( rInf.Y() + ( bB2T ? -PrtWidth() : PrtWidth() ) );
#ifdef BIDI
        else if ( bCounterDir )
            rInf.X( rInf.X() - PrtWidth() );
#endif
        else
            rInf.X( rInf.X() + PrtWidth() );
    }
    if( IsMultiPortion() && ((SwMultiPortion*)this)->HasTabulator() )
        rInf.IncSpaceIdx();

    rInf.SetIdx( rInf.GetIdx() + GetLen() );
}

/*************************************************************************
 *              virtual SwLinePortion::CalcSpacing()
 *************************************************************************/

long SwLinePortion::CalcSpacing( short nSpaceAdd, const SwTxtSizeInfo &rInf ) const
{
    return 0;
}

/*************************************************************************
 *              virtual SwLinePortion::GetExpTxt()
 *************************************************************************/

sal_Bool SwLinePortion::GetExpTxt( const SwTxtSizeInfo &rInf, XubString &rTxt ) const
{
    return sal_False;
}

/*************************************************************************
 *              virtual SwLinePortion::HandlePortion()
 *************************************************************************/

void SwLinePortion::HandlePortion( SwPortionHandler& rPH ) const
{
    String aString;
    rPH.Special( GetLen(), aString, GetWhichPor() );
}

