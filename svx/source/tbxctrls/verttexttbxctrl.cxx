/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: verttexttbxctrl.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 06:08:00 $
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
#include "precompiled_svx.hxx"

#include "dialmgr.hxx"
#include "dialogs.hrc"

#ifndef _SVX_VERT_TEXT_TBXCTRL_HXX
#include <verttexttbxctrl.hxx>
#endif
#ifndef _SVTOOLS_LANGUAGEOPTIONS_HXX
#include <svtools/languageoptions.hxx>
#endif
#ifndef _SFXAPP_HXX
#include <sfx2/app.hxx>
#endif
#ifndef _SFXENUMITEM_HXX
#include <svtools/eitem.hxx>
#endif
#ifndef _SV_TOOLBOX_HXX
#include <vcl/toolbox.hxx>
#endif
#include <rtl/ustring.hxx>

SFX_IMPL_TOOLBOX_CONTROL(SvxCTLTextTbxCtrl, SfxBoolItem);
SFX_IMPL_TOOLBOX_CONTROL(SvxVertTextTbxCtrl, SfxBoolItem);

// -----------------------------27.04.01 15:50--------------------------------

SvxCTLTextTbxCtrl::SvxCTLTextTbxCtrl(USHORT nSlotId, USHORT nId, ToolBox& rTbx ) :
    SvxVertCTLTextTbxCtrl( nSlotId, nId, rTbx )
{
    SetVert(FALSE);
    addStatusListener( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:CTLFontState" )));
}

SvxVertTextTbxCtrl::SvxVertTextTbxCtrl( USHORT nSlotId, USHORT nId, ToolBox& rTbx ) :
    SvxVertCTLTextTbxCtrl( nSlotId, nId, rTbx )
{
    SetVert(TRUE);
    addStatusListener( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:VerticalTextState" )));
}

/* ---------------------------------------------------------------------------*/
SvxVertCTLTextTbxCtrl::SvxVertCTLTextTbxCtrl( USHORT nSlotId, USHORT nId, ToolBox& rTbx ) :
    SfxToolBoxControl( nSlotId, nId, rTbx ),
    bCheckVertical(sal_True)
{
}

/* -----------------------------27.04.01 15:53--------------------------------

 ---------------------------------------------------------------------------*/
SvxVertCTLTextTbxCtrl::~SvxVertCTLTextTbxCtrl( )
{
}
/* -----------------------------27.04.01 15:50--------------------------------

 ---------------------------------------------------------------------------*/
void SvxVertCTLTextTbxCtrl::StateChanged(
    USHORT nSID,
    SfxItemState eState,
    const SfxPoolItem* pState )
{
    SvtLanguageOptions aLangOptions;
    BOOL bCalc = sal_False;
    BOOL bVisible = GetToolBox().IsItemVisible(GetId());
    sal_Bool bEnabled = sal_False;
    if ( nSID == SID_VERTICALTEXT_STATE )
        bEnabled = aLangOptions.IsVerticalTextEnabled();
    else if ( nSID == SID_CTLFONT_STATE )
        bEnabled = aLangOptions.IsCTLFontEnabled();
    else
    {
        SfxToolBoxControl::StateChanged(nSID, eState, pState);
        return;
    }

    if(bEnabled)
    {
        if(!bVisible)
        {
            GetToolBox().ShowItem( GetId(), TRUE );
            bCalc = sal_True;
        }
    }
    else if(bVisible)
    {
        GetToolBox().HideItem( GetId() );
        bCalc = sal_True;
    }
    if(bCalc)
    {
        ToolBox& rTbx = GetToolBox();
        Window* pParent = rTbx.GetParent();
        WindowType nWinType = pParent->GetType();
        if(WINDOW_FLOATINGWINDOW == nWinType)
        {
            Size aSize(rTbx.CalcWindowSizePixel());
            rTbx.SetPosSizePixel( Point(), aSize );
            pParent->SetOutputSizePixel( aSize );
        }
    }
}
/* -----------------------------27.04.01 15:50--------------------------------

 ---------------------------------------------------------------------------*/

