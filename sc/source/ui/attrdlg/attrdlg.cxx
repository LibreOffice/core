/*************************************************************************
 *
 *  $RCSfile: attrdlg.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:44:53 $
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

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

#ifndef PCH
#include "scitems.hxx"
#include <segmentc.hxx>

#include <sfx2/objsh.hxx>
#include <svx/numfmt.hxx>
#include <sfx2/tabdlg.hxx>
#include <svx/chardlg.hxx>
#include <svx/align.hxx>
#include <svx/border.hxx>
#include <svx/backgrnd.hxx>
/*
#include <sbx.hxx>
#include <sfxdoc.hxx>

#include <dialogs.hrc>
#include <numfmt.hxx>
#include <chardlg.hxx>
#include <border.hxx>
#include <backgrnd.hxx>
//#include <dialogs.hxx>
*/
#endif


//#include <align.hxx>      // SvxAlignmentTabPage

#include "tabpages.hxx"
#include "attrdlg.hxx"
#include "scresid.hxx"
#include "attrdlg.hrc"

#pragma hdrstop

SEG_EOFGLOBALS()
//==================================================================
#pragma SEG_FUNCDEF(attrdlg_01)

ScAttrDlg::ScAttrDlg( SfxViewFrame*     pFrame,
                      Window*           pParent,
                      const SfxItemSet* pCellAttrs )

    :   SfxTabDialog( pFrame,
                      pParent,
                      ScResId( RID_SCDLG_ATTR ),
                      pCellAttrs )
{
    AddTabPage( TP_NUMBER,      SvxNumberFormatTabPage::Create, 0 );
    AddTabPage( TP_FONT,        SvxCharStdPage::Create,         0 );
    AddTabPage( TP_ALIGNMENT,   SvxAlignmentTabPage::Create,    0 );
    AddTabPage( TP_BORDER,      SvxBorderTabPage::Create,       0 );
    AddTabPage( TP_BACKGROUND,  SvxBackgroundTabPage::Create,   0 );
    AddTabPage( TP_PROTECTION,  ScTabPageProtection::Create,    0 );
    FreeResource();
}

// -----------------------------------------------------------------------
#pragma SEG_FUNCDEF(attrdlg_02)

__EXPORT ScAttrDlg::~ScAttrDlg()
{
}

// -----------------------------------------------------------------------
#pragma SEG_FUNCDEF(attrdlg_03)

void __EXPORT ScAttrDlg::PageCreated( USHORT nPageId, SfxTabPage& rTabPage )
{
    SfxObjectShell* pDocSh = SfxObjectShell::Current();

    switch ( nPageId )
    {
        case TP_NUMBER:
        {
            SvxNumberFormatTabPage& rNumPage  = (SvxNumberFormatTabPage&)rTabPage;

            rNumPage.SetOkHdl( LINK( this, ScAttrDlg, OkHandler ) );
        }
        break;

        case TP_FONT:
        {
            const SfxPoolItem* pInfoItem = pDocSh->GetItem( SID_ATTR_CHAR_FONTLIST );

            DBG_ASSERT( pInfoItem, "FontListItem  not found :-(" );

            ((SvxCharStdPage&)rTabPage).
                SetFontList( *((const SvxFontListItem*)pInfoItem) );
        }
        break;

        default:
        break;
    }
}

// -----------------------------------------------------------------------
#pragma SEG_FUNCDEF(attrdlg_04)

IMPL_LINK( ScAttrDlg, OkHandler, void*, EMPTYARG )
{
    ((Link&)GetOKButton().GetClickHdl()).Call( NULL );

    return NULL;
}

/*------------------------------------------------------------------------

    $Log: not supported by cvs2svn $
    Revision 1.42  2000/09/17 14:08:55  willem.vandorp
    OpenOffice header added.

    Revision 1.41  2000/08/31 16:38:19  willem.vandorp
    Header and footer replaced

    Revision 1.40  1997/12/05 19:05:04  ANK
    Includes geaendert


      Rev 1.39   05 Dec 1997 20:05:04   ANK
   Includes geaendert

      Rev 1.38   02 Oct 1997 18:30:18   ANK
   Geaendert fuer SfxItem bei NumDialog

      Rev 1.37   29 Oct 1996 14:04:36   NN
   ueberall ScResId statt ResId

      Rev 1.36   23 Jan 1996 11:52:54   MO
   neuer Link

      Rev 1.35   08 Nov 1995 14:31:32   TRI
   301 Anpassung

      Rev 1.34   04 Oct 1995 18:59:32   MO
   257-Aenderung wieder entfernt

      Rev 1.32   24 Aug 1995 17:07:40   HJS
   header umstellung

      Rev 1.31   23 Aug 1995 15:57:12   MI
   SfxDocumentShell zu SfxObjectShell)

      Rev 1.30   19 Jul 1995 12:33:16   MO
   Ok-Handler an NumberPage uebergeben => Beenden des Dialoges ueber DoppelClick
   (BugId: 15805)
   Benoetigt Svx-Nachzieher

      Rev 1.29   15 Jun 1995 09:27:30   TRI
   EXPORT

      Rev 1.28   18 May 1995 14:12:40   MO
   Alignment: ScTabPage-> SvxTabPage, ScAttrs -> SvxItems

      Rev 1.27   17 May 1995 13:52:20   MO
   ViewFrame-Ctor

      Rev 1.26   19 Feb 1995 19:01:40   TRI
   sfxdoc.hxx included

      Rev 1.25   16 Feb 1995 17:51:48   MO
   * DocShell-Member nicht noetig
   -

      Rev 1.24   05 Feb 1995 12:14:42   MO
   New logfile T:/sc/source/ui/attrdlg/temp.__v
   Enter description of workfile.

      Rev 1.23   30 Jan 1995 12:22:54   TRI
   sbx.hxx include eingebaut

      Rev 1.22   27 Jan 1995 10:14:28   MO
   SvxNumberTabPage eingebaut

      Rev 1.21   26 Jan 1995 17:58:34   TRI
   __EXPORT bei virtuellen Methoden eingebaut

      Rev 1.20   22 Jan 1995 15:09:12   SC
   2.37 Anpassung

      Rev 1.19   18 Jan 1995 13:07:42   TRI
   Pragmas zur Segementierung eingebaut

      Rev 1.18   14 Dec 1994 13:50:32   MO
   Globale ResIds jetzt in sc.hrc

      Rev 1.17   13 Dec 1994 17:09:36   MO
   Dialog bekommt wieder eine SfxMap (SfxWhichMap nur fuer SfxTabDlg gedacht)

      Rev 1.16   07 Dec 1994 09:54:40   MO
   SfxMap -> SfxWhichMap (Sfx 233.b)

      Rev 1.15   21 Nov 1994 15:30:24   MO

      Rev 1.13   10 Nov 1994 14:28:52   MO
   PCHs

      Rev 1.12   09 Nov 1994 18:05:56   MO
   SvxBackground-Page

      Rev 1.11   06 Oct 1994 11:43:38   MO
   Anpassungen an Sfx 228g

      Rev 1.9   04 Oct 1994 15:56:26   MO
   Umstellung auf allgemeine Sfx-Loesung

      Rev 1.8   27 Sep 1994 16:10:40   MO

      Rev 1.7   20 Sep 1994 19:53:38   MO
   No change.

      Rev 1.6   19 Sep 1994 13:18:14   MO

      Rev 1.5   14 Sep 1994 12:13:48   MO
   Attribute werden nicht mehr aktiv vom Dialog geliefert, sondern
   muessen ueber GetCurAttrs() erfragt werden.
   Als Parameter wird jetzt SfxSetItem erwartet.

      Rev 1.4   09 Sep 1994 12:56:16   MO
   Hintergrund- und Zeichenseiten rudimentaer implementiert

      Rev 1.3   06 Sep 1994 16:54:24   MO
   Hintergrund-TabPage rudimentaer implementiert

      Rev 1.2   01 Sep 1994 12:46:00   MO

------------------------------------------------------------------------*/

#pragma SEG_EOFMODULE

