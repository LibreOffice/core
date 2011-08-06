/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"


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

// z.Z. werden von Joe nur die u.a. Moeglichkeiten unterstuetzt
#define ESCDIR_COUNT 5
static sal_uInt16 aEscDirArray[] =
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

/*************************************************************************
|*
|* Konstruktor fuer Klebepunkt-Autrittsrichtungs-Listbox
|*
\************************************************************************/

GlueEscDirLB::GlueEscDirLB( Window* pParent, const Reference< XFrame >& rFrame ) :
        ListBox( pParent, WinBits( WB_BORDER | WB_DROPDOWN ) ),
        m_xFrame( rFrame )
{
    String aStr; aStr += sal_Unicode('X');
    Size aXSize( GetTextWidth( aStr ), GetTextHeight() );
    //SetPosPixel( Point( aSize.Width(), 0 ) );
    SetSizePixel( Size( aXSize.Width() * 12, aXSize.Height() * 10 ) );
    Fill();
    //SelectEntryPos( 0 );
    Show();
}

/*************************************************************************
|*
|*  Dtor
|*
\************************************************************************/

GlueEscDirLB::~GlueEscDirLB()
{
}

/*************************************************************************
|*
|* Ermittelt die Austrittsrichtung und verschickt den entspr. Slot
|*
\************************************************************************/

void GlueEscDirLB::Select()
{
    sal_uInt16 nPos = GetSelectEntryPos();
    SfxUInt16Item aItem( SID_GLUE_ESCDIR, aEscDirArray[ nPos ] );

    if ( m_xFrame.is() )
    {
        Any a;
        Sequence< PropertyValue > aArgs( 1 );
        aArgs[0].Name   = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "GlueEscapeDirection" ));
        aItem.QueryValue( a );
        aArgs[0].Value  = a;
        SfxToolBoxControl::Dispatch( Reference< XDispatchProvider >( m_xFrame->getController(), UNO_QUERY ),
                                    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:GlueEscapeDirection" )),
                                    aArgs );
    }
/*
    SfxViewFrame::Current()->GetDispatcher()->Execute( SID_GLUE_ESCDIR, SFX_CALLMODE_ASYNCHRON |
                                                       SFX_CALLMODE_RECORD, &aItem, (void*) NULL, 0L );
*/
}

/*************************************************************************
|*
|* Fuellen der Listbox mit Strings
|*
\************************************************************************/

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

/*************************************************************************
|*
|* Konstruktor fuer Klebepunkt-Autrittsrichtungs-Toolbox-Control
|*
\************************************************************************/

SdTbxCtlGlueEscDir::SdTbxCtlGlueEscDir(
    sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx ) :
        SfxToolBoxControl( nSlotId, nId, rTbx )
{
}

/*************************************************************************
|*
|* Stellt Status in der Listbox des Controllers dar
|*
\************************************************************************/

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


/*************************************************************************
|*
|* Liefert Position im Array fuer EscDir zurueck (Mapping fuer Listbox)
|*
\************************************************************************/

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
