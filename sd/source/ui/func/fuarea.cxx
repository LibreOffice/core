/*************************************************************************
 *
 *  $RCSfile: fuarea.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:48:35 $
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
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#pragma hdrstop

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
#ifndef _SFX_BINDINGS_HXX //autogen
#include <sfx2/bindings.hxx>
#endif
#ifndef _SD_VIEWSHEL_HXX
#include "viewshel.hxx"
#endif

#include "drawdoc.hxx"
#include "sdview.hxx"
#include "sdwindow.hxx"
#include "fuarea.hxx"
#include "app.hrc"

TYPEINIT1( FuArea, FuPoor );

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

FuArea::FuArea(SdViewShell* pViewSh, SdWindow* pWin, SdView* pView,
               SdDrawDocument* pDoc, SfxRequest& rReq)
    : FuPoor(pViewSh, pWin, pView, pDoc, rReq)
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

        SvxAreaTabDialog* pDlg = new SvxAreaTabDialog( NULL, pNewAttr, pDoc, pView );

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

        SFX_BINDINGS().Invalidate( SidArray );

        delete pDlg;
        delete pNewAttr;
    }

    rReq.Ignore ();

}


