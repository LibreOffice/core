/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"

#include <tools/list.hxx>
#include <swtypes.hxx>
#include <hintids.hxx>
#include <com/sun/star/accessibility/XAccessible.hpp>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/i18n/XBreakIterator.hpp>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <com/sun/star/i18n/InputSequenceCheckMode.hpp>
#include <com/sun/star/i18n/UnicodeScript.hpp>
#include <vcl/help.hxx>
#include <vcl/graph.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/cmdevt.h>
#include <sot/storage.hxx>
#include <svl/macitem.hxx>
#include <unotools/securityoptions.hxx>
#include <basic/sbxvar.hxx>
#include <svl/ctloptions.hxx>
#include <basic/sbx.hxx>
#include <svl/eitem.hxx>
#include <svl/stritem.hxx>
#include <sfx2/ipclient.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/request.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <svl/ptitem.hxx>
#include <editeng/sizeitem.hxx>
#include <editeng/langitem.hxx>
#include <svx/htmlmode.hxx>
#include <svx/svdview.hxx>
#include <svx/svdhdl.hxx>
#include <svx/svdoutl.hxx>
#include <editeng/editeng.hxx>
#include <editeng/svxacorr.hxx>
#include <editeng/scripttypeitem.hxx>
#include <editeng/flditem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/brshitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/protitem.hxx>
#include <unotools/charclass.hxx>
#include <editeng/acorrcfg.hxx>
#include <SwSmartTagMgr.hxx>
#include <edtwin.hxx>
#include <view.hxx>
#include <wrtsh.hxx>
#include <IDocumentSettingAccess.hxx>
#include <fldbas.hxx>
#include <swmodule.hxx>
#include <docsh.hxx>
#include <viewopt.hxx>
#include <drawbase.hxx>
#include <dselect.hxx>
#include <textsh.hxx>
#include <shdwcrsr.hxx>
#include <txatbase.hxx>
#include <fmtanchr.hxx>
#include <fmtornt.hxx>
#include <fmtfsize.hxx>
#include <fmtclds.hxx>
#include <frmfmt.hxx>
#include <modcfg.hxx>
#include <fmtcol.hxx>
#include <wview.hxx>
#include <listsh.hxx>
#include <gloslst.hxx>
#include <inputwin.hxx>
#include <gloshdl.hxx>
#include <swundo.hxx>
#include <drwtxtsh.hxx>
#include <fchrfmt.hxx>
#include <fmturl.hxx>
#include <romenu.hxx>
#include <initui.hxx>
#include <frmatr.hxx>
#include <extinput.hxx>
#include <acmplwrd.hxx>
#include <swcalwrp.hxx>
#include <swdtflvr.hxx>
#include <wdocsh.hxx>
#include <crsskip.hxx>
#include <breakit.hxx>
#include <checkit.hxx>
#include <helpid.h>
#include <cmdid.h>
#include <docvw.hrc>
#include <uitool.hxx>
#include <fmtfollowtextflow.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <charfmt.hxx>
#include <numrule.hxx>
#include <pagedesc.hxx>
#include <svtools/ruler.hxx>
#include "formatclipboard.hxx"
#include <vos/mutex.hxx>
#include <vcl/svapp.hxx>
#include <IMark.hxx>
#include <doc.hxx>
#include <xmloff/odffields.hxx>
#include <PostItMgr.hxx>
#include <svx/svdlegacy.hxx>

using namespace sw::mark;
using namespace ::com::sun::star;

/*--------------------------------------------------------------------
    Beschreibung:   Globals
 --------------------------------------------------------------------*/

static bool bInputLanguageSwitched = false;
extern sal_Bool bNoInterrupt;       // in mainwn.cxx

//Normalerweise wird im MouseButtonUp eine Selektion aufgehoben wenn die
//Selektion nicht gerade aufgezogen wird. Leider wird im MouseButtonDown
//bei doppel-/dreifach-Klick Selektiert, diese Selektion wird in dem Handler
//komplett abgeschlossen und kann deshalb im Up nicht mehr unterschieden
//werden. Um dies Aufzuloese wird bHoldSelection im Down gesetzt und im
//Up ausgewertet.
static sal_Bool bHoldSelection      = sal_False;

sal_Bool bFrmDrag                   = sal_False;
sal_Bool bValidCrsrPos              = sal_False;
sal_Bool bModePushed                = sal_False;
sal_Bool bDDTimerStarted            = sal_False;
sal_Bool bFlushCharBuffer           = sal_False;
sal_Bool SwEditWin::bReplaceQuote   = sal_False;
sal_Bool bDDINetAttr                = sal_False;
SdrHdlKind eSdrMoveHdl          = HDL_USER;

QuickHelpData* SwEditWin::pQuickHlpData = 0;

long    SwEditWin::nDDStartPosY = 0;
long    SwEditWin::nDDStartPosX = 0;
Color   SwEditWin::aTextBackColor(COL_YELLOW);
Color   SwEditWin::aTextColor(COL_RED);
sal_Bool    SwEditWin::bTransparentBackColor = sal_False; // Hintergrund nicht transparent


extern sal_Bool     bExecuteDrag;

SfxShell* lcl_GetShellFromDispatcher( SwView& rView, const std::type_info& rType );

DBG_NAME(edithdl)

class SwAnchorMarker
{
private:
    SdrHdl*             mpHdl;
    basegfx::B2DPoint   maHdlPos;
    basegfx::B2DPoint   maLastPos;
    bool bTopRightHandle;
public:
    SwAnchorMarker( SdrHdl& rH )
    :   mpHdl( &rH ),
        maHdlPos( rH.getPosition() ),
        maLastPos( rH.getPosition() ),
        bTopRightHandle(rH.GetKind() == HDL_ANCHOR_TR)
    {
    }

    const basegfx::B2DPoint& GetLastPos() const { return maLastPos; }
    void SetLastPos( const basegfx::B2DPoint& rNew ) { maLastPos = rNew; }
    void SetPos( const basegfx::B2DPoint& rNew ) { mpHdl->setPosition( rNew ); }
    const basegfx::B2DPoint& GetPos() { return mpHdl->getPosition(); }
    const basegfx::B2DPoint& GetHdlPos() { return maHdlPos; }
    void ChgHdl( SdrHdl& rNew )
    {
        mpHdl = &rNew;
        bTopRightHandle = (rNew.GetKind() == HDL_ANCHOR_TR);
    }
    void SetSelected(bool bSelected)
    {
        mpHdl->SetSelected(bSelected);
    }

    // --> OD 2010-09-16 #i114522#
    const basegfx::B2DPoint GetPosForHitTest( const OutputDevice& rOut )
    {
        basegfx::B2DPoint aHitTestPos(rOut.GetViewTransformation() * GetPos());

        if ( bTopRightHandle )
        {
            aHitTestPos += basegfx::B2DPoint( -1.0, 1.0 );
        }
        else
        {
            aHitTestPos += basegfx::B2DPoint( 1.0, 1.0 );
        }

        aHitTestPos = rOut.GetInverseViewTransformation() * aHitTestPos;

        return aHitTestPos;
    }
};

struct QuickHelpData
{
    SvStringsISortDtor aArr;
    sal_uInt16* pAttrs;
    CommandExtTextInputData* pCETID;
    sal_uLong nTipId;
    sal_uInt16 nLen, nCurArrPos;
    sal_Bool bClear : 1, bChkInsBlank : 1, bIsTip : 1, bIsAutoText : 1;

    QuickHelpData() : pAttrs( 0 ), pCETID( 0 )  { ClearCntnt(); }

    void Move( QuickHelpData& rCpy );
    void ClearCntnt();
    void Start( SwWrtShell& rSh, sal_uInt16 nWrdLen );
    void Stop( SwWrtShell& rSh );

    sal_Bool HasCntnt() const       { return aArr.Count() && 0 != nLen; }

    void Inc( sal_Bool bEndLess )
        {
            if( ++nCurArrPos >= aArr.Count() )
                nCurArrPos = (bEndLess && !bIsAutoText )? 0 : nCurArrPos-1;
        }
    void Dec( sal_Bool bEndLess )
        {
            if( 0 == nCurArrPos-- )
                nCurArrPos = (bEndLess && !bIsAutoText ) ? aArr.Count()-1 : 0;
        }
    void FillStrArr( SwWrtShell& rSh, const String& rWord );
};


/*--------------------------------------------------------------------
    Beschreibung:   Minimale Bewegung Zittern vermeiden
 --------------------------------------------------------------------*/

#define HIT_PIX  2 /* Hit-Toleranz in Pixel */
#define MIN_MOVE 4

inline sal_Bool IsMinMove(const Point &rStartPos, const Point &rLPt)
{
    return Abs(rStartPos.X() - rLPt.X()) > MIN_MOVE ||
           Abs(rStartPos.Y() - rLPt.Y()) > MIN_MOVE;
}

/*--------------------------------------------------------------------
    JP 30.07.98: fuer MouseButtonDown - feststellen, ob ein DrawObject
                und KEIN SwgFrame getroffen wurde! Shift/Ctrl sollen
                nur bei DrawObjecte zum Selektieren fuehren, bei SwgFlys
                ggfs zum ausloesen von Hyperlinks (DownLoad/NewWindow!)
 --------------------------------------------------------------------*/
inline sal_Bool IsDrawObjSelectable( const SwWrtShell& rSh, const Point& rPt )
{
    sal_Bool bRet = sal_True;
    SdrObject* pObj;
    switch( rSh.GetObjCntType( rPt, pObj ))
    {
    case OBJCNT_NONE:
    case OBJCNT_FLY:
    case OBJCNT_GRF:
    case OBJCNT_OLE:
        bRet = sal_False;
        break;
    default:; //prevent warning
    }
    return bRet;
}

/*--------------------------------------------------------------------
    Beschreibung:   Pointer umschalten
 --------------------------------------------------------------------*/


void SwEditWin::UpdatePointer(const Point &rLPt, sal_uInt16 nModifier )
{
    const basegfx::B2DPoint aB2DLPt(rLPt.X(), rLPt.Y());
    SwWrtShell &rSh = rView.GetWrtShell();
    if( pApplyTempl )
    {
        PointerStyle eStyle = POINTER_FILL;
        if( rSh.IsOverReadOnlyPos( rLPt ))
        {
            if( pUserMarker )
            {
                delete pUserMarker;
                pUserMarker = 0L;
            }
            eStyle = POINTER_NOTALLOWED;
        }
        else
        {
            SwRect aRect;
            SwRect* pRect = &aRect;
            const SwFrmFmt* pFmt = 0;

            bool bFrameIsValidTarget = false;
            if( pApplyTempl->pFormatClipboard )
                bFrameIsValidTarget = pApplyTempl->pFormatClipboard->HasContentForThisType( nsSelectionType::SEL_FRM );
            else if( !pApplyTempl->nColor )
                bFrameIsValidTarget = ( pApplyTempl->eType == SFX_STYLE_FAMILY_FRAME );

            if( bFrameIsValidTarget &&
                        0 !=(pFmt = rSh.GetFmtFromObj( rLPt, &pRect )) &&
                        dynamic_cast< const SwFlyFrmFmt* >( pFmt))
            {
                //Highlight fuer Rahmen anwerfen
                Rectangle aTmp( pRect->SVRect() );

                if ( !pUserMarker )
                {
                    pUserMarker = new SdrDropMarkerOverlay( *rSh.GetDrawView(), aTmp );
                }
            }
            else if(pUserMarker)
            {
                delete pUserMarker;
                pUserMarker = 0L;
            }

            rSh.SwCrsrShell::SetVisCrsr( rLPt );
        }
        SetPointer( eStyle );
        return;
    }

    if( !rSh.VisArea().Width() )
        return;

    SET_CURR_SHELL(&rSh);

    if ( IsChainMode() )
    {
        SwRect aRect;
        int nChainable = rSh.Chainable( aRect, *rSh.GetFlyFrmFmt(), rLPt );
        PointerStyle eStyle = nChainable
                ? POINTER_CHAIN_NOTALLOWED : POINTER_CHAIN;
        if ( !nChainable )
        {
            Rectangle aTmp( aRect.SVRect() );

            if ( !pUserMarker )
            {
                pUserMarker = new SdrDropMarkerOverlay( *rSh.GetDrawView(), aTmp );
            }
        }
        else
        {
            delete pUserMarker;
            pUserMarker = 0L;
        }

        rView.GetViewFrame()->ShowStatusText(
                                        SW_RESSTR(STR_CHAIN_OK+nChainable));
        SetPointer( eStyle );
        return;
    }

    // Removed ExecHyperlink option.
    //sal_Bool bExecHyperlinks = rSh.GetViewOptions()->IsExecHyperlinks() ^
    //                     (nModifier == KEY_MOD2 ? sal_True : sal_False);
    sal_Bool bExecHyperlinks = rView.GetDocShell()->IsReadOnly();
    if ( !bExecHyperlinks )
    {
        SvtSecurityOptions aSecOpts;
        const sal_Bool bSecureOption = aSecOpts.IsOptionSet( SvtSecurityOptions::E_CTRLCLICK_HYPERLINK );
        if ( (  bSecureOption && nModifier == KEY_MOD1 ) ||
             ( !bSecureOption && nModifier != KEY_MOD1 ) )
            bExecHyperlinks = sal_True;
    }

    const sal_Bool bExecSmarttags  = nModifier == KEY_MOD1;

    SdrView *pSdrView = rSh.GetDrawView();
    sal_Bool bPrefSdrPointer = sal_False;
    sal_Bool bHitHandle = sal_False;
    sal_Bool bCntAtPos = sal_False;
    sal_Bool bIsDocReadOnly = rView.GetDocShell()->IsReadOnly() &&
                          rSh.IsCrsrReadonly();
    aActHitType = SDRHIT_NONE;
    PointerStyle eStyle = POINTER_TEXT;
    if ( !pSdrView )
        bCntAtPos = sal_True;
    else if ( sal_True == (bHitHandle = pSdrView->PickHandle( aB2DLPt ) != 0) )
    {
        aActHitType = SDRHIT_OBJECT;
        bPrefSdrPointer = sal_True;
    }
    else
    {
        const sal_Bool bNotInSelObj = !rSh.IsInsideSelectedObj( aB2DLPt );
        if ( rView.GetDrawFuncPtr() && !bInsDraw && bNotInSelObj )
        {
            aActHitType = SDRHIT_OBJECT;
            if (IsObjectSelect())
                eStyle = POINTER_ARROW;
            else
                bPrefSdrPointer = sal_True;
        }
        else
        {
            SdrObject* pObj = 0;
            pSdrView->SetHitTolerancePixel( HIT_PIX );
            if ( bNotInSelObj && bExecHyperlinks &&
                 pSdrView->PickObj( aB2DLPt, pSdrView->getHitTolLog(), pObj, SDRSEARCH_PICKMACRO ))
            {
                SdrObjMacroHitRec aTmp;
                aTmp.maPos = aB2DLPt;
                aTmp.mpSdrView = pSdrView;
                SetPointer( pObj->GetMacroPointer( aTmp ) );
                return;
            }
            else
            {
                // dvo: IsObjSelectable() eventually calls SdrView::PickObj, so
                // apparently this is used to determine whether this is a
                // drawling layer object or not.
                if ( rSh.IsObjSelectable( rLPt ) )
                {
                    if (pSdrView->IsTextEdit())
                    {
                        aActHitType = SDRHIT_NONE;
                        bPrefSdrPointer = sal_True;
                    }
                    else
                    {
                        SdrViewEvent aVEvt;
                        SdrHitKind eHit = pSdrView->PickAnything(aB2DLPt, aVEvt);

                        if (eHit == SDRHIT_URLFIELD && bExecHyperlinks)
                        {
                            aActHitType = SDRHIT_OBJECT;
                            bPrefSdrPointer = sal_True;
                        }
                        else
                        {
                            // if we're over a selected object, we show an
                            // ARROW by default. We only show a MOVE if 1) the
                            // object is selected, and 2) it may be moved
                            // (i.e., position is not protected).
                            bool bMovable =
                                (!bNotInSelObj) &&
                                (rSh.IsObjSelected() || rSh.IsFrmSelected()) &&
                                (!rSh.IsSelObjProtected(FLYPROTECT_POS));

                            eStyle = bMovable ? POINTER_MOVE : POINTER_ARROW;
                            aActHitType = SDRHIT_OBJECT;
                        }
                    }
                }
                else
                {
                    if ( rSh.IsFrmSelected() && !bNotInSelObj )
                    {
                        // dvo: this branch appears to be dead and should be
                        // removed in a future version. Reason: The condition
                        // !bNotInSelObj means that this branch will only be
                        // executed in the cursor points inside a selected
                        // object. However, if this is the case, the previous
                        // if( rSh.IsObjSelectable(rLPt) ) must always be true:
                        // rLPt is inside a selected object, then obviously
                        // rLPt is over a selectable object.
                        if (rSh.IsSelObjProtected(FLYPROTECT_SIZE))
                            eStyle = POINTER_NOTALLOWED;
                        else
                            eStyle = POINTER_MOVE;
                        aActHitType = SDRHIT_OBJECT;
                    }
                    else
                    {
                        if ( rView.GetDrawFuncPtr() )
                            bPrefSdrPointer = sal_True;
                        else
                            bCntAtPos = sal_True;
                    }
                }
            }
        }
    }
    if ( bPrefSdrPointer )
    {
        if (bIsDocReadOnly || (rSh.IsObjSelected() && rSh.IsSelObjProtected(FLYPROTECT_CONTENT)))
            SetPointer( POINTER_NOTALLOWED );
        else
        {
            if (rView.GetDrawFuncPtr() && rView.GetDrawFuncPtr()->IsInsertForm() && !bHitHandle)
                SetPointer( POINTER_DRAW_RECT );
            else
            {
                SetPointer( pSdrView->GetPreferedPointer( aB2DLPt, rSh.GetOut() ) );
            }
        }
    }
    else
    {
        if( !rSh.IsPageAtPos( rLPt ) || pAnchorMarker )
            eStyle = POINTER_ARROW;
        else
        {
            if( bCntAtPos )
            {
                SwContentAtPos aSwContentAtPos(
                    SwContentAtPos::SW_CLICKFIELD|
                    SwContentAtPos::SW_INETATTR|
                    SwContentAtPos::SW_FTN |
                    SwContentAtPos::SW_SMARTTAG );
                if( rSh.GetContentAtPos( rLPt, aSwContentAtPos) )
                {
                    const bool bClickToFollow = SwContentAtPos::SW_INETATTR == aSwContentAtPos.eCntntAtPos ||
                                                SwContentAtPos::SW_SMARTTAG == aSwContentAtPos.eCntntAtPos;

                     if( !bClickToFollow ||
                         (SwContentAtPos::SW_INETATTR == aSwContentAtPos.eCntntAtPos && bExecHyperlinks) ||
                         (SwContentAtPos::SW_SMARTTAG == aSwContentAtPos.eCntntAtPos && bExecSmarttags) )
                        eStyle = POINTER_REFHAND;
                }
            }
        }

        // which kind of text pointer have we to show - horz / vert - ?
        if( POINTER_TEXT == eStyle && rSh.IsInVerticalText( &rLPt ))
            eStyle = POINTER_TEXT_VERTICAL;

        SetPointer( eStyle );
    }
}

/*--------------------------------------------------------------------
    Beschreibung: Timer fuer Selektion vergroessern
 --------------------------------------------------------------------*/


IMPL_LINK( SwEditWin, TimerHandler, Timer *, EMPTYARG )
{
    DBG_PROFSTART(edithdl);

    SwWrtShell &rSh = rView.GetWrtShell();
    Point aModPt( aMovePos );
    const SwRect aOldVis( rSh.VisArea() );
    sal_Bool bDone = sal_False;

    if ( !rSh.VisArea().IsInside( aModPt ) )
    {
        if ( bInsDraw )
        {
            const int nMaxScroll = 40;
            rView.Scroll( Rectangle(aModPt,Size(1,1)), nMaxScroll, nMaxScroll);
            bDone = sal_True;
        }
        else if ( bFrmDrag )
        {
            (rSh.*rSh.fnDrag)(&aModPt,sal_False);
            bDone = sal_True;
        }
        if ( !bDone )
            aModPt = rSh.GetCntntPos( aModPt,aModPt.Y() > rSh.VisArea().Bottom() );
    }
    if ( !bDone && !(bFrmDrag || bInsDraw) )
    {
        if ( pRowColumnSelectionStart )
        {
            Point aPos( aModPt );
            rSh.SelectTableRowCol( *pRowColumnSelectionStart, &aPos, bIsRowDrag );
        }
        else
            (rSh.*rSh.fnSetCrsr)( &aModPt, sal_False );

        //fix(24138): Es kann sein, dass der "Sprung" ueber eine Tabelle so
        //nicht geschafft wird. Deshalb wir hier eben per Up/Down ueber die
        //Tabelle gesprungen.
        const SwRect& rVisArea = rSh.VisArea();
        if( aOldVis == rVisArea && !rSh.IsStartOfDoc() && !rSh.IsEndOfDoc() )
        {
            //JP 11.10.2001 Bug 72294 - take the center point of VisArea to
            //              decide in which direction the user want.
            if( aModPt.Y() < ( rVisArea.Top() + rVisArea.Height() / 2 ) )
                rSh.Up( sal_True, 1 );
            else
                rSh.Down( sal_True, 1 );
        }
    }

    aMovePos += rSh.VisArea().Pos() - aOldVis.Pos();
    JustifyAreaTimer();
    DBG_PROFSTOP(edithdl);
    return 0;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/


void SwEditWin::JustifyAreaTimer()
{
    const Rectangle &rVisArea = GetView().GetVisArea();
#ifdef UNX
    const long coMinLen = 100;
#else
    const long coMinLen = 50;
#endif
    long nTimeout = 800,
         nDiff = Max(
         Max( aMovePos.Y() - rVisArea.Bottom(), rVisArea.Top() - aMovePos.Y() ),
         Max( aMovePos.X() - rVisArea.Right(),  rVisArea.Left() - aMovePos.X()));
    aTimer.SetTimeout( Max( coMinLen, nTimeout - nDiff) );
}

void SwEditWin::LeaveArea(const Point &rPos)
{
    aMovePos = rPos;
    JustifyAreaTimer();
    if( !aTimer.IsActive() )
        aTimer.Start();
    if( pShadCrsr )
        delete pShadCrsr, pShadCrsr = 0;
}

inline void SwEditWin::EnterArea()
{
    aTimer.Stop();
}

/*------------------------------------------------------------------------
 Beschreibung:  Modus fuer Rahmen einfuegen
------------------------------------------------------------------------*/


void SwEditWin::InsFrm(sal_uInt16 nCols)
{
    StdDrawMode( OBJ_NONE, sal_False );
    bInsFrm = sal_True;
    nInsFrmColCount = nCols;
}



void SwEditWin::StdDrawMode( SdrObjKind eSdrObjectKind, sal_Bool bObjSelect )
{
    setSdrObjectCreationInfo(SdrObjectCreationInfo(eSdrObjectKind));

    if (bObjSelect)
        rView.SetDrawFuncPtr(new DrawSelection( &rView.GetWrtShell(), this, &rView ));
    else
        rView.SetDrawFuncPtr(new SwDrawBase( &rView.GetWrtShell(), this, &rView ));

    rView.SetSelDrawSlot();
    setSdrObjectCreationInfo(SdrObjectCreationInfo(eSdrObjectKind));
    if (bObjSelect)
        rView.GetDrawFuncPtr()->Activate( SID_OBJECT_SELECT );
    else
        rView.GetDrawFuncPtr()->Activate( sal::static_int_cast< sal_uInt16 >(eSdrObjectKind) );     // don't know if this is useful at all; but it keeps functionality as it was...
    bInsFrm = sal_False;
    nInsFrmColCount = 1;
}



void SwEditWin::StopInsFrm()
{
    if (rView.GetDrawFuncPtr())
    {
        rView.GetDrawFuncPtr()->Deactivate();
        rView.SetDrawFuncPtr(NULL);
    }
    rView.LeaveDrawCreate();    // Konstruktionsmode verlassen
    bInsFrm = sal_False;
    nInsFrmColCount = 1;
}

/*--------------------------------------------------------------------
 --------------------------------------------------------------------*/


sal_Bool SwEditWin::IsInputSequenceCheckingRequired( const String &rText, const SwPaM& rCrsr ) const
{
    const SvtCTLOptions& rCTLOptions = SW_MOD()->GetCTLOptions();
    if ( !rCTLOptions.IsCTLFontEnabled() ||
         !rCTLOptions.IsCTLSequenceChecking() )
         return sal_False;

    const xub_StrLen nFirstPos = rCrsr.Start()->nContent.GetIndex();
    if ( 0 == nFirstPos ) /* first char needs not to be checked */
        return sal_False;

    SwBreakIt *pBreakIter = SwBreakIt::Get();
    uno::Reference < i18n::XBreakIterator > xBI = pBreakIter->GetBreakIter();
    long nCTLScriptPos = -1;

    if (xBI.is())
    {
        if (xBI->getScriptType( rText, 0 ) == i18n::ScriptType::COMPLEX)
            nCTLScriptPos = 0;
        else
            nCTLScriptPos = xBI->nextScript( rText, 0, i18n::ScriptType::COMPLEX );
    }

    return (0 <= nCTLScriptPos && nCTLScriptPos <= rText.Len());
}


/*--------------------------------------------------------------------
     Beschreibung:  Der Character Buffer wird in das Dokument eingefuegt
 --------------------------------------------------------------------*/


void SwEditWin::FlushInBuffer()
{
    if ( aInBuffer.Len() )
    {
        SwWrtShell& rSh = rView.GetWrtShell();

        // generate new sequence input checker if not already done
        if ( !pCheckIt )
            pCheckIt = new SwCheckIt;

        uno::Reference < i18n::XExtendedInputSequenceChecker > xISC = pCheckIt->xCheck;
        if ( xISC.is() && IsInputSequenceCheckingRequired( aInBuffer, *rSh.GetCrsr() ) )
        {
            //
            // apply (Thai) input sequence checking/correction
            //

            rSh.Push(); // push current cursor to stack

            // get text from the beginning (i.e left side) of current selection
            // to the start of the paragraph
            rSh.NormalizePam();     // make point be the first (left) one
            if (!rSh.GetCrsr()->HasMark())
                rSh.GetCrsr()->SetMark();
            rSh.GetCrsr()->GetMark()->nContent = 0;
            String aLeftText( rSh.GetCrsr()->GetTxt() );

            SvtCTLOptions& rCTLOptions = SW_MOD()->GetCTLOptions();

            xub_StrLen nExpandSelection = 0;
            if (aLeftText.Len() > 0)
            {
                sal_Unicode cChar = '\0';

                xub_StrLen nTmpPos = aLeftText.Len();
                sal_Int16 nCheckMode = rCTLOptions.IsCTLSequenceCheckingRestricted() ?
                        i18n::InputSequenceCheckMode::STRICT : i18n::InputSequenceCheckMode::BASIC;

                rtl::OUString aOldText( aLeftText );
                rtl::OUString aNewText( aOldText );
                if (rCTLOptions.IsCTLSequenceCheckingTypeAndReplace())
                {
                    for (xub_StrLen k = 0;  k < aInBuffer.Len();  ++k)
                    {
                        cChar = aInBuffer.GetChar(k);
                        const xub_StrLen nPrevPos = static_cast<xub_StrLen>(xISC->correctInputSequence( aNewText, nTmpPos - 1, cChar, nCheckMode ));

                        // valid sequence or sequence could be corrected:
                        if (nPrevPos != aNewText.getLength())
                            nTmpPos = nPrevPos + 1;
                    }

                    // find position of first character that has changed
                    sal_Int32 nOldLen = aOldText.getLength();
                    sal_Int32 nNewLen = aNewText.getLength();
                    const sal_Unicode *pOldTxt = aOldText.getStr();
                    const sal_Unicode *pNewTxt = aNewText.getStr();
                    sal_Int32 nChgPos = 0;
                    while ( nChgPos < nOldLen && nChgPos < nNewLen &&
                            pOldTxt[nChgPos] == pNewTxt[nChgPos] )
                        ++nChgPos;

                    xub_StrLen nChgLen = static_cast< xub_StrLen >(nNewLen - nChgPos);
                    String aChgText( aNewText.copy( static_cast< xub_StrLen >(nChgPos), nChgLen ) );

                    if (aChgText.Len())
                    {
                        aInBuffer = aChgText;
                        nExpandSelection = aLeftText.Len() - static_cast< xub_StrLen >(nChgPos);
                    }
                    else
                        aInBuffer.Erase();
                }
                else
                {
                    for (xub_StrLen k = 0;  k < aInBuffer.Len();  ++k)
                    {
                        cChar = aInBuffer.GetChar(k);
                        if (xISC->checkInputSequence( aNewText, nTmpPos - 1, cChar, nCheckMode ))
                        {
                            // character can be inserted:
                            aNewText += rtl::OUString( (sal_Unicode) cChar );
                            ++nTmpPos;
                        }
                    }
                    aInBuffer = aNewText.copy( aOldText.getLength() );  // copy new text to be inserted to buffer
                }
            }

            // at this point now we will insert the buffer text 'normally' some lines below...

            rSh.Pop( sal_False );  // pop old cursor from stack

            if (!aInBuffer.Len())
                return;

            // if text prior to the original selection needs to be changed
            // as well, we now expand the selection accordingly.
            SwPaM &rCrsr = *rSh.GetCrsr();
            xub_StrLen nCrsrStartPos = rCrsr.Start()->nContent.GetIndex();
            DBG_ASSERT( nCrsrStartPos >= nExpandSelection, "cannot expand selection as specified!!" );
            if (nExpandSelection && nCrsrStartPos >= nExpandSelection)
            {
                if (!rCrsr.HasMark())
                    rCrsr.SetMark();
                rCrsr.Start()->nContent -= nExpandSelection;
            }
        }

        uno::Reference< frame::XDispatchRecorder > xRecorder =
                rView.GetViewFrame()->GetBindings().GetRecorder();
        if ( xRecorder.is() )
        {
            //Shell ermitteln
            SfxShell *pSfxShell = lcl_GetShellFromDispatcher( rView, typeid(SwTextShell) );
            // Request generieren und recorden
            if (pSfxShell)
            {
                SfxRequest aReq( rView.GetViewFrame(), FN_INSERT_STRING );
                aReq.AppendItem( SfxStringItem( FN_INSERT_STRING, aInBuffer ) );
                aReq.Done();
            }
        }
        //#21019# apply CTL and CJK language to the text input
        sal_Bool bLang = true;
        if(eBufferLanguage != LANGUAGE_DONTKNOW)
        {
            sal_uInt16 nWhich = 0;
            switch( GetI18NScriptTypeOfLanguage( eBufferLanguage ))
            {
                case  i18n::ScriptType::ASIAN:     nWhich = RES_CHRATR_CJK_LANGUAGE; break;
                case  i18n::ScriptType::COMPLEX:   nWhich = RES_CHRATR_CTL_LANGUAGE; break;
                case  i18n::ScriptType::LATIN:     nWhich = RES_CHRATR_LANGUAGE; break;
                default: bLang = sal_False;
            }
            if(bLang)
            {
                SfxItemSet aLangSet(rView.GetPool(), nWhich, nWhich);
                rSh.GetCurAttr(aLangSet);
                if(SFX_ITEM_DEFAULT <= aLangSet.GetItemState(nWhich, sal_True))
                {
                    LanguageType eLang = static_cast<const SvxLanguageItem&>(aLangSet.Get(nWhich)).GetLanguage();
                    if ( eLang == eBufferLanguage )
                        // current language attribute equal to language reported from system
                        bLang = sal_False;
                    else if ( !bInputLanguageSwitched && RES_CHRATR_LANGUAGE == nWhich /* && (eLang&LANGUAGE_MASK_PRIMARY) == LANGUAGE_ENGLISH */ )
                    {
                        // special case: switching between two "LATIN" languages
                        // In case the current keyboard setting might be suitable for both languages we can't safely assume that the user
                        // wants to use the language reported from the system, except if we knew that it was explicitly switched (thus the check for "bInputLangeSwitched").
                        // The language reported by the system could be just the system default language that the user is not even aware of,
                        // because no language selection tool is installed at all. In this case the OOo language should get preference as
                        // it might have been selected by the user explicitly.
                        // Usually this case happens if the OOo language is different to the system language but the system keyboard is still suitable
                        // for the OOo language (e.g. writing English texts with a German keyboard).
                        // For non-latin keyboards overwriting the attribute is still valid. We do this for kyrillic and greek ATM.
                        // In future versions of OOo this should be replaced by a configuration switch that allows to give the preference to
                        // the OOo setting or the system setting explicitly and/or a better handling of the script type.
                        sal_Int16 nScript = GetAppCharClass().getScript( aInBuffer, 0 );
                        i18n::UnicodeScript eType = (i18n::UnicodeScript) nScript;

                        bool bSystemIsNonLatin = false, bOOoLangIsNonLatin = false;
                        switch ( eType )
                        {
                            case i18n::UnicodeScript_kGreek:
                            case i18n::UnicodeScript_kCyrillic:
                                // in case other UnicodeScripts require special keyboards they can be added here
                                bSystemIsNonLatin = true;
                                break;
                            default:
                                break;
                        }

                        switch ( eLang )
                        {
                            case LANGUAGE_AZERI_CYRILLIC:
                            case LANGUAGE_BOSNIAN_CYRILLIC_BOSNIA_HERZEGOVINA:
                            case LANGUAGE_BULGARIAN:
                            case LANGUAGE_GREEK:
                            case LANGUAGE_RUSSIAN:
                            case LANGUAGE_RUSSIAN_MOLDOVA:
                            case LANGUAGE_SERBIAN_CYRILLIC:
                            case LANGUAGE_SERBIAN_CYRILLIC_BOSNIA_HERZEGOVINA:
                            case LANGUAGE_UZBEK_CYRILLIC:
                            case LANGUAGE_UKRAINIAN:
                            case LANGUAGE_BELARUSIAN:
                                bOOoLangIsNonLatin = true;
                                break;
                            default:
                                break;
                        }

                        bLang = (bSystemIsNonLatin != bOOoLangIsNonLatin);
                    }
                }
                if(bLang)
                {
                    SvxLanguageItem aLangItem( eBufferLanguage, nWhich );
                    rSh.SetAttrItem( aLangItem );
                }
            }
        }

        rSh.Insert( aInBuffer );
        eBufferLanguage = LANGUAGE_DONTKNOW;
        aInBuffer.Erase();
        bFlushCharBuffer = sal_False;
    }
}

#define MOVE_LEFT_SMALL     0
#define MOVE_UP_SMALL       1
#define MOVE_RIGHT_BIG      2
#define MOVE_DOWN_BIG       3
#define MOVE_LEFT_BIG       4
#define MOVE_UP_BIG         5
#define MOVE_RIGHT_SMALL    6
#define MOVE_DOWN_SMALL     7

// #121236# Support for shift key in writer
#define MOVE_LEFT_HUGE      8
#define MOVE_UP_HUGE        9
#define MOVE_RIGHT_HUGE     10
#define MOVE_DOWN_HUGE      11

void SwEditWin::ChangeFly( sal_uInt8 nDir, sal_Bool bWeb )
{
    SwWrtShell &rSh = rView.GetWrtShell();
    SwRect aTmp = rSh.GetFlyRect();
    if( aTmp.HasArea() &&
        !rSh.IsSelObjProtected( FLYPROTECT_POS ) )
    {
        SfxItemSet aSet(rSh.GetAttrPool(),
                        RES_FRM_SIZE, RES_FRM_SIZE,
                        RES_VERT_ORIENT, RES_ANCHOR,
                        RES_COL, RES_COL,
                        RES_PROTECT, RES_PROTECT,
                        RES_FOLLOW_TEXT_FLOW, RES_FOLLOW_TEXT_FLOW, 0);
        rSh.GetFlyFrmAttr( aSet );
        RndStdIds eAnchorId = ((SwFmtAnchor&)aSet.Get(RES_ANCHOR)).GetAnchorId();
        Size aSnap;
        bool bHuge(MOVE_LEFT_HUGE == nDir ||
            MOVE_UP_HUGE == nDir ||
            MOVE_RIGHT_HUGE == nDir ||
            MOVE_DOWN_HUGE == nDir);

        if(MOVE_LEFT_SMALL == nDir ||
            MOVE_UP_SMALL == nDir ||
            MOVE_RIGHT_SMALL == nDir ||
            MOVE_DOWN_SMALL == nDir )
        {
            aSnap = PixelToLogic(Size(1,1));
        }
        else
        {
            aSnap = rSh.GetViewOptions()->GetSnapSize();
            short nDiv = rSh.GetViewOptions()->GetDivisionX();
            if ( nDiv > 0 )
                aSnap.Width() = Max( (sal_uLong)1, (sal_uLong)aSnap.Width() / nDiv );
            nDiv = rSh.GetViewOptions()->GetDivisionY();
            if ( nDiv > 0 )
                aSnap.Height() = Max( (sal_uLong)1, (sal_uLong)aSnap.Height() / nDiv );
        }

        if(bHuge)
        {
            // #121236# 567twips == 1cm, but just take three times the normal snap
            aSnap = Size(aSnap.Width() * 3, aSnap.Height() * 3);
        }

        SwRect aBoundRect;
        Point aRefPoint;
        {
            SwFmtVertOrient aVert( (SwFmtVertOrient&)aSet.Get(RES_VERT_ORIENT) );
            const bool bFollowTextFlow =
                    static_cast<const SwFmtFollowTextFlow&>(aSet.Get(RES_FOLLOW_TEXT_FLOW)).GetValue();
            const SwPosition* pToCharCntntPos = ((SwFmtAnchor&)aSet.Get(RES_ANCHOR)).GetCntntAnchor();
            rSh.CalcBoundRect( aBoundRect, eAnchorId,
                               text::RelOrientation::FRAME, aVert.GetRelationOrient(),
                               pToCharCntntPos, bFollowTextFlow,
                               false, &aRefPoint );
        }
        long nLeft = Min( aTmp.Left() - aBoundRect.Left(), aSnap.Width() );
        long nRight = Min( aBoundRect.Right() - aTmp.Right(), aSnap.Width() );
        long nUp = Min( aTmp.Top() - aBoundRect.Top(), aSnap.Height() );
        long nDown = Min( aBoundRect.Bottom() - aTmp.Bottom(), aSnap.Height() );

        switch ( nDir )
        {
            case MOVE_LEFT_BIG:
            case MOVE_LEFT_HUGE:
            case MOVE_LEFT_SMALL: aTmp.Left( aTmp.Left() - nLeft );
                break;

            case MOVE_UP_BIG:
            case MOVE_UP_HUGE:
            case MOVE_UP_SMALL: aTmp.Top( aTmp.Top() - nUp );
                break;

            case MOVE_RIGHT_SMALL:
                if( aTmp.Width() < aSnap.Width() + MINFLY )
                    break;
                nRight = aSnap.Width(); // kein break
            case MOVE_RIGHT_HUGE:
            case MOVE_RIGHT_BIG: aTmp.Left( aTmp.Left() + nRight );
                break;

            case MOVE_DOWN_SMALL:
                if( aTmp.Height() < aSnap.Height() + MINFLY )
                    break;
                nDown = aSnap.Height(); // kein break
            case MOVE_DOWN_HUGE:
            case MOVE_DOWN_BIG: aTmp.Top( aTmp.Top() + nDown );
                break;

            default: ASSERT( sal_True, "ChangeFly: Unknown direction." );
        }
        sal_Bool bSet = sal_False;
        if ((FLY_AS_CHAR == eAnchorId) && ( nDir % 2 ))
        {
            long aDiff = aTmp.Top() - aRefPoint.Y();
            if( aDiff > 0 )
                aDiff = 0;
            else if ( aDiff < -aTmp.Height() )
                aDiff = -aTmp.Height();
            SwFmtVertOrient aVert( (SwFmtVertOrient&)aSet.Get(RES_VERT_ORIENT) );
            sal_Int16 eNew;
            if( bWeb )
            {
                eNew = aVert.GetVertOrient();
                sal_Bool bDown = 0 != ( nDir & 0x02 );
                switch( eNew )
                {
                    case text::VertOrientation::CHAR_TOP:
                        if( bDown ) eNew = text::VertOrientation::CENTER;
                    break;
                    case text::VertOrientation::CENTER:
                        eNew = bDown ? text::VertOrientation::TOP : text::VertOrientation::CHAR_TOP;
                    break;
                    case text::VertOrientation::TOP:
                        if( !bDown ) eNew = text::VertOrientation::CENTER;
                    break;
                    case text::VertOrientation::LINE_TOP:
                        if( bDown ) eNew = text::VertOrientation::LINE_CENTER;
                    break;
                    case text::VertOrientation::LINE_CENTER:
                        eNew = bDown ? text::VertOrientation::LINE_BOTTOM : text::VertOrientation::LINE_TOP;
                    break;
                    case text::VertOrientation::LINE_BOTTOM:
                        if( !bDown ) eNew = text::VertOrientation::LINE_CENTER;
                    break;
                    default:; //prevent warning
                }
            }
            else
            {
                aVert.SetPos( aDiff );
                eNew = text::VertOrientation::NONE;
            }
            aVert.SetVertOrient( eNew );
            aSet.Put( aVert );
            bSet = sal_True;
        }
        if (bWeb && (FLY_AT_PARA == eAnchorId)
            && ( nDir==MOVE_LEFT_SMALL || nDir==MOVE_RIGHT_BIG ))
        {
            SwFmtHoriOrient aHori( (SwFmtHoriOrient&)aSet.Get(RES_HORI_ORIENT) );
            sal_Int16 eNew;
            eNew = aHori.GetHoriOrient();
            switch( eNew )
            {
                case text::HoriOrientation::RIGHT:
                    if( nDir==MOVE_LEFT_SMALL )
                        eNew = text::HoriOrientation::LEFT;
                break;
                case text::HoriOrientation::LEFT:
                    if( nDir==MOVE_RIGHT_BIG )
                        eNew = text::HoriOrientation::RIGHT;
                break;
                default:; //prevent warning
            }
            if( eNew != aHori.GetHoriOrient() )
            {
                aHori.SetHoriOrient( eNew );
                aSet.Put( aHori );
                bSet = sal_True;
            }
        }
        rSh.StartAllAction();
        if( bSet )
            rSh.SetFlyFrmAttr( aSet );
        sal_Bool bSetPos = (FLY_AS_CHAR != eAnchorId);
        if(bSetPos && bWeb)
        {
            if (FLY_AT_PAGE != eAnchorId)
            {
                bSetPos = sal_False;
            }
            else
            {
                bSetPos = (::GetHtmlMode(rView.GetDocShell()) & HTMLMODE_SOME_ABS_POS) ?
                    sal_True : sal_False;
            }
        }
        if( bSetPos )
            rSh.SetFlyPos( aTmp.Pos() );
        rSh.EndAllAction();
    }
}
/* -----------------------------23.05.2002 11:35------------------------------

 ---------------------------------------------------------------------------*/
void SwEditWin::ChangeDrawing( sal_uInt8 nDir )
{
    SwWrtShell &rSh = rView.GetWrtShell();
    rSh.StartUndo();

    long nX = 0;
    long nY = 0;
    const sal_Bool bOnePixel(
        MOVE_LEFT_SMALL == nDir ||
        MOVE_UP_SMALL == nDir ||
        MOVE_RIGHT_SMALL == nDir ||
        MOVE_DOWN_SMALL == nDir);
    const sal_Bool bHuge(
        MOVE_LEFT_HUGE == nDir ||
        MOVE_UP_HUGE == nDir ||
        MOVE_RIGHT_HUGE == nDir ||
        MOVE_DOWN_HUGE == nDir);
    sal_uInt16 nAnchorDir = SW_MOVE_UP;
    switch(nDir)
    {
        case MOVE_LEFT_SMALL:
        case MOVE_LEFT_HUGE:
        case MOVE_LEFT_BIG:
            nX = -1;
            nAnchorDir = SW_MOVE_LEFT;
        break;
        case MOVE_UP_SMALL:
        case MOVE_UP_HUGE:
        case MOVE_UP_BIG:
            nY = -1;
        break;
        case MOVE_RIGHT_SMALL:
        case MOVE_RIGHT_HUGE:
        case MOVE_RIGHT_BIG:
            nX = +1;
            nAnchorDir = SW_MOVE_RIGHT;
        break;
        case MOVE_DOWN_SMALL:
        case MOVE_DOWN_HUGE:
        case MOVE_DOWN_BIG:
            nY = +1;
            nAnchorDir = SW_MOVE_DOWN;
        break;
    }

    if(0 != nX || 0 != nY)
    {
        sal_uInt8 nProtect = rSh.IsSelObjProtected( FLYPROTECT_POS|FLYPROTECT_SIZE );
        Size aSnap( rSh.GetViewOptions()->GetSnapSize() );
        short nDiv = rSh.GetViewOptions()->GetDivisionX();
        if ( nDiv > 0 )
            aSnap.Width() = Max( (sal_uLong)1, (sal_uLong)aSnap.Width() / nDiv );
        nDiv = rSh.GetViewOptions()->GetDivisionY();
        if ( nDiv > 0 )
            aSnap.Height() = Max( (sal_uLong)1, (sal_uLong)aSnap.Height() / nDiv );

        if(bOnePixel)
        {
            aSnap = PixelToLogic(Size(1,1));
        }
        else if(bHuge)
        {
            // #121236# 567twips == 1cm, but just take three times the normal snap
            aSnap = Size(aSnap.Width() * 3, aSnap.Height() * 3);
        }

        nX *= aSnap.Width();
        nY *= aSnap.Height();

        SdrView *pSdrView = rSh.GetDrawView();
        const SdrHdlList& rHdlList = pSdrView->GetHdlList();
        SdrHdl* pHdl = rHdlList.GetFocusHdl();
        rSh.StartAllAction();
        if(0L == pHdl)
        {
            // now move the selected draw objects
            // if the object's position is not protected
            if(0 == (nProtect&FLYPROTECT_POS))
            {
                sal_Bool bDummy1, bDummy2;
                const bool bVertAnchor = rSh.IsFrmVertical( sal_True, bDummy1, bDummy2 );
                const bool bHoriMove = !bVertAnchor == !( nDir % 2 );
                const bool bMoveAllowed =
                    !bHoriMove || (rSh.GetAnchorId() != FLY_AS_CHAR);
                if ( bMoveAllowed )
                {
                    pSdrView->MoveMarkedObj(basegfx::B2DVector(nX, nY));
                    rSh.SetModified();
                }
            }
        }
        else
        {
            // move handle with index nHandleIndex
            if(pHdl && (nX || nY))
            {
                if( HDL_ANCHOR == pHdl->GetKind() ||
                    HDL_ANCHOR_TR == pHdl->GetKind() )
                {
                    // anchor move cannot be allowed when position is protected
                    if(0 == (nProtect&FLYPROTECT_POS))
                        rSh.MoveAnchor( nAnchorDir );
                }
                //now resize if size is protected
                else if(0 == (nProtect&FLYPROTECT_SIZE))
                {
                    // move handle with index nHandleIndex
                    pSdrView->MoveHandleByVector(*pHdl, basegfx::B2DVector(nX, nY), 0, 0);
                    rSh.SetModified();
                }
            }
        }
        rSh.EndAllAction();
    }

    rSh.EndUndo();
}

/*--------------------------------------------------------------------
    Beschreibung:   KeyEvents
 --------------------------------------------------------------------*/



void SwEditWin::KeyInput(const KeyEvent &rKEvt)
{
    if( rKEvt.GetKeyCode().GetCode() == KEY_ESCAPE &&
        pApplyTempl && pApplyTempl->pFormatClipboard )
    {
        pApplyTempl->pFormatClipboard->Erase();
        SetApplyTemplate(SwApplyTemplate());
        rView.GetViewFrame()->GetBindings().Invalidate(SID_FORMATPAINTBRUSH);
    }

    SfxObjectShell *pObjSh = (SfxObjectShell*)rView.GetViewFrame()->GetObjectShell();
    if ( bLockInput || (pObjSh && pObjSh->GetProgress()) )
        // Wenn die Rechenleiste aktiv ist oder
        // auf dem Document ein Progress laeuft wird keine
        // Bestellungen angenommen.
        return;

    if( pShadCrsr )
        delete pShadCrsr, pShadCrsr = 0;
    aKeyInputFlushTimer.Stop();

    SwWrtShell &rSh = rView.GetWrtShell();
    sal_Bool bIsDocReadOnly = rView.GetDocShell()->IsReadOnly() &&
                          rSh.IsCrsrReadonly();

    //if the language changes the buffer must be flushed
    LanguageType eNewLanguage = GetInputLanguage();
    if(!bIsDocReadOnly && eBufferLanguage != eNewLanguage && aInBuffer.Len())
    {
        FlushInBuffer();
    }
    eBufferLanguage = eNewLanguage;

    QuickHelpData aTmpQHD;
    if( pQuickHlpData->bClear )
    {
        aTmpQHD.Move( *pQuickHlpData );
        pQuickHlpData->Stop( rSh );
    }

    // OS:auch die DrawView braucht noch ein readonly-Flag
    if ( !bIsDocReadOnly && rSh.GetDrawView() && rSh.GetDrawView()->KeyInput( rKEvt, this ) )
    {
        rSh.GetView().GetViewFrame()->GetBindings().InvalidateAll( sal_False );
        rSh.SetModified();
        return; // Event von der SdrView ausgewertet
    }

    if ( rView.GetDrawFuncPtr() && bInsFrm )
    {
        StopInsFrm();
        rSh.Edit();
    }

    sal_Bool bFlushBuffer = sal_False;
    sal_Bool bNormalChar = sal_False;
    sal_Bool bChkInsBlank = pQuickHlpData->bChkInsBlank;
    pQuickHlpData->bChkInsBlank = sal_False;

    KeyEvent aKeyEvent( rKEvt );
    // look for vertical mappings
    if( !bIsDocReadOnly && !rSh.IsSelFrmMode() && !rSh.IsObjSelected() )
    {
        //JP 21.2.2002: must changed from switch to if, because the Linux
        // compiler has problem with the code. Has to remove if the new general
        // handler exist.
        sal_uInt16 nKey = rKEvt.GetKeyCode().GetCode();

        if( KEY_UP == nKey || KEY_DOWN == nKey ||
            KEY_LEFT == nKey || KEY_RIGHT == nKey )
        {
            // In general, we want to map the direction keys if we are inside
            // some vertical formatted text.
            // 1. Exception: For a table cursor in a horizontal table, the
            //               directions should never be mapped.
            // 2. Exception: For a table cursor in a vertical table, the
            //               directions should always be mapped.
            const bool bVertText = rSh.IsInVerticalText();
            const bool bTblCrsr = rSh.GetTableCrsr();
            const bool bVertTable = rSh.IsTableVertical();
            if( ( bVertText && ( !bTblCrsr || bVertTable ) ) ||
                ( bTblCrsr && bVertTable ) )
            {
                // Attempt to integrate cursor travelling for mongolian layout does not work.
                // Thus, back to previous mapping of cursor keys to direction keys.
                if( KEY_UP == nKey ) nKey = KEY_LEFT;
                else if( KEY_DOWN == nKey ) nKey = KEY_RIGHT;
                else if( KEY_LEFT == nKey ) nKey = KEY_DOWN;
                else if( KEY_RIGHT == nKey ) nKey = KEY_UP;
            }

            if ( rSh.IsInRightToLeftText() )
            {
                if( KEY_LEFT == nKey ) nKey = KEY_RIGHT;
                else if( KEY_RIGHT == nKey ) nKey = KEY_LEFT;
            }

            aKeyEvent = KeyEvent( rKEvt.GetCharCode(),
                                  KeyCode( nKey, rKEvt.GetKeyCode().GetModifier() ),
                                  rKEvt.GetRepeat() );
        }
    }

    const KeyCode& rKeyCode = aKeyEvent.GetKeyCode();
    sal_Unicode aCh = aKeyEvent.GetCharCode();

    // enable switching to notes ankor with Ctrl - Alt - Page Up/Down
    // pressing this inside a note will switch to next/previous note
    if ((rKeyCode.IsMod1() && rKeyCode.IsMod2()) && ((rKeyCode.GetCode() == KEY_PAGEUP) || (rKeyCode.GetCode() == KEY_PAGEDOWN)))
    {
        const bool bNext = rKeyCode.GetCode()==KEY_PAGEDOWN ? true : false;
        const SwFieldType* pFldType = rSh.GetFldType( 0, RES_POSTITFLD );
        rSh.MoveFldType( pFldType, bNext );
        return;
    }

    const SwFrmFmt* pFlyFmt = rSh.GetFlyFrmFmt();
    if( pFlyFmt )
    {
        sal_uInt16 nEvent;

        if( 32 <= aCh &&
            0 == (( KEY_MOD1 | KEY_MOD2 ) & rKeyCode.GetModifier() ))
            nEvent = SW_EVENT_FRM_KEYINPUT_ALPHA;
        else
            nEvent = SW_EVENT_FRM_KEYINPUT_NOALPHA;

        const SvxMacro* pMacro = pFlyFmt->GetMacro().GetMacroTable().Get( nEvent );
        if( pMacro )
        {
            String sRet;
            SbxArrayRef xArgs = new SbxArray;
            SbxVariableRef xVar = new SbxVariable;
            xVar->PutString( pFlyFmt->GetName() );
            xArgs->Put( &xVar, 1 );

            xVar = new SbxVariable;
            if( SW_EVENT_FRM_KEYINPUT_ALPHA == nEvent )
                xVar->PutChar( aCh );
            else
                xVar->PutUShort( rKeyCode.GetModifier() | rKeyCode.GetCode() );
            xArgs->Put( &xVar, 2 );

            rSh.ExecMacro( *pMacro, &sRet, &xArgs );
            if( sRet.Len() && 0 != sRet.ToInt32() )
                return ;
        }
    }
    int nLclSelectionType;
    //A is converted to 1
    if( rKeyCode.GetFullCode() == (KEY_A | KEY_MOD1 |KEY_SHIFT)
        && rSh.HasDrawView() &&
        (0 != (nLclSelectionType = rSh.GetSelectionType()) &&
        ((nLclSelectionType & (nsSelectionType::SEL_FRM|nsSelectionType::SEL_GRF)) ||
        ((nLclSelectionType & (nsSelectionType::SEL_DRW|nsSelectionType::SEL_DRW_FORM)) &&
                0 != rSh.GetDrawView()->getSelectedIfSingle()))))
    {
        SdrHdlList& rHdlList = (SdrHdlList&)rSh.GetDrawView()->GetHdlList();
        SdrHdl* pAnchor = rHdlList.GetHdlByKind(HDL_ANCHOR);
        if ( ! pAnchor )
            pAnchor = rHdlList.GetHdlByKind(HDL_ANCHOR_TR);
        if(pAnchor)
            rHdlList.SetFocusHdl(pAnchor);
        return;
    }

    SvxAutoCorrCfg* pACfg = 0;
    SvxAutoCorrect* pACorr = 0;

    uno::Reference< frame::XDispatchRecorder > xRecorder =
            rView.GetViewFrame()->GetBindings().GetRecorder();
    if ( !xRecorder.is() )
    {
        pACfg = SvxAutoCorrCfg::Get();
        pACorr = pACfg->GetAutoCorrect();
    }

    SwModuleOptions* pModOpt = SW_MOD()->GetModuleConfig();

    TblChgWidthHeightType eTblChgMode = nsTblChgWidthHeightType::WH_COL_LEFT;    // initialization just for warning-free code
    sal_uInt16 nTblChgSize = 0;
    sal_Bool bStopKeyInputTimer = sal_True;
    String sFmlEntry;

    enum SW_KeyState { KS_Start,
                       KS_CheckKey, KS_InsChar, KS_InsTab,
                       KS_NoNum, KS_NumOff, KS_NumOrNoNum, KS_NumDown, KS_NumUp,
                       // -> #i23725#
                       KS_NumIndentInc, KS_NumIndentDec,
                       // <- #i23725#

                       KS_OutlineLvOff,
                       KS_NextCell, KS_PrevCell, KS_OutlineUp, KS_OutlineDown,
                       KS_GlossaryExpand, KS_NextPrevGlossary,
                       KS_AutoFmtByInput,
                       KS_NextObject, KS_PrevObject,
                       KS_KeyToView,
                       KS_LaunchOLEObject, KS_GoIntoFly, KS_GoIntoDrawing,
                       KS_EnterDrawHandleMode,
                       KS_CheckDocReadOnlyKeys,
                       KS_CheckAutoCorrect, KS_EditFormula,
                       KS_ColLeftBig, KS_ColRightBig,
                       KS_ColLeftSmall, KS_ColRightSmall,
                       KS_ColTopBig, KS_ColBottomBig,
                       KS_ColTopSmall, KS_ColBottomSmall,
                       KS_CellLeftBig, KS_CellRightBig,
                       KS_CellLeftSmall, KS_CellRightSmall,
                       KS_CellTopBig, KS_CellBottomBig,
                       KS_CellTopSmall, KS_CellBottomSmall,

                       KS_InsDel_ColLeftBig, KS_InsDel_ColRightBig,
                       KS_InsDel_ColLeftSmall, KS_InsDel_ColRightSmall,
                       KS_InsDel_ColTopBig, KS_InsDel_ColBottomBig,
                       KS_InsDel_ColTopSmall, KS_InsDel_ColBottomSmall,
                       KS_InsDel_CellLeftBig, KS_InsDel_CellRightBig,
                       KS_InsDel_CellLeftSmall, KS_InsDel_CellRightSmall,
                       KS_InsDel_CellTopBig, KS_InsDel_CellBottomBig,
                       KS_InsDel_CellTopSmall, KS_InsDel_CellBottomSmall,
                       KS_TblColCellInsDel,

                       KS_Fly_Change, KS_Draw_Change,
                       KS_SpecialInsert,
                       KS_EnterCharCell,
                       KS_GotoNextFieldMark,
                       KS_GotoPrevFieldMark,
                       KS_Ende };



    SW_KeyState eKeyState = bIsDocReadOnly ? KS_CheckDocReadOnlyKeys : KS_CheckKey;
    SW_KeyState eNextKeyState = KS_Ende;
    sal_uInt8 nDir = 0;

    if (nKS_NUMDOWN_Count > 0)
        nKS_NUMDOWN_Count--;

    if (nKS_NUMINDENTINC_Count > 0)
        nKS_NUMINDENTINC_Count--;

    while( KS_Ende != eKeyState )
    {
        SW_KeyState eFlyState = KS_KeyToView;

        switch( eKeyState )
        {
        case KS_CheckKey:
            eKeyState = KS_KeyToView;       // default weiter zur View

#ifdef DBG_UTIL
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
            // JP 19.01.99: zum Umschalten des Cursor Verhaltens in ReadOnly
            //              Bereichen
            if( 0x7210 == rKeyCode.GetFullCode() )
                rSh.SetReadOnlyAvailable( !rSh.IsReadOnlyAvailable() );
            else
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
#endif

            if( !rKeyCode.IsMod2() && '=' == aCh &&
                !rSh.IsTableMode() && rSh.GetTableFmt() &&
                rSh.IsSttPara() /*&& rSh.IsEndPara()*/ &&
                !rSh.HasReadonlySel() )
            {
                // in der Tabelle am Anfang der Zelle ein '=' ->
                //  EditZeile aufrufen (F2-Funktionalitaet)
                rSh.Push();
                if( !rSh.MoveSection( fnSectionCurr, fnSectionStart) &&
                    !rSh.IsTableBoxTextFormat() )
                {
                    // steht also am Anfang der Box
                    eKeyState = KS_EditFormula;
                    if( rSh.HasMark() )
                        rSh.SwapPam();
                    else
                        rSh.SttSelect();
                    rSh.MoveSection( fnSectionCurr, fnSectionEnd );
                    rSh.Pop( sal_True );
                    rSh.EndSelect();
                    sFmlEntry = '=';
                }
                else
                    rSh.Pop( sal_False );
            }
            else
            {
                if( pACorr && aTmpQHD.HasCntnt() && !rSh.HasSelection() &&
                    !rSh.HasReadonlySel() && !aTmpQHD.bIsAutoText &&
                    pACorr->GetSwFlags().nAutoCmpltExpandKey ==
                    (rKeyCode.GetModifier() | rKeyCode.GetCode()) )
                {
                    eKeyState = KS_GlossaryExpand;
                    break;
                }

                switch( rKeyCode.GetModifier() | rKeyCode.GetCode() )
                {
                case KEY_RIGHT | KEY_MOD2:
                    eKeyState = KS_ColRightBig;
                    eFlyState = KS_Fly_Change;
                    nDir = MOVE_RIGHT_SMALL;
                    goto KEYINPUT_CHECKTABLE;

                case KEY_LEFT | KEY_MOD2:
                    eKeyState = KS_ColRightSmall;
                    eFlyState = KS_Fly_Change;
                    nDir = MOVE_LEFT_SMALL;
                    goto KEYINPUT_CHECKTABLE;

                case KEY_RIGHT | KEY_MOD2 | KEY_SHIFT:
                    eKeyState = KS_ColLeftSmall;
                    goto KEYINPUT_CHECKTABLE;

                case KEY_LEFT | KEY_MOD2 | KEY_SHIFT:
                    eKeyState = KS_ColLeftBig;
                    goto KEYINPUT_CHECKTABLE;

                case KEY_RIGHT | KEY_MOD2 | KEY_MOD1:
                    eKeyState = KS_CellRightBig;
                    goto KEYINPUT_CHECKTABLE;

                case KEY_LEFT | KEY_MOD2 | KEY_MOD1:
                    eKeyState = KS_CellRightSmall;
                    goto KEYINPUT_CHECKTABLE;

                case KEY_RIGHT | KEY_MOD2 | KEY_SHIFT | KEY_MOD1:
                    eKeyState = KS_CellLeftSmall;
                    goto KEYINPUT_CHECKTABLE;

                case KEY_LEFT | KEY_MOD2 | KEY_SHIFT | KEY_MOD1:
                    eKeyState = KS_CellLeftBig;
                    goto KEYINPUT_CHECKTABLE;

                case KEY_UP | KEY_MOD2:
                    eKeyState = KS_ColBottomSmall;
                    eFlyState = KS_Fly_Change;
                    nDir = MOVE_UP_SMALL;
                    goto KEYINPUT_CHECKTABLE;

                case KEY_DOWN | KEY_MOD2:
                    eKeyState = KS_ColBottomBig;
                    eFlyState = KS_Fly_Change;
                    nDir = MOVE_DOWN_SMALL;
                    goto KEYINPUT_CHECKTABLE;

                case KEY_UP | KEY_MOD2 | KEY_MOD1:
                    eKeyState = KS_CellBottomSmall;
                    goto KEYINPUT_CHECKTABLE;

                case KEY_DOWN | KEY_MOD2 | KEY_MOD1:
                    eKeyState = KS_CellBottomBig;
                    goto KEYINPUT_CHECKTABLE;

                case KEY_UP | KEY_MOD2 | KEY_SHIFT | KEY_MOD1:
                    eKeyState = KS_CellTopBig;
                    goto KEYINPUT_CHECKTABLE;

                case KEY_DOWN | KEY_MOD2 | KEY_SHIFT | KEY_MOD1:
                    eKeyState = KS_CellTopSmall;
                    goto KEYINPUT_CHECKTABLE;

KEYINPUT_CHECKTABLE:
                    if( rSh.IsTableMode() || !rSh.GetTableFmt() )
                    {
                        if(KS_KeyToView != eFlyState)
                        {
                            if(!pFlyFmt && KS_KeyToView != eFlyState &&
                                (rSh.GetSelectionType() & (nsSelectionType::SEL_DRW|nsSelectionType::SEL_DRW_FORM))  &&
                                    rSh.GetDrawView()->areSdrObjectsSelected())
                                eKeyState = KS_Draw_Change;
                        }

                        if( pFlyFmt )
                            eKeyState = eFlyState;
                        else if( KS_Draw_Change != eKeyState)
                            eKeyState = KS_EnterCharCell;
                    }
                    break;

                // huge object move
                case KEY_RIGHT | KEY_SHIFT:
                case KEY_LEFT | KEY_SHIFT:
                case KEY_UP | KEY_SHIFT:
                case KEY_DOWN | KEY_SHIFT:
                {
                    const int nSelectionType = rSh.GetSelectionType();
                    if ( ( pFlyFmt
                           && ( nSelectionType & (nsSelectionType::SEL_FRM|nsSelectionType::SEL_OLE|nsSelectionType::SEL_GRF) ) )
                         || ( ( nSelectionType & (nsSelectionType::SEL_DRW|nsSelectionType::SEL_DRW_FORM) )
                              && rSh.GetDrawView()->areSdrObjectsSelected() ) )
                    {
                        eKeyState = pFlyFmt ? KS_Fly_Change : KS_Draw_Change;
                        switch ( rKeyCode.GetCode() )
                        {
                            case KEY_RIGHT: nDir = MOVE_RIGHT_HUGE; break;
                            case KEY_LEFT: nDir = MOVE_LEFT_HUGE; break;
                            case KEY_UP: nDir = MOVE_UP_HUGE; break;
                            case KEY_DOWN: nDir = MOVE_DOWN_HUGE; break;
                        }
                    }
                    break;
                }

//-------
// Insert/Delete
                case KEY_LEFT:
                case KEY_LEFT | KEY_MOD1:
                {
                    sal_Bool bMod1 = 0 != (rKeyCode.GetModifier() & KEY_MOD1);
                    if(!bMod1)
                    {
                        eFlyState = KS_Fly_Change;
                        nDir = MOVE_LEFT_BIG;
                    }
                    eTblChgMode = nsTblChgWidthHeightType::WH_FLAG_INSDEL |
                            ( bMod1
                                ? nsTblChgWidthHeightType::WH_CELL_LEFT
                                : nsTblChgWidthHeightType::WH_COL_LEFT );
                    nTblChgSize = pModOpt->GetTblVInsert();
                }
                    goto KEYINPUT_CHECKTABLE_INSDEL;
                case KEY_RIGHT | KEY_MOD1:
                {
                    eTblChgMode = nsTblChgWidthHeightType::WH_FLAG_INSDEL | nsTblChgWidthHeightType::WH_CELL_RIGHT;
                    nTblChgSize = pModOpt->GetTblVInsert();
                }
                    goto KEYINPUT_CHECKTABLE_INSDEL;
                case KEY_UP:
                case KEY_UP | KEY_MOD1:
                {
                    sal_Bool bMod1 = 0 != (rKeyCode.GetModifier() & KEY_MOD1);
                    if(!bMod1)
                    {
                        eFlyState = KS_Fly_Change;
                        nDir = MOVE_UP_BIG;
                    }
                    eTblChgMode = nsTblChgWidthHeightType::WH_FLAG_INSDEL |
                            ( bMod1
                                ? nsTblChgWidthHeightType::WH_CELL_TOP
                                : nsTblChgWidthHeightType::WH_ROW_TOP );
                    nTblChgSize = pModOpt->GetTblHInsert();
                }
                    goto KEYINPUT_CHECKTABLE_INSDEL;
                case KEY_DOWN:
                case KEY_DOWN | KEY_MOD1:
                {
                    sal_Bool bMod1 = 0 != (rKeyCode.GetModifier() & KEY_MOD1);
                    if(!bMod1)
                    {
                        eFlyState = KS_Fly_Change;
                        nDir = MOVE_DOWN_BIG;
                    }
                    eTblChgMode = nsTblChgWidthHeightType::WH_FLAG_INSDEL |
                            ( bMod1
                                ? nsTblChgWidthHeightType::WH_CELL_BOTTOM
                                : nsTblChgWidthHeightType::WH_ROW_BOTTOM );
                    nTblChgSize = pModOpt->GetTblHInsert();
                }
                    goto KEYINPUT_CHECKTABLE_INSDEL;

KEYINPUT_CHECKTABLE_INSDEL:
                    if( rSh.IsTableMode() || !rSh.GetTableFmt() ||
                        !bTblInsDelMode ||
                        sal_False /* Tabelle geschuetzt */
                            )
                    {
                        const int nSelectionType = rSh.GetSelectionType();

                        eKeyState = KS_KeyToView;
                        if(KS_KeyToView != eFlyState)
                        {
                            if((nSelectionType & (nsSelectionType::SEL_DRW|nsSelectionType::SEL_DRW_FORM))  &&
                                    rSh.GetDrawView()->areSdrObjectsSelected())
                                eKeyState = KS_Draw_Change;
                            else if(nSelectionType & (nsSelectionType::SEL_FRM|nsSelectionType::SEL_OLE|nsSelectionType::SEL_GRF))
                                eKeyState = KS_Fly_Change;
                        }
                    }
                    else
                    {
                        if( !bTblIsInsMode )
                            eTblChgMode = eTblChgMode | nsTblChgWidthHeightType::WH_FLAG_BIGGER;
                        eKeyState = KS_TblColCellInsDel;
                    }
                    break;

                case KEY_DELETE:
                    if (rSh.IsInFrontOfLabel() &&
                        rSh.NumOrNoNum(sal_False))
                        eKeyState = KS_NumOrNoNum;
                    break;

                case KEY_DELETE | KEY_MOD2:
                    if( !rSh.IsTableMode() && rSh.GetTableFmt() )
                    {
                        eKeyState = KS_Ende;
                        bTblInsDelMode = sal_True;
                        bTblIsInsMode = sal_False;
                        bTblIsColMode = sal_True;
                        aKeyInputTimer.Start();
                        bStopKeyInputTimer = sal_False;
                    }
                    break;
                case KEY_INSERT | KEY_MOD2:
                    if( !rSh.IsTableMode() && rSh.GetTableFmt() )
                    {
                        eKeyState = KS_Ende;
                        bTblInsDelMode = sal_True;
                        bTblIsInsMode = sal_True;
                        bTblIsColMode = sal_True;
                        aKeyInputTimer.Start();
                        bStopKeyInputTimer = sal_False;
                    }
                    break;

                case KEY_RETURN:                // Return
                    if ( !rSh.HasReadonlySel()
                         && !rSh.CrsrInsideInputFld() )
                    {
                        const int nSelectionType = rSh.GetSelectionType();
                        if(nSelectionType & nsSelectionType::SEL_OLE)
                            eKeyState = KS_LaunchOLEObject;
                        else if(nSelectionType & nsSelectionType::SEL_FRM)
                            eKeyState = KS_GoIntoFly;
                        else if((nSelectionType & nsSelectionType::SEL_DRW) &&
                                0 == (nSelectionType & nsSelectionType::SEL_DRW_TXT) &&
                                0 != rSh.GetDrawView()->getSelectedIfSingle())
                            eKeyState = KS_GoIntoDrawing;
                        else if( aTmpQHD.HasCntnt() && !rSh.HasSelection() &&
                            aTmpQHD.bIsAutoText )
                            eKeyState = KS_GlossaryExpand;

                        //RETURN und leerer Absatz in Numerierung -> Num. beenden
                        else if( !aInBuffer.Len() &&
                                 rSh.GetCurNumRule() &&
                                 !rSh.GetCurNumRule()->IsOutlineRule() &&
                                 !rSh.HasSelection() &&
                                rSh.IsSttPara() && rSh.IsEndPara() )
                            eKeyState = KS_NumOff, eNextKeyState = KS_OutlineLvOff;

                        //RETURN fuer neuen Absatz mit AutoFormatierung
                        else if( pACfg && pACfg->IsAutoFmtByInput() &&
                                !(nSelectionType & (nsSelectionType::SEL_GRF |
                                    nsSelectionType::SEL_OLE | nsSelectionType::SEL_FRM |
                                    nsSelectionType::SEL_TBL_CELLS | nsSelectionType::SEL_DRW |
                                    nsSelectionType::SEL_DRW_TXT)) )
                            eKeyState = KS_CheckAutoCorrect, eNextKeyState = KS_AutoFmtByInput;
                        else
                            eNextKeyState = eKeyState, eKeyState = KS_CheckAutoCorrect;
                    }
                    break;

                case KEY_RETURN | KEY_MOD2:     // ALT-Return
                    if ( !rSh.HasReadonlySel()
                         && !rSh.IsSttPara()
                         && rSh.GetCurNumRule()
                         && !rSh.CrsrInsideInputFld() )
                    {
                        eKeyState = KS_NoNum;
                    }
                    else if( rSh.CanSpecialInsert() )
                        eKeyState = KS_SpecialInsert;
                    break;

                case KEY_BACKSPACE:
                case KEY_BACKSPACE | KEY_SHIFT:
                    if( !rSh.HasReadonlySel() )
                    {
                        sal_Bool bDone = sal_False;
                        // Remove the paragraph indent, if the cursor is at the
                        // beginning of a paragraph, there is no selection
                        // and no numbering rule found at the current paragraph
                        // Also try to remove indent, if current paragraph
                        // has numbering rule, but isn't counted and only
                        // key <backspace> is hit.
                        const bool bOnlyBackspaceKey(
                                    KEY_BACKSPACE == rKeyCode.GetFullCode() );
                        if ( rSh.IsSttPara() &&
                             !rSh.HasSelection() && // i40834
                             ( NULL == rSh.GetCurNumRule() ||
                               ( rSh.IsNoNum() && bOnlyBackspaceKey ) ) )
                        {
                            bDone = rSh.TryRemoveIndent();
                        }

                        if (bDone)
                            eKeyState = KS_Ende;
                        else
                        {
                            if (rSh.IsSttPara() &&
                                ! rSh.IsNoNum())
                            {
                                if (nKS_NUMDOWN_Count > 0 &&
                                    0 < rSh.GetNumLevel())
                                {
                                    eKeyState = KS_NumUp;
                                    nKS_NUMDOWN_Count = 2;
                                    bDone = sal_True;
                                }
                                else if (nKS_NUMINDENTINC_Count > 0)
                                {
                                    eKeyState = KS_NumIndentDec;
                                    nKS_NUMINDENTINC_Count = 2;
                                    bDone = sal_True;
                                }
                            }
                            // If the cursor is in an empty paragraph, which has
                            // a numbering, but not the oultine numbering, and
                            // there is no selection, the numbering has to be
                            // deleted on key <Backspace>.
                            // Otherwise method <SwEditShell::NumOrNoNum(..)>
                            // should only change the <IsCounted()> state of
                            // the current paragraph depending of the key.
                            // On <backspace> it is set to <false>,
                            // on <shift-backspace> it is set to <true>.
                            // Thus, assure that method <SwEditShell::NumOrNum(..)>
                            // is only called for the intended purpose.
                            bool bCallNumOrNoNum( false );
                            {
                                if ( !bDone )
                                {
                                    if ( bOnlyBackspaceKey && !rSh.IsNoNum() )
                                    {
                                        bCallNumOrNoNum = true;
                                    }
                                    else if ( !bOnlyBackspaceKey && rSh.IsNoNum() )
                                    {
                                        bCallNumOrNoNum = true;
                                    }
                                    else if ( bOnlyBackspaceKey &&
                                              rSh.IsSttPara() && rSh.IsEndPara() &&
                                              !rSh.HasSelection() )
                                    {
                                        const SwNumRule* pCurrNumRule( rSh.GetCurNumRule() );
                                        if ( pCurrNumRule &&
                                             pCurrNumRule != rSh.GetOutlineNumRule() )
                                        {
                                            bCallNumOrNoNum = true;
                                        }
                                    }
                                }
                            }
                            if ( bCallNumOrNoNum &&
                                 rSh.NumOrNoNum( !bOnlyBackspaceKey, sal_True ) )
                            {
                                eKeyState = KS_NumOrNoNum;
                            }
                            // <--
                        }
                    }
                    break;

                case KEY_RIGHT:
                    {
                        eFlyState = KS_Fly_Change;
                        nDir = MOVE_RIGHT_BIG;
                        eTblChgMode = nsTblChgWidthHeightType::WH_FLAG_INSDEL | nsTblChgWidthHeightType::WH_COL_RIGHT;
                        nTblChgSize = pModOpt->GetTblVInsert();
                        goto KEYINPUT_CHECKTABLE_INSDEL;
                    }
                case KEY_TAB:
                {

#ifdef SW_CRSR_TIMER
                    sal_Bool bOld = rSh.ChgCrsrTimerFlag( sal_False );
#endif
                    if (rSh.IsFormProtected() || rSh.GetCurrentFieldmark() || rSh.GetChar(sal_False)==CH_TXT_ATR_FORMELEMENT)
                    {
                        eKeyState=KS_GotoNextFieldMark;
                    }
                    else if ( !rSh.IsMultiSelection() && rSh.CrsrInsideInputFld() )
                    {
                        GetView().GetViewFrame()->GetDispatcher()->Execute( FN_GOTO_NEXT_INPUTFLD );
                        eKeyState = KS_Ende;
                    }
                    else
                    if( rSh.GetCurNumRule() && rSh.IsSttOfPara() &&
                        !rSh.HasReadonlySel() )
                    {
                        if ( rSh.IsFirstOfNumRule() &&
                             numfunc::ChangeIndentOnTabAtFirstPosOfFirstListItem() )
                            eKeyState = KS_NumIndentInc;
                        else
                            eKeyState = KS_NumDown;
                    }
                    else if ( rSh.GetTableFmt() )
                    {
                        if( rSh.HasSelection() || rSh.HasReadonlySel() )
                            eKeyState = KS_NextCell;
                        else
                            eKeyState = KS_CheckAutoCorrect, eNextKeyState = KS_NextCell;
                    }
                    else if ( rSh.GetSelectionType() &
                                (nsSelectionType::SEL_GRF |
                                    nsSelectionType::SEL_FRM |
                                    nsSelectionType::SEL_OLE |
                                    nsSelectionType::SEL_DRW |
                                    nsSelectionType::SEL_DRW_FORM))

                            eKeyState = KS_NextObject;
                    else
                    {
                        eKeyState = KS_InsTab;
                        if( rSh.IsSttOfPara() && !rSh.HasReadonlySel() )
                        {
                            SwTxtFmtColl* pColl = rSh.GetCurTxtFmtColl();
                            if( pColl &&
                                //0 <= pColl->GetOutlineLevel() && #i24560#
                                //MAXLEVEL - 1 > pColl->GetOutlineLevel() )//#outline level,zhaojianwei
                                pColl->IsAssignedToListLevelOfOutlineStyle()
                                && MAXLEVEL-1 > pColl->GetAssignedOutlineStyleLevel() )//<-end,zhaojianwei
                                eKeyState = KS_OutlineDown;
                        }
                    }
#ifdef SW_CRSR_TIMER
                    rSh.ChgCrsrTimerFlag( bOld );
#endif
                }
                break;
                case KEY_TAB | KEY_SHIFT:
                {
#ifdef SW_CRSR_TIMER
                    sal_Bool bOld = rSh.ChgCrsrTimerFlag( sal_False );
                    sal_Bool bOld = rSh.ChgCrsrTimerFlag( sal_False );
#endif
                    if (rSh.IsFormProtected() || rSh.GetCurrentFieldmark()|| rSh.GetChar(sal_False)==CH_TXT_ATR_FORMELEMENT)
                    {
                        eKeyState=KS_GotoPrevFieldMark;
                    }
                    else if ( !rSh.IsMultiSelection() && rSh.CrsrInsideInputFld() )
                    {
                        GetView().GetViewFrame()->GetDispatcher()->Execute( FN_GOTO_PREV_INPUTFLD );
                        eKeyState = KS_Ende;
                    }
                    else if( rSh.GetCurNumRule() && rSh.IsSttOfPara() &&
                         !rSh.HasReadonlySel() )
                    {
                        if ( rSh.IsFirstOfNumRule() &&
                             numfunc::ChangeIndentOnTabAtFirstPosOfFirstListItem() )
                            eKeyState = KS_NumIndentDec;
                        else
                            eKeyState = KS_NumUp;
                    }
                    else if ( rSh.GetTableFmt() )
                    {
                        if( rSh.HasSelection() || rSh.HasReadonlySel() )
                            eKeyState = KS_PrevCell;
                        else
                            eKeyState = KS_CheckAutoCorrect, eNextKeyState = KS_PrevCell;
                    }
                    else if ( rSh.GetSelectionType() &
                                (nsSelectionType::SEL_GRF |
                                    nsSelectionType::SEL_FRM |
                                    nsSelectionType::SEL_OLE |
                                    nsSelectionType::SEL_DRW |
                                    nsSelectionType::SEL_DRW_FORM))

                            eKeyState = KS_PrevObject;
                    else
                    {
                        eKeyState = KS_Ende;
                        if( rSh.IsSttOfPara() && !rSh.HasReadonlySel() )
                        {
                            SwTxtFmtColl* pColl = rSh.GetCurTxtFmtColl();
                            //if( pColl && 0 < pColl->GetOutlineLevel() &&  //#outline level,zhaojianwei
                            //  MAXLEVEL - 1 >= pColl->GetOutlineLevel() )
                            if( pColl &&
                                pColl->IsAssignedToListLevelOfOutlineStyle() &&
                                0 < pColl->GetAssignedOutlineStyleLevel())
                                eKeyState = KS_OutlineUp;
                        }
                    }
#ifdef SW_CRSR_TIMER
                    rSh.ChgCrsrTimerFlag( bOld );
#endif
                }
                break;
                case KEY_TAB | KEY_MOD1:
                case KEY_TAB | KEY_MOD2:
                    if( !rSh.HasReadonlySel() )
                    {
                        if( aTmpQHD.HasCntnt() && !rSh.HasSelection() )
                        {
                            // zum naechsten Tip
                            aTmpQHD.Inc( pACorr && pACorr->GetSwFlags().
                                                   bAutoCmpltEndless );
                            eKeyState = KS_NextPrevGlossary;
                        }
                        else if( rSh.GetTableFmt() )
                            eKeyState = KS_InsTab;
                        else if((rSh.GetSelectionType() &
                                    (nsSelectionType::SEL_DRW|nsSelectionType::SEL_DRW_FORM|
                                        nsSelectionType::SEL_FRM|nsSelectionType::SEL_OLE|nsSelectionType::SEL_GRF))  &&
                                rSh.GetDrawView()->areSdrObjectsSelected())
                            eKeyState = KS_EnterDrawHandleMode;
                        else
                        {
                            eKeyState = KS_InsTab;
                        }
                    }
                    break;

                    case KEY_TAB | KEY_MOD1 | KEY_SHIFT:
                        if( aTmpQHD.HasCntnt() && !rSh.HasSelection() &&
                            !rSh.HasReadonlySel() )
                        {
                            // zum vorherigen Tip
                            aTmpQHD.Dec( pACorr && pACorr->GetSwFlags().
                                                        bAutoCmpltEndless );
                            eKeyState = KS_NextPrevGlossary;
                        }
                        else if((rSh.GetSelectionType() & (nsSelectionType::SEL_DRW|nsSelectionType::SEL_DRW_FORM|
                                        nsSelectionType::SEL_FRM|nsSelectionType::SEL_OLE|nsSelectionType::SEL_GRF)) &&
                                rSh.GetDrawView()->areSdrObjectsSelected())
                            eKeyState = KS_EnterDrawHandleMode;
                    break;
                    case KEY_F2 :
                    if( !rSh.HasReadonlySel() )
                    {
                        const int nSelectionType = rSh.GetSelectionType();
                        if(nSelectionType & nsSelectionType::SEL_FRM)
                            eKeyState = KS_GoIntoFly;
                        else if((nSelectionType & nsSelectionType::SEL_DRW))
                            eKeyState = KS_GoIntoDrawing;
                    }
                    break;
                }
            }
            break;
        case KS_CheckDocReadOnlyKeys:
            {
                eKeyState = KS_KeyToView;
                switch( rKeyCode.GetModifier() | rKeyCode.GetCode() )
                {
                    case KEY_TAB:
                    case KEY_TAB | KEY_SHIFT:
                        bNormalChar = sal_False;
                        eKeyState = KS_Ende;
                        if ( rSh.GetSelectionType() &
                                (nsSelectionType::SEL_GRF |
                                    nsSelectionType::SEL_FRM |
                                    nsSelectionType::SEL_OLE |
                                    nsSelectionType::SEL_DRW |
                                    nsSelectionType::SEL_DRW_FORM))

                        {
                            eKeyState = rKeyCode.GetModifier() & KEY_SHIFT ?
                                                KS_PrevObject : KS_NextObject;
                        }
                        else if ( !rSh.IsMultiSelection() && rSh.CrsrInsideInputFld() )
                        {
                            GetView().GetViewFrame()->GetDispatcher()->Execute(
                                KEY_SHIFT != rKeyCode.GetModifier() ? FN_GOTO_NEXT_INPUTFLD : FN_GOTO_PREV_INPUTFLD );
                        }
                        else
                        {
                            rSh.SelectNextPrevHyperlink( KEY_SHIFT != rKeyCode.GetModifier() );
                        }
                    break;
                    case KEY_RETURN:
                    {
                        const int nSelectionType = rSh.GetSelectionType();
                        if(nSelectionType & nsSelectionType::SEL_FRM)
                            eKeyState = KS_GoIntoFly;
                        else
                        {
                            SfxItemSet aSet(rSh.GetAttrPool(), RES_TXTATR_INETFMT, RES_TXTATR_INETFMT);
                            rSh.GetCurAttr(aSet);
                            if(SFX_ITEM_SET == aSet.GetItemState(RES_TXTATR_INETFMT, sal_False))
                            {
                                const SfxPoolItem& rItem = aSet.Get(RES_TXTATR_INETFMT, sal_True);
                                bNormalChar = sal_False;
                                eKeyState = KS_Ende;
                                rSh.ClickToINetAttr((const SwFmtINetFmt&)rItem, URLLOAD_NOFILTER);
                            }
                        }
                    }
                    break;
                }
            }
            break;

        case KS_EnterCharCell:
            {
                eKeyState = KS_KeyToView;
                switch ( rKeyCode.GetModifier() | rKeyCode.GetCode() )
                {
                    case KEY_RIGHT | KEY_MOD2:
                        rSh.Right( CRSR_SKIP_CHARS, sal_False, 1, sal_False );
                        eKeyState = KS_Ende;
                        FlushInBuffer();
                        break;
                    case KEY_LEFT | KEY_MOD2:
                        rSh.Left( CRSR_SKIP_CHARS, sal_False, 1, sal_False );
                        eKeyState = KS_Ende;
                        FlushInBuffer();
                        break;
                }
            }
            break;

        case KS_KeyToView:
            {
                eKeyState = KS_Ende;
                bNormalChar =
                    !rKeyCode.IsMod2() &&
                    rKeyCode.GetModifier() != (KEY_MOD1) &&
                    rKeyCode.GetModifier() != (KEY_MOD1|KEY_SHIFT) &&
                                SW_ISPRINTABLE( aCh );

                if (bNormalChar && rSh.IsInFrontOfLabel())
                {
                    rSh.NumOrNoNum(sal_False);
                }

                if( aInBuffer.Len() && ( !bNormalChar || bIsDocReadOnly ))
                    FlushInBuffer();

                if( rView.KeyInput( aKeyEvent ) )
                    bFlushBuffer = sal_True, bNormalChar = sal_False;
                else
                {
// OS 22.09.95: Da der Sfx Acceleratoren nur aufruft, wenn sie beim letzten
//              Statusupdate enabled wurden, muss copy ggf. von uns
//              'gewaltsam' gerufen werden.
                    if( rKeyCode.GetFunction() == KEYFUNC_COPY )
                        GetView().GetViewFrame()->GetBindings().Execute(SID_COPY);


                    if( !bIsDocReadOnly && bNormalChar )
                    {
                        const int nSelectionType = rSh.GetSelectionType();
                        SdrObject* pObj = rSh.GetDrawView()->getSelectedIfSingle();

                        if((nSelectionType & nsSelectionType::SEL_DRW) &&
                            0 == (nSelectionType & nsSelectionType::SEL_DRW_TXT) &&
                            pObj)
                        {
                            EnterDrawTextMode(sdr::legacy::GetLogicRange(*pObj).getCenter());
                            if ( dynamic_cast< SwDrawTextShell* >(rView.GetCurShell()) )
                                    ((SwDrawTextShell*)rView.GetCurShell())->Init();
                                rSh.GetDrawView()->KeyInput( rKEvt, this );
                            }
                        else if(nSelectionType & nsSelectionType::SEL_FRM)
                        {
                            rSh.UnSelectFrm();
                            rSh.LeaveSelFrmMode();
                            rView.AttrChangedNotify(&rSh);
                            rSh.MoveSection( fnSectionCurr, fnSectionEnd );
                        }
                        eKeyState = KS_InsChar;
                    }
                    else
                    {
                        bNormalChar = sal_False;
                        Window::KeyInput( aKeyEvent );
                    }
                }
            }
            break;
        case KS_LaunchOLEObject:
            rSh.LaunchOLEObj();
            eKeyState = KS_Ende;
        break;
        case KS_GoIntoFly :
            rSh.UnSelectFrm();
            rSh.LeaveSelFrmMode();
            rView.AttrChangedNotify(&rSh);
            rSh.MoveSection( fnSectionCurr, fnSectionEnd );
            eKeyState = KS_Ende;
        break;
        case KS_GoIntoDrawing :
        {
            SdrObject* pObj = rSh.GetDrawView()->getSelectedIfSingle();
            if(pObj)
            {
                EnterDrawTextMode(sdr::legacy::GetLogicRange(*pObj).getCenter());
                if ( dynamic_cast< SwDrawTextShell* >(rView.GetCurShell()) )
                    ((SwDrawTextShell*)rView.GetCurShell())->Init();
            }
            eKeyState = KS_Ende;
        }
        break;
        case KS_EnterDrawHandleMode:
        {
            const SdrHdlList& rHdlList = rSh.GetDrawView()->GetHdlList();
            sal_Bool bForward(!aKeyEvent.GetKeyCode().IsShift());

            ((SdrHdlList&)rHdlList).TravelFocusHdl(bForward);
            eKeyState = KS_Ende;
        }
        break;
        case KS_InsTab:
            if( dynamic_cast< const SwWebView* >(&rView))     //Kein Tabulator fuer Web!
            {
                Window::KeyInput( aKeyEvent );
                eKeyState = KS_Ende;
                break;
            }
            aCh = '\t';
            // kein break!
        case KS_InsChar:
            if (rSh.GetChar(sal_False)==CH_TXT_ATR_FORMELEMENT)
            {
                ::sw::mark::ICheckboxFieldmark* pFieldmark =
                    dynamic_cast< ::sw::mark::ICheckboxFieldmark* >
                        (rSh.GetCurrentFieldmark());
                OSL_ENSURE(pFieldmark,
                    "Where is my FieldMark??");
                if(pFieldmark)
                {
                    pFieldmark->SetChecked(!pFieldmark->IsChecked());
                    OSL_ENSURE(pFieldmark->IsExpanded(),
                        "where is the otherpos?");
                    if (pFieldmark->IsExpanded())
                    {
                        rSh.CalcLayout();
                    }
                }
                eKeyState = KS_Ende;
            }
            else if(!rSh.HasReadonlySel())
            {
                sal_Bool bIsNormalChar = GetAppCharClass().isLetterNumeric(
                                                            String( aCh ), 0 );
                if( bChkInsBlank && bIsNormalChar &&
                    (aInBuffer.Len() || !rSh.IsSttPara() || !rSh.IsEndPara() ))
                {
                    // vor dem Zeichen noch ein Blank einfuegen. Dieses
                    // kommt zwischen den Expandierten Text und dem neuen
                    // "nicht Worttrenner".
                    aInBuffer.Expand( aInBuffer.Len() + 1, ' ' );
                }


                sal_Bool bIsAutoCorrectChar =  SvxAutoCorrect::IsAutoCorrectChar( aCh );
                sal_Bool bRunNext = pACorr && pACorr->HasRunNext();
                if( !aKeyEvent.GetRepeat() && pACorr && ( bIsAutoCorrectChar || bRunNext ) &&
                        pACfg->IsAutoFmtByInput() &&
                    (( pACorr->IsAutoCorrFlag( ChgWeightUnderl ) &&
                        ( '*' == aCh || '_' == aCh ) ) ||
                     ( pACorr->IsAutoCorrFlag( ChgQuotes ) && ('\"' == aCh ))||
                     ( pACorr->IsAutoCorrFlag( ChgSglQuotes ) && ( '\'' == aCh))))
                {
                    FlushInBuffer();
                    rSh.AutoCorrect( *pACorr, aCh );
                    if( '\"' != aCh && '\'' != aCh )        // nur bei "*_" rufen!
                        rSh.UpdateAttr();
                }
                else if( !aKeyEvent.GetRepeat() && pACorr && ( bIsAutoCorrectChar || bRunNext ) &&
                        pACfg->IsAutoFmtByInput() &&
                    pACorr->IsAutoCorrFlag( CptlSttSntnc | CptlSttWrd |
                                            ChgOrdinalNumber | AddNonBrkSpace |
                                            ChgToEnEmDash | SetINetAttr |
                                            Autocorrect ) &&
                    '\"' != aCh && '\'' != aCh && '*' != aCh && '_' != aCh
                    )
                {
                    FlushInBuffer();
                    rSh.AutoCorrect( *pACorr, aCh );
                }
                else
                {
                    aInBuffer.Expand( aInBuffer.Len() + aKeyEvent.GetRepeat() + 1,aCh );
                    bFlushCharBuffer = Application::AnyInput( INPUT_KEYBOARD );
                    bFlushBuffer = !bFlushCharBuffer;
                    if( bFlushCharBuffer )
                        aKeyInputFlushTimer.Start();
                }
                eKeyState = KS_Ende;
            }
            else
            {
                InfoBox( this, SW_RES( MSG_READONLY_CONTENT )).Execute();
                eKeyState = KS_Ende;
            }
        break;

        case KS_CheckAutoCorrect:
        {
            if( pACorr && pACfg->IsAutoFmtByInput() &&
                pACorr->IsAutoCorrFlag( CptlSttSntnc | CptlSttWrd |
                                        ChgOrdinalNumber |
                                        ChgToEnEmDash | SetINetAttr |
                                        Autocorrect ) &&
                !rSh.HasReadonlySel() )
            {
                FlushInBuffer();
                rSh.AutoCorrect( *pACorr, static_cast< sal_Unicode >('\0') );
            }
            eKeyState = eNextKeyState;
        }
        break;

        default:
        {
            sal_uInt16 nSlotId = 0;
            FlushInBuffer();
            switch( eKeyState )
            {
            case KS_SpecialInsert:
                rSh.DoSpecialInsert();
                break;

            case KS_NoNum:
                rSh.NoNum();
                break;

            case KS_NumOff:
                // Shellwechsel - also vorher aufzeichnen
                rSh.DelNumRules();
                eKeyState = eNextKeyState;
                break;
            case KS_OutlineLvOff: // delete autofmt outlinelevel later
                break;

            case KS_NumDown:
                rSh.NumUpDown( sal_True );
                nKS_NUMDOWN_Count = 2; // #i23725#
                break;
            case KS_NumUp:
                rSh.NumUpDown( sal_False );
                break;

            case KS_NumIndentInc:
                rSh.ChangeIndentOfAllListLevels(360);
                nKS_NUMINDENTINC_Count = 2;
                break;

            case KS_GotoNextFieldMark:
                {
                    ::sw::mark::IFieldmark const * const pFieldmark = rSh.GetFieldmarkAfter();
                    if(pFieldmark) rSh.GotoFieldmark(pFieldmark);
                }
                break;

            case KS_GotoPrevFieldMark:
                {
                    ::sw::mark::IFieldmark const * const pFieldmark = rSh.GetFieldmarkBefore();
                    if(pFieldmark) rSh.GotoFieldmark(pFieldmark);
                }
                break;

            case KS_NumIndentDec:
                rSh.ChangeIndentOfAllListLevels(-360);
                // <--
                break;

            case KS_OutlineDown:
                rSh.OutlineUpDown( 1 );
                break;
            case KS_OutlineUp:
                rSh.OutlineUpDown( -1 );
                break;

            case KS_NextCell:
                //In Tabelle immer 'flushen'
                rSh.GoNextCell();
                nSlotId = FN_GOTO_NEXT_CELL;
                break;
            case KS_PrevCell:
                rSh.GoPrevCell();
                nSlotId = FN_GOTO_PREV_CELL;
                break;
            case KS_AutoFmtByInput:
                rSh.SplitNode( sal_True );
                break;

            case KS_NextObject:
            case KS_PrevObject:
                if(rSh.GotoObj( KS_NextObject == eKeyState, GOTOOBJ_GOTO_ANY))
                {
                    if( rSh.IsFrmSelected() &&
                        rView.GetDrawFuncPtr() )
                    {
                        rView.GetDrawFuncPtr()->Deactivate();
                        rView.SetDrawFuncPtr(NULL);
                        rView.LeaveDrawCreate();
                        rView.AttrChangedNotify( &rSh );
                    }
                    rSh.HideCrsr();
                    rSh.EnterSelFrmMode();
                }
            break;
            case KS_GlossaryExpand:
            {
                // ersetze das Wort oder Kuerzel durch den den Textbaustein
                rSh.StartUndo( UNDO_START );

                String sFnd( *aTmpQHD.aArr[ aTmpQHD.nCurArrPos ] );
                if( aTmpQHD.bIsAutoText )
                {
                    SwGlossaryList* pList = ::GetGlossaryList();
                    String sShrtNm;
                    String sGroup;
                    if(pList->GetShortName( sFnd, sShrtNm, sGroup))
                    {
                        rSh.SttSelect();
                        rSh.ExtendSelection( sal_False, aTmpQHD.nLen );
                        SwGlossaryHdl* pGlosHdl = GetView().GetGlosHdl();
                        pGlosHdl->SetCurGroup(sGroup, sal_True);
                        pGlosHdl->InsertGlossary( sShrtNm);
                        pQuickHlpData->bChkInsBlank = sal_True;
                    }
                }
                else
                {
                    rSh.Insert( sFnd.Erase( 0, aTmpQHD.nLen ));
                    pQuickHlpData->bChkInsBlank = !pACorr ||
                            pACorr->GetSwFlags().bAutoCmpltAppendBlanc;
                }
                rSh.EndUndo( UNDO_END );
            }
            break;

            case KS_NextPrevGlossary:
                pQuickHlpData->Move( aTmpQHD );
                pQuickHlpData->Start( rSh, USHRT_MAX );
                break;

            case KS_EditFormula:
            {
                const sal_uInt16 nId = SwInputChild::GetChildWindowId();

                SfxViewFrame* pVFrame = GetView().GetViewFrame();
                pVFrame->ToggleChildWindow( nId );
                SwInputChild* pChildWin = (SwInputChild*)pVFrame->
                                                    GetChildWindow( nId );
                if( pChildWin )
                    pChildWin->SetFormula( sFmlEntry );
            }
            break;

            case KS_ColLeftBig:         rSh.SetColRowWidthHeight( nsTblChgWidthHeightType::WH_COL_LEFT|nsTblChgWidthHeightType::WH_FLAG_BIGGER, pModOpt->GetTblHMove() );   break;
            case KS_ColRightBig:        rSh.SetColRowWidthHeight( nsTblChgWidthHeightType::WH_COL_RIGHT|nsTblChgWidthHeightType::WH_FLAG_BIGGER, pModOpt->GetTblHMove() );  break;
            case KS_ColLeftSmall:       rSh.SetColRowWidthHeight( nsTblChgWidthHeightType::WH_COL_LEFT, pModOpt->GetTblHMove() );   break;
            case KS_ColRightSmall:      rSh.SetColRowWidthHeight( nsTblChgWidthHeightType::WH_COL_RIGHT, pModOpt->GetTblHMove() );  break;
            case KS_ColBottomBig:       rSh.SetColRowWidthHeight( nsTblChgWidthHeightType::WH_ROW_BOTTOM|nsTblChgWidthHeightType::WH_FLAG_BIGGER, pModOpt->GetTblVMove() ); break;
            case KS_ColBottomSmall:     rSh.SetColRowWidthHeight( nsTblChgWidthHeightType::WH_ROW_BOTTOM, pModOpt->GetTblVMove() ); break;
            case KS_CellLeftBig:        rSh.SetColRowWidthHeight( nsTblChgWidthHeightType::WH_CELL_LEFT|nsTblChgWidthHeightType::WH_FLAG_BIGGER, pModOpt->GetTblHMove() );  break;
            case KS_CellRightBig:       rSh.SetColRowWidthHeight( nsTblChgWidthHeightType::WH_CELL_RIGHT|nsTblChgWidthHeightType::WH_FLAG_BIGGER, pModOpt->GetTblHMove() ); break;
            case KS_CellLeftSmall:      rSh.SetColRowWidthHeight( nsTblChgWidthHeightType::WH_CELL_LEFT, pModOpt->GetTblHMove() );  break;
            case KS_CellRightSmall:     rSh.SetColRowWidthHeight( nsTblChgWidthHeightType::WH_CELL_RIGHT, pModOpt->GetTblHMove() ); break;
            case KS_CellTopBig:         rSh.SetColRowWidthHeight( nsTblChgWidthHeightType::WH_CELL_TOP|nsTblChgWidthHeightType::WH_FLAG_BIGGER, pModOpt->GetTblVMove() );   break;
            case KS_CellBottomBig:      rSh.SetColRowWidthHeight( nsTblChgWidthHeightType::WH_CELL_BOTTOM|nsTblChgWidthHeightType::WH_FLAG_BIGGER, pModOpt->GetTblVMove() );    break;
            case KS_CellTopSmall:       rSh.SetColRowWidthHeight( nsTblChgWidthHeightType::WH_CELL_TOP, pModOpt->GetTblVMove() );   break;
            case KS_CellBottomSmall:    rSh.SetColRowWidthHeight( nsTblChgWidthHeightType::WH_CELL_BOTTOM, pModOpt->GetTblVMove() );    break;

//---------------
            case KS_InsDel_ColLeftBig:          rSh.SetColRowWidthHeight( nsTblChgWidthHeightType::WH_FLAG_INSDEL|nsTblChgWidthHeightType::WH_COL_LEFT|nsTblChgWidthHeightType::WH_FLAG_BIGGER, pModOpt->GetTblHInsert() ); break;
            case KS_InsDel_ColRightBig:         rSh.SetColRowWidthHeight( nsTblChgWidthHeightType::WH_FLAG_INSDEL|nsTblChgWidthHeightType::WH_COL_RIGHT|nsTblChgWidthHeightType::WH_FLAG_BIGGER, pModOpt->GetTblHInsert() );    break;
            case KS_InsDel_ColLeftSmall:        rSh.SetColRowWidthHeight( nsTblChgWidthHeightType::WH_FLAG_INSDEL|nsTblChgWidthHeightType::WH_COL_LEFT, pModOpt->GetTblHInsert() ); break;
            case KS_InsDel_ColRightSmall:       rSh.SetColRowWidthHeight( nsTblChgWidthHeightType::WH_FLAG_INSDEL|nsTblChgWidthHeightType::WH_COL_RIGHT, pModOpt->GetTblHInsert() );    break;
            case KS_InsDel_ColTopBig:           rSh.SetColRowWidthHeight( nsTblChgWidthHeightType::WH_FLAG_INSDEL|nsTblChgWidthHeightType::WH_ROW_TOP|nsTblChgWidthHeightType::WH_FLAG_BIGGER, pModOpt->GetTblVInsert() );  break;
            case KS_InsDel_ColBottomBig:        rSh.SetColRowWidthHeight( nsTblChgWidthHeightType::WH_FLAG_INSDEL|nsTblChgWidthHeightType::WH_ROW_BOTTOM|nsTblChgWidthHeightType::WH_FLAG_BIGGER, pModOpt->GetTblVInsert() );   break;
            case KS_InsDel_ColTopSmall:         rSh.SetColRowWidthHeight( nsTblChgWidthHeightType::WH_FLAG_INSDEL|nsTblChgWidthHeightType::WH_ROW_TOP, pModOpt->GetTblVInsert() );  break;
            case KS_InsDel_ColBottomSmall:      rSh.SetColRowWidthHeight( nsTblChgWidthHeightType::WH_FLAG_INSDEL|nsTblChgWidthHeightType::WH_ROW_BOTTOM, pModOpt->GetTblVInsert() );   break;
            case KS_InsDel_CellLeftBig:         rSh.SetColRowWidthHeight( nsTblChgWidthHeightType::WH_FLAG_INSDEL|nsTblChgWidthHeightType::WH_CELL_LEFT|nsTblChgWidthHeightType::WH_FLAG_BIGGER, pModOpt->GetTblHInsert() );    break;
            case KS_InsDel_CellRightBig:        rSh.SetColRowWidthHeight( nsTblChgWidthHeightType::WH_FLAG_INSDEL|nsTblChgWidthHeightType::WH_CELL_RIGHT|nsTblChgWidthHeightType::WH_FLAG_BIGGER, pModOpt->GetTblHInsert() );   break;
            case KS_InsDel_CellLeftSmall:       rSh.SetColRowWidthHeight( nsTblChgWidthHeightType::WH_FLAG_INSDEL|nsTblChgWidthHeightType::WH_CELL_LEFT, pModOpt->GetTblHInsert() );    break;
            case KS_InsDel_CellRightSmall:      rSh.SetColRowWidthHeight( nsTblChgWidthHeightType::WH_FLAG_INSDEL|nsTblChgWidthHeightType::WH_CELL_RIGHT, pModOpt->GetTblHInsert() );   break;
            case KS_InsDel_CellTopBig:          rSh.SetColRowWidthHeight( nsTblChgWidthHeightType::WH_FLAG_INSDEL|nsTblChgWidthHeightType::WH_CELL_TOP|nsTblChgWidthHeightType::WH_FLAG_BIGGER, pModOpt->GetTblVInsert() ); break;
            case KS_InsDel_CellBottomBig:       rSh.SetColRowWidthHeight( nsTblChgWidthHeightType::WH_FLAG_INSDEL|nsTblChgWidthHeightType::WH_CELL_BOTTOM|nsTblChgWidthHeightType::WH_FLAG_BIGGER, pModOpt->GetTblVInsert() );  break;
            case KS_InsDel_CellTopSmall:        rSh.SetColRowWidthHeight( nsTblChgWidthHeightType::WH_FLAG_INSDEL|nsTblChgWidthHeightType::WH_CELL_TOP, pModOpt->GetTblVInsert() ); break;
            case KS_InsDel_CellBottomSmall:     rSh.SetColRowWidthHeight( nsTblChgWidthHeightType::WH_FLAG_INSDEL|nsTblChgWidthHeightType::WH_CELL_BOTTOM, pModOpt->GetTblVInsert() );  break;
//---------------
            case KS_TblColCellInsDel:
                rSh.SetColRowWidthHeight( eTblChgMode, nTblChgSize );
                break;
            case KS_Fly_Change:
            {
                SdrView *pSdrView = rSh.GetDrawView();
                const SdrHdlList& rHdlList = pSdrView->GetHdlList();
                if(rHdlList.GetFocusHdl())
                    ChangeDrawing( nDir );
                else
                    ChangeFly( nDir, 0 != dynamic_cast< const SwWebView* >(&rView) );
            }
            break;
            case KS_Draw_Change :
                ChangeDrawing( nDir );
                break;
            default:; //prevent warning
            }
            if( nSlotId && rView.GetViewFrame()->GetBindings().GetRecorder().is() )
            {
                SfxRequest aReq(rView.GetViewFrame(), nSlotId );
                aReq.Done();
            }
            eKeyState = KS_Ende;
        }
        }
    }

    if( bStopKeyInputTimer )
    {
        aKeyInputTimer.Stop();
        bTblInsDelMode = sal_False;
    }

    // falls die gepufferten Zeichen eingefuegt werden sollen
    if( bFlushBuffer && aInBuffer.Len() )
    {
        //OS 16.02.96 11.04: bFlushCharBuffer wurde hier nicht zurueckgesetzt
        // warum nicht?
        sal_Bool bSave = bFlushCharBuffer;
        FlushInBuffer();
        bFlushCharBuffer = bSave;

        // evt. Tip-Hilfe anzeigen
        String sWord;
        if( bNormalChar && pACfg && pACorr &&
            ( pACfg->IsAutoTextTip() ||
              pACorr->GetSwFlags().bAutoCompleteWords ) &&
            rSh.GetPrevAutoCorrWord( *pACorr, sWord ) )
        {
            ShowAutoTextCorrectQuickHelp(sWord, pACfg, pACorr);
        }
    }
}

/*--------------------------------------------------------------------
     Beschreibung:  MouseEvents
 --------------------------------------------------------------------*/


void SwEditWin::RstMBDownFlags()
{
    //Nicht auf allen Systemen kommt vor dem modalen
    //Dialog noch ein MouseButton Up (wie unter WINDOWS).
    //Daher hier die Stati zuruecksetzen und die Maus
    //fuer den Dialog freigeben.
    bMBPressed = bNoInterrupt = sal_False;
    EnterArea();
    ReleaseMouse();
}



void SwEditWin::MouseButtonDown(const MouseEvent& _rMEvt)
{
    SwWrtShell &rSh = rView.GetWrtShell();

    // We have to check if a context menu is shown and we have an UI
    // active inplace client. In that case we have to ignore the mouse
    // button down event. Otherwise we would crash (context menu has been
    // opened by inplace client and we would deactivate the inplace client,
    // the contex menu is closed by VCL asynchronously which in the end
    // would work on deleted objects or the context menu has no parent anymore)
    // See #126086# and #128122#
    SfxInPlaceClient* pIPClient = rSh.GetSfxViewShell()->GetIPClient();
    sal_Bool bIsOleActive = ( pIPClient && pIPClient->IsObjectInPlaceActive() );

    if ( bIsOleActive && PopupMenu::IsInExecute() )
        return;

    MouseEvent rMEvt(_rMEvt);

    if (rView.GetPostItMgr()->IsHit(rMEvt.GetPosPixel()))
        return;

    rView.GetPostItMgr()->SetActiveSidebarWin(0);

    GrabFocus();

    //ignore key modifiers for format paintbrush
    {
        sal_Bool bExecFormatPaintbrush = pApplyTempl && pApplyTempl->pFormatClipboard
                                &&  pApplyTempl->pFormatClipboard->HasContent();
        if( bExecFormatPaintbrush )
            rMEvt = MouseEvent( _rMEvt.GetPosPixel(), _rMEvt.GetClicks(),
                                    _rMEvt.GetMode(), _rMEvt.GetButtons() );
    }

    bWasShdwCrsr = 0 != pShadCrsr;
    if( bWasShdwCrsr )
        delete pShadCrsr, pShadCrsr = 0;

    const Point aDocPos( PixelToLogic( rMEvt.GetPosPixel() ) );
    const basegfx::B2DPoint aB2DDocPos(aDocPos.X(), aDocPos.Y());

    if ( IsChainMode() )
    {
        SetChainMode( sal_False );
        SwRect aDummy;
        SwFlyFrmFmt *pFmt = (SwFlyFrmFmt*)rSh.GetFlyFrmFmt();
        if ( !rSh.Chainable( aDummy, *pFmt, aDocPos ) )
            rSh.Chain( *pFmt, aDocPos );
        UpdatePointer( aDocPos, rMEvt.GetModifier() );
        return;
    }

    //Nach GrabFocus sollte eine Shell gepusht sein. Das muss eigentlich
    //klappen aber in der Praxis ...
    lcl_SelectShellForDrop( rView );

    sal_Bool bIsDocReadOnly = rView.GetDocShell()->IsReadOnly();
    sal_Bool bCallBase = sal_True;

    if( pQuickHlpData->bClear )
        pQuickHlpData->Stop( rSh );
    pQuickHlpData->bChkInsBlank = sal_False;

    if( rSh.FinishOLEObj() )
        return; //InPlace beenden und der Klick zaehlt nicht mehr

    SET_CURR_SHELL( &rSh );

    SdrView *pSdrView = rSh.GetDrawView();
    if ( pSdrView )
    {
        if (pSdrView->MouseButtonDown( rMEvt, this ) )
        {
            rSh.GetView().GetViewFrame()->GetBindings().InvalidateAll(sal_False);
            return; // Event von der SdrView ausgewertet
        }
    }


    bIsInMove = sal_False;
    aStartPos = rMEvt.GetPosPixel();
    aRszMvHdlPt.X() = 0, aRszMvHdlPt.Y() = 0;

    sal_uInt8 nMouseTabCol = 0;
    const sal_Bool bTmp = !rSh.IsDrawCreate() && !pApplyTempl && !rSh.IsInSelect() &&
         rMEvt.GetClicks() == 1 && MOUSE_LEFT == rMEvt.GetButtons();
    if (  bTmp &&
         0 != (nMouseTabCol = rSh.WhichMouseTabCol( aDocPos ) ) &&
         !rSh.IsObjSelectable(aB2DDocPos) )
    {
        // Enhanced table selection
        if ( SW_TABSEL_HORI <= nMouseTabCol && SW_TABCOLSEL_VERT >= nMouseTabCol )
        {
            rSh.EnterStdMode();
            rSh.SelectTableRowCol( aDocPos );
            if( SW_TABSEL_HORI  != nMouseTabCol && SW_TABSEL_HORI_RTL  != nMouseTabCol)
            {
                pRowColumnSelectionStart = new Point( aDocPos );
                bIsRowDrag = SW_TABROWSEL_HORI == nMouseTabCol||
                            SW_TABROWSEL_HORI_RTL == nMouseTabCol ||
                            SW_TABCOLSEL_VERT == nMouseTabCol;
                bMBPressed = sal_True;
                CaptureMouse();
            }
            return;
        }

        if ( !rSh.IsTableMode() )
        {
            //Zuppeln von Tabellenspalten aus dem Dokument heraus.
            if(SW_TABCOL_VERT == nMouseTabCol || SW_TABCOL_HORI == nMouseTabCol)
                rView.SetTabColFromDoc( sal_True );
            else
                rView.SetTabRowFromDoc( sal_True );

            rView.SetTabColFromDocPos( aDocPos );
            rView.InvalidateRulerPos();
            SfxBindings& rBind = rView.GetViewFrame()->GetBindings();
            rBind.Update();
            if ( RulerColumnDrag( rMEvt,
                    (SW_TABCOL_VERT == nMouseTabCol || SW_TABROW_HORI == nMouseTabCol)) )
            {
                rView.SetTabColFromDoc( sal_False );
                rView.SetTabRowFromDoc( sal_False );
                rView.InvalidateRulerPos();
                rBind.Update();
                bCallBase = sal_False;
            }
            else
            {
                return;
            }
        }
    }
    else if (bTmp &&
             rSh.IsNumLabel(aDocPos))
    {
        SwTxtNode* pNodeAtPos = rSh.GetNumRuleNodeAtPos( aDocPos );
        rView.SetNumRuleNodeFromDoc( pNodeAtPos );
        rView.InvalidateRulerPos();
        SfxBindings& rBind = rView.GetViewFrame()->GetBindings();
        rBind.Update();

        if ( RulerMarginDrag( rMEvt,
                        rSh.IsVerticalModeAtNdAndPos( *pNodeAtPos, aDocPos ) ) )
        {
            rView.SetNumRuleNodeFromDoc( NULL );
            rView.InvalidateRulerPos();
            rBind.Update();
            bCallBase = sal_False;
        }
        else
        {
            // Make sure the pointer is set to 0, otherwise it may point to
            // nowhere after deleting the corresponding text node.
            rView.SetNumRuleNodeFromDoc( NULL );
            return;
        }
    }

    if ( rSh.IsInSelect() )
        rSh.EndSelect();

    //Abfrage auf LEFT, da sonst auch bei einem Click mit der rechten Taste
    //beispielsweise die Selektion aufgehoben wird.
    if ( MOUSE_LEFT == rMEvt.GetButtons() )
    {
        sal_Bool bOnlyText = sal_False;
        bMBPressed = bNoInterrupt = sal_True;
        nKS_NUMDOWN_Count = 0; // #i23725#

        CaptureMouse();

        //ggf. Cursorpositionen zuruecksetzen
        rSh.ResetCursorStack();

        switch ( rMEvt.GetModifier() + rMEvt.GetButtons() )
        {
            case MOUSE_LEFT:
            case MOUSE_LEFT + KEY_SHIFT:
            case MOUSE_LEFT + KEY_MOD2:
                if( rSh.IsObjSelected() )
                {
                    SdrHdl* pHdl;
                    if( !bIsDocReadOnly &&
                        !pAnchorMarker &&
                        0 != ( pHdl = pSdrView->PickHandle(aB2DDocPos) ) &&
                            ( pHdl->GetKind() == HDL_ANCHOR ||
                              pHdl->GetKind() == HDL_ANCHOR_TR ) )
                    {
                        // Set selected during drag
                        pHdl->SetSelected(true);
                        pAnchorMarker = new SwAnchorMarker( *pHdl );
                        UpdatePointer( aDocPos, rMEvt.GetModifier() );
                        return;
                    }
                }
                if ( EnterDrawMode( rMEvt, aDocPos ) )
                {
                    bNoInterrupt = sal_False;
                    return;
                }
                else  if ( rView.GetDrawFuncPtr() && bInsFrm )
                {
                    StopInsFrm();
                    rSh.Edit();
                }

                // Ohne SHIFT, da sonst Toggle bei Selektion nicht funktioniert
                if (rMEvt.GetClicks() == 1)
                {
                    if ( rSh.IsSelFrmMode())
                    {
                        SdrHdl* pHdl = rSh.GetDrawView()->PickHandle(aB2DDocPos);
                        sal_Bool bHitHandle = pHdl && pHdl->GetKind() != HDL_ANCHOR &&
                                                  pHdl->GetKind() != HDL_ANCHOR_TR;

                        if ((rSh.IsInsideSelectedObj(aB2DDocPos) || bHitHandle) &&
                            !(rMEvt.GetModifier() == KEY_SHIFT && !bHitHandle))
                        {
                            rSh.EnterSelFrmMode( &aDocPos );
                            if ( !pApplyTempl )
                            {
                                //nur, wenn keine Position zum Sizen getroffen ist.
                                if (!bHitHandle)
                                {
                                    StartDDTimer();
                                    SwEditWin::nDDStartPosY = aDocPos.Y();
                                    SwEditWin::nDDStartPosX = aDocPos.X();
                                }
                                bFrmDrag = sal_True;
                            }
                            bNoInterrupt = sal_False;
                            return;
                        }
                    }
                }
        }

        sal_Bool bExecHyperlinks = rView.GetDocShell()->IsReadOnly();
        if ( !bExecHyperlinks )
        {
            SvtSecurityOptions aSecOpts;
            const sal_Bool bSecureOption = aSecOpts.IsOptionSet( SvtSecurityOptions::E_CTRLCLICK_HYPERLINK );
            if ( (  bSecureOption && rMEvt.GetModifier() == KEY_MOD1 ) ||
                 ( !bSecureOption && rMEvt.GetModifier() != KEY_MOD1 ) )
                bExecHyperlinks = sal_True;
        }

        // Enhanced selection
        sal_uInt8 nNumberOfClicks = static_cast< sal_uInt8 >(rMEvt.GetClicks() % 4);
        if ( 0 == nNumberOfClicks && 0 < rMEvt.GetClicks() )
            nNumberOfClicks = 4;

        sal_Bool bExecDrawTextLink = sal_False;

        switch ( rMEvt.GetModifier() + rMEvt.GetButtons() )
        {
            case MOUSE_LEFT:
            case MOUSE_LEFT + KEY_MOD1:
            case MOUSE_LEFT + KEY_MOD2:
                switch ( nNumberOfClicks )
                {
                    case 1:
                    {
                        UpdatePointer( aDocPos, rMEvt.GetModifier() );
                        SwEditWin::nDDStartPosY = aDocPos.Y();
                        SwEditWin::nDDStartPosX = aDocPos.X();

                        // URL in DrawText-Objekt getroffen?
                        if (bExecHyperlinks && pSdrView)
                        {
                            SdrViewEvent aVEvt;
                            pSdrView->PickAnything(rMEvt, SDRMOUSEBUTTONDOWN, aVEvt);

                            if (aVEvt.meEvent == SDREVENT_EXECUTEURL)
                                bExecDrawTextLink = sal_True;
                        }

                        //Rahmen nur zu selektieren versuchen, wenn
                        //der Pointer bereits entsprechend geschaltet wurde
                        if ( aActHitType != SDRHIT_NONE && !rSh.IsSelFrmMode() &&
                            !GetView().GetViewFrame()->GetDispatcher()->IsLocked() &&
                            !bExecDrawTextLink)
                        {
                            // Test if there is a draw object at that position and if it should be selected.
                            sal_Bool bShould = rSh.ShouldObjectBeSelected(aDocPos);

                            if(bShould)
                            {
                                rView.NoRotate();
                                rSh.HideCrsr();

                                sal_Bool bUnLockView = !rSh.IsViewLocked();
                                rSh.LockView( sal_True );
                                sal_Bool bSelObj = rSh.SelectObj( aDocPos,
                                               rMEvt.IsMod1() ? SW_ENTER_GROUP : 0);
                                if( bUnLockView )
                                    rSh.LockView( sal_False );

                                if( bSelObj )
                                {
                                    // falls im Macro der Rahmen deselektiert
                                    // wurde, muss nur noch der Cursor
                                    // wieder angezeigt werden.
                                    if( FRMTYPE_NONE == rSh.GetSelFrmType() )
                                        rSh.ShowCrsr();
                                    else
                                    {
                                        if (rSh.IsFrmSelected() && rView.GetDrawFuncPtr())
                                        {
                                            rView.GetDrawFuncPtr()->Deactivate();
                                            rView.SetDrawFuncPtr(NULL);
                                            rView.LeaveDrawCreate();
                                            rView.AttrChangedNotify( &rSh );
                                        }

                                        rSh.EnterSelFrmMode( &aDocPos );
                                        bFrmDrag = sal_True;
                                        UpdatePointer( aDocPos, rMEvt.GetModifier() );
                                    }
                                    return;
                                }
                                else
                                    bOnlyText = rSh.IsObjSelectable(aB2DDocPos);

                                if (!rView.GetDrawFuncPtr())
                                    rSh.ShowCrsr();
                            }
                            else
                                bOnlyText = KEY_MOD1 != rMEvt.GetModifier();
                        }
                        else if ( rSh.IsSelFrmMode() &&
                                  (aActHitType == SDRHIT_NONE ||
                                   !rSh.IsInsideSelectedObj( aB2DDocPos )))
                        {
                            rView.NoRotate();
                            SdrHdl *pHdl;
                            if( !bIsDocReadOnly && !pAnchorMarker && 0 !=
                                ( pHdl = pSdrView->PickHandle(aB2DDocPos) ) &&
                                    ( pHdl->GetKind() == HDL_ANCHOR ||
                                      pHdl->GetKind() == HDL_ANCHOR_TR ) )
                            {
                                pAnchorMarker = new SwAnchorMarker( *pHdl );
                                UpdatePointer( aDocPos, rMEvt.GetModifier() );
                                return;
                            }
                            else
                            {
                                sal_Bool bUnLockView = !rSh.IsViewLocked();
                                rSh.LockView( sal_True );
                                sal_uInt8 nFlag = rMEvt.IsShift() ? SW_ADD_SELECT :0;
                                if( rMEvt.IsMod1() )
                                    nFlag = nFlag | SW_ENTER_GROUP;

                                if ( rSh.IsSelFrmMode() )
                                {
                                    rSh.UnSelectFrm();
                                    rSh.LeaveSelFrmMode();
                                    rView.AttrChangedNotify(&rSh);
                                }

                                sal_Bool bSelObj = rSh.SelectObj( aDocPos, nFlag );
                                if( bUnLockView )
                                    rSh.LockView( sal_False );

                                if( !bSelObj )
                                {
                                    // Cursor hier umsetzen, damit er nicht zuerst
                                    // im Rahmen gezeichnet wird; ShowCrsr() geschieht
                                    // in LeaveSelFrmMode()
                                    bValidCrsrPos = !(CRSR_POSCHG & (rSh.*rSh.fnSetCrsr)(&aDocPos,sal_False));
                                    rSh.LeaveSelFrmMode();
                                    rView.AttrChangedNotify( &rSh );
                                    bCallBase = sal_False;
                                }
                                else
                                {
                                    rSh.HideCrsr();
                                    rSh.EnterSelFrmMode( &aDocPos );
                                    rSh.SelFlyGrabCrsr();
                                    rSh.MakeSelVisible();
                                    bFrmDrag = sal_True;
                                    if( rSh.IsFrmSelected() &&
                                        rView.GetDrawFuncPtr() )
                                    {
                                        rView.GetDrawFuncPtr()->Deactivate();
                                        rView.SetDrawFuncPtr(NULL);
                                        rView.LeaveDrawCreate();
                                        rView.AttrChangedNotify( &rSh );
                                    }
                                    UpdatePointer( aDocPos, rMEvt.GetModifier() );
                                    return;
                                }
                            }
                        }

                        break;
                    }
                    case 2:
                    {
                        bFrmDrag = sal_False;
                        if ( !bIsDocReadOnly && rSh.IsInsideSelectedObj(aB2DDocPos) &&
                             0 == rSh.IsSelObjProtected( FLYPROTECT_CONTENT|FLYPROTECT_PARENT ) )

/* SJ: 01.03.2005: this is no good, on the one hand GetSelectionType is used as flag field (take a look into the GetSelectionType method)
   on the other hand the return value is used in a switch without proper masking (very nice), this must lead to trouble
*/
                        switch ( rSh.GetSelectionType() &~ ( nsSelectionType::SEL_FONTWORK | nsSelectionType::SEL_EXTRUDED_CUSTOMSHAPE ) )
                        {
                            case nsSelectionType::SEL_GRF:
                                RstMBDownFlags();
                                GetView().GetViewFrame()->GetBindings().Execute(
                                    FN_FORMAT_GRAFIC_DLG, 0, 0,
                                    SFX_CALLMODE_RECORD|SFX_CALLMODE_SLOT);
                                return;

                                // Doppelklick auf OLE-Objekt --> OLE-InPlace
                            case nsSelectionType::SEL_OLE:
                                if (!rSh.IsSelObjProtected(FLYPROTECT_CONTENT))
                                {
                                    RstMBDownFlags();
                                    rSh.LaunchOLEObj();
                                }
                                return;

                            case nsSelectionType::SEL_FRM:
                                RstMBDownFlags();
                                GetView().GetViewFrame()->GetBindings().Execute(
                                    FN_FORMAT_FRAME_DLG, 0, 0, SFX_CALLMODE_RECORD|SFX_CALLMODE_SLOT);
                                return;

                            case nsSelectionType::SEL_DRW:
                                RstMBDownFlags();
                                EnterDrawTextMode(aB2DDocPos);
                                if ( dynamic_cast< SwDrawTextShell* >(rView.GetCurShell()) )
                                    ((SwDrawTextShell*)rView.GetCurShell())->Init();
                                return;
                        }

                        //falls die Cursorposition korrigiert wurde oder
                        // ein Fly im ReadOnlyModus selektiert ist,
                        //keine Wortselektion.
                        if ( !bValidCrsrPos ||
                            (rSh.IsFrmSelected() && rSh.IsFrmSelected() ))
                            return;

                        SwField *pFld;
                        sal_Bool bFtn = sal_False;

                        if( !bIsDocReadOnly &&
                            ( 0 != ( pFld = rSh.GetCurFld() ) ||
                              0 != ( bFtn = rSh.GetCurFtn() )) )
                        {
                            RstMBDownFlags();
                            if( bFtn )
                                GetView().GetViewFrame()->GetBindings().Execute( FN_EDIT_FOOTNOTE );
                            else
                            {
                                sal_uInt16 nTypeId = pFld->GetTypeId();
                                SfxViewFrame* pVFrame = GetView().GetViewFrame();
                                switch( nTypeId )
                                {
                                case TYP_POSTITFLD:
                                case TYP_SCRIPTFLD:
                                {
                                    //falls es ein Readonly-Bereich ist, dann muss der Status
                                    //enabled werden
                                    sal_uInt16 nSlot = TYP_POSTITFLD == nTypeId ? FN_POSTIT : FN_JAVAEDIT;
                                    SfxBoolItem aItem(nSlot, sal_True);
                                    pVFrame->GetBindings().SetState(aItem);
                                    pVFrame->GetBindings().Execute(nSlot);
                                    break;
                                }
                                case TYP_AUTHORITY :
                                    pVFrame->GetBindings().Execute(FN_EDIT_AUTH_ENTRY_DLG);
                                break;
                                default:
                                    pVFrame->GetBindings().Execute(FN_EDIT_FIELD);
                                }
                            }
                            return;
                        }
                        //im Extended Mode hat Doppel- und
                        //Dreifachklick keine Auswirkungen.
                        if ( rSh.IsExtMode() || rSh.IsBlockMode() )
                            return;

                        //Wort selektieren, gfs. Additional Mode
                        if ( KEY_MOD1 == rMEvt.GetModifier() && !rSh.IsAddMode() )
                        {
                            rSh.EnterAddMode();
                            rSh.SelWrd( &aDocPos );
                            rSh.LeaveAddMode();
                        }
                        else
                            rSh.SelWrd( &aDocPos );
                        bHoldSelection = sal_True;
                        return;
                    }
                    case 3:
                    case 4:
                    {
                        bFrmDrag = sal_False;
                        //im Extended Mode hat Doppel- und
                        //Dreifachklick keine Auswirkungen.
                        if ( rSh.IsExtMode() )
                            return;

                        //falls die Cursorposition korrigiert wurde oder
                        // ein Fly im ReadOnlyModus selektiert ist,
                        //keine Wortselektion.
                        if ( !bValidCrsrPos || rSh.IsFrmSelected() )
                            return;

                        //Zeile selektieren, gfs. Additional Mode
                        const bool bMod = KEY_MOD1 == rMEvt.GetModifier() &&
                                         !rSh.IsAddMode();

                        if ( bMod )
                            rSh.EnterAddMode();

                        // Enhanced selection
                        if ( 3 == nNumberOfClicks )
                            rSh.SelSentence( &aDocPos );
                        else
                            rSh.SelPara( &aDocPos );

                        if ( bMod )
                            rSh.LeaveAddMode();

                        bHoldSelection = sal_True;
                        return;
                    }

                    default:
                        return;
                }
                /* no break */
            case MOUSE_LEFT + KEY_SHIFT:
            case MOUSE_LEFT + KEY_SHIFT + KEY_MOD1:
            {
                sal_Bool bLockView = bWasShdwCrsr;

                switch ( rMEvt.GetModifier() )
                {
                    case KEY_MOD1 + KEY_SHIFT:
                    {
                        if ( !bInsDraw && IsDrawObjSelectable( rSh, aDocPos ) )
                        {
                            rView.NoRotate();
                            rSh.HideCrsr();
                            if ( rSh.IsSelFrmMode() )
                                rSh.SelectObj(aDocPos, SW_ADD_SELECT | SW_ENTER_GROUP);
                            else
                            {   if ( rSh.SelectObj( aDocPos, SW_ADD_SELECT | SW_ENTER_GROUP ) )
                                {
                                    rSh.EnterSelFrmMode( &aDocPos );
                                    SwEditWin::nDDStartPosY = aDocPos.Y();
                                    SwEditWin::nDDStartPosX = aDocPos.X();
                                    bFrmDrag = sal_True;
                                    return;
                                }
                            }
                        }
                        else if( rSh.IsSelFrmMode() &&
                                 rSh.GetDrawView()->PickHandle( aB2DDocPos ))
                        {
                            bFrmDrag = sal_True;
                            bNoInterrupt = sal_False;
                            return;
                        }
                    }
                    break;
                    case KEY_MOD1:
                    if ( !bExecDrawTextLink )
                    {
                        if ( !bInsDraw && IsDrawObjSelectable( rSh, aDocPos ) )
                        {
                            rView.NoRotate();
                            rSh.HideCrsr();
                            if ( rSh.IsSelFrmMode() )
                                rSh.SelectObj(aDocPos, SW_ENTER_GROUP);
                            else
                            {   if ( rSh.SelectObj( aDocPos, SW_ENTER_GROUP ) )
                                {
                                    rSh.EnterSelFrmMode( &aDocPos );
                                    SwEditWin::nDDStartPosY = aDocPos.Y();
                                    SwEditWin::nDDStartPosX = aDocPos.X();
                                    bFrmDrag = sal_True;
                                    return;
                                }
                            }
                        }
                        else if( rSh.IsSelFrmMode() &&
                                 rSh.GetDrawView()->PickHandle( aB2DDocPos ))
                        {
                            bFrmDrag = sal_True;
                            bNoInterrupt = sal_False;
                            return;
                        }
                        else
                        {
                            if ( !rSh.IsAddMode() && !rSh.IsExtMode() && !rSh.IsBlockMode() )
                            {
                                rSh.PushMode();
                                bModePushed = sal_True;

                                sal_Bool bUnLockView = !rSh.IsViewLocked();
                                rSh.LockView( sal_True );
                                rSh.EnterAddMode();
                                if( bUnLockView )
                                    rSh.LockView( sal_False );
                            }
                            bCallBase = sal_False;
                        }
                    }
                    break;
                    case KEY_MOD2:
                    {
                        if ( !rSh.IsAddMode() && !rSh.IsExtMode() && !rSh.IsBlockMode() )
                        {
                            rSh.PushMode();
                            bModePushed = sal_True;
                            sal_Bool bUnLockView = !rSh.IsViewLocked();
                            rSh.LockView( sal_True );
                            rSh.EnterBlockMode();
                            if( bUnLockView )
                                rSh.LockView( sal_False );
                        }
                        bCallBase = sal_False;
                    }
                    break;
                    case KEY_SHIFT:
                    {
                        if ( !bInsDraw && IsDrawObjSelectable( rSh, aDocPos ) )
                        {
                            rView.NoRotate();
                            rSh.HideCrsr();
                            if ( rSh.IsSelFrmMode() )
                            {
                                rSh.SelectObj(aDocPos, SW_ADD_SELECT);

                                if(!pSdrView->areSdrObjectsSelected())
                                {
                                    rSh.LeaveSelFrmMode();
                                    rView.AttrChangedNotify(&rSh);
                                    bFrmDrag = sal_False;
                                }
                            }
                            else
                            {   if ( rSh.SelectObj( aDocPos ) )
                                {
                                    rSh.EnterSelFrmMode( &aDocPos );
                                    SwEditWin::nDDStartPosY = aDocPos.Y();
                                    SwEditWin::nDDStartPosX = aDocPos.X();
                                    bFrmDrag = sal_True;
                                    return;
                                }
                            }
                        }
                        else
                        {
                            if ( rSh.IsSelFrmMode() &&
                                 rSh.IsInsideSelectedObj(aB2DDocPos) )
                            {
                                rSh.EnterSelFrmMode( &aDocPos );
                                SwEditWin::nDDStartPosY = aDocPos.Y();
                                SwEditWin::nDDStartPosX = aDocPos.X();
                                bFrmDrag = sal_True;
                                return;
                            }
                            if ( rSh.IsSelFrmMode() )
                            {
                                rSh.UnSelectFrm();
                                rSh.LeaveSelFrmMode();
                                rView.AttrChangedNotify(&rSh);
                                bFrmDrag = sal_False;
                            }
                            if ( !rSh.IsExtMode() )
                            {
                                // keine Selection anfangen, wenn in ein URL-
                                // Feld oder eine -Grafik geklickt wird
                                sal_Bool bSttSelect = rSh.HasSelection() ||
                                                Pointer(POINTER_REFHAND) != GetPointer();

                                if( !bSttSelect )
                                {
                                    bSttSelect = sal_True;
                                    if( bExecHyperlinks )
                                    {
                                        SwContentAtPos aCntntAtPos(
                                            SwContentAtPos::SW_FTN |
                                            SwContentAtPos::SW_INETATTR );

                                        if( rSh.GetContentAtPos( aDocPos, aCntntAtPos ) )
                                        {
                                            if( !rSh.IsViewLocked() &&
                                                !rSh.IsReadOnlyAvailable() &&
                                                aCntntAtPos.IsInProtectSect() )
                                                    bLockView = sal_True;

                                            bSttSelect = sal_False;
                                        }
                                        else if( rSh.IsURLGrfAtPos( aDocPos ))
                                            bSttSelect = sal_False;
                                    }
                                }

                                if( bSttSelect )
                                    rSh.SttSelect();
                            }
                        }
                        bCallBase = sal_False;
                        break;
                    }
                    default:
                        if( !rSh.IsViewLocked() )
                        {
                            SwContentAtPos aCntntAtPos( SwContentAtPos::SW_CLICKFIELD |
                                                        SwContentAtPos::SW_INETATTR );
                            if( rSh.GetContentAtPos( aDocPos, aCntntAtPos, sal_False ) &&
                                !rSh.IsReadOnlyAvailable() &&
                                aCntntAtPos.IsInProtectSect() )
                                bLockView = sal_True;
                        }
                }

                if ( rSh.IsGCAttr() )
                {
                    rSh.GCAttr();
                    rSh.ClearGCAttr();
                }

                sal_Bool bOverSelect = rSh.ChgCurrPam( aDocPos ), bOverURLGrf = sal_False;
                if( !bOverSelect )
                    bOverURLGrf = bOverSelect = 0 != rSh.IsURLGrfAtPos( aDocPos );

                if ( !bOverSelect )
                {
                    const sal_Bool bTmpNoInterrupt = bNoInterrupt;
                    bNoInterrupt = sal_False;

                    if( !rSh.IsViewLocked() && bLockView )
                        rSh.LockView( sal_True );
                    else
                        bLockView = sal_False;

                    int nTmpSetCrsr = 0;

                    {   // nur temp. Move-Kontext aufspannen, da sonst die
                        // Abfrage auf die Inhaltsform nicht funktioniert!!!
                        MV_KONTEXT( &rSh );
                        nTmpSetCrsr = (rSh.*rSh.fnSetCrsr)(&aDocPos,bOnlyText);
                        bValidCrsrPos = !(CRSR_POSCHG & nTmpSetCrsr);
                        bCallBase = sal_False;
                    }

                    //#i42732# - notify the edit window that from now on we do not use the input language
                    if ( !(CRSR_POSOLD & nTmpSetCrsr) )
                        SetUseInputLanguage( sal_False );

                    if( bLockView )
                        rSh.LockView( sal_False );

                    bNoInterrupt = bTmpNoInterrupt;
                }
                if ( !bOverURLGrf && !bOnlyText )
                {
                    const int nSelType = rSh.GetSelectionType();
                    // Check in general, if an object is selectable at given position.
                    // Thus, also text fly frames in background become selectable via Ctrl-Click.
                    if ( nSelType & nsSelectionType::SEL_OLE ||
                         nSelType & nsSelectionType::SEL_GRF ||
                         rSh.IsObjSelectable(aB2DDocPos) )
                    {
                        MV_KONTEXT( &rSh );
                        if( !rSh.IsFrmSelected() )
                            rSh.GotoNextFly();
                        rSh.EnterSelFrmMode();
                        bCallBase = sal_False;
                    }
                }
                break;
            }
        }
    }
    if (bCallBase)
        Window::MouseButtonDown(rMEvt);
}

/*--------------------------------------------------------------------
    Beschreibung:   MouseMove
 --------------------------------------------------------------------*/


void SwEditWin::MouseMove(const MouseEvent& _rMEvt)
{
    MouseEvent rMEvt(_rMEvt);

    //ignore key modifiers for format paintbrush
    {
        sal_Bool bExecFormatPaintbrush = pApplyTempl && pApplyTempl->pFormatClipboard
                                &&  pApplyTempl->pFormatClipboard->HasContent();
        if( bExecFormatPaintbrush )
            rMEvt = MouseEvent( _rMEvt.GetPosPixel(), _rMEvt.GetClicks(),
                                    _rMEvt.GetMode(), _rMEvt.GetButtons() );
    }

    // solange eine Action laeuft sollte das MouseMove abgeklemmt sein
    // Ansonsten gibt es den Bug 40102
    SwWrtShell &rSh = rView.GetWrtShell();
    if( rSh.ActionPend() )
        return ;

    if( pShadCrsr && 0 != (rMEvt.GetModifier() + rMEvt.GetButtons() ) )
        delete pShadCrsr, pShadCrsr = 0;

    sal_Bool bIsDocReadOnly = rView.GetDocShell()->IsReadOnly();

    SET_CURR_SHELL( &rSh );

    //aPixPt == Point in Pixel, rel. zu ChildWin
    //aDocPt == Point in Twips, Dokumentkoordinaten
    const Point aPixPt( rMEvt.GetPosPixel() );
    const Point aDocPt( PixelToLogic( aPixPt ) );
    const basegfx::B2DPoint aB2DDocPt(aDocPt.X(), aDocPt.Y());

    if ( IsChainMode() )
    {
        UpdatePointer( aDocPt, rMEvt.GetModifier() );
        if ( rMEvt.IsLeaveWindow() )
            rView.GetViewFrame()->HideStatusText();
        return;
    }

    SdrView *pSdrView = rSh.GetDrawView();

    const SwCallMouseEvent aLastCallEvent( aSaveCallEvent );
    aSaveCallEvent.Clear();

    if ( !bIsDocReadOnly && pSdrView && pSdrView->MouseMove(rMEvt,this) )
    {
        SetPointer( POINTER_TEXT );
        return; // Event von der SdrView ausgewertet
    }

    const Point aOldPt( rSh.VisArea().Pos() );
    const sal_Bool bInsWin = rSh.VisArea().IsInside( aDocPt );

    if( pShadCrsr && !bInsWin )
        delete pShadCrsr, pShadCrsr = 0;

    if( bInsWin && pRowColumnSelectionStart )
    {
        EnterArea();
        Point aPos( aDocPt );
        if( rSh.SelectTableRowCol( *pRowColumnSelectionStart, &aPos, bIsRowDrag ))
            return;
    }

    // Position ist noetig fuer OS/2, da dort nach einem MB-Down
    // offensichtlich sofort ein MB-Move gerufen wird.
    if( bDDTimerStarted )
    {
        Point aDD( SwEditWin::nDDStartPosX, SwEditWin::nDDStartPosY );
        aDD = LogicToPixel( aDD );
        Rectangle aRect( aDD.X()-3, aDD.Y()-3, aDD.X()+3, aDD.Y()+3 );
        if ( !aRect.IsInside( aPixPt ) )    // MA 23. May. 95: Tatterschutz.
            StopDDTimer( &rSh, aDocPt );
    }

    if(rView.GetDrawFuncPtr())
    {
        if( bInsDraw  )
        {
            rView.GetDrawFuncPtr()->MouseMove( rMEvt );
            if ( !bInsWin )
            {
                Point aTmp( aDocPt );
                aTmp += rSh.VisArea().Pos() - aOldPt;
                LeaveArea( aTmp );
            }
            else
                EnterArea();
            return;
        }
        else if(!rSh.IsFrmSelected() && !rSh.IsObjSelected())
        {
            SfxBindings &rBnd = rSh.GetView().GetViewFrame()->GetBindings();
            Point aRelPos = rSh.GetRelativePagePosition(aDocPt);
            if(aRelPos.X() >= 0)
            {
                FieldUnit eMetric = ::GetDfltMetric(0 != dynamic_cast< SwWebView* >( &GetView()));
                SW_MOD()->PutItem(SfxUInt16Item(SID_ATTR_METRIC, static_cast< sal_uInt16 >(eMetric)));
                const SfxPointItem aTmp1( SID_ATTR_POSITION, aRelPos );
                rBnd.SetState( aTmp1 );
            }
            else
            {
                rBnd.Invalidate(SID_ATTR_POSITION);
            }
            rBnd.Invalidate(SID_ATTR_SIZE);
            const SfxStringItem aCell( SID_TABLE_CELL, aEmptyStr );
            rBnd.SetState( aCell );
        }
    }

    sal_uInt8 nMouseTabCol;
    if( !bIsDocReadOnly && bInsWin && !pApplyTempl && !rSh.IsInSelect() )
    {
        if ( SW_TABCOL_NONE != (nMouseTabCol = rSh.WhichMouseTabCol( aDocPt ) ) &&
             !rSh.IsObjSelectable(aB2DDocPt) )
        {
            sal_uInt16 nPointer = USHRT_MAX;
            bool bChkTblSel = false;

            switch ( nMouseTabCol )
            {
                case SW_TABCOL_VERT :
                case SW_TABROW_HORI :
                    nPointer = POINTER_VSIZEBAR;
                    bChkTblSel = true;
                    break;
                case SW_TABROW_VERT :
                case SW_TABCOL_HORI :
                    nPointer = POINTER_HSIZEBAR;
                    bChkTblSel = true;
                    break;
                // Enhanced table selection
                case SW_TABSEL_HORI :
                    nPointer = POINTER_TAB_SELECT_SE;
                    break;
                case SW_TABSEL_HORI_RTL :
                case SW_TABSEL_VERT :
                    nPointer = POINTER_TAB_SELECT_SW;
                    break;
                case SW_TABCOLSEL_HORI :
                case SW_TABROWSEL_VERT :
                    nPointer = POINTER_TAB_SELECT_S;
                    break;
                case SW_TABROWSEL_HORI :
                    nPointer = POINTER_TAB_SELECT_E;
                    break;
                case SW_TABROWSEL_HORI_RTL :
                case SW_TABCOLSEL_VERT :
                    nPointer = POINTER_TAB_SELECT_W;
                    break;
            }

            if ( USHRT_MAX != nPointer &&
                // Enhanced table selection is explicitely allowed in table mode
                ( !bChkTblSel || !rSh.IsTableMode() ) )
            {
                SetPointer( nPointer );
            }

            return;
        }
        else if (rSh.IsNumLabel(aDocPt, RULER_MOUSE_MARGINWIDTH))
        {
            SwTxtNode* pNodeAtPos = rSh.GetNumRuleNodeAtPos( aDocPt );
            const sal_uInt16 nPointer =
                    rSh.IsVerticalModeAtNdAndPos( *pNodeAtPos, aDocPt )
                    ? POINTER_VSIZEBAR
                    : POINTER_HSIZEBAR;
            SetPointer( nPointer );

            return;
        }
    }

    sal_Bool bDelShadCrsr = sal_True;

    switch ( rMEvt.GetModifier() + rMEvt.GetButtons() )
    {
        case MOUSE_LEFT:
            if( pAnchorMarker )
            {
                // Now we need to refresh the SdrHdl pointer of pAnchorMarker.
                // This looks a little bit tricky, but it solves the following
                // problem: the pAnchorMarker contains a pointer to an SdrHdl,
                // if the FindAnchorPos-call cause a scrolling of the visible
                // area, it's possible that the SdrHdl will be destroyed and a
                // new one will initialized at the original position(GetHdlPos).
                // So the pAnchorMarker has to find the right SdrHdl, if it's
                // the old one, it will find it with position aOld, if this one
                // is destroyed, it will find a new one at position GetHdlPos().
                // --> OD 2010-09-16 #i114522#
                const basegfx::B2DPoint aOld(pAnchorMarker->GetPosForHitTest(*(rSh.GetOut())));
                Point aNew = rSh.FindAnchorPos( aDocPt );
                SdrHdl* pHdl;
                if( (0!=( pHdl = pSdrView->PickHandle( aOld ) )||
                    0 !=(pHdl = pSdrView->PickHandle( pAnchorMarker->GetHdlPos()) ) ) &&
                        ( pHdl->GetKind() == HDL_ANCHOR ||
                          pHdl->GetKind() == HDL_ANCHOR_TR ) )
                {
                    pAnchorMarker->ChgHdl( *pHdl );
                    if( aNew.X() || aNew.Y() )
                    {
                        pAnchorMarker->SetPos( basegfx::B2DPoint(aNew.X(), aNew.Y()) );
                        pAnchorMarker->SetLastPos( aB2DDocPt );
                    }
                }
                else
                {
                    delete pAnchorMarker;
                    pAnchorMarker = NULL;
                }
            }
            if ( bInsDraw )
            {
                if ( !bMBPressed )
                    break;
                if ( bIsInMove || IsMinMove( aStartPos, aPixPt ) )
                {
                    if ( !bInsWin )
                        LeaveArea( aDocPt );
                    else
                        EnterArea();
                    if ( rView.GetDrawFuncPtr() )
                    {
                        pSdrView->SetOrthogonal(false);
                        rView.GetDrawFuncPtr()->MouseMove( rMEvt );
                    }
                    bIsInMove = sal_True;
                }
                return;
            }
        case MOUSE_LEFT + KEY_SHIFT:
        case MOUSE_LEFT + KEY_SHIFT + KEY_MOD1:
            if ( !bMBPressed )
                break;
        case MOUSE_LEFT + KEY_MOD1:
            if ( bFrmDrag && rSh.IsSelFrmMode() )
            {
                if( !bMBPressed )
                    break;

                if ( bIsInMove || IsMinMove( aStartPos, aPixPt ) )
                {
                    // Event-Verarbeitung fuers Resizen
                    if( pSdrView->areSdrObjectsSelected() )
                    {
                        const SwFrmFmt* pFlyFmt;
                        const SvxMacro* pMacro;

                        const Point aSttPt( PixelToLogic( aStartPos ) );

                        // geht es los?
                        if( HDL_USER == eSdrMoveHdl )
                        {
                            SdrHdl* pHdl = pSdrView->PickHandle( basegfx::B2DPoint(aSttPt.X(), aSttPt.Y()) );
                            eSdrMoveHdl = pHdl ? pHdl->GetKind() : HDL_MOVE;
                        }

                        sal_uInt16 nEvent = HDL_MOVE == eSdrMoveHdl
                                            ? SW_EVENT_FRM_MOVE
                                            : SW_EVENT_FRM_RESIZE;

                        if( 0 != ( pFlyFmt = rSh.GetFlyFrmFmt() ) &&
                            0 != ( pMacro = pFlyFmt->GetMacro().GetMacroTable().
                            Get( nEvent )) &&
                            aRszMvHdlPt != aDocPt )
                        {
                            aRszMvHdlPt = aDocPt;
                            sal_uInt16 nPos = 0;
                            String sRet;
                            SbxArrayRef xArgs = new SbxArray;
                            SbxVariableRef xVar = new SbxVariable;
                            xVar->PutString( pFlyFmt->GetName() );
                            xArgs->Put( &xVar, ++nPos );

                            if( SW_EVENT_FRM_RESIZE == nEvent )
                            {
                                xVar = new SbxVariable;
                                xVar->PutUShort( static_cast< sal_uInt16 >(eSdrMoveHdl) );
                                xArgs->Put( &xVar, ++nPos );
                            }

                            xVar = new SbxVariable;
                            xVar->PutLong( aDocPt.X() - aSttPt.X() );
                            xArgs->Put( &xVar, ++nPos );
                            xVar = new SbxVariable;
                            xVar->PutLong( aDocPt.Y() - aSttPt.Y() );
                            xArgs->Put( &xVar, ++nPos );

                            ReleaseMouse();

                            rSh.ExecMacro( *pMacro, &sRet, &xArgs );

                            CaptureMouse();

                            if( sRet.Len() && 0 != sRet.ToInt32() )
                                return ;
                        }
                    }
                    // Event-Verarbeitung fuers Resizen

                    if( bIsDocReadOnly )
                        break;

                    if ( rMEvt.IsShift() )
                    {
                        pSdrView->SetOrthogonal(true);
                        pSdrView->SetAngleSnapEnabled(true);
                    }
                    else
                    {
                        pSdrView->SetOrthogonal(false);
                        pSdrView->SetAngleSnapEnabled(false);
                    }

                    (rSh.*rSh.fnDrag)( &aDocPt, rMEvt.IsShift() );
                    bIsInMove = sal_True;
                }
                else if( bIsDocReadOnly )
                    break;

                if ( !bInsWin )
                {
                    Point aTmp( aDocPt );
                    aTmp += rSh.VisArea().Pos() - aOldPt;
                    LeaveArea( aTmp );
                }
                else if(bIsInMove)
                    EnterArea();
                return;
            }
            if ( !rSh.IsSelFrmMode() && !bDDINetAttr &&
                (IsMinMove( aStartPos,aPixPt ) || bIsInMove) &&
                (rSh.IsInSelect() || !rSh.ChgCurrPam( aDocPt )) )
            {
                if ( pSdrView )
                {
                    if ( rMEvt.IsShift() )
                        pSdrView->SetOrthogonal(true);
                    else
                        pSdrView->SetOrthogonal(false);
                }
                if ( !bInsWin )
                {
                    Point aTmp( aDocPt );
                    aTmp += rSh.VisArea().Pos() - aOldPt;
                    LeaveArea( aTmp );
                }
                else
                {
                    //JP 24.09.98: Fix fuer die Bugs 55592 / 55931
                    //JP 23.04.99: Fix fuer den Bugs 65289
                    //JP 06.07.99: Fix fuer den Bugs 67360
                    if( !rMEvt.IsSynthetic() &&
                            !(( MOUSE_LEFT + KEY_MOD1 ==
                            rMEvt.GetModifier() + rMEvt.GetButtons() ) &&
                            rSh.Is_FnDragEQBeginDrag() && !rSh.IsAddMode() ))
                    {
                        (rSh.*rSh.fnDrag)( &aDocPt,sal_False );

                        bValidCrsrPos = !(CRSR_POSCHG & (rSh.*rSh.fnSetCrsr)(&aDocPt,sal_False));
                        EnterArea();
                    }
                }
            }
            bDDINetAttr = sal_False;
            break;
        case 0:
        {
            if ( pApplyTempl )
                        {
                UpdatePointer(aDocPt, 0); // evtl. muss hier ein Rahmen markiert werden
                                break;
                        }
            //#i6193#, change ui if mouse is over SwPostItField
            // TODO: do the same thing for redlines SW_REDLINE
            SwRect aFldRect;
            SwContentAtPos aCntntAtPos( SwContentAtPos::SW_FIELD);
            if( rSh.GetContentAtPos( aDocPt, aCntntAtPos, sal_False, &aFldRect ) )
            {
                const SwField* pFld = aCntntAtPos.aFnd.pFld;
                if (pFld->Which()== RES_POSTITFLD)
                {
                    rView.GetPostItMgr()->SetShadowState(reinterpret_cast<const SwPostItField*>(pFld),false);
                }
                else
                    rView.GetPostItMgr()->SetShadowState(0,false);
            }
            else
                rView.GetPostItMgr()->SetShadowState(0,false);
                // no break;
        }
        case KEY_SHIFT:
        case KEY_MOD2:
        case KEY_MOD1:
            if ( !bInsDraw )
            {
                sal_Bool bTstShdwCrsr = sal_True;

                UpdatePointer( aDocPt, rMEvt.GetModifier() );

                const SwFrmFmt* pFmt = 0;
                const SwFmtINetFmt* pINet = 0;
                SwContentAtPos aCntntAtPos( SwContentAtPos::SW_INETATTR );
                if( rSh.GetContentAtPos( aDocPt, aCntntAtPos ) )
                    pINet = (SwFmtINetFmt*)aCntntAtPos.aFnd.pAttr;

                const void* pTmp = pINet;

                if( pINet ||
                    0 != ( pTmp = pFmt = rSh.GetFmtFromAnyObj( aDocPt )))
                {
                    bTstShdwCrsr = sal_False;
                    if( pTmp == pINet )
                        aSaveCallEvent.Set( pINet );
                    else
                    {
                        IMapObject* pIMapObj = pFmt->GetIMapObject( aDocPt );
                        if( pIMapObj )
                            aSaveCallEvent.Set( pFmt, pIMapObj );
                        else
                            aSaveCallEvent.Set( EVENT_OBJECT_URLITEM, pFmt );
                    }

                    // sollte wir ueber einem InternetFeld mit einem
                    // gebundenen Macro stehen?
                    if( aSaveCallEvent != aLastCallEvent )
                    {
                        if( aLastCallEvent.HasEvent() )
                            rSh.CallEvent( SFX_EVENT_MOUSEOUT_OBJECT,
                                            aLastCallEvent, sal_True );
                        // 0 besagt, das das Object gar keine Tabelle hat
                        if( !rSh.CallEvent( SFX_EVENT_MOUSEOVER_OBJECT,
                                        aSaveCallEvent ))
                            aSaveCallEvent.Clear();
                    }
                }
                else if( aLastCallEvent.HasEvent() )
                {
                    // Cursor stand auf einem Object
                    rSh.CallEvent( SFX_EVENT_MOUSEOUT_OBJECT,
                                    aLastCallEvent, sal_True );
                }

                if( bTstShdwCrsr && bInsWin && !bIsDocReadOnly &&
                    !bInsFrm &&
                    !rSh.GetViewOptions()->getBrowseMode() &&
                    rSh.GetViewOptions()->IsShadowCursor() &&
                    !(rMEvt.GetModifier() + rMEvt.GetButtons()) &&
                    !rSh.HasSelection() && !GetConnectMetaFile() )
                {
                    SwRect aRect;
                    sal_Int16 eOrient;
                    SwFillMode eMode = (SwFillMode)rSh.GetViewOptions()->GetShdwCrsrFillMode();
                    if( rSh.GetShadowCrsrPos( aDocPt, eMode, aRect, eOrient ))
                    {
                        if( !pShadCrsr )
                            pShadCrsr = new SwShadowCursor( *this,
                                SwViewOption::GetDirectCursorColor() );
                        if( text::HoriOrientation::RIGHT != eOrient && text::HoriOrientation::CENTER != eOrient )
                            eOrient = text::HoriOrientation::LEFT;
                        pShadCrsr->SetPos( aRect.Pos(), aRect.Height(), static_cast< sal_uInt16 >(eOrient) );
                        bDelShadCrsr = sal_False;
                    }
                }
            }
            break;
        case MOUSE_LEFT + KEY_MOD2:
            if( rSh.IsBlockMode() && !rMEvt.IsSynthetic() )
            {
                (rSh.*rSh.fnDrag)( &aDocPt,sal_False );
                bValidCrsrPos = !(CRSR_POSCHG & (rSh.*rSh.fnSetCrsr)(&aDocPt,sal_False));
                EnterArea();
            }
        break;
    }

    if( bDelShadCrsr && pShadCrsr )
        delete pShadCrsr, pShadCrsr = 0;
    bWasShdwCrsr = sal_False;
}

/*--------------------------------------------------------------------
    Beschreibung:   Button Up
 --------------------------------------------------------------------*/


void SwEditWin::MouseButtonUp(const MouseEvent& rMEvt)
{
    sal_Bool bCallBase = sal_True;

    sal_Bool bCallShadowCrsr = bWasShdwCrsr;
    bWasShdwCrsr = sal_False;
    if( pShadCrsr )
        delete pShadCrsr, pShadCrsr = 0;

    if( pRowColumnSelectionStart )
        DELETEZ( pRowColumnSelectionStart );

    SdrHdlKind eOldSdrMoveHdl = eSdrMoveHdl;
    eSdrMoveHdl = HDL_USER;     // fuer die MoveEvents - wieder zuruecksetzen

    // sicherheitshalber zuruecksetzen Bug 27900
    rView.SetTabColFromDoc( sal_False );
    rView.SetNumRuleNodeFromDoc(NULL);

    SwWrtShell &rSh = rView.GetWrtShell();
    SET_CURR_SHELL( &rSh );
    SdrView *pSdrView = rSh.GetDrawView();
    if ( pSdrView )
    {
        pSdrView->SetOrthogonal(false);

        if ( pSdrView->MouseButtonUp( rMEvt,this ) )
        {
            rSh.GetView().GetViewFrame()->GetBindings().InvalidateAll(sal_False);
            return; // Event von der SdrView ausgewertet
        }
    }
    //MouseButtonUp nur bearbeiten, wenn auch das Down an dieses Fenster ging.
    if ( !bMBPressed )
    {
        return;
    }

    Point aDocPt( PixelToLogic( rMEvt.GetPosPixel() ) );

    if ( bDDTimerStarted )
    {
        StopDDTimer( &rSh, aDocPt );
        bMBPressed = sal_False;
        if ( rSh.IsSelFrmMode() )
        {
            (rSh.*rSh.fnEndDrag)( &aDocPt, sal_False );
            bFrmDrag = sal_False;
        }
        bNoInterrupt = sal_False;
        ReleaseMouse();
        return;
    }

    if( pAnchorMarker )
    {
        // #121463# delete selected after drag
        pAnchorMarker->SetSelected(false);

        Point aPnt(basegfx::fround(pAnchorMarker->GetLastPos().getX()), basegfx::fround(pAnchorMarker->GetLastPos().getY()));
        DELETEZ( pAnchorMarker );
        if( aPnt.X() || aPnt.Y() )
            rSh.FindAnchorPos( aPnt, sal_True );
    }
    if ( bInsDraw && rView.GetDrawFuncPtr() )
    {
        if ( rView.GetDrawFuncPtr()->MouseButtonUp( rMEvt ) )
        {
            if (rView.GetDrawFuncPtr()) // Koennte im MouseButtonUp zerstoert worden sein
            {
                rView.GetDrawFuncPtr()->Deactivate();

                if (!rView.IsDrawMode())
                {
                    rView.SetDrawFuncPtr(NULL);
                    SfxBindings& rBind = rView.GetViewFrame()->GetBindings();
                    rBind.Invalidate( SID_ATTR_SIZE );
                    rBind.Invalidate( SID_TABLE_CELL );
                }
            }

            if ( rSh.IsObjSelected() )
            {
                rSh.EnterSelFrmMode();
                if (!rView.GetDrawFuncPtr())
                    StdDrawMode( OBJ_NONE, sal_True );
            }
            else if ( rSh.IsFrmSelected() )
            {
                rSh.EnterSelFrmMode();
                StopInsFrm();
            }
            else
            {
                const Point aDocPos( PixelToLogic( aStartPos ) );
                bValidCrsrPos = !(CRSR_POSCHG & (rSh.*rSh.fnSetCrsr)(&aDocPos,sal_False));
                rSh.Edit();
            }

            rView.AttrChangedNotify( &rSh );
        }
        else if (rMEvt.GetButtons() == MOUSE_RIGHT && rSh.IsDrawCreate())
            rView.GetDrawFuncPtr()->BreakCreate();   // Zeichnen abbrechen

        bNoInterrupt = sal_False;
        ReleaseMouse();
        return;
    }
    sal_Bool bPopMode = sal_False;
    switch ( rMEvt.GetModifier() + rMEvt.GetButtons() )
    {
        case MOUSE_LEFT:
            if ( bInsDraw && rSh.IsDrawCreate() )
            {
                if ( rView.GetDrawFuncPtr() && rView.GetDrawFuncPtr()->MouseButtonUp(rMEvt) == sal_True )
                {
                    rView.GetDrawFuncPtr()->Deactivate();
                    rView.AttrChangedNotify( &rSh );
                    if ( rSh.IsObjSelected() )
                        rSh.EnterSelFrmMode();
                    if ( rView.GetDrawFuncPtr() && bInsFrm )
                        StopInsFrm();
                }
                bCallBase = sal_False;
                break;
            }
        case MOUSE_LEFT + KEY_MOD1:
        case MOUSE_LEFT + KEY_MOD2:
        case MOUSE_LEFT + KEY_SHIFT + KEY_MOD1:
            if ( bFrmDrag && rSh.IsSelFrmMode() )
            {
                if ( rMEvt.IsMod1() ) //Kopieren und nicht moven.
                {
                    //Drag abbrechen, statt dessen internes Copy verwenden
                    basegfx::B2DRange aActionRange(rSh.GetDrawView()->TakeActionRange());

                    if (!aActionRange.isEmpty())
                    {
                        rSh.BreakDrag();
                        basegfx::B2DPoint aEndPt, aSttPt;

                        if ( rSh.GetSelFrmType() & FRMTYPE_FLY_ATCNT )
                        {
                            aEndPt = aActionRange.getMinimum();
                            aSttPt = rSh.GetDrawView()->getMarkedObjectSnapRange().getMinimum();
                        }
                        else
                        {
                            aEndPt = aActionRange.getCenter();
                            aSttPt = rSh.GetDrawView()->getMarkedObjectSnapRange().getCenter();
                        }

                        if(!aSttPt.equal(aEndPt))
                        {
                            rSh.StartUndo( UNDO_UI_DRAG_AND_COPY );
                            const Point aOldA(basegfx::fround(aSttPt.getX()), basegfx::fround(aSttPt.getY()));
                            const Point aOldB(basegfx::fround(aEndPt.getX()), basegfx::fround(aEndPt.getY()));
                            rSh.Copy(&rSh, aOldA, aOldB, sal_False);
                            rSh.EndUndo( UNDO_UI_DRAG_AND_COPY );
                        }
                    }
                    else
                        (rSh.*rSh.fnEndDrag)( &aDocPt,sal_False );
                }
                else
                {
                    {
                        const SwFrmFmt* pFlyFmt;
                        const SvxMacro* pMacro;

                        sal_uInt16 nEvent = HDL_MOVE == eOldSdrMoveHdl
                                            ? SW_EVENT_FRM_MOVE
                                            : SW_EVENT_FRM_RESIZE;

                        if( 0 != ( pFlyFmt = rSh.GetFlyFrmFmt() ) &&
                            0 != ( pMacro = pFlyFmt->GetMacro().GetMacroTable().
                            Get( nEvent )) )
                        {
                            const Point aSttPt( PixelToLogic( aStartPos ) );
                            aRszMvHdlPt = aDocPt;
                            sal_uInt16 nPos = 0;
                            SbxArrayRef xArgs = new SbxArray;
                            SbxVariableRef xVar = new SbxVariable;
                            xVar->PutString( pFlyFmt->GetName() );
                            xArgs->Put( &xVar, ++nPos );

                            if( SW_EVENT_FRM_RESIZE == nEvent )
                            {
                                xVar = new SbxVariable;
                                xVar->PutUShort( static_cast< sal_uInt16 >(eOldSdrMoveHdl) );
                                xArgs->Put( &xVar, ++nPos );
                            }

                            xVar = new SbxVariable;
                            xVar->PutLong( aDocPt.X() - aSttPt.X() );
                            xArgs->Put( &xVar, ++nPos );
                            xVar = new SbxVariable;
                            xVar->PutLong( aDocPt.Y() - aSttPt.Y() );
                            xArgs->Put( &xVar, ++nPos );

                            xVar = new SbxVariable;
                            xVar->PutUShort( 1 );
                            xArgs->Put( &xVar, ++nPos );

                            ReleaseMouse();

                            rSh.ExecMacro( *pMacro, 0, &xArgs );

                            CaptureMouse();
                        }
                    }
                    (rSh.*rSh.fnEndDrag)( &aDocPt,sal_False );
                }
                bFrmDrag = sal_False;
                bCallBase = sal_False;
                break;
            }
            bPopMode = sal_True;
            // no break
        case MOUSE_LEFT + KEY_SHIFT:
            if (rSh.IsSelFrmMode())
            {

                (rSh.*rSh.fnEndDrag)( &aDocPt, sal_False );
                bFrmDrag = sal_False;
                bCallBase = sal_False;
                break;
            }

            if( bHoldSelection )
            {
                //JP 27.04.99: Bug 65389 - das EndDrag sollte auf jedenfall
                //              gerufen werden.
                bHoldSelection = sal_False;
                (rSh.*rSh.fnEndDrag)( &aDocPt, sal_False );
            }
            else
            {
                if ( !rSh.IsInSelect() && rSh.ChgCurrPam( aDocPt ) )
                {
                    const sal_Bool bTmpNoInterrupt = bNoInterrupt;
                    bNoInterrupt = sal_False;
                    {   // nur temp. Move-Kontext aufspannen, da sonst die
                        // Abfrage auf die Inhaltsform nicht funktioniert!!!
                        MV_KONTEXT( &rSh );
                        const Point aDocPos( PixelToLogic( aStartPos ) );
                        bValidCrsrPos = !(CRSR_POSCHG & (rSh.*rSh.fnSetCrsr)(&aDocPos,sal_False));
                    }
                    bNoInterrupt = bTmpNoInterrupt;

                }
                else
                {
                    sal_Bool bInSel = rSh.IsInSelect();
                    (rSh.*rSh.fnEndDrag)( &aDocPt, sal_False );

                    // Internetfield? --> Link-Callen (DocLaden!!)
//JP 18.10.96: Bug 32437 -
//                  if( !rSh.HasSelection() )
                    if( !bInSel )
                    {
                        sal_uInt16 nFilter = URLLOAD_NOFILTER;
                        if( KEY_MOD1 == rMEvt.GetModifier() )
                            nFilter |= URLLOAD_NEWVIEW;

                        sal_Bool bExecHyperlinks = rView.GetDocShell()->IsReadOnly();
                        if ( !bExecHyperlinks )
                        {
                            SvtSecurityOptions aSecOpts;
                            const sal_Bool bSecureOption = aSecOpts.IsOptionSet( SvtSecurityOptions::E_CTRLCLICK_HYPERLINK );
                            if ( (  bSecureOption && rMEvt.GetModifier() == KEY_MOD1 ) ||
                                 ( !bSecureOption && rMEvt.GetModifier() != KEY_MOD1 ) )
                                bExecHyperlinks = sal_True;
                        }

                        const sal_Bool bExecSmarttags = rMEvt.GetModifier() == KEY_MOD1;

                        if(pApplyTempl)
                            bExecHyperlinks = sal_False;

                        SwContentAtPos aCntntAtPos( SwContentAtPos::SW_CLICKFIELD |
                                                    SwContentAtPos::SW_INETATTR |
                                                    SwContentAtPos::SW_SMARTTAG  | SwContentAtPos::SW_FORMCTRL);

                        if( rSh.GetContentAtPos( aDocPt, aCntntAtPos, sal_True ) )
                        {
                            sal_Bool bViewLocked = rSh.IsViewLocked();
                            if( !bViewLocked && !rSh.IsReadOnlyAvailable() &&
                                aCntntAtPos.IsInProtectSect() )
                                rSh.LockView( sal_True );

                            ReleaseMouse();

                            if( SwContentAtPos::SW_FIELD == aCntntAtPos.eCntntAtPos )
                            {
                                if ( aCntntAtPos.pFndTxtAttr != NULL
                                     && aCntntAtPos.pFndTxtAttr->Which() == RES_TXTATR_INPUTFIELD )
                                {
                                    // select content of Input Field, but exclude CH_TXT_ATR_INPUTFIELDSTART
                                    // and CH_TXT_ATR_INPUTFIELDEND
                                    rSh.SttSelect();
                                    rSh.SelectTxt( *(aCntntAtPos.pFndTxtAttr->GetStart()) + 1,
                                                   *(aCntntAtPos.pFndTxtAttr->End()) - 1 );
                                }
                                else
                                {
                                    rSh.ClickToField( *aCntntAtPos.aFnd.pFld );
                                }
                            }
                            else if ( SwContentAtPos::SW_SMARTTAG == aCntntAtPos.eCntntAtPos )
                            {
                                    // execute smarttag menu
                                    if ( bExecSmarttags && SwSmartTagMgr::Get().IsSmartTagsEnabled() )
                                        rView.ExecSmartTagPopup( aDocPt );
                            }
                            else if ( SwContentAtPos::SW_FORMCTRL == aCntntAtPos.eCntntAtPos )
                            {
                                ASSERT( aCntntAtPos.aFnd.pFldmark != NULL, "where is my field ptr???");
                                if ( aCntntAtPos.aFnd.pFldmark != NULL)
                                {
                                    IFieldmark *fieldBM = const_cast< IFieldmark* > ( aCntntAtPos.aFnd.pFldmark );
                                    if (fieldBM->GetFieldname( ).equalsAscii( ODF_FORMCHECKBOX ) )
                                    {
                                        ICheckboxFieldmark* pCheckboxFm = dynamic_cast<ICheckboxFieldmark*>(fieldBM);
                                        pCheckboxFm->SetChecked(!pCheckboxFm->IsChecked());
                                        pCheckboxFm->Invalidate();
                                        rSh.InvalidateWindows( rView.GetVisArea() );
                                    } else if (fieldBM->GetFieldname().equalsAscii( ODF_FORMDROPDOWN) ) {
                                        rView.ExecFieldPopup( aDocPt, fieldBM );
                                        fieldBM->Invalidate();
                                        rSh.InvalidateWindows( rView.GetVisArea() );
                                    } else {
                                        // unknown type..
                                    }
                                }
                            }
                            else // if ( SwContentAtPos::SW_INETATTR == aCntntAtPos.eCntntAtPos )
                            {
                                if ( bExecHyperlinks )
                                    rSh.ClickToINetAttr( *(SwFmtINetFmt*)aCntntAtPos.aFnd.pAttr, nFilter );
                            }

                            rSh.LockView( bViewLocked );
                            bCallShadowCrsr = sal_False;
                        }
                        else
                        {
                            aCntntAtPos = SwContentAtPos( SwContentAtPos::SW_FTN );
                            if( !rSh.GetContentAtPos( aDocPt, aCntntAtPos, sal_True ) && bExecHyperlinks )
                            {
                                SdrViewEvent aVEvt;

                                if (pSdrView)
                                    pSdrView->PickAnything(rMEvt, SDRMOUSEBUTTONDOWN, aVEvt);

                                if (pSdrView && aVEvt.meEvent == SDREVENT_EXECUTEURL)
                                {
                                    // URL-Feld getroffen
                                    if(aVEvt.maURLField.Len())
                                    {
                                        const String sURL(aVEvt.maURLField);
                                        const String sTarget(aVEvt.maTargetFrame);
                                        ::LoadURL( sURL, &rSh, nFilter, &sTarget);
                                    }
                                    bCallShadowCrsr = sal_False;
                                }
                                else
                                {
                                    // Grafik getroffen
                                    ReleaseMouse();
                                    if( rSh.ClickToINetGrf( aDocPt, nFilter ))
                                        bCallShadowCrsr = sal_False;
                                }
                            }
                        }

                        if( bCallShadowCrsr &&
                            rSh.GetViewOptions()->IsShadowCursor() &&
                            MOUSE_LEFT == (rMEvt.GetModifier() + rMEvt.GetButtons()) &&
                            !rSh.HasSelection() &&
                            !GetConnectMetaFile() &&
                            rSh.VisArea().IsInside( aDocPt ))
                        {
                            SwUndoId nLastUndoId(UNDO_EMPTY);
                            if (rSh.GetLastUndoInfo(0, & nLastUndoId))
                            {
                                if (UNDO_INS_FROM_SHADOWCRSR == nLastUndoId)
                                {
                                    rSh.Undo();
                                }
                            }
                            SwFillMode eMode = (SwFillMode)rSh.GetViewOptions()->GetShdwCrsrFillMode();
                            rSh.SetShadowCrsrPos( aDocPt, eMode );
                        }
                    }
                }
                bCallBase = sal_False;

            }

            // gfs. im Down gepushten Mode wieder zuruecksetzen
            if ( bPopMode && bModePushed )
            {
                rSh.PopMode();
                bModePushed = sal_False;
                bCallBase = sal_False;
            }
            break;

        default:
            ReleaseMouse();
            return;
    }

    if( pApplyTempl )
    {
        int eSelection = rSh.GetSelectionType();
        SwFormatClipboard* pFormatClipboard = pApplyTempl->pFormatClipboard;
        if( pFormatClipboard )//apply format paintbrush
        {
            //get some parameters
            SwWrtShell& rWrtShell = rView.GetWrtShell();
            SfxStyleSheetBasePool* pPool=0;
            bool bNoCharacterFormats = false;
            bool bNoParagraphFormats = false;
            {
                SwDocShell* pDocSh = rView.GetDocShell();
                if(pDocSh)
                    pPool = pDocSh->GetStyleSheetPool();
                if( (rMEvt.GetModifier()&KEY_MOD1) && (rMEvt.GetModifier()&KEY_SHIFT) )
                    bNoCharacterFormats = true;
                else if( rMEvt.GetModifier() & KEY_MOD1 )
                    bNoParagraphFormats = true;
            }
            //execute paste
            pFormatClipboard->Paste( rWrtShell, pPool, bNoCharacterFormats, bNoParagraphFormats );

            //if the clipboard is empty after paste remove the ApplyTemplate
            if(!pFormatClipboard->HasContent())
                SetApplyTemplate(SwApplyTemplate());
        }
        else if( pApplyTempl->nColor )
        {
            sal_uInt16 nId = 0;
            switch( pApplyTempl->nColor )
            {
                case SID_ATTR_CHAR_COLOR_EXT:
                    nId = RES_CHRATR_COLOR;
                break;
                case SID_ATTR_CHAR_COLOR_BACKGROUND_EXT:
                    nId = RES_CHRATR_BACKGROUND;
                break;
            }
            if( nId && (nsSelectionType::SEL_TXT|nsSelectionType::SEL_TBL) & eSelection)
            {
                if( rSh.IsSelection() && !rSh.HasReadonlySel() )
                {
                    if(nId == RES_CHRATR_BACKGROUND)
                    {
                        Color aColor( COL_TRANSPARENT  );
                        if( !SwEditWin::bTransparentBackColor )
                            aColor = SwEditWin::aTextBackColor;
                        rSh.SetAttrItem( SvxBrushItem( aColor, nId ) );
                    }
                    else
                        rSh.SetAttrItem( SvxColorItem(SwEditWin::aTextColor, nId) );
                    rSh.UnSetVisCrsr();
                    rSh.EnterStdMode();
                    rSh.SetVisCrsr(aDocPt);

                    pApplyTempl->bUndo = sal_True;
                    bCallBase = sal_False;
                    aTemplateTimer.Stop();
                }
                else if(rMEvt.GetClicks() == 1)
                {
                    // keine Selektion -> also Giesskanne abschalten
                    aTemplateTimer.Start();
                }
            }
        }
        else
        {
            String aStyleName;
            switch ( pApplyTempl->eType )
            {
                case SFX_STYLE_FAMILY_PARA:
                    if( (( nsSelectionType::SEL_TXT | nsSelectionType::SEL_TBL )
                         & eSelection ) && !rSh.HasReadonlySel() )
                    {
                        rSh.SetTxtFmtColl( pApplyTempl->aColl.pTxtColl );
                        pApplyTempl->bUndo = sal_True;
                        bCallBase = sal_False;
                        if ( pApplyTempl->aColl.pTxtColl )
                            aStyleName = pApplyTempl->aColl.pTxtColl->GetName();
                    }
                    break;
                case SFX_STYLE_FAMILY_CHAR:
                    if( (( nsSelectionType::SEL_TXT | nsSelectionType::SEL_TBL )
                         & eSelection ) && !rSh.HasReadonlySel() )
                    {
                        rSh.SetAttrItem( SwFmtCharFmt(pApplyTempl->aColl.pCharFmt) );
                        rSh.UnSetVisCrsr();
                        rSh.EnterStdMode();
                        rSh.SetVisCrsr(aDocPt);
                        pApplyTempl->bUndo = sal_True;
                        bCallBase = sal_False;
                        if ( pApplyTempl->aColl.pCharFmt )
                            aStyleName = pApplyTempl->aColl.pCharFmt->GetName();
                    }
                    break;
                case SFX_STYLE_FAMILY_FRAME :
                {
                    const SwFrmFmt* pFmt = rSh.GetFmtFromObj( aDocPt );
                    if(dynamic_cast< const SwFlyFrmFmt* >( pFmt))
                    {
                        rSh.SetFrmFmt( pApplyTempl->aColl.pFrmFmt, sal_False, &aDocPt );
                        pApplyTempl->bUndo = sal_True;
                        bCallBase = sal_False;
                        if( pApplyTempl->aColl.pFrmFmt )
                            aStyleName = pApplyTempl->aColl.pFrmFmt->GetName();
                    }
                    break;
                }
                case SFX_STYLE_FAMILY_PAGE:
                            // Kein Undo bei Seitenvorlagen
                    rSh.ChgCurPageDesc( *pApplyTempl->aColl.pPageDesc );
                    if ( pApplyTempl->aColl.pPageDesc )
                        aStyleName = pApplyTempl->aColl.pPageDesc->GetName();
                    bCallBase = sal_False;
                    break;
                case SFX_STYLE_FAMILY_PSEUDO:
                    if( !rSh.HasReadonlySel() )
                    {
                        rSh.SetCurNumRule( *pApplyTempl->aColl.pNumRule,
                                           false,
                                           pApplyTempl->aColl.pNumRule->GetDefaultListId() );
                        bCallBase = sal_False;
                        pApplyTempl->bUndo = sal_True;
                        if( pApplyTempl->aColl.pNumRule )
                            aStyleName = pApplyTempl->aColl.pNumRule->GetName();
                    }
                    break;
            }

            uno::Reference< frame::XDispatchRecorder > xRecorder =
                    rView.GetViewFrame()->GetBindings().GetRecorder();
            if ( aStyleName.Len() && xRecorder.is() )
            {
                SfxShell *pSfxShell = lcl_GetShellFromDispatcher( rView, typeid(SwTextShell) );
                if ( pSfxShell )
                {
                    SfxRequest aReq( rView.GetViewFrame(), SID_STYLE_APPLY );
                    aReq.AppendItem( SfxStringItem( SID_STYLE_APPLY, aStyleName ) );
                    aReq.AppendItem( SfxUInt16Item( SID_STYLE_FAMILY, (sal_uInt16) pApplyTempl->eType ) );
                    aReq.Done();
                }
            }
        }

    }
    ReleaseMouse();
    // Hier kommen nur verarbeitete MouseEvents an; nur bei diesen duerfen
    // die Modi zurueckgesetzt werden.
    bMBPressed = sal_False;

    //sicherheitshalber aufrufen, da jetzt das Selektieren bestimmt zu Ende ist.
    //Andernfalls koennte der Timeout des Timers Kummer machen.
    EnterArea();
    bNoInterrupt = sal_False;

    if (bCallBase)
        Window::MouseButtonUp(rMEvt);
}


/*--------------------------------------------------------------------
    Beschreibung:   Vorlage anwenden
 --------------------------------------------------------------------*/


void SwEditWin::SetApplyTemplate(const SwApplyTemplate &rTempl)
{
    static sal_Bool bIdle = sal_False;
    DELETEZ(pApplyTempl);
    SwWrtShell &rSh = rView.GetWrtShell();

    if(rTempl.pFormatClipboard)
    {
        pApplyTempl = new SwApplyTemplate( rTempl );
              SetPointer( POINTER_FILL );//@todo #i20119# maybe better a new brush pointer here in future
              rSh.NoEdit( sal_False );
              bIdle = rSh.GetViewOptions()->IsIdle();
              ((SwViewOption *)rSh.GetViewOptions())->SetIdle( sal_False );
    }
    else if(rTempl.nColor)
    {
        pApplyTempl = new SwApplyTemplate( rTempl );
        SetPointer( POINTER_FILL );
        rSh.NoEdit( sal_False );
        bIdle = rSh.GetViewOptions()->IsIdle();
        ((SwViewOption *)rSh.GetViewOptions())->SetIdle( sal_False );
    }
    else if( rTempl.eType )
    {
        pApplyTempl = new SwApplyTemplate( rTempl );
        SetPointer( POINTER_FILL  );
        rSh.NoEdit( sal_False );
        bIdle = rSh.GetViewOptions()->IsIdle();
        ((SwViewOption *)rSh.GetViewOptions())->SetIdle( sal_False );
    }
    else
    {
        SetPointer( POINTER_TEXT );
        rSh.UnSetVisCrsr();

        ((SwViewOption *)rSh.GetViewOptions())->SetIdle( bIdle );
        if ( !rSh.IsSelFrmMode() )
            rSh.Edit();
    }

    static sal_uInt16 __READONLY_DATA aInva[] =
    {
        SID_STYLE_WATERCAN,
        SID_ATTR_CHAR_COLOR_EXT,
        SID_ATTR_CHAR_COLOR_BACKGROUND_EXT,
        0
    };
    rView.GetViewFrame()->GetBindings().Invalidate(aInva);
}

/*--------------------------------------------------------------------
    Beschreibung:   ctor
 --------------------------------------------------------------------*/


SwEditWin::SwEditWin(Window *pParent, SwView &rMyView):
    Window(pParent, WinBits(WB_CLIPCHILDREN | WB_DIALOGCONTROL)),
    DropTargetHelper( this ),
    DragSourceHelper( this ),

    eBufferLanguage(LANGUAGE_DONTKNOW),
    pApplyTempl(0),
    pAnchorMarker( 0 ),
    pUserMarker( 0 ),
    pUserMarkerObj( 0 ),
    pShadCrsr( 0 ),
    pRowColumnSelectionStart( 0 ),

    rView( rMyView ),

    aActHitType(SDRHIT_NONE),
    m_nDropFormat( 0 ),
    m_nDropAction( 0 ),
    m_nDropDestination( 0 ),

    nInsFrmColCount( 1 ),
    meSdrObjectCreationInfo(),

    bLockInput(sal_False),
    bObjectSelect( sal_False ),
    nKS_NUMDOWN_Count(0),
    nKS_NUMINDENTINC_Count(0)
{
    SetHelpId(HID_EDIT_WIN);
    EnableChildTransparentMode();
    SetDialogControlFlags( WINDOW_DLGCTRL_RETURN | WINDOW_DLGCTRL_WANTFOCUS );

    bLinkRemoved = bMBPressed = bInsDraw = bInsFrm =
    bIsInDrag = bOldIdle = bOldIdleSet = bChainMode = bWasShdwCrsr = sal_False;
    //#i42732# initially use the input language
    bUseInputLanguage = sal_True;

    SetMapMode(MapMode(MAP_TWIP));

    SetPointer( POINTER_TEXT );
    aTimer.SetTimeoutHdl(LINK(this, SwEditWin, TimerHandler));

    bTblInsDelMode = sal_False;
    aKeyInputTimer.SetTimeout( 3000 );
    aKeyInputTimer.SetTimeoutHdl(LINK(this, SwEditWin, KeyInputTimerHandler));

    aKeyInputFlushTimer.SetTimeout( 200 );
    aKeyInputFlushTimer.SetTimeoutHdl(LINK(this, SwEditWin, KeyInputFlushHandler));

    // TemplatePointer fuer Farben soll nach Einfachclick
    // ohne Selektion zurueckgesetzt werden
    aTemplateTimer.SetTimeout(400);
    aTemplateTimer.SetTimeoutHdl(LINK(this, SwEditWin, TemplateTimerHdl));

    //JP 16.12.98: temporaere Loesung!!! Sollte bei jeder Cursorbewegung
    //          den Font von der akt. einfuege Position setzen!
    if( !rMyView.GetDocShell()->IsReadOnly() )
    {
        Font aFont;
        SetInputContext( InputContext( aFont, INPUTCONTEXT_TEXT |
                                            INPUTCONTEXT_EXTTEXTINPUT ) );
    }
}



SwEditWin::~SwEditWin()
{
    aKeyInputTimer.Stop();
    delete pShadCrsr;
    delete pRowColumnSelectionStart;
    if( pQuickHlpData->bClear && rView.GetWrtShellPtr() )
        pQuickHlpData->Stop( rView.GetWrtShell() );
    bExecuteDrag = sal_False;
    delete pApplyTempl;
    rView.SetDrawFuncPtr(NULL);

    if(pUserMarker)
    {
        delete pUserMarker;
    }

    delete pAnchorMarker;
}


/******************************************************************************
 *  Beschreibung: DrawTextEditMode einschalten
 ******************************************************************************/


void SwEditWin::EnterDrawTextMode( const basegfx::B2DPoint& aDocPos )
{
    if(rView.EnterDrawTextMode(aDocPos))
    {
        if (rView.GetDrawFuncPtr())
        {
            rView.GetDrawFuncPtr()->Deactivate();
            rView.SetDrawFuncPtr(0);
            rView.LeaveDrawCreate();
        }

        rView.NoRotate();
        rView.AttrChangedNotify( &rView.GetWrtShell() );
    }
}

/******************************************************************************
 *  Beschreibung: DrawMode einschalten
 ******************************************************************************/



sal_Bool SwEditWin::EnterDrawMode(const MouseEvent& rMEvt, const Point& aDocPos)
{
    SwWrtShell &rSh = rView.GetWrtShell();
    SdrView *pSdrView = rSh.GetDrawView();

    if ( rView.GetDrawFuncPtr() )
    {
        if (rSh.IsDrawCreate())
            return sal_True;

        sal_Bool bRet = rView.GetDrawFuncPtr()->MouseButtonDown( rMEvt );
        rView.AttrChangedNotify( &rSh );
        return bRet;
    }

    if ( pSdrView && pSdrView->IsTextEdit() )
    {
        sal_Bool bUnLockView = !rSh.IsViewLocked();
        rSh.LockView( sal_True );

        rSh.EndTextEdit(); // Danebengeklickt, Ende mit Edit
        rSh.SelectObj( aDocPos );
        if ( !rSh.IsObjSelected() && !rSh.IsFrmSelected() )
            rSh.LeaveSelFrmMode();
        else
        {
            SwEditWin::nDDStartPosY = aDocPos.Y();
            SwEditWin::nDDStartPosX = aDocPos.X();
            bFrmDrag = sal_True;
        }
        if( bUnLockView )
            rSh.LockView( sal_False );
        rView.AttrChangedNotify( &rSh );
        return sal_True;
    }
    return sal_False;
}

/******************************************************************************
 *  Beschreibung:
 ******************************************************************************/



sal_Bool SwEditWin::IsDrawSelMode()
{
    return IsObjectSelect();
}

/******************************************************************************
 *  Beschreibung:
 ******************************************************************************/


void SwEditWin::GetFocus()
{
    if ( rView.GetPostItMgr()->HasActiveSidebarWin() )
    {
        rView.GetPostItMgr()->GrabFocusOnActiveSidebarWin();
    }
    else
    {
        rView.GotFocus();
        Window::GetFocus();
        rView.GetWrtShell().InvalidateAccessibleFocus();
    }
}

/******************************************************************************
 *  Beschreibung:
 ******************************************************************************/



void SwEditWin::LoseFocus()
{
    rView.GetWrtShell().InvalidateAccessibleFocus();
    Window::LoseFocus();
    if( pQuickHlpData->bClear )
        pQuickHlpData->Stop( rView.GetWrtShell() );
    rView.LostFocus();
}

/******************************************************************************
 *  Beschreibung:
 ******************************************************************************/



void SwEditWin::Command( const CommandEvent& rCEvt )
{
    SwWrtShell &rSh = rView.GetWrtShell();

    if ( !rView.GetViewFrame() )
    {
        //Wenn der ViewFrame in Kuerze stirbt kein Popup mehr!
        Window::Command(rCEvt);
        return;
    }

    // The command event is send to the window after a possible context
    // menu from an inplace client has been closed. Now we have the chance
    // to deactivate the inplace client without any problem regarding parent
    // windows and code on the stack.
    // For more information, see #126086# and #128122#
    SfxInPlaceClient* pIPClient = rSh.GetSfxViewShell()->GetIPClient();
    sal_Bool bIsOleActive = ( pIPClient && pIPClient->IsObjectInPlaceActive() );
    if ( bIsOleActive && ( rCEvt.GetCommand() == COMMAND_CONTEXTMENU ))
    {
        rSh.FinishOLEObj();
        return;
    }

    sal_Bool bCallBase      = sal_True;

    switch ( rCEvt.GetCommand() )
    {
        case COMMAND_CONTEXTMENU:
        {
            const sal_uInt16 nId = SwInputChild::GetChildWindowId();
            SwInputChild* pChildWin = (SwInputChild*)GetView().GetViewFrame()->
                                                GetChildWindow( nId );

            if (rView.GetPostItMgr()->IsHit(rCEvt.GetMousePosPixel()))
                return;

            if((!pChildWin || pChildWin->GetView() != &rView) &&
                !rSh.IsDrawCreate() && !IsDrawAction())
            {
                SET_CURR_SHELL( &rSh );
                if (!pApplyTempl)
                {
                    if (bNoInterrupt == sal_True)
                    {
                        ReleaseMouse();
                        bNoInterrupt = sal_False;
                        bMBPressed = sal_False;
                    }
                    Point aDocPos( PixelToLogic( rCEvt.GetMousePosPixel() ) );
                    if ( !rCEvt.IsMouseEvent() )
                        aDocPos = rSh.GetCharRect().Center();
                    else
                    {
                        SelectMenuPosition(rSh, rCEvt.GetMousePosPixel());
                        rView.StopShellTimer();

                    }
                    const Point aPixPos = LogicToPixel( aDocPos );

                    if ( rView.GetDocShell()->IsReadOnly() )
                    {
                        SwReadOnlyPopup* pROPopup = new SwReadOnlyPopup( aDocPos, rView );

                        ui::ContextMenuExecuteEvent aEvent;
                        aEvent.SourceWindow = VCLUnoHelper::GetInterface( this );
                        aEvent.ExecutePosition.X = aPixPos.X();
                        aEvent.ExecutePosition.Y = aPixPos.Y();
                        Menu* pMenu = 0;
                        ::rtl::OUString sMenuName =
                            ::rtl::OUString::createFromAscii( "private:resource/ReadonlyContextMenu");
                        if( GetView().TryContextMenuInterception( *pROPopup, sMenuName, pMenu, aEvent ) )
                        {
                            if ( pMenu )
                            {
                                sal_uInt16 nExecId = ((PopupMenu*)pMenu)->Execute(this, aPixPos);
                                if( !::ExecuteMenuCommand( *static_cast<PopupMenu*>(pMenu), *rView.GetViewFrame(), nExecId ))
                                    pROPopup->Execute(this, nExecId);
                            }
                            else
                                pROPopup->Execute(this, aPixPos);
                        }
                        delete pROPopup;
                    }
                    else if ( !rView.ExecSpellPopup( aDocPos ) )
                        GetView().GetViewFrame()->GetDispatcher()->ExecutePopup( 0, this, &aPixPos);
                }
                else if (pApplyTempl->bUndo)
                    rSh.Do(SwWrtShell::UNDO);
                bCallBase = sal_False;
            }
        }
        break;

        case COMMAND_WHEEL:
        case COMMAND_STARTAUTOSCROLL:
        case COMMAND_AUTOSCROLL:
            if( pShadCrsr )
                delete pShadCrsr, pShadCrsr = 0;
            bCallBase = !rView.HandleWheelCommands( rCEvt );
            break;

        case COMMAND_VOICE:
            {
                //ggf. an Outliner weiterleiten
                if ( rSh.HasDrawView() && rSh.GetDrawView()->IsTextEdit() )
                {
                    bCallBase = sal_False;
                    rSh.GetDrawView()->GetTextEditOutlinerView()->Command( rCEvt );
                    break;
                }

                const CommandVoiceData *pCData = rCEvt.GetVoiceData();
                if ( VOICECOMMANDTYPE_CONTROL == pCData->GetType() )
                    break;


                sal_uInt16 nSlotId = 0;
                SfxPoolItem *pItem = 0;

                switch ( pCData->GetCommand() )
                {
                    case DICTATIONCOMMAND_NEWPARAGRAPH: nSlotId = FN_INSERT_BREAK; break;
                    case DICTATIONCOMMAND_NEWLINE:      nSlotId = FN_INSERT_LINEBREAK; break;
                    case DICTATIONCOMMAND_LEFT:         nSlotId = FN_PREV_WORD; break;
                    case DICTATIONCOMMAND_RIGHT:        nSlotId = FN_NEXT_WORD; break;
                    case DICTATIONCOMMAND_UP:           nSlotId = FN_LINE_UP; break;
                    case DICTATIONCOMMAND_DOWN:         nSlotId = FN_LINE_DOWN; break;
                    case DICTATIONCOMMAND_UNDO:         nSlotId = SID_UNDO; break;
                    case DICTATIONCOMMAND_REPEAT:       nSlotId = SID_REPEAT; break;
                    case DICTATIONCOMMAND_DEL:          nSlotId = FN_DELETE_BACK_WORD; break;

                    case DICTATIONCOMMAND_BOLD_ON:      nSlotId = SID_ATTR_CHAR_WEIGHT;
                                                        pItem = new SvxWeightItem( WEIGHT_BOLD, RES_CHRATR_WEIGHT );
                                                        break;
                    case DICTATIONCOMMAND_BOLD_OFF:     nSlotId = SID_ATTR_CHAR_WEIGHT;
                                                        pItem = new SvxWeightItem( WEIGHT_NORMAL, RES_CHRATR_WEIGHT );
                                                        break;
                    case DICTATIONCOMMAND_UNDERLINE_ON: nSlotId = SID_ATTR_CHAR_UNDERLINE;
                                                        pItem = new SvxUnderlineItem( UNDERLINE_SINGLE, RES_CHRATR_WEIGHT );
                                                        break;
                    case DICTATIONCOMMAND_UNDERLINE_OFF:nSlotId = SID_ATTR_CHAR_UNDERLINE;
                                                        pItem = new SvxUnderlineItem( UNDERLINE_NONE, RES_CHRATR_UNDERLINE );
                                                        break;
                    case DICTATIONCOMMAND_ITALIC_ON:    nSlotId = SID_ATTR_CHAR_POSTURE;
                                                        pItem = new SvxPostureItem( ITALIC_NORMAL, RES_CHRATR_POSTURE );
                                                        break;
                    case DICTATIONCOMMAND_ITALIC_OFF:   nSlotId = SID_ATTR_CHAR_POSTURE;
                                                        pItem = new SvxPostureItem( ITALIC_NONE, RES_CHRATR_POSTURE );
                                                        break;
                    case DICTATIONCOMMAND_NUMBERING_ON:
                                    if ( !rSh.GetCurNumRule() )
                                        nSlotId = FN_NUM_NUMBERING_ON;
                                    break;
                    case DICTATIONCOMMAND_NUMBERING_OFF:
                                    if ( rSh.GetCurNumRule() )
                                       nSlotId = FN_NUM_NUMBERING_ON;
                                    break;
                    case DICTATIONCOMMAND_TAB:
                                    {
                                       rSh.Insert( '\t' );
                                    }
                                    break;
                    case DICTATIONCOMMAND_UNKNOWN:
                                    {
                                        rView.GetWrtShell().Insert( pCData->GetText() );
                                    }
                                    break;

#ifdef DBG_UTIL
                    default:
                        ASSERT( !this, "unknown speech command." );
#endif
                }
                if ( nSlotId )
                {
                    bCallBase = sal_False;
                    if ( pItem )
                    {
                        const SfxPoolItem* aArgs[2];
                        aArgs[0] = pItem;
                        aArgs[1] = 0;
                        GetView().GetViewFrame()->GetBindings().Execute(
                                    nSlotId, aArgs, 0, SFX_CALLMODE_STANDARD );
                        delete pItem;
                    }
                    else
                        GetView().GetViewFrame()->GetBindings().Execute( nSlotId );
                }
            }
            break;

    case COMMAND_STARTEXTTEXTINPUT:
    {
        sal_Bool bIsDocReadOnly = rView.GetDocShell()->IsReadOnly() &&
                              rSh.IsCrsrReadonly();
        if(!bIsDocReadOnly)
        {
            if( rSh.HasDrawView() && rSh.GetDrawView()->IsTextEdit() )
            {
                bCallBase = sal_False;
                rSh.GetDrawView()->GetTextEditOutlinerView()->Command( rCEvt );
            }
            else
            {
                if( rSh.HasSelection() )
                    rSh.DelRight();

                bCallBase = sal_False;
                LanguageType eInputLanguage = GetInputLanguage();
                rSh.CreateExtTextInput(eInputLanguage);
            }
        }
        break;
    }
    case COMMAND_ENDEXTTEXTINPUT:
    {
        sal_Bool bIsDocReadOnly = rView.GetDocShell()->IsReadOnly() &&
                              rSh.IsCrsrReadonly();
        if(!bIsDocReadOnly)
        {
            if( rSh.HasDrawView() && rSh.GetDrawView()->IsTextEdit() )
            {
                bCallBase = sal_False;
                rSh.GetDrawView()->GetTextEditOutlinerView()->Command( rCEvt );
            }
            else
            {
                bCallBase = sal_False;
                String sRecord = rSh.DeleteExtTextInput();
                uno::Reference< frame::XDispatchRecorder > xRecorder =
                        rView.GetViewFrame()->GetBindings().GetRecorder();

                if ( sRecord.Len() )
                {
                    // #102812# convert quotes in IME text
                    // works on the last input character, this is escpecially in Korean text often done
                    // quotes that are inside of the string are not replaced!
                    const sal_Unicode aCh = sRecord.GetChar(sRecord.Len() - 1);
                    SvxAutoCorrCfg* pACfg = SvxAutoCorrCfg::Get();
                    SvxAutoCorrect* pACorr = pACfg->GetAutoCorrect();
                    if(pACorr &&
                        (( pACorr->IsAutoCorrFlag( ChgQuotes ) && ('\"' == aCh ))||
                        ( pACorr->IsAutoCorrFlag( ChgSglQuotes ) && ( '\'' == aCh))))
                    {
                        rSh.DelLeft();
                        rSh.AutoCorrect( *pACorr, aCh );
                    }

                    if ( xRecorder.is() )
                    {
                        //Shell ermitteln
                        SfxShell *pSfxShell = lcl_GetShellFromDispatcher( rView, typeid(SwTextShell) );
                        // Request generieren und recorden
                        if (pSfxShell)
                        {
                            SfxRequest aReq( rView.GetViewFrame(), FN_INSERT_STRING );
                            aReq.AppendItem( SfxStringItem( FN_INSERT_STRING, sRecord ) );
                            aReq.Done();
                        }
                    }
                }
            }
        }
    }
    break;
    case COMMAND_EXTTEXTINPUT:
    {
        sal_Bool bIsDocReadOnly = rView.GetDocShell()->IsReadOnly() &&
                              rSh.IsCrsrReadonly();
        if(!bIsDocReadOnly)
        {
            QuickHelpData aTmpQHD;
            if( pQuickHlpData->bClear )
            {
                aTmpQHD.Move( *pQuickHlpData );
                pQuickHlpData->Stop( rSh );
            }
            String sWord;
            if( rSh.HasDrawView() && rSh.GetDrawView()->IsTextEdit() )
            {
                bCallBase = sal_False;
                rSh.GetDrawView()->GetTextEditOutlinerView()->Command( rCEvt );
            }
            else
            {
                const CommandExtTextInputData* pData = rCEvt.GetExtTextInputData();
                if( pData )
                {
                    sWord = pData->GetText();
                    bCallBase = sal_False;
                    rSh.SetExtTextInputData( *pData );
                }
            }
                uno::Reference< frame::XDispatchRecorder > xRecorder =
                        rView.GetViewFrame()->GetBindings().GetRecorder();
                if(!xRecorder.is())
                {
                    SvxAutoCorrCfg* pACfg = SvxAutoCorrCfg::Get();
                    SvxAutoCorrect* pACorr = pACfg->GetAutoCorrect();
                    if( pACfg && pACorr &&
                        ( pACfg->IsAutoTextTip() ||
                          pACorr->GetSwFlags().bAutoCompleteWords ) &&
                        rSh.GetPrevAutoCorrWord( *pACorr, sWord ) )
                    {
                        ShowAutoTextCorrectQuickHelp(sWord, pACfg, pACorr, sal_True);
                    }
                }
        }
    }
    break;
    case COMMAND_CURSORPOS:
        // will be handled by the base class
        break;

    case COMMAND_PASTESELECTION:
        if( !rView.GetDocShell()->IsReadOnly() )
        {
            TransferableDataHelper aDataHelper(
                        TransferableDataHelper::CreateFromSelection( this ));
            if( !aDataHelper.GetXTransferable().is() )
                break;

            sal_uLong nDropFormat;
            sal_uInt16 nEventAction, nDropAction, nDropDestination;
            nDropDestination = GetDropDestination( rCEvt.GetMousePosPixel() );
            if( !nDropDestination )
                break;

            nDropAction = SotExchange::GetExchangeAction(
                                aDataHelper.GetDataFlavorExVector(),
                                nDropDestination, EXCHG_IN_ACTION_COPY,
                                EXCHG_IN_ACTION_COPY, nDropFormat,
                                nEventAction );
            if( EXCHG_INOUT_ACTION_NONE != nDropAction )
            {
                const Point aDocPt( PixelToLogic( rCEvt.GetMousePosPixel() ) );
                SwTransferable::PasteData( aDataHelper, rSh, nDropAction,
                                    nDropFormat, nDropDestination, sal_False,
                                    sal_False, &aDocPt, EXCHG_IN_ACTION_COPY,
                                    sal_True );
            }
        }
        break;
        case COMMAND_MODKEYCHANGE :
        {
            const CommandModKeyData* pCommandData = (const CommandModKeyData*)rCEvt.GetData();
            if(pCommandData->IsMod1() && !pCommandData->IsMod2())
            {
                sal_uInt16 nSlot = 0;
                if(pCommandData->IsLeftShift() && !pCommandData->IsRightShift())
                    nSlot = SID_ATTR_PARA_LEFT_TO_RIGHT;
                else if(!pCommandData->IsLeftShift() && pCommandData->IsRightShift())
                    nSlot = SID_ATTR_PARA_RIGHT_TO_LEFT;
                if(nSlot && SW_MOD()->GetCTLOptions().IsCTLFontEnabled())
                    GetView().GetViewFrame()->GetDispatcher()->Execute(nSlot);
            }
        }
        break;
        case COMMAND_HANGUL_HANJA_CONVERSION :
            GetView().GetViewFrame()->GetDispatcher()->Execute(SID_HANGUL_HANJA_CONVERSION);
        break;
        case COMMAND_INPUTLANGUAGECHANGE :
            //#i42732# update state of fontname if input language changes
            bInputLanguageSwitched = true;
            SetUseInputLanguage( sal_True );
        break;
        case COMMAND_SELECTIONCHANGE:
        {
            const CommandSelectionChangeData *pData = rCEvt.GetSelectionChangeData();
            rSh.SttCrsrMove();
            rSh.GoStartSentence();
            rSh.GetCrsr()->GetPoint()->nContent += sal::static_int_cast<sal_uInt16, sal_uLong>(pData->GetStart());
            rSh.SetMark();
            rSh.GetCrsr()->GetMark()->nContent += sal::static_int_cast<sal_uInt16, sal_uLong>(pData->GetEnd() - pData->GetStart());
            rSh.EndCrsrMove( sal_True );
        }
        break;
        case COMMAND_PREPARERECONVERSION:
        if( rSh.HasSelection() )
        {
            SwPaM *pCrsr = (SwPaM*)rSh.GetCrsr();

            if( rSh.IsMultiSelection() )
            {
                if( pCrsr && !pCrsr->HasMark() &&
                pCrsr->GetPoint() == pCrsr->GetMark() )
                {
                rSh.GoPrevCrsr();
                pCrsr = (SwPaM*)rSh.GetCrsr();
                }

                // Cancel all selections other than the last selected one.
                while( rSh.GetCrsr()->GetNext() != rSh.GetCrsr() )
                delete rSh.GetCrsr()->GetNext();
            }

            if( pCrsr )
            {
                sal_uLong nPosNodeIdx = pCrsr->GetPoint()->nNode.GetIndex();
                xub_StrLen nPosIdx = pCrsr->GetPoint()->nContent.GetIndex();
                sal_uLong nMarkNodeIdx = pCrsr->GetMark()->nNode.GetIndex();
                xub_StrLen nMarkIdx = pCrsr->GetMark()->nContent.GetIndex();

                if( !rSh.GetCrsr()->HasMark() )
                rSh.GetCrsr()->SetMark();

                rSh.SttCrsrMove();

                if( nPosNodeIdx < nMarkNodeIdx )
                {
                rSh.GetCrsr()->GetPoint()->nNode = nPosNodeIdx;
                rSh.GetCrsr()->GetPoint()->nContent = nPosIdx;
                rSh.GetCrsr()->GetMark()->nNode = nPosNodeIdx;
                rSh.GetCrsr()->GetMark()->nContent =
                    rSh.GetCrsr()->GetCntntNode( sal_True )->Len();
                }
                else if( nPosNodeIdx == nMarkNodeIdx )
                {
                rSh.GetCrsr()->GetPoint()->nNode = nPosNodeIdx;
                rSh.GetCrsr()->GetPoint()->nContent = nPosIdx;
                rSh.GetCrsr()->GetMark()->nNode = nMarkNodeIdx;
                rSh.GetCrsr()->GetMark()->nContent = nMarkIdx;
                }
                else
                {
                rSh.GetCrsr()->GetMark()->nNode = nMarkNodeIdx;
                rSh.GetCrsr()->GetMark()->nContent = nMarkIdx;
                rSh.GetCrsr()->GetPoint()->nNode = nMarkNodeIdx;
                rSh.GetCrsr()->GetPoint()->nContent =
                    rSh.GetCrsr()->GetCntntNode( sal_False )->Len();
                }

                rSh.EndCrsrMove( sal_True );
            }
        }
        break;
#ifdef DBG_UTIL
        default:
            ASSERT( !this, "unknown command." );
#endif
    }
    if (bCallBase)
        Window::Command(rCEvt);
}

/* -----------------25.08.2003 10:12-----------------
    #i18686#: select the object/cursor at the mouse
    position of the context menu request
 --------------------------------------------------*/
sal_Bool SwEditWin::SelectMenuPosition(SwWrtShell& rSh, const Point& rMousePos )
{
    sal_Bool bRet = sal_False;
    const Point aDocPos( PixelToLogic( rMousePos ) );
    const basegfx::B2DPoint aB2DDocPos(aDocPos.X(), aDocPos.Y());
    const bool bIsInsideSelectedObj( rSh.IsInsideSelectedObj( aB2DDocPos ) );
    //create a synthetic mouse event out of the coordinates
    MouseEvent aMEvt(rMousePos);
    SdrView *pSdrView = rSh.GetDrawView();
    if ( pSdrView )
    {
        // no close of insert_draw and reset of draw mode,
        // if context menu position is inside a selected object.
        if ( !bIsInsideSelectedObj && rView.GetDrawFuncPtr() )
        {

            rView.GetDrawFuncPtr()->Deactivate();
            rView.SetDrawFuncPtr(NULL);
            rView.LeaveDrawCreate();
            SfxBindings& rBind = rView.GetViewFrame()->GetBindings();
            rBind.Invalidate( SID_ATTR_SIZE );
            rBind.Invalidate( SID_TABLE_CELL );
        }

        // if draw text is active and there's a text selection
        // at the mouse position then do nothing
        if(rSh.GetSelectionType() & nsSelectionType::SEL_DRW_TXT)
        {
            OutlinerView* pOLV = pSdrView->GetTextEditOutlinerView();
            ESelection aSelection = pOLV->GetSelection();
            if(!aSelection.IsZero())
            {
                SdrOutliner* pOutliner = pSdrView->GetTextEditOutliner();
                sal_Bool bVertical = pOutliner->IsVertical();
                const EditEngine& rEditEng = pOutliner->GetEditEngine();
                Point aEEPos(aDocPos);
                const Rectangle& rOutputArea = pOLV->GetOutputArea();
                // regard vertical mode
                if(bVertical)
                {
                    aEEPos -= rOutputArea.TopRight();
                    //invert the horizontal direction and exchange X and Y
                    long nTemp = -aEEPos.X();
                    aEEPos.X() = aEEPos.Y();
                    aEEPos.Y() = nTemp;
                }
                else
                    aEEPos -= rOutputArea.TopLeft();

                EPosition aDocPosition = rEditEng.FindDocPosition(aEEPos);
                ESelection aCompare(aDocPosition.nPara, aDocPosition.nIndex);
                // make it a forward selection - otherwise the IsLess/IsGreater do not work :-(
                aSelection.Adjust();
                if(!aCompare.IsLess(aSelection)  && !aCompare.IsGreater(aSelection))
                {
                    return sal_False;
                }
            }

        }

        if (pSdrView->MouseButtonDown( aMEvt, this ) )
        {
            pSdrView->MouseButtonUp( aMEvt, this );
            rSh.GetView().GetViewFrame()->GetBindings().InvalidateAll(sal_False);
            return sal_True;
        }
    }
    rSh.ResetCursorStack();

    if ( EnterDrawMode( aMEvt, aDocPos ) )
    {
        return sal_True;
    }
    if ( rView.GetDrawFuncPtr() && bInsFrm )
    {
        StopInsFrm();
        rSh.Edit();
    }

    UpdatePointer( aDocPos, 0 );

    if( !rSh.IsSelFrmMode() &&
        !GetView().GetViewFrame()->GetDispatcher()->IsLocked() )
    {
        // #107513#
        // Test if there is a draw object at that position and if it should be selected.
        sal_Bool bShould = rSh.ShouldObjectBeSelected(aDocPos);

        if(bShould)
        {
            rView.NoRotate();
            rSh.HideCrsr();

            sal_Bool bUnLockView = !rSh.IsViewLocked();
            rSh.LockView( sal_True );
            sal_Bool bSelObj = rSh.SelectObj( aDocPos, 0);
            if( bUnLockView )
                rSh.LockView( sal_False );

            if( bSelObj )
            {
                bRet = sal_True;
                // falls im Macro der Rahmen deselektiert
                // wurde, muss nur noch der Cursor
                // wieder angezeigt werden.
                if( FRMTYPE_NONE == rSh.GetSelFrmType() )
                    rSh.ShowCrsr();
                else
                {
                    if (rSh.IsFrmSelected() && rView.GetDrawFuncPtr())
                    {
                        rView.GetDrawFuncPtr()->Deactivate();
                        rView.SetDrawFuncPtr(NULL);
                        rView.LeaveDrawCreate();
                        rView.AttrChangedNotify( &rSh );
                    }

                    rSh.EnterSelFrmMode( &aDocPos );
                    bFrmDrag = sal_True;
                    UpdatePointer( aDocPos, 0 );
                    return bRet;
                }
            }

            if (!rView.GetDrawFuncPtr())
                rSh.ShowCrsr();
        }
    }
    else if ( rSh.IsSelFrmMode() &&
              (aActHitType == SDRHIT_NONE ||
               !bIsInsideSelectedObj))
    {
        rView.NoRotate();
        sal_Bool bUnLockView = !rSh.IsViewLocked();
        rSh.LockView( sal_True );
        sal_uInt8 nFlag = 0;

        if ( rSh.IsSelFrmMode() )
        {
            rSh.UnSelectFrm();
            rSh.LeaveSelFrmMode();
            rView.AttrChangedNotify(&rSh);
            bRet = sal_True;
        }

        sal_Bool bSelObj = rSh.SelectObj( aDocPos, nFlag );
        if( bUnLockView )
            rSh.LockView( sal_False );

        if( !bSelObj )
        {
            // Cursor hier umsetzen, damit er nicht zuerst
            // im Rahmen gezeichnet wird; ShowCrsr() geschieht
            // in LeaveSelFrmMode()
            bValidCrsrPos = !(CRSR_POSCHG & (rSh.*rSh.fnSetCrsr)(&aDocPos,sal_False));
            rSh.LeaveSelFrmMode();
            rView.LeaveDrawCreate();
            rView.AttrChangedNotify( &rSh );
            bRet = sal_True;
        }
        else
        {
            rSh.HideCrsr();
            rSh.EnterSelFrmMode( &aDocPos );
            rSh.SelFlyGrabCrsr();
            rSh.MakeSelVisible();
            bFrmDrag = sal_True;
            if( rSh.IsFrmSelected() &&
                rView.GetDrawFuncPtr() )
            {
                rView.GetDrawFuncPtr()->Deactivate();
                rView.SetDrawFuncPtr(NULL);
                rView.LeaveDrawCreate();
                rView.AttrChangedNotify( &rSh );
            }
            UpdatePointer( aDocPos, 0 );
            bRet = sal_True;
        }
    }
    else if ( rSh.IsSelFrmMode() && bIsInsideSelectedObj )
    {
        // ## object at the mouse cursor is already selected - do nothing
        return sal_False;
    }

    if ( rSh.IsGCAttr() )
    {
        rSh.GCAttr();
        rSh.ClearGCAttr();
    }

    sal_Bool bOverSelect = rSh.ChgCurrPam( aDocPos ), bOverURLGrf = sal_False;
    if( !bOverSelect )
        bOverURLGrf = bOverSelect = 0 != rSh.IsURLGrfAtPos( aDocPos );

    if ( !bOverSelect )
    {
        {   // nur temp. Move-Kontext aufspannen, da sonst die
            // Abfrage auf die Inhaltsform nicht funktioniert!!!
            MV_KONTEXT( &rSh );
            (rSh.*rSh.fnSetCrsr)(&aDocPos, sal_False);
            bRet = sal_True;
        }
    }
    if( !bOverURLGrf )
    {
        const int nSelType = rSh.GetSelectionType();
        if( nSelType == nsSelectionType::SEL_OLE ||
            nSelType == nsSelectionType::SEL_GRF )
        {
            MV_KONTEXT( &rSh );
            if( !rSh.IsFrmSelected() )
                rSh.GotoNextFly();
            rSh.EnterSelFrmMode();
            bRet = sal_True;
        }
    }
    return bRet;
}

SfxShell* lcl_GetShellFromDispatcher( SwView& rView, const std::type_info& rType )
{
    //Shell ermitteln
    SfxShell* pShell;
    SfxDispatcher* pDispatcher = rView.GetViewFrame()->GetDispatcher();
    for(sal_uInt16  i = 0; sal_True; ++i )
    {
        pShell = pDispatcher->GetShell( i );
        if( !pShell || typeid(*pShell) == rType )
            break;
    }
    return pShell;
}



void SwEditWin::ClearTip()
{
}

IMPL_LINK( SwEditWin, KeyInputFlushHandler, Timer *, EMPTYARG )
{
    FlushInBuffer();
    return 0;
}


IMPL_LINK( SwEditWin, KeyInputTimerHandler, Timer *, EMPTYARG )
{
    bTblInsDelMode = sal_False;
    return 0;
}

void SwEditWin::_InitStaticData()
{
    pQuickHlpData = new QuickHelpData();
}

void SwEditWin::_FinitStaticData()
{
    delete pQuickHlpData;
}
/* -----------------23.01.2003 12:15-----------------
 * #i3370# remove quick help to prevent saving
 * of autocorrection suggestions
 * --------------------------------------------------*/
void SwEditWin::StopQuickHelp()
{
    if( HasFocus() && pQuickHlpData && pQuickHlpData->bClear  )
        pQuickHlpData->Stop( rView.GetWrtShell() );
}

/*-----------------23.02.97 18:39-------------------

--------------------------------------------------*/


IMPL_LINK(SwEditWin, TemplateTimerHdl, Timer*, EMPTYARG)
{
    SetApplyTemplate(SwApplyTemplate());
    return 0;
}


void SwEditWin::SetChainMode( sal_Bool bOn )
{
    if ( !bChainMode )
        StopInsFrm();

    if ( pUserMarker )
    {
        delete pUserMarker;
        pUserMarker = 0L;
    }

    bChainMode = bOn;
    if ( !bChainMode )
        rView.GetViewFrame()->HideStatusText();

    static sal_uInt16 __READONLY_DATA aInva[] =
    {
        FN_FRAME_CHAIN, FN_FRAME_UNCHAIN, 0
    };
    rView.GetViewFrame()->GetBindings().Invalidate(aInva);
}

uno::Reference< ::com::sun::star::accessibility::XAccessible > SwEditWin::CreateAccessible()
{
    vos::OGuard aGuard(Application::GetSolarMutex());   // this should have
                                                        // happend already!!!
    SwWrtShell *pSh = rView.GetWrtShellPtr();
    ASSERT( pSh, "no writer shell, no accessible object" );
    uno::Reference<
        ::com::sun::star::accessibility::XAccessible > xAcc;
    if( pSh )
        xAcc = pSh->CreateAccessible();

    return xAcc;
}

//-------------------------------------------------------------

void QuickHelpData::Move( QuickHelpData& rCpy )
{
    // Pointer verschieben
    aArr.Insert( &rCpy.aArr );
    rCpy.aArr.Remove( (sal_uInt16)0, rCpy.aArr.Count() );
//  nTipId = rCpy.nTipId;
    bClear = rCpy.bClear;
    nLen = rCpy.nLen;
    nCurArrPos = rCpy.nCurArrPos;
    bChkInsBlank = rCpy.bChkInsBlank;
    bIsTip = rCpy.bIsTip;
    bIsAutoText = rCpy.bIsAutoText;

    if( pCETID ) delete pCETID;
    pCETID = rCpy.pCETID;
    rCpy.pCETID = 0;

    if( pAttrs )
        delete[] pAttrs;
    pAttrs = rCpy.pAttrs;
    rCpy.pAttrs = 0;
}

void QuickHelpData::ClearCntnt()
{
    nLen = nCurArrPos = 0;
    bClear = bChkInsBlank = sal_False;
    nTipId = 0;
    aArr.DeleteAndDestroy( 0 , aArr.Count() );
    bIsTip = sal_True;
    bIsAutoText = sal_True;
    delete pCETID, pCETID = 0;
    delete[] pAttrs, pAttrs = 0;
}


void QuickHelpData::Start( SwWrtShell& rSh, sal_uInt16 nWrdLen )
{
    if( pCETID ) delete pCETID, pCETID = 0;
    if( pAttrs ) delete[] pAttrs, pAttrs = 0;

    if( USHRT_MAX != nWrdLen )
    {
        nLen = nWrdLen;
        nCurArrPos = 0;
    }
    bClear = sal_True;

    Window& rWin = rSh.GetView().GetEditWin();
    if( bIsTip )
    {
        Point aPt( rWin.OutputToScreenPixel( rWin.LogicToPixel(
                    rSh.GetCharRect().Pos() )));
        aPt.Y() -= 3;
        nTipId = Help::ShowTip( &rWin, Rectangle( aPt, Size( 1, 1 )),
                        *aArr[ nCurArrPos ],
                        QUICKHELP_LEFT | QUICKHELP_BOTTOM );
    }
    else
    {
        String sStr( *aArr[ nCurArrPos ] );
        sStr.Erase( 0, nLen );
        sal_uInt16 nL = sStr.Len();
        pAttrs = new sal_uInt16[ nL ];
        for( sal_uInt16 n = nL; n;  )
            *(pAttrs + --n ) = EXTTEXTINPUT_ATTR_DOTTEDUNDERLINE |
                                EXTTEXTINPUT_ATTR_HIGHLIGHT;
        pCETID = new CommandExtTextInputData( sStr, pAttrs, nL,
                                                0, 0, 0, sal_False );
        rSh.CreateExtTextInput(rWin.GetInputLanguage());
        rSh.SetExtTextInputData( *pCETID );
    }
}

void QuickHelpData::Stop( SwWrtShell& rSh )
{
    if( !bIsTip )
        rSh.DeleteExtTextInput( 0, sal_False );
    else if( nTipId )
        Help::HideTip( nTipId );
    ClearCntnt();
}

void QuickHelpData::FillStrArr( SwWrtShell& rSh, const String& rWord )
{
    // #i22961# get the letter-case context
    const CharClass& rCharClass = GetAppCharClass();
    const String aLWord = rCharClass.toLower( rWord, 0, rWord.Len() );
    const bool bIsUpper = (rWord == rCharClass.toUpper( rWord, 0, rWord.Len() ) );
    const bool bIsLower = (rWord == aLWord);

    // add all matching month/day names from the calendar
    salhelper::SingletonRef<SwCalendarWrapper>* pCalendar = s_getCalendarWrapper();
    (*pCalendar)->LoadDefaultCalendar( rSh.GetCurLang() );

    {
        uno::Sequence< i18n::CalendarItem > aNames(
                                            (*pCalendar)->getMonths() );
        for( int n = 0; n < 2; ++n )
        {
            for( long nPos = 0, nEnd = aNames.getLength(); nPos < nEnd; ++nPos )
            {
                const String& rS = aNames[ nPos ].FullName;
                if( rS.Len() <= rWord.Len() + 1 )
                    continue;
                const String aLName = rCharClass.toLower( rS, 0, rWord.Len() );
                if( aLName != aLWord )
                    continue;
                // #i22961# provide case-sensitive autocompletion suggestions
                String* pNew = new String( (bIsUpper==bIsLower) ? rS : (bIsUpper ? rCharClass.toUpper(rS,0,rS.Len()) : rCharClass.toLower(rS,0,rS.Len())) );
                if( !aArr.Insert( pNew ) )
                    delete pNew;
            }
            if( !n )                    // get data for the second loop
                aNames = (*pCalendar)->getDays();
        }
    }

    // and then add all words from the AutoCompleteWord-List
    const SwAutoCompleteWord& rACLst = rSh.GetAutoCompleteWords();
    sal_uInt16 nStt, nEnd;
    if( rACLst.GetRange( rWord, nStt, nEnd ) )
    {
        for(; nStt < nEnd; ++nStt )
        {
            const String& rS = rACLst[ nStt ];
            if( rS.Len() <= rWord.Len() )
                continue;
            // #i22961# provide case-sensitive autocompletion suggestions
            String* pNew = new String( (bIsUpper==bIsLower) ? rS : (bIsUpper ? rCharClass.toUpper(rS,0,rS.Len()) : rCharClass.toLower(rS,0,rS.Len())) );
            if( !aArr.Insert( pNew ) )
                delete pNew;
        }
    }
}

void SwEditWin::ShowAutoTextCorrectQuickHelp(
        const String& rWord, SvxAutoCorrCfg* pACfg, SvxAutoCorrect* pACorr,
        sal_Bool bFromIME )
{
    SwWrtShell& rSh = rView.GetWrtShell();
    pQuickHlpData->ClearCntnt();
    if( pACfg->IsAutoTextTip() )
    {
        SwGlossaryList* pList = ::GetGlossaryList();
        pList->HasLongName( rWord, &pQuickHlpData->aArr );
    }

    if( pQuickHlpData->aArr.Count() )
    {
        pQuickHlpData->bIsTip = sal_True;
        pQuickHlpData->bIsAutoText = sal_True;
    }
    else if( pACorr->GetSwFlags().bAutoCompleteWords )
    {
        pQuickHlpData->bIsAutoText = sal_False;
        pQuickHlpData->bIsTip = bFromIME ||
                    !pACorr ||
                    pACorr->GetSwFlags().bAutoCmpltShowAsTip;

        pQuickHlpData->FillStrArr( rSh, rWord );
    }

    if( pQuickHlpData->aArr.Count() )
        pQuickHlpData->Start( rSh, rWord.Len() );
}

/* -----------------29.03.2006 11:01-----------------
 *
 * --------------------------------------------------*/

void SwEditWin::SetUseInputLanguage( sal_Bool bNew )
{
    if ( bNew || bUseInputLanguage )
    {
        SfxBindings& rBind = GetView().GetViewFrame()->GetBindings();
        rBind.Invalidate( SID_ATTR_CHAR_FONT );
        rBind.Invalidate( SID_ATTR_CHAR_FONTHEIGHT );
    }
    bUseInputLanguage = bNew;
}

/*-- 13.11.2008 10:18:17---------------------------------------------------

  -----------------------------------------------------------------------*/
XubString SwEditWin::GetSurroundingText() const
{
    String sReturn;
    SwWrtShell& rSh = rView.GetWrtShell();
    if( rSh.HasSelection() && !rSh.IsMultiSelection() && rSh.IsSelOnePara() )
        rSh.GetSelectedText( sReturn, GETSELTXT_PARABRK_TO_ONLYCR  );
    else if( !rSh.HasSelection() )
    {
        SwPosition *pPos = rSh.GetCrsr()->GetPoint();
        xub_StrLen nPos = pPos->nContent.GetIndex();

        // get the sentence around the cursor
        rSh.HideCrsr();
        rSh.GoStartSentence();
        rSh.SetMark();
        rSh.GoEndSentence();
        rSh.GetSelectedText( sReturn, GETSELTXT_PARABRK_TO_ONLYCR  );

        pPos->nContent = nPos;
        rSh.ClearMark();
        rSh.HideCrsr();
    }

    return sReturn;
}
/*-- 13.11.2008 10:18:17---------------------------------------------------

  -----------------------------------------------------------------------*/
Selection SwEditWin::GetSurroundingTextSelection() const
{
    SwWrtShell& rSh = rView.GetWrtShell();
    if( rSh.HasSelection() )
    {
        String sReturn;
        rSh.GetSelectedText( sReturn, GETSELTXT_PARABRK_TO_ONLYCR  );
        return Selection( 0, sReturn.Len() );
    }
    else
    {
        // Return the position of the visible cursor in the sentence
        // around the visible cursor.
        SwPosition *pPos = rSh.GetCrsr()->GetPoint();
        xub_StrLen nPos = pPos->nContent.GetIndex();

        rSh.HideCrsr();
        rSh.GoStartSentence();
        xub_StrLen nStartPos = rSh.GetCrsr()->GetPoint()->nContent.GetIndex();

        pPos->nContent = nPos;
        rSh.ClearMark();
        rSh.ShowCrsr();

        return Selection( nPos - nStartPos, nPos - nStartPos );
    }
}
//IAccessibility2 Implementation 2009-----
// MT: Removed Windows::SwitchView() introduced with IA2 CWS.
// There are other notifications for this when the active view has chnaged, so please update the code to use that event mechanism
void SwEditWin::SwitchView()
{
#ifdef ACCESSIBLE_LAYOUT
    if (!Application::IsAccessibilityEnabled())
    {
        return ;
    }
    rView.GetWrtShell().InvalidateAccessibleFocus();
#endif
}
//-----IAccessibility2 Implementation 2009
