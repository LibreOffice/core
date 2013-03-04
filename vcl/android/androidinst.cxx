/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 *   Copyright (C) 2012 Novell, Inc.
 *   Michael Meeks <michael.meeks@suse.com> (initial developer)
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */
#include <android/androidinst.hxx>
#include <headless/svpdummies.hxx>
#include <generic/gendata.hxx>
#include <jni.h>
#include <android/log.h>
#include <android/looper.h>
#include <android/bitmap.h>
#include <osl/detail/android-bootstrap.h>
#include <rtl/strbuf.hxx>
#include <basebmp/scanlineformats.hxx>

#define LOGTAG "LibreOffice/androidinst"
#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, LOGTAG, __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, LOGTAG, __VA_ARGS__))
#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, LOGTAG, __VA_ARGS__))

static bool bHitIdle = false;

// Horrible hack
static int viewWidth = 1, viewHeight = 1;

class AndroidSalData : public SalGenericData
{
public:
    AndroidSalData( SalInstance *pInstance ) : SalGenericData( SAL_DATA_ANDROID, pInstance ) {}
    virtual void ErrorTrapPush() {}
    virtual bool ErrorTrapPop( bool ) { return false; }
};

#if 0  // Code from NativeActivity-based times left for reference inside #if 0 here and later

static rtl::OString MotionEdgeFlagsToString(int32_t nFlags)
{
    rtl::OStringBuffer aStr;
    if (nFlags == AMOTION_EVENT_EDGE_FLAG_NONE)
        aStr.append ("no-edge");
    if (nFlags & AMOTION_EVENT_EDGE_FLAG_TOP)
        aStr.append (":top");
    if (nFlags & AMOTION_EVENT_EDGE_FLAG_BOTTOM)
        aStr.append (":bottom");
    if (nFlags & AMOTION_EVENT_EDGE_FLAG_LEFT)
        aStr.append (":left");
    if (nFlags & AMOTION_EVENT_EDGE_FLAG_RIGHT)
        aStr.append (":right");
    return aStr.makeStringAndClear();
}

static rtl::OString KeyMetaStateToString(int32_t nFlags)
{
    rtl::OStringBuffer aStr;
    if (nFlags == AMETA_NONE)
        aStr.append ("no-meta");
    if (nFlags & AMETA_ALT_ON)
        aStr.append (":alt");
    if (nFlags & AMETA_SHIFT_ON)
        aStr.append (":shift");
    if (nFlags & AMETA_SYM_ON)
        aStr.append (":sym");
    return aStr.makeStringAndClear();
}

static sal_uInt16 KeyMetaStateToCode(AInputEvent *event)
{
    sal_uInt16 nCode = 0;
    int32_t nFlags = AKeyEvent_getMetaState(event);
    if (nFlags & AMETA_SHIFT_ON)
        nCode |= KEY_SHIFT;
    if (nFlags & AMETA_SYM_ON)
        nCode |= KEY_MOD1;
    if (nFlags & AMETA_ALT_ON)
        nCode |= KEY_MOD2;
    return nCode;
}

static sal_uInt16 KeyToCode(AInputEvent *event)
{
    sal_uInt16 nCode = 0;
    switch (AKeyEvent_getKeyCode(event)) {
#define MAP(a,b)                                 \
    case AKEYCODE_##a: nCode = KEY_##b; break
#define MAP_SAME(a) MAP(a,a)

    MAP_SAME(HOME);
    MAP_SAME(0); MAP_SAME(1); MAP_SAME(2); MAP_SAME(3); MAP_SAME(4);
    MAP_SAME(5); MAP_SAME(6); MAP_SAME(7); MAP_SAME(8); MAP_SAME(9);

    MAP_SAME(A); MAP_SAME(B); MAP_SAME(C); MAP_SAME(D);
    MAP_SAME(E); MAP_SAME(F); MAP_SAME(G); MAP_SAME(H);
    MAP_SAME(I); MAP_SAME(J); MAP_SAME(K); MAP_SAME(L);
    MAP_SAME(M); MAP_SAME(N); MAP_SAME(O); MAP_SAME(P);
    MAP_SAME(Q); MAP_SAME(R); MAP_SAME(S); MAP_SAME(T);
    MAP_SAME(U); MAP_SAME(V); MAP_SAME(W); MAP_SAME(X);
    MAP_SAME(Y); MAP_SAME(Z);

    MAP_SAME(TAB); MAP_SAME(SPACE); MAP_SAME(COMMA);

    MAP(ENTER,RETURN);
    MAP(PAGE_UP, PAGEUP);
    MAP(PAGE_DOWN, PAGEDOWN);
    MAP(DEL, DELETE);
    MAP(PERIOD, POINT);

    MAP(DPAD_UP, UP); MAP(DPAD_DOWN, DOWN);
    MAP(DPAD_LEFT, LEFT); MAP(DPAD_RIGHT, RIGHT);

    case AKEYCODE_BACK: // escape ?
    case AKEYCODE_UNKNOWN:
    case AKEYCODE_SOFT_LEFT:
    case AKEYCODE_SOFT_RIGHT:
    case AKEYCODE_CALL:
    case AKEYCODE_ENDCALL:
    case AKEYCODE_STAR:
    case AKEYCODE_POUND:
    case AKEYCODE_VOLUME_UP:
    case AKEYCODE_VOLUME_DOWN:
    case AKEYCODE_DPAD_CENTER:
    case AKEYCODE_POWER:
    case AKEYCODE_CAMERA:
    case AKEYCODE_CLEAR:
    case AKEYCODE_ALT_LEFT:
    case AKEYCODE_ALT_RIGHT:
    case AKEYCODE_SHIFT_LEFT:
    case AKEYCODE_SHIFT_RIGHT:
    case AKEYCODE_SYM:
    case AKEYCODE_EXPLORER:
    case AKEYCODE_ENVELOPE:
    case AKEYCODE_GRAVE:
    case AKEYCODE_MINUS:
    case AKEYCODE_EQUALS:
    case AKEYCODE_LEFT_BRACKET:
    case AKEYCODE_RIGHT_BRACKET:
    case AKEYCODE_BACKSLASH:
    case AKEYCODE_SEMICOLON:
    case AKEYCODE_APOSTROPHE:
    case AKEYCODE_SLASH:
    case AKEYCODE_AT:
    case AKEYCODE_NUM:
    case AKEYCODE_HEADSETHOOK:
    case AKEYCODE_FOCUS: // not widget, but camera focus
    case AKEYCODE_PLUS:
    case AKEYCODE_MENU:
    case AKEYCODE_NOTIFICATION:
    case AKEYCODE_SEARCH:
    case AKEYCODE_MEDIA_PLAY_PAUSE:
    case AKEYCODE_MEDIA_STOP:
    case AKEYCODE_MEDIA_NEXT:
    case AKEYCODE_MEDIA_PREVIOUS:
    case AKEYCODE_MEDIA_REWIND:
    case AKEYCODE_MEDIA_FAST_FORWARD:
    case AKEYCODE_MUTE:
    case AKEYCODE_PICTSYMBOLS:
    case AKEYCODE_SWITCH_CHARSET:
    case AKEYCODE_BUTTON_A:
    case AKEYCODE_BUTTON_B:
    case AKEYCODE_BUTTON_C:
    case AKEYCODE_BUTTON_X:
    case AKEYCODE_BUTTON_Y:
    case AKEYCODE_BUTTON_Z:
    case AKEYCODE_BUTTON_L1:
    case AKEYCODE_BUTTON_R1:
    case AKEYCODE_BUTTON_L2:
    case AKEYCODE_BUTTON_R2:
    case AKEYCODE_BUTTON_THUMBL:
    case AKEYCODE_BUTTON_THUMBR:
    case AKEYCODE_BUTTON_START:
    case AKEYCODE_BUTTON_SELECT:
    case AKEYCODE_BUTTON_MODE:
        LOGI("un-mapped keycode %d", nCode);
        nCode = 0;
        break;
#undef MAP_SAME
#undef MAP
    }
    LOGI("mapped %d -> %d", AKeyEvent_getKeyCode(event), nCode);
    return nCode;
}

#endif

static void BlitFrameRegionToWindow(ANativeWindow_Buffer *pOutBuffer,
                                    const basebmp::BitmapDeviceSharedPtr& aDev,
                                    const ARect &rSrcRect,
                                    int nDestX, int nDestY)
{
    basebmp::RawMemorySharedArray aSrcData = aDev->getBuffer();
    basegfx::B2IVector aDevSize = aDev->getSize();
    sal_Int32 nStride = aDev->getScanlineStride();
    unsigned char *pSrc = aSrcData.get();

    // FIXME: do some cropping goodness on aSrcRect to ensure no overflows etc.
    ARect aSrcRect = rSrcRect;

    for (unsigned int y = 0; y < (unsigned int)(aSrcRect.bottom - aSrcRect.top); y++)
    {
        unsigned char *sp = ( pSrc + nStride * (aSrcRect.top + y) +
                              aSrcRect.left * 4 /* src pixel size */ );

        switch (pOutBuffer->format) {
        case WINDOW_FORMAT_RGBA_8888:
        case WINDOW_FORMAT_RGBX_8888:
        {
            unsigned char *dp = ( (unsigned char *)pOutBuffer->bits +
                                  pOutBuffer->stride * 4 * (y + nDestY) +
                                  nDestX * 4 /* dest pixel size */ );
            for (unsigned int x = 0; x < (unsigned int)(aSrcRect.right - aSrcRect.left); x++)
            {
                dp[x*4 + 0] = sp[x*4 + 0]; // R
                dp[x*4 + 1] = sp[x*4 + 1]; // G
                dp[x*4 + 2] = sp[x*4 + 2]; // B
                dp[x*4 + 3] = 255; // A
            }
            break;
        }
        case WINDOW_FORMAT_RGB_565:
        {
            unsigned char *dp = ( (unsigned char *)pOutBuffer->bits +
                                  pOutBuffer->stride * 2 * (y + nDestY) +
                                  nDestX * 2 /* dest pixel size */ );
            for (unsigned int x = 0; x < (unsigned int)(aSrcRect.right - aSrcRect.left); x++)
            {
                unsigned char b = sp[x*3 + 0]; // B
                unsigned char g = sp[x*3 + 1]; // G
                unsigned char r = sp[x*3 + 2]; // R
                dp[x*2 + 0] = (r & 0xf8) | (g >> 5);
                dp[x*2 + 1] = ((g & 0x1c) << 5) | ((b & 0xf8) >> 3);
            }
            break;
        }
        default:
            LOGI("unknown pixel format %d !", pOutBuffer->format);
            break;
        }
    }
}

void AndroidSalInstance::BlitFrameToWindow(ANativeWindow_Buffer *pOutBuffer,
                                           const basebmp::BitmapDeviceSharedPtr& aDev)
{
    basegfx::B2IVector aDevSize = aDev->getSize();
    ARect aWhole = { 0, 0, aDevSize.getX(), aDevSize.getY() };
    BlitFrameRegionToWindow(pOutBuffer, aDev, aWhole, 0, 0);
}

void AndroidSalInstance::RedrawWindows(ANativeWindow *pWindow, ANativeWindow_Buffer *pBuffer)
{
    ANativeWindow_Buffer aOutBuffer;
    memset ((void *)&aOutBuffer, 0, sizeof (aOutBuffer));

    if (pBuffer != NULL)
        aOutBuffer = *pBuffer;
    else
    {
        if (!pWindow)
            return;

        //    ARect aRect;
        LOGI("pre lock #3");
        ANativeWindow_lock(pWindow, &aOutBuffer, NULL);
    }

    if (aOutBuffer.bits != NULL)
    {
        int i = 0;
        std::list< SalFrame* >::const_iterator it;
        for ( it = getFrames().begin(); it != getFrames().end(); i++, it++ )
        {
            SvpSalFrame *pFrame = static_cast<SvpSalFrame *>(*it);

            if (pFrame->IsVisible())
            {
                BlitFrameToWindow (&aOutBuffer, pFrame->getDevice());
            }
        }
    }
    else
        LOGI("no buffer for locked window");

    if (pBuffer && pWindow)
        ANativeWindow_unlockAndPost(pWindow);

    mbQueueReDraw = false;
}

void AndroidSalInstance::damaged(AndroidSalFrame */* frame */)
{
    mbQueueReDraw = true;
}

#if 0

static const char *app_cmd_name(int cmd)
{
    switch (cmd) {
    case APP_CMD_INPUT_CHANGED: return "INPUT_CHANGED";
    case APP_CMD_INIT_WINDOW: return "INIT_WINDOW";
    case APP_CMD_TERM_WINDOW: return "TERM_WINDOW";
    case APP_CMD_WINDOW_RESIZED: return "WINDOW_RESIZED";
    case APP_CMD_WINDOW_REDRAW_NEEDED: return "WINDOW_REDRAW_NEEDED";
    case APP_CMD_CONTENT_RECT_CHANGED: return "CONTENT_RECT_CHANGED";
    case APP_CMD_GAINED_FOCUS: return "GAINED_FOCUS";
    case APP_CMD_LOST_FOCUS: return "LOST_FOCUS";
    case APP_CMD_CONFIG_CHANGED: return "CONFIG_CHANGED";
    case APP_CMD_LOW_MEMORY: return "LOW_MEMORY";
    case APP_CMD_START: return "START";
    case APP_CMD_RESUME: return "RESUME";
    case APP_CMD_SAVE_STATE: return "SAVE_STATE";
    case APP_CMD_PAUSE: return "PAUSE";
    case APP_CMD_STOP: return "STOP";
    case APP_CMD_DESTROY: return "DESTROY";
    default:
        static char buf[10];
        sprintf(buf, "%d", cmd);
        return buf;
    }
}

#endif

void AndroidSalInstance::GetWorkArea( Rectangle& rRect )
{
    rRect = Rectangle( Point( 0, 0 ),
                       Size( 1280, 750 ) );

#if 0
    rRect = Rectangle( Point( 0, 0 ),
                       Size( ANativeWindow_getWidth( mpApp->window ),
                             ANativeWindow_getHeight( mpApp->window ) ) );
#endif
}

#if 0

void AndroidSalInstance::onAppCmd (struct android_app* app, int32_t cmd)
{
        LOGI("app cmd for app %p: %s", app, app_cmd_name(cmd));
        ANativeWindow *pWindow = mpApp->window;
        switch (cmd) {
        case APP_CMD_INIT_WINDOW:
        {
            ARect aRect = { 0, 0, 0, 0 };
            aRect.right = ANativeWindow_getWidth(pWindow);
            aRect.bottom = ANativeWindow_getHeight(pWindow);
            int nRet = ANativeWindow_setBuffersGeometry(
                                pWindow, ANativeWindow_getWidth(pWindow),
                                ANativeWindow_getHeight(pWindow),
                                WINDOW_FORMAT_RGBA_8888);
            LOGI("we have an app window ! %p %dx%x (%d) set %d",
                     pWindow, aRect.right, aRect.bottom,
                     ANativeWindow_getFormat(pWindow), nRet);
            maRedrawRegion = Region( Rectangle( 0, 0, ANativeWindow_getWidth(pWindow),
                                                ANativeWindow_getHeight(pWindow) ) );
            mbQueueReDraw = true;
            break;
        }
        case APP_CMD_WINDOW_RESIZED:
        {
            ARect aRect = { 0, 0, 0, 0 };
            aRect.right = ANativeWindow_getWidth(pWindow);
            aRect.bottom = ANativeWindow_getHeight(pWindow);
            LOGI("app window resized to ! %p %dx%x (%d)",
                 pWindow, aRect.right, aRect.bottom,
                 ANativeWindow_getFormat(pWindow));
            break;
        }

        case APP_CMD_WINDOW_REDRAW_NEEDED:
        {
            LOGI("redraw needed");
            maRedrawRegion = Region( Rectangle( 0, 0, ANativeWindow_getWidth(pWindow),
                                                ANativeWindow_getHeight(pWindow) ) );
            mbQueueReDraw = true;
            break;
        }

        case APP_CMD_CONTENT_RECT_CHANGED:
        {
            ARect aRect = mpApp->contentRect;
            LOGI("content rect changed [ k/b popped up etc. ] %d,%d->%d,%d",
                 aRect.left, aRect.top, aRect.right, aRect.bottom);
            break;
        }
        default:
            LOGI("unhandled app cmd %d", cmd);
            break;
        }
}

#endif

/*
 * Try too hard to get a frame, in the absence of anything better to do
 */
SalFrame *AndroidSalInstance::getFocusFrame() const
{
    SalFrame *pFocus = SvpSalFrame::GetFocusFrame();
    if (!pFocus) {
        LOGI("no focus frame, re-focusing first visible frame");
        const std::list< SalFrame* >& rFrames( getFrames() );
        for( std::list< SalFrame* >::const_iterator it = rFrames.begin(); it != rFrames.end(); ++it )
        {
            SvpSalFrame *pFrame = const_cast<SvpSalFrame*>(static_cast<const SvpSalFrame*>(*it));
            if( pFrame->IsVisible() )
            {
                pFrame->GetFocus();
                pFocus = pFrame;
                break;
            }
        }
    }
    return pFocus;
}

#if 0

int32_t AndroidSalInstance::onInputEvent (struct android_app* app, AInputEvent* event)
{
    bool bHandled = false;
    LOGI("input event for app %p, event %p type %d source %d device id %d",
         app, event,
         AInputEvent_getType(event),
         AInputEvent_getSource(event),
         AInputEvent_getDeviceId(event));

    switch (AInputEvent_getType(event))
    {
    case AINPUT_EVENT_TYPE_KEY:
    {
        int32_t nAction = AKeyEvent_getAction(event);
        LOGI("key event keycode %d '%s' %s flags (0x%x) 0x%x",
             AKeyEvent_getKeyCode(event),
             nAction == AKEY_EVENT_ACTION_DOWN ? "down" :
             nAction == AKEY_EVENT_ACTION_UP ? "up" : "multiple",
             KeyMetaStateToString(AKeyEvent_getMetaState(event)).getStr(),
             AKeyEvent_getMetaState (event),
             AKeyEvent_getFlags (event));

        // FIXME: the whole SALEVENT_KEYMODCHANGE stuff is going to be interesting
        // can we really emit that ? no input method madness required though.
        sal_uInt16 nEvent;
        SalKeyEvent aEvent;
        int64_t nNsTime = AKeyEvent_getEventTime(event);

        // FIXME: really we need a Java wrapper app as Mozilla has that does
        // key event translation for us, and provides -much- cleaner events.
        nEvent = (AKeyEvent_getAction(event) == AKEY_EVENT_ACTION_UP ?
                  SALEVENT_KEYUP : SALEVENT_KEYINPUT);
        sal_uInt16 nCode = KeyToCode(event);
        sal_uInt16 nMetaState = KeyMetaStateToCode(event);
        if (nCode >= KEY_0 && nCode <= KEY_9)
            aEvent.mnCharCode = '0' + nCode - KEY_0;
        else if (nCode >= KEY_A && nCode <= KEY_Z)
            aEvent.mnCharCode = (nMetaState & KEY_SHIFT ? 'A' : 'a') + nCode - KEY_A;
        else if (nCode == KEY_SPACE)
            aEvent.mnCharCode = ' ';
        else if (nCode == KEY_COMMA)
            aEvent.mnCharCode = ',';
        else if (nCode == KEY_POINT)
            aEvent.mnCharCode = '.';
        else
            aEvent.mnCharCode = 0;
        aEvent.mnTime = nNsTime / (1000 * 1000);
        aEvent.mnCode = nMetaState | nCode;
        aEvent.mnRepeat = AKeyEvent_getRepeatCount(event);

        SalFrame *pFocus = getFocusFrame();
        if (pFocus)
            bHandled = pFocus->CallCallback( nEvent, &aEvent );
        else
            LOGI("no focused frame to emit event on");

        LOGI("bHandled == %s", bHandled? "true": "false" );
        break;
    }
    case AINPUT_EVENT_TYPE_MOTION:
    {
        size_t nPoints = AMotionEvent_getPointerCount(event);
        LOGI("motion event %d %g,%g %d points: %s",
             AMotionEvent_getAction(event),
             AMotionEvent_getXOffset(event),
             AMotionEvent_getYOffset(event),
             (int)nPoints,
             MotionEdgeFlagsToString(AMotionEvent_getEdgeFlags(event)).getStr());
        for (size_t i = 0; i < nPoints; i++)
            LOGI("\t%d: %g,%g - pressure %g",
                 (int)i,
                 AMotionEvent_getX(event, i),
                 AMotionEvent_getY(event, i),
                 AMotionEvent_getPressure(event, i));

        SalMouseEvent aMouseEvent;
        sal_uInt16 nEvent = 0;

        // FIXME: all this filing the nEvent and aMouseEvent has to be cleaned up
        nEvent = AMotionEvent_getAction(event)? SALEVENT_MOUSEBUTTONUP: SALEVENT_MOUSEBUTTONDOWN;

        if (nPoints > 0)
        {
            aMouseEvent.mnX = AMotionEvent_getX(event, 0);
            aMouseEvent.mnY = AMotionEvent_getY(event, 0);
        } else {
            aMouseEvent.mnX = AMotionEvent_getXOffset(event);
            aMouseEvent.mnY = AMotionEvent_getYOffset(event);
        }

        int64_t nNsTime = AMotionEvent_getEventTime(event);
        aMouseEvent.mnTime = nNsTime / (1000 * 1000);
        aMouseEvent.mnCode = 0; // FIXME
        aMouseEvent.mnButton = MOUSE_LEFT; // FIXME

        SalFrame *pFocus = getFocusFrame();
        if (pFocus)
            bHandled = pFocus->CallCallback( nEvent, &aMouseEvent );
        else
            LOGI("no focused frame to emit event on");

        LOGI("bHandled == %s", bHandled? "true": "false" );

        break;
    }
    default:
        LOGI("unknown input event type %p %d",
             event, AInputEvent_getType(event));
        break;
    }
    return bHandled ? 1 : 0;
}

#endif

AndroidSalInstance *AndroidSalInstance::getInstance()
{
    if (!ImplGetSVData())
        return NULL;
    AndroidSalData *pData = static_cast<AndroidSalData *>(ImplGetSVData()->mpSalData);
    if (!pData)
        return NULL;
    return static_cast<AndroidSalInstance *>(pData->m_pInstance);
}

#if 0

extern "C" {
    void onAppCmd_cb (struct android_app *app, int32_t cmd)
    {
        AndroidSalInstance::getInstance()->onAppCmd(app, cmd);
    }

    int32_t onInputEvent_cb (struct android_app *app, AInputEvent *event)
    {
        return AndroidSalInstance::getInstance()->onInputEvent(app, event);
    }
}

#endif

AndroidSalInstance::AndroidSalInstance( SalYieldMutex *pMutex )
    : SvpSalInstance( pMutex )
    , mbQueueReDraw( false )
{
    LOGI("created Android Sal Instance thread: %d",
         (int)pthread_self());
}

AndroidSalInstance::~AndroidSalInstance()
{
    LOGI("destroyed Android Sal Instance");
}

void AndroidSalInstance::Wakeup()
{
    LOGI("Wakeup alooper");
    LOGI("busted - no global looper");
}

void AndroidSalInstance::DoReleaseYield (int nTimeoutMS)
{
    if (!bHitIdle)
        LOGI("hit idle !" );
    bHitIdle = true;

#if 1
    // Presumably this should never be called at all except in
    // NativeActivity-based apps with a GUI, where the message pump is
    // run here in vcl, but we don't have any NativeActivitry-based
    // apps any more.

    (void) nTimeoutMS;

    {
        static bool beenhere = false;
        if (!beenhere)
        {
            LOGI("**** Huh, %s called in non-NativeActivity app", __FUNCTION__);
            beenhere = true;
        }
        return;
    }
#else
    // release yield mutex
    sal_uLong nAcquireCount = ReleaseYieldMutex();

    LOGI("DoReleaseYield #3 %d thread: %d ms",
         nTimeoutMS, (int)pthread_self());

    struct android_poll_source *pSource = NULL;
    int outFd = 0, outEvents = 0;

    if (mbQueueReDraw)
        nTimeoutMS = 0;

    int nRet;
    nRet = ALooper_pollAll (nTimeoutMS, &outFd, &outEvents, (void**)&pSource);
    LOGI("ret #6 %d %d %d %p", nRet, outFd, outEvents, pSource);

    // acquire yield mutex again
    AcquireYieldMutex(nAcquireCount);

    if (nRet >= 0)
    {
        // Process this event.
        if (pSource != NULL)
            pSource->process(mpApp, pSource);
    }

    if (mbQueueReDraw && mpApp && mpApp->window)
        AndroidSalInstance::getInstance()->RedrawWindows (mpApp->window);
#endif
}

bool AndroidSalInstance::AnyInput( sal_uInt16 nType )
{
    if( (nType & VCL_INPUT_TIMER) != 0 )
        return CheckTimeout( false );
    // FIXME: ideally we should check our input queue here ...
    LOGI("FIXME: AnyInput returns false");
    return false;
}

class AndroidSalSystem : public SvpSalSystem {
public:
    AndroidSalSystem() : SvpSalSystem() {}
    virtual ~AndroidSalSystem() {}
    virtual int ShowNativeDialog( const rtl::OUString& rTitle,
                                  const rtl::OUString& rMessage,
                                  const std::list< rtl::OUString >& rButtons,
                                  int nDefButton );
};

SalSystem *AndroidSalInstance::CreateSalSystem()
{
    return new AndroidSalSystem();
}

class AndroidSalFrame : public SvpSalFrame
{
public:
    AndroidSalFrame( AndroidSalInstance *pInstance,
                     SalFrame           *pParent,
                     sal_uLong           nSalFrameStyle,
                     SystemParentData   *pSysParent )
        : SvpSalFrame( pInstance, pParent, nSalFrameStyle,
                       true, basebmp::Format::THIRTYTWO_BIT_TC_MASK_RGBA,
                       pSysParent )
    {
        enableDamageTracker();
        if (pParent == NULL && viewWidth > 1 && viewHeight > 1)
            SetPosSize(0, 0, viewWidth, viewHeight, SAL_FRAME_POSSIZE_WIDTH | SAL_FRAME_POSSIZE_HEIGHT);
    }

    virtual void GetWorkArea( Rectangle& rRect )
    {
        AndroidSalInstance::getInstance()->GetWorkArea( rRect );
    }

    virtual void damaged( const basegfx::B2IBox& rDamageRect)
    {
        if (rDamageRect.getWidth() <= 0 ||
            rDamageRect.getHeight() <= 0)
        {
            return;
        }
        AndroidSalInstance::getInstance()->damaged( this );
    }

    virtual void UpdateSettings( AllSettings &rSettings )
    {
        // Clobber the UI fonts
#if 0
        psp::FastPrintFontInfo aInfo;
        aInfo.m_aFamilyName = rtl::OUString( "Roboto" );
        aInfo.m_eItalic = ITALIC_NORMAL;
        aInfo.m_eWeight = WEIGHT_NORMAL;
        aInfo.m_eWidth = WIDTH_NORMAL;
        psp::PrintFontManager::get().matchFont( aInfo, rSettings.GetUILocale() );
#endif

        // FIXME: is 14 point enough ?
        Font aFont( rtl::OUString( "Roboto" ), Size( 0, 14 ) );

        StyleSettings aStyleSet = rSettings.GetStyleSettings();
        aStyleSet.SetAppFont( aFont );
        aStyleSet.SetHelpFont( aFont );
        aStyleSet.SetMenuFont( aFont );
        aStyleSet.SetToolFont( aFont );
        aStyleSet.SetLabelFont( aFont );
        aStyleSet.SetInfoFont( aFont );
        aStyleSet.SetRadioCheckFont( aFont );
        aStyleSet.SetPushButtonFont( aFont );
        aStyleSet.SetFieldFont( aFont );
        aStyleSet.SetIconFont( aFont );
        aStyleSet.SetGroupFont( aFont );

        rSettings.SetStyleSettings( aStyleSet );
    }
};

SalFrame *AndroidSalInstance::CreateChildFrame( SystemParentData* pParent, sal_uLong nStyle )
{
    return new AndroidSalFrame( this, NULL, nStyle, pParent );
}

SalFrame *AndroidSalInstance::CreateFrame( SalFrame* pParent, sal_uLong nStyle )
{
    return new AndroidSalFrame( this, pParent, nStyle, NULL );
}


// All the interesting stuff is slaved from the AndroidSalInstance
void InitSalData()   {}
void DeInitSalData() {}
void InitSalMain()   {}

void SalAbort( const rtl::OUString& rErrorText, bool bDumpCore )
{
    rtl::OUString aError( rErrorText );
    if( aError.isEmpty() )
        aError = rtl::OUString::createFromAscii("Unknown application error");
    LOGI("%s", rtl::OUStringToOString(rErrorText, osl_getThreadTextEncoding()).getStr() );

    LOGI("SalAbort: '%s'",
         rtl::OUStringToOString(aError, RTL_TEXTENCODING_ASCII_US).getStr());
    if( bDumpCore )
        abort();
    else
        _exit(1);
}

const OUString& SalGetDesktopEnvironment()
{
    static rtl::OUString aEnv( "android" );
    return aEnv;
}

SalData::SalData() :
    m_pInstance( 0 ),
    m_pPlugin( 0 ),
    m_pPIManager(0 )
{
}

SalData::~SalData()
{
}

// This is our main entry point:
SalInstance *CreateSalInstance()
{
    LOGI("Android: CreateSalInstance!");
    AndroidSalInstance* pInstance = new AndroidSalInstance( new SalYieldMutex() );
    new AndroidSalData( pInstance );
    pInstance->AcquireYieldMutex(1);
    return pInstance;
}

void DestroySalInstance( SalInstance *pInst )
{
    pInst->ReleaseYieldMutex();
    delete pInst;
}

#include <vcl/msgbox.hxx>

int AndroidSalSystem::ShowNativeDialog( const rtl::OUString& rTitle,
                                        const rtl::OUString& rMessage,
                                        const std::list< rtl::OUString >& rButtons,
                                        int nDefButton )
{
    (void)rButtons; (void)nDefButton;
    LOGI("LibreOffice native dialog '%s': '%s'",
         rtl::OUStringToOString(rTitle, RTL_TEXTENCODING_ASCII_US).getStr(),
         rtl::OUStringToOString(rMessage, RTL_TEXTENCODING_ASCII_US).getStr());
    LOGI("Dialog '%s': '%s'",
         rtl::OUStringToOString(rTitle, RTL_TEXTENCODING_ASCII_US).getStr(),
         rtl::OUStringToOString(rMessage, RTL_TEXTENCODING_ASCII_US).getStr());

    if (AndroidSalInstance::getInstance() != NULL)
    {
        // Does Android have a native dialog ? if not,. we have to do this ...

        // Of course it has. android.app.AlertDialog seems like a good
        // choice, it even has one, two or three buttons. Naturally,
        // it intended to be used from Java, so some verbose JNI
        // horror would be needed to use it directly here. Probably we
        // want some easier to use magic wrapper, hmm.

        ErrorBox aVclErrBox( NULL, WB_OK, rTitle );
        aVclErrBox.SetText( rMessage );
        aVclErrBox.Execute();
    }
    else
        LOGE("VCL not initialized");
    return 0;
}

// public static native void renderVCL(Bitmap bitmap);
extern "C" SAL_JNI_EXPORT void JNICALL
Java_org_libreoffice_experimental_desktop_Desktop_renderVCL(JNIEnv *env,
                                                            jobject /* clazz */,
                                                            jobject bitmap)
{
    if (!bHitIdle)
        return;

    AndroidBitmapInfo  info;
    void*              pixels;
    int                ret;

    if ((ret = AndroidBitmap_getInfo(env, bitmap, &info)) < 0) {
        LOGI("AndroidBitmap_getInfo() failed ! error=%d", ret);
        return;
    }

#if 0
    if (info.format != ANDROID_BITMAP_FORMAT_RGB_565) {
        LOGI("Bitmap format is not RGB_565 !");
        return;
    }
#endif

    if ((ret = AndroidBitmap_lockPixels(env, bitmap, &pixels)) < 0) {
        LOGI("AndroidBitmap_lockPixels() failed ! error=%d", ret);
    }

/*
typedef struct ANativeWindow_Buffer {
    // The number of pixels that are show horizontally.
    int32_t width;

    // The number of pixels that are shown vertically.
    int32_t height;

    // The number of *pixels* that a line in the buffer takes in
    // memory.  This may be >= width.
    int32_t stride;

    // The format of the buffer.  One of WINDOW_FORMAT_*
    int32_t format;

    // The actual bits.
    void* bits;

    // Do not touch.
    uint32_t reserved[6];
} ANativeWindow_Buffer;
*/

    ANativeWindow_Buffer dummyOut; // look like a window for now ...
    dummyOut.width = info.width;
    dummyOut.height = info.height;
    dummyOut.stride = info.stride / 4; // sigh !
    dummyOut.format = info.format;
    dummyOut.bits = pixels;
    AndroidSalInstance::getInstance()->RedrawWindows (NULL, &dummyOut);

    AndroidBitmap_unlockPixels(env, bitmap);
}

// public static native void setViewSize(int width, int height);
extern "C" SAL_JNI_EXPORT void JNICALL
Java_org_libreoffice_experimental_desktop_Desktop_setViewSize(JNIEnv * /* env */,
                                                              jobject /* clazz */,
                                                              jint width,
                                                              jint height)
{
    // Horrible
    viewWidth = width;
    viewHeight = height;
}

// public static native void key(char c, short timestamp);
extern "C" SAL_JNI_EXPORT void JNICALL
Java_org_libreoffice_experimental_desktop_Desktop_key(JNIEnv * /* env */,
                                                      jobject /* clazz */,
                                                      jchar c,
                                                      jshort /* timestamp */)
{
    SalFrame *pFocus = AndroidSalInstance::getInstance()->getFocusFrame();
    if (pFocus) {
        KeyEvent aEvent(c, c, 0);
        Application::PostKeyEvent(VCLEVENT_WINDOW_KEYINPUT, pFocus->GetWindow(), &aEvent);
        Application::PostKeyEvent(VCLEVENT_WINDOW_KEYUP, pFocus->GetWindow(), &aEvent);
    }
    else
        LOGW("No focused frame to emit event on");
}

// public static native void touch(int action, int x, int y, short timestamp);
extern "C" SAL_JNI_EXPORT void JNICALL
Java_org_libreoffice_experimental_desktop_Desktop_touch(JNIEnv * /* env */,
                                                        jobject /* clazz */,
                                                        jint action,
                                                        jint x,
                                                        jint y,
                                                        jshort /* timestamp */)
{
    SalFrame *pFocus = AndroidSalInstance::getInstance()->getFocusFrame();
    if (pFocus) {
        MouseEvent aEvent;
        sal_uLong nEvent;

        switch (action) {
        case AMOTION_EVENT_ACTION_DOWN:
            aEvent = MouseEvent(Point(x, y), 1, MOUSE_SIMPLECLICK, MOUSE_LEFT);
            nEvent = VCLEVENT_WINDOW_MOUSEBUTTONDOWN;
            break;
        case AMOTION_EVENT_ACTION_UP:
            aEvent = MouseEvent(Point(x, y), 1, MOUSE_SIMPLECLICK, MOUSE_LEFT);
            nEvent = VCLEVENT_WINDOW_MOUSEBUTTONUP;
            break;
        case AMOTION_EVENT_ACTION_MOVE:
            aEvent = MouseEvent(Point(x, y), 1, MOUSE_SIMPLEMOVE, MOUSE_LEFT);
            nEvent = VCLEVENT_WINDOW_MOUSEMOVE;
            break;
        default:
            LOGE("Java_org_libreoffice_experimental_desktop_Desktop_touch: Invalid action %d", action);
            return;
        }
        Application::PostMouseEvent(nEvent, pFocus->GetWindow(), &aEvent);
    }
    else
        LOGW("No focused frame to emit event on");
}

// public static native void zoom(float scale, int x, int y);
extern "C" SAL_JNI_EXPORT void JNICALL
Java_org_libreoffice_experimental_desktop_Desktop_zoom(JNIEnv * /* env */,
                                                       jobject /* clazz */,
                                                       jfloat scale,
                                                       jint x,
                                                       jint y)
{
    SalFrame *pFocus = AndroidSalInstance::getInstance()->getFocusFrame();
    if (pFocus) {
        SAL_INFO( "vcl.androidinst", "zoom: " << scale << "@(" << x << "," << y << ")" );
        ZoomEvent aEvent( Point( x, y ), scale);
        Application::PostZoomEvent(VCLEVENT_WINDOW_ZOOM, pFocus->GetWindow(), &aEvent);
    }
    else
        LOGW("No focused frame to emit event on");
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
