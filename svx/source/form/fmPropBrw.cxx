/*************************************************************************
 *
 *  $RCSfile: fmPropBrw.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: oj $ $Date: 2000-11-24 07:01:26 $
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
 *  Source License Version 1.1 (the License); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an AS IS basis,
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
#ifndef SVX_FMPROPBRW_HXX
#include "fmPropBrw.hxx"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _SFX_BINDINGS_HXX
#include <sfx2/bindings.hxx>
#endif
#ifndef _SFX_CHILDWIN_HXX
#include <sfx2/childwin.hxx>
#endif
#ifndef _SFX_OBJITEM_HXX
#include <sfx2/objitem.hxx>
#endif
#ifndef SVX_FMPROPCONTROL_HXX
#include "fmpropcontrol.hxx"
#endif
#ifndef SVX_FMPROPCONTROLLER_HXX
#include "fmPropController.hxx"
#endif
#ifndef _SVX_SVXIDS_HRC
#include "svxids.hrc"
#endif
#ifndef SVX_SVXPROPERTYBOX_HXX
#include "fmPropertyBox.hxx"
#endif
#ifndef _SVX_FMSHELL_HXX
#include "fmshell.hxx"
#endif
#ifndef _SVX_FMSHIMP_HXX
#include "fmshimp.hxx"
#endif
/*************************************************************************/
using namespace ::com::sun::star::uno;
//========================================================================
// class FmPropBrw
//========================================================================
const long STD_WIN_SIZE_X = 300;
const long STD_WIN_SIZE_Y = 350;

const long STD_MIN_SIZE_X = 250;
const long STD_MIN_SIZE_Y = 250;

const long STD_WIN_POS_X = 50;
const long STD_WIN_POS_Y = 50;
const long WIN_BORDER = 2;
const long MIN_WIN_SIZE_X = 50;
const long MIN_WIN_SIZE_Y = 50;

DBG_NAME(FmPropBrw);
//------------------------------------------------------------------------
FmPropBrw::FmPropBrw(const Reference< ::com::sun::star::lang::XMultiServiceFactory >&   _xORB,
                     SfxBindings *pBindings, SfxChildWindow *pMgr, Window* pParent)
          :SfxFloatingWindow(pBindings, pMgr, pParent,WinBits(WB_STDMODELESS|WB_SIZEABLE|WB_3DLOOK|WB_ROLLABLE))
          ,SfxControllerItem(SID_FM_PROPERTY_CONTROL, *pBindings)
          ,m_bInitialStateChange(sal_True)
          ,m_xORB(_xORB)
{
    DBG_CTOR(FmPropBrw,NULL);

    Size aPropWinSize(STD_WIN_SIZE_X,STD_WIN_SIZE_Y);
    SetMinOutputSizePixel(Size(STD_MIN_SIZE_X,STD_MIN_SIZE_Y));
    SetOutputSizePixel(aPropWinSize);

    pMasterController = new FmPropControl( _xORB,this );
    Point aPropWinPos = Point( WIN_BORDER, WIN_BORDER );
    aPropWinSize.Width() -= (2*WIN_BORDER);
    aPropWinSize.Height() -= (2*WIN_BORDER);

    pMasterController->SetPosSizePixel(aPropWinPos, aPropWinSize);
    pMasterController->Show();
}

//------------------------------------------------------------------------
FmPropBrw::~FmPropBrw()
{
    delete pMasterController;

    DBG_DTOR(FmPropBrw,NULL);
}

//-----------------------------------------------------------------------
sal_Bool FmPropBrw::Close()
{
    FmPropController* m_pActiveController = pMasterController->GetActiveController();
    if( m_pActiveController )
        m_pActiveController->DisconnectUI();

    pMasterController->UpdateController(Reference< XInterface > ());

    if( IsRollUp() )
        RollDown();

    // remember our bindings: while we're closed, we're deleted, too, so accessing the bindings after this
    // would be deadly
    // 10/19/00 - 79321 - FS
    SfxBindings& rBindings = SfxControllerItem::GetBindings();

    sal_Bool bClose = SfxFloatingWindow::Close();

    if (bClose)
    {
        rBindings.Invalidate(SID_FM_CTL_PROPERTIES);
        rBindings.Invalidate(SID_FM_PROPERTIES);
    }

    return bClose;
}

//-----------------------------------------------------------------------
void FmPropBrw::FillInfo( SfxChildWinInfo& rInfo ) const
{
    rInfo.bVisible = sal_False;
}

//------------------------------------------------------------------------
void FmPropBrw::Resize()
{
    SfxFloatingWindow::Resize();

    Size  aSize = GetOutputSizePixel();

    //////////////////////////////////////////////////////////////////////
    // Groesse anpassen
    Size aPropWinSize( aSize );
    aPropWinSize.Width() -= (2*WIN_BORDER);
    aPropWinSize.Height() -= (2*WIN_BORDER);

    pMasterController->SetSizePixel(aPropWinSize);
}

//-----------------------------------------------------------------------
void FmPropBrw::StateChanged(sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState)
{
try
{
    if (!pState  || SID_FM_PROPERTY_CONTROL != nSID)
        return;

    if (eState >= SFX_ITEM_AVAILABLE)
    {
        // wenn ich gerade neu angelegt worden bin, moechte ich den Fokus haben
        if (m_bInitialStateChange)
        {
            pMasterController->m_pPropBox->GrabFocus();
            m_bInitialStateChange = sal_False;
        }

        FmFormShell* pShell = PTR_CAST(FmFormShell,((SfxObjectItem*)pState)->GetShell());
        Reference< XInterface >  xObject;
        if (pShell)
        {
            // mal schauen ob ein object selektiert ist
            xObject = pShell->GetImpl()->getSelObject();
        }
        if (xObject != pMasterController->GetObject() || !xObject.is())  // leere Objecte immer neu anzeigen
            pMasterController->UpdateController(xObject, pShell);
    }
    else
    {
        pMasterController->UpdateController(Reference< XInterface > ());
    }
}

catch(Exception&)
{
    DBG_ERROR("FmPropBrw::StateChanged: Exception occured!");
}

}
