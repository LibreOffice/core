/*************************************************************************
 *
 *  $RCSfile: frmmgr.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:37 $
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
#include "ui_pch.hxx"
#endif

#pragma hdrstop

#include "cmdid.h"
#ifndef _UIPARAM_HXX
#include <uiparam.hxx>
#endif
#include "hintids.hxx"


#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif
#ifndef _SVX_PROTITEM_HXX //autogen
#include <svx/protitem.hxx>
#endif
#ifndef _SVX_BOXITEM_HXX //autogen
#include <svx/boxitem.hxx>
#endif
#ifndef _SVX_OPAQITEM_HXX //autogen
#include <svx/opaqitem.hxx>
#endif
#ifndef _SVX_LRSPITEM_HXX //autogen
#include <svx/lrspitem.hxx>
#endif
#ifndef _SVX_ULSPITEM_HXX //autogen
#include <svx/ulspitem.hxx>
#endif
#ifndef _SVX_SHADITEM_HXX //autogen
#include <svx/shaditem.hxx>
#endif


#ifndef _FMTCLDS_HXX //autogen
#include <fmtclds.hxx>
#endif
#include "wrtsh.hxx"
#include "view.hxx"
#include "viewopt.hxx"
#include "uitool.hxx"
#include "frmmgr.hxx"
#include "format.hxx"
#include "mdiexp.hxx"
#include "poolfmt.hxx"

static USHORT __FAR_DATA aFrmMgrRange[] = {
                            RES_FRMATR_BEGIN, RES_FRMATR_END-1,
                            SID_ATTR_BORDER_INNER, SID_ATTR_BORDER_INNER,
                            FN_SET_FRM_NAME, FN_SET_FRM_NAME,
                            0};

/*--------------------------------------------------------------------
     Beschreibung: Rahmen-Attribute ueber Shell ermitteln
 --------------------------------------------------------------------*/

SwFlyFrmAttrMgr::SwFlyFrmAttrMgr( BOOL bNew, SwWrtShell* pSh, BYTE nType ) :
    bAbsPos( FALSE ),
    bNewFrm( bNew ),
    aSet( (SwAttrPool&)pSh->GetAttrPool(), aFrmMgrRange ),
    pOwnSh( pSh )

{
    if ( bNewFrm )
    {
        // Defaults einstellen:
        USHORT nId;
        switch ( nType )
        {
            case FRMMGR_TYPE_TEXT:  nId = RES_POOLFRM_FRAME;    break;
            case FRMMGR_TYPE_OLE:   nId = RES_POOLFRM_OLE;      break;
            case FRMMGR_TYPE_GRF:   nId = RES_POOLFRM_GRAPHIC;  break;
        }
        aSet.SetParent( &pOwnSh->GetFmtFromPool( nId )->GetAttrSet());
        aSet.Put( SwFmtFrmSize( ATT_MIN_SIZE, DFLT_WIDTH, DFLT_HEIGHT ));
        if ( 0 != ::GetHtmlMode(pSh->GetView().GetDocShell()) )
            aSet.Put( SwFmtHoriOrient( 0, HORI_LEFT, PRTAREA ) );
    }
    else if ( nType == FRMMGR_TYPE_NONE )
        pOwnSh->GetFlyFrmAttr( aSet );
    ::PrepareBoxInfo( aSet, *pOwnSh );
}

SwFlyFrmAttrMgr::SwFlyFrmAttrMgr( BOOL bNew, SwWrtShell* pSh, const SfxItemSet &rSet ) :
    bAbsPos( FALSE ),
    bNewFrm( bNew ),
    aSet( rSet ),
    pOwnSh( pSh )

{
}


/*--------------------------------------------------------------------
     Beschreibung:  Initialisieren
 --------------------------------------------------------------------*/

void SwFlyFrmAttrMgr::UpdateAttrMgr()
{
    if ( !bNewFrm && pOwnSh->IsFrmSelected() )
        pOwnSh->GetFlyFrmAttr( aSet );
    ::PrepareBoxInfo( aSet, *pOwnSh );
}

void SwFlyFrmAttrMgr::_UpdateFlyFrm()
{
    const SfxPoolItem* pItem = 0;

    if (aSet.GetItemState(FN_SET_FRM_NAME, FALSE, &pItem) == SFX_ITEM_SET)
        pOwnSh->SetFlyName(((SfxStringItem *)pItem)->GetValue());

    pOwnSh->SetModified();

    if ( bAbsPos )
    {
        pOwnSh->SetFlyPos( aAbsPos );
        bAbsPos = FALSE;
    }
}

/*--------------------------------------------------------------------
    Beschreibung: Bestehenden Fly-Frame aendern
 --------------------------------------------------------------------*/

void SwFlyFrmAttrMgr::UpdateFlyFrm()
{
    ASSERT( pOwnSh->IsFrmSelected(),
        "Kein Rahmen selektiert oder keine Shell, Update nicht moeglich");

    // return wg. BASIC
    if( !aSet.Count() || !pOwnSh->IsFrmSelected() )
        return;

    pOwnSh->StartAllAction();
    pOwnSh->SetFlyFrmAttr( aSet );
    _UpdateFlyFrm();
    pOwnSh->EndAllAction();
}

/*--------------------------------------------------------------------
     Beschreibung:  Rahmen einfuegen
 --------------------------------------------------------------------*/

BOOL SwFlyFrmAttrMgr::InsertFlyFrm()
{
    pOwnSh->StartAllAction();

    BOOL bRet = 0 != pOwnSh->NewFlyFrm( aSet );

    // richtigen Mode an der Shell einschalten, Rahmen wurde aut. selektiert.
    if ( bRet )
    {
        _UpdateFlyFrm();
        pOwnSh->EnterSelFrmMode();
        FrameNotify(pOwnSh, FLY_DRAG_START);
    }
    pOwnSh->EndAllAction();
    return bRet;
}

/*------------------------------------------------------------------------
 Beschreibung:  Rahmen des Typs eAnchorType einfuegen. Position und
                Groesse werden explizit angegeben.
                Nicht erlaubte Werte des Aufzaehlungstypes werden
                korrigiert.
------------------------------------------------------------------------*/

void SwFlyFrmAttrMgr::InsertFlyFrm(RndStdIds    eAnchorType,
                                   const Point  &rPos,
                                   const Size   &rSize,
                                   BOOL bAbs )
{
    ASSERT( eAnchorType == FLY_PAGE     ||
            eAnchorType == FLY_AT_CNTNT ||
            eAnchorType == FLY_AUTO_CNTNT ||
            eAnchorType == FLY_AT_FLY ||
            eAnchorType == FLY_IN_CNTNT, "Rahmentyp nicht erlaubt" );

    if ( bAbs )
        SetAbsPos( rPos );
    else
        SetPos( rPos );

    SetSize( rSize );
    SetAnchor( eAnchorType );
    InsertFlyFrm();
}

/*--------------------------------------------------------------------
     Beschreibung:  Anker setzen
 --------------------------------------------------------------------*/

void SwFlyFrmAttrMgr::SetAnchor( RndStdIds eId )
{
    USHORT nPhyPageNum, nVirtPageNum;
    pOwnSh->GetPageNum( nPhyPageNum, nVirtPageNum );

    aSet.Put( SwFmtAnchor( eId, nPhyPageNum ) );
    if( FLY_PAGE == eId || FLY_AT_CNTNT == eId || FLY_AUTO_CNTNT == eId
        || FLY_AT_FLY == eId )
    {
        SwFmtVertOrient aVertOrient( GetVertOrient() );
        SwFmtHoriOrient aHoriOrient( GetHoriOrient() );
        aHoriOrient.SetRelationOrient( FRAME );
        aVertOrient.SetRelationOrient( FRAME );
        aSet.Put( aVertOrient );
        aSet.Put( aHoriOrient );
    }
}

/*------------------------------------------------------------------------
 Beschreibung:  Setzen des Attributs fuer Spalten
------------------------------------------------------------------------*/

void SwFlyFrmAttrMgr::SetCol( const SwFmtCol &rCol )
{
    aSet.Put( rCol );
}
/*--------------------------------------------------------------------
     Beschreibung:  Absolute Position setzen
 --------------------------------------------------------------------*/

void SwFlyFrmAttrMgr::SetAbsPos( const Point& rPoint )
{
    bAbsPos = TRUE;
    aAbsPos = rPoint;

    SwFmtVertOrient aVertOrient( GetVertOrient() );
    SwFmtHoriOrient aHoriOrient( GetHoriOrient() );
    aHoriOrient.SetHoriOrient( HORI_NONE );
    aVertOrient.SetVertOrient( VERT_NONE );
    aSet.Put( aVertOrient );
    aSet.Put( aHoriOrient );
}

/*--------------------------------------------------------------------
    Beschreibung: Metriken auf Korrektheit pruefen
 --------------------------------------------------------------------*/

void SwFlyFrmAttrMgr::ValidateMetrics( SwFrmValid& rVal, BOOL bOnlyPercentRefValue )
{
    if (!bOnlyPercentRefValue)
    {
        rVal.nMinHeight = MINFLY + CalcTopSpace() + CalcBottomSpace();
        rVal.nMinWidth =  MINFLY + CalcLeftSpace()+ CalcRightSpace();
    }

    SwRect aBoundRect;

    pOwnSh->CalcBoundRect(aBoundRect, rVal.eArea, rVal.eHRel, rVal.bMirror, NULL, &rVal.aPercentSize);

    if (bOnlyPercentRefValue)
        return;

    if ( rVal.eArea == FLY_PAGE || rVal.eArea == FLY_AT_FLY )
    {
        // MinimalPosition
        rVal.nMinHPos = aBoundRect.Left();
        rVal.nMinVPos = aBoundRect.Top();
        SwTwips nH = rVal.nHPos;
        SwTwips nV = rVal.nVPos;

        if (rVal.nHPos + rVal.nWidth > aBoundRect.Right())
        {
            if (rVal.eHori == HORI_NONE)
            {
                rVal.nHPos -= ((rVal.nHPos + rVal.nWidth) - aBoundRect.Right());
                nH = rVal.nHPos;
            }
            else
                rVal.nWidth = aBoundRect.Right() - rVal.nHPos;
        }

        if (rVal.nHPos + rVal.nWidth > aBoundRect.Right())
            rVal.nWidth = aBoundRect.Right() - rVal.nHPos;

        if (rVal.nVPos + rVal.nHeight > aBoundRect.Bottom())
        {
            if (rVal.eVert == SVX_VERT_NONE)
            {
                rVal.nVPos -= ((rVal.nVPos + rVal.nHeight) - aBoundRect.Bottom());
                nV = rVal.nVPos;
            }
            else
                rVal.nHeight = aBoundRect.Bottom() - rVal.nVPos;
        }

        if (rVal.nVPos + rVal.nHeight > aBoundRect.Bottom())
            rVal.nHeight = aBoundRect.Bottom() - rVal.nVPos;

        if ( rVal.eVert != SVX_VERT_NONE )
            nV = aBoundRect.Top();

        if ( rVal.eHori != HORI_NONE )
            nH = aBoundRect.Left();

        rVal.nMaxHPos   = aBoundRect.Right()  - rVal.nWidth;
        rVal.nMaxHeight = aBoundRect.Bottom() - nV;

        rVal.nMaxVPos   = aBoundRect.Bottom() - rVal.nHeight;
        rVal.nMaxWidth  = aBoundRect.Right()  - nH;
    }
    else if ( rVal.eArea == FLY_AT_CNTNT || rVal.eArea == FLY_AUTO_CNTNT )
    {
        SwTwips nH = rVal.nHPos;
        SwTwips nV = rVal.nVPos;

        if (rVal.nHPos + rVal.nWidth > aBoundRect.Right())
        {
            if (rVal.eHori == HORI_NONE)
            {
                rVal.nHPos -= ((rVal.nHPos + rVal.nWidth) - aBoundRect.Right());
                nH = rVal.nHPos;
            }
            else
                rVal.nWidth = aBoundRect.Right() - rVal.nHPos;
        }

        if (rVal.nVPos + rVal.nHeight > aBoundRect.Bottom())
        {
            if (rVal.eVert == SVX_VERT_NONE)
            {
                rVal.nVPos -= ((rVal.nVPos + rVal.nHeight) - aBoundRect.Bottom());
                nV = rVal.nVPos;
            }
            else
                rVal.nHeight = aBoundRect.Bottom() - rVal.nVPos;
        }

        if ( rVal.eVert != SVX_VERT_NONE )
            nV = aBoundRect.Top();

        if ( rVal.eHori != HORI_NONE )
            nH = aBoundRect.Left();

        rVal.nMinVPos  = aBoundRect.Top();
        rVal.nMaxVPos  = aBoundRect.Height() - rVal.nHeight;

        rVal.nMinHPos  = aBoundRect.Left();
        rVal.nMaxHPos  = aBoundRect.Right() - rVal.nWidth;

        // Maximale Breite Hoehe
        rVal.nMaxHeight  = rVal.nMaxVPos + rVal.nHeight - nV;
        rVal.nMaxWidth   = rVal.nMaxHPos + rVal.nWidth - nH;
    }
    else if ( rVal.eArea == FLY_IN_CNTNT )
    {
        rVal.nMinHPos = 0;
        rVal.nMaxHPos = 0;

        rVal.nMaxHeight = aBoundRect.Height();
        rVal.nMaxWidth  = aBoundRect.Width();

        rVal.nMaxVPos   = aBoundRect.Height();
        rVal.nMinVPos   = -aBoundRect.Height() + rVal.nHeight;
        if (rVal.nMaxVPos < rVal.nMinVPos)
        {
            rVal.nMinVPos = rVal.nMaxVPos;
            rVal.nMaxVPos = -aBoundRect.Height();
        }
    }
    if (rVal.nMaxWidth < rVal.nWidth)
        rVal.nWidth = rVal.nMaxWidth;
    if (rVal.nMaxHeight < rVal.nHeight)
        rVal.nHeight = rVal.nMaxHeight;
}

/*--------------------------------------------------------------------
    Beschreibung: Korrektur fuer Umrandung
 --------------------------------------------------------------------*/

long SwFlyFrmAttrMgr::CalcWidthSpace()
{
    SvxLRSpaceItem &rLR = (SvxLRSpaceItem&)aSet.Get(RES_LR_SPACE);
    return rLR.GetLeft() + rLR.GetRight() + CalcLeftSpace() + CalcRightSpace();
}

long SwFlyFrmAttrMgr::CalcHeightSpace()
{
    SvxULSpaceItem &rUL = (SvxULSpaceItem&)aSet.Get(RES_UL_SPACE);
    return rUL.GetUpper() + rUL.GetLower() + CalcTopSpace() + CalcBottomSpace();
}

SwTwips SwFlyFrmAttrMgr::CalcTopSpace()
{
    const SvxShadowItem& rShadow = GetShadow();
    const SvxBoxItem&    rBox    = GetBox();
    return rShadow.CalcShadowSpace(SHADOW_TOP ) + rBox.CalcLineSpace(BOX_LINE_TOP);
}

SwTwips SwFlyFrmAttrMgr::CalcBottomSpace()
{
    const SvxShadowItem& rShadow = GetShadow();
    const SvxBoxItem& rBox       = GetBox();
    return rShadow.CalcShadowSpace(SHADOW_BOTTOM) + rBox.CalcLineSpace(BOX_LINE_BOTTOM);
}

SwTwips SwFlyFrmAttrMgr::CalcLeftSpace()
{
    const SvxShadowItem& rShadow = GetShadow();
    const SvxBoxItem&    rBox    = GetBox();
    return rShadow.CalcShadowSpace(SHADOW_LEFT) + rBox.CalcLineSpace(BOX_LINE_LEFT);
}

SwTwips SwFlyFrmAttrMgr::CalcRightSpace()
{
    const SvxShadowItem& rShadow = GetShadow();
    const SvxBoxItem&    rBox    = GetBox();
    return rShadow.CalcShadowSpace(SHADOW_RIGHT) + rBox.CalcLineSpace(BOX_LINE_RIGHT);
}


/*--------------------------------------------------------------------
    Beschreibung: Attribut aus dem Set loeschen
 --------------------------------------------------------------------*/
void SwFlyFrmAttrMgr::DelAttr( USHORT nId )
{
    aSet.ClearItem( nId );
}

void SwFlyFrmAttrMgr::SetLRSpace( long nLeft, long nRight )
{
    ASSERT( LONG_MAX != nLeft && LONG_MAX != nRight, "Welchen Raend setzen?" );

    SvxLRSpaceItem aTmp( (SvxLRSpaceItem&)aSet.Get( RES_LR_SPACE ) );
    if( LONG_MAX != nLeft )
        aTmp.SetLeft( USHORT(nLeft) );
    if( LONG_MAX != nRight )
        aTmp.SetRight( USHORT(nRight) );
    aSet.Put( aTmp );
}

void SwFlyFrmAttrMgr::SetULSpace( long nTop, long nBottom )
{
    ASSERT( LONG_MAX != nTop && LONG_MAX != nBottom, Welchen Raend setzen? );

    SvxULSpaceItem aTmp( (SvxULSpaceItem&)aSet.Get( RES_UL_SPACE ) );
    if( LONG_MAX != nTop )
        aTmp.SetUpper( USHORT(nTop) );
    if( LONG_MAX != nBottom )
        aTmp.SetLower( USHORT(nBottom) );
    aSet.Put( aTmp );
}

void SwFlyFrmAttrMgr::SetWrapType( SwSurround eFly )
{
    SwFmtSurround aWrap( GetSurround() );
    aWrap.SetSurround( eFly );
    aSet.Put( aWrap );
}

void SwFlyFrmAttrMgr::SetAnchorOnly(BOOL bSet)
{
    SwFmtSurround aWrap( GetSurround() );
    aWrap.SetAnchorOnly(bSet);
    aSet.Put( aWrap );
}

void SwFlyFrmAttrMgr::SetContour(   BOOL bSet)
{
    SwFmtSurround aWrap( GetSurround() );
    aWrap.SetContour(bSet);
    aSet.Put( aWrap );
}

void SwFlyFrmAttrMgr::SetPos( const Point& rPoint )
{
    SwFmtVertOrient aVertOrient( GetVertOrient() );
    SwFmtHoriOrient aHoriOrient( GetHoriOrient() );

    aHoriOrient.SetPos       ( rPoint.X() );
    aHoriOrient.SetHoriOrient( HORI_NONE  );

    aVertOrient.SetPos       ( rPoint.Y() );
    aVertOrient.SetVertOrient( VERT_NONE  );

    aSet.Put( aVertOrient );
    aSet.Put( aHoriOrient );
}

void SwFlyFrmAttrMgr::SetHorzOrientation(SwHoriOrient eOrient)
{
    SwFmtHoriOrient aHoriOrient( GetHoriOrient() );
    aHoriOrient.SetHoriOrient( eOrient );
    aSet.Put( aHoriOrient );
}

void SwFlyFrmAttrMgr::SetVertOrientation(SvxFrameVertOrient eOrient)
{
    SwFmtVertOrient aVertOrient( GetVertOrient() );
    aVertOrient.SetVertOrient( (SwVertOrient)eOrient );
    aSet.Put( aVertOrient );
}

void SwFlyFrmAttrMgr::SetSizeType( SwFrmSize eType )
{
    SwFmtFrmSize aSize( GetFrmSize() );
    aSize.SetSizeType( eType );
    aSet.Put( aSize );
}

void SwFlyFrmAttrMgr::SetSize( const Size& rSize )
{
    SwFmtFrmSize aSize( GetFrmSize() );
    aSize.SetSize(Size(Max(rSize.Width(), long(MINFLY)), Max(rSize.Height(), long(MINFLY))));
    aSet.Put( aSize );
}

void SwFlyFrmAttrMgr::SetFrmSize(const SwFmtFrmSize& rFrmSize)
{
    aSet.Put(rFrmSize);
}

void SwFlyFrmAttrMgr::ProtectPosSize( BOOL bProt )
{
    SvxProtectItem aProtection( ((SvxProtectItem&)aSet.Get(RES_PROTECT)));
    aProtection.SetSizeProtect( bProt );
    aProtection.SetPosProtect ( bProt );
    aSet.Put( aProtection );
}

void SwFlyFrmAttrMgr::SetTransparent(BOOL bTrans)
{
    aSet.Put( SvxOpaqueItem( RES_OPAQUE, !bTrans ));
}


void SwFlyFrmAttrMgr::SetAttrSet(const SfxItemSet& rSet)
{
    aSet.ClearItem();
    aSet.Put( rSet );
}

/*--------------------------------------------------------------------
    Beschreibung: Validierung der Inputs
 --------------------------------------------------------------------*/

SwFrmValid::SwFrmValid() :
    bAuto(0),
    bMirror(0),
    nHPos(0),
    nMaxHPos(LONG_MAX),
    nMinHPos(0),

    nVPos(0),
    nMaxVPos(LONG_MAX),
    nMinVPos(0),

    nWidth( DFLT_WIDTH ),
    nMinWidth(0),
    nMaxWidth(LONG_MAX),

    nHeight( DFLT_HEIGHT ),
    nMaxHeight(LONG_MAX)
{
}

/*------------------------------------------------------------------------

    $Log: not supported by cvs2svn $
    Revision 1.168  2000/09/18 16:05:33  willem.vandorp
    OpenOffice header added.

    Revision 1.167  1998/11/04 18:43:28  MA
    #58858# Format fuer MakeNewFly durchreichen


      Rev 1.166   04 Nov 1998 19:43:28   MA
   #58858# Format fuer MakeNewFly durchreichen

      Rev 1.165   07 Apr 1998 12:00:42   OM
   Maximalwerte fuer Umlauf

      Rev 1.164   01 Apr 1998 15:15:22   OM
   #49023 Prozentuale Controller rechtzeitig initialisieren

      Rev 1.163   05 Feb 1998 15:27:38   OM
   Maximalwertberechnung fuer Rahmenausrichtung

      Rev 1.162   04 Feb 1998 15:53:56   AMA
   Chg: CalcBoundRect beruecksichtigt die neuen Rahmenausrichtungen

      Rev 1.161   24 Nov 1997 17:40:08   MA
   include

      Rev 1.160   20 Nov 1997 12:14:36   AMA
   Opt. SwSurround: GoldCut jetzt als Enum; nicht implementierte Enums entfernt

      Rev 1.159   12 Sep 1997 16:32:36   AMA
   Neu: Wenn am Rahmen verankerte Rahmen ausserhalb sein duerfen ...

      Rev 1.158   12 Sep 1997 10:38:00   OS
   ITEMID_* definiert

      Rev 1.157   15 Aug 1997 12:13:24   OS
   chartar/frmatr/txtatr aufgeteilt

      Rev 1.156   12 Aug 1997 15:58:10   OS
   frmitems/textitem/paraitem aufgeteilt

      Rev 1.155   07 Aug 1997 14:59:12   OM
   Headerfile-Umstellung

      Rev 1.154   30 Jul 1997 18:17:48   HJS
   includes

      Rev 1.153   23 Apr 1997 10:44:34   AMA
   Fix: FLY_AUTO_CNTNT Abstaende richtig berechnen

      Rev 1.152   16 Apr 1997 16:24:14   OS
   neu: SetContour und SetAnchorOnly

      Rev 1.151   16 Apr 1997 11:08:58   NF
   Include-Reihenfolge wegen Internal Compiler Error

      Rev 1.150   15 Apr 1997 16:27:58   AMA
   New: Rahmengebundene Rahmen und auto.positionierte Rahmen

      Rev 1.149   03 Feb 1997 16:00:36   OM
   Maximalwertberechnung fuer zeichengebundene Rahmen geaendert

      Rev 1.148   29 Jan 1997 13:06:24   MA
   unbenutzes entfernt

      Rev 1.147   28 Jan 1997 10:12:18   NF
   includes ...

      Rev 1.146   27 Jan 1997 16:22:30   OS
   HtmlMode wird ueber GetHtmlMode ermittelt

      Rev 1.145   06 Nov 1996 16:26:10   OM
   Maximalwertberechnung verbessert

      Rev 1.144   04 Nov 1996 16:50:44   OM
   Maximalwertbegrenzung fuer Umlauf-TP

      Rev 1.143   04 Nov 1996 14:58:42   OM
   ValidateMetric umgestellt

      Rev 1.142   30 Sep 1996 07:52:36   MA
   new: CalcWidthSpace, CalcHeightSpace

      Rev 1.141   26 Sep 1996 09:03:08   MA
   defautls fuer Rahmen + Aufraeumarbeiten

      Rev 1.140   23 Sep 1996 13:30:54   OS
   richtige max. V-Position und Hoehe fuer absatzgebundene Rahmen

      Rev 1.139   18 Sep 1996 10:39:18   OM
   Umlauf: Nur Anker

      Rev 1.138   12 Sep 1996 17:01:50   OS
   GetAnyCurRect() ersetzt GetCur*Rect

      Rev 1.137   11 Sep 1996 16:51:44   MA
   new: Umlauf nur Anker

      Rev 1.136   10 Sep 1996 17:00:44   OM
   Prozentuale Rahmen

      Rev 1.135   10 Sep 1996 14:21:48   MA
   chg: ein bischen vereinfacht

      Rev 1.134   10 Sep 1996 14:00:46   OM
   Aufgeraeumt

      Rev 1.133   06 Sep 1996 14:15:20   OM
   #31010# GPF bei fehlenden Groessenangaben behoben

      Rev 1.132   28 Aug 1996 11:52:48   OS
   includes

      Rev 1.131   01 Jul 1996 14:55:02   OM
   Neue Segs

      Rev 1.130   01 Jul 1996 14:53:58   OM
   Basic: Rahmengroesse in Prozent setzen

      Rev 1.129   15 May 1996 15:47:18   OS
   SwVertOrient -> SvxFrameVertOrient

      Rev 1.128   25 Apr 1996 15:59:24   MA
   #27183# neg. Position von zeichengeb. beruecksichtigen

      Rev 1.127   19 Apr 1996 17:00:34   MA
   chg: Vorbereitung fuer kleine Rahmen

      Rev 1.126   18 Apr 1996 14:10:32   JP
   unbenutzte Methoden entfernt

      Rev 1.125   22 Mar 1996 15:06:00   OS
   Umstellung 311

      Rev 1.124   21 Mar 1996 14:06:32   OM
   Umstellung 311

      Rev 1.123   20 Feb 1996 17:03:06   JP
   Umbau SfxItemPool -> SwAttrPool

      Rev 1.122   24 Nov 1995 16:58:12   OM
   PCH->PRECOMPILED

      Rev 1.121   17 Nov 1995 18:11:42   OM
   Rahmennamen setzen

      Rev 1.120   03 Nov 1995 20:17:30   MA
   GetUpper..Rect returne keine Objecte mehr

      Rev 1.119   30 Oct 1995 18:44:04   OM
   FrameNotify umgestellt

      Rev 1.118   30 Oct 1995 11:25:34   MA
   chg: Geraffel entfernt

      Rev 1.117   09 Aug 1995 21:50:48   ER
   ! static data _vor_ seg_eofglobals

      Rev 1.116   25 Jul 1995 19:12:06   OS
   +ProtectPos(), +ProtectSize()

      Rev 1.115   24 Jul 1995 19:13:30   MA
   chg: Ctor nur noch mit Shell

      Rev 1.114   17 Jul 1995 16:10:56   OS
   Default-Konstanten jetzt im Header

      Rev 1.113   21 May 1995 15:41:58   MA
   opt: FRMTYE_FLY_ANY.

      Rev 1.112   25 Apr 1995 17:34:06   OM
   Range des Set-Konstruktors um BoxInfoItem erweitert

      Rev 1.111   23 Apr 1995 20:45:26   OS
   Position/Size beim Aufziehen

      Rev 1.110   31 Mar 1995 12:07:38   MA
   fix: Bei PosProtect nicht den Inhalt schuetzen.
   opt: Diverse.
   opt: unoetige includes entfernt.

      Rev 1.109   24 Mar 1995 20:28:32   PK
   neuer member: pownsh

      Rev 1.108   21 Mar 1995 07:38:36   OM
   InsertFlyFrm wieder drin

      Rev 1.107   20 Mar 1995 19:18:30   OS
   unbenutzte Funktionen entfernt

      Rev 1.106   14 Feb 1995 17:26:06   MS
   PrepareBoxInfo

      Rev 1.105   13 Feb 1995 21:11:50   MS
   Rahmen aktiviert

      Rev 1.104   23 Jan 1995 18:14:14   OM
   Auf neue precompiled Header umgestellt

      Rev 1.103   09 Jan 1995 17:08:10   ER
    del: cmdid h

      Rev 1.102   20 Dec 1994 11:54:06   MA
   SS Drawing.

      Rev 1.101   18 Nov 1994 16:17:22   MA
   min -> Min, max -> Max.

------------------------------------------------------------------------*/

