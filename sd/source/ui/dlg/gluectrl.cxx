#/*************************************************************************
 *
 *  $RCSfile: gluectrl.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2004-07-06 12:25:15 $
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

#include <string> // HACK: prevent conflict between STLPORT and Workshop headers

#include <svx/dialogs.hrc>
#ifndef _SVDGLUE_HXX //autogen
#include <svx/svdglue.hxx>
#endif
#ifndef _SFXINTITEM_HXX //autogen
#include <svtools/intitem.hxx>
#endif
#ifndef _SFXAPP_HXX //autogen
#include <sfx2/app.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SV_TOOLBOX_HXX //autogen
#include <vcl/toolbox.hxx>
#endif
#ifndef _SFXVIEWFRM_HXX
#include <sfx2/viewfrm.hxx>
#endif

#include "strings.hrc"
#include "gluectrl.hxx"
#include "sdresid.hxx"
#include "app.hrc"

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::frame;

// z.Z. werden von Joe nur die u.a. Moeglichkeiten unterstuetzt
#define ESCDIR_COUNT 5
static UINT16 aEscDirArray[] =
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
    UINT16 nPos = GetSelectEntryPos();
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
    USHORT nSlotId, USHORT nId, ToolBox& rTbx ) :
        SfxToolBoxControl( nSlotId, nId, rTbx )
{
}

/*************************************************************************
|*
|* Stellt Status in der Listbox des Controllers dar
|*
\************************************************************************/

void SdTbxCtlGlueEscDir::StateChanged( USHORT nSId,
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
                    UINT16 nEscDir = ( (const SfxUInt16Item*) pState )->GetValue();
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

/*************************************************************************
|*
|* No Comment
|*
\************************************************************************/

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

UINT16 SdTbxCtlGlueEscDir::GetEscDirPos( UINT16 nEscDir )
{
    for( UINT16 i = 0; i < ESCDIR_COUNT; i++ )
    {
        if( aEscDirArray[ i ] == nEscDir )
            return( i );
    }
    return( 99 );
}
