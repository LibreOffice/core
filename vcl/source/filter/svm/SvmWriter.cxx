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

#include <vcl/filter/SvmWriter.hxx>
#include <vcl/TypeSerializer.hxx>

#include <tools/vcompat.hxx>

SvmWriter::SvmWriter(SvStream& rIStm)
    : mrStream(rIStm)
{
}

void SvmWriter::WriteColor(::Color aColor)
{
    mrStream.WriteUInt32(static_cast<sal_uInt32>(aColor));
}

SvStream& SvmWriter::Write(GDIMetaFile& rMetaFile)
{
    const SvStreamCompressFlags nStmCompressMode = mrStream.GetCompressMode();
    SvStreamEndian nOldFormat = mrStream.GetEndian();

    mrStream.SetEndian(SvStreamEndian::LITTLE);
    mrStream.WriteBytes("VCLMTF", 6);

    {
        VersionCompatWrite aCompat(mrStream, 1);

        mrStream.WriteUInt32(static_cast<sal_uInt32>(nStmCompressMode));
        TypeSerializer aSerializer(mrStream);
        aSerializer.writeMapMode(rMetaFile.GetPrefMapMode());
        aSerializer.writeSize(rMetaFile.GetPrefSize());
        mrStream.WriteUInt32(rMetaFile.GetActionSize());
    } // VersionCompatWrite dtor writes stuff into the header

    ImplMetaWriteData aWriteData;

    aWriteData.meActualCharSet = mrStream.GetStreamCharSet();

    MetaAction* pAct = rMetaFile.FirstAction();
    while (pAct)
    {
        MetaActionHandler(pAct, &aWriteData);
        pAct = rMetaFile.NextAction();
    }

    mrStream.SetEndian(nOldFormat);

    return mrStream;
}

void SvmWriter::MetaActionHandler(MetaAction* pAction, ImplMetaWriteData* pData)
{
    MetaActionType nType = pAction->GetType();

    switch (nType)
    {
        case MetaActionType::NONE:
        {
            ActionHandler(pAction);
        }
        break;

        case MetaActionType::PIXEL:
        {
            auto* pMetaAction = static_cast<MetaPixelAction*>(pAction);
            PixelHandler(pMetaAction);
        }
        break;

        case MetaActionType::POINT:
        {
            auto pMetaAction = static_cast<MetaPointAction*>(pAction);
            PointHandler(pMetaAction);
        }
        break;

        case MetaActionType::LINE:
        {
            auto* pMetaAction = static_cast<MetaLineAction*>(pAction);
            LineHandler(pMetaAction);
        }
        break;

        case MetaActionType::RECT:
        {
            auto* pMetaAction = static_cast<MetaRectAction*>(pAction);
            RectHandler(pMetaAction);
        }
        break;

        case MetaActionType::ROUNDRECT:
        {
            auto* pMetaAction = static_cast<MetaRoundRectAction*>(pAction);
            RoundRectHandler(pMetaAction);
        }
        break;

        case MetaActionType::ELLIPSE:
        {
            auto* pMetaAction = static_cast<MetaEllipseAction*>(pAction);
            EllipseHandler(pMetaAction);
        }
        break;

        case MetaActionType::ARC:
        {
            auto* pMetaAction = static_cast<MetaArcAction*>(pAction);
            ArcHandler(pMetaAction);
        }
        break;

        case MetaActionType::PIE:
        {
            auto* pMetaAction = static_cast<MetaPieAction*>(pAction);
            PieHandler(pMetaAction);
        }
        break;

        case MetaActionType::CHORD:
        {
            auto* pMetaAction = static_cast<MetaChordAction*>(pAction);
            ChordHandler(pMetaAction);
        }
        break;

        case MetaActionType::POLYLINE:
        {
            auto* pMetaAction = static_cast<MetaPolyLineAction*>(pAction);
            PolyLineHandler(pMetaAction);
        }
        break;

        case MetaActionType::POLYGON:
        {
            auto* pMetaAction = static_cast<MetaPolygonAction*>(pAction);
            PolygonHandler(pMetaAction);
        }
        break;

        case MetaActionType::POLYPOLYGON:
        {
            auto* pMetaAction = static_cast<MetaPolyPolygonAction*>(pAction);
            PolyPolygonHandler(pMetaAction);
        }
        break;

        /* default case prevents test failure and will be
        removed once all the handlers are completed */
        default:
            pAction->Write(mrStream, pData);
    }
}

void SvmWriter::ActionHandler(MetaAction* pAction)
{
    mrStream.WriteUInt16(static_cast<sal_uInt16>(pAction->GetType()));
}

void SvmWriter::PixelHandler(MetaPixelAction* pAction)
{
    mrStream.WriteUInt16(static_cast<sal_uInt16>(pAction->GetType()));
    VersionCompatWrite aCompat(mrStream, 1);
    TypeSerializer aSerializer(mrStream);
    aSerializer.writePoint(pAction->GetPoint());
    WriteColor(pAction->GetColor());
}

void SvmWriter::PointHandler(MetaPointAction* pAction)
{
    mrStream.WriteUInt16(static_cast<sal_uInt16>(pAction->GetType()));
    VersionCompatWrite aCompat(mrStream, 1);
    TypeSerializer aSerializer(mrStream);
    aSerializer.writePoint(pAction->GetPoint());
}

void SvmWriter::LineHandler(MetaLineAction* pAction)
{
    mrStream.WriteUInt16(static_cast<sal_uInt16>(pAction->GetType()));

    VersionCompatWrite aCompat(mrStream, 2);

    // Version 1
    TypeSerializer aSerializer(mrStream);
    aSerializer.writePoint(pAction->GetStartPoint());
    aSerializer.writePoint(pAction->GetEndPoint());
    // Version 2
    WriteLineInfo(mrStream, pAction->GetLineInfo());
}

void SvmWriter::RectHandler(MetaRectAction* pAction)
{
    mrStream.WriteUInt16(static_cast<sal_uInt16>(pAction->GetType()));

    VersionCompatWrite aCompat(mrStream, 1);
    TypeSerializer aSerializer(mrStream);
    aSerializer.writeRectangle(pAction->GetRect());
}

void SvmWriter::RoundRectHandler(MetaRoundRectAction* pAction)
{
    mrStream.WriteUInt16(static_cast<sal_uInt16>(pAction->GetType()));

    VersionCompatWrite aCompat(mrStream, 1);
    TypeSerializer aSerializer(mrStream);
    aSerializer.writeRectangle(pAction->GetRect());
    mrStream.WriteUInt32(pAction->GetHorzRound()).WriteUInt32(pAction->GetVertRound());
}

void SvmWriter::EllipseHandler(MetaEllipseAction* pAction)
{
    mrStream.WriteUInt16(static_cast<sal_uInt16>(pAction->GetType()));

    VersionCompatWrite aCompat(mrStream, 1);
    TypeSerializer aSerializer(mrStream);
    aSerializer.writeRectangle(pAction->GetRect());
}

void SvmWriter::ArcHandler(MetaArcAction* pAction)
{
    mrStream.WriteUInt16(static_cast<sal_uInt16>(pAction->GetType()));

    VersionCompatWrite aCompat(mrStream, 1);
    TypeSerializer aSerializer(mrStream);
    aSerializer.writeRectangle(pAction->GetRect());
    aSerializer.writePoint(pAction->GetStartPoint());
    aSerializer.writePoint(pAction->GetEndPoint());
}

void SvmWriter::PieHandler(MetaPieAction* pAction)
{
    mrStream.WriteUInt16(static_cast<sal_uInt16>(pAction->GetType()));

    VersionCompatWrite aCompat(mrStream, 1);
    TypeSerializer aSerializer(mrStream);
    aSerializer.writeRectangle(pAction->GetRect());
    aSerializer.writePoint(pAction->GetStartPoint());
    aSerializer.writePoint(pAction->GetEndPoint());
}

void SvmWriter::ChordHandler(MetaChordAction* pAction)
{
    mrStream.WriteUInt16(static_cast<sal_uInt16>(pAction->GetType()));

    VersionCompatWrite aCompat(mrStream, 1);
    TypeSerializer aSerializer(mrStream);
    aSerializer.writeRectangle(pAction->GetRect());
    aSerializer.writePoint(pAction->GetStartPoint());
    aSerializer.writePoint(pAction->GetEndPoint());
}

void SvmWriter::PolyLineHandler(MetaPolyLineAction* pAction)
{
    mrStream.WriteUInt16(static_cast<sal_uInt16>(pAction->GetType()));

    VersionCompatWrite aCompat(mrStream, 3);

    tools::Polygon aSimplePoly;
    pAction->GetPolygon().AdaptiveSubdivide(aSimplePoly);

    WritePolygon(mrStream, aSimplePoly); // Version 1
    WriteLineInfo(mrStream, pAction->GetLineInfo()); // Version 2

    bool bHasPolyFlags = pAction->GetPolygon().HasFlags(); // Version 3
    mrStream.WriteBool(bHasPolyFlags);
    if (bHasPolyFlags)
        pAction->GetPolygon().Write(mrStream);
}

void SvmWriter::PolygonHandler(MetaPolygonAction* pAction)
{
    mrStream.WriteUInt16(static_cast<sal_uInt16>(pAction->GetType()));

    VersionCompatWrite aCompat(mrStream, 2);

    tools::Polygon aSimplePoly; // Version 1
    pAction->GetPolygon().AdaptiveSubdivide(aSimplePoly);
    WritePolygon(mrStream, aSimplePoly);

    bool bHasPolyFlags = pAction->GetPolygon().HasFlags(); // Version 2
    mrStream.WriteBool(bHasPolyFlags);
    if (bHasPolyFlags)
        pAction->GetPolygon().Write(mrStream);
}

void SvmWriter::PolyPolygonHandler(MetaPolyPolygonAction* pAction)
{
    mrStream.WriteUInt16(static_cast<sal_uInt16>(pAction->GetType()));

    VersionCompatWrite aCompat(mrStream, 2);

    sal_uInt16 nNumberOfComplexPolygons = 0;
    sal_uInt16 i, nPolyCount = pAction->GetPolyPolygon().Count();

    tools::Polygon aSimplePoly; // Version 1
    mrStream.WriteUInt16(nPolyCount);
    for (i = 0; i < nPolyCount; i++)
    {
        const tools::Polygon& rPoly = pAction->GetPolyPolygon().GetObject(i);
        if (rPoly.HasFlags())
            nNumberOfComplexPolygons++;
        rPoly.AdaptiveSubdivide(aSimplePoly);
        WritePolygon(mrStream, aSimplePoly);
    }

    mrStream.WriteUInt16(nNumberOfComplexPolygons); // Version 2
    for (i = 0; nNumberOfComplexPolygons && (i < nPolyCount); i++)
    {
        const tools::Polygon& rPoly = pAction->GetPolyPolygon().GetObject(i);
        if (rPoly.HasFlags())
        {
            mrStream.WriteUInt16(i);
            rPoly.Write(mrStream);

            nNumberOfComplexPolygons--;
        }
    }
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
