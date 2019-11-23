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

#include <tools/vcompat.hxx>
#include <tools/stream.hxx>

#include <vcl/MetaBmpExScalePartAction.hxx>
#include <vcl/outdev.hxx>
#include <vcl/dibtools.hxx>

#include <TypeSerializer.hxx>

MetaBmpExScalePartAction::MetaBmpExScalePartAction() :
    MetaAction(MetaActionType::BMPEXSCALEPART)
{}

MetaBmpExScalePartAction::~MetaBmpExScalePartAction()
{}

MetaBmpExScalePartAction::MetaBmpExScalePartAction( const Point& rDstPt, const Size& rDstSz,
                                                    const Point& rSrcPt, const Size& rSrcSz,
                                                    const BitmapEx& rBmpEx ) :
    MetaAction  ( MetaActionType::BMPEXSCALEPART ),
    maBmpEx     ( rBmpEx ),
    maDstPt     ( rDstPt ),
    maDstSz     ( rDstSz ),
    maSrcPt     ( rSrcPt ),
    maSrcSz     ( rSrcSz )
{}

void MetaBmpExScalePartAction::Execute( OutputDevice* pOut )
{
    pOut->DrawBitmapEx( maDstPt, maDstSz, maSrcPt, maSrcSz, maBmpEx );
}

rtl::Reference<MetaAction> MetaBmpExScalePartAction::Clone()
{
    return new MetaBmpExScalePartAction( *this );
}

void MetaBmpExScalePartAction::Move( long nHorzMove, long nVertMove )
{
    maDstPt.Move( nHorzMove, nVertMove );
}

void MetaBmpExScalePartAction::Scale( double fScaleX, double fScaleY )
{
    tools::Rectangle aRectangle(maDstPt, maDstSz);
    aRectangle.Scale( fScaleX, fScaleY );
    maDstPt = aRectangle.TopLeft();
    maDstSz = aRectangle.GetSize();
}

void MetaBmpExScalePartAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    if( !!maBmpEx.GetBitmap() )
    {
        MetaAction::Write(rOStm, pData);
        VersionCompat aCompat(rOStm, StreamMode::WRITE, 1);
        WriteDIBBitmapEx(maBmpEx, rOStm);
        TypeSerializer aSerializer(rOStm);
        aSerializer.writePoint(maDstPt);
        aSerializer.writeSize(maDstSz);
        aSerializer.writePoint(maSrcPt);
        aSerializer.writeSize(maSrcSz);
    }
}

void MetaBmpExScalePartAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    VersionCompat aCompat(rIStm, StreamMode::READ);
    ReadDIBBitmapEx(maBmpEx, rIStm);
    TypeSerializer aSerializer(rIStm);
    aSerializer.readPoint(maDstPt);
    aSerializer.readSize(maDstSz);
    aSerializer.readPoint(maSrcPt);
    aSerializer.readSize(maSrcSz);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
