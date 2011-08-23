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
#include "precompiled_filter.hxx"
/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil -*- */
#include <com/sun/star/embed/Aspects.hpp>

#include <math.h>
#include <limits.h>
#include <vector>
#include <osl/endian.h>
#include <tools/solar.h>               // UINTXX
#include <rtl/math.hxx>

#include <sot/clsids.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <unotools/streamwrap.hxx>
#include <comphelper/processfactory.hxx>
#include <sot/exchange.hxx>
#include <sot/storinfo.hxx>
#include <vcl/cvtgrf.hxx>
#include "viscache.hxx"

// SvxItem-Mapping. Wird benoetigt um die SvxItem-Header erfolgreich zu includen
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
#include <svtools/filter.hxx>
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
#include <editeng/crsditem.hxx>
#include <editeng/shdditem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/colritem.hxx>
#include <svx/sxekitm.hxx>
#include <editeng/bulitem.hxx>
#include <svx/polysc3d.hxx>
#include <svx/extrud3d.hxx>
#include "svx/svditer.hxx"
#include <svx/xpoly.hxx>
#include "svx/xattr.hxx"
#include <filter/msfilter/msdffimp.hxx> // extern sichtbare Header-Datei
#include <editeng/outliner.hxx>
#include <editeng/outlobj.hxx>
#include <editeng/editobj.hxx>
#include <editeng/editeng.hxx>
#include "svx/gallery.hxx"
#include <com/sun/star/drawing/ShadeMode.hpp>
#include <svl/itempool.hxx>
#include <vcl/svapp.hxx>
#include <svx/svx3ditems.hxx>
#include <svx/svdoashp.hxx>
#include <svx/sdasaitm.hxx>
#include <ucbhelper/content.hxx>
#include <ucbhelper/contentbroker.hxx>
#include <vos/xception.hxx>
#ifndef _VOS_NO_NAMESPACE
using namespace vos;
#endif
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

using namespace ::com::sun::star    ;
using namespace ::com::sun::star::drawing;
using namespace uno		            ;
using namespace beans		        ;
using namespace drawing	            ;
using namespace container	        ;

#define ITEMVALUE(ItemSet,Id,Cast)  ((const Cast&)(ItemSet).Get(Id)).GetValue()

// static counter for OLE-Objects
static sal_uInt32 nMSOleObjCntr = 0;
#define MSO_OLE_Obj "MSO_OLE_Obj"


/*************************************************************************/
BOOL Impl_OlePres::Read( SvStream & rStm )
{
    ULONG nBeginPos = rStm.Tell();
    INT32 n;
    rStm >> n;
    if( n != -1 )
    {
        pBmp = new Bitmap;
        rStm >> *pBmp;
        if( rStm.GetError() == SVSTREAM_OK )
        {
            nFormat = FORMAT_BITMAP;
            aSize = pBmp->GetPrefSize();
            MapMode aMMSrc;
            if( !aSize.Width() || !aSize.Height() )
            {
                // letzte Chance
                aSize = pBmp->GetSizePixel();
                aMMSrc = MAP_PIXEL;
            }
            else
                aMMSrc = pBmp->GetPrefMapMode();
            MapMode aMMDst( MAP_100TH_MM );
            aSize = OutputDevice::LogicToLogic( aSize, aMMSrc, aMMDst );
            return TRUE;
        }
        else
        {
            delete pBmp;
            pBmp = NULL;

            pMtf = new GDIMetaFile();
            rStm.ResetError();
            rStm >> *pMtf;
            if( rStm.GetError() == SVSTREAM_OK )
            {
                nFormat = FORMAT_GDIMETAFILE;
                aSize = pMtf->GetPrefSize();
                MapMode aMMSrc = pMtf->GetPrefMapMode();
                MapMode aMMDst( MAP_100TH_MM );
                aSize = OutputDevice::LogicToLogic( aSize, aMMSrc, aMMDst );
                return TRUE;
            }
            else
            {
                delete pMtf;
                pMtf = NULL;
            }
        }

    }

    rStm.ResetError();
    rStm.Seek( nBeginPos );
    nFormat = ReadClipboardFormat( rStm );
    // JobSetup, bzw. TargetDevice ueberlesen
    // Information aufnehmen, um sie beim Schreiben nicht zu verlieren
    nJobLen = 0;
    rStm >> nJobLen;
    if( nJobLen >= 4 )
    {
        nJobLen -= 4;
        if( nJobLen )
        {
            pJob = new BYTE[ nJobLen ];
            rStm.Read( pJob, nJobLen );
        }
    }
    else
    {
        rStm.SetError( SVSTREAM_GENERALERROR );
        return FALSE;
    }
    UINT32 nAsp;
    rStm >> nAsp;
    USHORT nSvAsp = USHORT( nAsp );
    SetAspect( nSvAsp );
    rStm.SeekRel( 4 ); //L-Index ueberlesen
    rStm >> nAdvFlags;
    rStm.SeekRel( 4 ); //Compression
    UINT32 nWidth  = 0;
    UINT32 nHeight = 0;
    UINT32 nSize   = 0;
    rStm >> nWidth >> nHeight >> nSize;
    aSize.Width() = nWidth;
    aSize.Height() = nHeight;

    if( nFormat == FORMAT_GDIMETAFILE )
    {
        pMtf = new GDIMetaFile();
        ReadWindowMetafile( rStm, *pMtf, NULL );
    }
    else if( nFormat == FORMAT_BITMAP )
    {
        pBmp = new Bitmap();
        rStm >> *pBmp;
    }
    else
    {
        BYTE * p = new BYTE[ nSize ];
        rStm.Read( p, nSize );
        delete [] p;
        return FALSE;
    }
    return TRUE;
}

/************************************************************************/
void Impl_OlePres::Write( SvStream & rStm )
{
    WriteClipboardFormat( rStm, FORMAT_GDIMETAFILE );
    rStm << (INT32)(nJobLen +4);       // immer leeres TargetDevice
    if( nJobLen )
        rStm.Write( pJob, nJobLen );
    rStm << (UINT32)nAspect;
    rStm << (INT32)-1;      //L-Index immer -1
    rStm << (INT32)nAdvFlags;
    rStm << (INT32)0;       //Compression
    rStm << (INT32)aSize.Width();
    rStm << (INT32)aSize.Height();
    ULONG nPos = rStm.Tell();
    rStm << (INT32)0;

    if( GetFormat() == FORMAT_GDIMETAFILE && pMtf )
    {
        // Immer auf 1/100 mm, bis Mtf-Loesung gefunden
        // Annahme (keine Skalierung, keine Org-Verschiebung)
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
        DBG_ERROR( "unknown format" );
    }
    ULONG nEndPos = rStm.Tell();
    rStm.Seek( nPos );
    rStm << (UINT32)(nEndPos - nPos - 4);
    rStm.Seek( nEndPos );
}

Impl_OlePres * CreateCache_Impl( SotStorage * pStor )
{
    SotStorageStreamRef xOleObjStm =pStor->OpenSotStream( String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "Ole-Object" ) ),
                                                        STREAM_READ | STREAM_NOCREATE );
    if( xOleObjStm->GetError() )
        return NULL;
    SotStorageRef xOleObjStor = new SotStorage( *xOleObjStm );
    if( xOleObjStor->GetError() )
        return NULL;

    String aStreamName;
    if( xOleObjStor->IsContained( String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "\002OlePres000" ) ) ) )
        aStreamName = String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "\002OlePres000" ) );
    else if( xOleObjStor->IsContained( String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "\1Ole10Native" ) ) ) )
        aStreamName = String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "\1Ole10Native" ) );

    if( aStreamName.Len() == 0 )
        return NULL;


    for( USHORT i = 1; i < 10; i++ )
    {
        SotStorageStreamRef xStm = xOleObjStor->OpenSotStream( aStreamName,
                                                STREAM_READ | STREAM_NOCREATE );
        if( xStm->GetError() )
            break;

        xStm->SetBufferSize( 8192 );
        Impl_OlePres * pEle = new Impl_OlePres( 0 );
        if( pEle->Read( *xStm ) && !xStm->GetError() )
        {
            if( pEle->GetFormat() == FORMAT_GDIMETAFILE || pEle->GetFormat() == FORMAT_BITMAP )
                return pEle;
        }
        delete pEle;
        aStreamName = String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "\002OlePres00" ) );
        aStreamName += String( i );
    };
    return NULL;
}



//---------------------------------------------------------------------------
//  Hilfs Klassen aus MSDFFDEF.HXX
//---------------------------------------------------------------------------

SvStream& operator>>( SvStream& rIn, DffRecordHeader& rRec )
{
    rRec.nFilePos = rIn.Tell();
    UINT16 nTmp(0);
    rIn >> nTmp;
    rRec.nImpVerInst = nTmp;
    rRec.nRecVer = sal::static_int_cast< BYTE >(nTmp & 0x000F);
    rRec.nRecInstance = nTmp >> 4;
    rIn >> rRec.nRecType;
    rIn >> rRec.nRecLen;
    return rIn;
}

// Masse fuer dashed lines
#define LLEN_MIDDLE         (450)
#define LLEN_SPACE_MIDDLE   (360)
#define LLEN_LONG           (LLEN_MIDDLE * 2)
#define LLEN_SPACE_LONG     (LLEN_SPACE_MIDDLE + 20)
#define LLEN_POINT          (LLEN_MIDDLE / 4)
#define LLEN_SPACE_POINT    (LLEN_SPACE_MIDDLE / 4)

SvStream& operator>>( SvStream& rIn, DffPropSet& rRec )
{
    rRec.InitializePropSet();

    DffRecordHeader aHd;
    rIn >> aHd;
    UINT32 nPropCount = aHd.nRecInstance;

    // FilePos der ComplexData merken
    UINT32 nComplexDataFilePos = rIn.Tell() + ( nPropCount * 6 );

    for( UINT32 nPropNum = 0; nPropNum < nPropCount; nPropNum++ )
    {
        sal_uInt16 nTmp;
        sal_uInt32 nRecType, nContent, nContentEx = 0xffff0000;
        rIn >> nTmp
            >> nContent;

        nRecType = nTmp & 0x3fff;

        if ( nRecType > 0x3ff )
            break;
        if ( ( nRecType & 0x3f ) == 0x3f )
        {	// clear flags that have to be cleared
            rRec.mpContents[ nRecType ] &= ( ( nContent >> 16 ) ^ 0xffffffff );
            // set flags that have to be set
            rRec.mpContents[ nRecType ] |= nContent;
            nContentEx |= ( nContent >> 16 );
            rRec.Replace( nRecType, (void*)nContentEx );
        }
        else
        {
            DffPropFlags aPropFlag = { 1, 0, 0, 0 };
            if ( nTmp & 0x4000 )
                aPropFlag.bBlip = sal_True;
            if ( nTmp & 0x8000 )
                aPropFlag.bComplex = sal_True;
            if ( aPropFlag.bComplex && nContent && ( nComplexDataFilePos < aHd.GetRecEndFilePos() ) )
            {
                // normally nContent is the complete size of the complex property,
                // but this is not always true for IMsoArrays ( what the hell is a IMsoArray ? )

                // I love special threatments :-(
                if ( ( nRecType == DFF_Prop_pVertices ) || ( nRecType == DFF_Prop_pSegmentInfo )
                    || ( nRecType == DFF_Prop_fillShadeColors ) || ( nRecType == DFF_Prop_lineDashStyle )
                        || ( nRecType == DFF_Prop_pWrapPolygonVertices ) || ( nRecType == DFF_Prop_connectorPoints )
                            || ( nRecType == DFF_Prop_Handles ) || ( nRecType == DFF_Prop_pFormulas )
                                || ( nRecType == DFF_Prop_textRectangles ) )
                {
                    // now check if the current content size is possible, or 6 bytes too small
                    sal_uInt32  nOldPos = rIn.Tell();
                    sal_Int16   nNumElem, nNumElemReserved, nSize;

                    rIn.Seek( nComplexDataFilePos );
                    rIn >>  nNumElem >> nNumElemReserved >> nSize;
                    if ( nNumElemReserved >= nNumElem )
                    {
                        // the size of these array elements is nowhere defined,
                        // what if the size is negative ?
                        // ok, we will make it positive and shift it.
                        // for -16 this works
                        if ( nSize < 0 )
                            nSize = ( -nSize ) >> 2;
                        sal_uInt32 nDataSize = (sal_uInt32)( nSize * nNumElem );

                        // sometimes the content size is 6 bytes too small (array header information is missing )
                        if ( nDataSize == nContent )
                            nContent += 6;

                        // check if array fits into the PropertyContainer
                        if ( ( nComplexDataFilePos + nContent ) > aHd.GetRecEndFilePos() )
                            nContent = 0;
                    }
                    else
                        nContent = 0;
                    rIn.Seek( nOldPos );
                }
                if ( nContent )
                {
                    nContentEx = nComplexDataFilePos;   // insert the filepos of this property;
                    nComplexDataFilePos += nContent;    // store filepos, that is used for the next complex property
                }
                else                                    // a complex property needs content
                    aPropFlag.bSet = sal_False;         // otherwise something is wrong
            }
            rRec.mpContents[ nRecType ] = nContent;
            rRec.mpFlags[ nRecType ] = aPropFlag;
            rRec.Insert( nRecType, (void*)nContentEx );
        }
    }
    aHd.SeekToEndOfRecord( rIn );
    return rIn;
}

void DffPropSet::InitializePropSet() const
{
    /*
    cmc:
    " Boolean properties are grouped in bitfields by property set; note that
    the Boolean properties in each property set are contiguous. They are saved
    under the property ID of the last Boolean property in the set, and are
    placed in the value field in reverse order starting with the last property
    in the low bit. "

    e.g.

    fEditedWrap
    fBehindDocument
    fOnDblClickNotify
    fIsButton
    fOneD
    fHidden
    fPrint

    are all part of a group and all are by default false except for fPrint,
    which equates to a default bit sequence for the group of 0000001 -> 0x1

    If at a later stage word sets fBehindDocument away from the default it
    will be done by having a property named fPrint whose bitsequence will have
    the fBehindDocument bit set. e.g. a DFF_Prop_fPrint with value 0x200020
    has set bit 6 on so as to enable fBehindDocument (as well as disabling
    everything else)
    */

    memset( ( (DffPropSet*) this )->mpFlags, 0, 0x400 * sizeof(DffPropFlags) );
    ( (DffPropSet*) this )->Clear();

    DffPropFlags nFlags = { 1, 0, 0, 1 };

    ( (DffPropSet*) this )->mpContents[ DFF_Prop_LockAgainstGrouping ] = 0x0000;		//0x01ff0000;
    ( (DffPropSet*) this )->mpFlags[ DFF_Prop_LockAgainstGrouping ] = nFlags;
    ( (DffPropSet*) this )->Insert( DFF_Prop_LockAgainstGrouping, (void*)0xffff0000 );

    ( (DffPropSet*) this )->mpContents[ DFF_Prop_FitTextToShape ] = 0x0010;				//0x001f0010;
    ( (DffPropSet*) this )->mpFlags[ DFF_Prop_FitTextToShape ] = nFlags;
    ( (DffPropSet*) this )->Insert( DFF_Prop_FitTextToShape, (void*)0xffff0000 );

    ( (DffPropSet*) this )->mpContents[ DFF_Prop_gtextFStrikethrough ] = 0x0000;		//0xffff0000;
    ( (DffPropSet*) this )->mpFlags[ DFF_Prop_gtextFStrikethrough ] = nFlags;
    ( (DffPropSet*) this )->Insert( DFF_Prop_gtextFStrikethrough, (void*)0xffff0000 );

    ( (DffPropSet*) this )->mpContents[ DFF_Prop_pictureActive ] = 0x0000;				//0x000f0000;
    ( (DffPropSet*) this )->mpFlags[ DFF_Prop_pictureActive ] = nFlags;
    ( (DffPropSet*) this )->Insert( DFF_Prop_pictureActive, (void*)0xffff0000 );

    ( (DffPropSet*) this )->mpContents[ DFF_Prop_fFillOK ] = 0x0039;					//0x003f0039;
    ( (DffPropSet*) this )->mpFlags[ DFF_Prop_fFillOK ] = nFlags;
    ( (DffPropSet*) this )->Insert( DFF_Prop_fFillOK, (void*)0xffff0000 );

    ( (DffPropSet*) this )->mpContents[ DFF_Prop_fNoFillHitTest ] = 0x001c;				//0x001f001c;
    ( (DffPropSet*) this )->mpFlags[ DFF_Prop_fNoFillHitTest ] = nFlags;
    ( (DffPropSet*) this )->Insert( DFF_Prop_fNoFillHitTest, (void*)0xffff0000 );

    ( (DffPropSet*) this )->mpContents[ DFF_Prop_fNoLineDrawDash ] = 0x001e;			//0x001f000e;
    ( (DffPropSet*) this )->mpFlags[ DFF_Prop_fNoLineDrawDash ] = nFlags;
    ( (DffPropSet*) this )->Insert( DFF_Prop_fNoLineDrawDash, (void*)0xffff0000 );

    ( (DffPropSet*) this )->mpContents[ DFF_Prop_fshadowObscured ] = 0x0000;			//0x00030000;
    ( (DffPropSet*) this )->mpFlags[ DFF_Prop_fshadowObscured ] = nFlags;
    ( (DffPropSet*) this )->Insert( DFF_Prop_fshadowObscured, (void*)0xffff0000 );

    ( (DffPropSet*) this )->mpContents[ DFF_Prop_fPerspective ] = 0x0000;				//0x00010000;
    ( (DffPropSet*) this )->mpFlags[ DFF_Prop_fPerspective ] = nFlags;
    ( (DffPropSet*) this )->Insert( DFF_Prop_fPerspective, (void*)0xffff0000 );

    ( (DffPropSet*) this )->mpContents[ DFF_Prop_fc3DLightFace ] = 0x0001;				//0x000f0001;
    ( (DffPropSet*) this )->mpFlags[ DFF_Prop_fc3DLightFace ] = nFlags;
    ( (DffPropSet*) this )->Insert( DFF_Prop_fc3DLightFace, (void*)0xffff0000 );

    ( (DffPropSet*) this )->mpContents[ DFF_Prop_fc3DFillHarsh ] = 0x0016;				//0x001f0016;
    ( (DffPropSet*) this )->mpFlags[ DFF_Prop_fc3DFillHarsh ] = nFlags;
    ( (DffPropSet*) this )->Insert( DFF_Prop_fc3DFillHarsh, (void*)0xffff0000 );

    ( (DffPropSet*) this )->mpContents[ DFF_Prop_fBackground ] = 0x0000;				//0x001f0000;
    ( (DffPropSet*) this )->mpFlags[ DFF_Prop_fBackground ] = nFlags;
    ( (DffPropSet*) this )->Insert( DFF_Prop_fBackground, (void*)0xffff0000 );

    ( (DffPropSet*) this )->mpContents[ DFF_Prop_fCalloutLengthSpecified ] = 0x0010;	//0x00ef0010;
    ( (DffPropSet*) this )->mpFlags[ DFF_Prop_fCalloutLengthSpecified ] = nFlags;
    ( (DffPropSet*) this )->Insert( DFF_Prop_fCalloutLengthSpecified, (void*)0xffff0000 );

    ( (DffPropSet*) this )->mpContents[ DFF_Prop_fPrint ] = 0x0001;						//0x00ef0001;
    ( (DffPropSet*) this )->mpFlags[ DFF_Prop_fPrint ] = nFlags;
    ( (DffPropSet*) this )->Insert( DFF_Prop_fPrint, (void*)0xffff0000 );

    ( (DffPropSet*) this )->mpContents[ DFF_Prop_fillColor ] = 0xffffff;
    ( (DffPropSet*) this )->mpFlags[ DFF_Prop_fillColor ] = nFlags;
    ( (DffPropSet*) this )->Insert( DFF_Prop_fillColor, (void*)0xffff0000 );
}

void DffPropSet::Merge( DffPropSet& rMaster ) const
{
    for ( void* pDummy = rMaster.First(); pDummy; pDummy = rMaster.Next() )
    {
        UINT32 nRecType = rMaster.GetCurKey();
        if ( ( nRecType & 0x3f ) == 0x3f )		// this is something called FLAGS
        {
            UINT32 nCurrentFlags = mpContents[ nRecType ];
            UINT32 nMergeFlags = rMaster.mpContents[ nRecType ];
            nMergeFlags &=  ( nMergeFlags >> 16 ) | 0xffff0000;				// clearing low word
            nMergeFlags &= ( ( nCurrentFlags & 0xffff0000 )					// remove allready hard set
                            | ( nCurrentFlags >> 16 ) ) ^ 0xffffffff;		// attributes from mergeflags
            nCurrentFlags &= ( ( nMergeFlags & 0xffff0000 )					// apply zero master bits
                            | ( nMergeFlags >> 16 ) ) ^ 0xffffffff;
            nCurrentFlags |= (UINT16)nMergeFlags;							// apply filled master bits
            ( (DffPropSet*) this )->mpContents[ nRecType ] = nCurrentFlags;


            sal_uInt32 nNewContentEx = (sal_uInt32)(sal_uIntPtr)rMaster.GetCurObject();
            if ( ((DffPropSet*)this)->Seek( nRecType ) )
                nNewContentEx |= (sal_uInt32)(sal_uIntPtr)GetCurObject();
            ( (DffPropSet*) this )->Replace( nRecType, (void*)nNewContentEx );
        }
        else
        {
            if ( !IsProperty( nRecType ) || !IsHardAttribute( nRecType ) )
            {
                ( (DffPropSet*) this )->mpContents[ nRecType ] = rMaster.mpContents[ nRecType ];
                DffPropFlags nFlags( rMaster.mpFlags[ nRecType ] );
                nFlags.bSoftAttr = TRUE;
                ( (DffPropSet*) this )->mpFlags[ nRecType ] = nFlags;
                ( (DffPropSet*) this )->Insert( nRecType, pDummy );
            }
        }
    }
}

BOOL DffPropSet::IsHardAttribute( UINT32 nId ) const
{
    BOOL bRetValue = TRUE;
    nId &= 0x3ff;
    if ( ( nId & 0x3f ) >= 48 )	// is this a flag id
    {
        if ( ((DffPropSet*)this)->Seek( nId | 0x3f ) )
        {
            sal_uInt32 nContentEx = (sal_uInt32)(sal_uIntPtr)GetCurObject();
            bRetValue = ( nContentEx & ( 1 << ( 0xf - ( nId & 0xf ) ) ) ) != 0;
        }
    }
    else
        bRetValue = ( mpFlags[ nId ].bSoftAttr == 0 );
    return bRetValue;
};

UINT32 DffPropSet::GetPropertyValue( UINT32 nId, UINT32 nDefault ) const
{
    nId &= 0x3ff;
    return ( mpFlags[ nId ].bSet ) ? mpContents[ nId ] : nDefault;
};

bool DffPropSet::GetPropertyBool( UINT32 nId, bool bDefault ) const
{
    UINT32 nBaseId = nId | 31;              // base ID to get the UINT32 property value
    UINT32 nMask = 1 << (nBaseId - nId);    // bit mask of the boolean property

    UINT32 nPropValue = GetPropertyValue( nBaseId, bDefault ? nMask : 0 );
    return (nPropValue & nMask) != 0;
}

::rtl::OUString DffPropSet::GetPropertyString( UINT32 nId, SvStream& rStrm ) const
{
    sal_Size nOldPos = rStrm.Tell();
    ::rtl::OUStringBuffer aBuffer;
    sal_uInt32 nBufferSize = GetPropertyValue( nId );
    if( (nBufferSize > 0) && SeekToContent( nId, rStrm ) )
    {
        sal_Int32 nStrLen = static_cast< sal_Int32 >( nBufferSize / 2 );
        aBuffer.ensureCapacity( nStrLen );
        for( sal_Int32 nCharIdx = 0; nCharIdx < nStrLen; ++nCharIdx )
        {
            sal_uInt16 nChar = 0;
            rStrm >> nChar;
            if( nChar > 0 )
                aBuffer.append( static_cast< sal_Unicode >( nChar ) );
            else
                break;
        }
    }
    rStrm.Seek( nOldPos );
    return aBuffer.makeStringAndClear();
}

void DffPropSet::SetPropertyValue( UINT32 nId, UINT32 nValue ) const
{
    if ( !mpFlags[ nId ].bSet )
    {
        ( (DffPropSet*) this )->Insert( nId, (void*)nValue );
        ( (DffPropSet*) this )->mpFlags[ nId ].bSet = TRUE;
    }
    ( (DffPropSet*) this )->mpContents[ nId ] = nValue;
};

BOOL DffPropSet::SeekToContent( UINT32 nRecType, SvStream& rStrm ) const
{
    nRecType &= 0x3ff;
    if ( mpFlags[ nRecType ].bSet )
    {
        if ( mpFlags[ nRecType ].bComplex )
        {
            if ( ((DffPropSet*)this)->Seek( nRecType ) )
            {
                sal_uInt32 nOffset = (sal_uInt32)(sal_uIntPtr)GetCurObject();
                if ( nOffset && ( ( nOffset & 0xffff0000 ) != 0xffff0000 ) )
                {
                    rStrm.Seek( nOffset );
                    return TRUE;
                }
            }
        }
    }
    return FALSE;
}

DffPropertyReader::DffPropertyReader( const SvxMSDffManager& rMan ) :
    rManager( rMan ),
    pDefaultPropSet( NULL )
{
    InitializePropSet();
}

void DffPropertyReader::SetDefaultPropSet( SvStream& rStCtrl, UINT32 nOffsDgg ) const
{
    delete pDefaultPropSet;
    UINT32 nMerk = rStCtrl.Tell();
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
void DffPropertyReader::ReadPropSet( SvStream& rIn, void* pClientData, UINT32 nShapeId ) const
#else
void DffPropertyReader::ReadPropSet( SvStream& rIn, void* pClientData ) const
#endif
{
    ULONG nFilePos = rIn.Tell();
    rIn >> (DffPropertyReader&)*this;

    if ( IsProperty( DFF_Prop_hspMaster ) )
    {
        if ( rManager.SeekToShape( rIn, pClientData, GetPropertyValue( DFF_Prop_hspMaster ) ) )
        {
            DffRecordHeader aRecHd;
            rIn >> aRecHd;
            if ( rManager.SeekToRec( rIn, DFF_msofbtOPT, aRecHd.GetRecEndFilePos() ) )
            {
                DffPropSet aMasterPropSet;
                rIn >> aMasterPropSet;
                Merge( aMasterPropSet );
            }
        }
    }
//	if ( pDefaultPropSet )
//		Merge( *( pDefaultPropSet ) );

    ( (DffPropertyReader*) this )->mnFix16Angle = Fix16ToAngle( GetPropertyValue( DFF_Prop_Rotation, 0 ) );

#ifdef DBG_CUSTOMSHAPE

    String aURLStr;

    if( ::utl::LocalFileHelper::ConvertPhysicalNameToURL( String( RTL_CONSTASCII_STRINGPARAM( "d:\\ashape.dbg" ) ), aURLStr ) )
    {
        SvStream* pOut = ::utl::UcbStreamHelper::CreateStream( aURLStr, STREAM_WRITE );

        if( pOut )
        {
            pOut->Seek( STREAM_SEEK_TO_END );

            if ( IsProperty( DFF_Prop_adjustValue ) || IsProperty( DFF_Prop_pVertices ) )
            {
                pOut->WriteLine( "" );
                ByteString aString( "ShapeId: " );
                aString.Append( ByteString::CreateFromInt32( nShapeId ) );
                pOut->WriteLine( aString );
            }
            for ( sal_uInt32 i = DFF_Prop_adjustValue; i <=	DFF_Prop_adjust10Value; i++ )
            {
                if ( IsProperty( i ) )
                {
                    ByteString aString( "Prop_adjustValue" );
                    aString.Append( ByteString::CreateFromInt32( ( i - DFF_Prop_adjustValue ) + 1 ) );
                    aString.Append( ":" );
                    aString.Append( ByteString::CreateFromInt32( GetPropertyValue( i ) ) );
                    pOut->WriteLine( aString );
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
                        INT32 nLen = (INT32)GetPropertyValue( i );
                        if ( nLen )
                        {
                            pOut->WriteLine( "" );
                            ByteString aDesc( "Property:" );
                            aDesc.Append( ByteString::CreateFromInt32( i ) );
                            aDesc.Append( ByteString( "  Size:" ) );
                            aDesc.Append( ByteString::CreateFromInt32( nLen ) );
                            pOut->WriteLine( aDesc );
                            INT16	nNumElem, nNumElemMem, nNumSize;
                            rIn >> nNumElem >> nNumElemMem >> nNumSize;
                            aDesc = ByteString( "Entries: " );
                            aDesc.Append( ByteString::CreateFromInt32( nNumElem ) );
                            aDesc.Append( ByteString(  "  Size:" ) );
                            aDesc.Append( ByteString::CreateFromInt32( nNumSize ) );
                            pOut->WriteLine( aDesc );
                            if ( nNumSize < 0 )
                                nNumSize = ( ( -nNumSize ) >> 2 );
                            if ( !nNumSize )
                                nNumSize = 16;
                            nLen -= 6;
                            while ( nLen > 0 )
                            {
                                ByteString aString;
                                for ( UINT32 j = 0; nLen && ( j < ( nNumSize >> 1 ) ); j++ )
                                {
                                    for ( UINT32 k = 0; k < 2; k++ )
                                    {
                                        if ( nLen )
                                        {
                                            BYTE nVal;
                                            rIn >> nVal;
                                            if ( ( nVal >> 4 ) > 9 )
                                                *pOut << (BYTE)( ( nVal >> 4 ) + 'A' - 10 );
                                            else
                                                *pOut << (BYTE)( ( nVal >> 4 ) + '0' );

                                            if ( ( nVal & 0xf ) > 9 )
                                                *pOut << (BYTE)( ( nVal & 0xf ) + 'A' - 10 );
                                            else
                                                *pOut << (BYTE)( ( nVal & 0xf ) + '0' );

                                            nLen--;
                                        }
                                    }
                                    *pOut << (char)( ' ' );
                                }
                                pOut->WriteLine( aString );
                            }
                        }
                    }
                    else
                    {
                        ByteString aString( "Property" );
                        aString.Append( ByteString::CreateFromInt32( i ) );
                        aString.Append( ":" );
                        aString.Append( ByteString::CreateFromInt32( GetPropertyValue( i ) ) );
                        pOut->WriteLine( aString );
                    }
                }
            }

            delete pOut;
        }
    }

#endif

    rIn.Seek( nFilePos );
}


INT32 DffPropertyReader::Fix16ToAngle( INT32 nContent ) const
{
    INT32 nAngle = 0;
    if ( nContent )
    {
        nAngle = ( (INT16)( nContent >> 16) * 100L ) + ( ( ( nContent & 0x0000ffff) * 100L ) >> 16 );
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
    for ( SvxMSDffConnectorRule* pPtr = (SvxMSDffConnectorRule*)aCList.First();
            pPtr; pPtr = (SvxMSDffConnectorRule*)aCList.Next() )
        delete pPtr;
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
                rContainer.aCList.Insert( pRule, LIST_APPEND );
            }
            aCRule.SeekToEndOfRecord( rIn );
        }
    }
    return rIn;
}

void SvxMSDffManager::SolveSolver( const SvxMSDffSolverContainer& rSolver )
{
    sal_Int32 i, nCnt;
    for ( i = 0, nCnt = rSolver.aCList.Count(); i < nCnt; i++ )
    {
        SvxMSDffConnectorRule* pPtr = (SvxMSDffConnectorRule*)rSolver.aCList.GetObject( i );
        if ( pPtr->pCObj )
        {
            for ( int nN = 0; nN < 2; nN++ )
            {
                SdrObject*  pO;
                sal_uInt32  nC, nSpFlags;
                sal_Bool    bTail;
                if ( !nN )
                {
                    bTail = sal_True;
                    pO = pPtr->pAObj;
                    nC = pPtr->ncptiA;
                    nSpFlags = pPtr->nSpFlagsA;
                }
                else
                {
                    bTail = sal_False;
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
                                        nId = 0;	// SDRVERTALIGN_TOP;
                                    break;
                                    case 1 :
                                        nId = 3;	// SDRHORZALIGN_RIGHT;
                                    break;
                                    case 2 :
                                        nId = 2;	// SDRVERTALIGN_BOTTOM;
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
                                        sal_uInt32  nPointCount = 0;
                                        Rectangle aBoundRect( aPolyPoly.GetBoundRect() );
                                        if ( aBoundRect.GetWidth() && aBoundRect.GetHeight() )
                                        {
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
                                const rtl::OUString	sPath( RTL_CONSTASCII_USTRINGPARAM ( "Path" ) );
                                const rtl::OUString	sGluePointType( RTL_CONSTASCII_USTRINGPARAM ( "GluePointType" ) );
                                sal_Int16 nGluePointType = EnhancedCustomShapeGluePointType::SEGMENTS;
                                com::sun::star::uno::Any* pAny = aGeometryItem.GetPropertyValueByName( sPath, sGluePointType );
                                if ( pAny )
                                    *pAny >>= nGluePointType;
                                else
                                {
                                    const rtl::OUString	sType( RTL_CONSTASCII_USTRINGPARAM ( "Type" ) );
                                    rtl::OUString sShapeType;
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
                                            nId = 0;	// SDRVERTALIGN_TOP;
                                        break;
                                        case 1 :
                                            nId = 3;	// SDRHORZALIGN_RIGHT;
                                        break;
                                        case 2 :
                                            nId = 2;	// SDRVERTALIGN_BOTTOM;
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
                                    const rtl::OUString	sSegments( RTL_CONSTASCII_USTRINGPARAM ( "Segments" ) );
                                    const rtl::OUString	sCoordinates( RTL_CONSTASCII_USTRINGPARAM ( "Coordinates" ) );

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
                                                const rtl::OUString	sGluePoints( RTL_CONSTASCII_USTRINGPARAM ( "GluePoints" ) );
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
                                    String aPropName( RTL_CONSTASCII_USTRINGPARAM( "EndShape" ) );
                                    aAny <<= aXShape;
                                    SetPropValue( aAny, xPropSet, aPropName, sal_True );
                                    aPropName  = String( RTL_CONSTASCII_USTRINGPARAM( "EndGluePointIndex" ) );
                                    aAny <<= nId;
                                    SetPropValue( aAny, xPropSet, aPropName, sal_True );
                                }
                                else
                                {
                                    String aPropName( RTL_CONSTASCII_USTRINGPARAM( "StartShape" ) );
                                    aAny <<= aXShape;
                                    SetPropValue( aAny, xPropSet, aPropName, sal_True );
                                    aPropName = String( RTL_CONSTASCII_USTRINGPARAM( "StartGluePointIndex" ) );
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

static basegfx::B2DPolygon GetLineArrow( const sal_Int32 nLineWidth, const MSO_LineEnd eLineEnd,
    const MSO_LineEndWidth eLineWidth, const MSO_LineEndLength eLineLenght,
    sal_Int32& rnArrowWidth, sal_Bool& rbArrowCenter,
    String& rsArrowName, sal_Bool bScaleArrow )
{
    basegfx::B2DPolygon aRetval;
    double		fLineWidth = nLineWidth < 70 ? 70.0 : nLineWidth;
    double		fLenghtMul, fWidthMul;
    sal_Int32	nLineNumber;
    switch( eLineLenght )
    {
        default :
        case mso_lineMediumLenArrow		: fLenghtMul = 3.0; nLineNumber = 2; break;
        case mso_lineShortArrow			: fLenghtMul = 2.0; nLineNumber = 1; break;
        case mso_lineLongArrow			: fLenghtMul = 5.0; nLineNumber = 3; break;
    }
    switch( eLineWidth )
    {
        default :
        case mso_lineMediumWidthArrow	: fWidthMul = 3.0; nLineNumber += 3; break;
        case mso_lineNarrowArrow		: fWidthMul = 2.0; break;
        case mso_lineWideArrow			: fWidthMul = 5.0; nLineNumber += 6; break;
    }

    if ( bScaleArrow )	// #i33630 arrows imported from Word are too big
    {
        fWidthMul /= 1.75;
        fLenghtMul/= 1.75;
    }

    rbArrowCenter = sal_False;
    switch ( eLineEnd )
    {
        case mso_lineArrowEnd :
        {
            basegfx::B2DPolygon aTriangle;
            aTriangle.append(basegfx::B2DPoint( fWidthMul * fLineWidth * 0.50, 0.0 ));
            aTriangle.append(basegfx::B2DPoint( fWidthMul * fLineWidth, fLenghtMul * fLineWidth ));
            aTriangle.append(basegfx::B2DPoint( 0.0, fLenghtMul * fLineWidth ));
            aTriangle.setClosed(true);
            aRetval = aTriangle;
            rsArrowName = String( RTL_CONSTASCII_STRINGPARAM( "msArrowEnd " ), RTL_TEXTENCODING_UTF8 );
        }
        break;

        case mso_lineArrowOpenEnd :
        {
            switch( eLineLenght )
            {
                default :
                case mso_lineMediumLenArrow		: fLenghtMul = 4.5; break;
                case mso_lineShortArrow			: fLenghtMul = 3.5; break;
                case mso_lineLongArrow			: fLenghtMul = 6.0; break;
            }
            switch( eLineWidth )
            {
                default :
                case mso_lineMediumWidthArrow	: fWidthMul = 4.5; break;
                case mso_lineNarrowArrow		: fWidthMul = 3.5; break;
                case mso_lineWideArrow			: fWidthMul = 6.0; break;
            }
            basegfx::B2DPolygon aTriangle;
            aTriangle.append(basegfx::B2DPoint( fWidthMul * fLineWidth * 0.50 , 0.0 ));
            aTriangle.append(basegfx::B2DPoint( fWidthMul * fLineWidth, fLenghtMul * fLineWidth * 0.91 ));
            aTriangle.append(basegfx::B2DPoint( fWidthMul * fLineWidth * 0.85, fLenghtMul * fLineWidth ));
            aTriangle.append(basegfx::B2DPoint( fWidthMul * fLineWidth * 0.50, fLenghtMul * fLineWidth * 0.36 ));
            aTriangle.append(basegfx::B2DPoint( fWidthMul * fLineWidth * 0.15, fLenghtMul * fLineWidth ));
            aTriangle.append(basegfx::B2DPoint( 0.0, fLenghtMul * fLineWidth * 0.91 ));
            aTriangle.setClosed(true);
            aRetval = aTriangle;
            rsArrowName = String( RTL_CONSTASCII_STRINGPARAM( "msArrowOpenEnd " ), RTL_TEXTENCODING_UTF8 );
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
            aRetval = aTriangle;
            rsArrowName = String( RTL_CONSTASCII_STRINGPARAM( "msArrowStealthEnd " ), RTL_TEXTENCODING_UTF8 );
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
            aRetval = aTriangle;
            rbArrowCenter = sal_True;
            rsArrowName = String( RTL_CONSTASCII_STRINGPARAM( "msArrowDiamondEnd " ), RTL_TEXTENCODING_UTF8 );
        }
        break;
        case mso_lineArrowOvalEnd :
        {
            aRetval = XPolygon( Point( (sal_Int32)( fWidthMul * fLineWidth * 0.50 ), 0 ),
                                (sal_Int32)( fWidthMul * fLineWidth * 0.50 ),
                                    (sal_Int32)( fLenghtMul * fLineWidth * 0.50 ), 0, 3600 ).getB2DPolygon();
            rbArrowCenter = sal_True;
            rsArrowName = String( RTL_CONSTASCII_STRINGPARAM( "msArrowOvalEnd " ), RTL_TEXTENCODING_UTF8 );
        }
        break;
        default: break;
    }
    rsArrowName.Append( String::CreateFromInt32( nLineNumber ) );
    rnArrowWidth = (sal_Int32)( fLineWidth * fWidthMul );

    return aRetval;
}

void DffPropertyReader::ApplyLineAttributes( SfxItemSet& rSet, const MSO_SPT eShapeType ) const // #i28269#
{
    UINT32 nLineFlags(GetPropertyValue( DFF_Prop_fNoLineDrawDash ));

    if(!IsHardAttribute( DFF_Prop_fLine ) && !IsCustomShapeStrokedByDefault( eShapeType ))
    {
        nLineFlags &= ~0x08;
    }

    if ( nLineFlags & 8 )
    {
        // Linienattribute
        sal_Int32 nLineWidth = (INT32)GetPropertyValue( DFF_Prop_lineWidth, 9525 );

        MSO_LineDashing eLineDashing = (MSO_LineDashing)GetPropertyValue( DFF_Prop_lineDashing, mso_lineSolid );
        if ( eLineDashing == mso_lineSolid )
            rSet.Put(XLineStyleItem( XLINE_SOLID ) );
        else
        {
//			MSO_LineCap eLineCap = (MSO_LineCap)GetPropertyValue( DFF_Prop_lineEndCapStyle, mso_lineEndCapSquare );

            XDashStyle  eDash = XDASH_RECT;
            sal_uInt16	nDots = 1;
            sal_uInt32	nDotLen	= nLineWidth / 360;
            sal_uInt16	nDashes = 0;
            sal_uInt32	nDashLen = ( 8 * nLineWidth ) / 360;
            sal_uInt32	nDistance = ( 3 * nLineWidth ) / 360;;

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

            rSet.Put( XLineDashItem( String(), XDash( eDash, nDots, nDotLen, nDashes, nDashLen, nDistance ) ) );
            rSet.Put( XLineStyleItem( XLINE_DASH ) );
        }
        rSet.Put( XLineColorItem( String(), rManager.MSO_CLR_ToColor( GetPropertyValue( DFF_Prop_lineColor ), DFF_Prop_lineColor ) ) );
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
        XLineJoint eXLineJoint( XLINEJOINT_MITER );
        if ( eLineJoint == mso_lineJoinBevel )
            eXLineJoint = XLINEJOINT_BEVEL;
        else if ( eLineJoint == mso_lineJoinRound )
            eXLineJoint = XLINEJOINT_ROUND;
        rSet.Put( XLineJointItem( eXLineJoint ) );

        if ( nLineFlags & 0x10 )
        {
            sal_Bool bScaleArrows = rManager.pSdrModel->GetScaleUnit() == MAP_TWIP;
            ///////////////
            // LineStart //
            ///////////////
            if ( IsProperty( DFF_Prop_lineStartArrowhead ) )
            {
                MSO_LineEnd			eLineEnd = (MSO_LineEnd)GetPropertyValue( DFF_Prop_lineStartArrowhead );
                MSO_LineEndWidth	eWidth = (MSO_LineEndWidth)GetPropertyValue( DFF_Prop_lineStartArrowWidth, mso_lineMediumWidthArrow );
                MSO_LineEndLength	eLenght = (MSO_LineEndLength)GetPropertyValue( DFF_Prop_lineStartArrowLength, mso_lineMediumLenArrow );

                sal_Int32	nArrowWidth;
                sal_Bool	bArrowCenter;
                String		aArrowName;
                basegfx::B2DPolygon aPoly(GetLineArrow( nLineWidth, eLineEnd, eWidth, eLenght, nArrowWidth, bArrowCenter, aArrowName, bScaleArrows ));

                rSet.Put( XLineStartWidthItem( nArrowWidth ) );
                rSet.Put( XLineStartItem( aArrowName, basegfx::B2DPolyPolygon(aPoly) ) );
                rSet.Put( XLineStartCenterItem( bArrowCenter ) );
            }
            /////////////
            // LineEnd //
            /////////////
            if ( IsProperty( DFF_Prop_lineEndArrowhead ) )
            {
                MSO_LineEnd			eLineEnd = (MSO_LineEnd)GetPropertyValue( DFF_Prop_lineEndArrowhead );
                MSO_LineEndWidth	eWidth = (MSO_LineEndWidth)GetPropertyValue( DFF_Prop_lineEndArrowWidth, mso_lineMediumWidthArrow );
                MSO_LineEndLength	eLenght = (MSO_LineEndLength)GetPropertyValue( DFF_Prop_lineEndArrowLength, mso_lineMediumLenArrow );

                sal_Int32	nArrowWidth;
                sal_Bool	bArrowCenter;
                String		aArrowName;
                basegfx::B2DPolygon aPoly(GetLineArrow( nLineWidth, eLineEnd, eWidth, eLenght, nArrowWidth, bArrowCenter, aArrowName, bScaleArrows ));

                rSet.Put( XLineEndWidthItem( nArrowWidth ) );
                rSet.Put( XLineEndItem( aArrowName, basegfx::B2DPolyPolygon(aPoly) ) );
                rSet.Put( XLineEndCenterItem( bArrowCenter ) );
            }
            if ( IsProperty( DFF_Prop_lineEndCapStyle ) )
            {
                MSO_LineCap eLineCap = (MSO_LineCap)GetPropertyValue( DFF_Prop_lineEndCapStyle );
                const SfxPoolItem* pPoolItem = NULL;
                if ( rSet.GetItemState( XATTR_LINEDASH, FALSE, &pPoolItem ) == SFX_ITEM_SET )
                {
                    XDashStyle eNewStyle = XDASH_RECT;
                    if ( eLineCap == mso_lineEndCapRound )
                        eNewStyle = XDASH_ROUND;
                    const XDash& rOldDash = ( (const XLineDashItem*)pPoolItem )->GetDashValue();
                    if ( rOldDash.GetDashStyle() != eNewStyle )
                    {
                        XDash aNew( rOldDash );
                        aNew.SetDashStyle( eNewStyle );
                        rSet.Put( XLineDashItem( XubString(), aNew ) );
                    }
                }
            }
        }
    }
    else
        rSet.Put( XLineStyleItem( XLINE_NONE ) );
}

struct ShadeColor
{
    Color		aColor;
    double		fDist;

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
                sal_Int32	nColor;
                sal_Int32	nDist;

                rIn >> nColor >> nDist;
                rShadeColors.push_back( ShadeColor( rManager.MSO_CLR_ToColor( nColor, DFF_Prop_fillColor ), 1.0 - ( nDist / 65536.0 ) ) );
            }
        }
    }
    if ( !rShadeColors.size() )
    {
        rShadeColors.push_back( ShadeColor( rManager.MSO_CLR_ToColor( rProperties.GetPropertyValue( DFF_Prop_fillBackColor, COL_WHITE ), DFF_Prop_fillBackColor ), 0 ) );
        rShadeColors.push_back( ShadeColor( rManager.MSO_CLR_ToColor( rProperties.GetPropertyValue( DFF_Prop_fillColor, COL_WHITE ), DFF_Prop_fillColor ), 1 ) );
    }
    rIn.Seek( nPos );
}

struct QuantErr
{
    double	fRed;
    double	fGreen;
    double	fBlue;

    QuantErr() : fRed( 0.0 ), fGreen( 0.0 ), fBlue( 0.0 ){};
};

void ApplyRectangularGradientAsBitmap( const SvxMSDffManager& rManager, SvStream& rIn, SfxItemSet& rSet, const std::vector< ShadeColor >& rShadeColors, const DffObjData& rObjData, sal_Int32 nFix16Angle )
{
    Size aBitmapSizePixel( static_cast< sal_Int32 >( ( rObjData.aBoundRect.GetWidth() / 2540.0 ) * 90.0 ),		// we will create a bitmap with 90 dpi
                           static_cast< sal_Int32 >( ( rObjData.aBoundRect.GetHeight() / 2540.0 ) * 90.0 ) );
    if ( aBitmapSizePixel.Width() && aBitmapSizePixel.Height() && ( aBitmapSizePixel.Width() <= 1024 ) && ( aBitmapSizePixel.Height() <= 1024 ) )
    {
//		std::vector< QuantErr > aQuantErrCurrScan( aBitmapSizePixel.Width() + 1 );
//		std::vector< QuantErr > aQuantErrNextScan( aBitmapSizePixel.Width() + 1 );

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
                        aIter++;
                    }			
                    double fRed = aColorA.GetRed(), fGreen = aColorA.GetGreen(), fBlue = aColorA.GetBlue();
                    double fD1 = fB - fA;
                    if ( fD1 != 0.0 )
                    {
                        fRed   += ( ( ( fDist - fA ) * ( aColorB.GetRed() - aColorA.GetRed() ) ) / fD1 );		// + aQuantErrCurrScan[ nX ].fRed;
                        fGreen += ( ( ( fDist - fA ) * ( aColorB.GetGreen() - aColorA.GetGreen() ) ) / fD1 );	// + aQuantErrCurrScan[ nX ].fGreen;
                        fBlue  += ( ( ( fDist - fA ) * ( aColorB.GetBlue() - aColorA.GetBlue() ) ) / fD1 );		// + aQuantErrCurrScan[ nX ].fBlue;
                    }
                    sal_Int16 nRed   = static_cast< sal_Int16 >( fRed   + 0.5 );
                    sal_Int16 nGreen = static_cast< sal_Int16 >( fGreen + 0.5 );
                    sal_Int16 nBlue  = static_cast< sal_Int16 >( fBlue  + 0.5 );
/*
                    double fErr = fRed - nRed;
                    aQuantErrCurrScan[ nX + 1 ].fRed += 7.0 * fErr / 16.0;
                    if ( nX )
                        aQuantErrNextScan[ nX - 1 ].fRed += 3.0 * fErr / 16.0;
                    aQuantErrNextScan[ nX ].fRed += 5.0 * fErr / 16.0;
                    aQuantErrNextScan[ nX + 1 ].fRed += 1.0 * fErr / 16.0;

                    fErr = fGreen - nGreen;
                    aQuantErrCurrScan[ nX + 1 ].fGreen += 7.0 * fErr / 16.0;
                    if ( nX )
                        aQuantErrNextScan[ nX - 1 ].fGreen += 3.0 * fErr / 16.0;
                    aQuantErrNextScan[ nX ].fGreen += 5.0 * fErr / 16.0;
                    aQuantErrNextScan[ nX + 1 ].fGreen += 1.0 * fErr / 16.0;

                    fErr = fBlue - nBlue;
                    aQuantErrCurrScan[ nX + 1 ].fBlue += 7.0 * fErr / 16.0;
                    if ( nX )
                        aQuantErrNextScan[ nX - 1 ].fBlue += 3.0 * fErr / 16.0;
                    aQuantErrNextScan[ nX ].fBlue += 5.0 * fErr / 16.0;
                    aQuantErrNextScan[ nX + 1 ].fBlue += 1.0 * fErr / 16.0;
*/					
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
/*
                aQuantErrCurrScan.swap( aQuantErrNextScan );
                std::vector< QuantErr >::iterator aIter( aQuantErrNextScan.begin() );
                while( aIter != aQuantErrNextScan.end() )
                {
                    *aIter = QuantErr();
                    aIter++;
                }
*/
            }
            aBitmap.ReleaseAccess( pAcc );

            if ( nFix16Angle )
            {
                sal_Bool bRotateWithShape = sal_True;	// TRUE seems to be default
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
        
                    ULONG nMirrorFlags = BMP_MIRROR_NONE;
                    if ( rObjData.nSpFlags & SP_FFLIPV )
                        nMirrorFlags |= BMP_MIRROR_VERT;
                    if ( rObjData.nSpFlags & SP_FFLIPH )
                        nMirrorFlags |= BMP_MIRROR_HORZ;
                    if ( nMirrorFlags != BMP_MIRROR_NONE )
                        aBitmap.Mirror( nMirrorFlags );
                }
            }

            XOBitmap aXBmp( aBitmap, XBITMAP_STRETCH );
            rSet.Put( XFillBmpTileItem( sal_False ) );
            rSet.Put( XFillBitmapItem( String(), aXBmp ) );
        }
    }
}

void DffPropertyReader::ApplyFillAttributes( SvStream& rIn, SfxItemSet& rSet, const DffObjData& rObjData ) const
{
    UINT32 nFillFlags(GetPropertyValue( DFF_Prop_fNoFillHitTest ));

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
            case mso_fillSolid :			// Fill with a solid color
                eXFill = XFILL_SOLID;
            break;
            case mso_fillPattern :			// Fill with a pattern (bitmap)
            case mso_fillTexture :			// A texture (pattern with its own color map)
            case mso_fillPicture :			// Center a picture in the shape
                eXFill = XFILL_BITMAP;
            break;
            case mso_fillShadeCenter :		// Shade from bounding rectangle to end point
            {
                if ( rObjData.aBoundRect.IsEmpty() )// size of object needed to be able
                    eXFill = XFILL_GRADIENT;		// to create a bitmap substitution
                else
                    eXFill = XFILL_BITMAP;
            }
            break;
            case mso_fillShade :			// Shade from start to end points
            case mso_fillShadeShape :		// Shade from shape outline to end point
            case mso_fillShadeScale :		// Similar to mso_fillShade, but the fillAngle
            case mso_fillShadeTitle :		// special type - shade to title ---  for PP
                eXFill = XFILL_GRADIENT;
            break;
//			case mso_fillBackground	:		// Use the background fill color/pattern
            default: break;
        }
        rSet.Put( XFillStyleItem( eXFill ) );

        if (IsProperty(DFF_Prop_fillOpacity))
        {
            double nTrans = GetPropertyValue(DFF_Prop_fillOpacity);
            nTrans = (nTrans * 100) / 65536;
            rSet.Put(XFillTransparenceItem(
                sal_uInt16(100 - ::rtl::math::round(nTrans))));
        }

        if ( ( eMSO_FillType == mso_fillShadeCenter ) && ( eXFill == XFILL_BITMAP ) )
        {
            ApplyRectangularGradientAsBitmap( rManager, rIn, rSet, aShadeColors, rObjData, mnFix16Angle );
        }
        else if ( eXFill == XFILL_GRADIENT )
        {
            sal_Int32 nAngle = 3600 - ( ( Fix16ToAngle( GetPropertyValue( DFF_Prop_fillAngle, 0 ) ) + 5 ) / 10 );

            // Rotationswinkel in Bereich zwingen
            while ( nAngle >= 3600 )
                nAngle -= 3600;
            while ( nAngle < 0 )
                nAngle += 3600;

            sal_Int32 nFocus = GetPropertyValue( DFF_Prop_fillFocus, 0 );
            XGradientStyle eGrad = XGRAD_LINEAR;
            sal_Int32 nChgColors = 0;

            if ( nFocus < 0 )		// Bei negativem Focus sind die Farben zu tauschen
            {
                nFocus =- nFocus;
                nChgColors ^= 1;
            }
            if( nFocus > 40 && nFocus < 60 )
            {
                eGrad = XGRAD_AXIAL;	// Besser gehts leider nicht
            }

            USHORT nFocusX = (USHORT)nFocus;
            USHORT nFocusY = (USHORT)nFocus;

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
                    nFocusX = ( IsProperty( DFF_Prop_fillToRight ) ) ? 100 : 0;
                    nFocusY = ( IsProperty( DFF_Prop_fillToBottom ) ) ? 100 : 0;
                    nChgColors ^= 1;
                }
                break;
                default: break;
            }
            Color aCol1( rManager.MSO_CLR_ToColor( GetPropertyValue( DFF_Prop_fillColor, COL_WHITE ), DFF_Prop_fillColor ) );
            Color aCol2( rManager.MSO_CLR_ToColor( GetPropertyValue( DFF_Prop_fillBackColor, COL_WHITE ), DFF_Prop_fillBackColor ) );

            if ( nChgColors )
            {
                Color aZwi( aCol1 );
                aCol1 = aCol2;
                aCol2 = aZwi;
            }
            XGradient aGrad( aCol2, aCol1, eGrad, nAngle, nFocusX, nFocusY );
            aGrad.SetStartIntens( 100 );
            aGrad.SetEndIntens( 100 );
            rSet.Put( XFillGradientItem( String(), aGrad ) );
        }
        else if ( eXFill == XFILL_BITMAP )
        {
            if( IsProperty( DFF_Prop_fillBlip ) )
            {
                Graphic aGraf;
                // first try to get BLIP from cache
                BOOL bOK = rManager.GetBLIP( GetPropertyValue( DFF_Prop_fillBlip ), aGraf, NULL );
                // then try directly from stream (i.e. Excel chart hatches/bitmaps)
                if ( !bOK )
                    bOK = SeekToContent( DFF_Prop_fillBlip, rIn ) && rManager.GetBLIPDirect( rIn, aGraf, NULL );
                if ( bOK )
                {
                    Bitmap aBmp( aGraf.GetBitmap() );

                    if ( eMSO_FillType == mso_fillPattern )
                    {
                        Color aCol1( COL_WHITE ), aCol2( COL_WHITE );
                        if ( IsProperty( DFF_Prop_fillColor ) )
                            aCol1 = rManager.MSO_CLR_ToColor( GetPropertyValue( DFF_Prop_fillColor ), DFF_Prop_fillColor );
                        if ( IsProperty( DFF_Prop_fillBackColor ) )
                            aCol2 = rManager.MSO_CLR_ToColor( GetPropertyValue( DFF_Prop_fillBackColor ), DFF_Prop_fillBackColor );

                        XOBitmap aXOBitmap;

                        // Bitmap einsetzen
                        aXOBitmap.SetBitmap( aBmp );
                        aXOBitmap.SetBitmapType( XBITMAP_IMPORT );

                        if( aBmp.GetSizePixel().Width() == 8 && aBmp.GetSizePixel().Height() == 8 && aBmp.GetColorCount() == 2)
                        {
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
                        }
                        rSet.Put( XFillBitmapItem( String(), aXOBitmap ) );
                    }
                    else if ( eMSO_FillType == mso_fillTexture )
                    {
                        XOBitmap aXBmp( aBmp, XBITMAP_STRETCH );
                        rSet.Put( XFillBmpTileItem( sal_True ) );
                        rSet.Put( XFillBitmapItem( String(), aXBmp ) );
                        rSet.Put( XFillBmpSizeXItem( GetPropertyValue( DFF_Prop_fillWidth, 0 ) / 360 ) );
                        rSet.Put( XFillBmpSizeYItem( GetPropertyValue( DFF_Prop_fillHeight, 0 ) / 360 ) );
                        rSet.Put( XFillBmpSizeLogItem( sal_True ) );
                    }
                    else
                    {
                        XOBitmap aXBmp( aBmp, XBITMAP_STRETCH );
                        rSet.Put( XFillBitmapItem( String(), aXBmp ) );
                        rSet.Put( XFillBmpTileItem( sal_False ) );
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
//    sal_uInt32 nTextFlags = aTextObj.GetTextFlags();
    sal_Bool  bVerticalText = sal_False;
    sal_Int32 nTextLeft = GetPropertyValue( DFF_Prop_dxTextLeft, 25 * 3600 ) / 360;		// 0.25 cm (emu)
    sal_Int32 nTextRight = GetPropertyValue( DFF_Prop_dxTextRight, 25 * 3600 ) / 360;	// 0.25 cm (emu)
    sal_Int32 nTextTop = GetPropertyValue( DFF_Prop_dyTextTop, 13 * 3600 ) / 360;		// 0.13 cm (emu)
    sal_Int32 nTextBottom = GetPropertyValue( DFF_Prop_dyTextBottom, 13 * 3600 ) /360;	// 0.13 cm (emu)

    SdrTextVertAdjust eTVA;
    SdrTextHorzAdjust eTHA;

    if ( IsProperty( DFF_Prop_txflTextFlow ) )
    {
        MSO_TextFlow eTextFlow = (MSO_TextFlow)( GetPropertyValue( DFF_Prop_txflTextFlow ) & 0xFFFF );
        switch( eTextFlow )
        {
            case mso_txflTtoBA :	/* #68110# */	// Top to Bottom @-font, oben -> unten
            case mso_txflTtoBN :					// Top to Bottom non-@, oben -> unten
            case mso_txflVertN :					// Vertical, non-@, oben -> unten
                bVerticalText = sal_True;			// nTextRotationAngle += 27000;
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

        // Textverankerung lesen
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

        // Textverankerung lesen
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

//	rSet.Put( SdrTextAutoGrowWidthItem( (MSO_WrapMode)GetPropertyValue( DFF_Prop_WrapText, mso_wrapSquare ) != mso_wrapNone ? sal_False : sal_True ) );
//	rSet.Put( SdrTextAutoGrowHeightItem( ( GetPropertyValue( DFF_Prop_FitTextToShape ) & 2 ) != 0 ) );
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
    const rtl::OUString	sType( RTL_CONSTASCII_USTRINGPARAM ( "Type" ) );
    aProp.Name  = sType;
    aProp.Value <<= EnhancedCustomShapeTypeNames::Get( rObjData.eShapeType );
    aPropVec.push_back( aProp );

/*
    /////////////////
    // "MirroredX" //
    /////////////////
    if ( nShapeFlags & SP_FFLIPH )
    {
        const rtl::OUString	sMirroredX( RTL_CONSTASCII_USTRINGPARAM ( "MirroredX" ) );
        sal_Bool bMirroredX = sal_True;
        aProp.Name = sMirroredX;
        aProp.Value <<= bMirroredX;
        aPropVec.push_back( aProp );
    }
    /////////////////
    // "MirroredY" //
    /////////////////
    if ( nShapeFlags & SP_FFLIPV )
    {
        const rtl::OUString	sMirroredY( RTL_CONSTASCII_USTRINGPARAM ( "MirroredY" ) );
        sal_Bool bMirroredY = sal_True;
        aProp.Name = sMirroredY;
        aProp.Value <<= bMirroredY;
        aPropVec.push_back( aProp );
    }
*/
    ///////////////
    // "ViewBox" //
    ///////////////

    sal_Int32 nCoordWidth = 21600;	// needed to replace handle type center with absolute value
    sal_Int32 nCoordHeight= 21600;
    if ( IsProperty( DFF_Prop_geoLeft ) || IsProperty( DFF_Prop_geoTop ) || IsProperty( DFF_Prop_geoRight ) || IsProperty( DFF_Prop_geoBottom ) )
    {
        com::sun::star::awt::Rectangle aViewBox;
        const rtl::OUString	sViewBox( RTL_CONSTASCII_USTRINGPARAM ( "ViewBox" ) );
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
/*		sal_Int32	 nFontDirection = GetPropertyValue( DFF_Prop_cdirFont, mso_cdir0 ); */

        if ( eTextFlow == mso_txflBtoT )	// Bottom to Top non-@, unten -> oben
            nTextRotateAngle += 90;
        switch( GetPropertyValue( DFF_Prop_cdirFont, mso_cdir0 ) )	// SJ: mso_cdir90 and mso_cdir270 will be simulated by
        {															// activating vertical writing for the text objects
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
            const rtl::OUString	sTextRotateAngle( RTL_CONSTASCII_USTRINGPARAM ( "TextRotateAngle" ) );
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
        const rtl::OUString	sExtrusionOn( RTL_CONSTASCII_USTRINGPARAM ( "Extrusion" ) );
        aProp.Name = sExtrusionOn;
        aProp.Value <<= bExtrusionOn;
        aExtrusionPropVec.push_back( aProp );

        // "Brightness"
        if ( IsProperty( DFF_Prop_c3DAmbientIntensity ) )
        {
            const rtl::OUString	sExtrusionBrightness( RTL_CONSTASCII_USTRINGPARAM ( "Brightness" ) );
            double fBrightness = (sal_Int32)GetPropertyValue( DFF_Prop_c3DAmbientIntensity );
            fBrightness /= 655.36;
            aProp.Name = sExtrusionBrightness;
            aProp.Value <<= fBrightness;
            aExtrusionPropVec.push_back( aProp );
        }
        // "Depth" in 1/100mm
        if ( IsProperty( DFF_Prop_c3DExtrudeBackward ) || IsProperty( DFF_Prop_c3DExtrudeForward ) )
        {
            const rtl::OUString	sDepth( RTL_CONSTASCII_USTRINGPARAM ( "Depth" ) );
            double fBackDepth = (double)((sal_Int32)GetPropertyValue( DFF_Prop_c3DExtrudeBackward, 1270 * 360 )) / 360.0;
            double fForeDepth = (double)((sal_Int32)GetPropertyValue( DFF_Prop_c3DExtrudeForward ), 0 ) / 360.0;
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
            const rtl::OUString	sExtrusionDiffusion( RTL_CONSTASCII_USTRINGPARAM ( "Diffusion" ) );
            double fDiffusion = (sal_Int32)GetPropertyValue( DFF_Prop_c3DDiffuseAmt );
            fDiffusion /= 655.36;
            aProp.Name = sExtrusionDiffusion;
            aProp.Value <<= fDiffusion;
            aExtrusionPropVec.push_back( aProp );
        }
        // "NumberOfLineSegments"
        if ( IsProperty( DFF_Prop_c3DTolerance ) )
        {
            const rtl::OUString	sExtrusionNumberOfLineSegments( RTL_CONSTASCII_USTRINGPARAM ( "NumberOfLineSegments" ) );
            aProp.Name = sExtrusionNumberOfLineSegments;
            aProp.Value <<= (sal_Int32)GetPropertyValue( DFF_Prop_c3DTolerance );
            aExtrusionPropVec.push_back( aProp );
        }
        // "LightFace"
        const rtl::OUString	sExtrusionLightFace( RTL_CONSTASCII_USTRINGPARAM ( "LightFace" ) );
        sal_Bool bExtrusionLightFace = ( GetPropertyValue( DFF_Prop_fc3DLightFace ) & 1 ) != 0;
        aProp.Name = sExtrusionLightFace;
        aProp.Value <<= bExtrusionLightFace;
        aExtrusionPropVec.push_back( aProp );
        // "FirstLightHarsh"
        const rtl::OUString	sExtrusionFirstLightHarsh( RTL_CONSTASCII_USTRINGPARAM ( "FirstLightHarsh" ) );
        sal_Bool bExtrusionFirstLightHarsh = ( GetPropertyValue( DFF_Prop_fc3DFillHarsh ) & 2 ) != 0;
        aProp.Name = sExtrusionFirstLightHarsh;
        aProp.Value <<= bExtrusionFirstLightHarsh;
        aExtrusionPropVec.push_back( aProp );
        // "SecondLightHarsh"
        const rtl::OUString	sExtrusionSecondLightHarsh( RTL_CONSTASCII_USTRINGPARAM ( "SecondLightHarsh" ) );
        sal_Bool bExtrusionSecondLightHarsh = ( GetPropertyValue( DFF_Prop_fc3DFillHarsh ) & 1 ) != 0;
        aProp.Name = sExtrusionSecondLightHarsh;
        aProp.Value <<= bExtrusionSecondLightHarsh;
        aExtrusionPropVec.push_back( aProp );
        // "FirstLightLevel"
        if ( IsProperty( DFF_Prop_c3DKeyIntensity ) )
        {
            const rtl::OUString	sExtrusionFirstLightLevel( RTL_CONSTASCII_USTRINGPARAM ( "FirstLightLevel" ) );
            double fFirstLightLevel = (sal_Int32)GetPropertyValue( DFF_Prop_c3DKeyIntensity );
            fFirstLightLevel /= 655.36;
            aProp.Name = sExtrusionFirstLightLevel;
            aProp.Value <<= fFirstLightLevel;
            aExtrusionPropVec.push_back( aProp );
        }
        // "SecondLightLevel"
        if ( IsProperty( DFF_Prop_c3DFillIntensity ) )
        {
            const rtl::OUString	sExtrusionSecondLightLevel( RTL_CONSTASCII_USTRINGPARAM ( "SecondLightLevel" ) );
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
            const rtl::OUString	sExtrusionFirstLightDirection( RTL_CONSTASCII_USTRINGPARAM ( "FirstLightDirection" ) );
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
            const rtl::OUString	sExtrusionSecondLightDirection( RTL_CONSTASCII_USTRINGPARAM ( "SecondLightDirection" ) );
            aProp.Name = sExtrusionSecondLightDirection;
            aProp.Value <<= aExtrusionSecondLightDirection;
            aExtrusionPropVec.push_back( aProp );
        }

/* LockRotationCenter, OrientationAngle and Orientation needs to be converted to use the properties AngleX, AngleY and RotationAngle instead.
        // "LockRotationCenter"
        const rtl::OUString	sExtrusionLockRotationCenter( RTL_CONSTASCII_USTRINGPARAM ( "LockRotationCenter" ) );
        sal_Bool bExtrusionLockRotationCenter = ( GetPropertyValue( DFF_Prop_fc3DFillHarsh ) & 16 ) != 0;
        aProp.Name = sExtrusionLockRotationCenter;
        aProp.Value <<= bExtrusionLockRotationCenter;
        aExtrusionPropVec.push_back( aProp );

        // "Orientation"
        if ( IsProperty( DFF_Prop_c3DRotationAxisX ) || IsProperty( DFF_Prop_c3DRotationAxisY ) || IsProperty( DFF_Prop_c3DRotationAxisZ ) )
        {
            double fRotX = (double)((sal_Int32)GetPropertyValue( DFF_Prop_c3DRotationAxisX, 100 ));
            double fRotY = (double)((sal_Int32)GetPropertyValue( DFF_Prop_c3DRotationAxisY, 0 ));
            double fRotZ = (double)((sal_Int32)GetPropertyValue( DFF_Prop_c3DRotationAxisZ, 0 ));
            ::com::sun::star::drawing::Direction3D aExtrusionDirection( fRotX, fRotY, fRotZ );
            const rtl::OUString	sExtrusionDirection( RTL_CONSTASCII_USTRINGPARAM ( "Orientation" ) );
            aProp.Name = sExtrusionDirection;
            aProp.Value <<= aExtrusionDirection;
            aExtrusionPropVec.push_back( aProp );
        }
        // "OrientationAngle" in Grad
        if ( IsProperty( DFF_Prop_c3DRotationAngle ) )
        {
            const rtl::OUString	sExtrusionOrientationAngle( RTL_CONSTASCII_USTRINGPARAM ( "OrientationAngle" ) );
            double fOrientationAngle = (double)((sal_Int32)GetPropertyValue( DFF_Prop_c3DRotationAngle )) / 65536.0;
            aProp.Name = sExtrusionOrientationAngle;
            aProp.Value <<= fOrientationAngle;
            aExtrusionPropVec.push_back( aProp );
        }
*/

        // "Metal"
        const rtl::OUString	sExtrusionMetal( RTL_CONSTASCII_USTRINGPARAM ( "Metal" ) );
        sal_Bool bExtrusionMetal = ( GetPropertyValue( DFF_Prop_fc3DLightFace ) & 4 ) != 0;
        aProp.Name = sExtrusionMetal;
        aProp.Value <<= bExtrusionMetal;
        aExtrusionPropVec.push_back( aProp );
//		if ( IsProperty( DFF_Prop_c3DExtrudePlane ) )
//		{
//		UPS
//		}
        // "ShadeMode"
        if ( IsProperty( DFF_Prop_c3DRenderMode ) )
        {
            const rtl::OUString	sExtrusionShadeMode( RTL_CONSTASCII_USTRINGPARAM ( "ShadeMode" ) );
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
            const rtl::OUString	sExtrusionAngle( RTL_CONSTASCII_USTRINGPARAM ( "RotateAngle" ) );
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

                const rtl::OUString	sExtrusionRotationCenter( RTL_CONSTASCII_USTRINGPARAM ( "RotationCenter" ) );
                aProp.Name = sExtrusionRotationCenter;
                aProp.Value <<= aRotationCenter;
                aExtrusionPropVec.push_back( aProp );
            }
        }
        // "Shininess"
        if ( IsProperty( DFF_Prop_c3DShininess ) )
        {
            const rtl::OUString	sExtrusionShininess( RTL_CONSTASCII_USTRINGPARAM ( "Shininess" ) );
            double fShininess = (sal_Int32)GetPropertyValue( DFF_Prop_c3DShininess );
            fShininess /= 655.36;
            aProp.Name = sExtrusionShininess;
            aProp.Value <<= fShininess;
            aExtrusionPropVec.push_back( aProp );
        }
        // "Skew"
        if ( IsProperty( DFF_Prop_c3DSkewAmount ) || IsProperty( DFF_Prop_c3DSkewAngle ) )
        {
            const rtl::OUString	sExtrusionSkew( RTL_CONSTASCII_USTRINGPARAM ( "Skew" ) );
            double fSkewAmount = (sal_Int32)GetPropertyValue( DFF_Prop_c3DSkewAmount, 50 );
            double fSkewAngle = (double)((sal_Int32)GetPropertyValue( DFF_Prop_c3DSkewAngle, sal::static_int_cast< UINT32 >(-135 * 65536) )) / 65536.0;

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
            const rtl::OUString	sExtrusionSpecularity( RTL_CONSTASCII_USTRINGPARAM ( "Specularity" ) );
            double fSpecularity = (sal_Int32)GetPropertyValue( DFF_Prop_c3DSpecularAmt );
            fSpecularity /= 1333;
            aProp.Name = sExtrusionSpecularity;
            aProp.Value <<= fSpecularity;
            aExtrusionPropVec.push_back( aProp );
        }
        // "ProjectionMode"
        const rtl::OUString	sExtrusionProjectionMode( RTL_CONSTASCII_USTRINGPARAM ( "ProjectionMode" ) );
        ProjectionMode eProjectionMode = GetPropertyValue( DFF_Prop_fc3DFillHarsh ) & 4 ? ProjectionMode_PARALLEL : ProjectionMode_PERSPECTIVE;
        aProp.Name = sExtrusionProjectionMode;
        aProp.Value <<= eProjectionMode;
        aExtrusionPropVec.push_back( aProp );

        // "ViewPoint" in 1/100mm
        if ( IsProperty( DFF_Prop_c3DXViewpoint ) || IsProperty( DFF_Prop_c3DYViewpoint ) || IsProperty( DFF_Prop_c3DZViewpoint ) )
        {
            double fViewX = (double)((sal_Int32)GetPropertyValue( DFF_Prop_c3DXViewpoint, 1249920 )) / 360.0;
            double fViewY = (double)((sal_Int32)GetPropertyValue( DFF_Prop_c3DYViewpoint, (sal_uInt32)-1249920 ))/ 360.0;
            double fViewZ = (double)((sal_Int32)GetPropertyValue( DFF_Prop_c3DZViewpoint, 9000000 )) / 360.0;
            ::com::sun::star::drawing::Position3D aExtrusionViewPoint( fViewX, fViewY, fViewZ );
            const rtl::OUString	sExtrusionViewPoint( RTL_CONSTASCII_USTRINGPARAM ( "ViewPoint" ) );
            aProp.Name = sExtrusionViewPoint;
            aProp.Value <<= aExtrusionViewPoint;
            aExtrusionPropVec.push_back( aProp );
        }
        // "Origin"
        if ( IsProperty( DFF_Prop_c3DOriginX ) || IsProperty( DFF_Prop_c3DOriginY ) )
        {
            const rtl::OUString	sExtrusionOrigin( RTL_CONSTASCII_USTRINGPARAM ( "Origin" ) );
            double fOriginX = (double)((sal_Int32)GetPropertyValue( DFF_Prop_c3DOriginX, 0 ));
            double fOriginY = (double)((sal_Int32)GetPropertyValue( DFF_Prop_c3DOriginY, 0 ));
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
        const rtl::OUString	sExtrusionColor( RTL_CONSTASCII_USTRINGPARAM ( "Color" ) );
        sal_Bool bExtrusionColor = IsProperty( DFF_Prop_c3DExtrusionColor );	// ( GetPropertyValue( DFF_Prop_fc3DLightFace ) & 2 ) != 0;
        aProp.Name = sExtrusionColor;
        aProp.Value <<= bExtrusionColor;
        aExtrusionPropVec.push_back( aProp );
        if ( IsProperty( DFF_Prop_c3DExtrusionColor ) )
            rSet.Put( XSecondaryFillColorItem( String(), rManager.MSO_CLR_ToColor(
                GetPropertyValue( DFF_Prop_c3DExtrusionColor ), DFF_Prop_c3DExtrusionColor ) ) );
        // pushing the whole Extrusion element
        const rtl::OUString	sExtrusion( RTL_CONSTASCII_USTRINGPARAM ( "Extrusion" ) );
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
        sal_uInt16 i;
        sal_uInt16 nNumElem = 0;
        sal_uInt16 nNumElemMem = 0;
        sal_uInt16 nElemSize = 8;

        if ( SeekToContent( DFF_Prop_pFormulas, rIn ) )
            rIn >> nNumElem >> nNumElemMem >> nElemSize;

        sal_Int16 nP1, nP2, nP3;
        sal_uInt16 nFlags;

        uno::Sequence< rtl::OUString > aEquations( nNumElem );
        for ( i = 0; i < nNumElem; i++ )
        {
            rIn >> nFlags >> nP1 >> nP2 >> nP3;
            aEquations[ i ] = EnhancedCustomShape2d::GetEquation( nFlags, nP1, nP2, nP3 );
        }
        // pushing the whole Equations element
        const rtl::OUString	sEquations( RTL_CONSTASCII_USTRINGPARAM ( "Equations" ) );
        aProp.Name = sEquations;
        aProp.Value <<= aEquations;
        aPropVec.push_back( aProp );
    }

    ////////////////////////////////////////
    // "Handles" PropertySequence element //
    ////////////////////////////////////////
    if ( IsProperty( DFF_Prop_Handles ) )
    {
        sal_uInt16 i;
        sal_uInt16 nNumElem = 0;
        sal_uInt16 nNumElemMem = 0;
        sal_uInt16 nElemSize = 36;

        if ( SeekToContent( DFF_Prop_Handles, rIn ) )
            rIn >> nNumElem >> nNumElemMem >> nElemSize;
        if ( nElemSize == 36 )
        {
            uno::Sequence< beans::PropertyValues > aHandles( nNumElem );
            for ( i = 0; i < nNumElem; i++ )
            {
                PropVec aHandlePropVec;
                sal_uInt32	nFlags;
                sal_Int32	nPositionX, nPositionY, nCenterX, nCenterY, nRangeXMin, nRangeXMax, nRangeYMin, nRangeYMax;
                rIn >> nFlags
                    >> nPositionX
                    >> nPositionY
                    >> nCenterX
                    >> nCenterY
                    >> nRangeXMin
                    >> nRangeXMax
                    >> nRangeYMin
                    >> nRangeYMax;

                if ( nPositionX == 2 )	// replacing center position with absolute value
                    nPositionX = nCoordWidth / 2;
                if ( nPositionY == 2 )
                    nPositionY = nCoordHeight / 2;
                EnhancedCustomShapeParameterPair aPosition;
                EnhancedCustomShape2d::SetEnhancedCustomShapeHandleParameter( aPosition.First,  nPositionX, sal_True, sal_True  );
                EnhancedCustomShape2d::SetEnhancedCustomShapeHandleParameter( aPosition.Second, nPositionY, sal_True, sal_False );
                const rtl::OUString	sHandlePosition( RTL_CONSTASCII_USTRINGPARAM ( "Position" ) );
                aProp.Name = sHandlePosition;
                aProp.Value <<= aPosition;
                aHandlePropVec.push_back( aProp );

                if ( nFlags & MSDFF_HANDLE_FLAGS_MIRRORED_X )
                {
                    sal_Bool bMirroredX = sal_True;
                    const rtl::OUString	sHandleMirroredX( RTL_CONSTASCII_USTRINGPARAM ( "MirroredX" ) );
                    aProp.Name = sHandleMirroredX;
                    aProp.Value <<= bMirroredX;
                    aHandlePropVec.push_back( aProp );
                }
                if ( nFlags & MSDFF_HANDLE_FLAGS_MIRRORED_Y )
                {
                    sal_Bool bMirroredY = sal_True;
                    const rtl::OUString	sHandleMirroredY( RTL_CONSTASCII_USTRINGPARAM ( "MirroredY" ) );
                    aProp.Name = sHandleMirroredY;
                    aProp.Value <<= bMirroredY;
                    aHandlePropVec.push_back( aProp );
                }
                if ( nFlags & MSDFF_HANDLE_FLAGS_SWITCHED )
                {
                    sal_Bool bSwitched = sal_True;
                    const rtl::OUString	sHandleSwitched( RTL_CONSTASCII_USTRINGPARAM ( "Switched" ) );
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
                    if ( ( nPositionY >= 0x256 ) || ( nPositionY <= 0x107 ) )	// position y
                        nAdjustmentsWhichNeedsToBeConverted |= ( 1 << i );
                    EnhancedCustomShapeParameterPair aPolar;
                    EnhancedCustomShape2d::SetEnhancedCustomShapeHandleParameter( aPolar.First,  nCenterX, ( nFlags & 0x800  ) != 0, sal_True  );
                    EnhancedCustomShape2d::SetEnhancedCustomShapeHandleParameter( aPolar.Second, nCenterY, ( nFlags & 0x1000 ) != 0, sal_False );
                    const rtl::OUString	sHandlePolar( RTL_CONSTASCII_USTRINGPARAM ( "Polar" ) );
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
                    EnhancedCustomShape2d::SetEnhancedCustomShapeHandleParameter( aMap.First,  nCenterX, ( nFlags & 0x800  ) != 0, sal_True  );
                    EnhancedCustomShape2d::SetEnhancedCustomShapeHandleParameter( aMap.Second, nCenterY, ( nFlags & 0x1000 ) != 0, sal_False );
                    const rtl::OUString	sHandleMap( RTL_CONSTASCII_USTRINGPARAM ( "Map" ) );
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
                            ( nFlags & MSDFF_HANDLE_FLAGS_RANGE_X_MIN_IS_SPECIAL ) != 0, sal_True  );
                        const rtl::OUString	sHandleRangeXMinimum( RTL_CONSTASCII_USTRINGPARAM ( "RangeXMinimum" ) );
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
                            ( nFlags & MSDFF_HANDLE_FLAGS_RANGE_X_MAX_IS_SPECIAL ) != 0, sal_False );
                        const rtl::OUString	sHandleRangeXMaximum( RTL_CONSTASCII_USTRINGPARAM ( "RangeXMaximum" ) );
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
                            ( nFlags & MSDFF_HANDLE_FLAGS_RANGE_Y_MIN_IS_SPECIAL ) != 0, sal_True );
                        const rtl::OUString	sHandleRangeYMinimum( RTL_CONSTASCII_USTRINGPARAM ( "RangeYMinimum" ) );
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
                            ( nFlags & MSDFF_HANDLE_FLAGS_RANGE_Y_MAX_IS_SPECIAL ) != 0, sal_False );
                        const rtl::OUString	sHandleRangeYMaximum( RTL_CONSTASCII_USTRINGPARAM ( "RangeYMaximum" ) );
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
                            ( nFlags & MSDFF_HANDLE_FLAGS_RANGE_X_MIN_IS_SPECIAL ) != 0, sal_True  );
                        const rtl::OUString	sHandleRadiusRangeMinimum( RTL_CONSTASCII_USTRINGPARAM ( "RadiusRangeMinimum" ) );
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
                            ( nFlags & MSDFF_HANDLE_FLAGS_RANGE_X_MAX_IS_SPECIAL ) != 0, sal_False );
                        const rtl::OUString	sHandleRadiusRangeMaximum( RTL_CONSTASCII_USTRINGPARAM ( "RadiusRangeMaximum" ) );
                        aProp.Name = sHandleRadiusRangeMaximum;
                        aProp.Value <<= aRadiusRangeMaximum;
                        aHandlePropVec.push_back( aProp );
                    }
                }
                if ( aHandlePropVec.size() )
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
            const rtl::OUString	sHandles( RTL_CONSTASCII_USTRINGPARAM ( "Handles" ) );
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
            const rtl::OUString	sExtrusionAllowed( RTL_CONSTASCII_USTRINGPARAM ( "ExtrusionAllowed" ) );
            sal_Bool bExtrusionAllowed = ( GetPropertyValue( DFF_Prop_fFillOK ) & 16 ) != 0;
            aProp.Name = sExtrusionAllowed;
            aProp.Value <<= bExtrusionAllowed;
            aPathPropVec.push_back( aProp );
        }
        // "Path/ConcentricGradientFillAllowed"
        if ( IsHardAttribute( DFF_Prop_fFillShadeShapeOK ) )
        {
            const rtl::OUString	sConcentricGradientFillAllowed( RTL_CONSTASCII_USTRINGPARAM ( "ConcentricGradientFillAllowed" ) );
            sal_Bool bConcentricGradientFillAllowed = ( GetPropertyValue( DFF_Prop_fFillOK ) & 2 ) != 0;
            aProp.Name = sConcentricGradientFillAllowed;
            aProp.Value <<= bConcentricGradientFillAllowed;
            aPathPropVec.push_back( aProp );
        }
        // "Path/TextPathAllowed"
        if ( IsHardAttribute( DFF_Prop_fGtextOK ) || ( GetPropertyValue( DFF_Prop_gtextFStrikethrough, 0 ) & 0x4000 ) )
        {
            const rtl::OUString	sTextPathAllowed( RTL_CONSTASCII_USTRINGPARAM ( "TextPathAllowed" ) );
            sal_Bool bTextPathAllowed = ( GetPropertyValue( DFF_Prop_fFillOK ) & 4 ) != 0;
            aProp.Name = sTextPathAllowed;
            aProp.Value <<= bTextPathAllowed;
            aPathPropVec.push_back( aProp );
        }
        // Path/Coordinates
        if ( IsProperty( DFF_Prop_pVertices ) )
        {
            com::sun::star::uno::Sequence< com::sun::star::drawing::EnhancedCustomShapeParameterPair > aCoordinates;

            sal_uInt16 i;
            sal_uInt16 nNumElemVert = 0;
            sal_uInt16 nNumElemMemVert = 0;
            sal_uInt16 nElemSizeVert = 8;

            if ( SeekToContent( DFF_Prop_pVertices, rIn ) )
                rIn >> nNumElemVert >> nNumElemMemVert >> nElemSizeVert;
            if ( nNumElemVert )
            {
                sal_Int32 nX, nY;
                sal_Int16 nTmpA, nTmpB;
                aCoordinates.realloc( nNumElemVert );
                for ( i = 0; i < nNumElemVert; i++ )
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
            const rtl::OUString	sCoordinates( RTL_CONSTASCII_USTRINGPARAM ( "Coordinates" ) );
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
            sal_uInt16 nNumElemMemSeg = 0;
            sal_uInt16 nElemSizeSeg = 2;

            if ( SeekToContent( DFF_Prop_pSegmentInfo, rIn ) )
                rIn >> nNumElemSeg >> nNumElemMemSeg >> nElemSizeSeg;
            if ( nNumElemSeg )
            {
                sal_Int16 nCommand;
                sal_Int16 nCnt;
                aSegments.realloc( nNumElemSeg );
                for ( i = 0; i < nNumElemSeg; i++ )
                {
                    rIn >> nTmp;
                    nCommand = EnhancedCustomShapeSegmentCommand::UNKNOWN;
                    nCnt = (sal_Int16)( nTmp & 0xfff );
                    switch( nTmp >> 12 )
                    {
                        case 0x0: nCommand = EnhancedCustomShapeSegmentCommand::LINETO; if ( !nCnt ) nCnt = 1; break;
                        case 0x1: nCommand = EnhancedCustomShapeSegmentCommand::LINETO; if ( !nCnt ) nCnt = 1; break;	// seems to the relative lineto
                        case 0x4: nCommand = EnhancedCustomShapeSegmentCommand::MOVETO; if ( !nCnt ) nCnt = 1; break;
                        case 0x2: nCommand = EnhancedCustomShapeSegmentCommand::CURVETO; if ( !nCnt ) nCnt = 1; break;
                        case 0x3: nCommand = EnhancedCustomShapeSegmentCommand::CURVETO; if ( !nCnt ) nCnt = 1; break;	// seems to be the relative curveto
                        case 0x8: nCommand = EnhancedCustomShapeSegmentCommand::ENDSUBPATH; nCnt = 0; break;
                        case 0x6: nCommand = EnhancedCustomShapeSegmentCommand::CLOSESUBPATH; nCnt = 0; break;
                        case 0xa:
                        case 0xb:
                        {
                            switch ( ( nTmp >> 8 ) & 0xf )
                            {
                                case 0x0:
                                {
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
            const rtl::OUString	sSegments( RTL_CONSTASCII_USTRINGPARAM ( "Segments" ) );
            aProp.Name = sSegments;
            aProp.Value <<= aSegments;
            aPathPropVec.push_back( aProp );
        }
        // Path/StretchX
        if ( IsProperty( DFF_Prop_stretchPointX ) )
        {
            const rtl::OUString	sStretchX( RTL_CONSTASCII_USTRINGPARAM ( "StretchX" ) );
            sal_Int32 nStretchX = GetPropertyValue( DFF_Prop_stretchPointX, 0 );
            aProp.Name = sStretchX;
            aProp.Value <<= nStretchX;
            aPathPropVec.push_back( aProp );
        }
        // Path/StretchX
        if ( IsProperty( DFF_Prop_stretchPointY ) )
        {
            const rtl::OUString	sStretchY( RTL_CONSTASCII_USTRINGPARAM ( "StretchY" ) );
            sal_Int32 nStretchY = GetPropertyValue( DFF_Prop_stretchPointY, 0 );
            aProp.Name = sStretchY;
            aProp.Value <<= nStretchY;
            aPathPropVec.push_back( aProp );
        }
        // Path/TextFrames
        if ( IsProperty( DFF_Prop_textRectangles ) )
        {
            sal_uInt16 i;
            sal_uInt16 nNumElem = 0;
            sal_uInt16 nNumElemMem = 0;
            sal_uInt16 nElemSize = 16;

            if ( SeekToContent( DFF_Prop_textRectangles, rIn ) )
                rIn >> nNumElem >> nNumElemMem >> nElemSize;
            if ( nElemSize == 16 )
            {
                sal_Int32 nLeft, nTop, nRight, nBottom;
                com::sun::star::uno::Sequence< com::sun::star::drawing::EnhancedCustomShapeTextFrame > aTextFrames( nNumElem );
                for ( i = 0; i < nNumElem; i++ )
                {
                    rIn >> nLeft
                        >> nTop
                        >> nRight
                        >> nBottom;

                    EnhancedCustomShape2d::SetEnhancedCustomShapeParameter( aTextFrames[ i ].TopLeft.First,	 nLeft );
                    EnhancedCustomShape2d::SetEnhancedCustomShapeParameter( aTextFrames[ i ].TopLeft.Second, nTop  );
                    EnhancedCustomShape2d::SetEnhancedCustomShapeParameter( aTextFrames[ i ].BottomRight.First,	 nRight );
                    EnhancedCustomShape2d::SetEnhancedCustomShapeParameter( aTextFrames[ i ].BottomRight.Second, nBottom);
                }
                const rtl::OUString	sTextFrames( RTL_CONSTASCII_USTRINGPARAM ( "TextFrames" ) );
                aProp.Name = sTextFrames;
                aProp.Value <<= aTextFrames;
                aPathPropVec.push_back( aProp );
            }
        }
        //Path/GluePoints
        if ( IsProperty( DFF_Prop_connectorPoints ) )
        {
            com::sun::star::uno::Sequence< com::sun::star::drawing::EnhancedCustomShapeParameterPair > aGluePoints;

            sal_uInt16 i;
            sal_uInt16 nNumElemVert = 0;
            sal_uInt16 nNumElemMemVert = 0;
            sal_uInt16 nElemSizeVert = 8;

            if ( SeekToContent( DFF_Prop_connectorPoints, rIn ) )
                rIn >> nNumElemVert >> nNumElemMemVert >> nElemSizeVert;

            sal_Int32 nX, nY;
            sal_Int16 nTmpA, nTmpB;
            aGluePoints.realloc( nNumElemVert );
            for ( i = 0; i < nNumElemVert; i++ )
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
            const rtl::OUString	sGluePoints( RTL_CONSTASCII_USTRINGPARAM ( "GluePoints" ) );
            aProp.Name = sGluePoints;
            aProp.Value <<= aGluePoints;
            aPathPropVec.push_back( aProp );
        }
        if ( IsProperty( DFF_Prop_connectorType ) )
        {
            sal_Int16 nGluePointType = (sal_uInt16)GetPropertyValue( DFF_Prop_connectorType );
            const rtl::OUString	sGluePointType( RTL_CONSTASCII_USTRINGPARAM ( "GluePointType" ) );
            aProp.Name = sGluePointType;
            aProp.Value <<= nGluePointType;
            aPathPropVec.push_back( aProp );
        }
        // pushing the whole Path element
        if ( aPathPropVec.size() )
        {
            const rtl::OUString	sPath( RTL_CONSTASCII_USTRINGPARAM ( "Path" ) );
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
        const rtl::OUString	sTextPathOn( RTL_CONSTASCII_USTRINGPARAM ( "TextPath" ) );
        aProp.Name = sTextPathOn;
        aProp.Value <<= bTextPathOn;
        aTextPathPropVec.push_back( aProp );

        // TextPathMode
        const rtl::OUString	sTextPathMode( RTL_CONSTASCII_USTRINGPARAM ( "TextPathMode" ) );
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
        const rtl::OUString	sTextPathScaleX( RTL_CONSTASCII_USTRINGPARAM ( "ScaleX" ) );
        sal_Bool bTextPathScaleX = ( GetPropertyValue( DFF_Prop_gtextFStrikethrough ) & 0x40 ) != 0;
        aProp.Name = sTextPathScaleX;
        aProp.Value <<= bTextPathScaleX;
        aTextPathPropVec.push_back( aProp );
        // SameLetterHeights
        const rtl::OUString sSameLetterHeight( RTL_CONSTASCII_USTRINGPARAM ( "SameLetterHeights" ) );
        sal_Bool bSameLetterHeight = ( GetPropertyValue( DFF_Prop_gtextFStrikethrough ) & 0x80 ) != 0;
        aProp.Name = sSameLetterHeight;
        aProp.Value <<= bSameLetterHeight;
        aTextPathPropVec.push_back( aProp );

        // pushing the whole TextPath element
        const rtl::OUString	sTextPath( RTL_CONSTASCII_USTRINGPARAM ( "TextPath" ) );
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
        const rtl::OUString	sAdjustmentValues( RTL_CONSTASCII_USTRINGPARAM ( "AdjustmentValues" ) );
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

void DffPropertyReader::ApplyAttributes( SvStream& rIn, SfxItemSet& rSet, const DffObjData& rObjData ) const
{
//	MapUnit eMap( rManager.GetModel()->GetScaleUnit() );

    sal_Bool bHasShadow = sal_False;

    for ( void* pDummy = ((DffPropertyReader*)this)->First(); pDummy; pDummy = ((DffPropertyReader*)this)->Next() )
    {
        UINT32 nRecType = GetCurKey();
        UINT32 nContent = mpContents[ nRecType ];
        switch ( nRecType )
        {
            case DFF_Prop_gtextSize :
                rSet.Put( SvxFontHeightItem( rManager.ScalePt( nContent ), 100, EE_CHAR_FONTHEIGHT ) );
            break;
            // GeoText
            case DFF_Prop_gtextFStrikethrough :
            {
                if ( nContent & 0x20 )
                    rSet.Put( SvxWeightItem( nContent ? WEIGHT_BOLD : WEIGHT_NORMAL, EE_CHAR_WEIGHT ) );
                if ( nContent & 0x10 )
                    rSet.Put( SvxPostureItem( nContent ? ITALIC_NORMAL : ITALIC_NONE, EE_CHAR_ITALIC ) );
                if ( nContent & 0x08 )
                    rSet.Put( SvxUnderlineItem( nContent ? UNDERLINE_SINGLE : UNDERLINE_NONE, EE_CHAR_UNDERLINE ) );
                if ( nContent & 0x40 )
                    rSet.Put(SvxShadowedItem( nContent != 0, EE_CHAR_SHADOW ) );
//				if ( nContent & 0x02 )
//					rSet.Put( SvxCaseMapItem( nContent ? SVX_CASEMAP_KAPITAELCHEN : SVX_CASEMAP_NOT_MAPPED ) );
                if ( nContent & 0x01 )
                    rSet.Put( SvxCrossedOutItem( nContent ? STRIKEOUT_SINGLE : STRIKEOUT_NONE, EE_CHAR_STRIKEOUT ) );
            }
            break;

            case DFF_Prop_fillColor :
                rSet.Put( XFillColorItem( String(), rManager.MSO_CLR_ToColor( nContent, DFF_Prop_fillColor ) ) );
            break;

            // ShadowStyle
            case DFF_Prop_shadowType :
            {
                MSO_ShadowType eShadowType = (MSO_ShadowType)nContent;
                if( eShadowType != mso_shadowOffset )
                {
                    //   mso_shadowDouble
                    //   mso_shadowRich
                    //   mso_shadowEmbossOrEngrave
                    // koennen wir nicht, kreiere Default-Schatten mit default-
                    // Abstand
                    rSet.Put( SdrShadowXDistItem( 35 ) ); // 0,35 mm Schattendistanz
                    rSet.Put( SdrShadowYDistItem( 35 ) );
                }
            }
            break;
            case DFF_Prop_shadowColor :
                rSet.Put( SdrShadowColorItem( String(), rManager.MSO_CLR_ToColor( nContent, DFF_Prop_shadowColor ) ) );
            break;
            case DFF_Prop_shadowOpacity :
                rSet.Put( SdrShadowTransparenceItem( (sal_uInt16)( ( 0x10000 - nContent ) / 655 ) ) );
            break;
            case DFF_Prop_shadowOffsetX :
            {
                INT32 nVal = (INT32)nContent;
                rManager.ScaleEmu( nVal );
                if ( nVal )
                    rSet.Put( SdrShadowXDistItem( nVal ) );
            }
            break;
            case DFF_Prop_shadowOffsetY :
            {
                INT32 nVal = (INT32)nContent;
                rManager.ScaleEmu( nVal );
                if ( nVal )
                    rSet.Put( SdrShadowYDistItem( nVal ) );
            }
            break;
            case DFF_Prop_fshadowObscured :
            {
                bHasShadow = ( nContent & 2 ) != 0;
                if ( bHasShadow )
                {
                    if ( !IsProperty( DFF_Prop_shadowOffsetX ) )
                        rSet.Put( SdrShadowXDistItem( 35 ) );
                    if ( !IsProperty( DFF_Prop_shadowOffsetY ) )
                        rSet.Put( SdrShadowYDistItem( 35 ) );
                }
            }
            break;
        }
    }

    if ( bHasShadow )
    {
        // #160376# sj: activating shadow only if fill and or linestyle is used
        // this is required because of the latest drawing layer core changes.
        // Issue i104085 is related to this.
        UINT32 nLineFlags(GetPropertyValue( DFF_Prop_fNoLineDrawDash ));
        if(!IsHardAttribute( DFF_Prop_fLine ) && !IsCustomShapeStrokedByDefault( rObjData.eShapeType ))
            nLineFlags &= ~0x08;
        UINT32 nFillFlags(GetPropertyValue( DFF_Prop_fNoFillHitTest ));
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
                // case mso_fillBackground :
                default:
                    nFillFlags &=~0x10;			// no fillstyle used	
                break;
            }
        }
        if ( ( ( nLineFlags & 0x08 ) == 0 ) && ( ( nFillFlags & 0x10 ) == 0 ) )	// if there is no fillstyle and linestyle
            bHasShadow = sal_False;												// we are turning shadow off.

        if ( bHasShadow )
            rSet.Put( SdrShadowItem( bHasShadow ) );
    }
    ApplyLineAttributes( rSet, rObjData.eShapeType ); // #i28269#
    ApplyFillAttributes( rIn, rSet, rObjData );
    if ( rObjData.eShapeType != mso_sptNil )
    {
        ApplyCustomShapeGeometryAttributes( rIn, rSet, rObjData );
        ApplyCustomShapeTextAttributes( rSet );
    }
}

//---------------------------------------------------------------------------
//- Record Manager ----------------------------------------------------------
//---------------------------------------------------------------------------

DffRecordList::DffRecordList( DffRecordList* pList ) :
    nCount					( 0 ),
    nCurrent				( 0 ),
    pPrev					( pList ),
    pNext					( NULL )
{
    if ( pList )
        pList->pNext = this;
}

DffRecordList::~DffRecordList()
{
    delete pNext;
}

DffRecordManager::DffRecordManager() :
    DffRecordList	( NULL ),
    pCList			( (DffRecordList*)this )
{
}

DffRecordManager::DffRecordManager( SvStream& rIn ) :
    DffRecordList	( NULL ),
    pCList			( (DffRecordList*)this )
{
    Consume( rIn );
}

DffRecordManager::~DffRecordManager()
{
};


void DffRecordManager::Consume( SvStream& rIn, BOOL bAppend, UINT32 nStOfs )
{
    if ( !bAppend )
        Clear();
    UINT32 nOldPos = rIn.Tell();
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
            pCList->mHd[ pCList->nCount++ ].SeekToEndOfRecord( rIn );
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
    UINT32 nC = pCList->nCurrent + 1;
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
    UINT32 nCur = pCList->nCurrent;
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
    UINT32 nCnt = pCList->nCount;
    if ( nCnt-- )
    {
        pCList->nCurrent = nCnt;
        pRet = &pCList->mHd[ nCnt ];
    }
    return pRet;
}

BOOL DffRecordManager::SeekToContent( SvStream& rIn, UINT16 nRecId, DffSeekToContentMode eMode )
{
    DffRecordHeader* pHd = GetRecordHeader( nRecId, eMode );
    if ( pHd )
    {
        pHd->SeekToContent( rIn );
        return TRUE;
    }
    else
        return FALSE;
}

DffRecordHeader* DffRecordManager::GetRecordHeader( UINT16 nRecId, DffSeekToContentMode eMode )
{
    UINT32 nOldCurrent = pCList->nCurrent;
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
//  private Methoden
//---------------------------------------------------------------------------

struct EscherBlipCacheEntry
{
    ByteString	aUniqueID;
    sal_uInt32  nBlip;

    EscherBlipCacheEntry( sal_uInt32 nBlipId, const ByteString& rUniqueID ) :
        aUniqueID( rUniqueID ),
        nBlip( nBlipId ) {}
};

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

void SvxMSDffManager::Scale( Rectangle& rRect ) const
{
    rRect.Move( nMapXOfs, nMapYOfs );
    if ( bNeedMap )
    {
        rRect.Left()  =BigMulDiv( rRect.Left()  , nMapMul, nMapDiv );
        rRect.Top()   =BigMulDiv( rRect.Top()   , nMapMul, nMapDiv );
        rRect.Right() =BigMulDiv( rRect.Right() , nMapMul, nMapDiv );
        rRect.Bottom()=BigMulDiv( rRect.Bottom(), nMapMul, nMapDiv );
    }
}

void SvxMSDffManager::Scale( Polygon& rPoly ) const
{
    if ( !bNeedMap )
        return;
    USHORT nPointAnz = rPoly.GetSize();
    for ( USHORT nPointNum = 0; nPointNum < nPointAnz; nPointNum++ )
        Scale( rPoly[ nPointNum ] );
}

void SvxMSDffManager::Scale( PolyPolygon& rPoly ) const
{
    if ( !bNeedMap )
        return;
    USHORT nPolyAnz = rPoly.Count();
    for ( USHORT nPolyNum = 0; nPolyNum < nPolyAnz; nPolyNum++ )
        Scale( rPoly[ nPolyNum ] );
}

void SvxMSDffManager::ScaleEmu( sal_Int32& rVal ) const
{
    rVal = BigMulDiv( rVal, nEmuMul, nEmuDiv );
}

UINT32 SvxMSDffManager::ScalePt( UINT32 nVal ) const
{
    MapUnit eMap = pSdrModel->GetScaleUnit();
    Fraction aFact( GetMapFactor( MAP_POINT, eMap ).X() );
    long aMul = aFact.GetNumerator();
    long aDiv = aFact.GetDenominator() * 65536;
    aFact = Fraction( aMul, aDiv ); // nochmal versuchen zu kuerzen
    return BigMulDiv( nVal, aFact.GetNumerator(), aFact.GetDenominator() );
}

INT32 SvxMSDffManager::ScalePoint( INT32 nVal ) const
{
    return BigMulDiv( nVal, nPntMul, nPntDiv );
};

void SvxMSDffManager::SetModel(SdrModel* pModel, long nApplicationScale)
{
    pSdrModel = pModel;
    if( pModel && (0 < nApplicationScale) )
    {
        // PPT arbeitet nur mit Einheiten zu 576DPI
        // WW hingegen verwendet twips, dh. 1440DPI.
        MapUnit eMap = pSdrModel->GetScaleUnit();
        Fraction aFact( GetMapFactor(MAP_INCH, eMap).X() );
        long nMul=aFact.GetNumerator();
        long nDiv=aFact.GetDenominator()*nApplicationScale;
        aFact=Fraction(nMul,nDiv); // nochmal versuchen zu kuerzen
        // Bei 100TH_MM -> 2540/576=635/144
        // Bei Twip     -> 1440/576=5/2
        nMapMul  = aFact.GetNumerator();
        nMapDiv  = aFact.GetDenominator();
        bNeedMap = nMapMul!=nMapDiv;

        // MS-DFF-Properties sind grossteils in EMU (English Metric Units) angegeben
        // 1mm=36000emu, 1twip=635emu
        aFact=GetMapFactor(MAP_100TH_MM,eMap).X();
        nMul=aFact.GetNumerator();
        nDiv=aFact.GetDenominator()*360;
        aFact=Fraction(nMul,nDiv); // nochmal versuchen zu kuerzen
        // Bei 100TH_MM ->                            1/360
        // Bei Twip     -> 14,40/(25,4*360)=144/91440=1/635
        nEmuMul=aFact.GetNumerator();
        nEmuDiv=aFact.GetDenominator();

        // Und noch was fuer typografische Points
        aFact=GetMapFactor(MAP_POINT,eMap).X();
        nPntMul=aFact.GetNumerator();
        nPntDiv=aFact.GetDenominator();
    }
    else
    {
        pModel = 0;
        nMapMul = nMapDiv = nMapXOfs = nMapYOfs = nEmuMul = nEmuDiv = nPntMul = nPntDiv = 0;
        bNeedMap = FALSE;
    }
}

BOOL SvxMSDffManager::SeekToShape( SvStream& rSt, void* /* pClientData */, UINT32 nId ) const
{
    BOOL bRet = FALSE;
    if ( mpFidcls )
    {
        UINT32 nMerk = rSt.Tell();
        UINT32 nShapeId, nSec = ( nId >> 10 ) - 1;
        if ( nSec < mnIdClusters )
        {
            sal_IntPtr nOfs = (sal_IntPtr)maDgOffsetTable.Get( mpFidcls[ nSec ].dgid );
            if ( nOfs )
            {
                rSt.Seek( nOfs );
                DffRecordHeader aEscherF002Hd;
                rSt >> aEscherF002Hd;
                ULONG nEscherF002End = aEscherF002Hd.GetRecEndFilePos();
                DffRecordHeader aEscherObjListHd;
                while ( rSt.Tell() < nEscherF002End )
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
                                bRet = TRUE;
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

FASTBOOL SvxMSDffManager::SeekToRec( SvStream& rSt, USHORT nRecId, ULONG nMaxFilePos, DffRecordHeader* pRecHd, ULONG nSkipCount ) const
{
    FASTBOOL bRet = FALSE;
    ULONG nFPosMerk = rSt.Tell(); // FilePos merken fuer ggf. spaetere Restauration
    DffRecordHeader aHd;
    do
    {
        rSt >> aHd;
        if ( aHd.nRecType == nRecId )
        {
            if ( nSkipCount )
                nSkipCount--;
            else
            {
                bRet = TRUE;
                if ( pRecHd != NULL )
                    *pRecHd = aHd;
                else
                    aHd.SeekToBegOfRecord( rSt );
            }
        }
        if ( !bRet )
            aHd.SeekToEndOfRecord( rSt );
    }
    while ( rSt.GetError() == 0 && rSt.Tell() < nMaxFilePos && !bRet );
    if ( !bRet )
        rSt.Seek( nFPosMerk );	// FilePos restaurieren
    return bRet;
}

FASTBOOL SvxMSDffManager::SeekToRec2( USHORT nRecId1, USHORT nRecId2, ULONG nMaxFilePos, DffRecordHeader* pRecHd, ULONG nSkipCount ) const
{
    FASTBOOL bRet = FALSE;
    ULONG nFPosMerk = rStCtrl.Tell();	// FilePos merken fuer ggf. spaetere Restauration
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
                bRet = TRUE;
                if ( pRecHd )
                    *pRecHd = aHd;
                else
                    aHd.SeekToBegOfRecord( rStCtrl );
            }
        }
        if ( !bRet )
            aHd.SeekToEndOfRecord( rStCtrl );
    }
    while ( rStCtrl.GetError() == 0 && rStCtrl.Tell() < nMaxFilePos && !bRet );
    if ( !bRet )
        rStCtrl.Seek( nFPosMerk ); // FilePos restaurieren
    return bRet;
}


FASTBOOL SvxMSDffManager::GetColorFromPalette( USHORT /* nNum */, Color& rColor ) const
{
    // diese Methode ist in der zum Excel-Import
    // abgeleiteten Klasse zu ueberschreiben...
    rColor.SetColor( COL_WHITE );
    return TRUE;
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
    // Fuer Textfarben: Header ist 0xfeRRGGBB
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

    // Fuer Textfarben: Header ist 0xfeRRGGBB
    if ( ( nColorCode & 0xfe000000 ) == 0xfe000000 )	// sj: it needs to be checked if 0xfe is used in 
        nColorCode &= 0x00ffffff;						// other cases than ppt text -> if not this code can be removed

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
        else	// SYSCOLOR
        {
            const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();

//			UINT16 nParameter = (BYTE)( nColorCode >> 16);					// SJ: nice compiler optimization bug on windows, though downcasting
            UINT16 nParameter = sal_uInt16(( nColorCode >> 16 ) & 0x00ff);	// the HiByte of nParameter is not zero, an exclusive AND is helping :o
            UINT16 nFunctionBits = (UINT16)( ( nColorCode & 0x00000f00 ) >> 8 );
            UINT16 nAdditionalFlags = (UINT16)( ( nColorCode & 0x0000f000) >> 8 );
            UINT16 nColorIndex = sal_uInt16(nColorCode & 0x00ff);
            UINT32 nPropColor = 0;

            sal_uInt16	nCProp = 0;

            switch ( nColorIndex )
            {
                case mso_syscolorButtonFace :			aColor = rStyleSettings.GetFaceColor(); break;
                case mso_syscolorWindowText :			aColor = rStyleSettings.GetWindowTextColor(); break;
                case mso_syscolorMenu :					aColor = rStyleSettings.GetMenuColor(); break;
                case mso_syscolor3DLight :
                case mso_syscolorButtonHighlight :
                case mso_syscolorHighlight :			aColor = rStyleSettings.GetHighlightColor(); break;
                case mso_syscolorHighlightText :		aColor = rStyleSettings.GetHighlightTextColor(); break;
                case mso_syscolorCaptionText :			aColor = rStyleSettings.GetMenuTextColor(); break;
                case mso_syscolorActiveCaption :		aColor = rStyleSettings.GetHighlightColor(); break;
                case mso_syscolorButtonShadow :			aColor = rStyleSettings.GetShadowColor(); break;
                case mso_syscolorButtonText :			aColor = rStyleSettings.GetButtonTextColor(); break;
                case mso_syscolorGrayText :				aColor = rStyleSettings.GetDeactiveColor(); break;
                case mso_syscolorInactiveCaption :		aColor = rStyleSettings.GetDeactiveColor(); break;
                case mso_syscolorInactiveCaptionText :	aColor = rStyleSettings.GetDeactiveColor(); break;
                case mso_syscolorInfoBackground :		aColor = rStyleSettings.GetFaceColor(); break;
                case mso_syscolorInfoText :				aColor = rStyleSettings.GetInfoTextColor(); break;
                case mso_syscolorMenuText :				aColor = rStyleSettings.GetMenuTextColor(); break;
                case mso_syscolorScrollbar :			aColor = rStyleSettings.GetFaceColor(); break;
                case mso_syscolorWindow :				aColor = rStyleSettings.GetWindowColor(); break;
                case mso_syscolorWindowFrame :			aColor = rStyleSettings.GetWindowColor(); break;

                case mso_colorFillColor :
                {
                    nPropColor = GetPropertyValue( DFF_Prop_fillColor, 0xffffff );
                    nCProp = DFF_Prop_fillColor;
                }
                break;
                case mso_colorLineOrFillColor :		// ( use the line color only if there is a line )
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
                case mso_colorThis :				// ( use this color ... )
                {
                    nPropColor = GetPropertyValue( DFF_Prop_fillColor, 0xffffff );	//?????????????
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
                case mso_colorFillThenLine :		// ( use the fillcolor unless no fill and line )
                {
                    nPropColor = GetPropertyValue( DFF_Prop_fillColor, 0xffffff );	//?????????????
                    nCProp = DFF_Prop_fillColor;
                }
                break;
                case mso_colorIndexMask :			// ( extract the color index ) ?
                {
                    nPropColor = GetPropertyValue( DFF_Prop_fillColor, 0xffffff );	//?????????????
                    nCProp = DFF_Prop_fillColor;
                }
                break;
            }
            if ( nCProp && ( nPropColor & 0x10000000 ) == 0 )		// beware of looping recursive
                aColor = MSO_CLR_ToColor( nPropColor, nCProp );

            if( nAdditionalFlags & 0x80 )			// make color gray
            {
                UINT8 nZwi = aColor.GetLuminance();
                aColor = Color( nZwi, nZwi, nZwi );
            }
            switch( nFunctionBits )
            {
                case 0x01 :		// darken color by parameter
                {
                    aColor.SetRed( sal::static_int_cast< UINT8 >( ( nParameter * aColor.GetRed() ) >> 8 ) );
                    aColor.SetGreen( sal::static_int_cast< UINT8 >( ( nParameter * aColor.GetGreen() ) >> 8 ) );
                    aColor.SetBlue( sal::static_int_cast< UINT8 >( ( nParameter * aColor.GetBlue() ) >> 8 ) );
                }
                break;
                case 0x02 :		// lighten color by parameter
                {
                    UINT16 nInvParameter = ( 0x00ff - nParameter ) * 0xff;
                    aColor.SetRed( sal::static_int_cast< UINT8 >( ( nInvParameter + ( nParameter * aColor.GetRed() ) ) >> 8 ) );
                    aColor.SetGreen( sal::static_int_cast< UINT8 >( ( nInvParameter + ( nParameter * aColor.GetGreen() ) ) >> 8 ) );
                    aColor.SetBlue( sal::static_int_cast< UINT8 >( ( nInvParameter + ( nParameter * aColor.GetBlue() ) ) >> 8 ) );
                }
                break;
                case 0x03 :		// add grey level RGB(p,p,p)
                {
                    INT16 nR = (INT16)aColor.GetRed() + (INT16)nParameter;
                    INT16 nG = (INT16)aColor.GetGreen() + (INT16)nParameter;
                    INT16 nB = (INT16)aColor.GetBlue() + (INT16)nParameter;
                    if ( nR > 0x00ff )
                        nR = 0x00ff;
                    if ( nG > 0x00ff )
                        nG = 0x00ff;
                    if ( nB > 0x00ff )
                        nB = 0x00ff;
                    aColor = Color( (UINT8)nR, (UINT8)nG, (UINT8)nB );
                }
                break;
                case 0x04 :		// substract grey level RGB(p,p,p)
                {
                    INT16 nR = (INT16)aColor.GetRed() - (INT16)nParameter;
                    INT16 nG = (INT16)aColor.GetGreen() - (INT16)nParameter;
                    INT16 nB = (INT16)aColor.GetBlue() - (INT16)nParameter;
                    if ( nR < 0 )
                        nR = 0;
                    if ( nG < 0 )
                        nG = 0;
                    if ( nB < 0 )
                        nB = 0;
                    aColor = Color( (UINT8)nR, (UINT8)nG, (UINT8)nB );
                }
                break;
                case 0x05 :		// substract from grey level RGB(p,p,p)
                {
                    INT16 nR = (INT16)nParameter - (INT16)aColor.GetRed();
                    INT16 nG = (INT16)nParameter - (INT16)aColor.GetGreen();
                    INT16 nB = (INT16)nParameter - (INT16)aColor.GetBlue();
                    if ( nR < 0 )
                        nR = 0;
                    if ( nG < 0 )
                        nG = 0;
                    if ( nB < 0 )
                        nB = 0;
                    aColor = Color( (UINT8)nR, (UINT8)nG, (UINT8)nB );
                }
                break;
                case 0x06 :		// per component: black if < p, white if >= p
                {
                    aColor.SetRed( aColor.GetRed() < nParameter ? 0x00 : 0xff );
                    aColor.SetGreen( aColor.GetGreen() < nParameter ? 0x00 : 0xff );
                    aColor.SetBlue( aColor.GetBlue() < nParameter ? 0x00 : 0xff );
                }
                break;
            }
            if ( nAdditionalFlags & 0x40 )					// top-bit invert
                aColor = Color( aColor.GetRed() ^ 0x80, aColor.GetGreen() ^ 0x80, aColor.GetBlue() ^ 0x80 );

            if ( nAdditionalFlags & 0x20 )					// invert color
                aColor = Color(0xff - aColor.GetRed(), 0xff - aColor.GetGreen(), 0xff - aColor.GetBlue());
        }
    }
    else if ( ( nUpper & 4 ) && ( ( nColorCode & 0xfffff8 ) == 0 ) )
    {	// case of nUpper == 4 powerpoint takes this as agrument for a colorschemecolor
        GetColorFromPalette( nUpper, aColor );
    }
    else	// hart attributiert, eventuell mit Hinweis auf SYSTEMRGB
        aColor = Color( (BYTE)nColorCode, (BYTE)( nColorCode >> 8 ), (BYTE)( nColorCode >> 16 ) );
    return aColor;
}

FASTBOOL SvxMSDffManager::ReadDffString(SvStream& rSt, String& rTxt) const
{
    FASTBOOL bRet=FALSE;
    DffRecordHeader aStrHd;
    if( !ReadCommonRecordHeader(aStrHd, rSt) )
        rSt.Seek( aStrHd.nFilePos );
    else if ( aStrHd.nRecType == DFF_PST_TextBytesAtom || aStrHd.nRecType == DFF_PST_TextCharsAtom )
    {
        FASTBOOL bUniCode=aStrHd.nRecType==DFF_PST_TextCharsAtom;
        bRet=TRUE;
        ULONG nBytes = aStrHd.nRecLen;
        MSDFFReadZString( rSt, rTxt, nBytes, bUniCode );
        if( !bUniCode )
        {
            for ( xub_StrLen n = 0; n < nBytes; n++ )
            {
                if( rTxt.GetChar( n ) == 0x0B )
                    rTxt.SetChar( n, ' ' );		// Weicher Umbruch
                // TODO: Zeilenumbruch im Absatz via Outliner setzen.
            }
        }
        aStrHd.SeekToEndOfRecord( rSt );
    }
    else
        aStrHd.SeekToBegOfRecord( rSt );
    return bRet;
}

// sj: I just want to set a string for a text object that may contain multiple
// paragraphs. If I now take a look at the follwing code I get the impression that
// our outliner is too complicate to be used properly,
void SvxMSDffManager::ReadObjText( const String& rText, SdrObject* pObj ) const
{
    SdrTextObj* pText = PTR_CAST( SdrTextObj, pObj );
    if ( pText )
    {
        SdrOutliner& rOutliner = pText->ImpGetDrawOutliner();
        rOutliner.Init( OUTLINERMODE_TEXTOBJECT );

        BOOL bOldUpdateMode = rOutliner.GetUpdateMode();
        rOutliner.SetUpdateMode( FALSE );
        rOutliner.SetVertical( pText->IsVerticalWriting() );

        sal_uInt16 nParaIndex = 0;
        sal_uInt32 nParaSize;
        const sal_Unicode* pCurrent, *pBuf = rText.GetBuffer();
        const sal_Unicode* pEnd = rText.GetBuffer() + rText.Len();

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
                    nParaSize++;
            }
            ESelection aSelection( nParaIndex, 0, nParaIndex, 0 );
            String aParagraph( pCurrent, (sal_uInt16)nParaSize );
            if ( !nParaIndex && !aParagraph.Len() )					// SJ: we are crashing if the first paragraph is empty ?
                aParagraph += (sal_Unicode)' ';						// otherwise these two lines can be removed.
            rOutliner.Insert( aParagraph, nParaIndex, 0 );
            rOutliner.SetParaAttribs( nParaIndex, rOutliner.GetEmptyItemSet() );

            SfxItemSet aParagraphAttribs( rOutliner.GetEmptyItemSet() );
            if ( !aSelection.nStartPos )
                aParagraphAttribs.Put( SfxBoolItem( EE_PARA_BULLETSTATE, FALSE ) );
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

FASTBOOL SvxMSDffManager::ReadObjText(SvStream& rSt, SdrObject* pObj) const
{
    FASTBOOL bRet=FALSE;
    SdrTextObj* pText = PTR_CAST(SdrTextObj, pObj);
    if( pText )
    {
        DffRecordHeader aTextHd;
        if( !ReadCommonRecordHeader(aTextHd, rSt) )
            rSt.Seek( aTextHd.nFilePos );
        else if ( aTextHd.nRecType==DFF_msofbtClientTextbox )
        {
            bRet=TRUE;
            ULONG nRecEnd=aTextHd.GetRecEndFilePos();
            DffRecordHeader aHd;
            String aText;
//			UINT32 nInvent=pText->GetObjInventor();
//			UINT16 nIdent=pText->GetObjIdentifier();

            SdrOutliner& rOutliner=pText->ImpGetDrawOutliner();
//			sal_Int16 nMinDepth = rOutliner.GetMinDepth();
            USHORT nOutlMode = rOutliner.GetMode();

            { // Wohl 'nen kleiner Bug der EditEngine, das die
              // Absastzattribute bei Clear() nicht entfernt werden.
                FASTBOOL bClearParaAttribs = TRUE;
                rOutliner.SetStyleSheet( 0, NULL );
                SfxItemSet aSet(rOutliner.GetEmptyItemSet());
                aSet.Put(SvxColorItem( COL_BLACK ));
                rOutliner.SetParaAttribs(0,aSet);
                pText->SetMergedItemSet(aSet);

                bClearParaAttribs = FALSE;
                if( bClearParaAttribs )
                {
                    // Wohl 'nen kleiner Bug der EditEngine, dass die
                    // Absastzattribute bei Clear() nicht entfernt werden.
                    rOutliner.SetParaAttribs(0,rOutliner.GetEmptyItemSet());
                }
            }
            rOutliner.Init( OUTLINERMODE_TEXTOBJECT );

//			ULONG nFilePosMerker=rSt.Tell();
            ////////////////////////////////////
            // TextString und MetaChars lesen //
            ////////////////////////////////////
            do
            {
                if( !ReadCommonRecordHeader(aHd, rSt) )
                    rSt.Seek( aHd.nFilePos );
                else
                {
                    switch (aHd.nRecType)
                    {
                        //case TextHeaderAtom
                        //case TextSpecInfoAtom
                        case DFF_PST_TextBytesAtom:
                        case DFF_PST_TextCharsAtom:
                        {
                            aHd.SeekToBegOfRecord(rSt);
                            ReadDffString(rSt, aText);
                        }
                        break;
                        case DFF_PST_TextRulerAtom               :
                        {
                            UINT16 nLen = (UINT16)aHd.nRecLen;
                            if(nLen)
                            {
                                UINT16 nVal1, nVal2, nVal3;
                                UINT16 nDefaultTab = 2540; // PPT def: 1 Inch //rOutliner.GetDefTab();
                                UINT16 nMostrightTab = 0;
                                SfxItemSet aSet(rOutliner.GetEmptyItemSet());
                                SvxTabStopItem aTabItem(0, 0, SVX_TAB_ADJUST_DEFAULT, EE_PARA_TABS);

                                rSt >> nVal1;
                                rSt >> nVal2;
                                nLen -= 4;

                                // Allg. TAB verstellt auf Wert in nVal3
                                if(nLen && (nVal1 & 0x0001))
                                {
                                    rSt >> nVal3;
                                    nLen -= 2;
                                    nDefaultTab = (UINT16)(((UINT32)nVal3 * 1000) / 240);
                                }

                                // Weitere, frei gesetzte TABs
                                if(nLen && (nVal1 & 0x0004))
                                {
                                    rSt >> nVal1;
                                    nLen -= 2;

                                    // fest gesetzte TABs importieren
                                    while(nLen && nVal1--)
                                    {
                                        rSt >> nVal2;
                                        rSt >> nVal3;
                                        nLen -= 4;

                                        UINT16 nNewTabPos = (UINT16)(((UINT32)nVal2 * 1000) / 240);
                                        if(nNewTabPos > nMostrightTab)
                                            nMostrightTab = nNewTabPos;

                                        SvxTabStop aTabStop(nNewTabPos);
                                        aTabItem.Insert(aTabStop);
                                    }
                                }

                                // evtl. noch default-TABs ergaenzen (immer)
                                UINT16 nObjWidth = sal_uInt16(pObj->GetSnapRect().GetWidth() + 1);
                                UINT16 nDefaultTabPos = nDefaultTab;

                                while(nDefaultTabPos <= nObjWidth && nDefaultTabPos <= nMostrightTab)
                                    nDefaultTabPos =
                                        nDefaultTabPos + nDefaultTab;

                                while(nDefaultTabPos <= nObjWidth)
                                {
                                    SvxTabStop aTabStop(nDefaultTabPos);
                                    aTabItem.Insert(aTabStop);
                                    nDefaultTabPos =
                                        nDefaultTabPos + nDefaultTab;
                                }

                                // Falls TABs angelegt wurden, setze diese
                                if(aTabItem.Count())
                                {
                                    aSet.Put(aTabItem);
                                    rOutliner.SetParaAttribs(0, aSet);
                                }
                            }
                        }
                        break;
                    }
                    aHd.SeekToEndOfRecord( rSt );
                }
            }
            while ( rSt.GetError() == 0 && rSt.Tell() < nRecEnd );

            ////////////////////////
            // SHIFT-Ret ersetzen //
            ////////////////////////
            if ( aText.Len() )
            {
                aText += ' ';
                aText.SetChar( aText.Len()-1, 0x0D );
                rOutliner.SetText( aText, rOutliner.GetParagraph( 0 ) );

                // SHIFT-Ret ersetzen im Outliner
                if(aText.GetTokenCount(0x0B) > 1)
                {
                    UINT32 nParaCount = rOutliner.GetParagraphCount();
                    for(UINT16 a=0;a<nParaCount;a++)
                    {
                        Paragraph* pActPara = rOutliner.GetParagraph(a);
                        String aParaText = rOutliner.GetText(pActPara);
                        for(UINT16 b=0;b<aParaText.Len();b++)
                        {
                            if( aParaText.GetChar( b ) == 0x0B)
                            {
                                ESelection aSelection(a, b, a, b+1);
                                rOutliner.QuickInsertLineBreak(aSelection);
                            }
                        }
                    }
                }
            }
            OutlinerParaObject* pNewText=rOutliner.CreateParaObject();
            rOutliner.Init( nOutlMode );
            pText->NbcSetOutlinerParaObject(pNewText);
        }
        else
            aTextHd.SeekToBegOfRecord(rSt);

    }
    return bRet;
}

//static
void SvxMSDffManager::MSDFFReadZString( SvStream& rIn, String& rStr,
                                    ULONG nRecLen, FASTBOOL bUniCode )
{
    sal_uInt16 nLen = (sal_uInt16)nRecLen;
    if( nLen )
    {
        if ( bUniCode )
            nLen >>= 1;

        String sBuf;
        sal_Unicode* pBuf = sBuf.AllocBuffer( nLen );

        if( bUniCode )
        {
            rIn.Read( (sal_Char*)pBuf, nLen << 1 );

#ifdef OSL_BIGENDIAN
            for( sal_uInt16 n = 0; n < nLen; ++n, ++pBuf )
                *pBuf = SWAPSHORT( *pBuf );
#endif // ifdef OSL_BIGENDIAN
        }
        else
        {
            // use the String-Data as buffer for the 8bit characters and
            // change then all to unicode
            sal_Char* pReadPos = ((sal_Char*)pBuf) + nLen;
            rIn.Read( (sal_Char*)pReadPos, nLen );
            for( sal_uInt16 n = 0; n < nLen; ++n, ++pBuf, ++pReadPos )
                *pBuf = ByteString::ConvertToUnicode( *pReadPos, RTL_TEXTENCODING_MS_1252 );
        }

        rStr = sBuf.EraseTrailingChars( 0 );
    }
    else
        rStr.Erase();
}

SdrObject* SvxMSDffManager::ImportFontWork( SvStream& rStCt, SfxItemSet& rSet, Rectangle& rBoundRect ) const
{
    SdrObject*	pRet = NULL;
    String		aObjectText;
    String		aFontName;
    BOOL		bTextRotate = FALSE;

    ((SvxMSDffManager*)this)->mnFix16Angle = 0;	// we don't want to use this property in future
    if ( SeekToContent( DFF_Prop_gtextUNICODE, rStCt ) )
        MSDFFReadZString( rStCt, aObjectText, GetPropertyValue( DFF_Prop_gtextUNICODE ), TRUE );
    if ( SeekToContent( DFF_Prop_gtextFont, rStCt ) )
        MSDFFReadZString( rStCt, aFontName, GetPropertyValue( DFF_Prop_gtextFont ), TRUE );
    if ( GetPropertyValue( DFF_Prop_gtextFStrikethrough, 0 ) & 0x2000 )
    {
        // Text ist senkrecht formatiert, Box Kippen
        INT32 nHalfWidth = ( rBoundRect.GetWidth() + 1) >> 1;
        INT32 nHalfHeight = ( rBoundRect.GetHeight() + 1) >> 1;
        Point aTopLeft( rBoundRect.Left() + nHalfWidth - nHalfHeight,
                rBoundRect.Top() + nHalfHeight - nHalfWidth);
        Size aNewSize( rBoundRect.GetHeight(), rBoundRect.GetWidth() );
        Rectangle aNewRect( aTopLeft, aNewSize );
        rBoundRect = aNewRect;

        String aSrcText( aObjectText );
        aObjectText.Erase();
        for( UINT16 a = 0; a < aSrcText.Len(); a++ )
        {
            aObjectText += aSrcText.GetChar( a );
            aObjectText += '\n';
        }
        rSet.Put( SdrTextHorzAdjustItem( SDRTEXTHORZADJUST_CENTER ) );
        bTextRotate = TRUE;
    }
    if ( aObjectText.Len() )
    {	// FontWork-Objekt Mit dem Text in aObjectText erzeugen
        SdrObject* pNewObj = new SdrRectObj( OBJ_TEXT, rBoundRect );
        if( pNewObj )
        {
            pNewObj->SetModel( pSdrModel );
            ((SdrRectObj*)pNewObj)->SetText( aObjectText );
            SdrFitToSizeType eFTS = SDRTEXTFIT_PROPORTIONAL;
            rSet.Put( SdrTextFitToSizeTypeItem( eFTS ) );
            rSet.Put( SdrTextAutoGrowHeightItem( FALSE ) );
            rSet.Put( SdrTextAutoGrowWidthItem( FALSE ) );
            rSet.Put( SvxFontItem( FAMILY_DONTKNOW, aFontName, String(),
                            PITCH_DONTKNOW, RTL_TEXTENCODING_DONTKNOW, EE_CHAR_FONTINFO ));

            pNewObj->SetMergedItemSet(rSet);

            pRet = pNewObj->ConvertToPolyObj( FALSE, FALSE );
            if( !pRet )
                pRet = pNewObj;
            else
            {
                pRet->NbcSetSnapRect( rBoundRect );
                SdrObject::Free( pNewObj );
            }
            if( bTextRotate )
            {
                double a = 9000 * nPi180;
                pRet->NbcRotate( rBoundRect.Center(), 9000, sin( a ), cos( a ) );
            }
        }
    }
    return pRet;
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
    sal_Int32 nCropTop		= (sal_Int32)rPropSet.GetPropertyValue( DFF_Prop_cropFromTop, 0 );
    sal_Int32 nCropBottom	= (sal_Int32)rPropSet.GetPropertyValue( DFF_Prop_cropFromBottom, 0 );
    sal_Int32 nCropLeft		= (sal_Int32)rPropSet.GetPropertyValue( DFF_Prop_cropFromLeft, 0 );
    sal_Int32 nCropRight	= (sal_Int32)rPropSet.GetPropertyValue( DFF_Prop_cropFromRight, 0 );

    if( nCropTop || nCropBottom || nCropLeft || nCropRight )
    {
        double      fFactor;
        Size        aCropSize;
        BitmapEx    aCropBitmap;
        sal_uInt32  nTop( 0 ),  nBottom( 0 ), nLeft( 0 ), nRight( 0 );

        if ( pSet )	// use crop attributes ?
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
        if ( pSet )	// use crop attributes ?
            pSet->Put( SdrGrafCropItem( nLeft, nTop, nRight, nBottom ) );
        else
        {
            Rectangle aCropRect( nLeft, nTop, aCropSize.Width() - nRight, aCropSize.Height() - nBottom );
            aCropBitmap.Crop( aCropRect );
            rGraf = aCropBitmap;
        }
    }
}

SdrObject* SvxMSDffManager::ImportGraphic( SvStream& rSt, SfxItemSet& rSet, const DffObjData& rObjData ) const
{
    SdrObject*  pRet = NULL;
    String      aFilename;
    String      aLinkFileName, aLinkFilterName;
    Rectangle	aVisArea;

    MSO_BlipFlags eFlags = (MSO_BlipFlags)GetPropertyValue( DFF_Prop_pibFlags, mso_blipflagDefault );
    sal_uInt32 nBlipId = GetPropertyValue( DFF_Prop_pib, 0 );
    sal_Bool bGrfRead = sal_False,

    // Grafik verlinkt
    bLinkGrf = 0 != ( eFlags & mso_blipflagLinkToFile );
    {
        Graphic aGraf;	// be sure this graphic is deleted before swapping out
        if( SeekToContent( DFF_Prop_pibName, rSt ) )
            MSDFFReadZString( rSt, aFilename, GetPropertyValue( DFF_Prop_pibName ), TRUE );

        //   UND, ODER folgendes:
        if( !( eFlags & mso_blipflagDoNotSave ) ) // Grafik embedded
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
                    const ULONG nSkipBLIPLen = 20;
                    const ULONG nSkipShapePos = 4;
                    const ULONG nSkipBLIP = 4;
                    const ULONG nSkip =
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
                UINT32 nTransColor = GetPropertyValue( DFF_Prop_pictureTransparent, 0 );

                if ( aGraf.GetType() == GRAPHIC_BITMAP )
                {
                    BitmapEx	aBitmapEx( aGraf.GetBitmapEx() );
                    Bitmap		aBitmap( aBitmapEx.GetBitmap() );
                    Bitmap		aMask( aBitmap.CreateMask( MSO_CLR_ToColor( nTransColor, DFF_Prop_pictureTransparent ), 9 ) );
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
                fX /= 51;	// 50 + 1 to round
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
                nContrast *= 101;	//100 + 1 to round
                nContrast /= 0x10000;
                nContrast -= 100;
            }
            sal_Int16	nBrightness		= (sal_Int16)( (sal_Int32)GetPropertyValue( DFF_Prop_pictureBrightness, 0 ) / 327 );
            sal_Int32	nGamma			= GetPropertyValue( DFF_Prop_pictureGamma, 0x10000 );
            GraphicDrawMode eDrawMode	= GRAPHICDRAWMODE_STANDARD;
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
                            BitmapEx	aBitmapEx( aGraf.GetBitmapEx() );
                            if ( nBrightness || nContrast || ( nGamma != 0x10000 ) )
                                aBitmapEx.Adjust( nBrightness, (sal_Int16)nContrast, 0, 0, 0, (double)nGamma / 0x10000, FALSE );
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
                                aGdiMetaFile.Adjust( nBrightness, (sal_Int16)nContrast, 0, 0, 0, (double)nGamma / 0x10000, FALSE );
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

        // sollte es ein OLE-Object sein?
        if( bGrfRead && !bLinkGrf && IsProperty( DFF_Prop_pictureId ) )
        {
            // TODO/LATER: in future probably the correct aspect should be provided here
            sal_Int64 nAspect = embed::Aspects::MSOLE_CONTENT;
            // --> OD 2004-12-14 #i32596# - pass <nCalledByGroup> to method
            pRet = ImportOLE( GetPropertyValue( DFF_Prop_pictureId ), aGraf, rObjData.aBoundRect, aVisArea, rObjData.nCalledByGroup, nAspect );
            // <--
        }
        if( !pRet )
        {
            pRet = new SdrGrafObj;
            if( bGrfRead )
                ((SdrGrafObj*)pRet)->SetGraphic( aGraf );

            if( bLinkGrf && !bGrfRead )		// sj: #i55484# if the graphic was embedded ( bGrfRead == true ) then
            {								// we do not need to set a link. TODO: not to lose the information where the graphic is linked from
                UniString aName( ::URIHelper::SmartRel2Abs( INetURLObject(maBaseURL), aFilename, URIHelper::GetMaybeFileHdl(), true, false,
                                                                INetURLObject::WAS_ENCODED,
                                                                    INetURLObject::DECODE_UNAMBIGUOUS ) );

                String	    	aFilterName;
                INetURLObject   aURLObj( aName );

                if( aURLObj.GetProtocol() == INET_PROT_NOT_VALID )
                {
                    String aValidURL;

                    if( ::utl::LocalFileHelper::ConvertPhysicalNameToURL( aName, aValidURL ) )
                        aURLObj = INetURLObject( aValidURL );
                }

                if( aURLObj.GetProtocol() != INET_PROT_NOT_VALID )
                {
                    GraphicFilter* pGrfFilter = GraphicFilter::GetGraphicFilter();
                    aFilterName = pGrfFilter->GetImportFormatName(
                                    pGrfFilter->GetImportFormatNumberForShortName( aURLObj.getExtension() ) );
                }

                aLinkFileName = aName;
                aLinkFilterName = aFilterName;
            }
        }

        // set the size from BLIP if there is one
        if ( pRet && bGrfRead && !aVisArea.IsEmpty() )
            pRet->SetBLIPSizeRectangle( aVisArea );

        if ( !pRet->GetName().Len() )					// SJ 22.02.00 : PPT OLE IMPORT:
        {												// name is already set in ImportOLE !!
            // JP 01.12.99: SetName before SetModel - because in the other order the Bug 70098 is active
            if ( ( eFlags & mso_blipflagType ) != mso_blipflagComment )
            {
                INetURLObject aURL;
                aURL.SetSmartURL( aFilename );
                pRet->SetName( aURL.getBase() );
            }
            else
                pRet->SetName( aFilename );
        }
    }
    pRet->SetModel( pSdrModel ); // fuer GraphicLink erforderlich
    pRet->SetLogicRect( rObjData.aBoundRect );

    if ( pRet->ISA( SdrGrafObj ) )
    {
        if( aLinkFileName.Len() )
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
    aObjHd.SeekToBegOfRecord( rSt );	// FilePos restaurieren
    return pRet;
}

SdrObject* SvxMSDffManager::ImportGroup( const DffRecordHeader& rHd, SvStream& rSt, void* pClientData,
                                            Rectangle& rClientRect, const Rectangle& rGlobalChildRect,
                                                int nCalledByGroup, sal_Int32* pShapeId )
{
    SdrObject* pRet = NULL;

    if( pShapeId )
        *pShapeId = 0;

    rHd.SeekToContent( rSt );
    DffRecordHeader aRecHd;		// the first atom has to be the SpContainer for the GroupObject
    rSt >> aRecHd;
    if ( aRecHd.nRecType == DFF_msofbtSpContainer )
    {
        INT32 nGroupRotateAngle = 0;
        INT32 nSpFlags = 0;
        mnFix16Angle = 0;
        aRecHd.SeekToBegOfRecord( rSt );
        pRet = ImportObj( rSt, pClientData, rClientRect, rGlobalChildRect, nCalledByGroup + 1, pShapeId );
        if ( pRet )
        {
            nSpFlags = nGroupShapeFlags;
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
            aRecHd.SeekToEndOfRecord( rSt );
            while ( ( rSt.GetError() == 0 ) && ( rSt.Tell() < rHd.GetRecEndFilePos() ) )
            {
                DffRecordHeader aRecHd2;
                rSt >> aRecHd2;
                if ( aRecHd2.nRecType == DFF_msofbtSpgrContainer )
                {
                    Rectangle aGroupClientAnchor, aGroupChildAnchor;
                    GetGroupAnchors( aRecHd2, rSt, aGroupClientAnchor, aGroupChildAnchor, aClientRect, aGlobalChildRect );
                    aRecHd2.SeekToBegOfRecord( rSt );
                    sal_Int32 nShapeId;
                    SdrObject* pTmp = ImportGroup( aRecHd2, rSt, pClientData, aGroupClientAnchor, aGroupChildAnchor, nCalledByGroup + 1, &nShapeId );
                    if ( pTmp )
                    {
                        ((SdrObjGroup*)pRet)->GetSubList()->NbcInsertObject( pTmp );
                        if( nShapeId )
                            insertShapeId( nShapeId, pTmp );
                    }
                }
                else if ( aRecHd2.nRecType == DFF_msofbtSpContainer )
                {
                    aRecHd2.SeekToBegOfRecord( rSt );
                    sal_Int32 nShapeId;
                    SdrObject* pTmp = ImportShape( aRecHd2, rSt, pClientData, aClientRect, aGlobalChildRect, nCalledByGroup + 1, &nShapeId );
                    if ( pTmp )
                    {
                        ((SdrObjGroup*)pRet)->GetSubList()->NbcInsertObject( pTmp );
                        if( nShapeId )
                            insertShapeId( nShapeId, pTmp );
                    }
                }
                aRecHd2.SeekToEndOfRecord( rSt );
            }

    //		pRet->NbcSetSnapRect( aGroupBound );
            if ( nGroupRotateAngle )
            {
                double a = nGroupRotateAngle * nPi180;
                pRet->NbcRotate( aClientRect.Center(), nGroupRotateAngle, sin( a ), cos( a ) );
            }
            if ( nSpFlags & SP_FFLIPV )		// Vertikal gespiegelt?
            {	// BoundRect in aBoundRect
                Point aLeft( aClientRect.Left(), ( aClientRect.Top() + aClientRect.Bottom() ) >> 1 );
                Point aRight( aLeft.X() + 1000, aLeft.Y() );
                pRet->NbcMirror( aLeft, aRight );
            }
            if ( nSpFlags & SP_FFLIPH )		// Horizontal gespiegelt?
            {	// BoundRect in aBoundRect
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

    rHd.SeekToBegOfRecord( rSt );
    DffObjData aObjData( rHd, rClientRect, nCalledByGroup );
    maShapeRecords.Consume( rSt, FALSE );
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

    if ( mbTracing )
        mpTracer->AddAttribute( aObjData.nSpFlags & SP_FGROUP
                                ? rtl::OUString::createFromAscii( "GroupShape" )
                                : rtl::OUString::createFromAscii( "Shape" ),
                                rtl::OUString::valueOf( (sal_Int32)aObjData.nShapeId ) );
    aObjData.bOpt = maShapeRecords.SeekToContent( rSt, DFF_msofbtOPT, SEEK_FROM_CURRENT_AND_RESTART );
    if ( aObjData.bOpt )
    {
        maShapeRecords.Current()->SeekToBegOfRecord( rSt );
#ifdef DBG_AUTOSHAPE
        ReadPropSet( rSt, pClientData, (UINT32)aObjData.eShapeType );
#else
        ReadPropSet( rSt, pClientData );
#endif
    }
    else
    {
        InitializePropSet();	// get the default PropSet
        ( (DffPropertyReader*) this )->mnFix16Angle = 0;
    }

    aObjData.bChildAnchor = maShapeRecords.SeekToContent( rSt, DFF_msofbtChildAnchor, SEEK_FROM_CURRENT_AND_RESTART );
    if ( aObjData.bChildAnchor )
    {
        INT32 l, o, r, u;
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
    {	// Rotation auf BoundingBox anwenden, BEVOR ien Objekt generiert wurde
        if( mnFix16Angle )
        {
            long nAngle = mnFix16Angle;
            if ( ( nAngle > 4500 && nAngle <= 13500 ) || ( nAngle > 22500 && nAngle <= 31500 ) )
            {
                INT32 nHalfWidth = ( aObjData.aBoundRect.GetWidth() + 1 ) >> 1;
                INT32 nHalfHeight = ( aObjData.aBoundRect.GetHeight() + 1 ) >> 1;
                Point aTopLeft( aObjData.aBoundRect.Left() + nHalfWidth - nHalfHeight,
                                aObjData.aBoundRect.Top() + nHalfHeight - nHalfWidth );
                Size aNewSize( aObjData.aBoundRect.GetHeight(), aObjData.aBoundRect.GetWidth() );
                Rectangle aNewRect( aTopLeft, aNewSize );
                aObjData.aBoundRect = aNewRect;
            }
        }
        aTextRect = aObjData.aBoundRect;
        FASTBOOL bGraphic = IsProperty( DFF_Prop_pib ) ||
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
            nGroupShapeFlags = aObjData.nSpFlags;		// #73013#
        }
        else if ( ( aObjData.eShapeType != mso_sptNil ) || IsProperty( DFF_Prop_pVertices ) || bGraphic )
        {
            SfxItemSet	aSet( pSdrModel->GetItemPool() );

            sal_Bool	bIsConnector = ( ( aObjData.eShapeType >= mso_sptStraightConnector1 ) && ( aObjData.eShapeType <= mso_sptCurvedConnector5 ) );
            sal_Bool	bIsCustomShape = sal_False;
            sal_Int32	nObjectRotation = mnFix16Angle;
            sal_uInt32	nSpFlags = aObjData.nSpFlags;

            if ( bGraphic )
            {
                pRet = ImportGraphic( rSt, aSet, aObjData );		// SJ: #68396# is no longer true (fixed in ppt2000)
                ApplyAttributes( rSt, aSet, aObjData );
                pRet->SetMergedItemSet(aSet);
            }
            else if ( aObjData.eShapeType == mso_sptLine )
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

// the com.sun.star.drawing.EnhancedCustomShapeEngine is default, so we do not need to set a hard attribute
//						aSet.Put( SdrCustomShapeEngineItem( String::CreateFromAscii( "com.sun.star.drawing.EnhancedCustomShapeEngine" ) ) );
                    pRet = new SdrObjCustomShape();
                    pRet->SetModel( pSdrModel );

                    sal_Bool bIsFontwork = ( GetPropertyValue( DFF_Prop_gtextFStrikethrough, 0 ) & 0x4000 ) != 0;

                    // in case of a FontWork, the text is set by the escher import
                    if ( bIsFontwork )
                    {
                        String				aObjectText;
                        String				aFontName;
                        MSO_GeoTextAlign	eGeoTextAlign;

                        if ( SeekToContent( DFF_Prop_gtextFont, rSt ) )
                        {
                            SvxFontItem aLatin(EE_CHAR_FONTINFO), aAsian(EE_CHAR_FONTINFO_CJK), aComplex(EE_CHAR_FONTINFO_CTL);
                            GetDefaultFonts( aLatin, aAsian, aComplex );

                            MSDFFReadZString( rSt, aFontName, GetPropertyValue( DFF_Prop_gtextFont ), TRUE );
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
                            MSDFFReadZString( rSt, aObjectText, GetPropertyValue( DFF_Prop_gtextUNICODE ), TRUE );
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
                        if ( GetPropertyValue( DFF_Prop_gtextFStrikethrough, 0 ) & 0x1000 )	// SJ: Font Kerning On ?
                            aSet.Put( SvxKerningItem( 1, EE_CHAR_KERNING ) );
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
                            BOOL bOldUpdateMode = rOutliner.GetUpdateMode();
                            SdrModel* pModel = pRet->GetModel();
                            if ( pModel )
                                rOutliner.SetStyleSheetPool( (SfxStyleSheetPool*)pModel->GetStyleSheetPool() );
                            rOutliner.SetUpdateMode( FALSE );
                            rOutliner.SetText( *pParaObj );
                            VirtualDevice aVirDev( 1 );
                            aVirDev.SetMapMode( MAP_100TH_MM );
                            sal_uInt32 i, nParagraphs = rOutliner.GetParagraphCount();
                            if ( nParagraphs )
                            {
                                sal_Bool bCreateNewParaObject = sal_False;
                                for ( i = 0; i < nParagraphs; i++ )
                                {
                                    BOOL bIsRTL = aVirDev.GetTextIsRTL( rOutliner.GetText( rOutliner.GetParagraph( i ) ), 0, STRING_LEN );
                                    if ( bIsRTL )
                                    {
                                        SfxItemSet aSet2( rOutliner.GetParaAttribs( (USHORT)i ) );
                                        aSet2.Put( SvxFrameDirectionItem( FRMDIR_HORI_RIGHT_TOP, EE_PARA_WRITINGDIR ) );
                                        rOutliner.SetParaAttribs( (USHORT)i, aSet2 );
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
                        const rtl::OUString	sAdjustmentValues( RTL_CONSTASCII_USTRINGPARAM ( "AdjustmentValues" ) );
                        const rtl::OUString	sCoordinates( RTL_CONSTASCII_USTRINGPARAM ( "Coordinates" ) );
                        const rtl::OUString	sHandles( RTL_CONSTASCII_USTRINGPARAM ( "Handles" ) );
                        const rtl::OUString	sEquations( RTL_CONSTASCII_USTRINGPARAM ( "Equations" ) );
                        const rtl::OUString	sViewBox( RTL_CONSTASCII_USTRINGPARAM ( "ViewBox" ) );
                        const rtl::OUString	sPath( RTL_CONSTASCII_USTRINGPARAM ( "Path" ) );
                        const rtl::OUString	sTextFrames( RTL_CONSTASCII_USTRINGPARAM ( "TextFrames" ) );
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
//								const EnhancedCustomShapeParameterPair* pTmp = seqCoordinates.getArray();
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
                            aPolyBoundRect = Rectangle( -21600, 0, 21600, 43200 );	// defaulting

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
                                seqAdjustmentValues[ 0 ].State = com::sun::star::beans::PropertyState_DIRECT_VALUE;		// so this value will properly be stored
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
                            aGeometryItem.SetPropertyValue( aPropVal );		// storing the angle attribute
                        }
                        if ( nStartAngle != nEndAngle )
                        {
                            XPolygon aXPoly( aPolyBoundRect.Center(), aPolyBoundRect.GetWidth() / 2, aPolyBoundRect.GetHeight() / 2,
                                (USHORT)nStartAngle / 10, (USHORT)nEndAngle / 10, TRUE );
                            Rectangle aPolyPieRect( aXPoly.GetBoundRect() );

                            double	fYScale, fXScale;
                            double	fYOfs, fXOfs;

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
                            EnhancedCustomShape2d::SetEnhancedCustomShapeParameter( aTextFrame[ 0 ].TopLeft.First,	   nLeft );
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
                        const rtl::OUString	sType( RTL_CONSTASCII_USTRINGPARAM ( "Type" ) );
                        const rtl::OUString	sName( RTL_CONSTASCII_USTRINGPARAM ( "mso-spt100" ) );
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

                    bIsCustomShape = TRUE;

                    if( bIsConnector )
                    {
                        if( nObjectRotation )
                        {
                            double a = nObjectRotation * nPi180;
                            pRet->NbcRotate( aObjData.aBoundRect.Center(), nObjectRotation, sin( a ), cos( a ) );
                        }
                        // Horizontal gespiegelt?
                        if ( nSpFlags & SP_FFLIPH )
                        {
                            Rectangle aBndRect( pRet->GetSnapRect() );
                            Point aTop( ( aBndRect.Left() + aBndRect.Right() ) >> 1, aBndRect.Top() );
                            Point aBottom( aTop.X(), aTop.Y() + 1000 );
                            pRet->NbcMirror( aTop, aBottom );
                        }
                        // Vertikal gespiegelt?
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

                        // Konnektoren
                        MSO_ConnectorStyle eConnectorStyle = (MSO_ConnectorStyle)GetPropertyValue( DFF_Prop_cxstyle, mso_cxstyleStraight );

                        ((SdrEdgeObj*)pRet)->ConnectToNode(TRUE, NULL);
                        ((SdrEdgeObj*)pRet)->ConnectToNode(FALSE, NULL);

                        Point aPoint1( aObjData.aBoundRect.TopLeft() );
                        Point aPoint2( aObjData.aBoundRect.BottomRight() );

                        // Rotationen beachten
                        if ( nObjectRotation )
                        {
                            double a = nObjectRotation * nPi180;
                            Point aCenter( aObjData.aBoundRect.Center() );
                            double ss = sin(a);
                            double cc = cos(a);

                            RotatePoint(aPoint1, aCenter, ss, cc);
                            RotatePoint(aPoint2, aCenter, ss, cc);
                        }

                        // Linie innerhalb des Bereiches zurechtdrehen/spiegeln
                        if ( nSpFlags & SP_FFLIPH )
                        {
                            INT32 n = aPoint1.X();
                            aPoint1.X() = aPoint2.X();
                            aPoint2.X() = n;
                        }
                        if ( nSpFlags & SP_FFLIPV )
                        {
                            INT32 n = aPoint1.Y();
                            aPoint1.Y() = aPoint2.Y();
                            aPoint2.Y() = n;
                        }
                        nSpFlags &= ~( SP_FFLIPV | SP_FFLIPH );

                        pRet->NbcSetPoint(aPoint1, 0L);	// Startpunkt
                        pRet->NbcSetPoint(aPoint2, 1L);	// Endpunkt

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
                }
            }

            if ( pRet )
            {
                if( nObjectRotation )
                {
                    double a = nObjectRotation * nPi180;
                    pRet->NbcRotate( aObjData.aBoundRect.Center(), nObjectRotation, sin( a ), cos( a ) );
                }
                // Horizontal gespiegelt?
                if ( nSpFlags & SP_FFLIPH )
                {
                    Rectangle aBndRect( pRet->GetSnapRect() );
                    Point aTop( ( aBndRect.Left() + aBndRect.Right() ) >> 1, aBndRect.Top() );
                    Point aBottom( aTop.X(), aTop.Y() + 1000 );
                    pRet->NbcMirror( aTop, aBottom );
                }
                // Vertikal gespiegelt?
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
        ::rtl::OUString aObjName = GetPropertyString( DFF_Prop_wzName, rSt );
        if( aObjName.getLength() > 0 )
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

    if ( mbTracing )
        mpTracer->RemoveAttribute( aObjData.nSpFlags & SP_FGROUP
                                    ? rtl::OUString::createFromAscii( "GroupShape" )
                                    : rtl::OUString::createFromAscii( "Shape" ) );
    return pRet;
}

Rectangle SvxMSDffManager::GetGlobalChildAnchor( const DffRecordHeader& rHd, SvStream& rSt, Rectangle& aClientRect )
{
    Rectangle aChildAnchor;
    rHd.SeekToContent( rSt );
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
                            INT16 ls, ts, rs, bs;
                            rSt >> ts >> ls >> rs >> bs; // etwas seltsame Koordinatenreihenfolge ...
                            l = ls, t = ts, r = rs, b = bs;
                        }
                        Scale( l );
                        Scale( t );
                        Scale( r );
                        Scale( b );
                        aClientRect = Rectangle( l, t, r, b );
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
                aShapeAtom.SeekToEndOfRecord( rSt );
            }
        }
        aShapeHd.SeekToEndOfRecord( rSt );
    }
    return aChildAnchor;
}

void SvxMSDffManager::GetGroupAnchors( const DffRecordHeader& rHd, SvStream& rSt,
                            Rectangle& rGroupClientAnchor, Rectangle& rGroupChildAnchor,
                                const Rectangle& rClientRect, const Rectangle& rGlobalChildRect )
{
    sal_Bool bFirst = sal_True;
    rHd.SeekToContent( rSt );
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
                aShapeAtom.SeekToEndOfRecord( rSt );
            }
        }
        aShapeHd.SeekToEndOfRecord( rSt );
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
            UINT32  nBytesLeft = maShapeRecords.Current()->nRecLen;
            UINT32	nUDData;
            UINT16  nPID;
            while( 5 < nBytesLeft )
            {
                rSt >> nPID;
                if ( rSt.GetError() != 0 )
                    break;
                rSt >> nUDData;
                switch( nPID )
                {
                    case 0x038F: pImpRec->nXAlign = nUDData; break;
                    case 0x0390: pImpRec->nXRelTo = nUDData; break;
                    case 0x0391: pImpRec->nYAlign = nUDData; break;
                    case 0x0392: pImpRec->nYRelTo = nUDData; break;
                    case 0x03BF: pImpRec->nLayoutInTableCell = nUDData; break;
                }
                if ( rSt.GetError() != 0 )
                    break;
                pImpRec->bHasUDefProp = TRUE;
                nBytesLeft  -= 6;
            }
        }

        //  Textrahmen, auch Title oder Outline
        SdrObject*  pOrgObj  = pObj;
        SdrRectObj* pTextObj = 0;
        UINT32 nTextId = GetPropertyValue( DFF_Prop_lTxid, 0 );
        if( nTextId )
        {
            SfxItemSet aSet( pSdrModel->GetItemPool() );

            //Originally anything that as a mso_sptTextBox was created as a
            //textbox, this was changed for #88277# to be created as a simple
            //rect to keep impress happy. For the rest of us we'd like to turn
            //it back into a textbox again.
            FASTBOOL bTextFrame = (pImpRec->eShapeType == mso_sptTextBox);
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
            INT32 nTextLeft = GetPropertyValue( DFF_Prop_dxTextLeft, 91440L);
            INT32 nTextRight = GetPropertyValue( DFF_Prop_dxTextRight, 91440L );
            INT32 nTextTop = GetPropertyValue( DFF_Prop_dyTextTop, 45720L  );
            INT32 nTextBottom = GetPropertyValue( DFF_Prop_dyTextBottom, 45720L );

            ScaleEmu( nTextLeft );
            ScaleEmu( nTextRight );
            ScaleEmu( nTextTop );
            ScaleEmu( nTextBottom );

            INT32 nTextRotationAngle=0;
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

                            INT32 nOldTextLeft = nTextLeft;
                            INT32 nOldTextRight = nTextRight;
                            INT32 nOldTextTop = nTextTop;
                            INT32 nOldTextBottom = nTextBottom;

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

                            INT32 nOldTextLeft = nTextLeft;
                            INT32 nOldTextRight = nTextRight;
                            INT32 nOldTextTop = nTextTop;
                            INT32 nOldTextBottom = nTextBottom;

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

            // Die vertikalen Absatzeinrueckungen sind im BoundRect mit drin,
            // hier rausrechnen
            Rectangle aNewRect(rTextRect);
            aNewRect.Bottom() -= nTextTop + nTextBottom;
            aNewRect.Right() -= nTextLeft + nTextRight;

            // Nur falls es eine einfache Textbox ist, darf der Writer
            // das Objekt durch einen Rahmen ersetzen, ansonsten
            if( bTextFrame )
            {
                SvxMSDffShapeInfo aTmpRec( 0, pImpRec->nShapeId );
                aTmpRec.bSortByShapeId = TRUE;

                USHORT nFound;
                if( pShapeInfos->Seek_Entry( &aTmpRec, &nFound ) )
                {
                    SvxMSDffShapeInfo& rInfo = *pShapeInfos->GetObject(nFound);
                    pTextImpRec->bReplaceByFly   = rInfo.bReplaceByFly;
                    pTextImpRec->bLastBoxInChain = rInfo.bLastBoxInChain;
                }
            }

            if( !pObj )
                ApplyAttributes( rSt, aSet, rObjData );

            bool bFitText = false;
            if (GetPropertyValue(DFF_Prop_FitTextToShape) & 2)
            {
                aSet.Put( SdrTextAutoGrowHeightItem( TRUE ) );
                aSet.Put( SdrTextMinFrameHeightItem(
                    aNewRect.Bottom() - aNewRect.Top() ) );
                aSet.Put( SdrTextMinFrameWidthItem(
                    aNewRect.Right() - aNewRect.Left() ) );
                bFitText = true;
            }
            else
            {
                aSet.Put( SdrTextAutoGrowHeightItem( FALSE ) );
                aSet.Put( SdrTextAutoGrowWidthItem( FALSE ) );
            }

            switch ( (MSO_WrapMode)
                GetPropertyValue( DFF_Prop_WrapText, mso_wrapSquare ) )
            {
                case mso_wrapNone :
                    aSet.Put( SdrTextAutoGrowWidthItem( TRUE ) );
                    if (bFitText)
                    {
                        //can't do autowidth in flys #i107184#
                        pTextImpRec->bReplaceByFly = false;
                    }
                break;
                case mso_wrapByPoints :
                    aSet.Put( SdrTextContourFrameItem( TRUE ) );
                break;
                default: break;
            }

            // Abstaende an den Raendern der Textbox setzen
            aSet.Put( SdrTextLeftDistItem( nTextLeft ) );
            aSet.Put( SdrTextRightDistItem( nTextRight ) );
            aSet.Put( SdrTextUpperDistItem( nTextTop ) );
            aSet.Put( SdrTextLowerDistItem( nTextBottom ) );
            pTextImpRec->nDxTextLeft	= nTextLeft;
            pTextImpRec->nDyTextTop		= nTextTop;
            pTextImpRec->nDxTextRight	= nTextRight;
            pTextImpRec->nDyTextBottom	= nTextBottom;

            // Textverankerung lesen
            if ( IsProperty( DFF_Prop_anchorText ) )
            {
                MSO_Anchor eTextAnchor =
                    (MSO_Anchor)GetPropertyValue( DFF_Prop_anchorText );

                SdrTextVertAdjust eTVA = SDRTEXTVERTADJUST_CENTER;
                BOOL bTVASet(FALSE);
                SdrTextHorzAdjust eTHA = SDRTEXTHORZADJUST_CENTER;
                BOOL bTHASet(FALSE);

                switch( eTextAnchor )
                {
                    case mso_anchorTop:
                    {
                        eTVA = SDRTEXTVERTADJUST_TOP;
                        bTVASet = TRUE;
                    }
                    break;
                    case mso_anchorTopCentered:
                    {
                        eTVA = SDRTEXTVERTADJUST_TOP;
                        bTVASet = TRUE;
                        bTHASet = TRUE;
                    }
                    break;

                    case mso_anchorMiddle:
                        bTVASet = TRUE;
                    break;
                    case mso_anchorMiddleCentered:
                    {
                        bTVASet = TRUE;
                        bTHASet = TRUE;
                    }
                    break;
                    case mso_anchorBottom:
                    {
                        eTVA = SDRTEXTVERTADJUST_BOTTOM;
                        bTVASet = TRUE;
                    }
                    break;
                    case mso_anchorBottomCentered:
                    {
                        eTVA = SDRTEXTVERTADJUST_BOTTOM;
                        bTVASet = TRUE;
                        bTHASet = TRUE;
                    }
                    break;
    /*
                    case mso_anchorTopBaseline:
                    case mso_anchorBottomBaseline:
                    case mso_anchorTopCenteredBaseline:
                    case mso_anchorBottomCenteredBaseline:
                    break;
    */
                    default : break;
                }
                // Einsetzen
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

            // rotate text with shape ?
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
                                                     FALSE, &pPoolItem );
            if( SFX_ITEM_DEFAULT == eState )
                aSet.Put( XFillColorItem( String(),
                          Color( mnDefaultColor ) ) );
            pObj->SetMergedItemSet(aSet);
        }

        //Means that fBehindDocument is set
        if (GetPropertyValue(DFF_Prop_fPrint) & 0x20)
            pImpRec->bDrawHell = TRUE;
        else
            pImpRec->bDrawHell = FALSE;
        if (GetPropertyValue(DFF_Prop_fPrint) & 0x02)
            pImpRec->bHidden = TRUE;
        pTextImpRec->bDrawHell	= pImpRec->bDrawHell;
        pTextImpRec->bHidden = pImpRec->bHidden;
        pImpRec->nNextShapeId	= GetPropertyValue( DFF_Prop_hspNext, 0 );
        pTextImpRec->nNextShapeId=pImpRec->nNextShapeId;

        if ( nTextId )
        {
            pTextImpRec->aTextId.nTxBxS = (UINT16)( nTextId >> 16 );
            pTextImpRec->aTextId.nSequence = (UINT16)nTextId;
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

        UINT32 nLineFlags = GetPropertyValue( DFF_Prop_fNoLineDrawDash );
        pImpRec->eLineStyle = (nLineFlags & 8)
                            ? (MSO_LineStyle)GetPropertyValue(
                                                DFF_Prop_lineStyle,
                                                mso_lineSimple )
                            : (MSO_LineStyle)USHRT_MAX;
        pTextImpRec->eLineStyle = pImpRec->eLineStyle;

        if( pImpRec->nShapeId )
        {
            // Import-Record-Liste ergaenzen
            if( pOrgObj )
            {
                pImpRec->pObj = pOrgObj;
                rImportData.aRecords.Insert( pImpRec );
            }

            if( pTextObj && (pOrgObj != pTextObj) )
            {
                // Modify ShapeId (must be unique)
                pImpRec->nShapeId |= 0x8000000;
                pTextImpRec->pObj = pTextObj;
                rImportData.aRecords.Insert( pTextImpRec );
            }

            // Eintrag in Z-Order-Liste um Zeiger auf dieses Objekt ergaenzen
            /*Only store objects which are not deep inside the tree*/
            if( ( rObjData.nCalledByGroup == 0 )
                ||
                ( (rObjData.nSpFlags & SP_FGROUP)
                 && (rObjData.nCalledByGroup < 2) )
              )
                StoreShapeOrder( pImpRec->nShapeId,
                                ( ( (ULONG)pImpRec->aTextId.nTxBxS ) << 16 )
                                    + pImpRec->aTextId.nSequence, pObj );
        }
        else
            delete pImpRec;
    }

    return pObj;
};

void SvxMSDffManager::StoreShapeOrder(ULONG			nId,
                                      ULONG			nTxBx,
                                      SdrObject*	pObject,
                                      SwFlyFrmFmt*	pFly,
                                      short			nHdFtSection) const
{
    USHORT nShpCnt = pShapeOrders->Count();
    for (USHORT nShapeNum=0; nShapeNum < nShpCnt; nShapeNum++)
    {
        SvxMSDffShapeOrder& rOrder
            = *(SvxMSDffShapeOrder*)(pShapeOrders->GetObject( nShapeNum ));

        if( rOrder.nShapeId == nId )
        {
            rOrder.nTxBxComp = nTxBx;
            rOrder.pObj      = pObject;
            rOrder.pFly      = pFly;
            rOrder.nHdFtSection = nHdFtSection;
        }
    }
}


void SvxMSDffManager::ExchangeInShapeOrder(	SdrObject*   pOldObject,
                                            ULONG        nTxBx,
                                            SwFlyFrmFmt* pFly,
                                            SdrObject*   pObject) const
{
    USHORT nShpCnt = pShapeOrders->Count();
    for (USHORT nShapeNum=0; nShapeNum < nShpCnt; nShapeNum++)
    {
        SvxMSDffShapeOrder& rOrder
            = *(SvxMSDffShapeOrder*)(pShapeOrders->GetObject( nShapeNum ));

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
    USHORT nShpCnt = pShapeOrders->Count();
    for (USHORT nShapeNum=0; nShapeNum < nShpCnt; nShapeNum++)
    {
        SvxMSDffShapeOrder& rOrder
            = *(SvxMSDffShapeOrder*)(pShapeOrders->GetObject( nShapeNum ));

        if( rOrder.pObj == pObject )
        {
            rOrder.pObj      = 0;
            rOrder.pFly      = 0;
            rOrder.nTxBxComp = 0;
        }
    }
}




//---------------------------------------------------------------------------
//  Hilfs Deklarationen
//---------------------------------------------------------------------------

/*struct SvxMSDffBLIPInfo                       -> in's Header-File
{
    USHORT nBLIPType;       // Art des BLIP: z.B. 6 fuer PNG
    ULONG  nFilePos;        // Offset des BLIP im Daten-Stream
    ULONG  nBLIPSize;       // Anzahl Bytes, die der BLIP im Stream einnimmt
    SvxMSDffBLIPInfo(USHORT nBType, ULONG nFPos, ULONG nBSize):
        nBLIPType( nBType ), nFilePos( nFPos ), nBLIPSize( nBSize ){}
};
*/

SV_IMPL_PTRARR(			SvxMSDffBLIPInfos,		SvxMSDffBLIPInfo_Ptr	);

SV_IMPL_PTRARR(			SvxMSDffShapeOrders,	SvxMSDffShapeOrder_Ptr	);

SV_IMPL_OP_PTRARR_SORT(	SvxMSDffShapeInfos,		SvxMSDffShapeInfo_Ptr	);

SV_IMPL_OP_PTRARR_SORT(	SvxMSDffShapeTxBxSort,	SvxMSDffShapeOrder_Ptr	);


// Liste aller SvxMSDffImportRec fuer eine Gruppe
SV_IMPL_OP_PTRARR_SORT(MSDffImportRecords, MSDffImportRec_Ptr)

//---------------------------------------------------------------------------
//  exportierte Klasse: oeffentliche Methoden
//---------------------------------------------------------------------------

SvxMSDffManager::SvxMSDffManager(SvStream& rStCtrl_,
                                 const String& rBaseURL,
                                 long      nOffsDgg_,
                                 SvStream* pStData_,
                                 SdrModel* pSdrModel_,// s. unten: SetModel()
                                 long      nApplicationScale,
                                 ColorData mnDefaultColor_,
                                 ULONG     nDefaultFontHeight_,
                                 SvStream* pStData2_,
                                 MSFilterTracer* pTracer )
    :DffPropertyReader( *this ),
     pFormModel( NULL ),
     pBLIPInfos( new SvxMSDffBLIPInfos  ),
     pShapeInfos(  new SvxMSDffShapeInfos ),
     pShapeOrders( new SvxMSDffShapeOrders ),
     nDefaultFontHeight( nDefaultFontHeight_),
     nOffsDgg( nOffsDgg_ ),
     nBLIPCount(  USHRT_MAX ),				// mit Error initialisieren, da wir erst pruefen,
     nShapeCount( USHRT_MAX ),              // ob Kontroll-Stream korrekte Daten enthaellt
     maBaseURL( rBaseURL ),
     mpFidcls( NULL ),
     rStCtrl(  rStCtrl_  ),
     pStData(  pStData_  ),
     pStData2( pStData2_ ),
     nSvxMSDffSettings( 0 ),
     nSvxMSDffOLEConvFlags( 0 ),
     pEscherBlipCache( NULL ),
     mnDefaultColor( mnDefaultColor_),
     mpTracer( pTracer ),
     mbTracing( sal_False )
{
    if ( mpTracer )
    {
        uno::Any aAny( mpTracer->GetProperty( rtl::OUString::createFromAscii( "On" ) ) );
        aAny >>= mbTracing;
    }
    SetModel( pSdrModel_, nApplicationScale );

    // FilePos des/der Stream(s) merken
    ULONG nOldPosCtrl = rStCtrl.Tell();
    ULONG nOldPosData = pStData ? pStData->Tell() : nOldPosCtrl;

    // Falls kein Datenstream angegeben, gehen wir davon aus,
    // dass die BLIPs im Steuerstream stehen.
    if( !pStData )
        pStData = &rStCtrl;

    SetDefaultPropSet( rStCtrl, nOffsDgg );

    // Steuer Stream auslesen, im Erfolgsfall nBLIPCount setzen
    GetCtrlData( nOffsDgg );

    // Text-Box-Story-Ketten-Infos ueberpruefen
    CheckTxBxStoryChain();

    // alte FilePos des/der Stream(s) restaurieren
    rStCtrl.Seek( nOldPosCtrl );
    if( &rStCtrl != pStData )
        pStData->Seek( nOldPosData );
}

SvxMSDffManager::SvxMSDffManager( SvStream& rStCtrl_, const String& rBaseURL, MSFilterTracer* pTracer )
    :DffPropertyReader( *this ),
     pFormModel( NULL ),
     pBLIPInfos(   new SvxMSDffBLIPInfos  ),
     pShapeInfos(  new SvxMSDffShapeInfos ),
     pShapeOrders( new SvxMSDffShapeOrders ),
     nDefaultFontHeight( 24 ),
     nOffsDgg( 0 ),
     nBLIPCount(  USHRT_MAX ),				// mit Error initialisieren, da wir erst pruefen,
     nShapeCount( USHRT_MAX ),              // ob Kontroll-Stream korrekte Daten enthaellt
     maBaseURL( rBaseURL ),
     mpFidcls( NULL ),
     rStCtrl(  rStCtrl_  ),
     pStData( 0 ),
     pStData2( 0 ),
     nSvxMSDffSettings( 0 ),
     nSvxMSDffOLEConvFlags( 0 ),
     pEscherBlipCache( NULL ),
     mnDefaultColor( COL_DEFAULT ),
     mpTracer( pTracer ),
     mbTracing( sal_False )
{
    if ( mpTracer )
    {
        uno::Any aAny( mpTracer->GetProperty( rtl::OUString::createFromAscii( "On" ) ) );
        aAny >>= mbTracing;
    }
    SetModel( NULL, 0 );
}

SvxMSDffManager::~SvxMSDffManager()
{
    if ( pEscherBlipCache )
    {
        void* pPtr;
        for ( pPtr = pEscherBlipCache->First(); pPtr; pPtr = pEscherBlipCache->Next() )
            delete (EscherBlipCacheEntry*)pPtr;
        delete pEscherBlipCache;
    }
    delete pBLIPInfos;
    delete pShapeInfos;
    delete pShapeOrders;
    delete pFormModel;
    delete[] mpFidcls;
}

void SvxMSDffManager::InitSvxMSDffManager( long nOffsDgg_, SvStream* pStData_, sal_uInt32 nOleConvFlags )
{
    nOffsDgg = nOffsDgg_;
    pStData = pStData_;
    nSvxMSDffOLEConvFlags = nOleConvFlags;

    // FilePos des/der Stream(s) merken
    ULONG nOldPosCtrl = rStCtrl.Tell();

    SetDefaultPropSet( rStCtrl, nOffsDgg );

    // insert fidcl cluster table
    GetFidclData( nOffsDgg );

    // Steuer Stream auslesen, im Erfolgsfall nBLIPCount setzen
    GetCtrlData( nOffsDgg );

    // Text-Box-Story-Ketten-Infos ueberpruefen
    CheckTxBxStoryChain();

    // alte FilePos des/der Stream(s) restaurieren
    rStCtrl.Seek( nOldPosCtrl );
}

void SvxMSDffManager::SetDgContainer( SvStream& rSt )
{
    UINT32 nFilePos = rSt.Tell();
    DffRecordHeader aDgContHd;
    rSt >> aDgContHd;
    // insert this container only if there is also a DgAtom
    if ( SeekToRec( rSt, DFF_msofbtDg, aDgContHd.GetRecEndFilePos() ) )
    {
        DffRecordHeader aRecHd;
        rSt >> aRecHd;
        UINT32 nDrawingId = aRecHd.nRecInstance;
        maDgOffsetTable.Insert( nDrawingId, (void*)nFilePos );
        rSt.Seek( nFilePos );
    }
}

void SvxMSDffManager::GetFidclData( long nOffsDggL )
{
    if ( nOffsDggL )
    {
        UINT32 nDummy, nMerk = rStCtrl.Tell();
        rStCtrl.Seek( nOffsDggL );

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
                if ( aDggAtomHd.nRecLen == ( mnIdClusters * sizeof( FIDCL ) + 16 ) )
                {
                    mpFidcls = new FIDCL[ mnIdClusters ];
                    for ( UINT32 i = 0; i < mnIdClusters; i++ )
                    {
                        rStCtrl >> mpFidcls[ i ].dgid
                                >> mpFidcls[ i ].cspidCur;
                    }
                }
            }
        }
        rStCtrl.Seek( nMerk );
    }
}

void SvxMSDffManager::CheckTxBxStoryChain()
{
    SvxMSDffShapeInfos* pOld = pShapeInfos;
    USHORT nCnt				= pOld->Count();
    pShapeInfos				= new SvxMSDffShapeInfos( (nCnt < 255)
                                                     ? nCnt
                                                     : 255 );
    // altes Info-Array ueberarbeiten
    // (ist sortiert nach nTxBxComp)
    ULONG nChain    = ULONG_MAX;
    USHORT nObjMark = 0;
    BOOL bSetReplaceFALSE = FALSE;
    USHORT nObj;
    for( nObj = 0; nObj < nCnt; ++nObj )
    {
        SvxMSDffShapeInfo* pObj = pOld->GetObject( nObj );
        if( pObj->nTxBxComp )
        {
            pObj->bLastBoxInChain = FALSE;
            // Gruppenwechsel ?
            // --> OD 2008-07-28 #156763#
            // the text id also contains an internal drawing container id
            // to distinguish between text id of drawing objects in different
            // drawing containers.
//            if( nChain != (pObj->nTxBxComp & 0xFFFF0000) )
            if( nChain != pObj->nTxBxComp )
            // <--
            {
                // voriger war letzter seiner Gruppe
                if( nObj )
                    pOld->GetObject( nObj-1 )->bLastBoxInChain = TRUE;
                // Merker und Hilfs-Flag zuruecksetzen
                nObjMark = nObj;
                // --> OD 2008-07-28 #156763#
//                nChain   = pObj->nTxBxComp & 0xFFFF0000;
                nChain = pObj->nTxBxComp;
                // <--
                bSetReplaceFALSE = !pObj->bReplaceByFly;
            }
            else
            if( !pObj->bReplaceByFly )
            {
                // Objekt, das NICHT durch Rahmen ersetzt werden darf ?
                // Hilfs-Flag setzen
                bSetReplaceFALSE = TRUE;
                // ggfs Flag in Anfang der Gruppe austragen
                for( USHORT nObj2 = nObjMark; nObj2 < nObj; ++nObj2 )
                    pOld->GetObject( nObj2 )->bReplaceByFly = FALSE;
            }

            if( bSetReplaceFALSE )
            {
                pObj->bReplaceByFly = FALSE;
            }
        }
        // alle Shape-Info-Objekte in pShapeInfos umkopieren
        // (aber nach nShapeId sortieren)
        pObj->bSortByShapeId = TRUE;
        // --> OD 2008-07-28 #156763#
        pObj->nTxBxComp = pObj->nTxBxComp & 0xFFFF0000;
        // <--
        pShapeInfos->Insert( pObj );
    }
    // voriger war letzter seiner Gruppe
    if( nObj )
        pOld->GetObject( nObj-1 )->bLastBoxInChain = TRUE;
    // urspruengliches Array freigeben, ohne Objekte zu zerstoeren
    pOld->Remove((USHORT)0, nCnt);
    delete pOld;
}


/*****************************************************************************

    Einlesen der Shape-Infos im Ctor:
    ---------------------------------
    merken der Shape-Ids und zugehoerigen Blip-Nummern und TextBox-Infos
               =========                  ============	   =============
    und merken des File-Offsets fuer jedes Blip
                   ============
******************************************************************************/
void SvxMSDffManager::GetCtrlData( long nOffsDgg_ )
{
    // Start Offset unbedingt merken, falls wir nochmal aufsetzen muessen
    long nOffsDggL = nOffsDgg_;

    // Kontroll Stream positionieren
    rStCtrl.Seek( nOffsDggL );

    BYTE   nVer;
    USHORT nInst;
    USHORT nFbt;
    UINT32  nLength;
    if( !this->ReadCommonRecordHeader( rStCtrl, nVer, nInst, nFbt, nLength ) ) return;

    BOOL bOk;
    ULONG nPos = nOffsDggL + DFF_COMMON_RECORD_HEADER_SIZE;

    // Fall A: erst Drawing Group Container, dann n Mal Drawing Container
    if( DFF_msofbtDggContainer == nFbt )
    {
        GetDrawingGroupContainerData( rStCtrl, nLength );

         rStCtrl.Seek( STREAM_SEEK_TO_END );
        UINT32 nMaxStrPos = rStCtrl.Tell();

        nPos += nLength;
        // --> OD 2008-07-28 #156763#
        unsigned long nDrawingContainerId = 1;
        // <--
        do
        {
            rStCtrl.Seek( nPos );

            bOk = ReadCommonRecordHeader( rStCtrl, nVer, nInst, nFbt, nLength ) && ( DFF_msofbtDgContainer == nFbt );

            if( !bOk )
            {
                nPos++;
                rStCtrl.Seek( nPos );
                bOk = ReadCommonRecordHeader( rStCtrl, nVer, nInst, nFbt, nLength )
                        && ( DFF_msofbtDgContainer == nFbt );
            }
            if( bOk )
            {
                // --> OD 2008-07-28 #156763#
                GetDrawingContainerData( rStCtrl, nLength, nDrawingContainerId );
                // <--
            }
            nPos += DFF_COMMON_RECORD_HEADER_SIZE + nLength;
            // --> OD 2008-07-28 #156763#
            ++nDrawingContainerId;
            // <--
        }
        while( nPos < nMaxStrPos && bOk );
    }
}


// ab hier: Drawing Group Container  d.h. Dokument - weit gueltige Daten
//                      =======================           ========
//
void SvxMSDffManager::GetDrawingGroupContainerData( SvStream& rSt, ULONG nLenDgg )
{
    BYTE   nVer;
    USHORT nInst;
    USHORT nFbt;
    UINT32 nLength;

    ULONG nLenBStoreCont = 0, nLenFBSE = 0, nRead = 0;

    // Nach einem BStore Container suchen
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

    // Im BStore Container alle Header der Container und Atome auslesen und die
    // relevanten Daten aller enthaltenen FBSEs in unserem Pointer Array ablegen.
    // Dabei zaehlen wir die gefundenen FBSEs im Member nBLIPCount mit.

    const ULONG nSkipBLIPLen = 20;  // bis zu nBLIPLen zu ueberspringende Bytes
    const ULONG nSkipBLIPPos =  4;  // dahinter bis zu nBLIPPos zu skippen

    sal_uInt32 nBLIPLen = 0, nBLIPPos = 0;

    nRead = 0;
    do
    {
        if(!this->ReadCommonRecordHeader( rSt, nVer, nInst, nFbt, nLength)) return;
        nRead += DFF_COMMON_RECORD_HEADER_SIZE + nLength;
        if( DFF_msofbtBSE == nFbt )
        {
            nLenFBSE = nLength;
            // ist FBSE gross genug fuer unsere Daten
            BOOL bOk = ( nSkipBLIPLen + 4 + nSkipBLIPPos + 4 <= nLenFBSE );

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
                // Besonderheit:
                // Falls nBLIPLen kleiner ist als nLenFBSE UND nBLIPPos Null ist,
                // nehmen wir an, dass das Bild IM FBSE drin steht!
                if( (!nBLIPPos) && (nBLIPLen < nLenFBSE) )
                    nBLIPPos = rSt.Tell() + 4;

                // Das hat ja fein geklappt!
                // Wir merken uns, dass wir einen FBSE mehr im Pointer Array haben.
                nBLIPPos = Calc_nBLIPPos(nBLIPPos, rSt.Tell());

                if( USHRT_MAX == nBLIPCount )
                    nBLIPCount = 1;
                else
                    nBLIPCount++;

                // Jetzt die Infos fuer spaetere Zugriffe speichern
                pBLIPInfos->Insert( new SvxMSDffBLIPInfo( nInst, nBLIPPos, nBLIPLen ),
                                                          pBLIPInfos->Count() );
            }
        }
        rSt.SeekRel( nLength );
    }
    while( nRead < nLenBStoreCont );
}


// ab hier: Drawing Container  d.h. Seiten (Blatt, Dia) - weit gueltige Daten
//                      =================               ======
//
void SvxMSDffManager::GetDrawingContainerData( SvStream& rSt, ULONG nLenDg,
                                               const unsigned long nDrawingContainerId )
{
    BYTE nVer;USHORT nInst;USHORT nFbt;UINT32 nLength;

    ULONG nReadDg = 0;

    // Wir stehen in einem Drawing Container (je einer pro Seite)
    // und muessen nun
    // alle enthaltenen Shape Group Container abklappern
    do
    {
        if(!this->ReadCommonRecordHeader( rSt, nVer, nInst, nFbt, nLength)) return;
        nReadDg += DFF_COMMON_RECORD_HEADER_SIZE;
        // Patriarch gefunden (der oberste Shape Group Container) ?
        if( DFF_msofbtSpgrContainer == nFbt )
        {
            if(!this->GetShapeGroupContainerData( rSt, nLength, TRUE, nDrawingContainerId )) return;
        }
        else
        // blanker Shape Container ? (ausserhalb vom Shape Group Container)
        if( DFF_msofbtSpContainer == nFbt )
        {
            if(!this->GetShapeContainerData( rSt, nLength, ULONG_MAX, nDrawingContainerId )) return;
        }
        else
            rSt.SeekRel( nLength );
        nReadDg += nLength;
    }
    while( nReadDg < nLenDg );
}

BOOL SvxMSDffManager::GetShapeGroupContainerData( SvStream& rSt,
                                                  ULONG nLenShapeGroupCont,
                                                  BOOL bPatriarch,
                                                  const unsigned long nDrawingContainerId )
{
    BYTE nVer;USHORT nInst;USHORT nFbt;UINT32 nLength;
    long nStartShapeGroupCont = rSt.Tell();
    // Wir stehen in einem Shape Group Container (ggfs. mehrere pro Seite)
    // und muessen nun
    // alle enthaltenen Shape Container abklappern
    BOOL  bFirst = !bPatriarch;
    ULONG nReadSpGrCont = 0;
    do
    {
        if( !this->ReadCommonRecordHeader( rSt, nVer, nInst, nFbt, nLength ) )
            return FALSE;
        nReadSpGrCont += DFF_COMMON_RECORD_HEADER_SIZE;
        // Shape Container ?
        if( DFF_msofbtSpContainer == nFbt )
        {
            ULONG nGroupOffs = bFirst ? nStartShapeGroupCont - DFF_COMMON_RECORD_HEADER_SIZE : ULONG_MAX;
            if ( !this->GetShapeContainerData( rSt, nLength, nGroupOffs, nDrawingContainerId ) )
                return FALSE;
            bFirst = FALSE;
        }
        else
        // eingeschachtelter Shape Group Container ?
        if( DFF_msofbtSpgrContainer == nFbt )
        {
            if ( !this->GetShapeGroupContainerData( rSt, nLength, FALSE, nDrawingContainerId ) )
                return FALSE;
        }
        else
            rSt.SeekRel( nLength );
        nReadSpGrCont += nLength;
    }
    while( nReadSpGrCont < nLenShapeGroupCont );
    // den Stream wieder korrekt positionieren
    rSt.Seek( nStartShapeGroupCont + nLenShapeGroupCont );
    return TRUE;
}

BOOL SvxMSDffManager::GetShapeContainerData( SvStream& rSt,
                                             ULONG nLenShapeCont,
                                             ULONG nPosGroup,
                                             const unsigned long nDrawingContainerId )
{
    BYTE nVer;USHORT nInst;USHORT nFbt;UINT32 nLength;
    long  nStartShapeCont = rSt.Tell();
    // Wir stehen in einem Shape Container (ggfs. mehrere pro Sh. Group)
    // und muessen nun
    // die Shape Id und File-Pos (fuer spaetere, erneute Zugriffe)
    // und den ersten BStore Verweis (falls vorhanden) entnehmen
    ULONG nLenShapePropTbl = 0;
    ULONG nReadSpCont = 0;

    // File Offset des Shape-Containers bzw. der Gruppe(!) vermerken
    //
    ULONG nStartOffs = (ULONG_MAX > nPosGroup) ?
                            nPosGroup : nStartShapeCont - DFF_COMMON_RECORD_HEADER_SIZE;
    SvxMSDffShapeInfo aInfo( nStartOffs );

    // duerfte das Shape durch einen Rahmen ersetzt werden ?
    // (vorausgesetzt, es zeigt sich, dass es eine TextBox ist,
    //  und der Text nicht gedreht ist)
    BOOL bCanBeReplaced = (ULONG_MAX > nPosGroup) ? FALSE : TRUE;

    // wir wissen noch nicht, ob es eine TextBox ist
    MSO_SPT			eShapeType		= mso_sptNil;
    MSO_WrapMode	eWrapMode		= mso_wrapSquare;
//	BOOL			bIsTextBox		= FALSE;

    // Shape analysieren
    //
    do
    {
        if(!this->ReadCommonRecordHeader( rSt, nVer, nInst, nFbt, nLength)) return FALSE;
        nReadSpCont += DFF_COMMON_RECORD_HEADER_SIZE;
        // FSP ?
        if( ( DFF_msofbtSp == nFbt ) && ( 4 <= nLength ) )
        {
            // Wir haben den FSP gefunden: Shape Typ und Id vermerken!
            eShapeType = (MSO_SPT)nInst;
            rSt >> aInfo.nShapeId;
            rSt.SeekRel( nLength - 4 );
            nReadSpCont += nLength;
        }
        else if( DFF_msofbtOPT == nFbt ) // Shape Property Table ?
        {
            // Wir haben die Property Table gefunden:
            // nach der Blip Property suchen!
            ULONG  nPropRead = 0;
            USHORT nPropId;
            sal_uInt32  nPropVal;
            nLenShapePropTbl = nLength;
//			UINT32 nPropCount = nInst;
            long nStartShapePropTbl = rSt.Tell();
//			UINT32 nComplexDataFilePos = nStartShapePropTbl + (nPropCount * 6);
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
                                bCanBeReplaced = FALSE;
                        }
                    break;
                    case DFF_Prop_Rotation :
                        if( 0 != nPropVal )
                            bCanBeReplaced = FALSE;
                    break;

                    case DFF_Prop_gtextFStrikethrough :
                        if( ( 0x20002000 & nPropVal )  == 0x20002000 )
                            bCanBeReplaced = FALSE;
                    break;

                    case DFF_Prop_fc3DLightFace :
                        if( ( 0x00080008 & nPropVal ) == 0x00080008 )
                            bCanBeReplaced = FALSE;
                    break;

                    case DFF_Prop_WrapText :
                        eWrapMode = (MSO_WrapMode)nPropVal;
                    break;

                    default:
                    {
                        // Bit gesetzt und gueltig?
                        if( 0x4000 == ( nPropId & 0xC000 ) )
                        {
                            // Blip Property gefunden: BStore Idx vermerken!
                            nPropRead = nLenShapePropTbl;
                        }
                        else if( 0x8000 & nPropId )
                        {
                            // komplexe Prop gefunden:
                            // Laenge ist immer 6, nur die Laenge der nach der
                            // eigentlichen Prop-Table anhaengenden Extra-Daten
                            // ist unterschiedlich
                            nPropVal = 6;
                        }
                    }
                    break;
                }

/*
//JP 21.04.99: Bug 64510
// alte Version, die unter OS/2 zu Compilerfehlern fuehrt und damit arge
// Performance einbussen hat.

                if( 0x4000 == ( nPropId & 0xC000 ) )// Bit gesetzt und gueltig?
                {
                    // Blip Property gefunden: BStore Idx vermerken!
                    aInfo.nBStoreIdx = nPropVal;    // Index im BStore Container
                    break;
                }
                else
                if(    (    (    (DFF_Prop_txflTextFlow   == nPropId)
                              || (DFF_Prop_Rotation       == nPropId)
                              || (DFF_Prop_cdirFont       == nPropId) )
                         && (0 != nPropVal) )

                    || (    (DFF_Prop_gtextFStrikethrough == nPropId)
                         && ( (0x20002000 & nPropVal)  == 0x20002000) ) // also DFF_Prop_gtextFVertical
                    || (    (DFF_Prop_fc3DLightFace       == nPropId)
                         && ( (0x00080008 & nPropVal)  == 0x00080008) )	// also DFF_Prop_f3D
                  )
                {
                    bCanBeReplaced = FALSE;  // Mist: gedrehter Text oder 3D-Objekt!
                }
                else
                if( DFF_Prop_WrapText == nPropId )
                {
                    eWrapMode = (MSO_WrapMode)nPropVal;
                }
                ////////////////////////////////////////////////////////////////
                ////////////////////////////////////////////////////////////////
                // keine weitere Property-Auswertung: folge beim Shape-Import //
                ////////////////////////////////////////////////////////////////
                ////////////////////////////////////////////////////////////////
                else
                if( 0x8000 & nPropId )
                {
                    // komplexe Prop gefunden: Laenge lesen und ueberspringen
                    if(!SkipBytes( rSt, nPropVal )) return FALSE;
                    nPropRead += nPropVal;
                }
*/
            }
            while( nPropRead < nLenShapePropTbl );
            rSt.Seek( nStartShapePropTbl + nLenShapePropTbl );
            nReadSpCont += nLenShapePropTbl;
        }
        else if( ( DFF_msofbtClientTextbox == nFbt ) && ( 4 == nLength ) )	// Text-Box-Story-Eintrag gefunden
        {
            rSt >> aInfo.nTxBxComp;
            // --> OD 2008-07-28 #156763#
            // Add internal drawing container id to text id.
            // Note: The text id uses the first two bytes, while the internal
            // drawing container id used the second two bytes.
            aInfo.nTxBxComp = ( aInfo.nTxBxComp & 0xFFFF0000 ) +
                              nDrawingContainerId;
            DBG_ASSERT( (aInfo.nTxBxComp & 0x0000FFFF) == nDrawingContainerId,
                        "<SvxMSDffManager::GetShapeContainerData(..)> - internal drawing container Id could not be correctly merged into DFF_msofbtClientTextbox value." );
            // <--
        }
        else
        {
            rSt.SeekRel( nLength );
            nReadSpCont += nLength;
        }
    }
    while( nReadSpCont < nLenShapeCont );

    //
    // Jetzt ggfs. die Infos fuer spaetere Zugriffe auf das Shape speichern
    //
    if( aInfo.nShapeId )
    {
        // fuer Textboxen ggfs. ersetzen durch Rahmen erlauben
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
            aInfo.bReplaceByFly = TRUE;
        }
        pShapeInfos->Insert(  new SvxMSDffShapeInfo(  aInfo          ) );
        pShapeOrders->Insert( new SvxMSDffShapeOrder( aInfo.nShapeId ),
                              pShapeOrders->Count() );
    }

    // und den Stream wieder korrekt positionieren
    rSt.Seek( nStartShapeCont + nLenShapeCont );
    return TRUE;
}



/*****************************************************************************

    Zugriff auf ein Shape zur Laufzeit (ueber die Shape-Id)
    ----------------------------------
******************************************************************************/
BOOL SvxMSDffManager::GetShape(ULONG nId, SdrObject*&         rpShape,
                                          SvxMSDffImportData& rData)
{
    SvxMSDffShapeInfo aTmpRec(0, nId);
    aTmpRec.bSortByShapeId = TRUE;

    USHORT nFound;
    if( pShapeInfos->Seek_Entry(&aTmpRec, &nFound) )
    {
        SvxMSDffShapeInfo& rInfo = *pShapeInfos->GetObject( nFound );

        // eventuell altes Errorflag loeschen
        if( rStCtrl.GetError() )
            rStCtrl.ResetError();
        // FilePos des/der Stream(s) merken
        ULONG nOldPosCtrl = rStCtrl.Tell();
        ULONG nOldPosData = pStData ? pStData->Tell() : nOldPosCtrl;
        // das Shape im Steuer Stream anspringen
        rStCtrl.Seek( rInfo.nFilePos );

        // Falls missglueckt, den Fehlerstatus zuruecksetzen und Pech gehabt!
        if( rStCtrl.GetError() )
            rStCtrl.ResetError();
        else
            rpShape = ImportObj( rStCtrl, &rData, rData.aParentRect, rData.aParentRect );

        // alte FilePos des/der Stream(s) restaurieren
        rStCtrl.Seek( nOldPosCtrl );
        if( &rStCtrl != pStData )
            pStData->Seek( nOldPosData );
        return ( 0 != rpShape );
    }
    return FALSE;
}



/*      Zugriff auf ein BLIP zur Laufzeit (bei bereits bekannter Blip-Nr)
    ---------------------------------
******************************************************************************/
BOOL SvxMSDffManager::GetBLIP( ULONG nIdx_, Graphic& rData, Rectangle* pVisArea ) const
{
    BOOL bOk = FALSE;       // Ergebnisvariable initialisieren
    if ( pStData )
    {
        // check if a graphic for this blipId is already imported
        if ( nIdx_ && pEscherBlipCache )
        {
            EscherBlipCacheEntry* pEntry;
            for ( pEntry = (EscherBlipCacheEntry*)pEscherBlipCache->First(); pEntry;
                    pEntry = (EscherBlipCacheEntry*)pEscherBlipCache->Next() )
            {
                if ( pEntry->nBlip == nIdx_ )
                {	/* if this entry is available, then it should be possible
                    to get the Graphic via GraphicObject */
                    GraphicObject aGraphicObject( pEntry->aUniqueID );
                    rData = aGraphicObject.GetGraphic();
                    if ( rData.GetType() != GRAPHIC_NONE )
                        bOk = sal_True;
                    else
                        delete (EscherBlipCacheEntry*)pEscherBlipCache->Remove();
                    break;
                }
            }
        }
        if ( !bOk )
        {
            USHORT nIdx = USHORT( nIdx_ );
            if( !nIdx || (pBLIPInfos->Count() < nIdx) ) return FALSE;

            // eventuell alte(s) Errorflag(s) loeschen
            if( rStCtrl.GetError() )
                rStCtrl.ResetError();
            if(    ( &rStCtrl != pStData )
                && pStData->GetError() )
                pStData->ResetError();

            // FilePos des/der Stream(s) merken
            ULONG nOldPosCtrl = rStCtrl.Tell();
            ULONG nOldPosData = pStData ? pStData->Tell() : nOldPosCtrl;

            // passende Info-Struct aus unserem Pointer Array nehmen
            SvxMSDffBLIPInfo& rInfo = *(*pBLIPInfos)[ nIdx-1 ];

            // das BLIP Atom im Daten Stream anspringen
            pStData->Seek( rInfo.nFilePos );
            // ggfs. Fehlerstatus zuruecksetzen
            if( pStData->GetError() )
                pStData->ResetError();
            else
                bOk = GetBLIPDirect( *pStData, rData, pVisArea );
            if( pStData2 && !bOk )
            {
                // Fehler, aber zweite Chance: es gibt noch einen zweiten
                //         Datenstream, in dem die Grafik liegen koennte!
                if( pStData2->GetError() )
                    pStData2->ResetError();
                ULONG nOldPosData2 = pStData2->Tell();
                // das BLIP Atom im zweiten Daten Stream anspringen
                pStData2->Seek( rInfo.nFilePos );
                // ggfs. Fehlerstatus zuruecksetzen
                if( pStData2->GetError() )
                    pStData2->ResetError();
                else
                    bOk = GetBLIPDirect( *pStData2, rData, pVisArea );
                // alte FilePos des zweiten Daten-Stream restaurieren
                pStData2->Seek( nOldPosData2 );
            }
            // alte FilePos des/der Stream(s) restaurieren
            rStCtrl.Seek( nOldPosCtrl );
            if( &rStCtrl != pStData )
              pStData->Seek( nOldPosData );

            if ( bOk )
            {
                // create new BlipCacheEntry for this graphic
                GraphicObject aGraphicObject( rData );
                if ( !pEscherBlipCache )
                    const_cast <SvxMSDffManager*> (this)->pEscherBlipCache = new List();
                EscherBlipCacheEntry* pNewEntry = new EscherBlipCacheEntry( nIdx_, aGraphicObject.GetUniqueID() );
                pEscherBlipCache->Insert( pNewEntry, LIST_APPEND );
            }
        }
    }
    return bOk;
}

/*      Zugriff auf ein BLIP zur Laufzeit (mit korrekt positioniertem Stream)
    ---------------------------------
******************************************************************************/
BOOL SvxMSDffManager::GetBLIPDirect( SvStream& rBLIPStream, Graphic& rData, Rectangle* pVisArea ) const
{
    ULONG nOldPos = rBLIPStream.Tell();

    int nRes = GRFILTER_OPENERROR;  // Fehlervariable initialisieren

    // nachschauen, ob es sich auch wirklich um ein BLIP handelt
    UINT32 nLength;
    USHORT nInst, nFbt( 0 );
    BYTE   nVer;
    if( ReadCommonRecordHeader( rBLIPStream, nVer, nInst, nFbt, nLength) && ( 0xF018 <= nFbt ) && ( 0xF117 >= nFbt ) )
    {
        Size		aMtfSize100;
        BOOL		bMtfBLIP = FALSE;
        BOOL		bZCodecCompression = FALSE;
        // Nun exakt auf den Beginn der eingebetteten Grafik positionieren
        ULONG nSkip = ( nInst & 0x0001 ) ? 32 : 16;

        switch( nInst & 0xFFFE )
        {
            case 0x216 :			// Metafile header then compressed WMF
            case 0x3D4 :			// Metafile header then compressed EMF
            case 0x542 :			// Metafile hd. then compressed PICT
            {
                rBLIPStream.SeekRel( nSkip + 20 );

                // read in size of metafile in EMUS
                rBLIPStream >> aMtfSize100.Width() >> aMtfSize100.Height();

                // scale to 1/100mm
                aMtfSize100.Width() /= 360, aMtfSize100.Height() /= 360;

                if ( pVisArea )		// seem that we currently are skipping the visarea position
                    *pVisArea = Rectangle( Point(), aMtfSize100 );

                // skip rest of header
                nSkip = 6;
                bMtfBLIP = bZCodecCompression = TRUE;
            }
            break;
            case 0x46A :			// One byte tag then JPEG (= JFIF) data
            case 0x6E0 :			// One byte tag then PNG data
            case 0x7A8 :
                nSkip += 1;			// One byte tag then DIB data
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
            pOut->SetResizeOffset( 0 );	// sj: #i102257# setting ResizeOffset of 0 prevents from seeking
                                        // behind the stream end (allocating too much memory)
            pGrStream = pOut;
        }

//#define DBG_EXTRACTGRAPHICS
#ifdef DBG_EXTRACTGRAPHICS

        static sal_Int32 nCount;

        String aFileName( String( RTL_CONSTASCII_STRINGPARAM( "dbggfx" ) ) );
        aFileName.Append( String::CreateFromInt32( nCount++ ) );
        switch( nInst &~ 1 )
        {
            case 0x216 : aFileName.Append( String( RTL_CONSTASCII_STRINGPARAM( ".wmf" ) ) ); break;
            case 0x3d4 : aFileName.Append( String( RTL_CONSTASCII_STRINGPARAM( ".emf" ) ) ); break;
            case 0x542 : aFileName.Append( String( RTL_CONSTASCII_STRINGPARAM( ".pct" ) ) ); break;
            case 0x46a : aFileName.Append( String( RTL_CONSTASCII_STRINGPARAM( ".jpg" ) ) ); break;
            case 0x6e0 : aFileName.Append( String( RTL_CONSTASCII_STRINGPARAM( ".png" ) ) ); break;
            case 0x7a8 : aFileName.Append( String( RTL_CONSTASCII_STRINGPARAM( ".bmp" ) ) ); break;
        }

        String aURLStr;

        if( ::utl::LocalFileHelper::ConvertPhysicalNameToURL( Application::GetAppFileName(), aURLStr ) )
        {
            INetURLObject aURL( aURLStr );

            aURL.removeSegment();
            aURL.removeFinalSlash();
            aURL.Append( aFileName );

            SvStream* pDbgOut = ::utl::UcbStreamHelper::CreateStream( aURL.GetMainURL( INetURLObject::NO_DECODE ), STREAM_TRUNC | STREAM_WRITE );

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
        {	// DIBs direkt holen
            Bitmap aNew;
            if( aNew.Read( *pGrStream, FALSE ) )
            {
                rData = Graphic( aNew );
                nRes = GRFILTER_OK;
            }
        }
        else
        {	// und unsere feinen Filter darauf loslassen
            GraphicFilter* pGF = GraphicFilter::GetGraphicFilter();
            String aEmptyStr;
            nRes = pGF->ImportGraphic( rData, aEmptyStr, *pGrStream, GRFILTER_FORMAT_DONTKNOW );

            // SJ: I40472, sometimes the aspect ratio (aMtfSize100) does not match and we get scaling problems,
            // then it is better to use the prefsize that is stored within the metafile. Bug #72846# for what the
            // scaling has been implemented does not happen anymore.
            //
            // For pict graphics we will furthermore scale the metafile, because font scaling leads to error if the
            // dxarray is empty (this has been solved in wmf/emf but not for pict)
            if( bMtfBLIP && ( GRFILTER_OK == nRes ) && ( rData.GetType() == GRAPHIC_GDIMETAFILE ) && ( ( nInst & 0xFFFE ) == 0x542 ) )
            {
                if ( ( aMtfSize100.Width() >= 1000 ) && ( aMtfSize100.Height() >= 1000 ) )
                {	// #75956#, scaling does not work properly, if the graphic is less than 1cm
                    GDIMetaFile	aMtf( rData.GetGDIMetaFile() );
                    const Size	aOldSize( aMtf.GetPrefSize() );

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
        // ggfs. Fehlerstatus zuruecksetzen
        if ( ERRCODE_IO_PENDING == pGrStream->GetError() )
          pGrStream->ResetError();
        delete pOut;
    }
    rBLIPStream.Seek( nOldPos );    // alte FilePos des Streams restaurieren

    return ( GRFILTER_OK == nRes ); // Ergebniss melden
}

/* static */
BOOL SvxMSDffManager::ReadCommonRecordHeader(DffRecordHeader& rRec, SvStream& rIn)
{
    rRec.nFilePos = rIn.Tell();
    return SvxMSDffManager::ReadCommonRecordHeader( rIn,rRec.nRecVer,
                                                    rRec.nRecInstance,
                                                    rRec.nRecType,
                                                    rRec.nRecLen );
}


/* auch static */
BOOL SvxMSDffManager::ReadCommonRecordHeader( SvStream& rSt,
                                              BYTE&     rVer,
                                              USHORT&   rInst,
                                              USHORT&   rFbt,
                                              UINT32&    rLength )
{
    sal_uInt16 nTmp;
    rSt >> nTmp >> rFbt >> rLength;
    rVer = sal::static_int_cast< BYTE >(nTmp & 15);
    rInst = nTmp >> 4;
    return rSt.GetError() == 0;
}




BOOL SvxMSDffManager::ProcessClientAnchor(SvStream& rStData, ULONG nDatLen,
                                          char*& rpBuff, UINT32& rBuffLen ) const
{
    if( nDatLen )
    {
        rpBuff = new char[ nDatLen ];
        rBuffLen = nDatLen;
        rStData.Read( rpBuff, nDatLen );
    }
    return TRUE;
}

BOOL SvxMSDffManager::ProcessClientData(SvStream& rStData, ULONG nDatLen,
                                        char*& rpBuff, UINT32& rBuffLen ) const
{
    if( nDatLen )
    {
        rpBuff = new char[ nDatLen ];
        rBuffLen = nDatLen;
        rStData.Read( rpBuff, nDatLen );
    }
    return TRUE;
}


void SvxMSDffManager::ProcessClientAnchor2( SvStream& /* rSt */, DffRecordHeader& /* rHd */ , void* /* pData */, DffObjData& /* rObj */ )
{
    return;  // wird von SJ im Draw ueberladen
}

ULONG SvxMSDffManager::Calc_nBLIPPos( ULONG nOrgVal, ULONG /* nStreamPos */ ) const
{
    return nOrgVal;
}

BOOL SvxMSDffManager::GetOLEStorageName( long /* nOLEId */, String&, SvStorageRef&, uno::Reference < embed::XStorage >& ) const
{
    return FALSE;
}

BOOL SvxMSDffManager::ShapeHasText( ULONG /* nShapeId */, ULONG /* nFilePos */ ) const
{
    return TRUE;
}

// --> OD 2004-12-14 #i32596# - add new parameter <_nCalledByGroup>
SdrObject* SvxMSDffManager::ImportOLE( long nOLEId,
                                       const Graphic& rGrf,
                                       const Rectangle& rBoundRect,
                                       const Rectangle& rVisArea,
                                       const int /* _nCalledByGroup */,
                                       sal_Int64 nAspect ) const
// <--
{
    SdrObject* pRet = 0;
    String sStorageName;
    SvStorageRef xSrcStg;
    ErrCode nError = ERRCODE_NONE;
    uno::Reference < embed::XStorage > xDstStg;
    if( GetOLEStorageName( nOLEId, sStorageName, xSrcStg, xDstStg ))
        pRet = CreateSdrOLEFromStorage( sStorageName, xSrcStg, xDstStg,
                                        rGrf, rBoundRect, rVisArea, pStData, nError,
                                        nSvxMSDffOLEConvFlags, nAspect );
    return pRet;
}

const GDIMetaFile* SvxMSDffManager::lcl_GetMetaFileFromGrf_Impl( const Graphic& rGrf,
                                                        GDIMetaFile& rMtf )
{
    const GDIMetaFile* pMtf;
    if( GRAPHIC_BITMAP == rGrf.GetType() )
    {
        Point aPt;
        const Size aSz(lcl_GetPrefSize(rGrf, MAP_100TH_MM));

        VirtualDevice aVirtDev;
        aVirtDev.EnableOutput( FALSE );
        MapMode aMM(MAP_100TH_MM);
        aVirtDev.SetMapMode( aMM );

        rMtf.Record( &aVirtDev );
        rGrf.Draw( &aVirtDev, aPt, aSz );
        rMtf.Stop();
        rMtf.SetPrefMapMode(aMM);
        rMtf.SetPrefSize( aSz );

        pMtf = &rMtf;
    }
    else
        pMtf = &rGrf.GetGDIMetaFile();
    return pMtf;
}

BOOL SvxMSDffManager::MakeContentStream( SotStorage * pStor, const GDIMetaFile & rMtf )
{
    String aPersistStream( String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( SVEXT_PERSIST_STREAM ) ) );
    SotStorageStreamRef xStm = pStor->OpenSotStream( aPersistStream );
    xStm->SetVersion( pStor->GetVersion() );
    xStm->SetBufferSize( 8192 );

    USHORT nAspect = ASPECT_CONTENT;
    ULONG nAdviseModes = 2;

    Impl_OlePres aEle( FORMAT_GDIMETAFILE );
    // Die Groesse in 1/100 mm umrechnen
    // Falls eine nicht anwendbare MapUnit (Device abhaengig) verwendet wird,
    // versucht SV einen BestMatchden richtigen Wert zu raten.
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
    UINT32		nId;
    const sal_Char* pSvrName;
    const sal_Char* pDspName;
};
static ClsIDs aClsIDs[] = {

    { 0x000212F0, "MSWordArt",     		"Microsoft Word Art"	 		},
    { 0x000212F0, "MSWordArt.2",   		"Microsoft Word Art 2.0" 		},

    // MS Apps
    { 0x00030000, "ExcelWorksheet",		"Microsoft Excel Worksheet"		},
    { 0x00030001, "ExcelChart",			"Microsoft Excel Chart"			},
    { 0x00030002, "ExcelMacrosheet",	"Microsoft Excel Macro"			},
    { 0x00030003, "WordDocument",		"Microsoft Word Document"		},
    { 0x00030004, "MSPowerPoint",		"Microsoft PowerPoint"			},
    { 0x00030005, "MSPowerPointSho",	"Microsoft PowerPoint Slide Show"},
    { 0x00030006, "MSGraph",			"Microsoft Graph"				},
    { 0x00030007, "MSDraw",				"Microsoft Draw"				},
    { 0x00030008, "Note-It",			"Microsoft Note-It"				},
    { 0x00030009, "WordArt",			"Microsoft Word Art"			},
    { 0x0003000a, "PBrush",				"Microsoft PaintBrush Picture"	},
    { 0x0003000b, "Equation",			"Microsoft Equation Editor"		},
    { 0x0003000c, "Package",			"Package"						},
    { 0x0003000d, "SoundRec",			"Sound"							},
    { 0x0003000e, "MPlayer",			"Media Player"					},
    // MS Demos
    { 0x0003000f, "ServerDemo",			"OLE 1.0 Server Demo"			},
    { 0x00030010, "Srtest",				"OLE 1.0 Test Demo"				},
    { 0x00030011, "SrtInv",				"OLE 1.0 Inv Demo"				},
    { 0x00030012, "OleDemo",			"OLE 1.0 Demo"					},

    // Coromandel / Dorai Swamy / 718-793-7963
    { 0x00030013, "CoromandelIntegra",	"Coromandel Integra"			},
    { 0x00030014, "CoromandelObjServer","Coromandel Object Server"		},

    // 3-d Visions Corp / Peter Hirsch / 310-325-1339
    { 0x00030015, "StanfordGraphics",	"Stanford Graphics"				},

    // Deltapoint / Nigel Hearne / 408-648-4000
    { 0x00030016, "DGraphCHART",		"DeltaPoint Graph Chart"		},
    { 0x00030017, "DGraphDATA",			"DeltaPoint Graph Data"			},

    // Corel / Richard V. Woodend / 613-728-8200 x1153
    { 0x00030018, "PhotoPaint",			"Corel PhotoPaint"				},
    { 0x00030019, "CShow",				"Corel Show"					},
    { 0x0003001a, "CorelChart",			"Corel Chart"					},
    { 0x0003001b, "CDraw",				"Corel Draw"					},

    // Inset Systems / Mark Skiba / 203-740-2400
    { 0x0003001c, "HJWIN1.0",			"Inset Systems"					},

    // Mark V Systems / Mark McGraw / 818-995-7671
    { 0x0003001d, "ObjMakerOLE",		"MarkV Systems Object Maker"	},

    // IdentiTech / Mike Gilger / 407-951-9503
    { 0x0003001e, "FYI",				"IdentiTech FYI"				},
    { 0x0003001f, "FYIView",			"IdentiTech FYI Viewer"			},

    // Inventa Corporation / Balaji Varadarajan / 408-987-0220
    { 0x00030020, "Stickynote",			"Inventa Sticky Note"			},

    // ShapeWare Corp. / Lori Pearce / 206-467-6723
    { 0x00030021, "ShapewareVISIO10",   "Shapeware Visio 1.0"			},
    { 0x00030022, "ImportServer",		"Spaheware Import Server"		},

    // test app SrTest
    { 0x00030023, "SrvrTest",			"OLE 1.0 Server Test"			},

    // test app ClTest.  Doesn't really work as a server but is in reg db
    { 0x00030025, "Cltest",				"OLE 1.0 Client Test"			},

    // Microsoft ClipArt Gallery   Sherry Larsen-Holmes
    { 0x00030026, "MS_ClipArt_Gallery",	"Microsoft ClipArt Gallery"		},
    // Microsoft Project  Cory Reina
    { 0x00030027, "MSProject",			"Microsoft Project"				},

    // Microsoft Works Chart
    { 0x00030028, "MSWorksChart",		"Microsoft Works Chart"			},

    // Microsoft Works Spreadsheet
    { 0x00030029, "MSWorksSpreadsheet",	"Microsoft Works Spreadsheet"	},

    // AFX apps - Dean McCrory
    { 0x0003002A, "MinSvr",				"AFX Mini Server"				},
    { 0x0003002B, "HierarchyList",		"AFX Hierarchy List"			},
    { 0x0003002C, "BibRef",				"AFX BibRef"					},
    { 0x0003002D, "MinSvrMI",			"AFX Mini Server MI"			},
    { 0x0003002E, "TestServ",			"AFX Test Server"				},

    // Ami Pro
    { 0x0003002F, "AmiProDocument",		"Ami Pro Document"				},

    // WordPerfect Presentations For Windows
    { 0x00030030, "WPGraphics",			"WordPerfect Presentation"		},
    { 0x00030031, "WPCharts",			"WordPerfect Chart"				},

    // MicroGrafx Charisma
    { 0x00030032, "Charisma",			"MicroGrafx Charisma"			},
    { 0x00030033, "Charisma_30",		"MicroGrafx Charisma 3.0"		},
    { 0x00030034, "CharPres_30",		"MicroGrafx Charisma 3.0 Pres"	},
    // MicroGrafx Draw
    { 0x00030035, "Draw",				"MicroGrafx Draw"				},
    // MicroGrafx Designer
    { 0x00030036, "Designer_40",		"MicroGrafx Designer 4.0"		},

    // STAR DIVISION
//	{ 0x000424CA, "StarMath",			"StarMath 1.0"					},
    { 0x00043AD2, "FontWork",			"Star FontWork"					},
//	{ 0x000456EE, "StarMath2",			"StarMath 2.0"					},

    { 0, "", "" } };


BOOL SvxMSDffManager::ConvertToOle2( SvStream& rStm, UINT32 nReadLen,
                    const GDIMetaFile * pMtf, const SotStorageRef& rDest )
{
    BOOL bMtfRead = FALSE;
    SotStorageStreamRef xOle10Stm = rDest->OpenSotStream( String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "\1Ole10Native" ) ),
                                                    STREAM_WRITE| STREAM_SHARE_DENYALL );
    if( xOle10Stm->GetError() )
        return FALSE;

    UINT32 nType;
    UINT32 nRecType;
    UINT32 nStrLen;
    String aSvrName;
    UINT32 nDummy0;
    UINT32 nDummy1;
    UINT32 nDataLen;
    BYTE * pData;
    UINT32 nBytesRead = 0;
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
                aSvrName.Assign( String( pBuf, (USHORT) nStrLen-1, gsl_getSystemTextEncoding() ) );
                delete[] pBuf;
            }
            else
                break;
        }
        rStm >> nDummy0;
        rStm >> nDummy1;
        rStm >> nDataLen;

        nBytesRead += 6 * sizeof( UINT32 ) + nStrLen + nDataLen;

        if( !rStm.IsEof() && nReadLen > nBytesRead && nDataLen )
        {
            if( xOle10Stm.Is() )
            {
                pData = new BYTE[ nDataLen ];
                if( !pData )
                    return FALSE;

                rStm.Read( pData, nDataLen );

                // write to ole10 stream
                *xOle10Stm << nDataLen;
                xOle10Stm->Write( pData, nDataLen );
                xOle10Stm = SotStorageStreamRef();

                // set the compobj stream
                ClsIDs* pIds;
                for( pIds = aClsIDs; pIds->nId; pIds++ )
                {
                    if( COMPARE_EQUAL == aSvrName.CompareToAscii( pIds->pSvrName ) )
                        break;
                }
//				SvGlobalName* pClsId = NULL;
                String aShort, aFull;
                if( pIds->nId )
                {
                    // gefunden!
                    ULONG nCbFmt = SotExchange::RegisterFormatName( aSvrName );
                    rDest->SetClass( SvGlobalName( pIds->nId, 0, 0, 0xc0,0,0,0,0,0,0,0x46 ), nCbFmt,
                                    String( pIds->pDspName, RTL_TEXTENCODING_ASCII_US ) );
                }
                else
                {
                    ULONG nCbFmt = SotExchange::RegisterFormatName( aSvrName );
                    rDest->SetClass( SvGlobalName(), nCbFmt, aSvrName );
                }

                delete[] pData;
            }
            else if( nRecType == 5 && !pMtf )
            {
                ULONG nPos = rStm.Tell();
                UINT16 sz[4];
                rStm.Read( sz, 8 );
                //rStm.SeekRel( 8 );
                Graphic aGraphic;
                if( ERRCODE_NONE == GraphicConverter::Import( rStm, aGraphic ) && aGraphic.GetType() )
                {
                    const GDIMetaFile& rMtf = aGraphic.GetGDIMetaFile();
                    MakeContentStream( rDest, rMtf );
                    bMtfRead = TRUE;
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
        return TRUE;
    }

    return FALSE;
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

::rtl::OUString GetFilterNameFromClassID_Impl( const SvGlobalName& aGlobName )
{
    if ( aGlobName == SvGlobalName( SO3_SW_OLE_EMBED_CLASSID_60 ) )
        return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "StarOffice XML (Writer)" ) );

    if ( aGlobName == SvGlobalName( SO3_SW_OLE_EMBED_CLASSID_8 ) )
        return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "writer8" ) );

    if ( aGlobName == SvGlobalName( SO3_SC_OLE_EMBED_CLASSID_60 ) )
        return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "StarOffice XML (Calc)" ) );

    if ( aGlobName == SvGlobalName( SO3_SC_OLE_EMBED_CLASSID_8 ) )
        return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "calc8" ) );

    if ( aGlobName == SvGlobalName( SO3_SIMPRESS_OLE_EMBED_CLASSID_60 ) )
        return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "StarOffice XML (Impress)" ) );

    if ( aGlobName == SvGlobalName( SO3_SIMPRESS_OLE_EMBED_CLASSID_8 ) )
        return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "impress8" ) );

    if ( aGlobName == SvGlobalName( SO3_SDRAW_OLE_EMBED_CLASSID_60 ) )
        return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "StarOffice XML (Draw)" ) );

    if ( aGlobName == SvGlobalName( SO3_SDRAW_OLE_EMBED_CLASSID_8 ) )
        return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "draw8" ) );

    if ( aGlobName == SvGlobalName( SO3_SM_OLE_EMBED_CLASSID_60 ) )
        return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "StarOffice XML (Math)" ) );

    if ( aGlobName == SvGlobalName( SO3_SM_OLE_EMBED_CLASSID_8 ) )
        return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "math8" ) );

    if ( aGlobName == SvGlobalName( SO3_SCH_OLE_EMBED_CLASSID_60 ) )
        return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "StarOffice XML (Chart)" ) );

    if ( aGlobName == SvGlobalName( SO3_SCH_OLE_EMBED_CLASSID_8 ) )
        return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "chart8" ) );

    return ::rtl::OUString();
}

com::sun::star::uno::Reference < com::sun::star::embed::XEmbeddedObject >  SvxMSDffManager::CheckForConvertToSOObj( UINT32 nConvertFlags,
                        SotStorage& rSrcStg, const uno::Reference < embed::XStorage >& rDestStorage,
                        const Graphic& rGrf,
                        const Rectangle& rVisArea )
{
    uno::Reference < embed::XEmbeddedObject > xObj;
    SvGlobalName aStgNm = rSrcStg.GetClassName();
    const char* pName = GetInternalServerName_Impl( aStgNm );
    String sStarName;
    if ( pName )
        sStarName = String::CreateFromAscii( pName );
    else if ( nConvertFlags )
    {
        static struct _ObjImpType
        {
            UINT32 nFlag;
            const char* pFactoryNm;
            // GlobalNameId
            UINT32 n1;
            USHORT n2, n3;
            BYTE b8, b9, b10, b11, b12, b13, b14, b15;
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
                    sStarName = String::CreateFromAscii( pArr->pFactoryNm );
                    break;
                }
            }
        }
    }

    if ( sStarName.Len() )
    {
        //TODO/MBA: check if (and when) storage and stream will be destroyed!
        const SfxFilter* pFilter = 0;
        SvMemoryStream* pStream = new SvMemoryStream;
        if ( pName )
        {
            // TODO/LATER: perhaps we need to retrieve VisArea and Metafile from the storage also
            SotStorageStreamRef xStr = rSrcStg.OpenSotStream( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "package_stream" ) ), STREAM_STD_READ );
            *xStr >> *pStream;
        }
        else
        {
            SfxFilterMatcher aMatch( sStarName );
            SotStorageRef xStorage = new SotStorage( FALSE, *pStream );
            rSrcStg.CopyTo( xStorage );
            xStorage->Commit();
            xStorage.Clear();
            String aType = SfxFilter::GetTypeFromStorage( rSrcStg );
            if ( aType.Len() )
                pFilter = aMatch.GetFilter4EA( aType );
        }

        if ( pName || pFilter )
        {
            //Reuse current ole name
            String aDstStgName(String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM(MSO_OLE_Obj)));
            aDstStgName += String::CreateFromInt32(nMSOleObjCntr);

            ::rtl::OUString aFilterName;
            if ( pFilter )
                aFilterName = pFilter->GetName();
            else
                aFilterName = GetFilterNameFromClassID_Impl( aStgNm );

            uno::Sequence < beans::PropertyValue > aMedium( aFilterName.getLength() ? 3 : 2);
            aMedium[0].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "InputStream" ) );
            uno::Reference < io::XInputStream > xStream = new ::utl::OSeekableInputStreamWrapper( *pStream );
            aMedium[0].Value <<= xStream;
            aMedium[1].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "URL" ) );
            aMedium[1].Value <<= ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "private:stream" ) );

            if ( aFilterName.getLength() )
            {
                aMedium[2].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "FilterName" ) );
                aMedium[2].Value <<= aFilterName;
            }

            ::rtl::OUString aName( aDstStgName );
            comphelper::EmbeddedObjectContainer aCnt( rDestStorage );
            xObj = aCnt.InsertEmbeddedObject( aMedium, aName );

            if ( !xObj.is() )
            {
                if( aFilterName.getLength() )
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
            if( !pName && ( sStarName.EqualsAscii( "swriter" ) || sStarName.EqualsAscii( "scalc" ) ) )
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
                //xIPObj->EnableSetModified( FALSE );
                awt::Size aSize;
                aSize.Width = aSz.Width();
                aSize.Height = aSz.Height();
                xObj->setVisualAreaSize( nViewAspect, aSize );
                //xIPObj->EnableSetModified( TRUE );
            }
            else if ( sStarName.EqualsAscii( "smath" ) )
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
                const String& rStorageName,
                SotStorageRef& rSrcStorage,
                const uno::Reference < embed::XStorage >& xDestStorage,
                const Graphic& rGrf,
                const Rectangle& rBoundRect,
                const Rectangle& rVisArea,
                SvStream* pDataStrm,
                ErrCode& rError,
                UINT32 nConvertFlags,
                sal_Int64 nReccomendedAspect )
{
    sal_Int64 nAspect = nReccomendedAspect;
    SdrOle2Obj* pRet = 0;
    if( rSrcStorage.Is() && xDestStorage.is() && rStorageName.Len() )
    {
        comphelper::EmbeddedObjectContainer aCnt( xDestStorage );
        // Ist der 01Ole-Stream ueberhaupt vorhanden ?
        // ( ist er z.B. bei FontWork nicht )
        // Wenn nicht -> Einbindung als Grafik
        BOOL bValidStorage = FALSE;
        String aDstStgName( String::CreateFromAscii(
                                RTL_CONSTASCII_STRINGPARAM(MSO_OLE_Obj)));

        aDstStgName += String::CreateFromInt32( ++nMSOleObjCntr );

        {
            SvStorageRef xObjStg = rSrcStorage->OpenSotStorage( rStorageName,
                                STREAM_READWRITE| STREAM_SHARE_DENYALL );
            if( xObjStg.Is()  )
            {
                {
                    BYTE aTestA[10];	// exist the \1CompObj-Stream ?
                    SvStorageStreamRef xSrcTst = xObjStg->OpenSotStream(
                                String(RTL_CONSTASCII_STRINGPARAM("\1CompObj"),
                                        RTL_TEXTENCODING_MS_1252 ));
                    bValidStorage = xSrcTst.Is() && sizeof( aTestA ) ==
                                    xSrcTst->Read( aTestA, sizeof( aTestA ) );
                    if( !bValidStorage )
                    {
                        // or the \1Ole-Stream ?
                        xSrcTst = xObjStg->OpenSotStream(
                                    String(RTL_CONSTASCII_STRINGPARAM("\1Ole"),
                                            RTL_TEXTENCODING_MS_1252 ));
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
                            String( RTL_CONSTASCII_STRINGPARAM( "\3ObjInfo" ) ),
                            STREAM_STD_READ | STREAM_NOCREATE );
                        if ( xObjInfoSrc.Is() && !xObjInfoSrc->GetError() )
                        {
                            BYTE nByte = 0;
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
                        aObj.SetGraphic( rGrf, ::rtl::OUString() );

                        // TODO/MBA: check setting of PersistName
                        pRet = new SdrOle2Obj( aObj, String(), rBoundRect, false);
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
                    bValidStorage = FALSE;
                }
                else if( !xObjStor.Is() )
                    bValidStorage = FALSE;
            }
        }
        else if( pDataStrm )
        {
            UINT32 nLen, nDummy;
            *pDataStrm >> nLen >> nDummy;
            if( SVSTREAM_OK != pDataStrm->GetError() ||
                // Id in BugDoc - exist there other Ids?
                // The ConvertToOle2 - does not check for consistent
                0x30008 != nDummy )
                bValidStorage = FALSE;
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
                        //xInplaceObj->EnableSetModified( FALSE );
                        xObj->setVisualAreaSize( nAspect, aAwtSz );
                        //xInplaceObj->EnableSetModified( TRUE );*/
                    }
                    catch( uno::Exception& )
                    {
                        OSL_ENSURE( sal_False, "Could not set visual area of the object!\n" );
                    }
                }

                svt::EmbeddedObjectRef aObj( xObj, nAspect );

                // TODO/LATER: need MediaType
                aObj.SetGraphic( rGrf, ::rtl::OUString() );

                pRet = new SdrOle2Obj( aObj, aDstStgName, rBoundRect, false);
            }
        }
    }

    return pRet;
}

SdrObject* SvxMSDffManager::GetAutoForm( MSO_SPT eTyp ) const
{
    SdrObject* pRet = NULL;

    if(120 >= UINT16(eTyp))
    {
        pRet = new SdrRectObj();
    }

    DBG_ASSERT(pRet, "SvxMSDffManager::GetAutoForm -> UNKNOWN AUTOFORM");

    return pRet;
}

sal_Bool SvxMSDffManager::SetPropValue( const uno::Any& rAny, const uno::Reference< ::com::sun::star::beans::XPropertySet > & rXPropSet,
            const String& rPropName, sal_Bool bTestPropertyAvailability )
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
        catch( uno::Exception& )
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
        catch( uno::Exception& )
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
      nXAlign( 0 ),	// position n cm from left
      nXRelTo( 2 ), //   relative to column
      nYAlign( 0 ), // position n cm below
      nYRelTo( 2 ), //   relative to paragraph
      nLayoutInTableCell( 0 ), // element is laid out in table cell
      nTextRotationAngle( 0 ),
      nDxTextLeft( 144 ),
      nDyTextTop( 72 ),
      nDxTextRight(	144 ),
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
      nNextShapeId(	0 ),
      nShapeId( 0 ),
      eShapeType( mso_sptNil )
{
      eLineStyle      = mso_lineSimple; // GPF-Bug #66227#
      bDrawHell       = FALSE;
      bHidden         = FALSE;
//	  bInGroup		  = FALSE;
      bReplaceByFly   = FALSE;
      bLastBoxInChain = TRUE;
      bHasUDefProp    = FALSE; // was the DFF_msofbtUDefProp record set?
      bVFlip = FALSE;
      bHFlip = FALSE;
      bAutoWidth      = FALSE;
}

SvxMSDffImportRec::SvxMSDffImportRec(const SvxMSDffImportRec& rCopy)
    : pObj(	rCopy.pObj ),
      nXAlign( rCopy.nXAlign ),
      nXRelTo( rCopy.nXRelTo ),
      nYAlign( rCopy.nYAlign ),
      nYRelTo( rCopy.nYRelTo ),
      nLayoutInTableCell( rCopy.nLayoutInTableCell ),
      nTextRotationAngle( rCopy.nTextRotationAngle ),
      nDxTextLeft( rCopy.nDxTextLeft	),
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
      eShapeType( rCopy.eShapeType )
{
    eLineStyle       = rCopy.eLineStyle; // GPF-Bug #66227#
    bDrawHell        = rCopy.bDrawHell;
    bHidden          = rCopy.bHidden;
//			bInGroup		 = rCopy.bInGroup;
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
}

/* vi:set tabstop=4 shiftwidth=4 expandtab: */

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
        aIter++;
    }
}

SdrObject* SvxMSDffManager::getShapeForId( sal_Int32 nShapeId )
{
    SvxMSDffShapeIdContainer::iterator aIter( maShapeIdContainer.find(nShapeId) );
    return aIter != maShapeIdContainer.end() ? (*aIter).second : 0;
}
