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

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::frame;

// at the moment, Joe only supports the methods specified below
#define ESCDIR_COUNT 5
static const SdrEscapeDirection aEscDirArray[] =
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
GlueEscDirLB::GlueEscDirLB( vcl::Window* pParent, const Reference< XFrame >& rFrame ) :
        ListBox( pParent, WinBits( WB_BORDER | WB_DROPDOWN ) ),
        m_xFrame( rFrame )
{
    Size aXSize( GetTextWidth( "X" ), GetTextHeight() );
    SetSizePixel( Size( aXSize.Width() * 12, aXSize.Height() * 10 ) );
    Fill();
    Show();
}

GlueEscDirLB::~GlueEscDirLB()
{
}

/**
 * Determines the escape direction and sends the corresponding slot
 */
void GlueEscDirLB::Select()
{
    sal_Int32 nPos = GetSelectedEntryPos();
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
    InsertEntry( SdResId( STR_GLUE_ESCDIR_SMART ) );
    InsertEntry( SdResId( STR_GLUE_ESCDIR_LEFT ) );
    InsertEntry( SdResId( STR_GLUE_ESCDIR_RIGHT ) );
    InsertEntry( SdResId( STR_GLUE_ESCDIR_TOP ) );
    InsertEntry( SdResId( STR_GLUE_ESCDIR_BOTTOM ) );
    /*
    InsertEntry( SdResId( STR_GLUE_ESCDIR_LO ) );
    InsertEntry( SdResId( STR_GLUE_ESCDIR_LU ) );
    InsertEntry( SdResId( STR_GLUE_ESCDIR_RO ) );
    InsertEntry( SdResId( STR_GLUE_ESCDIR_RU ) );
    InsertEntry( SdResId( STR_GLUE_ESCDIR_HORZ ) );
    InsertEntry( SdResId( STR_GLUE_ESCDIR_VERT ) );
    InsertEntry( SdResId( STR_GLUE_ESCDIR_ALL ) );
    */
}

/**
 * Constructor for glue point escape direction toolbox control
 */
SdTbxCtlGlueEscDir::SdTbxCtlGlueEscDir(
    sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx ) :
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
                pGlueEscDirLB->Enable();
                if ( IsInvalidItem( pState ) )
                {
                    pGlueEscDirLB->SetNoSelection();
                }
                else
                {
                    SdrEscapeDirection nEscDir = static_cast<SdrEscapeDirection>(static_cast<const SfxUInt16Item*>( pState )->GetValue());
                    pGlueEscDirLB->SelectEntryPos( GetEscDirPos( nEscDir ) );
                }
            }
            else
            {
                pGlueEscDirLB->Disable();
                pGlueEscDirLB->SetNoSelection();
            }
        }
    }

    SfxToolBoxControl::StateChanged( nSId, eState, pState );
}

VclPtr<vcl::Window> SdTbxCtlGlueEscDir::CreateItemWindow( vcl::Window *pParent )
{
    if( GetSlotId() == SID_GLUE_ESCDIR )
        return VclPtr<GlueEscDirLB>::Create( pParent, m_xFrame ).get();

    return VclPtr<vcl::Window>();
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
