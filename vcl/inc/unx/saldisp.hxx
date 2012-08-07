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

#ifndef _SV_SALDISP_HXX
#define _SV_SALDISP_HXX

// -=-= exports =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class   SalDisplay;
class   SalColormap;
class   SalVisual;
class   SalXLib;

// -=-= #includes =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#include <unx/salunx.h>
#include <unx/saltype.h>
#include <vcl/salgtype.hxx>
#include <vcl/ptrstyle.hxx>
#include <sal/types.h>
#include <osl/mutex.h>
#include <vector>
#include <list>
#include <boost/unordered_map.hpp>
#include <tools/gen.hxx>
#include <salwtype.hxx>
#include <generic/gendisp.hxx>

#include <vclpluginapi.h>

// -=-= forwards -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class   BitmapPalette;
class   SalFrame;
class   ColorMask;

namespace vcl_sal { class WMAdaptor; }

// -=-= #defines -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#define PROPERTY_SUPPORT_WM_Parent_Pixmap_None  0x00000004
#define PROPERTY_SUPPORT_WM_ClientPos           0x00000008
#define PROPERTY_SUPPORT_3ButtonMouse           0x00000020

#define PROPERTY_BUG_XCopyArea_GXxor            0x00002000  // from window
#define PROPERTY_BUG_Tile                       0x00008000  // Recreate the
                                            // dither brush each time
#define PROPERTY_BUG_FillPolygon_Tile           0x00010000  // always Toggle Fillstyle
#define PROPERTY_BUG_DrawLine                   0x00020000  // a DrawLine is one point to short
#define PROPERTY_BUG_Bitmap_Bit_Order           0x00100000

#define PROPERTY_FEATURE_SharedMemory           0x02000000
#define PROPERTY_FEATURE_TrustedSolaris         0x04000000

#define PROPERTY_DEFAULT                        0x00000FCB

// ------------------------------------------------------------------------
// server vendor

typedef enum  {
    vendor_none = 0,
    vendor_sun,
    vendor_unknown
} srv_vendor_t;

extern "C" srv_vendor_t sal_GetServerVendor( Display *p_display );

// -=-= SalRGB -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// MSB/Bigendian view (SalColor == RGB, r=0xFF0000, g=0xFF00, b=0xFF)

enum SalRGB { RGB,  RBG,
              GBR,  GRB,
              BGR,  BRG,
              RGBA, RBGA,
              GBRA, GRBA,
              BGRA, BRGA,
              otherSalRGB };

// -=-= SalVisual =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class SalVisual : public XVisualInfo
{
    SalRGB          eRGBMode_;
    int             nRedShift_;
    int             nGreenShift_;
    int             nBlueShift_;
    int             nRedBits_;
    int             nGreenBits_;
    int             nBlueBits_;
public:
                            SalVisual();
                            ~SalVisual();
                            SalVisual( const XVisualInfo* pXVI );

    inline  VisualID        GetVisualId() const { return visualid; }
    inline  Visual         *GetVisual() const { return visual; }
    inline  int             GetClass() const { return c_class; }
    inline  int             GetDepth() const { return depth; }
    inline  SalRGB          GetMode() const { return eRGBMode_; }

            Pixel           GetTCPixel( SalColor nColor ) const;
            SalColor        GetTCColor( Pixel nPixel ) const;
};

// -=-= SalColormap =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class SalColormap
{
    const SalDisplay*       m_pDisplay;
    Colormap                m_hColormap;
    std::vector<SalColor>   m_aPalette;         // Pseudocolor
    SalVisual               m_aVisual;
    std::vector<sal_uInt16>     m_aLookupTable;     // Pseudocolor: 12bit reduction
    Pixel                   m_nWhitePixel;
    Pixel                   m_nBlackPixel;
    Pixel                   m_nUsed;            // Pseudocolor
    SalX11Screen            m_nXScreen;

    void            GetPalette();
    void            GetLookupTable();
public:
    SalColormap( const SalDisplay*  pSalDisplay,
                 Colormap           hColormap,
                 SalX11Screen       nXScreen );
    SalColormap( sal_uInt16         nDepth );
    SalColormap();
    ~SalColormap();

    inline  Colormap            GetXColormap() const { return m_hColormap; }
    inline  const SalDisplay*   GetDisplay() const { return m_pDisplay; }
    inline  Display*            GetXDisplay() const;
    inline  const SalVisual&    GetVisual() const { return m_aVisual; }
    inline  Visual*             GetXVisual() const { return m_aVisual.GetVisual(); }
    inline  Pixel               GetWhitePixel() const { return m_nWhitePixel; }
    inline  Pixel               GetBlackPixel() const { return m_nBlackPixel; }
    inline  Pixel               GetUsed() const { return m_nUsed; }
    inline  int                 GetClass() const { return m_aVisual.GetClass(); }

    sal_Bool            GetXPixels( XColor  &rColor,
                                    int      r,
                                    int      g,
                                    int      b ) const;
    inline  sal_Bool            GetXPixel( XColor  &rColor,
                                           int      r,
                                           int      g,
                                           int      b ) const;
    Pixel           GetPixel( SalColor nColor ) const;
    SalColor        GetColor( Pixel nPixel ) const;
};

// -=-= SalXLib =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
typedef int(*YieldFunc)(int fd, void* data);
struct YieldEntry;

class VCLPLUG_GEN_PUBLIC SalXLib
{
protected:
    timeval         m_aTimeout;
    sal_uLong       m_nTimeoutMS;
    int             m_pTimeoutFDS[2];

    int             nFDs_;
    fd_set          aReadFDS_;
    fd_set          aExceptionFDS_;
    YieldEntry      *pYieldEntries_;

public:
    SalXLib();
    virtual         ~SalXLib();
    virtual void    Init();

    virtual void    Yield( bool bWait, bool bHandleAllCurrentEvents );
    virtual void    Wakeup();
    virtual void    PostUserEvent();

    virtual void    Insert( int fd, void* data,
                            YieldFunc   pending,
                            YieldFunc   queued,
                            YieldFunc   handle );
    virtual void    Remove( int fd );

    virtual void    StartTimer( sal_uLong nMS );
    virtual void    StopTimer();

    bool            CheckTimeout( bool bExecuteTimers = true );
};

// -=-= SalDisplay -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

class SalI18N_InputMethod;
class SalI18N_KeyboardExtension;
class AttributeProvider;

extern "C" {
    typedef Bool(*X_if_predicate)(Display*,XEvent*,XPointer);
}

class VCLPLUG_GEN_PUBLIC SalDisplay : public SalGenericDisplay
{
public:
    struct RenderEntry
    {
        Pixmap      m_aPixmap;
        Picture     m_aPicture;

        RenderEntry() : m_aPixmap( 0 ), m_aPicture( 0 ) {}
    };

    typedef boost::unordered_map<int,RenderEntry> RenderEntryMap;

    struct ScreenData
    {
        bool                m_bInit;

        XLIB_Window         m_aRoot;
        XLIB_Window         m_aRefWindow;
        Size                m_aSize;
        SalVisual           m_aVisual;
        SalColormap         m_aColormap;
        GC                  m_aMonoGC;
        GC                  m_aCopyGC;
        GC                  m_aAndInvertedGC;
        GC                  m_aAndGC;
        GC                  m_aOrGC;
        GC                  m_aStippleGC;
        Pixmap              m_hInvert50;
        mutable RenderEntryMap m_aRenderData;

        ScreenData() :
        m_bInit( false ),
        m_aRoot( None ),
        m_aRefWindow( None ),
        m_aMonoGC( None ),
        m_aCopyGC( None ),
        m_aAndInvertedGC( None ),
        m_aAndGC( None ),
        m_aOrGC( None ),
        m_aStippleGC( None ),
        m_hInvert50( None ),
        m_aRenderData( 1 )
        {}
    };

protected:
    SalXLib        *pXLib_;
    SalI18N_InputMethod         *mpInputMethod;
    SalI18N_KeyboardExtension   *mpKbdExtension;

    AttributeProvider           *mpFactory;

    Display        *pDisp_;             // X Display

    SalX11Screen                 m_nXDefaultScreen;
    std::vector< ScreenData >    m_aScreens;
    ScreenData      m_aInvalidScreenData;
    Pair            aResolution_;       // [dpi]
    bool            mbExactResolution;
    sal_uLong       nMaxRequestSize_;   // [byte]

    srv_vendor_t    meServerVendor;

    // until x bytes

    XLIB_Cursor     aPointerCache_[POINTER_COUNT];

    // Keyboard
    sal_Bool        bNumLockFromXS_;    // Num Lock handled by X Server
    int             nNumLockIndex_;     // modifier index in modmap
    int             nNumLockMask_;      // keyevent state mask for
    KeySym          nShiftKeySym_;      // first shift modifier
    KeySym          nCtrlKeySym_;       // first control modifier
    KeySym          nMod1KeySym_;       // first mod1 modifier

    vcl_sal::WMAdaptor* m_pWMAdaptor;

    bool            m_bXinerama;
    std::vector< Rectangle > m_aXineramaScreens;
    std::vector< int > m_aXineramaScreenIndexMap;
    std::list<SalObject*> m_aSalObjects;

    bool            m_bUseRandRWrapper; // don't use randr on gtk, use gdk signals there

    mutable XLIB_Time  m_nLastUserEventTime; // mutable because changed on first access

    virtual long    Dispatch( XEvent *pEvent ) = 0;
    void            InitXinerama();
    void            InitRandR( XLIB_Window aRoot ) const;
    void            DeInitRandR();
    int             processRandREvent( XEvent* );

    void            doDestruct();
    void            addXineramaScreenUnique( int i, long i_nX, long i_nY, long i_nWidth, long i_nHeight );
public:
    static SalDisplay *GetSalDisplay( Display* display );
    static sal_Bool BestVisual( Display     *pDisp,
                                int          nScreen,
                                XVisualInfo &rVI );

                    SalDisplay( Display* pDisp );

    virtual        ~SalDisplay();

    void            Init();
    void            PrintInfo() const;

#ifdef DBG_UTIL
    void            DbgPrintDisplayEvent(const char *pComment, XEvent *pEvent) const;
#endif

    void            ModifierMapping();
    void            SimulateKeyPress( sal_uInt16 nKeyCode );
    sal_uInt16          GetIndicatorState() const;
    rtl::OUString          GetKeyNameFromKeySym( KeySym keysym ) const;
    rtl::OUString       GetKeyName( sal_uInt16 nKeyCode ) const;
    sal_uInt16          GetKeyCode( KeySym keysym, char*pcPrintable ) const;
    KeySym          GetKeySym( XKeyEvent      *pEvent,
                               unsigned char  *pPrintable,
                               int            *pLen,
                               KeySym         *pUnmodifiedKeySym,
                               Status         *pStatus,
                               XIC = NULL ) const;

    XLIB_Cursor           GetPointer( int ePointerStyle );
    virtual int           CaptureMouse( SalFrame *pCapture );

    void                  Remove( XEvent   *pEvent );
    virtual ScreenData   *initScreen( SalX11Screen nXScreen ) const;
    const ScreenData&     getDataForScreen( SalX11Screen nXScreen ) const
    {
        if( nXScreen.getXScreen() >= m_aScreens.size() )
            return m_aInvalidScreenData;
        if( ! m_aScreens[nXScreen.getXScreen()].m_bInit )
            initScreen( nXScreen );
        return m_aScreens[nXScreen.getXScreen()];
    }

    XLIB_Window     GetDrawable( SalX11Screen nXScreen ) const { return getDataForScreen( nXScreen ).m_aRefWindow; }
    Display        *GetDisplay() const { return pDisp_; }
    SalX11Screen    GetDefaultXScreen() const { return m_nXDefaultScreen; }
    const Size&     GetScreenSize( SalX11Screen nXScreen ) const { return getDataForScreen( nXScreen ).m_aSize; }
    srv_vendor_t    GetServerVendor() const { return meServerVendor; }
    void            SetServerVendor() { meServerVendor = sal_GetServerVendor(pDisp_); }
    sal_Bool        IsDisplay() const { return !!pXLib_; }
    GC              GetMonoGC( SalX11Screen nXScreen ) const { return getDataForScreen(nXScreen).m_aMonoGC; }
    GC              GetCopyGC( SalX11Screen nXScreen ) const { return getDataForScreen(nXScreen).m_aCopyGC; }
    GC              GetAndInvertedGC( SalX11Screen nXScreen ) const { return getDataForScreen(nXScreen).m_aAndInvertedGC; }
    GC              GetAndGC( SalX11Screen nXScreen ) const { return getDataForScreen(nXScreen).m_aAndGC; }
    GC              GetOrGC( SalX11Screen nXScreen ) const { return getDataForScreen(nXScreen).m_aOrGC; }
    GC              GetStippleGC( SalX11Screen nXScreen ) const { return getDataForScreen(nXScreen).m_aStippleGC; }
    GC              GetGC( sal_uInt16 nDepth, SalX11Screen nXScreen ) const;
    Pixmap          GetInvert50( SalX11Screen nXScreen ) const { return getDataForScreen(nXScreen).m_hInvert50; }
    const SalColormap&    GetColormap( SalX11Screen nXScreen ) const { return getDataForScreen(nXScreen).m_aColormap; }
    const SalVisual&      GetVisual( SalX11Screen nXScreen ) const { return getDataForScreen(nXScreen).m_aVisual; }
    RenderEntryMap&       GetRenderEntries( SalX11Screen nXScreen ) const { return getDataForScreen(nXScreen).m_aRenderData; }
    const Pair     &GetResolution() const { return aResolution_; }
    bool            GetExactResolution() const { return mbExactResolution; }
    sal_uLong       GetProperties() const { return PROPERTY_DEFAULT; }
    sal_uLong       GetMaxRequestSize() const { return nMaxRequestSize_; }
    XLIB_Time       GetLastUserEventTime( bool bAlwaysReget = false ) const;

    bool            XIfEventWithTimeout( XEvent*, XPointer, X_if_predicate, long i_nTimeout = 1000 ) const;
    SalXLib*        GetXLib() const { return pXLib_; }

    SalI18N_InputMethod*        GetInputMethod()  const { return mpInputMethod;  }
    SalI18N_KeyboardExtension*  GetKbdExtension() const { return mpKbdExtension; }
    void            SetInputMethod( SalI18N_InputMethod *pInputMethod )
    { mpInputMethod = pInputMethod; }
    void            SetKbdExtension(SalI18N_KeyboardExtension *pKbdExtension)
    { mpKbdExtension = pKbdExtension; }
    ::vcl_sal::WMAdaptor* getWMAdaptor() const { return m_pWMAdaptor; }
    bool            IsXinerama() const { return m_bXinerama; }
    const std::vector< Rectangle >& GetXineramaScreens() const { return m_aXineramaScreens; }
    XLIB_Window     GetRootWindow( SalX11Screen nXScreen ) const
            { return getDataForScreen( nXScreen ).m_aRoot; }
    const std::vector< ScreenData >& GetScreenData() { return m_aScreens; }
    unsigned int GetXScreenCount() const { return m_aScreens.size(); }

    const std::list< SalFrame* >& getFrames() const { return m_aFrames; }
    sal_Bool            IsNumLockFromXS() const { return bNumLockFromXS_; }

    std::list< SalObject* >& getSalObjects() { return m_aSalObjects; }

    virtual void    PostUserEvent() = 0;
};

// -=-= inlines =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

inline GC SalDisplay::GetGC( sal_uInt16 nDepth, SalX11Screen nXScreen ) const
{ return 1 == nDepth
         ? GetMonoGC( nXScreen )
         : getDataForScreen(nXScreen).m_aVisual.GetDepth() == nDepth
           ? GetCopyGC( nXScreen )
           : None; }

inline  Display *SalColormap::GetXDisplay() const
{ return m_pDisplay->GetDisplay(); }

class VCLPLUG_GEN_PUBLIC SalX11Display : public SalDisplay
{
public:
             SalX11Display( Display* pDisp );
    virtual ~SalX11Display();

    virtual long        Dispatch( XEvent *pEvent );
    virtual void        Yield();
    virtual void        PostUserEvent();

    sal_Bool            IsEvent();
    void                SetupInput( SalI18N_InputMethod *pInputMethod );
};


// get foreign key names
namespace vcl_sal {
    rtl::OUString getKeysymReplacementName(
        rtl::OUString pLang,
        KeySym nSymbol );
}


#endif // _SV_SALDISP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
