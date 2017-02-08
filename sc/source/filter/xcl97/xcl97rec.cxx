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

#include <svx/svdpool.hxx>
#include <svx/sdtaitm.hxx>
#include <svx/svdotext.hxx>
#include <editeng/editobj.hxx>
#include <svx/svdoole2.hxx>
#include <sot/storage.hxx>
#include <svl/itemset.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdocapt.hxx>
#include <svx/unoapi.hxx>
#include <editeng/writingmodeitem.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <tools/urlobj.hxx>

#include <rtl/math.hxx>
#include <svl/zformat.hxx>
#include "formulacell.hxx"
#include "drwlayer.hxx"

#include "xcl97rec.hxx"
#include "xcl97esc.hxx"
#include "editutil.hxx"
#include "xecontent.hxx"
#include "xeescher.hxx"
#include "xestyle.hxx"
#include "xelink.hxx"

#include "scitems.hxx"

#include <unotools/fltrcfg.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/frmdiritem.hxx>
#include <editeng/adjustitem.hxx>
#include <editeng/eeitem.hxx>
#include <filter/msfilter/msoleexp.hxx>

#include <unotools/localedatawrapper.hxx>

#include <stdio.h>

#include "document.hxx"
#include "conditio.hxx"
#include "rangelst.hxx"
#include "stlpool.hxx"
#include "viewopti.hxx"
#include "scextopt.hxx"
#include "docoptio.hxx"
#include "patattr.hxx"
#include "tabprotection.hxx"

#include <com/sun/star/embed/Aspects.hpp>
#include <com/sun/star/chart2/XCoordinateSystemContainer.hpp>
#include <com/sun/star/chart2/XChartTypeContainer.hpp>
#include <oox/token/tokens.hxx>
#include <oox/token/namespaces.hxx>
#include <oox/export/shapes.hxx>
#include <oox/export/utils.hxx>
#include <oox/export/vmlexport.hxx>

#include <memory>

using namespace ::com::sun::star;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::drawing::XShape;
using ::oox::drawingml::ShapeExport;
using ::oox::vml::VMLExport;
using namespace oox;

sal_Int32 XclExpObjList::mnDrawingMLCount;
sal_Int32 XclExpObjList::mnVmlCount;

XclExpObjList::XclExpObjList( const XclExpRoot& rRoot, XclEscherEx& rEscherEx ) :
    XclExpRoot( rRoot ),
    mnScTab( rRoot.GetCurrScTab() ),
    mrEscherEx( rEscherEx ),
    pSolverContainer( nullptr )
{
    pMsodrawingPerSheet = new XclExpMsoDrawing( rEscherEx );
    // open the DGCONTAINER and the patriarch group shape
    mrEscherEx.OpenContainer( ESCHER_DgContainer );
    Rectangle aRect( 0, 0, 0, 0 );
    mrEscherEx.EnterGroup( &aRect );
    mrEscherEx.UpdateDffFragmentEnd();
}

XclExpObjList::~XclExpObjList()
{
    std::for_each(maObjs.begin(), maObjs.end(), std::default_delete<XclObj>());
    delete pMsodrawingPerSheet;
    delete pSolverContainer;
}

sal_uInt16 XclExpObjList::Add( XclObj* pObj )
{
    OSL_ENSURE( maObjs.size() < 0xFFFF, "XclExpObjList::Add: too much for Xcl" );

    size_t nSize = maObjs.size();

    if ( nSize < 0xFFFF )
    {
        maObjs.push_back(pObj);
        ++nSize;
        pObj->SetId( nSize );
        pObj->SetTab( mnScTab );
    }
    else
    {
        delete pObj;
        nSize = 0;
    }

    return nSize;
}

void XclExpObjList::pop_back ()
{
    maObjs.pop_back();
}

void XclExpObjList::EndSheet()
{
    // Is there still something in the stream? -> The solver container
    if( mrEscherEx.HasPendingDffData() )
        pSolverContainer = new XclExpMsoDrawing( mrEscherEx );

    // close the DGCONTAINER created by XclExpObjList ctor MSODRAWING
    mrEscherEx.CloseContainer();
}

void XclExpObjList::Save( XclExpStream& rStrm )
{
    //! Escher must be written, even if there are no objects
    pMsodrawingPerSheet->Save( rStrm );

    std::vector<XclObj*>::iterator pIter;
    for ( pIter = maObjs.begin(); pIter != maObjs.end(); ++pIter )
        (*pIter)->Save( rStrm );

    if( pSolverContainer )
        pSolverContainer->Save( rStrm );
}

namespace {

bool IsVmlObject( const XclObj *rObj )
{
    switch( rObj->GetObjType() )
    {
        case EXC_OBJTYPE_NOTE:
            return true;
        default:
            return false;
    }
}

sal_Int32 GetVmlObjectCount( XclExpObjList& rList )
{
    sal_Int32 nNumVml = 0;

    std::vector<XclObj*>::iterator pIter;
    for ( pIter = rList.begin(); pIter != rList.end(); ++pIter )
        if( IsVmlObject( *pIter ) )
            ++nNumVml;

    return nNumVml;
}

bool IsValidObject( const XclObj& rObj )
{
    if (rObj.GetObjType() == EXC_OBJTYPE_CHART)
    {
        // Chart object.  Make sure it's a valid chart object.  We skip
        // invalid chart objects from exporting to prevent Excel from
        // complaining on load.

        const XclExpChartObj& rChartObj = static_cast<const XclExpChartObj&>(rObj);
        uno::Reference<chart2::XChartDocument> xChartDoc(rChartObj.GetChartDoc(), uno::UNO_QUERY);
        if (!xChartDoc.is())
            return false;

        uno::Reference<chart2::XDiagram> xDiagram = xChartDoc->getFirstDiagram();
        if (!xDiagram.is())
            return false;

        uno::Reference<chart2::XCoordinateSystemContainer> xCooSysContainer(xDiagram, uno::UNO_QUERY);
        if (!xCooSysContainer.is())
            return false;

        uno::Sequence<uno::Reference<chart2::XCoordinateSystem> > xCooSysSeq = xCooSysContainer->getCoordinateSystems();
        if (!xCooSysSeq.getLength())
            return false;

        for (sal_Int32 nCooSys = 0; nCooSys < xCooSysSeq.getLength(); ++nCooSys)
        {
            Reference<chart2::XChartTypeContainer> xChartTypeCont(xCooSysSeq[nCooSys], uno::UNO_QUERY);
            if (!xChartTypeCont.is())
                return false;

            uno::Sequence<uno::Reference<chart2::XChartType> > xChartTypeSeq = xChartTypeCont->getChartTypes();
            if (!xChartTypeSeq.getLength())
                // No chart type.  Not good.
                return false;
        }
    }

    return true;
}

void SaveDrawingMLObjects( XclExpObjList& rList, XclExpXmlStream& rStrm, sal_Int32& nDrawingMLCount )
{
    std::vector<XclObj*> aList;
    aList.reserve(rList.size());
    std::vector<XclObj*>::iterator it = rList.begin(), itEnd = rList.end();
    for (; it != itEnd; ++it)
    {
        if (IsVmlObject(*it) || !IsValidObject(**it))
            continue;

        aList.push_back(*it);
    }

    if (aList.empty())
        return;

    sal_Int32 nDrawing = ++nDrawingMLCount;
    OUString sId;
    sax_fastparser::FSHelperPtr pDrawing = rStrm.CreateOutputStream(
            XclXmlUtils::GetStreamName( "xl/", "drawings/drawing", nDrawing ),
            XclXmlUtils::GetStreamName( "../", "drawings/drawing", nDrawing ),
            rStrm.GetCurrentStream()->getOutputStream(),
            "application/vnd.openxmlformats-officedocument.drawing+xml",
            "http://schemas.openxmlformats.org/officeDocument/2006/relationships/drawing",
            &sId );

    rStrm.GetCurrentStream()->singleElement( XML_drawing,
            FSNS( XML_r, XML_id ),  XclXmlUtils::ToOString( sId ).getStr(),
            FSEND );

    rStrm.PushStream( pDrawing );
    pDrawing->startElement( FSNS( XML_xdr, XML_wsDr ),
            FSNS( XML_xmlns, XML_xdr ), XclXmlUtils::ToOString(rStrm.getNamespaceURL(OOX_NS(dmlSpreadDr))).getStr(),
            FSNS( XML_xmlns, XML_a ),   XclXmlUtils::ToOString(rStrm.getNamespaceURL(OOX_NS(dml))).getStr(),
            FSNS( XML_xmlns, XML_r ),   XclXmlUtils::ToOString(rStrm.getNamespaceURL(OOX_NS(officeRel))).getStr(),
            FSEND );

    for (it = aList.begin(), itEnd = aList.end(); it != itEnd; ++it)
        (*it)->SaveXml(rStrm);

    pDrawing->endElement( FSNS( XML_xdr, XML_wsDr ) );

    rStrm.PopStream();
}

void SaveVmlObjects( XclExpObjList& rList, XclExpXmlStream& rStrm, sal_Int32& nVmlCount )
{
    if( GetVmlObjectCount( rList ) == 0 )
        return;

    sal_Int32 nDrawing = ++nVmlCount;
    OUString sId;
    sax_fastparser::FSHelperPtr pVmlDrawing = rStrm.CreateOutputStream(
            XclXmlUtils::GetStreamName( "xl/", "drawings/vmlDrawing", nDrawing ),
            XclXmlUtils::GetStreamName( "../", "drawings/vmlDrawing", nDrawing ),
            rStrm.GetCurrentStream()->getOutputStream(),
            "application/vnd.openxmlformats-officedocument.vmlDrawing",
            "http://schemas.openxmlformats.org/officeDocument/2006/relationships/vmlDrawing",
            &sId );

    rStrm.GetCurrentStream()->singleElement( XML_legacyDrawing,
            FSNS( XML_r, XML_id ),  XclXmlUtils::ToOString( sId ).getStr(),
            FSEND );

    rStrm.PushStream( pVmlDrawing );
    pVmlDrawing->startElement( XML_xml,
            FSNS( XML_xmlns, XML_v ),   XclXmlUtils::ToOString(rStrm.getNamespaceURL(OOX_NS(vml))).getStr(),
            FSNS( XML_xmlns, XML_o ),   XclXmlUtils::ToOString(rStrm.getNamespaceURL(OOX_NS(vmlOffice))).getStr(),
            FSNS( XML_xmlns, XML_x ),   XclXmlUtils::ToOString(rStrm.getNamespaceURL(OOX_NS(vmlExcel))).getStr(),
            FSNS( XML_xmlns, XML_w10 ), XclXmlUtils::ToOString(rStrm.getNamespaceURL(OOX_NS(vmlWord))).getStr(),
            FSEND );

    std::vector<XclObj*>::iterator pIter;
    for ( pIter = rList.begin(); pIter != rList.end(); ++pIter )
    {
        if( !IsVmlObject( *pIter ) )
            continue;
        (*pIter)->SaveXml( rStrm );
    }

    pVmlDrawing->endElement( XML_xml );

    rStrm.PopStream();
}

}

void XclExpObjList::SaveXml( XclExpXmlStream& rStrm )
{
    if( pSolverContainer )
        pSolverContainer->SaveXml( rStrm );

    if( maObjs.empty())
        return;

    SaveDrawingMLObjects( *this, rStrm, mnDrawingMLCount );
    SaveVmlObjects( *this, rStrm, mnVmlCount );
}

void XclExpObjList::ResetCounters()
{
    mnDrawingMLCount    = 0;
    mnVmlCount          = 0;
}

// --- class XclObj --------------------------------------------------

XclObj::XclObj( XclExpObjectManager& rObjMgr, sal_uInt16 nObjType, bool bOwnEscher ) :
    XclExpRecord( EXC_ID_OBJ, 26 ),
    mrEscherEx( rObjMgr.GetEscherEx() ),
    pClientTextbox( nullptr ),
    pTxo( nullptr ),
    mnObjType( nObjType ),
    nObjId(0),
    nGrbit( 0x6011 ),   // AutoLine, AutoFill, Printable, Locked
    mnScTab(0),
    bFirstOnSheet( !rObjMgr.HasObj() ),
    mbOwnEscher( bOwnEscher )
{
    //! first object continues the first MSODRAWING record
    if ( bFirstOnSheet )
        pMsodrawing = rObjMgr.GetMsodrawingPerSheet();
    else
        pMsodrawing = new XclExpMsoDrawing( mrEscherEx );
}

XclObj::~XclObj()
{
    if ( !bFirstOnSheet )
        delete pMsodrawing;
    delete pClientTextbox;
    delete pTxo;
}

void XclObj::ImplWriteAnchor( const XclExpRoot& /*rRoot*/, const SdrObject* pSdrObj, const Rectangle* pChildAnchor )
{
    if( pChildAnchor )
    {
        mrEscherEx.AddChildAnchor( *pChildAnchor );
    }
    else if( pSdrObj )
    {
        std::unique_ptr< XclExpDffAnchorBase > xDffAnchor( mrEscherEx.CreateDffAnchor( *pSdrObj ) );
        xDffAnchor->WriteDffData( mrEscherEx );
    }
}

void XclObj::SetEscherShapeType( sal_uInt16 nType )
{
//2do: what about the other defined ot... types?
    switch ( nType )
    {
        case ESCHER_ShpInst_Line :
            mnObjType = EXC_OBJTYPE_LINE;
        break;
        case ESCHER_ShpInst_Rectangle :
        case ESCHER_ShpInst_RoundRectangle :
            mnObjType = EXC_OBJTYPE_RECTANGLE;
        break;
        case ESCHER_ShpInst_Ellipse :
            mnObjType = EXC_OBJTYPE_OVAL;
        break;
        case ESCHER_ShpInst_Arc :
            mnObjType = EXC_OBJTYPE_ARC;
        break;
        case ESCHER_ShpInst_TextBox :
            mnObjType = EXC_OBJTYPE_TEXT;
        break;
        case ESCHER_ShpInst_PictureFrame :
            mnObjType = EXC_OBJTYPE_PICTURE;
        break;
        default:
            mnObjType = EXC_OBJTYPE_DRAWING;
    }
}

void XclObj::SetText( const XclExpRoot& rRoot, const SdrTextObj& rObj )
{
    OSL_ENSURE( !pClientTextbox, "XclObj::SetText: already set" );
    if ( !pClientTextbox )
    {
        mrEscherEx.UpdateDffFragmentEnd();
        pClientTextbox = new XclExpMsoDrawing( mrEscherEx );
        mrEscherEx.AddAtom( 0, ESCHER_ClientTextbox );    // TXO record
        mrEscherEx.UpdateDffFragmentEnd();
        pTxo = new XclTxo( rRoot, rObj );
    }
}

void XclObj::WriteBody( XclExpStream& rStrm )
{
    OSL_ENSURE( mnObjType != EXC_OBJTYPE_UNKNOWN, "XclObj::WriteBody - unknown type" );

    // create a substream to be able to create subrecords
    SvMemoryStream aMemStrm;
    std::unique_ptr< XclExpStream > pXclStrm( new XclExpStream( aMemStrm, rStrm.GetRoot() ) );

    // write the ftCmo subrecord
    pXclStrm->StartRecord( EXC_ID_OBJCMO, 18 );
    *pXclStrm << mnObjType << nObjId << nGrbit;
    pXclStrm->WriteZeroBytes( 12 );
    pXclStrm->EndRecord();

    // write other subrecords
    WriteSubRecs( *pXclStrm );

    // write the ftEnd subrecord
    pXclStrm->StartRecord( EXC_ID_OBJEND, 0 );
    pXclStrm->EndRecord();

    // copy the data to the OBJ record
    pXclStrm.reset();
    aMemStrm.Seek( 0 );
    rStrm.CopyFromStream( aMemStrm );
}

void XclObj::Save( XclExpStream& rStrm )
{
    // MSODRAWING record (msofbtSpContainer)
    if ( !bFirstOnSheet )
        pMsodrawing->Save( rStrm );

    // OBJ
    XclExpRecord::Save( rStrm );

    // second MSODRAWING record and TXO and CONTINUE records
    SaveTextRecs( rStrm );
}

void XclObj::WriteSubRecs( XclExpStream& /*rStrm*/ )
{
}

void XclObj::SaveTextRecs( XclExpStream& rStrm )
{
    // MSODRAWING record (msofbtClientTextbox)
    if ( pClientTextbox )
        pClientTextbox->Save( rStrm );
    // TXO and CONTINUE records
    if ( pTxo )
        pTxo->Save( rStrm );
}

  // --- class XclObjComment ------------------------------------------

XclObjComment::XclObjComment( XclExpObjectManager& rObjMgr, const Rectangle& rRect, const EditTextObject& rEditObj, SdrCaptionObj* pCaption, bool bVisible, const ScAddress& rAddress, Rectangle &rFrom, Rectangle &rTo ) :
    XclObj( rObjMgr, EXC_OBJTYPE_NOTE, true )
            , maScPos( rAddress )
            , mpCaption( pCaption->Clone() )
            , mbVisible( bVisible )
            , maFrom ( rFrom )
            , maTo ( rTo )
{
    ProcessEscherObj( rObjMgr.GetRoot(), rRect, pCaption, bVisible);
    // TXO
    pTxo = new XclTxo( rObjMgr.GetRoot(), rEditObj, pCaption );
}

static void lcl_FillProps( EscherPropertyContainer& rPropOpt, SdrObject* pCaption, bool bVisible )
{
    if( pCaption )
    {
        Reference< XShape > aXShape = GetXShapeForSdrObject( pCaption );
        Reference< XPropertySet > aXPropSet( aXShape, UNO_QUERY );
        if( aXPropSet.is() )
        {
            rPropOpt.CreateFillProperties( aXPropSet, true);

            rPropOpt.AddOpt( ESCHER_Prop_lTxid, 0 );                        // undocumented
            rPropOpt.AddOpt( 0x0158, 0x00000000 );                          // undocumented

            sal_uInt32 nValue = 0;
            if( !rPropOpt.GetOpt( ESCHER_Prop_FitTextToShape, nValue ) )
                rPropOpt.AddOpt( ESCHER_Prop_FitTextToShape, 0x00080008 );      // bool field

            if( rPropOpt.GetOpt( ESCHER_Prop_fillColor, nValue ) )
            {
                // If the Colour is the same as the 'ToolTip' System colour then
                // use the default rather than the explicit colour value. This will
                // be incorrect where user has chosen to use this colour explicity.
                Color aColor( (sal_uInt8)nValue, (sal_uInt8)( nValue >> 8 ), (sal_uInt8)( nValue >> 16 ) );
                const StyleSettings& rSett = Application::GetSettings().GetStyleSettings();
                if( aColor == rSett.GetHelpColor().GetColor() )
                {
                    rPropOpt.AddOpt( ESCHER_Prop_fillColor, 0x08000050 );
                    rPropOpt.AddOpt( ESCHER_Prop_fillBackColor, 0x08000050 );
                }
            }
            else
                rPropOpt.AddOpt( ESCHER_Prop_fillColor, 0x08000050 );

            if( !rPropOpt.GetOpt( ESCHER_Prop_fillBackColor, nValue ) )
                rPropOpt.AddOpt( ESCHER_Prop_fillBackColor, 0x08000050 );
            if( !rPropOpt.GetOpt( ESCHER_Prop_fNoFillHitTest, nValue ) )
                rPropOpt.AddOpt( ESCHER_Prop_fNoFillHitTest, 0x00110010 );      // bool field
            if( !rPropOpt.GetOpt( ESCHER_Prop_shadowColor, nValue ) )
                rPropOpt.AddOpt( ESCHER_Prop_shadowColor, 0x00000000 );
            if( !rPropOpt.GetOpt( ESCHER_Prop_fshadowObscured, nValue ) )       // bool field
                rPropOpt.AddOpt( ESCHER_Prop_fshadowObscured, 0x00030003 );     // bool field
        }
    }

    sal_uInt32 nFlags = 0x000A0000;
    ::set_flag( nFlags, sal_uInt32(2), !bVisible );
    rPropOpt.AddOpt( ESCHER_Prop_fPrint, nFlags );                  // bool field
}

void XclObjComment::ProcessEscherObj( const XclExpRoot& rRoot, const Rectangle& rRect, SdrObject* pCaption, const bool bVisible )
{
    EscherPropertyContainer aPropOpt;

    lcl_FillProps( aPropOpt, pCaption, bVisible );

    nGrbit = 0;     // all off: AutoLine, AutoFill, Printable, Locked
    mrEscherEx.OpenContainer( ESCHER_SpContainer );
    mrEscherEx.AddShape( ESCHER_ShpInst_TextBox, SHAPEFLAG_HAVEANCHOR | SHAPEFLAG_HAVESPT );
    aPropOpt.Commit( mrEscherEx.GetStream() );

    XclExpDffNoteAnchor( rRoot, rRect ).WriteDffData( mrEscherEx );

    mrEscherEx.AddAtom( 0, ESCHER_ClientData );                        // OBJ record
    mrEscherEx.UpdateDffFragmentEnd();

    //! Be sure to construct the MSODRAWING ClientTextbox record _after_ the
    //! base OBJ's MSODRAWING record Escher data is completed.
    pClientTextbox = new XclExpMsoDrawing( mrEscherEx );
    mrEscherEx.AddAtom( 0, ESCHER_ClientTextbox );    // TXO record
    mrEscherEx.UpdateDffFragmentEnd();
    mrEscherEx.CloseContainer();   // ESCHER_SpContainer
}

XclObjComment::~XclObjComment()
{
}

void XclObjComment::Save( XclExpStream& rStrm )
{
    // content of this record
    XclObj::Save( rStrm );
}

class VmlCommentExporter : public VMLExport
{
    ScAddress           maScPos;
    SdrCaptionObj*      mpCaption;
    bool                mbVisible;
    Rectangle           maFrom;
    Rectangle           maTo;

public:
                        VmlCommentExporter ( sax_fastparser::FSHelperPtr p, ScAddress aScPos, SdrCaptionObj* pCaption, bool bVisible, Rectangle &aFrom, Rectangle &aTo );
protected:
    virtual void        Commit( EscherPropertyContainer& rProps, const Rectangle& rRect ) override;
    using VMLExport::StartShape;
    virtual sal_Int32   StartShape() override;
    using VMLExport::EndShape;
    virtual void        EndShape( sal_Int32 nShapeElement ) override;
};

VmlCommentExporter::VmlCommentExporter( sax_fastparser::FSHelperPtr p, ScAddress aScPos, SdrCaptionObj* pCaption,
                                        bool bVisible, Rectangle &aFrom, Rectangle &aTo )
    : VMLExport( p )
    , maScPos( aScPos )
    , mpCaption( pCaption )
    , mbVisible( bVisible )
    , maFrom ( aFrom )
    , maTo ( aTo )
{
}

void VmlCommentExporter::Commit( EscherPropertyContainer& rProps, const Rectangle& rRect )
{
    lcl_FillProps( rProps, mpCaption, mbVisible );
    rProps.AddOpt( ESCHER_Prop_fHidden, sal_uInt32(mbVisible) ); // bool field

    // shadow property value for comment ( set in lcl_FillProps [*] ) has been
    // overwritten by new value ( 0x20000 ) in the generic part of the export
    // ( see  EscherPropertyContainer::CreateShadowProperties )
    // Safer option here is to just force the needed value here for oox vml
    // export alone ( and avoid potential problems with binary export )
    // #TODO investigate value of ESCHER_Prop_fshadowObscured generally
    // in binary export ( if indeed this value is good for binary export )
    // we can change the heuristics and/or initialisation path and get
    // rid of line below.
    // [*] lcl_FillProps seems to be called twice when exporting to xlsx
    // once from XclObjComment::ProcessEscherObj #TODO look into that also
    rProps.AddOpt( ESCHER_Prop_fshadowObscured, 0x00030003 ); // force value for comments

    VMLExport::Commit( rProps, rRect );
}

sal_Int32 VmlCommentExporter::StartShape()
{
    AddShapeAttribute( XML_type, OString( "#_x0000_t202") );

    sal_Int32 nId = VMLExport::StartShape();

    return nId;
}

void VmlCommentExporter::EndShape( sal_Int32 nShapeElement )
{
    char pAnchor[100];
    sax_fastparser::FSHelperPtr pVmlDrawing = GetFS();
    snprintf( pAnchor, 100, "%ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld",
                  maFrom.Left(), maFrom.Top(), maFrom.Right(), maFrom.Bottom(),
                  maTo.Left(), maTo.Top(), maTo.Right(), maTo.Bottom() );

    pVmlDrawing->startElement( FSNS( XML_x, XML_ClientData ),
            XML_ObjectType, "Note",
            FSEND );
    pVmlDrawing->singleElement( FSNS( XML_x, XML_MoveWithCells ),
            FSEND );
    pVmlDrawing->singleElement( FSNS( XML_x, XML_SizeWithCells ),
            FSEND );
    XclXmlUtils::WriteElement( pVmlDrawing, FSNS( XML_x, XML_Anchor ), pAnchor );
    XclXmlUtils::WriteElement( pVmlDrawing, FSNS( XML_x, XML_AutoFill ), "False" );
    XclXmlUtils::WriteElement( pVmlDrawing, FSNS( XML_x, XML_Row ), maScPos.Row() );
    XclXmlUtils::WriteElement( pVmlDrawing, FSNS( XML_x, XML_Column ), sal_Int32( maScPos.Col() ) );
    pVmlDrawing->endElement( FSNS( XML_x, XML_ClientData ) );

    VMLExport::EndShape( nShapeElement );
}

void XclObjComment::SaveXml( XclExpXmlStream& rStrm )
{
    VmlCommentExporter aCommentExporter( rStrm.GetCurrentStream(), maScPos, mpCaption.get(), mbVisible, maFrom, maTo );
    aCommentExporter.AddSdrObject( *mpCaption );
}

// --- class XclObjDropDown ------------------------------------------

XclObjDropDown::XclObjDropDown( XclExpObjectManager& rObjMgr, const ScAddress& rPos, bool bFilt ) :
    XclObj( rObjMgr, EXC_OBJTYPE_DROPDOWN, true ),
    bIsFiltered( bFilt )
{
    SetLocked( true );
    SetPrintable( false );
    SetAutoFill( true );
    SetAutoLine( false );
    nGrbit |= 0x0100;   // undocumented
    mrEscherEx.OpenContainer( ESCHER_SpContainer );
    mrEscherEx.AddShape( ESCHER_ShpInst_HostControl, SHAPEFLAG_HAVEANCHOR | SHAPEFLAG_HAVESPT );
    EscherPropertyContainer aPropOpt;
    aPropOpt.AddOpt( ESCHER_Prop_LockAgainstGrouping, 0x01040104 ); // bool field
    aPropOpt.AddOpt( ESCHER_Prop_FitTextToShape, 0x00080008 );      // bool field
    aPropOpt.AddOpt( ESCHER_Prop_fNoFillHitTest, 0x00010000 );      // bool field
    aPropOpt.AddOpt( ESCHER_Prop_fNoLineDrawDash, 0x00080000 );     // bool field
    aPropOpt.AddOpt( ESCHER_Prop_fPrint, 0x000A0000 );              // bool field
    aPropOpt.Commit( mrEscherEx.GetStream() );

    XclExpDffDropDownAnchor( rObjMgr.GetRoot(), rPos ).WriteDffData( mrEscherEx );

    mrEscherEx.AddAtom( 0, ESCHER_ClientData );                        // OBJ record
    mrEscherEx.UpdateDffFragmentEnd();
    mrEscherEx.CloseContainer();   // ESCHER_SpContainer

    // old size + ftSbs + ftLbsData
    AddRecSize( 24 + 20 );
}

XclObjDropDown::~XclObjDropDown()
{
}

void XclObjDropDown::WriteSubRecs( XclExpStream& rStrm )
{
    // ftSbs subrecord - Scroll bars (dummy)
    rStrm.StartRecord( EXC_ID_OBJSBS, 20 );
    rStrm.WriteZeroBytes( 20 );
    rStrm.EndRecord();

    // ftLbsData subrecord - Listbox data
    sal_uInt16 nDropDownFlags = 0;
    ::insert_value( nDropDownFlags, EXC_OBJ_DROPDOWN_SIMPLE, 0, 2 );
    ::set_flag( nDropDownFlags, EXC_OBJ_DROPDOWN_FILTERED, bIsFiltered );
    rStrm.StartRecord( EXC_ID_OBJLBSDATA, 16 );
    rStrm   << (sal_uInt32)0 << (sal_uInt16)0 << (sal_uInt16)0x0301 << (sal_uInt16)0
            << nDropDownFlags << sal_uInt16( 20 ) << sal_uInt16( 130 );
    rStrm.EndRecord();
}

// --- class XclTxo --------------------------------------------------

static sal_uInt8 lcl_GetHorAlignFromItemSet( const SfxItemSet& rItemSet )
{
    sal_uInt8 nHorAlign = EXC_OBJ_HOR_LEFT;

    switch( static_cast< const SvxAdjustItem& >( rItemSet.Get( EE_PARA_JUST ) ).GetAdjust() )
    {
        case SVX_ADJUST_LEFT:   nHorAlign = EXC_OBJ_HOR_LEFT;      break;
        case SVX_ADJUST_CENTER: nHorAlign = EXC_OBJ_HOR_CENTER;    break;
        case SVX_ADJUST_RIGHT:  nHorAlign = EXC_OBJ_HOR_RIGHT;     break;
        case SVX_ADJUST_BLOCK:  nHorAlign = EXC_OBJ_HOR_JUSTIFY;   break;
        default:;
    }
    return nHorAlign;
}

static sal_uInt8 lcl_GetVerAlignFromItemSet( const SfxItemSet& rItemSet )
{
    sal_uInt8 nVerAlign = EXC_OBJ_VER_TOP;

    switch( static_cast< const SdrTextVertAdjustItem& >( rItemSet.Get( SDRATTR_TEXT_VERTADJUST ) ).GetValue() )
    {
        case SDRTEXTVERTADJUST_TOP:     nVerAlign = EXC_OBJ_VER_TOP;       break;
        case SDRTEXTVERTADJUST_CENTER:  nVerAlign = EXC_OBJ_VER_CENTER;    break;
        case SDRTEXTVERTADJUST_BOTTOM:  nVerAlign = EXC_OBJ_VER_BOTTOM;    break;
        case SDRTEXTVERTADJUST_BLOCK:   nVerAlign = EXC_OBJ_VER_JUSTIFY;   break;
    }
    return nVerAlign;
}

XclTxo::XclTxo( const OUString& rString, sal_uInt16 nFontIx ) :
    mpString( new XclExpString( rString ) ),
    mnRotation( EXC_OBJ_ORIENT_NONE ),
    mnHorAlign( EXC_OBJ_HOR_LEFT ),
    mnVerAlign( EXC_OBJ_VER_TOP )
{
    if( mpString->Len() )
    {
        // If there is text, Excel *needs* the 2nd CONTINUE record with at least two format runs
        mpString->AppendFormat( 0, nFontIx );
        mpString->AppendFormat( mpString->Len(), EXC_FONT_APP );
    }
}

XclTxo::XclTxo( const XclExpRoot& rRoot, const SdrTextObj& rTextObj ) :
    mpString( XclExpStringHelper::CreateString( rRoot, rTextObj ) ),
    mnRotation( EXC_OBJ_ORIENT_NONE ),
    mnHorAlign( EXC_OBJ_HOR_LEFT ),
    mnVerAlign( EXC_OBJ_VER_TOP )
{
    // additional alignment and orientation items
    const SfxItemSet& rItemSet = rTextObj.GetMergedItemSet();

    // horizontal alignment
    SetHorAlign( lcl_GetHorAlignFromItemSet( rItemSet ) );

    // vertical alignment
    SetVerAlign( lcl_GetVerAlignFromItemSet( rItemSet ) );

    // rotation
    long nAngle = rTextObj.GetRotateAngle();
    if( (4500 < nAngle) && (nAngle < 13500) )
        mnRotation = EXC_OBJ_ORIENT_90CCW;
    else if( (22500 < nAngle) && (nAngle < 31500) )
        mnRotation = EXC_OBJ_ORIENT_90CW;
    else
        mnRotation = EXC_OBJ_ORIENT_NONE;
}

XclTxo::XclTxo( const XclExpRoot& rRoot, const EditTextObject& rEditObj, SdrObject* pCaption ) :
    mpString( XclExpStringHelper::CreateString( rRoot, rEditObj ) ),
    mnRotation( EXC_OBJ_ORIENT_NONE ),
    mnHorAlign( EXC_OBJ_HOR_LEFT ),
    mnVerAlign( EXC_OBJ_VER_TOP )
{
    if(pCaption)
    {
        // Excel has one alignment per NoteObject while Calc supports
        // one alignment per paragraph - use the first paragraph
        // alignment (if set) as our overall alignment.
        OUString aParaText( rEditObj.GetText( 0 ) );
        if( !aParaText.isEmpty() )
        {
            const SfxItemSet& aSet( rEditObj.GetParaAttribs( 0));
            const SfxPoolItem* pItem = nullptr;
            if( aSet.GetItemState( EE_PARA_JUST, true, &pItem ) == SfxItemState::SET )
            {
                SvxAdjust eEEAlign = static_cast< const SvxAdjustItem& >( *pItem ).GetAdjust();
                pCaption->SetMergedItem( SvxAdjustItem( eEEAlign, EE_PARA_JUST ) );
            }
        }
        const SfxItemSet& rItemSet = pCaption->GetMergedItemSet();

        // horizontal alignment
        SetHorAlign( lcl_GetHorAlignFromItemSet( rItemSet ) );

        // vertical alignment
        SetVerAlign( lcl_GetVerAlignFromItemSet( rItemSet ) );

        // orientation alignment
        const SvxWritingModeItem& rItem = static_cast< const SvxWritingModeItem& >( rItemSet.Get( SDRATTR_TEXTDIRECTION ) );
        if( rItem.GetValue() == css::text::WritingMode_TB_RL )
            mnRotation = EXC_OBJ_ORIENT_90CW;
    }
}

void XclTxo::SaveCont( XclExpStream& rStrm )
{
    OSL_ENSURE( mpString.get(), "XclTxo::SaveCont - missing string" );

    // #i96858# do not save existing string formatting if text is empty
    sal_uInt16 nRunLen = mpString->IsEmpty() ? 0 : (8 * mpString->GetFormatsCount());
    // alignment
    sal_uInt16 nFlags = 0;
    ::insert_value( nFlags, mnHorAlign, 1, 3 );
    ::insert_value( nFlags, mnVerAlign, 4, 3 );

    rStrm << nFlags << mnRotation;
    rStrm.WriteZeroBytes( 6 );
    rStrm << mpString->Len() << nRunLen << sal_uInt32( 0 );
}

void XclTxo::Save( XclExpStream& rStrm )
{
    // Write the TXO part
    ExcRecord::Save( rStrm );

    // CONTINUE records are only written if there is some text
    if( !mpString->IsEmpty() )
    {
        // CONTINUE for character array
        rStrm.StartRecord( EXC_ID_CONT, mpString->GetBufferSize() + 1 );
        rStrm << static_cast< sal_uInt8 >( mpString->GetFlagField() & EXC_STRF_16BIT ); // only Unicode flag
        mpString->WriteBuffer( rStrm );
        rStrm.EndRecord();

        // CONTINUE for formatting runs
        rStrm.StartRecord( EXC_ID_CONT, 8 * mpString->GetFormatsCount() );
        const XclFormatRunVec& rFormats = mpString->GetFormats();
        for( XclFormatRunVec::const_iterator aIt = rFormats.begin(), aEnd = rFormats.end(); aIt != aEnd; ++aIt )
            rStrm << aIt->mnChar << aIt->mnFontIdx << sal_uInt32( 0 );
        rStrm.EndRecord();
    }
}

sal_uInt16 XclTxo::GetNum() const
{
    return EXC_ID_TXO;
}

std::size_t XclTxo::GetLen() const
{
    return 18;
}

// --- class XclObjOle -------------------------------------------

XclObjOle::XclObjOle( XclExpObjectManager& rObjMgr, const SdrObject& rObj ) :
    XclObj( rObjMgr, EXC_OBJTYPE_PICTURE ),
    rOleObj( rObj ),
    pRootStorage( rObjMgr.GetRoot().GetRootStorage().get() )
{
}

XclObjOle::~XclObjOle()
{
}

void XclObjOle::WriteSubRecs( XclExpStream& rStrm )
{
    // write only as embedded, not linked
    OUString        aStorageName( "MBD" );
    sal_Char        aBuf[ sizeof(sal_uInt32) * 2 + 1 ];
    // FIXME Eeek! Is this just a way to get a unique id?
    sal_uInt32          nPictureId = sal_uInt32(sal_uIntPtr(this) >> 2);
    sprintf( aBuf, "%08X", static_cast< unsigned int >( nPictureId ) );
    aStorageName += OUString::createFromAscii(aBuf);
    tools::SvRef<SotStorage>    xOleStg = pRootStorage->OpenSotStorage( aStorageName );
    if( xOleStg.is() )
    {
        uno::Reference < embed::XEmbeddedObject > xObj( static_cast<const SdrOle2Obj&>(rOleObj).GetObjRef() );
        if ( xObj.is() )
        {
            // set version to "old" version, because it must be
            // saved in MS notation.
            sal_uInt32                  nFl = 0;
            const SvtFilterOptions& rFltOpts = SvtFilterOptions::Get();
            if( rFltOpts.IsMath2MathType() )
                nFl |= OLE_STARMATH_2_MATHTYPE;

            if( rFltOpts.IsWriter2WinWord() )
                nFl |= OLE_STARWRITER_2_WINWORD;

            if( rFltOpts.IsCalc2Excel() )
                nFl |= OLE_STARCALC_2_EXCEL;

            if( rFltOpts.IsImpress2PowerPoint() )
                nFl |= OLE_STARIMPRESS_2_POWERPOINT;

            SvxMSExportOLEObjects   aOLEExpFilt( nFl );
            aOLEExpFilt.ExportOLEObject( xObj, *xOleStg );

            // OBJCF subrecord, undocumented as usual
            rStrm.StartRecord( EXC_ID_OBJCF, 2 );
            rStrm << sal_uInt16(0x0002);
            rStrm.EndRecord();

            // OBJFLAGS subrecord, undocumented as usual
            rStrm.StartRecord( EXC_ID_OBJFLAGS, 2 );
            sal_uInt16 nFlags = EXC_OBJ_PIC_MANUALSIZE;
            ::set_flag( nFlags, EXC_OBJ_PIC_SYMBOL, static_cast<const SdrOle2Obj&>(rOleObj).GetAspect() == embed::Aspects::MSOLE_ICON );
            rStrm << nFlags;
            rStrm.EndRecord();

            // OBJPICTFMLA subrecord, undocumented as usual
            XclExpString aName( xOleStg->GetUserName() );
            sal_uInt16 nPadLen = (sal_uInt16)(aName.GetSize() & 0x01);
            sal_uInt16 nFmlaLen = static_cast< sal_uInt16 >( 12 + aName.GetSize() + nPadLen );
            sal_uInt16 nSubRecLen = nFmlaLen + 6;

            rStrm.StartRecord( EXC_ID_OBJPICTFMLA, nSubRecLen );
            rStrm   << nFmlaLen
                    << sal_uInt16( 5 ) << sal_uInt32( 0 ) << sal_uInt8( 2 )
                    << sal_uInt32( 0 ) << sal_uInt8( 3 )
                    << aName;
            if( nPadLen )
                rStrm << sal_uInt8( 0 );       // pad byte
            rStrm << nPictureId;
            rStrm.EndRecord();
        }
    }
}

void XclObjOle::Save( XclExpStream& rStrm )
{
    // content of this record
    XclObj::Save( rStrm );
}

// --- class XclObjAny -------------------------------------------

XclObjAny::XclObjAny( XclExpObjectManager& rObjMgr, const Reference< XShape >& rShape, ScDocument* pDoc )
    : XclObj( rObjMgr, EXC_OBJTYPE_UNKNOWN )
    , mxShape( rShape )
    , mpDoc(pDoc)
{
}

XclObjAny::~XclObjAny()
{
}

void XclObjAny::WriteSubRecs( XclExpStream& rStrm )
{
    if( mnObjType == EXC_OBJTYPE_GROUP )
        // ftGmo subrecord
        rStrm << EXC_ID_OBJGMO << sal_uInt16(2) << sal_uInt16(0);
}

void XclObjAny::Save( XclExpStream& rStrm )
{
    if( mnObjType == EXC_OBJTYPE_GROUP )
        // old size + ftGmo
        AddRecSize( 6 );

    // content of this record
    XclObj::Save( rStrm );
}

// --- class ExcBof8_Base --------------------------------------------

ExcBof8_Base::ExcBof8_Base()
{
    nVers           = 0x0600;
    nRupBuild       = 0x0dbb;
    nRupYear        = 0x07cc;
    nLowestBiffVer  = 0x00000006;   // Biff8
}
void XclObjAny::WriteFromTo( XclExpXmlStream& rStrm, const Reference< XShape >& rShape, SCTAB nTab )
{
    sax_fastparser::FSHelperPtr pDrawing = rStrm.GetCurrentStream();

    awt::Point  aTopLeft    = rShape->getPosition();
    awt::Size   aSize       = rShape->getSize();
    Rectangle   aLocation( aTopLeft.X, aTopLeft.Y, aTopLeft.X + aSize.Width, aTopLeft.Y + aSize.Height );
    ScRange     aRange      = rStrm.GetRoot().GetDoc().GetRange( nTab, aLocation );
    Rectangle   aRangeRect  = rStrm.GetRoot().GetDoc().GetMMRect( aRange.aStart.Col(), aRange.aStart.Row(),
            aRange.aEnd.Col()-1, aRange.aEnd.Row()-1,
            nTab );

    pDrawing->startElement( FSNS( XML_xdr, XML_from ),
            FSEND );
    XclXmlUtils::WriteElement( pDrawing, FSNS( XML_xdr, XML_col ), (sal_Int32) aRange.aStart.Col() );
    XclXmlUtils::WriteElement( pDrawing, FSNS( XML_xdr, XML_colOff ),
            oox::drawingml::convertHmmToEmu( aLocation.Left() - aRangeRect.Left() ) );
    XclXmlUtils::WriteElement( pDrawing, FSNS( XML_xdr, XML_row ), (sal_Int32) aRange.aStart.Row() );
    XclXmlUtils::WriteElement( pDrawing, FSNS( XML_xdr, XML_rowOff ),
            oox::drawingml::convertHmmToEmu( aLocation.Top() - aRangeRect.Top() ) );
    pDrawing->endElement( FSNS( XML_xdr, XML_from ) );

    pDrawing->startElement( FSNS( XML_xdr, XML_to ),
            FSEND );
    XclXmlUtils::WriteElement( pDrawing, FSNS( XML_xdr, XML_col ), (sal_Int32) aRange.aEnd.Col() );
    XclXmlUtils::WriteElement( pDrawing, FSNS( XML_xdr, XML_colOff ),
            oox::drawingml::convertHmmToEmu( aLocation.Right() - aRangeRect.Right() ) );
    XclXmlUtils::WriteElement( pDrawing, FSNS( XML_xdr, XML_row ), (sal_Int32) aRange.aEnd.Row() );
    XclXmlUtils::WriteElement( pDrawing, FSNS( XML_xdr, XML_rowOff ),
            oox::drawingml::convertHmmToEmu( aLocation.Bottom() - aRangeRect.Bottom() ) );
    pDrawing->endElement( FSNS( XML_xdr, XML_to ) );
}

void XclObjAny::WriteFromTo( XclExpXmlStream& rStrm, const XclObjAny& rObj )
{
    WriteFromTo( rStrm, rObj.GetShape(), rObj.GetTab() );
}

static const char*
GetEditAs( XclObjAny& rObj )
{
    if( const SdrObject* pShape = EscherEx::GetSdrObject( rObj.GetShape() ) )
    {
        // OOXTODO: returning "twoCell"
        switch( ScDrawLayer::GetAnchorType( *pShape ) )
        {
            case SCA_CELL:  return "oneCell";
            default:        break;
        }
    }
    return "absolute";
}

namespace {

ScRefFlags parseRange(const OUString& rString, ScRange& rRange, ScDocument* pDoc)
{
    // start with the address convention set in the document
    formula::FormulaGrammar::AddressConvention eConv = pDoc->GetAddressConvention();
    ScRefFlags nResult = rRange.Parse(rString, pDoc, eConv);
    if ( nResult & ScRefFlags::VALID )
        return nResult;

    // try the default calc address convention
    nResult = rRange.Parse(rString, pDoc);
    if ( nResult & ScRefFlags::VALID )
        return nResult;

    // try excel a1
    nResult = rRange.Parse(rString, pDoc, formula::FormulaGrammar::CONV_XL_A1);
    if ( nResult & ScRefFlags::VALID )
        return nResult;

    // try r1c1
    return rRange.Parse(rString, pDoc, formula::FormulaGrammar::CONV_XL_R1C1);
}

ScRefFlags parseAddress(const OUString& rString, ScAddress& rAddress, ScDocument* pDoc)
{
    // start with the address convention set in the document
    formula::FormulaGrammar::AddressConvention eConv = pDoc->GetAddressConvention();
    ScRefFlags nResult = rAddress.Parse(rString, pDoc, eConv);
    if ( nResult & ScRefFlags::VALID )
        return nResult;

    // try the default calc address convention
    nResult = rAddress.Parse(rString, pDoc);
    if ( nResult & ScRefFlags::VALID )
        return nResult;

    // try excel a1
    nResult = rAddress.Parse(rString, pDoc, formula::FormulaGrammar::CONV_XL_A1);
    if ( nResult & ScRefFlags::VALID )
        return nResult;

    // try r1c1
    return rAddress.Parse(rString, pDoc, formula::FormulaGrammar::CONV_XL_R1C1);
}

bool transformURL(const OUString& rOldURL, OUString& rNewURL, ScDocument* pDoc)
{
    if (rOldURL.startsWith("#"))
    {
        //  URL has to be decoded for escaped characters (%20)
        OUString aURL = INetURLObject::decode( rOldURL,
                INetURLObject::DecodeMechanism::WithCharset );
        OUString aAddressString = aURL.copy(1);

        ScRange aRange;
        ScAddress aAddress;
        ScRefFlags nResult = parseRange(aAddressString, aRange, pDoc);
        if ( nResult & ScRefFlags::VALID )
        {
            OUString aString = aRange.Format(nResult, pDoc, formula::FormulaGrammar::CONV_XL_OOX);
            rNewURL = "#" + aString;
            return true;
        }
        else
        {
            nResult = parseAddress(aAddressString, aAddress, pDoc);
            if( nResult & ScRefFlags::VALID )
            {
                OUString aString = aAddress.Format(nResult, pDoc, formula::FormulaGrammar::CONV_XL_OOX);
                rNewURL = "#" + aString;
                return true;
            }
        }
    }

    rNewURL = rOldURL;
    return false;
}

class ScURLTransformer : public oox::drawingml::URLTransformer
{
public:
    explicit ScURLTransformer(ScDocument& rDoc)
        : mrDoc(rDoc)
    {
    }

    virtual OUString getTransformedString(const OUString& rURL) const override
    {
        OUString aNewURL;
        transformURL(rURL, aNewURL, &mrDoc);
        return aNewURL;
    }

    virtual bool isExternalURL(const OUString& rURL) const override
    {
        OUString aNewURL;
        return transformURL(rURL, aNewURL, &mrDoc);
    }

private:
    ScDocument& mrDoc;
};

}

void XclObjAny::SaveXml( XclExpXmlStream& rStrm )
{
    // ignore group shapes at the moment, we don't process them correctly
    // leading to ms2010 rejecting the content
    if( !mxShape.is() || mxShape->getShapeType() == "com.sun.star.drawing.GroupShape" )
        return;

    sax_fastparser::FSHelperPtr pDrawing = rStrm.GetCurrentStream();

    ShapeExport aDML(XML_xdr, pDrawing, nullptr, &rStrm, drawingml::DOCUMENT_XLSX);
    std::shared_ptr<oox::drawingml::URLTransformer> pURLTransformer(new ScURLTransformer(*mpDoc));
    aDML.SetURLTranslator(pURLTransformer);

    pDrawing->startElement( FSNS( XML_xdr, XML_twoCellAnchor ), // OOXTODO: oneCellAnchor, absoluteAnchor
            XML_editAs, GetEditAs( *this ),
            FSEND );
    Reference< XPropertySet > xPropSet( mxShape, UNO_QUERY );
    if (xPropSet.is())
    {
        WriteFromTo( rStrm, *this );
        aDML.WriteShape( mxShape );
    }

    pDrawing->singleElement( FSNS( XML_xdr, XML_clientData),
            // OOXTODO: XML_fLocksWithSheet
            // OOXTODO: XML_fPrintsWithSheet
            FSEND );
    pDrawing->endElement( FSNS( XML_xdr, XML_twoCellAnchor ) );
}

void ExcBof8_Base::SaveCont( XclExpStream& rStrm )
{
    rStrm.DisableEncryption();
    rStrm   << nVers << nDocType << nRupBuild << nRupYear
            << sal_uInt32(0)/*nFileHistory*/ << nLowestBiffVer;
}

sal_uInt16 ExcBof8_Base::GetNum() const
{
    return 0x0809;
}

std::size_t ExcBof8_Base::GetLen() const
{
    return 16;
}

// --- class ExcBof8 -------------------------------------------------

ExcBof8::ExcBof8()
{
    nDocType = 0x0010;
}

// --- class ExcBofW8 ------------------------------------------------

ExcBofW8::ExcBofW8()
{
    nDocType = 0x0005;
}

// --- class ExcBundlesheet8 -----------------------------------------

ExcBundlesheet8::ExcBundlesheet8( RootData& rRootData, SCTAB _nTab ) :
    ExcBundlesheetBase( rRootData, static_cast<sal_uInt16>(_nTab) ),
    sUnicodeName( rRootData.pER->GetTabInfo().GetScTabName( _nTab ) )
{
}

ExcBundlesheet8::ExcBundlesheet8( const OUString& rString ) :
    ExcBundlesheetBase(),
    sUnicodeName( rString )
{
}

void ExcBundlesheet8::SaveCont( XclExpStream& rStrm )
{
    m_nOwnPos = rStrm.GetSvStreamPos();
    // write dummy position, real position comes later
    rStrm.DisableEncryption();
    rStrm << sal_uInt32(0);
    rStrm.EnableEncryption();
    rStrm << nGrbit << GetName();
}

std::size_t ExcBundlesheet8::GetLen() const
{   // Text max 255 chars
    return 8 + GetName().GetBufferSize();
}

void ExcBundlesheet8::SaveXml( XclExpXmlStream& rStrm )
{
    OUString sId;
    rStrm.CreateOutputStream(
            XclXmlUtils::GetStreamName( "xl/", "worksheets/sheet", nTab+1),
            XclXmlUtils::GetStreamName( nullptr, "worksheets/sheet", nTab+1),
            rStrm.GetCurrentStream()->getOutputStream(),
            "application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml",
            "http://schemas.openxmlformats.org/officeDocument/2006/relationships/worksheet",
            &sId );

    rStrm.GetCurrentStream()->singleElement( XML_sheet,
            XML_name,               XclXmlUtils::ToOString( sUnicodeName ).getStr(),
            XML_sheetId,            OString::number( ( nTab+1 ) ).getStr(),
            XML_state,              nGrbit == 0x0000 ? "visible" : "hidden",
            FSNS( XML_r, XML_id ),  XclXmlUtils::ToOString( sId ).getStr(),
            FSEND );
}

// --- class XclObproj -----------------------------------------------

sal_uInt16 XclObproj::GetNum() const
{
    return 0x00D3;
}

std::size_t XclObproj::GetLen() const
{
    return 0;
}

// ---- class XclCodename --------------------------------------------

XclCodename::XclCodename( const OUString& r ) : aName( r )
{
}

void XclCodename::SaveCont( XclExpStream& rStrm )
{
    rStrm << aName;
}

sal_uInt16 XclCodename::GetNum() const
{
    return 0x01BA;
}

std::size_t XclCodename::GetLen() const
{
    return aName.GetSize();
}

// ---- Scenarios ----------------------------------------------------

ExcEScenarioCell::ExcEScenarioCell( sal_uInt16 nC, sal_uInt16 nR, const OUString& rTxt ) :
        nCol( nC ),
        nRow( nR ),
        sText( rTxt, EXC_STR_DEFAULT, 255 )
{
}

void ExcEScenarioCell::WriteAddress( XclExpStream& rStrm ) const
{
    rStrm << nRow << nCol;
}

void ExcEScenarioCell::WriteText( XclExpStream& rStrm ) const
{
    rStrm << sText;
}

void ExcEScenarioCell::SaveXml( XclExpXmlStream& rStrm ) const
{
    rStrm.GetCurrentStream()->singleElement( XML_inputCells,
            // OOXTODO: XML_deleted,
            // OOXTODO: XML_numFmtId,
            XML_r,      XclXmlUtils::ToOString( ScAddress( nCol, nRow, 0 ) ).getStr(),
            // OOXTODO: XML_undone,
            XML_val,    XclXmlUtils::ToOString( sText ).getStr(),
            FSEND );
}

ExcEScenario::ExcEScenario( const XclExpRoot& rRoot, SCTAB nTab )
{
    OUString  sTmpName;
    OUString  sTmpComm;
    OUString aTmp;
    Color   aDummyCol;
    ScScenarioFlags nFlags;

    ScDocument& rDoc = rRoot.GetDoc();
    rDoc.GetName(nTab, aTmp);
    sTmpName = aTmp;
    sName.Assign( sTmpName, EXC_STR_8BITLENGTH );
    nRecLen = 8 + sName.GetBufferSize();

    rDoc.GetScenarioData( nTab, aTmp, aDummyCol, nFlags );
    sTmpComm = aTmp;
    sComment.Assign( sTmpComm, EXC_STR_DEFAULT, 255 );
    if( sComment.Len() )
        nRecLen += sComment.GetSize();
    bProtected = (nFlags & ScScenarioFlags::Protected) != ScScenarioFlags::NONE;

    sUserName.Assign( rRoot.GetUserName(), EXC_STR_DEFAULT, 255 );
    nRecLen += sUserName.GetSize();

    const ScRangeList* pRList = rDoc.GetScenarioRanges( nTab );
    if( !pRList )
        return;

    bool    bContLoop = true;
    SCROW   nRow;
    SCCOL   nCol;
    OUString  sText;
    double  fVal;

    for( size_t nRange = 0; (nRange < pRList->size()) && bContLoop; nRange++ )
    {
        const ScRange* pRange = (*pRList)[nRange];
        for( nRow = pRange->aStart.Row(); (nRow <= pRange->aEnd.Row()) && bContLoop; nRow++ )
            for( nCol = pRange->aStart.Col(); (nCol <= pRange->aEnd.Col()) && bContLoop; nCol++ )
            {
                if( rDoc.HasValueData( nCol, nRow, nTab ) )
                {
                    rDoc.GetValue( nCol, nRow, nTab, fVal );
                    sText = ::rtl::math::doubleToUString( fVal,
                            rtl_math_StringFormat_Automatic,
                            rtl_math_DecimalPlaces_Max,
                            ScGlobal::pLocaleData->getNumDecimalSep()[0],
                            true );
                }
                else
                    sText = rDoc.GetString(nCol, nRow, nTab);
                bContLoop = Append( static_cast<sal_uInt16>(nCol),
                        static_cast<sal_uInt16>(nRow), sText );
            }
    }
}

ExcEScenario::~ExcEScenario()
{
}

bool ExcEScenario::Append( sal_uInt16 nCol, sal_uInt16 nRow, const OUString& rTxt )
{
    if( aCells.size() == EXC_SCEN_MAXCELL )
        return false;

    ExcEScenarioCell aCell(nCol, nRow, rTxt);
    aCells.push_back(aCell);
    nRecLen += 6 + aCell.GetStringBytes();        // 4 bytes address, 2 bytes ifmt
    return true;
}

void ExcEScenario::SaveCont( XclExpStream& rStrm )
{
    sal_uInt16 count = aCells.size();

    rStrm   << (sal_uInt16) count               // number of cells
            << sal_uInt8(bProtected)            // fProtection
            << (sal_uInt8) 0                    // fHidden
            << (sal_uInt8) sName.Len()          // length of scen name
            << (sal_uInt8) sComment.Len()       // length of comment
            << (sal_uInt8) sUserName.Len();     // length of user name
    sName.WriteFlagField( rStrm );
    sName.WriteBuffer( rStrm );

    rStrm << sUserName;

    if( sComment.Len() )
        rStrm << sComment;

    std::vector<ExcEScenarioCell>::iterator pIter;
    for( pIter = aCells.begin(); pIter != aCells.end(); ++pIter )
        pIter->WriteAddress( rStrm );           // pos of cell
    for( pIter = aCells.begin(); pIter != aCells.end(); ++pIter )
        pIter->WriteText( rStrm );              // string content
    rStrm.SetSliceSize( 2 );
    rStrm.WriteZeroBytes( 2 * count );  // date format
}

sal_uInt16 ExcEScenario::GetNum() const
{
    return 0x00AF;
}

std::size_t ExcEScenario::GetLen() const
{
    return nRecLen;
}

void ExcEScenario::SaveXml( XclExpXmlStream& rStrm )
{
    sax_fastparser::FSHelperPtr& rWorkbook = rStrm.GetCurrentStream();
    rWorkbook->startElement( XML_scenario,
            XML_name,       XclXmlUtils::ToOString( sName ).getStr(),
            XML_locked,     XclXmlUtils::ToPsz( bProtected ),
            // OOXTODO: XML_hidden,
            XML_count,      OString::number(  aCells.size() ).getStr(),
            XML_user,       XESTRING_TO_PSZ( sUserName ),
            XML_comment,    XESTRING_TO_PSZ( sComment ),
            FSEND );

    std::vector<ExcEScenarioCell>::iterator pIter;
    for( pIter = aCells.begin(); pIter != aCells.end(); ++pIter )
        pIter->SaveXml( rStrm );

    rWorkbook->endElement( XML_scenario );
}

ExcEScenarioManager::ExcEScenarioManager( const XclExpRoot& rRoot, SCTAB nTab ) :
        nActive( 0 )
{
    ScDocument& rDoc = rRoot.GetDoc();
    if( rDoc.IsScenario( nTab ) )
        return;

    SCTAB nFirstTab = nTab + 1;
    SCTAB nNewTab       = nFirstTab;

    while( rDoc.IsScenario( nNewTab ) )
    {
        aScenes.emplace_back( rRoot, nNewTab  );

        if( rDoc.IsActiveScenario( nNewTab ) )
            nActive = static_cast<sal_uInt16>(nNewTab - nFirstTab);
        nNewTab++;
    }
}

ExcEScenarioManager::~ExcEScenarioManager()
{
}

void ExcEScenarioManager::SaveCont( XclExpStream& rStrm )
{
    rStrm   << (sal_uInt16) aScenes.size()  // number of scenarios
            << nActive                      // active scen
            << nActive                      // last displayed
            << (sal_uInt16) 0;              // reference areas
}

void ExcEScenarioManager::Save( XclExpStream& rStrm )
{
    if( !aScenes.empty() )
        ExcRecord::Save( rStrm );

    for( ExcEScenario& rScenario : aScenes )
        rScenario.Save( rStrm );
}

void ExcEScenarioManager::SaveXml( XclExpXmlStream& rStrm )
{
    if( aScenes.empty() )
        return;

    sax_fastparser::FSHelperPtr& rWorkbook = rStrm.GetCurrentStream();
    rWorkbook->startElement( XML_scenarios,
            XML_current,    OString::number( nActive ).getStr(),
            XML_show,       OString::number( nActive ).getStr(),
            // OOXTODO: XML_sqref,
            FSEND );

    for( ExcEScenario& rScenario : aScenes )
        rScenario.SaveXml( rStrm );

    rWorkbook->endElement( XML_scenarios );
}

sal_uInt16 ExcEScenarioManager::GetNum() const
{
    return 0x00AE;
}

std::size_t ExcEScenarioManager::GetLen() const
{
    return 8;
}

struct XclExpTabProtectOption
{
    ScTableProtection::Option   eOption;
    sal_uInt16                  nMask;
};

XclExpSheetProtectOptions::XclExpSheetProtectOptions( const XclExpRoot& rRoot, SCTAB nTab ) :
    XclExpRecord( 0x0867, 23 )
{
    static const XclExpTabProtectOption aTable[] =
    {
        { ScTableProtection::OBJECTS,               0x0001 },
        { ScTableProtection::SCENARIOS,             0x0002 },
        { ScTableProtection::FORMAT_CELLS,          0x0004 },
        { ScTableProtection::FORMAT_COLUMNS,        0x0008 },
        { ScTableProtection::FORMAT_ROWS,           0x0010 },
        { ScTableProtection::INSERT_COLUMNS,        0x0020 },
        { ScTableProtection::INSERT_ROWS,           0x0040 },
        { ScTableProtection::INSERT_HYPERLINKS,     0x0080 },

        { ScTableProtection::DELETE_COLUMNS,        0x0100 },
        { ScTableProtection::DELETE_ROWS,           0x0200 },
        { ScTableProtection::SELECT_LOCKED_CELLS,   0x0400 },
        { ScTableProtection::SORT,                  0x0800 },
        { ScTableProtection::AUTOFILTER,            0x1000 },
        { ScTableProtection::PIVOT_TABLES,          0x2000 },
        { ScTableProtection::SELECT_UNLOCKED_CELLS, 0x4000 },

        { ScTableProtection::NONE,                  0x0000 }
    };

    mnOptions = 0x0000;
    ScTableProtection* pProtect = rRoot.GetDoc().GetTabProtection(nTab);
    if (!pProtect)
        return;

    for (int i = 0; aTable[i].nMask != 0x0000; ++i)
    {
        if ( pProtect->isOptionEnabled(aTable[i].eOption) )
            mnOptions |= aTable[i].nMask;
    }
}

void XclExpSheetProtectOptions::WriteBody( XclExpStream& rStrm )
{
    sal_uInt16 nBytes = 0x0867;
    rStrm << nBytes;

    unsigned char nZero = 0x00;
    for (int i = 0; i < 9; ++i)
        rStrm << nZero;

    nBytes = 0x0200;
    rStrm << nBytes;
    nBytes = 0x0100;
    rStrm << nBytes;
    nBytes = 0xFFFF;
    rStrm << nBytes << nBytes;

    rStrm << mnOptions;
    nBytes = 0;
    rStrm << nBytes;
}

XclExpSheetEnhancedProtection::XclExpSheetEnhancedProtection( const XclExpRoot& rRoot,
        const ScEnhancedProtection & rProt ) :
    XclExpRecord( 0x0868 ),
    mrRoot( rRoot ),
    maEnhancedProtection( rProt )
{
}

void XclExpSheetEnhancedProtection::WriteBody( XclExpStream& rStrm )
{
    sal_uInt16 nRecordType = 0x0868;
    rStrm << nRecordType;                   // frtHeader rt
    rStrm.WriteZeroBytesToRecord(10);       // frtHeader unused
    rStrm << EXC_ISFPROTECTION;             // isf
    rStrm.WriteZeroBytesToRecord(5);        // reserved1 (1 bytes) and reserved2 (4 bytes)

    XclRangeList aRefs;
    if (maEnhancedProtection.maRangeList.is())
        mrRoot.GetAddressConverter().ConvertRangeList( aRefs, *maEnhancedProtection.maRangeList, false);
    sal_uInt16 nCref = ulimit_cast<sal_uInt16>(aRefs.size());
    rStrm << nCref;                         // cref
    rStrm.WriteZeroBytesToRecord(6);        // cbFeatData if EXC_ISFFEC2 (4 bytes) and reserved3 (2 bytes)
    aRefs.Write( rStrm, true, nCref);       // refs

    // FeatProtection structure
    rStrm << maEnhancedProtection.mnAreserved;              // 1 bit A and 31 bits reserved
    rStrm << maEnhancedProtection.mnPasswordVerifier;       // wPassword
    rStrm << XclExpString( maEnhancedProtection.maTitle);   // stTitle
    bool bSDContainer = ((maEnhancedProtection.mnAreserved & 0x00000001) == 0x00000001);
    sal_uInt32 nCbSD = maEnhancedProtection.maSecurityDescriptor.size();
    SAL_WARN_IF( bSDContainer && nCbSD < 20, "sc.filter",
            "XclExpSheetEnhancedProtection A flag indicates container but cbSD < 20");
    SAL_WARN_IF( !bSDContainer && nCbSD > 0, "sc.filter",
            "XclExpSheetEnhancedProtection A flag indicates no container but cbSD > 0");
    if (bSDContainer)
    {
        rStrm << nCbSD;
        rStrm.Write( &maEnhancedProtection.maSecurityDescriptor.front(), nCbSD);
    }
}

void XclCalccount::SaveCont( XclExpStream& rStrm )
{
    rStrm << nCount;
}

XclCalccount::XclCalccount( const ScDocument& rDoc )
{
    nCount = rDoc.GetDocOptions().GetIterCount();
}

sal_uInt16 XclCalccount::GetNum() const
{
    return 0x000C;
}

std::size_t XclCalccount::GetLen() const
{
    return 2;
}

void XclCalccount::SaveXml( XclExpXmlStream& rStrm )
{
    rStrm.WriteAttributes(
            XML_iterateCount, OString::number( nCount ).getStr(),
            FSEND );
}

void XclIteration::SaveCont( XclExpStream& rStrm )
{
    rStrm << nIter;
}

XclIteration::XclIteration( const ScDocument& rDoc )
{
    nIter = rDoc.GetDocOptions().IsIter()? 1 : 0;
}

sal_uInt16 XclIteration::GetNum() const
{
    return 0x0011;
}

std::size_t XclIteration::GetLen() const
{
    return 2;
}

void XclIteration::SaveXml( XclExpXmlStream& rStrm )
{
    rStrm.WriteAttributes(
            XML_iterate, XclXmlUtils::ToPsz( nIter == 1 ),
            FSEND );
}

void XclDelta::SaveCont( XclExpStream& rStrm )
{
    rStrm << fDelta;
}

XclDelta::XclDelta( const ScDocument& rDoc )
{
    fDelta = rDoc.GetDocOptions().GetIterEps();
}

sal_uInt16 XclDelta::GetNum() const
{
    return 0x0010;
}

std::size_t XclDelta::GetLen() const
{
    return 8;
}

void XclDelta::SaveXml( XclExpXmlStream& rStrm )
{
    rStrm.WriteAttributes(
            XML_iterateDelta, OString::number( fDelta ).getStr(),
            FSEND );
}

XclExpFileEncryption::XclExpFileEncryption( const XclExpRoot& rRoot ) :
    XclExpRecord(0x002F, 54),
    mrRoot(rRoot)
{
}

XclExpFileEncryption::~XclExpFileEncryption()
{
}

void XclExpFileEncryption::WriteBody( XclExpStream& rStrm )
{
    // 0x0000 - neither standard nor strong encryption
    // 0x0001 - standard or strong encryption
    rStrm << static_cast<sal_uInt16>(0x0001);

    // 0x0000 - non standard encryption
    // 0x0001 - standard encryption
    sal_uInt16 nStdEnc = 0x0001;
    rStrm << nStdEnc << nStdEnc;

    sal_uInt8 pnDocId[16];
    sal_uInt8 pnSalt[16];
    sal_uInt8 pnSaltHash[16];
    XclExpEncrypterRef xEnc( new XclExpBiff8Encrypter(mrRoot) );
    xEnc->GetDocId(pnDocId);
    xEnc->GetSalt(pnSalt);
    xEnc->GetSaltDigest(pnSaltHash);

    rStrm.Write(pnDocId, 16);
    rStrm.Write(pnSalt, 16);
    rStrm.Write(pnSaltHash, 16);

    rStrm.SetEncrypter(xEnc);
}

XclExpInterfaceHdr::XclExpInterfaceHdr( sal_uInt16 nCodePage ) :
    XclExpUInt16Record( EXC_ID_INTERFACEHDR, nCodePage )
{
}

void XclExpInterfaceHdr::WriteBody( XclExpStream& rStrm )
{
    rStrm.DisableEncryption();
    rStrm << GetValue();
}

XclExpInterfaceEnd::XclExpInterfaceEnd() :
    XclExpRecord(0x00E2, 0) {}

XclExpInterfaceEnd::~XclExpInterfaceEnd() {}

void XclExpInterfaceEnd::WriteBody( XclExpStream& rStrm )
{
    // Don't forget to re-enable encryption.
    rStrm.EnableEncryption();
}

XclExpWriteAccess::XclExpWriteAccess() :
    XclExpRecord(0x005C, 112)
{
}

XclExpWriteAccess::~XclExpWriteAccess()
{
}

void XclExpWriteAccess::WriteBody( XclExpStream& rStrm )
{
    static const sal_uInt8 aData[] = {
        0x04, 0x00, 0x00,  'C',  'a',  'l',  'c', 0x20,
        0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
        0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
        0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
        0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
        0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
        0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
        0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
        0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
        0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
        0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
        0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
        0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
        0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20 };

    std::size_t nDataSize = sizeof(aData);
    for (std::size_t i = 0; i < nDataSize; ++i)
        rStrm << aData[i];
}

XclExpFileSharing::XclExpFileSharing( const XclExpRoot& rRoot, sal_uInt16 nPasswordHash, bool bRecommendReadOnly ) :
    XclExpRecord( EXC_ID_FILESHARING ),
    mnPasswordHash( nPasswordHash ),
    mbRecommendReadOnly( bRecommendReadOnly )
{
    if( rRoot.GetBiff() <= EXC_BIFF5 )
        maUserName.AssignByte( rRoot.GetUserName(), rRoot.GetTextEncoding(), EXC_STR_8BITLENGTH );
    else
        maUserName.Assign( rRoot.GetUserName() );
}

void XclExpFileSharing::Save( XclExpStream& rStrm )
{
    if( (mnPasswordHash != 0) || mbRecommendReadOnly )
        XclExpRecord::Save( rStrm );
}

void XclExpFileSharing::WriteBody( XclExpStream& rStrm )
{
    rStrm << sal_uInt16( mbRecommendReadOnly ? 1 : 0 ) << mnPasswordHash << maUserName;
}

XclExpProt4Rev::XclExpProt4Rev() :
    XclExpRecord(0x01AF, 2)
{
}

XclExpProt4Rev::~XclExpProt4Rev()
{
}

void XclExpProt4Rev::WriteBody( XclExpStream& rStrm )
{
    rStrm << static_cast<sal_uInt16>(0x0000);
}

XclExpProt4RevPass::XclExpProt4RevPass() :
    XclExpRecord(0x01BC, 2)
{
}

XclExpProt4RevPass::~XclExpProt4RevPass()
{
}

void XclExpProt4RevPass::WriteBody( XclExpStream& rStrm )
{
    rStrm << static_cast<sal_uInt16>(0x0000);
}

static const sal_uInt8 nDataRecalcId[] = {
    0xC1, 0x01, 0x00, 0x00, 0x54, 0x8D, 0x01, 0x00
};

XclExpRecalcId::XclExpRecalcId() :
    XclExpDummyRecord(0x01C1, nDataRecalcId, sizeof(nDataRecalcId))
{
}

static const sal_uInt8 nDataBookExt[] = {
    0x63, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x15, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x02
};

XclExpBookExt::XclExpBookExt() :
    XclExpDummyRecord(0x0863, nDataBookExt, sizeof(nDataBookExt))
{
}

XclRefmode::XclRefmode( const ScDocument& rDoc ) :
    XclExpBoolRecord( 0x000F, rDoc.GetAddressConvention() != formula::FormulaGrammar::CONV_XL_R1C1 )
{
}

void XclRefmode::SaveXml( XclExpXmlStream& rStrm )
{
    rStrm.WriteAttributes(
            XML_refMode, GetBool() ? "A1" : "R1C1",
            FSEND );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
