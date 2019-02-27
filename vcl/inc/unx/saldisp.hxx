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

#ifndef INCLUDED_VCL_INC_UNX_SALDISP_HXX
#define INCLUDED_VCL_INC_UNX_SALDISP_HXX

class   SalDisplay;
class   SalColormap;
class   SalVisual;
class   SalXLib;

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/Xrender.h>
#include <epoxy/glx.h>

#include <rtl/string.hxx>
#include <unx/salunx.h>
#include <unx/saltype.h>
#include <vcl/opengl/OpenGLContext.hxx>
#include <vcl/salgtype.hxx>
#include <vcl/ptrstyle.hxx>
#include <sal/types.h>
#include <osl/mutex.h>
#include <cassert>
#include <list>
#include <unordered_map>
#include <vector>
#include <tools/gen.hxx>
#include <salwtype.hxx>
#include <unx/gendata.hxx>
#include <unx/gendisp.hxx>
#include <o3tl/enumarray.hxx>

#include <vclpluginapi.h>

/* From <X11/Intrinsic.h> */
typedef unsigned long Pixel;

class   BitmapPalette;
class   SalFrame;
class   ColorMask;

namespace vcl_sal { class WMAdaptor; }

// server vendor

typedef enum  {
    vendor_none = 0,
    vendor_sun,
    vendor_unknown
} srv_vendor_t;

extern "C" srv_vendor_t sal_GetServerVendor( Display *p_display );

// MSB/Bigendian view (Color == RGB, r=0xFF0000, g=0xFF00, b=0xFF)

enum class SalRGB { RGB,  RBG,
              GBR,  GRB,
              BGR,  BRG,
              otherSalRGB };

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
                            SalVisual( const XVisualInfo* pXVI );

    VisualID        GetVisualId() const { return visualid; }
    Visual         *GetVisual() const { return visual; }
    int             GetClass() const { return c_class; }
    int             GetDepth() const { return depth; }

            Pixel           GetTCPixel( Color nColor ) const;
            Color           GetTCColor( Pixel nPixel ) const;
};

// A move-only flag, used by SalColormap to track ownership of its m_aVisual.visual:
struct OwnershipFlag {
    bool owner = false;

    OwnershipFlag() = default;

    OwnershipFlag(OwnershipFlag && other) noexcept: owner(other.owner) { other.owner = false; }

    OwnershipFlag & operator =(OwnershipFlag && other) noexcept {
        assert(&other != this);
        owner = other.owner;
        other.owner = false;
        return *this;
    }
};

class SalColormap
{
    const SalDisplay*       m_pDisplay;
    Colormap                m_hColormap;
    std::vector<Color>      m_aPalette;         // Pseudocolor
    SalVisual               m_aVisual;
    OwnershipFlag           m_aVisualOwnership;
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

    SalColormap(SalColormap &&) = default;
    SalColormap & operator =(SalColormap &&) = default;

    Colormap            GetXColormap() const { return m_hColormap; }
    const SalDisplay*   GetDisplay() const { return m_pDisplay; }
    inline  Display*            GetXDisplay() const;
    const SalVisual&    GetVisual() const { return m_aVisual; }
    Visual*             GetXVisual() const { return m_aVisual.GetVisual(); }
    Pixel               GetWhitePixel() const { return m_nWhitePixel; }
    Pixel               GetBlackPixel() const { return m_nBlackPixel; }
    Pixel               GetUsed() const { return m_nUsed; }

    bool            GetXPixels( XColor  &rColor,
                                    int      r,
                                    int      g,
                                    int      b ) const;
    inline  bool            GetXPixel( XColor  &rColor,
                                           int      r,
                                           int      g,
                                           int      b ) const;
    Pixel           GetPixel( Color nColor ) const;
    Color           GetColor( Pixel nPixel ) const;
};

class SalI18N_InputMethod;

typedef int(*YieldFunc)(int fd, void* data);

class VCLPLUG_GEN_PUBLIC SalXLib
{
protected:
    timeval         m_aTimeout;
    sal_uLong       m_nTimeoutMS;
    int             m_pTimeoutFDS[2];

    int             nFDs_;
    fd_set          aReadFDS_;
    fd_set          aExceptionFDS_;

    Display             *m_pDisplay;
    std::unique_ptr<SalI18N_InputMethod> m_pInputMethod;

public:
    SalXLib();
    virtual         ~SalXLib();
    virtual void    Init();

    virtual bool    Yield( bool bWait, bool bHandleAllCurrentEvents );
    virtual void    Wakeup();
    void            TriggerUserEventProcessing();

    virtual void    Insert( int fd, void* data,
                            YieldFunc   pending,
                            YieldFunc   queued,
                            YieldFunc   handle );
    virtual void    Remove( int fd );

    virtual void    StartTimer( sal_uInt64 nMS );
    virtual void    StopTimer();

    virtual bool    CheckTimeout( bool bExecuteTimers = true );

    SalI18N_InputMethod* GetInputMethod() const { return m_pInputMethod.get(); }
    Display*             GetDisplay() const { return m_pDisplay; }
};

class SalI18N_KeyboardExtension;
class AttributeProvider;

extern "C" {
    typedef Bool(*X_if_predicate)(Display*,XEvent*,XPointer);
}

class VCLPLUG_GEN_PUBLIC GLX11Window : public GLWindow
{
public:
    Display*           dpy;
    int                screen;
    Window             win;
    XVisualInfo*       vi;
    GLXContext         ctx;
    OString            GLXExtensions;

    bool HasGLXExtension(const char* name) const;

    GLX11Window();
    virtual bool Synchronize(bool bOnoff) const override;
    virtual ~GLX11Window() override;
};

class VCLPLUG_GEN_PUBLIC SalDisplay : public SalGenericDisplay
{
public:
    struct RenderEntry
    {
        Pixmap      m_aPixmap;
        Picture     m_aPicture;

        RenderEntry() : m_aPixmap( 0 ), m_aPicture( 0 ) {}
    };

    typedef std::unordered_map<int,RenderEntry> RenderEntryMap;

    struct ScreenData
    {
        bool                m_bInit;

        ::Window            m_aRoot;
        ::Window            m_aRefWindow;
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
    SalI18N_KeyboardExtension   *mpKbdExtension;

    Display        *pDisp_;             // X Display

    SalX11Screen                 m_nXDefaultScreen;
    std::vector< ScreenData >    m_aScreens;
    ScreenData const      m_aInvalidScreenData;
    Pair            aResolution_;       // [dpi]
    sal_uLong       nMaxRequestSize_;   // [byte]

    srv_vendor_t    meServerVendor;

    // until x bytes

    o3tl::enumarray<PointerStyle, Cursor> aPointerCache_;

    // Keyboard
    bool            bNumLockFromXS_;    // Num Lock handled by X Server
    int             nNumLockIndex_;     // modifier index in modmap
    KeySym          nShiftKeySym_;      // first shift modifier
    KeySym          nCtrlKeySym_;       // first control modifier
    KeySym          nMod1KeySym_;       // first mod1 modifier

    std::unique_ptr<vcl_sal::WMAdaptor> m_pWMAdaptor;

    bool            m_bXinerama;
    std::vector< tools::Rectangle > m_aXineramaScreens;
    std::vector< int > m_aXineramaScreenIndexMap;
    std::list<SalObject*> m_aSalObjects;

    bool            m_bUseRandRWrapper; // don't use randr on gtk, use gdk signals there

    mutable Time    m_nLastUserEventTime; // mutable because changed on first access

    virtual bool    Dispatch( XEvent *pEvent ) = 0;
    void            InitXinerama();
    void            InitRandR( ::Window aRoot ) const;
    void            DeInitRandR();
    void            processRandREvent( XEvent* );

    void            doDestruct();
    void            addXineramaScreenUnique( int i, long i_nX, long i_nY, long i_nWidth, long i_nHeight );
    Time            GetEventTimeImpl( bool bAlwaysReget = false ) const;
public:
    static bool BestOpenGLVisual(Display* pDisplay, int nScreen, XVisualInfo& rVI);
    static bool BestVisual(Display *pDisp, int nScreen, XVisualInfo &rVI);

                    SalDisplay( Display* pDisp );

    virtual        ~SalDisplay() override;

    void            Init();

#ifdef DBG_UTIL
    void            PrintInfo() const;
    void            DbgPrintDisplayEvent(const char *pComment, XEvent *pEvent) const;
#endif

    void            Beep() const;

    void            ModifierMapping();
    void            SimulateKeyPress( sal_uInt16 nKeyCode );
    KeyIndicatorState  GetIndicatorState() const;
    OUString        GetKeyNameFromKeySym( KeySym keysym ) const;
    OUString        GetKeyName( sal_uInt16 nKeyCode ) const;
    sal_uInt16      GetKeyCode( KeySym keysym, char*pcPrintable ) const;
    KeySym          GetKeySym( XKeyEvent      *pEvent,
                               char           *pPrintable,
                               int            *pLen,
                               KeySym         *pUnmodifiedKeySym,
                               Status         *pStatus,
                               XIC = nullptr ) const;

    Cursor                GetPointer( PointerStyle ePointerStyle );
    virtual int           CaptureMouse( SalFrame *pCapture );

    virtual ScreenData   *initScreen( SalX11Screen nXScreen ) const;
    const ScreenData&     getDataForScreen( SalX11Screen nXScreen ) const
    {
        if( nXScreen.getXScreen() >= m_aScreens.size() )
            return m_aInvalidScreenData;
        if( ! m_aScreens[nXScreen.getXScreen()].m_bInit )
            initScreen( nXScreen );
        return m_aScreens[nXScreen.getXScreen()];
    }

    ::Window         GetDrawable( SalX11Screen nXScreen ) const { return getDataForScreen( nXScreen ).m_aRefWindow; }
    Display        *GetDisplay() const { return pDisp_; }
    const SalX11Screen& GetDefaultXScreen() const { return m_nXDefaultScreen; }
    const Size&     GetScreenSize( SalX11Screen nXScreen ) const { return getDataForScreen( nXScreen ).m_aSize; }
    srv_vendor_t    GetServerVendor() const { return meServerVendor; }
    bool            IsDisplay() const { return !!pXLib_; }
    GC              GetCopyGC( SalX11Screen nXScreen ) const { return getDataForScreen(nXScreen).m_aCopyGC; }
    Pixmap          GetInvert50( SalX11Screen nXScreen ) const { return getDataForScreen(nXScreen).m_hInvert50; }
    const SalColormap&    GetColormap( SalX11Screen nXScreen ) const { return getDataForScreen(nXScreen).m_aColormap; }
    const SalVisual&      GetVisual( SalX11Screen nXScreen ) const { return getDataForScreen(nXScreen).m_aVisual; }
    RenderEntryMap&       GetRenderEntries( SalX11Screen nXScreen ) const { return getDataForScreen(nXScreen).m_aRenderData; }
    const Pair     &GetResolution() const { return aResolution_; }
    sal_uLong       GetMaxRequestSize() const { return nMaxRequestSize_; }
    Time            GetLastUserEventTime() const { return GetEventTimeImpl(); }
    // this is an equivalent of gdk_x11_get_server_time()
    Time            GetX11ServerTime() const { return GetEventTimeImpl( true ); }

    bool            XIfEventWithTimeout( XEvent*, XPointer, X_if_predicate ) const;
    SalXLib*        GetXLib() const { return pXLib_; }

    SalI18N_InputMethod*        GetInputMethod()  const { return pXLib_->GetInputMethod();  }
    SalI18N_KeyboardExtension*  GetKbdExtension() const { return mpKbdExtension; }
    void            SetKbdExtension(SalI18N_KeyboardExtension *pKbdExtension)
    { mpKbdExtension = pKbdExtension; }
    ::vcl_sal::WMAdaptor* getWMAdaptor() const { return m_pWMAdaptor.get(); }
    bool            IsXinerama() const { return m_bXinerama; }
    const std::vector< tools::Rectangle >& GetXineramaScreens() const { return m_aXineramaScreens; }
    ::Window        GetRootWindow( SalX11Screen nXScreen ) const
            { return getDataForScreen( nXScreen ).m_aRoot; }
    unsigned int GetXScreenCount() const { return m_aScreens.size(); }

    const SalFrameSet& getFrames() const { return m_aFrames; }
    bool            IsNumLockFromXS() const { return bNumLockFromXS_; }

    std::list< SalObject* >& getSalObjects() { return m_aSalObjects; }
};

inline  Display *SalColormap::GetXDisplay() const
{ return m_pDisplay->GetDisplay(); }

class VCLPLUG_GEN_PUBLIC SalX11Display : public SalDisplay
{
public:
             SalX11Display( Display* pDisp );
    virtual ~SalX11Display() override;

    virtual bool        Dispatch( XEvent *pEvent ) override;
    virtual void        Yield();
    virtual void        TriggerUserEventProcessing() override;

    bool                IsEvent();
    void                SetupInput();
};

namespace vcl_sal {
    // get foreign key names
    OUString getKeysymReplacementName(
        const OUString& pLang,
        KeySym nSymbol );

    inline SalDisplay *getSalDisplay(GenericUnixSalData const * data)
    {
        assert(data != nullptr);
        assert(data->GetType() != SAL_DATA_GTK3);
        return static_cast<SalDisplay *>(data->GetDisplay());
    }
}

#endif // INCLUDED_VCL_INC_UNX_SALDISP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
