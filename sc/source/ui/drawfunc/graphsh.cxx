/*************************************************************************
 *
 *  $RCSfile: graphsh.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: nn $ $Date: 2000-10-20 18:25:08 $
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

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

//------------------------------------------------------------------

#include <sfx2/app.hxx>
#include <sfx2/objface.hxx>
#include <sfx2/request.hxx>
#include <svtools/whiter.hxx>
#include <svx/svdograf.hxx>

#include "graphsh.hxx"
#include "sc.hrc"
#include "viewdata.hxx"
#include "drawview.hxx"
#include "scresid.hxx"

#define ScGraphicShell
#include "scslots.hxx"

#define ITEMVALUE(ItemSet,Id,Cast) ((const Cast&)(ItemSet).Get(Id)).GetValue()


SFX_IMPL_INTERFACE(ScGraphicShell, ScDrawShell, ScResId(SCSTR_GRAPHICSHELL) )
{
    SFX_OBJECTBAR_REGISTRATION( SFX_OBJECTBAR_OBJECT|SFX_VISIBILITY_STANDARD|SFX_VISIBILITY_SERVER,
                                ScResId(RID_GRAPHIC_OBJECTBAR) );
    SFX_POPUPMENU_REGISTRATION( ScResId(RID_POPUP_GRAPHIC) );
    SFX_OBJECTMENU_REGISTRATION( SID_OBJECTMENU0, ScResId(RID_OBJECTMENU_DRAW) );
}

TYPEINIT1( ScGraphicShell, ScDrawShell );

ScGraphicShell::ScGraphicShell(ScViewData* pData) :
    ScDrawShell(pData)
{
    SetHelpId(HID_SCSHELL_GRAPHIC);
    SetName(String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("GraphicObject")));
}

ScGraphicShell::~ScGraphicShell()
{
}

void ScGraphicShell::GetAttrState( SfxItemSet& rSet )
{
    SfxItemSet      aAttrSet( GetPool() );
    SfxWhichIter    aIter( rSet );
    USHORT          nWhich = aIter.FirstWhich();

    ScDrawView* pView = GetViewData()->GetScDrawView();
    pView->GetAttributes( aAttrSet );

    while( nWhich )
    {
        USHORT nSlotId = SfxItemPool::IsWhich( nWhich )
                            ? GetPool().GetSlotId( nWhich )
                            : nWhich;

        switch( nSlotId )
        {
            case( SID_ATTR_GRAF_MODE ):
            {
                if( SFX_ITEM_AVAILABLE <= aAttrSet.GetItemState( SDRATTR_GRAFMODE ) )
                {
                    rSet.Put( SfxUInt16Item( nSlotId,
                        ITEMVALUE( aAttrSet, SDRATTR_GRAFMODE, SdrGrafModeItem ) ) );
                }
            }
            break;

            case( SID_ATTR_GRAF_RED ):
            {
                if( SFX_ITEM_AVAILABLE <= aAttrSet.GetItemState( SDRATTR_GRAFRED ) )
                {
                    rSet.Put( SfxInt16Item( nSlotId,
                        ITEMVALUE( aAttrSet, SDRATTR_GRAFRED, SdrGrafRedItem ) ) );
                }
            }
            break;

            case( SID_ATTR_GRAF_GREEN ):
            {
                if( SFX_ITEM_AVAILABLE <= aAttrSet.GetItemState( SDRATTR_GRAFGREEN ) )
                {
                    rSet.Put( SfxInt16Item( nSlotId,
                        ITEMVALUE( aAttrSet, SDRATTR_GRAFGREEN, SdrGrafGreenItem ) ) );
                }
            }
            break;

            case( SID_ATTR_GRAF_BLUE ):
            {
                if( SFX_ITEM_AVAILABLE <= aAttrSet.GetItemState( SDRATTR_GRAFBLUE ) )
                {
                    rSet.Put( SfxInt16Item( nSlotId,
                        ITEMVALUE( aAttrSet, SDRATTR_GRAFBLUE, SdrGrafBlueItem ) ) );
                }
            }
            break;

            case( SID_ATTR_GRAF_LUMINANCE ):
            {
                if( SFX_ITEM_AVAILABLE <= aAttrSet.GetItemState( SDRATTR_GRAFLUMINANCE ) )
                {
                    rSet.Put( SfxInt16Item( nSlotId,
                        ITEMVALUE( aAttrSet, SDRATTR_GRAFLUMINANCE, SdrGrafLuminanceItem ) ) );
                }
            }
            break;

            case( SID_ATTR_GRAF_CONTRAST ):
            {
                if( SFX_ITEM_AVAILABLE <= aAttrSet.GetItemState( SDRATTR_GRAFCONTRAST ) )
                {
                    rSet.Put( SfxInt16Item( nSlotId,
                        ITEMVALUE( aAttrSet, SDRATTR_GRAFCONTRAST, SdrGrafContrastItem ) ) );
                }
            }
            break;

            case( SID_ATTR_GRAF_GAMMA ):
            {
                if( SFX_ITEM_AVAILABLE <= aAttrSet.GetItemState( SDRATTR_GRAFGAMMA ) )
                {
                    rSet.Put( SfxUInt32Item( nSlotId,
                        ITEMVALUE( aAttrSet, SDRATTR_GRAFGAMMA, SdrGrafGamma100Item ) ) );
                }
            }
            break;

            case( SID_ATTR_GRAF_TRANSPARENCE ):
            {
                if( SFX_ITEM_AVAILABLE <= aAttrSet.GetItemState( SDRATTR_GRAFTRANSPARENCE ) )
                {
                    const SdrMarkList&  rMarkList = pView->GetMarkList();
                    BOOL                bEnable = TRUE;

                    for( USHORT i = 0, nCount = rMarkList.GetMarkCount();
                            ( i < nCount ) && bEnable; i++ )
                    {
                        SdrObject* pObj = rMarkList.GetMark( 0 )->GetObj();

                        if( !pObj || !pObj->ISA( SdrGrafObj ) ||
                            ( (SdrGrafObj*) pObj )->HasGDIMetaFile() ||
                            ( (SdrGrafObj*) pObj )->IsAnimated() )
                        {
                            bEnable = FALSE;
                        }
                    }

                    if( bEnable )
                        rSet.Put( SfxUInt16Item( nSlotId,
                            ITEMVALUE( aAttrSet, SDRATTR_GRAFTRANSPARENCE, SdrGrafTransparenceItem ) ) );
                    else
                        rSet.DisableItem( SID_ATTR_GRAF_TRANSPARENCE );
                }
            }
            break;

            default:
            break;
        }

        nWhich = aIter.NextWhich();
    }
}

void ScGraphicShell::Execute( SfxRequest& rReq )
{
    SfxItemSet aSet( GetPool(), SDRATTR_GRAF_FIRST, SDRATTR_GRAF_LAST-1 );

    const SfxItemSet*   pArgs = rReq.GetArgs();
    const SfxPoolItem*  pItem;
    USHORT              nSlot = rReq.GetSlot();
    BOOL                bGeometryChanged = FALSE;

    if( !pArgs || SFX_ITEM_SET != pArgs->GetItemState( nSlot, FALSE, &pItem ))
        pItem = 0;

    switch( nSlot )
    {
        case SID_ATTR_GRAF_RED:
            if( pItem )
                aSet.Put( SdrGrafRedItem( ((SfxInt16Item*)pItem)->GetValue() ));
        break;

        case SID_ATTR_GRAF_GREEN:
            if( pItem )
                aSet.Put( SdrGrafGreenItem( ((SfxInt16Item*)pItem)->GetValue() ));
        break;

        case SID_ATTR_GRAF_BLUE:
            if( pItem )
                aSet.Put( SdrGrafBlueItem( ((SfxInt16Item*)pItem)->GetValue() ));
        break;

        case SID_ATTR_GRAF_LUMINANCE:
            if( pItem )
                aSet.Put( SdrGrafLuminanceItem( ((SfxInt16Item*)pItem)->GetValue() ));
        break;

        case SID_ATTR_GRAF_CONTRAST:
            if( pItem )
                aSet.Put( SdrGrafContrastItem( ((SfxInt16Item*)pItem)->GetValue() ));
        break;

        case SID_ATTR_GRAF_GAMMA:
            if( pItem )
                aSet.Put( SdrGrafGamma100Item( ((SfxUInt32Item*)pItem)->GetValue() ));
        break;

        case SID_ATTR_GRAF_TRANSPARENCE:
            if( pItem )
                aSet.Put( SdrGrafTransparenceItem( ((SfxUInt16Item*)pItem)->GetValue() ));
        break;

        case SID_ATTR_GRAF_MODE:
            if( pItem )
                aSet.Put( SdrGrafModeItem( (GraphicDrawMode)
                                        ((SfxUInt16Item*)pItem)->GetValue() ));
        break;

        default:
            break;
    }

    if( aSet.Count() )
    {
        ScDrawView* pView = GetViewData()->GetScDrawView();
        pView->SetAttributes( aSet );
    }

    Invalidate();
}




