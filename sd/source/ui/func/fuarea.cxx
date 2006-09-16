/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fuarea.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-16 18:46:34 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"


#include "fuarea.hxx"

#include <svx/svxids.hrc>
#ifndef _SVX_TAB_AREA_HXX //autogen
#include <svx/tabarea.hxx>
#endif
#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SFXINTITEM_HXX //autogen
#include <svtools/intitem.hxx>
#endif
#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif
#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif
#ifndef _SFXVIEWFRM_HXX
#include <sfx2/viewfrm.hxx>
#endif
#ifndef _SFX_BINDINGS_HXX //autogen
#include <sfx2/bindings.hxx>
#endif
#ifndef SD_VIEW_SHELL_HXX
#include "ViewShell.hxx"
#endif

#include "drawdoc.hxx"
#ifndef SD_VIEW_HXX
#include "View.hxx"
#endif
#ifndef SD_WINDOW_HXX
#include "Window.hxx"
#endif
#include "app.hrc"
#include <svx/svxdlg.hxx> //CHINA001
#include <svx/dialogs.hrc> //CHINA001

namespace sd {
TYPEINIT1( FuArea, FuPoor );

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

FuArea::FuArea( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq)
: FuPoor(pViewSh, pWin, pView, pDoc, rReq)
{
}

FunctionReference FuArea::Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq )
{
    FunctionReference xFunc( new FuArea( pViewSh, pWin, pView, pDoc, rReq ) );
    xFunc->DoExecute(rReq);
    return xFunc;
}

void FuArea::DoExecute( SfxRequest& rReq )
{
    const SfxItemSet* pArgs = rReq.GetArgs();

    if( !pArgs )
    {
        // erst einmal alle eingabeparameter fuer den dialog retten
        SfxItemSet aInputAttr( pDoc->GetPool() );
        pView->GetAttributes( aInputAttr );

        const XFillStyleItem    &rIFillStyleItem    = (const XFillStyleItem &) aInputAttr.Get (XATTR_FILLSTYLE);
        const XFillColorItem    &rIFillColorItem    = (const XFillColorItem &) aInputAttr.Get (XATTR_FILLCOLOR);
        const XFillGradientItem &rIFillGradientItem = (const XFillGradientItem &) aInputAttr.Get (XATTR_FILLGRADIENT);
        const XFillHatchItem    &rIFillHatchItem    = (const XFillHatchItem &) aInputAttr.Get (XATTR_FILLHATCH);
        const XFillBitmapItem   &rIXFillBitmapItem  = (const XFillBitmapItem &) aInputAttr.Get (XATTR_FILLBITMAP);

        SfxItemSet* pNewAttr = new SfxItemSet( pDoc->GetPool() );
        pView->GetAttributes( *pNewAttr );

        //CHINA001 SvxAreaTabDialog* pDlg = new SvxAreaTabDialog( NULL, pNewAttr, pDoc, pView );
        SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
        DBG_ASSERT(pFact, "Dialogdiet Factory fail!");//CHINA001
        AbstractSvxAreaTabDialog * pDlg = pFact->CreateSvxAreaTabDialog( NULL,
                                                                        pNewAttr,
                                                                        pDoc,
                                                                        ResId(RID_SVXDLG_AREA),
                                                                        pView);
        DBG_ASSERT(pDlg, "Dialogdiet fail!");//CHINA001
        if ( pDlg->Execute() == RET_OK )
        {
            pView->SetAttributes (*(pDlg->GetOutputItemSet ()));
        }

        // Attribute wurden geaendert, Listboxes in Objectbars muessen aktualisiert werden
        static USHORT SidArray[] = {
                        SID_ATTR_FILL_STYLE,
                        SID_ATTR_FILL_COLOR,
                        SID_ATTR_FILL_GRADIENT,
                        SID_ATTR_FILL_HATCH,
                        SID_ATTR_FILL_BITMAP,
                        0 };

        pViewShell->GetViewFrame()->GetBindings().Invalidate( SidArray );

        delete pDlg;
        delete pNewAttr;
    }

    rReq.Ignore ();

}

void FuArea::Activate()
{
}

void FuArea::Deactivate()
{
}

} // end of namespace sd
