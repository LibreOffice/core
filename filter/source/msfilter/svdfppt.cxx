/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include <osl/endian.h>
#include <vcl/svapp.hxx>
#include <unotools/tempfile.hxx>
#include <math.h>
#include <editeng/eeitem.hxx>
#include <sot/storage.hxx>
#include <sot/storinfo.hxx>
#include <sot/stg.hxx>
#include <com/sun/star/embed/Aspects.hpp>
#include <com/sun/star/office/XAnnotation.hpp>
#include <com/sun/star/office/XAnnotationAccess.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/geometry/RealPoint2D.hpp>
#include <com/sun/star/util/DateTime.hpp>
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
#include <svx/svdattr.hxx>
#include "svx/xattr.hxx"
#include "svx/svditext.hxx"
#include <svx/svdetc.hxx>
#include <editeng/bulitem.hxx>
#include <svx/polysc3d.hxx>
#include <svx/extrud3d.hxx>
#include <svx/svdoashp.hxx>
#include <editeng/tstpitem.hxx>
#include <editeng/unoprnms.hxx>
#include <editeng/editids.hrc>

#define ITEMVALUE(ItemSet,Id,Cast)  ((const Cast&)(ItemSet).Get(Id)).GetValue()
#include <editeng/adjitem.hxx>
#include <editeng/escpitem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/crsditem.hxx>
#include <editeng/shdditem.hxx>
#include <editeng/charreliefitem.hxx>
#include <editeng/fontitem.hxx>
#include <svx/svdoutl.hxx>
#include <editeng/editeng.hxx>
#include <editeng/lspcitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/lrspitem.hxx>
#include <vcl/metric.hxx>
#include <vcl/bmpacc.hxx>
#include <svx/svditer.hxx>
#include <svx/svdoedge.hxx>
#include <svx/sxekitm.hxx>
#include <editeng/flditem.hxx>
#include <tools/zcodec.hxx>
#include <filter/msfilter/svxmsbas.hxx>
#include <sfx2/objsh.hxx>
#include <editeng/brshitem.hxx>
#include <editeng/langitem.hxx>
#include <svx/svdoole2.hxx>
#include <svx/unoapi.hxx>
#include <toolkit/unohlp.hxx>
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
#include <algorithm>
#include <set>
#include <rtl/strbuf.hxx>

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

SvStream& operator>>( SvStream& rIn, PptCurrentUserAtom& rAtom )
{
    DffRecordHeader aHd;
    rIn >> aHd;
    if ( aHd.nRecType == PPT_PST_CurrentUserAtom )
    {
        sal_uInt32 nLen;
        sal_uInt16 nUserNameLen, nPad;
        rIn >> nLen
            >> rAtom.nMagic
            >> rAtom.nCurrentUserEdit
            >> nUserNameLen
            >> rAtom.nDocFileVersion
            >> rAtom.nMajorVersion
            >> rAtom.nMinorVersion
            >> nPad;
        rAtom.aCurrentUser = SvxMSDffManager::MSDFFReadZString( rIn, nUserNameLen, sal_True );
    }
    aHd.SeekToEndOfRecord( rIn );
    return rIn;
}

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
    aHd.SeekToEndOfRecord( rIn );
    return rIn;
}

sal_uInt16 PptSlidePersistList::FindPage(sal_uInt32 nId) const
{
    for ( sal_uInt16 i=0; i < size(); i++ )
    {
        if (operator[](i)->GetSlideId()==nId) return i;
    }
    return PPTSLIDEPERSIST_ENTRY_NOTFOUND;
}

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
    rIn >> rAtom.nAspect
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
    sal_Int32   nSlideX,nSlideY, nNoticeX, nNoticeY, nDummy;
    sal_uInt16  nSlidePageFormat;
    sal_Int8    nEmbeddedTrueType, nTitlePlaceHoldersOmitted, nRightToLeft, nShowComments;

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

void PptSlideLayoutAtom::Clear()
{
    eLayout = 0;
    for ( sal_uInt16 i = 0; i < 8; i++ )
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

void PptColorSchemeAtom::Clear()
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

SvStream& operator>>( SvStream& rIn, PptColorSchemeAtom& rAtom )
{
    DffRecordHeader aHd;
    rIn >> aHd;
    rIn.Read( rAtom.aData, 32 );
    aHd.SeekToEndOfRecord( rIn );
    return rIn;
}

SvStream& operator>>( SvStream& rIn, PptFontEntityAtom& rAtom )
{
    DffRecordHeader aHd;
    rIn >> aHd;
    sal_Unicode nTemp, cData[ 32 ];
    rIn.Read( cData, 64 );

    sal_uInt8   lfCharset, lfPitchAndFamily;

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
    rAtom.aName = rtl::OUString(cData, i);
    OutputDevice* pDev = (OutputDevice*)Application::GetDefaultDevice();
    rAtom.bAvailable = pDev->IsFontAvailable( rAtom.aName );
    aHd.SeekToEndOfRecord( rIn );
    return rIn;
}

class PptFontCollection: public boost::ptr_vector<PptFontEntityAtom> {
};

SvStream& operator>>( SvStream& rIn, PptUserEditAtom& rAtom )
{
    rIn >> rAtom.aHd
        >> rAtom.nLastSlideID
        >> rAtom.nVersion
        >> rAtom.nOffsetLastEdit
        >> rAtom.nOffsetPersistDirectory
        >> rAtom.nDocumentRef
        >> rAtom.nMaxPersistWritten
        >> rAtom.eLastViewType;
    rAtom.aHd.SeekToEndOfRecord(rIn);
    return rIn;
}

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

PptSlidePersistEntry::PptSlidePersistEntry() :
    pStyleSheet             ( NULL ),
    pHeaderFooterEntry      ( NULL ),
    pSolverContainer        ( NULL ),
    nSlidePersistStartOffset( 0 ),
    nSlidePersistEndOffset  ( 0 ),
    nBackgroundOffset       ( 0 ),
    nDrawingDgId            ( 0xffffffff ),
    pPresentationObjects    ( NULL ),
    pBObj                   ( NULL ),
    bBObjIsTemporary        ( sal_True ),
    ePageKind               ( PPT_MASTERPAGE ),
    bNotesMaster            ( sal_False ),
    bHandoutMaster          ( sal_False ),
    bStarDrawFiller         ( sal_False )
{
    HeaderFooterOfs[ 0 ] =  HeaderFooterOfs[ 1 ] = HeaderFooterOfs[ 2 ] = HeaderFooterOfs[ 3 ] = 0;
}


PptSlidePersistEntry::~PptSlidePersistEntry()
{
    delete pStyleSheet;
    delete pSolverContainer;
    delete[] pPresentationObjects;
};

SdrEscherImport::SdrEscherImport( PowerPointImportParam& rParam, const String& rBaseURL ) :
    SvxMSDffManager         ( rParam.rDocStream, rBaseURL ),
    pFonts                  ( NULL ),
    nStreamLen              ( 0 ),
    nTextStylesIndex        ( 0xffff ),
    eCharSetSystem          ( osl_getThreadTextEncoding() ),
    bWingdingsChecked       ( sal_False ),
    bWingdingsAvailable     ( sal_False ),
    bMonotypeSortsChecked   ( sal_False ),
    bMonotypeSortsAvailable ( sal_False ),
    bTimesNewRomanChecked   ( sal_False ),
    bTimesNewRomanAvailable ( sal_False ),
    rImportParam            ( rParam )
{
}

SdrEscherImport::~SdrEscherImport()
{
    for ( size_t i = 0, n = aOleObjectList.size(); i < n; ++i )
        delete aOleObjectList[ i ];
    aOleObjectList.clear();
    delete pFonts;
}

const PptSlideLayoutAtom* SdrEscherImport::GetSlideLayoutAtom() const
{
    return NULL;
}

sal_Bool SdrEscherImport::ReadString( rtl::OUString& rStr ) const
{
    sal_Bool bRet = sal_False;
    DffRecordHeader aStrHd;
    rStCtrl >> aStrHd;
    if (aStrHd.nRecType == PPT_PST_TextBytesAtom
        || aStrHd.nRecType == PPT_PST_TextCharsAtom
        || aStrHd.nRecType == PPT_PST_CString)
    {
        sal_Bool bUniCode =
            (aStrHd.nRecType == PPT_PST_TextCharsAtom
            || aStrHd.nRecType == PPT_PST_CString);
        bRet=sal_True;
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
    return sal_False;
}

sal_Bool SdrEscherImport::SeekToShape( SvStream& /*rSt*/, void* /*pClientData*/, sal_uInt32 /*nId*/) const
{
    return sal_False;
}

PptFontEntityAtom* SdrEscherImport::GetFontEnityAtom( sal_uInt32 nNum ) const
{
    PptFontEntityAtom* pRetValue = NULL;
    if ( pFonts && ( nNum < pFonts->size() ) )
        pRetValue = &(*pFonts)[ (sal_uInt16)nNum ];
    return pRetValue;
}

SdrObject* SdrEscherImport::ReadObjText( PPTTextObj* /*pTextObj*/, SdrObject* pObj, SdPage* /*pPage*/) const
{
    return pObj;
}

void SdrEscherImport::ProcessClientAnchor2( SvStream& rSt, DffRecordHeader& rHd, void* /*pData*/, DffObjData& rObj )
{
    sal_Int32 l, t, r, b;
    if ( rHd.nRecLen == 16 )
    {
        rSt >> l >> t >> r >> b;
    }
    else
    {
        sal_Int16 ls, ts, rs, bs;
        rSt >> ts >> ls >> rs >> bs; // etwas seltsame Koordinatenreihenfolge ...
        l = ls, t = ts, r = rs, b = bs;
    }
    Scale( l );
    Scale( t );
    Scale( r );
    Scale( b );
    rObj.aChildAnchor = Rectangle( l, t, r, b );
    rObj.bChildAnchor = sal_True;
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

                        pSearchColors[ j ].SetRed( (sal_uInt8)nSearch );
                        pSearchColors[ j ].SetGreen( (sal_uInt8)( nSearch >> 8 ) );
                        pSearchColors[ j ].SetBlue( (sal_uInt8)( nSearch >> 16 ) );

                        pReplaceColors[ j ].SetRed( (sal_uInt8)nReplace );
                        pReplaceColors[ j ].SetGreen( (sal_uInt8)( nReplace >> 8 ) );
                        pReplaceColors[ j ].SetBlue( (sal_uInt8)( nReplace >> 16 ) );
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

/* ProcessObject is called from ImplSdPPTImport::ProcessObj to hanlde all application specific things,
   such as the import of text, animation effects, header footer and placeholder.

   The parameter pOriginalObj is the object as it was imported by our general escher import, it must either
   be deleted or it can be returned to be inserted into the sdr page.
*/
SdrObject* SdrEscherImport::ProcessObj( SvStream& rSt, DffObjData& rObjData, void* pData, Rectangle& rTextRect, SdrObject* pOriginalObj )
{
    if ( pOriginalObj && pOriginalObj->ISA( SdrObjCustomShape ) )
        pOriginalObj->SetMergedItem( SdrTextFixedCellHeightItem( sal_True ) );

    // we are initializing our return value with the object that was imported by our escher import
    SdrObject* pRet = pOriginalObj;

    ProcessData& rData = *((ProcessData*)pData);
    PptSlidePersistEntry& rPersistEntry = rData.rPersistEntry;

    if ( ! ( rObjData.nSpFlags & SP_FGROUP  ) )     // sj: #114758# ...
    {
        PptOEPlaceholderAtom aPlaceholderAtom;

        if ( maShapeRecords.SeekToContent( rSt, DFF_msofbtClientData, SEEK_FROM_CURRENT_AND_RESTART ) )
        {
            sal_Int16 nHeaderFooterInstance = -1;
            DffRecordHeader aClientDataHd;
            while ( ( rSt.GetError() == 0 ) && ( rSt.Tell() < maShapeRecords.Current()->GetRecEndFilePos() ) )
            {
                rSt >> aClientDataHd;
                switch ( aClientDataHd.nRecType )
                {
                    // importing header/footer object from master page
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
                                rPersistEntry.HeaderFooterOfs[ nHeaderFooterInstance ] = rObjData.rSpHd.GetRecBegFilePos();
                        }
                    }
                    break;

                    case PPT_PST_RecolorInfoAtom :
                    {
                        if ( pRet && ( pRet->ISA( SdrGrafObj ) && ((SdrGrafObj*)pRet)->HasGDIMetaFile() ) )
                        {
                            Graphic aGraphic( ((SdrGrafObj*)pRet)->GetGraphic() );
                            RecolorGraphic( rSt, aClientDataHd.nRecLen, aGraphic );
                            ((SdrGrafObj*)pRet)->SetGraphic( aGraphic );
                        }
                    }
                    break;
                }
                aClientDataHd.SeekToEndOfRecord( rSt );
            }
        }
        if ( ( aPlaceholderAtom.nPlaceholderId == PPT_PLACEHOLDER_NOTESSLIDEIMAGE ) && ( rPersistEntry.bNotesMaster == sal_False ) )
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
            PPTTextObj aTextObj( rSt, (SdrPowerPointImport&)*this, rPersistEntry, &rObjData );
            if ( ( aTextObj.Count() || aTextObj.GetOEPlaceHolderAtom() ) )
            {
                sal_Bool bVerticalText = sal_False;
                // and if the text object is not empty, it must be applied to pRet, the object we
                // initially got from our escher import
                sal_Int32 nTextRotationAngle = 0;
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
                            bVerticalText = !bVerticalText;     // nTextRotationAngle += 27000;
                        break;
    //                  case mso_txflHorzN :                    // Horizontal non-@, normal
    //                  case mso_txflHorzA :                    // Horizontal @-font, normal
                        default: break;
                    }
                }
                sal_Int32 nFontDirection = GetPropertyValue( DFF_Prop_cdirFont, mso_cdir0 );
                nTextRotationAngle -= nFontDirection * 9000;
                if ( ( nFontDirection == 1 ) || ( nFontDirection == 3 ) )       // #104546#
                {
                    bVerticalText = !bVerticalText;
                }
                aTextObj.SetVertical( bVerticalText );
                if ( pRet )
                {
                    sal_Bool bDeleteSource = aTextObj.GetOEPlaceHolderAtom() != 0;
                    if ( bDeleteSource  && ( pRet->ISA( SdrGrafObj ) == sal_False )     // we are not allowed to get
                            && ( pRet->ISA( SdrObjGroup ) == sal_False )                // grouped placeholder objects
                                && ( pRet->ISA( SdrOle2Obj ) == sal_False ) )
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

                sal_uInt32 nDestinationInstance = aTextObj.GetInstance();
                if ( rPersistEntry.ePageKind == PPT_MASTERPAGE )
                {
                    if ( !rPersistEntry.pPresentationObjects )
                    {
                        rPersistEntry.pPresentationObjects = new sal_uInt32[ PPT_STYLESHEETENTRYS ];
                        memset( rPersistEntry.pPresentationObjects, 0, PPT_STYLESHEETENTRYS * 4 );
                    }
                    if ( !rPersistEntry.pPresentationObjects[ nDestinationInstance ] )
                        rPersistEntry.pPresentationObjects[ nDestinationInstance ] = rObjData.rSpHd.GetRecBegFilePos();
                }
                switch ( nDestinationInstance )
                {
                    case TSS_TYPE_PAGETITLE :
                    case TSS_TYPE_TITLE :
                    {
                        if ( GetSlideLayoutAtom()->eLayout == PPT_LAYOUT_TITLEMASTERSLIDE )
                            nDestinationInstance = TSS_TYPE_TITLE;
                        else
                            nDestinationInstance = TSS_TYPE_PAGETITLE;
                    }
                    break;
                    case TSS_TYPE_BODY :
                    case TSS_TYPE_HALFBODY :
                    case TSS_TYPE_QUARTERBODY :
                        nDestinationInstance = TSS_TYPE_BODY;
                    break;
                }
                aTextObj.SetDestinationInstance( (sal_uInt16)nDestinationInstance );

                bool bAutoFit = false; // auto-scale text into shape box
                switch ( aTextObj.GetInstance() )
                {
                    case TSS_TYPE_PAGETITLE :
                    case TSS_TYPE_TITLE : eTextKind = OBJ_TITLETEXT; break;
                    case TSS_TYPE_SUBTITLE : eTextKind = OBJ_TEXT; break;
                    case TSS_TYPE_BODY :
                    case TSS_TYPE_HALFBODY :
                    case TSS_TYPE_QUARTERBODY : eTextKind = OBJ_OUTLINETEXT; bAutoFit = true; break;
                }
                if ( aTextObj.GetDestinationInstance() != TSS_TYPE_TEXT_IN_SHAPE )
                {
                    if ( !aTextObj.GetOEPlaceHolderAtom() || !aTextObj.GetOEPlaceHolderAtom()->nPlaceholderId )
                    {
                        aTextObj.SetDestinationInstance( TSS_TYPE_TEXT_IN_SHAPE );
                        eTextKind = OBJ_RECT;
                    }
                }
                SdrObject* pTObj = NULL;
                sal_Bool bWordWrap = (MSO_WrapMode)GetPropertyValue( DFF_Prop_WrapText, mso_wrapSquare ) != mso_wrapNone;
                sal_Bool bFitShapeToText = ( GetPropertyValue( DFF_Prop_FitTextToShape ) & 2 ) != 0;

                if ( pRet && pRet->ISA( SdrObjCustomShape ) && ( eTextKind == OBJ_RECT ) )
                {
                    bAutoGrowHeight = bFitShapeToText;
                    if ( bWordWrap )
                        bAutoGrowWidth = sal_False;
                    else
                        bAutoGrowWidth = sal_True;
                    pTObj = pRet;
                    pRet = NULL;
                }
                else
                {
                    if ( pRet && pRet->ISA( SdrObjCustomShape ) )
                    {
                        SdrObject::Free( pRet );
                        pRet = NULL;
                    }
                    pTObj = new SdrRectObj( eTextKind != OBJ_RECT ? eTextKind : OBJ_TEXT );
                    pTObj->SetModel( pSdrModel );
                    SfxItemSet aSet( pSdrModel->GetItemPool() );
                    if ( !pRet )
                        ((SdrEscherImport*)this)->ApplyAttributes( rSt, aSet, rObjData );
                    pTObj->SetMergedItemSet( aSet );
                    if ( pRet )
                    {
                        pTObj->SetMergedItem( XLineStyleItem( XLINE_NONE ) );
                        pTObj->SetMergedItem( XFillStyleItem( XFILL_NONE ) );
                    }
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
                pTObj->SetMergedItem( SvxFrameDirectionItem( bVerticalText ? FRMDIR_VERT_TOP_RIGHT : FRMDIR_HORI_LEFT_TOP, EE_PARA_WRITINGDIR ) );

                //Autofit text only if there is no auto grow height and width
                //See fdo#41245
                if (bAutoFit && !bAutoGrowHeight && !bAutoGrowWidth)
                {
                    pTObj->SetMergedItem( SdrTextFitToSizeTypeItem(SDRTEXTFIT_AUTOFIT) );
                }

            if ( !pTObj->ISA( SdrObjCustomShape ) )
            {
                 pTObj->SetMergedItem( SdrTextAutoGrowWidthItem( bAutoGrowWidth ) );
                pTObj->SetMergedItem( SdrTextAutoGrowHeightItem( bAutoGrowHeight ) );
            }
            else
            {
                pTObj->SetMergedItem( SdrTextWordWrapItem( bWordWrap ) );
                pTObj->SetMergedItem( SdrTextAutoGrowHeightItem( bFitShapeToText ) );
            }

                pTObj->SetMergedItem( SdrTextVertAdjustItem( eTVA ) );
                pTObj->SetMergedItem( SdrTextHorzAdjustItem( eTHA ) );

                if ( nMinFrameHeight < 0 )
                    nMinFrameHeight = 0;
            if ( !pTObj->ISA( SdrObjCustomShape ) )
                pTObj->SetMergedItem( SdrTextMinFrameHeightItem( nMinFrameHeight ) );

                if ( nMinFrameWidth < 0 )
                    nMinFrameWidth = 0;
            if ( !pTObj->ISA( SdrObjCustomShape ) )
                pTObj->SetMergedItem( SdrTextMinFrameWidthItem( nMinFrameWidth ) );

                // Abstaende an den Raendern der Textbox setzen
                pTObj->SetMergedItem( SdrTextLeftDistItem( nTextLeft ) );
                pTObj->SetMergedItem( SdrTextRightDistItem( nTextRight ) );
                pTObj->SetMergedItem( SdrTextUpperDistItem( nTextTop ) );
                pTObj->SetMergedItem( SdrTextLowerDistItem( nTextBottom ) );
                pTObj->SetMergedItem( SdrTextFixedCellHeightItem( sal_True ) );

                if ( !pTObj->ISA( SdrObjCustomShape ) )
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
                    if ( !pTObj->ISA( SdrObjCustomShape ) && !bFitShapeToText && !bWordWrap )
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
                    // rotate text with shape ?
                    sal_Int32 nAngle = ( rObjData.nSpFlags & SP_FFLIPV ) ? -mnFix16Angle : mnFix16Angle;    // #72116# vertical flip -> rotate by using the other way
                    nAngle += nTextRotationAngle;

                    if ( !pTObj->ISA( SdrObjCustomShape ) )
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
            aSecPropSet.ReadPropSet( rSt, (ProcessData*)pData );
            sal_Int32 nTableProperties = aSecPropSet.GetPropertyValue( DFF_Prop_tableProperties, 0 );
            if ( nTableProperties & 3 )
            {
                if ( aSecPropSet.SeekToContent( DFF_Prop_tableRowProperties, rSt ) )
                {
                    sal_Int16 i, nRowCount = 0;
                    rSt >> nRowCount >> i >> i;
                    if ( nRowCount )
                    {
                        sal_uInt32* pTableArry = new sal_uInt32[ nRowCount + 2 ];
                        pTableArry[ 0 ] = nTableProperties;
                        pTableArry[ 1 ] = nRowCount;
                        for ( i = 0; i < nRowCount; i++ )
                            rSt >> pTableArry[ i + 2 ];
                        rData.pTableRowProperties = pTableArry;
                    }
                }
            }
        }
    }
    if ( pRet ) // sj: #i38501#, and and taking care of connections to group objects
    {
        if ( rObjData.nSpFlags & SP_FBACKGROUND )
        {
            pRet->NbcSetSnapRect( Rectangle( Point(), ((SdrPage*)rData.pPage)->GetSize() ) );   // Groesse setzen
        }
        if ( rPersistEntry.pSolverContainer )
        {
            for ( size_t i = 0; i < rPersistEntry.pSolverContainer->aCList.size(); ++i )
            {
                SvxMSDffConnectorRule* pPtr = rPersistEntry.pSolverContainer->aCList[ i ];
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
                sal_uInt32 nSec = ( rObjData.nShapeId >> 10 ) - 1;
                if ( !maFidcls.empty() && ( nSec < mnIdClusters ) )
                    maFidcls[ nSec ].dgid = rPersistEntry.nDrawingDgId; // insert the correct drawing id;
            }
        }
        if ( GetPropertyValue( DFF_Prop_fNoFillHitTest ) & 0x10 )
        {
            if ( (MSO_FillType)GetPropertyValue( DFF_Prop_fillType, mso_fillSolid ) == mso_fillBackground )
            {
                rData.aBackgroundColoredObjects.push_back( pRet );
            }
        }
    }
    return pRet;
}

SdrPowerPointImport::SdrPowerPointImport( PowerPointImportParam& rParam, const String& rBaseURL ) :
    SdrEscherImport     ( rParam, rBaseURL ),
    bOk                 ( rStCtrl.GetErrorCode() == SVSTREAM_OK ),
    pPersistPtr         ( NULL ),
    nPersistPtrAnz      ( 0 ),
    pDefaultSheet       ( NULL ),
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

        // try to allocate the UserEditAtom via CurrentUserAtom
        sal_uInt32 nCurrentUserEdit = rParam.aCurrentUserAtom.nCurrentUserEdit;
        if ( nCurrentUserEdit )
        {
            rStCtrl.Seek( nCurrentUserEdit );
            rStCtrl >> aUserEditAtom;
        }
        if ( !aUserEditAtom.nOffsetPersistDirectory )
        {   // if there is no UserEditAtom try to search the last one

            rStCtrl.Seek( 0 );
            DffRecordManager aPptRecManager;                            // contains all first level container and atoms
            aPptRecManager.Consume( rStCtrl, sal_False, nStreamLen );
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
                bOk = sal_False;
        }
    }
    if ( rStCtrl.GetError() != 0 )
        bOk = sal_False;

    if ( bOk )
    {
        nPersistPtrAnz = aUserEditAtom.nMaxPersistWritten + 1;
        if ( ( nPersistPtrAnz >> 2 ) > nStreamLen )     // sj: at least nPersistPtrAnz is not allowed to be greater than filesize
            bOk = sal_False;                                // (it should not be greater than the PPT_PST_PersistPtrIncrementalBlock, but
                                                        // we are reading this block later, so we do not have access yet)

        if ( bOk && ( nPersistPtrAnz < ( SAL_MAX_UINT32 / sizeof( sal_uInt32 ) ) ) )
            pPersistPtr = new (std::nothrow) sal_uInt32[ nPersistPtrAnz ];
        if ( !pPersistPtr )
            bOk = sal_False;
        if ( bOk )
        {
            memset( pPersistPtr, 0x00, nPersistPtrAnz * 4 );

            // SJ: new search mechanism from bottom to top (Issue 21122)
            PptUserEditAtom aCurrentEditAtom( aUserEditAtom );
            sal_uInt32 nCurrentEditAtomStrmPos = aCurrentEditAtom.aHd.GetRecEndFilePos();
            while( nCurrentEditAtomStrmPos )
            {
                sal_uInt32 nPersistIncPos = aCurrentEditAtom.nOffsetPersistDirectory;
                if ( nPersistIncPos )
                {
                    rStCtrl.Seek( nPersistIncPos );
                    DffRecordHeader aPersistHd;
                    rStCtrl >> aPersistHd;
                    if ( aPersistHd.nRecType == PPT_PST_PersistPtrIncrementalBlock )
                    {
                        sal_uLong nPibLen = aPersistHd.GetRecEndFilePos();
                        while ( bOk && ( rStCtrl.GetError() == 0 ) && ( rStCtrl.Tell() < nPibLen ) )
                        {
                            sal_uInt32 nOfs, nAnz;
                            rStCtrl >> nOfs;
                            nAnz = nOfs;
                            nOfs &= 0x000FFFFF;
                            nAnz >>= 20;
                            while ( bOk && ( rStCtrl.GetError() == 0 ) && ( nAnz > 0 ) && ( nOfs <= nPersistPtrAnz ) )
                            {
                                sal_uInt32 nPt;
                                rStCtrl >> nPt;
                                if ( !pPersistPtr[ nOfs ] )
                                {
                                    pPersistPtr[ nOfs ] = nPt;
                                    if ( pPersistPtr[ nOfs ] > nStreamLen )
                                    {
                                        bOk = sal_False;
                                        OSL_FAIL("SdrPowerPointImport::Ctor(): Ungueltiger Eintrag im Persist-Directory!");
                                    }
                                }
                                nAnz--;
                                nOfs++;
                            }
                            if ( bOk && nAnz > 0 )
                            {
                                OSL_FAIL("SdrPowerPointImport::Ctor(): Nicht alle Persist-Directory Entraege gelesen!");
                                bOk = sal_False;
                            }
                        }
                    }
                }
                nCurrentEditAtomStrmPos = aCurrentEditAtom.nOffsetLastEdit < nCurrentEditAtomStrmPos ? aCurrentEditAtom.nOffsetLastEdit : 0;
                if ( nCurrentEditAtomStrmPos )
                {
                    rStCtrl.Seek( nCurrentEditAtomStrmPos );
                    rStCtrl >> aCurrentEditAtom;
                }
            }
        }
    }
    if ( rStCtrl.GetError() != 0 )
        bOk = sal_False;
    if ( bOk )
    {   // Document PersistEntry checken
        nDocStreamPos = aUserEditAtom.nDocumentRef;
        if ( nDocStreamPos > nPersistPtrAnz )
        {
            OSL_FAIL("SdrPowerPointImport::Ctor(): aUserEditAtom.nDocumentRef ungueltig!");
            bOk = sal_False;
        }
    }
    if ( bOk )
    {   // Document FilePos checken
        nDocStreamPos = pPersistPtr[ nDocStreamPos ];
        if ( nDocStreamPos >= nStreamLen )
        {
            OSL_FAIL("SdrPowerPointImport::Ctor(): nDocStreamPos >= nStreamLen!");
            bOk = sal_False;
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
            bOk = sal_False;

        if ( bOk )
        {
            if ( !pFonts )
                ReadFontCollection();

            // reading TxPF, TxSI
            PPTTextCharacterStyleAtomInterpreter    aTxCFStyle; // SJ: ToDo, this atom needs to be interpreted, it contains character default styles for standard objects (instance4)
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
            pMasterPages->insert( pMasterPages->begin(), pE );

            sal_uInt16 nPageListNum = 0;
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
                    PptSlidePersistEntry* pE2 = new PptSlidePersistEntry;
                    rStCtrl >> pE2->aPersistAtom;
                    pE2->nSlidePersistStartOffset = rStCtrl.Tell();
                    pE2->ePageKind = PptPageKind( nPageListNum );
                    pPageList->push_back( pE2 );
                    pPreviousPersist = pE2;
                }
                if ( pPreviousPersist )
                    pPreviousPersist->nSlidePersistEndOffset = nSlideListWithTextHdEndOffset;
                pSlideListWithTextHd = aDocRecManager.GetRecordHeader( PPT_PST_SlideListWithText, SEEK_FROM_CURRENT );
                nPageListNum++;
            }

            // we will ensure that there is at least one master page
            if ( pMasterPages->size() == 1 )   // -> there is only a handout page available
            {
                PptSlidePersistEntry* pE2 = new PptSlidePersistEntry;
                pE2->bStarDrawFiller = sal_True;            // this is a dummy master page
                pMasterPages->insert( pMasterPages->begin() + 1, pE2 );
            }

            // now we will insert at least one notes master for each master page
            sal_uInt16 nMasterPage;
            sal_uInt16 nMasterPages = pMasterPages->size() - 1;
            for ( nMasterPage = 0; nMasterPage < nMasterPages; nMasterPage++ )
            {
                PptSlidePersistEntry* pE2 = new PptSlidePersistEntry;
                pE2->bNotesMaster = sal_True;
                pE2->bStarDrawFiller = sal_True;            // this is a dummy master page
                if ( !nMasterPage && aDocAtom.nNotesMasterPersist )
                {   // special treatment for the first notes master
                    pE2->aPersistAtom.nPsrReference = aDocAtom.nNotesMasterPersist;
                    pE2->bStarDrawFiller = sal_False;   // this is a dummy master page
                }
                pMasterPages->insert( pMasterPages->begin() + (( nMasterPage + 1 ) << 1), pE2 );
            }

            // Zu jeder Page noch das SlideAtom bzw. NotesAtom lesen, soweit vorhanden
            nPageListNum = 0;
            for ( nPageListNum = 0; nPageListNum < 3; nPageListNum++ )
            {
                PptSlidePersistList* pPageList = GetPageList( PptPageKind( nPageListNum ) );
                for ( sal_uInt16 nPageNum = 0; nPageNum < pPageList->size(); nPageNum++ )
                {
                    PptSlidePersistEntry* pE2 = (*pPageList)[ nPageNum ];
                    sal_uLong nPersist = pE2->aPersistAtom.nPsrReference;
                    if ( ( nPersist > 0 ) && ( nPersist < nPersistPtrAnz ) )
                    {
                        sal_uLong nFPos = pPersistPtr[ nPersist ];
                        if ( nFPos < nStreamLen )
                        {
                            rStCtrl.Seek( nFPos );
                            DffRecordHeader aSlideHd;
                            rStCtrl >> aSlideHd;
                            if ( SeekToRec( rStCtrl, PPT_PST_SlideAtom, aSlideHd.GetRecEndFilePos() ) )
                                rStCtrl >> pE2->aSlideAtom;
                            else if ( SeekToRec( rStCtrl, PPT_PST_NotesAtom, aSlideHd.GetRecEndFilePos() ) )
                                rStCtrl >> pE2->aNotesAtom;
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
                                        pE2->nDrawingDgId = aDgRecordHeader.nRecInstance;
                                        aDgRecordHeader.SeekToEndOfRecord( rStCtrl );
                                    }
                                    if ( SeekToRec( rStCtrl, DFF_msofbtSolverContainer, aPPTDgContainer.GetRecEndFilePos() ) )
                                    {
                                        pE2->pSolverContainer = new SvxMSDffSolverContainer;
                                        rStCtrl >> *( pE2->pSolverContainer );
                                    }
                                    aPPTDgContainer.SeekToBegOfRecord( rStCtrl );
                                    SetDgContainer( rStCtrl );  // set this, so that the escherimport is knowing of our drawings
                                }
                            }
                            // office xp is supporting more than one stylesheet
                            if ( ( pE2->ePageKind == PPT_MASTERPAGE ) && ( pE2->aSlideAtom.nMasterId == 0 ) && ( pE2->bNotesMaster == 0 ) )
                            {
                                PPTTextSpecInfo aTxSI( 0 );
                                if ( aTxSIStyle.bValid && !aTxSIStyle.aList.empty() )
                                    aTxSI = *( aTxSIStyle.aList[ 0 ] );

                                pE2->pStyleSheet = new PPTStyleSheet( aSlideHd, rStCtrl, *this, aTxCFStyle, aTxPFStyle, aTxSI );
                                pDefaultSheet = pE2->pStyleSheet;
                            }
                            if ( SeekToRec( rStCtrl, PPT_PST_ColorSchemeAtom, aSlideHd.GetRecEndFilePos() ) )
                                rStCtrl >> pE2->aColorScheme;
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
            DffRecordHeader* pHeadersFootersHd = aDocRecManager.GetRecordHeader( PPT_PST_HeadersFooters, SEEK_FROM_BEGINNING );
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
                for ( sal_uInt16 i = 0; i < pMasterPages->size(); i++ )
                {
                    if ( (*pMasterPages)[ i ]->bNotesMaster )
                        (*pMasterPages)[ i ]->pHeaderFooterEntry = new HeaderFooterEntry( aNotesMaster );
                    else
                        (*pMasterPages)[ i ]->pHeaderFooterEntry = new HeaderFooterEntry( aNormalMaster );
                }
            }
        }
    }
    if ( ( rStCtrl.GetError() != 0 ) || ( pDefaultSheet == NULL ) )
        bOk = sal_False;
    pPPTStyleSheet = pDefaultSheet;
    rStCtrl.Seek( 0 );
}

SdrPowerPointImport::~SdrPowerPointImport()
{
    for ( size_t i = 0, n = aHyperList.size(); i < n; ++i ) {
        delete aHyperList[ i ];
    }
    aHyperList.clear();
    delete pMasterPages;
    delete pSlidePages;
    delete pNotePages;
    delete[] pPersistPtr;
}

sal_Bool PPTConvertOCXControls::ReadOCXStream( SotStorageRef& rSrc,
        com::sun::star::uno::Reference<
        com::sun::star::drawing::XShape > *pShapeRef,
        sal_Bool bFloatingCtrl )
{
    bool bRes = false;
    uno::Reference< form::XFormComponent > xFComp;
    if ( mpPPTImporter && mpPPTImporter->ReadFormControl( rSrc, xFComp ) )
    {
        if ( xFComp.is() )
        {
            com::sun::star::awt::Size aSz;  // not used in import
            bRes = InsertControl( xFComp, aSz,pShapeRef,bFloatingCtrl);
        }
    }
    return bRes;
}

sal_Bool PPTConvertOCXControls::InsertControl(
        const com::sun::star::uno::Reference<
        com::sun::star::form::XFormComponent > &rFComp,
        const com::sun::star::awt::Size& rSize,
        com::sun::star::uno::Reference<
        com::sun::star::drawing::XShape > *pShape,
        sal_Bool /*bFloatingCtrl*/)
{
    sal_Bool bRetValue = sal_False;
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
                ->createInstance(String( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.drawing.ControlShape" ) ) );
            if( xCreate.is() )
            {
                xShape = ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >(xCreate, ::com::sun::star::uno::UNO_QUERY);
                if ( xShape.is() )
                {
                    xShape->setSize(rSize);
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
                        bRetValue = sal_True;
                    }
                }
            }
        }
    }
    catch( ... )
    {
        bRetValue = sal_False;
    }
    return bRetValue;
};
const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage >& PPTConvertOCXControls::GetDrawPage()
{
    if( !xDrawPage.is() && mxModel.is() )
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPages > xDrawPages;
        switch( ePageKind )
        {
            case PPT_SLIDEPAGE :
            case PPT_NOTEPAGE :
            {
                ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPagesSupplier >
                        xDrawPagesSupplier( mxModel, ::com::sun::star::uno::UNO_QUERY);
                if ( xDrawPagesSupplier.is() )
                    xDrawPages = xDrawPagesSupplier->getDrawPages();
            }
            break;

            case PPT_MASTERPAGE :
            {
                ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XMasterPagesSupplier >
                        xMasterPagesSupplier( mxModel, ::com::sun::star::uno::UNO_QUERY);
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

sal_Bool SdrPowerPointOLEDecompress( SvStream& rOutput, SvStream& rInput, sal_uInt32 nInputSize )
{
    sal_uInt32 nOldPos = rInput.Tell();
    char* pBuf = new char[ nInputSize ];
    rInput.Read( pBuf, nInputSize );
    ZCodec aZCodec( 0x8000, 0x8000 );
    aZCodec.BeginCompression();
    SvMemoryStream aSource( pBuf, nInputSize, STREAM_READ );
    aZCodec.Decompress( aSource, rOutput );
    const sal_Bool bSuccess(0L != aZCodec.EndCompression());
    delete[] pBuf;
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
    SdrObject* pRet = NULL;

    sal_uInt32 nOldPos = rStCtrl.Tell();

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
    for ( size_t i = 0; i < ((SdrPowerPointImport*)this)->aOleObjectList.size(); ++i )
    {
        pOe = ((SdrPowerPointImport*)this)->aOleObjectList[ i ];
        if ( pOe->nId != (sal_uInt32)nOLEId )
            continue;

        rStCtrl.Seek( pOe->nRecHdOfs );

        DffRecordHeader aHd;
        rStCtrl >> aHd;

        sal_uInt32 nLen = aHd.nRecLen - 4;
        if ( (sal_Int32)nLen > 0 )
        {
            sal_Bool bSuccess = sal_False;

            rStCtrl.SeekRel( 4 );

            ::utl::TempFile aTmpFile;
            aTmpFile.EnableKillingFile( sal_True );

            if ( aTmpFile.IsValid() )
            {
                SvStream* pDest = ::utl::UcbStreamHelper::CreateStream( aTmpFile.GetURL(), STREAM_TRUNC | STREAM_WRITE );
                if ( pDest )
                    bSuccess = SdrPowerPointOLEDecompress( *pDest, rStCtrl, nLen );
                delete pDest;
            }
            if ( bSuccess )
            {
                SvStream* pDest = ::utl::UcbStreamHelper::CreateStream( aTmpFile.GetURL(), STREAM_READ );
                Storage* pObjStor = pDest ? new Storage( *pDest, sal_True ) : NULL;
                if ( pObjStor )
                {
                    SotStorageRef xObjStor( new SotStorage( pObjStor ) );
                    if ( xObjStor.Is() && !xObjStor->GetError() )
                    {
                        if ( xObjStor->GetClassName() == SvGlobalName() )
                        {
                            ClsId aId( pObjStor->GetClassId() );
                            xObjStor->SetClass( SvGlobalName( aId.n1, aId.n2, aId.n3, aId.n4, aId.n5, aId.n6, aId.n7, aId.n8, aId.n9, aId.n10, aId.n11 ),
                                pObjStor->GetFormat(), pObjStor->GetUserName() );
                        }
                        SotStorageStreamRef xSrcTst = xObjStor->OpenSotStream( String( RTL_CONSTASCII_USTRINGPARAM( "\1Ole" ) ) );
                        if ( xSrcTst.Is() )
                        {
                            sal_uInt8 aTestA[ 10 ];
                            sal_Bool bGetItAsOle = ( sizeof( aTestA ) == xSrcTst->Read( aTestA, sizeof( aTestA ) ) );
                            if ( !bGetItAsOle )
                            {   // maybe there is a contentsstream in here
                                xSrcTst = xObjStor->OpenSotStream( String( RTL_CONSTASCII_USTRINGPARAM( "Contents" ) ), STREAM_READWRITE | STREAM_NOCREATE );
                                bGetItAsOle = ( xSrcTst.Is() && sizeof( aTestA ) == xSrcTst->Read( aTestA, sizeof( aTestA ) ) );
                            }
                            if ( bGetItAsOle )
                            {
                                ::rtl::OUString aNm;
                                // if ( nSvxMSDffOLEConvFlags )
                                {
                                    uno::Reference < embed::XStorage > xDestStorage( pOe->pShell->GetStorage() );
                                    uno::Reference < embed::XEmbeddedObject > xObj =
                                        CheckForConvertToSOObj( nSvxMSDffOLEConvFlags, *xObjStor, xDestStorage, rGraf, rVisArea );
                                    if( xObj.is() )
                                    {
                                        pOe->pShell->getEmbeddedObjectContainer().InsertEmbeddedObject( xObj, aNm );

                                        svt::EmbeddedObjectRef aObj( xObj, pOe->nAspect );

                                        // TODO/LATER: need MediaType for Graphic
                                        aObj.SetGraphic( rGraf, ::rtl::OUString() );
                                        pRet = new SdrOle2Obj( aObj, aNm, rBoundRect, sal_False );
                                    }
                                }
                                if ( !pRet && ( pOe->nType == PPT_PST_ExControl ) )
                                {
                                    uno::Reference< io::XInputStream > xIStrm = new utl::OSeekableInputStreamWrapper(*pDest );
                                    uno::Reference< frame::XModel > xModel( pOe->pShell ? pOe->pShell->GetModel() : NULL );
                                    PPTConvertOCXControls aPPTConvertOCXControls( this, xIStrm, xModel, eAktPageKind );
                                    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > xShape;
                                    if ( aPPTConvertOCXControls.ReadOCXStream( xObjStor, &xShape, sal_False ) )
                                        pRet = GetSdrObjectFromXShape( xShape );

                                }
                                if ( !pRet )
                                {
                                    aNm = pOe->pShell->getEmbeddedObjectContainer().CreateUniqueObjectName();

                                    // object is not an own object
                                    SotStorageRef xTarget = SotStorage::OpenOLEStorage( pOe->pShell->GetStorage(), aNm, STREAM_READWRITE );
                                    if ( xObjStor.Is() && xTarget.Is() )
                                    {
                                        xObjStor->CopyTo( xTarget );
                                        if( !xTarget->GetError() )
                                            xTarget->Commit();
                                    }
                                    xTarget.Clear();

                                    uno::Reference < embed::XEmbeddedObject > xObj =
                                        pOe->pShell->getEmbeddedObjectContainer().GetEmbeddedObject( aNm );
                                    if ( xObj.is() )
                                    {
                                        if ( pOe->nAspect != embed::Aspects::MSOLE_ICON )
                                        {
                                            //TODO/LATER: keep on hacking?!
                                            // modifiziert wollen wir nicht werden
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
                                        aObj.SetGraphic( aGraphic, ::rtl::OUString() );

                                        pRet = new SdrOle2Obj( aObj, aNm, rBoundRect, sal_False );
                                    }
                                }
                            }
                        }
                    }
                }
                delete pDest;
            }
        }
    }
    rStCtrl.Seek( nOldPos );

    return pRet;
}

SvMemoryStream* SdrPowerPointImport::ImportExOleObjStg( sal_uInt32 nPersistPtr, sal_uInt32& nOleId ) const
{
    SvMemoryStream* pRet = NULL;
    if ( nPersistPtr && ( nPersistPtr < nPersistPtrAnz ) )
    {
        sal_uInt32 nOldPos, nOfs = pPersistPtr[ nPersistPtr ];
        nOldPos = rStCtrl.Tell();
        rStCtrl.Seek( nOfs );
        DffRecordHeader aHd;
        rStCtrl >> aHd;
        if ( aHd.nRecType == DFF_PST_ExOleObjStg )
        {
            sal_uInt32 nLen = aHd.nRecLen - 4;
            if ( (sal_Int32)nLen > 0 )
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

        sal_uInt32 nOldPos = rStCtrl.Tell();
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
                        sal_uInt32 nPersistPtr, nIDoNotKnow1, nIDoNotKnow2;
                        rStCtrl >> nPersistPtr
                                >> nIDoNotKnow1
                                >> nIDoNotKnow2;

                        sal_uInt32 nOleId;
                        SvMemoryStream* pBas = ImportExOleObjStg( nPersistPtr, nOleId );
                        if ( pBas )
                        {
                            SotStorageRef xSource( new SotStorage( pBas, sal_True ) );
                            SotStorageRef xDest( new SotStorage( new SvMemoryStream(), sal_True ) );
                            if ( xSource.Is() && xDest.Is() )
                            {
                                // is this a visual basic storage ?
                                SotStorageRef xSubStorage = xSource->OpenSotStorage( String( RTL_CONSTASCII_USTRINGPARAM( "VBA" ) ),
                                    STREAM_READWRITE | STREAM_NOCREATE | STREAM_SHARE_DENYALL );
                                if( xSubStorage.Is() && ( SVSTREAM_OK == xSubStorage->GetError() ) )
                                {
                                    SotStorageRef xMacros = xDest->OpenSotStorage( String( RTL_CONSTASCII_USTRINGPARAM( "MACROS" ) ) );
                                    if ( xMacros.Is() )
                                    {
                                        SvStorageInfoList aList;
                                        xSource->FillInfoList( &aList );
                                        sal_uInt32 i;

                                        sal_Bool bCopied = sal_True;
                                        for ( i = 0; i < aList.size(); i++ )    // copy all entrys
                                        {
                                            const SvStorageInfo& rInfo = aList[ i ];
                                            if ( !xSource->CopyTo( rInfo.GetName(), xMacros, rInfo.GetName() ) )
                                                bCopied = sal_False;
                                        }
                                        if ( i && bCopied )
                                        {
                                            SvxImportMSVBasic aMSVBas( *pShell, *xDest, sal_True, sal_False );

                                            uno::Reference < embed::XStorage > xDoc( pShell->GetStorage() );
                                            if ( xDoc.is() )
                                            {
                                                SotStorageRef xVBA = SotStorage::OpenOLEStorage( xDoc, String( RTL_CONSTASCII_USTRINGPARAM( "_MS_VBA_Macros" ) ) );
                                                if ( xVBA.Is() && ( xVBA->GetError() == SVSTREAM_OK ) )
                                                {
                                                    SotStorageRef xSubVBA = xVBA->OpenSotStorage( String( RTL_CONSTASCII_USTRINGPARAM( "_MS_VBA_Overhead" ) ) );
                                                    if ( xSubVBA.Is() && ( xSubVBA->GetError() == SVSTREAM_OK ) )
                                                    {
                                                        SotStorageStreamRef xOriginal = xSubVBA->OpenSotStream( String( RTL_CONSTASCII_USTRINGPARAM( "_MS_VBA_Overhead2" ) ) );
                                                        if ( xOriginal.Is() && ( xOriginal->GetError() == SVSTREAM_OK ) )
                                                        {
                                                            if ( nPersistPtr && ( nPersistPtr < nPersistPtrAnz ) )
                                                            {
                                                                rStCtrl.Seek( pPersistPtr[ nPersistPtr ] );
                                                                rStCtrl >> *pHd;

                                                                *xOriginal  << nIDoNotKnow1
                                                                                << nIDoNotKnow2;

                                                                sal_uInt32 nToCopy, nBufSize;
                                                                nToCopy = pHd->nRecLen;
                                                                sal_uInt8* pBuf = new sal_uInt8[ 0x40000 ]; // 256KB Buffer
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
            sal_uInt16 i, nRecType(PPT_PST_ExEmbed);

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
                            sal_uInt32 nId;
                            rStCtrl.Seek( pPersistPtr[ aAt.nPersistPtr ] );
                            DffRecordHeader aHd;
                            rStCtrl >> aHd;
                            if ( aHd.nRecType == DFF_PST_ExOleObjStg )
                            {
                                rStCtrl >> nId;
                                aOleObjectList.push_back(
                                    new PPTOleEntry( aAt.nId, aHd.nFilePos, pShell, nRecType, aAt.nAspect ) );
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
    sal_Bool bRet = sal_False;
    DffRecordHeader* pEnvHd = aDocRecManager.GetRecordHeader( PPT_PST_Environment );
    if ( pEnvHd )
    {
        sal_uLong nFPosMerk = rStCtrl.Tell(); // FilePos merken fuer spaetere Restauration
        pEnvHd->SeekToContent( rStCtrl );
        DffRecordHeader aListHd;
        if ( SeekToRec( rStCtrl, PPT_PST_FontCollection, pEnvHd->GetRecEndFilePos(), &aListHd ) )
        {
            sal_uInt16 nCount2 = 0;
            VirtualDevice* pVDev = NULL;
            while ( SeekToRec( rStCtrl, PPT_PST_FontEntityAtom, aListHd.GetRecEndFilePos() ) )
            {
                bRet = sal_True;
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
                pFonts->insert( pFonts->begin() + nCount2++, pFont );
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


SdrObject* SdrPowerPointImport::ApplyTextObj( PPTTextObj* pTextObj, SdrTextObj* pSdrText, SdPage* /*pPage*/,
                                                SfxStyleSheet* pSheet, SfxStyleSheet** ppStyleSheetAry ) const
{
    SdrTextObj* pText = pSdrText;
    if ( pTextObj->Count() )
    {
        sal_uInt32 nDestinationInstance = pTextObj->GetDestinationInstance() ;
        SdrOutliner& rOutliner = pText->ImpGetDrawOutliner();
        if ( ( pText->GetObjInventor() == SdrInventor ) && ( pText->GetObjIdentifier() == OBJ_TITLETEXT ) ) // Outliner-Style fuer Titel-Textobjekt?!? (->von DL)
            rOutliner.Init( OUTLINERMODE_TITLEOBJECT );             // Outliner reset

        sal_Bool bOldUpdateMode = rOutliner.GetUpdateMode();
        rOutliner.SetUpdateMode( sal_False );
        if ( pSheet )
        {
            if ( rOutliner.GetStyleSheet( 0 ) != pSheet )
                rOutliner.SetStyleSheet( 0, pSheet );
        }
        rOutliner.SetVertical( pTextObj->GetVertical() );
        sal_Int16 nLastStartNumbering = -1;
        const PPTParagraphObj* pPreviousParagraph = NULL;
        for ( PPTParagraphObj* pPara = pTextObj->First(); pPara; pPara = pTextObj->Next() )
        {
            sal_uInt32 nTextSize = pPara->GetTextSize();
            if ( ! ( nTextSize & 0xffff0000 ) )
            {
                PPTPortionObj* pPortion;
                sal_Unicode* pParaText = new sal_Unicode[ nTextSize ];
                sal_Int32 nCurrentIndex = 0;
                for ( pPortion = pPara->First(); pPortion; pPortion = pPara->Next() )
                {
                    if ( pPortion->mpFieldItem )
                        pParaText[ nCurrentIndex++ ] = ' ';
                    else
                    {
                        sal_Int32 nCharacters = pPortion->Count();
                        const sal_Unicode* pSource = pPortion->maString.GetBuffer();
                        sal_Unicode* pDest = pParaText + nCurrentIndex;

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
                sal_uInt16  nParaIndex = (sal_uInt16)pTextObj->GetCurrentIndex();
                SfxStyleSheet* pS = ( ppStyleSheetAry ) ? ppStyleSheetAry[ pPara->pParaSet->mnDepth ] : pSheet;

                ESelection aSelection( nParaIndex, 0, nParaIndex, 0 );
                rOutliner.Insert( String(), nParaIndex, pPara->pParaSet->mnDepth );
                rOutliner.QuickInsertText( rtl::OUString(pParaText, nCurrentIndex), aSelection );
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
                    aParagraphAttribs.Put( SfxBoolItem( EE_PARA_BULLETSTATE, sal_False ) );

                if ( oStartNumbering )
                {
                    if ( *oStartNumbering != nLastStartNumbering )
                        rOutliner.SetNumberingStartValue( nParaIndex, *oStartNumbering );
                    else
                        rOutliner.SetNumberingStartValue( nParaIndex, -1 );
                    nLastStartNumbering = *oStartNumbering;
                }
                else
                {
                    nLastStartNumbering = -1;
                    rOutliner.SetNumberingStartValue( nParaIndex, nLastStartNumbering );
                }

                pPreviousParagraph = pPara;
                if ( !aSelection.nStartPos )    // in PPT empty paragraphs never gets a bullet
                {
                    aParagraphAttribs.Put( SfxBoolItem( EE_PARA_BULLETSTATE, sal_False ) );
                }
                aSelection.nStartPos = 0;
                rOutliner.QuickSetAttribs( aParagraphAttribs, aSelection );
                delete[] pParaText;
            }
        }
        OutlinerParaObject* pNewText = rOutliner.CreateParaObject();
        rOutliner.Clear();
        rOutliner.SetUpdateMode( bOldUpdateMode );
        pText->SetOutlinerParaObject( pNewText );
    }
    return pText;
}

sal_Bool SdrPowerPointImport::SeekToDocument( DffRecordHeader* pRecHd ) const
{
    sal_Bool bRet;
    sal_uLong nFPosMerk = rStCtrl.Tell(); // FilePos merken fuer ggf. spaetere Restauration
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
    sal_Bool bFound = rSourceHd.nRecType == PPT_PST_ProgTags;
    if ( !bFound )
        bFound = SeekToRec( rSt, PPT_PST_ProgTags, rSourceHd.GetRecEndFilePos(), &aProgTagsHd );
    if ( bFound )
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
                    String aPre = read_uInt16s_ToOUString(rSt, n);
                    n = (sal_uInt16)( i - 6 );
                    String aSuf = read_uInt16s_ToOUString(rSt, n);
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

sal_uInt32 SdrPowerPointImport::GetAktPageId()
{
    PptSlidePersistList* pList = GetPageList( eAktPageKind );
    if ( pList && nAktPageNum < pList->size() )
        return (*pList)[ (sal_uInt16)nAktPageNum ]->aPersistAtom.nSlideId;
    return 0;
}

sal_Bool SdrPowerPointImport::SeekToAktPage( DffRecordHeader* pRecHd ) const
{
    sal_Bool bRet = sal_False;
    PptSlidePersistList* pList = GetPageList( eAktPageKind );
    if ( pList && ( nAktPageNum < pList->size() ) )
    {
        sal_uLong nPersist = (*pList)[ (sal_uInt16)nAktPageNum ]->aPersistAtom.nPsrReference;
        if ( nPersist > 0 && nPersist < nPersistPtrAnz )
        {
            sal_uLong nFPos = 0;
            nFPos = pPersistPtr[ nPersist ];
            if ( nFPos < nStreamLen )
            {
                rStCtrl.Seek( nFPos );
                if ( pRecHd )
                    rStCtrl >> *pRecHd;
                bRet = sal_True;
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
        if ( pPageList && nMasterIndex < pPageList->size() )
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
        bool bInch = IsInch( eMap );
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

bool SdrPowerPointImport::GetColorFromPalette( sal_uInt16 nNum, Color& rColor ) const
{
    if ( nPageColorsNum != nAktPageNum || ePageColorsKind != eAktPageKind )
    {
        sal_uInt16 nSlideFlags = 0;
        PptSlidePersistList* pPageList = GetPageList( eAktPageKind );
        if ( pPageList && ( nAktPageNum < pPageList->size() ) )
        {
            PptSlidePersistEntry* pE = (*pPageList)[ nAktPageNum ];
            if ( pE )
                nSlideFlags = pE->aSlideAtom.nFlags;
            if ( ! ( nSlideFlags & 2 ) )
                ((SdrPowerPointImport*)this)->aPageColors = pE->aColorScheme;
        }
        if ( nSlideFlags & 2 )      // follow master colorscheme ?
        {
            PptSlidePersistList* pPageList2 = GetPageList( PPT_MASTERPAGE );
            if ( pPageList2 )
            {
                PptSlidePersistEntry* pMasterPersist = NULL;
                if ( eAktPageKind == PPT_MASTERPAGE )
                    pMasterPersist = (*pPageList2)[ nAktPageNum ];
                else
                {
                    if ( HasMasterPage( nAktPageNum, eAktPageKind ) )
                    {
                        sal_uInt16 nMasterNum = GetMasterPageIndex( nAktPageNum, eAktPageKind );
                        if ( nMasterNum < pPageList2->size() )
                            pMasterPersist = (*pPageList2)[ nMasterNum ];
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
                            pMasterPersist = (*pPageList2)[ nNextMaster ];
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
    return sal_True;
}

sal_Bool SdrPowerPointImport::SeekToShape( SvStream& rSt, void* pClientData, sal_uInt32 nId ) const
{
    sal_Bool bRet = SvxMSDffManager::SeekToShape( rSt, pClientData, nId );
    if ( !bRet )
    {
        ProcessData& rData = *( (ProcessData*)pClientData );
        PptSlidePersistEntry& rPersistEntry = rData.rPersistEntry;
        if ( rPersistEntry.ePageKind == PPT_SLIDEPAGE )
        {
            if ( HasMasterPage( nAktPageNum, eAktPageKind ) )
            {
                sal_uInt16 nMasterNum = GetMasterPageIndex( nAktPageNum, eAktPageKind );
                PptSlidePersistList* pPageList = GetPageList( PPT_MASTERPAGE );
                if ( pPageList && ( nMasterNum < pPageList->size() ) )
                {
                    PptSlidePersistEntry* pPersist = (*pPageList)[ nMasterNum ];    // get the masterpage's persistentry
                    if ( pPersist && pPersist->pPresentationObjects )
                    {
                        sal_uInt32 nCurrent(0L);
                        DffRecordList* pCList = maShapeRecords.pCList;              // we got a backup of the current position
                        if ( pCList )
                            nCurrent = pCList->nCurrent;
                        if ( ((SdrEscherImport*)this )->maShapeRecords.SeekToContent( rSt, DFF_msofbtClientData, SEEK_FROM_CURRENT_AND_RESTART ) )
                        {
                            sal_uInt32 nStreamPos = rSt.Tell();
                            PPTTextObj aTextObj( rSt, (SdrPowerPointImport&)*this, rPersistEntry, NULL );
                            if ( aTextObj.Count() || aTextObj.GetOEPlaceHolderAtom() )
                            {
                                sal_uInt32 nShapePos = 0;
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
                                }
                                if ( nShapePos )
                                {
                                    rSt.Seek( nShapePos );
                                    bRet = sal_True;
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

    return pRet;
}

void ImportComment10( SvxMSDffManager& rMan, SvStream& rStCtrl, SdrPage* pPage, DffRecordHeader& rComment10Hd )
{
    rtl::OUString   sAuthor;
    rtl::OUString   sText;
    rtl::OUString   sInitials;

    sal_Int32       nIndex = 0;
    util::DateTime  aDateTime;
    sal_Int32       nPosX = 0;
    sal_Int32       nPosY = 0;

    while ( ( rStCtrl.GetError() == 0 ) && ( rStCtrl.Tell() < rComment10Hd.GetRecEndFilePos() ) )
    {
        DffRecordHeader aCommentHd;
        rStCtrl >> aCommentHd;
        switch( aCommentHd.nRecType )
        {
            case PPT_PST_CString :
            {
                rtl::OUString aString = SvxMSDffManager::MSDFFReadZString( rStCtrl,
                    aCommentHd.nRecLen, sal_True );
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
                rStCtrl >> nIndex
                        >> aDateTime.Year
                        >> aDateTime.Month
                        >> aDateTime.Day    // DayOfWeek
                        >> aDateTime.Day
                        >> aDateTime.Hours
                        >> aDateTime.Minutes
                        >> aDateTime.Seconds
                        >> aDateTime.HundredthSeconds
                        >> nPosX
                        >> nPosY;

                aDateTime.HundredthSeconds /= 10;
            }
            break;
        }
        aCommentHd.SeekToEndOfRecord( rStCtrl );
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
    PptSlidePersistEntry& rSlidePersist = *(*pList)[ nAktPageNum ];
    if ( rSlidePersist.bStarDrawFiller )
        return;

    DffRecordHeader aPageHd;
    if ( SeekToAktPage( &aPageHd ) )
    {
        rSlidePersist.pHeaderFooterEntry = new HeaderFooterEntry( pMasterPersist );
        ProcessData aProcessData( rSlidePersist, (SdPage*)pRet );
        while ( ( rStCtrl.GetError() == 0 ) && ( rStCtrl.Tell() < aPageHd.GetRecEndFilePos() ) )
        {
            DffRecordHeader aHd;
            rStCtrl >> aHd;
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
                            aComment10Hd.SeekToEndOfRecord( rStCtrl );
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
                                    Rectangle aPageSize( Point(), pRet->GetSize() );
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
                                                rSlidePersist.pBObj = ImportObj( rStCtrl, (void*)&aProcessData, aPageSize, aPageSize );
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
                                            sal_uInt32 nSpFlags;
                                            rStCtrl >> nSpFlags >> nSpFlags;
                                            if ( nSpFlags & SP_FBACKGROUND )
                                            {
                                                aEscherObjListHd.SeekToBegOfRecord( rStCtrl );
                                                rSlidePersist.pBObj = ImportObj( rStCtrl, (void*)&aProcessData, aPageSize, aPageSize );
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
                                                Rectangle aEmpty;
                                                aShapeHd.SeekToBegOfRecord( rStCtrl );
                                                sal_Int32 nShapeId;
                                                aProcessData.pTableRowProperties = NULL;
                                                SdrObject* pObj = ImportObj( rStCtrl, (void*)&aProcessData, aEmpty, aEmpty, 0, &nShapeId );
                                                if ( pObj )
                                                {
                                                    if ( aProcessData.pTableRowProperties )
                                                        pObj = CreateTable( pObj, aProcessData.pTableRowProperties, aProcessData.rPersistEntry.pSolverContainer );

                                                    pRet->NbcInsertObject( pObj );

                                                    if( nShapeId )
                                                        insertShapeId( nShapeId, pObj );
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
                            if ( !aProcessData.aBackgroundColoredObjects.empty() )
                            {
                                if ( rSlidePersist.pBObj )
                                {
                                    const SfxPoolItem* pPoolItem = NULL;
                                    const SfxItemSet& rObjectItemSet = rSlidePersist.pBObj->GetMergedItemSet();

                                    //SfxItemState eState = rObjectItemSet.GetItemState( XATTR_FILLCOLOR, sal_False, &pPoolItem );
                                    if ( pPoolItem )
                                    {
                                        SfxItemSet aNewSet(*rObjectItemSet.GetPool());
                                        aNewSet.Put(*pPoolItem);
                                        aNewSet.Put(XFillStyleItem( XFILL_SOLID ));

                                        for (
                                            size_t i = 0, n = aProcessData.aBackgroundColoredObjects.size();
                                            i < n;
                                            ++i
                                        ) {
                                            aProcessData.aBackgroundColoredObjects[ i ]->SetMergedItemSet(aNewSet);
                                        }
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
            aHd.SeekToEndOfRecord( rStCtrl );
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
        PptSlidePersistEntry* pE = (*pPageList)[ nAktPageNum ];
        if ( pE )
            return &pE->aSlideAtom.aLayout;
    }
    return NULL;
}

sal_Bool SdrPowerPointImport::IsNoteOrHandout( sal_uInt16 nPageNum, PptPageKind /*ePageKind*/) const
{
    sal_Bool bNote = eAktPageKind == PPT_NOTEPAGE;
    if ( eAktPageKind == PPT_MASTERPAGE )
        bNote = ( nPageNum & 1 ) == 0;
    return bNote;
}

sal_uInt32 SdrPowerPointImport::GetMasterPageId( sal_uInt16 nPageNum, PptPageKind ePageKind ) const
{
    PptSlidePersistList* pPageList = GetPageList( ePageKind );
    if ( pPageList && nPageNum < pPageList->size() )
        return (*pPageList)[ nPageNum ]->aSlideAtom.nMasterId;
   return 0;
}

sal_uInt32 SdrPowerPointImport::GetNotesPageId( sal_uInt16 nPageNum ) const
{
    PptSlidePersistList* pPageList=GetPageList( PPT_SLIDEPAGE );
    if ( pPageList && nPageNum < pPageList->size() )
        return (*pPageList)[ nPageNum ]->aSlideAtom.nNotesId;
   return 0;
}

sal_Bool SdrPowerPointImport::HasMasterPage( sal_uInt16 nPageNum, PptPageKind ePageKind ) const
{
    if ( ePageKind == PPT_NOTEPAGE )
        return aDocAtom.nNotesMasterPersist != 0;
    if ( ePageKind == PPT_MASTERPAGE )
        return sal_False;
    return GetMasterPageId( nPageNum, ePageKind ) != 0;
}

sal_uInt16 SdrPowerPointImport::GetMasterPageIndex( sal_uInt16 nPageNum, PptPageKind ePageKind ) const
{
    sal_uInt16 nIdx = 0;
    if ( ePageKind == PPT_NOTEPAGE )
        return 2;
    sal_uInt32 nId = GetMasterPageId( nPageNum, ePageKind );
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
    sal_uLong nFPosMerk = rStCtrl.Tell(); // FilePos merken fuer spaetere Restauration
    DffRecordHeader aPageHd;
    if ( SeekToAktPage( &aPageHd ) )
    {   // und nun die Hintergrundattribute der Page suchen
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
                        rStCtrl >> (DffPropertyReader&)*this;
                        mnFix16Angle = Fix16ToAngle( GetPropertyValue( DFF_Prop_Rotation, 0 ) );
                        sal_uInt32 nColor = GetPropertyValue( DFF_Prop_fillColor, 0xffffff );
                        pSet = new SfxItemSet( pSdrModel->GetItemPool() );
                        DffObjData aObjData( aEscherObjectHd, Rectangle( 0, 0, 28000, 21000 ), 0 );
                        ApplyAttributes( rStCtrl, *pSet, aObjData );
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

        pRet->SetMergedItemSet(*pSet);

        pRet->SetMarkProtect( sal_True );
        pRet->SetMoveProtect( sal_True );
        pRet->SetResizeProtect( sal_True );
    }
    delete pSet;
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

sal_uInt32 HeaderFooterEntry::GetMaskForInstance( sal_uInt32 nInstance )
{
    sal_uInt32 nRet = 0;
    switch ( nInstance )
    {
        case 0 : nRet = 0x07ffff; break;
        case 1 : nRet = 0x100000; break;
        case 2 : nRet = 0x200000; break;
        case 3 : nRet = 0x080000; break;
    }
    return nRet;
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
                {
                    rE.pPlaceholder[ aHd.nRecInstance ] = MSDFFReadZString( rStCtrl,
                        aHd.nRecLen, sal_True );
                }
            }
            break;
        }
        aHd.SeekToEndOfRecord( rStCtrl );
    }
}

// no longer needed
sal_Unicode SdrPowerPointImport::PPTSubstitute( sal_uInt16 /*nFont*/, sal_Unicode /*nChar*/,
                                        sal_uInt32& /*nMappedFontId*/, Font& /*rFont*/, char /*nDefault*/) const
{
    return 0;
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
, mbSet( sal_False )
{}

SvStream& operator>>( SvStream& rIn, PPTExtParaLevel& rLevel )
{
    rLevel.mbSet = sal_True;
    rIn >> rLevel.mnExtParagraphMask;
    if ( rLevel.mnExtParagraphMask & 0x00800000 )
        rIn >> rLevel.mnBuBlip;
    if ( rLevel.mnExtParagraphMask & 0x02000000 )
        rIn >> rLevel.mnHasAnm;
    if ( rLevel.mnExtParagraphMask & 0x01000000 )
        rIn >> rLevel.mnAnmScheme;
    if ( rLevel.mnExtParagraphMask & 0x04000000 )
        rIn >> rLevel.mpfPP10Ext;
    rIn >> rLevel.mnExtCharacterMask;
    if ( rLevel.mnExtCharacterMask & 0x100000 )
        rIn >> rLevel.mcfPP10Ext;
    return rIn;
}

sal_Bool PPTExtParaProv::GetGraphic( sal_uInt32 nInstance, Graphic& rGraph ) const
{
    sal_Bool bRetValue = sal_False;
    PPTBuGraEntry* pPtr = NULL;
    if ( nInstance < aBuGraList.size() )
    {
        pPtr = aBuGraList[ nInstance ];
        if ( pPtr->nInstance == nInstance )
            bRetValue = sal_True;
    }
    if ( !bRetValue )
    {
        for (size_t i = 0; i < aBuGraList.size(); i++ )
        {
            pPtr = aBuGraList[ i ];
            if ( pPtr->nInstance == nInstance )
            {
                bRetValue = sal_True;
                break;
            }
        }
    }
    if ( bRetValue )
        rGraph = pPtr->aBuGra;
    return bRetValue;
}

PPTExtParaProv::PPTExtParaProv( SdrPowerPointImport& rMan, SvStream& rSt, const DffRecordHeader* pHd ) :
    bStyles         ( sal_False ),
    bGraphics       ( sal_False )
{
    sal_uInt32 nOldPos = rSt.Tell();

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
                        sal_uInt16 nType;
                        DffRecordHeader aBuGraAtomHd;
                        rSt >> aBuGraAtomHd;
                        if ( aBuGraAtomHd.nRecType == PPT_PST_ExtendedBuGraAtom )
                        {
                            rSt >> nType;
                            Graphic aGraphic;
                            if ( rMan.GetBLIPDirect( rSt, aGraphic, NULL ) )
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
                        aBuGraAtomHd.SeekToEndOfRecord( rSt );
                    }
                    if ( !aBuGraList.empty() )
                        bGraphics = sal_True;
                }
                break;

                case PPT_PST_ExtendedPresRuleContainer :
                    aExtendedPresRules.Consume( rSt, sal_False, aHd.GetRecEndFilePos() );
                break;
#ifdef DBG_UTIL
                default :
                    OSL_FAIL( "PPTExParaProv::PPTExParaProv - unknown atom reading ppt2000 num rules (SJ)" );
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
                        sal_uInt16 nDepth, i = 0;
                        rSt >> nDepth;
                        if ( i <= 5 )
                        {

                            while ( ( rSt.GetError() == 0 ) && ( rSt.Tell() < aHd.GetRecEndFilePos() ) && ( i < nDepth ) )
                            {
                                bStyles = sal_True;
                                rSt >> aExtParaSheet[ aHd.nRecInstance ].aExtParaLevel[ i++ ];
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
    for ( size_t i = 0, n = aBuGraList.size(); i < n; ++i )
        delete aBuGraList[ i ];
    aBuGraList.clear();
}

PPTNumberFormatCreator::PPTNumberFormatCreator( PPTExtParaProv* pParaProv ) :
    pExtParaProv ( pParaProv )
{
}

PPTNumberFormatCreator::~PPTNumberFormatCreator()
{
    delete pExtParaProv;
}

sal_Bool PPTNumberFormatCreator::ImplGetExtNumberFormat( SdrPowerPointImport& rManager,
    SvxNumberFormat& rNumberFormat, sal_uInt32 nLevel, sal_uInt32 nInstance, sal_uInt32 nDestinationInstance,
        boost::optional< sal_Int16 >& rStartNumbering, sal_uInt32 nFontHeight,  PPTParagraphObj* pPara )
{
    sal_Bool bHardAttribute = ( nDestinationInstance == 0xffffffff );

    sal_uInt32  nBuFlags = 0;
    sal_uInt16  nHasAnm = 0;
    sal_uInt32  nAnmScheme = 0xFFFF0003;
    sal_uInt16  nBuBlip = 0xffff;

    const PPTExtParaProv* pParaProv = pExtParaProv;
    if ( !pExtParaProv )
        pParaProv = ( pPara ) ? pPara->mrStyleSheet.pExtParaProv
                              : rManager.pPPTStyleSheet->pExtParaProv;
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
            bHardAttribute = sal_True;
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
                            nBuBlip = rLev.mnBuBlip;        // then there is no graphical Bullet possible
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
        if ( pParaProv->GetGraphic( nBuBlip, aGraphic ) )
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
        rStartNumbering = boost::optional< sal_Int16 >( nAnmScheme >> 16 );
    }
    return bHardAttribute;
}

void PPTNumberFormatCreator::GetNumberFormat( SdrPowerPointImport& rManager, SvxNumberFormat& rNumberFormat, sal_uInt32 nLevel, const PPTParaLevel& rParaLevel, const PPTCharLevel& rCharLevel, sal_uInt32 nInstance )
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

    boost::optional< sal_Int16 > oStartNumbering;
    ImplGetExtNumberFormat( rManager, rNumberFormat, nLevel, nInstance, 0xffffffff, oStartNumbering, rCharLevel.mnFontHeight, NULL );
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

sal_Bool PPTNumberFormatCreator::GetNumberFormat( SdrPowerPointImport& rManager, SvxNumberFormat& rNumberFormat, PPTParagraphObj* pParaObj,
                                                sal_uInt32 nDestinationInstance, boost::optional< sal_Int16 >& rStartNumbering )
{
    sal_uInt32 nHardCount = 0;
    nHardCount += pParaObj->GetAttrib( PPT_ParaAttr_BulletOn, nIsBullet, nDestinationInstance );
    nHardCount += pParaObj->GetAttrib( PPT_ParaAttr_BulletChar, nBulletChar, nDestinationInstance );
    nHardCount += pParaObj->GetAttrib( PPT_ParaAttr_BulletFont, nBulletFont, nDestinationInstance );
    nHardCount += pParaObj->GetAttrib( PPT_ParaAttr_BulletHeight, nBulletHeight, nDestinationInstance );
    nHardCount += pParaObj->GetAttrib( PPT_ParaAttr_BulletColor, nBulletColor, nDestinationInstance );
    nHardCount += pParaObj->GetAttrib( PPT_ParaAttr_TextOfs, nTextOfs, nDestinationInstance );
    nHardCount += pParaObj->GetAttrib( PPT_ParaAttr_BulletOfs, nBulletOfs, nDestinationInstance );

    if ( nIsBullet )
        rNumberFormat.SetNumberingType( SVX_NUM_CHAR_SPECIAL );

    sal_uInt32 nFontHeight = 24;
    PPTPortionObj* pPtr = pParaObj->First();
    if ( pPtr )
        pPtr->GetAttrib( PPT_CharAttr_FontHeight, nFontHeight, nDestinationInstance );
    if ( nIsBullet )
        nHardCount += ImplGetExtNumberFormat( rManager, rNumberFormat, pParaObj->pParaSet->mnDepth,
                                                    pParaObj->mnInstance, nDestinationInstance, rStartNumbering, nFontHeight, pParaObj );

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
    return ( nHardCount ) ? sal_True : sal_False;
}

void PPTNumberFormatCreator::ImplGetNumberFormat( SdrPowerPointImport& rManager, SvxNumberFormat& rNumberFormat, sal_uInt32 /*nLevel*/)
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

PPTCharSheet::PPTCharSheet( sal_uInt32 nInstance )
{
    sal_uInt32 nColor = PPT_COLSCHEME_TEXT_UND_ZEILEN;
    sal_uInt16 nFontHeight(0);
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
    for ( sal_uInt32 nDepth = 0; nDepth < 5; nDepth++ )
    {
        maCharLevel[ nDepth ].mnFlags = 0;
        maCharLevel[ nDepth ].mnFont = 0;
        maCharLevel[ nDepth ].mnAsianOrComplexFont = 0xffff;
        maCharLevel[ nDepth ].mnFontHeight = nFontHeight;
        maCharLevel[ nDepth ].mnFontColor = nColor;
        maCharLevel[ nDepth ].mnFontColorInStyleSheet = Color( (sal_uInt8)nColor, (sal_uInt8)( nColor >> 8 ), (sal_uInt8)( nColor >> 16 ) );
        maCharLevel[ nDepth ].mnEscapement = 0;
    }
}

PPTCharSheet::PPTCharSheet( const PPTCharSheet& rAttr )
{
    *this = rAttr;
}

void PPTCharSheet::Read( SvStream& rIn, sal_Bool /*bMasterStyle*/, sal_uInt32 nLevel, sal_Bool /*bFirst*/)
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
    if ( nCMask & ( 1 << PPT_CharAttr_ANSITypeface ) )          // 0x00400000
        rIn >> nVal16;
    if ( nCMask & ( 1 << PPT_CharAttr_Symbol ) )                // 0x00800000
        rIn >> nVal16;
    if ( nCMask & ( 1 << PPT_CharAttr_FontHeight ) )            // 0x00020000
        rIn >> maCharLevel[ nLevel ].mnFontHeight;
    if ( nCMask & ( 1 << PPT_CharAttr_FontColor ) )             // 0x00040000
    {
        rIn >> maCharLevel[ nLevel ].mnFontColor;
        if( ! (maCharLevel[ nLevel ].mnFontColor & 0xff000000 ) )
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
            OSL_FAIL( "PPTCharSheet::Read - unknown attribute, send me this document (SJ)" );
            rIn >> nVal16;
        }
        nCMask >>= 1;
    }
}

PPTParaSheet::PPTParaSheet( sal_uInt32 nInstance )
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
    }
    for ( sal_uInt32 i = 0; i < 5; i++ )
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

void PPTParaSheet::Read( SdrPowerPointImport&
#ifdef DBG_UTIL
                    rManager
#endif
                    , SvStream& rIn, sal_Bool /*bMasterStyle*/,
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
                OSL_FAIL( "PPTParaSheet::Read - unknown attribute, send me this document (SJ)" );
            }
#endif
            rIn >> nVal16;
        }
        nPMask >>= 1;
    }
}

PPTStyleSheet::PPTStyleSheet( const DffRecordHeader& rSlideHd, SvStream& rIn, SdrPowerPointImport& rManager,
                                const PPTTextCharacterStyleAtomInterpreter& /*rTxCFStyle*/, const PPTTextParagraphStyleAtomInterpreter& rTxPFStyle,
                                    const PPTTextSpecInfo& rTextSpecInfo ) :

    PPTNumberFormatCreator  ( new PPTExtParaProv( rManager, rIn, &rSlideHd ) ),
    maTxSI                  ( rTextSpecInfo )
{
    sal_uInt32 i;
    sal_uInt32 nOldFilePos = rIn.Tell();

    // default stylesheets
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

    /* SJ: try to locate the txMasterStyleAtom in the Environment

       it seems that the environment TextStyle is having a higher priority
       than the TextStyle that can be found within the master page
    */
    sal_Bool bFoundTxMasterStyleAtom04 = sal_False;
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
                bFoundTxMasterStyleAtom04 = sal_True;
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
    while ( ( aTxMasterStyleHd.nRecType == PPT_PST_TxMasterStyleAtom ) && ( rIn.Tell() < rSlideHd.GetRecEndFilePos() ) ) //TODO: aTxMasterStyleHd may be used without having been properly initialized
    {
        sal_uInt32 nInstance = aTxMasterStyleHd.nRecInstance;
        if ( ( nInstance < PPT_STYLESHEETENTRYS ) &&
            ( ( nInstance != TSS_TYPE_TEXT_IN_SHAPE ) || ( bFoundTxMasterStyleAtom04 == sal_False ) ) )
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
            sal_uInt16 nLevelAnz;
            rIn >> nLevelAnz;
            if ( nLevelAnz > 5 )
            {
                OSL_FAIL( "PPTStyleSheet::Ppt-TextStylesheet hat mehr als 5 Ebenen! (SJ)" );
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
                    rtl::OStringBuffer aMsg;
                    if ( rIn.Tell() > aTxMasterStyleHd.GetRecEndFilePos() )
                    {
                        aMsg.append(RTL_CONSTASCII_STRINGPARAM("\n  "));
                        aMsg.append(RTL_CONSTASCII_STRINGPARAM("reading too many bytes:"));
                        aMsg.append(static_cast<sal_Int32>(rIn.Tell() - aTxMasterStyleHd.GetRecEndFilePos()));
                    }
                    if ( rIn.Tell() < aTxMasterStyleHd.GetRecEndFilePos() )
                    {
                        aMsg.append(RTL_CONSTASCII_STRINGPARAM("\n  "));
                        aMsg.append(RTL_CONSTASCII_STRINGPARAM("reading too few bytes:"));
                        aMsg.append(static_cast<sal_Int32>(aTxMasterStyleHd.GetRecEndFilePos() - rIn.Tell()));
                    }
                    if (aMsg.getLength())
                    {
                        aMsg.insert(0, RTL_CONSTASCII_STRINGPARAM("]:"));
                        aMsg.insert(0, RTL_CONSTASCII_STRINGPARAM(
                            "PptStyleSheet::operator>>["));
                        OSL_FAIL(aMsg.getStr());
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
        DffRecordHeader* pEnvHeader2 = rManager.aDocRecManager.GetRecordHeader( PPT_PST_Environment );
        if ( pEnvHeader2 )
        {
            pEnvHeader2->SeekToContent( rIn );
            DffRecordHeader aTxMasterStyleHd2;
            while ( rIn.Tell() < pEnvHeader2->GetRecEndFilePos() )
            {
                rIn >> aTxMasterStyleHd2;
                if ( aTxMasterStyleHd2.nRecType == PPT_PST_TxMasterStyleAtom )
                {
                    sal_uInt16 nLevelAnz;
                    rIn >> nLevelAnz;

                    sal_uInt16 nLev = 0;
                    sal_Bool bFirst = sal_True;
                    while ( rIn.GetError() == 0 && rIn.Tell() < aTxMasterStyleHd2.GetRecEndFilePos() && nLev < nLevelAnz )
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
                    aTxMasterStyleHd2.SeekToEndOfRecord( rIn );
            }
        }
    }
    rIn.Seek( nOldFilePos );

    // will will create the default numbulletitem for each instance
    for ( i = 0; i < PPT_STYLESHEETENTRYS; i++ )
    {
        sal_uInt16          nLevels, nDepth = 0;
        SvxNumRuleType  eNumRuleType;

        switch ( i )
        {
            case TSS_TYPE_PAGETITLE :
            case TSS_TYPE_TITLE :
                nLevels = 1;
                eNumRuleType = SVX_RULETYPE_NUMBERING;
            break;
            case TSS_TYPE_SUBTITLE :
                nLevels = 10;
                eNumRuleType = SVX_RULETYPE_NUMBERING;
            break;
            case TSS_TYPE_BODY :
            case TSS_TYPE_HALFBODY :
            case TSS_TYPE_QUARTERBODY :
                nLevels = 10;
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
                        nLevels, sal_False, eNumRuleType );
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
                if ( eNumRuleType == SVX_RULETYPE_PRESENTATION_NUMBERING )
                    aRule.SetLevel( 0, aNumberFormat );
            }
        }
        mpNumBulletItem[ i ] = new SvxNumBulletItem( aRule, EE_PARA_NUMBULLET );
    }
}

PPTStyleSheet::~PPTStyleSheet()
{
    for ( sal_uInt32 i = 0; i < PPT_STYLESHEETENTRYS; i++ )
    {
        delete mpCharSheet[ i ];
        delete mpParaSheet[ i ];
        delete mpNumBulletItem[ i ];
    }
}

PPTParaPropSet::PPTParaPropSet() :
    pParaSet( new ImplPPTParaPropSet )
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

PPTParaPropSet& PPTParaPropSet::operator=( PPTParaPropSet& rParaPropSet )
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

PPTCharPropSet::PPTCharPropSet( sal_uInt32 nParagraph ) :
    mnParagraph     ( nParagraph ),
    mpFieldItem     ( NULL ),
    pCharSet        ( new ImplPPTCharPropSet )
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

void PPTCharPropSet::SetFont( sal_uInt16 nFont )
{
    sal_uInt32  nMask = 1 << PPT_CharAttr_Font;
    sal_uInt32  bDoNotMake = pCharSet->mnAttrSet & nMask;

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

PPTRuler::PPTRuler()
    : nRefCount(1)
    , nFlags(0)
    , nDefaultTab(0x240)
    , pTab(NULL)
    , nTabCount(0)
{
    memset(nTextOfs, 0, sizeof(nTextOfs));
    memset(nBulletOfs, 0, sizeof(nBulletOfs));
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

PPTTextRulerInterpreter::PPTTextRulerInterpreter( sal_uInt32 nFileOfs, SdrPowerPointImport& rMan, DffRecordHeader& rHeader, SvStream& rIn ) :
    mpImplRuler ( new PPTRuler() )
{
    if ( nFileOfs != 0xffffffff )
    {
        sal_uInt32 nOldPos = rIn.Tell();
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

            // number of indent levels, unused now
            if ( mpImplRuler->nFlags & 2 )
                rIn >> nTCount;
            if ( mpImplRuler->nFlags & 1 )
                rIn >> mpImplRuler->nDefaultTab;
            if ( mpImplRuler->nFlags & 4 )
            {
                rIn >> nTCount;
                if ( nTCount )
                {
                    mpImplRuler->nTabCount = (sal_uInt16)nTCount;
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
                if( mpImplRuler->nBulletOfs[ i ] > 0x7fff)
                {
                    // workaround
                    // when bullet offset is > 0x7fff, the paragraph should look like
                    // *    first line text
                    // second line text
                    //
                    // we add to bullet para indent 0xffff - bullet offset. it looks like
                    // best we can do for now
                    mpImplRuler->nTextOfs[ i ] += 0xffff - mpImplRuler->nBulletOfs[ i ];
                    mpImplRuler->nBulletOfs[ i ] = 0;
                }
            }
        }
        rIn.Seek( nOldPos );
    }
}

sal_Bool PPTTextRulerInterpreter::GetDefaultTab( sal_uInt32 /*nLevel*/, sal_uInt16& nValue ) const
{
    if ( ! ( mpImplRuler->nFlags & 1 ) )
        return sal_False;
    nValue = mpImplRuler->nDefaultTab;
    return sal_True;
}

sal_Bool PPTTextRulerInterpreter::GetTextOfs( sal_uInt32 nLevel, sal_uInt16& nValue ) const
{
    if ( ! ( ( nLevel < 5 ) && ( mpImplRuler->nFlags & ( 8 << nLevel ) ) ) )
        return sal_False;
    nValue = mpImplRuler->nTextOfs[ nLevel ];
    return sal_True;
}

sal_Bool PPTTextRulerInterpreter::GetBulletOfs( sal_uInt32 nLevel, sal_uInt16& nValue ) const
{
    if ( ! ( ( nLevel < 5 ) && ( mpImplRuler->nFlags & ( 256 << nLevel ) ) ) )
        return sal_False;
    nValue = mpImplRuler->nBulletOfs[ nLevel ];
    return sal_True;
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

PPTTextCharacterStyleAtomInterpreter::PPTTextCharacterStyleAtomInterpreter() :
    nFlags1 ( 0 ),
    nFlags2 ( 0 ),
    nFlags3 ( 0 )
{
}

sal_Bool PPTTextCharacterStyleAtomInterpreter::Read( SvStream& rIn, const DffRecordHeader& rRecHd )
{
    rRecHd.SeekToContent( rIn );

    rIn >> nFlags1
        >> nFlags2
        >> nFlags3
        >> n1
        >> nFontHeight
        >> nFontColor;

    return sal_True;
}

PPTTextCharacterStyleAtomInterpreter::~PPTTextCharacterStyleAtomInterpreter()
{
}

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
        aList.push_back( pEntry );
    }
    bValid = rIn.Tell() == rRecHd.GetRecEndFilePos();
    return bValid;
}

PPTTextSpecInfoAtomInterpreter::~PPTTextSpecInfoAtomInterpreter()
{
    for ( size_t i = 0, n = aList.size(); i < n; ++i ) {
        delete aList[ i ];
    }
    aList.clear();
}

void StyleTextProp9::Read( SvStream& rIn )
{
    rIn >> mnExtParagraphMask;
    if ( mnExtParagraphMask & 0x800000 )
        rIn >> mnBuBlip;
    if ( mnExtParagraphMask & 0x2000000 )
        rIn >> mnHasAnm;
    if ( mnExtParagraphMask & 0x1000000 )
        rIn >> mnAnmScheme;
    if ( mnExtParagraphMask & 0x4000000 )
        rIn >> mpfPP10Ext;
    rIn >> mnExtCharacterMask;
    if ( mnExtCharacterMask & 0x100000 )
        rIn >> mncfPP10Ext;
    rIn >> mnSpecialInfoMask;
    if ( mnSpecialInfoMask & 0x20 )
        rIn >> mnPP10Ext;
    if ( mnSpecialInfoMask & 0x40 )
        rIn >> mfBidi;
}

PPTStyleTextPropReader::PPTStyleTextPropReader( SvStream& rIn, SdrPowerPointImport& rMan, const DffRecordHeader& rTextHeader,
                                                        PPTTextRulerInterpreter& rRuler, const DffRecordHeader& rExtParaHd, sal_uInt32 nInstance )
{
    Init(rIn, rMan, rTextHeader, rRuler, rExtParaHd, nInstance);
}

void PPTStyleTextPropReader::ReadParaProps( SvStream& rIn, SdrPowerPointImport& rMan, const DffRecordHeader& rTextHeader,
                                            const String& aString, PPTTextRulerInterpreter& rRuler,
                                            sal_uInt32& nCharCount, sal_Bool& bTextPropAtom )
{
    sal_uInt32  nMask = 0; //TODO: nMask initialized here to suppress warning for now, see corresponding TODO below
    sal_uInt32  nCharAnzRead = 0;
    sal_uInt16  nDummy16;

    sal_uInt16 nStringLen = aString.Len();

    DffRecordHeader aTextHd2;
    rTextHeader.SeekToContent( rIn );
    if ( rMan.SeekToRec( rIn, PPT_PST_StyleTextPropAtom, rTextHeader.GetRecEndFilePos(), &aTextHd2 ) )
        bTextPropAtom = sal_True;
    while ( nCharAnzRead <= nStringLen )
    {
        PPTParaPropSet aParaPropSet;
        ImplPPTParaPropSet& aSet = *aParaPropSet.pParaSet;
        if ( bTextPropAtom )
        {
            rIn >> nCharCount
                >> aParaPropSet.pParaSet->mnDepth;  // Einruecktiefe

            aParaPropSet.pParaSet->mnDepth =        // taking care of about using not more than 9 outliner levels
                std::min(sal_uInt16(8),
                    aParaPropSet.pParaSet->mnDepth);

            nCharCount--;

            rIn >> nMask;
            aSet.mnAttrSet = nMask & 0x207df7;
            sal_uInt16 nBulFlg = 0;
            if ( nMask & 0xF )
                rIn >> nBulFlg; // Bullet-HardAttr-Flags
            aSet.mpArry[ PPT_ParaAttr_BulletOn    ] = ( nBulFlg & 1 ) ? 1 : 0;
            aSet.mpArry[ PPT_ParaAttr_BuHardFont  ] = ( nBulFlg & 2 ) ? 1 : 0;
            aSet.mpArry[ PPT_ParaAttr_BuHardColor ] = ( nBulFlg & 4 ) ? 1 : 0;

            if ( nMask & 0x0080 )   // buChar
                rIn >> aSet.mpArry[ PPT_ParaAttr_BulletChar ];
            if ( nMask & 0x0010 )   // buTypeface
                rIn >> aSet.mpArry[ PPT_ParaAttr_BulletFont ];
            if ( nMask & 0x0040 )   // buSize
            {
                rIn >> aSet.mpArry[ PPT_ParaAttr_BulletHeight ];
                if ( ! ( ( nMask & ( 1 << PPT_ParaAttr_BuHardHeight ) )
                         && ( nBulFlg & ( 1 << PPT_ParaAttr_BuHardHeight ) ) ) )
                    aSet.mnAttrSet ^= 0x40;
            }
            if ( nMask & 0x0020 )   // buColor
            {
                sal_uInt32 nVal32, nHiByte;
                rIn >> nVal32;
                nHiByte = nVal32 >> 24;
                if ( nHiByte <= 8 )
                    nVal32 = nHiByte | PPT_COLSCHEME;
                aSet.mnBulletColor = nVal32;
            }
            if ( nMask & 0x0800 )   // pfAlignment
            {
                rIn >> nDummy16;
                aSet.mpArry[ PPT_ParaAttr_Adjust ] = nDummy16 & 3;
            }
            if ( nMask & 0x1000 )   // pfLineSpacing
                rIn >> aSet.mpArry[ PPT_ParaAttr_LineFeed ];
            if ( nMask & 0x2000 )   // pfSpaceBefore
                rIn >> aSet.mpArry[ PPT_ParaAttr_UpperDist ];
            if ( nMask & 0x4000 )   // pfSpaceAfter
                rIn >> aSet.mpArry[ PPT_ParaAttr_LowerDist ];
            if ( nMask & 0x100 )    // pfLeftMargin
                rIn >> nDummy16;
            if ( nMask & 0x400 )    // pfIndent
                rIn >> nDummy16;
            if ( nMask & 0x8000 )   // pfDefaultTabSize
                rIn >> nDummy16;
            if ( nMask & 0x100000 ) // pfTabStops
            {
                sal_uInt16 i, nDistance, nAlignment, nNumberOfTabStops = 0;
                rIn >> nNumberOfTabStops;
                for ( i = 0; i < nNumberOfTabStops; i++ )
                {
                    rIn >> nDistance
                        >> nAlignment;
                }
            }
            if ( nMask & 0x10000 )  // pfBaseLine
                rIn >> nDummy16;
            if ( nMask & 0xe0000 )  // pfCharWrap, pfWordWrap, pfOverflow
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
            if ( nMask & 0x200000 ) // pfTextDirection
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

        if ( ( nCharCount > nStringLen ) || ( nStringLen < nCharAnzRead + nCharCount ) )
        {
            bTextPropAtom = sal_False;
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
            const sal_Unicode* pDat = aString.GetBuffer() + nCharAnzRead;
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

void PPTStyleTextPropReader::ReadCharProps( SvStream& rIn, PPTCharPropSet& aCharPropSet, const String& aString,
                                            sal_uInt32& nCharCount, sal_uInt32 nCharAnzRead,
                                            sal_Bool& bTextPropAtom, sal_uInt32 nExtParaPos,
                                            const std::vector< StyleTextProp9 >& aStyleTextProp9,
                                            sal_uInt32& nExtParaFlags, sal_uInt16& nBuBlip,
                                            sal_uInt16& nHasAnm, sal_uInt32& nAnmScheme )
{
    sal_uInt32  nMask = 0; //TODO: nMask initialized here to suppress warning for now, see corresponding TODO below
    sal_uInt16  nDummy16;
    sal_Int32   nCharsToRead;
    sal_uInt16 nStringLen = aString.Len();

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
            OSL_FAIL( "SJ:PPTStyleTextPropReader::could not get this PPT_PST_StyleTextPropAtom by reading the character attributes" );
        }
    }
    ImplPPTCharPropSet& aSet = *aCharPropSet.pCharSet;

    // character attributes
    rIn >> nMask;
    if ( (sal_uInt16)nMask )
    {
        aSet.mnAttrSet |= (sal_uInt16)nMask;
        rIn >> aSet.mnFlags;
    }
    if ( nMask & 0x10000 )  // cfTypeface
    {
        rIn >> aSet.mnFont;
        aSet.mnAttrSet |= 1 << PPT_CharAttr_Font;
    }
    if ( nMask & 0x200000 ) // cfFEOldTypeface
    {
        rIn >> aSet.mnAsianOrComplexFont;
        aSet.mnAttrSet |= 1 << PPT_CharAttr_AsianOrComplexFont;
    }
    if ( nMask & 0x400000 ) // cfANSITypeface
    {
        rIn >> aSet.mnANSITypeface;
        aSet.mnAttrSet |= 1 << PPT_CharAttr_ANSITypeface;
    }
    if ( nMask & 0x800000 ) // cfSymbolTypeface
    {
        rIn >> aSet.mnSymbolFont;
        aSet.mnAttrSet |= 1 << PPT_CharAttr_Symbol;
    }
    if ( nMask & 0x20000 )  // cfSize
    {
        rIn >> aSet.mnFontHeight;
        aSet.mnAttrSet |= 1 << PPT_CharAttr_FontHeight;
    }
    if ( nMask & 0x40000 )  // cfColor
    {
        sal_uInt32 nVal;
        rIn >> nVal;
        if ( !( nVal & 0xff000000 ) )
            nVal = PPT_COLSCHEME_HINTERGRUND;
        aSet.mnColor = nVal;
        aSet.mnAttrSet |= 1 << PPT_CharAttr_FontColor;
    }
    if ( nMask & 0x80000 )  // cfPosition
    {
        rIn >> aSet.mnEscapement;
        aSet.mnAttrSet |= 1 << PPT_CharAttr_Escapement;
    }
    if ( nExtParaPos )
    {
        sal_uInt32 nExtBuInd = nMask & 0x3c00;
        sal_uInt32  nExtParaNibble = 0;
        if ( nExtBuInd )
            nExtBuInd = ( aSet.mnFlags & 0x3c00 ) >> 10;
        if ( nExtBuInd < aStyleTextProp9.size() )
        {
            if ( nExtParaNibble && ( ( nExtBuInd + nExtParaNibble ) < aStyleTextProp9.size() ) )
                nExtBuInd += nExtParaNibble;

            nExtParaFlags = aStyleTextProp9[ nExtBuInd ].mnExtParagraphMask;
            nBuBlip = aStyleTextProp9[ nExtBuInd ].mnBuBlip;
            nHasAnm = aStyleTextProp9[ nExtBuInd ].mnHasAnm;
            nAnmScheme = aStyleTextProp9[ nExtBuInd ].mnAnmScheme;
        }
        if ( ( nExtBuInd & 0xf ) == 0xf )
            nExtParaNibble += 16;
    }
}

void PPTStyleTextPropReader::Init( SvStream& rIn, SdrPowerPointImport& rMan, const DffRecordHeader& rTextHeader,
                                   PPTTextRulerInterpreter& rRuler, const DffRecordHeader& rExtParaHd, sal_uInt32 nInstance )
{
    sal_uInt32 nMerk = rIn.Tell();
    sal_uInt32 nExtParaPos = ( rExtParaHd.nRecType == PPT_PST_ExtendedParagraphAtom ) ? rExtParaHd.nFilePos + 8 : 0;

    std::vector< StyleTextProp9 > aStyleTextProp9;
    if ( rExtParaHd.nRecType == PPT_PST_ExtendedParagraphAtom  )
    {
        rIn.Seek( rExtParaHd.nFilePos + 8 );
        while( ( rIn.GetError() == 0 ) && ( rIn.Tell() < rExtParaHd.GetRecEndFilePos() ) )
        {
            aStyleTextProp9.resize( aStyleTextProp9.size() + 1 );
            aStyleTextProp9.back().Read( rIn );
        }
        rIn.Seek( nMerk );
    }

    String aString;
    DffRecordHeader aTextHd;
    rIn >> aTextHd;
    sal_uInt32 nMaxLen = aTextHd.nRecLen;
    if ( nMaxLen >= 0xFFFF )
        nMaxLen = 0xFFFE;

    if( aTextHd.nRecType == PPT_PST_TextCharsAtom )
    {
        sal_uInt32 i;
        sal_Unicode nChar,*pBuf = new sal_Unicode[ ( nMaxLen >> 1 ) + 1 ];
        rIn.Read( pBuf, nMaxLen );
        nMaxLen >>= 1;
        pBuf[ nMaxLen ] = 0;
        sal_Unicode* pPtr = pBuf;
#ifdef OSL_BIGENDIAN
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
                aSpecMarkerList.push_back( (sal_uInt32)( i | PPT_SPEC_SYMBOL ) );
            else if ( nChar == 0xd )
            {
                if ( nInstance == TSS_TYPE_PAGETITLE )
                    *pPtr = 0xb;
                else
                    aSpecMarkerList.push_back( (sal_uInt32)( i | PPT_SPEC_NEWLINE ) );
            }
        }
        if ( i )
            aString = rtl::OUString(pBuf, i);
        delete[] pBuf;
    }
    else if( aTextHd.nRecType == PPT_PST_TextBytesAtom )
    {
        sal_Char *pBuf = new sal_Char[ nMaxLen + 1 ];
        pBuf[ nMaxLen ] = 0;
        rIn.Read( pBuf, nMaxLen );
        sal_Char* pPtr = pBuf;
        for (;;)
        {
            sal_Char cLo = *pPtr;
            if ( cLo == 0 )
                break;
            if ( cLo == 0xd )
            {
                if ( nInstance == TSS_TYPE_PAGETITLE )
                    *pPtr = 0xb;
                else
                    aSpecMarkerList.push_back( (sal_uInt32)( (pPtr - pBuf) | PPT_SPEC_NEWLINE ) );
            }
            pPtr++;
        }
        xub_StrLen nLen = sal::static_int_cast< xub_StrLen >( pPtr - pBuf );
        if ( nLen )
            aString = String( pBuf, nLen, RTL_TEXTENCODING_MS_1252 );
        delete[] pBuf;
    }
    else
    {
        // no chars, but potentially char/para props?
        sal_uInt32  nCharCount;
        sal_Bool    bTextPropAtom = sal_False;
        ReadParaProps( rIn, rMan, rTextHeader, aString, rRuler, nCharCount, bTextPropAtom );

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

    if ( aString.Len() )
    {
        sal_uInt32  nCharCount;
        sal_Bool    bTextPropAtom = sal_False;

        ReadParaProps( rIn, rMan, rTextHeader, aString, rRuler, nCharCount, bTextPropAtom );

        sal_Bool bEmptyParaPossible = sal_True;
        sal_uInt32 nCharAnzRead = 0;
        sal_uInt32 nCurrentPara = 0;
        size_t i = 1;                   // points to the next element to process
        sal_uInt32 nCurrentSpecMarker = aSpecMarkerList.empty() ? 0 : aSpecMarkerList[0];
        sal_uInt16 nStringLen = aString.Len();

        while ( nCharAnzRead < nStringLen )
        {
            sal_uInt32 nExtParaFlags = 0, nLatestParaUpdate = 0xffffffff, nAnmScheme = 0;
            sal_uInt16 nBuBlip = 0xffff, nHasAnm = 0;

            PPTCharPropSet aCharPropSet( nCurrentPara );
            if ( bTextPropAtom )
                ReadCharProps( rIn, aCharPropSet, aString, nCharCount, nCharAnzRead,
                               bTextPropAtom, nExtParaPos, aStyleTextProp9, nExtParaFlags,
                               nBuBlip, nHasAnm, nAnmScheme );
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
                            aCharPropSet.maString = String( aString, (sal_uInt16)nCharAnzRead, (sal_uInt16)nLen );
                        else if ( bEmptyParaPossible )
                            aCharPropSet.maString = String();
                        if ( nLen || bEmptyParaPossible )
                            aCharPropList.push_back( new PPTCharPropSet( aCharPropSet, nCurrentPara ) );
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
                            aCharPropList.push_back( new PPTCharPropSet( aCharPropSet, nCurrentPara ) );
                            nCharCount -= nLen;
                            nCharAnzRead += nLen;
                        }
                        PPTCharPropSet* pCPropSet = new PPTCharPropSet( aCharPropSet, nCurrentPara );
                        pCPropSet->maString = aString.GetChar( (sal_uInt16)nCharAnzRead );
                        if ( aCharPropSet.pCharSet->mnAttrSet & ( 1 << PPT_CharAttr_Symbol ) )
                            pCPropSet->SetFont( aCharPropSet.pCharSet->mnSymbolFont );
                        aCharPropList.push_back( pCPropSet );
                        nCharCount--;
                        nCharAnzRead++;
                        bEmptyParaPossible = sal_False;
                    }
                    nCurrentSpecMarker = ( i < aSpecMarkerList.size() ) ? aSpecMarkerList[ i++ ] : 0;
                }
                else
                {
                    aCharPropSet.maString = String( aString, (sal_uInt16)nCharAnzRead, (sal_uInt16)nCharCount );
                    aCharPropList.push_back( new PPTCharPropSet( aCharPropSet, nCurrentPara ) );
                    nCharAnzRead += nCharCount;
                    bEmptyParaPossible = sal_False;
                    break;
                }
            }
         }
        if ( !aCharPropList.empty() && ( aCharPropList.back()->mnParagraph != nCurrentPara ) )
        {
            PPTCharPropSet* pCharPropSet = new PPTCharPropSet( *aCharPropList.back(), nCurrentPara );
            pCharPropSet->maString = String();
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

struct FieldEntry
{
    sal_uInt32  nFieldType;
    sal_uInt32  nFieldStartPos;
    sal_uInt32  nFieldEndPos;
    String  aFieldUrl;

    FieldEntry( sal_uInt32 nType, sal_uInt32 nStart, sal_uInt32 nEnd )
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


PPTPortionObj::PPTPortionObj( const PPTStyleSheet& rStyleSheet, sal_uInt32 nInstance, sal_uInt32 nDepth ) :
    PPTCharPropSet  ( 0 ),
    mrStyleSheet    ( rStyleSheet ),
    mnInstance      ( nInstance ),
    mnDepth         ( ( nDepth > 4 ) ? 4 : nDepth )
{
}

PPTPortionObj::PPTPortionObj( PPTCharPropSet& rCharPropSet, const PPTStyleSheet& rStyleSheet, sal_uInt32 nInstance, sal_uInt32 nDepth ) :
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

sal_Bool PPTPortionObj::HasTabulator()
{
    sal_Bool bRetValue =    sal_False;
    sal_Int32           nCount;
    const sal_Unicode*  pPtr = maString.GetBuffer();
    for ( nCount = 0; nCount < maString.Len(); nCount++ )
    {
        if ( pPtr[ nCount ] == 0x9 )
        {
            bRetValue = sal_True;
            break;
        }

    }
    return bRetValue;
}

sal_Bool PPTPortionObj::GetAttrib( sal_uInt32 nAttr, sal_uInt32& nRetValue, sal_uInt32 nDestinationInstance )
{
    sal_uInt32  nMask = 1 << nAttr;
    nRetValue = 0;

    sal_uInt32 bIsHardAttribute = ( ( pCharSet->mnAttrSet & nMask ) != 0 ) ? 1 : 0;

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
                OSL_FAIL( "SJ:PPTPortionObj::GetAttrib ( hard attribute does not exist )" );
        }
    }
    else
    {
        const PPTCharLevel& rCharLevel = mrStyleSheet.mpCharSheet[ mnInstance ]->maCharLevel[ mnDepth ];
        PPTCharLevel* pCharLevel = NULL;
        if ( ( nDestinationInstance == 0xffffffff )
                || ( mnDepth && ( ( mnInstance == TSS_TYPE_SUBTITLE ) || ( mnInstance == TSS_TYPE_TEXT_IN_SHAPE ) ) ) )
            bIsHardAttribute = 1;
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
                nRetValue = ( rCharLevel.mnFlags & nMask ) ? 1 : 0;
                if ( pCharLevel )
                {
                    sal_uInt32 nTmp = ( pCharLevel->mnFlags & nMask ) ? 1 : 0;
                    if ( nRetValue != nTmp )
                        bIsHardAttribute = 1;
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
                OSL_FAIL( "SJ:PPTPortionObj::GetAttrib ( attribute does not exist )" );
        }
    }
    return (sal_Bool)bIsHardAttribute;
}

void PPTPortionObj::ApplyTo(  SfxItemSet& rSet, SdrPowerPointImport& rManager, sal_uInt32 nDestinationInstance )
{
    ApplyTo( rSet, rManager, nDestinationInstance, NULL );
}

void PPTPortionObj::ApplyTo(  SfxItemSet& rSet, SdrPowerPointImport& rManager, sal_uInt32 nDestinationInstance, const PPTTextObj* pTextObj )
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
        rSet.Put( SvxUnderlineItem( nVal != 0 ? UNDERLINE_SINGLE : UNDERLINE_NONE, EE_CHAR_UNDERLINE ) );

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
                            String(), pFontEnityAtom->ePitch, pFontEnityAtom->eCharSet, EE_CHAR_FONTINFO_CJK ) );
                rSet.Put( SvxFontItem( pFontEnityAtom->eFamily, pFontEnityAtom->aName,
                            String(), pFontEnityAtom->ePitch, pFontEnityAtom->eCharSet, EE_CHAR_FONTINFO_CTL ) );
            }
        }
    }
    if ( GetAttrib( PPT_CharAttr_Font, nVal, nDestinationInstance ) )
    {
        PptFontEntityAtom* pFontEnityAtom = rManager.GetFontEnityAtom( nVal );
        if ( pFontEnityAtom )
            rSet.Put( SvxFontItem( pFontEnityAtom->eFamily, pFontEnityAtom->aName, String(), pFontEnityAtom->ePitch, pFontEnityAtom->eCharSet, EE_CHAR_FONTINFO ) );
    }
    if ( GetAttrib( PPT_CharAttr_FontHeight, nVal, nDestinationInstance ) ) // Schriftgrad in Point
    {
        sal_uInt32 nHeight = rManager.ScalePoint( nVal );
        rSet.Put( SvxFontHeightItem( nHeight, 100, EE_CHAR_FONTHEIGHT ) );
        rSet.Put( SvxFontHeightItem( nHeight, 100, EE_CHAR_FONTHEIGHT_CJK ) );
        rSet.Put( SvxFontHeightItem( nHeight, 100, EE_CHAR_FONTHEIGHT_CTL ) );
    }

    if ( GetAttrib( PPT_CharAttr_Embossed, nVal, nDestinationInstance ) )
        rSet.Put( SvxCharReliefItem( nVal != 0 ? RELIEF_EMBOSSED : RELIEF_NONE, EE_CHAR_RELIEF ) );
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
                if ( rManager.GetBLIP( rManager.GetPropertyValue( DFF_Prop_fillBlip ), aGraf, NULL ) )
                {
                    Bitmap aBmp( aGraf.GetBitmap() );
                    Size aSize( aBmp.GetSizePixel() );
                    if ( aSize.Width() && aSize.Height() )
                    {
                        if ( aSize.Width () > 64 )
                            aSize.Width () = 64;
                        if ( aSize.Height() > 64 )
                            aSize.Height() = 64;

                        BitmapReadAccess*   pAcc = aBmp.AcquireReadAccess();
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
                                        const BitmapColor& rCol = pAcc->GetPaletteColor( (sal_uInt8) pAcc->GetPixel( nY, nX ) );
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
                        pItemSet->GetItemState( XATTR_FILLSTYLE, sal_False, &pFillStyleItem );
                        if ( pFillStyleItem )
                        {
                            XFillStyle eFillStyle = ((XFillStyleItem*)pFillStyleItem)->GetValue();
                            switch( eFillStyle )
                            {
                                case XFILL_SOLID :
                                {
                                    const SfxPoolItem* pFillColorItem = NULL;
                                    pItemSet->GetItemState( XATTR_FILLCOLOR, sal_False, &pFillColorItem );
                                    if ( pFillColorItem )
                                        aDefColor = ((XColorItem*)pFillColorItem)->GetColorValue();
                                }
                                break;
                                case XFILL_GRADIENT :
                                {
                                    const SfxPoolItem* pGradientItem = NULL;
                                    pItemSet->GetItemState( XATTR_FILLGRADIENT, sal_False, &pGradientItem );
                                    if ( pGradientItem )
                                        aDefColor = ((XFillGradientItem*)pGradientItem)->GetGradientValue().GetStartColor();
                                }
                                break;
                                case XFILL_HATCH :
                                case XFILL_BITMAP :
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
        if ( GetAttrib( PPT_CharAttr_FontColor, nVal, nDestinationInstance ) )  // Textfarbe (4Byte-Arg)
        {
            Color aCol( rManager.MSO_TEXT_CLR_ToColor( nVal ) );
            rSet.Put( SvxColorItem( aCol, EE_CHAR_COLOR ) );
            if ( nDestinationInstance == 0xffffffff )
                mrStyleSheet.mpCharSheet[ mnInstance ]->maCharLevel[ mnDepth ].mnFontColorInStyleSheet = aCol;
        }
        else if ( nVal & 0x0f000000 )   // this is not a hard attribute, but maybe the page has a different colerscheme,
        {                               // so that in this case we must use a hard color attribute
            Color   aCol( rManager.MSO_TEXT_CLR_ToColor( nVal ) );
            Color&  aColorInSheet = mrStyleSheet.mpCharSheet[ mnInstance ]->maCharLevel[ mnDepth ].mnFontColorInStyleSheet;
            if ( aColorInSheet != aCol )
                rSet.Put( SvxColorItem( aCol, EE_CHAR_COLOR ) );
        }
    }

    if ( GetAttrib( PPT_CharAttr_Escapement, nVal, nDestinationInstance ) ) // Hoch/Tiefstellung in %
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
    return NULL;
}

PPTParagraphObj::PPTParagraphObj( const PPTStyleSheet& rStyleSheet, sal_uInt32 nInstance, sal_uInt16 nDepth ) :
    PPTNumberFormatCreator  ( NULL ),
    mrStyleSheet            ( rStyleSheet ),
    mnInstance              ( nInstance ),
    mbTab                   ( sal_True ),       // style sheets always have to get the right tabulator setting
    mnPortionCount          ( 0 ),
    mpPortionList           ( NULL )
{
    if ( nDepth > 4 )
        nDepth = 4;
    pParaSet->mnDepth = nDepth;
}

PPTParagraphObj::PPTParagraphObj( PPTStyleTextPropReader& rPropReader,
        size_t const nCurParaPos, size_t& rnCurCharPos,
        const PPTStyleSheet& rStyleSheet,
                                    sal_uInt32 nInstance, PPTTextRulerInterpreter& rRuler ) :
    PPTParaPropSet          ( *rPropReader.aParaPropList[nCurParaPos] ),
    PPTNumberFormatCreator  ( NULL ),
    PPTTextRulerInterpreter ( rRuler ),
    mrStyleSheet            ( rStyleSheet ),
    mnInstance              ( nInstance ),
    mbTab                   ( sal_False ),
    mnCurrentObject         ( 0 ),
    mnPortionCount          ( 0 ),
    mpPortionList           ( NULL )
{
    if (rnCurCharPos < rPropReader.aCharPropList.size())
    {
        PPTCharPropSet* pCharPropSet = rPropReader.aCharPropList[rnCurCharPos];
        sal_uInt32 nCurrentParagraph = pCharPropSet->mnParagraph;
        for (size_t n = rnCurCharPos;
              n < rPropReader.aCharPropList.size() && rPropReader.aCharPropList[n]->mnParagraph == nCurrentParagraph; ++n )
            mnPortionCount++;   // counting number of portions that are part of this paragraph

        mpPortionList = new PPTPortionObj*[ mnPortionCount ];
        for ( sal_uInt32 i = 0; i < mnPortionCount; i++ )
        {
            pCharPropSet = rPropReader.aCharPropList[rnCurCharPos + i];
            if ( pCharPropSet )
            {
                PPTPortionObj* pPPTPortion = new PPTPortionObj( *pCharPropSet, rStyleSheet, nInstance, pParaSet->mnDepth );
                mpPortionList[ i ] = pPPTPortion;
                if ( !mbTab )
                    mbTab = mpPortionList[ i ]->HasTabulator();
            }
            else
            {
                OSL_FAIL( "SJ:PPTParagraphObj::It seems that there are missing some textportions" );
                mpPortionList[ i ] = NULL;
            }
        }
        rnCurCharPos += mnPortionCount;
    }
}

PPTParagraphObj::~PPTParagraphObj()
{
    ImplClear();
}

void PPTParagraphObj::AppendPortion( PPTPortionObj& rPPTPortion )
{
    sal_uInt32 i;
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

sal_Bool PPTParagraphObj::GetAttrib( sal_uInt32 nAttr, sal_uInt32& nRetValue, sal_uInt32 nDestinationInstance )
{
    sal_uInt32  nMask = 1 << nAttr;
    nRetValue = 0;

    if ( nAttr > 21 )
    {
        OSL_FAIL( "SJ:PPTParagraphObj::GetAttrib - attribute does not exist" );
        return sal_False;
    }

    sal_uInt32 bIsHardAttribute = ( ( pParaSet->mnAttrSet & nMask ) != 0 ) ? 1 : 0;

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
                if ( ( nDestinationInstance != 0xffffffff ) && mnPortionCount )
                {
                    PPTPortionObj* pPortion = mpPortionList[ 0 ];
                    if ( pPortion )
                    {
                        if ( pPortion->pCharSet->mnAttrSet & ( 1 << PPT_CharAttr_FontColor ) )
                            nRetValue = pPortion->pCharSet->mnColor;
                        else
                            nRetValue = mrStyleSheet.mpCharSheet[ nDestinationInstance ]->maCharLevel[ pParaSet->mnDepth ].mnFontColor;
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
                if ( ( nDestinationInstance != 0xffffffff ) && mnPortionCount )
                {
                    PPTPortionObj* pPortion = mpPortionList[ 0 ];
                    if ( pPortion )
                    {
                        if ( pPortion->pCharSet->mnAttrSet & ( 1 << PPT_CharAttr_Font ) )
                            nRetValue = pPortion->pCharSet->mnFont;
                        else
                            nRetValue = mrStyleSheet.mpCharSheet[ nDestinationInstance ]->maCharLevel[ pParaSet->mnDepth ].mnFont;
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
        if ( ( nDestinationInstance == 0xffffffff )
            || ( pParaSet->mnDepth && ( ( mnInstance == TSS_TYPE_SUBTITLE ) || ( mnInstance == TSS_TYPE_TEXT_IN_SHAPE ) ) ) )
            bIsHardAttribute = 1;
        else if ( nDestinationInstance != mnInstance )
            pParaLevel = &mrStyleSheet.mpParaSheet[ nDestinationInstance ]->maParaLevel[ pParaSet->mnDepth ];
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
                OSL_FAIL( "SJ:PPTParagraphObj::GetAttrib - this attribute does not make sense" );
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
                            bIsHardAttribute = pPortion->GetAttrib( PPT_CharAttr_Font, nRetValue, nDestinationInstance );
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
                            bIsHardAttribute = pPortion->GetAttrib( PPT_CharAttr_FontColor, nRetValue, nDestinationInstance );
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
    return (sal_Bool)bIsHardAttribute;
}

void PPTParagraphObj::ApplyTo( SfxItemSet& rSet,  boost::optional< sal_Int16 >& rStartNumbering, SdrPowerPointImport& rManager, sal_uInt32 nDestinationInstance, const PPTParagraphObj* /*pPrev*/)
{
    sal_Int16   nVal2;
    sal_uInt32  nVal, nUpperDist, nLowerDist;
    sal_uInt32  nInstance = nDestinationInstance != 0xffffffff ? nDestinationInstance : mnInstance;

    if ( ( nDestinationInstance != 0xffffffff ) || ( pParaSet->mnDepth <= 1 ) )
    {
        SvxNumBulletItem* pNumBulletItem = mrStyleSheet.mpNumBulletItem[ nInstance ];
        if ( pNumBulletItem )
        {
            SvxNumberFormat aNumberFormat( SVX_NUM_NUMBER_NONE );
            if ( GetNumberFormat( rManager, aNumberFormat, this, nDestinationInstance, rStartNumbering ) )
            {
                if ( aNumberFormat.GetNumberingType() == SVX_NUM_NUMBER_NONE )
                {
                    aNumberFormat.SetLSpace( 0 );
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

    sal_uInt32 nIsBullet2, _nTextOfs, _nBulletOfs, nHardAttribute = 0;
    GetAttrib( PPT_ParaAttr_BulletOn, nIsBullet2, nDestinationInstance );
    nHardAttribute += GetAttrib( PPT_ParaAttr_TextOfs, _nTextOfs, nDestinationInstance );
    nHardAttribute += GetAttrib( PPT_ParaAttr_BulletOfs, _nBulletOfs, nDestinationInstance );
    if ( !nIsBullet2 )
    {
        SvxLRSpaceItem aLRSpaceItem( EE_PARA_LRSPACE );
        sal_uInt16 nAbsLSpace = (sal_uInt16)( ( (sal_uInt32)_nTextOfs * 2540 ) / 576 );
        sal_uInt16 nFirstLineOffset = nAbsLSpace - (sal_uInt16)( ( (sal_uInt32)_nBulletOfs * 2540 ) / 576 );
        aLRSpaceItem.SetLeft( nAbsLSpace );
        aLRSpaceItem.SetTxtFirstLineOfstValue( -nFirstLineOffset );
        rSet.Put( aLRSpaceItem );
    }
    else
    {
        SvxLRSpaceItem aLRSpaceItem( EE_PARA_LRSPACE );
        aLRSpaceItem.SetLeft( 0 );
        aLRSpaceItem.SetTxtFirstLineOfstValue( 0 );
        rSet.Put( aLRSpaceItem );
    }
    if ( GetAttrib( PPT_ParaAttr_Adjust, nVal, nDestinationInstance ) )
    {
        if ( nVal <= 3 )
        {   // Absatzausrichtung
            static SvxAdjust const aAdj[ 4 ] = { SVX_ADJUST_LEFT, SVX_ADJUST_CENTER, SVX_ADJUST_RIGHT, SVX_ADJUST_BLOCK };
            rSet.Put( SvxAdjustItem( aAdj[ nVal ], EE_PARA_JUST ) );
        }
    }

    if ( GetAttrib( PPT_ParaAttr_AsianLB_1, nVal, nDestinationInstance ) )
        rSet.Put( SfxBoolItem( EE_PARA_FORBIDDENRULES, nVal != 0 ) );
    if ( GetAttrib( PPT_ParaAttr_AsianLB_3, nVal, nDestinationInstance ) )
        rSet.Put( SfxBoolItem( EE_PARA_HANGINGPUNCTUATION, nVal != 0 ) );

    if ( GetAttrib( PPT_ParaAttr_BiDi, nVal, nDestinationInstance ) )
        rSet.Put( SvxFrameDirectionItem( nVal == 1 ? FRMDIR_HORI_RIGHT_TOP : FRMDIR_HORI_LEFT_TOP, EE_PARA_WRITINGDIR ) );

    // LineSpacing
    PPTPortionObj* pPortion = First();
    sal_Bool bIsHardAttribute = GetAttrib( PPT_ParaAttr_LineFeed, nVal, nDestinationInstance );
    nVal2 = (sal_Int16)nVal;
    sal_uInt32 nFont = sal_uInt32();
    if ( pPortion && pPortion->GetAttrib( PPT_CharAttr_Font, nFont, nDestinationInstance ) )
        bIsHardAttribute = sal_True;

    if ( bIsHardAttribute )
    {
        if ( pPortion && ( nVal2 > 200 ) )
        {
            sal_uInt32 nFontHeight;
            pPortion->GetAttrib( PPT_CharAttr_FontHeight, nFontHeight, nDestinationInstance );
            nVal2 = -(sal_Int16)( ( nFontHeight * nVal * 8 ) / 100 );
        }
        rSet.Put( SdrTextFixedCellHeightItem( sal_True ), SDRATTR_TEXT_USEFIXEDCELLHEIGHT );
        SvxLineSpacingItem aItem( 200, EE_PARA_SBL );
        if ( nVal2 <= 0 ) {
            aItem.SetLineHeight( (sal_uInt16)( rManager.ScalePoint( -nVal2 ) / 8 ) );
            aItem.GetLineSpaceRule() = SVX_LINE_SPACE_FIX;
            aItem.GetInterLineSpaceRule() = SVX_INTER_LINE_SPACE_OFF;
        } else
        {
            sal_uInt8 nPropLineSpace = (sal_uInt8)nVal2;
            aItem.SetPropLineSpace( nPropLineSpace );
            aItem.GetLineSpaceRule() = SVX_LINE_SPACE_AUTO;
        }
        rSet.Put( aItem );
    }

    // Paragraph Spacing
    bIsHardAttribute = ( (sal_uInt32)GetAttrib( PPT_ParaAttr_UpperDist, nUpperDist, nDestinationInstance ) +
        (sal_uInt32)GetAttrib( PPT_ParaAttr_LowerDist, nLowerDist, nDestinationInstance ) ) != 0;
    if ( ( nUpperDist > 0 ) || ( nLowerDist > 0 ) )
    {
        if ( mnPortionCount )
        {
            sal_uInt32 nFontHeight = 0;
            mpPortionList[ mnPortionCount - 1 ]->GetAttrib( PPT_CharAttr_FontHeight, nFontHeight, nDestinationInstance );
            if ( ((sal_Int16)nUpperDist) > 0 )
                nUpperDist = - (sal_Int16)( ( nFontHeight * nUpperDist * 100 ) / 1000 );
            if ( ((sal_Int16)nLowerDist) > 0 )
                nLowerDist = - (sal_Int16)( ( nFontHeight * nLowerDist * 100 ) / 1000 );
        }
        bIsHardAttribute = sal_True;
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
        SvxTabStopItem aTabItem( 0, 0, SVX_TAB_ADJUST_DEFAULT, EE_PARA_TABS );
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
                if ( nTab > nTextOfs2 )
                    aTabItem.Insert( SvxTabStop( (sal_uInt16)( ( ( nTab - nTextOfs2 ) * 2540 ) / 576 ), eTabAdjust ) );
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
    for ( sal_uInt32 i = 0; i < mnPortionCount; i++ )
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
    sal_uInt32 i = mnCurrentObject + 1;
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

void PPTFieldEntry::GetDateTime( const sal_uInt32 nVal, SvxDateFormat& eDateFormat, SvxTimeFormat& eTimeFormat )
{
    eDateFormat = SVXDATEFORMAT_APPDEFAULT;
    eTimeFormat = SVXTIMEFORMAT_APPDEFAULT;
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
        SvxFieldItem* pFieldItem = new SvxFieldItem( SvxExtTimeField( Time( Time::SYSTEM ), SVXTIMETYPE_VAR, eTimeFormat ), EE_FEATURE_FIELD );
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
    mpImplTextObj->mpPlaceHolderAtom = NULL;
    mpImplTextObj->mnDestinationInstance = mpImplTextObj->mnInstance = 4;
    mpImplTextObj->mnCurrentObject = 0;
    mpImplTextObj->mnParagraphCount = 0;
    mpImplTextObj->mpParagraphList = NULL;
    mpImplTextObj->mnTextFlags = 0;
    mpImplTextObj->meShapeType = ( pObjData && pObjData->bShapeType ) ? pObjData->eShapeType : mso_sptMin;

    DffRecordHeader aExtParaHd;
    aExtParaHd.nRecType = 0;    // set empty

    sal_uInt32 bStatus = sal_True;

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
        // ClientData
        if ( rSdrPowerPointImport.maShapeRecords.SeekToContent( rIn, DFF_msofbtClientData, SEEK_FROM_CURRENT_AND_RESTART ) )
        {
            sal_uInt32 nOldPos = rIn.Tell();
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

        // ClientTextBox
        if ( rSdrPowerPointImport.maShapeRecords.SeekToContent( rIn, DFF_msofbtClientTextbox, SEEK_FROM_CURRENT_AND_RESTART ) )
        {
            DffRecordHeader aClientTextBoxHd( *rSdrPowerPointImport.maShapeRecords.Current() );
            sal_uInt32  nTextRulerAtomOfs = 0;  // case of zero -> this atom may be found in aClientDataContainerHd;
                                            // case of -1 -> ther is no atom of this kind
                                            // else -> this is the fileofs where we can get it

            // checkout if this is a referenced
            // textobj, if so the we will patch
            // the ClientTextBoxHd for a
            // equivalent one
            DffRecordHeader aTextHd;
            if ( rSdrPowerPointImport.SeekToRec( rIn, PPT_PST_OutlineTextRefAtom, aClientTextBoxHd.GetRecEndFilePos(), &aTextHd ) )
            {
                sal_uInt32 nRefNum;
                rIn >> nRefNum;

                if ( rSdrPowerPointImport.SeekToRec( rIn, PPT_PST_TextRulerAtom, aClientTextBoxHd.GetRecEndFilePos() ) )
                    nTextRulerAtomOfs = rIn.Tell();
                else
                    nTextRulerAtomOfs = 0xffffffff;

                sal_uInt32 nInstance = 0;
                switch( rSdrPowerPointImport.eAktPageKind )
                {
                    case PPT_NOTEPAGE :
                        nInstance++;
                    case PPT_MASTERPAGE :
                        nInstance++;
                    case PPT_SLIDEPAGE :
                    break;
                    default :
                        bStatus = sal_False;
                }
                if ( bStatus )
                {
                    sal_uInt32 nSlideId = rSdrPowerPointImport.GetAktPageId();
                    if ( !nSlideId )
                        bStatus = sal_False;
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
                        if ( pPageList && ( rSdrPowerPointImport.nAktPageNum < pPageList->size() ) )
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
                                bStatus = sal_False;
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
                    sal_uInt16 nInstance;
                    rIn >> nInstance;   // this number tells us the TxMasterStyleAtom Instance
                    if ( nInstance > 8 )
                        nInstance = 4;
                    aTextHd.SeekToEndOfRecord( rIn );
                    mpImplTextObj->mnInstance = nInstance;

                    sal_uInt32 nFilePos = rIn.Tell();
                    if ( rSdrPowerPointImport.SeekToRec2( PPT_PST_TextBytesAtom,
                                                          PPT_PST_TextCharsAtom,
                                                          aClientTextBoxHd.GetRecEndFilePos() )
                         || rSdrPowerPointImport.SeekToRec( rIn,
                                                            PPT_PST_StyleTextPropAtom,
                                                            aClientTextBoxHd.GetRecEndFilePos() ) )
                    {
                        PPTTextRulerInterpreter aTextRulerInterpreter( nTextRulerAtomOfs, rSdrPowerPointImport,
                                                                        aClientTextBoxHd, rIn );

                        PPTStyleTextPropReader aStyleTextPropReader( rIn, rSdrPowerPointImport, aClientTextBoxHd,
                                                                        aTextRulerInterpreter, aExtParaHd, nInstance );
                        sal_uInt32 nParagraphs = mpImplTextObj->mnParagraphCount = aStyleTextPropReader.aParaPropList.size();
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
                                    for ( size_t i = 0; i < aTextSpecInfoAtomInterpreter.aList.size(); ++i)
                                    {
                                        pSpecInfo = aTextSpecInfoAtomInterpreter.aList[ i ];
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
                            while ( rIn.Tell() < aClientTextBoxHd.GetRecEndFilePos() )
                            {
                                rIn >> aTextHd;
                                sal_uInt16 nVal = 0;
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
                                    {
                                        pEntry = new PPTFieldEntry;
                                        rIn >> pEntry->nPos;
                                        pEntry->pField1 = new SvxFieldItem( SvxFooterField(), EE_FEATURE_FIELD );
                                    }
                                    break;

                                    case PPT_PST_HeaderMCAtom :
                                    {
                                        pEntry = new PPTFieldEntry;
                                        rIn >> pEntry->nPos;
                                        pEntry->pField1 = new SvxFieldItem( SvxHeaderField(), EE_FEATURE_FIELD );
                                    }
                                    break;

                                    case PPT_PST_GenericDateMCAtom :
                                    {
                                        pEntry = new PPTFieldEntry;
                                        rIn >> pEntry->nPos;
                                        pEntry->pField1 = new SvxFieldItem( SvxDateTimeField(), EE_FEATURE_FIELD );
                                        if ( rPersistEntry.pHeaderFooterEntry ) // sj: #i34111# on master pages it is possible
                                        {                                       // that there is no HeaderFooterEntry available
                                            if ( rPersistEntry.pHeaderFooterEntry->nAtom & 0x20000 )    // auto date time
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
                                            //SvxFieldItem* pFieldItem = NULL;
                                            switch( aTextHd.nRecType )
                                            {
                                                case PPT_PST_SlideNumberMCAtom:
                                                    pEntry->pField1 = new SvxFieldItem( SvxPageField(), EE_FEATURE_FIELD );
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
                                                        sal_Unicode n;
                                                        xub_StrLen nLen;
                                                        String aStr;
                                                        bool inquote = sal_False;
                                                        for (nLen = 0, n = 0; nLen < 64; nLen++)
                                                        {
                                                            rIn >> n;

                                                            // Collect quoted characters into aStr
                                                            if ( n == '\'')
                                                                inquote = !inquote;
                                                            else if (!n)
                                                            {
                                                                // End of format string
                                                                pEntry->pString = new String( aStr );
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
                                                                aStr += n;
                                                            }
                                                        }
                                                    }
                                                    if ( pEntry->pString == NULL )
                                                    {
                                                        // Handle as previously
                                                        pEntry->pField1 = new SvxFieldItem( SvxDateField( Date( Date::SYSTEM ), SVXDATETYPE_FIX ), EE_FEATURE_FIELD );
                                                    }
                                                }
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
                                            for ( size_t i = 0; i < rSdrPowerPointImport.aHyperList.size(); ++i )
                                            {
                                                SdHyperlinkEntry* pHyperlink = rSdrPowerPointImport.aHyperList[ i ];
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
                                                            pEntry->pField1 = new SvxFieldItem( SvxURLField( aTarget, String(), SVXURLFORMAT_REPR ), EE_FEATURE_FIELD );
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
                                    // sorting fields ( hi >> lo )
                                    ::std::vector< PPTFieldEntry* >::iterator it = FieldList.begin();
                                    for( ; it != FieldList.end(); ++it ) {
                                        if ( (*it)->nPos < pEntry->nPos ) {
                                            break;
                                        }
                                    }
                                    if ( it != FieldList.end() ) {
                                        FieldList.insert( it, pEntry );
                                    } else {
                                        FieldList.push_back( pEntry );
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
                                    String aString( pSet->maString );
                                    sal_uInt32 nCount = aString.Len();
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
                                            if ( aString.GetChar( (sal_uInt16)nCount ) == 0x2a )
                                            {
                                                sal_uInt32 nBehind = aString.Len() - ( nCount + 1 );
                                                pSet->maString = String();
                                                if ( nBehind )
                                                {
                                                    PPTCharPropSet* pNewCPS = new PPTCharPropSet( *pSet );
                                                    pNewCPS->maString = String( aString, (sal_uInt16)nCount + 1, (sal_uInt16)nBehind );
                                                    aCharPropList.insert( aCharPropList.begin() + n + 1, pNewCPS );
                                                }
                                                if ( (*FE)->pField2 )
                                                {
                                                    PPTCharPropSet* pNewCPS = new PPTCharPropSet( *pSet );
                                                    pNewCPS->mpFieldItem = (*FE)->pField2, (*FE)->pField2 = NULL;
                                                    aCharPropList.insert( aCharPropList.begin() + n + 1, pNewCPS );

                                                    pNewCPS = new PPTCharPropSet( *pSet );
                                                    pNewCPS->maString = String( String( RTL_CONSTASCII_USTRINGPARAM( " " ) ) );
                                                    aCharPropList.insert( aCharPropList.begin() + n + 1, pNewCPS );
                                                }
                                                if ( nCount )
                                                {
                                                    PPTCharPropSet* pNewCPS = new PPTCharPropSet( *pSet );
                                                    pNewCPS->maString = String( aString, (sal_uInt16)0, (sal_uInt16)nCount );
                                                    aCharPropList.insert( aCharPropList.begin() + n++, pNewCPS );
                                                }
                                                if ( (*FE)->pField1 )
                                                {
                                                    pSet->mpFieldItem = (*FE)->pField1, (*FE)->pField1 = NULL;
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
                                                        PPTCharPropSet* pBefCPS = NULL;
                                                        if ( nCount )
                                                        {
                                                            pBefCPS = new PPTCharPropSet( *pSet );
                                                            pSet->maString = String( pSet->maString, (sal_uInt16)nCount, (sal_uInt16)( pSet->maString.Len() - nCount ) );
                                                        }
                                                        sal_uInt32  nIdx = n;
                                                        sal_Int32   nHyperLenLeft = nHyperLen;

                                                        while ( ( aCharPropList.size() > nIdx ) && nHyperLenLeft )
                                                        {
                                                            // the textrange hyperlink can take more than 1 paragraph
                                                            // the solution here is to clone the hyperlink...

                                                            PPTCharPropSet* pCurrent = aCharPropList[ nIdx ];
                                                            sal_Int32       nNextStringLen = pCurrent->maString.Len();

                                                            DBG_ASSERT( (*FE)->pField1, "missing field!" );
                                                            if (!(*FE)->pField1)
                                                                break;

                                                            const SvxURLField* pField = (const SvxURLField*)(*FE)->pField1->GetField();

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
                                                                    pCurrent->mpFieldItem = new SvxFieldItem( SvxURLField( pField->GetURL(), pCurrent->maString, SVXURLFORMAT_REPR ), EE_FEATURE_FIELD );
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
                                                                    pNewCPS->maString = String( pCurrent->maString, (sal_uInt16)nHyperLenLeft, (sal_uInt16)( nNextStringLen - nHyperLenLeft ) );
                                                                    aCharPropList.insert( aCharPropList.begin() + nIdx + 1, pNewCPS );
                                                                    String aRepresentation( pCurrent->maString, 0, (sal_uInt16)nHyperLenLeft );
                                                                    pCurrent->mpFieldItem = new SvxFieldItem( SvxURLField( pField->GetURL(), aRepresentation, SVXURLFORMAT_REPR ), EE_FEATURE_FIELD );
                                                                    nHyperLenLeft = 0;
                                                                }
                                                                pCurrent->maString = String();
                                                                pCurrent->SetColor( PPT_COLSCHEME_A_UND_HYPERLINK );
                                                            }
                                                            nIdx++;
                                                        }
                                                        delete (*FE)->pField1, (*FE)->pField1 = NULL;

                                                        if ( pBefCPS )
                                                        {
                                                            pBefCPS->maString = String( aString, (sal_uInt16)0, (sal_uInt16)nCount );
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
                                for( size_t j = 0, n2 = FieldList.size(); j < n2; ++j ) {
                                    delete FieldList[ j ];
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
        return NULL;
    return mpImplTextObj->mpParagraphList[ 0 ];
}

PPTParagraphObj* PPTTextObj::Next()
{
    sal_uInt32 i = mpImplTextObj->mnCurrentObject + 1;
    if ( i >= mpImplTextObj->mnParagraphCount )
        return NULL;
    mpImplTextObj->mnCurrentObject++;
    return mpImplTextObj->mpParagraphList[ i ];
}

const SfxItemSet* PPTTextObj::GetBackground() const
{
    if ( mpImplTextObj->mrPersistEntry.pBObj )
        return &mpImplTextObj->mrPersistEntry.pBObj->GetMergedItemSet();
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

sal_Bool IsLine( const SdrObject* pObj )
{
    return pObj->ISA( SdrPathObj ) && ((SdrPathObj*)pObj)->IsLine() && (((SdrPathObj*)pObj)->GetPointCount() == 2 );
}

sal_Bool GetCellPosition( const SdrObject* pObj, const std::set< sal_Int32 >& rRows, const std::set< sal_Int32 >& rColumns,
                            sal_Int32& nTableIndex, sal_Int32& nRow, sal_Int32& nRowCount, sal_Int32& nColumn, sal_Int32& nColumnCount )
{
    Rectangle aSnapRect( pObj->GetSnapRect() );
    sal_Bool bCellObject = ( aSnapRect.GetWidth() > 1 ) && ( aSnapRect.GetHeight() > 1 );
    if ( bCellObject )
    {
        std::set< sal_Int32 >::const_iterator aRowIter( rRows.find( aSnapRect.Top() ) );
        std::set< sal_Int32 >::const_iterator aColumnIter( rColumns.find( aSnapRect.Left() ) );
        if ( ( aRowIter == rRows.end() ) || ( aColumnIter == rColumns.end() ) )
            bCellObject = sal_False;
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
        Point aPt1( ((SdrPathObj*)pObj)->GetPoint( 0 ) );
        Point aPt2( ((SdrPathObj*)pObj)->GetPoint( 1 ) );
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

void CreateTableRows( Reference< XTableRows > xTableRows, const std::set< sal_Int32 >& rRows, sal_Int32 nTableBottom )
{
    if ( rRows.size() > 1 )
        xTableRows->insertByIndex( 0, rRows.size() - 1 );

    std::set< sal_Int32 >::const_iterator aIter( rRows.begin() );
    sal_Int32 nLastPosition( *aIter );
    Reference< XIndexAccess > xIndexAccess( xTableRows, UNO_QUERY_THROW );
    for ( sal_Int32 n = 0; n < xIndexAccess->getCount(); n++ )
    {
        sal_Int32 nHeight;
        if ( ++aIter != rRows.end() )
        {
            nHeight = *aIter - nLastPosition;
            nLastPosition = *aIter;
        }
        else
            nHeight = nTableBottom - nLastPosition;

        static const rtl::OUString  sWidth( RTL_CONSTASCII_USTRINGPARAM ( "Height" ) );
        Reference< XPropertySet > xPropSet( xIndexAccess->getByIndex( n ), UNO_QUERY_THROW );
        xPropSet->setPropertyValue( sWidth, Any( nHeight ) );
    }
}

void CreateTableColumns( Reference< XTableColumns > xTableColumns, const std::set< sal_Int32 >& rColumns, sal_Int32 nTableRight )
{
    if ( rColumns.size() > 1 )
        xTableColumns->insertByIndex( 0, rColumns.size() - 1 );

    std::set< sal_Int32 >::const_iterator aIter( rColumns.begin() );
    sal_Int32 nLastPosition( *aIter );
    Reference< XIndexAccess > xIndexAccess( xTableColumns, UNO_QUERY_THROW );
    for ( sal_Int32 n = 0; n < xIndexAccess->getCount(); n++ )
    {
        sal_Int32 nWidth;
        if ( ++aIter != rColumns.end() )
        {
            nWidth = *aIter - nLastPosition;
            nLastPosition = *aIter;
        }
        else
            nWidth = nTableRight - nLastPosition;

        static const rtl::OUString  sWidth( RTL_CONSTASCII_USTRINGPARAM ( "Width" ) );
        Reference< XPropertySet > xPropSet( xIndexAccess->getByIndex( n ), UNO_QUERY_THROW );
        xPropSet->setPropertyValue( sWidth, Any( nWidth ) );
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
       DBG_ASSERT( false, "exception caught!" );
   }
}

void ApplyCellAttributes( const SdrObject* pObj, Reference< XCell >& xCell )
{
    try
    {
        Reference< XPropertySet > xPropSet( xCell, UNO_QUERY_THROW );

        const sal_Int32 nLeftDist(((const SdrTextLeftDistItem&)pObj->GetMergedItem(SDRATTR_TEXT_LEFTDIST)).GetValue());
        const sal_Int32 nRightDist(((const SdrTextRightDistItem&)pObj->GetMergedItem(SDRATTR_TEXT_RIGHTDIST)).GetValue());
        const sal_Int32 nUpperDist(((const SdrTextUpperDistItem&)pObj->GetMergedItem(SDRATTR_TEXT_UPPERDIST)).GetValue());
        const sal_Int32 nLowerDist(((const SdrTextLowerDistItem&)pObj->GetMergedItem(SDRATTR_TEXT_LOWERDIST)).GetValue());
        static const rtl::OUString  sTopBorder( RTL_CONSTASCII_USTRINGPARAM( "TextUpperDistance" ) );
        static const rtl::OUString  sBottomBorder( RTL_CONSTASCII_USTRINGPARAM( "TextLowerDistance" ) );
        static const rtl::OUString  sLeftBorder( RTL_CONSTASCII_USTRINGPARAM( "TextLeftDistance" ) );
        static const rtl::OUString  sRightBorder( RTL_CONSTASCII_USTRINGPARAM( "TextRightDistance" ) );
        xPropSet->setPropertyValue( sTopBorder, Any( nUpperDist ) );
        xPropSet->setPropertyValue( sRightBorder, Any( nRightDist ) );
        xPropSet->setPropertyValue( sLeftBorder, Any( nLeftDist ) );
        xPropSet->setPropertyValue( sBottomBorder, Any( nLowerDist ) );

        static const rtl::OUString  sTextVerticalAdjust( RTL_CONSTASCII_USTRINGPARAM( "TextVerticalAdjust" ) );
        const SdrTextVertAdjust eTextVertAdjust(((const SdrTextVertAdjustItem&)pObj->GetMergedItem(SDRATTR_TEXT_VERTADJUST)).GetValue());
        drawing::TextVerticalAdjust eVA( drawing::TextVerticalAdjust_TOP );
        if ( eTextVertAdjust == SDRTEXTVERTADJUST_CENTER )
            eVA = drawing::TextVerticalAdjust_CENTER;
        else if ( eTextVertAdjust == SDRTEXTVERTADJUST_BOTTOM )
            eVA = drawing::TextVerticalAdjust_BOTTOM;
        xPropSet->setPropertyValue( sTextVerticalAdjust, Any( eVA ) );

        SfxItemSet aSet( pObj->GetMergedItemSet() );
        XFillStyle eFillStyle(((XFillStyleItem&)pObj->GetMergedItem( XATTR_FILLSTYLE )).GetValue());
        ::com::sun::star::drawing::FillStyle eFS( com::sun::star::drawing::FillStyle_NONE );
        switch( eFillStyle )
        {
            case XFILL_SOLID :
                {
                    static const rtl::OUString sFillColor( String( RTL_CONSTASCII_USTRINGPARAM( "FillColor" ) ) );
                    eFS = com::sun::star::drawing::FillStyle_SOLID;
                    Color aFillColor( ((XFillColorItem&)pObj->GetMergedItem( XATTR_FILLCOLOR )).GetColorValue() );
                    sal_Int32 nFillColor( aFillColor.GetColor() );
                    xPropSet->setPropertyValue( sFillColor, Any( nFillColor ) );
                }
                break;
            case XFILL_GRADIENT :
                {
                    eFS = com::sun::star::drawing::FillStyle_GRADIENT;
                    XGradient aXGradient(((const XFillGradientItem&)pObj->GetMergedItem(XATTR_FILLGRADIENT)).GetGradientValue());

                    com::sun::star::awt::Gradient aGradient;
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

                    static const rtl::OUString sFillGradient( String( RTL_CONSTASCII_USTRINGPARAM( "FillGradient" ) ) );
                    xPropSet->setPropertyValue( sFillGradient, Any( aGradient ) );
                }
                break;
            case XFILL_HATCH :
                eFS = com::sun::star::drawing::FillStyle_HATCH;
            break;
            case XFILL_BITMAP :
                {
                    eFS = com::sun::star::drawing::FillStyle_BITMAP;

                    XFillBitmapItem aXFillBitmapItem((const XFillBitmapItem&)pObj->GetMergedItem( XATTR_FILLBITMAP ));
                    XOBitmap aLocalXOBitmap( aXFillBitmapItem.GetBitmapValue() );
                    rtl::OUString aURL( RTL_CONSTASCII_USTRINGPARAM(UNO_NAME_GRAPHOBJ_URLPREFIX));
                    aURL += rtl::OStringToOUString(
                        aLocalXOBitmap.GetGraphicObject().GetUniqueID(),
                        RTL_TEXTENCODING_ASCII_US);

                    static const rtl::OUString sFillBitmapURL( String( RTL_CONSTASCII_USTRINGPARAM( "FillBitmapURL" ) ) );
                    xPropSet->setPropertyValue( sFillBitmapURL, Any( aURL ) );
                }
            break;
            case XFILL_NONE :
                eFS = com::sun::star::drawing::FillStyle_NONE;
            break;

        }
        static const rtl::OUString sFillStyle( String( RTL_CONSTASCII_USTRINGPARAM( "FillStyle" ) ) );
        xPropSet->setPropertyValue( sFillStyle, Any( eFS ) );
        if ( eFillStyle != XFILL_NONE )
        {
            sal_Int16 nFillTransparence( ( (const XFillTransparenceItem&)pObj->GetMergedItem( XATTR_FILLTRANSPARENCE ) ).GetValue() );
            if ( nFillTransparence != 100 )
            {
                nFillTransparence *= 100;
                static const rtl::OUString sFillTransparence( String( RTL_CONSTASCII_USTRINGPARAM( "FillTransparence" ) ) );
                xPropSet->setPropertyValue( sFillTransparence, Any( nFillTransparence ) );
            }
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
        SfxItemSet aSet( pLine->GetMergedItemSet() );
        XLineStyle eLineStyle(((XLineStyleItem&)pLine->GetMergedItem( XATTR_LINESTYLE )).GetValue());
        com::sun::star::table::BorderLine2 aBorderLine;
        switch( eLineStyle )
        {
            case XLINE_DASH :
            case XLINE_SOLID :
                {
                    Color aLineColor( ((XLineColorItem&)pLine->GetMergedItem( XATTR_LINECOLOR )).GetColorValue() );
                    aBorderLine.Color = aLineColor.GetColor();
                    aBorderLine.LineWidth = static_cast< sal_Int16 >( ((const XLineWidthItem&)(pLine->GetMergedItem(XATTR_LINEWIDTH))).GetValue() / 4 );
                    aBorderLine.LineStyle = eLineStyle == XLINE_SOLID ? table::BorderLineStyle::SOLID : table::BorderLineStyle::DASHED;
                }
                break;
            case XLINE_NONE :
                {
                    aBorderLine.LineWidth = 0;
                    aBorderLine.LineStyle = table::BorderLineStyle::NONE;
                }
            break;
        }
        Reference< XCellRange > xCellRange( xTable, UNO_QUERY_THROW );
        std::vector< sal_Int32 >::const_iterator aIter( vPositions.begin() );
        while( aIter != vPositions.end() )
        {
            static const rtl::OUString sTopBorder( String( RTL_CONSTASCII_USTRINGPARAM( "TopBorder" ) ) );
            static const rtl::OUString sBottomBorder( String( RTL_CONSTASCII_USTRINGPARAM( "BottomBorder" ) ) );
            static const rtl::OUString sLeftBorder( String( RTL_CONSTASCII_USTRINGPARAM( "LeftBorder" ) ) );
            static const rtl::OUString sRightBorder( String( RTL_CONSTASCII_USTRINGPARAM( "RightBorder" ) ) );
            static const rtl::OUString  sDiagonalTLBR( RTL_CONSTASCII_USTRINGPARAM ( "DiagonalTLBR" ) );
            static const rtl::OUString  sDiagonalBLTR( RTL_CONSTASCII_USTRINGPARAM ( "DiagonalBLTR" ) );

            sal_Int32 nPosition = *aIter & 0xffffff;
            sal_Int32 nFlags = *aIter &~0xffffff;
            sal_Int32 nRow = nPosition / nColumns;
            sal_Int32 nColumn = nPosition - ( nRow * nColumns );
            Reference< XCell > xCell( xCellRange->getCellByPosition( nColumn, nRow ) );
            Reference< XPropertySet > xPropSet( xCell, UNO_QUERY_THROW );

            if ( nFlags & LinePositionLeft )
                xPropSet->setPropertyValue( sLeftBorder, Any( aBorderLine ) );
            if ( nFlags & LinePositionTop )
                xPropSet->setPropertyValue( sTopBorder, Any( aBorderLine ) );
            if ( nFlags & LinePositionRight )
                xPropSet->setPropertyValue( sRightBorder, Any( aBorderLine ) );
            if ( nFlags & LinePositionBottom )
                xPropSet->setPropertyValue( sBottomBorder, Any( aBorderLine ) );
            if ( nFlags & LinePositionTLBR )
                xPropSet->setPropertyValue( sDiagonalTLBR, Any( sal_True ) );
            if ( nFlags & LinePositionBLTR )
                xPropSet->setPropertyValue( sDiagonalBLTR, Any( sal_True ) );
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
    if ( nRows && pGroup->ISA( SdrObjGroup ) )
    {
        SdrObjList* pSubList(((SdrObjGroup*)pGroup)->GetSubList());
        if ( pSubList )
        {
            std::set< sal_Int32 > aRows;
            std::set< sal_Int32 > aColumns;

            SdrObjListIter aGroupIter( *pSubList, IM_DEEPNOGROUPS, sal_False );
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
            ::sdr::table::SdrTableObj* pTable = new ::sdr::table::SdrTableObj( pSdrModel );
            pTable->uno_lock();
            Reference< XTable > xTable( pTable->getTable() );
            try
            {
                Reference< XColumnRowRange > xColumnRowRange( xTable, UNO_QUERY_THROW );

                CreateTableRows( xColumnRowRange->getRows(), aRows, pGroup->GetSnapRect().Bottom() );
                CreateTableColumns( xColumnRowRange->getColumns(), aColumns, pGroup->GetSnapRect().Right() );

                sal_Int32 nCellCount = aRows.size() * aColumns.size();
                sal_Int32 *pMergedCellIndexTable = new sal_Int32[ nCellCount ];
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
                            Reference< XCellRange > xCellRange( xTable, UNO_QUERY_THROW );
                            Reference< XCell > xCell( xCellRange->getCellByPosition( nColumn, nRow ) );

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
                delete[] pMergedCellIndexTable;

                // we are replacing the whole group object by a single table object, so
                // possibly connections to the group object have to be removed.
                if ( pSolverContainer )
                {
                    for ( size_t i = 0; i < pSolverContainer->aCList.size(); ++i )
                    {
                        SvxMSDffConnectorRule* pPtr = pSolverContainer->aCList[ i ];

                        // check connections to the group object
                        if ( pPtr->pAObj == pGroup )
                            pPtr->pAObj = NULL;
                        if ( pPtr->pBObj == pGroup )
                            pPtr->pBObj = NULL;

                        // check connections to all its subobjects
                        SdrObjListIter aIter( *pGroup, IM_DEEPWITHGROUPS );
                        while( aIter.IsMore() )
                        {
                            SdrObject* pPartObj = aIter.Next();
                            if ( pPtr->pAObj == pPartObj )
                                pPtr->pAObj = NULL;
                            if ( pPtr->pBObj == pPartObj )
                                pPtr->pBObj = NULL;
                        }
                    }
                }
                pTable->uno_unlock();
                pTable->SetSnapRect( pGroup->GetSnapRect() );
                pRet = pTable;

                //Remove Objects from shape map
                SdrObjListIter aIter( *pGroup, IM_DEEPWITHGROUPS );
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
