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

#include <sal/log.hxx>
#include <tools/stream.hxx>
#include <tools/vcompat.hxx>

#include <vcl/gdimtf.hxx>
#include <vcl/graphictools.hxx>
#include <vcl/outdev.hxx>
#include <vcl/MetaCommentAction.hxx>

MetaCommentAction::MetaCommentAction() :
    MetaAction  ( MetaActionType::COMMENT ),
    mnValue     ( 0 )
{
    ImplInitDynamicData( nullptr, 0UL );
}

MetaCommentAction::MetaCommentAction( const MetaCommentAction& rAct ) :
    MetaAction  ( MetaActionType::COMMENT ),
    maComment   ( rAct.maComment ),
    mnValue     ( rAct.mnValue )
{
    ImplInitDynamicData( rAct.mpData.get(), rAct.mnDataSize );
}

MetaCommentAction::MetaCommentAction( const OString& rComment, sal_Int32 nValue, const sal_uInt8* pData, sal_uInt32 nDataSize ) :
    MetaAction  ( MetaActionType::COMMENT ),
    maComment   ( rComment ),
    mnValue     ( nValue )
{
    ImplInitDynamicData( pData, nDataSize );
}

MetaCommentAction::~MetaCommentAction()
{
}

void MetaCommentAction::ImplInitDynamicData( const sal_uInt8* pData, sal_uInt32 nDataSize )
{
    if ( nDataSize && pData )
    {
        mnDataSize = nDataSize;
        mpData.reset( new sal_uInt8[ mnDataSize ] );
        memcpy( mpData.get(), pData, mnDataSize );
    }
    else
    {
        mnDataSize = 0;
        mpData = nullptr;
    }
}

void MetaCommentAction::Execute( OutputDevice* pOut )
{
    if ( pOut->GetConnectMetaFile() )
    {
        pOut->GetConnectMetaFile()->AddAction( this );
    }
}

rtl::Reference<MetaAction> MetaCommentAction::Clone()
{
    return new MetaCommentAction( *this );
}

void MetaCommentAction::Move( long nXMove, long nYMove )
{
    if ( nXMove || nYMove )
    {
        if ( mnDataSize && mpData )
        {
            bool bPathStroke = (maComment == "XPATHSTROKE_SEQ_BEGIN");
            if ( bPathStroke || maComment == "XPATHFILL_SEQ_BEGIN" )
            {
                SvMemoryStream  aMemStm( static_cast<void*>(mpData.get()), mnDataSize, StreamMode::READ );
                SvMemoryStream  aDest;
                if ( bPathStroke )
                {
                    SvtGraphicStroke aStroke;
                    ReadSvtGraphicStroke( aMemStm, aStroke );

                    tools::Polygon aPath;
                    aStroke.getPath( aPath );
                    aPath.Move( nXMove, nYMove );
                    aStroke.setPath( aPath );

                    tools::PolyPolygon aStartArrow;
                    aStroke.getStartArrow(aStartArrow);
                    aStartArrow.Move(nXMove, nYMove);
                    aStroke.setStartArrow(aStartArrow);

                    tools::PolyPolygon aEndArrow;
                    aStroke.getEndArrow(aEndArrow);
                    aEndArrow.Move(nXMove, nYMove);
                    aStroke.setEndArrow(aEndArrow);

                    WriteSvtGraphicStroke( aDest, aStroke );
                }
                else
                {
                    SvtGraphicFill aFill;
                    ReadSvtGraphicFill( aMemStm, aFill );

                    tools::PolyPolygon aPath;
                    aFill.getPath( aPath );
                    aPath.Move( nXMove, nYMove );
                    aFill.setPath( aPath );

                    WriteSvtGraphicFill( aDest, aFill );
                }
                mpData.reset();
                ImplInitDynamicData( static_cast<const sal_uInt8*>( aDest.GetData() ), aDest.Tell() );
            }
        }
    }
}

// SJ: 25.07.06 #i56656# we are not able to mirror certain kind of
// comments properly, especially the XPATHSTROKE and XPATHFILL lead to
// problems, so it is better to remove these comments when mirroring
// FIXME: fake comment to apply the next hunk in the right location
void MetaCommentAction::Scale( double fXScale, double fYScale )
{
    if ( ( fXScale != 1.0 ) || ( fYScale != 1.0 ) )
    {
        if ( mnDataSize && mpData )
        {
            bool bPathStroke = (maComment == "XPATHSTROKE_SEQ_BEGIN");
            if ( bPathStroke || maComment == "XPATHFILL_SEQ_BEGIN" )
            {
                SvMemoryStream  aMemStm( static_cast<void*>(mpData.get()), mnDataSize, StreamMode::READ );
                SvMemoryStream  aDest;
                if ( bPathStroke )
                {
                    SvtGraphicStroke aStroke;
                    ReadSvtGraphicStroke( aMemStm, aStroke );
                    aStroke.scale( fXScale, fYScale );
                    WriteSvtGraphicStroke( aDest, aStroke );
                }
                else
                {
                    SvtGraphicFill aFill;
                    ReadSvtGraphicFill( aMemStm, aFill );
                    tools::PolyPolygon aPath;
                    aFill.getPath( aPath );
                    aPath.Scale( fXScale, fYScale );
                    aFill.setPath( aPath );
                    WriteSvtGraphicFill( aDest, aFill );
                }
                mpData.reset();
                ImplInitDynamicData( static_cast<const sal_uInt8*>( aDest.GetData() ), aDest.Tell() );
            } else if( maComment == "EMF_PLUS_HEADER_INFO" ){
                SvMemoryStream  aMemStm( static_cast<void*>(mpData.get()), mnDataSize, StreamMode::READ );
                SvMemoryStream  aDest;

                sal_Int32 nLeft(0), nRight(0), nTop(0), nBottom(0);
                sal_Int32 nPixX(0), nPixY(0), nMillX(0), nMillY(0);
                float m11(0), m12(0), m21(0), m22(0), mdx(0), mdy(0);

                // read data
                aMemStm.ReadInt32( nLeft ).ReadInt32( nTop ).ReadInt32( nRight ).ReadInt32( nBottom );
                aMemStm.ReadInt32( nPixX ).ReadInt32( nPixY ).ReadInt32( nMillX ).ReadInt32( nMillY );
                aMemStm.ReadFloat( m11 ).ReadFloat( m12 ).ReadFloat( m21 ).ReadFloat( m22 ).ReadFloat( mdx ).ReadFloat( mdy );

                // add scale to the transformation
                m11 *= fXScale;
                m12 *= fXScale;
                m22 *= fYScale;
                m21 *= fYScale;

                // prepare new data
                aDest.WriteInt32( nLeft ).WriteInt32( nTop ).WriteInt32( nRight ).WriteInt32( nBottom );
                aDest.WriteInt32( nPixX ).WriteInt32( nPixY ).WriteInt32( nMillX ).WriteInt32( nMillY );
                aDest.WriteFloat( m11 ).WriteFloat( m12 ).WriteFloat( m21 ).WriteFloat( m22 ).WriteFloat( mdx ).WriteFloat( mdy );

                // save them
                ImplInitDynamicData( static_cast<const sal_uInt8*>( aDest.GetData() ), aDest.Tell() );
            }
        }
    }
}

void MetaCommentAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    MetaAction::Write(rOStm, pData);
    VersionCompat aCompat(rOStm, StreamMode::WRITE, 1);
    write_uInt16_lenPrefixed_uInt8s_FromOString(rOStm, maComment);
    rOStm.WriteInt32( mnValue ).WriteUInt32( mnDataSize );

    if ( mnDataSize )
        rOStm.WriteBytes( mpData.get(), mnDataSize );
}

void MetaCommentAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    VersionCompat aCompat(rIStm, StreamMode::READ);
    maComment = read_uInt16_lenPrefixed_uInt8s_ToOString(rIStm);
    rIStm.ReadInt32( mnValue ).ReadUInt32( mnDataSize );

    if (mnDataSize > rIStm.remainingSize())
    {
        SAL_WARN("vcl.gdi", "Parsing error: " << rIStm.remainingSize() <<
                 " available data, but " << mnDataSize << " claimed, truncating");
        mnDataSize = rIStm.remainingSize();
    }

    SAL_INFO("vcl.gdi", "MetaCommentAction::Read " << maComment);

    mpData.reset();

    if( mnDataSize )
    {
        mpData.reset(new sal_uInt8[ mnDataSize ]);
        rIStm.ReadBytes(mpData.get(), mnDataSize);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
