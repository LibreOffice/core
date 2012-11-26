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
#include "precompiled_svx.hxx"

#include <svx/svdoedge.hxx>
#include <svx/svdattrx.hxx>
#include <svx/svdview.hxx>
#include <svx/svdpage.hxx> // SdrObjList
#include <svx/connctrl.hxx>
#include <svx/dialmgr.hxx>
#include <svx/dlgutil.hxx>

#include <svx/sdr/contact/objectcontactofobjlistpainter.hxx>
#include <svx/sdr/contact/displayinfo.hxx>
#include <vcl/svapp.hxx>
#include <svx/svdlegacy.hxx>

/*************************************************************************
|*
|* Ctor SvxXConnectionPreview
|*
*************************************************************************/

SvxXConnectionPreview::SvxXConnectionPreview( Window* pParent, const ResId& rResId,
                            const SfxItemSet& rInAttrs ) :
                            Control ( pParent, rResId ),
                            rAttrs  ( rInAttrs ),
                            pEdgeObj( NULL ),
                            maSdrObjectVector(),
                            pView   ( NULL )
{
    SetMapMode( MAP_100TH_MM );
    SetStyles();
}

/*************************************************************************
|*
|* Dtor SvxXConnectionPreview
|*
*************************************************************************/

SvxXConnectionPreview::~SvxXConnectionPreview()
{
    for(sal_uInt32 a(0); a < maSdrObjectVector.size(); a++)
    {
        SdrObject::deleteSafe(const_cast< SdrObject* >(maSdrObjectVector[a]));
    }
}

/*************************************************************************
|*
|* Dtor SvxXConnectionPreview
|*
*************************************************************************/

void SvxXConnectionPreview::Construct()
{
    DBG_ASSERT( pView, "Keine gueltige View Uebergeben!" );

    if(pView->areSdrObjectsSelected())
    {
        bool bFound = false;
        const SdrObjectVector aSelection(pView->getSelectedSdrObjectVectorFromSdrMarkView());

        for(sal_uInt32 i(0); i < aSelection.size() && !bFound; i++)
        {
            const SdrEdgeObj* pTmpEdgeObj = dynamic_cast< const SdrEdgeObj* >(aSelection[i]);

            if(pTmpEdgeObj)
            {
                bFound = true;
                pEdgeObj = (SdrEdgeObj*) pTmpEdgeObj->CloneSdrObject();

                SdrObjConnection& rConn1 = (SdrObjConnection&)pEdgeObj->GetConnection(true);
                SdrObjConnection& rConn2 = (SdrObjConnection&)pEdgeObj->GetConnection(false);

                rConn1 = pTmpEdgeObj->GetConnection(true);
                rConn2 = pTmpEdgeObj->GetConnection(false);

                SdrObject* pTmpObj1 = pTmpEdgeObj->GetConnectedNode(true);
                SdrObject* pTmpObj2 = pTmpEdgeObj->GetConnectedNode(false);

                if( pTmpObj1 )
                {
                    SdrObject* pObj1 = pTmpObj1->CloneSdrObject();

                    maSdrObjectVector.push_back(pObj1);
                    pEdgeObj->ConnectToNode(true, pObj1);
                }

                if( pTmpObj2 )
                {
                    SdrObject* pObj2 = pTmpObj2->CloneSdrObject();

                    maSdrObjectVector.push_back(pObj2);
                    pEdgeObj->ConnectToNode(false, pObj2);
                }

                maSdrObjectVector.push_back(pEdgeObj);
            }
        }
    }

    if( !pEdgeObj )
    {
        pEdgeObj = new SdrEdgeObj(pView->getSdrModelFromSdrView());
    }

    // Groesse anpassen
    if( maSdrObjectVector.size() )
    {
        OutputDevice* pOD = pView->GetFirstOutputDevice(); // GetWin( 0 );
        Rectangle aRect(sdr::legacy::GetAllObjBoundRect(maSdrObjectVector));

        MapMode aMapMode = GetMapMode();
        aMapMode.SetMapUnit( pOD->GetMapMode().GetMapUnit() );
        SetMapMode( aMapMode );

        MapMode         aDisplayMap( aMapMode );
        Point           aNewPos;
        Size            aNewSize;
        const Size      aWinSize = PixelToLogic( GetOutputSizePixel(), aDisplayMap );
        const long      nWidth = aWinSize.Width();
        const long      nHeight = aWinSize.Height();
        double          fRectWH = (double) aRect.GetWidth() / aRect.GetHeight();
        double          fWinWH = (double) nWidth / nHeight;

        // Bitmap an Thumbgroesse anpassen (hier nicht!)
        if ( fRectWH < fWinWH)
        {
            aNewSize.Width() = (long) ( (double) nHeight * fRectWH );
            aNewSize.Height()= nHeight;
        }
        else
        {
            aNewSize.Width() = nWidth;
            aNewSize.Height()= (long) ( (double) nWidth / fRectWH );
        }

        Fraction aFrac1( aWinSize.Width(), aRect.GetWidth() );
        Fraction aFrac2( aWinSize.Height(), aRect.GetHeight() );
        Fraction aMinFrac( aFrac1 <= aFrac2 ? aFrac1 : aFrac2 );

        // MapMode umsetzen
        aDisplayMap.SetScaleX( aMinFrac );
        aDisplayMap.SetScaleY( aMinFrac );

        // Zentrierung
        aNewPos.X() = ( nWidth - aNewSize.Width() )  >> 1;
        aNewPos.Y() = ( nHeight - aNewSize.Height() ) >> 1;

        aDisplayMap.SetOrigin( LogicToLogic( aNewPos, aMapMode, aDisplayMap ) );
        SetMapMode( aDisplayMap );

        // Ursprung
        aNewPos = aDisplayMap.GetOrigin();
        aNewPos -= Point( aRect.TopLeft().X(), aRect.TopLeft().Y() );
        aDisplayMap.SetOrigin( aNewPos );
        SetMapMode( aDisplayMap );


        Point aPos;
        MouseEvent aMEvt( aPos, 1, 0, MOUSE_RIGHT );
        MouseButtonDown( aMEvt );
    }
}

/*************************************************************************
|*
|* SvxXConnectionPreview: Paint()
|*
*************************************************************************/

void SvxXConnectionPreview::Paint( const Rectangle& )
{
    if( maSdrObjectVector.size() )
    {
        // #110094#
        // This will not work anymore. To not start at Adam and Eve, i will
        // ATM not try to change all this stuff to really using an own model
        // and a view. I will just try to provide a mechanism to paint such
        // objects without own model and without a page/view with the new
        // mechanism.

        // New stuff: Use a ObjectContactOfObjListPainter.
        sdr::contact::ObjectContactOfObjListPainter aPainter(*this, maSdrObjectVector, 0);
        sdr::contact::DisplayInfo aDisplayInfo;

        // do processing
        aPainter.ProcessDisplay(aDisplayInfo);
    }
}

/*************************************************************************
|*
|* SvxXConnectionPreview: SetAttributes()
|*
*************************************************************************/

void SvxXConnectionPreview::SetAttributes( const SfxItemSet& rInAttrs )
{
    //pEdgeObj->SetItemSetAndBroadcast(rInAttrs);
    pEdgeObj->SetMergedItemSetAndBroadcast(rInAttrs);

    Invalidate();
}

/*************************************************************************
|*
|* Ermittelt die Anzahl der Linienversaetze anhand des Preview-Objektes
|*
*************************************************************************/

sal_uInt16 SvxXConnectionPreview::GetLineDeltaAnz()
{
    const SfxItemSet& rSet = pEdgeObj->GetMergedItemSet();
    sal_uInt16 nCount(0);

    if(SFX_ITEM_DONTCARE != rSet.GetItemState(SDRATTR_EDGELINEDELTAANZ))
        nCount = ((const SfxUInt16Item&)rSet.Get(SDRATTR_EDGELINEDELTAANZ)).GetValue();

    return nCount;
}

/*************************************************************************
|*
|* SvxXConnectionPreview: MouseButtonDown()
|*
*************************************************************************/

void SvxXConnectionPreview::MouseButtonDown( const MouseEvent& rMEvt )
{
    sal_Bool bZoomIn  = rMEvt.IsLeft() && !rMEvt.IsShift();
    sal_Bool bZoomOut = rMEvt.IsRight() || rMEvt.IsShift();
    sal_Bool bCtrl    = rMEvt.IsMod1();

    if( bZoomIn || bZoomOut )
    {
        MapMode aMapMode = GetMapMode();
        Fraction aXFrac = aMapMode.GetScaleX();
        Fraction aYFrac = aMapMode.GetScaleY();
        Fraction* pMultFrac;

        if( bZoomIn )
        {
            if( bCtrl )
                pMultFrac = new Fraction( 3, 2 );
            else
                pMultFrac = new Fraction( 11, 10 );
        }
        else
        {
            if( bCtrl )
                pMultFrac = new Fraction( 2, 3 );
            else
                pMultFrac = new Fraction( 10, 11 );
        }

        aXFrac *= *pMultFrac;
        aYFrac *= *pMultFrac;
        if( (double)aXFrac > 0.001 && (double)aXFrac < 1000.0 &&
            (double)aYFrac > 0.001 && (double)aYFrac < 1000.0 )
        {
            aMapMode.SetScaleX( aXFrac );
            aMapMode.SetScaleY( aYFrac );
            SetMapMode( aMapMode );

            Size aOutSize( GetOutputSize() );

            Point aPt( aMapMode.GetOrigin() );
            long nX = (long)( ( (double)aOutSize.Width() - ( (double)aOutSize.Width() * (double)*pMultFrac  ) ) / 2.0 + 0.5 );
            long nY = (long)( ( (double)aOutSize.Height() - ( (double)aOutSize.Height() * (double)*pMultFrac  ) ) / 2.0 + 0.5 );
            aPt.X() +=  nX;
            aPt.Y() +=  nY;

            aMapMode.SetOrigin( aPt );
            SetMapMode( aMapMode );

            Invalidate();
        }
        delete pMultFrac;
    }
}

void SvxXConnectionPreview::SetStyles()
{
    const StyleSettings& rStyles = Application::GetSettings().GetStyleSettings();
    SetDrawMode( GetSettings().GetStyleSettings().GetHighContrastMode() ? OUTPUT_DRAWMODE_CONTRAST : OUTPUT_DRAWMODE_COLOR );
    SetBackground( Wallpaper( Color( rStyles.GetFieldColor() ) ) );
}

void SvxXConnectionPreview::DataChanged( const DataChangedEvent& rDCEvt )
{
    Control::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) && (rDCEvt.GetFlags() & SETTINGS_STYLE) )
    {
        SetStyles();
    }
}

