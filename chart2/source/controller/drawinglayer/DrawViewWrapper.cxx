/*************************************************************************
 *
 *  $RCSfile: DrawViewWrapper.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: bm $ $Date: 2003-10-06 09:58:27 $
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

//.............................................................................
namespace chart
{
//.............................................................................

DrawViewWrapper::DrawViewWrapper( SdrModel* pModel, OutputDevice* pOut)
            : E3dView(pModel, pOut)
            , m_pWrappedDLPageView(NULL)
            , m_pMarkHandleProvider(NULL)
{
    m_pWrappedDLPageView = this->ShowPagePgNum( 0, Point(0,0) );

    /*
    m_pWrappedDLPageView->GetPage()->SetBorder(0, 0, 0, 0);
    m_pWrappedDLPageView->GetPage()->SetSize(Size(1000,1000));
    this->SetBordVisible(false);
    this->SetPageBorderVisible(false);
    */
}

DrawViewWrapper::~DrawViewWrapper()
{
    m_pWrappedDLPageView = NULL;//@ todo: sufficient? or remove necessary
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
            nHitTolerance = pOutDev->PixelToLogic(Size(HITPIX,0)).Width();
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

void DrawViewWrapper::InitRedraw( OutputDevice* pOut, const Region& rReg )
{
    this->E3dView::InitRedraw( pOut, rReg );
}

//.............................................................................
} //namespace chart
//.............................................................................
