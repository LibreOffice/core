/*************************************************************************
 *
 *  $RCSfile: verttexttbxctrl.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-27 15:05:01 $
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

SFX_IMPL_TOOLBOX_CONTROL(SvxCTLTextTbxCtrl, SfxBoolItem);
SFX_IMPL_TOOLBOX_CONTROL(SvxVertTextTbxCtrl, SfxBoolItem);
/* -----------------------------27.04.01 15:50--------------------------------

 ---------------------------------------------------------------------------*/
SvxVertCTLTextTbxCtrl::SvxVertCTLTextTbxCtrl( USHORT nId, ToolBox& rTbx, SfxBindings& rBindings ) :
    SfxToolBoxControl( nId, rTbx, rBindings ),
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
void SvxVertCTLTextTbxCtrl::StateChanged( USHORT nSID, SfxItemState eState,
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

