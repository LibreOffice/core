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

#include <unx/salunx.h>
#include <unx/saltype.h>
#include <vcl/salgtype.hxx>
#include <vcl/ptrstyle.hxx>
#include <sal/types.h>
#include <osl/mutex.h>
#include <list>
#include <unordered_map>
#include <vector>
#include <tools/gen.hxx>
#include <salwtype.hxx>
#include <generic/gendata.hxx>
#include <generic/gendisp.hxx>
#include <o3tl/enumarray.hxx>

#include <vclpluginapi.h>

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

// MSB/Bigendian view (SalColor == RGB, r=0xFF0000, g=0xFF00, b=0xFF)

enum SalRGB { RGB,  RBG,
              GBR,  GRB,
              BGR,  BRG,
              RGBA, RBGA,
              GBRA, GRBA,
              BGRA, BRGA,
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
                            ~SalVisual();
                            SalVisual( const XVisualInfo* pXVI );

    inline  VisualID        GetVisualId() const { return visualid; }
    inline  Visual         *GetVisual() const { return visual; }
    inline  int             GetClass() const { return c_class; }
    inline  int             GetDepth() const { return depth; }

            Pixel           GetTCPixel( SalColor nColor ) const;
            SalColor        GetTCColor( Pixel nPixel ) const;
};

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

    bool            GetXPixels( XColor  &rColor,
                                    int      r,
                                    int      g,
                                    int      b ) const;
    inline  bool            GetXPixel( XColor  &rColor,
                                           int      r,
                                           int      g,
                                           int      b ) const;
    Pixel           GetPixel( SalColor nColor ) const;
    SalColor        GetColor( Pixel nPixel ) const;
};

typedef int(*YieldFunc)(int fd, void* data);

class VCLPLUG_GEN_PUBLIC SalXLib
{
protected:
    timeval         m_aTimeout;
    sal_uLong       m_nTimeoutMS;
    int             m_pTimeoutFDS[2];
    bool            blockIdleTimeout;

    int             nFDs_;
    fd_set          aReadFDS_;
    fd_set          aExceptionFDS_;

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
    SalI18N_InputMethod         *mpInputMethod;
    SalI18N_KeyboardExtension   *mpKbdExtension;

    AttributeProvider           *mpFactory;

    Display        *pDisp_;             // X Display

    SalX11Screen                 m_nXDefaultScreen;
    std::vector< ScreenData >    m_aScreens;
    ScreenData      m_aInvalidScreenData;
    Pair            aResolution_;       // [dpi]
    sal_uLong       nMaxRequestSize_;   // [byte]

    srv_vendor_t    meServerVendor;

    // until x bytes

    o3tl::enumarray<PointerStyle, Cursor> aPointerCache_;

    // Keyboard
    bool            bNumLockFromXS_;    // Num Lock handled by X Server
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

    mutable Time    m_nLastUserEventTime; // mutable because changed on first access

    virtual bool    Dispatch( XEvent *pEvent ) = 0;
    void            InitXinerama();
    void            InitRandR( ::Window aRoot ) const;
    void            DeInitRandR();
    int             processRandREvent( XEvent* );

    void            doDestruct();
    void            addXineramaScreenUnique( int i, long i_nX, long i_nY, long i_nWidth, long i_nHeight );
public:
    static bool BestVisual( Display     *pDisp,
                                int          nScreen,
                                XVisualInfo &rVI );

                    SalDisplay( Display* pDisp );

    virtual        ~SalDisplay();

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
                               unsigned char  *pPrintable,
                               int            *pLen,
                               KeySym         *pUnmodifiedKeySym,
                               Status         *pStatus,
                               XIC = NULL ) const;

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
    SalX11Screen    GetDefaultXScreen() const { return m_nXDefaultScreen; }
    const Size&     GetScreenSize( SalX11Screen nXScreen ) const { return getDataForScreen( nXScreen ).m_aSize; }
    srv_vendor_t    GetServerVendor() const { return meServerVendor; }
    void            SetServerVendor() { meServerVendor = sal_GetServerVendor(pDisp_); }
    bool            IsDisplay() const { return !!pXLib_; }
    GC              GetMonoGC( SalX11Screen nXScreen ) const { return getDataForScreen(nXScreen).m_aMonoGC; }
    GC              GetCopyGC( SalX11Screen nXScreen ) const { return getDataForScreen(nXScreen).m_aCopyGC; }
    Pixmap          GetInvert50( SalX11Screen nXScreen ) const { return getDataForScreen(nXScreen).m_hInvert50; }
    const SalColormap&    GetColormap( SalX11Screen nXScreen ) const { return getDataForScreen(nXScreen).m_aColormap; }
    const SalVisual&      GetVisual( SalX11Screen nXScreen ) const { return getDataForScreen(nXScreen).m_aVisual; }
    RenderEntryMap&       GetRenderEntries( SalX11Screen nXScreen ) const { return getDataForScreen(nXScreen).m_aRenderData; }
    const Pair     &GetResolution() const { return aResolution_; }
    sal_uLong       GetMaxRequestSize() const { return nMaxRequestSize_; }
    Time            GetLastUserEventTime( bool bAlwaysReget = false ) const;

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
    ::Window        GetRootWindow( SalX11Screen nXScreen ) const
            { return getDataForScreen( nXScreen ).m_aRoot; }
    unsigned int GetXScreenCount() const { return m_aScreens.size(); }

    const std::list< SalFrame* >& getFrames() const { return m_aFrames; }
    bool            IsNumLockFromXS() const { return bNumLockFromXS_; }

    std::list< SalObject* >& getSalObjects() { return m_aSalObjects; }

    virtual void    PostUserEvent() SAL_OVERRIDE = 0;
};

inline  Display *SalColormap::GetXDisplay() const
{ return m_pDisplay->GetDisplay(); }

class VCLPLUG_GEN_PUBLIC SalX11Display : public SalDisplay
{
public:
             SalX11Display( Display* pDisp );
    virtual ~SalX11Display();

    virtual bool        Dispatch( XEvent *pEvent ) SAL_OVERRIDE;
    virtual void        Yield();
    virtual void        PostUserEvent() SAL_OVERRIDE;

    bool                IsEvent();
    void                SetupInput( SalI18N_InputMethod *pInputMethod );
};

namespace vcl_sal {
    // get foreign key names
    OUString getKeysymReplacementName(
        const OUString& pLang,
        KeySym nSymbol );

    inline SalDisplay *getSalDisplay(SalGenericData const * data)
    {
        assert(data != nullptr);
        assert(data->GetType() != SAL_DATA_GTK3);
        return static_cast<SalDisplay *>(data->GetDisplay());
    }
}

#endif // INCLUDED_VCL_INC_UNX_SALDISP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
