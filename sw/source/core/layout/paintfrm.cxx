/*************************************************************************
 *
 *  $RCSfile: paintfrm.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: jp $ $Date: 2000-10-30 20:32:29 $
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

#define ITEMID_BOXINFO      SID_ATTR_BORDER_INNER
#include <hintids.hxx>

#ifndef _SOUND_HXX //autogen
#include <vcl/sound.hxx>
#endif
#ifndef _POLY_HXX //autogen
#include <vcl/poly.hxx>
#endif
#define _SVSTDARR_LONGS
#include <svtools/svstdarr.hxx>

#ifndef _XOUTBMP_HXX //autogen
#include <svx/xoutbmp.hxx>
#endif
#ifndef _SFX_PROGRESS_HXX //autogen
#include <sfx2/progress.hxx>
#endif
#ifndef _SVX_BRSHITEM_HXX //autogen
#include <svx/brshitem.hxx>
#endif
#ifndef _SVX_OPAQITEM_HXX //autogen
#include <svx/opaqitem.hxx>
#endif
#ifndef _SVX_PRNTITEM_HXX //autogen
#include <svx/prntitem.hxx>
#endif
#ifndef _SVX_BOXITEM_HXX //autogen
#include <svx/boxitem.hxx>
#endif
#ifndef _SVX_SHADITEM_HXX //autogen
#include <svx/shaditem.hxx>
#endif
#ifndef _GRAPH_HXX //autogen
#include <vcl/graph.hxx>
#endif
#ifndef _SVDPAGV_HXX //autogen
#include <svx/svdpagv.hxx>
#endif


#ifndef _FMTSRND_HXX //autogen
#include <fmtsrnd.hxx>
#endif
#ifndef _FMTCLDS_HXX //autogen
#include <fmtclds.hxx>
#endif
#ifndef _SHL_HXX
#include <tools/shl.hxx>
#endif

#ifndef _SWMODULE_HXX
#include <swmodule.hxx>
#endif
#ifndef _ROOTFRM_HXX
#include <rootfrm.hxx>
#endif
#ifndef _PAGEFRM_HXX
#include <pagefrm.hxx>
#endif
#ifndef _CNTFRM_HXX
#include <cntfrm.hxx>
#endif
#ifndef _VIEWSH_HXX
#include <viewsh.hxx>
#endif
#ifndef _SECTION_HXX
#include <section.hxx>
#endif
#ifndef _SECTFRM_HXX
#include <sectfrm.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _VIEWIMP_HXX
#include <viewimp.hxx>
#endif
#ifndef _SWATRSET_HXX
#include <swatrset.hxx>
#endif
#ifndef _DFLYOBJ_HXX
#include <dflyobj.hxx>
#endif
#ifndef _FLYFRM_HXX
#include <flyfrm.hxx>
#endif
#ifndef _FRMTOOL_HXX
#include <frmtool.hxx>
#endif
#ifndef _VIEWOPT_HXX
#include <viewopt.hxx>
#endif
#ifndef _DVIEW_HXX
#include <dview.hxx>
#endif
#ifndef _DCONTACT_HXX
#include <dcontact.hxx>
#endif
#ifndef _TXTFRM_HXX
#include <txtfrm.hxx>
#endif
#ifndef _FTNFRM_HXX
#include <ftnfrm.hxx>
#endif
#ifndef _TABFRM_HXX
#include <tabfrm.hxx>
#endif
#ifndef _NOTXTFRM_HXX
#include <notxtfrm.hxx>
#endif
#ifndef _FRMSH_HXX
#include <frmsh.hxx>
#endif
#ifndef _SWREGION_HXX
#include <swregion.hxx>
#endif
#ifndef _LAYACT_HXX
#include <layact.hxx>
#endif
#ifndef _PAGEDESC_HXX
#include <pagedesc.hxx>
#endif
#ifndef _PTQUEUE_HXX
#include <ptqueue.hxx>
#endif
#ifndef _NOTEURL_HXX
#include <noteurl.hxx>
#endif
#ifndef _VIRTOUTP_HXX
#include <virtoutp.hxx>
#endif
#ifndef _LINEINFO_HXX
#include <lineinfo.hxx>
#endif
#ifndef _DBG_LAY_HXX
#include <dbg_lay.hxx>
#endif

#define GETOBJSHELL()       ((SfxObjectShell*)rSh.GetDoc()->GetDocShell())

//Tabellenhilfslinien an?
#define IS_SUBS_TABLE \
    (pGlobalShell->GetViewOptions()->IsTable() && \
     pGlobalShell->GetViewOptions()->IsSubsTable())
//sonstige Hilfslinien an?
#define IS_SUBS pGlobalShell->GetViewOptions()->IsSubsLines()
//Hilfslinien fuer Bereiche
#define IS_SUBS_SECTION pGlobalShell->GetViewOptions()->IsSectionBounds()

#define SW_MAXBORDERCACHE 20

//Klassendeklarationen. Hier weil sie eben nur in diesem File benoetigt
//werden.

#define SUBCOL_STANDARD 0x01    //normale Hilfslinien
#define SUBCOL_BREAK    0x02    //Hilfslinien bei Umbruechen
#define SUBCOL_GRAY     0x04    //normale Hilfslinien bei LIGHTGRAY-Hintergrund


//----- Klassen zum Sammeln von Umrandungen und Hilfslinien ---
class SwLineRect : public SwRect
{
    const Color    *pColor;
    const SwTabFrm *pTab;
          BYTE      nSubColor;  //Hilfslinien einfaerben
          BOOL      bPainted;   //schon gepaintet?
          BYTE      nLock;      //Um die Linien zum Hell-Layer abzugrenzen.
public:
    SwLineRect( const SwRect &rRect, const Color *pCol,
                const SwTabFrm *pT, const BYTE nSCol = SUBCOL_STANDARD );

    const Color         *GetColor() const { return pColor;}
    const SwTabFrm      *GetTab()   const { return pTab;  }
    void  SetPainted()                    { bPainted = TRUE; }
    void  Lock( BOOL bLock )              { if ( bLock )
                                                ++nLock;
                                            else if ( nLock )
                                                --nLock;
                                          }
    BOOL  IsPainted()               const { return bPainted; }
    BOOL  IsLocked()                const { return nLock != 0;  }
    BYTE  GetSubColor()             const { return nSubColor;}

    BOOL MakeUnion( const SwRect &rRect );
};

SV_DECL_VARARR( SwLRects, SwLineRect, 100, 100 );

class SwLineRects : public SwLRects
{
    USHORT nLastCount;  //unuetze Durchlaeufe im PaintLines verhindern.
public:
    SwLineRects() : nLastCount( 0 ) {}
    void AddLineRect( const SwRect& rRect,  const Color *pColor,
                      const SwTabFrm *pTab, const BYTE nSCol = SUBCOL_STANDARD );
    void ConnectEdges( OutputDevice *pOut );
    void PaintLines  ( OutputDevice *pOut );
    void LockLines( BOOL bLock );

    BYTE Free() const { return nFree; }
};

class SwSubsRects : public SwLineRects
{
    void RemoveSuperfluousSubsidiaryLines( const SwLineRects &rRects ); //;-)
public:
    void PaintSubsidiary( OutputDevice *pOut, const SwLineRects *pRects );

    inline void Ins( const SwRect &rRect, const BYTE nSCol );
};

//----------------- End Klassen Umrandungen ----------------------

static ViewShell *pGlobalShell = 0;

//Fuer PaintBackground, eigentlich lokal, aber fuer SwSavePaintStatics jetzt global
static FASTBOOL bPageOnly = FALSE;

//Wenn durchsichtige FlyInCnts im PaintBackground gepainted werden so soll der
//Hintergrund nicht mehr retouchiert werden.
//static FASTBOOL bLockFlyBackground = FALSE;

//Wenn vom Fly ein Metafile abgezogen wird, so soll nur der FlyInhalt und vor
//nur hintergrund vom FlyInhalt gepaintet werden.
static FASTBOOL bFlyMetafile = FALSE;
static OutputDevice *pFlyMetafileOut = 0;

//Die Retouche fuer Durchsichtige Flys wird vom Hintergrund der Flys
//erledigt. Dabei darf der Fly selbst natuerlich nicht ausgespart werden.
//siehe PaintBackground und lcl_SubtractFlys()
static SwFlyFrm *pRetoucheFly  = 0;
static SwFlyFrm *pRetoucheFly2 = 0;

//Groesse eines Pixel und die Haelfte davon. Wird jeweils bei Eintritt in
//SwRootFrm::Paint neu gesetzt.
static long nPixelSzW = 0, nPixelSzH = 0;
static long nHalfPixelSzW = 0, nHalfPixelSzH = 0;
static long nMinDistPixelW = 0, nMinDistPixelH = 0;
static FASTBOOL  bPixelHeightOdd;
static FASTBOOL  bPixelWidthOdd;

//Aktueller Zoomfaktor
static double aScaleX = 1.0;
static double aScaleY = 1.0;
static double aMinDistScale = 0.73;
static double aEdgeScale = 0.5;

//In pLines werden Umrandungen waehrend des Paint gesammelt und soweit
//moeglich zusammengefasst.
//In pSubsLines werden Hilfslinien gesammelt und zusammengefasst. Diese
//werden vor der Ausgabe mit pLines abgeglichen, so dass moeglichst keine
//Umrandungen von den Hilfslinen verdeckt werden.
//bTablines ist waerend des Paints einer Tabelle TRUE.
static SwLineRects *pLines = 0;
static SwSubsRects *pSubsLines = 0;

static SfxProgress *pProgress = 0;

//Nicht mehr als ein Beep pro Paint, wird auch im Textbereich benutzt!
FASTBOOL bOneBeepOnly = TRUE;

static SwFlyFrm *pFlyOnlyDraw = 0;

//Damit die Flys auch fuer den Hack richtig gepaintet werden koennen.
static FASTBOOL bTableHack = FALSE;

//Um das teure Ermitteln der RetoucheColor zu optimieren
Color aGlobalRetoucheColor;

//Statics fuer Umrandungsalignment setzen.
void SwCalcPixStatics( OutputDevice *pOut )
{
    Size aSz( pOut->PixelToLogic( Size( 1,1 )) );
    nPixelSzW = aSz.Width(), nPixelSzH = aSz.Height();
    nHalfPixelSzW = nPixelSzW / 2 + 1;
    nHalfPixelSzH = nPixelSzH / 2 + 1;
    nMinDistPixelW = nPixelSzW * 2 + 1;
    nMinDistPixelH = nPixelSzH * 2 + 1;
    bPixelHeightOdd = nPixelSzH % 2 ? TRUE : FALSE;
    bPixelWidthOdd  = nPixelSzW % 2 ? TRUE : FALSE;

    const MapMode &rMap = pOut->GetMapMode();
    aScaleX = rMap.GetScaleX();
    aScaleY = rMap.GetScaleY();
}

//Zum Sichern der statics, damit das Paint (quasi) reentrant wird.
class SwSavePaintStatics
{
//  FASTBOOL            bSLockFlyBackground,
    FASTBOOL            bSFlyMetafile,
                        bSPixelHeightOdd,
                        bSPixelWidthOdd,
                        bSOneBeepOnly,
                        bSPageOnly;
    ViewShell          *pSGlobalShell;
    OutputDevice       *pSFlyMetafileOut;
    SwFlyFrm           *pSRetoucheFly,
                       *pSRetoucheFly2,
                       *pSFlyOnlyDraw;
    SwLineRects        *pSLines;
    SwSubsRects        *pSSubsLines;
    SfxProgress        *pSProgress;
    long                nSPixelSzW,
                        nSPixelSzH,
                        nSHalfPixelSzW,
                        nSHalfPixelSzH,
                        nSMinDistPixelW,
                        nSMinDistPixelH;
    Color               aSGlobalRetoucheColor;
    double              aSScaleX,
                        aSScaleY;
public:
    SwSavePaintStatics();
    ~SwSavePaintStatics();
};

SwSavePaintStatics::SwSavePaintStatics() :
//  bSLockFlyBackground ( bLockFlyBackground),
    bSFlyMetafile       ( bFlyMetafile      ),
    bSPixelHeightOdd    ( bPixelHeightOdd   ),
    bSPixelWidthOdd     ( bPixelWidthOdd    ),
    bSOneBeepOnly       ( bOneBeepOnly      ),
    bSPageOnly          ( bPageOnly         ),
    pSGlobalShell       ( pGlobalShell      ),
    pSFlyMetafileOut    ( pFlyMetafileOut   ),
    pSRetoucheFly       ( pRetoucheFly      ),
    pSRetoucheFly2      ( pRetoucheFly2     ),
    pSFlyOnlyDraw       ( pFlyOnlyDraw      ),
    pSLines             ( pLines            ),
    pSSubsLines         ( pSubsLines        ),
    pSProgress          ( pProgress         ),
    nSPixelSzW          ( nPixelSzW         ),
    nSPixelSzH          ( nPixelSzH         ),
    nSHalfPixelSzW      ( nHalfPixelSzW     ),
    nSHalfPixelSzH      ( nHalfPixelSzH     ),
    nSMinDistPixelW     ( nMinDistPixelW    ),
    nSMinDistPixelH     ( nMinDistPixelH    ),
    aSGlobalRetoucheColor( aGlobalRetoucheColor ),
    aSScaleX            ( aScaleX           ),
    aSScaleY            ( aScaleY           )
{
    bPageOnly = /*bLockFlyBackground = */bFlyMetafile = FALSE;
    pFlyMetafileOut = 0;
    pRetoucheFly  = 0;
    pRetoucheFly2 = 0;
    nPixelSzW = nPixelSzH =
    nHalfPixelSzW = nHalfPixelSzH =
    nMinDistPixelW = nMinDistPixelH = 0;
    aScaleX = aScaleY = 1.0;
    aMinDistScale = 0.73;
    aEdgeScale = 0.5;
    pLines = 0;
    pSubsLines = 0;
    pProgress = 0;
    bOneBeepOnly = TRUE;
}

SwSavePaintStatics::~SwSavePaintStatics()
{
//  bLockFlyBackground = bSLockFlyBackground;
    pGlobalShell       = pSGlobalShell;
    bFlyMetafile       = bSFlyMetafile;
    bPixelHeightOdd    = bSPixelHeightOdd;
    bPixelWidthOdd     = bSPixelWidthOdd;
    bOneBeepOnly       = bSOneBeepOnly;
    bPageOnly          = bSPageOnly;
    pFlyMetafileOut    = pSFlyMetafileOut;
    pRetoucheFly       = pSRetoucheFly;
    pRetoucheFly2      = pSRetoucheFly2;
    pFlyOnlyDraw       = pSFlyOnlyDraw;
    pLines             = pSLines;
    pSubsLines         = pSSubsLines;
    pProgress          = pSProgress;
    nPixelSzW          = nSPixelSzW;
    nPixelSzH          = nSPixelSzH;
    nHalfPixelSzW      = nSHalfPixelSzW;
    nHalfPixelSzH      = nSHalfPixelSzH;
    nMinDistPixelW     = nSMinDistPixelW;
    nMinDistPixelH     = nSMinDistPixelH;
    aGlobalRetoucheColor = aSGlobalRetoucheColor;
    aScaleX            = aSScaleX;
    aScaleY            = aSScaleY;
}

//----------------- Implementierungen fuer Tabellenumrandung --------------

SV_IMPL_VARARR( SwLRects, SwLineRect );

SwLineRect::SwLineRect( const SwRect &rRect, const Color *pCol,
                        const SwTabFrm *pT, const BYTE nSCol ) :
    SwRect( rRect ),
    pColor( pCol ),
    pTab( pT ),
    bPainted( FALSE ),
    nSubColor( nSCol ),
    nLock( 0 )
{
}

BOOL SwLineRect::MakeUnion( const SwRect &rRect )
{
    //Es wurde bereits ausserhalb geprueft, ob die Rechtecke die gleiche
    //Ausrichtung (horizontal bzw. vertikal), Farbe usw. besitzen.
    if ( Height() > Width() ) //Vertikale Linie
    {
        if ( Left()  == rRect.Left() && Width() == rRect.Width() )
        {
            //Zusammenfassen wenn kein Luecke zwischen den Linien ist.
            const long nAdd = nPixelSzW + nHalfPixelSzW;
            if ( Bottom() + nAdd >= rRect.Top() &&
                 Top()    - nAdd <= rRect.Bottom()  )
            {
                Bottom( Max( Bottom(), rRect.Bottom() ) );
                Top   ( Min( Top(),    rRect.Top()    ) );
                return TRUE;
            }
        }
    }
    else
    {
        if ( Top()  == rRect.Top() && Height() == rRect.Height() )
        {
            //Zusammenfassen wenn kein Luecke zwischen den Linien ist.
            const long nAdd = nPixelSzW + nHalfPixelSzW;
            if ( Right() + nAdd >= rRect.Left() &&
                 Left()  + nAdd <= rRect.Right() )
            {
                Right( Max( Right(), rRect.Right() ) );
                Left ( Min( Left(),  rRect.Left()  ) );
                return TRUE;
            }
        }
    }
    return FALSE;
}

void SwLineRects::AddLineRect( const SwRect &rRect, const Color *pCol,
                               const SwTabFrm *pTab, const BYTE nSCol )
{
    //Rueckwaerts durch, weil Linien die zusammengefasst werden koennen i.d.R.
    //im gleichen Kontext gepaintet werden.
    for ( USHORT i = Count(); i ; )
    {
        SwLineRect &rLRect = operator[](--i);
        //Pruefen von Ausrichtung, Farbe, Tabelle.
        if ( rLRect.GetTab() == pTab &&
             !rLRect.IsPainted() && rLRect.GetSubColor() == nSCol &&
             (rLRect.Height() > rLRect.Width()) == (rRect.Height() > rRect.Width()) &&
             ((!rLRect.GetColor() && !pCol) ||
              (rLRect.GetColor() && pCol && *rLRect.GetColor() == *pCol)) )
        {
            if ( rLRect.MakeUnion( rRect ) )
                return;
        }
    }
    Insert( SwLineRect( rRect, pCol, pTab, nSCol ), Count() );
}

void SwLineRects::ConnectEdges( OutputDevice *pOut )
{
    if ( pOut->GetOutDevType() != OUTDEV_PRINTER )
    {
        //Fuer einen zu kleinen Zoom arbeite ich nicht.
        if ( aScaleX < aEdgeScale || aScaleY < aEdgeScale )
            return;
    }

    static const long nAdd = 20;

    SvPtrarr   aCheck( 64, 64 );

    for ( int i = 0; i < (int)Count(); ++i )
    {
        SwLineRect &rL1 = operator[](USHORT(i));
        if ( !rL1.GetTab() || rL1.IsPainted() || rL1.IsLocked() )
            continue;

        aCheck.Remove( 0, aCheck.Count() );

        const FASTBOOL bVert = rL1.Height() > rL1.Width();
        long nL1a, nL1b, nL1c, nL1d;

        if ( bVert )
        {   nL1a = rL1.Top();   nL1b = rL1.Left();
            nL1c = rL1.Right(); nL1d = rL1.Bottom();
        }
        else
        {   nL1a = rL1.Left();   nL1b = rL1.Top();
            nL1c = rL1.Bottom(); nL1d = rL1.Right();
        }

        //Alle moeglicherweise mit i1 zu verbindenden Linien einsammeln.
        for ( USHORT i2 = 0; i2 < Count(); ++i2 )
        {
            SwLineRect &rL2 = operator[](i2);
            if ( rL2.GetTab() != rL1.GetTab() ||
                 rL2.IsPainted()              ||
                 rL2.IsLocked()               ||
                 bVert == rL2.Height() > rL2.Width() )
                continue;

            long nL2a, nL2b, nL2c, nL2d;
            if ( bVert )
            {   nL2a = rL2.Top();   nL2b = rL2.Left();
                nL2c = rL2.Right(); nL2d = rL2.Bottom();
            }
            else
            {   nL2a = rL2.Left();   nL2b = rL2.Top();
                nL2c = rL2.Bottom(); nL2d = rL2.Right();
            }

            if ( (nL1a - nAdd < nL2d && nL1d + nAdd > nL2a) &&
                  ((nL1b >  nL2b && nL1c        < nL2c) ||
                   (nL1c >= nL2c && nL1b - nAdd < nL2c) ||
                   (nL1b <= nL2b && nL1c + nAdd > nL2b)) )
            {
                SwLineRect *pMSC = &rL2;
                aCheck.Insert( (void*&)pMSC, aCheck.Count() );
            }
        }
        if ( aCheck.Count() < 2 )
            continue;

        FASTBOOL bRemove = FALSE;

        //Fuer jede Linie jede alle folgenden checken.
        for ( USHORT k = 0; !bRemove && k < aCheck.Count(); ++k )
        {
            SwLineRect &rR1 = (SwLineRect&)*(SwLineRect*)aCheck[k];

            for ( USHORT k2 = k+1; !bRemove && k2 < aCheck.Count(); ++k2 )
            {
                SwLineRect &rR2 = (SwLineRect&)*(SwLineRect*)aCheck[k2];
                if ( bVert )
                {
                    SwLineRect *pLA = 0, *pLB;
                    if ( rR1.Top() < rR2.Top() )
                    {
                        pLA = &rR1; pLB = &rR2;
                    }
                    else if ( rR1.Top() > rR2.Top() )
                    {
                        pLA = &rR2; pLB = &rR1;
                    }
                    //beschreiben k1 und k2 eine Doppellinie?
                    if ( pLA && pLA->Bottom() + 60 > pLB->Top() )
                    {
                        if ( rL1.Top() < pLA->Top() )
                        {
                            if ( rL1.Bottom() == pLA->Bottom() )
                                continue;   //kleiner Irrtum (woher?)

                            SwRect aIns( rL1 );
                            aIns.Bottom( pLA->Bottom() );
                            if ( !rL1.IsInside( aIns ) )
                                continue;
                            const USHORT nFree = Free();
                            Insert( SwLineRect( aIns, rL1.GetColor(),
                                        rL1.GetTab() ), Count() );
                            if ( !nFree )
                            {
                                --i;
                                k = aCheck.Count();
                                break;
                            }
                        }

                        if ( rL1.Bottom() > pLB->Bottom() )
                            rL1.Top( pLB->Top() );  //i1 nach oben verlaengern
                        else
                            bRemove = TRUE;         //abbrechen, i1 entfernen
                    }
                }
                else
                {
                    SwLineRect *pLA = 0, *pLB;
                    if ( rR1.Left() < rR2.Left() )
                    {
                        pLA = &rR1; pLB = &rR2;
                    }
                    else if ( rR1.Left() > rR2.Left() )
                    {
                        pLA = &rR2; pLB = &rR1;
                    }
                    //Liegt eine 'doppellinie' vor?
                    if ( pLA && pLA->Right() + 60 > pLB->Left() )
                    {
                        if ( rL1.Left() < pLA->Left() )
                        {
                            if ( rL1.Right() == pLA->Right() )
                                continue;   //kleiner irrtum

                            SwRect aIns( rL1 );
                            aIns.Right( pLA->Right() );
                            if ( !rL1.IsInside( aIns ) )
                                continue;
                            const USHORT nFree = Free();
                            Insert( SwLineRect( aIns, rL1.GetColor(),
                                        rL1.GetTab() ), Count() );
                            if ( !nFree )
                            {
                                --i;
                                k = aCheck.Count();
                                break;
                            }
                        }
                        if ( rL1.Right() > pLB->Right() )
                            rL1.Left( pLB->Left() );
                        else
                            bRemove = TRUE;
                    }
                }
            }
        }
        if ( bRemove )
        {
            Remove( i, 1 );
            --i;            //keinen auslassen!
        }
    }
}

inline void SwSubsRects::Ins( const SwRect &rRect, const BYTE nSCol )
{
    //Linien die kuerzer als die breiteste Linienbreite sind werden
    //nicht aufgenommen.
    if ( rRect.Height() > DEF_LINE_WIDTH_4 || rRect.Width() > DEF_LINE_WIDTH_4 )
        Insert( SwLineRect( rRect, 0, 0, nSCol ), Count());
}

void SwSubsRects::RemoveSuperfluousSubsidiaryLines( const SwLineRects &rRects )
{
    //Alle Hilfslinien, die sich mit irgendwelchen Umrandungen decken werden
    //entfernt bzw. zerstueckelt..
    for ( USHORT i = 0; i < Count(); ++i )
    {
        SwLineRect &rSubs = operator[](i);

        if ( rSubs.IsPainted() )
            continue;

        const FASTBOOL bVert = rSubs.Height() > rSubs.Width();
        SwRect aSubs( rSubs );
        if ( bVert )
        {
            aSubs.Left  ( aSubs.Left()   - (nPixelSzW+nHalfPixelSzW) );
            aSubs.Right ( aSubs.Right()  + (nPixelSzW+nHalfPixelSzW) );
        }
        else
        {
            aSubs.Top   ( aSubs.Top()    - (nPixelSzH+nHalfPixelSzH) );
            aSubs.Bottom( aSubs.Bottom() + (nPixelSzH+nHalfPixelSzH) );
        }
        for ( USHORT k = 0; k < rRects.Count(); ++k )
        {
            SwLineRect &rLine = rRects[k];

            if ( !bVert == rLine.Height() > rLine.Width() ) //gleiche Ausrichtung?
                continue;

            if ( aSubs.IsOver( rLine ) )
            {
                if ( bVert ) //Vertikal?
                {
                    if ( aSubs.Left()  <= rLine.Right() &&
                         aSubs.Right() >= rLine.Left() )
                    {
                        long nTmp = rLine.Top()-(nPixelSzH+1);
                        if ( rSubs.Top() < nTmp )
                        {
                            SwRect aRect( rSubs );
                            aRect.Bottom( nTmp );
                            Ins( aRect, rSubs.GetSubColor() );
                        }
                        nTmp = rLine.Bottom()+nPixelSzH+1;
                        if ( rSubs.Bottom() > nTmp )
                        {
                            SwRect aRect( rSubs );
                            aRect.Top( nTmp );
                            Ins( aRect, rSubs.GetSubColor() );
                        }
                        Remove( i, 1 );
                        --i;
                        break;
                    }
                }
                else                                    //Horizontal
                {
                    if ( aSubs.Top()    <= rLine.Bottom() &&
                         aSubs.Bottom() >= rLine.Top() )
                    {
                        long nTmp = rLine.Left()-(nPixelSzW+1);
                        if ( rSubs.Left() < nTmp )
                        {
                            SwRect aRect( rSubs );
                            aRect.Right( nTmp );
                            Ins( aRect, rSubs.GetSubColor() );
                        }
                        nTmp = rLine.Right()+nPixelSzW+1;
                        if ( rSubs.Right() > nTmp )
                        {
                            SwRect aRect( rSubs );
                            aRect.Left( nTmp );
                            Ins( aRect, rSubs.GetSubColor() );
                        }
                        Remove( i, 1 );
                        --i;
                        break;
                    }
                }
            }
        }
    }
}

void SwLineRects::LockLines( BOOL bLock )
{
    for ( USHORT i = 0; i < Count(); ++i )
        operator[](i).Lock( bLock );
}

void SwLineRects::PaintLines( OutputDevice *pOut )
{
    //Painten der Umrandungen. Leider muessen wir zweimal durch.
    //Einmal fuer die innenliegenden und einmal fuer die Aussenkanten
    //der Tabellen.
    if ( Count() != nLastCount )
    {
        pOut->Push( PUSH_FILLCOLOR );

        ConnectEdges( pOut );
        const Color *pLast = 0;

        FASTBOOL bPaint2nd = FALSE;
        USHORT nMinCount = Count();
        for ( USHORT i = 0; i < Count(); ++i )
        {
            SwLineRect &rLRect = operator[](i);

            if ( rLRect.IsPainted() )
                continue;

            if ( rLRect.IsLocked() )
            {
                nMinCount = Min( nMinCount, i );
                continue;
            }

            //Jetzt malen oder erst in der zweiten Runde?
            FASTBOOL bPaint = TRUE;
            if ( rLRect.GetTab() )
            {
                if ( rLRect.Height() > rLRect.Width() )
                {
                    //Senkrechte Kante, ueberlappt sie mit der TabellenKante?
                    SwTwips nLLeft  = rLRect.Left()  - 30,
                            nLRight = rLRect.Right() + 30,
                            nTLeft  = rLRect.GetTab()->Frm().Left() + rLRect.GetTab()->Prt().Left(),
                            nTRight = rLRect.GetTab()->Frm().Left() + rLRect.GetTab()->Prt().Right();
                    if ( (nTLeft >= nLLeft && nTLeft <= nLRight) ||
                         (nTRight>= nLLeft && nTRight<= nLRight) )
                        bPaint = FALSE;
                }
                else
                {   //Waagerechte Kante, ueberlappt sie mit der Tabellenkante?
                    SwTwips nLTop    = rLRect.Top()    - 30,
                            nLBottom = rLRect.Bottom() + 30,
                            nTTop    = rLRect.GetTab()->Frm().Top()  + rLRect.GetTab()->Prt().Top(),
                            nTBottom = rLRect.GetTab()->Frm().Top()  + rLRect.GetTab()->Prt().Bottom();
                    if ( (nTTop    >= nLTop && nTTop      <= nLBottom) ||
                         (nTBottom >= nLTop && nTBottom <= nLBottom) )
                        bPaint = FALSE;
                }
            }
            if ( bPaint )
            {
                if ( !pLast || *pLast != *rLRect.GetColor() )
                {
                    pLast = rLRect.GetColor();
                    pOut->SetFillColor( *pLast );
                }
                pOut->DrawRect( rLRect.SVRect() );
                rLRect.SetPainted();
            }
            else
                bPaint2nd = TRUE;
        }
        if ( bPaint2nd )
            for ( i = 0; i < Count(); ++i )
            {
                SwLineRect &rLRect = operator[](i);
                if ( rLRect.IsPainted() )
                    continue;

                if ( rLRect.IsLocked() )
                {
                    nMinCount = Min( nMinCount, i );
                    continue;
                }

                if ( !pLast || *pLast != *rLRect.GetColor() )
                {
                    pLast = rLRect.GetColor();
                    pOut->SetFillColor( *pLast );
                }
                pOut->DrawRect( rLRect.SVRect() );
                rLRect.SetPainted();
            }
        nLastCount = nMinCount;
        pOut->Pop();
    }
}

void SwSubsRects::PaintSubsidiary( OutputDevice *pOut,
                                   const SwLineRects *pRects )
{
    if ( Count() )
    {
        //Alle Hilfslinien, die sich fast decken entfernen (Tabellen)
        for ( USHORT i = 0; i < Count(); ++i )
        {
            SwLineRect &rLi = operator[](i);
            const FASTBOOL bVert = rLi.Height() > rLi.Width();

            for ( USHORT k = i+1; k < Count(); ++k )
            {
                SwLineRect &rLk = operator[](k);
                if ( rLi.SSize() == rLk.SSize() )
                {
                    if ( bVert == rLk.Height() > rLk.Width() )
                    {
                        if ( bVert )
                        {
                            long nLi = rLi.Right();
                            long nLk = rLk.Right();
                            if ( rLi.Top() == rLk.Top() &&
                                 ((nLi < rLk.Left() && nLi+21 > rLk.Left()) ||
                                  (nLk < rLi.Left() && nLk+21 > rLi.Left())))
                            {
                                Remove( k, 1 );
                                //Nicht mit der inneren Schleife weiter, weil
                                //das Array schrumpfen koennte!
                                --i; k = Count();
                            }
                        }
                        else
                        {
                            long nLi = rLi.Bottom();
                            long nLk = rLk.Bottom();
                            if ( rLi.Left() == rLk.Left() &&
                                 ((nLi < rLk.Top() && nLi+21 > rLk.Top()) ||
                                  (nLk < rLi.Top() && nLk+21 > rLi.Top())))
                            {
                                Remove( k, 1 );
                                --i; k = Count();
                            }
                        }
                    }
                }
            }
        }


        if ( pRects && pRects->Count() )
            RemoveSuperfluousSubsidiaryLines( *pRects );

        if ( Count() )
        {
            pOut->Push( PUSH_FILLCOLOR );

            const Color aStandard( COL_LIGHTGRAY );
            const Color aGray( COL_GRAY );
            const Color aBreak(COL_BLUE );

            for ( USHORT i = 0; i < Count(); ++i )
            {
                SwLineRect &rLRect = operator[](i);
                if ( !rLRect.IsPainted() )
                {
                    const Color *pCol;
                    switch ( rLRect.GetSubColor() )
                    {
                        case SUBCOL_STANDARD: pCol = &aStandard; break;
                        case SUBCOL_BREAK:    pCol = &aBreak;    break;
                        case SUBCOL_GRAY:     pCol = &aGray;     break;
                    }
                    if ( pOut->GetFillColor() != *pCol )
                        pOut->SetFillColor( *pCol );
                    pOut->DrawRect( rLRect.SVRect() );
                    rLRect.SetPainted();
                }
            }
            pOut->Pop();
        }
    }
}

//-------------------------------------------------------------------------
//Diverse Functions die in diesem File so verwendet werden.

void MA_FASTCALL SizeBorderRect( SwRect &rRect )
{
    //Das Rechteck um einen Pixel ausdehnen, damit die aligned'en Umrandungen
    //korrekt gemalt werden.
    if ( pGlobalShell->GetOut()->GetOutDevType() != OUTDEV_PRINTER )
    {
        rRect.Pos().Y() = Max( 0L, rRect.Top() - nPixelSzH );
        rRect.Pos().X() = Max( 0L, rRect.Left()- nPixelSzW );
        rRect.SSize().Width() += nPixelSzW * 2;
        rRect.SSize().Height()+= nPixelSzH* 2;
    }
}

void MA_FASTCALL InvertSizeBorderRect( SwRect &rRect, ViewShell *pSh )
{
    if ( pSh->GetOut()->GetOutDevType() != OUTDEV_PRINTER )
    {
        rRect.Pos().Y() = rRect.Top() + nPixelSzH;
        rRect.Pos().X() = rRect.Left()+ nPixelSzW;
        rRect.SSize().Width() -= nPixelSzW * 2;
        rRect.SSize().Height()-= nPixelSzH* 2;
    }
}

void MA_FASTCALL SwAlignRect( SwRect &rRect, ViewShell *pSh )
{
    const OutputDevice *pOut = bFlyMetafile ?
                        pFlyMetafileOut : pSh->GetOut();
    Rectangle aTmp( rRect.SVRect() );
    //Bei ungeraden Pixelwerten korrigieren.
    if ( bPixelHeightOdd )
    {
        aTmp.Top()    += 1;
        aTmp.Bottom() -= 1;
    }
    if ( bPixelWidthOdd )
    {
        aTmp.Left()  += 1;
        aTmp.Right() -= 1;
    }
    aTmp = pOut->PixelToLogic( pOut->LogicToPixel( aTmp ) );

    SwRect aRect( aTmp );

    if ( rRect.Top() > aRect.Top() )
        rRect.Top( aRect.Top() + nHalfPixelSzH );
    else
        rRect.Top( aRect.Top() );

    if ( rRect.Bottom() < aRect.Bottom() )
        rRect.Bottom( aRect.Bottom() - nHalfPixelSzH );
    else
        rRect.Bottom( aRect.Bottom() );

    if ( rRect.Left() > aRect.Left() )
        rRect.Left( aRect.Left() + nHalfPixelSzW );
    else
        rRect.Left( aRect.Left() );

    if ( rRect.Right() < aRect.Right() )
        rRect.Right( aRect.Right() - nHalfPixelSzW );
    else
        rRect.Right( aRect.Right() );
}

long MA_FASTCALL lcl_AlignWidth( const long nWidth )
{
    if ( nWidth )
    {
        const long nW = nWidth % nPixelSzW;

        if ( !nW || nW > nHalfPixelSzW )
            return Max(1L, nWidth - nHalfPixelSzW);
    }
    return nWidth;
}

long MA_FASTCALL lcl_AlignHeight( const long nHeight )
{
    if ( nHeight )
    {
        const long nH = nHeight % nPixelSzH;

        if ( !nH || nH > nHalfPixelSzH )
            return Max(1L, nHeight - nHalfPixelSzH);
    }
    return nHeight;
}

long MA_FASTCALL lcl_MinHeightDist( const long nDist )
{
    if ( aScaleX < aMinDistScale || aScaleY < aMinDistScale )
        return nDist;
    return ::lcl_AlignHeight( Max( nDist, nMinDistPixelH ));
}

long MA_FASTCALL lcl_MinWidthDist( const long nDist )
{
    if ( aScaleX < aMinDistScale || aScaleY < aMinDistScale )
        return nDist;
    return ::lcl_AlignWidth( Max( nDist, nMinDistPixelW ));
}


//Ermittelt PrtArea plus Umrandung plus Schatten.

void MA_FASTCALL lcl_CalcBorderRect( SwRect &rRect, const SwFrm *pFrm,
                                        const SwBorderAttrs &rAttrs,
                                        const BOOL bShadow )
{
    if( pFrm->IsSctFrm() )
        rRect = pFrm->Frm();
    else
    {
        rRect = pFrm->Prt();
        rRect.Pos() += pFrm->Frm().Pos();

        if ( rAttrs.IsLine() || rAttrs.IsBorderDist() ||
             (bShadow && rAttrs.GetShadow().GetLocation() != SVX_SHADOW_NONE) )
        {
            const SvxBoxItem &rBox = rAttrs.GetBox();
            const FASTBOOL bTop = 0 != pFrm->Prt().Top();
            if ( bTop )
            {
                if ( rBox.GetTop() )
                    rRect.Top( rRect.Top() - rBox.CalcLineSpace( BOX_LINE_TOP ) );
                else if ( rAttrs.IsBorderDist() )
                    rRect.Top( rRect.Top() - rBox.GetDistance( BOX_LINE_TOP ) - 1 );
            }

            const FASTBOOL bBottom = pFrm->Prt().Height()+pFrm->Prt().Top() < pFrm->Frm().Height();
            if ( bBottom )
            {
                if ( rBox.GetBottom() )
                    rRect.SSize().Height() += rBox.CalcLineSpace( BOX_LINE_BOTTOM );
                else if ( rAttrs.IsBorderDist() )
                    rRect.SSize().Height() += rBox.GetDistance( BOX_LINE_BOTTOM ) + 1;
            }

            if ( rBox.GetLeft() )
                rRect.Left( rRect.Left() - rBox.CalcLineSpace( BOX_LINE_LEFT ) );
            else if ( rAttrs.IsBorderDist() )
                rRect.Left( rRect.Left() - rBox.GetDistance( BOX_LINE_LEFT ) - 1 );

            if ( rBox.GetRight() )
                rRect.SSize().Width() += rBox.CalcLineSpace( BOX_LINE_RIGHT );
            else if ( rAttrs.IsBorderDist() )
                rRect.SSize().Width() += rBox.GetDistance( BOX_LINE_RIGHT )  + 1;

            if ( bShadow && rAttrs.GetShadow().GetLocation() != SVX_SHADOW_NONE )
            {
                const SvxShadowItem &rShadow = rAttrs.GetShadow();
                if ( bTop )
                    rRect.Top ( rRect.Top() - rShadow.CalcShadowSpace(SHADOW_TOP) );
                rRect.Left( rRect.Left()- rShadow.CalcShadowSpace(SHADOW_LEFT) );
                if ( bBottom )
                    rRect.SSize().Height() += rShadow.CalcShadowSpace(SHADOW_BOTTOM);
                rRect.SSize().Width()  += rShadow.CalcShadowSpace(SHADOW_RIGHT);
            }
        }
    }

    ::SwAlignRect( rRect, pGlobalShell );
}

void MA_FASTCALL lcl_ExtendLeftAndRight( SwRect &rRect, const SwFrm *pFrm,
                                            const SwBorderAttrs &rAttrs )
{
    //In der Hoehe aufbohren wenn TopLine bzw. BottomLine entfallen.
    if ( rAttrs.GetBox().GetTop() && !rAttrs.GetTopLine( pFrm ) )
    {
        const SwFrm *pPre = pFrm->GetPrev();
        rRect.Top( pPre->Frm().Top() + pPre->Prt().Bottom() );
    }
    if ( rAttrs.GetBox().GetBottom() && !rAttrs.GetBottomLine( pFrm ) )
    {
        const SwFrm *pNxt = pFrm->GetNext();
        rRect.Bottom( pNxt->Frm().Top() + pNxt->Prt().Top() );
    }
}


void MA_FASTCALL lcl_SubtractFlys( const SwFrm *pFrm, const SwPageFrm *pPage,
                           const SwRect &rRect, SwRegionRects &rRegion )
{
    const SwSortDrawObjs &rObjs = *pPage->GetSortedObjs();
    const SwFlyFrm *pSelfFly = pFrm->IsInFly() ? pFrm->FindFlyFrm() : pRetoucheFly2;
    if ( !pRetoucheFly )
        pRetoucheFly = pRetoucheFly2;

    for ( USHORT j = 0; (j < rObjs.Count()) && rRegion.Count(); ++j )
    {
        SdrObject *pO = rObjs[j];
        if ( !pO->IsWriterFlyFrame() )
            continue;

        const SwFlyFrm *pFly = ((SwVirtFlyDrawObj*)pO)->GetFlyFrm();

        if ( pSelfFly == pFly || pRetoucheFly == pFly || !rRect.IsOver( pFly->Frm() ) )
            continue;

        if ( !pFly->GetFmt()->GetPrint().GetValue() &&
                (OUTDEV_PRINTER == pGlobalShell->GetOut()->GetOutDevType() ||
                pGlobalShell->IsPreView()))
            continue;

        const FASTBOOL bLowerOfSelf = pSelfFly && pFly->IsLowerOf( pSelfFly ) ?
                                            TRUE : FALSE;

        //Bei zeichengebundenem Fly nur diejenigen betrachten, in denen er
        //nicht selbst verankert ist.
        //#33429# Warum nur bei zeichengebundenen? Es macht doch nie Sinn
        //Rahmen abzuziehen in denen er selbst verankert ist oder?
        if ( pSelfFly && pSelfFly->IsLowerOf( pFly ) )
            continue;

        //#57194# Und warum gilt das nicht analog fuer den RetoucheFly?
        if ( pRetoucheFly && pRetoucheFly->IsLowerOf( pFly ) )
            continue;


#ifndef PRODUCT
        //Flys, die innerhalb des eigenen verankert sind, muessen eine
        //groessere OrdNum haben oder Zeichengebunden sein.
        if ( pSelfFly && bLowerOfSelf )
        {
            ASSERT( pFly->IsFlyInCntFrm() ||
                    pO->GetOrdNumDirect() > pSelfFly->GetVirtDrawObj()->GetOrdNumDirect(),
                    "Fly with wrong z-Order" );
        }
#endif

        BOOL bStopOnHell = TRUE;
        if ( pSelfFly )
        {
            const SdrObject *pTmp = pSelfFly->GetVirtDrawObj();
            if ( pO->GetLayer() == pTmp->GetLayer() )
            {
                if ( pO->GetOrdNumDirect() < pTmp->GetOrdNumDirect() )
                    //Im gleichen Layer werden nur obenliegende beachtet.
                    continue;
            }
            else
            {
                if ( !bLowerOfSelf && !pFly->GetFmt()->GetOpaque().GetValue() )
                    //Aus anderem Layer interessieren uns nur nicht transparente
                    //oder innenliegende
                    continue;
                bStopOnHell = FALSE;
            }
        }
        if ( pRetoucheFly )
        {
            const SdrObject *pTmp = pRetoucheFly->GetVirtDrawObj();
            if ( pO->GetLayer() == pTmp->GetLayer() )
            {
                if ( pO->GetOrdNumDirect() < pTmp->GetOrdNumDirect() )
                    //Im gleichen Layer werden nur obenliegende beachtet.
                    continue;
            }
            else
            {
                if ( !pFly->IsLowerOf( pRetoucheFly ) && !pFly->GetFmt()->GetOpaque().GetValue() )
                    //Aus anderem Layer interessieren uns nur nicht transparente
                    //oder innenliegende
                    continue;
                bStopOnHell = FALSE;
            }
        }

        //Wenn der Inhalt des Fly Transparent ist, wird er nicht abgezogen, es sei denn
        //er steht im Hell-Layer (#31941#)
        BOOL bHell = pO->GetLayer() == pFly->GetFmt()->GetDoc()->GetHellId();
        if ( (bStopOnHell && bHell) ||
             (pFly->Lower() && pFly->Lower()->IsNoTxtFrm() &&
              !bHell &&
              (((SwNoTxtFrm*)pFly->Lower())->IsTransparent() ||
               pFly->GetFmt()->GetSurround().IsContour())))
            continue;


        if ( bHell && pFly->GetAnchor()->IsInFly() )
        {
            //Damit die Umrandung nicht vom Hintergrund des anderen Flys
            //zerlegt wird.
            SwRect aRect;
            SwBorderAttrAccess aAccess( SwFrm::GetCache(), (SwFrm*)pFly );
            const SwBorderAttrs &rAttrs = *aAccess.Get();
            ::lcl_CalcBorderRect( aRect, pFly, rAttrs, TRUE );
            rRegion -= aRect;
        }
        else
        {
            SwRect aRect( pFly->Prt() );
            aRect += pFly->Frm().Pos();
            rRegion -= aRect;
        }
        continue;
    }
    if ( pRetoucheFly == pRetoucheFly2 )
        pRetoucheFly = 0;
}

inline FASTBOOL IsShortCut( const SwRect &rRect, const SwRect &rFrmRect )
{
    //Wenn der Frm vollstaendig rechts neben bzw. unter dem
    //Rect sitzt ist's genug mit Painten.
    return ( (rFrmRect.Top()  > rRect.Bottom()) ||
             (rFrmRect.Left() > rRect.Right()) );
}


//---------------- Ausgabe fuer das BrushItem ----------------

void lcl_DrawGraphic( const SvxBrushItem& rBrush, OutputDevice *pOut,
                      ViewShell &rSh, const SwRect &rGrf, const SwRect &rOut,
                      BOOL bClip, BOOL bGrfNum )
{
    const FASTBOOL bNotInside = bClip && !rOut.IsInside( rGrf );
    if ( bNotInside )
    {
        pOut->Push( PUSH_CLIPREGION );
        pOut->IntersectClipRegion( rOut.SVRect() );
    }

    //Hier kein Link, wir wollen die Grafik synchron laden!
    ((SvxBrushItem&)rBrush).SetDoneLink( Link() );
    GraphicObject *pGrf = (GraphicObject*)rBrush.GetGraphicObject(
                                                    GETOBJSHELL() );

    if( !bGrfNum &&
         ( pGrf->IsTransparent() || GRAPHIC_NONE == pGrf->GetType() ) )
    {
        const Color aColor( !rBrush.GetColor().GetTransparency() ||
                                bFlyMetafile
                            ? rBrush.GetColor()
                            : aGlobalRetoucheColor );

        if ( pOut->GetFillColor() != aColor )
            pOut->SetFillColor( aColor );
        pOut->DrawRect( rGrf.SVRect() );
    }
    pGrf->Draw( pOut, rGrf.Pos(), rGrf.SSize() );

    if ( bNotInside )
        pOut->Pop();
}

void MA_FASTCALL DrawGraphic( const SvxBrushItem *pBrush, OutputDevice *pOut,
    const SwRect &rOrg, const SwRect &rOut, const BYTE nGrfNum )
{
    ViewShell &rSh = *pGlobalShell;
    BOOL bReplaceGrfNum = GRFNUM_REPLACE == nGrfNum;
    BOOL bGrfNum = GRFNUM_NO != nGrfNum;
    Size aGrfSize;
    SvxGraphicPosition ePos = GPOS_NONE;
    if( pBrush && !bReplaceGrfNum )
    {
        if( rSh.GetViewOptions()->IsGraphic() )
        {
            ((SvxBrushItem*)pBrush)->SetDoneLink( STATIC_LINK(
                                    rSh.GetDoc(), SwDoc, BackgroundDone ) );
            SfxObjectShell &rObjSh = *GETOBJSHELL();
            const Graphic* pGrf = pBrush->GetGraphic( &rObjSh );
            if( pGrf && GRAPHIC_NONE != pGrf->GetType() )
            {
                ePos = pBrush->GetGraphicPos();
                if( pGrf->IsSupportedGraphic() )
                    aGrfSize = ::GetGraphicSizeTwip( *pGrf, pOut );
            }
        }
        else
            bReplaceGrfNum = bGrfNum;
    }

    SwRect aGrf;
    aGrf.SSize( aGrfSize );
    FASTBOOL bDraw = TRUE;
    FASTBOOL bRetouche = TRUE;
    switch ( ePos )
    {
    case GPOS_LT:
        aGrf.Pos() = rOrg.Pos();
        break;

    case GPOS_MT:
        aGrf.Pos().Y() = rOrg.Top();
        aGrf.Pos().X() = rOrg.Left() + rOrg.Width()/2 - aGrfSize.Width()/2;
        break;

    case GPOS_RT:
        aGrf.Pos().Y() = rOrg.Top();
        aGrf.Pos().X() = rOrg.Right() - aGrfSize.Width();
        break;

    case GPOS_LM:
        aGrf.Pos().Y() = rOrg.Top() + rOrg.Height()/2 - aGrfSize.Height()/2;
        aGrf.Pos().X() = rOrg.Left();
        break;

    case GPOS_MM:
        aGrf.Pos().Y() = rOrg.Top() + rOrg.Height()/2 - aGrfSize.Height()/2;
        aGrf.Pos().X() = rOrg.Left() + rOrg.Width()/2 - aGrfSize.Width()/2;
        break;

    case GPOS_RM:
        aGrf.Pos().Y() = rOrg.Top() + rOrg.Height()/2 - aGrfSize.Height()/2;
        aGrf.Pos().X() = rOrg.Right() - aGrfSize.Width();
        break;

    case GPOS_LB:
        aGrf.Pos().Y() = rOrg.Bottom() - aGrfSize.Height();
        aGrf.Pos().X() = rOrg.Left();
        break;

    case GPOS_MB:
        aGrf.Pos().Y() = rOrg.Bottom() - aGrfSize.Height();
        aGrf.Pos().X() = rOrg.Left() + rOrg.Width()/2 - aGrfSize.Width()/2;
        break;

    case GPOS_RB:
        aGrf.Pos().Y() = rOrg.Bottom() - aGrfSize.Height();
        aGrf.Pos().X() = rOrg.Right() - aGrfSize.Width();
        break;

    case GPOS_AREA:
        aGrf = rOrg;
        bRetouche = FALSE;
        break;

    case GPOS_TILED:
        {

// !!!!!!!!!!!
// Optimization from KA - new method on the graphic object, like the old
//                      method
//                          XOutBitmap::DrawTiledBitmapEx( &aVout,
//                                  aPoint, rSize, aOut, *pQuickDrawBmp );
// !!!!!!!!!!!
            aGrf.Pos() = rOrg.Pos();
            pOut->Push( PUSH_CLIPREGION );
            pOut->IntersectClipRegion( rOut.SVRect() );
            do {
                do{
                    if( aGrf.IsOver( rOut ) )
                        lcl_DrawGraphic( *pBrush, pOut, rSh, aGrf,
                                                rOut, FALSE, bGrfNum );
                    aGrf.Pos().X() += aGrf.Width();

                } while( aGrf.Left() < rOut.Right() );

                aGrf.Pos().X() = rOrg.Left();
                aGrf.Pos().Y() += aGrf.Height();

            }  while( aGrf.Top() < rOut.Bottom() ) ;
            pOut->Pop();

            bDraw = bRetouche = FALSE;
        }
        break;

    case GPOS_NONE:
        bDraw = FALSE;
        break;

    default: ASSERT( !pOut, "new Graphic position?" );
    }

    if ( bRetouche )
    {
        SwRegionRects aRegion( rOut, 4 );
        aRegion -= aGrf;
        pOut->Push( PUSH_FILLCOLOR );

        const Color aColor( pBrush && ( (!pBrush->GetColor().
                                            GetTransparency()) || bFlyMetafile )
                    ? pBrush->GetColor()
                    : aGlobalRetoucheColor );

        if( pOut->GetFillColor() != aColor )
            pOut->SetFillColor( aColor );
        for( USHORT i = 0; i < aRegion.Count(); ++i )
            pOut->DrawRect( aRegion[i].SVRect() );
        pOut->Pop();
    }
    if( bDraw && aGrf.IsOver( rOut ) )
        lcl_DrawGraphic( *pBrush, pOut, rSh, aGrf, rOut, TRUE, bGrfNum );

    if( bReplaceGrfNum )
    {
        const Bitmap& rBmp = SwNoTxtFrm::GetBitmap( FALSE );
        Font aTmp( pOut->GetFont() );
        ((Graphic*)0)->Draw( pOut, aEmptyStr, aTmp, rBmp,
                             rOrg.Pos(), rOrg.SSize() );
    }
}

//------------------------------------------------------------------------

/*************************************************************************
|*
|*  SwRootFrm::Paint()
|*
|*  Beschreibung
|*      Fuer jede sichtbare Seite, die von Rect berhrt wird einmal Painten.
|*      1. Umrandungen und Hintergruende Painten.
|*      2. Den DrawLayer (Ramen und Zeichenobjekte) der unter dem Dokument
|*         liegt painten (Hoelle).
|*      3. Den Dokumentinhalt (Text) Painten.
|*      4. Den Drawlayer der ueber dem Dokuemnt liegt painten.
|*
|*  Ersterstellung      MA 01. Jun. 92
|*  Letzte Aenderung    MA 10. Oct. 97
|*
|*************************************************************************/

void SwRootFrm::Paint( const SwRect& rRect ) const
{
    ASSERT( Lower() && Lower()->IsPageFrm(), "Lower der Root keine Seite." );

    PROTOCOL( this, PROT_FILE_INIT, 0, 0)

    FASTBOOL bResetRootPaint = FALSE;
    ViewShell *pSh = pCurrShell;

    if ( pSh->GetWin() )
    {
        if ( pSh->GetOut() == pSh->GetWin() && !pSh->GetWin()->IsVisible() )
        {
            return;
        }
        if ( SwRootFrm::bInPaint )
        {
            SwPaintQueue::Add( pSh, rRect );
            return;
        }
    }
    else
        SwRootFrm::bInPaint = bResetRootPaint = TRUE;

    SwSavePaintStatics *pStatics = 0;
    if ( pGlobalShell )
        pStatics = new SwSavePaintStatics();
    pGlobalShell = pSh;

    bOneBeepOnly = pSh->GetWin() != 0 && pSh->GetDoc()->IsFrmBeepEnabled();
    if( !pSh->GetWin() )
        pProgress = SfxProgress::GetActiveProgress( (SfxObjectShell*) pSh->GetDoc()->GetDocShell() );

    ::SwCalcPixStatics( pSh->GetOut() );
    aGlobalRetoucheColor = pSh->Imp()->GetRetoucheColor();

    //Ggf. eine Action ausloesen um klare Verhaeltnisse zu schaffen.
    //Durch diesen Kunstgriff kann in allen Paints davon ausgegangen werden,
    //das alle Werte gueltigt sind - keine Probleme, keine Sonderbehandlung(en).
    if ( !pSh->IsInEndAction() && !pSh->IsPaintInProgress() &&
         (!pSh->Imp()->IsAction() || !pSh->Imp()->GetLayAction().IsActionInProgress() ) )
    {
        ((SwRootFrm*)this)->ResetTurbo();
        SwLayAction aAction( (SwRootFrm*)this, pSh->Imp() );
        aAction.SetPaint( FALSE );
        aAction.SetComplete( FALSE );
        aAction.SetReschedule( pProgress ? TRUE : FALSE );
        aAction.Action();
        ((SwRootFrm*)this)->ResetTurboFlag();
        if ( !pSh->ActionPend() )
            pSh->Imp()->DelRegions();
    }

    SwRect aRect( rRect );
    aRect.Intersection( pSh->VisArea() );

    FASTBOOL bExtraData = ::IsExtraData( GetFmt()->GetDoc() );

    pLines = new SwLineRects;   //Sammler fuer Umrandungen.

    const SwPageFrm *pPage = pSh->Imp()->GetFirstVisPage();

    while ( pPage && !::IsShortCut( aRect, pPage->Frm() ) )
    {
        if ( !pPage->IsEmptyPage() && aRect.IsOver( pPage->Frm() ) )
        {
            if ( pSh->GetWin() )
                pSubsLines = new SwSubsRects;

            SwRect aPaintRect( pPage->Frm() );
            aPaintRect._Intersection( aRect );

            if ( bExtraData )
            {
                //Ja, das ist grob, aber wie macht man es besser?
                aPaintRect.Left( pPage->Frm().Left() );
                aPaintRect.Right( pPage->Frm().Right() );
                aPaintRect._Intersection( pSh->VisArea() );
            }

            pVout->Enter( pSh, aPaintRect, !bNoVirDev );

               SwRect aBorderRect( aPaintRect );
               ::SizeBorderRect( aBorderRect );

            aPaintRect.Top(  Max( 0L, aPaintRect.Top() - nPixelSzH ));
            aPaintRect.Left( Max( 0L, aPaintRect.Left()- nPixelSzW ));
            pVout->SetOrgRect( aPaintRect );

            pPage->PaintBaBo( aPaintRect, pPage, TRUE );

            if ( pSh->Imp()->HasDrawView() )
            {
                pLines->LockLines( TRUE );
                pSh->Imp()->PaintLayer( pSh->GetDoc()->GetHellId(), aBorderRect );
                pLines->PaintLines( pSh->GetOut() );
                pLines->LockLines( FALSE );
            }

            pPage->Paint( aPaintRect );
            pLines->PaintLines( pSh->GetOut() );

            BOOL bControlExtra = FALSE;
            if ( pSh->Imp()->HasDrawView() )
            {
                pSh->Imp()->PaintLayer( pSh->GetDoc()->GetHeavenId(), aBorderRect );
                if( pVout->IsFlushable() )
                    bControlExtra = TRUE;
                else
                    pSh->Imp()->PaintLayer( pSh->GetDoc()->GetControlsId(), aBorderRect );
                pLines->PaintLines( pSh->GetOut() );
            }

            if ( bExtraData )
                pPage->RefreshExtraData( aBorderRect );

            if ( pSh->GetWin() )
            {
                pPage->RefreshSubsidiary( aBorderRect );
                pSubsLines->PaintSubsidiary( pSh->GetOut(), pLines );
                DELETEZ( pSubsLines );
            }
            pVout->Leave();
            if( bControlExtra )
                pSh->Imp()->PaintLayer( pSh->GetDoc()->GetControlsId(), aBorderRect );
        }
        ASSERT( !pPage->GetNext() || pPage->GetNext()->IsPageFrm(),
                "Nachbar von Seite keine Seite." );
        pPage = (SwPageFrm*)pPage->GetNext();
    }

    DELETEZ( pLines );

    if ( pSh->GetWin() && pSh->Imp()->HasDrawView() &&
         pSh->Imp()->GetDrawView()->IsGridVisible() )
        pSh->Imp()->GetDrawView()->GetPageViewPgNum(0)->DrawGrid(
                                            *pSh->GetOut(), rRect.SVRect() );

    if ( bResetRootPaint )
        SwRootFrm::bInPaint = FALSE;
    if ( pStatics )
        delete pStatics;
    else
    {
        pProgress = 0;
        pGlobalShell = 0;
    }

    if ( ViewShell::IsLstEndAction() && pSh->GetWin() && pSh->Imp()->HasDrawView() )
        pSh->Imp()->GetDrawView()->PostPaint();
}

/*************************************************************************
|*
|*  SwRootFrm::HackPrepareLongTblPaint()
|*
|*  Ersterstellung      MA 27. Sep. 96
|*  Letzte Aenderung    MA 18. Nov. 97
|*
|*************************************************************************/

void SwRootFrm::HackPrepareLongTblPaint( int nMode )
{
    switch ( nMode )
    {
        case HACK_TABLEMODE_INIT       : ASSERT( !pLines, "HackPrepare: already prepared" );
                                         pLines = new SwLineRects;
                                         ASSERT( !pGlobalShell, "old GlobalShell lost" );
                                         pGlobalShell = GetShell();
                                         bTableHack = TRUE;
                                         break;
        case HACK_TABLEMODE_LOCKLINES  : pLines->LockLines( TRUE ); break;
        case HACK_TABLEMODE_PAINTLINES : pLines->PaintLines( GetShell()->GetOut() );
                                         break;
        case HACK_TABLEMODE_UNLOCKLINES: pLines->LockLines( FALSE ); break;
        case HACK_TABLEMODE_EXIT       : pLines->PaintLines( GetShell()->GetOut() );
                                         DELETEZ( pLines );
                                         pGlobalShell = 0;
                                         bTableHack = FALSE;
                                         break;
    }
}


/*************************************************************************
|*
|*  SwLayoutFrm::Paint()
|*
|*  Ersterstellung      MA 19. May. 92
|*  Letzte Aenderung    MA 19. Apr. 95
|*
|*************************************************************************/

void MA_FASTCALL lcl_EmergencyFormatFtnCont( SwFtnContFrm *pCont )
{
    //Es kann sein, dass der Cont vernichtet wird.
    SwCntntFrm *pCnt = pCont->ContainsCntnt();
    while ( pCnt && pCnt->IsInFtn() )
    {
        pCnt->Calc();
        pCnt = pCnt->GetNextCntntFrm();
    }
}


void SwLayoutFrm::Paint( const SwRect& rRect ) const
{
    const SwFrm *pFrm = Lower();
    if ( !pFrm )
        return;

    BOOL bCnt;
    if ( TRUE == (bCnt = pFrm->IsCntntFrm()) )
        pFrm->Calc();

    if ( pFrm->IsFtnContFrm() )
    {   ::lcl_EmergencyFormatFtnCont( (SwFtnContFrm*)pFrm );
        pFrm = Lower();
    }

    const SwPageFrm *pPage = 0;
    const FASTBOOL bWin   = pGlobalShell->GetWin() ? TRUE : FALSE;

    while ( IsAnLower( pFrm ) )
    {
        SwRect aPaintRect( pFrm->PaintArea() );
        if( ::IsShortCut( rRect, aPaintRect ) )
            break;
        if ( bCnt && pProgress )
            pProgress->Reschedule();

        //Wenn ein Frm es explizit will muss retouchiert werden.
        //Erst die Retouche, denn selbige koennte die aligned'en Raender
        //plaetten.
        if ( pFrm->IsRetouche() )
        {
            if ( pFrm->IsRetoucheFrm() && bWin && !pFrm->GetNext() )
            {   if ( !pPage )
                    pPage = FindPageFrm();
                pFrm->Retouche( pPage, rRect );
            }
            pFrm->ResetRetouche();
        }
        if ( rRect.IsOver( aPaintRect ) )
        {
            if ( bCnt && pFrm->IsCompletePaint() &&
                 !rRect.IsInside( aPaintRect ) && GetpApp()->AnyInput( INPUT_KEYBOARD ) )
            {
                //fix(8104): Es kann vorkommen, dass die Verarbeitung nicht
                //vollstaendig war, aber trotzdem Teile des Absatzes gepaintet
                //werden. In der Folge werden dann evtl. wiederum andere Teile
                //des Absatzes garnicht mehr gepaintet. Einziger Ausweg scheint
                //hier ein Invalidieren der Windows zu sein.
                //Um es nicht alzu Heftig werden zu lassen versuche ich hier
                //das Rechteck zu begrenzen indem der gewuenschte Teil gepaintet
                //und nur die uebrigen Absatzanteile invalidiert werden.
                if ( aPaintRect.Left()  == rRect.Left() &&
                     aPaintRect.Right() == rRect.Right() )
                {
                    aPaintRect.Bottom( rRect.Top() - 1 );
                    if ( aPaintRect.Height() > 0 )
                        pGlobalShell->InvalidateWindows(aPaintRect);
                    aPaintRect.Top( rRect.Bottom() + 1 );
                    aPaintRect.Bottom( pFrm->Frm().Bottom() );
                    if ( aPaintRect.Height() > 0 )
                        pGlobalShell->InvalidateWindows(aPaintRect);
                    aPaintRect.Top( pFrm->Frm().Top() );
                    aPaintRect.Bottom( pFrm->Frm().Bottom() );
                }
                else
                {
                    pGlobalShell->InvalidateWindows( aPaintRect );
                    pFrm = pFrm->GetNext();
                    if ( pFrm && (TRUE == (bCnt = pFrm->IsCntntFrm())) )
                        pFrm->Calc();
                    continue;
                }
            }
            pFrm->ResetCompletePaint();
            aPaintRect._Intersection( rRect );
            pFrm->Paint( aPaintRect );

            if ( Lower() && Lower()->IsColumnFrm() )
            {
                //Ggf. die Spaltentrennlinien malen. Fuer den Seitenbody ist
                //nicht der Upper sondern die Seite Zustaendig.
                const SwFrmFmt *pFmt = GetUpper() && GetUpper()->IsPageFrm()
                                            ? GetUpper()->GetFmt()
                                            : GetFmt();
                const SwFmtCol &rCol = pFmt->GetCol();
                if ( rCol.GetLineAdj() != COLADJ_NONE )
                {
                    if ( !pPage )
                        pPage = pFrm->FindPageFrm();
                    PaintColLines( aPaintRect, rCol, pPage );
                }
            }

            //Nur: Body, Column, Tab, Header, Footer, FtnCont, Ftn
            if ( bOneBeepOnly && (pFrm->GetType() & 0x08FC))
            {
                //Wenn der Frm ueber seinen Upper hinausragt gibts die gelbe Karte.
                const long nDeadline = Frm().Top() + Prt().Top() +
                                       Prt().Height() - 1;
                if ( pFrm->Frm().Bottom() > nDeadline )
                {
                    if ( pGlobalShell->VisArea().Bottom() > nDeadline &&
                         pGlobalShell->VisArea().Top() < nDeadline )
                    {
                        bOneBeepOnly = FALSE;
                        Sound::Beep();
                    }
                }
            }
        }
        if ( !bCnt && pFrm->GetNext() && pFrm->GetNext()->IsFtnContFrm() )
            ::lcl_EmergencyFormatFtnCont( (SwFtnContFrm*)pFrm->GetNext() );

        pFrm = pFrm->GetNext();
        if ( pFrm && (TRUE == (bCnt = pFrm->IsCntntFrm())) )
            pFrm->Calc();
    }
}

/*************************************************************************
|*
|*  SwFlyFrm::IsPaint()
|*
|*  Ersterstellung      MA 16. Jan. 97
|*  Letzte Aenderung    MA 16. Jan. 97
|*
|*************************************************************************/

BOOL SwFlyFrm::IsPaint( SdrObject *pObj, const ViewShell *pSh )
{
    SdrObjUserCall *pUserCall;

    if ( 0 == ( pUserCall = GetUserCall(pObj) ) )
        return TRUE;

    //Attributabhaengig nicht fuer Drucker oder PreView painten
    FASTBOOL bPaint =  pFlyOnlyDraw ||
                       ((SwContact*)pUserCall)->GetFmt()->GetPrint().GetValue();
    if ( !bPaint )
        bPaint = pSh->GetWin() && !pSh->IsPreView();

    if ( bPaint )
    {
        //Das Paint kann evtl. von von uebergeordneten Flys verhindert werden.
        SwFrm *pAnch = 0;
        if ( pObj->IsWriterFlyFrame() )
        {
            SwFlyFrm *pFly = ((SwVirtFlyDrawObj*)pObj)->GetFlyFrm();
            if ( pFlyOnlyDraw && pFlyOnlyDraw == pFly )
                return TRUE;

            //Die Anzeige eines Zwischenstadiums vermeiden, Flys die nicht mit
            //der Seite auf der sie verankert sind ueberlappen werden auch
            //nicht gepaintet.
            //HACK: Ausnahme: Drucken von Rahmen in Tabellen, diese koennen
            //bei uebergrossen Tabellen (HTML) schon mal auserhalb der Seite
            //stehen.
            SwPageFrm *pPage = pFly->FindPageFrm();
            if ( pPage )
            {
                if ( pPage->Frm().IsOver( pFly->Frm() ) )
                    pAnch = pFly->GetAnchor();
                else if ( bTableHack &&
                          pFly->Frm().Top() >= pFly->GetAnchor()->Frm().Top() &&
                            pFly->Frm().Top() < pFly->GetAnchor()->Frm().Bottom() &&
                          long(pSh->GetOut()) == long(pSh->GetPrt()) )
                {
                    pAnch = pFly->GetAnchor();
                }
            }

        }
        else
        {
            pAnch = ((SwDrawContact*)pUserCall)->GetAnchor();
            if ( pAnch )
            {
                if ( !pAnch->GetValidPosFlag() )
                    pAnch = 0;
                else if ( long(pSh->GetOut()) == long(pSh->GetPrt()) )
                {
                    //HACK: fuer das Drucken muessen wir ein paar Objekte
                    //weglassen, da diese sonst doppelt gedruckt werden.
                    //Die Objekte sollen gedruckt werden, wenn der TableHack
                    //gerade greift. In der Folge duerfen sie nicht gedruckt werden
                    //wenn sie mit der Seite dran sind, ueber der sie von der
                    //Position her gerade schweben.
                    SwPageFrm *pPage = pAnch->FindPageFrm();
                    if ( !bTableHack &&
                         !pPage->Frm().IsOver( pObj->GetBoundRect() ) )
                        pAnch = 0;
                }
            }
        }
        if ( pAnch )
        {
            if ( pAnch->IsInFly() )
                bPaint = SwFlyFrm::IsPaint( pAnch->FindFlyFrm()->GetVirtDrawObj(),
                                            pSh );
            else if ( pFlyOnlyDraw )
                bPaint = FALSE;
        }
        else
            bPaint = FALSE;
    }
    return bPaint;
}

/*************************************************************************
|*
|*  SwFlyFrm::Paint()
|*
|*  Ersterstellung      MA ??
|*  Letzte Aenderung    MA 16. Jan. 97
|*
|*************************************************************************/

//Weiter unten definiert
void MA_FASTCALL lcl_PaintLowerBorders( const SwLayoutFrm *pLay,
                               const SwRect &rRect, const SwPageFrm *pPage );

void SwFlyFrm::Paint( const SwRect& rRect ) const
{
    //wegen der Ueberlappung von Rahmen und Zeichenobjekten muessen die
    //Flys ihre Umrandung (und die der Innenliegenden) direkt ausgeben.
    //z.B. #33066#
    pLines->LockLines(TRUE);

    SwRect aRect( rRect );
    if ( !IsFlyInCntFrm() )
        ::InvertSizeBorderRect( aRect, pGlobalShell );
    aRect._Intersection( Frm() );

    OutputDevice *pOut = pGlobalShell->GetOut();
    pOut->Push( PUSH_CLIPREGION );
    pOut->SetClipRegion();
    const SwPageFrm *pPage = FindPageFrm();

    const SwNoTxtFrm *pNoTxt = Lower() && Lower()->IsNoTxtFrm()
                                                ? (SwNoTxtFrm*)Lower() : 0;
    FASTBOOL bTransparent = pNoTxt ? pNoTxt->IsTransparent() : FALSE,
             bContour     = GetFmt()->GetSurround().IsContour(),
             bHell, bPaintBack;

    if ( bTransparent &&
         GetVirtDrawObj()->GetLayer() == GetFmt()->GetDoc()->GetHellId() &&
         GetAnchor()->FindFlyFrm() )
    {
        SwFlyFrm *pOldRet = pRetoucheFly2; pRetoucheFly2 = (SwFlyFrm*)this;
        const SwFrm *pFrm = GetAnchor()->FindFlyFrm();
        SwBorderAttrAccess aAccess( SwFrm::GetCache(), pFrm );
        const SwBorderAttrs &rAttrs = *aAccess.Get();
        pFrm->PaintBackground( aRect, pPage, rAttrs, FALSE, FALSE );
        pRetoucheFly2 = pOldRet;
    }
//#33429#           else
    {
        PolyPolygon aPoly;
        if ( bContour )
            bContour = GetContour( aPoly );

        //Hintergrund painten fuer:
        bPaintBack = !pNoTxt || Prt().SSize() != Frm().SSize();
        //sowie fuer Transparente und Contour in der Hoelle
        bPaintBack = bPaintBack ||
                ((bTransparent || bContour ) &&
                TRUE == (bHell = GetVirtDrawObj()->GetLayer() == GetFmt()->GetDoc()->GetHellId()));
        //sowie fuer Transparente und Contour mit eigener Brush
        if ( !bPaintBack && (bTransparent||bContour) )
        {
            const SvxBrushItem &rBack = GetFmt()->GetBackground();
            bPaintBack = !rBack.GetColor().GetTransparency() ||
                             rBack.GetGraphicPos() != GPOS_NONE;
        }

        if ( bPaintBack )
        {
            //#24926# JP 01.02.96, PaintBaBo in teilen hier, damit PaintBorder
            //das orig. Rect bekommt, aber PaintBackground das begrenzte.

            pOut->Push( PUSH_FILLCOLOR );

            pPage = FindPageFrm();

            SwBorderAttrAccess aAccess( SwFrm::GetCache(), (SwFrm*)this );
            const SwBorderAttrs &rAttrs = *aAccess.Get();

/*          //Das Paint der FlyInCnt's wird vom Paint der Lines gerufen. Damit
            //der Rand (UL-/LRSpace) mit der richtigen Farbe retouchiert wird,
            //muss Das Paint an den Anker weitergereicht werden.
            FASTBOOL bUnlock = FALSE;
            if ( IsFlyInCntFrm() )
            {
                bLockFlyBackground = bUnlock = TRUE;
                SwBorderAttrAccess aAccess( SwFrm::GetCache(), GetAnchor());
                const SwBorderAttrs &rAttrs = *aAccess.Get();
                GetAnchor()->PaintBackground( aRect, pPage, rAttrs, FALSE );
            }
*/
            SwRegionRects aRegion( aRect );
            if ( pNoTxt && !bTransparent )
            {
                //Was wir eigentlich Painten wollen ist der schmale Streifen
                //zwischen PrtArea und aeusserer Umrandung.
                 SwRect aTmp( Prt() ); aTmp += Frm().Pos();
                aRegion -= aTmp;
            }
            if ( bContour )
            {
                pOut->Push();
                if ( !pOut->GetConnectMetaFile() || pOut->GetOutDevType() == OUTDEV_PRINTER )
                {
                    pOut->SetClipRegion( aPoly );
                }
                for ( USHORT i = 0; i < aRegion.Count(); ++i )
                    PaintBackground( aRegion[i], pPage, rAttrs, FALSE, TRUE );
                pOut->Pop();
            }
            else
                for ( USHORT i = 0; i < aRegion.Count(); ++i )
                    PaintBackground( aRegion[i], pPage, rAttrs, FALSE, TRUE );

            SwRect aTmp( rRect );
            if ( IsFlyInCntFrm() )
                ::SizeBorderRect( aTmp );
//??        aTmp._Intersection( Frm() );
            PaintBorder( aTmp, pPage, rAttrs );
/*          if ( bUnlock )
                bLockFlyBackground = FALSE;
*/
            pOut->Pop();
        }
    }

    SwLayoutFrm::Paint( aRect );
    Validate();

    pLines->LockLines( FALSE );
    pLines->PaintLines( pOut );

    pOut->Pop();

    if ( pProgress && pNoTxt )
        pProgress->Reschedule();
}
/*************************************************************************
|*
|*    SwTabFrm::Paint()
|*
|*    Ersterstellung    MA 11. May. 93
|*    Letzte Aenderung  MA 23. Mar. 95
|*
|*************************************************************************/

void SwTabFrm::Paint( const SwRect& rRect ) const
{
    if( pGlobalShell->GetViewOptions()->IsTable() )
        SwLayoutFrm::Paint( rRect );
    else if( pGlobalShell->GetWin() )
    {
        pGlobalShell->GetViewOptions()->
                DrawRect( pGlobalShell->GetOut(), rRect, COL_LIGHTGRAY );
    }
    ((SwTabFrm*)this)->ResetComplete();
}

/*************************************************************************
|*
|*  SwFrm::PaintShadow()
|*
|*  Beschreibung        Malt einen Shatten wenns das FrmFormat fordert.
|*      Der Schatten wird immer an den auesseren Rand des OutRect gemalt.
|*      Das OutRect wird ggf. so verkleinert, dass auf diesem das
|*      malen der Umrandung stattfinden kann.
|*  Ersterstellung      MA 21. Dec. 92
|*  Letzte Aenderung    MA 29. May. 97
|*
|*************************************************************************/

void SwFrm::PaintShadow( const SwRect& rRect, SwRect& rOutRect,
                         const SwPageFrm *pPage,
                         const SwBorderAttrs &rAttrs ) const
{
    const SvxShadowItem &rShadow = rAttrs.GetShadow();
    const long nWidth  = ::lcl_AlignWidth ( rShadow.GetWidth() );
    const long nHeight = ::lcl_AlignHeight( rShadow.GetWidth() );

    SwRects aRegion( 2, 2 );
    SwRect aOut( rOutRect );

    const FASTBOOL bCnt    = IsCntntFrm();
    const FASTBOOL bTop    = !bCnt || rAttrs.GetTopLine  ( this ) ? TRUE : FALSE;
    const FASTBOOL bBottom = !bCnt || rAttrs.GetBottomLine( this ) ? TRUE : FALSE;

    switch ( rShadow.GetLocation() )
    {
        case SVX_SHADOW_BOTTOMRIGHT:
            {
                aOut.Top ( aOut.Bottom() - nHeight );
                aOut.Left( aOut.Left()   + nWidth );
                if ( bBottom )
                    aRegion.Insert( aOut, aRegion.Count() );
                aOut.Left( aOut.Right()   - nWidth );
                aOut.Top ( rOutRect.Top() + nHeight );
                if ( bBottom )
                    aOut.Bottom( aOut.Bottom() - nHeight );
                if ( bCnt && (!bTop || !bBottom) )
                    ::lcl_ExtendLeftAndRight( aOut, this, rAttrs );
                aRegion.Insert( aOut, aRegion.Count() );

                rOutRect.Right ( rOutRect.Right() - nWidth );
                rOutRect.Bottom( rOutRect.Bottom()- nHeight );
            }
            break;
        case SVX_SHADOW_TOPLEFT:
            {
                aOut.Bottom( aOut.Top()   + nHeight );
                aOut.Right ( aOut.Right() - nWidth );
                if ( bTop )
                    aRegion.Insert( aOut, aRegion.Count() );
                aOut.Right ( aOut.Left() + nWidth );
                aOut.Bottom( rOutRect.Bottom() - nHeight );
                if ( bTop )
                    aOut.Top( aOut.Top() + nHeight );
                if ( bCnt && (!bBottom || !bTop) )
                    ::lcl_ExtendLeftAndRight( aOut, this, rAttrs );
                aRegion.Insert( aOut, aRegion.Count() );

                rOutRect.Left( rOutRect.Left() + nWidth );
                rOutRect.Top(  rOutRect.Top() + nHeight );
            }
            break;
        case SVX_SHADOW_TOPRIGHT:
            {
                aOut.Bottom( aOut.Top() + nHeight );
                aOut.Left (  aOut.Left()+ nWidth );
                if ( bTop )
                    aRegion.Insert( aOut, aRegion.Count() );
                aOut.Left  ( aOut.Right() - nWidth );
                aOut.Bottom( rOutRect.Bottom() - nHeight );
                if ( bTop )
                    aOut.Top( aOut.Top() + nHeight );
                if ( bCnt && (!bBottom || bTop) )
                    ::lcl_ExtendLeftAndRight( aOut, this, rAttrs );
                aRegion.Insert( aOut, aRegion.Count() );

                rOutRect.Right( rOutRect.Right() - nWidth );
                rOutRect.Top( rOutRect.Top() + nHeight );
            }
            break;
        case SVX_SHADOW_BOTTOMLEFT:
            {
                aOut.Top  ( aOut.Bottom()- nHeight );
                aOut.Right( aOut.Right() - nWidth );
                if ( bBottom )
                    aRegion.Insert( aOut, aRegion.Count() );
                aOut.Right( aOut.Left() + nWidth );
                aOut.Top( rOutRect.Top() + nHeight );
                if ( bBottom )
                    aOut.Bottom( aOut.Bottom() - nHeight );
                if ( bCnt && (!bTop || !bBottom) )
                    ::lcl_ExtendLeftAndRight( aOut, this, rAttrs );
                aRegion.Insert( aOut, aRegion.Count() );

                rOutRect.Left( rOutRect.Left() + nWidth );
                rOutRect.Bottom( rOutRect.Bottom() - nHeight );
            }
            break;
#ifndef PRODUCT
        default:    ASSERT( !this, "new ShadowLocation() ?" );
#endif
    }

    OutputDevice *pOut = pGlobalShell->GetOut();
    if ( pOut->GetFillColor() != rShadow.GetColor() )
        pOut->SetFillColor( rShadow.GetColor() );
    for ( USHORT i = 0; i < aRegion.Count(); ++i )
    {
        SwRect &rOut = aRegion[i];
        aOut = rOut;
        ::SwAlignRect( aOut, pGlobalShell );
        if ( rRect.IsOver( aOut ) && aOut.Height() > 0 && aOut.Width() > 0 )
        {
            aOut._Intersection( rRect );
            pOut->DrawRect( aOut.SVRect() );
        }
    }
}

/*************************************************************************
|*
|*  SwFrm::PaintBorderLine()
|*
|*  Ersterstellung      MA 22. Dec. 92
|*  Letzte Aenderung    MA 22. Jan. 95
|*
|*************************************************************************/

void SwFrm::PaintBorderLine( const SwRect& rRect,
                             const SwRect& rOutRect,
                             const SwPageFrm *pPage,
                             const Color *pColor ) const
{
    if ( !rOutRect.IsOver( rRect ) )
        return;

    SwRect aOut( rOutRect );
    aOut._Intersection( rRect );

    const SwTabFrm *pTab = IsCellFrm() ? FindTabFrm() : 0;
    if ( pPage->GetSortedObjs() )
    {
        SwRegionRects aRegion( aOut, 4, 1 );
        ::lcl_SubtractFlys( this, pPage, aOut, aRegion );
        for ( USHORT i = 0; i < aRegion.Count(); ++i )
            pLines->AddLineRect( aRegion[i], pColor, pTab );
    }
    else
        pLines->AddLineRect( aOut, pColor, pTab );
}

/*************************************************************************
|*
|*  SwFrm::PaintBorderLines()
|*
|*  Beschreibung        Nur alle Linien einfach oder alle Linien doppelt!!!!
|*  Ersterstellung      MA 22. Dec. 92
|*  Letzte Aenderung    MA 22. Mar. 95
|*
|*************************************************************************/

void MA_FASTCALL lcl_SubTopBottom( SwRect &rRect, const SvxBoxItem &rBox,
                          const SwBorderAttrs &rAttrs, const SwFrm *pFrm )
{
    const BOOL bCnt = pFrm->IsCntntFrm();
    if ( rBox.GetTop() && rBox.GetTop()->GetInWidth() &&
         (!bCnt || rAttrs.GetTopLine( pFrm )) )
    {
        const long nDist = ::lcl_MinHeightDist( rBox.GetTop()->GetDistance() );
        rRect.Top( rRect.Top() +
                         ::lcl_AlignHeight( rBox.GetTop()->GetOutWidth() ) + nDist );
    }

    if ( rBox.GetBottom() && rBox.GetBottom()->GetInWidth() &&
         (!bCnt || rAttrs.GetBottomLine( pFrm)))
    {
        const long nDist = ::lcl_MinHeightDist( rBox.GetBottom()->GetDistance() );
        rRect.Bottom( rRect.Bottom() -
                        (::lcl_AlignHeight( rBox.GetBottom()->GetOutWidth() + 1 ) +
                         nDist));
    }
}

void MA_FASTCALL lcl_SubLeftRight( SwRect &rRect, const SvxBoxItem &rBox )
{
    if ( rBox.GetLeft() && rBox.GetLeft()->GetInWidth() )
    {
        const long nDist = ::lcl_MinWidthDist( rBox.GetLeft()->GetDistance() );
        rRect.Left( rRect.Left() +
                        ::lcl_AlignWidth( rBox.GetLeft()->GetOutWidth() ) + nDist );
    }

    if ( rBox.GetRight() && rBox.GetRight()->GetInWidth() )
    {
        const long nDist = ::lcl_MinWidthDist( rBox.GetRight()->GetDistance() );
        rRect.Right( rRect.Right() -
                        (::lcl_AlignWidth( rBox.GetRight()->GetOutWidth() + 1 ) +
                         nDist));
    }
}

void MA_FASTCALL lcl_PaintLeftLine( const SwFrm *pFrm, const SwPageFrm *pPage,
                           const SwRect &rOutRect, const SwRect &rRect,
                           const SwBorderAttrs &rAttrs )
{
    const SvxBoxItem &rBox = rAttrs.GetBox();
    const SvxBorderLine *pLeft = rBox.GetLeft();

    if ( !pLeft )
        return;

    SwRect aRect( rOutRect );
    aRect.Width( ::lcl_AlignWidth( pLeft->GetOutWidth() ) );

    const BOOL bCnt = pFrm->IsCntntFrm();

    if ( bCnt )
        ::lcl_ExtendLeftAndRight( aRect, pFrm, rAttrs );

    if ( !pLeft->GetInWidth() )
        ::lcl_SubTopBottom( aRect, rBox, rAttrs, pFrm );

    pFrm->PaintBorderLine( rRect, aRect, pPage, &pLeft->GetColor() );

    if ( pLeft->GetInWidth() )
    {
        const long nDist = ::lcl_MinWidthDist( pLeft->GetDistance() );
        aRect.Pos().X() = aRect.Right() + nDist;
        aRect.Width( ::lcl_AlignWidth( pLeft->GetInWidth() ) );
        ::lcl_SubTopBottom( aRect, rBox, rAttrs, pFrm );
        pFrm->PaintBorderLine( rRect, aRect, pPage, &pLeft->GetColor() );
    }
}

void MA_FASTCALL lcl_PaintRightLine( const SwFrm *pFrm, const SwPageFrm *pPage,
                            const SwRect &rOutRect, const SwRect &rRect,
                            const SwBorderAttrs &rAttrs )
{
    const SvxBoxItem &rBox = rAttrs.GetBox();
    const SvxBorderLine *pRight = rBox.GetRight();

    if ( !pRight )
        return;

    SwRect aRect( rOutRect );
    aRect.Left( aRect.Right() - ::lcl_AlignWidth( pRight->GetOutWidth() ) + 1 );

    const BOOL bCnt = pFrm->IsCntntFrm();

    if ( bCnt )
        ::lcl_ExtendLeftAndRight( aRect, pFrm, rAttrs );

    if ( !pRight->GetInWidth() )
        ::lcl_SubTopBottom( aRect, rBox, rAttrs, pFrm );

    pFrm->PaintBorderLine( rRect, aRect, pPage, &pRight->GetColor() );

    if ( pRight->GetInWidth() )
    {
        const long nDist = ::lcl_MinWidthDist( pRight->GetDistance() );
        aRect.Right( aRect.Left() - nDist );
        aRect.Left ( aRect.Right()- ::lcl_AlignWidth( pRight->GetInWidth()  ) + 1 );
        ::lcl_SubTopBottom( aRect, rBox, rAttrs, pFrm );
        pFrm->PaintBorderLine( rRect, aRect, pPage, &pRight->GetColor() );
    }
}

void MA_FASTCALL lcl_PaintTopLine( const SwFrm *pFrm, const SwPageFrm *pPage,
                          const SwRect &rOutRect, const SwRect &rRect,
                          const SwBorderAttrs &rAttrs )
{
    const SvxBoxItem &rBox = rAttrs.GetBox();
    const SvxBorderLine *pTop = rBox.GetTop();

    if ( !pTop )
        return;

    SwRect aRect( rOutRect );
    aRect.Height( ::lcl_AlignHeight( pTop->GetOutWidth() ) );
    pFrm->PaintBorderLine( rRect, aRect, pPage, &pTop->GetColor() );

    if ( pTop->GetInWidth() )
    {
        const long nDist = ::lcl_MinHeightDist( pTop->GetDistance() );
        aRect.Pos().Y() = aRect.Bottom() + nDist;
        aRect.Height( ::lcl_AlignHeight( pTop->GetInWidth() ) );
        ::lcl_SubLeftRight( aRect, rBox );
        pFrm->PaintBorderLine( rRect, aRect, pPage, &pTop->GetColor() );
    }
}

void MA_FASTCALL lcl_PaintBottomLine( const SwFrm *pFrm, const SwPageFrm *pPage,
                             const SwRect &rOutRect, const SwRect &rRect,
                             const SwBorderAttrs &rAttrs )
{
    const SvxBoxItem &rBox = rAttrs.GetBox();
    const SvxBorderLine *pBottom = rBox.GetBottom();

    if ( !pBottom )
        return;

    SwRect aRect( rOutRect );
    aRect.Top( aRect.Bottom() - ::lcl_AlignHeight( pBottom->GetOutWidth() ) + 1);
    pFrm->PaintBorderLine( rRect, aRect, pPage, &pBottom->GetColor() );

    if ( pBottom->GetInWidth() )
    {
        const long nDist = ::lcl_MinHeightDist( pBottom->GetDistance() );
        aRect.Bottom( aRect.Top()   - nDist );
        aRect.Top   ( aRect.Bottom()- ::lcl_AlignHeight( pBottom->GetInWidth()  ) + 1 );
        ::lcl_SubLeftRight( aRect, rBox );
        pFrm->PaintBorderLine( rRect, aRect, pPage, &pBottom->GetColor() );
    }
}

/*************************************************************************
|*
|*  SwFrm::PaintBorder()
|*
|*  Beschreibung        Malt Schatten und Umrandung
|*  Ersterstellung      MA 23.01.92
|*  Letzte Aenderung    MA 29. Jul. 96
|*
|*************************************************************************/

void SwFrm::PaintBorder( const SwRect& rRect, const SwPageFrm *pPage,
                         const SwBorderAttrs &rAttrs ) const
{
    //fuer (Row,Body,Ftn,Root,Column,NoTxt) gibt's hier nix zu tun
    if ( (GetType() & 0x90C5) || (Prt().SSize() == Frm().SSize()) )
        return;

    if ( (GetType() & 0x2000) &&    //Cell
         !pGlobalShell->GetViewOptions()->IsTable() )
        return;

    const FASTBOOL bLine   = rAttrs.IsLine();
    const FASTBOOL bShadow = rAttrs.GetShadow().GetLocation() != SVX_SHADOW_NONE;
    if ( bLine || bShadow )
    {
        //Wenn das Rechteck vollstandig innerhalb der PrtArea liegt,
        //so braucht kein Rand gepainted werden.
        //Fuer die PrtArea muss der Aligned'e Wert zugrunde gelegt werden,
        //anderfalls wuerden u.U. Teile nicht verarbeitet.
        SwRect aRect( Prt() );
        aRect += Frm().Pos();
        ::SwAlignRect( aRect, pGlobalShell );
        if ( aRect.IsInside( rRect ) )
            return;

        if ( !pPage )
            pPage = FindPageFrm();

        ::lcl_CalcBorderRect( aRect, this, rAttrs, TRUE );
        rAttrs.SetGetCacheLine( TRUE );
        if ( bShadow )
            PaintShadow( rRect, aRect, pPage, rAttrs );
        if ( bLine )
        {
            ::lcl_PaintLeftLine  ( this, pPage, aRect, rRect, rAttrs );
            ::lcl_PaintRightLine ( this, pPage, aRect, rRect, rAttrs );
            if ( !IsCntntFrm() || rAttrs.GetTopLine( this ) )
                ::lcl_PaintTopLine( this, pPage, aRect, rRect, rAttrs );
            if ( !IsCntntFrm() || rAttrs.GetBottomLine( this ) )
                ::lcl_PaintBottomLine( this, pPage, aRect, rRect, rAttrs );
        }
        rAttrs.SetGetCacheLine( FALSE );
    }
}
/*************************************************************************
|*
|*  SwFtnContFrm::PaintBorder()
|*
|*  Beschreibung        Spezialimplementierung wg. der Fussnotenlinie.
|*      Derzeit braucht nur der obere Rand beruecksichtigt werden.
|*      Auf andere Linien und Schatten wird verzichtet.
|*  Ersterstellung      MA 27. Feb. 93
|*  Letzte Aenderung    MA 08. Sep. 93
|*
|*************************************************************************/

void SwFtnContFrm::PaintBorder( const SwRect& rRect, const SwPageFrm *pPage,
                                const SwBorderAttrs & ) const
{
    //Wenn das Rechteck vollstandig innerhalb der PrtArea liegt, so gibt es
    //keinen Rand zu painten.
    SwRect aRect( Prt() );
    aRect.Pos() += Frm().Pos();
    if ( !aRect.IsInside( rRect ) )
        PaintLine( rRect, pPage );
}
/*************************************************************************
|*
|*  SwFtnContFrm::PaintLine()
|*
|*  Beschreibung        Fussnotenline malen.
|*  Ersterstellung      MA 02. Mar. 93
|*  Letzte Aenderung    MA 28. Mar. 94
|*
|*************************************************************************/

void SwFtnContFrm::PaintLine( const SwRect& rRect,
                              const SwPageFrm *pPage ) const
{
    //Laenge der Linie ergibt sich aus der prozentualen Angabe am PageDesc.
    //Die Position ist ebenfalls am PageDesc angegeben.
    //Der Pen steht direkt im PageDesc.

    if ( !pPage )
        pPage = FindPageFrm();
    const SwPageFtnInfo &rInf = pPage->GetPageDesc()->GetFtnInfo();

    Fraction aFract( Prt().Width(), 1 );
    const SwTwips nWidth = (long)(aFract *= rInf.GetWidth());

    SwTwips nX = Frm().Left() + Prt().Left();
    switch ( rInf.GetAdj() )
    {
        case FTNADJ_CENTER:
            nX += Prt().Width()/2 - nWidth/2; break;
        case FTNADJ_RIGHT:
            nX = nX + Prt().Width() - nWidth; break;
        case FTNADJ_LEFT:
            /* do nothing */; break;
        default:
            ASSERT( !this, "Neues Adjustment fuer Fussnotenlinie?" );
    }
    const SwRect aLineRect( Point( nX, Frm().Pos().Y() + rInf.GetTopDist() ),
                            Size( nWidth, rInf.GetLineWidth()));

    if ( aLineRect.HasArea() )
        PaintBorderLine( rRect, aLineRect , pPage, &rInf.GetLineColor() );
}

/*************************************************************************
|*
|*  SwLayoutFrm::PaintColLines()
|*
|*  Beschreibung        Painted die Trennlinien fuer die innenliegenden
|*                      Spalten.
|*  Ersterstellung      MA 21. Jun. 93
|*  Letzte Aenderung    MA 28. Mar. 94
|*
|*************************************************************************/

void SwLayoutFrm::PaintColLines( const SwRect &rRect, const SwFmtCol &rFmtCol,
                                 const SwPageFrm *pPage ) const
{
    const SwFrm *pCol = Lower();
    if ( !pCol || !pCol->IsColumnFrm() )
        return;

    //Laenge der Linie ergibt sich aus der prozentualen Angabe im Attribut.
    //Die Position ist ebenfalls im Attribut angegeben.
    //Der Pen steht direkt im Attribut.

    const SwTwips nHeight = Prt().Height() * rFmtCol.GetLineHeight() / 100;

    SwTwips nY = Frm().Top() + Prt().Top();
    switch ( rFmtCol.GetLineAdj() )
    {
        case COLADJ_CENTER:
            nY += Prt().Height()/2 - nHeight/2; break;
        case COLADJ_BOTTOM:
            nY = nY + Prt().Height() - nHeight; break;
        case COLADJ_TOP:
            /* do nothing */; break;
        default:
            ASSERT( !this, "Neues Adjustment fuer Spaltenlinie?" );
    }

    const Size aSz( rFmtCol.GetLineWidth(), nHeight );
    const SwTwips nPenHalf = rFmtCol.GetLineWidth() / 2;

    //Damit uns nichts verlorengeht muessen wir hier etwas grosszuegiger sein.
    SwRect aRect( rRect );
    aRect.Pos().X() -= rFmtCol.GetLineWidth() + nPixelSzW;
    aRect.SSize().Width() += 2 * rFmtCol.GetLineWidth() + nPixelSzW;

    while ( pCol->GetNext() )
    {
        const SwRect aLineRect( Point( pCol->Frm().Right() - nPenHalf, nY ),
                                aSz );
        if ( aRect.IsOver( aLineRect ) )
            PaintBorderLine( aRect, aLineRect , pPage, &rFmtCol.GetLineColor() );
        pCol = pCol->GetNext();
    }
}

/*************************************************************************
|*
|*  SwPageFrm::PaintAllBorders()
|*
|*  Beschreibung        Malt Schatten und Umrandung aller innenliegenden Frms
|*  Ersterstellung      MA 23. Jan. 95
|*  Letzte Aenderung    MA 06. May. 95
|*
|*************************************************************************/

void MA_FASTCALL lcl_PaintLowerBorders( const SwLayoutFrm *pLay,
                               const SwRect &rRect, const SwPageFrm *pPage )
{
    const SwFrm *pFrm = pLay->Lower();
    if ( pFrm )
    {
        OutputDevice *pOut = pGlobalShell->GetOut();
        pOut->Push( PUSH_FILLCOLOR );
        do
        {   if ( pFrm->Frm().IsOver( rRect ) )
            {
                if ( pFrm->IsLayoutFrm() )
                    ::lcl_PaintLowerBorders( (SwLayoutFrm*)pFrm, rRect, pPage );

                SwBorderAttrAccess aAccess( SwFrm::GetCache(), (SwFrm*)pFrm );
                const SwBorderAttrs &rAttrs = *aAccess.Get();
                pFrm->PaintBorder( rRect, pPage, rAttrs );
            }
            pFrm = pFrm->GetNext();

        } while ( pFrm && !::IsShortCut( rRect, pFrm->Frm() ) );
        pOut->Pop();
    }
}

void SwPageFrm::PaintAllBorders( const SwRect &rRect ) const
{
    ::lcl_PaintLowerBorders( this, rRect, this );
}
/*************************************************************************
|*
|*  SwFrm::PaintBaBo()
|*
|*  Ersterstellung      MA 22. Oct. 93
|*  Letzte Aenderung    MA 19. Jun. 96
|*
|*************************************************************************/

void SwFrm::PaintBaBo( const SwRect& rRect, const SwPageFrm *pPage,
                       const BOOL bLowerBorder ) const
{
    if ( !pPage )
        pPage = FindPageFrm();

    OutputDevice *pOut = pGlobalShell->GetOut();
    pOut->Push( PUSH_FILLCOLOR );

    SwBorderAttrAccess aAccess( SwFrm::GetCache(), (SwFrm*)this );
    const SwBorderAttrs &rAttrs = *aAccess.Get();

/*  //FlyInCnt's kommen hier aus der ::Retouche hier an.
    FASTBOOL bUnlock = FALSE;
    if ( IsFlyFrm() && ((SwFlyFrm*)this)->IsFlyInCntFrm() )
    {
        bLockFlyBackground = bUnlock = TRUE;
        const SwFlyFrm *pFly = (const SwFlyFrm*)this;
        SwBorderAttrAccess aAccess( SwFrm::GetCache(), pFly );
        const SwBorderAttrs &rAttrs = *aAccess.Get();
        pFly->GetAnchor()->PaintBackground( rRect, pPage, rAttrs, FALSE );
    }
*/
    PaintBackground( rRect, pPage, rAttrs, FALSE, bLowerBorder );
    SwRect aRect( rRect );
    ::SizeBorderRect( aRect );
    PaintBorder( aRect, pPage, rAttrs );
/*  if ( bUnlock )
        bLockFlyBackground = FALSE;
*/
    pOut->Pop();
}

/*************************************************************************
|*
|*  SwFrm::PaintBackground()
|*
|*  Ersterstellung      MA 04. Jan. 93
|*  Letzte Aenderung    MA 06. Feb. 97
|*
|*************************************************************************/

void SwFrm::PaintBackground( const SwRect &rRect, const SwPageFrm *pPage,
                              const SwBorderAttrs & rAttrs,
                             const BOOL bLowerMode,
                             const BOOL bLowerBorder ) const
{
    FASTBOOL bResetPageOnly = FALSE;

    ViewShell *pSh = pGlobalShell;
    const FASTBOOL bWin = pSh->GetWin() ? TRUE : FALSE;
    const SvxBrushItem* pItem;
    const Color* pCol;
    SwRect aOrigBackRect;
    FASTBOOL bBack= GetBackgroundBrush( pItem, pCol, aOrigBackRect, bLowerMode );
    const FASTBOOL bPageFrm = IsPageFrm();
    FASTBOOL bLowMode = TRUE;

    //- Ausgabe wenn ein eigener Hintergrund mitgebracht wird.
    //- Retouche fuer durchsichtige Flys muss vom Hintergrund des Flys
    //  uebernommen werden.
    //  ->Selbiges fuer Flys die eine Grafik enthalten.
    FASTBOOL bFlyBackground = !bFlyMetafile && !bBack && IsFlyFrm();
    if ( bFlyBackground && (!GetLower() || !((SwFlyFrm*)this)->Lower()->IsNoTxtFrm()) )
         bPageOnly = bResetPageOnly = TRUE;

    SwRect aPaintRect( Frm() );
    if( IsTxtFrm() )
        aPaintRect = UnionFrm( TRUE );

    if ( aPaintRect.IsOver( rRect ) )
    {
    if ( bFlyBackground )
    {
        const SwFrm *pBackFrm = bPageOnly || !((SwFlyFrm*)this)->GetAnchor()->IsInFly()
                            ? (SwFrm*)pPage
                            : ((SwFlyFrm*)this)->GetAnchor()->FindFlyFrm();

        SwFlyFrm *pOld = pRetoucheFly;
        pRetoucheFly = (SwFlyFrm*)this;

        SwRect aRect;
        ::lcl_CalcBorderRect( aRect, this, rAttrs, FALSE );
        aRect.Intersection( rRect );
        pBackFrm->PaintBaBo( aRect, pPage, bLowerBorder );
        pRetoucheFly = pOld;
        //Clipping muss aufgehoben werden, weil vom Formatter oft ein Clipping
        //gesetzt wird.
        OutputDevice *pOut = pSh->GetOut();
        pOut->Push( PUSH_CLIPREGION );
        pOut->SetClipRegion();
        pLines->PaintLines( GetShell()->GetOut() );
        pOut->Pop();

    }
    else if ( bBack || bPageFrm || !bLowerMode )
    {
        const FASTBOOL bBrowse = pSh->GetDoc()->IsBrowseMode();

        SwRect aRect;
        if ( (bPageFrm && bBrowse) ||
             (IsTxtFrm() && Prt().SSize() == Frm().SSize()) )
        {
            aRect = Frm();
            ::SwAlignRect( aRect, pGlobalShell );
        }
        else
        {
            ::lcl_CalcBorderRect( aRect, this, rAttrs, FALSE );
            if ( (IsTxtFrm() || IsTabFrm()) && GetPrev() )
            {
                if ( GetPrev()->GetAttrSet()->GetBackground() ==
                     GetAttrSet()->GetBackground() )
                {
                    aRect.Top( Frm().Top() );
                }
            }
        }
        aRect.Intersection( rRect );

        OutputDevice *pOut = pSh->GetOut();

        if ( bPageFrm && bWin && !bBrowse )
        {
            //Irgendjemand muss sich um den Rand der Seite kuemmern. Eine Farbe
            //kann fuer diesen Rand niemals angegeben sein.
            SwRect aPgRect( Prt() );
            aPgRect.Pos() += Frm().Pos();
            if ( !aPgRect.IsInside( rRect ) )
            {
                aPgRect = Frm();
                aPgRect._Intersection( rRect );
                SwRegionRects aPgRegion( aPgRect );
                aPgRegion -= aRect;
                if ( pPage->GetSortedObjs() )
                    ::lcl_SubtractFlys( this, pPage, aPgRect, aPgRegion );
                if ( aPgRegion.Count() )
                {
                    if ( pOut->GetFillColor() != aGlobalRetoucheColor )
                        pOut->SetFillColor( aGlobalRetoucheColor );
                    for ( USHORT i = 0; i < aPgRegion.Count(); ++i )
                    {
                        if ( 1 < aPgRegion.Count() )
                        {
                            ::SwAlignRect( aPgRegion[i], pGlobalShell );
                            if( !aPgRegion[i].HasArea() )
                                continue;
                        }
                        pOut->DrawRect( aPgRegion[i].SVRect() );
                    }
                }
            }
        }
        if ( aRect.HasArea() )
        {
            SvxBrushItem* pNewItem;
            SwRegionRects aRegion( aRect );
            if( pCol )
            {
                pNewItem = new SvxBrushItem( *pCol );
                pItem = pNewItem;
            }
            if ( pPage->GetSortedObjs() )
                ::lcl_SubtractFlys( this, pPage, aRect, aRegion );
            for ( USHORT i = 0; i < aRegion.Count(); ++i )
            {
                if ( 1 < aRegion.Count() )
                {
                    ::SwAlignRect( aRegion[i], pGlobalShell );
                    if( !aRegion[i].HasArea() )
                        continue;
                }
                ::DrawGraphic( pItem, pOut, aOrigBackRect, aRegion[i] );
            }
            if( pCol )
                delete pNewItem;
        }
    }
    else
        bLowMode = bLowerMode ? TRUE : FALSE;
    }
    if ( bResetPageOnly )
        bPageOnly = FALSE;

    //Jetzt noch Lower und dessen Nachbarn.
    //Wenn ein Frn dabei die Kette verlaesst also nicht mehr Lower von mir ist
    //so hoert der Spass auf.
    const SwFrm *pFrm = GetLower();
    if ( !bPageOnly && pFrm )
    {
        SwRect aFrmRect;
        SwRect aRect( PaintArea() );
        aRect._Intersection( rRect );
        SwRect aBorderRect( aRect );
        ::SizeBorderRect( aBorderRect );
        do
        {   if ( pProgress )
                pProgress->Reschedule();

            aFrmRect = pFrm->PaintArea();
            if ( aFrmRect.IsOver( aBorderRect ) )
            {
                SwBorderAttrAccess aAccess( SwFrm::GetCache(), (SwFrm*)pFrm );
                const SwBorderAttrs &rAttrs = *aAccess.Get();
                if ( ( pFrm->IsLayoutFrm() && bLowerBorder ) ||
                     aFrmRect.IsOver( aRect ) )
                    pFrm->PaintBackground( aRect, pPage, rAttrs, bLowMode,
                                           bLowerBorder );
                if ( bLowerBorder )
                    pFrm->PaintBorder( aBorderRect, pPage, rAttrs );
            }
            pFrm = pFrm->GetNext();
        } while ( pFrm && pFrm->GetUpper() == this &&
                  !::IsShortCut( aBorderRect, aFrmRect ) );
    }
}

/*************************************************************************
|*
|*  SwPageFrm::RefreshSubsidiary()
|*
|*  Beschreibung        Erneuert alle Hilfslinien der Seite.
|*  Ersterstellung      MA 04. Nov. 92
|*  Letzte Aenderung    MA 10. May. 95
|*
|*************************************************************************/

void SwPageFrm::RefreshSubsidiary( const SwRect &rRect ) const
{
    if ( IS_SUBS || IS_SUBS_TABLE || IS_SUBS_SECTION )
    {
        SwRect aRect( rRect );
        ::SwAlignRect( aRect, pGlobalShell );
        if ( aRect.HasArea() )
        {
            //Beim Paint ueber die Root wird das Array von dort gesteuert.
            //Anderfalls kuemmern wir uns selbst darum.
            FASTBOOL bDelSubs = FALSE;
            if ( !pSubsLines )
            {
                pSubsLines = new SwSubsRects;
                bDelSubs = TRUE;
            }

            RefreshLaySubsidiary( this, aRect );
            if ( GetSortedObjs() )
            {
                const SwSortDrawObjs &rObjs = *GetSortedObjs();
                for ( USHORT i = 0; i < rObjs.Count(); ++i )
                {
                    SdrObject *pO = rObjs[i];
                    if ( pO->IsWriterFlyFrame() )
                    {
                        const SwFlyFrm *pFly = ((SwVirtFlyDrawObj*)pO)->GetFlyFrm();
                        if ( pFly->Frm().IsOver( aRect ) )
                        {
                            if ( !pFly->Lower() || !pFly->Lower()->IsNoTxtFrm() ||
                                    !((SwNoTxtFrm*)pFly->Lower())->HasAnimation())
                                pFly->RefreshLaySubsidiary( this, aRect );
                        }
                    }
                }
            }
            if ( bDelSubs )
            {
                pSubsLines->PaintSubsidiary( pGlobalShell->GetOut(), pLines );
                DELETEZ( pSubsLines );
            }
        }
    }
}

/*************************************************************************
|*
|*  SwLayoutFrm::RefreshLaySubsidiary()
|*
|*  Ersterstellung      MA 04. Nov. 92
|*  Letzte Aenderung    MA 22. Jan. 95
|*
|*************************************************************************/

void SwLayoutFrm::RefreshLaySubsidiary( const SwPageFrm *pPage,
                                        const SwRect &rRect ) const
{
    const FASTBOOL bNoLowerColumn = !Lower() || !Lower()->IsColumnFrm();
    const FASTBOOL bSubsOpt   = IS_SUBS;
    const FASTBOOL bSubsTable = ((GetType() & (FRM_ROW | FRM_CELL)) && IS_SUBS_TABLE);
    const FASTBOOL bSubsOther = (GetType() & (FRM_HEADER | FRM_FOOTER | FRM_FTN )) && bSubsOpt;
    const FASTBOOL bSubsSect  = IsSctFrm() &&
                                bNoLowerColumn &&
                                IS_SUBS_SECTION;
    const FASTBOOL bSubsFly   = (GetType() & FRM_FLY) &&
                                bSubsOpt &&
                                bNoLowerColumn &&
                                (!Lower() || !Lower()->IsNoTxtFrm() ||
                                 !((SwNoTxtFrm*)Lower())->HasAnimation());
    FASTBOOL bSubsBody = FALSE;
    if ( GetType() & FRM_BODY )
    {
        if ( IsPageBodyFrm() )
            bSubsBody = bSubsOpt && bNoLowerColumn;                                 //nur ohne Spalten
        else    //Spaltenbody
        {
            if ( GetUpper()->GetUpper()->IsSctFrm() )
                bSubsBody = IS_SUBS_SECTION;
            else
                bSubsBody = bSubsOpt;
        }
    }

    if ( bSubsOther || bSubsSect  || bSubsBody || bSubsTable || bSubsFly )
        PaintSubsidiaryLines( pPage, rRect );

    const SwFrm *pLow = Lower();
    while ( pLow && !::IsShortCut( rRect, pLow->Frm() ) )
    {   if ( pLow->Frm().IsOver( rRect ) && pLow->Frm().HasArea() )
        {
            if ( pLow->IsLayoutFrm() )
                ((const SwLayoutFrm*)pLow)->RefreshLaySubsidiary( pPage, rRect);
            else if ( pLow->GetDrawObjs() )
            {
                const SwDrawObjs &rObjs = *pLow->GetDrawObjs();
                for ( USHORT i = 0; i < rObjs.Count(); ++i )
                {
                    SdrObject *pO = rObjs[i];
                    if ( pO->IsWriterFlyFrame() )
                    {
                        const SwFlyFrm *pFly = ((SwVirtFlyDrawObj*)pO)->GetFlyFrm();
                        if ( pFly->IsFlyInCntFrm() && pFly->Frm().IsOver( rRect ) )
                        {
                            if ( !pFly->Lower() || !pFly->Lower()->IsNoTxtFrm() ||
                                 !((SwNoTxtFrm*)pFly->Lower())->HasAnimation())
                                pFly->RefreshLaySubsidiary( pPage, rRect );
                        }
                    }
                }
            }
        }
        pLow = pLow->GetNext();
    }
}

/*************************************************************************
|*
|*  SwLayoutFrm::PaintSubsidiaryLines()
|*
|*  Beschreibung        Hilfslinien um die PrtAreas malen
|*      Nur die LayoutFrm's die direkt Cntnt enthalten.
|*  Ersterstellung      MA 21. May. 92
|*  Letzte Aenderung    MA 22. Jan. 95
|*
|*************************************************************************/

//Malt die angegebene Linie, achtet darauf, dass keine Flys uebermalt werden.

void MA_FASTCALL lcl_RefreshLine( const SwLayoutFrm *pLay, const SwPageFrm *pPage,
                         const Point &rP1, const Point &rP2, const BYTE nSubColor )
{
    //In welche Richtung gehts? Kann nur Horizontal oder Vertikal sein.
    ASSERT( ((rP1.X() == rP2.X()) || (rP1.Y() == rP2.Y())),
            "Schraege Hilfslinien sind nicht erlaubt." );
    const PtPtr pDirPt = rP1.X() == rP2.X() ? pY : pX;
    const PtPtr pOthPt = pDirPt == pX ? pY : pX;
    const SzPtr pDirSz = pDirPt == pX ? pWidth : pHeight;
    const SzPtr pOthSz = pDirSz == pWidth ? pHeight : pWidth;
    Point aP1( rP1 ),
          aP2( rP2 );

    while ( aP1.*pDirPt < aP2.*pDirPt )
    {   //Der Startpunkt wird jetzt, falls er in einem Fly sitzt, direkt
        //hinter den Fly gesetzt.
        //Wenn der Endpunkt in einem Fly sitzt oder zwischen Start und Endpunkt
        //ein Fly sitzt, so wird der Endpunkt eben an den Start herangezogen.
        //Auf diese art und weise wird eine Portion nach der anderen
        //ausgegeben.

        //Wenn ich selbst ein Fly bin, weiche ich nur denjenigen Flys aus,
        //die 'ueber' mir sitzen; d.h. die in dem Array hinter mir stehen.
        //Auch wenn ich in einem Fly sitze oder in einem Fly im Fly usw. weiche
        //ich keinem dieser Flys aus.
        SwOrderIter aIter( pPage );
        const SwFlyFrm *pMyFly = pLay->FindFlyFrm();
        if ( pMyFly )
        {
            aIter.Current( pMyFly->GetVirtDrawObj() );
            while ( 0 != (pMyFly = pMyFly->GetAnchor()->FindFlyFrm()) )
            {
                if ( aIter()->GetOrdNum() > pMyFly->GetVirtDrawObj()->GetOrdNum() )
                    aIter.Current( pMyFly->GetVirtDrawObj() );
            }
        }
        else
            aIter.Bottom();

        while ( aIter() )
        {
            const SwVirtFlyDrawObj *pObj = (SwVirtFlyDrawObj*)aIter();
            const SwFlyFrm *pFly = pObj ? pObj->GetFlyFrm() : 0;

            //Mir selbst weiche ich natuerlich nicht aus. Auch wenn ich
            //_in_ dem Fly sitze weiche ich nicht aus.
            if ( !pFly || (pFly == pLay || pFly->IsAnLower( pLay )) )
            {   aIter.Next();
                continue;
            }

            //Sitzt das Obj auf der Linie
            const Rectangle &rBound = pObj->GetBoundRect();
            const Point aDrPt( rBound.TopLeft() );
            const Size  aDrSz( rBound.GetSize() );
            if ( rP1.*pOthPt >= aDrPt.*pOthPt &&
                 rP1.*pOthPt <= (aDrPt.*pOthPt + aDrSz.*pOthSz) )
            {
                if ( aP1.*pDirPt >= aDrPt.*pDirPt &&
                         aP1.*pDirPt <= (aDrPt.*pDirPt + aDrSz.*pDirSz) )
                    aP1.*pDirPt = aDrPt.*pDirPt + aDrSz.*pDirSz;

                if ( aP2.*pDirPt >= aDrPt.*pDirPt &&
                     aP1.*pDirPt < (aDrPt.*pDirPt - 1) )
                    aP2.*pDirPt = aDrPt.*pDirPt - 1;
            }
            aIter.Next();
        }

        if ( aP1.*pDirPt < aP2.*pDirPt )
        {
            SwRect aRect( aP1, aP2 );
            pSubsLines->AddLineRect( aRect, 0, 0, nSubColor );
        }
        aP1 = aP2;
        aP1.*pDirPt += 1;
        aP2 = rP2;
    }
}

void SwLayoutFrm::PaintSubsidiaryLines( const SwPageFrm *pPage,
                                        const SwRect &rRect ) const
{
    //Wenn die Linien der Zellen nicht durchgehen siehts irgendwie nicht so
    //Toll aus; deswegen wird fuer die Zellen der Frm benutzt.
    const FASTBOOL bCell = IsCellFrm();
    const FASTBOOL bFlys = pPage->GetSortedObjs() ? TRUE : FALSE;
    SwRect aOriginal( bCell ? Frm() : Prt() );
    if ( !bCell )
        aOriginal.Pos() += Frm().Pos();

    ::SwAlignRect( aOriginal, pGlobalShell );
    if ( !aOriginal.IsOver( rRect ) )
        return;

    SwRect aOut( aOriginal );
    aOut._Intersection( rRect );
    aOut.Intersection( PaintArea() );

    const SwTwips nRight = aOut.Right();
    const SwTwips nBottom= aOut.Bottom();

    const Point aRT( nRight, aOut.Top() );
    const Point aRB( nRight, nBottom );
    const Point aLB( aOut.Left(), nBottom );

    BYTE nSubColor = SUBCOL_STANDARD;
    const SvxBrushItem &rBrush = pPage->GetFmt()->GetBackground();
    const Color aTmp( COL_LIGHTGRAY );
    if ( rBrush.GetColor() == aTmp ||
         (rBrush.GetColor().GetTransparency() &&
          aGlobalRetoucheColor == aTmp ))
    {
        nSubColor = SUBCOL_GRAY;
    }

    BOOL bBreak = FALSE;
    if ( GetType() & 0x0084 ) //Body oder Column
    {
        const SwCntntFrm *pCnt = ContainsCntnt();
        if ( pCnt )
        {
            if ( FALSE == (bBreak = pCnt->IsPageBreak( TRUE )) && IsColumnFrm() )
                bBreak = pCnt->IsColBreak( TRUE );
        }
    }

    if ( bFlys )
    {
        if ( aOriginal.Left() == aOut.Left() )
            ::lcl_RefreshLine( this, pPage, aOut.Pos(), aLB, nSubColor );
        if ( aOriginal.Right() == nRight )
            ::lcl_RefreshLine( this, pPage, aRT, aRB, nSubColor );
        if ( !bCell )
        {
            if ( aOriginal.Top() == aOut.Top() )
                ::lcl_RefreshLine( this, pPage, aOut.Pos(), aRT,
                                   bBreak ? SUBCOL_BREAK : nSubColor );
            if ( aOriginal.Bottom() == nBottom )
                ::lcl_RefreshLine( this, pPage, aLB, aRB, nSubColor );
        }
    }
    else
    {
        if ( aOriginal.Left() == aOut.Left() )
        {
            SwRect aRect( aOut.Pos(), aLB );
            pSubsLines->AddLineRect( aRect, 0, 0, nSubColor );
        }
        if ( aOriginal.Right() == nRight )
        {
            SwRect aRect( aRT, aRB );
            pSubsLines->AddLineRect( aRect, 0, 0, nSubColor );
        }
        if ( !bCell )
        {
            if ( aOriginal.Top() == aOut.Top() )
            {
                SwRect aRect( aOut.Pos(), aRT );
                pSubsLines->AddLineRect( aRect, 0, 0,
                                         bBreak ? SUBCOL_BREAK : nSubColor );
            }
            if ( aOriginal.Bottom() == nBottom )
            {
                SwRect aRect( aLB, aRB );
                pSubsLines->AddLineRect( aRect, 0, 0, nSubColor );
            }
        }
    }
}

/*************************************************************************
|*
|*  SwPageFrm::RefreshExtraData(), SwLayoutFrm::RefreshExtraData()
|*
|*  Beschreibung        Erneuert alle Extradaten (Zeilennummern usw) der Seite.
|*                      Grundsaetzlich sind nur diejenigen Objekte beruecksichtig,
|*                      die in die seitliche Ausdehnung des Rects ragen.
|*  Ersterstellung      MA 20. Jan. 98
|*  Letzte Aenderung    MA 18. Feb. 98
|*
|*************************************************************************/

void SwPageFrm::RefreshExtraData( const SwRect &rRect ) const
{
    const SwLineNumberInfo &rInfo = GetFmt()->GetDoc()->GetLineNumberInfo();
    FASTBOOL bLineInFly = rInfo.IsPaintLineNumbers() && rInfo.IsCountInFlys()
        || (SwHoriOrient)SW_MOD()->GetRedlineMarkPos() != HORI_NONE;

    SwRect aRect( rRect );
    ::SwAlignRect( aRect, pGlobalShell );
    if ( aRect.HasArea() )
    {
        SwLayoutFrm::RefreshExtraData( aRect );

        if ( bLineInFly && GetSortedObjs() )
            for ( USHORT i = 0; i < GetSortedObjs()->Count(); ++i )
            {
                SdrObject *pO = (*GetSortedObjs())[i];
                if ( pO->IsWriterFlyFrame() )
                {
                    const SwFlyFrm *pFly = ((SwVirtFlyDrawObj*)pO)->GetFlyFrm();
                    if ( pFly->Frm().Top() <= aRect.Bottom() &&
                         pFly->Frm().Bottom() >= aRect.Top() )
                        pFly->RefreshExtraData( aRect );
                }
            }
    }
}

void SwLayoutFrm::RefreshExtraData( const SwRect &rRect ) const
{

    const SwLineNumberInfo &rInfo = GetFmt()->GetDoc()->GetLineNumberInfo();
    FASTBOOL bLineInBody = rInfo.IsPaintLineNumbers(),
             bLineInFly  = bLineInBody && rInfo.IsCountInFlys(),
             bRedLine = (SwHoriOrient)SW_MOD()->GetRedlineMarkPos()!=HORI_NONE;

    const SwCntntFrm *pCnt = ContainsCntnt();
    while ( pCnt && IsAnLower( pCnt ) )
    {
        if ( pCnt->IsTxtFrm() && ( bRedLine ||
             ( !pCnt->IsInTab() &&
               ((bLineInBody && pCnt->IsInDocBody()) ||
               (bLineInFly  && pCnt->IsInFly())) ) ) &&
             pCnt->Frm().Top() <= rRect.Bottom() &&
             pCnt->Frm().Bottom() >= rRect.Top() )
        {
            ((SwTxtFrm*)pCnt)->PaintExtraData( rRect );
        }
        if ( bLineInFly && pCnt->GetDrawObjs() )
            for ( USHORT i = 0; i < pCnt->GetDrawObjs()->Count(); ++i )
            {
                SdrObject *pO = (*pCnt->GetDrawObjs())[i];
                if ( pO->IsWriterFlyFrame() )
                {
                    const SwFlyFrm *pFly = ((SwVirtFlyDrawObj*)pO)->GetFlyFrm();
                    if ( pFly->IsFlyInCntFrm() &&
                         pFly->Frm().Top() <= rRect.Bottom() &&
                         pFly->Frm().Bottom() >= rRect.Top() )
                        pFly->RefreshExtraData( rRect );
                }
        }
        pCnt = pCnt->GetNextCntntFrm();
    }
}

/*************************************************************************
|*
|*    SwFrm::Retouche
|*
|*    Beschreibung      Retouche fuer einen Bereich.
|*      Retouche wird nur dann durchgefuehrt, wenn der Frm der letzte seiner
|*      Kette ist. Der Gesamte Bereich des Upper unterhalb des Frm wird
|*      per PaintBackground gecleared.
|*    Ersterstellung    MA 13. Apr. 93
|*    Letzte Aenderung  MA 25. Jul. 96
|*
|*************************************************************************/

void SwFrm::Retouche( const SwPageFrm * pPage, const SwRect &rRect ) const
{
    if ( bFlyMetafile )
        return;

    ASSERT( GetUpper(), "Retoucheversuch ohne Upper." );
    ASSERT( GetShell() && pGlobalShell->GetWin(), "Retouche auf dem Drucker?" );

    SwRect aRetouche( GetUpper()->PaintArea() );
    aRetouche.Top( Frm().Top() + Frm().Height() );
    aRetouche.Intersection( pGlobalShell->VisArea() );

    if ( aRetouche.HasArea() )
    {
        //Uebergebenes Rect ausparen. Dafuer brauchen wir leider eine Region
        //zum ausstanzen.
        SwRegionRects aRegion( aRetouche );
        aRegion -= rRect;
        ViewShell *pSh = GetShell();
        for ( USHORT i = 0; i < aRegion.Count(); ++i )
        {
            SwRect &rRetouche = aRegion[i];

            GetUpper()->PaintBaBo( rRetouche, pPage, TRUE );

            //Hoelle und Himmel muessen auch refreshed werden.
            //Um Rekursionen zu vermeiden muss mein Retouche Flag zuerst
            //zurueckgesetzt werden!
            ResetRetouche();
            SwRect aRetouche( rRetouche );
            ::SizeBorderRect( aRetouche );
            pSh->Imp()->PaintLayer( pSh->GetDoc()->GetHellId(), aRetouche );
            pSh->Imp()->PaintLayer( pSh->GetDoc()->GetHeavenId(), aRetouche );
            pSh->Imp()->PaintLayer( pSh->GetDoc()->GetControlsId(), aRetouche );
            SetRetouche();

            //Da wir uns ausserhalb aller Paint-Bereiche begeben muessen hier
            //leider die Hilfslinien erneuert werden.
            pPage->RefreshSubsidiary( aRetouche );
        }
    }
    if ( ViewShell::IsLstEndAction() )
        ResetRetouche();
}

/*************************************************************************
|*
|*  SwFrm::GetBackgroundBrush()
|*
|*  Beschreibung        Liefert die Backgroundbrush fuer den Bereich des
|*      des Frm. Die Brush wird entweder von ihm selbst oder von einem
|*      Upper vorgegeben, die erste Brush wird benutzt.
|*      Ist fuer keinen Frm eine Brush angegeben, so wird FALSE zurueck-
|*      geliefert.
|*  Ersterstellung      MA 23. Dec. 92
|*  Letzte Aenderung    MA 04. Feb. 97
|*
|*************************************************************************/

BOOL SwFrm::GetBackgroundBrush( const SvxBrushItem* & rpBrush,
                                const Color*& rpCol,
                                SwRect &rOrigRect,
                                BOOL bLowerMode ) const
{
    const SwFrm *pFrm = this;
    ViewShell *pSh = GetShell();
    const SwViewOption *pOpt = pSh->GetViewOptions();
    rpBrush = 0;
    rpCol = NULL;
    do
    {   if ( pFrm->IsPageFrm() && !pOpt->IsPageBack() )
            return FALSE;

        const SvxBrushItem &rBack = pFrm->GetAttrSet()->GetBackground();
        if( pFrm->IsSctFrm() )
        {
            const SwSection* pSection = ((SwSectionFrm*)pFrm)->GetSection();
            if( pSection && ( TOX_HEADER_SECTION == pSection->GetType() ||
                TOX_CONTENT_SECTION == pSection->GetType() ) &&
                rBack.GetColor().GetTransparency() &&
                rBack.GetGraphicPos() == GPOS_NONE && pOpt->IsIndexBackground() &&
                pSh->GetOut()->GetOutDevType() != OUTDEV_PRINTER )
                rpCol = &pOpt->GetIndexBackgrndColor();
        }
        if ( !rBack.GetColor().GetTransparency() ||
             rBack.GetGraphicPos() != GPOS_NONE || rpCol )
        {
            rpBrush = &rBack;
            if ( pFrm->IsPageFrm() && pSh->GetDoc()->IsBrowseMode() )
                rOrigRect = pFrm->Frm();
            else
            {
                if ( pFrm->Frm().SSize() != pFrm->Prt().SSize() )
                {
                    SwBorderAttrAccess aAccess( SwFrm::GetCache(), pFrm );
                    const SwBorderAttrs &rAttrs = *aAccess.Get();
                    ::lcl_CalcBorderRect( rOrigRect, pFrm, rAttrs, FALSE );
                }
                else
                {
                    rOrigRect = pFrm->Prt();
                    rOrigRect += pFrm->Frm().Pos();
                }
            }
            return TRUE;
        }
        if ( bLowerMode )
            return FALSE;
        if ( pFrm->IsFlyFrm() )
            pFrm = ((SwFlyFrm*)pFrm)->GetAnchor();
        else
            pFrm = pFrm->GetUpper();
    } while ( pFrm );
    return FALSE;
}

/*************************************************************************
|*
|*  SwFrmFmt::GetGraphic()
|*
|*  Ersterstellung      MA 23. Jul. 96
|*  Letzte Aenderung    MA 23. Jul. 96
|*
|*************************************************************************/

void SetOutDevAndWin( ViewShell *pSh, OutputDevice *pO,
                      Window *pW, USHORT nZoom )
{
    pSh->pOut = pO;
    pSh->pWin = pW;
    pSh->pOpt->SetZoom( nZoom );
}

Graphic SwFrmFmt::MakeGraphic( ImageMap* pMap )
{
    return Graphic();
}

Graphic SwFlyFrmFmt::MakeGraphic( ImageMap* pMap )
{
    Graphic aRet;
    //irgendeinen Fly suchen!
    SwClientIter aIter( *this );
    SwClient *pFirst = aIter.First( TYPE(SwFrm) );
    ViewShell *pSh;
    if ( pFirst && 0 != ( pSh = ((SwFrm*)pFirst)->GetShell()) )
    {
        ViewShell *pOldGlobal = pGlobalShell;
        pGlobalShell = pSh;

        FASTBOOL bNoteURL = pMap &&
            SFX_ITEM_SET != GetAttrSet().GetItemState( RES_URL, TRUE );
        if( bNoteURL )
        {
            ASSERT( !pNoteURL, "MakeGraphic: pNoteURL already used? " );
            pNoteURL = new SwNoteURL;
        }
        SwFlyFrm *pFly = (SwFlyFrm*)pFirst;

        OutputDevice *pOld = pSh->GetOut();
        VirtualDevice aDev( *pOld );
        aDev.EnableOutput( FALSE );

        GDIMetaFile aMet;
        MapMode aMap( pOld->GetMapMode().GetMapUnit() );
        aDev.SetMapMode( aMap );
        aMet.SetPrefMapMode( aMap );

        ::SwCalcPixStatics( pSh->GetOut() );
        aMet.SetPrefSize( pFly->Frm().SSize() );

        aMet.Record( &aDev );
        aDev.SetLineColor();
        aDev.SetFillColor();
        aDev.SetFont( pOld->GetFont() );

        Window *pWin = pSh->GetWin();
        USHORT nZoom = pSh->GetViewOptions()->GetZoom();
        ::SetOutDevAndWin( pSh, &aDev, 0, 100 );
        bFlyMetafile = TRUE;
        pFlyMetafileOut = pWin;

        SwViewImp *pImp = pSh->Imp();
        pFlyOnlyDraw = pFly;
        pLines = new SwLineRects;

        //Rechteck ggf. ausdehnen, damit die Umrandunge mit aufgezeichnet werden.
        SwRect aOut( pFly->Frm() );
        SwBorderAttrAccess aAccess( SwFrm::GetCache(), pFly );
        const SwBorderAttrs &rAttrs = *aAccess.Get();
        if ( rAttrs.CalcRightLine() )
            aOut.SSize().Width() += 2*nPixelSzW;
        if ( rAttrs.CalcBottomLine() )
            aOut.SSize().Height()+= 2*nPixelSzH;

        pImp->PaintLayer( pSh->GetDoc()->GetHellId(), aOut );
        pLines->PaintLines( &aDev );
        if ( pFly->IsFlyInCntFrm() )
            pFly->Paint( aOut );
        pLines->PaintLines( &aDev );
        pImp->PaintLayer( pSh->GetDoc()->GetHeavenId(), aOut );
        pLines->PaintLines( &aDev );
        if( pSh->GetViewOptions()->IsControl() )
        {
            pImp->PaintLayer( pSh->GetDoc()->GetControlsId(), aOut );
            pLines->PaintLines( &aDev );
        }
        DELETEZ( pLines );
        pFlyOnlyDraw = 0;

        pFlyMetafileOut = 0;
        bFlyMetafile = FALSE;
        ::SetOutDevAndWin( pSh, pOld, pWin, nZoom );

        aMet.Stop();
        aMet.Move( -pFly->Frm().Left(), -pFly->Frm().Top() );
        aRet = Graphic( aMet );

        if( bNoteURL )
        {
            ASSERT( pNoteURL, "MakeGraphic: Good Bye, NoteURL." );
            pNoteURL->FillImageMap( pMap, pFly->Frm().Pos(), aMap );
            delete pNoteURL;
            pNoteURL = NULL;
        }
        pGlobalShell = pOldGlobal;
    }
    return aRet;
}

Graphic SwDrawFrmFmt::MakeGraphic( ImageMap* pMap )
{
    Graphic aRet;
    SdrModel *pMod = GetDoc()->GetDrawModel();
    if ( pMod )
    {
        SdrObject *pObj = FindSdrObject();
        SdrView *pView = new SdrView( pMod );
        SdrPageView *pPgView = pView->ShowPagePgNum( 0, Point() );
        pView->MarkObj( pObj, pPgView );
        aRet = pView->GetMarkedObjBitmap();
        pView->HidePage( pPgView );
        delete pView;
    }
    return aRet;
}



