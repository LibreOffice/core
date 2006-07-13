/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PresentationViewShellBase.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: obo $ $Date: 2006-07-13 10:31:14 $
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

#include "PresentationViewShellBase.hxx"

#ifndef SD_RESID_HXX
#include "sdresid.hxx"
#endif
#ifndef SD_DRAW_DOC_SHELL_HXX
#include "DrawDocShell.hxx"
#endif
#include "strings.hrc"
#include "ViewTabBar.hxx"
#include "UpdateLockManager.hxx"

#include <sfx2/viewfrm.hxx>
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XLAYOUTMANAGER_HPP_
#include <com/sun/star/frame/XLayoutManager.hpp>
#endif

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace sd {

class DrawDocShell;

TYPEINIT1(PresentationViewShellBase, ViewShellBase);

// We have to expand the SFX_IMPL_VIEWFACTORY macro to call LateInit() after a
// new PresentationViewShellBase object has been constructed.

/*
SFX_IMPL_VIEWFACTORY(PresentationViewShellBase, SdResId(STR_DEFAULTVIEW))
{
    SFX_VIEW_REGISTRATION(DrawDocShell);
}
*/
SfxViewFactory* PresentationViewShellBase::pFactory;
SfxViewShell* __EXPORT PresentationViewShellBase::CreateInstance (
    SfxViewFrame *pFrame, SfxViewShell *pOldView)
{
    PresentationViewShellBase* pBase =
        new PresentationViewShellBase(pFrame, pOldView);
    pBase->LateInit();
    return pBase;
}
void PresentationViewShellBase::RegisterFactory( USHORT nPrio )
{
    pFactory = new SfxViewFactory(
        &CreateInstance,&InitFactory,nPrio,SdResId(STR_DEFAULTVIEW));
    InitFactory();
}
void PresentationViewShellBase::InitFactory()
{
    SFX_VIEW_REGISTRATION(DrawDocShell);
}




PresentationViewShellBase::PresentationViewShellBase (
    SfxViewFrame* pFrame,
    SfxViewShell* pOldShell)
    : ViewShellBase (pFrame, pOldShell, ViewShell::ST_PRESENTATION)
{
    GetUpdateLockManager().Disable();

    // Hide the automatic (non-context sensitive) tool bars.
    if (pFrame!=NULL && pFrame->GetFrame()!=NULL)
    {
        Reference<beans::XPropertySet> xFrameSet (
            pFrame->GetFrame()->GetFrameInterface(),
            UNO_QUERY);
        if (xFrameSet.is())
        {
            Reference<beans::XPropertySet> xLayouterSet (
                xFrameSet->getPropertyValue(::rtl::OUString::createFromAscii("LayoutManager")),
                UNO_QUERY);
            if (xLayouterSet.is())
            {
                xLayouterSet->setPropertyValue(
                    ::rtl::OUString::createFromAscii("AutomaticToolbars"),
                    makeAny(sal_False));
            }
        }
    }
}




ViewTabBar* PresentationViewShellBase::CreateViewTabBar (void)
{
    // The ViewTabBar is not supported.
    return NULL;
}




PresentationViewShellBase::~PresentationViewShellBase (void)
{
}



} // end of namespace sd

