/*************************************************************************
 *
 *  $RCSfile: htmlexp2.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:45:12 $
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
#include "filt_pch.hxx"
#endif

#pragma hdrstop

//------------------------------------------------------------------------

#ifdef MAC
#define _SYSDEP_HXX
#endif

#include <svx/svditer.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdpage.hxx>
#include <svx/xoutbmp.hxx>
#include <so3/ipobj.hxx>
#include <sot/exchange.hxx>
#include <svtools/htmlkywd.hxx>
#include <svtools/htmlout.hxx>
#ifndef SVTOOLS_URIHELPER_HXX
#include <svtools/urihelper.hxx>
#endif
#include <tools/urlobj.hxx>

#if defined(WIN) || defined(WNT)
#ifndef _SVWIN_H
#include <tools/svwin.h>
#endif
#endif

#include "htmlexp.hxx"
#include "global.hxx"
#include "flttools.hxx"
#include "document.hxx"
#include "drwlayer.hxx"


//------------------------------------------------------------------------

void ScHTMLExport::PrepareGraphics( ScDrawLayer* pDrawLayer, USHORT nTab,
        USHORT nStartCol, USHORT nStartRow, USHORT nEndCol, USHORT nEndRow )
{
    if ( pDrawLayer->HasObjectsInRows( nTab, nStartRow, nEndRow ) )
    {
        SdrPage* pDrawPage = pDrawLayer->GetPage( nTab );
        if ( pDrawPage )
        {
            bTabHasGraphics = TRUE;
            FillGraphList( pDrawPage, nTab,
                nStartCol, nStartRow, nEndCol, nEndRow );
            for ( ScHTMLGraphEntry* pE = aGraphList.First(); pE;
                    pE = aGraphList.Next() )
            {
                if ( !pE->bInCell )
                {   // nicht alle in Zellen: einige neben Tabelle
                    bTabAlignedLeft = TRUE;
                    break;
                }
            }
        }
    }
}


void ScHTMLExport::FillGraphList( const SdrPage* pPage, USHORT nTab,
        USHORT nStartCol, USHORT nStartRow, USHORT nEndCol, USHORT nEndRow )
{
    ULONG   nObjCount = pPage->GetObjCount();
    if ( nObjCount )
    {
        Rectangle aRect;
        if ( !bAll )
            aRect = pDoc->GetMMRect( nStartCol, nStartRow, nEndCol, nEndRow, nTab );
        SdrObjListIter aIter( *pPage, IM_FLAT );
        SdrObject* pObject = aIter.Next();
        while ( pObject )
        {
            Rectangle aObjRect = pObject->GetBoundRect();
            if ( bAll || aRect.IsInside( aObjRect ) )
            {
                switch ( pObject->GetObjIdentifier() )
                {
                    case OBJ_GRAF:
                    case OBJ_OLE2:
                    {
                        Size aSpace;
                        ScRange aR = pDoc->GetRange( nTab, aObjRect );
                        // Rectangle in mm/100
                        Size aSize( MMToPixel( aObjRect.GetSize() ) );
                        USHORT nCol1 = aR.aStart.Col();
                        USHORT nRow1 = aR.aStart.Row();
                        USHORT nCol2 = aR.aEnd.Col();
                        USHORT nRow2 = aR.aEnd.Row();
                        // alle Zellen unter der Grafik leer?
                        BOOL bInCell = (pDoc->GetEmptyLinesInBlock(
                            nCol1, nRow1, nTab, nCol2, nRow2, nTab, DIR_TOP )
                            == (nRow2 - nRow1));    // rows-1 !
                        if ( bInCell )
                        {   // Spacing innerhalb der Span-Cell
                            Rectangle aCellRect = pDoc->GetMMRect(
                                nCol1, nRow1, nCol2, nRow2, nTab );
                            aSpace = MMToPixel( Size(
                                aCellRect.GetWidth() - aObjRect.GetWidth(),
                                aCellRect.GetHeight() - aObjRect.GetHeight() ));
                            aSpace.Width() += (nCol2-nCol1) * (nCellSpacing+1);
                            aSpace.Height() += (nRow2-nRow1) * (nCellSpacing+1);
                            aSpace.Width() /= 2;
                            aSpace.Height() /= 2;
                        }
                        ScHTMLGraphEntry* pE = new ScHTMLGraphEntry( pObject,
                            aR, aSize, bInCell, aSpace );
                        aGraphList.Insert( pE, LIST_APPEND );
                    }
                    break;
                    default:
                        DBG_ERRORFILE( "FillGraphList: kein OBJ_GRAF, kein OBJ_OLE2, unnu?" );
                }
            }
            pObject = aIter.Next();
        }
    }
}


void ScHTMLExport::WriteGraphEntry( ScHTMLGraphEntry* pE )
{
    SdrObject* pObject = pE->pObject;
    ByteString aOpt;
    (((aOpt += ' ') += sHTML_O_width) += '=') +=
        ByteString::CreateFromInt32( pE->aSize.Width() );
    (((aOpt += ' ') += sHTML_O_height) += '=') +=
        ByteString::CreateFromInt32( pE->aSize.Height() );
    if ( pE->bInCell )
    {
        (((aOpt += ' ') += sHTML_O_hspace) += '=') +=
            ByteString::CreateFromInt32( pE->aSpace.Width() );
        (((aOpt += ' ') += sHTML_O_vspace) += '=') +=
            ByteString::CreateFromInt32( pE->aSpace.Height() );
    }
    switch ( pObject->GetObjIdentifier() )
    {
        case OBJ_GRAF:
        {
            const SdrGrafObj* pSGO = (SdrGrafObj*)pObject;
            const SdrGrafObjGeoData* pGeo = (SdrGrafObjGeoData*)pSGO->GetGeoData();
            USHORT nMirrorCase = (pGeo->aGeo.nDrehWink == 18000 ?
                    ( pGeo->bMirrored ? 3 : 4 ) : ( pGeo->bMirrored ? 2 : 1 ));
            BOOL bHMirr = ( ( nMirrorCase == 2 ) || ( nMirrorCase == 4 ) );
            BOOL bVMirr = ( ( nMirrorCase == 3 ) || ( nMirrorCase == 4 ) );
            ULONG nXOutFlags = 0;
            if ( bHMirr )
                nXOutFlags |= XOUTBMP_MIRROR_HORZ;
            if ( bVMirr )
                nXOutFlags |= XOUTBMP_MIRROR_VERT;
            String aLinkName;
            if ( pSGO->IsLinkedGraphic() )
                aLinkName = pSGO->GetFileName();
            WriteImage( aLinkName, pSGO->GetGraphic(), aOpt, nXOutFlags );
            pE->bWritten = TRUE;
        }
        break;
        case OBJ_OLE2:
        {
            const SvInPlaceObjectRef& rRef = ((SdrOle2Obj*)pObject)->GetObjRef();
            GDIMetaFile* pPic = NULL;
            SvData aData( FORMAT_GDIMETAFILE );
            if( rRef->GetData( &aData ) )
                aData.GetData( &pPic, TRANSFER_REFERENCE );
            if( pPic )
            {
                Graphic aGraph( *pPic );
                String aLinkName;
                WriteImage( aLinkName, aGraph, aOpt );
                pE->bWritten = TRUE;
            }
        }
        break;
    }
}


void ScHTMLExport::WriteImage( String& rLinkName, const Graphic& rGrf,
            const ByteString& rImgOptions, ULONG nXOutFlags )
{
    // embeddete Grafik -> via WriteGraphic schreiben
    if( !rLinkName.Len() )
    {
        if( aStreamPath.Len() > 0 )
        {
            // Grafik als (JPG-)File speichern
            String aGrfNm( aStreamPath );
            USHORT nErr = XOutBitmap::WriteGraphic( rGrf, aGrfNm, _STRINGCONST( "JPG" ), nXOutFlags );
            if( !nErr )     // sonst fehlerhaft, da ist nichts auszugeben
            {
                rLinkName = URIHelper::SmartRelToAbs( aGrfNm );
                if ( HasCId() )
                    MakeCIdURL( rLinkName );
            }
        }
    }
    else
    {
        if( bCopyLocalFileToINet || HasCId() )
        {
            CopyLocalFileToINet( rLinkName, aStreamPath );
            if ( HasCId() )
                MakeCIdURL( rLinkName );
        }
        else
            rLinkName = URIHelper::SmartRelToAbs( rLinkName );
    }
    if( rLinkName.Len() )
    {   // <IMG SRC="..."[ rImgOptions]>
        rStrm << '<' << sHTML_image << ' ' << sHTML_O_src << "=\"";
        HTMLOutFuncs::Out_String( rStrm, INetURLObject::AbsToRel( rLinkName ), eDestEnc ) << '\"';
        if ( rImgOptions.Len() )
            rStrm << rImgOptions.GetBuffer();
        rStrm << '>' << sNewLine << GetIndentStr();
    }
}




