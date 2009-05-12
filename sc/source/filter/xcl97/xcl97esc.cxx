/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: xcl97esc.cxx,v $
 * $Revision: 1.26.128.1 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

#include <com/sun/star/awt/XControlModel.hpp>
#include <com/sun/star/embed/XClassifiedObject.hpp>
#include <com/sun/star/form/XFormsSupplier.hpp>
#include <com/sun/star/script/ScriptEventDescriptor.hpp>
#include <com/sun/star/script/XEventAttacherManager.hpp>

#include <svx/svdpage.hxx>
#include <svx/outlobj.hxx>
#include <svx/svdotext.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdoole2.hxx>
#include <svx/unoapi.hxx>
#include <svx/fmglob.hxx>
#include <svx/msocximex.hxx>
#include <vcl/outdev.hxx>
#include <unotools/tempfile.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <tools/debug.hxx>
#include <svx/sdasitm.hxx>

#include <sot/exchange.hxx>
#include "xeescher.hxx"

#include "global.hxx"
#include "document.hxx"
#include "drwlayer.hxx"
#include "xcl97rec.hxx"
#include "xehelper.hxx"
#include "xechart.hxx"
#include "xcl97esc.hxx"

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::UNO_QUERY_THROW;
using ::com::sun::star::container::XIndexAccess;
using ::com::sun::star::embed::XClassifiedObject;
using ::com::sun::star::drawing::XShape;
using ::com::sun::star::awt::XControlModel;
using ::com::sun::star::form::XFormsSupplier;
using ::com::sun::star::script::ScriptEventDescriptor;
using ::com::sun::star::script::XEventAttacherManager;

// --- class XclEscherEx ---------------------------------------------

XclEscherEx::XclEscherEx( const XclExpRoot& rRoot, SvStream& rStrm, UINT32 nDrawings )
        :
        EscherEx( rStrm, nDrawings ),
        XclExpRoot( rRoot ),
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
    SdrObject* pObj = GetSdrObjectFromXShape( rShape );
    if ( !pObj )
        pCurrXclObj = new XclObjAny( GetRoot() );  // just what is it?!?
    else
    {
        pCurrXclObj = NULL;
        sal_uInt16 nObjType = pObj->GetObjIdentifier();

        if( nObjType == OBJ_OLE2 )
        {
            //! not-const because GetObjRef may load the OLE object
            Reference < XClassifiedObject > xObj( ((SdrOle2Obj*)pObj)->GetObjRef(), UNO_QUERY );
            if ( xObj.is() )
            {
                SvGlobalName aObjClsId( xObj->getClassID() );
                if ( SotExchange::IsChart( aObjClsId ) )
                {   // yes, it's a chart diagram
                    GetOldRoot().pObjRecs->Add( new XclExpChartObj( GetRoot(), rShape ) );
                    pCurrXclObj = NULL;     // no metafile or whatsoever
                }
                else    // metafile and OLE object
                    pCurrXclObj = new XclObjOle( GetRoot(), *pObj );
            }
            else    // just a metafile
                pCurrXclObj = new XclObjAny( GetRoot() );
        }
        else if( nObjType == OBJ_UNO )
        {
            pCurrXclObj = CreateCtrlObj( rShape );
            if( !pCurrXclObj )
                pCurrXclObj = new XclObjAny( GetRoot() );   // just a metafile
        }
        else if( !ScDrawLayer::IsNoteCaption( pObj ) )
        {
            // #107540# ignore permanent note shapes
            // #i12190# do not ignore callouts (do not filter by object type ID)
            pCurrXclObj = new XclObjAny( GetRoot() );   // just a metafile
        }
    }
    if ( pCurrXclObj )
    {
        if ( !GetOldRoot().pObjRecs->Add( pCurrXclObj ) )
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
                        XclExpDffAnchor* pAnchor = new XclExpDffAnchor( GetRoot() );
                        pAnchor->SetFlags( *pObj );
                        pCurrAppData->SetClientAnchor( pAnchor );
                    }
                    const SdrTextObj* pTextObj = PTR_CAST( SdrTextObj, pObj );
                    if ( pTextObj && !ImplXclEscherExIsFontwork( pTextObj ) && ( pObj->GetObjIdentifier() != OBJ_CAPTION ) )
                    {
                        const OutlinerParaObject* pParaObj = pTextObj->GetOutlinerParaObject();
                        if( pParaObj )
                            pCurrAppData->SetClientTextbox(
                                new XclEscherClientTextbox( GetRoot(), *pTextObj, pCurrXclObj ) );
                    }
                }
                else
                {
                    if ( !bInGroup )
                        pCurrAppData->SetClientAnchor( new XclExpDffAnchor( GetRoot() ) );
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
            XclObj* pLastObj = static_cast< XclObj* >( GetOldRoot().pObjRecs->Last() );
            DBG_ASSERT( pLastObj == pCurrXclObj, "XclEscherEx::EndShape - wrong object" );
            if ( pLastObj == pCurrXclObj )
            {
                GetOldRoot().pObjRecs->Remove();
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

#if EXC_EXP_OCX_CTRL

XclExpOcxControlObj* XclEscherEx::CreateCtrlObj( Reference< XShape > xShape )
{
    ::std::auto_ptr< XclExpOcxControlObj > xOcxCtrl;

    Reference< XControlModel > xCtrlModel = XclControlHelper::GetControlModel( xShape );
    if( xCtrlModel.is() )
    {
        // output stream
        if( !mxCtlsStrm.Is() )
            mxCtlsStrm = OpenStream( EXC_STREAM_CTLS );
        if( mxCtlsStrm.Is() )
        {
            String aClassName;
            sal_uInt32 nStrmStart = static_cast< sal_uInt32 >( mxCtlsStrm->Tell() );

            // writes from xCtrlModel into mxCtlsStrm, raw class name returned in aClassName
            if( SvxMSConvertOCXControls::WriteOCXExcelKludgeStream( mxCtlsStrm, xCtrlModel, xShape->getSize(), aClassName ) )
            {
                sal_uInt32 nStrmSize = static_cast< sal_uInt32 >( mxCtlsStrm->Tell() - nStrmStart );
                // adjust the class name to "Forms.***.1"
                aClassName.InsertAscii( "Forms.", 0 ).AppendAscii( ".1" );
                xOcxCtrl.reset( new XclExpOcxControlObj( GetRoot(), xShape, aClassName, nStrmStart, nStrmSize ) );
            }
        }
    }
    return xOcxCtrl.release();
}

#else

XclExpTbxControlObj* XclEscherEx::CreateCtrlObj( Reference< XShape > xShape )
{
    ::std::auto_ptr< XclExpTbxControlObj > xTbxCtrl( new XclExpTbxControlObj( GetRoot(), xShape ) );
    if( xTbxCtrl->GetObjType() == EXC_OBJTYPE_UNKNOWN )
        xTbxCtrl.reset();

    if( xTbxCtrl.get() )
    {
        // find attached macro
        Reference< XControlModel > xCtrlModel = XclControlHelper::GetControlModel( xShape );
        ConvertTbxMacro( *xTbxCtrl, xCtrlModel );
    }
    return xTbxCtrl.release();
}

void XclEscherEx::ConvertTbxMacro( XclExpTbxControlObj& rTbxCtrlObj, Reference< XControlModel > xCtrlModel )
{
    SdrPage* pSdrPage = GetSdrPage( GetCurrScTab() );
    if( xCtrlModel.is() && GetDocShell() && pSdrPage ) try
    {
        Reference< XFormsSupplier > xFormsSupplier( pSdrPage->getUnoPage(), UNO_QUERY_THROW );
        Reference< XIndexAccess > xFormsIA( xFormsSupplier->getForms(), UNO_QUERY_THROW );

        // 1) try to find the index of the processed control in the form

        Reference< XIndexAccess > xFormIA;  // needed in step 2) below
        sal_Int32 nFoundIdx = -1;

        // search all existing forms in the draw page
        for( sal_Int32 nFormIdx = 0, nFormCount = xFormsIA->getCount();
                (nFoundIdx < 0) && (nFormIdx < nFormCount); ++nFormIdx )
        {
            // get the XIndexAccess interface of the form with index nFormIdx
            if( xFormIA.set( xFormsIA->getByIndex( nFormIdx ), UNO_QUERY ) )
            {
                // search all elements (controls) of the current form by index
                for( sal_Int32 nCtrlIdx = 0, nCtrlCount = xFormIA->getCount();
                        (nFoundIdx < 0) && (nCtrlIdx < nCtrlCount); ++nCtrlIdx )
                {
                    // compare implementation pointers of the control models
                    Reference< XControlModel > xCurrModel( xFormIA->getByIndex( nCtrlIdx ), UNO_QUERY );
                    if( xCtrlModel.get() == xCurrModel.get() )
                        nFoundIdx = nCtrlIdx;
                }
            }
        }

        // 2) try to find an attached macro

        if( xFormIA.is() && (nFoundIdx >= 0) )
        {
            Reference< XEventAttacherManager > xEventMgr( xFormIA, UNO_QUERY_THROW );
            // loop over all events attached to the found control
            const Sequence< ScriptEventDescriptor > aEventSeq( xEventMgr->getScriptEvents( nFoundIdx ) );
            bool bFound = false;
            for( sal_Int32 nEventIdx = 0, nEventCount = aEventSeq.getLength();
                    !bFound && (nEventIdx < nEventCount); ++nEventIdx )
            {
                // try to set the event data at the Excel control object, returns true on success
                bFound = rTbxCtrlObj.SetMacroLink( aEventSeq[ nEventIdx ] );
            }
        }
    }
    catch( Exception& )
    {
    }
}

#endif

// --- class XclEscher -----------------------------------------------

XclEscher::XclEscher( const XclExpRoot& rRoot, UINT32 nDrawings ) :
    XclExpRoot( rRoot )
{
    pTempFile = new utl::TempFile;
    pTempFile->EnableKillingFile();
    pStrm = utl::UcbStreamHelper::CreateStream( pTempFile->GetURL(), STREAM_STD_READWRITE );
    pStrm->SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );
    pEx = new XclEscherEx( rRoot, *pStrm, nDrawings );
}


XclEscher::~XclEscher()
{
    delete pEx;
    delete pStrm;
    delete pTempFile;
}


void XclEscher::AddSdrPage()
{
    if( SdrPage* pPage = GetSdrPage( GetCurrScTab() ) )
        pEx->AddSdrPage( *pPage );
    // #106213# the first dummy object may still be open
    DBG_ASSERT( pEx->GetGroupLevel() <= 1, "XclEscher::AddSdrPage - still groups open?" );
    while( pEx->GetGroupLevel() )
        pEx->LeaveGroup();
}


// Escher client anchor =======================================================

XclExpDffAnchor::XclExpDffAnchor( const XclExpRoot& rRoot, sal_uInt16 nFlags ) :
    XclExpRoot( rRoot ),
    maAnchor( rRoot.GetCurrScTab() ),
    mnFlags( nFlags )
{
}

XclExpDffAnchor::XclExpDffAnchor( const XclExpRoot& rRoot, const SdrObject& rSdrObj ) :
    XclExpRoot( rRoot ),
    maAnchor( rRoot.GetCurrScTab() )
{
    SetFlags( rSdrObj );
    maAnchor.SetRect( GetDoc(), rSdrObj.GetCurrentBoundRect(), MAP_100TH_MM );
}

void XclExpDffAnchor::SetFlags( const SdrObject& rSdrObj )
{
    // Special case "page anchor" (X==0,Y==1) -> lock pos and size.
    const Point& rPos = rSdrObj.GetAnchorPos();
    mnFlags = ((rPos.X() == 0) && (rPos.Y() == 1)) ? EXC_ESC_ANCHOR_LOCKED : 0;
}

void XclExpDffAnchor::WriteData( EscherEx& rEx, const Rectangle& rRect )
{
    // the rectangle is already in twips
    maAnchor.SetRect( GetDoc(), rRect, MAP_TWIP );
    WriteData( rEx );
}


void XclExpDffAnchor::WriteData( EscherEx& rEx ) const
{
    rEx.AddAtom( 18, ESCHER_ClientAnchor );
    rEx.GetStream() << mnFlags << maAnchor;
}


// ----------------------------------------------------------------------------

XclExpDffNoteAnchor::XclExpDffNoteAnchor( const XclExpRoot& rRoot, const Rectangle& rRect ) :
    XclExpDffAnchor( rRoot, EXC_ESC_ANCHOR_SIZELOCKED )
{
    maAnchor.SetRect( GetDoc(), rRect, MAP_100TH_MM );
}


// ----------------------------------------------------------------------------

XclExpDffDropDownAnchor::XclExpDffDropDownAnchor( const XclExpRoot& rRoot, const ScAddress& rScPos ) :
    XclExpDffAnchor( rRoot, EXC_ESC_ANCHOR_POSLOCKED )
{
    GetAddressConverter().ConvertAddress( maAnchor.maFirst, rScPos, true );
    maAnchor.maLast.mnCol = maAnchor.maFirst.mnCol + 1;
    maAnchor.maLast.mnRow = maAnchor.maFirst.mnRow + 1;
    maAnchor.mnLX = maAnchor.mnTY = maAnchor.mnRX = maAnchor.mnBY = 0;
}


// ============================================================================

// --- class XclEscherClientData -------------------------------------

void XclEscherClientData::WriteData( EscherEx& rEx ) const
{   // actual data is in the following OBJ record
    rEx.AddAtom( 0, ESCHER_ClientData );
}


// --- class XclEscherClientTextbox -------------------------------------

XclEscherClientTextbox::XclEscherClientTextbox( const XclExpRoot& rRoot,
            const SdrTextObj& rObj, XclObj* pObj )
        :
        XclExpRoot( rRoot ),
        rTextObj( rObj ),
        pXclObj( pObj )
{
}


void XclEscherClientTextbox::WriteData( EscherEx& /*rEx*/ ) const
{
    pXclObj->SetText( GetRoot(), rTextObj );
}


