/*************************************************************************
 *
 *  $RCSfile: msdffimp.cxx,v $
 *
 *  $Revision: 1.22 $
 *
 *  last change: $Author: cmc $ $Date: 2001-03-23 09:14:36 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/


#include <math.h>

#include <limits.h>

#ifndef _SOLAR_H
 #include <tools/solar.h>               // UINTXX
#endif

#pragma hdrstop

// SvxItem-Mapping. Wird benoetigt um die SvxItem-Header erfolgreich zu includen

#ifndef _EDITDATA_HXX
#include <editdata.hxx>
#endif

#include <svtools/urihelper.hxx>

//      textitem.hxx        editdata.hxx
#define ITEMID_COLOR        EE_CHAR_COLOR
#define ITEMID_FONT         EE_CHAR_FONTINFO
#define ITEMID_FONTHEIGHT   EE_CHAR_FONTHEIGHT
#define ITEMID_FONTWIDTH    EE_CHAR_FONTWIDTH
#define ITEMID_WEIGHT       EE_CHAR_WEIGHT
#define ITEMID_UNDERLINE    EE_CHAR_UNDERLINE
#define ITEMID_CROSSEDOUT   EE_CHAR_STRIKEOUT
#define ITEMID_POSTURE      EE_CHAR_ITALIC
#define ITEMID_CONTOUR      EE_CHAR_OUTLINE
#define ITEMID_SHADOWED     EE_CHAR_SHADOW
#define ITEMID_ESCAPEMENT   EE_CHAR_ESCAPEMENT
#define ITEMID_AUTOKERN     EE_CHAR_PAIRKERNING
#define ITEMID_WORDLINEMODE EE_CHAR_WLM
//      paraitem.hxx       editdata.hxx
#define ITEMID_ADJUST      EE_PARA_JUST
#define ITEMID_FIELD       EE_FEATURE_FIELD

#ifndef _STREAM_HXX //autogen
#include <tools/stream.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX //autogen
#include <tools/debug.hxx>
#endif
#ifndef _TOOLS_ZCODEC_HXX
#include <tools/zcodec.hxx>
#endif
#ifndef _UNOTOOLS_UCBSTREAMHELPER_HXX
#include <unotools/ucbstreamhelper.hxx>
#endif
#ifndef _UNOTOOLS_LOCALFILEHELPER_HXX
#include <unotools/localfilehelper.hxx>
#endif
#ifndef _FILTER_HXX //autogen
#include <svtools/filter.hxx>
#endif
#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif
#ifndef _TOOLS_URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _SV_VIRDEV_HXX //autogen
#include <vcl/virdev.hxx>
#endif
#ifndef _SV_BMPACC_HXX
#include <vcl/bmpacc.hxx>
#endif
#ifndef _SVSTOR_HXX //autogen
#include <so3/svstor.hxx>
#endif

#ifndef _SFX_OBJFAC_HXX
#include <sfx2/docfac.hxx>
#endif
#ifndef _SFX_DOCFILT_HACK_HXX
#include <sfx2/docfilt.hxx>
#endif
#ifndef _SFXDOCFILE_HXX
#include <sfx2/docfile.hxx>
#endif
#ifndef _SFX_FCONTNR_HXX
#include <sfx2/fcontnr.hxx>
#endif
#ifndef _SFXMODULE_HXX
#include <sfx2/module.hxx>
#endif


#ifndef _EEITEM_HXX //autogen
#include <eeitem.hxx>
#endif

#ifndef _SDGCPITM_HXX
#ifndef ITEMID_GRF_CROP
#define ITEMID_GRF_CROP 0
#endif
#include <sdgcpitm.hxx>
#endif

#ifndef _SDGMOITM_HXX
#include <sdgmoitm.hxx>
#endif
#ifndef _EEITEMID_HXX
#include "eeitemid.hxx"
#endif
#ifndef _SVX_TSPTITEM_HXX
#include "tstpitem.hxx"
#endif
#ifndef _SVX_FMMODEL_HXX
#include "fmmodel.hxx"
#endif
#ifndef _SVDMODEL_HXX
#include "svdmodel.hxx"
#endif
#ifndef _SVDOBJ_HXX
#include "svdobj.hxx"
#endif
#ifndef _SVDPAGE_HXX
#include "svdpage.hxx"
#endif
#ifndef _SVDOGRP_HXX
#include "svdogrp.hxx"
#endif
#ifndef _SVDOGRAF_HXX
#include "svdograf.hxx"
#endif
#ifndef _SVDOTEXT_HXX
#include "svdotext.hxx"
#endif
#ifndef _SVDORECT_HXX
#include "svdorect.hxx"
#endif
#ifndef _SVDOCAPT_HXX
#include "svdocapt.hxx"
#endif

#ifndef _SVDOEDGE_HXX
#include "svdoedge.hxx"
#endif

#ifndef _SVDOCIRC_HXX
#include "svdocirc.hxx"
#endif
#ifndef _SVDOUTL_HXX
#include "svdoutl.hxx"
#endif
#ifndef _SVDOOLE2_HXX
#include <svdoole2.hxx>
#endif
#ifndef _SVDOPATH_HXX
#include <svdopath.hxx>
#endif

#ifndef _SVDTRANS_HXX
#include <svdtrans.hxx>
#endif

#ifndef _SDGLUITM_HXX
#include <sdgluitm.hxx>
#endif
#ifndef _SVX_FHGTITEM_HXX //autogen
#include <fhgtitem.hxx>
#endif
#ifndef _SVX_WGHTITEM_HXX //autogen
#include <wghtitem.hxx>
#endif
#ifndef _SVX_POSTITEM_HXX //autogen
#include <postitem.hxx>
#endif
#ifndef _SVX_UDLNITEM_HXX //autogen
#include <udlnitem.hxx>
#endif
#ifndef _SVX_CRSDITEM_HXX //autogen
#include <crsditem.hxx>
#endif
#ifndef _SVX_SHDDITEM_HXX //autogen
#include <shdditem.hxx>
#endif
#ifndef _SVX_FONTITEM_HXX //autogen
#include <fontitem.hxx>
#endif
#ifndef _SVX_COLRITEM_HXX //autogen
#include <colritem.hxx>
#endif
#ifndef _SXEKITM_HXX
#include <sxekitm.hxx>
#endif

#ifndef _SVX_BULITEM_HXX
#include <bulitem.hxx>
#endif
#ifndef _E3D_POLYSC3D_HXX
#include "polysc3d.hxx"
#endif
#ifndef _E3D_EXTRUD3D_HXX
#include "extrud3d.hxx"
#endif
#ifndef _SVDITER_HXX
#include "svditer.hxx"
#endif

#ifndef _XPOLY_HXX
#include "xpoly.hxx"
#endif

#ifndef _XATTR_HXX
#include "xattr.hxx"
#endif

#ifndef _IMPGRF_HXX //autogen
#include "impgrf.hxx"
#endif

#define _MSDFFIMP_CXX

#ifndef _MSDFFIMP_HXX
#include "msdffimp.hxx" // extern sichtbare Header-Datei
#endif
#ifndef _MSASHAPE_HXX
#include "msashape.hxx"
#endif
#ifndef _GALLERY_HXX_
#include "gallery.hxx"
#endif

#ifndef _SFXITEMPOOL_HXX
#include <svtools/itempool.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

#ifndef _SVX3DITEMS_HXX
#include <svx3ditems.hxx>
#endif

#ifndef _UCBHELPER_CONTENT_HXX_
#include <ucbhelper/content.hxx>
#endif
#ifndef _UCBHELPER_CONTENTBROKER_HXX_
#include <ucbhelper/contentbroker.hxx>
#endif
#include <vos/xception.hxx>
#ifndef _VOS_NO_NAMESPACE
using namespace vos;
#endif

//---------------------------------------------------------------------------
//  Hilfs Routinen
//---------------------------------------------------------------------------

#define ITEMVALUE(ItemSet,Id,Cast)  ((const Cast&)(ItemSet).Get(Id)).GetValue()

BOOL SvxMSDffManager::ReadINT32( SvStream& rStrm, INT32& rTarget )
{
    rStrm >> rTarget;
    return 0 == rStrm.GetError();
}
BOOL SvxMSDffManager::ReadINT16( SvStream& rStrm, INT16& rTarget )
{
    rStrm >> rTarget;
    return 0 == rStrm.GetError();
}
BOOL SvxMSDffManager::ReadBYTE( SvStream& rStrm, BYTE& rTarget )
{
    rStrm >> rTarget;
    return 0 == rStrm.GetError();
}


//---------------------------------------------------------------------------
//  Hilfs Klassen aus MSDFFDEF.HXX
//---------------------------------------------------------------------------

SvStream& operator>>( SvStream& rIn, DffRecordHeader& rRec )
{
    rRec.nFilePos = rIn.Tell();
    UINT16 nTmp;
    rIn >> nTmp;
    rRec.nImpVerInst = nTmp;
    rRec.nRecVer = nTmp & 0x000F;
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
        {   // clear flags that have to be cleared
            rRec.mpContents[ nRecType ] &= ( ( nContent >> 16 ) ^ 0xffffffff );
            // set flags that have to be set
            rRec.mpContents[ nRecType ] |= nContent;
            nContentEx |= nContent & ( nContent >> 16 );
            rRec.Replace( nRecType, (void*)nContentEx );
        }
        else
        {
            DffPropFlags aPropFlag = { 1, 0, 0, 0 };
            if ( nTmp & 0x4000 )
                aPropFlag.bBlip = TRUE;
            if ( nTmp & 0x8000 )
                aPropFlag.bComplex = TRUE;
            if ( aPropFlag.bComplex )
            {
                if ( nContent )
                {
                    if ( nRecType == DFF_Prop_pVertices )
                    {   // the ContentValue may be 6 bytes too small sometimes
                        UINT32  nOldPos = rIn.Tell();
                        INT16   nSize, nNumElem;
                        rIn.Seek( nComplexDataFilePos );
                        rIn >>  nNumElem >> nSize >> nSize;
                        if ( nSize < 0 )
                            nSize = ( -nSize ) >> 2;
                        if ( (sal_uInt32)( nSize * nNumElem ) == nContent )
                            nContent += 6;
                        rIn.Seek( nOldPos );
                    }
                    nContentEx = nComplexDataFilePos;
                    nComplexDataFilePos += nContent;
                }
                else
                    aPropFlag.bSet = FALSE;
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
    memset( ( (DffPropSet*) this )->mpFlags, 0, 0x400 * sizeof( DffPropFlags ) );
    ( (DffPropSet*) this )->Clear();

    DffPropFlags nFlags = { 1, 0, 0, 1 };

    ( (DffPropSet*) this )->mpContents[ DFF_Prop_LockAgainstGrouping ] = 0x0000;        //0x01ff0000;
    ( (DffPropSet*) this )->mpFlags[ DFF_Prop_LockAgainstGrouping ] = nFlags;
    ( (DffPropSet*) this )->Insert( DFF_Prop_LockAgainstGrouping, (void*)0xffff0000 );

    ( (DffPropSet*) this )->mpContents[ DFF_Prop_FitTextToShape ] = 0x0010;             //0x001f0010;
    ( (DffPropSet*) this )->mpFlags[ DFF_Prop_FitTextToShape ] = nFlags;
    ( (DffPropSet*) this )->Insert( DFF_Prop_FitTextToShape, (void*)0xffff0000 );

    ( (DffPropSet*) this )->mpContents[ DFF_Prop_gtextFStrikethrough ] = 0x0000;        //0xffff0000;
    ( (DffPropSet*) this )->mpFlags[ DFF_Prop_gtextFStrikethrough ] = nFlags;
    ( (DffPropSet*) this )->Insert( DFF_Prop_gtextFStrikethrough, (void*)0xffff0000 );

    ( (DffPropSet*) this )->mpContents[ DFF_Prop_pictureActive ] = 0x0000;              //0x000f0000;
    ( (DffPropSet*) this )->mpFlags[ DFF_Prop_pictureActive ] = nFlags;
    ( (DffPropSet*) this )->Insert( DFF_Prop_pictureActive, (void*)0xffff0000 );

    ( (DffPropSet*) this )->mpContents[ DFF_Prop_fFillOK ] = 0x0039;                    //0x003f0039;
    ( (DffPropSet*) this )->mpFlags[ DFF_Prop_fFillOK ] = nFlags;
    ( (DffPropSet*) this )->Insert( DFF_Prop_fFillOK, (void*)0xffff0000 );

    ( (DffPropSet*) this )->mpContents[ DFF_Prop_fNoFillHitTest ] = 0x001c;             //0x001f001c;
    ( (DffPropSet*) this )->mpFlags[ DFF_Prop_fNoFillHitTest ] = nFlags;
    ( (DffPropSet*) this )->Insert( DFF_Prop_fNoFillHitTest, (void*)0xffff0000 );

    ( (DffPropSet*) this )->mpContents[ DFF_Prop_fNoLineDrawDash ] = 0x001e;            //0x001f000e;
    ( (DffPropSet*) this )->mpFlags[ DFF_Prop_fNoLineDrawDash ] = nFlags;
    ( (DffPropSet*) this )->Insert( DFF_Prop_fNoLineDrawDash, (void*)0xffff0000 );

    ( (DffPropSet*) this )->mpContents[ DFF_Prop_fshadowObscured ] = 0x0000;            //0x00030000;
    ( (DffPropSet*) this )->mpFlags[ DFF_Prop_fshadowObscured ] = nFlags;
    ( (DffPropSet*) this )->Insert( DFF_Prop_fshadowObscured, (void*)0xffff0000 );

    ( (DffPropSet*) this )->mpContents[ DFF_Prop_fPerspective ] = 0x0000;               //0x00010000;
    ( (DffPropSet*) this )->mpFlags[ DFF_Prop_fPerspective ] = nFlags;
    ( (DffPropSet*) this )->Insert( DFF_Prop_fPerspective, (void*)0xffff0000 );

    ( (DffPropSet*) this )->mpContents[ DFF_Prop_fc3DLightFace ] = 0x0001;              //0x000f0001;
    ( (DffPropSet*) this )->mpFlags[ DFF_Prop_fc3DLightFace ] = nFlags;
    ( (DffPropSet*) this )->Insert( DFF_Prop_fc3DLightFace, (void*)0xffff0000 );

    ( (DffPropSet*) this )->mpContents[ DFF_Prop_fc3DFillHarsh ] = 0x0016;              //0x001f0016;
    ( (DffPropSet*) this )->mpFlags[ DFF_Prop_fc3DFillHarsh ] = nFlags;
    ( (DffPropSet*) this )->Insert( DFF_Prop_fc3DFillHarsh, (void*)0xffff0000 );

    ( (DffPropSet*) this )->mpContents[ DFF_Prop_fBackground ] = 0x0000;                //0x001f0000;
    ( (DffPropSet*) this )->mpFlags[ DFF_Prop_fBackground ] = nFlags;
    ( (DffPropSet*) this )->Insert( DFF_Prop_fBackground, (void*)0xffff0000 );

    ( (DffPropSet*) this )->mpContents[ DFF_Prop_fCalloutLengthSpecified ] = 0x0010;    //0x00ef0010;
    ( (DffPropSet*) this )->mpFlags[ DFF_Prop_fCalloutLengthSpecified ] = nFlags;
    ( (DffPropSet*) this )->Insert( DFF_Prop_fCalloutLengthSpecified, (void*)0xffff0000 );

    ( (DffPropSet*) this )->mpContents[ DFF_Prop_fPrint ] = 0x0001;                     //0x00ef0001;
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
        if ( ( nRecType & 0x3f ) == 0x3f )      // this is something called FLAGS
        {
            UINT32 nCurrentFlags = mpContents[ nRecType ];
            UINT32 nMergeFlags = rMaster.mpContents[ nRecType ];
            nMergeFlags &=  ( nMergeFlags >> 16 ) | 0xffff0000;             // clearing low word
            nMergeFlags &= ( ( nCurrentFlags & 0xffff0000 )                 // remove allready hard set
                            | ( nCurrentFlags >> 16 ) ) ^ 0xffffffff;       // attributes from mergeflags
            nCurrentFlags &= ( ( nMergeFlags & 0xffff0000 )                 // apply zero master bits
                            | ( nMergeFlags >> 16 ) ) ^ 0xffffffff;
            nCurrentFlags |= (UINT16)nMergeFlags;                           // apply filled master bits
            ( (DffPropSet*) this )->mpContents[ nRecType ] = nCurrentFlags;
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
    if ( ( nId & 0x3f ) >= 48 ) // is this a flag id
    {
        if ( ((DffPropSet*)this)->Seek( nId | 0x3f ) )
        {
            UINT32 nContentEx = (UINT32)GetCurObject();
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
                UINT32 nOffset = (UINT32)GetCurObject();
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

#ifdef DBG_AUTOSHAPE
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
//  if ( pDefaultPropSet )
//      Merge( *( pDefaultPropSet ) );

    ( (DffPropertyReader*) this )->mnFix16Angle = Fix16ToAngle( GetPropertyValue( DFF_Prop_Rotation, 0 ) );

#ifdef DBG_AUTOSHAPE

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
            for ( sal_uInt32 i = DFF_Prop_adjustValue; i <= DFF_Prop_adjust10Value; i++ )
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
                            INT16   nNumElem, nNumElemMem, nNumSize;
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

void DffPropertyReader::ApplyAttributes( SvStream& rIn, SfxItemSet& rSet, SdrObject* pObj ) const
{
    MapUnit eMap( rManager.GetModel()->GetScaleUnit() );

    for ( void* pDummy = ((DffPropertyReader*)this)->First(); pDummy; pDummy = ((DffPropertyReader*)this)->Next() )
    {
        UINT32 nRecType = GetCurKey();
        UINT32 nContent = mpContents[ nRecType ];
        switch ( nRecType )
        {
            case DFF_Prop_gtextSize :
                rSet.Put( SvxFontHeightItem( rManager.ScalePt( nContent ) ) );
            break;
            // GeoText
            case DFF_Prop_gtextFStrikethrough :
            {
                if ( nContent & 0x20 )
                    rSet.Put( SvxWeightItem( nContent ? WEIGHT_BOLD : WEIGHT_NORMAL ) );
                if ( nContent & 0x10 )
                    rSet.Put( SvxPostureItem( nContent ? ITALIC_NORMAL : ITALIC_NONE ) );
                if ( nContent & 0x08 )
                    rSet.Put( SvxUnderlineItem( nContent ? UNDERLINE_SINGLE : UNDERLINE_NONE ) );
                if ( nContent & 0x40 )
                    rSet.Put(SvxShadowedItem( nContent != 0 ) );
//              if ( nContent & 0x02 )
//                  rSet.Put( SvxCaseMapItem( nContent ? SVX_CASEMAP_KAPITAELCHEN : SVX_CASEMAP_NOT_MAPPED ) );
                if ( nContent & 0x01 )
                    rSet.Put( SvxCrossedOutItem( nContent ? STRIKEOUT_SINGLE : STRIKEOUT_NONE ) );
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
                if ( ( nContent & 0x02 ) == 0 )
                    break;
            }
            case DFF_Prop_fShadow :
            {
                rSet.Put( SdrShadowItem( nContent != 0 ) );
                if ( nContent )
                {
                    if ( !IsProperty( DFF_Prop_shadowOffsetX ) )
                        rSet.Put( SdrShadowXDistItem( 35 ) );
                    if ( !IsProperty( DFF_Prop_shadowOffsetY ) )
                        rSet.Put( SdrShadowYDistItem( 35 ) );
                }
            }
            break;
            // Protection
            case DFF_Prop_LockPosition :
            {
                if ( pObj )
                    pObj->SetMoveProtect( nContent != 0 );
            }
            break;
            case DFF_Prop_LockAgainstSelect :
            {
                if ( pObj )
                    pObj->SetMarkProtect( nContent != 0 );
            }
            break;
        }
    }

    UINT32 nLineFlags = GetPropertyValue( DFF_Prop_fNoLineDrawDash );
    if ( nLineFlags & 8 )
    {   // Linienattribute
        MSO_LineDashing eLineDashing = (MSO_LineDashing)GetPropertyValue( DFF_Prop_lineDashing, mso_lineSolid );
        if ( eLineDashing == mso_lineSolid )
            rSet.Put(XLineStyleItem( XLINE_SOLID ) );
        else
        {
            switch ( eLineDashing )
            {
                case mso_lineDotSys           : rSet.Put(XLineDashItem(String(),XDash(XDASH_RECT,1,LLEN_POINT,0,0,LLEN_SPACE_POINT)));             break;
                case mso_lineDashGEL          : rSet.Put(XLineDashItem(String(),XDash(XDASH_RECT,0,0,1,LLEN_MIDDLE,LLEN_SPACE_MIDDLE)));           break;
                case mso_lineDashDotGEL       : rSet.Put(XLineDashItem(String(),XDash(XDASH_RECT,1,LLEN_POINT,1,LLEN_MIDDLE, LLEN_SPACE_MIDDLE))); break;
                case mso_lineLongDashGEL      : rSet.Put(XLineDashItem(String(),XDash(XDASH_RECT,0,0,1,LLEN_LONG,LLEN_SPACE_LONG)));               break;
                case mso_lineLongDashDotGEL   : rSet.Put(XLineDashItem(String(),XDash(XDASH_RECT,1,LLEN_POINT,1,LLEN_LONG,LLEN_SPACE_LONG)));      break;
                case mso_lineLongDashDotDotGEL: rSet.Put(XLineDashItem(String(),XDash(XDASH_RECT,2,LLEN_POINT,1,LLEN_LONG,LLEN_SPACE_LONG)));      break;
            }
            rSet.Put( XLineStyleItem( XLINE_DASH ) );
        }
        rSet.Put( XLineColorItem( String(), rManager.MSO_CLR_ToColor( GetPropertyValue( DFF_Prop_lineColor ), DFF_Prop_lineColor ) ) );
        if ( IsProperty( DFF_Prop_lineOpacity ) )
            rSet.Put( XLineTransparenceItem( USHORT( 100 - ( ( GetPropertyValue( DFF_Prop_lineOpacity, 0x10000 ) * 100 ) >> 16 ) ) ) );

        INT32 nVal = (INT32)GetPropertyValue( DFF_Prop_lineWidth, 9525 );
        rManager.ScaleEmu( nVal );
        rSet.Put( XLineWidthItem( nVal ) );

        if ( nLineFlags & 0x10 )
        {
            ///////////////
            // LineStart //
            ///////////////
            if ( IsProperty( DFF_Prop_lineStartArrowhead ) )
            {
                MSO_LineEnd aLineEnd = (MSO_LineEnd)GetPropertyValue( DFF_Prop_lineStartArrowhead );
                FASTBOOL bLineCenter = FALSE;
                XPolygon aPoly;
                INT32 nWdt = ( (const XLineWidthItem&)( rSet.Get( XATTR_LINEWIDTH ) ) ).GetValue();
                if ( !nWdt )
                    nWdt = 9;

                switch ( aLineEnd )
                {
                    case mso_lineArrowEnd :
                    {
                        XPolygon aTriangle( 3 );
                        aTriangle[0].X()=1000; aTriangle[0].Y()=   0;
                        aTriangle[1].X()=   0; aTriangle[1].Y()=2000;
                        aTriangle[2].X()=2000; aTriangle[2].Y()=2000;
                        aPoly = aTriangle;
                    }
                    break;
                    case mso_lineArrowOpenEnd :
                    {
                        XPolygon aTriangle( 6 );
                        aTriangle[0].X()=1000; aTriangle[0].Y()=   0;
                        aTriangle[1].X()=   0; aTriangle[1].Y()=2000;
                        aTriangle[2].X()= 400; aTriangle[2].Y()=2000;
                        aTriangle[3].X()=1000; aTriangle[3].Y()= 500;
                        aTriangle[4].X()=2000 - 400; aTriangle[4].Y()=2000;
                        aTriangle[5].X()=2000; aTriangle[5].Y()=2000;
                        aPoly = aTriangle;
                    }
                    break;
                    case mso_lineArrowStealthEnd :
                    {
                        XPolygon aTriangle(4);
                        aTriangle[0].X()=1000; aTriangle[0].Y()=   0;
                        aTriangle[1].X()=   0; aTriangle[1].Y()=2000;
                        aTriangle[2].X()=1000; aTriangle[2].Y()=1500;
                        aTriangle[3].X()=2000; aTriangle[3].Y()=2000;
                        aPoly=aTriangle;
                    }
                    break;
                    case mso_lineArrowDiamondEnd :
                    {
                        XPolygon aTriangle(4);
                        aTriangle[0].X()=1000; aTriangle[0].Y()=   0;
                        aTriangle[1].X()=   0; aTriangle[1].Y()=1000;
                        aTriangle[2].X()=1000; aTriangle[2].Y()=2000;
                        aTriangle[3].X()=2000; aTriangle[3].Y()=1000;
                        aPoly=aTriangle;
                        bLineCenter=TRUE ;

                    }
                    break;
                    case mso_lineArrowOvalEnd :
                    {
                        aPoly = XPolygon( Point( 1000, 0 ), 1000, 1000, 0, 3600 );
                        bLineCenter = TRUE;
                    }
                    break;
                }
                // ggfs. Pfeilende/Start an Writer-Mass anpassen
                double nArrowScaleFactor = ( MAP_TWIP == rManager.GetModel()->GetScaleUnit() ) ? 2.1 : 1.0;
                rSet.Put( XLineStartWidthItem( (INT32)( ( 280 + (INT32)( (double)nWdt * 2.5 ) ) / nArrowScaleFactor ) ) );
                rSet.Put( XLineStartItem( String(
                                    RTL_CONSTASCII_STRINGPARAM( "linestart" ),
                                    RTL_TEXTENCODING_MS_1252 ),
                                    aPoly ) );
                rSet.Put( XLineStartCenterItem( bLineCenter ) );

                MSO_LineEndWidth eWidth = (MSO_LineEndWidth)GetPropertyValue( DFF_Prop_lineStartArrowWidth, mso_lineMediumWidthArrow );
                double fFactor;
                switch( eWidth )
                {
                    case mso_lineNarrowArrow :
                        fFactor = 0.583;
                    break;
                    case mso_lineWideArrow :
                        fFactor = 1.715;
                    break;
                    default:
                    case mso_lineMediumWidthArrow :
                        fFactor = 1.0;
                    break;
                }
                if( 1.0 != fFactor )
                {
                    const SfxPoolItem* pPoolItem=NULL;
                    if ( rSet.GetItemState( XATTR_LINESTART, FALSE, &pPoolItem ) == SFX_ITEM_SET )
                    {
                        XPolygon aNewPoly( ( (const XLineStartItem*)pPoolItem )->GetValue() );
                        aNewPoly.Scale( fFactor, 1.0 );
                        rSet.Put( XLineStartItem( String(), aNewPoly ) );
                    }
                    if ( rSet.GetItemState( XATTR_LINESTARTWIDTH, FALSE, &pPoolItem ) == SFX_ITEM_SET )
                    {
                        INT32 nNewWidth = ( (const XLineStartWidthItem*)pPoolItem )->GetValue();
                        nNewWidth = (INT32)( fFactor * (double)nNewWidth );
                        rSet.Put( XLineStartWidthItem( nNewWidth ) );
                    }
                }
                MSO_LineEndLength eLength = (MSO_LineEndLength)GetPropertyValue( DFF_Prop_lineStartArrowLength, mso_lineMediumLenArrow );
                switch ( eLength )
                {
                    case mso_lineShortArrow :
                        fFactor = 1.715;
                    break;
                    case mso_lineLongArrow :
                        fFactor = 0.583;
                    break;
                    default :
                    case mso_lineMediumLenArrow :
                        fFactor = 1.0;
                    break;
                }
                if ( fFactor != 1.0 )
                {
                    const SfxPoolItem* pPoolItem = NULL;
                    if ( rSet.GetItemState( XATTR_LINESTART, FALSE, &pPoolItem ) == SFX_ITEM_SET )
                    {
                        XPolygon aNewPoly( ( (const XLineStartItem*)pPoolItem )->GetValue() );
                        aNewPoly.Scale( fFactor, 1.0 );
                        rSet.Put( XLineStartItem( String(), aNewPoly ) );
                    }
                }
            }
            /////////////
            // LineEnd //
            /////////////
            if ( IsProperty( DFF_Prop_lineEndArrowhead ) )
            {
                MSO_LineEnd aLineEnd = (MSO_LineEnd)GetPropertyValue( DFF_Prop_lineEndArrowhead );
                FASTBOOL bLineCenter = FALSE;
                XPolygon aPoly;
                INT32 nWdt = ( (const XLineWidthItem&)( rSet.Get( XATTR_LINEWIDTH ) ) ).GetValue();
                if ( !nWdt )
                    nWdt = 9;

                switch ( aLineEnd )
                {
                    case mso_lineArrowEnd :
                    {
                        XPolygon aTriangle( 3 );
                        aTriangle[0].X()=1000; aTriangle[0].Y()=   0;
                        aTriangle[1].X()=   0; aTriangle[1].Y()=2000;
                        aTriangle[2].X()=2000; aTriangle[2].Y()=2000;
                        aPoly = aTriangle;
                    }
                    break;
                    case mso_lineArrowOpenEnd :
                    {
                        XPolygon aTriangle( 6 );
                        aTriangle[0].X()=1000; aTriangle[0].Y()=   0;
                        aTriangle[1].X()=   0; aTriangle[1].Y()=2000;
                        aTriangle[2].X()= 400; aTriangle[2].Y()=2000;
                        aTriangle[3].X()=1000; aTriangle[3].Y()= 500;
                        aTriangle[4].X()=2000 - 400; aTriangle[4].Y()=2000;
                        aTriangle[5].X()=2000; aTriangle[5].Y()=2000;
                        aPoly = aTriangle;
                    }
                    break;
                    case mso_lineArrowStealthEnd :
                    {
                        XPolygon aTriangle(4);
                        aTriangle[0].X()=1000; aTriangle[0].Y()=   0;
                        aTriangle[1].X()=   0; aTriangle[1].Y()=2000;
                        aTriangle[2].X()=1000; aTriangle[2].Y()=1500;
                        aTriangle[3].X()=2000; aTriangle[3].Y()=2000;
                        aPoly=aTriangle;
                    }
                    break;
                    case mso_lineArrowDiamondEnd :
                    {
                        XPolygon aTriangle(4);
                        aTriangle[0].X()=1000; aTriangle[0].Y()=   0;
                        aTriangle[1].X()=   0; aTriangle[1].Y()=1000;
                        aTriangle[2].X()=1000; aTriangle[2].Y()=2000;
                        aTriangle[3].X()=2000; aTriangle[3].Y()=1000;
                        aPoly=aTriangle;
                        bLineCenter=TRUE ;

                    }
                    break;
                    case mso_lineArrowOvalEnd :
                    {
                        aPoly = XPolygon( Point( 1000, 0 ), 1000, 1000, 0, 3600 );
                        bLineCenter = TRUE;
                    }
                    break;
                }
                // ggfs. Pfeilende/Start an Writer-Mass anpassen
                double nArrowScaleFactor = ( MAP_TWIP == rManager.GetModel()->GetScaleUnit() ) ? 2.1 : 1.0;
                rSet.Put( XLineEndWidthItem( (INT32)( ( 280 + (INT32)( (double)nWdt * 2.5 ) ) / nArrowScaleFactor ) ) );
                rSet.Put( XLineEndItem( String(
                                    RTL_CONSTASCII_STRINGPARAM( "lineend" ),
                                    RTL_TEXTENCODING_MS_1252 ),
                                    aPoly ) );
                rSet.Put( XLineEndCenterItem( bLineCenter ) );

                MSO_LineEndWidth eWidth =( MSO_LineEndWidth)GetPropertyValue( DFF_Prop_lineEndArrowWidth, mso_lineMediumWidthArrow );
                double fFactor;
                switch( eWidth )
                {
                    case mso_lineNarrowArrow :
                        fFactor = 0.583;
                    break;
                    case mso_lineWideArrow :
                        fFactor = 1.715;
                    break;
                    default:
                    case mso_lineMediumWidthArrow :
                        fFactor = 1.0;
                    break;
                }
                if( 1.0 != fFactor )
                {
                    const SfxPoolItem* pPoolItem = NULL;
                    if ( rSet.GetItemState( XATTR_LINEEND,FALSE, &pPoolItem ) == SFX_ITEM_SET )
                    {
                        XPolygon aNewPoly( ( (const XLineEndItem*)pPoolItem )->GetValue() );
                        // Basisbreite des Pfeilspizendreiecks veraendern
                        aNewPoly.Scale( fFactor, 1.0 );
                        rSet.Put( XLineEndItem( String(), aNewPoly ) );
                    }
                    if ( rSet.GetItemState( XATTR_LINEENDWIDTH, FALSE, &pPoolItem ) == SFX_ITEM_SET )
                    {
                        INT32 nNewWidth = ( (const XLineEndWidthItem*)pPoolItem )->GetValue();
                        nNewWidth = (INT32)( fFactor * (double)nNewWidth );
                        rSet.Put( XLineEndWidthItem( nNewWidth ) );
                    }
                }
                MSO_LineEndLength eLength = (MSO_LineEndLength)GetPropertyValue( DFF_Prop_lineEndArrowLength, mso_lineMediumLenArrow );
                switch ( eLength )
                {
                    case mso_lineShortArrow :
                        fFactor = 1.715;
                    break;
                    case mso_lineLongArrow :
                        fFactor = 0.583;
                    break;
                    default :
                    case mso_lineMediumLenArrow :
                        fFactor = 1.0;
                    break;
                }
                if( 1.0 != fFactor )
                {
                    const SfxPoolItem* pPoolItem = NULL;
                    if ( rSet.GetItemState( XATTR_LINEEND, FALSE, &pPoolItem ) == SFX_ITEM_SET )
                    {
                        XPolygon aNewPoly( ( (const XLineEndItem*)pPoolItem )->GetValue() );
                        // Schenkellaenge des Pfeilspizendreiecks veraendern
                        aNewPoly.Scale( fFactor, 1.0 );
                        rSet.Put( XLineEndItem( String(), aNewPoly ) );
                    }
                }
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
                    const XDash& rOldDash = ( (const XLineDashItem*)pPoolItem )->GetValue();
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

    if ( GetPropertyValue( DFF_Prop_fNoFillHitTest ) & 0x10 )
    {
        MSO_FillType eMSO_FillType = (MSO_FillType)GetPropertyValue( DFF_Prop_fillType, mso_fillSolid );
        XFillStyle eXFill = XFILL_NONE;
        switch( eMSO_FillType )
        {
            case mso_fillSolid :            // Fill with a solid color
            {
                eXFill = XFILL_SOLID;
                if ( IsProperty( DFF_Prop_fillOpacity ) )
                    rSet.Put( XFillTransparenceItem( sal_uInt16( 100 - ( ( GetPropertyValue( DFF_Prop_fillOpacity ) * 100 ) >> 16 ) ) ) );
            }
            break;
            case mso_fillPattern :          // Fill with a pattern (bitmap)
            case mso_fillTexture :          // A texture (pattern with its own color map)
            case mso_fillPicture :          // Center a picture in the shape
                eXFill = XFILL_BITMAP;
            break;
            case mso_fillShade :            // Shade from start to end points
            case mso_fillShadeCenter :      // Shade from bounding rectangle to end point
            case mso_fillShadeShape :       // Shade from shape outline to end point
            case mso_fillShadeScale :       // Similar to mso_fillShade, but the fillAngle
            case mso_fillShadeTitle :       // special type - shade to title ---  for PP
                eXFill = XFILL_GRADIENT;
            break;
//          case mso_fillBackground :       // Use the background fill color/pattern
        }
        rSet.Put( XFillStyleItem( eXFill ) );

        if ( eXFill == XFILL_GRADIENT )
        {
            long nAngle = 1800;

            // FadeAngle der Fuellung feststellen
            INT32 nFadeAngle = GetPropertyValue( DFF_Prop_fillAngle, 0 );
            if ( nFadeAngle )
                nFadeAngle = 1800 + ( 3600 - ( ( Fix16ToAngle( nFadeAngle ) + 5 ) / 10 ) );
            else
                nFadeAngle = 1800;

            // FadeAngle auf Objektrotation aufschlagen
            if ( nFadeAngle )
                nAngle = nAngle + nFadeAngle;

            // Rotationswinkel in Bereich zwingen
            while ( nAngle >= 3600 )
                nAngle -= 3600;
            while ( nAngle < 0 )
                nAngle += 3600;

            long nFocus = GetPropertyValue( DFF_Prop_fillFocus, 0 );
            XGradientStyle eGrad = XGRAD_LINEAR;
            BOOL bChgColors(FALSE);

            if ( nFadeAngle == 1800 )
                bChgColors = !bChgColors;

            if ( !nFocus )
                bChgColors = !bChgColors;
            else if ( nFocus < 0 )      // Bei negativem Focus sind die Farben zu tauschen
            {
                nFocus =- nFocus;
                bChgColors = !bChgColors;
            }
            if( nFocus > 40 && nFocus < 60 )
            {
                eGrad = XGRAD_AXIAL;    // Besser gehts leider nicht
                bChgColors = !bChgColors;
            }

            USHORT nFocusX = (USHORT)nFocus;
            USHORT nFocusY = (USHORT)nFocus;

            switch( eMSO_FillType )
            {
                case mso_fillShadeShape :
                {
                    eGrad = XGRAD_RECT;
                    nFocusY = nFocusX = 50;
                    bChgColors = !bChgColors;
                }
                break;
                case mso_fillShadeCenter :
                {
                    eGrad = XGRAD_RECT;
                    nFocusX = ( IsProperty( DFF_Prop_fillToRight ) ) ? 100 : 0;
                    nFocusY = ( IsProperty( DFF_Prop_fillToBottom ) ) ? 100 : 0;
                    bChgColors = !bChgColors;
                }
                break;
            }
            Color aCol1( rManager.MSO_CLR_ToColor( GetPropertyValue( DFF_Prop_fillColor, COL_WHITE ), DFF_Prop_fillColor ) );
            Color aCol2( rManager.MSO_CLR_ToColor( GetPropertyValue( DFF_Prop_fillBackColor, COL_WHITE ), DFF_Prop_fillBackColor ) );

            if ( bChgColors )
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
            ULONG nWdt = GetPropertyValue( DFF_Prop_fillWidth, 0 );
            ULONG nHgt = GetPropertyValue( DFF_Prop_fillHeight, 0 );
            if( IsProperty( DFF_Prop_fillBlip ) )
            {
                Graphic aGraf;
                if ( rManager.GetBLIP( GetPropertyValue( DFF_Prop_fillBlip ), aGraf ) )
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
                    else
                    {
                        XOBitmap aXBmp( aBmp, XBITMAP_STRETCH );
                        rSet.Put( XFillBitmapItem( String(), aXBmp ) );
                        if ( eMSO_FillType == mso_fillPicture )
                            rSet.Put( XFillBmpTileItem( FALSE ) );
                    }
                }
            }
        }
    }
    else
        rSet.Put( XFillStyleItem( XFILL_NONE ) );
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
    UINT32 nCurrent = pCList->nCurrent;
    if ( !nCurrent && pCList->pPrev )
    {
        pCList = pCList->pPrev;
        nCurrent = pCList->nCount;
    }
    if ( nCurrent-- )
    {
        pCList->nCurrent = nCurrent;
        pRet = &pCList->mHd[ nCurrent ];
    }
    return pRet;
}

DffRecordHeader* DffRecordManager::Last()
{
    DffRecordHeader* pRet = NULL;
    while ( pCList->pNext )
        pCList = pCList->pNext;
    UINT32 nCount = pCList->nCount;
    if ( nCount-- )
    {
        pCList->nCurrent = nCount;
        pRet = &pCList->mHd[ nCount ];
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

void SvxMSDffManager::Scale( long& rVal ) const
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

void SvxMSDffManager::Scale( XPolygon& rPoly ) const
{
    if ( !bNeedMap )
        return;
    USHORT nPointAnz = rPoly.GetPointCount();
    for ( USHORT nPointNum = 0; nPointNum < nPointAnz; nPointNum++ )
        Scale( rPoly[ nPointNum ] );
}

void SvxMSDffManager::Scale( XPolyPolygon& rPoly ) const
{
    if ( !bNeedMap )
        return;
    USHORT nPolyAnz = rPoly.Count();
    for ( USHORT nPolyNum = 0; nPolyNum < nPolyAnz; nPolyNum++ )
        Scale( rPoly[ nPolyNum ] );
}

void SvxMSDffManager::ScaleEmu( long& rVal ) const
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

BOOL SvxMSDffManager::SeekToShape( SvStream& rSt, void* pClientData, UINT32 nId ) const
{
    BOOL bRet = FALSE;
    if ( mpFidcls )
    {
        UINT32 nMerk = rSt.Tell();
        UINT32 nShapeId, nSec = ( nId >> 10 ) - 1;
        if ( nSec < mnIdClusters )
        {
            UINT32 nOfs = (UINT32)maDgOffsetTable.Get( mpFidcls[ nSec ].dgid );
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
        rSt.Seek( nFPosMerk );  // FilePos restaurieren
    return bRet;
}

FASTBOOL SvxMSDffManager::SeekToRec2( USHORT nRecId1, USHORT nRecId2, ULONG nMaxFilePos, DffRecordHeader* pRecHd, ULONG nSkipCount ) const
{
    FASTBOOL bRet = FALSE;
    ULONG nFPosMerk = rStCtrl.Tell();   // FilePos merken fuer ggf. spaetere Restauration
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


FASTBOOL SvxMSDffManager::GetColorFromPalette( USHORT nNum, Color& rColor ) const
{
    // diese Methode ist in der zum Excel-Import
    // abgeleiteten Klasse zu ueberschreiben...
    rColor.SetColor( COL_WHITE );
    return TRUE;
}


Color SvxMSDffManager::MSO_CLR_ToColor( sal_uInt32 nColorCode, sal_uInt16 nContentProperty ) const
{
    Color aColor( mnDefaultColor );

    // Fuer Textfarben: Header ist 0xfeRRGGBB
    if ( ( nColorCode & 0xfe000000 ) == 0xfe000000 )
        nColorCode &= 0x00ffffff;

    sal_uInt8 nUpper = (sal_uInt8)( nColorCode >> 24 );
    if( nUpper & 0x1b )     // if( nUpper & 0x1f )
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
            UINT16 nParameter = (BYTE)( nColorCode >> 16);
            UINT16 nFunctionBits = (UINT16)( ( nColorCode & 0x00000f00 ) >> 8 );
            UINT16 nAdditionalFlags = (UINT16)( ( nColorCode & 0x0000f000) >> 8 );
            UINT16 nColorIndex = (BYTE)nColorCode;
            UINT32 nPropColor;

            sal_uInt16  nCProp = DFF_Prop_lineColor;
            switch ( nColorIndex )
            {
                case 0xf0 : // msocolorFillColor
                {
                    nPropColor = GetPropertyValue( DFF_Prop_fillColor, 0xffffff );
                    nCProp = DFF_Prop_fillColor;
                }
                break;
                case 0xf1 : // msocolorLineOrFillColor ( use the line color only if there is a line )
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
                case 0xf2 : // msocolorLineColor
                {
                    nPropColor = GetPropertyValue( DFF_Prop_lineColor, 0 );
                    nCProp = DFF_Prop_lineColor;
                }
                break;
                case 0xf3 : // msocolorShadowColor
                {
                    nPropColor = GetPropertyValue( DFF_Prop_shadowColor, 0x808080 );
                    nCProp = DFF_Prop_shadowColor;
                }
                break;
                case 0xf4 : // msocolorThis ( use this color ... )
                {
                    nPropColor = GetPropertyValue( DFF_Prop_fillColor, 0xffffff );  //?????????????
                    nCProp = DFF_Prop_fillColor;
                }
                break;
                case 0xf5 : // msocolorFillBackColor
                {
                    nPropColor = GetPropertyValue( DFF_Prop_fillBackColor, 0xffffff );
                    nCProp = DFF_Prop_fillBackColor;
                }
                break;
                case 0xf6 : // msocolorLineBackColor
                {
                    nPropColor = GetPropertyValue( DFF_Prop_lineBackColor, 0xffffff );
                    nCProp = DFF_Prop_lineBackColor;
                }
                break;
                case 0xf7 : // msocolorFillThenLine ( use the fillcolor unless no fill and line )
                {
                    nPropColor = GetPropertyValue( DFF_Prop_fillColor, 0xffffff );  //?????????????
                    nCProp = DFF_Prop_fillColor;
                }
                break;
                case 0xff : // msocolorIndexMask ( extract the color index )
                {
                    nPropColor = GetPropertyValue( DFF_Prop_fillColor, 0xffffff );  //?????????????
                    nCProp = DFF_Prop_fillColor;
                }
                break;
            }
            if ( ( nPropColor & 0x10000000 ) == 0 ) // beware of looping recursive
                aColor = MSO_CLR_ToColor( nPropColor, nCProp );
            if( nAdditionalFlags & 0x80 )           // make color gray
            {
                UINT8 nZwi = aColor.GetLuminance();
                aColor = Color( nZwi, nZwi, nZwi );
            }
            switch( nFunctionBits )
            {
                case 0x01 :     // darken color by parameter
                {
                    aColor.SetRed( ( nParameter * aColor.GetRed() ) >> 8 );
                    aColor.SetGreen( ( nParameter * aColor.GetGreen() ) >> 8 );
                    aColor.SetBlue( ( nParameter * aColor.GetBlue() ) >> 8 );
                }
                break;
                case 0x02 :     // lighten color by parameter
                {
                    UINT16 nInvParameter = ( 0x00ff - nParameter ) * 0xff;
                    aColor.SetRed( ( nInvParameter + ( nParameter * aColor.GetRed() ) ) >> 8 );
                    aColor.SetGreen( ( nInvParameter + ( nParameter * aColor.GetGreen() ) ) >> 8 );
                    aColor.SetBlue( ( nInvParameter + ( nParameter * aColor.GetBlue() ) ) >> 8 );
                }
                break;
                case 0x03 :     // add grey level RGB(p,p,p)
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
                case 0x04 :     // substract grey level RGB(p,p,p)
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
                case 0x05 :     // substract from grey level RGB(p,p,p)
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
    else    // hart attributiert, eventuell mit Hinweis auf SYSTEMRGB
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
                    rTxt.SetChar( n, ' ' );     // Weicher Umbruch
                // TODO: Zeilenumbruch im Absatz via Outliner setzen.
            }
        }
        aStrHd.SeekToEndOfRecord( rSt );
    }
    else
        aStrHd.SeekToBegOfRecord( rSt );
    return bRet;
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
            UINT32 nInvent=pText->GetObjInventor();
            UINT16 nIdent=pText->GetObjIdentifier();

            SdrOutliner& rOutliner=pText->ImpGetDrawOutliner();
            USHORT nMinDepth = rOutliner.GetMinDepth();
#if SUPD>601
            USHORT nOutlMode = rOutliner.GetMode();
#endif
            { // Wohl 'nen kleiner Bug der EditEngine, das die
              // Absastzattribute bei Clear() nicht entfernt werden.
                FASTBOOL bClearParaAttribs = TRUE;
#if SUPD>601
                rOutliner.SetStyleSheet( 0, NULL );
#endif
                SfxItemSet aSet(rOutliner.GetEmptyItemSet());
                aSet.Put(SvxColorItem( COL_BLACK ));
                rOutliner.SetParaAttribs(0,aSet);
                pText->SetItemSet(aSet);

                bClearParaAttribs = FALSE;
                if( bClearParaAttribs )
                {
                    // Wohl 'nen kleiner Bug der EditEngine, dass die
                    // Absastzattribute bei Clear() nicht entfernt werden.
                    rOutliner.SetParaAttribs(0,rOutliner.GetEmptyItemSet());
                }
            }
#if SUPD>601
            rOutliner.Init( OUTLINERMODE_TEXTOBJECT );
#endif
            rOutliner.SetMinDepth(0);

            ULONG nFilePosMerker=rSt.Tell();
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
                                SvxTabStopItem aTabItem(0, 0);

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
                                UINT16 nObjWidth = pObj->GetSnapRect().GetWidth() + 1;
                                UINT16 nDefaultTabPos = nDefaultTab;

                                while(nDefaultTabPos <= nObjWidth && nDefaultTabPos <= nMostrightTab)
                                    nDefaultTabPos += nDefaultTab;

                                while(nDefaultTabPos <= nObjWidth)
                                {
                                    SvxTabStop aTabStop(nDefaultTabPos);
                                    aTabItem.Insert(aTabStop);
                                    nDefaultTabPos += nDefaultTab;
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
#if SUPD>601
            OutlinerParaObject* pNewText=rOutliner.CreateParaObject();
            rOutliner.Init( nOutlMode );
            rOutliner.SetMinDepth(nMinDepth);
            pText->NbcSetOutlinerParaObject(pNewText);
#endif
        }
        else
            aTextHd.SeekToBegOfRecord(rSt);

    }
    return bRet;
}


void SvxMSDffManager::RetrieveNameOfBLIP(SvStream&      rSt,
                                         String&        rGraphName,
                                         ULONG          nMaxRead)
{
    rGraphName.Erase();
    DffPropSet* pProps = 0;
    ULONG nOldPosData = rSt.Tell();
    DffRecordHeader aRecHd;
    ULONG nRead = 0;
    while( nRead < nMaxRead )
    {
        if( !ReadCommonRecordHeader(aRecHd, rSt) )
        {
            rSt.Seek( aRecHd.nFilePos );
            break;
        }
        else
        {
            switch( aRecHd.nRecType )
            {
                case DFF_msofbtOPT:
                {
                    aRecHd.SeekToBegOfRecord( rSt );
                    pProps = new DffPropSet( TRUE );
                    if ( pProps )
                        rSt >> *pProps;
                }
                break;
            }
            aRecHd.SeekToEndOfRecord( rSt );
            nRead += aRecHd.nRecLen;
        }
    }
    if( pProps )
    {
        MSO_BlipFlags eFlags = (MSO_BlipFlags)pProps->GetPropertyValue( DFF_Prop_pibFlags, mso_blipflagDefault );
        String aFileName;
        if( pProps->SeekToContent( DFF_Prop_pibName, rSt ) )
            MSDFFReadZString( rSt, aFileName,
                        pProps->GetPropertyValue( DFF_Prop_pibName ), TRUE );
        if ((eFlags & mso_blipflagType) == mso_blipflagComment)
            rGraphName = aFileName;
        else
        {
            INetURLObject aURL;
            aURL.SetSmartURL( aFileName );
            rGraphName = aURL.getBase();
        }
        delete pProps;
    }
    rSt.Seek( nOldPosData );
}



void SvxMSDffManager::MSDFFReadZString( SvStream& rIn, String& rStr,
                                    ULONG nRecLen, FASTBOOL bUniCode ) const
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

#ifdef __BIGENDIAN
            for( sal_uInt16 n = 0; n < nLen; ++n, ++pBuf )
                *pBuf = SWAPSHORT( *pBuf );
#endif // ifdef __BIGENDIAN
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


SdrObject* SvxMSDffManager::Import3DObject( SdrObject* pRet, SfxItemSet& aSet, Rectangle& aBoundRect, BOOL bIsAutoText ) const
{
    // pRet Umwandeln in Szene mit 3D Objekt
    E3dDefaultAttributes a3DDefaultAttr;
    a3DDefaultAttr.SetDefaultLatheCharacterMode( TRUE );
    a3DDefaultAttr.SetDefaultExtrudeCharacterMode( TRUE );

    E3dScene* pScene = new E3dPolyScene( a3DDefaultAttr );
    double fDepth = 1270.0;
    BOOL bSceneHasObjects( FALSE );

    if( pRet->IsGroupObject() )
    {
        SdrObjListIter aIter( *pRet, IM_DEEPWITHGROUPS );
        while( aIter.IsMore() )
        {
            SdrObject* pPartObj = aIter.Next();
            SdrObject* pNewObj = pPartObj->ConvertToPolyObj( FALSE, FALSE );
            SdrPathObj* pPath = PTR_CAST( SdrPathObj, pNewObj );

            if( pPath )
            {
                E3dCompoundObject* p3DObj = new E3dExtrudeObj( a3DDefaultAttr, pPath->GetPathPoly(), fDepth );
                p3DObj->NbcSetLayer( pRet->GetLayer() );
                aSet.Put( XLineStyleItem( XLINE_NONE ) );   // ... aber keine Linien
                // Feststellen, ob ein FILL_Attribut gesetzt ist.
                // Falls nicht, Fuellattribut hart setzen
                XFillStyle eFillStyle = ITEMVALUE( aSet, XATTR_FILLSTYLE, XFillStyleItem );
                if ( eFillStyle == XFILL_NONE )
                    aSet.Put( XFillStyleItem( XFILL_SOLID ) );
                pScene->Insert3DObj( p3DObj );
                bSceneHasObjects = TRUE;
            }
            delete pNewObj; // Aufraeumen
        }
    }
    else
    {
        SdrObject* pNewObj = pRet->ConvertToPolyObj( FALSE, FALSE );
        SdrPathObj* pPath = PTR_CAST( SdrPathObj, pNewObj );

        if ( pPath )
        {
            E3dCompoundObject* p3DObj = new E3dExtrudeObj( a3DDefaultAttr, pPath->GetPathPoly(), fDepth );
            p3DObj->NbcSetLayer( pRet->GetLayer() );
            aSet.Put( XLineStyleItem( XLINE_NONE ) );// ... aber keine Linien
            // Feststellen, ob ein FILL_Attribut gesetzt ist.
            // Falls nicht, Fuellattribut hart setzen
            XFillStyle eFillStyle = ITEMVALUE( aSet, XATTR_FILLSTYLE, XFillStyleItem );
            if( eFillStyle == XFILL_NONE )
                aSet.Put( XFillStyleItem( XFILL_SOLID ) );
            pScene->Insert3DObj( p3DObj );
            bSceneHasObjects = TRUE;
        }
        delete pNewObj; // Aufraeumen
    }

    // Hat das Umwandeln geklappt?
    if ( bSceneHasObjects )
    {
        // Returnwert aendern
        delete pRet;
        pRet = pScene;

        // Kameraeinstellungen, Perspektive ...
        Camera3D& rCamera = (Camera3D&)pScene->GetCamera();
        const Volume3D& rVolume = pScene->GetBoundVolume();
        Point aCenter = aBoundRect.Center();
        Matrix4D aMatrix;
        // 3D-Objekt auf die Mitte des Gesamtrechtecks zentrieren
        aMatrix.Translate( Vector3D( -aCenter.X(), aCenter.Y(), 0.0 ) );
        pScene->SetTransform(pScene->GetTransform() * aMatrix);
        pScene->CorrectSceneDimensions();
        pScene->NbcSetSnapRect( aBoundRect );

        // InitScene replacement
        double fW = rVolume.GetWidth();
        double fH = rVolume.GetHeight();
        double fCamZ = rVolume.MaxVec().Z() + ( ( fW + fH ) / 2.0 );

        rCamera.SetAutoAdjustProjection( FALSE );
        rCamera.SetViewWindow( -fW / 2, - fH / 2, fW, fH);
        Vector3D aLookAt( 0.0, 0.0, 0.0 );

        double fCaMX = 3472.0;
        double fCaMY = 3472.0;
        double fCaMZ = fCamZ < 100.0 ? 100.0 : fCamZ;
        sal_Int32 nVal = GetPropertyValue( DFF_Prop_c3DXViewpoint, 0 );
        if ( nVal )
        {
            ScaleEmu( nVal );
            fCaMX = (double)nVal;
        }
        nVal = GetPropertyValue( DFF_Prop_c3DYViewpoint, 0 );
        if ( nVal )
        {
            ScaleEmu(nVal);
            fCaMY = (double)-nVal;
        }
        nVal = GetPropertyValue( DFF_Prop_c3DZViewpoint, 0 );
        if ( nVal )
        {
            ScaleEmu(nVal);
            fCaMZ = (double)nVal;
        }
        Vector3D aCamPos( fCaMX, fCaMY, fCaMZ );
        rCamera.SetPosAndLookAt( aCamPos, aLookAt );
        rCamera.SetFocalLength( 100.0 );
        rCamera.SetDefaults( Vector3D( 0.0, 0.0, 100.0 ), aLookAt, 100.0 );

        // Default: non perspective

        rCamera.SetProjection( GetPropertyValue( DFF_Prop_fc3DFillHarsh, 0 ) & 4 ? PR_PERSPECTIVE : PR_PARALLEL );
        pScene->SetCamera( rCamera );
        pScene->SetItemSet(aSet);

        pScene->SetRectsDirty();
        pScene->InitTransformationSet();

        // Merker fuer Kameraaenderungen

        INT32 nLightKeyX = GetPropertyValue( DFF_Prop_c3DKeyX, 0 );
        INT32 nLightKeyY = GetPropertyValue( DFF_Prop_c3DKeyY, 0 );
        INT32 nLightKeyZ = GetPropertyValue( DFF_Prop_c3DKeyZ, 0 );
        if ( nLightKeyX )
            ScaleEmu( nLightKeyX );
        else
            nLightKeyX = 139;
        if ( nLightKeyY )
            ScaleEmu( nLightKeyY );
        if ( nLightKeyZ )
            ScaleEmu( nLightKeyZ );
        else
            nLightKeyZ = 1;
        Vector3D aLightKey( (double)nLightKeyX, (double)nLightKeyY, (double)nLightKeyZ );

        UINT32 nAmbient = GetPropertyValue( DFF_Prop_c3DAmbientIntensity, 16000 ) / 1000;
        UINT32 nSpecular = GetPropertyValue( DFF_Prop_c3DSpecularAmt, 0 );
        UINT32 nDiffuse = GetPropertyValue( DFF_Prop_c3DDiffuseAmt, 0 );
        INT32 n3dRotate = GetPropertyValue( DFF_Prop_c3DYRotationAngle, 0 );
        if ( n3dRotate )
            pScene->NbcRotateY( (double)Fix16ToAngle( n3dRotate ) * nPi180 );
        n3dRotate = GetPropertyValue( DFF_Prop_c3DXRotationAngle, 0 );
        if ( n3dRotate )
            pScene->NbcRotateX( (double)Fix16ToAngle( n3dRotate ) * nPi180 );
        if ( IsProperty( DFF_Prop_c3DRenderMode ) )
        {
            MSO_3DRenderMode eRenderMode = (MSO_3DRenderMode)GetPropertyValue( DFF_Prop_c3DRenderMode, 0 );
            if ( eRenderMode == mso_Wireframe )
            {
                aSet.Put( XLineStyleItem( XLINE_SOLID ) );  // Linien: Durchgehend
                aSet.Put( XFillStyleItem ( XFILL_NONE ) );  // Flaeche: unsichtbar
                pScene->SetItemSet(aSet);

                // 3D: Doppelseitig
                SdrObjListIter aIter( *pScene, IM_DEEPWITHGROUPS );
                while ( aIter.IsMore() )
                {
                    SdrObject* pSingleObj = aIter.Next();
                    if ( pSingleObj->ISA(E3dExtrudeObj ) )
                    {
                        E3dExtrudeObj* pSingleExtrude = (E3dExtrudeObj*)pSingleObj;
                        pSingleExtrude->SetItem(Svx3DDoubleSidedItem(TRUE));
                    }
                }
            }
        }

        BOOL bUseBackSide = FALSE;
        if ( IsProperty( DFF_Prop_c3DExtrudeBackward ) || IsProperty( DFF_Prop_c3DExtrudeForward ) )
        {
            nVal = GetPropertyValue( DFF_Prop_c3DExtrudeBackward, GetPropertyValue( DFF_Prop_c3DExtrudeForward, 0 ) );
            ScaleEmu( nVal );
            SdrObjListIter aIter( *pScene, IM_DEEPWITHGROUPS );
            while( aIter.IsMore() )
            {
                SdrObject* pSingleObj = aIter.Next();
                if ( pSingleObj->ISA( E3dExtrudeObj ) )
                {
                    E3dExtrudeObj* pSingleExtrude = (E3dExtrudeObj*)pSingleObj;
                    sal_Int32 nSingleExtrudeDepth = pSingleExtrude->GetExtrudeDepth();
                    if( nVal != nSingleExtrudeDepth )
                    {
                        if ( nVal == 338667 )
                        {
                            // MS unendlich
                            pSingleExtrude->SetItem(Svx3DDepthItem(sal_uInt32((fDepth * 18.0)+0.5)));

                            UINT16 nBackScale = (UINT16)((0.1 * 100.0) + 0.5);
                            pSingleExtrude->SetItem(Svx3DBackscaleItem(nBackScale));

                            UINT16 nPercentDiagonal = (UINT16)((0.01 * 200.0) + 0.5);
                            pSingleExtrude->SetItem(Svx3DPercentDiagonalItem(nPercentDiagonal));

                            Matrix4D aMirrorMat;
                            aMirrorMat.Scale( 1.0, -1.0, -1.0 );
                            pSingleExtrude->NbcSetTransform( pSingleExtrude->GetTransform() * aMirrorMat);
                            bUseBackSide = TRUE;
                        }
                        else
                            pSingleExtrude->SetItem(Svx3DDepthItem(sal_uInt32(nVal + 0.5)));
                    }
                }
            }
        }

        // Ausgleichsrotation
        if( (INT32)aCamPos.X() != 0 && (INT32)aCamPos.Y() != 0 )
        {
            // Ausgleichsdrehung notwendig...
            double fArcTan = atan2( rCamera.GetVUV().Y(), rCamera.GetVUV().X() );
            fArcTan = ( nPi / 2.0 ) - fArcTan;
            rCamera.SetBankAngle( fArcTan );
            pScene->SetCamera( rCamera );
        }
        if ( (INT32)aCamPos.X() == 0 && (INT32)aCamPos.Y() == 0 )
        {
            // Ausgleichsrotation fuer plane Flaechen
            pScene->NbcRotateX( 0.5 * nPi180 );
        }

        // Licht
        aLightKey.Normalize();

        // Beleuchtung umdrehen bei AutoText-Objekten
        if ( bIsAutoText )
            aLightKey = -aLightKey;

        pScene->GetLightGroup().SetDirection( aLightKey );
        nAmbient = ( nAmbient - 4 ) << 3;
        Color aAmbientCol( (UINT8)nAmbient, (UINT8)nAmbient, (UINT8)nAmbient );
        pScene->GetLightGroup().SetIntensity( aAmbientCol, Base3DMaterialAmbient );
        pScene->GetLightGroup().SetGlobalAmbientLight( aAmbientCol );
        if ( nSpecular )
        {   // 80% statt 100% (?)
            Color aSpecularCol( 204, 204, 204 );
            pScene->GetLightGroup().SetIntensity( aSpecularCol, Base3DMaterialSpecular );
        }
        if ( nDiffuse )
        {   // 43% von 80% (?)
            Color aDiffuseCol( 45, 45, 45 );
            pScene->GetLightGroup().SetIntensity( aDiffuseCol, Base3DMaterialDiffuse );
        }

        // Positionierung anpassen
        // aBoundRect: 2D-Position des Ursprungsshapes
        Rectangle aFrontPos;
        Rectangle aNewSize;
        Rectangle aOldSize;

        // OldSize ermitteln. NICHT ueber pScene->GetSnapRect, dies liefert
        // nur GetCamera().GetDeviceWindow() !!!
        SdrObjListIter aIter( *pScene, IM_DEEPWITHGROUPS );
        while ( aIter.IsMore() )
        {
            SdrObject* pSingleObj = aIter.Next();
            if ( pSingleObj->ISA( E3dExtrudeObj ) )
            {
                E3dExtrudeObj* pSingleExtrude = (E3dExtrudeObj*)pSingleObj;
                aOldSize.Union( pSingleExtrude->GetSnapRect() );
            }
        }

        // Groesse der 2D-Bildschirmabbildung der Original-Frontpolygone berechnen
        aIter.Reset();
        while ( aIter.IsMore() )
        {
            SdrObject* pSingleObj = aIter.Next();
            if ( pSingleObj->ISA( E3dExtrudeObj ) )
            {
                E3dExtrudeObj* pSingleExtrude = (E3dExtrudeObj*)pSingleObj;
                double fFrontPlaneDepth = (double)pSingleExtrude->GetExtrudeDepth();

                if ( bUseBackSide )
                    fFrontPlaneDepth = 0.0;
                Matrix4D aFullTransMat = pSingleExtrude->GetFullTransform();
                pScene->GetCameraSet().SetObjectTrans( aFullTransMat );

                const PolyPolygon3D& rSourcePolyPoly = pSingleExtrude->GetExtrudePolygon();
                for ( UINT16 a = 0; a < rSourcePolyPoly.Count(); a++ )
                {
                    const Polygon3D& rSourcePoly = rSourcePolyPoly[ a ];
                    for ( UINT16 b = 0; b < rSourcePoly.GetPointCount(); b++ )
                    {
                        Vector3D aPoint = rSourcePoly[ b ];
                        aPoint.Z() = fFrontPlaneDepth;
                        aPoint = pScene->GetCameraSet().ObjectToViewCoor( aPoint );
                        Point aPolyPoint( (long)( aPoint.X() + 0.5 ), (long)( aPoint.Y() + 0.5 ) );
                        aFrontPos.Union( Rectangle( aPolyPoint, aPolyPoint ) );
                    }
                }
            }
        }
        // in aFrontPos steht nun die 2D-Abbildung des Urspruenglichen Objektes
        if ( aFrontPos.Left() )
            aNewSize.Left() = ( aOldSize.Left() * aBoundRect.Left() ) / aFrontPos.Left();
        if ( aFrontPos.Right() )
            aNewSize.Right() = ( aOldSize.Right() * aBoundRect.Right() ) / aFrontPos.Right();
        if ( aFrontPos.Top() )
            aNewSize.Top() = ( aOldSize.Top() * aBoundRect.Top() ) / aFrontPos.Top();
        if ( aFrontPos.Bottom() )
            aNewSize.Bottom() = ( aOldSize.Bottom() * aBoundRect.Bottom() ) / aFrontPos.Bottom();

        // UpperLeft uebertragen
        pScene->NbcSetSnapRect( aNewSize );
        pScene->SetModel( pSdrModel );

//-/        pScene->NbcSetAttributes( aSet, FALSE );
        pScene->SetItemSet(aSet);
    }
    else
        delete pScene;  // Aufraeumen
    return pRet;
}

SdrObject* SvxMSDffManager::ImportWordArt( SvStream& rStCtrl, SfxItemSet& rSet, Rectangle& rBoundRect ) const
{
    SdrObject*  pRet = NULL;
    String      aObjectText;
    String      aFontName;
    INT32       nAngle = mnFix16Angle;
    BOOL        bTextRotate = FALSE;

    ((SvxMSDffManager*)this)->mnFix16Angle = 0; // we don't want to use this property in future
    if ( SeekToContent( DFF_Prop_gtextUNICODE, rStCtrl ) )
        MSDFFReadZString( rStCtrl, aObjectText, GetPropertyValue( DFF_Prop_gtextUNICODE ), TRUE );
    if ( SeekToContent( DFF_Prop_gtextFont, rStCtrl ) )
        MSDFFReadZString( rStCtrl, aFontName, GetPropertyValue( DFF_Prop_gtextFont ), TRUE );
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
    {   // WordArt-Objekt Mit dem Text in aObjectText erzeugen
        SdrObject* pNewObj = new SdrRectObj( OBJ_TEXT, rBoundRect );
        if( pNewObj )
        {
            pNewObj->SetModel( pSdrModel );
            ((SdrRectObj*)pNewObj)->SetText( aObjectText );
            SdrFitToSizeType eFTS = SDRTEXTFIT_PROPORTIONAL;
            rSet.Put( SdrTextFitToSizeTypeItem( eFTS ) );
            rSet.Put( SdrTextAutoGrowHeightItem( FALSE ) );
            rSet.Put( SdrTextAutoGrowWidthItem( FALSE ) );
            rSet.Put( SvxFontItem( FAMILY_DONTKNOW, aFontName, String() ) );
            pNewObj->SetItemSet(rSet);

            pRet = pNewObj->ConvertToPolyObj( FALSE, FALSE );
            if( !pRet )
                pRet = pNewObj;
            else
            {
                pRet->NbcSetSnapRect( rBoundRect );
                delete pNewObj;
            }
            if( nAngle || bTextRotate )
            {
                if ( bTextRotate )
                    nAngle = NormAngle360( nAngle + 9000 );
                if( nAngle )
                {
                    double a = nAngle * nPi180;
                    pRet->NbcRotate( rBoundRect.Center(), nAngle, sin( a ), cos( a ) );
                }
            }
        }
    }
    return pRet;
}

SdrObject* SvxMSDffManager::ImportGraphic( SvStream& rSt, SfxItemSet& rSet, Rectangle& aBoundRect, const DffObjData& rObjData ) const
{
    SdrObject* pRet = NULL;

    Graphic aGraf;
    String aFilename;

    MSO_BlipFlags eFlags = (MSO_BlipFlags)GetPropertyValue( DFF_Prop_pibFlags, mso_blipflagDefault );
    ULONG nBlipId = GetPropertyValue( DFF_Prop_pib, 0 );
    BOOL bGrfRead = FALSE,
    // Grafik verlinkt
    bLinkGrf = 0 != ( eFlags & mso_blipflagLinkToFile );

    if( SeekToContent( DFF_Prop_pibName, rSt ) )
        MSDFFReadZString( rSt, aFilename, GetPropertyValue( DFF_Prop_pibName ), TRUE );

    //   UND, ODER folgendes:
    if( !( eFlags & mso_blipflagDoNotSave ) ) // Grafik embedded
        if (!(bGrfRead = GetBLIP(nBlipId, aGraf)))
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
                        bGrfRead = GetBLIPDirect( rSt, aGraf );
                }
            }
        }

    if ( bGrfRead )
    {   // the writer is doing it's own cropping, so this part affects only impress and calc
        if ( GetSvxMSDffSettings() & SVXMSDFF_SETTINGS_CROP_BITMAPS )
        {
            sal_uInt32 nCropTop     = GetPropertyValue( DFF_Prop_cropFromTop, 0 );
            sal_uInt32 nCropBottom  = GetPropertyValue( DFF_Prop_cropFromBottom, 0 );
            sal_uInt32 nCropLeft    = GetPropertyValue( DFF_Prop_cropFromLeft, 0 );
            sal_uInt32 nCropRight   = GetPropertyValue( DFF_Prop_cropFromRight, 0 );

            if( nCropTop || nCropBottom || nCropLeft || nCropRight )
            {
                double fFactor;
                Size aCropSize( Application::GetDefaultDevice()->LogicToLogic( aGraf.GetPrefSize(), aGraf.GetPrefMapMode(), MAP_100TH_MM ) );
                UINT32 nTop( 0 ), nBottom( 0 ), nLeft( 0 ), nRight( 0 );

                if ( nCropTop )
                {
                    fFactor = (double)nCropTop / 65536.0;
                    nTop = (UINT32)( ( (double)( aCropSize.Height() + 1 ) * fFactor ) + 0.5 );
                }
                if ( nCropBottom )
                {
                    fFactor = (double)nCropBottom / 65536.0;
                    nBottom = (UINT32)( ( (double)( aCropSize.Height() + 1 ) * fFactor ) + 0.5 );
                }
                if ( nCropLeft )
                {
                    fFactor = (double)nCropLeft / 65536.0;
                    nLeft = (UINT32)( ( (double)( aCropSize.Width() + 1 ) * fFactor ) + 0.5 );
                }
                if ( nCropRight )
                {
                    fFactor = (double)nCropRight / 65536.0;
                    nRight = (UINT32)( ( (double)( aCropSize.Width() + 1 ) * fFactor ) + 0.5 );
                }
                rSet.Put( SdrGrafCropItem( nLeft, nTop, nRight, nBottom ) );
            }
        }
        if ( IsProperty( DFF_Prop_pictureTransparent ) )
        {
            UINT32 nTransColor = GetPropertyValue( DFF_Prop_pictureTransparent, 0 );

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

        sal_Int32   nContrast = GetPropertyValue( DFF_Prop_pictureContrast, 65536 );
        if ( nContrast <= 0x10000 )
            nContrast = ( nContrast / 656 ) - 99;
        else if ( nContrast < 0x14835 )
            nContrast = ( nContrast - 0x10000 ) / 800;
        else if ( nContrast < 0x1b961 )
            nContrast = ( nContrast - 0x14835 ) / 1350 + 21;
        else
        {
            nContrast = nContrast / 5000 + 40;
            if ( nContrast > 100 )
                nContrast = 100;
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
                if ( ( GetPropertyValue( DFF_Prop_pictureContrast, 0 ) == 0x4ccd )
                    && ( GetPropertyValue( DFF_Prop_pictureBrightness, 0 ) == 0x599a ) )
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

            // Was: currently the luminance and contrast items are available
            // in impress only
            // Now: available in writer as well, so logically only do
            // hackery for excel import
            if ( !(GetSvxMSDffSettings() & SVXMSDFF_SETTINGS_IMPORT_EXCEL)
                    && ( ( rObjData.nSpFlags & SP_FOLESHAPE ) == 0 ) )
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
                }
            }
        }
    }

    // sollte es ein OLE-Object sein?
    if( bGrfRead && !bLinkGrf && IsProperty( DFF_Prop_pictureId ) )
        pRet = ImportOLE( GetPropertyValue( DFF_Prop_pictureId ), aGraf, aBoundRect );
    if( !pRet )
    {
        pRet = new SdrGrafObj;
        if( bGrfRead )
            ((SdrGrafObj*)pRet)->SetGraphic( aGraf );
#if SUPD>601
        if( bLinkGrf )
        {
            UniString aName( ::URIHelper::SmartRelToAbs( aFilename, FALSE,
                                                                INetURLObject::WAS_ENCODED,
                                                                    INetURLObject::DECODE_UNAMBIGUOUS ) );
            sal_Bool bSetFileName = TRUE;

            if ( bGrfRead )
            {

                // There is still an embedded graphic that could be used. Sometimes
                // a graphiclink is also set. The problem is that the graphic cache will
                // not swapout graphics when a graphiclink exists, so a validity check has to be done

                if ( ( eFlags & mso_blipflagLinkToFile ) == mso_blipflagComment )
                    bSetFileName = FALSE;
                else
                {
                    try
                    {
                        ::ucb::Content  aCnt( aName, ::com::sun::star::uno::Reference<
                            ::com::sun::star::ucb::XCommandEnvironment >() );
                        ::rtl::OUString     aTitle;

                        aCnt.getPropertyValue( ::rtl::OUString::createFromAscii( "Title" ) ) >>= aTitle;
                        bSetFileName = ( aTitle.getLength() > 0 );
                    }
                    catch( ... )
                    {
                        // this file did not exist, so we will not set this as graphiclink
                        bSetFileName = FALSE;
                    }
                }
            }
            if ( bSetFileName )
                ((SdrGrafObj*)pRet)->SetFileName( aName );
        }
#endif
    }
    if ( !pRet->GetName().Len() )                   // SJ 22.02.00 : PPT OLE IMPORT:
    {                                               // name is already set in ImportOLE !!
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
    pRet->SetModel( pSdrModel ); // fuer GraphicLink erforderlich
    pRet->SetLogicRect( aBoundRect );
    return pRet;
}

// PptSlidePersistEntry& rPersistEntry, SdPage* pPage
SdrObject* SvxMSDffManager::ImportObj( SvStream& rSt, void* pClientData,
                                       const Rectangle* pRect,
                                       int nCalledByGroup )
{
    SdrObject* pRet = NULL;
    ULONG nFPosMerk = rSt.Tell(); // FilePos merken fuer spaetere Restauration
    DffRecordHeader aObjHd;
    rSt >> aObjHd;
    ULONG nFPosMerk1 = rSt.Tell();
    ULONG nObjRecEnd = aObjHd.GetRecEndFilePos();
    Rectangle aBoundRect;
    if ( pRect )
        aBoundRect = *pRect;
    if ( aObjHd.nRecType == DFF_msofbtSpgrContainer )
    {
        mnFix16Angle = 0;
        FASTBOOL b1st = TRUE;
        INT32   nGroupRotateAngle = 0;
        INT32   nSpFlags = 0;
        Rectangle aGroupBound( aBoundRect );
        while ( rSt.GetError() == 0 && rSt.Tell() < nObjRecEnd )
        {
            DffRecordHeader aRecHd;
            rSt >> aRecHd;
            if ( aObjHd.nRecType == DFF_msofbtSpContainer ||
                 aObjHd.nRecType == DFF_msofbtSpgrContainer )
            {
                aRecHd.SeekToBegOfRecord( rSt );
                SdrObject* pTmp = ImportObj( rSt, pClientData, &aGroupBound, nCalledByGroup+1);
                if( b1st )
                {   // Gruppenattribute, ...
                    b1st = FALSE;
                    if ( !pTmp )
                        break;
                    pRet = pTmp;
                    aGroupBound = pRet->GetSnapRect();
                    nGroupRotateAngle = mnFix16Angle;
                    nSpFlags = nGroupShapeFlags;
                }
                else if ( pTmp )
                    ((SdrObjGroup*)pRet)->GetSubList()->NbcInsertObject( pTmp );
            }
            aRecHd.SeekToEndOfRecord( rSt );
        }
        if( !aGroupBound.IsEmpty() )
        {
            pRet->NbcSetSnapRect( aGroupBound );
            if ( nGroupRotateAngle )
            {
                double a = nGroupRotateAngle * nPi180;
                pRet->NbcRotate( aGroupBound.Center(), nGroupRotateAngle, sin( a ), cos( a ) );
            }
            if ( nSpFlags & SP_FFLIPV )     // Vertikal gespiegelt?
            {   // BoundRect in aBoundRect
                Point aLeft( aGroupBound.Left(), ( aGroupBound.Top() + aGroupBound.Bottom() ) >> 1 );
                Point aRight( aLeft.X() + 1000, aLeft.Y() );
                pRet->NbcMirror( aLeft, aRight );
            }
            if ( nSpFlags & SP_FFLIPH )     // Horizontal gespiegelt?
            {   // BoundRect in aBoundRect
                Point aTop( ( aGroupBound.Left() + aGroupBound.Right() ) >> 1, aGroupBound.Top() );
                Point aBottom( aTop.X(), aTop.Y() + 1000 );
                pRet->NbcMirror( aTop, aBottom );
            }
        }
    }
    else if ( aObjHd.nRecType == DFF_msofbtSpContainer )
    {
        DffObjData aObjData( aObjHd, aBoundRect, nCalledByGroup, nFPosMerk );
        aObjHd.SeekToBegOfRecord( rSt );
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
            InitializePropSet();    // get the default PropSet
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
        }

        aObjData.bClientAnchor = maShapeRecords.SeekToContent( rSt, DFF_msofbtClientAnchor, SEEK_FROM_CURRENT_AND_RESTART );
        if ( aObjData.bClientAnchor )
            ProcessClientAnchor2( rSt, *maShapeRecords.Current(), pClientData, aObjData );

        if ( aObjData.bChildAnchor )
            aBoundRect = aObjData.aChildAnchor;

        if ( aObjData.nSpFlags & SP_FBACKGROUND )
            aBoundRect = Rectangle( Point(), Size( 1, 1 ) );

        Rectangle aTextRect;
        if ( !aBoundRect.IsEmpty() )
        {   // Rotation auf BoundingBox anwenden, BEVOR ien Objekt generiert wurde
            rSt.Seek( nFPosMerk );
            if( mnFix16Angle )
            {
                long nAngle = mnFix16Angle;
                if ( ( nAngle > 4500 && nAngle <= 13500 ) || ( nAngle > 22500 && nAngle <= 31500 ) )
                {
                    INT32 nHalfWidth = ( aBoundRect.GetWidth() + 1 ) >> 1;
                    INT32 nHalfHeight = ( aBoundRect.GetHeight() + 1 ) >> 1;
                    Point aTopLeft( aBoundRect.Left() + nHalfWidth - nHalfHeight,
                                    aBoundRect.Top() + nHalfHeight - nHalfWidth );
                    Size aNewSize( aBoundRect.GetHeight(), aBoundRect.GetWidth() );
                    Rectangle aNewRect( aTopLeft, aNewSize );
                    aBoundRect = aNewRect;
                }
            }
            aTextRect = aBoundRect;
            FASTBOOL bGraphic = IsProperty( DFF_Prop_pib ) ||
                                IsProperty( DFF_Prop_pibName ) ||
                                IsProperty( DFF_Prop_pibFlags );

            if ( aObjData.nSpFlags & SP_FGROUP )
            {
                pRet = new SdrObjGroup;
                pRet->NbcSetLogicRect( aBoundRect );
                nGroupShapeFlags = aObjData.nSpFlags;       // #73013#
            }
            else if ( ( aObjData.eShapeType != mso_sptNil ) || IsProperty( DFF_Prop_pVertices ) || bGraphic )
            {
                UINT32      nSpecialGroupSettings = 0;
                SfxItemSet  aSet( pSdrModel->GetItemPool() );

                sal_Bool    bIsConnector = ( ( aObjData.eShapeType >= mso_sptStraightConnector1 ) && ( aObjData.eShapeType <= mso_sptCurvedConnector5 ) );
                sal_Bool    bIsAutoShape = FALSE;

                if ( bGraphic )
                {
                    pRet = ImportGraphic( rSt, aSet, aBoundRect, aObjData );
                    if ( pRet )
                    {
                        if ( !IsHardAttribute( DFF_Prop_fFilled ) )     // the default for graphic objects is: not filled
                        {
                            UINT32 nOldProp = GetPropertyValue( DFF_Prop_fNoFillHitTest );
                            if ( nOldProp & 0x10 )
                                SetPropertyValue( DFF_Prop_fNoFillHitTest, nOldProp &~0x10 );
                        }
                        if ( !IsHardAttribute( DFF_Prop_fLine ) )       // the default for graphic objects is: no line
                        {
                            UINT32 nOldProp = GetPropertyValue( DFF_Prop_fNoLineDrawDash );
                            if ( nOldProp & 8 )
                                SetPropertyValue( DFF_Prop_fNoLineDrawDash, nOldProp &~8 );
                        }
                        if ( GetSvxMSDffSettings() & ( SVXMSDFF_SETTINGS_IMPORT_PPT | SVXMSDFF_SETTINGS_IMPORT_EXCEL ) )
                        {   // impress does not support line propertys on graphic objects
                            BOOL bFilled = ( GetPropertyValue( DFF_Prop_fNoFillHitTest ) & 16 ) != 0;
                            BOOL bLine = ( GetPropertyValue( DFF_Prop_fNoLineDrawDash ) & 8 ) != 0;
                            if ( bLine || bFilled )
                            {
                                SdrObject* pRect;
                                SdrObject* pGroup = new SdrObjGroup;
                                if ( pGroup )
                                {
                                    if ( bFilled )
                                    {
                                        pRect = new SdrRectObj( aBoundRect );
                                        if ( pRect )
                                        {
                                            pGroup->GetSubList()->NbcInsertObject( pRect );
                                            nSpecialGroupSettings = 2;
                                        }
                                    }
                                    pGroup->GetSubList()->NbcInsertObject( pRet );
                                    if ( bLine )
                                    {
                                        pRect = new SdrRectObj( aBoundRect );
                                        if ( pRect )
                                            pGroup->GetSubList()->NbcInsertObject( pRect );
                                    }
                                    pRet = pGroup;
                                }
                            }
                        }
                    }
                    aObjData.nSpFlags &=~ ( SP_FFLIPH | SP_FFLIPV );            // #68396#
                }
                else
                {
                    SvxMSDffAutoShape aAutoShape( *this, rSt, aObjData, aBoundRect, mnFix16Angle );
                    if ( !aAutoShape.IsEmpty() )
                    {
                        ApplyAttributes( rSt, aSet, NULL );
                        pRet = aAutoShape.GetObject( pSdrModel, aSet, TRUE );
                        aTextRect = aAutoShape.GetTextRect();
                        bIsAutoShape = TRUE;
                    }
                    else if ( aObjData.eShapeType == mso_sptTextBox )
                    {
                        if ( ( GetPropertyValue( DFF_Prop_fNoLineDrawDash ) & 8 )
                            || ( GetPropertyValue( DFF_Prop_fNoFillHitTest ) & 0x10 ) )
                            pRet = new SdrRectObj( OBJ_TEXT, aBoundRect );
                    }
                    else if (
                        mso_sptWedgeRectCallout == aObjData.eShapeType ||
                        mso_sptWedgeRRectCallout == aObjData.eShapeType ||
                        mso_sptWedgeEllipseCallout == aObjData.eShapeType ||
                        mso_sptBalloon == aObjData.eShapeType ||
                        mso_sptCloudCallout == aObjData.eShapeType )
                    {
                        // Balloon mappen
                        if( mso_sptBalloon == aObjData.eShapeType )
                            aObjData.eShapeType = mso_sptWedgeRRectCallout;

                        pRet = GetAutoForm( aObjData.eShapeType );
                        if ( pRet )
                            pRet->NbcSetSnapRect( aBoundRect ); // Groesse setzen
                    }
                    else if ( ( ( aObjData.eShapeType >= mso_sptCallout1 ) && ( aObjData.eShapeType <= mso_sptAccentBorderCallout3 ) )
                                || ( aObjData.eShapeType == mso_sptCallout90 )
                                || ( aObjData.eShapeType == mso_sptAccentCallout90 )
                                || ( aObjData.eShapeType == mso_sptBorderCallout90 )
                                || ( aObjData.eShapeType == mso_sptAccentBorderCallout90 ) )
                    {
                        pRet = new SdrCaptionObj( aBoundRect );
                        INT32 nAdjust0 = GetPropertyValue( DFF_Prop_adjustValue, 0 );
                        INT32 nAdjust1 = GetPropertyValue( DFF_Prop_adjust2Value, 0 );
                        if( nAdjust0 | nAdjust1 )
                        {   // AdjustValues anwenden, nur welche ?!?
                            nAdjust0 = ( nAdjust0 * 100 ) / 850;
                            nAdjust1 = ( nAdjust1 * 100 ) / 1275;
                            Point aTailPos( nAdjust0 + aBoundRect.Left(), nAdjust1 + aBoundRect.Top() );
                            ((SdrCaptionObj*)pRet)->NbcSetTailPos( aTailPos );
                        }
                    }
                    else if( ( aObjData.eShapeType >= mso_sptTextPlainText ) && ( aObjData.eShapeType <= mso_sptTextCanDown ) ) // WordArt
                    {
                        aObjData.bIsAutoText = TRUE;
                        pRet = ImportWordArt( rSt, aSet, aBoundRect );
                    }
                    else if ( aObjData.eShapeType == mso_sptLine )
                    {
                        pRet = new SdrPathObj( aBoundRect.TopLeft(), aBoundRect.BottomRight() );
                    }
                    else if( bIsConnector )
                    {
                        // Konnektoren
                        MSO_ConnectorStyle eConnectorStyle = (MSO_ConnectorStyle)GetPropertyValue( DFF_Prop_cxstyle, mso_cxstyleStraight );

                        pRet = new SdrEdgeObj();
                        if( pRet )
                        {
                            pRet->SetLogicRect(aBoundRect);

                            ((SdrEdgeObj*)pRet)->ConnectToNode(TRUE, NULL);
                            ((SdrEdgeObj*)pRet)->ConnectToNode(FALSE, NULL);

                            Point aPoint1( aBoundRect.TopLeft() );
                            Point aPoint2( aBoundRect.BottomRight() );

                            // Rotationen beachten
                            if ( mnFix16Angle )
                            {
                                double a = mnFix16Angle * nPi180;
                                Point aCenter( aBoundRect.Center() );
                                double ss = sin(a);
                                double cc = cos(a);

                                RotatePoint(aPoint1, aCenter, ss, cc);
                                RotatePoint(aPoint2, aCenter, ss, cc);
                            }

                            // Linie innerhalb des Bereiches zurechtdrehen/spiegeln
                            if ( aObjData.nSpFlags & SP_FFLIPH )
                            {
                                INT32 n = aPoint1.X();
                                aPoint1.X() = aPoint2.X();
                                aPoint2.X() = n;
                            }
                            if ( aObjData.nSpFlags & SP_FFLIPV )
                            {
                                INT32 n = aPoint1.Y();
                                aPoint1.Y() = aPoint2.Y();
                                aPoint2.Y() = n;
                            }
                            aObjData.nSpFlags &= ~( SP_FFLIPV | SP_FFLIPH );

                            pRet->NbcSetPoint(aPoint1, 0);  // Startpunkt
                            pRet->NbcSetPoint(aPoint2, 1);  // Endpunkt

                            switch( eConnectorStyle )
                            {
                                case mso_cxstyleBent:
                                    aSet.Put( SdrEdgeKindItem( SDREDGE_ORTHOLINES ) );
                                break;
                                case mso_cxstyleCurved:
                                    aSet.Put( SdrEdgeKindItem( SDREDGE_BEZIER ) );
                                break;
                                default: // mso_cxstyleStraight || mso_cxstyleNone
                                    aSet.Put( SdrEdgeKindItem( SDREDGE_ONELINE ) );
                                break;
                            }
                        }
                    }
                    else if ( ( (int)aObjData.eShapeType > (int)mso_sptRectangle ) && ( (int)aObjData.eShapeType < (int)mso_sptTextBox ) )
                    {
                        pRet = GetAutoForm( aObjData.eShapeType );
                        if ( pRet )
                            pRet->NbcSetSnapRect( aBoundRect ); // Groesse setzen
                    }
                }
                if ( pRet )
                {
                    if ( !bIsAutoShape )
                    {
                         ApplyAttributes( rSt, aSet, pRet );
                        if ( !GetPropertyValue( DFF_Prop_gtextSize, 0 ) )
                            aSet.Put( SvxFontHeightItem( ScalePt( 24 << 16 ) ) );
                        if ( aObjData.eShapeType == mso_sptTextBox )
                            aSet.Put( SdrTextMinFrameHeightItem( aBoundRect.GetHeight() ) );
                        pRet->SetModel( pSdrModel );
                        pRet->SetItemSet(aSet);

                        // Rotieren
                        if( mnFix16Angle && !bIsConnector )
                        {
                            double a = mnFix16Angle * nPi180;
                            pRet->NbcRotate( aBoundRect.Center(), mnFix16Angle, sin( a ), cos( a ) );
                        }
                    }
                    // FillStyle != XFILL_NONE und nicht geschlossenes Polygon-Objekt?
                    if( pRet->ISA( SdrPathObj ) )
                    {
                        XFillStyle eFillStyle = ITEMVALUE( aSet, XATTR_FILLSTYLE, XFillStyleItem );
                        if( eFillStyle != XFILL_NONE )
                        {
                            // Das Polygon des Objektes muss geschlossen werden
                            if(!((SdrPathObj*)pRet)->IsClosed())
                                ((SdrPathObj*)pRet)->ToggleClosed(0);
                        }
                    }
                    // Handelt es sich um 3D?
                    if( GetPropertyValue( DFF_Prop_fc3DLightFace ) & 8 )
                    {
                        // #81981# not all objects are effected by 3d effects
                        if ( !bGraphic )
                            pRet = Import3DObject( pRet, aSet, aBoundRect, aObjData.bIsAutoText );
                    }
                }
                if ( pRet )
                {
                    if ( nSpecialGroupSettings )
                    {
                        SdrObjList* pObjectList = pObjectList = pRet->GetSubList();
                        if ( pObjectList )
                        {
                            INT32   nCount = pObjectList->GetObjCount();
                            if ( nSpecialGroupSettings == 2 )
                            {
                                // a graphic was imported into impress, the fill attribute has
                                // to be set on the first object only
                                aSet.Put( XFillStyleItem( XFILL_NONE ) );
                                for ( INT32 i = nCount; --i > 0; )
                                {
                                    SdrObject*  pObj = pObjectList->GetObj( i );
                                    if ( pObj )
                                        pObj->SetItemSet(aSet);
                                }
                            }
                        }
                    }
                    // Horizontal gespiegelt?
                    if ( aObjData.nSpFlags & SP_FFLIPH )
                    {
                        Rectangle aBoundRect( pRet->GetSnapRect() );
                        Point aTop( ( aBoundRect.Left() + aBoundRect.Right() ) >> 1, aBoundRect.Top() );
                        Point aBottom( aTop.X(), aTop.Y() + 1000 );
                        pRet->NbcMirror( aTop, aBottom );
                    }
                    // Vertikal gespiegelt?
                    if ( aObjData.nSpFlags & SP_FFLIPV )
                    {
                        Rectangle aBoundRect( pRet->GetSnapRect() );
                        Point aLeft( aBoundRect.Left(), ( aBoundRect.Top() + aBoundRect.Bottom() ) >> 1 );
                        Point aRight( aLeft.X() + 1000, aLeft.Y() );
                        pRet->NbcMirror( aLeft, aRight );
                    }
                }
            }
        }
        pRet =
            ProcessObj( rSt, aObjData, pClientData, aTextRect, pRet);
    }
    rSt.Seek( nFPosMerk );  // FilePos restaurieren
    return pRet;
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
            UINT32  nLongDummy;
            UINT32* pUDData;
            UINT16  nPID;
            while( 5 < nBytesLeft )
            {
                if( !ReadUINT16(rSt, nPID) )
                    break;
                switch( nPID )
                {
                case 0x038F: pUDData = &pImpRec->nXAlign; break;
                case 0x0390: pUDData = &pImpRec->nXRelTo; break;
                case 0x0391: pUDData = &pImpRec->nYAlign; break;
                case 0x0392: pUDData = &pImpRec->nYRelTo; break;
                default: pUDData = &nLongDummy;
                }
                if( !ReadUINT32(rSt, *pUDData) )
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

            // Distance of Textbox to it's surrounding Autoshape
            INT32 nTextLeft  =GetPropertyValue( DFF_Prop_dxTextLeft, 91440L);
            INT32 nTextRight =GetPropertyValue( DFF_Prop_dxTextRight, 91440L );
            INT32 nTextTop   =GetPropertyValue( DFF_Prop_dyTextTop, 45720L  );
            INT32 nTextBottom=GetPropertyValue( DFF_Prop_dyTextBottom, 45720L );

            ScaleEmu( nTextLeft );
            ScaleEmu( nTextRight );
            ScaleEmu( nTextTop );
            ScaleEmu( nTextBottom );

            // Die vertikalen Absatzeinrueckungen sind im BoundRect mit drin, hier rausrechnen
            rTextRect.Bottom() -= nTextTop + nTextBottom;

            INT32 nTextRotationAngle = 0;
            if ( IsProperty( DFF_Prop_txflTextFlow ) )
            {
                MSO_TextFlow eTextFlow = (MSO_TextFlow)( GetPropertyValue( DFF_Prop_txflTextFlow ) & 0xFFFF );
                switch( eTextFlow )
                {
                    case mso_txflBtoT :                     // Bottom to Top non-@, unten -> oben
                        nTextRotationAngle = 9000;
                    break;
                    case mso_txflTtoBA :    /* #68110# */   // Top to Bottom @-font, oben -> unten
                    case mso_txflTtoBN :                    // Top to Bottom non-@, oben -> unten
                    case mso_txflVertN :                    // Vertical, non-@, oben -> unten
                        nTextRotationAngle = 27000;
                    break;
                    case mso_txflHorzN :                    // Horizontal non-@, normal
                    case mso_txflHorzA :                    // Horizontal @-font, normal
                    default :
                        nTextRotationAngle = 0;
                    break;
                }
                if ( nTextRotationAngle )
                {
                    if ( rObjData.nSpFlags & SP_FFLIPV )
                    {
                        if ( nTextRotationAngle == 9000 )
                            nTextRotationAngle = 27000;
                        else if ( nTextRotationAngle == 27000 )
                            nTextRotationAngle = 9000;
                    }
                    Point nCenter( rTextRect.Center() );
                    long nDX = rTextRect.Right() - rTextRect.Left();
                    long nDY = rTextRect.Bottom() - rTextRect.Top();
                    rTextRect.Left()       = nCenter.X() - nDY/2;
                    rTextRect.Top()        = nCenter.Y() - nDX/2;
                    rTextRect.Right()      = rTextRect.Left() + nDY;
                    rTextRect.Bottom()     = rTextRect.Top()  + nDX;
                }
            }

            FASTBOOL bTextFrame =   (   ( pImpRec->eShapeType
                                            == mso_sptTextSimple )
                                 || ( pImpRec->eShapeType
                                            == mso_sptTextBox    )
                                 || (    (    ( pImpRec->eShapeType
                                                    == mso_sptRectangle      )
                                           || ( pImpRec->eShapeType
                                                    == mso_sptRoundRectangle )
                                         )
                                      && ( eWrapMode == mso_wrapSquare )
                                      && ShapeHasText(pImpRec->nShapeId,
                                                          rObjData.nOldFilePos)
                                    ) );

            if( bTextFrame && pObj )
                pTextObj = PTR_CAST(SdrRectObj, pObj);
            if( !pTextObj )
                pTextObj = new SdrRectObj( OBJ_TEXT, rTextRect );

            if( pTextObj != pObj )
                pTextImpRec = new SvxMSDffImportRec( *pImpRec );

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
                ApplyAttributes( rSt, aSet, pTextObj );

            if ( nTextRotationAngle )
            {
                double a = nTextRotationAngle * nPi180;
                pTextObj->NbcRotate( rTextRect.Center(), nTextRotationAngle, sin( a ), cos( a ) );
            }

            switch ( (MSO_WrapMode)GetPropertyValue( DFF_Prop_WrapText, mso_wrapSquare ) )
            {
                case mso_wrapNone :
                {
                    if ( GetPropertyValue( DFF_Prop_FitTextToShape ) & 2 )  // be sure this is FitShapeToText
                        aSet.Put( SdrTextAutoGrowWidthItem( TRUE ) );
                }
                break;

                case mso_wrapByPoints :
                    aSet.Put( SdrTextContourFrameItem( TRUE ) );
                break;
            }

            // Abstaende an den Raendern der Textbox setzen
            aSet.Put( SdrTextLeftDistItem( nTextLeft ) );
            aSet.Put( SdrTextRightDistItem( nTextRight ) );
            aSet.Put( SdrTextUpperDistItem( nTextTop ) );
            aSet.Put( SdrTextLowerDistItem( nTextBottom ) );
            pTextImpRec->nDxTextLeft    = nTextLeft;
            pTextImpRec->nDyTextTop     = nTextTop;
            pTextImpRec->nDxTextRight   = nTextRight;
            pTextImpRec->nDyTextBottom  = nTextBottom;

            // Textverankerung lesen
            if ( IsProperty( DFF_Prop_anchorText ) )
            {
                MSO_Anchor eTextAnchor = (MSO_Anchor)GetPropertyValue( DFF_Prop_anchorText );

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
                }
                // Einsetzen
                if ( bTVASet )
                    aSet.Put( SdrTextVertAdjustItem( eTVA ) );
                if ( bTHASet )
                    aSet.Put( SdrTextHorzAdjustItem( eTHA ) );
            }
            aSet.Put( SdrTextMinFrameHeightItem( rTextRect.Bottom() - rTextRect.Top() ) );

            pTextObj->SetModel( pSdrModel );
            pTextObj->SetItemSet(aSet);

            // rotate text with shape ?
            if ( mnFix16Angle )
            {
                double a = mnFix16Angle * nPi180;
                pTextObj->NbcRotate( rObjData.rBoundRect.Center(), mnFix16Angle, sin( a ), cos( a ) );
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
                    pOrgObj = pObj;
                    pObj    = pGroup;
                }
            }
        }
        else if( !pObj )
        {
            // simple rectangular objects are ignored by ImportObj()  :-(
            // this is OK for Draw but not for Calc and Writer
            // cause here these objects have a default border
            pObj = new SdrRectObj( rTextRect );
            pOrgObj = pObj;
            pObj->SetModel( pSdrModel );
            SfxItemSet aSet( pSdrModel->GetItemPool() );
            ApplyAttributes( rSt, aSet, pObj );

            const SfxPoolItem* pPoolItem=NULL;
            SfxItemState eState = aSet.GetItemState( XATTR_FILLCOLOR,
                                                     FALSE, &pPoolItem );
            if( SFX_ITEM_DEFAULT == eState )
                aSet.Put( XFillColorItem( String(),
                          Color( mnDefaultColor ) ) );
            pObj->SetItemSet(aSet);
        }

        pImpRec->bDrawHell      = (BOOL)GetPropertyValue( DFF_Prop_fPrint, 0 );
        pTextImpRec->bDrawHell  = pImpRec->bDrawHell;
        pImpRec->nNextShapeId   = GetPropertyValue( DFF_Prop_hspNext, 0 );
        pTextImpRec->nNextShapeId=pImpRec->nNextShapeId;

        if ( nTextId )
        {
            pTextImpRec->aTextId.nTxBxS = (UINT16)( nTextId >> 16 );
            pTextImpRec->aTextId.nSequence = (UINT16)nTextId;
        }

        pTextImpRec->nDxWrapDistLeft = GetPropertyValue(
                                    DFF_Prop_dxWrapDistLeft, 0 ) / 635L;
        pTextImpRec->nDyWrapDistTop = GetPropertyValue(
                                    DFF_Prop_dyWrapDistTop, 0 ) / 635L;
        pTextImpRec->nDxWrapDistRight = GetPropertyValue(
                                    DFF_Prop_dxWrapDistRight, 0 ) / 635L;
        pTextImpRec->nDyWrapDistBottom = GetPropertyValue(
                                    DFF_Prop_dyWrapDistBottom, 0 ) / 635L;
            // 16.16 fraction times total image width or height, as appropriate.

        pImpRec->nCropFromTop = GetPropertyValue(
                                    DFF_Prop_cropFromTop, 0 );
        pImpRec->nCropFromBottom = GetPropertyValue(
                                    DFF_Prop_cropFromBottom, 0 );
        pImpRec->nCropFromLeft = GetPropertyValue(
                                    DFF_Prop_cropFromLeft, 0 );
        pImpRec->nCropFromRight = GetPropertyValue(
                                    DFF_Prop_cropFromRight, 0 );

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

void SvxMSDffManager::StoreShapeOrder(ULONG         nId,
                                      ULONG         nTxBx,
                                      SdrObject*    pObject,
                                      SwFlyFrmFmt*  pFly,
                                      short         nHdFtSection) const
{
    USHORT nShapeCount = pShapeOrders->Count();
    for (USHORT nShapeNum=0; nShapeNum < nShapeCount; nShapeNum++)
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


void SvxMSDffManager::ExchangeInShapeOrder( SdrObject*   pOldObject,
                                            ULONG        nTxBx,
                                            SwFlyFrmFmt* pFly,
                                            SdrObject*   pObject) const
{
    USHORT nShapeCount = pShapeOrders->Count();
    for (USHORT nShapeNum=0; nShapeNum < nShapeCount; nShapeNum++)
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
    USHORT nShapeCount = pShapeOrders->Count();
    for (USHORT nShapeNum=0; nShapeNum < nShapeCount; nShapeNum++)
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

SV_IMPL_PTRARR(         SvxMSDffBLIPInfos,      SvxMSDffBLIPInfo_Ptr    );

SV_IMPL_PTRARR(         SvxMSDffShapeOrders,    SvxMSDffShapeOrder_Ptr  );

SV_IMPL_OP_PTRARR_SORT( SvxMSDffShapeInfos,     SvxMSDffShapeInfo_Ptr   );

SV_IMPL_OP_PTRARR_SORT( SvxMSDffShapeTxBxSort,  SvxMSDffShapeOrder_Ptr  );


// Liste aller SvxMSDffImportRec fuer eine Gruppe
SV_IMPL_OP_PTRARR_SORT(MSDffImportRecords, MSDffImportRec_Ptr)




//---------------------------------------------------------------------------
//  exportierte Klasse: oeffentliche Methoden
//---------------------------------------------------------------------------

SvxMSDffManager::SvxMSDffManager(SvStream& rStCtrl_,
                                 long      nOffsDgg_,
                                 SvStream* pStData_,
                                 SdrModel* pSdrModel_,// s. unten: SetModel()
                                 long      nApplicationScale,
                                 ColorData mnDefaultColor_,
                                 ULONG     nDefaultFontHeight_,
                                 SvStream* pStData2_)
    :DffPropertyReader( *this ),
     pBLIPInfos(   new SvxMSDffBLIPInfos  ),
     pFormModel( NULL ),
     pShapeInfos(  new SvxMSDffShapeInfos ),
     pShapeOrders( new SvxMSDffShapeOrders ),
     pStData(  pStData_  ),
     pStData2( pStData2_ ),
     rStCtrl(  rStCtrl_  ),
     nOffsDgg( nOffsDgg_ ),
     mpFidcls( NULL ),
     mnDefaultColor( mnDefaultColor_),
     nDefaultFontHeight( nDefaultFontHeight_),
     nBLIPCount(  USHRT_MAX ),              // mit Error initialisieren, da wir erst pruefen,
     nShapeCount( USHRT_MAX ),              // ob Kontroll-Stream korrekte Daten enthaellt
     nSvxMSDffSettings( 0 ),
     nSvxMSDffOLEConvFlags( 0 )
{
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

SvxMSDffManager::SvxMSDffManager( SvStream& rStCtrl_ )
    :DffPropertyReader( *this ),
     pBLIPInfos(   new SvxMSDffBLIPInfos  ),
     pFormModel( NULL ),
     pShapeInfos(  new SvxMSDffShapeInfos ),
     pShapeOrders( new SvxMSDffShapeOrders ),
     pStData( 0 ),
     pStData2( 0 ),
     mpFidcls( NULL ),
     rStCtrl(  rStCtrl_  ),
     nOffsDgg( 0 ),
     mnDefaultColor( COL_DEFAULT ),
     nDefaultFontHeight( 24 ),
     nBLIPCount(  USHRT_MAX ),              // mit Error initialisieren, da wir erst pruefen,
     nShapeCount( USHRT_MAX ),              // ob Kontroll-Stream korrekte Daten enthaellt
     nSvxMSDffSettings( 0 ),
     nSvxMSDffOLEConvFlags( 0 )
{
    SetModel( NULL, 0 );
}

SvxMSDffManager::~SvxMSDffManager()
{
    delete pBLIPInfos;
    delete pShapeInfos;
    delete pShapeOrders;
    delete pFormModel;
    delete mpFidcls;
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

void SvxMSDffManager::GetFidclData( long nOffsDgg )
{
    if ( nOffsDgg )
    {
        UINT32 nDummy, nMerk = rStCtrl.Tell();
        rStCtrl.Seek( nOffsDgg );

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
    USHORT nCount            = pOld->Count();
    pShapeInfos              = new SvxMSDffShapeInfos( (nCount < 255)
                                                     ? nCount
                                                     : 255 );
    // altes Info-Array ueberarbeiten
    // (ist sortiert nach nTxBxComp)
    ULONG nChain    = ULONG_MAX;
    USHORT nObjMark = 0;
    BOOL bSetReplaceFALSE = FALSE;
    USHORT nObj;
    for( nObj = 0; nObj < nCount; ++nObj )
    {
        SvxMSDffShapeInfo* pObj = pOld->GetObject( nObj );
        if( pObj->nTxBxComp )
        {
            pObj->bLastBoxInChain = FALSE;
            // Gruppenwechsel ?
            if( nChain != (pObj->nTxBxComp & 0xFFFF0000) )
            {
                // voriger war letzter seiner Gruppe
                if( nObj )
                    pOld->GetObject( nObj-1 )->bLastBoxInChain = TRUE;
                // Merker und Hilfs-Flag zuruecksetzen
                nObjMark = nObj;
                nChain   = pObj->nTxBxComp & 0xFFFF0000;
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
        pShapeInfos->Insert( pObj );
    }
    // voriger war letzter seiner Gruppe
    if( nObj )
        pOld->GetObject( nObj-1 )->bLastBoxInChain = TRUE;
    // urspruengliches Array freigeben, ohne Objekte zu zerstoeren
    pOld->Remove((USHORT)0, nCount);
    delete pOld;
}


/*****************************************************************************

    Einlesen der Shape-Infos im Ctor:
    ---------------------------------
    merken der Shape-Ids und zugehoerigen Blip-Nummern und TextBox-Infos
               =========                  ============     =============
    und merken des File-Offsets fuer jedes Blip
                   ============
******************************************************************************/
void SvxMSDffManager::GetCtrlData( long nOffsDgg_ )
{
    // Start Offset unbedingt merken, falls wir nochmal aufsetzen muessen
    long nOffsDgg = nOffsDgg_;

    // Kontroll Stream positionieren
    rStCtrl.Seek( nOffsDgg );

    BYTE   nVer;
    USHORT nInst;
    USHORT nFbt;
    ULONG  nLength;
    if( !this->ReadCommonRecordHeader( rStCtrl, nVer, nInst, nFbt, nLength ) ) return;

    BOOL bOk;
    ULONG nPos = nOffsDgg + DFF_COMMON_RECORD_HEADER_SIZE;

    // Fall A: erst Drawing Group Container, dann n Mal Drawing Container
    if( DFF_msofbtDggContainer == nFbt )
    {
        GetDrawingGroupContainerData( rStCtrl, nLength );

        nPos += nLength;
        do
        {
            rStCtrl.Seek( nPos );

            bOk =      this->ReadCommonRecordHeader( rStCtrl, nVer, nInst, nFbt, nLength )
                    && ( DFF_msofbtDgContainer == nFbt );

            if( !bOk )
            {
                nPos++;
                rStCtrl.Seek( nPos );
                bOk =      this->ReadCommonRecordHeader( rStCtrl, nVer, nInst, nFbt, nLength )
                        && ( DFF_msofbtDgContainer == nFbt );
            }
            if( bOk )
                GetDrawingContainerData( rStCtrl, nLength );
            nPos += DFF_COMMON_RECORD_HEADER_SIZE + nLength;
        }
        while( bOk );
    }
    else
    {
        ;
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
    ULONG  nLength;

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
        if(!SkipBytes( rSt, nLength))   return;
    }
    while( nRead < nLenDgg );

    if( !nLenBStoreCont ) return;

    // Im BStore Container alle Header der Container und Atome auslesen und die
    // relevanten Daten aller enthaltenen FBSEs in unserem Pointer Array ablegen.
    // Dabei zaehlen wir die gefundenen FBSEs im Member nBLIPCount mit.

    const ULONG nSkipBLIPLen = 20;  // bis zu nBLIPLen zu ueberspringende Bytes
    const ULONG nSkipBLIPPos =  4;  // dahinter bis zu nBLIPPos zu skippen

    ULONG nBLIPLen = 0, nBLIPPos = 0;

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

            if(     bOk )
            {
                bOk =   (          ( SkipBytes( rSt, nSkipBLIPLen) )
                            && ( ReadUINT32(rSt, nBLIPLen    ) )
                        && ( SkipBytes( rSt, nSkipBLIPPos) )
                            && ( ReadUINT32(rSt, nBLIPPos    ) )    );

                nLength -= nSkipBLIPLen+ 4 + nSkipBLIPPos + 4;
                if( 0 > nLength )
                    nLength = 0;
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
        if(!SkipBytes( rSt, nLength))   return;
    }
    while( nRead < nLenBStoreCont );
}


// ab hier: Drawing Container  d.h. Seiten (Blatt, Dia) - weit gueltige Daten
//                      =================               ======
//
void SvxMSDffManager::GetDrawingContainerData( SvStream& rSt, ULONG nLenDg )
{
    BYTE nVer;USHORT nInst;USHORT nFbt;ULONG nLength;

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
            if(!this->GetShapeGroupContainerData( rSt, nLength, TRUE)) return;
        }
        else
        // blanker Shape Container ? (ausserhalb vom Shape Group Container)
        if( DFF_msofbtSpContainer == nFbt )
        {
            if(!this->GetShapeContainerData( rSt, nLength)) return;
        }
        else
        {
            if(!SkipBytes( rSt, nLength)) return;
        }
        nReadDg += nLength;
    }
    while( nReadDg < nLenDg );
}

BOOL SvxMSDffManager::GetShapeGroupContainerData( SvStream& rSt,
                                                  ULONG nLenShapeGroupCont,
                                                  BOOL bPatriarch )
{
    BYTE nVer;USHORT nInst;USHORT nFbt;ULONG nLength;
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
            if ( !this->GetShapeContainerData( rSt, nLength, nGroupOffs ) )
                return FALSE;
            bFirst = FALSE;
        }
        else
        // eingeschachtelter Shape Group Container ?
        if( DFF_msofbtSpgrContainer == nFbt )
        {
            if ( !this->GetShapeGroupContainerData( rSt, nLength, FALSE ) )
                return FALSE;
        }
        else if( !SkipBytes( rSt, nLength ) )
            return FALSE;
        nReadSpGrCont += nLength;
    }
    while( nReadSpGrCont < nLenShapeGroupCont );
    // den Stream wieder korrekt positionieren
    rSt.Seek( nStartShapeGroupCont + nLenShapeGroupCont );
    return TRUE;
}

BOOL SvxMSDffManager::GetShapeContainerData( SvStream& rSt, ULONG nLenShapeCont, ULONG nPosGroup )
{
    BYTE nVer;USHORT nInst;USHORT nFbt;ULONG nLength;
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
    MSO_SPT         eShapeType      = mso_sptNil;
    MSO_WrapMode    eWrapMode       = mso_wrapSquare;
    BOOL            bIsTextBox      = FALSE;

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
            ReadUINT32(rSt, aInfo.nShapeId);
            if ( !SkipBytes( rSt, nLength - 4 ) )
                return FALSE;
            nReadSpCont += nLength;
        }
        else if( DFF_msofbtOPT == nFbt ) // Shape Property Table ?
        {
            // Wir haben die Property Table gefunden:
            // nach der Blip Property suchen!
            ULONG  nPropRead = 0;
            USHORT nPropId;
            ULONG  nPropVal;
            nLenShapePropTbl = nLength;
            UINT32 nPropCount = nInst;
            long nStartShapePropTbl = rSt.Tell();
            UINT32 nComplexDataFilePos = nStartShapePropTbl + ( nPropCount * 6 );
            do
            {
                ReadUINT16(rSt, nPropId);
                ReadUINT32(rSt, nPropVal);
                nPropRead += 6;

                switch( nPropId )
                {
                    case DFF_Prop_txflTextFlow :
                    case DFF_Prop_cdirFont :
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
                        if( 0x4000 == ( nPropId & 0xC000 ) )// Bit gesetzt und gueltig?
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
                         && ( (0x00080008 & nPropVal)  == 0x00080008) ) // also DFF_Prop_f3D
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
        else if( ( DFF_msofbtClientTextbox == nFbt ) && ( 4 == nLength ) )  // Text-Box-Story-Eintrag gefunden
        {
            ReadUINT32(rSt, aInfo.nTxBxComp);
        }
        else
        {
            if ( !SkipBytes( rSt, nLength ) )
                return FALSE;
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
                      && ( eWrapMode == mso_wrapSquare )
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
            //rpShape = ImportObjAtCurrentStreamPos( rStCtrl, rData );
            rpShape = ImportObj( rStCtrl, &rData, &rData.aParentRect );

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
BOOL SvxMSDffManager::GetBLIP(ULONG nIdx_, Graphic& rData) const
{
    BOOL bOk = FALSE;       // Ergebnisvariable initialisieren
    if ( pStData )
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
            bOk = GetBLIPDirect( *pStData, rData );
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
                bOk = GetBLIPDirect( *pStData2, rData );
            // alte FilePos des zweiten Daten-Stream restaurieren
            pStData2->Seek( nOldPosData2 );
        }
        // alte FilePos des/der Stream(s) restaurieren
        rStCtrl.Seek( nOldPosCtrl );
        if( &rStCtrl != pStData )
          pStData->Seek( nOldPosData );
    }
    return bOk;
}

/*      Zugriff auf ein BLIP zur Laufzeit (mit korrekt positioniertem Stream)
    ---------------------------------
******************************************************************************/
BOOL SvxMSDffManager::GetBLIPDirect(SvStream& rBLIPStream, Graphic& rData) const
{
    ULONG nOldPos = rBLIPStream.Tell();

    int nRes = GRFILTER_OPENERROR;  // Fehlervariable initialisieren

    // nachschauen, ob es sich auch wirklich um ein BLIP handelt
    ULONG  nLength;
    USHORT nInst, nFbt;
    BYTE   nVer;
    if( ReadCommonRecordHeader( rBLIPStream, nVer, nInst, nFbt, nLength) && ( 0xF018 <= nFbt ) && ( 0xF117 >= nFbt ) )
    {
        Size        aMtfSize100;
        BOOL        bMtfBLIP = FALSE;
        BOOL        bZCodecCompression = FALSE;
        // Nun exakt auf den Beginn der eingebetteten Grafik positionieren
        ULONG nSkip = ( nInst & 0x0001 ) ? 32 : 16;

        switch( nInst & 0xFFFE )
        {
            case 0x216 :            // Metafile header then compressed WMF
            case 0x3D4 :            // Metafile header then compressed EMF
            case 0x542 :            // Metafile hd. then compressed PICT
            {
                SkipBytes( rBLIPStream, nSkip + 20 );

                // read in size of metafile in EMUS
                rBLIPStream >> aMtfSize100.Width() >> aMtfSize100.Height();

                // scale to 1/100mm
                aMtfSize100.Width() /= 360, aMtfSize100.Height() /= 360;

                // skip rest of header
                nSkip = 6;
                bMtfBLIP = bZCodecCompression = TRUE;
            }
            break;
            case 0x46A :            // One byte tag then JPEG (= JFIF) data
            case 0x6E0 :            // One byte tag then PNG data
            case 0x7A8 :
                nSkip += 1;         // One byte tag then DIB data
            break;
        }
        if( SkipBytes( rBLIPStream, nSkip ) )
        {
            SvStream* pGrStream = &rBLIPStream;
            SvMemoryStream* pOut = 0;
            if( bZCodecCompression )
            {
                pOut = new SvMemoryStream( 0x8000, 0x4000 );
                if ( ZCodecDecompressed( rBLIPStream, *pOut, TRUE ) )
                    pGrStream = pOut;
            }

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

                SvStream* pDbgOut = ::utl::UcbStreamHelper::CreateStream( aURL.GetMainURL(), STREAM_TRUNC | STREAM_WRITE );

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
                            delete pDat;
                        }
                    }

                    delete pDbgOut;
                }
            }
#endif

            if( ( nInst & 0xFFFE ) == 0x7A8 )
            {   // DIBs direkt holen
                Bitmap aNew;
                if( aNew.Read( *pGrStream, FALSE ) )
                {
                    rData = Graphic( aNew );
                    nRes = GRFILTER_OK;
                }
            }
            else
            {   // und unsere feinen Filter darauf loslassen
                GraphicFilter* pGF = GetGrfFilter();
                String aEmptyStr;
                nRes = pGF->ImportGraphic( rData, aEmptyStr, *pGrStream, GRFILTER_FORMAT_DONTKNOW );

                if( bMtfBLIP && ( GRFILTER_OK == nRes ) && ( rData.GetType() == GRAPHIC_GDIMETAFILE ) )
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
            // ggfs. Fehlerstatus zuruecksetzen
            if ( ERRCODE_IO_PENDING == pGrStream->GetError() )
              pGrStream->ResetError();
            delete pOut;
        }
    }
    rBLIPStream.Seek( nOldPos );    // alte FilePos des Streams restaurieren

    return ( GRFILTER_OK == nRes ); // Ergebniss melden
}

/*static*/

#define ZCODEC_DEC_BUFSIZE 0x4000

BOOL SvxMSDffManager::ZCodecDecompressed( SvStream& rIn, SvStream& rOut, BOOL bLookForEnd )
{
    long nOutStartPos = rOut.Tell();
    long nBytesOut = 0;

    ZCodec aZCodec;
    aZCodec.BeginCompression();

    if( bLookForEnd )
    {
        BYTE* pBuf = new BYTE[ ZCODEC_DEC_BUFSIZE ];
        long  nBytesRead;
        while( TRUE )
        {
            nBytesRead = aZCodec.Read( rIn, pBuf, ZCODEC_DEC_BUFSIZE );
            if( nBytesRead > 0 )
            {
                rOut.Write( pBuf, nBytesRead );
                nBytesOut += nBytesRead;
            }
            else
                break;
        }
        if( nBytesRead < 0 ) nBytesOut = 0;     // Error: Ergebnisse ungueltig !!
        delete pBuf;
    }
    else
        nBytesOut = aZCodec.Decompress( rIn, rOut );

    aZCodec.EndCompression();

    rOut.Seek( nOutStartPos );
    return (0 < nBytesOut);
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
                                              ULONG&    rLength )
{
    //----------------------------------------------
    UINT16 aBits;
    if( !ReadUINT16( rSt, aBits ) )
        return FALSE;
    rVer  =  aBits & 0x000F;
    rInst = (aBits & 0xFFF0) >> 4;
    //----------------------------------------------
    if( !ReadUINT16( rSt, rFbt ) )
        return FALSE;
    //----------------------------------------------
    if( 0 != rSt.IsEof() )
        return FALSE;
    return ReadUINT32( rSt, rLength );
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


void SvxMSDffManager::ProcessClientAnchor2( SvStream& rSt, DffRecordHeader& rHd, void* pData, DffObjData& rObj )
{
    return;  // wird von SJ im Draw ueberladen
}


ULONG SvxMSDffManager::Calc_nBLIPPos( ULONG nOrgVal, ULONG nStreamPos ) const
{
    return nOrgVal;
}

BOOL SvxMSDffManager::GetOLEStorageName( long nOLEId, String& ,
                                        SvStorageRef& , SvStorageRef& ) const
{
    return FALSE;
}

BOOL SvxMSDffManager::ShapeHasText( ULONG nShapeId, ULONG nFilePos ) const
{
    return TRUE;
}

SdrObject* SvxMSDffManager::ImportOLE( long nOLEId, const Graphic& rGrf,
                                        const Rectangle& rBoundRect ) const
{
    SdrObject* pRet = 0;
    String sStorageName;
    SvStorageRef xSrcStg, xDstStg;
    if( GetOLEStorageName( nOLEId, sStorageName, xSrcStg, xDstStg ))
        pRet = CreateSdrOLEFromStorage( sStorageName, xSrcStg, xDstStg,
                                        rGrf, rBoundRect, pStData,
                                        nSvxMSDffOLEConvFlags );
    return pRet;
}

const GDIMetaFile* SvxMSDffManager::lcl_GetMetaFileFromGrf_Impl( const Graphic& rGrf,
                                                        GDIMetaFile& rMtf )
{
    const GDIMetaFile* pMtf;
    if( GRAPHIC_BITMAP == rGrf.GetType() )
    {
        Point aPt;
        MapMode aMM( MAP_100TH_MM );

        const Size aSz( OutputDevice::LogicToLogic(
                                rGrf.GetPrefSize(),
                                rGrf.GetPrefMapMode(),
                                aMM ));

        VirtualDevice aVirtDev;
        aVirtDev.EnableOutput( FALSE );
        aVirtDev.SetMapMode( aMM );

        rMtf.Record( &aVirtDev );
        rGrf.Draw( &aVirtDev, aPt, aSz );
        rMtf.Stop();
        rMtf.SetPrefMapMode( aMM );
        rMtf.SetPrefSize( aSz );

        pMtf = &rMtf;
    }
    else
        pMtf = &rGrf.GetGDIMetaFile();
    return pMtf;
}


#ifndef SVX_LIGHT
const SvInPlaceObjectRef SvxMSDffManager::CheckForConvertToSOObj( UINT32 nConvertFlags,
                        SvStorage& rSrcStg, SvStorage& rDestStorage )
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
        { OLE_EXCEL_2_STARCALC, "scalc",
            0x00020820L, 0x0000, 0x0000,
            0xc0,0x00,0x00,0x00,0x00,0x00,0x00,0x46 },
        { OLE_POWERPOINT_2_STARIMPRESS, "simpress",
            0x64818d10L, 0x4f9b, 0x11cf,
            0x86,0xea,0x00,0xaa,0x00,0xb9,0x29,0xe8 },
        { 0,0 }
    };

    SvInPlaceObjectRef xIPObj;
    SvGlobalName aStgNm;
    BOOL bFirstCompare = TRUE;
    for( const _ObjImpType* pArr = aArr; pArr->nFlag; ++pArr )
        if( nConvertFlags & pArr->nFlag )
        {
            SvGlobalName aTypeName( pArr->n1, pArr->n2, pArr->n3,
                            pArr->b8, pArr->b9, pArr->b10, pArr->b11,
                            pArr->b12, pArr->b13, pArr->b14, pArr->b15 );
            if( bFirstCompare )
            {
                aStgNm = rSrcStg.GetClassName();
                bFirstCompare = FALSE;
            }
            if( aStgNm == aTypeName )
            {
                String sStarName( String::CreateFromAscii( pArr->pFactoryNm ));
                const SfxObjectFactory* pFact =
                            SfxObjectFactory::GetFactory( sStarName );
                if( pFact && pFact->GetFilterContainer() )
                {
                    SfxMedium* pMed = new SfxMedium( &rSrcStg, FALSE );
                    const SfxFilter* pFilter = 0;
                    if( !pFact->GetFilterContainer()->
                        GetFilter4Content( *pMed, &pFilter ) && pFilter )
                    {
                        String aEmptyStr;

                        //then the StarFactory can import this storage
                        pMed->SetFilter( pFilter );
                        SvStorageRef xStor = new SvStorage( aEmptyStr);

                        SfxObjectShellLock xDoc( pFact->CreateObject(
                                        SFX_CREATE_MODE_EMBEDDED ));
                        if( xDoc.Is() )
                        {
                            xIPObj = &xDoc;
                            String aDstStgName( String::CreateFromAscii(
                                RTL_CONSTASCII_STRINGPARAM( "MSO_OLE_Obj" )));

                            static sal_uInt32 nMSOleObjCntr = 0;
                            aDstStgName += String::CreateFromInt32( ++nMSOleObjCntr );

                            SvStorageRef xObjStor( rDestStorage.OpenStorage(
                                                    aDstStgName,
                                    STREAM_READWRITE| STREAM_SHARE_DENYALL ));

                            xDoc->DoLoad( pMed );
                            xDoc->DoSaveAs( xObjStor );
                            xDoc->DoSaveCompleted( xObjStor );
                            pMed = 0;
                        }
                    }
                    delete pMed;
                }
                break;
            }
        }
    return xIPObj;
}
#endif

SdrOle2Obj* SvxMSDffManager::CreateSdrOLEFromStorage(
                const String& rStorageName,
                SvStorageRef& rSrcStorage,
                SvStorageRef& rDestStorage,
                const Graphic& rGrf,
                const Rectangle& rBoundRect,
                SvStream* pDataStrm,
                UINT32 nConvertFlags )
{
    SdrOle2Obj* pRet = 0;
#ifndef SVX_LIGHT
    if( rSrcStorage.Is() && rDestStorage.Is() && rStorageName.Len() )
    {
        // Ist der 01Ole-Stream ueberhaupt vorhanden ?
        // ( ist er z.B. bei WordArt nicht )
        // Wenn nicht -> Einbindung als Grafik
        BOOL bValidStorage = FALSE;
        String aDstStgName( String::CreateFromAscii(
                                RTL_CONSTASCII_STRINGPARAM( "MSO_OLE_Obj" )));

        static sal_uInt32 nMSOleObjCntr = 0;
        aDstStgName += String::CreateFromInt32( ++nMSOleObjCntr );
        SvStorageRef xObjStor;

        {
            SvStorageRef xObjStg = rSrcStorage->OpenStorage( rStorageName,
                                STREAM_READWRITE| STREAM_SHARE_DENYALL );
            if( xObjStg.Is()  )
            {
                {
                    BYTE aTestA[10];    // exist the \1CompObj-Stream ?
                    SvStorageStreamRef xSrcTst = xObjStg->OpenStream(
                                String( RTL_CONSTASCII_STRINGPARAM( "\1CompObj" ),
                                        RTL_TEXTENCODING_MS_1252 ));
                    bValidStorage = xSrcTst.Is() && sizeof( aTestA ) ==
                                    xSrcTst->Read( aTestA, sizeof( aTestA ) );
                    if( !bValidStorage )
                    {
                        // or the \1Ole-Stream ?
                        xSrcTst = xObjStg->OpenStream(
                                    String( RTL_CONSTASCII_STRINGPARAM( "\1Ole" ),
                                            RTL_TEXTENCODING_MS_1252 ));
                        bValidStorage = xSrcTst.Is() && sizeof( aTestA ) ==
                                        xSrcTst->Read( aTestA, sizeof( aTestA ) );
                    }
                }

                if( bValidStorage && nConvertFlags )
                {
                    SvInPlaceObjectRef xIPObj( CheckForConvertToSOObj(
                                nConvertFlags, *xObjStg, *rDestStorage ));
                    if( xIPObj.Is() )
                    {
                        pRet = new SdrOle2Obj( xIPObj, String(), rBoundRect,
                                                /*TRUE*/ FALSE );
                        // we have the Object, don't create another
                        bValidStorage = FALSE;
                    }
                }
            }
        }

        if( bValidStorage )
        {
            rSrcStorage->CopyTo( rStorageName, rDestStorage, aDstStgName );

            xObjStor = rDestStorage->OpenStorage( aDstStgName,
                                STREAM_READWRITE| STREAM_SHARE_DENYALL );
            if( xObjStor.Is() && xObjStor->GetError() )
            {
                rDestStorage->SetError( xObjStor->GetError() );
                xObjStor.Clear();
                bValidStorage = FALSE;
            }
            else if( SVSTREAM_OK != rDestStorage->GetError() || !xObjStor.Is() )
                bValidStorage = FALSE;
            else
            {
                GDIMetaFile aMtf;
                SvEmbeddedObject::MakeContentStream( xObjStor,
                            *lcl_GetMetaFileFromGrf_Impl( rGrf, aMtf ) );
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
                xObjStor = rDestStorage->OpenStorage( aDstStgName
                                /*, STREAM_READWRITE | STREAM_SHARE_DENYALL*/ );

                GDIMetaFile aMtf;
                SotStorageRef xRef( &xObjStor );
                bValidStorage = SvEmbeddedObject::ConvertToOle2(
                            *pDataStrm, nLen,
                            lcl_GetMetaFileFromGrf_Impl( rGrf, aMtf ), xRef );
            }
        }

        if( bValidStorage )
        {
            SvInPlaceObjectRef xInplaceObj( ((SvFactory*)SvInPlaceObject::
                                    ClassFactory())->CreateAndLoad( xObjStor ) );
            if( xInplaceObj.Is() )
            {
                // VisArea am OutplaceObject setzen!!
                Size aSz( OutputDevice::LogicToLogic( rGrf.GetPrefSize(),
                    rGrf.GetPrefMapMode(), MapMode( xInplaceObj->GetMapUnit() ) ) );
                // modifiziert wollen wir nicht werden
                xInplaceObj->EnableSetModified( FALSE );
                xInplaceObj->SetVisArea( Rectangle( Point(), aSz ));
                xInplaceObj->EnableSetModified( TRUE );

                XubString aNm;
                pRet = new SdrOle2Obj( xInplaceObj, aNm,
                                        rBoundRect, /*TRUE*/ FALSE );
            }
        }
    }
#endif
    return pRet;
}

SdrObject* SvxMSDffManager::GetAutoForm( MSO_SPT eTyp ) const
{
    UINT16 nNewType = 1; // Rectangle als default
    SdrObject* pRet = NULL;
    switch ( eTyp )
    {
        case mso_sptCurvedLeftArrow           : nNewType= 53; break;
        case mso_sptCurvedRightArrow          : nNewType= 52; break;
        case mso_sptCurvedUpArrow             : nNewType= 54; break;
        case mso_sptCurvedDownArrow           : nNewType= 55; break;
        case mso_sptWave                      : nNewType= 89; break;
        case mso_sptDoubleWave                : nNewType= 90; break;

        case mso_sptWedgeRectCallout          : nNewType= 98; break;
        case mso_sptWedgeRRectCallout         : nNewType= 99; break;
        case mso_sptWedgeEllipseCallout       : nNewType=100; break;
        case mso_sptCloudCallout              : nNewType=101; break;
    }
    // Model holen falls noch nicht angelegt
    if( !pFormModel )
    {
        if ( GalleryExplorer::GetSdrObjCount( GALLERY_THEME_POWERPOINT ) )
        {
            ((SvxMSDffManager*)this)->pFormModel = new FmFormModel();
            SfxItemPool& rPool = pFormModel->GetItemPool();
            rPool.FreezeIdRanges();
            if ( !GalleryExplorer::GetSdrObj( GALLERY_THEME_POWERPOINT, 0, pFormModel ) && pFormModel ) // Objekte sind nicht da
                delete ((SvxMSDffManager*)this)->pFormModel, ((SvxMSDffManager*)this)->pFormModel = NULL;
        }
    }
    if( pFormModel && pFormModel->GetPageCount() )  // Objekt aus dem model holen, falls es  noch klappt
    {
        const SdrObject* pObj = pFormModel->GetPage( 0 )->GetObj( 120 - nNewType );
        if( pObj )
            pRet = (SdrObject*)pObj->Clone();
    }
    return pRet;
}

