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

#include <osl/thread.h>
#include <sal/log.hxx>
#include <tools/stream.hxx>
#include <tools/vcompat.hxx>
#include <tools/helpers.hxx>
#include <unotools/fontdefs.hxx>

#include <vcl/dibtools.hxx>
#include <vcl/outdev.hxx>
#include <vcl/graphictools.hxx>
#include <vcl/MetaPixelAction.hxx>
#include <vcl/MetaPointAction.hxx>
#include <vcl/MetaLineAction.hxx>
#include <vcl/MetaRectAction.hxx>
#include <vcl/MetaRoundRectAction.hxx>
#include <vcl/MetaEllipseAction.hxx>
#include <vcl/MetaPieAction.hxx>
#include <vcl/MetaArcAction.hxx>
#include <vcl/MetaChordAction.hxx>
#include <vcl/MetaPolyLineAction.hxx>
#include <vcl/MetaPolygonAction.hxx>
#include <vcl/MetaPolyPolygonAction.hxx>
#include <vcl/MetaTextAction.hxx>
#include <vcl/MetaTextArrayAction.hxx>
#include <vcl/MetaStretchTextAction.hxx>
#include <vcl/MetaTextRectAction.hxx>
#include <vcl/MetaTextLineAction.hxx>
#include <vcl/MetaBmpAction.hxx>
#include <vcl/MetaBmpScaleAction.hxx>
#include <vcl/MetaBmpScalePartAction.hxx>
#include <vcl/MetaBmpExAction.hxx>
#include <vcl/MetaBmpExScaleAction.hxx>
#include <vcl/MetaBmpExScalePartAction.hxx>
#include <vcl/MetaMaskAction.hxx>
#include <vcl/MetaMaskScaleAction.hxx>
#include <vcl/MetaMaskScalePartAction.hxx>
#include <vcl/MetaGradientAction.hxx>
#include <vcl/MetaGradientExAction.hxx>
#include <vcl/MetaHatchAction.hxx>
#include <vcl/MetaWallpaperAction.hxx>
#include <vcl/MetaClipRegionAction.hxx>
#include <vcl/MetaISectRectClipRegionAction.hxx>
#include <vcl/MetaISectRegionClipRegionAction.hxx>
#include <vcl/MetaMoveClipRegionAction.hxx>
#include <vcl/MetaLineColorAction.hxx>
#include <vcl/MetaFillColorAction.hxx>
#include <vcl/MetaTextLanguageAction.hxx>
#include <vcl/MetaLayoutModeAction.hxx>
#include <vcl/MetaCommentAction.hxx>
#include <vcl/MetaRefPointAction.hxx>
#include <vcl/MetaEPSAction.hxx>
#include <vcl/MetaFloatTransparentAction.hxx>
#include <vcl/MetaTransparentAction.hxx>
#include <vcl/MetaRasterOpAction.hxx>
#include <vcl/MetaPopAction.hxx>
#include <vcl/MetaPushAction.hxx>
#include <vcl/MetaFontAction.hxx>
#include <vcl/MetaMapModeAction.hxx>
#include <vcl/MetaTextAlignAction.hxx>
#include <vcl/MetaOverlineColorAction.hxx>
#include <vcl/MetaTextLineColorAction.hxx>
#include <vcl/MetaTextFillColorAction.hxx>
#include <vcl/MetaTextColorAction.hxx>

#include <TypeSerializer.hxx>

#include <stdio.h>
#include <string.h>

namespace
{

const char *
meta_action_name(MetaActionType nMetaAction)
{
#ifndef SAL_LOG_INFO
    (void) nMetaAction;
    return "";
#else
    switch( nMetaAction )
    {
    case MetaActionType::NONE: return "NULL";
    case MetaActionType::PIXEL: return "PIXEL";
    case MetaActionType::POINT: return "POINT";
    case MetaActionType::LINE: return "LINE";
    case MetaActionType::RECT: return "RECT";
    case MetaActionType::ROUNDRECT: return "ROUNDRECT";
    case MetaActionType::ELLIPSE: return "ELLIPSE";
    case MetaActionType::ARC: return "ARC";
    case MetaActionType::PIE: return "PIE";
    case MetaActionType::CHORD: return "CHORD";
    case MetaActionType::POLYLINE: return "POLYLINE";
    case MetaActionType::POLYGON: return "POLYGON";
    case MetaActionType::POLYPOLYGON: return "POLYPOLYGON";
    case MetaActionType::TEXT: return "TEXT";
    case MetaActionType::TEXTARRAY: return "TEXTARRAY";
    case MetaActionType::STRETCHTEXT: return "STRETCHTEXT";
    case MetaActionType::TEXTRECT: return "TEXTRECT";
    case MetaActionType::BMP: return "BMP";
    case MetaActionType::BMPSCALE: return "BMPSCALE";
    case MetaActionType::BMPSCALEPART: return "BMPSCALEPART";
    case MetaActionType::BMPEX: return "BMPEX";
    case MetaActionType::BMPEXSCALE: return "BMPEXSCALE";
    case MetaActionType::BMPEXSCALEPART: return "BMPEXSCALEPART";
    case MetaActionType::MASK: return "MASK";
    case MetaActionType::MASKSCALE: return "MASKSCALE";
    case MetaActionType::MASKSCALEPART: return "MASKSCALEPART";
    case MetaActionType::GRADIENT: return "GRADIENT";
    case MetaActionType::HATCH: return "HATCH";
    case MetaActionType::WALLPAPER: return "WALLPAPER";
    case MetaActionType::CLIPREGION: return "CLIPREGION";
    case MetaActionType::ISECTRECTCLIPREGION: return "ISECTRECTCLIPREGION";
    case MetaActionType::ISECTREGIONCLIPREGION: return "ISECTREGIONCLIPREGION";
    case MetaActionType::MOVECLIPREGION: return "MOVECLIPREGION";
    case MetaActionType::LINECOLOR: return "LINECOLOR";
    case MetaActionType::FILLCOLOR: return "FILLCOLOR";
    case MetaActionType::TEXTCOLOR: return "TEXTCOLOR";
    case MetaActionType::TEXTFILLCOLOR: return "TEXTFILLCOLOR";
    case MetaActionType::TEXTALIGN: return "TEXTALIGN";
    case MetaActionType::MAPMODE: return "MAPMODE";
    case MetaActionType::FONT: return "FONT";
    case MetaActionType::PUSH: return "PUSH";
    case MetaActionType::POP: return "POP";
    case MetaActionType::RASTEROP: return "RASTEROP";
    case MetaActionType::Transparent: return "TRANSPARENT";
    case MetaActionType::EPS: return "EPS";
    case MetaActionType::REFPOINT: return "REFPOINT";
    case MetaActionType::TEXTLINECOLOR: return "TEXTLINECOLOR";
    case MetaActionType::TEXTLINE: return "TEXTLINE";
    case MetaActionType::FLOATTRANSPARENT: return "FLOATTRANSPARENT";
    case MetaActionType::GRADIENTEX: return "GRADIENTEX";
    case MetaActionType::LAYOUTMODE: return "LAYOUTMODE";
    case MetaActionType::TEXTLANGUAGE: return "TEXTLANGUAGE";
    case MetaActionType::OVERLINECOLOR: return "OVERLINECOLOR";
    case MetaActionType::COMMENT: return "COMMENT";
    default:
        // Yes, return a pointer to a static buffer. This is a very
        // local debugging output function, so no big deal.
        static char buffer[6];
        sprintf(buffer, "%u", static_cast<unsigned int>(nMetaAction));
        return buffer;
    }
#endif
}

} //anonymous namespace

void ImplScalePoly( tools::Polygon& rPoly, double fScaleX, double fScaleY )
{
    for( sal_uInt16 i = 0, nCount = rPoly.GetSize(); i < nCount; i++ )
        rPoly[i].Scale( fScaleX, fScaleY );
}

MetaAction::MetaAction() :
    mnType( MetaActionType::NONE )
{
}

MetaAction::MetaAction( MetaActionType nType ) :
    mnType( nType )
{
}

MetaAction::MetaAction( MetaAction const & rOther ) :
    SimpleReferenceObject(), mnType( rOther.mnType )
{
}

MetaAction::~MetaAction()
{
}

void MetaAction::Execute( OutputDevice* )
{
}

rtl::Reference<MetaAction> MetaAction::Clone()
{
    return new MetaAction;
}

void MetaAction::Move( long, long )
{
}

void MetaAction::Scale( double, double )
{
}

void MetaAction::Write( SvStream& rOStm, ImplMetaWriteData* )
{
    rOStm.WriteUInt16( static_cast<sal_uInt16>(mnType) );
}

void MetaAction::Read( SvStream&, ImplMetaReadData* )
{
    // DO NOT read mnType - ReadMetaAction already did that!
}

MetaAction* MetaAction::ReadMetaAction( SvStream& rIStm, ImplMetaReadData* pData )
{
    MetaAction* pAction = nullptr;
    sal_uInt16 nTmp = 0;
    rIStm.ReadUInt16( nTmp );
    MetaActionType nType = static_cast<MetaActionType>(nTmp);

    SAL_INFO("vcl.gdi", "ReadMetaAction " << meta_action_name( nType ));

    switch( nType )
    {
        case MetaActionType::NONE: pAction = new MetaAction; break;
        case MetaActionType::PIXEL: pAction = new MetaPixelAction; break;
        case MetaActionType::POINT: pAction = new MetaPointAction; break;
        case MetaActionType::LINE: pAction = new MetaLineAction; break;
        case MetaActionType::RECT: pAction = new MetaRectAction; break;
        case MetaActionType::ROUNDRECT: pAction = new MetaRoundRectAction; break;
        case MetaActionType::ELLIPSE: pAction = new MetaEllipseAction; break;
        case MetaActionType::ARC: pAction = new MetaArcAction; break;
        case MetaActionType::PIE: pAction = new MetaPieAction; break;
        case MetaActionType::CHORD: pAction = new MetaChordAction; break;
        case MetaActionType::POLYLINE: pAction = new MetaPolyLineAction; break;
        case MetaActionType::POLYGON: pAction = new MetaPolygonAction; break;
        case MetaActionType::POLYPOLYGON: pAction = new MetaPolyPolygonAction; break;
        case MetaActionType::TEXT: pAction = new MetaTextAction; break;
        case MetaActionType::TEXTARRAY: pAction = new MetaTextArrayAction; break;
        case MetaActionType::STRETCHTEXT: pAction = new MetaStretchTextAction; break;
        case MetaActionType::TEXTRECT: pAction = new MetaTextRectAction; break;
        case MetaActionType::TEXTLINE: pAction = new MetaTextLineAction; break;
        case MetaActionType::BMP: pAction = new MetaBmpAction; break;
        case MetaActionType::BMPSCALE: pAction = new MetaBmpScaleAction; break;
        case MetaActionType::BMPSCALEPART: pAction = new MetaBmpScalePartAction; break;
        case MetaActionType::BMPEX: pAction = new MetaBmpExAction; break;
        case MetaActionType::BMPEXSCALE: pAction = new MetaBmpExScaleAction; break;
        case MetaActionType::BMPEXSCALEPART: pAction = new MetaBmpExScalePartAction; break;
        case MetaActionType::MASK: pAction = new MetaMaskAction; break;
        case MetaActionType::MASKSCALE: pAction = new MetaMaskScaleAction; break;
        case MetaActionType::MASKSCALEPART: pAction = new MetaMaskScalePartAction; break;
        case MetaActionType::GRADIENT: pAction = new MetaGradientAction; break;
        case MetaActionType::GRADIENTEX: pAction = new MetaGradientExAction; break;
        case MetaActionType::HATCH: pAction = new MetaHatchAction; break;
        case MetaActionType::WALLPAPER: pAction = new MetaWallpaperAction; break;
        case MetaActionType::CLIPREGION: pAction = new MetaClipRegionAction; break;
        case MetaActionType::ISECTRECTCLIPREGION: pAction = new MetaISectRectClipRegionAction; break;
        case MetaActionType::ISECTREGIONCLIPREGION: pAction = new MetaISectRegionClipRegionAction; break;
        case MetaActionType::MOVECLIPREGION: pAction = new MetaMoveClipRegionAction; break;
        case MetaActionType::LINECOLOR: pAction = new MetaLineColorAction; break;
        case MetaActionType::FILLCOLOR: pAction = new MetaFillColorAction; break;
        case MetaActionType::TEXTCOLOR: pAction = new MetaTextColorAction; break;
        case MetaActionType::TEXTFILLCOLOR: pAction = new MetaTextFillColorAction; break;
        case MetaActionType::TEXTLINECOLOR: pAction = new MetaTextLineColorAction; break;
        case MetaActionType::OVERLINECOLOR: pAction = new MetaOverlineColorAction; break;
        case MetaActionType::TEXTALIGN: pAction = new MetaTextAlignAction; break;
        case MetaActionType::MAPMODE: pAction = new MetaMapModeAction; break;
        case MetaActionType::FONT: pAction = new MetaFontAction; break;
        case MetaActionType::PUSH: pAction = new MetaPushAction; break;
        case MetaActionType::POP: pAction = new MetaPopAction; break;
        case MetaActionType::RASTEROP: pAction = new MetaRasterOpAction; break;
        case MetaActionType::Transparent: pAction = new MetaTransparentAction; break;
        case MetaActionType::FLOATTRANSPARENT: pAction = new MetaFloatTransparentAction; break;
        case MetaActionType::EPS: pAction = new MetaEPSAction; break;
        case MetaActionType::REFPOINT: pAction = new MetaRefPointAction; break;
        case MetaActionType::COMMENT: pAction = new MetaCommentAction; break;
        case MetaActionType::LAYOUTMODE: pAction = new MetaLayoutModeAction; break;
        case MetaActionType::TEXTLANGUAGE: pAction = new MetaTextLanguageAction; break;

        default:
        {
            VersionCompat aCompat(rIStm, StreamMode::READ);
        }
        break;
    }

    if( pAction )
        pAction->Read( rIStm, pData );

    return pAction;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
