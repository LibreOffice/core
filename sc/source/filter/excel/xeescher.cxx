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

#include <xeescher.hxx>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/form/FormComponentType.hpp>
#include <com/sun/star/awt/VisualEffect.hpp>
#include <com/sun/star/awt/ScrollBarOrientation.hpp>
#include <com/sun/star/awt/XControlModel.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/form/binding/XBindableValue.hpp>
#include <com/sun/star/form/binding/XListEntrySink.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/chart/XChartDocument.hpp>

#include <set>
#include <vcl/BitmapReadAccess.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdocapt.hxx>
#include <editeng/outlobj.hxx>
#include <unotools/tempfile.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <svtools/embedhlp.hxx>

#include <unonames.hxx>
#include <convuno.hxx>
#include <postit.hxx>

#include <fapihelper.hxx>
#include <xcl97esc.hxx>
#include <xechart.hxx>
#include <xeformula.hxx>
#include <xehelper.hxx>
#include <xelink.hxx>
#include <xename.hxx>
#include <xestyle.hxx>
#include <xllink.hxx>
#include <xltools.hxx>
#include <userdat.hxx>
#include <drwlayer.hxx>
#include <svl/itemset.hxx>
#include <svx/unoapi.hxx>
#include <svx/sdtaitm.hxx>
#include <document.hxx>
#include <svx/xfillit0.hxx>
#include <svx/xflclit.hxx>

#include <comphelper/sequence.hxx>
#include <oox/token/tokens.hxx>
#include <oox/token/relationship.hxx>
#include <oox/export/drawingml.hxx>
#include <oox/export/chartexport.hxx>
#include <oox/export/utils.hxx>
#include <oox/token/namespaces.hxx>
#include <memory>

using namespace com::sun::star;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::lang::XServiceInfo;
using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::drawing::XShape;
using ::com::sun::star::drawing::XShapes;
using ::com::sun::star::frame::XModel;
using ::com::sun::star::awt::XControlModel;
using ::com::sun::star::form::binding::XBindableValue;
using ::com::sun::star::form::binding::XListEntrySink;
using ::com::sun::star::script::ScriptEventDescriptor;
using ::com::sun::star::table::CellAddress;
using ::com::sun::star::table::CellRangeAddress;
using ::oox::drawingml::DrawingML;
using ::oox::drawingml::ChartExport;
using namespace oox;

namespace
{

const char *ToHorizAlign( SdrTextHorzAdjust eAdjust )
{
    switch( eAdjust )
    {
        case SDRTEXTHORZADJUST_CENTER:
            return "center";
        case SDRTEXTHORZADJUST_RIGHT:
            return "right";
        case SDRTEXTHORZADJUST_BLOCK:
            return "justify";
        case SDRTEXTHORZADJUST_LEFT:
        default:
            return "left";
    }
}

const char *ToVertAlign( SdrTextVertAdjust eAdjust )
{
    switch( eAdjust )
    {
        case SDRTEXTVERTADJUST_CENTER:
            return "center";
        case SDRTEXTVERTADJUST_BOTTOM:
            return "bottom";
        case SDRTEXTVERTADJUST_BLOCK:
            return "justify";
        case SDRTEXTVERTADJUST_TOP:
        default:
            return "top";
    }
}

void lcl_WriteAnchorVertex( sax_fastparser::FSHelperPtr const & rComments, const tools::Rectangle &aRect )
{
    rComments->startElement(FSNS(XML_xdr, XML_col));
    rComments->writeEscaped( OUString::number( aRect.Left() ) );
    rComments->endElement( FSNS( XML_xdr, XML_col ) );
    rComments->startElement(FSNS(XML_xdr, XML_colOff));
    rComments->writeEscaped( OUString::number( aRect.Top() ) );
    rComments->endElement( FSNS( XML_xdr, XML_colOff ) );
    rComments->startElement(FSNS(XML_xdr, XML_row));
    rComments->writeEscaped( OUString::number( aRect.Right() ) );
    rComments->endElement( FSNS( XML_xdr, XML_row ) );
    rComments->startElement(FSNS(XML_xdr, XML_rowOff));
    rComments->writeEscaped( OUString::number( aRect.Bottom() ) );
    rComments->endElement( FSNS( XML_xdr, XML_rowOff ) );
}

tools::Long lcl_hmm2output(tools::Long value, bool bInEMU)
{
    return o3tl::convert(value, o3tl::Length::mm100, bInEMU ? o3tl::Length::emu : o3tl::Length::px);
}

void lcl_GetFromTo( const XclExpRoot& rRoot, const tools::Rectangle &aRect, sal_Int32 nTab, tools::Rectangle &aFrom, tools::Rectangle &aTo, bool bInEMU = false )
{
    sal_Int32 nCol = 0, nRow = 0;
    sal_Int32 nColOff = 0, nRowOff= 0;

    const bool bRTL = rRoot.GetDoc().IsNegativePage( nTab );
    if (!bRTL)
    {
        while(true)
        {
            tools::Rectangle r = rRoot.GetDoc().GetMMRect( nCol,nRow,nCol,nRow,nTab );
            if( r.Left() <= aRect.Left() )
            {
                nCol++;
                nColOff = aRect.Left() - r.Left();
            }
            if( r.Top() <= aRect.Top() )
            {
                nRow++;
                nRowOff = aRect.Top() - r.Top();
            }
            if( r.Left() > aRect.Left() && r.Top() > aRect.Top() )
            {
                aFrom = tools::Rectangle( nCol-1, lcl_hmm2output( nColOff, bInEMU ),
                                   nRow-1, lcl_hmm2output( nRowOff, bInEMU ) );
                break;
            }
        }
    }
    else
    {
        while(true)
        {
            tools::Rectangle r = rRoot.GetDoc().GetMMRect( nCol,nRow,nCol,nRow,nTab );
            if( r.Left() >= aRect.Left() )
            {
                nCol++;
                nColOff = r.Left() - aRect.Left();
            }
            if( r.Top() <= aRect.Top() )
            {
                nRow++;
                nRowOff = aRect.Top() - r.Top();
            }
            if( r.Left() < aRect.Left() && r.Top() > aRect.Top() )
            {
                aFrom = tools::Rectangle( nCol-1, lcl_hmm2output( nColOff, bInEMU ),
                                   nRow-1, lcl_hmm2output( nRowOff, bInEMU ) );
                break;
            }
        }
    }
    if (!bRTL)
    {
        while(true)
        {
            tools::Rectangle r = rRoot.GetDoc().GetMMRect( nCol,nRow,nCol,nRow,nTab );
            if( r.Right() < aRect.Right() )
                nCol++;
            if( r.Bottom() < aRect.Bottom() )
                nRow++;
            if( r.Right() >= aRect.Right() && r.Bottom() >= aRect.Bottom() )
            {
                aTo = tools::Rectangle( nCol, lcl_hmm2output( aRect.Right() - r.Left(), bInEMU ),
                                 nRow, lcl_hmm2output( aRect.Bottom() - r.Top(), bInEMU ));
                break;
            }
        }
    }
    else
    {
        while(true)
        {
            tools::Rectangle r = rRoot.GetDoc().GetMMRect( nCol,nRow,nCol,nRow,nTab );
            if( r.Right() >= aRect.Right() )
                nCol++;
            if( r.Bottom() < aRect.Bottom() )
                nRow++;
            if( r.Right() < aRect.Right() && r.Bottom() >= aRect.Bottom() )
            {
                aTo = tools::Rectangle( nCol, lcl_hmm2output( r.Left() - aRect.Right(), bInEMU ),
                                 nRow, lcl_hmm2output( aRect.Bottom() - r.Top(), bInEMU ));
                break;
            }
        }
    }
}

} // namespace

// Escher client anchor =======================================================

XclExpDffAnchorBase::XclExpDffAnchorBase( const XclExpRoot& rRoot, sal_uInt16 nFlags ) :
    XclExpRoot( rRoot ),
    mnFlags( nFlags )
{
}

void XclExpDffAnchorBase::SetFlags( const SdrObject& rSdrObj )
{
    ImplSetFlags( rSdrObj );
}

void XclExpDffAnchorBase::SetSdrObject( const SdrObject& rSdrObj )
{
    ImplSetFlags( rSdrObj );
    ImplCalcAnchorRect( rSdrObj.GetCurrentBoundRect(), MapUnit::Map100thMM );
}

void XclExpDffAnchorBase::WriteDffData( EscherEx& rEscherEx ) const
{
    rEscherEx.AddAtom( 18, ESCHER_ClientAnchor );
    rEscherEx.GetStream().WriteUInt16( mnFlags );
    WriteXclObjAnchor( rEscherEx.GetStream(), maAnchor );
}

void XclExpDffAnchorBase::WriteData( EscherEx& rEscherEx, const tools::Rectangle& rRect )
{
    // the passed rectangle is in twips
    ImplCalcAnchorRect( rRect, MapUnit::MapTwip );
    WriteDffData( rEscherEx );
}

void XclExpDffAnchorBase::ImplSetFlags( const SdrObject& )
{
    OSL_FAIL( "XclExpDffAnchorBase::ImplSetFlags - not implemented" );
}

void XclExpDffAnchorBase::ImplCalcAnchorRect( const tools::Rectangle&, MapUnit )
{
    OSL_FAIL( "XclExpDffAnchorBase::ImplCalcAnchorRect - not implemented" );
}

XclExpDffSheetAnchor::XclExpDffSheetAnchor( const XclExpRoot& rRoot ) :
    XclExpDffAnchorBase( rRoot ),
    mnScTab( rRoot.GetCurrScTab() )
{
}

void XclExpDffSheetAnchor::ImplSetFlags( const SdrObject& rSdrObj )
{
    // set flags for cell/page anchoring
    if ( ScDrawLayer::GetAnchorType( rSdrObj ) == SCA_CELL )
        mnFlags = 0;
    else
        mnFlags = EXC_ESC_ANCHOR_LOCKED;
}

void XclExpDffSheetAnchor::ImplCalcAnchorRect( const tools::Rectangle& rRect, MapUnit eMapUnit )
{
    maAnchor.SetRect( GetRoot(), mnScTab, rRect, eMapUnit );
}

XclExpDffEmbeddedAnchor::XclExpDffEmbeddedAnchor( const XclExpRoot& rRoot,
        const Size& rPageSize, sal_Int32 nScaleX, sal_Int32 nScaleY ) :
    XclExpDffAnchorBase( rRoot ),
    maPageSize( rPageSize ),
    mnScaleX( nScaleX ),
    mnScaleY( nScaleY )
{
}

void XclExpDffEmbeddedAnchor::ImplSetFlags( const SdrObject& /*rSdrObj*/ )
{
    // TODO (unsupported feature): fixed size
}

void XclExpDffEmbeddedAnchor::ImplCalcAnchorRect( const tools::Rectangle& rRect, MapUnit eMapUnit )
{
    maAnchor.SetRect( maPageSize, mnScaleX, mnScaleY, rRect, eMapUnit );
}

XclExpDffNoteAnchor::XclExpDffNoteAnchor( const XclExpRoot& rRoot, const tools::Rectangle& rRect ) :
    XclExpDffAnchorBase( rRoot, EXC_ESC_ANCHOR_SIZELOCKED )
{
    maAnchor.SetRect( rRoot, rRoot.GetCurrScTab(), rRect, MapUnit::Map100thMM );
}

XclExpDffDropDownAnchor::XclExpDffDropDownAnchor( const XclExpRoot& rRoot, const ScAddress& rScPos ) :
    XclExpDffAnchorBase( rRoot, EXC_ESC_ANCHOR_POSLOCKED )
{
    GetAddressConverter().ConvertAddress( maAnchor.maFirst, rScPos, true );
    maAnchor.maLast.mnCol = maAnchor.maFirst.mnCol + 1;
    maAnchor.maLast.mnRow = maAnchor.maFirst.mnRow + 1;
    maAnchor.mnLX = maAnchor.mnTY = maAnchor.mnRX = maAnchor.mnBY = 0;
}

// MSODRAWING* records ========================================================

XclExpMsoDrawingBase::XclExpMsoDrawingBase( XclEscherEx& rEscherEx, sal_uInt16 nRecId ) :
    XclExpRecord( nRecId ),
    mrEscherEx( rEscherEx ),
    mnFragmentKey( rEscherEx.InitNextDffFragment() )
{
}

void XclExpMsoDrawingBase::WriteBody( XclExpStream& rStrm )
{
    OSL_ENSURE( mrEscherEx.GetStreamPos() == mrEscherEx.GetDffFragmentPos( mnFragmentKey ),
        "XclExpMsoDrawingBase::WriteBody - DFF stream position mismatch" );
    rStrm.CopyFromStream( mrEscherEx.GetStream(), mrEscherEx.GetDffFragmentSize( mnFragmentKey ) );
}

XclExpMsoDrawingGroup::XclExpMsoDrawingGroup( XclEscherEx& rEscherEx ) :
    XclExpMsoDrawingBase( rEscherEx, EXC_ID_MSODRAWINGGROUP )
{
    SvStream& rDffStrm = mrEscherEx.GetStream();

    // write the DGGCONTAINER with some default settings
    mrEscherEx.OpenContainer( ESCHER_DggContainer );

    // TODO: stuff the OPT atom with our own document defaults?
    static const sal_uInt8 spnDffOpt[] = {
        0xBF, 0x00, 0x08, 0x00, 0x08, 0x00, 0x81, 0x01,
        0x09, 0x00, 0x00, 0x08, 0xC0, 0x01, 0x40, 0x00,
        0x00, 0x08
    };
    mrEscherEx.AddAtom( sizeof( spnDffOpt ), ESCHER_OPT, 3, 3 );
    rDffStrm.WriteBytes(spnDffOpt, sizeof(spnDffOpt));

    // SPLITMENUCOLORS contains colors in toolbar
    static const sal_uInt8 spnDffSplitMenuColors[] = {
        0x0D, 0x00, 0x00, 0x08, 0x0C, 0x00, 0x00, 0x08,
        0x17, 0x00, 0x00, 0x08, 0xF7, 0x00, 0x00, 0x10
    };
    mrEscherEx.AddAtom( sizeof( spnDffSplitMenuColors ), ESCHER_SplitMenuColors, 0, 4 );
    rDffStrm.WriteBytes(spnDffSplitMenuColors, sizeof(spnDffSplitMenuColors));

    // close the DGGCONTAINER
    mrEscherEx.CloseContainer();
    mrEscherEx.UpdateDffFragmentEnd();
}

XclExpMsoDrawing::XclExpMsoDrawing( XclEscherEx& rEscherEx ) :
    XclExpMsoDrawingBase( rEscherEx, EXC_ID_MSODRAWING )
{
}

XclExpImgData::XclExpImgData( const Graphic& rGraphic, sal_uInt16 nRecId ) :
    maGraphic( rGraphic ),
    mnRecId( nRecId )
{
}

void XclExpImgData::Save( XclExpStream& rStrm )
{
    Bitmap aBmp = maGraphic.GetBitmapEx().GetBitmap();
    if( aBmp.GetBitCount() != 24 )
        aBmp.Convert( BmpConversion::N24Bit );

    Bitmap::ScopedReadAccess pAccess(aBmp);
    if( !pAccess )
        return;

    sal_Int32 nWidth = ::std::min< sal_Int32 >( pAccess->Width(), 0xFFFF );
    sal_Int32 nHeight = ::std::min< sal_Int32 >( pAccess->Height(), 0xFFFF );
    if( (nWidth <= 0) || (nHeight <= 0) )
        return;

    sal_uInt8 nPadding = static_cast< sal_uInt8 >( nWidth & 0x03 );
    sal_uInt32 nTmpSize = static_cast< sal_uInt32 >( (nWidth * 3 + nPadding) * nHeight + 12 );

    rStrm.StartRecord( mnRecId, nTmpSize + 4 );

    rStrm   << EXC_IMGDATA_BMP                      // BMP format
            << EXC_IMGDATA_WIN                      // Windows
            << nTmpSize                             // size after _this_ field
            << sal_uInt32( 12 )                     // BITMAPCOREHEADER size
            << static_cast< sal_uInt16 >( nWidth )  // width
            << static_cast< sal_uInt16 >( nHeight ) // height
            << sal_uInt16( 1 )                      // planes
            << sal_uInt16( 24 );                    // bits per pixel

    for( sal_Int32 nY = nHeight - 1; nY >= 0; --nY )
    {
        Scanline pScanline = pAccess->GetScanline( nY );
        for( sal_Int32 nX = 0; nX < nWidth; ++nX )
        {
            const BitmapColor& rBmpColor = pAccess->GetPixelFromData( pScanline, nX );
            rStrm << rBmpColor.GetBlue() << rBmpColor.GetGreen() << rBmpColor.GetRed();
        }
        rStrm.WriteZeroBytes( nPadding );
    }

    rStrm.EndRecord();
}

void XclExpImgData::SaveXml( XclExpXmlStream& rStrm )
{
    sax_fastparser::FSHelperPtr pWorksheet = rStrm.GetCurrentStream();

    DrawingML aDML(pWorksheet, &rStrm, drawingml::DOCUMENT_XLSX);
    OUString rId = aDML.WriteImage( maGraphic );
    pWorksheet->singleElement(XML_picture, FSNS(XML_r, XML_id), rId);
}

XclExpControlHelper::XclExpControlHelper( const XclExpRoot& rRoot ) :
    XclExpRoot( rRoot ),
    mnEntryCount( 0 )
{
}

XclExpControlHelper::~XclExpControlHelper()
{
}

void XclExpControlHelper::ConvertSheetLinks( Reference< XShape > const & xShape )
{
    mxCellLink.reset();
    mxCellLinkAddress.SetInvalid();
    mxSrcRange.reset();
    mnEntryCount = 0;

    // get control model
    Reference< XControlModel > xCtrlModel = XclControlHelper::GetControlModel( xShape );
    if( !xCtrlModel.is() )
        return;

    // *** cell link *** ------------------------------------------------------

    Reference< XBindableValue > xBindable( xCtrlModel, UNO_QUERY );
    if( xBindable.is() )
    {
        Reference< XServiceInfo > xServInfo( xBindable->getValueBinding(), UNO_QUERY );
        if( xServInfo.is() && xServInfo->supportsService( SC_SERVICENAME_VALBIND ) )
        {
            ScfPropertySet aBindProp( xServInfo );
            CellAddress aApiAddress;
            if( aBindProp.GetProperty( aApiAddress, SC_UNONAME_BOUNDCELL ) )
            {
                ScUnoConversion::FillScAddress( mxCellLinkAddress, aApiAddress );
                if( GetTabInfo().IsExportTab( mxCellLinkAddress.Tab() ) )
                    mxCellLink = GetFormulaCompiler().CreateFormula( EXC_FMLATYPE_CONTROL, mxCellLinkAddress );
            }
        }
    }

    // *** source range *** ---------------------------------------------------

    Reference< XListEntrySink > xEntrySink( xCtrlModel, UNO_QUERY );
    if( !xEntrySink.is() )
        return;

    Reference< XServiceInfo > xServInfo( xEntrySink->getListEntrySource(), UNO_QUERY );
    if( !(xServInfo.is() && xServInfo->supportsService( SC_SERVICENAME_LISTSOURCE )) )
        return;

    ScfPropertySet aSinkProp( xServInfo );
    CellRangeAddress aApiRange;
    if( aSinkProp.GetProperty( aApiRange, SC_UNONAME_CELLRANGE ) )
    {
        ScRange aSrcRange;
        ScUnoConversion::FillScRange( aSrcRange, aApiRange );
        if( (aSrcRange.aStart.Tab() == aSrcRange.aEnd.Tab()) && GetTabInfo().IsExportTab( aSrcRange.aStart.Tab() ) )
            mxSrcRange = GetFormulaCompiler().CreateFormula( EXC_FMLATYPE_CONTROL, aSrcRange );
        mnEntryCount = static_cast< sal_uInt16 >( aSrcRange.aEnd.Col() - aSrcRange.aStart.Col() + 1 );
    }
}

void XclExpControlHelper::WriteFormula( XclExpStream& rStrm, const XclTokenArray& rTokArr )
{
    sal_uInt16 nFmlaSize = rTokArr.GetSize();
    rStrm << nFmlaSize << sal_uInt32( 0 );
    rTokArr.WriteArray( rStrm );
    if( nFmlaSize & 1 )             // pad to 16-bit
        rStrm << sal_uInt8( 0 );
}

void XclExpControlHelper::WriteFormulaSubRec( XclExpStream& rStrm, sal_uInt16 nSubRecId, const XclTokenArray& rTokArr )
{
    rStrm.StartRecord( nSubRecId, (rTokArr.GetSize() + 5) & ~1 );
    WriteFormula( rStrm, rTokArr );
    rStrm.EndRecord();
}

//delete for exporting OCX
//#if EXC_EXP_OCX_CTRL

XclExpOcxControlObj::XclExpOcxControlObj( XclExpObjectManager& rObjMgr, Reference< XShape > const & xShape,
        const tools::Rectangle* pChildAnchor, const OUString& rClassName, sal_uInt32 nStrmStart, sal_uInt32 nStrmSize ) :
    XclObj( rObjMgr, EXC_OBJTYPE_PICTURE, true ),
    XclExpControlHelper( rObjMgr.GetRoot() ),
    maClassName( rClassName ),
    mnStrmStart( nStrmStart ),
    mnStrmSize( nStrmSize )
{
    ScfPropertySet aCtrlProp( XclControlHelper::GetControlModel( xShape ) );

    // OBJ record flags
    SetLocked( true );
    SetPrintable( aCtrlProp.GetBoolProperty( "Printable" ) );
    SetAutoFill( false );
    SetAutoLine( false );

    // fill DFF property set
    mrEscherEx.OpenContainer( ESCHER_SpContainer );
    mrEscherEx.AddShape( ESCHER_ShpInst_HostControl,
                         ShapeFlag::HaveShapeProperty | ShapeFlag::HaveAnchor | ShapeFlag::OLEShape );
    tools::Rectangle aDummyRect;
    EscherPropertyContainer aPropOpt( mrEscherEx.GetGraphicProvider(), mrEscherEx.QueryPictureStream(), aDummyRect );
    aPropOpt.AddOpt( ESCHER_Prop_FitTextToShape,    0x00080008 );   // bool field
    aPropOpt.AddOpt( ESCHER_Prop_lineColor,         0x08000040 );
    aPropOpt.AddOpt( ESCHER_Prop_fNoLineDrawDash,   0x00080000 );   // bool field

    // #i51348# name of the control, may overwrite shape name
    OUString aCtrlName;
    if( aCtrlProp.GetProperty( aCtrlName, "Name" ) && !aCtrlName.isEmpty() )
        aPropOpt.AddOpt( ESCHER_Prop_wzName, aCtrlName );

    // meta file
    //TODO - needs check
    Reference< XPropertySet > xShapePS( xShape, UNO_QUERY );
    if( xShapePS.is() && aPropOpt.CreateGraphicProperties( xShapePS, "MetaFile", false ) )
    {
        sal_uInt32 nBlipId;
        if( aPropOpt.GetOpt( ESCHER_Prop_pib, nBlipId ) )
            aPropOpt.AddOpt( ESCHER_Prop_pictureId, nBlipId );
    }

    // write DFF property set to stream
    aPropOpt.Commit( mrEscherEx.GetStream() );

    // anchor
    ImplWriteAnchor( SdrObject::getSdrObjectFromXShape( xShape ), pChildAnchor );

    mrEscherEx.AddAtom( 0, ESCHER_ClientData );                       // OBJ record
    mrEscherEx.CloseContainer();  // ESCHER_SpContainer
    mrEscherEx.UpdateDffFragmentEnd();

    // spreadsheet links
    ConvertSheetLinks( xShape );
}

void XclExpOcxControlObj::WriteSubRecs( XclExpStream& rStrm )
{
    // OBJCF - clipboard format
    rStrm.StartRecord( EXC_ID_OBJCF, 2 );
    rStrm << sal_uInt16( 2 );
    rStrm.EndRecord();

    // OBJFLAGS
    rStrm.StartRecord( EXC_ID_OBJFLAGS, 2 );
    rStrm << sal_uInt16( 0x0031 );
    rStrm.EndRecord();

    // OBJPICTFMLA
    XclExpString aClass( maClassName );
    sal_uInt16 nClassNameSize = static_cast< sal_uInt16 >( aClass.GetSize() );
    sal_uInt16 nClassNamePad = nClassNameSize & 1;
    sal_uInt16 nFirstPartSize = 12 + nClassNameSize + nClassNamePad;

    const XclTokenArray* pCellLink = GetCellLinkTokArr();
    sal_uInt16 nCellLinkSize = pCellLink ? ((pCellLink->GetSize() + 7) & 0xFFFE) : 0;

    const XclTokenArray* pSrcRange = GetSourceRangeTokArr();
    sal_uInt16 nSrcRangeSize = pSrcRange ? ((pSrcRange->GetSize() + 7) & 0xFFFE) : 0;

    sal_uInt16 nPictFmlaSize = nFirstPartSize + nCellLinkSize + nSrcRangeSize + 18;
    rStrm.StartRecord( EXC_ID_OBJPICTFMLA, nPictFmlaSize );

    rStrm   << nFirstPartSize                           // size of first part
            << sal_uInt16( 5 )                          // formula size
            << sal_uInt32( 0 )                          // unknown ID
            << sal_uInt8( 0x02 ) << sal_uInt32( 0 )     // tTbl token with unknown ID
            << sal_uInt8( 3 )                           // pad to word
            << aClass;                                  // "Forms.***.1"
    rStrm.WriteZeroBytes( nClassNamePad );              // pad to word
    rStrm   << mnStrmStart                              // start in 'Ctls' stream
            << mnStrmSize                               // size in 'Ctls' stream
            << sal_uInt32( 0 );                         // class ID size
    // cell link
    rStrm << nCellLinkSize;
    if( pCellLink )
        WriteFormula( rStrm, *pCellLink );
    // list source range
    rStrm << nSrcRangeSize;
    if( pSrcRange )
        WriteFormula( rStrm, *pSrcRange );

    rStrm.EndRecord();
}

//#else

XclExpTbxControlObj::XclExpTbxControlObj( XclExpObjectManager& rRoot, Reference< XShape > const & xShape , const tools::Rectangle* pChildAnchor ) :
    XclObj( rRoot, EXC_OBJTYPE_UNKNOWN, true ),
    XclMacroHelper( rRoot ),
    mxShape( xShape ),
    meEventType( EXC_TBX_EVENT_ACTION ),
    mnHeight( 0 ),
    mnState( 0 ),
    mnLineCount( 0 ),
    mnSelEntry( 0 ),
    mnScrollValue( 0 ),
    mnScrollMin( 0 ),
    mnScrollMax( 100 ),
    mnScrollStep( 1 ),
    mnScrollPage( 10 ),
    mbFlatButton( false ),
    mbFlatBorder( false ),
    mbMultiSel( false ),
    mbScrollHor( false ),
    mbPrint( false ),
    mbVisible( false ),
    mnShapeId( 0 )
{
    namespace FormCompType = css::form::FormComponentType;
    namespace AwtVisualEffect = css::awt::VisualEffect;
    namespace AwtScrollOrient = css::awt::ScrollBarOrientation;

    ScfPropertySet aCtrlProp( XclControlHelper::GetControlModel( xShape ) );
    if( !xShape.is() || !aCtrlProp.Is() )
        return;

    mnHeight = xShape->getSize().Height;
    if( mnHeight <= 0 )
        return;

    // control type
    sal_Int16 nClassId = 0;
    if( aCtrlProp.GetProperty( nClassId, "ClassId" ) )
    {
        switch( nClassId )
        {
            case FormCompType::COMMANDBUTTON:   mnObjType = EXC_OBJTYPE_BUTTON;       meEventType = EXC_TBX_EVENT_ACTION; break;
            case FormCompType::RADIOBUTTON:     mnObjType = EXC_OBJTYPE_OPTIONBUTTON; meEventType = EXC_TBX_EVENT_ACTION; break;
            case FormCompType::CHECKBOX:        mnObjType = EXC_OBJTYPE_CHECKBOX;     meEventType = EXC_TBX_EVENT_ACTION; break;
            case FormCompType::LISTBOX:         mnObjType = EXC_OBJTYPE_LISTBOX;      meEventType = EXC_TBX_EVENT_CHANGE; break;
            case FormCompType::COMBOBOX:        mnObjType = EXC_OBJTYPE_DROPDOWN;     meEventType = EXC_TBX_EVENT_CHANGE; break;
            case FormCompType::GROUPBOX:        mnObjType = EXC_OBJTYPE_GROUPBOX;     meEventType = EXC_TBX_EVENT_MOUSE;  break;
            case FormCompType::FIXEDTEXT:       mnObjType = EXC_OBJTYPE_LABEL;        meEventType = EXC_TBX_EVENT_MOUSE;  break;
            case FormCompType::SCROLLBAR:       mnObjType = EXC_OBJTYPE_SCROLLBAR;    meEventType = EXC_TBX_EVENT_VALUE;  break;
            case FormCompType::SPINBUTTON:      mnObjType = EXC_OBJTYPE_SPIN;         meEventType = EXC_TBX_EVENT_VALUE;  break;
        }
    }
    if( mnObjType == EXC_OBJTYPE_UNKNOWN )
        return;

    // OBJ record flags
    SetLocked( true );
    mbPrint = aCtrlProp.GetBoolProperty( "Printable" );
    SetPrintable( mbPrint );
    SetAutoFill( false );
    SetAutoLine( false );

    // fill DFF property set
    mrEscherEx.OpenContainer( ESCHER_SpContainer );
    mrEscherEx.AddShape( ESCHER_ShpInst_HostControl, ShapeFlag::HaveAnchor | ShapeFlag::HaveShapeProperty );
    EscherPropertyContainer aPropOpt;
    mbVisible = aCtrlProp.GetBoolProperty( "EnableVisible" );
    aPropOpt.AddOpt( ESCHER_Prop_fPrint, mbVisible ? 0x00080000 : 0x00080002 ); // visible flag

    aPropOpt.AddOpt( ESCHER_Prop_LockAgainstGrouping, 0x01000100 ); // bool field
    aPropOpt.AddOpt( ESCHER_Prop_lTxid, 0 );                        // Text ID
    aPropOpt.AddOpt( ESCHER_Prop_WrapText, 0x00000001 );
    aPropOpt.AddOpt( ESCHER_Prop_FitTextToShape, 0x001A0008 );      // bool field
    aPropOpt.AddOpt( ESCHER_Prop_fNoFillHitTest, 0x00100000 );      // bool field
    aPropOpt.AddOpt( ESCHER_Prop_fNoLineDrawDash, 0x00080000 );     // bool field

    // #i51348# name of the control, may overwrite shape name
    if( aCtrlProp.GetProperty( msCtrlName, "Name" ) && !msCtrlName.isEmpty() )
        aPropOpt.AddOpt( ESCHER_Prop_wzName, msCtrlName );

    //Export description as alt text
    if( SdrObject* pSdrObj = SdrObject::getSdrObjectFromXShape( xShape ) )
    {
        OUString aAltTxt;
        OUString aDescrText = pSdrObj->GetDescription();
        if(!aDescrText.isEmpty())
            aAltTxt = aDescrText.copy( 0, std::min<sal_Int32>(MSPROP_DESCRIPTION_MAX_LEN, aDescrText.getLength()) );
        aPropOpt.AddOpt( ESCHER_Prop_wzDescription, aAltTxt );
    }

    // write DFF property set to stream
    aPropOpt.Commit( mrEscherEx.GetStream() );

    // anchor
    ImplWriteAnchor( SdrObject::getSdrObjectFromXShape( xShape ), pChildAnchor );

    mrEscherEx.AddAtom( 0, ESCHER_ClientData );                       // OBJ record
    mrEscherEx.UpdateDffFragmentEnd();

    // control label
    if( aCtrlProp.GetProperty( msLabel, "Label" ) )
    {
        /*  Be sure to construct the MSODRAWING record containing the
            ClientTextbox atom after the base OBJ's MSODRAWING record data is
            completed. */
        pClientTextbox.reset( new XclExpMsoDrawing( mrEscherEx ) );
        mrEscherEx.AddAtom( 0, ESCHER_ClientTextbox );  // TXO record
        mrEscherEx.UpdateDffFragmentEnd();

        sal_uInt16 nXclFont = EXC_FONT_APP;
        if( !msLabel.isEmpty() )
        {
            XclFontData aFontData;
            GetFontPropSetHelper().ReadFontProperties( aFontData, aCtrlProp, EXC_FONTPROPSET_CONTROL );
            if( (!aFontData.maName.isEmpty() ) && (aFontData.mnHeight > 0) )
                nXclFont = GetFontBuffer().Insert( aFontData, EXC_COLOR_CTRLTEXT );
        }

        pTxo.reset( new XclTxo( msLabel, nXclFont ) );
        pTxo->SetHorAlign( (mnObjType == EXC_OBJTYPE_BUTTON) ? EXC_OBJ_HOR_CENTER : EXC_OBJ_HOR_LEFT );
        pTxo->SetVerAlign( EXC_OBJ_VER_CENTER );
    }

    mrEscherEx.CloseContainer();  // ESCHER_SpContainer

    // other properties
    aCtrlProp.GetProperty( mnLineCount, "LineCount" );

    // border style
    sal_Int16 nApiButton = AwtVisualEffect::LOOK3D;
    sal_Int16 nApiBorder = AwtVisualEffect::LOOK3D;
    switch( nClassId )
    {
        case FormCompType::LISTBOX:
        case FormCompType::COMBOBOX:
            aCtrlProp.GetProperty( nApiBorder, "Border" );
        break;
        case FormCompType::CHECKBOX:
        case FormCompType::RADIOBUTTON:
            aCtrlProp.GetProperty( nApiButton, "VisualEffect" );
            nApiBorder = AwtVisualEffect::NONE;
        break;
        // Push button cannot be set to flat in Excel
        case FormCompType::COMMANDBUTTON:
            nApiBorder = AwtVisualEffect::LOOK3D;
        break;
        // Label does not support a border in Excel
        case FormCompType::FIXEDTEXT:
            nApiBorder = AwtVisualEffect::NONE;
        break;
        /*  Scroll bar and spin button have a "Border" property, but it is
            really used for a border, and not for own 3D/flat look (#i34712#). */
        case FormCompType::SCROLLBAR:
        case FormCompType::SPINBUTTON:
            nApiButton = AwtVisualEffect::LOOK3D;
            nApiBorder = AwtVisualEffect::NONE;
        break;
        // Group box does not support flat style (#i34712#)
        case FormCompType::GROUPBOX:
            nApiBorder = AwtVisualEffect::LOOK3D;
        break;
    }
    mbFlatButton = nApiButton != AwtVisualEffect::LOOK3D;
    mbFlatBorder = nApiBorder != AwtVisualEffect::LOOK3D;

    // control state
    sal_Int16 nApiState = 0;
    if( aCtrlProp.GetProperty( nApiState, "State" ) )
    {
        switch( nApiState )
        {
            case 0: mnState = EXC_OBJ_CHECKBOX_UNCHECKED;  break;
            case 1: mnState = EXC_OBJ_CHECKBOX_CHECKED;    break;
            case 2: mnState = EXC_OBJ_CHECKBOX_TRISTATE;   break;
        }
    }

    // special control contents
    switch( nClassId )
    {
        case FormCompType::LISTBOX:
        {
            mbMultiSel = aCtrlProp.GetBoolProperty( "MultiSelection" );
            Sequence< sal_Int16 > aSelection;
            if( aCtrlProp.GetProperty( aSelection, "SelectedItems" ) )
            {
                if( aSelection.hasElements() )
                {
                    mnSelEntry = aSelection[ 0 ] + 1;
                    comphelper::sequenceToContainer(maMultiSel, aSelection);
                }
            }

            // convert listbox with dropdown button to Excel dropdown
            if( aCtrlProp.GetBoolProperty( "Dropdown" ) )
                mnObjType = EXC_OBJTYPE_DROPDOWN;
        }
        break;

        case FormCompType::COMBOBOX:
        {
            Sequence< OUString > aStringList;
            OUString aDefText;
            if( aCtrlProp.GetProperty( aStringList, "StringItemList" ) &&
                aCtrlProp.GetProperty( aDefText, "Text" ) &&
                aStringList.hasElements() && !aDefText.isEmpty() )
            {
                auto nIndex = comphelper::findValue(aStringList, aDefText);
                if( nIndex != -1 )
                    mnSelEntry = static_cast< sal_Int16 >( nIndex + 1 );  // 1-based
                if( mnSelEntry > 0 )
                    maMultiSel.resize( 1, mnSelEntry - 1 );
            }

            // convert combobox without dropdown button to Excel listbox
            if( !aCtrlProp.GetBoolProperty( "Dropdown" ) )
                mnObjType = EXC_OBJTYPE_LISTBOX;
        }
        break;

        case FormCompType::SCROLLBAR:
        {
            sal_Int32 nApiValue = 0;
            if( aCtrlProp.GetProperty( nApiValue, "ScrollValueMin" ) )
                mnScrollMin = limit_cast< sal_uInt16 >( nApiValue, EXC_OBJ_SCROLLBAR_MIN, EXC_OBJ_SCROLLBAR_MAX );
            if( aCtrlProp.GetProperty( nApiValue, "ScrollValueMax" ) )
                mnScrollMax = limit_cast< sal_uInt16 >( nApiValue, mnScrollMin, EXC_OBJ_SCROLLBAR_MAX );
            if( aCtrlProp.GetProperty( nApiValue, "ScrollValue" ) )
                mnScrollValue = limit_cast< sal_uInt16 >( nApiValue, mnScrollMin, mnScrollMax );
            if( aCtrlProp.GetProperty( nApiValue, "LineIncrement" ) )
                mnScrollStep = limit_cast< sal_uInt16 >( nApiValue, EXC_OBJ_SCROLLBAR_MIN, EXC_OBJ_SCROLLBAR_MAX );
            if( aCtrlProp.GetProperty( nApiValue, "BlockIncrement" ) )
                mnScrollPage = limit_cast< sal_uInt16 >( nApiValue, EXC_OBJ_SCROLLBAR_MIN, EXC_OBJ_SCROLLBAR_MAX );
            if( aCtrlProp.GetProperty( nApiValue, "Orientation" ) )
                mbScrollHor = nApiValue == AwtScrollOrient::HORIZONTAL;
        }
        break;

        case FormCompType::SPINBUTTON:
        {
            sal_Int32 nApiValue = 0;
            if( aCtrlProp.GetProperty( nApiValue, "SpinValueMin" ) )
                mnScrollMin = limit_cast< sal_uInt16 >( nApiValue, EXC_OBJ_SCROLLBAR_MIN, EXC_OBJ_SCROLLBAR_MAX );
            if( aCtrlProp.GetProperty( nApiValue, "SpinValueMax" ) )
                mnScrollMax = limit_cast< sal_uInt16 >( nApiValue, mnScrollMin, EXC_OBJ_SCROLLBAR_MAX );
            if( aCtrlProp.GetProperty( nApiValue, "SpinValue" ) )
                mnScrollValue = limit_cast< sal_uInt16 >( nApiValue, mnScrollMin, mnScrollMax );
            if( aCtrlProp.GetProperty( nApiValue, "SpinIncrement" ) )
                mnScrollStep = limit_cast< sal_uInt16 >( nApiValue, EXC_OBJ_SCROLLBAR_MIN, EXC_OBJ_SCROLLBAR_MAX );
            if( aCtrlProp.GetProperty( nApiValue, "Orientation" ) )
                mbScrollHor = nApiValue == AwtScrollOrient::HORIZONTAL;
        }
        break;
    }

    {
        Reference< XControlModel > xCtrlModel = XclControlHelper::GetControlModel( xShape );
        if( xCtrlModel.is() )
        {
            Reference< XBindableValue > xBindable( xCtrlModel, UNO_QUERY );
            if( xBindable.is() )
            {
                Reference< XServiceInfo > xServInfo( xBindable->getValueBinding(), UNO_QUERY );
                if( xServInfo.is() && xServInfo->supportsService( SC_SERVICENAME_VALBIND ) )
                {
                    ScfPropertySet aBindProp( xServInfo );
                    CellAddress aApiAddress;
                    if( aBindProp.GetProperty( aApiAddress, SC_UNONAME_BOUNDCELL ) )
                    {
                        ScUnoConversion::FillScAddress( mxCellLinkAddress, aApiAddress );
                        if( SdrObject* pSdrObj = SdrObject::getSdrObjectFromXShape( xShape ) )
                            lcl_GetFromTo( rRoot, pSdrObj->GetLogicRect(), mxCellLinkAddress.Tab(), maAreaFrom, maAreaTo, true );
                    }
                }
            }
        }
    }

    // spreadsheet links
    ConvertSheetLinks( xShape );
}

bool XclExpTbxControlObj::SetMacroLink( const ScriptEventDescriptor& rEvent )
{
    return XclMacroHelper::SetMacroLink( rEvent, meEventType );
}

void XclExpTbxControlObj::WriteSubRecs( XclExpStream& rStrm )
{
    switch( mnObjType )
    {
        // *** Push buttons, labels ***

        case EXC_OBJTYPE_BUTTON:
        case EXC_OBJTYPE_LABEL:
            // ftMacro - macro link
            WriteMacroSubRec( rStrm );
        break;

        // *** Check boxes, option buttons ***

        case EXC_OBJTYPE_CHECKBOX:
        case EXC_OBJTYPE_OPTIONBUTTON:
        {
            // ftCbls - box properties
            sal_uInt16 nStyle = 0;
            ::set_flag( nStyle, EXC_OBJ_CHECKBOX_FLAT, mbFlatButton );

            rStrm.StartRecord( EXC_ID_OBJCBLS, 12 );
            rStrm << mnState;
            rStrm.WriteZeroBytes( 8 );
            rStrm << nStyle;
            rStrm.EndRecord();

            // ftMacro - macro link
            WriteMacroSubRec( rStrm );
            // ftCblsFmla subrecord - cell link
            WriteCellLinkSubRec( rStrm, EXC_ID_OBJCBLSFMLA );

            // ftCblsData subrecord - box properties, again
            rStrm.StartRecord( EXC_ID_OBJCBLS, 8 );
            rStrm << mnState;
            rStrm.WriteZeroBytes( 4 );
            rStrm << nStyle;
            rStrm.EndRecord();
        }
        break;

        // *** List boxes, combo boxes ***

        case EXC_OBJTYPE_LISTBOX:
        case EXC_OBJTYPE_DROPDOWN:
        {
            sal_uInt16 nEntryCount = GetSourceEntryCount();

            // ftSbs subrecord - Scroll bars
            sal_Int32 nLineHeight = XclTools::GetHmmFromTwips( 200 );   // always 10pt
            if( mnObjType == EXC_OBJTYPE_LISTBOX )
                mnLineCount = static_cast< sal_uInt16 >( mnHeight / nLineHeight );
            mnScrollValue = 0;
            mnScrollMin = 0;
            sal_uInt16 nInvisLines = (nEntryCount >= mnLineCount) ? (nEntryCount - mnLineCount) : 0;
            mnScrollMax = limit_cast< sal_uInt16 >( nInvisLines, EXC_OBJ_SCROLLBAR_MIN, EXC_OBJ_SCROLLBAR_MAX );
            mnScrollStep = 1;
            mnScrollPage = limit_cast< sal_uInt16 >( mnLineCount, EXC_OBJ_SCROLLBAR_MIN, EXC_OBJ_SCROLLBAR_MAX );
            mbScrollHor = false;
            WriteSbs( rStrm );

            // ftMacro - macro link
            WriteMacroSubRec( rStrm );
            // ftSbsFmla subrecord - cell link
            WriteCellLinkSubRec( rStrm, EXC_ID_OBJSBSFMLA );

            // ftLbsData - source data range and box properties
            sal_uInt16 nStyle = 0;
            ::insert_value( nStyle, mbMultiSel ? EXC_OBJ_LISTBOX_MULTI : EXC_OBJ_LISTBOX_SINGLE, 4, 2 );
            ::set_flag( nStyle, EXC_OBJ_LISTBOX_FLAT, mbFlatBorder );

            rStrm.StartRecord( EXC_ID_OBJLBSDATA, 0 );

            if( const XclTokenArray* pSrcRange = GetSourceRangeTokArr() )
            {
                rStrm << static_cast< sal_uInt16 >( (pSrcRange->GetSize() + 7) & 0xFFFE );
                WriteFormula( rStrm, *pSrcRange );
            }
            else
                rStrm << sal_uInt16( 0 );

            rStrm << nEntryCount << mnSelEntry << nStyle << sal_uInt16( 0 );
            if( mnObjType == EXC_OBJTYPE_LISTBOX )
            {
                if( nEntryCount )
                {
                    ScfUInt8Vec aSelEx( nEntryCount, 0 );
                    for( const auto& rItem : maMultiSel )
                        if( rItem < nEntryCount )
                            aSelEx[ rItem ] = 1;
                    rStrm.Write( aSelEx.data(), aSelEx.size() );
                }
            }
            else if( mnObjType == EXC_OBJTYPE_DROPDOWN )
            {
                rStrm << sal_uInt16( 0 ) << mnLineCount << sal_uInt16( 0 ) << sal_uInt16( 0 );
            }

            rStrm.EndRecord();
        }
        break;

        // *** Spin buttons, scrollbars ***

        case EXC_OBJTYPE_SPIN:
        case EXC_OBJTYPE_SCROLLBAR:
        {
            // ftSbs subrecord - scroll bars
            WriteSbs( rStrm );
            // ftMacro - macro link
            WriteMacroSubRec( rStrm );
            // ftSbsFmla subrecord - cell link
            WriteCellLinkSubRec( rStrm, EXC_ID_OBJSBSFMLA );
        }
        break;

        // *** Group boxes ***

        case EXC_OBJTYPE_GROUPBOX:
        {
            // ftMacro - macro link
            WriteMacroSubRec( rStrm );

            // ftGboData subrecord - group box properties
            sal_uInt16 nStyle = 0;
            ::set_flag( nStyle, EXC_OBJ_GROUPBOX_FLAT, mbFlatBorder );

            rStrm.StartRecord( EXC_ID_OBJGBODATA, 6 );
            rStrm   << sal_uInt32( 0 )
                    << nStyle;
            rStrm.EndRecord();
        }
        break;
    }
}

void XclExpTbxControlObj::WriteCellLinkSubRec( XclExpStream& rStrm, sal_uInt16 nSubRecId )
{
    if( const XclTokenArray* pCellLink = GetCellLinkTokArr() )
        WriteFormulaSubRec( rStrm, nSubRecId, *pCellLink );
}

void XclExpTbxControlObj::WriteSbs( XclExpStream& rStrm )
{
    sal_uInt16 nOrient = 0;
    ::set_flag( nOrient, EXC_OBJ_SCROLLBAR_HOR, mbScrollHor );
    sal_uInt16 nStyle = EXC_OBJ_SCROLLBAR_DEFFLAGS;
    ::set_flag( nStyle, EXC_OBJ_SCROLLBAR_FLAT, mbFlatButton );

    rStrm.StartRecord( EXC_ID_OBJSBS, 20 );
    rStrm   << sal_uInt32( 0 )              // reserved
            << mnScrollValue                // thumb position
            << mnScrollMin                  // thumb min pos
            << mnScrollMax                  // thumb max pos
            << mnScrollStep                 // line increment
            << mnScrollPage                 // page increment
            << nOrient                      // 0 = vertical, 1 = horizontal
            << sal_uInt16( 15 )             // thumb width
            << nStyle;                      // flags/style
    rStrm.EndRecord();
}

void XclExpTbxControlObj::setShapeId(sal_Int32 aShapeId)
{
    mnShapeId = aShapeId;
}

// save into xl\drawings\drawing1.xml
void XclExpTbxControlObj::SaveXml( XclExpXmlStream& rStrm )
{
    sax_fastparser::FSHelperPtr& pDrawing = rStrm.GetCurrentStream();

    pDrawing->startElement(FSNS(XML_mc, XML_AlternateContent),
        FSNS(XML_xmlns, XML_mc), rStrm.getNamespaceURL(OOX_NS(mce)));
    pDrawing->startElement(FSNS(XML_mc, XML_Choice),
        FSNS(XML_xmlns, XML_a14), rStrm.getNamespaceURL(OOX_NS(a14)),
        XML_Requires, "a14");

    pDrawing->startElement(FSNS(XML_xdr, XML_twoCellAnchor), XML_editAs, "oneCell");
    {
        pDrawing->startElement(FSNS(XML_xdr, XML_from));
        lcl_WriteAnchorVertex(pDrawing, maAreaFrom);
        pDrawing->endElement(FSNS(XML_xdr, XML_from));
        pDrawing->startElement(FSNS(XML_xdr, XML_to));
        lcl_WriteAnchorVertex(pDrawing, maAreaTo);
        pDrawing->endElement(FSNS(XML_xdr, XML_to));

        pDrawing->startElement(FSNS(XML_xdr, XML_sp));
        {
            // xdr:nvSpPr
            pDrawing->startElement(FSNS(XML_xdr, XML_nvSpPr));
            {
                pDrawing->singleElement(FSNS(XML_xdr, XML_cNvPr),
                    XML_id, OString::number(mnShapeId).getStr(),
                    XML_name, msCtrlName, // control name
                    XML_descr, msLabel, // description as alt text
                    XML_hidden, mbVisible ? "0" : "1");
                pDrawing->singleElement(FSNS(XML_xdr, XML_cNvSpPr));
            }
            pDrawing->endElement(FSNS(XML_xdr, XML_nvSpPr));

            // xdr:spPr
            pDrawing->startElement(FSNS(XML_xdr, XML_spPr));
            {
                // a:xfrm
                pDrawing->startElement(FSNS(XML_a, XML_xfrm));
                {
                    pDrawing->singleElement(FSNS(XML_a, XML_off),
                        XML_x, "0",
                        XML_y, "0");
                    pDrawing->singleElement(FSNS(XML_a, XML_ext),
                        XML_cx, "0",
                        XML_cy, "0");
                }
                pDrawing->endElement(FSNS(XML_a, XML_xfrm));

                // a:prstGeom
                pDrawing->startElement(FSNS(XML_a, XML_prstGeom), XML_prst, "rect");
                {
                    pDrawing->singleElement(FSNS(XML_a, XML_avLst));
                }
                pDrawing->endElement(FSNS(XML_a, XML_prstGeom));
            }
            pDrawing->endElement(FSNS(XML_xdr, XML_spPr));

            // xdr:txBody
            {
                pDrawing->startElement(FSNS(XML_xdr, XML_txBody));

#define DEFLRINS 254
#define DEFTBINS 127
                sal_Int32 nLeft, nRight, nTop, nBottom;
                nLeft = nRight = DEFLRINS;
                nTop = nBottom = DEFTBINS;

                // top inset looks a bit different compared to ppt export
                // check if something related doesn't work as expected
                Reference< XPropertySet > rXPropSet(mxShape, UNO_QUERY);

                try
                {
                    css::uno::Any mAny;

                    mAny = rXPropSet->getPropertyValue("TextLeftDistance");
                    if (mAny.hasValue())
                        mAny >>= nLeft;

                    mAny = rXPropSet->getPropertyValue("TextRightDistance");
                    if (mAny.hasValue())
                        mAny >>= nRight;

                    mAny = rXPropSet->getPropertyValue("TextUpperDistance");
                    if (mAny.hasValue())
                        mAny >>= nTop;

                    mAny = rXPropSet->getPropertyValue("TextLowerDistance");
                    if (mAny.hasValue())
                        mAny >>= nBottom;
                }
                catch (...)
                {
                }

                // Specifies the inset of the bounding rectangle.
                // Insets are used just as internal margins for text boxes within shapes.
                // If this attribute is omitted, then a value of 45720 or 0.05 inches is implied.
                pDrawing->startElementNS(XML_a, XML_bodyPr,
                    XML_lIns, sax_fastparser::UseIf(OString::number(oox::drawingml::convertHmmToEmu(nLeft)), nLeft != DEFLRINS),
                    XML_rIns, sax_fastparser::UseIf(OString::number(oox::drawingml::convertHmmToEmu(nRight)), nRight != DEFLRINS),
                    XML_tIns, sax_fastparser::UseIf(OString::number(oox::drawingml::convertHmmToEmu(nTop)), nTop != DEFTBINS),
                    XML_bIns, sax_fastparser::UseIf(OString::number(oox::drawingml::convertHmmToEmu(nBottom)), nBottom != DEFTBINS),
                    XML_anchor, "ctr");

                {
                    bool bTextAutoGrowHeight = false;

                    try
                    {
                        css::uno::Any mAny;

                        mAny = rXPropSet->getPropertyValue("TextAutoGrowHeight");
                        if (mAny.hasValue())
                            mAny >>= bTextAutoGrowHeight;
                    }
                    catch (...)
                    {
                    }

                    pDrawing->singleElementNS(XML_a, (bTextAutoGrowHeight ? XML_spAutoFit : XML_noAutofit));
                }

                pDrawing->endElementNS(XML_a, XML_bodyPr);

                {
                    pDrawing->startElementNS(XML_a, XML_p);
                    pDrawing->startElementNS(XML_a, XML_r);
                    pDrawing->startElementNS(XML_a, XML_t);
                    pDrawing->write(msLabel);
                    pDrawing->endElementNS(XML_a, XML_t);
                    pDrawing->endElementNS(XML_a, XML_r);
                    pDrawing->endElementNS(XML_a, XML_p);
                }

                pDrawing->endElement(FSNS(XML_xdr, XML_txBody));
            }
        }
        pDrawing->endElement(FSNS(XML_xdr, XML_sp));
        pDrawing->singleElement(FSNS(XML_xdr, XML_clientData));
    }
    pDrawing->endElement(FSNS(XML_xdr, XML_twoCellAnchor));
    pDrawing->endElement( FSNS( XML_mc, XML_Choice ) );
    pDrawing->endElement( FSNS( XML_mc, XML_AlternateContent ) );
}

// output into ctrlProp1.xml
OUString XclExpTbxControlObj::SaveControlPropertiesXml(XclExpXmlStream& rStrm) const
{
    OUString sIdFormControlPr;

    switch (mnObjType)
    {
        case EXC_OBJTYPE_CHECKBOX:
        {
            const sal_Int32 nDrawing = DrawingML::getNewDrawingUniqueId();
            sax_fastparser::FSHelperPtr pFormControl = rStrm.CreateOutputStream(
                    XclXmlUtils::GetStreamName( "xl/", "ctrlProps/ctrlProps", nDrawing ),
                    XclXmlUtils::GetStreamName( "../", "ctrlProps/ctrlProps", nDrawing ),
                    rStrm.GetCurrentStream()->getOutputStream(),
                    "application/vnd.ms-excel.controlproperties+xml",
                    oox::getRelationship(Relationship::CTRLPROP),
                    &sIdFormControlPr );

            rStrm.PushStream( pFormControl );
            // checkbox
            // <formControlPr
            //      xmlns="http://schemas.microsoft.com/office/spreadsheetml/2009/9/main"
            //      objectType="CheckBox" checked="Checked" lockText="1" noThreeD="1"/>
            //
            pFormControl->write("<formControlPr xmlns=\"http://schemas.microsoft.com/office/spreadsheetml/2009/9/main\" objectType=\"CheckBox\"");
            if (mnState == EXC_OBJ_CHECKBOX_CHECKED)
                pFormControl->write(" checked=\"Checked\"");

            pFormControl->write(" autoLine=\"false\"");

            if (mbPrint)
                pFormControl->write(" print=\"true\"");
            else
                pFormControl->write(" print=\"false\"");

            if (mxCellLinkAddress.IsValid())
            {
                OUString aCellLink = mxCellLinkAddress.Format(ScRefFlags::ADDR_ABS,
                    &GetDoc(),
                    ScAddress::Details(::formula::FormulaGrammar::CONV_XL_A1));

                // "Sheet1!$C$5"
                pFormControl->write(" fmlaLink=\"");
                if (aCellLink.indexOf('!') < 0)
                {
                    pFormControl->write(GetTabInfo().GetScTabName(mxCellLinkAddress.Tab()));
                    pFormControl->write("!");
                }
                pFormControl->write(aCellLink);
                pFormControl->write("\"");
            }

            pFormControl->write(" lockText=\"1\" noThreeD=\"1\"/>");
            rStrm.PopStream();

            break;
        }
    }

    return sIdFormControlPr;
}

// output into sheet1.xml
void XclExpTbxControlObj::SaveSheetXml(XclExpXmlStream& rStrm, const OUString& aIdFormControlPr) const
{
    switch (mnObjType)
    {
        case EXC_OBJTYPE_CHECKBOX:
        {
            sax_fastparser::FSHelperPtr& rWorksheet = rStrm.GetCurrentStream();

            rWorksheet->startElement(FSNS(XML_mc, XML_AlternateContent),
                FSNS(XML_xmlns, XML_mc), rStrm.getNamespaceURL(OOX_NS(mce)));
            rWorksheet->startElement(FSNS(XML_mc, XML_Choice), XML_Requires, "x14");

            rWorksheet->startElement(
                XML_control,
                XML_shapeId, OString::number(mnShapeId).getStr(),
                FSNS(XML_r, XML_id), aIdFormControlPr,
                XML_name, msLabel); // text to display with checkbox button

            rWorksheet->write("<controlPr defaultSize=\"0\" locked=\"1\" autoFill=\"0\" autoLine=\"0\" autoPict=\"0\"");

            if (mbPrint)
                rWorksheet->write(" print=\"true\"");
            else
                rWorksheet->write(" print=\"false\"");

            if (!msCtrlName.isEmpty())
            {
                rWorksheet->write(" altText=\"");
                rWorksheet->write(msCtrlName); // alt text
                rWorksheet->write("\"");
            }

            rWorksheet->write(">");

            rWorksheet->startElement(XML_anchor, XML_moveWithCells, "true", XML_sizeWithCells, "false");
            rWorksheet->startElement(XML_from);
            lcl_WriteAnchorVertex(rWorksheet, maAreaFrom);
            rWorksheet->endElement(XML_from);
            rWorksheet->startElement(XML_to);
            lcl_WriteAnchorVertex(rWorksheet, maAreaTo);
            rWorksheet->endElement(XML_to);
            rWorksheet->endElement( XML_anchor );

            rWorksheet->write("</controlPr>");

            rWorksheet->endElement(XML_control);
            rWorksheet->endElement( FSNS( XML_mc, XML_Choice ) );
            rWorksheet->endElement( FSNS( XML_mc, XML_AlternateContent ) );

            break;
        }
    }
}

//#endif

XclExpChartObj::XclExpChartObj( XclExpObjectManager& rObjMgr, Reference< XShape > const & xShape, const tools::Rectangle* pChildAnchor, ScDocument* pDoc ) :
    XclObj( rObjMgr, EXC_OBJTYPE_CHART ),
    XclExpRoot( rObjMgr.GetRoot() ), mxShape( xShape ),
    mpDoc(pDoc)
{
    // create the MSODRAWING record contents for the chart object
    mrEscherEx.OpenContainer( ESCHER_SpContainer );
    mrEscherEx.AddShape( ESCHER_ShpInst_HostControl, ShapeFlag::HaveAnchor | ShapeFlag::HaveShapeProperty );
    EscherPropertyContainer aPropOpt;
    aPropOpt.AddOpt( ESCHER_Prop_LockAgainstGrouping, 0x01040104 );
    aPropOpt.AddOpt( ESCHER_Prop_FitTextToShape, 0x00080008 );
    aPropOpt.AddOpt( ESCHER_Prop_fillColor, 0x0800004E );
    aPropOpt.AddOpt( ESCHER_Prop_fillBackColor, 0x0800004D );
    aPropOpt.AddOpt( ESCHER_Prop_fNoFillHitTest, 0x00110010 );
    aPropOpt.AddOpt( ESCHER_Prop_lineColor, 0x0800004D );
    aPropOpt.AddOpt( ESCHER_Prop_fNoLineDrawDash, 0x00080008 );
    aPropOpt.AddOpt( ESCHER_Prop_fshadowObscured, 0x00020000 );
    aPropOpt.AddOpt( ESCHER_Prop_fPrint, 0x00080000 );
    aPropOpt.Commit( mrEscherEx.GetStream() );

    // anchor
    SdrObject* pSdrObj = SdrObject::getSdrObjectFromXShape( xShape );
    ImplWriteAnchor( pSdrObj, pChildAnchor );

    // client data (the following OBJ record)
    mrEscherEx.AddAtom( 0, ESCHER_ClientData );
    mrEscherEx.CloseContainer();  // ESCHER_SpContainer
    mrEscherEx.UpdateDffFragmentEnd();

    // load the chart OLE object
    if( SdrOle2Obj* pSdrOleObj = dynamic_cast< SdrOle2Obj* >( pSdrObj ) )
        svt::EmbeddedObjectRef::TryRunningState( pSdrOleObj->GetObjRef() );

    // create the chart substream object
    ScfPropertySet aShapeProp( xShape );
    Reference< XModel > xModel;
    aShapeProp.GetProperty( xModel, "Model" );
    mxChartDoc.set( xModel,UNO_QUERY );
    css::awt::Rectangle aBoundRect;
    aShapeProp.GetProperty( aBoundRect, "BoundRect" );
    tools::Rectangle aChartRect( Point( aBoundRect.X, aBoundRect.Y ), Size( aBoundRect.Width, aBoundRect.Height ) );
    mxChart = std::make_shared<XclExpChart>( GetRoot(), xModel, aChartRect );
}

XclExpChartObj::~XclExpChartObj()
{
}

void XclExpChartObj::Save( XclExpStream& rStrm )
{
    // content of OBJ record
    XclObj::Save( rStrm );
    // chart substream
    mxChart->Save( rStrm );
}

void XclExpChartObj::SaveXml( XclExpXmlStream& rStrm )
{
    sax_fastparser::FSHelperPtr pDrawing = rStrm.GetCurrentStream();

    // FIXME: two cell? it seems the two cell anchor is incorrect.
    pDrawing->startElement( FSNS( XML_xdr, XML_twoCellAnchor ), // OOXTODO: oneCellAnchor, absoluteAnchor
            XML_editAs, "oneCell" );
    Reference< XPropertySet > xPropSet( mxShape, UNO_QUERY );
    if (xPropSet.is())
    {
        XclObjAny::WriteFromTo( rStrm, mxShape, GetTab() );
        ChartExport aChartExport(XML_xdr, pDrawing, mxChartDoc, &rStrm, drawingml::DOCUMENT_XLSX);
        auto pURLTransformer = std::make_shared<ScURLTransformer>(*mpDoc);
        aChartExport.SetURLTranslator(pURLTransformer);
        static sal_Int32 nChartCount = 0;
        nChartCount++;
        sal_Int32 nID = rStrm.GetUniqueId();
        aChartExport.WriteChartObj( mxShape, nID, nChartCount );
        // TODO: get the correcto chart number
    }

    pDrawing->singleElement( FSNS( XML_xdr, XML_clientData)
            // OOXTODO: XML_fLocksWithSheet
            // OOXTODO: XML_fPrintsWithSheet
    );
    pDrawing->endElement( FSNS( XML_xdr, XML_twoCellAnchor ) );
}

const css::uno::Reference<css::chart::XChartDocument>& XclExpChartObj::GetChartDoc() const
{
    return mxChartDoc;
}

XclExpNote::XclExpNote(const XclExpRoot& rRoot, const ScAddress& rScPos,
        const ScPostIt* pScNote, std::u16string_view rAddText)
    : XclExpRecord(EXC_ID_NOTE)
    , mrRoot(rRoot)
    , maScPos(rScPos)
    , mnObjId(EXC_OBJ_INVALID_ID)
    , mbVisible(pScNote && pScNote->IsCaptionShown())
    , meTHA(SDRTEXTHORZADJUST_LEFT)
    , meTVA(SDRTEXTVERTADJUST_TOP)
    , mbAutoScale(false)
    , mbLocked(false)
    , mbAutoFill(false)
    , mbColHidden(false)
    , mbRowHidden(false)
{
    // get the main note text
    OUString aNoteText;
    if( pScNote )
    {
        aNoteText = pScNote->GetText();
        const EditTextObject *pEditObj = pScNote->GetEditTextObject();
        if( pEditObj )
            mpNoteContents = XclExpStringHelper::CreateString( rRoot, *pEditObj );
    }
    // append additional text
    aNoteText = ScGlobal::addToken( aNoteText, rAddText, '\n', 2 );

    // initialize record dependent on BIFF type
    switch( rRoot.GetBiff() )
    {
        case EXC_BIFF5:
            maNoteText = OUStringToOString(aNoteText, rRoot.GetTextEncoding());
        break;

        case EXC_BIFF8:
        {
            // TODO: additional text
            if( pScNote )
            {
                if( SdrCaptionObj* pCaption = pScNote->GetOrCreateCaption( maScPos ) )
                {
                    lcl_GetFromTo( rRoot, pCaption->GetLogicRect(), maScPos.Tab(), maCommentFrom, maCommentTo );
                    if( const OutlinerParaObject* pOPO = pCaption->GetOutlinerParaObject() )
                        mnObjId = rRoot.GetObjectManager().AddObj( std::make_unique<XclObjComment>( rRoot.GetObjectManager(), pCaption->GetLogicRect(), pOPO->GetTextObject(), pCaption, mbVisible, maScPos, maCommentFrom, maCommentTo ) );

                    SfxItemSet aItemSet = pCaption->GetMergedItemSet();
                    meTVA       = pCaption->GetTextVerticalAdjust();
                    meTHA       = pCaption->GetTextHorizontalAdjust();
                    mbAutoScale = pCaption->GetFitToSize() != drawing::TextFitToSizeType_NONE;
                    mbLocked    = pCaption->IsMoveProtect() || pCaption->IsResizeProtect();

                    // AutoFill style would change if Postit.cxx object creation values are changed
                    OUString aCol(aItemSet.Get(XATTR_FILLCOLOR).GetValue());
                    mbAutoFill  = aCol.isEmpty() && (aItemSet.Get(XATTR_FILLSTYLE).GetValue() == drawing::FillStyle_SOLID);
                    mbRowHidden = (rRoot.GetDoc().RowHidden(maScPos.Row(),maScPos.Tab()));
                    mbColHidden = (rRoot.GetDoc().ColHidden(maScPos.Col(),maScPos.Tab()));
                }
                // stAuthor (variable): An XLUnicodeString that specifies the name of the comment
                // author. String length MUST be greater than or equal to 1 and less than or equal
                // to 54.
                if( pScNote->GetAuthor().isEmpty() )
                    maAuthor = XclExpString( " " );
                else
                    maAuthor = XclExpString( pScNote->GetAuthor(), XclStrFlags::NONE, 54 );
            }

            SetRecSize( 9 + maAuthor.GetSize() );
        }
        break;

        default:    DBG_ERROR_BIFF();
    }
}

void XclExpNote::Save( XclExpStream& rStrm )
{
    switch( rStrm.GetRoot().GetBiff() )
    {
        case EXC_BIFF5:
        {
            // write the NOTE record directly, there may be the need to create more than one
            const char* pcBuffer = maNoteText.getStr();
            sal_uInt16 nCharsLeft = static_cast< sal_uInt16 >( maNoteText.getLength() );

            while( nCharsLeft )
            {
                sal_uInt16 nWriteChars = ::std::min( nCharsLeft, EXC_NOTE5_MAXLEN );

                rStrm.StartRecord( EXC_ID_NOTE, 6 + nWriteChars );
                if( pcBuffer == maNoteText.getStr() )
                {
                    // first record: row, col, length of complete text
                    rStrm   << static_cast< sal_uInt16 >( maScPos.Row() )
                            << static_cast< sal_uInt16 >( maScPos.Col() )
                            << nCharsLeft;  // still contains full length
                }
                else
                {
                    // next records: -1, 0, length of current text segment
                    rStrm   << sal_uInt16( 0xFFFF )
                            << sal_uInt16( 0 )
                            << nWriteChars;
                }
                rStrm.Write( pcBuffer, nWriteChars );
                rStrm.EndRecord();

                pcBuffer += nWriteChars;
                nCharsLeft = nCharsLeft - nWriteChars;
            }
        }
        break;

        case EXC_BIFF8:
            if( mnObjId != EXC_OBJ_INVALID_ID )
                XclExpRecord::Save( rStrm );
        break;

        default:    DBG_ERROR_BIFF();
    }
}

void XclExpNote::WriteBody( XclExpStream& rStrm )
{
    // BIFF5/BIFF7 is written separately
    OSL_ENSURE_BIFF( rStrm.GetRoot().GetBiff() == EXC_BIFF8 );

    sal_uInt16 nFlags = 0;
    ::set_flag( nFlags, EXC_NOTE_VISIBLE, mbVisible );

    rStrm   << static_cast< sal_uInt16 >( maScPos.Row() )
            << static_cast< sal_uInt16 >( maScPos.Col() )
            << nFlags
            << mnObjId
            << maAuthor
            << sal_uInt8( 0 );
}

void XclExpNote::WriteXml( sal_Int32 nAuthorId, XclExpXmlStream& rStrm )
{
    sax_fastparser::FSHelperPtr rComments = rStrm.GetCurrentStream();

    rComments->startElement( XML_comment,
            XML_ref,        XclXmlUtils::ToOString(mrRoot.GetDoc(), maScPos),
            XML_authorId,   OString::number(nAuthorId)
            // OOXTODO: XML_guid
    );
    rComments->startElement(XML_text);
    // OOXTODO: phoneticPr, rPh, r
    if( mpNoteContents )
        mpNoteContents->WriteXml( rStrm );
    rComments->endElement( XML_text );

/*
   Export of commentPr is disabled, since the current (Oct 2010)
   version of MSO 2010 doesn't yet support commentPr
 */
#if 1//def XLSX_OOXML_FUTURE
    if( rStrm.getVersion() == oox::core::ISOIEC_29500_2008 )
    {
        rComments->startElement(FSNS(XML_mc, XML_AlternateContent));
        rComments->startElement(FSNS(XML_mc, XML_Choice), XML_Requires, "v2");
        rComments->startElement( XML_commentPr,
                XML_autoFill,       ToPsz( mbAutoFill ),
                XML_autoScale,      ToPsz( mbAutoScale ),
                XML_colHidden,      ToPsz( mbColHidden ),
                XML_locked,         ToPsz( mbLocked ),
                XML_rowHidden,      ToPsz( mbRowHidden ),
                XML_textHAlign,     ToHorizAlign( meTHA ),
                XML_textVAlign,     ToVertAlign( meTVA )  );
        rComments->startElement(XML_anchor, XML_moveWithCells, "false", XML_sizeWithCells, "false");
        rComments->startElement(FSNS(XML_xdr, XML_from));
        lcl_WriteAnchorVertex( rComments, maCommentFrom );
        rComments->endElement( FSNS( XML_xdr, XML_from ) );
        rComments->startElement(FSNS(XML_xdr, XML_to));
        lcl_WriteAnchorVertex( rComments, maCommentTo );
        rComments->endElement( FSNS( XML_xdr, XML_to ) );
        rComments->endElement( XML_anchor );
        rComments->endElement( XML_commentPr );

        rComments->endElement( FSNS( XML_mc, XML_Choice ) );
        rComments->startElement(FSNS(XML_mc, XML_Fallback));
        // Any fallback code ?
        rComments->endElement( FSNS( XML_mc, XML_Fallback ) );
        rComments->endElement( FSNS( XML_mc, XML_AlternateContent ) );
    }
#endif
    rComments->endElement( XML_comment );
}

XclMacroHelper::XclMacroHelper( const XclExpRoot& rRoot ) :
    XclExpControlHelper( rRoot )
{
}

XclMacroHelper::~XclMacroHelper()
{
}

void XclMacroHelper::WriteMacroSubRec( XclExpStream& rStrm )
{
    if( mxMacroLink )
        WriteFormulaSubRec( rStrm, EXC_ID_OBJMACRO, *mxMacroLink );
}

bool
XclMacroHelper::SetMacroLink( const ScriptEventDescriptor& rEvent, const XclTbxEventType& nEventType )
{
    OUString aMacroName = XclControlHelper::ExtractFromMacroDescriptor( rEvent, nEventType );
    if( !aMacroName.isEmpty() )
    {
        return SetMacroLink( aMacroName );
    }
    return false;
}

bool
XclMacroHelper::SetMacroLink( const OUString& rMacroName )
{
    if( !rMacroName.isEmpty() )
    {
        sal_uInt16 nExtSheet = GetLocalLinkManager().FindExtSheet( EXC_EXTSH_OWNDOC );
        sal_uInt16 nNameIdx = GetNameManager().InsertMacroCall( rMacroName, true, false );
        mxMacroLink = GetFormulaCompiler().CreateNameXFormula( nExtSheet, nNameIdx );
        return true;
    }
    return false;
}

XclExpShapeObj::XclExpShapeObj( XclExpObjectManager& rRoot, css::uno::Reference< css::drawing::XShape > const & xShape, ScDocument* pDoc ) :
    XclObjAny( rRoot, xShape, pDoc ),
    XclMacroHelper( rRoot )
{
    if( SdrObject* pSdrObj = ::GetSdrObjectFromXShape( xShape ) )
    {
        ScMacroInfo* pInfo = ScDrawLayer::GetMacroInfo( pSdrObj );
        if ( pInfo && !pInfo->GetMacro().isEmpty() )
// FIXME ooo330-m2: XclControlHelper::GetXclMacroName was removed in upstream sources; they started to call XclTools::GetXclMacroName instead; is this enough? it has only one parameter
//            SetMacroLink( XclControlHelper::GetXclMacroName( pInfo->GetMacro(), rRoot.GetDocShell() ) );
            SetMacroLink( XclTools::GetXclMacroName( pInfo->GetMacro() ) );
    }
}

XclExpShapeObj::~XclExpShapeObj()
{
}

void XclExpShapeObj::WriteSubRecs( XclExpStream& rStrm )
{
    XclObjAny::WriteSubRecs( rStrm );
    WriteMacroSubRec( rStrm );
}

XclExpComments::XclExpComments( SCTAB nTab, XclExpRecordList< XclExpNote >& rNotes )
    : mnTab( nTab ), mrNotes( rNotes )
{
}

void XclExpComments::SaveXml( XclExpXmlStream& rStrm )
{
    if( mrNotes.IsEmpty() )
        return;

    sax_fastparser::FSHelperPtr rComments = rStrm.CreateOutputStream(
            XclXmlUtils::GetStreamName( "xl/", "comments", mnTab + 1 ),
            XclXmlUtils::GetStreamName( "../", "comments", mnTab + 1 ),
            rStrm.GetCurrentStream()->getOutputStream(),
            "application/vnd.openxmlformats-officedocument.spreadsheetml.comments+xml",
            oox::getRelationship(Relationship::COMMENTS));
    rStrm.PushStream( rComments );

    if( rStrm.getVersion() == oox::core::ISOIEC_29500_2008 )
        rComments->startElement( XML_comments,
            XML_xmlns, rStrm.getNamespaceURL(OOX_NS(xls)),
            FSNS(XML_xmlns, XML_mc), rStrm.getNamespaceURL(OOX_NS(mce)),
            FSNS(XML_xmlns, XML_xdr), rStrm.getNamespaceURL(OOX_NS(dmlSpreadDr)),
            FSNS(XML_xmlns, XML_v2), rStrm.getNamespaceURL(OOX_NS(mceTest)),
            FSNS( XML_mc, XML_Ignorable ), "v2" );
    else
        rComments->startElement( XML_comments,
            XML_xmlns, rStrm.getNamespaceURL(OOX_NS(xls)),
            FSNS(XML_xmlns, XML_xdr), rStrm.getNamespaceURL(OOX_NS(dmlSpreadDr)) );

    rComments->startElement(XML_authors);

    typedef std::set<OUString> Authors;
    Authors aAuthors;

    size_t nNotes = mrNotes.GetSize();
    for( size_t i = 0; i < nNotes; ++i )
    {
        aAuthors.insert( XclXmlUtils::ToOUString( mrNotes.GetRecord( i )->GetAuthor() ) );
    }

    for( const auto& rAuthor : aAuthors )
    {
        rComments->startElement(XML_author);
        rComments->writeEscaped( rAuthor );
        rComments->endElement( XML_author );
    }

    rComments->endElement( XML_authors );
    rComments->startElement(XML_commentList);

    Authors::const_iterator aAuthorsBegin = aAuthors.begin();
    for( size_t i = 0; i < nNotes; ++i )
    {
        XclExpRecordList< XclExpNote >::RecordRefType xNote = mrNotes.GetRecord( i );
        Authors::const_iterator aAuthor = aAuthors.find(
                XclXmlUtils::ToOUString( xNote->GetAuthor() ) );
        sal_Int32 nAuthorId = distance( aAuthorsBegin, aAuthor );
        xNote->WriteXml( nAuthorId, rStrm );
    }

    rComments->endElement( XML_commentList );
    rComments->endElement( XML_comments );

    rStrm.PopStream();
}

// object manager =============================================================

XclExpObjectManager::XclExpObjectManager( const XclExpRoot& rRoot ) :
    XclExpRoot( rRoot )
{
    InitStream( true );
    mxEscherEx = std::make_shared<XclEscherEx>( GetRoot(), *this, *mxDffStrm );
}

XclExpObjectManager::XclExpObjectManager( const XclExpObjectManager& rParent ) :
    XclExpRoot( rParent.GetRoot() )
{
    InitStream( false );
    mxEscherEx = std::make_shared<XclEscherEx>( GetRoot(), *this, *mxDffStrm, rParent.mxEscherEx.get() );
}

XclExpObjectManager::~XclExpObjectManager()
{
}

XclExpDffAnchorBase* XclExpObjectManager::CreateDffAnchor() const
{
    return new XclExpDffSheetAnchor( GetRoot() );
}

rtl::Reference< XclExpRecordBase > XclExpObjectManager::CreateDrawingGroup()
{
    return new XclExpMsoDrawingGroup( *mxEscherEx );
}

void XclExpObjectManager::StartSheet()
{
    mxObjList = new XclExpObjList( GetRoot(), *mxEscherEx );
}

rtl::Reference< XclExpRecordBase > XclExpObjectManager::ProcessDrawing( const SdrPage* pSdrPage )
{
    if( pSdrPage )
        mxEscherEx->AddSdrPage( *pSdrPage );
    // the first dummy object may still be open
    OSL_ENSURE( mxEscherEx->GetGroupLevel() <= 1, "XclExpObjectManager::ProcessDrawing - still groups open?" );
    while( mxEscherEx->GetGroupLevel() )
        mxEscherEx->LeaveGroup();
    mxObjList->EndSheet();
    return mxObjList;
}

rtl::Reference< XclExpRecordBase > XclExpObjectManager::ProcessDrawing( const Reference< XShapes >& rxShapes )
{
    if( rxShapes.is() )
        mxEscherEx->AddUnoShapes( rxShapes );
    // the first dummy object may still be open
    OSL_ENSURE( mxEscherEx->GetGroupLevel() <= 1, "XclExpObjectManager::ProcessDrawing - still groups open?" );
    while( mxEscherEx->GetGroupLevel() )
        mxEscherEx->LeaveGroup();
    mxObjList->EndSheet();
    return mxObjList;
}

void XclExpObjectManager::EndDocument()
{
    mxEscherEx->EndDocument();
}

XclExpMsoDrawing* XclExpObjectManager::GetMsodrawingPerSheet()
{
    return mxObjList->GetMsodrawingPerSheet();
}

bool XclExpObjectManager::HasObj() const
{
    return !mxObjList->empty();
}

sal_uInt16 XclExpObjectManager::AddObj( std::unique_ptr<XclObj> pObjRec )
{
    return mxObjList->Add( std::move(pObjRec) );
}

std::unique_ptr<XclObj> XclExpObjectManager::RemoveLastObj()
{
    return mxObjList->pop_back();
}

void XclExpObjectManager::InitStream( bool bTempFile )
{
    if( bTempFile )
    {
        mxTempFile = std::make_shared<::utl::TempFile>();
        if( mxTempFile->IsValid() )
        {
            mxTempFile->EnableKillingFile();
            mxDffStrm = ::utl::UcbStreamHelper::CreateStream( mxTempFile->GetURL(), StreamMode::STD_READWRITE );
        }
    }

    if( !mxDffStrm )
        mxDffStrm = std::make_unique<SvMemoryStream>();

    mxDffStrm->SetEndian( SvStreamEndian::LITTLE );
}

XclExpEmbeddedObjectManager::XclExpEmbeddedObjectManager(
        const XclExpObjectManager& rParent, const Size& rPageSize, sal_Int32 nScaleX, sal_Int32 nScaleY ) :
    XclExpObjectManager( rParent ),
    maPageSize( rPageSize ),
    mnScaleX( nScaleX ),
    mnScaleY( nScaleY )
{
}

XclExpDffAnchorBase* XclExpEmbeddedObjectManager::CreateDffAnchor() const
{
    return new XclExpDffEmbeddedAnchor( GetRoot(), maPageSize, mnScaleX, mnScaleY );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
