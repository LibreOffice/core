/*************************************************************************
 *
 *  $RCSfile: tplcitem.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:52:31 $
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

// INCLUDE ---------------------------------------------------------------

#include <svtools/intitem.hxx>

#pragma hdrstop

#include "templdlg.hxx"
#include "bindings.hxx"
#include "tplpitem.hxx"
#include "tplcitem.hxx"
#include "templdgi.hxx"

#include "sfx.hrc"
#include "dialog.hrc"

// STATIC DATA -----------------------------------------------------------

// Konstruktor

SfxTemplateControllerItem::SfxTemplateControllerItem(
        USHORT nId,                 // ID
        SfxCommonTemplateDialog_Impl &rDlg, // Controller-Instanz, dem dieses Item zugeordnet ist.
        SfxBindings &rBindings):
    SfxControllerItem(nId, rBindings),
    rTemplateDlg(rDlg)
{
}

// -----------------------------------------------------------------------

// Benachrichtigung "uber Status"anderung; wird an den
// im Konstruktor "ubergebenen Controller propagiert

void SfxTemplateControllerItem::StateChanged( USHORT nSID, SfxItemState eState,
                                              const SfxPoolItem* pItem )
{
    switch(nSID)
    {
        case SID_STYLE_FAMILY1:
        case SID_STYLE_FAMILY2:
        case SID_STYLE_FAMILY3:
        case SID_STYLE_FAMILY4:
        case SID_STYLE_FAMILY5:
        {
            FASTBOOL bAvailable = SFX_ITEM_AVAILABLE == eState;
            if ( !bAvailable )
                rTemplateDlg.SetFamilyState(GetId(), 0);
            else {
                const SfxTemplateItem *pStateItem = PTR_CAST(
                    SfxTemplateItem, pItem);
                DBG_ASSERT(pStateItem != 0, "SfxTemplateItem erwartet");
                rTemplateDlg.SetFamilyState( GetId(), pStateItem );
            }
            BOOL bDisable = eState == SFX_ITEM_DISABLED;
            // Familie Disablen
            USHORT nFamily;
            switch( GetId())
            {
                case SID_STYLE_FAMILY1:
                    nFamily = 1; break;
                case SID_STYLE_FAMILY2:
                    nFamily = 2; break;
                case SID_STYLE_FAMILY3:
                    nFamily = 3; break;
                case SID_STYLE_FAMILY4:
                    nFamily = 4; break;
                case SID_STYLE_FAMILY5:
                    nFamily = 5; break;
                default: DBG_ERROR("unbekannte StyleFamily"); break;
            }
            rTemplateDlg.EnableFamilyItem( nFamily, !bDisable );
            break;
        }
        case SID_STYLE_WATERCAN:
        {
            if ( eState == SFX_ITEM_DISABLED )
                rTemplateDlg.SetWaterCanState(0);
            else if( eState == SFX_ITEM_AVAILABLE )
            {
                const SfxBoolItem *pStateItem = PTR_CAST(SfxBoolItem, pItem);
                DBG_ASSERT(pStateItem != 0, "BoolItem erwartet");
                rTemplateDlg.SetWaterCanState(pStateItem);
            }
            break;
        }
        case SID_STYLE_EDIT:
            rTemplateDlg.EnableEdit( SFX_ITEM_DISABLED != eState );
            break;
        case SID_STYLE_DELETE:
            rTemplateDlg.EnableDel( SFX_ITEM_DISABLED != eState );
            break;
        case SID_STYLE_NEW_BY_EXAMPLE:

            rTemplateDlg.EnableExample_Impl(
                GetId(), SFX_ITEM_DISABLED != eState );
            break;
        case SID_STYLE_UPDATE_BY_EXAMPLE:
        {
            rTemplateDlg.EnableExample_Impl(
                GetId(), eState != SFX_ITEM_DISABLED );
            // Das Select Disabled dann, falls enabled und Style Readonly
/*          String aStr = rTemplateDlg.GetSelectedEntry();
            if( aStr.Len() ) rTemplateDlg.SelectStyle( aStr ); */
            break;
        }
        case SID_STYLE_NEW:
        {
            rTemplateDlg.EnableNew( SFX_ITEM_DISABLED != eState );
            break;
        }
        case SID_STYLE_DRAGHIERARCHIE:
        {
            rTemplateDlg.EnableTreeDrag( SFX_ITEM_DISABLED != eState );
            break;
        }
        case SID_STYLE_FAMILY :
        {
            const SfxUInt16Item *pStateItem = PTR_CAST( SfxUInt16Item, pItem);
            if (pStateItem)
                rTemplateDlg.SetFamily( pStateItem->GetValue() );
            break;
        }
    }
}


