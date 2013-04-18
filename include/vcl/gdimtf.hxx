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

#ifndef _SV_GDIMTF_HXX
#define _SV_GDIMTF_HXX

#include <vcl/dllapi.h>
#include <tools/gen.hxx>
#include <tools/link.hxx>
#include <vcl/mapmod.hxx>
#include <vector>

class OutputDevice;
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

#define GDI_METAFILE_END                ((size_t)0xFFFFFFFF)
#define GDI_METAFILE_LABEL_NOTFOUND     ((size_t)0xFFFFFFFF)

#ifndef METAFILE_END
#define METAFILE_END                    GDI_METAFILE_END
#endif

#ifndef METAFILE_LABEL_NOTFOUND
#define METAFILE_LABEL_NOTFOUND         GDI_METAFILE_LABEL_NOTFOUND
#endif

#define MTF_MIRROR_NONE             0x00000000UL
#define MTF_MIRROR_HORZ             0x00000001UL
#define MTF_MIRROR_VERT             0x00000002UL

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

class VCL_DLLPUBLIC GDIMetaFile
{
private:
    ::std::vector< MetaAction* > aList;
    size_t          nCurrentActionElement;

    MapMode         aPrefMapMode;
    Size            aPrefSize;
    Link            aHookHdlLink;
    GDIMetaFile*    pPrev;
    GDIMetaFile*    pNext;
    OutputDevice*   pOutDev;
    sal_Bool            bPause;
    sal_Bool            bRecord;
    sal_Bool            bUseCanvas;


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
    SAL_DLLPRIVATE bool          ImplPlayWithRenderer( OutputDevice* pOut, const Point& rPos, Size rDestSize );
    SAL_DLLPRIVATE void          ImplDelegate2PluggableRenderer( const MetaCommentAction* pAct, OutputDevice* pOut );


protected:

    virtual void    Linker( OutputDevice* pOut, sal_Bool bLink );
    virtual long    Hook();

public:
                    GDIMetaFile();
                    GDIMetaFile( const GDIMetaFile& rMtf );
    virtual         ~GDIMetaFile();

    GDIMetaFile&    operator=( const GDIMetaFile& rMtf );
    sal_Bool        operator==( const GDIMetaFile& rMtf ) const;
    sal_Bool        operator!=( const GDIMetaFile& rMtf ) const { return !( *this == rMtf ); }

    void            Clear();
    sal_Bool        Mirror( sal_uLong nMirrorFlags );
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
    Rectangle       GetBoundRect( OutputDevice& i_rReference, Rectangle* pHairline = 0 ) const;

    void            Adjust( short nLuminancePercent = 0, short nContrastPercent = 0,
                            short nChannelRPercent = 0,  short nChannelGPercent = 0,
                            short nChannelBPercent = 0,  double fGamma = 1.0,
                            sal_Bool bInvert = sal_False
                    );

    void            Convert( MtfConversion eConversion );
    void            ReplaceColors( const Color* pSearchColors, const Color* rReplaceColors,
                                   sal_uLong nColorCount, sal_uLong* pTols = NULL );

    GDIMetaFile     GetMonochromeMtf( const Color& rCol ) const;

    void            Record( OutputDevice* pOutDev );
    sal_Bool        IsRecord() const { return bRecord; }

    void            Play( GDIMetaFile& rMtf, size_t nPos = GDI_METAFILE_END );
    void            Play( OutputDevice* pOutDev, size_t nPos = GDI_METAFILE_END );
    void            Play( OutputDevice* pOutDev, const Point& rPos,
                          const Size& rSize, size_t nPos = GDI_METAFILE_END );

    void            Pause( sal_Bool bPause );
    sal_Bool        IsPause() const { return bPause; }

    void            Stop();

    void            WindStart();
    void            WindPrev();

    size_t          GetActionSize() const;

    void            AddAction( MetaAction* pAction );
    void            AddAction( MetaAction* pAction, size_t nPos );
    void            RemoveAction( size_t nPos );
    void            push_back( MetaAction* pAction );

    MetaAction*     FirstAction();
    MetaAction*     NextAction();
    MetaAction*     GetAction( size_t nAction ) const;
    MetaAction*     GetCurAction() const { return GetAction( nCurrentActionElement ); }
    /**
     * @param pAction takes ownership
     */
    MetaAction*     ReplaceAction( MetaAction* pAction, size_t nAction );

    const Size&     GetPrefSize() const { return aPrefSize; }
    void            SetPrefSize( const Size& rSize ) { aPrefSize = rSize; }

    const MapMode&  GetPrefMapMode() const { return aPrefMapMode; }
    void            SetPrefMapMode( const MapMode& rMapMode ) { aPrefMapMode = rMapMode; }

    void            SetHookHdl( const Link& rLink ) { aHookHdlLink = rLink; }
    const Link&     GetHookHdl() const { return aHookHdlLink; }

    sal_uLong       GetChecksum() const;
    sal_uLong       GetSizeBytes() const;

    // Methods for reading and writing the new formats;
    // the Read method also reads the old format
    SvStream&       Read( SvStream& rIStm );
    SvStream&       Write( SvStream& rOStm );

    // Stream-operators write (still) the old format
    // and read both the old and the new format
    friend VCL_DLLPUBLIC SvStream& operator>>( SvStream& rIStm, GDIMetaFile& rGDIMetaFile );
    friend VCL_DLLPUBLIC SvStream& operator<<( SvStream& rOStm, const GDIMetaFile& rGDIMetaFile );

    sal_Bool        CreateThumbnail( sal_uInt32 nMaximumExtent, BitmapEx& rBmpEx, const BitmapEx* pOverlay = NULL, const Rectangle* pOverlayRect = NULL ) const;

    void            UseCanvas( sal_Bool _bUseCanvas );
    sal_Bool        GetUseCanvas() const { return bUseCanvas; }
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
MetaCommentAction* makePluggableRendererAction( const OUString& rRendererServiceName,
                                                const OUString& rGraphicServiceName,
                                                const void* pData,
                                                sal_uInt32 nDataSize );

#endif // _SV_GDIMTF_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
