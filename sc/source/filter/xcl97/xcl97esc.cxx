/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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


#include <com/sun/star/awt/XControlModel.hpp>
#include <com/sun/star/embed/XClassifiedObject.hpp>
#include <com/sun/star/form/XFormsSupplier.hpp>
#include <com/sun/star/script/ScriptEventDescriptor.hpp>
#include <com/sun/star/script/XEventAttacherManager.hpp>

#include <svx/svdpage.hxx>
#include <editeng/outlobj.hxx>
#include <svx/svdotext.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdoole2.hxx>
#include <svx/unoapi.hxx>
#include <svx/fmglob.hxx>
#include <vcl/outdev.hxx>
#include <unotools/tempfile.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <svx/sdasitm.hxx>
#include <sfx2/docfile.hxx>

#include <sot/exchange.hxx>
#include "xeescher.hxx"

#include "global.hxx"
#include "document.hxx"
#include "drwlayer.hxx"
#include "xecontent.hxx"
#include <editeng/flditem.hxx>
#include "userdat.hxx"
#include "xcl97rec.hxx"
#include "xehelper.hxx"
#include "xechart.hxx"
#include "xcl97esc.hxx"

using ::rtl::OUString;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::UNO_QUERY_THROW;
using ::com::sun::star::container::XIndexAccess;
using ::com::sun::star::embed::XClassifiedObject;
using ::com::sun::star::drawing::XShape;
using ::com::sun::star::awt::XControlModel;
using ::com::sun::star::form::XFormsSupplier;
using ::com::sun::star::script::ScriptEventDescriptor;
using ::com::sun::star::script::XEventAttacherManager;

// ============================================================================

XclEscherExGlobal::XclEscherExGlobal( const XclExpRoot& rRoot ) :
    XclExpRoot( rRoot )
{
    SetBaseURI( GetMedium().GetBaseURL( true ) );
}

SvStream* XclEscherExGlobal::ImplQueryPictureStream()
{
    mxPicTempFile.reset( new ::utl::TempFile );
    if( mxPicTempFile->IsValid() )
    {
        mxPicTempFile->EnableKillingFile();
        mxPicStrm.reset( ::utl::UcbStreamHelper::CreateStream( mxPicTempFile->GetURL(), STREAM_STD_READWRITE ) );
        mxPicStrm->SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );
    }
    return mxPicStrm.get();
}

// ============================================================================

XclEscherEx::XclEscherEx( const XclExpRoot& rRoot, XclExpObjectManager& rObjMgr, SvStream& rStrm, const XclEscherEx* pParent ) :
    EscherEx( pParent ? pParent->mxGlobal : EscherExGlobalRef( new XclEscherExGlobal( rRoot ) ), &rStrm ),
    XclExpRoot( rRoot ),
    mrObjMgr( rObjMgr ),
    pCurrXclObj( NULL ),
    pCurrAppData( NULL ),
    pTheClientData( new XclEscherClientData ),
    pAdditionalText( NULL ),
    nAdditionalText( 0 ),
    mnNextKey( 0 ),
    mbIsRootDff( pParent == 0 )
{
    InsertPersistOffset( mnNextKey, 0 );
}


XclEscherEx::~XclEscherEx()
{
    OSL_ENSURE( !aStack.empty(), "~XclEscherEx: stack not empty" );
    DeleteCurrAppData();
    delete pTheClientData;
}


sal_uInt32 XclEscherEx::InitNextDffFragment()
{
    /*  Current value of mnNextKey will be used by caller to refer to the
        starting point of the DFF fragment. The key exists already in the
        PersistTable (has been inserted by c'tor of previous call of
        InitNextDffFragment(), has been updated by UpdateDffFragmentEnd(). */
    sal_uInt32 nPersistKey = mnNextKey;

    /*  Prepare the next key that is used by caller as end point of the DFF
        fragment. Will be updated by caller when writing to the DFF stream,
        using the UpdateDffFragmentEnd() function. This is needed to find DFF
        data written by the SVX base class implementation without interaction,
        e.g. the solver container that will be written after the last shape. */
    ++mnNextKey;
    InsertPersistOffset( mnNextKey, mpOutStrm->Tell() );

    return nPersistKey;
}

void XclEscherEx::UpdateDffFragmentEnd()
{
    // update existing fragment key with new stream position
    ReplacePersistOffset( mnNextKey, mpOutStrm->Tell() );
}

sal_uInt32 XclEscherEx::GetDffFragmentPos( sal_uInt32 nFragmentKey )
{
    /*  TODO: this function is non-const because PersistTable::PtGetOffsetByID()
        is non-const due to tools/List usage. */
    return GetPersistOffset( nFragmentKey );
}

sal_uInt32 XclEscherEx::GetDffFragmentSize( sal_uInt32 nFragmentKey )
{
    /*  TODO: this function is non-const because PersistTable::PtGetOffsetByID()
        is non-const due to tools/List usage. */
    return GetDffFragmentPos( nFragmentKey + 1 ) - GetDffFragmentPos( nFragmentKey );
}

bool XclEscherEx::HasPendingDffData()
{
    /*  TODO: this function is non-const because PersistTable::PtGetOffsetByID()
        is non-const due to tools/List usage. */
    return GetDffFragmentPos( mnNextKey ) < GetStreamPos();
}

XclExpDffAnchorBase* XclEscherEx::CreateDffAnchor( const SdrObject& rSdrObj ) const
{
    // the object manager creates the correct anchor type according to context
    XclExpDffAnchorBase* pAnchor = mrObjMgr.CreateDffAnchor();
    // pass the drawing object, that will calculate the anchor position
    pAnchor->SetSdrObject( rSdrObj );
    return pAnchor;
}

namespace {

bool lcl_IsFontwork( const SdrObject* pObj )
{
    bool bIsFontwork = false;
    if( pObj->GetObjIdentifier() == OBJ_CUSTOMSHAPE )
    {
        const OUString aTextPath = "TextPath";
        SdrCustomShapeGeometryItem& rGeometryItem = (SdrCustomShapeGeometryItem&)
            pObj->GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY );
        if( Any* pAny = rGeometryItem.GetPropertyValueByName( aTextPath, aTextPath ) )
            *pAny >>= bIsFontwork;
    }
    return bIsFontwork;
}

} // namespace

EscherExHostAppData* XclEscherEx::StartShape( const Reference< XShape >& rxShape, const Rectangle* pChildAnchor )
{
    if ( nAdditionalText )
        nAdditionalText++;
    sal_Bool bInGroup = ( pCurrXclObj != NULL );
    if ( bInGroup )
    {   // stacked recursive group object
        if ( !pCurrAppData->IsStackedGroup() )
        {   //! UpdateDffFragmentEnd only once
            pCurrAppData->SetStackedGroup( sal_True );
            UpdateDffFragmentEnd();
        }
    }
    aStack.push( std::make_pair( pCurrXclObj, pCurrAppData ) );
    pCurrAppData = new XclEscherHostAppData;
    SdrObject* pObj = GetSdrObjectFromXShape( rxShape );
    if ( !pObj )
        pCurrXclObj = new XclObjAny( mrObjMgr, rxShape );  // just what is it?!?
    else
    {
        pCurrXclObj = NULL;
        sal_uInt16 nObjType = pObj->GetObjIdentifier();

        if( nObjType == OBJ_OLE2 )
        {
            // no OLE objects in embedded drawings (chart shapes)
            if( mbIsRootDff )
            {
                //! not-const because GetObjRef may load the OLE object
                Reference < XClassifiedObject > xObj( ((SdrOle2Obj*)pObj)->GetObjRef(), UNO_QUERY );
                if ( xObj.is() )
                {
                    SvGlobalName aObjClsId( xObj->getClassID() );
                    if ( SotExchange::IsChart( aObjClsId ) )
                    {   // yes, it's a chart diagram
                        mrObjMgr.AddObj( new XclExpChartObj( mrObjMgr, rxShape, pChildAnchor ) );
                        pCurrXclObj = NULL;     // no metafile or whatsoever
                    }
                    else    // metafile and OLE object
                        pCurrXclObj = new XclObjOle( mrObjMgr, *pObj );
                }
                else    // just a metafile
                    pCurrXclObj = new XclObjAny( mrObjMgr, rxShape );
            }
            else
                pCurrXclObj = new XclObjAny( mrObjMgr, rxShape );
        }
        else if( nObjType == OBJ_UNO )
        {
#if EXC_EXP_OCX_CTRL
            // no ActiveX controls in embedded drawings (chart shapes)
            if( mbIsRootDff )
                pCurrXclObj = CreateCtrlObj( rxShape, pChildAnchor );
#else
            pCurrXclObj = CreateCtrlObj( rxShape, pChildAnchor );
#endif
            if( !pCurrXclObj )
                pCurrXclObj = new XclObjAny( mrObjMgr, rxShape );   // just a metafile
        }
        else if( !ScDrawLayer::IsNoteCaption( pObj ) )
        {
            // ignore permanent note shapes
            // #i12190# do not ignore callouts (do not filter by object type ID)
            pCurrXclObj = ShapeInteractionHelper::CreateShapeObj( mrObjMgr, rxShape );
            ShapeInteractionHelper::PopulateShapeInteractionInfo( mrObjMgr, rxShape, *pCurrAppData );
        }
    }
    if ( pCurrXclObj )
    {
        if ( !mrObjMgr.AddObj( pCurrXclObj ) )
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
                        /*  Create a dummy anchor carrying the flags. Real
                            coordinates are calculated later in virtual call of
                            WriteData(EscherEx&,const Rectangle&). */
                        XclExpDffAnchorBase* pAnchor = mrObjMgr.CreateDffAnchor();
                        pAnchor->SetFlags( *pObj );
                        pCurrAppData->SetClientAnchor( pAnchor );
                    }
                    const SdrTextObj* pTextObj = PTR_CAST( SdrTextObj, pObj );
                    if( pTextObj && !lcl_IsFontwork( pTextObj ) && (pObj->GetObjIdentifier() != OBJ_CAPTION) )
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
                        pCurrAppData->SetClientAnchor( mrObjMgr.CreateDffAnchor() );
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
        pCurrAppData->SetDontWriteShape( sal_True );
    return pCurrAppData;
}


void XclEscherEx::EndShape( sal_uInt16 nShapeType, sal_uInt32 nShapeID )
{
    // own escher data created? -> never delete such objects
    bool bOwnEscher = pCurrXclObj && pCurrXclObj->IsOwnEscher();

    // post process the current object - not for objects with own escher data
    if( pCurrXclObj && !bOwnEscher )
    {
        // escher data of last shape not written? -> delete it from object list
        if( nShapeID == 0 )
        {
            XclObj* pLastObj = mrObjMgr.RemoveLastObj();
            OSL_ENSURE( pLastObj == pCurrXclObj, "XclEscherEx::EndShape - wrong object" );
            DELETEZ( pLastObj );
            pCurrXclObj = 0;
        }

        if( pCurrXclObj )
        {
            // set shape type
            if ( pCurrAppData->IsStackedGroup() )
                pCurrXclObj->SetEscherShapeTypeGroup();
            else
            {
                pCurrXclObj->SetEscherShapeType( nShapeType );
                UpdateDffFragmentEnd();
            }
        }
    }

    // get next object from stack
    DeleteCurrAppData();
    if (aStack.empty())
    {
        pCurrXclObj = NULL;
        pCurrAppData = NULL;
    }
    else
    {
        pCurrXclObj = aStack.top().first;
        pCurrAppData = aStack.top().second;
        aStack.pop();
    }
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

void XclEscherEx::EndDocument()
{
    if( mbIsRootDff )
        Flush( static_cast< XclEscherExGlobal& >( *mxGlobal ).GetPictureStream() );

    // seek back DFF stream to prepare saving the MSODRAWING[GROUP] records
    mpOutStrm->Seek( 0 );
}

#if EXC_EXP_OCX_CTRL

XclExpOcxControlObj* XclEscherEx::CreateCtrlObj( Reference< XShape > xShape, const Rectangle* pChildAnchor )
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
                xOcxCtrl.reset( new XclExpOcxControlObj( mrObjMgr, xShape, pChildAnchor, aClassName, nStrmStart, nStrmSize ) );
            }
        }
    }
    return xOcxCtrl.release();
}

#else

XclExpTbxControlObj* XclEscherEx::CreateCtrlObj( Reference< XShape > xShape, const Rectangle* pChildAnchor )
{
    ::std::auto_ptr< XclExpTbxControlObj > xTbxCtrl( new XclExpTbxControlObj( mrObjMgr, xShape, pChildAnchor ) );
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

void XclEscherEx::DeleteCurrAppData()
{
    if ( pCurrAppData )
    {
        delete pCurrAppData->GetClientAnchor();
//      delete pCurrAppData->GetClientData();
        delete pCurrAppData->GetClientTextbox();
    delete pCurrAppData->GetInteractionInfo();
        delete pCurrAppData;
    }
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

XclExpShapeObj*
ShapeInteractionHelper::CreateShapeObj( XclExpObjectManager& rObjMgr, const Reference< XShape >& xShape )
{
    return new XclExpShapeObj( rObjMgr, xShape );
}

void
ShapeInteractionHelper::PopulateShapeInteractionInfo( XclExpObjectManager& rObjMgr, const Reference< XShape >& xShape, EscherExHostAppData& rHostAppData )
{
   try
   {
      SvMemoryStream* pMemStrm = NULL;
      rtl::OUString sHyperLink;
      rtl::OUString sMacro;
      if ( ScMacroInfo* pInfo = ScDrawLayer::GetMacroInfo( ::GetSdrObjectFromXShape( xShape ) ) )
      {
         sHyperLink = pInfo->GetHlink();
         sMacro = pInfo->GetMacro();
      }
      if (  !sHyperLink.isEmpty() )
      {
         pMemStrm = new SvMemoryStream();
         XclExpStream tmpStream( *pMemStrm, rObjMgr.GetRoot() );
         ScAddress dummyAddress;
         SvxURLField aUrlField;
         aUrlField.SetURL( sHyperLink );
         XclExpHyperlink hExpHlink( rObjMgr.GetRoot(), aUrlField, dummyAddress );
         hExpHlink.WriteEmbeddedData( tmpStream );
      }
      if ( !sHyperLink.isEmpty() || !sMacro.isEmpty() )
          rHostAppData.SetInteractionInfo( new InteractionInfo( pMemStrm, true ) );
   }
   catch( Exception& )
   {
   }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
