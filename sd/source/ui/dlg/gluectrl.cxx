/*************************************************************************
 *
 *  $RCSfile: gluectrl.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ka $ $Date: 2000-09-21 16:11:35 $
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

GlueEscDirLB::GlueEscDirLB( Window* pParent ) :
        ListBox( pParent, WinBits( WB_BORDER | WB_DROPDOWN ) )
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

__EXPORT GlueEscDirLB::~GlueEscDirLB()
{
}

/*************************************************************************
|*
|* Ermittelt die Austrittsrichtung und verschickt den entspr. Slot
|*
\************************************************************************/

void __EXPORT GlueEscDirLB::Select()
{
    UINT16 nPos = GetSelectEntryPos();
    SfxUInt16Item aItem( SID_GLUE_ESCDIR, aEscDirArray[ nPos ] );


    SfxViewFrame::Current()->GetDispatcher()->Execute( SID_GLUE_ESCDIR, SFX_CALLMODE_ASYNCHRON |
                                                       SFX_CALLMODE_RECORD, &aItem, (void*) NULL, 0L );
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

SdTbxCtlGlueEscDir::SdTbxCtlGlueEscDir( USHORT nId, ToolBox& rTbx,
                                    SfxBindings& rBindings ) :
        SfxToolBoxControl( nId, rTbx, rBindings )
{
}

/*************************************************************************
|*
|* Stellt Status in der Listbox des Controllers dar
|*
\************************************************************************/

void __EXPORT SdTbxCtlGlueEscDir::StateChanged( USHORT nSId,
                        SfxItemState eState, const SfxPoolItem* pState )
{
    if( eState == SFX_ITEM_AVAILABLE )
    {
        GlueEscDirLB* pGlueEscDirLB = (GlueEscDirLB*) ( GetToolBox().
                                            GetItemWindow( SID_GLUE_ESCDIR ) );
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

Window* __EXPORT SdTbxCtlGlueEscDir::CreateItemWindow( Window *pParent )
{
    if( GetId() == SID_GLUE_ESCDIR )
    {
        return( new GlueEscDirLB( pParent ) );
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

/*************************************************************************

      Source Code Control System - History

      $Log: not supported by cvs2svn $
      Revision 1.1.1.1  2000/09/18 16:48:32  hr
      initial import

      Revision 1.12  2000/09/17 15:23:35  willem.vandorp
      OpenOffice header added.

      Revision 1.11  2000/09/06 13:47:52  willem.vandorp
      Header and footer replaced.

      Revision 1.10  2000/05/24 16:11:27  aw
      UNICODE some sal_Unicode casts and some CreateFromInt32()

      Revision 1.9  2000/05/23 14:35:43  hr
      prevent conflict between STLPORT and Workshop

      Revision 1.8  2000/05/09 11:40:37  pw
      UNICODE

      Revision 1.7  2000/02/11 12:46:46  hr
      #70473# changes for unicode ( patched by automated patchtool )

      Revision 1.6  1998/01/14 16:41:54  DL
      includes


      Rev 1.5   14 Jan 1998 17:41:54   DL
   includes

      Rev 1.4   14 Nov 1996 09:55:34   TRI
   includes

      Rev 1.3   29 Oct 1996 10:51:28   DL
   Neu: SdResId

      Rev 1.2   07 Aug 1996 12:15:34   SOH
   Aufteilung und Entfernung von svdraw

      Rev 1.1   30 Jul 1996 16:31:16   SOH
   Noch nicht unterstuetzte EscDirs auskommentiert

      Rev 1.0   19 Jul 1996 09:51:28   SOH
   Initial revision.

      Rev 1.17   10 Jul 1996 19:07:48   NF
   includes

      Rev 1.16   27 Jun 1996 08:51:34   SOH
   SfxDispatcher.Execute()-Umstellung (2. Parameter wird jetzt verodert)

      Rev 1.15   04 Jun 1996 15:49:32   SOH
   #28246#: Fehlende __EXPORTs

      Rev 1.14   14 Mar 1996 17:10:18   SOH
   Neu: SdTbxCtlDiaPages und SdPagesField

      Rev 1.13   04 Dec 1995 13:32:14   DL
   Neuer Link

      Rev 1.12   17 Nov 1995 08:40:10   DL
   max() -> Max()

      Rev 1.11   16 Nov 1995 17:01:30   SOH
   StateChanged-Aenderungen (303)

      Rev 1.10   07 Nov 1995 16:15:48   TRI
   Change->Set

      Rev 1.9   01 Nov 1995 16:24:28   SOH
   String-Optimierung

      Rev 1.8   12 Oct 1995 17:16:50   SOH
   Umstellung auf Large Fonts und andere Systeme

      Rev 1.7   20 Jul 1995 13:12:46   SOH
   GetFocusHdl(): setzt Cursor auf Sec.

      Rev 1.6   04 Jul 1995 16:54:22   TRI
   EXPORTS

      Rev 1.5   03 May 1995 17:50:24   SOH
   Umstellung auf 246

      Rev 1.4   27 Mar 1995 14:08:50   SOH
   Umstellung auf 242.a

      Rev 1.3   23 Mar 1995 14:35:42   TRI
   __EXPORT eingebaut

      Rev 1.2   23 Mar 1995 11:31:44   TRI
   Segmentierung

      Rev 1.1   14 Feb 1995 01:37:38   MH
   Kommentar gerichtet

      Rev 1.0   13 Feb 1995 17:15:40   SOH
   Initial revision.

      Rev 1.12   11 Feb 1995 18:36:18   SOH
   Raus: Linien- und Flaechen-Control

      Rev 1.11   04 Feb 1995 15:18:34   TRI
   ExecuteCall ambiguos

      Rev 1.10   01 Feb 1995 12:22:40   TRI
   Pragmas zur Segmentierung eingebaut

      Rev 1.9   20 Jan 1995 11:41:00   ESO
   sfxdoc.hxx und sfxdlg.hxx

      Rev 1.8   19 Jan 1995 16:48:12   ESO
   Dispatcher-Aufrufe auf asynchron gesetzt

      Rev 1.7   19 Jan 1995 15:53:06   SOH
   Umstellung der SID_s von app.hrc -> dialogs.hrc

      Rev 1.6   17 Jan 1995 15:21:30   SOH
   Strings -> Resourcen

      Rev 1.5   16 Jan 1995 15:22:50   SOH
   Farbverlauf und Schraffuren: keine Dummys mehr

      Rev 1.4   11 Jan 1995 12:56:58   SOH
   DiaHdl erweitert

      Rev 1.3   06 Jan 1995 10:23:54   SOH
   Neu: Dia-TbxCtrls

      Rev 1.2   22 Dec 1994 15:53:16   SOH
   Doc-Ref wird zur Laufzeit geholt; EXECUTE wird nur ausgefuehrt, wenn vom User gerufen

      Rev 1.1   22 Dec 1994 12:00:28   SOH
   Groessen-Anpassung

      Rev 1.0   20 Dec 1994 10:01:54   SOH
   Initial revision.

*************************************************************************/

