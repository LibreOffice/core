/*************************************************************************
 *
 *  $RCSfile: DrawViewWrapper.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-12 15:35:08 $
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
 *  Copyright: 2003 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#include "DrawViewWrapper.hxx"
#include "DrawModelWrapper.hxx"
#include "ConfigurationAccess.hxx"
#include "ViewSingletons.hxx"

// header for class SdrPage
#ifndef _SVDPAGE_HXX
#include <svx/svdpage.hxx>
#endif
//header for class SdrPageView
#ifndef _SVDPAGV_HXX
#include <svx/svdpagv.hxx>
#endif
// header for class SdrModel
#ifndef _SVDMODEL_HXX
#include <svx/svdmodel.hxx>
#endif

#ifndef _SVDETC_HXX
#include <svx/svdetc.hxx>
#endif

#ifndef _SVDOUTL_HXX
#include <svx/svdoutl.hxx>
#endif

// header for class SvxForbiddenCharactersTable
#ifndef _FORBIDDENCHARACTERSTABLE_HXX
#include <svx/forbiddencharacterstable.hxx>
#endif

#ifndef _SVX_SVXIDS_HRC
#include <svx/svxids.hrc>
#endif

//.............................................................................
namespace chart
{
//.............................................................................
    /*
void lcl_initOutliner( SdrOutliner* pTargetOutliner, SdrOutliner* pSourceOutliner )
{
    //just an unsuccessful try to initialize the text edit outliner correctly
    //if( bInit )
    {
        pTargetOutliner->EraseVirtualDevice();
        pTargetOutliner->SetUpdateMode(FALSE);
        pTargetOutliner->SetEditTextObjectPool( pSourceOutliner->GetEditTextObjectPool() );
        pTargetOutliner->SetDefTab( pSourceOutliner->GetDefTab() );
    }

    pTargetOutliner->SetRefDevice( pSourceOutliner->GetRefDevice() );
    pTargetOutliner->SetForbiddenCharsTable( pSourceOutliner->GetForbiddenCharsTable() );
    pTargetOutliner->SetAsianCompressionMode( pSourceOutliner->GetAsianCompressionMode() );
    pTargetOutliner->SetKernAsianPunctuation( pSourceOutliner->IsKernAsianPunctuation() );
    pTargetOutliner->SetStyleSheetPool( pSourceOutliner->GetStyleSheetPool() );
    pTargetOutliner->SetRefMapMode( pSourceOutliner->GetRefMapMode() );
    pTargetOutliner->SetDefaultLanguage( pSourceOutliner->GetDefaultLanguage() );
    pTargetOutliner->SetHyphenator( pSourceOutliner->GetHyphenator() );

    USHORT nX, nY;
    pSourceOutliner->GetGlobalCharStretching( nX, nY );
    pTargetOutliner->SetGlobalCharStretching( nX, nY );

    *//*
    if ( !GetRefDevice() )
    {
        MapMode aMapMode(eObjUnit, Point(0,0), aObjUnit, aObjUnit);
        pTargetOutliner->SetRefMapMode(aMapMode);
    }
    *//*
}
*/

DrawViewWrapper::DrawViewWrapper( SdrModel* pSdrModel, OutputDevice* pOut)
            : E3dView(pSdrModel, pOut)
            , m_pWrappedDLPageView(NULL)
            , m_pMarkHandleProvider(NULL)
            , m_apOutliner( SdrMakeOutliner( OUTLINERMODE_TEXTOBJECT, pSdrModel ) )
{
    // #114898#
    SetBufferedOutputAllowed(sal_True);

    ReInit();
}

void DrawViewWrapper::ReInit()
{
    OutputDevice* pOutDev = this->GetWin(0);
    Size aOutputSize(100,100);
    if(pOutDev)
        aOutputSize = pOutDev->GetOutputSize();

    m_pWrappedDLPageView = this->ShowPagePgNum( 0, Point(0,0) );
    m_pWrappedDLPageView->GetPage()->SetSize( aOutputSize );
    this->SetPageBorderVisible(false);
    this->SetBordVisible(false);
    this->SetGridVisible(false);
    this->SetGridFront(false);
    this->SetHlplVisible(false);
    this->SetNoDragXorPolys(true);//for interactive 3D resize-dragging: paint only a single rectangle (not a simulated 3D object)
    //this->SetResizeAtCenter(true);//for interactive resize-dragging: keep the object center fix

    //a correct work area is at least necessary for correct values in the position and  size dialog
    //Rectangle aRect = pOutDev->PixelToLogic(Rectangle(Point(0,0), aOutputSize));
    Rectangle aRect(Point(0,0), aOutputSize);
    this->SetWorkArea(aRect);
}

DrawViewWrapper::~DrawViewWrapper()
{
    m_pWrappedDLPageView = NULL;//@ todo: sufficient? or remove necessary
    aComeBackTimer.Stop();//@todo this should be done in destructor of base class
}

//virtual
void DrawViewWrapper::SetMarkHandles()
{
    if( m_pMarkHandleProvider && m_pMarkHandleProvider->getMarkHandles( aHdl ) )
        return;
    else
        SdrView::SetMarkHandles();
}

SdrObject* DrawViewWrapper::getHitObject( const Point& rPnt ) const
{
    const short HITPIX=2; //hit-tolerance in pixel

    SdrObject* pRet = NULL;
    //ULONG nOptions =SDRSEARCH_DEEP|SDRSEARCH_PASS2BOUND|SDRSEARCH_PASS3NEAREST;
    //ULONG nOptions = SDRSEARCH_TESTMARKABLE;
    ULONG nOptions = SDRSEARCH_DEEP | SDRSEARCH_TESTMARKABLE;
    //ULONG nOptions = SDRSEARCH_DEEP|SDRSEARCH_ALSOONMASTER|SDRSEARCH_WHOLEPAGE|SDRSEARCH_PASS2BOUND|SDRSEARCH_PASS3NEAREST;
    //ULONG nOptions = 0;

    short nHitTolerance = 50;
    {
        OutputDevice* pOutDev = this->GetWin(0);
        if(pOutDev)
            nHitTolerance = static_cast<short>(pOutDev->PixelToLogic(Size(HITPIX,0)).Width());
    }
    this->SdrView::PickObj(rPnt, nHitTolerance, pRet, m_pWrappedDLPageView, nOptions);
    return pRet;
}

void DrawViewWrapper::MarkObject( SdrObject* pObj )
{
    bool bFrameDragSingles = true;//true == green == surrounding handles
    pObj->SetMarkProtect(false);
    if( m_pMarkHandleProvider )
        bFrameDragSingles = m_pMarkHandleProvider->getFrameDragSingles();
    this->SetFrameDragSingles(bFrameDragSingles);//decide wether each single object should get handles
    this->SdrView::MarkObj( pObj, m_pWrappedDLPageView );
    this->SetMarkHdlHidden(FALSE);
}


void DrawViewWrapper::setMarkHandleProvider( MarkHandleProvider* pMarkHandleProvider )
{
    m_pMarkHandleProvider = pMarkHandleProvider;
}

void DrawViewWrapper::CompleteRedraw( OutputDevice* pOut, const Region& rReg )
{
    svtools::ColorConfig aColorConfig;
    Color aFillColor = Color( aColorConfig.GetColorValue( svtools::DOCCOLOR ).nColor );
    this->SetApplicationBackgroundColor(aFillColor);
    this->E3dView::CompleteRedraw( pOut, rReg );
}

SdrObject* DrawViewWrapper::getSelectedObject() const
{
    SdrObject* pObj(NULL);
    const SdrMarkList& rMarkList = this->GetMarkedObjectList();
    if(rMarkList.GetMarkCount() == 1)
    {
        SdrMark* pMark = rMarkList.GetMark(0);
        pObj = pMark->GetObj();
    }
    return pObj;
}

SdrObject* DrawViewWrapper::getTextEditObject() const
{
    SdrObject* pObj = this->getSelectedObject();
    SdrObject* pTextObj = NULL;
    if( pObj && pObj->HasTextEdit())
        pTextObj = (SdrTextObj*)pObj;
    return pTextObj;
}

SdrOutliner* DrawViewWrapper::getOutliner() const
{
//    lcl_initOutliner( m_apOutliner.get(), &GetModel()->GetDrawOutliner() );
    return m_apOutliner.get();
}

SfxItemSet DrawViewWrapper::getPositionAndSizeItemSetFromMarkedObject() const
{
    SfxItemSet aFullSet( GetModel()->GetItemPool(),
                    SID_ATTR_TRANSFORM_POS_X,SID_ATTR_TRANSFORM_ANGLE,
                    SID_ATTR_TRANSFORM_PROTECT_POS,SID_ATTR_TRANSFORM_AUTOHEIGHT,
                    SDRATTR_ECKENRADIUS,SDRATTR_ECKENRADIUS,
                    SID_ATTR_METRIC,SID_ATTR_METRIC,
                    0);
    SfxItemSet aGeoSet( E3dView::GetGeoAttrFromMarked() );
    aFullSet.Put( aGeoSet );
    aFullSet.Put( SfxUInt16Item(SID_ATTR_METRIC,ViewSingletons::getConfigurationAccess()->getFieldUnit()) );
    return aFullSet;
}

//.............................................................................
} //namespace chart
//.............................................................................
