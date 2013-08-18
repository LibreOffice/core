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

#include <svx/dialogs.hrc>
#include <svx/svdglue.hxx>
#include <svl/intitem.hxx>
#include <sfx2/app.hxx>
#include <sfx2/dispatch.hxx>
#include <vcl/toolbox.hxx>
#include <sfx2/viewfrm.hxx>

#include "strings.hrc"
#include "gluectrl.hxx"
#include "sdresid.hxx"
#include "app.hrc"

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::frame;

// at the moment, Joe only supports the methods specified below
#define ESCDIR_COUNT 5
static const sal_uInt16 aEscDirArray[] =
{
    SDRESC_SMART,
    SDRESC_LEFT,
    SDRESC_RIGHT,
    SDRESC_TOP,
    SDRESC_BOTTOM,
//  SDRESC_LO,
//  SDRESC_LU,
//  SDRESC_RO,
//  SDRESC_RU,
//  SDRESC_HORZ,
//  SDRESC_VERT,
//  SDRESC_ALL
};



SFX_IMPL_TOOLBOX_CONTROL( SdTbxCtlGlueEscDir, SfxUInt16Item )

/**
 * Constructor for glue point escape direction Listbox
 */
GlueEscDirLB::GlueEscDirLB( Window* pParent, const Reference< XFrame >& rFrame ) :
        ListBox( pParent, WinBits( WB_BORDER | WB_DROPDOWN ) ),
        m_xFrame( rFrame )
{
    String aStr; aStr += sal_Unicode('X');
    Size aXSize( GetTextWidth( aStr ), GetTextHeight() );
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
    sal_uInt16 nPos = GetSelectEntryPos();
    SfxUInt16Item aItem( SID_GLUE_ESCDIR, aEscDirArray[ nPos ] );

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
    InsertEntry( String( SdResId( STR_GLUE_ESCDIR_SMART ) ) );
    InsertEntry( String( SdResId( STR_GLUE_ESCDIR_LEFT ) ) );
    InsertEntry( String( SdResId( STR_GLUE_ESCDIR_RIGHT ) ) );
    InsertEntry( String( SdResId( STR_GLUE_ESCDIR_TOP ) ) );
    InsertEntry( String( SdResId( STR_GLUE_ESCDIR_BOTTOM ) ) );
    /*
    InsertEntry( String( SdResId( STR_GLUE_ESCDIR_LO ) ) );
    InsertEntry( String( SdResId( STR_GLUE_ESCDIR_LU ) ) );
    InsertEntry( String( SdResId( STR_GLUE_ESCDIR_RO ) ) );
    InsertEntry( String( SdResId( STR_GLUE_ESCDIR_RU ) ) );
    InsertEntry( String( SdResId( STR_GLUE_ESCDIR_HORZ ) ) );
    InsertEntry( String( SdResId( STR_GLUE_ESCDIR_VERT ) ) );
    InsertEntry( String( SdResId( STR_GLUE_ESCDIR_ALL ) ) );
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
    if( eState == SFX_ITEM_AVAILABLE )
    {
        GlueEscDirLB* pGlueEscDirLB = (GlueEscDirLB*) ( GetToolBox().
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
                    sal_uInt16 nEscDir = ( (const SfxUInt16Item*) pState )->GetValue();
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

Window* SdTbxCtlGlueEscDir::CreateItemWindow( Window *pParent )
{
    if( GetSlotId() == SID_GLUE_ESCDIR )
    {
        return( new GlueEscDirLB( pParent, m_xFrame ) );
    }

    return( NULL );
}


/**
 * Returns position in the array for EscDir (Mapping for Listbox)
 */
sal_uInt16 SdTbxCtlGlueEscDir::GetEscDirPos( sal_uInt16 nEscDir )
{
    for( sal_uInt16 i = 0; i < ESCDIR_COUNT; i++ )
    {
        if( aEscDirArray[ i ] == nEscDir )
            return( i );
    }
    return( 99 );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
