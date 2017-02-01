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

#include <com/sun/star/embed/Aspects.hpp>

#include <math.h>
#include <limits.h>
#include <vector>
#include <osl/endian.h>
#include <osl/file.hxx>
#include <tools/solar.h>
#include <rtl/math.hxx>

#include <comphelper/classids.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <unotools/streamwrap.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>
#include <comphelper/seqstream.hxx>
#include <comphelper/storagehelper.hxx>
#include <comphelper/sequence.hxx>
#include <sot/exchange.hxx>
#include <sot/storinfo.hxx>
#include <vcl/cvtgrf.hxx>
#include <vcl/wmf.hxx>
#include <vcl/settings.hxx>
#include "viscache.hxx"

// SvxItem-Mapping. Is needed to successfully include the SvxItem-Header
#include <editeng/eeitem.hxx>
#include <editeng/editdata.hxx>
#include <svl/urihelper.hxx>
#include <tools/stream.hxx>
#include <tools/debug.hxx>
#include <tools/zcodec.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <filter/msfilter/escherex.hxx>
#include <basegfx/range/b2drange.hxx>
#include <com/sun/star/container/XIdentifierContainer.hpp>
#include <com/sun/star/drawing/XGluePointsSupplier.hpp>
#include <com/sun/star/drawing/Position3D.hpp>
#include <com/sun/star/drawing/Direction3D.hpp>
#include <com/sun/star/drawing/GluePoint2.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <editeng/charscaleitem.hxx>
#include <editeng/kernitem.hxx>
#include <vcl/graphicfilter.hxx>
#include <tools/urlobj.hxx>
#include <vcl/virdev.hxx>
#include <vcl/bitmapaccess.hxx>
#include <sot/storage.hxx>
#include <sfx2/docfac.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/fcontnr.hxx>
#include <sfx2/module.hxx>
#include <svx/sdgcpitm.hxx>
#include <svx/sdgmoitm.hxx>
#include <editeng/tstpitem.hxx>
#include <svx/fmmodel.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdogrp.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdotext.hxx>
#include <svx/svdorect.hxx>
#include <svx/svdocapt.hxx>
#include <svx/svdoedge.hxx>
#include <svx/svdocirc.hxx>
#include <svx/svdoutl.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdopath.hxx>
#include <editeng/frmdir.hxx>
#include <editeng/frmdiritem.hxx>
#include <svx/svdtrans.hxx>
#include <svx/sxenditm.hxx>
#include <svx/sdgluitm.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/crossedoutitem.hxx>
#include <editeng/shdditem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/colritem.hxx>
#include <svx/sxekitm.hxx>
#include <editeng/bulletitem.hxx>
#include <svx/polysc3d.hxx>
#include <svx/extrud3d.hxx>
#include "svx/svditer.hxx"
#include <svx/xpoly.hxx>
#include "svx/xattr.hxx"
#include <filter/msfilter/classids.hxx>
#include <filter/msfilter/msdffimp.hxx>
#include <editeng/outliner.hxx>
#include <editeng/outlobj.hxx>
#include <editeng/editobj.hxx>
#include <editeng/editeng.hxx>
#include "svx/gallery.hxx"
#include <com/sun/star/drawing/ShadeMode.hpp>
#include <svl/itempool.hxx>
#include <vcl/dibtools.hxx>
#include <vcl/svapp.hxx>
#include <svx/svx3ditems.hxx>
#include <svx/svdoashp.hxx>
#include <svx/sdasaitm.hxx>
#include "svx/EnhancedCustomShapeTypeNames.hxx"
#include "svx/EnhancedCustomShapeGeometry.hxx"
#include <com/sun/star/drawing/EnhancedCustomShapeParameterPair.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeParameterType.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeSegment.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeGluePointType.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeSegmentCommand.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeTextFrame.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeAdjustmentValue.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeTextPathMode.hpp>
#include <com/sun/star/beans/PropertyValues.hpp>
#include <com/sun/star/drawing/ProjectionMode.hpp>
#include "svx/EnhancedCustomShape2d.hxx"
#include <svx/xbitmap.hxx>
#include <rtl/strbuf.hxx>
#include <rtl/ustring.hxx>
#include <svtools/embedhlp.hxx>
#include <memory>
#include <o3tl/make_unique.hxx>

using namespace ::com::sun::star    ;
using namespace ::com::sun::star::drawing;
using namespace uno                 ;
using namespace beans               ;
using namespace drawing             ;
using namespace container           ;

// static counter for OLE-Objects
static sal_uInt32 nMSOleObjCntr = 0;
#define MSO_OLE_Obj "MSO_OLE_Obj"

struct SvxMSDffBLIPInfo
{
    sal_uLong  nFilePos;    ///< offset of the BLIP in data strem
    explicit SvxMSDffBLIPInfo(sal_uLong nFPos)
        : nFilePos(nFPos)
    {
    }
};

/// the following will be sorted by the order of their appearance:
struct SvxMSDffBLIPInfos : public std::vector<SvxMSDffBLIPInfo> {};

/************************************************************************/
void Impl_OlePres::Write( SvStream & rStm )
{
    WriteClipboardFormat( rStm, SotClipboardFormatId::GDIMETAFILE );
    rStm.WriteInt32( 4 );       // a TargetDevice that's always empty
    rStm.WriteUInt32( nAspect );
    rStm.WriteInt32( -1 );      //L-Index always -1
    rStm.WriteInt32( nAdvFlags );
    rStm.WriteInt32( 0 );       //Compression
    rStm.WriteInt32( aSize.Width() );
    rStm.WriteInt32( aSize.Height() );
    sal_uLong nPos = rStm.Tell();
    rStm.WriteInt32( 0 );

    if( nFormat == SotClipboardFormatId::GDIMETAFILE && pMtf )
    {
        // Always to 1/100 mm, until Mtf-Solution found
        // Assumption (no scaling, no origin translation)
        DBG_ASSERT( pMtf->GetPrefMapMode().GetScaleX() == Fraction( 1, 1 ),
                    "X-Skalierung im Mtf" );
        DBG_ASSERT( pMtf->GetPrefMapMode().GetScaleY() == Fraction( 1, 1 ),
                    "Y-Skalierung im Mtf" );
        DBG_ASSERT( pMtf->GetPrefMapMode().GetOrigin() == Point(),
                    "Origin-Verschiebung im Mtf" );
        MapUnit nMU = pMtf->GetPrefMapMode().GetMapUnit();
        if( MapUnit::Map100thMM != nMU )
        {
            Size aPrefS( pMtf->GetPrefSize() );
            Size aS( aPrefS );
            aS = OutputDevice::LogicToLogic( aS, nMU, MapUnit::Map100thMM );

            pMtf->Scale( Fraction( aS.Width(), aPrefS.Width() ),
                         Fraction( aS.Height(), aPrefS.Height() ) );
            pMtf->SetPrefMapMode( MapUnit::Map100thMM );
            pMtf->SetPrefSize( aS );
        }
        WriteWindowMetafileBits( rStm, *pMtf );
    }
    else
    {
        OSL_FAIL( "unknown format" );
    }
    sal_uLong nEndPos = rStm.Tell();
    rStm.Seek( nPos );
    rStm.WriteUInt32( nEndPos - nPos - 4 );
    rStm.Seek( nEndPos );
}

DffPropertyReader::DffPropertyReader( const SvxMSDffManager& rMan )
    : rManager(rMan)
    , pDefaultPropSet(nullptr)
    , mnFix16Angle(0)
    , mbRotateGranientFillWithAngle(false)
{
    InitializePropSet( DFF_msofbtOPT );
}

void DffPropertyReader::SetDefaultPropSet( SvStream& rStCtrl, sal_uInt32 nOffsDgg ) const
{
    const_cast<DffPropertyReader*>(this)->pDefaultPropSet.reset();
    sal_uInt32 nMerk = rStCtrl.Tell();
    rStCtrl.Seek( nOffsDgg );
    DffRecordHeader aRecHd;
    bool bOk = ReadDffRecordHeader( rStCtrl, aRecHd );
    if (bOk && aRecHd.nRecType == DFF_msofbtDggContainer)
    {
        if ( SvxMSDffManager::SeekToRec( rStCtrl, DFF_msofbtOPT, aRecHd.GetRecEndFilePos() ) )
        {
            const_cast<DffPropertyReader*>(this)->pDefaultPropSet.reset( new DffPropSet );
            ReadDffPropSet( rStCtrl, *pDefaultPropSet );
        }
    }
    rStCtrl.Seek( nMerk );
}

#ifdef DBG_CUSTOMSHAPE
void DffPropertyReader::ReadPropSet( SvStream& rIn, void* pClientData, sal_uInt32 nShapeId ) const
#else
void DffPropertyReader::ReadPropSet( SvStream& rIn, void* pClientData ) const
#endif
{
    sal_uLong nFilePos = rIn.Tell();
    ReadDffPropSet( rIn, (DffPropertyReader&)*this );

    if ( IsProperty( DFF_Prop_hspMaster ) )
    {
        if ( rManager.SeekToShape( rIn, pClientData, GetPropertyValue( DFF_Prop_hspMaster, 0 ) ) )
        {
            DffRecordHeader aRecHd;
            bool bOk = ReadDffRecordHeader(rIn, aRecHd);
            if (bOk && SvxMSDffManager::SeekToRec(rIn, DFF_msofbtOPT, aRecHd.GetRecEndFilePos()))
            {
                rIn |= (DffPropertyReader&)*this;
            }
        }
    }

    const_cast<DffPropertyReader*>(this)->mnFix16Angle = Fix16ToAngle( GetPropertyValue( DFF_Prop_Rotation, 0 ) );

#ifdef DBG_CUSTOMSHAPE

    OUString aURLStr;

    if( osl::FileBase::getFileURLFromSystemPath( OUString("d:\\ashape.dbg"), aURLStr ) == osl::FileBase::E_None )
    {
        std::unique_ptr<SvStream> xOut(::utl::UcbStreamHelper::CreateStream( aURLStr, StreamMode::WRITE ));

        if( xOut )
        {
            xOut->Seek( STREAM_SEEK_TO_END );

            if ( IsProperty( DFF_Prop_adjustValue ) || IsProperty( DFF_Prop_pVertices ) )
            {
                xOut->WriteLine( "" );
                OString aString("ShapeId: " + OString::number(nShapeId));
                xOut->WriteLine(aString);
            }
            for ( sal_uInt32 i = DFF_Prop_adjustValue; i <= DFF_Prop_adjust10Value; i++ )
            {
                if ( IsProperty( i ) )
                {
                    OString aString("Prop_adjustValue" + OString::number( ( i - DFF_Prop_adjustValue ) + 1 ) +
                                    ":" + OString::number(GetPropertyValue(i)) );
                    xOut->WriteLine(aString);
                }
            }
            sal_Int32 i;
            for ( i = 320; i < 383; i++ )
            {
                if ( ( i >= DFF_Prop_adjustValue ) && ( i <= DFF_Prop_adjust10Value ) )
                    continue;
                if ( IsProperty( i ) )
                {
                    if ( SeekToContent( i, rIn ) )
                    {
                        sal_Int32 nLen = (sal_Int32)GetPropertyValue( i );
                        if ( nLen )
                        {
                            xOut->WriteLine( "" );
                            OStringBuffer aDesc("Property:" + OString::number(i) +
                                                "  Size:" + OString::number(nLen));
                            xOut->WriteLine(aDesc.makeStringAndClear());
                            sal_Int16   nNumElem, nNumElemMem, nNumSize;
                            rIn >> nNumElem >> nNumElemMem >> nNumSize;
                            aDesc.append("Entries: " + OString::number(nNumElem) +
                                         "  Size:" + OString::number(nNumSize));
                            xOut->WriteLine(aDesc.makeStringAndClear());
                            if ( nNumSize < 0 )
                                nNumSize = ( ( -nNumSize ) >> 2 );
                            if ( !nNumSize )
                                nNumSize = 16;
                            nLen -= 6;
                            while ( nLen > 0 )
                            {
                                for ( sal_uInt32 j = 0; nLen && ( j < ( nNumSize >> 1 ) ); j++ )
                                {
                                    for ( sal_uInt32 k = 0; k < 2; k++ )
                                    {
                                        if ( nLen )
                                        {
                                            sal_uInt8 nVal;
                                            rIn >> nVal;
                                            if ( ( nVal >> 4 ) > 9 )
                                                *xOut << (sal_uInt8)( ( nVal >> 4 ) + 'A' - 10 );
                                            else
                                                *xOut << (sal_uInt8)( ( nVal >> 4 ) + '0' );

                                            if ( ( nVal & 0xf ) > 9 )
                                                *xOut << (sal_uInt8)( ( nVal & 0xf ) + 'A' - 10 );
                                            else
                                                *xOut << (sal_uInt8)( ( nVal & 0xf ) + '0' );

                                            nLen--;
                                        }
                                    }
                                    *xOut << (char)( ' ' );
                                }
                                xOut->WriteLine( OString() );
                            }
                        }
                    }
                    else
                    {
                        OString aString("Property" + OString::number(i) +
                                        ":" + OString::number(GetPropertyValue(i)));
                        xOut->WriteLine(aString);
                    }
                }
            }
        }
    }

#endif

    rIn.Seek( nFilePos );
}


sal_Int32 DffPropertyReader::Fix16ToAngle( sal_Int32 nContent )
{
    sal_Int32 nAngle = 0;
    if ( nContent )
    {
        nAngle = ( (sal_Int16)( nContent >> 16) * 100L ) + ( ( ( nContent & 0x0000ffff) * 100L ) >> 16 );
        nAngle = NormAngle360( -nAngle );
    }
    return nAngle;
}

DffPropertyReader::~DffPropertyReader()
{
}


SvStream& operator>>( SvStream& rIn, SvxMSDffConnectorRule& rRule )
{
    rIn.ReadUInt32( rRule.nRuleId )
       .ReadUInt32( rRule.nShapeA )
       .ReadUInt32( rRule.nShapeB )
       .ReadUInt32( rRule.nShapeC )
       .ReadUInt32( rRule.ncptiA )
       .ReadUInt32( rRule.ncptiB );

    return rIn;
}

SvxMSDffSolverContainer::SvxMSDffSolverContainer()
{
}

SvxMSDffSolverContainer::~SvxMSDffSolverContainer()
{
    for(SvxMSDffConnectorRule* i : aCList) {
        delete i;
    }
    aCList.clear();
}

SvStream& ReadSvxMSDffSolverContainer( SvStream& rIn, SvxMSDffSolverContainer& rContainer )
{
    DffRecordHeader aHd;
    bool bOk = ReadDffRecordHeader( rIn, aHd );
    if (bOk && aHd.nRecType == DFF_msofbtSolverContainer)
    {
        DffRecordHeader aCRule;
        auto nEndPos = DffPropSet::SanitizeEndPos(rIn, aHd.GetRecEndFilePos());
        while ( rIn.good() && ( rIn.Tell() < nEndPos ) )
        {
            if (!ReadDffRecordHeader(rIn, aCRule))
                break;
            if ( aCRule.nRecType == DFF_msofbtConnectorRule )
            {
                SvxMSDffConnectorRule* pRule = new SvxMSDffConnectorRule;
                rIn >> *pRule;
                rContainer.aCList.push_back( pRule );
            }
            if (!aCRule.SeekToEndOfRecord(rIn))
                break;
        }
    }
    return rIn;
}

void SvxMSDffManager::SolveSolver( const SvxMSDffSolverContainer& rSolver )
{
    size_t i, nCnt;
    for ( i = 0, nCnt = rSolver.aCList.size(); i < nCnt; i++ )
    {
        SvxMSDffConnectorRule* pPtr = rSolver.aCList[ i ];
        if ( pPtr->pCObj )
        {
            for ( int nN = 0; nN < 2; nN++ )
            {
                SdrObject*  pO;
                sal_uInt32  nC, nSpFlags;
                if ( !nN )
                {
                    pO = pPtr->pAObj;
                    nC = pPtr->ncptiA;
                    nSpFlags = pPtr->nSpFlagsA;
                }
                else
                {
                    pO = pPtr->pBObj;
                    nC = pPtr->ncptiB;
                    nSpFlags = pPtr->nSpFlagsB;
                }
                if ( pO )
                {
                    SdrGluePoint aGluePoint;
                    Reference< XShape > aXShape( pO->getUnoShape(), UNO_QUERY );
                    Reference< XShape > aXConnector( pPtr->pCObj->getUnoShape(), UNO_QUERY );
                    SdrGluePointList* pList = pO->ForceGluePointList();

                    sal_Int32 nId = nC;
                    SdrInventor nInventor = pO->GetObjInventor();

                    if( nInventor == SdrInventor::Default )
                    {
                        bool bValidGluePoint = false;
                        sal_uInt32 nObjId = pO->GetObjIdentifier();
                        switch( nObjId )
                        {
                            case OBJ_GRUP :
                            case OBJ_GRAF :
                            case OBJ_RECT :
                            case OBJ_TEXT :
                            case OBJ_PAGE :
                            case OBJ_TEXTEXT :
                            case OBJ_wegFITTEXT :
                            case OBJ_wegFITALLTEXT :
                            case OBJ_TITLETEXT :
                            case OBJ_OUTLINETEXT :
                            {
                                if ( nC & 1 )
                                {
                                    if ( nSpFlags & SP_FFLIPH )
                                        nC ^= 2;    // 1 <-> 3
                                }
                                else
                                {
                                    if ( nSpFlags & SP_FFLIPV )
                                        nC ^= 1;    // 0 <-> 2
                                }
                                switch( nC )
                                {
                                    case 0 :
                                        nId = 0;    // SdrAlign::VERT_TOP;
                                    break;
                                    case 1 :
                                        nId = 3;    // SdrAlign::HORZ_RIGHT;
                                    break;
                                    case 2 :
                                        nId = 2;    // SdrAlign::VERT_BOTTOM;
                                    break;
                                    case 3 :
                                        nId = 1; // SdrAlign::HORZ_LEFT;
                                    break;
                                }
                                if ( nId <= 3 )
                                    bValidGluePoint = true;
                            }
                            break;
                            case OBJ_POLY :
                            case OBJ_PLIN :
                            case OBJ_LINE :
                            case OBJ_PATHLINE :
                            case OBJ_PATHFILL :
                            case OBJ_FREELINE :
                            case OBJ_FREEFILL :
                            case OBJ_SPLNLINE :
                            case OBJ_SPLNFILL :
                            case OBJ_PATHPOLY :
                            case OBJ_PATHPLIN :
                            {
                                if (pList)
                                {
                                    if (pList->GetCount() > nC )
                                    {
                                        bValidGluePoint = true;
                                        nId = (sal_Int32)((*pList)[ (sal_uInt16)nC].GetId() + 3 );
                                    }
                                    else
                                    {
                                        bool bNotFound = true;

                                        tools::PolyPolygon aPolyPoly( EscherPropertyContainer::GetPolyPolygon( aXShape ) );
                                        sal_uInt16 k, j, nPolySize = aPolyPoly.Count();
                                        if ( nPolySize )
                                        {
                                            Rectangle aBoundRect( aPolyPoly.GetBoundRect() );
                                            if ( aBoundRect.GetWidth() && aBoundRect.GetHeight() )
                                            {
                                                sal_uInt32  nPointCount = 0;
                                                for ( k = 0; bNotFound && ( k < nPolySize ); k++ )
                                                {
                                                    const tools::Polygon& rPolygon = aPolyPoly.GetObject( k );
                                                    for ( j = 0; bNotFound && ( j < rPolygon.GetSize() ); j++ )
                                                    {
                                                        PolyFlags eFlags = rPolygon.GetFlags( j );
                                                        if ( eFlags == PolyFlags::Normal )
                                                        {
                                                            if ( nC == nPointCount )
                                                            {
                                                                const Point& rPoint = rPolygon.GetPoint( j );
                                                                double fXRel = rPoint.X() - aBoundRect.Left();
                                                                double fYRel = rPoint.Y() - aBoundRect.Top();
                                                                sal_Int32 nWidth = aBoundRect.GetWidth();
                                                                if ( !nWidth )
                                                                    nWidth = 1;
                                                                sal_Int32 nHeight= aBoundRect.GetHeight();
                                                                if ( !nHeight )
                                                                    nHeight = 1;
                                                                fXRel /= (double)nWidth;
                                                                fXRel *= 10000;
                                                                fYRel /= (double)nHeight;
                                                                fYRel *= 10000;
                                                                aGluePoint.SetPos( Point( (sal_Int32)fXRel, (sal_Int32)fYRel ) );
                                                                aGluePoint.SetPercent( true );
                                                                aGluePoint.SetAlign( SdrAlign::VERT_TOP | SdrAlign::HORZ_LEFT );
                                                                aGluePoint.SetEscDir( SdrEscapeDirection::SMART );
                                                                nId = (sal_Int32)((*pList)[ pList->Insert( aGluePoint ) ].GetId() + 3 );
                                                                bNotFound = false;
                                                            }
                                                            nPointCount++;
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                        if ( !bNotFound )
                                        {
                                            bValidGluePoint = true;
                                        }
                                    }
                                }
                            }
                            break;

                            case OBJ_CUSTOMSHAPE :
                            {
                                const SfxPoolItem& aCustomShape =  static_cast<SdrObjCustomShape*>(pO)->GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY );
                                SdrCustomShapeGeometryItem aGeometryItem( static_cast<const SdrCustomShapeGeometryItem&>(aCustomShape) );
                                const OUString sPath( "Path" );
                                const OUString sGluePointType( "GluePointType" );
                                sal_Int16 nGluePointType = EnhancedCustomShapeGluePointType::SEGMENTS;
                                css::uno::Any* pAny = aGeometryItem.GetPropertyValueByName( sPath, sGluePointType );
                                if ( pAny )
                                    *pAny >>= nGluePointType;
                                else
                                {
                                    const OUString sType( "Type" );
                                    OUString sShapeType;
                                    pAny = aGeometryItem.GetPropertyValueByName( sType );
                                    if ( pAny )
                                        *pAny >>= sShapeType;
                                    MSO_SPT eSpType = EnhancedCustomShapeTypeNames::Get( sShapeType );
                                    nGluePointType = GetCustomShapeConnectionTypeDefault( eSpType );
                                }
                                if ( nGluePointType == EnhancedCustomShapeGluePointType::CUSTOM )
                                {
                                    if ( pList && ( pList->GetCount() > nC ) )
                                    {
                                        bValidGluePoint = true;
                                        nId = (sal_Int32)((*pList)[ (sal_uInt16)nC].GetId() + 3 );
                                    }
                                }
                                else if ( nGluePointType == EnhancedCustomShapeGluePointType::RECT )
                                {
                                    if ( nC & 1 )
                                    {
                                        if ( nSpFlags & SP_FFLIPH )
                                            nC ^= 2;    // 1 <-> 3
                                    }
                                    else
                                    {
                                        if ( nSpFlags & SP_FFLIPV )
                                            nC ^= 1;    // 0 <-> 2
                                    }
                                    switch( nC )
                                    {
                                        case 0 :
                                            nId = 0;    // SdrAlign::VERT_TOP;
                                        break;
                                        case 1 :
                                            nId = 3;    // SdrAlign::HORZ_RIGHT;
                                        break;
                                        case 2 :
                                            nId = 2;    // SdrAlign::VERT_BOTTOM;
                                        break;
                                        case 3 :
                                            nId = 1; // SdrAlign::HORZ_LEFT;
                                        break;
                                    }
                                    if ( nId <= 3 )
                                        bValidGluePoint = true;
                                }
                                else if ( nGluePointType == EnhancedCustomShapeGluePointType::SEGMENTS )
                                {
                                    const OUString sSegments( "Segments" );
                                    const OUString sCoordinates( "Coordinates" );

                                    sal_uInt32 k, nPt = nC;
                                    css::uno::Sequence< css::drawing::EnhancedCustomShapeSegment > aSegments;
                                    pAny = aGeometryItem.GetPropertyValueByName( sPath, sSegments );
                                    if ( pAny )
                                    {
                                        if ( *pAny >>= aSegments )
                                        {
                                            for ( nPt = 0, k = 1; nC && ( k < (sal_uInt32)aSegments.getLength() ); k++ )
                                            {
                                                sal_Int16 j, nCnt2 = aSegments[ k ].Count;
                                                if ( aSegments[ k ].Command != EnhancedCustomShapeSegmentCommand::UNKNOWN )
                                                {
                                                    for ( j = 0; nC && ( j < nCnt2 ); j++ )
                                                    {
                                                        switch( aSegments[ k ].Command )
                                                        {
                                                            case EnhancedCustomShapeSegmentCommand::ENDSUBPATH :
                                                            case EnhancedCustomShapeSegmentCommand::CLOSESUBPATH :
                                                            case EnhancedCustomShapeSegmentCommand::LINETO :
                                                            case EnhancedCustomShapeSegmentCommand::MOVETO :
                                                            {
                                                                nC--;
                                                                nPt++;
                                                            }
                                                            break;
                                                            case EnhancedCustomShapeSegmentCommand::ELLIPTICALQUADRANTX :
                                                            case EnhancedCustomShapeSegmentCommand::ELLIPTICALQUADRANTY :
                                                            break;

                                                            case EnhancedCustomShapeSegmentCommand::CURVETO :
                                                            {
                                                                nC--;
                                                                nPt += 3;
                                                            }
                                                            break;

                                                            case EnhancedCustomShapeSegmentCommand::ANGLEELLIPSETO :
                                                            case EnhancedCustomShapeSegmentCommand::ANGLEELLIPSE :
                                                            {
                                                                nC--;
                                                                nPt += 3;
                                                            }
                                                            break;
                                                            case EnhancedCustomShapeSegmentCommand::ARCTO :
                                                            case EnhancedCustomShapeSegmentCommand::ARC :
                                                            case EnhancedCustomShapeSegmentCommand::CLOCKWISEARCTO :
                                                            case EnhancedCustomShapeSegmentCommand::CLOCKWISEARC :
                                                            {
                                                                nC--;
                                                                nPt += 4;
                                                            }
                                                            break;
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                    pAny = aGeometryItem.GetPropertyValueByName( sPath, sCoordinates );
                                    if ( pAny )
                                    {
                                        css::uno::Sequence< css::drawing::EnhancedCustomShapeParameterPair > aCoordinates;
                                        *pAny >>= aCoordinates;
                                        if ( nPt < (sal_uInt32)aCoordinates.getLength() )
                                        {
                                            nId = 4;
                                            css::drawing::EnhancedCustomShapeParameterPair& rPara = aCoordinates[ nPt ];
                                            sal_Int32 nX = 0, nY = 0;
                                            if ( ( rPara.First.Value >>= nX ) && ( rPara.Second.Value >>= nY ) )
                                            {
                                                const OUString sGluePoints( "GluePoints" );
                                                css::uno::Sequence< css::drawing::EnhancedCustomShapeParameterPair > aGluePoints;
                                                pAny = aGeometryItem.GetPropertyValueByName( sPath, sGluePoints );
                                                if ( pAny )
                                                    *pAny >>= aGluePoints;
                                                sal_Int32 nGluePoints = aGluePoints.getLength();
                                                aGluePoints.realloc( nGluePoints + 1 );
                                                EnhancedCustomShape2d::SetEnhancedCustomShapeParameter( aGluePoints[ nGluePoints ].First, nX );
                                                EnhancedCustomShape2d::SetEnhancedCustomShapeParameter( aGluePoints[ nGluePoints ].Second, nY );
                                                PropertyValue aProp;
                                                aProp.Name = sGluePoints;
                                                aProp.Value <<= aGluePoints;
                                                aGeometryItem.SetPropertyValue( sPath, aProp );
                                                bValidGluePoint = true;
                                                static_cast<SdrObjCustomShape*>(pO)->SetMergedItem( aGeometryItem );
                                                SdrGluePointList* pLst = pO->ForceGluePointList();
                                                if ( pLst->GetCount() > nGluePoints )
                                                    nId = (sal_Int32)((*pLst)[ (sal_uInt16)nGluePoints ].GetId() + 3 );
                                            }
                                        }
                                    }
                                }
                            }
                            break;
                        }
                        if ( bValidGluePoint )
                        {
                            Reference< XPropertySet > xPropSet( aXConnector, UNO_QUERY );
                            if ( xPropSet.is() )
                            {
                                if ( nN )
                                {
                                    OUString aPropName( "EndShape" );
                                    SetPropValue( Any(aXShape), xPropSet, aPropName, true );
                                    aPropName = "EndGluePointIndex";
                                    SetPropValue( Any(nId), xPropSet, aPropName, true );
                                }
                                else
                                {
                                    OUString aPropName( "StartShape" );
                                    SetPropValue( Any(aXShape), xPropSet, aPropName, true );
                                    aPropName = "StartGluePointIndex";
                                    SetPropValue( Any(nId), xPropSet, aPropName, true );
                                }

                                // Not sure what this is good for, repaint or broadcast of object change.
                                //( Thus I am adding repaint here
                                pO->SetChanged();
                                pO->BroadcastObjectChange();
                            }
                        }
                    }
                }
            }
        }
    }
}


static basegfx::B2DPolyPolygon GetLineArrow( const sal_Int32 nLineWidth, const MSO_LineEnd eLineEnd,
    const MSO_LineEndWidth eLineWidth, const MSO_LineEndLength eLineLength,
    sal_Int32& rnArrowWidth, bool& rbArrowCenter,
    OUString& rsArrowName, bool bScaleArrow )
{
    basegfx::B2DPolyPolygon aRetPolyPoly;
    // 70 100mm = 2pt = 40 twip. In MS, line width less than 2pt has the same size arrow as 2pt
    //If the unit is twip. Make all use this unit especially the critical value 70/40.
    sal_Int32   nLineWidthCritical = bScaleArrow ? 40 : 70;
    double      fLineWidth = nLineWidth < nLineWidthCritical ? nLineWidthCritical : nLineWidth;

    double      fLengthMul, fWidthMul;
    sal_Int32   nLineNumber;
    switch( eLineLength )
    {
        default :
        case mso_lineMediumLenArrow     : fLengthMul = 3.0; nLineNumber = 2; break;
        case mso_lineShortArrow         : fLengthMul = 2.0; nLineNumber = 1; break;
        case mso_lineLongArrow          : fLengthMul = 5.0; nLineNumber = 3; break;
    }
    switch( eLineWidth )
    {
        default :
        case mso_lineMediumWidthArrow   : fWidthMul = 3.0; nLineNumber += 3; break;
        case mso_lineNarrowArrow        : fWidthMul = 2.0; break;
        case mso_lineWideArrow          : fWidthMul = 5.0; nLineNumber += 6; break;
    }

    rbArrowCenter = false;
    OUStringBuffer aArrowName;
    switch ( eLineEnd )
    {
        case mso_lineArrowEnd :
        {
            basegfx::B2DPolygon aTriangle;
            aTriangle.append(basegfx::B2DPoint( fWidthMul * fLineWidth * 0.50, 0.0 ));
            aTriangle.append(basegfx::B2DPoint( fWidthMul * fLineWidth, fLengthMul * fLineWidth ));
            aTriangle.append(basegfx::B2DPoint( 0.0, fLengthMul * fLineWidth ));
            aTriangle.setClosed(true);
            aRetPolyPoly = basegfx::B2DPolyPolygon(aTriangle);
            aArrowName.append("msArrowEnd ");
        }
        break;

        case mso_lineArrowOpenEnd :
        {
            switch( eLineLength )
            {
                default :
                case mso_lineMediumLenArrow     : fLengthMul = 4.5; break;
                case mso_lineShortArrow         : fLengthMul = 3.5; break;
                case mso_lineLongArrow          : fLengthMul = 6.0; break;
            }
            switch( eLineWidth )
            {
                default :
                case mso_lineMediumWidthArrow   : fWidthMul = 4.5; break;
                case mso_lineNarrowArrow        : fWidthMul = 3.5; break;
                case mso_lineWideArrow          : fWidthMul = 6.0; break;
            }
            basegfx::B2DPolygon aTriangle;
            aTriangle.append(basegfx::B2DPoint( fWidthMul * fLineWidth * 0.50 , 0.0 ));
            aTriangle.append(basegfx::B2DPoint( fWidthMul * fLineWidth, fLengthMul * fLineWidth * 0.91 ));
            aTriangle.append(basegfx::B2DPoint( fWidthMul * fLineWidth * 0.85, fLengthMul * fLineWidth ));
            aTriangle.append(basegfx::B2DPoint( fWidthMul * fLineWidth * 0.50, fLengthMul * fLineWidth * 0.36 ));
            aTriangle.append(basegfx::B2DPoint( fWidthMul * fLineWidth * 0.15, fLengthMul * fLineWidth ));
            aTriangle.append(basegfx::B2DPoint( 0.0, fLengthMul * fLineWidth * 0.91 ));
            aTriangle.setClosed(true);
            aRetPolyPoly = basegfx::B2DPolyPolygon(aTriangle);
            aArrowName.append("msArrowOpenEnd ");
        }
        break;
        case mso_lineArrowStealthEnd :
        {
            basegfx::B2DPolygon aTriangle;
            aTriangle.append(basegfx::B2DPoint( fWidthMul * fLineWidth * 0.50 , 0.0 ));
            aTriangle.append(basegfx::B2DPoint( fWidthMul * fLineWidth , fLengthMul * fLineWidth ));
            aTriangle.append(basegfx::B2DPoint( fWidthMul * fLineWidth * 0.50 , fLengthMul * fLineWidth * 0.60 ));
            aTriangle.append(basegfx::B2DPoint( 0.0, fLengthMul * fLineWidth ));
            aTriangle.setClosed(true);
            aRetPolyPoly = basegfx::B2DPolyPolygon(aTriangle);
            aArrowName.append("msArrowStealthEnd ");
        }
        break;
        case mso_lineArrowDiamondEnd :
        {
            basegfx::B2DPolygon aTriangle;
            aTriangle.append(basegfx::B2DPoint( fWidthMul * fLineWidth * 0.50 , 0.0 ));
            aTriangle.append(basegfx::B2DPoint( fWidthMul * fLineWidth , fLengthMul * fLineWidth * 0.50 ));
            aTriangle.append(basegfx::B2DPoint( fWidthMul * fLineWidth * 0.50 , fLengthMul * fLineWidth ));
            aTriangle.append(basegfx::B2DPoint( 0.0, fLengthMul * fLineWidth * 0.50 ));
            aTriangle.setClosed(true);
            aRetPolyPoly = basegfx::B2DPolyPolygon(aTriangle);
            rbArrowCenter = true;
            aArrowName.append("msArrowDiamondEnd ");
        }
        break;
        case mso_lineArrowOvalEnd :
        {
            aRetPolyPoly = basegfx::B2DPolyPolygon( XPolygon( Point( (sal_Int32)( fWidthMul * fLineWidth * 0.50 ), 0 ),
                                (sal_Int32)( fWidthMul * fLineWidth * 0.50 ),
                                    (sal_Int32)( fLengthMul * fLineWidth * 0.50 ), 0, 3600 ).getB2DPolygon() );
            rbArrowCenter = true;
            aArrowName.append("msArrowOvalEnd ");
        }
        break;
        default: break;
    }
    aArrowName.append(nLineNumber);
    rsArrowName = aArrowName.makeStringAndClear();
    rnArrowWidth = (sal_Int32)( fLineWidth * fWidthMul );

    return aRetPolyPoly;
}

void DffPropertyReader::ApplyLineAttributes( SfxItemSet& rSet, const MSO_SPT eShapeType ) const // #i28269#
{
    sal_uInt32 nLineFlags(GetPropertyValue( DFF_Prop_fNoLineDrawDash, 0 ));

    if(!IsHardAttribute( DFF_Prop_fLine ) && !IsCustomShapeStrokedByDefault( eShapeType ))
    {
        nLineFlags &= ~0x08;
    }

    if ( nLineFlags & 8 )
    {
        // Line Attributes
        sal_Int32 nLineWidth = (sal_Int32)GetPropertyValue( DFF_Prop_lineWidth, 9525 );

        // support LineCap
        const MSO_LineCap eLineCap((MSO_LineCap)GetPropertyValue(DFF_Prop_lineEndCapStyle, mso_lineEndCapSquare));

        switch(eLineCap)
        {
            default: /* case mso_lineEndCapFlat */
            {
                // no need to set, it is the default. If this changes, this needs to be activated
                // rSet.Put(XLineCapItem(css::drawing::LineCap_BUTT));
                break;
            }
            case mso_lineEndCapRound:
            {
                rSet.Put(XLineCapItem(css::drawing::LineCap_ROUND));
                break;
            }
            case mso_lineEndCapSquare:
            {
                rSet.Put(XLineCapItem(css::drawing::LineCap_SQUARE));
                break;
            }
        }

        MSO_LineDashing eLineDashing = (MSO_LineDashing)GetPropertyValue( DFF_Prop_lineDashing, mso_lineSolid );
        if ( eLineDashing == mso_lineSolid )
            rSet.Put(XLineStyleItem( drawing::LineStyle_SOLID ) );
        else
        {
            css::drawing::DashStyle  eDash = css::drawing::DashStyle_RECT;
            sal_uInt16  nDots = 1;
            sal_uInt32  nDotLen = nLineWidth / 360;
            sal_uInt16  nDashes = 0;
            sal_uInt32  nDashLen = ( 8 * nLineWidth ) / 360;
            sal_uInt32  nDistance = ( 3 * nLineWidth ) / 360;

            switch ( eLineDashing )
            {
                default:
                case mso_lineDotSys :
                {
                    nDots = 1;
                    nDashes = 0;
                    nDistance = nDotLen;
                }
                break;

                case mso_lineDashGEL :
                {
                    nDots = 0;
                    nDashes = 1;
                    nDashLen = ( 4 * nLineWidth ) / 360;
                }
                break;

                case mso_lineDashDotGEL :
                {
                    nDots = 1;
                    nDashes = 1;
                    nDashLen = ( 4 * nLineWidth ) / 360;
                }
                break;

                case mso_lineLongDashGEL :
                {
                    nDots = 0;
                    nDashes = 1;
                }
                break;

                case mso_lineLongDashDotGEL :
                {
                    nDots = 1;
                    nDashes = 1;
                }
                break;

                case mso_lineLongDashDotDotGEL:
                {
                    nDots = 2;
                    nDashes = 1;
                }
                break;
            }

            rSet.Put( XLineDashItem( OUString(), XDash( eDash, nDots, nDotLen, nDashes, nDashLen, nDistance ) ) );
            rSet.Put( XLineStyleItem( drawing::LineStyle_DASH ) );
        }
        rSet.Put( XLineColorItem( OUString(), rManager.MSO_CLR_ToColor( GetPropertyValue( DFF_Prop_lineColor, 0 ) ) ) );
        if ( IsProperty( DFF_Prop_lineOpacity ) )
        {
            double nTrans = GetPropertyValue(DFF_Prop_lineOpacity, 0x10000);
            nTrans = (nTrans * 100) / 65536;
            rSet.Put(XLineTransparenceItem(
                sal_uInt16(100 - ::rtl::math::round(nTrans))));
        }

        rManager.ScaleEmu( nLineWidth );
        rSet.Put( XLineWidthItem( nLineWidth ) );

        // SJ: LineJoint (setting each time a line is set, because our internal joint type has another default)
        MSO_LineJoin eLineJointDefault = mso_lineJoinMiter;
        if ( eShapeType == mso_sptMin )
            eLineJointDefault = mso_lineJoinRound;
        MSO_LineJoin eLineJoint = (MSO_LineJoin)GetPropertyValue( DFF_Prop_lineJoinStyle, eLineJointDefault );
        css::drawing::LineJoint eXLineJoint( css::drawing::LineJoint_MITER );
        if ( eLineJoint == mso_lineJoinBevel )
            eXLineJoint = css::drawing::LineJoint_BEVEL;
        else if ( eLineJoint == mso_lineJoinRound )
            eXLineJoint = css::drawing::LineJoint_ROUND;
        rSet.Put( XLineJointItem( eXLineJoint ) );

        if ( nLineFlags & 0x10 )
        {
            bool bScaleArrows = rManager.pSdrModel->GetScaleUnit() == MapUnit::MapTwip;

            // LineStart

            if ( IsProperty( DFF_Prop_lineStartArrowhead ) )
            {
                MSO_LineEnd         eLineEnd = (MSO_LineEnd)GetPropertyValue( DFF_Prop_lineStartArrowhead, 0 );
                MSO_LineEndWidth    eWidth = (MSO_LineEndWidth)GetPropertyValue( DFF_Prop_lineStartArrowWidth, mso_lineMediumWidthArrow );
                MSO_LineEndLength   eLength = (MSO_LineEndLength)GetPropertyValue( DFF_Prop_lineStartArrowLength, mso_lineMediumLenArrow );

                sal_Int32   nArrowWidth;
                bool        bArrowCenter;
                OUString    aArrowName;
                basegfx::B2DPolyPolygon aPolyPoly(GetLineArrow( nLineWidth, eLineEnd, eWidth, eLength, nArrowWidth, bArrowCenter, aArrowName, bScaleArrows ));

                rSet.Put( XLineStartWidthItem( nArrowWidth ) );
                rSet.Put( XLineStartItem( aArrowName, aPolyPoly) );
                rSet.Put( XLineStartCenterItem( bArrowCenter ) );
            }

            // LineEnd

            if ( IsProperty( DFF_Prop_lineEndArrowhead ) )
            {
                MSO_LineEnd         eLineEnd = (MSO_LineEnd)GetPropertyValue( DFF_Prop_lineEndArrowhead, 0 );
                MSO_LineEndWidth    eWidth = (MSO_LineEndWidth)GetPropertyValue( DFF_Prop_lineEndArrowWidth, mso_lineMediumWidthArrow );
                MSO_LineEndLength   eLength = (MSO_LineEndLength)GetPropertyValue( DFF_Prop_lineEndArrowLength, mso_lineMediumLenArrow );

                sal_Int32   nArrowWidth;
                bool        bArrowCenter;
                OUString    aArrowName;
                basegfx::B2DPolyPolygon aPolyPoly(GetLineArrow( nLineWidth, eLineEnd, eWidth, eLength, nArrowWidth, bArrowCenter, aArrowName, bScaleArrows ));

                rSet.Put( XLineEndWidthItem( nArrowWidth ) );
                rSet.Put( XLineEndItem( aArrowName, aPolyPoly ) );
                rSet.Put( XLineEndCenterItem( bArrowCenter ) );
            }
        }
    }
    else
        rSet.Put( XLineStyleItem( drawing::LineStyle_NONE ) );
}

struct ShadeColor
{
    Color       aColor;
    double      fDist;

    ShadeColor( const Color& rC, double fR ) : aColor( rC ), fDist( fR ) {};
};

void GetShadeColors( const SvxMSDffManager& rManager, const DffPropertyReader& rProperties, SvStream& rIn, std::vector< ShadeColor >& rShadeColors )
{
    sal_uInt32 nPos = rIn.Tell();
    if ( rProperties.IsProperty( DFF_Prop_fillShadeColors ) )
    {
        sal_uInt16 i = 0, nNumElem = 0, nNumElemReserved = 0, nSize = 0;
        bool bOk = false;
        if (rProperties.SeekToContent(DFF_Prop_fillShadeColors, rIn))
        {
            rIn.ReadUInt16( nNumElem ).ReadUInt16( nNumElemReserved ).ReadUInt16( nSize );
            //sanity check that the stream is long enough to fulfill nNumElem * 2 sal_Int32s
            bOk = rIn.remainingSize() / (2*sizeof(sal_Int32)) >= nNumElem;
        }
        if (bOk)
        {
            for ( ; i < nNumElem; i++ )
            {
                sal_Int32 nColor(0);
                sal_Int32 nDist(0);

                rIn.ReadInt32( nColor ).ReadInt32( nDist );
                rShadeColors.push_back( ShadeColor( rManager.MSO_CLR_ToColor( nColor, DFF_Prop_fillColor ), 1.0 - ( nDist / 65536.0 ) ) );
            }
        }
    }
    if ( rShadeColors.empty() )
    {
        rShadeColors.push_back( ShadeColor( rManager.MSO_CLR_ToColor( rProperties.GetPropertyValue( DFF_Prop_fillBackColor, COL_WHITE ), DFF_Prop_fillBackColor ), 0 ) );
        rShadeColors.push_back( ShadeColor( rManager.MSO_CLR_ToColor( rProperties.GetPropertyValue( DFF_Prop_fillColor, COL_WHITE ), DFF_Prop_fillColor ), 1 ) );
    }
    rIn.Seek( nPos );
}

void ApplyRectangularGradientAsBitmap( const SvxMSDffManager& rManager, SvStream& rIn, SfxItemSet& rSet, const std::vector< ShadeColor >& rShadeColors, const DffObjData& rObjData, sal_Int32 nFix16Angle )
{
    Size aBitmapSizePixel( static_cast< sal_Int32 >( ( rObjData.aBoundRect.GetWidth() / 2540.0 ) * 90.0 ),      // we will create a bitmap with 90 dpi
                           static_cast< sal_Int32 >( ( rObjData.aBoundRect.GetHeight() / 2540.0 ) * 90.0 ) );
    if ( aBitmapSizePixel.Width() && aBitmapSizePixel.Height() && ( aBitmapSizePixel.Width() <= 1024 ) && ( aBitmapSizePixel.Height() <= 1024 ) )
    {
        double fFocusX = rManager.GetPropertyValue( DFF_Prop_fillToRight, 0 ) / 65536.0;
        double fFocusY = rManager.GetPropertyValue( DFF_Prop_fillToBottom, 0 ) / 65536.0;

        Bitmap aBitmap( aBitmapSizePixel, 24 );
        BitmapWriteAccess* pAcc = aBitmap.AcquireWriteAccess();
        if ( pAcc )
        {
            sal_Int32 nX, nY;
            for ( nY = 0; nY < aBitmapSizePixel.Height(); nY++ )
            {
                for ( nX = 0; nX < aBitmapSizePixel.Width(); nX++ )
                {
                    double fX = static_cast< double >( nX ) / aBitmapSizePixel.Width();
                    double fY = static_cast< double >( nY ) / aBitmapSizePixel.Height();

                    double fD, fDist;
                    if ( fX < fFocusX )
                    {
                        if ( fY < fFocusY )
                        {
                            if ( fX > fY )
                            {
                                fDist = fY;
                                fD = fFocusY;
                            }
                            else
                            {
                                fDist = fX;
                                fD = fFocusX;
                            }
                        }
                        else
                        {
                            if ( fX > ( 1 - fY ) )
                            {
                                fDist = 1 - fY;
                                fD = 1 - fFocusY;
                            }
                            else
                            {
                                fDist = fX;
                                fD = fFocusX;
                            }
                        }
                    }
                    else
                    {
                        if ( fY < fFocusY )
                        {
                            if ( ( 1 - fX ) > fY )
                            {
                                fDist = fY;
                                fD = fFocusY;
                            }
                            else
                            {
                                fDist = 1 - fX;
                                fD = 1 - fFocusX;
                            }
                        }
                        else
                        {
                            if ( ( 1 - fX ) > ( 1 - fY ) )
                            {
                                fDist = 1 - fY;
                                fD = 1 - fFocusY;
                            }
                            else
                            {
                                fDist = 1 - fX;
                                fD = 1 - fFocusX;
                            }
                        }
                    }
                    if ( fD != 0.0 )
                        fDist /= fD;

                    std::vector< ShadeColor >::const_iterator aIter( rShadeColors.begin() );
                    double fA = 0.0;
                    Color aColorA = aIter->aColor;
                    double fB = 1.0;
                    Color aColorB( aColorA );
                    while ( aIter != rShadeColors.end() )
                    {
                        if ( aIter->fDist <= fDist )
                        {
                            if ( aIter->fDist >= fA )
                            {
                                fA = aIter->fDist;
                                aColorA = aIter->aColor;
                            }
                        }
                        if ( aIter->fDist > fDist )
                        {
                            if ( aIter->fDist <= fB )
                            {
                                fB = aIter->fDist;
                                aColorB = aIter->aColor;
                            }
                        }
                        ++aIter;
                    }
                    double fRed = aColorA.GetRed(), fGreen = aColorA.GetGreen(), fBlue = aColorA.GetBlue();
                    double fD1 = fB - fA;
                    if ( fD1 != 0.0 )
                    {
                        fRed   += ( ( ( fDist - fA ) * ( aColorB.GetRed() - aColorA.GetRed() ) ) / fD1 );       // + aQuantErrCurrScan[ nX ].fRed;
                        fGreen += ( ( ( fDist - fA ) * ( aColorB.GetGreen() - aColorA.GetGreen() ) ) / fD1 );   // + aQuantErrCurrScan[ nX ].fGreen;
                        fBlue  += ( ( ( fDist - fA ) * ( aColorB.GetBlue() - aColorA.GetBlue() ) ) / fD1 );     // + aQuantErrCurrScan[ nX ].fBlue;
                    }
                    sal_Int16 nRed   = static_cast< sal_Int16 >( fRed   + 0.5 );
                    sal_Int16 nGreen = static_cast< sal_Int16 >( fGreen + 0.5 );
                    sal_Int16 nBlue  = static_cast< sal_Int16 >( fBlue  + 0.5 );
                    if ( nRed < 0 )
                        nRed = 0;
                    if ( nRed > 255 )
                        nRed = 255;
                    if ( nGreen < 0 )
                        nGreen = 0;
                    if ( nGreen > 255 )
                        nGreen = 255;
                    if ( nBlue < 0 )
                        nBlue = 0;
                    if ( nBlue > 255 )
                        nBlue = 255;

                    pAcc->SetPixel( nY, nX, BitmapColor( static_cast< sal_Int8 >( nRed ), static_cast< sal_Int8 >( nGreen ), static_cast< sal_Int8 >( nBlue ) ) );
                }
            }
            Bitmap::ReleaseAccess( pAcc );

            if ( nFix16Angle )
            {
                bool bRotateWithShape = true;   // sal_True seems to be default
                sal_uInt32 nPos = rIn.Tell();
                if ( const_cast< SvxMSDffManager& >( rManager ).maShapeRecords.SeekToContent( rIn, DFF_msofbtUDefProp, SEEK_FROM_CURRENT_AND_RESTART ) )
                {
                    const_cast< SvxMSDffManager& >( rManager ).maShapeRecords.Current()->SeekToBegOfRecord( rIn );
                    DffPropertyReader aSecPropSet( rManager );
                    aSecPropSet.ReadPropSet( rIn, nullptr );
                    sal_Int32 nSecFillProperties = aSecPropSet.GetPropertyValue( DFF_Prop_fNoFillHitTest, 0x200020 );
                    bRotateWithShape = ( nSecFillProperties & 0x0020 );
                }
                rIn.Seek( nPos );
                if ( bRotateWithShape )
                {
                    aBitmap.Rotate( nFix16Angle / 10, rShadeColors[ 0 ].aColor );

                    BmpMirrorFlags nMirrorFlags = BmpMirrorFlags::NONE;
                    if ( rObjData.nSpFlags & SP_FFLIPV )
                        nMirrorFlags |= BmpMirrorFlags::Vertical;
                    if ( rObjData.nSpFlags & SP_FFLIPH )
                        nMirrorFlags |= BmpMirrorFlags::Horizontal;
                    if ( nMirrorFlags != BmpMirrorFlags::NONE )
                        aBitmap.Mirror( nMirrorFlags );
                }
            }

            rSet.Put(XFillBmpTileItem(false));
            rSet.Put(XFillBitmapItem(OUString(), Graphic(aBitmap)));
        }
    }
}

void DffPropertyReader::ApplyFillAttributes( SvStream& rIn, SfxItemSet& rSet, const DffObjData& rObjData ) const
{
    sal_uInt32 nFillFlags(GetPropertyValue( DFF_Prop_fNoFillHitTest, 0 ));

    std::vector< ShadeColor > aShadeColors;
    GetShadeColors( rManager, *this, rIn, aShadeColors );

    if(!IsHardAttribute( DFF_Prop_fFilled ) && !IsCustomShapeFilledByDefault( rObjData.eShapeType ))
    {
        nFillFlags &= ~0x10;
    }

    if ( nFillFlags & 0x10 )
    {
        MSO_FillType eMSO_FillType = (MSO_FillType)GetPropertyValue( DFF_Prop_fillType, mso_fillSolid );
        drawing::FillStyle eXFill = drawing::FillStyle_NONE;
        switch( eMSO_FillType )
        {
            case mso_fillSolid :            // Fill with a solid color
                eXFill = drawing::FillStyle_SOLID;
            break;
            case mso_fillPattern :          // Fill with a pattern (bitmap)
            case mso_fillTexture :          // A texture (pattern with its own color map)
            case mso_fillPicture :          // Center a picture in the shape
                eXFill = drawing::FillStyle_BITMAP;
            break;
            case mso_fillShadeCenter :      // Shade from bounding rectangle to end point
            {
                //If it is imported as a bitmap, it will not work well with transparency especially 100
                //But the gradient look well comparing with imported as gradient. And rotate with shape
                //also works better. So here just keep it.
                if ( rObjData.aBoundRect.IsEmpty() )// size of object needed to be able
                    eXFill = drawing::FillStyle_GRADIENT;        // to create a bitmap substitution
                else
                    eXFill = drawing::FillStyle_BITMAP;
            }
            break;
            case mso_fillShade :            // Shade from start to end points
            case mso_fillShadeShape :       // Shade from shape outline to end point
            case mso_fillShadeScale :       // Similar to mso_fillShade, but the fillAngle
            case mso_fillShadeTitle :       // special type - shade to title ---  for PP
                eXFill = drawing::FillStyle_GRADIENT;
            break;
//          case mso_fillBackground :       // Use the background fill color/pattern
            default: break;
        }
        rSet.Put( XFillStyleItem( eXFill ) );

        double dTrans  = 1.0;
        double dBackTrans = 1.0;
        if (IsProperty(DFF_Prop_fillOpacity))
        {
            dTrans = GetPropertyValue(DFF_Prop_fillOpacity, 0) / 65536.0;
            if ( eXFill != drawing::FillStyle_GRADIENT )
            {
                dTrans = dTrans * 100;
                rSet.Put(XFillTransparenceItem(
                    sal_uInt16(100 - ::rtl::math::round(dTrans))));
            }
        }

        if ( IsProperty(DFF_Prop_fillBackOpacity) )
            dBackTrans = GetPropertyValue(DFF_Prop_fillBackOpacity, 0) / 65536.0;

        if ( ( eMSO_FillType == mso_fillShadeCenter ) && ( eXFill == drawing::FillStyle_BITMAP ) )
        {
            ApplyRectangularGradientAsBitmap( rManager, rIn, rSet, aShadeColors, rObjData, mnFix16Angle );
        }
        else if ( eXFill == drawing::FillStyle_GRADIENT )
        {
            ImportGradientColor ( rSet, eMSO_FillType, dTrans , dBackTrans );
        }
        else if ( eXFill == drawing::FillStyle_BITMAP )
        {
            if( IsProperty( DFF_Prop_fillBlip ) )
            {
                Graphic aGraf;
                // first try to get BLIP from cache
                bool bOK = const_cast<SvxMSDffManager&>(rManager).GetBLIP( GetPropertyValue( DFF_Prop_fillBlip, 0 ), aGraf );
                // then try directly from stream (i.e. Excel chart hatches/bitmaps)
                if ( !bOK )
                    bOK = SeekToContent( DFF_Prop_fillBlip, rIn ) && SvxMSDffManager::GetBLIPDirect( rIn, aGraf );
                if ( bOK )
                {
                    if ( eMSO_FillType == mso_fillPattern )
                    {
                        Bitmap aBmp( aGraf.GetBitmap() );
                        if( aBmp.GetSizePixel().Width() == 8 && aBmp.GetSizePixel().Height() == 8 && aBmp.GetColorCount() == 2)
                        {
                            Color aCol1( COL_WHITE ), aCol2( COL_WHITE );

                            if ( IsProperty( DFF_Prop_fillColor ) )
                                aCol1 = rManager.MSO_CLR_ToColor( GetPropertyValue( DFF_Prop_fillColor, 0 ), DFF_Prop_fillColor );

                            if ( IsProperty( DFF_Prop_fillBackColor ) )
                                aCol2 = rManager.MSO_CLR_ToColor( GetPropertyValue( DFF_Prop_fillBackColor, 0 ), DFF_Prop_fillBackColor );

                            // Create a bitmap for the pattern with expected colors
                            Bitmap aResult(Size(8, 8), 24);
                            {
                                Bitmap::ScopedReadAccess pRead(aBmp);
                                Bitmap::ScopedWriteAccess pWrite(aResult);

                                for (long y = 0; y < pWrite->Height(); ++y)
                                {
                                    for (long x = 0; x < pWrite->Width(); ++x)
                                    {
                                        Color aReadColor;
                                        if (pRead->HasPalette())
                                            aReadColor = pRead->GetPaletteColor(pRead->GetPixelIndex(y, x));
                                        else
                                            aReadColor = pRead->GetPixel(y, x);

                                        if (aReadColor.GetColor() == 0)
                                            pWrite->SetPixel(y, x, aCol2);
                                        else
                                            pWrite->SetPixel(y, x, aCol1);
                                    }
                                }
                            }
                            aGraf = Graphic(aResult);
                        }

                        rSet.Put(XFillBitmapItem(OUString(), aGraf));
                    }
                    else if ( eMSO_FillType == mso_fillTexture )
                    {
                        rSet.Put(XFillBmpTileItem(true));
                        rSet.Put(XFillBitmapItem(OUString(), aGraf));
                        rSet.Put(XFillBmpSizeXItem(GetPropertyValue(DFF_Prop_fillWidth, 0) / 360));
                        rSet.Put(XFillBmpSizeYItem(GetPropertyValue(DFF_Prop_fillHeight, 0) / 360));
                        rSet.Put(XFillBmpSizeLogItem(true));
                    }
                    else
                    {
                        rSet.Put(XFillBitmapItem(OUString(), aGraf));
                        rSet.Put(XFillBmpTileItem(false));
                    }
                }
            }
        }
    }
    else
        rSet.Put( XFillStyleItem( drawing::FillStyle_NONE ) );
}

void DffPropertyReader::ApplyCustomShapeTextAttributes( SfxItemSet& rSet ) const
{
    bool  bVerticalText = false;
    sal_Int32 nTextLeft = GetPropertyValue( DFF_Prop_dxTextLeft, 25 * 3600 ) / 360;     // 0.25 cm (emu)
    sal_Int32 nTextRight = GetPropertyValue( DFF_Prop_dxTextRight, 25 * 3600 ) / 360;   // 0.25 cm (emu)
    sal_Int32 nTextTop = GetPropertyValue( DFF_Prop_dyTextTop, 13 * 3600 ) / 360;       // 0.13 cm (emu)
    sal_Int32 nTextBottom = GetPropertyValue( DFF_Prop_dyTextBottom, 13 * 3600 ) /360;  // 0.13 cm (emu)

    SdrTextVertAdjust eTVA;
    SdrTextHorzAdjust eTHA;

    if ( IsProperty( DFF_Prop_txflTextFlow ) )
    {
        MSO_TextFlow eTextFlow = (MSO_TextFlow)( GetPropertyValue( DFF_Prop_txflTextFlow, 0 ) & 0xFFFF );
        switch( eTextFlow )
        {
            case mso_txflTtoBA :    /* #68110# */   // Top to Bottom @-font, oben -> unten
            case mso_txflTtoBN :                    // Top to Bottom non-@, oben -> unten
            case mso_txflVertN :                    // Vertical, non-@, oben -> unten
                bVerticalText = true;           // nTextRotationAngle += 27000;
            break;
            default: break;
        }
    }
    sal_Int32 nFontDirection = GetPropertyValue( DFF_Prop_cdirFont, mso_cdir0 );
    if ( ( nFontDirection == 1 ) || ( nFontDirection == 3 ) )
        bVerticalText = !bVerticalText;

    if ( bVerticalText )
    {
        eTHA = SDRTEXTHORZADJUST_CENTER;

        // read text anchor
        sal_uInt32 eTextAnchor = GetPropertyValue( DFF_Prop_anchorText, mso_anchorTop );

        switch( eTextAnchor )
        {
            case mso_anchorTop:
            case mso_anchorTopCentered:
            case mso_anchorTopBaseline:
            case mso_anchorTopCenteredBaseline:
                eTHA = SDRTEXTHORZADJUST_RIGHT;
            break;

            case mso_anchorMiddle :
            case mso_anchorMiddleCentered:
                eTHA = SDRTEXTHORZADJUST_CENTER;
            break;

            case mso_anchorBottom:
            case mso_anchorBottomCentered:
            case mso_anchorBottomBaseline:
            case mso_anchorBottomCenteredBaseline:
                eTHA = SDRTEXTHORZADJUST_LEFT;
            break;
        }
        // if there is a 100% use of following attributes, the textbox can been aligned also in vertical direction
        switch ( eTextAnchor )
        {
            case mso_anchorTopCentered :
            case mso_anchorMiddleCentered :
            case mso_anchorBottomCentered :
            case mso_anchorTopCenteredBaseline:
            case mso_anchorBottomCenteredBaseline:
                eTVA = SDRTEXTVERTADJUST_CENTER;
            break;

            default :
                eTVA = SDRTEXTVERTADJUST_TOP;
            break;
        }
    }
    else
    {
        eTVA = SDRTEXTVERTADJUST_CENTER;

        // read text anchor
        sal_uInt32 eTextAnchor = GetPropertyValue( DFF_Prop_anchorText, mso_anchorTop );

        switch( eTextAnchor )
        {
            case mso_anchorTop:
            case mso_anchorTopCentered:
            case mso_anchorTopBaseline:
            case mso_anchorTopCenteredBaseline:
                eTVA = SDRTEXTVERTADJUST_TOP;
            break;

            case mso_anchorMiddle :
            case mso_anchorMiddleCentered:
                eTVA = SDRTEXTVERTADJUST_CENTER;
            break;

            case mso_anchorBottom:
            case mso_anchorBottomCentered:
            case mso_anchorBottomBaseline:
            case mso_anchorBottomCenteredBaseline:
                eTVA = SDRTEXTVERTADJUST_BOTTOM;
            break;
        }
        // if there is a 100% usage of following attributes, the textbox can be aligned also in horizontal direction
        switch ( eTextAnchor )
        {
            case mso_anchorTopCentered :
            case mso_anchorMiddleCentered :
            case mso_anchorBottomCentered :
            case mso_anchorTopCenteredBaseline:
            case mso_anchorBottomCenteredBaseline:
                eTHA = SDRTEXTHORZADJUST_CENTER;    // the text has to be displayed using the full width;
            break;

            default :
                eTHA = SDRTEXTHORZADJUST_LEFT;
            break;
        }
    }
    rSet.Put( SvxFrameDirectionItem( bVerticalText ? FRMDIR_VERT_TOP_RIGHT : FRMDIR_HORI_LEFT_TOP, EE_PARA_WRITINGDIR ) );

    rSet.Put( SdrTextVertAdjustItem( eTVA ) );
    rSet.Put( SdrTextHorzAdjustItem( eTHA ) );

    rSet.Put( makeSdrTextLeftDistItem( nTextLeft ) );
    rSet.Put( makeSdrTextRightDistItem( nTextRight ) );
    rSet.Put( makeSdrTextUpperDistItem( nTextTop ) );
    rSet.Put( makeSdrTextLowerDistItem( nTextBottom ) );

    rSet.Put( makeSdrTextWordWrapItem( (MSO_WrapMode)GetPropertyValue( DFF_Prop_WrapText, mso_wrapSquare ) != mso_wrapNone ) );
    rSet.Put( makeSdrTextAutoGrowHeightItem( ( GetPropertyValue( DFF_Prop_FitTextToShape, 0 ) & 2 ) != 0 ) );
}

void DffPropertyReader::ApplyCustomShapeGeometryAttributes( SvStream& rIn, SfxItemSet& rSet, const DffObjData& rObjData ) const
{

    sal_uInt32 nAdjustmentsWhichNeedsToBeConverted = 0;


    // creating SdrCustomShapeGeometryItem

    typedef std::vector< beans::PropertyValue > PropVec;

    // aPropVec will be filled with all PropertyValues
    PropVec aPropVec;
    PropertyValue aProp;


    // "Type" property, including the predefined CustomShape type name

    const OUString sType( "Type" );
    aProp.Name  = sType;
    aProp.Value <<= EnhancedCustomShapeTypeNames::Get( rObjData.eShapeType );
    aPropVec.push_back( aProp );


    // "ViewBox"


    sal_Int32 nCoordWidth = 21600;  // needed to replace handle type center with absolute value
    sal_Int32 nCoordHeight= 21600;
    if ( IsProperty( DFF_Prop_geoLeft ) || IsProperty( DFF_Prop_geoTop ) || IsProperty( DFF_Prop_geoRight ) || IsProperty( DFF_Prop_geoBottom ) )
    {
        css::awt::Rectangle aViewBox;
        const OUString sViewBox( "ViewBox" );
        aViewBox.X = GetPropertyValue( DFF_Prop_geoLeft, 0 );
        aViewBox.Y = GetPropertyValue( DFF_Prop_geoTop, 0 );
        aViewBox.Width = nCoordWidth = ((sal_Int32)GetPropertyValue( DFF_Prop_geoRight, 21600 ) ) - aViewBox.X;
        aViewBox.Height = nCoordHeight = ((sal_Int32)GetPropertyValue( DFF_Prop_geoBottom, 21600 ) ) - aViewBox.Y;
        aProp.Name = sViewBox;
        aProp.Value <<= aViewBox;
        aPropVec.push_back( aProp );
    }

    // TextRotateAngle

    if ( IsProperty( DFF_Prop_txflTextFlow ) || IsProperty( DFF_Prop_cdirFont ) )
    {
        sal_Int32 nTextRotateAngle = 0;
        MSO_TextFlow eTextFlow = (MSO_TextFlow)( GetPropertyValue( DFF_Prop_txflTextFlow, 0 ) & 0xFFFF );

        if ( eTextFlow == mso_txflBtoT )    // Bottom to Top non-@
            nTextRotateAngle += 90;
        switch( GetPropertyValue( DFF_Prop_cdirFont, mso_cdir0 ) )  // SJ: mso_cdir90 and mso_cdir270 will be simulated by
        {                                                           // activating vertical writing for the text objects
            case mso_cdir90 :
            {
                if ( eTextFlow == mso_txflTtoBA )
                    nTextRotateAngle -= 180;
            }
            break;
            case mso_cdir180: nTextRotateAngle -= 180; break;
            case mso_cdir270:
            {
                if ( eTextFlow != mso_txflTtoBA )
                    nTextRotateAngle -= 180;
            }
            break;
            default: break;
        }
        if ( nTextRotateAngle )
        {
            double fTextRotateAngle = nTextRotateAngle;
            const OUString sTextRotateAngle( "TextRotateAngle" );
            aProp.Name = sTextRotateAngle;
            aProp.Value <<= fTextRotateAngle;
            aPropVec.push_back( aProp );
        }
    }

    // "Extrusion" PropertySequence element

    bool bExtrusionOn = ( GetPropertyValue( DFF_Prop_fc3DLightFace, 0 ) & 8 ) != 0;
    if ( bExtrusionOn )
    {
        PropVec aExtrusionPropVec;

        // "Extrusion"
        const OUString sExtrusionOn( "Extrusion" );
        aProp.Name = sExtrusionOn;
        aProp.Value <<= bExtrusionOn;
        aExtrusionPropVec.push_back( aProp );

        // "Brightness"
        if ( IsProperty( DFF_Prop_c3DAmbientIntensity ) )
        {
            const OUString sExtrusionBrightness( "Brightness" );
            double fBrightness = (sal_Int32)GetPropertyValue( DFF_Prop_c3DAmbientIntensity, 0 );
            fBrightness /= 655.36;
            aProp.Name = sExtrusionBrightness;
            aProp.Value <<= fBrightness;
            aExtrusionPropVec.push_back( aProp );
        }
        // "Depth" in 1/100mm
        if ( IsProperty( DFF_Prop_c3DExtrudeBackward ) || IsProperty( DFF_Prop_c3DExtrudeForward ) )
        {
            const OUString sDepth( "Depth" );
            double fBackDepth = (double)((sal_Int32)GetPropertyValue( DFF_Prop_c3DExtrudeBackward, 1270 * 360 )) / 360.0;
            double fForeDepth = (double)((sal_Int32)GetPropertyValue( DFF_Prop_c3DExtrudeForward, 0 )) / 360.0;
            double fDepth = fBackDepth + fForeDepth;
            double fFraction = fDepth != 0.0 ? fForeDepth / fDepth : 0;
            EnhancedCustomShapeParameterPair aDepthParaPair;
            aDepthParaPair.First.Value <<= fDepth;
            aDepthParaPair.First.Type = EnhancedCustomShapeParameterType::NORMAL;
            aDepthParaPair.Second.Value <<= fFraction;
            aDepthParaPair.Second.Type = EnhancedCustomShapeParameterType::NORMAL;
            aProp.Name = sDepth;
            aProp.Value <<= aDepthParaPair;
            aExtrusionPropVec.push_back( aProp );
        }
        // "Diffusion"
        if ( IsProperty( DFF_Prop_c3DDiffuseAmt ) )
        {
            const OUString sExtrusionDiffusion( "Diffusion" );
            double fDiffusion = (sal_Int32)GetPropertyValue( DFF_Prop_c3DDiffuseAmt, 0 );
            fDiffusion /= 655.36;
            aProp.Name = sExtrusionDiffusion;
            aProp.Value <<= fDiffusion;
            aExtrusionPropVec.push_back( aProp );
        }
        // "NumberOfLineSegments"
        if ( IsProperty( DFF_Prop_c3DTolerance ) )
        {
            const OUString sExtrusionNumberOfLineSegments( "NumberOfLineSegments" );
            aProp.Name = sExtrusionNumberOfLineSegments;
            aProp.Value <<= (sal_Int32)GetPropertyValue( DFF_Prop_c3DTolerance, 0 );
            aExtrusionPropVec.push_back( aProp );
        }
        // "LightFace"
        const OUString sExtrusionLightFace( "LightFace" );
        bool bExtrusionLightFace = ( GetPropertyValue( DFF_Prop_fc3DLightFace, 0 ) & 1 ) != 0;
        aProp.Name = sExtrusionLightFace;
        aProp.Value <<= bExtrusionLightFace;
        aExtrusionPropVec.push_back( aProp );
        // "FirstLightHarsh"
        const OUString sExtrusionFirstLightHarsh( "FirstLightHarsh" );
        bool bExtrusionFirstLightHarsh = ( GetPropertyValue( DFF_Prop_fc3DFillHarsh, 0 ) & 2 ) != 0;
        aProp.Name = sExtrusionFirstLightHarsh;
        aProp.Value <<= bExtrusionFirstLightHarsh;
        aExtrusionPropVec.push_back( aProp );
        // "SecondLightHarsh"
        const OUString sExtrusionSecondLightHarsh( "SecondLightHarsh" );
        bool bExtrusionSecondLightHarsh = ( GetPropertyValue( DFF_Prop_fc3DFillHarsh, 0 ) & 1 ) != 0;
        aProp.Name = sExtrusionSecondLightHarsh;
        aProp.Value <<= bExtrusionSecondLightHarsh;
        aExtrusionPropVec.push_back( aProp );
        // "FirstLightLevel"
        if ( IsProperty( DFF_Prop_c3DKeyIntensity ) )
        {
            const OUString sExtrusionFirstLightLevel( "FirstLightLevel" );
            double fFirstLightLevel = (sal_Int32)GetPropertyValue( DFF_Prop_c3DKeyIntensity, 0 );
            fFirstLightLevel /= 655.36;
            aProp.Name = sExtrusionFirstLightLevel;
            aProp.Value <<= fFirstLightLevel;
            aExtrusionPropVec.push_back( aProp );
        }
        // "SecondLightLevel"
        if ( IsProperty( DFF_Prop_c3DFillIntensity ) )
        {
            const OUString sExtrusionSecondLightLevel( "SecondLightLevel" );
            double fSecondLightLevel = (sal_Int32)GetPropertyValue( DFF_Prop_c3DFillIntensity, 0 );
            fSecondLightLevel /= 655.36;
            aProp.Name = sExtrusionSecondLightLevel;
            aProp.Value <<= fSecondLightLevel;
            aExtrusionPropVec.push_back( aProp );
        }
        // "FirtstLightDirection"
        if ( IsProperty( DFF_Prop_c3DKeyX ) || IsProperty( DFF_Prop_c3DKeyY ) || IsProperty( DFF_Prop_c3DKeyZ ) )
        {
            double fLightX = (double)((sal_Int32)GetPropertyValue( DFF_Prop_c3DKeyX, 50000 ));
            double fLightY = (double)((sal_Int32)GetPropertyValue( DFF_Prop_c3DKeyY, 0 ));
            double fLightZ = (double)((sal_Int32)GetPropertyValue( DFF_Prop_c3DKeyZ, 10000 ));
            css::drawing::Direction3D aExtrusionFirstLightDirection( fLightX, fLightY, fLightZ );
            const OUString sExtrusionFirstLightDirection( "FirstLightDirection" );
            aProp.Name = sExtrusionFirstLightDirection;
            aProp.Value <<= aExtrusionFirstLightDirection;
            aExtrusionPropVec.push_back( aProp );
        }
        // "SecondLightDirection"
        if ( IsProperty( DFF_Prop_c3DFillX ) || IsProperty( DFF_Prop_c3DFillY ) || IsProperty( DFF_Prop_c3DFillZ ) )
        {
            double fLight2X = (double)((sal_Int32)GetPropertyValue( DFF_Prop_c3DFillX, (sal_uInt32)-50000 ));
            double fLight2Y = (double)((sal_Int32)GetPropertyValue( DFF_Prop_c3DFillY, 0 ));
            double fLight2Z = (double)((sal_Int32)GetPropertyValue( DFF_Prop_c3DFillZ, 10000 ));
            css::drawing::Direction3D aExtrusionSecondLightDirection( fLight2X, fLight2Y, fLight2Z );
            const OUString sExtrusionSecondLightDirection( "SecondLightDirection" );
            aProp.Name = sExtrusionSecondLightDirection;
            aProp.Value <<= aExtrusionSecondLightDirection;
            aExtrusionPropVec.push_back( aProp );
        }

        // "Metal"
        const OUString sExtrusionMetal( "Metal" );
        bool bExtrusionMetal = ( GetPropertyValue( DFF_Prop_fc3DLightFace, 0 ) & 4 ) != 0;
        aProp.Name = sExtrusionMetal;
        aProp.Value <<= bExtrusionMetal;
        aExtrusionPropVec.push_back( aProp );
        // "ShadeMode"
        if ( IsProperty( DFF_Prop_c3DRenderMode ) )
        {
            const OUString sExtrusionShadeMode( "ShadeMode" );
            sal_uInt32 nExtrusionRenderMode = GetPropertyValue( DFF_Prop_c3DRenderMode, 0 );
            css::drawing::ShadeMode eExtrusionShadeMode( css::drawing::ShadeMode_FLAT );
            if ( nExtrusionRenderMode == mso_Wireframe )
                eExtrusionShadeMode = css::drawing::ShadeMode_DRAFT;

            aProp.Name = sExtrusionShadeMode;
            aProp.Value <<= eExtrusionShadeMode;
            aExtrusionPropVec.push_back( aProp );
        }
        // "RotateAngle" in Grad
        if ( IsProperty( DFF_Prop_c3DXRotationAngle ) || IsProperty( DFF_Prop_c3DYRotationAngle ) )
        {
            const OUString sExtrusionAngle( "RotateAngle" );
            double fAngleX = (double)((sal_Int32)GetPropertyValue( DFF_Prop_c3DXRotationAngle, 0 )) / 65536.0;
            double fAngleY = (double)((sal_Int32)GetPropertyValue( DFF_Prop_c3DYRotationAngle, 0 )) / 65536.0;
            EnhancedCustomShapeParameterPair aRotateAnglePair;
            aRotateAnglePair.First.Value <<= fAngleX;
            aRotateAnglePair.First.Type = EnhancedCustomShapeParameterType::NORMAL;
            aRotateAnglePair.Second.Value <<= fAngleY;
            aRotateAnglePair.Second.Type = EnhancedCustomShapeParameterType::NORMAL;
            aProp.Name = sExtrusionAngle;
            aProp.Value <<= aRotateAnglePair;
            aExtrusionPropVec.push_back( aProp );
        }

        // "AutoRotationCenter"
        if ( ( GetPropertyValue( DFF_Prop_fc3DFillHarsh, 0 ) & 8 ) == 0 )
        {
            // "RotationCenter"
            if ( IsProperty( DFF_Prop_c3DRotationCenterX ) || IsProperty( DFF_Prop_c3DRotationCenterY ) || IsProperty( DFF_Prop_c3DRotationCenterZ ) )
            {
                css::drawing::Direction3D aRotationCenter(
                    (double)((sal_Int32)GetPropertyValue( DFF_Prop_c3DRotationCenterX, 0 )) / 360.0,
                    (double)((sal_Int32)GetPropertyValue( DFF_Prop_c3DRotationCenterY, 0 )) / 360.0,
                    (double)((sal_Int32)GetPropertyValue( DFF_Prop_c3DRotationCenterZ, 0 )) / 360.0 );

                const OUString sExtrusionRotationCenter( "RotationCenter" );
                aProp.Name = sExtrusionRotationCenter;
                aProp.Value <<= aRotationCenter;
                aExtrusionPropVec.push_back( aProp );
            }
        }
        // "Shininess"
        if ( IsProperty( DFF_Prop_c3DShininess ) )
        {
            const OUString sExtrusionShininess( "Shininess" );
            double fShininess = (sal_Int32)GetPropertyValue( DFF_Prop_c3DShininess, 0 );
            fShininess /= 655.36;
            aProp.Name = sExtrusionShininess;
            aProp.Value <<= fShininess;
            aExtrusionPropVec.push_back( aProp );
        }
        // "Skew"
        if ( IsProperty( DFF_Prop_c3DSkewAmount ) || IsProperty( DFF_Prop_c3DSkewAngle ) )
        {
            const OUString sExtrusionSkew( "Skew" );
            double fSkewAmount = (sal_Int32)GetPropertyValue( DFF_Prop_c3DSkewAmount, 50 );
            double fSkewAngle = (double)((sal_Int32)GetPropertyValue( DFF_Prop_c3DSkewAngle, sal::static_int_cast< sal_uInt32 >(-135 * 65536) )) / 65536.0;

            EnhancedCustomShapeParameterPair aSkewPair;
            aSkewPair.First.Value <<= fSkewAmount;
            aSkewPair.First.Type = EnhancedCustomShapeParameterType::NORMAL;
            aSkewPair.Second.Value <<= fSkewAngle;
            aSkewPair.Second.Type = EnhancedCustomShapeParameterType::NORMAL;
            aProp.Name = sExtrusionSkew;
            aProp.Value <<= aSkewPair;
            aExtrusionPropVec.push_back( aProp );
        }
        // "Specularity"
        if ( IsProperty( DFF_Prop_c3DSpecularAmt ) )
        {
            const OUString sExtrusionSpecularity( "Specularity" );
            double fSpecularity = (sal_Int32)GetPropertyValue( DFF_Prop_c3DSpecularAmt, 0 );
            fSpecularity /= 1333;
            aProp.Name = sExtrusionSpecularity;
            aProp.Value <<= fSpecularity;
            aExtrusionPropVec.push_back( aProp );
        }
        // "ProjectionMode"
        const OUString sExtrusionProjectionMode( "ProjectionMode" );
        ProjectionMode eProjectionMode = (GetPropertyValue( DFF_Prop_fc3DFillHarsh, 0 ) & 4) ? ProjectionMode_PARALLEL : ProjectionMode_PERSPECTIVE;
        aProp.Name = sExtrusionProjectionMode;
        aProp.Value <<= eProjectionMode;
        aExtrusionPropVec.push_back( aProp );

        // "ViewPoint" in 1/100mm
        if ( IsProperty( DFF_Prop_c3DXViewpoint ) || IsProperty( DFF_Prop_c3DYViewpoint ) || IsProperty( DFF_Prop_c3DZViewpoint ) )
        {
            double fViewX = (double)((sal_Int32)GetPropertyValue( DFF_Prop_c3DXViewpoint, 1250000 )) / 360.0;
            double fViewY = (double)((sal_Int32)GetPropertyValue( DFF_Prop_c3DYViewpoint, (sal_uInt32)-1250000 ))/ 360.0;
            double fViewZ = (double)((sal_Int32)GetPropertyValue( DFF_Prop_c3DZViewpoint, 9000000 )) / 360.0;
            css::drawing::Position3D aExtrusionViewPoint( fViewX, fViewY, fViewZ );
            const OUString sExtrusionViewPoint( "ViewPoint" );
            aProp.Name = sExtrusionViewPoint;
            aProp.Value <<= aExtrusionViewPoint;
            aExtrusionPropVec.push_back( aProp );
        }
        // "Origin"
        if ( IsProperty( DFF_Prop_c3DOriginX ) || IsProperty( DFF_Prop_c3DOriginY ) )
        {
            const OUString sExtrusionOrigin( "Origin" );
            double fOriginX = (double)((sal_Int32)GetPropertyValue( DFF_Prop_c3DOriginX, 32768 ));
            double fOriginY = (double)((sal_Int32)GetPropertyValue( DFF_Prop_c3DOriginY, (sal_uInt32)-32768 ));
            fOriginX /= 65536;
            fOriginY /= 65536;
            EnhancedCustomShapeParameterPair aOriginPair;
            aOriginPair.First.Value <<= fOriginX;
            aOriginPair.First.Type = EnhancedCustomShapeParameterType::NORMAL;
            aOriginPair.Second.Value <<= fOriginY;
            aOriginPair.Second.Type = EnhancedCustomShapeParameterType::NORMAL;
            aProp.Name = sExtrusionOrigin;
            aProp.Value <<= aOriginPair;
            aExtrusionPropVec.push_back( aProp );
        }
        // "ExtrusionColor"
        const OUString sExtrusionColor( "Color" );
        bool bExtrusionColor = IsProperty( DFF_Prop_c3DExtrusionColor );    // ( GetPropertyValue( DFF_Prop_fc3DLightFace ) & 2 ) != 0;
        aProp.Name = sExtrusionColor;
        aProp.Value <<= bExtrusionColor;
        aExtrusionPropVec.push_back( aProp );
        if ( IsProperty( DFF_Prop_c3DExtrusionColor ) )
            rSet.Put( XSecondaryFillColorItem( OUString(), rManager.MSO_CLR_ToColor(
                GetPropertyValue( DFF_Prop_c3DExtrusionColor, 0 ), DFF_Prop_c3DExtrusionColor ) ) );
        // pushing the whole Extrusion element
        aProp.Name = "Extrusion";
        aProp.Value <<= comphelper::containerToSequence(aExtrusionPropVec);
        aPropVec.push_back( aProp );
    }


    // "Equations" PropertySequence element

    if ( IsProperty( DFF_Prop_pFormulas ) )
    {
        sal_uInt16 nNumElem = 0;

        if ( SeekToContent( DFF_Prop_pFormulas, rIn ) )
        {
            sal_uInt16 nNumElemMem = 0;
            sal_uInt16 nElemSize = 8;
            rIn.ReadUInt16( nNumElem ).ReadUInt16( nNumElemMem ).ReadUInt16( nElemSize );
        }
        if ( nNumElem <= 128 )
        {
            uno::Sequence< OUString > aEquations( nNumElem );
            for ( sal_uInt16 i = 0; i < nNumElem; i++ )
            {
                sal_Int16 nP1(0), nP2(0), nP3(0);
                sal_uInt16 nFlags(0);
                rIn.ReadUInt16( nFlags ).ReadInt16( nP1 ).ReadInt16( nP2 ).ReadInt16( nP3 );
                aEquations[ i ] = EnhancedCustomShape2d::GetEquation( nFlags, nP1, nP2, nP3 );
            }
            // pushing the whole Equations element
            const OUString sEquations( "Equations" );
            aProp.Name = sEquations;
            aProp.Value <<= aEquations;
            aPropVec.push_back( aProp );
        }
    }


    // "Handles" PropertySequence element

    if ( IsProperty( DFF_Prop_Handles ) )
    {
        sal_uInt16 nNumElem = 0;
        sal_uInt16 nElemSize = 36;

        if ( SeekToContent( DFF_Prop_Handles, rIn ) )
        {
            sal_uInt16 nNumElemMem = 0;
            rIn.ReadUInt16( nNumElem ).ReadUInt16( nNumElemMem ).ReadUInt16( nElemSize );
        }
        bool bImport = false;
        if (nElemSize == 36)
        {
            //sanity check that the stream is long enough to fulfill nNumElem * nElemSize;
            bImport = rIn.remainingSize() / nElemSize >= nNumElem;
        }
        if (bImport)
        {
            uno::Sequence< beans::PropertyValues > aHandles( nNumElem );
            for ( sal_uInt16 i = 0; i < nNumElem; i++ )
            {
                PropVec aHandlePropVec;
                sal_uInt32  nFlagsTmp;
                SvxMSDffHandleFlags nFlags;
                sal_Int32   nPositionX, nPositionY, nCenterX, nCenterY, nRangeXMin, nRangeXMax, nRangeYMin, nRangeYMax;
                rIn.ReadUInt32( nFlagsTmp )
                   .ReadInt32( nPositionX )
                   .ReadInt32( nPositionY )
                   .ReadInt32( nCenterX )
                   .ReadInt32( nCenterY )
                   .ReadInt32( nRangeXMin )
                   .ReadInt32( nRangeXMax )
                   .ReadInt32( nRangeYMin )
                   .ReadInt32( nRangeYMax );
                nFlags = static_cast<SvxMSDffHandleFlags>(nFlagsTmp);
                if ( nPositionX == 2 )  // replacing center position with absolute value
                    nPositionX = nCoordWidth / 2;
                if ( nPositionY == 2 )
                    nPositionY = nCoordHeight / 2;
                EnhancedCustomShapeParameterPair aPosition;
                EnhancedCustomShape2d::SetEnhancedCustomShapeHandleParameter( aPosition.First,  nPositionX, true, true  );
                EnhancedCustomShape2d::SetEnhancedCustomShapeHandleParameter( aPosition.Second, nPositionY, true, false );
                const OUString sHandlePosition( "Position" );
                aProp.Name = sHandlePosition;
                aProp.Value <<= aPosition;
                aHandlePropVec.push_back( aProp );

                if ( nFlags & SvxMSDffHandleFlags::MIRRORED_X )
                {
                    bool bMirroredX = true;
                    const OUString sHandleMirroredX( "MirroredX" );
                    aProp.Name = sHandleMirroredX;
                    aProp.Value <<= bMirroredX;
                    aHandlePropVec.push_back( aProp );
                }
                if ( nFlags & SvxMSDffHandleFlags::MIRRORED_Y )
                {
                    bool bMirroredY = true;
                    const OUString sHandleMirroredY( "MirroredY" );
                    aProp.Name = sHandleMirroredY;
                    aProp.Value <<= bMirroredY;
                    aHandlePropVec.push_back( aProp );
                }
                if ( nFlags & SvxMSDffHandleFlags::SWITCHED )
                {
                    bool bSwitched = true;
                    const OUString sHandleSwitched( "Switched" );
                    aProp.Name = sHandleSwitched;
                    aProp.Value <<= bSwitched;
                    aHandlePropVec.push_back( aProp );
                }
                if ( nFlags & SvxMSDffHandleFlags::POLAR )
                {
                    if ( nCenterX == 2 )
                        nCenterX = nCoordWidth / 2;
                    if ( nCenterY == 2 )
                        nCenterY = nCoordHeight / 2;
                    if ( ( nPositionY >= 0x256 ) || ( nPositionY <= 0x107 ) )   // position y
                        nAdjustmentsWhichNeedsToBeConverted |= ( 1 << i );
                    EnhancedCustomShapeParameterPair aPolar;
                    EnhancedCustomShape2d::SetEnhancedCustomShapeHandleParameter( aPolar.First,  nCenterX, bool( nFlags & SvxMSDffHandleFlags::CENTER_X_IS_SPECIAL ), true  );
                    EnhancedCustomShape2d::SetEnhancedCustomShapeHandleParameter( aPolar.Second, nCenterY, bool( nFlags & SvxMSDffHandleFlags::CENTER_Y_IS_SPECIAL ), false );
                    const OUString sHandlePolar( "Polar" );
                    aProp.Name = sHandlePolar;
                    aProp.Value <<= aPolar;
                    aHandlePropVec.push_back( aProp );
                }
                if ( nFlags & SvxMSDffHandleFlags::MAP )
                {
                    if ( nCenterX == 2 )
                        nCenterX = nCoordWidth / 2;
                    if ( nCenterY == 2 )
                        nCenterY = nCoordHeight / 2;
                    EnhancedCustomShapeParameterPair aMap;
                    EnhancedCustomShape2d::SetEnhancedCustomShapeHandleParameter( aMap.First,  nCenterX, bool( nFlags & SvxMSDffHandleFlags::CENTER_X_IS_SPECIAL ), true  );
                    EnhancedCustomShape2d::SetEnhancedCustomShapeHandleParameter( aMap.Second, nCenterY, bool( nFlags & SvxMSDffHandleFlags::CENTER_Y_IS_SPECIAL ), false );
                    const OUString sHandleMap( "Map" );
                    aProp.Name = sHandleMap;
                    aProp.Value <<= aMap;
                    aHandlePropVec.push_back( aProp );
                }
                if ( nFlags & SvxMSDffHandleFlags::RANGE )
                {
                    if ( (sal_uInt32)nRangeXMin != 0x80000000 )
                    {
                        if ( nRangeXMin == 2 )
                            nRangeXMin = nCoordWidth / 2;
                        EnhancedCustomShapeParameter aRangeXMinimum;
                        EnhancedCustomShape2d::SetEnhancedCustomShapeHandleParameter( aRangeXMinimum,  nRangeXMin,
                            bool( nFlags & SvxMSDffHandleFlags::RANGE_X_MIN_IS_SPECIAL ), true  );
                        const OUString sHandleRangeXMinimum( "RangeXMinimum" );
                        aProp.Name = sHandleRangeXMinimum;
                        aProp.Value <<= aRangeXMinimum;
                        aHandlePropVec.push_back( aProp );
                    }
                    if ( (sal_uInt32)nRangeXMax != 0x7fffffff )
                    {
                        if ( nRangeXMax == 2 )
                            nRangeXMax = nCoordWidth / 2;
                        EnhancedCustomShapeParameter aRangeXMaximum;
                        EnhancedCustomShape2d::SetEnhancedCustomShapeHandleParameter( aRangeXMaximum, nRangeXMax,
                            bool( nFlags & SvxMSDffHandleFlags::RANGE_X_MAX_IS_SPECIAL ), false );
                        const OUString sHandleRangeXMaximum( "RangeXMaximum" );
                        aProp.Name = sHandleRangeXMaximum;
                        aProp.Value <<= aRangeXMaximum;
                        aHandlePropVec.push_back( aProp );
                    }
                    if ( (sal_uInt32)nRangeYMin != 0x80000000 )
                    {
                        if ( nRangeYMin == 2 )
                            nRangeYMin = nCoordHeight / 2;
                        EnhancedCustomShapeParameter aRangeYMinimum;
                        EnhancedCustomShape2d::SetEnhancedCustomShapeHandleParameter( aRangeYMinimum, nRangeYMin,
                            bool( nFlags & SvxMSDffHandleFlags::RANGE_Y_MIN_IS_SPECIAL ), true );
                        const OUString sHandleRangeYMinimum( "RangeYMinimum" );
                        aProp.Name = sHandleRangeYMinimum;
                        aProp.Value <<= aRangeYMinimum;
                        aHandlePropVec.push_back( aProp );
                    }
                    if ( (sal_uInt32)nRangeYMax != 0x7fffffff )
                    {
                        if ( nRangeYMax == 2 )
                            nRangeYMax = nCoordHeight / 2;
                        EnhancedCustomShapeParameter aRangeYMaximum;
                        EnhancedCustomShape2d::SetEnhancedCustomShapeHandleParameter( aRangeYMaximum, nRangeYMax,
                            bool( nFlags & SvxMSDffHandleFlags::RANGE_Y_MAX_IS_SPECIAL ), false );
                        const OUString sHandleRangeYMaximum( "RangeYMaximum" );
                        aProp.Name = sHandleRangeYMaximum;
                        aProp.Value <<= aRangeYMaximum;
                        aHandlePropVec.push_back( aProp );
                    }
                }
                if ( nFlags & SvxMSDffHandleFlags::RADIUS_RANGE )
                {
                    if ( (sal_uInt32)nRangeXMin != 0x7fffffff )
                    {
                        if ( nRangeXMin == 2 )
                            nRangeXMin = nCoordWidth / 2;
                        EnhancedCustomShapeParameter aRadiusRangeMinimum;
                        EnhancedCustomShape2d::SetEnhancedCustomShapeHandleParameter( aRadiusRangeMinimum, nRangeXMin,
                            bool( nFlags & SvxMSDffHandleFlags::RANGE_X_MIN_IS_SPECIAL ), true  );
                        const OUString sHandleRadiusRangeMinimum( "RadiusRangeMinimum" );
                        aProp.Name = sHandleRadiusRangeMinimum;
                        aProp.Value <<= aRadiusRangeMinimum;
                        aHandlePropVec.push_back( aProp );
                    }
                    if ( (sal_uInt32)nRangeXMax != 0x80000000 )
                    {
                        if ( nRangeXMax == 2 )
                            nRangeXMax = nCoordWidth / 2;
                        EnhancedCustomShapeParameter aRadiusRangeMaximum;
                        EnhancedCustomShape2d::SetEnhancedCustomShapeHandleParameter( aRadiusRangeMaximum, nRangeXMax,
                            bool( nFlags & SvxMSDffHandleFlags::RANGE_X_MAX_IS_SPECIAL ), false );
                        const OUString sHandleRadiusRangeMaximum( "RadiusRangeMaximum" );
                        aProp.Name = sHandleRadiusRangeMaximum;
                        aProp.Value <<= aRadiusRangeMaximum;
                        aHandlePropVec.push_back( aProp );
                    }
                }
                if ( !aHandlePropVec.empty() )
                {
                    aHandles[ i ] = comphelper::containerToSequence(aHandlePropVec);
                }
            }
            // pushing the whole Handles element
            aProp.Name = "Handles";
            aProp.Value <<= aHandles;
            aPropVec.push_back( aProp );
        }
    }
    else
    {
        const mso_CustomShape* pDefCustomShape = GetCustomShapeContent( rObjData.eShapeType );
        if ( pDefCustomShape && pDefCustomShape->nHandles && pDefCustomShape->pHandles )
        {
            sal_Int32 i, nCnt = pDefCustomShape->nHandles;
            const SvxMSDffHandle* pData = pDefCustomShape->pHandles;
            for ( i = 0; i < nCnt; i++, pData++ )
            {
                if ( pData->nFlags & SvxMSDffHandleFlags::POLAR )
                {
                    if ( ( pData->nPositionY >= 0x256 ) || ( pData->nPositionY <= 0x107 ) )
                        nAdjustmentsWhichNeedsToBeConverted |= ( 1 << i );
                }
            }
        }
    }

    // "Path" PropertySequence element

    {
        PropVec aPathPropVec;

        // "Path/ExtrusionAllowed"
        if ( IsHardAttribute( DFF_Prop_f3DOK ) )
        {
            const OUString sExtrusionAllowed( "ExtrusionAllowed" );
            bool bExtrusionAllowed = ( GetPropertyValue( DFF_Prop_fFillOK, 0 ) & 16 ) != 0;
            aProp.Name = sExtrusionAllowed;
            aProp.Value <<= bExtrusionAllowed;
            aPathPropVec.push_back( aProp );
        }
        // "Path/ConcentricGradientFillAllowed"
        if ( IsHardAttribute( DFF_Prop_fFillShadeShapeOK ) )
        {
            const OUString sConcentricGradientFillAllowed( "ConcentricGradientFillAllowed" );
            bool bConcentricGradientFillAllowed = ( GetPropertyValue( DFF_Prop_fFillOK, 0 ) & 2 ) != 0;
            aProp.Name = sConcentricGradientFillAllowed;
            aProp.Value <<= bConcentricGradientFillAllowed;
            aPathPropVec.push_back( aProp );
        }
        // "Path/TextPathAllowed"
        if ( IsHardAttribute( DFF_Prop_fGtextOK ) || ( GetPropertyValue( DFF_Prop_gtextFStrikethrough, 0 ) & 0x4000 ) )
        {
            const OUString sTextPathAllowed( "TextPathAllowed" );
            bool bTextPathAllowed = ( GetPropertyValue( DFF_Prop_fFillOK, 0 ) & 4 ) != 0;
            aProp.Name = sTextPathAllowed;
            aProp.Value <<= bTextPathAllowed;
            aPathPropVec.push_back( aProp );
        }
        // Path/Coordinates
        if ( IsProperty( DFF_Prop_pVertices ) )
        {
            css::uno::Sequence< css::drawing::EnhancedCustomShapeParameterPair > aCoordinates;
            sal_uInt16 nNumElemVert = 0;
            sal_uInt16 nElemSizeVert = 8;

            if ( SeekToContent( DFF_Prop_pVertices, rIn ) )
            {
                sal_uInt16 nNumElemMemVert = 0;
                rIn.ReadUInt16( nNumElemVert ).ReadUInt16( nNumElemMemVert ).ReadUInt16( nElemSizeVert );
            }
            if (nElemSizeVert != 8)
                nElemSizeVert = 4;
            //sanity check that the stream is long enough to fulfill nNumElem * nElemSize;
            bool bImport = rIn.remainingSize() / nElemSizeVert >= nNumElemVert;
            if (bImport)
            {
                aCoordinates.realloc( nNumElemVert );
                for (sal_uInt16 i = 0; i < nNumElemVert; ++i)
                {
                    sal_Int32 nX(0), nY(0);

                    if ( nElemSizeVert == 8 )
                    {
                        rIn.ReadInt32( nX )
                           .ReadInt32( nY );
                    }
                    else
                    {
                        sal_Int16 nTmpA(0), nTmpB(0);
                        rIn.ReadInt16( nTmpA )
                           .ReadInt16( nTmpB );

                        nX = nTmpA;
                        nY = nTmpB;
                    }
                    EnhancedCustomShape2d::SetEnhancedCustomShapeParameter( aCoordinates[ i ].First, nX );
                    EnhancedCustomShape2d::SetEnhancedCustomShapeParameter( aCoordinates[ i ].Second, nY );
                }
            }
            const OUString sCoordinates( "Coordinates" );
            aProp.Name = sCoordinates;
            aProp.Value <<= aCoordinates;
            aPathPropVec.push_back( aProp );
        }
        // Path/Segments
        if ( IsProperty( DFF_Prop_pSegmentInfo ) )
        {
            css::uno::Sequence< css::drawing::EnhancedCustomShapeSegment > aSegments;

            sal_uInt16 i, nTmp;
            sal_uInt16 nNumElemSeg = 0;

            if ( SeekToContent( DFF_Prop_pSegmentInfo, rIn ) )
            {
                sal_uInt16 nNumElemMemSeg = 0;
                sal_uInt16 nElemSizeSeg = 2;
                rIn.ReadUInt16( nNumElemSeg ).ReadUInt16( nNumElemMemSeg ).ReadUInt16( nElemSizeSeg );
            }
            std::size_t nMaxEntriesPossible = rIn.remainingSize() / sizeof(sal_uInt16);
            if (nNumElemSeg > nMaxEntriesPossible)
            {
                SAL_WARN("filter.ms", "NumElem list is longer than remaining bytes, ppt or parser is wrong");
                nNumElemSeg = nMaxEntriesPossible;
            }
            if ( nNumElemSeg )
            {
                sal_Int16 nCommand;
                sal_Int16 nCnt;
                aSegments.realloc( nNumElemSeg );
                for ( i = 0; i < nNumElemSeg; i++ )
                {
                    rIn.ReadUInt16( nTmp );
                    nCommand = EnhancedCustomShapeSegmentCommand::UNKNOWN;
                    nCnt = (sal_Int16)( nTmp & 0x1fff );//Last 13 bits for segment points number
                    switch( nTmp >> 13 )//First 3 bits for command type
                    {
                        case 0x0:
                            nCommand = EnhancedCustomShapeSegmentCommand::LINETO;
                            if ( !nCnt ) nCnt = 1;
                            break;
                        case 0x1:
                            nCommand = EnhancedCustomShapeSegmentCommand::CURVETO;
                            if ( !nCnt ) nCnt = 1;
                            break;
                        case 0x2:
                            nCommand = EnhancedCustomShapeSegmentCommand::MOVETO;
                            if ( !nCnt ) nCnt = 1;
                            break;
                        case 0x3:
                            nCommand = EnhancedCustomShapeSegmentCommand::CLOSESUBPATH;
                            nCnt = 0;
                            break;
                        case 0x4:
                            nCommand = EnhancedCustomShapeSegmentCommand::ENDSUBPATH;
                            nCnt = 0;
                            break;
                        case 0x5:
                        case 0x6:
                        {
                            switch ( ( nTmp >> 8 ) & 0x1f )//5 bits next to command type is for path escape type
                            {
                                case 0x0:
                                {
                                    //It is msopathEscapeExtension which is transformed into LINETO.
                                    //If issue happens, I think this part can be comment so that it will be taken as unknown command.
                                    //When export, origin data will be export without any change.
                                    nCommand = EnhancedCustomShapeSegmentCommand::LINETO;
                                    if ( !nCnt )
                                        nCnt = 1;
                                }
                                break;
                                case 0x1:
                                {
                                    nCommand = EnhancedCustomShapeSegmentCommand::ANGLEELLIPSETO;
                                    nCnt = ( nTmp & 0xff ) / 3;
                                }
                                break;
                                case 0x2:
                                {
                                    nCommand = EnhancedCustomShapeSegmentCommand::ANGLEELLIPSE;
                                    nCnt = ( nTmp & 0xff ) / 3;
                                }
                                break;
                                case 0x3:
                                {
                                    nCommand = EnhancedCustomShapeSegmentCommand::ARCTO;
                                    nCnt = ( nTmp & 0xff ) >> 2;
                                };
                                break;
                                case 0x4:
                                {
                                    nCommand = EnhancedCustomShapeSegmentCommand::ARC;
                                    nCnt = ( nTmp & 0xff ) >> 2;
                                }
                                break;
                                case 0x5:
                                {
                                    nCommand = EnhancedCustomShapeSegmentCommand::CLOCKWISEARCTO;
                                    nCnt = ( nTmp & 0xff ) >> 2;
                                }
                                break;
                                case 0x6:
                                {
                                    nCommand = EnhancedCustomShapeSegmentCommand::CLOCKWISEARC;
                                    nCnt = ( nTmp & 0xff ) >> 2;
                                }
                                break;
                                case 0x7:
                                {
                                    nCommand = EnhancedCustomShapeSegmentCommand::ELLIPTICALQUADRANTX;
                                    nCnt = nTmp & 0xff;
                                }
                                break;
                                case 0x8:
                                {
                                    nCommand = EnhancedCustomShapeSegmentCommand::ELLIPTICALQUADRANTY;
                                    nCnt = nTmp & 0xff;
                                }
                                break;
                                case 0xa: nCommand = EnhancedCustomShapeSegmentCommand::NOFILL; nCnt = 0; break;
                                case 0xb: nCommand = EnhancedCustomShapeSegmentCommand::NOSTROKE; nCnt = 0; break;
                            }
                        }
                        break;
                    }
                    // if the command is unknown, we will store all the data in nCnt, so it will be possible to export without loss
                    if ( nCommand == EnhancedCustomShapeSegmentCommand::UNKNOWN )
                        nCnt = (sal_Int16)nTmp;
                    aSegments[ i ].Command = nCommand;
                    aSegments[ i ].Count = nCnt;
                }
            }
            const OUString sSegments( "Segments" );
            aProp.Name = sSegments;
            aProp.Value <<= aSegments;
            aPathPropVec.push_back( aProp );
        }
        // Path/StretchX
        if ( IsProperty( DFF_Prop_stretchPointX ) )
        {
            const OUString sStretchX( "StretchX" );
            sal_Int32 nStretchX = GetPropertyValue( DFF_Prop_stretchPointX, 0 );
            aProp.Name = sStretchX;
            aProp.Value <<= nStretchX;
            aPathPropVec.push_back( aProp );
        }
        // Path/StretchX
        if ( IsProperty( DFF_Prop_stretchPointY ) )
        {
            const OUString sStretchY( "StretchY" );
            sal_Int32 nStretchY = GetPropertyValue( DFF_Prop_stretchPointY, 0 );
            aProp.Name = sStretchY;
            aProp.Value <<= nStretchY;
            aPathPropVec.push_back( aProp );
        }
        // Path/TextFrames
        if ( IsProperty( DFF_Prop_textRectangles ) )
        {
            sal_uInt16 nNumElem = 0;
            sal_uInt16 nElemSize = 16;

            if ( SeekToContent( DFF_Prop_textRectangles, rIn ) )
            {
                sal_uInt16 nNumElemMem = 0;
                rIn.ReadUInt16( nNumElem ).ReadUInt16( nNumElemMem ).ReadUInt16( nElemSize );
            }
            bool bImport = false;
            if (nElemSize == 16)
            {
                //sanity check that the stream is long enough to fulfill nNumElem * nElemSize;
                bImport = rIn.remainingSize() / nElemSize >= nNumElem;
            }
            if (bImport)
            {
                css::uno::Sequence< css::drawing::EnhancedCustomShapeTextFrame > aTextFrames( nNumElem );
                for (sal_uInt16 i = 0; i < nNumElem; ++i)
                {
                    sal_Int32 nLeft(0), nTop(0), nRight(0), nBottom(0);

                    rIn.ReadInt32( nLeft )
                       .ReadInt32( nTop )
                       .ReadInt32( nRight )
                       .ReadInt32( nBottom );

                    EnhancedCustomShape2d::SetEnhancedCustomShapeParameter( aTextFrames[ i ].TopLeft.First,  nLeft );
                    EnhancedCustomShape2d::SetEnhancedCustomShapeParameter( aTextFrames[ i ].TopLeft.Second, nTop  );
                    EnhancedCustomShape2d::SetEnhancedCustomShapeParameter( aTextFrames[ i ].BottomRight.First,  nRight );
                    EnhancedCustomShape2d::SetEnhancedCustomShapeParameter( aTextFrames[ i ].BottomRight.Second, nBottom);
                }
                const OUString sTextFrames( "TextFrames" );
                aProp.Name = sTextFrames;
                aProp.Value <<= aTextFrames;
                aPathPropVec.push_back( aProp );
            }
        }
        //Path/GluePoints
        if ( IsProperty( DFF_Prop_connectorPoints ) )
        {
            css::uno::Sequence< css::drawing::EnhancedCustomShapeParameterPair > aGluePoints;
            sal_uInt16 nNumElemVert = 0;
            sal_uInt16 nNumElemMemVert = 0;
            sal_uInt16 nElemSizeVert = 8;

            if ( SeekToContent( DFF_Prop_connectorPoints, rIn ) )
                rIn.ReadUInt16( nNumElemVert ).ReadUInt16( nNumElemMemVert ).ReadUInt16( nElemSizeVert );

            bool bImport = false;
            if (nNumElemVert)
            {
                //sanity check that the stream is long enough to fulfill nNumElemVert * nElemSizeVert;
                bImport = rIn.remainingSize() / nElemSizeVert >= nNumElemVert;
            }

            if (bImport)
            {
                aGluePoints.realloc( nNumElemVert );
                for (sal_uInt16 i = 0; i < nNumElemVert; ++i)
                {
                    sal_Int32 nX(0), nY(0);
                    if ( nElemSizeVert == 8 )
                    {
                        rIn.ReadInt32( nX )
                           .ReadInt32( nY );
                    }
                    else
                    {
                        sal_Int16 nTmpA(0), nTmpB(0);

                        rIn.ReadInt16( nTmpA )
                           .ReadInt16( nTmpB );

                        nX = nTmpA;
                        nY = nTmpB;
                    }
                    EnhancedCustomShape2d::SetEnhancedCustomShapeParameter( aGluePoints[ i ].First,  nX );
                    EnhancedCustomShape2d::SetEnhancedCustomShapeParameter( aGluePoints[ i ].Second, nY );
                }
            }
            const OUString sGluePoints( "GluePoints" );
            aProp.Name = sGluePoints;
            aProp.Value <<= aGluePoints;
            aPathPropVec.push_back( aProp );
        }
        if ( IsProperty( DFF_Prop_connectorType ) )
        {
            sal_Int16 nGluePointType = (sal_uInt16)GetPropertyValue( DFF_Prop_connectorType, 0 );
            const OUString sGluePointType( "GluePointType" );
            aProp.Name = sGluePointType;
            aProp.Value <<= nGluePointType;
            aPathPropVec.push_back( aProp );
        }
        // pushing the whole Path element
        if ( !aPathPropVec.empty() )
        {
            aProp.Name = "Path";
            aProp.Value <<= comphelper::containerToSequence(aPathPropVec);
            aPropVec.push_back( aProp );
        }
    }

    // "TextPath" PropertySequence element

    bool bTextPathOn = ( GetPropertyValue( DFF_Prop_gtextFStrikethrough, 0 ) & 0x4000 ) != 0;
    if ( bTextPathOn )
    {
        PropVec aTextPathPropVec;

        // TextPath
        const OUString sTextPathOn( "TextPath" );
        aProp.Name = sTextPathOn;
        aProp.Value <<= bTextPathOn;
        aTextPathPropVec.push_back( aProp );

        // TextPathMode
        const OUString sTextPathMode( "TextPathMode" );
        bool bTextPathFitPath = ( GetPropertyValue( DFF_Prop_gtextFStrikethrough, 0 ) & 0x100 ) != 0;

        bool bTextPathFitShape;
        if ( IsHardAttribute( DFF_Prop_gtextFStretch ) )
            bTextPathFitShape = ( GetPropertyValue( DFF_Prop_gtextFStrikethrough, 0 ) & 0x400 ) != 0;
        else
        {
            bTextPathFitShape = true;
            switch( rObjData.eShapeType )
            {
                case mso_sptTextArchUpCurve :
                case mso_sptTextArchDownCurve :
                case mso_sptTextCircleCurve :
                case mso_sptTextButtonCurve :
                    bTextPathFitShape = false;
                    break;
                default : break;
            }
        }
        EnhancedCustomShapeTextPathMode eTextPathMode( EnhancedCustomShapeTextPathMode_NORMAL );
        if ( bTextPathFitShape )
            eTextPathMode = EnhancedCustomShapeTextPathMode_SHAPE;
        else if ( bTextPathFitPath )
            eTextPathMode = EnhancedCustomShapeTextPathMode_PATH;
        aProp.Name = sTextPathMode;
        aProp.Value <<= eTextPathMode;
        aTextPathPropVec.push_back( aProp );

        // ScaleX
        const OUString sTextPathScaleX( "ScaleX" );
        bool bTextPathScaleX = ( GetPropertyValue( DFF_Prop_gtextFStrikethrough, 0 ) & 0x40 ) != 0;
        aProp.Name = sTextPathScaleX;
        aProp.Value <<= bTextPathScaleX;
        aTextPathPropVec.push_back( aProp );
        // SameLetterHeights
        const OUString sSameLetterHeight( "SameLetterHeights" );
        bool bSameLetterHeight = ( GetPropertyValue( DFF_Prop_gtextFStrikethrough, 0 ) & 0x80 ) != 0;
        aProp.Name = sSameLetterHeight;
        aProp.Value <<= bSameLetterHeight;
        aTextPathPropVec.push_back( aProp );

        // pushing the whole TextPath element
        aProp.Name = "TextPath";
        aProp.Value <<= comphelper::containerToSequence(aTextPathPropVec);
        aPropVec.push_back( aProp );
    }

    // "AdjustmentValues" // The AdjustmentValues are imported at last, because depending to the type of the
    //////////////////////// handle (POLAR) we will convert the adjustment value from a fixed float to double

    // checking the last used adjustment handle, so we can determine how many handles are to allocate
    sal_uInt32 i = DFF_Prop_adjust10Value;
    while ( ( i >= DFF_Prop_adjustValue ) && !IsProperty( i ) )
        i--;
    sal_Int32 nAdjustmentValues = ( i - DFF_Prop_adjustValue ) + 1;
    if ( nAdjustmentValues )
    {
        uno::Sequence< css::drawing::EnhancedCustomShapeAdjustmentValue > aAdjustmentSeq( nAdjustmentValues );
        while( --nAdjustmentValues >= 0 )
        {
            sal_Int32 nValue = 0;
            beans::PropertyState ePropertyState = beans::PropertyState_DEFAULT_VALUE;
            if ( IsProperty( i ) )
            {
                nValue = GetPropertyValue( i, 0 );
                ePropertyState = beans::PropertyState_DIRECT_VALUE;
            }
            if ( nAdjustmentsWhichNeedsToBeConverted & ( 1 << ( i - DFF_Prop_adjustValue ) ) )
            {
                double fValue = nValue;
                fValue /= 65536;
                aAdjustmentSeq[ nAdjustmentValues ].Value <<= fValue;
            }
            else
                aAdjustmentSeq[ nAdjustmentValues ].Value <<= nValue;
            aAdjustmentSeq[ nAdjustmentValues ].State = ePropertyState;
            i--;
        }
        const OUString sAdjustmentValues( "AdjustmentValues" );
        aProp.Name = sAdjustmentValues;
        aProp.Value <<= aAdjustmentSeq;
        aPropVec.push_back( aProp );
    }

    // creating the whole property set
    rSet.Put( SdrCustomShapeGeometryItem( comphelper::containerToSequence(aPropVec) ) );
}

void DffPropertyReader::ApplyAttributes( SvStream& rIn, SfxItemSet& rSet ) const
{
    Rectangle aEmptyRect;
    DffRecordHeader aHdTemp;
    DffObjData aDffObjTemp( aHdTemp, aEmptyRect, 0 );
    ApplyAttributes( rIn, rSet, aDffObjTemp );
}

void DffPropertyReader::ApplyAttributes( SvStream& rIn, SfxItemSet& rSet, DffObjData& rObjData ) const
{
    bool bHasShadow = false;
    bool bNonZeroShadowOffset = false;

    if ( IsProperty( DFF_Prop_gtextSize ) )
        rSet.Put( SvxFontHeightItem( rManager.ScalePt( GetPropertyValue( DFF_Prop_gtextSize, 0 ) ), 100, EE_CHAR_FONTHEIGHT ) );
    sal_uInt32 nFontAttributes = GetPropertyValue( DFF_Prop_gtextFStrikethrough, 0 );
    if ( nFontAttributes & 0x20 )
        rSet.Put( SvxWeightItem( (nFontAttributes & 0x20) ? WEIGHT_BOLD : WEIGHT_NORMAL, EE_CHAR_WEIGHT ) );
    if ( nFontAttributes & 0x10 )
        rSet.Put( SvxPostureItem( (nFontAttributes & 0x10) ? ITALIC_NORMAL : ITALIC_NONE, EE_CHAR_ITALIC ) );
    if ( nFontAttributes & 0x08 )
        rSet.Put( SvxUnderlineItem( (nFontAttributes & 0x08) ? LINESTYLE_SINGLE : LINESTYLE_NONE, EE_CHAR_UNDERLINE ) );
    if ( nFontAttributes & 0x40 )
        rSet.Put( SvxShadowedItem( (nFontAttributes & 0x40) != 0, EE_CHAR_SHADOW ) );
//    if ( nFontAttributes & 0x02 )
//        rSet.Put( SvxCaseMapItem( nFontAttributes & 0x02 ? SVX_CASEMAP_KAPITAELCHEN : SVX_CASEMAP_NOT_MAPPED ) );
    if ( nFontAttributes & 0x01 )
        rSet.Put( SvxCrossedOutItem( (nFontAttributes & 0x01) ? STRIKEOUT_SINGLE : STRIKEOUT_NONE, EE_CHAR_STRIKEOUT ) );
    if ( IsProperty( DFF_Prop_fillColor ) )
        rSet.Put( XFillColorItem( OUString(), rManager.MSO_CLR_ToColor( GetPropertyValue( DFF_Prop_fillColor, 0 ), DFF_Prop_fillColor ) ) );
    if ( IsProperty( DFF_Prop_shadowColor ) )
        rSet.Put( makeSdrShadowColorItem( rManager.MSO_CLR_ToColor( GetPropertyValue( DFF_Prop_shadowColor, 0 ), DFF_Prop_shadowColor ) ) );
    else
    {
        //The default value for this property is 0x00808080
        rSet.Put( makeSdrShadowColorItem( rManager.MSO_CLR_ToColor( 0x00808080, DFF_Prop_shadowColor ) ) );
    }
    if ( IsProperty( DFF_Prop_shadowOpacity ) )
        rSet.Put( makeSdrShadowTransparenceItem( (sal_uInt16)( ( 0x10000 - GetPropertyValue( DFF_Prop_shadowOpacity, 0 ) ) / 655 ) ) );
    if ( IsProperty( DFF_Prop_shadowOffsetX ) )
    {
        sal_Int32 nVal = static_cast< sal_Int32 >( GetPropertyValue( DFF_Prop_shadowOffsetX, 0 ) );
        rManager.ScaleEmu( nVal );
        rSet.Put( makeSdrShadowXDistItem( nVal ) );
        bNonZeroShadowOffset = ( nVal > 0 );
    }
    if ( IsProperty( DFF_Prop_shadowOffsetY ) )
    {
        sal_Int32 nVal = static_cast< sal_Int32 >( GetPropertyValue( DFF_Prop_shadowOffsetY, 0 ) );
        rManager.ScaleEmu( nVal );
        rSet.Put( makeSdrShadowYDistItem( nVal ) );
        bNonZeroShadowOffset = ( nVal > 0 );
    }
    if ( IsProperty( DFF_Prop_fshadowObscured ) )
    {
        bHasShadow = ( GetPropertyValue( DFF_Prop_fshadowObscured, 0 ) & 2 ) != 0;
        if ( bHasShadow )
        {
            if ( !IsProperty( DFF_Prop_shadowOffsetX ) )
                rSet.Put( makeSdrShadowXDistItem( 35 ) );
            if ( !IsProperty( DFF_Prop_shadowOffsetY ) )
                rSet.Put( makeSdrShadowYDistItem( 35 ) );
        }
    }
    if ( IsProperty( DFF_Prop_shadowType ) )
    {
        MSO_ShadowType eShadowType = static_cast< MSO_ShadowType >( GetPropertyValue( DFF_Prop_shadowType, 0 ) );
        if( eShadowType != mso_shadowOffset && !bNonZeroShadowOffset )
        {
            //0.12" == 173 twip == 302 100mm
            sal_uInt32 nDist = rManager.pSdrModel->GetScaleUnit() == MapUnit::MapTwip ? 173: 302;
            rSet.Put( makeSdrShadowXDistItem( nDist ) );
            rSet.Put( makeSdrShadowYDistItem( nDist ) );
        }
    }
    if ( bHasShadow )
    {
        static bool bCheckShadow(false);

        // #i124477# Found no reason not to set shadow, esp. since it is applied to evtl. existing text
        // and will lead to an error if in PPT someone used text and added the object shadow to the
        // object carrying that text. I found no cases where this leads to problems (the old bugtracker
        // task #160376# from sj is unfortunately no longer available). Keeping the code for now
        // to allow easy fallback when this shows problems in the future
        if(bCheckShadow)
        {
            // #160376# sj: activating shadow only if fill and or linestyle is used
            // this is required because of the latest drawing layer core changes.
            // #i104085# is related to this.
            sal_uInt32 nLineFlags(GetPropertyValue( DFF_Prop_fNoLineDrawDash, 0 ));
            if(!IsHardAttribute( DFF_Prop_fLine ) && !IsCustomShapeStrokedByDefault( rObjData.eShapeType ))
                nLineFlags &= ~0x08;
            sal_uInt32 nFillFlags(GetPropertyValue( DFF_Prop_fNoFillHitTest, 0 ));
            if(!IsHardAttribute( DFF_Prop_fFilled ) && !IsCustomShapeFilledByDefault( rObjData.eShapeType ))
                nFillFlags &= ~0x10;
            if ( nFillFlags & 0x10 )
            {
                MSO_FillType eMSO_FillType = (MSO_FillType)GetPropertyValue( DFF_Prop_fillType, mso_fillSolid );
                switch( eMSO_FillType )
                {
                    case mso_fillSolid :
                    case mso_fillPattern :
                    case mso_fillTexture :
                    case mso_fillPicture :
                    case mso_fillShade :
                    case mso_fillShadeCenter :
                    case mso_fillShadeShape :
                    case mso_fillShadeScale :
                    case mso_fillShadeTitle :
                    break;
                    default:
                        nFillFlags &=~0x10;         // no fillstyle used
                    break;
                }
            }
            if ( ( ( nLineFlags & 0x08 ) == 0 ) && ( ( nFillFlags & 0x10 ) == 0 ) && ( rObjData.eShapeType != mso_sptPictureFrame ))    // if there is no fillstyle and linestyle
                bHasShadow = false;                                             // we are turning shadow off.
        }

        if ( bHasShadow )
            rSet.Put( makeSdrShadowItem( bHasShadow ) );
    }
    ApplyLineAttributes( rSet, rObjData.eShapeType ); // #i28269#
    ApplyFillAttributes( rIn, rSet, rObjData );
    if ( rObjData.eShapeType != mso_sptNil || IsProperty( DFF_Prop_pVertices ) )
    {
        ApplyCustomShapeGeometryAttributes( rIn, rSet, rObjData );
        ApplyCustomShapeTextAttributes( rSet );
        if ( rManager.GetSvxMSDffSettings() & SVXMSDFF_SETTINGS_IMPORT_EXCEL )
        {
            if ( mnFix16Angle || ( rObjData.nSpFlags & SP_FFLIPV ) )
                CheckAndCorrectExcelTextRotation( rIn, rSet, rObjData );
        }
    }
}

void DffPropertyReader::CheckAndCorrectExcelTextRotation( SvStream& rIn, SfxItemSet& rSet, DffObjData& rObjData ) const
{
    bool bRotateTextWithShape = rObjData.bRotateTextWithShape;
    if ( rObjData.bOpt2 )        // sj: #158494# is the second property set available ? if then we have to check the xml data of
    {                            // the shape, because the textrotation of Excel 2003 and greater versions is stored there
                                // (upright property of the textbox)
        if ( rManager.pSecPropSet->SeekToContent( DFF_Prop_metroBlob, rIn ) )
        {
            sal_uInt32 nLen = rManager.pSecPropSet->GetPropertyValue( DFF_Prop_metroBlob, 0 );
            if ( nLen )
            {
                css::uno::Sequence< sal_Int8 > aXMLDataSeq( nLen );
                rIn.ReadBytes(aXMLDataSeq.getArray(), nLen);
                css::uno::Reference< css::io::XInputStream > xInputStream
                    ( new ::comphelper::SequenceInputStream( aXMLDataSeq ) );
                try
                {
                    css::uno::Reference< css::uno::XComponentContext > xContext( ::comphelper::getProcessComponentContext() );
                    css::uno::Reference< css::embed::XStorage > xStorage
                        ( ::comphelper::OStorageHelper::GetStorageOfFormatFromInputStream(
                            OFOPXML_STORAGE_FORMAT_STRING, xInputStream, xContext, true ) );
                    if ( xStorage.is() )
                    {
                        const OUString sDRS( "drs" );
                        css::uno::Reference< css::embed::XStorage >
                            xStorageDRS( xStorage->openStorageElement( sDRS, css::embed::ElementModes::SEEKABLEREAD ) );
                        if ( xStorageDRS.is() )
                        {
                            const OUString sShapeXML( "shapexml.xml" );
                            css::uno::Reference< css::io::XStream > xShapeXMLStream( xStorageDRS->openStreamElement( sShapeXML, css::embed::ElementModes::SEEKABLEREAD ) );
                            if ( xShapeXMLStream.is() )
                            {
                                css::uno::Reference< css::io::XInputStream > xShapeXMLInputStream( xShapeXMLStream->getInputStream() );
                                if ( xShapeXMLInputStream.is() )
                                {
                                    css::uno::Sequence< sal_Int8 > aSeq;
                                    sal_Int32 nBytesRead = xShapeXMLInputStream->readBytes( aSeq, 0x7fffffff );
                                    if ( nBytesRead )
                                    {    // for only one property I spare to use a XML parser at this point, this
                                        // should be enhanced if needed

                                        bRotateTextWithShape = true;    // using the correct xml default
                                        const char* pArry = reinterpret_cast< char* >( aSeq.getArray() );
                                        const char* const pUpright = "upright=";
                                        const char* pEnd = pArry + nBytesRead;
                                        const char* pPtr = pArry;
                                        while( ( pPtr + 12 ) < pEnd )
                                        {
                                            if ( !memcmp( pUpright, pPtr, 8 ) )
                                            {
                                                bRotateTextWithShape = ( pPtr[ 9 ] != '1' ) && ( pPtr[ 9 ] != 't' );
                                                break;
                                            }
                                            else
                                                pPtr++;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                catch( css::uno::Exception& )
                {
                }
            }
        }
    }
    if ( !bRotateTextWithShape )
    {
        const css::uno::Any* pAny;
        SdrCustomShapeGeometryItem aGeometryItem(static_cast<const SdrCustomShapeGeometryItem&>(rSet.Get( SDRATTR_CUSTOMSHAPE_GEOMETRY )));
        const OUString sTextRotateAngle( "TextRotateAngle" );
        pAny = aGeometryItem.GetPropertyValueByName( sTextRotateAngle );
        double fExtraTextRotateAngle = 0.0;
        if ( pAny )
            *pAny >>= fExtraTextRotateAngle;

        if ( rManager.mnFix16Angle )
            fExtraTextRotateAngle += mnFix16Angle / 100.0;
        if ( rObjData.nSpFlags & SP_FFLIPV )
            fExtraTextRotateAngle -= 180.0;

        css::beans::PropertyValue aTextRotateAngle;
        aTextRotateAngle.Name = sTextRotateAngle;
        aTextRotateAngle.Value <<= fExtraTextRotateAngle;
        aGeometryItem.SetPropertyValue( aTextRotateAngle );
        rSet.Put( aGeometryItem );
    }
}


void DffPropertyReader::ImportGradientColor( SfxItemSet& aSet,MSO_FillType eMSO_FillType, double dTrans , double dBackTrans) const
{
    //MS Focus prop will impact the start and end color position. And AOO does not
    //support this prop. So need some swap for the two color to keep fidelity with AOO and MS shape.
    //So below var is defined.
    sal_Int32 nChgColors = 0;
    sal_Int32 nAngle = GetPropertyValue( DFF_Prop_fillAngle, 0 );
    sal_Int32 nRotateAngle = 0;
    if(nAngle >= 0)
        nChgColors ^= 1;

    //Translate a MS clockwise(+) or count clockwise angle(-) into a AOO count clock wise angle
    nAngle=3600 - ( ( Fix16ToAngle(nAngle) + 5 ) / 10 );
    //Make sure this angle belongs to 0~3600
    while ( nAngle >= 3600 ) nAngle -= 3600;
    while ( nAngle < 0 ) nAngle += 3600;

    //Rotate angle
    if ( mbRotateGranientFillWithAngle )
    {
        nRotateAngle = GetPropertyValue( DFF_Prop_Rotation, 0 );
        if(nRotateAngle)//fixed point number
            nRotateAngle = ( (sal_Int16)( nRotateAngle >> 16) * 100L ) + ( ( ( nRotateAngle & 0x0000ffff) * 100L ) >> 16 );
        nRotateAngle = ( nRotateAngle + 5 ) / 10 ;//round up
        //nAngle is a clockwise angle. If nRotateAngle is a clockwise angle, then gradient need be rotated a little less
        //Or it need be rotated a little more
        nAngle -=  nRotateAngle;
    }
    while ( nAngle >= 3600 ) nAngle -= 3600;
    while ( nAngle < 0 ) nAngle += 3600;

    css::awt::GradientStyle eGrad = css::awt::GradientStyle_LINEAR;

    sal_Int32 nFocus = GetPropertyValue( DFF_Prop_fillFocus, 0 );
    if ( !nFocus )
        nChgColors ^= 1;
    else if ( nFocus < 0 )//If it is a negative focus, the color will be swapped
    {
        nFocus = -nFocus;
        nChgColors ^= 1;
    }

    if( nFocus > 40 && nFocus < 60 )
    {
        eGrad = css::awt::GradientStyle_AXIAL;//A axial gradient other than linear
        nChgColors ^= 1;
    }
    //if the type is linear or axial, just save focus to nFocusX and nFocusY for export
    //Core function does no need them. They serves for rect gradient(CenterXY).
    sal_uInt16 nFocusX = (sal_uInt16)nFocus;
    sal_uInt16 nFocusY = (sal_uInt16)nFocus;

    switch( eMSO_FillType )
    {
    case mso_fillShadeShape :
        {
            eGrad = css::awt::GradientStyle_RECT;
            nFocusY = nFocusX = 50;
            nChgColors ^= 1;
        }
        break;
    case mso_fillShadeCenter :
        {
            eGrad = css::awt::GradientStyle_RECT;
            //A MS fillTo prop specifies the relative position of the left boundary
            //of the center rectangle in a concentric shaded fill. Use 100 or 0 to keep fidelity
            nFocusX=(GetPropertyValue( DFF_Prop_fillToRight, 0 )==0x10000) ? 100 : 0;
            nFocusY=(GetPropertyValue( DFF_Prop_fillToBottom,0 )==0x10000) ? 100 : 0;
            nChgColors ^= 1;
        }
        break;
        default: break;
    }

    Color aCol1( rManager.MSO_CLR_ToColor( GetPropertyValue( DFF_Prop_fillColor, COL_WHITE ), DFF_Prop_fillColor ) );
    Color aCol2( rManager.MSO_CLR_ToColor( GetPropertyValue( DFF_Prop_fillBackColor, COL_WHITE ), DFF_Prop_fillBackColor ) );
    if ( nChgColors )
    {
        //Swap start and end color
        Color aZwi( aCol1 );
        aCol1 = aCol2;
        aCol2 = aZwi;
        //Swap two colors' transparency
        double dTemp = dTrans;
        dTrans = dBackTrans;
        dBackTrans = dTemp;
    }

    //Construct gradient item
    XGradient aGrad( aCol2, aCol1, eGrad, nAngle, nFocusX, nFocusY );
    //Intensity has been merged into color. So here just set is as 100
    aGrad.SetStartIntens( 100 );
    aGrad.SetEndIntens( 100 );
    aSet.Put( XFillGradientItem( OUString(), aGrad ) );
    //Construct tranparency item. This item can coordinate with both solid and gradient.
    if ( dTrans < 1.0 || dBackTrans < 1.0 )
    {
        sal_uInt8 nStartCol = (sal_uInt8)( (1 - dTrans )* 255 );
        sal_uInt8 nEndCol = (sal_uInt8)( ( 1- dBackTrans ) * 255 );
        aCol1 = Color(nStartCol, nStartCol, nStartCol);
        aCol2 = Color(nEndCol, nEndCol, nEndCol);

        XGradient aGrad2( aCol2 ,  aCol1 , eGrad, nAngle, nFocusX, nFocusY );
        aSet.Put( XFillFloatTransparenceItem( OUString(), aGrad2 ) );
    }
}


//- Record Manager ----------------------------------------------------------


DffRecordList::DffRecordList( DffRecordList* pList ) :
    nCount                  ( 0 ),
    nCurrent                ( 0 ),
    pPrev                   ( pList ),
    pNext                   ( nullptr )
{
    if ( pList )
        pList->pNext.reset( this );
}

DffRecordList::~DffRecordList()
{
}

DffRecordManager::DffRecordManager() :
    DffRecordList   ( nullptr ),
    pCList          ( static_cast<DffRecordList*>(this) )
{
}

DffRecordManager::DffRecordManager( SvStream& rIn ) :
    DffRecordList   ( nullptr ),
    pCList          ( static_cast<DffRecordList*>(this) )
{
    Consume( rIn );
}

void DffRecordManager::Consume( SvStream& rIn, sal_uInt32 nStOfs )
{
    Clear();
    sal_uInt32 nOldPos = rIn.Tell();
    if ( !nStOfs )
    {
        DffRecordHeader aHd;
        bool bOk = ReadDffRecordHeader( rIn, aHd );
        if (bOk && aHd.nRecVer == DFF_PSFLAG_CONTAINER)
            nStOfs = aHd.GetRecEndFilePos();
    }
    if ( nStOfs )
    {
        pCList = static_cast<DffRecordList*>(this);
        while ( pCList->pNext )
            pCList = pCList->pNext.get();
        while (rIn.good() && ( ( rIn.Tell() + 8 ) <=  nStOfs ))
        {
            if ( pCList->nCount == DFF_RECORD_MANAGER_BUF_SIZE )
                pCList = new DffRecordList( pCList );
            if (!ReadDffRecordHeader(rIn, pCList->mHd[ pCList->nCount ]))
                break;
            bool bSeekSucceeded = pCList->mHd[ pCList->nCount++ ].SeekToEndOfRecord(rIn);
            if (!bSeekSucceeded)
                break;
        }
        rIn.Seek( nOldPos );
    }
}

void DffRecordManager::Clear()
{
    pCList = static_cast<DffRecordList*>(this);
    pNext.reset();
    nCurrent = 0;
    nCount = 0;
}

DffRecordHeader* DffRecordManager::Current()
{
    DffRecordHeader* pRet = nullptr;
    if ( pCList->nCurrent < pCList->nCount )
        pRet = &pCList->mHd[ pCList->nCurrent ];
    return pRet;
}

DffRecordHeader* DffRecordManager::First()
{
    DffRecordHeader* pRet = nullptr;
    pCList = static_cast<DffRecordList*>(this);
    if ( pCList->nCount )
    {
        pCList->nCurrent = 0;
        pRet = &pCList->mHd[ 0 ];
    }
    return pRet;
}

DffRecordHeader* DffRecordManager::Next()
{
    DffRecordHeader* pRet = nullptr;
    sal_uInt32 nC = pCList->nCurrent + 1;
    if ( nC < pCList->nCount )
    {
        pCList->nCurrent++;
        pRet = &pCList->mHd[ nC ];
    }
    else if ( pCList->pNext )
    {
        pCList = pCList->pNext.get();
        pCList->nCurrent = 0;
        pRet = &pCList->mHd[ 0 ];
    }
    return pRet;
}

DffRecordHeader* DffRecordManager::Prev()
{
    DffRecordHeader* pRet = nullptr;
    sal_uInt32 nCur = pCList->nCurrent;
    if ( !nCur && pCList->pPrev )
    {
        pCList = pCList->pPrev;
        nCur = pCList->nCount;
    }
    if ( nCur-- )
    {
        pCList->nCurrent = nCur;
        pRet = &pCList->mHd[ nCur ];
    }
    return pRet;
}

DffRecordHeader* DffRecordManager::Last()
{
    DffRecordHeader* pRet = nullptr;
    while ( pCList->pNext )
        pCList = pCList->pNext.get();
    sal_uInt32 nCnt = pCList->nCount;
    if ( nCnt-- )
    {
        pCList->nCurrent = nCnt;
        pRet = &pCList->mHd[ nCnt ];
    }
    return pRet;
}

bool DffRecordManager::SeekToContent( SvStream& rIn, sal_uInt16 nRecId, DffSeekToContentMode eMode )
{
    DffRecordHeader* pHd = GetRecordHeader( nRecId, eMode );
    if ( pHd )
    {
        pHd->SeekToContent( rIn );
        return true;
    }
    else
        return false;
}

DffRecordHeader* DffRecordManager::GetRecordHeader( sal_uInt16 nRecId, DffSeekToContentMode eMode )
{
    sal_uInt32 nOldCurrent = pCList->nCurrent;
    DffRecordList* pOldList = pCList;
    DffRecordHeader* pHd;

    if ( eMode == SEEK_FROM_BEGINNING )
        pHd = First();
    else
        pHd = Next();

    while ( pHd )
    {
        if ( pHd->nRecType == nRecId )
            break;
        pHd = Next();
    }
    if ( !pHd && eMode == SEEK_FROM_CURRENT_AND_RESTART )
    {
        DffRecordHeader* pBreak = &pOldList->mHd[ nOldCurrent ];
        pHd = First();
        if ( pHd )
        {
            while ( pHd != pBreak )
            {
                if ( pHd->nRecType == nRecId )
                    break;
                pHd = Next();
            }
            if ( pHd->nRecType != nRecId )
                pHd = nullptr;
        }
    }
    if ( !pHd )
    {
        pCList = pOldList;
        pOldList->nCurrent = nOldCurrent;
    }
    return pHd;
}


//  private methods


bool CompareSvxMSDffShapeInfoById::operator() (
    std::shared_ptr<SvxMSDffShapeInfo> const& lhs,
    std::shared_ptr<SvxMSDffShapeInfo> const& rhs) const
{
    return lhs->nShapeId < rhs->nShapeId;
}

bool CompareSvxMSDffShapeInfoByTxBxComp::operator() (
    std::shared_ptr<SvxMSDffShapeInfo> const& lhs,
    std::shared_ptr<SvxMSDffShapeInfo> const& rhs) const
{
    return lhs->nTxBxComp < rhs->nTxBxComp;
}

void SvxMSDffManager::Scale( sal_Int32& rVal ) const
{
    if ( bNeedMap )
        rVal = BigMulDiv( rVal, nMapMul, nMapDiv );
}

void SvxMSDffManager::Scale( Point& rPos ) const
{
    rPos.X() += nMapXOfs;
    rPos.Y() += nMapYOfs;
    if ( bNeedMap )
    {
        rPos.X() = BigMulDiv( rPos.X(), nMapMul, nMapDiv );
        rPos.Y() = BigMulDiv( rPos.Y(), nMapMul, nMapDiv );
    }
}

void SvxMSDffManager::Scale( Size& rSiz ) const
{
    if ( bNeedMap )
    {
        rSiz.Width() = BigMulDiv( rSiz.Width(), nMapMul, nMapDiv );
        rSiz.Height() = BigMulDiv( rSiz.Height(), nMapMul, nMapDiv );
    }
}

void SvxMSDffManager::ScaleEmu( sal_Int32& rVal ) const
{
    rVal = BigMulDiv( rVal, nEmuMul, nEmuDiv );
}

sal_uInt32 SvxMSDffManager::ScalePt( sal_uInt32 nVal ) const
{
    MapUnit eMap = pSdrModel->GetScaleUnit();
    Fraction aFact( GetMapFactor( MapUnit::MapPoint, eMap ).X() );
    long aMul = aFact.GetNumerator();
    long aDiv = aFact.GetDenominator() * 65536;
    aFact = Fraction( aMul, aDiv ); // try again to shorten it
    return BigMulDiv( nVal, aFact.GetNumerator(), aFact.GetDenominator() );
}

sal_Int32 SvxMSDffManager::ScalePoint( sal_Int32 nVal ) const
{
    return BigMulDiv( nVal, nPntMul, nPntDiv );
};

void SvxMSDffManager::SetModel(SdrModel* pModel, long nApplicationScale)
{
    pSdrModel = pModel;
    if( pModel && (0 < nApplicationScale) )
    {
        // PPT works in units of 576DPI
        // WW on the other side uses twips, i.e. 1440DPI.
        MapUnit eMap = pSdrModel->GetScaleUnit();
        Fraction aFact( GetMapFactor(MapUnit::MapInch, eMap).X() );
        long nMul=aFact.GetNumerator();
        long nDiv=aFact.GetDenominator()*nApplicationScale;
        aFact=Fraction(nMul,nDiv); // try again to shorten it
        // For 100TH_MM -> 2540/576=635/144
        // For Twip     -> 1440/576=5/2
        nMapMul  = aFact.GetNumerator();
        nMapDiv  = aFact.GetDenominator();
        bNeedMap = nMapMul!=nMapDiv;

        // MS-DFF-Properties are mostly given in EMU (English Metric Units)
        // 1mm=36000emu, 1twip=635emu
        aFact=GetMapFactor(MapUnit::Map100thMM,eMap).X();
        nMul=aFact.GetNumerator();
        nDiv=aFact.GetDenominator()*360;
        aFact=Fraction(nMul,nDiv); // try again to shorten it
        // For 100TH_MM ->                            1/360
        // For Twip     -> 14,40/(25,4*360)=144/91440=1/635
        nEmuMul=aFact.GetNumerator();
        nEmuDiv=aFact.GetDenominator();

        // And something for typographic Points
        aFact=GetMapFactor(MapUnit::MapPoint,eMap).X();
        nPntMul=aFact.GetNumerator();
        nPntDiv=aFact.GetDenominator();
    }
    else
    {
        pModel = nullptr;
        nMapMul = nMapDiv = nMapXOfs = nMapYOfs = nEmuMul = nEmuDiv = nPntMul = nPntDiv = 0;
        bNeedMap = false;
    }
}

bool SvxMSDffManager::SeekToShape( SvStream& rSt, void* /* pClientData */, sal_uInt32 nId ) const
{
    bool bRet = false;
    if ( !maFidcls.empty() )
    {
        sal_uInt32 nMerk = rSt.Tell();
        sal_uInt32 nShapeId, nSec = ( nId >> 10 ) - 1;
        if ( nSec < mnIdClusters )
        {
            OffsetMap::const_iterator it = maDgOffsetTable.find( maFidcls[ nSec ].dgid );
            if ( it != maDgOffsetTable.end() )
            {
                sal_IntPtr nOfs = it->second;
                rSt.Seek( nOfs );
                DffRecordHeader aEscherF002Hd;
                bool bOk = ReadDffRecordHeader( rSt, aEscherF002Hd );
                sal_uLong nEscherF002End = bOk ? aEscherF002Hd.GetRecEndFilePos() : 0;
                while (rSt.good() && rSt.Tell() < nEscherF002End)
                {
                    DffRecordHeader aEscherObjListHd;
                    if (!ReadDffRecordHeader(rSt, aEscherObjListHd))
                        break;
                    if ( aEscherObjListHd.nRecVer != 0xf )
                        aEscherObjListHd.SeekToEndOfRecord( rSt );
                    else if ( aEscherObjListHd.nRecType == DFF_msofbtSpContainer )
                    {
                        DffRecordHeader aShapeHd;
                        if ( SeekToRec( rSt, DFF_msofbtSp, aEscherObjListHd.GetRecEndFilePos(), &aShapeHd ) )
                        {
                            rSt.ReadUInt32( nShapeId );
                            if ( nId == nShapeId )
                            {
                                aEscherObjListHd.SeekToBegOfRecord( rSt );
                                bRet = true;
                                break;
                            }
                        }
                        aEscherObjListHd.SeekToEndOfRecord( rSt );
                    }
                }
            }
        }
        if ( !bRet )
            rSt.Seek( nMerk );
    }
    return bRet;
}

bool SvxMSDffManager::SeekToRec( SvStream& rSt, sal_uInt16 nRecId, sal_uLong nMaxFilePos, DffRecordHeader* pRecHd, sal_uLong nSkipCount )
{
    bool bRet = false;
    sal_uLong nFPosMerk = rSt.Tell(); // store FilePos to restore it later if necessary
    do
    {
        DffRecordHeader aHd;
        if (!ReadDffRecordHeader(rSt, aHd))
            break;
        if (aHd.nRecLen > nMaxLegalDffRecordLength)
            break;
        if ( aHd.nRecType == nRecId )
        {
            if ( nSkipCount )
                nSkipCount--;
            else
            {
                bRet = true;
                if ( pRecHd != nullptr )
                    *pRecHd = aHd;
                else
                {
                    bool bSeekSuccess = aHd.SeekToBegOfRecord(rSt);
                    if (!bSeekSuccess)
                    {
                        bRet = false;
                        break;
                    }
                }
            }
        }
        if ( !bRet )
        {
            bool bSeekSuccess = aHd.SeekToEndOfRecord(rSt);
            if (!bSeekSuccess)
                break;
        }
    }
    while ( rSt.good() && rSt.Tell() < nMaxFilePos && !bRet );
    if ( !bRet )
        rSt.Seek( nFPosMerk );  // restore original FilePos
    return bRet;
}

bool SvxMSDffManager::SeekToRec2( sal_uInt16 nRecId1, sal_uInt16 nRecId2, sal_uLong nMaxFilePos ) const
{
    bool bRet = false;
    sal_uLong nFPosMerk = rStCtrl.Tell();   // remember FilePos for conditionally later restoration
    do
    {
        DffRecordHeader aHd;
        if (!ReadDffRecordHeader(rStCtrl, aHd))
            break;
        if ( aHd.nRecType == nRecId1 || aHd.nRecType == nRecId2 )
        {
            bRet = true;
            bool bSeekSuccess = aHd.SeekToBegOfRecord(rStCtrl);
            if (!bSeekSuccess)
            {
                bRet = false;
                break;
            }
        }
        if ( !bRet )
        {
            bool bSeekSuccess = aHd.SeekToEndOfRecord(rStCtrl);
            if (!bSeekSuccess)
                break;
        }
    }
    while ( rStCtrl.good() && rStCtrl.Tell() < nMaxFilePos && !bRet );
    if ( !bRet )
        rStCtrl.Seek( nFPosMerk ); // restore FilePos
    return bRet;
}


bool SvxMSDffManager::GetColorFromPalette( sal_uInt16 /* nNum */, Color& rColor ) const
{
    // This method has to be overwritten in the class
    // derived for the excel export
    rColor.SetColor( COL_WHITE );
    return true;
}

// sj: the documentation is not complete, especially in ppt the normal rgb for text
// color is written as 0xfeRRGGBB, this can't be explained by the documentation, nearly
// every bit in the upper code is set -> so there seems to be a special handling for
// ppt text colors, i decided not to fix this in MSO_CLR_ToColor because of possible
// side effects, instead MSO_TEXT_CLR_ToColor is called for PPT text colors, to map
// the color code to something that behaves like the other standard color codes used by
// fill and line color
Color SvxMSDffManager::MSO_TEXT_CLR_ToColor( sal_uInt32 nColorCode ) const
{
    // for text colors: Header is 0xfeRRGGBB
    if ( ( nColorCode & 0xfe000000 ) == 0xfe000000 )
        nColorCode &= 0x00ffffff;
    else
    {
        // for colorscheme colors the color index are the lower three bits of the upper byte
        if ( ( nColorCode & 0xf8000000 ) == 0 ) // this must be a colorscheme index
        {
            nColorCode >>= 24;
            nColorCode |= 0x8000000;
        }
    }
    return MSO_CLR_ToColor( nColorCode );
}

Color SvxMSDffManager::MSO_CLR_ToColor( sal_uInt32 nColorCode, sal_uInt16 nContentProperty ) const
{
    Color aColor( mnDefaultColor );

    // for text colors: Header is 0xfeRRGGBB
    if ( ( nColorCode & 0xfe000000 ) == 0xfe000000 )    // sj: it needs to be checked if 0xfe is used in
        nColorCode &= 0x00ffffff;                       // other cases than ppt text -> if not this code can be removed

    sal_uInt8 nUpper = (sal_uInt8)( nColorCode >> 24 );

    // sj: below change from 0x1b to 0x19 was done because of i84812 (0x02 -> rgb color),
    // now I have some problems to fix i104685 (there the color value is 0x02000000 whichs requires
    // a 0x2 scheme color to be displayed properly), the color docu seems to be incomplete
    if( nUpper & 0x19 )      // if( nUpper & 0x1f )
    {
        if( ( nUpper & 0x08 ) || ( ( nUpper & 0x10 ) == 0 ) )
        {
            // SCHEMECOLOR
            if ( !GetColorFromPalette( ( nUpper & 8 ) ? (sal_uInt16)nColorCode : nUpper, aColor ) )
            {
                switch( nContentProperty )
                {
                    case DFF_Prop_pictureTransparent :
                    case DFF_Prop_shadowColor :
                    case DFF_Prop_fillBackColor :
                    case DFF_Prop_fillColor :
                        aColor = Color( COL_WHITE );
                    break;
                    case DFF_Prop_lineColor :
                    {
                        aColor = Color( COL_BLACK );
                    }
                    break;
                }
            }
        }
        else    // SYSCOLOR
        {
            const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();

            sal_uInt16 nParameter = sal_uInt16(( nColorCode >> 16 ) & 0x00ff);  // the HiByte of nParameter is not zero, an exclusive AND is helping :o
            sal_uInt16 nFunctionBits = (sal_uInt16)( ( nColorCode & 0x00000f00 ) >> 8 );
            sal_uInt16 nAdditionalFlags = (sal_uInt16)( ( nColorCode & 0x0000f000) >> 8 );
            sal_uInt16 nColorIndex = sal_uInt16(nColorCode & 0x00ff);
            sal_uInt32 nPropColor = 0;

            sal_uInt16  nCProp = 0;

            switch ( nColorIndex )
            {
                case mso_syscolorButtonFace :           aColor = rStyleSettings.GetFaceColor(); break;
                case mso_syscolorWindowText :           aColor = rStyleSettings.GetWindowTextColor(); break;
                case mso_syscolorMenu :                 aColor = rStyleSettings.GetMenuColor(); break;
                case mso_syscolor3DLight :
                case mso_syscolorButtonHighlight :
                case mso_syscolorHighlight :            aColor = rStyleSettings.GetHighlightColor(); break;
                case mso_syscolorHighlightText :        aColor = rStyleSettings.GetHighlightTextColor(); break;
                case mso_syscolorCaptionText :          aColor = rStyleSettings.GetMenuTextColor(); break;
                case mso_syscolorActiveCaption :        aColor = rStyleSettings.GetHighlightColor(); break;
                case mso_syscolorButtonShadow :         aColor = rStyleSettings.GetShadowColor(); break;
                case mso_syscolorButtonText :           aColor = rStyleSettings.GetButtonTextColor(); break;
                case mso_syscolorGrayText :             aColor = rStyleSettings.GetDeactiveColor(); break;
                case mso_syscolorInactiveCaption :      aColor = rStyleSettings.GetDeactiveColor(); break;
                case mso_syscolorInactiveCaptionText :  aColor = rStyleSettings.GetDeactiveColor(); break;
                case mso_syscolorInfoBackground :       aColor = rStyleSettings.GetFaceColor(); break;
                case mso_syscolorInfoText :             aColor = rStyleSettings.GetLabelTextColor(); break;
                case mso_syscolorMenuText :             aColor = rStyleSettings.GetMenuTextColor(); break;
                case mso_syscolorScrollbar :            aColor = rStyleSettings.GetFaceColor(); break;
                case mso_syscolorWindow :               aColor = rStyleSettings.GetWindowColor(); break;
                case mso_syscolorWindowFrame :          aColor = rStyleSettings.GetWindowColor(); break;

                case mso_colorFillColor :
                {
                    nPropColor = GetPropertyValue( DFF_Prop_fillColor, 0xffffff );
                    nCProp = DFF_Prop_fillColor;
                }
                break;
                case mso_colorLineOrFillColor :     // ( use the line color only if there is a line )
                {
                    if ( GetPropertyValue( DFF_Prop_fNoLineDrawDash, 0 ) & 8 )
                    {
                        nPropColor = GetPropertyValue( DFF_Prop_lineColor, 0 );
                        nCProp = DFF_Prop_lineColor;
                    }
                    else
                    {
                        nPropColor = GetPropertyValue( DFF_Prop_fillColor, 0xffffff );
                        nCProp = DFF_Prop_fillColor;
                    }
                }
                break;
                case mso_colorLineColor :
                {
                    nPropColor = GetPropertyValue( DFF_Prop_lineColor, 0 );
                    nCProp = DFF_Prop_lineColor;
                }
                break;
                case mso_colorShadowColor :
                {
                    nPropColor = GetPropertyValue( DFF_Prop_shadowColor, 0x808080 );
                    nCProp = DFF_Prop_shadowColor;
                }
                break;
                case mso_colorThis :                // ( use this color ... )
                {
                    nPropColor = GetPropertyValue( DFF_Prop_fillColor, 0xffffff );  //?????????????
                    nCProp = DFF_Prop_fillColor;
                }
                break;
                case mso_colorFillBackColor :
                {
                    nPropColor = GetPropertyValue( DFF_Prop_fillBackColor, 0xffffff );
                    nCProp = DFF_Prop_fillBackColor;
                }
                break;
                case mso_colorLineBackColor :
                {
                    nPropColor = GetPropertyValue( DFF_Prop_lineBackColor, 0xffffff );
                    nCProp = DFF_Prop_lineBackColor;
                }
                break;
                case mso_colorFillThenLine :        // ( use the fillcolor unless no fill and line )
                {
                    nPropColor = GetPropertyValue( DFF_Prop_fillColor, 0xffffff );  //?????????????
                    nCProp = DFF_Prop_fillColor;
                }
                break;
                case mso_colorIndexMask :           // ( extract the color index ) ?
                {
                    nPropColor = GetPropertyValue( DFF_Prop_fillColor, 0xffffff );  //?????????????
                    nCProp = DFF_Prop_fillColor;
                }
                break;
            }
            if ( nCProp && ( nPropColor & 0x10000000 ) == 0 )       // beware of looping recursive
                aColor = MSO_CLR_ToColor( nPropColor, nCProp );

            if( nAdditionalFlags & 0x80 )           // make color gray
            {
                sal_uInt8 nZwi = aColor.GetLuminance();
                aColor = Color( nZwi, nZwi, nZwi );
            }
            switch( nFunctionBits )
            {
                case 0x01 :     // darken color by parameter
                {
                    aColor.SetRed( sal::static_int_cast< sal_uInt8 >( ( nParameter * aColor.GetRed() ) >> 8 ) );
                    aColor.SetGreen( sal::static_int_cast< sal_uInt8 >( ( nParameter * aColor.GetGreen() ) >> 8 ) );
                    aColor.SetBlue( sal::static_int_cast< sal_uInt8 >( ( nParameter * aColor.GetBlue() ) >> 8 ) );
                }
                break;
                case 0x02 :     // lighten color by parameter
                {
                    sal_uInt16 nInvParameter = ( 0x00ff - nParameter ) * 0xff;
                    aColor.SetRed( sal::static_int_cast< sal_uInt8 >( ( nInvParameter + ( nParameter * aColor.GetRed() ) ) >> 8 ) );
                    aColor.SetGreen( sal::static_int_cast< sal_uInt8 >( ( nInvParameter + ( nParameter * aColor.GetGreen() ) ) >> 8 ) );
                    aColor.SetBlue( sal::static_int_cast< sal_uInt8 >( ( nInvParameter + ( nParameter * aColor.GetBlue() ) ) >> 8 ) );
                }
                break;
                case 0x03 :     // add grey level RGB(p,p,p)
                {
                    sal_Int16 nR = (sal_Int16)aColor.GetRed() + (sal_Int16)nParameter;
                    sal_Int16 nG = (sal_Int16)aColor.GetGreen() + (sal_Int16)nParameter;
                    sal_Int16 nB = (sal_Int16)aColor.GetBlue() + (sal_Int16)nParameter;
                    if ( nR > 0x00ff )
                        nR = 0x00ff;
                    if ( nG > 0x00ff )
                        nG = 0x00ff;
                    if ( nB > 0x00ff )
                        nB = 0x00ff;
                    aColor = Color( (sal_uInt8)nR, (sal_uInt8)nG, (sal_uInt8)nB );
                }
                break;
                case 0x04 :     // subtract grey level RGB(p,p,p)
                {
                    sal_Int16 nR = (sal_Int16)aColor.GetRed() - (sal_Int16)nParameter;
                    sal_Int16 nG = (sal_Int16)aColor.GetGreen() - (sal_Int16)nParameter;
                    sal_Int16 nB = (sal_Int16)aColor.GetBlue() - (sal_Int16)nParameter;
                    if ( nR < 0 )
                        nR = 0;
                    if ( nG < 0 )
                        nG = 0;
                    if ( nB < 0 )
                        nB = 0;
                    aColor = Color( (sal_uInt8)nR, (sal_uInt8)nG, (sal_uInt8)nB );
                }
                break;
                case 0x05 :     // subtract from gray level RGB(p,p,p)
                {
                    sal_Int16 nR = (sal_Int16)nParameter - (sal_Int16)aColor.GetRed();
                    sal_Int16 nG = (sal_Int16)nParameter - (sal_Int16)aColor.GetGreen();
                    sal_Int16 nB = (sal_Int16)nParameter - (sal_Int16)aColor.GetBlue();
                    if ( nR < 0 )
                        nR = 0;
                    if ( nG < 0 )
                        nG = 0;
                    if ( nB < 0 )
                        nB = 0;
                    aColor = Color( (sal_uInt8)nR, (sal_uInt8)nG, (sal_uInt8)nB );
                }
                break;
                case 0x06 :     // per component: black if < p, white if >= p
                {
                    aColor.SetRed( aColor.GetRed() < nParameter ? 0x00 : 0xff );
                    aColor.SetGreen( aColor.GetGreen() < nParameter ? 0x00 : 0xff );
                    aColor.SetBlue( aColor.GetBlue() < nParameter ? 0x00 : 0xff );
                }
                break;
            }
            if ( nAdditionalFlags & 0x40 )                  // top-bit invert
                aColor = Color( aColor.GetRed() ^ 0x80, aColor.GetGreen() ^ 0x80, aColor.GetBlue() ^ 0x80 );

            if ( nAdditionalFlags & 0x20 )                  // invert color
                aColor = Color(0xff - aColor.GetRed(), 0xff - aColor.GetGreen(), 0xff - aColor.GetBlue());
        }
    }
    else if ( ( nUpper & 4 ) && ( ( nColorCode & 0xfffff8 ) == 0 ) )
    {   // case of nUpper == 4 powerpoint takes this as argument for a colorschemecolor
        GetColorFromPalette( nUpper, aColor );
    }
    else    // attributed hard, maybe with hint to SYSTEMRGB
        aColor = Color( (sal_uInt8)nColorCode, (sal_uInt8)( nColorCode >> 8 ), (sal_uInt8)( nColorCode >> 16 ) );
    return aColor;
}

void SvxMSDffManager::ReadObjText( SvStream& rStream, SdrObject* pObj )
{
    DffRecordHeader aRecHd;
    if (!ReadDffRecordHeader(rStream, aRecHd))
        return;
    if( aRecHd.nRecType == DFF_msofbtClientTextbox || aRecHd.nRecType == 0x1022 )
    {
        while (rStream.good() && rStream.Tell() < aRecHd.GetRecEndFilePos())
        {
            DffRecordHeader aHd;
            if (!ReadDffRecordHeader(rStream, aHd))
                break;
            switch( aHd.nRecType )
            {
                case DFF_PST_TextBytesAtom:
                case DFF_PST_TextCharsAtom:
                    {
                        bool bUniCode = ( aHd.nRecType == DFF_PST_TextCharsAtom );
                        sal_uInt32 nBytes = aHd.nRecLen;
                        OUString aStr = MSDFFReadZString( rStream, nBytes, bUniCode );
                        ReadObjText( aStr, pObj );
                    }
                    break;
                default:
                    break;
            }
            aHd.SeekToEndOfRecord( rStream );
        }
    }
}

// sj: I just want to set a string for a text object that may contain multiple
// paragraphs. If I now take a look at the following code I get the impression that
// our outliner is too complicate to be used properly,
void SvxMSDffManager::ReadObjText( const OUString& rText, SdrObject* pObj )
{
    SdrTextObj* pText = dynamic_cast<SdrTextObj*>( pObj  );
    if ( pText )
    {
        SdrOutliner& rOutliner = pText->ImpGetDrawOutliner();
        rOutliner.Init( OutlinerMode::TextObject );

        bool bOldUpdateMode = rOutliner.GetUpdateMode();
        rOutliner.SetUpdateMode( false );
        rOutliner.SetVertical( pText->IsVerticalWriting() );

        sal_Int32 nParaIndex = 0;
        sal_Int32 nParaSize;
        const sal_Unicode* pBuf = rText.getStr();
        const sal_Unicode* pEnd = rText.getStr() + rText.getLength();

        while( pBuf < pEnd )
        {
            const sal_Unicode* pCurrent = pBuf;

            for ( nParaSize = 0; pBuf < pEnd; )
            {
                sal_Unicode nChar = *pBuf++;
                if ( nChar == 0xa )
                {
                    if ( ( pBuf < pEnd ) && ( *pBuf == 0xd ) )
                        pBuf++;
                    break;
                }
                else if ( nChar == 0xd )
                {
                    if ( ( pBuf < pEnd ) && ( *pBuf == 0xa ) )
                        pBuf++;
                    break;
                }
                else
                    ++nParaSize;
            }
            ESelection aSelection( nParaIndex, 0, nParaIndex, 0 );
            OUString aParagraph( pCurrent, nParaSize );
            if ( !nParaIndex && aParagraph.isEmpty() )              // SJ: we are crashing if the first paragraph is empty ?
                aParagraph += " ";                   // otherwise these two lines can be removed.
            rOutliner.Insert( aParagraph, nParaIndex );
            rOutliner.SetParaAttribs( nParaIndex, rOutliner.GetEmptyItemSet() );

            SfxItemSet aParagraphAttribs( rOutliner.GetEmptyItemSet() );
            if ( !aSelection.nStartPos )
                aParagraphAttribs.Put( SfxBoolItem( EE_PARA_BULLETSTATE, false ) );
            aSelection.nStartPos = 0;
            rOutliner.QuickSetAttribs( aParagraphAttribs, aSelection );
            nParaIndex++;
        }
        OutlinerParaObject* pNewText = rOutliner.CreateParaObject();
        rOutliner.Clear();
        rOutliner.SetUpdateMode( bOldUpdateMode );
        pText->SetOutlinerParaObject( pNewText );
    }
}

//static
OUString SvxMSDffManager::MSDFFReadZString(SvStream& rIn,
    sal_uInt32 nLen, bool bUniCode)
{
    if (!nLen)
        return OUString();

    OUString sBuf;

    if( bUniCode )
        sBuf = read_uInt16s_ToOUString(rIn, nLen/2);
    else
        sBuf = read_uInt8s_ToOUString(rIn, nLen, RTL_TEXTENCODING_MS_1252);

    return comphelper::string::stripEnd(sBuf, 0);
}

static Size lcl_GetPrefSize(const Graphic& rGraf, const MapMode& aWanted)
{
    MapMode aPrefMapMode(rGraf.GetPrefMapMode());
    if (aPrefMapMode == aWanted)
        return rGraf.GetPrefSize();
    Size aRetSize;
    if (aPrefMapMode == MapUnit::MapPixel)
    {
        aRetSize = Application::GetDefaultDevice()->PixelToLogic(
            rGraf.GetPrefSize(), aWanted);
    }
    else
    {
        aRetSize = OutputDevice::LogicToLogic(
            rGraf.GetPrefSize(), rGraf.GetPrefMapMode(), aWanted);
    }
    return aRetSize;
}

// sj: if the parameter pSet is null, then the resulting crop bitmap will be stored in rGraf,
// otherwise rGraf is untouched and pSet is used to store the corresponding SdrGrafCropItem
static void lcl_ApplyCropping( const DffPropSet& rPropSet, SfxItemSet* pSet, Graphic& rGraf )
{
    sal_Int32 nCropTop      = (sal_Int32)rPropSet.GetPropertyValue( DFF_Prop_cropFromTop, 0 );
    sal_Int32 nCropBottom   = (sal_Int32)rPropSet.GetPropertyValue( DFF_Prop_cropFromBottom, 0 );
    sal_Int32 nCropLeft     = (sal_Int32)rPropSet.GetPropertyValue( DFF_Prop_cropFromLeft, 0 );
    sal_Int32 nCropRight    = (sal_Int32)rPropSet.GetPropertyValue( DFF_Prop_cropFromRight, 0 );

    if( nCropTop || nCropBottom || nCropLeft || nCropRight )
    {
        double      fFactor;
        Size        aCropSize;
        BitmapEx    aCropBitmap;
        sal_uInt32  nTop( 0 ),  nBottom( 0 ), nLeft( 0 ), nRight( 0 );

        if ( pSet ) // use crop attributes ?
            aCropSize = lcl_GetPrefSize( rGraf, MapUnit::Map100thMM );
        else
        {
            aCropBitmap = rGraf.GetBitmapEx();
            aCropSize = aCropBitmap.GetSizePixel();
        }
        if ( nCropTop )
        {
            fFactor = (double)nCropTop / 65536.0;
            nTop = (sal_uInt32)( ( (double)( aCropSize.Height() + 1 ) * fFactor ) + 0.5 );
        }
        if ( nCropBottom )
        {
            fFactor = (double)nCropBottom / 65536.0;
            nBottom = (sal_uInt32)( ( (double)( aCropSize.Height() + 1 ) * fFactor ) + 0.5 );
        }
        if ( nCropLeft )
        {
            fFactor = (double)nCropLeft / 65536.0;
            nLeft = (sal_uInt32)( ( (double)( aCropSize.Width() + 1 ) * fFactor ) + 0.5 );
        }
        if ( nCropRight )
        {
            fFactor = (double)nCropRight / 65536.0;
            nRight = (sal_uInt32)( ( (double)( aCropSize.Width() + 1 ) * fFactor ) + 0.5 );
        }
        if ( pSet ) // use crop attributes ?
            pSet->Put( SdrGrafCropItem( nLeft, nTop, nRight, nBottom ) );
        else
        {
            Rectangle aCropRect( nLeft, nTop, aCropSize.Width() - nRight, aCropSize.Height() - nBottom );
            aCropBitmap.Crop( aCropRect );
            rGraf = aCropBitmap;
        }
    }
}

SdrObject* SvxMSDffManager::ImportGraphic( SvStream& rSt, SfxItemSet& rSet, const DffObjData& rObjData )
{
    SdrObject*  pRet = nullptr;
    OUString    aFileName;
    OUString    aLinkFileName, aLinkFilterName;
    Rectangle   aVisArea;

    MSO_BlipFlags eFlags = (MSO_BlipFlags)GetPropertyValue( DFF_Prop_pibFlags, mso_blipflagDefault );
    sal_uInt32 nBlipId = GetPropertyValue( DFF_Prop_pib, 0 );
    bool bGrfRead = false,

    // Graphic linked
    bLinkGrf = 0 != ( eFlags & mso_blipflagLinkToFile );
    {
        Graphic aGraf;  // be sure this graphic is deleted before swapping out
        if( SeekToContent( DFF_Prop_pibName, rSt ) )
            aFileName = MSDFFReadZString( rSt, GetPropertyValue( DFF_Prop_pibName, 0 ), true );

        //   AND, OR the following:
        if( !( eFlags & mso_blipflagDoNotSave ) ) // Graphic embedded
        {
            bGrfRead = GetBLIP( nBlipId, aGraf, &aVisArea );
            if ( !bGrfRead )
            {
                /*
                Still no luck, lets look at the end of this record for a FBSE pool,
                this fallback is a specific case for how word does it sometimes
                */
                rObjData.rSpHd.SeekToEndOfRecord( rSt );
                DffRecordHeader aHd;
                bool bOk = ReadDffRecordHeader(rSt, aHd);
                if (bOk && DFF_msofbtBSE == aHd.nRecType)
                {
                    const sal_uLong nSkipBLIPLen = 20;
                    const sal_uLong nSkipShapePos = 4;
                    const sal_uLong nSkipBLIP = 4;
                    const sal_uLong nSkip =
                        nSkipBLIPLen + 4 + nSkipShapePos + 4 + nSkipBLIP;

                    if (nSkip <= aHd.nRecLen)
                    {
                        rSt.SeekRel(nSkip);
                        if (0 == rSt.GetError())
                            bGrfRead = GetBLIPDirect( rSt, aGraf, &aVisArea );
                    }
                }
            }
        }
        if ( bGrfRead )
        {
            // the writer is doing its own cropping, so this part affects only impress and calc,
            // unless we're inside a group, in which case writer doesn't crop either
            if (( GetSvxMSDffSettings() & SVXMSDFF_SETTINGS_CROP_BITMAPS ) || rObjData.nCalledByGroup != 0 )
                lcl_ApplyCropping( *this, ( rObjData.nSpFlags & SP_FOLESHAPE ) == 0 ? &rSet : nullptr, aGraf );

            if ( IsProperty( DFF_Prop_pictureTransparent ) )
            {
                sal_uInt32 nTransColor = GetPropertyValue( DFF_Prop_pictureTransparent, 0 );

                if ( aGraf.GetType() == GraphicType::Bitmap )
                {
                    BitmapEx    aBitmapEx( aGraf.GetBitmapEx() );
                    Bitmap      aBitmap( aBitmapEx.GetBitmap() );
                    Bitmap      aMask( aBitmap.CreateMask( MSO_CLR_ToColor( nTransColor, DFF_Prop_pictureTransparent ), 9 ) );
                    if ( aBitmapEx.IsTransparent() )
                        aMask.CombineSimple( aBitmapEx.GetMask(), BmpCombine::Or );
                    aGraf = BitmapEx( aBitmap, aMask );
                }
            }

            sal_Int32 nContrast = GetPropertyValue( DFF_Prop_pictureContrast, 0x10000 );
            /*
            0x10000 is msoffice 50%
            < 0x10000 is in units of 1/50th of 0x10000 per 1%
            > 0x10000 is in units where
            a msoffice x% is stored as 50/(100-x) * 0x10000

            plus, a (ui) microsoft % ranges from 0 to 100, OOO
            from -100 to 100, so also normalize into that range
            */
            if ( nContrast > 0x10000 )
            {
                double fX = nContrast;
                fX /= 0x10000;
                fX /= 51;   // 50 + 1 to round
                fX = 1/fX;
                nContrast = static_cast<sal_Int32>(fX);
                nContrast -= 100;
                nContrast = -nContrast;
                nContrast = (nContrast-50)*2;
            }
            else if ( nContrast == 0x10000 )
                nContrast = 0;
            else
            {
                nContrast *= 101;   //100 + 1 to round
                nContrast /= 0x10000;
                nContrast -= 100;
            }
            sal_Int16   nBrightness     = (sal_Int16)( (sal_Int32)GetPropertyValue( DFF_Prop_pictureBrightness, 0 ) / 327 );
            sal_Int32   nGamma          = GetPropertyValue( DFF_Prop_pictureGamma, 0x10000 );
            GraphicDrawMode eDrawMode   = GRAPHICDRAWMODE_STANDARD;
            switch ( GetPropertyValue( DFF_Prop_pictureActive, 0 ) & 6 )
            {
                case 4 : eDrawMode = GRAPHICDRAWMODE_GREYS; break;
                case 6 : eDrawMode = GRAPHICDRAWMODE_MONO; break;
                case 0 :
                {
                    //office considers the converted values of (in OOo) 70 to be the
                    //"watermark" values, which can vary slightly due to rounding from the
                    //above values
                    if (( nContrast == -70 ) && ( nBrightness == 70 ))
                    {
                        nContrast = 0;
                        nBrightness = 0;
                        eDrawMode = GRAPHICDRAWMODE_WATERMARK;
                    };
                }
                break;
            }

            if ( nContrast || nBrightness || ( nGamma != 0x10000 ) || ( eDrawMode != GRAPHICDRAWMODE_STANDARD ) )
            {
                // MSO uses a different algorithm for contrast+brightness, LO applies contrast before brightness,
                // while MSO apparently applies half of brightness before contrast and half after. So if only
                // contrast or brightness need to be altered, the result is the same, but if both are involved,
                // there's no way to map that, so just force a conversion of the image.
                bool needsConversion = nContrast != 0 && nBrightness != 0;
                if ( ( rObjData.nSpFlags & SP_FOLESHAPE ) == 0 && !needsConversion )
                {
                    if ( nBrightness )
                        rSet.Put( SdrGrafLuminanceItem( nBrightness ) );
                    if ( nContrast )
                        rSet.Put( SdrGrafContrastItem( (sal_Int16)nContrast ) );
                    if ( nGamma != 0x10000 )
                        rSet.Put( SdrGrafGamma100Item( nGamma / 655 ) );
                    if ( eDrawMode != GRAPHICDRAWMODE_STANDARD )
                        rSet.Put( SdrGrafModeItem( eDrawMode ) );
                }
                else
                {
                    if ( eDrawMode == GRAPHICDRAWMODE_WATERMARK )
                    {
                        nContrast = 60;
                        nBrightness = 70;
                        eDrawMode = GRAPHICDRAWMODE_STANDARD;
                    }
                    switch ( aGraf.GetType() )
                    {
                        case GraphicType::Bitmap :
                        {
                            BitmapEx    aBitmapEx( aGraf.GetBitmapEx() );
                            if ( nBrightness || nContrast || ( nGamma != 0x10000 ) )
                                aBitmapEx.Adjust( nBrightness, (sal_Int16)nContrast, 0, 0, 0, (double)nGamma / 0x10000, false, true );
                            if ( eDrawMode == GRAPHICDRAWMODE_GREYS )
                                aBitmapEx.Convert( BMP_CONVERSION_8BIT_GREYS );
                            else if ( eDrawMode == GRAPHICDRAWMODE_MONO )
                                aBitmapEx.Convert( BMP_CONVERSION_1BIT_THRESHOLD );
                            aGraf = aBitmapEx;

                        }
                        break;

                        case GraphicType::GdiMetafile :
                        {
                            GDIMetaFile aGdiMetaFile( aGraf.GetGDIMetaFile() );
                            if ( nBrightness || nContrast || ( nGamma != 0x10000 ) )
                                aGdiMetaFile.Adjust( nBrightness, (sal_Int16)nContrast, 0, 0, 0, (double)nGamma / 0x10000, false, true );
                            if ( eDrawMode == GRAPHICDRAWMODE_GREYS )
                                aGdiMetaFile.Convert( MtfConversion::N8BitGreys );
                            else if ( eDrawMode == GRAPHICDRAWMODE_MONO )
                                aGdiMetaFile.Convert( MtfConversion::N1BitThreshold );
                            aGraf = aGdiMetaFile;
                        }
                        break;
                        default: break;
                    }
                }
            }
        }

        // should it be an OLE object?
        if( bGrfRead && !bLinkGrf && IsProperty( DFF_Prop_pictureId ) )
        {
            // TODO/LATER: in future probably the correct aspect should be provided here
            sal_Int64 nAspect = embed::Aspects::MSOLE_CONTENT;
            // #i32596# - pass <nCalledByGroup> to method
            pRet = ImportOLE( GetPropertyValue( DFF_Prop_pictureId, 0 ), aGraf, rObjData.aBoundRect, aVisArea, rObjData.nCalledByGroup, nAspect );
        }
        if( !pRet )
        {
            pRet = new SdrGrafObj;
            if( bGrfRead )
                static_cast<SdrGrafObj*>(pRet)->SetGraphic( aGraf );

            if( bLinkGrf && !bGrfRead )     // sj: #i55484# if the graphic was embedded ( bGrfRead == true ) then
            {                               // we do not need to set a link. TODO: not to lose the information where the graphic is linked from
                INetURLObject aAbsURL;
                if ( !INetURLObject( maBaseURL ).GetNewAbsURL( aFileName, &aAbsURL ) )
                {
                    OUString aValidURL;
                    if( osl::FileBase::getFileURLFromSystemPath( aFileName, aValidURL ) == osl::FileBase::E_None )
                        aAbsURL = INetURLObject( aValidURL );
                }
                if( aAbsURL.GetProtocol() != INetProtocol::NotValid )
                {
                    GraphicFilter &rGrfFilter = GraphicFilter::GetGraphicFilter();
                    aLinkFilterName = rGrfFilter.GetImportFormatName(
                        rGrfFilter.GetImportFormatNumberForShortName( aAbsURL.getExtension() ) );
                    aLinkFileName = aAbsURL.GetMainURL( INetURLObject::DecodeMechanism::ToIUri );
                }
                else
                    aLinkFileName = aFileName;
            }
        }

        // set the size from BLIP if there is one
        if ( pRet && bGrfRead && !aVisArea.IsEmpty() )
            pRet->SetBLIPSizeRectangle( aVisArea );

        if (pRet->GetName().isEmpty())                   // SJ 22.02.00 : PPT OLE IMPORT:
        {                                                // name is already set in ImportOLE !!
            // JP 01.12.99: SetName before SetModel - because in the other order the Bug 70098 is active
            if ( ( eFlags & mso_blipflagType ) != mso_blipflagComment )
            {
                INetURLObject aURL;
                aURL.SetSmartURL( aFileName );
                pRet->SetName( aURL.getBase() );
            }
            else
                pRet->SetName( aFileName );
        }
    }
    pRet->SetModel( pSdrModel ); // required for GraphicLink
    pRet->SetLogicRect( rObjData.aBoundRect );

    if ( dynamic_cast<const SdrGrafObj* >(pRet) !=  nullptr )
    {
        if( aLinkFileName.getLength() )
            static_cast<SdrGrafObj*>(pRet)->SetGraphicLink( aLinkFileName, ""/*TODO?*/, aLinkFilterName );

        if ( bLinkGrf && !bGrfRead )
        {
            Graphic aGraf(static_cast<SdrGrafObj*>(pRet)->GetGraphic());
            lcl_ApplyCropping( *this, &rSet, aGraf );
        }
    }

    return pRet;
}

// PptSlidePersistEntry& rPersistEntry, SdPage* pPage
SdrObject* SvxMSDffManager::ImportObj( SvStream& rSt, void* pClientData,
    Rectangle& rClientRect, const Rectangle& rGlobalChildRect, int nCalledByGroup, sal_Int32* pShapeId )
{
    SdrObject* pRet = nullptr;
    DffRecordHeader aObjHd;
    bool bOk = ReadDffRecordHeader(rSt, aObjHd);
    if (bOk && aObjHd.nRecType == DFF_msofbtSpgrContainer)
    {
        pRet = ImportGroup( aObjHd, rSt, pClientData, rClientRect, rGlobalChildRect, nCalledByGroup, pShapeId );
    }
    else if (bOk && aObjHd.nRecType == DFF_msofbtSpContainer)
    {
        pRet = ImportShape( aObjHd, rSt, pClientData, rClientRect, rGlobalChildRect, nCalledByGroup, pShapeId );
    }
    aObjHd.SeekToBegOfRecord( rSt );    // restore FilePos
    return pRet;
}

SdrObject* SvxMSDffManager::ImportGroup( const DffRecordHeader& rHd, SvStream& rSt, void* pClientData,
                                            Rectangle& rClientRect, const Rectangle& rGlobalChildRect,
                                                int nCalledByGroup, sal_Int32* pShapeId )
{
    SdrObject* pRet = nullptr;

    if( pShapeId )
        *pShapeId = 0;

    if (!rHd.SeekToContent(rSt))
        return pRet;

    DffRecordHeader aRecHd;     // the first atom has to be the SpContainer for the GroupObject
    bool bOk = ReadDffRecordHeader(rSt, aRecHd);
    if (bOk && aRecHd.nRecType == DFF_msofbtSpContainer)
    {
        mnFix16Angle = 0;
        if (!aRecHd.SeekToBegOfRecord(rSt))
            return pRet;
        pRet = ImportObj( rSt, pClientData, rClientRect, rGlobalChildRect, nCalledByGroup + 1, pShapeId );
        if ( pRet )
        {
            sal_Int32 nGroupRotateAngle = 0;
            sal_Int32 nSpFlags = nGroupShapeFlags;
            nGroupRotateAngle = mnFix16Angle;

            Rectangle aClientRect( rClientRect );

            Rectangle aGlobalChildRect;
            if ( !nCalledByGroup || rGlobalChildRect.IsEmpty() )
                aGlobalChildRect = GetGlobalChildAnchor( rHd, rSt, aClientRect );
            else
                aGlobalChildRect = rGlobalChildRect;

            if ( ( nGroupRotateAngle > 4500 && nGroupRotateAngle <= 13500 )
                || ( nGroupRotateAngle > 22500 && nGroupRotateAngle <= 31500 ) )
            {
                sal_Int32 nHalfWidth = ( aClientRect.GetWidth() + 1 ) >> 1;
                sal_Int32 nHalfHeight = ( aClientRect.GetHeight() + 1 ) >> 1;
                Point aTopLeft( aClientRect.Left() + nHalfWidth - nHalfHeight,
                                aClientRect.Top() + nHalfHeight - nHalfWidth );
                const long nRotatedWidth = aClientRect.GetHeight();
                const long nRotatedHeight = aClientRect.GetWidth();
                Size aNewSize(nRotatedWidth, nRotatedHeight);
                Rectangle aNewRect( aTopLeft, aNewSize );
                aClientRect = aNewRect;
            }

            // now importing the inner objects of the group
            if (!aRecHd.SeekToEndOfRecord(rSt))
                return pRet;

            while (rSt.good() && ( rSt.Tell() < rHd.GetRecEndFilePos()))
            {
                DffRecordHeader aRecHd2;
                if (!ReadDffRecordHeader(rSt, aRecHd2))
                    break;
                if ( aRecHd2.nRecType == DFF_msofbtSpgrContainer )
                {
                    Rectangle aGroupClientAnchor, aGroupChildAnchor;
                    GetGroupAnchors( aRecHd2, rSt, aGroupClientAnchor, aGroupChildAnchor, aClientRect, aGlobalChildRect );
                    if (!aRecHd2.SeekToBegOfRecord(rSt))
                        return pRet;
                    sal_Int32 nShapeId;
                    SdrObject* pTmp = ImportGroup( aRecHd2, rSt, pClientData, aGroupClientAnchor, aGroupChildAnchor, nCalledByGroup + 1, &nShapeId );
                    if ( pTmp && pRet && static_cast<SdrObjGroup*>(pRet)->GetSubList() )
                    {
                        static_cast<SdrObjGroup*>(pRet)->GetSubList()->NbcInsertObject( pTmp );
                        if( nShapeId )
                            insertShapeId( nShapeId, pTmp );
                    }
                }
                else if ( aRecHd2.nRecType == DFF_msofbtSpContainer )
                {
                    if (!aRecHd2.SeekToBegOfRecord(rSt))
                        return pRet;
                    sal_Int32 nShapeId;
                    SdrObject* pTmp = ImportShape( aRecHd2, rSt, pClientData, aClientRect, aGlobalChildRect, nCalledByGroup + 1, &nShapeId );
                    if ( pTmp && pRet && static_cast<SdrObjGroup*>(pRet)->GetSubList())
                    {
                        static_cast<SdrObjGroup*>(pRet)->GetSubList()->NbcInsertObject( pTmp );
                        if( nShapeId )
                            insertShapeId( nShapeId, pTmp );
                    }
                }
                if (!aRecHd2.SeekToEndOfRecord(rSt))
                    return pRet;
            }

            if ( nGroupRotateAngle )
            {
                double a = nGroupRotateAngle * nPi180;
                pRet->NbcRotate( aClientRect.Center(), nGroupRotateAngle, sin( a ), cos( a ) );
            }
            if ( nSpFlags & SP_FFLIPV )     // Vertical flip?
            {   // BoundRect in aBoundRect
                Point aLeft( aClientRect.Left(), ( aClientRect.Top() + aClientRect.Bottom() ) >> 1 );
                Point aRight( aLeft.X() + 1000, aLeft.Y() );
                pRet->NbcMirror( aLeft, aRight );
            }
            if ( nSpFlags & SP_FFLIPH )     // Horizontal flip?
            {   // BoundRect in aBoundRect
                Point aTop( ( aClientRect.Left() + aClientRect.Right() ) >> 1, aClientRect.Top() );
                Point aBottom( aTop.X(), aTop.Y() + 1000 );
                pRet->NbcMirror( aTop, aBottom );
            }
        }
    }
    if (size_t(nCalledByGroup) < maPendingGroupData.size())
    {
        // finalization for this group is pending, do it now
        pRet = FinalizeObj(maPendingGroupData.back().first, pRet);
        maPendingGroupData.pop_back();
    }
    return pRet;
}

SdrObject* SvxMSDffManager::ImportShape( const DffRecordHeader& rHd, SvStream& rSt, void* pClientData,
                                            Rectangle& rClientRect, const Rectangle& rGlobalChildRect,
                                            int nCalledByGroup, sal_Int32* pShapeId )
{
    SdrObject* pRet = nullptr;

    if( pShapeId )
        *pShapeId = 0;

    if (!rHd.SeekToBegOfRecord(rSt))
        return pRet;

    DffObjData aObjData( rHd, rClientRect, nCalledByGroup );

    aObjData.bRotateTextWithShape = ( GetSvxMSDffSettings() & SVXMSDFF_SETTINGS_IMPORT_EXCEL ) == 0;
    maShapeRecords.Consume( rSt );
    if( maShapeRecords.SeekToContent( rSt,
        DFF_msofbtUDefProp ) )
    {
        sal_uInt32  nBytesLeft = maShapeRecords.Current()->nRecLen;
        while( 5 < nBytesLeft )
        {
            sal_uInt16 nPID(0);
            rSt.ReadUInt16(nPID);
            if (!rSt.good())
                break;
            sal_uInt32 nUDData(0);
            rSt.ReadUInt32(nUDData);
            if (!rSt.good())
                break;
            if (nPID == 447)
            {
                mbRotateGranientFillWithAngle = nUDData & 0x20;
                break;
            }
            nBytesLeft  -= 6;
        }
    }
    aObjData.bShapeType = maShapeRecords.SeekToContent( rSt, DFF_msofbtSp );
    if ( aObjData.bShapeType )
    {
        rSt.ReadUInt32( aObjData.nShapeId )
           .ReadUInt32( aObjData.nSpFlags );
        aObjData.eShapeType = (MSO_SPT)maShapeRecords.Current()->nRecInstance;
    }
    else
    {
        aObjData.nShapeId = 0;
        aObjData.nSpFlags = 0;
        aObjData.eShapeType = mso_sptNil;
    }

    if( pShapeId )
        *pShapeId = aObjData.nShapeId;

    aObjData.bOpt = maShapeRecords.SeekToContent( rSt, DFF_msofbtOPT, SEEK_FROM_CURRENT_AND_RESTART );
    if ( aObjData.bOpt )
    {
        if (!maShapeRecords.Current()->SeekToBegOfRecord(rSt))
            return pRet;
#ifdef DBG_AUTOSHAPE
        ReadPropSet( rSt, pClientData, (sal_uInt32)aObjData.eShapeType );
#else
        ReadPropSet( rSt, pClientData );
#endif
    }
    else
    {
        InitializePropSet( DFF_msofbtOPT ); // get the default PropSet
        static_cast<DffPropertyReader*>(this)->mnFix16Angle = 0;
    }

    aObjData.bOpt2 = maShapeRecords.SeekToContent( rSt, DFF_msofbtUDefProp, SEEK_FROM_CURRENT_AND_RESTART );
    if ( aObjData.bOpt2 )
    {
        maShapeRecords.Current()->SeekToBegOfRecord( rSt );
        delete pSecPropSet;
        pSecPropSet = new DffPropertyReader( *this );
        pSecPropSet->ReadPropSet( rSt, nullptr );
    }

    aObjData.bChildAnchor = maShapeRecords.SeekToContent( rSt, DFF_msofbtChildAnchor, SEEK_FROM_CURRENT_AND_RESTART );
    if ( aObjData.bChildAnchor )
    {
        sal_Int32 l, o, r, u;
        rSt.ReadInt32( l ).ReadInt32( o ).ReadInt32( r ).ReadInt32( u );
        Scale( l );
        Scale( o );
        Scale( r );
        Scale( u );
        aObjData.aChildAnchor = Rectangle( l, o, r, u );
        if ( !rGlobalChildRect.IsEmpty() && !rClientRect.IsEmpty() && rGlobalChildRect.GetWidth() && rGlobalChildRect.GetHeight() )
        {
            double fWidth = r - l;
            double fHeight= u - o;
            double fXScale = (double)rClientRect.GetWidth() / (double)rGlobalChildRect.GetWidth();
            double fYScale = (double)rClientRect.GetHeight() / (double)rGlobalChildRect.GetHeight();
            double fl = ( ( l - rGlobalChildRect.Left() ) * fXScale ) + rClientRect.Left();
            double fo = ( ( o - rGlobalChildRect.Top()  ) * fYScale ) + rClientRect.Top();
            fWidth *= fXScale;
            fHeight *= fYScale;
            aObjData.aChildAnchor = Rectangle( Point( (sal_Int32)fl, (sal_Int32)fo ), Size( (sal_Int32)( fWidth + 1 ), (sal_Int32)( fHeight + 1 ) ) );
        }
    }

    aObjData.bClientAnchor = maShapeRecords.SeekToContent( rSt, DFF_msofbtClientAnchor, SEEK_FROM_CURRENT_AND_RESTART );
    if ( aObjData.bClientAnchor )
        ProcessClientAnchor2( rSt, *maShapeRecords.Current(), pClientData, aObjData );

    if ( aObjData.bChildAnchor )
        aObjData.aBoundRect = aObjData.aChildAnchor;

    if ( aObjData.nSpFlags & SP_FBACKGROUND )
        aObjData.aBoundRect = Rectangle( Point(), Size( 1, 1 ) );

    Rectangle aTextRect;
    if ( !aObjData.aBoundRect.IsEmpty() )
    {   // apply rotation to the BoundingBox BEFORE an object has been generated
        if( mnFix16Angle )
        {
            long nAngle = mnFix16Angle;
            if ( ( nAngle > 4500 && nAngle <= 13500 ) || ( nAngle > 22500 && nAngle <= 31500 ) )
            {
                sal_Int32 nHalfWidth = ( aObjData.aBoundRect.GetWidth() + 1 ) >> 1;
                sal_Int32 nHalfHeight = ( aObjData.aBoundRect.GetHeight() + 1 ) >> 1;
                Point aTopLeft( aObjData.aBoundRect.Left() + nHalfWidth - nHalfHeight,
                                aObjData.aBoundRect.Top() + nHalfHeight - nHalfWidth );
                Size aNewSize( aObjData.aBoundRect.GetHeight(), aObjData.aBoundRect.GetWidth() );
                Rectangle aNewRect( aTopLeft, aNewSize );
                aObjData.aBoundRect = aNewRect;
            }
        }
        aTextRect = aObjData.aBoundRect;
        bool bGraphic = IsProperty( DFF_Prop_pib ) ||
                            IsProperty( DFF_Prop_pibName ) ||
                            IsProperty( DFF_Prop_pibFlags );

        if ( aObjData.nSpFlags & SP_FGROUP )
        {
            pRet = new SdrObjGroup;
            /*  After CWS aw033 has been integrated, an empty group object
                cannot store its resulting bounding rectangle anymore. We have
                to return this rectangle via rClientRect now, but only, if
                caller has not passed an own bounding ractangle. */
            if ( rClientRect.IsEmpty() )
                 rClientRect = aObjData.aBoundRect;
            nGroupShapeFlags = aObjData.nSpFlags;
        }
        else if ( ( aObjData.eShapeType != mso_sptNil ) || IsProperty( DFF_Prop_pVertices ) || bGraphic )
        {
            SfxItemSet  aSet( pSdrModel->GetItemPool() );

            bool    bIsConnector = ( ( aObjData.eShapeType >= mso_sptStraightConnector1 ) && ( aObjData.eShapeType <= mso_sptCurvedConnector5 ) );
            sal_Int32   nObjectRotation = mnFix16Angle;
            sal_uInt32  nSpFlags = aObjData.nSpFlags;

            if ( bGraphic )
            {
                if (!mbSkipImages) {
                    pRet = ImportGraphic( rSt, aSet, aObjData );        // SJ: #68396# is no longer true (fixed in ppt2000)
                    ApplyAttributes( rSt, aSet, aObjData );
                    pRet->SetMergedItemSet(aSet);
                }
            }
            else if ( aObjData.eShapeType == mso_sptLine && !( GetPropertyValue( DFF_Prop_fc3DLightFace, 0 ) & 8 ) )
            {
                basegfx::B2DPolygon aPoly;
                aPoly.append(basegfx::B2DPoint(aObjData.aBoundRect.Left(), aObjData.aBoundRect.Top()));
                aPoly.append(basegfx::B2DPoint(aObjData.aBoundRect.Right(), aObjData.aBoundRect.Bottom()));
                pRet = new SdrPathObj(OBJ_LINE, basegfx::B2DPolyPolygon(aPoly));
                pRet->SetModel( pSdrModel );
                ApplyAttributes( rSt, aSet, aObjData );
                pRet->SetMergedItemSet(aSet);
            }
            else
            {
                if ( GetCustomShapeContent( aObjData.eShapeType ) || IsProperty( DFF_Prop_pVertices ) )
                {

                    ApplyAttributes( rSt, aSet, aObjData );

                    pRet = new SdrObjCustomShape();
                    pRet->SetModel( pSdrModel );

                    bool bIsFontwork = ( GetPropertyValue( DFF_Prop_gtextFStrikethrough, 0 ) & 0x4000 ) != 0;

                    // in case of a FontWork, the text is set by the escher import
                    if ( bIsFontwork )
                    {
                        OUString            aObjectText;
                        OUString            aFontName;
                        MSO_GeoTextAlign    eGeoTextAlign;

                        if ( SeekToContent( DFF_Prop_gtextFont, rSt ) )
                        {
                            SvxFontItem aLatin(EE_CHAR_FONTINFO), aAsian(EE_CHAR_FONTINFO_CJK), aComplex(EE_CHAR_FONTINFO_CTL);
                            GetDefaultFonts( aLatin, aAsian, aComplex );

                            aFontName = MSDFFReadZString( rSt, GetPropertyValue( DFF_Prop_gtextFont, 0 ), true );
                            aSet.Put( SvxFontItem( aLatin.GetFamily(), aFontName, aLatin.GetStyleName(),
                                        PITCH_DONTKNOW, RTL_TEXTENCODING_DONTKNOW, EE_CHAR_FONTINFO ));
                            aSet.Put( SvxFontItem( aLatin.GetFamily(), aFontName, aLatin.GetStyleName(),
                                        PITCH_DONTKNOW, RTL_TEXTENCODING_DONTKNOW, EE_CHAR_FONTINFO_CJK ) );
                            aSet.Put( SvxFontItem( aLatin.GetFamily(), aFontName, aLatin.GetStyleName(),
                                        PITCH_DONTKNOW, RTL_TEXTENCODING_DONTKNOW, EE_CHAR_FONTINFO_CTL ) );
                        }

                        // SJ: applying fontattributes for Fontwork :
                        if ( IsHardAttribute( DFF_Prop_gtextFItalic ) )
                            aSet.Put( SvxPostureItem( ( GetPropertyValue( DFF_Prop_gtextFStrikethrough, 0 ) & 0x0010 ) != 0 ? ITALIC_NORMAL : ITALIC_NONE, EE_CHAR_ITALIC ) );

                        if ( IsHardAttribute( DFF_Prop_gtextFBold ) )
                            aSet.Put( SvxWeightItem( ( GetPropertyValue( DFF_Prop_gtextFStrikethrough, 0 ) & 0x0020 ) != 0 ? WEIGHT_BOLD : WEIGHT_NORMAL, EE_CHAR_WEIGHT ) );

                        // SJ TODO: Vertical Writing is not correct, instead
                        // this should be replaced through "CharacterRotation"
                        // by 90 degrees, therefore a new Item has to be
                        // supported by svx core, api and xml file format
                        static_cast<SdrObjCustomShape*>(pRet)->SetVerticalWriting( ( GetPropertyValue( DFF_Prop_gtextFStrikethrough, 0 ) & 0x2000 ) != 0 );

                        if ( SeekToContent( DFF_Prop_gtextUNICODE, rSt ) )
                        {
                            aObjectText = MSDFFReadZString( rSt, GetPropertyValue( DFF_Prop_gtextUNICODE, 0 ), true );
                            ReadObjText( aObjectText, pRet );
                        }

                        eGeoTextAlign = ( (MSO_GeoTextAlign)GetPropertyValue( DFF_Prop_gtextAlign, mso_alignTextCenter ) );
                        {
                            SdrTextHorzAdjust eHorzAdjust;
                            switch( eGeoTextAlign )
                            {
                                case mso_alignTextLetterJust :
                                case mso_alignTextWordJust :
                                case mso_alignTextStretch : eHorzAdjust = SDRTEXTHORZADJUST_BLOCK; break;
                                default:
                                case mso_alignTextInvalid :
                                case mso_alignTextCenter : eHorzAdjust = SDRTEXTHORZADJUST_CENTER; break;
                                case mso_alignTextLeft : eHorzAdjust = SDRTEXTHORZADJUST_LEFT; break;
                                case mso_alignTextRight : eHorzAdjust = SDRTEXTHORZADJUST_RIGHT; break;
                            }
                            aSet.Put( SdrTextHorzAdjustItem( eHorzAdjust ) );

                            SdrFitToSizeType eFTS = SdrFitToSizeType::NONE;
                            if ( eGeoTextAlign == mso_alignTextStretch )
                                eFTS = SdrFitToSizeType::AllLines;
                            aSet.Put( SdrTextFitToSizeTypeItem( eFTS ) );
                        }
                        if ( IsProperty( DFF_Prop_gtextSpacing ) )
                        {
                            sal_Int32 nTextWidth = GetPropertyValue( DFF_Prop_gtextSpacing, 1 << 16 ) / 655;
                            if ( nTextWidth != 100 )
                                aSet.Put( SvxCharScaleWidthItem( (sal_uInt16)nTextWidth, EE_CHAR_FONTWIDTH ) );
                        }
                        if ( GetPropertyValue( DFF_Prop_gtextFStrikethrough, 0 ) & 0x1000 ) // SJ: Font Kerning On ?
                            aSet.Put( SvxKerningItem( 1, EE_CHAR_KERNING ) );

                        // #i119496# the resize autoshape to fit text attr of word art in MS PPT is always false
                        aSet.Put(makeSdrTextAutoGrowHeightItem(false));
                        aSet.Put(makeSdrTextAutoGrowWidthItem(false));
                    }
                    pRet->SetMergedItemSet( aSet );

                    // sj: taking care of rtl, ltr. In case of fontwork mso. seems not to be able to set
                    // proper text directions, instead the text default is depending to the string.
                    // so we have to calculate the a text direction from string:
                    if ( bIsFontwork )
                    {
                        OutlinerParaObject* pParaObj = static_cast<SdrObjCustomShape*>(pRet)->GetOutlinerParaObject();
                        if ( pParaObj )
                        {
                            SdrOutliner& rOutliner = static_cast<SdrObjCustomShape*>(pRet)->ImpGetDrawOutliner();
                            bool bOldUpdateMode = rOutliner.GetUpdateMode();
                            SdrModel* pModel = pRet->GetModel();
                            if ( pModel )
                                rOutliner.SetStyleSheetPool( static_cast<SfxStyleSheetPool*>(pModel->GetStyleSheetPool()) );
                            rOutliner.SetUpdateMode( false );
                            rOutliner.SetText( *pParaObj );
                            ScopedVclPtrInstance< VirtualDevice > pVirDev(DeviceFormat::BITMASK);
                            pVirDev->SetMapMode( MapUnit::Map100thMM );
                            sal_Int32 i, nParagraphs = rOutliner.GetParagraphCount();
                            if ( nParagraphs )
                            {
                                bool bCreateNewParaObject = false;
                                for ( i = 0; i < nParagraphs; i++ )
                                {
                                    OUString aString(rOutliner.GetText(rOutliner.GetParagraph(i)));
                                    bool bIsRTL = pVirDev->GetTextIsRTL(aString, 0, aString.getLength());
                                    if ( bIsRTL )
                                    {
                                        SfxItemSet aSet2( rOutliner.GetParaAttribs( i ) );
                                        aSet2.Put( SvxFrameDirectionItem( FRMDIR_HORI_RIGHT_TOP, EE_PARA_WRITINGDIR ) );
                                        rOutliner.SetParaAttribs( i, aSet2 );
                                        bCreateNewParaObject = true;
                                    }
                                }
                                if  ( bCreateNewParaObject )
                                {
                                    OutlinerParaObject* pNewText = rOutliner.CreateParaObject();
                                    rOutliner.Init( OutlinerMode::TextObject );
                                    static_cast<SdrObjCustomShape*>(pRet)->NbcSetOutlinerParaObject( pNewText );
                                }
                            }
                            rOutliner.Clear();
                            rOutliner.SetUpdateMode( bOldUpdateMode );
                        }
                    }

                    // mso_sptArc special treating:
                    // sj: since we actually can't render the arc because of its weird SnapRect settings,
                    // we will create a new CustomShape, that can be saved/loaded without problems.
                    // We will change the shape type, so this code applies only if importing arcs from msoffice.
                    if ( aObjData.eShapeType == mso_sptArc )
                    {
                        const OUString sAdjustmentValues( "AdjustmentValues" );
                        const OUString sCoordinates( "Coordinates" );
                        const OUString sHandles( "Handles" );
                        const OUString sEquations( "Equations" );
                        const OUString sViewBox( "ViewBox" );
                        const OUString sPath( "Path" );
                        const OUString sTextFrames( "TextFrames" );
                        SdrCustomShapeGeometryItem aGeometryItem( static_cast<const SdrCustomShapeGeometryItem&>(static_cast<SdrObjCustomShape*>(pRet)->GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY ) ) );
                        css::uno::Sequence< css::drawing::EnhancedCustomShapeParameterPair> seqCoordinates;
                        css::uno::Sequence< css::drawing::EnhancedCustomShapeAdjustmentValue > seqAdjustmentValues;

                        // before clearing the GeometryItem we have to store the current Coordinates
                        const uno::Any* pAny = ((SdrCustomShapeGeometryItem&)aGeometryItem).GetPropertyValueByName( sPath, sCoordinates );
                        Rectangle aPolyBoundRect;
                        Point aStartPt( 0,0 );
                        if ( pAny && ( *pAny >>= seqCoordinates ) && ( seqCoordinates.getLength() >= 4 ) )
                        {
                            sal_Int32 nPtNum, nNumElemVert = seqCoordinates.getLength();
                            XPolygon aXP( (sal_uInt16)nNumElemVert );
                            for ( nPtNum = 0; nPtNum < nNumElemVert; nPtNum++ )
                            {
                                Point aP;
                                sal_Int32 nX = 0, nY = 0;
                                seqCoordinates[ nPtNum ].First.Value >>= nX;
                                seqCoordinates[ nPtNum ].Second.Value >>= nY;
                                aP.X() = nX;
                                aP.Y() = nY;
                                aXP[ (sal_uInt16)nPtNum ] = aP;
                            }
                            aPolyBoundRect = Rectangle( aXP.GetBoundRect() );
                            if ( nNumElemVert >= 3 )
                            { // arc first command is always wr -- clockwise arc
                                // the parameters are : (left,top),(right,bottom),start(x,y),end(x,y)
                                aStartPt = aXP[2];
                            }
                        }
                        else
                            aPolyBoundRect = Rectangle( -21600, 0, 21600, 43200 );  // defaulting

                        // clearing items, so MergeDefaultAttributes will set the corresponding defaults from EnhancedCustomShapeGeometry
                        aGeometryItem.ClearPropertyValue( sHandles );
                        aGeometryItem.ClearPropertyValue( sEquations );
                        aGeometryItem.ClearPropertyValue( sViewBox );
                        aGeometryItem.ClearPropertyValue( sPath );

                        sal_Int32 nEndAngle = 9000;
                        sal_Int32 nStartAngle = 0;
                        pAny = ((SdrCustomShapeGeometryItem&)aGeometryItem).GetPropertyValueByName( sAdjustmentValues );
                        if ( pAny && ( *pAny >>= seqAdjustmentValues ) && seqAdjustmentValues.getLength() > 1 )
                        {
                            if ( seqAdjustmentValues[ 0 ].State == css::beans::PropertyState_DIRECT_VALUE )
                            {
                                double fNumber;
                                seqAdjustmentValues[ 0 ].Value >>= fNumber;
                                nEndAngle = NormAngle360( - (sal_Int32)fNumber * 100 );
                            }
                            else
                            {
                                //normal situation:if endAngle != 90,there will be a direct_value,but for damaged curve,the endAngle need to recalculate.
                                Point cent = aPolyBoundRect.Center();
                                double fNumber;
                                if ( aStartPt.Y() == cent.Y() )
                                    fNumber = ( aStartPt.X() >= cent.X() ) ? 0:180.0;
                                else if ( aStartPt.X() == cent.X() )
                                    fNumber = ( aStartPt.Y() >= cent.Y() ) ? 90.0: 270.0;
                                else
                                {
                                    fNumber = atan2( double( aStartPt.X() - cent.X() ),double( aStartPt.Y() - cent.Y() ) )+ F_PI; // 0..2PI
                                    fNumber /= F_PI180; // 0..360.0
                                }
                                nEndAngle = NormAngle360( - (sal_Int32)fNumber * 100 );
                                seqAdjustmentValues[ 0 ].Value <<= fNumber;
                                seqAdjustmentValues[ 0 ].State = css::beans::PropertyState_DIRECT_VALUE;     // so this value will properly be stored
                            }

                            if ( seqAdjustmentValues[ 1 ].State == css::beans::PropertyState_DIRECT_VALUE )
                            {
                                double fNumber;
                                seqAdjustmentValues[ 1 ].Value >>= fNumber;
                                nStartAngle = NormAngle360( - (sal_Int32)fNumber * 100 );
                            }
                            else
                            {
                                seqAdjustmentValues[ 1 ].Value <<= 0.0;
                                seqAdjustmentValues[ 1 ].State = css::beans::PropertyState_DIRECT_VALUE;
                            }

                            PropertyValue aPropVal;
                            aPropVal.Name = sAdjustmentValues;
                            aPropVal.Value <<= seqAdjustmentValues;
                            aGeometryItem.SetPropertyValue( aPropVal );     // storing the angle attribute
                        }
                        if ( nStartAngle != nEndAngle )
                        {
                            XPolygon aXPoly( aPolyBoundRect.Center(), aPolyBoundRect.GetWidth() / 2, aPolyBoundRect.GetHeight() / 2,
                                (sal_uInt16)nStartAngle / 10, (sal_uInt16)nEndAngle / 10, true );
                            Rectangle aPolyPieRect( aXPoly.GetBoundRect() );

                            double  fYScale = 0.0, fXScale = 0.0;
                            double  fYOfs, fXOfs;

                            Point aP( aObjData.aBoundRect.Center() );
                            Size aS( aObjData.aBoundRect.GetSize() );
                            aP.X() -= aS.Width() / 2;
                            aP.Y() -= aS.Height() / 2;
                            Rectangle aLogicRect( aP, aS );

                            fYOfs = fXOfs = 0.0;

                            if ( aPolyBoundRect.GetWidth() && aPolyPieRect.GetWidth() )
                            {
                                fXScale = (double)aLogicRect.GetWidth() / (double)aPolyPieRect.GetWidth();
                                if ( nSpFlags & SP_FFLIPH )
                                    fXOfs = ( (double)aPolyPieRect.Right() - (double)aPolyBoundRect.Right() ) * fXScale;
                                else
                                    fXOfs = ( (double)aPolyBoundRect.Left() - (double)aPolyPieRect.Left() ) * fXScale;
                            }
                            if ( aPolyBoundRect.GetHeight() && aPolyPieRect.GetHeight() )
                            {
                                fYScale = (double)aLogicRect.GetHeight() / (double)aPolyPieRect.GetHeight();
                                if ( nSpFlags & SP_FFLIPV )
                                    fYOfs = ( (double)aPolyPieRect.Bottom() - (double)aPolyBoundRect.Bottom() ) * fYScale;
                                else
                                    fYOfs = ((double)aPolyBoundRect.Top() - (double)aPolyPieRect.Top() ) * fYScale;
                            }

                            if ( aPolyPieRect.GetWidth() )
                                fXScale = (double)aPolyBoundRect.GetWidth() / (double)aPolyPieRect.GetWidth();
                            if ( aPolyPieRect.GetHeight() )
                                fYScale = (double)aPolyBoundRect.GetHeight() / (double)aPolyPieRect.GetHeight();

                            Rectangle aOldBoundRect( aObjData.aBoundRect );
                            aObjData.aBoundRect = Rectangle( Point( aLogicRect.Left() + (sal_Int32)fXOfs, aLogicRect.Top() + (sal_Int32)fYOfs ),
                                 Size( (sal_Int32)( aLogicRect.GetWidth() * fXScale ), (sal_Int32)( aLogicRect.GetHeight() * fYScale ) ) );

                            // creating the text frame -> scaling into (0,0),(21600,21600) destination coordinate system
                            double fTextFrameScaleX = 0.0;
                            double fTextFrameScaleY = 0.0;
                            if (aPolyBoundRect.GetWidth())
                                fTextFrameScaleX = (double)21600 / (double)aPolyBoundRect.GetWidth();
                            if (aPolyBoundRect.GetHeight())
                                fTextFrameScaleY = (double)21600 / (double)aPolyBoundRect.GetHeight();

                            sal_Int32 nLeft  = (sal_Int32)(( aPolyPieRect.Left()  - aPolyBoundRect.Left() ) * fTextFrameScaleX );
                            sal_Int32 nTop   = (sal_Int32)(( aPolyPieRect.Top()   - aPolyBoundRect.Top() )  * fTextFrameScaleY );
                            sal_Int32 nRight = (sal_Int32)(( aPolyPieRect.Right() - aPolyBoundRect.Left() ) * fTextFrameScaleX );
                            sal_Int32 nBottom= (sal_Int32)(( aPolyPieRect.Bottom()- aPolyBoundRect.Top() )  * fTextFrameScaleY );
                            css::uno::Sequence< css::drawing::EnhancedCustomShapeTextFrame > aTextFrame( 1 );
                            EnhancedCustomShape2d::SetEnhancedCustomShapeParameter( aTextFrame[ 0 ].TopLeft.First,     nLeft );
                            EnhancedCustomShape2d::SetEnhancedCustomShapeParameter( aTextFrame[ 0 ].TopLeft.Second,    nTop );
                            EnhancedCustomShape2d::SetEnhancedCustomShapeParameter( aTextFrame[ 0 ].BottomRight.First, nRight );
                            EnhancedCustomShape2d::SetEnhancedCustomShapeParameter( aTextFrame[ 0 ].BottomRight.Second,nBottom );
                            PropertyValue aProp;
                            aProp.Name = sTextFrames;
                            aProp.Value <<= aTextFrame;
                            aGeometryItem.SetPropertyValue( sPath, aProp );

                            // sj: taking care of the different rotation points, since the new arc is having a bigger snaprect
                            if ( mnFix16Angle )
                            {
                                sal_Int32 nAngle = mnFix16Angle;
                                if ( nSpFlags & SP_FFLIPH )
                                    nAngle = 36000 - nAngle;
                                if ( nSpFlags & SP_FFLIPV )
                                    nAngle = -nAngle;
                                double a = nAngle * F_PI18000;
                                double ss = sin( a );
                                double cc = cos( a );
                                Point aP1( aOldBoundRect.TopLeft() );
                                Point aC1( aObjData.aBoundRect.Center() );
                                Point aP2( aOldBoundRect.TopLeft() );
                                Point aC2( aOldBoundRect.Center() );
                                RotatePoint( aP1, aC1, ss, cc );
                                RotatePoint( aP2, aC2, ss, cc );
                                aObjData.aBoundRect.Move( aP2.X() - aP1.X(), aP2.Y() - aP1.Y() );
                            }
                        }
                        static_cast<SdrObjCustomShape*>(pRet)->SetMergedItem( aGeometryItem );
                        static_cast<SdrObjCustomShape*>(pRet)->MergeDefaultAttributes();

                        // now setting a new name, so the above correction is only done once when importing from ms
                        SdrCustomShapeGeometryItem aGeoName( static_cast<const SdrCustomShapeGeometryItem&>(static_cast<SdrObjCustomShape*>(pRet)->GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY ) ) );
                        const OUString sType( "Type" );
                        const OUString sName( "mso-spt100" );
                        PropertyValue aPropVal;
                        aPropVal.Name = sType;
                        aPropVal.Value <<= sName;
                        aGeoName.SetPropertyValue( aPropVal );
                        static_cast<SdrObjCustomShape*>(pRet)->SetMergedItem( aGeoName );
                    }
                    else
                        static_cast<SdrObjCustomShape*>(pRet)->MergeDefaultAttributes();

                    pRet->SetSnapRect( aObjData.aBoundRect );
                    EnhancedCustomShape2d aCustomShape2d( pRet );
                    aTextRect = aCustomShape2d.GetTextRect();

                    if( bIsConnector )
                    {
                        if( nObjectRotation )
                        {
                            double a = nObjectRotation * nPi180;
                            pRet->NbcRotate( aObjData.aBoundRect.Center(), nObjectRotation, sin( a ), cos( a ) );
                        }
                        // mirrored horizontally?
                        if ( nSpFlags & SP_FFLIPH )
                        {
                            Rectangle aBndRect( pRet->GetSnapRect() );
                            Point aTop( ( aBndRect.Left() + aBndRect.Right() ) >> 1, aBndRect.Top() );
                            Point aBottom( aTop.X(), aTop.Y() + 1000 );
                            pRet->NbcMirror( aTop, aBottom );
                        }
                        // mirrored vertically?
                        if ( nSpFlags & SP_FFLIPV )
                        {
                            Rectangle aBndRect( pRet->GetSnapRect() );
                            Point aLeft( aBndRect.Left(), ( aBndRect.Top() + aBndRect.Bottom() ) >> 1 );
                            Point aRight( aLeft.X() + 1000, aLeft.Y() );
                            pRet->NbcMirror( aLeft, aRight );
                        }
                        basegfx::B2DPolyPolygon aPoly( static_cast<SdrObjCustomShape*>(pRet)->GetLineGeometry( true ) );
                        SdrObject::Free( pRet );

                        pRet = new SdrEdgeObj();
                        ApplyAttributes( rSt, aSet, aObjData );
                        pRet->SetLogicRect( aObjData.aBoundRect );
                        pRet->SetMergedItemSet(aSet);

                        // connectors
                        MSO_ConnectorStyle eConnectorStyle = (MSO_ConnectorStyle)GetPropertyValue( DFF_Prop_cxstyle, mso_cxstyleStraight );

                        static_cast<SdrEdgeObj*>(pRet)->ConnectToNode(true, nullptr);
                        static_cast<SdrEdgeObj*>(pRet)->ConnectToNode(false, nullptr);

                        Point aPoint1( aObjData.aBoundRect.TopLeft() );
                        Point aPoint2( aObjData.aBoundRect.BottomRight() );

                        // pay attention to the rotations
                        if ( nObjectRotation )
                        {
                            double a = nObjectRotation * nPi180;
                            Point aCenter( aObjData.aBoundRect.Center() );
                            double ss = sin(a);
                            double cc = cos(a);

                            RotatePoint(aPoint1, aCenter, ss, cc);
                            RotatePoint(aPoint2, aCenter, ss, cc);

                            // #i120437# reset rotation, it is part of the path and shall not be applied again
                            nObjectRotation = 0;
                        }

                        // rotate/mirror line within the area as we need it
                        if ( nSpFlags & SP_FFLIPH )
                        {
                            sal_Int32 n = aPoint1.X();
                            aPoint1.X() = aPoint2.X();
                            aPoint2.X() = n;

                            // #i120437# reset hor filp
                            nSpFlags &= ~SP_FFLIPH;
                        }
                        if ( nSpFlags & SP_FFLIPV )
                        {
                            sal_Int32 n = aPoint1.Y();
                            aPoint1.Y() = aPoint2.Y();
                            aPoint2.Y() = n;

                            // #i120437# reset ver filp
                            nSpFlags &= ~SP_FFLIPV;
                        }

                        pRet->NbcSetPoint(aPoint1, 0L); // start point
                        pRet->NbcSetPoint(aPoint2, 1L); // endpoint

                        sal_Int32 n1HorzDist, n1VertDist, n2HorzDist, n2VertDist;
                        n1HorzDist = n1VertDist = n2HorzDist = n2VertDist = 0;
                        switch( eConnectorStyle )
                        {
                            case mso_cxstyleBent:
                            {
                                aSet.Put( SdrEdgeKindItem( SdrEdgeKind::OrthoLines ) );
                                n1HorzDist = n1VertDist = n2HorzDist = n2VertDist = 630;
                            }
                            break;
                            case mso_cxstyleCurved:
                                aSet.Put( SdrEdgeKindItem( SdrEdgeKind::Bezier ) );
                            break;
                            default: // mso_cxstyleStraight || mso_cxstyleNone
                                aSet.Put( SdrEdgeKindItem( SdrEdgeKind::OneLine ) );
                            break;
                        }
                        aSet.Put( SdrEdgeNode1HorzDistItem( n1HorzDist ) );
                        aSet.Put( SdrEdgeNode1VertDistItem( n1VertDist ) );
                        aSet.Put( SdrEdgeNode2HorzDistItem( n2HorzDist ) );
                        aSet.Put( SdrEdgeNode2VertDistItem( n2VertDist ) );

                        static_cast<SdrEdgeObj*>(pRet)->SetEdgeTrackPath( aPoly );
                        pRet->SetMergedItemSet( aSet );
                    }
                    if ( aObjData.eShapeType == mso_sptLine )
                    {
                        pRet->SetMergedItemSet(aSet);
                        static_cast<SdrObjCustomShape*>(pRet)->MergeDefaultAttributes();
                    }
                }
            }

            if ( pRet )
            {
                if( nObjectRotation )
                {
                    double a = nObjectRotation * nPi180;
                    pRet->NbcRotate( aObjData.aBoundRect.Center(), nObjectRotation, sin( a ), cos( a ) );
                }
                // mirrored horizontally?
                if ( nSpFlags & SP_FFLIPH )
                {
                    Rectangle aBndRect( pRet->GetSnapRect() );
                    Point aTop( ( aBndRect.Left() + aBndRect.Right() ) >> 1, aBndRect.Top() );
                    Point aBottom( aTop.X(), aTop.Y() + 1000 );
                    pRet->NbcMirror( aTop, aBottom );
                }
                // mirrored vertically?
                if ( nSpFlags & SP_FFLIPV )
                {
                    Rectangle aBndRect( pRet->GetSnapRect() );
                    Point aLeft( aBndRect.Left(), ( aBndRect.Top() + aBndRect.Bottom() ) >> 1 );
                    Point aRight( aLeft.X() + 1000, aLeft.Y() );
                    pRet->NbcMirror( aLeft, aRight );
                }
            }
        }
    }

    // #i51348# #118052# name of the shape
    if( pRet )
    {
        OUString aObjName = GetPropertyString( DFF_Prop_wzName, rSt );
        if( !aObjName.isEmpty() )
            pRet->SetName( aObjName );
    }

    pRet =
        ProcessObj( rSt, aObjData, pClientData, aTextRect, pRet);

    if ( pRet )
    {
        sal_Int32 nGroupProperties( GetPropertyValue( DFF_Prop_fPrint, 0 ) );
        pRet->SetVisible( ( nGroupProperties & 2 ) == 0 );
        pRet->SetPrintable( ( nGroupProperties & 1 ) != 0 );
    }

    //Import alt text as description
    if ( pRet && SeekToContent( DFF_Prop_wzDescription, rSt ) )
    {
        OUString aAltText = MSDFFReadZString(rSt, GetPropertyValue(DFF_Prop_wzDescription, 0), true);
        pRet->SetDescription( aAltText );
    }

    // If this shape opens a new group, push back its object data because
    // finalization will be called when nested objects have been imported;
    // otherwise, just finalize here
    if (size_t(nCalledByGroup) > maPendingGroupData.size())
    {
        std::shared_ptr<DffRecordHeader> rHdClone(new DffRecordHeader(aObjData.rSpHd));
        maPendingGroupData.push_back( make_pair(DffObjData(rHdClone, aObjData), rHdClone) );
    }
    else
    {
        pRet = FinalizeObj(aObjData, pRet);
    }
    return pRet;
}

Rectangle SvxMSDffManager::GetGlobalChildAnchor( const DffRecordHeader& rHd, SvStream& rSt, Rectangle& aClientRect )
{
    Rectangle aChildAnchor;
    if (!rHd.SeekToContent(rSt))
        return aChildAnchor;

    bool bIsClientRectRead = false;
    while ( ( rSt.GetError() == 0 ) && ( rSt.Tell() < rHd.GetRecEndFilePos() ) )
    {
        DffRecordHeader aShapeHd;
        if (!ReadDffRecordHeader(rSt, aShapeHd))
            break;
        if ( ( aShapeHd.nRecType == DFF_msofbtSpContainer ) ||
                ( aShapeHd.nRecType == DFF_msofbtSpgrContainer ) )
        {
            DffRecordHeader aShapeHd2( aShapeHd );
            if ( aShapeHd.nRecType == DFF_msofbtSpgrContainer )
                ReadDffRecordHeader( rSt, aShapeHd2 );
            while (rSt.good() && rSt.Tell() < aShapeHd2.GetRecEndFilePos())
            {
                DffRecordHeader aShapeAtom;
                if (!ReadDffRecordHeader(rSt, aShapeAtom))
                    break;

                if ( aShapeAtom.nRecType == DFF_msofbtClientAnchor )
                {
                    if ( GetSvxMSDffSettings() & SVXMSDFF_SETTINGS_IMPORT_PPT )
                    {
                        sal_Int32 l, t, r, b;
                        if ( aShapeAtom.nRecLen == 16 )
                        {
                            rSt.ReadInt32( l ).ReadInt32( t ).ReadInt32( r ).ReadInt32( b );
                        }
                        else
                        {
                            sal_Int16 ls, ts, rs, bs;
                            rSt.ReadInt16( ts ).ReadInt16( ls ).ReadInt16( rs ).ReadInt16( bs ); // the order of coordinates is a bit strange...
                            l = ls;
                            t = ts;
                            r = rs;
                            b = bs;
                        }
                        Scale( l );
                        Scale( t );
                        Scale( r );
                        Scale( b );
                        if ( bIsClientRectRead )
                        {
                            Rectangle aChild( l, t, r, b );
                            aChildAnchor.Union( aChild );
                        }
                        else
                        {
                            aClientRect = Rectangle( l, t, r, b );
                            bIsClientRectRead = true;
                        }
                    }
                    break;
                }
                else if ( aShapeAtom.nRecType == DFF_msofbtChildAnchor )
                {
                    sal_Int32 l, o, r, u;
                    rSt.ReadInt32( l ).ReadInt32( o ).ReadInt32( r ).ReadInt32( u );
                    Scale( l );
                    Scale( o );
                    Scale( r );
                    Scale( u );
                    Rectangle aChild( l, o, r, u );
                    aChildAnchor.Union( aChild );
                    break;
                }
                if (!aShapeAtom.SeekToEndOfRecord(rSt))
                    break;
            }
        }
        if (!aShapeHd.SeekToEndOfRecord(rSt))
            break;
    }
    return aChildAnchor;
}

void SvxMSDffManager::GetGroupAnchors( const DffRecordHeader& rHd, SvStream& rSt,
                            Rectangle& rGroupClientAnchor, Rectangle& rGroupChildAnchor,
                                const Rectangle& rClientRect, const Rectangle& rGlobalChildRect )
{
    if (!rHd.SeekToContent(rSt))
        return;

    bool bFirst = true;
    DffRecordHeader aShapeHd;
    while (rSt.good() && rSt.Tell() < rHd.GetRecEndFilePos())
    {
        if (!ReadDffRecordHeader(rSt, aShapeHd))
            break;
        if ( ( aShapeHd.nRecType == DFF_msofbtSpContainer ) ||
                ( aShapeHd.nRecType == DFF_msofbtSpgrContainer ) )
        {
            DffRecordHeader aShapeHd2( aShapeHd );
            if ( aShapeHd.nRecType == DFF_msofbtSpgrContainer )
                ReadDffRecordHeader( rSt, aShapeHd2 );
            while (rSt.good() && rSt.Tell() < aShapeHd2.GetRecEndFilePos())
            {
                DffRecordHeader aShapeAtom;
                if (!ReadDffRecordHeader(rSt, aShapeAtom))
                    break;
                if ( aShapeAtom.nRecType == DFF_msofbtChildAnchor )
                {
                    sal_Int32 l, o, r, u;
                    rSt.ReadInt32( l ).ReadInt32( o ).ReadInt32( r ).ReadInt32( u );
                    Scale( l );
                    Scale( o );
                    Scale( r );
                    Scale( u );
                    Rectangle aChild( l, o, r, u );

                    if ( bFirst )
                    {
                        if ( !rGlobalChildRect.IsEmpty() && !rClientRect.IsEmpty() && rGlobalChildRect.GetWidth() && rGlobalChildRect.GetHeight() )
                        {
                            double fWidth = r - l;
                            double fHeight= u - o;
                            double fXScale = (double)rClientRect.GetWidth() / (double)rGlobalChildRect.GetWidth();
                            double fYScale = (double)rClientRect.GetHeight() / (double)rGlobalChildRect.GetHeight();
                            double fl = ( ( l - rGlobalChildRect.Left() ) * fXScale ) + rClientRect.Left();
                            double fo = ( ( o - rGlobalChildRect.Top()  ) * fYScale ) + rClientRect.Top();
                            fWidth *= fXScale;
                            fHeight *= fYScale;
                            rGroupClientAnchor = Rectangle( Point( (sal_Int32)fl, (sal_Int32)fo ), Size( (sal_Int32)( fWidth + 1 ), (sal_Int32)( fHeight + 1 ) ) );
                        }
                        bFirst = false;
                    }
                    else
                        rGroupChildAnchor.Union( aChild );
                    break;
                }
                if (!aShapeAtom.SeekToEndOfRecord(rSt))
                    break;
            }
        }
        if (!aShapeHd.SeekToEndOfRecord(rSt))
            break;
    }
}

SdrObject* SvxMSDffManager::ProcessObj(SvStream& rSt,
                                       DffObjData& rObjData,
                                       void* pData,
                                       Rectangle& rTextRect,
                                       SdrObject* pObj
                                       )
{
    if( !rTextRect.IsEmpty() )
    {
        SvxMSDffImportData& rImportData = *static_cast<SvxMSDffImportData*>(pData);
        SvxMSDffImportRec* pImpRec = new SvxMSDffImportRec;
        bool bDeleteImpRec = true;
        SvxMSDffImportRec* pTextImpRec = pImpRec;
        bool bDeleteTextImpRec = false;

        // fill Import Record with data
        pImpRec->nShapeId   = rObjData.nShapeId;
        pImpRec->eShapeType = rObjData.eShapeType;

        MSO_WrapMode eWrapMode( (MSO_WrapMode)GetPropertyValue(
                                                            DFF_Prop_WrapText,
                                                            mso_wrapSquare ) );
        rObjData.bClientAnchor = maShapeRecords.SeekToContent( rSt,
                                            DFF_msofbtClientAnchor,
                                            SEEK_FROM_CURRENT_AND_RESTART );
        if( rObjData.bClientAnchor )
            ProcessClientAnchor( rSt,
                    maShapeRecords.Current()->nRecLen,
                    pImpRec->pClientAnchorBuffer, pImpRec->nClientAnchorLen );

        rObjData.bClientData = maShapeRecords.SeekToContent( rSt,
                                            DFF_msofbtClientData,
                                            SEEK_FROM_CURRENT_AND_RESTART );
        if( rObjData.bClientData )
            ProcessClientData( rSt,
                    maShapeRecords.Current()->nRecLen,
                    pImpRec->pClientDataBuffer, pImpRec->nClientDataLen );


        // process user (== Winword) defined parameters in 0xF122 record
        if(    maShapeRecords.SeekToContent( rSt,
                                             DFF_msofbtUDefProp,
                                             SEEK_FROM_CURRENT_AND_RESTART )
            && maShapeRecords.Current()->nRecLen )
        {
            sal_uInt32  nBytesLeft = maShapeRecords.Current()->nRecLen;
            while( 5 < nBytesLeft )
            {
                sal_uInt16 nPID(0);
                rSt.ReadUInt16(nPID);
                if (!rSt.good())
                    break;
                sal_uInt32 nUDData(0);
                rSt.ReadUInt32(nUDData);
                switch (nPID)
                {
                    case 0x038F: pImpRec->nXAlign = nUDData; break;
                    case 0x0390:
                        delete pImpRec->pXRelTo;
                        pImpRec->pXRelTo = new sal_uInt32;
                        *(pImpRec->pXRelTo) = nUDData;
                        break;
                    case 0x0391: pImpRec->nYAlign = nUDData; break;
                    case 0x0392:
                        delete pImpRec->pYRelTo;
                        pImpRec->pYRelTo = new sal_uInt32;
                        *(pImpRec->pYRelTo) = nUDData;
                        break;
                    case 0x03BF: pImpRec->nLayoutInTableCell = nUDData; break;
                    case 0x0393:
                    // This seems to correspond to o:hrpct from .docx (even including
                    // the difference that it's in 0.1% even though the .docx spec
                    // says it's in 1%).
                        pImpRec->relativeHorizontalWidth = nUDData;
                        break;
                    case 0x0394:
                    // And this is really just a guess, but a mere presence of this
                    // flag makes a horizontal rule be as wide as the page (unless
                    // overridden by something), so it probably matches o:hr from .docx.
                        pImpRec->isHorizontalRule = true;
                        break;
                }
                if (!rSt.good())
                    break;
                nBytesLeft  -= 6;
            }
        }

        //  text frame, also Title or Outline
        SdrObject*  pOrgObj  = pObj;
        SdrRectObj* pTextObj = nullptr;
        sal_uInt32 nTextId = GetPropertyValue( DFF_Prop_lTxid, 0 );
        if( nTextId )
        {
            SfxItemSet aSet( pSdrModel->GetItemPool() );

            //Originally anything that as a mso_sptTextBox was created as a
            //textbox, this was changed for #88277# to be created as a simple
            //rect to keep impress happy. For the rest of us we'd like to turn
            //it back into a textbox again.
            bool bTextFrame = (pImpRec->eShapeType == mso_sptTextBox);
            if (!bTextFrame)
            {
                //Either
                //a) it's a simple text object or
                //b) it's a rectangle with text and square wrapping.
                bTextFrame =
                (
                    (pImpRec->eShapeType == mso_sptTextSimple) ||
                    (
                        (pImpRec->eShapeType == mso_sptRectangle)
                        && (eWrapMode == mso_wrapSquare)
                        && ShapeHasText(pImpRec->nShapeId, rObjData.rSpHd.GetRecBegFilePos() )
                    )
                );
            }

            if (bTextFrame)
            {
                SdrObject::Free( pObj );
                pObj = pOrgObj = nullptr;
            }

            // Distance of Textbox to its surrounding Customshape
            sal_Int32 nTextLeft = GetPropertyValue( DFF_Prop_dxTextLeft, 91440L);
            sal_Int32 nTextRight = GetPropertyValue( DFF_Prop_dxTextRight, 91440L );
            sal_Int32 nTextTop = GetPropertyValue( DFF_Prop_dyTextTop, 45720L  );
            sal_Int32 nTextBottom = GetPropertyValue( DFF_Prop_dyTextBottom, 45720L );

            ScaleEmu( nTextLeft );
            ScaleEmu( nTextRight );
            ScaleEmu( nTextTop );
            ScaleEmu( nTextBottom );

            sal_Int32 nTextRotationAngle=0;
            bool bVerticalText = false;
            if ( IsProperty( DFF_Prop_txflTextFlow ) )
            {
                MSO_TextFlow eTextFlow = (MSO_TextFlow)(GetPropertyValue(
                    DFF_Prop_txflTextFlow, 0) & 0xFFFF);
                switch( eTextFlow )
                {
                    case mso_txflBtoT:
                        nTextRotationAngle = 9000;
                    break;
                    case mso_txflVertN:
                    case mso_txflTtoBN:
                        nTextRotationAngle = 27000;
                        break;
                    case mso_txflTtoBA:
                        bVerticalText = true;
                    break;
                    case mso_txflHorzA:
                        bVerticalText = true;
                        nTextRotationAngle = 9000;
                    break;
                    case mso_txflHorzN:
                    default :
                        break;
                }
            }

            if (nTextRotationAngle)
            {
                switch (nTextRotationAngle)
                {
                    case 9000:
                        {
                            long nWidth = rTextRect.GetWidth();
                            rTextRect.Right() = rTextRect.Left() + rTextRect.GetHeight();
                            rTextRect.Bottom() = rTextRect.Top() + nWidth;

                            sal_Int32 nOldTextLeft = nTextLeft;
                            sal_Int32 nOldTextRight = nTextRight;
                            sal_Int32 nOldTextTop = nTextTop;
                            sal_Int32 nOldTextBottom = nTextBottom;

                            nTextLeft = nOldTextBottom;
                            nTextRight = nOldTextTop;
                            nTextTop = nOldTextLeft;
                            nTextBottom = nOldTextRight;
                        }
                        break;
                    case 27000:
                        {
                            long nWidth = rTextRect.GetWidth();
                            rTextRect.Right() = rTextRect.Left() + rTextRect.GetHeight();
                            rTextRect.Bottom() = rTextRect.Top() + nWidth;

                            sal_Int32 nOldTextLeft = nTextLeft;
                            sal_Int32 nOldTextRight = nTextRight;
                            sal_Int32 nOldTextTop = nTextTop;
                            sal_Int32 nOldTextBottom = nTextBottom;

                            nTextLeft = nOldTextTop;
                            nTextRight = nOldTextBottom;
                            nTextTop = nOldTextRight;
                            nTextBottom = nOldTextLeft;
                        }
                        break;
                }
            }

            pTextObj = new SdrRectObj(OBJ_TEXT, rTextRect);
            pTextImpRec = new SvxMSDffImportRec(*pImpRec);
            bDeleteTextImpRec = true;

            // the vertical paragraph indents are part of the BoundRect,
            // here we 'remove' them by calculating
            Rectangle aNewRect(rTextRect);
            aNewRect.Bottom() -= nTextTop + nTextBottom;
            aNewRect.Right() -= nTextLeft + nTextRight;

            // Only if it's a simple textbox may Writer replace
            // the object with a frame, otherwise
            if( bTextFrame )
            {
                std::shared_ptr<SvxMSDffShapeInfo> const pTmpRec(
                    new SvxMSDffShapeInfo(0, pImpRec->nShapeId));

                SvxMSDffShapeInfos_ById::const_iterator const it =
                    m_xShapeInfosById->find(pTmpRec);
                if (it != m_xShapeInfosById->end())
                {
                    SvxMSDffShapeInfo& rInfo = **it;
                    pTextImpRec->bReplaceByFly   = rInfo.bReplaceByFly;
                }
            }

            if( !pObj )
                ApplyAttributes( rSt, aSet, rObjData );

            bool bFitText = false;
            if (GetPropertyValue(DFF_Prop_FitTextToShape, 0) & 2)
            {
                aSet.Put( makeSdrTextAutoGrowHeightItem( true ) );
                aSet.Put( makeSdrTextMinFrameHeightItem(
                    aNewRect.Bottom() - aNewRect.Top() ) );
                aSet.Put( makeSdrTextMinFrameWidthItem(
                    aNewRect.Right() - aNewRect.Left() ) );
                bFitText = true;
            }
            else
            {
                aSet.Put( makeSdrTextAutoGrowHeightItem( false ) );
                aSet.Put( makeSdrTextAutoGrowWidthItem( false ) );
            }

            switch ( (MSO_WrapMode)
                GetPropertyValue( DFF_Prop_WrapText, mso_wrapSquare ) )
            {
                case mso_wrapNone :
                    aSet.Put( makeSdrTextAutoGrowWidthItem( true ) );
                    if (bFitText)
                    {
                        //can't do autowidth in flys #i107184#
                        pTextImpRec->bReplaceByFly = false;
                    }
                break;
                case mso_wrapByPoints :
                    aSet.Put( makeSdrTextContourFrameItem( true ) );
                break;
                default: break;
            }

            // set margins at the border of the textbox
            aSet.Put( makeSdrTextLeftDistItem( nTextLeft ) );
            aSet.Put( makeSdrTextRightDistItem( nTextRight ) );
            aSet.Put( makeSdrTextUpperDistItem( nTextTop ) );
            aSet.Put( makeSdrTextLowerDistItem( nTextBottom ) );
            pTextImpRec->nDxTextLeft    = nTextLeft;
            pTextImpRec->nDyTextTop     = nTextTop;
            pTextImpRec->nDxTextRight   = nTextRight;
            pTextImpRec->nDyTextBottom  = nTextBottom;

            // read text anchor
            if ( IsProperty( DFF_Prop_anchorText ) )
            {
                MSO_Anchor eTextAnchor =
                    (MSO_Anchor)GetPropertyValue( DFF_Prop_anchorText, 0 );

                SdrTextVertAdjust eTVA = SDRTEXTVERTADJUST_CENTER;
                bool bTVASet(false);
                SdrTextHorzAdjust eTHA = SDRTEXTHORZADJUST_CENTER;
                bool bTHASet(false);

                switch( eTextAnchor )
                {
                    case mso_anchorTop:
                    {
                        eTVA = SDRTEXTVERTADJUST_TOP;
                        bTVASet = true;
                    }
                    break;
                    case mso_anchorTopCentered:
                    {
                        eTVA = SDRTEXTVERTADJUST_TOP;
                        bTVASet = true;
                        bTHASet = true;
                    }
                    break;

                    case mso_anchorMiddle:
                        bTVASet = true;
                    break;
                    case mso_anchorMiddleCentered:
                    {
                        bTVASet = true;
                        bTHASet = true;
                    }
                    break;
                    case mso_anchorBottom:
                    {
                        eTVA = SDRTEXTVERTADJUST_BOTTOM;
                        bTVASet = true;
                    }
                    break;
                    case mso_anchorBottomCentered:
                    {
                        eTVA = SDRTEXTVERTADJUST_BOTTOM;
                        bTVASet = true;
                        bTHASet = true;
                    }
                    break;
                    default : break;
                }
                // insert
                if ( bTVASet )
                    aSet.Put( SdrTextVertAdjustItem( eTVA ) );
                if ( bTHASet )
                    aSet.Put( SdrTextHorzAdjustItem( eTHA ) );
            }

            pTextObj->SetMergedItemSet(aSet);
            pTextObj->SetModel(pSdrModel);

            if (bVerticalText)
                pTextObj->SetVerticalWriting(true);

            if (nTextRotationAngle)
            {
                long nMinWH = rTextRect.GetWidth() < rTextRect.GetHeight() ?
                    rTextRect.GetWidth() : rTextRect.GetHeight();
                nMinWH /= 2;
                Point aPivot(rTextRect.TopLeft());
                aPivot.X() += nMinWH;
                aPivot.Y() += nMinWH;
                double a = nTextRotationAngle * nPi180;
                pTextObj->NbcRotate(aPivot, nTextRotationAngle, sin(a), cos(a));
            }

            // rotate text with shape?
            if ( mnFix16Angle )
            {
                double a = mnFix16Angle * nPi180;
                pTextObj->NbcRotate( rObjData.aBoundRect.Center(), mnFix16Angle,
                    sin( a ), cos( a ) );
            }

            if( !pObj )
            {
                pObj = pTextObj;
            }
            else
            {
                if( pTextObj != pObj )
                {
                    SdrObject* pGroup = new SdrObjGroup;
                    pGroup->GetSubList()->NbcInsertObject( pObj );
                    pGroup->GetSubList()->NbcInsertObject( pTextObj );
                    if (pOrgObj == pObj)
                        pOrgObj = pGroup;
                    else
                        pOrgObj = pObj;
                    pObj = pGroup;
                }
            }
        }
        else if( !pObj )
        {
            // simple rectangular objects are ignored by ImportObj()  :-(
            // this is OK for Draw but not for Calc and Writer
            // cause here these objects have a default border
            pObj = new SdrRectObj(rTextRect);
            pOrgObj = pObj;
            pObj->SetModel( pSdrModel );
            SfxItemSet aSet( pSdrModel->GetItemPool() );
            ApplyAttributes( rSt, aSet, rObjData );

            const SfxPoolItem* pPoolItem=nullptr;
            SfxItemState eState = aSet.GetItemState( XATTR_FILLCOLOR,
                                                     false, &pPoolItem );
            if( SfxItemState::DEFAULT == eState )
                aSet.Put( XFillColorItem( OUString(),
                          Color( mnDefaultColor ) ) );
            pObj->SetMergedItemSet(aSet);
        }

        //Means that fBehindDocument is set
        if (GetPropertyValue(DFF_Prop_fPrint, 0) & 0x20)
            pImpRec->bDrawHell = true;
        else
            pImpRec->bDrawHell = false;
        if (GetPropertyValue(DFF_Prop_fPrint, 0) & 0x02)
            pImpRec->bHidden = true;
        pTextImpRec->bDrawHell  = pImpRec->bDrawHell;
        pTextImpRec->bHidden = pImpRec->bHidden;
        pImpRec->nNextShapeId   = GetPropertyValue( DFF_Prop_hspNext, 0 );
        pTextImpRec->nNextShapeId=pImpRec->nNextShapeId;

        if ( nTextId )
        {
            pTextImpRec->aTextId.nTxBxS = (sal_uInt16)( nTextId >> 16 );
            pTextImpRec->aTextId.nSequence = (sal_uInt16)nTextId;
        }

        pTextImpRec->nDxWrapDistLeft = GetPropertyValue(
                                    DFF_Prop_dxWrapDistLeft, 114935L ) / 635L;
        pTextImpRec->nDyWrapDistTop = GetPropertyValue(
                                    DFF_Prop_dyWrapDistTop, 0 ) / 635L;
        pTextImpRec->nDxWrapDistRight = GetPropertyValue(
                                    DFF_Prop_dxWrapDistRight, 114935L ) / 635L;
        pTextImpRec->nDyWrapDistBottom = GetPropertyValue(
                                    DFF_Prop_dyWrapDistBottom, 0 ) / 635L;
        // 16.16 fraction times total image width or height, as appropriate.

        if (SeekToContent(DFF_Prop_pWrapPolygonVertices, rSt))
        {
            delete pTextImpRec->pWrapPolygon;
            pTextImpRec->pWrapPolygon = nullptr;
            sal_uInt16 nNumElemVert(0), nNumElemMemVert(0), nElemSizeVert(0);
            rSt.ReadUInt16( nNumElemVert ).ReadUInt16( nNumElemMemVert ).ReadUInt16( nElemSizeVert );
            bool bOk = false;
            if (nNumElemVert && ((nElemSizeVert == 8) || (nElemSizeVert == 4)))
            {
                bOk = rSt.remainingSize() / nElemSizeVert >= nNumElemVert;
            }
            if (bOk)
            {
                pTextImpRec->pWrapPolygon = new tools::Polygon(nNumElemVert);
                for (sal_uInt16 i = 0; i < nNumElemVert; ++i)
                {
                    sal_Int32 nX(0), nY(0);
                    if (nElemSizeVert == 8)
                        rSt.ReadInt32( nX ).ReadInt32( nY );
                    else
                    {
                        sal_Int16 nSmallX(0), nSmallY(0);
                        rSt.ReadInt16( nSmallX ).ReadInt16( nSmallY );
                        nX = nSmallX;
                        nY = nSmallY;
                    }
                    (*(pTextImpRec->pWrapPolygon))[i].X() = nX;
                    (*(pTextImpRec->pWrapPolygon))[i].Y() = nY;
                }
            }
        }

        pImpRec->nCropFromTop = GetPropertyValue(
                                    DFF_Prop_cropFromTop, 0 );
        pImpRec->nCropFromBottom = GetPropertyValue(
                                    DFF_Prop_cropFromBottom, 0 );
        pImpRec->nCropFromLeft = GetPropertyValue(
                                    DFF_Prop_cropFromLeft, 0 );
        pImpRec->nCropFromRight = GetPropertyValue(
                                    DFF_Prop_cropFromRight, 0 );

        pImpRec->bVFlip = (rObjData.nSpFlags & SP_FFLIPV) != 0;
        pImpRec->bHFlip = (rObjData.nSpFlags & SP_FFLIPH) != 0;

        sal_uInt32 nLineFlags = GetPropertyValue( DFF_Prop_fNoLineDrawDash, 0 );
        pImpRec->eLineStyle = (nLineFlags & 8)
                            ? (MSO_LineStyle)GetPropertyValue(
                                                DFF_Prop_lineStyle,
                                                mso_lineSimple )
                            : MSO_LineStyle_NONE;
        pTextImpRec->eLineStyle = pImpRec->eLineStyle;

        pImpRec->eLineDashing = (MSO_LineDashing)GetPropertyValue(
                DFF_Prop_lineDashing, mso_lineSolid );
        pTextImpRec->eLineDashing = pImpRec->eLineDashing;

        if( pImpRec->nShapeId )
        {
            // Import-Record-Liste ergaenzen
            if( pOrgObj )
            {
                pImpRec->pObj = pOrgObj;
                rImportData.m_Records.insert(std::unique_ptr<SvxMSDffImportRec>(pImpRec));
                bDeleteImpRec = false;
                if (pImpRec == pTextImpRec)
                    bDeleteTextImpRec = false;
            }

            if( pTextObj && (pOrgObj != pTextObj) )
            {
                // Modify ShapeId (must be unique)
                pImpRec->nShapeId |= 0x8000000;
                pTextImpRec->pObj = pTextObj;
                rImportData.m_Records.insert(std::unique_ptr<SvxMSDffImportRec>(pTextImpRec));
                bDeleteTextImpRec = false;
                if (pTextImpRec == pImpRec)
                    bDeleteImpRec = false;
            }

            // entry in the z-order-list in order to complement the pointer to this object
            /*Only store objects which are not deep inside the tree*/
            if( ( rObjData.nCalledByGroup == 0 )
                ||
                ( (rObjData.nSpFlags & SP_FGROUP)
                 && (rObjData.nCalledByGroup < 2) )
              )
                StoreShapeOrder( pImpRec->nShapeId,
                                ( ( (sal_uLong)pImpRec->aTextId.nTxBxS ) << 16 )
                                    + pImpRec->aTextId.nSequence, pObj );
        }

        if (bDeleteImpRec)
            delete pImpRec;

        if (bDeleteTextImpRec)
            delete pTextImpRec;
    }

    return pObj;
};

SdrObject* SvxMSDffManager::FinalizeObj(DffObjData& /* rObjData */, SdrObject* pObj)
{
    return pObj;
}


void SvxMSDffManager::StoreShapeOrder(sal_uLong         nId,
                                      sal_uLong         nTxBx,
                                      SdrObject*    pObject,
                                      SwFlyFrameFormat*  pFly,
                                      short         nHdFtSection) const
{
    sal_uInt16 nShpCnt = m_pShapeOrders->size();
    for (sal_uInt16 nShapeNum=0; nShapeNum < nShpCnt; nShapeNum++)
    {
        SvxMSDffShapeOrder& rOrder = *(*m_pShapeOrders)[ nShapeNum ];

        if( rOrder.nShapeId == nId )
        {
            rOrder.nTxBxComp = nTxBx;
            rOrder.pObj      = pObject;
            rOrder.pFly      = pFly;
            rOrder.nHdFtSection = nHdFtSection;
        }
    }
}


void SvxMSDffManager::ExchangeInShapeOrder( SdrObject*   pOldObject,
                                            sal_uLong    nTxBx,
                                            SdrObject*   pObject) const
{
    sal_uInt16 nShpCnt = m_pShapeOrders->size();
    for (sal_uInt16 nShapeNum=0; nShapeNum < nShpCnt; nShapeNum++)
    {
        SvxMSDffShapeOrder& rOrder = *(*m_pShapeOrders)[ nShapeNum ];

        if( rOrder.pObj == pOldObject )
        {
            rOrder.pFly      = nullptr;
            rOrder.pObj      = pObject;
            rOrder.nTxBxComp = nTxBx;
        }
    }
}


void SvxMSDffManager::RemoveFromShapeOrder( SdrObject* pObject ) const
{
    sal_uInt16 nShpCnt = m_pShapeOrders->size();
    for (sal_uInt16 nShapeNum=0; nShapeNum < nShpCnt; nShapeNum++)
    {
        SvxMSDffShapeOrder& rOrder = *(*m_pShapeOrders)[ nShapeNum ];

        if( rOrder.pObj == pObject )
        {
            rOrder.pObj      = nullptr;
            rOrder.pFly      = nullptr;
            rOrder.nTxBxComp = 0;
        }
    }
}


//  exported class: Public Methods

SvxMSDffManager::SvxMSDffManager(SvStream& rStCtrl_,
                                 const OUString& rBaseURL,
                                 sal_uInt32 nOffsDgg_,
                                 SvStream* pStData_,
                                 SdrModel* pSdrModel_,// see SetModel() below
                                 long      nApplicationScale,
                                 ColorData mnDefaultColor_,
                                 SvStream* pStData2_,
                                 bool bSkipImages )
    :DffPropertyReader( *this ),
     m_pBLIPInfos( new SvxMSDffBLIPInfos ),
     m_xShapeInfosByTxBxComp( new SvxMSDffShapeInfos_ByTxBxComp ),
     m_pShapeOrders( new SvxMSDffShapeOrders ),
     nOffsDgg( nOffsDgg_ ),
     nBLIPCount(  USHRT_MAX ),              // initialize with error, since we fist check if the
     nGroupShapeFlags(0),                   // ensure initialization here, as some corrupted
                                            // files may yield to this being uninitialized
     maBaseURL( rBaseURL ),
     mnCurMaxShapeId(0),
     mnDrawingsSaved(0),
     mnIdClusters(0),
     rStCtrl(  rStCtrl_  ),
     pStData(  pStData_  ),
     pStData2( pStData2_ ),
     nSvxMSDffSettings( 0 ),
     nSvxMSDffOLEConvFlags( 0 ),
     pSecPropSet( nullptr ),
     mnDefaultColor( mnDefaultColor_),
     mbSkipImages (bSkipImages)
{
    SetModel( pSdrModel_, nApplicationScale );

    // remember FilePos of the stream(s)
    sal_uLong nOldPosCtrl = rStCtrl.Tell();
    sal_uLong nOldPosData = pStData ? pStData->Tell() : nOldPosCtrl;

    // if no data stream is given we assume that the BLIPs
    // are in the control stream
    if( !pStData )
        pStData = &rStCtrl;

    SetDefaultPropSet( rStCtrl, nOffsDgg );

    // read control stream, if successful set nBLIPCount
    GetCtrlData( nOffsDgg );

    // check Text-Box-Story-Chain-Infos
    CheckTxBxStoryChain();

    // restore old FilePos of the stream(s)
    rStCtrl.Seek( nOldPosCtrl );
    if( &rStCtrl != pStData )
        pStData->Seek( nOldPosData );
}

SvxMSDffManager::SvxMSDffManager( SvStream& rStCtrl_, const OUString& rBaseURL )
    :DffPropertyReader( *this ),
     m_pBLIPInfos( new SvxMSDffBLIPInfos ),
     m_xShapeInfosByTxBxComp( new SvxMSDffShapeInfos_ByTxBxComp ),
     m_pShapeOrders( new SvxMSDffShapeOrders ),
     nOffsDgg( 0 ),
     nBLIPCount(  USHRT_MAX ),              // initialize with error, since we first have to check
     nGroupShapeFlags(0),
     maBaseURL( rBaseURL ),
     mnCurMaxShapeId(0),
     mnDrawingsSaved(0),
     mnIdClusters(0),
     rStCtrl(  rStCtrl_  ),
     pStData( nullptr ),
     pStData2( nullptr ),
     nSvxMSDffSettings( 0 ),
     nSvxMSDffOLEConvFlags( 0 ),
     pSecPropSet( nullptr ),
     mnDefaultColor( COL_DEFAULT ),
     mbSkipImages(false)
{
    SetModel( nullptr, 0 );
}

SvxMSDffManager::~SvxMSDffManager()
{
    delete pSecPropSet;
    delete m_pBLIPInfos;
    delete m_pShapeOrders;
}

void SvxMSDffManager::InitSvxMSDffManager( sal_uInt32 nOffsDgg_, SvStream* pStData_, sal_uInt32 nOleConvFlags )
{
    nOffsDgg = nOffsDgg_;
    pStData = pStData_;
    nSvxMSDffOLEConvFlags = nOleConvFlags;

    // remember FilePos of the stream(s)
    sal_uLong nOldPosCtrl = rStCtrl.Tell();

    SetDefaultPropSet( rStCtrl, nOffsDgg );

    // insert fidcl cluster table
    GetFidclData( nOffsDgg );

    // read control stream, if successful, set nBLIPCount
    GetCtrlData( nOffsDgg );

    // check Text-Box-Story-Chain-Infos
    CheckTxBxStoryChain();

    // restore old FilePos of the stream(s)
    rStCtrl.Seek( nOldPosCtrl );
}

void SvxMSDffManager::SetDgContainer( SvStream& rSt )
{
    sal_uInt32 nFilePos = rSt.Tell();
    DffRecordHeader aDgContHd;
    bool bOk = ReadDffRecordHeader(rSt, aDgContHd);
    // insert this container only if there is also a DggAtom
    if (bOk && SeekToRec(rSt, DFF_msofbtDg, aDgContHd.GetRecEndFilePos()))
    {
        DffRecordHeader aRecHd;
        if (ReadDffRecordHeader(rSt, aRecHd))
        {
            sal_uInt32 nDrawingId = aRecHd.nRecInstance;
            maDgOffsetTable[nDrawingId] = nFilePos;
        }
    }
    rSt.Seek(nFilePos);
}

void SvxMSDffManager::GetFidclData( sal_uInt32 nOffsDggL )
{
    if (!nOffsDggL)
        return;

    sal_uInt32 nDummy, nMerk = rStCtrl.Tell();

    if (nOffsDggL == rStCtrl.Seek(nOffsDggL))
    {
        DffRecordHeader aRecHd;
        bool bOk = ReadDffRecordHeader(rStCtrl, aRecHd);

        DffRecordHeader aDggAtomHd;
        if (bOk && SeekToRec(rStCtrl, DFF_msofbtDgg, aRecHd.GetRecEndFilePos(), &aDggAtomHd))
        {
            aDggAtomHd.SeekToContent( rStCtrl );
            rStCtrl.ReadUInt32( mnCurMaxShapeId )
                   .ReadUInt32( mnIdClusters )
                   .ReadUInt32( nDummy )
                   .ReadUInt32( mnDrawingsSaved );

            if ( mnIdClusters-- > 2 )
            {
                const std::size_t nFIDCLsize = sizeof(sal_uInt32) * 2;
                if ( aDggAtomHd.nRecLen == ( mnIdClusters * nFIDCLsize + 16 ) )
                {
                    std::size_t nMaxEntriesPossible = rStCtrl.remainingSize() / nFIDCLsize;
                    SAL_WARN_IF(nMaxEntriesPossible < mnIdClusters,
                        "filter.ms", "FIDCL list longer than remaining bytes, ppt or parser is wrong");
                    mnIdClusters = std::min(nMaxEntriesPossible, static_cast<std::size_t>(mnIdClusters));

                    maFidcls.resize(mnIdClusters);
                    for (sal_uInt32 i = 0; i < mnIdClusters; ++i)
                    {
                        rStCtrl.ReadUInt32( maFidcls[ i ].dgid )
                               .ReadUInt32( maFidcls[ i ].cspidCur );
                    }
                }
            }
        }
    }
    rStCtrl.Seek( nMerk );
}

void SvxMSDffManager::CheckTxBxStoryChain()
{
    m_xShapeInfosById.reset(new SvxMSDffShapeInfos_ById);
    // mangle old Info array, sorted by nTxBxComp
    sal_uLong nChain    = ULONG_MAX;
    bool bSetReplaceFALSE = false;
    for (SvxMSDffShapeInfos_ByTxBxComp::iterator iter =
                m_xShapeInfosByTxBxComp->begin(),
            mark = m_xShapeInfosByTxBxComp->begin();
         iter != m_xShapeInfosByTxBxComp->end(); ++iter)
    {
        std::shared_ptr<SvxMSDffShapeInfo> const pObj = *iter;
        if( pObj->nTxBxComp )
        {
            // group change?
            // the text id also contains an internal drawing container id
            // to distinguish between text id of drawing objects in different
            // drawing containers.
            if( nChain != pObj->nTxBxComp )
            {
                // reset mark and helper flag
                mark = iter;
                nChain = pObj->nTxBxComp;
                bSetReplaceFALSE = !pObj->bReplaceByFly;
            }
            else if( !pObj->bReplaceByFly )
            {
                // object that must NOT be replaced by frame?
                bSetReplaceFALSE = true;
                // maybe reset flags in start of group
                for (SvxMSDffShapeInfos_ByTxBxComp::iterator itemp = mark;
                        itemp != iter; ++itemp)
                {
                    (*itemp)->bReplaceByFly = false;
                }
            }

            if( bSetReplaceFALSE )
            {
                pObj->bReplaceByFly = false;
            }
        }
        // copy all Shape Info objects to m_xShapeInfosById, sorted by nShapeId
        pObj->nTxBxComp = pObj->nTxBxComp & 0xFFFF0000;
        m_xShapeInfosById->insert( pObj );
    }
    // free original array but don't free its elements
    m_xShapeInfosByTxBxComp.reset();
}


/*****************************************************************************

    Reading the Shape-Infos in the Ctor:
    ---------------------------------
    remembering the Shape-Ids and the associated Blip-Numbers und TextBox-Infos
                    =========                    ============     =============
    and remembering the File-Offsets for each Blip
                       ============
******************************************************************************/
void SvxMSDffManager::GetCtrlData( sal_uInt32 nOffsDgg_ )
{
    // absolutely remember Start Offset, in case we have to position again
    sal_uInt32 nOffsDggL = nOffsDgg_;

    // position control stream
    if (nOffsDggL != rStCtrl.Seek(nOffsDggL))
        return;

    sal_uInt8   nVer;
    sal_uInt16 nInst;
    sal_uInt16 nFbt;
    sal_uInt32  nLength;
    if( !ReadCommonRecordHeader( rStCtrl, nVer, nInst, nFbt, nLength ) ) return;

    sal_uLong nPos = nOffsDggL + DFF_COMMON_RECORD_HEADER_SIZE;

    // case A: first Drawing Group Container, then n times Drawing Container
    if( DFF_msofbtDggContainer == nFbt )
    {
        bool bOk;
        GetDrawingGroupContainerData( rStCtrl, nLength );

        rStCtrl.Seek( STREAM_SEEK_TO_END );
        sal_uInt32 nMaxStrPos = rStCtrl.Tell();

        nPos += nLength;
        unsigned long nDrawingContainerId = 1;
        do
        {
            if (nPos != rStCtrl.Seek(nPos))
                break;

            bOk = ReadCommonRecordHeader( rStCtrl, nVer, nInst, nFbt, nLength ) && ( DFF_msofbtDgContainer == nFbt );

            if( !bOk )
            {
                nPos++;                // ????????? TODO: trying to get an one-hit wonder, this code code should be rewritten...
                if (nPos != rStCtrl.Seek(nPos))
                    break;
                bOk = ReadCommonRecordHeader( rStCtrl, nVer, nInst, nFbt, nLength )
                        && ( DFF_msofbtDgContainer == nFbt );
            }
            if( bOk )
            {
                GetDrawingContainerData( rStCtrl, nLength, nDrawingContainerId );
            }
            nPos += DFF_COMMON_RECORD_HEADER_SIZE + nLength;
            ++nDrawingContainerId;
        }
        while( ( rStCtrl.GetError() == 0 ) && ( nPos < nMaxStrPos ) && bOk );
    }
}


// from here on: Drawing Group Container  i.e. document-wide valid data

void SvxMSDffManager::GetDrawingGroupContainerData( SvStream& rSt, sal_uLong nLenDgg )
{
    sal_uInt8   nVer;
    sal_uInt16 nInst;
    sal_uInt16 nFbt;
    sal_uInt32 nLength;

    sal_uLong nLenBStoreCont = 0, nLenFBSE = 0, nRead = 0;

    // search for a  BStore Container
    do
    {
        if(!ReadCommonRecordHeader( rSt, nVer, nInst, nFbt, nLength)) return;
        nRead += DFF_COMMON_RECORD_HEADER_SIZE + nLength;
        if( DFF_msofbtBstoreContainer == nFbt )
        {
            nLenBStoreCont = nLength;       break;
        }
        rSt.SeekRel( nLength );
    }
    while( nRead < nLenDgg );

    if( !nLenBStoreCont ) return;

    // Read all atoms of the containers from the BStore container and store all
    // relevant data of all contained FBSEs in out pointer array.
    // We also count all found FBSEs in member nBLIPCount.

    const sal_uLong nSkipBLIPLen = 20;  // skip to get to the nBLIPLen
    const sal_uLong nSkipBLIPPos =  4;  // thereafter skip up to nBLIPPos

    sal_uInt32 nBLIPLen = 0, nBLIPPos = 0;

    nRead = 0;
    do
    {
        if(!ReadCommonRecordHeader( rSt, nVer, nInst, nFbt, nLength)) return;
        nRead += DFF_COMMON_RECORD_HEADER_SIZE + nLength;
        if( DFF_msofbtBSE == nFbt && /* magic value from spec */ 0x2 == nVer )
        {
            nLenFBSE = nLength;
            // is FBSE big enough for our data
            bool bOk = ( nSkipBLIPLen + 4 + nSkipBLIPPos + 4 <= nLenFBSE );

            if( bOk )
            {
                rSt.SeekRel( nSkipBLIPLen );
                rSt.ReadUInt32( nBLIPLen );
                rSt.SeekRel( nSkipBLIPPos );
                rSt.ReadUInt32( nBLIPPos );
                bOk = rSt.GetError() == 0;

                nLength -= nSkipBLIPLen+ 4 + nSkipBLIPPos + 4;
            }

            if( bOk )
            {
                // specialty:
                // If nBLIPLen is less than nLenFBSE AND nBLIPPos is NULL,
                // then we assume, that the image is in FBSE!
                if( (!nBLIPPos) && (nBLIPLen < nLenFBSE) )
                    nBLIPPos = rSt.Tell() + 4;

                if( USHRT_MAX == nBLIPCount )
                    nBLIPCount = 1;
                else
                    nBLIPCount++;

                // now save the info for later access
                m_pBLIPInfos->push_back(SvxMSDffBLIPInfo(nBLIPPos));
            }
            rSt.SeekRel( nLength );
        }
        else return; // invalid input
    }
    while( nRead < nLenBStoreCont );
}


// from now on: Drawing Container  which means Pages (Sheet, Slide) - wide valid data
//                      =================               ======

void SvxMSDffManager::GetDrawingContainerData( SvStream& rSt, sal_uLong nLenDg,
                                               const unsigned long nDrawingContainerId )
{
    sal_uInt8 nVer;sal_uInt16 nInst;sal_uInt16 nFbt;sal_uInt32 nLength;

    sal_uLong nReadDg = 0;

    // We are now in a drawing container (one per each page) and
    // we now have to iterate through all contained shape group containers
    do
    {
        if(!ReadCommonRecordHeader( rSt, nVer, nInst, nFbt, nLength)) return;
        nReadDg += DFF_COMMON_RECORD_HEADER_SIZE;
        // Patriarch found (the upmost shape group container) ?
        if( DFF_msofbtSpgrContainer == nFbt )
        {
            if(!this->GetShapeGroupContainerData( rSt, nLength, true, nDrawingContainerId )) return;
        }
        // empty Shape Container ? (outside of shape group container)
        else if( DFF_msofbtSpContainer == nFbt )
        {
            if(!this->GetShapeContainerData( rSt, nLength, ULONG_MAX, nDrawingContainerId )) return;
        }
        else
            rSt.SeekRel( nLength );
        nReadDg += nLength;
    }
    while( nReadDg < nLenDg );
}

bool SvxMSDffManager::GetShapeGroupContainerData( SvStream& rSt,
                                                  sal_uLong nLenShapeGroupCont,
                                                  bool bPatriarch,
                                                  const unsigned long nDrawingContainerId )
{
    sal_uInt8 nVer;sal_uInt16 nInst;sal_uInt16 nFbt;sal_uInt32 nLength;
    long nStartShapeGroupCont = rSt.Tell();
    // We are now in a shape group container (conditionally mulitple per page)
    // an we now have to iterate through all contained shape containers
    bool  bFirst = !bPatriarch;
    sal_uLong nReadSpGrCont = 0;
    do
    {
        if( !ReadCommonRecordHeader( rSt, nVer, nInst, nFbt, nLength ) )
            return false;
        nReadSpGrCont += DFF_COMMON_RECORD_HEADER_SIZE;
        // Shape Container?
        if( DFF_msofbtSpContainer == nFbt )
        {
            sal_uLong nGroupOffs = bFirst ? nStartShapeGroupCont - DFF_COMMON_RECORD_HEADER_SIZE : ULONG_MAX;
            if ( !this->GetShapeContainerData( rSt, nLength, nGroupOffs, nDrawingContainerId ) )
                return false;
            bFirst = false;
        }
        // nested shape group container ?
        else if( DFF_msofbtSpgrContainer == nFbt )
        {
            if ( !this->GetShapeGroupContainerData( rSt, nLength, false, nDrawingContainerId ) )
                return false;
        }
        else
            rSt.SeekRel( nLength );
        nReadSpGrCont += nLength;
    }
    while( nReadSpGrCont < nLenShapeGroupCont );
    // position the stream correctly
    rSt.Seek( nStartShapeGroupCont + nLenShapeGroupCont );
    return true;
}

bool SvxMSDffManager::GetShapeContainerData( SvStream& rSt,
                                             sal_uLong nLenShapeCont,
                                             sal_uLong nPosGroup,
                                             const unsigned long nDrawingContainerId )
{
    sal_uInt8 nVer;sal_uInt16 nInst;sal_uInt16 nFbt;sal_uInt32 nLength;
    long  nStartShapeCont = rSt.Tell();

    // We are in a shape container (possibly more than one per shape group) and we now
    // have to fetch the shape id and file position (to be able to access them again later)
    // and the first BStore reference (if present).
    sal_uLong nLenShapePropTbl = 0;
    sal_uLong nReadSpCont = 0;

    // Store file offset of the shape containers or respectively the group(!).
    sal_uLong nStartOffs = (ULONG_MAX > nPosGroup) ?
                            nPosGroup : nStartShapeCont - DFF_COMMON_RECORD_HEADER_SIZE;
    SvxMSDffShapeInfo aInfo( nStartOffs );

    // Can the shape be replaced with a frame?
    // (provided that it is a TextBox and the text is not rotated)
    bool bCanBeReplaced = nPosGroup >= ULONG_MAX;

    // we don't know yet whether it's a TextBox
    MSO_SPT         eShapeType      = mso_sptNil;

    // analyze Shape

    do
    {
        if(!ReadCommonRecordHeader( rSt, nVer, nInst, nFbt, nLength)) return false;
        nReadSpCont += DFF_COMMON_RECORD_HEADER_SIZE;
        // FSP ?
        if( ( DFF_msofbtSp == nFbt ) && ( 4 <= nLength ) )
        {
            // we've found the FSP gefunden: note Shape Type and Id!
            eShapeType = (MSO_SPT)nInst;
            rSt.ReadUInt32( aInfo.nShapeId );
            rSt.SeekRel( nLength - 4 );
            nReadSpCont += nLength;
        }
        else if( DFF_msofbtOPT == nFbt ) // Shape Property Table ?
        {
            // We've found the Property Table:
            // search for the Blip Property!
            sal_uLong  nPropRead = 0;
            sal_uInt16 nPropId;
            sal_uInt32  nPropVal;
            nLenShapePropTbl = nLength;
            long nStartShapePropTbl = rSt.Tell();
            do
            {
                rSt.ReadUInt16( nPropId )
                   .ReadUInt32( nPropVal );
                nPropRead += 6;

                switch( nPropId )
                {
                    case DFF_Prop_txflTextFlow :
                        //Writer can now handle vertical textflows in its
                        //native frames, to only need to do this for the
                        //other two formats

                        //Writer will handle all textflow except BtoT
                        if (GetSvxMSDffSettings() &
                            (SVXMSDFF_SETTINGS_IMPORT_PPT |
                             SVXMSDFF_SETTINGS_IMPORT_EXCEL))
                        {
                            if( 0 != nPropVal )
                                bCanBeReplaced = false;
                        }
                        else if (
                            (nPropVal != mso_txflHorzN) &&
                            (nPropVal != mso_txflTtoBA)
                                )
                        {
                            bCanBeReplaced = false;
                        }
                    break;
                    case DFF_Prop_cdirFont :
                        //Writer can now handle right to left and left
                        //to right in its native frames, so only do
                        //this for the other two formats.
                        if (GetSvxMSDffSettings() &
                            (SVXMSDFF_SETTINGS_IMPORT_PPT |
                             SVXMSDFF_SETTINGS_IMPORT_EXCEL))
                        {
                            if( 0 != nPropVal )
                                bCanBeReplaced = false;
                        }
                    break;
                    case DFF_Prop_Rotation :
                        if( 0 != nPropVal )
                            bCanBeReplaced = false;
                    break;

                    case DFF_Prop_gtextFStrikethrough :
                        if( ( 0x20002000 & nPropVal )  == 0x20002000 )
                            bCanBeReplaced = false;
                    break;

                    case DFF_Prop_fc3DLightFace :
                        if( ( 0x00080008 & nPropVal ) == 0x00080008 )
                            bCanBeReplaced = false;
                    break;

                    case DFF_Prop_WrapText :
                        //TODO: eWrapMode = (MSO_WrapMode)nPropVal;
                    break;

                    default:
                    {
                        // is the Bit set and valid?
                        if( 0x4000 == ( nPropId & 0xC000 ) )
                        {
                            // Blip Property found: remember BStore Idx!
                            nPropRead = nLenShapePropTbl;
                        }
                        else if( 0x8000 & nPropId )
                        {
                            // complex Prop found:
                            // Length is always 6. The length of the appended extra data
                            // after the actual prop table is of different size.
                            nPropVal = 6;
                        }
                    }
                    break;
                }
            }
            while( nPropRead < nLenShapePropTbl );
            rSt.Seek( nStartShapePropTbl + nLenShapePropTbl );
            nReadSpCont += nLenShapePropTbl;
        }
        else if( ( DFF_msofbtClientTextbox == nFbt ) && ( 4 == nLength ) )  // Text-Box-Story-Entry found
        {
            rSt.ReadUInt32( aInfo.nTxBxComp );
            // Add internal drawing container id to text id.
            // Note: The text id uses the first two bytes, while the internal
            // drawing container id used the second two bytes.
            aInfo.nTxBxComp = ( aInfo.nTxBxComp & 0xFFFF0000 ) +
                              nDrawingContainerId;
            DBG_ASSERT( (aInfo.nTxBxComp & 0x0000FFFF) == nDrawingContainerId,
                        "<SvxMSDffManager::GetShapeContainerData(..)> - internal drawing container Id could not be correctly merged into DFF_msofbtClientTextbox value." );
        }
        else
        {
            rSt.SeekRel( nLength );
            nReadSpCont += nLength;
        }
    }
    while( nReadSpCont < nLenShapeCont );


    // Now possibly store the information for subsequent accesses to the shape

    if( aInfo.nShapeId )
    {
        // Possibly allow replacement of textboxes with frames
        if(     bCanBeReplaced
             && aInfo.nTxBxComp
             && (
                    ( eShapeType == mso_sptTextSimple )
                 || ( eShapeType == mso_sptTextBox    )
                 || (    (    ( eShapeType == mso_sptRectangle      )
                           || ( eShapeType == mso_sptRoundRectangle )
                         )
                ) ) )
        {
            aInfo.bReplaceByFly = true;
        }
        m_xShapeInfosByTxBxComp->insert(std::make_shared<SvxMSDffShapeInfo>(
                    aInfo));
        m_pShapeOrders->push_back(o3tl::make_unique<SvxMSDffShapeOrder>(
                    aInfo.nShapeId ));
    }

    // and position the Stream correctly again
    rSt.Seek( nStartShapeCont + nLenShapeCont );
    return true;
}


/*****************************************************************************

    Access to a shape at runtime (via the Shape-Id)
    ----------------------------
******************************************************************************/
bool SvxMSDffManager::GetShape(sal_uLong nId, SdrObject*&         rpShape,
                                          SvxMSDffImportData& rData)
{
    std::shared_ptr<SvxMSDffShapeInfo> const pTmpRec(
        new SvxMSDffShapeInfo(0, nId));

    SvxMSDffShapeInfos_ById::const_iterator const it =
        m_xShapeInfosById->find(pTmpRec);
    if (it != m_xShapeInfosById->end())
    {
        // Possibly delete old error flag.
        if( rStCtrl.GetError() )
            rStCtrl.ResetError();
        // store FilePos of the stream(s)
        sal_uLong nOldPosCtrl = rStCtrl.Tell();
        sal_uLong nOldPosData = pStData ? pStData->Tell() : nOldPosCtrl;
        // jump to the shape in the control stream
        sal_uLong const nFilePos((*it)->nFilePos);
        bool bSeeked = (nFilePos == rStCtrl.Seek(nFilePos));

        // if it failed, reset error statusF
        if (!bSeeked || rStCtrl.GetError())
            rStCtrl.ResetError();
        else
            rpShape = ImportObj( rStCtrl, &rData, rData.aParentRect, rData.aParentRect );

        // restore old alte FilePos of the stream(s)
        rStCtrl.Seek( nOldPosCtrl );
        if( &rStCtrl != pStData && pStData )
            pStData->Seek( nOldPosData );
        return ( nullptr != rpShape );
    }
    return false;
}


/*      access to a BLIP at runtime (if the Blip-Number is already known)
        ---------------------------
******************************************************************************/
bool SvxMSDffManager::GetBLIP( sal_uLong nIdx_, Graphic& rData, Rectangle* pVisArea )
{
    if (!pStData)
        return false;

    bool bOk = false;       // initialize result variable

    // check if a graphic for this blipId is already imported
    if ( nIdx_)
    {
        std::map<sal_uInt32,OString>::iterator iter = aEscherBlipCache.find(nIdx_);

        if (iter != aEscherBlipCache.end())
        {
            /* if this entry is available, then it should be possible
            to get the Graphic via GraphicObject */
            GraphicObject aGraphicObject( iter->second );
            rData = aGraphicObject.GetGraphic();
            if ( rData.GetType() != GraphicType::NONE )
                bOk = true;
            else
                aEscherBlipCache.erase(iter);
        }
    }

    if ( !bOk )
    {
        sal_uInt16 nIdx = sal_uInt16( nIdx_ );
        if (!nIdx || (m_pBLIPInfos->size() < nIdx))
            return false;

        // possibly delete old error flag(s)
        if( rStCtrl.GetError() )
            rStCtrl.ResetError();
        if(    ( &rStCtrl != pStData )
            && pStData->GetError() )
            pStData->ResetError();

        // remember FilePos of the stream(s)
        sal_uLong nOldPosCtrl = rStCtrl.Tell();
        sal_uLong nOldPosData = pStData ? pStData->Tell() : nOldPosCtrl;

        // fetch matching info struct out of the pointer array
        SvxMSDffBLIPInfo& rInfo = (*m_pBLIPInfos)[ nIdx-1 ];
        // jump to the BLIP atom in the data stream
        pStData->Seek( rInfo.nFilePos );
        // possibly reset error status
        if( pStData->GetError() )
            pStData->ResetError();
        else
            bOk = GetBLIPDirect( *pStData, rData, pVisArea );
        if( pStData2 && !bOk )
        {
            // Error, but the is a second chance: There is a second
            //         data stream in which the graphic could be stored!
            if( pStData2->GetError() )
                pStData2->ResetError();
            sal_uLong nOldPosData2 = pStData2->Tell();
            // jump to the BLIP atom in the second data stream
            pStData2->Seek( rInfo.nFilePos );
            // reset error status if necessary
            if( pStData2->GetError() )
                pStData2->ResetError();
            else
                bOk = GetBLIPDirect( *pStData2, rData, pVisArea );
            // restore of FilePos of the second data stream
            pStData2->Seek( nOldPosData2 );
        }
        // restore old FilePos of the stream(s)
        rStCtrl.Seek( nOldPosCtrl );
        if( &rStCtrl != pStData )
          pStData->Seek( nOldPosData );

        if ( bOk )
        {
            // create new BlipCacheEntry for this graphic
            GraphicObject aGraphicObject( rData );
            aEscherBlipCache.insert(std::make_pair(nIdx_,aGraphicObject.GetUniqueID()));
        }
    }

    return bOk;
}

/*      access to a BLIP at runtime (with correctly positioned stream)
    ---------------------------------
******************************************************************************/
bool SvxMSDffManager::GetBLIPDirect( SvStream& rBLIPStream, Graphic& rData, Rectangle* pVisArea )
{
    sal_uLong nOldPos = rBLIPStream.Tell();

    int nRes = GRFILTER_OPENERROR;  // initialize error variable

    // check whether it's really a BLIP
    sal_uInt32 nLength;
    sal_uInt16 nInst, nFbt( 0 );
    sal_uInt8   nVer;
    if( ReadCommonRecordHeader( rBLIPStream, nVer, nInst, nFbt, nLength) && ( 0xF018 <= nFbt ) && ( 0xF117 >= nFbt ) )
    {
        Size        aMtfSize100;
        bool        bMtfBLIP = false;
        bool        bZCodecCompression = false;
        // now position it exactly at the beginning of the embedded graphic
        sal_uLong nSkip = ( nInst & 0x0001 ) ? 32 : 16;

        switch( nInst & 0xFFFE )
        {
            case 0x216 :            // Metafile header then compressed WMF
            case 0x3D4 :            // Metafile header then compressed EMF
            case 0x542 :            // Metafile hd. then compressed PICT
            {
                rBLIPStream.SeekRel( nSkip + 20 );

                // read in size of metafile in EMUS
                sal_Int32 width, height;
                rBLIPStream.ReadInt32( width ).ReadInt32( height );
                aMtfSize100.Width() = width;
                aMtfSize100.Height() = height;

                // scale to 1/100mm
                aMtfSize100.Width() /= 360;
                aMtfSize100.Height() /= 360;

                if ( pVisArea )     // seem that we currently are skipping the visarea position
                    *pVisArea = Rectangle( Point(), aMtfSize100 );

                // skip rest of header
                nSkip = 6;
                bMtfBLIP = bZCodecCompression = true;
            }
            break;
            case 0x46A :            // One byte tag then JPEG (= JFIF) data
            case 0x6E0 :            // One byte tag then PNG data
            case 0x6E2 :            // One byte tag then JPEG in CMYK color space
            case 0x7A8 :
                nSkip += 1;         // One byte tag then DIB data
            break;
        }
        rBLIPStream.SeekRel( nSkip );

        SvStream* pGrStream = &rBLIPStream;
        std::unique_ptr<SvMemoryStream> xOut;
        if( bZCodecCompression )
        {
            xOut.reset(new SvMemoryStream( 0x8000, 0x4000 ));
            ZCodec aZCodec( 0x8000, 0x8000 );
            aZCodec.BeginCompression();
            aZCodec.Decompress( rBLIPStream, *xOut );
            aZCodec.EndCompression();
            xOut->Seek( STREAM_SEEK_TO_BEGIN );
            xOut->SetResizeOffset( 0 ); // sj: #i102257# setting ResizeOffset of 0 prevents from seeking
                                        // behind the stream end (allocating too much memory)
            pGrStream = xOut.get();
        }

#ifdef DEBUG_FILTER_MSDFFIMP
        // extract graphics from ole storage into "dbggfxNNN.*"
        static sal_Int32 nGrfCount;

        OUString aFileName = "dbggfx" + OUString::number( nGrfCount++ );
        switch( nInst &~ 1 )
        {
            case 0x216 : aFileName += ".wmf"; break;
            case 0x3d4 : aFileName += ".emf"; break;
            case 0x542 : aFileName += ".pct"; break;
            case 0x46a : aFileName += ".jpg"; break;
            case 0x6e0 : aFileName += ".png"; break;
            case 0x6e2 : aFileName += ".jpg"; break;
            case 0x7a8 : aFileName += ".bmp"; break;
        }

        OUString aURLStr;
        if( osl::FileBase::getFileURLFromSystemPath( Application::GetAppFileName(), aURLStr ) == osl::FileBase::E_None )
        {
            INetURLObject aURL( aURLStr );

            aURL.removeSegment();
            aURL.removeFinalSlash();
            aURL.Append( aFileName );

            aURLStr = aURL.GetMainURL( INetURLObject::DecodeMechanism::NONE );

            SAL_INFO("filter.ms", "dumping " << aURLStr);

            std::unique_ptr<SvStream> pDbgOut(::utl::UcbStreamHelper::CreateStream(aURLStr, StreamMode::TRUNC | StreamMode::WRITE));

            if( pDbgOut )
            {
                if ( bZCodecCompression )
                {
                    xOut->Seek( STREAM_SEEK_TO_END );
                    pDbgOut->Write( xOut->GetData(), xOut->Tell() );
                    xOut->Seek( STREAM_SEEK_TO_BEGIN );
                }
                else
                {
                    sal_Int32 nDbgLen = nLength - nSkip;
                    if ( nDbgLen )
                    {
                        std::unique_ptr<sal_Char[]> xDat(new sal_Char[ nDbgLen ]);
                        pGrStream->Read( xDat.get(), nDbgLen );
                        pDbgOut->Write( xDat.get(), nDbgLen );
                        pGrStream->SeekRel( -nDbgLen );
                    }
                }
            }
        }
#endif

        if( ( nInst & 0xFFFE ) == 0x7A8 )
        {   // getting the DIBs immediately
            Bitmap aNew;
            if( ReadDIB(aNew, *pGrStream, false) )
            {
                rData = Graphic( aNew );
                nRes = GRFILTER_OK;
            }
        }
        else
        {   // and unleash our filter
            GraphicFilter& rGF = GraphicFilter::GetGraphicFilter();
            nRes = rGF.ImportGraphic( rData, "", *pGrStream );

            // SJ: I40472, sometimes the aspect ratio (aMtfSize100) does not match and we get scaling problems,
            // then it is better to use the prefsize that is stored within the metafile. Bug #72846# for what the
            // scaling has been implemented does not happen anymore.
            //
            // For pict graphics we will furthermore scale the metafile, because font scaling leads to error if the
            // dxarray is empty (this has been solved in wmf/emf but not for pict)
            if( bMtfBLIP && ( GRFILTER_OK == nRes ) && ( rData.GetType() == GraphicType::GdiMetafile ) && ( ( nInst & 0xFFFE ) == 0x542 ) )
            {
                if ( ( aMtfSize100.Width() >= 1000 ) && ( aMtfSize100.Height() >= 1000 ) )
                {   // #75956#, scaling does not work properly, if the graphic is less than 1cm
                    GDIMetaFile aMtf( rData.GetGDIMetaFile() );
                    const Size  aOldSize( aMtf.GetPrefSize() );

                    if( aOldSize.Width() && ( aOldSize.Width() != aMtfSize100.Width() ) &&
                        aOldSize.Height() && ( aOldSize.Height() != aMtfSize100.Height() ) )
                    {
                        aMtf.Scale( (double) aMtfSize100.Width() / aOldSize.Width(),
                                    (double) aMtfSize100.Height() / aOldSize.Height() );
                        aMtf.SetPrefSize( aMtfSize100 );
                        aMtf.SetPrefMapMode( MapUnit::Map100thMM );
                        rData = aMtf;
                    }
                }
            }
        }
        // reset error status if necessary
        if ( ERRCODE_IO_PENDING == pGrStream->GetError() )
          pGrStream->ResetError();
    }
    rBLIPStream.Seek( nOldPos );    // restore old FilePos of the strem

    return ( GRFILTER_OK == nRes ); // return result
}

/* also static */
bool SvxMSDffManager::ReadCommonRecordHeader(SvStream& rSt,
    sal_uInt8& rVer, sal_uInt16& rInst, sal_uInt16& rFbt, sal_uInt32& rLength)
{
    sal_uInt16 nTmp(0);
    rSt.ReadUInt16( nTmp ).ReadUInt16( rFbt ).ReadUInt32( rLength );
    rVer = sal::static_int_cast< sal_uInt8 >(nTmp & 15);
    rInst = nTmp >> 4;
    if (!rSt.good())
        return false;
    if (rLength > nMaxLegalDffRecordLength)
        return false;
    return true;
}

bool SvxMSDffManager::ProcessClientAnchor(SvStream& rStData, sal_uInt32 nDatLen,
                                          char*& rpBuff, sal_uInt32& rBuffLen )
{
    if( nDatLen )
    {
        rBuffLen = std::min(rStData.remainingSize(), static_cast<sal_uInt64>(nDatLen));
        rpBuff = new char[rBuffLen];
        rBuffLen = rStData.ReadBytes(rpBuff, rBuffLen);
    }
    return true;
}

bool SvxMSDffManager::ProcessClientData(SvStream& rStData, sal_uInt32 nDatLen,
                                        char*& rpBuff, sal_uInt32& rBuffLen )
{
    if( nDatLen )
    {
        rBuffLen = std::min(rStData.remainingSize(), static_cast<sal_uInt64>(nDatLen));
        rpBuff = new char[rBuffLen];
        rBuffLen = rStData.ReadBytes(rpBuff, rBuffLen);
    }
    return true;
}


void SvxMSDffManager::ProcessClientAnchor2( SvStream& /* rSt */, DffRecordHeader& /* rHd */ , void* /* pData */, DffObjData& /* rObj */ )
{
    return;  // will be overridden by SJ in Draw
}

bool SvxMSDffManager::GetOLEStorageName( long /* nOLEId */, OUString&, tools::SvRef<SotStorage>&, uno::Reference < embed::XStorage >& ) const
{
    return false;
}

bool SvxMSDffManager::ShapeHasText( sal_uLong /* nShapeId */, sal_uLong /* nFilePos */ ) const
{
    return true;
}

// #i32596# - add new parameter <_nCalledByGroup>
SdrObject* SvxMSDffManager::ImportOLE( long nOLEId,
                                       const Graphic& rGrf,
                                       const Rectangle& rBoundRect,
                                       const Rectangle& rVisArea,
                                       const int /* _nCalledByGroup */,
                                       sal_Int64 nAspect ) const
{
    SdrObject* pRet = nullptr;
    OUString sStorageName;
    tools::SvRef<SotStorage> xSrcStg;
    ErrCode nError = ERRCODE_NONE;
    uno::Reference < embed::XStorage > xDstStg;
    if( GetOLEStorageName( nOLEId, sStorageName, xSrcStg, xDstStg ))
        pRet = CreateSdrOLEFromStorage( sStorageName, xSrcStg, xDstStg,
                                        rGrf, rBoundRect, rVisArea, pStData, nError,
                                        nSvxMSDffOLEConvFlags, nAspect, maBaseURL);
    return pRet;
}

bool SvxMSDffManager::MakeContentStream( SotStorage * pStor, const GDIMetaFile & rMtf )
{
    tools::SvRef<SotStorageStream> xStm = pStor->OpenSotStream(SVEXT_PERSIST_STREAM);
    xStm->SetVersion( pStor->GetVersion() );
    xStm->SetBufferSize( 8192 );

    sal_uInt16 nAspect = ASPECT_CONTENT;
    sal_uLong nAdviseModes = 2;

    Impl_OlePres aEle( SotClipboardFormatId::GDIMETAFILE );
    // Convert the size in 1/100 mm
    // If a not applicable MapUnit (device dependent) is used,
    // SV tries to guess a best match for the right value
    Size aSize = rMtf.GetPrefSize();
    const MapMode& aMMSrc = rMtf.GetPrefMapMode();
    MapMode aMMDst( MapUnit::Map100thMM );
    aSize = OutputDevice::LogicToLogic( aSize, aMMSrc, aMMDst );
    aEle.SetSize( aSize );
    aEle.SetAspect( nAspect );
    aEle.SetAdviseFlags( nAdviseModes );
    aEle.SetMtf( rMtf );
    aEle.Write( *xStm );

    xStm->SetBufferSize( 0 );
    return xStm->GetError() == SVSTREAM_OK;
}

struct ClsIDs {
    sal_uInt32      nId;
    const sal_Char* pSvrName;
    const sal_Char* pDspName;
};
static const ClsIDs aClsIDs[] = {

    { 0x000212F0, "MSWordArt",          "Microsoft Word Art"            },
    { 0x000212F0, "MSWordArt.2",        "Microsoft Word Art 2.0"        },

    // MS Apps
    { 0x00030000, "ExcelWorksheet",     "Microsoft Excel Worksheet"     },
    { 0x00030001, "ExcelChart",         "Microsoft Excel Chart"         },
    { 0x00030002, "ExcelMacrosheet",    "Microsoft Excel Macro"         },
    { 0x00030003, "WordDocument",       "Microsoft Word Document"       },
    { 0x00030004, "MSPowerPoint",       "Microsoft PowerPoint"          },
    { 0x00030005, "MSPowerPointSho",    "Microsoft PowerPoint Slide Show"},
    { 0x00030006, "MSGraph",            "Microsoft Graph"               },
    { 0x00030007, "MSDraw",             "Microsoft Draw"                },
    { 0x00030008, "Note-It",            "Microsoft Note-It"             },
    { 0x00030009, "WordArt",            "Microsoft Word Art"            },
    { 0x0003000a, "PBrush",             "Microsoft PaintBrush Picture"  },
    { 0x0003000b, "Equation",           "Microsoft Equation Editor"     },
    { 0x0003000c, "Package",            "Package"                       },
    { 0x0003000d, "SoundRec",           "Sound"                         },
    { 0x0003000e, "MPlayer",            "Media Player"                  },
    // MS Demos
    { 0x0003000f, "ServerDemo",         "OLE 1.0 Server Demo"           },
    { 0x00030010, "Srtest",             "OLE 1.0 Test Demo"             },
    { 0x00030011, "SrtInv",             "OLE 1.0 Inv Demo"              },
    { 0x00030012, "OleDemo",            "OLE 1.0 Demo"                  },

    // Coromandel / Dorai Swamy / 718-793-7963
    { 0x00030013, "CoromandelIntegra",  "Coromandel Integra"            },
    { 0x00030014, "CoromandelObjServer","Coromandel Object Server"      },

    // 3-d Visions Corp / Peter Hirsch / 310-325-1339
    { 0x00030015, "StanfordGraphics",   "Stanford Graphics"             },

    // Deltapoint / Nigel Hearne / 408-648-4000
    { 0x00030016, "DGraphCHART",        "DeltaPoint Graph Chart"        },
    { 0x00030017, "DGraphDATA",         "DeltaPoint Graph Data"         },

    // Corel / Richard V. Woodend / 613-728-8200 x1153
    { 0x00030018, "PhotoPaint",         "Corel PhotoPaint"              },
    { 0x00030019, "CShow",              "Corel Show"                    },
    { 0x0003001a, "CorelChart",         "Corel Chart"                   },
    { 0x0003001b, "CDraw",              "Corel Draw"                    },

    // Inset Systems / Mark Skiba / 203-740-2400
    { 0x0003001c, "HJWIN1.0",           "Inset Systems"                 },

    // Mark V Systems / Mark McGraw / 818-995-7671
    { 0x0003001d, "ObjMakerOLE",        "MarkV Systems Object Maker"    },

    // IdentiTech / Mike Gilger / 407-951-9503
    { 0x0003001e, "FYI",                "IdentiTech FYI"                },
    { 0x0003001f, "FYIView",            "IdentiTech FYI Viewer"         },

    // Inventa Corporation / Balaji Varadarajan / 408-987-0220
    { 0x00030020, "Stickynote",         "Inventa Sticky Note"           },

    // ShapeWare Corp. / Lori Pearce / 206-467-6723
    { 0x00030021, "ShapewareVISIO10",   "Shapeware Visio 1.0"           },
    { 0x00030022, "ImportServer",       "Spaheware Import Server"       },

    // test app SrTest
    { 0x00030023, "SrvrTest",           "OLE 1.0 Server Test"           },

    // test app ClTest.  Doesn't really work as a server but is in reg db
    { 0x00030025, "Cltest",             "OLE 1.0 Client Test"           },

    // Microsoft ClipArt Gallery   Sherry Larsen-Holmes
    { 0x00030026, "MS_ClipArt_Gallery", "Microsoft ClipArt Gallery"     },
    // Microsoft Project  Cory Reina
    { 0x00030027, "MSProject",          "Microsoft Project"             },

    // Microsoft Works Chart
    { 0x00030028, "MSWorksChart",       "Microsoft Works Chart"         },

    // Microsoft Works Spreadsheet
    { 0x00030029, "MSWorksSpreadsheet", "Microsoft Works Spreadsheet"   },

    // AFX apps - Dean McCrory
    { 0x0003002A, "MinSvr",             "AFX Mini Server"               },
    { 0x0003002B, "HierarchyList",      "AFX Hierarchy List"            },
    { 0x0003002C, "BibRef",             "AFX BibRef"                    },
    { 0x0003002D, "MinSvrMI",           "AFX Mini Server MI"            },
    { 0x0003002E, "TestServ",           "AFX Test Server"               },

    // Ami Pro
    { 0x0003002F, "AmiProDocument",     "Ami Pro Document"              },

    // WordPerfect Presentations For Windows
    { 0x00030030, "WPGraphics",         "WordPerfect Presentation"      },
    { 0x00030031, "WPCharts",           "WordPerfect Chart"             },

    // MicroGrafx Charisma
    { 0x00030032, "Charisma",           "MicroGrafx Charisma"           },
    { 0x00030033, "Charisma_30",        "MicroGrafx Charisma 3.0"       },
    { 0x00030034, "CharPres_30",        "MicroGrafx Charisma 3.0 Pres"  },
    // MicroGrafx Draw
    { 0x00030035, "Draw",               "MicroGrafx Draw"               },
    // MicroGrafx Designer
    { 0x00030036, "Designer_40",        "MicroGrafx Designer 4.0"       },

    // STAR DIVISION
    { 0x00043AD2, "FontWork",           "Star FontWork"                 },

    { 0, "", "" } };


bool SvxMSDffManager::ConvertToOle2( SvStream& rStm, sal_uInt32 nReadLen,
                    const GDIMetaFile * pMtf, const tools::SvRef<SotStorage>& rDest )
{
    bool bMtfRead = false;
    tools::SvRef<SotStorageStream> xOle10Stm = rDest->OpenSotStream( "\1Ole10Native",
                                                    StreamMode::WRITE| StreamMode::SHARE_DENYALL );
    if( xOle10Stm->GetError() )
        return false;

    sal_uInt32 nType;
    sal_uInt32 nRecType;
    sal_uInt32 nStrLen;
    OUString   aSvrName;
    sal_uInt32 nDummy0;
    sal_uInt32 nDummy1;
    sal_uInt32 nDataLen;
    sal_uInt32 nBytesRead = 0;
    do
    {
        rStm.ReadUInt32( nType );
        rStm.ReadUInt32( nRecType );
        rStm.ReadUInt32( nStrLen );
        if( nStrLen )
        {
            if( 0x10000L > nStrLen )
            {
                std::unique_ptr<sal_Char[]> pBuf(new sal_Char[ nStrLen ]);
                rStm.ReadBytes(pBuf.get(), nStrLen);
                aSvrName = OUString( pBuf.get(), (sal_uInt16) nStrLen-1, osl_getThreadTextEncoding() );
            }
            else
                break;
        }
        rStm.ReadUInt32( nDummy0 );
        rStm.ReadUInt32( nDummy1 );
        rStm.ReadUInt32( nDataLen );

        nBytesRead += 6 * sizeof( sal_uInt32 ) + nStrLen + nDataLen;

        if( !rStm.IsEof() && nReadLen > nBytesRead && nDataLen )
        {
            if( xOle10Stm.Is() )
            {
                std::unique_ptr<sal_uInt8[]> pData(new sal_uInt8[ nDataLen ]);
                if( !pData )
                    return false;

                rStm.ReadBytes(pData.get(), nDataLen);

                // write to ole10 stream
                xOle10Stm->WriteUInt32( nDataLen );
                xOle10Stm->WriteBytes(pData.get(), nDataLen);
                xOle10Stm = tools::SvRef<SotStorageStream>();

                // set the compobj stream
                const ClsIDs* pIds;
                for( pIds = aClsIDs; pIds->nId; pIds++ )
                {
                    if( aSvrName == OUString::createFromAscii(pIds->pSvrName) )
                        break;
                }

                if( pIds->nId )
                {
                    // found!
                    SotClipboardFormatId nCbFmt = SotExchange::RegisterFormatName( aSvrName );
                    rDest->SetClass( SvGlobalName( pIds->nId, 0, 0, 0xc0,0,0,0,0,0,0,0x46 ), nCbFmt,
                                    OUString::createFromAscii( pIds->pDspName ) );
                }
                else
                {
                    SotClipboardFormatId nCbFmt = SotExchange::RegisterFormatName( aSvrName );
                    rDest->SetClass( SvGlobalName(), nCbFmt, aSvrName );
                }
            }
            else if( nRecType == 5 && !pMtf )
            {
                sal_uLong nPos = rStm.Tell();
                sal_uInt16 sz[4];
                rStm.ReadBytes( sz, 8 );
                Graphic aGraphic;
                if( ERRCODE_NONE == GraphicConverter::Import( rStm, aGraphic ) && aGraphic.GetType() != GraphicType::NONE )
                {
                    const GDIMetaFile& rMtf = aGraphic.GetGDIMetaFile();
                    MakeContentStream( rDest.get(), rMtf );
                    bMtfRead = true;
                }
                // set behind the data
                rStm.Seek( nPos + nDataLen );
            }
            else
                rStm.SeekRel( nDataLen );
        }
    } while( !rStm.IsEof() && nReadLen >= nBytesRead );

    if( !bMtfRead && pMtf )
    {
        MakeContentStream( rDest.get(), *pMtf );
        return true;
    }

    return false;
}

const char* GetInternalServerName_Impl( const SvGlobalName& aGlobName )
{
    if ( aGlobName == SvGlobalName( SO3_SW_OLE_EMBED_CLASSID_60 )
      || aGlobName == SvGlobalName( SO3_SW_OLE_EMBED_CLASSID_8 ) )
        return "swriter";
    else if ( aGlobName == SvGlobalName( SO3_SC_OLE_EMBED_CLASSID_60 )
      || aGlobName == SvGlobalName( SO3_SC_OLE_EMBED_CLASSID_8 ) )
        return "scalc";
    else if ( aGlobName == SvGlobalName( SO3_SIMPRESS_OLE_EMBED_CLASSID_60 )
      || aGlobName == SvGlobalName( SO3_SIMPRESS_OLE_EMBED_CLASSID_8 ) )
        return "simpress";
    else if ( aGlobName == SvGlobalName( SO3_SDRAW_OLE_EMBED_CLASSID_60 )
      || aGlobName == SvGlobalName( SO3_SDRAW_OLE_EMBED_CLASSID_8 ) )
        return "sdraw";
    else if ( aGlobName == SvGlobalName( SO3_SM_OLE_EMBED_CLASSID_60 )
      || aGlobName == SvGlobalName( SO3_SM_OLE_EMBED_CLASSID_8 ) )
        return "smath";
    else if ( aGlobName == SvGlobalName( SO3_SCH_OLE_EMBED_CLASSID_60 )
      || aGlobName == SvGlobalName( SO3_SCH_OLE_EMBED_CLASSID_8 ) )
        return "schart";
    return nullptr;
}

OUString GetFilterNameFromClassID_Impl( const SvGlobalName& aGlobName )
{
    if ( aGlobName == SvGlobalName( SO3_SW_OLE_EMBED_CLASSID_60 ) )
        return OUString( "StarOffice XML (Writer)" );

    if ( aGlobName == SvGlobalName( SO3_SW_OLE_EMBED_CLASSID_8 ) )
        return OUString( "writer8" );

    if ( aGlobName == SvGlobalName( SO3_SC_OLE_EMBED_CLASSID_60 ) )
        return OUString( "StarOffice XML (Calc)" );

    if ( aGlobName == SvGlobalName( SO3_SC_OLE_EMBED_CLASSID_8 ) )
        return OUString( "calc8" );

    if ( aGlobName == SvGlobalName( SO3_SIMPRESS_OLE_EMBED_CLASSID_60 ) )
        return OUString( "StarOffice XML (Impress)" );

    if ( aGlobName == SvGlobalName( SO3_SIMPRESS_OLE_EMBED_CLASSID_8 ) )
        return OUString( "impress8" );

    if ( aGlobName == SvGlobalName( SO3_SDRAW_OLE_EMBED_CLASSID_60 ) )
        return OUString( "StarOffice XML (Draw)" );

    if ( aGlobName == SvGlobalName( SO3_SDRAW_OLE_EMBED_CLASSID_8 ) )
        return OUString( "draw8" );

    if ( aGlobName == SvGlobalName( SO3_SM_OLE_EMBED_CLASSID_60 ) )
        return OUString( "StarOffice XML (Math)" );

    if ( aGlobName == SvGlobalName( SO3_SM_OLE_EMBED_CLASSID_8 ) )
        return OUString( "math8" );

    if ( aGlobName == SvGlobalName( SO3_SCH_OLE_EMBED_CLASSID_60 ) )
        return OUString( "StarOffice XML (Chart)" );

    if ( aGlobName == SvGlobalName( SO3_SCH_OLE_EMBED_CLASSID_8 ) )
        return OUString( "chart8" );

    return OUString();
}

css::uno::Reference < css::embed::XEmbeddedObject >  SvxMSDffManager::CheckForConvertToSOObj( sal_uInt32 nConvertFlags,
                        SotStorage& rSrcStg, const uno::Reference < embed::XStorage >& rDestStorage,
                        const Graphic& rGrf,
                        const Rectangle& rVisArea, OUString const& rBaseURL)
{
    uno::Reference < embed::XEmbeddedObject > xObj;
    SvGlobalName aStgNm = rSrcStg.GetClassName();
    const char* pName = GetInternalServerName_Impl( aStgNm );
    OUString sStarName;
    if ( pName )
        sStarName = OUString::createFromAscii( pName );
    else if ( nConvertFlags )
    {
        static struct ObjImpType
        {
            sal_uInt32 nFlag;
            const char* pFactoryNm;
            // GlobalNameId
            sal_uInt32 n1;
            sal_uInt16 n2, n3;
            sal_uInt8 b8, b9, b10, b11, b12, b13, b14, b15;
        } aArr[] = {
            { OLE_MATHTYPE_2_STARMATH, "smath", MSO_EQUATION3_CLASSID },
            { OLE_MATHTYPE_2_STARMATH, "smath", MSO_EQUATION2_CLASSID },
            { OLE_WINWORD_2_STARWRITER, "swriter", MSO_WW8_CLASSID },
            // Excel table
            { OLE_EXCEL_2_STARCALC, "scalc", MSO_EXCEL5_CLASSID },
            { OLE_EXCEL_2_STARCALC, "scalc", MSO_EXCEL8_CLASSID },
            // 114465: additional Excel OLE chart classId to above.
            { OLE_EXCEL_2_STARCALC, "scalc", MSO_EXCEL8_CHART_CLASSID },
            // PowerPoint presentation
            { OLE_POWERPOINT_2_STARIMPRESS, "simpress", MSO_PPT8_CLASSID },
            // PowerPoint slide
            { OLE_POWERPOINT_2_STARIMPRESS, "simpress", MSO_PPT8_SLIDE_CLASSID },
            { 0, nullptr,
              0, 0, 0,
              0, 0, 0, 0, 0, 0, 0, 0 }
        };

        for( const ObjImpType* pArr = aArr; pArr->nFlag; ++pArr )
        {
            if( nConvertFlags & pArr->nFlag )
            {
                SvGlobalName aTypeName( pArr->n1, pArr->n2, pArr->n3,
                                pArr->b8, pArr->b9, pArr->b10, pArr->b11,
                                pArr->b12, pArr->b13, pArr->b14, pArr->b15 );

                if ( aStgNm == aTypeName )
                {
                    sStarName = OUString::createFromAscii( pArr->pFactoryNm );
                    break;
                }
            }
        }
    }

    if ( sStarName.getLength() )
    {
        //TODO/MBA: check if (and when) storage and stream will be destroyed!
        std::shared_ptr<const SfxFilter> pFilter;
        std::unique_ptr<SvMemoryStream> xMemStream (new SvMemoryStream);
        if ( pName )
        {
            // TODO/LATER: perhaps we need to retrieve VisArea and Metafile from the storage also
            tools::SvRef<SotStorageStream> xStr = rSrcStg.OpenSotStream( "package_stream", StreamMode::STD_READ );
            xStr->ReadStream( *xMemStream );
        }
        else
        {
            SfxFilterMatcher aMatch( sStarName );
            tools::SvRef<SotStorage> xStorage = new SotStorage( false, *xMemStream );
            rSrcStg.CopyTo( xStorage.get() );
            xStorage->Commit();
            xStorage.Clear();
            OUString aType = SfxFilter::GetTypeFromStorage( rSrcStg );
            if ( aType.getLength() )
                pFilter = aMatch.GetFilter4EA( aType );
        }

#ifdef DEBUG_FILTER_MSFILTER
        // extract embedded ole streams into "/tmp/embedded_stream_NNN"
        static sal_Int32 nOleCount(0);
        OUString aTmpName("/tmp/embedded_stream_");
        aTmpName += OUString::number(nOleCount++);
        aTmpName += ".bin";
        SvFileStream aTmpStream(aTmpName,StreamMode::READ|StreamMode::WRITE|StreamMode::TRUNC);
        xMemStream->Seek(0);
        *xMemStream >> aTmpStream;
        aTmpStream.Close();
#endif
        if ( pName || pFilter )
        {
            //Reuse current ole name
            OUString aDstStgName(MSO_OLE_Obj);
            aDstStgName += OUString::number(nMSOleObjCntr);

            OUString aFilterName;
            if ( pFilter )
                aFilterName = pFilter->GetName();
            else
                aFilterName = GetFilterNameFromClassID_Impl( aStgNm );

            uno::Sequence<beans::PropertyValue> aMedium(aFilterName.isEmpty() ? 3 : 4);
            aMedium[0].Name = "InputStream";
            uno::Reference < io::XInputStream > xStream = new ::utl::OSeekableInputStreamWrapper( *xMemStream );
            aMedium[0].Value <<= xStream;
            aMedium[1].Name = "URL";
            aMedium[1].Value <<= OUString( "private:stream" );
            aMedium[2].Name = "DocumentBaseURL";
            aMedium[2].Value <<= OUString(rBaseURL);

            if ( !aFilterName.isEmpty() )
            {
                aMedium[3].Name = "FilterName";
                aMedium[3].Value <<= aFilterName;
            }

            OUString aName( aDstStgName );
            comphelper::EmbeddedObjectContainer aCnt( rDestStorage );
            xObj = aCnt.InsertEmbeddedObject(aMedium, aName, &rBaseURL);

            if ( !xObj.is() )
            {
                if( !aFilterName.isEmpty() )
                {
                    // throw the filter parameter away as workaround
                    aMedium.realloc( 2 );
                    xObj = aCnt.InsertEmbeddedObject(aMedium, aName, &rBaseURL);
                }

                if ( !xObj.is() )
                     return xObj;
            }

            // TODO/LATER: ViewAspect must be passed from outside!
            sal_Int64 nViewAspect = embed::Aspects::MSOLE_CONTENT;

            // JP 26.10.2001: Bug 93374 / 91928 the writer
            // objects need the correct visarea needs the
            // correct visarea, but this is not true for
            // PowerPoint (see bugdoc 94908b)
            // SJ: 19.11.2001 bug 94908, also chart objects
            // needs the correct visarea

            // If pName is set this is an own embedded object, it should have the correct size internally
            // TODO/LATER: it might make sense in future to set the size stored in internal object
            if( !pName && ( sStarName == "swriter" || sStarName == "scalc" ) )
            {
                MapMode aMapMode( VCLUnoHelper::UnoEmbed2VCLMapUnit( xObj->getMapUnit( nViewAspect ) ) );
                Size aSz;
                if ( rVisArea.IsEmpty() )
                    aSz = lcl_GetPrefSize(rGrf, aMapMode );
                else
                {
                    aSz = rVisArea.GetSize();
                    aSz = OutputDevice::LogicToLogic( aSz, MapMode( MapUnit::Map100thMM ), aMapMode );
                }

                // don't modify the object
                //TODO/LATER: remove those hacks, that needs to be done differently!
                //xIPObj->EnableSetModified( sal_False );
                awt::Size aSize;
                aSize.Width = aSz.Width();
                aSize.Height = aSz.Height();
                xObj->setVisualAreaSize( nViewAspect, aSize );
                //xIPObj->EnableSetModified( sal_True );
            }
            else if ( sStarName == "smath" )
            {   // SJ: force the object to recalc its visarea
                //TODO/LATER: wait for PrinterChangeNotification
                //xIPObj->OnDocumentPrinterChanged( NULL );
            }
        }
    }

    return xObj;
}

// TODO/MBA: code review and testing!
SdrOle2Obj* SvxMSDffManager::CreateSdrOLEFromStorage(
                const OUString& rStorageName,
                tools::SvRef<SotStorage>& rSrcStorage,
                const uno::Reference < embed::XStorage >& xDestStorage,
                const Graphic& rGrf,
                const Rectangle& rBoundRect,
                const Rectangle& rVisArea,
                SvStream* pDataStrm,
                ErrCode& rError,
                sal_uInt32 nConvertFlags,
                sal_Int64 nRecommendedAspect,
                OUString const& rBaseURL)
{
    sal_Int64 nAspect = nRecommendedAspect;
    SdrOle2Obj* pRet = nullptr;
    if( rSrcStorage.Is() && xDestStorage.is() && rStorageName.getLength() )
    {
        comphelper::EmbeddedObjectContainer aCnt( xDestStorage );
        // does the 01Ole-Stream exist at all?
        // (that's not the case for e.g. Fontwork )
        // If that's not the case -> include it as graphic
        bool bValidStorage = false;
        OUString aDstStgName(MSO_OLE_Obj);

        aDstStgName += OUString::number( ++nMSOleObjCntr );

        {
            tools::SvRef<SotStorage> xObjStg = rSrcStorage->OpenSotStorage( rStorageName );
            if( xObjStg.Is()  )
            {
                {
                    sal_uInt8 aTestA[10];   // exist the \1CompObj-Stream ?
                    tools::SvRef<SotStorageStream> xSrcTst = xObjStg->OpenSotStream( "\1CompObj" );
                    bValidStorage = xSrcTst.Is() && sizeof( aTestA ) ==
                                    xSrcTst->ReadBytes(aTestA, sizeof(aTestA));
                    if( !bValidStorage )
                    {
                        // or the \1Ole-Stream ?
                        xSrcTst = xObjStg->OpenSotStream( "\1Ole" );
                        bValidStorage = xSrcTst.Is() && sizeof(aTestA) ==
                                    xSrcTst->ReadBytes(aTestA, sizeof(aTestA));
                    }
                }

                if( bValidStorage )
                {
                    if ( nAspect != embed::Aspects::MSOLE_ICON )
                    {
                        // check whether the object is iconified one
                        // usually this information is already known, the only exception
                        // is a kind of embedded objects in Word documents
                        // TODO/LATER: should the caller be notified if the aspect changes in future?

                        tools::SvRef<SotStorageStream> xObjInfoSrc = xObjStg->OpenSotStream(
                            "\3ObjInfo", StreamMode::STD_READ );
                        if ( xObjInfoSrc.Is() && !xObjInfoSrc->GetError() )
                        {
                            sal_uInt8 nByte = 0;
                            xObjInfoSrc->ReadUChar( nByte );
                            if ( ( nByte >> 4 ) & embed::Aspects::MSOLE_ICON )
                                nAspect = embed::Aspects::MSOLE_ICON;
                        }
                    }

                    uno::Reference < embed::XEmbeddedObject > xObj( CheckForConvertToSOObj(
                            nConvertFlags, *xObjStg, xDestStorage, rGrf,
                            rVisArea, rBaseURL));
                    if ( xObj.is() )
                    {
                        svt::EmbeddedObjectRef aObj( xObj, nAspect );

                        // TODO/LATER: need MediaType
                        aObj.SetGraphic( rGrf, OUString() );

                        // TODO/MBA: check setting of PersistName
                        pRet = new SdrOle2Obj( aObj, OUString(), rBoundRect);
                        // we have the Object, don't create another
                        bValidStorage = false;
                    }
                }
            }
        }

        if( bValidStorage )
        {
            // object is not an own object
            tools::SvRef<SotStorage> xObjStor = SotStorage::OpenOLEStorage( xDestStorage, aDstStgName, StreamMode::READWRITE );

            if ( xObjStor.Is() )
            {
                tools::SvRef<SotStorage> xSrcStor = rSrcStorage->OpenSotStorage( rStorageName, StreamMode::READ );
                xSrcStor->CopyTo( xObjStor.get() );

                if( !xObjStor->GetError() )
                    xObjStor->Commit();

                if( xObjStor->GetError() )
                {
                    rError = xObjStor->GetError();
                    bValidStorage = false;
                }
                else if( !xObjStor.Is() )
                    bValidStorage = false;
            }
        }
        else if( pDataStrm )
        {
            sal_uInt32 nLen, nDummy;
            pDataStrm->ReadUInt32( nLen ).ReadUInt32( nDummy );
            if( SVSTREAM_OK != pDataStrm->GetError() ||
                // Id in BugDoc - exist there other Ids?
                // The ConvertToOle2 - does not check for consistent
                0x30008 != nDummy )
                bValidStorage = false;
            else
            {
                // or is it an OLE-1 Stream in the DataStream?
                tools::SvRef<SotStorage> xObjStor = SotStorage::OpenOLEStorage( xDestStorage, aDstStgName );
                //TODO/MBA: remove metafile conversion from ConvertToOle2
                //when is this code used?!
                GDIMetaFile aMtf;
                bValidStorage = ConvertToOle2( *pDataStrm, nLen, &aMtf, xObjStor );
                xObjStor->Commit();
            }
        }

        if( bValidStorage )
        {
            uno::Reference < embed::XEmbeddedObject > xObj = aCnt.GetEmbeddedObject( aDstStgName );
            if( xObj.is() )
            {
                // the visual area must be retrieved from the metafile (object doesn't know it so far)

                if ( nAspect != embed::Aspects::MSOLE_ICON )
                {
                    // working with visual area can switch the object to running state
                    awt::Size aAwtSz;
                    try
                    {
                        // the provided visual area should be used, if there is any
                        if ( rVisArea.IsEmpty() )
                        {
                            MapUnit aMapUnit = VCLUnoHelper::UnoEmbed2VCLMapUnit( xObj->getMapUnit( nAspect ) );
                            Size aSz(lcl_GetPrefSize(rGrf, MapMode(aMapUnit)));
                            aAwtSz.Width = aSz.Width();
                            aAwtSz.Height = aSz.Height();
                        }
                        else
                        {
                            aAwtSz.Width = rVisArea.GetWidth();
                            aAwtSz.Height = rVisArea.GetHeight();
                        }
                        //xInplaceObj->EnableSetModified( sal_False );
                        xObj->setVisualAreaSize( nAspect, aAwtSz );
                        //xInplaceObj->EnableSetModified( sal_True );
                    }
                    catch( const uno::Exception& )
                    {
                        OSL_FAIL( "Could not set visual area of the object!\n" );
                    }
                }

                svt::EmbeddedObjectRef aObj( xObj, nAspect );

                // TODO/LATER: need MediaType
                aObj.SetGraphic( rGrf, OUString() );

                pRet = new SdrOle2Obj( aObj, aDstStgName, rBoundRect);
            }
        }
    }

    return pRet;
}

bool SvxMSDffManager::SetPropValue( const uno::Any& rAny, const uno::Reference< css::beans::XPropertySet > & rXPropSet,
            const OUString& rPropName, bool bTestPropertyAvailability )
{
    bool bRetValue = true;
    if ( bTestPropertyAvailability )
    {
        bRetValue = false;
        try
        {
            uno::Reference< beans::XPropertySetInfo >
                aXPropSetInfo( rXPropSet->getPropertySetInfo() );
            if ( aXPropSetInfo.is() )
                bRetValue = aXPropSetInfo->hasPropertyByName( rPropName );
        }
        catch( const uno::Exception& )
        {
            bRetValue = false;
        }
    }
    if ( bRetValue )
    {
        try
        {
            rXPropSet->setPropertyValue( rPropName, rAny );
            bRetValue = true;
        }
        catch( const uno::Exception& )
        {
            bRetValue = false;
        }
    }
    return bRetValue;
}

SvxMSDffImportRec::SvxMSDffImportRec()
    : pObj( nullptr ),
      pWrapPolygon(nullptr),
      pClientAnchorBuffer( nullptr ),
      nClientAnchorLen(  0 ),
      pClientDataBuffer( nullptr ),
      nClientDataLen(    0 ),
      nXAlign( 0 ), // position n cm from left
      pXRelTo( nullptr ), //   relative to column
      nYAlign( 0 ), // position n cm below
      pYRelTo( nullptr ), //   relative to paragraph
      nLayoutInTableCell( 0 ), // element is laid out in table cell
      nFlags( 0 ),
      nDxTextLeft( 144 ),
      nDyTextTop( 72 ),
      nDxTextRight( 144 ),
      nDyTextBottom( 72 ),
      nDxWrapDistLeft( 0 ),
      nDyWrapDistTop( 0 ),
      nDxWrapDistRight( 0 ),
      nDyWrapDistBottom(0 ),
      nCropFromTop( 0 ),
      nCropFromBottom( 0 ),
      nCropFromLeft( 0 ),
      nCropFromRight( 0 ),
      aTextId( 0, 0 ),
      nNextShapeId( 0 ),
      nShapeId( 0 ),
      eShapeType( mso_sptNil ),
      relativeHorizontalWidth( -1 ),
      isHorizontalRule( false )
{
      eLineStyle      = mso_lineSimple; // GPF-Bug #66227#
      eLineDashing    = mso_lineSolid;
      bDrawHell       = false;
      bHidden         = false;

      bReplaceByFly   = false;
      bVFlip          = false;
      bHFlip          = false;
      bAutoWidth      = false;
}

SvxMSDffImportRec::SvxMSDffImportRec(const SvxMSDffImportRec& rCopy)
    : pObj( rCopy.pObj ),
      nXAlign( rCopy.nXAlign ),
      pXRelTo( nullptr ),
      nYAlign( rCopy.nYAlign ),
      pYRelTo( nullptr ),
      nLayoutInTableCell( rCopy.nLayoutInTableCell ),
      nFlags( rCopy.nFlags ),
      nDxTextLeft( rCopy.nDxTextLeft    ),
      nDyTextTop( rCopy.nDyTextTop ),
      nDxTextRight( rCopy.nDxTextRight ),
      nDyTextBottom( rCopy.nDyTextBottom ),
      nDxWrapDistLeft( rCopy.nDxWrapDistLeft ),
      nDyWrapDistTop( rCopy.nDyWrapDistTop ),
      nDxWrapDistRight( rCopy.nDxWrapDistRight ),
      nDyWrapDistBottom(rCopy.nDyWrapDistBottom ),
      nCropFromTop( rCopy.nCropFromTop ),
      nCropFromBottom( rCopy.nCropFromBottom ),
      nCropFromLeft( rCopy.nCropFromLeft ),
      nCropFromRight( rCopy.nCropFromRight ),
      aTextId( rCopy.aTextId ),
      nNextShapeId( rCopy.nNextShapeId ),
      nShapeId( rCopy.nShapeId ),
      eShapeType( rCopy.eShapeType ),
      relativeHorizontalWidth( rCopy.relativeHorizontalWidth ),
      isHorizontalRule( rCopy.isHorizontalRule )
{
    if (rCopy.pXRelTo)
    {
       pXRelTo = new sal_uInt32;
       *pXRelTo = *(rCopy.pXRelTo);
    }
    if (rCopy.pYRelTo)
    {
       pYRelTo = new sal_uInt32;
       *pYRelTo = *(rCopy.pYRelTo);
    }
    eLineStyle       = rCopy.eLineStyle; // GPF-Bug #66227#
    eLineDashing     = rCopy.eLineDashing;
    bDrawHell        = rCopy.bDrawHell;
    bHidden          = rCopy.bHidden;
    bReplaceByFly    = rCopy.bReplaceByFly;
    bAutoWidth       = rCopy.bAutoWidth;
    bVFlip = rCopy.bVFlip;
    bHFlip = rCopy.bHFlip;
    nClientAnchorLen = rCopy.nClientAnchorLen;
    if( rCopy.nClientAnchorLen )
    {
        pClientAnchorBuffer = new char[ nClientAnchorLen ];
        memcpy( pClientAnchorBuffer,
                rCopy.pClientAnchorBuffer,
                nClientAnchorLen );
    }
    else
        pClientAnchorBuffer = nullptr;

    nClientDataLen = rCopy.nClientDataLen;
    if( rCopy.nClientDataLen )
    {
        pClientDataBuffer = new char[ nClientDataLen ];
        memcpy( pClientDataBuffer,
                rCopy.pClientDataBuffer,
                nClientDataLen );
    }
    else
        pClientDataBuffer = nullptr;

    if (rCopy.pWrapPolygon)
        pWrapPolygon = new tools::Polygon(*rCopy.pWrapPolygon);
    else
        pWrapPolygon = nullptr;
}

SvxMSDffImportRec::~SvxMSDffImportRec()
{
    delete[] pClientAnchorBuffer;
    delete[] pClientDataBuffer;
    delete pWrapPolygon;
    delete pXRelTo;
    delete pYRelTo;
}

void SvxMSDffManager::insertShapeId( sal_Int32 nShapeId, SdrObject* pShape )
{
    maShapeIdContainer[nShapeId] = pShape;
}

void SvxMSDffManager::removeShapeId( SdrObject* pShape )
{
    SvxMSDffShapeIdContainer::iterator aIter( maShapeIdContainer.begin() );
    const SvxMSDffShapeIdContainer::iterator aEnd( maShapeIdContainer.end() );
    while( aIter != aEnd )
    {
        if( (*aIter).second == pShape )
        {
            maShapeIdContainer.erase( aIter );
            break;
        }
        ++aIter;
    }
}

SdrObject* SvxMSDffManager::getShapeForId( sal_Int32 nShapeId )
{
    SvxMSDffShapeIdContainer::iterator aIter( maShapeIdContainer.find(nShapeId) );
    return aIter != maShapeIdContainer.end() ? (*aIter).second : nullptr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
