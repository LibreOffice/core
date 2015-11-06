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
        m_pController->Dispatch( ".uno:AbsoluteRecord",
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
    SvxFmAbsRecWin*     pWin = static_cast<SvxFmAbsRecWin*>( pToolBox->GetItemWindow(nId) );

    assert(pWin && "Control not found!");

    if (pState)
    {
        const SfxInt32Item* pItem = dynamic_cast< const SfxInt32Item* >( pState );
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


VclPtr<vcl::Window> SvxFmTbxCtlAbsRec::CreateItemWindow( vcl::Window* pParent )
{
    VclPtrInstance<SvxFmAbsRecWin> pWin( pParent, this );
    pWin->SetUniqueId( UID_ABSOLUTE_RECORD_WINDOW );
    return pWin.get();
}

SFX_IMPL_TOOLBOX_CONTROL( SvxFmTbxCtlRecText, SfxBoolItem );

SvxFmTbxCtlRecText::SvxFmTbxCtlRecText( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx )
    :SfxToolBoxControl( nSlotId, nId, rTbx )
{
}


SvxFmTbxCtlRecText::~SvxFmTbxCtlRecText()
{
}


VclPtr<vcl::Window> SvxFmTbxCtlRecText::CreateItemWindow( vcl::Window* pParent )
{
    OUString aText(SVX_RESSTR(RID_STR_REC_TEXT));
    VclPtrInstance<FixedText> pFixedText( pParent );
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


VclPtr<vcl::Window> SvxFmTbxCtlRecFromText::CreateItemWindow( vcl::Window* pParent )
{
    OUString aText(SVX_RESSTR(RID_STR_REC_FROM_TEXT));
    VclPtrInstance<FixedText> pFixedText( pParent, WB_CENTER );
    Size aSize( pFixedText->GetTextWidth( aText ), pFixedText->GetTextHeight( ) );
    aSize.Width() += 12;
    pFixedText->SetText( aText );
    pFixedText->SetSizePixel( aSize );
    pFixedText->SetBackground(Wallpaper(Color(COL_TRANSPARENT)));
    return pFixedText.get();
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


VclPtr<vcl::Window> SvxFmTbxCtlRecTotal::CreateItemWindow( vcl::Window* pParent )
{
    pFixedText.reset(VclPtr<FixedText>::Create( pParent ));
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
        aText = static_cast<const SfxStringItem*>(pState)->GetValue();
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
    rTbx.SetItemBits(nId, rTbx.GetItemBits(nId) | ToolBoxItemBits::REPEAT);

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
    rTbx.SetItemBits(nId, rTbx.GetItemBits(nId) | ToolBoxItemBits::REPEAT);
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
