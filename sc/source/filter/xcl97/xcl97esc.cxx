/*************************************************************************
 *
 *  $RCSfile: xcl97esc.cxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: obo $ $Date: 2004-06-04 11:07:17 $
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


#ifndef _OUTLOBJ_HXX //autogen wg. OutlinerParaObject
#include <svx/outlobj.hxx>
#endif
#ifndef _SVDOTEXT_HXX //autogen wg. SdrTextObj
#include <svx/svdotext.hxx>
#endif
#ifndef _SVDOBJ_HXX //autogen wg. SdrObject
#include <svx/svdobj.hxx>
#endif
#ifndef _SVDOOLE2_HXX //autogen wg. SdrOle2Obj
#include <svx/svdoole2.hxx>
#endif
#ifndef _SVX_UNOAPI_HXX_
#include <svx/unoapi.hxx>
#endif
#ifndef _SVX_FMGLOB_HXX
#include <svx/fmglob.hxx>
#endif
#ifndef _IPOBJ_HXX //autogen wg. SvInPlaceObject
#include <so3/ipobj.hxx>
#endif
#ifndef _SV_OUTDEV_HXX //autogen wg. OutputDevice
#include <vcl/outdev.hxx>
#endif
#ifndef _UNOTOOLS_TEMPFILE_HXX
#include <unotools/tempfile.hxx>
#endif
#ifndef _UNTOOLS_UCBSTREAMHELPER_HXX
#include <unotools/ucbstreamhelper.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX //autogen wg. DBG_ERRORFILE
#include <tools/debug.hxx>
#endif

#include <sot/exchange.hxx>

#include "global.hxx"
#include "document.hxx"
#include "drwlayer.hxx"
#include "xcl97esc.hxx"
#include "xcl97rec.hxx"

#ifndef SC_XCLEXPCHARTS_HXX
#include "XclExpCharts.hxx"
#endif


// --- class XclEscherEx ---------------------------------------------

XclEscherEx::XclEscherEx( SvStream& rStrm, UINT32 nDrawings, RootData& rRoot )
        :
        EscherEx( rStrm, nDrawings ),
        aOcxConverter( *rRoot.pER ),
        rRootData( rRoot ),
        pPicTempFile( NULL ),
        pPicStrm( NULL ),
        pCurrXclObj( NULL ),
        pCurrAppData( NULL ),
        pTheClientData( new XclEscherClientData ),
        pAdditionalText( NULL ),
        nAdditionalText( 0 )
{
    aOffsetMap.Insert( (void*) 0, LIST_APPEND );        // start of stream
    // Xcl forgets OLE objects completely if the Escher object is not EMF and
    // the corresponding OLE application is opened and nothing is changed.
    SetOleEmf( TRUE );
}


XclEscherEx::~XclEscherEx()
{
    DBG_ASSERT( !aStack.Count(), "~XclEscherEx: stack not empty" );
    DeleteCurrAppData();
    delete pTheClientData;
    if ( pPicStrm )
    {
        delete pPicStrm;
    }
    if ( pPicTempFile )
        delete pPicTempFile;
}


SvStream* XclEscherEx::QueryPicStream()
{
    if ( !pPicStrm )
    {
        if ( !pPicTempFile )
        {
            pPicTempFile = new utl::TempFile;
            if ( pPicTempFile->IsValid() )
                pPicTempFile->EnableKillingFile();
            else
            {
                delete pPicTempFile;
                pPicTempFile = NULL;
            }
        }
        if ( pPicTempFile )
        {
            pPicStrm = utl::UcbStreamHelper::CreateStream( pPicTempFile->GetURL(), STREAM_STD_READWRITE );
            pPicStrm->SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );
        }
    }
    return pPicStrm;
}


void XclEscherEx::InsertAtCurrentPos( UINT32 nBytes, BOOL bCont )
{
    ULONG nPos = GetStreamPos();
    ULONG nCnt = aOffsetMap.Count();
    ULONG j, nOff;
    for ( j=0, nOff = (ULONG) aOffsetMap.First(); j<nCnt;
            j++, nOff = (ULONG) aOffsetMap.Next() )
    {
        if ( nOff >= nPos )
            aOffsetMap.Replace( (void*) (nOff + nBytes) );
    }
    EscherEx::InsertAtCurrentPos( nBytes, bCont );
}


ULONG XclEscherEx::AddCurrentOffsetToMap()
{
    aOffsetMap.Insert( (void*) GetStreamPos(), LIST_APPEND );
    return aOffsetMap.Count() - 1;
}


void XclEscherEx::ReplaceCurrentOffsetInMap( ULONG nPos )
{
    aOffsetMap.Replace( (void*) GetStreamPos(), nPos );
}


EscherExHostAppData* XclEscherEx::StartShape( const com::sun::star::uno::Reference<
                                                com::sun::star::drawing::XShape >& rShape )
{
    const XclExpRoot& rRoot = *rRootData.pER;

    if ( nAdditionalText )
        nAdditionalText++;
    BOOL bInGroup = ( pCurrXclObj != NULL );
    if ( bInGroup )
    {   // stacked recursive group object
        if ( !pCurrAppData->IsStackedGroup() )
        {   //! UpdateStopPos only once
            pCurrAppData->SetStackedGroup( TRUE );
            pCurrXclObj->UpdateStopPos();
        }
    }
    aStack.Push( pCurrXclObj );
    aStack.Push( pCurrAppData );
    pCurrAppData = new XclEscherHostAppData;
    const SdrObject* pObj = GetSdrObject( rShape );
    if ( !pObj )
        pCurrXclObj = new XclObjAny( rRoot );  // just what is it?!?
    else
    {
        pCurrXclObj = NULL;
        sal_uInt16 nObjType = pObj->GetObjIdentifier();

        if( nObjType == OBJ_OLE2 )
        {
            //! not-const because GetObjRef may load the OLE object
            SvInPlaceObjectRef xObj( ((SdrOle2Obj*)pObj)->GetObjRef() );
            if ( xObj.Is() )
            {
                SvGlobalName aObjClsId( *xObj->GetSvFactory() );
                if ( SotExchange::IsChart( aObjClsId ) )
                {   // yes, it's a chart diagram
                    rRootData.pObjRecs->Add( new XclObjChart( rRoot, rShape ) );
                    pCurrXclObj = NULL;     // no metafile or whatsoever
                }
                else    // metafile and OLE object
                    pCurrXclObj = new XclObjOle( rRoot, *pObj );
            }
            else    // just a metafile
                pCurrXclObj = new XclObjAny( rRoot );
        }
        else if( nObjType == OBJ_CAPTION )  // #107540# ignore permanent note shapes
        {
            pCurrXclObj = NULL;
        }
        else if( nObjType >= OBJ_FM_CONTROL )
        {
            pCurrXclObj = aOcxConverter.CreateCtrlObj( rShape );
            if( !pCurrXclObj )
                pCurrXclObj = new XclObjAny( rRoot );   // just a metafile
        }
        else
            pCurrXclObj = new XclObjAny( rRoot );   // just a metafile
    }
    if ( pCurrXclObj )
    {
        if ( !rRootData.pObjRecs->Add( pCurrXclObj ) )
        {   // maximum count reached, object got deleted
            pCurrXclObj = NULL;
        }
        else
        {
            pCurrAppData->SetClientData( pTheClientData );
            if ( nAdditionalText == 0 )
            {
                if ( pObj )
                {
                    if ( !bInGroup )
                    {
                        /*  Create a dummy anchor carrying the flags. Real coordinates are
                            calculated later in WriteData(EscherEx&,const Rectangle&). */
                        XclExpEscherAnchor* pAnchor = new XclExpEscherAnchor( rRoot );
                        pAnchor->SetFlags( *pObj );
                        pCurrAppData->SetClientAnchor( pAnchor );
                    }
                    const SdrTextObj* pTextObj = PTR_CAST( SdrTextObj, pObj );
                    if ( pTextObj )
                    {
                        const OutlinerParaObject* pParaObj = pTextObj->GetOutlinerParaObject();
                        if( pParaObj )
                            pCurrAppData->SetClientTextbox(
                                new XclEscherClientTextbox(
                                rRootData, *pTextObj, pCurrXclObj ) );
                    }
                }
                else
                {
                    if ( !bInGroup )
                        pCurrAppData->SetClientAnchor( new XclExpEscherAnchor( rRoot ) );
                }
            }
            else if ( nAdditionalText == 3 )
            {
                if ( pAdditionalText )
                {
                    pAdditionalText->SetXclObj( pCurrXclObj );
                    pCurrAppData->SetClientTextbox( pAdditionalText );
                }
            }
        }
    }
    if ( !pCurrXclObj )
        pCurrAppData->SetDontWriteShape( TRUE );
    return pCurrAppData;
}


void XclEscherEx::EndShape( UINT16 nShapeType, UINT32 nShapeID )
{
    // own escher data created? -> never delete such objects
    bool bOwnEscher = pCurrXclObj && pCurrXclObj->IsOwnEscher();

    // post process the current object - not for objects with own escher data
    if( pCurrXclObj && !bOwnEscher )
    {
        // escher data of last shape not written? -> delete it from object list
        if( nShapeID == 0 )
        {
            XclObj* pLastObj = static_cast< XclObj* >( rRootData.pObjRecs->Last() );
            DBG_ASSERT( pLastObj == pCurrXclObj, "XclEscherEx::EndShape - wrong object" );
            if ( pLastObj == pCurrXclObj )
            {
                rRootData.pObjRecs->Remove();
                DELETEZ( pCurrXclObj );
            }
        }

        if( pCurrXclObj )
        {
            // set shape type
            if ( pCurrAppData->IsStackedGroup() )
                pCurrXclObj->SetEscherShapeTypeGroup();
            else
            {
                pCurrXclObj->SetEscherShapeType( nShapeType );
                pCurrXclObj->UpdateStopPos();
            }
        }
    }

    // get next object from stack
    DeleteCurrAppData();
    pCurrAppData = static_cast< XclEscherHostAppData* >( aStack.Pop() );
    pCurrXclObj = static_cast< XclObj* >( aStack.Pop() );
    if( nAdditionalText == 3 )
        nAdditionalText = 0;
}


EscherExHostAppData* XclEscherEx::EnterAdditionalTextGroup()
{
    nAdditionalText = 1;
    pAdditionalText = (XclEscherClientTextbox*) pCurrAppData->GetClientTextbox();
    pCurrAppData->SetClientTextbox( NULL );
    return pCurrAppData;
}


void XclEscherEx::DeleteCurrAppData()
{
    if ( pCurrAppData )
    {
        delete pCurrAppData->GetClientAnchor();
//      delete pCurrAppData->GetClientData();
        delete pCurrAppData->GetClientTextbox();
        delete pCurrAppData;
    }
}


void XclEscherEx::EndDocument()
{
    Flush( pPicStrm );
}


// --- class XclEscher -----------------------------------------------

XclEscher::XclEscher( UINT32 nDrawings, RootData& rRoot )
{
    pTempFile = new utl::TempFile;
    pTempFile->EnableKillingFile();
    pStrm = utl::UcbStreamHelper::CreateStream( pTempFile->GetURL(), STREAM_STD_READWRITE );
    pStrm->SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );
    pEx = new XclEscherEx( *pStrm, nDrawings, rRoot );
}


XclEscher::~XclEscher()
{
    delete pEx;
    delete pStrm;
    delete pTempFile;
}


void XclEscher::AddSdrPage( RootData& rRootData )
{
    if( ScDrawLayer* pDrawLayer = rRootData.pDoc->GetDrawLayer() )
        if( SdrPage* pPage = pDrawLayer->GetPage(
                    static_cast<sal_uInt16>(rRootData.pER->GetCurrScTab()) ) )
            pEx->AddSdrPage( *pPage );
    // #106213# the first dummy object may still be open
    DBG_ASSERT( pEx->GetGroupLevel() <= 1, "XclEscher::AddSdrPage - still groups open?" );
    while( pEx->GetGroupLevel() )
        pEx->LeaveGroup();
}


// Escher client anchor =======================================================

XclExpEscherAnchor::XclExpEscherAnchor( const XclExpRoot& rRoot, sal_uInt16 nFlags ) :
    XclExpRoot( rRoot ),
    maAnchor( rRoot.GetCurrScTab() ),
    mnFlags( nFlags )
{
}

XclExpEscherAnchor::XclExpEscherAnchor( const XclExpRoot& rRoot, const SdrObject& rSdrObj ) :
    XclExpRoot( rRoot ),
    maAnchor( rRoot.GetCurrScTab() )
{
    SetFlags( rSdrObj );
    maAnchor.SetRect( GetDoc(), rSdrObj.GetCurrentBoundRect(), MAP_100TH_MM );
}

void XclExpEscherAnchor::SetFlags( const SdrObject& rSdrObj )
{
    // Special case "page anchor" (X==0,Y==1) -> lock pos and size.
    const Point& rPos = rSdrObj.GetAnchorPos();
    mnFlags = ((rPos.X() == 0) && (rPos.Y() == 1)) ? EXC_ESC_ANCHOR_LOCKED : 0;
}


BOOL XclExpEscherAnchor::FindNextCol( sal_uInt16& nCol, short nDir )
{
    while ( nDir < 0 ? 0 < nCol : nCol < static_cast<sal_uInt16>(MAXCOL) )
    {
        nCol += nDir;
        if ( !(GetDoc().GetColFlags( static_cast<SCCOL>(nCol),
                        maAnchor.mnScTab) & CR_HIDDEN) )
            return TRUE;
    }
    return FALSE;
}


BOOL XclExpEscherAnchor::FindNextRow( sal_uInt16& nRow, short nDir )
{
    while ( nDir < 0 ? 0 < nRow : nRow < static_cast<sal_uInt16>(MAXROW) )
    {
        nRow += nDir;
        if ( !(GetDoc().GetRowFlags( static_cast<SCROW>(nRow),
                        maAnchor.mnScTab) & CR_HIDDEN) )
            return TRUE;
    }
    return FALSE;
}


void XclExpEscherAnchor::WriteData( EscherEx& rEx, const Rectangle& rRect )
{
    // the rectangle is already in twips
    maAnchor.SetRect( GetDoc(), rRect, MAP_TWIP );
    WriteData( rEx );
}


void XclExpEscherAnchor::WriteData( EscherEx& rEx ) const
{
    rEx.AddAtom( 18, ESCHER_ClientAnchor );
    rEx.GetStream() << mnFlags << maAnchor;
}


// ----------------------------------------------------------------------------

XclExpEscherNoteAnchor::XclExpEscherNoteAnchor( const XclExpRoot& rRoot, const ScAddress& rPos ) :
    XclExpEscherAnchor( rRoot, EXC_ESC_ANCHOR_SIZELOCKED )
{
    BOOL bBad = FALSE;
    maAnchor.mnLCol = static_cast<sal_uInt16>(rPos.Col());
    // go right
    if ( !FindNextCol( maAnchor.mnLCol, 1 ) )
        bBad = TRUE;
    else
    {
        maAnchor.mnRCol = maAnchor.mnLCol;
        bBad = !FindNextCol( maAnchor.mnRCol, 1 );
    }
    if ( bBad )
    {   // go left
        bBad = FALSE;
        maAnchor.mnRCol = static_cast<sal_uInt16>(rPos.Col());
        if ( !FindNextCol( maAnchor.mnRCol, -1 ) )
            maAnchor.mnLCol = maAnchor.mnRCol =
                static_cast<sal_uInt16>(rPos.Col()); // hopeless
        else
        {
            maAnchor.mnLCol = maAnchor.mnRCol;
            if ( !FindNextCol( maAnchor.mnLCol, -1 ) )
                maAnchor.mnLCol = maAnchor.mnRCol;
        }
    }
    if ( maAnchor.mnLCol == maAnchor.mnRCol )
    {
        maAnchor.mnLX = 0;
        maAnchor.mnRX = 1023;
    }
    else
    {
        maAnchor.mnLX = 0x00c0;
        maAnchor.mnRX = 0x0326;
    }

    BOOL bTop = FALSE;
    maAnchor.mnTRow = static_cast<sal_uInt16>(rPos.Row());
    switch ( maAnchor.mnTRow )
    {
        case 0 :
        case 1 :
            maAnchor.mnTRow = 0;
            bTop = (GetDoc().GetRowFlags( static_cast<SCROW>(maAnchor.mnTRow),
                        static_cast<SCTAB>(maAnchor.mnScTab)) & CR_HIDDEN) == 0;
        break;
        default:
            maAnchor.mnTRow -= 2;
    }
    // go down
    if ( !bTop && !FindNextRow( maAnchor.mnTRow, 1 ) )
        bBad = TRUE;
    else
    {
        maAnchor.mnBRow = maAnchor.mnTRow;
        for ( int j=0; j<4 && !bBad; j++ )
        {
            bBad = !FindNextRow( maAnchor.mnBRow, 1 );
        }
    }
    if ( bBad )
    {   // go up
        bBad = FALSE;
        maAnchor.mnBRow = static_cast<sal_uInt16>(rPos.Row());
        if ( !FindNextRow( maAnchor.mnBRow, -1 ) )
            maAnchor.mnTRow = maAnchor.mnBRow =
                static_cast<sal_uInt16>(rPos.Row()); // hopeless
        else
        {
            maAnchor.mnTRow = maAnchor.mnBRow;
            for ( int j=0; j<4 && !bBad; j++ )
            {
                USHORT nBkp = maAnchor.mnTRow;
                if ( !FindNextRow( maAnchor.mnTRow, -1 ) )
                {
                    bBad = TRUE;
                    maAnchor.mnTRow = nBkp;
                }
            }
        }
    }
    if ( maAnchor.mnTRow == maAnchor.mnBRow )
    {
        maAnchor.mnTY = 0;
        maAnchor.mnBY = 255;
    }
    else if ( maAnchor.mnTRow == 0 && maAnchor.mnTRow == rPos.Row() )
    {
        maAnchor.mnTY = 0x001e;
        maAnchor.mnBY = 0x0078;
    }
    else
    {
        maAnchor.mnTY = 0x0069;
        maAnchor.mnBY = 0x00c4;
    }
}


// ----------------------------------------------------------------------------

XclExpEscherDropDownAnchor::XclExpEscherDropDownAnchor( const XclExpRoot& rRoot, const ScAddress& rPos ) :
    XclExpEscherAnchor( rRoot, EXC_ESC_ANCHOR_POSLOCKED )
{
    maAnchor.mnLCol = static_cast<sal_uInt16>(rPos.Col());
    maAnchor.mnTRow = static_cast<sal_uInt16>(rPos.Row());
    maAnchor.mnRCol = maAnchor.mnLCol + 1;
    maAnchor.mnBRow = maAnchor.mnTRow + 1;
    maAnchor.mnLX = maAnchor.mnTY = maAnchor.mnRX = maAnchor.mnBY = 0;
}


// ============================================================================

// --- class XclEscherClientData -------------------------------------

void XclEscherClientData::WriteData( EscherEx& rEx ) const
{   // actual data is in the following OBJ record
    rEx.AddAtom( 0, ESCHER_ClientData );
}


// --- class XclEscherClientTextbox -------------------------------------

XclEscherClientTextbox::XclEscherClientTextbox( RootData& rRoot,
            const SdrTextObj& rObj, XclObj* pObj )
        :
        rRootData( rRoot ),
        rTextObj( rObj ),
        pXclObj( pObj )
{
}


void XclEscherClientTextbox::WriteData( EscherEx& rEx ) const
{
    pXclObj->SetText( *rRootData.pER, rTextObj );
}


