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
#include <svtools/htmlkywd.hxx>
#include <svtools/htmlout.hxx>
#include <svl/urihelper.hxx>
#include <tools/urlobj.hxx>

#include <htmlexp.hxx>
#include <global.hxx>
#include <document.hxx>
#include <drwlayer.hxx>
#include <rtl/strbuf.hxx>

using namespace com::sun::star;

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
                    bTabAlignedLeft = true;
                    break;
                }
            }
        }
    }
}

void ScHTMLExport::FillGraphList( const SdrPage* pPage, SCTAB nTab,
        SCCOL nStartCol, SCROW nStartRow,   SCCOL nEndCol, SCROW nEndRow )
{
    if ( pPage->GetObjCount() )
    {
        tools::Rectangle aRect;
        if ( !bAll )
            aRect = pDoc->GetMMRect( nStartCol, nStartRow, nEndCol, nEndRow, nTab );
        SdrObjListIter aIter( pPage, SdrIterMode::Flat );
        SdrObject* pObject = aIter.Next();
        while ( pObject )
        {
            tools::Rectangle aObjRect = pObject->GetCurrentBoundRect();
            if ( (bAll || aRect.IsInside( aObjRect )) && !ScDrawLayer::IsNoteCaption(pObject) )
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
                bool bInCell = (pDoc->GetEmptyLinesInBlock(
                    nCol1, nRow1, nTab, nCol2, nRow2, nTab, DIR_TOP )
                    == static_cast< SCSIZE >( nRow2 - nRow1 ));    // rows-1 !
                if ( bInCell )
                {   // Spacing in spanning cell
                    tools::Rectangle aCellRect = pDoc->GetMMRect(
                        nCol1, nRow1, nCol2, nRow2, nTab );
                    aSpace = MMToPixel( Size(
                        aCellRect.GetWidth() - aObjRect.GetWidth(),
                        aCellRect.GetHeight() - aObjRect.GetHeight() ));
                    aSpace.AdjustWidth((nCol2-nCol1) * (nCellSpacing+1) );
                    aSpace.AdjustHeight((nRow2-nRow1) * (nCellSpacing+1) );
                    aSpace.setWidth( aSpace.Width() / 2 );
                    aSpace.setHeight( aSpace.Height() / 2 );
                }
                aGraphList.emplace_back( pObject,
                    aR, aSize, bInCell, aSpace );
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
            const SdrGrafObj* pSGO = static_cast<SdrGrafObj*>(pObject);
            const SdrGrafObjGeoData* pGeo = static_cast<SdrGrafObjGeoData*>(pSGO->GetGeoData());
            sal_uInt16 nMirrorCase = (pGeo->aGeo.nRotationAngle == 18000 ?
                    ( pGeo->bMirrored ? 3 : 4 ) : ( pGeo->bMirrored ? 2 : 1 ));
            bool bHMirr = ( ( nMirrorCase == 2 ) || ( nMirrorCase == 4 ) );
            bool bVMirr = ( ( nMirrorCase == 3 ) || ( nMirrorCase == 4 ) );
            XOutFlags nXOutFlags = XOutFlags::NONE;
            if ( bHMirr )
                nXOutFlags |= XOutFlags::MirrorHorz;
            if ( bVMirr )
                nXOutFlags |= XOutFlags::MirrorVert;
            OUString aLinkName;
            if ( pSGO->IsLinkedGraphic() )
                aLinkName = pSGO->GetFileName();
            WriteImage( aLinkName, pSGO->GetGraphic(), aOpt, nXOutFlags );
            pE->bWritten = true;
        }
        break;
        case OBJ_OLE2:
        {
            const Graphic* pGraphic = static_cast<SdrOle2Obj*>(pObject)->GetGraphic();
            if ( pGraphic )
            {
                OUString aLinkName;
                WriteImage( aLinkName, *pGraphic, aOpt );
                pE->bWritten = true;
            }
        }
        break;
        default:
        {
            Graphic aGraph(SdrExchangeView::GetObjGraphic(*pObject));
            OUString aLinkName;
            WriteImage( aLinkName, aGraph, aOpt );
            pE->bWritten = true;
        }
    }
}

void ScHTMLExport::WriteImage( OUString& rLinkName, const Graphic& rGrf,
            const OString& rImgOptions, XOutFlags nXOutFlags )
{
    // Embedded graphic -> create an image file
    if( rLinkName.isEmpty() )
    {
        if( !aStreamPath.isEmpty() )
        {
            // Save as a PNG
            OUString aGrfNm( aStreamPath );
            nXOutFlags |= XOutFlags::UseNativeIfPossible;
            ErrCode nErr = XOutBitmap::WriteGraphic( rGrf, aGrfNm,
                "PNG", nXOutFlags );

            // If it worked, create a URL for the IMG tag
            if( !nErr )
            {
                rLinkName = URIHelper::SmartRel2Abs(
                        INetURLObject(aBaseURL),
                        aGrfNm,
                        URIHelper::GetMaybeFileHdl());
            }
        }
    }
    else
    {
        // Linked graphic - figure out the URL for the IMG tag
        if( bCopyLocalFileToINet )
        {
            CopyLocalFileToINet( rLinkName, aStreamPath );
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
        rStrm.WriteChar( '<' ).WriteCharPtr( OOO_STRING_SVTOOLS_HTML_image ).WriteChar( ' ' ).WriteCharPtr( OOO_STRING_SVTOOLS_HTML_O_src ).WriteCharPtr( "=\"" );
        HTMLOutFuncs::Out_String( rStrm, URIHelper::simpleNormalizedMakeRelative(
                    aBaseURL,
                    rLinkName ), eDestEnc ).WriteChar( '\"' );
        if ( !rImgOptions.isEmpty() )
            rStrm.WriteCharPtr( rImgOptions.getStr() );
        rStrm.WriteChar( '>' ).WriteCharPtr( SAL_NEWLINE_STRING ).WriteCharPtr( GetIndentStr() );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
