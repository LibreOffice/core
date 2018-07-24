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

#include <memory>
#include <com/sun/star/awt/XControlModel.hpp>
#include <com/sun/star/embed/XClassifiedObject.hpp>
#include <com/sun/star/form/XFormsSupplier.hpp>
#include <com/sun/star/script/ScriptEventDescriptor.hpp>
#include <com/sun/star/script/XEventAttacherManager.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include <svx/svdpage.hxx>
#include <editeng/outlobj.hxx>
#include <o3tl/make_unique.hxx>
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
#include <xeescher.hxx>

#include <global.hxx>
#include <document.hxx>
#include <drwlayer.hxx>
#include <xecontent.hxx>
#include <editeng/flditem.hxx>
#include <userdat.hxx>
#include <xcl97rec.hxx>
#include <xehelper.hxx>
#include <xechart.hxx>
#include <xcl97esc.hxx>
#include <unotools/streamwrap.hxx>
#include <oox/ole/olehelper.hxx>
#include <sfx2/objsh.hxx>

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
using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::uno::Any;
using ::com::sun::star::form::XFormsSupplier;
using ::com::sun::star::io::XOutputStream;
using ::com::sun::star::script::ScriptEventDescriptor;
using ::com::sun::star::script::XEventAttacherManager;

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
        mxPicStrm = ::utl::UcbStreamHelper::CreateStream( mxPicTempFile->GetURL(), StreamMode::STD_READWRITE );
        mxPicStrm->SetEndian( SvStreamEndian::LITTLE );
    }
    return mxPicStrm.get();
}

XclEscherEx::XclEscherEx( const XclExpRoot& rRoot, XclExpObjectManager& rObjMgr, SvStream& rStrm, const XclEscherEx* pParent ) :
    EscherEx( pParent ? pParent->mxGlobal : std::shared_ptr<EscherExGlobal>( new XclEscherExGlobal( rRoot ) ), &rStrm ),
    XclExpRoot( rRoot ),
    mrObjMgr( rObjMgr ),
    pCurrXclObj( nullptr ),
    pCurrAppData( nullptr ),
    pTheClientData( new XclEscherClientData ),
    pAdditionalText( nullptr ),
    nAdditionalText( 0 ),
    mnNextKey( 0 ),
    mbIsRootDff( pParent == nullptr )
{
    InsertPersistOffset( mnNextKey, 0 );
}

XclEscherEx::~XclEscherEx()
{
    OSL_ENSURE( aStack.empty(), "~XclEscherEx: stack not empty" );
    DeleteCurrAppData();
    pTheClientData.reset();
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
        const SdrCustomShapeGeometryItem& rGeometryItem =
            pObj->GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY );
        if( const Any* pAny = rGeometryItem.GetPropertyValueByName( aTextPath, aTextPath ) )
            *pAny >>= bIsFontwork;
    }
    return bIsFontwork;
}

} // namespace

EscherExHostAppData* XclEscherEx::StartShape( const Reference< XShape >& rxShape, const tools::Rectangle* pChildAnchor )
{
    if ( nAdditionalText )
        nAdditionalText++;
    bool bInGroup = ( pCurrXclObj != nullptr );
    if ( bInGroup && pCurrAppData )
    {   // stacked recursive group object
        if ( !pCurrAppData->IsStackedGroup() )
        {   //! UpdateDffFragmentEnd only once
            pCurrAppData->SetStackedGroup( true );
            UpdateDffFragmentEnd();
        }
    }
    aStack.push( std::make_pair( pCurrXclObj, std::move(pCurrAppData) ) );
    pCurrAppData.reset( new XclEscherHostAppData );
    SdrObject* pObj = GetSdrObjectFromXShape( rxShape );
    //added for exporting OCX control
    sal_Int16 nMsCtlType = 0;
    if ( !pObj )
        pCurrXclObj = new XclObjAny( mrObjMgr, rxShape, &GetDocRef() );  // just what is it?!?
    else
    {
        pCurrXclObj = nullptr;
        sal_uInt16 nObjType = pObj->GetObjIdentifier();

        if( nObjType == OBJ_OLE2 )
        {
            // no OLE objects in embedded drawings (chart shapes)
            if( mbIsRootDff )
            {
                //! not-const because GetObjRef may load the OLE object
                Reference < XClassifiedObject > xObj( static_cast<SdrOle2Obj*>(pObj)->GetObjRef(), UNO_QUERY );
                if ( xObj.is() )
                {
                    SvGlobalName aObjClsId( xObj->getClassID() );
                    if ( SotExchange::IsChart( aObjClsId ) )
                    {   // yes, it's a chart diagram
                        mrObjMgr.AddObj( o3tl::make_unique<XclExpChartObj>( mrObjMgr, rxShape, pChildAnchor ) );
                        pCurrXclObj = nullptr;     // no metafile or whatsoever
                    }
                    else    // metafile and OLE object
                        pCurrXclObj = new XclObjOle( mrObjMgr, *pObj );
                }
                else    // just a metafile
                    pCurrXclObj = new XclObjAny( mrObjMgr, rxShape, &GetDocRef() );
            }
            else
                pCurrXclObj = new XclObjAny( mrObjMgr, rxShape, &GetDocRef() );
        }
        else if( nObjType == OBJ_UNO )
        {
            //added for exporting OCX control
            Reference< XPropertySet > xPropSet( rxShape, UNO_QUERY );
            Any aAny;
            try
            {
                aAny = xPropSet->getPropertyValue("ControlTypeinMSO");
                aAny >>= nMsCtlType;
            }
            catch(const Exception&)
            {
                SAL_WARN("sc", "XclEscherEx::StartShape, this control can't get the property ControlTypeinMSO!");
            }
            if( nMsCtlType == 2 )  //OCX Form Control
                pCurrXclObj = CreateOCXCtrlObj( rxShape, pChildAnchor ).release();
            else  //TBX Form Control
                pCurrXclObj = CreateTBXCtrlObj( rxShape, pChildAnchor ).release();
            if( !pCurrXclObj )
                pCurrXclObj = new XclObjAny( mrObjMgr, rxShape, &GetDocRef() );   // just a metafile
        }
        else if( !ScDrawLayer::IsNoteCaption( pObj ) )
        {
            // ignore permanent note shapes
            // #i12190# do not ignore callouts (do not filter by object type ID)
            pCurrXclObj = ShapeInteractionHelper::CreateShapeObj( mrObjMgr, rxShape, &GetDocRef() );
            ShapeInteractionHelper::PopulateShapeInteractionInfo( mrObjMgr, rxShape, *pCurrAppData );
        }
    }
    if ( pCurrXclObj )
    {
        if ( !mrObjMgr.AddObj( std::unique_ptr<XclObj>(pCurrXclObj) ) )
        {   // maximum count reached, object got deleted
            pCurrXclObj = nullptr;
        }
        else
        {
            pCurrAppData->SetClientData( pTheClientData.get() );
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
                    const SdrTextObj* pTextObj = dynamic_cast<SdrTextObj*>( pObj  );
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
    if(pObj)
    {
        //add  for exporting OCX control
        //for OCX control import from MS office file,we need keep the id value as MS office file.
        //GetOldRoot().pObjRecs->Add( pCurrXclObj ) statement has generated the id value as obj id rule;
        //but we trick it here.
        sal_uInt16 nObjType = pObj->GetObjIdentifier();
        if( nObjType == OBJ_UNO && pCurrXclObj )
        {
            Reference< XPropertySet > xPropSet( rxShape, UNO_QUERY );
            Any aAny;
            try
            {
                aAny = xPropSet->getPropertyValue("ObjIDinMSO");
            }
            catch(const Exception&)
            {
                SAL_WARN("sc", "XclEscherEx::StartShape, this control can't get the property ObjIDinMSO!");
            }
            sal_uInt16 nObjIDinMSO = 0xFFFF;
            aAny >>= nObjIDinMSO;
            if( nObjIDinMSO != 0xFFFF && nMsCtlType == 2)  //OCX
            {
                pCurrXclObj->SetId(nObjIDinMSO);
            }
        }
    }
    if ( !pCurrXclObj )
        pCurrAppData->SetDontWriteShape( true );
    return pCurrAppData.get();
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
            std::unique_ptr<XclObj> pLastObj = mrObjMgr.RemoveLastObj();
            OSL_ENSURE( pLastObj.get() == pCurrXclObj, "XclEscherEx::EndShape - wrong object" );
            pCurrXclObj = nullptr;
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
        pCurrXclObj = nullptr;
        pCurrAppData = nullptr;
    }
    else
    {
        pCurrXclObj = aStack.top().first;
        pCurrAppData = std::move(aStack.top().second);
    }
    if( nAdditionalText == 3 )
        nAdditionalText = 0;
}

EscherExHostAppData* XclEscherEx::EnterAdditionalTextGroup()
{
    nAdditionalText = 1;
    pAdditionalText = static_cast<XclEscherClientTextbox*>( pCurrAppData->GetClientTextbox() );
    pCurrAppData->SetClientTextbox( nullptr );
    return pCurrAppData.get();
}

void XclEscherEx::EndDocument()
{
    if( mbIsRootDff )
        Flush( static_cast< XclEscherExGlobal& >( *mxGlobal ).GetPictureStream() );

    // seek back DFF stream to prepare saving the MSODRAWING[GROUP] records
    mpOutStrm->Seek( 0 );
}

std::unique_ptr<XclExpOcxControlObj> XclEscherEx::CreateOCXCtrlObj( Reference< XShape > const & xShape, const tools::Rectangle* pChildAnchor )
{
    ::std::unique_ptr< XclExpOcxControlObj > xOcxCtrl;

    Reference< XControlModel > xCtrlModel = XclControlHelper::GetControlModel( xShape );
    if( xCtrlModel.is() )
    {
        // output stream
        if( !mxCtlsStrm.is() )
            mxCtlsStrm = OpenStream( EXC_STREAM_CTLS );
        if( mxCtlsStrm.is() )
        {
            OUString aClassName;
            sal_uInt32 nStrmStart = static_cast< sal_uInt32 >( mxCtlsStrm->Tell() );

            // writes from xCtrlModel into mxCtlsStrm, raw class name returned in aClassName
            Reference< XOutputStream > xOut( new utl::OSeekableOutputStreamWrapper( *mxCtlsStrm ) );
            Reference< css::frame::XModel > xModel( GetDocShell() ? GetDocShell()->GetModel() : nullptr );
            if( xModel.is() && xOut.is() && oox::ole::MSConvertOCXControls::WriteOCXExcelKludgeStream( xModel, xOut, xCtrlModel, xShape->getSize(), aClassName ) )
            {
                sal_uInt32 nStrmSize = static_cast< sal_uInt32 >( mxCtlsStrm->Tell() - nStrmStart );
                // adjust the class name to "Forms.***.1"
                aClassName = "Forms." + aClassName +  ".1";
                xOcxCtrl.reset( new XclExpOcxControlObj( mrObjMgr, xShape, pChildAnchor, aClassName, nStrmStart, nStrmSize ) );
            }
        }
    }
    return xOcxCtrl;
}

std::unique_ptr<XclExpTbxControlObj> XclEscherEx::CreateTBXCtrlObj( Reference< XShape > const & xShape, const tools::Rectangle* pChildAnchor )
{
    ::std::unique_ptr< XclExpTbxControlObj > xTbxCtrl( new XclExpTbxControlObj( mrObjMgr, xShape, pChildAnchor ) );
    if( xTbxCtrl->GetObjType() == EXC_OBJTYPE_UNKNOWN )
        xTbxCtrl.reset();

    if( xTbxCtrl.get() )
    {
        // find attached macro
        Reference< XControlModel > xCtrlModel = XclControlHelper::GetControlModel( xShape );
        ConvertTbxMacro( *xTbxCtrl, xCtrlModel );
    }
    return xTbxCtrl;
}

void XclEscherEx::ConvertTbxMacro( XclExpTbxControlObj& rTbxCtrlObj, Reference< XControlModel > const & xCtrlModel )
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

void XclEscherEx::DeleteCurrAppData()
{
    if ( pCurrAppData )
    {
        delete pCurrAppData->GetClientAnchor();
//      delete pCurrAppData->GetClientData();
        delete pCurrAppData->GetClientTextbox();
        delete pCurrAppData->GetInteractionInfo();
        pCurrAppData.reset();
    }
}

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
ShapeInteractionHelper::CreateShapeObj( XclExpObjectManager& rObjMgr, const Reference< XShape >& xShape, ScDocument* pDoc )
{
    return new XclExpShapeObj( rObjMgr, xShape, pDoc );
}

void
ShapeInteractionHelper::PopulateShapeInteractionInfo( const XclExpObjectManager& rObjMgr, const Reference< XShape >& xShape, EscherExHostAppData& rHostAppData )
{
   try
   {
      SvMemoryStream* pMemStrm = nullptr;
      OUString sHyperLink;
      OUString sMacro;
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
          rHostAppData.SetInteractionInfo( new InteractionInfo( pMemStrm ) );
   }
   catch( Exception& )
   {
   }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
