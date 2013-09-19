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

#include <config_lgpl.h>

#include <cstdarg>

#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>

#include "premac.h"
#include <Cocoa/Cocoa.h>
#include "postmac.h"

#include <osl/thread.h>

#include <plugin/impl.hxx>

extern NPNetscapeFuncs aNPNFuncs;

#include <tools/debug.hxx>

using namespace plugstringhelper;

using ::rtl::OUString;
using ::rtl::OUStringToOString;

#if OSL_DEBUG_LEVEL > 1
void TRACE( char const * s );
void TRACEN( char const * s, long n );
#else
#define TRACE(x)
#define TRACEN(x,n)
#endif


struct SysPlugData
{
    MacPluginComm::NP_CGContext m_aCGContext;
    NP_Port                     m_aNPPort;
    NSView*                     m_pParentView;
    NSView*                     m_pPlugView;
    int                         m_nDrawingModel;
    NSPoint                     m_aLastPlugViewOrigin;
    bool                        m_bSetWindowOnDraw;
    SysPlugData()
    {
        memset( this, 0, sizeof(*this) );
    }
};

::boost::shared_ptr<SysPlugData> CreateSysPlugData()
{
    return ::boost::shared_ptr<SysPlugData>(new SysPlugData);
}

void XPlugin_Impl::SetSysPlugDataParentView(SystemEnvData const& rEnvData)
{
    m_pSysPlugData->m_pParentView = rEnvData.pView;
}

extern "C" {

void /*SAL_CALL NP_LOADDS*/  NPN_ForceRedraw_Impl(NPP instance)
{
    TRACE( "NPN_ForceRedraw_Impl" );
    XPlugin_Impl* pImpl = XPluginManager_Impl::getXPluginFromNPP( instance );
    if( pImpl )
    {
        SysPlugData& rPlugData( pImpl->getSysPlugData() );
        if( rPlugData.m_pPlugView )
            [rPlugData.m_pPlugView setNeedsDisplay: YES];
    }
}

NPError /*SAL_CALL NP_LOADDS*/  NPN_SetValue_Impl( NPP instance,
                                          NPPVariable variable,
                                          void* value )
{
    TRACE( "NPN_SetValue_Impl" );
    switch( variable )
    {
        case (NPPVariable)1000: // NPNVpluginDrawingModel
        {
            // ugly, but that's the way we need to do it
            int nDrawingModel = (int)value;

            TRACEN( "drawing model: ", nDrawingModel );
            XPlugin_Impl* pImpl =
                XPluginManager_Impl::getXPluginFromNPP( instance );
            if (pImpl)
                pImpl->getSysPlugData().m_nDrawingModel = nDrawingModel;
        }
        break;
        default:
        break;
    }
    return NPERR_NO_ERROR;
}

} // extern "C"

struct FakeEventRecord : public EventRecord
{
    FakeEventRecord()
    {
        memset( this, 0, sizeof(EventRecord) );
        ::GetGlobalMouse( &where );
        when = ::TickCount();
        modifiers = ::GetCurrentEventKeyModifiers();
    }
};


@interface OOoPluginView : NSView
{
    XPlugin_Impl*   m_pImpl;
    MacPluginComm*  m_pCom;
}
-(id)initWithInstance: (XPlugin_Impl*)i_pImpl pluginComm: (MacPluginComm*)i_pCom frame: (NSRect)i_aRect;
-(void)drawRect: (NSRect)i_aRect;
-(BOOL)isOpaque;
-(BOOL)isFlipped;

// NSResponder
-(void)mouseMoved:   (NSEvent*)i_pEvent;
-(void)mouseDown:    (NSEvent*)i_pEvent;
-(void)mouseDragged: (NSEvent*)i_pEvent;
-(void)mouseUp:      (NSEvent*)i_pEvent;
-(void)rightMouseDown:    (NSEvent*)i_pEvent;
-(void)rightMouseDragged: (NSEvent*)i_pEvent;
-(void)rightMouseUp:      (NSEvent*)i_pEvent;
-(void)otherMouseDown:    (NSEvent*)i_pEvent;
-(void)otherMouseDragged: (NSEvent*)i_pEvent;
-(void)otherMouseUp:      (NSEvent*)i_pEvent;
-(void)mouseEntered: (NSEvent*)i_pEvent;
-(void)mouseExited:  (NSEvent*)i_pEvent;
@end

@implementation OOoPluginView
-(id)initWithInstance: (XPlugin_Impl*)i_pImpl pluginComm: (MacPluginComm*)i_pCom frame: (NSRect) i_aRect
{
    if( (self = [super initWithFrame: i_aRect]) )
    {
        m_pImpl = i_pImpl;
        m_pCom = i_pCom;
    }
    return self;
}

-(void)drawRect: (NSRect) i_aRect
{
    (void) i_aRect; // unused
    m_pCom->drawView( m_pImpl );
}

-(BOOL)isOpaque
{
    return NO;
}

-(BOOL)isFlipped
{
    return YES;
}

// NSResponder
-(void)mouseMoved:   (NSEvent*)i_pEvent
{
    (void) i_pEvent; // unused
    FakeEventRecord aRec;
    aRec.what = osEvt + 18; // NPEventType_AdjustCursorEvent
    m_pCom->NPP_HandleEvent( m_pImpl->getNPPInstance(), &aRec );
}

-(void)mouseDown:    (NSEvent*)i_pEvent
{
    (void) i_pEvent; // unused
    FakeEventRecord aRec;
    aRec.what = mouseDown;
    m_pCom->NPP_HandleEvent( m_pImpl->getNPPInstance(), &aRec );
}

-(void)mouseDragged: (NSEvent*)i_pEvent
{
    (void) i_pEvent; // unused
    FakeEventRecord aRec;
    aRec.what = aRec.what = osEvt + 18; // NPEventType_AdjustCursorEvent
    m_pCom->NPP_HandleEvent( m_pImpl->getNPPInstance(), &aRec );
}

-(void)mouseUp:      (NSEvent*)i_pEvent
{
    (void) i_pEvent; // unused
    FakeEventRecord aRec;
    aRec.what = mouseUp;
    m_pCom->NPP_HandleEvent( m_pImpl->getNPPInstance(), &aRec );
}

-(void)rightMouseDown:    (NSEvent*)i_pEvent
{
    (void) i_pEvent; // unused
    FakeEventRecord aRec;
    aRec.what = mouseDown;
    m_pCom->NPP_HandleEvent( m_pImpl->getNPPInstance(), &aRec );
}

-(void)rightMouseDragged: (NSEvent*)i_pEvent
{
    (void) i_pEvent; // unused
    FakeEventRecord aRec;
    aRec.what = aRec.what = osEvt + 18; // NPEventType_AdjustCursorEvent
    m_pCom->NPP_HandleEvent( m_pImpl->getNPPInstance(), &aRec );
}

-(void)rightMouseUp:      (NSEvent*)i_pEvent
{
    (void) i_pEvent; // unused
    FakeEventRecord aRec;
    aRec.what = mouseUp;
    m_pCom->NPP_HandleEvent( m_pImpl->getNPPInstance(), &aRec );
}

-(void)otherMouseDown:    (NSEvent*)i_pEvent
{
    (void) i_pEvent; // unused
    FakeEventRecord aRec;
    aRec.what = mouseDown;
    m_pCom->NPP_HandleEvent( m_pImpl->getNPPInstance(), &aRec );
}

-(void)otherMouseDragged: (NSEvent*)i_pEvent
{
    (void) i_pEvent; // unused
    FakeEventRecord aRec;
    aRec.what = aRec.what = osEvt + 18; // NPEventType_AdjustCursorEvent
    m_pCom->NPP_HandleEvent( m_pImpl->getNPPInstance(), &aRec );
}

-(void)otherMouseUp:      (NSEvent*)i_pEvent
{
    (void) i_pEvent; // unused
    FakeEventRecord aRec;
    aRec.what = mouseUp;
    m_pCom->NPP_HandleEvent( m_pImpl->getNPPInstance(), &aRec );
}

-(void)mouseEntered: (NSEvent*)i_pEvent
{
    (void) i_pEvent; // unused
    FakeEventRecord aRec;
    aRec.what = aRec.what = osEvt + 18; // NPEventType_AdjustCursorEvent
    m_pCom->NPP_HandleEvent( m_pImpl->getNPPInstance(), &aRec );
}

-(void)mouseExited:  (NSEvent*)i_pEvent
{
    (void) i_pEvent; // unused
    FakeEventRecord aRec;
    aRec.what = aRec.what = osEvt + 18; // NPEventType_AdjustCursorEvent
    m_pCom->NPP_HandleEvent( m_pImpl->getNPPInstance(), &aRec );
}

@end

//--------------------------------------------------------------------------------------------------
MacPluginComm::MacPluginComm( const rtl::OUString& i_rMimetype, const rtl::OUString& i_rBundle, NSView* i_pParent )
    : PluginComm( OUStringToOString( i_rBundle, RTL_TEXTENCODING_UTF8 ) ),
      m_xBundle( NULL ),
      m_hPlugLib( NULL ),
      m_pNullTimer( NULL )
{
    (void) i_rMimetype; // unused
    (void) i_pParent; // unused
    // initialize plugin function table
    memset( &m_aNPPfuncs, 0, sizeof( m_aNPPfuncs ) );

    // load the bundle
    CFURLRef xURL = createURL( i_rBundle );
    m_xBundle = CFBundleCreate( NULL, xURL );
    CFRelease( xURL );
    if( m_xBundle )
    {
        // ask the plugin library
        // first get its location
        CFURLRef xLibURL = CFBundleCopyExecutableURL( m_xBundle );
        if( xLibURL )
        {
            // get the file system path
            rtl::OUString aModuleURL( CFURLtoOSLURL( xLibURL ) );
            CFRelease( xLibURL );
            m_hPlugLib = osl_loadModule( aModuleURL.pData, SAL_LOADMODULE_DEFAULT );
            #if OSL_DEBUG_LEVEL > 1
            if( ! m_hPlugLib )
                fprintf( stderr, "module %s could not be loaded\n", OUStringToOString( aModuleURL, RTL_TEXTENCODING_UTF8 ).getStr() );
            #endif
        }
        #if OSL_DEBUG_LEVEL > 1
        else
            fprintf( stderr, "bundle %s has no exectutable URL\n", OUStringToOString( i_rBundle, RTL_TEXTENCODING_UTF8 ).getStr() );
        #endif
    }
    else
    {
        #if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, "bundle %s could not be loaded\n", OUStringToOString( i_rBundle, RTL_TEXTENCODING_UTF8 ).getStr() );
        #endif
    }

    DBG_ASSERT( m_xBundle && m_hPlugLib, "loading plugin bundle failed!" );


    m_aNPPfuncs.size = sizeof( m_aNPPfuncs );
    m_aNPPfuncs.version = 0;


    m_eCall = eNP_Initialize;
    execute();
}

//--------------------------------------------------------------------------------------------------
MacPluginComm::~MacPluginComm()
{
    if( m_hPlugLib )
    {
        // NPP_Shutdown();
        NPError (*pShutdown)();
        if( retrieveFunction( "NP_Shutdown", (void**)&pShutdown ) )
        {
            NPError nErr = (*pShutdown)(); (void)nErr;
            DBG_ASSERT( nErr == NPERR_NO_ERROR, "NP_Shutdown() failed!" );
        }
        osl_unloadModule( m_hPlugLib );
    }
    if( m_xBundle )
        CFRelease( m_xBundle );
}

//--------------------------------------------------------------------------------------------------
sal_Bool MacPluginComm::retrieveFunction( const char* i_pName, void** o_ppFunc ) const
{
    if( ! m_hPlugLib || ! o_ppFunc )
        return sal_False;

    *o_ppFunc = (void*)osl_getAsciiFunctionSymbol( m_hPlugLib, i_pName );

    if( ! *o_ppFunc && m_xBundle )
    {
        rtl::OUString aName( OUString::createFromAscii( *i_pName == '_' ? i_pName+1 : i_pName ) );
        CFStringRef xName = createString( aName );
        if( xName )
        {
            *o_ppFunc =  CFBundleGetFunctionPointerForName( m_xBundle, xName );
            CFRelease( xName );
        }
    }

    return (*o_ppFunc != NULL);
}

IMPL_LINK_NOARG(MacPluginComm, NullTimerHdl)
{
    // note: this is a Timer handler, we are already protected by the SolarMutex

    FakeEventRecord aRec;
    aRec.what = nullEvent;
    aRec.where.h = aRec.where.v = 20000;

    for( std::list< XPlugin_Impl* >::iterator it = m_aNullEventClients.begin();
         it != m_aNullEventClients.end(); ++it )
    {
        SysPlugData& rPlugData( (*it)->getSysPlugData() );
        if( rPlugData.m_pPlugView ) // for safety do not dispatch null events before first NPP_SetWindow
            (*m_aNPPfuncs.event)( (*it)->getNPPInstance(), &aRec );
    }

    return 0;
}

//--------------------------------------------------------------------------------------------------

long MacPluginComm::doIt()
{
    long nRet = 0;
    switch( m_eCall )
    {
    case eNP_Initialize:
    {
        TRACE( "eNP_Initialize" );
        NPError (*pInit)( NPNetscapeFuncs* );
        if( retrieveFunction( "NP_Initialize", (void**)&pInit ) )
        {
            nRet = (*pInit)( &aNPNFuncs );

            NPError nErr = NPERR_NO_ERROR;
            NPError (*pEntry)( NPPluginFuncs* );
            retrieveFunction( "NP_GetEntryPoints", (void**)&pEntry );
            nErr = (*pEntry)( &m_aNPPfuncs );

            DBG_ASSERT( nErr == NPERR_NO_ERROR, "NP_GetEntryPoints() failed!" );
        }
        else
        {
            nRet = NPERR_GENERIC_ERROR;
        }
        DBG_ASSERT( nRet == NPERR_NO_ERROR, "### NP_Initialize() failed!" );
    }
    break;
    case eNPP_Destroy:
        if( m_aNullEventClients.empty() )
            delete m_pNullTimer, m_pNullTimer = NULL;

        TRACE( "eNPP_Destroy" );
        nRet = (m_aNPPfuncs.destroy
                ? (*m_aNPPfuncs.destroy)(
                    (NPP)m_aArgs[0],
                    (NPSavedData**)m_aArgs[1] )
                : NPERR_GENERIC_ERROR);
        break;
    case eNPP_DestroyStream:
        TRACE( "eNPP_DestroyStream" );
        nRet =  (m_aNPPfuncs.destroystream
                 ? (*m_aNPPfuncs.destroystream)(
                     (NPP)m_aArgs[0],
                     (NPStream*)m_aArgs[1],
                     (NPError)(sal_IntPtr)m_aArgs[2] )
                 : NPERR_GENERIC_ERROR);
        break;
    case eNPP_New:
        TRACE( "eNPP_New" );
        nRet = (m_aNPPfuncs.newp
                ? (*m_aNPPfuncs.newp)(
                    (NPMIMEType)m_aArgs[0],
                    (NPP)m_aArgs[1],
                    (uint16_t)(sal_IntPtr)m_aArgs[2],
                    (int16_t)(sal_IntPtr)m_aArgs[3],
                    (char**)m_aArgs[4],
                    (char**)m_aArgs[5],
                    (NPSavedData*)m_aArgs[6] )
                : NPERR_GENERIC_ERROR);

        if( ! m_pNullTimer && m_aNPPfuncs.event )
        {
            m_pNullTimer = new AutoTimer();
            m_pNullTimer->SetTimeout( 50 );
            m_pNullTimer->SetTimeoutHdl( LINK( this, MacPluginComm, NullTimerHdl ) );
            m_pNullTimer->Start();
        }

        break;
    case eNPP_NewStream:
        TRACE( "eNPP_NewStream" );
        nRet = (m_aNPPfuncs.newstream
                ? (*m_aNPPfuncs.newstream)(
                    (NPP)m_aArgs[0],
                    (NPMIMEType)m_aArgs[1],
                    (NPStream*)m_aArgs[2],
                    (NPBool)(sal_IntPtr)m_aArgs[3],
                    (uint16_t*)m_aArgs[4] )
                : NPERR_GENERIC_ERROR);
        break;
    case eNPP_Print:
        TRACE( "eNPP_Print" );
        if (m_aNPPfuncs.print)
            (*m_aNPPfuncs.print)(
                (NPP)m_aArgs[0],
                (NPPrint*)m_aArgs[1] );
        break;
    case eNPP_SetWindow:
    {
        TRACE( "eNPP_SetWindow" );
        nRet = (m_aNPPfuncs.setwindow
                ? (*m_aNPPfuncs.setwindow)(
                    (NPP)m_aArgs[0],
                    (NPWindow*)m_aArgs[1] )
                : NPERR_GENERIC_ERROR);

        break;
    }
    case eNPP_HandleEvent:
    {
        TRACE( "eNPP_HandleEvent" );
        nRet = (m_aNPPfuncs.event
                ? (*m_aNPPfuncs.event)(
                    (NPP)m_aArgs[0],
                    m_aArgs[1] )
                : NPERR_GENERIC_ERROR);

        break;
    }
    case eNPP_StreamAsFile:
        TRACE( "eNPP_StreamAsFile" );
        if (m_aNPPfuncs.asfile)
            (*m_aNPPfuncs.asfile)(
                (NPP)m_aArgs[0],
                (NPStream*)m_aArgs[1],
                (char*)m_aArgs[2] );
        break;
    case eNPP_URLNotify:
        TRACE( "eNPP_URLNotify" );
        if (m_aNPPfuncs.urlnotify)
            (*m_aNPPfuncs.urlnotify)(
                (NPP)m_aArgs[0],
                (char*)m_aArgs[1],
                (NPReason)(sal_IntPtr)m_aArgs[2],
                m_aArgs[3] );
        break;
    case eNPP_Write:
        TRACEN( "eNPP_Write n=", (int32_t)m_aArgs[3] );
        nRet = (m_aNPPfuncs.write
                ? (*m_aNPPfuncs.write)(
                    (NPP)m_aArgs[0],
                    (NPStream*)m_aArgs[1],
                    (int32_t)m_aArgs[2],
                    (int32_t)m_aArgs[3],
                    m_aArgs[4] )
                : 0);
        break;
    case eNPP_WriteReady:
        TRACE( "eNPP_WriteReady" );
        nRet = (m_aNPPfuncs.writeready
                ? (*m_aNPPfuncs.writeready)(
                    (NPP)m_aArgs[0],
                    (NPStream*)m_aArgs[1] )
                : 0);
        break;
    case eNPP_GetValue:
        TRACE( "eNPP_GetValue" );
        nRet = (m_aNPPfuncs.getvalue
                ? (*m_aNPPfuncs.getvalue)(
                    (NPP)m_aArgs[0],
                    (NPPVariable)(int)m_aArgs[1],
                    m_aArgs[2] )
                : NPERR_GENERIC_ERROR);
        break;
    case eNPP_SetValue:
        TRACE( "eNPP_SetValue" );
        nRet = (m_aNPPfuncs.setvalue
                ? (*m_aNPPfuncs.setvalue)(
                    (NPP)m_aArgs[0],
                    (NPNVariable)(int)m_aArgs[1],
                    m_aArgs[2] )
                : NPERR_GENERIC_ERROR);
        break;
    case eNPP_Shutdown:
    {
        TRACE( "eNPP_Shutdown" );
        NPP_ShutdownUPP pFunc;
        if (retrieveFunction( "NPP_Shutdown", (void**)&pFunc ))
            (*pFunc)();
    }
    break;
    case eNPP_Initialize:
        TRACE( "eNPP_Initialize" );
        OSL_FAIL( "NPP_Initialize: not implemented!" );
        break;
    case eNPP_GetJavaClass:
        TRACE( "eNPP_GetJavaClass" );
        OSL_FAIL( "NPP_GetJavaClass: not implemented!" );
        break;
    }
    return nRet;
}

//--------------------------------------------------------------------------------------------------
NPError MacPluginComm::NPP_Destroy( XPlugin_Impl* i_pImpl, NPSavedData** save )
{
    // remove from NullEvent timer
    m_aNullEventClients.remove( i_pImpl );

    NPError nErr = NPP_Destroy( i_pImpl->getNPPInstance(), save );

    // release plugin view
    SysPlugData& rPlugData( i_pImpl->getSysPlugData() );
    if( rPlugData.m_pPlugView )
    {
        [rPlugData.m_pPlugView removeFromSuperview];
        [rPlugData.m_pPlugView release];
        rPlugData.m_pPlugView = nil;
    }

    return nErr;
}


NPError MacPluginComm::NPP_Destroy( NPP instance, NPSavedData** save )
{
    DBG_ASSERT( m_aNPPfuncs.destroy, "### NPP_Destroy(): null pointer in NPP functions table!" );
    m_eCall = eNPP_Destroy;
    m_aArgs[0] = (void*)instance;
    m_aArgs[1] = (void*)save;
    return (NPError)execute();
}

//--------------------------------------------------------------------------------------------------
NPError MacPluginComm::NPP_DestroyStream( NPP instance, NPStream* stream, NPError reason )
{
    DBG_ASSERT( m_aNPPfuncs.destroystream, "### NPP_DestroyStream(): null pointer in NPP functions table!" );
    m_eCall = eNPP_DestroyStream;
    m_aArgs[0] = (void*)instance;
    m_aArgs[1] = (void*)stream;
    m_aArgs[2] = (void*)(intptr_t)reason;
    return (NPError)execute();
}

//--------------------------------------------------------------------------------------------------
NPError MacPluginComm::NPP_New( NPMIMEType pluginType, NPP instance, uint16_t mode, int16_t argc,
                                  char* argn[], char* argv[], NPSavedData *saved )
{
    XPlugin_Impl* pImpl = XPluginManager_Impl::getXPluginFromNPP( instance );
    DBG_ASSERT( pImpl, "no instance found" );

    if( pImpl ) // sanity check
        m_aNullEventClients.push_back( pImpl );

    DBG_ASSERT( m_aNPPfuncs.newp, "### NPP_New(): null pointer in NPP functions table!" );
    #if OSL_DEBUG_LEVEL > 1
    fprintf( stderr, "NPP_New( %s. %p, %d, %d",
             pluginType, instance, (int)mode, (int)argc );
    for( int16_t i = 0; i < argc; i++ )
    fprintf( stderr, "\n%s = %s", argn[i], argv[i] );
    fprintf( stderr, ", %p )\n", saved );
    #endif
    m_eCall = eNPP_New;
    m_aArgs[0] = (void*)pluginType;
    m_aArgs[1] = (void*)instance;
    m_aArgs[2] = (void*)(intptr_t)mode;
    m_aArgs[3] = (void*)(intptr_t)argc;
    m_aArgs[4] = (void*)argn;
    m_aArgs[5] = (void*)argv;
    m_aArgs[6] = (void*)saved;

    return (NPError)execute();
}

//--------------------------------------------------------------------------------------------------
NPError MacPluginComm::NPP_NewStream( NPP instance, NPMIMEType type, NPStream* stream,
                                        NPBool seekable, uint16_t* stype )
{
    DBG_ASSERT( m_aNPPfuncs.newstream, "### NPP_NewStream(): null pointer in NPP functions table!" );
    m_eCall = eNPP_NewStream;
    m_aArgs[0] = (void*)instance;
    m_aArgs[1] = (void*)type;
    m_aArgs[2] = (void*)stream;
    m_aArgs[3] = (void*)(intptr_t)seekable;
    m_aArgs[4] = (void*)stype;
    return (NPError)execute();
}

//--------------------------------------------------------------------------------------------------
void MacPluginComm::NPP_Print( NPP instance, NPPrint* platformPrint )
{
    DBG_ASSERT( m_aNPPfuncs.print, "### NPP_Print(): null pointer in NPP functions table!" );
    m_eCall = eNPP_Print;
    m_aArgs[0] = (void*)instance;
    m_aArgs[1] = (void*)platformPrint;
    execute();
}

//--------------------------------------------------------------------------------------------------
NPError MacPluginComm::NPP_SetWindow( NPP instance, NPWindow* window )
{
    DBG_ASSERT( m_aNPPfuncs.setwindow, "### NPP_SetWindow(): null pointer in NPP functions table!" );
    m_eCall = eNPP_SetWindow;
    m_aArgs[0] = (void*)instance;
    m_aArgs[1] = (void*)window;
    return (NPError)execute();
}

//--------------------------------------------------------------------------------------------------
NPError MacPluginComm::NPP_HandleEvent( NPP instance, void* event )
{
    DBG_ASSERT( m_aNPPfuncs.event, "### NPP_HandleEvent(): null pointer in NPP functions table!" );
    m_eCall = eNPP_HandleEvent;
    m_aArgs[0] = (void*)instance;
    m_aArgs[1] = event;
    return (NPError)execute();
}

//--------------------------------------------------------------------------------------------------
void MacPluginComm::NPP_StreamAsFile( NPP instance, NPStream* stream, const char* fname )
{
    DBG_ASSERT( m_aNPPfuncs.asfile, "### NPP_StreamAsFile(): null pointer in NPP functions table!" );
    m_eCall = eNPP_StreamAsFile;
    m_aArgs[0] = (void*)instance;
    m_aArgs[1] = (void*)stream;
    m_aArgs[2] = (void*)fname;
    execute();
}

//--------------------------------------------------------------------------------------------------
void MacPluginComm::NPP_URLNotify( NPP instance, const char* url, NPReason reason, void* notifyData )
{
    DBG_ASSERT( m_aNPPfuncs.urlnotify, "### NPP_URLNotify(): null pointer in NPP functions table!" );
    m_eCall = eNPP_URLNotify;
    m_aArgs[0] = (void*)instance;
    m_aArgs[1] = (void*)url;
    m_aArgs[2] = (void*)(intptr_t)reason;
    m_aArgs[3] = notifyData;
    execute();
}

//--------------------------------------------------------------------------------------------------
int32_t MacPluginComm::NPP_Write( NPP instance, NPStream* stream, int32_t offset, int32_t len, void* buffer )
{
    DBG_ASSERT( m_aNPPfuncs.write, "### NPP_Write(): null pointer in NPP functions table!" );
    m_eCall = eNPP_Write;
    m_aArgs[0] = (void*)instance;
    m_aArgs[1] = (void*)stream;
    m_aArgs[2] = (void*)offset;
    m_aArgs[3] = (void*)len;
    m_aArgs[4] = buffer;
    return (NPError)execute();
}

//--------------------------------------------------------------------------------------------------
int32_t MacPluginComm::NPP_WriteReady( NPP instance, NPStream* stream )
{
    DBG_ASSERT( m_aNPPfuncs.writeready, "### NPP_WriteReady(): null pointer in NPP functions table!" );
    m_eCall = eNPP_WriteReady;
    m_aArgs[0] = (void*)instance;
    m_aArgs[1] = (void*)stream;
    return execute();
}

//--------------------------------------------------------------------------------------------------
NPError MacPluginComm::NPP_GetValue( NPP instance, NPPVariable variable, void *ret_value )
{
    DBG_ASSERT( m_aNPPfuncs.getvalue, "### NPP_GetValue(): null pointer in NPP functions table!" );
    m_eCall = eNPP_GetValue;
    m_aArgs[0] = (void*)instance;
    m_aArgs[1] = (void*)variable;
    m_aArgs[2] = ret_value;
    return (NPError)execute();
}

//--------------------------------------------------------------------------------------------------
NPError MacPluginComm::NPP_SetValue( NPP instance, NPNVariable variable, void *set_value )
{
    DBG_ASSERT( m_aNPPfuncs.setvalue, "### NPP_SetValue(): null pointer in NPP functions table!" );
    m_eCall = eNPP_SetValue;
    m_aArgs[0] = (void*)instance;
    m_aArgs[1] = (void*)variable;
    m_aArgs[2] = set_value;
    return (NPError)execute();
}

//--------------------------------------------------------------------------------------------------
void * MacPluginComm::NPP_GetJavaClass()
{
    OSL_FAIL( "no java class available!" );
    return 0;
}

//--------------------------------------------------------------------------------------------------
NPError MacPluginComm::NPP_Initialize()
{
    return NPERR_NO_ERROR;
}

//--------------------------------------------------------------------------------------------------
void MacPluginComm::NPP_Shutdown()
{
    m_eCall = eNPP_Shutdown;
    execute();
}

//--------------------------------------------------------------------------------------------------
NPError MacPluginComm::NPP_SetWindow( XPlugin_Impl* i_pImpl )
{
    // update window NPWindow from view
    SysPlugData& rPlugData( i_pImpl->getSysPlugData() );

    // update plug view
    NSRect aPlugRect = [rPlugData.m_pParentView frame];
    aPlugRect.origin.x = aPlugRect.origin.y = 0;
    if( ! rPlugData.m_pPlugView )
    {
        rPlugData.m_pPlugView = [[OOoPluginView alloc] initWithInstance: i_pImpl pluginComm: this frame: aPlugRect];
        [rPlugData.m_pParentView addSubview: rPlugData.m_pPlugView];
    }
    else
        [rPlugData.m_pPlugView setFrame: aPlugRect];

    NPWindow* pNPWin = i_pImpl->getNPWindow();
    NSWindow* pWin = [rPlugData.m_pPlugView window];
    NSRect aWinRect = [pWin frame];
    NSRect aBounds = [rPlugData.m_pPlugView frame];
    NSRect aVisibleBounds = [rPlugData.m_pPlugView visibleRect];

    #if OSL_DEBUG_LEVEL > 1
    fprintf( stderr, "visible bounds = %d+%d+%dx%d\n",
             (int)aVisibleBounds.origin.x, (int)aVisibleBounds.origin.y,
             (int)aVisibleBounds.size.width, (int)aVisibleBounds.size.height );
    #endif

    NSPoint aViewOrigin = [rPlugData.m_pPlugView convertPoint: NSZeroPoint toView: nil];
    // save view origin so we can notice movement of the view in drawView
    // in case of a moved view we need to reset the port/context
    rPlugData.m_aLastPlugViewOrigin = aViewOrigin;

    // convert view origin to topdown coordinates
    aViewOrigin.y = aWinRect.size.height - aViewOrigin.y;

    // same for clipping
    NSPoint aClipOrigin = [rPlugData.m_pPlugView convertPoint: aVisibleBounds.origin toView: nil];
    aClipOrigin.y = aWinRect.size.height - aClipOrigin.y;

    #if OSL_DEBUG_LEVEL > 1
    fprintf( stderr, "view origin: %d+%d, clip origin = %d+%d\n",
             (int)aViewOrigin.x, (int)aViewOrigin.y,
             (int)aClipOrigin.x, (int)aClipOrigin.y );
    #endif

    pNPWin->x                = aViewOrigin.x;
    pNPWin->y                = aViewOrigin.y;
    pNPWin->width            = aBounds.size.width;
    pNPWin->height           = aBounds.size.height;
    pNPWin->clipRect.left    = aClipOrigin.x;
    pNPWin->clipRect.top     = aClipOrigin.y;
    pNPWin->clipRect.right   = aClipOrigin.x + aVisibleBounds.size.width;
    pNPWin->clipRect.bottom  = aClipOrigin.y + aVisibleBounds.size.height;

    if( rPlugData.m_nDrawingModel == 1 )
    {
        rPlugData.m_aCGContext.window = reinterpret_cast<WindowRef>([pWin windowRef]);
        pNPWin->window = &rPlugData.m_aCGContext;
        rPlugData.m_aCGContext.context = reinterpret_cast<CGContextRef>([[pWin graphicsContext] graphicsPort]);
        #if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, "window is %p, context is %p\n",
                 rPlugData.m_aCGContext.window, rPlugData.m_aCGContext.context );
        #endif
    }
    else
    {
        rPlugData.m_aNPPort.port = GetWindowPort( reinterpret_cast<WindowRef>([pWin windowRef]) );
        rPlugData.m_aNPPort.portx = aClipOrigin.x;
        rPlugData.m_aNPPort.porty = aClipOrigin.y;
        pNPWin->window = &rPlugData.m_aNPPort;
        #if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, "port is %p at (%d,%d)\n",
                 rPlugData.m_aNPPort.port, (int)rPlugData.m_aNPPort.portx, (int)rPlugData.m_aNPPort.porty );
        #endif
    }

    if( pNPWin->width == 0 || pNPWin->height == 0 || [rPlugData.m_pPlugView isHiddenOrHasHiddenAncestor] )
        rPlugData.m_bSetWindowOnDraw = true;

    NPError nErr = NPP_SetWindow( i_pImpl->getNPPInstance(), i_pImpl->getNPWindow() );

    return nErr;
}

void MacPluginComm::drawView( XPlugin_Impl* i_pImpl )
{
    SysPlugData& rPlugData( i_pImpl->getSysPlugData() );

    // check if the view was moved since the last SetWindow
    NSPoint aViewOrigin = [rPlugData.m_pPlugView convertPoint: NSZeroPoint toView: nil];
    if( rPlugData.m_bSetWindowOnDraw ||
        aViewOrigin.x != rPlugData.m_aLastPlugViewOrigin.x ||
        aViewOrigin.y != rPlugData.m_aLastPlugViewOrigin.y )
    {
        NPP_SetWindow( i_pImpl );
        rPlugData.m_bSetWindowOnDraw = false;
    }

    // send a paint event
    NSWindow* pWin = [rPlugData.m_pPlugView window];
    FakeEventRecord aRec;
    aRec.what       =  updateEvt;
    aRec.message    = (uint32_t)[pWin windowRef];
    this->NPP_HandleEvent( i_pImpl->getNPPInstance(), &aRec );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
