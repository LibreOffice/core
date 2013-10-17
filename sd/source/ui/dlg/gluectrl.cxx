/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/

#

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"


#include <string> // HACK: prevent conflict between STLPORT and Workshop headers

#include <svx/dialogs.hrc>
#include <svx/sdrglue.hxx>
#include <svl/intitem.hxx>
#include <sfx2/app.hxx>
#include <sfx2/dispatch.hxx>
#include <vcl/toolbox.hxx>
#include <sfx2/viewfrm.hxx>
#include <svx/gluepoint.hxx>

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
    sdr::glue::GluePoint::ESCAPE_DIRECTION_SMART,
    sdr::glue::GluePoint::ESCAPE_DIRECTION_LEFT,
    sdr::glue::GluePoint::ESCAPE_DIRECTION_RIGHT,
    sdr::glue::GluePoint::ESCAPE_DIRECTION_TOP,
    sdr::glue::GluePoint::ESCAPE_DIRECTION_BOTTOM
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

sal_uInt16 SdTbxCtlGlueEscDir::GetEscDirPos( sal_uInt16 nEscDir )
{
    for( sal_uInt16 i = 0; i < ESCDIR_COUNT; i++ )
    {
        if( aEscDirArray[ i ] == nEscDir )
            return( i );
    }
    return( 99 );
}
