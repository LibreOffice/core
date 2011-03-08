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

#ifndef _SV_GDIMTF_HXX
#define _SV_GDIMTF_HXX

#include <vcl/dllapi.h>
#include <tools/gen.hxx>
#include <tools/list.hxx>
#include <tools/link.hxx>
#include <tools/string.hxx>
#include <vcl/mapmod.hxx>

class OutputDevice;
class ImpLabelList;
class MetaAction;
class MetaCommentAction;
class SvStream;
class Color;
class BitmapEx;
class Polygon;
class PolyPolygon;
class Gradient;

// ---------------------
// - GDIMetaFile-Types -
// ---------------------

#define GDI_METAFILE_END                ((ULONG)0xFFFFFFFF)
#define GDI_METAFILE_LABEL_NOTFOUND     ((ULONG)0xFFFFFFFF)

#ifndef METAFILE_END
#define METAFILE_END                    GDI_METAFILE_END
#endif

#ifndef METAFILE_LABEL_NOTFOUND
#define METAFILE_LABEL_NOTFOUND         GDI_METAFILE_LABEL_NOTFOUND
#endif

// -----------
// - Defines -
// -----------

#define MTF_MIRROR_NONE             0x00000000UL
#define MTF_MIRROR_HORZ             0x00000001UL
#define MTF_MIRROR_VERT             0x00000002UL

// ---------
// - Enums -
// ---------

enum MtfConversion
{
    MTF_CONVERSION_NONE = 0,
    MTF_CONVERSION_1BIT_THRESHOLD = 1,
    MTF_CONVERSION_8BIT_GREYS = 2
};

// -----------------------------
// - Color conversion routines -
// -----------------------------

typedef Color (*ColorExchangeFnc)( const Color& rColor, const void* pColParam );
typedef BitmapEx (*BmpExchangeFnc)( const BitmapEx& rBmpEx, const void* pBmpParam );


// ---------------
// - GDIMetaFile -
// ---------------

class VCL_DLLPUBLIC GDIMetaFile : protected List
{
private:

    MapMode         aPrefMapMode;
    Size            aPrefSize;
    Link            aHookHdlLink;
    GDIMetaFile*    pPrev;
    GDIMetaFile*    pNext;
    OutputDevice*   pOutDev;
    ImpLabelList*   pLabelList;
    BOOL            bPause;
    BOOL            bRecord;
    BOOL            bUseCanvas;


    SAL_DLLPRIVATE static Color    ImplColAdjustFnc( const Color& rColor, const void* pColParam );
    SAL_DLLPRIVATE static BitmapEx ImplBmpAdjustFnc( const BitmapEx& rBmpEx, const void* pBmpParam );

    SAL_DLLPRIVATE static Color    ImplColConvertFnc( const Color& rColor, const void* pColParam );
    SAL_DLLPRIVATE static BitmapEx ImplBmpConvertFnc( const BitmapEx& rBmpEx, const void* pBmpParam );

    SAL_DLLPRIVATE static Color    ImplColMonoFnc( const Color& rColor, const void* pColParam );
    SAL_DLLPRIVATE static BitmapEx ImplBmpMonoFnc( const BitmapEx& rBmpEx, const void* pBmpParam );

    SAL_DLLPRIVATE static Color    ImplColReplaceFnc( const Color& rColor, const void* pColParam );
    SAL_DLLPRIVATE static BitmapEx ImplBmpReplaceFnc( const BitmapEx& rBmpEx, const void* pBmpParam );

    SAL_DLLPRIVATE void            ImplExchangeColors( ColorExchangeFnc pFncCol, const void* pColParam,
                                                       BmpExchangeFnc pFncBmp, const void* pBmpParam );

    SAL_DLLPRIVATE Point           ImplGetRotatedPoint( const Point& rPt, const Point& rRotatePt,
                                                        const Size& rOffset, double fSin, double fCos );
    SAL_DLLPRIVATE Polygon         ImplGetRotatedPolygon( const Polygon& rPoly, const Point& rRotatePt,
                                                          const Size& rOffset, double fSin, double fCos );
    SAL_DLLPRIVATE PolyPolygon     ImplGetRotatedPolyPolygon( const PolyPolygon& rPoly, const Point& rRotatePt,
                                                              const Size& rOffset, double fSin, double fCos );
    SAL_DLLPRIVATE void            ImplAddGradientEx( GDIMetaFile&          rMtf,
                                                      const OutputDevice&   rMapDev,
                                                      const PolyPolygon&    rPolyPoly,
                                                      const Gradient&       rGrad       );
    SAL_DLLPRIVATE bool            ImplPlayWithRenderer( OutputDevice* pOut, const Point& rPos, Size rDestSize );
    SAL_DLLPRIVATE void          ImplDelegate2PluggableRenderer( const MetaCommentAction* pAct, OutputDevice* pOut );


protected:

    virtual void    Linker( OutputDevice* pOut, BOOL bLink );
    virtual long    Hook();

public:
                    GDIMetaFile();
                    GDIMetaFile( const GDIMetaFile& rMtf );
    virtual         ~GDIMetaFile();

    using List::operator==;
    using List::operator!=;
    GDIMetaFile&    operator=( const GDIMetaFile& rMtf );
    BOOL            operator==( const GDIMetaFile& rMtf ) const;
    BOOL            operator!=( const GDIMetaFile& rMtf ) const { return !( *this == rMtf ); }

    void            Clear();
    sal_Bool        IsEqual( const GDIMetaFile& rMtf ) const;
    BOOL            Mirror( ULONG nMirrorFlags );
    void            Move( long nX, long nY );
    // additional Move method getting specifics how to handle MapMode( MAP_PIXEL )
    void            Move( long nX, long nY, long nDPIX, long nDPIY );
    void            Scale( double fScaleX, double fScaleY );
    void            Scale( const Fraction& rScaleX, const Fraction& rScaleY );
    void            Rotate( long nAngle10 );
    void            Clip( const Rectangle& );
    /* get the bound rect of the contained actions
     * caveats:
     * - clip actions will limit the contained actions,
     *   but the current clipregion of the passed OutputDevice will not
     * - coordinates of actions will be transformed to preferred mapmode
     * - the returned rectangle is relative to the preferred mapmode of the metafile
    */
    Rectangle       GetBoundRect( OutputDevice& i_rReference );

    void            Adjust( short nLuminancePercent = 0, short nContrastPercent = 0,
                            short nChannelRPercent = 0, short nChannelGPercent = 0,
                            short nChannelBPercent = 0, double fGamma = 1.0, BOOL bInvert = FALSE );
    void            Convert( MtfConversion eConversion );
    void            ReplaceColors( const Color& rSearchColor, const Color& rReplaceColor, ULONG nTol = 0 );
    void            ReplaceColors( const Color* pSearchColors, const Color* rReplaceColors,
                                   ULONG nColorCount, ULONG* pTols = NULL );

    GDIMetaFile     GetMonochromeMtf( const Color& rCol ) const;

    void            Record( OutputDevice* pOutDev );
    BOOL            IsRecord() const { return bRecord; }

    void            Play( GDIMetaFile& rMtf, ULONG nPos = GDI_METAFILE_END );
    void            Play( OutputDevice* pOutDev, ULONG nPos = GDI_METAFILE_END );
    void            Play( OutputDevice* pOutDev, const Point& rPos,
                          const Size& rSize, ULONG nPos = GDI_METAFILE_END );

    void            Pause( BOOL bPause );
    BOOL            IsPause() const { return bPause; }

    void            Stop();

    void            WindStart();
    void            WindEnd();
    void            Wind( ULONG nAction );
    void            WindPrev();
    void            WindNext();

    ULONG           GetActionCount() const { return Count(); }
    void            AddAction( MetaAction* pAction );
    void            AddAction( MetaAction* pAction, ULONG nPos );
    void            RemoveAction( ULONG nPos );
    MetaAction*     CopyAction( ULONG nPos ) const;
    MetaAction*     GetCurAction() const { return (MetaAction*) GetCurObject(); }
    MetaAction*     GetAction( ULONG nAction ) const { return (MetaAction*) GetObject( nAction ); }
    MetaAction*     FirstAction() { return (MetaAction*) First(); }
    MetaAction*     NextAction() {  return (MetaAction*) Next(); }
    MetaAction*     ReplaceAction( MetaAction* pAction, ULONG nAction ) { return (MetaAction*) Replace( pAction, nAction ); }

    ULONG           GetActionPos( const String& rLabel );
    BOOL            InsertLabel( const String& rLabel, ULONG nActionPos );
    void            RemoveLabel( const String& rLabel );
    void            RenameLabel( const String& rLabel, const String& rNewLabel );
    ULONG           GetLabelCount() const;
    String          GetLabel( ULONG nLabel );

    BOOL            SaveStatus();

    const Size&     GetPrefSize() const { return aPrefSize; }
    void            SetPrefSize( const Size& rSize ) { aPrefSize = rSize; }

    const MapMode&  GetPrefMapMode() const { return aPrefMapMode; }
    void            SetPrefMapMode( const MapMode& rMapMode ) { aPrefMapMode = rMapMode; }

    void            SetHookHdl( const Link& rLink ) { aHookHdlLink = rLink; }
    const Link&     GetHookHdl() const { return aHookHdlLink; }

    ULONG           GetChecksum() const;
    ULONG           GetSizeBytes() const;

    // Methoden zum Lesen und Schreiben des neuen Formats;
    // die Read-Methode kann auch das alte Format lesen
    SvStream&       Read( SvStream& rIStm );
    SvStream&       Write( SvStream& rOStm );

    // Stream-Operatoren schreiben das alte Format (noch)
    // und lesen sowohl das alte wie auch das neue Format
    friend VCL_DLLPUBLIC SvStream& operator>>( SvStream& rIStm, GDIMetaFile& rGDIMetaFile );
    friend VCL_DLLPUBLIC SvStream& operator<<( SvStream& rOStm, const GDIMetaFile& rGDIMetaFile );

    BOOL           CreateThumbnail( sal_uInt32 nMaximumExtent, BitmapEx& rBmpEx, const BitmapEx* pOverlay = NULL, const Rectangle* pOverlayRect = NULL ) const;

    void           UseCanvas( BOOL _bUseCanvas );
    BOOL           GetUseCanvas() const { return bUseCanvas; }
};

/** Create a special metaaction that delegates rendering to specified
    service.

    This factory function creates a MetaCommentAction that delegates
    rendering to the specified services, once played back in the
    metafile.

    @param rRendererServiceName
    Renderer service. Gets an awt::XGraphic on instantiation

    @param rGraphicServiceName
    Graphic service. Gets the raw data on instantiation

    @param pData
    Raw data. Gets copied

    @param nDataSize
    Length, in byte, of raw data
 */
MetaCommentAction* makePluggableRendererAction( const rtl::OUString& rRendererServiceName,
                                                const rtl::OUString& rGraphicServiceName,
                                                const void* pData,
                                                sal_uInt32 nDataSize );

#endif // _SV_GDIMTF_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
