/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: drviewsi.cxx,v $
 *
 *  $Revision: 1.19 $
 *
 *  last change: $Author: vg $ $Date: 2006-11-01 14:18:06 $
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

#include "DrawViewShell.hxx"

#define ITEMID_COLOR            ATTR_ANIMATION_COLOR
#ifndef _XTABLE_HXX
#include <svx/xtable.hxx>
#endif
#include "sdattr.hxx"
#ifndef _AEITEM_HXX //autogen
#include <svtools/aeitem.hxx>
#endif
#ifndef _EEITEM_HXX //autogen
#include <svx/eeitem.hxx>
#endif
#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif
#ifndef _SVDITER_HXX //autogen
#include <svx/svditer.hxx>
#endif
#ifndef _SVX_COLRITEM_HXX //autogen
#include <svx/colritem.hxx>
#endif
#ifndef _SFXVIEWFRM_HXX //autogen
#include <sfx2/viewfrm.hxx>
#endif
#ifndef _SVXIDS_HRC
#include <svx/svxids.hrc>
#endif
#ifndef _SVDUNDO_HXX //autogen
#include <svx/svdundo.hxx>
#endif
#ifndef _E3D_VIEW3D_HXX //autogen
#include <svx/view3d.hxx>
#endif

#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif

#ifndef _SVX_FLOAT3D_HXX
#include <svx/float3d.hxx>
#endif
#ifndef _SVX_F3DCHILD_HXX
#include <svx/f3dchild.hxx>
#endif
#ifndef _SVX_DIALOGS_HRC //autogen
#include <svx/dialogs.hrc>
#endif

#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif


#include "app.hrc"
#include "strings.hrc"

#include "drawdoc.hxx"
#include "DrawDocShell.hxx"
#include "anminfo.hxx"
#include "unoaprms.hxx"                 // Undo-Action
#include "sdundogr.hxx"                 // Undo Gruppe
#ifndef SD_DRAW_VIEW_HXX
#include "drawview.hxx"
#endif
#ifndef SD_WINDOW_HXX
#include "Window.hxx"
#endif
#include "sdresid.hxx"

using namespace ::com::sun::star;

namespace sd {

#define ATTR_MISSING    0       // Attribut nicht verfuegbar
#define ATTR_MIXED      1       // Attribut uneindeutig (bei Mehrfachselektion)
#define ATTR_SET        2       // Attribut eindeutig

#define ITEMVALUE(ItemSet,Id,Cast)  ((const Cast&)(ItemSet).Get(Id)).GetValue()

/*************************************************************************
|*
|* SfxRequests fuer EffekteWindow bearbeiten
|*
\************************************************************************/

void DrawViewShell::ExecEffectWin( SfxRequest& rReq )
{
    CheckLineTo (rReq);

    USHORT nSId = rReq.GetSlot();

    switch( nSId )
    {
        case SID_3D_INIT:
        {
            USHORT nId = Svx3DChildWindow::GetChildWindowId();
            SfxChildWindow* pWindow = GetViewFrame()->GetChildWindow( nId );
            if( pWindow )
            {
                Svx3DWin* p3DWin = (Svx3DWin*)( pWindow->GetWindow() );
                if( p3DWin )
                    p3DWin->InitColorLB( GetDoc() );
            }
        }
        break;

        case SID_3D_STATE:
        {
            Update3DWindow();
        }
        break;

        case SID_3D_ASSIGN:
        {
            AssignFrom3DWindow();
        }
        break;

    }
}

/*************************************************************************
|*
|* 3D - Assign / Update
|*
\************************************************************************/
void DrawViewShell::Update3DWindow()
{
    USHORT nId = Svx3DChildWindow::GetChildWindowId();
    SfxChildWindow* pWindow = GetViewFrame()->GetChildWindow( nId );
    if( pWindow )
    {
        Svx3DWin* p3DWin = (Svx3DWin*) pWindow->GetWindow();
        if( p3DWin && p3DWin->IsUpdateMode() )
        {
            SfxItemSet aTmpItemSet = GetView()->Get3DAttributes();
            p3DWin->Update( aTmpItemSet );
        }
    }
}

/*----------------------------------------------------------------------------*/

#pragma optimize ( "", off )

void DrawViewShell::AssignFrom3DWindow()
{
    USHORT nId = Svx3DChildWindow::GetChildWindowId();
    SfxChildWindow* pWin = GetViewFrame()->GetChildWindow( nId );
    if( pWin )
    {
        Svx3DWin* p3DWin = (Svx3DWin*) pWin->GetWindow();
        if( p3DWin && GetView() )
        {
            if(!GetView()->IsPresObjSelected())
            {
                SfxItemSet aSet( GetDoc()->GetPool(),
                    SDRATTR_START,  SDRATTR_END,
                    0, 0);
                p3DWin->GetAttr( aSet );

                // Eigene UNDO-Klammerung auch um die Wandlung in 3D
                GetView()->BegUndo(String(SdResId(STR_UNDO_APPLY_3D_FAVOURITE)));

                if(GetView()->IsConvertTo3DObjPossible())
                {
                    // Nur TextAttribute zuweisen
                    SfxItemSet aTextSet( GetDoc()->GetPool(),
                        EE_ITEMS_START, EE_ITEMS_END, 0 );
                    aTextSet.Put( aSet, FALSE );
                    GetView()->SetAttributes( aTextSet );

                    // Text in 3D umwandeln
                    USHORT nSId = SID_CONVERT_TO_3D;
                    SfxBoolItem aItem( nSId, TRUE );
                    GetViewFrame()->GetDispatcher()->Execute(
                        nSId, SFX_CALLMODE_SYNCHRON | SFX_CALLMODE_RECORD, &aItem, 0L );

                    // Feststellen, ob ein FILL_Attribut gesetzt ist.
                    // Falls nicht, Fuellattribut hart setzen
                    XFillStyle eFillStyle = ITEMVALUE( aSet, XATTR_FILLSTYLE, XFillStyleItem );
                    if(eFillStyle == XFILL_NONE)
                        aSet.Put(XFillStyleItem (XFILL_SOLID));

                    // remove some 3DSCENE attributes since these were
                    // created by convert to 3D and may not be changed
                    // to the defaults again.
                    aSet.ClearItem(SDRATTR_3DSCENE_DISTANCE);
                    aSet.ClearItem(SDRATTR_3DSCENE_FOCAL_LENGTH);
                    aSet.ClearItem(SDRATTR_3DOBJ_DEPTH);
                }

                // Attribute zuweisen
                GetView()->Set3DAttributes( aSet );

                // Ende UNDO
                GetView()->EndUndo();
            }
            else
            {
                InfoBox aInfoBox (
                    GetActiveWindow(),
                    String(SdResId(STR_ACTION_NOTPOSSIBLE)));
                aInfoBox.Execute();
            }

            // Focus zurueckholen
            GetActiveWindow()->GrabFocus();
        }
    }
}

#pragma optimize ( "", on )

}
