/*************************************************************************
 *
 *  $RCSfile: notxtfrm.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: jp $ $Date: 2001-03-08 21:19:07 $
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

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _URLOBJ_HXX //autogen
#include <tools/urlobj.hxx>
#endif
#ifndef _SV_PRINT_HXX
#include <vcl/print.hxx>
#endif
#ifndef _SV_VIRDEV_HXX //autogen
#include <vcl/virdev.hxx>
#endif
#ifndef _SV_POLY_HXX
#include <vcl/poly.hxx>
#endif

#ifndef _GOODIES_IMAPOBJ_HXX
#include <svtools/imapobj.hxx>
#endif
#ifndef _IMAP_HXX //autogen
#include <svtools/imap.hxx>
#endif
#ifndef SVTOOLS_URIHELPER_HXX
#include <svtools/urihelper.hxx>
#endif
#ifndef _IPOBJ_HXX //autogen
#include <so3/ipobj.hxx>
#endif
#ifndef _IPENV_HXX //autogen
#include <so3/ipenv.hxx>
#endif
#ifndef _SOERR_HXX //autogen
#include <so3/soerr.hxx>
#endif
#ifndef _SFX_PROGRESS_HXX //autogen
#include <sfx2/progress.hxx>
#endif
#ifndef _SFXDOCFILE_HXX //autogen
#include <sfx2/docfile.hxx>
#endif
#ifndef _SFX_PRINTER_HXX //autogen
#include <sfx2/printer.hxx>
#endif
#ifndef _SVX_UDLNITEM_HXX //autogen
#include <svx/udlnitem.hxx>
#endif
#ifndef _SVX_COLRITEM_HXX //autogen
#include <svx/colritem.hxx>
#endif
#ifndef _XOUTBMP_HXX //autogen
#include <svx/xoutbmp.hxx>
#endif

#ifndef _FMTURL_HXX
#include <fmturl.hxx>
#endif
#ifndef _FMTSRND_HXX
#include <fmtsrnd.hxx>
#endif
#ifndef _FRMFMT_HXX
#include <frmfmt.hxx>
#endif
#ifndef _SWRECT_HXX
#include <swrect.hxx>
#endif
#ifndef _FESH_HXX
#include <fesh.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
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
#ifndef _VIEWIMP_HXX
#include <viewimp.hxx>
#endif
#ifndef _PAM_HXX
#include <pam.hxx>
#endif
#ifndef _HINTS_HXX
#include <hints.hxx>
#endif
#ifndef _ROOTFRM_HXX
#include <rootfrm.hxx>
#endif
#ifndef _DFLYOBJ_HXX
#include <dflyobj.hxx>
#endif
#ifndef _PAGEFRM_HXX
#include <pagefrm.hxx>
#endif
#ifndef _NOTXTFRM_HXX
#include <notxtfrm.hxx>
#endif
#ifndef _GRFATR_HXX
#include <grfatr.hxx>
#endif
#ifndef _CHARATR_HXX
#include <charatr.hxx>
#endif
#ifndef _FMTORNT_HXX
#include <fmtornt.hxx>
#endif
#ifndef _NDNOTXT_HXX
#include <ndnotxt.hxx>
#endif
#ifndef _NDGRF_HXX
#include <ndgrf.hxx>
#endif
#ifndef _NDOLE_HXX
#include <ndole.hxx>
#endif
#ifndef _SWREGION_HXX
#include <swregion.hxx>
#endif
#ifndef _POOLFMT_HXX
#include <poolfmt.hxx>
#endif
#ifndef _FRMSH_HXX
#include <frmsh.hxx>
#endif

#ifndef _MDIEXP_HXX
#include <mdiexp.hxx>
#endif
#ifndef _SWWAIT_HXX
#include <swwait.hxx>
#endif
#ifndef _COMCORE_HRC
#include <comcore.hrc>
#endif

#define DEFTEXTSIZE  12

//Zum asynchronen (erstmaligem) anfordern von Grafiken
class SwRequestGraphic : public SwClient
{
    Timer aTimer;
    ViewShell* pSh;
public:
    SwRequestGraphic( ViewShell& rVSh, SwGrfNode& rNd );
    virtual void Modify( SfxPoolItem *pOld, SfxPoolItem *pNew );
    DECL_STATIC_LINK( SwRequestGraphic, RequestGraphic, void *);
};


extern void ClrContourCache( const SdrObject *pObj ); // TxtFly.Cxx


inline BOOL GetRealURL( const SwGrfNode& rNd, String& rTxt )
{
    BOOL bRet = rNd.GetFileFilterNms( &rTxt, 0 );
    if( bRet )
        rTxt = URIHelper::removePassword( rTxt, INetURLObject::WAS_ENCODED,
                                           INetURLObject::DECODE_UNAMBIGUOUS);
    return bRet;
}

SwRequestGraphic::SwRequestGraphic( ViewShell& rVSh, SwGrfNode& rNd )
    : SwClient( &rNd ), pSh( &rVSh )
{
    aTimer.SetTimeout( 1 );
    aTimer.SetTimeoutHdl( STATIC_LINK( this, SwRequestGraphic, RequestGraphic ) );
    aTimer.Start();
}


void SwRequestGraphic::Modify( SfxPoolItem *pOld, SfxPoolItem *pNew )
{
    if( pOld && RES_OBJECTDYING == pOld->Which() &&
        ((SwPtrMsgPoolItem *)pOld)->pObject == pRegisteredIn )
    {
        pRegisteredIn->Remove( this );
        aTimer.Stop();
        delete this;
    }
}


IMPL_STATIC_LINK( SwRequestGraphic, RequestGraphic, void*, EMPTYARG )
{
    if ( pThis->GetRegisteredIn() && GRAPHIC_DEFAULT ==
        ((SwGrfNode*)pThis->GetRegisteredIn())->GetGrf().GetType() )
    {
        SwGrfNode* pGrfNd = (SwGrfNode*)pThis->GetRegisteredIn();
        ViewShell* pVSh, *pTmpSh;
        pGrfNd->GetDoc()->GetEditShell( &pVSh );
        if( pVSh )
        {
            pTmpSh = pVSh;
            // existiert die Shell noch?
            do {
                if( pThis->pSh == pTmpSh )
                {
                    CurrShell aTmp( pTmpSh );
                    pGrfNd->SetTransferPriority( SFX_TFPRIO_VISIBLE_HIGHRES_GRAPHIC );
                    pGrfNd->SwapIn();
                }
            } while( pVSh != ( pTmpSh = (ViewShell*)pTmpSh->GetNext()) );
        }
    }
    delete pThis;
    return 0;
}


void lcl_PaintReplacement( const SwRect &rRect, const String &rText,
                           const ViewShell &rSh, const SwNoTxtFrm *pFrm,
                           FASTBOOL bDefect )
{
    static Font *pFont = 0;
    if ( !pFont )
    {
        pFont = new Font();
        pFont->SetWeight( WEIGHT_BOLD );
        pFont->SetStyleName( aEmptyStr );
        pFont->SetName( String::CreateFromAscii(
                            RTL_CONSTASCII_STRINGPARAM( "Helvetica" )));
        pFont->SetFamily( FAMILY_SWISS );
        pFont->SetTransparent( TRUE );
    }

    Color aCol( COL_RED );
    FontUnderline eUnderline = UNDERLINE_NONE;
    const SwFmtURL &rURL = pFrm->FindFlyFrm()->GetFmt()->GetURL();
    if( rURL.GetURL().Len() || rURL.GetMap() )
    {
        FASTBOOL bVisited = FALSE;
        if ( rURL.GetMap() )
        {
            ImageMap *pMap = (ImageMap*)rURL.GetMap();
            for( USHORT i = 0; i < pMap->GetIMapObjectCount(); i++ )
            {
                IMapObject *pObj = pMap->GetIMapObject( i );
                if( rSh.GetDoc()->IsVisitedURL( pObj->GetURL() ) )
                {
                    bVisited = TRUE;
                    break;
                }
            }
        }
        else if ( rURL.GetURL().Len() )
            bVisited = rSh.GetDoc()->IsVisitedURL( rURL.GetURL() );

        SwFmt *pFmt = rSh.GetDoc()->GetFmtFromPool( bVisited ?
                RES_POOLCHR_INET_VISIT : RES_POOLCHR_INET_NORMAL );
        aCol = pFmt->GetColor().GetValue();
        eUnderline = pFmt->GetUnderline().GetUnderline();
    }

    pFont->SetUnderline( eUnderline );
    pFont->SetColor( aCol );

    const Bitmap& rBmp = SwNoTxtFrm::GetBitmap( bDefect );
    ((Graphic*)0)->Draw( rSh.GetOut(), rText, *pFont, rBmp,
                         rRect.Pos(), rRect.SSize() );
}

const Bitmap& SwNoTxtFrm::GetBitmap( BOOL bErrorBmp )
{
    Bitmap** ppRet;
    USHORT nResId;
    if( bErrorBmp )
    {
        ppRet = &pErrorBmp;
        nResId = RID_GRAPHIC_ERRORBMP;
    }
    else
    {
        ppRet = &pReplaceBmp;
        nResId = RID_GRAPHIC_REPLACEBMP;
    }

    if( !*ppRet )
        *ppRet = new Bitmap( SW_RES( nResId ) );
    return **ppRet;
}

/*************************************************************************
|*
|*    SwGrfFrm::SwGrfFrm(ViewShell * const,SwGrfNode *)
|*
|*    Beschreibung
|*    Ersterstellung    JP 05.03.91
|*    Letzte Aenderung  MA 03. Mar. 93
|*
*************************************************************************/


SwNoTxtFrm::SwNoTxtFrm(SwNoTxtNode * const pNode)
    : SwCntntFrm(pNode)
{
    InitCtor();
}

// Initialisierung: z.Zt. Eintragen des Frames im Cache


void SwNoTxtFrm::InitCtor()
{
    nType = FRM_NOTXT;
    // Das Gewicht der Grafik ist 0, wenn sie noch nicht
    // gelesen ist, < 0, wenn ein Lesefehler auftrat und
    // Ersatzdarstellung angewendet werden musste und >0,
    // wenn sie zur Verfuegung steht.
    nWeight = 0;
}

/*************************************************************************
|*
|*    SwNoTxtNode::MakeFrm()
|*
|*    Beschreibung
|*    Ersterstellung    JP 05.03.91
|*    Letzte Aenderung  MA 03. Mar. 93
|*
*************************************************************************/


SwCntntFrm *SwNoTxtNode::MakeFrm()
{
    return new SwNoTxtFrm(this);
}

/*************************************************************************
|*
|*    SwNoTxtFrm::~SwNoTxtFrm()
|*
|*    Beschreibung
|*    Ersterstellung    JP 05.03.91
|*    Letzte Aenderung  MA 30. Apr. 96
|*
*************************************************************************/

SwNoTxtFrm::~SwNoTxtFrm()
{
    StopAnimation();
}

/*************************************************************************
|*
|*    void SwNoTxtFrm::Modify( SwHint * pOld, SwHint * pNew )
|*
|*    Beschreibung
|*    Ersterstellung    JP 05.03.91
|*    Letzte Aenderung  JP 05.03.91
|*
*************************************************************************/

void SetOutDev( ViewShell *pSh, OutputDevice *pOut )
{
    pSh->pOut = pOut;
}




void lcl_ClearArea( const SwFrm &rFrm,
                    OutputDevice &rOut, const SwRect& rPtArea,
                    const SwRect &rGrfArea, BOOL bIsOLE )
{
    SwRegionRects aRegion( rPtArea, 4, 4 );
    aRegion -= rGrfArea;

    if ( aRegion.Count() )
    {
        const SvxBrushItem *pItem; const Color *pCol; SwRect aOrigRect;
        if ( rFrm.GetBackgroundBrush( pItem, pCol, aOrigRect, FALSE ) )
            for( USHORT i = 0; i < aRegion.Count(); ++i )
                ::DrawGraphic( pItem, &rOut, aOrigRect, aRegion[i] );
        else
        {
            rOut.Push( PUSH_FILLCOLOR );
            rOut.SetFillColor( rFrm.GetShell()->Imp()->GetRetoucheColor());
            for( USHORT i = 0; i < aRegion.Count(); ++i )
                rOut.DrawRect( aRegion[i].SVRect() );
            rOut.Pop();
        }
    }
}

/*************************************************************************
|*
|*    void SwNoTxtFrm::Paint()
|*
|*    Beschreibung
|*    Ersterstellung    JP 05.03.91
|*    Letzte Aenderung  MA 10. Jan. 97
|*
*************************************************************************/

void SwNoTxtFrm::Paint( const SwRect &rRect ) const
{
    if ( Frm().IsEmpty() )
        return;

    const ViewShell* pSh = GetShell();
    if( !pSh->GetViewOptions()->IsGraphic() )
    {
        StopAnimation();
        if ( pSh->GetWin() )
        {
            const SwNoTxtNode* pNd = GetNode()->GetNoTxtNode();
            String aTxt( pNd->GetAlternateText() );
            if ( !aTxt.Len() && pNd->IsGrfNode() )
                GetRealURL( *(SwGrfNode*)pNd, aTxt );
            if( !aTxt.Len() )
                aTxt = FindFlyFrm()->GetFmt()->GetName();
            lcl_PaintReplacement( Frm(), aTxt, *pSh, this, FALSE );
        }
        return;
    }

    if ( pSh->Imp()->IsPaintInScroll() && pSh->GetWin() && rRect != Frm() &&
         HasAnimation() )
    {
        pSh->GetWin()->Invalidate( Frm().SVRect() );
        return;
    }


    SfxProgress::EnterLock(); //Keine Progress-Reschedules im Paint (SwapIn)

    OutputDevice *pOut = pSh->GetOut();
    pOut->Push();
    BOOL bClip = TRUE;
    PolyPolygon aPoly;
    if ( (!pOut->GetConnectMetaFile() || pOut->GetOutDevType() == OUTDEV_PRINTER) &&
         FindFlyFrm()->GetContour( aPoly ) )
    {
        pOut->SetClipRegion( aPoly );
        bClip = FALSE;
    }

    SwRect aOrigPaint( rRect );
    if ( HasAnimation() && pSh->GetWin() )
    {
        aOrigPaint = Frm(); aOrigPaint += Prt().Pos();
    }

    SwRect aGrfArea( Frm() );
    SwRect aPaintArea( aGrfArea );
    aPaintArea._Intersection( aOrigPaint );

    SwRect aNormal( Frm().Pos() + Prt().Pos(), Prt().SSize() );
    aNormal.Justify(); //Normalisiertes Rechteck fuer die Vergleiche

    BOOL bIsOleNode = GetNode()->IsOLENode();
    if( aPaintArea.IsOver( aNormal ) )
    {
        // berechne die 4 zu loeschenden Rechtecke
        if( pSh->GetWin() )
            ::lcl_ClearArea( *this, *pSh->GetOut(), aPaintArea, aNormal,
                            bIsOleNode );

        // in der Schnittmenge vom PaintBereich und der Bitmap liegt
        // der absolut sichtbare Bereich vom Frame
        aPaintArea._Intersection( aNormal );

        if ( bClip )
            pOut->IntersectClipRegion( aPaintArea.SVRect() );
        PaintPicture( pOut, aGrfArea, aNormal );
    }
    else
        // wenn nicht sichtbar, loesche einfach den angegebenen Bereich
        lcl_ClearArea( *this, *pSh->GetOut(), aPaintArea, SwRect(),
                        bIsOleNode );

    pOut->Pop();
    SfxProgress::LeaveLock();
}


/*************************************************************************
|*
|*    void SwNoTxtFrm::GetGrfArea()
|*
|*    Beschreibung      Errechne die Position und die Groesse der Bitmap
|*                      innerhalb des uebergebenem Rechtecks.
|*
|*    Ersterstellung    JP 03.09.91
|*    Letzte Aenderung  MA 11. Oct. 94
|*
*************************************************************************/

void SwNoTxtFrm::GetGrfArea( SwRect &rRect, SwRect* pOrigRect,
                             BOOL ) const
{
    // JP 23.01.2001: currently only used for scaling the contour of graphics!
    //                  all other is handled by the GraphicObject

    //In rRect wird das sichbare Rechteck der Grafik gesteckt.
    //In pOrigRect werden Pos+Size der Gesamtgrafik gesteck.

    const SwAttrSet& rAttrSet = GetNode()->GetSwAttrSet();
    const SwCropGrf& rCrop = rAttrSet.GetCropGrf();

    //Grafik wird vom Node eingelesen falls notwendig. Kann aber schiefgehen.
    long nLeftCrop, nRightCrop, nTopCrop, nBottomCrop;
    Size aOrigSz( ((SwNoTxtNode*)GetNode())->GetTwipSize() );
    if ( !aOrigSz.Width() )
    {
        aOrigSz.Width() = Prt().Width();
        nLeftCrop  = -rCrop.GetLeft();
        nRightCrop = -rCrop.GetRight();
    }
    else
    {
        nLeftCrop = Max( aOrigSz.Width() -
                            (rCrop.GetRight() + rCrop.GetLeft()), long(1) );
        const double nScale = double(Prt().Width())  / double(nLeftCrop);
        nLeftCrop  = long(nScale * -rCrop.GetLeft() );
        nRightCrop = long(nScale * -rCrop.GetRight() );
    }

    if( !aOrigSz.Height() )
    {
        aOrigSz.Height() = Prt().Height();
        nTopCrop   = -rCrop.GetTop();
        nBottomCrop= -rCrop.GetBottom();
    }
    else
    {
        nTopCrop = Max( aOrigSz.Height() - (rCrop.GetTop() + rCrop.GetBottom()), long(1) );
        const double nScale = double(Prt().Height()) / double(nTopCrop);
        nTopCrop   = long(nScale * -rCrop.GetTop() );
        nBottomCrop= long(nScale * -rCrop.GetBottom() );
    }

    Size  aVisSz( Prt().SSize() );
    Size  aGrfSz( aVisSz );
    Point aVisPt( Frm().Pos() + Prt().Pos() );
    Point aGrfPt( aVisPt );

    //Erst das 'sichtbare' Rect einstellen.
    if ( nLeftCrop > 0 )
    {
        aVisPt.X()  += nLeftCrop;
        aVisSz.Width() -= nLeftCrop;
    }
    if ( nTopCrop > 0 )
    {
        aVisPt.Y()   += nTopCrop;
        aVisSz.Height() -= nTopCrop;
    }
    if ( nRightCrop > 0 )
        aVisSz.Width() -= nRightCrop;
    if ( nBottomCrop > 0 )
        aVisSz.Height() -= nBottomCrop;

    rRect.Pos  ( aVisPt );
    rRect.SSize( aVisSz );

    //Ggf. Die Gesamtgrafik berechnen
    if ( pOrigRect )
    {
        Size aTmpSz( aGrfSz );
        aGrfPt.X()    += nLeftCrop;
        aTmpSz.Width() -= nLeftCrop + nRightCrop;
        aGrfPt.Y()      += nTopCrop;
        aTmpSz.Height()-= nTopCrop + nBottomCrop;

        pOrigRect->Pos  ( aGrfPt );
        pOrigRect->SSize( aTmpSz );
    }
}

/*************************************************************************
|*
|*    Size SwNoTxtFrm::GetSize()
|*
|*    Beschreibung      Gebe die Groesse des umgebenen FLys und
|*                      damit die der Grafik zurueck.
|*    Ersterstellung    JP 04.03.91
|*    Letzte Aenderung  JP 31.08.94
|*
*************************************************************************/


const Size& SwNoTxtFrm::GetSize() const
{
    // gebe die Groesse des Frames zurueck
    const SwFrm *pFly = FindFlyFrm();
    if( !pFly )
        pFly = this;
    return pFly->Prt().SSize();
}

/*************************************************************************
|*
|*    SwNoTxtFrm::MakeAll()
|*
|*    Ersterstellung    MA 29. Nov. 96
|*    Letzte Aenderung  MA 29. Nov. 96
|*
*************************************************************************/


void SwNoTxtFrm::MakeAll()
{
    SwCntntNotify aNotify( this );
    SwBorderAttrAccess aAccess( SwFrm::GetCache(), this );
    const SwBorderAttrs &rAttrs = *aAccess.Get();

    while ( !bValidPos || !bValidSize || !bValidPrtArea )
    {
        MakePos();

        if ( !bValidSize )
            Frm().Width( GetUpper()->Prt().Width() );

        MakePrtArea( rAttrs );

        if ( !bValidSize )
        {   bValidSize = TRUE;
            Format();
        }
    }
}

/*************************************************************************
|*
|*    SwNoTxtFrm::Format()
|*
|*    Beschreibung      Errechne die Groesse der Bitmap, wenn noetig
|*    Ersterstellung    JP 11.03.91
|*    Letzte Aenderung  MA 13. Mar. 96
|*
*************************************************************************/


void SwNoTxtFrm::Format( const SwBorderAttrs * )
{
    const Size aNewSize( GetSize() );

    // hat sich die Hoehe geaendert?
    SwTwips nChgHght = (SwTwips)(aNewSize.Height() - Prt().Height());
    const SzPtr pVar = pVARSIZE;
    if( nChgHght > 0)
        Grow( nChgHght, pVar );
    else if( nChgHght < 0)
        Shrink( Min(Prt().Height(), -nChgHght), pVar );
}

/*************************************************************************
|*
|*    SwNoTxtFrm::GetCharRect()
|*
|*    Beschreibung
|*    Ersterstellung    SS 29-Apr-1991
|*    Letzte Aenderung  MA 10. Oct. 94
|*
|*************************************************************************/


BOOL SwNoTxtFrm::GetCharRect( SwRect &rRect, const SwPosition& rPos,
                              SwCrsrMoveState *pCMS ) const
{
    if ( &rPos.nNode.GetNode() != (SwNode*)GetNode() )
        return FALSE;

    Calc();
    SwRect aFrameRect( Frm() );
    rRect = aFrameRect;
    rRect.Pos( Frm().Pos() + Prt().Pos() );
    rRect.SSize( Prt().SSize() );

    rRect.Justify();

    // liegt die Bitmap ueberhaupt im sichtbaren Berich ?
    if( !aFrameRect.IsOver( rRect ) )
    {
        // wenn nicht dann steht der Cursor auf dem Frame
        rRect = aFrameRect;
        rRect.Width( 1 );
    }
    else
        rRect._Intersection( aFrameRect );

    if ( pCMS )
    {
        if ( pCMS->bRealHeight )
        {
            pCMS->aRealHeight.Y() = rRect.Height();
            pCMS->aRealHeight.X() = 0;
        }
    }

    return TRUE;
}


BOOL SwNoTxtFrm::GetCrsrOfst(SwPosition* pPos, Point& aPoint,
                            const SwCrsrMoveState* ) const
{
    SwCntntNode* pCNd = (SwCntntNode*)GetNode();
    pPos->nNode = *pCNd;
    pPos->nContent.Assign( pCNd, 0 );
    return TRUE;
}

#define CLEARCACHE( pNd ) {\
    (pNd)->GetGrfObj().ReleaseFromCache();\
    SwFlyFrm* pFly = FindFlyFrm();\
    if( pFly && pFly->GetFmt()->GetSurround().IsContour() )\
    {\
        ClrContourCache( pFly->GetVirtDrawObj() );\
        pFly->NotifyBackground( FindPageFrm(), Prt(), PREP_FLY_ATTR_CHG );\
    }\
}

void SwNoTxtFrm::Modify( SfxPoolItem* pOld, SfxPoolItem* pNew )
{
    USHORT nWhich = pNew ? pNew->Which() : pOld ? pOld->Which() : 0;

    if ( RES_GRAPHIC_PIECE_ARRIVED != nWhich &&
         RES_GRAPHIC_ARRIVED != nWhich &&
         RES_GRF_REREAD_AND_INCACHE != nWhich )
        SwCntntFrm::Modify( pOld, pNew );

    FASTBOOL bCompletePaint = TRUE;

    switch( nWhich )
    {
    case RES_OBJECTDYING:
        break;

    case RES_GRF_REREAD_AND_INCACHE:
        if( ND_GRFNODE == GetNode()->GetNodeType() )
        {
            bCompletePaint = FALSE;
            SwGrfNode* pNd = (SwGrfNode*) GetNode();

            ViewShell *pVSh = 0;
            pNd->GetDoc()->GetEditShell( &pVSh );
            if( pVSh )
            {
                GraphicAttr aAttr;
                if( pNd->GetGrfObj().IsCached( pVSh->GetOut(), Point(),
                            Prt().SSize(), &pNd->GetGraphicAttr( aAttr, this ) ))
                {
                    ViewShell *pSh = pVSh;
                    do {
                        SET_CURR_SHELL( pSh );
                        if( pSh->GetWin() )
                        {
                            if( pSh->IsPreView() )
                                ::RepaintPagePreview( pSh, Frm().SVRect() );
                            else
                                pSh->GetWin()->Invalidate( Frm().SVRect() );
                        }
                    } while( pVSh != (pSh = (ViewShell*)pSh->GetNext() ));
                }
                else
                    pNd->SwapIn();
            }
        }
        break;

    case RES_UPDATE_ATTR:
    case RES_FMT_CHG:
        CLEARCACHE( (SwGrfNode*) GetNode() )
        break;

    case RES_ATTRSET_CHG:
        {
            for( USHORT n = RES_GRFATR_BEGIN; n < RES_GRFATR_END; ++n )
                if( SFX_ITEM_SET == ((SwAttrSetChg*)pOld)->GetChgSet()->
                                GetItemState( n, FALSE ))
                {
                    CLEARCACHE( (SwGrfNode*) GetNode() )
                    break;
                }
            if( RES_GRFATR_END == n )           // not found
                return ;
        }
        break;

    case RES_GRAPHIC_PIECE_ARRIVED:
    case RES_GRAPHIC_ARRIVED:
        if ( GetNode()->GetNodeType() == ND_GRFNODE )
        {
            bCompletePaint = FALSE;
            SwGrfNode* pNd = (SwGrfNode*) GetNode();

            CLEARCACHE( pNd )

            SwRect aRect( Frm() );

            ViewShell *pVSh = 0;
            pNd->GetDoc()->GetEditShell( &pVSh );
            if( !pVSh )
                break;

            ViewShell *pSh = pVSh;
            do {
                SET_CURR_SHELL( pSh );
                if( pSh->IsPreView() )
                {
                    if( pSh->GetWin() )
                        ::RepaintPagePreview( pSh, aRect );
                }
                else if ( pSh->VisArea().IsOver( aRect ) &&
                     OUTDEV_WINDOW == pSh->GetOut()->GetOutDevType() )
                {
                    const GraphicObject& rGrfObj = pNd->GetGrfObj();
                    BOOL bSizeUnequal;
                    if( rGrfObj.IsAnimated() )
                        pSh->GetWin()->Invalidate( aRect.SVRect() );
                    else if ( pNd->IsTransparent() ||
                              (bSizeUnequal = (Frm().SSize() != Prt().SSize())))
                    {
                        if ( bSizeUnequal )
                        {
                            //Wieder mal die Umrandungen...
                            aRect.Pos().X() = Max( long(0),
                                            (long)(aRect.Left() - 20) );
                            aRect.Pos().Y() = Max( long(0),
                                            (long)(aRect.Top()  - 20) );
                            aRect.SSize().Width() += 40;
                            aRect.SSize().Height() += 40;
                        }
                        //virtuelles device erzeugen und einstellen.
                        OutputDevice *pOld = pSh->GetOut();
                        VirtualDevice *pVout = new VirtualDevice( *pOld );
                        MapMode aMapMode( pOld->GetMapMode() );
                        pVout->SetMapMode( aMapMode );
                        if( pVout->SetOutputSize( aRect.SSize() ) )
                        {
//                          pVout->SetPen( pOld->GetPen() );
                            pVout->SetLineColor(pOld->GetLineColor());
//                          pVout->SetFillInBrush( pOld->GetFillInBrush() );
                            pVout->SetFillColor(pOld->GetFillColor());
                            Point aOrigin( aRect.Pos() );
                            aOrigin.X() = -aOrigin.X(); aOrigin.Y() = -aOrigin.Y();
                            aMapMode.SetOrigin( aOrigin );
                            pVout->SetMapMode( aMapMode );
                            ::SetOutDev( pSh, pVout );
                            pSh->GetLayout()->Paint( aRect );
                            pOld->DrawOutDev( aRect.Pos(), aRect.SSize(),
                                              aRect.Pos(), aRect.SSize(), *pVout );
                            ::SetOutDev( pSh, pOld );
                        }
                        else
                            pSh->Paint( aRect.SVRect() );
                        delete pVout;
                    }
                    else
                    {
                        aRect = Prt(); aRect += Frm().Pos();
                        Paint( aRect );
                    }
                }

                pSh = (ViewShell *)pSh->GetNext();
            } while( pSh != pVSh );
        }
        break;

    default:
        if( !pNew || RES_GRFATR_BEGIN > nWhich || nWhich >= RES_GRFATR_END )
            return;
    }

    if( bCompletePaint )
    {
        InvalidatePrt();
        SetCompletePaint();
    }
}

// Ausgabe der Grafik. Hier wird entweder eine QuickDraw-Bmp oder
// eine Grafik vorausgesetzt. Ist nichts davon vorhanden, wird
// eine Ersatzdarstellung ausgegeben.

void SwNoTxtFrm::PaintPicture( OutputDevice* pOut, const SwRect &rGrfArea,
                               const SwRect &rPaintGrfArea ) const
{
    ViewShell* pShell = GetShell();
    const FASTBOOL bPrn = pOut == pShell->GetPrt() ||
                          pOut->GetConnectMetaFile();

    SwNoTxtNode& rNoTNd = *(SwNoTxtNode*)GetNode();
    SwGrfNode* pGrfNd = rNoTNd.GetGrfNode();
    SwOLENode* pOLENd = rNoTNd.GetOLENode();

    if( pGrfNd )
    {
        FASTBOOL bForceSwap = FALSE, bContinue = TRUE;
        pGrfNd->SetFrameInPaint( TRUE );
        GraphicObject& rGrfObj = pGrfNd->GetGrfObj();

        GraphicAttr aGrfAttr;
        pGrfNd->GetGraphicAttr( aGrfAttr, this );

        if( !bPrn )
        {
            if( GRAPHIC_DEFAULT == rGrfObj.GetType() &&
                pGrfNd->IsLinkedFile() )
            {
                Size aTmpSz;
                ::so3::SvLinkSource* pGrfObj = pGrfNd->GetLink()->GetObj();
                if( !pGrfObj ||
                    !pGrfObj->IsDataComplete() ||
                    !(aTmpSz = pGrfNd->GetTwipSize()).Width() ||
                    !aTmpSz.Height() || !pGrfNd->GetAutoFmtLvl() )
                {
                    pGrfNd->SetAutoFmtLvl( 1 );
                    new SwRequestGraphic( *GetShell(), *pGrfNd );//zerstoert sich selbst!
                }
                String aTxt( pGrfNd->GetAlternateText() );
                if ( !aTxt.Len() )
                    GetRealURL( *pGrfNd, aTxt );
                ::lcl_PaintReplacement( rGrfArea, aTxt, *pShell, this, FALSE );
                bContinue = FALSE;
            }
            else if( rGrfObj.IsCached( pOut, rGrfArea.Pos(),
                                    rGrfArea.SSize(), &aGrfAttr ))
            {
                rGrfObj.Draw( pOut, rGrfArea.Pos(), rGrfArea.SSize(),
                                &aGrfAttr );
                bForceSwap = TRUE;
                bContinue = FALSE;
            }
        }

        if( bContinue )
        {
            const FASTBOOL bSwapped = rGrfObj.IsSwappedOut();
            const FASTBOOL bSwappedIn = pGrfNd->SwapIn( bPrn );
            if( bSwappedIn && rGrfObj.GetGraphic().IsSupportedGraphic())
            {
                const FASTBOOL bAnimate = rGrfObj.IsAnimated() &&
                                            !pShell->IsPreView();
                if( bAnimate &&
                    FindFlyFrm() != ::GetFlyFromMarked( 0, pShell ))
                {
                    OutputDevice* pVout;
                    if( pOut == pShell->GetOut() && SwRootFrm::FlushVout() )
                        pVout = pOut, pOut = pShell->GetOut();
                    else if( pShell->GetWin() &&
                             OUTDEV_VIRDEV == pOut->GetOutDevType() )
                        pVout = pOut, pOut = pShell->GetWin();
                    else
                        pVout = 0;

                    ASSERT( OUTDEV_VIRDEV != pOut->GetOutDevType(),
                            "pOut sollte kein virtuelles Device sein" );
                    rGrfObj.StartAnimation( pOut, rGrfArea.Pos(),
                                        rGrfArea.SSize(), long(this),
                                        0, GRFMGR_DRAW_STANDARD, pVout );
                }
                else
                    rGrfObj.Draw( pOut, rGrfArea.Pos(), rGrfArea.SSize(),
                                    &aGrfAttr );
            }
            else
            {
                USHORT nResId = 0;
                if( bSwappedIn )
                {
                    if( GRAPHIC_NONE == rGrfObj.GetType() )
                        nResId = STR_COMCORE_READERROR;
                    else if ( !rGrfObj.GetGraphic().IsSupportedGraphic() )
                        nResId = STR_COMCORE_CANT_SHOW;
                }
                ((SwNoTxtFrm*)this)->nWeight = -1;
                String aText;
                if ( !nResId &&
                    !(aText = pGrfNd->GetAlternateText()).Len() &&
                     (!GetRealURL( *pGrfNd, aText ) || !aText.Len()))
                {
                    nResId = STR_COMCORE_READERROR;
                }
                if ( nResId )
                    aText = SW_RESSTR( nResId );

                ::lcl_PaintReplacement( rGrfArea, aText, *pShell, this, TRUE );
            }

            //Beim Drucken duerfen wir nicht die Grafiken sammeln...
            if( bSwapped && bPrn )
                bForceSwap = TRUE;
        }
        pGrfNd->SetFrameInPaint( FALSE );
        if( bForceSwap )
            pGrfNd->SwapOut();
    }
    else if( pOLENd )
    {
        SvInPlaceObjectRef xRef( pOLENd->GetOLEObj().GetOleRef() );

        // Im BrowseModus gibt es nicht unbedingt einen Drucker und
        // damit kein JobSetup, also legen wir eines an ...
        JobSetup* pJobSetup = (JobSetup*)pOLENd->GetDoc()->GetJobsetup();
        FASTBOOL bDummyJobSetup = 0 == pJobSetup;
        if( bDummyJobSetup )
            pJobSetup = new JobSetup();
        xRef->DoDraw( pOut, rGrfArea.Pos(), rGrfArea.SSize(), *pJobSetup );
        if( bDummyJobSetup )
            delete pJobSetup;  // ... und raeumen wieder auf.

        //Objecte mit die beim Sichtbarwerden aktiviert werden wollen
        //werden jetzt Connected (Aktivieren wird vom Sfx erledigt).
        if( !bPrn && ((SVOBJ_MISCSTATUS_ACTIVATEWHENVISIBLE|
            SVOBJ_MISCSTATUS_ALWAYSACTIVATE) & xRef->GetMiscStatus()) &&
            pShell->ISA( SwCrsrShell ))
        {
            const SwFlyFrm *pFly = FindFlyFrm();
            ASSERT( pFly, "OLE not in FlyFrm" );
            ((SwFEShell*)pShell)->ConnectObj( xRef, pFly->Prt(), pFly->Frm());
        }
    }
}


BOOL SwNoTxtFrm::IsTransparent() const
{
    const ViewShell* pSh = GetShell();
    if ( !pSh || !pSh->GetViewOptions()->IsGraphic() )
        return TRUE;

    const SwGrfNode *pNd;
    if( 0 != (pNd = GetNode()->GetGrfNode()) )
        return pNd->IsTransparent();

    //#29381# OLE sind immer Transparent.
    return TRUE;
}


void SwNoTxtFrm::StopAnimation( OutputDevice* pOut ) const
{
    //animierte Grafiken anhalten
    SwGrfNode* pGrfNd = (SwGrfNode*)GetNode()->GetGrfNode();
    if( pGrfNd && pGrfNd->IsAnimated() )
        pGrfNd->GetGrfObj().StopAnimation( pOut, long(this) );
}


BOOL SwNoTxtFrm::HasAnimation() const
{
    const SwGrfNode* pGrfNd = GetNode()->GetGrfNode();
    return pGrfNd && pGrfNd->IsAnimated();
}



