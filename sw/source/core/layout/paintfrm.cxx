/*************************************************************************
 *
 *  $RCSfile: paintfrm.cxx,v $
 *
 *  $Revision: 1.85 $
 *
 *  last change: $Author: kz $ $Date: 2004-08-02 14:12:27 $
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

#define ITEMID_BOXINFO      SID_ATTR_BORDER_INNER
#include <hintids.hxx>

#ifndef _SOUND_HXX //autogen
#include <vcl/sound.hxx>
#endif
#ifndef _TL_POLY_HXX
#include <tools/poly.hxx>
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
// --> collapsing borders FME 2005-05-27 #i29550#
#ifndef SVX_FRAMELINK_HXX
#include <svx/framelink.hxx>
#endif
#include <map>
// <--
#ifndef _GRAPH_HXX //autogen
#include <vcl/graph.hxx>
#endif
#ifndef _SVDPAGV_HXX //autogen
#include <svx/svdpagv.hxx>
#endif
#ifndef SW_TGRDITEM_HXX
#include <tgrditem.hxx>
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
#ifndef _ACCESSIBILITYOPTIONS_HXX
#include <accessibilityoptions.hxx>
#endif
// OD 20.12.2002 #94627#
#ifndef _DOCSH_HXX
#include <docsh.hxx>
#endif
// OD 28.02.2003 #b4779636#, #107692#
#ifndef _SWTABLE_HXX
#include <swtable.hxx>
#endif
// OD 02.07.2003 #108784#
#ifndef _SVDOGRP_HXX
#include <svx/svdogrp.hxx>
#endif
// OD 2004-05-24 #i28701#
#ifndef _SORTEDOBJS_HXX
#include <sortedobjs.hxx>
#endif

#define GETOBJSHELL()       ((SfxObjectShell*)rSh.GetDoc()->GetDocShell())

//Tabellenhilfslinien an?
#define IS_SUBS_TABLE \
    (pGlobalShell->GetViewOptions()->IsTable() && \
    !pGlobalShell->GetViewOptions()->IsPagePreview()&&\
    !pGlobalShell->GetViewOptions()->IsReadonly()&&\
     SwViewOption::IsTableBoundaries())
//sonstige Hilfslinien an?
#define IS_SUBS (!pGlobalShell->GetViewOptions()->IsPagePreview() && \
        !pGlobalShell->GetViewOptions()->IsReadonly() && \
        SwViewOption::IsDocBoundaries())
//Hilfslinien fuer Bereiche
#define IS_SUBS_SECTION (!pGlobalShell->GetViewOptions()->IsPagePreview() && \
                            !pGlobalShell->GetViewOptions()->IsReadonly()&&\
                            SwViewOption::IsSectionBoundaries())
#define IS_SUBS_FLYS (!pGlobalShell->GetViewOptions()->IsPagePreview() && \
                            !pGlobalShell->GetViewOptions()->IsReadonly()&&\
                                SwViewOption::IsObjectBoundaries())

#define SW_MAXBORDERCACHE 20

//Klassendeklarationen. Hier weil sie eben nur in diesem File benoetigt
//werden.

#define SUBCOL_PAGE     0x01    //Helplines of the page
#define SUBCOL_BREAK    0x02    //Helpline for a page or column break
#define SUBCOL_TAB      0x08    //Helplines inside tables
#define SUBCOL_FLY      0x10    //Helplines inside fly frames
#define SUBCOL_SECT     0x20    //Helplines inside sections

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
                const SwTabFrm *pT , const BYTE nSCol );

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
                      const SwTabFrm *pTab, const BYTE nSCol );
    void ConnectEdges( OutputDevice *pOut );
    void PaintLines  ( OutputDevice *pOut );
    void LockLines( BOOL bLock );

    /// OD 13.08.2002 - correct type of function
    const USHORT Free() const { return nFree; }
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
// OD 18.11.2002 #99672# - global variable for sub-lines of body, header, footer,
// section and footnote frames.
static SwSubsRects *pSpecSubsLines = 0;

static SfxProgress *pProgress = 0;

static SwFlyFrm *pFlyOnlyDraw = 0;

//Damit die Flys auch fuer den Hack richtig gepaintet werden koennen.
static FASTBOOL bTableHack = FALSE;

//Um das teure Ermitteln der RetoucheColor zu optimieren
Color aGlobalRetoucheColor;

//Statics fuer Umrandungsalignment setzen.
// OD 05.05.2003 #107169# - adjustment for 'small' twip-to-pixel relations:
// For 'small' twip-to-pixel relations (less then 2:1)
// values of <nHalfPixelSzW> and <nHalfPixelSzH> are set to ZERO.
void SwCalcPixStatics( OutputDevice *pOut )
{
    // OD 30.04.2003 #107169# - determine 'small' twip-to-pixel relation
    sal_Bool bSmallTwipToPxRelW = sal_False;
    sal_Bool bSmallTwipToPxRelH = sal_False;
    {
        Size aCheckTwipToPxRelSz( pOut->PixelToLogic( Size( 100, 100 )) );
        if ( (aCheckTwipToPxRelSz.Width()/100.0) < 2.0 )
        {
            bSmallTwipToPxRelW = sal_True;
        }
        if ( (aCheckTwipToPxRelSz.Height()/100.0) < 2.0 )
        {
            bSmallTwipToPxRelH = sal_True;
        }
    }

    Size aSz( pOut->PixelToLogic( Size( 1,1 )) );

    nPixelSzW = aSz.Width();
    if( !nPixelSzW )
        nPixelSzW = 1;
    nPixelSzH = aSz.Height();
    if( !nPixelSzH )
        nPixelSzH = 1;

    // OD 06.05.2003 #107169# - consider 'small' twip-to-pixel relations
    if ( !bSmallTwipToPxRelW )
    {
        nHalfPixelSzW = nPixelSzW / 2 + 1;
    }
    else
    {
        nHalfPixelSzW = 0;
    }
    // OD 06.05.2003 #107169# - consider 'small' twip-to-pixel relations
    if ( !bSmallTwipToPxRelH )
    {
        nHalfPixelSzH = nPixelSzH / 2 + 1;
    }
    else
    {
        nHalfPixelSzH = 0;
    }

    nMinDistPixelW = nPixelSzW * 2 + 1;
    nMinDistPixelH = nPixelSzH * 2 + 1;

    const MapMode &rMap = pOut->GetMapMode();
    aScaleX = rMap.GetScaleX();
    aScaleY = rMap.GetScaleY();
}

//Zum Sichern der statics, damit das Paint (quasi) reentrant wird.
class SwSavePaintStatics
{
    FASTBOOL            bSFlyMetafile,
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
    bSFlyMetafile       ( bFlyMetafile      ),
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
    bFlyMetafile = FALSE;
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
}

SwSavePaintStatics::~SwSavePaintStatics()
{
    pGlobalShell       = pSGlobalShell;
    bFlyMetafile       = bSFlyMetafile;
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
                 Left()  - nAdd <= rRect.Right() )
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
        {
            nL1a = rL1.Top();   nL1b = rL1.Left();
            nL1c = rL1.Right(); nL1d = rL1.Bottom();
        }
        else
        {
            nL1a = rL1.Left();   nL1b = rL1.Top();
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
            {
                nL2a = rL2.Top();   nL2b = rL2.Left();
                nL2c = rL2.Right(); nL2d = rL2.Bottom();
            }
            else
            {
                nL2a = rL2.Left();   nL2b = rL2.Top();
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
                                        rL1.GetTab(), SUBCOL_TAB ), Count() );
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
                                        rL1.GetTab(), SUBCOL_TAB ), Count() );
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
        // OD 18.11.2002 #99672# - get a copy instead of a reference, because
        // an <insert> may destroy the object due to a necessary array resize.
        const SwLineRect aSubsLineRect = SwLineRect( operator[](i) );

        // OD 19.12.2002 #106318# - add condition <aSubsLineRect.IsLocked()>
        // in order to consider only border lines, which are *not* locked.
        if ( aSubsLineRect.IsPainted() ||
             aSubsLineRect.IsLocked() )
            continue;

        const bool bVerticalSubs = aSubsLineRect.Height() > aSubsLineRect.Width();
        SwRect aSubsRect( aSubsLineRect );
        if ( bVerticalSubs )
        {
            aSubsRect.Left  ( aSubsRect.Left()  - (nPixelSzW+nHalfPixelSzW) );
            aSubsRect.Right ( aSubsRect.Right() + (nPixelSzW+nHalfPixelSzW) );
        }
        else
        {
            aSubsRect.Top   ( aSubsRect.Top()    - (nPixelSzH+nHalfPixelSzH) );
            aSubsRect.Bottom( aSubsRect.Bottom() + (nPixelSzH+nHalfPixelSzH) );
        }
        for ( USHORT k = 0; k < rRects.Count(); ++k )
        {
            SwLineRect &rLine = rRects[k];

            // OD 20.12.2002 #106318# - do *not* consider painted or locked
            // border lines.
            // OD 20.01.2003 #i1837# - locked border lines have to be considered.
            if ( rLine.IsLocked () )
                continue;

            if ( !bVerticalSubs == rLine.Height() > rLine.Width() ) //gleiche Ausrichtung?
                continue;

            if ( aSubsRect.IsOver( rLine ) )
            {
                if ( bVerticalSubs ) //Vertikal?
                {
                    if ( aSubsRect.Left()  <= rLine.Right() &&
                         aSubsRect.Right() >= rLine.Left() )
                    {
                        long nTmp = rLine.Top()-(nPixelSzH+1);
                        if ( aSubsLineRect.Top() < nTmp )
                        {
                            SwRect aNewSubsRect( aSubsLineRect );
                            aNewSubsRect.Bottom( nTmp );
                            Insert( SwLineRect( aNewSubsRect, 0, 0,
                                                aSubsLineRect.GetSubColor() ), Count());
                        }
                        nTmp = rLine.Bottom()+nPixelSzH+1;
                        if ( aSubsLineRect.Bottom() > nTmp )
                        {
                            SwRect aNewSubsRect( aSubsLineRect );
                            aNewSubsRect.Top( nTmp );
                            Insert( SwLineRect( aNewSubsRect, 0, 0,
                                                aSubsLineRect.GetSubColor() ), Count());
                        }
                        Remove( i, 1 );
                        --i;
                        break;
                    }
                }
                else                                    //Horizontal
                {
                    if ( aSubsRect.Top() <= rLine.Bottom() &&
                         aSubsRect.Bottom() >= rLine.Top() )
                    {
                        long nTmp = rLine.Left()-(nPixelSzW+1);
                        if ( aSubsLineRect.Left() < nTmp )
                        {
                            SwRect aNewSubsRect( aSubsLineRect );
                            aNewSubsRect.Right( nTmp );
                            Insert( SwLineRect( aNewSubsRect, 0, 0,
                                                aSubsLineRect.GetSubColor() ), Count());
                        }
                        nTmp = rLine.Right()+nPixelSzW+1;
                        if ( aSubsLineRect.Right() > nTmp )
                        {
                            SwRect aNewSubsRect( aSubsLineRect );
                            aNewSubsRect.Left( nTmp );
                            Insert( SwLineRect( aNewSubsRect, 0, 0,
                                                aSubsLineRect.GetSubColor() ), Count());
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
        // OD 2004-04-23 #116347#
        pOut->Push( PUSH_FILLCOLOR|PUSH_LINECOLOR );
        pOut->SetLineColor();

        ConnectEdges( pOut );
        const Color *pLast = 0;

        FASTBOOL bPaint2nd = FALSE;
        USHORT nMinCount = Count();
        USHORT i;

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

                    ULONG nOldDrawMode = pOut->GetDrawMode();
                    if( pGlobalShell->GetWin() &&
                        Application::GetSettings().GetStyleSettings().GetHighContrastMode() )
                        pOut->SetDrawMode( 0 );

                    pOut->SetFillColor( *pLast );
                    pOut->SetDrawMode( nOldDrawMode );
                }
                if( !rLRect.IsEmpty() )
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

                    ULONG nOldDrawMode = pOut->GetDrawMode();
                    if( pGlobalShell->GetWin() &&
                        Application::GetSettings().GetStyleSettings().GetHighContrastMode() )
                    {
                        pOut->SetDrawMode( 0 );
                    }

                    pOut->SetFillColor( *pLast );
                    pOut->SetDrawMode( nOldDrawMode );
                }
                if( !rLRect.IsEmpty() )
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
            const bool bVerticalSubs = rLi.Height() > rLi.Width();

            for ( USHORT k = i+1; k < Count(); ++k )
            {
                SwLineRect &rLk = operator[](k);
                if ( rLi.SSize() == rLk.SSize() )
                {
                    if ( bVerticalSubs == rLk.Height() > rLk.Width() )
                    {
                        if ( bVerticalSubs )
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
            // OD 2004-04-23 #116347#
            pOut->Push( PUSH_FILLCOLOR|PUSH_LINECOLOR );
            pOut->SetLineColor();

            // OD 14.01.2003 #106660# - reset draw mode in high contrast
            // mode in order to get fill color set at output device.
            // Recover draw mode after draw of lines.
            // Necessary for the subsidiary lines painted by the fly frames.
            ULONG nOldDrawMode = pOut->GetDrawMode();
            if( pGlobalShell->GetWin() &&
                Application::GetSettings().GetStyleSettings().GetHighContrastMode() )
            {
                pOut->SetDrawMode( 0 );
            }

            for ( USHORT i = 0; i < Count(); ++i )
            {
                SwLineRect &rLRect = operator[](i);
                // OD 19.12.2002 #106318# - add condition <!rLRect.IsLocked()>
                // to prevent paint of locked subsidiary lines.
                if ( !rLRect.IsPainted() &&
                     !rLRect.IsLocked() )
                {
                    const Color *pCol;
                    switch ( rLRect.GetSubColor() )
                    {
                        case SUBCOL_PAGE: pCol = &SwViewOption::GetDocBoundariesColor(); break;
                        case SUBCOL_FLY: pCol = &SwViewOption::GetObjectBoundariesColor(); break;
                        case SUBCOL_TAB: pCol = &SwViewOption::GetTableBoundariesColor(); break;
                        case SUBCOL_SECT: pCol = &SwViewOption::GetSectionBoundColor(); break;
                        case SUBCOL_BREAK:    pCol = &SwViewOption::GetPageBreakColor(); break;
                    }

                    if ( pOut->GetFillColor() != *pCol )
                        pOut->SetFillColor( *pCol );
                    pOut->DrawRect( rLRect.SVRect() );

                    rLRect.SetPainted();
                }
            }

            // OD 14.01.2003 #106660# - recovering draw mode
            pOut->SetDrawMode( nOldDrawMode );

            pOut->Pop();
        }
    }
}

//-------------------------------------------------------------------------
//Diverse Functions die in diesem File so verwendet werden.

// OD 20.02.2003 - Note: function <SwAlignRect(..)> also used outside this file.
// OD 29.04.2003 #107169# - correction: adjust rectangle on pixel level in order
//          to assure, that the border 'leaves its original pixel', if it has to.
//          No prior adjustments for odd relation between pixel and twip.
void MA_FASTCALL SwAlignRect( SwRect &rRect, ViewShell *pSh )
{
    if( !rRect.HasArea() )
        return;

    // OD 03.09.2002 #102450#
    // Assure that view shell (parameter <pSh>) exists, if the output device
    // is taken from this view shell --> no output device, no alignment.
    // Output device taken from view shell <pSh>, if <bFlyMetafile> not set.
    if ( !bFlyMetafile && !pSh )
    {
        return;
    }

    const OutputDevice *pOut = bFlyMetafile ?
                        pFlyMetafileOut : pSh->GetOut();

    // OD 28.04.2003 #107169# - hold original rectangle in pixel
    const Rectangle aOrgPxRect = pOut->LogicToPixel( rRect.SVRect() );
    // OD 29.04.2003 #107169# - determine pixel-center rectangle in twip
    const SwRect aPxCenterRect( pOut->PixelToLogic( aOrgPxRect ) );

    // OD 06.05.2003 #107169# - perform adjustments on pixel level.
    SwRect aAlignedPxRect( aOrgPxRect );
    if ( rRect.Top() > aPxCenterRect.Top() )
    {
        // 'leave pixel overlapping on top'
        aAlignedPxRect.Top( aAlignedPxRect.Top() + 1 );
    }

    if ( rRect.Bottom() < aPxCenterRect.Bottom() )
    {
        // 'leave pixel overlapping on bottom'
        aAlignedPxRect.Bottom( aAlignedPxRect.Bottom() - 1 );
    }

    if ( rRect.Left() > aPxCenterRect.Left() )
    {
        // 'leave pixel overlapping on left'
        aAlignedPxRect.Left( aAlignedPxRect.Left() + 1 );
    }

    if ( rRect.Right() < aPxCenterRect.Right() )
    {
        // 'leave pixel overlapping on right'
        aAlignedPxRect.Right( aAlignedPxRect.Right() - 1 );
    }

    // OD 11.10.2002 #103636# - consider negative width/height
    // check, if aligned SwRect has negative width/height.
    // If Yes, adjust it to width/height = 0 twip.
    // NOTE: A SwRect with negative width/height can occur, if the width/height
    //     of the given SwRect in twip was less than a pixel in twip and that
    //     the alignment calculates that the aligned SwRect should not contain
    //     the pixels the width/height is on.
    if ( aAlignedPxRect.Width() < 0 )
    {
        aAlignedPxRect.Width(0);
    }
    if ( aAlignedPxRect.Height() < 0 )
    {
        aAlignedPxRect.Height(0);
    }
    // OD 30.04.2003 #107169# - consider zero width/height
    // For converting a rectangle from pixel to logic it needs a width/height.
    // Thus, set width/height to one, if it's zero and correct this on the twip
    // level after the conversion.
    sal_Bool bZeroWidth = sal_False;
    if ( aAlignedPxRect.Width() == 0 )
    {
        aAlignedPxRect.Width(1);
        bZeroWidth = sal_True;
    }
    sal_Bool bZeroHeight = sal_False;
    if ( aAlignedPxRect.Height() == 0 )
    {
        aAlignedPxRect.Height(1);
        bZeroHeight = sal_True;
    }

    rRect = pOut->PixelToLogic( aAlignedPxRect.SVRect() );

    // OD 30.04.2003 #107169# - consider zero width/height and adjust calculated
    // aligned twip rectangle.
    // OD 19.05.2003 #109667# - reset width/height to zero; previous negative
    // width/height haven't to be considered.
    if ( bZeroWidth )
    {
        rRect.Width(0);
    }
    if ( bZeroHeight )
    {
        rRect.Height(0);
    }
}

/** OD 19.05.2003 #109667# - helper method for twip adjustments on pixel base

    method compares the x- or y-pixel position of two twip-point. If the x-/y-pixel
    positions are the same, the x-/y-pixel position of the second twip point is
    adjusted by a given amount of pixels.

    @author OD
*/
void lcl_CompPxPosAndAdjustPos( const OutputDevice&  _rOut,
                                const Point&         _rRefPt,
                                Point&               _rCompPt,
                                const sal_Bool       _bChkXPos,
                                const sal_Int8       _nPxAdjustment )
{
    const Point aRefPxPt = _rOut.LogicToPixel( _rRefPt );
    Point aCompPxPt = _rOut.LogicToPixel( _rCompPt );

    if ( _bChkXPos )
    {
        if ( aCompPxPt.X() == aRefPxPt.X() )
        {
            aCompPxPt.X() += _nPxAdjustment ;
            const Point aAdjustedCompPt = _rOut.PixelToLogic( aCompPxPt );
            _rCompPt.X() = aAdjustedCompPt.X();
        }
    }
    else
    {
        if ( aCompPxPt.Y() == aRefPxPt.Y() )
        {
            aCompPxPt.Y() += _nPxAdjustment ;
            const Point aAdjustedCompPt = _rOut.PixelToLogic( aCompPxPt );
            _rCompPt.Y() = aAdjustedCompPt.Y();
        }
    }
}

/** OD 25.09.2002 #99739# - method to pixel-align rectangle for drawing graphic object

    Because for drawing a graphic left-top-corner and size coordinations are
    used, these coordinations have to be determined on pixel level.
    Thus, convert rectangle to pixel and then convert left-top-corner and
    size of pixel rectangle back to logic.
    This calculation is necessary, because there exists a different between
    the convert from logic to pixel of a normal rectangle with its left-top-
    and right-bottom-corner and the same convert of the same rectangle
    with left-top-corner and size.
    Call this method before each <GraphicObject.Draw(...)>

    @author OD
*/
void SwAlignGrfRect( SwRect *pGrfRect, const OutputDevice &rOut )
{
    Rectangle aPxRect = rOut.LogicToPixel( pGrfRect->SVRect() );
    pGrfRect->Pos( rOut.PixelToLogic( aPxRect.TopLeft() ) );
    pGrfRect->SSize( rOut.PixelToLogic( aPxRect.GetSize() ) );
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
    // OD 23.01.2003 #106386# - special handling for cell frames.
    // The printing area of a cell frame is completely enclosed in the frame area
    // and a cell frame has no shadow. Thus, for cell frames the calculated
    // area equals the frame area.
    // Notes: Borders of cell frames in R2L text direction will switch its side
    //        - left border is painted on the right; right border on the left.
    //        See <lcl_PaintLeftLine> and <lcl_PaintRightLine>.
    if( pFrm->IsSctFrm() )
    {
        rRect = pFrm->Prt();
        rRect.Pos() += pFrm->Frm().Pos();
    }
    else if ( pFrm->IsCellFrm() )
        rRect = pFrm->Frm();
    else
    {
        rRect = pFrm->Prt();
        rRect.Pos() += pFrm->Frm().Pos();

        if ( rAttrs.IsLine() || rAttrs.IsBorderDist() ||
             (bShadow && rAttrs.GetShadow().GetLocation() != SVX_SHADOW_NONE) )
        {
            SwRectFn fnRect = pFrm->IsVertical() ? fnRectVert : fnRectHori;
            const SvxBoxItem &rBox = rAttrs.GetBox();
            const FASTBOOL bTop = 0 != (pFrm->*fnRect->fnGetTopMargin)();
            if ( bTop )
            {
                SwTwips nDiff = rBox.GetTop() ?
                    rBox.CalcLineSpace( BOX_LINE_TOP ) :
                    ( rAttrs.IsBorderDist() ?
                      // OD 23.01.2003 #106386# - increase of distance by
                      // one twip is incorrect.
                      rBox.GetDistance( BOX_LINE_TOP ) : 0 );
                if( nDiff )
                    (rRect.*fnRect->fnSubTop)( nDiff );
            }

            const FASTBOOL bBottom = 0 != (pFrm->*fnRect->fnGetBottomMargin)();
            if ( bBottom )
            {
                SwTwips nDiff = 0;
                // --> collapsing borders FME 2005-05-27 #i29550#
                if ( pFrm->IsTabFrm() &&
                     ((SwTabFrm*)pFrm)->IsCollapsingBorders() )
                {
                    // For collapsing borders, we have to add the height of
                    // the height of the last line
                    nDiff = ((SwTabFrm*)pFrm)->GetBottomLineSize();
                }
                // <-- collapsing
                else
                {
                    nDiff = rBox.GetBottom() ?
                    rBox.CalcLineSpace( BOX_LINE_BOTTOM ) :
                    ( rAttrs.IsBorderDist() ?
                      // OD 23.01.2003 #106386# - increase of distance by
                      // one twip is incorrect.
                      rBox.GetDistance( BOX_LINE_BOTTOM ) : 0 );
                }
                if( nDiff )
                    (rRect.*fnRect->fnAddBottom)( nDiff );
            }

            if ( rBox.GetLeft() )
                (rRect.*fnRect->fnSubLeft)( rBox.CalcLineSpace( BOX_LINE_LEFT ) );
            else if ( rAttrs.IsBorderDist() )
                 // OD 23.01.2003 #106386# - increase of distance by one twip is incorrect.
                (rRect.*fnRect->fnSubLeft)( rBox.GetDistance( BOX_LINE_LEFT ) );

            if ( rBox.GetRight() )
                (rRect.*fnRect->fnAddRight)( rBox.CalcLineSpace( BOX_LINE_RIGHT ) );
            else if ( rAttrs.IsBorderDist() )
                 // OD 23.01.2003 #106386# - increase of distance by one twip is incorrect.
                (rRect.*fnRect->fnAddRight)( rBox.GetDistance( BOX_LINE_RIGHT ) );

            if ( bShadow && rAttrs.GetShadow().GetLocation() != SVX_SHADOW_NONE )
            {
                const SvxShadowItem &rShadow = rAttrs.GetShadow();
                if ( bTop )
                    (rRect.*fnRect->fnSubTop)(rShadow.CalcShadowSpace(SHADOW_TOP));
                (rRect.*fnRect->fnSubLeft)(rShadow.CalcShadowSpace(SHADOW_LEFT));
                if ( bBottom )
                    (rRect.*fnRect->fnAddBottom)
                                    (rShadow.CalcShadowSpace( SHADOW_BOTTOM ));
                (rRect.*fnRect->fnAddRight)(rShadow.CalcShadowSpace(SHADOW_RIGHT));
            }
        }
    }

    ::SwAlignRect( rRect, pGlobalShell );
}

void MA_FASTCALL lcl_ExtendLeftAndRight( SwRect&                _rRect,
                                         const SwFrm&           _rFrm,
                                         const SwBorderAttrs&   _rAttrs,
                                         const SwRectFn&        _rRectFn )
{
    // OD 21.05.2003 #108789# - extend left/right border/shadow rectangle to
    // bottom of previous frame/to top of next frame, if border/shadow is joined
    // with previous/next frame.
    if ( _rAttrs.JoinedWithPrev( _rFrm ) )
    {
        const SwFrm* pPrevFrm = _rFrm.GetPrev();
        (_rRect.*_rRectFn->fnSetTop)( (pPrevFrm->*_rRectFn->fnGetPrtBottom)() );
    }
    if ( _rAttrs.JoinedWithNext( _rFrm ) )
    {
        const SwFrm* pNextFrm = _rFrm.GetNext();
        (_rRect.*_rRectFn->fnSetBottom)( (pNextFrm->*_rRectFn->fnGetPrtTop)() );
    }
}


void MA_FASTCALL lcl_SubtractFlys( const SwFrm *pFrm, const SwPageFrm *pPage,
                           const SwRect &rRect, SwRegionRects &rRegion )
{
    const SwSortedObjs& rObjs = *pPage->GetSortedObjs();
    const SwFlyFrm* pSelfFly = pFrm->IsInFly() ? pFrm->FindFlyFrm() : pRetoucheFly2;
    if ( !pRetoucheFly )
        pRetoucheFly = pRetoucheFly2;

    for ( USHORT j = 0; (j < rObjs.Count()) && rRegion.Count(); ++j )
    {
        const SwAnchoredObject* pAnchoredObj = rObjs[j];
        const SdrObject* pSdrObj = pAnchoredObj->GetDrawObj();

        // OD 2004-01-15 #110582# - do not consider invisible objects
        if ( !pPage->GetFmt()->GetDoc()->IsVisibleLayerId( pSdrObj->GetLayer() ) )
            continue;

        if ( !pAnchoredObj->ISA(SwFlyFrm) )
            continue;

        const SwFlyFrm *pFly = static_cast<const SwFlyFrm*>(pAnchoredObj);

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
                    pSdrObj->GetOrdNumDirect() > pSelfFly->GetVirtDrawObj()->GetOrdNumDirect(),
                    "Fly with wrong z-Order" );
        }
#endif

        BOOL bStopOnHell = TRUE;
        if ( pSelfFly )
        {
            const SdrObject *pTmp = pSelfFly->GetVirtDrawObj();
            if ( pSdrObj->GetLayer() == pTmp->GetLayer() )
            {
                if ( pSdrObj->GetOrdNumDirect() < pTmp->GetOrdNumDirect() )
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
            if ( pSdrObj->GetLayer() == pTmp->GetLayer() )
            {
                if ( pSdrObj->GetOrdNumDirect() < pTmp->GetOrdNumDirect() )
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
        BOOL bHell = pSdrObj->GetLayer() == pFly->GetFmt()->GetDoc()->GetHellId();
        if ( (bStopOnHell && bHell) ||
             /// OD 05.08.2002 - change internal order of condition
             ///    first check "!bHell", then "..->Lower()" and "..->IsNoTxtFrm()"
             ///    have not to be performed, if frame is in "Hell"
             ( !bHell && pFly->Lower() && pFly->Lower()->IsNoTxtFrm() &&
               ( ((SwNoTxtFrm*)pFly->Lower())->IsTransparent() ||
                 ((SwNoTxtFrm*)pFly->Lower())->HasAnimation() ||
                 pFly->GetFmt()->GetSurround().IsContour()
               )
             )
           )
            continue;

        // OD 08.10.2002 #103898#
        // Own if-statements for transparent background/shadow of fly frames
        // (#99657#) in order to handle special conditions.
        if ( pFly->IsBackgroundTransparent() )
        {
            // Background <pFly> is transparent drawn. Thus normally, its region
            // have not to be substracted from given region.
            // But, if method is called for a fly frame and
            // <pFly> is a direct lower of this fly frame and
            // <pFly> inherites its transparent background brush from its parent,
            // then <pFly> frame area have to be subtracted from given region.
            // NOTE: Because in Status Quo transparent backgrounds can only be
            //     assigned to fly frames, the handle of this special case
            //     avoids drawing of transparent areas more than once, if
            //     a fly frame inherites a transparent background from its
            //     parent fly frame.
            if ( pFrm->IsFlyFrm() &&
                 (pFly->GetAnchorFrm()->FindFlyFrm() == pFrm) &&
                 static_cast<const SwFlyFrmFmt*>(pFly->GetFmt())->IsBackgroundBrushInherited()
               )
            {
                SwRect aRect;
                SwBorderAttrAccess aAccess( SwFrm::GetCache(), (SwFrm*)pFly );
                const SwBorderAttrs &rAttrs = *aAccess.Get();
                ::lcl_CalcBorderRect( aRect, pFly, rAttrs, TRUE );
                rRegion -= aRect;
                continue;
            }
            else
            {
                continue;
            }
        }
        if ( pFly->IsShadowTransparent() )
        {
            continue;
        }

        if ( bHell && pFly->GetAnchorFrm()->IsInFly() )
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

/** lcl_DrawGraphicBackgrd - local help method to draw a background for a graphic

    OD 17.10.2002 #103876#
    Under certain circumstances we have to draw a background for a graphic.
    This method takes care of the conditions and draws the background with the
    corresponding color.
    Method introduced for bug fix #103876# in order to optimize drawing tiled
    background graphics. Previously, this code was integrated in method
    <lcl_DrawGraphic>.
    Method implemented as a inline, checking the conditions and calling method
    method <lcl_implDrawGraphicBackgrd(..)> for the intrinsic drawing.

    @author OD

    @param _rBackgrdBrush
    background brush contain the color the background has to be drawn.

    @param _pOut
    output device the background has to be drawn in.

    @param _rPaintRect
    paint retangle in the output device, which has to be drawn with the background.
    rectangle have to be aligned by method ::SwAlignRect

    @param _rGraphicObj
    graphic object, for which the background has to be drawn. Used for checking
    the transparency of its bitmap, its type and if the graphic is drawn transparent

    @param _bNumberingGraphic
    boolean indicating that graphic is used as a numbering.

    @param _bBackgrdAlreadyDrawn
    boolean (optional; default: false) indicating, if the background is already drawn.
*/
void lcl_implDrawGraphicBackgrd( const SvxBrushItem& _rBackgrdBrush,
                                 OutputDevice* _pOut,
                                 const SwRect& _rAlignedPaintRect,
                                 const GraphicObject& _rGraphicObj )
{
    /// determine color of background
    ///     If color of background brush is not "no fill"/"auto fill" or
    ///     <bFlyMetafile> is set, use color of background brush, otherwise
    ///     use global retouche color.
    const Color aColor( ( (_rBackgrdBrush.GetColor() != COL_TRANSPARENT) || bFlyMetafile )
                        ? _rBackgrdBrush.GetColor()
                        : aGlobalRetoucheColor );

    /// determine, if background color have to be drawn transparent
    /// and calculate transparency percent value
    sal_Int8 nTransparencyPercent = 0;
    bool bDrawTransparent = false;
    if ( aColor.GetTransparency() != 0 )
    ///     background color is transparent --> draw transparent.
    {
        bDrawTransparent = true;
        nTransparencyPercent = (aColor.GetTransparency()*100 + 0x7F)/0xFF;
    }
    else if ( (_rGraphicObj.GetAttr().GetTransparency() != 0) &&
                (_rBackgrdBrush.GetColor() == COL_TRANSPARENT) )
    ///     graphic is drawn transparent and background color is
    ///     "no fill"/"auto fill" --> draw transparent
    {
        bDrawTransparent = true;
        nTransparencyPercent = (_rGraphicObj.GetAttr().GetTransparency()*100 + 0x7F)/0xFF;
    }

    if ( bDrawTransparent )
    {
        /// draw background transparent
        if( _pOut->GetFillColor() != aColor.GetRGBColor() )
            _pOut->SetFillColor( aColor.GetRGBColor() );
        PolyPolygon aPoly( _rAlignedPaintRect.SVRect() );
        _pOut->DrawTransparent( aPoly, nTransparencyPercent );
    }
    else
    {
        /// draw background opaque
        if ( _pOut->GetFillColor() != aColor )
            _pOut->SetFillColor( aColor );
        _pOut->DrawRect( _rAlignedPaintRect.SVRect() );
    }
}

inline void lcl_DrawGraphicBackgrd( const SvxBrushItem& _rBackgrdBrush,
                                    OutputDevice* _pOut,
                                    const SwRect& _rAlignedPaintRect,
                                    const GraphicObject& _rGraphicObj,
                                    bool _bNumberingGraphic,
                                    bool _bBackgrdAlreadyDrawn = false )
{
    /// draw background with background color, if
    ///     (1) graphic is not used as a numbering AND
    ///     (2) background is not already drawn AND
    ///     (3) intrinsic graphic is transparent OR intrinsic graphic doesn't exists
    if ( !_bNumberingGraphic &&
         !_bBackgrdAlreadyDrawn &&
         ( _rGraphicObj.IsTransparent() || _rGraphicObj.GetType() == GRAPHIC_NONE  )
       )
    {
        lcl_implDrawGraphicBackgrd( _rBackgrdBrush, _pOut, _rAlignedPaintRect, _rGraphicObj );
    }
}

/// OD 06.08.2002 #99657# - Note: the transparency of the background graphic
///     is saved in SvxBrushItem.GetGraphicObject(<shell>).GetAttr().Set/GetTransparency()
///     and is considered in the drawing of the graphic.
///     Thus, to provide transparent background graphic for text frames nothing
///     has to be coded.
/// OD 25.09.2002 #99739# - use align rectangle for drawing graphic
/// OD 25.09.2002 #99739# - pixel-align coordinations for drawing graphic.
/// OD 17.10.2002 #103876# - outsource code for drawing background of the graphic
///     with a background color in method <lcl_DrawGraphicBackgrd>
///     Also, change type of <bGrfNum> and <bClip> from <BOOL> to <bool>.
void lcl_DrawGraphic( const SvxBrushItem& rBrush, OutputDevice *pOut,
                      ViewShell &rSh, const SwRect &rGrf, const SwRect &rOut,
                      bool bClip, bool bGrfNum,
                      bool bBackgrdAlreadyDrawn = false )
                      /// OD 02.09.2002 #99657#
                      /// add parameter <bBackgrdAlreadyDrawn> to indicate
                      /// that the background is already drawn.
{
    /// OD 25.09.2002 #99739# - calculate align rectangle from parameter <rGrf>
    ///     and use aligned rectangle <aAlignedGrfRect> in the following code
    SwRect aAlignedGrfRect = rGrf;
    ::SwAlignRect( aAlignedGrfRect, &rSh );

    /// OD 17.10.2002 #103876# - change type from <FASTBOOL> to <bool>.
    const bool bNotInside = bClip && !rOut.IsInside( aAlignedGrfRect );
    if ( bNotInside )
    {
        pOut->Push( PUSH_CLIPREGION );
        pOut->IntersectClipRegion( rOut.SVRect() );
    }

    //Hier kein Link, wir wollen die Grafik synchron laden!
    ((SvxBrushItem&)rBrush).SetDoneLink( Link() );
    GraphicObject *pGrf = (GraphicObject*)rBrush.GetGraphicObject(
                                                    GETOBJSHELL() );

    /// OD 17.10.2002 #103876# - outsourcing drawing of background with a background color.
    ::lcl_DrawGraphicBackgrd( rBrush, pOut, aAlignedGrfRect, *pGrf, bGrfNum, bBackgrdAlreadyDrawn );

    /// OD 25.09.2002 #99739# -
    /// Because for drawing a graphic left-top-corner and size coordinations are
    /// used, these coordinations have to be determined on pixel level.
    ::SwAlignGrfRect( &aAlignedGrfRect, *pOut );
    pGrf->Draw( pOut, aAlignedGrfRect.Pos(), aAlignedGrfRect.SSize() );

    if ( bNotInside )
        pOut->Pop();
} // end of method <lcl_DrawGraphic>

void MA_FASTCALL DrawGraphic( const SvxBrushItem *pBrush,
                              OutputDevice *pOutDev,
                              const SwRect &rOrg,
                              const SwRect &rOut,
                              const BYTE nGrfNum,
                              const sal_Bool bConsiderBackgroundTransparency )
    /// OD 05.08.2002 #99657# - add 6th parameter to indicate that method should
    ///   consider background transparency, saved in the color of the brush item
{
    ViewShell &rSh = *pGlobalShell;
    /// OD 17.10.2002 #103876# - change type from <BOOL> to <bool>
    bool bReplaceGrfNum = GRFNUM_REPLACE == nGrfNum;
    bool bGrfNum = GRFNUM_NO != nGrfNum;
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
                    // don't the use the specific output device! Bug 94802
                    aGrfSize = ::GetGraphicSizeTwip( *pGrf, 0 );
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
        /// OD 05.09.2002 #102912#
        /// In spite the fact that the background graphic have to fill the complete
        /// area, it has been checked, if the graphic will completely fill out
        /// the region to be painted <rOut> and thus, nothing has to be retouched.
        /// For example, this is the case for a fly frame without a background
        /// brush positioned on the border of the page and inherited the
        /// background brush from the page.
        bRetouche = !rOut.IsInside( aGrf );
        break;

    case GPOS_TILED:
        {
            // OD 17.10.2002 #103876# - draw background of tiled graphic
            // before drawing tiled graphic in loop
            // determine graphic object
            GraphicObject* pGraphicObj = const_cast< GraphicObject* >(pBrush->GetGraphicObject( GETOBJSHELL() ));
            // calculate aligned paint rectangle
            SwRect aAlignedPaintRect = rOut;
            ::SwAlignRect( aAlignedPaintRect, &rSh );
            // OD 25.10.2002 #103876# - draw background color for aligned paint rectangle
            lcl_DrawGraphicBackgrd( *pBrush, pOutDev, aAlignedPaintRect, *pGraphicObj, bGrfNum );

            // set left-top-corner of background graphic to left-top-corner of the
            // area, from which the background brush is determined.
            aGrf.Pos() = rOrg.Pos();
            // setup clipping at output device
            pOutDev->Push( PUSH_CLIPREGION );
            pOutDev->IntersectClipRegion( rOut.SVRect() );
            // OD 28.10.2002 #103876# - use new method <GraphicObject::DrawTiled(::)>
            {
                // calculate paint offset
                Point aPaintOffset( aAlignedPaintRect.Pos() - aGrf.Pos() );
                // draw background graphic tiled for aligned paint rectangle
                pGraphicObj->DrawTiled( pOutDev,
                                        aAlignedPaintRect.SVRect(),
                                        aGrf.SSize(),
                                        Size( aPaintOffset.X(), aPaintOffset.Y() ) );
            }
            // reset clipping at output device
            pOutDev->Pop();
            // set <bDraw> and <bRetouche> to false, indicating that background
            // graphic and background are already drawn.
            bDraw = bRetouche = FALSE;
        }
        break;

    case GPOS_NONE:
        bDraw = FALSE;
        break;

    default: ASSERT( !pOutDev, "new Graphic position?" );
    }

    /// OD 02.09.2002 #99657#
    /// init variable <bGrfBackgrdAlreadDrawn> to indicate, if background of
    /// graphic is already drawn or not.
    bool bGrfBackgrdAlreadyDrawn = false;
    if ( bRetouche )
    {
        // OD 2004-04-23 #116347#
        pOutDev->Push( PUSH_FILLCOLOR|PUSH_LINECOLOR );
        pOutDev->SetLineColor();

        // OD 07.08.2002 #99657# #GetTransChg#
        //     check, if a existing background graphic (not filling the complete
        //     background) is transparent drawn and the background color is
        //     "no fill" respectively "auto fill", if background transparency
        //     has to be considered.
        //     If YES, memorise transparency of background graphic.
        //     check also, if background graphic bitmap is transparent.
        bool bTransparentGrfWithNoFillBackgrd = false;
        sal_Int32 nGrfTransparency = 0;
        bool bGrfIsTransparent = false;
        if ( (ePos != GPOS_NONE) &&
             (ePos != GPOS_TILED) && (ePos != GPOS_AREA)
           )
        {
            GraphicObject *pGrf = (GraphicObject*)pBrush->GetGraphicObject(
                                                    GETOBJSHELL() );
            if ( bConsiderBackgroundTransparency )
            {
                GraphicAttr pGrfAttr = pGrf->GetAttr();
                if ( (pGrfAttr.GetTransparency() != 0) &&
                     ( pBrush && (pBrush->GetColor() == COL_TRANSPARENT) )
                   )
                {
                    bTransparentGrfWithNoFillBackgrd = true;
                    nGrfTransparency = pGrfAttr.GetTransparency();
                }
            }
            if ( pGrf->IsTransparent() )
            {
                bGrfIsTransparent = true;
            }
        }

        /// OD 06.08.2002 #99657# #GetTransChg# - to get color of brush,
        ///     check background color against COL_TRANSPARENT ("no fill"/"auto fill")
        ///     instead of checking, if transparency is not set.
        const Color aColor( pBrush &&
                            ( !(pBrush->GetColor() == COL_TRANSPARENT) ||
                              bFlyMetafile )
                    ? pBrush->GetColor()
                    : aGlobalRetoucheColor );

        /// OD 08.08.2002 #99657# - determine, if background region have to be
        ///     drawn transparent.
        ///     background region has to be drawn transparent, if
        ///         background transparency have to be considered
        ///     AND
        ///       ( background color is transparent OR
        ///         background graphic is transparent and background color is "no fill"
        ///       )
        sal_Bool bDrawTransparent = bConsiderBackgroundTransparency &&
                                ( ( aColor.GetTransparency() != 0) ||
                                    bTransparentGrfWithNoFillBackgrd );

        /// OD 06.08.2002 #99657# - if background region have to be drawn
        ///     transparent, set only the RGB values of the background color as
        ///     the fill color for the output device.
        if ( bDrawTransparent )
        {
            if( pOutDev->GetFillColor() != aColor.GetRGBColor() )
                pOutDev->SetFillColor( aColor.GetRGBColor() );
        }
        else
        {
            if( pOutDev->GetFillColor() != aColor )
                pOutDev->SetFillColor( aColor );
        }

        /// OD 02.09.2002 #99657#
        if ( bDrawTransparent )
        {
            /// background region have to be drawn transparent.
            /// Thus, create a poly-polygon from the region and draw it with
            /// the corresponding transparency precent.
            PolyPolygon aDrawPoly( rOut.SVRect() );
            if ( aGrf.HasArea() )
            {
                if ( !bGrfIsTransparent )
                {
                    /// substract area of background graphic from draw area
                    /// OD 08.10.2002 #103898# - consider only that part of the
                    ///     graphic area that is overlapping with draw area.
                    SwRect aTmpGrf = aGrf;
                    aTmpGrf.Intersection( rOut );
                    if ( aTmpGrf.HasArea() )
                    {
                        Polygon aGrfPoly( aTmpGrf.SVRect() );
                        aDrawPoly.Insert( aGrfPoly );
                    }
                }
                else
                    bGrfBackgrdAlreadyDrawn = true;
            }
            /// calculate transparency percent:
            /// ( <transparency value[0x01..0xFF]>*100 + 0x7F ) / 0xFF
            /// If there is a background graphic with a background color "no fill"/"auto fill",
            /// the transparency value is taken from the background graphic,
            /// otherwise take the transparency value from the color.
            sal_Int8 nTransparencyPercent =
              (( bTransparentGrfWithNoFillBackgrd ? nGrfTransparency : aColor.GetTransparency()
               )*100 + 0x7F)/0xFF;
            /// draw poly-polygon transparent
            pOutDev->DrawTransparent( aDrawPoly, nTransparencyPercent );
        }
        else
        {
            SwRegionRects aRegion( rOut, 4 );
            if ( !bGrfIsTransparent )
                aRegion -= aGrf;
            else
                bGrfBackgrdAlreadyDrawn = true;
            /// loop rectangles of background region, which has to be drawn
            for( USHORT i = 0; i < aRegion.Count(); ++i )
            {
                pOutDev->DrawRect( aRegion[i].SVRect() );
            }
        }
       pOutDev ->Pop();
    }

    if( bDraw && aGrf.IsOver( rOut ) )
        /// OD 02.09.2002 #99657#
        /// add parameter <bGrfBackgrdAlreadyDrawn>
        lcl_DrawGraphic( *pBrush, pOutDev, rSh, aGrf, rOut, true, bGrfNum,
                         bGrfBackgrdAlreadyDrawn );

    if( bReplaceGrfNum )
    {
        const Bitmap& rBmp = SwNoTxtFrm::GetBitmap( FALSE );
        Font aTmp( pOutDev->GetFont() );
        ((Graphic*)0)->Draw( pOutDev, aEmptyStr, aTmp, rBmp,
                             rOrg.Pos(), rOrg.SSize() );
    }
}

//------------------------------------------------------------------------

/** local help method for SwRootFrm::Paint(..) - Adjust given rectangle to pixel size

    By OD at 27.09.2002 for #103636#
    In order to avoid paint errors caused by multiple alignments - e.g. method
    ::SwAlignRect(..) - and other changes to the rectangle to be painted,
    this method is called for the rectangle to be painted in order to
    adjust it to the pixel it is overlapping.

    @author OD
*/
void lcl_AdjustRectToPixelSize( SwRect& io_aSwRect, const OutputDevice &aOut )
{
    /// local constant object of class <Size> to determine number of Twips
    /// representing a pixel.
    const Size aTwipToPxSize( aOut.PixelToLogic( Size( 1,1 )) );

    /// local object of class <Rectangle> in Twip coordinates
    /// calculated from given rectangle aligned to pixel centers.
    const Rectangle aPxCenterRect = aOut.PixelToLogic(
            aOut.LogicToPixel( io_aSwRect.SVRect() ) );

    /// local constant object of class <Rectangle> representing given rectangle
    /// in pixel.
    const Rectangle aOrgPxRect = aOut.LogicToPixel( io_aSwRect.SVRect() );

    /// calculate adjusted rectangle from pixel centered rectangle.
    /// Due to rounding differences <aPxCenterRect> doesn't exactly represents
    /// the Twip-centers. Thus, adjust borders by half of pixel width/height plus 1.
    /// Afterwards, adjust calculated Twip-positions of the all borders.
    Rectangle aSizedRect = aPxCenterRect;
    aSizedRect.Left() -= (aTwipToPxSize.Width()/2 + 1);
    aSizedRect.Right() += (aTwipToPxSize.Width()/2 + 1);
    aSizedRect.Top() -= (aTwipToPxSize.Height()/2 + 1);
    aSizedRect.Bottom() += (aTwipToPxSize.Height()/2 + 1);

    /// adjust left()
    while ( (aOut.LogicToPixel(aSizedRect)).Left() < aOrgPxRect.Left() )
    {
        ++aSizedRect.Left();
    }
    /// adjust right()
    while ( (aOut.LogicToPixel(aSizedRect)).Right() > aOrgPxRect.Right() )
    {
        --aSizedRect.Right();
    }
    /// adjust top()
    while ( (aOut.LogicToPixel(aSizedRect)).Top() < aOrgPxRect.Top() )
    {
        ++aSizedRect.Top();
    }
    /// adjust bottom()
    while ( (aOut.LogicToPixel(aSizedRect)).Bottom() > aOrgPxRect.Bottom() )
    {
        --aSizedRect.Bottom();
    }

    io_aSwRect = SwRect( aSizedRect );

#ifndef PRODUCT
    Rectangle aTestOrgPxRect = aOut.LogicToPixel( io_aSwRect.SVRect() );
    Rectangle aTestNewPxRect = aOut.LogicToPixel( aSizedRect );
    ASSERT( aTestOrgPxRect == aTestNewPxRect,
            "Error in lcl_AlignRectToPixelSize(..): Adjusted rectangle has incorrect position or size");
#if OSL_DEBUG_LEVEL > 1
    Rectangle aTestNewRect( aSizedRect );
    /// check Left()
    --aSizedRect.Left();
    aTestNewPxRect = aOut.LogicToPixel( aSizedRect );
    ASSERT( aTestOrgPxRect.Left() >= (aTestNewPxRect.Left()+1),
            "Error in lcl_AlignRectToPixelSize(..): Left() not correct adjusted");
    ++aSizedRect.Left();
    /// check Right()
    ++aSizedRect.Right();
    aTestNewPxRect = aOut.LogicToPixel( aSizedRect );
    ASSERT( aTestOrgPxRect.Right() <= (aTestNewPxRect.Right()-1),
            "Error in lcl_AlignRectToPixelSize(..): Right() not correct adjusted");
    --aSizedRect.Right();
    /// check Top()
    --aSizedRect.Top();
    aTestNewPxRect = aOut.LogicToPixel( aSizedRect );
    ASSERT( aTestOrgPxRect.Top() >= (aTestNewPxRect.Top()+1),
            "Error in lcl_AlignRectToPixelSize(..): Top() not correct adjusted");
    ++aSizedRect.Top();
    /// check Bottom()
    ++aSizedRect.Bottom();
    aTestNewPxRect = aOut.LogicToPixel( aSizedRect );
    ASSERT( aTestOrgPxRect.Bottom() <= (aTestNewPxRect.Bottom()-1),
            "Error in lcl_AlignRectToPixelSize(..): Bottom() not correct adjusted");
    --aSizedRect.Bottom();
#endif
#endif
}


//
// FUNCTIONS USED FOR COLLAPSING TABLE BORDER LINES START
//

struct SwLineEntry
{
    SwTwips mnKey;
    SwTwips mnStartPos;
    SwTwips mnEndPos;

    svx::frame::Style maAttribute;

    enum OverlapType { NO_OVERLAP, NO_SEGMENTS, ONE_SEGMENT_LEFT, ONE_SEGMENT_RIGHT, TWO_SEGMENTS };

public:
    SwLineEntry( SwTwips nKey,
                 SwTwips nStartPos,
                 SwTwips nEndPos,
                 const svx::frame::Style& rAttribute );

    BYTE Overlaps( const SwLineEntry& rComp ) const;
};

SwLineEntry::SwLineEntry( SwTwips nKey,
                          SwTwips nStartPos,
                          SwTwips nEndPos,
                          const svx::frame::Style& rAttribute )
    :   mnKey( nKey ),
        mnStartPos( nStartPos ),
        mnEndPos( nEndPos ),
        maAttribute( rAttribute )
{
}

BYTE SwLineEntry::Overlaps( const SwLineEntry& rComp )  const
{
    BYTE eRet;

    if ( mnStartPos >= rComp.mnEndPos || mnEndPos <= rComp.mnStartPos )
        eRet = NO_OVERLAP;
    else if ( mnStartPos < rComp.mnStartPos && mnEndPos > rComp.mnEndPos )
        eRet = TWO_SEGMENTS;
    else if ( mnStartPos >= rComp.mnStartPos && mnEndPos <= rComp.mnEndPos )
        eRet = NO_SEGMENTS;
    else
        eRet = ( mnStartPos < rComp.mnStartPos ) ? ONE_SEGMENT_LEFT : ONE_SEGMENT_RIGHT;

    return eRet;
}


typedef std::multimap< SwTwips, SwLineEntry >::iterator SwLineEntryIter;
typedef std::multimap< SwTwips, SwLineEntry >::const_iterator SwLineEntryConstIter;

class SwTabFrmPainter
{
    std::multimap< SwTwips, SwLineEntry > maVertLines;
    std::multimap< SwTwips, SwLineEntry > maHoriLines;
    const SwTabFrm& mrTabFrm;

    void Insert( SwLineEntry&, bool bHori );
    void Insert( const SwFrm& rFrm, const SvxBoxItem& rBoxItem );
    void HandleFrame( const SwFrm& rFrm );
    void FindStylesForLine( const Point&,
                            const Point&,
                            svx::frame::Style*,
                            bool bHori ) const;

public:
    SwTabFrmPainter( const SwTabFrm& rTabFrm );

    void PaintLines( OutputDevice& rDev, const SwRect& rRect ) const;
};

SwTabFrmPainter::SwTabFrmPainter( const SwTabFrm& rTabFrm )
    : mrTabFrm( rTabFrm )
{
    HandleFrame( rTabFrm );
}

void SwTabFrmPainter::HandleFrame( const SwFrm& rFrm )
{
    if ( rFrm.IsLayoutFrm() )
    {
        SwBorderAttrAccess aAccess( SwFrm::GetCache(), &rFrm );
        const SwBorderAttrs& rAttrs = *aAccess.Get();
        const SvxBoxItem& rBox = rAttrs.GetBox();

        Insert( rFrm, rBox );
        // recurse, but do not recurse into lower tabframes:
        SwFrm* pLower = ((SwLayoutFrm&)rFrm).Lower();
        while ( pLower )
        {
            if ( !pLower->IsTabFrm() )
                HandleFrame( *pLower );
            pLower = pLower->GetNext();
        }
    }
}

void SwTabFrmPainter::PaintLines( OutputDevice& rDev, const SwRect& rRect ) const
{
    const SwFrm* pTmpFrm = &mrTabFrm;
    SWRECTFN( pTmpFrm )

    SwLineEntryConstIter aIter = maHoriLines.begin();
    bool bHori = true;

    // color for subsidiary lines:
    const Color& rCol( SwViewOption::GetTableBoundariesColor() );

    // high contrast mode:
    // overrides the color of non-subsidiary lines.
    const Color* pHCColor = 0;
    ULONG nOldDrawMode = rDev.GetDrawMode();
    if( pGlobalShell->GetWin() &&
        Application::GetSettings().GetStyleSettings().GetHighContrastMode() )
    {
        pHCColor = &SwViewOption::GetFontColor();
        rDev.SetDrawMode( 0 );
    }

    // set clip region:
    rDev.Push( PUSH_CLIPREGION );
    Size aSize( rRect.SSize() );
    // Hack! Necessary, because the layout is not pixel aligned!
    aSize.Width() += nPixelSzW; aSize.Height() += nPixelSzH;
    rDev.SetClipRegion( Rectangle( rRect.Pos(), aSize ) );

    // The following stuff if necessary to have the new table borders fit
    // into a ::SwAlignRect adjusted world.
    const SwTwips nTwipXCorr =  bVert ? 0 : Max( 0L, nHalfPixelSzW - 2 );    // 1 < 2 < 3 ;-)
    const SwTwips nTwipYCorr = !bVert ? 0 : Max( 0L, nHalfPixelSzW - 2 );    // 1 < 2 < 3 ;-)
    const SwFrm* pUpper = mrTabFrm.GetUpper();
    SwRect aUpper( pUpper->Prt() );
    aUpper.Pos() += pUpper->Frm().Pos();
    SwRect aUpperAligned( aUpper );
    ::SwAlignRect( aUpperAligned, pGlobalShell );

    while ( true )
    {
        if ( aIter == maHoriLines.end() )
        {
            aIter = maVertLines.begin();
            bHori = false;
        }

        if ( aIter == maVertLines.end() )
            break;

        const SwLineEntry& rEntry = (*aIter).second;
        const svx::frame::Style& rEntryStyle( (*aIter).second.maAttribute );

        Point aStart, aEnd;
        if ( bHori )
        {
            aStart.X() = rEntry.mnStartPos;
            aStart.Y() = rEntry.mnKey;
            aEnd.X() = rEntry.mnEndPos;
            aEnd.Y() = rEntry.mnKey;
        }
        else
        {
            aStart.X() = rEntry.mnKey;
            aStart.Y() = rEntry.mnStartPos;
            aEnd.X() = rEntry.mnKey;
            aEnd.Y() = rEntry.mnEndPos;
        }

        SwRect aRepaintRect( aStart, aEnd );

        // the repaint rectangle has to be moved a bit for the centered lines:
        SwTwips nRepaintRectSize = !rEntryStyle.GetWidth() ? 1 : rEntryStyle.GetWidth();
        if ( bHori )
        {
            aRepaintRect.Height( 2 * nRepaintRectSize );
            aRepaintRect.Pos().Y() -= nRepaintRectSize;
        }
        else
        {
            aRepaintRect.Width( 2 * nRepaintRectSize );
            aRepaintRect.Pos().X() -= nRepaintRectSize;
        }

        if ( rRect.IsOver( aRepaintRect ) )
        {
            svx::frame::Style aStyles[ 7 ];
            aStyles[ 0 ] = rEntryStyle;
            FindStylesForLine( aStart, aEnd, aStyles, bHori );

            // subsidiary lines
            const Color* pTmpColor = 0;
            if ( 0 == aStyles[ 0 ].GetWidth() )
            {
                if ( IS_SUBS_TABLE && pGlobalShell->GetWin() )
                    aStyles[ 0 ].Set( rCol, 1, 0, 0 );
            }
            else
                pTmpColor = pHCColor;

            // The line sizes stored in the line style have to be adjusted as well.
            // This will guarantee that lines with the same twip size will have the
            // same pixel size.
            for ( int i = 0; i < 7; ++i )
            {
                sal_uInt16 nPrim = aStyles[ i ].Prim();
                sal_uInt16 nDist = aStyles[ i ].Dist();
                sal_uInt16 nSecn = aStyles[ i ].Secn();

                if ( nPrim > 0 )
                    nPrim = (sal_uInt16)( Max( 1L, nPixelSzH * ( nPrim / nPixelSzH ) ) );
                if ( nDist > 0 )
                    nDist = (sal_uInt16)( Max( 1L, nPixelSzH * ( nDist / nPixelSzH ) ) );
                if ( nSecn > 0 )
                    nSecn = (sal_uInt16)( Max( 1L, nPixelSzH * ( nSecn / nPixelSzH ) ) );

                aStyles[ i ].Set( nPrim, nDist, nSecn );
            }

            // The (twip) positions will be adjusted to meet these requirements:
            // 1. The y coordinates are located in the middle of the pixel grid
            // 2. The x coordinated are located at the beginning of the pixel grid
            // This is done, because the horizontal lines are painted "at beginning",
            // whereas the vertical lines are painted "centered". By making the line
            // sizes a multiple of one pixel size, we can assure, that all lines having
            // the same twip size have the same pixel size, independent of their position
            // on the screen.
            Point aPaintStart = rDev.PixelToLogic( rDev.LogicToPixel( aStart ) );
            Point aPaintEnd = rDev.PixelToLogic( rDev.LogicToPixel( aEnd ) );

            if( pGlobalShell->GetWin() )
            {
                // The table borders do not use SwAlignRect, but all the other frames do.
                // Therefore we tweak the outer borders a bit to achieve that the outer
                // borders match the subsidiary lines of the upper:
                if ( aStart.X() == aUpper.Left() )
                    aPaintStart.X() = aUpperAligned.Left();
                else if ( aStart.X() == aUpper._Right() )
                    aPaintStart.X() = aUpperAligned._Right();
                if ( aStart.Y() == aUpper.Top() )
                    aPaintStart.Y() = aUpperAligned.Top();
                else if ( aStart.Y() == aUpper._Bottom() )
                    aPaintStart.Y() = aUpperAligned._Bottom();

                if ( aEnd.X() == aUpper.Left() )
                    aPaintEnd.X() = aUpperAligned.Left();
                else if ( aEnd.X() == aUpper._Right() )
                    aPaintEnd.X() = aUpperAligned._Right();
                if ( aEnd.Y() == aUpper.Top() )
                    aPaintEnd.Y() = aUpperAligned.Top();
                else if ( aEnd.Y() == aUpper._Bottom() )
                    aPaintEnd.Y() = aUpperAligned._Bottom();
            }

            aPaintStart.X() -= nTwipXCorr; // nHalfPixelSzW - 2 to assure that we do not leave the pixel
            aPaintEnd.X()   -= nTwipXCorr;
            aPaintStart.Y() -= nTwipYCorr;
            aPaintEnd.Y()   -= nTwipYCorr;

            // Here comes the painting stuff: Thank you, DR, great job!!!
            if ( bHori )
                svx::frame::DrawHorFrameBorder
                (
                    rDev,
                    aPaintStart,
                    aPaintEnd,
                    aStyles[ 0 ],   // current style
                    aStyles[ 1 ],   // aLFromT
                    aStyles[ 2 ],   // aLFromL
                    aStyles[ 3 ],   // aLFromB
                    aStyles[ 4 ],   // aRFromT
                    aStyles[ 5 ],   // aRFromR
                    aStyles[ 6 ],   // aRFromB
                    pTmpColor
                );
            else
                svx::frame::DrawVerFrameBorder
                (
                    rDev,
                    aPaintStart,
                    aPaintEnd,
                    aStyles[ 0 ],   // current style
                    aStyles[ 1 ],   // aTFromL
                    aStyles[ 2 ],   // aTFromT
                    aStyles[ 3 ],   // aTFromR
                    aStyles[ 4 ],   // aBFromL
                    aStyles[ 5 ],   // aBFromB
                    aStyles[ 6 ],   // aBFromR
                    pTmpColor
                );
        }

        ++aIter;
    }

    // restore output device:
    rDev.Pop();
    rDev.SetDrawMode( nOldDrawMode );
}

// Finds the lines that join the line defined by (StartPoint, EndPoint) in either
// StartPoint or Endpoint. The styles of these lines are required for DR's magic
// line painting functions.
void SwTabFrmPainter::FindStylesForLine( const Point& rStartPoint,
                                         const Point& rEndPoint,
                                         svx::frame::Style* pStyles,
                                         bool bHori ) const
{
    // pStyles[ 1 ] = bHori ? aLFromT : TFromL
    // pStyles[ 2 ] = bHori ? aLFromL : TFromT,
    // pStyles[ 3 ] = bHori ? aLFromB : TFromR,
    // pStyles[ 4 ] = bHori ? aRFromT : BFromL,
    // pStyles[ 5 ] = bHori ? aRFromR : BFromB,
    // pStyles[ 6 ] = bHori ? aRFromB : BFromR,

    std::pair< SwLineEntryConstIter, SwLineEntryConstIter > aVertRange = maVertLines.equal_range( rStartPoint.X() );
    std::pair< SwLineEntryConstIter, SwLineEntryConstIter > aHoriRange = maHoriLines.equal_range( rStartPoint.Y() );

    SwLineEntryConstIter aIter = aVertRange.first;
    while ( aIter != aVertRange.second )
    {
        const SwLineEntry& rEntry = (*aIter).second;
        if ( bHori )
        {
            if ( rStartPoint.Y() == rEntry.mnStartPos )
                pStyles[ 3 ] = rEntry.maAttribute;
            else if ( rStartPoint.Y() == rEntry.mnEndPos )
                pStyles[ 1 ] = rEntry.maAttribute;
        }
        else
        {
            if ( rStartPoint.Y() == rEntry.mnEndPos )
                pStyles[ 2 ] = rEntry.maAttribute;
            else if ( rEndPoint.Y() == rEntry.mnStartPos )
                pStyles[ 5 ] = rEntry.maAttribute;
        }
        ++aIter;
    }

    aIter = aHoriRange.first;
    while ( aIter != aHoriRange.second )
    {
        const SwLineEntry& rEntry = (*aIter).second;
        if ( bHori )
        {
            if ( rStartPoint.X() == rEntry.mnEndPos )
                pStyles[ 2 ] = rEntry.maAttribute;
            else if ( rEndPoint.X() == rEntry.mnStartPos )
                pStyles[ 5 ] = rEntry.maAttribute;
        }
        else
        {
            if ( rStartPoint.X() == rEntry.mnEndPos )
                pStyles[ 1 ] = rEntry.maAttribute;
            else if ( rStartPoint.X() == rEntry.mnStartPos )
                pStyles[ 3 ] = rEntry.maAttribute;
        }
        ++aIter;
    }

    if ( bHori )
    {
        aVertRange = maVertLines.equal_range( rEndPoint.X() );
        aIter = aVertRange.first;
        while ( aIter != aVertRange.second )
        {
            const SwLineEntry& rEntry = (*aIter).second;
            if ( rEndPoint.Y() == rEntry.mnStartPos )
                pStyles[ 6 ] = rEntry.maAttribute;
            else if ( rEndPoint.Y() == rEntry.mnEndPos )
                pStyles[ 4 ] = rEntry.maAttribute;
            ++aIter;
        }
    }
    else
    {
        aHoriRange = maHoriLines.equal_range( rEndPoint.Y() );
        aIter = aHoriRange.first;
        while ( aIter != aHoriRange.second )
        {
            const SwLineEntry& rEntry = (*aIter).second;
            if ( rEndPoint.X() == rEntry.mnEndPos )
                pStyles[ 4 ] = rEntry.maAttribute;
            else if ( rEndPoint.X() == rEntry.mnStartPos )
                pStyles[ 6 ] = rEntry.maAttribute;
            ++aIter;
        }
    }
}

void SwTabFrmPainter::Insert( const SwFrm& rFrm, const SvxBoxItem& rBoxItem )
{
    // build 4 line entries for the 4 borders:
    SwRect aBorderRect = rFrm.Frm();
    if ( rFrm.IsTabFrm() )
    {
        aBorderRect = rFrm.Prt();
        aBorderRect.Pos() += rFrm.Frm().Pos();
    }

    const SwTwips nLeft   = aBorderRect._Left();
    const SwTwips nRight  = aBorderRect._Right();
    const SwTwips nTop    = aBorderRect._Top();
    const SwTwips nBottom = aBorderRect._Bottom();

    svx::frame::Style aL( rBoxItem.GetLeft() );
    svx::frame::Style aR( rBoxItem.GetRight() );
    svx::frame::Style aT( rBoxItem.GetTop() );
    svx::frame::Style aB( rBoxItem.GetBottom() );

    aR.MirrorSelf();
    aB.MirrorSelf();

    bool bVert = mrTabFrm.IsVertical();
    bool bR2L  = mrTabFrm.IsRightToLeft();

    aL.SetRefMode( svx::frame::REFMODE_CENTERED );
    aR.SetRefMode( svx::frame::REFMODE_CENTERED );
    aT.SetRefMode( !bVert ? svx::frame::REFMODE_BEGIN : svx::frame::REFMODE_END );
    aB.SetRefMode( !bVert ? svx::frame::REFMODE_BEGIN : svx::frame::REFMODE_END );

    SwLineEntry aLeft  ( nLeft,   nTop,  nBottom, bVert ? aB : ( bR2L ? aR : aL ) );
    SwLineEntry aRight ( nRight,  nTop,  nBottom, bVert ? aT : ( bR2L ? aL : aR ) );
    SwLineEntry aTop   ( nTop,    nLeft, nRight,  bVert ? aL : aT );
    SwLineEntry aBottom( nBottom, nLeft, nRight,  bVert ? aR : aB );

    Insert( aLeft, false );
    Insert( aRight, false );
    Insert( aTop, true );
    Insert( aBottom, true );
}

void SwTabFrmPainter::Insert( SwLineEntry& rNew, bool bHori )
{
    // get all lines from structure, that have key entry of pLE
    std::multimap< SwTwips, SwLineEntry >* pLines = bHori ? &maHoriLines : &maVertLines;
    const SwTwips nKey = rNew.mnKey;
    std::pair< SwLineEntryIter, SwLineEntryIter > aPair = pLines->equal_range( nKey );
    std::list< SwLineEntry > aInsertList;
    SwLineEntryIter aIter = aPair.first;

    bool bInsert = true;

    while ( aIter != aPair.second )
    {
        const BYTE nOverlapType = (*aIter).second.Overlaps( rNew );
        if ( SwLineEntry::NO_OVERLAP != nOverlapType )
        {
            const svx::frame::Style& rAttr = (*aIter).second.maAttribute;
            const svx::frame::Style  aNewAttr = rNew.maAttribute > rAttr ? rNew.maAttribute : rAttr;

            // rNew is completely covered by *aIter:
            if ( SwLineEntry::TWO_SEGMENTS == nOverlapType )
            {
                // left segment
                SwLineEntry aLeft( nKey, (*aIter).second.mnStartPos,
                                   rNew.mnStartPos, rAttr );

                aInsertList.push_back( aLeft );

                // right regment
                (*aIter).second.mnStartPos = rNew.mnEndPos;

                // insert new entry
                rNew.maAttribute = aNewAttr;

                break;
            }
            // rNew is partly covered by *aIter:
            else if ( SwLineEntry::ONE_SEGMENT_LEFT == nOverlapType ||
                      SwLineEntry::ONE_SEGMENT_RIGHT == nOverlapType )
            {
                bInsert = false;
                SwTwips nNewStartPos;
                SwTwips nNewEndPos;

                if ( SwLineEntry::ONE_SEGMENT_LEFT == nOverlapType )
                {
                    nNewStartPos = rNew.mnStartPos;
                    nNewEndPos = (*aIter).second.mnEndPos;
                    (*aIter).second.mnEndPos = rNew.mnStartPos;
                }
                else
                {
                    nNewStartPos = (*aIter).second.mnStartPos;
                    nNewEndPos = rNew.mnEndPos;
                    (*aIter).second.mnStartPos = rNew.mnEndPos;
                }

                SwLineEntry aNew( nKey, nNewStartPos, nNewEndPos, aNewAttr );
                aInsertList.push_back( aNew );
            }
            // *aIter is completely covered by rNew:
            else if ( SwLineEntry::NO_SEGMENTS == nOverlapType )
            {
                bInsert = false;
                (*aIter).second.maAttribute = aNewAttr;
            }
        }

        ++aIter;
    }

    std::list< SwLineEntry >::iterator aListIter = aInsertList.begin();
    for ( aListIter; aListIter != aInsertList.end(); ++aListIter )
    {
        pLines->insert( std::pair< const SwTwips, SwLineEntry >( nKey, *aListIter ) );
    }

    if ( bInsert )
        pLines->insert( std::pair< const SwTwips, SwLineEntry >( nKey, rNew ) );
}

//
// FUNCTIONS USED FOR COLLAPSING TABLE BORDER LINES END
//






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

    // #104289#. During painting, something (OLE) can
    // load the linguistic, which in turn can cause a reformat
    // of the document. Dangerous! We better set this flag to
    // avoid the reformat.
    const sal_Bool bOldAction = IsCallbackActionEnabled();
    ((SwRootFrm*)this)->SetCallbackActionEnabled( FALSE );

    const SwPageFrm *pPage = pSh->Imp()->GetFirstVisPage();

    while ( pPage && !::IsShortCut( aRect, pPage->Frm() ) )
    {
        if ( !pPage->IsEmptyPage() && aRect.IsOver( pPage->Frm() ) )
        {
            if ( pSh->GetWin() )
            {
                pSubsLines = new SwSubsRects;
                // OD 18.11.2002 #99672# - create array for special sub-lines
                pSpecSubsLines = new SwSubsRects;
            }

            SwRect aPaintRect( pPage->Frm() );
            aPaintRect._Intersection( aRect );

            if ( bExtraData )
            {
                //Ja, das ist grob, aber wie macht man es besser?
                SWRECTFN( pPage )
                (aPaintRect.*fnRect->fnSetLeftAndWidth)(
                    (pPage->Frm().*fnRect->fnGetLeft)(),
                    (pPage->Frm().*fnRect->fnGetWidth)() );
                aPaintRect._Intersection( pSh->VisArea() );
            }

            /// OD 27.09.2002 #103636# - changed method SwLayVout::Enter(..)
            /// 2nd parameter is no longer <const> and will be set to the
            /// rectangle the virtual output device is calculated from <aPaintRect>,
            /// if the virtual output is used.
            pVout->Enter( pSh, aPaintRect, !bNoVirDev );

            /// OD 27.09.2002 #103636# - adjust paint rectangle to pixel size
            /// Thus, all objects overlapping on pixel level with the unadjusted
            /// paint rectangle will be considered in the paint.
            lcl_AdjustRectToPixelSize( aPaintRect, *(pSh->GetOut()) );

            pVout->SetOrgRect( aPaintRect );

            pPage->PaintBaBo( aPaintRect, pPage, TRUE );

            /// OD 29.08.2002 #102450#
            /// determine background color of page for <PaintLayer> method
            /// calls, paint <hell> or <heaven>
            const Color aPageBackgrdColor = pPage->GetDrawBackgrdColor();

            if ( pSh->Imp()->HasDrawView() )
            {
                pLines->LockLines( TRUE );
                // OD 29.08.2002 #102450# - add 3rd parameter
                // OD 09.12.2002 #103045# - add 4th parameter for horizontal text direction.
                pSh->Imp()->PaintLayer( pSh->GetDoc()->GetHellId(), aPaintRect,
                                        &aPageBackgrdColor, (pPage->IsRightToLeft() ? true : false) );
                pLines->PaintLines( pSh->GetOut() );
                pLines->LockLines( FALSE );
            }

            if( pSh->GetWin() )
            {
                // OD 18.11.2002 #99672# - collect sub-lines
                pPage->RefreshSubsidiary( aPaintRect );
                // OD 18.11.2002 #99672# - paint special sub-lines
                pSpecSubsLines->PaintSubsidiary( pSh->GetOut(), NULL );
            }

            pPage->Paint( aPaintRect );

            // OD 20.12.2002 #94627# - no paint of page border and shadow, if
            // writer is in place mode.
            if( pSh->GetWin() &&
                !pSh->GetDoc()->GetDocShell()->IsInPlaceActive() )
            {
                // OD 12.02.2003 #i9719#, #105645# - use new method
                // <SwPageFrm::PaintBorderAndShadow(..)>.
                pPage->PaintBorderAndShadow( pPage->Frm(), pSh );
            }

            pLines->PaintLines( pSh->GetOut() );

            BOOL bControlExtra = FALSE;
            if ( pSh->Imp()->HasDrawView() )
            {
                /// OD 29.08.2002 #102450# - add 3rd parameter
                // OD 09.12.2002 #103045# - add 4th parameter for horizontal text direction.
                pSh->Imp()->PaintLayer( pSh->GetDoc()->GetHeavenId(), aPaintRect,
                                        &aPageBackgrdColor,
                                        (pPage->IsRightToLeft() ? true : false) );
                if( pVout->IsFlushable() )
                    bControlExtra = TRUE;
                else
                    pSh->Imp()->PaintLayer( pSh->GetDoc()->GetControlsId(), aPaintRect );
                pLines->PaintLines( pSh->GetOut() );
            }

            if ( bExtraData )
                pPage->RefreshExtraData( aPaintRect );

            if ( pSh->GetWin() )
            {
                pSubsLines->PaintSubsidiary( pSh->GetOut(), pLines );
                DELETEZ( pSubsLines );
                DELETEZ( pSpecSubsLines );
            }
            pVout->Leave();
            if( bControlExtra )
                pSh->Imp()->PaintLayer( pSh->GetDoc()->GetControlsId(), aPaintRect );
        }
        ASSERT( !pPage->GetNext() || pPage->GetNext()->IsPageFrm(),
                "Nachbar von Seite keine Seite." );
        pPage = (SwPageFrm*)pPage->GetNext();
    }

    DELETEZ( pLines );

    if ( pSh->GetWin() && pSh->Imp()->HasDrawView() &&
         pSh->Imp()->GetDrawView()->IsGridVisible() )
    {
        pSh->Imp()->GetDrawView()->GetPageViewPgNum(0)->DrawGrid(
            *pSh->GetOut(), rRect.SVRect(), SwViewOption::GetTextGridColor() );
    }

    // call RefreshAllIAOManagers only once in paint, so do it here
    // and not in DrawOneLayer()
    if(pSh->GetWin() && pSh->Imp()->HasDrawView())
    {
        pSh->Imp()->GetDrawView()->RefreshAllIAOManagers();
    }

    if ( bResetRootPaint )
        SwRootFrm::bInPaint = FALSE;
    if ( pStatics )
        delete pStatics;
    else
    {
        pProgress = 0;
        pGlobalShell = 0;
    }

    //if ( ViewShell::IsLstEndAction() && pSh->GetWin() && pSh->Imp()->HasDrawView() )
    //  pSh->Imp()->GetDrawView()->PostPaint();

    ((SwRootFrm*)this)->SetCallbackActionEnabled( bOldAction );
}

#ifdef LONG_TABLE_HACK

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

#endif


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

class SwShortCut
{
    SwRectDist fnCheck;
    long nLimit;
public:
    SwShortCut( const SwFrm& rFrm, const SwRect& rRect );
    BOOL Stop( const SwRect& rRect ) const
        { return (rRect.*fnCheck)( nLimit ) > 0; }
};

SwShortCut::SwShortCut( const SwFrm& rFrm, const SwRect& rRect )
{
    BOOL bVert = rFrm.IsVertical();
    BOOL bR2L = rFrm.IsRightToLeft();
    if( rFrm.IsNeighbourFrm() && bVert == bR2L )
    {
        if( bVert )
        {
            fnCheck = &SwRect::GetBottomDistance;
            nLimit = rRect.Top();
        }
        else
        {
            fnCheck = &SwRect::GetLeftDistance;
            nLimit = rRect.Left() + rRect.Width();
        }
    }
    else if( bVert == rFrm.IsNeighbourFrm() )
    {
        fnCheck = &SwRect::GetTopDistance;
        nLimit = rRect.Top() + rRect.Height();
    }
    else
    {
        fnCheck = &SwRect::GetRightDistance;
        nLimit = rRect.Left();
    }
}

void SwLayoutFrm::Paint( const SwRect& rRect ) const
{
    const SwFrm *pFrm = Lower();
    if ( !pFrm )
        return;

    SwShortCut aShortCut( *pFrm, rRect );
    BOOL bCnt;
    if ( TRUE == (bCnt = pFrm->IsCntntFrm()) )
        pFrm->Calc();

    if ( pFrm->IsFtnContFrm() )
    {
        ::lcl_EmergencyFormatFtnCont( (SwFtnContFrm*)pFrm );
        pFrm = Lower();
    }

    const SwPageFrm *pPage = 0;
    const FASTBOOL bWin   = pGlobalShell->GetWin() ? TRUE : FALSE;

    while ( IsAnLower( pFrm ) )
    {
        SwRect aPaintRect( pFrm->PaintArea() );
        if( aShortCut.Stop( aPaintRect ) )
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
        }
        if ( !bCnt && pFrm->GetNext() && pFrm->GetNext()->IsFtnContFrm() )
            ::lcl_EmergencyFormatFtnCont( (SwFtnContFrm*)pFrm->GetNext() );

        pFrm = pFrm->GetNext();
        if ( pFrm && (TRUE == (bCnt = pFrm->IsCntntFrm())) )
            pFrm->Calc();
    }
}


/** FlyFrm::IsBackgroundTransparent - for feature #99657#

    OD 12.08.2002
    determines, if background of fly frame has to be drawn transparent
    declaration found in /core/inc/flyfrm.cxx
    OD 08.10.2002 #103898# - If the background of the fly frame itself is not
    transparent and the background is inherited from its parent/grandparent,
    the background brush, used for drawing, has to be investigated for transparency.

    @author OD

    @return true, if background is transparent drawn.
*/
const sal_Bool SwFlyFrm::IsBackgroundTransparent() const
{
    sal_Bool bBackgroundTransparent = GetFmt()->IsBackgroundTransparent();
    if ( !bBackgroundTransparent &&
         static_cast<const SwFlyFrmFmt*>(GetFmt())->IsBackgroundBrushInherited() )
    {
        const SvxBrushItem* pBackgrdBrush = 0;
        const Color* pSectionTOXColor = 0;
        SwRect aDummyRect;
        if ( GetBackgroundBrush( pBackgrdBrush, pSectionTOXColor, aDummyRect, false) )
        {
            if ( pSectionTOXColor &&
                 (pSectionTOXColor->GetTransparency() != 0) &&
                 (pSectionTOXColor->GetColor() != COL_TRANSPARENT) )
            {
                bBackgroundTransparent = sal_True;
            }
            else if ( pBackgrdBrush )
            {
                if ( (pBackgrdBrush->GetColor().GetTransparency() != 0) &&
                     (pBackgrdBrush->GetColor() != COL_TRANSPARENT) )
                {
                    bBackgroundTransparent = sal_True;
                }
                else
                {
                    const GraphicObject *pTmpGrf =
                            static_cast<const GraphicObject*>(pBackgrdBrush->GetGraphicObject());
                    if ( (pTmpGrf) &&
                         (pTmpGrf->GetAttr().GetTransparency() != 0)
                       )
                    {
                        bBackgroundTransparent = sal_True;
                    }
                }
            }
        }
    }

    return bBackgroundTransparent;
};

/** FlyFrm::IsShadowTransparent - for feature #99657#

    OD 13.08.2002
    determine, if shadow color of fly frame has to be drawn transparent
    declaration found in /core/inc/flyfrm.cxx

    @author OD

    @return true, if shadow color is transparent.
*/
const sal_Bool SwFlyFrm::IsShadowTransparent() const
{
    return GetFmt()->IsShadowTransparent();
};

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
        if ( pObj->ISA(SwVirtFlyDrawObj) )
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
                    pAnch = pFly->AnchorFrm();
                else if ( bTableHack &&
                          pFly->Frm().Top() >= pFly->GetAnchorFrm()->Frm().Top() &&
                          pFly->Frm().Top() < pFly->GetAnchorFrm()->Frm().Bottom() &&
                          long(pSh->GetOut()) == long(pSh->GetPrt()) )
                {
                    pAnch = pFly->AnchorFrm();
                }
            }

        }
        else
        {
            // OD 13.10.2003 #i19919# - consider 'virtual' drawing objects
            // OD 2004-03-29 #i26791#
            pAnch = ((SwDrawContact*)pUserCall)->GetAnchorFrm( pObj );
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
                    const SwPageFrm *pPage = pAnch->FindPageFrm();
                    if ( !bTableHack &&
                         !pPage->Frm().IsOver( pObj->GetCurrentBoundRect() ) )
                        pAnch = 0;
                }
            }
            else
            {
                // OD 02.07.2003 #108784# - debug assert
                if ( !pObj->ISA(SdrObjGroup) )
                {
                    ASSERT( false, "<SwFlyFrm::IsPaint(..)> - paint of drawing object without anchor frame!?" );
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
    aRect._Intersection( Frm() );

    OutputDevice* pOut = pGlobalShell->GetOut();
    pOut->Push( PUSH_CLIPREGION );
    pOut->SetClipRegion();
    const SwPageFrm* pPage = FindPageFrm();

    const SwNoTxtFrm *pNoTxt = Lower() && Lower()->IsNoTxtFrm()
                                                ? (SwNoTxtFrm*)Lower() : 0;
    /// OD 19.08.2002 #99657# - rename local variable
    ///     <bTransparent> --> <bIsGraphicTransparent>
    FASTBOOL bIsGraphicTransparent = pNoTxt ? pNoTxt->IsTransparent() : FALSE,
             bContour     = GetFmt()->GetSurround().IsContour(),
             bHell, bPaintBack;

    if ( bIsGraphicTransparent &&
         GetVirtDrawObj()->GetLayer() == GetFmt()->GetDoc()->GetHellId() &&
         GetAnchorFrm()->FindFlyFrm() )
    {
        SwFlyFrm *pOldRet = pRetoucheFly2; pRetoucheFly2 = (SwFlyFrm*)this;
        const SwFrm *pFrm = GetAnchorFrm()->FindFlyFrm();
        SwBorderAttrAccess aAccess( SwFrm::GetCache(), pFrm );
        const SwBorderAttrs &rAttrs = *aAccess.Get();
        pFrm->PaintBackground( aRect, pPage, rAttrs, FALSE, FALSE );
        pRetoucheFly2 = pOldRet;
    }
//#33429#           else
    {
        PolyPolygon aPoly;
        if ( bContour )
        {
            // OD 16.04.2003 #i13147# - add 2nd parameter with value <sal_True>
            // to indicate that method is called for paint in order to avoid
            // load of the intrinsic graphic.
            bContour = GetContour( aPoly, sal_True );
        }

        //Hintergrund painten fuer:
        bPaintBack = !pNoTxt || Prt().SSize() != Frm().SSize();
        //sowie fuer Transparente und Contour in der Hoelle
        bPaintBack = bPaintBack ||
                ((bIsGraphicTransparent || bContour ) &&
                TRUE == (bHell = GetVirtDrawObj()->GetLayer() == GetFmt()->GetDoc()->GetHellId()));
        //sowie fuer Transparente und Contour mit eigener Brush
        if ( !bPaintBack && (bIsGraphicTransparent||bContour) )
        {
            const SvxBrushItem &rBack = GetFmt()->GetBackground();
            /// OD 07.08.2002 #99657# #GetTransChg#
            ///     to determine, if background has to be painted, by checking, if
            ///     background color is not COL_TRANSPARENT ("no fill"/"auto fill")
            ///     or a background graphic exists.
            bPaintBack = !(rBack.GetColor() == COL_TRANSPARENT) ||
            ///bPaintBack = !rBack.GetColor().GetTransparency() ||
                         rBack.GetGraphicPos() != GPOS_NONE;
        }

        if ( bPaintBack )
        {
            //#24926# JP 01.02.96, PaintBaBo in teilen hier, damit PaintBorder
            //das orig. Rect bekommt, aber PaintBackground das begrenzte.

            // OD 2004-04-23 #116347#
            pOut->Push( PUSH_FILLCOLOR|PUSH_LINECOLOR );
            pOut->SetLineColor();

            pPage = FindPageFrm();

            SwBorderAttrAccess aAccess( SwFrm::GetCache(), (SwFrm*)this );
            const SwBorderAttrs &rAttrs = *aAccess.Get();

            // OD 06.08.2002 #99657# - paint border before painting background
            // paint border
            {
                SwRect aTmp( rRect );
                PaintBorder( aTmp, pPage, rAttrs );
            }

            // paint background
            {
                SwRegionRects aRegion( aRect );
                if ( pNoTxt && !bIsGraphicTransparent )
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
            }

            pOut->Pop();
        }
    }

    // OD 19.12.2002 #106318# - fly frame will paint it's subsidiary lines and
    // the subsidiary lines of its lowers on its own, due to overlapping with
    // other fly frames or other objects.
    if( pGlobalShell->GetWin() )
    {
        bool bSubsLineRectsCreated;
        if ( pSubsLines )
        {
            // Lock already existing subsidiary lines
            pSubsLines->LockLines( TRUE );
            bSubsLineRectsCreated = false;
        }
        else
        {
            // create new subsidiardy lines
            pSubsLines = new SwSubsRects;
            bSubsLineRectsCreated = true;
        }

        bool bSpecSubsLineRectsCreated;
        if ( pSpecSubsLines )
        {
            // Lock already existing special subsidiary lines
            pSpecSubsLines->LockLines( TRUE );
            bSpecSubsLineRectsCreated = false;
        }
        else
        {
            // create new special subsidiardy lines
            pSpecSubsLines = new SwSubsRects;
            bSpecSubsLineRectsCreated = true;
        }
        // Add subsidiary lines of fly frame and its lowers
        RefreshLaySubsidiary( pPage, aRect );
        // paint subsidiary lines of fly frame and its lowers
        pSpecSubsLines->PaintSubsidiary( pOut, NULL );
        pSubsLines->PaintSubsidiary( pOut, pLines );
        if ( !bSubsLineRectsCreated )
            // unlock subsidiary lines
            pSubsLines->LockLines( FALSE );
        else
            // delete created subsidiary lines container
            DELETEZ( pSubsLines );

        if ( !bSpecSubsLineRectsCreated )
            // unlock special subsidiary lines
            pSpecSubsLines->LockLines( FALSE );
        else
            // delete created special subsidiary lines container
            DELETEZ( pSpecSubsLines );
    }

    SwLayoutFrm::Paint( aRect );
    Validate();

    // OD 19.12.2002 #106318# - first paint lines added by fly frame paint
    // and then unlock other lines.
    pLines->PaintLines( pOut );
    pLines->LockLines( FALSE );

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
    if ( pGlobalShell->GetViewOptions()->IsTable() )
    {
        // --> collapsing borders FME 2005-05-27 #i29550#
        if ( IsCollapsingBorders() )
        {
            SwBorderAttrAccess aAccess( SwFrm::GetCache(), (SwFrm*)this );
            const SwBorderAttrs &rAttrs = *aAccess.Get();

            // paint shadow
            if ( rAttrs.GetShadow().GetLocation() != SVX_SHADOW_NONE )
            {
                SwRect aRect;
                ::lcl_CalcBorderRect( aRect, this, rAttrs, TRUE );
                const SwPageFrm* pPage = FindPageFrm();
                PaintShadow( rRect, aRect, pPage, rAttrs );
            }

            // paint lines
            SwTabFrmPainter aHelper( *this );
            aHelper.PaintLines( *pGlobalShell->GetOut(), rRect );
        }
        // <-- collapsing

        SwLayoutFrm::Paint( rRect );
    }
    // OD 10.01.2003 #i6467# - no light grey rectangle for page preview
    else if ( pGlobalShell->GetWin() && !pGlobalShell->IsPreView() )
    {
        // OD 10.01.2003 #i6467# - intersect output rectangle with table frame
        SwRect aTabRect( Prt() );
        aTabRect.Pos() += Frm().Pos();
        SwRect aTabOutRect( rRect );
        aTabOutRect.Intersection( aTabRect );
        pGlobalShell->GetViewOptions()->
                DrawRect( pGlobalShell->GetOut(), aTabOutRect, COL_LIGHTGRAY );
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
/// OD 23.08.2002 #99657#
///     draw full shadow rectangle for frames with transparent drawn backgrounds.
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
    const FASTBOOL bTop    = !bCnt || rAttrs.GetTopLine  ( *(this) ) ? TRUE : FALSE;
    const FASTBOOL bBottom = !bCnt || rAttrs.GetBottomLine( *(this) ) ? TRUE : FALSE;

    SvxShadowLocation eLoc = rShadow.GetLocation();

    SWRECTFN( this )
    if( IsVertical() )
    {
        switch( eLoc )
        {
            case SVX_SHADOW_BOTTOMRIGHT: eLoc = SVX_SHADOW_BOTTOMLEFT;  break;
            case SVX_SHADOW_TOPLEFT:     eLoc = SVX_SHADOW_TOPRIGHT;    break;
            case SVX_SHADOW_TOPRIGHT:    eLoc = SVX_SHADOW_BOTTOMRIGHT; break;
            case SVX_SHADOW_BOTTOMLEFT:  eLoc = SVX_SHADOW_TOPLEFT;     break;
        }
    }

    /// OD 23.08.2002 #99657# - determine, if full shadow rectangle have to
    ///     be drawn or only two shadow rectangles beside the frame.
    ///     draw full shadow rectangle, if frame background is drawn transparent.
    ///     Status Quo:
    ///         SwLayoutFrm can have transparent drawn backgrounds. Thus,
    ///         "asked" their frame format.
    sal_Bool bDrawFullShadowRectangle =
            ( IsLayoutFrm() &&
              (static_cast<const SwLayoutFrm*>(this))->GetFmt()->IsBackgroundTransparent()
            );
    switch ( eLoc )
    {
        case SVX_SHADOW_BOTTOMRIGHT:
            {
                if ( bDrawFullShadowRectangle )
                {
                    /// OD 06.08.2002 #99657# - draw full shadow rectangle
                    aOut.Top( aOut.Top() + nHeight );
                    aOut.Left( aOut.Left() + nWidth );
                    aRegion.Insert( aOut, aRegion.Count() );
                }
                else
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
                        ::lcl_ExtendLeftAndRight( aOut, *(this), rAttrs, fnRect );
                    aRegion.Insert( aOut, aRegion.Count() );
                }

                rOutRect.Right ( rOutRect.Right() - nWidth );
                rOutRect.Bottom( rOutRect.Bottom()- nHeight );
            }
            break;
        case SVX_SHADOW_TOPLEFT:
            {
                if ( bDrawFullShadowRectangle )
                {
                    /// OD 06.08.2002 #99657# - draw full shadow rectangle
                    aOut.Bottom( aOut.Bottom() - nHeight );
                    aOut.Right( aOut.Right() - nWidth );
                    aRegion.Insert( aOut, aRegion.Count() );
                }
                else
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
                        ::lcl_ExtendLeftAndRight( aOut, *(this), rAttrs, fnRect );
                    aRegion.Insert( aOut, aRegion.Count() );
                }

                rOutRect.Left( rOutRect.Left() + nWidth );
                rOutRect.Top(  rOutRect.Top() + nHeight );
            }
            break;
        case SVX_SHADOW_TOPRIGHT:
            {
                if ( bDrawFullShadowRectangle )
                {
                    /// OD 06.08.2002 #99657# - draw full shadow rectangle
                    aOut.Bottom( aOut.Bottom() - nHeight);
                    aOut.Left( aOut.Left() + nWidth );
                    aRegion.Insert( aOut, aRegion.Count() );
                }
                else
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
                        ::lcl_ExtendLeftAndRight( aOut, *(this), rAttrs, fnRect );
                    aRegion.Insert( aOut, aRegion.Count() );
                }

                rOutRect.Right( rOutRect.Right() - nWidth );
                rOutRect.Top( rOutRect.Top() + nHeight );
            }
            break;
        case SVX_SHADOW_BOTTOMLEFT:
            {
                if ( bDrawFullShadowRectangle )
                {
                    /// OD 06.08.2002 #99657# - draw full shadow rectangle
                    aOut.Top( aOut.Top() + nHeight );
                    aOut.Right( aOut.Right() - nWidth );
                    aRegion.Insert( aOut, aRegion.Count() );
                }
                else
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
                        ::lcl_ExtendLeftAndRight( aOut, *(this), rAttrs, fnRect );
                    aRegion.Insert( aOut, aRegion.Count() );
                }

                rOutRect.Left( rOutRect.Left() + nWidth );
                rOutRect.Bottom( rOutRect.Bottom() - nHeight );
            }
            break;
#ifndef PRODUCT
        default:    ASSERT( !this, "new ShadowLocation() ?" );
#endif
    }

    OutputDevice *pOut = pGlobalShell->GetOut();

    ULONG nOldDrawMode = pOut->GetDrawMode();
    Color aShadowColor( rShadow.GetColor() );
    if( aRegion.Count() && pGlobalShell->GetWin() &&
        Application::GetSettings().GetStyleSettings().GetHighContrastMode() )
    {
        // Is heigh contrast mode, the output device has already set the
        // DRAWMODE_SETTINGSFILL flag. This causes the SetFillColor function
        // to ignore the setting of a new color. Therefore we have to reset
        // the drawing mode
        pOut->SetDrawMode( 0 );
        aShadowColor = SwViewOption::GetFontColor();
    }

    if ( pOut->GetFillColor() != aShadowColor )
        pOut->SetFillColor( aShadowColor );

    pOut->SetDrawMode( nOldDrawMode );

    for ( USHORT i = 0; i < aRegion.Count(); ++i )
    {
        SwRect &rOut = aRegion[i];
        aOut = rOut;
        // OD 30.09.2002 #103636# - no SwAlign of shadow rectangle
        // no alignment necessary, because (1) <rRect> is already aligned
        // and because (2) paint of border and background will occur later.
        // Thus, (1) assures that no conflicts with neighbour object will occure
        // and (2) assures that border and background is not affected by the
        // shadow paint.
        /*
        ::SwAlignRect( aOut, pGlobalShell );
        */
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
    BYTE nSubCol = ( IsCellFrm() || IsRowFrm() ) ? SUBCOL_TAB :
                   ( IsInSct() ? SUBCOL_SECT :
                   ( IsInFly() ? SUBCOL_FLY : SUBCOL_PAGE ) );
    if( pColor && pGlobalShell->GetWin() &&
        Application::GetSettings().GetStyleSettings().GetHighContrastMode() )
    {
        pColor = &SwViewOption::GetFontColor();
    }

    if ( pPage->GetSortedObjs() )
    {
        SwRegionRects aRegion( aOut, 4, 1 );
        ::lcl_SubtractFlys( this, pPage, aOut, aRegion );
        for ( USHORT i = 0; i < aRegion.Count(); ++i )
            pLines->AddLineRect( aRegion[i], pColor, pTab, nSubCol );
    }
    else
        pLines->AddLineRect( aOut, pColor, pTab, nSubCol );
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

// OD 29.04.2003 #107169# - method called for left and right border rectangles.
// For a printer output device perform adjustment for non-overlapping top and
// bottom border rectangles. Thus, add parameter <_bPrtOutputDev> to indicate
// printer output device.
// NOTE: For printer output device left/right border rectangle <_iorRect>
//       has to be already non-overlapping the outer top/bottom border rectangle.
void MA_FASTCALL lcl_SubTopBottom( SwRect&              _iorRect,
                                   const SvxBoxItem&    _rBox,
                                   const SwBorderAttrs& _rAttrs,
                                   const SwFrm&         _rFrm,
                                   const SwRectFn&      _rRectFn,
                                   const sal_Bool       _bPrtOutputDev )
{
    const BOOL bCnt = _rFrm.IsCntntFrm();
    if ( _rBox.GetTop() && _rBox.GetTop()->GetInWidth() &&
         ( !bCnt || _rAttrs.GetTopLine( _rFrm ) )
       )
    {
        // substract distance between outer and inner line.
        SwTwips nDist = ::lcl_MinHeightDist( _rBox.GetTop()->GetDistance() );
        // OD 19.05.2003 #109667# - non-overlapping border rectangles:
        // adjust x-/y-position, if inner top line is a hair line (width = 1)
        sal_Bool bIsInnerTopLineHairline = sal_False;
        if ( !_bPrtOutputDev )
        {
            // additionally substract width of top outer line
            // --> left/right inner/outer line doesn't overlap top outer line.
            nDist += ::lcl_AlignHeight( _rBox.GetTop()->GetOutWidth() );
        }
        else
        {
            // OD 29.04.2003 #107169# - additionally substract width of top inner line
            // --> left/right inner/outer line doesn't overlap top inner line.
            nDist += ::lcl_AlignHeight( _rBox.GetTop()->GetInWidth() );
            bIsInnerTopLineHairline = _rBox.GetTop()->GetInWidth() == 1;
        }
        (_iorRect.*_rRectFn->fnSubTop)( -nDist );
        // OD 19.05.2003 #109667# - adjust calculated border top, if inner top line
        // is a hair line
        if ( bIsInnerTopLineHairline )
        {
            if ( _rFrm.IsVertical() )
            {
                // right of border rectangle has to be checked and adjusted
                Point aCompPt( _iorRect.Right(), 0 );
                Point aRefPt( aCompPt.X() + 1, aCompPt.Y() );
                lcl_CompPxPosAndAdjustPos( *(pGlobalShell->GetOut()),
                                          aRefPt, aCompPt,
                                          sal_True, -1 );
                _iorRect.Right( aCompPt.X() );
            }
            else
            {
                // top of border rectangle has to be checked and adjusted
                Point aCompPt( 0, _iorRect.Top() );
                Point aRefPt( aCompPt.X(), aCompPt.Y() - 1 );
                lcl_CompPxPosAndAdjustPos( *(pGlobalShell->GetOut()),
                                          aRefPt, aCompPt,
                                          sal_False, +1 );
                _iorRect.Top( aCompPt.Y() );
            }
        }
    }

    if ( _rBox.GetBottom() && _rBox.GetBottom()->GetInWidth() &&
         ( !bCnt || _rAttrs.GetBottomLine( _rFrm ) )
       )
    {
        // substract distance between outer and inner line.
        SwTwips nDist = ::lcl_MinHeightDist( _rBox.GetBottom()->GetDistance() );
        // OD 19.05.2003 #109667# - non-overlapping border rectangles:
        // adjust x-/y-position, if inner bottom line is a hair line (width = 1)
        sal_Bool bIsInnerBottomLineHairline = sal_False;
        if ( !_bPrtOutputDev )
        {
            // additionally substract width of bottom outer line
            // --> left/right inner/outer line doesn't overlap bottom outer line.
            nDist += ::lcl_AlignHeight( _rBox.GetBottom()->GetOutWidth() );
        }
        else
        {
            // OD 29.04.2003 #107169# - additionally substract width of bottom inner line
            // --> left/right inner/outer line doesn't overlap bottom inner line.
            nDist += ::lcl_AlignHeight( _rBox.GetBottom()->GetInWidth() );
            bIsInnerBottomLineHairline = _rBox.GetBottom()->GetInWidth() == 1;
        }
        (_iorRect.*_rRectFn->fnAddBottom)( -nDist );
        // OD 19.05.2003 #109667# - adjust calculated border bottom, if inner
        // bottom line is a hair line.
        if ( bIsInnerBottomLineHairline )
        {
            if ( _rFrm.IsVertical() )
            {
                // left of border rectangle has to be checked and adjusted
                Point aCompPt( _iorRect.Left(), 0 );
                Point aRefPt( aCompPt.X() - 1, aCompPt.Y() );
                lcl_CompPxPosAndAdjustPos( *(pGlobalShell->GetOut()),
                                          aRefPt, aCompPt,
                                          sal_True, +1 );
                _iorRect.Left( aCompPt.X() );
            }
            else
            {
                // bottom of border rectangle has to be checked and adjusted
                Point aCompPt( 0, _iorRect.Bottom() );
                Point aRefPt( aCompPt.X(), aCompPt.Y() + 1 );
                lcl_CompPxPosAndAdjustPos( *(pGlobalShell->GetOut()),
                                          aRefPt, aCompPt,
                                          sal_False, -1 );
                _iorRect.Bottom( aCompPt.Y() );
            }
        }
    }
}

// method called for top and bottom border rectangles.
void MA_FASTCALL lcl_SubLeftRight( SwRect&           rRect,
                                   const SvxBoxItem& rBox,
                                   const SwRectFn&   rRectFn )
{
    if ( rBox.GetLeft() && rBox.GetLeft()->GetInWidth() )
    {
        const long nDist = ::lcl_MinWidthDist( rBox.GetLeft()->GetDistance() )
                           + ::lcl_AlignWidth( rBox.GetLeft()->GetOutWidth() );
        (rRect.*rRectFn->fnSubLeft)( -nDist );
    }

    if ( rBox.GetRight() && rBox.GetRight()->GetInWidth() )
    {
        const long nDist = ::lcl_MinWidthDist( rBox.GetRight()->GetDistance() )
                           + ::lcl_AlignWidth( rBox.GetRight()->GetOutWidth() );
        (rRect.*rRectFn->fnAddRight)( -nDist );
    }
}

// OD 19.05.2003 #109667# - merge <lcl_PaintLeftLine> and <lcl_PaintRightLine>
// into new method <lcl_PaintLeftRightLine(..)>
void lcl_PaintLeftRightLine( const sal_Bool         _bLeft,
                             const SwFrm&           _rFrm,
                             const SwPageFrm&       _rPage,
                             const SwRect&          _rOutRect,
                             const SwRect&          _rRect,
                             const SwBorderAttrs&   _rAttrs,
                             const SwRectFn&        _rRectFn )
{
    const SvxBoxItem& rBox = _rAttrs.GetBox();
    const sal_Bool bR2L = _rFrm.IsCellFrm() && _rFrm.IsRightToLeft();
    const SvxBorderLine* pLeftRightBorder = 0;
    if ( _bLeft )
    {
        pLeftRightBorder = bR2L ? rBox.GetRight() : rBox.GetLeft();
    }
    else
    {
        pLeftRightBorder = bR2L ? rBox.GetLeft() : rBox.GetRight();
    }
    // OD 06.05.2003 #107169# - init boolean indicating printer output device.
    const sal_Bool bPrtOutputDev =
            ( OUTDEV_PRINTER == pGlobalShell->GetOut()->GetOutDevType() );

    if ( !pLeftRightBorder )
    {
        return;
    }

    SwRect aRect( _rOutRect );
    if ( _bLeft )
    {
        (aRect.*_rRectFn->fnAddRight)( ::lcl_AlignWidth( pLeftRightBorder->GetOutWidth() ) -
                                       (aRect.*_rRectFn->fnGetWidth)() );
    }
    else
    {
        (aRect.*_rRectFn->fnSubLeft)( ::lcl_AlignWidth( pLeftRightBorder->GetOutWidth() ) -
                                      (aRect.*_rRectFn->fnGetWidth)() );
    }

    const BOOL bCnt = _rFrm.IsCntntFrm();

    if ( bCnt )
    {
        ::lcl_ExtendLeftAndRight( aRect, _rFrm, _rAttrs, _rRectFn );
    }

    // OD 06.05.2003 #107169# - adjustments for printer output device
    if ( bPrtOutputDev )
    {
        // substract width of outer top line.
        if ( rBox.GetTop() && (!bCnt || _rAttrs.GetTopLine( _rFrm )) )
        {
            long nDist = ::lcl_AlignHeight( rBox.GetTop()->GetOutWidth() );
            (aRect.*_rRectFn->fnSubTop)( -nDist );
            // OD 19.05.2003 #109667# - If outer top line is hair line, calculated
            // top has to be adjusted.
            if ( nDist == 1 )
            {
                if ( _rFrm.IsVertical() )
                {
                    // right of border rectangle has to be checked and adjusted
                    Point aCompPt( aRect.Right(), 0 );
                    Point aRefPt( aCompPt.X() + 1, aCompPt.Y() );
                    lcl_CompPxPosAndAdjustPos( *(pGlobalShell->GetOut()),
                                              aRefPt, aCompPt,
                                              sal_True, -1 );
                    aRect.Right( aCompPt.X() );
                }
                else
                {
                    // top of border rectangle has to be checked and adjusted
                    Point aCompPt( 0, aRect.Top() );
                    Point aRefPt( aCompPt.X(), aCompPt.Y() - 1 );
                    lcl_CompPxPosAndAdjustPos( *(pGlobalShell->GetOut()),
                                              aRefPt, aCompPt,
                                              sal_False, +1 );
                    aRect.Top( aCompPt.Y() );
                }
            }
        }
        // substract width of outer bottom line.
        if ( rBox.GetBottom() && (!bCnt || _rAttrs.GetBottomLine( _rFrm )) )
        {
            long nDist = ::lcl_AlignHeight( rBox.GetBottom()->GetOutWidth());
            (aRect.*_rRectFn->fnAddBottom)( -nDist );
            // OD 19.05.2003 #109667# - If outer bottom line is hair line, calculated
            // top has to be adjusted.
            if ( nDist == 1 )
            {
                if ( _rFrm.IsVertical() )
                {
                    // left of border rectangle has to be checked and adjusted
                    Point aCompPt( aRect.Left(), 0 );
                    Point aRefPt( aCompPt.X() - 1, aCompPt.Y() );
                    lcl_CompPxPosAndAdjustPos( *(pGlobalShell->GetOut()),
                                              aRefPt, aCompPt,
                                              sal_True, +1 );
                    aRect.Left( aCompPt.X() );
                }
                else
                {
                    // bottom of border rectangle has to be checked and adjusted
                    Point aCompPt( 0, aRect.Bottom() );
                    Point aRefPt( aCompPt.X(), aCompPt.Y() + 1 );
                    lcl_CompPxPosAndAdjustPos( *(pGlobalShell->GetOut()),
                                              aRefPt, aCompPt,
                                              sal_False, -1 );
                    aRect.Bottom( aCompPt.Y() );
                }
            }
        }
    }

    if ( !pLeftRightBorder->GetInWidth() )
    {
        // OD 06.05.2003 #107169# - add 6th parameter
        ::lcl_SubTopBottom( aRect, rBox, _rAttrs, _rFrm, _rRectFn, bPrtOutputDev );
    }

    // OD 29.04.2003 #107169# - paint SwAligned-rectangle
    {
        SwRect aPaintRect( aRect );
        ::SwAlignRect( aPaintRect, _rFrm.GetShell() );
        // if <SwAlignRect> reveals rectangle with no width, adjust rectangle
        // to the prior left postion with width of one twip.
        if ( (aPaintRect.*_rRectFn->fnGetWidth)() == 0 )
        {
            if ( _bLeft )
            {
                (aPaintRect.*_rRectFn->fnSetLeft)( (aRect.*_rRectFn->fnGetLeft)() );
                (aPaintRect.*_rRectFn->fnSetRight)( (aRect.*_rRectFn->fnGetLeft)() );
                (aPaintRect.*_rRectFn->fnAddRight)( 1 );
            }
            else
            {
                (aPaintRect.*_rRectFn->fnSetLeft)( (aRect.*_rRectFn->fnGetRight)() - 1 );
                (aPaintRect.*_rRectFn->fnSetRight)( (aRect.*_rRectFn->fnGetRight)() - 1 );
                (aPaintRect.*_rRectFn->fnAddRight)( 1 );
            }
        }
        _rFrm.PaintBorderLine( _rRect, aPaintRect, &_rPage, &pLeftRightBorder->GetColor() );
    }

    if ( pLeftRightBorder->GetInWidth() )
    {
        const long nDist = ::lcl_MinWidthDist( pLeftRightBorder->GetDistance() );
        long nWidth = ::lcl_AlignWidth( pLeftRightBorder->GetInWidth() );
        if ( _bLeft )
        {
            (aRect.*_rRectFn->fnAddRight)( nDist + nWidth );
            (aRect.*_rRectFn->fnSubLeft)( nWidth - (aRect.*_rRectFn->fnGetWidth)() );
        }
        else
        {
            (aRect.*_rRectFn->fnSubLeft)( nDist + nWidth );
            (aRect.*_rRectFn->fnAddRight)( nWidth - (aRect.*_rRectFn->fnGetWidth)() );
        }
        // OD 06.05.2003 #107169# - add 6th parameter
        ::lcl_SubTopBottom( aRect, rBox, _rAttrs, _rFrm, _rRectFn, bPrtOutputDev );
        // OD 29.04.2003 #107169# - paint SwAligned-rectangle
        {
            SwRect aPaintRect( aRect );
            ::SwAlignRect( aPaintRect, _rFrm.GetShell() );
            // if <SwAlignRect> reveals rectangle with no width, adjust
            // rectangle to the prior left postion with width of one twip.
            if ( (aPaintRect.*_rRectFn->fnGetWidth)() == 0 )
            {
                if ( _bLeft )
                {
                    (aPaintRect.*_rRectFn->fnSetLeft)( (aRect.*_rRectFn->fnGetLeft)() );
                    (aPaintRect.*_rRectFn->fnSetRight)( (aRect.*_rRectFn->fnGetLeft)() );
                    (aPaintRect.*_rRectFn->fnAddRight)( 1 );
                }
                else
                {
                    (aPaintRect.*_rRectFn->fnSetLeft)( (aRect.*_rRectFn->fnGetRight)() - 1 );
                    (aPaintRect.*_rRectFn->fnSetRight)( (aRect.*_rRectFn->fnGetRight)() - 1 );
                    (aPaintRect.*_rRectFn->fnAddRight)( 1 );
                }
            }
            _rFrm.PaintBorderLine( _rRect, aPaintRect, &_rPage, &pLeftRightBorder->GetColor() );
        }
    }
}

// OD 19.05.2003 #109667# - merge <lcl_PaintTopLine> and <lcl_PaintBottomLine>
// into <lcl_PaintTopLine>
void lcl_PaintTopBottomLine( const sal_Bool         _bTop,
                             const SwFrm&           _rFrm,
                             const SwPageFrm&       _rPage,
                             const SwRect&          _rOutRect,
                             const SwRect&          _rRect,
                             const SwBorderAttrs&   _rAttrs,
                             const SwRectFn&        _rRectFn )
{
    const SvxBoxItem& rBox = _rAttrs.GetBox();
    const SvxBorderLine* pTopBottomBorder = 0;
    if ( _bTop )
    {
        pTopBottomBorder = rBox.GetTop();
    }
    else
    {
        pTopBottomBorder = rBox.GetBottom();
    }

    if ( !pTopBottomBorder )
    {
        return;
    }

    SwRect aRect( _rOutRect );
    if ( _bTop )
    {
        (aRect.*_rRectFn->fnAddBottom)( ::lcl_AlignHeight( pTopBottomBorder->GetOutWidth() ) -
                                        (aRect.*_rRectFn->fnGetHeight)() );
    }
    else
    {
        (aRect.*_rRectFn->fnSubTop)( ::lcl_AlignHeight( pTopBottomBorder->GetOutWidth() ) -
                                     (aRect.*_rRectFn->fnGetHeight)() );
    }

    // OD 29.04.2003 #107169# - paint SwAligned-rectangle
    {
        SwRect aPaintRect( aRect );
        ::SwAlignRect( aPaintRect, _rFrm.GetShell() );
        // if <SwAlignRect> reveals rectangle with no width, adjust rectangle
        // to the prior top postion with width of one twip.
        if ( (aPaintRect.*_rRectFn->fnGetHeight)() == 0 )
        {
            if ( _bTop )
            {
                (aPaintRect.*_rRectFn->fnSetTop)( (aRect.*_rRectFn->fnGetTop)() );
                (aPaintRect.*_rRectFn->fnSetBottom)( (aRect.*_rRectFn->fnGetTop)() );
                (aPaintRect.*_rRectFn->fnAddBottom)( 1 );
            }
            else
            {
                (aPaintRect.*_rRectFn->fnSetTop)( (aRect.*_rRectFn->fnGetBottom)() - 1 );
                (aPaintRect.*_rRectFn->fnSetBottom)( (aRect.*_rRectFn->fnGetBottom)() - 1 );
                (aPaintRect.*_rRectFn->fnAddBottom)( 1 );
            }
        }
        _rFrm.PaintBorderLine( _rRect, aPaintRect, &_rPage, &pTopBottomBorder->GetColor() );
    }

    if ( pTopBottomBorder->GetInWidth() )
    {
        const long nDist = ::lcl_MinHeightDist( pTopBottomBorder->GetDistance() );
        const long nHeight = ::lcl_AlignHeight( pTopBottomBorder->GetInWidth() );
        if ( _bTop )
        {
            (aRect.*_rRectFn->fnAddBottom)( nDist + nHeight );
            (aRect.*_rRectFn->fnSubTop)( nHeight - (aRect.*_rRectFn->fnGetHeight)() );
        }
        else
        {
            (aRect.*_rRectFn->fnSubTop)( nDist + nHeight );
            (aRect.*_rRectFn->fnAddBottom)( nHeight -(aRect.*_rRectFn->fnGetHeight)() );
        }
        ::lcl_SubLeftRight( aRect, rBox, _rRectFn );
        // OD 29.04.2003 #107169# - paint SwAligned-rectangle
        {
            SwRect aPaintRect( aRect );
            ::SwAlignRect( aPaintRect, _rFrm.GetShell() );
            // if <SwAlignRect> reveals rectangle with no width, adjust
            // rectangle to the prior top postion with width of one twip.
            if ( (aPaintRect.*_rRectFn->fnGetHeight)() == 0 )
            {
                if ( _bTop )
                {
                    (aPaintRect.*_rRectFn->fnSetTop)( (aRect.*_rRectFn->fnGetTop)() );
                    (aPaintRect.*_rRectFn->fnSetBottom)( (aRect.*_rRectFn->fnGetTop)() );
                    (aPaintRect.*_rRectFn->fnAddBottom)( 1 );
                }
                else
                {
                    (aPaintRect.*_rRectFn->fnSetTop)( (aRect.*_rRectFn->fnGetBottom)() - 1 );
                    (aPaintRect.*_rRectFn->fnSetBottom)( (aRect.*_rRectFn->fnGetBottom)() - 1 );
                    (aPaintRect.*_rRectFn->fnAddBottom)( 1 );
                }
            }
            _rFrm.PaintBorderLine( _rRect, aPaintRect, &_rPage, &pTopBottomBorder->GetColor() );
        }
    }
}


/*************************************************************************
|*
|*  const SwFrm* lcl_HasNextCell( const SwFrm& rFrm )
|*
|* No comment. #i15844#
|*
|*************************************************************************/

const SwFrm* lcl_HasNextCell( const SwFrm& rFrm )
{
    ASSERT( rFrm.IsCellFrm(),
            "lcl_HasNextCell( const SwFrm& rFrm ) should be called with SwCellFrm" )

    const SwFrm* pTmpFrm = &rFrm;
    do
    {
        if ( pTmpFrm->GetNext() )
            return pTmpFrm->GetNext();

        pTmpFrm = pTmpFrm->GetUpper()->GetUpper();
    }
    while ( pTmpFrm->IsCellFrm() );

    return 0;
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

/** local method to determine cell frame, from which the border attributes
    for paint of top/bottom border has to be used.

    OD 21.02.2003 #b4779636#, #107692#

    @author OD


    @param _pCellFrm
    input parameter - constant pointer to cell frame for which the cell frame
    for the border attributes has to be determined.

    @param _rCellBorderAttrs
    input parameter - constant reference to the border attributes of cell frame
    <_pCellFrm>.

    @param _bTop
    input parameter - boolean, that controls, if cell frame for top border or
    for bottom border has to be determined.

    @return constant pointer to cell frame, for which the border attributes has
    to be used
*/
const SwFrm* lcl_GetCellFrmForBorderAttrs( const SwFrm*         _pCellFrm,
                                           const SwBorderAttrs& _rCellBorderAttrs,
                                           const bool           _bTop )
{
    ASSERT( _pCellFrm, "No cell frame available, dying soon" )

    // determine, if cell frame is at bottom/top border of a table frame and
    // the table frame has/is a follow.
    const SwFrm* pTmpFrm = _pCellFrm;
    bool bCellAtBorder = true;
    bool bCellAtLeftBorder = !_pCellFrm->GetPrev();
    bool bCellAtRightBorder = !_pCellFrm->GetNext();
    while( !pTmpFrm->IsRowFrm() || !pTmpFrm->GetUpper()->IsTabFrm() )
    {
        pTmpFrm = pTmpFrm->GetUpper();
        if ( pTmpFrm->IsRowFrm() &&
             (_bTop ? pTmpFrm->GetPrev() : pTmpFrm->GetNext())
           )
        {
            bCellAtBorder = false;
        }
        if ( pTmpFrm->IsCellFrm() )
        {
            if ( pTmpFrm->GetPrev() )
            {
                bCellAtLeftBorder = false;
            }
            if ( pTmpFrm->GetNext() )
            {
                bCellAtRightBorder = false;
            }
        }
    }
    ASSERT( pTmpFrm && pTmpFrm->IsRowFrm(), "No RowFrm available" );

    const SwLayoutFrm* pParentRowFrm = static_cast<const SwLayoutFrm*>(pTmpFrm);
    const SwTabFrm* pParentTabFrm =
            static_cast<const SwTabFrm*>(pParentRowFrm->GetUpper());

    const bool bCellNeedsAttribute = bCellAtBorder &&
                                     ( _bTop ?
                                      // bCellInFirstRowWithMaster
                                       ( !pParentRowFrm->GetPrev() &&
                                         pParentTabFrm->IsFollow() &&
                                         0 == pParentTabFrm->GetTable()->GetRowsToRepeat() ) :
                                      // bCellInLastRowWithFollow
                                       ( !pParentRowFrm->GetNext() &&
                                         pParentTabFrm->GetFollow() )
                                     );

    const SwFrm* pRet = _pCellFrm;
    if ( bCellNeedsAttribute )
    {
        // determine, if cell frame has no borders inside the table.
        const SwFrm* pNextCell = 0;
        bool bNoBordersInside = false;

        if ( bCellAtLeftBorder && ( 0 != ( pNextCell = lcl_HasNextCell( *_pCellFrm ) ) ) )
        {
            SwBorderAttrAccess aAccess( SwFrm::GetCache(), pNextCell );
            const SwBorderAttrs &rBorderAttrs = *aAccess.Get();
            const SvxBoxItem& rBorderBox = rBorderAttrs.GetBox();
            bCellAtRightBorder = !lcl_HasNextCell( *pNextCell );
            bNoBordersInside =
                ( !rBorderBox.GetTop()    || !pParentRowFrm->GetPrev() ) &&
                  !rBorderBox.GetLeft() &&
                ( !rBorderBox.GetRight()  || bCellAtRightBorder ) &&
                ( !rBorderBox.GetBottom() || !pParentRowFrm->GetNext() );
        }
        else
        {
            const SvxBoxItem& rBorderBox = _rCellBorderAttrs.GetBox();
            bNoBordersInside =
                ( !rBorderBox.GetTop()    || !pParentRowFrm->GetPrev() ) &&
                ( !rBorderBox.GetLeft()   || bCellAtLeftBorder ) &&
                ( !rBorderBox.GetRight()  || bCellAtRightBorder ) &&
                ( !rBorderBox.GetBottom() || !pParentRowFrm->GetNext() );
        }

        if ( bNoBordersInside )
        {
            if ( _bTop && !_rCellBorderAttrs.GetBox().GetTop() )
            {
                // #b4779636#-hack:
                // Cell frame has no top border and no border inside the table, but
                // it is at the top border of a table frame, which is a follow.
                // Thus, use border attributes of cell frame in first row of complete table.
                // First, determine first table frame of complete table.
                SwTabFrm* pMasterTabFrm = pParentTabFrm->FindMaster( true );
                // determine first row of complete table.
                const SwFrm* pFirstRow = pMasterTabFrm->GetLower();
                // return first cell in first row
                SwFrm* pTmpFrm = const_cast<SwFrm*>(pFirstRow->GetLower());
                while ( !pTmpFrm->IsCellFrm() ||
                        ( pTmpFrm->GetLower() && pTmpFrm->GetLower()->IsRowFrm() )
                      )
                {
                    pTmpFrm = pTmpFrm->GetLower();
                }
                ASSERT( pTmpFrm && pTmpFrm->IsCellFrm(), "No CellFrm available" );
                pRet = pTmpFrm;
            }
            else if ( !_bTop && !_rCellBorderAttrs.GetBox().GetBottom() )
            {
                // #b4779636#-hack:
                // Cell frame has no bottom border and no border inside the table,
                // but it is at the bottom border of a table frame, which has a follow.
                // Thus, use border attributes of cell frame in last row of complete table.
                // First, determine last table frame of complete table.
                SwTabFrm* pLastTabFrm = const_cast<SwTabFrm*>(pParentTabFrm->GetFollow());
                while ( pLastTabFrm->GetFollow() )
                {
                    pLastTabFrm = pLastTabFrm->GetFollow();
                }
                // determine last row of complete table.
                SwFrm* pLastRow = pLastTabFrm->GetLower();
                while ( pLastRow->GetNext() )
                {
                    pLastRow = pLastRow->GetNext();
                }
                // return first bottom border cell in last row
                SwFrm* pTmpFrm = const_cast<SwFrm*>(pLastRow->GetLower());
                while ( !pTmpFrm->IsCellFrm() ||
                        ( pTmpFrm->GetLower() && pTmpFrm->GetLower()->IsRowFrm() )
                      )
                {
                    if ( pTmpFrm->IsRowFrm() )
                    {
                        while ( pTmpFrm->GetNext() )
                        {
                            pTmpFrm = pTmpFrm->GetNext();
                        }
                    }
                    pTmpFrm = pTmpFrm->GetLower();
                }
                ASSERT( pTmpFrm && pTmpFrm->IsCellFrm(), "No CellFrm available" );
                pRet = pTmpFrm;
            }
        }
    }

    return pRet;
}

void SwFrm::PaintBorder( const SwRect& rRect, const SwPageFrm *pPage,
                         const SwBorderAttrs &rAttrs ) const
{
    //fuer (Row,Body,Ftn,Root,Column,NoTxt) gibt's hier nix zu tun
    if ( (GetType() & 0x90C5) || (Prt().SSize() == Frm().SSize()) )
        return;

    if ( (GetType() & 0x2000) &&    //Cell
         !pGlobalShell->GetViewOptions()->IsTable() )
        return;

    // --> collapsing borders FME 2005-05-27 #i29550#
    if ( IsTabFrm() || IsCellFrm() || IsRowFrm() )
    {
        const SwTabFrm* pTabFrm = FindTabFrm();
        if ( pTabFrm->IsCollapsingBorders() )
            return;
    }
    // <--

    const bool bLine = rAttrs.IsLine() ? true : false;
    const bool bShadow = rAttrs.GetShadow().GetLocation() != SVX_SHADOW_NONE;

    // OD 24.02.2003 #b4779636#, #107692# - flag to control,
    // if #b4779636#-hack has to be used.
    const bool bb4779636HackActive = true;
    // OD 21.02.2003 #b4779636#, #107692#
    const SwFrm* pCellFrmForBottomBorderAttrs = 0;
    const SwFrm* pCellFrmForTopBorderAttrs = 0;
    bool         bFoundCellForTopOrBorderAttrs = false;
    if ( bb4779636HackActive && IsCellFrm() )
    {
        pCellFrmForBottomBorderAttrs = lcl_GetCellFrmForBorderAttrs( this, rAttrs, false );
        if ( pCellFrmForBottomBorderAttrs != this )
            bFoundCellForTopOrBorderAttrs = true;
        pCellFrmForTopBorderAttrs = lcl_GetCellFrmForBorderAttrs( this, rAttrs, true );
        if ( pCellFrmForTopBorderAttrs != this )
            bFoundCellForTopOrBorderAttrs = true;
    }

    // OD 24.02.2003 #b4779636#, #107692# - add condition <bFoundCellForTopOrBorderAttrs>
    // for #b4779636#-hack
    if ( bLine || bShadow || bFoundCellForTopOrBorderAttrs )
    {
        //Wenn das Rechteck vollstandig innerhalb der PrtArea liegt,
        //so braucht kein Rand gepainted werden.
        //Fuer die PrtArea muss der Aligned'e Wert zugrunde gelegt werden,
        //anderfalls wuerden u.U. Teile nicht verarbeitet.
        SwRect aRect( Prt() );
        aRect += Frm().Pos();
        ::SwAlignRect( aRect, pGlobalShell );
        // OD 27.09.2002 #103636# - new local boolean variable in order to
        // suspend border paint under special cases - see below.
        // NOTE: This is a fix for the implementation of feature #99657#.
        bool bDrawOnlyShadowForTransparentFrame = false;
        if ( aRect.IsInside( rRect ) )
        {
            // OD 27.09.2002 #103636# - paint shadow, if background is transparent.
            // Because of introduced transparent background for fly frame #99657#,
            // the shadow have to be drawn if the background is transparent,
            // in spite the fact that the paint rectangle <rRect> lies fully
            // in the printing area.
            // NOTE to chosen solution:
            //     On transparent background, continue processing, but suspend
            //     drawing of border by setting <bDrawOnlyShadowForTransparentFrame>
            //     to true.
            if ( IsLayoutFrm() &&
                 static_cast<const SwLayoutFrm*>(this)->GetFmt()->IsBackgroundTransparent() )
            {
                 bDrawOnlyShadowForTransparentFrame = true;
            }
            else
            {
                return;
            }
        }

        if ( !pPage )
            pPage = FindPageFrm();

        ::lcl_CalcBorderRect( aRect, this, rAttrs, TRUE );
        rAttrs.SetGetCacheLine( TRUE );
        if ( bShadow )
            PaintShadow( rRect, aRect, pPage, rAttrs );
        // OD 27.09.2002 #103636# - suspend drawing of border
        // add condition < NOT bDrawOnlyShadowForTransparentFrame > - see above
        // OD 24.02.2003 #b4779636#, #107692# - add condition <bFoundCellForTopOrBorderAttrs>
        // for #b4779636#-hack.
        if ( ( bLine || bFoundCellForTopOrBorderAttrs ) &&
             !bDrawOnlyShadowForTransparentFrame )
        {
            const SwFrm* pDirRefFrm = IsCellFrm() ? FindTabFrm() : this;
            SWRECTFN( pDirRefFrm )
            // OD 19.05.2003 #109667# - use new method <lcl_PaintLeftRightLine(..)>
            //::lcl_PaintLeftLine  ( this, pPage, aRect, rRect, rAttrs, fnRect );
            //::lcl_PaintRightLine ( this, pPage, aRect, rRect, rAttrs, fnRect );
            ::lcl_PaintLeftRightLine ( sal_True, *(this), *(pPage), aRect, rRect, rAttrs, fnRect );
            ::lcl_PaintLeftRightLine ( sal_False, *(this), *(pPage), aRect, rRect, rAttrs, fnRect );
            if ( !IsCntntFrm() || rAttrs.GetTopLine( *(this) ) )
            {
                // OD 21.02.2003 #b4779636#, #107692# -
                // #b4779636#-hack: If another cell frame for top border
                // paint is found, paint its top border.
                if ( IsCellFrm() && pCellFrmForTopBorderAttrs != this )
                {
                    SwBorderAttrAccess aAccess( SwFrm::GetCache(),
                                                pCellFrmForTopBorderAttrs );
                    const SwBorderAttrs &rTopAttrs = *aAccess.Get();
                    // OD 19.05.2003 #109667# - use new method <lcl_PaintTopBottomLine(..)>
                    //::lcl_PaintTopLine( this, pPage, aRect, rRect, rTopAttrs, fnRect );
                    ::lcl_PaintTopBottomLine( sal_True, *(this), *(pPage), aRect, rRect, rTopAttrs, fnRect );
                }
                else
                {
                    // OD 19.05.2003 #109667# - use new method <lcl_PaintTopBottomLine(..)>
                    //::lcl_PaintTopLine( this, pPage, aRect, rRect, rAttrs, fnRect );
                    ::lcl_PaintTopBottomLine( sal_True, *(this), *(pPage), aRect, rRect, rAttrs, fnRect );
                }
            }
            if ( !IsCntntFrm() || rAttrs.GetBottomLine( *(this) ) )
            {
                // OD 21.02.2003 #b4779636#, #107692# -
                // #b4779636#-hack: If another cell frame for bottom border
                // paint is found, paint its bottom border.
                if ( IsCellFrm() && pCellFrmForBottomBorderAttrs != this )
                {
                    SwBorderAttrAccess aAccess( SwFrm::GetCache(),
                                                pCellFrmForBottomBorderAttrs );
                    const SwBorderAttrs &rBottomAttrs = *aAccess.Get();
                    // OD 19.05.2003 #109667# - use new method <lcl_PaintTopBottomLine(..)>
                    //::lcl_PaintBottomLine(this, pPage, aRect, rRect, rBottomAttrs, fnRect);
                    ::lcl_PaintTopBottomLine(sal_False, *(this), *(pPage), aRect, rRect, rBottomAttrs, fnRect);
                }
                else
                {
                    // OD 19.05.2003 #109667# - use new method <lcl_PaintTopBottomLine(..)>
                    //::lcl_PaintBottomLine(this, pPage, aRect, rRect, rAttrs, fnRect);
                    ::lcl_PaintTopBottomLine(sal_False, *(this), *(pPage), aRect, rRect, rAttrs, fnRect);
                }
            }
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

    SWRECTFN( this )
    SwTwips nPrtWidth = (Prt().*fnRect->fnGetWidth)();
    Fraction aFract( nPrtWidth, 1 );
    const SwTwips nWidth = (long)(aFract *= rInf.GetWidth());

    SwTwips nX = (this->*fnRect->fnGetPrtLeft)();
    switch ( rInf.GetAdj() )
    {
        case FTNADJ_CENTER:
            nX += nPrtWidth/2 - nWidth/2; break;
        case FTNADJ_RIGHT:
            nX += nPrtWidth - nWidth; break;
        case FTNADJ_LEFT:
            /* do nothing */; break;
        default:
            ASSERT( !this, "Neues Adjustment fuer Fussnotenlinie?" );
    }
    SwTwips nLineWidth = rInf.GetLineWidth();
    const SwRect aLineRect = bVert ?
        SwRect( Point(Frm().Left()+Frm().Width()-rInf.GetTopDist()-nLineWidth,
                      nX), Size( nLineWidth, nWidth ) )
            : SwRect( Point( nX, Frm().Pos().Y() + rInf.GetTopDist() ),
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

    SwRectFn fnRect = pCol->IsVertical() ? fnRectVert : fnRectHori;
    SwRect aLineRect = Prt();
    aLineRect += Frm().Pos();

    SwTwips nTop = ((aLineRect.*fnRect->fnGetHeight)()*rFmtCol.GetLineHeight())
                   / 100 - (aLineRect.*fnRect->fnGetHeight)();
    SwTwips nBottom = 0;

    switch ( rFmtCol.GetLineAdj() )
    {
        case COLADJ_CENTER:
            nBottom = nTop / 2; nTop -= nBottom; break;
        case COLADJ_TOP:
            nBottom = nTop; nTop = 0; break;
        case COLADJ_BOTTOM:
            break;
        default:
            ASSERT( !this, "Neues Adjustment fuer Spaltenlinie?" );
    }

    if( nTop )
        (aLineRect.*fnRect->fnSubTop)( nTop );
    if( nBottom )
        (aLineRect.*fnRect->fnAddBottom)( nBottom );

    SwTwips nPenHalf = rFmtCol.GetLineWidth();
    (aLineRect.*fnRect->fnSetWidth)( nPenHalf );
    nPenHalf /= 2;

    //Damit uns nichts verlorengeht muessen wir hier etwas grosszuegiger sein.
    SwRect aRect( rRect );
    (aRect.*fnRect->fnSubLeft)( nPenHalf + nPixelSzW );
    (aRect.*fnRect->fnAddRight)( nPenHalf + nPixelSzW );
    SwRectGet fnGetX = IsRightToLeft() ? fnRect->fnGetLeft : fnRect->fnGetRight;
    while ( pCol->GetNext() )
    {
        (aLineRect.*fnRect->fnSetPosX)
            ( (pCol->Frm().*fnGetX)() - nPenHalf );
        if ( aRect.IsOver( aLineRect ) )
            PaintBorderLine( aRect, aLineRect , pPage, &rFmtCol.GetLineColor());
        pCol = pCol->GetNext();
    }
}

void SwPageFrm::PaintGrid( OutputDevice* pOut, SwRect &rRect ) const
{
    if( !bHasGrid || pRetoucheFly || pRetoucheFly2 )
        return;
    GETGRID( this )
    if( pGrid && ( OUTDEV_PRINTER != pOut->GetOutDevType() ?
        pGrid->GetDisplayGrid() : pGrid->GetPrintGrid() ) )
    {
        const SwLayoutFrm* pBody = FindBodyCont();
        if( pBody )
        {
            SwRect aGrid( pBody->Prt() );
            aGrid += pBody->Frm().Pos();

            SwRect aInter( aGrid );
            aInter.Intersection( rRect );
            if( aInter.HasArea() )
            {
                BOOL bGrid = pGrid->GetRubyTextBelow();
                BOOL bCell = GRID_LINES_CHARS == pGrid->GetGridType();
                long nGrid = pGrid->GetBaseHeight();
                long nRuby = pGrid->GetRubyHeight();
                long nSum = nGrid + nRuby;
                const Color *pCol = &pGrid->GetColor();

                SwTwips nRight = aInter.Left() + aInter.Width();
                SwTwips nBottom = aInter.Top() + aInter.Height();
                if( IsVertical() )
                {
                    SwTwips nOrig = aGrid.Left() + aGrid.Width();
                    SwTwips nY = nOrig + nSum *
                                 ( ( nOrig - aInter.Left() ) / nSum );
                    SwRect aTmp( Point( nY, aInter.Top() ),
                                Size( 1, aInter.Height() ) );
                    SwTwips nX = aGrid.Top() + nGrid *
                                ( ( aInter.Top() - aGrid.Top() )/ nGrid );
                    if( nX < aInter.Top() )
                        nX += nGrid;
                    SwTwips nGridBottom = aGrid.Top() + aGrid.Height();
                    BOOL bLeft = aGrid.Top() >= aInter.Top();
                    BOOL bRight = nGridBottom <= nBottom;
                    BOOL bBorder = bLeft || bRight;
                    while( nY > nRight )
                    {
                        aTmp.Pos().X() = nY;
                        if( bGrid )
                        {
                            nY -= nGrid;
                            SwTwips nPosY = Max( aInter.Left(), nY );
                            SwTwips nHeight = Min(nRight, aTmp.Pos().X())-nPosY;
                            if( nHeight > 0 )
                            {
                                if( bCell )
                                {
                                    SwRect aVert( Point( nPosY, nX ),
                                                Size( nHeight, 1 ) );
                                    while( aVert.Top() <= nBottom )
                                    {
                                        PaintBorderLine(rRect,aVert,this,pCol);
                                        aVert.Pos().Y() += nGrid;
                                    }
                                }
                                else if( bBorder )
                                {
                                    SwRect aVert( Point( nPosY, aGrid.Top() ),
                                                  Size( nHeight, 1 ) );
                                    if( bLeft )
                                        PaintBorderLine(rRect,aVert,this,pCol);
                                    if( bRight )
                                    {
                                        aVert.Pos().Y() = nGridBottom;
                                        PaintBorderLine(rRect,aVert,this,pCol);
                                    }
                                }
                            }
                        }
                        else
                        {
                            nY -= nRuby;
                            if( bBorder )
                            {
                                SwTwips nPos = Max( aInter.Left(), nY );
                                SwTwips nW = Min(nRight, aTmp.Pos().X()) - nPos;
                                SwRect aVert( Point( nPos, aGrid.Top() ),
                                              Size( nW, 1 ) );
                                if( nW > 0 )
                                {
                                    if( bLeft )
                                        PaintBorderLine(rRect,aVert,this,pCol);
                                    if( bRight )
                                    {
                                        aVert.Pos().Y() = nGridBottom;
                                        PaintBorderLine(rRect,aVert,this,pCol);
                                    }
                                }
                            }
                        }
                        bGrid = !bGrid;
                    }
                    while( nY >= aInter.Left() )
                    {
                        aTmp.Pos().X() = nY;
                        PaintBorderLine( rRect, aTmp, this, pCol);
                        if( bGrid )
                        {
                            nY -= nGrid;
                            SwTwips nHeight = aTmp.Pos().X()
                                              - Max(aInter.Left(), nY );
                            if( nHeight > 0 )
                            {
                                if( bCell )
                                {
                                    SwRect aVert( Point(aTmp.Pos().X()-nHeight,
                                                  nX ), Size( nHeight, 1 ) );
                                    while( aVert.Top() <= nBottom )
                                    {
                                        PaintBorderLine(rRect,aVert,this,pCol);
                                        aVert.Pos().Y() += nGrid;
                                    }
                                }
                                else if( bBorder )
                                {
                                    SwRect aVert( Point(aTmp.Pos().X()-nHeight,
                                            aGrid.Top() ), Size( nHeight, 1 ) );
                                    if( bLeft )
                                        PaintBorderLine(rRect,aVert,this,pCol);
                                    if( bRight )
                                    {
                                        aVert.Pos().Y() = nGridBottom;
                                        PaintBorderLine(rRect,aVert,this,pCol);
                                    }
                                }
                            }
                        }
                        else
                        {
                            nY -= nRuby;
                            if( bBorder )
                            {
                                SwTwips nPos = Max( aInter.Left(), nY );
                                SwTwips nW = Min(nRight, aTmp.Pos().X()) - nPos;
                                SwRect aVert( Point( nPos, aGrid.Top() ),
                                              Size( nW, 1 ) );
                                if( nW > 0 )
                                {
                                    if( bLeft )
                                        PaintBorderLine(rRect,aVert,this,pCol);
                                    if( bRight )
                                    {
                                        aVert.Pos().Y() = nGridBottom;
                                        PaintBorderLine(rRect,aVert,this,pCol);
                                    }
                                }
                            }
                        }
                        bGrid = !bGrid;
                    }
                }
                else
                {
                    SwTwips nOrig = aGrid.Top();
                    SwTwips nY = nOrig + nSum *( (aInter.Top()-nOrig)/nSum );
                    SwRect aTmp( Point( aInter.Left(), nY ),
                                Size( aInter.Width(), 1 ) );
                    SwTwips nX = aGrid.Left() + nGrid *
                                ( ( aInter.Left() - aGrid.Left() )/ nGrid );
                    if( nX < aInter.Left() )
                        nX += nGrid;
                    SwTwips nGridRight = aGrid.Left() + aGrid.Width();
                    BOOL bLeft = aGrid.Left() >= aInter.Left();
                    BOOL bRight = nGridRight <= nRight;
                    BOOL bBorder = bLeft || bRight;
                    while( nY < aInter.Top() )
                    {
                        aTmp.Pos().Y() = nY;
                        if( bGrid )
                        {
                            nY += nGrid;
                            SwTwips nPosY = Max( aInter.Top(), aTmp.Pos().Y() );
                            SwTwips nHeight = Min(nBottom, nY ) - nPosY;
                            if( nHeight )
                            {
                                if( bCell )
                                {
                                    SwRect aVert( Point( nX, nPosY ),
                                                Size( 1, nHeight ) );
                                    while( aVert.Left() <= nRight )
                                    {
                                        PaintBorderLine(rRect,aVert,this,pCol);
                                        aVert.Pos().X() += nGrid;
                                    }
                                }
                                else if ( bBorder )
                                {
                                    SwRect aVert( Point( aGrid.Left(), nPosY ),
                                                Size( 1, nHeight ) );
                                    if( bLeft )
                                        PaintBorderLine(rRect,aVert,this,pCol);
                                    if( bRight )
                                    {
                                        aVert.Pos().X() = nGridRight;
                                        PaintBorderLine(rRect,aVert,this,pCol);
                                    }
                                }
                            }
                        }
                        else
                        {
                            nY += nRuby;
                            if( bBorder )
                            {
                                SwTwips nPos = Max(aInter.Top(),aTmp.Pos().Y());
                                SwTwips nH = Min( nBottom, nY ) - nPos;
                                SwRect aVert( Point( aGrid.Left(), nPos ),
                                            Size( 1, nH ) );
                                if( nH > 0 )
                                {
                                    if( bLeft )
                                        PaintBorderLine(rRect,aVert,this,pCol);
                                    if( bRight )
                                    {
                                        aVert.Pos().X() = nGridRight;
                                        PaintBorderLine(rRect,aVert,this,pCol);
                                    }
                                }
                            }
                        }
                        bGrid = !bGrid;
                    }
                    while( nY <= nBottom )
                    {
                        aTmp.Pos().Y() = nY;
                        PaintBorderLine( rRect, aTmp, this, pCol);
                        if( bGrid )
                        {
                            nY += nGrid;
                            SwTwips nHeight = Min(nBottom, nY) - aTmp.Pos().Y();
                            if( nHeight )
                            {
                                if( bCell )
                                {
                                    SwRect aVert( Point( nX, aTmp.Pos().Y() ),
                                                Size( 1, nHeight ) );
                                    while( aVert.Left() <= nRight )
                                    {
                                        PaintBorderLine( rRect, aVert, this, pCol);
                                        aVert.Pos().X() += nGrid;
                                    }
                                }
                                else if( bBorder )
                                {
                                    SwRect aVert( Point( aGrid.Left(),
                                        aTmp.Pos().Y() ), Size( 1, nHeight ) );
                                    if( bLeft )
                                        PaintBorderLine(rRect,aVert,this,pCol);
                                    if( bRight )
                                    {
                                        aVert.Pos().X() = nGridRight;
                                        PaintBorderLine(rRect,aVert,this,pCol);
                                    }
                                }
                            }
                        }
                        else
                        {
                            nY += nRuby;
                            if( bBorder )
                            {
                                SwTwips nPos = Max(aInter.Top(),aTmp.Pos().Y());
                                SwTwips nH = Min( nBottom, nY ) - nPos;
                                SwRect aVert( Point( aGrid.Left(), nPos ),
                                            Size( 1, nH ) );
                                if( nH > 0 )
                                {
                                    if( bLeft )
                                        PaintBorderLine(rRect,aVert,this,pCol);
                                    if( bRight )
                                    {
                                        aVert.Pos().X() = nGridRight;
                                        PaintBorderLine(rRect,aVert,this,pCol);
                                    }
                                }
                            }
                        }
                        bGrid = !bGrid;
                    }
                }
            }
        }
    }
}

/** paint margin area of a page

    OD 20.11.2002 for #104598#:
    implement paint of margin area; margin area will be painted for a
    view shell with a window and if the document is not in online layout.

    @author OD

    @param _rOutputRect
    input parameter - constant instance reference of the rectangle, for
    which an output has to be generated.

    @param _pViewShell
    input parameter - instance of the view shell, on which the output
    has to be generated.
*/
void SwPageFrm::PaintMarginArea( const SwRect& _rOutputRect,
                                 ViewShell* _pViewShell ) const
{
    if ( _pViewShell->GetWin() &&
         !_pViewShell->GetDoc()->IsBrowseMode()
       )
    {
        SwRect aPgPrtRect( Prt() );
        aPgPrtRect.Pos() += Frm().Pos();
        if ( !aPgPrtRect.IsInside( _rOutputRect ) )
        {
            SwRect aPgRect = Frm();
            aPgRect._Intersection( _rOutputRect );
            SwRegionRects aPgRegion( aPgRect );
            aPgRegion -= aPgPrtRect;
            const SwPageFrm* pPage = static_cast<const SwPageFrm*>(this);
            if ( pPage->GetSortedObjs() )
                ::lcl_SubtractFlys( this, pPage, aPgRect, aPgRegion );
            if ( aPgRegion.Count() )
            {
                OutputDevice *pOut = _pViewShell->GetOut();
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
}

// ----------------------------------------------------------------------
//
// const SwPageFrm::mnBorderPxWidth, const SwPageFrm::mnShadowPxWidth
// SwPageFrm::GetBorderRect (..), SwPageFrm::GetRightShadowRect(..),
// SwPageFrm::GetBottomShadowRect(..),
// SwPageFrm::PaintBorderAndShadow(..),
// SwPageFrm::GetBorderAndShadowBoundRect(..)
//
// OD 12.02.2003 for #i9719# and #105645#
// ----------------------------------------------------------------------

const sal_Int8 SwPageFrm::mnBorderPxWidth = 1;
const sal_Int8 SwPageFrm::mnShadowPxWidth = 2;

/** determine rectangle for page border

    OD 12.02.2003 for #i9719# and #105645#

    @author OD
*/
void SwPageFrm::GetBorderRect( const SwRect& _rPageRect,
                               ViewShell*    _pViewShell,
                               SwRect& _orBorderRect ) const
{
    SwRect aAlignedPageRect( _rPageRect );
    SwAlignRect( aAlignedPageRect, _pViewShell );
    Rectangle aBorderPxRect =
            _pViewShell->GetOut()->LogicToPixel( aAlignedPageRect.SVRect() );

    aBorderPxRect.Left() = aBorderPxRect.Left() - mnBorderPxWidth;
    aBorderPxRect.Top() = aBorderPxRect.Top() - mnBorderPxWidth;
    aBorderPxRect.Right() = aBorderPxRect.Right() + mnBorderPxWidth;
    aBorderPxRect.Bottom() = aBorderPxRect.Bottom() + mnBorderPxWidth;

    _orBorderRect =
            SwRect( _pViewShell->GetOut()->PixelToLogic( aBorderPxRect ) );
}

/** determine rectangle for right page shadow

    OD 12.02.2003 for #i9719# and #105645#

    @author OD
*/
void SwPageFrm::GetRightShadowRect( const SwRect& _rPageRect,
                                    ViewShell*    _pViewShell,
                                    SwRect&       _orRightShadowRect ) const
{
    SwRect aAlignedPageRect( _rPageRect );
    SwAlignRect( aAlignedPageRect, _pViewShell );
    Rectangle aPagePxRect =
            _pViewShell->GetOut()->LogicToPixel( aAlignedPageRect.SVRect() );

    Rectangle aRightShadowPxRect(
                    aPagePxRect.Right() + mnShadowPxWidth,
                    aPagePxRect.Top() + 1,
                    aPagePxRect.Right() + mnBorderPxWidth + mnShadowPxWidth,
                    aPagePxRect.Bottom() + mnBorderPxWidth + mnShadowPxWidth );

    _orRightShadowRect =
            SwRect( _pViewShell->GetOut()->PixelToLogic( aRightShadowPxRect ) );
}

/** determine rectangle for bottom page shadow

    OD 12.02.2003 for #i9719# and #105645#

    @author OD
*/
void SwPageFrm::GetBottomShadowRect( const SwRect& _rPageRect,
                                     ViewShell*    _pViewShell,
                                     SwRect&       _orBottomShadowRect ) const
{
    SwRect aAlignedPageRect( _rPageRect );
    SwAlignRect( aAlignedPageRect, _pViewShell );
    Rectangle aPagePxRect =
            _pViewShell->GetOut()->LogicToPixel( aAlignedPageRect.SVRect() );

    Rectangle aBottomShadowPxRect(
                    aPagePxRect.Left() + 1,
                    aPagePxRect.Bottom() + mnShadowPxWidth,
                    aPagePxRect.Right() + mnBorderPxWidth + mnShadowPxWidth,
                    aPagePxRect.Bottom() + mnBorderPxWidth + mnShadowPxWidth );

    _orBottomShadowRect =
            SwRect( _pViewShell->GetOut()->PixelToLogic( aBottomShadowPxRect ) );
}

/** paint page border and shadow

    OD 12.02.2003 for #i9719# and #105645#
    implement paint of page border and shadow

    @author OD
*/
void SwPageFrm::PaintBorderAndShadow( const SwRect& _rPageRect,
                                      ViewShell*    _pViewShell ) const
{
    // get color for page border and shadow paint
    const Color& rColor = SwViewOption::GetFontColor();

    // save current fill and line color of output device
    Color aFill( _pViewShell->GetOut()->GetFillColor() );
    Color aLine( _pViewShell->GetOut()->GetLineColor() );

    // paint page border
    _pViewShell->GetOut()->SetFillColor(); // OD 20.02.2003 #107369# - no fill color
    _pViewShell->GetOut()->SetLineColor( rColor );
    SwRect aPaintRect;
    GetBorderRect( _rPageRect, _pViewShell, aPaintRect );
    _pViewShell->GetOut()->DrawRect( aPaintRect.SVRect() );

    // paint shadow
    _pViewShell->GetOut()->SetFillColor( rColor );
    GetRightShadowRect( _rPageRect, _pViewShell, aPaintRect );
    _pViewShell->GetOut()->DrawRect( aPaintRect.SVRect() );
    GetBottomShadowRect( _rPageRect, _pViewShell, aPaintRect );
    _pViewShell->GetOut()->DrawRect( aPaintRect.SVRect() );

    _pViewShell->GetOut()->SetFillColor( aFill );
    _pViewShell->GetOut()->SetLineColor( aLine );

}

/** get bound rectangle of border and shadow for repaints

    OD 12.02.2003 for #i9719# and #105645#

    author OD
*/
void SwPageFrm::GetBorderAndShadowBoundRect(
                                const SwRect& _rPageRect,
                                ViewShell*    _pViewShell,
                                SwRect& _orBorderAndShadowBoundRect ) const
{
    SwRect aTmpRect;
    GetBorderRect( _rPageRect, _pViewShell, _orBorderAndShadowBoundRect );
    GetRightShadowRect( _rPageRect, _pViewShell, aTmpRect );
    _orBorderAndShadowBoundRect.Union( aTmpRect );
    GetBottomShadowRect( _rPageRect, _pViewShell, aTmpRect );
    _orBorderAndShadowBoundRect.Union( aTmpRect );
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
    // OD 2004-04-23 #116347#
    pOut->Push( PUSH_FILLCOLOR|PUSH_LINECOLOR );
    pOut->SetLineColor();

    SwBorderAttrAccess aAccess( SwFrm::GetCache(), (SwFrm*)this );
    const SwBorderAttrs &rAttrs = *aAccess.Get();

    // OD 20.11.2002 #104598# - take care of page margin area
    // Note: code move from <SwFrm::PaintBackground(..)> to new method
    // <SwPageFrm::Paintmargin(..)>.
    if ( IsPageFrm() )
    {
        static_cast<const SwPageFrm*>(this)->PaintMarginArea( rRect, pGlobalShell );
    }

    // OD 06.08.2002 #99657# - paint border before painting background
    // paint grid for page frame and paint border
    {
        SwRect aRect( rRect );
        if( IsPageFrm() )
            ((SwPageFrm*)this)->PaintGrid( pOut, aRect );
        PaintBorder( aRect, pPage, rAttrs );
    }

    // paint background
    {
        PaintBackground( rRect, pPage, rAttrs, FALSE, bLowerBorder );
    }

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
/// OD 05.09.2002 #102912#
/// Do not paint background for fly frames without a background brush by
/// calling <PaintBaBo> at the page or at the fly frame its anchored
void SwFrm::PaintBackground( const SwRect &rRect, const SwPageFrm *pPage,
                              const SwBorderAttrs & rAttrs,
                             const BOOL bLowerMode,
                             const BOOL bLowerBorder ) const
{
    // OD 20.01.2003 #i1837# - no paint of table background, if corresponding
    // option is *not* set.
    if( IsTabFrm() &&
        !pGlobalShell->GetViewOptions()->IsTable() )
    {
        return;
    }

    ViewShell *pSh = pGlobalShell;
    const FASTBOOL bWin = pSh->GetWin() ? TRUE : FALSE;
    const SvxBrushItem* pItem;
    /// OD 05.09.2002 #102912#
    /// temporary background brush for a fly frame without a background brush
    SvxBrushItem* pTmpBackBrush = 0;
    const Color* pCol;
    SwRect aOrigBackRect;
    const FASTBOOL bPageFrm = IsPageFrm();
    FASTBOOL bLowMode = TRUE;

    FASTBOOL bBack = GetBackgroundBrush( pItem, pCol, aOrigBackRect, bLowerMode );
    //- Ausgabe wenn ein eigener Hintergrund mitgebracht wird.
    bool bNoFlyBackground = !bFlyMetafile && !bBack && IsFlyFrm();
    if ( bNoFlyBackground )
    {
        // OD 05.09.2002 #102912# - Fly frame has no background.
        // Try to find background brush at parents, if previous call of
        // <GetBackgroundBrush> disabled this option with the parameter <bLowerMode>
        if ( bLowerMode )
        {
            bBack = GetBackgroundBrush( pItem, pCol, aOrigBackRect, false );
        }
        // If still no background found for the fly frame, initialize the
        // background brush <pItem> with global retouche color and set <bBack>
        // to TRUE, that fly frame will paint its background using this color.
        if ( !bBack )
        {
            // OD 10.01.2003 #i6467# - on print output, pdf output and
            // in embedded mode not editing color COL_WHITE is used instead of
            // the global retouche color.
            if ( pSh->GetOut()->GetOutDevType() == OUTDEV_PRINTER ||
                 pSh->GetViewOptions()->IsPDFExport() ||
                 ( pSh->GetDoc()->GetDocShell()->GetCreateMode() == SFX_CREATE_MODE_EMBEDDED &&
                   !pSh->GetDoc()->GetDocShell()->IsInPlaceActive()
                 )
               )
            {
                pTmpBackBrush = new SvxBrushItem( Color( COL_WHITE ) );
            }
            else
            {
                pTmpBackBrush = new SvxBrushItem( aGlobalRetoucheColor );
            }
            pItem = pTmpBackBrush;
            bBack = true;
        }
    }

    SwRect aPaintRect( Frm() );
    if( IsTxtFrm() || IsSctFrm() )
        aPaintRect = UnionFrm( TRUE );

    if ( aPaintRect.IsOver( rRect ) )
    {
        if ( bBack || bPageFrm || !bLowerMode )
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

                {
                    /// OD 06.08.2002 #99657# - determine, if background transparency
                    ///     have to be considered for drawing.
                    ///     --> Status Quo: background transparency have to be
                    ///        considered for fly frames
                    const sal_Bool bConsiderBackgroundTransparency = IsFlyFrm();
                    for ( USHORT i = 0; i < aRegion.Count(); ++i )
                    {
                        if ( 1 < aRegion.Count() )
                        {
                            ::SwAlignRect( aRegion[i], pGlobalShell );
                            if( !aRegion[i].HasArea() )
                                continue;
                        }
                        /// OD 06.08.2002 #99657# - add 6th parameter to indicate, if
                        ///     background transparency have to be considered
                        ///     Set missing 5th parameter to the default value GRFNUM_NO
                        ///         - see declaration in /core/inc/frmtool.hxx.
                        ::DrawGraphic( pItem, pOut, aOrigBackRect, aRegion[i], GRFNUM_NO,
                                bConsiderBackgroundTransparency );
                    }
                }
                if( pCol )
                    delete pNewItem;
            }
        }
        else
            bLowMode = bLowerMode ? TRUE : FALSE;
    }

    /// OD 05.09.2002 #102912#
    /// delete temporary background brush.
    delete pTmpBackBrush;

    //Jetzt noch Lower und dessen Nachbarn.
    //Wenn ein Frn dabei die Kette verlaesst also nicht mehr Lower von mir ist
    //so hoert der Spass auf.
    const SwFrm *pFrm = GetLower();
    if ( pFrm )
    {
        SwRect aFrmRect;
        SwRect aRect( PaintArea() );
        aRect._Intersection( rRect );
        SwRect aBorderRect( aRect );
        SwShortCut aShortCut( *pFrm, aBorderRect );
        do
        {   if ( pProgress )
                pProgress->Reschedule();

            aFrmRect = pFrm->PaintArea();
            if ( aFrmRect.IsOver( aBorderRect ) )
            {
                SwBorderAttrAccess aAccess( SwFrm::GetCache(), (SwFrm*)pFrm );
                const SwBorderAttrs &rAttrs = *aAccess.Get();
                /// OD 06.08.2002 #99657# - paint border before painting background
                if ( bLowerBorder )
                    pFrm->PaintBorder( aBorderRect, pPage, rAttrs );
                if ( ( pFrm->IsLayoutFrm() && bLowerBorder ) ||
                     aFrmRect.IsOver( aRect ) )
                    pFrm->PaintBackground( aRect, pPage, rAttrs, bLowMode,
                                           bLowerBorder );
            }
            pFrm = pFrm->GetNext();
        } while ( pFrm && pFrm->GetUpper() == this &&
                  !aShortCut.Stop( aFrmRect ) );
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
    if ( IS_SUBS || IS_SUBS_TABLE || IS_SUBS_SECTION || IS_SUBS_FLYS )
    {
        SwRect aRect( rRect );
        // OD 18.02.2003 #104989# - Not necessary and incorrect alignment of
        // the output rectangle.
        //::SwAlignRect( aRect, pGlobalShell );
        if ( aRect.HasArea() )
        {
            //Beim Paint ueber die Root wird das Array von dort gesteuert.
            //Anderfalls kuemmern wir uns selbst darum.
            FASTBOOL bDelSubs = FALSE;
            if ( !pSubsLines )
            {
                pSubsLines = new SwSubsRects;
                // OD 20.12.2002 #106318# - create container for special subsidiary lines
                pSpecSubsLines = new SwSubsRects;
                bDelSubs = TRUE;
            }

            RefreshLaySubsidiary( this, aRect );

            if ( bDelSubs )
            {
                // OD 20.12.2002 #106318# - paint special subsidiary lines
                // and delete its container
                pSpecSubsLines->PaintSubsidiary( pGlobalShell->GetOut(), NULL );
                DELETEZ( pSpecSubsLines );

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
    const FASTBOOL bSubsFly   = IS_SUBS_FLYS &&
                                (GetType() & FRM_FLY) &&
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
    if( !pLow )
        return;
    SwShortCut aShortCut( *pLow, rRect );
    while( pLow && !aShortCut.Stop( pLow->Frm() ) )
    {
        if ( pLow->Frm().IsOver( rRect ) && pLow->Frm().HasArea() )
        {
            if ( pLow->IsLayoutFrm() )
                ((const SwLayoutFrm*)pLow)->RefreshLaySubsidiary( pPage, rRect);
            else if ( pLow->GetDrawObjs() )
            {
                const SwSortedObjs& rObjs = *(pLow->GetDrawObjs());
                for ( sal_uInt32 i = 0; i < rObjs.Count(); ++i )
                {
                    const SwAnchoredObject* pAnchoredObj = rObjs[i];
                    if ( pPage->GetFmt()->GetDoc()->IsVisibleLayerId(
                                    pAnchoredObj->GetDrawObj()->GetLayer() ) &&
                         pAnchoredObj->ISA(SwFlyFrm) )
                    {
                        const SwFlyFrm *pFly =
                                    static_cast<const SwFlyFrm*>(pAnchoredObj);
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
PointPtr pX = &Point::nA;
PointPtr pY = &Point::nB;
SizePtr pWidth = &Size::nA;
SizePtr pHeight = &Size::nB;

// OD 18.11.2002 #99672# - new parameter <_pSubsLines>
void MA_FASTCALL lcl_RefreshLine( const SwLayoutFrm *pLay,
                                  const SwPageFrm *pPage,
                                  const Point &rP1,
                                  const Point &rP2,
                                  const BYTE nSubColor,
                                  SwLineRects* _pSubsLines )
{
    //In welche Richtung gehts? Kann nur Horizontal oder Vertikal sein.
    ASSERT( ((rP1.X() == rP2.X()) || (rP1.Y() == rP2.Y())),
            "Schraege Hilfslinien sind nicht erlaubt." );
    const PTPTR pDirPt = rP1.X() == rP2.X() ? pY : pX;
    const PTPTR pOthPt = pDirPt == pX ? pY : pX;
    const SIZEPTR pDirSz = pDirPt == pX ? pWidth : pHeight;
    const SIZEPTR pOthSz = pDirSz == pWidth ? pHeight : pWidth;
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
            while ( 0 != (pMyFly = pMyFly->GetAnchorFrm()->FindFlyFrm()) )
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
            {
                aIter.Next();
                continue;
            }

            // OD 19.12.2002 #106318# - do *not* consider fly frames with
            // a transparent background.
            // OD 2004-02-12 #110582#-2 - do *not* consider fly frame, which
            // belongs to a invisible layer
            if ( pFly->IsBackgroundTransparent() ||
                 !pFly->GetFmt()->GetDoc()->IsVisibleLayerId( pObj->GetLayer() ) )
            {
                aIter.Next();
                continue;
            }

            //Sitzt das Obj auf der Linie
            const Rectangle &rBound = pObj->GetCurrentBoundRect();
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
            // OD 18.11.2002 #99672# - use parameter <_pSubsLines> instead of
            // global variable <pSubsLines>.
            _pSubsLines->AddLineRect( aRect, 0, 0, nSubColor );
        }
        aP1 = aP2;
        aP1.*pDirPt += 1;
        aP2 = rP2;
    }
}

void SwLayoutFrm::PaintSubsidiaryLines( const SwPageFrm *pPage,
                                        const SwRect &rRect ) const
{
    // --> collapsing borders FME 2005-05-27 #i29550#
    if ( IsTabFrm() || IsCellFrm() || IsRowFrm() )
    {
        const SwTabFrm* pTabFrm = FindTabFrm();
        if ( pTabFrm->IsCollapsingBorders() )
            return;
    }
    // <-- collapsing

    const bool bFlys = pPage->GetSortedObjs() ? true : false;

    const bool bCell = IsCellFrm() ? true : false;
    // use frame area for cells
    // OD 13.02.2003 #i3662# - for section use also frame area
    const bool bUseFrmArea = bCell || IsSctFrm();
    SwRect aOriginal( bUseFrmArea ? Frm() : Prt() );
    if ( !bUseFrmArea )
        aOriginal.Pos() += Frm().Pos();

    // OD 13.02.2003 #i3662# - enlarge top of column body frame's printing area
    // in sections to top of section frame.
    const bool bColBodyInSection = IsBodyFrm() &&
                                   !IsPageBodyFrm() &&
                                   GetUpper()->GetUpper()->IsSctFrm();
    if ( bColBodyInSection )
    {
        if ( IsVertical() )
            aOriginal.Right( GetUpper()->GetUpper()->Frm().Right() );
        else
            aOriginal.Top( GetUpper()->GetUpper()->Frm().Top() );
    }

    ::SwAlignRect( aOriginal, pGlobalShell );

    if ( !aOriginal.IsOver( rRect ) )
        return;

    SwRect aOut( aOriginal );
    aOut._Intersection( rRect );
    // OD 13.02.2003 #i3662# - do not intersect *enlarged* column body frame's
    // printing area with the paint area of the body frame. Otherwise enlargement
    // will get lost.
    if ( !bColBodyInSection )
    {
        aOut.Intersection( PaintArea() );
    }

    const SwTwips nRight = aOut.Right();
    const SwTwips nBottom= aOut.Bottom();

    const Point aRT( nRight, aOut.Top() );
    const Point aRB( nRight, nBottom );
    const Point aLB( aOut.Left(), nBottom );

    BYTE nSubColor = ( bCell || IsRowFrm() ) ? SUBCOL_TAB :
                     ( IsInSct() ? SUBCOL_SECT :
                     ( IsInFly() ? SUBCOL_FLY : SUBCOL_PAGE ) );

    // OD 05.11.2002 #102406# - body frames are responsible for page/column breaks.
    BOOL bBreak = FALSE;
    if ( IsBodyFrm() )
    {
        const SwCntntFrm *pCnt = ContainsCntnt();
        if ( pCnt )
        {
            // OD 05.11.2002 #102406# - adjust setting of <bBreak>.
            bBreak = pCnt->IsPageBreak( TRUE ) ||
                     ( IsColBodyFrm() && pCnt->IsColBreak( TRUE ) );
        }
    }

    // OD 18.11.2002 #99672# - collect body, header, footer, footnote and section
    // sub-lines in <pSpecSubsLine> array.
    const bool bSpecialSublines = IsBodyFrm() || IsHeaderFrm() || IsFooterFrm() ||
                                  IsFtnFrm() || IsSctFrm();
    SwLineRects* pUsedSubsLines = bSpecialSublines ? pSpecSubsLines : pSubsLines;

    // NOTE: for cell frames only left and right (horizontal layout) respectively
    //      top and bottom (vertical layout) lines painted.
    const bool bVert = IsVertical() ? true : false;
    if ( bFlys )
    {
        // OD 14.11.2002 #104822# - add control for drawing left and right lines
        if ( !bCell || !bVert )
        {
            if ( aOriginal.Left() == aOut.Left() )
                ::lcl_RefreshLine( this, pPage, aOut.Pos(), aLB, nSubColor,
                                   pUsedSubsLines );
            // OD 14.11.2002 #104821# - in vertical layout set page/column break at right
            if ( aOriginal.Right() == nRight )
                ::lcl_RefreshLine( this, pPage, aRT, aRB,
                                   (bBreak && bVert) ? SUBCOL_BREAK : nSubColor,
                                   pUsedSubsLines );
        }
        // OD 14.11.2002 #104822# - adjust control for drawing top and bottom lines
        if ( !bCell || bVert )
        {
            if ( aOriginal.Top() == aOut.Top() )
                // OD 14.11.2002 #104821# - in horizontal layout set page/column break at top
                ::lcl_RefreshLine( this, pPage, aOut.Pos(), aRT,
                                   (bBreak && !bVert) ? SUBCOL_BREAK : nSubColor,
                                   pUsedSubsLines );
            if ( aOriginal.Bottom() == nBottom )
                ::lcl_RefreshLine( this, pPage, aLB, aRB, nSubColor,
                                   pUsedSubsLines );
        }
    }
    else
    {
        // OD 14.11.2002 #104822# - add control for drawing left and right lines
        if ( !bCell || !IsVertical() )
        {
            if ( aOriginal.Left() == aOut.Left() )
            {
                SwRect aRect( aOut.Pos(), aLB );
                pUsedSubsLines->AddLineRect( aRect, 0, 0, nSubColor );
            }
            // OD 14.11.2002 #104821# - in vertical layout set page/column break at right
            if ( aOriginal.Right() == nRight )
            {
                SwRect aRect( aRT, aRB );
                pUsedSubsLines->AddLineRect( aRect, 0, 0,
                        (bBreak && bVert) ? SUBCOL_BREAK : nSubColor );
            }
        }
        // OD 14.11.2002 #104822# - adjust control for drawing top and bottom lines
        if ( !bCell || IsVertical() )
        {
            if ( aOriginal.Top() == aOut.Top() )
            {
                // OD 14.11.2002 #104821# - in horizontal layout set page/column break at top
                SwRect aRect( aOut.Pos(), aRT );
                pUsedSubsLines->AddLineRect( aRect, 0, 0,
                        (bBreak && !bVert) ? SUBCOL_BREAK : nSubColor );
            }
            if ( aOriginal.Bottom() == nBottom )
            {
                SwRect aRect( aLB, aRB );
                pUsedSubsLines->AddLineRect( aRect, 0, 0, nSubColor );
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
                const SwAnchoredObject* pAnchoredObj = (*GetSortedObjs())[i];
                if ( pAnchoredObj->ISA(SwFlyFrm) )
                {
                    const SwFlyFrm *pFly = static_cast<const SwFlyFrm*>(pAnchoredObj);
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
            for ( sal_uInt32 i = 0; i < pCnt->GetDrawObjs()->Count(); ++i )
            {
                const SwAnchoredObject* pAnchoredObj = (*pCnt->GetDrawObjs())[i];
                if ( pAnchoredObj->ISA(SwFlyFrm) )
                {
                    const SwFlyFrm *pFly = static_cast<const SwFlyFrm*>(pAnchoredObj);
                    if ( pFly->IsFlyInCntFrm() &&
                         pFly->Frm().Top() <= rRect.Bottom() &&
                         pFly->Frm().Bottom() >= rRect.Top() )
                        pFly->RefreshExtraData( rRect );
                }
        }
        pCnt = pCnt->GetNextCntntFrm();
    }
}

/** SwPageFrm::GetDrawBackgrdColor - for #102450#

    determine the color, that is respectively will be drawn as background
    for the page frame.
    Using existing method SwFrm::GetBackgroundBrush to determine the color
    that is set at the page frame respectively is parent. If none is found
    return the global retouche color

    @author OD

    @return Color
*/
const Color& SwPageFrm::GetDrawBackgrdColor() const
{
    const SvxBrushItem* pBrushItem;
    const Color* pDummyColor;
    SwRect aDummyRect;
    if ( GetBackgroundBrush( pBrushItem, pDummyColor, aDummyRect, true) )
        return pBrushItem->GetColor();
    else
        return aGlobalRetoucheColor;
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
            SwRect aRetouchePart( rRetouche );
            if ( aRetouchePart.HasArea() )
            {
                // OD 30.08.2002 #102450#
                // determine background color of page for <PaintLayer> method
                // calls, painting <hell> or <heaven>
                const Color aPageBackgrdColor = pPage->GetDrawBackgrdColor();
                // OD 29.08.2002 #102450#
                // add 3rd parameter to <PaintLayer> method calls
                // OD 09.12.2002 #103045# - add 4th parameter for horizontal text direction.
                pSh->Imp()->PaintLayer( pSh->GetDoc()->GetHellId(),
                                        aRetouchePart, &aPageBackgrdColor,
                                        (pPage->IsRightToLeft() ? true : false) );
                pSh->Imp()->PaintLayer( pSh->GetDoc()->GetHeavenId(),
                                        aRetouchePart, &aPageBackgrdColor,
                                        (pPage->IsRightToLeft() ? true : false) );
                pSh->Imp()->PaintLayer( pSh->GetDoc()->GetControlsId(),
                                        aRetouchePart );
            }

            SetRetouche();

            //Da wir uns ausserhalb aller Paint-Bereiche begeben muessen hier
            //leider die Hilfslinien erneuert werden.
            pPage->RefreshSubsidiary( aRetouchePart );
        }
    }
    if ( ViewShell::IsLstEndAction() )
        ResetRetouche();
}

/** SwFrm::GetBackgroundBrush

    @descr
    determine the background brush for the frame:
    the background brush is taken from it-self or from its parent (anchor/upper).
    Normally, the background brush is taken, which has no transparent color or
    which has a background graphic. But there are some special cases:
    (1) No background brush is taken from a page frame, if view option "IsPageBack"
        isn't set.
    (2) Background brush from a index section is taken under special conditions.
        In this case parameter <rpCol> is set to the index shading color.
    (3) New (OD 20.08.2002) - Background brush is taken, if on background drawing
        of the frame transparency is considered and its color is not "no fill"/"auto fill"
    ---- old description in german:
    Beschreibung        Liefert die Backgroundbrush fuer den Bereich des
        des Frm. Die Brush wird entweder von ihm selbst oder von einem
        Upper vorgegeben, die erste Brush wird benutzt.
        Ist fuer keinen Frm eine Brush angegeben, so wird FALSE zurueck-
        geliefert.
    Ersterstellung      MA 23. Dec. 92
    Letzte Aenderung    MA 04. Feb. 97

    @param rpBrush
    output parameter - constant reference pointer the found background brush

    @param rpCol
    output parameter - constant reference pointer to the color of the index shading
    set under special conditions, if background brush is taken from an index section.

    @param rOrigRect
    in-/output parameter - reference to the retangle the background brush is
    considered for - adjusted to the frame, from which the background brush is
    taken.

    @parem bLowerMode
    input parameter - boolean indicating, if background brush should *not* be
    taken from parent.

    @author MA
    @change 20.08.2002 by OD
    @docdate 20.08.2002

    @return true, if a background brush for the frame is found
*/
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
            /// OD 20.08.2002 #99657# #GetTransChg#
            ///     Note: If frame <pFrm> is a section of the index and
            ///         it its background color is "no fill"/"auto fill" and
            ///         it has no background graphic and
            ///         we are not in the page preview and
            ///         we are not in read-only mode and
            ///         option "index shadings" is set and
            ///         the output is not the printer
            ///         then set <rpCol> to the color of the index shading
            if( pSection && (   TOX_HEADER_SECTION == pSection->GetType() ||
                                TOX_CONTENT_SECTION == pSection->GetType() ) &&
                (rBack.GetColor() == COL_TRANSPARENT) &&
                ///rBack.GetColor().GetTransparency() &&
                rBack.GetGraphicPos() == GPOS_NONE &&
                !pOpt->IsPagePreview() && !pOpt->IsReadonly() &&
                SwViewOption::IsIndexShadings() &&
                !pOpt->IsPDFExport() &&
                pSh->GetOut()->GetOutDevType() != OUTDEV_PRINTER )
            {
                rpCol = &SwViewOption::GetIndexShadingsColor();
            }
        }

        /// OD 20.08.2002 #99657#
        ///     determine, if background draw of frame <pFrm> considers transparency
        ///     --> Status Quo: background transparency have to be
        ///                     considered for fly frames
        const sal_Bool bConsiderBackgroundTransparency = pFrm->IsFlyFrm();
        /// OD 20.08.2002 #99657#
        ///     add condition:
        ///     If <bConsiderBackgroundTransparency> is set - see above -,
        ///     return brush of frame <pFrm>, if its color is *not* "no fill"/"auto fill"
        if ( !rBack.GetColor().GetTransparency() ||
             rBack.GetGraphicPos() != GPOS_NONE ||
             rpCol ||
             (bConsiderBackgroundTransparency && (rBack.GetColor() != COL_TRANSPARENT))
           )
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
            /// Do not try to get background brush from parent (anchor/upper)
            return FALSE;

        /// get parent frame - anchor or upper - for next loop
        if ( pFrm->IsFlyFrm() )
            /// OD 20.08.2002 - use "static_cast" instead of "old C-cast"
            pFrm = (static_cast<const SwFlyFrm*>(pFrm))->GetAnchorFrm();
            ///pFrm = ((SwFlyFrm*)pFrm)->GetAnchor();
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

        // OD 09.12.2002 #103045# - determine page, fly frame is on
        const SwPageFrm* pFlyPage = pFly->FindPageFrm();
        // OD 30.08.2002 #102450#
        // determine color of page, the fly frame is on, for <PaintLayer> method
        // calls, painting <hell> or <heaven>
        const Color aPageBackgrdColor = pFlyPage->GetDrawBackgrdColor();
        // OD 30.08.2002 #102450# - add 3rd parameter
        // OD 09.12.2002 #103045# - add 4th parameter for horizontal text direction.
        pImp->PaintLayer( pSh->GetDoc()->GetHellId(), aOut, &aPageBackgrdColor,
                          (pFlyPage->IsRightToLeft() ? true : false) );
        pLines->PaintLines( &aDev );
        if ( pFly->IsFlyInCntFrm() )
            pFly->Paint( aOut );
        pLines->PaintLines( &aDev );
        /// OD 30.08.2002 #102450# - add 3rd parameter
        pImp->PaintLayer( pSh->GetDoc()->GetHeavenId(), aOut, &aPageBackgrdColor,
                          (pFlyPage->IsRightToLeft() ? true : false) );
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


