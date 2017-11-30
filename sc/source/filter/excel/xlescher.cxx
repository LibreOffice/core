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

#include <xlescher.hxx>

#include <com/sun/star/drawing/XControlShape.hpp>
#include <com/sun/star/script/ScriptEventDescriptor.hpp>
#include <svx/unoapi.hxx>
#include <document.hxx>
#include <xestream.hxx>
#include <xistream.hxx>
#include <xlroot.hxx>
#include <xltools.hxx>

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::drawing::XShape;
using ::com::sun::star::drawing::XControlShape;
using ::com::sun::star::awt::XControlModel;
using ::com::sun::star::script::ScriptEventDescriptor;

namespace {

/** Returns the scaling factor to calculate coordinates from twips. */
double lclGetTwipsScale( MapUnit eMapUnit )
{
    /*  We cannot use OutputDevice::LogicToLogic() or the XclTools
        conversion functions to calculate drawing layer coordinates due to
        Calc's strange definition of a point (1 inch == 72.27 points, instead
        of 72 points).
        NOTE: Calc's definition changed from TeX points (72.27) to PS points
        (72), so the MapUnit::MapTwip case now actually also delivers a scale of 1.0
    */
    double fScale = 1.0;
    switch( eMapUnit )
    {
        case MapUnit::MapTwip:      fScale = 1;               break;  // Calc twips <-> real twips
        case MapUnit::Map100thMM:  fScale = HMM_PER_TWIPS;   break;  // Calc twips <-> 1/100mm
        default:            OSL_FAIL( "lclGetTwipsScale - map unit not implemented" );
    }
    return fScale;
}

/** Calculates a drawing layer X position (in twips) from an object column position. */
long lclGetXFromCol( const ScDocument& rDoc, SCTAB nScTab, sal_uInt16 nXclCol, sal_uInt16 nOffset, double fScale )
{
    SCCOL nScCol = static_cast< SCCOL >( nXclCol );
    return static_cast< long >( fScale * (rDoc.GetColOffset( nScCol, nScTab ) +
        ::std::min( nOffset / 1024.0, 1.0 ) * rDoc.GetColWidth( nScCol, nScTab )) + 0.5 );
}

/** Calculates a drawing layer Y position (in twips) from an object row position. */
long lclGetYFromRow( const ScDocument& rDoc, SCTAB nScTab, sal_uInt16 nXclRow, sal_uInt16 nOffset, double fScale )
{
    SCROW nScRow = static_cast< SCROW >( nXclRow );
    return static_cast< long >( fScale * (rDoc.GetRowOffset( nScRow, nScTab ) +
        ::std::min( nOffset / 256.0, 1.0 ) * rDoc.GetRowHeight( nScRow, nScTab )) + 0.5 );
}

/** Calculates an object column position from a drawing layer X position (in twips). */
void lclGetColFromX(
        const ScDocument& rDoc, SCTAB nScTab, sal_uInt16& rnXclCol,
        sal_uInt16& rnOffset, sal_uInt16 nXclStartCol, sal_uInt16 nXclMaxCol,
        long& rnStartW, long nX, double fScale )
{
    // rnStartW in conjunction with nXclStartCol is used as buffer for previously calculated width
    long nTwipsX = static_cast< long >( nX / fScale + 0.5 );
    long nColW = 0;
    for( rnXclCol = nXclStartCol; rnXclCol <= nXclMaxCol; ++rnXclCol )
    {
        nColW = rDoc.GetColWidth( static_cast< SCCOL >( rnXclCol ), nScTab );
        if( rnStartW + nColW > nTwipsX )
            break;
        rnStartW += nColW;
    }
    rnOffset = nColW ? static_cast< sal_uInt16 >( (nTwipsX - rnStartW) * 1024.0 / nColW + 0.5 ) : 0;
}

/** Calculates an object row position from a drawing layer Y position (in twips). */
void lclGetRowFromY(
        const ScDocument& rDoc, SCTAB nScTab, sal_uInt32& rnXclRow,
        sal_uInt32& rnOffset, sal_uInt32 nXclStartRow, sal_uInt32 nXclMaxRow,
        long& rnStartH, long nY, double fScale )
{
    // rnStartH in conjunction with nXclStartRow is used as buffer for previously calculated height
    long nTwipsY = static_cast< long >( nY / fScale + 0.5 );
    long nRowH = 0;
    bool bFound = false;
    for( SCROW nRow = static_cast< SCROW >( nXclStartRow ); static_cast<unsigned>(nRow) <= nXclMaxRow; ++nRow )
    {
        nRowH = rDoc.GetRowHeight( nRow, nScTab );
        if( rnStartH + nRowH > nTwipsY )
        {
            rnXclRow = static_cast< sal_uInt32 >( nRow );
            bFound = true;
            break;
        }
        rnStartH += nRowH;
    }
    if( !bFound )
        rnXclRow = nXclMaxRow;
    rnOffset = static_cast< sal_uInt32 >( nRowH ? ((nTwipsY - rnStartH) * 256.0 / nRowH + 0.5) : 0 );
}

/** Mirrors a rectangle (from LTR to RTL layout or vice versa). */
void lclMirrorRectangle( tools::Rectangle& rRect )
{
    long nLeft = rRect.Left();
    rRect.Left() = -rRect.Right();
    rRect.Right() = -nLeft;
}

sal_uInt16 lclGetEmbeddedScale( long nPageSize, sal_Int32 nPageScale, long nPos, double fPosScale )
{
    return static_cast< sal_uInt16 >( nPos * fPosScale / nPageSize * nPageScale + 0.5 );
}

} // namespace

XclObjAnchor::XclObjAnchor() :
    mnLX( 0 ),
    mnTY( 0 ),
    mnRX( 0 ),
    mnBY( 0 )
{
}

tools::Rectangle XclObjAnchor::GetRect( const XclRoot& rRoot, SCTAB nScTab, MapUnit eMapUnit ) const
{
    ScDocument& rDoc = rRoot.GetDoc();
    double fScale = lclGetTwipsScale( eMapUnit );
    tools::Rectangle aRect(
        lclGetXFromCol(rDoc, nScTab, std::min<SCCOL>(maFirst.mnCol, MAXCOL), mnLX, fScale),
        lclGetYFromRow(rDoc, nScTab, std::min<SCROW>(maFirst.mnRow, MAXROW), mnTY, fScale),
        lclGetXFromCol(rDoc, nScTab, std::min<SCCOL>(maLast.mnCol, MAXCOL),  mnRX + 1, fScale),
        lclGetYFromRow(rDoc, nScTab, std::min<SCROW>(maLast.mnRow, MAXROW),  mnBY, fScale));

    // adjust coordinates in mirrored sheets
    if( rDoc.IsLayoutRTL( nScTab ) )
        lclMirrorRectangle( aRect );
    return aRect;
}

void XclObjAnchor::SetRect( const XclRoot& rRoot, SCTAB nScTab, const tools::Rectangle& rRect, MapUnit eMapUnit )
{
    ScDocument& rDoc = rRoot.GetDoc();
    sal_uInt16 nXclMaxCol = rRoot.GetXclMaxPos().Col();
    sal_uInt16 nXclMaxRow = static_cast<sal_uInt16>( rRoot.GetXclMaxPos().Row());

    // adjust coordinates in mirrored sheets
    tools::Rectangle aRect( rRect );
    if( rDoc.IsLayoutRTL( nScTab ) )
        lclMirrorRectangle( aRect );

    double fScale = lclGetTwipsScale( eMapUnit );
    long nDummy = 0;
    lclGetColFromX( rDoc, nScTab, maFirst.mnCol, mnLX, 0,             nXclMaxCol, nDummy, aRect.Left(),   fScale );
    lclGetColFromX( rDoc, nScTab, maLast.mnCol,  mnRX, maFirst.mnCol, nXclMaxCol, nDummy, aRect.Right(),  fScale );
    nDummy = 0;
    lclGetRowFromY( rDoc, nScTab, maFirst.mnRow, mnTY, 0,             nXclMaxRow, nDummy, aRect.Top(),    fScale );
    lclGetRowFromY( rDoc, nScTab, maLast.mnRow,  mnBY, maFirst.mnRow, nXclMaxRow, nDummy, aRect.Bottom(), fScale );
}

void XclObjAnchor::SetRect( const Size& rPageSize, sal_Int32 nScaleX, sal_Int32 nScaleY,
        const tools::Rectangle& rRect, MapUnit eMapUnit )
{
    double fScale = 1.0;
    switch( eMapUnit )
    {
        case MapUnit::MapTwip:      fScale = HMM_PER_TWIPS; break;  // Calc twips -> 1/100mm
        case MapUnit::Map100thMM:  fScale = 1.0;           break;  // Calc 1/100mm -> 1/100mm
        default:            OSL_FAIL( "XclObjAnchor::SetRect - map unit not implemented" );
    }

    /*  In objects with DFF client anchor, the position of the shape is stored
        in the cell address components of the client anchor. In old BIFF3-BIFF5
        objects, the position is stored in the offset components of the anchor. */
    maFirst.mnCol = lclGetEmbeddedScale( rPageSize.Width(),  nScaleX, rRect.Left(),   fScale );
    maFirst.mnRow = lclGetEmbeddedScale( rPageSize.Height(), nScaleY, rRect.Top(),    fScale );
    maLast.mnCol = lclGetEmbeddedScale( rPageSize.Width(),  nScaleX, rRect.Right(),  fScale );
    maLast.mnRow = lclGetEmbeddedScale( rPageSize.Height(), nScaleY, rRect.Bottom(), fScale );

    // for safety, clear the other members
    mnLX = mnTY = mnRX = mnBY = 0;
}

XclObjLineData::XclObjLineData() :
    mnColorIdx( EXC_OBJ_LINE_AUTOCOLOR ),
    mnStyle( EXC_OBJ_LINE_SOLID ),
    mnWidth( EXC_OBJ_LINE_HAIR ),
    mnAuto( EXC_OBJ_LINE_AUTO )
{
}

XclImpStream& operator>>( XclImpStream& rStrm, XclObjLineData& rLineData )
{
    rLineData.mnColorIdx = rStrm.ReaduInt8();
    rLineData.mnStyle = rStrm.ReaduInt8();
    rLineData.mnWidth = rStrm.ReaduInt8();
    rLineData.mnAuto = rStrm.ReaduInt8();
    return rStrm;
}

XclObjFillData::XclObjFillData() :
    mnBackColorIdx( EXC_OBJ_LINE_AUTOCOLOR ),
    mnPattColorIdx( EXC_OBJ_FILL_AUTOCOLOR ),
    mnPattern( EXC_PATT_SOLID ),
    mnAuto( EXC_OBJ_FILL_AUTO )
{
}

XclImpStream& operator>>( XclImpStream& rStrm, XclObjFillData& rFillData )
{
    rFillData.mnBackColorIdx = rStrm.ReaduInt8();
    rFillData.mnPattColorIdx = rStrm.ReaduInt8();
    rFillData.mnPattern = rStrm.ReaduInt8();
    rFillData.mnAuto = rStrm.ReaduInt8();
    return rStrm;
}

XclObjTextData::XclObjTextData() :
    mnTextLen( 0 ),
    mnFormatSize( 0 ),
    mnLinkSize( 0 ),
    mnDefFontIdx( EXC_FONT_APP ),
    mnFlags( 0 ),
    mnOrient( EXC_OBJ_ORIENT_NONE ),
    mnButtonFlags( 0 ),
    mnShortcut( 0 ),
    mnShortcutEA( 0 )
{
}

void XclObjTextData::ReadObj3( XclImpStream& rStrm )
{
    mnTextLen = rStrm.ReaduInt16();
    rStrm.Ignore( 2 );
    mnFormatSize = rStrm.ReaduInt16();
    mnDefFontIdx = rStrm.ReaduInt16();
    rStrm.Ignore( 2 );
    mnFlags = rStrm.ReaduInt16();
    mnOrient = rStrm.ReaduInt16();
    rStrm.Ignore( 8 );
}

void XclObjTextData::ReadObj5( XclImpStream& rStrm )
{
    mnTextLen = rStrm.ReaduInt16();
    rStrm.Ignore( 2 );
    mnFormatSize = rStrm.ReaduInt16();
    mnDefFontIdx = rStrm.ReaduInt16();
    rStrm.Ignore( 2 );
    mnFlags = rStrm.ReaduInt16();
    mnOrient = rStrm.ReaduInt16();
    rStrm.Ignore( 2 );
    mnLinkSize = rStrm.ReaduInt16();
    rStrm.Ignore( 2 );
    mnButtonFlags = rStrm.ReaduInt16();
    mnShortcut = rStrm.ReaduInt16();
    mnShortcutEA = rStrm.ReaduInt16();
}

void XclObjTextData::ReadTxo8( XclImpStream& rStrm )
{
    mnFlags = rStrm.ReaduInt16();
    mnOrient = rStrm.ReaduInt16();
    mnButtonFlags = rStrm.ReaduInt16();
    mnShortcut = rStrm.ReaduInt16();
    mnShortcutEA = rStrm.ReaduInt16();
    mnTextLen = rStrm.ReaduInt16();
    mnFormatSize = rStrm.ReaduInt16();
}

Reference< XControlModel > XclControlHelper::GetControlModel( Reference< XShape > const & xShape )
{
    Reference< XControlModel > xCtrlModel;
    Reference< XControlShape > xCtrlShape( xShape, UNO_QUERY );
    if( xCtrlShape.is() )
        xCtrlModel = xCtrlShape->getControl();
    return xCtrlModel;
}

namespace {

static const struct
{
    const sal_Char*     mpcListenerType;
    const sal_Char*     mpcEventMethod;
}
spTbxListenerData[] =
{
    // Attention: MUST be in order of the XclTbxEventType enum!
    /*EXC_TBX_EVENT_ACTION*/    { "XActionListener",     "actionPerformed"        },
    /*EXC_TBX_EVENT_MOUSE*/     { "XMouseListener",      "mouseReleased"          },
    /*EXC_TBX_EVENT_TEXT*/      { "XTextListener",       "textChanged"            },
    /*EXC_TBX_EVENT_VALUE*/     { "XAdjustmentListener", "adjustmentValueChanged" },
    /*EXC_TBX_EVENT_CHANGE*/    { "XChangeListener",     "changed"                }
};

} // namespace

bool XclControlHelper::FillMacroDescriptor( ScriptEventDescriptor& rDescriptor,
        XclTbxEventType eEventType, const OUString& rXclMacroName, SfxObjectShell* pDocShell )
{
    if( !rXclMacroName.isEmpty() )
    {
        rDescriptor.ListenerType = OUString::createFromAscii( spTbxListenerData[ eEventType ].mpcListenerType );
        rDescriptor.EventMethod = OUString::createFromAscii( spTbxListenerData[ eEventType ].mpcEventMethod );
        rDescriptor.ScriptType = "Script";
        rDescriptor.ScriptCode = XclTools::GetSbMacroUrl( rXclMacroName, pDocShell );
        return true;
    }
    return false;
}

OUString XclControlHelper::ExtractFromMacroDescriptor(
        const ScriptEventDescriptor& rDescriptor, XclTbxEventType eEventType )
{
    if( (!rDescriptor.ScriptCode.isEmpty()) &&
            rDescriptor.ScriptType.equalsIgnoreAsciiCase("Script") &&
            rDescriptor.ListenerType.equalsAscii( spTbxListenerData[ eEventType ].mpcListenerType ) &&
            rDescriptor.EventMethod.equalsAscii( spTbxListenerData[ eEventType ].mpcEventMethod ) )
        return XclTools::GetXclMacroName( rDescriptor.ScriptCode );
    return OUString();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
