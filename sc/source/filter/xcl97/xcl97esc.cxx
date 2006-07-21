/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xcl97esc.cxx,v $
 *
 *  $Revision: 1.22 $
 *
 *  last change: $Author: kz $ $Date: 2006-07-21 12:37:20 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

#ifndef _XCL97ESC_HXX
#include "xcl97esc.hxx"
#endif

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
#ifndef _SDASITM_HXX
#include <svx/sdasitm.hxx>
#endif

#include <sot/exchange.hxx>

#ifndef SC_XEESCHER_HXX
#include "xeescher.hxx"
#endif

#include "global.hxx"
#include "document.hxx"
#include "drwlayer.hxx"
#include "xcl97rec.hxx"

#ifndef SC_XEHELPER_HXX
#include "xehelper.hxx"
#endif
#ifndef SC_XCLEXPCHARTS_HXX
#include "XclExpCharts.hxx"
#endif

#ifndef _COM_SUN_STAR_EMBED_XCLASSIFIEDOBJECT_HPP_
#include <com/sun/star/embed/XClassifiedObject.hpp>
#endif

using namespace com::sun::star;

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

sal_Bool ImplXclEscherExIsFontwork( const SdrObject* pObj )
{
    const rtl::OUString sTextPath( RTL_CONSTASCII_USTRINGPARAM ( "TextPath" ) );

    sal_Bool bIsFontwork = sal_False;
    if ( pObj->GetObjIdentifier() == OBJ_CUSTOMSHAPE )
    {
        SdrCustomShapeGeometryItem& rGeometryItem = (SdrCustomShapeGeometryItem&)
            pObj->GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY );

        com::sun::star::uno::Any* pAny = rGeometryItem.GetPropertyValueByName( sTextPath, sTextPath );
        if ( pAny )
            *pAny >>= bIsFontwork;
    }
    return bIsFontwork;
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
            uno::Reference < embed::XClassifiedObject > xObj( ((SdrOle2Obj*)pObj)->GetObjRef(), uno::UNO_QUERY );
            if ( xObj.is() )
            {
                SvGlobalName aObjClsId( xObj->getClassID() );
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
        else if( nObjType >= OBJ_FM_CONTROL )
        {
            pCurrXclObj = aOcxConverter.CreateCtrlObj( rShape );
            if( !pCurrXclObj )
                pCurrXclObj = new XclObjAny( rRoot );   // just a metafile
        }
        else if( pObj->GetLayer() != SC_LAYER_INTERN )
        {
            // #107540# ignore permanent note shapes
            // #i12190# do not ignore callouts (do not filter by object type ID)
            pCurrXclObj = new XclObjAny( rRoot );   // just a metafile
        }
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
                    if ( pTextObj && !ImplXclEscherExIsFontwork( pTextObj ) && ( pObj->GetObjIdentifier() != OBJ_CAPTION ) )
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


void XclEscher::AddSdrPage( const XclExpRoot& rRoot )
{
    if( SdrPage* pPage = rRoot.GetSdrPage( rRoot.GetCurrScTab() ) )
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

XclExpEscherNoteAnchor::XclExpEscherNoteAnchor( const XclExpRoot& rRoot, const Rectangle& rRect ) :
    XclExpEscherAnchor( rRoot, EXC_ESC_ANCHOR_SIZELOCKED )
{
    maAnchor.SetRect( GetDoc(), rRect, MAP_100TH_MM );
}


// ----------------------------------------------------------------------------

XclExpEscherDropDownAnchor::XclExpEscherDropDownAnchor( const XclExpRoot& rRoot, const ScAddress& rScPos ) :
    XclExpEscherAnchor( rRoot, EXC_ESC_ANCHOR_POSLOCKED )
{
    GetAddressConverter().ConvertAddress( maAnchor.maXclRange.maFirst, rScPos, true );
    maAnchor.maXclRange.maLast.mnCol = maAnchor.maXclRange.maFirst.mnCol + 1;
    maAnchor.maXclRange.maLast.mnRow = maAnchor.maXclRange.maFirst.mnRow + 1;
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


