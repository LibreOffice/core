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

#include <svx/svdglue.hxx>
#include <svl/intitem.hxx>
#include <vcl/toolbox.hxx>

#include <strings.hrc>
#include <gluectrl.hxx>
#include <sdresid.hxx>
#include <app.hrc>

#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/XFrame.hpp>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::frame;

// at the moment, Joe only supports the methods specified below
#define ESCDIR_COUNT 5
const SdrEscapeDirection aEscDirArray[] =
{
    SdrEscapeDirection::SMART,
    SdrEscapeDirection::LEFT,
    SdrEscapeDirection::RIGHT,
    SdrEscapeDirection::TOP,
    SdrEscapeDirection::BOTTOM
};

SFX_IMPL_TOOLBOX_CONTROL( SdTbxCtlGlueEscDir, SfxUInt16Item )

/**
 * Constructor for glue point escape direction Listbox
 */
GlueEscDirLB::GlueEscDirLB(vcl::Window* pParent, const Reference<XFrame>& rFrame)
    : InterimItemWindow(pParent, "modules/simpress/ui/gluebox.ui", "GlueBox")
    , m_xFrame(rFrame)
    , m_xWidget(m_xBuilder->weld_combo_box("gluetype"))
{
    InitControlBase(m_xWidget.get());

    Fill();

    m_xWidget->connect_changed(LINK(this, GlueEscDirLB, SelectHdl));
    m_xWidget->connect_key_press(LINK(this, GlueEscDirLB, KeyInputHdl));

    SetSizePixel(m_xWidget->get_preferred_size());

    Show();
}

void GlueEscDirLB::dispose()
{
    m_xWidget.reset();
    InterimItemWindow::dispose();
}

GlueEscDirLB::~GlueEscDirLB()
{
    disposeOnce();
}

void GlueEscDirLB::set_sensitive(bool bSensitive)
{
    Enable(bSensitive);
    m_xWidget->set_sensitive(bSensitive);
}

IMPL_LINK(GlueEscDirLB, KeyInputHdl, const KeyEvent&, rKEvt, bool)
{
    return ChildKeyInput(rKEvt);
}

/**
 * Determines the escape direction and sends the corresponding slot
 */
IMPL_LINK(GlueEscDirLB, SelectHdl, weld::ComboBox&, rBox, void)
{
    sal_Int32 nPos = rBox.get_active();
    SfxUInt16Item aItem( SID_GLUE_ESCDIR, static_cast<sal_uInt16>(aEscDirArray[ nPos ]) );

    if ( m_xFrame.is() )
    {
        Any a;
        Sequence< PropertyValue > aArgs( 1 );
        aArgs[0].Name   = "GlueEscapeDirection";
        aItem.QueryValue( a );
        aArgs[0].Value  = a;
        SfxToolBoxControl::Dispatch( Reference< XDispatchProvider >( m_xFrame->getController(), UNO_QUERY ),
                                    ".uno:GlueEscapeDirection",
                                    aArgs );
    }
}

/**
 * Fills the Listbox with strings
 */
void GlueEscDirLB::Fill()
{
    m_xWidget->append_text( SdResId( STR_GLUE_ESCDIR_SMART ) );
    m_xWidget->append_text( SdResId( STR_GLUE_ESCDIR_LEFT ) );
    m_xWidget->append_text( SdResId( STR_GLUE_ESCDIR_RIGHT ) );
    m_xWidget->append_text( SdResId( STR_GLUE_ESCDIR_TOP ) );
    m_xWidget->append_text( SdResId( STR_GLUE_ESCDIR_BOTTOM ) );
    /*
    m_xWidget->append_text( SdResId( STR_GLUE_ESCDIR_LO ) );
    m_xWidget->append_text( SdResId( STR_GLUE_ESCDIR_LU ) );
    m_xWidget->append_text( SdResId( STR_GLUE_ESCDIR_RO ) );
    m_xWidget->append_text( SdResId( STR_GLUE_ESCDIR_RU ) );
    m_xWidget->append_text( SdResId( STR_GLUE_ESCDIR_HORZ ) );
    m_xWidget->append_text( SdResId( STR_GLUE_ESCDIR_VERT ) );
    m_xWidget->append_text( SdResId( STR_GLUE_ESCDIR_ALL ) );
    */
}

/**
 * Constructor for glue point escape direction toolbox control
 */
SdTbxCtlGlueEscDir::SdTbxCtlGlueEscDir(
    sal_uInt16 nSlotId, ToolBoxItemId nId, ToolBox& rTbx ) :
        SfxToolBoxControl( nSlotId, nId, rTbx )
{
}

/**
 * Represents state in the listbox of the controller
 */
void SdTbxCtlGlueEscDir::StateChanged( sal_uInt16 nSId,
                        SfxItemState eState, const SfxPoolItem* pState )
{
    if( eState == SfxItemState::DEFAULT )
    {
        GlueEscDirLB* pGlueEscDirLB = static_cast<GlueEscDirLB*> ( GetToolBox().
                                            GetItemWindow( GetId() ) );
        if( pGlueEscDirLB )
        {
            if( pState )
            {
                pGlueEscDirLB->set_sensitive(true);
                if ( IsInvalidItem( pState ) )
                {
                    pGlueEscDirLB->set_active(-1);
                }
                else
                {
                    SdrEscapeDirection nEscDir = static_cast<SdrEscapeDirection>(static_cast<const SfxUInt16Item*>( pState )->GetValue());
                    pGlueEscDirLB->set_active( GetEscDirPos( nEscDir ) );
                }
            }
            else
            {
                pGlueEscDirLB->set_sensitive(false);
                pGlueEscDirLB->set_active(-1);
            }
        }
    }

    SfxToolBoxControl::StateChanged( nSId, eState, pState );
}

VclPtr<InterimItemWindow> SdTbxCtlGlueEscDir::CreateItemWindow( vcl::Window *pParent )
{
    if( GetSlotId() == SID_GLUE_ESCDIR )
        return VclPtr<GlueEscDirLB>::Create( pParent, m_xFrame ).get();

    return VclPtr<InterimItemWindow>();
}

/**
 * Returns position in the array for EscDir (Mapping for Listbox)
 */
sal_uInt16 SdTbxCtlGlueEscDir::GetEscDirPos( SdrEscapeDirection nEscDir )
{
    for( sal_uInt16 i = 0; i < ESCDIR_COUNT; i++ )
    {
        if( aEscDirArray[ i ] == nEscDir )
            return i;
    }
    return 99;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
