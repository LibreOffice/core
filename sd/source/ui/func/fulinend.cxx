/*************************************************************************
 *
 *  $RCSfile: fulinend.cxx,v $
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

#ifndef _XTABLE_HXX
#include <svx/xtable.hxx>
#endif
#ifndef _SVX_DLG_NAME_HXX //autogen
#include <svx/dlgname.hxx>
#endif
#ifndef _SVDOBJ_HXX //autogen
#include <svx/svdobj.hxx>
#endif
#ifndef _SVDOPATH_HXX //autogen
#include <svx/svdopath.hxx>
#endif
#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif

#include "strings.hrc"

#ifndef _SD_VIEWSHEL_HXX
#include "viewshel.hxx"
#endif



#include "sdresid.hxx"
#include "drawdoc.hxx"
#include "sdview.hxx"
#include "sdwindow.hxx"
#include "fulinend.hxx"

#define BITMAP_WIDTH  32
#define BITMAP_HEIGHT 12

TYPEINIT1( FuLineEnd, FuPoor );

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

FuLineEnd::FuLineEnd(SdViewShell* pViewSh, SdWindow* pWin, SdView* pView,
                    SdDrawDocument* pDoc, SfxRequest& rReq)
    : FuPoor(pViewSh, pWin, pView, pDoc, rReq)
{
    const SdrMarkList& rMarkList = pView->GetMarkList();

    if( rMarkList.GetMarkCount() == 1 )
    {
        const SdrObject* pObj = rMarkList.GetMark(0)->GetObj();
        const SdrObject* pNewObj;
        SdrObject* pConvPolyObj = NULL;

        if( pObj->ISA( SdrPathObj ) )
        {
            pNewObj = pObj;
        }
        else
        {
            SdrObjTransformInfoRec aInfoRec;
            pObj->TakeObjInfo( aInfoRec );

            if( aInfoRec.bCanConvToPath &&
                pObj->GetObjInventor() == SdrInventor &&
                pObj->GetObjIdentifier() != OBJ_GRUP )
                // bCanConvToPath ist bei Gruppenobjekten TRUE,
                // stuerzt aber bei ConvertToPathObj() ab !
            {
                pNewObj = pConvPolyObj = pObj->ConvertToPolyObj( TRUE, FALSE );

                if( !pNewObj || !pNewObj->ISA( SdrPathObj ) )
                    return; // Abbruch, zusaetzliche Sicherheit, die bei
                            // Gruppenobjekten aber nichts bringt.
            }
            else return; // Abbruch
        }

        const XPolygon aXPoly = ( (SdrPathObj*) pNewObj )->GetPathPoly().GetObject( 0 );

        // Loeschen des angelegten PolyObjektes
        if( pConvPolyObj )
            delete pConvPolyObj;

        XLineEndList* pLineEndList = pDoc->GetLineEndList();
        XLineEndEntry* pEntry;

        String aNewName( SdResId( STR_LINEEND ) );
        String aDesc( SdResId( STR_DESC_LINEEND ) );
        String aName;

        long nCount = pLineEndList->Count();
        long j = 1;
        BOOL bDifferent = FALSE;

        while( !bDifferent )
        {
            aName = aNewName;
            aName.Append( sal_Unicode(' ') );
            aName.Append( UniString::CreateFromInt32( j++ ) );
            bDifferent = TRUE;
            for( long i = 0; i < nCount && bDifferent; i++ )
            {
                if( aName == pLineEndList->Get( i )->GetName() )
                    bDifferent = FALSE;
            }
        }

        SvxNameDialog* pDlg = new SvxNameDialog( NULL, aName, aDesc );

        if( pDlg->Execute() == RET_OK )
        {
            pDlg->GetName( aName );
            bDifferent = TRUE;

            for( long i = 0; i < nCount && bDifferent; i++ )
            {
                if( aName == pLineEndList->Get( i )->GetName() )
                    bDifferent = FALSE;
            }

            if( bDifferent )
            {
            /*
                XOutdevItemPool* pItemPool = (XOutdevItemPool*) &pViewSh->GetPool();
                XLineAttrSetItem aXLSet( pItemPool );
                XFillAttrSetItem aXFSet( pItemPool );
                VirtualDevice    aTmpVD ( *pWindow );
                XOutputDevice    aTmpXOut( &aTmpVD, pItemPool );

                aTmpVD.ChangeMapMode( MAP_100TH_MM );
                aTmpVD.SetOutputSize( aTmpVD.PixelToLogic( Size( BITMAP_WIDTH * 2, BITMAP_HEIGHT ) ) );
                aXLSet.GetItemSet().Put( XLineStartWidthItem( aTmpVD.GetOutputSize().Height() ) );
                aXLSet.GetItemSet().Put( XLineEndWidthItem( aTmpVD.GetOutputSize().Height() ) );
                aXFSet.GetItemSet().Put( XFillColorItem( String(), Color( COL_WHITE ) ) );
                aXFSet.GetItemSet().Put( XFillStyleItem( XFILL_SOLID ) );
                aXLSet.GetItemSet().Put( XLineStyleItem( XLINE_NONE ) );

                aTmpXOut.SetLineAttr( ( const XLineAttrSetItem& ) pItemPool->Put( aXLSet ) );
                aTmpXOut.SetFillAttr( ( const XFillAttrSetItem& ) pItemPool->Put( aXFSet ) );
                aTmpXOut.DrawRect( Rectangle( Point(), aTmpVD.GetOutputSize() ) );

                aXLSet.GetItemSet().Put( XLineStyleItem( XLINE_SOLID ) );
                aXLSet.GetItemSet().Put( XLineStartItem( String(), aXPoly ) );
                aXLSet.GetItemSet().Put( XLineEndItem( String(), aXPoly ) );

                aTmpXOut.SetLineAttr( ( const XLineAttrSetItem& ) pItemPool->Put( aXLSet ) );
                aTmpXOut.DrawLine( Point( 0, aTmpVD.GetOutputSize().Height() / 2 ),
                                    Point( aTmpVD.GetOutputSize().Width(), aTmpVD.GetOutputSize().Height() / 2 ) );
                Bitmap* pBitmap = new Bitmap( aTmpVD.GetBitmap( Point(), aTmpVD.GetOutputSize() ) );
            */
                pEntry = new XLineEndEntry( aXPoly, aName );
                pLineEndList->Insert( pEntry, LIST_APPEND);
            }
            else
            {
                String aStr(SdResId( STR_WARN_NAME_DUPLICATE ));
                WarningBox aWarningBox( pWin, WinBits( WB_OK ),
                     aStr );
                aWarningBox.Execute();
            }
        }
        delete pDlg;
    }
}


