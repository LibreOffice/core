/*************************************************************************
 *
 *  $RCSfile: tbxform.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: fs $ $Date: 2000-09-21 12:31:31 $
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
#include <string> // HACK: prevent conflict between STLPORT and Workshop headers

#ifndef _REF_HXX
#include <tools/ref.hxx>
#endif
#ifndef _SHL_HXX
#include <tools/shl.hxx>
#endif
#ifndef _SFXINTITEM_HXX //autogen
#include <svtools/intitem.hxx>
#endif
#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif
#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SV_TOOLBOX_HXX //autogen
#include <vcl/toolbox.hxx>
#endif
#ifndef _SV_FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif
#ifndef _SVX_FMITEMS_HXX //autogen
#include "fmitems.hxx"
#endif

#ifndef _TOOLS_INTN_HXX //autogen
#include <tools/intn.hxx>
#endif

#ifndef _SV_SOUND_HXX //autogen
#include <vcl/sound.hxx>
#endif

#pragma hdrstop

#ifndef _SVX_DIALMGR_HXX
#include "dialmgr.hxx"
#endif
#ifndef _SVX_DIALOGS_HRC
#include "dialogs.hrc"
#endif

#ifndef _TBXCTL_HXX
#include "tbxctl.hxx"
#endif
#ifndef _TBXDRAW_HXX
#include "tbxdraw.hxx"
#endif
#ifndef _SVX_TBXFORM_HXX
#include "tbxform.hxx"
#endif
#ifndef _SVX_FMRESIDS_HRC
#include "fmresids.hrc"
#endif
#ifndef _SVX_FMITEMS_HXX
#include "fmitems.hxx"
#endif

//========================================================================
// class FmInputRecordNoDialog
//========================================================================

FmInputRecordNoDialog::FmInputRecordNoDialog(Window * pParent)
    :ModalDialog( pParent, SVX_RES(RID_SVX_DLG_INPUTRECORDNO))
    ,m_aLabel(this, ResId(1))
    ,m_aRecordNo(this, ResId(1))
    ,m_aOk(this, ResId(1))
    ,m_aCancel(this, ResId(1))
{
    m_aRecordNo.SetMin(1);
    m_aRecordNo.SetMax(0x7FFFFFFF);
    m_aRecordNo.SetStrictFormat(TRUE);
    m_aRecordNo.SetDecimalDigits(0);

    FreeResource();
}

//========================================================================
// class SvxFmAbsRecWin
//========================================================================

// -----------------------------------------------------------------------
SvxFmAbsRecWin::SvxFmAbsRecWin( Window* _pParent, SfxToolBoxControl* _pController )
    :NumericField( _pParent, WB_BORDER )
    ,m_pController(_pController)
{
    SetMin(1);
    SetFirst(1);
    SetSpinSize(1);
    SetSizePixel( Size(70,19) );

    International rInt = GetInternational();
    rInt.EnableNumThousandSep(FALSE);
    SetInternational(rInt);
    SetDecimalDigits(0);
    SetStrictFormat(TRUE);
}

// -----------------------------------------------------------------------
SvxFmAbsRecWin::~SvxFmAbsRecWin()
{
}

// -----------------------------------------------------------------------
void SvxFmAbsRecWin::FirePosition()
{
    INT32 nRecord = GetValue();
    if (nRecord < GetMin() || nRecord > GetMax())
    {
        Sound::Beep();
        return;
    }

    FmFormInfoItem aItem( SID_FM_RECORD_ABSOLUTE, FmFormInfo(nRecord,-1));

    m_pController->GetBindings().GetDispatcher()->Execute( SID_FM_RECORD_ABSOLUTE, SFX_CALLMODE_RECORD, &aItem, 0L );

    // to update our content we explicitly call StateChanged : a simple Invalidate(m_nId) is insufficient
    // as our StateChanged won't be called if entered a invalid position which didn't cause the cursor
    // to be moved.
    SfxPoolItem* pState = NULL;
    SfxItemState eState = m_pController->GetBindings().QueryState(m_pController->GetId(), pState);
    ((SfxControllerItem*)m_pController)->StateChanged(m_pController->GetId(), eState, pState);
    delete pState;
}

// -----------------------------------------------------------------------
void SvxFmAbsRecWin::LoseFocus()
{
    FirePosition();
}

// -----------------------------------------------------------------------
void SvxFmAbsRecWin::KeyInput( const KeyEvent& rKeyEvent )
{
    if( rKeyEvent.GetKeyCode() == KEY_RETURN && GetText().Len() )
        FirePosition();
    else
        NumericField::KeyInput( rKeyEvent );
}


//========================================================================
// class SvxFmConfigWin
//========================================================================
DBG_NAME(SvxFmConfigWin);
SvxFmConfigWin::SvxFmConfigWin( USHORT nId, ResId aRIdWin, ResId aRIdTbx, SfxBindings& rBindings )
    :SfxPopupWindow( nId, aRIdWin, rBindings )
    ,aTbx( this, GetBindings(), aRIdTbx )
{
    DBG_CTOR(SvxFmConfigWin,NULL);
    aTbx.Initialize();

    ToolBox& rBox = aTbx.GetToolBox();
    aSelectLink = rBox.GetSelectHdl();
    rBox.SetSelectHdl( LINK(this, SvxFmConfigWin, TbxSelectHdl) );
    Size aSize = aTbx.CalcWindowSizePixel();
    rBox.SetSizePixel( aSize );
    SetOutputSizePixel( aSize );
    FreeResource();
}

// -----------------------------------------------------------------------
SvxFmConfigWin::~SvxFmConfigWin()
{
    DBG_DTOR(SvxFmConfigWin,NULL);
}

// -----------------------------------------------------------------------
void SvxFmConfigWin::PopupModeEnd()
{
    aTbx.GetToolBox().EndSelection();
    SfxPopupWindow::PopupModeEnd();
}

// -----------------------------------------------------------------------
void SvxFmConfigWin::Update()
{
    ToolBox *pBox = &aTbx.GetToolBox();
    aTbx.Activate( pBox );
    aTbx.Deactivate( pBox );
}

//-----------------------------------------------------------------------
IMPL_LINK( SvxFmConfigWin, TbxSelectHdl, ToolBox*, pBox )
{
    if ( IsInPopupMode() )
        EndPopupMode();

    aSelectLink.Call( &aTbx.GetToolBox() );

    return 0;
}

//========================================================================
// class SvxFmTbxCtlConfig
//========================================================================

SFX_IMPL_TOOLBOX_CONTROL( SvxFmTbxCtlConfig, SfxUInt16Item );

//-----------------------------------------------------------------------
SvxFmTbxCtlConfig::SvxFmTbxCtlConfig( USHORT nId, ToolBox& rTbx, SfxBindings& rBindings)
    :SfxToolBoxControl( nId, rTbx, rBindings )
    ,nLastSlot( 0 )
{
}

//-----------------------------------------------------------------------
void SvxFmTbxCtlConfig::StateChanged(USHORT nSID, SfxItemState eState, const SfxPoolItem* pState )
{
    if (nSID == SID_FM_CONFIG)
    {
        UINT16 nSlot   = 0;
        if (eState >= SFX_ITEM_AVAILABLE)
            nSlot = ((SfxUInt16Item*)pState)->GetValue();

        //////////////////////////////////////////////////////////////////////
        // Ist dieser Slot ein gueltiger Slot?
        BOOL bValidSlot = FALSE;

        switch( nSlot )
        {
            case SID_FM_PUSHBUTTON:
            case SID_FM_RADIOBUTTON:
            case SID_FM_CHECKBOX:
            case SID_FM_FIXEDTEXT:
            case SID_FM_GROUPBOX:
            case SID_FM_LISTBOX:
            case SID_FM_COMBOBOX:
            case SID_FM_URLBUTTON:
            case SID_FM_EDIT:
            case SID_FM_DBGRID:
            case SID_FM_IMAGEBUTTON:
            case SID_FM_IMAGECONTROL:
            case SID_FM_FILECONTROL:
            case SID_FM_DATEFIELD:
            case SID_FM_TIMEFIELD:
            case SID_FM_NUMERICFIELD:
            case SID_FM_CURRENCYFIELD:
            case SID_FM_PATTERNFIELD:
            case SID_FM_DESIGN_MODE:
                bValidSlot = TRUE;
        }
        //////////////////////////////////////////////////////////////////////
        // Setzen des Images
        if (bValidSlot)
        {
            //  Image aImage = SFX_IMAGEMANAGER()->GetImage( nSlot );
            GetToolBox().SetItemImage( SID_FM_CONFIG, SFX_IMAGEMANAGER()->GetImage( nSlot ) );
            nLastSlot = nSlot;
        }
    }
    SfxToolBoxControl::StateChanged( nSID, eState,pState );
}

//-----------------------------------------------------------------------
SfxPopupWindowType SvxFmTbxCtlConfig::GetPopupWindowType() const
{
    return( nLastSlot == 0 ? SFX_POPUPWINDOW_ONCLICK : SFX_POPUPWINDOW_ONTIMEOUT );
}

//-----------------------------------------------------------------------
SfxPopupWindow* SvxFmTbxCtlConfig::CreatePopupWindow()
{
    if ( GetId() == SID_FM_CONFIG )
    {
        SvxFmConfigWin* pWin = new SvxFmConfigWin( GetId(), SVX_RES( RID_SVXTBX_FORM ),
                                  SVX_RES( TBX_FORM ), GetBindings() );
        pWin->StartPopupMode( &GetToolBox(), TRUE );
        pWin->StartSelection();
        pWin->Show();

        return pWin;
    }
    return NULL;
}

//-----------------------------------------------------------------------
void SvxFmTbxCtlConfig::Select( USHORT nModifier )
{
    //////////////////////////////////////////////////////////////////////
    // Click auf den Button SID_FM_CONFIG in der ObjectBar
    if (nLastSlot)
        GetBindings().GetDispatcher()->Execute( nLastSlot );
}


//========================================================================
// class SvxFmTbxCtlAbsRec
//========================================================================

SFX_IMPL_TOOLBOX_CONTROL( SvxFmTbxCtlAbsRec, FmFormInfoItem );
DBG_NAME(SvxFmTbxCtlAbsRec);
//-----------------------------------------------------------------------
SvxFmTbxCtlAbsRec::SvxFmTbxCtlAbsRec( USHORT nId, ToolBox& rTbx, SfxBindings& rBindings )
    :SfxToolBoxControl( nId, rTbx, rBindings )
{
    DBG_CTOR(SvxFmTbxCtlAbsRec,NULL);
}

//-----------------------------------------------------------------------
SvxFmTbxCtlAbsRec::~SvxFmTbxCtlAbsRec()
{
    DBG_DTOR(SvxFmTbxCtlAbsRec,NULL);
}

//-----------------------------------------------------------------------
void SvxFmTbxCtlAbsRec::StateChanged( USHORT nSID, SfxItemState eState, const SfxPoolItem* pState )
{
    USHORT              nId = GetId();
    ToolBox*            pToolBox = &GetToolBox();
    SvxFmAbsRecWin*     pWin = (SvxFmAbsRecWin*)( pToolBox->GetItemWindow(nId) );

    DBG_ASSERT( pWin, "Control not found!" );

    if (pState)
    {
        FmFormInfo aInfo = ((FmFormInfoItem*)pState)->GetInfo();
        if (aInfo.Count > 0)
        {
            pWin->SetMax(aInfo.Count);
        }
        else
        {
            pWin->SetMax(LONG_MAX);
        }
        pWin->SetValue(aInfo.Pos);
        pWin->SetReadOnly(aInfo.ReadOnly);
    }

    BOOL bEnable = SFX_ITEM_DISABLED != eState && pState;
    if (!bEnable)
        pWin->SetText(String());

    //////////////////////////////////////////////////////////////////////
    // Enablen/disablen des Fensters
    pToolBox->EnableItem(nId, bEnable);
    SfxToolBoxControl::StateChanged( nSID, eState,pState );
}

//-----------------------------------------------------------------------
Window* SvxFmTbxCtlAbsRec::CreateItemWindow( Window* pParent )
{
    SvxFmAbsRecWin* pWin = new SvxFmAbsRecWin( pParent, this );
    return pWin;
}


//========================================================================
// SvxFmTbxCtlRecText
//========================================================================

SFX_IMPL_TOOLBOX_CONTROL( SvxFmTbxCtlRecText, SfxBoolItem );
DBG_NAME(SvxFmTbxCtlRecText);
//-----------------------------------------------------------------------
SvxFmTbxCtlRecText::SvxFmTbxCtlRecText( USHORT nId, ToolBox& rTbx, SfxBindings& rBindings )
    :SfxToolBoxControl( nId, rTbx, rBindings )
{
    DBG_CTOR(SvxFmTbxCtlRecText,NULL);
}

//-----------------------------------------------------------------------
SvxFmTbxCtlRecText::~SvxFmTbxCtlRecText()
{
    DBG_DTOR(SvxFmTbxCtlRecText,NULL);
}

//-----------------------------------------------------------------------
Window* SvxFmTbxCtlRecText::CreateItemWindow( Window* pParent )
{
    XubString aText( SVX_RES(RID_STR_REC_TEXT) );
    FixedText* pFixedText = new FixedText( pParent );
    Size aSize( pFixedText->GetTextWidth( aText ), pFixedText->GetTextHeight( ) );
    pFixedText->SetText( aText );
    aSize.Width() += 6;
    pFixedText->SetSizePixel( aSize );

    return pFixedText;
}


//========================================================================
// SvxFmTbxCtlRecFromText
//========================================================================

SFX_IMPL_TOOLBOX_CONTROL( SvxFmTbxCtlRecFromText, SfxBoolItem );
DBG_NAME(SvxFmTbxCtlRecFromText);
//-----------------------------------------------------------------------
SvxFmTbxCtlRecFromText::SvxFmTbxCtlRecFromText( USHORT nId, ToolBox& rTbx, SfxBindings& rBindings )
    :SfxToolBoxControl( nId, rTbx, rBindings )
{
    DBG_CTOR(SvxFmTbxCtlRecFromText,NULL);
}

//-----------------------------------------------------------------------
SvxFmTbxCtlRecFromText::~SvxFmTbxCtlRecFromText()
{
    DBG_DTOR(SvxFmTbxCtlRecFromText,NULL);
}

//-----------------------------------------------------------------------
Window* SvxFmTbxCtlRecFromText::CreateItemWindow( Window* pParent )
{
    XubString aText( SVX_RES(RID_STR_REC_FROM_TEXT) );
    FixedText* pFixedText = new FixedText( pParent, WB_CENTER );
    Size aSize( pFixedText->GetTextWidth( aText ), pFixedText->GetTextHeight( ) );
    aSize.Width() += 12;
    pFixedText->SetText( aText );
    pFixedText->SetSizePixel( aSize );

    return pFixedText;
}


//========================================================================
// SvxFmTbxCtlRecTotal
//========================================================================
DBG_NAME(SvxFmTbxCtlRecTotal);
SFX_IMPL_TOOLBOX_CONTROL( SvxFmTbxCtlRecTotal, SfxStringItem );

//-----------------------------------------------------------------------
SvxFmTbxCtlRecTotal::SvxFmTbxCtlRecTotal( USHORT nId, ToolBox& rTbx, SfxBindings& rBindings )
    :SfxToolBoxControl( nId, rTbx, rBindings )
    ,pFixedText( NULL )
{
    DBG_CTOR(SvxFmTbxCtlRecTotal,NULL);
}

//-----------------------------------------------------------------------
SvxFmTbxCtlRecTotal::~SvxFmTbxCtlRecTotal()
{
    DBG_DTOR(SvxFmTbxCtlRecTotal,NULL);
}

//-----------------------------------------------------------------------
Window* SvxFmTbxCtlRecTotal::CreateItemWindow( Window* pParent )
{
    pFixedText = new FixedText( pParent );
    String aSample("123456", sizeof("123456"));
    Size aSize( pFixedText->GetTextWidth( aSample ), pFixedText->GetTextHeight( ) );
    aSize.Width() += 12;
    pFixedText->SetSizePixel( aSize );
    return pFixedText;
}

//-----------------------------------------------------------------------
void SvxFmTbxCtlRecTotal::StateChanged( USHORT nSID, SfxItemState eState, const SfxPoolItem* pState )
{
    //////////////////////////////////////////////////////////////////////
    // Setzen des FixedTextes
    if (GetId() != SID_FM_RECORD_TOTAL)
        return;

    XubString aText;
    if (pState)
        aText = ((SfxStringItem*)pState)->GetValue();
    else
        aText = '?';

    pFixedText->SetText( aText );
    pFixedText->Update();
    pFixedText->Flush();

    SfxToolBoxControl::StateChanged( nSID, eState,pState );
}

/*
//========================================================================
// SvxFmTbxCtlRecTotal
//========================================================================
SFX_IMPL_TOOLBOX_CONTROL( SvxFmTbxCtlFilterText, SfxBoolItem );

//-----------------------------------------------------------------------
SvxFmTbxCtlFilterText::SvxFmTbxCtlFilterText( USHORT nId, ToolBox& rTbx, SfxBindings& rBindings )
    :SfxToolBoxControl( nId, rTbx, rBindings )
    ,pFixedText( NULL )
    ,aText(SVX_RES(RID_STR_FORM_FILTERED))
{
}

//-----------------------------------------------------------------------
SvxFmTbxCtlFilterText::~SvxFmTbxCtlFilterText()
{
}

//-----------------------------------------------------------------------
Window* SvxFmTbxCtlFilterText::CreateItemWindow( Window* pParent )
{
    pFixedText = new FixedText( pParent );
    Size aSize = pFixedText->GetTextSize( aText );
    aSize.Width() += 6;
    pFixedText->SetSizePixel( aSize );

    return pFixedText;
}

//-----------------------------------------------------------------------
void SvxFmTbxCtlFilterText::StateChanged( USHORT nSID, SfxItemState eState, const SfxPoolItem* pState )
{
    //////////////////////////////////////////////////////////////////////
    // Setzen des FixedTextes
    if (GetId() != SID_FM_FORM_FILTERED)
        return;

    if (pState && ((SfxBoolItem*)pState)->GetValue())
        pFixedText->SetText( aText );
    else
        pFixedText->SetText( "" );

    SfxToolBoxControl::StateChanged( nSID, eState,pState );
}     */

//========================================================================
// SvxFmTbxNextRec
//========================================================================
SFX_IMPL_TOOLBOX_CONTROL( SvxFmTbxNextRec, SfxBoolItem );

//-----------------------------------------------------------------------
SvxFmTbxNextRec::SvxFmTbxNextRec( USHORT nId, ToolBox& rTbx, SfxBindings& rBindings )
    :SfxToolBoxControl( nId, rTbx, rBindings )
{
    rTbx.SetItemBits(nId, rTbx.GetItemBits(nId) | TIB_REPEAT);

    AllSettings aSettings = rTbx.GetSettings();
    MouseSettings aMouseSettings = aSettings.GetMouseSettings();
    aMouseSettings.SetButtonRepeat(aMouseSettings.GetButtonRepeat() / 4);
    aSettings.SetMouseSettings(aMouseSettings);
    rTbx.SetSettings(aSettings, TRUE);
}

//========================================================================
// SvxFmTbxPrevRec
//========================================================================
SFX_IMPL_TOOLBOX_CONTROL( SvxFmTbxPrevRec, SfxBoolItem );

//-----------------------------------------------------------------------
SvxFmTbxPrevRec::SvxFmTbxPrevRec( USHORT nId, ToolBox& rTbx, SfxBindings& rBindings )
    :SfxToolBoxControl( nId, rTbx, rBindings )
{
    rTbx.SetItemBits(nId, rTbx.GetItemBits(nId) | TIB_REPEAT);

    /*AllSettings   aSettings = rTbx.GetSettings();
    MouseSettings aMouseSettings = aSettings.GetMouseSettings();
    aMouseSettings.SetButtonRepeat(aMouseSettings.GetButtonRepeat() / 2);
    aSettings.SetMouseSettings(aMouseSettings);
    rTbx.SetSettings(aSettings, TRUE);*/
}


