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
#include <android/log.h>
#include <android/looper.h>
#include <lo-bootstrap.h>
#include <osl/detail/android_native_app_glue.h>
#include <rtl/strbuf.hxx>

class AndroidSalData : public SalGenericData
{
public:
    AndroidSalData( SalInstance *pInstance ) : SalGenericData( SAL_DATA_ANDROID, pInstance ) {}
    virtual void ErrorTrapPush() {}
    virtual bool ErrorTrapPop( bool ) { return false; }
};

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

static void BlitFrameRegionToWindow(ANativeWindow *pWindow,
                                    const basebmp::BitmapDeviceSharedPtr& aDev,
                                    const ARect &rSrcRect,
                                    int nDestX, int nDestY)
{
    fprintf (stderr, "Blit frame src %d,%d->%d,%d to position %d, %d\n",
             rSrcRect.left, rSrcRect.top, rSrcRect.right, rSrcRect.bottom,
             nDestX, nDestY);
    ARect aRect;
    ANativeWindow_Buffer aOutBuffer;
    memset ((void *)&aOutBuffer, 0, sizeof (aOutBuffer));
    int32_t nRet = ANativeWindow_lock(pWindow, &aOutBuffer, &aRect);
    fprintf (stderr, "locked window %d returned rect: %d,%d->%d,%d "
             "buffer: %dx%d stride %d, format %d, bits %p\n",
             nRet, aRect.left, aRect.top, aRect.right, aRect.bottom,
             aOutBuffer.width, aOutBuffer.height, aOutBuffer.stride,
             aOutBuffer.format, aOutBuffer.bits);
    if (aOutBuffer.bits == NULL)
    {
        fprintf (stderr, "no buffer for locked window\n");
        ANativeWindow_unlockAndPost(pWindow);
        return;
    }

    // FIXME: do some cropping goodness on aSrcRect to ensure no overflows etc.
    ARect aSrcRect = rSrcRect;
    sal_Int32 nStride = aDev->getScanlineStride();
    basebmp::RawMemorySharedArray aSrcData = aDev->getBuffer();
    unsigned char *pSrc = aSrcData.get();

    for (unsigned int y = 0; y < (unsigned int)(aSrcRect.bottom - aSrcRect.top); y++)
    {
        unsigned char *sp = ( pSrc + nStride * (y + aSrcRect.top) +
                              aSrcRect.left * 3 /* src pixel size */ );
        unsigned char *dp = ( (unsigned char *)aOutBuffer.bits +
                              aOutBuffer.stride * (y + nDestY) +
                              nDestX * 4 /* dest pixel size */ );
        for (unsigned int x = 0; x < (unsigned int)(aSrcRect.right - aSrcRect.left); x++)
        {
            dp[x*4 + 0] = sp[x*3 + 0]; // B
            dp[x*4 + 1] = sp[x*3 + 1]; // G
            dp[x*4 + 2] = sp[x*3 + 2]; // R
            dp[x*4 + 3] = 255; // A
        }
    }
    fprintf (stderr, "done blit!\n");
#if 0
    // hard-code / guess at a format ...
    int32_t *p = (int32_t *)aBuffer.bits;
    for (int32_t y = 0; y < aBuffer.height; y++)
    {
        for (int32_t x = 0; x < aBuffer.stride / 4; x++)
            *p++ = (y << 24) + x;
    }
#endif

    ANativeWindow_unlockAndPost(pWindow);
    fprintf (stderr, "done render!\n");
}

void AndroidSalInstance::BlitFrameToWindow(ANativeWindow *pWindow,
                                           const basebmp::BitmapDeviceSharedPtr& aDev)
{
    basegfx::B2IVector aDevSize = aDev->getSize();
    ARect aWhole = { 0, 0, 400, 400 }; // FIXME: aDevSize.getX(), aDevSize.getY() };
    BlitFrameRegionToWindow(pWindow, aDev, aWhole, 0, 0);
}

void AndroidSalInstance::RedrawWindows(ANativeWindow *pWindow)
{
    std::list< SalFrame* >::const_iterator it;
    for ( it = getFrames().begin(); it != getFrames().end(); it++ )
    {
        SvpSalFrame *pFrame = static_cast<SvpSalFrame *>(*it);
        BlitFrameToWindow (pWindow, pFrame->getDevice());
    }
}

void AndroidSalInstance::onAppCmd (struct android_app* app, int32_t cmd)
{
        fprintf (stderr, "app cmd for app %p, cmd %d\n", app, cmd);
        ANativeWindow *pWindow = mpApp->window;
        switch (cmd) {
        case APP_CMD_INIT_WINDOW:
        {
            ARect aRect = { 0, 0, 0, 0 };
            aRect.right = ANativeWindow_getWidth(pWindow);
            aRect.bottom = ANativeWindow_getHeight(pWindow);
            fprintf (stderr, "we have an app window ! %p %dx%x (%d)\n",
                     pWindow, aRect.right, aRect.bottom,
                     ANativeWindow_getFormat(pWindow));

            RedrawWindows(pWindow);
            break;
        }
        case APP_CMD_WINDOW_RESIZED:
        {
            ARect aRect = { 0, 0, 0, 0 };
            aRect.right = ANativeWindow_getWidth(pWindow);
            aRect.bottom = ANativeWindow_getHeight(pWindow);
            fprintf (stderr, "app window resized to ! %p %dx%x (%d)\n",
                     pWindow, aRect.right, aRect.bottom,
                     ANativeWindow_getFormat(pWindow));
            RedrawWindows(pWindow);
            break;
        }

        case APP_CMD_WINDOW_REDRAW_NEEDED:
        {
            RedrawWindows(pWindow);
            break;
        }

        case APP_CMD_CONTENT_RECT_CHANGED:
        {
            ARect aRect = mpApp->contentRect;
            fprintf (stderr, "content rect changed [ k/b popped up etc. ] %d,%d->%d,%d\n",
                     aRect.left, aRect.top, aRect.right, aRect.bottom);
            break;
        }
        default:
            fprintf (stderr, "unhandled app cmd %d\n", cmd);
            break;
        }
}

int32_t AndroidSalInstance::onInputEvent (struct android_app* app, AInputEvent* event)
{
        fprintf (stderr, "input event for app %p, event %p type %d source %d device id %d\n",
                 app, event,
                 AInputEvent_getType(event),
                 AInputEvent_getSource(event),
                 AInputEvent_getDeviceId(event));

        switch (AInputEvent_getType(event))
        {
        case AINPUT_EVENT_TYPE_KEY:
        {
            int32_t nAction = AKeyEvent_getAction(event);
            fprintf (stderr, "key event keycode %d '%s' %s\n",
                     AKeyEvent_getKeyCode(event),
                     nAction == AKEY_EVENT_ACTION_DOWN ? "down" :
                     nAction == AKEY_EVENT_ACTION_UP ? "up" : "multiple",
                     KeyMetaStateToString(AKeyEvent_getMetaState(event)).getStr());
            break;
        }
        case AINPUT_EVENT_TYPE_MOTION:
        {
            fprintf (stderr, "motion event %d %g %g %s\n",
                     AMotionEvent_getAction(event),
                     AMotionEvent_getXOffset(event),
                     AMotionEvent_getYOffset(event),
                     MotionEdgeFlagsToString(AMotionEvent_getEdgeFlags(event)).getStr());
            break;
        }
        default:
            fprintf (stderr, "unknown event type %p %d\n",
                     event, AInputEvent_getType(event));
        }
        return 1; // handled 0 for not ...
}

AndroidSalInstance *AndroidSalInstance::getInstance()
{
    if (!ImplGetSVData())
        return NULL;
    AndroidSalData *pData = static_cast<AndroidSalData *>(ImplGetSVData()->mpSalData);
    if (!pData)
        return NULL;
    return static_cast<AndroidSalInstance *>(pData->m_pInstance);
}

extern "C" {
    void onAppCmd_cb (struct android_app* app, int32_t cmd)
    {
        AndroidSalInstance::getInstance()->onAppCmd(app, cmd);
    }

    int32_t onInputEvent_cb (struct android_app* app, AInputEvent* event)
    {
        return AndroidSalInstance::getInstance()->onInputEvent(app, event);
    }
}

AndroidSalInstance::AndroidSalInstance( SalYieldMutex *pMutex )
    : SvpSalInstance( pMutex )
{
    mpApp = lo_get_app();
    fprintf (stderr, "created Android Sal Instance for app %p window %p\n",
             mpApp,
             mpApp ? mpApp->window : NULL);
    if (mpApp)
    {
        pthread_mutex_lock (&mpApp->mutex);
        mpApp->onAppCmd = onAppCmd_cb;
        mpApp->onInputEvent = onInputEvent_cb;
        if (mpApp->window != NULL)
            onAppCmd_cb (mpApp, APP_CMD_INIT_WINDOW);
        pthread_mutex_unlock (&mpApp->mutex);
    }
}

AndroidSalInstance::~AndroidSalInstance()
{
    fprintf (stderr, "destroyed Android Sal Instance\n");
}

void AndroidSalInstance::Wakeup()
{
    fprintf (stderr, "Wakeup alooper\n");
    if (mpApp && mpApp->looper)
        ALooper_wake (mpApp->looper);
    else
        fprintf (stderr, "busted - no global looper\n");
}

void AndroidSalInstance::DoReleaseYield (int nTimeoutMS)
{
    // release yield mutex
    sal_uLong nAcquireCount = ReleaseYieldMutex();

    fprintf (stderr, "DoReleaseYield #2 %d ms\n", nTimeoutMS);
    void *outData = NULL;
    int outFd = 0, outEvents = 0;
    int nRet = ALooper_pollAll(nTimeoutMS, &outFd, &outEvents, &outData);
    fprintf (stderr, "ret %d %d %d %p\n", nRet, outFd, outEvents, outData);

    // acquire yield mutex again
    AcquireYieldMutex(nAcquireCount);

    // FIXME: this is more or less deranged: why can we not
    // set a callback in the native app glue's ALooper_addFd ?
    if (nRet == LOOPER_ID_MAIN)
        mpApp->cmdPollSource.process(mpApp, &mpApp->cmdPollSource);
    if (nRet == LOOPER_ID_INPUT)
        mpApp->inputPollSource.process(mpApp, &mpApp->inputPollSource);
}

bool AndroidSalInstance::AnyInput( sal_uInt16 nType )
{
    (void) nType;
    // FIXME: ideally we should check the input queue to avoid being busy ...
    fprintf (stderr, "FIXME: AnyInput returns true\n");
    // mpApp->inputQueue ? ...
    return true;
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

// All the interesting stuff is slaved from the AndroidSalInstance
void InitSalData()   {}
void DeInitSalData() {}
void InitSalMain()   {}
void DeInitSalMain() {}

void SalAbort( const rtl::OUString& rErrorText, bool bDumpCore )
{
    rtl::OUString aError( rErrorText );
    if( aError.isEmpty() )
        aError = rtl::OUString::createFromAscii("Unknown application error");
    ::fprintf( stderr, "%s\n", rtl::OUStringToOString(rErrorText, osl_getThreadTextEncoding()).getStr() );

    __android_log_print(ANDROID_LOG_INFO, "SalAbort: '%s'",
                        rtl::OUStringToOString(aError, RTL_TEXTENCODING_ASCII_US).getStr());
    if( bDumpCore )
        abort();
    else
        _exit(1);
}

const OUString& SalGetDesktopEnvironment()
{
    static rtl::OUString aEnv( RTL_CONSTASCII_USTRINGPARAM( "android" ) );
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
    fprintf (stderr, "Android: CreateSalInstance!\n");
    AndroidSalInstance* pInstance = new AndroidSalInstance( new SalYieldMutex() );
    new AndroidSalData( pInstance );
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
    fprintf (stderr, "LibreOffice native dialog '%s': '%s'\n",
             rtl::OUStringToOString(rTitle, RTL_TEXTENCODING_ASCII_US).getStr(),
             rtl::OUStringToOString(rMessage, RTL_TEXTENCODING_ASCII_US).getStr());
    __android_log_print(ANDROID_LOG_INFO, "LibreOffice - dialog '%s': '%s'",
                        rtl::OUStringToOString(rTitle, RTL_TEXTENCODING_ASCII_US).getStr(),
                        rtl::OUStringToOString(rMessage, RTL_TEXTENCODING_ASCII_US).getStr());

    if (AndroidSalInstance::getInstance() != NULL)
    {
        // Does Android have a native dialog ? if not,. we have to do this ...
        ErrorBox aVclErrBox( NULL, WB_OK, rTitle );
        aVclErrBox.SetText( rMessage );
        aVclErrBox.Execute();
    }
    else
        fprintf (stderr, "VCL not initialized\n");
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
