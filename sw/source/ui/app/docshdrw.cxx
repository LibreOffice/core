/*************************************************************************
 *
 *  $RCSfile: docshdrw.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:31 $
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

#ifdef PRECOMPILED
#include "ui_pch.hxx"
#endif

#pragma hdrstop

#include "hintids.hxx"

#ifndef _SVX_SVXIDS_HRC //autogen
#include <svx/svxids.hrc>
#endif

#define ITEMID_COLOR_TABLE      SID_COLOR_TABLE
#define ITEMID_GRADIENT_LIST    SID_GRADIENT_LIST
#define ITEMID_HATCH_LIST       SID_HATCH_LIST
#define ITEMID_BITMAP_LIST      SID_BITMAP_LIST
#define ITEMID_DASH_LIST        SID_DASH_LIST
#define ITEMID_LINEEND_LIST     SID_LINEEND_LIST


#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif
#ifndef _OFF_APP_HXX //autogen
#include <offmgr/app.hxx>
#endif
#ifndef _SVX_DRAWITEM_HXX //autogen
#include <svx/drawitem.hxx>
#endif
#ifndef _SVDMODEL_HXX //autogen
#include <svx/svdmodel.hxx>
#endif
#ifndef _SVDOUTL_HXX
#include <svx/svdoutl.hxx>
#endif

#ifndef _SVX_LANGITEM_HXX
#include <svx/langitem.hxx>
#endif

#include "docsh.hxx"
#include "doc.hxx"


/*--------------------------------------------------------------------
    Beschreibung: Document laden
 --------------------------------------------------------------------*/


void  SwDocShell::InitDraw()
{
    SdrModel *pDrDoc = pDoc->GetDrawModel();
    if( pDrDoc )
    {
        // Listen, bzw. Tables im ItemSet der DocShell anlegen
        PutItem( SvxGradientListItem( pDrDoc->GetGradientList() ) );
        PutItem( SvxHatchListItem( pDrDoc->GetHatchList() ) );
        PutItem( SvxBitmapListItem( pDrDoc->GetBitmapList() ) );
        PutItem( SvxDashListItem( pDrDoc->GetDashList() ) );
        PutItem( SvxLineEndListItem( pDrDoc->GetLineEndList() ) );

        Outliner& rOutliner = pDrDoc->GetDrawOutliner();
        rOutliner.SetHyphenator( ::GetHyphenator() );
        const SfxPoolItem& rItem = GetDoc()->GetDefault(RES_CHRATR_LANGUAGE);
        rOutliner.SetDefaultLanguage(((const SvxLanguageItem&)rItem).GetLanguage());

    }
    else
        PutItem( SvxColorTableItem( OFF_APP()->GetStdColorTable() ));
}

/*------------------------------------------------------------------------
    $Log: not supported by cvs2svn $
    Revision 1.18  2000/09/18 16:05:10  willem.vandorp
    OpenOffice header added.

    Revision 1.17  2000/02/09 10:14:12  os
    #72716# set hyphenator and language at the DrawOutliner

    Revision 1.16  1997/11/29 15:04:46  MA
    includes


      Rev 1.15   29 Nov 1997 16:04:46   MA
   includes

      Rev 1.14   24 Nov 1997 14:22:50   MA
   includes

      Rev 1.13   03 Sep 1997 15:53:54   OS
   DLL-Umbau

      Rev 1.12   14 Aug 1996 12:00:56   JP
   svdraw.hxx entfernt

      Rev 1.11   29 Jul 1996 19:37:38   MA
   includes

      Rev 1.10   07 Mar 1996 12:09:58   HJS
   2 defines zu viel

      Rev 1.9   07 Dec 1995 08:07:08   SWG
   clooks

      Rev 1.8   05 Dec 1995 09:17:10   JP
   InitDraw: keine Parameter mehr

      Rev 1.7   24 Nov 1995 16:56:52   OM
   PCH->PRECOMPILED

      Rev 1.6   17 Nov 1995 19:15:50   OS
   ColorTable immer von der App

      Rev 1.5   07 Nov 1995 15:25:50   AMA
   Fix: ColorTable an der DocShell setzen.

      Rev 1.4   03 Nov 1995 19:27:56   AMA
   Opt.StartUp: DrawView/Model erst bei Bedarf.

      Rev 1.3   22 Aug 1995 09:00:00   MA
   svxitems-header entfernt

      Rev 1.2   09 Aug 1995 16:39:14   MA
   drawing-undo-header rein

      Rev 1.1   21 Mar 1995 02:23:32   ER
   _svdorect_hxx definiert => _svdcapt_hxx definieren

      Rev 1.0   13 Feb 1995 12:20:30   MS
   Initial revision.
------------------------------------------------------------------------*/


