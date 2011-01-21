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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

#include "xeescher.hxx"

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/form/FormComponentType.hpp>
#include <com/sun/star/awt/VisualEffect.hpp>
#include <com/sun/star/awt/ScrollBarOrientation.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/form/binding/XBindableValue.hpp>
#include <com/sun/star/form/binding/XValueBinding.hpp>
#include <com/sun/star/form/binding/XListEntrySink.hpp>
#include <com/sun/star/form/binding/XListEntrySource.hpp>
#include <com/sun/star/script/ScriptEventDescriptor.hpp>

#include <set>
#include <rtl/ustrbuf.h>
#include <vcl/bmpacc.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdocapt.hxx>
#include <editeng/outlobj.hxx>
#include <editeng/editobj.hxx>
#include <unotools/tempfile.hxx>
#include <unotools/ucbstreamhelper.hxx>

#include "editutil.hxx"
#include "unonames.hxx"
#include "convuno.hxx"
#include "postit.hxx"

#include "fapihelper.hxx"
#include "xechart.hxx"
#include "xeformula.hxx"
#include "xelink.hxx"
#include "xename.hxx"
#include "xestyle.hxx"

using namespace ::oox;

using ::rtl::OString;
using ::rtl::OUString;
using ::rtl::OUStringBuffer;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::lang::XServiceInfo;
using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::drawing::XShape;
using ::com::sun::star::drawing::XShapes;
using ::com::sun::star::frame::XModel;
using ::com::sun::star::embed::XEmbeddedObject;
using ::com::sun::star::awt::XControlModel;
using ::com::sun::star::form::binding::XBindableValue;
using ::com::sun::star::form::binding::XValueBinding;
using ::com::sun::star::form::binding::XListEntrySink;
using ::com::sun::star::form::binding::XListEntrySource;
using ::com::sun::star::script::ScriptEventDescriptor;
using ::com::sun::star::table::CellAddress;
using ::com::sun::star::table::CellRangeAddress;

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
    ImplCalcAnchorRect( rSdrObj.GetCurrentBoundRect(), MAP_100TH_MM );
}

void XclExpDffAnchorBase::WriteDffData( EscherEx& rEscherEx ) const
{
    rEscherEx.AddAtom( 18, ESCHER_ClientAnchor );
    rEscherEx.GetStream() << mnFlags << maAnchor;
}

void XclExpDffAnchorBase::WriteData( EscherEx& rEscherEx, const Rectangle& rRect )
{
    // the passed rectangle is in twips
    ImplCalcAnchorRect( rRect, MAP_TWIP );
    WriteDffData( rEscherEx );
}

void XclExpDffAnchorBase::ImplSetFlags( const SdrObject& )
{
    OSL_ENSURE( false, "XclExpDffAnchorBase::ImplSetFlags - not implemented" );
}

void XclExpDffAnchorBase::ImplCalcAnchorRect( const Rectangle&, MapUnit )
{
    OSL_ENSURE( false, "XclExpDffAnchorBase::ImplCalcAnchorRect - not implemented" );
}

// ----------------------------------------------------------------------------

XclExpDffSheetAnchor::XclExpDffSheetAnchor( const XclExpRoot& rRoot ) :
    XclExpDffAnchorBase( rRoot ),
    mnScTab( rRoot.GetCurrScTab() )
{
}

void XclExpDffSheetAnchor::ImplSetFlags( const SdrObject& rSdrObj )
{
    // Special case "page anchor" (X==0,Y==1) -> lock pos and size.
    const Point& rPos = rSdrObj.GetAnchorPos();
    mnFlags = ((rPos.X() == 0) && (rPos.Y() == 1)) ? EXC_ESC_ANCHOR_LOCKED : 0;
}

void XclExpDffSheetAnchor::ImplCalcAnchorRect( const Rectangle& rRect, MapUnit eMapUnit )
{
    maAnchor.SetRect( GetRoot(), mnScTab, rRect, eMapUnit );
}

// ----------------------------------------------------------------------------

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

void XclExpDffEmbeddedAnchor::ImplCalcAnchorRect( const Rectangle& rRect, MapUnit eMapUnit )
{
    maAnchor.SetRect( maPageSize, mnScaleX, mnScaleY, rRect, eMapUnit, true );
}

// ----------------------------------------------------------------------------

XclExpDffNoteAnchor::XclExpDffNoteAnchor( const XclExpRoot& rRoot, const Rectangle& rRect ) :
    XclExpDffAnchorBase( rRoot, EXC_ESC_ANCHOR_SIZELOCKED )
{
    maAnchor.SetRect( rRoot, rRoot.GetCurrScTab(), rRect, MAP_100TH_MM );
}

// ----------------------------------------------------------------------------

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

// ----------------------------------------------------------------------------

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
    rDffStrm.Write( spnDffOpt, sizeof( spnDffOpt ) );

    // SPLITMENUCOLORS contains colors in toolbar
    static const sal_uInt8 spnDffSplitMenuColors[] = {
        0x0D, 0x00, 0x00, 0x08, 0x0C, 0x00, 0x00, 0x08,
        0x17, 0x00, 0x00, 0x08, 0xF7, 0x00, 0x00, 0x10
    };
    mrEscherEx.AddAtom( sizeof( spnDffSplitMenuColors ), ESCHER_SplitMenuColors, 0, 4 );
    rDffStrm.Write( spnDffSplitMenuColors, sizeof( spnDffSplitMenuColors ) );

    // close the DGGCONTAINER
    mrEscherEx.CloseContainer();
    mrEscherEx.UpdateDffFragmentEnd();
}

// ----------------------------------------------------------------------------

XclExpMsoDrawing::XclExpMsoDrawing( XclEscherEx& rEscherEx ) :
    XclExpMsoDrawingBase( rEscherEx, EXC_ID_MSODRAWING )
{
}

// ============================================================================

XclExpImgData::XclExpImgData( const Graphic& rGraphic, sal_uInt16 nRecId ) :
    maGraphic( rGraphic ),
    mnRecId( nRecId )
{
}

void XclExpImgData::Save( XclExpStream& rStrm )
{
    Bitmap aBmp = maGraphic.GetBitmap();
    if( aBmp.GetBitCount() != 24 )
        aBmp.Convert( BMP_CONVERSION_24BIT );

    if( BitmapReadAccess* pAccess = aBmp.AcquireReadAccess() )
    {
        sal_Int32 nWidth = ::std::min< sal_Int32 >( pAccess->Width(), 0xFFFF );
        sal_Int32 nHeight = ::std::min< sal_Int32 >( pAccess->Height(), 0xFFFF );
        if( (nWidth > 0) && (nHeight > 0) )
        {
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
                for( sal_Int32 nX = 0; nX < nWidth; ++nX )
                {
                    const BitmapColor& rBmpColor = pAccess->GetPixel( nY, nX );
                    rStrm << rBmpColor.GetBlue() << rBmpColor.GetGreen() << rBmpColor.GetRed();
                }
                rStrm.WriteZeroBytes( nPadding );
            }

            rStrm.EndRecord();
        }
        aBmp.ReleaseAccess( pAccess );
    }
}

// ============================================================================

XclExpControlHelper::XclExpControlHelper( const XclExpRoot& rRoot ) :
    XclExpRoot( rRoot ),
    mnEntryCount( 0 )
{
}

XclExpControlHelper::~XclExpControlHelper()
{
}

void XclExpControlHelper::ConvertSheetLinks( Reference< XShape > xShape )
{
    mxCellLink.reset();
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
        if( xServInfo.is() && xServInfo->supportsService( CREATE_OUSTRING( SC_SERVICENAME_VALBIND ) ) )
        {
            ScfPropertySet aBindProp( xServInfo );
            CellAddress aApiAddress;
            if( aBindProp.GetProperty( aApiAddress, CREATE_OUSTRING( SC_UNONAME_BOUNDCELL ) ) )
            {
                ScAddress aCellLink;
                ScUnoConversion::FillScAddress( aCellLink, aApiAddress );
                if( GetTabInfo().IsExportTab( aCellLink.Tab() ) )
                    mxCellLink = GetFormulaCompiler().CreateFormula( EXC_FMLATYPE_CONTROL, aCellLink );
            }
        }
    }

    // *** source range *** ---------------------------------------------------

    Reference< XListEntrySink > xEntrySink( xCtrlModel, UNO_QUERY );
    if( xEntrySink.is() )
    {
        Reference< XServiceInfo > xServInfo( xEntrySink->getListEntrySource(), UNO_QUERY );
        if( xServInfo.is() && xServInfo->supportsService( CREATE_OUSTRING( SC_SERVICENAME_LISTSOURCE ) ) )
        {
            ScfPropertySet aSinkProp( xServInfo );
            CellRangeAddress aApiRange;
            if( aSinkProp.GetProperty( aApiRange, CREATE_OUSTRING( SC_UNONAME_CELLRANGE ) ) )
            {
                ScRange aSrcRange;
                ScUnoConversion::FillScRange( aSrcRange, aApiRange );
                if( (aSrcRange.aStart.Tab() == aSrcRange.aEnd.Tab()) && GetTabInfo().IsExportTab( aSrcRange.aStart.Tab() ) )
                    mxSrcRange = GetFormulaCompiler().CreateFormula( EXC_FMLATYPE_CONTROL, aSrcRange );
                mnEntryCount = static_cast< sal_uInt16 >( aSrcRange.aEnd.Col() - aSrcRange.aStart.Col() + 1 );
            }
        }
    }
}

void XclExpControlHelper::WriteFormula( XclExpStream& rStrm, const XclTokenArray& rTokArr ) const
{
    sal_uInt16 nFmlaSize = rTokArr.GetSize();
    rStrm << nFmlaSize << sal_uInt32( 0 );
    rTokArr.WriteArray( rStrm );
    if( nFmlaSize & 1 )             // pad to 16-bit
        rStrm << sal_uInt8( 0 );
}

void XclExpControlHelper::WriteFormulaSubRec( XclExpStream& rStrm, sal_uInt16 nSubRecId, const XclTokenArray& rTokArr ) const
{
    rStrm.StartRecord( nSubRecId, (rTokArr.GetSize() + 5) & ~1 );
    WriteFormula( rStrm, rTokArr );
    rStrm.EndRecord();
}

// ----------------------------------------------------------------------------

#if EXC_EXP_OCX_CTRL

XclExpOcxControlObj::XclExpOcxControlObj( XclExpObjectManager& rObjMgr, Reference< XShape > xShape,
        const Rectangle* pChildAnchor, const String& rClassName, sal_uInt32 nStrmStart, sal_uInt32 nStrmSize ) :
    XclObj( rObjMgr, EXC_OBJTYPE_PICTURE, true ),
    XclExpControlHelper( rObjMgr.GetRoot() ),
    maClassName( rClassName ),
    mnStrmStart( nStrmStart ),
    mnStrmSize( nStrmSize )
{
    ScfPropertySet aCtrlProp( XclControlHelper::GetControlModel( xShape ) );

    // OBJ record flags
    SetLocked( sal_True );
    SetPrintable( aCtrlProp.GetBoolProperty( CREATE_OUSTRING( "Printable" ) ) );
    SetAutoFill( sal_False );
    SetAutoLine( sal_False );

    // fill DFF property set
    mrEscherEx.OpenContainer( ESCHER_SpContainer );
    mrEscherEx.AddShape( ESCHER_ShpInst_HostControl, SHAPEFLAG_HAVESPT | SHAPEFLAG_HAVEANCHOR | SHAPEFLAG_OLESHAPE );
    Rectangle aDummyRect;
    EscherPropertyContainer aPropOpt( mrEscherEx, mrEscherEx.QueryPicStream(), aDummyRect );
    aPropOpt.AddOpt( ESCHER_Prop_FitTextToShape,    0x00080008 );   // bool field
    aPropOpt.AddOpt( ESCHER_Prop_lineColor,         0x08000040 );
    aPropOpt.AddOpt( ESCHER_Prop_fNoLineDrawDash,   0x00080000 );   // bool field

    // #i51348# name of the control, may overwrite shape name
    OUString aCtrlName;
    if( aCtrlProp.GetProperty( aCtrlName, CREATE_OUSTRING( "Name" ) ) && (aCtrlName.getLength() > 0) )
        aPropOpt.AddOpt( ESCHER_Prop_wzName, aCtrlName );

    // meta file
    //! TODO - needs check
    Reference< XPropertySet > xShapePS( xShape, UNO_QUERY );
    if( xShapePS.is() && aPropOpt.CreateGraphicProperties( xShapePS, CREATE_STRING( "MetaFile" ), sal_False ) )
    {
        sal_uInt32 nBlipId;
        if( aPropOpt.GetOpt( ESCHER_Prop_pib, nBlipId ) )
            aPropOpt.AddOpt( ESCHER_Prop_pictureId, nBlipId );
    }

    // write DFF property set to stream
    aPropOpt.Commit( mrEscherEx.GetStream() );

    // anchor
    ImplWriteAnchor( GetRoot(), SdrObject::getSdrObjectFromXShape( xShape ), pChildAnchor );

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

    rStrm   << sal_uInt16( nFirstPartSize )             // size of first part
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

#else

XclExpTbxControlObj::XclExpTbxControlObj( XclExpObjectManager& rObjMgr, Reference< XShape > xShape, const Rectangle* pChildAnchor ) :
    XclObj( rObjMgr, EXC_OBJTYPE_UNKNOWN, true ),
    XclExpControlHelper( rObjMgr.GetRoot() ),
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
    mbScrollHor( false )
{
    namespace FormCompType = ::com::sun::star::form::FormComponentType;
    namespace AwtVisualEffect = ::com::sun::star::awt::VisualEffect;
    namespace AwtScrollOrient = ::com::sun::star::awt::ScrollBarOrientation;

    ScfPropertySet aCtrlProp( XclControlHelper::GetControlModel( xShape ) );
    if( !xShape.is() || !aCtrlProp.Is() )
        return;

    mnHeight = xShape->getSize().Height;
    if( mnHeight <= 0 )
        return;

    // control type
    sal_Int16 nClassId = 0;
    if( aCtrlProp.GetProperty( nClassId, CREATE_OUSTRING( "ClassId" ) ) )
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
    SetLocked( sal_True );
    SetPrintable( aCtrlProp.GetBoolProperty( CREATE_OUSTRING( "Printable" ) ) );
    SetAutoFill( sal_False );
    SetAutoLine( sal_False );

    // fill DFF property set
    mrEscherEx.OpenContainer( ESCHER_SpContainer );
    mrEscherEx.AddShape( ESCHER_ShpInst_HostControl, SHAPEFLAG_HAVEANCHOR | SHAPEFLAG_HAVESPT );
    EscherPropertyContainer aPropOpt;
    bool bVisible = aCtrlProp.GetBoolProperty( CREATE_OUSTRING( "EnableVisible" ) );
    aPropOpt.AddOpt( ESCHER_Prop_fPrint, bVisible ? 0x00080000 : 0x00080002 ); // visible flag

    aPropOpt.AddOpt( ESCHER_Prop_LockAgainstGrouping, 0x01000100 ); // bool field
    aPropOpt.AddOpt( ESCHER_Prop_lTxid, 0 );                        // Text ID
    aPropOpt.AddOpt( ESCHER_Prop_WrapText, 0x00000001 );
    aPropOpt.AddOpt( ESCHER_Prop_FitTextToShape, 0x001A0008 );      // bool field
    aPropOpt.AddOpt( ESCHER_Prop_fNoFillHitTest, 0x00100000 );      // bool field
    aPropOpt.AddOpt( ESCHER_Prop_fNoLineDrawDash, 0x00080000 );     // bool field

    // #i51348# name of the control, may overwrite shape name
    OUString aCtrlName;
    if( aCtrlProp.GetProperty( aCtrlName, CREATE_OUSTRING( "Name" ) ) && (aCtrlName.getLength() > 0) )
        aPropOpt.AddOpt( ESCHER_Prop_wzName, aCtrlName );

    // write DFF property set to stream
    aPropOpt.Commit( mrEscherEx.GetStream() );

    // anchor
    ImplWriteAnchor( GetRoot(), SdrObject::getSdrObjectFromXShape( xShape ), pChildAnchor );

    mrEscherEx.AddAtom( 0, ESCHER_ClientData );                       // OBJ record
    mrEscherEx.UpdateDffFragmentEnd();

    // control label
    OUString aString;
    if( aCtrlProp.GetProperty( aString, CREATE_OUSTRING( "Label" ) ) )
    {
        /*  Be sure to construct the MSODRAWING record containing the
            ClientTextbox atom after the base OBJ's MSODRAWING record data is
            completed. */
        pClientTextbox = new XclExpMsoDrawing( mrEscherEx );
        mrEscherEx.AddAtom( 0, ESCHER_ClientTextbox );  // TXO record
        mrEscherEx.UpdateDffFragmentEnd();

        sal_uInt16 nXclFont = EXC_FONT_APP;
        if( aString.getLength() > 0 )
        {
            XclFontData aFontData;
            GetFontPropSetHelper().ReadFontProperties( aFontData, aCtrlProp, EXC_FONTPROPSET_CONTROL );
            if( (aFontData.maName.Len() > 0) && (aFontData.mnHeight > 0) )
                nXclFont = GetFontBuffer().Insert( aFontData, EXC_COLOR_CTRLTEXT );
        }

        pTxo = new XclTxo( aString, nXclFont );
        pTxo->SetHorAlign( (mnObjType == EXC_OBJTYPE_BUTTON) ? EXC_OBJ_HOR_CENTER : EXC_OBJ_HOR_LEFT );
        pTxo->SetVerAlign( EXC_OBJ_VER_CENTER );
    }

    mrEscherEx.CloseContainer();  // ESCHER_SpContainer

    // other properties
    aCtrlProp.GetProperty( mnLineCount, CREATE_OUSTRING( "LineCount" ) );

    // border style
    sal_Int16 nApiButton = AwtVisualEffect::LOOK3D;
    sal_Int16 nApiBorder = AwtVisualEffect::LOOK3D;
    switch( nClassId )
    {
        case FormCompType::LISTBOX:
        case FormCompType::COMBOBOX:
            aCtrlProp.GetProperty( nApiBorder, CREATE_OUSTRING( "Border" ) );
        break;
        case FormCompType::CHECKBOX:
        case FormCompType::RADIOBUTTON:
            aCtrlProp.GetProperty( nApiButton, CREATE_OUSTRING( "VisualEffect" ) );
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
    if( aCtrlProp.GetProperty( nApiState, CREATE_OUSTRING( "State" ) ) )
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
            mbMultiSel = aCtrlProp.GetBoolProperty( CREATE_OUSTRING( "MultiSelection" ) );
            Sequence< sal_Int16 > aSelection;
            if( aCtrlProp.GetProperty( aSelection, CREATE_OUSTRING( "SelectedItems" ) ) )
            {
                sal_Int32 nLen = aSelection.getLength();
                if( nLen > 0 )
                {
                    mnSelEntry = aSelection[ 0 ] + 1;
                    maMultiSel.resize( nLen );
                    const sal_Int16* pnBegin = aSelection.getConstArray();
                    ::std::copy( pnBegin, pnBegin + nLen, maMultiSel.begin() );
                }
            }

            // convert listbox with dropdown button to Excel dropdown
            if( aCtrlProp.GetBoolProperty( CREATE_OUSTRING( "Dropdown" ) ) )
                mnObjType = EXC_OBJTYPE_DROPDOWN;
        }
        break;

        case FormCompType::COMBOBOX:
        {
            Sequence< OUString > aStringList;
            OUString aDefText;
            if( aCtrlProp.GetProperty( aStringList, CREATE_OUSTRING( "StringItemList" ) ) &&
                aCtrlProp.GetProperty( aDefText, CREATE_OUSTRING( "Text" ) ) &&
                aStringList.getLength() && aDefText.getLength() )
            {
                const OUString* pBegin = aStringList.getConstArray();
                const OUString* pEnd = pBegin + aStringList.getLength();
                const OUString* pString = ::std::find( pBegin, pEnd, aDefText );
                if( pString != pEnd )
                    mnSelEntry = static_cast< sal_Int16 >( pString - pBegin + 1 );  // 1-based
                if( mnSelEntry > 0 )
                    maMultiSel.resize( 1, mnSelEntry - 1 );
            }

            // convert combobox without dropdown button to Excel listbox
            if( !aCtrlProp.GetBoolProperty( CREATE_OUSTRING( "Dropdown" ) ) )
                mnObjType = EXC_OBJTYPE_LISTBOX;
        }
        break;

        case FormCompType::SCROLLBAR:
        {
            sal_Int32 nApiValue = 0;
            if( aCtrlProp.GetProperty( nApiValue, CREATE_OUSTRING( "ScrollValueMin" ) ) )
                mnScrollMin = limit_cast< sal_uInt16 >( nApiValue, EXC_OBJ_SCROLLBAR_MIN, EXC_OBJ_SCROLLBAR_MAX );
            if( aCtrlProp.GetProperty( nApiValue, CREATE_OUSTRING( "ScrollValueMax" ) ) )
                mnScrollMax = limit_cast< sal_uInt16 >( nApiValue, mnScrollMin, EXC_OBJ_SCROLLBAR_MIN );
            if( aCtrlProp.GetProperty( nApiValue, CREATE_OUSTRING( "ScrollValue" ) ) )
                mnScrollValue = limit_cast< sal_uInt16 >( nApiValue, mnScrollMin, mnScrollMax );
            if( aCtrlProp.GetProperty( nApiValue, CREATE_OUSTRING( "LineIncrement" ) ) )
                mnScrollStep = limit_cast< sal_uInt16 >( nApiValue, EXC_OBJ_SCROLLBAR_MIN, EXC_OBJ_SCROLLBAR_MAX );
            if( aCtrlProp.GetProperty( nApiValue, CREATE_OUSTRING( "BlockIncrement" ) ) )
                mnScrollPage = limit_cast< sal_uInt16 >( nApiValue, EXC_OBJ_SCROLLBAR_MIN, EXC_OBJ_SCROLLBAR_MAX );
            if( aCtrlProp.GetProperty( nApiValue, CREATE_OUSTRING( "Orientation" ) ) )
                mbScrollHor = nApiValue == AwtScrollOrient::HORIZONTAL;
        }
        break;

        case FormCompType::SPINBUTTON:
        {
            sal_Int32 nApiValue = 0;
            if( aCtrlProp.GetProperty( nApiValue, CREATE_OUSTRING( "SpinValueMin" ) ) )
                mnScrollMin = limit_cast< sal_uInt16 >( nApiValue, EXC_OBJ_SCROLLBAR_MIN, EXC_OBJ_SCROLLBAR_MAX );
            if( aCtrlProp.GetProperty( nApiValue, CREATE_OUSTRING( "SpinValueMax" ) ) )
                mnScrollMax = limit_cast< sal_uInt16 >( nApiValue, mnScrollMin, EXC_OBJ_SCROLLBAR_MAX );
            if( aCtrlProp.GetProperty( nApiValue, CREATE_OUSTRING( "SpinValue" ) ) )
                mnScrollValue = limit_cast< sal_uInt16 >( nApiValue, mnScrollMin, mnScrollMax );
            if( aCtrlProp.GetProperty( nApiValue, CREATE_OUSTRING( "SpinIncrement" ) ) )
                mnScrollStep = limit_cast< sal_uInt16 >( nApiValue, EXC_OBJ_SCROLLBAR_MIN, EXC_OBJ_SCROLLBAR_MAX );
            if( aCtrlProp.GetProperty( nApiValue, CREATE_OUSTRING( "Orientation" ) ) )
                mbScrollHor = nApiValue == AwtScrollOrient::HORIZONTAL;
        }
        break;
    }

    // spreadsheet links
    ConvertSheetLinks( xShape );
}

bool XclExpTbxControlObj::SetMacroLink( const ScriptEventDescriptor& rEvent )
{
    String aMacroName = XclControlHelper::ExtractFromMacroDescriptor( rEvent, meEventType );
    if( aMacroName.Len() )
    {
        sal_uInt16 nExtSheet = GetLocalLinkManager().FindExtSheet( EXC_EXTSH_OWNDOC );
        sal_uInt16 nNameIdx = GetNameManager().InsertMacroCall( aMacroName, true, false );
        mxMacroLink = GetFormulaCompiler().CreateNameXFormula( nExtSheet, nNameIdx );
        return true;
    }
    return false;
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
                    for( ScfInt16Vec::const_iterator aIt = maMultiSel.begin(), aEnd = maMultiSel.end(); aIt != aEnd; ++aIt )
                        if( *aIt < nEntryCount )
                            aSelEx[ *aIt ] = 1;
                    rStrm.Write( &aSelEx[ 0 ], aSelEx.size() );
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

void XclExpTbxControlObj::WriteMacroSubRec( XclExpStream& rStrm )
{
    if( mxMacroLink.is() )
        WriteFormulaSubRec( rStrm, EXC_ID_OBJMACRO, *mxMacroLink );
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

#endif

// ----------------------------------------------------------------------------

XclExpChartObj::XclExpChartObj( XclExpObjectManager& rObjMgr, Reference< XShape > xShape, const Rectangle* pChildAnchor ) :
    XclObj( rObjMgr, EXC_OBJTYPE_CHART ),
    XclExpRoot( rObjMgr.GetRoot() )
{
    // create the MSODRAWING record contents for the chart object
    mrEscherEx.OpenContainer( ESCHER_SpContainer );
    mrEscherEx.AddShape( ESCHER_ShpInst_HostControl, SHAPEFLAG_HAVEANCHOR | SHAPEFLAG_HAVESPT );
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
    ImplWriteAnchor( GetRoot(), pSdrObj, pChildAnchor );

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
    aShapeProp.GetProperty( xModel, CREATE_OUSTRING( "Model" ) );
    ::com::sun::star::awt::Rectangle aBoundRect;
    aShapeProp.GetProperty( aBoundRect, CREATE_OUSTRING( "BoundRect" ) );
    Rectangle aChartRect( Point( aBoundRect.X, aBoundRect.Y ), Size( aBoundRect.Width, aBoundRect.Height ) );
    mxChart.reset( new XclExpChart( GetRoot(), xModel, aChartRect ) );
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

// ============================================================================

XclExpNote::XclExpNote( const XclExpRoot& rRoot, const ScAddress& rScPos,
        const ScPostIt* pScNote, const String& rAddText ) :
    XclExpRecord( EXC_ID_NOTE ),
    maScPos( rScPos ),
    mnObjId( EXC_OBJ_INVALID_ID ),
    mbVisible( pScNote && pScNote->IsCaptionShown() )
{
    // get the main note text
    String aNoteText;
    if( pScNote )
        aNoteText = pScNote->GetText();
    // append additional text
    ScGlobal::AddToken( aNoteText, rAddText, '\n', 2 );
    maOrigNoteText = aNoteText;

    // initialize record dependent on BIFF type
    switch( rRoot.GetBiff() )
    {
        case EXC_BIFF5:
            maNoteText = ByteString( aNoteText, rRoot.GetTextEncoding() );
        break;

        case EXC_BIFF8:
        {
            // TODO: additional text
            if( pScNote )
                if( SdrCaptionObj* pCaption = pScNote->GetOrCreateCaption( maScPos ) )
                    if( const OutlinerParaObject* pOPO = pCaption->GetOutlinerParaObject() )
                        mnObjId = rRoot.GetObjectManager().AddObj( new XclObjComment( rRoot.GetObjectManager(), pCaption->GetLogicRect(), pOPO->GetTextObject(), pCaption, mbVisible ) );

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
            const sal_Char* pcBuffer = maNoteText.GetBuffer();
            sal_uInt16 nCharsLeft = static_cast< sal_uInt16 >( maNoteText.Len() );

            while( nCharsLeft )
            {
                sal_uInt16 nWriteChars = ::std::min( nCharsLeft, EXC_NOTE5_MAXLEN );

                rStrm.StartRecord( EXC_ID_NOTE, 6 + nWriteChars );
                if( pcBuffer == maNoteText.GetBuffer() )
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
    DBG_ASSERT_BIFF( rStrm.GetRoot().GetBiff() == EXC_BIFF8 );

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
            XML_ref,        XclXmlUtils::ToOString( maScPos ).getStr(),
            XML_authorId,   OString::valueOf( nAuthorId ).getStr(),
            // OOXTODO: XML_guid,
            FSEND );
    rComments->startElement( XML_text, FSEND );
    // OOXTODO: phoneticPr, rPh, r
    rComments->startElement( XML_t, FSEND );
    rComments->writeEscaped( XclXmlUtils::ToOUString( maOrigNoteText ) );
    rComments->endElement ( XML_t );
    rComments->endElement( XML_text );
    rComments->endElement( XML_comment );
}

// ============================================================================

XclExpComments::XclExpComments( SCTAB nTab, XclExpRecordList< XclExpNote >& rNotes )
    : mnTab( nTab ), mrNotes( rNotes )
{
}

struct OUStringLess : public std::binary_function<OUString, OUString, bool>
{
    bool operator()(const OUString& x, const OUString& y) const
    {
        return x.compareTo( y ) <= 0;
    }
};

void XclExpComments::SaveXml( XclExpXmlStream& rStrm )
{
    if( mrNotes.IsEmpty() )
        return;

    sax_fastparser::FSHelperPtr rComments = rStrm.CreateOutputStream(
            XclXmlUtils::GetStreamName( "xl/", "comments", mnTab + 1 ),
            XclXmlUtils::GetStreamName( "../", "comments", mnTab + 1 ),
            rStrm.GetCurrentStream()->getOutputStream(),
            "application/vnd.openxmlformats-officedocument.spreadsheetml.comments+xml",
            "http://schemas.openxmlformats.org/officeDocument/2006/relationships/comments" );
    rStrm.PushStream( rComments );

    rComments->startElement( XML_comments,
            XML_xmlns, "http://schemas.openxmlformats.org/spreadsheetml/2006/main",
            FSEND );
    rComments->startElement( XML_authors, FSEND );

    typedef std::set< OUString, OUStringLess > Authors;
    Authors aAuthors;

    size_t nNotes = mrNotes.GetSize();
    for( size_t i = 0; i < nNotes; ++i )
    {
        aAuthors.insert( XclXmlUtils::ToOUString( mrNotes.GetRecord( i )->GetAuthor() ) );
    }

    for( Authors::const_iterator b = aAuthors.begin(), e = aAuthors.end(); b != e; ++b )
    {
        rComments->startElement( XML_author, FSEND );
        rComments->writeEscaped( *b );
        rComments->endElement( XML_author );
    }

    rComments->endElement( XML_authors );
    rComments->startElement( XML_commentList, FSEND );

    for( size_t i = 0; i < nNotes; ++i )
    {
        XclExpNoteList::RecordRefType xNote = mrNotes.GetRecord( i );
        Authors::const_iterator aAuthor = aAuthors.find(
                XclXmlUtils::ToOUString( xNote->GetAuthor() ) );
        sal_Int32 nAuthorId = distance( aAuthors.begin(), aAuthor );
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
    mxEscherEx.reset( new XclEscherEx( GetRoot(), *this, *mxDffStrm ) );
}

XclExpObjectManager::XclExpObjectManager( const XclExpObjectManager& rParent ) :
    XclExpRoot( rParent.GetRoot() )
{
    InitStream( false );
    mxEscherEx.reset( new XclEscherEx( GetRoot(), *this, *mxDffStrm, rParent.mxEscherEx.get() ) );
}

XclExpObjectManager::~XclExpObjectManager()
{
}

XclExpDffAnchorBase* XclExpObjectManager::CreateDffAnchor() const
{
    return new XclExpDffSheetAnchor( GetRoot() );
}

ScfRef< XclExpRecordBase > XclExpObjectManager::CreateDrawingGroup()
{
    return ScfRef< XclExpRecordBase >( new XclExpMsoDrawingGroup( *mxEscherEx ) );
}

void XclExpObjectManager::StartSheet()
{
    mxObjList.reset( new XclExpObjList( GetRoot(), *mxEscherEx ) );
}

ScfRef< XclExpRecordBase > XclExpObjectManager::ProcessDrawing( SdrPage* pSdrPage )
{
    if( pSdrPage )
        mxEscherEx->AddSdrPage( *pSdrPage );
    // #106213# the first dummy object may still be open
    DBG_ASSERT( mxEscherEx->GetGroupLevel() <= 1, "XclExpObjectManager::ProcessDrawing - still groups open?" );
    while( mxEscherEx->GetGroupLevel() )
        mxEscherEx->LeaveGroup();
    mxObjList->EndSheet();
    return mxObjList;
}

ScfRef< XclExpRecordBase > XclExpObjectManager::ProcessDrawing( const Reference< XShapes >& rxShapes )
{
    if( rxShapes.is() )
        mxEscherEx->AddUnoShapes( rxShapes );
    // #106213# the first dummy object may still be open
    DBG_ASSERT( mxEscherEx->GetGroupLevel() <= 1, "XclExpObjectManager::ProcessDrawing - still groups open?" );
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
    return mxObjList->Count() > 0;
}

sal_uInt16 XclExpObjectManager::AddObj( XclObj* pObjRec )
{
    return mxObjList->Add( pObjRec );
}

XclObj* XclExpObjectManager::RemoveLastObj()
{
    XclObj* pLastObj = static_cast< XclObj* >( mxObjList->Last() );
    mxObjList->Remove();    // remove current, which is the Last()
    return pLastObj;
}

void XclExpObjectManager::InitStream( bool bTempFile )
{
    if( bTempFile )
    {
        mxTempFile.reset( new ::utl::TempFile );
        if( mxTempFile->IsValid() )
        {
            mxTempFile->EnableKillingFile();
            mxDffStrm.reset( ::utl::UcbStreamHelper::CreateStream( mxTempFile->GetURL(), STREAM_STD_READWRITE ) );
        }
    }

    if( !mxDffStrm.get() )
        mxDffStrm.reset( new SvMemoryStream );

    mxDffStrm->SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );
}

// ----------------------------------------------------------------------------

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

// ============================================================================

