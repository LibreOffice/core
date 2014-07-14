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

#include <svx/fmmodel.hxx>
#include <svx/svdpage.hxx>
#include <tools/shl.hxx>
#include <swmodule.hxx>
#include <svtools/accessibilityoptions.hxx>
#include <svx/svdpagv.hxx>
#include <fmtanchr.hxx>
#include <frmfmt.hxx>
#include <svx/svdlegacy.hxx>
#include <svx/svdoutl.hxx>
#ifdef DBG_UTIL
#include <svx/fmglob.hxx>
#endif
#include "fesh.hxx"
#include "pagefrm.hxx"
#include "rootfrm.hxx"
#include "viewimp.hxx"
#include "dflyobj.hxx"
#include "viewopt.hxx"
#include "printdata.hxx"
#include "dcontact.hxx"
#include "dview.hxx"
#include "flyfrm.hxx"
#include <vcl/svapp.hxx>
#include <IDocumentDrawModelAccess.hxx>
#include <drawdoc.hxx>

/*************************************************************************
|*
|*  SwSaveHdl
|*
|*  Ersterstellung      MA 14. Feb. 95
|*  Letzte Aenderung    MA 02. Jun. 98
|*
|*************************************************************************/
//SwSaveHdl::SwSaveHdl( SwViewImp *pI ) :
//  pImp( pI ),
//  bXorVis( sal_False )
//{
    //if ( pImp->HasDrawView() )
    //{
    //  bXorVis = pImp->GetDrawView()->IsShownXorVisible( pImp->GetShell()->GetOut());
    //  if ( bXorVis )
    //      pImp->GetDrawView()->HideShownXor( pImp->GetShell()->GetOut() );
    //}
//}


//SwSaveHdl::~SwSaveHdl()
//{
    //if ( bXorVis )
    //  pImp->GetDrawView()->ShowShownXor( pImp->GetShell()->GetOut() );
//}


/*************************************************************************
|*
|*  SwViewImp::StartAction(), EndAction()
|*
|*  Ersterstellung      MA 14. Feb. 95
|*  Letzte Aenderung    MA 14. Sep. 98
|*
|*************************************************************************/


void SwViewImp::StartAction()
{
    if ( HasDrawView() )
    {
        SET_CURR_SHELL( GetShell() );
        if ( dynamic_cast< SwFEShell* >(pSh) )
            ((SwFEShell*)pSh)->HideChainMarker();   //Kann sich geaendert haben
    }
}



void SwViewImp::EndAction()
{
    if ( HasDrawView() )
    {
        SET_CURR_SHELL( GetShell() );
        if ( dynamic_cast< SwFEShell* >(pSh) )
            ((SwFEShell*)pSh)->SetChainMarker();    //Kann sich geaendert haben
    }
}

/*************************************************************************
|*
|*  SwViewImp::LockPaint(), UnlockPaint()
|*
|*  Ersterstellung      MA 11. Jun. 96
|*  Letzte Aenderung    MA 11. Jun. 96
|*
|*************************************************************************/


void SwViewImp::LockPaint()
{
}



void SwViewImp::UnlockPaint()
{
}


/*************************************************************************
|*
|*  SwViewImp::PaintLayer(), PaintDispatcher()
|*
|*  Ersterstellung      MA 20. Dec. 94
|*  Letzte Aenderung    AMA 04. Jun. 98
|*
|*************************************************************************/
// OD 29.08.2002 #102450#
// add 3rd parameter <const Color* pPageBackgrdColor> for setting this
// color as the background color at the outliner of the draw view.
// OD 09.12.2002 #103045# - add 4th parameter for the horizontal text direction
// of the page in order to set the default horizontal text direction at the
// outliner of the draw view for painting layers <hell> and <heaven>.
// OD 25.06.2003 #108784# - correct type of 1st parameter
void SwViewImp::PaintLayer( const SdrLayerID _nLayerID,
                            SwPrintData const*const pPrintData,
                            const SwRect& ,
                            const Color* _pPageBackgrdColor,
                            const bool _bIsPageRightToLeft,
                            sdr::contact::ViewObjectContactRedirector* pRedirector ) const
{
    if ( HasDrawView() )
    {
        //change the draw mode in high contrast mode
        OutputDevice* pOutDev = GetShell()->GetOut();
        sal_uLong nOldDrawMode = pOutDev->GetDrawMode();
        if( GetShell()->GetWin() &&
            Application::GetSettings().GetStyleSettings().GetHighContrastMode() &&
            (!GetShell()->IsPreView()||SW_MOD()->GetAccessibilityOptions().GetIsForPagePreviews()))
        {
            pOutDev->SetDrawMode( nOldDrawMode | DRAWMODE_SETTINGSLINE | DRAWMODE_SETTINGSFILL |
                                DRAWMODE_SETTINGSTEXT | DRAWMODE_SETTINGSGRADIENT );
        }

        // OD 29.08.2002 #102450#
        // For correct handling of accessibility, high contrast, the page background
        // color is set as the background color at the outliner of the draw view.
        // Only necessary for the layers hell and heaven
        Color aOldOutlinerBackgrdColor;
        // OD 09.12.2002 #103045# - set default horizontal text direction on
        // painting <hell> or <heaven>.
        EEHorizontalTextDirection aOldEEHoriTextDir = EE_HTEXTDIR_L2R;
        const IDocumentDrawModelAccess* pIDDMA = GetShell()->getIDocumentDrawModelAccess();
        if ( (_nLayerID == pIDDMA->GetHellId()) ||
             (_nLayerID == pIDDMA->GetHeavenId()) )
        {
            ASSERT( _pPageBackgrdColor,
                    "incorrect usage of SwViewImp::PaintLayer: pPageBackgrdColor have to be set for painting layer <hell> or <heaven>");
            if ( _pPageBackgrdColor )
            {
                aOldOutlinerBackgrdColor =
                        GetDrawView()->getSdrModelFromSdrView().GetDrawOutliner().GetBackgroundColor();
                GetDrawView()->getSdrModelFromSdrView().GetDrawOutliner().SetBackgroundColor( *_pPageBackgrdColor );
            }

            aOldEEHoriTextDir =
                GetDrawView()->getSdrModelFromSdrView().GetDrawOutliner().GetDefaultHorizontalTextDirection();
            EEHorizontalTextDirection aEEHoriTextDirOfPage =
                _bIsPageRightToLeft ? EE_HTEXTDIR_R2L : EE_HTEXTDIR_L2R;
            GetDrawView()->getSdrModelFromSdrView().GetDrawOutliner().SetDefaultHorizontalTextDirection( aEEHoriTextDirOfPage );
        }

        pOutDev->Push( PUSH_LINECOLOR ); // #114231#
        if (pPrintData)
        {
            // hide drawings but not form controls (form controls are handled elsewhere)
            SwDrawView* pSdrView = const_cast< SwDrawView* >(GetDrawView());
            pSdrView->setHideDraw( !pPrintData->IsPrintDraw() );
        }

        if(GetDrawView()->GetSdrPageView())
        {
            GetDrawView()->GetSdrPageView()->DrawLayer( _nLayerID, pOutDev, pRedirector );
        }
        else
        {
            OSL_ENSURE(false, "Missing SdrPageView, use ShowSdrPage() first (!)");
        }

        pOutDev->Pop();

        // OD 29.08.2002 #102450#
        // reset background color of the outliner
        // OD 09.12.2002 #103045# - reset default horizontal text direction
        if ( (_nLayerID == pIDDMA->GetHellId()) ||
             (_nLayerID == pIDDMA->GetHeavenId()) )
        {
            GetDrawView()->getSdrModelFromSdrView().GetDrawOutliner().SetBackgroundColor( aOldOutlinerBackgrdColor );
            GetDrawView()->getSdrModelFromSdrView().GetDrawOutliner().SetDefaultHorizontalTextDirection( aOldEEHoriTextDir );
        }

        pOutDev->SetDrawMode( nOldDrawMode );
    }
}

/*************************************************************************
|*
|*  SwViewImp::IsDragPossible()
|*
|*  Ersterstellung      MA 19. Jan. 93
|*  Letzte Aenderung    MA 16. Jan. 95
|*
|*************************************************************************/
#define WIEDUWILLST 400

sal_Bool SwViewImp::IsDragPossible( const Point &rPoint )
{
    if ( !HasDrawView() || !GetDrawView()->areSdrObjectsSelected() )
        return sal_False;

    const SdrObjectVector aSelection(GetDrawView()->getSelectedSdrObjectVectorFromSdrMarkView());
    SdrObject *pO = aSelection[aSelection.size() - 1];

    SwRect aRect;
    if( pO && ::CalcClipRect( pO, aRect, sal_False ) )
    {
        SwRect aTmp;
        ::CalcClipRect( pO, aTmp, sal_True );
        aRect.Union( aTmp );
    }
    else
        aRect = GetShell()->GetLayout()->Frm();

    aRect.Top(    aRect.Top()    - WIEDUWILLST );
    aRect.Bottom( aRect.Bottom() + WIEDUWILLST );
    aRect.Left(   aRect.Left()   - WIEDUWILLST );
    aRect.Right(  aRect.Right()  + WIEDUWILLST );

    return aRect.IsInside( rPoint );
}

/*************************************************************************
|*
|*  SwViewImp::NotifySizeChg()
|*
|*  Ersterstellung      MA 23. Jun. 93
|*  Letzte Aenderung    MA 05. Oct. 98
|*
|*************************************************************************/

void SwViewImp::NotifySizeChg( const Size &rNewSz )
{
    if ( !HasDrawView() )
        return;

    if ( GetDrawView() && GetDrawView()->GetSdrPageView() )
        GetDrawView()->GetSdrPageView()->getSdrPageFromSdrPageView().SetPageScale( basegfx::B2DVector(rNewSz.Width(), rNewSz.Height()) );

    //Begrenzung des Arbeitsbereiches.
    const basegfx::B2DRange aRange(
        DOCUMENTBORDER, DOCUMENTBORDER,
        DOCUMENTBORDER + rNewSz.getWidth(), DOCUMENTBORDER + rNewSz.getHeight());
    const basegfx::B2DRange& rOldWork = GetDrawView()->GetWorkArea();
    bool bCheckDrawObjs(false);

    if(!aRange.equal(rOldWork))
    {
        if ( rOldWork.getMaxY() > aRange.getMaxY() || rOldWork.getMaxX() > aRange.getMaxX())
            bCheckDrawObjs = true;
        GetDrawView()->SetWorkArea( aRange );
    }
    if ( !bCheckDrawObjs )
        return;

    ASSERT( pSh->getIDocumentDrawModelAccess()->GetDrawModel(), "NotifySizeChg without DrawModel" );
    SdrPage* pPage = pSh->getIDocumentDrawModelAccess()->GetDrawModel()->GetPage( 0 );
    const sal_uLong nObjs = pPage->GetObjCount();
    for( sal_uLong nObj = 0; nObj < nObjs; ++nObj )
    {
        SdrObject *pObj = pPage->GetObj( nObj );
        if( !dynamic_cast< SwVirtFlyDrawObj* >(pObj) )
        {
            //Teilfix(26793): Objekte, die in Rahmen verankert sind, brauchen
            //nicht angepasst werden.
            const SwContact *pCont = (SwContact*)findConnectionToSdrObject(pObj);
            //JP - 16.3.00 Bug 73920: this function might be called by the
            //              InsertDocument, when a PageDesc-Attribute is
            //              set on a node. Then the SdrObject must not have
            //              an UserCall.
            if( !pCont || !dynamic_cast< const SwDrawContact* >(pCont) )
                continue;

            const SwFrm *pAnchor = ((SwDrawContact*)pCont)->GetAnchorFrm();
            if ( !pAnchor || pAnchor->IsInFly() || !pAnchor->IsValid() ||
                 !pAnchor->GetUpper() || !pAnchor->FindPageFrm() ||
                 (FLY_AS_CHAR == pCont->GetFmt()->GetAnchor().GetAnchorId()) )
            {
                continue;
            }

            // OD 19.06.2003 #108784# - no move for drawing objects in header/footer
            if ( pAnchor->FindFooterOrHeader() )
            {
                continue;
            }

            const basegfx::B2DRange& rBound(pObj->getObjectRange(GetDrawView()));

            if ( !aRange.isInside( rBound ) )
            {
                basegfx::B2DPoint aMove(0.0, 0.0);

                if(rBound.getMinX() > aRange.getMaxX())
                {
                    aMove.setX((aRange.getMaxX() - rBound.getMinX()) - MINFLY);
                }
                if(rBound.getMinY() > aRange.getMaxY())
                {
                    aMove.setY((aRange.getMaxY() - rBound.getMinY()) - MINFLY);
                }
                if(!aMove.equalZero())
            {
                    sdr::legacy::transformSdrObject(*pObj, basegfx::tools::createTranslateB2DHomMatrix(aMove));
                }

                //Notanker: Grosse Objekte nicht nach oben verschwinden lassen.
                aMove = basegfx::B2DPoint(0.0, 0.0);

                if(rBound.getMaxY() < aRange.getMinY())
                {
                    aMove.setX((rBound.getMaxY() - aRange.getMinY()) - MINFLY);
                }
                if(rBound.getMaxX() < aRange.getMinX())
                {
                    aMove.setY((rBound.getMaxX() - aRange.getMinX()) - MINFLY);
                }
                if(!aMove.equalZero())
                {
                    sdr::legacy::transformSdrObject(*pObj, basegfx::tools::createTranslateB2DHomMatrix(aMove));
                }
            }
        }
    }
}



