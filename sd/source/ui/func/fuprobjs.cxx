/*************************************************************************
 *
 *  $RCSfile: fuprobjs.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: cl $ $Date: 2000-11-15 13:34:21 $
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

#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SFXSTYLE_HXX //autogen
#include <svtools/style.hxx>
#endif
#ifndef _OUTLINER_HXX
#include <svx/outliner.hxx>
#endif
#ifndef _SFXSMPLHINT_HXX //autogen
#include <svtools/smplhint.hxx>
#endif

#pragma hdrstop

#include "app.hrc"
#include "res_bmp.hrc"
#include "strings.hrc"
#include "glob.hrc"
#include "prltempl.hrc"

#include "sdresid.hxx"
#include "fuprobjs.hxx"
#include "drawdoc.hxx"
#include "outlnvsh.hxx"
#include "viewshel.hxx"
#include "glob.hxx"
#include "prlayout.hxx"
#include "prltempl.hxx"
#include "unchss.hxx"


TYPEINIT1( FuPresentationObjects, FuPoor );


/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

FuPresentationObjects::FuPresentationObjects(SdViewShell* pViewSh,
                                           SdWindow* pWin, SdView* pView,
                                           SdDrawDocument* pDoc,
                                           SfxRequest& rReq)
     : FuPoor(pViewSh, pWin, pView, pDoc, rReq)

{
    // ergibt die Selektion ein eindeutiges Praesentationslayout?
    // wenn nicht, duerfen die Vorlagen nicht bearbeitet werden
    SfxItemSet aSet(pDoc->GetItemPool(), SID_STATUS_LAYOUT, SID_STATUS_LAYOUT);
    ( (SdOutlineViewShell*) pViewSh )->GetStatusBarState( aSet );
    String aLayoutName = (((SfxStringItem&)aSet.Get(SID_STATUS_LAYOUT)).GetValue());
    DBG_ASSERT(aLayoutName.Len(), "Layout unbestimmt");

    BOOL    bUnique = FALSE;
    USHORT  nDepth, nTmp;
    SdOutlineView* pOlView = (SdOutlineView*)( (SdOutlineViewShell*) pViewSh )->GetView();
    OutlinerView* pOutlinerView = pOlView->GetViewByWindow( (Window*) pWin );
    Outliner* pOutl = pOutlinerView->GetOutliner();
    List* pList = pOutlinerView->CreateSelectionList();
    Paragraph* pPara = (Paragraph*)pList->First();
    nDepth = pOutl->GetDepth( pOutl->GetAbsPos( pPara ) );
    while( pPara )
    {
        nTmp = pOutl->GetDepth( pOutl->GetAbsPos( pPara ) );

        if( nDepth != nTmp )
        {
            bUnique = FALSE;
            break;
        }
        bUnique = TRUE;

        pPara = (Paragraph*) pList->Next();
    }

    if( bUnique )
    {
        String aStyleName = aLayoutName;
        aStyleName.AppendAscii( RTL_CONSTASCII_STRINGPARAM( SD_LT_SEPARATOR ) );
        USHORT nDlgId = TAB_PRES_LAYOUT_TEMPLATE_3;
        PresentationObjects ePO;

        if( nDepth == 0 )
        {
            ePO = PO_TITLE;
            String aStr(SdResId( STR_LAYOUT_TITLE ));
            aStyleName.Append( aStr );
        }
        else
        {
            ePO = (PresentationObjects) ( PO_OUTLINE_1 + nDepth - 1 );
            String aStr(SdResId( STR_LAYOUT_OUTLINE ));
            aStyleName.Append( aStr );
            aStyleName.Append( sal_Unicode(' ') );
            aStyleName.Append( UniString::CreateFromInt32( nDepth ) );
        }

        SfxStyleSheetBasePool* pStyleSheetPool = pDocSh->GetStyleSheetPool();
        SfxStyleSheetBase* pStyleSheet = pStyleSheetPool->Find( aStyleName,
                            (SfxStyleFamily) SD_LT_FAMILY );
        DBG_ASSERT(pStyleSheet, "StyleSheet nicht gefunden");

        if( pStyleSheet )
        {
            SfxStyleSheetBase& rStyleSheet = *pStyleSheet;

            SdPresLayoutTemplateDlg* pDlg = new SdPresLayoutTemplateDlg( pDocSh, NULL,
                                                SdResId( nDlgId ), rStyleSheet, ePO, pStyleSheetPool );

            if( pDlg->Execute() == RET_OK )
            {
                const SfxItemSet* pOutSet = pDlg->GetOutputItemSet();
                // Undo-Action
                StyleSheetUndoAction* pAction = new StyleSheetUndoAction
                                                (pDoc, (SfxStyleSheet*)pStyleSheet,
                                                    pOutSet);
                pDocSh->GetUndoManager()->AddUndoAction(pAction);

                pStyleSheet->GetItemSet().Put( *pOutSet );
                ( (SfxStyleSheet*) pStyleSheet )->Broadcast( SfxSimpleHint( SFX_HINT_DATACHANGED ) );
            }
            delete( pDlg );
        }
    }
}

/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

FuPresentationObjects::~FuPresentationObjects()
{
}



