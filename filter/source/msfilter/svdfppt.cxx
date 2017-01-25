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

#include <osl/endian.h>
#include <vcl/svapp.hxx>
#include <unotools/tempfile.hxx>
#include <tools/diagnose_ex.h>
#include <math.h>
#include <editeng/eeitem.hxx>
#include <editeng/editdata.hxx>
#include <sot/storage.hxx>
#include <sot/storinfo.hxx>
#include <sot/stg.hxx>
#include <com/sun/star/embed/Aspects.hpp>
#include <com/sun/star/office/XAnnotation.hpp>
#include <com/sun/star/office/XAnnotationAccess.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/geometry/RealPoint2D.hpp>
#include <com/sun/star/util/DateTime.hpp>
#include <com/sun/star/drawing/BitmapMode.hpp>
#include <unotools/streamwrap.hxx>
#include <filter/msfilter/svdfppt.hxx>
#include <svx/xpoly.hxx>
#include <svx/svdtrans.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdogrp.hxx>
#include <svx/svdorect.hxx>
#include <svx/svdopage.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdopath.hxx>
#include <svx/svdocirc.hxx>
#include <svx/svdocapt.hxx>
#include <svx/svdotable.hxx>
#include <editeng/outlobj.hxx>
#include <editeng/numdef.hxx>
#include <svx/svdattr.hxx>
#include "svx/xattr.hxx"
#include <svx/svdetc.hxx>
#include <editeng/bulletitem.hxx>
#include <editeng/hngpnctitem.hxx>
#include <editeng/forbiddenruleitem.hxx>
#include <svx/polysc3d.hxx>
#include <svx/extrud3d.hxx>
#include <svx/svdoashp.hxx>
#include <editeng/tstpitem.hxx>
#include <editeng/unoprnms.hxx>
#include <editeng/editids.hrc>

#include <editeng/adjustitem.hxx>
#include <editeng/escapementitem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/crossedoutitem.hxx>
#include <editeng/shdditem.hxx>
#include <editeng/charreliefitem.hxx>
#include <editeng/fontitem.hxx>
#include <svx/svdoutl.hxx>
#include <editeng/editeng.hxx>
#include <editeng/lspcitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/lrspitem.hxx>
#include <vcl/metric.hxx>
#include <vcl/bitmapaccess.hxx>
#include <svx/svditer.hxx>
#include <svx/svdoedge.hxx>
#include <svx/sxekitm.hxx>
#include <editeng/flditem.hxx>
#include <tools/zcodec.hxx>
#include <filter/msfilter/svxmsbas.hxx>
#include <sfx2/objsh.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/langitem.hxx>
#include <svx/svdoole2.hxx>
#include <svx/unoapi.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/drawing/XControlShape.hpp>
#include <com/sun/star/form/XFormComponent.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/drawing/XMasterPagesSupplier.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/TextVerticalAdjust.hpp>
#include <editeng/writingmodeitem.hxx>
#include <vcl/print.hxx>
#include <editeng/svxfont.hxx>
#include <editeng/frmdiritem.hxx>
#include <svx/sdtfchim.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <editeng/scripttypeitem.hxx>
#include "com/sun/star/awt/Gradient.hpp"
#include <com/sun/star/table/XMergeableCellRange.hpp>
#include <com/sun/star/table/BorderLine2.hpp>
#include <com/sun/star/table/BorderLineStyle.hpp>
#include <vcl/virdev.hxx>
#include <svtools/embedhlp.hxx>
#include <o3tl/enumrange.hxx>
#include <o3tl/make_unique.hxx>
#include <boost/optional.hpp>

#include <algorithm>
#include <cassert>
#include <set>
#include <utility>
#include <rtl/strbuf.hxx>
#include <tools/time.hxx>
#include <memory>

// PPT ColorScheme Slots
#define PPT_COLSCHEME                       (0x08000000)
#define PPT_COLSCHEME_HINTERGRUND           (0x08000000)
#define PPT_COLSCHEME_TEXT_UND_ZEILEN       (0x08000001)
#define PPT_COLSCHEME_TITELTEXT             (0x08000003)
#define PPT_COLSCHEME_A_UND_HYPERLINK       (0x08000006)

#define ANSI_CHARSET            0
#define SYMBOL_CHARSET          2

/* Font Families */
#define FF_ROMAN                0x10
#define FF_SWISS                0x20
#define FF_MODERN               0x30
#define FF_SCRIPT               0x40
#define FF_DECORATIVE           0x50

#define DEFAULT_PITCH           0x00
#define FIXED_PITCH             0x01
#define VARIABLE_PITCH          0x02

using namespace ::com::sun::star    ;
using namespace uno                 ;
using namespace beans               ;
using namespace drawing             ;
using namespace container           ;
using namespace table               ;

PowerPointImportParam::PowerPointImportParam( SvStream& rDocStrm, sal_uInt32 nFlags ) :
    rDocStream      ( rDocStrm ),
    nImportFlags    ( nFlags )
{
}

SvStream& ReadPptCurrentUserAtom( SvStream& rIn, PptCurrentUserAtom& rAtom )
{
    DffRecordHeader aHd;
    ReadDffRecordHeader( rIn, aHd );
    if ( aHd.nRecType == PPT_PST_CurrentUserAtom )
    {
        sal_uInt32 nLen;
        sal_uInt16 nUserNameLen, nPad;
        rIn.ReadUInt32( nLen )
           .ReadUInt32( rAtom.nMagic )
           .ReadUInt32( rAtom.nCurrentUserEdit )
           .ReadUInt16( nUserNameLen )
           .ReadUInt16( rAtom.nDocFileVersion )
           .ReadUChar( rAtom.nMajorVersion )
           .ReadUChar( rAtom.nMinorVersion )
           .ReadUInt16( nPad );
        rAtom.aCurrentUser = SvxMSDffManager::MSDFFReadZString( rIn, nUserNameLen, true );
    }
    aHd.SeekToEndOfRecord( rIn );
    return rIn;
}

void PptSlidePersistAtom::Clear()
{
    nReserved = nPsrReference = nFlags = nNumberTexts = nSlideId = 0;
}

SvStream& ReadPptSlidePersistAtom( SvStream& rIn, PptSlidePersistAtom& rAtom )
{
    DffRecordHeader aHd;
    ReadDffRecordHeader( rIn, aHd );
    rIn
       .ReadUInt32( rAtom.nPsrReference )
       .ReadUInt32( rAtom.nFlags )
       .ReadUInt32( rAtom.nNumberTexts )
       .ReadUInt32( rAtom.nSlideId );
    aHd.SeekToEndOfRecord( rIn );
    return rIn;
}

PptSlidePersistList::PptSlidePersistList() {}

PptSlidePersistList::~PptSlidePersistList() {}

sal_uInt16 PptSlidePersistList::FindPage(sal_uInt32 nId) const
{
    for ( size_t i=0; i < mvEntries.size(); i++ )
    {
        if (mvEntries[ i ]->GetSlideId() == nId) return i;
    }
    return PPTSLIDEPERSIST_ENTRY_NOTFOUND;
}

SvStream& ReadPptInteractiveInfoAtom( SvStream& rIn, PptInteractiveInfoAtom& rAtom )
{
    rIn.ReadUInt32( rAtom.nSoundRef )
       .ReadUInt32( rAtom.nExHyperlinkId )
       .ReadUChar( rAtom.nAction )
       .ReadUChar( rAtom.nOleVerb )
       .ReadUChar( rAtom.nJump )
       .ReadUChar( rAtom.nFlags )
       .ReadUChar( rAtom.nHyperlinkType )
       .ReadUChar( rAtom.nUnknown1 )
       .ReadUChar( rAtom.nUnknown2 )
       .ReadUChar( rAtom.nUnknown3 );
    return rIn;
}

SvStream& ReadPptExOleObjAtom( SvStream& rIn, PptExOleObjAtom& rAtom )
{
    sal_uInt32 nDummy1;
    sal_uInt32 nDummy2;
    sal_uInt32 nDummy4;

    rIn.ReadUInt32( rAtom.nAspect )
       .ReadUInt32( nDummy1 )
       .ReadUInt32( rAtom.nId )
       .ReadUInt32( nDummy2 )
       .ReadUInt32( rAtom.nPersistPtr )
       .ReadUInt32( nDummy4 );
    return rIn;
}

const Size& PptDocumentAtom::GetPageSize(const Size& rSiz)
{
    return rSiz;
}

SvStream& ReadPptDocumentAtom(SvStream& rIn, PptDocumentAtom& rAtom)
{
// Actual format:
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
    sal_Int32   nSlideX,nSlideY, nNoticeX, nNoticeY, nDummy;
    sal_uInt16  nSlidePageFormat;
    sal_Int8    nEmbeddedTrueType, nTitlePlaceHoldersOmitted, nRightToLeft, nShowComments;

    ReadDffRecordHeader( rIn, aHd );
    rIn
       .ReadInt32( nSlideX ).ReadInt32( nSlideY )
       .ReadInt32( nNoticeX ).ReadInt32( nNoticeY )
       .ReadInt32( nDummy ).ReadInt32( nDummy )             // skip ZoomRation
       .ReadUInt32( rAtom.nNotesMasterPersist )
       .ReadUInt32( rAtom.nHandoutMasterPersist )
       .ReadUInt16( rAtom.n1stPageNumber )
       .ReadUInt16( nSlidePageFormat )
       .ReadSChar( nEmbeddedTrueType )
       .ReadSChar( nTitlePlaceHoldersOmitted )
       .ReadSChar( nRightToLeft )
       .ReadSChar( nShowComments );
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

void PptSlideLayoutAtom::Clear()
{
    eLayout = PptSlideLayout::TITLESLIDE;
    for (PptPlaceholder & i : aPlaceholderId)
        i = PptPlaceholder::NONE;
}

SvStream& ReadPptSlideLayoutAtom( SvStream& rIn, PptSlideLayoutAtom& rAtom )
{
    sal_Int32 nTmp;
    rIn.ReadInt32(nTmp);
    rAtom.eLayout = static_cast<PptSlideLayout>(nTmp);
    static_assert(sizeof(rAtom.aPlaceholderId) == 8, "wrong size of serialized array");
    rIn.ReadBytes(rAtom.aPlaceholderId, 8);
    return rIn;
}

SvStream& ReadPptSlideAtom( SvStream& rIn, PptSlideAtom& rAtom )
{
    DffRecordHeader aHd;
    ReadDffRecordHeader( rIn, aHd );
    ReadPptSlideLayoutAtom( rIn, rAtom.aLayout );
    rIn.ReadUInt32( rAtom.nMasterId )
       .ReadUInt32( rAtom.nNotesId )
       .ReadUInt16( rAtom.nFlags );
    aHd.SeekToEndOfRecord( rIn );
    return rIn;
}

void PptSlideAtom::Clear()
{
    nMasterId = nNotesId = 0;
    nFlags = 0;
}

SvStream& ReadPptNotesAtom( SvStream& rIn, PptNotesAtom& rAtom )
{
    DffRecordHeader aHd;
    ReadDffRecordHeader( rIn, aHd );
    rIn
       .ReadUInt32( rAtom.nSlideId )
       .ReadUInt16( rAtom.nFlags );
    aHd.SeekToEndOfRecord( rIn );
    return rIn;
}

void PptNotesAtom::Clear()
{
    nSlideId = 0;
    nFlags = 0;
}

PptColorSchemeAtom::PptColorSchemeAtom()
{
    memset(&aData[0], 0, 32);
}

Color PptColorSchemeAtom::GetColor( sal_uInt16 nNum ) const
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

SvStream& ReadPptColorSchemeAtom( SvStream& rIn, PptColorSchemeAtom& rAtom )
{
    DffRecordHeader aHd;
    ReadDffRecordHeader( rIn, aHd );
    rIn.ReadBytes(rAtom.aData, 32);
    aHd.SeekToEndOfRecord( rIn );
    return rIn;
}

SvStream& ReadPptFontEntityAtom( SvStream& rIn, PptFontEntityAtom& rAtom )
{
    DffRecordHeader aHd;
    ReadDffRecordHeader( rIn, aHd );
    sal_Unicode nTemp, cData[ 32 ];
    rIn.ReadBytes(cData, 64);

    sal_uInt8   lfCharset, lfPitchAndFamily;

    rIn.ReadUChar( lfCharset )
       .ReadUChar( rAtom.lfClipPrecision )
       .ReadUChar( rAtom.lfQuality )
       .ReadUChar( lfPitchAndFamily );

    switch( lfCharset )
    {
        case SYMBOL_CHARSET :
            rAtom.eCharSet = RTL_TEXTENCODING_SYMBOL;
        break;
        case ANSI_CHARSET :
            rAtom.eCharSet = RTL_TEXTENCODING_MS_1252;
        break;

        default :
            rAtom.eCharSet = osl_getThreadTextEncoding();
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
#ifdef OSL_BIGENDIAN
        cData[ i ] = ( nTemp >> 8 ) | ( nTemp << 8 );
#endif
    }
    rAtom.aName = OUString(cData, i);
    OutputDevice* pDev = Application::GetDefaultDevice();
    rAtom.bAvailable = pDev->IsFontAvailable( rAtom.aName );
    aHd.SeekToEndOfRecord( rIn );
    return rIn;
}

class PptFontCollection : public std::vector<std::unique_ptr<PptFontEntityAtom>>
{
};

SvStream& ReadPptUserEditAtom( SvStream& rIn, PptUserEditAtom& rAtom )
{
    ReadDffRecordHeader( rIn, rAtom.aHd );
    rIn
       .ReadInt32( rAtom.nLastSlideID )
       .ReadUInt32( rAtom.nVersion )
       .ReadUInt32( rAtom.nOffsetLastEdit )
       .ReadUInt32( rAtom.nOffsetPersistDirectory )
       .ReadUInt32( rAtom.nDocumentRef )
       .ReadUInt32( rAtom.nMaxPersistWritten )
       .ReadInt16( rAtom.eLastViewType );
    rAtom.aHd.SeekToEndOfRecord(rIn);
    return rIn;
}

void PptOEPlaceholderAtom::Clear()
{
    nPlacementId = 0;
    nPlaceholderSize = 0;
    nPlaceholderId = PptPlaceholder::NONE;
}

SvStream& ReadPptOEPlaceholderAtom( SvStream& rIn, PptOEPlaceholderAtom& rAtom )
{
    rIn.ReadUInt32( rAtom.nPlacementId );
    sal_uInt8 nTmp;
    rIn.ReadUChar(nTmp);
    rAtom.nPlaceholderId = static_cast<PptPlaceholder>(nTmp);
    rIn.ReadUChar( rAtom.nPlaceholderSize );
    return rIn;
}

PptSlidePersistEntry::PptSlidePersistEntry() :
    pStyleSheet             ( nullptr ),
    pHeaderFooterEntry      ( nullptr ),
    pSolverContainer        ( nullptr ),
    nSlidePersistStartOffset( 0 ),
    nSlidePersistEndOffset  ( 0 ),
    nBackgroundOffset       ( 0 ),
    nDrawingDgId            ( 0xffffffff ),
    pPresentationObjects    ( nullptr ),
    pBObj                   ( nullptr ),
    bBObjIsTemporary        ( true ),
    ePageKind               ( PPT_MASTERPAGE ),
    bNotesMaster            ( false ),
    bHandoutMaster          ( false ),
    bStarDrawFiller         ( false )
{
    HeaderFooterOfs[ 0 ] =  HeaderFooterOfs[ 1 ] = HeaderFooterOfs[ 2 ] = HeaderFooterOfs[ 3 ] = 0;
}


PptSlidePersistEntry::~PptSlidePersistEntry()
{
    delete pStyleSheet;
    delete pHeaderFooterEntry;
    delete pSolverContainer;
    delete[] pPresentationObjects;
};

SdrEscherImport::SdrEscherImport( PowerPointImportParam& rParam, const OUString& rBaseURL ) :
    SvxMSDffManager         ( rParam.rDocStream, rBaseURL ),
    m_pFonts                ( nullptr ),
    nStreamLen              ( 0 ),
    rImportParam            ( rParam )
{
}

SdrEscherImport::~SdrEscherImport()
{
    for (PPTOleEntry* i : aOleObjectList)
        delete i;
    aOleObjectList.clear();
    delete m_pFonts;
}

const PptSlideLayoutAtom* SdrEscherImport::GetSlideLayoutAtom() const
{
    return nullptr;
}

bool SdrEscherImport::ReadString( OUString& rStr ) const
{
    bool bRet = false;
    DffRecordHeader aStrHd;
    ReadDffRecordHeader( rStCtrl, aStrHd );
    if (aStrHd.nRecType == PPT_PST_TextBytesAtom
        || aStrHd.nRecType == PPT_PST_TextCharsAtom
        || aStrHd.nRecType == PPT_PST_CString)
    {
        bool bUniCode =
            (aStrHd.nRecType == PPT_PST_TextCharsAtom
            || aStrHd.nRecType == PPT_PST_CString);
        bRet = true;
        sal_uLong nBytes = aStrHd.nRecLen;
        rStr = MSDFFReadZString( rStCtrl, nBytes, bUniCode );
        aStrHd.SeekToEndOfRecord( rStCtrl );
    }
    else
        aStrHd.SeekToBegOfRecord( rStCtrl );
    return bRet;
}

bool SdrEscherImport::GetColorFromPalette(sal_uInt16 /*nNum*/, Color& /*rColor*/) const
{
    return false;
}

bool SdrEscherImport::SeekToShape( SvStream& /*rSt*/, void* /*pClientData*/, sal_uInt32 /*nId*/) const
{
    return false;
}

PptFontEntityAtom* SdrEscherImport::GetFontEnityAtom( sal_uInt32 nNum ) const
{
    PptFontEntityAtom* pRetValue = nullptr;
    if (m_pFonts && (nNum < m_pFonts->size()))
        pRetValue = (*m_pFonts)[ nNum ].get();
    return pRetValue;
}

SdrObject* SdrEscherImport::ReadObjText( PPTTextObj* /*pTextObj*/, SdrObject* pObj, SdPageCapsule /*pPage*/) const
{
    return pObj;
}

void SdrEscherImport::ProcessClientAnchor2( SvStream& rSt, DffRecordHeader& rHd, void* /*pData*/, DffObjData& rObj )
{
    sal_Int32 l, t, r, b;
    if ( rHd.nRecLen == 16 )
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
    rObj.aChildAnchor = Rectangle( l, t, r, b );
    rObj.bChildAnchor = true;
    return;
};

void SdrEscherImport::RecolorGraphic( SvStream& rSt, sal_uInt32 nRecLen, Graphic& rGraphic )
{
    if ( rGraphic.GetType() == GraphicType::GdiMetafile )
    {
        sal_uInt16 nX, nGlobalColorsCount, nFillColorsCount;

        rSt.ReadUInt16( nX )
           .ReadUInt16( nGlobalColorsCount )
           .ReadUInt16( nFillColorsCount )
           .ReadUInt16( nX )
           .ReadUInt16( nX )
           .ReadUInt16( nX );

        if ( ( nGlobalColorsCount <= 64 ) && ( nFillColorsCount <= 64 ) )
        {
            if ( (sal_uInt32)( ( nGlobalColorsCount + nFillColorsCount ) * 44 + 12 ) == nRecLen )
            {
                sal_uInt32 OriginalGlobalColors[ 64 ];
                sal_uInt32 NewGlobalColors[ 64 ];

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
                        rSt.ReadUInt16( nChanged );
                        if ( nChanged & 1 )
                        {
                            sal_uInt32 nColor = 0;
                            rSt.ReadUChar( nDummy )
                               .ReadUChar( nRed )
                               .ReadUChar( nDummy )
                               .ReadUChar( nGreen )
                               .ReadUChar( nDummy )
                               .ReadUChar( nBlue )
                               .ReadUInt32( nIndex );

                            if ( nIndex < 8 )
                            {
                                Color aColor = MSO_CLR_ToColor( nIndex << 24 );
                                nRed = aColor.GetRed();
                                nGreen = aColor.GetGreen();
                                nBlue = aColor.GetBlue();
                            }
                            nColor = nRed | ( nGreen << 8 ) | ( nBlue << 16 );
                            *pCurrentNew++ = nColor;
                            rSt.ReadUChar( nDummy )
                               .ReadUChar( nRed )
                               .ReadUChar( nDummy )
                               .ReadUChar( nGreen )
                               .ReadUChar( nDummy )
                               .ReadUChar( nBlue );
                            nColor = nRed | ( nGreen << 8 ) | ( nBlue << 16 );
                            *pCurrentOriginal++ = nColor;
                            (*pCount)++;
                        }
                        rSt.Seek( nPos + 44 );
                    }
                    pCount = &nFillColorsChanged;
                    i = nFillColorsCount;
                }
                if ( nGlobalColorsChanged || nFillColorsChanged )
                {
                    std::unique_ptr<Color[]> pSearchColors(new Color[ nGlobalColorsChanged ]);
                    std::unique_ptr<Color[]> pReplaceColors(new Color[ nGlobalColorsChanged ]);

                    for ( j = 0; j < nGlobalColorsChanged; j++ )
                    {
                        sal_uInt32 nSearch = OriginalGlobalColors[ j ];
                        sal_uInt32 nReplace = NewGlobalColors[ j ];

                        pSearchColors[ j ].SetRed( (sal_uInt8)nSearch );
                        pSearchColors[ j ].SetGreen( (sal_uInt8)( nSearch >> 8 ) );
                        pSearchColors[ j ].SetBlue( (sal_uInt8)( nSearch >> 16 ) );

                        pReplaceColors[ j ].SetRed( (sal_uInt8)nReplace );
                        pReplaceColors[ j ].SetGreen( (sal_uInt8)( nReplace >> 8 ) );
                        pReplaceColors[ j ].SetBlue( (sal_uInt8)( nReplace >> 16 ) );
                    }
                    GDIMetaFile aGdiMetaFile( rGraphic.GetGDIMetaFile() );
                    aGdiMetaFile.ReplaceColors( pSearchColors.get(), pReplaceColors.get(),
                        nGlobalColorsChanged );
                    rGraphic = aGdiMetaFile;
                }
            }
        }
    }
}

sal_uLong DffPropSet::SanitizeEndPos(SvStream &rIn, sal_uLong nEndRecPos)
{
    auto nStreamLen = rIn.Tell() + rIn.remainingSize();
    if (nEndRecPos > nStreamLen)
    {
        SAL_WARN("filter.ms", "Parsing error: " << nStreamLen <<
                 " max end pos, but " << nEndRecPos << " claimed, truncating");
        nEndRecPos = nStreamLen;
    }
    return nEndRecPos;
}

/* ProcessObject is called from ImplSdPPTImport::ProcessObj to handle all application specific things,
   such as the import of text, animation effects, header footer and placeholder.

   The parameter pOriginalObj is the object as it was imported by our general escher import, it must either
   be deleted or it can be returned to be inserted into the sdr page.
*/
SdrObject* SdrEscherImport::ProcessObj( SvStream& rSt, DffObjData& rObjData, void* pData, Rectangle& rTextRect, SdrObject* pOriginalObj )
{
    if ( dynamic_cast<const SdrObjCustomShape* >(pOriginalObj) !=  nullptr )
        pOriginalObj->SetMergedItem( SdrTextFixedCellHeightItem( true ) );

    // we are initializing our return value with the object that was imported by our escher import
    SdrObject* pRet = pOriginalObj;

    ProcessData& rData = *static_cast<ProcessData*>(pData);
    PptSlidePersistEntry& rPersistEntry = rData.rPersistEntry;

    if ( ! ( rObjData.nSpFlags & SP_FGROUP  ) )     // sj: #114758# ...
    {
        PptOEPlaceholderAtom aPlaceholderAtom;

        if ( maShapeRecords.SeekToContent( rSt, DFF_msofbtClientData, SEEK_FROM_CURRENT_AND_RESTART ) )
        {
            sal_Int16 nHeaderFooterInstance = -1;
            DffRecordHeader aClientDataHd;
            auto nEndRecPos = SanitizeEndPos(rSt, maShapeRecords.Current()->GetRecEndFilePos());
            while ( ( rSt.GetError() == 0 ) && ( rSt.Tell() < nEndRecPos ) )
            {
                ReadDffRecordHeader( rSt, aClientDataHd );
                switch ( aClientDataHd.nRecType )
                {
                    // importing header/footer object from master page
                    case PPT_PST_OEPlaceholderAtom :
                    {
                        ReadPptOEPlaceholderAtom( rSt, aPlaceholderAtom );
                        if ( nHeaderFooterInstance == -1 )
                        {
                            switch ( aPlaceholderAtom.nPlaceholderId )
                            {
                                case PptPlaceholder::MASTERSLIDENUMBER :    nHeaderFooterInstance++;
                                    SAL_FALLTHROUGH;
                                case PptPlaceholder::MASTERFOOTER :         nHeaderFooterInstance++;
                                    SAL_FALLTHROUGH;
                                case PptPlaceholder::MASTERHEADER :         nHeaderFooterInstance++;
                                    SAL_FALLTHROUGH;
                                case PptPlaceholder::MASTERDATE :           nHeaderFooterInstance++; break;
                                default: break;

                            }
                            if ( ! ( nHeaderFooterInstance & 0xfffc ) )     // is this a valid instance ( 0->3 )
                                rPersistEntry.HeaderFooterOfs[ nHeaderFooterInstance ] = rObjData.rSpHd.GetRecBegFilePos();
                        }
                    }
                    break;

                    case PPT_PST_RecolorInfoAtom :
                    {
                        if ( dynamic_cast<const SdrGrafObj* >(pRet) != nullptr && static_cast<SdrGrafObj*>(pRet)->HasGDIMetaFile() )
                        {
                            Graphic aGraphic( static_cast<SdrGrafObj*>(pRet)->GetGraphic() );
                            RecolorGraphic( rSt, aClientDataHd.nRecLen, aGraphic );
                            static_cast<SdrGrafObj*>(pRet)->SetGraphic( aGraphic );
                        }
                    }
                    break;
                }
                if (!aClientDataHd.SeekToEndOfRecord(rSt))
                    break;
            }
        }
        if ( ( aPlaceholderAtom.nPlaceholderId == PptPlaceholder::NOTESSLIDEIMAGE ) && !rPersistEntry.bNotesMaster )
        {
            sal_uInt16 nPageNum = pSdrModel->GetPageCount();
            if ( nPageNum > 0 )
                nPageNum--;

            // replacing the object which we will return with a SdrPageObj
            SdrObject::Free( pRet );
            pRet = new SdrPageObj( rObjData.aBoundRect, pSdrModel->GetPage( nPageNum - 1 ) );
        }
        else
        {
            // try to load some ppt text
            PPTTextObj aTextObj( rSt, static_cast<SdrPowerPointImport&>(*this), rPersistEntry, &rObjData );
            if ( ( aTextObj.Count() || aTextObj.GetOEPlaceHolderAtom() ) )
            {
                bool bVerticalText = false;
                // and if the text object is not empty, it must be applied to pRet, the object we
                // initially got from our escher import
                sal_Int32 nTextRotationAngle = 0;
                if ( IsProperty( DFF_Prop_txflTextFlow ) )
                {
                    MSO_TextFlow eTextFlow = (MSO_TextFlow)( GetPropertyValue( DFF_Prop_txflTextFlow, 0 ) & 0xFFFF );
                    switch( eTextFlow )
                    {
                        case mso_txflBtoT :                     // Bottom to Top non-@, unten -> oben
                            nTextRotationAngle += 9000;
                        break;
                        case mso_txflTtoBA :    /* #68110# */   // Top to Bottom @-font, oben -> unten
                        case mso_txflTtoBN :                    // Top to Bottom non-@, oben -> unten
                        case mso_txflVertN :                    // Vertical, non-@, oben -> unten
                            bVerticalText = !bVerticalText;     // nTextRotationAngle += 27000;
                        break;
    //                  case mso_txflHorzN :                    // Horizontal non-@, normal
    //                  case mso_txflHorzA :                    // Horizontal @-font, normal
                        default: break;
                    }
                }
                sal_Int32 nFontDirection = GetPropertyValue( DFF_Prop_cdirFont, mso_cdir0 );
                nTextRotationAngle -= nFontDirection * 9000;
                if ( ( nFontDirection == 1 ) || ( nFontDirection == 3 ) )
                {
                    bVerticalText = !bVerticalText;
                }
                aTextObj.SetVertical( bVerticalText );
                if ( pRet )
                {
                    bool bDeleteSource = aTextObj.GetOEPlaceHolderAtom() != nullptr;
                    if ( bDeleteSource  && dynamic_cast<const SdrGrafObj* >(pRet) ==  nullptr     // we are not allowed to get
                            && dynamic_cast<const SdrObjGroup* >(pRet) ==  nullptr                // grouped placeholder objects
                                && dynamic_cast<const SdrOle2Obj* >(pRet) ==  nullptr )
                        SdrObject::Free( pRet );
                }
                sal_uInt32 nTextFlags = aTextObj.GetTextFlags();
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
                bool    bAutoGrowWidth, bAutoGrowHeight;

                SdrTextVertAdjust eTVA;
                SdrTextHorzAdjust eTHA;

                nTextFlags &= PPT_TEXTOBJ_FLAGS_PARA_ALIGNMENT_USED_LEFT   | PPT_TEXTOBJ_FLAGS_PARA_ALIGNMENT_USED_RIGHT
                            | PPT_TEXTOBJ_FLAGS_PARA_ALIGNMENT_USED_CENTER | PPT_TEXTOBJ_FLAGS_PARA_ALIGNMENT_USED_BLOCK;

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
                    switch ( eTextAnchor )
                    {
                        case mso_anchorTopCentered :
                        case mso_anchorMiddleCentered :
                        case mso_anchorBottomCentered :
                        case mso_anchorTopCenteredBaseline:
                        case mso_anchorBottomCenteredBaseline:
                        {
                            // check if it is sensible to use the centered alignment
                            const sal_uInt32 nMask = PPT_TEXTOBJ_FLAGS_PARA_ALIGNMENT_USED_LEFT | PPT_TEXTOBJ_FLAGS_PARA_ALIGNMENT_USED_CENTER | PPT_TEXTOBJ_FLAGS_PARA_ALIGNMENT_USED_RIGHT | PPT_TEXTOBJ_FLAGS_PARA_ALIGNMENT_USED_BLOCK;
                            switch (nTextFlags & nMask)
                            {
                            case PPT_TEXTOBJ_FLAGS_PARA_ALIGNMENT_USED_LEFT:
                            case PPT_TEXTOBJ_FLAGS_PARA_ALIGNMENT_USED_CENTER:
                            case PPT_TEXTOBJ_FLAGS_PARA_ALIGNMENT_USED_RIGHT:
                            case PPT_TEXTOBJ_FLAGS_PARA_ALIGNMENT_USED_BLOCK:
                                eTVA = SDRTEXTVERTADJUST_CENTER;    // If the textobject has only one type of alignment, then the text has not to be displayed using the full width;
                                break;
                            }
                            break;
                        }
                        default:
                            break;
                    }
                    nMinFrameWidth = rTextRect.GetWidth() - ( nTextLeft + nTextRight );
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
                    switch ( eTextAnchor )
                    {
                        case mso_anchorTopCentered :
                        case mso_anchorMiddleCentered :
                        case mso_anchorBottomCentered :
                        case mso_anchorTopCenteredBaseline:
                        case mso_anchorBottomCenteredBaseline:
                        {
                            // check if it is sensible to use the centered alignment
                            const sal_uInt32 nMask = PPT_TEXTOBJ_FLAGS_PARA_ALIGNMENT_USED_LEFT | PPT_TEXTOBJ_FLAGS_PARA_ALIGNMENT_USED_CENTER | PPT_TEXTOBJ_FLAGS_PARA_ALIGNMENT_USED_RIGHT | PPT_TEXTOBJ_FLAGS_PARA_ALIGNMENT_USED_BLOCK;
                            switch (nTextFlags & nMask)
                            {
                            case PPT_TEXTOBJ_FLAGS_PARA_ALIGNMENT_USED_LEFT:
                            case PPT_TEXTOBJ_FLAGS_PARA_ALIGNMENT_USED_CENTER:
                            case PPT_TEXTOBJ_FLAGS_PARA_ALIGNMENT_USED_RIGHT:
                            case PPT_TEXTOBJ_FLAGS_PARA_ALIGNMENT_USED_BLOCK:
                                eTHA = SDRTEXTHORZADJUST_CENTER;    // If the textobject has only one type of alignment, then the text has not to be displayed using the full width;
                                break;
                            }
                            break;
                        }
                        default:
                            break;
                    }
                    nMinFrameHeight = rTextRect.GetHeight() - ( nTextTop + nTextBottom );
                }

                SdrObjKind eTextKind = OBJ_RECT;
                if ( ( aPlaceholderAtom.nPlaceholderId == PptPlaceholder::NOTESSLIDEIMAGE )
                    || ( aPlaceholderAtom.nPlaceholderId == PptPlaceholder::MASTERNOTESSLIDEIMAGE ) )
                {
                    aTextObj.SetInstance( TSS_Type::Notes );
                    eTextKind = OBJ_TITLETEXT;
                }
                else if ( ( aPlaceholderAtom.nPlaceholderId == PptPlaceholder::MASTERNOTESBODYIMAGE )
                    || ( aPlaceholderAtom.nPlaceholderId == PptPlaceholder::NOTESBODY ) )
                {
                    aTextObj.SetInstance( TSS_Type::Notes );
                    eTextKind = OBJ_TEXT;
                }

                TSS_Type nDestinationInstance = aTextObj.GetInstance();
                if ( rPersistEntry.ePageKind == PPT_MASTERPAGE )
                {
                    if ( !rPersistEntry.pPresentationObjects )
                    {
                        rPersistEntry.pPresentationObjects = new sal_uInt32[ PPT_STYLESHEETENTRYS ];
                        memset( rPersistEntry.pPresentationObjects, 0, PPT_STYLESHEETENTRYS * 4 );
                    }
                    if ( !rPersistEntry.pPresentationObjects[ (int)nDestinationInstance ] )
                        rPersistEntry.pPresentationObjects[ (int)nDestinationInstance ] = rObjData.rSpHd.GetRecBegFilePos();
                }
                switch ( nDestinationInstance )
                {
                    case TSS_Type::PageTitle :
                    case TSS_Type::Title :
                    {
                        if ( GetSlideLayoutAtom()->eLayout == PptSlideLayout::TITLEMASTERSLIDE )
                            nDestinationInstance = TSS_Type::Title;
                        else
                            nDestinationInstance = TSS_Type::PageTitle;
                    }
                    break;
                    case TSS_Type::Body :
                    case TSS_Type::HalfBody :
                    case TSS_Type::QuarterBody :
                        nDestinationInstance = TSS_Type::Body;
                    break;
                    default: break;
                }
                aTextObj.SetDestinationInstance( nDestinationInstance );

                bool bAutoFit = false; // auto-scale text into shape box
                switch ( aTextObj.GetInstance() )
                {
                    case TSS_Type::PageTitle :
                    case TSS_Type::Title : eTextKind = OBJ_TITLETEXT; break;
                    case TSS_Type::Subtitle : eTextKind = OBJ_TEXT; break;
                    case TSS_Type::Body :
                    case TSS_Type::HalfBody :
                    case TSS_Type::QuarterBody : eTextKind = OBJ_OUTLINETEXT; bAutoFit = true; break;
                    default: break;
                }
                if ( aTextObj.GetDestinationInstance() != TSS_Type::TextInShape )
                {
                    if ( !aTextObj.GetOEPlaceHolderAtom() || aTextObj.GetOEPlaceHolderAtom()->nPlaceholderId == PptPlaceholder::NONE )
                    {
                        aTextObj.SetDestinationInstance( TSS_Type::TextInShape );
                        eTextKind = OBJ_RECT;
                    }
                }
                SdrObject* pTObj = nullptr;
                bool bWordWrap = (MSO_WrapMode)GetPropertyValue( DFF_Prop_WrapText, mso_wrapSquare ) != mso_wrapNone;
                bool bFitShapeToText = ( GetPropertyValue( DFF_Prop_FitTextToShape, 0 ) & 2 ) != 0;

                if ( dynamic_cast<const SdrObjCustomShape* >(pRet) !=  nullptr && ( eTextKind == OBJ_RECT ) )
                {
                    bAutoGrowHeight = bFitShapeToText;
                    if ( bWordWrap )
                        bAutoGrowWidth = false;
                    else
                        bAutoGrowWidth = true;
                    pTObj = pRet;
                    pRet = nullptr;
                }
                else
                {
                    if ( dynamic_cast<const SdrObjCustomShape* >(pRet) !=  nullptr )
                    {
                        SdrObject::Free( pRet );
                        pRet = nullptr;
                    }
                    pTObj = new SdrRectObj( eTextKind != OBJ_RECT ? eTextKind : OBJ_TEXT );
                    pTObj->SetModel( pSdrModel );
                    SfxItemSet aSet( pSdrModel->GetItemPool() );
                    if ( !pRet )
                        ApplyAttributes( rSt, aSet, rObjData );
                    pTObj->SetMergedItemSet( aSet );
                    if ( pRet )
                    {
                        pTObj->SetMergedItem( XLineStyleItem( drawing::LineStyle_NONE ) );
                        pTObj->SetMergedItem( XFillStyleItem( drawing::FillStyle_NONE ) );
                    }
                    if ( bVerticalText )
                    {
                        bAutoGrowWidth = bFitShapeToText;
                        bAutoGrowHeight = false;
                    }
                    else
                    {
                        bAutoGrowWidth = false;

                        // #119885# re-activating bFitShapeToText here, could not find deeper explanations
                        // for it (it was from 2005). Keeping the old comment here for reference
                        // old comment: // bFitShapeToText; can't be used, because we cut the text if it is too height,
                        bAutoGrowHeight = bFitShapeToText;
                    }
                }
                pTObj->SetMergedItem( SvxFrameDirectionItem( bVerticalText ? FRMDIR_VERT_TOP_RIGHT : FRMDIR_HORI_LEFT_TOP, EE_PARA_WRITINGDIR ) );

                //Autofit text only if there is no auto grow height and width
                //See fdo#41245
                if (bAutoFit && !bAutoGrowHeight && !bAutoGrowWidth)
                {
                    pTObj->SetMergedItem( SdrTextFitToSizeTypeItem(SdrFitToSizeType::Autofit) );
                }

            if ( dynamic_cast<const SdrObjCustomShape* >(pTObj) ==  nullptr )
            {
                 pTObj->SetMergedItem( makeSdrTextAutoGrowWidthItem( bAutoGrowWidth ) );
                pTObj->SetMergedItem( makeSdrTextAutoGrowHeightItem( bAutoGrowHeight ) );
            }
            else
            {
                pTObj->SetMergedItem( makeSdrTextWordWrapItem( bWordWrap ) );
                pTObj->SetMergedItem( makeSdrTextAutoGrowHeightItem( bFitShapeToText ) );
            }

            pTObj->SetMergedItem( SdrTextVertAdjustItem( eTVA ) );
            pTObj->SetMergedItem( SdrTextHorzAdjustItem( eTHA ) );

            if ( nMinFrameHeight < 0 )
                nMinFrameHeight = 0;
            if ( dynamic_cast<const SdrObjCustomShape* >(pTObj) ==  nullptr )
                pTObj->SetMergedItem( makeSdrTextMinFrameHeightItem( nMinFrameHeight ) );

            if ( nMinFrameWidth < 0 )
                nMinFrameWidth = 0;
            if ( dynamic_cast<const SdrObjCustomShape* >(pTObj) ==  nullptr )
                pTObj->SetMergedItem( makeSdrTextMinFrameWidthItem( nMinFrameWidth ) );

            // set margins at the borders of the textbox
            pTObj->SetMergedItem( makeSdrTextLeftDistItem( nTextLeft ) );
            pTObj->SetMergedItem( makeSdrTextRightDistItem( nTextRight ) );
            pTObj->SetMergedItem( makeSdrTextUpperDistItem( nTextTop ) );
            pTObj->SetMergedItem( makeSdrTextLowerDistItem( nTextBottom ) );
            pTObj->SetMergedItem( SdrTextFixedCellHeightItem( true ) );

            if ( dynamic_cast<const SdrObjCustomShape* >(pTObj) ==  nullptr )
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
                    if ( dynamic_cast<const SdrObjCustomShape* >(pTObj) ==  nullptr && !bFitShapeToText && !bWordWrap )
                    {
                        SdrTextObj* pText = dynamic_cast<SdrTextObj*>( pTObj  );
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
                                        pTObj->SetMergedItem( SdrTextVertAdjustItem( SDRTEXTVERTADJUST_CENTER ) );
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
                                        pTObj->SetMergedItem( SdrTextHorzAdjustItem( SDRTEXTHORZADJUST_CENTER ) );
                                }
                            }
                        }
                    }
                    // rotate text with shape?
                    sal_Int32 nAngle = ( rObjData.nSpFlags & SP_FFLIPV ) ? -mnFix16Angle : mnFix16Angle;    // #72116# vertical flip -> rotate by using the other way
                    nAngle += nTextRotationAngle;

                    if ( dynamic_cast< const SdrObjCustomShape* >(pTObj) ==  nullptr )
                    {
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
                            pTObj->NbcRotate( rObjData.aBoundRect.Center(), nAngle, sin( a ), cos( a ) );
                        }
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
    }
    else
    {
        if ( maShapeRecords.SeekToContent( rSt, DFF_msofbtUDefProp, SEEK_FROM_CURRENT_AND_RESTART ) )
        {
            maShapeRecords.Current()->SeekToBegOfRecord( rSt );
            DffPropertyReader aSecPropSet( *this );
            aSecPropSet.ReadPropSet( rSt, pData );
            sal_Int32 nTableProperties = aSecPropSet.GetPropertyValue( DFF_Prop_tableProperties, 0 );
            if ( nTableProperties & 3 )
            {
                if ( aSecPropSet.SeekToContent( DFF_Prop_tableRowProperties, rSt ) )
                {
                    sal_Int16 i, nRowCount = 0;
                    rSt.ReadInt16( nRowCount ).ReadInt16( i ).ReadInt16( i );
                    const size_t nMinRecordSize = 4;
                    const size_t nMaxRecords = rSt.remainingSize() / nMinRecordSize;
                    if (nRowCount > 0 && static_cast<size_t>(nRowCount) > nMaxRecords)
                    {
                        SAL_WARN("filter.ms", "Parsing error: " << nMaxRecords <<
                                 " max possible entries, but " << nRowCount << " claimed, truncating");
                        nRowCount = nMaxRecords;
                    }
                    if (nRowCount > 0)
                    {
                        std::unique_ptr<sal_uInt32[]> pTableArry(new sal_uInt32[ nRowCount + 2 ]);
                        pTableArry[ 0 ] = nTableProperties;
                        pTableArry[ 1 ] = nRowCount;
                        for ( i = 0; i < nRowCount; i++ )
                            rSt.ReadUInt32( pTableArry[ i + 2 ] );
                        rData.pTableRowProperties = std::move(pTableArry);
                    }
                }
            }
        }
    }
    if ( pRet ) // sj: #i38501#, and taking care of connections to group objects
    {
        if ( rObjData.nSpFlags & SP_FBACKGROUND )
        {
            pRet->NbcSetSnapRect( Rectangle( Point(), rData.pPage.page->GetSize() ) );   // set size
        }
        if ( rPersistEntry.pSolverContainer )
        {
            for (SvxMSDffConnectorRule* pPtr : rPersistEntry.pSolverContainer->aCList)
            {
                if ( rObjData.nShapeId == pPtr->nShapeC )
                    pPtr->pCObj = pRet;
                else
                {
                    SdrObject* pConnectObj = pRet;
                    if ( pOriginalObj && dynamic_cast< const SdrObjGroup* >(pRet) !=  nullptr )
                    {   /* check if the original object from the escherimport is part of the group object,
                        if this is the case, we will use the original object to connect to */
                        SdrObjListIter aIter( *pRet, SdrIterMode::DeepWithGroups );
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
                sal_uInt32 nSec = ( rObjData.nShapeId >> 10 ) - 1;
                if ( !maFidcls.empty() && ( nSec < mnIdClusters ) )
                    maFidcls[ nSec ].dgid = rPersistEntry.nDrawingDgId; // insert the correct drawing id;
            }
        }
        if ( GetPropertyValue( DFF_Prop_fNoFillHitTest, 0 ) & 0x10 )
        {
            if ( (MSO_FillType)GetPropertyValue( DFF_Prop_fillType, mso_fillSolid ) == mso_fillBackground )
            {
                rData.aBackgroundColoredObjects.push_back( pRet );
            }
        }
    }
    return pRet;
}

SdrPowerPointImport::SdrPowerPointImport( PowerPointImportParam& rParam, const OUString& rBaseURL ) :
    SdrEscherImport     ( rParam, rBaseURL ),
    bOk                 ( rStCtrl.GetErrorCode() == SVSTREAM_OK ),
    pPersistPtr         ( nullptr ),
    nPersistPtrAnz      ( 0 ),
    pDefaultSheet       ( nullptr ),
    m_pMasterPages      ( nullptr ),
    m_pSlidePages       ( nullptr ),
    m_pNotePages        ( nullptr ),
    nAktPageNum         ( 0 ),
    nDocStreamPos       ( 0 ),
    nPageColorsNum      ( 0xFFFF ),
    ePageColorsKind     ( PPT_MASTERPAGE ),
    eAktPageKind        ( PPT_MASTERPAGE )
{
    if ( bOk )
    {
        rStCtrl.Seek( STREAM_SEEK_TO_END );
        nStreamLen = rStCtrl.Tell();

        // try to allocate the UserEditAtom via CurrentUserAtom
        sal_uInt32 nCurrentUserEdit = rParam.aCurrentUserAtom.nCurrentUserEdit;
        if ( nCurrentUserEdit )
        {
            rStCtrl.Seek( nCurrentUserEdit );
            ReadPptUserEditAtom( rStCtrl, aUserEditAtom );
        }
        if ( !aUserEditAtom.nOffsetPersistDirectory )
        {   // if there is no UserEditAtom try to search the last one

            rStCtrl.Seek( 0 );
            DffRecordManager aPptRecManager;                            // contains all first level container and atoms
            aPptRecManager.Consume( rStCtrl, nStreamLen );
            DffRecordHeader* pHd;
            for ( pHd = aPptRecManager.Last(); pHd; pHd = aPptRecManager.Prev() )
            {
                if ( pHd->nRecType == PPT_PST_UserEditAtom )
                {
                    pHd->SeekToBegOfRecord( rStCtrl );
                    ReadPptUserEditAtom( rStCtrl, aUserEditAtom );
                    break;
                }
            }
            if ( !pHd )
                bOk = false;
        }
    }
    if ( rStCtrl.GetError() != 0 )
        bOk = false;

    if ( bOk )
    {
        nPersistPtrAnz = aUserEditAtom.nMaxPersistWritten + 1;
        if ( ( nPersistPtrAnz >> 2 ) > nStreamLen )     // sj: at least nPersistPtrAnz is not allowed to be greater than filesize
            bOk = false;                                // (it should not be greater than the PPT_PST_PersistPtrIncrementalBlock, but
                                                        // we are reading this block later, so we do not have access yet)

        if ( bOk && ( nPersistPtrAnz < ( SAL_MAX_UINT32 / sizeof( sal_uInt32 ) ) -1 ) )
            pPersistPtr = new (std::nothrow) sal_uInt32[ nPersistPtrAnz + 1 ];
        if ( !pPersistPtr )
            bOk = false;
        if ( bOk )
        {
            memset( pPersistPtr, 0x00, (nPersistPtrAnz+1) * sizeof(sal_uInt32) );

            // SJ: new search mechanism from bottom to top (Issue 21122)
            PptUserEditAtom aCurrentEditAtom( aUserEditAtom );
            sal_uInt32 nCurrentEditAtomStrmPos = aCurrentEditAtom.aHd.GetRecEndFilePos();
            while( nCurrentEditAtomStrmPos )
            {
                sal_uInt32 nPersistIncPos = aCurrentEditAtom.nOffsetPersistDirectory;
                if (nPersistIncPos && rStCtrl.Seek(nPersistIncPos) == nPersistIncPos)
                {
                    DffRecordHeader aPersistHd;
                    ReadDffRecordHeader( rStCtrl, aPersistHd );
                    if ( aPersistHd.nRecType == PPT_PST_PersistPtrIncrementalBlock )
                    {
                        sal_uLong nPibLen = aPersistHd.GetRecEndFilePos();
                        while ( bOk && ( rStCtrl.GetError() == 0 ) && ( rStCtrl.Tell() < nPibLen ) )
                        {
                            sal_uInt32 nOfs(0);
                            rStCtrl.ReadUInt32( nOfs );
                            sal_uInt32 nAnz = nOfs;
                            nOfs &= 0x000FFFFF;
                            nAnz >>= 20;
                            while ( bOk && ( rStCtrl.GetError() == 0 ) && ( nAnz > 0 ) && ( nOfs <= nPersistPtrAnz ) )
                            {
                                sal_uInt32 nPt(0);
                                rStCtrl.ReadUInt32( nPt );
                                if ( !pPersistPtr[ nOfs ] )
                                {
                                    pPersistPtr[ nOfs ] = nPt;
                                    if ( pPersistPtr[ nOfs ] > nStreamLen )
                                    {
                                        bOk = false;
                                        OSL_FAIL("SdrPowerPointImport::Ctor(): Ungueltiger Eintrag im Persist-Directory!");
                                    }
                                }
                                nAnz--;
                                nOfs++;
                            }
                            if ( bOk && nAnz > 0 )
                            {
                                OSL_FAIL("SdrPowerPointImport::Ctor(): Nicht alle Persist-Directory Entraege gelesen!");
                                bOk = false;
                            }
                        }
                    }
                }
                nCurrentEditAtomStrmPos = aCurrentEditAtom.nOffsetLastEdit < nCurrentEditAtomStrmPos ? aCurrentEditAtom.nOffsetLastEdit : 0;
                if ( nCurrentEditAtomStrmPos )
                {
                    rStCtrl.Seek( nCurrentEditAtomStrmPos );
                    ReadPptUserEditAtom( rStCtrl, aCurrentEditAtom );
                }
            }
        }
    }
    if ( rStCtrl.GetError() != 0 )
        bOk = false;
    if ( bOk )
    {   // check Document PersistEntry
        nDocStreamPos = aUserEditAtom.nDocumentRef;
        if ( nDocStreamPos > nPersistPtrAnz )
        {
            OSL_FAIL("SdrPowerPointImport::Ctor(): aUserEditAtom.nDocumentRef ungueltig!");
            bOk = false;
        }
    }
    if ( bOk )
    {   // check Document FilePos
        nDocStreamPos = pPersistPtr[ nDocStreamPos ];
        if ( nDocStreamPos >= nStreamLen )
        {
            OSL_FAIL("SdrPowerPointImport::Ctor(): nDocStreamPos >= nStreamLen!");
            bOk = false;
        }
    }
    if ( bOk )
    {
        rStCtrl.Seek( nDocStreamPos );
        aDocRecManager.Consume( rStCtrl );

        DffRecordHeader aDocHd;
        ReadDffRecordHeader( rStCtrl, aDocHd );
        // read DocumentAtom
        DffRecordHeader aDocAtomHd;
        ReadDffRecordHeader( rStCtrl, aDocAtomHd );
        if ( aDocHd.nRecType == PPT_PST_Document && aDocAtomHd.nRecType == PPT_PST_DocumentAtom )
        {
            aDocAtomHd.SeekToBegOfRecord( rStCtrl );
            ReadPptDocumentAtom( rStCtrl, aDocAtom );
        }
        else
            bOk = false;

        if ( bOk )
        {
            if (!m_pFonts)
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
                            OSL_FAIL( "SdrTextSpecInfoAtomInterpreter::Ctor(): parsing error, this document needs to be analysed (SJ)" );
                        }
                    }
#endif
                }
            }

            // TODO:: PPT_PST_TxPFStyleAtom

            // read SlidePersists
            m_pMasterPages = new PptSlidePersistList;
            m_pSlidePages = new PptSlidePersistList;
            m_pNotePages  = new PptSlidePersistList;

            // now always creating the handout page, it will be the first in our masterpage list
            std::unique_ptr<PptSlidePersistEntry> pE(new PptSlidePersistEntry);
            pE->aPersistAtom.nPsrReference = aDocAtom.nHandoutMasterPersist;
            pE->bHandoutMaster = true;
            if ( !aDocAtom.nHandoutMasterPersist )
                pE->bStarDrawFiller = true;     // this is a dummy master page
            m_pMasterPages->insert(m_pMasterPages->begin(), std::move(pE));

            DffRecordHeader* pSlideListWithTextHd = aDocRecManager.GetRecordHeader( PPT_PST_SlideListWithText );
            PptSlidePersistEntry* pPreviousPersist = nullptr;
            for (sal_uInt16 nPageListNum = 0;
                 pSlideListWithTextHd && nPageListNum < 3; ++nPageListNum)
            {
                pSlideListWithTextHd->SeekToContent( rStCtrl );
                PptSlidePersistList* pPageList = GetPageList( PptPageKind( nPageListNum ) );
                sal_uInt32 nSlideListWithTextHdEndOffset = pSlideListWithTextHd->GetRecEndFilePos();
                while ( SeekToRec( rStCtrl, PPT_PST_SlidePersistAtom, nSlideListWithTextHdEndOffset ) )
                {
                    if ( pPreviousPersist )
                        pPreviousPersist->nSlidePersistEndOffset = rStCtrl.Tell();
                    std::unique_ptr<PptSlidePersistEntry> pE2(new PptSlidePersistEntry);
                    ReadPptSlidePersistAtom( rStCtrl, pE2->aPersistAtom );
                    pE2->nSlidePersistStartOffset = rStCtrl.Tell();
                    pE2->ePageKind = PptPageKind( nPageListNum );
                    pPreviousPersist = pE2.get();
                    pPageList->push_back(std::move(pE2));
                }
                if ( pPreviousPersist )
                    pPreviousPersist->nSlidePersistEndOffset = nSlideListWithTextHdEndOffset;
                pSlideListWithTextHd = aDocRecManager.GetRecordHeader( PPT_PST_SlideListWithText, SEEK_FROM_CURRENT );
            }

            // we will ensure that there is at least one master page
            if (m_pMasterPages->size() == 1) // -> there is only a handout page available
            {
                std::unique_ptr<PptSlidePersistEntry> pE2(new PptSlidePersistEntry);
                pE2->bStarDrawFiller = true;            // this is a dummy master page
                m_pMasterPages->insert(m_pMasterPages->begin() + 1, std::move(pE2));
            }

            // now we will insert at least one notes master for each master page
            sal_uInt16 nMasterPage;
            sal_uInt16 nMasterPages = m_pMasterPages->size() - 1;
            for ( nMasterPage = 0; nMasterPage < nMasterPages; nMasterPage++ )
            {
                std::unique_ptr<PptSlidePersistEntry> pE2(new PptSlidePersistEntry);
                pE2->bNotesMaster = true;
                pE2->bStarDrawFiller = true;            // this is a dummy master page
                if ( !nMasterPage && aDocAtom.nNotesMasterPersist )
                {   // special treatment for the first notes master
                    pE2->aPersistAtom.nPsrReference = aDocAtom.nNotesMasterPersist;
                    pE2->bStarDrawFiller = false;   // this is a dummy master page
                }
                m_pMasterPages->insert(m_pMasterPages->begin() + ((nMasterPage + 1) << 1), std::move(pE2));
            }

            // read for each page the SlideAtom respectively the NotesAtom if it exists
            for (sal_uInt16 nPageListNum = 0; nPageListNum < 3; ++nPageListNum)
            {
                PptSlidePersistList* pPageList = GetPageList( PptPageKind( nPageListNum ) );
                for ( size_t nPageNum = 0; nPageNum < pPageList->size(); nPageNum++ )
                {
                    PptSlidePersistEntry& rE2 = (*pPageList)[ nPageNum ];
                    sal_uLong nPersist = rE2.aPersistAtom.nPsrReference;
                    if ( ( nPersist > 0 ) && ( nPersist < nPersistPtrAnz ) )
                    {
                        sal_uLong nFPos = pPersistPtr[ nPersist ];
                        if ( nFPos < nStreamLen )
                        {
                            rStCtrl.Seek( nFPos );
                            DffRecordHeader aSlideHd;
                            ReadDffRecordHeader( rStCtrl, aSlideHd );
                            if ( SeekToRec( rStCtrl, PPT_PST_SlideAtom, aSlideHd.GetRecEndFilePos() ) )
                                ReadPptSlideAtom( rStCtrl, rE2.aSlideAtom );
                            else if ( SeekToRec( rStCtrl, PPT_PST_NotesAtom, aSlideHd.GetRecEndFilePos() ) )
                                ReadPptNotesAtom( rStCtrl, rE2.aNotesAtom );
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
                                        ReadDffRecordHeader( rStCtrl, aDgRecordHeader );
                                        rE2.nDrawingDgId = aDgRecordHeader.nRecInstance;
                                        aDgRecordHeader.SeekToEndOfRecord( rStCtrl );
                                    }
                                    if ( SeekToRec( rStCtrl, DFF_msofbtSolverContainer, aPPTDgContainer.GetRecEndFilePos() ) )
                                    {
                                        rE2.pSolverContainer = new SvxMSDffSolverContainer;
                                        ReadSvxMSDffSolverContainer( rStCtrl, *( rE2.pSolverContainer ) );
                                    }
                                    aPPTDgContainer.SeekToBegOfRecord( rStCtrl );
                                    SetDgContainer( rStCtrl );  // set this, so that the escherimport is knowing of our drawings
                                }
                            }
                            // office xp is supporting more than one stylesheet
                            if ( ( rE2.ePageKind == PPT_MASTERPAGE ) && ( rE2.aSlideAtom.nMasterId == 0 ) && !rE2.bNotesMaster )
                            {
                                PPTTextSpecInfo aTxSI( 0 );
                                if ( aTxSIStyle.bValid && !aTxSIStyle.aList.empty() )
                                    aTxSI = *( aTxSIStyle.aList[ 0 ] );

                                rE2.pStyleSheet = new PPTStyleSheet( aSlideHd, rStCtrl, *this, aTxPFStyle, aTxSI );
                                pDefaultSheet = rE2.pStyleSheet;
                            }
                            if ( SeekToRec( rStCtrl, PPT_PST_ColorSchemeAtom, aSlideHd.GetRecEndFilePos() ) )
                                ReadPptColorSchemeAtom( rStCtrl, rE2.aColorScheme );
                            else
                            {
                                OSL_FAIL( "SdrPowerPointImport::Ctor(): could not get SlideColorScheme! (SJ)" );
                            }
                        }
                        else
                        {
                            OSL_FAIL("SdrPowerPointImport::Ctor(): Persist-Eintrag fehlerhaft! (SJ)");
                        }
                    }
                }
            }
            DffRecordHeader* pHeadersFootersHd = aDocRecManager.GetRecordHeader( PPT_PST_HeadersFooters );
            if ( pHeadersFootersHd )
            {
                HeaderFooterEntry aNormalMaster, aNotesMaster;
                for ( ; pHeadersFootersHd; pHeadersFootersHd = aDocRecManager.GetRecordHeader( PPT_PST_HeadersFooters, SEEK_FROM_CURRENT ) )
                {
                    if ( pHeadersFootersHd->nRecInstance == 3 )         // normal master
                        ImportHeaderFooterContainer( *pHeadersFootersHd, aNormalMaster );
                    else if ( pHeadersFootersHd->nRecInstance == 4 )    // notes master
                        ImportHeaderFooterContainer( *pHeadersFootersHd, aNotesMaster );
                }
                for (size_t i = 0; i < m_pMasterPages->size(); i++)
                {
                    if ((*m_pMasterPages)[ i ].bNotesMaster)
                        (*m_pMasterPages)[ i ].pHeaderFooterEntry = new HeaderFooterEntry( aNotesMaster );
                    else
                        (*m_pMasterPages)[ i ].pHeaderFooterEntry = new HeaderFooterEntry( aNormalMaster );
                }
            }
        }
    }
    if ( ( rStCtrl.GetError() != 0 ) || ( pDefaultSheet == nullptr ) )
        bOk = false;
    pPPTStyleSheet = pDefaultSheet;
    rStCtrl.Seek( 0 );
}

SdrPowerPointImport::~SdrPowerPointImport()
{
    for (SdHyperlinkEntry* i : aHyperList) {
        delete i;
    }
    aHyperList.clear();
    delete m_pMasterPages;
    delete m_pSlidePages;
    delete m_pNotePages;
    delete[] pPersistPtr;
}

bool PPTConvertOCXControls::ReadOCXStream( tools::SvRef<SotStorage>& rSrc,
        css::uno::Reference< css::drawing::XShape > *pShapeRef )
{
    bool bRes = false;
    uno::Reference< form::XFormComponent > xFComp;
    if ( mpPPTImporter && mpPPTImporter->ReadFormControl( rSrc, xFComp ) )
    {
        if ( xFComp.is() )
        {
            css::awt::Size aSz;  // not used in import
            bRes = InsertControl( xFComp, aSz,pShapeRef, false/*bFloatingCtrl*/);
        }
    }
    return bRes;
}

bool PPTConvertOCXControls::InsertControl(
        const css::uno::Reference< css::form::XFormComponent > &rFComp,
        const css::awt::Size& rSize,
        css::uno::Reference< css::drawing::XShape > *pShape,
        bool /*bFloatingCtrl*/)
{
    bool bRetValue = false;
    try
    {
        css::uno::Reference< css::drawing::XShape >  xShape;

        const css::uno::Reference< css::container::XIndexContainer > & rFormComps =
            GetFormComps();

        css::uno::Any aTmp( &rFComp, cppu::UnoType<css::form::XFormComponent>::get() );

        rFormComps->insertByIndex( rFormComps->getCount(), aTmp );

        const css::uno::Reference< css::lang::XMultiServiceFactory > & rServiceFactory =
            GetServiceFactory();
        if( rServiceFactory.is() )
        {
            css::uno::Reference< css::uno::XInterface >  xCreate = rServiceFactory
                ->createInstance( "com.sun.star.drawing.ControlShape" );
            if( xCreate.is() )
            {
                xShape.set(xCreate, css::uno::UNO_QUERY);
                if ( xShape.is() )
                {
                    xShape->setSize(rSize);
                    // set the Control-Model at the Control-Shape
                    css::uno::Reference< css::drawing::XControlShape >  xControlShape( xShape,
                        css::uno::UNO_QUERY );
                    css::uno::Reference< css::awt::XControlModel >  xControlModel( rFComp,
                        css::uno::UNO_QUERY );
                    if ( xControlShape.is() && xControlModel.is() )
                    {
                        xControlShape->setControl( xControlModel );
                        if (pShape)
                            *pShape = xShape;
                        bRetValue = true;
                    }
                }
            }
        }
    }
    catch( ... )
    {
        bRetValue = false;
    }
    return bRetValue;
};
void PPTConvertOCXControls::GetDrawPage()
{
    if( !xDrawPage.is() && mxModel.is() )
    {
        css::uno::Reference< css::drawing::XDrawPages > xDrawPages;
        switch( ePageKind )
        {
            case PPT_SLIDEPAGE :
            case PPT_NOTEPAGE :
            {
                css::uno::Reference< css::drawing::XDrawPagesSupplier >
                        xDrawPagesSupplier( mxModel, css::uno::UNO_QUERY);
                if ( xDrawPagesSupplier.is() )
                    xDrawPages = xDrawPagesSupplier->getDrawPages();
            }
            break;

            case PPT_MASTERPAGE :
            {
                css::uno::Reference< css::drawing::XMasterPagesSupplier >
                        xMasterPagesSupplier( mxModel, css::uno::UNO_QUERY);
                if ( xMasterPagesSupplier.is() )
                    xDrawPages = xMasterPagesSupplier->getMasterPages();
            }
            break;
        }
        if ( xDrawPages.is() && xDrawPages->getCount() )
        {
            xDrawPages->getCount();
            css::uno::Any aAny( xDrawPages->getByIndex( xDrawPages->getCount() - 1 ) );
            aAny >>= xDrawPage;
        }
    }
}

bool SdrPowerPointOLEDecompress( SvStream& rOutput, SvStream& rInput, sal_uInt32 nInputSize )
{
    sal_uInt32 nOldPos = rInput.Tell();
    std::unique_ptr<char[]> pBuf(new char[ nInputSize ]);
    rInput.ReadBytes(pBuf.get(), nInputSize);
    ZCodec aZCodec( 0x8000, 0x8000 );
    aZCodec.BeginCompression();
    SvMemoryStream aSource( pBuf.get(), nInputSize, StreamMode::READ );
    aZCodec.Decompress( aSource, rOutput );
    const bool bSuccess(0L != aZCodec.EndCompression());
    rInput.Seek( nOldPos );
    return bSuccess;
}

// #i32596# - add new parameter <_nCalledByGroup>
SdrObject* SdrPowerPointImport::ImportOLE( long nOLEId,
                                           const Graphic& rGraf,
                                           const Rectangle& rBoundRect,
                                           const Rectangle& rVisArea,
                                           const int /*_nCalledByGroup*/,
                                           sal_Int64 /*nAspect*/ ) const
{
    SdrObject* pRet = nullptr;

    sal_uInt32 nOldPos = rStCtrl.Tell();

    Graphic aGraphic( rGraf );

    if ( const_cast<SdrPowerPointImport*>(this)->maShapeRecords.SeekToContent( rStCtrl, DFF_msofbtClientData, SEEK_FROM_CURRENT_AND_RESTART ) )
    {
        DffRecordHeader aPlaceHd;

        auto nEndRecPos = SanitizeEndPos(rStCtrl, const_cast<SdrPowerPointImport*>(this)->maShapeRecords.Current()->GetRecEndFilePos());
        while ( ( rStCtrl.GetError() == 0 )
            && ( rStCtrl.Tell() < nEndRecPos ) )
        {
            ReadDffRecordHeader( rStCtrl, aPlaceHd );
            if ( aPlaceHd.nRecType == PPT_PST_RecolorInfoAtom )
            {
                const_cast<SdrPowerPointImport*>(this)->RecolorGraphic( rStCtrl, aPlaceHd.nRecLen, aGraphic );
                break;
            }
            else
            {
                if (!aPlaceHd.SeekToEndOfRecord(rStCtrl))
                    break;
            }
        }
    }

    for (PPTOleEntry* pOe : const_cast<SdrPowerPointImport*>(this)->aOleObjectList)
    {
        if ( pOe->nId != (sal_uInt32)nOLEId )
            continue;

        rStCtrl.Seek( pOe->nRecHdOfs );

        DffRecordHeader aHd;
        ReadDffRecordHeader( rStCtrl, aHd );

        sal_uInt32 nLen = aHd.nRecLen - 4;
        if ( (sal_Int32)nLen > 0 )
        {
            bool bSuccess = false;

            rStCtrl.SeekRel( 4 );

            ::utl::TempFile aTmpFile;
            aTmpFile.EnableKillingFile();

            if ( aTmpFile.IsValid() )
            {
                std::unique_ptr<SvStream> pDest(::utl::UcbStreamHelper::CreateStream( aTmpFile.GetURL(), StreamMode::TRUNC | StreamMode::WRITE ));
                if ( pDest )
                    bSuccess = SdrPowerPointOLEDecompress( *pDest, rStCtrl, nLen );
            }
            if ( bSuccess )
            {
                std::unique_ptr<SvStream> pDest(::utl::UcbStreamHelper::CreateStream( aTmpFile.GetURL(), StreamMode::READ ));
                Storage* pObjStor = pDest ? new Storage( *pDest, true ) : nullptr;
                if ( pObjStor )
                {
                    tools::SvRef<SotStorage> xObjStor( new SotStorage( pObjStor ) );
                    if ( xObjStor.is() && !xObjStor->GetError() )
                    {
                        if ( xObjStor->GetClassName() == SvGlobalName() )
                        {
                            xObjStor->SetClass( SvGlobalName( pObjStor->GetClassId() ), pObjStor->GetFormat(), pObjStor->GetUserName() );
                        }
                        tools::SvRef<SotStorageStream> xSrcTst = xObjStor->OpenSotStream( "\1Ole" );
                        if ( xSrcTst.is() )
                        {
                            sal_uInt8 aTestA[ 10 ];
                            bool bGetItAsOle = (sizeof(aTestA) == xSrcTst->ReadBytes(aTestA, sizeof(aTestA)));
                            if ( !bGetItAsOle )
                            {   // maybe there is a contents stream in here
                                xSrcTst = xObjStor->OpenSotStream( "Contents", StreamMode::READWRITE | StreamMode::NOCREATE );
                                bGetItAsOle = (xSrcTst.is() &&
                                    sizeof(aTestA) == xSrcTst->ReadBytes(aTestA, sizeof(aTestA)));
                            }
                            if ( bGetItAsOle )
                            {
                                OUString aNm;
                                // if ( nSvxMSDffOLEConvFlags )
                                {
                                    uno::Reference < embed::XStorage > xDestStorage( pOe->pShell->GetStorage() );
                                    uno::Reference < embed::XEmbeddedObject > xObj =
                                        CheckForConvertToSOObj(nSvxMSDffOLEConvFlags, *xObjStor, xDestStorage, rGraf, rVisArea, maBaseURL);
                                    if( xObj.is() )
                                    {
                                        pOe->pShell->getEmbeddedObjectContainer().InsertEmbeddedObject( xObj, aNm );

                                        svt::EmbeddedObjectRef aObj( xObj, pOe->nAspect );

                                        // TODO/LATER: need MediaType for Graphic
                                        aObj.SetGraphic( rGraf, OUString() );
                                        pRet = new SdrOle2Obj( aObj, aNm, rBoundRect );
                                    }
                                }
                                if ( !pRet && ( pOe->nType == PPT_PST_ExControl ) )
                                {
                                    uno::Reference< frame::XModel > xModel( pOe->pShell->GetModel() );
                                    PPTConvertOCXControls aPPTConvertOCXControls( this, xModel, eAktPageKind );
                                    css::uno::Reference< css::drawing::XShape > xShape;
                                    if ( aPPTConvertOCXControls.ReadOCXStream( xObjStor, &xShape ) )
                                        pRet = GetSdrObjectFromXShape( xShape );

                                }
                                if ( !pRet )
                                {
                                    aNm = pOe->pShell->getEmbeddedObjectContainer().CreateUniqueObjectName();

                                    // object is not an own object
                                    tools::SvRef<SotStorage> xTarget = SotStorage::OpenOLEStorage( pOe->pShell->GetStorage(), aNm, StreamMode::READWRITE );
                                    if ( xObjStor.is() && xTarget.is() )
                                    {
                                        xObjStor->CopyTo( xTarget.get() );
                                        if( !xTarget->GetError() )
                                            xTarget->Commit();
                                    }
                                    xTarget.clear();

                                    uno::Reference < embed::XEmbeddedObject > xObj =
                                        pOe->pShell->getEmbeddedObjectContainer().GetEmbeddedObject( aNm );
                                    if ( xObj.is() )
                                    {
                                        if ( pOe->nAspect != embed::Aspects::MSOLE_ICON )
                                        {
                                            //TODO/LATER: keep on hacking?!
                                            // we don't want to be modified
                                            //xInplaceObj->EnableSetModified( sal_False );
                                            if ( rVisArea.IsEmpty() )
                                            {
                                                MapUnit aMapUnit = VCLUnoHelper::UnoEmbed2VCLMapUnit( xObj->getMapUnit( pOe->nAspect ) );
                                                Size aSize( OutputDevice::LogicToLogic( aGraphic.GetPrefSize(),
                                                    aGraphic.GetPrefMapMode(), MapMode( aMapUnit ) ) );

                                                awt::Size aSz;
                                                aSz.Width = aSize.Width();
                                                aSz.Height = aSize.Height();
                                                xObj->setVisualAreaSize( pOe->nAspect, aSz );
                                            }
                                            else
                                            {
                                                awt::Size aSize( rVisArea.GetSize().Width(), rVisArea.GetSize().Height() );
                                                xObj->setVisualAreaSize( pOe->nAspect, aSize );
                                            }
                                            //xInplaceObj->EnableSetModified( sal_True );
                                        }

                                        svt::EmbeddedObjectRef aObj( xObj, pOe->nAspect );

                                        // TODO/LATER: need MediaType for Graphic
                                        aObj.SetGraphic( aGraphic, OUString() );

                                        pRet = new SdrOle2Obj( aObj, aNm, rBoundRect );
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

SvMemoryStream* SdrPowerPointImport::ImportExOleObjStg( sal_uInt32 nPersistPtr, sal_uInt32& nOleId ) const
{
    SvMemoryStream* pRet = nullptr;
    if ( nPersistPtr && ( nPersistPtr < nPersistPtrAnz ) )
    {
        sal_uInt32 nOldPos, nOfs = pPersistPtr[ nPersistPtr ];
        nOldPos = rStCtrl.Tell();
        rStCtrl.Seek( nOfs );
        DffRecordHeader aHd;
        ReadDffRecordHeader( rStCtrl, aHd );
        if ( aHd.nRecType == DFF_PST_ExOleObjStg )
        {
            sal_uInt32 nLen = aHd.nRecLen - 4;
            if ( (sal_Int32)nLen > 0 )
            {
                rStCtrl.ReadUInt32( nOleId );
                pRet = new SvMemoryStream;
                ZCodec aZCodec( 0x8000, 0x8000 );
                aZCodec.BeginCompression();
                aZCodec.Decompress( rStCtrl, *pRet );
                if ( !aZCodec.EndCompression() )
                {
                    delete pRet;
                    pRet = nullptr;
                }
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

        sal_uInt32 nOldPos = rStCtrl.Tell();
        if ( nFilterOptions & 1 )
        {
            pHd = aDocRecManager.GetRecordHeader( PPT_PST_List );
            if ( pHd )
            {
                // we try to locate the basic atom
                pHd->SeekToContent( rStCtrl );
                if ( SeekToRec( rStCtrl, PPT_PST_VBAInfo, pHd->GetRecEndFilePos(), pHd ) )
                {
                    if ( SeekToRec( rStCtrl, PPT_PST_VBAInfoAtom, pHd->GetRecEndFilePos(), pHd ) )
                    {
                        sal_uInt32 nPersistPtr, nIDoNotKnow1, nIDoNotKnow2;
                        rStCtrl.ReadUInt32( nPersistPtr )
                               .ReadUInt32( nIDoNotKnow1 )
                               .ReadUInt32( nIDoNotKnow2 );

                        sal_uInt32 nOleId;
                        SvMemoryStream* pBas = ImportExOleObjStg( nPersistPtr, nOleId );
                        if ( pBas )
                        {
                            tools::SvRef<SotStorage> xSource( new SotStorage( pBas, true ) );
                            tools::SvRef<SotStorage> xDest( new SotStorage( new SvMemoryStream(), true ) );
                            if ( xSource.is() && xDest.is() )
                            {
                                // is this a visual basic storage ?
                                tools::SvRef<SotStorage> xSubStorage = xSource->OpenSotStorage( "VBA",
                                    StreamMode::READWRITE | StreamMode::NOCREATE | StreamMode::SHARE_DENYALL );
                                if( xSubStorage.is() && ( SVSTREAM_OK == xSubStorage->GetError() ) )
                                {
                                    tools::SvRef<SotStorage> xMacros = xDest->OpenSotStorage( "MACROS" );
                                    if ( xMacros.is() )
                                    {
                                        SvStorageInfoList aList;
                                        xSource->FillInfoList( &aList );
                                        sal_uInt32 i;

                                        bool bCopied = true;
                                        for ( i = 0; i < aList.size(); i++ )    // copy all entries
                                        {
                                            const SvStorageInfo& rInfo = aList[ i ];
                                            if ( !xSource->CopyTo( rInfo.GetName(), xMacros.get(), rInfo.GetName() ) )
                                                bCopied = false;
                                        }
                                        if ( i && bCopied )
                                        {
                                            uno::Reference < embed::XStorage > xDoc( pShell->GetStorage() );
                                            if ( xDoc.is() )
                                            {
                                                tools::SvRef<SotStorage> xVBA = SotStorage::OpenOLEStorage( xDoc, SvxImportMSVBasic::GetMSBasicStorageName() );
                                                if ( xVBA.is() && ( xVBA->GetError() == SVSTREAM_OK ) )
                                                {
                                                    tools::SvRef<SotStorage> xSubVBA = xVBA->OpenSotStorage( "_MS_VBA_Overhead" );
                                                    if ( xSubVBA.is() && ( xSubVBA->GetError() == SVSTREAM_OK ) )
                                                    {
                                                        tools::SvRef<SotStorageStream> xOriginal = xSubVBA->OpenSotStream( "_MS_VBA_Overhead2" );
                                                        if ( xOriginal.is() && ( xOriginal->GetError() == SVSTREAM_OK ) )
                                                        {
                                                            if ( nPersistPtr && ( nPersistPtr < nPersistPtrAnz ) )
                                                            {
                                                                rStCtrl.Seek( pPersistPtr[ nPersistPtr ] );
                                                                ReadDffRecordHeader( rStCtrl, *pHd );

                                                                xOriginal->WriteUInt32( nIDoNotKnow1 )
                                                                          .WriteUInt32( nIDoNotKnow2 );

                                                                sal_uInt32 nToCopy, nBufSize;
                                                                nToCopy = pHd->nRecLen;
                                                                std::unique_ptr<sal_uInt8[]> pBuf(new sal_uInt8[ 0x40000 ]); // 256KB Buffer
                                                                if ( pBuf )
                                                                {
                                                                    while ( nToCopy )
                                                                    {
                                                                        nBufSize = ( nToCopy >= 0x40000 ) ? 0x40000 : nToCopy;
                                                                        rStCtrl.ReadBytes(pBuf.get(), nBufSize);
                                                                        xOriginal->WriteBytes(pBuf.get(), nBufSize);
                                                                        nToCopy -= nBufSize;
                                                                    }
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                                xVBA->Commit();
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
        pHd = aDocRecManager.GetRecordHeader( PPT_PST_ExObjList );
        if ( pHd )
        {
            DffRecordHeader*    pExEmbed = nullptr;

            pHd->SeekToBegOfRecord( rStCtrl );
            std::unique_ptr<DffRecordManager> pExObjListManager(new DffRecordManager( rStCtrl ));
            sal_uInt16 i, nRecType(PPT_PST_ExEmbed);

            for ( i = 0; i < 2; i++ )
            {
                switch ( i )
                {
                    case 0 : nRecType = PPT_PST_ExEmbed; break;
                    case 1 : nRecType = PPT_PST_ExControl; break;
                }
                for ( pExEmbed = pExObjListManager->GetRecordHeader( nRecType );
                        pExEmbed; pExEmbed = pExObjListManager->GetRecordHeader( nRecType, SEEK_FROM_CURRENT ) )
                {
                    pExEmbed->SeekToContent( rStCtrl );

                    DffRecordHeader aExOleAtHd;
                    if ( SeekToRec( rStCtrl, PPT_PST_ExOleObjAtom, pExEmbed->GetRecEndFilePos(), &aExOleAtHd ) )
                    {
                        PptExOleObjAtom aAt;
                        ReadPptExOleObjAtom( rStCtrl, aAt );

                        if ( aAt.nPersistPtr && ( aAt.nPersistPtr < nPersistPtrAnz ) )
                        {
                            sal_uInt32 nId;
                            rStCtrl.Seek( pPersistPtr[ aAt.nPersistPtr ] );
                            DffRecordHeader aHd;
                            ReadDffRecordHeader( rStCtrl, aHd );
                            if ( aHd.nRecType == DFF_PST_ExOleObjStg )
                            {
                                rStCtrl.ReadUInt32( nId );
                                aOleObjectList.push_back(
                                    new PPTOleEntry( aAt.nId, aHd.nFilePos, pShell, nRecType, aAt.nAspect ) );
                            }
                        }
                    }
                }
            }
        }
        rStCtrl.Seek( nOldPos );
    }
}

bool SdrPowerPointImport::ReadFontCollection()
{
    bool bRet = false;
    DffRecordHeader* pEnvHd = aDocRecManager.GetRecordHeader( PPT_PST_Environment );
    if ( pEnvHd )
    {
        sal_uLong nFPosMerk = rStCtrl.Tell(); // remember FilePos for restoring it later
        pEnvHd->SeekToContent( rStCtrl );
        DffRecordHeader aListHd;
        if ( SeekToRec( rStCtrl, PPT_PST_FontCollection, pEnvHd->GetRecEndFilePos(), &aListHd ) )
        {
            sal_uInt16 nCount2 = 0;
            while ( SeekToRec( rStCtrl, PPT_PST_FontEntityAtom, aListHd.GetRecEndFilePos() ) )
            {
                bRet = true;
                if (!m_pFonts)
                    m_pFonts = new PptFontCollection;
                std::unique_ptr<PptFontEntityAtom> pFont(new PptFontEntityAtom);
                ReadPptFontEntityAtom( rStCtrl, *pFont );

                vcl::Font aFont;
                aFont.SetCharSet( pFont->eCharSet );
                aFont.SetFamilyName( pFont->aName );
                aFont.SetFamily( pFont->eFamily );
                aFont.SetPitch( pFont->ePitch );
                aFont.SetFontHeight( 100 );

                // following block is necessary, because our old PowerPoint export did not set the
                // correct charset
                if ( pFont->aName.equalsIgnoreAsciiCase( "Wingdings" ) ||
                     pFont->aName.equalsIgnoreAsciiCase( "Wingdings 2" ) ||
                     pFont->aName.equalsIgnoreAsciiCase( "Wingdings 3" ) ||
                     pFont->aName.equalsIgnoreAsciiCase( "Monotype Sorts" ) ||
                     pFont->aName.equalsIgnoreAsciiCase( "Monotype Sorts 2" ) ||
                     pFont->aName.equalsIgnoreAsciiCase( "Webdings" ) ||
                     pFont->aName.equalsIgnoreAsciiCase( "StarBats" ) ||
                     pFont->aName.equalsIgnoreAsciiCase( "StarMath" ) ||
                     pFont->aName.equalsIgnoreAsciiCase( "ZapfDingbats" ) )
                {
                    pFont->eCharSet = RTL_TEXTENCODING_SYMBOL;
                };
                m_pFonts->insert(m_pFonts->begin() + nCount2++, std::move(pFont));
            }
        }
        rStCtrl.Seek( nFPosMerk ); // restore FilePos
    }
    return bRet;
}

PptSlidePersistList* SdrPowerPointImport::GetPageList(PptPageKind ePageKind) const
{
    if ( ePageKind == PPT_MASTERPAGE )
        return m_pMasterPages;
    if ( ePageKind == PPT_SLIDEPAGE )
        return m_pSlidePages;
    if ( ePageKind == PPT_NOTEPAGE )
        return m_pNotePages;
    return nullptr;
}

SdrOutliner* SdrPowerPointImport::GetDrawOutliner( SdrTextObj* pSdrText )
{
    if ( !pSdrText )
        return nullptr;
    else
        return &pSdrText->ImpGetDrawOutliner();
}


SdrObject* SdrPowerPointImport::ReadObjText( PPTTextObj* pTextObj, SdrObject* pSdrObj, SdPageCapsule pPage ) const
{
    SdrTextObj* pText = dynamic_cast<SdrTextObj*>( pSdrObj  );
    if ( pText )
    {
        if ( !ApplyTextObj( pTextObj, pText, pPage, nullptr, nullptr ) )
            pSdrObj = nullptr;
    }
    return pSdrObj;
}


SdrObject* SdrPowerPointImport::ApplyTextObj( PPTTextObj* pTextObj, SdrTextObj* pSdrText, SdPageCapsule /*pPage*/,
                                                SfxStyleSheet* pSheet, SfxStyleSheet** ppStyleSheetAry ) const
{
    SdrTextObj* pText = pSdrText;
    if ( pTextObj->Count() )
    {
        TSS_Type nDestinationInstance = pTextObj->GetDestinationInstance() ;
        SdrOutliner& rOutliner = pText->ImpGetDrawOutliner();
        if ( ( pText->GetObjInventor() == SdrInventor::Default ) && ( pText->GetObjIdentifier() == OBJ_TITLETEXT ) ) // Outliner-Style for Title-Text object?!? (->of DL)
            rOutliner.Init( OutlinerMode::TitleObject );             // Outliner reset

        bool bOldUpdateMode = rOutliner.GetUpdateMode();
        rOutliner.SetUpdateMode( false );
        if ( pSheet )
        {
            if ( rOutliner.GetStyleSheet( 0 ) != pSheet )
                rOutliner.SetStyleSheet( 0, pSheet );
        }
        rOutliner.SetVertical( pTextObj->GetVertical() );
        const PPTParagraphObj* pPreviousParagraph = nullptr;
        for ( PPTParagraphObj* pPara = pTextObj->First(); pPara; pPara = pTextObj->Next() )
        {
            sal_uInt32 nTextSize = pPara->GetTextSize();
            if ( ! ( nTextSize & 0xffff0000 ) )
            {
                PPTPortionObj* pPortion;
                std::unique_ptr<sal_Unicode[]> pParaText(new sal_Unicode[ nTextSize ]);
                sal_Int32 nCurrentIndex = 0;
                for ( pPortion = pPara->First(); pPortion; pPortion = pPara->Next() )
                {
                    if ( pPortion->mpFieldItem )
                        pParaText[ nCurrentIndex++ ] = ' ';
                    else
                    {
                        sal_Int32 nCharacters = pPortion->Count();
                        const sal_Unicode* pSource = pPortion->maString.getStr();
                        sal_Unicode* pDest = pParaText.get() + nCurrentIndex;

                        sal_uInt32 nFont;
                        pPortion->GetAttrib( PPT_CharAttr_Font, nFont, pTextObj->GetInstance() );
                        PptFontEntityAtom* pFontEnityAtom = GetFontEnityAtom( nFont );
                        if ( pFontEnityAtom && ( pFontEnityAtom->eCharSet == RTL_TEXTENCODING_SYMBOL ) )
                        {
                            sal_Unicode nUnicode;
                            for (sal_Int32 i = 0; i < nCharacters; i++ )
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
                sal_Int32  nParaIndex = pTextObj->GetCurrentIndex();
                SfxStyleSheet* pS = ( ppStyleSheetAry ) ? ppStyleSheetAry[ pPara->pParaSet->mnDepth ] : pSheet;

                ESelection aSelection( nParaIndex, 0, nParaIndex, 0 );
                rOutliner.Insert( OUString(), nParaIndex, pPara->pParaSet->mnDepth );
                rOutliner.QuickInsertText( OUString(pParaText.get(), nCurrentIndex), aSelection );
                rOutliner.SetParaAttribs( nParaIndex, rOutliner.GetEmptyItemSet() );
                if ( pS )
                    rOutliner.SetStyleSheet( nParaIndex, pS );

                for ( pPortion = pPara->First(); pPortion; pPortion = pPara->Next() )
                {
                    SfxItemSet aPortionAttribs( rOutliner.GetEmptyItemSet() );
                    std::unique_ptr<SvxFieldItem> pFieldItem(pPortion->GetTextField());
                    if ( pFieldItem )
                    {
                        rOutliner.QuickInsertField( *pFieldItem, ESelection( nParaIndex, aSelection.nEndPos, nParaIndex, aSelection.nEndPos + 1 ) );
                        aSelection.nEndPos++;
                    }
                    else
                    {
                        const sal_Unicode *pF, *pPtr = pPortion->maString.getStr();
                        const sal_Unicode *pMax = pPtr + pPortion->maString.getLength();
                        sal_Int32 nLen;
                        for ( pF = pPtr; pPtr < pMax; pPtr++ )
                        {
                            if ( *pPtr == 0xb )
                            {
                                nLen = pPtr - pF;
                                if ( nLen )
                                    aSelection.nEndPos =
                                        sal::static_int_cast< sal_uInt16 >(
                                            aSelection.nEndPos + nLen );
                                pF = pPtr + 1;
                                rOutliner.QuickInsertLineBreak( ESelection( nParaIndex, aSelection.nEndPos, nParaIndex, aSelection.nEndPos + 1 ) );
                                aSelection.nEndPos++;
                            }
                        }
                        nLen = pPtr - pF;
                        if ( nLen )
                            aSelection.nEndPos = sal::static_int_cast< sal_uInt16 >(
                                aSelection.nEndPos + nLen );
                    }
                    pPortion->ApplyTo( aPortionAttribs, (SdrPowerPointImport&)*this, nDestinationInstance, pTextObj );
                    rOutliner.QuickSetAttribs( aPortionAttribs, aSelection );
                    aSelection.nStartPos = aSelection.nEndPos;
                }
                boost::optional< sal_Int16 > oStartNumbering;
                SfxItemSet aParagraphAttribs( rOutliner.GetEmptyItemSet() );
                pPara->ApplyTo( aParagraphAttribs, oStartNumbering, (SdrPowerPointImport&)*this, nDestinationInstance, pPreviousParagraph );

                sal_uInt32  nIsBullet2 = 0; //, nInstance = nDestinationInstance != 0xffffffff ? nDestinationInstance : pTextObj->GetInstance();
                pPara->GetAttrib( PPT_ParaAttr_BulletOn, nIsBullet2, nDestinationInstance );
                if ( !nIsBullet2 )
                    aParagraphAttribs.Put( SfxBoolItem( EE_PARA_BULLETSTATE, false ) );

                pPreviousParagraph = pPara;
                if ( !aSelection.nStartPos )    // in PPT empty paragraphs never gets a bullet
                {
                    aParagraphAttribs.Put( SfxBoolItem( EE_PARA_BULLETSTATE, false ) );
                }
                aSelection.nStartPos = 0;
                rOutliner.QuickSetAttribs( aParagraphAttribs, aSelection );
            }
        }
        OutlinerParaObject* pNewText = rOutliner.CreateParaObject();
        rOutliner.Clear();
        rOutliner.SetUpdateMode( bOldUpdateMode );
        pText->SetOutlinerParaObject( pNewText );
    }
    return pText;
}

bool SdrPowerPointImport::SeekToDocument( DffRecordHeader* pRecHd ) const
{
    bool bRet;
    sal_uLong nFPosMerk = rStCtrl.Tell(); // remember FilePos for restoring it should the situation arise
    rStCtrl.Seek( nDocStreamPos );
    DffRecordHeader aDocHd;
    ReadDffRecordHeader( rStCtrl, aDocHd );
    bRet = aDocHd.nRecType == PPT_PST_Document;
    if ( bRet )
    {
        if ( pRecHd )
            *pRecHd = aDocHd;
        else
            aDocHd.SeekToBegOfRecord( rStCtrl );
    }
    if ( !bRet )
        rStCtrl.Seek( nFPosMerk ); // restore FilePos
    return bRet;
}

bool SdrPowerPointImport::SeekToContentOfProgTag( sal_Int32 nVersion, SvStream& rSt,
                                const DffRecordHeader& rSourceHd, DffRecordHeader& rContentHd )
{
    bool    bRetValue = false;
    sal_uInt32  nOldPos = rSt.Tell();

    DffRecordHeader aProgTagsHd, aProgTagBinaryDataHd;
    rSourceHd.SeekToContent( rSt );
    bool bFound = rSourceHd.nRecType == PPT_PST_ProgTags;
    if ( !bFound )
        bFound = SeekToRec( rSt, PPT_PST_ProgTags, rSourceHd.GetRecEndFilePos(), &aProgTagsHd );
    if ( bFound )
    {
        while( SeekToRec( rSt, PPT_PST_ProgBinaryTag, aProgTagsHd.GetRecEndFilePos(), &aProgTagBinaryDataHd ) )
        {
            ReadDffRecordHeader( rSt, rContentHd );
            if ( rContentHd.nRecType == PPT_PST_CString )
            {
                sal_uInt16  n = 6;
                sal_uInt32  i = rContentHd.nRecLen >> 1;
                if ( i > n )
                {
                    OUString aPre = read_uInt16s_ToOUString(rSt, n);
                    n = (sal_uInt16)( i - 6 );
                    OUString aSuf = read_uInt16s_ToOUString(rSt, n);
                    sal_Int32 nV = aSuf.toInt32();
                    if ( ( nV == nVersion ) && ( aPre == "___PPT" ) )
                    {
                        rContentHd.SeekToEndOfRecord( rSt );
                        ReadDffRecordHeader( rSt, rContentHd );
                        if ( rContentHd.nRecType == PPT_PST_BinaryTagData )
                        {
                            bRetValue = true;
                            break;
                        }
                    }
                }
            }
            if (!aProgTagBinaryDataHd.SeekToEndOfRecord(rSt))
                break;
        }
    }
    if ( !bRetValue )
        rSt.Seek( nOldPos );
    return bRetValue;
}

sal_uInt32 SdrPowerPointImport::GetAktPageId()
{
    PptSlidePersistList* pList = GetPageList( eAktPageKind );
    if ( pList && nAktPageNum < pList->size() )
        return (*pList)[ (sal_uInt16)nAktPageNum ].aPersistAtom.nSlideId;
    return 0;
}

bool SdrPowerPointImport::SeekToAktPage( DffRecordHeader* pRecHd ) const
{
    bool bRet = false;
    PptSlidePersistList* pList = GetPageList( eAktPageKind );
    if ( pList && ( nAktPageNum < pList->size() ) )
    {
        sal_uLong nPersist = (*pList)[ (sal_uInt16)nAktPageNum ].aPersistAtom.nPsrReference;
        if ( nPersist > 0 && nPersist < nPersistPtrAnz )
        {
            sal_uLong nFPos = 0;
            nFPos = pPersistPtr[ nPersist ];
            if ( nFPos < nStreamLen )
            {
                rStCtrl.Seek( nFPos );
                if ( pRecHd )
                    ReadDffRecordHeader( rStCtrl, *pRecHd );
                bRet = true;
            }
        }
    }
    return bRet;
}

sal_uInt16 SdrPowerPointImport::GetPageCount( PptPageKind ePageKind ) const
{
    PptSlidePersistList* pList = GetPageList( ePageKind );
    if ( pList )
        return pList->size();
    return 0;
}

void SdrPowerPointImport::SetPageNum( sal_uInt16 nPageNum, PptPageKind eKind )
{
    eAktPageKind = eKind;
    nAktPageNum = nPageNum;

    pPPTStyleSheet = nullptr;

    bool bHasMasterPage = true;
    sal_uInt16 nMasterIndex = 0;

    if ( eKind == PPT_MASTERPAGE )
        nMasterIndex = nPageNum;
    else
    {
        if ( HasMasterPage( nPageNum, eKind ) )
            nMasterIndex = GetMasterPageIndex( nPageNum, eKind );
        else
            bHasMasterPage = false;
    }
    if ( bHasMasterPage )
    {
        PptSlidePersistList* pPageList = GetPageList( PPT_MASTERPAGE );
        if ( pPageList && nMasterIndex < pPageList->size() )
        {
            PptSlidePersistEntry* pMasterPersist = &(*pPageList)[ nMasterIndex ];
            if ( ( pMasterPersist->pStyleSheet == nullptr ) && pMasterPersist->aSlideAtom.nMasterId )
            {
                nMasterIndex = m_pMasterPages->FindPage( pMasterPersist->aSlideAtom.nMasterId );
                if ( nMasterIndex != PPTSLIDEPERSIST_ENTRY_NOTFOUND )
                    pMasterPersist = &(*pPageList)[ nMasterIndex ];
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
    // PPT works with units of 576 dpi in any case. To avoid inacurracies
    // I do round the last decimal digit away.
    if ( nMapMul > 2 * nMapDiv )
    {
        MapUnit eMap = pSdrModel->GetScaleUnit();
        bool bInch = IsInch( eMap );
        long nInchMul = 1, nInchDiv = 1;
        if ( bInch )
        {   // temporarily convert size (for rounding it) from inch to metric units
            Fraction aFact(GetMapFactor(eMap,MapUnit::Map100thMM).X());
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

bool SdrPowerPointImport::GetColorFromPalette( sal_uInt16 nNum, Color& rColor ) const
{
    if ( nPageColorsNum != nAktPageNum || ePageColorsKind != eAktPageKind )
    {
        sal_uInt16 nSlideFlags = 0;
        PptSlidePersistList* pPageList = GetPageList( eAktPageKind );
        if ( pPageList && ( nAktPageNum < pPageList->size() ) )
        {
            assert( !pPageList->is_null( nAktPageNum ) );
            const PptSlidePersistEntry& rE = (*pPageList)[ nAktPageNum ];
            nSlideFlags = rE.aSlideAtom.nFlags;
            if ( ! ( nSlideFlags & 2 ) )
                const_cast<SdrPowerPointImport*>(this)->aPageColors = rE.aColorScheme;
        }
        if ( nSlideFlags & 2 )      // follow master colorscheme?
        {
            PptSlidePersistList* pPageList2 = GetPageList( PPT_MASTERPAGE );
            if ( pPageList2 )
            {
                PptSlidePersistEntry* pMasterPersist = nullptr;
                if ( eAktPageKind == PPT_MASTERPAGE )
                    pMasterPersist = &(*pPageList2)[ nAktPageNum ];
                else
                {
                    if ( HasMasterPage( nAktPageNum, eAktPageKind ) )
                    {
                        sal_uInt16 nMasterNum = GetMasterPageIndex( nAktPageNum, eAktPageKind );
                        if ( nMasterNum < pPageList2->size() )
                            pMasterPersist = &(*pPageList2)[ nMasterNum ];
                    }
                }
                if ( pMasterPersist )
                {
                    while( (pMasterPersist->aSlideAtom.nFlags & 2) // it is possible that a masterpage
                        && pMasterPersist->aSlideAtom.nMasterId )  // itself is following a master colorscheme
                    {
                        auto nOrigMasterId = pMasterPersist->aSlideAtom.nMasterId;
                        sal_uInt16 nNextMaster = m_pMasterPages->FindPage(nOrigMasterId);
                        if (nNextMaster == PPTSLIDEPERSIST_ENTRY_NOTFOUND)
                            break;
                        pMasterPersist = &(*pPageList2)[ nNextMaster ];
                        if (pMasterPersist->aSlideAtom.nMasterId == nOrigMasterId)
                        {
                            SAL_WARN("filter.ms", "loop in atom chain");
                            break;
                        }
                    }
                    const_cast<SdrPowerPointImport*>(this)->aPageColors = pMasterPersist->aColorScheme;
                }
            }
        }
        // register current color scheme
        const_cast<SdrPowerPointImport*>(this)->nPageColorsNum = nAktPageNum;
        const_cast<SdrPowerPointImport*>(this)->ePageColorsKind = eAktPageKind;
    }
    rColor = aPageColors.GetColor( nNum );
    return true;
}

bool SdrPowerPointImport::SeekToShape( SvStream& rSt, void* pClientData, sal_uInt32 nId ) const
{
    bool bRet = SvxMSDffManager::SeekToShape( rSt, pClientData, nId );
    if (!bRet && pClientData)
    {
        ProcessData& rData = *static_cast<ProcessData*>(pClientData);
        PptSlidePersistEntry& rPersistEntry = rData.rPersistEntry;
        if ( rPersistEntry.ePageKind == PPT_SLIDEPAGE )
        {
            if ( HasMasterPage( nAktPageNum, eAktPageKind ) )
            {
                sal_uInt16 nMasterNum = GetMasterPageIndex( nAktPageNum, eAktPageKind );
                PptSlidePersistList* pPageList = GetPageList( PPT_MASTERPAGE );
                if ( pPageList && ( nMasterNum < pPageList->size() ) )
                {
                    assert( !pPageList->is_null( nMasterNum ) );
                    const PptSlidePersistEntry& rPersist = (*pPageList)[ nMasterNum ];    // get the masterpage's persistentry
                    if ( rPersist.pPresentationObjects )
                    {
                        sal_uInt32 nCurrent(0L);
                        DffRecordList* pCList = maShapeRecords.pCList;              // we got a backup of the current position
                        if ( pCList )
                            nCurrent = pCList->nCurrent;
                        if ( const_cast<SdrPowerPointImport*>(this)->maShapeRecords.SeekToContent( rSt, DFF_msofbtClientData, SEEK_FROM_CURRENT_AND_RESTART ) )
                        {
                            sal_uInt32 nStreamPos = rSt.Tell();
                            PPTTextObj aTextObj( rSt, (SdrPowerPointImport&)*this, rPersistEntry, nullptr );
                            if ( aTextObj.Count() || aTextObj.GetOEPlaceHolderAtom() )
                            {
                                sal_uInt32 nShapePos = 0;
                                switch ( aTextObj.GetInstance() )
                                {
                                    case TSS_Type::Title :
                                        nShapePos = rPersist.pPresentationObjects[ (int)TSS_Type::PageTitle ];
                                    break;
                                    case TSS_Type::PageTitle :
                                        nShapePos = rPersist.pPresentationObjects[ (int)TSS_Type::PageTitle ];
                                    break;
                                    case TSS_Type::Subtitle :
                                    case TSS_Type::HalfBody :
                                    case TSS_Type::QuarterBody :
                                    case TSS_Type::Body :
                                        nShapePos = rPersist.pPresentationObjects[ (int)TSS_Type::Body ];
                                    break;
                                    default: break;
                                }
                                if ( nShapePos )
                                {
                                    rSt.Seek( nShapePos );
                                    bRet = true;
                                }
                            }
                            if ( !bRet )
                                rSt.Seek( nStreamPos );
                        }
                        if ( pCList )                                               // restoring
                            pCList->nCurrent = nCurrent;
                        const_cast<SdrPowerPointImport*>(this)->maShapeRecords.pCList = pCList;
                    }
                }
            }
        }
    }
    return bRet;
}

SdrPage* SdrPowerPointImport::MakeBlancPage( bool bMaster ) const
{
    SdrPage* pRet = pSdrModel->AllocPage( bMaster );
    pRet->SetSize( GetPageSize() );

    return pRet;
}

void ImportComment10( SvxMSDffManager& rMan, SvStream& rStCtrl, SdrPage* pPage, DffRecordHeader& rComment10Hd )
{
    OUString        sAuthor;
    OUString        sText;
    OUString        sInitials;

    sal_Int32       nIndex = 0;
    util::DateTime  aDateTime;
    sal_Int32       nPosX = 0;
    sal_Int32       nPosY = 0;


    auto nEndRecPos = DffPropSet::SanitizeEndPos(rStCtrl, rComment10Hd.GetRecEndFilePos());
    while ( ( rStCtrl.GetError() == 0 ) && ( rStCtrl.Tell() < nEndRecPos ) )
    {
        DffRecordHeader aCommentHd;
        ReadDffRecordHeader( rStCtrl, aCommentHd );
        switch( aCommentHd.nRecType )
        {
            case PPT_PST_CString :
            {
                OUString aString = SvxMSDffManager::MSDFFReadZString( rStCtrl,
                    aCommentHd.nRecLen, true );
                switch ( aCommentHd.nRecInstance )
                {
                    case 0 : sAuthor = aString;     break;
                    case 1 : sText = aString;       break;
                    case 2 : sInitials = aString;   break;
                }
            }
            break;

            case PPT_PST_CommentAtom10 :
            {
                rStCtrl.ReadInt32( nIndex )
                       .ReadInt16( aDateTime.Year )
                       .ReadUInt16( aDateTime.Month )
                       .ReadUInt16( aDateTime.Day )    // DayOfWeek
                       .ReadUInt16( aDateTime.Day )
                       .ReadUInt16( aDateTime.Hours )
                       .ReadUInt16( aDateTime.Minutes )
                       .ReadUInt16( aDateTime.Seconds )
                       .ReadUInt32( aDateTime.NanoSeconds )
                       .ReadInt32( nPosX )
                       .ReadInt32( nPosY );

                aDateTime.NanoSeconds *= ::tools::Time::nanoPerMilli;
            }
            break;
        }
        if (!aCommentHd.SeekToEndOfRecord(rStCtrl))
            break;
    }
    Point aPosition( nPosX, nPosY );
    rMan.Scale( aPosition );

    try
    {
        uno::Reference< office::XAnnotationAccess > xAnnotationAccess( pPage->getUnoPage(), UNO_QUERY_THROW );
        uno::Reference< office::XAnnotation > xAnnotation( xAnnotationAccess->createAndInsertAnnotation() );
        xAnnotation->setPosition( geometry::RealPoint2D( aPosition.X() / 100.0, aPosition.Y() / 100.0 ) );
        xAnnotation->setAuthor( sAuthor );
        xAnnotation->setDateTime( aDateTime );
        uno::Reference< text::XText > xText( xAnnotation->getTextRange() );
        xText->setString( sText );
    }
    catch( const uno::Exception& )
    {

    }
}


// be sure not to import masterpages with this method
void SdrPowerPointImport::ImportPage( SdrPage* pRet, const PptSlidePersistEntry* pMasterPersist )
{
    sal_uInt32 nMerk = rStCtrl.Tell();
    PptSlidePersistList* pList = GetPageList( eAktPageKind );
    if ( ( !pList ) || ( pList->size() <= nAktPageNum ) )
        return;
    PptSlidePersistEntry& rSlidePersist = (*pList)[ nAktPageNum ];
    if ( rSlidePersist.bStarDrawFiller )
        return;

    DffRecordHeader aPageHd;
    if ( SeekToAktPage( &aPageHd ) )
    {
        rSlidePersist.pHeaderFooterEntry = new HeaderFooterEntry( pMasterPersist );
        ProcessData aProcessData( rSlidePersist, SdPageCapsule(pRet) );
        auto nEndRecPos = SanitizeEndPos(rStCtrl, aPageHd.GetRecEndFilePos());
        while ( ( rStCtrl.GetError() == 0 ) && ( rStCtrl.Tell() < nEndRecPos ) )
        {
            DffRecordHeader aHd;
            ReadDffRecordHeader( rStCtrl, aHd );
            switch ( aHd.nRecType )
            {
                case PPT_PST_HeadersFooters :
                {
                    ImportHeaderFooterContainer( aHd, *rSlidePersist.pHeaderFooterEntry );
                }
                break;

                case PPT_PST_ProgTags :
                {
                    DffRecordHeader aContentDataHd;
                    if ( SeekToContentOfProgTag( 10, rStCtrl, aHd, aContentDataHd ) )
                    {
                        DffRecordHeader aComment10Hd;
                        while( ( rStCtrl.GetError() == 0 ) && SeekToRec( rStCtrl, PPT_PST_Comment10, aContentDataHd.GetRecEndFilePos(), &aComment10Hd ) )
                        {
                            ImportComment10( *this, rStCtrl, pRet, aComment10Hd );
                            if (!aComment10Hd.SeekToEndOfRecord(rStCtrl))
                                break;
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
                        auto nPPEndRecPos = SanitizeEndPos(rStCtrl, aPPDrawHd.GetRecEndFilePos());
                        while ( ( rStCtrl.GetError() == 0 ) && ( rStCtrl.Tell() < nPPEndRecPos ) )
                        {
                            DffRecordHeader aEscherObjListHd;
                            ReadDffRecordHeader( rStCtrl, aEscherObjListHd );
                            switch ( aEscherObjListHd.nRecType )
                            {
                                case DFF_msofbtSpContainer :
                                {
                                    Rectangle aPageSize( Point(), pRet->GetSize() );
                                    if ( rSlidePersist.aSlideAtom.nFlags & 4 )          // follow master background?
                                    {
                                        if ( HasMasterPage( nAktPageNum, eAktPageKind ) )
                                        {
                                            sal_uInt16 nMasterNum = GetMasterPageIndex( nAktPageNum, eAktPageKind );
                                            PptSlidePersistList* pPageList = GetPageList( PPT_MASTERPAGE );
                                            PptSlidePersistEntry* pE = &(*pPageList)[ nMasterNum ];
                                            while( ( pE->aSlideAtom.nFlags & 4 ) && pE->aSlideAtom.nMasterId )
                                            {
                                                auto nOrigMasterId = pE->aSlideAtom.nMasterId;
                                                sal_uInt16 nNextMaster = m_pMasterPages->FindPage(nOrigMasterId);
                                                if ( nNextMaster == PPTSLIDEPERSIST_ENTRY_NOTFOUND )
                                                    break;
                                                else
                                                    pE = &(*pPageList)[ nNextMaster ];
                                                if (pE->aSlideAtom.nMasterId == nOrigMasterId)
                                                {
                                                    SAL_WARN("filter.ms", "loop in atom chain");
                                                    break;
                                                }
                                            }
                                            if ( pE->nBackgroundOffset )
                                            {
                                                // do not follow master colorscheme?
                                                bool bTemporary = ( rSlidePersist.aSlideAtom.nFlags & 2 ) != 0;
                                                sal_uInt32 nPos = rStCtrl.Tell();
                                                rStCtrl.Seek( pE->nBackgroundOffset );
                                                rSlidePersist.pBObj = ImportObj( rStCtrl, static_cast<void*>(&aProcessData), aPageSize, aPageSize );
                                                rSlidePersist.bBObjIsTemporary = bTemporary;
                                                rStCtrl.Seek( nPos );
                                            }
                                        }
                                    }
                                    else
                                    {
                                        DffRecordHeader aShapeHd;
                                        ReadDffRecordHeader( rStCtrl, aShapeHd );
                                        if ( aShapeHd.nRecType == DFF_msofbtSp )
                                        {
                                            sal_uInt32 nSpFlags;
                                            rStCtrl.ReadUInt32( nSpFlags ).ReadUInt32( nSpFlags );
                                            if ( nSpFlags & SP_FBACKGROUND )
                                            {
                                                aEscherObjListHd.SeekToBegOfRecord( rStCtrl );
                                                rSlidePersist.pBObj = ImportObj( rStCtrl, static_cast<void*>(&aProcessData), aPageSize, aPageSize );
                                                rSlidePersist.bBObjIsTemporary = false;
                                            }
                                        }
                                    }
                                }
                                break;
                            }
                            if ( aEscherObjListHd.nRecType == DFF_msofbtSpContainer )
                                break;
                            if (!aEscherObjListHd.SeekToEndOfRecord(rStCtrl))
                                break;
                        }

                        // now importing page
                        rStCtrl.Seek( nPPDrawOfs );
                        auto nHdEndRecPos = SanitizeEndPos(rStCtrl, aPPDrawHd.GetRecEndFilePos());
                        while ( ( rStCtrl.GetError() == 0 ) && ( rStCtrl.Tell() < nHdEndRecPos ) )
                        {
                            DffRecordHeader aEscherObjListHd;
                            ReadDffRecordHeader( rStCtrl, aEscherObjListHd );
                            switch ( aEscherObjListHd.nRecType )
                            {
                                case DFF_msofbtSpgrContainer :
                                {
                                    DffRecordHeader aShapeHd;
                                    if ( SeekToRec( rStCtrl, DFF_msofbtSpContainer, aEscherObjListHd.GetRecEndFilePos(), &aShapeHd ) )
                                    {
                                        aShapeHd.SeekToEndOfRecord( rStCtrl );
                                        auto nListEndRecPos = SanitizeEndPos(rStCtrl, aEscherObjListHd.GetRecEndFilePos());
                                        while ( ( rStCtrl.GetError() == 0 ) && ( rStCtrl.Tell() < nListEndRecPos ) )
                                        {
                                            ReadDffRecordHeader( rStCtrl, aShapeHd );
                                            if ( ( aShapeHd.nRecType == DFF_msofbtSpContainer ) || ( aShapeHd.nRecType == DFF_msofbtSpgrContainer ) )
                                            {
                                                Rectangle aEmpty;
                                                aShapeHd.SeekToBegOfRecord( rStCtrl );
                                                sal_Int32 nShapeId;
                                                aProcessData.pTableRowProperties.reset();
                                                SdrObject* pObj = ImportObj( rStCtrl, static_cast<void*>(&aProcessData), aEmpty, aEmpty, 0, &nShapeId );
                                                if ( pObj )
                                                {
                                                    if ( aProcessData.pTableRowProperties )
                                                        pObj = CreateTable( pObj, aProcessData.pTableRowProperties.get(), aProcessData.rPersistEntry.pSolverContainer );

                                                    pRet->NbcInsertObject( pObj );

                                                    if( nShapeId )
                                                        insertShapeId( nShapeId, pObj );
                                                }
                                            }
                                            bool bSuccess = aShapeHd.SeekToEndOfRecord(rStCtrl);
                                            if (!bSuccess)
                                                break;
                                        }
                                    }
                                }
                                break;
                            }
                            if ( aEscherObjListHd.nRecType == DFF_msofbtSpgrContainer )
                                break;
                            if (!aEscherObjListHd.SeekToEndOfRecord(rStCtrl))
                                break;
                        }

                        // Handle shapes where the fill matches the background
                        // fill (mso_fillBackground).
                        if (rSlidePersist.ePageKind == PPT_SLIDEPAGE)
                        {
                            if (!aProcessData.aBackgroundColoredObjects.empty())
                            {
                                if (!rSlidePersist.pBObj)
                                {
                                    for (auto pObject : aProcessData.aBackgroundColoredObjects)
                                    {
                                        // The shape wants a background, but the slide doesn't have
                                        // one: default to white.
                                        SfxItemSet aNewSet(*pObject->GetMergedItemSet().GetPool());
                                        aNewSet.Put(XFillStyleItem(css::drawing::FillStyle_SOLID));
                                        aNewSet.Put(XFillColorItem(OUString(), Color(COL_WHITE)));
                                        pObject->SetMergedItemSet(aNewSet);
                                    }
                                }
                            }
                        }

                        if ( rSlidePersist.pBObj )
                        {
                            // #i99386# transfer the attributes from the temporary BackgroundObject
                            // to the Page and delete it. Maybe rSlidePersist.bBObjIsTemporary is
                            // obsolete here, too.
                            pRet->getSdrPageProperties().ClearItem();
                            pRet->getSdrPageProperties().PutItemSet(rSlidePersist.pBObj->GetMergedItemSet());
                            SdrObject::Free( rSlidePersist.pBObj );
                        }
                    }
                }
                break;
            }
            if (!aHd.SeekToEndOfRecord(rStCtrl))
                break;
        }
        if ( rSlidePersist.pSolverContainer )
            SolveSolver( *rSlidePersist.pSolverContainer );
    }
    rStCtrl.Seek( nMerk );
}

const PptSlideLayoutAtom* SdrPowerPointImport::GetSlideLayoutAtom() const
{
    PptSlidePersistList* pPageList = GetPageList( eAktPageKind );
    if ( pPageList && nAktPageNum < pPageList->size() )
    {
        assert( !pPageList->is_null( nAktPageNum ) );
        return &(*pPageList)[ nAktPageNum ].aSlideAtom.aLayout;
    }
    return nullptr;
}

bool SdrPowerPointImport::IsNoteOrHandout( sal_uInt16 nPageNum, PptPageKind /*ePageKind*/) const
{
    bool bNote = eAktPageKind == PPT_NOTEPAGE;
    if ( eAktPageKind == PPT_MASTERPAGE )
        bNote = ( nPageNum & 1 ) == 0;
    return bNote;
}

sal_uInt32 SdrPowerPointImport::GetMasterPageId( sal_uInt16 nPageNum, PptPageKind ePageKind ) const
{
    PptSlidePersistList* pPageList = GetPageList( ePageKind );
    if ( pPageList && nPageNum < pPageList->size() )
        return (*pPageList)[ nPageNum ].aSlideAtom.nMasterId;
   return 0;
}

sal_uInt32 SdrPowerPointImport::GetNotesPageId( sal_uInt16 nPageNum ) const
{
    PptSlidePersistList* pPageList=GetPageList( PPT_SLIDEPAGE );
    if ( pPageList && nPageNum < pPageList->size() )
        return (*pPageList)[ nPageNum ].aSlideAtom.nNotesId;
   return 0;
}

bool SdrPowerPointImport::HasMasterPage( sal_uInt16 nPageNum, PptPageKind ePageKind ) const
{
    if ( ePageKind == PPT_NOTEPAGE )
        return aDocAtom.nNotesMasterPersist != 0;
    if ( ePageKind == PPT_MASTERPAGE )
        return false;
    return GetMasterPageId( nPageNum, ePageKind ) != 0;
}

sal_uInt16 SdrPowerPointImport::GetMasterPageIndex( sal_uInt16 nPageNum, PptPageKind ePageKind ) const
{
    sal_uInt16 nIdx = 0;
    if ( ePageKind == PPT_NOTEPAGE )
        return 2;
    sal_uInt32 nId = GetMasterPageId( nPageNum, ePageKind );
    if (nId && m_pMasterPages)
    {
        nIdx = m_pMasterPages->FindPage( nId );
        if ( nIdx == PPTSLIDEPERSIST_ENTRY_NOTFOUND )
            nIdx = 0;
    }
    return nIdx;
}

SdrObject* SdrPowerPointImport::ImportPageBackgroundObject( const SdrPage& rPage, sal_uInt32& nBgFileOffset )
{
    SdrObject* pRet = nullptr;
    std::unique_ptr<SfxItemSet> pSet;
    sal_uLong nFPosMerk = rStCtrl.Tell(); // remember FilePos for restoring it later
    DffRecordHeader aPageHd;
    if ( SeekToAktPage( &aPageHd ) )
    {   // and now search for the background attributes of the Page
        sal_uLong nPageRecEnd = aPageHd.GetRecEndFilePos();
        DffRecordHeader aPPDrawHd;
        if ( SeekToRec( rStCtrl, PPT_PST_PPDrawing, nPageRecEnd, &aPPDrawHd ) )
        {
            sal_uLong nPPDrawEnd = aPPDrawHd.GetRecEndFilePos();
            DffRecordHeader aEscherF002Hd;
            if ( SeekToRec( rStCtrl, DFF_msofbtDgContainer, nPPDrawEnd, &aEscherF002Hd ) )
            {
                sal_uLong nEscherF002End = aEscherF002Hd.GetRecEndFilePos();
                DffRecordHeader aEscherObjectHd;
                if ( SeekToRec( rStCtrl, DFF_msofbtSpContainer, nEscherF002End, &aEscherObjectHd ) )
                {
                    nBgFileOffset = aEscherObjectHd.GetRecBegFilePos();
                    //sal_uLong nEscherObjectEnd = aEscherObjectHd.GetRecEndFilePos();
                    //DffRecordHeader aEscherPropertiesHd;
                    if ( SeekToRec( rStCtrl, DFF_msofbtOPT,nEscherF002End ) )
                    {
                        ReadDffPropSet( rStCtrl, (DffPropertyReader&)*this );
                        mnFix16Angle = Fix16ToAngle( GetPropertyValue( DFF_Prop_Rotation, 0 ) );
                        sal_uInt32 nColor = GetPropertyValue( DFF_Prop_fillColor, 0xffffff );
                        pSet.reset(new SfxItemSet( pSdrModel->GetItemPool() ));
                        DffObjData aObjData( aEscherObjectHd, Rectangle( 0, 0, 28000, 21000 ), 0 );
                        ApplyAttributes( rStCtrl, *pSet, aObjData );
                        Color aColor( MSO_CLR_ToColor( nColor ) );
                        pSet->Put( XFillColorItem( OUString(), aColor ) );
                    }
                }
            }
        }
    }
    rStCtrl.Seek( nFPosMerk ); // restore FilePos
    if ( !pSet )
    {
        pSet.reset(new SfxItemSet( pSdrModel->GetItemPool() ));
        pSet->Put( XFillStyleItem( drawing::FillStyle_NONE ) );
    }
    pSet->Put( XLineStyleItem( drawing::LineStyle_NONE ) );
    Rectangle aRect( rPage.GetLftBorder(), rPage.GetUppBorder(), rPage.GetWdt()-rPage.GetRgtBorder(), rPage.GetHgt()-rPage.GetLwrBorder() );
    pRet = new SdrRectObj( aRect );
    pRet->SetModel( pSdrModel );

    pRet->SetMergedItemSet(*pSet);

    pRet->SetMarkProtect( true );
    pRet->SetMoveProtect( true );
    pRet->SetResizeProtect( true );
    return pRet;
}

HeaderFooterEntry::HeaderFooterEntry( const PptSlidePersistEntry* pMPE ) :
    pMasterPersist  ( pMPE ),
    nAtom           ( 0 )
{
    if ( pMPE )
    {
        HeaderFooterEntry* pMHFE = pMPE->pHeaderFooterEntry;
        if ( pMHFE )
        {
            nAtom = pMPE->pHeaderFooterEntry->nAtom;
            pPlaceholder[ 0 ] = pMHFE->pPlaceholder[ 0 ];
            pPlaceholder[ 1 ] = pMHFE->pPlaceholder[ 1 ];
            pPlaceholder[ 2 ] = pMHFE->pPlaceholder[ 2 ];
            pPlaceholder[ 3 ] = pMHFE->pPlaceholder[ 3 ];
        }
    }
}

HeaderFooterEntry::~HeaderFooterEntry()
{
}


sal_uInt32 HeaderFooterEntry::IsToDisplay( sal_uInt32 nInstance )
{
    sal_uInt32 nMask = 0;
    switch ( nInstance )
    {
        case 0 : nMask = 0x010000; break;
        case 1 : nMask = 0x100000; break;
        case 2 : nMask = 0x200000; break;
        case 3 : nMask = 0x080000; break;
    }
    return ( nAtom & nMask );
}

// The following method checks if the slide is using a different colorscheme than
// its master, if this is the fact, then the HeaderFooter must probably be
// imported as real sdrobject. In this case, the return value is the offset to the
// master header footer object, so it can be re-loaded with a different color set
sal_uInt32 HeaderFooterEntry::NeedToImportInstance( const sal_uInt32 nInstance, const PptSlidePersistEntry& rSlidePersist )
{
    sal_uInt32 nRet = 0;
    if ( pMasterPersist )
    {
        if ( !( rSlidePersist.aSlideAtom.nFlags & 2 ) )
        {   // not following the master persist, so we have to check if the colors are changed
            if ( memcmp( &rSlidePersist.aColorScheme, &pMasterPersist->aColorScheme, 32 ) )
            {
                nRet = pMasterPersist->HeaderFooterOfs[ nInstance ];
            }
        }
    }
    return nRet;
}

void SdrEscherImport::ImportHeaderFooterContainer( DffRecordHeader& rHd, HeaderFooterEntry& rE )
{
    rHd.SeekToContent( rStCtrl );
    auto nEndRecPos = SanitizeEndPos(rStCtrl, rHd.GetRecEndFilePos());
    while ( ( rStCtrl.GetError() == 0 ) && ( rStCtrl.Tell() < nEndRecPos ) )
    {
        DffRecordHeader aHd;
        ReadDffRecordHeader( rStCtrl, aHd );
        switch ( aHd.nRecType )
        {
            case PPT_PST_HeadersFootersAtom :
                rStCtrl.ReadUInt32( rE.nAtom );
            break;

            case PPT_PST_CString :
            {
                if ( aHd.nRecInstance < 4 )
                {
                    rE.pPlaceholder[ aHd.nRecInstance ] = MSDFFReadZString( rStCtrl,
                        aHd.nRecLen, true );
                }
            }
            break;
        }
        if (!aHd.SeekToEndOfRecord(rStCtrl))
            break;
    }
}

PPTBuGraEntry::PPTBuGraEntry( Graphic& rGraphic, sal_uInt32 nInst ) :
    nInstance       ( nInst ),
    aBuGra          ( rGraphic )  {}

PPTExtParaLevel::PPTExtParaLevel()
: mnExtParagraphMask( 0 )
, mnBuBlip( 0xffff )
, mnHasAnm( 0 )
, mnAnmScheme( 0 )
, mpfPP10Ext( 0 )
, mnExtCharacterMask( 0 )
, mcfPP10Ext( 0 )
, mbSet( false )
{}

SvStream& ReadPPTExtParaLevel( SvStream& rIn, PPTExtParaLevel& rLevel )
{
    rLevel.mbSet = true;
    rIn.ReadUInt32( rLevel.mnExtParagraphMask );
    if ( rLevel.mnExtParagraphMask & 0x00800000 )
        rIn.ReadUInt16( rLevel.mnBuBlip );
    if ( rLevel.mnExtParagraphMask & 0x02000000 )
        rIn.ReadUInt16( rLevel.mnHasAnm );
    if ( rLevel.mnExtParagraphMask & 0x01000000 )
        rIn.ReadUInt32( rLevel.mnAnmScheme );
    if ( rLevel.mnExtParagraphMask & 0x04000000 )
        rIn.ReadUInt32( rLevel.mpfPP10Ext );
    rIn.ReadUInt32( rLevel.mnExtCharacterMask );
    if ( rLevel.mnExtCharacterMask & 0x100000 )
        rIn.ReadUInt32( rLevel.mcfPP10Ext );
    return rIn;
}

bool PPTExtParaProv::GetGraphic( sal_uInt32 nInstance, Graphic& rGraph ) const
{
    bool bRetValue = false;
    PPTBuGraEntry* pPtr = nullptr;
    if ( nInstance < aBuGraList.size() )
    {
        pPtr = aBuGraList[ nInstance ];
        if ( pPtr->nInstance == nInstance )
            bRetValue = true;
    }
    if ( !bRetValue )
    {
        for (PPTBuGraEntry* i : aBuGraList)
        {
            pPtr = i;
            if ( pPtr->nInstance == nInstance )
            {
                bRetValue = true;
                break;
            }
        }
    }
    if ( bRetValue )
        rGraph = pPtr->aBuGra;
    return bRetValue;
}

PPTExtParaProv::PPTExtParaProv( SdrPowerPointImport& rMan, SvStream& rSt, const DffRecordHeader* pHd ) :
    bStyles         ( false ),
    bGraphics       ( false )
{
    sal_uInt32 nOldPos = rSt.Tell();

    // here we have to get the graphical bullets...

    DffRecordHeader aHd;
    DffRecordHeader aContentDataHd;

    const DffRecordHeader* pListHd = rMan.aDocRecManager.GetRecordHeader( PPT_PST_List );
    if( pListHd )
        pListHd->SeekToContent( rSt );
    if ( pListHd && SdrPowerPointImport::SeekToContentOfProgTag( 9, rSt, *pListHd, aContentDataHd ) )
    {
        auto nEndRecPos = DffPropSet::SanitizeEndPos(rSt, aContentDataHd.GetRecEndFilePos());
        while ( ( rSt.GetError() == 0 ) && ( rSt.Tell() < nEndRecPos ) )
        {
            ReadDffRecordHeader( rSt, aHd );
            switch ( aHd.nRecType )
            {
                case PPT_PST_ExtendedBuGraContainer :
                {
                    auto nHdEndRecPos = DffPropSet::SanitizeEndPos(rSt, aHd.GetRecEndFilePos());
                    while ( ( rSt.GetError() == 0 ) && ( rSt.Tell() < nHdEndRecPos ) )
                    {
                        sal_uInt16 nType;
                        DffRecordHeader aBuGraAtomHd;
                        ReadDffRecordHeader( rSt, aBuGraAtomHd );
                        if ( aBuGraAtomHd.nRecType == PPT_PST_ExtendedBuGraAtom )
                        {
                            rSt.ReadUInt16( nType );
                            Graphic aGraphic;
                            if ( SvxMSDffManager::GetBLIPDirect( rSt, aGraphic ) )
                            {
                                sal_uInt32 nInstance = aBuGraAtomHd.nRecInstance;
                                PPTBuGraEntry* pBuGra = new PPTBuGraEntry( aGraphic, nInstance );
                                size_t n = 0;
                                size_t nBuGraCount = aBuGraList.size();
                                if ( nBuGraCount )
                                {
                                    if ( aBuGraList[ nBuGraCount - 1 ]->nInstance < nInstance )
                                        n = nBuGraCount;
                                    else
                                    {   // maybe the instances are not sorted, we sort it
                                        for ( n = 0; n < nBuGraCount; n++ )
                                        {   // sorting fields ( hi >> lo )
                                            if ( aBuGraList[ n ]->nInstance < nInstance )
                                                break;
                                        }
                                    }
                                }
                                if ( n < nBuGraCount ) {
                                    aBuGraList.insert( aBuGraList.begin() + n, pBuGra );
                                } else {
                                    aBuGraList.push_back( pBuGra );
                                }
                            }
#ifdef DBG_UTIL
                            else OSL_FAIL( "PPTExParaProv::PPTExParaProv - bullet graphic is not valid (SJ)" );
#endif
                        }
#ifdef DBG_UTIL
                        else OSL_FAIL( "PPTExParaProv::PPTExParaProv - unknown atom interpreting the PPT_PST_ExtendedBuGraContainer (SJ)" );
#endif
                        if (!aBuGraAtomHd.SeekToEndOfRecord(rSt))
                            break;
                    }
                    if ( !aBuGraList.empty() )
                        bGraphics = true;
                }
                break;

                case PPT_PST_ExtendedPresRuleContainer :
                    aExtendedPresRules.Consume( rSt, aHd.GetRecEndFilePos() );
                break;
#ifdef DBG_UTIL
                default :
                    OSL_FAIL( "PPTExParaProv::PPTExParaProv - unknown atom reading ppt2000 num rules (SJ)" );
                break;
                case PPT_PST_MasterText :   // first seen in: ms-tt02.ppt
                case PPT_PST_SrKinsoku :
                case PPT_PST_TextDefaults9Atom :
                case PPT_PST_PresentationAdvisorFlags9Atom :
                case PPT_PST_HtmlDocInfo9Atom :
                case PPT_PST_GridSpacing10Atom :
                case PPT_PST_CommentIndex10 :
                case PPT_PST_DocToolbarStates10Atom :
                break;
#endif
            }
            if (!aHd.SeekToEndOfRecord(rSt))
                break;
        }
    }

    if ( pHd && SdrPowerPointImport::SeekToContentOfProgTag( 9, rSt, *pHd, aContentDataHd ) )
    {   // get the extended paragraph styles on mainmaster ( graphical bullets, num ruling ... )
        auto nEndRecPos = DffPropSet::SanitizeEndPos(rSt, aContentDataHd.GetRecEndFilePos());
        while ( ( rSt.GetError() == 0 ) && ( rSt.Tell() < nEndRecPos ) )
        {
            ReadDffRecordHeader( rSt, aHd );
            switch ( aHd.nRecType )
            {
                case PPT_PST_ExtendedParagraphMasterAtom :
                {
                    if ( aHd.nRecInstance < PPT_STYLESHEETENTRYS )
                    {
                        sal_uInt16 nDepth = 0, i = 0;
                        rSt.ReadUInt16( nDepth );
                        if ( i <= 5 )
                        {
                            auto nHdEndRecPos = DffPropSet::SanitizeEndPos(rSt, aHd.GetRecEndFilePos());
                            while ( ( rSt.GetError() == 0 ) && ( rSt.Tell() < nHdEndRecPos ) && ( i < nDepth ) )
                            {
                                bStyles = true;
                                ReadPPTExtParaLevel( rSt, aExtParaSheet[ (TSS_Type)aHd.nRecInstance ].aExtParaLevel[ i++ ] );
                            }
#ifdef DBG_UTIL
                            if ( rSt.Tell() != aHd.GetRecEndFilePos() )
                                OSL_FAIL( "PPTExParaProv::PPTExParaProv - error reading PPT_PST_ExtendedParagraphMasterAtom (SJ)" );
#endif
                        }
#ifdef DBG_UTIL
                        else OSL_FAIL( "PPTExParaProv::PPTExParaProv - depth is greater than 5 (SJ)" );
#endif
                    }
#ifdef DBG_UTIL
                    else OSL_FAIL( "PPTExParaProv::PPTExParaProv - instance out of range (SJ)" );
#endif
                }
                break;
                default :
                    OSL_FAIL( "PPTExParaProv::PPTExParaProv - unknown atom, assuming PPT_PST_ExtendedParagraphMasterAtom (SJ)" );
                break;
                case PPT_PST_HashCodeAtom :
                case PPT_PST_BuildList :
                case PPT_PST_SlideFlags10Atom :
                case PPT_PST_SlideTime10Atom :
                case 0xf144 :
                break;
            }
            if (!aHd.SeekToEndOfRecord(rSt))
                break;
        }
    }
    rSt.Seek( nOldPos );
}

PPTExtParaProv::~PPTExtParaProv()
{
    for (PPTBuGraEntry* i : aBuGraList)
        delete i;
    aBuGraList.clear();
}

PPTNumberFormatCreator::PPTNumberFormatCreator( PPTExtParaProv* pParaProv )
    : nIsBullet(0)
    , nBulletChar(0)
    , nBulletFont(0)
    , nBulletHeight(0)
    , nBulletColor(0)
    , nTextOfs(0)
    , nBulletOfs(0)
    , pExtParaProv(pParaProv)
{
}

PPTNumberFormatCreator::~PPTNumberFormatCreator()
{
}

bool PPTNumberFormatCreator::ImplGetExtNumberFormat( SdrPowerPointImport& rManager,
    SvxNumberFormat& rNumberFormat, sal_uInt32 nLevel, TSS_Type nInstance, TSS_Type nDestinationInstance,
        boost::optional< sal_Int16 >& rStartNumbering, sal_uInt32 nFontHeight,  PPTParagraphObj* pPara )
{
    bool bHardAttribute = ( nDestinationInstance == TSS_Type::Unknown );

    sal_uInt32  nBuFlags = 0;
    sal_uInt16  nHasAnm = 0;
    sal_uInt32  nAnmScheme = 0xFFFF0003;
    sal_uInt16  nBuBlip = 0xffff;

    const PPTExtParaProv* pParaProv = pExtParaProv.get();
    if ( !pExtParaProv )
        pParaProv = pPara ? pPara->mrStyleSheet.pExtParaProv.get()
                          : rManager.pPPTStyleSheet->pExtParaProv.get();
    if ( pPara )
    {
        nBuFlags = pPara->pParaSet->mnExtParagraphMask;
        if ( nBuFlags )
        {
            if ( nBuFlags & 0x00800000 )
                nBuBlip = pPara->pParaSet->mnBuBlip;
            if ( nBuFlags & 0x01000000 )
                nAnmScheme = pPara->pParaSet->mnAnmScheme;
            if ( nBuFlags & 0x02000000 )
                nHasAnm = pPara->pParaSet->mnHasAnm;
            bHardAttribute = true;
        }
    }

    if ( ( nBuFlags & 0x03800000 ) != 0x03800000 )  // merge style sheet
    {   // we have to read the master attributes
        if ( pParaProv && ( nLevel < 5 ) )
        {
            if ( pParaProv->bStyles )
            {
                const PPTExtParaLevel& rLev = pParaProv->aExtParaSheet[ nInstance ].aExtParaLevel[ nLevel ];
                if ( rLev.mbSet )
                {
                    sal_uInt32 nMaBuFlags = rLev.mnExtParagraphMask;

                    if ( (!( nBuFlags & 0x00800000)) && ( nMaBuFlags & 0x00800000 ) )
                    {
                        if (!( nBuFlags & 0x02000000))          // if there is a BuStart without BuInstance,
                            nBuBlip = rLev.mnBuBlip;            // then there is no graphical Bullet possible
                    }
                    if ( (!( nBuFlags & 0x01000000)) && ( nMaBuFlags & 0x01000000 ) )
                        nAnmScheme = rLev.mnAnmScheme;
                    if ( (!( nBuFlags & 0x02000000)) && ( nMaBuFlags & 0x02000000 ) )
                        nHasAnm = rLev.mnHasAnm;
                    nBuFlags |= nMaBuFlags;
                }
            }
        }
    }
    if ( nBuBlip != 0xffff )        // set graphical bullet
    {
        Graphic aGraphic;
        if ( pParaProv && pParaProv->GetGraphic( nBuBlip, aGraphic ) )
        {
            SvxBrushItem aBrush( aGraphic, GPOS_MM, SID_ATTR_BRUSH );
            rNumberFormat.SetGraphicBrush( &aBrush );
            sal_uInt32 nHeight = (sal_uInt32)( (double)nFontHeight * 0.2540 * nBulletHeight + 0.5 );
            Size aPrefSize( aGraphic.GetPrefSize() );
            sal_uInt32 nWidth = ( nHeight * aPrefSize.Width() ) / aPrefSize.Height();
            rNumberFormat.SetGraphicSize( Size( nWidth, nHeight ) );
            rNumberFormat.SetNumberingType ( SVX_NUM_BITMAP );
        }
    }
    else if ( nHasAnm )
    {
        switch( static_cast< sal_uInt16 >( nAnmScheme ) )
        {
            default :
            case 0 :
            {
                rNumberFormat.SetNumberingType( SVX_NUM_CHARS_LOWER_LETTER );
                rNumberFormat.SetSuffix( "." );
            }
            break;
            case 1 :
            {
                rNumberFormat.SetNumberingType( SVX_NUM_CHARS_UPPER_LETTER );
                rNumberFormat.SetSuffix( "." );
            }
            break;
            case 2 :
            {
                rNumberFormat.SetNumberingType( SVX_NUM_ARABIC );
                rNumberFormat.SetSuffix( ")" );
            }
            break;
            case 3 :
            {
                rNumberFormat.SetNumberingType( SVX_NUM_ARABIC );
                rNumberFormat.SetSuffix( "." );
            }
            break;
            case 4 :
            {
                rNumberFormat.SetNumberingType( SVX_NUM_ROMAN_LOWER );
                rNumberFormat.SetSuffix( ")" );
                rNumberFormat.SetPrefix( "(" );
            }
            break;
            case 5 :
            {
                rNumberFormat.SetNumberingType( SVX_NUM_ROMAN_LOWER );
                rNumberFormat.SetSuffix( ")" );
            }
            break;
            case 6 :
            {
                rNumberFormat.SetNumberingType( SVX_NUM_ROMAN_LOWER );
                rNumberFormat.SetSuffix( "." );
            }
            break;
            case 7 :
            {
                rNumberFormat.SetNumberingType( SVX_NUM_ROMAN_UPPER );
                rNumberFormat.SetSuffix( "." );
            }
            break;
            case 8 :
            {
                rNumberFormat.SetNumberingType( SVX_NUM_CHARS_LOWER_LETTER );
                rNumberFormat.SetSuffix( ")" );
                rNumberFormat.SetPrefix( "(" );
            }
            break;
            case 9 :
            {
                rNumberFormat.SetNumberingType( SVX_NUM_CHARS_LOWER_LETTER );
                rNumberFormat.SetSuffix( ")" );
            }
            break;
            case 10 :
            {
                rNumberFormat.SetNumberingType( SVX_NUM_CHARS_UPPER_LETTER );
                rNumberFormat.SetSuffix( ")" );
                rNumberFormat.SetPrefix( "(" );
            }
            break;
            case 11 :
            {
                rNumberFormat.SetNumberingType( SVX_NUM_CHARS_UPPER_LETTER );
                rNumberFormat.SetSuffix( ")" );
            }
            break;
            case 12 :
            {
                rNumberFormat.SetNumberingType( SVX_NUM_ARABIC );
                rNumberFormat.SetSuffix( ")" );
                rNumberFormat.SetPrefix( "(" );
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
                rNumberFormat.SetSuffix( ")" );
                rNumberFormat.SetPrefix( "(" );
            }
            break;
            case 15 :
            {
                rNumberFormat.SetNumberingType( SVX_NUM_ROMAN_UPPER );
                rNumberFormat.SetSuffix( ")" );
            }
            break;
            case 16: // Simplified Chinese.
            {
                rNumberFormat.SetNumberingType( SVX_NUM_NUMBER_UPPER_ZH );
            }
            break;
            case 17: // Simplified Chinese with single-byte period.
            {
                rNumberFormat.SetNumberingType( SVX_NUM_NUMBER_UPPER_ZH );
                rNumberFormat.SetSuffix( "." );
            }
            break;
            case 18: // Double byte circle numbers.
            case 19: // Wingdings white circle numbers.
            case 20: // Wingdings black circle numbers.
            {
                rNumberFormat.SetNumberingType( SVX_NUM_CIRCLE_NUMBER );
            }
            break;
            case 21: // Traditional Chinese.
            {
                rNumberFormat.SetNumberingType( SVX_NUM_NUMBER_UPPER_ZH_TW );
            }
            break;
            case 22: // Traditional Chinese with single-byte period.
            {
                rNumberFormat.SetNumberingType( SVX_NUM_NUMBER_UPPER_ZH_TW );
                rNumberFormat.SetSuffix( "." );
            }
            break;
            case 26: // Japanese/Korean.
            {
                rNumberFormat.SetNumberingType( SVX_NUM_NUMBER_LOWER_ZH );
            }
            break;
            case 27: // Japanese/Korean with single-byte period.
            {
                rNumberFormat.SetNumberingType( SVX_NUM_NUMBER_LOWER_ZH );
                rNumberFormat.SetSuffix( "." );
            }
            break;
            case 28: // Double-byte Arabic numbers.
            {
                rNumberFormat.SetNumberingType( SVX_NUM_FULL_WIDTH_ARABIC );
            }
            break;
            case 29: // Double-byte Arabic numbers with double-byte period.
            {
                rNumberFormat.SetNumberingType( SVX_NUM_FULL_WIDTH_ARABIC );
                rNumberFormat.SetSuffix( OUString( sal_Unicode(0xff0e) ) );
            }
            break;
            case 38: // Japanese with double-byte period.
            {
                rNumberFormat.SetNumberingType( SVX_NUM_NUMBER_LOWER_ZH ); // No such type. Instead with Lower Chinese Number
                rNumberFormat.SetSuffix( OUString( sal_Unicode(0xff0e) ) );
            }
            break;
        }
        rStartNumbering = boost::optional< sal_Int16 >( nAnmScheme >> 16 );
        sal_Int16 nBuStart = *rStartNumbering;
        //The Seventh bit of nBuFlags that specifies whether fBulletHasAutoNumber exists,
        //and fBulletHasAutoNumber that specifies whether this paragraph has an automatic numbering scheme.
        if ( ( nBuFlags & 0x02000000 ) && ( nBuStart != -1 ))
        {
            rNumberFormat.SetStart( static_cast<sal_uInt16>(nBuStart) );
        }
    }
    return bHardAttribute;
}

void PPTNumberFormatCreator::GetNumberFormat( SdrPowerPointImport& rManager, SvxNumberFormat& rNumberFormat, sal_uInt32 nLevel, const PPTParaLevel& rParaLevel, const PPTCharLevel& rCharLevel, TSS_Type nInstance )
{
    nIsBullet = ( rParaLevel.mnBuFlags & ( 1 << PPT_ParaAttr_BulletOn ) ) != 0 ? 1 : 0;
    nBulletChar = rParaLevel.mnBulletChar;

    bool bBuHardFont;
    bBuHardFont = ( rParaLevel.mnBuFlags & ( 1 << PPT_ParaAttr_BuHardFont ) ) != 0;
    if ( bBuHardFont )
        nBulletFont = rParaLevel.mnBulletFont;
    else
        nBulletFont = rCharLevel.mnFont;
    nBulletHeight = rParaLevel.mnBulletHeight;
    nBulletColor = rParaLevel.mnBulletColor;
    nTextOfs = rParaLevel.mnTextOfs;
    nBulletOfs = rParaLevel.mnBulletOfs;

    boost::optional< sal_Int16 > oStartNumbering;
    ImplGetExtNumberFormat( rManager, rNumberFormat, nLevel, nInstance, TSS_Type::Unknown, oStartNumbering, rCharLevel.mnFontHeight, nullptr );
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
                vcl::Font aFont;
                aFont.SetCharSet( pFontEnityAtom->eCharSet );
                aFont.SetFamilyName( pFontEnityAtom->aName );
                aFont.SetFamily( pFontEnityAtom->eFamily );
                aFont.SetPitch( pFontEnityAtom->ePitch );
                rNumberFormat.SetBulletFont( &aFont );
            }
        }
        break;
    }
}

bool PPTNumberFormatCreator::GetNumberFormat( SdrPowerPointImport& rManager, SvxNumberFormat& rNumberFormat, PPTParagraphObj* pParaObj,
                                                TSS_Type nDestinationInstance, boost::optional< sal_Int16 >& rStartNumbering )
{
    sal_uInt32 nHardCount = 0;
    nHardCount += pParaObj->GetAttrib( PPT_ParaAttr_BulletOn, nIsBullet, nDestinationInstance ) ? 1 : 0;
    nHardCount += pParaObj->GetAttrib( PPT_ParaAttr_BulletChar, nBulletChar, nDestinationInstance ) ? 1 : 0;
    nHardCount += pParaObj->GetAttrib( PPT_ParaAttr_BulletFont, nBulletFont, nDestinationInstance ) ? 1 : 0;
    nHardCount += pParaObj->GetAttrib( PPT_ParaAttr_BulletHeight, nBulletHeight, nDestinationInstance ) ? 1 : 0;
    nHardCount += pParaObj->GetAttrib( PPT_ParaAttr_BulletColor, nBulletColor, nDestinationInstance ) ? 1 : 0;
    nHardCount += pParaObj->GetAttrib( PPT_ParaAttr_TextOfs, nTextOfs, nDestinationInstance ) ? 1 : 0;
    nHardCount += pParaObj->GetAttrib( PPT_ParaAttr_BulletOfs, nBulletOfs, nDestinationInstance ) ? 1 : 0;

    if ( nIsBullet )
        rNumberFormat.SetNumberingType( SVX_NUM_CHAR_SPECIAL );

    sal_uInt32 nFontHeight = 24;
    PPTPortionObj* pPtr = pParaObj->First();
    if ( pPtr )
        pPtr->GetAttrib( PPT_CharAttr_FontHeight, nFontHeight, nDestinationInstance );
    if ( nIsBullet )
        nHardCount += ImplGetExtNumberFormat( rManager, rNumberFormat, pParaObj->pParaSet->mnDepth,
                                                    pParaObj->mnInstance, nDestinationInstance, rStartNumbering, nFontHeight, pParaObj ) ? 1 : 0;

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
                    pPtr->GetAttrib( PPT_CharAttr_Font, nFont, nDestinationInstance );
                    PptFontEntityAtom* pFontEnityAtom = rManager.GetFontEnityAtom( nFont );
                    if ( pFontEnityAtom )
                    {
                        vcl::Font aFont;
                        aFont.SetCharSet( pFontEnityAtom->eCharSet );
                        aFont.SetFamilyName( pFontEnityAtom->aName );
                        aFont.SetFamily( pFontEnityAtom->eFamily );
                        aFont.SetPitch( pFontEnityAtom->ePitch );
                        rNumberFormat.SetBulletFont( &aFont );
                    }
                }
            }
            break;
        }
    }
    return nHardCount != 0;
}

void PPTNumberFormatCreator::ImplGetNumberFormat( SdrPowerPointImport& rManager, SvxNumberFormat& rNumberFormat, sal_uInt32 /*nLevel*/)
{
    vcl::Font aFont;
    PptFontEntityAtom* pAtom = rManager.GetFontEnityAtom( nBulletFont );
    if ( pAtom )
    {
        rtl_TextEncoding eCharSet( pAtom->eCharSet );
        aFont.SetFamilyName( pAtom->aName );
        aFont.SetCharSet( eCharSet );
        aFont.SetFamily( pAtom->eFamily );
        aFont.SetPitch( pAtom->ePitch );
    }
    Color aCol( rManager.MSO_TEXT_CLR_ToColor( nBulletColor ) );
    aFont.SetColor( aCol );

    sal_uInt16 nBuChar = (sal_uInt16)nBulletChar;
    if ( aFont.GetCharSet() == RTL_TEXTENCODING_SYMBOL )
    {
        nBuChar &= 0x00ff;
        nBuChar |= 0xf000;
    }
    rNumberFormat.SetBulletFont( &aFont );
    rNumberFormat.SetBulletChar( nBuChar );
    rNumberFormat.SetBulletRelSize( (sal_uInt16)nBulletHeight );
    rNumberFormat.SetBulletColor( aCol );
    sal_uInt16 nAbsLSpace = (sal_uInt16)( ( (sal_uInt32)nTextOfs * 2540 ) / 576 );
    sal_uInt16 nFirstLineOffset = nAbsLSpace - (sal_uInt16)( ( (sal_uInt32)nBulletOfs * 2540 ) / 576 );
    rNumberFormat.SetAbsLSpace( nAbsLSpace );
    rNumberFormat.SetFirstLineOffset( -nFirstLineOffset );
}

PPTCharSheet::PPTCharSheet( TSS_Type nInstance )
{
    sal_uInt32 nColor = PPT_COLSCHEME_TEXT_UND_ZEILEN;
    sal_uInt16 nFontHeight(0);
    switch ( nInstance )
    {
        case TSS_Type::PageTitle :
        case TSS_Type::Title :
        {
            nColor = PPT_COLSCHEME_TITELTEXT;
            nFontHeight = 44;
        }
        break;
        case TSS_Type::Body :
        case TSS_Type::Subtitle :
        case TSS_Type::HalfBody :
        case TSS_Type::QuarterBody :
            nFontHeight = 32;
        break;
        case TSS_Type::Notes :
            nFontHeight = 12;
        break;
        case TSS_Type::Unused :
        case TSS_Type::TextInShape :
            nFontHeight = 24;
        break;
        default: break;
    }
    for (PPTCharLevel & nDepth : maCharLevel)
    {
        nDepth.mnFlags = 0;
        nDepth.mnFont = 0;
        nDepth.mnAsianOrComplexFont = 0xffff;
        nDepth.mnFontHeight = nFontHeight;
        nDepth.mnFontColor = nColor;
        nDepth.mnFontColorInStyleSheet = Color( (sal_uInt8)nColor, (sal_uInt8)( nColor >> 8 ), (sal_uInt8)( nColor >> 16 ) );
        nDepth.mnEscapement = 0;
    }
}

PPTCharSheet::PPTCharSheet( const PPTCharSheet& rAttr )
{
    *this = rAttr;
}

void PPTCharSheet::Read( SvStream& rIn, bool /*bMasterStyle*/, sal_uInt32 nLevel, bool /*bFirst*/)
{
    // Zeichenattribute
    sal_uInt32 nCMask;
    sal_uInt16 nVal16;
    rIn.ReadUInt32( nCMask );

    if ( nCMask & 0x0000FFFF )
    {
        sal_uInt16 nBitAttr;
        maCharLevel[ nLevel ].mnFlags &= ~( (sal_uInt16)nCMask );
        rIn.ReadUInt16( nBitAttr ); // Bit attributes (bold, underlined, ...)
        maCharLevel[ nLevel ].mnFlags |= nBitAttr;
    }
    if ( nCMask & ( 1 << PPT_CharAttr_Font ) )                  // 0x00010000
        rIn.ReadUInt16( maCharLevel[ nLevel ].mnFont );
    if ( nCMask & ( 1 << PPT_CharAttr_AsianOrComplexFont ) )    // 0x00200000
        rIn.ReadUInt16( maCharLevel[ nLevel ].mnAsianOrComplexFont );
    if ( nCMask & ( 1 << PPT_CharAttr_ANSITypeface ) )          // 0x00400000
        rIn.ReadUInt16( nVal16 );
    if ( nCMask & ( 1 << PPT_CharAttr_Symbol ) )                // 0x00800000
        rIn.ReadUInt16( nVal16 );
    if ( nCMask & ( 1 << PPT_CharAttr_FontHeight ) )            // 0x00020000
        rIn.ReadUInt16( maCharLevel[ nLevel ].mnFontHeight );
    if ( nCMask & ( 1 << PPT_CharAttr_FontColor ) )             // 0x00040000
    {
        rIn.ReadUInt32( maCharLevel[ nLevel ].mnFontColor );
        if( ! (maCharLevel[ nLevel ].mnFontColor & 0xff000000 ) )
            maCharLevel[ nLevel ].mnFontColor = PPT_COLSCHEME_HINTERGRUND;
    }
    if ( nCMask & ( 1 << PPT_CharAttr_Escapement ) )            // 0x00080000
        rIn.ReadUInt16( maCharLevel[ nLevel ].mnEscapement );
    if ( nCMask & 0x00100000 )                                  // 0x00100000
        rIn.ReadUInt16( nVal16 );

    nCMask >>= 24;
    while( nCMask )
    {
        if ( nCMask & 1 )
        {
            OSL_FAIL( "PPTCharSheet::Read - unknown attribute, send me this document (SJ)" );
            rIn.ReadUInt16( nVal16 );
        }
        nCMask >>= 1;
    }
}

PPTParaSheet::PPTParaSheet( TSS_Type nInstance )
{
    sal_uInt16 nBuFlags = 0;
    sal_uInt32 nBulletColor = 0x8000000;
    sal_uInt16 nUpperDist = 0;

    switch ( nInstance )
    {
        case TSS_Type::PageTitle :
        case TSS_Type::Title :
            nBulletColor = PPT_COLSCHEME_TITELTEXT;
        break;
        case TSS_Type::Body :
        case TSS_Type::Subtitle :
        case TSS_Type::HalfBody :
        case TSS_Type::QuarterBody :
        {
            nBuFlags = 1;
            nUpperDist = 0x14;
        }
        break;
        case TSS_Type::Notes :
            nUpperDist = 0x1e;
        break;
        default: break;
    }
    for (PPTParaLevel & i : maParaLevel)
    {
        i.mnBuFlags = nBuFlags;
        i.mnBulletChar = 0x2022;
        i.mnBulletFont = 0;
        i.mnBulletHeight = 100;
        i.mnBulletColor = nBulletColor;
        i.mnAdjust = 0;
        i.mnLineFeed = 100;
        i.mnLowerDist = 0;
        i.mnUpperDist = nUpperDist;
        i.mnTextOfs = 0;
        i.mnBulletOfs = 0;
        i.mnDefaultTab = 0x240;
        i.mnAsianLineBreak = 0;
        i.mnBiDi = 0;
    }
}

PPTParaSheet::PPTParaSheet( const PPTParaSheet& rSheet )
{
    *this = rSheet;
}

void PPTParaSheet::Read( SdrPowerPointImport&
#ifdef DBG_UTIL
                    rManager
#endif
                    , SvStream& rIn, bool /*bMasterStyle*/,
                    sal_uInt32 nLevel, bool bFirst )
{
    // Absatzattribute
    sal_uInt16  nVal16, i, nMask16;
    sal_uInt32  nVal32, nPMask;
    rIn.ReadUInt32( nPMask );

    nMask16 = (sal_uInt16)nPMask & 0xf;
    if ( nMask16 )
    {
        rIn.ReadUInt16( nVal16 );
        maParaLevel[ nLevel ].mnBuFlags &=~ nMask16;
        nVal16 &= nMask16;
        maParaLevel[ nLevel ].mnBuFlags |= nVal16;
    }
    if ( nPMask & 0x0080 )
        rIn.ReadUInt16( maParaLevel[ nLevel ].mnBulletChar );
    if ( nPMask & 0x0010 )
        rIn.ReadUInt16( maParaLevel[ nLevel ].mnBulletFont );
    if ( nPMask & 0x0040 )
    {
        rIn.ReadUInt16( nVal16 );
        maParaLevel[ nLevel ].mnBulletHeight = nVal16;
    }
    if ( nPMask & 0x0020 )
    {
        rIn.ReadUInt32( nVal32 );
        maParaLevel[ nLevel ].mnBulletColor = nVal32;
    }
    if ( bFirst )
    {
        if ( nPMask & 0xF00 )
        {   // AbsJust!
            rIn.ReadUInt16( nVal16 );
            maParaLevel[ nLevel ].mnAdjust = nVal16 & 3;
        }
        if ( nPMask & 0x1000 )
            rIn.ReadUInt16( maParaLevel[ nLevel ].mnLineFeed );
        if ( nPMask & 0x2000 )
            rIn.ReadUInt16( maParaLevel[ nLevel ].mnUpperDist );
        if ( nPMask & 0x4000 )
            rIn.ReadUInt16( maParaLevel[ nLevel ].mnLowerDist );
        if ( nPMask & 0x8000 )
            rIn.ReadUInt16( maParaLevel[ nLevel ].mnTextOfs );
        if ( nPMask & 0x10000 )
            rIn.ReadUInt16( maParaLevel[ nLevel ].mnBulletOfs );
        if ( nPMask & 0x20000 )
            rIn.ReadUInt16( maParaLevel[ nLevel ].mnDefaultTab );
        if ( nPMask & 0x200000 )
        {
            // number of tabulators
            rIn.ReadUInt16( nVal16 );
            if (rIn.remainingSize() / sizeof(nVal32) < nVal16)
                return;
            for ( i = 0; i < nVal16; i++ )
                rIn.ReadUInt32( nVal32 );      // reading the tabulators
        }
        if ( nPMask & 0x40000 )
            rIn.ReadUInt16( nVal16 );
        if ( nPMask & 0x80000 )
            rIn.ReadUInt16( maParaLevel[ nLevel ].mnAsianLineBreak );
        if ( nPMask & 0x100000 )
            rIn.ReadUInt16( maParaLevel[ nLevel ].mnBiDi );
    }
    else
    {
        if ( nPMask & 0x800 )
        {
            rIn.ReadUInt16( nVal16 );
            maParaLevel[ nLevel ].mnAdjust = nVal16 & 3;
        }
        if ( nPMask & 0x1000 )
            rIn.ReadUInt16( maParaLevel[ nLevel ].mnLineFeed );
        if ( nPMask & 0x2000 )
            rIn.ReadUInt16( maParaLevel[ nLevel ].mnUpperDist );
        if ( nPMask & 0x4000 )
            rIn.ReadUInt16( maParaLevel[ nLevel ].mnLowerDist );
        if ( nPMask & 0x8000 )
            rIn.ReadUInt16( nVal16 );
        if ( nPMask & 0x100 )
            rIn.ReadUInt16( maParaLevel[ nLevel ].mnTextOfs );
        if ( nPMask & 0x200 )
            rIn.ReadUInt16( nVal16 );
        if ( nPMask & 0x400 )
            rIn.ReadUInt16( maParaLevel[ nLevel ].mnBulletOfs );
        if ( nPMask & 0x10000 )
            rIn.ReadUInt16( nVal16 );
        if ( nPMask & 0xe0000 )
        {
            sal_uInt16 nFlagsToModifyMask = (sal_uInt16)( ( nPMask >> 17 ) & 7 );
            rIn.ReadUInt16( nVal16 );
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
            rIn.ReadUInt16( nVal16 );
            for ( i = 0; i < nVal16; i++ )
                rIn.ReadUInt32( nVal32 );      // reading the tabulators
        }
        if ( nPMask & 0x200000 )
            rIn.ReadUInt16( maParaLevel[ nLevel ].mnBiDi );
    }

    nPMask >>= 22;
    while( nPMask )
    {
        if ( nPMask & 1 )
        {
#ifdef DBG_UTIL
            if (!(rManager.rImportParam.nImportFlags & PPT_IMPORTFLAGS_NO_TEXT_ASSERT))
            {
                OSL_FAIL( "PPTParaSheet::Read - unknown attribute, send me this document (SJ)" );
            }
#endif
            rIn.ReadUInt16( nVal16 );
        }
        nPMask >>= 1;
    }
}

void PPTParaSheet::UpdateBulletRelSize(  sal_uInt32 nLevel, sal_uInt16 nFontHeight )
{
    if ( maParaLevel[ nLevel ].mnBulletHeight > 0x7fff ) // a negative value is the absolute bullet height
    {
        sal_Int16  nBulletRelSize = ( sal_Int16 )maParaLevel[ nLevel ].mnBulletHeight;
        nBulletRelSize = nFontHeight ? ((-nBulletRelSize) * 100 ) / nFontHeight : 100;
        if ( nBulletRelSize < 0 ) //bullet size over flow
            nBulletRelSize = 100;
        maParaLevel[ nLevel ].mnBulletHeight = nBulletRelSize;
    }
}

PPTStyleSheet::PPTStyleSheet( const DffRecordHeader& rSlideHd, SvStream& rIn, SdrPowerPointImport& rManager,
                              const PPTTextParagraphStyleAtomInterpreter& rTxPFStyle,
                              const PPTTextSpecInfo& rTextSpecInfo ) :

    PPTNumberFormatCreator  ( new PPTExtParaProv( rManager, rIn, &rSlideHd ) ),
    maTxSI                  ( rTextSpecInfo )
{
    sal_uInt32 nOldFilePos = rIn.Tell();

    // default stylesheets
    mpCharSheet[ TSS_Type::PageTitle ] = new PPTCharSheet( TSS_Type::PageTitle );
    mpCharSheet[ TSS_Type::Body ] = new PPTCharSheet( TSS_Type::Body );
    mpCharSheet[ TSS_Type::Notes ] = new PPTCharSheet(  TSS_Type::Notes );
    mpCharSheet[ TSS_Type::Unused ] = new PPTCharSheet( TSS_Type::Unused );   // this entry is not used by ppt
    mpCharSheet[ TSS_Type::TextInShape ] = new PPTCharSheet( TSS_Type::TextInShape );
    mpParaSheet[ TSS_Type::PageTitle ] = new PPTParaSheet( TSS_Type::PageTitle );
    mpParaSheet[ TSS_Type::Body ] = new PPTParaSheet( TSS_Type::Body );
    mpParaSheet[ TSS_Type::Notes ] = new PPTParaSheet( TSS_Type::Notes );
    mpParaSheet[ TSS_Type::Unused ] = new PPTParaSheet( TSS_Type::Unused );
    mpParaSheet[ TSS_Type::TextInShape ] = new PPTParaSheet( TSS_Type::TextInShape );
    mpCharSheet[ TSS_Type::QuarterBody ] = mpCharSheet[ TSS_Type::HalfBody ] = mpCharSheet[ TSS_Type::Title ] = mpCharSheet[ TSS_Type::Subtitle ] = nullptr;
    mpParaSheet[ TSS_Type::QuarterBody ] = mpParaSheet[ TSS_Type::HalfBody ] = mpParaSheet[ TSS_Type::Title ] = mpParaSheet[ TSS_Type::Subtitle ] = nullptr;

    /* SJ: try to locate the txMasterStyleAtom in the Environment

       it seems that the environment TextStyle is having a higher priority
       than the TextStyle that can be found within the master page
    */
    bool bFoundTxMasterStyleAtom04 = false;
    DffRecordHeader* pEnvHeader = rManager.aDocRecManager.GetRecordHeader( PPT_PST_Environment );
    if ( pEnvHeader )
    {
        pEnvHeader->SeekToContent( rIn );
        DffRecordHeader aTxMasterStyleHd;
        auto nEndRecPos = DffPropSet::SanitizeEndPos(rIn, pEnvHeader->GetRecEndFilePos());
        while (rIn.Tell() < nEndRecPos)
        {
            ReadDffRecordHeader( rIn, aTxMasterStyleHd );
            if ( aTxMasterStyleHd.nRecType == PPT_PST_TxMasterStyleAtom )
            {
                sal_uInt16 nLevelAnz(0);
                rIn.ReadUInt16(nLevelAnz);

                sal_uInt16 nLev = 0;
                bool bFirst = true;
                bFoundTxMasterStyleAtom04 = true;
                auto nTxEndRecPos = DffPropSet::SanitizeEndPos(rIn, aTxMasterStyleHd.GetRecEndFilePos());
                while (rIn.GetError() == 0 && rIn.Tell() < nTxEndRecPos && nLev < nLevelAnz && nLev < nMaxPPTLevels)
                {
                    if ( nLev )
                    {
                        mpParaSheet[ TSS_Type::TextInShape ]->maParaLevel[ nLev ] = mpParaSheet[ TSS_Type::TextInShape ]->maParaLevel[ nLev - 1 ];
                        mpCharSheet[ TSS_Type::TextInShape ]->maCharLevel[ nLev ] = mpCharSheet[ TSS_Type::TextInShape ]->maCharLevel[ nLev - 1 ];
                    }
                    mpParaSheet[ TSS_Type::TextInShape ]->Read( rManager, rIn, true, nLev, bFirst );
                    if ( !nLev )
                    {
                        // set paragraph defaults for instance 4 (TSS_Type::TextInShape)
                        if ( rTxPFStyle.bValid )
                        {
                            PPTParaLevel& rParaLevel = mpParaSheet[ TSS_Type::TextInShape ]->maParaLevel[ 0 ];
                            rParaLevel.mnAsianLineBreak = 0;
                            if ( rTxPFStyle.bForbiddenRules )
                                rParaLevel.mnAsianLineBreak |= 1;
                            if ( !rTxPFStyle.bLatinTextWrap )
                                rParaLevel.mnAsianLineBreak |= 2;
                            if ( rTxPFStyle.bHangingPunctuation )
                                rParaLevel.mnAsianLineBreak |= 4;
                        }
                    }
                    mpCharSheet[ TSS_Type::TextInShape ]->Read( rIn, true, nLev, bFirst );
                    mpParaSheet[ TSS_Type::TextInShape ]->UpdateBulletRelSize(  nLev, mpCharSheet[ TSS_Type::TextInShape ]->maCharLevel[ nLev ].mnFontHeight );
                    bFirst = false;
                    nLev++;
                }
                break;
            }
            else
            {
                if (!aTxMasterStyleHd.SeekToEndOfRecord(rIn))
                    break;
            }
        }
    }

    rSlideHd.SeekToContent( rIn );

    DffRecordHeader aTxMasterStyleHd;
    auto nEndRecPos = DffPropSet::SanitizeEndPos(rIn, rSlideHd.GetRecEndFilePos());
    while (rIn.Tell() < nEndRecPos)
    {
        ReadDffRecordHeader( rIn, aTxMasterStyleHd );
        if ( aTxMasterStyleHd.nRecType == PPT_PST_TxMasterStyleAtom )
            break;
        else
        {
            if (!aTxMasterStyleHd.SeekToEndOfRecord(rIn))
                break;
        }
    }
    while ( ( aTxMasterStyleHd.nRecType == PPT_PST_TxMasterStyleAtom ) && ( rIn.Tell() < nEndRecPos ) ) //TODO: aTxMasterStyleHd may be used without having been properly initialized
    {
        TSS_Type nInstance = (TSS_Type)aTxMasterStyleHd.nRecInstance;
        if ( ( nInstance <= TSS_Type::LAST ) &&
            ( ( nInstance != TSS_Type::TextInShape ) || !bFoundTxMasterStyleAtom04 ) )
        {
            if ( nInstance > TSS_Type::TextInShape )
            {
                delete mpCharSheet[ nInstance ];    // be sure to delete the old one if this instance comes twice
                delete mpParaSheet[ nInstance ];

                switch ( nInstance )
                {
                    case TSS_Type::Subtitle :
                    {
                        mpCharSheet[ TSS_Type::Subtitle ] = new PPTCharSheet( *( mpCharSheet[ TSS_Type::Body ] ) );
                        mpParaSheet[ TSS_Type::Subtitle ] = new PPTParaSheet( *( mpParaSheet[ TSS_Type::Body ] ) );
                    }
                    break;
                    case TSS_Type::Title :
                    {
                        mpCharSheet[ TSS_Type::Title ] = new PPTCharSheet( *( mpCharSheet[ TSS_Type::PageTitle ] ) );
                        mpParaSheet[ TSS_Type::Title ] = new PPTParaSheet( *( mpParaSheet[ TSS_Type::PageTitle ] ) );
                    }
                    break;
                    case TSS_Type::HalfBody :
                    {
                        mpCharSheet[ TSS_Type::HalfBody ] = new PPTCharSheet( *( mpCharSheet[ TSS_Type::Body ] ) );
                        mpParaSheet[ TSS_Type::HalfBody ] = new PPTParaSheet( *( mpParaSheet[ TSS_Type::Body ] ) );
                    }
                    break;

                    case TSS_Type::QuarterBody :
                    {
                        mpCharSheet[ TSS_Type::QuarterBody ] = new PPTCharSheet( *( mpCharSheet[ TSS_Type::Body ] ) );
                        mpParaSheet[ TSS_Type::QuarterBody ] = new PPTParaSheet( *( mpParaSheet[ TSS_Type::Body ] ) );
                    }
                    break;
                    default: break;
                }
            }
            sal_uInt16 nLevelAnz(0);
            rIn.ReadUInt16(nLevelAnz);
            if (nLevelAnz > nMaxPPTLevels)
            {
                OSL_FAIL( "PPTStyleSheet::Ppt-TextStylesheet hat mehr als 5 Ebenen! (SJ)" );
                nLevelAnz = nMaxPPTLevels;
            }
            sal_uInt16  nLev = 0;
            bool    bFirst = true;

            auto nTxEndRecPos = DffPropSet::SanitizeEndPos(rIn, aTxMasterStyleHd.GetRecEndFilePos());
            while ( rIn.GetError() == 0 && rIn.Tell() < nTxEndRecPos && nLev < nLevelAnz )
            {
                if ( nLev && ( nInstance < TSS_Type::Subtitle ) )
                {
                    mpParaSheet[ nInstance ]->maParaLevel[ nLev ] = mpParaSheet[ nInstance ]->maParaLevel[ nLev - 1 ];
                    mpCharSheet[ nInstance ]->maCharLevel[ nLev ] = mpCharSheet[ nInstance ]->maCharLevel[ nLev - 1 ];
                }

                // Exception: Template 5, 6 (MasterTitle Title und SubTitle)
                if ( nInstance >= TSS_Type::Subtitle )
                {
                    bFirst = false;

                    sal_uInt16 nDontKnow;
                    rIn.ReadUInt16( nDontKnow );
                }
                mpParaSheet[ nInstance ]->Read( rManager, rIn, true, nLev, bFirst );
                mpCharSheet[ nInstance ]->Read( rIn, true, nLev, bFirst );
                mpParaSheet[ nInstance ]->UpdateBulletRelSize(  nLev, mpCharSheet[ nInstance ]->maCharLevel[ nLev ].mnFontHeight );
                bFirst = false;
                nLev++;
            }
#ifdef DBG_UTIL
            if (!(rManager.rImportParam.nImportFlags & PPT_IMPORTFLAGS_NO_TEXT_ASSERT))
            {
                if ( rIn.GetError() == 0 )
                {
                    OStringBuffer aMsg;
                    if ( rIn.Tell() > aTxMasterStyleHd.GetRecEndFilePos() )
                    {
                        aMsg.append("\n  reading too many bytes:" +
                                    OString::number(rIn.Tell() - aTxMasterStyleHd.GetRecEndFilePos()));
                    }
                    if ( rIn.Tell() < aTxMasterStyleHd.GetRecEndFilePos() )
                    {
                        aMsg.append("\n  reading too few bytes:" +
                                    OString::number(aTxMasterStyleHd.GetRecEndFilePos() - rIn.Tell()));
                    }
                    if (aMsg.getLength())
                    {
                        aMsg.insert(0, "PptStyleSheet::operator>>[]");
                        OSL_FAIL(aMsg.getStr());
                    }
                }
                if ( rIn.Tell() != aTxMasterStyleHd.GetRecEndFilePos() )
                    SAL_WARN( "filter.ms", "SJ: Falsche Anzahl von Bytes gelesen beim Import der PPT-Formatvorlagen");
            }
#endif
        }
        if (!aTxMasterStyleHd.SeekToEndOfRecord(rIn))
            break;
        ReadDffRecordHeader( rIn, aTxMasterStyleHd );
    }
    if ( !mpCharSheet[ TSS_Type::Subtitle ] )
    {
        mpCharSheet[ TSS_Type::Subtitle ] = new PPTCharSheet( *( mpCharSheet[ TSS_Type::Body ] ) );
        mpParaSheet[ TSS_Type::Subtitle ] = new PPTParaSheet( *( mpParaSheet[ TSS_Type::Body ] ) );
    }
    if ( !mpCharSheet[ TSS_Type::Title ] )
    {
        mpCharSheet[ TSS_Type::Title ] = new PPTCharSheet( *( mpCharSheet[ TSS_Type::PageTitle ] ) );
        mpParaSheet[ TSS_Type::Title ] = new PPTParaSheet( *( mpParaSheet[ TSS_Type::PageTitle ] ) );
    }
    if ( !mpCharSheet[ TSS_Type::HalfBody ] )
    {
        mpCharSheet[ TSS_Type::HalfBody ] = new PPTCharSheet( *( mpCharSheet[ TSS_Type::Body ] ) );
        mpParaSheet[ TSS_Type::HalfBody ] = new PPTParaSheet( *( mpParaSheet[ TSS_Type::Body ] ) );
    }
    if ( !mpCharSheet[ TSS_Type::QuarterBody ] )
    {
        mpCharSheet[ TSS_Type::QuarterBody ] = new PPTCharSheet( *( mpCharSheet[ TSS_Type::Body ] ) );
        mpParaSheet[ TSS_Type::QuarterBody ] = new PPTParaSheet( *( mpParaSheet[ TSS_Type::Body ] ) );
    }
    if ( !bFoundTxMasterStyleAtom04 )
    {   // try to locate the txMasterStyleAtom in the Environment
        DffRecordHeader* pEnvHeader2 = rManager.aDocRecManager.GetRecordHeader( PPT_PST_Environment );
        if ( pEnvHeader2 )
        {
            pEnvHeader2->SeekToContent( rIn );
            DffRecordHeader aTxMasterStyleHd2;
            auto nEnvEndRecPos = DffPropSet::SanitizeEndPos(rIn, pEnvHeader2->GetRecEndFilePos());
            while (rIn.Tell() < nEnvEndRecPos)
            {
                ReadDffRecordHeader( rIn, aTxMasterStyleHd2 );
                if ( aTxMasterStyleHd2.nRecType == PPT_PST_TxMasterStyleAtom )
                {
                    sal_uInt16 nLevelAnz;
                    rIn.ReadUInt16( nLevelAnz );

                    sal_uInt16 nLev = 0;
                    bool bFirst = true;
                    auto nTxEndRecPos = DffPropSet::SanitizeEndPos(rIn, aTxMasterStyleHd2.GetRecEndFilePos());
                    while ( rIn.GetError() == 0 && rIn.Tell() < nTxEndRecPos && nLev < nLevelAnz )
                    {
                        if ( nLev )
                        {
                            mpParaSheet[ TSS_Type::TextInShape ]->maParaLevel[ nLev ] = mpParaSheet[ TSS_Type::TextInShape ]->maParaLevel[ nLev - 1 ];
                            mpCharSheet[ TSS_Type::TextInShape ]->maCharLevel[ nLev ] = mpCharSheet[ TSS_Type::TextInShape ]->maCharLevel[ nLev - 1 ];
                        }
                        mpParaSheet[ TSS_Type::TextInShape ]->Read( rManager, rIn, true, nLev, bFirst );
                        if ( !nLev )
                        {
                            // set paragraph defaults for instance 4 (TSS_Type::TextInShape)
                            if ( rTxPFStyle.bValid )
                            {
                                PPTParaLevel& rParaLevel = mpParaSheet[ TSS_Type::TextInShape ]->maParaLevel[ 0 ];
                                rParaLevel.mnAsianLineBreak = 0;
                                if ( rTxPFStyle.bForbiddenRules )
                                    rParaLevel.mnAsianLineBreak |= 1;
                                if ( !rTxPFStyle.bLatinTextWrap )
                                    rParaLevel.mnAsianLineBreak |= 2;
                                if ( rTxPFStyle.bHangingPunctuation )
                                    rParaLevel.mnAsianLineBreak |= 4;
                            }
                        }
                        mpCharSheet[ TSS_Type::TextInShape ]->Read( rIn, true, nLev, bFirst );
                        mpParaSheet[ TSS_Type::TextInShape ]->UpdateBulletRelSize(  nLev, mpCharSheet[ TSS_Type::TextInShape ]->maCharLevel[ nLev ].mnFontHeight );
                        bFirst = false;
                        nLev++;
                    }
                    break;
                }
                else
                {
                    if (!aTxMasterStyleHd2.SeekToEndOfRecord(rIn))
                        break;
                }
            }
        }
    }
    rIn.Seek( nOldFilePos );

    // will will create the default numbulletitem for each instance
    for ( auto i : o3tl::enumrange<TSS_Type>() )
    {
        sal_uInt16          nLevels, nDepth = 0;
        SvxNumRuleType  eNumRuleType;

        switch ( i )
        {
            case TSS_Type::PageTitle :
            case TSS_Type::Title :
                nLevels = 1;
                eNumRuleType = SvxNumRuleType::NUMBERING;
            break;
            case TSS_Type::Subtitle :
                nLevels = SVX_MAX_NUM;
                eNumRuleType = SvxNumRuleType::NUMBERING;
            break;
            case TSS_Type::Body :
            case TSS_Type::HalfBody :
            case TSS_Type::QuarterBody :
                nLevels = SVX_MAX_NUM;
                eNumRuleType = SvxNumRuleType::PRESENTATION_NUMBERING;
            break;
            default :
            case TSS_Type::Notes :
            case TSS_Type::Unused :
            case TSS_Type::TextInShape :
                nLevels = SVX_MAX_NUM;
                eNumRuleType = SvxNumRuleType::NUMBERING;
            break;
        }
        SvxNumRule aRule( SvxNumRuleFlags::BULLET_REL_SIZE | SvxNumRuleFlags::BULLET_COLOR |
                        SvxNumRuleFlags::CHAR_TEXT_DISTANCE | SvxNumRuleFlags::SYMBOL_ALIGNMENT,
                        nLevels, false, eNumRuleType );
        for ( sal_uInt16 nCount = 0; nDepth < nLevels; nCount++ )
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
                if ( eNumRuleType == SvxNumRuleType::PRESENTATION_NUMBERING )
                    aRule.SetLevel( 0, aNumberFormat );
            }
        }
        mpNumBulletItem[ i ] = new SvxNumBulletItem( aRule, EE_PARA_NUMBULLET );
    }
}

PPTStyleSheet::~PPTStyleSheet()
{
    for ( auto i : o3tl::enumrange<TSS_Type>() )
    {
        delete mpCharSheet[ i ];
        delete mpParaSheet[ i ];
        delete mpNumBulletItem[ i ];
    }
}

PPTParaPropSet::PPTParaPropSet()
    : mnOriginalTextPos(0)
    , pParaSet( new ImplPPTParaPropSet )
{
    pParaSet->mnHasAnm = 1;
}

PPTParaPropSet::PPTParaPropSet( PPTParaPropSet& rParaPropSet )
{
    pParaSet = rParaPropSet.pParaSet;
    pParaSet->mnRefCount++;

    mnOriginalTextPos = rParaPropSet.mnOriginalTextPos;
}

PPTParaPropSet::~PPTParaPropSet()
{
    if ( ! ( --pParaSet->mnRefCount ) )
        delete pParaSet;
}

PPTParaPropSet& PPTParaPropSet::operator=( const PPTParaPropSet& rParaPropSet )
{
    if ( this != &rParaPropSet )
    {
        if ( ! ( --pParaSet->mnRefCount ) )
            delete pParaSet;
        pParaSet = rParaPropSet.pParaSet;
        pParaSet->mnRefCount++;

        mnOriginalTextPos = rParaPropSet.mnOriginalTextPos;
    }
    return *this;
}

PPTCharPropSet::PPTCharPropSet(sal_uInt32 nParagraph)
    : mnOriginalTextPos(0)
    , mnParagraph(nParagraph)
    , mpFieldItem(nullptr)
    , mpImplPPTCharPropSet()
{
    mnHylinkOrigColor = 0;
    mbIsHyperlink = false;
    mbHardHylinkOrigColor = false;
    mnLanguage[ 0 ] = mnLanguage[ 1 ] = mnLanguage[ 2 ] = 0;
}

PPTCharPropSet::PPTCharPropSet( const PPTCharPropSet& rCharPropSet )
    : mpImplPPTCharPropSet( rCharPropSet.mpImplPPTCharPropSet )
{
    mnHylinkOrigColor = rCharPropSet.mnHylinkOrigColor;
    mbIsHyperlink = rCharPropSet.mbIsHyperlink;
    mbHardHylinkOrigColor = rCharPropSet.mbHardHylinkOrigColor;

    mnParagraph = rCharPropSet.mnParagraph;
    mnOriginalTextPos = rCharPropSet.mnOriginalTextPos;
    maString = rCharPropSet.maString;
    mpFieldItem.reset( rCharPropSet.mpFieldItem ? new SvxFieldItem( *rCharPropSet.mpFieldItem ) : nullptr );
    mnLanguage[ 0 ] = rCharPropSet.mnLanguage[ 0 ];
    mnLanguage[ 1 ] = rCharPropSet.mnLanguage[ 1 ];
    mnLanguage[ 2 ] = rCharPropSet.mnLanguage[ 2 ];
}

PPTCharPropSet::PPTCharPropSet( const PPTCharPropSet& rCharPropSet, sal_uInt32 nParagraph )
    : mpImplPPTCharPropSet(rCharPropSet.mpImplPPTCharPropSet)
{
    mnHylinkOrigColor = rCharPropSet.mnHylinkOrigColor;
    mbIsHyperlink = rCharPropSet.mbIsHyperlink;
    mbHardHylinkOrigColor = rCharPropSet.mbHardHylinkOrigColor;

    mnParagraph = nParagraph;
    mnOriginalTextPos = rCharPropSet.mnOriginalTextPos;
    maString = rCharPropSet.maString;
    mpFieldItem.reset( rCharPropSet.mpFieldItem ? new SvxFieldItem( *rCharPropSet.mpFieldItem ) : nullptr );
    mnLanguage[ 0 ] = mnLanguage[ 1 ] = mnLanguage[ 2 ] = 0;
}

PPTCharPropSet::~PPTCharPropSet()
{
}

PPTCharPropSet& PPTCharPropSet::operator=( const PPTCharPropSet& rCharPropSet )
{
    if ( this != &rCharPropSet )
    {
        mpImplPPTCharPropSet = rCharPropSet.mpImplPPTCharPropSet;
        mnOriginalTextPos = rCharPropSet.mnOriginalTextPos;
        mnParagraph = rCharPropSet.mnParagraph;
        maString = rCharPropSet.maString;
        mpFieldItem.reset( rCharPropSet.mpFieldItem ? new SvxFieldItem( *rCharPropSet.mpFieldItem ) : nullptr );
    }
    return *this;
}

void PPTCharPropSet::SetFont( sal_uInt16 nFont )
{
    sal_uInt32  nMask = 1 << PPT_CharAttr_Font;
    bool bDoNotMake = (mpImplPPTCharPropSet->mnAttrSet & nMask) != 0;

    if ( bDoNotMake )
        bDoNotMake = nFont == mpImplPPTCharPropSet->mnFont;

    if ( !bDoNotMake )
    {
        mpImplPPTCharPropSet->mnFont = nFont;
        mpImplPPTCharPropSet->mnAttrSet |= nMask;
    }
}

void PPTCharPropSet::SetColor( sal_uInt32 nColor )
{
    mpImplPPTCharPropSet->mnColor = nColor;
    mpImplPPTCharPropSet->mnAttrSet |= 1 << PPT_CharAttr_FontColor;
}

PPTRuler::PPTRuler()
    : nRefCount(1)
    , nFlags(0)
    , nDefaultTab(0x240)
    , pTab(nullptr)
    , nTabCount(0)
{
    memset(nTextOfs, 0, sizeof(nTextOfs));
    memset(nBulletOfs, 0, sizeof(nBulletOfs));
}

PPTRuler::~PPTRuler()
{
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

PPTTextRulerInterpreter::PPTTextRulerInterpreter( sal_uInt32 nFileOfs, DffRecordHeader& rHeader, SvStream& rIn ) :
    mpImplRuler ( new PPTRuler() )
{
    if ( nFileOfs != 0xffffffff )
    {
        sal_uInt32 nOldPos = rIn.Tell();
        DffRecordHeader rHd;
        if ( nFileOfs )
        {
            rIn.Seek( nFileOfs );
            ReadDffRecordHeader( rIn, rHd );
        }
        else
        {
            rHeader.SeekToContent( rIn );
            if ( SvxMSDffManager::SeekToRec( rIn, PPT_PST_TextRulerAtom, rHeader.GetRecEndFilePos(), &rHd ) )
                nFileOfs++;
        }
        if ( nFileOfs )
        {
            bool bRecordOk = true;

            sal_Int16   nTCount(0);
            sal_Int32   i;
            rIn.ReadInt32( mpImplRuler->nFlags );

            // number of indent levels, unused now
            if ( mpImplRuler->nFlags & 2 )
                rIn.ReadInt16( nTCount );
            if ( mpImplRuler->nFlags & 1 )
                rIn.ReadUInt16( mpImplRuler->nDefaultTab );
            if ( mpImplRuler->nFlags & 4 )
            {
                rIn.ReadInt16(nTCount);

                const size_t nMaxPossibleRecords = rIn.remainingSize() / (2*sizeof(sal_uInt16));
                const sal_uInt16 nTabCount(nTCount);

                bRecordOk = nTabCount <= nMaxPossibleRecords;

                if (nTCount && bRecordOk)
                {
                    mpImplRuler->nTabCount = nTabCount;
                    mpImplRuler->pTab.reset( new PPTTabEntry[ mpImplRuler->nTabCount ] );
                    for ( i = 0; i < nTCount; i++ )
                    {
                        rIn.ReadUInt16( mpImplRuler->pTab[ i ].nOffset )
                           .ReadUInt16( mpImplRuler->pTab[ i ].nStyle );
                    }
                }
            }

            if (bRecordOk)
            {
                for ( i = 0; i < 5; i++ )
                {
                    if ( mpImplRuler->nFlags & ( 8 << i ) )
                        rIn.ReadUInt16( mpImplRuler->nTextOfs[ i ] );
                    if ( mpImplRuler->nFlags & ( 256 << i ) )
                        rIn.ReadUInt16( mpImplRuler->nBulletOfs[ i ] );
                    if( mpImplRuler->nBulletOfs[ i ] > 0x7fff)
                    {
                        // workaround
                        // when bullet offset is > 0x7fff, the paragraph should look like
                        // *    first line text
                        // second line text

                        // we add to bullet para indent 0xffff - bullet offset. it looks like
                        // best we can do for now
                        mpImplRuler->nTextOfs[ i ] += 0xffff - mpImplRuler->nBulletOfs[ i ];
                        mpImplRuler->nBulletOfs[ i ] = 0;
                    }
                }
            }
        }
        rIn.Seek( nOldPos );
    }
}

bool PPTTextRulerInterpreter::GetDefaultTab( sal_uInt32 /*nLevel*/, sal_uInt16& nValue ) const
{
    if ( ! ( mpImplRuler->nFlags & 1 ) )
        return false;
    nValue = mpImplRuler->nDefaultTab;
    return true;
}

bool PPTTextRulerInterpreter::GetTextOfs( sal_uInt32 nLevel, sal_uInt16& nValue ) const
{
    if ( ! ( ( nLevel < 5 ) && ( mpImplRuler->nFlags & ( 8 << nLevel ) ) ) )
        return false;
    nValue = mpImplRuler->nTextOfs[ nLevel ];
    return true;
}

bool PPTTextRulerInterpreter::GetBulletOfs( sal_uInt32 nLevel, sal_uInt16& nValue ) const
{
    if ( ! ( ( nLevel < 5 ) && ( mpImplRuler->nFlags & ( 256 << nLevel ) ) ) )
        return false;
    nValue = mpImplRuler->nBulletOfs[ nLevel ];
    return true;
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

PPTTextParagraphStyleAtomInterpreter::PPTTextParagraphStyleAtomInterpreter() :
    bValid              ( false ),
    bForbiddenRules     ( false ),
    bHangingPunctuation ( false ),
    bLatinTextWrap      ( false )
{
}

bool PPTTextParagraphStyleAtomInterpreter::Read( SvStream& rIn, const DffRecordHeader& rRecHd )
{
    bValid = false;
    rRecHd.SeekToContent( rIn );
    sal_uInt32 nDummy32, nFlags, nRecEndPos = rRecHd.GetRecEndFilePos();
    sal_uInt16 nDummy16;

    rIn.ReadUInt16( nDummy16 )
       .ReadUInt32( nFlags );

    if ( nFlags & 0xf && ( rIn.Tell() < nRecEndPos ) )
        rIn.ReadUInt16( nDummy16 );    // BuFlags
    if ( nFlags & 0x80 && ( rIn.Tell() < nRecEndPos ) )
        rIn.ReadUInt16( nDummy16 );    // BuChar
    if ( nFlags & 0x10 && ( rIn.Tell() < nRecEndPos ) )
        rIn.ReadUInt16( nDummy16 );    // nBuFont;
    if ( nFlags & 0x40 && ( rIn.Tell() < nRecEndPos ) )
        rIn.ReadUInt16( nDummy16 );    // nBuHeight;
    if ( nFlags & 0x0020 && ( rIn.Tell() < nRecEndPos ) )
        rIn.ReadUInt32( nDummy32 );    // nBuColor;
    if ( nFlags & 0x800 && ( rIn.Tell() < nRecEndPos ) )
        rIn.ReadUInt16( nDummy16 );    // AbsJust!
    if ( nFlags & 0x400 && ( rIn.Tell() < nRecEndPos ) )
        rIn.ReadUInt16( nDummy16 );
    if ( nFlags & 0x200 && ( rIn.Tell() < nRecEndPos ) )
        rIn.ReadUInt16( nDummy16 );
    if ( nFlags & 0x100 && ( rIn.Tell() < nRecEndPos ) )
        rIn.ReadUInt16( nDummy16 );
    if ( nFlags & 0x1000 && ( rIn.Tell() < nRecEndPos ) )
        rIn.ReadUInt16( nDummy16 );    // LineFeed
    if ( nFlags & 0x2000 && ( rIn.Tell() < nRecEndPos ) )
        rIn.ReadUInt16( nDummy16 );    // nUpperDist
    if ( nFlags & 0x4000 && ( rIn.Tell() < nRecEndPos ) )
        rIn.ReadUInt16( nDummy16 );    // nLowerDist
    if ( nFlags & 0x8000 && ( rIn.Tell() < nRecEndPos ) )
        rIn.ReadUInt16( nDummy16 );
    if ( nFlags & 0x10000 && ( rIn.Tell() < nRecEndPos ) )
        rIn.ReadUInt16( nDummy16 );
    if ( nFlags & 0xe0000 && ( rIn.Tell() < nRecEndPos ) )
    {
        rIn.ReadUInt16( nDummy16 );
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
            rIn.ReadUInt16( nDummy16 );
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

PPTTextSpecInfo::PPTTextSpecInfo( sal_uInt32 _nCharIdx ) :
    nCharIdx        ( _nCharIdx ),
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
    bValid  ( false )
{
}

bool PPTTextSpecInfoAtomInterpreter::Read( SvStream& rIn, const DffRecordHeader& rRecHd,
    sal_uInt16 nRecordType, const PPTTextSpecInfo* pTextSpecDefault )
{
    bValid = false;
    sal_uInt32  nCharIdx = 0;
    rRecHd.SeekToContent( rIn );

    auto nEndRecPos = DffPropSet::SanitizeEndPos(rIn, rRecHd.GetRecEndFilePos());
    while (rIn.Tell() < nEndRecPos && rIn.good())
    {
        if ( nRecordType == PPT_PST_TextSpecInfoAtom )
        {
            sal_uInt32 nCharCount(0);
            rIn.ReadUInt32( nCharCount );
            nCharIdx += nCharCount;
        }

        sal_uInt32 nFlags(0);
        rIn.ReadUInt32(nFlags);

        PPTTextSpecInfo* pEntry = new PPTTextSpecInfo( nCharIdx );
        if ( pTextSpecDefault )
        {
            pEntry->nDontKnow = pTextSpecDefault->nDontKnow;
            pEntry->nLanguage[ 0 ] = pTextSpecDefault->nLanguage[ 0 ];
            pEntry->nLanguage[ 1 ] = pTextSpecDefault->nLanguage[ 1 ];
            pEntry->nLanguage[ 2 ] = pTextSpecDefault->nLanguage[ 2 ];
        }
        for (sal_uInt32 i = 1; nFlags && i ; i <<= 1)
        {
            sal_uInt16 nLang = 0;
            switch( nFlags & i )
            {
                case 0 : break;
                case 1 : rIn.ReadUInt16( pEntry->nDontKnow ); break;
                case 2 : rIn.ReadUInt16( nLang ); break;
                case 4 : rIn.ReadUInt16( nLang ); break;
                default :
                {
                    rIn.SeekRel( 2 );
                }
            }
            if ( nLang )
            {
                // #i119985#, we could probably handle this better if we have a
                // place to over-ride the final language for weak
                // characters/fields to fallback to, rather than the current
                // application locale. Assuming that we can determine what the
                // default fallback language for a given .ppt, etc is during
                // load time.
                if (i == 2)
                {
                    pEntry->nLanguage[ 0 ] = pEntry->nLanguage[ 1 ] = pEntry->nLanguage[ 2 ] = nLang;
                }
            }
            nFlags &= ~i;
        }
        aList.push_back( pEntry );
    }
    bValid = rIn.Tell() == rRecHd.GetRecEndFilePos();
    return bValid;
}

PPTTextSpecInfoAtomInterpreter::~PPTTextSpecInfoAtomInterpreter()
{
    for (PPTTextSpecInfo* i : aList) {
        delete i;
    }
    aList.clear();
}

void StyleTextProp9::Read( SvStream& rIn )
{
    rIn.ReadUInt32( mnExtParagraphMask );
    if ( mnExtParagraphMask & 0x800000 )
        rIn.ReadUInt16( mnBuBlip );
    if ( mnExtParagraphMask & 0x2000000 )
        rIn.ReadUInt16( mnHasAnm );
    if ( mnExtParagraphMask & 0x1000000 )
        rIn.ReadUInt32( mnAnmScheme );
    if ( mnExtParagraphMask & 0x4000000 )
        rIn.ReadUInt32( mpfPP10Ext );
    rIn.ReadUInt32( mnExtCharacterMask );
    if ( mnExtCharacterMask & 0x100000 )
        rIn.ReadUInt32( mncfPP10Ext );
    rIn.ReadUInt32( mnSpecialInfoMask );
    if ( mnSpecialInfoMask & 0x20 )
        rIn.ReadUInt32( mnPP10Ext );
    if ( mnSpecialInfoMask & 0x40 )
        rIn.ReadUInt16( mfBidi );
}

PPTStyleTextPropReader::PPTStyleTextPropReader( SvStream& rIn, const DffRecordHeader& rTextHeader,
                                                        PPTTextRulerInterpreter& rRuler, const DffRecordHeader& rExtParaHd, TSS_Type nInstance )
{
    Init(rIn, rTextHeader, rRuler, rExtParaHd, nInstance);
}

void PPTStyleTextPropReader::ReadParaProps( SvStream& rIn, const DffRecordHeader& rTextHeader,
                                            const OUString& aString, PPTTextRulerInterpreter& rRuler,
                                            sal_uInt32& nCharCount, bool& bTextPropAtom )
{
    sal_uInt32  nMask = 0; //TODO: nMask initialized here to suppress warning for now, see corresponding TODO below
    sal_uInt32  nCharAnzRead = 0;
    sal_uInt16  nDummy16;

    sal_uInt16 nStringLen = aString.getLength();

    DffRecordHeader aTextHd2;
    rTextHeader.SeekToContent( rIn );
    if ( SvxMSDffManager::SeekToRec( rIn, PPT_PST_StyleTextPropAtom, rTextHeader.GetRecEndFilePos(), &aTextHd2 ) )
        bTextPropAtom = true;
    while ( nCharAnzRead <= nStringLen )
    {
        PPTParaPropSet aParaPropSet;
        ImplPPTParaPropSet& aSet = *aParaPropSet.pParaSet;
        if ( bTextPropAtom )
        {
            rIn.ReadUInt32( nCharCount )
               .ReadUInt16( aParaPropSet.pParaSet->mnDepth );  // indent depth

            aParaPropSet.pParaSet->mnDepth =        // taking care of about using not more than 9 outliner levels
                std::min(sal_uInt16(8),
                    aParaPropSet.pParaSet->mnDepth);

            nCharCount--;

            rIn.ReadUInt32( nMask );
            aSet.mnAttrSet = nMask & 0x207df7;
            sal_uInt16 nBulFlg = 0;
            if ( nMask & 0xF )
                rIn.ReadUInt16( nBulFlg ); // Bullet-HardAttr-Flags
            aSet.mpArry[ PPT_ParaAttr_BulletOn    ] = ( nBulFlg & 1 ) ? 1 : 0;
            aSet.mpArry[ PPT_ParaAttr_BuHardFont  ] = ( nBulFlg & 2 ) ? 1 : 0;
            aSet.mpArry[ PPT_ParaAttr_BuHardColor ] = ( nBulFlg & 4 ) ? 1 : 0;

            // NOTE: one might think that the hard-coded numbers here are the
            // same as the PPT_ParaAttr_* constants, but it's NOT always true!
            if ( nMask & 0x0080 )   // buChar
            {
                rIn.ReadUInt16( aSet.mpArry[ PPT_ParaAttr_BulletChar ] );
                if (!rIn.good())
                {
                    aSet.mnAttrSet &= ~(1 << PPT_ParaAttr_BulletChar);
                }
            }
            if ( nMask & 0x0010 )   // buTypeface
            {
                rIn.ReadUInt16( aSet.mpArry[ PPT_ParaAttr_BulletFont ] );
                if (!rIn.good())
                {
                    aSet.mnAttrSet &= ~(1 << PPT_ParaAttr_BulletFont);
                }
            }
            if ( nMask & 0x0040 )   // buSize
            {
                rIn.ReadUInt16( aSet.mpArry[ PPT_ParaAttr_BulletHeight ] );
                if (!rIn.good()
                    || !((nMask & (1 << PPT_ParaAttr_BuHardHeight))
                         && (nBulFlg & (1 << PPT_ParaAttr_BuHardHeight))))
                {
                    aSet.mnAttrSet &= ~(1 << PPT_ParaAttr_BulletHeight);
                }
            }
            if ( nMask & 0x0020 )   // buColor
            {
                sal_uInt32 nVal32, nHiByte;
                rIn.ReadUInt32( nVal32 );
                if (!rIn.good())
                {
                    aSet.mnBulletColor = 0; // no flag for this? default it
                }
                else
                {
                    nHiByte = nVal32 >> 24;
                    if ( nHiByte <= 8 )
                        nVal32 = nHiByte | PPT_COLSCHEME;
                    aSet.mnBulletColor = nVal32;
                }
            }
            if ( nMask & 0x0800 )   // pfAlignment
            {
                rIn.ReadUInt16( nDummy16 );
                if (!rIn.good())
                {
                    aSet.mnAttrSet &= ~(1 << PPT_ParaAttr_Adjust);
                }
                else
                {
                    aSet.mpArry[ PPT_ParaAttr_Adjust ] = nDummy16 & 3;
                }
            }
            if ( nMask & 0x1000 )   // pfLineSpacing
            {
                rIn.ReadUInt16( aSet.mpArry[ PPT_ParaAttr_LineFeed ] );
                if (!rIn.good())
                {
                    aSet.mnAttrSet &= ~(1 << PPT_ParaAttr_LineFeed);
                }
            }
            if ( nMask & 0x2000 )   // pfSpaceBefore
            {
                rIn.ReadUInt16( aSet.mpArry[ PPT_ParaAttr_UpperDist ] );
                if (!rIn.good())
                {
                    aSet.mnAttrSet &= ~(1 << PPT_ParaAttr_UpperDist);
                }
            }
            if ( nMask & 0x4000 )   // pfSpaceAfter
            {
                rIn.ReadUInt16( aSet.mpArry[ PPT_ParaAttr_LowerDist ] );
                if (!rIn.good())
                {
                    aSet.mnAttrSet &= ~(1 << PPT_ParaAttr_LowerDist);
                }
            }
            if ( nMask & 0x100 )    // pfLeftMargin
            {
                rIn.ReadUInt16( aSet.mpArry[ PPT_ParaAttr_TextOfs ] );
                if (!rIn.good())
                {
                    aSet.mnAttrSet &= ~(1 << PPT_ParaAttr_TextOfs);
                }
                else
                {
                    aSet.mnAttrSet |= 1 << PPT_ParaAttr_TextOfs;
                }
            }
            if ( nMask & 0x400 )    // pfIndent
            {
                rIn.ReadUInt16( aSet.mpArry[ PPT_ParaAttr_BulletOfs ] );
                if (!rIn.good())
                {
                    aSet.mnAttrSet &= ~(1 << PPT_ParaAttr_BulletOfs);
                }
                else
                {
                    aSet.mnAttrSet |= 1 << PPT_ParaAttr_BulletOfs;
                }
            }
            if ( nMask & 0x8000 )   // pfDefaultTabSize
            {
                rIn.ReadUInt16( nDummy16 );
                if (!rIn.good())
                {
                    // TODO?
                }
            }
            if ( nMask & 0x100000 ) // pfTabStops
            {
                sal_uInt16 i, nDistance, nAlignment, nNumberOfTabStops = 0;
                rIn.ReadUInt16( nNumberOfTabStops );
                if (!rIn.good())
                {
                    // TODO?
                }
                else
                {
                    const size_t nMinRecordSize = 4;
                    const size_t nMaxRecords = rIn.remainingSize() / nMinRecordSize;
                    if (nNumberOfTabStops > nMaxRecords)
                    {
                        SAL_WARN("filter.ms", "Parsing error: " << nMaxRecords <<
                                 " max possible entries, but " << nNumberOfTabStops << " claimed, truncating");
                        nNumberOfTabStops = nMaxRecords;
                    }
                    for (i = 0; i < nNumberOfTabStops; ++i)
                    {
                        rIn.ReadUInt16( nDistance )
                           .ReadUInt16( nAlignment );
                    }
                }
            }
            if ( nMask & 0x10000 )  // pfBaseLine
            {
                rIn.ReadUInt16( nDummy16 );
                if (!rIn.good())
                {
                    // TODO?
                }
            }
            if ( nMask & 0xe0000 )  // pfCharWrap, pfWordWrap, pfOverflow
            {
                rIn.ReadUInt16( nDummy16 );
                if (!rIn.good())
                {   // clear flag to avoid invalid access
                    aSet.mnAttrSet &= ~((1 << PPT_ParaAttr_AsianLB_1)
                                      | (1 << PPT_ParaAttr_AsianLB_2)
                                      | (1 << PPT_ParaAttr_AsianLB_3));
                }
                else
                {
                    if (nMask & 0x20000)
                        aSet.mpArry[PPT_ParaAttr_AsianLB_1] = nDummy16 & 1;
                    if (nMask & 0x40000)
                        aSet.mpArry[PPT_ParaAttr_AsianLB_2] = (nDummy16 >> 1) & 1;
                    if (nMask & 0x80000)
                        aSet.mpArry[PPT_ParaAttr_AsianLB_3] = (nDummy16 >> 2) & 1;
                    aSet.mnAttrSet |= ((nMask >> 17) & 7) << PPT_ParaAttr_AsianLB_1;
                }
            }
            if ( nMask & 0x200000 ) // pfTextDirection
            {
                rIn.ReadUInt16( aSet.mpArry[ PPT_ParaAttr_BiDi ] );
                if (!rIn.good())
                {   // clear flag to avoid invalid access
                    aSet.mnAttrSet &= ~(1 << PPT_ParaAttr_BiDi);
                }
            }
        }
        else
            nCharCount = nStringLen;

        //if the textofs attr has been read at above, need not to reset.
        if ( ( !( aSet.mnAttrSet & 1 << PPT_ParaAttr_TextOfs ) ) && rRuler.GetTextOfs( aParaPropSet.pParaSet->mnDepth, aSet.mpArry[ PPT_ParaAttr_TextOfs ] ) )
            aSet.mnAttrSet |= 1 << PPT_ParaAttr_TextOfs;
        if ( ( !( aSet.mnAttrSet & 1 << PPT_ParaAttr_BulletOfs ) ) && rRuler.GetBulletOfs( aParaPropSet.pParaSet->mnDepth, aSet.mpArry[ PPT_ParaAttr_BulletOfs ] ) )
            aSet.mnAttrSet |= 1 << PPT_ParaAttr_BulletOfs;
        if ( rRuler.GetDefaultTab( aParaPropSet.pParaSet->mnDepth, aSet.mpArry[ PPT_ParaAttr_DefaultTab ] ) )
            aSet.mnAttrSet |= 1 << PPT_ParaAttr_DefaultTab;

        if ( ( nCharCount > nStringLen ) || ( nStringLen < nCharAnzRead + nCharCount ) )
        {
            bTextPropAtom = false;
            nCharCount = nStringLen - nCharAnzRead;
            // please fix the right hand side of
            // PPTParaPropSet& PPTParaPropSet::operator=(PPTParaPropSet&),
            // it should be a const reference
            PPTParaPropSet aTmpPPTParaPropSet;
            aParaPropSet = aTmpPPTParaPropSet;
            OSL_FAIL( "SJ:PPTStyleTextPropReader::could not get this PPT_PST_StyleTextPropAtom by reading the paragraph attributes" );
        }
        PPTParaPropSet* pPara = new PPTParaPropSet( aParaPropSet );
        pPara->mnOriginalTextPos = nCharAnzRead;
        aParaPropList.push_back( pPara );
        if ( nCharCount )
        {
            sal_uInt32   nCount;
            const sal_Unicode* pDat = aString.getStr() + nCharAnzRead;
            for ( nCount = 0; nCount < nCharCount; nCount++ )
            {
                if ( pDat[ nCount ] == 0xd )
                {
                    pPara = new PPTParaPropSet( aParaPropSet );
                    pPara->mnOriginalTextPos = nCharAnzRead + nCount + 1;
                    aParaPropList.push_back( pPara );
                }
            }
        }
        nCharAnzRead += nCharCount + 1;
    }
}

void PPTStyleTextPropReader::ReadCharProps( SvStream& rIn, PPTCharPropSet& aCharPropSet, const OUString& aString,
                                            sal_uInt32& nCharCount, sal_uInt32 nCharAnzRead,
                                            bool& bTextPropAtom, sal_uInt32 nExtParaPos,
                                            const std::vector< StyleTextProp9 >& aStyleTextProp9,
                                            sal_uInt32& nExtParaFlags, sal_uInt16& nBuBlip,
                                            sal_uInt16& nHasAnm, sal_uInt32& nAnmScheme )
{
    sal_uInt32  nMask = 0; //TODO: nMask initialized here to suppress warning for now, see corresponding TODO below
    sal_uInt16  nDummy16;
    sal_Int32   nCharsToRead;
    sal_uInt16  nStringLen = aString.getLength();

    rIn.ReadUInt16( nDummy16 );
    nCharCount = (rIn.good()) ? nDummy16 : 0;
    rIn.ReadUInt16( nDummy16 );
    nCharsToRead = nStringLen - ( nCharAnzRead + nCharCount );
    if ( nCharsToRead < 0 )
    {
        nCharCount = nStringLen - nCharAnzRead;
        if ( nCharsToRead < -1 )
        {
            bTextPropAtom = false;
            OSL_FAIL( "SJ:PPTStyleTextPropReader::could not get this PPT_PST_StyleTextPropAtom by reading the character attributes" );
        }
    }
    ImplPPTCharPropSet& aSet = *aCharPropSet.mpImplPPTCharPropSet;

    // character attributes
    rIn.ReadUInt32( nMask );
    if ( (sal_uInt16)nMask )
    {
        aSet.mnAttrSet |= (sal_uInt16)nMask;
        rIn.ReadUInt16( aSet.mnFlags );
    }
    if ( nMask & 0x10000 )  // cfTypeface
    {
        rIn.ReadUInt16( aSet.mnFont );
        aSet.mnAttrSet |= 1 << PPT_CharAttr_Font;
    }
    if ( nMask & 0x200000 ) // cfFEOldTypeface
    {
        rIn.ReadUInt16( aSet.mnAsianOrComplexFont );
        aSet.mnAttrSet |= 1 << PPT_CharAttr_AsianOrComplexFont;
    }
    if ( nMask & 0x400000 ) // cfANSITypeface
    {
        rIn.ReadUInt16( aSet.mnANSITypeface );
        aSet.mnAttrSet |= 1 << PPT_CharAttr_ANSITypeface;
    }
    if ( nMask & 0x800000 ) // cfSymbolTypeface
    {
        rIn.ReadUInt16( aSet.mnSymbolFont );
        aSet.mnAttrSet |= 1 << PPT_CharAttr_Symbol;
    }
    if ( nMask & 0x20000 )  // cfSize
    {
        rIn.ReadUInt16( aSet.mnFontHeight );
        aSet.mnAttrSet |= 1 << PPT_CharAttr_FontHeight;
    }
    if ( nMask & 0x40000 )  // cfColor
    {
        sal_uInt32 nVal;
        rIn.ReadUInt32( nVal );
        if ( !( nVal & 0xff000000 ) )
            nVal = PPT_COLSCHEME_HINTERGRUND;
        aSet.mnColor = nVal;
        aSet.mnAttrSet |= 1 << PPT_CharAttr_FontColor;
    }
    if ( nMask & 0x80000 )  // cfPosition
    {
        rIn.ReadUInt16( aSet.mnEscapement );
        aSet.mnAttrSet |= 1 << PPT_CharAttr_Escapement;
    }
    if ( nExtParaPos )
    {
        sal_uInt32 nExtBuInd = nMask & 0x3c00;
        if ( nExtBuInd )
            nExtBuInd = ( aSet.mnFlags & 0x3c00 ) >> 10;
        if ( nExtBuInd < aStyleTextProp9.size() )
        {
            nExtParaFlags = aStyleTextProp9[ nExtBuInd ].mnExtParagraphMask;
            nBuBlip = aStyleTextProp9[ nExtBuInd ].mnBuBlip;
            nHasAnm = aStyleTextProp9[ nExtBuInd ].mnHasAnm;
            nAnmScheme = aStyleTextProp9[ nExtBuInd ].mnAnmScheme;
        }
    }
}

void PPTStyleTextPropReader::Init( SvStream& rIn, const DffRecordHeader& rTextHeader,
                                   PPTTextRulerInterpreter& rRuler, const DffRecordHeader& rExtParaHd, TSS_Type nInstance )
{
    sal_uInt32 nMerk = rIn.Tell();
    sal_uInt32 nExtParaPos = ( rExtParaHd.nRecType == PPT_PST_ExtendedParagraphAtom ) ? rExtParaHd.nFilePos + 8 : 0;

    std::vector< StyleTextProp9 > aStyleTextProp9;
    if ( rExtParaHd.nRecType == PPT_PST_ExtendedParagraphAtom  )
    {
        rIn.Seek( rExtParaHd.nFilePos + 8 );

        auto nEndRecPos = DffPropSet::SanitizeEndPos(rIn, rExtParaHd.GetRecEndFilePos());
        while( ( rIn.GetError() == 0 ) && ( rIn.Tell() < nEndRecPos ) )
        {
            aStyleTextProp9.resize( aStyleTextProp9.size() + 1 );
            aStyleTextProp9.back().Read( rIn );
        }
        rIn.Seek( nMerk );
    }

    OUString aString;
    DffRecordHeader aTextHd;
    ReadDffRecordHeader( rIn, aTextHd );
    sal_uInt32 nMaxLen = aTextHd.nRecLen;
    if ( nMaxLen >= 0xFFFF )
        nMaxLen = 0xFFFE;

    if( aTextHd.nRecType == PPT_PST_TextCharsAtom )
    {
        sal_uInt32 i;
        sal_Unicode nChar;
        std::unique_ptr<sal_Unicode[]> pBuf(new sal_Unicode[ ( nMaxLen >> 1 ) + 1 ]);
        rIn.ReadBytes(pBuf.get(), nMaxLen);
        nMaxLen >>= 1;
        pBuf[ nMaxLen ] = 0;
        sal_Unicode* pPtr = pBuf.get();
#ifdef OSL_BIGENDIAN
        sal_Unicode nTemp;
        for ( i = 0; i < nMaxLen; i++ )
        {
            nTemp = *pPtr;
            *pPtr++ = ( nTemp << 8 ) | ( nTemp >> 8 );
        }
        pPtr = pBuf.get();
#endif

        for ( i = 0; i < nMaxLen; pPtr++, i++ )
        {
            nChar = *pPtr;
            if ( !nChar )
                break;
            if ( ( nChar & 0xff00 ) == 0xf000 )         // in this special case we got a symbol
                aSpecMarkerList.push_back( (sal_uInt32)( i | PPT_SPEC_SYMBOL ) );
            else if ( nChar == 0xd )
            {
                if ( nInstance == TSS_Type::PageTitle )
                    *pPtr = 0xb;
                else
                    aSpecMarkerList.push_back( (sal_uInt32)( i | PPT_SPEC_NEWLINE ) );
            }
        }
        if ( i )
            aString = OUString(pBuf.get(), i);
    }
    else if( aTextHd.nRecType == PPT_PST_TextBytesAtom )
    {
        std::unique_ptr<sal_Char[]> pBuf(new sal_Char[ nMaxLen + 1 ]);
        pBuf[ nMaxLen ] = 0;
        rIn.ReadBytes(pBuf.get(), nMaxLen);
        sal_Char* pPtr = pBuf.get();
        for (;;)
        {
            sal_Char cLo = *pPtr;
            if ( cLo == 0 )
                break;
            if ( cLo == 0xd )
            {
                if ( nInstance == TSS_Type::PageTitle )
                    *pPtr = 0xb;
                else
                    aSpecMarkerList.push_back( (sal_uInt32)( (pPtr - pBuf.get()) | PPT_SPEC_NEWLINE ) );
            }
            pPtr++;
        }
        sal_Int32 nLen = pPtr - pBuf.get();
        if ( nLen )
            aString = OUString( pBuf.get(), nLen, RTL_TEXTENCODING_MS_1252 );
    }
    else
    {
        // no chars, but potentially char/para props?
        sal_uInt32  nCharCount;
        bool        bTextPropAtom = false;
        ReadParaProps( rIn, rTextHeader, aString, rRuler, nCharCount, bTextPropAtom );

        if ( bTextPropAtom )
        {
            // yeah, StyleTextProp is there, read it all & push to
            // aParaPropList
            PPTCharPropSet aCharPropSet(0);
            aCharPropSet.mnOriginalTextPos = 0;

            sal_uInt32 nCharAnzRead = 0;
            sal_uInt32 nExtParaFlags = 0, nAnmScheme = 0;
            sal_uInt16 nBuBlip = 0xffff, nHasAnm = 0;
            ReadCharProps( rIn, aCharPropSet, aString, nCharCount, nCharAnzRead,
                           bTextPropAtom, nExtParaPos, aStyleTextProp9, nExtParaFlags,
                           nBuBlip, nHasAnm, nAnmScheme );

            aCharPropList.push_back( new PPTCharPropSet( aCharPropSet, 0 ) );
        }
    }

    if ( !aString.isEmpty() )
    {
        sal_uInt32  nCharCount;
        bool        bTextPropAtom = false;

        ReadParaProps( rIn, rTextHeader, aString, rRuler, nCharCount, bTextPropAtom );

        bool bEmptyParaPossible = true;
        sal_uInt32 nCharAnzRead = 0;
        sal_uInt32 nCurrentPara = 0;
        size_t i = 1;                   // points to the next element to process
        sal_uInt32 nCurrentSpecMarker = aSpecMarkerList.empty() ? 0 : aSpecMarkerList[0];
        sal_uInt32 nStringLen = aString.getLength();

        while ( nCharAnzRead < nStringLen )
        {
            sal_uInt32 nExtParaFlags = 0, nLatestParaUpdate = 0xffffffff, nAnmScheme = 0;
            sal_uInt16 nBuBlip = 0xffff, nHasAnm = 0;

            PPTCharPropSet aCharPropSet( nCurrentPara );
            if ( bTextPropAtom )
            {
                ReadCharProps( rIn, aCharPropSet, aString, nCharCount, nCharAnzRead,
                               bTextPropAtom, nExtParaPos, aStyleTextProp9, nExtParaFlags,
                               nBuBlip, nHasAnm, nAnmScheme );
                if (!rIn.good())
                    break;
            }
            else
                nCharCount = nStringLen;

            sal_uInt32 nLen;
            while( nCharCount )
            {
                if ( nExtParaPos && ( nLatestParaUpdate != nCurrentPara ) && ( nCurrentPara < aParaPropList.size() ) )
                {
                    PPTParaPropSet* pPropSet = aParaPropList[ nCurrentPara ];
                    pPropSet->pParaSet->mnExtParagraphMask = nExtParaFlags;
                    if ( nExtParaFlags & 0x800000 )
                        pPropSet->pParaSet->mnBuBlip = nBuBlip;
                    if ( nExtParaFlags & 0x01000000 )
                        pPropSet->pParaSet->mnAnmScheme = nAnmScheme;
                    if ( nExtParaFlags & 0x02000000 )
                        pPropSet->pParaSet->mnHasAnm = nHasAnm;
                    nLatestParaUpdate = nCurrentPara;
                }
                aCharPropSet.mnOriginalTextPos = nCharAnzRead;
                if ( nCurrentSpecMarker &&  ( ( nCurrentSpecMarker & 0xffff ) < ( nCharAnzRead + nCharCount ) ) )
                {
                    if ( nCurrentSpecMarker & PPT_SPEC_NEWLINE )
                    {
                        nLen = ( nCurrentSpecMarker & 0xffff ) - nCharAnzRead;
                        if ( nLen )
                            aCharPropSet.maString = aString.copy( nCharAnzRead, nLen );
                        else if ( bEmptyParaPossible )
                            aCharPropSet.maString.clear();
                        if ( nLen || bEmptyParaPossible )
                            aCharPropList.push_back( new PPTCharPropSet( aCharPropSet, nCurrentPara ) );
                        nCurrentPara++;
                        nLen++;
                        nCharAnzRead += nLen;
                        nCharCount -= nLen;
                        bEmptyParaPossible = true;
                    }
                    else if ( nCurrentSpecMarker & PPT_SPEC_SYMBOL )
                    {
                        if ( ( nCurrentSpecMarker & 0xffff ) != nCharAnzRead )
                        {
                            nLen = ( nCurrentSpecMarker & 0xffff ) - nCharAnzRead;
                            aCharPropSet.maString = aString.copy(nCharAnzRead, nLen);
                            aCharPropList.push_back( new PPTCharPropSet( aCharPropSet, nCurrentPara ) );
                            nCharCount -= nLen;
                            nCharAnzRead += nLen;
                        }
                        PPTCharPropSet* pCPropSet = new PPTCharPropSet( aCharPropSet, nCurrentPara );
                        pCPropSet->maString = aString.copy(nCharAnzRead, 1);
                        if ( aCharPropSet.mpImplPPTCharPropSet->mnAttrSet & ( 1 << PPT_CharAttr_Symbol ) )
                            pCPropSet->SetFont( aCharPropSet.mpImplPPTCharPropSet->mnSymbolFont );
                        aCharPropList.push_back( pCPropSet );
                        nCharCount--;
                        nCharAnzRead++;
                        bEmptyParaPossible = false;
                    }
                    nCurrentSpecMarker = ( i < aSpecMarkerList.size() ) ? aSpecMarkerList[ i++ ] : 0;
                }
                else
                {
                    if (nCharAnzRead > static_cast<sal_uInt32>(aString.getLength()))
                        aCharPropSet.maString = OUString();
                    else
                    {
                        sal_Int32 nStrLen = nCharCount;
                        sal_Int32 nMaxStrLen = aString.getLength() - nCharAnzRead;
                        if (nStrLen > nMaxStrLen)
                            nStrLen = nMaxStrLen;
                        aCharPropSet.maString = aString.copy(nCharAnzRead, nStrLen);
                    }
                    aCharPropList.push_back( new PPTCharPropSet( aCharPropSet, nCurrentPara ) );
                    nCharAnzRead += nCharCount;
                    bEmptyParaPossible = false;
                    break;
                }
            }
        }
        if ( !aCharPropList.empty() && ( aCharPropList.back()->mnParagraph != nCurrentPara ) )
        {
            PPTCharPropSet* pCharPropSet = new PPTCharPropSet( *aCharPropList.back(), nCurrentPara );
            (pCharPropSet->maString).clear();
            pCharPropSet->mnOriginalTextPos = nStringLen - 1;
            aCharPropList.push_back( pCharPropSet );
        }
    }
    rIn.Seek( nMerk );
}

PPTStyleTextPropReader::~PPTStyleTextPropReader()
{
    for ( PPTParaPropSetList::const_iterator it = aParaPropList.begin(); it != aParaPropList.end(); ++it )
        delete *it;
    for ( PPTCharPropSetList::const_iterator it = aCharPropList.begin(); it != aCharPropList.end(); ++it )
        delete *it;
}

PPTPortionObj::PPTPortionObj( const PPTStyleSheet& rStyleSheet, TSS_Type nInstance, sal_uInt32 nDepth ) :
    PPTCharPropSet  ( 0 ),
    mrStyleSheet    ( rStyleSheet ),
    mnInstance      ( nInstance ),
    mnDepth         ( ( nDepth > 4 ) ? 4 : nDepth )
{
}

PPTPortionObj::PPTPortionObj( const PPTCharPropSet& rCharPropSet, const PPTStyleSheet& rStyleSheet, TSS_Type nInstance, sal_uInt32 nDepth ) :
    PPTCharPropSet  ( rCharPropSet ),
    mrStyleSheet    ( rStyleSheet ),
    mnInstance      ( nInstance ),
    mnDepth         ( ( nDepth > 4 ) ? 4 : nDepth )
{
}

PPTPortionObj::PPTPortionObj( const PPTPortionObj& rPortionObj ) :
    PPTCharPropSet      ( rPortionObj ),
    mrStyleSheet        ( rPortionObj.mrStyleSheet ),
    mnInstance          ( rPortionObj.mnInstance ),
    mnDepth             ( rPortionObj.mnDepth )
{
}

PPTPortionObj::~PPTPortionObj()
{
}

bool PPTPortionObj::HasTabulator()
{
    bool bRetValue =    false;
    sal_Int32           nCount;
    const sal_Unicode*  pPtr = maString.getStr();
    for ( nCount = 0; nCount < maString.getLength(); nCount++ )
    {
        if ( pPtr[ nCount ] == 0x9 )
        {
            bRetValue = true;
            break;
        }

    }
    return bRetValue;
}

bool PPTPortionObj::GetAttrib( sal_uInt32 nAttr, sal_uInt32& rRetValue, TSS_Type nDestinationInstance ) const
{
    sal_uInt32  nMask = 1 << nAttr;
    rRetValue = 0;

    bool bIsHardAttribute = ( ( mpImplPPTCharPropSet->mnAttrSet & nMask ) != 0 );

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
                rRetValue = ( mpImplPPTCharPropSet->mnFlags & nMask ) ? 1 : 0;
            break;
            case PPT_CharAttr_Font :
                rRetValue = mpImplPPTCharPropSet->mnFont;
            break;
            case PPT_CharAttr_AsianOrComplexFont :
                rRetValue = mpImplPPTCharPropSet->mnAsianOrComplexFont;
            break;
            case PPT_CharAttr_FontHeight :
                rRetValue = mpImplPPTCharPropSet->mnFontHeight;
            break;
            case PPT_CharAttr_FontColor :
                rRetValue = mpImplPPTCharPropSet->mnColor;
            break;
            case PPT_CharAttr_Escapement :
                rRetValue = mpImplPPTCharPropSet->mnEscapement;
            break;
            default :
                OSL_FAIL( "SJ:PPTPortionObj::GetAttrib ( hard attribute does not exist )" );
        }
    }
    else
    {
        const PPTCharLevel& rCharLevel = mrStyleSheet.mpCharSheet[ mnInstance ]->maCharLevel[ mnDepth ];
        PPTCharLevel* pCharLevel = nullptr;
        if ( ( nDestinationInstance == TSS_Type::Unknown )
                || ( mnDepth && ( ( mnInstance == TSS_Type::Subtitle ) || ( mnInstance == TSS_Type::TextInShape ) ) ) )
            bIsHardAttribute = true;
        else if ( nDestinationInstance != mnInstance )
            pCharLevel = &mrStyleSheet.mpCharSheet[ nDestinationInstance ]->maCharLevel[ mnDepth ];
        switch( nAttr )
        {
            case PPT_CharAttr_Bold :
            case PPT_CharAttr_Italic :
            case PPT_CharAttr_Underline :
            case PPT_CharAttr_Shadow :
            case PPT_CharAttr_Strikeout :
            case PPT_CharAttr_Embossed :
            {
                rRetValue = ( rCharLevel.mnFlags & nMask ) ? 1 : 0;
                if ( pCharLevel )
                {
                    sal_uInt32 nTmp = ( pCharLevel->mnFlags & nMask ) ? 1 : 0;
                    if ( rRetValue != nTmp )
                        bIsHardAttribute = true;
                }
            }
            break;
            case PPT_CharAttr_Font :
            {
                rRetValue = rCharLevel.mnFont;
                if ( pCharLevel && ( rRetValue != pCharLevel->mnFont ) )
                    bIsHardAttribute = true;
            }
            break;
            case PPT_CharAttr_AsianOrComplexFont :
            {
                rRetValue = rCharLevel.mnAsianOrComplexFont;
                if ( pCharLevel && ( rRetValue != pCharLevel->mnAsianOrComplexFont ) )
                    bIsHardAttribute = true;
            }
            break;
            case PPT_CharAttr_FontHeight :
            {
                rRetValue = rCharLevel.mnFontHeight;
                if ( pCharLevel && ( rRetValue != pCharLevel->mnFontHeight ) )
                    bIsHardAttribute = true;
            }
            break;
            case PPT_CharAttr_FontColor :
            {
                rRetValue = rCharLevel.mnFontColor;
                if ( pCharLevel && ( rRetValue != pCharLevel->mnFontColor ) )
                    bIsHardAttribute = true;
            }
            break;
            case PPT_CharAttr_Escapement :
            {
                rRetValue = rCharLevel.mnEscapement;
                if ( pCharLevel && ( rRetValue != pCharLevel->mnEscapement ) )
                    bIsHardAttribute = true;
            }
            break;
            default :
                OSL_FAIL( "SJ:PPTPortionObj::GetAttrib ( attribute does not exist )" );
        }
    }
    return bIsHardAttribute;
}

void PPTPortionObj::ApplyTo(  SfxItemSet& rSet, SdrPowerPointImport& rManager, TSS_Type nDestinationInstance )
{
    ApplyTo( rSet, rManager, nDestinationInstance, nullptr );
}

void PPTPortionObj::ApplyTo(  SfxItemSet& rSet, SdrPowerPointImport& rManager, TSS_Type nDestinationInstance, const PPTTextObj* pTextObj )
{
    sal_uInt32  nVal;
    if ( GetAttrib( PPT_CharAttr_Bold, nVal, nDestinationInstance ) )
    {
        rSet.Put( SvxWeightItem( nVal != 0 ? WEIGHT_BOLD : WEIGHT_NORMAL, EE_CHAR_WEIGHT ) );
        rSet.Put( SvxWeightItem( nVal != 0 ? WEIGHT_BOLD : WEIGHT_NORMAL, EE_CHAR_WEIGHT_CJK ) );
        rSet.Put( SvxWeightItem( nVal != 0 ? WEIGHT_BOLD : WEIGHT_NORMAL, EE_CHAR_WEIGHT_CTL ) );
    }
    if ( GetAttrib( PPT_CharAttr_Italic, nVal, nDestinationInstance ) )
    {
        rSet.Put( SvxPostureItem( nVal != 0 ? ITALIC_NORMAL : ITALIC_NONE, EE_CHAR_ITALIC ) );
        rSet.Put( SvxPostureItem( nVal != 0 ? ITALIC_NORMAL : ITALIC_NONE, EE_CHAR_ITALIC_CJK ) );
        rSet.Put( SvxPostureItem( nVal != 0 ? ITALIC_NORMAL : ITALIC_NONE, EE_CHAR_ITALIC_CTL ) );
    }
    if ( GetAttrib( PPT_CharAttr_Underline, nVal, nDestinationInstance ) )
        rSet.Put( SvxUnderlineItem( nVal != 0 ? LINESTYLE_SINGLE : LINESTYLE_NONE, EE_CHAR_UNDERLINE ) );

    if ( GetAttrib( PPT_CharAttr_Shadow, nVal, nDestinationInstance ) )
        rSet.Put( SvxShadowedItem( nVal != 0, EE_CHAR_SHADOW ) );

    if ( GetAttrib( PPT_CharAttr_Strikeout, nVal, nDestinationInstance ) )
        rSet.Put( SvxCrossedOutItem( nVal != 0 ? STRIKEOUT_SINGLE : STRIKEOUT_NONE, EE_CHAR_STRIKEOUT ) );

    sal_uInt32  nAsianFontId = 0xffff;
    if ( GetAttrib( PPT_CharAttr_AsianOrComplexFont, nAsianFontId, nDestinationInstance ) )
    {
        if ( nAsianFontId != 0xffff )
        {
            PptFontEntityAtom* pFontEnityAtom = rManager.GetFontEnityAtom( nAsianFontId );
            if ( pFontEnityAtom )
            {
                rSet.Put( SvxFontItem( pFontEnityAtom->eFamily, pFontEnityAtom->aName,
                            OUString(), pFontEnityAtom->ePitch, pFontEnityAtom->eCharSet, EE_CHAR_FONTINFO_CJK ) );
                rSet.Put( SvxFontItem( pFontEnityAtom->eFamily, pFontEnityAtom->aName,
                            OUString(), pFontEnityAtom->ePitch, pFontEnityAtom->eCharSet, EE_CHAR_FONTINFO_CTL ) );
            }
        }
    }
    if ( GetAttrib( PPT_CharAttr_Font, nVal, nDestinationInstance ) )
    {
        PptFontEntityAtom* pFontEnityAtom = rManager.GetFontEnityAtom( nVal );
        if ( pFontEnityAtom )
        {
            rSet.Put( SvxFontItem( pFontEnityAtom->eFamily, pFontEnityAtom->aName, OUString(), pFontEnityAtom->ePitch, pFontEnityAtom->eCharSet, EE_CHAR_FONTINFO ) );

            // #i119475# bullet font info for CJK and CTL
            if ( RTL_TEXTENCODING_SYMBOL ==  pFontEnityAtom->eCharSet )
            {
                rSet.Put( SvxFontItem( pFontEnityAtom->eFamily, pFontEnityAtom->aName, OUString(), pFontEnityAtom->ePitch, pFontEnityAtom->eCharSet, EE_CHAR_FONTINFO_CJK ) );
                rSet.Put( SvxFontItem( pFontEnityAtom->eFamily, pFontEnityAtom->aName, OUString(), pFontEnityAtom->ePitch, pFontEnityAtom->eCharSet, EE_CHAR_FONTINFO_CTL ) );
            }
        }
    }
    if ( GetAttrib( PPT_CharAttr_FontHeight, nVal, nDestinationInstance ) ) // Schriftgrad in Point
    {
        sal_uInt32 nHeight = rManager.ScalePoint( nVal );
        rSet.Put( SvxFontHeightItem( nHeight, 100, EE_CHAR_FONTHEIGHT ) );
        rSet.Put( SvxFontHeightItem( nHeight, 100, EE_CHAR_FONTHEIGHT_CJK ) );
        rSet.Put( SvxFontHeightItem( nHeight, 100, EE_CHAR_FONTHEIGHT_CTL ) );
    }

    if ( GetAttrib( PPT_CharAttr_Embossed, nVal, nDestinationInstance ) )
        rSet.Put( SvxCharReliefItem( nVal != 0 ? FontRelief::Embossed : FontRelief::NONE, EE_CHAR_RELIEF ) );
    if ( nVal ) /* if Embossed is set, the font color depends to the fillstyle/color of the object,
                   if the object has no fillstyle, the font color depends to fillstyle of the background */
    {
        Color aDefColor( COL_BLACK );
        MSO_FillType eFillType = mso_fillSolid;
        if ( rManager.GetPropertyValue( DFF_Prop_fNoFillHitTest, 0 ) & 0x10 )
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
                aDefColor = rManager.MSO_CLR_ToColor( rManager.GetPropertyValue( DFF_Prop_fillColor, 0 ) );
            break;
            case mso_fillPattern :
                aDefColor = rManager.MSO_CLR_ToColor( rManager.GetPropertyValue( DFF_Prop_fillBackColor, 0 ) );
            break;
            case mso_fillTexture :
            {
                Graphic aGraf;
                if ( rManager.GetBLIP( rManager.GetPropertyValue( DFF_Prop_fillBlip, 0 ), aGraf ) )
                {
                    Bitmap aBmp( aGraf.GetBitmap() );
                    Size aSize( aBmp.GetSizePixel() );
                    if ( aSize.Width() && aSize.Height() )
                    {
                        if ( aSize.Width () > 64 )
                            aSize.Width () = 64;
                        if ( aSize.Height() > 64 )
                            aSize.Height() = 64;

                        Bitmap::ScopedReadAccess pAcc(aBmp);
                        if( pAcc )
                        {
                            sal_uLong nRt = 0, nGn = 0, nBl = 0;
                            const long nWidth = aSize.Width();
                            const long nHeight = aSize.Height();

                            if( pAcc->HasPalette() )
                            {
                                for( long nY = 0L; nY < nHeight; nY++ )
                                {
                                    for( long nX = 0L; nX < nWidth; nX++ )
                                    {
                                        const BitmapColor& rCol = pAcc->GetPaletteColor( pAcc->GetPixelIndex( nY, nX ) );
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
                            pAcc.reset();
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
                        const SfxPoolItem* pFillStyleItem = nullptr;
                        pItemSet->GetItemState( XATTR_FILLSTYLE, false, &pFillStyleItem );
                        if ( pFillStyleItem )
                        {
                            drawing::FillStyle eFillStyle = static_cast<const XFillStyleItem*>(pFillStyleItem)->GetValue();
                            switch( eFillStyle )
                            {
                                case drawing::FillStyle_SOLID :
                                {
                                    const SfxPoolItem* pFillColorItem = nullptr;
                                    pItemSet->GetItemState( XATTR_FILLCOLOR, false, &pFillColorItem );
                                    if ( pFillColorItem )
                                        aDefColor = static_cast<const XColorItem*>(pFillColorItem)->GetColorValue();
                                }
                                break;
                                case drawing::FillStyle_GRADIENT :
                                {
                                    const SfxPoolItem* pGradientItem = nullptr;
                                    pItemSet->GetItemState( XATTR_FILLGRADIENT, false, &pGradientItem );
                                    if ( pGradientItem )
                                        aDefColor = static_cast<const XFillGradientItem*>(pGradientItem)->GetGradientValue().GetStartColor();
                                }
                                break;
                                case drawing::FillStyle_HATCH :
                                case drawing::FillStyle_BITMAP :
                                    aDefColor = Color( COL_WHITE );
                                break;
                                default: break;
                            }
                        }
                    }
                }
            }
            break;
            default: break;
        }
        rSet.Put( SvxColorItem( aDefColor, EE_CHAR_COLOR ) );
    }
    else
    {
        if ( GetAttrib( PPT_CharAttr_FontColor, nVal, nDestinationInstance ) )  // text color (4Byte-Arg)
        {
            Color aCol( rManager.MSO_TEXT_CLR_ToColor( nVal ) );
            rSet.Put( SvxColorItem( aCol, EE_CHAR_COLOR ) );
            if ( nDestinationInstance == TSS_Type::Unknown )
                mrStyleSheet.mpCharSheet[ mnInstance ]->maCharLevel[ mnDepth ].mnFontColorInStyleSheet = aCol;
        }
        else if ( nVal & 0x0f000000 )   // this is not a hard attribute, but maybe the page has a different colorscheme,
        {                               // so that in this case we must use a hard color attribute
            Color   aCol( rManager.MSO_TEXT_CLR_ToColor( nVal ) );
            Color&  aColorInSheet = mrStyleSheet.mpCharSheet[ mnInstance ]->maCharLevel[ mnDepth ].mnFontColorInStyleSheet;
            if ( aColorInSheet != aCol )
                rSet.Put( SvxColorItem( aCol, EE_CHAR_COLOR ) );
        }
    }

    if ( GetAttrib( PPT_CharAttr_Escapement, nVal, nDestinationInstance ) ) // super-/subscript in %
    {
        sal_uInt16  nEsc = 0;
        sal_uInt8   nProp = 100;

        if ( nVal )
        {
            nEsc = (sal_Int16)nVal;
            nProp = DFLT_ESC_PROP;
        }
        SvxEscapementItem aItem( nEsc, nProp, EE_CHAR_ESCAPEMENT );
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
    return nullptr;
}

PPTParagraphObj::PPTParagraphObj( const PPTStyleSheet& rStyleSheet, TSS_Type nInstance, sal_uInt16 nDepth ) :
    PPTNumberFormatCreator  ( nullptr ),
    mrStyleSheet            ( rStyleSheet ),
    mnInstance              ( nInstance ),
    mbTab                   ( true ),      // style sheets always have to get the right tabulator setting
    mnCurrentObject         ( 0 )
{
    if ( nDepth > 4 )
        nDepth = 4;
    pParaSet->mnDepth = nDepth;
}

PPTParagraphObj::PPTParagraphObj( PPTStyleTextPropReader& rPropReader,
        size_t const nCurParaPos, size_t& rnCurCharPos,
        const PPTStyleSheet& rStyleSheet,
        TSS_Type nInstance, PPTTextRulerInterpreter& rRuler ) :
    PPTParaPropSet          ( *rPropReader.aParaPropList[nCurParaPos] ),
    PPTNumberFormatCreator  ( nullptr ),
    PPTTextRulerInterpreter ( rRuler ),
    mrStyleSheet            ( rStyleSheet ),
    mnInstance              ( nInstance ),
    mbTab                   ( false ),
    mnCurrentObject         ( 0 )
{
    if (rnCurCharPos < rPropReader.aCharPropList.size())
    {
        sal_uInt32 const nCurrentParagraph =
            rPropReader.aCharPropList[rnCurCharPos]->mnParagraph;
        for (; rnCurCharPos < rPropReader.aCharPropList.size() &&
             rPropReader.aCharPropList[rnCurCharPos]->mnParagraph == nCurrentParagraph;
             ++rnCurCharPos)
        {
            PPTCharPropSet *const pCharPropSet =
                rPropReader.aCharPropList[rnCurCharPos];
            std::unique_ptr<PPTPortionObj> pPPTPortion(new PPTPortionObj(
                    *pCharPropSet, rStyleSheet, nInstance, pParaSet->mnDepth));
            if (!mbTab)
            {
                mbTab = pPPTPortion->HasTabulator();
            }
            m_PortionList.push_back(std::move(pPPTPortion));
        }
    }
}

PPTParagraphObj::~PPTParagraphObj()
{
}

void PPTParagraphObj::AppendPortion( PPTPortionObj& rPPTPortion )
{
    m_PortionList.push_back(
            o3tl::make_unique<PPTPortionObj>(rPPTPortion));
    if ( !mbTab )
    {
        mbTab = m_PortionList.back()->HasTabulator();
    }
}

void PPTParagraphObj::UpdateBulletRelSize( sal_uInt32& nBulletRelSize ) const
{
    if ( nBulletRelSize > 0x7fff )      // a negative value is the absolute bullet height
    {
        sal_uInt16 nFontHeight = 0;
        if (!m_PortionList.empty())
        {
            PPTPortionObj const& rPortion = *m_PortionList.front();
            if (rPortion.mpImplPPTCharPropSet->mnAttrSet & (1 << PPT_CharAttr_FontHeight))
            {
                nFontHeight = rPortion.mpImplPPTCharPropSet->mnFontHeight;
            }
        }
        // if we do not have a hard attributed fontheight, the fontheight is taken from the style
        if ( !nFontHeight )
            nFontHeight = mrStyleSheet.mpCharSheet[ mnInstance ]->maCharLevel[ pParaSet->mnDepth ].mnFontHeight;
        nBulletRelSize = nFontHeight ? ((-((sal_Int16)nBulletRelSize)) * 100 ) / nFontHeight : 100;
    }
}

bool PPTParagraphObj::GetAttrib( sal_uInt32 nAttr, sal_uInt32& rRetValue, TSS_Type nDestinationInstance )
{
    sal_uInt32  nMask = 1 << nAttr;
    rRetValue = 0;

    if ( nAttr > 21 )
    {
        OSL_FAIL( "SJ:PPTParagraphObj::GetAttrib - attribute does not exist" );
        return false;
    }

    bool bIsHardAttribute = ( ( pParaSet->mnAttrSet & nMask ) != 0 );

    if ( bIsHardAttribute )
    {
        if ( nAttr == PPT_ParaAttr_BulletColor )
        {
            bool bHardBulletColor;
            if ( pParaSet->mnAttrSet & ( 1 << PPT_ParaAttr_BuHardColor ) )
                bHardBulletColor = pParaSet->mpArry[ PPT_ParaAttr_BuHardColor ] != 0;
            else
                bHardBulletColor = ( mrStyleSheet.mpParaSheet[ mnInstance ]->maParaLevel[ pParaSet->mnDepth ].mnBuFlags
                                        & ( 1 << PPT_ParaAttr_BuHardColor ) ) != 0;
            if ( bHardBulletColor )
                rRetValue = pParaSet->mnBulletColor;
            else
            {
                rRetValue = PPT_COLSCHEME_TEXT_UND_ZEILEN;
                if ((nDestinationInstance != TSS_Type::Unknown) && !m_PortionList.empty())
                {
                    PPTPortionObj const& rPortion = *m_PortionList.front();
                    if (rPortion.mpImplPPTCharPropSet->mnAttrSet & (1 << PPT_CharAttr_FontColor))
                    {
                        rRetValue = rPortion.mpImplPPTCharPropSet->mnColor;
                    }
                    else
                    {
                        rRetValue = mrStyleSheet.mpCharSheet[ nDestinationInstance ]->maCharLevel[ pParaSet->mnDepth ].mnFontColor;
                    }
                }
            }
        }
        else if ( nAttr == PPT_ParaAttr_BulletFont )
        {
            bool bHardBuFont;
            if ( pParaSet->mnAttrSet & ( 1 << PPT_ParaAttr_BuHardFont ) )
                bHardBuFont = pParaSet->mpArry[ PPT_ParaAttr_BuHardFont ] != 0;
            else
                bHardBuFont = ( mrStyleSheet.mpParaSheet[ mnInstance ]->maParaLevel[ pParaSet->mnDepth ].mnBuFlags
                                        & ( 1 << PPT_ParaAttr_BuHardFont ) ) != 0;
            if ( bHardBuFont )
                rRetValue = pParaSet->mpArry[ PPT_ParaAttr_BulletFont ];
            else
            {
                // it is the font used which assigned to the first character of the following text
                rRetValue = 0;
                if ((nDestinationInstance != TSS_Type::Unknown) && !m_PortionList.empty())
                {
                    PPTPortionObj const& rPortion = *m_PortionList.front();
                    if (rPortion.mpImplPPTCharPropSet->mnAttrSet & ( 1 << PPT_CharAttr_Font ) )
                    {
                        rRetValue = rPortion.mpImplPPTCharPropSet->mnFont;
                    }
                    else
                    {
                        rRetValue = mrStyleSheet.mpCharSheet[ nDestinationInstance ]->maCharLevel[ pParaSet->mnDepth ].mnFont;
                    }
                }
            }
        }
        else
            rRetValue = pParaSet->mpArry[ nAttr ];
    }
    else
    {
        const PPTParaLevel& rParaLevel = mrStyleSheet.mpParaSheet[ mnInstance ]->maParaLevel[ pParaSet->mnDepth ];

        PPTParaLevel* pParaLevel = nullptr;
        if ( ( nDestinationInstance == TSS_Type::Unknown )
            || ( pParaSet->mnDepth && ( ( mnInstance == TSS_Type::Subtitle ) || ( mnInstance == TSS_Type::TextInShape ) ) ) )
            bIsHardAttribute = true;
        else if ( nDestinationInstance != mnInstance )
            pParaLevel = &mrStyleSheet.mpParaSheet[ nDestinationInstance ]->maParaLevel[ pParaSet->mnDepth ];
        switch ( nAttr )
        {
            case PPT_ParaAttr_BulletOn :
            {
                rRetValue = rParaLevel.mnBuFlags & ( 1 << PPT_ParaAttr_BulletOn );
                if ( pParaLevel )
                {
                    if ( rRetValue != ( (sal_uInt32)pParaLevel->mnBuFlags & ( 1 << PPT_ParaAttr_BulletOn ) ) )
                        bIsHardAttribute = true;
                }
            }
            break;
            case PPT_ParaAttr_BuHardFont :
            case PPT_ParaAttr_BuHardColor :
            case PPT_ParaAttr_BuHardHeight :
                OSL_FAIL( "SJ:PPTParagraphObj::GetAttrib - this attribute does not make sense" );
            break;
            case PPT_ParaAttr_BulletChar :
            {
                rRetValue = rParaLevel.mnBulletChar;
                if ( pParaLevel && ( rRetValue != pParaLevel->mnBulletChar ) )
                    bIsHardAttribute = true;
            }
            break;
            case PPT_ParaAttr_BulletFont :
            {
                bool bHardBuFont;
                if ( pParaSet->mnAttrSet & ( 1 << PPT_ParaAttr_BuHardFont ) )
                    bHardBuFont = pParaSet->mpArry[ PPT_ParaAttr_BuHardFont ] != 0;
                else
                    bHardBuFont = ( rParaLevel.mnBuFlags & ( 1 << PPT_ParaAttr_BuHardFont ) ) != 0;
                if ( bHardBuFont )
                {
                    rRetValue = rParaLevel.mnBulletFont;
                    if ( pParaLevel && ( rRetValue != pParaLevel->mnBulletFont ) )
                        bIsHardAttribute = true;
                }
                else
                {
                    if (!m_PortionList.empty())
                    {
                        PPTPortionObj const& rPortion = *m_PortionList.front();
                        bIsHardAttribute = rPortion.GetAttrib(
                            PPT_CharAttr_Font, rRetValue, nDestinationInstance);
                    }
                    else
                    {
                        rRetValue = mrStyleSheet.mpCharSheet[ mnInstance ]->maCharLevel[ pParaSet->mnDepth ].mnFont;
                        bIsHardAttribute = true;
                    }
                }
            }
            break;
            case PPT_ParaAttr_BulletHeight :
            {
                rRetValue = rParaLevel.mnBulletHeight;
                if ( pParaLevel && ( rRetValue != pParaLevel->mnBulletHeight ) )
                    bIsHardAttribute = true;
            }
            break;
            case PPT_ParaAttr_BulletColor :
            {
                bool bHardBulletColor;
                if ( pParaSet->mnAttrSet & ( 1 << PPT_ParaAttr_BuHardColor ) )
                    bHardBulletColor = pParaSet->mpArry[ PPT_ParaAttr_BuHardColor ] != 0;
                else
                    bHardBulletColor = ( rParaLevel.mnBuFlags & ( 1 << PPT_ParaAttr_BuHardColor ) ) != 0;
                if ( bHardBulletColor )
                {
                    rRetValue = rParaLevel.mnBulletColor;
                    if ( pParaLevel && ( rRetValue != pParaLevel->mnBulletColor ) )
                        bIsHardAttribute = true;
                }
                else
                {
                    if (!m_PortionList.empty())
                    {
                        PPTPortionObj const& rPortion = *m_PortionList.front();
                        if (rPortion.mbIsHyperlink )
                        {
                            if( rPortion.mbHardHylinkOrigColor )
                                rRetValue = rPortion.mnHylinkOrigColor;
                            else
                                rRetValue = mrStyleSheet.mpCharSheet[ mnInstance ]->maCharLevel[ pParaSet->mnDepth ].mnFontColor;
                            bIsHardAttribute = true;
                        }
                        else
                        {
                            bIsHardAttribute = rPortion.GetAttrib( PPT_CharAttr_FontColor, rRetValue, nDestinationInstance );
                        }
                    }
                    else
                    {
                        rRetValue = mrStyleSheet.mpCharSheet[ mnInstance ]->maCharLevel[ pParaSet->mnDepth ].mnFontColor;
                        bIsHardAttribute = true;
                    }
                }
            }
            break;
            case PPT_ParaAttr_Adjust :
            {
                rRetValue = rParaLevel.mnAdjust;
                if ( pParaLevel && ( rRetValue != pParaLevel->mnAdjust ) )
                    bIsHardAttribute = true;
            }
            break;
            case PPT_ParaAttr_LineFeed :
            {
                rRetValue = rParaLevel.mnLineFeed;
                if ( pParaLevel && ( rRetValue != pParaLevel->mnLineFeed ) )
                    bIsHardAttribute = true;
            }
            break;
            case PPT_ParaAttr_UpperDist :
            {
                rRetValue = rParaLevel.mnUpperDist;
                if ( pParaLevel && ( rRetValue != pParaLevel->mnUpperDist ) )
                    bIsHardAttribute = true;
            }
            break;
            case PPT_ParaAttr_LowerDist :
            {
                rRetValue = rParaLevel.mnLowerDist;
                if ( pParaLevel && ( rRetValue != pParaLevel->mnLowerDist ) )
                    bIsHardAttribute = true;
            }
            break;
            case PPT_ParaAttr_TextOfs :
            {
                rRetValue = rParaLevel.mnTextOfs;
                if ( pParaLevel && ( rRetValue != pParaLevel->mnTextOfs ) )
                    bIsHardAttribute = true;
            }
            break;
            case PPT_ParaAttr_BulletOfs :
            {
                rRetValue = rParaLevel.mnBulletOfs;
                if ( pParaLevel && ( rRetValue != pParaLevel->mnBulletOfs ) )
                    bIsHardAttribute = true;
            }
            break;
            case PPT_ParaAttr_DefaultTab :
            {
                rRetValue = rParaLevel.mnDefaultTab;
                if ( pParaLevel && ( rRetValue != pParaLevel->mnDefaultTab ) )
                    bIsHardAttribute = true;
            }
            break;
            case PPT_ParaAttr_AsianLB_1 :
            {
                rRetValue = rParaLevel.mnAsianLineBreak & 1;
                if ( pParaLevel && ( rRetValue != ( (sal_uInt32)pParaLevel->mnAsianLineBreak & 1 ) ) )
                    bIsHardAttribute = true;
            }
            break;
            case PPT_ParaAttr_AsianLB_2 :
            {
                rRetValue = ( rParaLevel.mnAsianLineBreak >> 1 ) & 1;
                if ( pParaLevel && ( rRetValue != ( ( (sal_uInt32)pParaLevel->mnAsianLineBreak >> 1 ) & 1 ) ) )
                    bIsHardAttribute = true;
            }
            break;
            case PPT_ParaAttr_AsianLB_3 :
            {
                rRetValue = ( rParaLevel.mnAsianLineBreak >> 2 ) & 1;
                if ( pParaLevel && ( rRetValue != ( ( (sal_uInt32)pParaLevel->mnAsianLineBreak >> 2 ) & 1 ) ) )
                    bIsHardAttribute = true;
            }
            break;
            case PPT_ParaAttr_BiDi :
            {
                rRetValue = rParaLevel.mnBiDi;
                if ( pParaLevel && ( rRetValue != pParaLevel->mnBiDi ) )
                    bIsHardAttribute = true;
            }
            break;
        }
    }
    return bIsHardAttribute;
}

void PPTParagraphObj::ApplyTo( SfxItemSet& rSet,  boost::optional< sal_Int16 >& rStartNumbering, SdrPowerPointImport& rManager, TSS_Type nDestinationInstance, const PPTParagraphObj* /*pPrev*/)
{
    sal_Int16   nVal2;
    sal_uInt32  nVal, nUpperDist, nLowerDist;
    TSS_Type    nInstance = nDestinationInstance != TSS_Type::Unknown ? nDestinationInstance : mnInstance;

    if ( ( nDestinationInstance != TSS_Type::Unknown ) || ( pParaSet->mnDepth <= 1 ) )
    {
        SvxNumBulletItem* pNumBulletItem = mrStyleSheet.mpNumBulletItem[ nInstance ];
        if ( pNumBulletItem )
        {
            SvxNumberFormat aNumberFormat( SVX_NUM_NUMBER_NONE );
            if ( GetNumberFormat( rManager, aNumberFormat, this, nDestinationInstance, rStartNumbering ) )
            {
                if ( aNumberFormat.GetNumberingType() == SVX_NUM_NUMBER_NONE )
                {
                    aNumberFormat.SetAbsLSpace( 0 );
                    aNumberFormat.SetFirstLineOffset( 0 );
                    aNumberFormat.SetCharTextDistance( 0 );
                    aNumberFormat.SetFirstLineIndent( 0 );
                    aNumberFormat.SetIndentAt( 0 );
                }
                SvxNumBulletItem aNewNumBulletItem( *pNumBulletItem );
                SvxNumRule* pRule = aNewNumBulletItem.GetNumRule();
                if ( pRule )
                {
                    pRule->SetLevel( pParaSet->mnDepth, aNumberFormat );
                    sal_uInt16 i, n;
                    for ( i = 0; i < pRule->GetLevelCount(); i++ )
                    {
                        if ( i != pParaSet->mnDepth )
                        {
                            n = i > 4 ? 4 : i;

                            SvxNumberFormat aNumberFormat2( pRule->GetLevel( i ) );
                            const PPTParaLevel& rParaLevel = mrStyleSheet.mpParaSheet[ nInstance ]->maParaLevel[ n ];
                            const PPTCharLevel& rCharLevel = mrStyleSheet.mpCharSheet[ nInstance ]->maCharLevel[ n ];
                            sal_uInt32 nColor;
                            if ( rParaLevel.mnBuFlags & ( 1 << PPT_ParaAttr_BuHardColor ) )
                                nColor = rParaLevel.mnBulletColor;
                            else
                                nColor = rCharLevel.mnFontColor;
                            aNumberFormat2.SetBulletColor( rManager.MSO_TEXT_CLR_ToColor( nColor ) );
                            pRule->SetLevel( i, aNumberFormat2 );
                        }
                    }
                    rSet.Put( aNewNumBulletItem );
                }
            }
        }
    }

    sal_uInt32 nIsBullet2, _nTextOfs, _nBulletOfs;
    GetAttrib(PPT_ParaAttr_BulletOn, nIsBullet2, nDestinationInstance);
    GetAttrib(PPT_ParaAttr_TextOfs, _nTextOfs, nDestinationInstance);
    GetAttrib(PPT_ParaAttr_BulletOfs, _nBulletOfs, nDestinationInstance);
    if ( !nIsBullet2 )
    {
        SvxLRSpaceItem aLRSpaceItem( EE_PARA_LRSPACE );
        sal_uInt16 nAbsLSpace = (sal_uInt16)( ( (sal_uInt32)_nTextOfs * 2540 ) / 576 );
        sal_uInt16 nFirstLineOffset = nAbsLSpace - (sal_uInt16)( ( (sal_uInt32)_nBulletOfs * 2540 ) / 576 );
        aLRSpaceItem.SetLeft( nAbsLSpace );
        aLRSpaceItem.SetTextFirstLineOfstValue( -nFirstLineOffset );
        rSet.Put( aLRSpaceItem );
    }
    else
    {
        SvxLRSpaceItem aLRSpaceItem( EE_PARA_LRSPACE );
        aLRSpaceItem.SetLeft( 0 );
        aLRSpaceItem.SetTextFirstLineOfstValue( 0 );
        rSet.Put( aLRSpaceItem );
    }
    if ( GetAttrib( PPT_ParaAttr_Adjust, nVal, nDestinationInstance ) )
    {
        if ( nVal <= 3 )
        {   // paragraph adjustment
            static SvxAdjust const aAdj[ 4 ] = { SVX_ADJUST_LEFT, SVX_ADJUST_CENTER, SVX_ADJUST_RIGHT, SVX_ADJUST_BLOCK };
            rSet.Put( SvxAdjustItem( aAdj[ nVal ], EE_PARA_JUST ) );
        }
    }

    if ( GetAttrib( PPT_ParaAttr_AsianLB_1, nVal, nDestinationInstance ) )
        rSet.Put(SvxForbiddenRuleItem(nVal != 0, EE_PARA_FORBIDDENRULES));
    if ( GetAttrib( PPT_ParaAttr_AsianLB_3, nVal, nDestinationInstance ) )
        rSet.Put(SvxHangingPunctuationItem(nVal != 0, EE_PARA_HANGINGPUNCTUATION));

    if ( GetAttrib( PPT_ParaAttr_BiDi, nVal, nDestinationInstance ) )
        rSet.Put( SvxFrameDirectionItem( nVal == 1 ? FRMDIR_HORI_RIGHT_TOP : FRMDIR_HORI_LEFT_TOP, EE_PARA_WRITINGDIR ) );

    // LineSpacing
    PPTPortionObj* pPortion = First();
    bool bIsHardAttribute = GetAttrib( PPT_ParaAttr_LineFeed, nVal, nDestinationInstance );
    nVal2 = (sal_Int16)nVal;
    sal_uInt32 nFont = sal_uInt32();
    if ( pPortion && pPortion->GetAttrib( PPT_CharAttr_Font, nFont, nDestinationInstance ) )
        bIsHardAttribute = true;

    if ( bIsHardAttribute )
    {
        if ( pPortion && ( nVal2 > 200 ) )
        {
            sal_uInt32 nFontHeight;
            pPortion->GetAttrib( PPT_CharAttr_FontHeight, nFontHeight, nDestinationInstance );
            nVal2 = -(sal_Int16)( ( nFontHeight * nVal * 8 ) / 100 );
        }
        SdrTextFixedCellHeightItem aHeightItem(true);
        aHeightItem.SetWhich(SDRATTR_TEXT_USEFIXEDCELLHEIGHT);
        rSet.Put( aHeightItem );
        SvxLineSpacingItem aItem( 200, EE_PARA_SBL );
        if ( nVal2 <= 0 ) {
            aItem.SetLineHeight( (sal_uInt16)( rManager.ScalePoint( -nVal2 ) / 8 ) );
            aItem.SetLineSpaceRule( SvxLineSpaceRule::Fix );
            aItem.SetInterLineSpaceRule(SvxInterLineSpaceRule::Off);
        } else
        {
            sal_uInt8 nPropLineSpace = (sal_uInt8)nVal2;
            aItem.SetPropLineSpace( nPropLineSpace );
            aItem.SetLineSpaceRule( SvxLineSpaceRule::Auto );
        }
        rSet.Put( aItem );
    }

    // Paragraph Spacing
    bIsHardAttribute = ( (sal_uInt32)GetAttrib( PPT_ParaAttr_UpperDist, nUpperDist, nDestinationInstance ) +
        (sal_uInt32)GetAttrib( PPT_ParaAttr_LowerDist, nLowerDist, nDestinationInstance ) ) != 0;
    if ( ( nUpperDist > 0 ) || ( nLowerDist > 0 ) )
    {
        if (!m_PortionList.empty())
        {
            sal_uInt32 nFontHeight = 0;
            m_PortionList.back()->GetAttrib(
                    PPT_CharAttr_FontHeight, nFontHeight, nDestinationInstance);
            if ( ((sal_Int16)nUpperDist) > 0 )
                nUpperDist = - (sal_Int16)( ( nFontHeight * nUpperDist * 100 ) / 1000 );
            if ( ((sal_Int16)nLowerDist) > 0 )
                nLowerDist = - (sal_Int16)( ( nFontHeight * nLowerDist * 100 ) / 1000 );
        }
        bIsHardAttribute = true;
    }
    if ( bIsHardAttribute )
    {
        SvxULSpaceItem aULSpaceItem( EE_PARA_ULSPACE );
        nVal2 = (sal_Int16)nUpperDist;
        if ( nVal2 <= 0 )
            aULSpaceItem.SetUpper( (sal_uInt16)(((sal_uInt32) - nVal2 * 2540 ) / ( 72 * 8 ) ) );
        else
        {
            aULSpaceItem.SetUpperValue( 0 );
            aULSpaceItem.SetPropUpper( (sal_uInt16)nUpperDist == 100 ? 101 : (sal_uInt16)nUpperDist );
        }
        nVal2 = (sal_Int16)nLowerDist;
        if ( nVal2 <= 0 )
            aULSpaceItem.SetLower( (sal_uInt16)(((sal_uInt32) - nVal2 * 2540 ) / ( 72 * 8 ) ) );
        else
        {
            aULSpaceItem.SetLowerValue( 0 );
            aULSpaceItem.SetPropLower( (sal_uInt16)nLowerDist == 100 ? 101 : (sal_uInt16)nLowerDist );
        }
        rSet.Put( aULSpaceItem );
    }

    if ( mbTab )    // makes it sense to apply tabsettings
    {
        sal_uInt32 i, nDefaultTab, nTab, nTextOfs2 = 0;
        sal_uInt32 nLatestManTab = 0;
        GetAttrib( PPT_ParaAttr_TextOfs, nTextOfs2, nDestinationInstance );
        GetAttrib( PPT_ParaAttr_BulletOfs, nTab, nDestinationInstance );
        GetAttrib( PPT_ParaAttr_BulletOn, i, nDestinationInstance );
        GetAttrib( PPT_ParaAttr_DefaultTab, nDefaultTab, nDestinationInstance );
        SvxTabStopItem aTabItem( 0, 0, SvxTabAdjust::Default, EE_PARA_TABS );
        if ( GetTabCount() )
        {
            //paragraph offset = MIN(first_line_offset, hanging_offset)
            sal_uInt32 nParaOffset = std::min(nTextOfs2, nTab);
            for ( i = 0; i < GetTabCount(); i++ )
            {
                SvxTabAdjust eTabAdjust;
                nTab = GetTabOffsetByIndex( (sal_uInt16)i );
                switch( GetTabStyleByIndex( (sal_uInt16)i ) )
                {
                    case 1 :    eTabAdjust = SvxTabAdjust::Center; break;
                    case 2 :    eTabAdjust = SvxTabAdjust::Right; break;
                    case 3 :    eTabAdjust = SvxTabAdjust::Decimal; break;
                    default :   eTabAdjust = SvxTabAdjust::Left;
                }
                if ( nTab > nParaOffset )//If tab stop greater than paragraph offset
                    aTabItem.Insert( SvxTabStop( ( ( (long( nTab - nTextOfs2 )) * 2540 ) / 576 ), eTabAdjust ) );
            }
            nLatestManTab = nTab;
        }
        if ( nIsBullet2 == 0 )
            aTabItem.Insert( SvxTabStop( (sal_uInt16)0 ) );
        if ( nDefaultTab )
        {
            nTab = ( nTextOfs2 > nLatestManTab ) ? nTextOfs2 : nLatestManTab;
            nTab /= nDefaultTab;
            nTab = nDefaultTab * ( 1 + nTab );
            for ( i = 0; ( i < 20 ) && ( nTab < 0x1b00 ); i++ )
            {
                aTabItem.Insert( SvxTabStop( (sal_uInt16)( ( ( nTab - nTextOfs2 ) * 2540 ) / 576 ) ) );
                nTab += nDefaultTab;
            }
        }
        rSet.Put( aTabItem );
    }
}

sal_uInt32 PPTParagraphObj::GetTextSize()
{
    sal_uInt32 nCount, nRetValue = 0;
    for (std::unique_ptr<PPTPortionObj> & i : m_PortionList)
    {
        PPTPortionObj const& rPortionObj = *i;
        nCount = rPortionObj.Count();
        if ((!nCount) && rPortionObj.mpFieldItem)
            nCount++;
        nRetValue += nCount;
    }
    return nRetValue;
}

PPTPortionObj* PPTParagraphObj::First()
{
    mnCurrentObject = 0;
    if (m_PortionList.empty())
        return nullptr;
    return m_PortionList.front().get();
}

PPTPortionObj* PPTParagraphObj::Next()
{
    sal_uInt32 i = mnCurrentObject + 1;
    if (i >= m_PortionList.size())
        return nullptr;
    mnCurrentObject++;
    return m_PortionList[i].get();
}

PPTFieldEntry::~PPTFieldEntry()
{
    delete pField1;
    delete pField2;
    delete pString;
};

void PPTFieldEntry::GetDateTime( const sal_uInt32 nVal, SvxDateFormat& eDateFormat, SvxTimeFormat& eTimeFormat )
{
    eDateFormat = SVXDATEFORMAT_APPDEFAULT;
    eTimeFormat = SVXTIMEFORMAT_APPDEFAULT;
    // evaluate ID
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
            SAL_FALLTHROUGH;
        case 9:
            eTimeFormat = SVXTIMEFORMAT_24_HM;
        break;
        case 8:
            eDateFormat = SVXDATEFORMAT_A;
            SAL_FALLTHROUGH;
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
}

void PPTFieldEntry::SetDateTime( sal_uInt32 nVal )
{
    SvxDateFormat eDateFormat;
    SvxTimeFormat eTimeFormat;
    GetDateTime( nVal, eDateFormat, eTimeFormat );
    if ( eDateFormat != SVXDATEFORMAT_APPDEFAULT )
        pField1 = new SvxFieldItem( SvxDateField( Date( Date::SYSTEM ), SVXDATETYPE_VAR, eDateFormat ), EE_FEATURE_FIELD );
    if ( eTimeFormat != SVXTIMEFORMAT_APPDEFAULT )
    {
        SvxFieldItem* pFieldItem = new SvxFieldItem( SvxExtTimeField( tools::Time( tools::Time::SYSTEM ), SVXTIMETYPE_VAR, eTimeFormat ), EE_FEATURE_FIELD );
        if ( pField1 )
            pField2 = pFieldItem;
        else
            pField1 = pFieldItem;
    }
}

PPTTextObj::PPTTextObj( SvStream& rIn, SdrPowerPointImport& rSdrPowerPointImport, PptSlidePersistEntry& rPersistEntry, DffObjData* pObjData ) :
    mpImplTextObj   ( new ImplPPTTextObj( rPersistEntry ) )
{
    mpImplTextObj->mnRefCount = 1;
    mpImplTextObj->mnShapeId = 0;
    mpImplTextObj->mnShapeMaster = 0;
    mpImplTextObj->mpPlaceHolderAtom = nullptr;
    mpImplTextObj->mnDestinationInstance = mpImplTextObj->mnInstance = TSS_Type::TextInShape;
    mpImplTextObj->mnCurrentObject = 0;
    mpImplTextObj->mnParagraphCount = 0;
    mpImplTextObj->mpParagraphList = nullptr;
    mpImplTextObj->mnTextFlags = 0;
    mpImplTextObj->meShapeType = ( pObjData && pObjData->bShapeType ) ? pObjData->eShapeType : mso_sptMin;

    DffRecordHeader aExtParaHd;
    aExtParaHd.nRecType = 0;    // set empty


    DffRecordHeader aShapeContainerHd;
    ReadDffRecordHeader( rIn, aShapeContainerHd );

    if ( ( pObjData == nullptr ) || ( pObjData->bShapeType ) )
    {
        PPTExtParaProv* pExtParaProv = rSdrPowerPointImport.pPPTStyleSheet->pExtParaProv.get();
        if ( pObjData )
        {
            mpImplTextObj->mnShapeId = pObjData->nShapeId;
            if ( pObjData->nSpFlags & SP_FHAVEMASTER )
                mpImplTextObj->mnShapeMaster = rSdrPowerPointImport.GetPropertyValue( DFF_Prop_hspMaster, 0 );
        }
        // ClientData
        if ( rSdrPowerPointImport.maShapeRecords.SeekToContent( rIn, DFF_msofbtClientData, SEEK_FROM_CURRENT_AND_RESTART ) )
        {
            sal_uInt32 nOldPos = rIn.Tell();
            DffRecordHeader& aClientDataContainerHd = *rSdrPowerPointImport.maShapeRecords.Current();
            DffRecordHeader aPlaceHolderAtomHd;
            if ( SvxMSDffManager::SeekToRec( rIn, PPT_PST_OEPlaceholderAtom, aClientDataContainerHd.GetRecEndFilePos(), &aPlaceHolderAtomHd ) )
            {
                mpImplTextObj->mpPlaceHolderAtom = new PptOEPlaceholderAtom;
                ReadPptOEPlaceholderAtom( rIn, *( mpImplTextObj->mpPlaceHolderAtom ) );
            }
            rIn.Seek( nOldPos );
            DffRecordHeader aProgTagHd;
            if ( SdrPowerPointImport::SeekToContentOfProgTag( 9, rIn, aClientDataContainerHd, aProgTagHd ) )
            {
                ReadDffRecordHeader( rIn, aExtParaHd );
            }
        }

        // ClientTextBox
        if ( rSdrPowerPointImport.maShapeRecords.SeekToContent( rIn, DFF_msofbtClientTextbox, SEEK_FROM_CURRENT_AND_RESTART ) )
        {
            bool bStatus = true;


            DffRecordHeader aClientTextBoxHd( *rSdrPowerPointImport.maShapeRecords.Current() );
            sal_uInt32  nTextRulerAtomOfs = 0;  // case of zero -> this atom may be found in aClientDataContainerHd;
                                            // case of -1 -> there is no atom of this kind
                                            // else -> this is the fileofs where we can get it

            // checkout if this is a referenced
            // textobj, if so the we will patch
            // the ClientTextBoxHd for a
            // equivalent one
            DffRecordHeader aTextHd;
            if ( SvxMSDffManager::SeekToRec( rIn, PPT_PST_OutlineTextRefAtom, aClientTextBoxHd.GetRecEndFilePos(), &aTextHd ) )
            {
                sal_uInt32 nRefNum;
                rIn.ReadUInt32( nRefNum );

                if ( SvxMSDffManager::SeekToRec( rIn, PPT_PST_TextRulerAtom, aClientTextBoxHd.GetRecEndFilePos() ) )
                    nTextRulerAtomOfs = rIn.Tell();
                else
                    nTextRulerAtomOfs = 0xffffffff;

                sal_uInt32 nInstance = 0;
                switch( rSdrPowerPointImport.eAktPageKind )
                {
                    case PPT_NOTEPAGE :
                        nInstance++;
                        SAL_FALLTHROUGH;
                    case PPT_MASTERPAGE :
                        nInstance++;
                        break;
                    case PPT_SLIDEPAGE :
                    break;
                    default :
                        bStatus = false;
                }
                if ( bStatus )
                {
                    sal_uInt32 nSlideId = rSdrPowerPointImport.GetAktPageId();
                    if ( !nSlideId )
                        bStatus = false;
                    else
                    {
                        if ( !aExtParaHd.nRecType )
                        {
                            sal_uInt32 nOldPos = rIn.Tell();
                            // try to locate the referenced ExtendedParaHd
                            DffRecordHeader* pHd = pExtParaProv->
                                                        aExtendedPresRules.GetRecordHeader( PPT_PST_ExtendedParagraphHeaderAtom,
                                                                                            SEEK_FROM_CURRENT_AND_RESTART );
                            DffRecordHeader     aPresRuleHd;
                            DffRecordHeader*    pFirst = pHd;

                            sal_uInt32  nTmpSlideId, nTmpRef;
                            while ( pHd )
                            {
                                pHd->SeekToContent( rIn );
                                rIn.ReadUInt32( nTmpSlideId )
                                   .ReadUInt32( nTmpRef );         // this seems to be the instance

                                if ( ( nTmpSlideId == nSlideId ) && ( pHd->nRecInstance == nRefNum ) )
                                {
                                    if (!pHd->SeekToEndOfRecord(rIn))
                                        break;
                                    ReadDffRecordHeader( rIn, aPresRuleHd );
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
                        PptSlidePersistEntry* pE = nullptr;
                        if ( pPageList && ( rSdrPowerPointImport.nAktPageNum < pPageList->size() ) )
                            pE = &(*pPageList)[ rSdrPowerPointImport.nAktPageNum ];
                        if ( (!pE) || (!pE->nSlidePersistStartOffset) || ( pE->aPersistAtom.nSlideId != nSlideId ) )
                            bStatus = false;
                        else
                        {
                            auto nOffset(pE->nSlidePersistStartOffset);
                            bStatus = (nOffset == rIn.Seek(nOffset));
                            // now we got the right page and are searching for the right
                            // TextHeaderAtom
                            auto nEndRecPos = DffPropSet::SanitizeEndPos(rIn, pE->nSlidePersistEndOffset);
                            while (bStatus && rIn.Tell() < nEndRecPos)
                            {
                                ReadDffRecordHeader( rIn, aClientTextBoxHd );
                                if ( aClientTextBoxHd.nRecType == PPT_PST_TextHeaderAtom )
                                {
                                    if ( aClientTextBoxHd.nRecInstance == nRefNum )
                                    {
                                        aClientTextBoxHd.SeekToEndOfRecord( rIn );
                                        break;
                                    }
                                }
                                if (!aClientTextBoxHd.SeekToEndOfRecord(rIn))
                                    break;
                            }
                            if ( rIn.Tell() > pE->nSlidePersistEndOffset )
                                bStatus = false;
                            else
                            {   // patching the RecordHeader
                                aClientTextBoxHd.nFilePos -= DFF_COMMON_RECORD_HEADER_SIZE;
                                aClientTextBoxHd.nRecLen += DFF_COMMON_RECORD_HEADER_SIZE;
                                aClientTextBoxHd.nRecType = DFF_msofbtClientTextbox;
                                aClientTextBoxHd.nRecVer = DFF_PSFLAG_CONTAINER;

                                // we have to calculate the correct record len
                                DffRecordHeader aTmpHd;
                                nEndRecPos = DffPropSet::SanitizeEndPos(rIn, pE->nSlidePersistEndOffset);
                                while (rIn.Tell() < nEndRecPos)
                                {
                                    ReadDffRecordHeader( rIn, aTmpHd );
                                    if ( ( aTmpHd.nRecType == PPT_PST_SlidePersistAtom ) || ( aTmpHd.nRecType == PPT_PST_TextHeaderAtom ) )
                                        break;
                                    if (!aTmpHd.SeekToEndOfRecord(rIn))
                                        break;
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
                if ( SvxMSDffManager::SeekToRec( rIn, PPT_PST_TextHeaderAtom, aClientTextBoxHd.GetRecEndFilePos(), &aTextHd ) )
                {
                    // TextHeaderAtom is always the first Atom
                    sal_uInt16 nTmp;
                    rIn.ReadUInt16( nTmp );   // this number tells us the TxMasterStyleAtom Instance
                    if ( nTmp > 8 )
                        nTmp = 4;
                    TSS_Type nInstance = (TSS_Type)nTmp;
                    aTextHd.SeekToEndOfRecord( rIn );
                    mpImplTextObj->mnInstance = nInstance;

                    sal_uInt32 nFilePos = rIn.Tell();
                    if ( rSdrPowerPointImport.SeekToRec2( PPT_PST_TextBytesAtom,
                                                          PPT_PST_TextCharsAtom,
                                                          aClientTextBoxHd.GetRecEndFilePos() )
                         || SvxMSDffManager::SeekToRec( rIn,
                                                            PPT_PST_StyleTextPropAtom,
                                                            aClientTextBoxHd.GetRecEndFilePos() ) )
                    {
                        PPTTextRulerInterpreter aTextRulerInterpreter( nTextRulerAtomOfs, aClientTextBoxHd, rIn );

                        PPTStyleTextPropReader aStyleTextPropReader( rIn, aClientTextBoxHd,
                                                                        aTextRulerInterpreter, aExtParaHd, nInstance );
                        sal_uInt32 nParagraphs = mpImplTextObj->mnParagraphCount = aStyleTextPropReader.aParaPropList.size();
                        if ( nParagraphs )
                        {
                            // the language settings will be merged into the list of PPTCharPropSet
                            DffRecordHeader aTextSpecInfoHd;
                            PPTTextSpecInfoAtomInterpreter aTextSpecInfoAtomInterpreter;
                            if ( SvxMSDffManager::SeekToRec( rIn, PPT_PST_TextSpecInfoAtom,
                                                        aClientTextBoxHd.GetRecEndFilePos(), &aTextSpecInfoHd ) )
                            {
                                if ( aTextSpecInfoAtomInterpreter.Read( rIn, aTextSpecInfoHd, PPT_PST_TextSpecInfoAtom,
                                        &(rSdrPowerPointImport.pPPTStyleSheet->maTxSI) ) )
                                {
                                    sal_uInt32  nI = 0;
                                    for (PPTTextSpecInfo* pSpecInfo : aTextSpecInfoAtomInterpreter.aList)
                                    {
                                        sal_uInt32 nCharIdx = pSpecInfo->nCharIdx;

                                        // portions and text have to been splitted in some cases
                                        for ( ; nI < aStyleTextPropReader.aCharPropList.size(); )
                                        {
                                            PPTCharPropSet* pSet = aStyleTextPropReader.aCharPropList[ nI ];
                                            if ( pSet->mnOriginalTextPos < nCharIdx )
                                            {
                                                pSet->mnLanguage[ 0 ] = pSpecInfo->nLanguage[ 0 ];
                                                pSet->mnLanguage[ 1 ] = pSpecInfo->nLanguage[ 1 ];
                                                pSet->mnLanguage[ 2 ] = pSpecInfo->nLanguage[ 2 ];
                                                // test if the current portion needs to be splitted
                                                if ( pSet->maString.getLength() > 1 )
                                                {
                                                    sal_Int32 nIndexOfNextPortion = pSet->maString.getLength() + pSet->mnOriginalTextPos;
                                                    sal_Int32 nNewLen = nIndexOfNextPortion - nCharIdx;
                                                    sal_Int32 nOldLen = pSet->maString.getLength() - nNewLen;

                                                    if ( ( nNewLen > 0 ) && ( nOldLen > 0 ) )
                                                    {
                                                        OUString aString( pSet->maString );
                                                        PPTCharPropSet* pNew = new PPTCharPropSet( *pSet );
                                                        pSet->maString = aString.copy( 0, nOldLen);
                                                        pNew->maString = aString.copy( nOldLen, nNewLen);
                                                        pNew->mnOriginalTextPos += nOldLen;
                                                        aStyleTextPropReader.aCharPropList.insert( aStyleTextPropReader.aCharPropList.begin() + nI + 1, pNew );
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
                                        OSL_FAIL( "SdrTextSpecInfoAtomInterpreter::Ctor(): parsing error, this document needs to be analysed (SJ)" );
                                    }
                                }
#endif
                            }
                            // now will search for possible textextensions such as date/time fields
                            // or ParaTabStops and append them on this textobj
                            rIn.Seek( nFilePos );
                            ::std::vector< PPTFieldEntry* > FieldList;
                            auto nEndRecPos = DffPropSet::SanitizeEndPos(rIn, aClientTextBoxHd.GetRecEndFilePos());
                            while (rIn.Tell() < nEndRecPos)
                            {
                                ReadDffRecordHeader( rIn, aTextHd );
                                sal_uInt16 nVal = 0;
                                std::unique_ptr<PPTFieldEntry> xEntry;
                                switch ( aTextHd.nRecType )
                                {
                                    case PPT_PST_DateTimeMCAtom :
                                    {
                                        xEntry.reset(new PPTFieldEntry);
                                        rIn.ReadUInt16(xEntry->nPos)
                                           .ReadUInt16( nVal )
                                           .ReadUInt16( nVal );
                                        xEntry->SetDateTime( nVal & 0xff );
                                    }
                                    break;

                                    case PPT_PST_FooterMCAtom :
                                    {
                                        xEntry.reset(new PPTFieldEntry);
                                        rIn.ReadUInt16(xEntry->nPos);
                                        xEntry->pField1 = new SvxFieldItem( SvxFooterField(), EE_FEATURE_FIELD );
                                    }
                                    break;

                                    case PPT_PST_HeaderMCAtom :
                                    {
                                        xEntry.reset(new PPTFieldEntry);
                                        rIn.ReadUInt16(xEntry->nPos);
                                        xEntry->pField1 = new SvxFieldItem( SvxHeaderField(), EE_FEATURE_FIELD );
                                    }
                                    break;

                                    case PPT_PST_GenericDateMCAtom :
                                    {
                                        xEntry.reset(new PPTFieldEntry);
                                        rIn.ReadUInt16(xEntry->nPos);
                                        xEntry->pField1 = new SvxFieldItem( SvxDateTimeField(), EE_FEATURE_FIELD );
                                        if ( rPersistEntry.pHeaderFooterEntry ) // sj: #i34111# on master pages it is possible
                                        {                                       // that there is no HeaderFooterEntry available
                                            if ( rPersistEntry.pHeaderFooterEntry->nAtom & 0x20000 )    // auto date time
                                                xEntry->SetDateTime( rPersistEntry.pHeaderFooterEntry->nAtom & 0xff );
                                            else
                                                xEntry->pString = new OUString( rPersistEntry.pHeaderFooterEntry->pPlaceholder[ nVal ] );
                                        }
                                    }
                                    break;

                                    case PPT_PST_SlideNumberMCAtom :
                                    case PPT_PST_RTFDateTimeMCAtom :
                                    {
                                        xEntry.reset(new PPTFieldEntry);
                                        if ( aTextHd.nRecLen >= 4 )
                                        {
                                            rIn.ReadUInt16(xEntry->nPos)
                                               .ReadUInt16( nVal );

                                            // evaluate ID
                                            //SvxFieldItem* pFieldItem = NULL;
                                            switch( aTextHd.nRecType )
                                            {
                                                case PPT_PST_SlideNumberMCAtom:
                                                    xEntry->pField1 = new SvxFieldItem( SvxPageField(), EE_FEATURE_FIELD );
                                                break;

                                                case PPT_PST_RTFDateTimeMCAtom:
                                                {
                                                    // Rude workaround for one special case reported
                                                    // by a customer. (#i75203#)

                                                    // Don't even attempt to handle the general use
                                                    // case for PPT_PST_RTFDateTimeMCAtom (a generic
                                                    // MS style date/time format string). Just handle
                                                    // the special case where the format string
                                                    // contains only one or several possibly empty
                                                    // quoted strings. I.e. something that doesn't
                                                    // expand to any date or time at all, but to a
                                                    // fixed string. How on earth somebody manages to
                                                    // produce such things in PPT slides I have no
                                                    // idea.
                                                    if (nVal == 0)
                                                    {
                                                        OUString aStr;
                                                        bool inquote = false;
                                                        for (int nLen = 0; nLen < 64; ++nLen)
                                                        {
                                                            sal_Unicode n(0);
                                                            rIn.ReadUtf16( n );

                                                            // Collect quoted characters into aStr
                                                            if ( n == '\'')
                                                                inquote = !inquote;
                                                            else if (!n)
                                                            {
                                                                // End of format string
                                                                xEntry->pString = new OUString( aStr );
                                                                break;
                                                            }
                                                            else if (!inquote)
                                                            {
                                                                // Non-quoted character, i.e. a real
                                                                // format specifier. We don't handle
                                                                // those. Sorry.
                                                                break;
                                                            }
                                                            else
                                                            {
                                                                aStr += OUStringLiteral1(n);
                                                            }
                                                        }
                                                    }
                                                    if (!xEntry->pString)
                                                    {
                                                        // Handle as previously
                                                        xEntry->pField1 = new SvxFieldItem( SvxDateField( Date( Date::SYSTEM ), SVXDATETYPE_FIX ), EE_FEATURE_FIELD );
                                                    }
                                                }
                                            }
                                        }
                                    }
                                    break;

                                    case PPT_PST_InteractiveInfo :
                                    {
                                        DffRecordHeader aHdInteractiveInfoAtom;
                                        if ( SvxMSDffManager::SeekToRec( rIn, PPT_PST_InteractiveInfoAtom, aTextHd.GetRecEndFilePos(), &aHdInteractiveInfoAtom ) )
                                        {
                                            PptInteractiveInfoAtom aInteractiveInfoAtom;
                                            ReadPptInteractiveInfoAtom( rIn, aInteractiveInfoAtom );
                                            for (SdHyperlinkEntry* pHyperlink : rSdrPowerPointImport.aHyperList)
                                            {
                                                if ( pHyperlink->nIndex == aInteractiveInfoAtom.nExHyperlinkId )
                                                {
                                                    aTextHd.SeekToEndOfRecord( rIn );
                                                    ReadDffRecordHeader( rIn, aTextHd );
                                                    if ( aTextHd.nRecType != PPT_PST_TxInteractiveInfoAtom )
                                                    {
                                                        aTextHd.SeekToBegOfRecord( rIn );
                                                        continue;
                                                    }
                                                    else
                                                    {
                                                        sal_uInt32 nStartPos, nEndPos;
                                                        rIn.ReadUInt32( nStartPos )
                                                           .ReadUInt32( nEndPos );
                                                        if ( nEndPos )
                                                        {
                                                            xEntry.reset(new PPTFieldEntry);
                                                            xEntry->nPos = (sal_uInt16)nStartPos;
                                                            xEntry->nTextRangeEnd = (sal_uInt16)nEndPos;
                                                            OUString aTarget( pHyperlink->aTarget );
                                                            if ( !pHyperlink->aConvSubString.isEmpty() )
                                                            {
                                                                aTarget += "#";
                                                                aTarget += pHyperlink->aConvSubString;
                                                            }
                                                            xEntry->pField1 = new SvxFieldItem( SvxURLField( aTarget, OUString(), SVXURLFORMAT_REPR ), EE_FEATURE_FIELD );
                                                        }
                                                    }
                                                    break;
                                                }
                                            }
                                        }
                                    }
                                    break;
                                }
                                if (!aTextHd.SeekToEndOfRecord(rIn))
                                    break;
                                if (xEntry)
                                {
                                    // sorting fields ( hi >> lo )
                                    ::std::vector< PPTFieldEntry* >::iterator it = FieldList.begin();
                                    for( ; it != FieldList.end(); ++it ) {
                                        if ( (*it)->nPos < xEntry->nPos ) {
                                            break;
                                        }
                                    }
                                    if ( it != FieldList.end() ) {
                                        FieldList.insert(it, xEntry.release());
                                    } else {
                                        FieldList.push_back(xEntry.release());
                                    }
                                }
                            }
                            if ( !FieldList.empty() )
                            {
                                ::std::vector< PPTFieldEntry* >::iterator FE = FieldList.begin();
                                PPTCharPropSetList& aCharPropList = aStyleTextPropReader.aCharPropList;

                                sal_Int32   i = nParagraphs - 1;
                                sal_Int32   n = aCharPropList.size() - 1;

                                // at this point we just have a list of textportions(aCharPropList)
                                // the next while loop tries to resolve the list of fields(pFieldList)
                                while( ( FE < FieldList.end() ) && ( n >= 0 ) && ( i >= 0 ) )
                                {
                                    PPTCharPropSet* pSet  = aCharPropList[n];
                                    OUString aString( pSet->maString );
                                    sal_uInt32 nCount = aString.getLength();
                                    sal_uInt32 nPos = pSet->mnOriginalTextPos + nCount;
                                    while ( ( FE < FieldList.end() ) && nCount-- )
                                    {
                                        nPos--;
                                        while ( ( FE < FieldList.end() ) && ( (*FE)->nPos > nPos ) )
                                            ++FE;
                                        if ( !(FE < FieldList.end()) )
                                            break;

                                        if ( (*FE)->nPos == nPos )
                                        {
                                            if ( aString[nCount] == 0x2a )
                                            {
                                                sal_uInt32 nBehind = aString.getLength() - ( nCount + 1 );
                                                pSet->maString.clear();
                                                if ( nBehind )
                                                {
                                                    PPTCharPropSet* pNewCPS = new PPTCharPropSet( *pSet );
                                                    pNewCPS->maString = aString.copy( nCount + 1, nBehind );
                                                    aCharPropList.insert( aCharPropList.begin() + n + 1, pNewCPS );
                                                }
                                                if ( (*FE)->pField2 )
                                                {
                                                    PPTCharPropSet* pNewCPS = new PPTCharPropSet( *pSet );
                                                    pNewCPS->mpFieldItem.reset( (*FE)->pField2 );
                                                    (*FE)->pField2 = nullptr;
                                                    aCharPropList.insert( aCharPropList.begin() + n + 1, pNewCPS );

                                                    pNewCPS = new PPTCharPropSet( *pSet );
                                                    pNewCPS->maString = " ";
                                                    aCharPropList.insert( aCharPropList.begin() + n + 1, pNewCPS );
                                                }
                                                if ( nCount )
                                                {
                                                    PPTCharPropSet* pNewCPS = new PPTCharPropSet( *pSet );
                                                    pNewCPS->maString = aString.copy( 0, nCount );
                                                    aCharPropList.insert( aCharPropList.begin() + n++, pNewCPS );
                                                }
                                                if ( (*FE)->pField1 )
                                                {
                                                    pSet->mpFieldItem.reset( (*FE)->pField1 );
                                                    (*FE)->pField1 = nullptr;
                                                }
                                                else if ( (*FE)->pString )
                                                    pSet->maString = *(*FE)->pString;
                                            }
                                            else
                                            {
                                                if ( (*FE)->nTextRangeEnd )   // text range hyperlink
                                                {
                                                    sal_uInt32 nHyperLen = (*FE)->nTextRangeEnd - nPos;
                                                    if ( nHyperLen )
                                                    {
                                                        PPTCharPropSet* pBefCPS = nullptr;
                                                        if ( nCount )
                                                        {
                                                            pBefCPS = new PPTCharPropSet( *pSet );
                                                            pSet->maString = pSet->maString.copy(nCount, pSet->maString.getLength() - nCount);
                                                        }
                                                        sal_uInt32  nIdx = n;
                                                        sal_Int32   nHyperLenLeft = nHyperLen;

                                                        while ( ( aCharPropList.size() > nIdx ) && nHyperLenLeft )
                                                        {
                                                            // the textrange hyperlink can take more than 1 paragraph
                                                            // the solution here is to clone the hyperlink...

                                                            PPTCharPropSet* pCurrent = aCharPropList[ nIdx ];
                                                            sal_Int32       nNextStringLen = pCurrent->maString.getLength();

                                                            DBG_ASSERT( (*FE)->pField1, "missing field!" );
                                                            if (!(*FE)->pField1)
                                                                break;

                                                            const SvxURLField* pField = static_cast<const SvxURLField*>((*FE)->pField1->GetField());

                                                            pCurrent->mbIsHyperlink = true;
                                                            pCurrent->mnHylinkOrigColor = pCurrent->mpImplPPTCharPropSet->mnColor;
                                                            pCurrent->mbHardHylinkOrigColor = ( ( pCurrent->mpImplPPTCharPropSet->mnAttrSet >>PPT_CharAttr_FontColor ) & 1)>0;

                                                            if ( pCurrent->mpFieldItem )
                                                            {
                                                                pCurrent->SetColor( PPT_COLSCHEME_A_UND_HYPERLINK );
                                                                if ( dynamic_cast< const SvxURLField* >(pCurrent->mpFieldItem->GetField()) != nullptr)
                                                                    break;
                                                                nHyperLenLeft--;
                                                            }
                                                            else if ( nNextStringLen )
                                                            {
                                                                if ( nNextStringLen <= nHyperLenLeft )
                                                                {
                                                                    pCurrent->mpFieldItem.reset( new SvxFieldItem( SvxURLField( pField->GetURL(), pCurrent->maString, SVXURLFORMAT_REPR ), EE_FEATURE_FIELD ) );
                                                                    nHyperLenLeft -= nNextStringLen;

                                                                    if ( nHyperLenLeft )
                                                                    {
                                                                        // if the next portion is in a higher paragraph,
                                                                        // the textrange is to decrease (because of the LineBreak character)
                                                                        if ( aCharPropList.size() > ( nIdx + 1 ) )
                                                                        {
                                                                            PPTCharPropSet* pNext = aCharPropList[ nIdx + 1 ];
                                                                            if ( pNext->mnParagraph > pCurrent->mnParagraph )
                                                                                nHyperLenLeft--;
                                                                        }
                                                                    }
                                                                }
                                                                else
                                                                {
                                                                    PPTCharPropSet* pNewCPS = new PPTCharPropSet( *pCurrent );
                                                                    pNewCPS->maString = pCurrent->maString.copy( nHyperLenLeft,( nNextStringLen - nHyperLenLeft ) );
                                                                    aCharPropList.insert( aCharPropList.begin() + nIdx + 1, pNewCPS );
                                                                    OUString aRepresentation = pCurrent->maString.copy( 0, nHyperLenLeft );
                                                                    pCurrent->mpFieldItem.reset( new SvxFieldItem( SvxURLField( pField->GetURL(), aRepresentation, SVXURLFORMAT_REPR ), EE_FEATURE_FIELD ) );
                                                                    nHyperLenLeft = 0;
                                                                }
                                                                (pCurrent->maString).clear();
                                                                pCurrent->SetColor( PPT_COLSCHEME_A_UND_HYPERLINK );
                                                            }
                                                            nIdx++;
                                                        }
                                                        delete (*FE)->pField1;
                                                        (*FE)->pField1 = nullptr;

                                                        if ( pBefCPS )
                                                        {
                                                            pBefCPS->maString = aString.copy( 0, nCount );
                                                            aCharPropList.insert( aCharPropList.begin() + n, pBefCPS );
                                                            n++;
                                                        }
                                                    }
                                                }
                                            }
                                            break;
                                        }
                                    }
                                    n--;
                                }
                                for(PPTFieldEntry* j : FieldList) {
                                    delete j;
                                }
                            }
                            mpImplTextObj->mpParagraphList = new PPTParagraphObj*[ nParagraphs ];
                            for (size_t nCurCharPos = 0, nCurPos = 0;
                                nCurPos < aStyleTextPropReader.aParaPropList.size();
                                ++nCurPos)
                            {
                                PPTParagraphObj* pPara = new PPTParagraphObj(
                                    aStyleTextPropReader, nCurPos, nCurCharPos,
                                    *rSdrPowerPointImport.pPPTStyleSheet,
                                    nInstance, aTextRulerInterpreter );
                                mpImplTextObj->mpParagraphList[ nCurPos ] = pPara;

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
        return nullptr;
    return mpImplTextObj->mpParagraphList[ 0 ];
}

PPTParagraphObj* PPTTextObj::Next()
{
    sal_uInt32 i = mpImplTextObj->mnCurrentObject + 1;
    if ( i >= mpImplTextObj->mnParagraphCount )
        return nullptr;
    mpImplTextObj->mnCurrentObject++;
    return mpImplTextObj->mpParagraphList[ i ];
}

const SfxItemSet* PPTTextObj::GetBackground() const
{
    if ( mpImplTextObj->mrPersistEntry.pBObj )
        return &mpImplTextObj->mrPersistEntry.pBObj->GetMergedItemSet();
    else
        return nullptr;
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

bool IsLine( const SdrObject* pObj )
{
    return dynamic_cast< const SdrPathObj* >(pObj) !=  nullptr &&
           static_cast<const SdrPathObj*>(pObj)->IsLine() &&
           static_cast<const SdrPathObj*>(pObj)->GetPointCount() == 2;
}

bool GetCellPosition( const SdrObject* pObj, const std::set< sal_Int32 >& rRows, const std::set< sal_Int32 >& rColumns,
                            sal_Int32& nTableIndex, sal_Int32& nRow, sal_Int32& nRowCount, sal_Int32& nColumn, sal_Int32& nColumnCount )
{
    Rectangle aSnapRect( pObj->GetSnapRect() );
    bool bCellObject = ( aSnapRect.GetWidth() > 1 ) && ( aSnapRect.GetHeight() > 1 );
    if ( bCellObject )
    {
        std::set< sal_Int32 >::const_iterator aRowIter( rRows.find( aSnapRect.Top() ) );
        std::set< sal_Int32 >::const_iterator aColumnIter( rColumns.find( aSnapRect.Left() ) );
        if ( ( aRowIter == rRows.end() ) || ( aColumnIter == rColumns.end() ) )
            bCellObject = false;
        else
        {
            nRowCount = 1;
            nRow = std::distance( rRows.begin(), aRowIter );
            while( ++aRowIter != rRows.end() )
            {
                if ( *aRowIter >= aSnapRect.Bottom() )
                    break;
                nRowCount++;
            }
            nColumnCount = 1;
            nColumn = std::distance( rColumns.begin(), aColumnIter );
            while( ++aColumnIter != rColumns.end() )
            {
                if ( *aColumnIter >= aSnapRect.Right() )
                    break;
                nColumnCount++;
            }
            nTableIndex = nRow * rColumns.size() + nColumn;
        }
    }
    return bCellObject;
}

#define LinePositionLeft    0x01000000
#define LinePositionTop     0x02000000
#define LinePositionRight   0x04000000
#define LinePositionBottom  0x08000000
#define LinePositionTLBR    0x10000000
#define LinePositionBLTR    0x20000000


void GetRowPositions( const Rectangle& rSnapRect, const std::set< sal_Int32 >& rRows,
                        const std::set< sal_Int32 >& rColumns, std::vector< sal_Int32 >& rPositions, sal_Int32 nColumn, sal_Int32 nFlags )
{
    std::set< sal_Int32 >::const_iterator aRow( rRows.find( rSnapRect.Top() ) );
    if ( aRow != rRows.end() )
    {
        sal_Int32 nRow = std::distance( rRows.begin(), aRow );
        while( ( aRow != rRows.end() ) && ((*aRow) < rSnapRect.Bottom() ) )
        {
            if ( nFlags & LinePositionLeft )
                rPositions.push_back( ( ( nRow * rColumns.size() ) + nColumn ) | LinePositionLeft );
            if ( nFlags & LinePositionRight )
                rPositions.push_back( ( ( nRow * rColumns.size() ) + ( nColumn - 1 ) ) | LinePositionRight );

            ++nRow;
            ++aRow;
        }
    }
}


void GetColumnPositions( const Rectangle& rSnapRect, const std::set< sal_Int32 >& /* rRows */,
                        const std::set< sal_Int32 >& rColumns, std::vector< sal_Int32 >& rPositions, sal_Int32 nRow, sal_Int32 nFlags )
{
    std::set< sal_Int32 >::const_iterator aColumn( rColumns.find( rSnapRect.Left() ) );
    if ( aColumn != rColumns.end() )
    {
        sal_Int32 nColumn = std::distance( rColumns.begin(), aColumn );
        while( ( aColumn != rColumns.end() ) && ((*aColumn) < rSnapRect.Right() ) )
        {
            if ( nFlags & LinePositionTop )
                rPositions.push_back( ( ( nRow * rColumns.size() ) + nColumn ) | LinePositionTop );
            if ( nFlags & LinePositionBottom )
                rPositions.push_back( ( ( ( nRow - 1 ) * rColumns.size() ) + nColumn ) | LinePositionBottom );

            ++nColumn;
            ++aColumn;
        }
    }
}

void GetLinePositions( const SdrObject* pObj, const std::set< sal_Int32 >& rRows, const std::set< sal_Int32 >& rColumns,
                        std::vector< sal_Int32 >& rPositions, const Rectangle& rGroupSnap )
{
    Rectangle aSnapRect( pObj->GetSnapRect() );
    if ( aSnapRect.Left() == aSnapRect.Right() )
    {
        std::set< sal_Int32 >::const_iterator aColumn( rColumns.find( aSnapRect.Left() ) );
        if ( ( aColumn != rColumns.end() ) || ( aSnapRect.Left() == rGroupSnap.Right() ) )
        {
            sal_Int32 nColumn, nFlags;
            if ( aColumn != rColumns.end() )
            {
                nColumn = std::distance( rColumns.begin(), aColumn );
                nFlags = LinePositionLeft;
                if ( aColumn != rColumns.begin() )
                    nFlags |= LinePositionRight;
            }
            else
            {
                nColumn = rColumns.size();
                nFlags = LinePositionRight;
            }
            GetRowPositions( aSnapRect, rRows, rColumns, rPositions, nColumn, nFlags );
        }
    }
    else if ( aSnapRect.Top() == aSnapRect.Bottom() )
    {
        std::set< sal_Int32 >::const_iterator aRow( rRows.find( aSnapRect.Top() ) );
        if ( ( aRow != rRows.end() ) || ( aSnapRect.Top() == rGroupSnap.Bottom() ) )
        {
            sal_Int32 nRow, nFlags;
            if ( aRow != rRows.end() )
            {
                nRow = std::distance( rRows.begin(), aRow );
                nFlags = LinePositionTop;
                if ( aRow != rRows.begin() )
                    nFlags |= LinePositionBottom;
            }
            else
            {
                nRow = rRows.size();
                nFlags = LinePositionBottom;
            }
            GetColumnPositions( aSnapRect, rRows, rColumns, rPositions, nRow, nFlags );
        }
    }
    else
    {
        sal_uInt32 nPosition = 0;
        Point aPt1( static_cast<const SdrPathObj*>(pObj)->GetPoint( 0 ) );
        Point aPt2( static_cast<const SdrPathObj*>(pObj)->GetPoint( 1 ) );
        if ( aPt1.X() < aPt2.X() )
            nPosition |= aPt1.Y() < aPt2.Y() ? LinePositionTLBR : LinePositionBLTR;
        else
            nPosition |= aPt1.Y() < aPt2.Y() ? LinePositionBLTR : LinePositionTLBR;

        std::set< sal_Int32 >::const_iterator aRow( rRows.find( aPt1.Y() < aPt2.Y() ? aPt1.Y() : aPt2.Y() ) );
        std::set< sal_Int32 >::const_iterator aColumn( rColumns.find( aPt1.X() < aPt2.X() ? aPt1.X() : aPt2.X() ) );
        if ( ( aRow != rRows.end() ) && ( aColumn != rColumns.end() ) )
        {
            nPosition |= ( std::distance( rRows.begin(), aRow ) * rColumns.size() ) + std::distance( rColumns.begin(), aColumn );
            rPositions.push_back( nPosition );
        }
    }
}

void CreateTableRows( const Reference< XTableRows >& xTableRows, const std::set< sal_Int32 >& rRows, sal_Int32 nTableBottom )
{
    if ( rRows.size() > 1 )
        xTableRows->insertByIndex( 0, rRows.size() - 1 );

    std::set< sal_Int32 >::const_iterator aIter( rRows.begin() );
    sal_Int32 nLastPosition( *aIter );
    for ( sal_Int32 n = 0; n < xTableRows->getCount(); n++ )
    {
        sal_Int32 nHeight;
        if ( ++aIter != rRows.end() )
        {
            nHeight = *aIter - nLastPosition;
            nLastPosition = *aIter;
        }
        else
            nHeight = nTableBottom - nLastPosition;

        Reference< XPropertySet > xPropSet( xTableRows->getByIndex( n ), UNO_QUERY_THROW );
        xPropSet->setPropertyValue( "Height", Any( nHeight ) );
    }
}

void CreateTableColumns( const Reference< XTableColumns >& xTableColumns, const std::set< sal_Int32 >& rColumns, sal_Int32 nTableRight )
{
    if ( rColumns.size() > 1 )
        xTableColumns->insertByIndex( 0, rColumns.size() - 1 );

    std::set< sal_Int32 >::const_iterator aIter( rColumns.begin() );
    sal_Int32 nLastPosition( *aIter );
    for ( sal_Int32 n = 0; n < xTableColumns->getCount(); n++ )
    {
        sal_Int32 nWidth;
        if ( ++aIter != rColumns.end() )
        {
            nWidth = *aIter - nLastPosition;
            nLastPosition = *aIter;
        }
        else
            nWidth = nTableRight - nLastPosition;

        Reference< XPropertySet > xPropSet( xTableColumns->getByIndex( n ), UNO_QUERY_THROW );
        xPropSet->setPropertyValue( "Width", Any( nWidth ) );
    }
}

void MergeCells( const Reference< XTable >& xTable, sal_Int32 nCol, sal_Int32 nRow, sal_Int32 nColSpan, sal_Int32 nRowSpan )
{
   DBG_ASSERT( (nColSpan > 1) || (nRowSpan > 1), "nonsense parameter!!" );
   DBG_ASSERT( (nCol >= 0) && (nCol < xTable->getColumnCount()) && (nRow >= 0) && (nRow < xTable->getRowCount()), "die celle gibts nicht!!" );
   DBG_ASSERT( (nColSpan >= 1) && ((nCol  + nColSpan - 1) < xTable->getColumnCount()), "nColSpan murks!" );
   DBG_ASSERT(  (nRowSpan >= 1) && ((nRow  + nRowSpan - 1) < xTable->getRowCount()), "nRowSpan murks!" );

   if( xTable.is() ) try
   {
       Reference< XMergeableCellRange > xRange( xTable->createCursorByRange( xTable->getCellRangeByPosition( nCol, nRow,nCol + nColSpan - 1, nRow + nRowSpan - 1 ) ), UNO_QUERY_THROW );
       if( xRange->isMergeable() )
               xRange->merge();
   }
   catch( const Exception& )
   {
       DBG_UNHANDLED_EXCEPTION();
   }
}

void ApplyCellAttributes( const SdrObject* pObj, Reference< XCell >& xCell )
{
    try
    {
        Reference< XPropertySet > xPropSet( xCell, UNO_QUERY_THROW );

        const sal_Int32 nLeftDist(static_cast<const SdrMetricItem&>(pObj->GetMergedItem(SDRATTR_TEXT_LEFTDIST)).GetValue());
        const sal_Int32 nRightDist(static_cast<const SdrMetricItem&>(pObj->GetMergedItem(SDRATTR_TEXT_RIGHTDIST)).GetValue());
        const sal_Int32 nUpperDist(static_cast<const SdrMetricItem&>(pObj->GetMergedItem(SDRATTR_TEXT_UPPERDIST)).GetValue());
        const sal_Int32 nLowerDist(static_cast<const SdrMetricItem&>(pObj->GetMergedItem(SDRATTR_TEXT_LOWERDIST)).GetValue());
        xPropSet->setPropertyValue( "TextUpperDistance", Any( nUpperDist ) );
        xPropSet->setPropertyValue( "TextRightDistance", Any( nRightDist ) );
        xPropSet->setPropertyValue( "TextLeftDistance", Any( nLeftDist ) );
        xPropSet->setPropertyValue( "TextLowerDistance", Any( nLowerDist ) );

        const SdrTextVertAdjust eTextVertAdjust(static_cast<const SdrTextVertAdjustItem&>(pObj->GetMergedItem(SDRATTR_TEXT_VERTADJUST)).GetValue());
        drawing::TextVerticalAdjust eVA( drawing::TextVerticalAdjust_TOP );
        if ( eTextVertAdjust == SDRTEXTVERTADJUST_CENTER )
            eVA = drawing::TextVerticalAdjust_CENTER;
        else if ( eTextVertAdjust == SDRTEXTVERTADJUST_BOTTOM )
            eVA = drawing::TextVerticalAdjust_BOTTOM;
        xPropSet->setPropertyValue( "TextVerticalAdjust", Any( eVA ) );

        //set textHorizontalAdjust and TextWritingMode attr
        const sal_Int32 eHA(static_cast<const SdrTextHorzAdjustItem&>(pObj->GetMergedItem(SDRATTR_TEXT_HORZADJUST)).GetValue());
        const SvxFrameDirection eDirection = (const SvxFrameDirection)(static_cast<const SvxFrameDirectionItem&>(pObj->GetMergedItem(EE_PARA_WRITINGDIR)).GetValue());
        xPropSet->setPropertyValue(  "TextHorizontalAdjust" , Any( eHA ) );
        if ( eDirection == FRMDIR_VERT_TOP_RIGHT )
        {//vertical writing
            xPropSet->setPropertyValue(  "TextWritingMode" , Any( css::text::WritingMode_TB_RL ) );
        }
        drawing::FillStyle eFillStyle(static_cast<const XFillStyleItem&>(pObj->GetMergedItem( XATTR_FILLSTYLE )).GetValue());
        css::drawing::FillStyle eFS( css::drawing::FillStyle_NONE );
        switch( eFillStyle )
        {
            case drawing::FillStyle_SOLID :
                {
                    eFS = css::drawing::FillStyle_SOLID;
                    Color aFillColor( static_cast<const XFillColorItem&>(pObj->GetMergedItem( XATTR_FILLCOLOR )).GetColorValue() );
                    sal_Int32 nFillColor( aFillColor.GetColor() );
                    xPropSet->setPropertyValue( "FillColor", Any( nFillColor ) );
                }
                break;
            case drawing::FillStyle_GRADIENT :
                {
                    eFS = css::drawing::FillStyle_GRADIENT;
                    XGradient aXGradient(static_cast<const XFillGradientItem&>(pObj->GetMergedItem(XATTR_FILLGRADIENT)).GetGradientValue());

                    css::awt::Gradient aGradient;
                    aGradient.Style = (awt::GradientStyle) aXGradient.GetGradientStyle();
                    aGradient.StartColor = (sal_Int32)aXGradient.GetStartColor().GetColor();
                    aGradient.EndColor = (sal_Int32)aXGradient.GetEndColor().GetColor();
                    aGradient.Angle = (short)aXGradient.GetAngle();
                    aGradient.Border = aXGradient.GetBorder();
                    aGradient.XOffset = aXGradient.GetXOffset();
                    aGradient.YOffset = aXGradient.GetYOffset();
                    aGradient.StartIntensity = aXGradient.GetStartIntens();
                    aGradient.EndIntensity = aXGradient.GetEndIntens();
                    aGradient.StepCount = aXGradient.GetSteps();

                    xPropSet->setPropertyValue( "FillGradient", Any( aGradient ) );
                }
                break;
            case drawing::FillStyle_HATCH :
                eFS = css::drawing::FillStyle_HATCH;
            break;
            case drawing::FillStyle_BITMAP :
                {
                    eFS = css::drawing::FillStyle_BITMAP;

                    const XFillBitmapItem aXFillBitmapItem(static_cast<const XFillBitmapItem&>(pObj->GetMergedItem( XATTR_FILLBITMAP )));
                    OUString aURL( UNO_NAME_GRAPHOBJ_URLPREFIX);
                    aURL += OStringToOUString(
                        aXFillBitmapItem.GetGraphicObject().GetUniqueID(),
                        RTL_TEXTENCODING_ASCII_US);

                    xPropSet->setPropertyValue("FillBitmapURL", Any( aURL ) );

                    const XFillBmpStretchItem aStretchItem(static_cast<const XFillBmpStretchItem&>(pObj->GetMergedItem( XATTR_FILLBMP_STRETCH )));
                    const XFillBmpTileItem aTileItem(static_cast<const XFillBmpTileItem&>(pObj->GetMergedItem( XATTR_FILLBMP_TILE )));
                    if( aTileItem.GetValue() )
                        xPropSet->setPropertyValue( "FillBitmapMode", Any( css::drawing::BitmapMode_REPEAT ) );
                    else if( aStretchItem.GetValue() )
                        xPropSet->setPropertyValue( "FillBitmapMode", Any( css::drawing::BitmapMode_STRETCH ) );
                    else
                        xPropSet->setPropertyValue( "FillBitmapMode", Any( css::drawing::BitmapMode_NO_REPEAT ) );
                }
            break;
            default:
            case drawing::FillStyle_NONE :
                eFS = css::drawing::FillStyle_NONE;
            break;

        }
        xPropSet->setPropertyValue( "FillStyle", Any( eFS ) );
        if ( eFillStyle != drawing::FillStyle_NONE )
        {
            sal_Int16 nFillTransparence( static_cast<const XFillTransparenceItem&>(pObj->GetMergedItem( XATTR_FILLTRANSPARENCE ) ).GetValue() );
            xPropSet->setPropertyValue( "FillTransparence", Any( nFillTransparence ) );
        }
    }
    catch( const Exception& )
    {
    }
}

void ApplyCellLineAttributes( const SdrObject* pLine, Reference< XTable >& xTable, const std::vector< sal_Int32 >& vPositions, sal_Int32 nColumns )
{
    try
    {
        drawing::LineStyle eLineStyle(static_cast<const XLineStyleItem&>(pLine->GetMergedItem( XATTR_LINESTYLE )).GetValue());
        css::table::BorderLine2 aBorderLine;
        switch( eLineStyle )
        {
            case drawing::LineStyle_DASH :
            case drawing::LineStyle_SOLID :
                {
                    Color aLineColor( static_cast<const XLineColorItem&>(pLine->GetMergedItem( XATTR_LINECOLOR )).GetColorValue() );
                    aBorderLine.Color = aLineColor.GetColor();
                    // Avoid width = 0, the min value should be 1.
                    sal_Int32 nLineWidth = std::max(sal_Int32(1), static_cast<const XLineWidthItem&>(pLine->GetMergedItem(XATTR_LINEWIDTH)) .GetValue() / 4);
                    aBorderLine.LineWidth = static_cast< sal_Int16 >( nLineWidth );
                    aBorderLine.LineStyle = eLineStyle == drawing::LineStyle_SOLID ? table::BorderLineStyle::SOLID : table::BorderLineStyle::DASHED;
                }
                break;
            default:
            case drawing::LineStyle_NONE :
                {
                    aBorderLine.LineWidth = 0;
                    aBorderLine.LineStyle = table::BorderLineStyle::NONE;
                }
            break;
        }
        std::vector< sal_Int32 >::const_iterator aIter( vPositions.begin() );
        while( aIter != vPositions.end() )
        {
            sal_Int32 nPosition = *aIter & 0xffffff;
            sal_Int32 nFlags = *aIter &~0xffffff;
            sal_Int32 nRow = nPosition / nColumns;
            sal_Int32 nColumn = nPosition - ( nRow * nColumns );
            Reference< XCell > xCell( xTable->getCellByPosition( nColumn, nRow ) );
            Reference< XPropertySet > xPropSet( xCell, UNO_QUERY_THROW );

            if ( nFlags & LinePositionLeft )
                xPropSet->setPropertyValue( "LeftBorder", Any( aBorderLine ) );
            if ( nFlags & LinePositionTop )
                xPropSet->setPropertyValue( "TopBorder", Any( aBorderLine ) );
            if ( nFlags & LinePositionRight )
                xPropSet->setPropertyValue( "RightBorder", Any( aBorderLine ) );
            if ( nFlags & LinePositionBottom )
                xPropSet->setPropertyValue( "BottomBorder", Any( aBorderLine ) );
            if ( nFlags & LinePositionTLBR )
                xPropSet->setPropertyValue( "DiagonalTLBR", Any( true ) );
            if ( nFlags & LinePositionBLTR )
                xPropSet->setPropertyValue( "DiagonalBLTR", Any( true ) );
            ++aIter;
        }
    }
    catch( const Exception& )
    {
    }
}

SdrObject* SdrPowerPointImport::CreateTable( SdrObject* pGroup, sal_uInt32* pTableArry, SvxMSDffSolverContainer* pSolverContainer )
{
    SdrObject* pRet = pGroup;
    sal_uInt32 nRows = pTableArry[ 1 ];
    if ( nRows && dynamic_cast< const SdrObjGroup* >(pGroup) !=  nullptr )
    {
        SdrObjList* pSubList(static_cast<SdrObjGroup*>(pGroup)->GetSubList());
        if ( pSubList )
        {
            std::set< sal_Int32 > aRows;
            std::set< sal_Int32 > aColumns;

            SdrObjListIter aGroupIter( *pSubList, SdrIterMode::DeepNoGroups, false );
            while( aGroupIter.IsMore() )
            {
                const SdrObject* pObj( aGroupIter.Next() );
                if ( !IsLine( pObj ) )
                {
                    Rectangle aSnapRect( pObj->GetSnapRect() );
                    aRows.insert( aSnapRect.Top() );
                    aColumns.insert( aSnapRect.Left() );
                }
            }
            sdr::table::SdrTableObj* pTable = new sdr::table::SdrTableObj( pSdrModel );
            pTable->uno_lock();
            Reference< XTable > xTable( pTable->getTable() );
            try
            {
                CreateTableRows( xTable->getRows(), aRows, pGroup->GetSnapRect().Bottom() );
                CreateTableColumns( xTable->getColumns(), aColumns, pGroup->GetSnapRect().Right() );

                sal_Int32 nCellCount = aRows.size() * aColumns.size();
                std::unique_ptr<sal_Int32[]> pMergedCellIndexTable(new sal_Int32[ nCellCount ]);
                for ( sal_Int32 i = 0; i < nCellCount; i++ )
                    pMergedCellIndexTable[ i ] = i;

                aGroupIter.Reset();
                while( aGroupIter.IsMore() )
                {
                    SdrObject* pObj( aGroupIter.Next() );
                    if ( !IsLine( pObj ) )
                    {
                        sal_Int32 nTableIndex = 0;
                        sal_Int32 nRow = 0;
                        sal_Int32 nRowCount = 0;
                        sal_Int32 nColumn = 0;
                        sal_Int32 nColumnCount = 0;
                        if ( GetCellPosition( pObj, aRows, aColumns, nTableIndex, nRow, nRowCount, nColumn, nColumnCount ) )
                        {
                            Reference< XCell > xCell( xTable->getCellByPosition( nColumn, nRow ) );

                            ApplyCellAttributes( pObj, xCell );

                            if ( ( nRowCount > 1 ) || ( nColumnCount > 1 ) )    // cell merging
                            {
                                MergeCells( xTable, nColumn, nRow, nColumnCount, nRowCount );
                                for ( sal_Int32 nRowIter = 0; nRowIter < nRowCount; nRowIter++ )
                                {
                                    for ( sal_Int32 nColumnIter = 0; nColumnIter < nColumnCount; nColumnIter++ )
                                    {   // now set the correct index for the merged cell
                                        pMergedCellIndexTable[ ( ( nRow + nRowIter ) * aColumns.size() ) + nColumn + nColumnIter ] = nTableIndex;
                                    }
                                }
                            }

                            // applying text
                            OutlinerParaObject* pParaObject = pObj->GetOutlinerParaObject();
                            if ( pParaObject )
                            {
                                SdrText* pSdrText = pTable->getText( nTableIndex );
                                if ( pSdrText )
                                    pSdrText->SetOutlinerParaObject(new OutlinerParaObject(*pParaObject) );
                            }
                        }
                    }
                }
                aGroupIter.Reset();
                while( aGroupIter.IsMore() )
                {
                    SdrObject* pObj( aGroupIter.Next() );
                    if ( IsLine( pObj ) )
                    {
                        std::vector< sal_Int32 > vPositions;    // containing cell indexes + cell position
                        GetLinePositions( pObj, aRows, aColumns, vPositions, pGroup->GetSnapRect() );

                        // correcting merged cell position
                        std::vector< sal_Int32 >::iterator aIter( vPositions.begin() );
                        while( aIter != vPositions.end() )
                        {
                            sal_Int32 nOldPosition = *aIter & 0xffff;
                            sal_Int32 nOldFlags = *aIter & 0xffff0000;
                            sal_Int32 nNewPosition = pMergedCellIndexTable[ nOldPosition ] | nOldFlags;
                            *aIter++ = nNewPosition;
                        }
                        ApplyCellLineAttributes( pObj, xTable, vPositions, aColumns.size() );
                    }
                }
                pMergedCellIndexTable.reset();

                // we are replacing the whole group object by a single table object, so
                // possibly connections to the group object have to be removed.
                if ( pSolverContainer )
                {
                    for (SvxMSDffConnectorRule* pPtr : pSolverContainer->aCList)
                    {
                        // check connections to the group object
                        if ( pPtr->pAObj == pGroup )
                            pPtr->pAObj = nullptr;
                        if ( pPtr->pBObj == pGroup )
                            pPtr->pBObj = nullptr;

                        // check connections to all its subobjects
                        SdrObjListIter aIter( *pGroup, SdrIterMode::DeepWithGroups );
                        while( aIter.IsMore() )
                        {
                            SdrObject* pPartObj = aIter.Next();
                            if ( pPtr->pAObj == pPartObj )
                                pPtr->pAObj = nullptr;
                            if ( pPtr->pBObj == pPartObj )
                                pPtr->pBObj = nullptr;
                        }
                        //In MS, the one_row_one_col table is made up of five
                        //shape,the connector is connected to some part of a
                        //table.  But for us, the connector is connected to the
                        //whole group table,so the connector obj is a group
                        //table when export by us. We should process this
                        //situation when importing.
                        if ( pPtr->pAObj == pGroup )
                            pPtr->pAObj = pTable;
                        if ( pPtr->pBObj == pGroup )
                            pPtr->pBObj = pTable;
                    }
                }
                pTable->uno_unlock();
                pTable->SetSnapRect( pGroup->GetSnapRect() );
                pRet = pTable;

                //Remove Objects from shape map
                SdrObjListIter aIter( *pGroup, SdrIterMode::DeepWithGroups );
                while( aIter.IsMore() )
                {
                    SdrObject* pPartObj = aIter.Next();
                    removeShapeId( pPartObj );
                }

                SdrObject::Free( pGroup );
            }
            catch( const Exception& )
            {
                pTable->uno_unlock();
                SdrObject* pObj = pTable;
                SdrObject::Free( pObj );
            }
        }
    }
    return pRet;
}

bool SdrPowerPointImport::IsVerticalText() const
{
    bool bVerticalText = false;
    if ( IsProperty( DFF_Prop_txflTextFlow ) )
    {
        MSO_TextFlow eTextFlow = (MSO_TextFlow)( GetPropertyValue( DFF_Prop_txflTextFlow, 0 ) & 0xFFFF );
        switch( eTextFlow )
        {
        case mso_txflTtoBA :                    // Top to Bottom @-font, above -> below
        case mso_txflTtoBN :                    // Top to Bottom non-@, above -> below
        case mso_txflVertN :                    // Vertical, non-@, above -> below
            bVerticalText = !bVerticalText;
            break;
        default: break;
        }
    }

    return bVerticalText;
}

void    SdrPowerPointImport::ApplyTextAnchorAttributes( PPTTextObj& rTextObj, SfxItemSet& rSet ) const
{
    SdrTextVertAdjust eTVA;
    SdrTextHorzAdjust eTHA;

    sal_uInt32 nTextFlags = rTextObj.GetTextFlags();

    nTextFlags &= PPT_TEXTOBJ_FLAGS_PARA_ALIGNMENT_USED_LEFT   | PPT_TEXTOBJ_FLAGS_PARA_ALIGNMENT_USED_RIGHT
        | PPT_TEXTOBJ_FLAGS_PARA_ALIGNMENT_USED_CENTER | PPT_TEXTOBJ_FLAGS_PARA_ALIGNMENT_USED_BLOCK;

    if ( IsVerticalText() )
    {
        eTVA = SDRTEXTVERTADJUST_BLOCK;
        eTHA = SDRTEXTHORZADJUST_CENTER;

        // Textverankerung lesen
        MSO_Anchor eTextAnchor = (MSO_Anchor)GetPropertyValue( DFF_Prop_anchorText, mso_anchorTop );

        switch( eTextAnchor )
        {
        case mso_anchorTop:
        case mso_anchorTopCentered:
            eTHA = SDRTEXTHORZADJUST_RIGHT;
            break;

        case mso_anchorMiddle :
        case mso_anchorMiddleCentered:
            eTHA = SDRTEXTHORZADJUST_CENTER;
            break;

        case mso_anchorBottom:
        case mso_anchorBottomCentered:
            eTHA = SDRTEXTHORZADJUST_LEFT;
            break;

        default:
            break;
        }
        // if there is a 100% use of following attributes, the textbox can been aligned also in vertical direction
        switch ( eTextAnchor )
        {
        case mso_anchorTopCentered :
        case mso_anchorMiddleCentered :
        case mso_anchorBottomCentered :
            {
                // check if it is sensible to use the centered alignment
                sal_uInt32 nMask = PPT_TEXTOBJ_FLAGS_PARA_ALIGNMENT_USED_LEFT | PPT_TEXTOBJ_FLAGS_PARA_ALIGNMENT_USED_RIGHT;
                if ( ( nTextFlags & nMask ) != nMask )  // if the textobject has left or also right aligned pararagraphs
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
            eTVA = SDRTEXTVERTADJUST_TOP;
            break;

        case mso_anchorMiddle :
        case mso_anchorMiddleCentered:
            eTVA = SDRTEXTVERTADJUST_CENTER;
            break;

        case mso_anchorBottom:
        case mso_anchorBottomCentered:
            eTVA = SDRTEXTVERTADJUST_BOTTOM;
            break;

        default:
            break;
        }

        // if there is a 100% usage of following attributes, the textbox can be aligned also in horizontal direction
        switch ( eTextAnchor )
        {
        case mso_anchorTopCentered :
        case mso_anchorMiddleCentered :
        case mso_anchorBottomCentered :
            {
                // check if it is sensible to use the centered alignment
                sal_uInt32 nMask = PPT_TEXTOBJ_FLAGS_PARA_ALIGNMENT_USED_LEFT | PPT_TEXTOBJ_FLAGS_PARA_ALIGNMENT_USED_RIGHT;
                if ( ( nTextFlags & nMask ) != nMask )  // if the textobject has left or also right aligned pararagraphs
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
    }
    rSet.Put( SdrTextVertAdjustItem( eTVA ) );
    rSet.Put( SdrTextHorzAdjustItem( eTHA ) );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
