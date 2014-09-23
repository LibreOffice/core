/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <string>
#include <svl/intitem.hxx>
#include <svl/eitem.hxx>
#include <svl/stritem.hxx>
#include <sfx2/dispatch.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/fixed.hxx>
#include <vcl/settings.hxx>
#include "fmitems.hxx"
#include "formtoolbars.hxx"


#include <svx/dialmgr.hxx>
#include <svx/dialogs.hrc>
#include "svx/tbxctl.hxx"
#include "tbxform.hxx"
#include "svx/fmresids.hrc"
#include "fmhelp.hrc"
#include <sfx2/viewfrm.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/imagemgr.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::frame;
using ::com::sun::star::beans::XPropertySet;

SvxFmAbsRecWin::SvxFmAbsRecWin( vcl::Window* _pParent, SfxToolBoxControl* _pController )
    :NumericField( _pParent, WB_BORDER )
    ,m_pController(_pController)
{
    SetMin(1);
    SetFirst(1);
    SetSpinSize(1);
    SetSizePixel( Size(70,19) );

    SetDecimalDigits(0);
    SetStrictFormat(true);
}


SvxFmAbsRecWin::~SvxFmAbsRecWin()
{
}


void SvxFmAbsRecWin::FirePosition( bool _bForce )
{
    if ( _bForce || IsValueChangedFromSaved() )
    {
        sal_Int64 nRecord = GetValue();
        if (nRecord < GetMin() || nRecord > GetMax())
        {
            return;
        }

        SfxInt32Item aPositionParam( FN_PARAM_1, static_cast<sal_Int32>(nRecord) );

        Any a;
        Sequence< PropertyValue > aArgs( 1 );
        aArgs[0].Name = "Position";
        aPositionParam.QueryValue( a );
        aArgs[0].Value = a;
        m_pController->Dispatch( OUString( ".uno:AbsoluteRecord" ),
                                 aArgs );
        m_pController->updateStatus();

        SaveValue();
    }
}


void SvxFmAbsRecWin::LoseFocus()
{
    FirePosition( false );
}


void SvxFmAbsRecWin::KeyInput( const KeyEvent& rKeyEvent )
{
    if( rKeyEvent.GetKeyCode() == KEY_RETURN && !GetText().isEmpty() )
        FirePosition( true );
    else
        NumericField::KeyInput( rKeyEvent );
}


// class SvxFmTbxCtlConfig


struct MapSlotToCmd
{
    sal_uInt16      nSlotId;
    const char* pCommand;
};

static const MapSlotToCmd SlotToCommands[] =
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


SvxFmTbxCtlConfig::SvxFmTbxCtlConfig( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx )
    : SfxToolBoxControl( nSlotId, nId, rTbx )
    ,nLastSlot( 0 )
{
    rTbx.SetItemBits( nId, TIB_DROPDOWN | rTbx.GetItemBits( nId ) );
}


void SvxFmTbxCtlConfig::StateChanged(sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState )
{
    if (nSID == SID_FM_CONFIG)
    {
        sal_uInt16 nSlot   = 0;
        if (eState >= SfxItemState::DEFAULT)
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
                OUString aSlotURL = "slot:" + OUString::number( nSlot);
                Image aImage = GetImage( m_xFrame, aSlotURL, hasBigImages() );
                GetToolBox().SetItemImage( GetId(), aImage );
                nLastSlot = nSlot;
            }
            break;
        }
    }
    SfxToolBoxControl::StateChanged( nSID, eState,pState );
}


SfxPopupWindowType SvxFmTbxCtlConfig::GetPopupWindowType() const
{
    return( nLastSlot == 0 ? SFX_POPUPWINDOW_ONCLICK : SFX_POPUPWINDOW_ONTIMEOUT );
}


SfxPopupWindow* SvxFmTbxCtlConfig::CreatePopupWindow()
{
    if ( GetSlotId() == SID_FM_CONFIG )
    {
        ::svxform::FormToolboxes aToolboxes( m_xFrame );
        createAndPositionSubToolBar( aToolboxes.getToolboxResourceName( SID_FM_CONFIG ) );
    }
    return NULL;
}


void SvxFmTbxCtlConfig::Select( sal_uInt16 /*nSelectModifier*/ )
{

    // Click auf den Button SID_FM_CONFIG in der ObjectBar
    if ( nLastSlot )
    {
        sal_uInt16 n = 0;
        while( SlotToCommands[n].nSlotId > 0 )
        {
            if ( SlotToCommands[n].nSlotId == nLastSlot )
                break;
            n++;
        }

        if ( SlotToCommands[n].nSlotId > 0 )
        {
            Sequence< PropertyValue > aArgs;
            Dispatch( OUString::createFromAscii( SlotToCommands[n].pCommand ),
                      aArgs );
        }
    }
}

SFX_IMPL_TOOLBOX_CONTROL( SvxFmTbxCtlAbsRec, SfxInt32Item );

SvxFmTbxCtlAbsRec::SvxFmTbxCtlAbsRec( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx )
    :SfxToolBoxControl( nSlotId, nId, rTbx )
{
}


SvxFmTbxCtlAbsRec::~SvxFmTbxCtlAbsRec()
{
}


void SvxFmTbxCtlAbsRec::StateChanged( sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState )
{
    sal_uInt16              nId = GetId();
    ToolBox*            pToolBox = &GetToolBox();
    SvxFmAbsRecWin*     pWin = (SvxFmAbsRecWin*)( pToolBox->GetItemWindow(nId) );

    assert(pWin && "Control not found!");

    if (pState)
    {
        const SfxInt32Item* pItem = PTR_CAST( SfxInt32Item, pState );
        DBG_ASSERT( pItem, "SvxFmTbxCtlAbsRec::StateChanged: invalid item!" );
        pWin->SetValue( pItem ? pItem->GetValue() : -1 );
    }

    bool bEnable = SfxItemState::DISABLED != eState && pState;
    if (!bEnable)
        pWin->SetText(OUString());


    // Enablen/disablen des Fensters
    pToolBox->EnableItem(nId, bEnable);
    SfxToolBoxControl::StateChanged( nSID, eState,pState );
}


vcl::Window* SvxFmTbxCtlAbsRec::CreateItemWindow( vcl::Window* pParent )
{
    SvxFmAbsRecWin* pWin = new SvxFmAbsRecWin( pParent, this );
    pWin->SetUniqueId( UID_ABSOLUTE_RECORD_WINDOW );
    return pWin;
}

SFX_IMPL_TOOLBOX_CONTROL( SvxFmTbxCtlRecText, SfxBoolItem );

SvxFmTbxCtlRecText::SvxFmTbxCtlRecText( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx )
    :SfxToolBoxControl( nSlotId, nId, rTbx )
{
}


SvxFmTbxCtlRecText::~SvxFmTbxCtlRecText()
{
}


vcl::Window* SvxFmTbxCtlRecText::CreateItemWindow( vcl::Window* pParent )
{
    OUString aText(SVX_RESSTR(RID_STR_REC_TEXT));
    FixedText* pFixedText = new FixedText( pParent );
    Size aSize( pFixedText->GetTextWidth( aText ), pFixedText->GetTextHeight( ) );
    pFixedText->SetText( aText );
    aSize.Width() += 6;
    pFixedText->SetSizePixel( aSize );
    pFixedText->SetBackground(Wallpaper(Color(COL_TRANSPARENT)));

    return pFixedText;
}

SFX_IMPL_TOOLBOX_CONTROL( SvxFmTbxCtlRecFromText, SfxBoolItem );

SvxFmTbxCtlRecFromText::SvxFmTbxCtlRecFromText( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx )
    :SfxToolBoxControl( nSlotId, nId, rTbx )
{
}


SvxFmTbxCtlRecFromText::~SvxFmTbxCtlRecFromText()
{
}


vcl::Window* SvxFmTbxCtlRecFromText::CreateItemWindow( vcl::Window* pParent )
{
    OUString aText(SVX_RESSTR(RID_STR_REC_FROM_TEXT));
    FixedText* pFixedText = new FixedText( pParent, WB_CENTER );
    Size aSize( pFixedText->GetTextWidth( aText ), pFixedText->GetTextHeight( ) );
    aSize.Width() += 12;
    pFixedText->SetText( aText );
    pFixedText->SetSizePixel( aSize );
    pFixedText->SetBackground(Wallpaper(Color(COL_TRANSPARENT)));
    return pFixedText;
}

SFX_IMPL_TOOLBOX_CONTROL( SvxFmTbxCtlRecTotal, SfxStringItem );


SvxFmTbxCtlRecTotal::SvxFmTbxCtlRecTotal( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx )
    :SfxToolBoxControl( nSlotId, nId, rTbx )
    ,pFixedText( NULL )
{
}


SvxFmTbxCtlRecTotal::~SvxFmTbxCtlRecTotal()
{
}


vcl::Window* SvxFmTbxCtlRecTotal::CreateItemWindow( vcl::Window* pParent )
{
    pFixedText = new FixedText( pParent );
    OUString aSample("123456");
    Size aSize( pFixedText->GetTextWidth( aSample ), pFixedText->GetTextHeight( ) );
    aSize.Width() += 12;
    pFixedText->SetSizePixel( aSize );
    pFixedText->SetBackground();
    pFixedText->SetPaintTransparent(true);
    return pFixedText;
}


void SvxFmTbxCtlRecTotal::StateChanged( sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState )
{

    // Setzen des FixedTextes
    if (GetSlotId() != SID_FM_RECORD_TOTAL)
        return;

    OUString aText;
    if (pState)
        aText = ((SfxStringItem*)pState)->GetValue();
    else
        aText = "?";

    pFixedText->SetText( aText );
    pFixedText->Update();
    pFixedText->Flush();

    SfxToolBoxControl::StateChanged( nSID, eState,pState );
}

SFX_IMPL_TOOLBOX_CONTROL( SvxFmTbxNextRec, SfxBoolItem );


SvxFmTbxNextRec::SvxFmTbxNextRec( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx )
    :SfxToolBoxControl( nSlotId, nId, rTbx )
{
    rTbx.SetItemBits(nId, rTbx.GetItemBits(nId) | TIB_REPEAT);

    AllSettings aSettings = rTbx.GetSettings();
    MouseSettings aMouseSettings = aSettings.GetMouseSettings();
    aMouseSettings.SetButtonRepeat(aMouseSettings.GetButtonRepeat() / 4);
    aSettings.SetMouseSettings(aMouseSettings);
    rTbx.SetSettings(aSettings, true);
}

SFX_IMPL_TOOLBOX_CONTROL( SvxFmTbxPrevRec, SfxBoolItem );


SvxFmTbxPrevRec::SvxFmTbxPrevRec( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx )
    :SfxToolBoxControl( nSlotId, nId, rTbx )
{
    rTbx.SetItemBits(nId, rTbx.GetItemBits(nId) | TIB_REPEAT);
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
