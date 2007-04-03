/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ImpressViewShellBase.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-04-03 16:26:19 $
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

#include "precompiled_sd.hxx"

#include "ImpressViewShellBase.hxx"

#include "DrawDocShell.hxx"
#ifndef SD_RESID_HXX
#include "sdresid.hxx"
#endif
#include "strings.hrc"
#include "app.hrc"
#include "framework/FrameworkHelper.hxx"
#include "framework/ImpressModule.hxx"
#include "MasterPageObserver.hxx"

#ifndef _SFXREQUEST_HXX
#include <sfx2/request.hxx>
#endif

namespace sd {

TYPEINIT1(ImpressViewShellBase, ViewShellBase);

// We have to expand the SFX_IMPL_VIEWFACTORY macro to call LateInit() after a
// new ImpressViewShellBase object has been constructed.

/*
SFX_IMPL_VIEWFACTORY(ImpressViewShellBase, SdResId(STR_DEFAULTVIEW))
{
    SFX_VIEW_REGISTRATION(DrawDocShell);
}
*/
SfxViewFactory* ImpressViewShellBase::pFactory;
SfxViewShell* __EXPORT ImpressViewShellBase::CreateInstance (
    SfxViewFrame *pFrame, SfxViewShell *pOldView)
{
    ImpressViewShellBase* pBase = new ImpressViewShellBase(pFrame, pOldView);
    pBase->LateInit(::rtl::OUString());
    return pBase;
}
void ImpressViewShellBase::RegisterFactory( USHORT nPrio )
{
    pFactory = new SfxViewFactory(
        &CreateInstance,&InitFactory,nPrio,SdResId(STR_DEFAULTVIEW));
    InitFactory();
}
void ImpressViewShellBase::InitFactory()
{
    SFX_VIEW_REGISTRATION(DrawDocShell);
}








ImpressViewShellBase::ImpressViewShellBase (
    SfxViewFrame* _pFrame,
    SfxViewShell* pOldShell)
    : ViewShellBase (_pFrame, pOldShell)
{
    MasterPageObserver::Instance().RegisterDocument (*GetDocShell()->GetDoc());
}




ImpressViewShellBase::~ImpressViewShellBase (void)
{
    MasterPageObserver::Instance().UnregisterDocument (*GetDocShell()->GetDoc());
}




void ImpressViewShellBase::Execute (SfxRequest& rRequest)
{
    USHORT nSlotId = rRequest.GetSlot();

    switch (nSlotId)
    {
        case SID_LEFT_PANE_DRAW:
            // Prevent a Draw-only slots from being executed.
            rRequest.Cancel();
            break;

        default:
            // The remaining requests are forwarded to our base class.
            ViewShellBase::Execute(rRequest);
            break;
    }
}




void ImpressViewShellBase::InitializeFramework (void)
{
    ::com::sun::star::uno::Reference<com::sun::star::frame::XController>
        xController (GetController());
    sd::framework::ImpressModule::Initialize(xController);
}

} // end of namespace sd

