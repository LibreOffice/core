/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: gdimtf.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 17:54:43 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _SV_GDIMTF_HXX
#define _SV_GDIMTF_HXX

#ifndef _VCL_DLLAPI_H
#include <vcl/dllapi.h>
#endif

#ifndef _GEN_HXX
#include <tools/gen.hxx>
#endif
#ifndef _LIST_HXX
#include <tools/list.hxx>
#endif
#ifndef _LINK_HXX
#include <tools/link.hxx>
#endif
#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef _SV_MAPMOD_HXX
#include <vcl/mapmod.hxx>
#endif

class OutputDevice;
class ImpLabelList;
class MetaAction;
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

//#if 0 // _SOLAR__PRIVATE

typedef Color (*ColorExchangeFnc)( const Color& rColor, const void* pColParam );
typedef BitmapEx (*BmpExchangeFnc)( const BitmapEx& rBmpEx, const void* pBmpParam );

//#endif // __PRIVATE

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

//#if 0 // _SOLAR__PRIVATE

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

//#endif // __PRIVATE

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
    void            Scale( double fScaleX, double fScaleY );
    void            Scale( const Fraction& rScaleX, const Fraction& rScaleY );
    void            Rotate( long nAngle10 );

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
};

#endif // _SV_GDIMTF_HXX

