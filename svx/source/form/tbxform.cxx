/*************************************************************************
 *
 *  $RCSfile: tbxform.cxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: obo $ $Date: 2004-07-06 13:15:11 $
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
//#include <tools/intn.hxx>
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
#ifndef _SVX_FMHELP_HRC
#include "fmhelp.hrc"
#endif

#ifndef _SFXVIEWFRM_HXX
#include <sfx2/viewfrm.hxx>
#endif
#ifndef _SFXVIEWSH_HXX
#include <sfx2/viewsh.hxx>
#endif
#include <sfx2/imagemgr.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;


/* //CHINA001
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
*/ //CHINA001
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

    SetDecimalDigits(0);
    SetStrictFormat(TRUE);
}

// -----------------------------------------------------------------------
SvxFmAbsRecWin::~SvxFmAbsRecWin()
{
}

// -----------------------------------------------------------------------
void SvxFmAbsRecWin::FirePosition( sal_Bool _bForce )
{
    if ( _bForce || ( GetText() != GetSavedValue() ) )
    {
        INT32 nRecord = GetValue();
        if (nRecord < GetMin() || nRecord > GetMax())
        {
            Sound::Beep();
            return;
        }

        SfxInt32Item aPositionParam( FN_PARAM_1, nRecord );

        Any a;
        Sequence< PropertyValue > aArgs( 1 );
        aArgs[0].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Position" ));
        aPositionParam.QueryValue( a );
        aArgs[0].Value = a;
        m_pController->Dispatch( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:AbsoluteRecord" )),
                                 aArgs );
//        m_pController->GetBindings().GetDispatcher()->Execute( SID_FM_RECORD_ABSOLUTE, SFX_CALLMODE_RECORD, &aPositionParam, 0L );

        // to update our content we explicitly call StateChanged : a simple Invalidate(m_nId) is insufficient
        // as our StateChanged won't be called if entered a invalid position which didn't cause the cursor
        // to be moved.

//      SfxPoolItem* pState = NULL;
//      SfxItemState eState = m_pController->GetBindings().QueryState(m_pController->GetId(), pState);
//      ((SfxControllerItem*)m_pController)->StateChanged(m_pController->GetSlotId(), eState, pState);
        m_pController->updateStatus();
//      delete pState;

        SaveValue();
    }
}

// -----------------------------------------------------------------------
void SvxFmAbsRecWin::LoseFocus()
{
    FirePosition( sal_False );
}

// -----------------------------------------------------------------------
void SvxFmAbsRecWin::KeyInput( const KeyEvent& rKeyEvent )
{
    if( rKeyEvent.GetKeyCode() == KEY_RETURN && GetText().Len() )
        FirePosition( sal_True );
    else
        NumericField::KeyInput( rKeyEvent );
}

//========================================================================
// class SvxFmTbxCtlConfig
//========================================================================

struct MapSlotToCmd
{
    USHORT      nSlotId;
    const char* pCommand;
};

static MapSlotToCmd SlotToCommands[] =
{
    { SID_FM_PUSHBUTTON,        ".uno:Pushbutton"       },
    { SID_FM_RADIOBUTTON,       ".uno:RadioButton"      },
    { SID_FM_CHECKBOX,          ".uno:CheckBox"         },
    { SID_FM_FIXEDTEXT,         ".uno:Label"            },
    { SID_FM_GROUPBOX,          ".uno:GroupBox"         },
    { SID_FM_LISTBOX,           ".uno:ListBox"          },
    { SID_FM_COMBOBOX,          ".uno:ComboBox"         },
    { SID_FM_EDIT,              ".uno:Edit"             },
    { SID_FM_DBGRID,            ".uno:Grid"             },
    { SID_FM_IMAGEBUTTON,       ".uno:Imagebutton"      },
    { SID_FM_IMAGECONTROL,      ".uno:ImageControl"     },
    { SID_FM_FILECONTROL,       ".uno:FileControl"      },
    { SID_FM_DATEFIELD,         ".uno:DateField"        },
    { SID_FM_TIMEFIELD,         ".uno:TimeField"        },
    { SID_FM_NUMERICFIELD,      ".uno:NumericField"     },
    { SID_FM_CURRENCYFIELD,     ".uno:CurrencyField"    },
    { SID_FM_PATTERNFIELD,      ".uno:PatternField"     },
    { SID_FM_DESIGN_MODE,       ".uno:SwitchControlDesignMode" },
    { SID_FM_FORMATTEDFIELD,    ".uno:FormattedField"   },
    { SID_FM_SCROLLBAR,         ".uno:ScrollBar"        },
    { SID_FM_SPINBUTTON,        ".uno:SpinButton"       },
    { 0,                        ""                      }
};

SFX_IMPL_TOOLBOX_CONTROL( SvxFmTbxCtlConfig, SfxUInt16Item );

//-----------------------------------------------------------------------
SvxFmTbxCtlConfig::SvxFmTbxCtlConfig( USHORT nSlotId, USHORT nId, ToolBox& rTbx )
    : SfxToolBoxControl( nSlotId, nId, rTbx )
    ,nLastSlot( 0 )
{
    rTbx.SetItemBits( nId, TIB_DROPDOWN | rTbx.GetItemBits( nId ) );
}

//-----------------------------------------------------------------------
void SvxFmTbxCtlConfig::StateChanged(USHORT nSID, SfxItemState eState, const SfxPoolItem* pState )
{
    if (nSID == SID_FM_CONFIG)
    {
        UINT16 nSlot   = 0;
        if (eState >= SFX_ITEM_AVAILABLE)
            nSlot = ((SfxUInt16Item*)pState)->GetValue();

        switch( nSlot )
        {
            case SID_FM_PUSHBUTTON:
            case SID_FM_RADIOBUTTON:
            case SID_FM_CHECKBOX:
            case SID_FM_FIXEDTEXT:
            case SID_FM_GROUPBOX:
            case SID_FM_LISTBOX:
            case SID_FM_COMBOBOX:
            case SID_FM_NAVIGATIONBAR:
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
            case SID_FM_FORMATTEDFIELD:
            case SID_FM_SCROLLBAR:
            case SID_FM_SPINBUTTON:
            {   // set a new image, matching to this slot
                rtl::OUString aSlotURL( RTL_CONSTASCII_USTRINGPARAM( "slot:" ));
                aSlotURL += rtl::OUString::valueOf( sal_Int32( nSlot ));
                Image aImage = GetImage( m_xFrame,
                                        aSlotURL,
                                        hasBigImages(),
                                        GetToolBox().GetDisplayBackground().GetColor().IsDark() );

//              SfxViewFrame* pFrame = GetBindings().GetDispatcher()->GetFrame();
//              GetToolBox().SetItemImage( SID_FM_CONFIG, pFrame->GetImageManager()->GetImage( nSlot, GetToolBox().GetDisplayBackground().GetColor().IsDark() ) );
                nLastSlot = nSlot;
            }
            break;
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
    if ( GetSlotId() == SID_FM_CONFIG )
    {
        rtl::OUString aToolBarResStr( RTL_CONSTASCII_USTRINGPARAM( "private:resource/toolbar/formsbar" ));
        createAndPositionSubToolBar( aToolBarResStr );
    }
    return NULL;
}

//-----------------------------------------------------------------------
void SvxFmTbxCtlConfig::Select( USHORT nModifier )
{
    //////////////////////////////////////////////////////////////////////
    // Click auf den Button SID_FM_CONFIG in der ObjectBar
    if ( nLastSlot )
    {
        USHORT n = 0;
        while( SlotToCommands[n].nSlotId > 0 )
        {
            if ( SlotToCommands[n].nSlotId == nLastSlot )
                break;
            n++;
        }

        if ( SlotToCommands[n].nSlotId > 0 )
        {
            Sequence< PropertyValue > aArgs;
            Dispatch( rtl::OUString::createFromAscii( SlotToCommands[n].pCommand ),
                      aArgs );
            //      GetBindings().GetDispatcher()->Execute( nLastSlot );
        }
    }
}


//========================================================================
// class SvxFmTbxCtlAbsRec
//========================================================================

SFX_IMPL_TOOLBOX_CONTROL( SvxFmTbxCtlAbsRec, SfxInt32Item );
DBG_NAME(SvxFmTbxCtlAbsRec);
//-----------------------------------------------------------------------
SvxFmTbxCtlAbsRec::SvxFmTbxCtlAbsRec( USHORT nSlotId, USHORT nId, ToolBox& rTbx )
    :SfxToolBoxControl( nSlotId, nId, rTbx )
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
        const SfxInt32Item* pItem = PTR_CAST( SfxInt32Item, pState );
        DBG_ASSERT( pItem, "SvxFmTbxCtlAbsRec::StateChanged: invalid item!" );
        pWin->SetValue( pItem ? pItem->GetValue() : -1 );
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
    pWin->SetUniqueId( UID_ABSOLUTE_RECORD_WINDOW );
    return pWin;
}


//========================================================================
// SvxFmTbxCtlRecText
//========================================================================

SFX_IMPL_TOOLBOX_CONTROL( SvxFmTbxCtlRecText, SfxBoolItem );
DBG_NAME(SvxFmTbxCtlRecText);
//-----------------------------------------------------------------------
SvxFmTbxCtlRecText::SvxFmTbxCtlRecText( USHORT nSlotId, USHORT nId, ToolBox& rTbx )
    :SfxToolBoxControl( nSlotId, nId, rTbx )
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
SvxFmTbxCtlRecFromText::SvxFmTbxCtlRecFromText( USHORT nSlotId, USHORT nId, ToolBox& rTbx )
    :SfxToolBoxControl( nSlotId, nId, rTbx )
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
SvxFmTbxCtlRecTotal::SvxFmTbxCtlRecTotal( USHORT nSlotId, USHORT nId, ToolBox& rTbx )
    :SfxToolBoxControl( nSlotId, nId, rTbx )
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
    String aSample( "123456", sizeof( "123456" ) - 1 );
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
    if (GetSlotId() != SID_FM_RECORD_TOTAL)
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

//========================================================================
// SvxFmTbxNextRec
//========================================================================
SFX_IMPL_TOOLBOX_CONTROL( SvxFmTbxNextRec, SfxBoolItem );

//-----------------------------------------------------------------------
SvxFmTbxNextRec::SvxFmTbxNextRec( USHORT nSlotId, USHORT nId, ToolBox& rTbx )
    :SfxToolBoxControl( nSlotId, nId, rTbx )
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
SvxFmTbxPrevRec::SvxFmTbxPrevRec( USHORT nSlotId, USHORT nId, ToolBox& rTbx )
    :SfxToolBoxControl( nSlotId, nId, rTbx )
{
    rTbx.SetItemBits(nId, rTbx.GetItemBits(nId) | TIB_REPEAT);
}


