/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <svx/svditer.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdpage.hxx>
#include <svx/xoutbmp.hxx>
#include <svx/svdxcgv.hxx>
#include <sot/exchange.hxx>
#include <svtools/htmlkywd.hxx>
#include <svtools/htmlout.hxx>
#include <svtools/transfer.hxx>
#include <svtools/embedtransfer.hxx>
#include <svl/urihelper.hxx>
#include <tools/urlobj.hxx>

#include "htmlexp.hxx"
#include "global.hxx"
#include "document.hxx"
#include "drwlayer.hxx"
#include "ftools.hxx"
#include <rtl/strbuf.hxx>

using namespace com::sun::star;

//------------------------------------------------------------------------

void ScHTMLExport::PrepareGraphics( ScDrawLayer* pDrawLayer, SCTAB nTab,
        SCCOL nStartCol, SCROW nStartRow,   SCCOL nEndCol, SCROW nEndRow )
{
    if ( pDrawLayer->HasObjectsInRows( nTab, nStartRow, nEndRow ) )
    {
        SdrPage* pDrawPage = pDrawLayer->GetPage( static_cast<sal_uInt16>(nTab) );
        if ( pDrawPage )
        {
            bTabHasGraphics = true;
            FillGraphList( pDrawPage, nTab, nStartCol, nStartRow, nEndCol, nEndRow );
            size_t ListSize = aGraphList.size();
            for ( size_t i = 0; i < ListSize; ++i )
            {
                ScHTMLGraphEntry* pE = &aGraphList[ i ];
                if ( !pE->bInCell )
                {   // not all cells: table next to some
                    bTabAlignedLeft = sal_True;
                    break;
                }
            }
        }
    }
}


void ScHTMLExport::FillGraphList( const SdrPage* pPage, SCTAB nTab,
        SCCOL nStartCol, SCROW nStartRow,   SCCOL nEndCol, SCROW nEndRow )
{
    sal_uLong   nObjCount = pPage->GetObjCount();
    if ( nObjCount )
    {
        Rectangle aRect;
        if ( !bAll )
            aRect = pDoc->GetMMRect( nStartCol, nStartRow, nEndCol, nEndRow, nTab );
        SdrObjListIter aIter( *pPage, IM_FLAT );
        SdrObject* pObject = aIter.Next();
        while ( pObject )
        {
            Rectangle aObjRect = pObject->GetCurrentBoundRect();
            if ( bAll || aRect.IsInside( aObjRect ) )
            {
                Size aSpace;
                ScRange aR = pDoc->GetRange( nTab, aObjRect );
                // Rectangle in mm/100
                Size aSize( MMToPixel( aObjRect.GetSize() ) );
                // If the image is somewhere in a merged range we must
                // move the anchor to the upper left (THE span cell).
                pDoc->ExtendOverlapped( aR );
                SCCOL nCol1 = aR.aStart.Col();
                SCROW nRow1 = aR.aStart.Row();
                SCCOL nCol2 = aR.aEnd.Col();
                SCROW nRow2 = aR.aEnd.Row();
                // All cells empty under object?
                sal_Bool bInCell = (pDoc->GetEmptyLinesInBlock(
                    nCol1, nRow1, nTab, nCol2, nRow2, nTab, DIR_TOP )
                    == static_cast< SCSIZE >( nRow2 - nRow1 ));    // rows-1 !
                if ( bInCell )
                {   // Spacing in spanning cell
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
                aGraphList.push_back( pE );
            }
            pObject = aIter.Next();
        }
    }
}


void ScHTMLExport::WriteGraphEntry( ScHTMLGraphEntry* pE )
{
    SdrObject* pObject = pE->pObject;
    OStringBuffer aBuf;
    aBuf.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_width).append('=').
        append(static_cast<sal_Int32>(pE->aSize.Width()));
    aBuf.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_height).append('=').
        append(static_cast<sal_Int32>(pE->aSize.Height()));
    if ( pE->bInCell )
    {
        aBuf.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_hspace).append('=').
            append(static_cast<sal_Int32>(pE->aSpace.Width()));
        aBuf.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_vspace).append('=').
            append(static_cast<sal_Int32>(pE->aSpace.Height()));
    }
    OString aOpt = aBuf.makeStringAndClear();
    switch ( pObject->GetObjIdentifier() )
    {
        case OBJ_GRAF:
        {
            const SdrGrafObj* pSGO = (SdrGrafObj*)pObject;
            const SdrGrafObjGeoData* pGeo = (SdrGrafObjGeoData*)pSGO->GetGeoData();
            sal_uInt16 nMirrorCase = (pGeo->aGeo.nDrehWink == 18000 ?
                    ( pGeo->bMirrored ? 3 : 4 ) : ( pGeo->bMirrored ? 2 : 1 ));
            sal_Bool bHMirr = ( ( nMirrorCase == 2 ) || ( nMirrorCase == 4 ) );
            sal_Bool bVMirr = ( ( nMirrorCase == 3 ) || ( nMirrorCase == 4 ) );
            sal_uLong nXOutFlags = 0;
            if ( bHMirr )
                nXOutFlags |= XOUTBMP_MIRROR_HORZ;
            if ( bVMirr )
                nXOutFlags |= XOUTBMP_MIRROR_VERT;
            OUString aLinkName;
            if ( pSGO->IsLinkedGraphic() )
                aLinkName = pSGO->GetFileName();
            WriteImage( aLinkName, pSGO->GetGraphic(), aOpt, nXOutFlags );
            pE->bWritten = sal_True;
        }
        break;
        case OBJ_OLE2:
        {
            const Graphic* pGraphic = static_cast<SdrOle2Obj*>(pObject)->GetGraphic();
            if ( pGraphic )
            {
                OUString aLinkName;
                WriteImage( aLinkName, *pGraphic, aOpt );
                pE->bWritten = sal_True;
            }
        }
        break;
        default:
        {
            Graphic aGraph( SdrExchangeView::GetObjGraphic(
                pDoc->GetDrawLayer(), pObject ) );
            OUString aLinkName;
            WriteImage( aLinkName, aGraph, aOpt );
            pE->bWritten = sal_True;
        }
    }
}


void ScHTMLExport::WriteImage( OUString& rLinkName, const Graphic& rGrf,
            const OString& rImgOptions, sal_uLong nXOutFlags )
{
    // Embedded graphic -> create an image file
    if( rLinkName.isEmpty() )
    {
        if( !aStreamPath.isEmpty() )
        {
            // Save as a PNG
            OUString aGrfNm( aStreamPath );
            nXOutFlags |= XOUTBMP_USE_NATIVE_IF_POSSIBLE;
            sal_uInt16 nErr = XOutBitmap::WriteGraphic( rGrf, aGrfNm,
                "PNG", nXOutFlags );

            // If it worked, create a URL for the IMG tag
            if( !nErr )
            {
                rLinkName = URIHelper::SmartRel2Abs(
                        INetURLObject(aBaseURL),
                        aGrfNm,
                        URIHelper::GetMaybeFileHdl());
                if ( HasCId() )
                    MakeCIdURL( rLinkName );
            }
        }
    }
    else
    {
        // Linked graphic - figure out the URL for the IMG tag
        if( bCopyLocalFileToINet || HasCId() )
        {
            CopyLocalFileToINet( rLinkName, aStreamPath );
            if ( HasCId() )
                MakeCIdURL( rLinkName );
        }
        else
            rLinkName = URIHelper::SmartRel2Abs(
                    INetURLObject(aBaseURL),
                    rLinkName,
                    URIHelper::GetMaybeFileHdl());
    }

    // If a URL was set, output the IMG tag.
    // <IMG SRC="..."[ rImgOptions]>
    if( !rLinkName.isEmpty() )
    {
        rStrm << '<' << OOO_STRING_SVTOOLS_HTML_image << ' ' << OOO_STRING_SVTOOLS_HTML_O_src << "=\"";
        HTMLOutFuncs::Out_String( rStrm, URIHelper::simpleNormalizedMakeRelative(
                    aBaseURL,
                    rLinkName ), eDestEnc ) << '\"';
        if ( !rImgOptions.isEmpty() )
            rStrm << rImgOptions.getStr();
        rStrm << '>' << sNewLine << GetIndentStr();
    }
}




/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
