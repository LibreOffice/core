/*************************************************************************
 *
 *  $RCSfile: svdfppt.cxx,v $
 *
 *  $Revision: 1.110 $
 *
 *  last change: $Author: hr $ $Date: 2003-08-07 15:25:33 $
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

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

#include <math.h>

#ifndef _EEITEM_HXX //autogen
#include <eeitem.hxx>
#endif
#ifndef _SVSTOR_HXX //autogen
#include <so3/svstor.hxx>
#endif
#ifndef _SOT_STORINFO_HXX
#include <sot/storinfo.hxx>
#endif
#ifndef _STG_HXX
#include <sot/stg.hxx>
#endif

#include "svdfppt.hxx"
#include "xpoly.hxx"
#include "svdtrans.hxx"
#include "svdmodel.hxx"
#include "svdpage.hxx"
#include "svdobj.hxx"
#include "svdogrp.hxx"
#include "svdorect.hxx"
#include "svdopage.hxx"
#include "svdograf.hxx"
#include "svdopath.hxx"
#include "svdocirc.hxx"
#include "svdocapt.hxx"
#include "svdattr.hxx"
#include "xattr.hxx"
#include "svditext.hxx"
#include "svdetc.hxx"
#include <bulitem.hxx>
#include "polysc3d.hxx"
#include "extrud3d.hxx"

#ifndef _EEITEMID_HXX
#include "eeitemid.hxx"
#endif
#ifndef _SVX_TSPTITEM_HXX
#include "tstpitem.hxx"
#endif
#if defined(JOEENV) && defined(JOEDEBUG)
#include "impinccv.h" // etwas Testkram
#endif

#if defined(DBG_EXTRACTOLEOBJECTS) || defined(DBG_EXTRACTFONTMETRICS)
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _UNOTOOLS_LOCALFILEHELPER_HXX
#include <unotools/localfilehelper.hxx>
#endif
#endif

#define ITEMVALUE(ItemSet,Id,Cast)  ((const Cast&)(ItemSet).Get(Id)).GetValue()
#ifndef _SVX_ADJITEM_HXX //autogen
#include <adjitem.hxx>
#endif
#ifndef _SVX_ESCPITEM_HXX //autogen
#include <escpitem.hxx>
#endif
#ifndef _SVX_COLRITEM_HXX //autogen
#include <colritem.hxx>
#endif
#ifndef _SVSTOR_HXX //autogen
#include <svstor.hxx>
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
#ifndef _SVX_CHARRELIEFITEM_HXX
#include <charreliefitem.hxx>
#endif
#ifndef _SVX_FONTITEM_HXX
#include <fontitem.hxx>
#endif
#ifndef _SVDOUTL_HXX
#include <svdoutl.hxx>
#endif
#ifndef _EDITENG_HXX
#include <editeng.hxx>
#endif
#ifndef _EEITEMID_HXX
#include <eeitemid.hxx>
#endif
#ifndef _SVX_LSPCITEM_HXX //autogen
#include <lspcitem.hxx>
#endif
#ifndef _SVX_ULSPITEM_HXX //autogen
#include <ulspitem.hxx>
#endif
#ifndef _SVX_LRSPITEM_HXX
#include <lrspitem.hxx>
#endif
#ifndef _SV_METRIC_HXX //autogen
#include <vcl/metric.hxx>
#endif
#ifndef _SV_BMPACC_HXX //autogen
#include <vcl/bmpacc.hxx>
#endif
#ifndef _SVDITER_HXX
#include <svditer.hxx>
#endif
#ifndef _SVDOEDGE_HXX
#include <svdoedge.hxx>
#endif
#ifndef _SXEKITM_HXX
#include <sxekitm.hxx>
#endif
#ifndef _SVX_FLDITEM_HXX
#include <flditem.hxx>
#endif
#ifndef _SYCHCONV_HXX
#include <svtools/sychconv.hxx>
#endif
#ifndef _ZCODEC_HXX
#include <tools/zcodec.hxx>
#endif
#ifndef _SVXMSBAS_HXX
#include <svxmsbas.hxx>
#endif
#ifndef _SFX_OBJSH_HXX
#include <sfx2/objsh.hxx>
#endif
#ifndef _SVX_BRSHITEM_HXX
#include <brshitem.hxx>
#endif
#ifndef _SVX_LANGITEM_HXX
#include <langitem.hxx>
#endif
#ifndef _SVDOOLE2_HXX
#include <svdoole2.hxx>
#endif
#ifndef _SVX_UNOAPI_HXX_
#include <unoapi.hxx>
#endif
#ifndef _TOOLKIT_UNOHLP_HXX
#include <toolkit/unohlp.hxx>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XINDEXCONTAINER_HPP_
#include <com/sun/star/container/XIndexContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XSHAPES_HPP_
#include <com/sun/star/drawing/XShapes.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XCONTROLSHAPE_HPP_
#include <com/sun/star/drawing/XControlShape.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XFORMCOMPONENT_HPP_
#include <com/sun/star/form/XFormComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XDRAWPAGESSUPPLIER_HPP_
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XMASTERPAGESSUPPLIER_HPP_
#include <com/sun/star/drawing/XMasterPagesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XDRAWPAGESUPPLIER_HPP_
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_SIZE_HPP_
#include <com/sun/star/awt/Size.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_POINT_HPP_
#include <com/sun/star/awt/Point.hpp>
#endif
#ifndef _SVX_WRITINGMODEITEM_HXX
#include "writingmodeitem.hxx"
#endif
#ifndef _SV_PRINT_HXX
#include <vcl/print.hxx>
#endif
#ifndef _SVX_SVXFONT_HXX
#include <svxfont.hxx>
#endif
#ifndef _SVX_FRMDIRITEM_HXX
#include <frmdiritem.hxx>
#endif
#ifndef _UNTOOLS_UCBSTREAMHELPER_HXX
#include <unotools/ucbstreamhelper.hxx>
#endif
#ifndef _SVX_SCRIPTTYPEITEM_HXX
#include <scripttypeitem.hxx>
#endif
#ifndef _EMBOBJ_HXX
#include <so3/embobj.hxx>
#endif
#ifndef _IPOBJ_HXX
#include <so3/ipobj.hxx>
#endif
#include <vcl/virdev.hxx>
#include <algorithm>
#pragma hdrstop

////////////////////////////////////////////////////////////////////////////////////////////////////

// PPT ColorScheme Slots
#define PPT_COLSCHEME                       (0x08000000)
#define PPT_COLSCHEME_HINTERGRUND           (0x08000000)
#define PPT_COLSCHEME_TEXT_UND_ZEILEN       (0x08000001)
#define PPT_COLSCHEME_SCHATTEN              (0x08000002)
#define PPT_COLSCHEME_TITELTEXT             (0x08000003)
#define PPT_COLSCHEME_FUELLBEREICHE         (0x08000004)
#define PPT_COLSCHEME_AKZENT                (0x08000005)
#define PPT_COLSCHEME_A_UND_HYPERLINK       (0x08000006)
#define PPT_COLSCHEME_A_H_GESICHERT         (0x08000007)

////////////////////////////////////////////////////////////////////////////////////////////////////

#define ANSI_CHARSET            0
#define DEFAULT_CHARSET         1
#define SYMBOL_CHARSET          2
#define SHIFTJIS_CHARSET        128
#define HANGEUL_CHARSET         129
#define CHINESEBIG5_CHARSET     136
#define OEM_CHARSET             255

////////////////////////////////////////////////////////////////////////////////////////////////////

/* Font Families */
#define FF_DONTCARE             0x00
#define FF_ROMAN                0x10
#define FF_SWISS                0x20
#define FF_MODERN               0x30
#define FF_SCRIPT               0x40
#define FF_DECORATIVE           0x50

////////////////////////////////////////////////////////////////////////////////////////////////////

#define DEFAULT_PITCH           0x00
#define FIXED_PITCH             0x01
#define VARIABLE_PITCH          0x02

using namespace ::com::sun::star    ;
using namespace uno                 ;
using namespace beans               ;
using namespace drawing             ;
using namespace container           ;

////////////////////////////////////////////////////////////////////////////////////////////////////

PowerPointImportParam::PowerPointImportParam( SvStream& rDocStrm, sal_uInt32 nFlags, MSFilterTracer* pT ) :
    rDocStream      ( rDocStrm ),
    nImportFlags    ( nFlags ),
    pTracer         ( pT )
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void PptSlidePersistAtom::Clear()
{
    nReserved = nPsrReference = nFlags = nNumberTexts = nSlideId = 0;
}

SvStream& operator>>( SvStream& rIn, PptSlidePersistAtom& rAtom )
{
    DffRecordHeader aHd;
    rIn >> aHd
        >> rAtom.nPsrReference
        >> rAtom.nFlags
        >> rAtom.nNumberTexts
        >> rAtom.nSlideId;
//      >> rAtom.nReserved;
    aHd.SeekToEndOfRecord( rIn );
    return rIn;
}

SV_IMPL_PTRARR(_PptSlidePersistList,PptSlidePersistEntry*);

USHORT PptSlidePersistList::FindPage(UINT32 nId) const
{
    for ( USHORT i=0; i < Count(); i++ )
    {
        if (operator[](i)->GetSlideId()==nId) return i;
    }
    return PPTSLIDEPERSIST_ENTRY_NOTFOUND;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

SvStream& operator>>( SvStream& rIn, PptInteractiveInfoAtom& rAtom )
{
    rIn >> rAtom.nSoundRef
        >> rAtom.nExHyperlinkId
        >> rAtom.nAction
        >> rAtom.nOleVerb
        >> rAtom.nJump
        >> rAtom.nFlags
        >> rAtom.nHyperlinkType
        >> rAtom.nUnknown1
        >> rAtom.nUnknown2
        >> rAtom.nUnknown3;
    return rIn;
}

SvStream& operator>>( SvStream& rIn, PptExOleObjAtom& rAtom )
{
    rIn >> rAtom.nDummy0
        >> rAtom.nDummy1
        >> rAtom.nId
        >> rAtom.nDummy2
        >> rAtom.nPersistPtr
        >> rAtom.nDummy4;
    return rIn;
}

Size PptDocumentAtom::GetPageSize(const Size& rSiz) const
{
    return rSiz;
/*
    Size aRet;
    switch ( eSlidesPageFormat )
    {
        // Wenn man in Powerpoint als Seitenformat "Bildschirmgroesse"
        // einstellt, dann zeigt dieser Dialog zwar 24x18cm an, die
        // angezeigte Seite ist aber anders. Das sieht man, wenn man
        // ein Rechteck seitenfuellend aufzieht und sich dessen Groesse
        // ansieht. Die importierten Abmessungen sind auf jeden Fall
        // die, die auch im Ppt-File stehen. Ich denke, das es sich
        // hier eher um ein Bug in PowerPoint handelt, das im
        // Seitenformat-Dialog bei "Bildschirmgroesse" falsche Masse
        // angezeigt werden (vielleicht ja auch bildschirmabhaengig?).
//      case PPTPF_SCREEN  : aRet.Width()=4082; aRet.Height()=5443; break;
        case PPTPF_USLETTER: aRet.Width()=4896; aRet.Height()=6336; break;
        case PPTPF_A4      : aRet.Width()=4762; aRet.Height()=6735; break;
//      case PPTPF_35MMDIA : aRet.Width()=4082; aRet.Height()=6123; break;
//      case PPTPF_OVERHEAD: aRet.Width()=4082; aRet.Height()=5443; break;
    }
    if ( aRet.Width() )
    {
        if ( rSiz.Width() > rSiz.Height() )
        {   // Querformat
            long nMerk = aRet.Width();
            aRet.Width() = aRet.Height();
            aRet.Height() = nMerk;
        }
    }
    else    // CustomFormat oder Unbekannt oder Screen,Dia,Overhead
        aRet = rSiz;
    return aRet;
*/
}

SvStream& operator>>(SvStream& rIn, PptDocumentAtom& rAtom)
{
// Tatsaechliches Format:
//  00 aSlidePageSizeXY  8
//  08 aNotesPageSizeXY  8
//  16 aZoomRatio (OLE)  8
//  24 nNotesMasterPersist   4
//  28 nHandoutMasterPersist 4
//  32 n1stPageNumber    2
//  34 ePageFormat       2
//  36 bEmbeddedTrueType 1
//  37 bOmitTitlePlace   1
//  38 bRightToLeft      1
//  39 bShowComments     1

    DffRecordHeader aHd;
    INT32   nSlideX,nSlideY, nNoticeX, nNoticeY, nDummy;
    UINT16  nSlidePageFormat;
    INT8    nEmbeddedTrueType, nTitlePlaceHoldersOmitted, nRightToLeft, nShowComments;

    rIn >> aHd
        >> nSlideX >> nSlideY
        >> nNoticeX >> nNoticeY
        >> nDummy >> nDummy             // ZoomRation ueberspringen
        >> rAtom.nNotesMasterPersist
        >> rAtom.nHandoutMasterPersist
        >> rAtom.n1stPageNumber
        >> nSlidePageFormat
        >> nEmbeddedTrueType
        >> nTitlePlaceHoldersOmitted
        >> nRightToLeft
        >> nShowComments;
    rAtom.aSlidesPageSize.Width() = nSlideX;
    rAtom.aSlidesPageSize.Height() = nSlideY;
    rAtom.aNotesPageSize.Width() = nNoticeX;
    rAtom.aNotesPageSize.Height() = nNoticeY;
    rAtom.eSlidesPageFormat = (PptPageFormat)nSlidePageFormat;
    rAtom.bEmbeddedTrueType = nEmbeddedTrueType;
    rAtom.bTitlePlaceholdersOmitted = nTitlePlaceHoldersOmitted;
    rAtom.bRightToLeft = nRightToLeft;
    rAtom.bShowComments = nShowComments;
    aHd.SeekToEndOfRecord( rIn );
    return rIn;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void PptSlideLayoutAtom::Clear()
{
    eLayout = 0;
    for ( USHORT i = 0; i < 8; i++ )
    {
        aPlaceholderId[ i ] = 0;
        aPlacementId[ i ] = 0;
    }
}

SvStream& operator>>( SvStream& rIn, PptSlideLayoutAtom& rAtom )
{
    rIn >> rAtom.eLayout;
    rIn.Read( rAtom.aPlaceholderId, 8 );
    return rIn;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

SvStream& operator>>( SvStream& rIn, PptSlideAtom& rAtom )
{
    DffRecordHeader aHd;
    rIn >> aHd
        >> rAtom.aLayout
        >> rAtom.nMasterId
        >> rAtom.nNotesId
        >> rAtom.nFlags;
    aHd.SeekToEndOfRecord( rIn );
    return rIn;
}

void PptSlideAtom::Clear()
{
    nMasterId = nNotesId = 0;
    nFlags = 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

SvStream& operator>>( SvStream& rIn, PptNotesAtom& rAtom )
{
    DffRecordHeader aHd;
    rIn >> aHd
        >> rAtom.nSlideId
        >> rAtom.nFlags;
    aHd.SeekToEndOfRecord( rIn );
    return rIn;
}

void PptNotesAtom::Clear()
{
    nSlideId = 0;
    nFlags = 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Color PptColorSchemeAtom::GetColor( USHORT nNum ) const
{
    Color aRetval;
    if ( nNum < 8 )
    {
        nNum <<= 2;
        aRetval.SetRed( aData[ nNum++ ] );
        aRetval.SetGreen( aData[ nNum++ ] );
        aRetval.SetBlue( aData[ nNum++ ] );
    }
    return aRetval;
}

SvStream& operator>>( SvStream& rIn, PptColorSchemeAtom& rAtom )
{
    DffRecordHeader aHd;
    rIn >> aHd;
    rIn.Read( rAtom.aData, 32 );
    aHd.SeekToEndOfRecord( rIn );
    return rIn;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

SvStream& operator>>( SvStream& rIn, PptFontEntityAtom& rAtom )
{
    DffRecordHeader aHd;
    rIn >> aHd;
    sal_Unicode nTemp, cData[ 32 ];
    rIn.Read( cData, 64 );

    sal_uInt8   lfCharset, lfPitchAndFamily;

    rAtom.fScaling = 1.0;

    rIn >> lfCharset
        >> rAtom.lfClipPrecision
        >> rAtom.lfQuality
        >> lfPitchAndFamily;

    switch( lfCharset )
    {
        case SYMBOL_CHARSET :
            rAtom.eCharSet = RTL_TEXTENCODING_SYMBOL;
        break;
        case ANSI_CHARSET :
            rAtom.eCharSet = RTL_TEXTENCODING_MS_1252;
        break;

//      case DEFAULT_CHARSET :
//      case SHIFTJIS_CHARSET :
//      case HANGEUL_CHARSET :
//      case CHINESEBIG5_CHARSET :
//      case OEM_CHARSET :
        default :
            rAtom.eCharSet = gsl_getSystemTextEncoding();
    }
    switch ( lfPitchAndFamily & 0xf0 )
    {
        case FF_ROMAN:
            rAtom.eFamily = FAMILY_ROMAN;
        break;

        case FF_SWISS:
            rAtom.eFamily = FAMILY_SWISS;
        break;

        case FF_MODERN:
            rAtom.eFamily = FAMILY_MODERN;
        break;

        case FF_SCRIPT:
            rAtom.eFamily = FAMILY_SCRIPT;
        break;

        case FF_DECORATIVE:
             rAtom.eFamily = FAMILY_DECORATIVE;
        break;

        default:
            rAtom.eFamily = FAMILY_DONTKNOW;
        break;
    }

    switch ( lfPitchAndFamily & 0x0f )
    {
        case FIXED_PITCH:
            rAtom.ePitch = PITCH_FIXED;
        break;

        case DEFAULT_PITCH:
        case VARIABLE_PITCH:
        default:
            rAtom.ePitch = PITCH_VARIABLE;
        break;
    }
    sal_uInt16 i;
    for ( i = 0; i < 32; i++ )
    {
        nTemp = cData[ i ];
        if ( !nTemp )
            break;
#ifdef __BIGENDIAN
        cData[ i ] = ( nTemp >> 8 ) | ( nTemp << 8 );
#endif
    }
    rAtom.aName = String( cData, i );
    OutputDevice* pDev = (OutputDevice*)Application::GetDefaultDevice();
    rAtom.bAvailable = pDev->IsFontAvailable( rAtom.aName );
    aHd.SeekToEndOfRecord( rIn );
    return rIn;
}

SV_DECL_PTRARR_DEL( PptFontEntityAtomList, PptFontEntityAtom*, 16, 16 );
SV_IMPL_PTRARR( PptFontEntityAtomList, PptFontEntityAtom* );

class PptFontCollection: public PptFontEntityAtomList {
};

////////////////////////////////////////////////////////////////////////////////////////////////////

SvStream& operator>>( SvStream& rIn, PptUserEditAtom& rAtom )
{
    DffRecordHeader aHd;
    rIn >> aHd
        >> rAtom.nLastSlideID
        >> rAtom.nVersion
        >> rAtom.nOffsetLastEdit
        >> rAtom.nOffsetPersistDirectory
        >> rAtom.nDocumentRef
        >> rAtom.nMaxPersistWritten
        >> rAtom.eLastViewType;
    aHd.SeekToEndOfRecord(rIn);
    return rIn;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void PptOEPlaceholderAtom::Clear()
{
    nPlacementId = 0;
    nPlaceholderSize = nPlaceholderId = 0;
}

SvStream& operator>>( SvStream& rIn, PptOEPlaceholderAtom& rAtom )
{
    rIn >> rAtom.nPlacementId
        >> rAtom.nPlaceholderId
        >> rAtom.nPlaceholderSize;
    return rIn;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

PptSlidePersistEntry::PptSlidePersistEntry() :
    bNotesMaster            ( FALSE ),
    bHandoutMaster          ( FALSE ),
    bStarDrawFiller         ( FALSE ),
    pHeaderFooterEntry      ( NULL ),
    pSolverContainer        ( NULL ),
    pPresentationObjects    ( NULL ),
    pStyleSheet             ( NULL ),
    pBObj                   ( NULL ),
    bBObjIsTemporary        ( sal_True ),
    ePageKind               ( PPT_MASTERPAGE ),
    nDrawingDgId            ( 0xffffffff ),
    nBackgroundOffset       ( 0 ),
    nSlidePersistStartOffset( 0 ),
    nSlidePersistEndOffset  ( 0 )
{
}


PptSlidePersistEntry::~PptSlidePersistEntry()
{
    delete pStyleSheet;
    delete pSolverContainer;
    delete[] pPresentationObjects;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

SdrEscherImport::SdrEscherImport( PowerPointImportParam& rParam ) :
    SvxMSDffManager         ( rParam.rDocStream, rParam.pTracer ),
    pFonts                  ( NULL ),
    nStreamLen              ( 0 ),
    nTextStylesIndex        ( 0xffff ),
    bWingdingsChecked       ( FALSE ),
    bWingdingsAvailable     ( FALSE ),
    bMonotypeSortsChecked   ( FALSE ),
    bMonotypeSortsAvailable ( FALSE ),
    bTimesNewRomanChecked   ( FALSE ),
    bTimesNewRomanAvailable ( FALSE ),
    rImportParam            ( rParam ),
    eCharSetSystem          ( gsl_getSystemTextEncoding() )
{
}

SdrEscherImport::~SdrEscherImport()
{
    void* pPtr;
    for ( pPtr = aHFMasterList.First(); pPtr; pPtr = aHFMasterList.Next() )
        delete (HeaderFooterMaster*)pPtr;
    for ( pPtr = aOleObjectList.First(); pPtr; pPtr = aOleObjectList.Next() )
        delete (PPTOleEntry*)pPtr;
    delete pFonts;
}

const PptSlideLayoutAtom* SdrEscherImport::GetSlideLayoutAtom() const
{
    return NULL;
}

sal_Bool SdrEscherImport::ReadString( String& rStr ) const
{
    sal_Bool bRet = FALSE;
    DffRecordHeader aStrHd;
    rStCtrl >> aStrHd;
    if (aStrHd.nRecType == PPT_PST_TextBytesAtom
        || aStrHd.nRecType == PPT_PST_TextCharsAtom
        || aStrHd.nRecType == PPT_PST_CString)
    {
        sal_Bool bUniCode =
            (aStrHd.nRecType == PPT_PST_TextCharsAtom
            || aStrHd.nRecType == PPT_PST_CString);
        bRet=TRUE;
        ULONG nBytes = aStrHd.nRecLen;
        MSDFFReadZString( rStCtrl, rStr, nBytes, bUniCode );
        aStrHd.SeekToEndOfRecord( rStCtrl );
    }
    else
        aStrHd.SeekToBegOfRecord( rStCtrl );
    return bRet;
}

FASTBOOL SdrEscherImport::GetColorFromPalette(USHORT nNum, Color& rColor) const
{
    return FALSE;
}

BOOL SdrEscherImport::SeekToShape( SvStream& rSt, void* pClientData, UINT32 nId ) const
{
    return FALSE;
}

PptFontEntityAtom* SdrEscherImport::GetFontEnityAtom( UINT32 nNum ) const
{
    PptFontEntityAtom* pRetValue = NULL;
    if ( pFonts && ( nNum < pFonts->Count() ) )
        pRetValue = (*pFonts)[ (USHORT)nNum ];
    return pRetValue;
}

CharSet SdrEscherImport::GetCharSet( UINT32 nNum ) const
{
    CharSet eRetValue( eCharSetSystem );
    if ( pFonts && ( nNum < pFonts->Count() ) )
        eRetValue = (*pFonts)[ (USHORT)nNum ]->eCharSet;
    return eRetValue;
}

BOOL SdrEscherImport::IsFontAvailable( UINT32 nNum ) const
{
    BOOL bRetValue = FALSE;
    if ( pFonts && ( nNum < pFonts->Count() ) )
        bRetValue = (*pFonts)[ (USHORT)nNum ]->bAvailable;
    return bRetValue;
}

SdrObject* SdrEscherImport::ReadObjText( PPTTextObj* pTextObj, SdrObject* pObj, SdPage* pPage ) const
{
    return pObj;
}

void SdrEscherImport::ProcessClientAnchor2( SvStream& rSt, DffRecordHeader& rHd, void* pData, DffObjData& rObj )
{
    INT16 ls, os, rs, us;
    rSt >> os >> ls >> rs >> us; // etwas seltsame Koordinatenreihenfolge ...
    long l = ls, o = os, r = rs, u = us;
    Scale( l );
    Scale( o );
    Scale( r );
    Scale( u );
    rObj.aChildAnchor = Rectangle( l, o, r, u );
    rObj.bChildAnchor = TRUE;
    return;
};

void SdrEscherImport::RecolorGraphic( SvStream& rSt, sal_uInt32 nRecLen, Graphic& rGraphic )
{
    if ( rGraphic.GetType() == GRAPHIC_GDIMETAFILE )
    {
        sal_uInt16 nX, nGlobalColorsCount, nFillColorsCount;

        rSt >> nX
            >> nGlobalColorsCount
            >> nFillColorsCount
            >> nX
            >> nX
            >> nX;

        if ( ( nGlobalColorsCount <= 64 ) && ( nFillColorsCount <= 64 ) )
        {
            if ( (sal_uInt32)( ( nGlobalColorsCount + nFillColorsCount ) * 44 + 12 ) == nRecLen )
            {
                sal_uInt32 OriginalGlobalColors[ 64 ];
                sal_uInt32 NewGlobalColors[ 64 ];
                sal_uInt32 OriginalFillColors[ 64 ];
                sal_uInt32 NewFillColors[ 64 ];

                sal_uInt32 i, j, nGlobalColorsChanged, nFillColorsChanged;
                nGlobalColorsChanged = nFillColorsChanged = 0;

                sal_uInt32* pCurrentOriginal = OriginalGlobalColors;
                sal_uInt32* pCurrentNew = NewGlobalColors;
                sal_uInt32* pCount = &nGlobalColorsChanged;
                i = nGlobalColorsCount;

                for ( j = 0; j < 2; j++ )
                {
                    for ( ; i > 0; i-- )
                    {
                        sal_uInt32 nIndex, nPos = rSt.Tell();
                        sal_uInt8  nDummy, nRed, nGreen, nBlue;
                        sal_uInt16 nChanged;
                        rSt >> nChanged;
                        if ( nChanged & 1 )
                        {
                            sal_uInt32 nColor = 0;
                            rSt >> nDummy
                                >> nRed
                                >> nDummy
                                >> nGreen
                                >> nDummy
                                >> nBlue
                                >> nIndex;

                            if ( nIndex < 8 )
                            {
                                Color aColor = MSO_CLR_ToColor( nIndex << 24 );
                                nRed = aColor.GetRed();
                                nGreen = aColor.GetGreen();
                                nBlue = aColor.GetBlue();
                            }
                            nColor = nRed | ( nGreen << 8 ) | ( nBlue << 16 );
                            *pCurrentNew++ = nColor;
                            rSt >> nDummy
                                >> nRed
                                >> nDummy
                                >> nGreen
                                >> nDummy
                                >> nBlue;
                            nColor = nRed | ( nGreen << 8 ) | ( nBlue << 16 );
                            *pCurrentOriginal++ = nColor;
                            (*pCount)++;
                        }
                        rSt.Seek( nPos + 44 );
                    }
                    pCurrentOriginal = OriginalFillColors;
                    pCurrentNew = NewFillColors;
                    pCount = &nFillColorsChanged;
                    i = nFillColorsCount;
                }
                if ( nGlobalColorsChanged || nFillColorsChanged )
                {
                    Color* pSearchColors = new Color[ nGlobalColorsChanged ];
                    Color* pReplaceColors = new Color[ nGlobalColorsChanged ];

                    for ( j = 0; j < nGlobalColorsChanged; j++ )
                    {
                        sal_uInt32 nSearch = OriginalGlobalColors[ j ];
                        sal_uInt32 nReplace = NewGlobalColors[ j ];

                        pSearchColors[ j ].SetRed( (BYTE)nSearch );
                        pSearchColors[ j ].SetGreen( (BYTE)( nSearch >> 8 ) );
                        pSearchColors[ j ].SetBlue( (BYTE)( nSearch >> 16 ) );

                        pReplaceColors[ j ].SetRed( (BYTE)nReplace );
                        pReplaceColors[ j ].SetGreen( (BYTE)( nReplace >> 8 ) );
                        pReplaceColors[ j ].SetBlue( (BYTE)( nReplace >> 16 ) );
                    }
                    GDIMetaFile aGdiMetaFile( rGraphic.GetGDIMetaFile() );
                    aGdiMetaFile.ReplaceColors( pSearchColors, pReplaceColors,
                        nGlobalColorsChanged, NULL );
                    rGraphic = aGdiMetaFile;

                    delete[] pSearchColors;
                    delete[] pReplaceColors;
                }
            }
        }
    }
}

SdrObject* SdrEscherImport::ProcessObj( SvStream& rSt, DffObjData& rObjData, void* pData, Rectangle& rTextRect, SdrObject* pOriginalObj )
{
    SdrObject* pRet = pOriginalObj;
    ProcessData& rData = *((ProcessData*)pData);
    PptSlidePersistEntry& rPersistEntry = rData.rPersistEntry;

    PptOEPlaceholderAtom aPlaceholderAtom;
    INT16 nHeaderFooterInstance = -1;

    if ( maShapeRecords.SeekToContent( rSt, DFF_msofbtClientData, SEEK_FROM_CURRENT_AND_RESTART ) )
    {
        DffRecordHeader aPlaceHd;
        while ( ( rSt.GetError() == 0 ) && ( rSt.Tell() < maShapeRecords.Current()->GetRecEndFilePos() ) )
        {
            rSt >> aPlaceHd;
            switch ( aPlaceHd.nRecType )
            {
                case PPT_PST_OEPlaceholderAtom :
                {
                    rSt >> aPlaceholderAtom;
                    if ( nHeaderFooterInstance == -1 )
                    {
                        switch ( aPlaceholderAtom.nPlaceholderId )
                        {
                            case PPT_PLACEHOLDER_MASTERSLIDENUMBER :    nHeaderFooterInstance++;
                            case PPT_PLACEHOLDER_MASTERFOOTER :         nHeaderFooterInstance++;
                            case PPT_PLACEHOLDER_MASTERHEADER :         nHeaderFooterInstance++;
                            case PPT_PLACEHOLDER_MASTERDATE :           nHeaderFooterInstance++; break;
                        }
                        if ( ! ( nHeaderFooterInstance & 0xfffc ) )     // is this a valid instance ( 0->3 )
                        {
                            if ( ( rPersistEntry.aSlideAtom.aLayout.eLayout != PPT_LAYOUT_TITLEMASTERSLIDE )
                                    || ( aDocAtom.bTitlePlaceholdersOmitted == FALSE ) )
                            {
                                if ( !rPersistEntry.pHeaderFooterEntry )    // generate masterheaderfooter first
                                {
                                    HeaderFooterMaster* pHFM = new HeaderFooterMaster;
                                    if ( pHFM )
                                    {
                                        ((SdrEscherImport*)this )->aHFMasterList.Insert( (void*)pHFM, LIST_APPEND );
                                        HeaderFooterEntry* pHFE = new HeaderFooterEntry( *pHFM, rData.pPage );
                                        rPersistEntry.pHeaderFooterEntry = pHFE;
                                        pHFM->aHeaderFooterEntryList.Insert( pHFE, LIST_APPEND );
                                        DffRecordHeader* pHd;
                                        for ( pHd = ((SdrEscherImport*)this )->aDocRecManager.GetRecordHeader( PPT_PST_HeadersFooters, SEEK_FROM_BEGINNING );
                                                            pHd; pHd = ((SdrEscherImport*)this )->aDocRecManager.GetRecordHeader( PPT_PST_HeadersFooters, SEEK_FROM_CURRENT ) )
                                        {
                                            if ( rPersistEntry.bNotesMaster )
                                            {
                                                if ( pHd->nRecInstance == 4 )
                                                    break;
                                            }
                                            else if ( rPersistEntry.bHandoutMaster )
                                                continue;
                                            else if ( pHd->nRecInstance == 3 )      // normal master page
                                                break;
                                        }
                                        if ( pHd )
                                            ((SdrEscherImport*)this )->ImportHeaderFooterContainer( *pHd, *pHFE );
                                    }
                                }
                                if ( rPersistEntry.pHeaderFooterEntry )
                                {
                                    HeaderFooterMaster& rHFM = rPersistEntry.pHeaderFooterEntry->rMaster;
                                    if ( !rHFM.pOfs[ nHeaderFooterInstance ] )
                                        rHFM.pOfs[ nHeaderFooterInstance ] = rObjData.rSpHd.GetRecBegFilePos();
                                }
                            }
                            else
                            {
                                delete pRet;
                                return NULL;
                            }
                        }
                    }
                }
                break;

                case PPT_PST_RecolorInfoAtom :
                {
                    if ( pRet && ( pRet->ISA( SdrGrafObj ) && ((SdrGrafObj*)pRet)->HasGDIMetaFile() ) )
                    {
                        Graphic aGraphic( ((SdrGrafObj*)pRet)->GetGraphic() );
                        RecolorGraphic( rSt, aPlaceHd.nRecLen, aGraphic );
                        ((SdrGrafObj*)pRet)->SetGraphic( aGraphic );
                    }
                }
                break;
            }
            aPlaceHd.SeekToEndOfRecord( rSt );
        }
    }
    if ( ( aPlaceholderAtom.nPlaceholderId == PPT_PLACEHOLDER_NOTESSLIDEIMAGE ) && ( rPersistEntry.bNotesMaster == FALSE ) )
    {   // bPageObj
        USHORT nPageNum = pSdrModel->GetPageCount();
        if ( nPageNum > 0 )
            nPageNum--;
        delete pRet;
        pRet = new SdrPageObj( rObjData.rBoundRect, nPageNum );
    }
    else
    {
        PPTTextObj aTextObj( rSt, (SdrPowerPointImport&)*this, rPersistEntry, &rObjData );
        if ( ( aTextObj.Count() || aTextObj.GetOEPlaceHolderAtom() ) )
        {
            INT32 nTextRotationAngle = 0;
            if ( IsProperty( DFF_Prop_txflTextFlow ) )
            {
                MSO_TextFlow eTextFlow = (MSO_TextFlow)( GetPropertyValue( DFF_Prop_txflTextFlow ) & 0xFFFF );
                switch( eTextFlow )
                {
                    case mso_txflBtoT :                     // Bottom to Top non-@, unten -> oben
                        nTextRotationAngle += 9000;
                    break;
                    case mso_txflTtoBA :    /* #68110# */   // Top to Bottom @-font, oben -> unten
                    case mso_txflTtoBN :                    // Top to Bottom non-@, oben -> unten
                    case mso_txflVertN :                    // Vertical, non-@, oben -> unten
                        aTextObj.SetVertical( sal_True );   // nTextRotationAngle += 27000;
                    break;
//                  case mso_txflHorzN :                    // Horizontal non-@, normal
//                  case mso_txflHorzA :                    // Horizontal @-font, normal
                }
            }
            sal_Int32 nFontDirection = GetPropertyValue( DFF_Prop_cdirFont, mso_cdir0 );
            nTextRotationAngle -= nFontDirection * 9000;
            if ( ( nFontDirection == 1 ) || ( nFontDirection == 3 ) )       // #104546#
            {
                sal_Int32 nHalfWidth = ( rTextRect.GetWidth() + 1 ) >> 1;
                sal_Int32 nHalfHeight = ( rTextRect.GetHeight() + 1 ) >> 1;
                Point aTopLeft( rTextRect.Left() + nHalfWidth - nHalfHeight,
                                rTextRect.Top() + nHalfHeight - nHalfWidth );
                Size aNewSize( rTextRect.GetHeight(), rTextRect.GetWidth() );
                Rectangle aNewRect( aTopLeft, aNewSize );
                rTextRect = aNewRect;
            }
            if ( pRet )
            {
                BOOL bDeleteSource = FALSE;
                if ( ( ( GetPropertyValue( DFF_Prop_fNoFillHitTest ) & 0x10 )
                                + ( GetPropertyValue( DFF_Prop_fNoLineDrawDash ) & 8 ) ) == 0 )
                {
                    bDeleteSource = ( GetPropertyValue( DFF_Prop_FitTextToShape ) & 2 )         // fit shape to text
                                                && ( rObjData.eShapeType == mso_sptRectangle );
                }
                if ( aTextObj.GetOEPlaceHolderAtom() )              // we are not allowed to get
                    bDeleteSource = TRUE;                                                       // grouped placeholder objects
                if ( bDeleteSource  && ( pRet->ISA( SdrGrafObj ) == FALSE )
                        && ( pRet->ISA( SdrObjGroup ) == FALSE )
                            && ( pRet->ISA( SdrOle2Obj ) == FALSE ) )
                    delete pRet, pRet = NULL;
            }
            sal_uInt32 nTextFlags = aTextObj.GetTextFlags();
            sal_Bool  bVerticalText = aTextObj.GetVertical();
            sal_Int32 nTextLeft = GetPropertyValue( DFF_Prop_dxTextLeft, 25 * 3600 );   // 0.25 cm (emu)
            sal_Int32 nTextRight = GetPropertyValue( DFF_Prop_dxTextRight, 25 * 3600 ); // 0.25 cm (emu)
            sal_Int32 nTextTop = GetPropertyValue( DFF_Prop_dyTextTop, 13 * 3600 );     // 0.13 cm (emu)
            sal_Int32 nTextBottom = GetPropertyValue( DFF_Prop_dyTextBottom, 13 * 3600 );
            ScaleEmu( nTextLeft );
            ScaleEmu( nTextRight );
            ScaleEmu( nTextTop );
            ScaleEmu( nTextBottom );

            sal_Int32   nMinFrameWidth = 0;
            sal_Int32   nMinFrameHeight = 0;
            sal_Bool    bAutoGrowWidth, bAutoGrowHeight;

            SdrTextVertAdjust eTVA;
            SdrTextHorzAdjust eTHA;

            nTextFlags &= PPT_TEXTOBJ_FLAGS_PARA_ALIGNMENT_USED_LEFT   | PPT_TEXTOBJ_FLAGS_PARA_ALIGNMENT_USED_RIGHT
                        | PPT_TEXTOBJ_FLAGS_PARA_ALIGNMENT_USED_CENTER | PPT_TEXTOBJ_FLAGS_PARA_ALIGNMENT_USED_BLOCK;

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
                    {
                        // check if it is sensible to use the centered alignment
                        sal_uInt32 nMask = PPT_TEXTOBJ_FLAGS_PARA_ALIGNMENT_USED_LEFT | PPT_TEXTOBJ_FLAGS_PARA_ALIGNMENT_USED_RIGHT;
                        if ( ( nTextFlags & nMask ) != nMask )  // if the textobject has left and also right aligned pararagraphs
                            eTVA = SDRTEXTVERTADJUST_CENTER;    // the text has to be displayed using the full width;
                    }
                    break;

                    default :
                    {
                        if ( nTextFlags == PPT_TEXTOBJ_FLAGS_PARA_ALIGNMENT_USED_LEFT )
                            eTVA = SDRTEXTVERTADJUST_TOP;
                        else if ( nTextFlags == PPT_TEXTOBJ_FLAGS_PARA_ALIGNMENT_USED_RIGHT )
                            eTVA = SDRTEXTVERTADJUST_BOTTOM;
                    }
                    break;
                }
                nMinFrameWidth = rTextRect.GetWidth() - ( nTextLeft + nTextRight );
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
                    {
                        // check if it is sensible to use the centered alignment
                        sal_uInt32 nMask = PPT_TEXTOBJ_FLAGS_PARA_ALIGNMENT_USED_LEFT | PPT_TEXTOBJ_FLAGS_PARA_ALIGNMENT_USED_RIGHT;
                        if ( ( nTextFlags & nMask ) != nMask )  // if the textobject has left and also right aligned pararagraphs
                            eTHA = SDRTEXTHORZADJUST_CENTER;    // the text has to be displayed using the full width;
                    }
                    break;

                    default :
                    {
                        if ( nTextFlags == PPT_TEXTOBJ_FLAGS_PARA_ALIGNMENT_USED_LEFT )
                            eTHA = SDRTEXTHORZADJUST_LEFT;
                        else if ( nTextFlags == PPT_TEXTOBJ_FLAGS_PARA_ALIGNMENT_USED_RIGHT )
                            eTHA = SDRTEXTHORZADJUST_RIGHT;
                    }
                    break;
                }
                nMinFrameHeight = rTextRect.GetHeight() - ( nTextTop + nTextBottom );
            }

            SdrObjKind eTextKind = OBJ_RECT;
            if ( ( aPlaceholderAtom.nPlaceholderId == PPT_PLACEHOLDER_NOTESSLIDEIMAGE )
                || ( aPlaceholderAtom.nPlaceholderId == PPT_PLACEHOLDER_MASTERNOTESSLIDEIMAGE ) )
            {
                aTextObj.SetInstance( 2 );
                eTextKind = OBJ_TITLETEXT;
            }
            else if ( ( aPlaceholderAtom.nPlaceholderId == PPT_PLACEHOLDER_MASTERNOTESBODYIMAGE )
                || ( aPlaceholderAtom.nPlaceholderId == PPT_PLACEHOLDER_NOTESBODY ) )
            {
                aTextObj.SetInstance( 2 );
                eTextKind = OBJ_TEXT;
            }

            sal_uInt32 nInstanceInSheet = aTextObj.GetInstance();
            if ( ( rPersistEntry.ePageKind == PPT_MASTERPAGE ) )
            {
                if ( !rPersistEntry.pPresentationObjects )
                {
                    rPersistEntry.pPresentationObjects = new UINT32[ PPT_STYLESHEETENTRYS ];
                    memset( rPersistEntry.pPresentationObjects, 0, PPT_STYLESHEETENTRYS * 4 );
                }
                if ( !rPersistEntry.pPresentationObjects[ nInstanceInSheet ] )
                    rPersistEntry.pPresentationObjects[ nInstanceInSheet ] = rObjData.nOldFilePos;
            }
            switch ( nInstanceInSheet )
            {
                case TSS_TYPE_PAGETITLE :
                case TSS_TYPE_TITLE :
                {
                    if ( GetSlideLayoutAtom()->eLayout == PPT_LAYOUT_TITLEMASTERSLIDE )
                        nInstanceInSheet = TSS_TYPE_TITLE;
                    else
                        nInstanceInSheet = TSS_TYPE_PAGETITLE;
                }
                break;
                case TSS_TYPE_BODY :
//              case TSS_TYPE_SUBTITLE :
                case TSS_TYPE_HALFBODY :
                case TSS_TYPE_QUARTERBODY :
                    nInstanceInSheet = TSS_TYPE_BODY;
                break;
            }
            aTextObj.SetMappedInstance( (sal_uInt16)nInstanceInSheet );

            switch ( aTextObj.GetInstance() )
            {
                case TSS_TYPE_PAGETITLE :
                case TSS_TYPE_TITLE : eTextKind = OBJ_TITLETEXT; break;
                case TSS_TYPE_SUBTITLE : eTextKind = OBJ_TEXT; break;
                case TSS_TYPE_BODY :
                case TSS_TYPE_HALFBODY :
                case TSS_TYPE_QUARTERBODY : eTextKind = OBJ_OUTLINETEXT; break;
            }
            SdrObject* pTObj = NULL;
            sal_Bool bWordWrap = (MSO_WrapMode)GetPropertyValue( DFF_Prop_WrapText, mso_wrapSquare ) != mso_wrapNone;
            sal_Bool bFitShapeToText = ( GetPropertyValue( DFF_Prop_FitTextToShape ) & 2 ) != 0;
            if ( bWordWrap || ( eTextKind != OBJ_RECT ) )
            {
                pTObj = new SdrRectObj( eTextKind != OBJ_RECT ? eTextKind : OBJ_TEXT );
                if ( bVerticalText )
                {
                    bAutoGrowWidth = bFitShapeToText;   // bFitShapeToText; can't be used, because we cut the text if it is too height,
                    bAutoGrowHeight = sal_False;
                }
                else
                {
                    bAutoGrowWidth = sal_False;
                    bAutoGrowHeight = sal_True;         // bFitShapeToText; can't be used, because we cut the text if it is too height,
                }
            }
            else
            {
                pTObj = new SdrRectObj();
                bAutoGrowHeight = bAutoGrowWidth = bFitShapeToText;
            }
            pTObj->SetModel( pSdrModel );
            SfxItemSet aSet( pSdrModel->GetItemPool() );
            if ( !pRet )
                ((SdrEscherImport*)this)->ApplyAttributes( rSt, aSet, pTObj );
            else if ( !((SdrRectObj*)pTObj)->IsTextFrame() )
            {
                aSet.Put( XLineStyleItem( XLINE_NONE ) );
                aSet.Put( XFillStyleItem( XFILL_NONE ) );
            }
            aSet.Put( SvxFrameDirectionItem( bVerticalText ? FRMDIR_VERT_TOP_RIGHT : FRMDIR_HORI_LEFT_TOP, EE_PARA_WRITINGDIR ) );

             aSet.Put( SdrTextAutoGrowWidthItem( bAutoGrowWidth ) );
            aSet.Put( SdrTextAutoGrowHeightItem( bAutoGrowHeight ) );

            aSet.Put( SdrTextVertAdjustItem( eTVA ) );
            aSet.Put( SdrTextHorzAdjustItem( eTHA ) );

            if ( nMinFrameHeight < 0 )
                nMinFrameHeight = 0;
            aSet.Put( SdrTextMinFrameHeightItem( nMinFrameHeight ) );

            if ( nMinFrameWidth < 0 )
                nMinFrameWidth = 0;
            aSet.Put( SdrTextMinFrameWidthItem( nMinFrameWidth ) );

            // Abstaende an den Raendern der Textbox setzen
            aSet.Put( SdrTextLeftDistItem( nTextLeft ) );
            aSet.Put( SdrTextRightDistItem( nTextRight ) );
            aSet.Put( SdrTextUpperDistItem( nTextTop ) );
            aSet.Put( SdrTextLowerDistItem( nTextBottom ) );
            pTObj->SetItemSet( aSet );
            pTObj->SetSnapRect( rTextRect );
            pTObj = ReadObjText( &aTextObj, pTObj, rData.pPage );

            if ( pTObj )
            {
                /* check if our new snaprect makes trouble,
                   because we do not display the ADJUST_BLOCK
                   properly if the textsize is bigger than the
                   snaprect of the object. Then we will use
                   ADJUST_CENTER instead of ADJUST_BLOCK.
                */
                if ( !bFitShapeToText && !bWordWrap )
                {
                    SdrTextObj* pText = PTR_CAST( SdrTextObj, pTObj );
                    if ( pText )
                    {
                        if ( bVerticalText )
                        {
                            if ( eTVA == SDRTEXTVERTADJUST_BLOCK )
                            {
                                Size aTextSize( pText->GetTextSize() );
                                aTextSize.Width() += nTextLeft + nTextRight;
                                aTextSize.Height() += nTextTop + nTextBottom;
                                if ( rTextRect.GetHeight() < aTextSize.Height() )
                                    pTObj->SetItem( SdrTextVertAdjustItem( SDRTEXTVERTADJUST_CENTER ) );
                            }
                        }
                        else
                        {
                            if ( eTHA == SDRTEXTHORZADJUST_BLOCK )
                            {
                                Size aTextSize( pText->GetTextSize() );
                                aTextSize.Width() += nTextLeft + nTextRight;
                                aTextSize.Height() += nTextTop + nTextBottom;
                                if ( rTextRect.GetWidth() < aTextSize.Width() )
                                    pTObj->SetItem( SdrTextHorzAdjustItem( SDRTEXTHORZADJUST_CENTER ) );
                            }
                        }
                    }
                }
                // rotate text with shape ?
                sal_Int32 nAngle = ( rObjData.nSpFlags & SP_FFLIPV ) ? -mnFix16Angle : mnFix16Angle;    // #72116# vertical flip -> rotate by using the other way
                nAngle += nTextRotationAngle;

                if ( rObjData.nSpFlags & SP_FFLIPV )
                {
                    double a = 18000 * nPi180;
                    pTObj->Rotate( rTextRect.Center(), 18000, sin( a ), cos( a ) );
                }
                if ( rObjData.nSpFlags & SP_FFLIPH )
                    nAngle = 36000 - nAngle;
                if ( nAngle )
                {
                    double a = nAngle * nPi180;
                    pTObj->NbcRotate( rObjData.rBoundRect.Center(), nAngle, sin( a ), cos( a ) );
                }
                if ( pRet )
                {
                    SdrObject* pGroup = new SdrObjGroup;
                    pGroup->GetSubList()->NbcInsertObject( pRet );
                    pGroup->GetSubList()->NbcInsertObject( pTObj );
                    pRet = pGroup;
                }
                else
                    pRet = pTObj;
            }
        }
    }
    if ( pRet )
    {
        if ( rObjData.nSpFlags & SP_FBACKGROUND )
        {
            pRet->NbcSetSnapRect( Rectangle( Point(), ((SdrPage*)rData.pPage)->GetSize() ) );   // Groesse setzen
        }
        if ( ( nHeaderFooterInstance != -1 )  && rPersistEntry.pHeaderFooterEntry ) // headerfooter ?
        {
            HeaderFooterEntry& rHFE = *rPersistEntry.pHeaderFooterEntry;
            if ( rHFE.bMaster && ( !rHFE.pSdrObject[ nHeaderFooterInstance ] ) )            // master ?
            {
                rHFE.pInstanceOrder[ rHFE.nInstanceCount++ ] = nHeaderFooterInstance;
                rHFE.pSdrObject[ nHeaderFooterInstance ] = pRet;
                rHFE.pPageIndex[ nHeaderFooterInstance ] = ((SdrPage*)rData.pPage)->GetObjCount();
                pRet = NULL;
            }
        }
        if ( rPersistEntry.pSolverContainer )
        {
            for ( SvxMSDffConnectorRule* pPtr = (SvxMSDffConnectorRule*)rPersistEntry.pSolverContainer->aCList.First();
                pPtr; pPtr = (SvxMSDffConnectorRule*)rPersistEntry.pSolverContainer->aCList.Next() )
            {
                if ( rObjData.nShapeId == pPtr->nShapeC )
                    pPtr->pCObj = pRet;
                else
                {
                    SdrObject* pConnectObj = pRet;
                    if ( pOriginalObj && pRet->ISA( SdrObjGroup ) )
                    {   /* check if the original object from the escherimport is part of the group object,
                        if this is the case, we will use the original object to connect to */
                        SdrObjListIter aIter( *pRet, IM_DEEPWITHGROUPS );
                        while( aIter.IsMore() )
                        {
                            SdrObject* pPartObj = aIter.Next();
                            if ( pPartObj == pOriginalObj )
                            {
                                pConnectObj = pPartObj;
                                break;
                            }
                        }
                    }
                    if ( rObjData.nShapeId == pPtr->nShapeA )
                    {
                        pPtr->pAObj = pConnectObj;
                        pPtr->nSpFlagsA = rObjData.nSpFlags;
                    }
                    if ( rObjData.nShapeId == pPtr->nShapeB )
                    {
                        pPtr->pBObj = pConnectObj;
                        pPtr->nSpFlagsB = rObjData.nSpFlags;
                    }
                }
            }
        }
        if ( rPersistEntry.ePageKind == PPT_MASTERPAGE )
        {   // maybe the escher clusterlist is not correct, but we have to got the right page by using the
            // spMaster property, so we are patching the table
            if ( rPersistEntry.nDrawingDgId != 0xffffffff )
            {
                UINT32 nSec = ( rObjData.nShapeId >> 10 ) - 1;
                if ( nSec < mnIdClusters )
                    mpFidcls[ nSec ].dgid = rPersistEntry.nDrawingDgId; // insert the correct drawing id;
            }
        }
        if ( GetPropertyValue( DFF_Prop_fNoFillHitTest ) & 0x10 )
        {
            if ( (MSO_FillType)GetPropertyValue( DFF_Prop_fillType, mso_fillSolid ) == mso_fillBackground )
            {
                if ( !rData.pBackgroundColoredObjects )
                    rData.pBackgroundColoredObjects = new List;
                rData.pBackgroundColoredObjects->Insert( pRet, LIST_APPEND );
            }
        }
    }
    return pRet;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrEscherImport::CheckWingdings() const
{
    OutputDevice* pDev = (OutputDevice*)Application::GetDefaultDevice();
    ((SdrEscherImport*)this)->bWingdingsAvailable = pDev->IsFontAvailable( String( RTL_CONSTASCII_USTRINGPARAM( "WINGDINGS" ) ) );
    ((SdrEscherImport*)this)->bWingdingsChecked = TRUE;
}

void SdrEscherImport::CheckMonotypeSorts() const
{
    OutputDevice* pDev = (OutputDevice*)Application::GetDefaultDevice();
    ((SdrEscherImport*)this)->bMonotypeSortsAvailable = pDev->IsFontAvailable( String( RTL_CONSTASCII_USTRINGPARAM( "MONOTYPE SORTS" ) ) );
    ((SdrEscherImport*)this)->bMonotypeSortsChecked = TRUE;
}

void SdrEscherImport::CheckTimesNewRoman() const
{
    OutputDevice* pDev = (OutputDevice*)Application::GetDefaultDevice();
    ((SdrEscherImport*)this)->bTimesNewRomanAvailable = pDev->IsFontAvailable( String( RTL_CONSTASCII_USTRINGPARAM( "TIMES NEW ROMAN" ) ) );
    ((SdrEscherImport*)this)->bTimesNewRomanChecked = TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

SdrPowerPointImport::SdrPowerPointImport( PowerPointImportParam& rParam ) :
    bOk                 ( rStCtrl.GetErrorCode() == SVSTREAM_OK ),
    SdrEscherImport     ( rParam ),
    pDefaultSheet       ( NULL ),
    pPersistPtr         ( NULL ),
    nPersistPtrAnz      ( 0 ),
    pMasterPages        ( NULL ),
    pSlidePages         ( NULL ),
    pNotePages          ( NULL ),
    nAktPageNum         ( 0 ),
    nDocStreamPos       ( 0 ),
    nPageColorsNum      ( 0xFFFF ),
    ePageColorsKind     ( PPT_MASTERPAGE ),
    eAktPageKind        ( PPT_MASTERPAGE )
{
    DffRecordHeader* pHd;
    if ( bOk )
    {
        rStCtrl.Seek( STREAM_SEEK_TO_END );
        nStreamLen = rStCtrl.Tell();
        rStCtrl.Seek( 0 );
        aPptRecManager.Consume( rStCtrl, FALSE, nStreamLen );

        // UserEditAtom lesen
        // Erstmal immer das letzte, falls mehrere vorhanden sind.
        for ( pHd = aPptRecManager.Last(); pHd; pHd = aPptRecManager.Prev() )
        {
            if ( pHd->nRecType == PPT_PST_UserEditAtom )
            {
                pHd->SeekToBegOfRecord( rStCtrl );
                rStCtrl >> aUserEditAtom;
                break;
            }
        }
        if ( !pHd )
            bOk = FALSE;
    }
    if ( rStCtrl.GetError() != 0 )
        bOk = FALSE;
    if ( bOk )
    {   // PersistPtrs lesen (alle)
        nPersistPtrAnz = aUserEditAtom.nMaxPersistWritten + 1;  // 1 mehr, damit ich immer direkt indizieren kann
        pPersistPtr = new UINT32[ nPersistPtrAnz ];             // (die fangen naemlich eigentlich bei 1 an)
        memset( pPersistPtr, 0x00, nPersistPtrAnz * 4 );

        for ( pHd = aPptRecManager.GetRecordHeader( PPT_PST_PersistPtrIncrementalBlock, SEEK_FROM_BEGINNING );
                pHd; pHd = aPptRecManager.GetRecordHeader( PPT_PST_PersistPtrIncrementalBlock, SEEK_FROM_CURRENT ) )
        {
            ULONG nPibLen = pHd->GetRecEndFilePos();
            pHd->SeekToContent( rStCtrl );
            while ( bOk && ( rStCtrl.GetError() == 0 ) && ( rStCtrl.Tell() < nPibLen ) )
            {
                UINT32 nOfs, nAnz;
                rStCtrl >> nOfs;
                nAnz = nOfs;
                nOfs &= 0x000FFFFF;
                nAnz >>= 20;
                while ( bOk && ( rStCtrl.GetError() == 0 ) && ( nAnz > 0 ) && ( nOfs < nPersistPtrAnz ) )
                {
                    rStCtrl >> pPersistPtr[ nOfs ];
                    if ( pPersistPtr[ nOfs ] > nStreamLen )
                    {
                        bOk = FALSE;
                        DBG_ERROR("SdrPowerPointImport::Ctor(): Ungueltiger Eintrag im Persist-Directory!");
                    }
                    nOfs++;
                    nAnz--;
                }
                if ( bOk && nAnz > 0 )
                {
                    DBG_ERROR("SdrPowerPointImport::Ctor(): Nicht alle Persist-Directory Entraege gelesen!");
                    bOk = FALSE;
                }
            }
        }
    }
    if ( rStCtrl.GetError() != 0 )
        bOk = FALSE;
    if ( bOk )
    {   // Document PersistEntry checken
        nDocStreamPos = aUserEditAtom.nDocumentRef;
        if ( nDocStreamPos > nPersistPtrAnz )
        {
            DBG_ERROR("SdrPowerPointImport::Ctor(): aUserEditAtom.nDocumentRef ungueltig!");
            bOk = FALSE;
        }
    }
    if ( bOk )
    {   // Document FilePos checken
        nDocStreamPos = pPersistPtr[ nDocStreamPos ];
        if ( nDocStreamPos >= nStreamLen )
        {
            DBG_ERROR("SdrPowerPointImport::Ctor(): nDocStreamPos >= nStreamLen!");
            bOk = FALSE;
        }
    }
    if ( bOk )
    {
        rStCtrl.Seek( nDocStreamPos );
        aDocRecManager.Consume( rStCtrl );

        DffRecordHeader aDocHd;
        rStCtrl >> aDocHd;
        // DocumentAtom lesen
        DffRecordHeader aDocAtomHd;
        rStCtrl >> aDocAtomHd;
        if ( aDocHd.nRecType == PPT_PST_Document && aDocAtomHd.nRecType == PPT_PST_DocumentAtom )
        {
            aDocAtomHd.SeekToBegOfRecord( rStCtrl );
            rStCtrl >> aDocAtom;
        }
        else
            bOk = FALSE;

        if ( bOk )
        {
            if ( !pFonts )
                ReadFontCollection();

            // reading TxPF, TxSI
            PPTTextParagraphStyleAtomInterpreter    aTxPFStyle;
            PPTTextSpecInfoAtomInterpreter          aTxSIStyle; // styles (default language setting ... )

            DffRecordHeader* pEnvHd = aDocRecManager.GetRecordHeader( PPT_PST_Environment );
            if ( pEnvHd )
            {
                pEnvHd->SeekToContent( rStCtrl );
                DffRecordHeader aTxPFStyleRecHd;
                if ( SeekToRec( rStCtrl, PPT_PST_TxPFStyleAtom, pEnvHd->GetRecEndFilePos(), &aTxPFStyleRecHd ) )
                    aTxPFStyle.Read( rStCtrl, aTxPFStyleRecHd );

                pEnvHd->SeekToContent( rStCtrl );
                DffRecordHeader aTxSIStyleRecHd;
                if ( SeekToRec( rStCtrl, PPT_PST_TxSIStyleAtom, pEnvHd->GetRecEndFilePos(), &aTxSIStyleRecHd ) )
                {
                    aTxSIStyle.Read( rStCtrl, aTxSIStyleRecHd, PPT_PST_TxSIStyleAtom );
#ifdef DBG_UTIL
                    if ( !aTxSIStyle.bValid )
                    {
                        if (!(rImportParam.nImportFlags & PPT_IMPORTFLAGS_NO_TEXT_ASSERT ))
                        {
                            DBG_ERROR( "SdrTextSpecInfoAtomInterpreter::Ctor(): parsing error, this document needs to be analysed (SJ)" );
                        }
                    }
#endif
                }
            }

            // todo:: PPT_PST_TxPFStyleAtom

            // SlidePersists Lesen
            pMasterPages=new PptSlidePersistList;
            pSlidePages =new PptSlidePersistList;
            pNotePages  =new PptSlidePersistList;

            // now always creating the handout page, it will be the first in our masterpage list
            PptSlidePersistEntry* pE = new PptSlidePersistEntry;
            pE->aPersistAtom.nPsrReference = aDocAtom.nHandoutMasterPersist;
            pE->bHandoutMaster = sal_True;
            if ( !aDocAtom.nHandoutMasterPersist )
                pE->bStarDrawFiller = sal_True;     // this is a dummy master page
            pMasterPages->C40_INSERT( PptSlidePersistEntry, pE, 0 );

            USHORT nPageListNum = 0;
            DffRecordHeader* pSlideListWithTextHd = aDocRecManager.GetRecordHeader( PPT_PST_SlideListWithText );
            PptSlidePersistEntry* pPreviousPersist = NULL;
            while ( pSlideListWithTextHd && ( nPageListNum < 3 ) )
            {
                pSlideListWithTextHd->SeekToContent( rStCtrl );
                PptSlidePersistList* pPageList = GetPageList( PptPageKind( nPageListNum ) );
                sal_uInt32 nSlideListWithTextHdEndOffset = pSlideListWithTextHd->GetRecEndFilePos();
                while ( SeekToRec( rStCtrl, PPT_PST_SlidePersistAtom, nSlideListWithTextHdEndOffset ) )
                {
                    if ( pPreviousPersist )
                        pPreviousPersist->nSlidePersistEndOffset = rStCtrl.Tell();
                    PptSlidePersistEntry* pE = new PptSlidePersistEntry;
                    rStCtrl >> pE->aPersistAtom;
                    pE->nSlidePersistStartOffset = rStCtrl.Tell();
                    pE->ePageKind = PptPageKind( nPageListNum );
                    pPageList->C40_INSERT( PptSlidePersistEntry, pE, pPageList->Count() );
                    pPreviousPersist = pE;
                }
                if ( pPreviousPersist )
                    pPreviousPersist->nSlidePersistEndOffset = nSlideListWithTextHdEndOffset;
                pSlideListWithTextHd = aDocRecManager.GetRecordHeader( PPT_PST_SlideListWithText, SEEK_FROM_CURRENT );
                nPageListNum++;
            }

            // we will ensure that there is at least one master page
            if ( pMasterPages->Count() == 1 )   // -> there is only a handout page available
            {
                PptSlidePersistEntry* pE = new PptSlidePersistEntry;
                pE->bStarDrawFiller = sal_True;         // this is a dummy master page
                pMasterPages->C40_INSERT( PptSlidePersistEntry, pE, 1 );
            }

            // now we will insert at least one notes master for each master page
            sal_uInt16 nMasterPage;
            sal_uInt16 nMasterPages = pMasterPages->Count() - 1;
            for ( nMasterPage = 0; nMasterPage < nMasterPages; nMasterPage++ )
            {
                PptSlidePersistEntry* pE = new PptSlidePersistEntry;
                pE->bNotesMaster = sal_True;
                pE->bStarDrawFiller = sal_True;         // this is a dummy master page
                if ( !nMasterPage && aDocAtom.nNotesMasterPersist )
                {   // special treatment for the first notes master
                    pE->aPersistAtom.nPsrReference = aDocAtom.nNotesMasterPersist;
                    pE->bStarDrawFiller = sal_False;    // this is a dummy master page
                }
                pMasterPages->C40_INSERT( PptSlidePersistEntry, pE, ( nMasterPage + 1 ) << 1 );
            }

            // Zu jeder Page noch das SlideAtom bzw. NotesAtom lesen, soweit vorhanden
            nPageListNum = 0;
            for ( nPageListNum = 0; nPageListNum < 3; nPageListNum++ )
            {
                PptSlidePersistList* pPageList = GetPageList( PptPageKind( nPageListNum ) );
                for ( USHORT nPageNum = 0; nPageNum < pPageList->Count(); nPageNum++ )
                {
                    PptSlidePersistEntry* pE = (*pPageList)[ nPageNum ];
                    ULONG nPersist = pE->aPersistAtom.nPsrReference;
                    if ( ( nPersist > 0 ) && ( nPersist < nPersistPtrAnz ) )
                    {
                        ULONG nFPos = pPersistPtr[ nPersist ];
                        if ( nFPos < nStreamLen )
                        {
                            rStCtrl.Seek( nFPos );
                            DffRecordHeader aSlideHd;
                            rStCtrl >> aSlideHd;
                            if ( SeekToRec( rStCtrl, PPT_PST_SlideAtom, aSlideHd.GetRecEndFilePos() ) )
                                rStCtrl >> pE->aSlideAtom;
                            else if ( SeekToRec( rStCtrl, PPT_PST_NotesAtom, aSlideHd.GetRecEndFilePos() ) )
                                rStCtrl >> pE->aNotesAtom;
                            aSlideHd.SeekToContent( rStCtrl );

                            DffRecordHeader aPPTDrawingHd;
                            if ( SeekToRec( rStCtrl, PPT_PST_PPDrawing, aSlideHd.GetRecEndFilePos(), &aPPTDrawingHd ) )
                            {
                                DffRecordHeader aPPTDgContainer;
                                if ( SeekToRec( rStCtrl, DFF_msofbtDgContainer, aPPTDrawingHd.GetRecEndFilePos(), &aPPTDgContainer ) )
                                {
                                    if ( SeekToRec( rStCtrl, DFF_msofbtDg, aPPTDrawingHd.GetRecEndFilePos() ) )
                                    {
                                        DffRecordHeader aDgRecordHeader;
                                        rStCtrl >> aDgRecordHeader;
                                        pE->nDrawingDgId = aDgRecordHeader.nRecInstance;
                                        aDgRecordHeader.SeekToEndOfRecord( rStCtrl );
                                    }
                                    if ( SeekToRec( rStCtrl, DFF_msofbtSolverContainer, aPPTDgContainer.GetRecEndFilePos() ) )
                                    {
                                        pE->pSolverContainer = new SvxMSDffSolverContainer;
                                        rStCtrl >> *( pE->pSolverContainer );
                                    }
                                    aPPTDgContainer.SeekToBegOfRecord( rStCtrl );
                                    SetDgContainer( rStCtrl );  // set this, so that the escherimport is knowing of our drawings
                                }
                            }
                            // office xp is supporting more than one stylesheet
                            if ( ( pE->ePageKind == PPT_MASTERPAGE ) && ( pE->aSlideAtom.nMasterId == 0 ) && ( pE->bNotesMaster == 0 ) )
                            {
                                PPTTextSpecInfo aTxSI( 0 );
                                if ( aTxSIStyle.bValid && aTxSIStyle.aList.Count() )
                                    aTxSI = *( ( (PPTTextSpecInfo*)aTxSIStyle.aList.GetObject( 0 ) ) );

                                pE->pStyleSheet = new PPTStyleSheet( aSlideHd, rStCtrl, *this, aTxPFStyle, aTxSI );
                                pDefaultSheet = pE->pStyleSheet;
                            }
                            if ( SeekToRec( rStCtrl, PPT_PST_ColorSchemeAtom, aSlideHd.GetRecEndFilePos() ) )
                                rStCtrl >> pE->aColorScheme;
                            else
                                DBG_ERROR( "SdrPowerPointImport::Ctor(): could not get SlideColorScheme! (SJ)" );
                        }
                        else
                        {
                            DBG_ERROR("SdrPowerPointImport::Ctor(): Persist-Eintrag fehlerhaft! (SJ)");
                        }
                    }
                }
            }
        }
    }
    if ( ( rStCtrl.GetError() != 0 ) || ( pDefaultSheet == NULL ) )
        bOk = FALSE;
    pPPTStyleSheet = pDefaultSheet;
    rStCtrl.Seek( 0 );
}

SdrPowerPointImport::~SdrPowerPointImport()
{
    for ( void* pPtr = aHyperList.First(); pPtr; pPtr = aHyperList.Next() )
        delete (SdHyperlinkEntry*)pPtr;
    delete pMasterPages;
    delete pSlidePages;
    delete pNotePages;
    delete[] pPersistPtr;
}

sal_Bool PPTConvertOCXControls::InsertControl(
        const com::sun::star::uno::Reference<
        com::sun::star::form::XFormComponent > &rFComp,
        const com::sun::star::awt::Size& rSize,
        com::sun::star::uno::Reference<
        com::sun::star::drawing::XShape > *pShape,
        BOOL bFloatingCtrl )
{
    sal_Bool bRetValue = FALSE;
    try
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >  xShape;

        const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer > & rFormComps =
            GetFormComps();

        ::com::sun::star::uno::Any aTmp( &rFComp, ::getCppuType((const ::com::sun::star::uno::Reference<
            com::sun::star::form::XFormComponent >*)0) );

        rFormComps->insertByIndex( rFormComps->getCount(), aTmp );

        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > & rServiceFactory =
            GetServiceFactory();
        if( rServiceFactory.is() )
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >  xCreate = rServiceFactory
                ->createInstance(String( RTL_CONSTASCII_STRINGPARAM( "com.sun.star.drawing.ControlShape" ) ) );
            if( xCreate.is() )
            {
                xShape = ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >(xCreate, ::com::sun::star::uno::UNO_QUERY);
                if ( xShape.is() )
                {
                    xShape->setSize(rSize);
//                  GetShapes()->add( xShape );
                    // Das Control-Model am Control-Shape setzen
                    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XControlShape >  xControlShape( xShape,
                        ::com::sun::star::uno::UNO_QUERY );
                    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >  xControlModel( rFComp,
                        ::com::sun::star::uno::UNO_QUERY );
                    if ( xControlShape.is() && xControlModel.is() )
                    {
                        xControlShape->setControl( xControlModel );
                        if (pShape)
                            *pShape = xShape;
                        bRetValue = TRUE;
                    }
                }
            }
        }
    }
    catch( ... )
    {
        bRetValue = FALSE;
    }
    return bRetValue;
};

const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage >& PPTConvertOCXControls::GetDrawPage()
{
    if( !xDrawPage.is() && pDocSh )
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > xModel( pDocSh->GetModel() );
        ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPages > xDrawPages;
        switch( ePageKind )
        {
            case PPT_SLIDEPAGE :
            case PPT_NOTEPAGE :
            {
                ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPagesSupplier >
                        xDrawPagesSupplier( xModel, ::com::sun::star::uno::UNO_QUERY);
                if ( xDrawPagesSupplier.is() )
                    xDrawPages = xDrawPagesSupplier->getDrawPages();
            }
            break;

            case PPT_MASTERPAGE :
            {
                ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XMasterPagesSupplier >
                        xMasterPagesSupplier( xModel, ::com::sun::star::uno::UNO_QUERY);
                if ( xMasterPagesSupplier.is() )
                    xDrawPages = xMasterPagesSupplier->getMasterPages();
            }
            break;
        }
        if ( xDrawPages.is() && xDrawPages->getCount() )
        {
            xDrawPages->getCount();
            ::com::sun::star::uno::Any aAny( xDrawPages->getByIndex( xDrawPages->getCount() - 1 ) );
            aAny >>= xDrawPage;
        }
    }
    return xDrawPage;
}


SdrObject* SdrPowerPointImport::ImportOLE( long nOLEId, const Graphic& rGraf, const Rectangle& rBoundRect ) const
{
    SdrObject* pRet = NULL;

    UINT32 nOldPos = rStCtrl.Tell();

    Graphic aGraphic( rGraf );

    if ( ((SdrPowerPointImport*)this)->maShapeRecords.SeekToContent( rStCtrl, DFF_msofbtClientData, SEEK_FROM_CURRENT_AND_RESTART ) )
    {
        DffRecordHeader aPlaceHd;
        while ( ( rStCtrl.GetError() == 0 )
            && ( rStCtrl.Tell() < ((SdrPowerPointImport*)this)->maShapeRecords.Current()->GetRecEndFilePos() ) )
        {
            rStCtrl >> aPlaceHd;
            if ( aPlaceHd.nRecType == PPT_PST_RecolorInfoAtom )
            {
                ((SdrPowerPointImport*)this)->RecolorGraphic( rStCtrl, aPlaceHd.nRecLen, aGraphic );
                break;
            }
            else
                aPlaceHd.SeekToEndOfRecord( rStCtrl );
        }
    }

    PPTOleEntry* pOe;
    for ( pOe = (PPTOleEntry*)((SdrPowerPointImport*)this)->aOleObjectList.First(); pOe;
            pOe = (PPTOleEntry*)((SdrPowerPointImport*)this)->aOleObjectList.Next() )
    {
        if ( pOe->nId != (sal_uInt32)nOLEId )
            continue;

        rStCtrl.Seek( pOe->nRecHdOfs );

        DffRecordHeader aHd;
        rStCtrl >> aHd;

        sal_uInt32 nLen = aHd.nRecLen - 4;
        if ( (INT32)nLen > 0 )
        {
            char* pBuf = new char[ nLen ];
            rStCtrl.SeekRel( 4 );
            rStCtrl.Read( pBuf, nLen );
            SvMemoryStream* pDest = new SvMemoryStream;
            ZCodec aZCodec( 0x8000, 0x8000 );
            aZCodec.BeginCompression();
            SvMemoryStream aSource( pBuf, nLen, STREAM_READ );
            aZCodec.Decompress( aSource, *pDest );
            delete[] pBuf;

#ifdef DBG_EXTRACTOLEOBJECTS

            static sal_Int32    nCount;
            String              aFileURLStr;

            if( ::utl::LocalFileHelper::ConvertPhysicalNameToURL( Application::GetAppFileName(), aFileURLStr ) )
            {
                INetURLObject   aURL( aFileURLStr );
                String          aFileName( String( RTL_CONSTASCII_STRINGPARAM( "dbgole" ) ) );

                aFileName.Append( String::CreateFromInt32( nCount++ ) );
                aURL.SetName( aFileName );

                SvStream* pDbgOut = ::utl::UcbStreamHelper::CreateStream( aURL.GetMainURL( INetURLObject::NO_DECODE ), STREAM_TRUNC | STREAM_WRITE );

                if( pDbgOut )
                {
                    pDest->Seek( STREAM_SEEK_TO_END );
                    pDbgOut->Write( pDest->GetData(), pDest->Tell() );
                    pDest->Seek( STREAM_SEEK_TO_BEGIN );
                    delete pDbgOut;
                }
            }
#endif
            if ( !aZCodec.EndCompression() )
                delete pDest;
            else
            {
                Storage* pObjStor = new Storage( *pDest, TRUE );
                if ( pObjStor )
                {
                    SvStorageRef xObjStor( new SvStorage( pObjStor ) );
                    if ( xObjStor.Is() )
                    {
                        if ( xObjStor->GetClassName() == SvGlobalName() )
                        {
                            ClsId aId( pObjStor-> GetClassId() );
                            xObjStor->SetClass( SvGlobalName( aId.n1, aId.n2, aId.n3, aId.n4, aId.n5, aId.n6, aId.n7, aId.n8, aId.n9, aId.n10, aId.n11 ),
                                pObjStor->GetFormat(), pObjStor->GetUserName() );
                        }
                        SvStorageStreamRef xSrcTst = xObjStor->OpenStream( String( RTL_CONSTASCII_USTRINGPARAM( "\1Ole" ) ) );
                        if ( xSrcTst.Is() )
                        {
                            BYTE aTestA[ 10 ];
                            BOOL bGetItAsOle = ( sizeof( aTestA ) == xSrcTst->Read( aTestA, sizeof( aTestA ) ) );
                            if ( !bGetItAsOle )
                            {   // maybe there is a contentsstream in here
                                xSrcTst = xObjStor->OpenStream( String( RTL_CONSTASCII_USTRINGPARAM( "Contents" ) ), STREAM_READWRITE | STREAM_NOCREATE );
                                bGetItAsOle = ( xSrcTst.Is() && sizeof( aTestA ) == xSrcTst->Read( aTestA, sizeof( aTestA ) ) );
                            }
                            if ( bGetItAsOle )
                            {
                                if ( nSvxMSDffOLEConvFlags )
                                {
                                    SvStorageRef xDestStorage( pOe->pShell->GetStorage() );
                                    SvInPlaceObjectRef xIPObj( CheckForConvertToSOObj(
                                                nSvxMSDffOLEConvFlags, *xObjStor,
                                                *xDestStorage, rGraf ));
                                    if( xIPObj.Is() )
                                    {
                                        String aNm( pOe->pShell->InsertObject( xIPObj, String() )->GetObjName() );
                                        pRet = new SdrOle2Obj( xIPObj, aNm, rBoundRect, FALSE );
                                    }
                                }
                                if ( !pRet && ( pOe->nType == PPT_PST_ExControl ) )
                                {
                                    PPTConvertOCXControls aPPTConvertOCXControls( pOe->pShell, eAktPageKind );
                                    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > xShape;
                                    if ( aPPTConvertOCXControls.ReadOCXStream( xObjStor, &xShape, FALSE ) )
                                        pRet = GetSdrObjectFromXShape( xShape );
                                }
                                if ( !pRet )
                                {
                                    GDIMetaFile aMtf;
                                    SvEmbeddedObject::MakeContentStream( xObjStor,
                                                *lcl_GetMetaFileFromGrf_Impl( aGraphic, aMtf ) );

                                    SvInPlaceObjectRef xInplaceObj( ((SvFactory*)SvInPlaceObject::
                                                            ClassFactory())->CreateAndLoad( xObjStor ) );
                                    if( xInplaceObj.Is() )
                                    {

                                        // VisArea am OutplaceObject setzen!!
                                        Size aSize( OutputDevice::LogicToLogic( aGraphic.GetPrefSize(),
                                            aGraphic.GetPrefMapMode(), MapMode( xInplaceObj->GetMapUnit() ) ) );
                                        // modifiziert wollen wir nicht werden

                                        xInplaceObj->EnableSetModified( FALSE );
                                        xInplaceObj->SetVisArea( Rectangle( Point(), aSize ) );
                                        xInplaceObj->EnableSetModified( TRUE );

                                        String aNm( pOe->pShell->InsertObject( xInplaceObj, String() )->GetObjName() );
                                        pRet = new SdrOle2Obj( xInplaceObj, aNm, rBoundRect, FALSE );
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    rStCtrl.Seek( nOldPos );

    return pRet;
}

SvMemoryStream* SdrPowerPointImport::ImportExOleObjStg( UINT32 nPersistPtr, UINT32& nOleId ) const
{
    SvMemoryStream* pRet = NULL;
    if ( nPersistPtr && ( nPersistPtr < nPersistPtrAnz ) )
    {
        UINT32 nOldPos, nOfs = pPersistPtr[ nPersistPtr ];
        nOldPos = rStCtrl.Tell();
        rStCtrl.Seek( nOfs );
        DffRecordHeader aHd;
        rStCtrl >> aHd;
        if ( aHd.nRecType == DFF_PST_ExOleObjStg )
        {
            UINT32 nLen = aHd.nRecLen - 4;
            if ( (INT32)nLen > 0 )
            {
                rStCtrl >> nOleId;
                pRet = new SvMemoryStream;
                ZCodec aZCodec( 0x8000, 0x8000 );
                aZCodec.BeginCompression();
                aZCodec.Decompress( rStCtrl, *pRet );
                if ( !aZCodec.EndCompression() )
                    delete pRet, pRet = NULL;
            }
        }
        rStCtrl.Seek( nOldPos );
    }
    return pRet;
}

void SdrPowerPointImport::SeekOle( SfxObjectShell* pShell, sal_uInt32 nFilterOptions )
{
    if ( pShell )
    {
        DffRecordHeader*    pHd;

        UINT32 nOldPos = rStCtrl.Tell();
        if ( nFilterOptions & 1 )
        {
            pHd = aDocRecManager.GetRecordHeader( PPT_PST_List, SEEK_FROM_BEGINNING );
            if ( pHd )
            {
                // we try to locate the basic atom
                pHd->SeekToContent( rStCtrl );
                if ( SeekToRec( rStCtrl, PPT_PST_VBAInfo, pHd->GetRecEndFilePos(), pHd ) )
                {
                    if ( SeekToRec( rStCtrl, PPT_PST_VBAInfoAtom, pHd->GetRecEndFilePos(), pHd ) )
                    {
                        UINT32 nPersistPtr, nIDoNotKnow1, nIDoNotKnow2;
                        rStCtrl >> nPersistPtr
                                >> nIDoNotKnow1
                                >> nIDoNotKnow2;

                        UINT32 nOleId;
                        SvMemoryStream* pBas = ImportExOleObjStg( nPersistPtr, nOleId );
                        if ( pBas )
                        {
                            SvStorageRef xSource( new SvStorage( pBas, TRUE ) );
                            SvStorageRef xDest( new SvStorage( new SvMemoryStream(), TRUE ) );
                            if ( xSource.Is() && xDest.Is() )
                            {
                                // is this a visual basic storage ?
                                SvStorageRef xSubStorage = xSource->OpenStorage( String( RTL_CONSTASCII_USTRINGPARAM( "VBA" ) ),
                                    STREAM_READWRITE | STREAM_NOCREATE | STREAM_SHARE_DENYALL );
                                if( xSubStorage.Is() && ( SVSTREAM_OK == xSubStorage->GetError() ) )
                                {
                                    SvStorageRef xMacros = xDest->OpenStorage( String( RTL_CONSTASCII_USTRINGPARAM( "MACROS" ) ) );
                                    if ( xMacros.Is() )
                                    {
                                        SvStorageInfoList aList;
                                        xSource->FillInfoList( &aList );
                                        UINT32 i;

                                        BOOL bCopied = TRUE;
                                        for ( i = 0; i < aList.Count(); i++ )   // copy all entrys
                                        {
                                            const SvStorageInfo& rInfo = aList[ i ];
                                            if ( !xSource->CopyTo( rInfo.GetName(), xMacros, rInfo.GetName() ) )
                                                bCopied = FALSE;
                                        }
                                        if ( i && bCopied )
                                        {
                                            SvxImportMSVBasic aMSVBas( *pShell, *xDest, TRUE, FALSE );
                                            int nSuccess = aMSVBas.Import( String( RTL_CONSTASCII_USTRINGPARAM( "MACROS" ) ),
                                                    String( RTL_CONSTASCII_USTRINGPARAM( "VBA" ) ), TRUE, FALSE );

                                            SvStorageRef xDoc( pShell->GetStorage() );
                                            if ( xDoc.Is() && ( xDoc->GetError() == SVSTREAM_OK ) )
                                            {
                                                SvStorageRef xVBA = xDoc->OpenStorage( String( RTL_CONSTASCII_USTRINGPARAM( "_MS_VBA_Macros" ) ) );
                                                if ( xVBA.Is() && ( xVBA->GetError() == SVSTREAM_OK ) )
                                                {
                                                    SvStorageRef xSubVBA = xVBA->OpenStorage( String( RTL_CONSTASCII_USTRINGPARAM( "_MS_VBA_Overhead" ) ) );
                                                    if ( xSubVBA.Is() && ( xSubVBA->GetError() == SVSTREAM_OK ) )
                                                    {
                                                        SvStorageStreamRef xOriginal = xSubVBA->OpenStream( String( RTL_CONSTASCII_USTRINGPARAM( "_MS_VBA_Overhead2" ) ) );
                                                        if ( xOriginal.Is() && ( xOriginal->GetError() == SVSTREAM_OK ) )
                                                        {
                                                            if ( nPersistPtr && ( nPersistPtr < nPersistPtrAnz ) )
                                                            {
                                                                rStCtrl.Seek( pPersistPtr[ nPersistPtr ] );
                                                                rStCtrl >> *pHd;

                                                                *xOriginal  << nIDoNotKnow1
                                                                                << nIDoNotKnow2;

                                                                UINT32 nSource, nToCopy, nBufSize;
                                                                nSource = rStCtrl.Tell();
                                                                nToCopy = pHd->nRecLen;
                                                                BYTE* pBuf = new BYTE[ 0x40000 ];   // 256KB Buffer
                                                                if ( pBuf )
                                                                {
                                                                    while ( nToCopy )
                                                                    {
                                                                        nBufSize = ( nToCopy >= 0x40000 ) ? 0x40000 : nToCopy;
                                                                        rStCtrl.Read( pBuf, nBufSize );
                                                                        xOriginal->Write( pBuf, nBufSize );
                                                                        nToCopy -= nBufSize;
                                                                    }
                                                                    delete[] pBuf;
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        pHd = aDocRecManager.GetRecordHeader( PPT_PST_ExObjList, SEEK_FROM_BEGINNING );
        if ( pHd )
        {
            DffRecordManager*   pExObjListManager = NULL;
            DffRecordHeader*    pExEmbed = NULL;

            pHd->SeekToBegOfRecord( rStCtrl );
            pExObjListManager = new DffRecordManager( rStCtrl );
            sal_uInt16 i, nRecType;

            for ( i = 0; i < 2; i++ )
            {
                switch ( i )
                {
                    case 0 : nRecType = PPT_PST_ExEmbed; break;
                    case 1 : nRecType = PPT_PST_ExControl; break;
                }
                for ( pExEmbed = pExObjListManager->GetRecordHeader( nRecType, SEEK_FROM_BEGINNING );
                        pExEmbed; pExEmbed = pExObjListManager->GetRecordHeader( nRecType, SEEK_FROM_CURRENT ) )
                {
                    pExEmbed->SeekToContent( rStCtrl );

                    DffRecordHeader aExOleAtHd;
                    if ( SeekToRec( rStCtrl, PPT_PST_ExOleObjAtom, pExEmbed->GetRecEndFilePos(), &aExOleAtHd ) )
                    {
                        PptExOleObjAtom aAt;
                        rStCtrl >> aAt;

                        if ( aAt.nPersistPtr && ( aAt.nPersistPtr < nPersistPtrAnz ) )
                        {
                            UINT32 nId;
                            rStCtrl.Seek( pPersistPtr[ aAt.nPersistPtr ] );
                            DffRecordHeader aHd;
                            rStCtrl >> aHd;
                            if ( aHd.nRecType == DFF_PST_ExOleObjStg )
                            {
                                rStCtrl >> nId;
                                aOleObjectList.Insert( new PPTOleEntry( aAt.nId, aHd.nFilePos, pShell, nRecType ) );
                            }
                        }
                    }
                }
            }
            delete pExObjListManager;
        }
        rStCtrl.Seek( nOldPos );
    }
}

sal_Bool SdrPowerPointImport::ReadFontCollection()
{
    sal_Bool bRet = FALSE;
    DffRecordHeader* pEnvHd = aDocRecManager.GetRecordHeader( PPT_PST_Environment );
    if ( pEnvHd )
    {
        ULONG nFPosMerk = rStCtrl.Tell(); // FilePos merken fuer spaetere Restauration
        pEnvHd->SeekToContent( rStCtrl );
        DffRecordHeader aListHd;
        if ( SeekToRec( rStCtrl, PPT_PST_FontCollection, pEnvHd->GetRecEndFilePos(), &aListHd ) )
        {
            sal_uInt16 nCount = 0;
            VirtualDevice* pVDev = NULL;
            while ( SeekToRec( rStCtrl, PPT_PST_FontEntityAtom, aListHd.GetRecEndFilePos() ) )
            {
                bRet = TRUE;
                if ( !pFonts )
                    pFonts = new PptFontCollection;
                PptFontEntityAtom* pFont = new PptFontEntityAtom;
                rStCtrl >> *pFont;

                Font aFont;
                aFont.SetCharSet( pFont->eCharSet );
                aFont.SetName( pFont->aName );
                aFont.SetFamily( pFont->eFamily );
                aFont.SetPitch( pFont->ePitch );
                aFont.SetHeight( 100 );

                if ( mbTracing && !pFont->bAvailable )
                    mpTracer->Trace( rtl::OUString::createFromAscii( "sd1000" ), pFont->aName );

                SvxFont aTmpFont( aFont );
                if ( !pVDev )
                    pVDev = new VirtualDevice;
                aTmpFont.SetPhysFont( pVDev );
                FontMetric aMetric( pVDev->GetFontMetric() );

                sal_uInt16 nTxtHeight = (sal_uInt16)aMetric.GetAscent() + (sal_uInt16)aMetric.GetDescent();

                if ( nTxtHeight )
                {
                    double fScaling = 120.0 / (double)nTxtHeight;
                    if ( ( fScaling > 0.50 ) && ( fScaling < 1.5 ) )
                        pFont->fScaling = fScaling;
                }

#ifdef DBG_EXTRACTFONTMETRICS

                String  aFileURLStr;
                if( ::utl::LocalFileHelper::ConvertPhysicalNameToURL( Application::GetAppFileName(), aFileURLStr ) )
                {
                    INetURLObject   aURL( aFileURLStr );
                    aURL.SetName( String( RTL_CONSTASCII_STRINGPARAM( "dbgfontmetrics.txt" ) ) );

                    SvStream* pDbgOut = ::utl::UcbStreamHelper::CreateStream( aURL.GetMainURL( INetURLObject::NO_DECODE ), STREAM_WRITE );
                    if( pDbgOut )
                    {
                        pDbgOut->Seek( STREAM_SEEK_TO_END );

                        Printer* pPrinter = NULL;
                        if ( pSdrModel->GetRefDevice() && pSdrModel->GetRefDevice()->GetOutDevType() == OUTDEV_PRINTER )
                            pPrinter = (Printer*)pSdrModel->GetRefDevice();
                        if ( pPrinter )
                        {
                            Font aOldFont( pPrinter->GetFont() );
                            aFont.SetKerning( TRUE );
                            pPrinter->SetFont( aFont );
                            aMetric = pPrinter->GetFontMetric();
                            pPrinter->SetFont( aOldFont );
                        }

                        if ( ( pPrinter == NULL ) || ( aMetric.GetLeading() == 0 ) )
                        {
                            VirtualDevice aVirDev( 1 );
                            aVirDev.SetFont( aFont );
                            aMetric = aVirDev.GetFontMetric();
                        }
                        ByteString aFontName( aFont.GetName(), RTL_TEXTENCODING_UTF8 );
                        ByteString aHeight( ByteString::CreateFromInt32( aMetric.GetLineHeight() ) );
                        ByteString aAscent( ByteString::CreateFromInt32( aMetric.GetAscent() ) );
                        ByteString aDescent( ByteString::CreateFromInt32( aMetric.GetDescent() ) );
                        ByteString aLeading( ByteString::CreateFromInt32( aMetric.GetLeading() ) );
                        ByteString aPhysHeight( ByteString::CreateFromInt32( nTxtHeight ) );

                        *pDbgOut                                             << (sal_uInt8)0xa
                                 << "FontName  : " << aFontName.GetBuffer()  << (sal_uInt8)0xa
                                 << "    Height: " << aHeight.GetBuffer()    << (sal_uInt8)0xa
                                 << "    Ascent: " << aAscent.GetBuffer()    << (sal_uInt8)0xa
                                 << "    Descent:" << aDescent.GetBuffer()   << (sal_uInt8)0xa
                                 << "    Leading:" << aLeading.GetBuffer()   << (sal_uInt8)0xa
                                 << "PhysHeight :" << aPhysHeight.GetBuffer()<< (sal_uInt8)0xa;
                    }
                    delete pDbgOut;
                 }
#endif
                // following block is necessary, because our old PowerPoint export did not set the
                // correct charset
                if ( pFont->aName.EqualsIgnoreCaseAscii( "Wingdings" ) ||
                    pFont->aName.EqualsIgnoreCaseAscii( "Wingdings 2" ) ||
                        pFont->aName.EqualsIgnoreCaseAscii( "Wingdings 3" ) ||
                            pFont->aName.EqualsIgnoreCaseAscii( "Monotype Sorts" ) ||
                                pFont->aName.EqualsIgnoreCaseAscii( "Monotype Sorts 2" ) ||
                                    pFont->aName.EqualsIgnoreCaseAscii( "Webdings" ) ||
                                        pFont->aName.EqualsIgnoreCaseAscii( "StarBats" ) ||
                                            pFont->aName.EqualsIgnoreCaseAscii( "StarMath" ) ||
                                                pFont->aName.EqualsIgnoreCaseAscii( "ZapfDingbats" ) )
                {
                    pFont->eCharSet = RTL_TEXTENCODING_SYMBOL;
                };
                pFonts->C40_INSERT( PptFontEntityAtom, pFont, nCount++ );
            }
            delete pVDev;
        }
        rStCtrl.Seek( nFPosMerk ); // FilePos restaurieren
    }
    return bRet;
}

PptSlidePersistList* SdrPowerPointImport::GetPageList(PptPageKind ePageKind) const
{
    if ( ePageKind == PPT_MASTERPAGE )
        return pMasterPages;
    if ( ePageKind == PPT_SLIDEPAGE )
        return pSlidePages;
    if ( ePageKind == PPT_NOTEPAGE )
        return pNotePages;
    return NULL;
}

SdrOutliner* SdrPowerPointImport::GetDrawOutliner( SdrTextObj* pSdrText ) const
{
    if ( !pSdrText )
        return NULL;
    else
        return &pSdrText->ImpGetDrawOutliner();
}


SdrObject* SdrPowerPointImport::ReadObjText( PPTTextObj* pTextObj, SdrObject* pSdrObj, SdPage* pPage ) const
{
    SdrTextObj* pText = PTR_CAST( SdrTextObj, pSdrObj );
    if ( pText )
    {
        if ( !ApplyTextObj( pTextObj, pText, pPage, NULL, NULL ) )
            pSdrObj = NULL;
    }
    return pSdrObj;
}


SdrObject* SdrPowerPointImport::ApplyTextObj( PPTTextObj* pTextObj, SdrTextObj* pSdrText, SdPage* pPage,
                                                SfxStyleSheet* pSheet, SfxStyleSheet** ppStyleSheetAry ) const
{
    SdrTextObj* pText = pSdrText;
    if ( pTextObj->Count() )
    {
        UINT32 nInstanceInSheet = pTextObj->GetMappedInstance() ;
        SdrOutliner& rOutliner = pText->ImpGetDrawOutliner();
        rOutliner.SetMinDepth( 0 );
        if ( ( pText->GetObjInventor() == SdrInventor ) && ( pText->GetObjIdentifier() == OBJ_TITLETEXT ) ) // Outliner-Style fuer Titel-Textobjekt?!? (->von DL)
            rOutliner.Init( OUTLINERMODE_TITLEOBJECT );             // Outliner reset

        BOOL bOldUpdateMode = rOutliner.GetUpdateMode();
        rOutliner.SetUpdateMode( FALSE );
        if ( pSheet )
        {
            if ( rOutliner.GetStyleSheet( 0 ) != pSheet )
                rOutliner.SetStyleSheet( 0, pSheet );
        }
        rOutliner.SetVertical( pTextObj->GetVertical() );
        const PPTParagraphObj* pPreviousParagraph = NULL;
        for ( PPTParagraphObj* pPara = pTextObj->First(); pPara; pPara = pTextObj->Next() )
        {
            UINT32 nTextSize = pPara->GetTextSize();
            if ( ! ( nTextSize & 0xffff0000 ) )
            {
                PPTPortionObj* pPortion;
                sal_Unicode* pParaText = new sal_Unicode[ nTextSize ];
                UINT32 nCurrentIndex = 0;
                for ( pPortion = pPara->First(); pPortion; pPortion = pPara->Next() )
                {
                    if ( pPortion->mpFieldItem )
                        pParaText[ nCurrentIndex++ ] = ' ';
                    else
                    {
                        sal_uInt32 nCharacters = pPortion->Count();
                        const sal_Unicode* pSource = pPortion->maString.GetBuffer();
                        sal_Unicode* pDest = pParaText + nCurrentIndex;

                        sal_uInt32 nFont;
                        pPortion->GetAttrib( PPT_CharAttr_Font, nFont, pTextObj->GetInstance() );
                        PptFontEntityAtom* pFontEnityAtom = GetFontEnityAtom( nFont );
                        if ( pFontEnityAtom && ( pFontEnityAtom->eCharSet == RTL_TEXTENCODING_SYMBOL ) )
                        {
                            sal_uInt32 i;
                            sal_Unicode nUnicode;
                            for ( i = 0; i < nCharacters; i++ )
                            {
                                nUnicode = pSource[ i ];
                                if ( ! ( nUnicode & 0xff00 ) )
                                    nUnicode |= 0xf000;
                                pDest[ i ] = nUnicode;
                            }
                        }
                        else
                            memcpy( pDest, pSource, nCharacters << 1 );
                        nCurrentIndex += nCharacters;
                    }
                }
                sal_uInt16  nParaIndex = (UINT16)pTextObj->GetCurrentIndex();
                SfxStyleSheet* pS = ( ppStyleSheetAry ) ? ppStyleSheetAry[ pPara->pParaSet->mnDepth ] : pSheet;

                ESelection aSelection( nParaIndex, 0, nParaIndex, 0 );
                rOutliner.Insert( String(), nParaIndex, pPara->GetLevel() );
                rOutliner.QuickInsertText( String( pParaText, (UINT16)nCurrentIndex ), aSelection );
                rOutliner.SetParaAttribs( nParaIndex, rOutliner.GetEmptyItemSet() );
                if ( pS )
                    rOutliner.SetStyleSheet( nParaIndex, pS );

                for ( pPortion = pPara->First(); pPortion; pPortion = pPara->Next() )
                {
                    SfxItemSet aPortionAttribs( rOutliner.GetEmptyItemSet() );
                    SvxFieldItem* pFieldItem = pPortion->GetTextField();
                    if ( pFieldItem )
                    {
                        rOutliner.QuickInsertField( *pFieldItem, ESelection( nParaIndex, aSelection.nEndPos, nParaIndex, aSelection.nEndPos + 1 ) );
                        aSelection.nEndPos++;
                        delete pFieldItem;
                    }
                    else
                    {
                        const sal_Unicode *pF, *pPtr = pPortion->maString.GetBuffer();
                        const sal_Unicode *pMax = pPtr + pPortion->maString.Len();
                        sal_Int32 nLen;
                        for ( pF = pPtr; pPtr < pMax; pPtr++ )
                        {
                            if ( *pPtr == 0xb )
                            {
                                nLen = pPtr - pF;
                                if ( nLen )
                                    aSelection.nEndPos += (sal_uInt16)nLen;
                                pF = pPtr + 1;
                                rOutliner.QuickInsertLineBreak( ESelection( nParaIndex, aSelection.nEndPos, nParaIndex, aSelection.nEndPos + 1 ) );
                                aSelection.nEndPos++;
                            }
                        }
                        nLen = pPtr - pF;
                        if ( nLen )
                            aSelection.nEndPos += (sal_uInt16)nLen;
                    }
                    pPortion->ApplyTo( aPortionAttribs, (SdrPowerPointImport&)*this, nInstanceInSheet, pTextObj );
                    rOutliner.QuickSetAttribs( aPortionAttribs, aSelection );
                    aSelection.nStartPos = aSelection.nEndPos;
                }
                SfxItemSet aParagraphAttribs( rOutliner.GetEmptyItemSet() );
                pPara->ApplyTo( aParagraphAttribs, (SdrPowerPointImport&)*this, nInstanceInSheet, pPreviousParagraph );
                pPreviousParagraph = pPara;
                if ( !aSelection.nStartPos )    // in PPT empty paragraphs never gets a bullet
                    aParagraphAttribs.Put( SfxUInt16Item( EE_PARA_BULLETSTATE, FALSE ) );
                aSelection.nStartPos = 0;
                rOutliner.QuickSetAttribs( aParagraphAttribs, aSelection );
                delete[] pParaText;
            }
        }
        OutlinerParaObject* pNewText = rOutliner.CreateParaObject();
        rOutliner.Clear();
        rOutliner.SetMinDepth( 0 );
        rOutliner.SetUpdateMode( bOldUpdateMode );
        pText->SetOutlinerParaObject( pNewText );
    }
    return pText;
}

sal_Bool SdrPowerPointImport::SeekToDocument( DffRecordHeader* pRecHd ) const
{
    sal_Bool bRet;
    ULONG nFPosMerk = rStCtrl.Tell(); // FilePos merken fuer ggf. spaetere Restauration
    rStCtrl.Seek( nDocStreamPos );
    DffRecordHeader aDocHd;
    rStCtrl >> aDocHd;
    bRet = aDocHd.nRecType == PPT_PST_Document;
    if ( bRet )
    {
        if ( pRecHd )
            *pRecHd = aDocHd;
        else
            aDocHd.SeekToBegOfRecord( rStCtrl );
    }
    if ( !bRet )
        rStCtrl.Seek( nFPosMerk ); // FilePos restaurieren
    return bRet;
}

sal_Bool SdrPowerPointImport::SeekToContentOfProgTag( sal_Int32 nVersion, SvStream& rSt,
                                const DffRecordHeader& rSourceHd, DffRecordHeader& rContentHd )
{
    sal_Bool    bRetValue = sal_False;
    sal_uInt32  nOldPos = rSt.Tell();

    DffRecordHeader aProgTagsHd, aProgTagBinaryDataHd;

    rSourceHd.SeekToContent( rSt );
    if ( SeekToRec( rSt, PPT_PST_ProgTags, rSourceHd.GetRecEndFilePos(), &aProgTagsHd ) )
    {
        while( SeekToRec( rSt, PPT_PST_ProgBinaryTag, aProgTagsHd.GetRecEndFilePos(), &aProgTagBinaryDataHd ) )
        {
            rSt >> rContentHd;
            if ( rContentHd.nRecType == PPT_PST_CString )
            {
                sal_uInt16  n = 6;
                sal_uInt32  i = rContentHd.nRecLen >> 1;
                if ( i > n )
                {
                    String aPre, aSuf;
                    sal_Unicode *pTmp = aPre.AllocBuffer( n );
                    while ( n-- )
                        rSt >> *pTmp++;
                    n = (sal_uInt16)( i - 6 );
                    pTmp = aSuf.AllocBuffer( n );
                    while ( n-- )
                        rSt >> *pTmp++;
                    sal_Int32 nV = aSuf.ToInt32();
                    if ( ( nV == nVersion ) && ( aPre == String( RTL_CONSTASCII_USTRINGPARAM( "___PPT" ) ) ) )
                    {
                        rContentHd.SeekToEndOfRecord( rSt );
                        rSt >> rContentHd;
                        if ( rContentHd.nRecType == PPT_PST_BinaryTagData )
                        {
                            bRetValue = sal_True;
                            break;
                        }
                    }
                }
            }
            aProgTagBinaryDataHd.SeekToEndOfRecord( rSt );
        }
    }
    if ( !bRetValue )
        rSt.Seek( nOldPos );
    return bRetValue;
}

UINT32 SdrPowerPointImport::GetAktPageId()
{
    PptSlidePersistList* pList = GetPageList( eAktPageKind );
    if ( pList && nAktPageNum < pList->Count() )
        return (*pList)[ (USHORT)nAktPageNum ]->aPersistAtom.nSlideId;
    return 0;
}

sal_Bool SdrPowerPointImport::SeekToAktPage( DffRecordHeader* pRecHd ) const
{
    sal_Bool bRet = FALSE;
    PptSlidePersistList* pList = GetPageList( eAktPageKind );
    if ( pList && ( nAktPageNum < pList->Count() ) )
    {
        ULONG nPersist = (*pList)[ (USHORT)nAktPageNum ]->aPersistAtom.nPsrReference;
        if ( nPersist > 0 && nPersist < nPersistPtrAnz )
        {
            ULONG nFPos = 0;
            nFPos = pPersistPtr[ nPersist ];
            if ( nFPos < nStreamLen )
            {
                rStCtrl.Seek( nFPos );
                if ( pRecHd )
                    rStCtrl >> *pRecHd;
                bRet = TRUE;
            }
        }
    }
    return bRet;
}

USHORT SdrPowerPointImport::GetPageCount( PptPageKind ePageKind ) const
{
    PptSlidePersistList* pList = GetPageList( ePageKind );
    if ( pList )
        return pList->Count();
    return 0;
}

void SdrPowerPointImport::SetPageNum( sal_uInt16 nPageNum, PptPageKind eKind )
{
    eAktPageKind = eKind;
    nAktPageNum = nPageNum;

    pPPTStyleSheet = NULL;

    sal_Bool bHasMasterPage = sal_True;
    sal_uInt16 nMasterIndex = 0;

    if ( eKind == PPT_MASTERPAGE )
        nMasterIndex = nPageNum;
    else
    {
        if ( HasMasterPage( nPageNum, eKind ) )
            nMasterIndex = GetMasterPageIndex( nPageNum, eKind );
        else
            bHasMasterPage = sal_False;
    }
    if ( bHasMasterPage )
    {
        PptSlidePersistList* pPageList = GetPageList( PPT_MASTERPAGE );
        if ( pPageList && nMasterIndex < pPageList->Count() )
        {
            PptSlidePersistEntry* pMasterPersist = (*pPageList)[ nMasterIndex ];
            if ( ( pMasterPersist->pStyleSheet == NULL ) && pMasterPersist->aSlideAtom.nMasterId )
            {
                nMasterIndex = pMasterPages->FindPage( pMasterPersist->aSlideAtom.nMasterId );
                if ( nMasterIndex != PPTSLIDEPERSIST_ENTRY_NOTFOUND )
                    pMasterPersist = (*pPageList)[ nMasterIndex ];
            }
            pPPTStyleSheet = pMasterPersist->pStyleSheet;
         }
    }
    if ( !pPPTStyleSheet )
        pPPTStyleSheet = pDefaultSheet;
}

Size SdrPowerPointImport::GetPageSize() const
{
    Size aRet( IsNoteOrHandout( nAktPageNum, eAktPageKind ) ? aDocAtom.GetNotesPageSize() : aDocAtom.GetSlidesPageSize() );
    Scale( aRet );
    // PPT arbeitet nur mit Einheiten zu 576DPI. Um Ungenauigkeiten zu
    // vermeiden runde ich die letzte Nachkommastelle metrisch weg.
    if ( nMapMul > 2 * nMapDiv )
    {
        MapUnit eMap = pSdrModel->GetScaleUnit();
        sal_Bool bInch = IsInch( eMap );
        long nInchMul = 1, nInchDiv = 1;
        if ( bInch )
        {   // Size temporaer (zum runden) in nach metric konvertieren
            Fraction aFact(GetMapFactor(eMap,MAP_100TH_MM).X());
            nInchMul = aFact.GetNumerator();
            nInchDiv = aFact.GetDenominator();
            aRet.Width() = BigMulDiv( aRet.Width(), nInchMul, nInchDiv );
            aRet.Height() = BigMulDiv( aRet.Height(), nInchMul, nInchDiv );
        }
        aRet.Width() += 5; aRet.Width() /= 10; aRet.Width()*=10;
        aRet.Height() += 5; aRet.Height() /= 10; aRet.Height()*=10;
        if ( bInch )
        {
            aRet.Width() = BigMulDiv( aRet.Width(), nInchDiv, nInchMul );
            aRet.Height() = BigMulDiv( aRet.Height(), nInchDiv, nInchMul );
        }
    }
    return aRet;
}

FASTBOOL SdrPowerPointImport::GetColorFromPalette( USHORT nNum, Color& rColor ) const
{
    if ( nPageColorsNum != nAktPageNum || ePageColorsKind != eAktPageKind )
    {
        UINT16 nSlideFlags = 0;
        PptSlidePersistList* pPageList = GetPageList( eAktPageKind );
        if ( pPageList && ( nAktPageNum < pPageList->Count() ) )
        {
            PptSlidePersistEntry* pE = (*pPageList)[ nAktPageNum ];
            if ( pE )
                nSlideFlags = pE->aSlideAtom.nFlags;
            if ( ! ( nSlideFlags & 2 ) )
                ((SdrPowerPointImport*)this)->aPageColors = pE->aColorScheme;
        }
        if ( nSlideFlags & 2 )      // follow master colorscheme ?
        {
            PptSlidePersistList* pPageList = GetPageList( PPT_MASTERPAGE );
            if ( pPageList )
            {
                PptSlidePersistEntry* pMasterPersist = NULL;
                if ( eAktPageKind == PPT_MASTERPAGE )
                    pMasterPersist = (*pPageList)[ nAktPageNum ];
                else
                {
                    if ( HasMasterPage( nAktPageNum, eAktPageKind ) )
                    {
                        sal_uInt16 nMasterNum = GetMasterPageIndex( nAktPageNum, eAktPageKind );
                        if ( nMasterNum < pPageList->Count() )
                            pMasterPersist = (*pPageList)[ nMasterNum ];
                    }
                }
                if ( pMasterPersist )
                {
                    while( ( pMasterPersist && pMasterPersist->aSlideAtom.nFlags & 2 )  // it is possible that a masterpage
                        && pMasterPersist->aSlideAtom.nMasterId )                       // itself is following a master colorscheme
                    {
                        sal_uInt16 nNextMaster = pMasterPages->FindPage( pMasterPersist->aSlideAtom.nMasterId );
                        if ( nNextMaster == PPTSLIDEPERSIST_ENTRY_NOTFOUND )
                            break;
                        else
                            pMasterPersist = (*pPageList)[ nNextMaster ];
                    }
                    ((SdrPowerPointImport*)this)->aPageColors = pMasterPersist->aColorScheme;
                }
            }
        }
        // momentanes Farbschema eintragen
        ((SdrPowerPointImport*)this)->nPageColorsNum = nAktPageNum;
        ((SdrPowerPointImport*)this)->ePageColorsKind = eAktPageKind;
    }
    rColor = aPageColors.GetColor( nNum );
    return TRUE;
}

BOOL SdrPowerPointImport::SeekToShape( SvStream& rSt, void* pClientData, UINT32 nId ) const
{
    BOOL bRet = SvxMSDffManager::SeekToShape( rSt, pClientData, nId );
    if ( !bRet )
    {
        ProcessData& rData = *( (ProcessData*)pClientData );
        PptSlidePersistEntry& rPersistEntry = rData.rPersistEntry;
        if ( rPersistEntry.ePageKind == PPT_SLIDEPAGE )
        {
            if ( HasMasterPage( nAktPageNum, eAktPageKind ) )
            {
                USHORT nMasterNum = GetMasterPageIndex( nAktPageNum, eAktPageKind );
                PptSlidePersistList* pPageList = GetPageList( PPT_MASTERPAGE );
                if ( pPageList && ( nMasterNum < pPageList->Count() ) )
                {
                    PptSlidePersistEntry* pPersist = (*pPageList)[ nMasterNum ];    // get the masterpage's persistentry
                    if ( pPersist && pPersist->pPresentationObjects )
                    {
                        UINT32 nCurrent;
                        DffRecordList* pCList = maShapeRecords.pCList;              // we got a backup of the current position
                        if ( pCList )
                            nCurrent = pCList->nCurrent;
                        if ( ((SdrEscherImport*)this )->maShapeRecords.SeekToContent( rSt, DFF_msofbtClientData, SEEK_FROM_CURRENT_AND_RESTART ) )
                        {
                            UINT32 nStreamPos = rSt.Tell();
                            PPTTextObj aTextObj( rSt, (SdrPowerPointImport&)*this, rPersistEntry, NULL );
                            if ( aTextObj.Count() || aTextObj.GetOEPlaceHolderAtom() )
                            {
                                UINT32 nShapePos = 0;
                                switch ( aTextObj.GetInstance() )
                                {
                                    case TSS_TYPE_TITLE :
                                        nShapePos = pPersist->pPresentationObjects[ TSS_TYPE_PAGETITLE ];
                                    break;
                                    case TSS_TYPE_PAGETITLE :
                                        nShapePos = pPersist->pPresentationObjects[ TSS_TYPE_PAGETITLE ];
                                    break;
                                    case TSS_TYPE_SUBTITLE :
                                    case TSS_TYPE_HALFBODY :
                                    case TSS_TYPE_QUARTERBODY :
                                    case TSS_TYPE_BODY :
                                        nShapePos = pPersist->pPresentationObjects[ TSS_TYPE_BODY ];
                                    break;
//                                  case TSS_TYPE_NOTES :
//                                  case TSS_TYPE_UNUSED :
//                                  case TSS_TYPE_TEXT_IN_SHAPE :
                                }
                                if ( nShapePos )
                                {
                                    rSt.Seek( nShapePos );
                                    bRet = TRUE;
                                }
                            }
                            if ( !bRet )
                                rSt.Seek( nStreamPos );
                        }
                        if ( pCList )                                               // restoring
                            pCList->nCurrent = nCurrent;
                        ((SdrEscherImport*)this )->maShapeRecords.pCList = pCList;
                    }
                }
            }
        }
    }
    return bRet;
}

SdrPage* SdrPowerPointImport::MakeBlancPage( sal_Bool bMaster ) const
{
    SdrPage* pRet = pSdrModel->AllocPage( bMaster );
    pRet->SetSize( GetPageSize() );

/*
    SJ (21.08.00) : since bug #77576# i decided not to set a border size.

    Size aPageSize( aDocAtom.GetSlidesPageSize() ); // PageSize in 576DPI-Units
    long nHMarg = aPageSize.Width() - aDocAtom.aSlidesPageSize.Width();
    long nVMarg = aPageSize.Height() - aDocAtom.aSlidesPageSize.Height();
    if ( nHMarg > 0 )
    {
        Scale( nHMarg );
        pRet->SetLftBorder( nHMarg / 2 );
        pRet->SetRgtBorder( nHMarg - nHMarg / 2 );
    }
    if ( nVMarg > 0 )
    {
        Scale( nVMarg );
        pRet->SetUppBorder( nVMarg / 2 );
        pRet->SetLwrBorder( nVMarg - nVMarg / 2 );
    }
*/
    return pRet;
}


// be sure not to import masterpages with this method
// be sure not to import masterpages with this method
void SdrPowerPointImport::ImportPage( SdrPage* pRet, const PptSlidePersistEntry* pMasterPersist )
{
    UINT32 nMerk = rStCtrl.Tell();
    HeaderFooterEntry* pHFEM = NULL;

    PptSlidePersistList* pList = GetPageList( eAktPageKind );
    if ( ( !pList ) || ( pList->Count() <= nAktPageNum ) )
        return;
    PptSlidePersistEntry& rSlidePersist = *(*pList)[ nAktPageNum ];
    if ( rSlidePersist.bStarDrawFiller )
        return;

    if ( pMasterPersist )
        pHFEM = pMasterPersist->pHeaderFooterEntry; // get the masterpage's HeaderFooterEntry

    DffRecordHeader aPageHd;
    if ( SeekToAktPage( &aPageHd ) )
    {
        if ( mbTracing )
            mpTracer->AddAttribute( eAktPageKind == PPT_SLIDEPAGE
                                    ? rtl::OUString::createFromAscii( "Page" )
                                    : rtl::OUString::createFromAscii( "NotesPage" ),
                                    rtl::OUString::valueOf( (sal_Int32)nAktPageNum + 1 ) );
        HeaderFooterEntry* pHFE = NULL;
        if ( pHFEM )
        {
            pHFE = new HeaderFooterEntry( *pHFEM, (SdPage*)pRet );
            rSlidePersist.pHeaderFooterEntry = pHFE;
            if ( ( rSlidePersist.aSlideAtom.aLayout.eLayout == PPT_LAYOUT_TITLESLIDE )
                    && ( aDocAtom.bTitlePlaceholdersOmitted == TRUE ) )
            {
                pHFE->rMaster.nDirtyInstance = 0xf;
                pHFE->nAtom = 0;
            }
        }
        ProcessData aProcessData( rSlidePersist, (SdPage*)pRet );
        while ( ( rStCtrl.GetError() == 0 ) && ( rStCtrl.Tell() < aPageHd.GetRecEndFilePos() ) )
        {
            DffRecordHeader aHd;
            rStCtrl >> aHd;
            switch ( aHd.nRecType )
            {
                case PPT_PST_HeadersFooters :
                {
                    if ( pHFE )
                    {
                        ImportHeaderFooterContainer( aHd, *pHFE );
                        for ( UINT32 i = 0; i < 4; i++ )
                        {
                            UINT32 nPosition = pHFE->rMaster.NeedToImportInstance( i, *pHFE );
                            if ( nPosition )
                            {
                                rStCtrl.Seek( nPosition );
                                SdrObject* pObj = ImportObj( rStCtrl, (void*)&aProcessData, NULL );
                                if ( pObj )
                                {   // cause of this object is already dirty, we can inserted it directly
                                    pHFE->nAtom &= ~pHFE->GetMaskForInstance( i );
                                    pRet->NbcInsertObject( pObj );
                                }
                            }
                        }
                    }
                }
                break;

                case PPT_PST_PPDrawing :
                {
                    DffRecordHeader aPPDrawHd;
                    if ( SeekToRec( rStCtrl, DFF_msofbtDgContainer, aHd.GetRecEndFilePos(), &aPPDrawHd ) )
                    {
                        sal_uInt32 nPPDrawOfs = rStCtrl.Tell();

                        // importing the background object before importing the page
                        while ( ( rStCtrl.GetError() == 0 ) && ( rStCtrl.Tell() < aPPDrawHd.GetRecEndFilePos() ) )
                        {
                            DffRecordHeader aEscherObjListHd;
                            rStCtrl >> aEscherObjListHd;
                            switch ( aEscherObjListHd.nRecType )
                            {
                                case DFF_msofbtSpContainer :
                                {
                                    if ( rSlidePersist.aSlideAtom.nFlags & 4 )          // follow master background ?
                                    {
                                        if ( HasMasterPage( nAktPageNum, eAktPageKind ) )
                                        {
                                            sal_uInt16 nMasterNum = GetMasterPageIndex( nAktPageNum, eAktPageKind );
                                            PptSlidePersistList* pPageList = GetPageList( PPT_MASTERPAGE );
                                            PptSlidePersistEntry* pE = (*pPageList)[ nMasterNum ];
                                            while( ( pE->aSlideAtom.nFlags & 4 ) && pE->aSlideAtom.nMasterId )
                                            {
                                                sal_uInt16 nNextMaster = pMasterPages->FindPage( pE->aSlideAtom.nMasterId );
                                                if ( nNextMaster == PPTSLIDEPERSIST_ENTRY_NOTFOUND )
                                                    break;
                                                else
                                                    pE = (*pPageList)[ nNextMaster ];
                                            }
                                            if ( pE->nBackgroundOffset )
                                            {
                                                // do not follow master colorscheme ?
                                                sal_Bool bTemporary = ( rSlidePersist.aSlideAtom.nFlags & 2 ) != 0;
                                                sal_uInt32 nPos = rStCtrl.Tell();
                                                rStCtrl.Seek( pE->nBackgroundOffset );
                                                rSlidePersist.pBObj = ImportObj( rStCtrl, (void*)&aProcessData, NULL );
                                                rSlidePersist.bBObjIsTemporary = bTemporary;
                                                rStCtrl.Seek( nPos );
                                            }
                                        }
                                    }
                                    else
                                    {
                                        DffRecordHeader aShapeHd;
                                        rStCtrl >> aShapeHd;
                                        if ( aShapeHd.nRecType == DFF_msofbtSp )
                                        {
                                            UINT32 nSpFlags;
                                            rStCtrl >> nSpFlags >> nSpFlags;
                                            if ( nSpFlags & SP_FBACKGROUND )
                                            {
                                                aEscherObjListHd.SeekToBegOfRecord( rStCtrl );
                                                rSlidePersist.pBObj = ImportObj( rStCtrl, (void*)&aProcessData, NULL );
                                                rSlidePersist.bBObjIsTemporary = sal_False;
                                            }
                                        }
                                    }
                                }
                                break;
                            }
                            if ( aEscherObjListHd.nRecType == DFF_msofbtSpContainer )
                                break;
                            aEscherObjListHd.SeekToEndOfRecord( rStCtrl );
                        }

                        // now importing page
                        rStCtrl.Seek( nPPDrawOfs );
                        while ( ( rStCtrl.GetError() == 0 ) && ( rStCtrl.Tell() < aPPDrawHd.GetRecEndFilePos() ) )
                        {
                            DffRecordHeader aEscherObjListHd;
                            rStCtrl >> aEscherObjListHd;
                            switch ( aEscherObjListHd.nRecType )
                            {
                                case DFF_msofbtSpgrContainer :
                                {
                                    DffRecordHeader aShapeHd;
                                    if ( SeekToRec( rStCtrl, DFF_msofbtSpContainer, aEscherObjListHd.GetRecEndFilePos(), &aShapeHd ) )
                                    {
                                        aShapeHd.SeekToEndOfRecord( rStCtrl );
                                        while ( ( rStCtrl.GetError() == 0 ) && ( rStCtrl.Tell() < aEscherObjListHd.GetRecEndFilePos() ) )
                                        {
                                            rStCtrl >> aShapeHd;
                                            if ( ( aShapeHd.nRecType == DFF_msofbtSpContainer ) || ( aShapeHd.nRecType == DFF_msofbtSpgrContainer ) )
                                            {
                                                aShapeHd.SeekToBegOfRecord( rStCtrl );
                                                aProcessData.nGroupingFlags = 0;
                                                SdrObject* pObj = ImportObj( rStCtrl, (void*)&aProcessData, NULL );
                                                if ( pObj )
                                                {
                                                    // maybe this is an animated textobj
                                                    if ( aProcessData.nGroupingFlags && pObj->ISA( SdrObjGroup ) )
                                                    {
                                                        SdrObjList* pObjectList = ((SdrObjGroup*)pObj)->GetSubList();
                                                        if ( pObjectList )
                                                        {
                                                            if ( pObjectList->GetObjCount() == 2 )
                                                            {
                                                                pRet->NbcInsertObject( pObjectList->NbcRemoveObject( 0 ) );
                                                                SdrObject* pTemp = pObjectList->NbcRemoveObject( 0 );
                                                                delete pObj;
                                                                pObj = pTemp;
                                                            }
                                                        }
                                                    }
                                                    pRet->NbcInsertObject( pObj );
                                                }
                                            }
                                            aShapeHd.SeekToEndOfRecord( rStCtrl );
                                        }
                                    }
                                }
                                break;
                            }
                            if ( aEscherObjListHd.nRecType == DFF_msofbtSpgrContainer )
                                break;
                            aEscherObjListHd.SeekToEndOfRecord( rStCtrl );
                        }

                        /* There are a lot of Shapes who are dependent to
                           the current background color */
                        if ( rSlidePersist.ePageKind == PPT_SLIDEPAGE )
                        {
                            List* pList = aProcessData.pBackgroundColoredObjects;
                            if ( pList )
                            {
                                if ( rSlidePersist.pBObj )
                                {
                                    void* pPtr;
                                    const SfxPoolItem* pPoolItem = NULL;

                                    SfxItemState eState = rSlidePersist.pBObj->GetItemSet()
                                        .GetItemState( XATTR_FILLCOLOR, FALSE, &pPoolItem );
                                    if ( pPoolItem )
                                    {
                                        for ( pPtr = pList->First(); pPtr; pPtr = pList->Next() )
                                        {
                                            ((SdrObject*)pPtr)->SetItem( *pPoolItem );
                                            ((SdrObject*)pPtr)->SetItem( XFillStyleItem( XFILL_SOLID ) );
                                        }
                                    }
                                }
                            }
                        }
                        if ( rSlidePersist.pBObj )
                        {
                            if ( rSlidePersist.bBObjIsTemporary )
                                delete rSlidePersist.pBObj;
                            else
                                pRet->SetBackgroundObj( rSlidePersist.pBObj );
                        }
                    }
                }
                break;
            }
            aHd.SeekToEndOfRecord( rStCtrl );
        }
        if ( rSlidePersist.pSolverContainer )
            SolveSolver( *rSlidePersist.pSolverContainer );
        if ( mbTracing )
            mpTracer->RemoveAttribute( eAktPageKind == PPT_SLIDEPAGE
                                        ? rtl::OUString::createFromAscii( "Page" )
                                        : rtl::OUString::createFromAscii( "NotesPage" ) );
    }
    rStCtrl.Seek( nMerk );
}

const PptSlideLayoutAtom* SdrPowerPointImport::GetSlideLayoutAtom() const
{
    PptSlidePersistList* pPageList = GetPageList( eAktPageKind );
    if ( pPageList && nAktPageNum < pPageList->Count() )
    {
        PptSlidePersistEntry* pE = (*pPageList)[ nAktPageNum ];
        if ( pE )
            return &pE->aSlideAtom.aLayout;
    }
    return NULL;
}

sal_Bool SdrPowerPointImport::IsNoteOrHandout( USHORT nPageNum, PptPageKind ePageKind ) const
{
    sal_Bool bNote = eAktPageKind == PPT_NOTEPAGE;
    if ( eAktPageKind == PPT_MASTERPAGE )
        bNote = ( nPageNum & 1 ) == 0;
    return bNote;
}

UINT32 SdrPowerPointImport::GetMasterPageId( USHORT nPageNum, PptPageKind ePageKind ) const
{
    PptSlidePersistList* pPageList = GetPageList( ePageKind );
    if ( pPageList && nPageNum < pPageList->Count() )
        return (*pPageList)[ nPageNum ]->aSlideAtom.nMasterId;
   return 0;
}

UINT32 SdrPowerPointImport::GetNotesPageId( USHORT nPageNum ) const
{
    PptSlidePersistList* pPageList=GetPageList( PPT_SLIDEPAGE );
    if ( pPageList && nPageNum < pPageList->Count() )
        return (*pPageList)[ nPageNum ]->aSlideAtom.nNotesId;
   return 0;
}

sal_Bool SdrPowerPointImport::HasMasterPage( USHORT nPageNum, PptPageKind ePageKind ) const
{
    if ( ePageKind == PPT_NOTEPAGE )
        return aDocAtom.nNotesMasterPersist != 0;
    if ( ePageKind == PPT_MASTERPAGE )
        return FALSE;
    return GetMasterPageId( nPageNum, ePageKind ) != 0;
}

USHORT SdrPowerPointImport::GetMasterPageIndex( USHORT nPageNum, PptPageKind ePageKind ) const
{
    USHORT nIdx = 0;
    if ( ePageKind == PPT_NOTEPAGE )
        return 2;
    UINT32 nId = GetMasterPageId( nPageNum, ePageKind );
    if ( nId && pMasterPages )
    {
        nIdx = pMasterPages->FindPage( nId );
        if ( nIdx == PPTSLIDEPERSIST_ENTRY_NOTFOUND )
            nIdx = 0;
    }
    return nIdx;
}

SdrObject* SdrPowerPointImport::ImportPageBackgroundObject( const SdrPage& rPage, sal_uInt32& nBgFileOffset, sal_Bool bForce )
{
    SdrObject* pRet = NULL;
    sal_Bool bCreateObj = bForce;
    SfxItemSet* pSet = NULL;
    ULONG nFPosMerk = rStCtrl.Tell(); // FilePos merken fuer spaetere Restauration
    DffRecordHeader aPageHd;
    if ( SeekToAktPage( &aPageHd ) )
    {   // und nun die Hintergrundattribute der Page suchen
        ULONG nPageRecEnd = aPageHd.GetRecEndFilePos();
        DffRecordHeader aPPDrawHd;
        if ( SeekToRec( rStCtrl, PPT_PST_PPDrawing, nPageRecEnd, &aPPDrawHd ) )
        {
            ULONG nPPDrawEnd = aPPDrawHd.GetRecEndFilePos();
            DffRecordHeader aEscherF002Hd;
            if ( SeekToRec( rStCtrl, DFF_msofbtDgContainer, nPPDrawEnd, &aEscherF002Hd ) )
            {
                ULONG nEscherF002End = aEscherF002Hd.GetRecEndFilePos();
                DffRecordHeader aEscherObjectHd;
                if ( SeekToRec( rStCtrl, DFF_msofbtSpContainer, nEscherF002End, &aEscherObjectHd ) )
                {
                    nBgFileOffset = aEscherObjectHd.GetRecBegFilePos();
                    ULONG nEscherObjectEnd = aEscherObjectHd.GetRecEndFilePos();
                    //DffRecordHeader aEscherPropertiesHd;
                    if ( SeekToRec( rStCtrl, DFF_msofbtOPT,nEscherF002End ) )
                    {
                        rStCtrl >> (DffPropertyReader&)*this;
                        mnFix16Angle = Fix16ToAngle( GetPropertyValue( DFF_Prop_Rotation, 0 ) );
                        UINT32 nColor = GetPropertyValue( DFF_Prop_fillColor, 0xffffff );
                        pSet = new SfxItemSet( pSdrModel->GetItemPool() );
                        ApplyAttributes( rStCtrl, *pSet );
                        Color aColor( MSO_CLR_ToColor( nColor ) );
                        pSet->Put( XFillColorItem( String(), aColor ) );
                    }
                }
            }
        }
    }
    rStCtrl.Seek( nFPosMerk ); // FilePos restaurieren
    if ( bCreateObj )
    {
        if ( !pSet )
        {
            pSet = new SfxItemSet( pSdrModel->GetItemPool() );
            pSet->Put( XFillStyleItem( XFILL_NONE ) );
        }
        pSet->Put( XLineStyleItem( XLINE_NONE ) );
        Rectangle aRect( rPage.GetLftBorder(), rPage.GetUppBorder(), rPage.GetWdt()-rPage.GetRgtBorder(), rPage.GetHgt()-rPage.GetLwrBorder() );
        pRet = new SdrRectObj( aRect );
        pRet->SetModel( pSdrModel );

        pRet->SetItemSet(*pSet);

        pRet->SetMarkProtect( TRUE );
        pRet->SetMoveProtect( TRUE );
        pRet->SetResizeProtect( TRUE );
    }
    delete pSet;
    return pRet;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

HeaderFooterEntry::HeaderFooterEntry( HeaderFooterMaster& rM, SdPage* pPage ) :
    rMaster         ( rM ),
    pSdPage         ( pPage ),
    nAtom           ( 0 ),
    nInstanceCount  ( 0 ),
    bMaster         ( TRUE )
{
    for ( UINT32 i = 0; i < 4; i++ )
    {
        pSdrObject[ i ] = NULL;
    }
}

HeaderFooterEntry::HeaderFooterEntry( HeaderFooterEntry& rE, SdPage* pPage ) :
    rMaster         ( rE.rMaster ),
    pSdPage         ( pPage ),
    nAtom           ( rE.nAtom ),
    nInstanceCount  ( rE.nInstanceCount ),
    bMaster         ( FALSE )
{
    for ( UINT32 i = 0; i < 4; i++ )
    {
        if ( i < nInstanceCount )
            pInstanceOrder[ i ] = rE.pInstanceOrder[ i ];
        pPlaceholder[ i ] = rE.pPlaceholder[ i ];
        pSdrObject[ i ] = NULL;
    }
    rMaster.aHeaderFooterEntryList.Insert( (void*)this, LIST_APPEND );
}

HeaderFooterEntry::~HeaderFooterEntry()
{
    for ( UINT32 i = 0; i < 4; delete pSdrObject[ i++ ] );
}

UINT32 HeaderFooterEntry::GetMaskForInstance( UINT32 nInstance )
{
    UINT32 nRet = 0;
    switch ( nInstance )
    {
        case 0 : nRet = 0x07ffff; break;
        case 1 : nRet = 0x100000; break;
        case 2 : nRet = 0x200000; break;
        case 3 : nRet = 0x080000; break;
    }
    return nRet;
}

UINT32 HeaderFooterEntry::IsToDisplay( UINT32 nInstance )
{
    UINT32 nMask = 0;
    switch ( nInstance )
    {
        case 0 : nMask = 0x010000; break;
        case 1 : nMask = 0x100000; break;
        case 2 : nMask = 0x200000; break;
        case 3 : nMask = 0x080000; break;
    }
    return ( nAtom & nMask );
}

HeaderFooterMaster::HeaderFooterMaster() :
    nDirtyInstance  ( 0 )
{
    for ( UINT32 i = 0; i < 4; i++ )
    {
        pOfs[ i ] = 0;
     }
}

HeaderFooterMaster::~HeaderFooterMaster()
{
    for ( void* pPtr = aHeaderFooterEntryList.First(); pPtr; pPtr = aHeaderFooterEntryList.Next() )
        delete (HeaderFooterEntry*)pPtr;
}

UINT32 HeaderFooterMaster::NeedToImportInstance( UINT32 nInstance, HeaderFooterEntry& rE )
{
    UINT32 nRet = 0;
    UINT32 nCount = aHeaderFooterEntryList.Count();
    if ( nCount > 1 )
    {
        HeaderFooterEntry& rEM = *( (HeaderFooterEntry*)aHeaderFooterEntryList.First() );
        BOOL bPlaceholderChanged = rEM.pPlaceholder[ nInstance ] != rE.pPlaceholder[ nInstance ];
        UINT32 nMask = rE.GetMaskForInstance( nInstance );
        UINT32 nInstMask = 1 << nInstance;
        if ( ( rEM.nAtom & nMask ) != ( rE.nAtom & nMask ) )
            nDirtyInstance |= nInstMask;
        if ( bPlaceholderChanged )
            nDirtyInstance |= nInstMask;
        if  ( nDirtyInstance & nInstMask )
        {
            if ( rE.IsToDisplay( nInstance ) )
                nRet = pOfs[ nInstance ];
        }
    }
    return nRet;
}

void SdrEscherImport::ImportHeaderFooterContainer( DffRecordHeader& rHd, HeaderFooterEntry& rE )
{
    rHd.SeekToContent( rStCtrl );
    while ( ( rStCtrl.GetError() == 0 ) && ( rStCtrl.Tell() < rHd.GetRecEndFilePos() ) )
    {
        DffRecordHeader aHd;
        rStCtrl >> aHd;
        switch ( aHd.nRecType )
        {
            case PPT_PST_HeadersFootersAtom :
                rStCtrl >> rE.nAtom;
            break;

            case PPT_PST_CString :
            {
                if ( aHd.nRecInstance < 4 )
                    MSDFFReadZString( rStCtrl, rE.pPlaceholder[ aHd.nRecInstance ], aHd.nRecLen, TRUE );
            }
            break;
        }
        aHd.SeekToEndOfRecord( rStCtrl );
    }
}

static sal_Unicode PPTExportMapper( sal_Unicode nUni, BOOL& bNeedsStarBats )
{
    bNeedsStarBats = FALSE;

    sal_Unicode cLo, cReplace;
    cLo = cReplace = 0;
    switch ( nUni )
    {
        case 132 : cLo = 175; break;
        case 147 : cLo = 174; break;
        // Currency
        case 0x00A2:    cReplace = 224; break;
        case 0x00A4:    cReplace = 225; break;
        case 0x00A5:    cReplace = 226; break;
        case 0x20A1:    cReplace = 228; break;
        case 0x20A2:    cReplace = 229; break;
        case 0x20A3:    cReplace = 230; break;
        case 0x20A4:    cReplace = 231; break;
        case 0x20A7:    cReplace = 227; break;
        case 0x20A8:    cReplace = 234; break;
        case 0x20A9:    cReplace = 232; break;
        case 0x20AB:    cReplace = 233; break;
        case 0x20AC:    cReplace = 128; break;
        // Punctuation and other
        case 0x201A:    cReplace = 130; break;      // SINGLE LOW-9 QUOTATION MARK
        case 0x0192:    cReplace = 131; break;      // LATIN SMALL LETTER F WITH HOOK
        case 0x201E:                                // DOUBLE LOW-9 QUOTATION MARK
        case 0x301F:                                // LOW DOUBLE PRIME QUOTATION MARK
                        cReplace = 132; break;
        case 0x2026:    cReplace = 133; break;      // HORIZONTAL ELLIPSES
        case 0x2020:    cReplace = 134; break;      // DAGGER
        case 0x2021:    cReplace = 135; break;      // DOUBLE DAGGER
        case 0x02C6:    cReplace = 136; break;      // MODIFIER LETTER CIRCUMFLEX ACCENT
        case 0x2030:    cReplace = 137; break;      // PER MILLE SIGN
        case 0x0160:    cReplace = 138; break;      // LATIN CAPITAL LETTER S WITH CARON
        case 0x2039:    cReplace = 139; break;      // SINGLE LEFT-POINTING ANGLE QUOTATION MARK
        case 0x0152:    cReplace = 140; break;      // LATIN CAPITAL LIGATURE OE
        case 0x017D:    cReplace = 142; break;      // LATIN CAPITAL LETTER Z WITH CARON
        case 0x2018:                                // LEFT SINGLE QUOTATION MARK
        case 0x02BB:                                // MODIFIER LETTER TURNED COMMA
                        cReplace = 145; break;
        case 0x2019:                                // RIGHT SINGLE QUOTATION MARK
        case 0x02BC:                                // MODIFIER LETTER APOSTROPHE
                        cReplace = 146; break;
        case 0x201C:                                // LEFT DOUBLE QUOTATION MARK
        case 0x301D:                                // REVERSED DOUBLE PRIME QUOTATION MARK
                        cReplace = 147; break;
        case 0x201D:                                // RIGHT DOUBLE QUOTATION MARK
        case 0x301E:                                // REVERSED DOUBLE PRIME QUOTATION MARK
                        cReplace = 148; break;
        case 0x2022:    cReplace = 149; break;      // BULLET
        case 0x2013:    cReplace = 150; break;      // EN DASH
        case 0x2014:    cReplace = 151; break;      // EM DASH
        case 0x02DC:    cReplace = 152; break;      // SMALL TILDE
        case 0x2122:    cReplace = 153; break;      // TRADE MARK SIGN
        case 0x0161:    cReplace = 154; break;      // LATIN SMALL LETTER S WITH CARON
        case 0x203A:    cReplace = 155; break;      // SINGLE RIGHT-POINTING ANGLE QUOTATION MARK
        case 0x0153:    cReplace = 156; break;      // LATIN SMALL LIGATURE OE
        case 0x017E:    cReplace = 158; break;      // LATIN SMALL LETTER Z WITH CARON
        case 0x0178:    cReplace = 159; break;      // LATIN CAPITAL LETTER Y WITH DIAERESIS
        case 0x00B6:    cReplace = 222; break;      // PILCROW SIGN / PARAGRAPH SIGN
    }
    if ( cReplace )
    {
        bNeedsStarBats = TRUE;
        return cReplace;
    }
    else
        return cLo;
}

// no longer needed
sal_Unicode SdrPowerPointImport::PPTSubstitute( UINT16 nFont, sal_Unicode nChar,
                                        UINT32& nMappedFontId, Font& rFont, char nDefault ) const
{
    return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

PPTBuGraEntry::PPTBuGraEntry( Graphic& rGraphic, UINT32 nInst ) :
    aBuGra          ( rGraphic ),
    nInstance       ( nInst ) {};

PPTExtParaLevel::PPTExtParaLevel() :
    nBuFlags        ( 0 ),
    nBuStart        ( 0 ),
    nBuInstance     ( 0xffff ),
    nNumberingType  ( 0x30001 ),
    bSet            ( FALSE )
{}

SvStream& operator>>( SvStream& rIn, PPTExtParaLevel& rLevel )
{
    rLevel.bSet = TRUE;
    rIn >> rLevel.nBuFlags;
    if ( rLevel.nBuFlags & 0x00800000 )
        rIn >> rLevel.nBuInstance;
    if ( rLevel.nBuFlags & 0x01000000 )
        rIn >> rLevel.nNumberingType;
    if ( rLevel.nBuFlags & 0x02000000 )
        rIn >> rLevel.nBuStart;
#ifdef DBG_UTIL
    UINT32 nTemp;
    rIn >> nTemp;
    if ( nTemp )
        DBG_ERROR( ">>PPTExtParaLevel - undefined value (SJ)" );
#else
    rIn.SeekRel( 4 );
#endif
    return rIn;
}

BOOL PPTExtParaProv::GetGraphic( UINT32 nInstance, Graphic& rGraph ) const
{
    BOOL bRetValue = FALSE;
    PPTBuGraEntry* pPtr;
    if ( nInstance < aBuGraList.Count() )
    {
        pPtr = (PPTBuGraEntry*)aBuGraList.GetObject( nInstance );
        if ( pPtr->nInstance == nInstance )
            bRetValue = TRUE;
    }
    if ( !bRetValue )
    {
        sal_uInt32 i;
        for ( i = 0; i < aBuGraList.Count(); i++ )
        {
            pPtr = (PPTBuGraEntry*)aBuGraList.GetObject( i );
            if ( pPtr->nInstance == nInstance )
            {
                bRetValue = TRUE;
                break;
            }
        }
    }
    if ( bRetValue )
        rGraph = pPtr->aBuGra;
    return bRetValue;
}

PPTExtParaProv::PPTExtParaProv( SdrPowerPointImport& rMan, SvStream& rSt, const DffRecordHeader* pHd ) :
    bStyles         ( FALSE ),
    bGraphics       ( FALSE )
{
    UINT32 nOldPos = rSt.Tell();

    // here we have to get the graphical bullets...

    DffRecordHeader aHd;
    DffRecordHeader aContentDataHd;

    const DffRecordHeader* pListHd = rMan.aDocRecManager.GetRecordHeader( PPT_PST_List, SEEK_FROM_BEGINNING );
    while( pListHd )
    {
        pListHd->SeekToContent( rSt );
        if ( !rMan.SeekToContentOfProgTag( 9, rSt, *pListHd, aContentDataHd ) )
            break;
        while ( ( rSt.GetError() == 0 ) && ( rSt.Tell() < aContentDataHd.GetRecEndFilePos() ) )
        {
            rSt >> aHd;
            switch ( aHd.nRecType )
            {
                case PPT_PST_ExtendedBuGraContainer :
                {
                    while ( ( rSt.GetError() == 0 ) && ( rSt.Tell() < aHd.GetRecEndFilePos() ) )
                    {
                        UINT16 nType;
                        DffRecordHeader aBuGraAtomHd;
                        rSt >> aBuGraAtomHd;
                        if ( aBuGraAtomHd.nRecType == PPT_PST_ExtendedBuGraAtom )
                        {
                            rSt >> nType;
                            Graphic aGraphic;
                            if ( rMan.GetBLIPDirect( rSt, aGraphic ) )
                            {
                                UINT32 nInstance = aBuGraAtomHd.nRecInstance;
                                PPTBuGraEntry* pBuGra = new PPTBuGraEntry( aGraphic, nInstance );
                                UINT32 n = 0;
                                UINT32 nBuGraCount = aBuGraList.Count();
                                if ( nBuGraCount )
                                {
                                    if ( ( (PPTBuGraEntry*)aBuGraList.GetObject( nBuGraCount - 1 ) )->nInstance < nInstance )
                                        n = nBuGraCount;
                                    else
                                    {   // maybe the instances are not sorted, we sort it
                                        for ( n = 0; n < nBuGraCount; n++ )
                                        {   // sorting fields ( hi >> lo )
                                            if ( ( (PPTBuGraEntry*)aBuGraList.GetObject( n ) )->nInstance < nInstance )
                                                break;
                                        }
                                    }
                                }
                                aBuGraList.Insert( pBuGra, (UINT32)n );
                            }
#ifdef DBG_UTIL
                            else DBG_ERROR( "PPTExParaProv::PPTExParaProv - bullet graphic is not valid (SJ)" );
#endif
                        }
#ifdef DBG_UTIL
                        else DBG_ERROR( "PPTExParaProv::PPTExParaProv - unknown atom interpreting the PPT_PST_ExtendedBuGraContainer (SJ)" );
#endif
                        aBuGraAtomHd.SeekToEndOfRecord( rSt );
                    }
                    if ( aBuGraList.Count() )
                        bGraphics = TRUE;
                }
                break;

                case PPT_PST_ExtendedPresRuleContainer :
                    aExtendedPresRules.Consume( rSt, FALSE, aHd.GetRecEndFilePos() );
                break;
#ifdef DBG_UTIL
                default :
                    DBG_ERROR( "PPTExParaProv::PPTExParaProv - unknown atom reading ppt2000 num rules (SJ)" );
                case PPT_PST_MasterText :   // first seen in: ms-tt02.ppt
                case PPT_PST_SrKinsoku :
                case PPT_PST_NewlyAddedAtom4016 :
                case PPT_PST_NewlyAddedAtomByPPT2000_6010 :
                case PPT_PST_NewlyAddedAtomByPPT2000_6011 :
                case PPT_PST_NewlyAddedAtomByXP1037 :
                case PPT_PST_NewlyAddedAtomByXP12004 :
                case PPT_PST_NewlyAddedAtomByXP14001 :
                break;
#endif
            }
            aHd.SeekToEndOfRecord( rSt );
        }
        break;
    }

    while( pHd )
    {   // get the extended paragraph styles on mainmaster ( graphical bullets, num ruling ... )
        if ( !rMan.SeekToContentOfProgTag( 9, rSt, *pHd, aContentDataHd ) )
            break;
        while ( ( rSt.GetError() == 0 ) && ( rSt.Tell() < aContentDataHd.GetRecEndFilePos() ) )
        {
            rSt >> aHd;
            switch ( aHd.nRecType )
            {
                case PPT_PST_ExtendedParagraphMasterAtom :
                {
                    if ( aHd.nRecInstance < PPT_STYLESHEETENTRYS )
                    {
                        UINT16 nDepth, i = 0;
                        rSt >> nDepth;
                        if ( i <= 5 )
                        {

                            while ( ( rSt.GetError() == 0 ) && ( rSt.Tell() < aHd.GetRecEndFilePos() ) && ( i < nDepth ) )
                            {
                                bStyles = TRUE;
                                rSt >> aExtParaSheet[ aHd.nRecInstance ].aExtParaLevel[ i++ ];
                            }
#ifdef DBG_UTIL
                            if ( rSt.Tell() != aHd.GetRecEndFilePos() )
                                DBG_ERROR( "PPTExParaProv::PPTExParaProv - error reading PPT_PST_ExtendedParagraphMasterAtom (SJ)" );
#endif
                        }
#ifdef DBG_UTIL
                        else DBG_ERROR( "PPTExParaProv::PPTExParaProv - depth is greater than 5 (SJ)" );
#endif
                    }
#ifdef DBG_UTIL
                    else DBG_ERROR( "PPTExParaProv::PPTExParaProv - instance out of range (SJ)" );
#endif
                }
                break;
                default :
                    DBG_ERROR( "PPTExParaProv::PPTExParaProv - unknown atom, assuming PPT_PST_ExtendedParagraphMasterAtom (SJ)" );
                case PPT_PST_NewlyAddedAtomByXP11008 :
                case PPT_PST_NewlyAddedAtomByXP11010 :
                case PPT_PST_NewlyAddedAtomByXP12010 :
                case PPT_PST_NewlyAddedAtomByXP12011 :
                case 0xf144 :
                break;
            }
            aHd.SeekToEndOfRecord( rSt );
        }
        break;
    }
    rSt.Seek( nOldPos );
}

PPTExtParaProv::~PPTExtParaProv()
{
    void* pPtr;
    for ( pPtr = aBuGraList.First(); pPtr; pPtr = aBuGraList.Next() )
        delete (PPTBuGraEntry*)pPtr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

PPTNumberFormatCreator::PPTNumberFormatCreator( PPTExtParaProv* pParaProv ) :
    pExtParaProv ( pParaProv )
{
}

PPTNumberFormatCreator::~PPTNumberFormatCreator()
{
    delete pExtParaProv;
}

BOOL PPTNumberFormatCreator::ImplGetExtNumberFormat( SdrPowerPointImport& rManager,
    SvxNumberFormat& rNumberFormat, UINT32 nLevel, UINT32 nInstance, UINT32 nInstanceInSheet,
        UINT32 nFontHeight, PPTParagraphObj* pPara )
{
    BOOL bHardAttribute = ( nInstanceInSheet == 0xffffffff );

    UINT32  nBuFlags = 0;
    UINT16  nBuStart = 0;
    UINT32  nNumberingType = 0x30001;
    UINT16  nBuInstance = 0xffff;

    const PPTExtParaProv* pParaProv = pExtParaProv;
    if ( !pExtParaProv )
        pParaProv = ( pPara ) ? pPara->mrStyleSheet.pExtParaProv
                              : rManager.pPPTStyleSheet->pExtParaProv;
    if ( pPara )
    {
        nBuFlags = pPara->pParaSet->nBuFlags;
        if ( nBuFlags )
        {
            if ( nBuFlags & 0x00800000 )
                nBuInstance = pPara->pParaSet->nBuInstance;
            if ( nBuFlags & 0x01000000 )
                nNumberingType = pPara->pParaSet->nNumberingType;
            if ( nBuFlags & 0x02000000 )
                nBuStart = pPara->pParaSet->nBuStart;
            bHardAttribute = TRUE;
        }
    }
    if ( ( nBuFlags & 0x03800000 ) != 0x03800000 )  // merge style sheet
    {   // we have to read the master attributes
        if ( pParaProv && ( nLevel < 5 ) )
        {
            if ( pParaProv->bStyles )
            {
                const PPTExtParaLevel& rLev = pParaProv->aExtParaSheet[ nInstance ].aExtParaLevel[ nLevel ];
                if ( rLev.bSet )
                {
                    UINT32 nMaBuFlags = rLev.nBuFlags;

                    if ( (!( nBuFlags & 0x00800000)) && ( nMaBuFlags & 0x00800000 ) )
                    {
                        if (!( nBuFlags & 0x02000000))          // if there is a BuStart without BuInstance,
                            nBuInstance = rLev.nBuInstance;     // then there is no graphical Bullet possible
                    }
                    if ( (!( nBuFlags & 0x01000000)) && ( nMaBuFlags & 0x01000000 ) )
                        nNumberingType = rLev.nNumberingType;
                    if ( (!( nBuFlags & 0x02000000)) && ( nMaBuFlags & 0x02000000 ) )
                        nBuStart = rLev.nBuStart;
                    nBuFlags |= nMaBuFlags;
                }
            }
        }
    }
    if ( nBuInstance != 0xffff )        // set graphical bullet
    {
        Graphic aGraphic;
        if ( pParaProv->GetGraphic( nBuInstance, aGraphic ) )
        {
            SvxBrushItem aBrush( aGraphic, GPOS_MM );
            rNumberFormat.SetGraphicBrush( &aBrush );
            sal_uInt32 nHeight = (sal_uInt32)( (double)nFontHeight * 0.2540 * nBulletHeight + 0.5 );
            Size aPrefSize( aGraphic.GetPrefSize() );
            UINT32 nWidth = ( nHeight * aPrefSize.Width() ) / aPrefSize.Height();
            rNumberFormat.SetGraphicSize( Size( nWidth, nHeight ) );
            rNumberFormat.SetNumberingType ( SVX_NUM_BITMAP );
        }
    }
    else if ( nBuStart && ( nBuFlags & 0x03000000 ) )
    {
        if ( nNumberingType & 1 )
        {
            switch( nNumberingType >> 16 )
            {
                default :
                case 0 :
                {
                    rNumberFormat.SetNumberingType( SVX_NUM_CHARS_LOWER_LETTER );
                    rNumberFormat.SetSuffix( String( RTL_CONSTASCII_USTRINGPARAM( "." ) ) );
                }
                break;
                case 1 :
                {
                    rNumberFormat.SetNumberingType( SVX_NUM_CHARS_UPPER_LETTER );
                    rNumberFormat.SetSuffix( String( RTL_CONSTASCII_USTRINGPARAM( "." ) ) );
                }
                break;
                case 2 :
                {
                    rNumberFormat.SetNumberingType( SVX_NUM_ARABIC );
                    rNumberFormat.SetSuffix( String( RTL_CONSTASCII_USTRINGPARAM( ")" ) ) );
                }
                break;
                case 3 :
                {
                    rNumberFormat.SetNumberingType( SVX_NUM_ARABIC );
                    rNumberFormat.SetSuffix( String( RTL_CONSTASCII_USTRINGPARAM( "." ) ) );
                }
                break;
                case 4 :
                {
                    rNumberFormat.SetNumberingType( SVX_NUM_ROMAN_LOWER );
                    rNumberFormat.SetSuffix( String( RTL_CONSTASCII_USTRINGPARAM( ")" ) ) );
                    rNumberFormat.SetPrefix( String( RTL_CONSTASCII_USTRINGPARAM( "(" ) ) );
                }
                break;
                case 5 :
                {
                    rNumberFormat.SetNumberingType( SVX_NUM_ROMAN_LOWER );
                    rNumberFormat.SetSuffix( String( RTL_CONSTASCII_USTRINGPARAM( ")" ) ) );
                }
                break;
                case 6 :
                {
                    rNumberFormat.SetNumberingType( SVX_NUM_ROMAN_LOWER );
                    rNumberFormat.SetSuffix( String( RTL_CONSTASCII_USTRINGPARAM( "." ) ) );
                }
                break;
                case 7 :
                {
                    rNumberFormat.SetNumberingType( SVX_NUM_ROMAN_UPPER );
                    rNumberFormat.SetSuffix( String( RTL_CONSTASCII_USTRINGPARAM( "." ) ) );
                }
                break;
                case 8 :
                {
                    rNumberFormat.SetNumberingType( SVX_NUM_CHARS_LOWER_LETTER );
                    rNumberFormat.SetSuffix( String( RTL_CONSTASCII_USTRINGPARAM( ")" ) ) );
                    rNumberFormat.SetPrefix( String( RTL_CONSTASCII_USTRINGPARAM( "(" ) ) );
                }
                break;
                case 9 :
                {
                    rNumberFormat.SetNumberingType( SVX_NUM_CHARS_LOWER_LETTER );
                    rNumberFormat.SetSuffix( String( RTL_CONSTASCII_USTRINGPARAM( ")" ) ) );
                }
                break;
                case 10 :
                {
                    rNumberFormat.SetNumberingType( SVX_NUM_CHARS_UPPER_LETTER );
                    rNumberFormat.SetSuffix( String( RTL_CONSTASCII_USTRINGPARAM( ")" ) ) );
                    rNumberFormat.SetPrefix( String( RTL_CONSTASCII_USTRINGPARAM( "(" ) ) );
                }
                break;
                case 11 :
                {
                    rNumberFormat.SetNumberingType( SVX_NUM_CHARS_UPPER_LETTER );
                    rNumberFormat.SetSuffix( String( RTL_CONSTASCII_USTRINGPARAM( ")" ) ) );
                }
                break;
                case 12 :
                {
                    rNumberFormat.SetNumberingType( SVX_NUM_ARABIC );
                    rNumberFormat.SetSuffix( String( RTL_CONSTASCII_USTRINGPARAM( ")" ) ) );
                    rNumberFormat.SetPrefix( String( RTL_CONSTASCII_USTRINGPARAM( "(" ) ) );
                }
                break;
                case 13 :
                {
                    rNumberFormat.SetNumberingType( SVX_NUM_ARABIC );
                }
                break;
                case 14 :
                {
                    rNumberFormat.SetNumberingType( SVX_NUM_ROMAN_UPPER );
                    rNumberFormat.SetSuffix( String( RTL_CONSTASCII_USTRINGPARAM( ")" ) ) );
                    rNumberFormat.SetPrefix( String( RTL_CONSTASCII_USTRINGPARAM( "(" ) ) );
                }
                break;
                case 15 :
                {
                    rNumberFormat.SetNumberingType( SVX_NUM_ROMAN_UPPER );
                    rNumberFormat.SetSuffix( String( RTL_CONSTASCII_USTRINGPARAM( ")" ) ) );
                }
                break;
            }
        }
//      if ( nBuFlags & 0x02000000 )
//          rNumberFormat.SetStart( nBuStart );
    }
    return bHardAttribute;
}

void PPTNumberFormatCreator::GetNumberFormat( SdrPowerPointImport& rManager, SvxNumberFormat& rNumberFormat, UINT32 nLevel, const PPTParaLevel& rParaLevel, const PPTCharLevel& rCharLevel, UINT32 nInstance )
{
    nIsBullet = ( rParaLevel.mnBuFlags & ( 1 << PPT_ParaAttr_BulletOn ) ) != 0 ? 1 : 0;
    nBulletChar = rParaLevel.mnBulletChar;

    sal_Bool bBuHardFont;
    bBuHardFont = ( rParaLevel.mnBuFlags & ( 1 << PPT_ParaAttr_BuHardFont ) ) != 0;
    if ( bBuHardFont )
        nBulletFont = rParaLevel.mnBulletFont;
    else
        nBulletFont = rCharLevel.mnFont;
    nBulletHeight = rParaLevel.mnBulletHeight;
    nBulletColor = rParaLevel.mnBulletColor;
    nTextOfs = rParaLevel.mnTextOfs;
    nBulletOfs = rParaLevel.mnBulletOfs;

    ImplGetExtNumberFormat( rManager, rNumberFormat, nLevel, nInstance, 0xffffffff, rCharLevel.mnFontHeight, NULL );
    if ( ( rNumberFormat.GetNumberingType() != SVX_NUM_BITMAP ) && ( nBulletHeight > 0x7fff ) )
        nBulletHeight = rCharLevel.mnFontHeight ? ((-((sal_Int16)nBulletHeight)) * 100 ) / rCharLevel.mnFontHeight : 100;
    ImplGetNumberFormat( rManager, rNumberFormat, nLevel );
    switch ( rNumberFormat.GetNumberingType() )
    {
        case SVX_NUM_CHARS_UPPER_LETTER :
        case SVX_NUM_CHARS_LOWER_LETTER :
        case SVX_NUM_ROMAN_UPPER :
        case SVX_NUM_ROMAN_LOWER :
        case SVX_NUM_ARABIC :
        case SVX_NUM_CHARS_UPPER_LETTER_N :
        case SVX_NUM_CHARS_LOWER_LETTER_N :
        {
            sal_uInt32 nFont = rCharLevel.mnFont;
            PptFontEntityAtom* pFontEnityAtom = rManager.GetFontEnityAtom( nFont );
            if ( pFontEnityAtom )
            {
                Font aFont;
                aFont.SetCharSet( pFontEnityAtom->eCharSet );
                aFont.SetName( pFontEnityAtom->aName );
                aFont.SetFamily( pFontEnityAtom->eFamily );
                aFont.SetPitch( pFontEnityAtom->ePitch );
                rNumberFormat.SetBulletFont( &aFont );
            }
        }
        break;
    }
}

BOOL PPTNumberFormatCreator::GetNumberFormat( SdrPowerPointImport& rManager, SvxNumberFormat& rNumberFormat, PPTParagraphObj* pParaObj, UINT32 nInstanceInSheet )
{
    UINT32 nHardCount = 0;
    nHardCount += pParaObj->GetAttrib( PPT_ParaAttr_BulletOn, nIsBullet, nInstanceInSheet );
    nHardCount += pParaObj->GetAttrib( PPT_ParaAttr_BulletChar, nBulletChar, nInstanceInSheet );
    nHardCount += pParaObj->GetAttrib( PPT_ParaAttr_BulletFont, nBulletFont, nInstanceInSheet );
    nHardCount += pParaObj->GetAttrib( PPT_ParaAttr_BulletHeight, nBulletHeight, nInstanceInSheet );
    nHardCount += pParaObj->GetAttrib( PPT_ParaAttr_BulletColor, nBulletColor, nInstanceInSheet );
    nHardCount += pParaObj->GetAttrib( PPT_ParaAttr_TextOfs, nTextOfs, nInstanceInSheet );
    nHardCount += pParaObj->GetAttrib( PPT_ParaAttr_BulletOfs, nBulletOfs, nInstanceInSheet );

    UINT32 nFontHeight = 24;
    PPTPortionObj* pPtr = pParaObj->First();
    if ( pPtr )
        pPtr->GetAttrib( PPT_CharAttr_FontHeight, nFontHeight, nInstanceInSheet );
    nHardCount += ImplGetExtNumberFormat( rManager, rNumberFormat, pParaObj->pParaSet->mnDepth,
                                                pParaObj->mnInstance, nInstanceInSheet, nFontHeight, pParaObj );

    if ( rNumberFormat.GetNumberingType() != SVX_NUM_BITMAP )
        pParaObj->UpdateBulletRelSize( nBulletHeight );
    if ( nHardCount )
        ImplGetNumberFormat( rManager, rNumberFormat, pParaObj->pParaSet->mnDepth );

    if ( nHardCount )
    {
        switch ( rNumberFormat.GetNumberingType() )
        {
            case SVX_NUM_CHARS_UPPER_LETTER :
            case SVX_NUM_CHARS_LOWER_LETTER :
            case SVX_NUM_ROMAN_UPPER :
            case SVX_NUM_ROMAN_LOWER :
            case SVX_NUM_ARABIC :
            case SVX_NUM_CHARS_UPPER_LETTER_N :
            case SVX_NUM_CHARS_LOWER_LETTER_N :
            {
                if ( pPtr )
                {
                    sal_uInt32 nFont;
                    pPtr->GetAttrib( PPT_CharAttr_Font, nFont, nInstanceInSheet );
                    PptFontEntityAtom* pFontEnityAtom = rManager.GetFontEnityAtom( nFont );
                    if ( pFontEnityAtom )
                    {
                        Font aFont;
                        aFont.SetCharSet( pFontEnityAtom->eCharSet );
                        aFont.SetName( pFontEnityAtom->aName );
                        aFont.SetFamily( pFontEnityAtom->eFamily );
                        aFont.SetPitch( pFontEnityAtom->ePitch );
                        rNumberFormat.SetBulletFont( &aFont );
                    }
                }
            }
            break;
        }
    }
    return ( nHardCount ) ? TRUE : FALSE;
}

void PPTNumberFormatCreator::ImplGetNumberFormat( SdrPowerPointImport& rManager, SvxNumberFormat& rNumberFormat, UINT32 nLevel )
{
    Font aFont;
    PptFontEntityAtom* pAtom = rManager.GetFontEnityAtom( nBulletFont );
    if ( pAtom )
    {
        CharSet eCharSet( pAtom->eCharSet );
        aFont.SetName( pAtom->aName );
        aFont.SetCharSet( eCharSet );
        aFont.SetFamily( pAtom->eFamily );
        aFont.SetPitch( pAtom->ePitch );
    }
    Color aCol( rManager.MSO_CLR_ToColor( nBulletColor ) );
    aFont.SetColor( aCol );

    sal_uInt16 nBuChar = (sal_uInt16)nBulletChar;
    if ( aFont.GetCharSet() == RTL_TEXTENCODING_SYMBOL )
    {
        nBuChar &= 0x00ff;
        nBuChar |= 0xf000;
    }
    rNumberFormat.SetBulletFont( &aFont );
    rNumberFormat.SetBulletChar( nBuChar );
    rNumberFormat.SetBulletRelSize( (UINT16)nBulletHeight );
    rNumberFormat.SetBulletColor( aCol );
    UINT16 nAbsLSpace = (UINT16)( ( (UINT32)nTextOfs * 2540 ) / 576 );
    UINT16 nFirstLineOffset = nAbsLSpace - (UINT16)( ( (UINT32)nBulletOfs * 2540 ) / 576 );
    rNumberFormat.SetAbsLSpace( nAbsLSpace );
    rNumberFormat.SetFirstLineOffset( -nFirstLineOffset );
}

////////////////////////////////////////////////////////////////////////////////////////////////////

PPTCharSheet::PPTCharSheet( UINT32 nInstance )
{
    UINT32 nColor = PPT_COLSCHEME_TEXT_UND_ZEILEN;
    UINT16 nFontHeight;
    switch ( nInstance )
    {
        case TSS_TYPE_PAGETITLE :
        case TSS_TYPE_TITLE :
        {
            nColor = PPT_COLSCHEME_TITELTEXT;
            nFontHeight = 44;
        }
        break;
        case TSS_TYPE_BODY :
        case TSS_TYPE_SUBTITLE :
        case TSS_TYPE_HALFBODY :
        case TSS_TYPE_QUARTERBODY :
            nFontHeight = 32;
        break;
        case TSS_TYPE_NOTES :
            nFontHeight = 12;
        break;
        case TSS_TYPE_UNUSED :
        case TSS_TYPE_TEXT_IN_SHAPE :
            nFontHeight = 24;
        break;
    }
    for ( UINT32 nDepth = 0; nDepth < 5; nDepth++ )
    {
        maCharLevel[ nDepth ].mnFlags = 0;
        maCharLevel[ nDepth ].mnFont = 0;
        maCharLevel[ nDepth ].mnAsianOrComplexFont = 0xffff;
        maCharLevel[ nDepth ].mnFontHeight = nFontHeight;
        maCharLevel[ nDepth ].mnFontColor = nColor;
        maCharLevel[ nDepth ].mnFontColorInStyleSheet = Color( (BYTE)nColor, (BYTE)( nColor >> 8 ), (BYTE)( nColor >> 16 ) );
        maCharLevel[ nDepth ].mnEscapement = 0;
    }
}

PPTCharSheet::PPTCharSheet( const PPTCharSheet& rAttr )
{
    *this = rAttr;
}

void PPTCharSheet::Read( SvStream& rIn, sal_Bool bMasterStyle, sal_uInt32 nLevel, sal_Bool bFirst )
{
    // Zeichenattribute
    sal_uInt32 nCMask;
    sal_uInt16 nVal16;
    rIn >> nCMask;

    if ( nCMask & 0x0000FFFF )
    {
        sal_uInt16 nBitAttr;
        maCharLevel[ nLevel ].mnFlags &= ~( (sal_uInt16)nCMask );
        rIn >> nBitAttr; // Bit-Attribute (Fett, Unterstrichen, ...)
        maCharLevel[ nLevel ].mnFlags |= nBitAttr;
    }
    if ( nCMask & ( 1 << PPT_CharAttr_Font ) )                  // 0x00010000
        rIn >> maCharLevel[ nLevel ].mnFont;
    if ( nCMask & ( 1 << PPT_CharAttr_AsianOrComplexFont ) )    // 0x00200000
        rIn >> maCharLevel[ nLevel ].mnAsianOrComplexFont;
    if ( nCMask & ( 1 << PPT_CharAttr_Unknown2 ) )              // 0x00400000
        rIn >> nVal16;
    if ( nCMask & ( 1 << PPT_CharAttr_Symbol ) )                // 0x00800000
        rIn >> nVal16;
    if ( nCMask & ( 1 << PPT_CharAttr_FontHeight ) )            // 0x00020000
        rIn >> maCharLevel[ nLevel ].mnFontHeight;
    if ( nCMask & ( 1 << PPT_CharAttr_FontColor ) )             // 0x00040000
    {
        rIn >> maCharLevel[ nLevel ].mnFontColor;
        if( ! (maCharLevel[ nLevel ].mnFontColor && 0xff000000 ) )
            maCharLevel[ nLevel ].mnFontColor = PPT_COLSCHEME_HINTERGRUND;
    }
    if ( nCMask & ( 1 << PPT_CharAttr_Escapement ) )            // 0x00080000
        rIn >> maCharLevel[ nLevel ].mnEscapement;
    if ( nCMask & 0x00100000 )                                  // 0x00100000
        rIn >> nVal16;

    nCMask >>= 24;
    while( nCMask )
    {
        if ( nCMask & 1 )
        {
            DBG_ERROR( "PPTCharSheet::Read - unknown attribute, send me this document (SJ)" );
            rIn >> nVal16;
        }
        nCMask >>= 1;
    }
}

PPTParaSheet::PPTParaSheet( UINT32 nInstance )
{
    sal_uInt16 nBuFlags = 0;
    sal_uInt32 nBulletColor = 0x8000000;
    sal_uInt16 nUpperDist = 0;

    switch ( nInstance )
    {
        case TSS_TYPE_PAGETITLE :
        case TSS_TYPE_TITLE :
            nBulletColor = PPT_COLSCHEME_TITELTEXT;
        break;
        case TSS_TYPE_BODY :
        case TSS_TYPE_SUBTITLE :
        case TSS_TYPE_HALFBODY :
        case TSS_TYPE_QUARTERBODY :
        {
            nBuFlags = 1;
            nUpperDist = 0x14;
        }
        break;
        case TSS_TYPE_NOTES :
            nUpperDist = 0x1e;
        break;
/*
        default :
        case TSS_TYPE_UNUSED :
        case TSS_TYPE_TEXT_IN_SHAPE :
        break;
*/
    }
    for ( UINT32 i = 0; i < 5; i++ )
    {
        maParaLevel[ i ].mnBuFlags = nBuFlags;
        maParaLevel[ i ].mnBulletChar = 0x2022;
        maParaLevel[ i ].mnBulletFont = 0;
        maParaLevel[ i ].mnBulletHeight = 100;
        maParaLevel[ i ].mnBulletColor = nBulletColor;
        maParaLevel[ i ].mnAdjust = 0;
        maParaLevel[ i ].mnLineFeed = 100;
        maParaLevel[ i ].mnLowerDist = 0;
        maParaLevel[ i ].mnUpperDist = nUpperDist;
        maParaLevel[ i ].mnTextOfs = 0;
        maParaLevel[ i ].mnBulletOfs = 0;
        maParaLevel[ i ].mnDefaultTab = 0x240;
        maParaLevel[ i ].mnAsianLineBreak = 0;
        maParaLevel[ i ].mnBiDi = 0;
    }
}

PPTParaSheet::PPTParaSheet( const PPTParaSheet& rSheet )
{
    *this = rSheet;
}

void PPTParaSheet::Read( SdrPowerPointImport& rManager, SvStream& rIn, sal_Bool bMasterStyle,
                    sal_uInt32 nLevel, sal_Bool bFirst )
{
    // Absatzattribute
    sal_uInt16  nVal16, i, nMask16;
    sal_uInt32  nVal32, nPMask;
    rIn >> nPMask;

    nMask16 = (sal_uInt16)nPMask & 0xf;
    if ( nMask16 )
    {
        rIn >> nVal16;
        maParaLevel[ nLevel ].mnBuFlags &=~ nMask16;
        nVal16 &= nMask16;
        maParaLevel[ nLevel ].mnBuFlags |= nVal16;
    }
    if ( nPMask & 0x0080 )
        rIn >> maParaLevel[ nLevel ].mnBulletChar;
    if ( nPMask & 0x0010 )
        rIn >> maParaLevel[ nLevel ].mnBulletFont;
    if ( nPMask & 0x0040 )
    {
        rIn >> nVal16;
        maParaLevel[ nLevel ].mnBulletHeight = nVal16;
    }
    if ( nPMask & 0x0020 )
    {
        rIn >> nVal32;
        maParaLevel[ nLevel ].mnBulletColor = nVal32;
    }
    if ( bFirst )
    {
        if ( nPMask & 0xF00 )
        {   // AbsJust!
            rIn >> nVal16;
            maParaLevel[ nLevel ].mnAdjust = nVal16 & 3;
        }
        if ( nPMask & 0x1000 )
            rIn >> maParaLevel[ nLevel ].mnLineFeed;
        if ( nPMask & 0x2000 )
            rIn >> maParaLevel[ nLevel ].mnUpperDist;
        if ( nPMask & 0x4000 )
            rIn >> maParaLevel[ nLevel ].mnLowerDist;
        if ( nPMask & 0x8000 )
            rIn >> maParaLevel[ nLevel ].mnTextOfs;
        if ( nPMask & 0x10000 )
            rIn >> maParaLevel[ nLevel ].mnBulletOfs;
        if ( nPMask & 0x20000 )
            rIn >> maParaLevel[ nLevel ].mnDefaultTab;
        if ( nPMask & 0x200000 )
        {
            // number of tabulators
            rIn >> nVal16;
            for ( i = 0; i < nVal16; i++ )
                rIn >> nVal32;      // reading the tabulators
        }
        if ( nPMask & 0x40000 )
            rIn >> nVal16;
        if ( nPMask & 0x80000 )
            rIn >> maParaLevel[ nLevel ].mnAsianLineBreak;
        if ( nPMask & 0x100000 )
            rIn >> maParaLevel[ nLevel ].mnBiDi;
    }
    else
    {
        if ( nPMask & 0x800 )
        {
            rIn >> nVal16;
            maParaLevel[ nLevel ].mnAdjust = nVal16 & 3;
        }
        if ( nPMask & 0x1000 )
            rIn >> maParaLevel[ nLevel ].mnLineFeed;
        if ( nPMask & 0x2000 )
            rIn >> maParaLevel[ nLevel ].mnUpperDist;
        if ( nPMask & 0x4000 )
            rIn >> maParaLevel[ nLevel ].mnLowerDist;
        if ( nPMask & 0x8000 )
            rIn >> nVal16;
        if ( nPMask & 0x100 )
            rIn >> maParaLevel[ nLevel ].mnTextOfs;
        if ( nPMask & 0x200 )
            rIn >> nVal16;
        if ( nPMask & 0x400 )
            rIn >> maParaLevel[ nLevel ].mnBulletOfs;
        if ( nPMask & 0x10000 )
            rIn >> nVal16;
        if ( nPMask & 0xe0000 )
        {
            sal_uInt16 nFlagsToModifyMask = (sal_uInt16)( ( nPMask >> 17 ) & 7 );
            rIn >> nVal16;
            // bits that are not involved to zero
            nVal16 &= nFlagsToModifyMask;
            // bits that are to change to zero
            maParaLevel[ nLevel ].mnAsianLineBreak &=~nFlagsToModifyMask;
            // now set the corresponding bits
            maParaLevel[ nLevel ].mnAsianLineBreak |= nVal16;
        }
        if ( nPMask & 0x100000 )
        {
            // number of tabulators
            rIn >> nVal16;
            for ( i = 0; i < nVal16; i++ )
                rIn >> nVal32;      // reading the tabulators
        }
        if ( nPMask & 0x200000 )
            rIn >> maParaLevel[ nLevel ].mnBiDi;        // #88602#
    }

    nPMask >>= 22;
    while( nPMask )
    {
        if ( nPMask & 1 )
        {
#ifdef DBG_UTIL
            if (!(rManager.rImportParam.nImportFlags & PPT_IMPORTFLAGS_NO_TEXT_ASSERT))
            {
                DBG_ERROR( "PPTParaSheet::Read - unknown attribute, send me this document (SJ)" );
            }
#endif
            rIn >> nVal16;
        }
        nPMask >>= 1;
    }
}

PPTStyleSheet::PPTStyleSheet( const DffRecordHeader& rSlideHd, SvStream& rIn, SdrPowerPointImport& rManager,
                                const PPTTextParagraphStyleAtomInterpreter& rTxPFStyle, const PPTTextSpecInfo& rTextSpecInfo ) :
    maTxSI                  ( rTextSpecInfo ),
    PPTNumberFormatCreator  ( new PPTExtParaProv( rManager, rIn, &rSlideHd ) )
{
    UINT32 i;
    UINT32 nOldFilePos = rIn.Tell();

    // default stylesheets
    BOOL bFoundTxMasterStyleAtom04 = FALSE;
    mpCharSheet[ TSS_TYPE_PAGETITLE ] = new PPTCharSheet( TSS_TYPE_PAGETITLE );
    mpCharSheet[ TSS_TYPE_BODY ] = new PPTCharSheet( TSS_TYPE_BODY );
    mpCharSheet[ TSS_TYPE_NOTES ] = new PPTCharSheet(  TSS_TYPE_NOTES );
    mpCharSheet[ TSS_TYPE_UNUSED ] = new PPTCharSheet( TSS_TYPE_UNUSED );   // this entry is not used by ppt
    mpCharSheet[ TSS_TYPE_TEXT_IN_SHAPE ] = new PPTCharSheet( TSS_TYPE_TEXT_IN_SHAPE );
    mpParaSheet[ TSS_TYPE_PAGETITLE ] = new PPTParaSheet( TSS_TYPE_PAGETITLE );
    mpParaSheet[ TSS_TYPE_BODY ] = new PPTParaSheet( TSS_TYPE_BODY );
    mpParaSheet[ TSS_TYPE_NOTES ] = new PPTParaSheet( TSS_TYPE_NOTES );
    mpParaSheet[ TSS_TYPE_UNUSED ] = new PPTParaSheet( TSS_TYPE_UNUSED );
    mpParaSheet[ TSS_TYPE_TEXT_IN_SHAPE ] = new PPTParaSheet( TSS_TYPE_TEXT_IN_SHAPE );
    mpCharSheet[ TSS_TYPE_QUARTERBODY ] = mpCharSheet[ TSS_TYPE_HALFBODY ] = mpCharSheet[ TSS_TYPE_TITLE ] = mpCharSheet[ TSS_TYPE_SUBTITLE ] = NULL;
    mpParaSheet[ TSS_TYPE_QUARTERBODY ] = mpParaSheet[ TSS_TYPE_HALFBODY ] = mpParaSheet[ TSS_TYPE_TITLE ] = mpParaSheet[ TSS_TYPE_SUBTITLE ] = NULL;

    rSlideHd.SeekToContent( rIn );
    DffRecordHeader aTxMasterStyleHd;
    while ( rIn.Tell() < rSlideHd.GetRecEndFilePos() )
    {
        rIn >> aTxMasterStyleHd;
        if ( aTxMasterStyleHd.nRecType == PPT_PST_TxMasterStyleAtom )
            break;
        else
            aTxMasterStyleHd.SeekToEndOfRecord( rIn );
    }
    while ( ( aTxMasterStyleHd.nRecType == PPT_PST_TxMasterStyleAtom ) && ( rIn.Tell() < rSlideHd.GetRecEndFilePos() ) )
    {
        UINT32 nInstance = aTxMasterStyleHd.nRecInstance;
        if ( nInstance < PPT_STYLESHEETENTRYS )
        {
            if ( nInstance > 4 )
            {
                delete mpCharSheet[ nInstance ];    // be sure to delete the old one if this instance comes twice
                delete mpParaSheet[ nInstance ];

                switch ( nInstance )
                {
                    case TSS_TYPE_SUBTITLE :
                    {
                        mpCharSheet[ TSS_TYPE_SUBTITLE ] = new PPTCharSheet( *( mpCharSheet[ TSS_TYPE_BODY ] ) );
                        mpParaSheet[ TSS_TYPE_SUBTITLE ] = new PPTParaSheet( *( mpParaSheet[ TSS_TYPE_BODY ] ) );
                    }
                    break;
                    case TSS_TYPE_TITLE :
                    {
                        mpCharSheet[ TSS_TYPE_TITLE ] = new PPTCharSheet( *( mpCharSheet[ TSS_TYPE_PAGETITLE ] ) );
                        mpParaSheet[ TSS_TYPE_TITLE ] = new PPTParaSheet( *( mpParaSheet[ TSS_TYPE_PAGETITLE ] ) );
                    }
                    break;
                    case TSS_TYPE_HALFBODY :
                    {
                        mpCharSheet[ TSS_TYPE_HALFBODY ] = new PPTCharSheet( *( mpCharSheet[ TSS_TYPE_BODY ] ) );
                        mpParaSheet[ TSS_TYPE_HALFBODY ] = new PPTParaSheet( *( mpParaSheet[ TSS_TYPE_BODY ] ) );
                    }
                    break;

                    case TSS_TYPE_QUARTERBODY :
                    {
                        mpCharSheet[ TSS_TYPE_QUARTERBODY ] = new PPTCharSheet( *( mpCharSheet[ TSS_TYPE_BODY ] ) );
                        mpParaSheet[ TSS_TYPE_QUARTERBODY ] = new PPTParaSheet( *( mpParaSheet[ TSS_TYPE_BODY ] ) );
                    }
                    break;
                }
            }
            else if ( nInstance == 4 )
                bFoundTxMasterStyleAtom04 = TRUE;

            sal_uInt16 nLevelAnz;
            rIn >> nLevelAnz;
            if ( nLevelAnz > 5 )
            {
                DBG_ERROR( "PPTStyleSheet::Ppt-TextStylesheet hat mehr als 5 Ebenen! (SJ)" );
                nLevelAnz = 5;
            }
            sal_uInt16  nLev = 0;
            sal_Bool    bFirst = sal_True;

            while ( rIn.GetError() == 0 && rIn.Tell() < aTxMasterStyleHd.GetRecEndFilePos() && nLev < nLevelAnz )
            {
                if ( nLev && ( nInstance < 5 ) )
                {
                    mpParaSheet[ nInstance ]->maParaLevel[ nLev ] = mpParaSheet[ nInstance ]->maParaLevel[ nLev - 1 ];
                    mpCharSheet[ nInstance ]->maCharLevel[ nLev ] = mpCharSheet[ nInstance ]->maCharLevel[ nLev - 1 ];
                }

                // Ausnahme: Vorlage 5, 6 (MasterTitle Titel und SubTitel)
                if ( nInstance >= TSS_TYPE_SUBTITLE )
                {
                    bFirst = sal_False;

                    sal_uInt16 nDontKnow;
                    rIn >> nDontKnow;
                }
                mpParaSheet[ nInstance ]->Read( rManager, rIn, sal_True, nLev, bFirst );
                mpCharSheet[ nInstance ]->Read( rIn, sal_True, nLev, bFirst );
                bFirst = sal_False;
                nLev++;
            }
#ifdef DBG_UTIL
            if (!(rManager.rImportParam.nImportFlags & PPT_IMPORTFLAGS_NO_TEXT_ASSERT))
            {
                if ( rIn.GetError() == 0 )
                {
                    ByteString aMsg;
                    if ( rIn.Tell() > aTxMasterStyleHd.GetRecEndFilePos() )
                    {
                        aMsg += "\n  ";
                        aMsg += "reading too many bytes:";
                        aMsg += ByteString::CreateFromInt32( rIn.Tell() - aTxMasterStyleHd.GetRecEndFilePos() );
                    }
                    if ( rIn.Tell() < aTxMasterStyleHd.GetRecEndFilePos() )
                    {
                        aMsg += "\n  ";
                        aMsg += "reading too less bytes:";
                        aMsg += ByteString::CreateFromInt32( aTxMasterStyleHd.GetRecEndFilePos() - rIn.Tell() );
                    }
                    if ( aMsg.Len() != 0 )
                    {
                        aMsg.Insert( "]:", 0 );
                        aMsg.Insert( "PptStyleSheet::operator>>[", 0 );
                        DBG_ERROR(aMsg.GetBuffer());
                    }
                }
                if ( rIn.Tell() != aTxMasterStyleHd.GetRecEndFilePos() )
                    DBG_ASSERT(0, "SJ: Falsche Anzahl von Bytes gelesen beim Import der PPT-Formatvorlagen");
            }
#endif
        }
        aTxMasterStyleHd.SeekToEndOfRecord( rIn );
        rIn >> aTxMasterStyleHd;
    }
    if ( !mpCharSheet[ TSS_TYPE_SUBTITLE ] )
    {
        mpCharSheet[ TSS_TYPE_SUBTITLE ] = new PPTCharSheet( *( mpCharSheet[ TSS_TYPE_BODY ] ) );
        mpParaSheet[ TSS_TYPE_SUBTITLE ] = new PPTParaSheet( *( mpParaSheet[ TSS_TYPE_BODY ] ) );
    }
    if ( !mpCharSheet[ TSS_TYPE_TITLE ] )
    {
        mpCharSheet[ TSS_TYPE_TITLE ] = new PPTCharSheet( *( mpCharSheet[ TSS_TYPE_PAGETITLE ] ) );
        mpParaSheet[ TSS_TYPE_TITLE ] = new PPTParaSheet( *( mpParaSheet[ TSS_TYPE_PAGETITLE ] ) );
    }
    if ( !mpCharSheet[ TSS_TYPE_HALFBODY ] )
    {
        mpCharSheet[ TSS_TYPE_HALFBODY ] = new PPTCharSheet( *( mpCharSheet[ TSS_TYPE_BODY ] ) );
        mpParaSheet[ TSS_TYPE_HALFBODY ] = new PPTParaSheet( *( mpParaSheet[ TSS_TYPE_BODY ] ) );
    }
    if ( !mpCharSheet[ TSS_TYPE_QUARTERBODY ] )
    {
        mpCharSheet[ TSS_TYPE_QUARTERBODY ] = new PPTCharSheet( *( mpCharSheet[ TSS_TYPE_BODY ] ) );
        mpParaSheet[ TSS_TYPE_QUARTERBODY ] = new PPTParaSheet( *( mpParaSheet[ TSS_TYPE_BODY ] ) );
    }
    if ( !bFoundTxMasterStyleAtom04 )
    {   // try to locate the txMasterStyleAtom in the Environment
        DffRecordHeader* pEnvHeader = rManager.aDocRecManager.GetRecordHeader( PPT_PST_Environment );
        if ( pEnvHeader )
        {
            pEnvHeader->SeekToContent( rIn );
            DffRecordHeader aTxMasterStyleHd;
            while ( rIn.Tell() < pEnvHeader->GetRecEndFilePos() )
            {
                rIn >> aTxMasterStyleHd;
                if ( aTxMasterStyleHd.nRecType == PPT_PST_TxMasterStyleAtom )
                {
                    sal_uInt16 nLevelAnz;
                    rIn >> nLevelAnz;

                    sal_uInt16 nLev = 0;
                    sal_Bool bFirst = sal_True;
                    while ( rIn.GetError() == 0 && rIn.Tell() < aTxMasterStyleHd.GetRecEndFilePos() && nLev < nLevelAnz )
                    {
                        if ( nLev )
                        {
                            mpParaSheet[ TSS_TYPE_TEXT_IN_SHAPE ]->maParaLevel[ nLev ] = mpParaSheet[ TSS_TYPE_TEXT_IN_SHAPE ]->maParaLevel[ nLev - 1 ];
                            mpCharSheet[ TSS_TYPE_TEXT_IN_SHAPE ]->maCharLevel[ nLev ] = mpCharSheet[ TSS_TYPE_TEXT_IN_SHAPE ]->maCharLevel[ nLev - 1 ];
                        }
                        mpParaSheet[ TSS_TYPE_TEXT_IN_SHAPE ]->Read( rManager, rIn, sal_True, nLev, bFirst );
                        if ( !nLev )
                        {
                            // set paragraph defaults for instance 4 (TSS_TYPE_TEXT_IN_SHAPE)
                            if ( rTxPFStyle.bValid )
                            {
                                PPTParaLevel& rParaLevel = mpParaSheet[ TSS_TYPE_TEXT_IN_SHAPE ]->maParaLevel[ 0 ];
                                rParaLevel.mnAsianLineBreak = 0;
                                if ( rTxPFStyle.bForbiddenRules )
                                    rParaLevel.mnAsianLineBreak |= 1;
                                if ( !rTxPFStyle.bLatinTextWrap )
                                    rParaLevel.mnAsianLineBreak |= 2;
                                if ( rTxPFStyle.bHangingPunctuation )
                                    rParaLevel.mnAsianLineBreak |= 4;
                            }
                        }
                        mpCharSheet[ TSS_TYPE_TEXT_IN_SHAPE ]->Read( rIn, sal_True, nLev, bFirst );
                        bFirst = sal_False;
                        nLev++;
                    }
                    break;
                }
                else
                    aTxMasterStyleHd.SeekToEndOfRecord( rIn );
            }
        }
    }
    rIn.Seek( nOldFilePos );

    // will will create the default numbulletitem for each instance
    for ( i = 0; i < PPT_STYLESHEETENTRYS; i++ )
    {
        UINT16          nLevels, nDepth = 0;
        SvxNumRuleType  eNumRuleType;

        switch ( i )
        {
            case TSS_TYPE_PAGETITLE :
            case TSS_TYPE_TITLE :
                nLevels = 1;
                eNumRuleType = SVX_RULETYPE_NUMBERING;
            break;
            case TSS_TYPE_SUBTITLE :
                nLevels = 9;
                eNumRuleType = SVX_RULETYPE_NUMBERING;
            break;
            case TSS_TYPE_BODY :
            case TSS_TYPE_HALFBODY :
            case TSS_TYPE_QUARTERBODY :
                nDepth = 1;
                nLevels = 9;
                eNumRuleType = SVX_RULETYPE_PRESENTATION_NUMBERING;
            break;
            default :
            case TSS_TYPE_NOTES :
            case TSS_TYPE_UNUSED :
            case TSS_TYPE_TEXT_IN_SHAPE :
                nLevels = 10;
                eNumRuleType = SVX_RULETYPE_NUMBERING;
            break;
        }
        SvxNumRule aRule( NUM_BULLET_REL_SIZE | NUM_BULLET_COLOR |
                        NUM_CHAR_TEXT_DISTANCE | NUM_SYMBOL_ALIGNMENT,
                        nLevels, FALSE, eNumRuleType );
        if ( eNumRuleType == SVX_RULETYPE_PRESENTATION_NUMBERING )
            nLevels++;
        for ( UINT16 nCount = 0; nDepth < nLevels; nCount++ )
        {
            const PPTParaLevel& rParaLevel = mpParaSheet[ i ]->maParaLevel[ nCount ];
            const PPTCharLevel& rCharLevel = mpCharSheet[ i ]->maCharLevel[ nCount ];
            SvxNumberFormat aNumberFormat( SVX_NUM_CHAR_SPECIAL );
            aNumberFormat.SetBulletChar( ' ' );
            GetNumberFormat( rManager, aNumberFormat, nCount, rParaLevel, rCharLevel, i );
            aRule.SetLevel( nDepth++, aNumberFormat );
            if ( nCount >= 4 )
            {
                for ( ;nDepth < nLevels; nDepth++ )
                    aRule.SetLevel( nDepth, aNumberFormat );
                if ( eNumRuleType == SVX_RULETYPE_PRESENTATION_NUMBERING )
                    aRule.SetLevel( 0, aNumberFormat );
            }
        }
        mpNumBulletItem[ i ] = new SvxNumBulletItem( aRule, EE_PARA_NUMBULLET );
    }
}

PPTStyleSheet::~PPTStyleSheet()
{
    for ( UINT32 i = 0; i < PPT_STYLESHEETENTRYS; i++ )
    {
        delete mpCharSheet[ i ];
        delete mpParaSheet[ i ];
        delete mpNumBulletItem[ i ];
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

PPTParaPropSet::PPTParaPropSet() :
    pParaSet( new ImplPPTParaPropSet )
{
    pParaSet->nBuStart = 1;
}

PPTParaPropSet::PPTParaPropSet( PPTParaPropSet& rParaPropSet )
{
    pParaSet = rParaPropSet.pParaSet;
    pParaSet->mnRefCount++;

    mnOriginalTextPos = rParaPropSet.mnOriginalTextPos;

#ifdef DBG_EXTRACT_BUDATA
    mnCharacters = rParaPropSet.mnCharacters;
#endif

}

PPTParaPropSet::~PPTParaPropSet()
{
    if ( ! ( --pParaSet->mnRefCount ) )
        delete pParaSet;
}

PPTParaPropSet& PPTParaPropSet::operator=( PPTParaPropSet& rParaPropSet )
{
    if ( this != &rParaPropSet )
    {
        if ( ! ( --pParaSet->mnRefCount ) )
            delete pParaSet;
        pParaSet = rParaPropSet.pParaSet;
        pParaSet->mnRefCount++;

        mnOriginalTextPos = rParaPropSet.mnOriginalTextPos;
#ifdef DBG_EXTRACT_BUDATA
        mnCharacters = rParaPropSet.mnCharacters;
#endif
    }
    return *this;
}

PPTCharPropSet::PPTCharPropSet( sal_uInt32 nParagraph ) :
    pCharSet        ( new ImplPPTCharPropSet ),
    mnParagraph     ( nParagraph ),
    mpFieldItem     ( NULL )
{
    mnLanguage[ 0 ] = mnLanguage[ 1 ] = mnLanguage[ 2 ] = 0;
}

PPTCharPropSet::PPTCharPropSet( PPTCharPropSet& rCharPropSet )
{
    pCharSet = rCharPropSet.pCharSet;
    pCharSet->mnRefCount++;

    mnParagraph = rCharPropSet.mnParagraph;
    mnOriginalTextPos = rCharPropSet.mnOriginalTextPos;
    maString = rCharPropSet.maString;
    mpFieldItem = ( rCharPropSet.mpFieldItem ) ? new SvxFieldItem( *rCharPropSet.mpFieldItem ) : NULL;
    mnLanguage[ 0 ] = rCharPropSet.mnLanguage[ 0 ];
    mnLanguage[ 1 ] = rCharPropSet.mnLanguage[ 1 ];
    mnLanguage[ 2 ] = rCharPropSet.mnLanguage[ 2 ];
#ifdef DBG_EXTRACT_BUDATA
    mnCharacters = rCharPropSet.mnCharacters;
#endif
}

PPTCharPropSet::PPTCharPropSet( PPTCharPropSet& rCharPropSet, sal_uInt32 nParagraph )
{
    pCharSet = rCharPropSet.pCharSet;
    pCharSet->mnRefCount++;

    mnParagraph = nParagraph;
    mnOriginalTextPos = rCharPropSet.mnOriginalTextPos;
    maString = rCharPropSet.maString;
    mpFieldItem = ( rCharPropSet.mpFieldItem ) ? new SvxFieldItem( *rCharPropSet.mpFieldItem ) : NULL;
    mnLanguage[ 0 ] = mnLanguage[ 1 ] = mnLanguage[ 2 ] = 0;
#ifdef DBG_EXTRACT_BUDATA
    mnCharacters = rCharPropSet.mnCharacters;
#endif
}

PPTCharPropSet::~PPTCharPropSet()
{
    if ( ! ( --pCharSet->mnRefCount ) )
        delete pCharSet;
    delete mpFieldItem;
}

PPTCharPropSet& PPTCharPropSet::operator=( PPTCharPropSet& rCharPropSet )
{
    if ( this != &rCharPropSet )
    {
        if ( ! ( --pCharSet->mnRefCount ) )
            delete pCharSet;
        pCharSet = rCharPropSet.pCharSet;
        pCharSet->mnRefCount++;

        mnOriginalTextPos = rCharPropSet.mnOriginalTextPos;
        mnParagraph = rCharPropSet.mnParagraph;
        maString = rCharPropSet.maString;
        mpFieldItem = ( rCharPropSet.mpFieldItem ) ? new SvxFieldItem( *rCharPropSet.mpFieldItem ) : NULL;
#ifdef DBG_EXTRACT_BUDATA
        mnCharacters = rCharPropSet.mnCharacters;
#endif
    }
    return *this;
}

void PPTCharPropSet::ImplMakeUnique()
{
    if ( pCharSet->mnRefCount > 1 )
    {
        ImplPPTCharPropSet& rOld = *pCharSet;
        rOld.mnRefCount--;
        pCharSet = new ImplPPTCharPropSet( rOld );
        pCharSet->mnRefCount = 1;
    }
}

void PPTCharPropSet::SetFont( UINT16 nFont )
{
    UINT32  nMask = 1 << PPT_CharAttr_Font;
    UINT32  bDoNotMake = pCharSet->mnAttrSet & nMask;

    if ( bDoNotMake )
        bDoNotMake = nFont == pCharSet->mnFont;

    if ( !bDoNotMake )
    {
        ImplMakeUnique();
        pCharSet->mnFont = nFont;
        pCharSet->mnAttrSet |= nMask;
    }
}

void PPTCharPropSet::SetColor( sal_uInt32 nColor )
{
    ImplMakeUnique();
    pCharSet->mnColor = nColor;
    pCharSet->mnAttrSet |= 1 << PPT_CharAttr_FontColor;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

PPTRuler::PPTRuler() :
    nRefCount   ( 1 ),
    nFlags      ( 0 ),
    nTabCount   ( 0 ),
    pTab        ( NULL )
{
}

PPTRuler::~PPTRuler()
{
    delete[] pTab;
};


PPTTextRulerInterpreter::PPTTextRulerInterpreter() :
    mpImplRuler ( new PPTRuler() )
{
}

PPTTextRulerInterpreter::PPTTextRulerInterpreter( PPTTextRulerInterpreter& rRuler )
{
    mpImplRuler = rRuler.mpImplRuler;
    mpImplRuler->nRefCount++;
}

PPTTextRulerInterpreter::PPTTextRulerInterpreter( UINT32 nFileOfs, SdrPowerPointImport& rMan, DffRecordHeader& rHeader, SvStream& rIn ) :
    mpImplRuler ( new PPTRuler() )
{
    if ( nFileOfs != 0xffffffff )
    {
        UINT32 nOldPos = rIn.Tell();
        DffRecordHeader rHd;
        if ( nFileOfs )
        {
            rIn.Seek( nFileOfs );
            rIn >> rHd;
        }
        else
        {
            rHeader.SeekToContent( rIn );
            if ( rMan.SeekToRec( rIn, PPT_PST_TextRulerAtom, rHeader.GetRecEndFilePos(), &rHd ) )
                nFileOfs++;
        }
        if ( nFileOfs )
        {
            sal_Int16   nTCount;
            sal_Int32   i;
            rIn >> mpImplRuler->nFlags;
            if ( mpImplRuler->nFlags & 1 )
                rIn >> mpImplRuler->nDefaultTab;
            if ( mpImplRuler->nFlags & 4 )
            {
                rIn >> nTCount;
                if ( nTCount )
                {
                    mpImplRuler->nTabCount = (UINT16)nTCount;
                    mpImplRuler->pTab = new PPTTabEntry[ mpImplRuler->nTabCount ];
                    for ( i = 0; i < nTCount; i++ )
                    {
                        rIn >> mpImplRuler->pTab[ i ].nOffset
                            >> mpImplRuler->pTab[ i ].nStyle;
                    }
                }
            }
            for ( i = 0; i < 5; i++ )
            {
                if ( mpImplRuler->nFlags & ( 8 << i ) )
                    rIn >> mpImplRuler->nTextOfs[ i ];
                if ( mpImplRuler->nFlags & ( 256 << i ) )
                    rIn >> mpImplRuler->nBulletOfs[ i ];
            }
        }
        rIn.Seek( nOldPos );
    }
}

BOOL PPTTextRulerInterpreter::GetDefaultTab( UINT32 nLevel, UINT16& nValue ) const
{
    if ( ! ( mpImplRuler->nFlags & 1 ) )
        return FALSE;
    nValue = mpImplRuler->nDefaultTab;
    return TRUE;
}

BOOL PPTTextRulerInterpreter::GetTextOfs( UINT32 nLevel, UINT16& nValue ) const
{
    if ( ! ( ( nLevel < 5 ) && ( mpImplRuler->nFlags & ( 8 << nLevel ) ) ) )
        return FALSE;
    nValue = mpImplRuler->nTextOfs[ nLevel ];
    return TRUE;
}

BOOL PPTTextRulerInterpreter::GetBulletOfs( UINT32 nLevel, UINT16& nValue ) const
{
    if ( ! ( ( nLevel < 5 ) && ( mpImplRuler->nFlags & ( 256 << nLevel ) ) ) )
        return FALSE;
    nValue = mpImplRuler->nBulletOfs[ nLevel ];
    return TRUE;
}

PPTTextRulerInterpreter& PPTTextRulerInterpreter::operator=( PPTTextRulerInterpreter& rRuler )
{
    if ( this != &rRuler )
    {
        if ( ! ( --mpImplRuler->nRefCount ) )
            delete mpImplRuler;
        mpImplRuler = rRuler.mpImplRuler;
        mpImplRuler->nRefCount++;
    }
    return *this;
}

PPTTextRulerInterpreter::~PPTTextRulerInterpreter()
{
    if ( ! ( --mpImplRuler->nRefCount ) )
        delete mpImplRuler;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
PPTTextParagraphStyleAtomInterpreter::PPTTextParagraphStyleAtomInterpreter() :
    bValid              ( sal_False ),
    bForbiddenRules     ( sal_False ),
    bHangingPunctuation ( sal_False ),
    bLatinTextWrap      ( sal_False )
{
}

sal_Bool PPTTextParagraphStyleAtomInterpreter::Read( SvStream& rIn, const DffRecordHeader& rRecHd )
{
    bValid = sal_False;
    rRecHd.SeekToContent( rIn );
    sal_uInt32 nDummy32, nFlags, nRecEndPos = rRecHd.GetRecEndFilePos();
    sal_uInt16 nDummy16;

    rIn >> nDummy16
        >> nFlags;

    if ( nFlags & 0xf && ( rIn.Tell() < nRecEndPos ) )
        rIn >> nDummy16;    // BuFlags
    if ( nFlags & 0x80 && ( rIn.Tell() < nRecEndPos ) )
        rIn >> nDummy16;    // BuChar
    if ( nFlags & 0x10 && ( rIn.Tell() < nRecEndPos ) )
        rIn >> nDummy16;    // nBuFont;
    if ( nFlags & 0x40 && ( rIn.Tell() < nRecEndPos ) )
        rIn >> nDummy16;    // nBuHeight;
    if ( nFlags & 0x0020 && ( rIn.Tell() < nRecEndPos ) )
        rIn >> nDummy32;    // nBuColor;
    if ( nFlags & 0x800 && ( rIn.Tell() < nRecEndPos ) )
        rIn >> nDummy16;    // AbsJust!
    if ( nFlags & 0x400 && ( rIn.Tell() < nRecEndPos ) )
        rIn >> nDummy16;
    if ( nFlags & 0x200 && ( rIn.Tell() < nRecEndPos ) )
        rIn >> nDummy16;
    if ( nFlags & 0x100 && ( rIn.Tell() < nRecEndPos ) )
        rIn >> nDummy16;
    if ( nFlags & 0x1000 && ( rIn.Tell() < nRecEndPos ) )
        rIn >> nDummy16;    // LineFeed
    if ( nFlags & 0x2000 && ( rIn.Tell() < nRecEndPos ) )
        rIn >> nDummy16;    // nUpperDist
    if ( nFlags & 0x4000 && ( rIn.Tell() < nRecEndPos ) )
        rIn >> nDummy16;    // nLowerDist
    if ( nFlags & 0x8000 && ( rIn.Tell() < nRecEndPos ) )
        rIn >> nDummy16;
    if ( nFlags & 0x10000 && ( rIn.Tell() < nRecEndPos ) )
        rIn >> nDummy16;
    if ( nFlags & 0xe0000 && ( rIn.Tell() < nRecEndPos ) )
    {
        rIn >> nDummy16;
        if ( nFlags & 0x20000 )
            bForbiddenRules = ( nDummy16 & 1 ) == 1;
        if ( nFlags & 0x40000 )
            bLatinTextWrap = ( nDummy16 & 2 ) == 0;
        if ( nFlags & 0x80000 )
            bHangingPunctuation = ( nDummy16 & 4 ) == 4;
    }
    nFlags &=~ 0xfffff;
    sal_uInt32 nMask = 0x100000;
    while ( nFlags && nMask && ( rIn.Tell() < nRecEndPos ) )
    {
        if ( nFlags & nMask )
        {
            rIn >> nDummy16;
            nFlags ^= nMask;
        }
        nMask <<= 1;
    }
    bValid = rIn.Tell() == nRecEndPos;
    return bValid;
}

PPTTextParagraphStyleAtomInterpreter::~PPTTextParagraphStyleAtomInterpreter()
{

}

////////////////////////////////////////////////////////////////////////////////////////////////////

PPTTextSpecInfo::PPTTextSpecInfo( sal_uInt32 nCharIdx ) :
    nCharIdx        ( nCharIdx ),
    nDontKnow       ( 1 )
{
    nLanguage[ 0 ] = 0x400;
    nLanguage[ 1 ] = 0;
    nLanguage[ 2 ] = 0;
}

PPTTextSpecInfo::~PPTTextSpecInfo()
{
}

PPTTextSpecInfoAtomInterpreter::PPTTextSpecInfoAtomInterpreter() :
    bValid  ( sal_False )
{
}

sal_Bool PPTTextSpecInfoAtomInterpreter::Read( SvStream& rIn, const DffRecordHeader& rRecHd,
    sal_uInt16 nRecordType, const PPTTextSpecInfo* pTextSpecDefault )
{
    bValid = sal_False;
    sal_uInt32  nCharIdx = 0;
    rRecHd.SeekToContent( rIn );

    while ( rIn.Tell() < rRecHd.GetRecEndFilePos() )
    {
        sal_uInt32  nCharCount,
                    nFlags, i;

        if ( nRecordType == PPT_PST_TextSpecInfoAtom )
        {
            rIn >> nCharCount;
            nCharIdx += nCharCount;
        }
        rIn >> nFlags;

        PPTTextSpecInfo* pEntry = new PPTTextSpecInfo( nCharIdx );
        if ( pTextSpecDefault )
        {
            pEntry->nDontKnow = pTextSpecDefault->nDontKnow;
            pEntry->nLanguage[ 0 ] = pTextSpecDefault->nLanguage[ 0 ];
            pEntry->nLanguage[ 1 ] = pTextSpecDefault->nLanguage[ 1 ];
            pEntry->nLanguage[ 2 ] = pTextSpecDefault->nLanguage[ 2 ];
        }
        for ( i = 1; nFlags && i ; i <<= 1 )
        {
            sal_uInt16 nLang = 0;
            switch( nFlags & i )
            {
                case 0 : break;
                case 1 : rIn >> pEntry->nDontKnow; break;
                case 2 : rIn >> nLang; break;
                case 4 : rIn >> nLang; break;
                default :
                {
                    rIn.SeekRel( 2 );
                }
            }
            if ( nLang )
            {
                sal_uInt16 nScriptType = GetI18NScriptTypeOfLanguage( nLang );
                if ( nScriptType & SCRIPTTYPE_LATIN )
                    pEntry->nLanguage[ 0 ] = nLang;
                if ( nScriptType & SCRIPTTYPE_ASIAN )
                    pEntry->nLanguage[ 1 ] = nLang;
                if ( nScriptType & SCRIPTTYPE_COMPLEX )
                    pEntry->nLanguage[ 2 ] = nLang;
            }
            nFlags &= ~i;
        }
        aList.Insert( pEntry, LIST_APPEND );
    }
    bValid = rIn.Tell() == rRecHd.GetRecEndFilePos();
    return bValid;
}

PPTTextSpecInfoAtomInterpreter::~PPTTextSpecInfoAtomInterpreter()
{
    void *pPtr;
    for ( pPtr = aList.First(); pPtr; pPtr = aList.Next() )
        delete (PPTTextSpecInfo*)pPtr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

PPTStyleTextPropReader::PPTStyleTextPropReader( SvStream& rIn, SdrPowerPointImport& rMan, const DffRecordHeader& rTextHeader,
                                                        PPTTextRulerInterpreter& rRuler, const DffRecordHeader& rExtParaHd, sal_uInt32 nInstance )
{
    sal_uInt32 nMerk = rIn.Tell();
    sal_uInt32 nExtParaPos = ( rExtParaHd.nRecType == PPT_PST_ExtendedParagraphAtom ) ? rExtParaHd.nFilePos + 8 : 0;

    String aString;
    DffRecordHeader aTextHd;
    rIn >> aTextHd;
    sal_uInt32 nMaxLen = aTextHd.nRecLen;
    if ( nMaxLen > 0xFFFF )
        nMaxLen = 0xFFFF;

    if( aTextHd.nRecType == PPT_PST_TextCharsAtom )
    {
        sal_uInt32 i;
        sal_Unicode nChar,*pBuf = new sal_Unicode[ ( nMaxLen >> 1 ) + 1 ];
        rIn.Read( pBuf, nMaxLen );
        nMaxLen >>= 1;
        pBuf[ nMaxLen ] = 0;
        sal_Unicode* pPtr = pBuf;
#ifdef __BIGENDIAN
        sal_Unicode nTemp;
        for ( i = 0; i < nMaxLen; i++ )
        {
            nTemp = *pPtr;
            *pPtr++ = ( nTemp << 8 ) | ( nTemp >> 8 );
        }
        pPtr = pBuf;
#endif

        for ( i = 0; i < nMaxLen; pPtr++, i++ )
        {
            nChar = *pPtr;
            if ( !nChar )
                break;
            if ( ( nChar & 0xff00 ) == 0xf000 )         // in this special case we got a symbol
                aSpecMarkerList.Insert( (void*)( i | PPT_SPEC_SYMBOL ), LIST_APPEND );
            else if ( nChar == 0xd )
            {
                if ( nInstance == TSS_TYPE_PAGETITLE )
                    *pPtr = 0xb;
                else
                    aSpecMarkerList.Insert( (void*)( i | PPT_SPEC_NEWLINE ), LIST_APPEND );
            }
        }
        if ( i )
            aString = String( pBuf, (sal_uInt16)i );
        delete[] pBuf;
    }
    else
    {
        sal_Char cLo, *pBuf = new sal_Char[ nMaxLen + 1 ];
        pBuf[ nMaxLen ] = 0;
        rIn.Read( pBuf, nMaxLen );
        sal_Char* pPtr = pBuf;
        while ( ( cLo = *pPtr ) )
        {
            if ( cLo == 0xd )
            {
                if ( nInstance == TSS_TYPE_PAGETITLE )
                    *pPtr = 0xb;
                else
                    aSpecMarkerList.Insert( (void*)( pPtr - pBuf | PPT_SPEC_NEWLINE ), LIST_APPEND );
            }
            pPtr++;
        }
        xub_StrLen nLen = pPtr - pBuf;
        if ( nLen )
            aString = String( pBuf, nLen, RTL_TEXTENCODING_MS_1252 );
        delete[] pBuf;
    }
    if ( aString.Len() )
    {
        sal_uInt32  nMask;
        sal_uInt32  nCharCount, nCharAnzRead = 0;
        sal_Int32   nCharsToRead;
        sal_uInt16  i, j, nDummy16;
        sal_Bool    bTextPropAtom = sal_False;

        sal_uInt16 nStringLen = aString.Len();

        DffRecordHeader aTextHd;

#ifdef DBG_EXTRACT_BUDATA
        sal_uInt16 nParaEntryCount = 0;
        sal_uInt16 nCharEntryCount = 0;
#endif

        rTextHeader.SeekToContent( rIn );
        if ( rMan.SeekToRec( rIn, PPT_PST_StyleTextPropAtom, rTextHeader.GetRecEndFilePos(), &aTextHd ) )
            bTextPropAtom = sal_True;
        while ( nCharAnzRead <= nStringLen )
        {
            PPTParaPropSet aParaPropSet;
            ImplPPTParaPropSet& aSet = *aParaPropSet.pParaSet;
            if ( bTextPropAtom )
            {
                rIn >> nCharCount
                    >> aParaPropSet.pParaSet->mnDepth;  // Einruecktiefe

#ifdef DBG_EXTRACT_BUDATA
                aSet.mnEntryCount = ++nParaEntryCount;
                aParaPropSet.mnCharacters = (sal_uInt16)nCharCount;
#endif
                nCharCount--;

                rIn >> nMask;
                aSet.mnAttrSet = nMask & 0x21fdf7;
                sal_uInt16 nBulFlg = 0;
                if ( nMask & 0xF )
                    rIn >> nBulFlg; // Bullet-HardAttr-Flags
                aSet.mpArry[ PPT_ParaAttr_BulletOn    ] = ( nBulFlg & 1 ) ? 1 : 0;
                aSet.mpArry[ PPT_ParaAttr_BuHardFont  ] = ( nBulFlg & 2 ) ? 1 : 0;
                aSet.mpArry[ PPT_ParaAttr_BuHardColor ] = ( nBulFlg & 4 ) ? 1 : 0;

                if ( nMask & 0x0080 )
                    rIn >> aSet.mpArry[ PPT_ParaAttr_BulletChar ];
                if ( nMask & 0x0010 )
                    rIn >> aSet.mpArry[ PPT_ParaAttr_BulletFont ];
                if ( nMask & 0x0040 )
                {
                    rIn >> aSet.mpArry[ PPT_ParaAttr_BulletHeight ];
                    if ( ! ( ( nMask & ( 1 << PPT_ParaAttr_BuHardHeight ) )
                        && ( nBulFlg && ( 1 << PPT_ParaAttr_BuHardHeight ) ) ) )
                        aSet.mnAttrSet ^= 0x40;
                }
                if ( nMask & 0x0020 )
                {
                    sal_uInt32 nVal32, nHiByte;
                    rIn >> nVal32;
                    nHiByte = nVal32 >> 24;
                    if ( nHiByte <= 8 )
                        nVal32 = nHiByte | PPT_COLSCHEME;
                    aSet.mnBulletColor = nVal32;
                }
                if ( nMask & 0x0F00 )
                {
                    if ( nMask & 0x800 )
                    {   // AbsJust!
                        rIn >> nDummy16;
                        aSet.mpArry[ PPT_ParaAttr_Adjust ] = nDummy16 & 3;
                    }
                    if ( nMask & 0x400 )
                        rIn >> nDummy16;
                    if ( nMask & 0x200 )
                        rIn >> nDummy16;
                    if ( nMask & 0x100 )
                        rIn >> nDummy16;
                }
                if ( nMask & 0x1000 )
                    rIn >> aSet.mpArry[ PPT_ParaAttr_LineFeed ];
                if ( nMask & 0x2000 )
                    rIn >> aSet.mpArry[ PPT_ParaAttr_UpperDist ];
                if ( nMask & 0x4000 )
                    rIn >> aSet.mpArry[ PPT_ParaAttr_LowerDist ];
                if ( nMask & 0x8000 )
                    rIn >> nDummy16;
                if ( nMask & 0x10000 )
                    rIn >> nDummy16;
                if ( nMask & 0xe0000 )
                {
                    rIn >> nDummy16;
                    if ( nMask & 0x20000 )
                        aSet.mpArry[ PPT_ParaAttr_AsianLB_1 ] = nDummy16 & 1;
                    if ( nMask & 0x40000 )
                        aSet.mpArry[ PPT_ParaAttr_AsianLB_2 ] = ( nDummy16 >> 1 ) & 1;
                    if ( nMask & 0x80000 )
                        aSet.mpArry[ PPT_ParaAttr_AsianLB_3 ] = ( nDummy16 >> 2 ) & 1;
                    aSet.mnAttrSet |= ( ( nMask >> 17 ) & 7 ) << PPT_ParaAttr_AsianLB_1;
                }
                if ( nMask & 0x200000 )                     // #88602#
                    rIn >> aSet.mpArry[ PPT_ParaAttr_BiDi ];
            }
            else
                nCharCount = nStringLen;

            if ( rRuler.GetTextOfs( aParaPropSet.pParaSet->mnDepth, aSet.mpArry[ PPT_ParaAttr_TextOfs ] ) )
                aSet.mnAttrSet |= 1 << PPT_ParaAttr_TextOfs;
            if ( rRuler.GetBulletOfs( aParaPropSet.pParaSet->mnDepth, aSet.mpArry[ PPT_ParaAttr_BulletOfs ] ) )
                aSet.mnAttrSet |= 1 << PPT_ParaAttr_BulletOfs;
            if ( rRuler.GetDefaultTab( aParaPropSet.pParaSet->mnDepth, aSet.mpArry[ PPT_ParaAttr_DefaultTab ] ) )
                aSet.mnAttrSet |= 1 << PPT_ParaAttr_DefaultTab;

            if ( ( nCharCount > nStringLen ) || ( nStringLen - ( nCharAnzRead + nCharCount ) < 0 ) )
            {
                bTextPropAtom = sal_False;
                nCharCount = nStringLen - nCharAnzRead;
                // please fix the right hand side of
                // PPTParaPropSet& PPTParaPropSet::operator=(PPTParaPropSet&),
                // it should be a const reference
                PPTParaPropSet aTmpPPTParaPropSet;
                aParaPropSet = aTmpPPTParaPropSet;
                DBG_ERROR( "SJ:PPTStyleTextPropReader::could not get this PPT_PST_StyleTextPropAtom by reading the paragraph attributes" );
            }
            PPTParaPropSet* pPara = new PPTParaPropSet( aParaPropSet );
            pPara->mnOriginalTextPos = nCharAnzRead;
            aParaPropList.Insert( pPara, LIST_APPEND );
            if ( nCharCount )
            {
                sal_uInt32   nCount;
                const sal_Unicode* pDat = aString.GetBuffer() + nCharAnzRead;
                for ( nCount = 0; nCount < nCharCount; nCount++ )
                {
                    if ( pDat[ nCount ] == 0xd )
                    {
                        pPara = new PPTParaPropSet( aParaPropSet );
                        pPara->mnOriginalTextPos = nCharAnzRead + nCount + 1;
                        aParaPropList.Insert( pPara, LIST_APPEND );
                    }
                }
            }
            nCharAnzRead += nCharCount + 1;
        }

        sal_Bool bEmptyParaPossible = sal_True;
        sal_uInt32 nCurrentPara = nCharAnzRead = 0;
        sal_uInt32 nCurrentSpecMarker = (sal_uInt32)aSpecMarkerList.First();
        sal_uInt32 nExtBuInd = 0x3c00;

        while ( nCharAnzRead < nStringLen )
        {
            sal_uInt32 nBuFlags, nNumberingType, nLatestParaUpdate = 0xffffffff;
            sal_uInt16 nBuInstance, nBuStart;

            PPTCharPropSet aCharPropSet( nCurrentPara );
            if ( bTextPropAtom )
            {
                rIn >> nDummy16;
                nCharCount = nDummy16;
                rIn >> nDummy16;
                nCharsToRead = nStringLen - ( nCharAnzRead + nCharCount );
                if ( nCharsToRead < 0 )
                {
                    nCharCount = nStringLen - nCharAnzRead;
                    if ( nCharsToRead < -1 )
                    {
                        bTextPropAtom = sal_False;
                        DBG_ERROR( "SJ:PPTStyleTextPropReader::could not get this PPT_PST_StyleTextPropAtom by reading the character attributes" );
                    }
                }
                ImplPPTCharPropSet& aSet = *aCharPropSet.pCharSet;

#ifdef DBG_EXTRACT_BUDATA
                aSet.mnEntryCount = ++nCharEntryCount;
                aCharPropSet.mnCharacters = (sal_uInt16)nCharCount;
#endif
                // character attributes
                rIn >> nMask;

                if ( (sal_uInt16)nMask )
                {
                    aSet.mnAttrSet |= (sal_uInt16)nMask;
                    rIn >> aSet.mnFlags;
                }
                // Die Sortierung der Char-Attribs ist etwas durcheinander...
                static sal_uInt16 __READONLY_DATA aCharAttrTable[16] =
                {
                    16, 21, 22, 23, 17, 18, 19, 20,
                    24, 25, 26, 27, 28, 29, 30, 31
                };
                for ( i = 0; i < 16; i++ )
                {
                    // Reihenfolge ist bei einfachen Textobjekten normal...
                    j = aCharAttrTable[ i ];
                    if ( nMask & ( 1 << j ) )
                    {
                        switch ( j )
                        {
                            case PPT_CharAttr_Symbol :
                            {
                                rIn >> aSet.mnSymbolFont;
                                aSet.mnAttrSet |= 1 << PPT_CharAttr_Symbol;
                            }
                            break;
                            case PPT_CharAttr_Font :
                            {
                                rIn >> aSet.mnFont;
                                aSet.mnAttrSet |= 1 << PPT_CharAttr_Font;
                            }
                            break;
                            case PPT_CharAttr_AsianOrComplexFont :
                            {
                                rIn >> aSet.mnAsianOrComplexFont;
                                aSet.mnAttrSet |= 1 << PPT_CharAttr_AsianOrComplexFont;
                            }
                            break;
                            case PPT_CharAttr_Unknown2 :
                            {
                                rIn >> aSet.mnUnknown2;
                                aSet.mnAttrSet |= 1 << PPT_CharAttr_Unknown2;
                            }
                            break;
                            case PPT_CharAttr_FontHeight :
                            {
                                rIn >> aSet.mnFontHeight;
                                aSet.mnAttrSet |= 1 << PPT_CharAttr_FontHeight;
                            }
                            break;
                            case PPT_CharAttr_FontColor :
                            {
                                sal_uInt32 nVal;
                                rIn >> nVal;
                                if ( !( nVal & 0xff000000 ) )
                                    nVal = PPT_COLSCHEME_HINTERGRUND;
                                aSet.mnColor = nVal;
                                aSet.mnAttrSet |= 1 << PPT_CharAttr_FontColor;
                            }
                            break;
                            case PPT_CharAttr_Escapement :
                            {
                                rIn >> aSet.mnEscapement;
                                aSet.mnAttrSet |= 1 << PPT_CharAttr_Escapement;
                            }
                            break;
                            default :
                                rIn >> nDummy16;
                        }
                    }
                }
            }
            else
                nCharCount = nStringLen;

            /* SJ: get the new ppt2000 numrules (f???!!!, why are the bullet information
               stored in the portion and not paragraph section ?
               that makes no sense and cost me days of my life.)
            */
            if ( nExtParaPos && ( ( nMask & 0x3c00 ) != nExtBuInd ) )
            {
                nExtBuInd = nMask & 0x3c00;
                if ( nExtParaPos < rExtParaHd.GetRecEndFilePos() )
                {
                    sal_uInt32 nOldPos = rIn.Tell();
                    rIn.Seek( nExtParaPos );
                    rIn >> nBuFlags;
                    if ( nBuFlags & 0x800000 )
                        rIn >> nBuInstance;
                    if ( nBuFlags & 0x01000000 )
                        rIn >> nNumberingType;
                    if ( nBuFlags & 0x02000000 )
                        rIn >> nBuStart;
                    nExtParaPos = rIn.Tell() + 8;
                    rIn.Seek( nOldPos );
                }
            }

            sal_uInt32 nLen;
            while( nCharCount )
            {
                if ( nExtParaPos && ( nLatestParaUpdate != nCurrentPara ) && ( nCurrentPara < aParaPropList.Count() ) )
                {
                    PPTParaPropSet* pPropSet = (PPTParaPropSet*)aParaPropList.GetObject( nCurrentPara );
                    pPropSet->pParaSet->nBuFlags = nBuFlags;
                    if ( nBuFlags & 0x800000 )
                        pPropSet->pParaSet->nBuInstance = nBuInstance;
                    if ( nBuFlags & 0x01000000 )
                        pPropSet->pParaSet->nNumberingType = nNumberingType;
                    if ( nBuFlags & 0x02000000 )
                        pPropSet->pParaSet->nBuStart = nBuStart;
                    nLatestParaUpdate = nCurrentPara;
                }

                aCharPropSet.mnOriginalTextPos = nCharAnzRead;
                if ( nCurrentSpecMarker &&  ( ( nCurrentSpecMarker & 0xffff ) < ( nCharAnzRead + nCharCount ) ) )
                {
                    if ( nCurrentSpecMarker & PPT_SPEC_NEWLINE )
                    {
                        nLen = ( nCurrentSpecMarker & 0xffff ) - nCharAnzRead;
                        if ( nLen )
                            aCharPropSet.maString = String( aString, (sal_uInt16)nCharAnzRead, (sal_uInt16)nLen );
                        else if ( bEmptyParaPossible )
                            aCharPropSet.maString = String();
                        if ( nLen || bEmptyParaPossible )
                            aCharPropList.Insert( new PPTCharPropSet( aCharPropSet, nCurrentPara ), LIST_APPEND );
                        nCurrentPara++;
                        nLen++;
                        nCharAnzRead += nLen;
                        nCharCount -= nLen;
                        bEmptyParaPossible = sal_True;
                    }
                    else if ( nCurrentSpecMarker & PPT_SPEC_SYMBOL )
                    {
                        if ( ( nCurrentSpecMarker & 0xffff ) != nCharAnzRead )
                        {
                            nLen = ( nCurrentSpecMarker & 0xffff ) - nCharAnzRead;
                            aCharPropSet.maString = String( aString, (sal_uInt16)nCharAnzRead, (sal_uInt16)nLen );
                            aCharPropList.Insert( new PPTCharPropSet( aCharPropSet, nCurrentPara ), LIST_APPEND );
                            nCharCount -= nLen;
                            nCharAnzRead += nLen;
                        }
                        PPTCharPropSet* pCPropSet = new PPTCharPropSet( aCharPropSet, nCurrentPara );
                        pCPropSet->maString = aString.GetChar( (sal_uInt16)nCharAnzRead );
                        if ( aCharPropSet.pCharSet->mnAttrSet & ( 1 << PPT_CharAttr_Symbol ) )
                            pCPropSet->SetFont( aCharPropSet.pCharSet->mnSymbolFont );
                        aCharPropList.Insert( pCPropSet, LIST_APPEND );
                        nCharCount--;
                        nCharAnzRead++;
                        bEmptyParaPossible = sal_False;
                    }
                    nCurrentSpecMarker = (sal_uInt32)aSpecMarkerList.Next();
                }
                else
                {
                    aCharPropSet.maString = String( aString, (sal_uInt16)nCharAnzRead, (sal_uInt16)nCharCount );
                    aCharPropList.Insert( new PPTCharPropSet( aCharPropSet, nCurrentPara ), LIST_APPEND );
                    nCharAnzRead += nCharCount;
                    bEmptyParaPossible = sal_False;
                    break;
                }
            }
         }
        if ( aCharPropList.Count() && ( ((PPTCharPropSet*)aCharPropList.Last())->mnParagraph != nCurrentPara ) )
        {
            PPTCharPropSet* pCharPropSet = new PPTCharPropSet( *(PPTCharPropSet*)aCharPropList.Last(), nCurrentPara );
            pCharPropSet->maString = String();
            pCharPropSet->mnOriginalTextPos = nStringLen - 1;
            aCharPropList.Insert( pCharPropSet, LIST_APPEND );
        }
    }
    rIn.Seek( nMerk );
}

PPTStyleTextPropReader::~PPTStyleTextPropReader()
{
    void* pTmp;
    for ( pTmp = aParaPropList.First(); pTmp; pTmp = aParaPropList.Next() )
        delete (PPTParaPropSet*)pTmp;
    for ( pTmp = aCharPropList.First(); pTmp; pTmp = aCharPropList.Next() )
        delete (PPTCharPropSet*)pTmp;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

struct FieldEntry
{
    UINT32  nFieldType;
    UINT32  nFieldStartPos;
    UINT32  nFieldEndPos;
    String  aFieldUrl;

    FieldEntry( UINT32 nType, UINT32 nStart, UINT32 nEnd )
    {
        nFieldType = nType;
        nFieldStartPos = nStart;
        nFieldEndPos = nEnd;
    }
    FieldEntry( FieldEntry& rFieldEntry )
    {
        nFieldType = rFieldEntry.nFieldType;
        nFieldStartPos = rFieldEntry.nFieldStartPos;
        nFieldEndPos = rFieldEntry.nFieldEndPos;
        aFieldUrl = rFieldEntry.aFieldUrl;
    }
};


PPTPortionObj::PPTPortionObj( const PPTStyleSheet& rStyleSheet, UINT32 nInstance, UINT32 nDepth ) :
    mrStyleSheet    ( rStyleSheet ),
    mnInstance      ( nInstance ),
    mnDepth         ( ( nDepth > 4 ) ? 4 : nDepth ),
    PPTCharPropSet  ( 0 )
{
}

PPTPortionObj::PPTPortionObj( PPTCharPropSet& rCharPropSet, const PPTStyleSheet& rStyleSheet, UINT32 nInstance, UINT32 nDepth ) :
    PPTCharPropSet  ( rCharPropSet ),
    mrStyleSheet    ( rStyleSheet ),
    mnInstance      ( nInstance ),
    mnDepth         ( nDepth )
{
}

PPTPortionObj::PPTPortionObj( PPTPortionObj& rPortionObj ) :
    PPTCharPropSet      ( rPortionObj ),
    mrStyleSheet        ( rPortionObj.mrStyleSheet ),
    mnInstance          ( rPortionObj.mnInstance ),
    mnDepth             ( rPortionObj.mnDepth )
{
}

PPTPortionObj::~PPTPortionObj()
{
}

BOOL PPTPortionObj::HasTabulator()
{
    BOOL bRetValue =    FALSE;
    sal_Int32           nCount;
    const sal_Unicode*  pPtr = maString.GetBuffer();
    for ( nCount = 0; nCount < maString.Len(); nCount++ )
    {
        if ( pPtr[ nCount ] == 0x9 )
        {
            bRetValue = TRUE;
            break;
        }

    }
    return bRetValue;
}

BOOL PPTPortionObj::GetAttrib( UINT32 nAttr, UINT32& nRetValue, UINT32 nInstanceInSheet )
{
    UINT32  nMask = 1 << nAttr;
    nRetValue = 0;

    UINT32 bIsHardAttribute = ( ( pCharSet->mnAttrSet & nMask ) != 0 ) ? 1 : 0;

    if ( bIsHardAttribute )
    {
        switch ( nAttr )
        {
            case PPT_CharAttr_Bold :
            case PPT_CharAttr_Italic :
            case PPT_CharAttr_Underline :
            case PPT_CharAttr_Shadow :
            case PPT_CharAttr_Strikeout :
            case PPT_CharAttr_Embossed :
                nRetValue = ( pCharSet->mnFlags & nMask ) ? 1 : 0;
            break;
            case PPT_CharAttr_Font :
                nRetValue = pCharSet->mnFont;
            break;
            case PPT_CharAttr_AsianOrComplexFont :
                nRetValue = pCharSet->mnAsianOrComplexFont;
            break;
            case PPT_CharAttr_FontHeight :
                nRetValue = pCharSet->mnFontHeight;
            break;
            case PPT_CharAttr_FontColor :
                nRetValue = pCharSet->mnColor;
            break;
            case PPT_CharAttr_Escapement :
                nRetValue = pCharSet->mnEscapement;
            break;
            default :
                DBG_ERROR( "SJ:PPTPortionObj::GetAttrib ( hard attribute does not exist )" );
        }
    }
    else
    {
        const PPTCharLevel& rCharLevel = mrStyleSheet.mpCharSheet[ mnInstance ]->maCharLevel[ mnDepth ];
        PPTCharLevel* pCharLevel = NULL;
        if ( ( nInstanceInSheet == 0xffffffff )
                || ( mnDepth && ( ( mnInstance == TSS_TYPE_SUBTITLE ) || ( mnInstance == TSS_TYPE_TEXT_IN_SHAPE ) ) ) )
            bIsHardAttribute = 1;
        else if ( nInstanceInSheet != mnInstance )
            pCharLevel = &mrStyleSheet.mpCharSheet[ nInstanceInSheet ]->maCharLevel[ mnDepth ];
        switch( nAttr )
        {
            case PPT_CharAttr_Bold :
            case PPT_CharAttr_Italic :
            case PPT_CharAttr_Underline :
            case PPT_CharAttr_Shadow :
            case PPT_CharAttr_Strikeout :
            case PPT_CharAttr_Embossed :
            {
                nRetValue = ( rCharLevel.mnFlags & nMask ) ? 1 : 0;
                if ( pCharLevel )
                {
                    UINT32 nTmp = ( pCharLevel->mnFlags & nMask ) ? 1 : 0;
                    if ( nRetValue != nTmp )
                        bIsHardAttribute = 1;
                }
                if ( nRetValue && ( nInstanceInSheet == TSS_TYPE_TEXT_IN_SHAPE ) )
                {
                    nRetValue = 0;          // no inheritance for standard textobjects
                    bIsHardAttribute = 1;   // this attribute must be hard formatted
                }
            }
            break;
            case PPT_CharAttr_Font :
            {
                nRetValue = rCharLevel.mnFont;
                if ( pCharLevel && ( nRetValue != pCharLevel->mnFont ) )
                    bIsHardAttribute = 1;
            }
            break;
            case PPT_CharAttr_AsianOrComplexFont :
            {
                nRetValue = rCharLevel.mnAsianOrComplexFont;
                if ( pCharLevel && ( nRetValue != pCharLevel->mnAsianOrComplexFont ) )
                    bIsHardAttribute = 1;
            }
            break;
            case PPT_CharAttr_FontHeight :
            {
                nRetValue = rCharLevel.mnFontHeight;
                if ( pCharLevel && ( nRetValue != pCharLevel->mnFontHeight ) )
                    bIsHardAttribute = 1;
            }
            break;
            case PPT_CharAttr_FontColor :
            {
                nRetValue = rCharLevel.mnFontColor;
                if ( pCharLevel && ( nRetValue != pCharLevel->mnFontColor ) )
                    bIsHardAttribute = 1;
            }
            break;
            case PPT_CharAttr_Escapement :
            {
                nRetValue = rCharLevel.mnEscapement;
                if ( pCharLevel && ( nRetValue != pCharLevel->mnEscapement ) )
                    bIsHardAttribute = 1;
            }
            break;
            default :
                DBG_ERROR( "SJ:PPTPortionObj::GetAttrib ( attribute does not exist )" );
        }
    }
    return (BOOL)bIsHardAttribute;
}

void PPTPortionObj::ApplyTo(  SfxItemSet& rSet, SdrPowerPointImport& rManager, UINT32 nInstanceInSheet )
{
    ApplyTo( rSet, rManager, nInstanceInSheet, NULL );
}

void PPTPortionObj::ApplyTo(  SfxItemSet& rSet, SdrPowerPointImport& rManager, UINT32 nInstanceInSheet, const PPTTextObj* pTextObj )
{
    UINT32  nVal;
    if ( GetAttrib( PPT_CharAttr_Bold, nVal, nInstanceInSheet ) )
        rSet.Put( SvxWeightItem( nVal != 0 ? WEIGHT_BOLD : WEIGHT_NORMAL ) );

    if ( GetAttrib( PPT_CharAttr_Italic, nVal, nInstanceInSheet ) )
        rSet.Put( SvxPostureItem( nVal != 0 ? ITALIC_NORMAL : ITALIC_NONE ) );

    if ( GetAttrib( PPT_CharAttr_Underline, nVal, nInstanceInSheet ) )
        rSet.Put( SvxUnderlineItem( nVal != 0 ? UNDERLINE_SINGLE : UNDERLINE_NONE ) );

    if ( GetAttrib( PPT_CharAttr_Shadow, nVal, nInstanceInSheet ) )
        rSet.Put( SvxShadowedItem( nVal != 0 ) );

    if ( GetAttrib( PPT_CharAttr_Strikeout, nVal, nInstanceInSheet ) )
        rSet.Put( SvxCrossedOutItem( nVal != 0 ? STRIKEOUT_SINGLE : STRIKEOUT_NONE ) );

    sal_uInt32  nAsianFontId = 0xffff;
    if ( GetAttrib( PPT_CharAttr_AsianOrComplexFont, nAsianFontId, nInstanceInSheet ) )
    {
        if ( nAsianFontId != 0xffff )
        {
            PptFontEntityAtom* pFontEnityAtom = rManager.GetFontEnityAtom( nAsianFontId );
            if ( pFontEnityAtom )
            {
                rSet.Put( SvxFontItem( pFontEnityAtom->eFamily, pFontEnityAtom->aName,
                            String(), pFontEnityAtom->ePitch, pFontEnityAtom->eCharSet, EE_CHAR_FONTINFO_CJK ) );
                rSet.Put( SvxFontItem( pFontEnityAtom->eFamily, pFontEnityAtom->aName,
                            String(), pFontEnityAtom->ePitch, pFontEnityAtom->eCharSet, EE_CHAR_FONTINFO_CTL ) );
            }
        }
    }
    if ( GetAttrib( PPT_CharAttr_Font, nVal, nInstanceInSheet ) )
    {
        PptFontEntityAtom* pFontEnityAtom = rManager.GetFontEnityAtom( nVal );
        if ( pFontEnityAtom )
            rSet.Put( SvxFontItem( pFontEnityAtom->eFamily, pFontEnityAtom->aName, String(), pFontEnityAtom->ePitch, pFontEnityAtom->eCharSet ) );
    }
    if ( GetAttrib( PPT_CharAttr_FontHeight, nVal, nInstanceInSheet ) ) // Schriftgrad in Point
    {
        sal_uInt32 nHeight = rManager.ScalePoint( nVal );
        rSet.Put( SvxFontHeightItem( nHeight, 100, EE_CHAR_FONTHEIGHT ) );
        rSet.Put( SvxFontHeightItem( nHeight, 100, EE_CHAR_FONTHEIGHT_CJK ) );
        rSet.Put( SvxFontHeightItem( nHeight, 100, EE_CHAR_FONTHEIGHT_CTL ) );
    }

    if ( GetAttrib( PPT_CharAttr_Embossed, nVal, nInstanceInSheet ) )
        rSet.Put( SvxCharReliefItem( nVal != 0 ? RELIEF_EMBOSSED : RELIEF_NONE ) );
    if ( nVal ) /* if Embossed is set, the font color depends to the fillstyle/color of the object,
                   if the object has no fillstyle, the font color depends to fillstyle of the background */
    {
        Color aDefColor( COL_BLACK );
        MSO_FillType eFillType = mso_fillSolid;
        if ( rManager.GetPropertyValue( DFF_Prop_fNoFillHitTest ) & 0x10 )
            eFillType = (MSO_FillType)rManager.GetPropertyValue( DFF_Prop_fillType, mso_fillSolid );
        else
            eFillType = mso_fillBackground;
        switch( eFillType )
        {
            case mso_fillShade :
            case mso_fillShadeCenter :
            case mso_fillShadeShape :
            case mso_fillShadeScale :
            case mso_fillShadeTitle :
            case mso_fillSolid :
                aDefColor = rManager.MSO_CLR_ToColor( rManager.GetPropertyValue( DFF_Prop_fillColor ) );
            break;
            case mso_fillPattern :
                aDefColor = rManager.MSO_CLR_ToColor( rManager.GetPropertyValue( DFF_Prop_fillBackColor ) );
            break;
            case mso_fillTexture :
            {
                Graphic aGraf;
                if ( rManager.GetBLIP( rManager.GetPropertyValue( DFF_Prop_fillBlip ), aGraf ) )
                {
                    Bitmap aBmp( aGraf.GetBitmap() );
                    Size aSize( aBmp.GetSizePixel() );
                    if ( aSize.Width() && aSize.Height() )
                    {
                        if ( aSize.Width () > 64 )
                            aSize.Width () = 64;
                        if ( aSize.Height() > 64 )
                            aSize.Height() = 64;

                        ULONG nRt = 0, nGn = 0, nBl = 0;
                        BitmapReadAccess*   pAcc = aBmp.AcquireReadAccess();
                        if( pAcc )
                        {
                            const long nWidth = aSize.Width();
                            const long nHeight = aSize.Height();

                            if( pAcc->HasPalette() )
                            {
                                for( long nY = 0L; nY < nHeight; nY++ )
                                {
                                    for( long nX = 0L; nX < nWidth; nX++ )
                                    {
                                        const BitmapColor& rCol = pAcc->GetPaletteColor( (BYTE) pAcc->GetPixel( nY, nX ) );
                                        nRt+=rCol.GetRed(); nGn+=rCol.GetGreen(); nBl+=rCol.GetBlue();
                                    }
                                }
                            }
                            else
                            {
                                for( long nY = 0L; nY < nHeight; nY++ )
                                {
                                    for( long nX = 0L; nX < nWidth; nX++ )
                                    {
                                        const BitmapColor aCol( pAcc->GetPixel( nY, nX ) );
                                        nRt+=aCol.GetRed(); nGn+=aCol.GetGreen(); nBl+=aCol.GetBlue();
                                    }
                                }
                            }
                            aBmp.ReleaseAccess( pAcc );
                            sal_uInt32 nC = ( aSize.Width() * aSize.Height() );
                            nRt /= nC;
                            nGn /= nC;
                            nBl /= nC;
                            aDefColor = Color(sal_uInt8( nRt ), sal_uInt8( nGn ),sal_uInt8( nBl ) );
                        }
                    }
                }
            }
            break;
            case mso_fillBackground :
            {
                if ( pTextObj ) // the textobject is needed
                {
                    const SfxItemSet* pItemSet = pTextObj->GetBackground();
                    if ( pItemSet )
                    {
                        const SfxPoolItem* pFillStyleItem = NULL;
                        pItemSet->GetItemState( XATTR_FILLSTYLE, FALSE, &pFillStyleItem );
                        if ( pFillStyleItem )
                        {
                            XFillStyle eFillStyle = ((XFillStyleItem*)pFillStyleItem)->GetValue();
                            switch( eFillStyle )
                            {
                                case XFILL_SOLID :
                                {
                                    const SfxPoolItem* pFillColorItem = NULL;
                                    pItemSet->GetItemState( XATTR_FILLCOLOR, FALSE, &pFillColorItem );
                                    if ( pFillColorItem )
                                        aDefColor = ((XColorItem*)pFillColorItem)->GetValue();
                                }
                                break;
                                case XFILL_GRADIENT :
                                {
                                    const SfxPoolItem* pGradientItem = NULL;
                                    pItemSet->GetItemState( XATTR_FILLGRADIENT, FALSE, &pGradientItem );
                                    if ( pGradientItem )
                                        aDefColor = ((XFillGradientItem*)pGradientItem)->GetValue().GetStartColor();
                                }
                                break;
                                case XFILL_HATCH :
                                case XFILL_BITMAP :
                                    aDefColor = Color( COL_WHITE );
                                break;
                            }
                        }
                    }
                }
            }
            break;
//          case mso_fillPicture :
        }
        rSet.Put( SvxColorItem( aDefColor, EE_CHAR_COLOR ) );
    }
    else
    {
        if ( GetAttrib( PPT_CharAttr_FontColor, nVal, nInstanceInSheet ) )  // Textfarbe (4Byte-Arg)
        {
            Color aCol( rManager.MSO_CLR_ToColor( nVal ) );
            rSet.Put( SvxColorItem( aCol, EE_CHAR_COLOR ) );
            if ( nInstanceInSheet == 0xffffffff )
                mrStyleSheet.mpCharSheet[ mnInstance ]->maCharLevel[ mnDepth ].mnFontColorInStyleSheet = aCol;
        }
        else if ( nVal & 0x0f000000 )   // this is not a hard attribute, but maybe the page has a different colerscheme,
        {                               // so that in this case we must use a hard color attribute
            Color   aCol( rManager.MSO_CLR_ToColor( nVal ) );
            Color&  aColorInSheet = mrStyleSheet.mpCharSheet[ mnInstance ]->maCharLevel[ mnDepth ].mnFontColorInStyleSheet;
            if ( aColorInSheet != aCol )
                rSet.Put( SvxColorItem( aCol, EE_CHAR_COLOR ) );
        }
    }

    if ( GetAttrib( PPT_CharAttr_Escapement, nVal, nInstanceInSheet ) ) // Hoch/Tiefstellung in %
    {
        sal_uInt16  nEsc = 0;
        sal_uInt8   nProp = 100;

        if ( nVal )
        {
            nEsc = (sal_Int16)nVal;
            nProp = DFLT_ESC_PROP;
        }
        SvxEscapementItem aItem( nEsc, nProp );
        rSet.Put( aItem );
    }
    if ( mnLanguage[ 0 ] )
        rSet.Put( SvxLanguageItem( mnLanguage[ 0 ], EE_CHAR_LANGUAGE ) );
    if ( mnLanguage[ 1 ] )
        rSet.Put( SvxLanguageItem( mnLanguage[ 1 ], EE_CHAR_LANGUAGE_CJK ) );
    if ( mnLanguage[ 2 ] )
        rSet.Put( SvxLanguageItem( mnLanguage[ 2 ], EE_CHAR_LANGUAGE_CTL ) );
}

SvxFieldItem* PPTPortionObj::GetTextField()
{
    if ( mpFieldItem )
        return new SvxFieldItem( *mpFieldItem );
    return NULL;
}

//  -----------------------------------------------------------------------

PPTParagraphObj::PPTParagraphObj( const PPTStyleSheet& rStyleSheet, UINT32 nInstance, UINT16 nDepth ) :
    mrStyleSheet            ( rStyleSheet ),
    mnInstance              ( nInstance ),
    mnPortionCount          ( 0 ),
    mpPortionList           ( NULL ),
    PPTNumberFormatCreator  ( NULL ),
    mbTab                   ( TRUE )        // style sheets always have to get the right tabulator setting
{
    if ( nDepth > 4 )
        nDepth = 4;
    pParaSet->mnDepth = nDepth;
}

PPTParagraphObj::PPTParagraphObj( PPTStyleTextPropReader& rPropReader, const PPTStyleSheet& rStyleSheet,
                                    UINT32 nInstance, PPTTextRulerInterpreter& rRuler ) :
    PPTParaPropSet          ( *( (PPTParaPropSet*)rPropReader.aParaPropList.GetCurObject() ) ),
    PPTTextRulerInterpreter ( rRuler ),
    mrStyleSheet            ( rStyleSheet ),
    mnInstance              ( nInstance ),
    PPTNumberFormatCreator  ( NULL ),
    mbTab                   ( FALSE ),
    mnCurrentObject         ( 0 ),
    mnPortionCount          ( 0 ),
    mpPortionList           ( NULL )
{
    sal_uInt32 nCurPos = rPropReader.aCharPropList.GetCurPos();
    PPTCharPropSet* pCharPropSet = (PPTCharPropSet*)rPropReader.aCharPropList.GetCurObject();
    if ( pCharPropSet )
    {
        sal_uInt32 nCurrentParagraph = pCharPropSet->mnParagraph;
        for ( ; pCharPropSet && ( pCharPropSet->mnParagraph == nCurrentParagraph ); pCharPropSet = (PPTCharPropSet*)rPropReader.aCharPropList.Next() )
            mnPortionCount++;   // counting number of portions that are part of this paragraph
        pCharPropSet = (PPTCharPropSet*)rPropReader.aCharPropList.Seek( nCurPos );

        mpPortionList = new PPTPortionObj*[ mnPortionCount ];
        for ( UINT32 i = 0; i < mnPortionCount; i++ )
        {
            if ( pCharPropSet )
            {
                PPTPortionObj* pPPTPortion = new PPTPortionObj( *pCharPropSet, rStyleSheet, nInstance, pParaSet->mnDepth );
                mpPortionList[ i ] = pPPTPortion;
                if ( !mbTab )
                    mbTab = mpPortionList[ i ]->HasTabulator();
            }
            else
            {
                DBG_ERROR( "SJ:PPTParagraphObj::It seems that there are missing some textportions" );
                mpPortionList[ i ] = NULL;
            }
            pCharPropSet = (PPTCharPropSet*)rPropReader.aCharPropList.Next();
        }
    }
}

PPTParagraphObj::~PPTParagraphObj()
{
    ImplClear();
}

void PPTParagraphObj::AppendPortion( PPTPortionObj& rPPTPortion )
{
    UINT32 i;
    PPTPortionObj** mpOldPortionList = mpPortionList;
    mpPortionList = new PPTPortionObj*[ ++mnPortionCount ];
    for ( i = 0; i < mnPortionCount - 1; i++ )
        mpPortionList[ i ] = mpOldPortionList[ i ];
    delete[] mpOldPortionList;
    mpPortionList[ mnPortionCount - 1 ] = new PPTPortionObj( rPPTPortion );
    if ( !mbTab )
        mbTab = mpPortionList[ mnPortionCount - 1 ]->HasTabulator();
}

void PPTParagraphObj::UpdateBulletRelSize( sal_uInt32& nBulletRelSize ) const
{
    if ( nBulletRelSize > 0x7fff )      // a negative value is the absolute bullet height
    {
        sal_uInt16 nFontHeight = 0;
        if ( mpPortionList )
        {
            PPTPortionObj* pPortion = mpPortionList[ 0 ];
            if ( pPortion && ( pPortion->pCharSet->mnAttrSet & ( 1 << PPT_CharAttr_FontHeight ) ) )
                nFontHeight = pPortion->pCharSet->mnFontHeight;
        }
        // if we do not have a hard attributed fontheight, the fontheight is taken from the style
        if ( !nFontHeight )
            nFontHeight = mrStyleSheet.mpCharSheet[ mnInstance ]->maCharLevel[ pParaSet->mnDepth ].mnFontHeight;
        nBulletRelSize = nFontHeight ? ((-((sal_Int16)nBulletRelSize)) * 100 ) / nFontHeight : 100;
    }
}

BOOL PPTParagraphObj::GetAttrib( UINT32 nAttr, UINT32& nRetValue, UINT32 nInstanceInSheet )
{
    UINT32  nMask = 1 << nAttr;
    nRetValue = 0;

    if ( nAttr > 21 )
    {
        DBG_ERROR( "SJ:PPTParagraphObj::GetAttrib - attribute does not exist" );
        return FALSE;
    }

    UINT32 bIsHardAttribute = ( ( pParaSet->mnAttrSet & nMask ) != 0 ) ? 1 : 0;

    if ( bIsHardAttribute )
    {
        if ( nAttr == PPT_ParaAttr_BulletColor )
        {
            sal_Bool bHardBulletColor;
            if ( pParaSet->mnAttrSet & ( 1 << PPT_ParaAttr_BuHardColor ) )
                bHardBulletColor = pParaSet->mpArry[ PPT_ParaAttr_BuHardColor ] != 0;
            else
                bHardBulletColor = ( mrStyleSheet.mpParaSheet[ mnInstance ]->maParaLevel[ pParaSet->mnDepth ].mnBuFlags
                                        & ( 1 << PPT_ParaAttr_BuHardColor ) ) != 0;
            if ( bHardBulletColor )
                nRetValue = pParaSet->mnBulletColor;
            else
            {
                nRetValue = PPT_COLSCHEME_TEXT_UND_ZEILEN;
                if ( ( nInstanceInSheet != 0xffffffff ) && mnPortionCount )
                {
                    PPTPortionObj* pPortion = mpPortionList[ 0 ];
                    if ( pPortion )
                    {
                        if ( pPortion->pCharSet->mnAttrSet & ( 1 << PPT_CharAttr_FontColor ) )
                            nRetValue = pPortion->pCharSet->mnColor;
                        else
                            nRetValue = mrStyleSheet.mpCharSheet[ nInstanceInSheet ]->maCharLevel[ pParaSet->mnDepth ].mnFontColor;
                    }
                }
            }
        }
        else if ( nAttr == PPT_ParaAttr_BulletFont )
        {
            sal_Bool bHardBuFont;
            if ( pParaSet->mnAttrSet & ( 1 << PPT_ParaAttr_BuHardFont ) )
                bHardBuFont = pParaSet->mpArry[ PPT_ParaAttr_BuHardFont ] != 0;
            else
                bHardBuFont = ( mrStyleSheet.mpParaSheet[ mnInstance ]->maParaLevel[ pParaSet->mnDepth ].mnBuFlags
                                        & ( 1 << PPT_ParaAttr_BuHardFont ) ) != 0;
            if ( bHardBuFont )
                nRetValue = pParaSet->mpArry[ PPT_ParaAttr_BulletFont ];
            else
            {
                // it is the font used which assigned to the first character of the following text
                nRetValue = 0;
                if ( ( nInstanceInSheet != 0xffffffff ) && mnPortionCount )
                {
                    PPTPortionObj* pPortion = mpPortionList[ 0 ];
                    if ( pPortion )
                    {
                        if ( pPortion->pCharSet->mnAttrSet & ( 1 << PPT_CharAttr_Font ) )
                            nRetValue = pPortion->pCharSet->mnFont;
                        else
                            nRetValue = mrStyleSheet.mpCharSheet[ nInstanceInSheet ]->maCharLevel[ pParaSet->mnDepth ].mnFont;
                    }
                }
            }
        }
        else
            nRetValue = pParaSet->mpArry[ nAttr ];
    }
    else
    {
        const PPTParaLevel& rParaLevel = mrStyleSheet.mpParaSheet[ mnInstance ]->maParaLevel[ pParaSet->mnDepth ];

        PPTParaLevel* pParaLevel = NULL;
        if ( ( nInstanceInSheet == 0xffffffff )
            || ( pParaSet->mnDepth && ( ( mnInstance == TSS_TYPE_SUBTITLE ) || ( mnInstance == TSS_TYPE_TEXT_IN_SHAPE ) ) ) )
            bIsHardAttribute = 1;
        else if ( nInstanceInSheet != mnInstance )
            pParaLevel = &mrStyleSheet.mpParaSheet[ nInstanceInSheet ]->maParaLevel[ pParaSet->mnDepth ];
        switch ( nAttr )
        {
            case PPT_ParaAttr_BulletOn :
            {
                nRetValue = rParaLevel.mnBuFlags & ( 1 << PPT_ParaAttr_BulletOn );
                if ( pParaLevel )
                {
                    if ( nRetValue != ( (sal_uInt32)pParaLevel->mnBuFlags & ( 1 << PPT_ParaAttr_BulletOn ) ) )
                        bIsHardAttribute = 1;
                }
            }
            break;
            case PPT_ParaAttr_BuHardFont :
            case PPT_ParaAttr_BuHardColor :
            case PPT_ParaAttr_BuHardHeight :
                DBG_ERROR( "SJ:PPTParagraphObj::GetAttrib - this attribute does not make sense" );
            break;
            case PPT_ParaAttr_BulletChar :
            {
                nRetValue = rParaLevel.mnBulletChar;
                if ( pParaLevel && ( nRetValue != pParaLevel->mnBulletChar ) )
                    bIsHardAttribute = 1;
            }
            break;
            case PPT_ParaAttr_BulletFont :
            {
                sal_Bool bHardBuFont;
                if ( pParaSet->mnAttrSet & ( 1 << PPT_ParaAttr_BuHardFont ) )
                    bHardBuFont = pParaSet->mpArry[ PPT_ParaAttr_BuHardFont ] != 0;
                else
                    bHardBuFont = ( rParaLevel.mnBuFlags & ( 1 << PPT_ParaAttr_BuHardFont ) ) != 0;
                if ( bHardBuFont )
                {
                    nRetValue = rParaLevel.mnBulletFont;
                    if ( pParaLevel && ( nRetValue != pParaLevel->mnBulletFont ) )
                        bIsHardAttribute = 1;
                }
                else
                {
                    if ( mnPortionCount )
                    {
                        PPTPortionObj* pPortion = mpPortionList[ 0 ];
                        if ( pPortion )
                            bIsHardAttribute = pPortion->GetAttrib( PPT_CharAttr_Font, nRetValue, nInstanceInSheet );
                    }
                    else
                    {
                        nRetValue = mrStyleSheet.mpCharSheet[ mnInstance ]->maCharLevel[ pParaSet->mnDepth ].mnFont;
                        bIsHardAttribute = 1;
                    }
                }
            }
            break;
            case PPT_ParaAttr_BulletHeight :
            {
                nRetValue = rParaLevel.mnBulletHeight;
                if ( pParaLevel && ( nRetValue != pParaLevel->mnBulletHeight ) )
                    bIsHardAttribute = 1;
            }
            break;
            case PPT_ParaAttr_BulletColor :
            {
                sal_Bool bHardBulletColor;
                if ( pParaSet->mnAttrSet & ( 1 << PPT_ParaAttr_BuHardColor ) )
                    bHardBulletColor = pParaSet->mpArry[ PPT_ParaAttr_BuHardColor ] != 0;
                else
                    bHardBulletColor = ( rParaLevel.mnBuFlags & ( 1 << PPT_ParaAttr_BuHardColor ) ) != 0;
                if ( bHardBulletColor )
                {
                    nRetValue = rParaLevel.mnBulletColor;
                    if ( pParaLevel && ( nRetValue != pParaLevel->mnBulletColor ) )
                        bIsHardAttribute = 1;
                }
                else
                {
                    if ( mnPortionCount )
                    {
                        PPTPortionObj* pPortion = mpPortionList[ 0 ];
                        if ( pPortion )
                            bIsHardAttribute = pPortion->GetAttrib( PPT_CharAttr_FontColor, nRetValue, nInstanceInSheet );
                    }
                    else
                    {
                        nRetValue = mrStyleSheet.mpCharSheet[ mnInstance ]->maCharLevel[ pParaSet->mnDepth ].mnFontColor;
                        bIsHardAttribute = 1;
                    }
                }
            }
            break;
            case PPT_ParaAttr_Adjust :
            {
                nRetValue = rParaLevel.mnAdjust;
                if ( pParaLevel && ( nRetValue != pParaLevel->mnAdjust ) )
                    bIsHardAttribute = 1;
            }
            break;
            case PPT_ParaAttr_LineFeed :
            {
                nRetValue = rParaLevel.mnLineFeed;
                if ( pParaLevel && ( nRetValue != pParaLevel->mnLineFeed ) )
                    bIsHardAttribute = 1;
            }
            break;
            case PPT_ParaAttr_UpperDist :
            {
                nRetValue = rParaLevel.mnUpperDist;
                if ( pParaLevel && ( nRetValue != pParaLevel->mnUpperDist ) )
                    bIsHardAttribute = 1;
            }
            break;
            case PPT_ParaAttr_LowerDist :
            {
                nRetValue = rParaLevel.mnLowerDist;
                if ( pParaLevel && ( nRetValue != pParaLevel->mnLowerDist ) )
                    bIsHardAttribute = 1;
            }
            break;
            case PPT_ParaAttr_TextOfs :
            {
                nRetValue = rParaLevel.mnTextOfs;
                if ( pParaLevel && ( nRetValue != pParaLevel->mnTextOfs ) )
                    bIsHardAttribute = 1;
            }
            break;
            case PPT_ParaAttr_BulletOfs :
            {
                nRetValue = rParaLevel.mnBulletOfs;
                if ( pParaLevel && ( nRetValue != pParaLevel->mnBulletOfs ) )
                    bIsHardAttribute = 1;
            }
            break;
            case PPT_ParaAttr_DefaultTab :
            {
                nRetValue = rParaLevel.mnDefaultTab;
                if ( pParaLevel && ( nRetValue != pParaLevel->mnBulletOfs ) )
                    bIsHardAttribute = 1;
            }
            break;
            case PPT_ParaAttr_AsianLB_1 :
            {
                nRetValue = rParaLevel.mnAsianLineBreak & 1;
                if ( pParaLevel && ( nRetValue != ( (sal_uInt32)pParaLevel->mnAsianLineBreak & 1 ) ) )
                    bIsHardAttribute = 1;
            }
            break;
            case PPT_ParaAttr_AsianLB_2 :
            {
                nRetValue = ( rParaLevel.mnAsianLineBreak >> 1 ) & 1;
                if ( pParaLevel && ( nRetValue != ( ( (sal_uInt32)pParaLevel->mnAsianLineBreak >> 1 ) & 1 ) ) )
                    bIsHardAttribute = 1;
            }
            break;
            case PPT_ParaAttr_AsianLB_3 :
            {
                nRetValue = ( rParaLevel.mnAsianLineBreak >> 2 ) & 1;
                if ( pParaLevel && ( nRetValue != ( ( (sal_uInt32)pParaLevel->mnAsianLineBreak >> 2 ) & 1 ) ) )
                    bIsHardAttribute = 1;
            }
            break;
            case PPT_ParaAttr_BiDi :
            {
                nRetValue = rParaLevel.mnBiDi;
                if ( pParaLevel && ( nRetValue != pParaLevel->mnBiDi ) )
                    bIsHardAttribute = 1;
            }
            break;
        }
    }
    return (BOOL)bIsHardAttribute;
}

void PPTParagraphObj::ApplyTo( SfxItemSet& rSet, SdrPowerPointImport& rManager, UINT32 nInstanceInSheet, const PPTParagraphObj* pPrev )
{
    INT16   nVal2;
    UINT32  nVal, nUpperDist, nLowerDist;
    UINT32  nInstance = mnInstance;

    if ( ( nInstanceInSheet != 0xffffffff ) || ( GetLevel() <= 1 ) )
    {
        SvxNumBulletItem* pNumBulletItem = mrStyleSheet.mpNumBulletItem[ nInstance ];
        if ( pNumBulletItem )
        {
            SvxNumberFormat aNumberFormat( SVX_NUM_CHAR_SPECIAL );
            aNumberFormat.SetBulletChar( ' ' );
            if ( GetNumberFormat( rManager, aNumberFormat, this, nInstanceInSheet ) )
            {
                SvxNumBulletItem aNewNumBulletItem( *pNumBulletItem );
                SvxNumRule* pRule = aNewNumBulletItem.GetNumRule();
                if ( pRule )
                {
                    pRule->SetLevel( GetLevel(), aNumberFormat );
                    if ( nInstanceInSheet == 0xffffffff )
                    {
                        sal_uInt16 i, n;
                        for ( i = 1; i < pRule->GetLevelCount(); i++ )
                        {
                            n = ( i > 4 ) ? 4 : i;

                            sal_uInt32 nInst = nInstance;
                            switch( nInst )
                            {
                                case TSS_TYPE_BODY :
                                case TSS_TYPE_HALFBODY :
                                case TSS_TYPE_QUARTERBODY :
                                {
                                    if ( !n )
                                        nInst = 0;
                                    else
                                        n -= 1;
                                }
                                break;
                            }
                            SvxNumberFormat aNumberFormat( pRule->GetLevel( i ) );
                            const PPTParaLevel& rParaLevel = mrStyleSheet.mpParaSheet[ nInst ]->maParaLevel[ n ];
                            const PPTCharLevel& rCharLevel = mrStyleSheet.mpCharSheet[ nInst ]->maCharLevel[ n ];
                            sal_uInt32 nColor;
                            if ( rParaLevel.mnBuFlags & ( 1 << PPT_ParaAttr_BuHardColor ) )
                                nColor = rParaLevel.mnBulletColor;
                            else
                                nColor = rCharLevel.mnFontColor;
                            aNumberFormat.SetBulletColor( rManager.MSO_CLR_ToColor( nColor ) );
                            pRule->SetLevel( i, aNumberFormat );
                        }
                    }
                    rSet.Put( aNewNumBulletItem );
                }
            }
        }
    }
    UINT32 nIsBullet;
    GetAttrib( PPT_ParaAttr_BulletOn, nIsBullet, nInstanceInSheet );
    rSet.Put( SfxUInt16Item( EE_PARA_BULLETSTATE, nIsBullet == 0 ? 0 : 1 ) );

    if ( GetAttrib( PPT_ParaAttr_TextOfs, nVal, nInstanceInSheet ) )
    {
        SvxLRSpaceItem aLRSpaceItem( ITEMID_LRSPACE );
        aLRSpaceItem.SetLeft( (UINT16)(((UINT32) nVal * 2540 ) / ( 72 * 8 ) ) );
        rSet.Put( aLRSpaceItem );
    }

    if ( GetAttrib( PPT_ParaAttr_Adjust, nVal, nInstanceInSheet ) )
    {
        if ( nVal <= 3 )
        {   // Absatzausrichtung
            static SvxAdjust __READONLY_DATA aAdj[ 4 ] = { SVX_ADJUST_LEFT, SVX_ADJUST_CENTER, SVX_ADJUST_RIGHT, SVX_ADJUST_BLOCK };
            rSet.Put( SvxAdjustItem( aAdj[ nVal ] ) );
        }
    }

    if ( GetAttrib( PPT_ParaAttr_AsianLB_1, nVal, nInstanceInSheet ) )
        rSet.Put( SfxBoolItem( EE_PARA_FORBIDDENRULES, nVal != 0 ) );
    if ( GetAttrib( PPT_ParaAttr_AsianLB_3, nVal, nInstanceInSheet ) )
        rSet.Put( SfxBoolItem( EE_PARA_HANGINGPUNCTUATION, nVal != 0 ) );

    if ( GetAttrib( PPT_ParaAttr_BiDi, nVal, nInstanceInSheet ) )
        rSet.Put( SvxFrameDirectionItem( nVal == 1 ? FRMDIR_HORI_RIGHT_TOP : FRMDIR_HORI_LEFT_TOP, EE_PARA_WRITINGDIR ) );

    // LineSpacing
    PPTPortionObj* pPortion = First();
    BOOL bIsHardAttribute = GetAttrib( PPT_ParaAttr_LineFeed, nVal, nInstanceInSheet );
    nVal2 = (INT16)nVal;
    if ( ( pPrev == NULL ) && bIsHardAttribute && pPortion && ( nVal2 > 100 ) )
    {   // first paragraph: each linespacing will be converted to 'at least' to take the spacing above the line
        UINT32 nFontHeight;
        pPortion->GetAttrib( PPT_CharAttr_FontHeight, nFontHeight, nInstanceInSheet );
        nVal2 = -(INT16)( ( nFontHeight * nVal * 8 ) / 100 );
        bIsHardAttribute = TRUE;
    }
    sal_uInt32 nFont;
    if ( pPortion && pPortion->GetAttrib( PPT_CharAttr_Font, nFont, nInstanceInSheet ) )
        bIsHardAttribute = TRUE;

    if ( bIsHardAttribute )
    {
        if ( pPortion && ( nVal2 > 200 ) )
        {
            UINT32 nFontHeight;
            pPortion->GetAttrib( PPT_CharAttr_FontHeight, nFontHeight, nInstanceInSheet );
            nVal2 = -(sal_Int16)( ( nFontHeight * nVal * 8 ) / 100 );
        }
        SvxLineSpacingItem aItem( 200, EE_PARA_SBL );
        if ( nVal2 <= 0 )
            aItem.SetLineHeight( (UINT16)( rManager.ScalePoint( -nVal2 ) / 8 ) );
        else
        {
            sal_uInt8 nPropLineSpace = (BYTE)nVal2;
            if ( pPortion )
            {
                PptFontEntityAtom* pAtom = rManager.GetFontEnityAtom( nFont );
                if ( pAtom )
                    nPropLineSpace = (sal_uInt8)((double)nVal2 * pAtom->fScaling + 0.5);
            }
            aItem.SetPropLineSpace( nPropLineSpace );
            aItem.GetLineSpaceRule() = SVX_LINE_SPACE_AUTO;
        }
        rSet.Put( aItem );
    }

    // Paragraph Spacing
    UINT32 nFontHeight = 0;
    bIsHardAttribute = ( (UINT32)GetAttrib( PPT_ParaAttr_UpperDist, nUpperDist, nInstanceInSheet ) +
        (UINT32)GetAttrib( PPT_ParaAttr_LowerDist, nLowerDist, nInstanceInSheet ) ) != 0;
    if ( ( nUpperDist > 0 ) || ( nLowerDist > 0 ) )
    {
        if ( mnPortionCount )
        {
            mpPortionList[ mnPortionCount - 1 ]->GetAttrib( PPT_CharAttr_FontHeight, nFontHeight, nInstanceInSheet );
            if ( ((INT16)nUpperDist) > 0 )
                nUpperDist = - (sal_Int16)( ( nFontHeight * nUpperDist * 100 ) / 1000 );
            if ( ((INT16)nLowerDist) > 0 )
                nLowerDist = - (sal_Int16)( ( nFontHeight * nLowerDist * 100 ) / 1000 );
        }
        bIsHardAttribute = TRUE;
    }
    if ( bIsHardAttribute )
    {
        SvxULSpaceItem aULSpaceItem( ITEMID_ULSPACE );
        nVal2 = (INT16)nUpperDist;
        if ( nVal2 <= 0 )
            aULSpaceItem.SetUpper( (UINT16)(((UINT32) - nVal2 * 2540 ) / ( 72 * 8 ) ) );
        else
        {
            aULSpaceItem.SetUpperValue( 0 );
            aULSpaceItem.SetPropUpper( (UINT16)nUpperDist == 100 ? 101 : (UINT16)nUpperDist );
        }
        nVal2 = (INT16)nLowerDist;
        if ( nVal2 <= 0 )
            aULSpaceItem.SetLower( (UINT16)(((UINT32) - nVal2 * 2540 ) / ( 72 * 8 ) ) );
        else
        {
            aULSpaceItem.SetLowerValue( 0 );
            aULSpaceItem.SetPropLower( (UINT16)nLowerDist == 100 ? 101 : (UINT16)nLowerDist );
        }
        rSet.Put( aULSpaceItem );
    }

    if ( mbTab )    // makes it sense to apply tabsettings
    {
        UINT32 i, nDefaultTab, nTab, nTextOfs = 0;
        UINT32 nLatestManTab = 0;
        GetAttrib( PPT_ParaAttr_TextOfs, nTextOfs, nInstanceInSheet );
        GetAttrib( PPT_ParaAttr_BulletOfs, nTab, nInstanceInSheet );
        GetAttrib( PPT_ParaAttr_BulletOn, i, nInstanceInSheet );
        GetAttrib( PPT_ParaAttr_DefaultTab, nDefaultTab, nInstanceInSheet );
        SvxTabStopItem aTabItem( 0, 0 );
        if ( GetTabCount() )
        {
            for ( i = 0; i < GetTabCount(); i++ )
            {
                SvxTabAdjust eTabAdjust;
                nTab = GetTabOffsetByIndex( (sal_uInt16)i );
                switch( GetTabStyleByIndex( (sal_uInt16)i ) )
                {
                    case 1 :    eTabAdjust = SVX_TAB_ADJUST_CENTER; break;
                    case 2 :    eTabAdjust = SVX_TAB_ADJUST_RIGHT; break;
                    case 3 :    eTabAdjust = SVX_TAB_ADJUST_DECIMAL; break;
                    default :   eTabAdjust = SVX_TAB_ADJUST_LEFT;
                }
                if ( nTab > nTextOfs )
                    aTabItem.Insert( SvxTabStop( (UINT16)( ( ( nTab - nTextOfs ) * 2540 ) / 576 ), eTabAdjust ) );
            }
            nLatestManTab = nTab;
        }
        if ( nIsBullet == 0 )
            aTabItem.Insert( SvxTabStop( (sal_uInt16)0 ) );
        if ( nDefaultTab )
        {
            nTab = ( nTextOfs > nLatestManTab ) ? nTextOfs : nLatestManTab;
            nTab /= nDefaultTab;
            nTab = nDefaultTab * ( 1 + nTab );
            for ( i = 0; ( i < 20 ) && ( nTab < 0x1b00 ); i++ )
            {
                aTabItem.Insert( SvxTabStop( (UINT16)( ( ( nTab - nTextOfs ) * 2540 ) / 576 ) ) );
                nTab += nDefaultTab;
            }
        }
        rSet.Put( aTabItem );
    }
}

UINT16 PPTParagraphObj::GetLevel()
{
    UINT16 nRetValue = pParaSet->mnDepth;
    switch ( mnInstance )
    {
        case TSS_TYPE_BODY :
        case TSS_TYPE_HALFBODY :
        case TSS_TYPE_QUARTERBODY :
            nRetValue++;
    }
    return nRetValue;
}

UINT32 PPTParagraphObj::GetTextSize()
{
    UINT32 nCount, nRetValue = 0;
    for ( UINT32 i = 0; i < mnPortionCount; i++ )
    {
        PPTPortionObj* pPortionObj = mpPortionList[ i ];
        nCount = pPortionObj->Count();
        if ( ( !nCount ) && pPortionObj->mpFieldItem )
            nCount++;
        nRetValue += nCount;
    }
    return nRetValue;
}

PPTPortionObj* PPTParagraphObj::First()
{
    mnCurrentObject = 0;
    if ( !mnPortionCount )
        return NULL;
    return mpPortionList[ 0 ];
}

PPTPortionObj* PPTParagraphObj::Next()
{
    UINT32 i = mnCurrentObject + 1;
    if ( i >= mnPortionCount )
        return NULL;
    mnCurrentObject++;
    return mpPortionList[ i ];
}

void PPTParagraphObj::ImplClear()
{
    for ( void* pPtr = First(); pPtr; pPtr = Next() )
        delete (PPTPortionObj*)pPtr;
    delete[] mpPortionList;
}

PPTFieldEntry::~PPTFieldEntry()
{
    delete pField1;
    delete pField2;
    delete pString;
};

void PPTFieldEntry::SetDateTime( UINT32 nVal )
{
    SvxDateFormat eDateFormat( SVXDATEFORMAT_APPDEFAULT );
    SvxTimeFormat eTimeFormat( SVXTIMEFORMAT_APPDEFAULT );
    // ID auswerten
    switch( nVal )
    {
        case 0:
        case 6:
            eDateFormat = SVXDATEFORMAT_A;
        break;
        case 1:
            eDateFormat = SVXDATEFORMAT_F;
        break;
        case 2:
        case 3:
            eDateFormat = SVXDATEFORMAT_D;
        break;
        case 4:
        case 5:
            eDateFormat = SVXDATEFORMAT_C;
        break;
        case 7:
            eDateFormat = SVXDATEFORMAT_A;
        case 9:
            eTimeFormat = SVXTIMEFORMAT_24_HM;
        break;
        case 8:
            eDateFormat = SVXDATEFORMAT_A;
        case 11:
            eTimeFormat = SVXTIMEFORMAT_12_HM;
        break;
        case 10:
            eTimeFormat = SVXTIMEFORMAT_24_HMS;
        break;
        case 12:
            eTimeFormat = SVXTIMEFORMAT_12_HMS;
        break;
    }
    if ( eDateFormat != SVXDATEFORMAT_APPDEFAULT )
        pField1 = new SvxFieldItem( SvxDateField( Date(), SVXDATETYPE_VAR, eDateFormat ) );
    if ( eTimeFormat != SVXTIMEFORMAT_APPDEFAULT )
    {
        SvxFieldItem* pFieldItem = new SvxFieldItem( SvxExtTimeField( Time(), SVXTIMETYPE_VAR, eTimeFormat ) );
        if ( pField1 )
            pField2 = pFieldItem;
        else
            pField1 = pFieldItem;
    }
}

#ifdef DBG_EXTRACT_BUDATA
ByteString Implgethex( sal_uInt32 nHex, sal_Int32 nBytes )
{
    ByteString aHexString;
    sal_Int32 nBitsLeft = nBytes << 3;
    while( nBitsLeft )
    {
        sal_uInt32 nNumb = ( nHex << ( 32 - nBitsLeft ) ) >> 28;
        if ( nNumb > 9 )
            nNumb += 'A' - 10;
        else
            nNumb += '0';
        aHexString+=(sal_Char)nNumb;
        nBitsLeft -= 4;
    }
    aHexString += ' ';
    return aHexString;
}
#endif

//  -----------------------------------------------------------------------

PPTTextObj::PPTTextObj( SvStream& rIn, SdrPowerPointImport& rSdrPowerPointImport, PptSlidePersistEntry& rPersistEntry, DffObjData* pObjData ) :
    mpImplTextObj   ( new ImplPPTTextObj( rPersistEntry ) )
{
    mpImplTextObj->mnRefCount = 1;
    mpImplTextObj->mnShapeId = 0;
    mpImplTextObj->mnShapeMaster = 0;
    mpImplTextObj->mpPlaceHolderAtom = NULL;
    mpImplTextObj->mnMappedInstance = mpImplTextObj->mnInstance = 4;
    mpImplTextObj->mnCurrentObject = 0;
    mpImplTextObj->mnParagraphCount = 0;
    mpImplTextObj->mpParagraphList = NULL;
    mpImplTextObj->mnTextFlags = 0;
    mpImplTextObj->meShapeType = ( pObjData && pObjData->bShapeType ) ? pObjData->eShapeType : mso_sptMin;

    DffRecordHeader aExtParaHd;
    aExtParaHd.nRecType = 0;    // set empty

    UINT32 bStatus = TRUE;

    DffRecordHeader aShapeContainerHd;
    rIn >> aShapeContainerHd;

    if ( ( pObjData == NULL ) || ( pObjData->bShapeType ) )
    {
        PPTExtParaProv* pExtParaProv = rSdrPowerPointImport.pPPTStyleSheet->pExtParaProv;
        if ( pObjData )
        {
            mpImplTextObj->mnShapeId = pObjData->nShapeId;
            if ( pObjData->nSpFlags & SP_FHAVEMASTER )
                mpImplTextObj->mnShapeMaster = rSdrPowerPointImport.GetPropertyValue( DFF_Prop_hspMaster, 0 );
        }
        ////////////////
        // ClientData //
        ////////////////
        if ( rSdrPowerPointImport.maShapeRecords.SeekToContent( rIn, DFF_msofbtClientData, SEEK_FROM_CURRENT_AND_RESTART ) )
        {
            UINT32 nOldPos = rIn.Tell();
            DffRecordHeader& aClientDataContainerHd = *rSdrPowerPointImport.maShapeRecords.Current();
            DffRecordHeader aPlaceHolderAtomHd;
            if ( rSdrPowerPointImport.SeekToRec( rIn, PPT_PST_OEPlaceholderAtom, aClientDataContainerHd.GetRecEndFilePos(), &aPlaceHolderAtomHd ) )
            {
                mpImplTextObj->mpPlaceHolderAtom = new PptOEPlaceholderAtom;
                rIn >> *( mpImplTextObj->mpPlaceHolderAtom );
            }
            rIn.Seek( nOldPos );
            DffRecordHeader aProgTagHd;
            if ( rSdrPowerPointImport.SeekToContentOfProgTag( 9, rIn, aClientDataContainerHd, aProgTagHd ) )
            {
                rIn >> aExtParaHd;
            }
        }

        ///////////////////
        // ClientTextBox //
        ///////////////////
        if ( rSdrPowerPointImport.maShapeRecords.SeekToContent( rIn, DFF_msofbtClientTextbox, SEEK_FROM_CURRENT_AND_RESTART ) )
        {
            DffRecordHeader aClientTextBoxHd( *rSdrPowerPointImport.maShapeRecords.Current() );
            UINT32  nTextRulerAtomOfs = 0;  // case of zero -> this atom may be found in aClientDataContainerHd;
                                            // case of -1 -> ther is no atom of this kind
                                            // else -> this is the fileofs where we can get it

            //////////////////////////////////////
            // checkout if this is a referenced //
            // textobj, if so the we will patch //
            // the ClientTextBoxHd for a        //
            // equivalent one                   //
            //////////////////////////////////////
            DffRecordHeader aTextHd;
            if ( rSdrPowerPointImport.SeekToRec( rIn, PPT_PST_OutlineTextRefAtom, aClientTextBoxHd.GetRecEndFilePos(), &aTextHd ) )
            {
                UINT32 nRefNum;
                rIn >> nRefNum;

                if ( rSdrPowerPointImport.SeekToRec( rIn, PPT_PST_TextRulerAtom, aClientTextBoxHd.GetRecEndFilePos() ) )
                    nTextRulerAtomOfs = rIn.Tell();
                else
                    nTextRulerAtomOfs = 0xffffffff;

                UINT32 nInstance = 0;
                switch( rSdrPowerPointImport.eAktPageKind )
                {
                    case PPT_NOTEPAGE :
                        nInstance++;
                    case PPT_MASTERPAGE :
                        nInstance++;
                    case PPT_SLIDEPAGE :
                    break;
                    default :
                        bStatus = FALSE;
                }
                if ( bStatus )
                {
                    sal_uInt32 nSlideId = rSdrPowerPointImport.GetAktPageId();
                    if ( !nSlideId )
                        bStatus = FALSE;
                    else
                    {
                        if ( !aExtParaHd.nRecType )
                        {
                            UINT32 nOldPos = rIn.Tell();
                            // try to locate the referenced ExtendedParaHd
                            DffRecordHeader* pHd = pExtParaProv->
                                                        aExtendedPresRules.GetRecordHeader( PPT_PST_ExtendedParagraphHeaderAtom,
                                                                                            SEEK_FROM_CURRENT_AND_RESTART );
                            DffRecordHeader     aPresRuleHd;
                            DffRecordHeader*    pFirst = pHd;

                            UINT32  nTmpSlideId, nTmpRef;
                            while ( pHd )
                            {
                                pHd->SeekToContent( rIn );
                                rIn >> nTmpSlideId
                                    >> nTmpRef;         // this seems to be the instance

                                if ( ( nTmpSlideId == nSlideId ) && ( pHd->nRecInstance == nRefNum ) )
                                {
                                    pHd->SeekToEndOfRecord( rIn );
                                    rIn >> aPresRuleHd;
                                    if ( aPresRuleHd.nRecType == PPT_PST_ExtendedParagraphAtom )
                                    {
                                        aExtParaHd = aPresRuleHd;
                                        break;
                                    }
                                }
                                pHd = pExtParaProv->
                                        aExtendedPresRules.GetRecordHeader( PPT_PST_ExtendedParagraphHeaderAtom,
                                                                            SEEK_FROM_CURRENT_AND_RESTART );
                                if ( pHd == pFirst )
                                    break;
                            }
                            rIn.Seek( nOldPos );
                        }
                        // now pHd points to the right SlideListWithText Container
                        PptSlidePersistList* pPageList = rSdrPowerPointImport.GetPageList( rSdrPowerPointImport.eAktPageKind );
                        PptSlidePersistEntry* pE = NULL;
                        if ( pPageList && ( rSdrPowerPointImport.nAktPageNum < pPageList->Count() ) )
                            pE = (*pPageList)[ rSdrPowerPointImport.nAktPageNum ];
                        if ( (!pE) || (!pE->nSlidePersistStartOffset) || ( pE->aPersistAtom.nSlideId != nSlideId ) )
                            bStatus = sal_False;
                        else
                        {
                            rIn.Seek( pE->nSlidePersistStartOffset );
                            // now we got the right page and are searching for the right
                            // TextHeaderAtom
                            while ( rIn.Tell() < pE->nSlidePersistEndOffset )
                            {
                                rIn >> aClientTextBoxHd;
                                if ( aClientTextBoxHd.nRecType == PPT_PST_TextHeaderAtom )
                                {
                                    if ( aClientTextBoxHd.nRecInstance == nRefNum )
                                    {
                                        aClientTextBoxHd.SeekToEndOfRecord( rIn );
                                        break;
                                    }
                                }
                                aClientTextBoxHd.SeekToEndOfRecord( rIn );
                            }
                            if ( rIn.Tell() > pE->nSlidePersistEndOffset )
                                bStatus = FALSE;
                            else
                            {   // patching the RecordHeader
                                aClientTextBoxHd.nFilePos -= DFF_COMMON_RECORD_HEADER_SIZE;
                                aClientTextBoxHd.nRecLen += DFF_COMMON_RECORD_HEADER_SIZE;
                                aClientTextBoxHd.nRecType = DFF_msofbtClientTextbox;
                                aClientTextBoxHd.nRecVer = DFF_PSFLAG_CONTAINER;

                                // we have to calculate the correct record len
                                DffRecordHeader aTmpHd;
                                while ( rIn.Tell() < pE->nSlidePersistEndOffset )
                                {
                                    rIn >> aTmpHd;
                                    if ( ( aTmpHd.nRecType == PPT_PST_SlidePersistAtom ) || ( aTmpHd.nRecType == PPT_PST_TextHeaderAtom ) )
                                        break;
                                    aTmpHd.SeekToEndOfRecord( rIn );
                                    aClientTextBoxHd.nRecLen += aTmpHd.nRecLen + DFF_COMMON_RECORD_HEADER_SIZE;
                                }
                                aClientTextBoxHd.SeekToContent( rIn );
                            }
                        }
                    }
                }
            }

            if ( bStatus )
            {
                if ( rSdrPowerPointImport.SeekToRec( rIn, PPT_PST_TextHeaderAtom, aClientTextBoxHd.GetRecEndFilePos(), &aTextHd ) )
                {
                    // TextHeaderAtom is always the first Atom
                    UINT16 nInstance;
                    rIn >> nInstance;   // this number tells us the TxMasterStyleAtom Instance
                    if ( nInstance > 8 )
                        nInstance = 4;
                    aTextHd.SeekToEndOfRecord( rIn );
                    mpImplTextObj->mnInstance = nInstance;

                    UINT32 nFilePos = rIn.Tell();
                    if ( rSdrPowerPointImport.SeekToRec2( PPT_PST_TextBytesAtom, PPT_PST_TextCharsAtom, aClientTextBoxHd.GetRecEndFilePos() ) )
                    {
                        PPTTextRulerInterpreter aTextRulerInterpreter( nTextRulerAtomOfs, rSdrPowerPointImport,
                                                                        aClientTextBoxHd, rIn );

                        PPTStyleTextPropReader aStyleTextPropReader( rIn, rSdrPowerPointImport, aClientTextBoxHd,
                                                                        aTextRulerInterpreter, aExtParaHd, nInstance );
                        sal_uInt32 nParagraphs = mpImplTextObj->mnParagraphCount = aStyleTextPropReader.aParaPropList.Count();

#ifdef DBG_EXTRACT_BUDATA

                        // testcode
                        sal_uInt32 nExtParaPos = ( aExtParaHd.nRecType == PPT_PST_ExtendedParagraphAtom ) ? aExtParaHd.nFilePos + 8 : 0;
                        if ( nExtParaPos )
                        {
                            sal_uInt32 i, nOldPos = rIn.Tell();
                            rIn.Seek( nExtParaPos );
                            sal_Char nLineBreak = 0xa;
                            SvFileStream aFileStream( String( ByteString( "d:\\numberings.txt" ), RTL_TEXTENCODING_UTF8 ), STREAM_WRITE );
                            aFileStream.Seek( STREAM_SEEK_TO_END );

                            if ( nParagraphs )
                            {
                                ByteString aParaEntries( "ParagraphEntryCount:" );
                                PPTParaPropSet* pSet = (PPTParaPropSet*)aStyleTextPropReader.aParaPropList.GetObject( nParagraphs - 1 );
                                aParaEntries.Append( Implgethex( pSet->pParaSet->mnEntryCount, 2 ) );
                                aFileStream.Write( aParaEntries.GetBuffer(), aParaEntries.Len() );
                                aFileStream << nLineBreak;
                            }
                            sal_uInt32 nPortionEntries = aStyleTextPropReader.aCharPropList.Count();
                            if ( nPortionEntries )
                            {
                                ByteString aCharEntries( "CharacterEntryCount:" );
                                PPTCharPropSet* pSet = (PPTCharPropSet*)aStyleTextPropReader.aCharPropList.GetObject( nPortionEntries - 1 );
                                aCharEntries.Append( Implgethex( pSet->pCharSet->mnEntryCount, 2 ) );
                                aFileStream.Write( aCharEntries.GetBuffer(), aCharEntries.Len() );
                                aFileStream << nLineBreak;
                            }
                            sal_uInt32 nCharCount = aStyleTextPropReader.aCharPropList.Count();
                            while ( rIn.Tell() < aExtParaHd.GetRecEndFilePos() )
                            {
                                ByteString aHex;

                                sal_uInt32 nBuFlags, nNumberingType, nDontKnow1, nDontKnow2;
                                sal_uInt16 nBuInstance, nBuStart;

                                rIn >> nBuFlags;
                                aHex.Append( Implgethex( nBuFlags, 4 ) );
                                if ( nBuFlags & 0x800000 )
                                {
                                    rIn >> nBuInstance;
                                    aHex.Append( Implgethex( nBuInstance, 2 ) );
                                }
                                if ( nBuFlags & 0x01000000 )
                                {
                                    rIn >> nNumberingType;
                                    aHex.Append( Implgethex( nNumberingType, 4 ) );
                                }
                                if ( nBuFlags & 0x02000000 )
                                {
                                    rIn >> nBuStart;
                                    aHex.Append( Implgethex( nBuStart, 2 ) );
                                }
                                rIn >> nDontKnow1
                                    >> nDontKnow2;

                                aHex.Append( Implgethex( nDontKnow1, 4 ) );
                                aHex.Append( Implgethex( nDontKnow2, 4 ) );
                                aFileStream.Write( aHex.GetBuffer(), aHex.Len() );
                                aFileStream << nLineBreak;
                            }
                            for ( i = 0; i < nParagraphs; i++ )
                            {
                                ByteString aBuFlag;
                                PPTParaPropSet* pSet = (PPTParaPropSet*)aStyleTextPropReader.aParaPropList.GetObject( i );
                                aBuFlag.Append( Implgethex( pSet->mnCharacters, 2 ) );
                                aBuFlag.Append( ByteString( "ParaFlags:" ) );
                                aBuFlag.Append( Implgethex( pSet->pParaSet->mnAttrSet, 4 ) );
                                aBuFlag.Append( Implgethex( pSet->pParaSet->mpArry[ PPT_ParaAttr_BulletOn ], 1 ) );
                                aFileStream.Write( aBuFlag.GetBuffer(), aBuFlag.Len() );
                                aFileStream << nLineBreak;
                            }
                            for ( i = 0; i < nPortionEntries; i++ )
                            {
                                ByteString aBuFlag;
                                PPTCharPropSet* pSet = (PPTCharPropSet*)aStyleTextPropReader.aCharPropList.GetObject( i );
                                aBuFlag.Append( Implgethex( pSet->mnCharacters, 2 ) );
                                aBuFlag.Append( ByteString( "CharFlags:" ) );
                                aBuFlag.Append( Implgethex( pSet->pCharSet->mnAttrSet, 4 ) );
                                aFileStream.Write( aBuFlag.GetBuffer(), aBuFlag.Len() );
                                aFileStream << nLineBreak;
                            }
                            aFileStream << nLineBreak;
                            aFileStream << nLineBreak;
                            rIn.Seek( nOldPos );
                        }

#endif

                        if ( nParagraphs )
                        {
                            // the language settings will be merged into the list of PPTCharPropSet
                            DffRecordHeader aTextSpecInfoHd;
                            PPTTextSpecInfoAtomInterpreter aTextSpecInfoAtomInterpreter;
                            if ( rSdrPowerPointImport.SeekToRec( rIn, PPT_PST_TextSpecInfoAtom,
                                                        aClientTextBoxHd.GetRecEndFilePos(), &aTextSpecInfoHd ) )
                            {
                                if ( aTextSpecInfoAtomInterpreter.Read( rIn, aTextSpecInfoHd, PPT_PST_TextSpecInfoAtom,
                                        &(rSdrPowerPointImport.pPPTStyleSheet->maTxSI) ) )
                                {
                                    sal_uInt32  nI = 0;
                                    PPTTextSpecInfo* pSpecInfo;
                                    for ( pSpecInfo = (PPTTextSpecInfo*)aTextSpecInfoAtomInterpreter.aList.First();
                                        pSpecInfo; pSpecInfo =(PPTTextSpecInfo*)aTextSpecInfoAtomInterpreter.aList.Next() )
                                    {
                                        sal_uInt32 nCharIdx = pSpecInfo->nCharIdx;

                                        // portions and text have to been splitted in some cases
                                        for ( ; nI < aStyleTextPropReader.aCharPropList.Count(); )
                                        {
                                            PPTCharPropSet* pSet = (PPTCharPropSet*)aStyleTextPropReader.aCharPropList.GetObject( nI );
                                            if ( pSet->mnOriginalTextPos < nCharIdx )
                                            {
                                                pSet->mnLanguage[ 0 ] = pSpecInfo->nLanguage[ 0 ];
                                                pSet->mnLanguage[ 1 ] = pSpecInfo->nLanguage[ 1 ];
                                                pSet->mnLanguage[ 2 ] = pSpecInfo->nLanguage[ 2 ];
                                                // test if the current portion needs to be splitted
                                                if ( pSet->maString.Len() > 1 )
                                                {
                                                    sal_Int32 nIndexOfNextPortion = pSet->maString.Len() + pSet->mnOriginalTextPos;
                                                    sal_Int32 nNewLen = nIndexOfNextPortion - nCharIdx;
                                                    sal_Int32 nOldLen = pSet->maString.Len() - nNewLen;

                                                    if ( ( nNewLen > 0 ) && ( nOldLen > 0 ) )
                                                    {
                                                        String aString( pSet->maString );
                                                        PPTCharPropSet* pNew = new PPTCharPropSet( *pSet );
                                                        pSet->maString = String( aString, 0, (sal_uInt16)nOldLen );
                                                        pNew->maString = String( aString, (sal_uInt16)nOldLen, (sal_uInt16)nNewLen );
                                                        pNew->mnOriginalTextPos += nOldLen;
                                                        aStyleTextPropReader.aCharPropList.Insert( pNew, nI + 1 );
                                                    }
                                                }
                                            }
                                            else
                                                break;
                                            nI++;
                                        }
                                    }
                                }
#ifdef DBG_UTIL
                                else
                                {
                                    if (!(rSdrPowerPointImport.rImportParam.nImportFlags & PPT_IMPORTFLAGS_NO_TEXT_ASSERT))
                                    {
                                        DBG_ERROR( "SdrTextSpecInfoAtomInterpreter::Ctor(): parsing error, this document needs to be analysed (SJ)" );
                                    }
                                }
#endif
                            }
                            //
                            // now will search for possible textextensions such as date/time fields
                            // or ParaTabStops and append them on this textobj
                            //
                            rIn.Seek( nFilePos );
                            List* pFieldList = NULL;
                            while ( rIn.Tell() < aClientTextBoxHd.GetRecEndFilePos() )
                            {
                                rIn >> aTextHd;
                                UINT16 nVal = 0;
                                PPTFieldEntry* pEntry = NULL;
                                switch ( aTextHd.nRecType )
                                {
                                    case PPT_PST_DateTimeMCAtom :
                                    {
                                        pEntry = new PPTFieldEntry;
                                        rIn >> pEntry->nPos
                                            >> nVal
                                            >> nVal;
                                        pEntry->SetDateTime( nVal & 0xff );
                                    }
                                    break;

                                    case PPT_PST_FooterMCAtom :
                                        nVal++;
                                    case PPT_PST_HeaderMCAtom :
                                        nVal++;
                                    case PPT_PST_GenericDateMCAtom :
                                    {
                                        if ( rPersistEntry.pHeaderFooterEntry )
                                        {
                                            pEntry = new PPTFieldEntry;
                                            rIn >> pEntry->nPos;
                                            if ( !nVal && ( rPersistEntry.pHeaderFooterEntry->nAtom & 0x20000 ) )   // auto date time
                                                pEntry->SetDateTime( rPersistEntry.pHeaderFooterEntry->nAtom & 0xff );
                                            else
                                                pEntry->pString = new String( rPersistEntry.pHeaderFooterEntry->pPlaceholder[ nVal ] );
                                        }
                                    }
                                    break;

                                    case PPT_PST_SlideNumberMCAtom :
                                    case PPT_PST_RTFDateTimeMCAtom :
                                    {
                                        pEntry = new PPTFieldEntry;
                                        if ( aTextHd.nRecLen >= 4 )
                                        {
                                            rIn >> pEntry->nPos
                                                >> nVal;

                                            // ID auswerten
                                            SvxFieldItem* pFieldItem = NULL;
                                            switch( aTextHd.nRecType )
                                            {
                                                case PPT_PST_SlideNumberMCAtom:
                                                    pEntry->pField1 = new SvxFieldItem( SvxPageField() );
                                                break;
                                                case PPT_PST_RTFDateTimeMCAtom:
                                                    pEntry->pField1 = new SvxFieldItem( SvxDateField( Date(), SVXDATETYPE_FIX ) );
                                                break;
                                            }
                                        }
                                    }
                                    break;

                                    case PPT_PST_InteractiveInfo :
                                    {
                                        DffRecordHeader aHdInteractiveInfoAtom;
                                        if ( rSdrPowerPointImport.SeekToRec( rIn, PPT_PST_InteractiveInfoAtom, aTextHd.GetRecEndFilePos(), &aHdInteractiveInfoAtom ) )
                                        {
                                            PptInteractiveInfoAtom aInteractiveInfoAtom;
                                            rIn >> aInteractiveInfoAtom;
                                            for ( SdHyperlinkEntry* pHyperlink = (SdHyperlinkEntry*)rSdrPowerPointImport.aHyperList.First();
                                                    pHyperlink; pHyperlink = (SdHyperlinkEntry*)rSdrPowerPointImport.aHyperList.Next() )
                                            {
                                                if ( pHyperlink->nIndex == aInteractiveInfoAtom.nExHyperlinkId )
                                                {
                                                    aTextHd.SeekToEndOfRecord( rIn );
                                                    rIn >> aTextHd;
                                                    if ( aTextHd.nRecType != PPT_PST_TxInteractiveInfoAtom )
                                                    {
                                                        aTextHd.SeekToBegOfRecord( rIn );
                                                        continue;
                                                    }
                                                    else
                                                    {
                                                        sal_uInt32 nStartPos, nEndPos;
                                                        rIn >> nStartPos
                                                            >> nEndPos;
                                                        if ( nEndPos )
                                                        {
                                                            pEntry = new PPTFieldEntry;
                                                            pEntry->nPos = (sal_uInt16)nStartPos;
                                                            pEntry->nTextRangeEnd = (sal_uInt16)nEndPos;
                                                            String aTarget( pHyperlink->aTarget );
                                                            if ( pHyperlink->aConvSubString.Len() )
                                                            {
                                                                aTarget.Append( (sal_Unicode)'#' );
                                                                aTarget.Append( pHyperlink->aConvSubString );
                                                            }
                                                            pEntry->pField1 = new SvxFieldItem( SvxURLField( aTarget, String(), SVXURLFORMAT_REPR ) );
                                                        }
                                                    }
                                                    break;
                                                }
                                            }
                                        }
                                    }
                                    break;
                                }
                                aTextHd.SeekToEndOfRecord( rIn );
                                if ( pEntry )
                                {
                                    if ( !pFieldList )
                                        pFieldList = new List;
                                    UINT32 n;
                                    for ( n = 0; n < pFieldList->Count(); n++ )
                                    {   // sorting fields ( hi >> lo )
                                        if ( ( (PPTFieldEntry*)pFieldList->GetObject( n ) )->nPos < pEntry->nPos )
                                            break;
                                    }
                                    pFieldList->Insert( pEntry, (UINT32)n );
                                }
                            }
                            if ( pFieldList )
                            {
                                PPTFieldEntry* pFE = (PPTFieldEntry*)pFieldList->First();
                                List& aCharPropList = aStyleTextPropReader.aCharPropList;

                                sal_Int32   i = nParagraphs - 1;
                                sal_Int32   n = aCharPropList.Count() - 1;

                                // at this point we just have a list of textportions(aCharPropList)
                                // the next while loop tries to resolve the list of fields(pFieldList)
                                while( pFE && ( n >= 0 ) && ( i >= 0 ) )
                                {
                                     PPTCharPropSet* pSet  = (PPTCharPropSet*)aCharPropList.GetObject( n );
                                    String aString( pSet->maString );
                                    UINT32 nCount = aString.Len();
                                    UINT32 nPos = pSet->mnOriginalTextPos + nCount;
                                    while ( pFE && nCount-- )
                                    {
                                        nPos--;
                                        while ( pFE && ( pFE->nPos > nPos ) )
                                            pFE = (PPTFieldEntry*)pFieldList->Next();
                                        if ( !pFE )
                                            break;

                                        if ( pFE->nPos == nPos )
                                        {
                                            if ( aString.GetChar( (sal_uInt16)nCount ) == 0x2a )
                                            {
                                                UINT32 nBehind = aString.Len() - ( nCount + 1 );
                                                pSet->maString = String();
                                                if ( nBehind )
                                                {
                                                    PPTCharPropSet* pNewCPS = new PPTCharPropSet( *pSet );
                                                    pNewCPS->maString = String( aString, (UINT16)nCount + 1, (UINT16)nBehind );
                                                    aCharPropList.Insert( pNewCPS, n + 1 );
                                                }
                                                if ( pFE->pField2 )
                                                {
                                                    PPTCharPropSet* pNewCPS = new PPTCharPropSet( *pSet );
                                                    pNewCPS->mpFieldItem = pFE->pField2, pFE->pField2 = NULL;
                                                    aCharPropList.Insert( pNewCPS, n + 1 );

                                                    pNewCPS = new PPTCharPropSet( *pSet );
                                                    pNewCPS->maString = String( String( RTL_CONSTASCII_USTRINGPARAM( " " ) ) );
                                                    aCharPropList.Insert( pNewCPS, n + 1 );
                                                }
                                                if ( nCount )
                                                {
                                                    PPTCharPropSet* pNewCPS = new PPTCharPropSet( *pSet );
                                                    pNewCPS->maString = String( aString, (UINT16)0, (UINT16)nCount );
                                                    aCharPropList.Insert( pNewCPS, n++ );
                                                }
                                                if ( pFE->pField1 )
                                                {
                                                    pSet->mpFieldItem = pFE->pField1, pFE->pField1 = NULL;
                                                }
                                                else if ( pFE->pString )
                                                    pSet->maString = *pFE->pString;
                                            }
                                            else
                                            {
                                                if ( pFE->nTextRangeEnd )   // text range hyperlink
                                                {
                                                    sal_uInt32 nHyperLen = pFE->nTextRangeEnd - nPos;
                                                    if ( nHyperLen )
                                                    {
                                                        PPTCharPropSet* pBefCPS = NULL;
                                                        if ( nCount )
                                                        {
                                                            pBefCPS = new PPTCharPropSet( *pSet );
                                                            pSet->maString = String( pSet->maString, (UINT16)nCount, (UINT16)( pSet->maString.Len() - nCount ) );
                                                        }
                                                        sal_uInt32  nIdx = n;
                                                        sal_Int32   nHyperLenLeft = nHyperLen;

                                                        while ( ( aCharPropList.Count() > nIdx ) && nHyperLenLeft )
                                                        {
                                                            // the textrange hyperlink can take more than 1 paragraph
                                                            // the solution here is to clone the hyperlink...

                                                            PPTCharPropSet* pCurrent = (PPTCharPropSet*)aCharPropList.GetObject( nIdx );
                                                            sal_Int32       nNextStringLen = pCurrent->maString.Len();

                                                            const SvxURLField* pField = (const SvxURLField*)pFE->pField1->GetField();

                                                            if ( pCurrent->mpFieldItem )
                                                            {
                                                                pCurrent->SetColor( PPT_COLSCHEME_A_UND_HYPERLINK );
                                                                if ( pCurrent->mpFieldItem->GetField()->ISA( SvxURLField ) )
                                                                    break;
                                                                nHyperLenLeft--;
                                                            }
                                                            else if ( nNextStringLen )
                                                            {
                                                                if ( nNextStringLen <= nHyperLenLeft )
                                                                {
                                                                    pCurrent->mpFieldItem = new SvxFieldItem( SvxURLField( pField->GetURL(), pCurrent->maString, SVXURLFORMAT_REPR ) );
                                                                    nHyperLenLeft -= nNextStringLen;

                                                                    if ( nHyperLenLeft )
                                                                    {
                                                                        // if the next portion is in a higher paragraph,
                                                                        // the textrange is to decrease (because of the LineBreak character)
                                                                        if ( aCharPropList.Count() > ( nIdx + 1 ) )
                                                                        {
                                                                            PPTCharPropSet* pNext = (PPTCharPropSet*)aCharPropList.GetObject( nIdx + 1 );
                                                                            if ( pNext->mnParagraph > pCurrent->mnParagraph )
                                                                                nHyperLenLeft--;
                                                                        }
                                                                    }
                                                                }
                                                                else
                                                                {
                                                                    PPTCharPropSet* pNewCPS = new PPTCharPropSet( *pCurrent );
                                                                    pNewCPS->maString = String( pCurrent->maString, (UINT16)nHyperLenLeft, (UINT16)( nNextStringLen - nHyperLenLeft ) );
                                                                    aCharPropList.Insert( pNewCPS, nIdx + 1 );
                                                                    String aRepresentation( pCurrent->maString, 0, (UINT16)nHyperLenLeft );
                                                                    pCurrent->mpFieldItem = new SvxFieldItem( SvxURLField( pField->GetURL(), aRepresentation, SVXURLFORMAT_REPR ) );
                                                                    nHyperLenLeft = 0;
                                                                }
                                                                pCurrent->maString = String();
                                                                pCurrent->SetColor( PPT_COLSCHEME_A_UND_HYPERLINK );
                                                            }
                                                            nIdx++;
                                                        }
                                                        delete pFE->pField1, pFE->pField1 = NULL;

                                                        if ( pBefCPS )
                                                        {
                                                            pBefCPS->maString = String( aString, (UINT16)0, (UINT16)nCount );
                                                            aCharPropList.Insert( pBefCPS, n++ );

                                                        }
                                                    }
                                                }
                                            }
                                            break;
                                        }
                                    }
                                    n--;
                                }
                                for ( void* pPtr = pFieldList->First(); pPtr; pPtr = pFieldList->Next() )
                                    delete (PPTFieldEntry*)pPtr;
                                delete pFieldList;
                            }
                            mpImplTextObj->mpParagraphList = new PPTParagraphObj*[ nParagraphs ];
                            aStyleTextPropReader.aCharPropList.First();
                            UINT32 nCount = 0;
                            for ( void* pPtr = aStyleTextPropReader.aParaPropList.First();
                                        pPtr;
                                        pPtr = aStyleTextPropReader.aParaPropList.Next() )
                            {
                                PPTParagraphObj* pPara = new PPTParagraphObj( aStyleTextPropReader, *rSdrPowerPointImport.pPPTStyleSheet, nInstance, aTextRulerInterpreter );
                                mpImplTextObj->mpParagraphList[ nCount++ ] = pPara;

                                sal_uInt32 nParaAdjust, nFlags = 0;
                                pPara->GetAttrib( PPT_ParaAttr_Adjust, nParaAdjust, GetInstance() );

                                switch ( nParaAdjust )
                                {
                                    case 0 : nFlags = PPT_TEXTOBJ_FLAGS_PARA_ALIGNMENT_USED_LEFT;   break;
                                    case 1 : nFlags = PPT_TEXTOBJ_FLAGS_PARA_ALIGNMENT_USED_CENTER; break;
                                    case 2 : nFlags = PPT_TEXTOBJ_FLAGS_PARA_ALIGNMENT_USED_RIGHT;  break;
                                    case 3 : nFlags = PPT_TEXTOBJ_FLAGS_PARA_ALIGNMENT_USED_BLOCK;  break;
                                }
                                mpImplTextObj->mnTextFlags |= nFlags;
                            }
                        }
                    }
                }
            }
        }
    }
}

PPTTextObj::PPTTextObj( PPTTextObj& rTextObj )
{
    mpImplTextObj = rTextObj.mpImplTextObj;
    mpImplTextObj->mnRefCount++;
}

PPTTextObj::~PPTTextObj()
{
    ImplClear();
}

PPTParagraphObj* PPTTextObj::First()
{
    mpImplTextObj->mnCurrentObject = 0;
    if ( !mpImplTextObj->mnParagraphCount )
        return NULL;
    return mpImplTextObj->mpParagraphList[ 0 ];
}

PPTParagraphObj* PPTTextObj::Next()
{
    UINT32 i = mpImplTextObj->mnCurrentObject + 1;
    if ( i >= mpImplTextObj->mnParagraphCount )
        return NULL;
    mpImplTextObj->mnCurrentObject++;
    return mpImplTextObj->mpParagraphList[ i ];
}

const SfxItemSet* PPTTextObj::GetBackground() const
{
    if ( mpImplTextObj->mrPersistEntry.pBObj )
        return &mpImplTextObj->mrPersistEntry.pBObj->GetItemSet();
    else
        return NULL;
}

void PPTTextObj::ImplClear()
{
    if ( ! ( --mpImplTextObj->mnRefCount ) )
    {
        for ( PPTParagraphObj* pPtr = First(); pPtr; pPtr = Next() )
            delete pPtr;
        delete[] mpImplTextObj->mpParagraphList;
        delete mpImplTextObj->mpPlaceHolderAtom;
        delete mpImplTextObj;
    }
}

PPTTextObj& PPTTextObj::operator=( PPTTextObj& rTextObj )
{
    if ( this != &rTextObj )
    {
        ImplClear();
        mpImplTextObj = rTextObj.mpImplTextObj;
        mpImplTextObj->mnRefCount++;
    }
    return *this;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

