/*************************************************************************
 *
 *  $RCSfile: fuparagr.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:48:35 $
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


// Ext-Absatz-Tab-Page
#define ITEMID_HYPHENZONE       SID_ATTR_PARA_HYPHENZONE
#define ITEMID_FMTBREAK         SID_ATTR_PARA_PAGEBREAK
#define ITEMID_FMTSPLIT         SID_ATTR_PARA_SPLIT
#define ITEMID_WIDOWS           SID_ATTR_PARA_WIDOWS
#define ITEMID_ORPHANS          SID_ATTR_PARA_ORPHANS

#ifndef _EEITEM_HXX //autogen
#include <svx/eeitem.hxx>
#endif
#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SFX_BINDINGS_HXX //autogen
#include <sfx2/bindings.hxx>
#endif
#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif
#include <svx/hyznitem.hxx>
#include <svx/brkitem.hxx>
#include <svx/spltitem.hxx>
#include <svx/widwitem.hxx>
#include <svx/orphitem.hxx>
#include <svx/svxids.hrc>
#include <svx/editdata.hxx>

#ifndef _EEITEMID_HXX
#include <svx/eeitemid.hxx>
#endif
#ifndef _SVX_LRSPITEM_HXX //autogen
#include <svx/lrspitem.hxx>
#endif
#include "app.hrc"
#include "paragr.hxx"
#include "sdview.hxx"
#include "fuparagr.hxx"
#include "drawdoc.hxx"

TYPEINIT1( FuParagraph, FuPoor );

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

FuParagraph::FuParagraph( SdViewShell* pViewSh, SdWindow* pWin, SdView* pView,
                          SdDrawDocument* pDoc, SfxRequest& rReq)
       : FuPoor(pViewSh, pWin, pView, pDoc, rReq)
{
    const SfxItemSet* pArgs = rReq.GetArgs();

    if( !pArgs )
    {
        SfxItemSet aEditAttr( pDoc->GetPool() );
        pView->GetAttributes( aEditAttr );
        SfxItemPool *pPool =  aEditAttr.GetPool();
        SfxItemSet aNewAttr( *pPool,
                                EE_ITEMS_START, EE_ITEMS_END,
                                SID_ATTR_TABSTOP_OFFSET, SID_ATTR_TABSTOP_OFFSET,
                                SID_ATTR_PARA_HYPHENZONE, SID_ATTR_PARA_HYPHENZONE,
                                SID_ATTR_PARA_PAGEBREAK, SID_ATTR_PARA_PAGEBREAK,
                                SID_ATTR_PARA_SPLIT, SID_ATTR_PARA_SPLIT,
                                SID_ATTR_PARA_WIDOWS, SID_ATTR_PARA_WIDOWS,
                                SID_ATTR_PARA_ORPHANS, SID_ATTR_PARA_ORPHANS,
                                0 );

        aNewAttr.Put( aEditAttr );

        // Die Werte sind erst einmal uebernommen worden, um den Dialog anzuzeigen.
        // Muss natuerlich noch geaendert werden
        // aNewAttr.Put( SvxParaDlgLimitsItem( 567 * 50, 5670) );

        aNewAttr.Put( SvxHyphenZoneItem() );
        aNewAttr.Put( SvxFmtBreakItem() );
        aNewAttr.Put( SvxFmtSplitItem() );
        aNewAttr.Put( SvxWidowsItem() );
        aNewAttr.Put( SvxOrphansItem() );

        // linker Rand als Offset
        const long nOff = ( (SvxLRSpaceItem&)aNewAttr.Get( EE_PARA_LRSPACE ) ).GetTxtLeft();
        // Umrechnung, da TabulatorTabPage immer von Twips ausgeht !
        long nTwipOff = OutputDevice::LogicToLogic( nOff, MAP_100TH_MM, MAP_TWIP );
        SfxInt32Item aOff( SID_ATTR_TABSTOP_OFFSET, nTwipOff );
        aNewAttr.Put( aOff );

        SdParagraphDlg* pDlg = new SdParagraphDlg( NULL, &aNewAttr );


        USHORT nResult = pDlg->Execute();

        switch( nResult )
        {
            case RET_OK:
            {
                rReq.Done( *( pDlg->GetOutputItemSet() ) );

                pArgs = rReq.GetArgs();
            }
            break;

            default:
            {
                delete pDlg;
            }
            return; // Abbruch
        }
        delete( pDlg );
    }
    pView->SetAttributes( *pArgs );

    // invalidieren der Slots
    static USHORT SidArray[] = {
//                          SID_ATTR_TABSTOP,  // Neu
                            SID_ATTR_PARA_ADJUST_LEFT,
                            SID_ATTR_PARA_ADJUST_RIGHT,
                            SID_ATTR_PARA_ADJUST_CENTER,
                            SID_ATTR_PARA_ADJUST_BLOCK,
                            SID_ATTR_PARA_LINESPACE_10,
                            SID_ATTR_PARA_LINESPACE_15,
                            SID_ATTR_PARA_LINESPACE_20,
                            SID_ATTR_PARA_LRSPACE, // Neu
                            SID_PARASPACE_INCREASE,
                            SID_PARASPACE_DECREASE,
                            0 };

    SFX_BINDINGS().Invalidate( SidArray );
}



