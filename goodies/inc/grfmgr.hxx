/*************************************************************************
 *
 *  $RCSfile: grfmgr.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:30:09 $
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

#ifndef _GRFMGR_HXX
#define _GRFMGR_HXX

#ifndef _SV_GRAPH_HXX
#include <vcl/graph.hxx>
#endif

// -----------
// - Defines -
// -----------

#define GRFMGR_DRAW_NOTCACHED           0x00000000UL
#define GRFMGR_DRAW_CACHED              0x00000001UL
#define GRFMGR_DRAW_BILINEAR            0x00000002UL
#define GRFMGR_DRAW_STANDARD            (GRFMGR_DRAW_CACHED|GRFMGR_DRAW_BILINEAR)

// --------------------
// - AutoSwap Defines -
// --------------------

#define GRFMGR_AUTOSWAPSTREAM_LINK      ((SvStream*)0x00000000UL)
#define GRFMGR_AUTOSWAPSTREAM_LOADED    ((SvStream*)0xfffffffdUL)
#define GRFMGR_AUTOSWAPSTREAM_TEMP      ((SvStream*)0xfffffffeUL)
#define GRFMGR_AUTOSWAPSTREAM_NONE      ((SvStream*)0xffffffffUL)

// ----------------------
// - Adjustment Defines -
// ----------------------

#define ADJUSTMENT_NONE                 0x00000000UL
#define ADJUSTMENT_DRAWMODE             0x00000001UL
#define ADJUSTMENT_COLORS               0x00000002UL
#define ADJUSTMENT_MIRROR               0x00000004UL
#define ADJUSTMENT_ROTATE               0x00000008UL
#define ADJUSTMENT_TRANSPARENCY         0x00000010UL
#define ADJUSTMENT_ALL                  0xFFFFFFFFUL

// ---------
// - Enums -
// ---------

enum GraphicDrawMode
{
    GRAPHICDRAWMODE_STANDARD = 0,
    GRAPHICDRAWMODE_GREYS = 1,
    GRAPHICDRAWMODE_MONO = 2,
    GRAPHICDRAWMODE_WATERMARK = 3
};

// ------------
// - Forwards -
// ------------

class GraphicManager;
class SvStream;
class BitmapWriteAccess;
class GraphicCache;
struct GrfSimpleCacheObj;

// ---------------
// - GraphicAttr -
// ---------------

class GraphicAttr
{
private:

    Size            maLogSize;
    double          mfGamma;
    ULONG           mnMirrFlags;
    long            mnLeftCrop;
    long            mnTopCrop;
    long            mnRightCrop;
    long            mnBottomCrop;
    USHORT          mnRotate10;
    short           mnContPercent;
    short           mnLumPercent;
    short           mnRPercent;
    short           mnGPercent;
    short           mnBPercent;
    BOOL            mbInvert;
    BYTE            mcTransparency;
    GraphicDrawMode meDrawMode;
    Rectangle       maCropRect;

    void*           mpDummy;

public:

                    GraphicAttr();
                    ~GraphicAttr();

    BOOL            operator==( const GraphicAttr& rAttr ) const;
    BOOL            operator!=( const GraphicAttr& rAttr ) const { return !( *this == rAttr ); }

    void            SetDrawMode( GraphicDrawMode eDrawMode ) { meDrawMode = eDrawMode; }
    GraphicDrawMode GetDrawMode() const { return meDrawMode; }

    void            SetMirrorFlags( ULONG nMirrFlags ) { mnMirrFlags = nMirrFlags; }
    ULONG           GetMirrorFlags() const { return mnMirrFlags; }

    void            SetCrop( long nLeft_100TH_MM, long nTop_100TH_MM, long nRight_100TH_MM, long nBottom_100TH_MM )
                    {
                        mnLeftCrop = nLeft_100TH_MM; mnTopCrop = nTop_100TH_MM;
                        mnRightCrop = nRight_100TH_MM; mnBottomCrop = nBottom_100TH_MM;
                    }
    long            GetLeftCrop() const { return mnLeftCrop; }
    long            GetTopCrop() const { return mnTopCrop; }
    long            GetRightCrop() const { return mnRightCrop; }
    long            GetBottomCrop() const { return mnBottomCrop; }

    void            SetRotation( USHORT nRotate10, const Size& rUnrotatedSize ) { mnRotate10 = nRotate10; maLogSize = rUnrotatedSize; }
    USHORT          GetRotation() const { return mnRotate10; }

    void            SetLuminance( short nLuminancePercent ) { mnLumPercent = nLuminancePercent; }
    short           GetLuminance() const { return mnLumPercent; }

    void            SetContrast( short nContrastPercent ) { mnContPercent = nContrastPercent; }
    short           GetContrast() const { return mnContPercent; }

    void            SetChannelR( short nChannelRPercent ) { mnRPercent = nChannelRPercent; }
    short           GetChannelR() const { return mnRPercent; }

    void            SetChannelG( short nChannelGPercent ) { mnGPercent = nChannelGPercent; }
    short           GetChannelG() const { return mnGPercent; }

    void            SetChannelB( short nChannelBPercent ) { mnBPercent = nChannelBPercent; }
    short           GetChannelB() const { return mnBPercent; }

    void            SetGamma( double fGamma ) { mfGamma = fGamma; }
    double          GetGamma() const { return mfGamma; }

    void            SetInvert( BOOL bInvert ) { mbInvert = bInvert; }
    BOOL            IsInvert() const { return mbInvert; }

    void            SetTransparency( BYTE cTransparency ) { mcTransparency = cTransparency; }
    BYTE            GetTransparency() const { return mcTransparency; }

    const Size&     GetUntransformedSize() const { return maLogSize; }

    BOOL            IsSpecialDrawMode() const { return( meDrawMode != GRAPHICDRAWMODE_STANDARD ); }
    BOOL            IsMirrored() const { return( mnMirrFlags != 0UL ); }
    BOOL            IsCropped() const
                    {
                        return( mnLeftCrop != 0 || mnTopCrop != 0 ||
                                mnRightCrop != 0 || mnBottomCrop != 0 );
                    }
    BOOL            IsRotated() const { return( ( mnRotate10 % 3600 ) != 0 ); }
    BOOL            IsTransparent() const { return( mcTransparency > 0 ); }
    BOOL            IsAdjusted() const
                    {
                        return( mnLumPercent != 0 || mnContPercent != 0 || mnRPercent != 0 ||
                                mnGPercent != 0 || mnBPercent != 0 || mfGamma != 1.0 || mbInvert );
                    }

    friend SvStream& operator<<( SvStream& rOStm, const GraphicAttr& rAttr );
    friend SvStream& operator>>( SvStream& rIStm, GraphicAttr& rAttr );
};

// -----------------
// - GraphicObject -
// -----------------

class GraphicObject : public SvDataCopyStream
{
    friend class GraphicManager;

private:

    static GraphicManager*  mpGlobalMgr;

    Graphic                 maGraphic;
    GraphicAttr             maAttr;
    Size                    maPrefSize;
    MapMode                 maPrefMapMode;
    ULONG                   mnSizeBytes;
    GraphicType             meType;
    GraphicManager*         mpMgr;
    String*                 mpLink;
    Link*                   mpSwapStreamHdl;
    void*                   mpDummy1;
    Timer*                  mpSwapOutTimer;
    GrfSimpleCacheObj*      mpSimpleCache;
    void*                   mpDummy2;
    BOOL                    mbAutoSwapped   : 1;
    BOOL                    mbTransparent   : 1;
    BOOL                    mbAnimated      : 1;
    BOOL                    mbEPS           : 1;
    BOOL                    mbIsInSwapIn    : 1;
    BOOL                    mbIsInSwapOut   : 1;
    BOOL                    mbDummyFlag7    : 1;
    BOOL                    mbDummyFlag8    : 1;

#if __PRIVATE

    void                    ImplConstruct();
    void                    ImplAssignGraphicData();
    void                    ImplSetGraphicManager( const GraphicManager* pMgr, const ByteString* pID = NULL );
    void                    ImplAutoSwapIn( BOOL bIgnoreSwapState );
    BOOL                    ImplIsAutoSwapped() const { return mbAutoSwapped; }

                            DECL_LINK( ImplAutoSwapOutHdl, void* );

#endif // __PRIVATE

protected:

    virtual void            GraphicManagerDestroyed();
    virtual SvStream*       GetSwapStream() const;

    // !!! to be removed
    virtual ULONG           GetReleaseFromCache() const;

    virtual void            Load( SvStream& );
    virtual void            Save( SvStream& );
    virtual void            Assign( const SvDataCopyStream& );

public:

                            TYPEINFO();

                            GraphicObject( const GraphicManager* pMgr = NULL );
                            GraphicObject( const Graphic& rGraphic, const GraphicManager* pMgr = NULL );
                            GraphicObject( const Graphic& rGraphic, const String& rLink, const GraphicManager* pMgr = NULL );
                            GraphicObject( const GraphicObject& rCacheObj, const GraphicManager* pMgr = NULL );
                            GraphicObject( const ByteString& rUniqueID, const GraphicManager* pMgr = NULL );
                            ~GraphicObject();

    GraphicObject&          operator=( const GraphicObject& rCacheObj );
    BOOL                    operator==( const GraphicObject& rCacheObj ) const;
    BOOL                    operator!=( const GraphicObject& rCacheObj ) const { return !( *this == rCacheObj ); }

    BOOL                    HasSwapStreamHdl() const { return( mpSwapStreamHdl != NULL && mpSwapStreamHdl->IsSet() ); }
    void                    SetSwapStreamHdl();
    void                    SetSwapStreamHdl( const Link& rHdl, const ULONG nSwapOutTimeout = 0UL );
    Link                    GetSwapStreamHdl() const;
    ULONG                   GetSwapOutTimeout() const { return( mpSwapOutTimer ? mpSwapOutTimer->GetTimeout() : 0 ); }

    void                    FireSwapInRequest();
    void                    FireSwapOutRequest();

    void                    SetGraphicManager( const GraphicManager& rMgr );
    GraphicManager&         GetGraphicManager() const { return *mpMgr; }

    BOOL                    IsCached( OutputDevice* pOut, const Point& rPt, const Size& rSz,
                                      const GraphicAttr* pAttr = NULL, ULONG nFlags = GRFMGR_DRAW_STANDARD) const;
    void                    ReleaseFromCache();

    const Graphic&          GetGraphic() const;
    void                    SetGraphic( const Graphic& rGraphic );
    void                    SetGraphic( const Graphic& rGraphic, const String& rLink );

    Graphic                 GetTransformedGraphic( const GraphicAttr* pAttr = NULL ) const;

    void                    SetAttr( const GraphicAttr& rAttr );
    const GraphicAttr&      GetAttr() const { return maAttr; }

    BOOL                    HasLink() const { return( mpLink != NULL && mpLink->Len() > 0 ); }
    void                    SetLink();
    void                    SetLink( const String& rLink );
    String                  GetLink() const;

    ByteString              GetUniqueID() const;

    GraphicType             GetType() const { return meType; }
    const Size&             GetPrefSize() const { return maPrefSize; }
    const MapMode&          GetPrefMapMode() const { return maPrefMapMode; }
    ULONG                   GetSizeBytes() const { return mnSizeBytes; }
    ULONG                   GetChecksum() const;
    BOOL                    IsTransparent() const { return mbTransparent; }
    BOOL                    IsAnimated() const { return mbAnimated; }
    BOOL                    IsEPS() const { return mbEPS; }

    void                    ResetAnimationLoopCount();
    List*                   GetAnimationInfoList() const;
    Link                    GetAnimationNotifyHdl() const { return maGraphic.GetAnimationNotifyHdl(); }
    void                    SetAnimationNotifyHdl( const Link& rLink );

    BOOL                    SwapOut();
    BOOL                    SwapOut( SvStream* pOStm );
    BOOL                    SwapIn();
    BOOL                    SwapIn( SvStream* pIStm );

    BOOL                    IsInSwapIn() const { return mbIsInSwapIn; }
    BOOL                    IsInSwapOut() const { return mbIsInSwapOut; }
    BOOL                    IsInSwap() const { return( mbIsInSwapOut || mbIsInSwapOut ); }
    BOOL                    IsSwappedOut() const { return maGraphic.IsSwapOut(); }

    BOOL                    Draw( OutputDevice* pOut, const Point& rPt, const Size& rSz,
                                  const GraphicAttr* pAttr = NULL, ULONG nFlags = GRFMGR_DRAW_STANDARD );

    BOOL                    StartAnimation( OutputDevice* pOut, const Point& rPt, const Size& rSz, long nExtraData = 0L,
                                            const GraphicAttr* pAttr = NULL, ULONG nFlags = GRFMGR_DRAW_STANDARD,
                                            OutputDevice* pFirstFrameOutDev = NULL );

    void                    StopAnimation( OutputDevice* pOut = NULL, long nExtraData = 0L );

    friend SvStream&        operator<<( SvStream& rOStm, const GraphicObject& rGraphicObj );
    friend SvStream&        operator>>( SvStream& rIStm, GraphicObject& rGraphicObj );
};

// ------------------
// - GraphicManager -
// ------------------

class GraphicManager
{
    friend class GraphicObject;
    friend class GraphicDisplayCacheEntry;

private:

    List            maObjList;
    GraphicCache*   mpCache;

                    GraphicManager( const GraphicManager& rGraphicManager ) {}
    GraphicManager& operator=( const GraphicManager& rGraphicManager ) { return *this; }

#if __PRIVATE

    BOOL            ImplDraw( OutputDevice* pOut, const Point& rPt,
                              const Size& rSz, GraphicObject& rObj,
                              const GraphicAttr& rAttr, BOOL& rCached );

    BOOL            ImplCreateOutput( OutputDevice* pOut, const Point& rPt, const Size& rSz,
                                      const BitmapEx& rBmpEx, const GraphicAttr& rAttr,
                                      BitmapEx* pBmpEx = NULL );
    BOOL            ImplCreateOutput( OutputDevice* pOut, const Point& rPt, const Size& rSz,
                                      const GDIMetaFile& rMtf, const GraphicAttr& rAttr,
                                      GDIMetaFile* pMtf = NULL );

    BOOL            ImplCreateScaled( const BitmapEx& rBmpEx,
                                      long* pMapIX, long* pMapFX, long* pMapIY, long* pMapFY,
                                      long nStartX, long nEndX, long nStartY, long nEndY,
                                      BitmapEx& rOutBmpEx );

    BOOL            ImplCreateRotatedScaled( const BitmapEx& rBmpEx,
                                             USHORT nRot10, const Size& rOutSzPix, const Size& rUntSzPix,
                                             long* pMapIX, long* pMapFX, long* pMapIY, long* pMapFY,
                                             long nStartX, long nEndX, long nStartY, long nEndY,
                                             BitmapEx& rOutBmpEx );

    static void     ImplAdjust( BitmapEx& rBmpEx, const GraphicAttr& rAttr, ULONG nAdjustmentFlags );
    static void     ImplAdjust( GDIMetaFile& rMtf, const GraphicAttr& rAttr, ULONG nAdjustmentFlags );
    static void     ImplAdjust( Animation& rAnimation, const GraphicAttr& rAttr, ULONG nAdjustmentFlags );

    static void     ImplDraw( OutputDevice* pOut, const Point& rPt, const Size& rSz,
                              const BitmapEx& rBmpEx, const GraphicAttr& rAttr );
    static void     ImplDraw( OutputDevice* pOut, const Point& rPt, const Size& rSz,
                              const GDIMetaFile& rMtf, const GraphicAttr& rAttr );

                    // Only used by GraphicObject's Ctor's and Dtor's
    void            ImplRegisterObj( const GraphicObject& rObj, Graphic& rSubstitute, const ByteString* pID );
    void            ImplUnregisterObj( const GraphicObject& rObj );
    inline BOOL     ImplHasObjects() const { return( maObjList.Count() > 0UL ); }

                    // Only used in swap case by GraphicObject
    void            ImplGraphicObjectWasSwappedOut( const GraphicObject& rObj );
    BOOL            ImplFillSwappedGraphicObject( const GraphicObject& rObj, Graphic& rSubstitute );
    void            ImplGraphicObjectWasSwappedIn( const GraphicObject& rObj );

    ByteString      ImplGetUniqueID( const GraphicObject& rObj ) const;

#endif // __PRIVATE

public:

                    GraphicManager( ULONG nCacheSize = 10000000UL, ULONG nMaxObjCacheSize = 2400000UL );
                    ~GraphicManager();

    void            SetMaxCacheSize( ULONG nNewCacheSize );
    ULONG           GetMaxCacheSize() const;

    void            SetMaxObjCacheSize( ULONG nNewMaxObjSize, BOOL bDestroyGreaterCached = FALSE );
    ULONG           GetMaxObjCacheSize() const;

    ULONG           GetUsedCacheSize() const;
    ULONG           GetFreeCacheSize() const;

    void            ClearCache();

    void            ReleaseFromCache( const GraphicObject& rObj );

    BOOL            IsInCache( OutputDevice* pOut, const Point& rPt, const Size& rSz,
                               const GraphicObject& rObj, const GraphicAttr& rAttr ) const;

    BOOL            DrawObj( OutputDevice* pOut, const Point& rPt, const Size& rSz,
                             GraphicObject& rObj, const GraphicAttr& rAttr,
                             const ULONG nFlags, BOOL& rCached );
};

#endif // _GRFMGR_HXX
