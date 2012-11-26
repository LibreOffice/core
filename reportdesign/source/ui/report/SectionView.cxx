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


#include "precompiled_reportdesign.hxx"
#include "SectionView.hxx"
#include "DesignView.hxx"
#include <RptPage.hxx>
#include <RptObject.hxx>
#include <RptDef.hxx>
#include <svx/svxids.hrc>
#include <svx/svddrgmt.hxx>
#include <vcl/scrbar.hxx>
#include "ReportSection.hxx"
#include "ReportWindow.hxx"
#include "uistrings.hrc"
#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>

namespace rptui
{
    using namespace ::com::sun::star;

//----------------------------------------------------------------------------
DBG_NAME( rpt_OSectionView )
OSectionView::OSectionView( SdrModel& rModel, OReportSection* _pSectionWindow, OReportWindow* pEditor )
    :SdrView( rModel, _pSectionWindow )
    ,m_pReportWindow( pEditor )
    ,m_pSectionWindow(_pSectionWindow)
{
    DBG_CTOR( rpt_OSectionView,NULL);
    // SetPagePaintingAllowed(false);
    SetBufferedOutputAllowed(true);
    SetBufferedOverlayAllowed(true);
    SetPageBorderVisible(false);
    SetBordVisible();
    SetQuickTextEditMode(sal_False);
}

//----------------------------------------------------------------------------

OSectionView::~OSectionView()
{
    DBG_DTOR( rpt_OSectionView,NULL);
}

//----------------------------------------------------------------------------

void OSectionView::handleSelectionChange()
{
    // call parent
    SdrView::handleSelectionChange();

    // local reactions
    if ( m_pReportWindow && m_pSectionWindow && !m_pSectionWindow->getPage()->getSpecialMode() )
    {
        //m_pReportWindow->unmarkAllObjects(this); // WHY
        DlgEdHint aHint( RPTUI_HINT_SELECTIONCHANGED );
        m_pReportWindow->getReportView()->Broadcast( aHint );
        m_pReportWindow->getReportView()->UpdatePropertyBrowserDelayed(*this);
    }
}

//----------------------------------------------------------------------------

void OSectionView::MakeVisibleAtView( const basegfx::B2DRange& rRange, Window& rWin )
{
    DBG_CHKTHIS( rpt_OSectionView,NULL);
    // visible area
    const basegfx::B2DRange aVisRange(rWin.GetLogicRange());

    // check, if rectangle is inside visible area
    if ( !aVisRange.isInside( rRange ) )
    {
        // calculate scroll distance; the rectangle must be inside the visible area
        double fScrollX(0.0);
        double fScrollY(0.0);

        const double fVisLeft(aVisRange.getMinX());
        const double fVisRight(aVisRange.getMaxX());
        const double fVisTop(aVisRange.getMinY());
        const double fVisBottom(aVisRange.getMaxY());

        // don't scroll beyond the page size
        const double fPageWidth(m_pSectionWindow->getPage()->GetPageScale().getX());
        const double fPageHeight(m_pSectionWindow->getPage()->GetPageScale().getY());

        if ( fVisRight + fScrollX > fPageWidth )
        {
            fScrollX = fPageWidth - fVisRight;
        }

        if ( fVisLeft + fScrollX < 0 )
        {
            fScrollX = -fVisLeft;
        }

        if ( fVisBottom + fScrollY > fPageHeight )
        {
            fScrollY = fPageHeight - fVisBottom;
        }

        if ( fVisTop + fScrollY < 0 )
        {
            fScrollY = -fVisTop;
        }

        // scroll window
        const sal_uInt32 nScrollX(basegfx::fround(fScrollX));
        const sal_uInt32 nScrollY(basegfx::fround(fScrollY));
        MapMode aMap( rWin.GetMapMode() );

        rWin.Update();
        rWin.Scroll( -nScrollX, -nScrollY );
        aMap.SetOrigin( Point( aMap.GetOrigin().X() - nScrollX, aMap.GetOrigin().Y() - nScrollY ) );
        rWin.SetMapMode( aMap );
        rWin.Update();
        rWin.Invalidate();

        if ( m_pReportWindow )
        {
            const DlgEdHint aHint( RPTUI_HINT_WINDOWSCROLLED );
            m_pReportWindow->getReportView()->Broadcast( aHint );
        }
    }
    else
    {
        rWin.Invalidate(INVALIDATE_NOERASE);
    }
}
//------------------------------------------------------------------------------
void OSectionView::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    DBG_CHKTHIS( rpt_OSectionView,NULL);
    SdrView::Notify(rBC,rHint);
    const SdrBaseHint* pSdrHint = dynamic_cast< const SdrBaseHint* >(&rHint);

    if ( pSdrHint )
    {
        const SdrObject* pObj = pSdrHint->GetSdrHintObject();
        const SdrHintKind eKind = pSdrHint->GetSdrHintKind();
        // check for change of selected object
        if((HINT_OBJCHG_MOVE == eKind || HINT_OBJCHG_RESIZE == eKind || HINT_OBJCHG_ATTR == eKind)
            && pObj && IsObjMarked(*pObj))
        {
            SetMarkHandles();
        }
        else if ( eKind == HINT_OBJREMOVED )
        {
            ObjectRemovedInAliveMode(pObj);
    }
}
}

//------------------------------------------------------------------------------
void OSectionView::ObjectRemovedInAliveMode( const SdrObject* _pObject )
{
    if(_pObject)
    {
    DBG_CHKTHIS( rpt_OSectionView,NULL);
        const SdrObjectVector aSelection(getSelectedSdrObjectVectorFromSdrMarkView());

        for( sal_uInt32 i(0); i < aSelection.size(); i++ )
    {
            SdrObject* pSdrObj = aSelection[i];

        if (_pObject == pSdrObj)
        {
            BrkAction();
                MarkObj( *pSdrObj, true );
            break;
        }
    }
}
}

// -----------------------------------------------------------------------------
void OSectionView::SetMarkedToLayer( SdrLayerID _nLayerNo )
{
    if (areSdrObjectsSelected())
    {
        //  #i11702# use SdrUndoObjectLayerChange for undo
        //  STR_UNDO_SELATTR is "Attributes" - should use a different text later
        BegUndo( );
        const SdrObjectVector aSelection(getSelectedSdrObjectVectorFromSdrMarkView());

        for (sal_uInt32 i(0); i < aSelection.size(); i++)
        {
            OCustomShape* pObj = dynamic_cast< OCustomShape* >(aSelection[i]);

            if ( pObj )
            {
                AddUndo( new SdrUndoObjectLayerChange( *pObj, pObj->GetLayer(), _nLayerNo) );
                pObj->SetLayer( _nLayerNo );
                OObjectBase* pBaseObj = dynamic_cast<OObjectBase*>(pObj);

                try
                {
                    pBaseObj->getReportComponent()->setPropertyValue(PROPERTY_OPAQUE,uno::makeAny(_nLayerNo == RPT_LAYER_FRONT));
                }
                catch(const uno::Exception&)
                {
                    DBG_UNHANDLED_EXCEPTION();
                }
            }
        }

        EndUndo();
    }
}
// -----------------------------------------------------------------------------
bool OSectionView::OnlyShapesMarked() const
{
    const SdrObjectVector aSelection(getSelectedSdrObjectVectorFromSdrMarkView());

    if(!aSelection.size())
    {
        return false;
    }

    sal_uInt32 a(0);

    for(; a < aSelection.size(); a++)
    {
        if(!dynamic_cast< OCustomShape* >(aSelection[a]))
        {
            break;
        }
    }

    return a == aSelection.size();
}

bool OSectionView::IsDragResize() const
{
    const SdrDragMethod* pDragMethod = GetDragMethod();
    if (pDragMethod)
    {
        bool bMoveOnly = pDragMethod->getMoveOnly();
        if (bMoveOnly == false)
        {
            // current marked components will be resized
            return true;
        }
    }
    return false;
}

// -----------------------------------------------------------------------------
short OSectionView::GetLayerIdOfMarkedObjects() const
{
    short nRet = SHRT_MAX;
    const SdrObjectVector aSelection(getSelectedSdrObjectVectorFromSdrMarkView());

    for ( sal_uInt32 i(0); i < aSelection.size(); ++i )
    {
        const SdrObject* pObj = aSelection[i];

        if ( nRet == SHRT_MAX )
        {
            nRet = pObj->GetLayer();
        }
        else if ( nRet != pObj->GetLayer() )
        {
            nRet = -1;
            break;
        }
    }

    if ( nRet == SHRT_MAX )
    {
        nRet = -1;
    }

    return nRet;
}

//============================================================================
} // rptui
//============================================================================
