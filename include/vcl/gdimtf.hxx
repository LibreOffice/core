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

#ifndef INCLUDED_VCL_GDIMTF_HXX
#define INCLUDED_VCL_GDIMTF_HXX

#include <vcl/dllapi.h>
#include <tools/gen.hxx>
#include <vcl/mapmod.hxx>
#include <vcl/bitmap.hxx>
#include <vcl/vclptr.hxx>
#include <vector>

class OutputDevice;
class MetaAction;
class MetaCommentAction;
class SvStream;
class Color;
class BitmapEx;
namespace tools {
    class Polygon;
    class PolyPolygon;
}
class Gradient;


#define GDI_METAFILE_END                ((size_t)0xFFFFFFFF)
#define GDI_METAFILE_LABEL_NOTFOUND     ((size_t)0xFFFFFFFF)

enum class MtfConversion
{
    N1BitThreshold,
    N8BitGreys
};


typedef Color (*ColorExchangeFnc)( const Color& rColor, const void* pColParam );
typedef BitmapEx (*BmpExchangeFnc)( const BitmapEx& rBmpEx, const void* pBmpParam );

class VCL_DLLPUBLIC GDIMetaFile final
{
private:
    ::std::vector< MetaAction* > m_aList;
    size_t          m_nCurrentActionElement;

    MapMode         m_aPrefMapMode;
    Size            m_aPrefSize;
    GDIMetaFile*    m_pPrev;
    GDIMetaFile*    m_pNext;
    VclPtr<OutputDevice> m_pOutDev;
    bool            m_bPause;
    bool            m_bRecord;
    bool            m_bUseCanvas;


    SAL_DLLPRIVATE static Color         ImplColAdjustFnc( const Color& rColor, const void* pColParam );
    SAL_DLLPRIVATE static BitmapEx      ImplBmpAdjustFnc( const BitmapEx& rBmpEx, const void* pBmpParam );

    SAL_DLLPRIVATE static Color         ImplColConvertFnc( const Color& rColor, const void* pColParam );
    SAL_DLLPRIVATE static BitmapEx      ImplBmpConvertFnc( const BitmapEx& rBmpEx, const void* pBmpParam );

    SAL_DLLPRIVATE static Color         ImplColMonoFnc( const Color& rColor, const void* pColParam );
    SAL_DLLPRIVATE static BitmapEx      ImplBmpMonoFnc( const BitmapEx& rBmpEx, const void* pBmpParam );

    SAL_DLLPRIVATE static Color         ImplColReplaceFnc( const Color& rColor, const void* pColParam );
    SAL_DLLPRIVATE static BitmapEx      ImplBmpReplaceFnc( const BitmapEx& rBmpEx, const void* pBmpParam );

    SAL_DLLPRIVATE void                 ImplExchangeColors( ColorExchangeFnc pFncCol, const void* pColParam,
                                                            BmpExchangeFnc pFncBmp, const void* pBmpParam );

    SAL_DLLPRIVATE static Point         ImplGetRotatedPoint( const Point& rPt, const Point& rRotatePt,
                                                             const Size& rOffset, double fSin, double fCos );
    SAL_DLLPRIVATE static tools::Polygon ImplGetRotatedPolygon( const tools::Polygon& rPoly, const Point& rRotatePt,
                                                               const Size& rOffset, double fSin, double fCos );
    SAL_DLLPRIVATE static tools::PolyPolygon ImplGetRotatedPolyPolygon( const tools::PolyPolygon& rPoly, const Point& rRotatePt,
                                                                   const Size& rOffset, double fSin, double fCos );
    SAL_DLLPRIVATE static void          ImplAddGradientEx( GDIMetaFile& rMtf,
                                                           const OutputDevice& rMapDev,
                                                           const tools::PolyPolygon& rPolyPoly,
                                                           const Gradient& rGrad );

    SAL_DLLPRIVATE bool                 ImplPlayWithRenderer( OutputDevice* pOut, const Point& rPos, Size rLogicDestSize );

    void                                Linker( OutputDevice* pOut, bool bLink );

public:
                    GDIMetaFile();
                    GDIMetaFile( const GDIMetaFile& rMtf );
                    ~GDIMetaFile();

    GDIMetaFile&    operator=( const GDIMetaFile& rMtf );
    bool            operator==( const GDIMetaFile& rMtf ) const;
    bool            operator!=( const GDIMetaFile& rMtf ) const { return !( *this == rMtf ); }

    void            Clear();
    bool            Mirror( BmpMirrorFlags nMirrorFlags );
    void            Move( long nX, long nY );
    // additional Move method getting specifics how to handle MapMode( MapUnit::MapPixel )
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
    Rectangle       GetBoundRect( OutputDevice& i_rReference, Rectangle* pHairline = nullptr ) const;

    void            Adjust( short nLuminancePercent, short nContrastPercent,
                            short nChannelRPercent = 0,  short nChannelGPercent = 0,
                            short nChannelBPercent = 0,  double fGamma = 1.0,
                            bool bInvert = false, bool msoBrightness = false );

    void            Convert( MtfConversion eConversion );
    void            ReplaceColors( const Color* pSearchColors, const Color* rReplaceColors,
                                   sal_uLong nColorCount );

    GDIMetaFile     GetMonochromeMtf( const Color& rCol ) const;

    void            Record( OutputDevice* pOutDev );
    bool            IsRecord() const { return m_bRecord; }

    void            Play( GDIMetaFile& rMtf );
    void            Play( OutputDevice* pOutDev, size_t nPos = GDI_METAFILE_END );
    void            Play( OutputDevice* pOutDev, const Point& rPos,
                          const Size& rSize );

    void            Pause( bool bPause );
    bool            IsPause() const { return m_bPause; }

    void            Stop();

    void            WindStart();
    void            WindPrev();

    size_t          GetActionSize() const;

    void            AddAction( MetaAction* pAction );
    void            AddAction( MetaAction* pAction, size_t nPos );
    void            RemoveAction( size_t nPos );
    void            push_back( MetaAction* pAction );
    /**
     * @param pAction takes ownership
     * @param nAction the action to replace
     */
    MetaAction*     ReplaceAction( MetaAction* pAction, size_t nAction );

    MetaAction*     FirstAction();
    MetaAction*     NextAction();
    MetaAction*     GetAction( size_t nAction ) const;
    MetaAction*     GetCurAction() const { return GetAction( m_nCurrentActionElement ); }

    const Size&     GetPrefSize() const { return m_aPrefSize; }
    void            SetPrefSize( const Size& rSize ) { m_aPrefSize = rSize; }

    const MapMode&  GetPrefMapMode() const { return m_aPrefMapMode; }
    void            SetPrefMapMode( const MapMode& rMapMode ) { m_aPrefMapMode = rMapMode; }


    BitmapChecksum  GetChecksum() const;
    sal_uLong       GetSizeBytes() const;

    // Methods for reading and writing the new formats;
    // the Read method also reads the old format
    SvStream&       Read( SvStream& rIStm );
    SvStream&       Write( SvStream& rOStm );

    // Stream-operators write (still) the old format
    // and read both the old and the new format
    friend VCL_DLLPUBLIC SvStream& ReadGDIMetaFile( SvStream& rIStm, GDIMetaFile& rGDIMetaFile );
    friend VCL_DLLPUBLIC SvStream& WriteGDIMetaFile( SvStream& rOStm, const GDIMetaFile& rGDIMetaFile );

    /// Creates an antialiased thumbnail, with maximum width or height of nMaximumExtent.
    bool            CreateThumbnail(BitmapEx& rBitmapEx,
                                    sal_uInt32 nMaximumExtent = 256,
                                    BmpConversion nColorConversion = BmpConversion::N24Bit,
                                    BmpScaleFlag nScaleFlag = BmpScaleFlag::BestQuality) const;

    void            UseCanvas( bool _bUseCanvas );
    bool            GetUseCanvas() const { return m_bUseCanvas; }
};

#endif // INCLUDED_VCL_GDIMTF_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
