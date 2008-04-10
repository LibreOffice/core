/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: xlescher.cxx,v $
 * $Revision: 1.14 $
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
#include "xlescher.hxx"
#include <com/sun/star/drawing/XControlShape.hpp>
#include <svx/unoapi.hxx>
#include "document.hxx"
#include "xistream.hxx"
#include "xestream.hxx"

using ::rtl::OUString;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::drawing::XShape;
using ::com::sun::star::drawing::XControlShape;
using ::com::sun::star::awt::XControlModel;

// Structs and classes ========================================================

// Escher client anchor -------------------------------------------------------

namespace {

/** Returns the scaling factor to calculate coordinates from twips. */
double lclGetTwipsScale( MapUnit eMapUnit )
{
    /*  #111027# We cannot use OutputDevice::LogicToLogic() or the XclTools conversion
        functions to calculate drawing layer coordinates due to Calc's strange definition
        of a point (1 inch == 72.27 points, instead of 72 points). */
    double fScale = 1.0;
    switch( eMapUnit )
    {
        case MAP_TWIP:      fScale = 72 / POINTS_PER_INCH;  break;  // Calc twips <-> real twips
        case MAP_100TH_MM:  fScale = HMM_PER_TWIPS;         break;  // Calc twips <-> 1/100mm
        default:            DBG_ERRORFILE( "lclGetTwipsScale - map unit not implemented" );
    }
    return fScale;
}

/** Calculates a drawing layer X position (in twips) from an Escher object column position. */
long lclGetXFromCol( ScDocument& rDoc, SCTAB nScTab, sal_uInt16 nXclCol, sal_uInt16 nOffset, double fScale )
{
    SCCOL nScCol = static_cast< SCCOL >( nXclCol );
    return static_cast< long >( fScale * (rDoc.GetColOffset( nScCol, nScTab ) +
        ::std::min( nOffset / 1024.0, 1.0 ) * rDoc.GetColWidth( nScCol, nScTab )) + 0.5 );
}

/** Calculates a drawing layer Y position (in twips) from an Escher object row position. */
long lclGetYFromRow( ScDocument& rDoc, SCTAB nScTab, sal_uInt16 nXclRow, sal_uInt16 nOffset, double fScale )
{
    SCROW nScRow = static_cast< SCROW >( nXclRow );
    return static_cast< long >( fScale * (rDoc.GetRowOffset( nScRow, nScTab ) +
        ::std::min( nOffset / 256.0, 1.0 ) * rDoc.GetRowHeight( nScRow, nScTab )) + 0.5 );
}

/** Calculates an Escher object column position from a drawing layer X position (in twips). */
void lclGetColFromX(
        ScDocument& rDoc, SCTAB nScTab, sal_uInt16& rnXclCol,
        sal_uInt16& rnOffset, sal_uInt16 nXclStartCol,
        long& rnStartW, long nX, double fScale )
{
    // rnStartW in conjunction with nXclStartCol is used as buffer for previously calculated width
    long nTwipsX = static_cast< long >( nX / fScale + 0.5 );
    long nColW = 0;
    for( rnXclCol = nXclStartCol; rnXclCol <= MAXCOL; ++rnXclCol )
    {
        nColW = rDoc.GetColWidth( static_cast<SCCOL>(rnXclCol), nScTab );
        if( rnStartW + nColW > nTwipsX )
            break;
        rnStartW += nColW;
    }
    rnOffset = nColW ? static_cast< sal_uInt16 >( (nTwipsX - rnStartW) * 1024.0 / nColW + 0.5 ) : 0;
}

/** Calculates an Escher object row position from a drawing layer Y position (in twips). */
void lclGetRowFromY(
        ScDocument& rDoc, SCTAB nScTab,
        sal_uInt16& rnXclRow, sal_uInt16& rnOffset, sal_uInt16 nXclStartRow,
        long& rnStartH, long nY, double fScale )
{
    // rnStartH in conjunction with nXclStartRow is used as buffer for previously calculated height
    long nTwipsY = static_cast< long >( nY / fScale + 0.5 );
    long nRowH = 0;
    ScCoupledCompressedArrayIterator< SCROW, BYTE, USHORT> aIter(
            rDoc.GetRowFlagsArray( nScTab), static_cast<SCROW>(nXclStartRow),
            MAXROW, CR_HIDDEN, 0, rDoc.GetRowHeightArray( nScTab));
    for ( ; aIter; ++aIter )
    {
        nRowH = *aIter;
        if( rnStartH + nRowH > nTwipsY )
        {
            rnXclRow = static_cast< sal_uInt16 >( aIter.GetPos() );
            break;
        }
        rnStartH += nRowH;
    }
    if (!aIter)
        rnXclRow = static_cast< sal_uInt16 >( aIter.GetIterEnd() );  // down to the bottom..
    rnOffset = static_cast< sal_uInt16 >( nRowH ? ((nTwipsY - rnStartH) * 256.0 / nRowH + 0.5) : 0 );
}

/** Mirrors a rectangle (from LTR to RTL layout or vice versa). */
void lclMirrorRectangle( Rectangle& rRect )
{
    long nLeft = rRect.Left();
    rRect.Left() = -rRect.Right();
    rRect.Right() = -nLeft;
}

} // namespace

// ----------------------------------------------------------------------------

XclEscherAnchor::XclEscherAnchor( SCTAB nScTab ) :
    mnScTab( nScTab ),
    mnLX( 0 ),
    mnTY( 0 ),
    mnRX( 0 ),
    mnBY( 0 )
{
}

Rectangle XclEscherAnchor::GetRect( ScDocument& rDoc, MapUnit eMapUnit ) const
{
    double fScale = lclGetTwipsScale( eMapUnit );
    Rectangle aRect(
        lclGetXFromCol( rDoc, mnScTab, maXclRange.maFirst.mnCol, mnLX, fScale ),
        lclGetYFromRow( rDoc, mnScTab, maXclRange.maFirst.mnRow, mnTY, fScale ),
        lclGetXFromCol( rDoc, mnScTab, maXclRange.maLast.mnCol,  mnRX + 1, fScale ),
        lclGetYFromRow( rDoc, mnScTab, maXclRange.maLast.mnRow,  mnBY, fScale ) );

    // #106948# adjust coordinates in mirrored sheets
    if( rDoc.IsLayoutRTL( mnScTab ) )
        lclMirrorRectangle( aRect );
    return aRect;
}

void XclEscherAnchor::SetRect( ScDocument& rDoc, const Rectangle& rRect, MapUnit eMapUnit )
{
    Rectangle aRect( rRect );
    // #106948# adjust coordinates in mirrored sheets
    if( rDoc.IsLayoutRTL( mnScTab ) )
        lclMirrorRectangle( aRect );

    double fScale = lclGetTwipsScale( eMapUnit );
    long nDummy = 0;
    lclGetColFromX( rDoc, mnScTab, maXclRange.maFirst.mnCol, mnLX, 0,                        nDummy, aRect.Left(),   fScale );
    lclGetColFromX( rDoc, mnScTab, maXclRange.maLast.mnCol,  mnRX, maXclRange.maFirst.mnCol, nDummy, aRect.Right(),  fScale );
    nDummy = 0;
    lclGetRowFromY( rDoc, mnScTab, maXclRange.maFirst.mnRow, mnTY, 0,                        nDummy, aRect.Top(),    fScale );
    lclGetRowFromY( rDoc, mnScTab, maXclRange.maLast.mnRow,  mnBY, maXclRange.maFirst.mnRow, nDummy, aRect.Bottom(), fScale );
}

SvStream& operator>>( SvStream& rStrm, XclEscherAnchor& rAnchor )
{
    return rStrm
        >> rAnchor.maXclRange.maFirst.mnCol >> rAnchor.mnLX
        >> rAnchor.maXclRange.maFirst.mnRow >> rAnchor.mnTY
        >> rAnchor.maXclRange.maLast.mnCol  >> rAnchor.mnRX
        >> rAnchor.maXclRange.maLast.mnRow  >> rAnchor.mnBY;
}

SvStream& operator<<( SvStream& rStrm, const XclEscherAnchor& rAnchor )
{
    return rStrm
        << rAnchor.maXclRange.maFirst.mnCol << rAnchor.mnLX
        << rAnchor.maXclRange.maFirst.mnRow << rAnchor.mnTY
        << rAnchor.maXclRange.maLast.mnCol  << rAnchor.mnRX
        << rAnchor.maXclRange.maLast.mnRow  << rAnchor.mnBY;
}

XclImpStream& operator>>( XclImpStream& rStrm, XclEscherAnchor& rAnchor )
{
    return rStrm
        >> rAnchor.maXclRange.maFirst.mnCol >> rAnchor.mnLX
        >> rAnchor.maXclRange.maFirst.mnRow >> rAnchor.mnTY
        >> rAnchor.maXclRange.maLast.mnCol  >> rAnchor.mnRX
        >> rAnchor.maXclRange.maLast.mnRow  >> rAnchor.mnBY;
}

XclExpStream& operator<<( XclExpStream& rStrm, const XclEscherAnchor& rAnchor )
{
    return rStrm
        << rAnchor.maXclRange.maFirst.mnCol << rAnchor.mnLX
        << rAnchor.maXclRange.maFirst.mnRow << rAnchor.mnTY
        << rAnchor.maXclRange.maLast.mnCol  << rAnchor.mnRX
        << rAnchor.maXclRange.maLast.mnRow  << rAnchor.mnBY;
}

// ----------------------------------------------------------------------------

XclObjId::XclObjId() :
    mnScTab( SCTAB_INVALID ),
    mnObjId( EXC_OBJ_INVALID_ID )
{
}

XclObjId::XclObjId( SCTAB nScTab, sal_uInt16 nObjId ) :
    mnScTab( nScTab ),
    mnObjId( nObjId )
{
}

bool operator==( const XclObjId& rL, const XclObjId& rR )
{
    return (rL.mnScTab == rR.mnScTab) && (rL.mnObjId == rR.mnObjId);
}

bool operator<( const XclObjId& rL, const XclObjId& rR )
{
    return (rL.mnScTab < rR.mnScTab) || ((rL.mnScTab == rR.mnScTab) && (rL.mnObjId < rR.mnObjId));
}

// ----------------------------------------------------------------------------

XclTxoData::XclTxoData() :
    mnFlags( 0 ),
    mnOrient( EXC_TXO_TEXTROT_NONE ),
    mnTextLen( 0 ),
    mnFormatSize( 0 )
{
}

sal_uInt8 XclTxoData::GetXclHorAlignment() const
{
    sal_uInt8 nXclAlign = 0;
    ::extract_value( nXclAlign, mnFlags, 1, 3 );
    return nXclAlign;
}

void XclTxoData::SetXclHorAlignment( sal_uInt8 nXclAlign )
{
    ::insert_value( mnFlags, nXclAlign, 1, 3 );
}

sal_uInt8 XclTxoData::GetXclVerAlignment() const
{
    sal_uInt8 nXclAlign = 0;
    ::extract_value( nXclAlign, mnFlags, 4, 3 );
    return nXclAlign;
}

void XclTxoData::SetXclVerAlignment( sal_uInt8 nXclAlign )
{
    ::insert_value( mnFlags, nXclAlign, 4, 3 );
}

XclImpStream& operator>>( XclImpStream& rStrm, XclTxoData& rData )
{
    rStrm >> rData.mnFlags >> rData.mnOrient;
    rStrm.Ignore( 6 );
    return rStrm >> rData.mnTextLen >> rData.mnFormatSize;
}

XclExpStream& operator<<( XclExpStream& rStrm, const XclTxoData& rData )
{
    rStrm << rData.mnFlags << rData.mnOrient;
    rStrm.WriteZeroBytes( 6 );
    return rStrm << rData.mnTextLen << rData.mnFormatSize << sal_uInt32( 0 );
}

// ----------------------------------------------------------------------------

Reference< XControlModel > XclControlObjHelper::GetModelFromShape( Reference< XShape > xShape )
{
    Reference< XControlModel > xCtrlModel;
    Reference< XControlShape > xCtrlShape( xShape, UNO_QUERY );
    if( xCtrlShape.is() )
        xCtrlModel = xCtrlShape->getControl();
    return xCtrlModel;
}

OUString XclControlObjHelper::GetTbxServiceName( sal_uInt16 nCtrlType )
{
    OUString aName;
#define LCL_CREATE_NAME( name ) CREATE_OUSTRING( "com.sun.star.form.component." name )
    switch( nCtrlType )
    {
        case EXC_OBJ_CMO_BUTTON:        aName = LCL_CREATE_NAME( "CommandButton" ); break;
        case EXC_OBJ_CMO_CHECKBOX:      aName = LCL_CREATE_NAME( "CheckBox" );      break;
        case EXC_OBJ_CMO_OPTIONBUTTON:  aName = LCL_CREATE_NAME( "RadioButton" );   break;
        case EXC_OBJ_CMO_LABEL:         aName = LCL_CREATE_NAME( "FixedText" );     break;
        case EXC_OBJ_CMO_LISTBOX:       aName = LCL_CREATE_NAME( "ListBox" );       break;
        case EXC_OBJ_CMO_GROUPBOX:      aName = LCL_CREATE_NAME( "GroupBox" );      break;
        case EXC_OBJ_CMO_COMBOBOX:      aName = LCL_CREATE_NAME( "ListBox" );       break;  // it's a dropdown listbox
        case EXC_OBJ_CMO_SPIN:          aName = LCL_CREATE_NAME( "SpinButton" );    break;
        case EXC_OBJ_CMO_SCROLLBAR:     aName = LCL_CREATE_NAME( "ScrollBar" );     break;
        default:    DBG_ERRORFILE( "XclControlObjHelper::GetTbxServiceName - unknown control type" );
    }
#undef LCL_CREATE_NAME
    return aName;
}

OUString XclControlObjHelper::GetTbxControlName( sal_uInt16 nCtrlType )
{
    OUString aName;
    switch( nCtrlType )
    {
        case EXC_OBJ_CMO_BUTTON:        aName = CREATE_OUSTRING( "CommandButton" ); break;
        case EXC_OBJ_CMO_CHECKBOX:      aName = CREATE_OUSTRING( "CheckBox" );      break;
        case EXC_OBJ_CMO_OPTIONBUTTON:  aName = CREATE_OUSTRING( "OptionButton" );  break;
        case EXC_OBJ_CMO_LABEL:         aName = CREATE_OUSTRING( "Label" );         break;
        case EXC_OBJ_CMO_LISTBOX:       aName = CREATE_OUSTRING( "ListBox" );       break;
        case EXC_OBJ_CMO_GROUPBOX:      aName = CREATE_OUSTRING( "GroupBox" );      break;
        case EXC_OBJ_CMO_COMBOBOX:      aName = CREATE_OUSTRING( "ComboBox" );      break;
        case EXC_OBJ_CMO_SPIN:          aName = CREATE_OUSTRING( "SpinButton" );    break;
        case EXC_OBJ_CMO_SCROLLBAR:     aName = CREATE_OUSTRING( "ScrollBar" );     break;
        default:    DBG_ERRORFILE( "XclControlObjHelper::GetTbxControlName - unknown control type" );
    }
    return aName;
}

OUString XclControlObjHelper::GetTbxListenerType( sal_uInt16 nCtrlType )
{
    OUString aType;
    switch( nCtrlType )
    {
        case EXC_OBJ_CMO_BUTTON:
        case EXC_OBJ_CMO_CHECKBOX:
        case EXC_OBJ_CMO_OPTIONBUTTON:
            aType = CREATE_OUSTRING( "XActionListener" );
        break;
        case EXC_OBJ_CMO_LABEL:
        case EXC_OBJ_CMO_GROUPBOX:
            aType = CREATE_OUSTRING( "XMouseListener" );
        break;
        case EXC_OBJ_CMO_LISTBOX:
        case EXC_OBJ_CMO_COMBOBOX:
            aType = CREATE_OUSTRING( "XChangeListener" );
        break;
        case EXC_OBJ_CMO_SPIN:
        case EXC_OBJ_CMO_SCROLLBAR:
            aType = CREATE_OUSTRING( "XAdjustmentListener" );
        break;
        default:
            DBG_ERRORFILE( "XclControlObjHelper::GetTbxListenerType - unknown control type" );
    }
    return aType;
}

OUString XclControlObjHelper::GetTbxEventMethod( sal_uInt16 nCtrlType )
{
    OUString aMethod;
    switch( nCtrlType )
    {
        case EXC_OBJ_CMO_BUTTON:
        case EXC_OBJ_CMO_CHECKBOX:
        case EXC_OBJ_CMO_OPTIONBUTTON:
            aMethod = CREATE_OUSTRING( "actionPerformed" );
        break;
        case EXC_OBJ_CMO_LABEL:
        case EXC_OBJ_CMO_GROUPBOX:
            aMethod = CREATE_OUSTRING( "mouseReleased" );
        break;
        case EXC_OBJ_CMO_LISTBOX:
        case EXC_OBJ_CMO_COMBOBOX:
            aMethod = CREATE_OUSTRING( "changed" );
        break;
        case EXC_OBJ_CMO_SPIN:
        case EXC_OBJ_CMO_SCROLLBAR:
            aMethod = CREATE_OUSTRING( "adjustmentValueChanged" );
        break;
        default:
            DBG_ERRORFILE( "XclControlObjHelper::GetTbxEventMethod - unknown control type" );
    }
    return aMethod;
}

OUString XclControlObjHelper::GetTbxScriptType()
{
    return CREATE_OUSTRING( "Script" );
}

#define EXC_TBX_MACRONAME_PRE "vnd.sun.star.script:Standard."
#define EXC_TBX_MACRONAME_SUF "?language=Basic&location=document"

OUString XclControlObjHelper::GetScMacroName( const String& rXclMacroName )
{
    return CREATE_OUSTRING( EXC_TBX_MACRONAME_PRE ) + rXclMacroName + CREATE_OUSTRING( EXC_TBX_MACRONAME_SUF );
}

String XclControlObjHelper::GetXclMacroName( const OUString& rScMacroName )
{
    static const OUString saMacroNamePre = CREATE_OUSTRING( EXC_TBX_MACRONAME_PRE );
    static const OUString saMacroNameSuf = CREATE_OUSTRING( EXC_TBX_MACRONAME_SUF );
    const sal_Int32 snMacroNamePreLen = static_cast< sal_Int32 >( sizeof( EXC_TBX_MACRONAME_PRE ) - 1 );
    const sal_Int32 snMacroNameSufLen = static_cast< sal_Int32 >( sizeof( EXC_TBX_MACRONAME_SUF ) - 1 );

    sal_Int32 snScMacroNameLen = rScMacroName.getLength();
    sal_Int32 snXclMacroNameLen = snScMacroNameLen - snMacroNamePreLen - snMacroNameSufLen;

    String aXclMacroName;
    if( (snXclMacroNameLen > 0) && rScMacroName.match( saMacroNamePre, 0 ) &&
            rScMacroName.match( saMacroNameSuf, snScMacroNameLen - snMacroNameSufLen ) )
        aXclMacroName = rScMacroName.copy( snMacroNamePreLen, snXclMacroNameLen );

    return aXclMacroName;
}

// ============================================================================

