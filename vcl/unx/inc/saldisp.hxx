/*************************************************************************
 *
 *  $RCSfile: saldisp.hxx,v $
 *
 *  $Revision: 1.24 $
 *
 *  last change: $Author: rt $ $Date: 2004-06-17 12:26:03 $
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

#ifndef _SV_SALDISP_HXX
#define _SV_SALDISP_HXX

// -=-= exports =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class   SalDisplay;
class   SalColormap;
class   SalVisual;
class   SalXLib;

// -=-= #includes =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#ifndef _SALUNX_H
#include <salunx.h>
#endif
#ifndef _SV_SALGTYPE_HXX
#include <salgtype.hxx>
#endif
#ifndef _SV_PTRSTYLE_HXX
#include <ptrstyle.hxx>
#endif
#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif
#ifndef _OSL_MUTEX_H
#include <osl/mutex.h>
#endif
#include <vector>
#include <list>
#ifndef _GEN_HXX
#include <tools/gen.hxx>
#endif
#ifndef _SV_SALWTYPE_HXX
#include <salwtype.hxx>
#endif

// -=-= forwards -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class   BitmapPalette;
class   SalFrame;
class   ColorMask;

namespace vcl_sal { class WMAdaptor; }

// -=-= #defines -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#define PROPERTY_SUPPORT_WM_SetPos              0x00000001
#define PROPERTY_SUPPORT_WM_Screen              0x00000002
#define PROPERTY_SUPPORT_WM_Parent_Pixmap_None  0x00000004
#define PROPERTY_SUPPORT_WM_ClientPos           0x00000008
#define PROPERTY_SUPPORT_XSetClipMask           0x00000010  // for bitmap ops.
#define PROPERTY_SUPPORT_3ButtonMouse           0x00000020

#define PROPERTY_BUG_XA_FAMILY_NAME_nil         0x00001000
#define PROPERTY_BUG_XCopyArea_GXxor            0x00002000  // from window
#define PROPERTY_BUG_Stipple                    0x00004000  // 0/1 inverted
#define PROPERTY_BUG_Tile                       0x00008000  // Recreate the
                                            // dither brush each time
#define PROPERTY_BUG_FillPolygon_Tile           0x00010000  // always Toggle Fillstyle
#define PROPERTY_BUG_DrawLine                   0x00020000  // a DrawLine is one point to short
#define PROPERTY_BUG_CopyPlane_RevertBWPixel    0x00040000  // revert fg and bg for xcopyplane
#define PROPERTY_BUG_CopyArea_OnlySmallSlices   0x00080000
#define PROPERTY_BUG_Bitmap_Bit_Order           0x00100000

#define PROPERTY_FEATURE_Maximize               0x01000000
#define PROPERTY_FEATURE_SharedMemory           0x02000000
#define PROPERTY_FEATURE_TrustedSolaris         0x04000000

#define PROPERTY_DEFAULT                        0x00000FCB

// ------------------------------------------------------------------------
// server vendor

typedef enum  {
    vendor_none = 0,
    vendor_attachmate,
    vendor_excursion,
    vendor_hp,
    vendor_hummingbird,
    vendor_ibm,
    vendor_sco,
    vendor_sgi,
    vendor_sun,
    vendor_xfree,
    vendor_xinside,
    vendor_xprinter,
    vendor_unknown
} srv_vendor_t;

extern "C" srv_vendor_t sal_GetServerVendor( Display *p_display );

// -=-= SalWM =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
enum SalWM { olwm,      // Open Look
             mwm,       // Motif
             kwm,       // KDE Desktop Environment
             FourDwm,   // SGI
             vuewm,     // HP
             dtwm,      // CDE
             winmgr,    // Oracle NC
             twm,
             fvwm,      // ...
             pmwm,      // SCO
             otherwm };

// -=-= SalRGB -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// MSB/Bigendian Sicht (SalColor == RGB, r=0xFF0000, g=0xFF00, b=0xFF)

enum SalRGB { RGB,  RBG,
              GBR,  GRB,
              BGR,  BRG,
              RGBA, RBGA,
              GBRA, GRBA,
              BGRA, BRGA,
              other };

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
                            ~SalVisual();
                            SalVisual( const XVisualInfo* pXVI );

    inline  VisualID        GetVisualId() const { return visualid; }
    inline  Visual         *GetVisual() const { return visual; }
    inline  int             GetClass() const { return c_class; }
    inline  int             GetDepth() const { return depth; }
    inline  SalRGB          GetMode() const { return eRGBMode_; }

            Pixel           GetTCPixel( SalColor nColor ) const;
            SalColor        GetTCColor( Pixel nPixel ) const;
            BOOL            Convert( int &n0, int &n1, int &n2, int &n3 ); // 32bit
             BOOL           Convert( int &n0, int &n1, int &n2 ); // 24bit
};

// -=-= SalColormap =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class SalColormap
{
    SalDisplay     *pDisplay_;
    Colormap        hColormap_;
    SalColor       *pPalette_;          // Pseudocolor
    SalVisual      *pVisual_;
    USHORT         *pLookupTable_;      // Pseudocolor: 12bit reduction
    Pixel           nWhitePixel_;
    Pixel           nBlackPixel_;
    Pixel           nUsed_;             // Pseudocolor

    void            GetPalette();
    void            GetLookupTable();
public:
    SalColormap( SalDisplay          *pSalDisplay,
                 Colormap             hColormap );
    SalColormap( const BitmapPalette &rpPalette );
    SalColormap( USHORT               nDepth );
    SalColormap();
    ~SalColormap();

    inline  Colormap        GetXColormap() const { return hColormap_; }
    inline  SalDisplay     *GetDisplay() const { return pDisplay_; }
    inline  Display        *GetXDisplay() const;
    inline  SalVisual      *GetVisual() const;
    inline  Visual         *GetXVisual() const;
    inline  Pixel           GetWhitePixel() const { return nWhitePixel_; }
    inline  Pixel           GetBlackPixel() const { return nBlackPixel_; }
    inline  Pixel           GetUsed() const { return nUsed_; }
    inline  int             GetClass() const;

    BOOL            GetXPixels( XColor  &rColor,
                                int      r,
                                int      g,
                                int      b ) const;
    inline  BOOL            GetXPixel( XColor  &rColor,
                                       int      r,
                                       int      g,
                                       int      b ) const;
    Pixel           GetPixel( SalColor nColor ) const;
    SalColor        GetColor( Pixel nPixel ) const;
    void            SetPalette( const BitmapPalette &rPalette );
};

// -=-= SalXLib =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
typedef int(*YieldFunc)(int fd, void* data);
struct YieldEntry;

class SalXLib
{
protected:
    timeval         Timeout_;
    ULONG           nTimeoutMS_;
    int             pTimeoutFDS_[2];

    BOOL            bWasXError_;
    BOOL            bIgnoreXErrors_;
    bool            m_bHaveSystemChildFrames;
    int                         nIgnoreErrorLevel;

    int             nFDs_;
    fd_set          aReadFDS_;
    fd_set          aExceptionFDS_;
    YieldEntry      *pYieldEntries_;

public:
    SalXLib();
    virtual         ~SalXLib();
    virtual void        Init();

    virtual void        Yield( BOOL bWait );
    virtual void        Wakeup();
    virtual void        PostUserEvent();

    virtual void    Insert( int fd, void* data,
                            YieldFunc   pending,
                            YieldFunc   queued,
                            YieldFunc   handle );
    virtual void    Remove( int fd );

    void            XError( Display *pDisp, XErrorEvent *pEvent );
    inline  void                    PushErrorTrap() { nIgnoreErrorLevel++; }
    inline  void                    PopErrorTrap() { nIgnoreErrorLevel--; }
    inline  BOOL            WasXError() const { return bWasXError_; }
    inline  BOOL            GetIgnoreXErrors() const { return bIgnoreXErrors_; }
    inline  void            SetIgnoreXErrors( BOOL b )
    { bIgnoreXErrors_ = b; bWasXError_ = FALSE; }

    virtual void            StartTimer( ULONG nMS );
    virtual void            StopTimer();

    bool            CheckTimeout( bool bExecuteTimers = true );

    void            setHaveSystemChildFrame()
    { m_bHaveSystemChildFrames = true; }
};

// -=-= SalDisplay -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

class SalI18N_InputMethod;
class SalI18N_KeyboardExtension;
class XlfdStorage;
class ExtendedFontStruct;
class ExtendedXlfd;
class AttributeProvider;
class SalUnicodeConverter;
class SalConverterCache;

DECLARE_LIST( SalFontCache, ExtendedFontStruct* )

extern "C" {
    struct SnDisplay;
    struct SnLauncheeContext;
};

class SalDisplay
{
// -=-= UserEvent =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    struct SalUserEvent
    {
        SalFrame*       m_pFrame;
        void*           m_pData;
        USHORT          m_nEvent;

        SalUserEvent( SalFrame* pFrame, void* pData, USHORT nEvent = SALEVENT_USEREVENT )
                : m_pFrame( pFrame ),
                  m_pData( pData ),
                  m_nEvent( nEvent )
        {}
    };

protected:
    SalXLib        *pXLib_;
    SalI18N_InputMethod         *mpInputMethod;
    SalI18N_KeyboardExtension   *mpKbdExtension;

    AttributeProvider           *mpFactory;
    XlfdStorage                 *mpFontList;
    const ExtendedXlfd          *mpFallbackFactory;

    Display        *pDisp_;             // X Display
    Screen         *pScreen_;           // XDefaultScreenOfDisplay
    int             nScreen_;           // XDefaultScreen
    SalVisual      *pRootVisual_;       // default visual of screen
    XLIB_Window     hRootWindow_;
    Size            aSize_;             // Screen Size [pixel]
    Pair            aResolution_;       // [dpi]
    bool            mbExactResolution;
    ULONG           nMaxRequestSize_;   // [byte]

    srv_vendor_t    meServerVendor;
    SalWM           eWindowManager_;
    ULONG           nProperties_;       // PROPERTY_SUPPORT, BUG, FEATURE
    BOOL            bLocal_;            // Server==Client? Init
    // in SalDisplay::IsLocal()
    BOOL            mbLocalIsValid;     // bLocal_ is valid ?
    // until x bytes

    oslMutex        hEventGuard_;
    std::list< SalUserEvent > m_aUserEvents;

    XLIB_Cursor     aPointerCache_[POINTER_COUNT];
    SalFrame*       m_pCapture;

    // GDI
    SalVisual      *pVisual_;           // Visual
    XLIB_Window     hRefWindow_;
    GC              pMonoGC_;
    GC              pCopyGC_;
    GC              pAndInvertedGC_;
    GC              pAndGC_;
    GC              pOrGC_;
    GC              pStippleGC_;
    Pixmap          hInvert50_;
    SalColormap    *m_pColormap;

    SalFontCache   *pFontCache_;

    // Keyboard
    BOOL            bNumLockFromXS_;    // Num Lock handled by X Server
    int             nNumLockIndex_;     // modifier index in modmap
    int             nNumLockMask_;      // keyevent state mask for
    KeySym          nShiftKeySym_;      // first shift modifier
    KeySym          nCtrlKeySym_;       // first control modifier
    KeySym          nMod1KeySym_;       // first mod1 modifier
    ByteString      m_aKeyboardName;

    vcl_sal::WMAdaptor* m_pWMAdaptor;

    bool            m_bXinerama;
    std::vector< Rectangle > m_aXineramaScreens;
    std::list<SalFrame*> m_aFrames;

    struct SnDisplay           *m_pSnDisplay;
    struct SnLauncheeContext   *m_pSnLauncheeContext;

    void            DestroyFontCache();
    virtual long    Dispatch( XEvent *pEvent ) = 0;
    void            InitXinerama();

    void            doDestruct();
public:
    static  SalDisplay     *GetSalDisplay( Display* display );
    static  BOOL            BestVisual( Display     *pDisp,
                                        int          nScreen,
                                        XVisualInfo &rVI );

                            SalDisplay( Display* pDisp,
                                        Colormap aColMap = None );

    virtual ~SalDisplay();


    virtual void            registerFrame( SalFrame* pFrame );
    virtual void            deregisterFrame( SalFrame* pFrame );
    void                    setHaveSystemChildFrame() const
    { pXLib_->setHaveSystemChildFrame(); }

    void            Init( Colormap hXColmap, Visual *pVisual );

    void            SendInternalEvent( SalFrame* pFrame, void* pData, USHORT nEvent = SALEVENT_USEREVENT );
    bool            DispatchInternalEvent();
    void            PrintInfo() const;

    void            PrintEvent( const ByteString &rComment,
                                XEvent       *pEvent ) const;

    void            AddFontPath( const ByteString &rPath ) const;
    XlfdStorage*    GetXlfdList();
    ExtendedFontStruct*
    GetFont( const ExtendedXlfd *pFont,
             const Size& rPixelSize, sal_Bool bVertical );
    const ExtendedXlfd*
    GetFallbackFactory()
    { return mpFallbackFactory; }

    void            Beep() const;

    void            ModifierMapping();
    String          GetKeyNameFromKeySym( KeySym keysym ) const;
    XubString       GetKeyName( USHORT nKeyCode ) const;
    USHORT          GetKeyCode( KeySym keysym, char*pcPrintable ) const;
    KeySym          GetKeySym( XKeyEvent      *pEvent,
                               unsigned char  *pPrintable,
                               int            *pLen,
                               Status         *pStatus,
                               XIC = NULL ) const;

    XLIB_Cursor     GetPointer( int ePointerStyle );
    virtual int CaptureMouse( SalFrame *pCapture );

    BOOL            IsLocal();

    void            Remove( XEvent   *pEvent );

    XLIB_Window     GetRootWindow() const { return hRootWindow_; }
    XLIB_Window     GetDrawable() const { return hRefWindow_; }
    Display        *GetDisplay() const { return pDisp_; }
    int             GetScreenNumber() const { return nScreen_; }
    srv_vendor_t    GetServerVendor() const { return meServerVendor; }
    void            SetServerVendor() { meServerVendor = sal_GetServerVendor(pDisp_); }
    BOOL            IsDisplay() const { return !!pXLib_; }
    GC              GetMonoGC() const { return pMonoGC_; }
    GC              GetCopyGC() const { return pCopyGC_; }
    GC              GetAndInvertedGC() const { return pAndInvertedGC_; }
    GC              GetAndGC() const { return pAndGC_; }
    GC              GetOrGC() const { return pOrGC_; }
    GC              GetStippleGC() const { return pStippleGC_; }
    GC              GetGC( USHORT nDepth ) const;
    Pixmap          GetInvert50() const { return hInvert50_; }
    SalColormap    &GetColormap() const { return *m_pColormap; }
    SalVisual      *GetVisual() const { return pVisual_; }
    SalVisual      *GetRootVisual() const { return pRootVisual_; }
    const Size     &GetScreenSize() const { return aSize_; }
    const Pair     &GetResolution() const { return aResolution_; }
    bool            GetExactResolution() const { return mbExactResolution; }
    ULONG           GetProperties() const { return nProperties_; }
    ULONG           GetMaxRequestSize() const { return nMaxRequestSize_; }
    void            GetScreenFontResolution( sal_Int32& rDPIX, sal_Int32& rDPIY ) const;

    BOOL            MouseCaptured( const SalFrame *pFrameData ) const
    { return m_pCapture == pFrameData; }
    SalFrame*   GetCaptureFrame() const
    { return m_pCapture; }
    SalXLib*         GetXLib() { return pXLib_; }

    SalI18N_InputMethod*            GetInputMethod()  { return mpInputMethod;  }
    SalI18N_KeyboardExtension*  GetKbdExtension() { return mpKbdExtension; }
    void            SetInputMethod( SalI18N_InputMethod *pInputMethod )
    { mpInputMethod = pInputMethod; }
    void            SetKbdExtension(SalI18N_KeyboardExtension *pKbdExtension)
    { mpKbdExtension = pKbdExtension; }
    const char* GetKeyboardName( BOOL bRefresh = FALSE );
    ::vcl_sal::WMAdaptor* getWMAdaptor() const { return m_pWMAdaptor; }
    bool            IsXinerama() const { return m_bXinerama; }
    const std::vector< Rectangle >& GetXineramaScreens() const { return m_aXineramaScreens; }

    const std::list< SalFrame* >& getFrames()
    { return m_aFrames; }

};

// -=-= inlines =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

inline GC SalDisplay::GetGC( USHORT nDepth ) const
{ return 1 == nDepth
         ? pMonoGC_
         : pVisual_->GetDepth() == nDepth
           ? pCopyGC_
           : NULL; }

inline  Display *SalColormap::GetXDisplay() const
{ return pDisplay_->GetDisplay(); }

inline SalVisual *SalColormap::GetVisual() const
{ return pVisual_ ? pVisual_ : pDisplay_->GetVisual(); }

inline Visual *SalColormap::GetXVisual() const
{ return GetVisual()->GetVisual(); }

inline int SalColormap::GetClass() const
{ return pVisual_ ? pVisual_->GetClass() : PseudoColor; }

class SalX11Display : public SalDisplay
{
public:
             SalX11Display( Display* pDisp,
                         Visual* pVisual = NULL,
                         Colormap aColMap = None );
    virtual ~SalX11Display();

    virtual long        Dispatch( XEvent *pEvent );
    virtual void        Yield( BOOL bWait );

    BOOL     IsEvent();
};

/*----------------------------------------------------------
 keep track of correct size of the initial window
 */
// get foreign key names
namespace vcl_sal {
    String getKeysymReplacementName(
        const char* pKeyboard,
        KeySym nSymbol );
}


#endif // _SV_SALDISP_HXX

