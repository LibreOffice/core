/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: viscrs.cxx,v $
 *
 *  $Revision: 1.29 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-07 14:51:53 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"


#ifndef _SVSTDARR_HXX
#define _SVSTDARR_USHORTS
#include <svtools/svstdarr.hxx>
#endif

#ifndef _DIALOG_HXX //autogen
#include <vcl/dialog.hxx>
#endif
#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _WRKWIN_HXX //autogen
#include <vcl/wrkwin.hxx>
#endif

#ifndef _VIEWOPT_HXX //autogen
#include <viewopt.hxx>
#endif
#ifndef _FRMTOOL_HXX
#include <frmtool.hxx>
#endif
#ifndef _VISCRS_HXX
#include <viscrs.hxx>
#endif
#ifndef _CRSRSH_HXX
#include <crsrsh.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _SWTABLE_HXX
#include <swtable.hxx>
#endif
#ifndef _VIEWIMP_HXX
#include <viewimp.hxx>
#endif
#ifndef _DVIEW_HXX
#include <dview.hxx>
#endif
#ifndef _ROOTFRM_HXX
#include <rootfrm.hxx>
#endif
#ifndef _TXTFRM_HXX
#include <txtfrm.hxx>   // SwTxtFrm
#endif
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif
#ifndef _EXTINPUT_HXX
#include <extinput.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _SCRIPTINFO_HXX
#include <scriptinfo.hxx>
#endif

#ifndef _MDIEXP_HXX
#include <mdiexp.hxx>           // GetSearchDialog
#endif
#ifndef _COMCORE_HRC
#include <comcore.hrc>          // ResId fuer Abfrage wenn zu Search & Replaces
#endif


// OD 24.01.2003 #106593# - no longer needed, included in <frmtool.hxx>
//extern void MA_FASTCALL SwAlignRect( SwRect &rRect, ViewShell *pSh );
extern void SwCalcPixStatics( OutputDevice *pOut );


//Damit beim ShowCrsr nicht immer wieder die gleiche Size teuer ermittelt
//werden muss, hier statische Member, die beim Wechsel des MapModes
// angepasst werden

long SwSelPaintRects::nPixPtX = 0;
long SwSelPaintRects::nPixPtY = 0;
MapMode* SwSelPaintRects::pMapMode = 0;



//#define SHOW_BOOKMARKS
//#define SHOW_REDLINES

#ifdef SHOW_BOOKMARKS

#ifndef _BOOKMRK_HXX
#include <bookmrk.hxx>
#endif

class SwBookmarkRects : public SwSelPaintRects
{
    virtual void Paint( const Rectangle& rRect );
    virtual void FillRects();

public:
    SwBookmarkRects( const SwCrsrShell& rSh ) : SwSelPaintRects( rSh ) {}
};

void SwBookmarkRects::Paint( const Rectangle& rRect )
{
    Window* pWin = GetShell()->GetWin();

    RasterOp eOld( pWin->GetRasterOp() );
    BOOL bLCol = pWin->IsLineColor();
    Color aLCol( pWin->GetLineColor() );
    BOOL bFCol = pWin->IsFillColor();
    Color aFCol( pWin->GetFillColor() );

    pWin->SetRasterOp( ROP_XOR );
    Color aCol( RGB_COLORDATA( 0xF0, 0xC8, 0xF0 ) ^ COL_WHITE );
    pWin->SetFillColor( aCol );
    pWin->SetLineColor( aCol );

    pWin->DrawRect( rRect );

    if( bLCol ) pWin->SetLineColor( aLCol ); else pWin->SetLineColor();
    if( bFCol ) pWin->SetFillColor( aFCol ); else pWin->SetFillColor();
    pWin->SetRasterOp( eOld );
}

void SwBookmarkRects::FillRects()
{
    SwRegionRects aReg( GetShell()->VisArea() );

    const SwBookmarks& rBkmkTbl = GetShell()->getIDocumentBookmarkAccess()->getBookmarks();
    SwShellCrsr* pCrsr = 0;
    for( USHORT n = 0; n < rBkmkTbl.Count(); ++n )
    {
        const SwBookmark& rBkmk = *rBkmkTbl[ n ];
        if( rBkmk.IsBookMark() && rBkmk.GetOtherPos() )
        {
            if( !pCrsr )
            {
                pCrsr = new SwShellCrsr( *GetShell(), rBkmk.GetPos() );
                pCrsr->SetMark();
            }
            else
                *pCrsr->GetPoint() = rBkmk.GetPos();
            *pCrsr->GetMark() = *rBkmk.GetOtherPos();
            pCrsr->FillRects();
            for( USHORT i = 0; i < pCrsr->Count(); ++i )
                aReg -= (*pCrsr)[ i ];

            pCrsr->Remove( 0, i );
        }
    }
    if( pCrsr ) delete pCrsr;

    aReg.Invert();
    SwRects::Insert( &aReg, 0 );
}

SwBookmarkRects* pBookMarkRects = 0;

void ShowBookmarks( const SwCrsrShell* pSh, int nAction, const SwRect* pRect = 0 )
{
    if( !pBookMarkRects && pSh->getIDocumentBookmarkAccess()->getBookmarks().Count() )
        pBookMarkRects = new SwBookmarkRects( *pSh );

    if( pBookMarkRects )
    {
        switch( nAction )
        {
        case 1: pBookMarkRects->Show(); break;
        case 2: pBookMarkRects->Hide(); break;
        case 3: pBookMarkRects->Invalidate( *pRect ); break;
        }

        if( !pBookMarkRects->Count() )
            delete pBookMarkRects, pBookMarkRects = 0;
    }
}

#define SHOWBOOKMARKS1( nAct )          ShowBookmarks( GetShell(),nAct );
#define SHOWBOOKMARKS2( nAct, pRect )   ShowBookmarks( GetShell(),nAct, pRect );

#else

#define SHOWBOOKMARKS1( nAct )
#define SHOWBOOKMARKS2( nAct, pRect )

#endif

#ifdef SHOW_REDLINES

#ifndef _REDLINE_HXX
#include <redline.hxx>
#endif

class SwRedlineRects : public SwSelPaintRects
{
    USHORT nMode;
    USHORT nNm;

    virtual void Paint( const Rectangle& rRect );
    virtual void FillRects();

public:
    SwRedlineRects( const SwCrsrShell& rSh, USHORT nName, USHORT n )
        : SwSelPaintRects( rSh ), nMode( n ), nNm( nName )
    {}
};

void SwRedlineRects::Paint( const Rectangle& rRect )
{
    Window* pWin = GetShell()->GetWin();

    RasterOp eOld( pWin->GetRasterOp() );
    BOOL bLCol = pWin->IsLineColor();
    Color aLCol( pWin->GetLineColor() );
    BOOL bFCol = pWin->IsFillColor();
    Color aFCol( pWin->GetFillColor() );

    pWin->SetRasterOp( ROP_XOR );
    Color aCol;

    UINT8 nVal = 0xc8 - ( (nMode / 4) * 16 );
    switch( nMode % 4 )
    {
    case 0: aCol = RGB_COLORDATA( nVal, nVal, 0xFF );   break;
    case 1: aCol = RGB_COLORDATA( 0xFF, 0xc8, nVal );   break;
    case 2: aCol = RGB_COLORDATA( nVal, 0xFF, nVal );   break;
    case 3: aCol = RGB_COLORDATA( 0xFF, nVal, nVal );   break;
    }
    aCol = aCol.GetColor() ^ COL_WHITE;

    pWin->SetFillColor( aCol );
    pWin->SetLineColor( aCol );

    pWin->DrawRect( rRect );

    if( bLCol ) pWin->SetLineColor( aLCol ); else pWin->SetLineColor();
    if( bFCol ) pWin->SetFillColor( aFCol ); else pWin->SetFillColor();
    pWin->SetRasterOp( eOld );
}

void SwRedlineRects::FillRects()
{
    SwRegionRects aReg( GetShell()->VisArea() );

    const SwRedlineTbl& rTbl = GetShell()->GetDoc()->GetRedlineTbl();
    SwShellCrsr* pCrsr = 0;
    for( USHORT n = 0; n < rTbl.Count(); ++n )
    {
        const SwRedline& rRed = *rTbl[ n ];
        if( rRed.HasMark() && (nMode % 4 ) == rRed.GetType() &&
            nNm == rRed.GetAuthor() )
        {
            if( !pCrsr )
            {
                pCrsr = new SwShellCrsr( *GetShell(), *rRed.GetPoint() );
                pCrsr->SetMark();
            }
            else
                *pCrsr->GetPoint() = *rRed.GetPoint();
            *pCrsr->GetMark() = *rRed.GetMark();
            pCrsr->FillRects();
            for( USHORT i = 0; i < pCrsr->Count(); ++i )
                aReg -= (*pCrsr)[ i ];

            pCrsr->Remove( 0, i );
        }
    }
    if( pCrsr ) delete pCrsr;

    aReg.Invert();
    SwRects::Insert( &aReg, 0 );
}

SwRedlineRects* aRedlines[ 10 * 4 ];
static int bFirstCall = TRUE;

void ShowRedlines( const SwCrsrShell* pSh, int nAction, const SwRect* pRect = 0 )
{
    if( bFirstCall )
    {
        memset( aRedlines, 0, sizeof(aRedlines));
        bFirstCall = FALSE;
    }

    const SwRedlineTbl& rTbl = pSh->GetDoc()->GetRedlineTbl();
    const SwRedlineAuthorTbl& rAuthorTbl = pSh->GetDoc()->GetRedlineAuthorTbl();

    for( USHORT n = 0; n < rAuthorTbl.Count(); ++n )
    {
        for( int i = 0; i < 4; ++i  )
        {
            SwRedlineRects** ppRedRect = &aRedlines[ n * 4 + i ];
            if( rTbl.Count() && !*ppRedRect )
                *ppRedRect = new SwRedlineRects( *pSh, n, n * 4 + i );

            if( *ppRedRect )
            {
                switch( nAction )
                {
                case 1: (*ppRedRect)->Show(); break;
                case 2: (*ppRedRect)->Hide(); break;
                case 3: (*ppRedRect)->Invalidate( *pRect ); break;
                }

                if( !(*ppRedRect)->Count() )
                    delete *ppRedRect, *ppRedRect = 0;
            }
        }
    }
}

#define SHOWREDLINES1( nAct )           ShowRedlines( GetShell(),nAct );
#define SHOWREDLINES2( nAct, pRect )    ShowRedlines( GetShell(),nAct, pRect );

#else

#define SHOWREDLINES1( nAct )
#define SHOWREDLINES2( nAct, pRect )

#endif

#ifdef JP_REDLINE
    if( GetDoc()->GetRedlineTbl().Count() )
    {
        SwRedlineTbl& rRedlineTbl = (SwRedlineTbl&)GetDoc()->GetRedlineTbl();
        for( USHORT i = 0; i < rRedlineTbl.Count(); ++i )
            rRedlineTbl[ i ]->HideRects( *GetShell() );
    }
#endif

// --------  Ab hier Klassen / Methoden fuer den nicht Text-Cursor ------

SwVisCrsr::SwVisCrsr( const SwCrsrShell * pCShell )
    : pCrsrShell( pCShell )
{
    pCShell->GetWin()->SetCursor( &aTxtCrsr );
    bIsVisible = aTxtCrsr.IsVisible();
    bIsDragCrsr = FALSE;
    aTxtCrsr.SetWidth( 0 );

#ifdef SW_CRSR_TIMER
    bTimerOn = TRUE;
    SetTimeout( 50 );       // 50msec Verzoegerung
#endif
}



SwVisCrsr::~SwVisCrsr()
{
#ifdef SW_CRSR_TIMER
    if( bTimerOn )
        Stop();     // Timer stoppen
#endif

    if( bIsVisible && aTxtCrsr.IsVisible() )
        aTxtCrsr.Hide();

    pCrsrShell->GetWin()->SetCursor( 0 );
}




void SwVisCrsr::Show()
{
    if( !bIsVisible )
    {
        bIsVisible = TRUE;

        // muss ueberhaupt angezeigt werden ?
        if( pCrsrShell->VisArea().IsOver( pCrsrShell->aCharRect ) )
#ifdef SW_CRSR_TIMER
        {
            if( bTimerOn )
                Start();            // Timer aufsetzen
            else
            {
                if( IsActive() )
                    Stop();         // Timer Stoppen

                _SetPosAndShow();
            }
        }
#else
            _SetPosAndShow();
#endif
    }
}



void SwVisCrsr::Hide()
{
    if( bIsVisible )
    {
        bIsVisible = FALSE;

#ifdef SW_CRSR_TIMER
        if( IsActive() )
            Stop();         // Timer Stoppen
#endif

        if( aTxtCrsr.IsVisible() )      // sollten die Flags nicht gueltig sein?
            aTxtCrsr.Hide();
    }
}

#ifdef SW_CRSR_TIMER

void __EXPORT SwVisCrsr::Timeout()
{
    ASSERT( !bIsDragCrsr, "Timer vorher abschalten" );
    if( bIsVisible )
    {
        if ( !pCrsrShell->GetWin() ) //SwFrmFmt::GetGraphic setzt das Win temp aus!
            Start();
        else
            _SetPosAndShow();
    }
}

BOOL SwCrsrShell::ChgCrsrTimerFlag( BOOL bTimerOn )
{
    return pVisCrsr->ChgTimerFlag( bTimerOn );
}


BOOL SwVisCrsr::ChgTimerFlag( BOOL bFlag )
{
    bOld = bTimerOn;
    if( !bFlag && bIsVisible && IsActive() )
    {
        Stop();         // Timer Stoppen
        _SetPosAndShow();
    }
    bTimerOn = bFlag;
    return bOld;
}

#endif


void SwVisCrsr::_SetPosAndShow()
{
    SwRect aRect;
    long nTmpY = pCrsrShell->aCrsrHeight.Y();
    if( 0 > nTmpY )
    {
        nTmpY = -nTmpY;
        aTxtCrsr.SetOrientation( 900 );
        aRect = SwRect( pCrsrShell->aCharRect.Pos(),
           Size( pCrsrShell->aCharRect.Height(), nTmpY ) );
        aRect.Pos().X() += pCrsrShell->aCrsrHeight.X();
        if( pCrsrShell->IsOverwriteCrsr() )
            aRect.Pos().Y() += aRect.Width();
    }
    else
    {
        aTxtCrsr.SetOrientation( 0 );
        aRect = SwRect( pCrsrShell->aCharRect.Pos(),
           Size( pCrsrShell->aCharRect.Width(), nTmpY ) );
        aRect.Pos().Y() += pCrsrShell->aCrsrHeight.X();
    }

    // check if cursor should show the current cursor bidi level
    aTxtCrsr.SetDirection( CURSOR_DIRECTION_NONE );
    const SwCursor* pTmpCrsr = pCrsrShell->_GetCrsr();

    if ( pTmpCrsr && !pCrsrShell->IsOverwriteCrsr() )
    {
        SwNode& rNode = pTmpCrsr->GetPoint()->nNode.GetNode();
        if( rNode.IsTxtNode() )
        {
            const SwTxtNode& rTNd = *rNode.GetTxtNode();
            const SwFrm* pFrm = rTNd.GetFrm( 0, 0, FALSE );
            if ( pFrm )
            {
                const SwScriptInfo* pSI = ((SwTxtFrm*)pFrm)->GetScriptInfo();
                 // cursor level has to be shown
                if ( pSI && pSI->CountDirChg() > 1 )
                {
                    aTxtCrsr.SetDirection(
                        ( pTmpCrsr->GetCrsrBidiLevel() % 2 ) ?
                          CURSOR_DIRECTION_RTL :
                          CURSOR_DIRECTION_LTR );
                }

                if ( pFrm->IsRightToLeft() )
                {
                    const OutputDevice *pOut = pCrsrShell->GetOut();
                    if ( pOut )
                    {
                        long nSize = pOut->GetSettings().GetStyleSettings().GetCursorSize();
                        Size aSize( nSize, nSize );
                        aSize = pOut->PixelToLogic( aSize );
                        aRect.Left( aRect.Left() - aSize.Width() );
                    }
                }
            }
        }
    }

    if( aRect.Height() )
    {
        ::SwCalcPixStatics( pCrsrShell->GetOut() );
        ::SwAlignRect( aRect, (ViewShell*)pCrsrShell );
    }
    if( !pCrsrShell->IsOverwriteCrsr() || bIsDragCrsr ||
        pCrsrShell->IsSelection() )
        aRect.Width( 0 );

    aTxtCrsr.SetSize( aRect.SSize() );

    aTxtCrsr.SetPos( aRect.Pos() );
    if ( !pCrsrShell->IsCrsrReadonly()  || pCrsrShell->GetViewOptions()->IsSelectionInReadonly() )
    {
        if ( pCrsrShell->GetDrawView() )
            ((SwDrawView*)pCrsrShell->GetDrawView())->SetAnimationEnabled(
                    !pCrsrShell->IsSelection() );

        USHORT nStyle = bIsDragCrsr ? CURSOR_SHADOW : 0;
        if( nStyle != aTxtCrsr.GetStyle() )
        {
            aTxtCrsr.SetStyle( nStyle );
            aTxtCrsr.SetWindow( bIsDragCrsr ? pCrsrShell->GetWin() : 0 );
        }

        aTxtCrsr.Show();
    }
}


/*  */
// ------ Ab hier Klassen / Methoden fuer die Selectionen -------

// #i75172#
#ifndef _SDR_OVERLAY_OVERLAYMANAGER_HXX
#include <svx/sdr/overlay/overlaymanager.hxx>
#endif

#ifndef _SDRPAINTWINDOW_HXX
#include <svx/sdrpaintwindow.hxx>
#endif

#ifndef _BGFX_POLYGON_B2DPOLYGONTOOLS_HXX
#include <basegfx/polygon/b2dpolygontools.hxx>
#endif

#ifndef _BGFX_POLYGON_B2DPOLYGON_HXX
#include <basegfx/polygon/b2dpolygon.hxx>
#endif

#ifndef _BGFX_POLYPOLYGON_B2DPOLYGONTOOLS_HXX
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#endif

#ifndef _BGFX_MATRIX_B2DHOMMATRIX_HXX
#include <basegfx/matrix/b2dhommatrix.hxx>
#endif

#ifndef _SV_HATCH_HXX
#include <vcl/hatch.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////
// #i75172#

enum SwOverlayType { SW_OVERLAY_INVERT, SW_OVERLAY_HATCH, SW_OVERLAY_TRANSPARENT, SW_OVERLAY_LIGHT_TRANSPARENT };

namespace sdr
{
    namespace overlay
    {
        class OverlaySwSelPaintRects : public OverlayObject
        {
            // geometry
            std::vector< basegfx::B2DRange >        maRanges;
            SwOverlayType                           mePaintType;

            // Draw geometry
            virtual void drawGeometry(OutputDevice& rOutputDevice);

            // Create the BaseRange. This method needs to calculate maBaseRange.
            virtual void createBaseRange(OutputDevice& rOutputDevice);

        public:
            OverlaySwSelPaintRects(Color aBaseColor, const std::vector< basegfx::B2DRange >& rRanges, SwOverlayType eType);
            virtual ~OverlaySwSelPaintRects();

            // data access
            const std::vector< basegfx::B2DRange >& getB2DRanges() const { return maRanges; }
            void setB2DRanges(const std::vector< basegfx::B2DRange >& rNew);

            // Hittest with logical coordinates
            virtual sal_Bool isHit(const basegfx::B2DPoint& rPos, double fTol) const;

            // transform object coordinates. Needs to transform maSecondPosition
            // and maThirdPosition.
            virtual void transform(const basegfx::B2DHomMatrix& rMatrix);
        };

        void OverlaySwSelPaintRects::drawGeometry(OutputDevice& rOutputDevice)
        {
            rOutputDevice.SetLineColor();
            rOutputDevice.SetFillColor(getBaseColor());

            if ( mePaintType == SW_OVERLAY_INVERT )
            {
                rOutputDevice.Push();
                rOutputDevice.SetRasterOp( ROP_XOR );
                rOutputDevice.SetFillColor( COL_WHITE );
            }

            for(sal_uInt32 a(0); a < maRanges.size(); a++)
            {
                const basegfx::B2DRange& rRange(maRanges[a]);
                const Rectangle aRectangle(
                    basegfx::fround(rRange.getMinX()), basegfx::fround(rRange.getMinY()),
                    basegfx::fround(rRange.getMaxX()), basegfx::fround(rRange.getMaxY()));

                switch(mePaintType)
                {
                    default: // SW_OVERLAY_INVERT
                    {
                        rOutputDevice.DrawRect( aRectangle );
                        break;
                    }
                    case SW_OVERLAY_HATCH :
                    {
                        rOutputDevice.DrawHatch(PolyPolygon(Polygon(aRectangle)), Hatch(HATCH_SINGLE, getBaseColor(), 2, 450));
                        break;
                    }
                    case SW_OVERLAY_TRANSPARENT :
                    {
                        rOutputDevice.DrawTransparent(PolyPolygon(Polygon(aRectangle)), 50);
                        break;
                    }
                    case SW_OVERLAY_LIGHT_TRANSPARENT :
                    {
                        rOutputDevice.DrawTransparent(PolyPolygon(Polygon(aRectangle)), 80);
                        break;
                    }
                }
            }

            if(mePaintType == SW_OVERLAY_INVERT)
            {
                rOutputDevice.Pop();
            }
        }

        void OverlaySwSelPaintRects::createBaseRange(OutputDevice& /*rOutputDevice*/)
        {
            maBaseRange.reset();

            for(sal_uInt32 a(0); a < maRanges.size(); a++)
            {
                const basegfx::B2DRange& rCandidate = maRanges[a];
                maBaseRange.expand(rCandidate);
            }
        }

        OverlaySwSelPaintRects::OverlaySwSelPaintRects(Color aBaseColor, const std::vector< basegfx::B2DRange >& rRanges, SwOverlayType eType)
        :   OverlayObject(aBaseColor),
            maRanges(rRanges),
            mePaintType(eType)
        {
#ifdef DBG_UTIL
            // Here is a debugger test method for checking the new cursor modes.
            // Default is SW_OVERLAY_INVERT, of course...
            static sal_uInt16 nChangeForTest(0);
            switch(nChangeForTest) {
                default: mePaintType=SW_OVERLAY_INVERT; break;
                case 1: mePaintType=SW_OVERLAY_HATCH; break;
                case 2: mePaintType=SW_OVERLAY_TRANSPARENT; break;
                case 3: mePaintType=SW_OVERLAY_LIGHT_TRANSPARENT; break;
            }
#endif
        }

        OverlaySwSelPaintRects::~OverlaySwSelPaintRects()
        {
            if(getOverlayManager())
            {
                getOverlayManager()->remove(*this);
            }
        }

        void OverlaySwSelPaintRects::setB2DRanges(const std::vector< basegfx::B2DRange >& rNew)
        {
            if(rNew != maRanges)
            {
                maRanges = rNew;
                objectChange();
            }
        }

        sal_Bool OverlaySwSelPaintRects::isHit(const basegfx::B2DPoint& rPos, double /*fTol*/) const
        {
            if(isHittable())
            {
                for(sal_uInt32 a(0); a < maRanges.size(); a++)
                {
                    const basegfx::B2DRange& rCandidate = maRanges[a];

                    if(rCandidate.isInside(rPos))
                    {
                        return true;
                    }
                }
            }

            return sal_False;
        }

        void OverlaySwSelPaintRects::transform(const basegfx::B2DHomMatrix& rMatrix)
        {
            if(!rMatrix.isIdentity())
            {
                for(sal_uInt32 a(0); a < maRanges.size(); a++)
                {
                    basegfx::B2DRange& rCandidate = maRanges[a];
                    rCandidate.transform(rMatrix);
                }

                // register change (after change)
                objectChange();
            }
        }
    } // end of namespace overlay
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////
// #i75172#
// Following this flag, a lot of SwSelPaintRects may be removed in the future.
// This includes ::Invalidate, all ::Paint and the derivated implementations.
// I leave them for now to be able to follow old behaviour for some time.
static bool bTestOverlay(true);

//////////////////////////////////////////////////////////////////////////////

SwSelPaintRects::SwSelPaintRects( const SwCrsrShell& rCSh )
:   SwRects( 0 ),
    pCShell( &rCSh ),
    mpCursorOverlay(0)
{
}

SwSelPaintRects::~SwSelPaintRects()
{
    Hide();
}

void SwSelPaintRects::swapContent(SwSelPaintRects& rSwap)
{
    SwRects aTempRects;
    aTempRects.Insert(this, 0);

    Remove(0, Count());
    Insert(&rSwap, 0);

    rSwap.Remove(0, rSwap.Count());
    rSwap.Insert(&aTempRects, 0);

    // #i75172# also swap mpCursorOverlay
    if(bTestOverlay)
    {
        sdr::overlay::OverlayObject* pTempOverlay = getCursorOverlay();
        setCursorOverlay(rSwap.getCursorOverlay());
        rSwap.setCursorOverlay(pTempOverlay);
    }
}

void SwSelPaintRects::Hide()
{
    if(mpCursorOverlay)
    {
        delete mpCursorOverlay;
        mpCursorOverlay = 0;
    }

    for( USHORT n = 0; n < Count(); ++n )
        Paint( (*this)[n] );
    SwRects::Remove( 0, Count() );
}

void SwSelPaintRects::Show()
{
    if(bTestOverlay)
    {
        SdrView* pView = (SdrView*)pCShell->GetDrawView();

        if(pView && pView->PaintWindowCount())
        {
            SwRects::Remove( 0, SwRects::Count() );
            FillRects();
            std::vector< basegfx::B2DRange > aNewRanges;
            Window* pWin = GetShell()->GetWin();

            for(sal_uInt16 a(0); a < Count(); a++)
            {
                const SwRect aNextRect((*this)[a]);
                Rectangle aPntRect(aNextRect.SVRect());

                if(pWin)
                {
                    // avoid single-pixel overlaps
                    Rectangle aCalcRect( aPntRect );
                    bool bChange(false);

                    ++aCalcRect.Bottom();
                    ++aCalcRect.Right();

                    aPntRect = pWin->LogicToPixel( aPntRect );
                    aCalcRect = pWin->LogicToPixel( aCalcRect );

                    if(aPntRect.Bottom() == aCalcRect.Bottom())
                    {
                        --aPntRect.Bottom();
                        bChange = true;
                    }

                    if(aPntRect.Right() == aCalcRect.Right())
                    {
                        --aPntRect.Right();
                        bChange = true;
                    }

                    if(bChange)
                    {
                        aPntRect = pWin->PixelToLogic(aPntRect);
                    }
                    else
                    {
                        aPntRect = aNextRect.SVRect();
                    }
                }

                aNewRanges.push_back(basegfx::B2DRange(aPntRect.Left(), aPntRect.Top(), aPntRect.Right(), aPntRect.Bottom()));
            }

            if(mpCursorOverlay)
            {
                if(aNewRanges.size())
                {
                    static_cast< sdr::overlay::OverlaySwSelPaintRects* >(mpCursorOverlay)->setB2DRanges(aNewRanges);
                }
                else
                {
                    delete mpCursorOverlay;
                    mpCursorOverlay = 0;
                }
            }
            else if(Count())
            {
                SdrPaintWindow* pCandidate = pView->GetPaintWindow(0);
                sdr::overlay::OverlayManager* pTargetOverlay = pCandidate->GetOverlayManager();

                if(pTargetOverlay)
                {
                    Color aHighlight(COL_BLACK);
                    const OutputDevice *pOut = GetShell()->GetOut();

                    if(pOut)
                    {
                        aHighlight = pOut->GetSettings().GetStyleSettings().GetHighlightColor();
                    }

                    mpCursorOverlay = new sdr::overlay::OverlaySwSelPaintRects(aHighlight, aNewRanges, SW_OVERLAY_INVERT);
                    pTargetOverlay->add(*mpCursorOverlay);
                }
            }
        }
    }
    else
    {
        if( pCShell->GetDrawView() )
        {
            SdrView* pView = (SdrView*)pCShell->GetDrawView();
            pView->SetAnimationEnabled( !pCShell->IsSelection() );
        }

        SwRects aTmp;
        aTmp.Insert( this, 0 );     // Kopie vom Array

        SwRects::Remove( 0, SwRects::Count() );
        FillRects();

        if( Count() || aTmp.Count() )
        {
            SwRegionRects aReg( pCShell->VisArea() );
            USHORT n;

            // suche die neu selektierten Rechtecke heraus
            aReg.Remove( 0, aReg.Count() );
            aReg.Insert( this, 0 );

            for( n = 0; n < aTmp.Count(); ++n )
                aReg -= aTmp[n];

            // jetzt sollten in aReg nur noch die neuen Rechtecke vorliegen
            for( n = 0; n < aReg.Count(); ++n )
                Paint( aReg[n] );

            // suche die nicht mehr selektierten Rechtecke heraus
            if( aTmp.Count() )
            {
                aReg.Remove( 0, aReg.Count() );
                aReg.Insert( &aTmp, 0 );

                for( n = 0; n < Count(); ++n )
                    aReg -= (*this)[n];
                // jetzt sollten in aReg nur noch die alten Rechtecke vorliegen
                for( n = 0; n < aReg.Count(); ++n )
                    Paint( aReg[n] );
            }
        }
    }
}

void SwSelPaintRects::Invalidate( const SwRect& rRect )
{
    USHORT nSz = Count();
    if( !nSz )
        return;

    SwRegionRects aReg( GetShell()->VisArea() );
    aReg.Remove( 0, aReg.Count() );
    aReg.Insert( this, 0 );
    aReg -= rRect;
    SwRects::Remove( 0, nSz );
    SwRects::Insert( &aReg, 0 );

    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // Liegt die Selection rechts oder unten ausserhalb des sichtbaren
    // Bereiches, so ist diese nie auf eine Pixel rechts/unten aligned.
    // Das muss hier erkannt und ggf. das Rechteckt erweitert werden.
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    if( GetShell()->bVisPortChgd && 0 != ( nSz = Count()) )
    {
        SwSelPaintRects::Get1PixelInLogic( *GetShell() );
        SwRect* pRect = (SwRect*)GetData();
        for( ; nSz--; ++pRect )
        {
            if( pRect->Right() == GetShell()->aOldRBPos.X() )
                pRect->Right( pRect->Right() + nPixPtX );
            if( pRect->Bottom() == GetShell()->aOldRBPos.Y() )
                pRect->Bottom( pRect->Bottom() + nPixPtY );
        }
    }
}

void SwSelPaintRects::Paint( const Rectangle& rRect )
{
    if(bTestOverlay)
    {
    }
    else
    {
        GetShell()->GetWin()->Invert( rRect );
    }
}

/*
 * Rectangle ist in Dokument-Koordianten !!
 * pWin != 0 ->  auch wirklich malen
 *      == 0 ->  nur testen, ob es gemalt werden kann
 */

void SwSelPaintRects::Paint( const SwRect& rRect )
{
    if(bTestOverlay)
    {
    }
    else
    {
        Window* pWin = GetShell()->GetWin();
        const SwRect& rVisArea = GetShell()->VisArea();

        if( !pWin || rRect.IsEmpty() || !rVisArea.IsOver( rRect ) )
            return;

        Rectangle aPntRect( rRect.SVRect() );
        Rectangle aCalcRect( aPntRect );

        aPntRect = pWin->LogicToPixel( aPntRect );

        // falls nach der "Normalisierung" kein Rectangle besteht -> Ende
        if( aPntRect.Left() == aPntRect.Right() ||
            aPntRect.Top() == aPntRect.Bottom() )
            return;

        // damit Linien nicht doppelt invertiert werden, muss jeweis von
        // der rechten und unteren Seite ein PIXEL abgezogen werden !
        // Pixel heisst, gleichgueltig, welcher MapMode heute zaehlt !

        BOOL bChg = FALSE;
        BOOL bTstRight  = rRect.Right() < rVisArea.Right();
        BOOL bTstBottom = rRect.Bottom() < rVisArea.Bottom();

        if( bTstBottom || bTstRight )
        {
            ++aCalcRect.Bottom();
            ++aCalcRect.Right();
            aCalcRect = pWin->LogicToPixel( aCalcRect );

            if( bTstBottom && aPntRect.Bottom() == aCalcRect.Bottom() )
            {
                --aPntRect.Bottom();
                bChg = TRUE;
            }
            if( bTstRight && aPntRect.Right() == aCalcRect.Right() )
            {
                --aPntRect.Right();
                bChg = TRUE;
            }
        }

        if( bChg )
            Paint( pWin->PixelToLogic( aPntRect ));
        else
            Paint( rRect.SVRect() );
    }
}

// check current MapMode of the shell and set possibly the static members.
// Optional set the parameters pX, pY
void SwSelPaintRects::Get1PixelInLogic( const ViewShell& rSh,
                                        long* pX, long* pY )
{
    const OutputDevice* pOut = rSh.GetWin();
    if ( ! pOut )
        pOut = rSh.GetOut();

    const MapMode& rMM = pOut->GetMapMode();
    if( pMapMode->GetMapUnit() != rMM.GetMapUnit() ||
        pMapMode->GetScaleX() != rMM.GetScaleX() ||
        pMapMode->GetScaleY() != rMM.GetScaleY() )
    {
        *pMapMode = rMM;
        Size aTmp( 1, 1 );
        aTmp = pOut->PixelToLogic( aTmp );
        nPixPtX = aTmp.Width();
        nPixPtY = aTmp.Height();
    }
    if( pX )
        *pX = nPixPtX;
    if( pY )
        *pY = nPixPtY;
}


/*  */

SwShellCrsr::SwShellCrsr( const SwCrsrShell& rCShell, const SwPosition &rPos )
    : SwCursor(rPos,0,false), SwSelPaintRects(rCShell), pPt(SwPaM::GetPoint())
{}


SwShellCrsr::SwShellCrsr( const SwCrsrShell& rCShell, const SwPosition &rPos,
                            const Point& rPtPos, SwPaM* pRing )
    : SwCursor(rPos, pRing, false), SwSelPaintRects(rCShell), aMkPt(rPtPos),
    aPtPt(rPtPos), pPt(SwPaM::GetPoint())
{}


SwShellCrsr::SwShellCrsr( SwShellCrsr& rICrsr )
    : SwCursor(rICrsr), SwSelPaintRects(*rICrsr.GetShell()),
    aMkPt(rICrsr.GetMkPos()), aPtPt(rICrsr.GetPtPos()), pPt(SwPaM::GetPoint())
{}

SwShellCrsr::~SwShellCrsr() {}

SwShellCrsr::operator SwShellCrsr* ()   { return this; }

void SwShellCrsr::SetMark()
{
    if( SwPaM::GetPoint() == pPt )
        aMkPt = aPtPt;
    else
        aPtPt = aMkPt;
    SwPaM::SetMark();
}

void SwShellCrsr::FillRects()
{
    // die neuen Rechtecke berechnen
    if( HasMark() &&
        GetPoint()->nNode.GetNode().IsCntntNode() &&
        GetPoint()->nNode.GetNode().GetCntntNode()->GetFrm() &&
        (GetMark()->nNode == GetPoint()->nNode ||
        (GetMark()->nNode.GetNode().IsCntntNode() &&
         GetMark()->nNode.GetNode().GetCntntNode()->GetFrm() )  ))
        GetDoc()->GetRootFrm()->CalcFrmRects( *this, GetShell()->IsTableMode() );
}


void SwShellCrsr::Show()
{
    SwShellCrsr * pTmp = this;
    do {
        pTmp->SwSelPaintRects::Show();
    } while( this != ( pTmp = (SwShellCrsr*)*(SwCursor*)(pTmp->GetNext() )));

    SHOWBOOKMARKS1( 1 )
    SHOWREDLINES1( 1 )
}


    // Dieses Rechteck wird neu gepaintet, also ist die SSelection in
    // dem Bereich ungueltig
void SwShellCrsr::Invalidate( const SwRect& rRect )
{
    SwShellCrsr * pTmp = this;
    SwCursor* pTmpCrsr;

    do
    {
        pTmp->SwSelPaintRects::Invalidate( rRect );

        // --> FME 2005-08-18 #125102#
        // skip any non SwShellCrsr objects in the ring
        // (see:SwAutoFormat::DeleteSel()
        // <--
        Ring* pTmpRing = pTmp;
        pTmp = 0;
        do
        {
            pTmpRing = pTmpRing->GetNext();
            pTmpCrsr = dynamic_cast<SwCursor*>(pTmpRing);
            if ( pTmpCrsr )
                pTmp = (SwShellCrsr*)*pTmpCrsr;
        }
        while ( !pTmp );
    }
    while( this != pTmp );

    SHOWBOOKMARKS2( 3, &rRect )
    SHOWREDLINES2( 3, &rRect )
}


void SwShellCrsr::Hide()
{
    SwShellCrsr * pTmp = this;
    do {
        pTmp->SwSelPaintRects::Hide();
    } while( this != ( pTmp = (SwShellCrsr*)*(SwCursor*)(pTmp->GetNext() )));

    SHOWBOOKMARKS1( 2 )
    SHOWREDLINES1( 2 )
}

SwCursor* SwShellCrsr::Create( SwPaM* pRing ) const
{
    return new SwShellCrsr( *GetShell(), *GetPoint(), GetPtPos(), pRing );
}


short SwShellCrsr::MaxReplaceArived()
{
    short nRet = RET_YES;
    Window* pDlg = ::GetSearchDialog();
    if( pDlg )
    {
        // alte Actions beenden; die Tabellen-Frames werden angelegt und
        // eine SSelection kann erzeugt werden
        SvUShorts aArr;
        USHORT nActCnt;
        ViewShell *pShell = GetDoc()->GetRootFrm()->GetCurrShell(),
                  *pSh = pShell;
        do {
            for( nActCnt = 0; pSh->ActionPend(); ++nActCnt )
                pSh->EndAction();
            aArr.Insert( nActCnt, aArr.Count() );
        } while( pShell != ( pSh = (ViewShell*)pSh->GetNext() ) );

        {
            nRet = QueryBox( pDlg, SW_RES( MSG_COMCORE_ASKSEARCH )).Execute();
        }

        for( USHORT n = 0; n < aArr.Count(); ++n )
        {
            for( nActCnt = aArr[n]; nActCnt--; )
                pSh->StartAction();
            pSh = (ViewShell*)pSh->GetNext();
        }
    }
    else
        // ansonsten aus dem Basic, und dann auf RET_YES schalten
        nRet = RET_YES;

    return nRet;
}

void SwShellCrsr::SaveTblBoxCntnt( const SwPosition* pPos )
{
    ((SwCrsrShell*)GetShell())->SaveTblBoxCntnt( pPos );
}

BOOL SwShellCrsr::UpDown( BOOL bUp, USHORT nCnt )
{
    return SwCursor::UpDown( bUp, nCnt,
                            &GetPtPos(), GetShell()->GetUpDownX() );
}

#ifndef PRODUCT

// JP 05.03.98: zum Testen des UNO-Crsr Verhaltens hier die Implementierung
//              am sichtbaren Cursor

BOOL SwShellCrsr::IsSelOvr( int eFlags )
{
    return SwCursor::IsSelOvr( eFlags );
}

#endif

// TRUE: an die Position kann der Cursor gesetzt werden
BOOL SwShellCrsr::IsAtValidPos( BOOL bPoint ) const
{
    if( GetShell() && ( GetShell()->IsAllProtect() ||
        GetShell()->GetViewOptions()->IsReadonly() ||
        ( GetShell()->Imp()->GetDrawView() &&
          GetShell()->Imp()->GetDrawView()->GetMarkedObjectList().GetMarkCount() )))
        return TRUE;

    return SwCursor::IsAtValidPos( bPoint );
}

/*  */

SwShellTableCrsr::SwShellTableCrsr( const SwCrsrShell& rCrsrSh,
                                    const SwPosition& rPos )
    : SwCursor(rPos,0,false), SwShellCrsr(rCrsrSh, rPos), SwTableCursor(rPos)
{
}

SwShellTableCrsr::SwShellTableCrsr( const SwCrsrShell& rCrsrSh,
                    const SwPosition& rMkPos, const Point& rMkPt,
                    const SwPosition& rPtPos, const Point& rPtPt )
    : SwCursor(rPtPos,0,false), SwShellCrsr(rCrsrSh, rPtPos), SwTableCursor(rPtPos)
{
    SetMark();
    *GetMark() = rMkPos;
    GetMkPos() = rMkPt;
    GetPtPos() = rPtPt;
}

SwShellTableCrsr::~SwShellTableCrsr() {}

void SwShellTableCrsr::SetMark()                { SwShellCrsr::SetMark(); }
SwShellTableCrsr::operator SwShellCrsr* ()      { return this; }
SwShellTableCrsr::operator SwTableCursor* ()    { return this; }
SwShellTableCrsr::operator SwShellTableCrsr* () { return this; }

SwCursor* SwShellTableCrsr::Create( SwPaM* pRing ) const
{
    return SwShellCrsr::Create( pRing );
}
short SwShellTableCrsr::MaxReplaceArived()
{
    return SwShellCrsr::MaxReplaceArived();
}
void SwShellTableCrsr::SaveTblBoxCntnt( const SwPosition* pPos )
{
    SwShellCrsr::SaveTblBoxCntnt( pPos );
}


void SwShellTableCrsr::FillRects()
{
    // die neuen Rechtecke berechnen
    // JP 16.01.98: wenn der Cursor noch "geparkt" ist nichts machen!!
    if( !aSelBoxes.Count() || bParked ||
        !GetPoint()->nNode.GetIndex() )
        return;

    SwRegionRects aReg( GetShell()->VisArea() );
    SwNodes& rNds = GetDoc()->GetNodes();
    for( USHORT n = 0; n < aSelBoxes.Count(); ++n )
    {
        const SwStartNode* pSttNd = (*(aSelBoxes.GetData() + n ))->GetSttNd();
        const SwTableNode* pSelTblNd = pSttNd->FindTableNode();

        SwNodeIndex aIdx( *pSttNd );
           SwCntntNode* pCNd = rNds.GoNextSection( &aIdx, TRUE, FALSE );

        // TABLE IN TABLE
        // (see also lcl_FindTopLevelTable in unoobj2.cxx for a different
        // version to do this)
        const SwTableNode* pCurTblNd = pCNd->FindTableNode();
        while ( pSelTblNd != pCurTblNd && pCurTblNd )
        {
            aIdx = pCurTblNd->EndOfSectionIndex();
            pCNd = rNds.GoNextSection( &aIdx, TRUE, FALSE );
            pCurTblNd = pCNd->FindTableNode();
        }

        if( !pCNd )
            continue;

        SwFrm* pFrm = pCNd->GetFrm( &GetSttPos() );
        while( pFrm && !pFrm->IsCellFrm() )
            pFrm = pFrm->GetUpper();

        ASSERT( pFrm, "Node nicht in einer Tabelle" );

        while ( pFrm )
        {
            if( pFrm && aReg.GetOrigin().IsOver( pFrm->Frm() ) )
                aReg -= pFrm->Frm();

            pFrm = pFrm->GetNextCellLeaf( MAKEPAGE_NONE );
        }
    }
    aReg.Invert();
    Insert( &aReg, 0 );
}


// Pruefe, ob sich der SPoint innerhalb der Tabellen-SSelection befindet
BOOL SwShellTableCrsr::IsInside( const Point& rPt ) const
{
    // die neuen Rechtecke berechnen
    // JP 16.01.98: wenn der Cursor noch "geparkt" ist nichts machen!!
    if( !aSelBoxes.Count() || bParked ||
        !GetPoint()->nNode.GetIndex()  )
        return FALSE;

    SwNodes& rNds = GetDoc()->GetNodes();
    for( USHORT n = 0; n < aSelBoxes.Count(); ++n )
    {
        SwNodeIndex aIdx( *(*(aSelBoxes.GetData() + n ))->GetSttNd() );
        SwCntntNode* pCNd = rNds.GoNextSection( &aIdx, TRUE, FALSE );
        if( !pCNd )
            continue;

        SwFrm* pFrm = pCNd->GetFrm( &GetPtPos() );
        while( pFrm && !pFrm->IsCellFrm() )
            pFrm = pFrm->GetUpper();
        ASSERT( pFrm, "Node nicht in einer Tabelle" );
        if( pFrm && pFrm->Frm().IsInside( rPt ) )
            return TRUE;
    }
    return FALSE;
}

#ifndef PRODUCT

// JP 05.03.98: zum Testen des UNO-Crsr Verhaltens hier die Implementierung
//              am sichtbaren Cursor
BOOL SwShellTableCrsr::IsSelOvr( int eFlags )
{
    return SwShellCrsr::IsSelOvr( eFlags );
}

#endif

BOOL SwShellTableCrsr::IsAtValidPos( BOOL bPoint ) const
{
    return SwShellCrsr::IsAtValidPos( bPoint );
}

