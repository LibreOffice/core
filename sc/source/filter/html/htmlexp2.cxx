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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"



//------------------------------------------------------------------------

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
#include <svx/svdlegacy.hxx>

#include "htmlexp.hxx"
#include "global.hxx"
#include "document.hxx"
#include "drwlayer.hxx"
#include "ftools.hxx"

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
            bTabHasGraphics = sal_True;
            FillGraphList( pDrawPage, nTab,
                nStartCol, nStartRow, nEndCol, nEndRow );
            for ( ScHTMLGraphEntry* pE = aGraphList.First(); pE;
                    pE = aGraphList.Next() )
            {
                if ( !pE->bInCell )
                {   // nicht alle in Zellen: einige neben Tabelle
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
            Rectangle aObjRect = sdr::legacy::GetBoundRect(*pObject);
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
                aGraphList.Insert( pE, LIST_APPEND );
            }
            pObject = aIter.Next();
        }
    }
}


void ScHTMLExport::WriteGraphEntry( ScHTMLGraphEntry* pE )
{
    SdrObject* pObject = pE->pObject;
    ByteString aOpt;
    (((aOpt += ' ') += OOO_STRING_SVTOOLS_HTML_O_width) += '=') +=
        ByteString::CreateFromInt32( pE->aSize.Width() );
    (((aOpt += ' ') += OOO_STRING_SVTOOLS_HTML_O_height) += '=') +=
        ByteString::CreateFromInt32( pE->aSize.Height() );
    if ( pE->bInCell )
    {
        (((aOpt += ' ') += OOO_STRING_SVTOOLS_HTML_O_hspace) += '=') +=
            ByteString::CreateFromInt32( pE->aSpace.Width() );
        (((aOpt += ' ') += OOO_STRING_SVTOOLS_HTML_O_vspace) += '=') +=
            ByteString::CreateFromInt32( pE->aSpace.Height() );
    }
    switch ( pObject->GetObjIdentifier() )
    {
        case OBJ_GRAF:
        {
            const SdrGrafObj* pSGO = (SdrGrafObj*)pObject;
            const basegfx::B2DVector& rScale = pSGO->getSdrObjectScale();
            sal_uLong nXOutFlags = 0;

            if(basegfx::fTools::less(rScale.getX(), 0.0))
            {
                nXOutFlags |= XOUTBMP_MIRROR_HORZ;
            }

            if(basegfx::fTools::less(rScale.getY(), 0.0))
            {
                nXOutFlags |= XOUTBMP_MIRROR_VERT;
            }

            String aLinkName;
            if ( pSGO->IsLinkedGraphic() )
                aLinkName = pSGO->GetFileName();
            WriteImage( aLinkName, pSGO->GetGraphic(), aOpt, nXOutFlags );
            pE->bWritten = sal_True;
        }
        break;
        case OBJ_OLE2:
        {
            Graphic* pGraphic = ((SdrOle2Obj*)pObject)->GetGraphic();
            if ( pGraphic )
            {
                String aLinkName;
                WriteImage( aLinkName, *pGraphic, aOpt );
                pE->bWritten = sal_True;
            }
        }
        break;
        default:
        {
            const Graphic aGraph(GetObjGraphic(*pObject));
            String aLinkName;
            WriteImage( aLinkName, aGraph, aOpt );
            pE->bWritten = sal_True;
        }
    }
}


void ScHTMLExport::WriteImage( String& rLinkName, const Graphic& rGrf,
            const ByteString& rImgOptions, sal_uLong nXOutFlags )
{
    // embeddete Grafik -> via WriteGraphic schreiben
    if( !rLinkName.Len() )
    {
        if( aStreamPath.Len() > 0 )
        {
            // Grafik als (JPG-)File speichern
            String aGrfNm( aStreamPath );
            nXOutFlags |= XOUTBMP_USE_NATIVE_IF_POSSIBLE;
            sal_uInt16 nErr = XOutBitmap::WriteGraphic( rGrf, aGrfNm,
                CREATE_STRING( "JPG" ), nXOutFlags );
            if( !nErr )     // sonst fehlerhaft, da ist nichts auszugeben
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
    if( rLinkName.Len() )
    {   // <IMG SRC="..."[ rImgOptions]>
        rStrm << '<' << OOO_STRING_SVTOOLS_HTML_image << ' ' << OOO_STRING_SVTOOLS_HTML_O_src << "=\"";
        HTMLOutFuncs::Out_String( rStrm, URIHelper::simpleNormalizedMakeRelative(
                    aBaseURL,
                    rLinkName ), eDestEnc ) << '\"';
        if ( rImgOptions.Len() )
            rStrm << rImgOptions.GetBuffer();
        rStrm << '>' << sNewLine << GetIndentStr();
    }
}




