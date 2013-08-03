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
#include <tools/solar.h>
#include <rtl/math.hxx>

#include <comphelper/classids.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <unotools/streamwrap.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>
#include <comphelper/seqstream.hxx>
#include <comphelper/storagehelper.hxx>
#include <sot/exchange.hxx>
#include <sot/storinfo.hxx>
#include <vcl/cvtgrf.hxx>
#include <vcl/wmf.hxx>
#include "viscache.hxx"

// SvxItem-Mapping. Is needed to successfully include the SvxItem-Header
#include <editeng/eeitem.hxx>
#include <editeng/editdata.hxx>
#include <svl/urihelper.hxx>
#include <tools/stream.hxx>
#include <tools/debug.hxx>
#include <tools/zcodec.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <unotools/localfilehelper.hxx>
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
#include <tools/string.hxx>
#include <tools/urlobj.hxx>
#include <vcl/virdev.hxx>
#include <vcl/bmpacc.hxx>
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
#include <filter/msfilter/msdffimp.hxx> // externally visible header file
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
#include <boost/scoped_array.hpp>
#include <boost/scoped_ptr.hpp>

using namespace ::com::sun::star    ;
using namespace ::com::sun::star::drawing;
using namespace uno                 ;
using namespace beans               ;
using namespace drawing             ;
using namespace container           ;

// static counter for OLE-Objects
static sal_uInt32 nMSOleObjCntr = 0;
#define MSO_OLE_Obj "MSO_OLE_Obj"

/************************************************************************/
void Impl_OlePres::Write( SvStream & rStm )
{
    WriteClipboardFormat( rStm, FORMAT_GDIMETAFILE );
    rStm << (sal_Int32)(nJobLen +4);       // a TargetDevice that's always empty
    if( nJobLen )
        rStm.Write( pJob, nJobLen );
    rStm << (sal_uInt32)nAspect;
    rStm << (sal_Int32)-1;      //L-Index always -1
    rStm << (sal_Int32)nAdvFlags;
    rStm << (sal_Int32)0;       //Compression
    rStm << (sal_Int32)aSize.Width();
    rStm << (sal_Int32)aSize.Height();
    sal_uLong nPos = rStm.Tell();
    rStm << (sal_Int32)0;

    if( GetFormat() == FORMAT_GDIMETAFILE && pMtf )
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
        if( MAP_100TH_MM != nMU )
        {
            Size aPrefS( pMtf->GetPrefSize() );
            Size aS( aPrefS );
            aS = OutputDevice::LogicToLogic( aS, nMU, MAP_100TH_MM );

            pMtf->Scale( Fraction( aS.Width(), aPrefS.Width() ),
                         Fraction( aS.Height(), aPrefS.Height() ) );
            pMtf->SetPrefMapMode( MAP_100TH_MM );
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
    rStm << (sal_uInt32)(nEndPos - nPos - 4);
    rStm.Seek( nEndPos );
}

DffPropertyReader::DffPropertyReader( const SvxMSDffManager& rMan ) :
    rManager( rMan ),
    pDefaultPropSet( NULL ),
    mbRotateGranientFillWithAngle ( 0 )
{
    InitializePropSet( DFF_msofbtOPT );
}

void DffPropertyReader::SetDefaultPropSet( SvStream& rStCtrl, sal_uInt32 nOffsDgg ) const
{
    delete pDefaultPropSet;
    sal_uInt32 nMerk = rStCtrl.Tell();
    rStCtrl.Seek( nOffsDgg );
    DffRecordHeader aRecHd;
    rStCtrl >> aRecHd;
    if ( aRecHd.nRecType == DFF_msofbtDggContainer )
    {
        if ( rManager.SeekToRec( rStCtrl, DFF_msofbtOPT, aRecHd.GetRecEndFilePos() ) )
        {
            ( (DffPropertyReader*) this )->pDefaultPropSet = new DffPropSet;
            rStCtrl >> *pDefaultPropSet;
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
    rIn >> (DffPropertyReader&)*this;

    if ( IsProperty( DFF_Prop_hspMaster ) )
    {
        if ( rManager.SeekToShape( rIn, pClientData, GetPropertyValue( DFF_Prop_hspMaster ) ) )
        {
            DffRecordHeader aRecHd;
            rIn >> aRecHd;
            if ( rManager.SeekToRec( rIn, DFF_msofbtOPT, aRecHd.GetRecEndFilePos() ) )
            {
                rIn |= (DffPropertyReader&)*this;
            }
        }
    }

    ( (DffPropertyReader*) this )->mnFix16Angle = Fix16ToAngle( GetPropertyValue( DFF_Prop_Rotation, 0 ) );

#ifdef DBG_CUSTOMSHAPE

    OUString aURLStr;

    if( ::utl::LocalFileHelper::ConvertPhysicalNameToURL( OUString("d:\\ashape.dbg"), aURLStr ) )
    {
        SvStream* pOut = ::utl::UcbStreamHelper::CreateStream( aURLStr, STREAM_WRITE );

        if( pOut )
        {
            pOut->Seek( STREAM_SEEK_TO_END );

            if ( IsProperty( DFF_Prop_adjustValue ) || IsProperty( DFF_Prop_pVertices ) )
            {
                pOut->WriteLine( "" );
                OString aString("ShapeId: " + OString::number(nShapeId));
                pOut->WriteLine(aString);
            }
            for ( sal_uInt32 i = DFF_Prop_adjustValue; i <= DFF_Prop_adjust10Value; i++ )
            {
                if ( IsProperty( i ) )
                {
                    OString aString("Prop_adjustValue" + OString::number( ( i - DFF_Prop_adjustValue ) + 1 ) +
                                    ":" + OString::number(GetPropertyValue(i)) );
                    pOut->WriteLine(aString);
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
                            pOut->WriteLine( "" );
                            OStringBuffer aDesc("Property:" + OString::number(i) +
                                                "  Size:" + OString::number(nLen));
                            pOut->WriteLine(aDesc.makeStringAndClear());
                            sal_Int16   nNumElem, nNumElemMem, nNumSize;
                            rIn >> nNumElem >> nNumElemMem >> nNumSize;
                            aDesc.append("Entries: " + OString::number(nNumElem) +
                                         "  Size:" + OString::number(nNumSize));
                            pOut->WriteLine(aDesc.makeStringAndClear());
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
                                                *pOut << (sal_uInt8)( ( nVal >> 4 ) + 'A' - 10 );
                                            else
                                                *pOut << (sal_uInt8)( ( nVal >> 4 ) + '0' );

                                            if ( ( nVal & 0xf ) > 9 )
                                                *pOut << (sal_uInt8)( ( nVal & 0xf ) + 'A' - 10 );
                                            else
                                                *pOut << (sal_uInt8)( ( nVal & 0xf ) + '0' );

                                            nLen--;
                                        }
                                    }
                                    *pOut << (char)( ' ' );
                                }
                                pOut->WriteLine( OString() );
                            }
                        }
                    }
                    else
                    {
                        OString aString("Property" + OString::number(i) +
                                        ":" + OString::number(GetPropertyValue(i)));
                        pOut->WriteLine(aString);
                    }
                }
            }

            delete pOut;
        }
    }

#endif

    rIn.Seek( nFilePos );
}


sal_Int32 DffPropertyReader::Fix16ToAngle( sal_Int32 nContent ) const
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
    delete pDefaultPropSet;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

SvStream& operator>>( SvStream& rIn, SvxMSDffConnectorRule& rRule )
{
    rIn >> rRule.nRuleId
        >> rRule.nShapeA
        >> rRule.nShapeB
        >> rRule.nShapeC
        >> rRule.ncptiA
        >> rRule.ncptiB;

    return rIn;
}

SvxMSDffSolverContainer::SvxMSDffSolverContainer()
{
}

SvxMSDffSolverContainer::~SvxMSDffSolverContainer()
{
    for( size_t i = 0, n = aCList.size(); i < n; ++i ) {
        delete aCList[ i ];
    }
    aCList.clear();
}

SvStream& operator>>( SvStream& rIn, SvxMSDffSolverContainer& rContainer )
{
    DffRecordHeader aHd;
    rIn >> aHd;
    if ( aHd.nRecType == DFF_msofbtSolverContainer )
    {
        DffRecordHeader aCRule;
        while ( ( rIn.GetError() == 0 ) && ( rIn.Tell() < aHd.GetRecEndFilePos() ) )
        {
            rIn >> aCRule;
            if ( aCRule.nRecType == DFF_msofbtConnectorRule )
            {
                SvxMSDffConnectorRule* pRule = new SvxMSDffConnectorRule;
                rIn >> *pRule;
                rContainer.aCList.push_back( pRule );
            }
            aCRule.SeekToEndOfRecord( rIn );
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
                    Any aAny;
                    SdrGluePoint aGluePoint;
                    Reference< XShape > aXShape( pO->getUnoShape(), UNO_QUERY );
                    Reference< XShape > aXConnector( pPtr->pCObj->getUnoShape(), UNO_QUERY );
                    SdrGluePointList* pList = pO->ForceGluePointList();

                    sal_Bool bValidGluePoint = sal_False;
                    sal_Int32 nId = nC;
                    sal_uInt32 nInventor = pO->GetObjInventor();

                    if( nInventor == SdrInventor )
                    {
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
                                        nId = 0;    // SDRVERTALIGN_TOP;
                                    break;
                                    case 1 :
                                        nId = 3;    // SDRHORZALIGN_RIGHT;
                                    break;
                                    case 2 :
                                        nId = 2;    // SDRVERTALIGN_BOTTOM;
                                    break;
                                    case 3 :
                                        nId = 1; // SDRHORZALIGN_LEFT;
                                    break;
                                }
                                if ( nId <= 3 )
                                    bValidGluePoint = sal_True;
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
                                if ( pList && ( pList->GetCount() > nC ) )
                                {
                                    bValidGluePoint = sal_True;
                                    nId = (sal_Int32)((*pList)[ (sal_uInt16)nC].GetId() + 3 );
                                }
                                else
                                {
                                    sal_Bool bNotFound = sal_True;

                                    PolyPolygon aPolyPoly( EscherPropertyContainer::GetPolyPolygon( aXShape ) );
                                    sal_uInt16 k, j, nPolySize = aPolyPoly.Count();
                                    if ( nPolySize )
                                    {
                                        Rectangle aBoundRect( aPolyPoly.GetBoundRect() );
                                        if ( aBoundRect.GetWidth() && aBoundRect.GetHeight() )
                                        {
                                            sal_uInt32  nPointCount = 0;
                                            for ( k = 0; bNotFound && ( k < nPolySize ); k++ )
                                            {
                                                const Polygon& rPolygon = aPolyPoly.GetObject( k );
                                                for ( j = 0; bNotFound && ( j < rPolygon.GetSize() ); j++ )
                                                {
                                                    PolyFlags eFlags = rPolygon.GetFlags( j );
                                                    if ( eFlags == POLY_NORMAL )
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
                                                            aGluePoint.SetPercent( sal_True );
                                                            aGluePoint.SetAlign( SDRVERTALIGN_TOP | SDRHORZALIGN_LEFT );
                                                            aGluePoint.SetEscDir( SDRESC_SMART );
                                                            nId = (sal_Int32)((*pList)[ pList->Insert( aGluePoint ) ].GetId() + 3 );
                                                            bNotFound = sal_False;
                                                        }
                                                        nPointCount++;
                                                    }
                                                }
                                            }
                                        }
                                    }
                                    if ( !bNotFound )
                                    {
                                        bValidGluePoint = sal_True;
                                    }
                                }
                            }
                            break;

                            case OBJ_CUSTOMSHAPE :
                            {
                                SdrCustomShapeGeometryItem aGeometryItem( (SdrCustomShapeGeometryItem&)((SdrObjCustomShape*)pO)->GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY ) );
                                const OUString sPath( "Path" );
                                const OUString sGluePointType( "GluePointType" );
                                sal_Int16 nGluePointType = EnhancedCustomShapeGluePointType::SEGMENTS;
                                com::sun::star::uno::Any* pAny = aGeometryItem.GetPropertyValueByName( sPath, sGluePointType );
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
                                        bValidGluePoint = sal_True;
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
                                            nId = 0;    // SDRVERTALIGN_TOP;
                                        break;
                                        case 1 :
                                            nId = 3;    // SDRHORZALIGN_RIGHT;
                                        break;
                                        case 2 :
                                            nId = 2;    // SDRVERTALIGN_BOTTOM;
                                        break;
                                        case 3 :
                                            nId = 1; // SDRHORZALIGN_LEFT;
                                        break;
                                    }
                                    if ( nId <= 3 )
                                        bValidGluePoint = sal_True;
                                }
                                else if ( nGluePointType == EnhancedCustomShapeGluePointType::SEGMENTS )
                                {
                                    const OUString sSegments( "Segments" );
                                    const OUString sCoordinates( "Coordinates" );

                                    sal_uInt32 k, nPt = nC;
                                    com::sun::star::uno::Sequence< com::sun::star::drawing::EnhancedCustomShapeSegment > aSegments;
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
                                        com::sun::star::uno::Sequence< com::sun::star::drawing::EnhancedCustomShapeParameterPair > aCoordinates;
                                        *pAny >>= aCoordinates;
                                        if ( nPt < (sal_uInt32)aCoordinates.getLength() )
                                        {
                                            nId = 4;
                                            com::sun::star::drawing::EnhancedCustomShapeParameterPair& rPara = aCoordinates[ nPt ];
                                            sal_Int32 nX = 0, nY = 0;
                                            if ( ( rPara.First.Value >>= nX ) && ( rPara.Second.Value >>= nY ) )
                                            {
                                                const OUString sGluePoints( "GluePoints" );
                                                com::sun::star::uno::Sequence< com::sun::star::drawing::EnhancedCustomShapeParameterPair > aGluePoints;
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
                                                bValidGluePoint = sal_True;
                                                ((SdrObjCustomShape*)pO)->SetMergedItem( aGeometryItem );
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
                                    aAny <<= aXShape;
                                    SetPropValue( aAny, xPropSet, aPropName, sal_True );
                                    aPropName = "EndGluePointIndex";
                                    aAny <<= nId;
                                    SetPropValue( aAny, xPropSet, aPropName, sal_True );
                                }
                                else
                                {
                                    OUString aPropName( "StartShape" );
                                    aAny <<= aXShape;
                                    SetPropValue( aAny, xPropSet, aPropName, sal_True );
                                    aPropName = "StartGluePointIndex";
                                    aAny <<= nId;
                                    SetPropValue( aAny, xPropSet, aPropName, sal_True );
                                }

                                // Not sure what this is good for, repaint or broadcast of object change.
                                //( Thus i am adding repaint here
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

////////////////////////////////////////////////////////////////////////////////////////////////////

static basegfx::B2DPolyPolygon GetLineArrow( const sal_Int32 nLineWidth, const MSO_LineEnd eLineEnd,
    const MSO_LineEndWidth eLineWidth, const MSO_LineEndLength eLineLenght,
    sal_Int32& rnArrowWidth, sal_Bool& rbArrowCenter,
    OUString& rsArrowName, sal_Bool bScaleArrow )
{
    basegfx::B2DPolyPolygon aRetPolyPoly;
    // 70 100mm = 2pt = 40 twip. In MS, line width less than 2pt has the same size arrow as 2pt
    //If the unit is twip. Make all use this unit especailly the critical value 70/40.
    sal_Int32   nLineWidthCritical = bScaleArrow ? 40 : 70;
    double      fLineWidth = nLineWidth < nLineWidthCritical ? nLineWidthCritical : nLineWidth;;

    double      fLenghtMul, fWidthMul;
    sal_Int32   nLineNumber;
    switch( eLineLenght )
    {
        default :
        case mso_lineMediumLenArrow     : fLenghtMul = 3.0; nLineNumber = 2; break;
        case mso_lineShortArrow         : fLenghtMul = 2.0; nLineNumber = 1; break;
        case mso_lineLongArrow          : fLenghtMul = 5.0; nLineNumber = 3; break;
    }
    switch( eLineWidth )
    {
        default :
        case mso_lineMediumWidthArrow   : fWidthMul = 3.0; nLineNumber += 3; break;
        case mso_lineNarrowArrow        : fWidthMul = 2.0; break;
        case mso_lineWideArrow          : fWidthMul = 5.0; nLineNumber += 6; break;
    }

    rbArrowCenter = sal_False;
    OUStringBuffer aArrowName;
    switch ( eLineEnd )
    {
        case mso_lineArrowEnd :
        {
            basegfx::B2DPolygon aTriangle;
            aTriangle.append(basegfx::B2DPoint( fWidthMul * fLineWidth * 0.50, 0.0 ));
            aTriangle.append(basegfx::B2DPoint( fWidthMul * fLineWidth, fLenghtMul * fLineWidth ));
            aTriangle.append(basegfx::B2DPoint( 0.0, fLenghtMul * fLineWidth ));
            aTriangle.setClosed(true);
            aRetPolyPoly = basegfx::B2DPolyPolygon(aTriangle);
            aArrowName.append("msArrowEnd ");
        }
        break;

        case mso_lineArrowOpenEnd :
        {
            switch( eLineLenght )
            {
                default :
                case mso_lineMediumLenArrow     : fLenghtMul = 4.5; break;
                case mso_lineShortArrow         : fLenghtMul = 3.5; break;
                case mso_lineLongArrow          : fLenghtMul = 6.0; break;
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
            aTriangle.append(basegfx::B2DPoint( fWidthMul * fLineWidth, fLenghtMul * fLineWidth * 0.91 ));
            aTriangle.append(basegfx::B2DPoint( fWidthMul * fLineWidth * 0.85, fLenghtMul * fLineWidth ));
            aTriangle.append(basegfx::B2DPoint( fWidthMul * fLineWidth * 0.50, fLenghtMul * fLineWidth * 0.36 ));
            aTriangle.append(basegfx::B2DPoint( fWidthMul * fLineWidth * 0.15, fLenghtMul * fLineWidth ));
            aTriangle.append(basegfx::B2DPoint( 0.0, fLenghtMul * fLineWidth * 0.91 ));
            aTriangle.setClosed(true);
            aRetPolyPoly = basegfx::B2DPolyPolygon(aTriangle);
            aArrowName.append("msArrowOpenEnd ");
        }
        break;
        case mso_lineArrowStealthEnd :
        {
            basegfx::B2DPolygon aTriangle;
            aTriangle.append(basegfx::B2DPoint( fWidthMul * fLineWidth * 0.50 , 0.0 ));
            aTriangle.append(basegfx::B2DPoint( fWidthMul * fLineWidth , fLenghtMul * fLineWidth ));
            aTriangle.append(basegfx::B2DPoint( fWidthMul * fLineWidth * 0.50 , fLenghtMul * fLineWidth * 0.60 ));
            aTriangle.append(basegfx::B2DPoint( 0.0, fLenghtMul * fLineWidth ));
            aTriangle.setClosed(true);
            aRetPolyPoly = basegfx::B2DPolyPolygon(aTriangle);
            aArrowName.append("msArrowStealthEnd ");
        }
        break;
        case mso_lineArrowDiamondEnd :
        {
            basegfx::B2DPolygon aTriangle;
            aTriangle.append(basegfx::B2DPoint( fWidthMul * fLineWidth * 0.50 , 0.0 ));
            aTriangle.append(basegfx::B2DPoint( fWidthMul * fLineWidth , fLenghtMul * fLineWidth * 0.50 ));
            aTriangle.append(basegfx::B2DPoint( fWidthMul * fLineWidth * 0.50 , fLenghtMul * fLineWidth ));
            aTriangle.append(basegfx::B2DPoint( 0.0, fLenghtMul * fLineWidth * 0.50 ));
            aTriangle.setClosed(true);
            aRetPolyPoly = basegfx::B2DPolyPolygon(aTriangle);
            rbArrowCenter = sal_True;
            aArrowName.append("msArrowDiamondEnd ");
        }
        break;
        case mso_lineArrowOvalEnd :
        {
            aRetPolyPoly = basegfx::B2DPolyPolygon( XPolygon( Point( (sal_Int32)( fWidthMul * fLineWidth * 0.50 ), 0 ),
                                (sal_Int32)( fWidthMul * fLineWidth * 0.50 ),
                                    (sal_Int32)( fLenghtMul * fLineWidth * 0.50 ), 0, 3600 ).getB2DPolygon() );
            rbArrowCenter = sal_True;
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
    sal_uInt32 nLineFlags(GetPropertyValue( DFF_Prop_fNoLineDrawDash ));

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
                // rSet.Put(XLineCapItem(com::sun::star::drawing::LineCap_BUTT));
                break;
            }
            case mso_lineEndCapRound:
            {
                rSet.Put(XLineCapItem(com::sun::star::drawing::LineCap_ROUND));
                break;
            }
            case mso_lineEndCapSquare:
            {
                rSet.Put(XLineCapItem(com::sun::star::drawing::LineCap_SQUARE));
                break;
            }
        }

        MSO_LineDashing eLineDashing = (MSO_LineDashing)GetPropertyValue( DFF_Prop_lineDashing, mso_lineSolid );
        if ( eLineDashing == mso_lineSolid )
            rSet.Put(XLineStyleItem( XLINE_SOLID ) );
        else
        {
            XDashStyle  eDash = XDASH_RECT;
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
            rSet.Put( XLineStyleItem( XLINE_DASH ) );
        }
        rSet.Put( XLineColorItem( OUString(), rManager.MSO_CLR_ToColor( GetPropertyValue( DFF_Prop_lineColor ), DFF_Prop_lineColor ) ) );
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
        com::sun::star::drawing::LineJoint eXLineJoint( com::sun::star::drawing::LineJoint_MITER );
        if ( eLineJoint == mso_lineJoinBevel )
            eXLineJoint = com::sun::star::drawing::LineJoint_BEVEL;
        else if ( eLineJoint == mso_lineJoinRound )
            eXLineJoint = com::sun::star::drawing::LineJoint_ROUND;
        rSet.Put( XLineJointItem( eXLineJoint ) );

        if ( nLineFlags & 0x10 )
        {
            sal_Bool bScaleArrows = rManager.pSdrModel->GetScaleUnit() == MAP_TWIP;
            ///////////////
            // LineStart //
            ///////////////
            if ( IsProperty( DFF_Prop_lineStartArrowhead ) )
            {
                MSO_LineEnd         eLineEnd = (MSO_LineEnd)GetPropertyValue( DFF_Prop_lineStartArrowhead );
                MSO_LineEndWidth    eWidth = (MSO_LineEndWidth)GetPropertyValue( DFF_Prop_lineStartArrowWidth, mso_lineMediumWidthArrow );
                MSO_LineEndLength   eLenght = (MSO_LineEndLength)GetPropertyValue( DFF_Prop_lineStartArrowLength, mso_lineMediumLenArrow );

                sal_Int32   nArrowWidth;
                sal_Bool    bArrowCenter;
                OUString    aArrowName;
                basegfx::B2DPolyPolygon aPolyPoly(GetLineArrow( nLineWidth, eLineEnd, eWidth, eLenght, nArrowWidth, bArrowCenter, aArrowName, bScaleArrows ));

                rSet.Put( XLineStartWidthItem( nArrowWidth ) );
                rSet.Put( XLineStartItem( aArrowName, aPolyPoly) );
                rSet.Put( XLineStartCenterItem( bArrowCenter ) );
            }
            /////////////
            // LineEnd //
            /////////////
            if ( IsProperty( DFF_Prop_lineEndArrowhead ) )
            {
                MSO_LineEnd         eLineEnd = (MSO_LineEnd)GetPropertyValue( DFF_Prop_lineEndArrowhead );
                MSO_LineEndWidth    eWidth = (MSO_LineEndWidth)GetPropertyValue( DFF_Prop_lineEndArrowWidth, mso_lineMediumWidthArrow );
                MSO_LineEndLength   eLenght = (MSO_LineEndLength)GetPropertyValue( DFF_Prop_lineEndArrowLength, mso_lineMediumLenArrow );

                sal_Int32   nArrowWidth;
                sal_Bool    bArrowCenter;
                OUString    aArrowName;
                basegfx::B2DPolyPolygon aPolyPoly(GetLineArrow( nLineWidth, eLineEnd, eWidth, eLenght, nArrowWidth, bArrowCenter, aArrowName, bScaleArrows ));

                rSet.Put( XLineEndWidthItem( nArrowWidth ) );
                rSet.Put( XLineEndItem( aArrowName, aPolyPoly ) );
                rSet.Put( XLineEndCenterItem( bArrowCenter ) );
            }
        }
    }
    else
        rSet.Put( XLineStyleItem( XLINE_NONE ) );
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
        if ( rProperties.SeekToContent( DFF_Prop_fillShadeColors, rIn ) )
        {
            sal_uInt16 i = 0, nNumElem = 0, nNumElemReserved = 0, nSize = 0;
            rIn >> nNumElem >> nNumElemReserved >> nSize;
            for ( ; i < nNumElem; i++ )
            {
                sal_Int32   nColor;
                sal_Int32   nDist;

                rIn >> nColor >> nDist;
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

struct QuantErr
{
    double  fRed;
    double  fGreen;
    double  fBlue;

    QuantErr() : fRed( 0.0 ), fGreen( 0.0 ), fBlue( 0.0 ){};
};

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
                                fDist = fY, fD = fFocusY;
                            else
                                fDist = fX, fD = fFocusX;
                        }
                        else
                        {
                            if ( fX > ( 1 - fY ) )
                                fDist = ( 1 - fY ), fD = 1 - fFocusY;
                            else
                                fDist = fX, fD = fFocusX;
                        }
                    }
                    else
                    {
                        if ( fY < fFocusY )
                        {
                            if ( ( 1 - fX ) > fY )
                                fDist = fY, fD = fFocusY;
                            else
                                fDist = ( 1 - fX ), fD = 1 - fFocusX;
                        }
                        else
                        {
                            if ( ( 1 - fX ) > ( 1 - fY ) )
                                fDist = ( 1 - fY ), fD = 1 - fFocusY;
                            else
                                fDist = ( 1 - fX ), fD = 1 - fFocusX;
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
            aBitmap.ReleaseAccess( pAcc );

            if ( nFix16Angle )
            {
                sal_Bool bRotateWithShape = sal_True;   // sal_True seems to be default
                sal_uInt32 nPos = rIn.Tell();
                if ( const_cast< SvxMSDffManager& >( rManager ).maShapeRecords.SeekToContent( rIn, DFF_msofbtUDefProp, SEEK_FROM_CURRENT_AND_RESTART ) )
                {
                    const_cast< SvxMSDffManager& >( rManager ).maShapeRecords.Current()->SeekToBegOfRecord( rIn );
                    DffPropertyReader aSecPropSet( rManager );
                    aSecPropSet.ReadPropSet( rIn, NULL );
                    sal_Int32 nSecFillProperties = aSecPropSet.GetPropertyValue( DFF_Prop_fNoFillHitTest, 0x200020 );
                    bRotateWithShape = ( nSecFillProperties & 0x0020 );
                }
                rIn.Seek( nPos );
                if ( bRotateWithShape )
                {
                    aBitmap.Rotate( nFix16Angle / 10, rShadeColors[ 0 ].aColor );

                    sal_uLong nMirrorFlags = BMP_MIRROR_NONE;
                    if ( rObjData.nSpFlags & SP_FFLIPV )
                        nMirrorFlags |= BMP_MIRROR_VERT;
                    if ( rObjData.nSpFlags & SP_FFLIPH )
                        nMirrorFlags |= BMP_MIRROR_HORZ;
                    if ( nMirrorFlags != BMP_MIRROR_NONE )
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
    sal_uInt32 nFillFlags(GetPropertyValue( DFF_Prop_fNoFillHitTest ));

    std::vector< ShadeColor > aShadeColors;
    GetShadeColors( rManager, *this, rIn, aShadeColors );

    if(!IsHardAttribute( DFF_Prop_fFilled ) && !IsCustomShapeFilledByDefault( rObjData.eShapeType ))
    {
        nFillFlags &= ~0x10;
    }

    if ( nFillFlags & 0x10 )
    {
        MSO_FillType eMSO_FillType = (MSO_FillType)GetPropertyValue( DFF_Prop_fillType, mso_fillSolid );
        XFillStyle eXFill = XFILL_NONE;
        switch( eMSO_FillType )
        {
            case mso_fillSolid :            // Fill with a solid color
                eXFill = XFILL_SOLID;
            break;
            case mso_fillPattern :          // Fill with a pattern (bitmap)
            case mso_fillTexture :          // A texture (pattern with its own color map)
            case mso_fillPicture :          // Center a picture in the shape
                eXFill = XFILL_BITMAP;
            break;
            case mso_fillShadeCenter :      // Shade from bounding rectangle to end point
            {
                //If it is imported as a bitmap, it will not work well with transparecy especially 100
                //But the gradient look well comparing with imported as gradient. And rotate with shape
                //also works better. So here just keep it.
                if ( rObjData.aBoundRect.IsEmpty() )// size of object needed to be able
                    eXFill = XFILL_GRADIENT;        // to create a bitmap substitution
                else
                    eXFill = XFILL_BITMAP;
            }
            break;
            case mso_fillShade :            // Shade from start to end points
            case mso_fillShadeShape :       // Shade from shape outline to end point
            case mso_fillShadeScale :       // Similar to mso_fillShade, but the fillAngle
            case mso_fillShadeTitle :       // special type - shade to title ---  for PP
                eXFill = XFILL_GRADIENT;
            break;
//          case mso_fillBackground :       // Use the background fill color/pattern
            default: break;
        }
        rSet.Put( XFillStyleItem( eXFill ) );

        double dTrans  = 1.0;
        double dBackTrans = 1.0;
        if (IsProperty(DFF_Prop_fillOpacity))
        {
            dTrans = GetPropertyValue(DFF_Prop_fillOpacity) / 65536.0;
            if ( eXFill != XFILL_GRADIENT )
            {
                dTrans = dTrans * 100;
                rSet.Put(XFillTransparenceItem(
                    sal_uInt16(100 - ::rtl::math::round(dTrans))));
            }
        }

        if ( IsProperty(DFF_Prop_fillBackOpacity) )
            dBackTrans = GetPropertyValue(DFF_Prop_fillBackOpacity) / 65536.0;

        if ( ( eMSO_FillType == mso_fillShadeCenter ) && ( eXFill == XFILL_BITMAP ) )
        {
            ApplyRectangularGradientAsBitmap( rManager, rIn, rSet, aShadeColors, rObjData, mnFix16Angle );
        }
        else if ( eXFill == XFILL_GRADIENT )
        {
            ImportGradientColor ( rSet, eMSO_FillType, dTrans , dBackTrans );
        }
        else if ( eXFill == XFILL_BITMAP )
        {
            if( IsProperty( DFF_Prop_fillBlip ) )
            {
                Graphic aGraf;
                // first try to get BLIP from cache
                sal_Bool bOK = const_cast<SvxMSDffManager&>(rManager).GetBLIP( GetPropertyValue( DFF_Prop_fillBlip ), aGraf, NULL );
                // then try directly from stream (i.e. Excel chart hatches/bitmaps)
                if ( !bOK )
                    bOK = SeekToContent( DFF_Prop_fillBlip, rIn ) && rManager.GetBLIPDirect( rIn, aGraf, NULL );
                if ( bOK )
                {
                    if ( eMSO_FillType == mso_fillPattern )
                    {
                        Bitmap aBmp( aGraf.GetBitmap() );
                        if( aBmp.GetSizePixel().Width() == 8 && aBmp.GetSizePixel().Height() == 8 && aBmp.GetColorCount() == 2)
                        {
                            Color aCol1( COL_WHITE ), aCol2( COL_WHITE );

                            if ( IsProperty( DFF_Prop_fillColor ) )
                                aCol1 = rManager.MSO_CLR_ToColor( GetPropertyValue( DFF_Prop_fillColor ), DFF_Prop_fillColor );

                            if ( IsProperty( DFF_Prop_fillBackColor ) )
                                aCol2 = rManager.MSO_CLR_ToColor( GetPropertyValue( DFF_Prop_fillBackColor ), DFF_Prop_fillBackColor );

                            XOBitmap aXOBitmap( aBmp );
                            aXOBitmap.Bitmap2Array();
                            aXOBitmap.SetBitmapType( XBITMAP_8X8 );
                            aXOBitmap.SetPixelSize( aBmp.GetSizePixel() );

                            if( aXOBitmap.GetBackgroundColor() == COL_BLACK )
                            {
                                aXOBitmap.SetPixelColor( aCol1 );
                                aXOBitmap.SetBackgroundColor( aCol2 );
                            }
                            else
                            {
                                aXOBitmap.SetPixelColor( aCol2 );
                                aXOBitmap.SetBackgroundColor( aCol1 );
                            }

                            aXOBitmap.Array2Bitmap();
                            aGraf = Graphic( aXOBitmap.GetBitmap()  );
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
        rSet.Put( XFillStyleItem( XFILL_NONE ) );
}

void DffPropertyReader::ApplyCustomShapeTextAttributes( SfxItemSet& rSet ) const
{
    sal_Bool  bVerticalText = sal_False;
    sal_Int32 nTextLeft = GetPropertyValue( DFF_Prop_dxTextLeft, 25 * 3600 ) / 360;     // 0.25 cm (emu)
    sal_Int32 nTextRight = GetPropertyValue( DFF_Prop_dxTextRight, 25 * 3600 ) / 360;   // 0.25 cm (emu)
    sal_Int32 nTextTop = GetPropertyValue( DFF_Prop_dyTextTop, 13 * 3600 ) / 360;       // 0.13 cm (emu)
    sal_Int32 nTextBottom = GetPropertyValue( DFF_Prop_dyTextBottom, 13 * 3600 ) /360;  // 0.13 cm (emu)

    SdrTextVertAdjust eTVA;
    SdrTextHorzAdjust eTHA;

    if ( IsProperty( DFF_Prop_txflTextFlow ) )
    {
        MSO_TextFlow eTextFlow = (MSO_TextFlow)( GetPropertyValue( DFF_Prop_txflTextFlow ) & 0xFFFF );
        switch( eTextFlow )
        {
            case mso_txflTtoBA :    /* #68110# */   // Top to Bottom @-font, oben -> unten
            case mso_txflTtoBN :                    // Top to Bottom non-@, oben -> unten
            case mso_txflVertN :                    // Vertical, non-@, oben -> unten
                bVerticalText = sal_True;           // nTextRotationAngle += 27000;
            break;
            default: break;
        }
    }
    sal_Int32 nFontDirection = GetPropertyValue( DFF_Prop_cdirFont, mso_cdir0 );
    if ( ( nFontDirection == 1 ) || ( nFontDirection == 3 ) )
        bVerticalText = !bVerticalText;

    if ( bVerticalText )
    {
        eTVA = SDRTEXTVERTADJUST_BLOCK;
        eTHA = SDRTEXTHORZADJUST_CENTER;

        // read text anchor
        MSO_Anchor eTextAnchor = (MSO_Anchor)GetPropertyValue( DFF_Prop_anchorText, mso_anchorTop );

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
        eTHA = SDRTEXTHORZADJUST_BLOCK;

        // read text anchor
        MSO_Anchor eTextAnchor = (MSO_Anchor)GetPropertyValue( DFF_Prop_anchorText, mso_anchorTop );

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

    rSet.Put( SdrTextLeftDistItem( nTextLeft ) );
    rSet.Put( SdrTextRightDistItem( nTextRight ) );
    rSet.Put( SdrTextUpperDistItem( nTextTop ) );
    rSet.Put( SdrTextLowerDistItem( nTextBottom ) );

    rSet.Put( SdrTextWordWrapItem( (MSO_WrapMode)GetPropertyValue( DFF_Prop_WrapText, mso_wrapSquare ) != mso_wrapNone ? sal_True : sal_False ) );
    rSet.Put( SdrTextAutoGrowHeightItem( ( GetPropertyValue( DFF_Prop_FitTextToShape ) & 2 ) != 0 ) );
}

void DffPropertyReader::ApplyCustomShapeGeometryAttributes( SvStream& rIn, SfxItemSet& rSet, const DffObjData& rObjData ) const
{

    sal_uInt32 nAdjustmentsWhichNeedsToBeConverted = 0;

    ///////////////////////////////////////
    // creating SdrCustomShapeGeometryItem //
    ///////////////////////////////////////
    typedef uno::Sequence< beans::PropertyValue > PropSeq;
    typedef std::vector< beans::PropertyValue > PropVec;
    typedef PropVec::iterator PropVecIter;
    PropVecIter aIter;
    PropVecIter aEnd;


    // aPropVec will be filled with all PropertyValues
    PropVec aPropVec;
    PropertyValue aProp;

    /////////////////////////////////////////////////////////////////////
    // "Type" property, including the predefined CustomShape type name //
    /////////////////////////////////////////////////////////////////////
    const OUString sType( "Type" );
    aProp.Name  = sType;
    aProp.Value <<= EnhancedCustomShapeTypeNames::Get( rObjData.eShapeType );
    aPropVec.push_back( aProp );

    ///////////////
    // "ViewBox" //
    ///////////////

    sal_Int32 nCoordWidth = 21600;  // needed to replace handle type center with absolute value
    sal_Int32 nCoordHeight= 21600;
    if ( IsProperty( DFF_Prop_geoLeft ) || IsProperty( DFF_Prop_geoTop ) || IsProperty( DFF_Prop_geoRight ) || IsProperty( DFF_Prop_geoBottom ) )
    {
        com::sun::star::awt::Rectangle aViewBox;
        const OUString sViewBox( "ViewBox" );
        aViewBox.X = GetPropertyValue( DFF_Prop_geoLeft, 0 );
        aViewBox.Y = GetPropertyValue( DFF_Prop_geoTop, 0 );
        aViewBox.Width = nCoordWidth = ((sal_Int32)GetPropertyValue( DFF_Prop_geoRight, 21600 ) ) - aViewBox.X;
        aViewBox.Height = nCoordHeight = ((sal_Int32)GetPropertyValue( DFF_Prop_geoBottom, 21600 ) ) - aViewBox.Y;
        aProp.Name = sViewBox;
        aProp.Value <<= aViewBox;
        aPropVec.push_back( aProp );
    }
    /////////////////////
    // TextRotateAngle //
    /////////////////////
    if ( IsProperty( DFF_Prop_txflTextFlow ) || IsProperty( DFF_Prop_cdirFont ) )
    {
        sal_Int32 nTextRotateAngle = 0;
        MSO_TextFlow eTextFlow = (MSO_TextFlow)( GetPropertyValue( DFF_Prop_txflTextFlow ) & 0xFFFF );

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
    //////////////////////////////////////////
    // "Extrusion" PropertySequence element //
    //////////////////////////////////////////
    sal_Bool bExtrusionOn = ( GetPropertyValue( DFF_Prop_fc3DLightFace ) & 8 ) != 0;
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
            double fBrightness = (sal_Int32)GetPropertyValue( DFF_Prop_c3DAmbientIntensity );
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
            double fDiffusion = (sal_Int32)GetPropertyValue( DFF_Prop_c3DDiffuseAmt );
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
            aProp.Value <<= (sal_Int32)GetPropertyValue( DFF_Prop_c3DTolerance );
            aExtrusionPropVec.push_back( aProp );
        }
        // "LightFace"
        const OUString sExtrusionLightFace( "LightFace" );
        sal_Bool bExtrusionLightFace = ( GetPropertyValue( DFF_Prop_fc3DLightFace ) & 1 ) != 0;
        aProp.Name = sExtrusionLightFace;
        aProp.Value <<= bExtrusionLightFace;
        aExtrusionPropVec.push_back( aProp );
        // "FirstLightHarsh"
        const OUString sExtrusionFirstLightHarsh( "FirstLightHarsh" );
        sal_Bool bExtrusionFirstLightHarsh = ( GetPropertyValue( DFF_Prop_fc3DFillHarsh ) & 2 ) != 0;
        aProp.Name = sExtrusionFirstLightHarsh;
        aProp.Value <<= bExtrusionFirstLightHarsh;
        aExtrusionPropVec.push_back( aProp );
        // "SecondLightHarsh"
        const OUString sExtrusionSecondLightHarsh( "SecondLightHarsh" );
        sal_Bool bExtrusionSecondLightHarsh = ( GetPropertyValue( DFF_Prop_fc3DFillHarsh ) & 1 ) != 0;
        aProp.Name = sExtrusionSecondLightHarsh;
        aProp.Value <<= bExtrusionSecondLightHarsh;
        aExtrusionPropVec.push_back( aProp );
        // "FirstLightLevel"
        if ( IsProperty( DFF_Prop_c3DKeyIntensity ) )
        {
            const OUString sExtrusionFirstLightLevel( "FirstLightLevel" );
            double fFirstLightLevel = (sal_Int32)GetPropertyValue( DFF_Prop_c3DKeyIntensity );
            fFirstLightLevel /= 655.36;
            aProp.Name = sExtrusionFirstLightLevel;
            aProp.Value <<= fFirstLightLevel;
            aExtrusionPropVec.push_back( aProp );
        }
        // "SecondLightLevel"
        if ( IsProperty( DFF_Prop_c3DFillIntensity ) )
        {
            const OUString sExtrusionSecondLightLevel( "SecondLightLevel" );
            double fSecondLightLevel = (sal_Int32)GetPropertyValue( DFF_Prop_c3DFillIntensity );
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
            ::com::sun::star::drawing::Direction3D aExtrusionFirstLightDirection( fLightX, fLightY, fLightZ );
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
            ::com::sun::star::drawing::Direction3D aExtrusionSecondLightDirection( fLight2X, fLight2Y, fLight2Z );
            const OUString sExtrusionSecondLightDirection( "SecondLightDirection" );
            aProp.Name = sExtrusionSecondLightDirection;
            aProp.Value <<= aExtrusionSecondLightDirection;
            aExtrusionPropVec.push_back( aProp );
        }

        // "Metal"
        const OUString sExtrusionMetal( "Metal" );
        sal_Bool bExtrusionMetal = ( GetPropertyValue( DFF_Prop_fc3DLightFace ) & 4 ) != 0;
        aProp.Name = sExtrusionMetal;
        aProp.Value <<= bExtrusionMetal;
        aExtrusionPropVec.push_back( aProp );
        // "ShadeMode"
        if ( IsProperty( DFF_Prop_c3DRenderMode ) )
        {
            const OUString sExtrusionShadeMode( "ShadeMode" );
            sal_uInt32 nExtrusionRenderMode = GetPropertyValue( DFF_Prop_c3DRenderMode );
            com::sun::star::drawing::ShadeMode eExtrusionShadeMode( com::sun::star::drawing::ShadeMode_FLAT );
            if ( nExtrusionRenderMode == mso_Wireframe )
                eExtrusionShadeMode = com::sun::star::drawing::ShadeMode_DRAFT;

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
        if ( ( GetPropertyValue( DFF_Prop_fc3DFillHarsh ) & 8 ) == 0 )
        {
            // "RotationCenter"
            if ( IsProperty( DFF_Prop_c3DRotationCenterX ) || IsProperty( DFF_Prop_c3DRotationCenterY ) || IsProperty( DFF_Prop_c3DRotationCenterZ ) )
            {
                ::com::sun::star::drawing::Direction3D aRotationCenter(
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
            double fShininess = (sal_Int32)GetPropertyValue( DFF_Prop_c3DShininess );
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
            double fSpecularity = (sal_Int32)GetPropertyValue( DFF_Prop_c3DSpecularAmt );
            fSpecularity /= 1333;
            aProp.Name = sExtrusionSpecularity;
            aProp.Value <<= fSpecularity;
            aExtrusionPropVec.push_back( aProp );
        }
        // "ProjectionMode"
        const OUString sExtrusionProjectionMode( "ProjectionMode" );
        ProjectionMode eProjectionMode = GetPropertyValue( DFF_Prop_fc3DFillHarsh ) & 4 ? ProjectionMode_PARALLEL : ProjectionMode_PERSPECTIVE;
        aProp.Name = sExtrusionProjectionMode;
        aProp.Value <<= eProjectionMode;
        aExtrusionPropVec.push_back( aProp );

        // "ViewPoint" in 1/100mm
        if ( IsProperty( DFF_Prop_c3DXViewpoint ) || IsProperty( DFF_Prop_c3DYViewpoint ) || IsProperty( DFF_Prop_c3DZViewpoint ) )
        {
            double fViewX = (double)((sal_Int32)GetPropertyValue( DFF_Prop_c3DXViewpoint, 1250000 )) / 360.0;
            double fViewY = (double)((sal_Int32)GetPropertyValue( DFF_Prop_c3DYViewpoint, (sal_uInt32)-1250000 ))/ 360.0;
            double fViewZ = (double)((sal_Int32)GetPropertyValue( DFF_Prop_c3DZViewpoint, 9000000 )) / 360.0;
            ::com::sun::star::drawing::Position3D aExtrusionViewPoint( fViewX, fViewY, fViewZ );
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
        sal_Bool bExtrusionColor = IsProperty( DFF_Prop_c3DExtrusionColor );    // ( GetPropertyValue( DFF_Prop_fc3DLightFace ) & 2 ) != 0;
        aProp.Name = sExtrusionColor;
        aProp.Value <<= bExtrusionColor;
        aExtrusionPropVec.push_back( aProp );
        if ( IsProperty( DFF_Prop_c3DExtrusionColor ) )
            rSet.Put( XSecondaryFillColorItem( OUString(), rManager.MSO_CLR_ToColor(
                GetPropertyValue( DFF_Prop_c3DExtrusionColor ), DFF_Prop_c3DExtrusionColor ) ) );
        // pushing the whole Extrusion element
        const OUString sExtrusion( "Extrusion" );
        PropSeq aExtrusionPropSeq( aExtrusionPropVec.size() );
        aIter = aExtrusionPropVec.begin();
        aEnd = aExtrusionPropVec.end();
        beans::PropertyValue* pExtrusionValues = aExtrusionPropSeq.getArray();
        while ( aIter != aEnd )
            *pExtrusionValues++ = *aIter++;
        aProp.Name = sExtrusion;
        aProp.Value <<= aExtrusionPropSeq;
        aPropVec.push_back( aProp );
    }

    /////////////////////////////////////////
    // "Equations" PropertySequence element //
    /////////////////////////////////////////
    if ( IsProperty( DFF_Prop_pFormulas ) )
    {
        sal_uInt16 nNumElem = 0;

        if ( SeekToContent( DFF_Prop_pFormulas, rIn ) )
        {
            sal_uInt16 nNumElemMem = 0;
            sal_uInt16 nElemSize = 8;
            rIn >> nNumElem >> nNumElemMem >> nElemSize;
        }
        sal_Int16 nP1, nP2, nP3;
        sal_uInt16 nFlags;

        uno::Sequence< OUString > aEquations( nNumElem );
        for ( sal_uInt16 i = 0; i < nNumElem; i++ )
        {
            rIn >> nFlags >> nP1 >> nP2 >> nP3;
            aEquations[ i ] = EnhancedCustomShape2d::GetEquation( nFlags, nP1, nP2, nP3 );
        }
        // pushing the whole Equations element
        const OUString sEquations( "Equations" );
        aProp.Name = sEquations;
        aProp.Value <<= aEquations;
        aPropVec.push_back( aProp );
    }

    ////////////////////////////////////////
    // "Handles" PropertySequence element //
    ////////////////////////////////////////
    if ( IsProperty( DFF_Prop_Handles ) )
    {
        sal_uInt16 nNumElem = 0;
        sal_uInt16 nElemSize = 36;

        if ( SeekToContent( DFF_Prop_Handles, rIn ) )
        {
            sal_uInt16 nNumElemMem = 0;
            rIn >> nNumElem >> nNumElemMem >> nElemSize;
        }
        if ( nElemSize == 36 )
        {
            uno::Sequence< beans::PropertyValues > aHandles( nNumElem );
            for ( sal_uInt16 i = 0; i < nNumElem; i++ )
            {
                PropVec aHandlePropVec;
                sal_uInt32  nFlags;
                sal_Int32   nPositionX, nPositionY, nCenterX, nCenterY, nRangeXMin, nRangeXMax, nRangeYMin, nRangeYMax;
                rIn >> nFlags
                    >> nPositionX
                    >> nPositionY
                    >> nCenterX
                    >> nCenterY
                    >> nRangeXMin
                    >> nRangeXMax
                    >> nRangeYMin
                    >> nRangeYMax;

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

                if ( nFlags & MSDFF_HANDLE_FLAGS_MIRRORED_X )
                {
                    sal_Bool bMirroredX = sal_True;
                    const OUString sHandleMirroredX( "MirroredX" );
                    aProp.Name = sHandleMirroredX;
                    aProp.Value <<= bMirroredX;
                    aHandlePropVec.push_back( aProp );
                }
                if ( nFlags & MSDFF_HANDLE_FLAGS_MIRRORED_Y )
                {
                    sal_Bool bMirroredY = sal_True;
                    const OUString sHandleMirroredY( "MirroredY" );
                    aProp.Name = sHandleMirroredY;
                    aProp.Value <<= bMirroredY;
                    aHandlePropVec.push_back( aProp );
                }
                if ( nFlags & MSDFF_HANDLE_FLAGS_SWITCHED )
                {
                    sal_Bool bSwitched = sal_True;
                    const OUString sHandleSwitched( "Switched" );
                    aProp.Name = sHandleSwitched;
                    aProp.Value <<= bSwitched;
                    aHandlePropVec.push_back( aProp );
                }
                if ( nFlags & MSDFF_HANDLE_FLAGS_POLAR )
                {
                    if ( nCenterX == 2 )
                        nCenterX = nCoordWidth / 2;
                    if ( nCenterY == 2 )
                        nCenterY = nCoordHeight / 2;
                    if ( ( nPositionY >= 0x256 ) || ( nPositionY <= 0x107 ) )   // position y
                        nAdjustmentsWhichNeedsToBeConverted |= ( 1 << i );
                    EnhancedCustomShapeParameterPair aPolar;
                    EnhancedCustomShape2d::SetEnhancedCustomShapeHandleParameter( aPolar.First,  nCenterX, ( nFlags & 0x800  ) != 0, true  );
                    EnhancedCustomShape2d::SetEnhancedCustomShapeHandleParameter( aPolar.Second, nCenterY, ( nFlags & 0x1000 ) != 0, false );
                    const OUString sHandlePolar( "Polar" );
                    aProp.Name = sHandlePolar;
                    aProp.Value <<= aPolar;
                    aHandlePropVec.push_back( aProp );
                }
                if ( nFlags & MSDFF_HANDLE_FLAGS_MAP )
                {
                    if ( nCenterX == 2 )
                        nCenterX = nCoordWidth / 2;
                    if ( nCenterY == 2 )
                        nCenterY = nCoordHeight / 2;
                    EnhancedCustomShapeParameterPair aMap;
                    EnhancedCustomShape2d::SetEnhancedCustomShapeHandleParameter( aMap.First,  nCenterX, ( nFlags & 0x800  ) != 0, true  );
                    EnhancedCustomShape2d::SetEnhancedCustomShapeHandleParameter( aMap.Second, nCenterY, ( nFlags & 0x1000 ) != 0, false );
                    const OUString sHandleMap( "Map" );
                    aProp.Name = sHandleMap;
                    aProp.Value <<= aMap;
                    aHandlePropVec.push_back( aProp );
                }
                if ( nFlags & MSDFF_HANDLE_FLAGS_RANGE )
                {
                    if ( (sal_uInt32)nRangeXMin != 0x80000000 )
                    {
                        if ( nRangeXMin == 2 )
                            nRangeXMin = nCoordWidth / 2;
                        EnhancedCustomShapeParameter aRangeXMinimum;
                        EnhancedCustomShape2d::SetEnhancedCustomShapeHandleParameter( aRangeXMinimum,  nRangeXMin,
                            ( nFlags & MSDFF_HANDLE_FLAGS_RANGE_X_MIN_IS_SPECIAL ) != 0, true  );
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
                            ( nFlags & MSDFF_HANDLE_FLAGS_RANGE_X_MAX_IS_SPECIAL ) != 0, false );
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
                            ( nFlags & MSDFF_HANDLE_FLAGS_RANGE_Y_MIN_IS_SPECIAL ) != 0, true );
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
                            ( nFlags & MSDFF_HANDLE_FLAGS_RANGE_Y_MAX_IS_SPECIAL ) != 0, false );
                        const OUString sHandleRangeYMaximum( "RangeYMaximum" );
                        aProp.Name = sHandleRangeYMaximum;
                        aProp.Value <<= aRangeYMaximum;
                        aHandlePropVec.push_back( aProp );
                    }
                }
                if ( nFlags & MSDFF_HANDLE_FLAGS_RADIUS_RANGE )
                {
                    if ( (sal_uInt32)nRangeXMin != 0x7fffffff )
                    {
                        if ( nRangeXMin == 2 )
                            nRangeXMin = nCoordWidth / 2;
                        EnhancedCustomShapeParameter aRadiusRangeMinimum;
                        EnhancedCustomShape2d::SetEnhancedCustomShapeHandleParameter( aRadiusRangeMinimum, nRangeXMin,
                            ( nFlags & MSDFF_HANDLE_FLAGS_RANGE_X_MIN_IS_SPECIAL ) != 0, true  );
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
                            ( nFlags & MSDFF_HANDLE_FLAGS_RANGE_X_MAX_IS_SPECIAL ) != 0, false );
                        const OUString sHandleRadiusRangeMaximum( "RadiusRangeMaximum" );
                        aProp.Name = sHandleRadiusRangeMaximum;
                        aProp.Value <<= aRadiusRangeMaximum;
                        aHandlePropVec.push_back( aProp );
                    }
                }
                if ( !aHandlePropVec.empty() )
                {
                    PropSeq aHandlePropSeq( aHandlePropVec.size() );
                    aIter = aHandlePropVec.begin();
                    aEnd = aHandlePropVec.end();
                    beans::PropertyValue* pHandleValues = aHandlePropSeq.getArray();
                    while ( aIter != aEnd )
                        *pHandleValues++ = *aIter++;
                    aHandles[ i ] = aHandlePropSeq;
                }
            }
            // pushing the whole Handles element
            const OUString sHandles( "Handles" );
            aProp.Name = sHandles;
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
                if ( pData->nFlags & MSDFF_HANDLE_FLAGS_POLAR )
                {
                    if ( ( pData->nPositionY >= 0x256 ) || ( pData->nPositionY <= 0x107 ) )
                        nAdjustmentsWhichNeedsToBeConverted |= ( 1 << i );
                }
            }
        }
    }
    /////////////////////////////////////
    // "Path" PropertySequence element //
    /////////////////////////////////////
    {
        PropVec aPathPropVec;

        // "Path/ExtrusionAllowed"
        if ( IsHardAttribute( DFF_Prop_f3DOK ) )
        {
            const OUString sExtrusionAllowed( "ExtrusionAllowed" );
            sal_Bool bExtrusionAllowed = ( GetPropertyValue( DFF_Prop_fFillOK ) & 16 ) != 0;
            aProp.Name = sExtrusionAllowed;
            aProp.Value <<= bExtrusionAllowed;
            aPathPropVec.push_back( aProp );
        }
        // "Path/ConcentricGradientFillAllowed"
        if ( IsHardAttribute( DFF_Prop_fFillShadeShapeOK ) )
        {
            const OUString sConcentricGradientFillAllowed( "ConcentricGradientFillAllowed" );
            sal_Bool bConcentricGradientFillAllowed = ( GetPropertyValue( DFF_Prop_fFillOK ) & 2 ) != 0;
            aProp.Name = sConcentricGradientFillAllowed;
            aProp.Value <<= bConcentricGradientFillAllowed;
            aPathPropVec.push_back( aProp );
        }
        // "Path/TextPathAllowed"
        if ( IsHardAttribute( DFF_Prop_fGtextOK ) || ( GetPropertyValue( DFF_Prop_gtextFStrikethrough, 0 ) & 0x4000 ) )
        {
            const OUString sTextPathAllowed( "TextPathAllowed" );
            sal_Bool bTextPathAllowed = ( GetPropertyValue( DFF_Prop_fFillOK ) & 4 ) != 0;
            aProp.Name = sTextPathAllowed;
            aProp.Value <<= bTextPathAllowed;
            aPathPropVec.push_back( aProp );
        }
        // Path/Coordinates
        if ( IsProperty( DFF_Prop_pVertices ) )
        {
            com::sun::star::uno::Sequence< com::sun::star::drawing::EnhancedCustomShapeParameterPair > aCoordinates;
            sal_uInt16 nNumElemVert = 0;
            sal_uInt16 nElemSizeVert = 8;

            if ( SeekToContent( DFF_Prop_pVertices, rIn ) )
            {
                sal_uInt16 nNumElemMemVert = 0;
                rIn >> nNumElemVert >> nNumElemMemVert >> nElemSizeVert;
            }
            if ( nNumElemVert )
            {
                sal_Int32 nX, nY;
                sal_Int16 nTmpA, nTmpB;
                aCoordinates.realloc( nNumElemVert );
                for ( sal_uInt16 i = 0; i < nNumElemVert; i++ )
                {
                    if ( nElemSizeVert == 8 )
                    {
                        rIn >> nX
                            >> nY;
                    }
                    else
                    {
                        rIn >> nTmpA
                            >> nTmpB;

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
            com::sun::star::uno::Sequence< com::sun::star::drawing::EnhancedCustomShapeSegment > aSegments;

            sal_uInt16 i, nTmp;
            sal_uInt16 nNumElemSeg = 0;

            if ( SeekToContent( DFF_Prop_pSegmentInfo, rIn ) )
            {
                sal_uInt16 nNumElemMemSeg = 0;
                sal_uInt16 nElemSizeSeg = 2;
                rIn >> nNumElemSeg >> nNumElemMemSeg >> nElemSizeSeg;
            }
            if ( nNumElemSeg )
            {
                sal_Int16 nCommand;
                sal_Int16 nCnt;
                aSegments.realloc( nNumElemSeg );
                for ( i = 0; i < nNumElemSeg; i++ )
                {
                    rIn >> nTmp;
                    nCommand = EnhancedCustomShapeSegmentCommand::UNKNOWN;
                    nCnt = (sal_Int16)( nTmp & 0x1fff );//Last 13 bits for segment points number
                    switch( nTmp >> 13 )//First 3 bits for command type
                    {
                        case 0x0: nCommand = EnhancedCustomShapeSegmentCommand::LINETO; if ( !nCnt ) nCnt = 1; break;
                        case 0x1: nCommand = EnhancedCustomShapeSegmentCommand::CURVETO; if ( !nCnt ) nCnt = 1; break;
                        case 0x2: nCommand = EnhancedCustomShapeSegmentCommand::MOVETO; if ( !nCnt ) nCnt = 1; break;
                        case 0x3: nCommand = EnhancedCustomShapeSegmentCommand::CLOSESUBPATH; nCnt = 0; break;
                        case 0x4: nCommand = EnhancedCustomShapeSegmentCommand::ENDSUBPATH; nCnt = 0; break;
                        case 0x5:
                        case 0x6:
                        {
                            switch ( ( nTmp >> 8 ) & 0x1f )//5 bits next to command type is for path escape type
                            {
                                case 0x0:
                                {
                                    //It is msopathEscapeExtension which is transformed into LINETO.
                                    //If issue happens, I think this part can be comment so that it will be taken as unknow command.
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
                rIn >> nNumElem >> nNumElemMem >> nElemSize;
            }
            if ( nElemSize == 16 )
            {
                sal_Int32 nLeft, nTop, nRight, nBottom;
                com::sun::star::uno::Sequence< com::sun::star::drawing::EnhancedCustomShapeTextFrame > aTextFrames( nNumElem );
                for ( sal_uInt16 i = 0; i < nNumElem; i++ )
                {
                    rIn >> nLeft
                        >> nTop
                        >> nRight
                        >> nBottom;

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
            com::sun::star::uno::Sequence< com::sun::star::drawing::EnhancedCustomShapeParameterPair > aGluePoints;
            sal_uInt16 nNumElemVert = 0;
            sal_uInt16 nNumElemMemVert = 0;
            sal_uInt16 nElemSizeVert = 8;

            if ( SeekToContent( DFF_Prop_connectorPoints, rIn ) )
                rIn >> nNumElemVert >> nNumElemMemVert >> nElemSizeVert;

            sal_Int32 nX, nY;
            sal_Int16 nTmpA, nTmpB;
            aGluePoints.realloc( nNumElemVert );
            for ( sal_uInt16 i = 0; i < nNumElemVert; i++ )
            {
                if ( nElemSizeVert == 8 )
                {
                    rIn >> nX
                        >> nY;
                }
                else
                {
                    rIn >> nTmpA
                        >> nTmpB;

                    nX = nTmpA;
                    nY = nTmpB;
                }
                EnhancedCustomShape2d::SetEnhancedCustomShapeParameter( aGluePoints[ i ].First,  nX );
                EnhancedCustomShape2d::SetEnhancedCustomShapeParameter( aGluePoints[ i ].Second, nY );
            }
            const OUString sGluePoints( "GluePoints" );
            aProp.Name = sGluePoints;
            aProp.Value <<= aGluePoints;
            aPathPropVec.push_back( aProp );
        }
        if ( IsProperty( DFF_Prop_connectorType ) )
        {
            sal_Int16 nGluePointType = (sal_uInt16)GetPropertyValue( DFF_Prop_connectorType );
            const OUString sGluePointType( "GluePointType" );
            aProp.Name = sGluePointType;
            aProp.Value <<= nGluePointType;
            aPathPropVec.push_back( aProp );
        }
        // pushing the whole Path element
        if ( !aPathPropVec.empty() )
        {
            const OUString sPath( "Path" );
            PropSeq aPathPropSeq( aPathPropVec.size() );
            aIter = aPathPropVec.begin();
            aEnd = aPathPropVec.end();
            beans::PropertyValue* pPathValues = aPathPropSeq.getArray();
            while ( aIter != aEnd )
                *pPathValues++ = *aIter++;
            aProp.Name = sPath;
            aProp.Value <<= aPathPropSeq;
            aPropVec.push_back( aProp );
        }
    }
    /////////////////////////////////////////
    // "TextPath" PropertySequence element //
    /////////////////////////////////////////
    sal_Bool bTextPathOn = ( GetPropertyValue( DFF_Prop_gtextFStrikethrough ) & 0x4000 ) != 0;
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
        sal_Bool bTextPathFitPath = ( GetPropertyValue( DFF_Prop_gtextFStrikethrough ) & 0x100 ) != 0;

        sal_Bool bTextPathFitShape;
        if ( IsHardAttribute( DFF_Prop_gtextFStretch ) )
            bTextPathFitShape = ( GetPropertyValue( DFF_Prop_gtextFStrikethrough ) & 0x400 ) != 0;
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
        sal_Bool bTextPathScaleX = ( GetPropertyValue( DFF_Prop_gtextFStrikethrough ) & 0x40 ) != 0;
        aProp.Name = sTextPathScaleX;
        aProp.Value <<= bTextPathScaleX;
        aTextPathPropVec.push_back( aProp );
        // SameLetterHeights
        const OUString sSameLetterHeight( "SameLetterHeights" );
        sal_Bool bSameLetterHeight = ( GetPropertyValue( DFF_Prop_gtextFStrikethrough ) & 0x80 ) != 0;
        aProp.Name = sSameLetterHeight;
        aProp.Value <<= bSameLetterHeight;
        aTextPathPropVec.push_back( aProp );

        // pushing the whole TextPath element
        const OUString sTextPath( "TextPath" );
        PropSeq aTextPathPropSeq( aTextPathPropVec.size() );
        aIter = aTextPathPropVec.begin();
        aEnd = aTextPathPropVec.end();
        beans::PropertyValue* pTextPathValues = aTextPathPropSeq.getArray();
        while ( aIter != aEnd )
            *pTextPathValues++ = *aIter++;
        aProp.Name = sTextPath;
        aProp.Value <<= aTextPathPropSeq;
        aPropVec.push_back( aProp );
    }
    ////////////////////////
    // "AdjustmentValues" // The AdjustmentValues are imported at last, because depending to the type of the
    //////////////////////// handle (POLAR) we will convert the adjustment value from a fixed float to double

    // checking the last used adjustment handle, so we can determine how many handles are to allocate
    sal_Int32 i = DFF_Prop_adjust10Value;
    while ( ( i >= DFF_Prop_adjustValue ) && !IsProperty( i ) )
        i--;
    sal_Int32 nAdjustmentValues = ( i - DFF_Prop_adjustValue ) + 1;
    if ( nAdjustmentValues )
    {
        uno::Sequence< com::sun::star::drawing::EnhancedCustomShapeAdjustmentValue > aAdjustmentSeq( nAdjustmentValues );
        while( --nAdjustmentValues >= 0 )
        {
            sal_Int32 nValue = 0;
            beans::PropertyState ePropertyState = beans::PropertyState_DEFAULT_VALUE;
            if ( IsProperty( i ) )
            {
                nValue = GetPropertyValue( i );
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
    PropSeq aSeq( aPropVec.size() );
    beans::PropertyValue* pValues = aSeq.getArray();
    aIter = aPropVec.begin();
    aEnd = aPropVec.end();
    while ( aIter != aEnd )
        *pValues++ = *aIter++;
    rSet.Put( SdrCustomShapeGeometryItem( aSeq ) );
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
    sal_Bool bHasShadow = sal_False;

    if ( IsProperty( DFF_Prop_gtextSize ) )
        rSet.Put( SvxFontHeightItem( rManager.ScalePt( GetPropertyValue( DFF_Prop_gtextSize ) ), 100, EE_CHAR_FONTHEIGHT ) );
    sal_uInt32 nFontAttributes = GetPropertyValue( DFF_Prop_gtextFStrikethrough );
    if ( nFontAttributes & 0x20 )
        rSet.Put( SvxWeightItem( nFontAttributes & 0x20 ? WEIGHT_BOLD : WEIGHT_NORMAL, EE_CHAR_WEIGHT ) );
    if ( nFontAttributes & 0x10 )
        rSet.Put( SvxPostureItem( nFontAttributes & 0x10 ? ITALIC_NORMAL : ITALIC_NONE, EE_CHAR_ITALIC ) );
    if ( nFontAttributes & 0x08 )
        rSet.Put( SvxUnderlineItem( nFontAttributes & 0x08 ? UNDERLINE_SINGLE : UNDERLINE_NONE, EE_CHAR_UNDERLINE ) );
    if ( nFontAttributes & 0x40 )
        rSet.Put( SvxShadowedItem( (nFontAttributes & 0x40) != 0, EE_CHAR_SHADOW ) );
//    if ( nFontAttributes & 0x02 )
//        rSet.Put( SvxCaseMapItem( nFontAttributes & 0x02 ? SVX_CASEMAP_KAPITAELCHEN : SVX_CASEMAP_NOT_MAPPED ) );
    if ( nFontAttributes & 0x01 )
        rSet.Put( SvxCrossedOutItem( nFontAttributes & 0x01 ? STRIKEOUT_SINGLE : STRIKEOUT_NONE, EE_CHAR_STRIKEOUT ) );
    if ( IsProperty( DFF_Prop_fillColor ) )
        rSet.Put( XFillColorItem( OUString(), rManager.MSO_CLR_ToColor( GetPropertyValue( DFF_Prop_fillColor ), DFF_Prop_fillColor ) ) );
    if ( IsProperty( DFF_Prop_shadowColor ) )
        rSet.Put( SdrShadowColorItem( rManager.MSO_CLR_ToColor( GetPropertyValue( DFF_Prop_shadowColor ), DFF_Prop_shadowColor ) ) );
    else
    {
        //The default value for this property is 0x00808080
        rSet.Put( SdrShadowColorItem( rManager.MSO_CLR_ToColor( 0x00808080, DFF_Prop_shadowColor ) ) );
    }
    if ( IsProperty( DFF_Prop_shadowOpacity ) )
        rSet.Put( SdrShadowTransparenceItem( (sal_uInt16)( ( 0x10000 - GetPropertyValue( DFF_Prop_shadowOpacity ) ) / 655 ) ) );
    if ( IsProperty( DFF_Prop_shadowOffsetX ) )
    {
        sal_Int32 nVal = static_cast< sal_Int32 >( GetPropertyValue( DFF_Prop_shadowOffsetX ) );
        rManager.ScaleEmu( nVal );
        rSet.Put( SdrShadowXDistItem( nVal ) );
    }
    if ( IsProperty( DFF_Prop_shadowOffsetY ) )
    {
        sal_Int32 nVal = static_cast< sal_Int32 >( GetPropertyValue( DFF_Prop_shadowOffsetY ) );
        rManager.ScaleEmu( nVal );
        rSet.Put( SdrShadowYDistItem( nVal ) );
    }
    if ( IsProperty( DFF_Prop_fshadowObscured ) )
    {
        bHasShadow = ( GetPropertyValue( DFF_Prop_fshadowObscured ) & 2 ) != 0;
        if ( bHasShadow )
        {
            if ( !IsProperty( DFF_Prop_shadowOffsetX ) )
                rSet.Put( SdrShadowXDistItem( 35 ) );
            if ( !IsProperty( DFF_Prop_shadowOffsetY ) )
                rSet.Put( SdrShadowYDistItem( 35 ) );
        }
    }
    if ( IsProperty( DFF_Prop_shadowType ) )
    {
        MSO_ShadowType eShadowType = static_cast< MSO_ShadowType >( GetPropertyValue( DFF_Prop_shadowType ) );
        if( eShadowType != mso_shadowOffset )
        {
            //0.12" == 173 twip == 302 100mm
            sal_uInt32 nDist = rManager.pSdrModel->GetScaleUnit() == MAP_TWIP ? 173: 302;
            rSet.Put( SdrShadowXDistItem( nDist ) );
            rSet.Put( SdrShadowYDistItem( nDist ) );
        }
    }
    if ( bHasShadow )
    {
        // #160376# sj: activating shadow only if fill and or linestyle is used
        // this is required because of the latest drawing layer core changes.
        // Issue i104085 is related to this.
        sal_uInt32 nLineFlags(GetPropertyValue( DFF_Prop_fNoLineDrawDash ));
        if(!IsHardAttribute( DFF_Prop_fLine ) && !IsCustomShapeStrokedByDefault( rObjData.eShapeType ))
            nLineFlags &= ~0x08;
        sal_uInt32 nFillFlags(GetPropertyValue( DFF_Prop_fNoFillHitTest ));
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
            bHasShadow = sal_False;                                             // we are turning shadow off.

        if ( bHasShadow )
            rSet.Put( SdrShadowItem( bHasShadow ) );
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
    sal_Bool bRotateTextWithShape = rObjData.bRotateTextWithShape;
    if ( rObjData.bOpt2 )        // sj: #158494# is the second property set available ? if then we have to check the xml data of
    {                            // the shape, because the textrotation of Excel 2003 and greater versions is stored there
                                // (upright property of the textbox)
        if ( rManager.pSecPropSet->SeekToContent( DFF_Prop_metroBlob, rIn ) )
        {
            sal_uInt32 nLen = rManager.pSecPropSet->GetPropertyValue( DFF_Prop_metroBlob );
            if ( nLen )
            {
                ::com::sun::star::uno::Sequence< sal_Int8 > aXMLDataSeq( nLen );
                rIn.Read( aXMLDataSeq.getArray(), nLen );
                ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > xInputStream
                    ( new ::comphelper::SequenceInputStream( aXMLDataSeq ) );
                try
                {
                    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > xContext( ::comphelper::getProcessComponentContext() );
                    ::com::sun::star::uno::Reference< com::sun::star::embed::XStorage > xStorage
                        ( ::comphelper::OStorageHelper::GetStorageOfFormatFromInputStream(
                            OFOPXML_STORAGE_FORMAT_STRING, xInputStream, xContext, sal_True ) );
                    if ( xStorage.is() )
                    {
                        const OUString sDRS( "drs" );
                        ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >
                            xStorageDRS( xStorage->openStorageElement( sDRS, ::com::sun::star::embed::ElementModes::SEEKABLEREAD ) );
                        if ( xStorageDRS.is() )
                        {
                            const OUString sShapeXML( "shapexml.xml" );
                            ::com::sun::star::uno::Reference< ::com::sun::star::io::XStream > xShapeXMLStream( xStorageDRS->openStreamElement( sShapeXML, ::com::sun::star::embed::ElementModes::SEEKABLEREAD ) );
                            if ( xShapeXMLStream.is() )
                            {
                                ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > xShapeXMLInputStream( xShapeXMLStream->getInputStream() );
                                if ( xShapeXMLInputStream.is() )
                                {
                                    ::com::sun::star::uno::Sequence< sal_Int8 > aSeq;
                                    sal_Int32 nBytesRead = xShapeXMLInputStream->readBytes( aSeq, 0x7fffffff );
                                    if ( nBytesRead )
                                    {    // for only one property I spare to use a XML parser at this point, this
                                        // should be enhanced if needed

                                        bRotateTextWithShape = sal_True;    // using the correct xml default
                                        const char* pArry = reinterpret_cast< char* >( aSeq.getArray() );
                                        const char* pUpright = "upright=";
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
                catch( com::sun::star::uno::Exception& )
                {
                }
            }
        }
    }
    if ( !bRotateTextWithShape )
    {
        const com::sun::star::uno::Any* pAny, aAny;
        SdrCustomShapeGeometryItem aGeometryItem((SdrCustomShapeGeometryItem&)rSet.Get( SDRATTR_CUSTOMSHAPE_GEOMETRY ));
        const OUString sTextRotateAngle( "TextRotateAngle" );
        pAny = aGeometryItem.GetPropertyValueByName( sTextRotateAngle );
        double fExtraTextRotateAngle = 0.0;
        if ( pAny )
            *pAny >>= fExtraTextRotateAngle;

        if ( rManager.mnFix16Angle )
            fExtraTextRotateAngle += mnFix16Angle / 100.0;
        if ( rObjData.nSpFlags & SP_FFLIPV )
            fExtraTextRotateAngle -= 180.0;

        com::sun::star::beans::PropertyValue aTextRotateAngle;
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

    XGradientStyle eGrad = XGRAD_LINEAR;

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
        eGrad = XGRAD_AXIAL;//A axial gradient other than linear
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
            eGrad = XGRAD_RECT;
            nFocusY = nFocusX = 50;
            nChgColors ^= 1;
        }
        break;
    case mso_fillShadeCenter :
        {
            eGrad = XGRAD_RECT;
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
    aSet.Put( XFillGradientItem( String(), aGrad ) );
    //Construct tranparency item. This item can coodinate with both solid and gradient.
    if ( dTrans < 1.0 || dBackTrans < 1.0 )
    {
        sal_uInt8 nStartCol = (sal_uInt8)( (1 - dTrans )* 255 );
        sal_uInt8 nEndCol = (sal_uInt8)( ( 1- dBackTrans ) * 255 );
        aCol1 = Color(nStartCol, nStartCol, nStartCol);
        aCol2 = Color(nEndCol, nEndCol, nEndCol);

        XGradient aGrad2( aCol2 ,  aCol1 , eGrad, nAngle, nFocusX, nFocusY );
        aSet.Put( XFillFloatTransparenceItem( String(), aGrad2 ) );
    }
}

//---------------------------------------------------------------------------
//- Record Manager ----------------------------------------------------------
//---------------------------------------------------------------------------

DffRecordList::DffRecordList( DffRecordList* pList ) :
    nCount                  ( 0 ),
    nCurrent                ( 0 ),
    pPrev                   ( pList ),
    pNext                   ( NULL )
{
    if ( pList )
        pList->pNext = this;
}

DffRecordList::~DffRecordList()
{
    delete pNext;
}

DffRecordManager::DffRecordManager() :
    DffRecordList   ( NULL ),
    pCList          ( (DffRecordList*)this )
{
}

DffRecordManager::DffRecordManager( SvStream& rIn ) :
    DffRecordList   ( NULL ),
    pCList          ( (DffRecordList*)this )
{
    Consume( rIn );
}

DffRecordManager::~DffRecordManager()
{
};


void DffRecordManager::Consume( SvStream& rIn, sal_Bool bAppend, sal_uInt32 nStOfs )
{
    if ( !bAppend )
        Clear();
    sal_uInt32 nOldPos = rIn.Tell();
    if ( !nStOfs )
    {
        DffRecordHeader aHd;
        rIn >> aHd;
        if ( aHd.nRecVer == DFF_PSFLAG_CONTAINER )
            nStOfs = aHd.GetRecEndFilePos();
    }
    if ( nStOfs )
    {
        pCList = (DffRecordList*)this;
        while ( pCList->pNext )
            pCList = pCList->pNext;
        while ( ( rIn.GetError() == 0 ) && ( ( rIn.Tell() + 8 ) <=  nStOfs ) )
        {
            if ( pCList->nCount == DFF_RECORD_MANAGER_BUF_SIZE )
                pCList = new DffRecordList( pCList );
            rIn >> pCList->mHd[ pCList->nCount ];
            bool bSeekSucceeded = pCList->mHd[ pCList->nCount++ ].SeekToEndOfRecord(rIn);
            if (!bSeekSucceeded)
                break;
        }
        rIn.Seek( nOldPos );
    }
}

void DffRecordManager::Clear()
{
    pCList = (DffRecordList*)this;
    delete pNext, pNext = NULL;
    nCurrent = 0;
    nCount = 0;
}

DffRecordHeader* DffRecordManager::Current()
{
    DffRecordHeader* pRet = NULL;
    if ( pCList->nCurrent < pCList->nCount )
        pRet = &pCList->mHd[ pCList->nCurrent ];
    return pRet;
}

DffRecordHeader* DffRecordManager::First()
{
    DffRecordHeader* pRet = NULL;
    pCList = (DffRecordList*)this;
    if ( pCList->nCount )
    {
        pCList->nCurrent = 0;
        pRet = &pCList->mHd[ 0 ];
    }
    return pRet;
}

DffRecordHeader* DffRecordManager::Next()
{
    DffRecordHeader* pRet = NULL;
    sal_uInt32 nC = pCList->nCurrent + 1;
    if ( nC < pCList->nCount )
    {
        pCList->nCurrent++;
        pRet = &pCList->mHd[ nC ];
    }
    else if ( pCList->pNext )
    {
        pCList = pCList->pNext;
        pCList->nCurrent = 0;
        pRet = &pCList->mHd[ 0 ];
    }
    return pRet;
}

DffRecordHeader* DffRecordManager::Prev()
{
    DffRecordHeader* pRet = NULL;
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
    DffRecordHeader* pRet = NULL;
    while ( pCList->pNext )
        pCList = pCList->pNext;
    sal_uInt32 nCnt = pCList->nCount;
    if ( nCnt-- )
    {
        pCList->nCurrent = nCnt;
        pRet = &pCList->mHd[ nCnt ];
    }
    return pRet;
}

sal_Bool DffRecordManager::SeekToContent( SvStream& rIn, sal_uInt16 nRecId, DffSeekToContentMode eMode )
{
    DffRecordHeader* pHd = GetRecordHeader( nRecId, eMode );
    if ( pHd )
    {
        pHd->SeekToContent( rIn );
        return sal_True;
    }
    else
        return sal_False;
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
                pHd = NULL;
        }
    }
    if ( !pHd )
    {
        pCList = pOldList;
        pOldList->nCurrent = nOldCurrent;
    }
    return pHd;
}

//---------------------------------------------------------------------------
//  private methods
//---------------------------------------------------------------------------

bool CompareSvxMSDffShapeInfoById::operator() (
    ::boost::shared_ptr<SvxMSDffShapeInfo> const& lhs,
    ::boost::shared_ptr<SvxMSDffShapeInfo> const& rhs) const
{
    return lhs->nShapeId < rhs->nShapeId;
}

bool CompareSvxMSDffShapeInfoByTxBxComp::operator() (
    ::boost::shared_ptr<SvxMSDffShapeInfo> const& lhs,
    ::boost::shared_ptr<SvxMSDffShapeInfo> const& rhs) const
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
    Fraction aFact( GetMapFactor( MAP_POINT, eMap ).X() );
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
        Fraction aFact( GetMapFactor(MAP_INCH, eMap).X() );
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
        aFact=GetMapFactor(MAP_100TH_MM,eMap).X();
        nMul=aFact.GetNumerator();
        nDiv=aFact.GetDenominator()*360;
        aFact=Fraction(nMul,nDiv); // try again to shorten it
        // For 100TH_MM ->                            1/360
        // For Twip     -> 14,40/(25,4*360)=144/91440=1/635
        nEmuMul=aFact.GetNumerator();
        nEmuDiv=aFact.GetDenominator();

        // And something for typographic Points
        aFact=GetMapFactor(MAP_POINT,eMap).X();
        nPntMul=aFact.GetNumerator();
        nPntDiv=aFact.GetDenominator();
    }
    else
    {
        pModel = 0;
        nMapMul = nMapDiv = nMapXOfs = nMapYOfs = nEmuMul = nEmuDiv = nPntMul = nPntDiv = 0;
        bNeedMap = sal_False;
    }
}

sal_Bool SvxMSDffManager::SeekToShape( SvStream& rSt, void* /* pClientData */, sal_uInt32 nId ) const
{
    sal_Bool bRet = sal_False;
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
                rSt >> aEscherF002Hd;
                sal_uLong nEscherF002End = aEscherF002Hd.GetRecEndFilePos();
                DffRecordHeader aEscherObjListHd;
                while (rSt.good() && rSt.Tell() < nEscherF002End)
                {
                    rSt >> aEscherObjListHd;
                    if ( aEscherObjListHd.nRecVer != 0xf )
                        aEscherObjListHd.SeekToEndOfRecord( rSt );
                    else if ( aEscherObjListHd.nRecType == DFF_msofbtSpContainer )
                    {
                        DffRecordHeader aShapeHd;
                        if ( SeekToRec( rSt, DFF_msofbtSp, aEscherObjListHd.GetRecEndFilePos(), &aShapeHd ) )
                        {
                            rSt >> nShapeId;
                            if ( nId == nShapeId )
                            {
                                aEscherObjListHd.SeekToBegOfRecord( rSt );
                                bRet = sal_True;
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

bool SvxMSDffManager::SeekToRec( SvStream& rSt, sal_uInt16 nRecId, sal_uLong nMaxFilePos, DffRecordHeader* pRecHd, sal_uLong nSkipCount ) const
{
    bool bRet = sal_False;
    sal_uLong nFPosMerk = rSt.Tell(); // store FilePos to restore it later if necessary
    DffRecordHeader aHd;
    do
    {
        rSt >> aHd;
        if (!rSt.good())
            break;
        if (aHd.nRecLen > nMaxLegalDffRecordLength)
            break;
        if ( aHd.nRecType == nRecId )
        {
            if ( nSkipCount )
                nSkipCount--;
            else
            {
                bRet = sal_True;
                if ( pRecHd != NULL )
                    *pRecHd = aHd;
                else
                    aHd.SeekToBegOfRecord( rSt );
            }
        }
        if ( !bRet )
        {
            bool bSeekSuccess = aHd.SeekToEndOfRecord( rSt );
            if (!bSeekSuccess)
                break;
        }
    }
    while ( rSt.good() && rSt.Tell() < nMaxFilePos && !bRet );
    if ( !bRet )
        rSt.Seek( nFPosMerk );  // restore original FilePos
    return bRet;
}

bool SvxMSDffManager::SeekToRec2( sal_uInt16 nRecId1, sal_uInt16 nRecId2, sal_uLong nMaxFilePos, DffRecordHeader* pRecHd, sal_uLong nSkipCount ) const
{
    bool bRet = sal_False;
    sal_uLong nFPosMerk = rStCtrl.Tell();   // remember FilePos for conditionally later restauration
    DffRecordHeader aHd;
    do
    {
        rStCtrl >> aHd;
        if ( aHd.nRecType == nRecId1 || aHd.nRecType == nRecId2 )
        {
            if ( nSkipCount )
                nSkipCount--;
            else
            {
                bRet = sal_True;
                if ( pRecHd )
                    *pRecHd = aHd;
                else
                    aHd.SeekToBegOfRecord( rStCtrl );
            }
        }
        if ( !bRet )
            aHd.SeekToEndOfRecord( rStCtrl );
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
    return sal_True;
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
                case mso_syscolorInfoText :             aColor = rStyleSettings.GetInfoTextColor(); break;
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
                    if ( GetPropertyValue( DFF_Prop_fNoLineDrawDash ) & 8 )
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
                case 0x04 :     // substract grey level RGB(p,p,p)
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
                case 0x05 :     // substract from gray level RGB(p,p,p)
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
    {   // case of nUpper == 4 powerpoint takes this as agrument for a colorschemecolor
        GetColorFromPalette( nUpper, aColor );
    }
    else    // attributed hard, maybe with hint to SYSTEMRGB
        aColor = Color( (sal_uInt8)nColorCode, (sal_uInt8)( nColorCode >> 8 ), (sal_uInt8)( nColorCode >> 16 ) );
    return aColor;
}

void SvxMSDffManager::ReadObjText( SvStream& rStream, SdrObject* pObj )
{
    DffRecordHeader aRecHd;
    rStream >> aRecHd;
    if( aRecHd.nRecType == DFF_msofbtClientTextbox || aRecHd.nRecType == 0x1022 )
    {
        while( rStream.GetError() == 0 && rStream.Tell() < aRecHd.GetRecEndFilePos() )
        {
            DffRecordHeader aHd;
            rStream >> aHd;
            switch( aHd.nRecType )
            {
                case DFF_PST_TextBytesAtom:
                case DFF_PST_TextCharsAtom:
                    {
                        bool bUniCode = ( aHd.nRecType == DFF_PST_TextCharsAtom );
                        sal_uInt32 nBytes = aHd.nRecLen;
                        String aStr = MSDFFReadZString( rStream, nBytes, bUniCode );
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
// paragraphs. If I now take a look at the follwing code I get the impression that
// our outliner is too complicate to be used properly,
void SvxMSDffManager::ReadObjText( const OUString& rText, SdrObject* pObj )
{
    SdrTextObj* pText = PTR_CAST( SdrTextObj, pObj );
    if ( pText )
    {
        SdrOutliner& rOutliner = pText->ImpGetDrawOutliner();
        rOutliner.Init( OUTLINERMODE_TEXTOBJECT );

        sal_Bool bOldUpdateMode = rOutliner.GetUpdateMode();
        rOutliner.SetUpdateMode( sal_False );
        rOutliner.SetVertical( pText->IsVerticalWriting() );

        sal_Int32 nParaIndex = 0;
        sal_Int32 nParaSize;
        const sal_Unicode* pCurrent, *pBuf = rText.getStr();
        const sal_Unicode* pEnd = rText.getStr() + rText.getLength();

        while( pBuf < pEnd )
        {
            pCurrent = pBuf;

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
            rOutliner.Insert( aParagraph, nParaIndex, 0 );
            rOutliner.SetParaAttribs( nParaIndex, rOutliner.GetEmptyItemSet() );

            SfxItemSet aParagraphAttribs( rOutliner.GetEmptyItemSet() );
            if ( !aSelection.nStartPos )
                aParagraphAttribs.Put( SfxBoolItem( EE_PARA_BULLETSTATE, sal_False ) );
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

static Size lcl_GetPrefSize(const Graphic& rGraf, MapMode aWanted)
{
    MapMode aPrefMapMode(rGraf.GetPrefMapMode());
    if (aPrefMapMode == aWanted)
        return rGraf.GetPrefSize();
    Size aRetSize;
    if (aPrefMapMode == MAP_PIXEL)
    {
        aRetSize = Application::GetDefaultDevice()->PixelToLogic(
            rGraf.GetPrefSize(), aWanted);
    }
    else
    {
        aRetSize = Application::GetDefaultDevice()->LogicToLogic(
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
            aCropSize = lcl_GetPrefSize( rGraf, MAP_100TH_MM );
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
    SdrObject*  pRet = NULL;
    OUString    aFileName;
    OUString    aLinkFileName, aLinkFilterName;
    Rectangle   aVisArea;

    MSO_BlipFlags eFlags = (MSO_BlipFlags)GetPropertyValue( DFF_Prop_pibFlags, mso_blipflagDefault );
    sal_uInt32 nBlipId = GetPropertyValue( DFF_Prop_pib, 0 );
    sal_Bool bGrfRead = sal_False,

    // Graphic linked
    bLinkGrf = 0 != ( eFlags & mso_blipflagLinkToFile );
    {
        Graphic aGraf;  // be sure this graphic is deleted before swapping out
        if( SeekToContent( DFF_Prop_pibName, rSt ) )
            aFileName = MSDFFReadZString( rSt, GetPropertyValue( DFF_Prop_pibName ), sal_True );

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
                rSt >> aHd;
                if( DFF_msofbtBSE == aHd.nRecType )
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
            // the writer is doing it's own cropping, so this part affects only impress and calc
            if ( GetSvxMSDffSettings() & SVXMSDFF_SETTINGS_CROP_BITMAPS )
                lcl_ApplyCropping( *this, ( rObjData.nSpFlags & SP_FOLESHAPE ) == 0 ? &rSet : NULL, aGraf );

            if ( IsProperty( DFF_Prop_pictureTransparent ) )
            {
                sal_uInt32 nTransColor = GetPropertyValue( DFF_Prop_pictureTransparent, 0 );

                if ( aGraf.GetType() == GRAPHIC_BITMAP )
                {
                    BitmapEx    aBitmapEx( aGraf.GetBitmapEx() );
                    Bitmap      aBitmap( aBitmapEx.GetBitmap() );
                    Bitmap      aMask( aBitmap.CreateMask( MSO_CLR_ToColor( nTransColor, DFF_Prop_pictureTransparent ), 9 ) );
                    if ( aBitmapEx.IsTransparent() )
                        aMask.CombineSimple( aBitmapEx.GetMask(), BMP_COMBINE_OR );
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
            switch ( GetPropertyValue( DFF_Prop_pictureActive ) & 6 )
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
                if ( ( rObjData.nSpFlags & SP_FOLESHAPE ) == 0 )
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
                        case GRAPHIC_BITMAP :
                        {
                            BitmapEx    aBitmapEx( aGraf.GetBitmapEx() );
                            if ( nBrightness || nContrast || ( nGamma != 0x10000 ) )
                                aBitmapEx.Adjust( nBrightness, (sal_Int16)nContrast, 0, 0, 0, (double)nGamma / 0x10000, sal_False );
                            if ( eDrawMode == GRAPHICDRAWMODE_GREYS )
                                aBitmapEx.Convert( BMP_CONVERSION_8BIT_GREYS );
                            else if ( eDrawMode == GRAPHICDRAWMODE_MONO )
                                aBitmapEx.Convert( BMP_CONVERSION_1BIT_THRESHOLD );
                            aGraf = aBitmapEx;

                        }
                        break;

                        case GRAPHIC_GDIMETAFILE :
                        {
                            GDIMetaFile aGdiMetaFile( aGraf.GetGDIMetaFile() );
                            if ( nBrightness || nContrast || ( nGamma != 0x10000 ) )
                                aGdiMetaFile.Adjust( nBrightness, (sal_Int16)nContrast, 0, 0, 0, (double)nGamma / 0x10000, sal_False );
                            if ( eDrawMode == GRAPHICDRAWMODE_GREYS )
                                aGdiMetaFile.Convert( MTF_CONVERSION_8BIT_GREYS );
                            else if ( eDrawMode == GRAPHICDRAWMODE_MONO )
                                aGdiMetaFile.Convert( MTF_CONVERSION_1BIT_THRESHOLD );
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
            pRet = ImportOLE( GetPropertyValue( DFF_Prop_pictureId ), aGraf, rObjData.aBoundRect, aVisArea, rObjData.nCalledByGroup, nAspect );
        }
        if( !pRet )
        {
            pRet = new SdrGrafObj;
            if( bGrfRead )
                ((SdrGrafObj*)pRet)->SetGraphic( aGraf );

            if( bLinkGrf && !bGrfRead )     // sj: #i55484# if the graphic was embedded ( bGrfRead == true ) then
            {                               // we do not need to set a link. TODO: not to lose the information where the graphic is linked from
                INetURLObject aAbsURL;
                if ( !INetURLObject( maBaseURL ).GetNewAbsURL( aFileName, &aAbsURL ) )
                {
                    OUString aValidURL;
                    if( ::utl::LocalFileHelper::ConvertPhysicalNameToURL( aFileName, aValidURL ) )
                        aAbsURL = INetURLObject( aValidURL );
                }
                if( aAbsURL.GetProtocol() != INET_PROT_NOT_VALID )
                {
                    GraphicFilter &rGrfFilter = GraphicFilter::GetGraphicFilter();
                    aLinkFilterName = rGrfFilter.GetImportFormatName(
                        rGrfFilter.GetImportFormatNumberForShortName( aAbsURL.getExtension() ) );
                    aLinkFileName = aAbsURL.GetMainURL( INetURLObject::DECODE_TO_IURI );
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

    if ( pRet->ISA( SdrGrafObj ) )
    {
        if( aLinkFileName.getLength() )
            ((SdrGrafObj*)pRet)->SetGraphicLink( aLinkFileName, aLinkFilterName );

        if ( bLinkGrf && !bGrfRead )
        {
            ((SdrGrafObj*)pRet)->ForceSwapIn();
            Graphic aGraf(((SdrGrafObj*)pRet)->GetGraphic());
            lcl_ApplyCropping( *this, &rSet, aGraf );
        }
        ((SdrGrafObj*)pRet)->ForceSwapOut();
    }

    return pRet;
}

// PptSlidePersistEntry& rPersistEntry, SdPage* pPage
SdrObject* SvxMSDffManager::ImportObj( SvStream& rSt, void* pClientData,
    Rectangle& rClientRect, const Rectangle& rGlobalChildRect, int nCalledByGroup, sal_Int32* pShapeId )
{
    SdrObject* pRet = NULL;
    DffRecordHeader aObjHd;
    rSt >> aObjHd;
    if ( aObjHd.nRecType == DFF_msofbtSpgrContainer )
    {
        pRet = ImportGroup( aObjHd, rSt, pClientData, rClientRect, rGlobalChildRect, nCalledByGroup, pShapeId );
    }
    else if ( aObjHd.nRecType == DFF_msofbtSpContainer )
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
    SdrObject* pRet = NULL;

    if( pShapeId )
        *pShapeId = 0;

    if (!rHd.SeekToContent(rSt))
        return pRet;

    DffRecordHeader aRecHd;     // the first atom has to be the SpContainer for the GroupObject
    rSt >> aRecHd;
    if ( aRecHd.nRecType == DFF_msofbtSpContainer )
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
                Size aNewSize( aClientRect.GetHeight(), aClientRect.GetWidth() );
                Rectangle aNewRect( aTopLeft, aNewSize );
                aClientRect = aNewRect;
            }

            // now importing the inner objects of the group
            if (!aRecHd.SeekToEndOfRecord(rSt))
                return pRet;

            while ( ( rSt.GetError() == 0 ) && ( rSt.Tell() < rHd.GetRecEndFilePos() ) )
            {
                DffRecordHeader aRecHd2;
                rSt >> aRecHd2;
                if ( aRecHd2.nRecType == DFF_msofbtSpgrContainer )
                {
                    Rectangle aGroupClientAnchor, aGroupChildAnchor;
                    GetGroupAnchors( aRecHd2, rSt, aGroupClientAnchor, aGroupChildAnchor, aClientRect, aGlobalChildRect );
                    if (!aRecHd2.SeekToBegOfRecord(rSt))
                        return pRet;
                    sal_Int32 nShapeId;
                    SdrObject* pTmp = ImportGroup( aRecHd2, rSt, pClientData, aGroupClientAnchor, aGroupChildAnchor, nCalledByGroup + 1, &nShapeId );
                    if ( pTmp && pRet && ((SdrObjGroup*)pRet)->GetSubList() )
                    {
                        ((SdrObjGroup*)pRet)->GetSubList()->NbcInsertObject( pTmp );
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
                    if ( pTmp && pRet && ((SdrObjGroup*)pRet)->GetSubList())
                    {
                        ((SdrObjGroup*)pRet)->GetSubList()->NbcInsertObject( pTmp );
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
    return pRet;
}

SdrObject* SvxMSDffManager::ImportShape( const DffRecordHeader& rHd, SvStream& rSt, void* pClientData,
                                            Rectangle& rClientRect, const Rectangle& rGlobalChildRect,
                                            int nCalledByGroup, sal_Int32* pShapeId )
{
    SdrObject* pRet = NULL;

    if( pShapeId )
        *pShapeId = 0;

    if (!rHd.SeekToBegOfRecord(rSt))
        return pRet;

    DffObjData aObjData( rHd, rClientRect, nCalledByGroup );
    aObjData.bRotateTextWithShape = ( GetSvxMSDffSettings() & SVXMSDFF_SETTINGS_IMPORT_EXCEL ) == 0;
    maShapeRecords.Consume( rSt, sal_False );
    if( maShapeRecords.SeekToContent( rSt,
        DFF_msofbtUDefProp,
        SEEK_FROM_BEGINNING ) )
    {
        sal_uInt32  nBytesLeft = maShapeRecords.Current()->nRecLen;
        sal_uInt32  nUDData;
        sal_uInt16  nPID;
        while( 5 < nBytesLeft )
        {
            rSt >> nPID;
            if ( rSt.GetError() != 0 )
                break;
            rSt >> nUDData;
            if ( rSt.GetError() != 0 )
                break;
            if ( nPID == 447 ) //
            {
                mbRotateGranientFillWithAngle = nUDData & 0x20;
                break;
            }
            nBytesLeft  -= 6;
        }
    }
    aObjData.bShapeType = maShapeRecords.SeekToContent( rSt, DFF_msofbtSp, SEEK_FROM_BEGINNING );
    if ( aObjData.bShapeType )
    {
        rSt >> aObjData.nShapeId
            >> aObjData.nSpFlags;
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
        ( (DffPropertyReader*) this )->mnFix16Angle = 0;
    }

    aObjData.bOpt2 = maShapeRecords.SeekToContent( rSt, DFF_msofbtUDefProp, SEEK_FROM_CURRENT_AND_RESTART );
    if ( aObjData.bOpt2 )
    {
        maShapeRecords.Current()->SeekToBegOfRecord( rSt );
        delete pSecPropSet;
        pSecPropSet = new DffPropertyReader( *this );
        pSecPropSet->ReadPropSet( rSt, NULL );
    }

    aObjData.bChildAnchor = maShapeRecords.SeekToContent( rSt, DFF_msofbtChildAnchor, SEEK_FROM_CURRENT_AND_RESTART );
    if ( aObjData.bChildAnchor )
    {
        sal_Int32 l, o, r, u;
        rSt >> l >> o >> r >> u;
        Scale( l );
        Scale( o );
        Scale( r );
        Scale( u );
        aObjData.aChildAnchor = Rectangle( l, o, r, u );
        if ( !rGlobalChildRect.IsEmpty() && !rClientRect.IsEmpty() && rGlobalChildRect.GetWidth() && rGlobalChildRect.GetHeight() )
        {
            double fl = l;
            double fo = o;
            double fWidth = r - l;
            double fHeight= u - o;
            double fXScale = (double)rClientRect.GetWidth() / (double)rGlobalChildRect.GetWidth();
            double fYScale = (double)rClientRect.GetHeight() / (double)rGlobalChildRect.GetHeight();
            fl = ( ( l - rGlobalChildRect.Left() ) * fXScale ) + rClientRect.Left();
            fo = ( ( o - rGlobalChildRect.Top()  ) * fYScale ) + rClientRect.Top();
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
            nGroupShapeFlags = aObjData.nSpFlags;       // #73013#
        }
        else if ( ( aObjData.eShapeType != mso_sptNil ) || IsProperty( DFF_Prop_pVertices ) || bGraphic )
        {
            SfxItemSet  aSet( pSdrModel->GetItemPool() );

            sal_Bool    bIsConnector = ( ( aObjData.eShapeType >= mso_sptStraightConnector1 ) && ( aObjData.eShapeType <= mso_sptCurvedConnector5 ) );
            sal_Int32   nObjectRotation = mnFix16Angle;
            sal_uInt32  nSpFlags = aObjData.nSpFlags;

            if ( bGraphic )
            {
                pRet = ImportGraphic( rSt, aSet, aObjData );        // SJ: #68396# is no longer true (fixed in ppt2000)
                ApplyAttributes( rSt, aSet, aObjData );
                pRet->SetMergedItemSet(aSet);
            }
            else if ( aObjData.eShapeType == mso_sptLine && !( GetPropertyValue( DFF_Prop_fc3DLightFace ) & 8 ) )
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

                    sal_Bool bIsFontwork = ( GetPropertyValue( DFF_Prop_gtextFStrikethrough, 0 ) & 0x4000 ) != 0;

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

                            aFontName = MSDFFReadZString( rSt, GetPropertyValue( DFF_Prop_gtextFont ), sal_True );
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

                        // SJ TODO: Vertical Writing is not correct, instead this should be
                        // replaced through "CharacterRotation" by 90°, therefore a new Item has to be
                        // supported by svx core, api and xml file format
                        ((SdrObjCustomShape*)pRet)->SetVerticalWriting( ( GetPropertyValue( DFF_Prop_gtextFStrikethrough, 0 ) & 0x2000 ) != 0 );

                        if ( SeekToContent( DFF_Prop_gtextUNICODE, rSt ) )
                        {
                            aObjectText = MSDFFReadZString( rSt, GetPropertyValue( DFF_Prop_gtextUNICODE ), sal_True );
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

                            SdrFitToSizeType eFTS = SDRTEXTFIT_NONE;
                            if ( eGeoTextAlign == mso_alignTextStretch )
                                eFTS = SDRTEXTFIT_ALLLINES;
                            aSet.Put( SdrTextFitToSizeTypeItem( eFTS ) );
                        }
                        if ( IsProperty( DFF_Prop_gtextSpacing ) )
                        {
                            sal_Int32 nTextWidth = GetPropertyValue( DFF_Prop_gtextSpacing, 100 < 16 ) / 655;
                            if ( nTextWidth != 100 )
                                aSet.Put( SvxCharScaleWidthItem( (sal_uInt16)nTextWidth, EE_CHAR_FONTWIDTH ) );
                        }
                        if ( GetPropertyValue( DFF_Prop_gtextFStrikethrough, 0 ) & 0x1000 ) // SJ: Font Kerning On ?
                            aSet.Put( SvxKerningItem( 1, EE_CHAR_KERNING ) );

                        // #i119496# the resize autoshape to fit text attr of word art in MS PPT is always false
                        aSet.Put(SdrTextAutoGrowHeightItem(false));
                        aSet.Put(SdrTextAutoGrowWidthItem(false));
                    }
                    pRet->SetMergedItemSet( aSet );

                    // sj: taking care of rtl, ltr. In case of fontwork mso. seems not to be able to set
                    // proper text directions, instead the text default is depending to the string.
                    // so we have to calculate the a text direction from string:
                    if ( bIsFontwork )
                    {
                        OutlinerParaObject* pParaObj = ((SdrObjCustomShape*)pRet)->GetOutlinerParaObject();
                        if ( pParaObj )
                        {
                            SdrOutliner& rOutliner = ((SdrObjCustomShape*)pRet)->ImpGetDrawOutliner();
                            sal_Bool bOldUpdateMode = rOutliner.GetUpdateMode();
                            SdrModel* pModel = pRet->GetModel();
                            if ( pModel )
                                rOutliner.SetStyleSheetPool( (SfxStyleSheetPool*)pModel->GetStyleSheetPool() );
                            rOutliner.SetUpdateMode( sal_False );
                            rOutliner.SetText( *pParaObj );
                            VirtualDevice aVirDev( 1 );
                            aVirDev.SetMapMode( MAP_100TH_MM );
                            sal_Int32 i, nParagraphs = rOutliner.GetParagraphCount();
                            if ( nParagraphs )
                            {
                                sal_Bool bCreateNewParaObject = sal_False;
                                for ( i = 0; i < nParagraphs; i++ )
                                {
                                    sal_Bool bIsRTL = aVirDev.GetTextIsRTL( rOutliner.GetText( rOutliner.GetParagraph( i ) ), 0, STRING_LEN );
                                    if ( bIsRTL )
                                    {
                                        SfxItemSet aSet2( rOutliner.GetParaAttribs( i ) );
                                        aSet2.Put( SvxFrameDirectionItem( FRMDIR_HORI_RIGHT_TOP, EE_PARA_WRITINGDIR ) );
                                        rOutliner.SetParaAttribs( i, aSet2 );
                                        bCreateNewParaObject = sal_True;
                                    }
                                }
                                if  ( bCreateNewParaObject )
                                {
                                    OutlinerParaObject* pNewText = rOutliner.CreateParaObject();
                                    rOutliner.Init( OUTLINERMODE_TEXTOBJECT );
                                    ((SdrObjCustomShape*)pRet)->NbcSetOutlinerParaObject( pNewText );
                                }
                            }
                            rOutliner.Clear();
                            rOutliner.SetUpdateMode( bOldUpdateMode );
                        }
                    }

                    // mso_sptArc special treating:
                    // sj: since we actually can't render the arc because of its weird SnapRect settings,
                    // we will create a new CustomShape, that can be saved/loaded without problems.
                    // We will change the shape type, so this code applys only if importing arcs from msoffice.
                    if ( aObjData.eShapeType == mso_sptArc )
                    {
                        const OUString sAdjustmentValues( "AdjustmentValues" );
                        const OUString sCoordinates( "Coordinates" );
                        const OUString sHandles( "Handles" );
                        const OUString sEquations( "Equations" );
                        const OUString sViewBox( "ViewBox" );
                        const OUString sPath( "Path" );
                        const OUString sTextFrames( "TextFrames" );
                        SdrCustomShapeGeometryItem aGeometryItem( (SdrCustomShapeGeometryItem&)((SdrObjCustomShape*)pRet)->GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY ) );
                        com::sun::star::uno::Sequence< com::sun::star::drawing::EnhancedCustomShapeParameterPair> seqCoordinates;
                        com::sun::star::uno::Sequence< com::sun::star::drawing::EnhancedCustomShapeAdjustmentValue > seqAdjustmentValues;

                        // before clearing the GeometryItem we have to store the current Coordinates
                        const uno::Any* pAny = ((SdrCustomShapeGeometryItem&)aGeometryItem).GetPropertyValueByName( sPath, sCoordinates );
                        Rectangle aPolyBoundRect;
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
                            double fNumber;
                            if ( seqAdjustmentValues[ 0 ].State == com::sun::star::beans::PropertyState_DIRECT_VALUE )
                            {
                                seqAdjustmentValues[ 0 ].Value >>= fNumber;
                                nEndAngle = NormAngle360( - (sal_Int32)fNumber * 100 );
                            }
                            else
                            {
                                fNumber = 270.0;
                                seqAdjustmentValues[ 0 ].Value <<= fNumber;
                                seqAdjustmentValues[ 0 ].State = com::sun::star::beans::PropertyState_DIRECT_VALUE;     // so this value will properly be stored
                            }

                            if ( seqAdjustmentValues[ 1 ].State == com::sun::star::beans::PropertyState_DIRECT_VALUE )
                            {
                                seqAdjustmentValues[ 1 ].Value >>= fNumber;
                                nStartAngle = NormAngle360( - (sal_Int32)fNumber * 100 );
                            }
                            else
                            {
                                fNumber = 0.0;
                                seqAdjustmentValues[ 1 ].Value <<= fNumber;
                                seqAdjustmentValues[ 1 ].State = com::sun::star::beans::PropertyState_DIRECT_VALUE;
                            }

                            PropertyValue aPropVal;
                            aPropVal.Name = sAdjustmentValues;
                            aPropVal.Value <<= seqAdjustmentValues;
                            aGeometryItem.SetPropertyValue( aPropVal );     // storing the angle attribute
                        }
                        if ( nStartAngle != nEndAngle )
                        {
                            XPolygon aXPoly( aPolyBoundRect.Center(), aPolyBoundRect.GetWidth() / 2, aPolyBoundRect.GetHeight() / 2,
                                (sal_uInt16)nStartAngle / 10, (sal_uInt16)nEndAngle / 10, sal_True );
                            Rectangle aPolyPieRect( aXPoly.GetBoundRect() );

                            double  fYScale, fXScale;
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

                            fXScale = (double)aPolyBoundRect.GetWidth() / (double)aPolyPieRect.GetWidth();
                            fYScale = (double)aPolyBoundRect.GetHeight() / (double)aPolyPieRect.GetHeight();

                            Rectangle aOldBoundRect( aObjData.aBoundRect );
                            aObjData.aBoundRect = Rectangle( Point( aLogicRect.Left() + (sal_Int32)fXOfs, aLogicRect.Top() + (sal_Int32)fYOfs ),
                                 Size( (sal_Int32)( aLogicRect.GetWidth() * fXScale ), (sal_Int32)( aLogicRect.GetHeight() * fYScale ) ) );

                            // creating the text frame -> scaling into (0,0),(21600,21600) destination coordinate system
                            double fTextFrameScaleX = (double)21600 / (double)aPolyBoundRect.GetWidth();
                            double fTextFrameScaleY = (double)21600 / (double)aPolyBoundRect.GetHeight();
                            sal_Int32 nLeft  = (sal_Int32)(( aPolyPieRect.Left()  - aPolyBoundRect.Left() ) * fTextFrameScaleX );
                            sal_Int32 nTop   = (sal_Int32)(( aPolyPieRect.Top()   - aPolyBoundRect.Top() )  * fTextFrameScaleY );
                            sal_Int32 nRight = (sal_Int32)(( aPolyPieRect.Right() - aPolyBoundRect.Left() ) * fTextFrameScaleX );
                            sal_Int32 nBottom= (sal_Int32)(( aPolyPieRect.Bottom()- aPolyBoundRect.Top() )  * fTextFrameScaleY );
                            com::sun::star::uno::Sequence< com::sun::star::drawing::EnhancedCustomShapeTextFrame > aTextFrame( 1 );
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
                        ((SdrObjCustomShape*)pRet)->SetMergedItem( aGeometryItem );
                        ((SdrObjCustomShape*)pRet)->MergeDefaultAttributes();

                        // now setting a new name, so the above correction is only done once when importing from ms
                        SdrCustomShapeGeometryItem aGeoName( (SdrCustomShapeGeometryItem&)((SdrObjCustomShape*)pRet)->GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY ) );
                        const OUString sType( "Type" );
                        const OUString sName( "mso-spt100" );
                        PropertyValue aPropVal;
                        aPropVal.Name = sType;
                        aPropVal.Value <<= sName;
                        aGeoName.SetPropertyValue( aPropVal );
                        ((SdrObjCustomShape*)pRet)->SetMergedItem( aGeoName );
                    }
                    else
                        ((SdrObjCustomShape*)pRet)->MergeDefaultAttributes();

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
                        basegfx::B2DPolyPolygon aPoly( SdrObjCustomShape::GetLineGeometry( (SdrObjCustomShape*)pRet, sal_True ) );
                        SdrObject::Free( pRet );

                        pRet = new SdrEdgeObj();
                        ApplyAttributes( rSt, aSet, aObjData );
                        pRet->SetLogicRect( aObjData.aBoundRect );
                        pRet->SetMergedItemSet(aSet);

                        // connectors
                        MSO_ConnectorStyle eConnectorStyle = (MSO_ConnectorStyle)GetPropertyValue( DFF_Prop_cxstyle, mso_cxstyleStraight );

                        ((SdrEdgeObj*)pRet)->ConnectToNode(sal_True, NULL);
                        ((SdrEdgeObj*)pRet)->ConnectToNode(sal_False, NULL);

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
                                aSet.Put( SdrEdgeKindItem( SDREDGE_ORTHOLINES ) );
                                n1HorzDist = n1VertDist = n2HorzDist = n2VertDist = 630;
                            }
                            break;
                            case mso_cxstyleCurved:
                                aSet.Put( SdrEdgeKindItem( SDREDGE_BEZIER ) );
                            break;
                            default: // mso_cxstyleStraight || mso_cxstyleNone
                                aSet.Put( SdrEdgeKindItem( SDREDGE_ONELINE ) );
                            break;
                        }
                        aSet.Put( SdrEdgeNode1HorzDistItem( n1HorzDist ) );
                        aSet.Put( SdrEdgeNode1VertDistItem( n1VertDist ) );
                        aSet.Put( SdrEdgeNode2HorzDistItem( n2HorzDist ) );
                        aSet.Put( SdrEdgeNode2VertDistItem( n2VertDist ) );

                        ((SdrEdgeObj*)pRet)->SetEdgeTrackPath( aPoly );
                        pRet->SetMergedItemSet( aSet );
                    }
                    if ( aObjData.eShapeType == mso_sptLine )
                    {
                        pRet->SetMergedItemSet(aSet);
                        ((SdrObjCustomShape*)pRet)->MergeDefaultAttributes();
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
        sal_Int32 nGroupProperties( GetPropertyValue( DFF_Prop_fPrint ) );
        pRet->SetVisible( ( nGroupProperties & 2 ) == 0 );
        pRet->SetPrintable( ( nGroupProperties & 1 ) != 0 );
    }

    //Import alt text as description
    if ( pRet && SeekToContent( DFF_Prop_wzDescription, rSt ) )
    {
        OUString aAltText = MSDFFReadZString(rSt, GetPropertyValue(DFF_Prop_wzDescription), true);
        pRet->SetDescription( aAltText );
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
        rSt >> aShapeHd;
        if ( ( aShapeHd.nRecType == DFF_msofbtSpContainer ) ||
                ( aShapeHd.nRecType == DFF_msofbtSpgrContainer ) )
        {
            DffRecordHeader aShapeHd2( aShapeHd );
            if ( aShapeHd.nRecType == DFF_msofbtSpgrContainer )
                rSt >> aShapeHd2;
            while( ( rSt.GetError() == 0 ) && ( rSt.Tell() < aShapeHd2.GetRecEndFilePos() ) )
            {
                DffRecordHeader aShapeAtom;
                rSt >> aShapeAtom;

                if ( aShapeAtom.nRecType == DFF_msofbtClientAnchor )
                {
                    if ( GetSvxMSDffSettings() & SVXMSDFF_SETTINGS_IMPORT_PPT )
                    {
                        sal_Int32 l, t, r, b;
                        if ( aShapeAtom.nRecLen == 16 )
                        {
                            rSt >> l >> t >> r >> b;
                        }
                        else
                        {
                            sal_Int16 ls, ts, rs, bs;
                            rSt >> ts >> ls >> rs >> bs; // the order of coordinates is a bit strange...
                            l = ls, t = ts, r = rs, b = bs;
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
                    rSt >> l >> o >> r >> u;
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

    sal_Bool bFirst = sal_True;
    DffRecordHeader aShapeHd;
    while ( ( rSt.GetError() == 0 ) && ( rSt.Tell() < rHd.GetRecEndFilePos() ) )
    {
        rSt >> aShapeHd;
        if ( ( aShapeHd.nRecType == DFF_msofbtSpContainer ) ||
                ( aShapeHd.nRecType == DFF_msofbtSpgrContainer ) )
        {
            DffRecordHeader aShapeHd2( aShapeHd );
            if ( aShapeHd.nRecType == DFF_msofbtSpgrContainer )
                rSt >> aShapeHd2;
            while( ( rSt.GetError() == 0 ) && ( rSt.Tell() < aShapeHd2.GetRecEndFilePos() ) )
            {
                DffRecordHeader aShapeAtom;
                rSt >> aShapeAtom;
                if ( aShapeAtom.nRecType == DFF_msofbtChildAnchor )
                {
                    sal_Int32 l, o, r, u;
                    rSt >> l >> o >> r >> u;
                    Scale( l );
                    Scale( o );
                    Scale( r );
                    Scale( u );
                    Rectangle aChild( l, o, r, u );

                    if ( bFirst )
                    {
                        if ( !rGlobalChildRect.IsEmpty() && !rClientRect.IsEmpty() && rGlobalChildRect.GetWidth() && rGlobalChildRect.GetHeight() )
                        {
                            double fl = l;
                            double fo = o;
                            double fWidth = r - l;
                            double fHeight= u - o;
                            double fXScale = (double)rClientRect.GetWidth() / (double)rGlobalChildRect.GetWidth();
                            double fYScale = (double)rClientRect.GetHeight() / (double)rGlobalChildRect.GetHeight();
                            fl = ( ( l - rGlobalChildRect.Left() ) * fXScale ) + rClientRect.Left();
                            fo = ( ( o - rGlobalChildRect.Top()  ) * fYScale ) + rClientRect.Top();
                            fWidth *= fXScale;
                            fHeight *= fYScale;
                            rGroupClientAnchor = Rectangle( Point( (sal_Int32)fl, (sal_Int32)fo ), Size( (sal_Int32)( fWidth + 1 ), (sal_Int32)( fHeight + 1 ) ) );
                        }
                        bFirst = sal_False;
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
        SvxMSDffImportData& rImportData = *(SvxMSDffImportData*)pData;
        SvxMSDffImportRec* pImpRec = new SvxMSDffImportRec;
        SvxMSDffImportRec* pTextImpRec = pImpRec;

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
            sal_uInt32  nUDData;
            sal_uInt16  nPID;
            while( 5 < nBytesLeft )
            {
                rSt >> nPID;
                if ( rSt.GetError() != 0 )
                    break;
                rSt >> nUDData;
                switch( nPID )
                {
                    case 0x038F: pImpRec->nXAlign = nUDData; break;
                    case 0x0390:
                        if (pImpRec->pXRelTo)
                            delete pImpRec->pXRelTo;
                        pImpRec->pXRelTo = new sal_uInt32;
                        *(pImpRec->pXRelTo) = nUDData;
                        break;
                    case 0x0391: pImpRec->nYAlign = nUDData; break;
                    case 0x0392:
                        if (pImpRec->pYRelTo)
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
                    // overriden by something), so it probably matches o:hr from .docx.
                        pImpRec->isHorizontalRule = true;
                        break;
                }
                if ( rSt.GetError() != 0 )
                    break;
                pImpRec->bHasUDefProp = sal_True;
                nBytesLeft  -= 6;
            }
        }

        //  text frame, also Title or Outline
        SdrObject*  pOrgObj  = pObj;
        SdrRectObj* pTextObj = 0;
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
                //a) its a simple text object or
                //b) its a rectangle with text and square wrapping.
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
                pObj = pOrgObj = 0;
            }

            // Distance of Textbox to it's surrounding Customshape
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
                    DFF_Prop_txflTextFlow) & 0xFFFF);
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
                    case mso_txflHorzN:
                    default :
                        break;
                }
            }

            if (nTextRotationAngle)
            {
                while (nTextRotationAngle > 360000)
                    nTextRotationAngle-=9000;
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
                    default:
                        break;
                }
            }

            pTextObj = new SdrRectObj(OBJ_TEXT, rTextRect);
            pTextImpRec = new SvxMSDffImportRec(*pImpRec);

            // the vertical paragraph indents are part of the BoundRect,
            // here we 'remove' them by calculating
            Rectangle aNewRect(rTextRect);
            aNewRect.Bottom() -= nTextTop + nTextBottom;
            aNewRect.Right() -= nTextLeft + nTextRight;

            // Only if it's a simple textbox may Writer replace
            // the object with a frame, otherwise
            if( bTextFrame )
            {
                ::boost::shared_ptr<SvxMSDffShapeInfo> const pTmpRec(
                    new SvxMSDffShapeInfo(0, pImpRec->nShapeId));

                SvxMSDffShapeInfos_ById::const_iterator const it =
                    m_pShapeInfosById->find(pTmpRec);
                if (it != m_pShapeInfosById->end())
                {
                    SvxMSDffShapeInfo& rInfo = **it;
                    pTextImpRec->bReplaceByFly   = rInfo.bReplaceByFly;
                    pTextImpRec->bLastBoxInChain = rInfo.bLastBoxInChain;
                }
            }

            if( !pObj )
                ApplyAttributes( rSt, aSet, rObjData );

            bool bFitText = false;
            if (GetPropertyValue(DFF_Prop_FitTextToShape) & 2)
            {
                aSet.Put( SdrTextAutoGrowHeightItem( sal_True ) );
                aSet.Put( SdrTextMinFrameHeightItem(
                    aNewRect.Bottom() - aNewRect.Top() ) );
                aSet.Put( SdrTextMinFrameWidthItem(
                    aNewRect.Right() - aNewRect.Left() ) );
                bFitText = true;
            }
            else
            {
                aSet.Put( SdrTextAutoGrowHeightItem( sal_False ) );
                aSet.Put( SdrTextAutoGrowWidthItem( sal_False ) );
            }

            switch ( (MSO_WrapMode)
                GetPropertyValue( DFF_Prop_WrapText, mso_wrapSquare ) )
            {
                case mso_wrapNone :
                    aSet.Put( SdrTextAutoGrowWidthItem( sal_True ) );
                    if (bFitText)
                    {
                        //can't do autowidth in flys #i107184#
                        pTextImpRec->bReplaceByFly = false;
                    }
                break;
                case mso_wrapByPoints :
                    aSet.Put( SdrTextContourFrameItem( sal_True ) );
                break;
                default: break;
            }

            // set margins at the border of the textbox
            aSet.Put( SdrTextLeftDistItem( nTextLeft ) );
            aSet.Put( SdrTextRightDistItem( nTextRight ) );
            aSet.Put( SdrTextUpperDistItem( nTextTop ) );
            aSet.Put( SdrTextLowerDistItem( nTextBottom ) );
            pTextImpRec->nDxTextLeft    = nTextLeft;
            pTextImpRec->nDyTextTop     = nTextTop;
            pTextImpRec->nDxTextRight   = nTextRight;
            pTextImpRec->nDyTextBottom  = nTextBottom;

            // read text anchor
            if ( IsProperty( DFF_Prop_anchorText ) )
            {
                MSO_Anchor eTextAnchor =
                    (MSO_Anchor)GetPropertyValue( DFF_Prop_anchorText );

                SdrTextVertAdjust eTVA = SDRTEXTVERTADJUST_CENTER;
                sal_Bool bTVASet(sal_False);
                SdrTextHorzAdjust eTHA = SDRTEXTHORZADJUST_CENTER;
                sal_Bool bTHASet(sal_False);

                switch( eTextAnchor )
                {
                    case mso_anchorTop:
                    {
                        eTVA = SDRTEXTVERTADJUST_TOP;
                        bTVASet = sal_True;
                    }
                    break;
                    case mso_anchorTopCentered:
                    {
                        eTVA = SDRTEXTVERTADJUST_TOP;
                        bTVASet = sal_True;
                        bTHASet = sal_True;
                    }
                    break;

                    case mso_anchorMiddle:
                        bTVASet = sal_True;
                    break;
                    case mso_anchorMiddleCentered:
                    {
                        bTVASet = sal_True;
                        bTHASet = sal_True;
                    }
                    break;
                    case mso_anchorBottom:
                    {
                        eTVA = SDRTEXTVERTADJUST_BOTTOM;
                        bTVASet = sal_True;
                    }
                    break;
                    case mso_anchorBottomCentered:
                    {
                        eTVA = SDRTEXTVERTADJUST_BOTTOM;
                        bTVASet = sal_True;
                        bTHASet = sal_True;
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
                pTextObj->SetVerticalWriting(sal_True);

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

            const SfxPoolItem* pPoolItem=NULL;
            SfxItemState eState = aSet.GetItemState( XATTR_FILLCOLOR,
                                                     sal_False, &pPoolItem );
            if( SFX_ITEM_DEFAULT == eState )
                aSet.Put( XFillColorItem( OUString(),
                          Color( mnDefaultColor ) ) );
            pObj->SetMergedItemSet(aSet);
        }

        //Means that fBehindDocument is set
        if (GetPropertyValue(DFF_Prop_fPrint) & 0x20)
            pImpRec->bDrawHell = sal_True;
        else
            pImpRec->bDrawHell = sal_False;
        if (GetPropertyValue(DFF_Prop_fPrint) & 0x02)
            pImpRec->bHidden = sal_True;
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
            sal_uInt16 nNumElemVert, nNumElemMemVert, nElemSizeVert;
            rSt >> nNumElemVert >> nNumElemMemVert >> nElemSizeVert;
            if (nNumElemVert && ((nElemSizeVert == 8) || (nElemSizeVert == 4)))
            {
                pTextImpRec->pWrapPolygon = new Polygon(nNumElemVert);
                for (sal_uInt16 i = 0; i < nNumElemVert; ++i)
                {
                    sal_Int32 nX, nY;
                    if (nElemSizeVert == 8)
                        rSt >> nX >> nY;
                    else
                    {
                        sal_Int16 nSmallX, nSmallY;
                        rSt >> nSmallX >> nSmallY;
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

        pImpRec->bVFlip = (rObjData.nSpFlags & SP_FFLIPV) ? true : false;
        pImpRec->bHFlip = (rObjData.nSpFlags & SP_FFLIPH) ? true : false;

        sal_uInt32 nLineFlags = GetPropertyValue( DFF_Prop_fNoLineDrawDash );
        pImpRec->eLineStyle = (nLineFlags & 8)
                            ? (MSO_LineStyle)GetPropertyValue(
                                                DFF_Prop_lineStyle,
                                                mso_lineSimple )
                            : (MSO_LineStyle)USHRT_MAX;
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
                rImportData.aRecords.insert( pImpRec );
            }

            if( pTextObj && (pOrgObj != pTextObj) )
            {
                // Modify ShapeId (must be unique)
                pImpRec->nShapeId |= 0x8000000;
                pTextImpRec->pObj = pTextObj;
                rImportData.aRecords.insert( pTextImpRec );
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
        else
            delete pImpRec;
    }

    return pObj;
};

void SvxMSDffManager::StoreShapeOrder(sal_uLong         nId,
                                      sal_uLong         nTxBx,
                                      SdrObject*    pObject,
                                      SwFlyFrmFmt*  pFly,
                                      short         nHdFtSection) const
{
    sal_uInt16 nShpCnt = pShapeOrders->size();
    for (sal_uInt16 nShapeNum=0; nShapeNum < nShpCnt; nShapeNum++)
    {
        SvxMSDffShapeOrder& rOrder = *(*pShapeOrders)[ nShapeNum ];

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
                                            sal_uLong        nTxBx,
                                            SwFlyFrmFmt* pFly,
                                            SdrObject*   pObject) const
{
    sal_uInt16 nShpCnt = pShapeOrders->size();
    for (sal_uInt16 nShapeNum=0; nShapeNum < nShpCnt; nShapeNum++)
    {
        SvxMSDffShapeOrder& rOrder = *(*pShapeOrders)[ nShapeNum ];

        if( rOrder.pObj == pOldObject )
        {
            rOrder.pFly      = pFly;
            rOrder.pObj      = pObject;
            rOrder.nTxBxComp = nTxBx;
        }
    }
}


void SvxMSDffManager::RemoveFromShapeOrder( SdrObject* pObject ) const
{
    sal_uInt16 nShpCnt = pShapeOrders->size();
    for (sal_uInt16 nShapeNum=0; nShapeNum < nShpCnt; nShapeNum++)
    {
        SvxMSDffShapeOrder& rOrder = *(*pShapeOrders)[ nShapeNum ];

        if( rOrder.pObj == pObject )
        {
            rOrder.pObj      = 0;
            rOrder.pFly      = 0;
            rOrder.nTxBxComp = 0;
        }
    }
}




//---------------------------------------------------------------------------
//  exported class: Public Methods
//---------------------------------------------------------------------------

SvxMSDffManager::SvxMSDffManager(SvStream& rStCtrl_,
                                 const OUString& rBaseURL,
                                 sal_uInt32 nOffsDgg_,
                                 SvStream* pStData_,
                                 SdrModel* pSdrModel_,// see SetModel() below
                                 long      nApplicationScale,
                                 ColorData mnDefaultColor_,
                                 sal_uLong     nDefaultFontHeight_,
                                 SvStream* pStData2_ )
    :DffPropertyReader( *this ),
     pFormModel( NULL ),
     pBLIPInfos( new SvxMSDffBLIPInfos  ),
     m_pShapeInfosByTxBxComp( new SvxMSDffShapeInfos_ByTxBxComp ),
     pShapeOrders( new SvxMSDffShapeOrders ),
     nDefaultFontHeight( nDefaultFontHeight_),
     nOffsDgg( nOffsDgg_ ),
     nBLIPCount(  USHRT_MAX ),              // initialize with error, since we fist check if the
     nShapeCount( USHRT_MAX ),              // control stream has correct data
     nGroupShapeFlags(0),                   // ensure initialization here, as some corrupted
                                            // files may yield to this being unitialized
     maBaseURL( rBaseURL ),
     rStCtrl(  rStCtrl_  ),
     pStData(  pStData_  ),
     pStData2( pStData2_ ),
     nSvxMSDffSettings( 0 ),
     nSvxMSDffOLEConvFlags( 0 ),
     pSecPropSet( NULL ),
     mnDefaultColor( mnDefaultColor_),
     mbTracing( sal_False )
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
     pFormModel( NULL ),
     pBLIPInfos(   new SvxMSDffBLIPInfos  ),
     m_pShapeInfosByTxBxComp( new SvxMSDffShapeInfos_ByTxBxComp ),
     pShapeOrders( new SvxMSDffShapeOrders ),
     nDefaultFontHeight( 24 ),
     nOffsDgg( 0 ),
     nBLIPCount(  USHRT_MAX ),              // initialize with error, since we first have to check
     nShapeCount( USHRT_MAX ),              // whether the control stream contains the correct data
     maBaseURL( rBaseURL ),
     rStCtrl(  rStCtrl_  ),
     pStData( 0 ),
     pStData2( 0 ),
     nSvxMSDffSettings( 0 ),
     nSvxMSDffOLEConvFlags( 0 ),
     pSecPropSet( NULL ),
     mnDefaultColor( COL_DEFAULT ),
     mbTracing( sal_False )
{
    SetModel( NULL, 0 );
}

SvxMSDffManager::~SvxMSDffManager()
{
    delete pSecPropSet;
    delete pBLIPInfos;
    delete pShapeOrders;
    delete pFormModel;
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
    rSt >> aDgContHd;
    // insert this container only if there is also a DgAtom
    if ( SeekToRec( rSt, DFF_msofbtDg, aDgContHd.GetRecEndFilePos() ) )
    {
        DffRecordHeader aRecHd;
        rSt >> aRecHd;
        sal_uInt32 nDrawingId = aRecHd.nRecInstance;
        maDgOffsetTable[ nDrawingId ] = nFilePos;
        rSt.Seek( nFilePos );
    }
}

void SvxMSDffManager::GetFidclData( sal_uInt32 nOffsDggL )
{
    if (!nOffsDggL)
        return;

    sal_uInt32 nDummy, nMerk = rStCtrl.Tell();

    if (nOffsDggL == rStCtrl.Seek(nOffsDggL))
    {
        DffRecordHeader aRecHd;
        rStCtrl >> aRecHd;

        DffRecordHeader aDggAtomHd;
        if ( SeekToRec( rStCtrl, DFF_msofbtDgg, aRecHd.GetRecEndFilePos(), &aDggAtomHd ) )
        {
            aDggAtomHd.SeekToContent( rStCtrl );
            rStCtrl >> mnCurMaxShapeId
                    >> mnIdClusters
                    >> nDummy
                    >> mnDrawingsSaved;

            if ( mnIdClusters-- > 2 )
            {
                const sal_Size nFIDCLsize = sizeof(sal_uInt32) * 2;
                if ( aDggAtomHd.nRecLen == ( mnIdClusters * nFIDCLsize + 16 ) )
                {
                    sal_Size nMaxEntriesPossible = rStCtrl.remainingSize() / nFIDCLsize;
                    SAL_WARN_IF(nMaxEntriesPossible < mnIdClusters,
                        "filter.ms", "FIDCL list longer than remaining bytes, ppt or parser is wrong");
                    mnIdClusters = std::min(nMaxEntriesPossible, static_cast<sal_Size>(mnIdClusters));

                    maFidcls.resize(mnIdClusters);
                    for (sal_uInt32 i = 0; i < mnIdClusters; ++i)
                    {
                        rStCtrl >> maFidcls[ i ].dgid
                                >> maFidcls[ i ].cspidCur;
                    }
                }
            }
        }
    }
    rStCtrl.Seek( nMerk );
}

void SvxMSDffManager::CheckTxBxStoryChain()
{
    m_pShapeInfosById.reset(new SvxMSDffShapeInfos_ById);
    // mangle old Info array, sorted by nTxBxComp
    sal_uLong nChain    = ULONG_MAX;
    sal_Bool bSetReplaceFALSE = sal_False;
    for (SvxMSDffShapeInfos_ByTxBxComp::iterator iter =
                m_pShapeInfosByTxBxComp->begin(),
            mark = m_pShapeInfosByTxBxComp->begin();
         iter != m_pShapeInfosByTxBxComp->end(); ++iter)
    {
        boost::shared_ptr<SvxMSDffShapeInfo> const pObj = *iter;
        if( pObj->nTxBxComp )
        {
            pObj->bLastBoxInChain = sal_False;
            // group change?
            // #156763#
            // the text id also contains an internal drawing container id
            // to distinguish between text id of drawing objects in different
            // drawing containers.
            if( nChain != pObj->nTxBxComp )
            {
                // previous was last of its group
                if (iter != m_pShapeInfosByTxBxComp->begin())
                {
                    SvxMSDffShapeInfos_ByTxBxComp::iterator prev(iter);
                    --prev;
                    (*prev)->bLastBoxInChain = true;
                }
                // reset mark and helper flag
                mark = iter;
                nChain = pObj->nTxBxComp;
                bSetReplaceFALSE = !pObj->bReplaceByFly;
            }
            else if( !pObj->bReplaceByFly )
            {
                // object that must NOT be replaced by frame?
                bSetReplaceFALSE = sal_True;
                // maybe reset flags in start of group
                for (SvxMSDffShapeInfos_ByTxBxComp::iterator itemp = mark;
                        itemp != iter; ++itemp)
                {
                    (*itemp)->bReplaceByFly = false;
                }
            }

            if( bSetReplaceFALSE )
            {
                pObj->bReplaceByFly = sal_False;
            }
        }
        // copy all Shape Info objects to m_pShapeInfosById, sorted by nShapeId
        pObj->nTxBxComp = pObj->nTxBxComp & 0xFFFF0000;
        m_pShapeInfosById->insert( pObj );
    }
    // last one was last of its group
    if (!m_pShapeInfosByTxBxComp->empty())
    {
        (*m_pShapeInfosByTxBxComp->rbegin())->bLastBoxInChain = true;
    }
    // free original array but don't free its elements
    m_pShapeInfosByTxBxComp.reset();
}


/*****************************************************************************

    Reading the Shape-Infos in the Ctor:
    ---------------------------------
    remembering the Shape-Ids and the associated Blip-Numbers und TextBox-Infos
                    =========                    ============     =============
    and remebering the File-Offsets for each Blip
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
    if( !this->ReadCommonRecordHeader( rStCtrl, nVer, nInst, nFbt, nLength ) ) return;

    sal_Bool bOk;
    sal_uLong nPos = nOffsDggL + DFF_COMMON_RECORD_HEADER_SIZE;

    // case A: first Drawing Group Container, then n times Drawing Container
    if( DFF_msofbtDggContainer == nFbt )
    {
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


// from here on: Drawing Group Container  i.e. Ddocument-wide valid data
//                      =======================           ========
//
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
        if(!this->ReadCommonRecordHeader( rSt, nVer, nInst, nFbt, nLength)) return;
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
        if(!this->ReadCommonRecordHeader( rSt, nVer, nInst, nFbt, nLength)) return;
        nRead += DFF_COMMON_RECORD_HEADER_SIZE + nLength;
        if( DFF_msofbtBSE == nFbt )
        {
            nLenFBSE = nLength;
            // is FBSE big enough for our data
            sal_Bool bOk = ( nSkipBLIPLen + 4 + nSkipBLIPPos + 4 <= nLenFBSE );

            if( bOk )
            {
                rSt.SeekRel( nSkipBLIPLen );
                rSt >> nBLIPLen;
                rSt.SeekRel( nSkipBLIPPos );
                rSt >> nBLIPPos;
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

                // That worked great!
                // We store, that we do have one FBSE more in the pointer array.
                nBLIPPos = Calc_nBLIPPos(nBLIPPos, rSt.Tell());

                if( USHRT_MAX == nBLIPCount )
                    nBLIPCount = 1;
                else
                    nBLIPCount++;

                // now save the info for later access
                pBLIPInfos->push_back( new SvxMSDffBLIPInfo( nInst, nBLIPPos, nBLIPLen ) );
            }
        }
        rSt.SeekRel( nLength );
    }
    while( nRead < nLenBStoreCont );
}


// from now on: Drawing Container  which means Pages (Sheet, Slide) - wide valid data
//                      =================               ======
//
void SvxMSDffManager::GetDrawingContainerData( SvStream& rSt, sal_uLong nLenDg,
                                               const unsigned long nDrawingContainerId )
{
    sal_uInt8 nVer;sal_uInt16 nInst;sal_uInt16 nFbt;sal_uInt32 nLength;

    sal_uLong nReadDg = 0;

    // We are now in a drawing container (one per each page) and
    // we now have to iterate through all contained shape group containers
    do
    {
        if(!this->ReadCommonRecordHeader( rSt, nVer, nInst, nFbt, nLength)) return;
        nReadDg += DFF_COMMON_RECORD_HEADER_SIZE;
        // Patriarch found (the upmost shape group container) ?
        if( DFF_msofbtSpgrContainer == nFbt )
        {
            if(!this->GetShapeGroupContainerData( rSt, nLength, sal_True, nDrawingContainerId )) return;
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

sal_Bool SvxMSDffManager::GetShapeGroupContainerData( SvStream& rSt,
                                                  sal_uLong nLenShapeGroupCont,
                                                  sal_Bool bPatriarch,
                                                  const unsigned long nDrawingContainerId )
{
    sal_uInt8 nVer;sal_uInt16 nInst;sal_uInt16 nFbt;sal_uInt32 nLength;
    long nStartShapeGroupCont = rSt.Tell();
    // We are now in a shape group container (conditionally mulitple per page)
    // an we now have to iterate through all contained shape containers
    sal_Bool  bFirst = !bPatriarch;
    sal_uLong nReadSpGrCont = 0;
    do
    {
        if( !this->ReadCommonRecordHeader( rSt, nVer, nInst, nFbt, nLength ) )
            return sal_False;
        nReadSpGrCont += DFF_COMMON_RECORD_HEADER_SIZE;
        // Shape Container?
        if( DFF_msofbtSpContainer == nFbt )
        {
            sal_uLong nGroupOffs = bFirst ? nStartShapeGroupCont - DFF_COMMON_RECORD_HEADER_SIZE : ULONG_MAX;
            if ( !this->GetShapeContainerData( rSt, nLength, nGroupOffs, nDrawingContainerId ) )
                return sal_False;
            bFirst = sal_False;
        }
        // nested shape group container ?
        else if( DFF_msofbtSpgrContainer == nFbt )
        {
            if ( !this->GetShapeGroupContainerData( rSt, nLength, sal_False, nDrawingContainerId ) )
                return sal_False;
        }
        else
            rSt.SeekRel( nLength );
        nReadSpGrCont += nLength;
    }
    while( nReadSpGrCont < nLenShapeGroupCont );
    // possition the steam correctly
    rSt.Seek( nStartShapeGroupCont + nLenShapeGroupCont );
    return sal_True;
}

sal_Bool SvxMSDffManager::GetShapeContainerData( SvStream& rSt,
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

    // Store file offset of the shape containers or respectivly the group(!).
    sal_uLong nStartOffs = (ULONG_MAX > nPosGroup) ?
                            nPosGroup : nStartShapeCont - DFF_COMMON_RECORD_HEADER_SIZE;
    SvxMSDffShapeInfo aInfo( nStartOffs );

    // Can the shape be replaced with a frame?
    // (provided that it is a TextBox and the text is not rotated)
    sal_Bool bCanBeReplaced = (ULONG_MAX > nPosGroup) ? sal_False : sal_True;

    // we don't know yet whether it's a TextBox
    MSO_SPT         eShapeType      = mso_sptNil;

    // analyze Shape
    //
    do
    {
        if(!this->ReadCommonRecordHeader( rSt, nVer, nInst, nFbt, nLength)) return sal_False;
        nReadSpCont += DFF_COMMON_RECORD_HEADER_SIZE;
        // FSP ?
        if( ( DFF_msofbtSp == nFbt ) && ( 4 <= nLength ) )
        {
            // we've found the FSP gefunden: note Shape Type and Id!
            eShapeType = (MSO_SPT)nInst;
            rSt >> aInfo.nShapeId;
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
                rSt >> nPropId
                    >> nPropVal;
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
                                bCanBeReplaced = sal_False;
                        }
                    break;
                    case DFF_Prop_Rotation :
                        if( 0 != nPropVal )
                            bCanBeReplaced = sal_False;
                    break;

                    case DFF_Prop_gtextFStrikethrough :
                        if( ( 0x20002000 & nPropVal )  == 0x20002000 )
                            bCanBeReplaced = sal_False;
                    break;

                    case DFF_Prop_fc3DLightFace :
                        if( ( 0x00080008 & nPropVal ) == 0x00080008 )
                            bCanBeReplaced = sal_False;
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
            rSt >> aInfo.nTxBxComp;
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

    //
    // Now possibly store the information for subsequent accesses to the shape
    //
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
            aInfo.bReplaceByFly = sal_True;
        }
        m_pShapeInfosByTxBxComp->insert(::boost::shared_ptr<SvxMSDffShapeInfo>(
                    new SvxMSDffShapeInfo(aInfo)));
        pShapeOrders->push_back( new SvxMSDffShapeOrder( aInfo.nShapeId ) );
    }

    // and position the Stream correctly again
    rSt.Seek( nStartShapeCont + nLenShapeCont );
    return sal_True;
}



/*****************************************************************************

    Access to a shape at runtime (via the Shape-Id)
    ----------------------------
******************************************************************************/
sal_Bool SvxMSDffManager::GetShape(sal_uLong nId, SdrObject*&         rpShape,
                                          SvxMSDffImportData& rData)
{
    ::boost::shared_ptr<SvxMSDffShapeInfo> const pTmpRec(
        new SvxMSDffShapeInfo(0, nId));

    SvxMSDffShapeInfos_ById::const_iterator const it =
        m_pShapeInfosById->find(pTmpRec);
    if (it != m_pShapeInfosById->end())
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
        if( &rStCtrl != pStData )
            pStData->Seek( nOldPosData );
        return ( 0 != rpShape );
    }
    return sal_False;
}



/*      access to a BLIP at runtime (if the Blip-Number is already known)
        ---------------------------
******************************************************************************/
sal_Bool SvxMSDffManager::GetBLIP( sal_uLong nIdx_, Graphic& rData, Rectangle* pVisArea )
{
    sal_Bool bOk = sal_False;       // initialize result variable
    if ( pStData )
    {
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
                if ( rData.GetType() != GRAPHIC_NONE )
                    bOk = sal_True;
                else
                    aEscherBlipCache.erase(iter);
            }
        }
        if ( !bOk )
        {
            sal_uInt16 nIdx = sal_uInt16( nIdx_ );
            if( !nIdx || (pBLIPInfos->size() < nIdx) ) return sal_False;

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
            SvxMSDffBLIPInfo& rInfo = (*pBLIPInfos)[ nIdx-1 ];
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
                // restore olf FilePos of the second data stream
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
    }
    return bOk;
}

/*      access to a BLIP at runtime (with correctly positioned stream)
    ---------------------------------
******************************************************************************/
sal_Bool SvxMSDffManager::GetBLIPDirect( SvStream& rBLIPStream, Graphic& rData, Rectangle* pVisArea ) const
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
        sal_Bool        bMtfBLIP = sal_False;
        sal_Bool        bZCodecCompression = sal_False;
        // now position it exactly at the beinning of the embedded graphic
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
                rBLIPStream >> width >> height;
                aMtfSize100.Width() = width;
                aMtfSize100.Height() = height;

                // scale to 1/100mm
                aMtfSize100.Width() /= 360, aMtfSize100.Height() /= 360;

                if ( pVisArea )     // seem that we currently are skipping the visarea position
                    *pVisArea = Rectangle( Point(), aMtfSize100 );

                // skip rest of header
                nSkip = 6;
                bMtfBLIP = bZCodecCompression = sal_True;
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
        SvMemoryStream* pOut = NULL;
        if( bZCodecCompression )
        {
            pOut = new SvMemoryStream( 0x8000, 0x4000 );
            ZCodec aZCodec( 0x8000, 0x8000 );
            aZCodec.BeginCompression();
            aZCodec.Decompress( rBLIPStream, *pOut );
            aZCodec.EndCompression();
            pOut->Seek( STREAM_SEEK_TO_BEGIN );
            pOut->SetResizeOffset( 0 ); // sj: #i102257# setting ResizeOffset of 0 prevents from seeking
                                        // behind the stream end (allocating too much memory)
            pGrStream = pOut;
        }

#if OSL_DEBUG_LEVEL > 2
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
        if( ::utl::LocalFileHelper::ConvertPhysicalNameToURL( Application::GetAppFileName(), aURLStr ) )
        {
            INetURLObject aURL( aURLStr );

            aURL.removeSegment();
            aURL.removeFinalSlash();
            aURL.Append( aFileName );

            aURLStr = aURL.GetMainURL( INetURLObject::NO_DECODE );

            SAL_INFO("filter.ms", "dumping " << aURLStr);

            SvStream* pDbgOut = ::utl::UcbStreamHelper::CreateStream(aURLStr, STREAM_TRUNC | STREAM_WRITE);

            if( pDbgOut )
            {
                if ( bZCodecCompression )
                {
                    pOut->Seek( STREAM_SEEK_TO_END );
                    pDbgOut->Write( pOut->GetData(), pOut->Tell() );
                    pOut->Seek( STREAM_SEEK_TO_BEGIN );
                }
                else
                {
                    sal_Int32 nDbgLen = nLength - nSkip;
                    if ( nDbgLen )
                    {
                        sal_Char* pDat = new sal_Char[ nDbgLen ];
                        pGrStream->Read( pDat, nDbgLen );
                        pDbgOut->Write( pDat, nDbgLen );
                        pGrStream->SeekRel( -nDbgLen );
                        delete[] pDat;
                    }
                }

                delete pDbgOut;
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
            OUString aEmptyStr;
            nRes = rGF.ImportGraphic( rData, aEmptyStr, *pGrStream, GRFILTER_FORMAT_DONTKNOW );

            // SJ: I40472, sometimes the aspect ratio (aMtfSize100) does not match and we get scaling problems,
            // then it is better to use the prefsize that is stored within the metafile. Bug #72846# for what the
            // scaling has been implemented does not happen anymore.
            //
            // For pict graphics we will furthermore scale the metafile, because font scaling leads to error if the
            // dxarray is empty (this has been solved in wmf/emf but not for pict)
            if( bMtfBLIP && ( GRFILTER_OK == nRes ) && ( rData.GetType() == GRAPHIC_GDIMETAFILE ) && ( ( nInst & 0xFFFE ) == 0x542 ) )
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
                        aMtf.SetPrefMapMode( MAP_100TH_MM );
                        rData = aMtf;
                    }
                }
            }
        }
        // reset error status if necessary
        if ( ERRCODE_IO_PENDING == pGrStream->GetError() )
          pGrStream->ResetError();
        delete pOut;
    }
    rBLIPStream.Seek( nOldPos );    // restore old FilePos of the strem

    return ( GRFILTER_OK == nRes ); // return result
}

/* also static */
bool SvxMSDffManager::ReadCommonRecordHeader(SvStream& rSt,
    sal_uInt8& rVer, sal_uInt16& rInst, sal_uInt16& rFbt, sal_uInt32& rLength)
{
    sal_uInt16 nTmp(0);
    rSt >> nTmp >> rFbt >> rLength;
    rVer = sal::static_int_cast< sal_uInt8 >(nTmp & 15);
    rInst = nTmp >> 4;
    if (!rSt.good())
        return false;
    if (rLength > nMaxLegalDffRecordLength)
        return false;
    return true;
}

sal_Bool SvxMSDffManager::ProcessClientAnchor(SvStream& rStData, sal_uInt32 nDatLen,
                                          char*& rpBuff, sal_uInt32& rBuffLen ) const
{
    if( nDatLen )
    {
        rBuffLen = std::min(rStData.remainingSize(), static_cast<sal_Size>(nDatLen));
        rpBuff = new char[rBuffLen];
        rBuffLen = rStData.Read(rpBuff, rBuffLen);
    }
    return sal_True;
}

sal_Bool SvxMSDffManager::ProcessClientData(SvStream& rStData, sal_uInt32 nDatLen,
                                        char*& rpBuff, sal_uInt32& rBuffLen ) const
{
    if( nDatLen )
    {
        rBuffLen = std::min(rStData.remainingSize(), static_cast<sal_Size>(nDatLen));
        rpBuff = new char[rBuffLen];
        rBuffLen = rStData.Read(rpBuff, rBuffLen);
    }
    return sal_True;
}


void SvxMSDffManager::ProcessClientAnchor2( SvStream& /* rSt */, DffRecordHeader& /* rHd */ , void* /* pData */, DffObjData& /* rObj */ )
{
    return;  // will be overloaded by SJ in Draw
}

sal_uLong SvxMSDffManager::Calc_nBLIPPos( sal_uLong nOrgVal, sal_uLong /* nStreamPos */ ) const
{
    return nOrgVal;
}

sal_Bool SvxMSDffManager::GetOLEStorageName( long /* nOLEId */, OUString&, SvStorageRef&, uno::Reference < embed::XStorage >& ) const
{
    return sal_False;
}

sal_Bool SvxMSDffManager::ShapeHasText( sal_uLong /* nShapeId */, sal_uLong /* nFilePos */ ) const
{
    return sal_True;
}

// #i32596# - add new parameter <_nCalledByGroup>
SdrObject* SvxMSDffManager::ImportOLE( long nOLEId,
                                       const Graphic& rGrf,
                                       const Rectangle& rBoundRect,
                                       const Rectangle& rVisArea,
                                       const int /* _nCalledByGroup */,
                                       sal_Int64 nAspect ) const
{
    SdrObject* pRet = 0;
    OUString sStorageName;
    SvStorageRef xSrcStg;
    ErrCode nError = ERRCODE_NONE;
    uno::Reference < embed::XStorage > xDstStg;
    if( GetOLEStorageName( nOLEId, sStorageName, xSrcStg, xDstStg ))
        pRet = CreateSdrOLEFromStorage( sStorageName, xSrcStg, xDstStg,
                                        rGrf, rBoundRect, rVisArea, pStData, nError,
                                        nSvxMSDffOLEConvFlags, nAspect );
    return pRet;
}

sal_Bool SvxMSDffManager::MakeContentStream( SotStorage * pStor, const GDIMetaFile & rMtf )
{
    SotStorageStreamRef xStm = pStor->OpenSotStream(OUString(SVEXT_PERSIST_STREAM));
    xStm->SetVersion( pStor->GetVersion() );
    xStm->SetBufferSize( 8192 );

    sal_uInt16 nAspect = ASPECT_CONTENT;
    sal_uLong nAdviseModes = 2;

    Impl_OlePres aEle( FORMAT_GDIMETAFILE );
    // Convert the size in 1/100 mm
    // If a not applicable MapUnit (device dependend) is used,
    // SV tries to guess a best match for the right value
    Size aSize = rMtf.GetPrefSize();
    MapMode aMMSrc = rMtf.GetPrefMapMode();
    MapMode aMMDst( MAP_100TH_MM );
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


sal_Bool SvxMSDffManager::ConvertToOle2( SvStream& rStm, sal_uInt32 nReadLen,
                    const GDIMetaFile * pMtf, const SotStorageRef& rDest )
{
    sal_Bool bMtfRead = sal_False;
    SotStorageStreamRef xOle10Stm = rDest->OpenSotStream( OUString("\1Ole10Native"),
                                                    STREAM_WRITE| STREAM_SHARE_DENYALL );
    if( xOle10Stm->GetError() )
        return sal_False;

    sal_uInt32 nType;
    sal_uInt32 nRecType;
    sal_uInt32 nStrLen;
    OUString   aSvrName;
    sal_uInt32 nDummy0;
    sal_uInt32 nDummy1;
    sal_uInt32 nDataLen;
    sal_uInt8 * pData;
    sal_uInt32 nBytesRead = 0;
    do
    {
        rStm >> nType;
        rStm >> nRecType;
        rStm >> nStrLen;
        if( nStrLen )
        {
            if( 0x10000L > nStrLen )
            {
                sal_Char * pBuf = new sal_Char[ nStrLen ];
                rStm.Read( pBuf, nStrLen );
                aSvrName = OUString( pBuf, (sal_uInt16) nStrLen-1, osl_getThreadTextEncoding() );
                delete[] pBuf;
            }
            else
                break;
        }
        rStm >> nDummy0;
        rStm >> nDummy1;
        rStm >> nDataLen;

        nBytesRead += 6 * sizeof( sal_uInt32 ) + nStrLen + nDataLen;

        if( !rStm.IsEof() && nReadLen > nBytesRead && nDataLen )
        {
            if( xOle10Stm.Is() )
            {
                pData = new sal_uInt8[ nDataLen ];
                if( !pData )
                    return sal_False;

                rStm.Read( pData, nDataLen );

                // write to ole10 stream
                *xOle10Stm << nDataLen;
                xOle10Stm->Write( pData, nDataLen );
                xOle10Stm = SotStorageStreamRef();

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
                    sal_uLong nCbFmt = SotExchange::RegisterFormatName( aSvrName );
                    rDest->SetClass( SvGlobalName( pIds->nId, 0, 0, 0xc0,0,0,0,0,0,0,0x46 ), nCbFmt,
                                    OUString::createFromAscii( pIds->pDspName ) );
                }
                else
                {
                    sal_uLong nCbFmt = SotExchange::RegisterFormatName( aSvrName );
                    rDest->SetClass( SvGlobalName(), nCbFmt, aSvrName );
                }

                delete[] pData;
            }
            else if( nRecType == 5 && !pMtf )
            {
                sal_uLong nPos = rStm.Tell();
                sal_uInt16 sz[4];
                rStm.Read( sz, 8 );
                Graphic aGraphic;
                if( ERRCODE_NONE == GraphicConverter::Import( rStm, aGraphic ) && aGraphic.GetType() )
                {
                    const GDIMetaFile& rMtf = aGraphic.GetGDIMetaFile();
                    MakeContentStream( rDest, rMtf );
                    bMtfRead = sal_True;
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
        MakeContentStream( rDest, *pMtf );
        return sal_True;
    }

    return sal_False;
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
    return 0;
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

com::sun::star::uno::Reference < com::sun::star::embed::XEmbeddedObject >  SvxMSDffManager::CheckForConvertToSOObj( sal_uInt32 nConvertFlags,
                        SotStorage& rSrcStg, const uno::Reference < embed::XStorage >& rDestStorage,
                        const Graphic& rGrf,
                        const Rectangle& rVisArea )
{
    uno::Reference < embed::XEmbeddedObject > xObj;
    SvGlobalName aStgNm = rSrcStg.GetClassName();
    const char* pName = GetInternalServerName_Impl( aStgNm );
    OUString sStarName;
    if ( pName )
        sStarName = OUString::createFromAscii( pName );
    else if ( nConvertFlags )
    {
        static struct _ObjImpType
        {
            sal_uInt32 nFlag;
            const char* pFactoryNm;
            // GlobalNameId
            sal_uInt32 n1;
            sal_uInt16 n2, n3;
            sal_uInt8 b8, b9, b10, b11, b12, b13, b14, b15;
        } aArr[] = {
            { OLE_MATHTYPE_2_STARMATH, "smath",
                0x0002ce02L, 0x0000, 0x0000,
                0xc0,0x00,0x00,0x00,0x00,0x00,0x00,0x46 },
            { OLE_MATHTYPE_2_STARMATH, "smath",
                0x00021700L, 0x0000, 0x0000,
                0xc0,0x00,0x00,0x00,0x00,0x00,0x00,0x46 },
            { OLE_WINWORD_2_STARWRITER, "swriter",
                0x00020906L, 0x0000, 0x0000,
                0xc0,0x00,0x00,0x00,0x00,0x00,0x00,0x46 },
            { OLE_EXCEL_2_STARCALC, "scalc",                // Excel table
                0x00020810L, 0x0000, 0x0000,
                0xc0,0x00,0x00,0x00,0x00,0x00,0x00,0x46 },
            { OLE_EXCEL_2_STARCALC, "scalc",                // Excel chart
                0x00020820L, 0x0000, 0x0000,
                0xc0,0x00,0x00,0x00,0x00,0x00,0x00,0x46 },
            // 114465: additional Excel OLE chart classId to above.
            { OLE_EXCEL_2_STARCALC, "scalc",
                0x00020821L, 0x0000, 0x0000,
                0xc0,0x00,0x00,0x00,0x00,0x00,0x00,0x46 },
            { OLE_POWERPOINT_2_STARIMPRESS, "simpress",     // PowerPoint presentation
                0x64818d10L, 0x4f9b, 0x11cf,
                0x86,0xea,0x00,0xaa,0x00,0xb9,0x29,0xe8 },
            { OLE_POWERPOINT_2_STARIMPRESS, "simpress",     // PowerPoint slide
                0x64818d11L, 0x4f9b, 0x11cf,
                0x86,0xea,0x00,0xaa,0x00,0xb9,0x29,0xe8 },
            { 0, 0,
              0, 0, 0,
              0, 0, 0, 0, 0, 0, 0, 0 }
        };

        for( const _ObjImpType* pArr = aArr; pArr->nFlag; ++pArr )
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
        const SfxFilter* pFilter = 0;
        ::boost::scoped_ptr<SvMemoryStream> xMemStream (new SvMemoryStream);
        if ( pName )
        {
            // TODO/LATER: perhaps we need to retrieve VisArea and Metafile from the storage also
            SotStorageStreamRef xStr = rSrcStg.OpenSotStream( OUString( "package_stream" ), STREAM_STD_READ );
            *xStr >> *xMemStream;
        }
        else
        {
            SfxFilterMatcher aMatch( sStarName );
            SotStorageRef xStorage = new SotStorage( sal_False, *xMemStream );
            rSrcStg.CopyTo( xStorage );
            xStorage->Commit();
            xStorage.Clear();
            OUString aType = SfxFilter::GetTypeFromStorage( rSrcStg );
            if ( aType.getLength() )
                pFilter = aMatch.GetFilter4EA( aType );
        }

#if OSL_DEBUG_LEVEL > 2
        // extract embedded ole streams into "/tmp/embedded_stream_NNN"
        static sal_Int32 nOleCount(0);
        OUString aTmpName("/tmp/embedded_stream_");
        aTmpName += OUString::number(nOleCount++);
        aTmpName += ".bin";
        SvFileStream aTmpStream(aTmpName,STREAM_READ|STREAM_WRITE|STREAM_TRUNC);
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

            uno::Sequence < beans::PropertyValue > aMedium( aFilterName.isEmpty() ? 2 : 3);
            aMedium[0].Name = OUString( "InputStream" );
            uno::Reference < io::XInputStream > xStream = new ::utl::OSeekableInputStreamWrapper( *xMemStream );
            aMedium[0].Value <<= xStream;
            aMedium[1].Name = OUString( "URL" );
            aMedium[1].Value <<= OUString( "private:stream" );

            if ( !aFilterName.isEmpty() )
            {
                aMedium[2].Name = OUString( "FilterName" );
                aMedium[2].Value <<= aFilterName;
            }

            OUString aName( aDstStgName );
            comphelper::EmbeddedObjectContainer aCnt( rDestStorage );
            xObj = aCnt.InsertEmbeddedObject( aMedium, aName );

            if ( !xObj.is() )
            {
                if( !aFilterName.isEmpty() )
                {
                    // throw the filter parameter away as workaround
                    aMedium.realloc( 2 );
                    xObj = aCnt.InsertEmbeddedObject( aMedium, aName );
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
            // TODO/LATER: it might make sence in future to set the size stored in internal object
            if( !pName && ( sStarName == "swriter" || sStarName == "scalc" ) )
            {
                MapMode aMapMode( VCLUnoHelper::UnoEmbed2VCLMapUnit( xObj->getMapUnit( nViewAspect ) ) );
                Size aSz;
                if ( rVisArea.IsEmpty() )
                    aSz = lcl_GetPrefSize(rGrf, aMapMode );
                else
                {
                    aSz = rVisArea.GetSize();
                    aSz = OutputDevice::LogicToLogic( aSz, MapMode( MAP_100TH_MM ), aMapMode );
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
                SotStorageRef& rSrcStorage,
                const uno::Reference < embed::XStorage >& xDestStorage,
                const Graphic& rGrf,
                const Rectangle& rBoundRect,
                const Rectangle& rVisArea,
                SvStream* pDataStrm,
                ErrCode& rError,
                sal_uInt32 nConvertFlags,
                sal_Int64 nRecommendedAspect )
{
    sal_Int64 nAspect = nRecommendedAspect;
    SdrOle2Obj* pRet = 0;
    if( rSrcStorage.Is() && xDestStorage.is() && rStorageName.getLength() )
    {
        comphelper::EmbeddedObjectContainer aCnt( xDestStorage );
        // does the 01Ole-Stream exist at all?
        // (that's not the case for e.g. Fontwork )
        // If that's not the case -> include it as graphic
        sal_Bool bValidStorage = sal_False;
        OUString aDstStgName(MSO_OLE_Obj);

        aDstStgName += OUString::number( ++nMSOleObjCntr );

        {
            SvStorageRef xObjStg = rSrcStorage->OpenSotStorage( rStorageName,
                                STREAM_READWRITE| STREAM_SHARE_DENYALL );
            if( xObjStg.Is()  )
            {
                {
                    sal_uInt8 aTestA[10];   // exist the \1CompObj-Stream ?
                    SvStorageStreamRef xSrcTst = xObjStg->OpenSotStream( OUString( "\1CompObj" ) );
                    bValidStorage = xSrcTst.Is() && sizeof( aTestA ) ==
                                    xSrcTst->Read( aTestA, sizeof( aTestA ) );
                    if( !bValidStorage )
                    {
                        // or the \1Ole-Stream ?
                        xSrcTst = xObjStg->OpenSotStream( OUString( "\1Ole" ) );
                        bValidStorage = xSrcTst.Is() && sizeof(aTestA) ==
                                        xSrcTst->Read(aTestA, sizeof(aTestA));
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

                        SvStorageStreamRef xObjInfoSrc = xObjStg->OpenSotStream(
                            OUString( "\3ObjInfo" ), STREAM_STD_READ | STREAM_NOCREATE );
                        if ( xObjInfoSrc.Is() && !xObjInfoSrc->GetError() )
                        {
                            sal_uInt8 nByte = 0;
                            *xObjInfoSrc >> nByte;
                            if ( ( nByte >> 4 ) & embed::Aspects::MSOLE_ICON )
                                nAspect = embed::Aspects::MSOLE_ICON;
                        }
                    }

                    uno::Reference < embed::XEmbeddedObject > xObj( CheckForConvertToSOObj(
                                nConvertFlags, *xObjStg, xDestStorage, rGrf, rVisArea ));
                    if ( xObj.is() )
                    {
                        svt::EmbeddedObjectRef aObj( xObj, nAspect );

                        // TODO/LATER: need MediaType
                        aObj.SetGraphic( rGrf, OUString() );

                        // TODO/MBA: check setting of PersistName
                        pRet = new SdrOle2Obj( aObj, OUString(), rBoundRect, false);
                        // we have the Object, don't create another
                        bValidStorage = false;
                    }
                }
            }
        }

        if( bValidStorage )
        {
            // object is not an own object
            SotStorageRef xObjStor = SotStorage::OpenOLEStorage( xDestStorage, aDstStgName, STREAM_READWRITE );

            if ( xObjStor.Is() )
            {
                SotStorageRef xSrcStor = rSrcStorage->OpenSotStorage( rStorageName, STREAM_READ );
                xSrcStor->CopyTo( xObjStor );

                if( !xObjStor->GetError() )
                    xObjStor->Commit();

                if( xObjStor->GetError() )
                {
                    rError = xObjStor->GetError();
                    bValidStorage = sal_False;
                }
                else if( !xObjStor.Is() )
                    bValidStorage = sal_False;
            }
        }
        else if( pDataStrm )
        {
            sal_uInt32 nLen, nDummy;
            *pDataStrm >> nLen >> nDummy;
            if( SVSTREAM_OK != pDataStrm->GetError() ||
                // Id in BugDoc - exist there other Ids?
                // The ConvertToOle2 - does not check for consistent
                0x30008 != nDummy )
                bValidStorage = sal_False;
            else
            {
                // or is it an OLE-1 Stream in the DataStream?
                SvStorageRef xObjStor = SotStorage::OpenOLEStorage( xDestStorage, aDstStgName );
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

                pRet = new SdrOle2Obj( aObj, aDstStgName, rBoundRect, false);
            }
        }
    }

    return pRet;
}

sal_Bool SvxMSDffManager::SetPropValue( const uno::Any& rAny, const uno::Reference< ::com::sun::star::beans::XPropertySet > & rXPropSet,
            const OUString& rPropName, sal_Bool bTestPropertyAvailability )
{
    sal_Bool bRetValue = sal_True;
    if ( bTestPropertyAvailability )
    {
        bRetValue = sal_False;
        try
        {
            uno::Reference< beans::XPropertySetInfo >
                aXPropSetInfo( rXPropSet->getPropertySetInfo() );
            if ( aXPropSetInfo.is() )
                bRetValue = aXPropSetInfo->hasPropertyByName( rPropName );
        }
        catch( const uno::Exception& )
        {
            bRetValue = sal_False;
        }
    }
    if ( bRetValue )
    {
        try
        {
            rXPropSet->setPropertyValue( rPropName, rAny );
            bRetValue = sal_True;
        }
        catch( const uno::Exception& )
        {
            bRetValue = sal_False;
        }
    }
    return bRetValue;
}

SvxMSDffImportRec::SvxMSDffImportRec()
    : pObj( 0 ),
      pWrapPolygon(0),
      pClientAnchorBuffer( 0 ),
      nClientAnchorLen(  0 ),
      pClientDataBuffer( 0 ),
      nClientDataLen(    0 ),
      nXAlign( 0 ), // position n cm from left
      pXRelTo( NULL ), //   relative to column
      nYAlign( 0 ), // position n cm below
      pYRelTo( NULL ), //   relative to paragraph
      nLayoutInTableCell( 0 ), // element is laid out in table cell
      nTextRotationAngle( 0 ),
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
      bDrawHell       = sal_False;
      bHidden         = sal_False;

      bReplaceByFly   = sal_False;
      bLastBoxInChain = sal_True;
      bHasUDefProp    = sal_False; // was the DFF_msofbtUDefProp record set?
      bVFlip = sal_False;
      bHFlip = sal_False;
      bAutoWidth      = sal_False;
}

SvxMSDffImportRec::SvxMSDffImportRec(const SvxMSDffImportRec& rCopy)
    : pObj( rCopy.pObj ),
      nXAlign( rCopy.nXAlign ),
      pXRelTo( NULL ),
      nYAlign( rCopy.nYAlign ),
      pYRelTo( NULL ),
      nLayoutInTableCell( rCopy.nLayoutInTableCell ),
      nTextRotationAngle( rCopy.nTextRotationAngle ),
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
    bLastBoxInChain  = rCopy.bLastBoxInChain;
    bHasUDefProp     = rCopy.bHasUDefProp;
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
        pClientAnchorBuffer = 0;

    nClientDataLen = rCopy.nClientDataLen;
    if( rCopy.nClientDataLen )
    {
        pClientDataBuffer = new char[ nClientDataLen ];
        memcpy( pClientDataBuffer,
                rCopy.pClientDataBuffer,
                nClientDataLen );
    }
    else
        pClientDataBuffer = 0;

    if (rCopy.pWrapPolygon)
        pWrapPolygon = new Polygon(*rCopy.pWrapPolygon);
    else
        pWrapPolygon = 0;
}

SvxMSDffImportRec::~SvxMSDffImportRec()
{
    if (pClientAnchorBuffer)
        delete[] pClientAnchorBuffer;
    if (pClientDataBuffer)
        delete[] pClientDataBuffer;
    if (pWrapPolygon)
        delete pWrapPolygon;
    if (pXRelTo)
        delete pXRelTo;
    if (pYRelTo)
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
    return aIter != maShapeIdContainer.end() ? (*aIter).second : 0;
}

SvxMSDffShapeOrders::~SvxMSDffShapeOrders()
{
    for( const_iterator it = begin(); it != end(); ++it )
        delete *it;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
