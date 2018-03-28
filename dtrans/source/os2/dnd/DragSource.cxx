/**************************************************************
*
* Licensed to the Apache Software Foundation (ASF) under one
* or more contributor license agreements.  See the NOTICE file
* distributed with this work for additional information
* regarding copyright ownership.  The ASF licenses this file
* to you under the Apache License, Version 2.0 (the
* "License"); you may not use this file except in compliance
* with the License.  You may obtain a copy of the License at
*
*   http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing,
* software distributed under the License is distributed on an
* "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
* KIND, either express or implied.  See the License for the
* specific language governing permissions and limitations
* under the License.
*
*************************************************************/


// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dtrans.hxx"
#include <com/sun/star/datatransfer/dnd/DNDConstants.hpp>
#include <com/sun/star/datatransfer/XTransferable.hpp>
#include <com/sun/star/awt/MouseButton.hpp>
#include <com/sun/star/awt/MouseEvent.hpp>

#include <vcl/window.hxx>

#include "DragSource.hxx"
#include "globals.hxx"

using namespace com::sun::star::datatransfer::dnd::DNDConstants;

// for AOO internal D&D we provide the Transferable
Reference<XTransferable> DragSource::g_XTransferable;
// the handle of the window starting the drag
HWND DragSource::g_DragSourceHwnd = NULLHANDLE;


DragSource::DragSource( const Reference<XMultiServiceFactory>& sf):
    WeakComponentImplHelper4< XDragSource,
            XInitialization,
            XDragSourceContext,
            XServiceInfo >(m_aMutex),
    m_serviceFactory(sf),
    pSourceDraginfo(NULL),
    pSharedMem(NULL),
    pDTShareMem(NULL)
{
    g_moduleCount.modCnt.acquire( &g_moduleCount.modCnt);
    debug_printf("DragSource::DragSource");
}

DragSource::~DragSource()
{
    g_moduleCount.modCnt.release( &g_moduleCount.modCnt);
    debug_printf("DragSource::~DragSource");
}

// XInitialization

//
// aArguments contains a machine id
//
void SAL_CALL DragSource::initialize( const Sequence< Any >& aArguments )
throw(Exception, RuntimeException)
{
    if (aArguments.getLength() < 2) {
        throw Exception(OUString(RTL_CONSTASCII_USTRINGPARAM("DragSource::initialize: Not enough parameter.")),
                        static_cast<OWeakObject*>(this));
    }

    m_hWnd = *(HWND*)aArguments[1].getValue();
    debug_printf("DragSource::initialize hwnd %x", m_hWnd);
    // init done in DropTarget, window is already subclassed
    SetWindowDragSourcePtr( m_hWnd, this);
}

void SAL_CALL DragSource::disposing()
{
    debug_printf("DragSource::disposing hwnd %x", m_hWnd);
    SetWindowDragSourcePtr( m_hWnd, 0);
}

// XDragSource
sal_Bool SAL_CALL DragSource::isDragImageSupported(  )
throw(RuntimeException)
{
    return 0;
}

sal_Int32 SAL_CALL DragSource::getDefaultCursor( sal_Int8 /*dragAction*/ )
throw( IllegalArgumentException, RuntimeException)
{
    return 0;
}

//
// Notifies the XDragSourceListener by calling dragDropEnd
//
void SAL_CALL DragSource::startDrag(
    const DragGestureEvent& /* trigger */,
    sal_Int8 sourceActions,
    sal_Int32 /* cursor */,
    sal_Int32 /* image */,
    const Reference<XTransferable >& trans,
    const Reference<XDragSourceListener >& listener ) throw( RuntimeException)
{
    debug_printf("DragSource::startDrag hwnd %x, sourceActions %d",
                 m_hWnd, sourceActions);

    DRAGITEM    dragItem;
    DRAGIMAGE   dimg;
    HSTR        hstrType, hstrRMF;
    HWND        hwndTarget;

    // store transferable for internal AOO d&d operations
    g_XTransferable = trans;
    // store drag source window handle in a global field since we can
    // start only one drag operation at time
    g_DragSourceHwnd = m_hWnd;

#if 1
    // dump data flavours
    Sequence<DataFlavor> seq = g_XTransferable->getTransferDataFlavors();
    for( int i=0; i<seq.getLength(); i++) {
        DataFlavor df = seq[i];
        debug_printf("DragSource::startDrag mimetype %s",
                     ::rtl::OUStringToOString( df.MimeType, RTL_TEXTENCODING_UTF8 ).getStr());
    }
#endif

    dragSourceListener = listener;
    debug_printf("DragSource::startDrag dragSourceListener 0x%x", dragSourceListener.get());

    // allocate OS/2 specific resources
    pSourceDraginfo = DrgAllocDraginfo(1);
    pSourceDraginfo->hwndSource = m_hWnd;

    hstrType = DrgAddStrHandle( DRT_TEXT);
    hstrRMF = DrgAddStrHandle( "<DRM_DTSHARE,DRF_TEXT>,<DRM_SHAREDMEM,DRF_TEXT>,<DRM_OS2FILE,DRF_TEXT>");

    dragItem.hwndItem          = m_hWnd;
    dragItem.ulItemID          = 1;
    dragItem.hstrType          = hstrType;
    dragItem.hstrRMF           = hstrRMF;
    dragItem.hstrContainerName = NULL; // force rendering
    dragItem.hstrSourceName    = NULL;
    dragItem.hstrTargetName    = NULL;
    dragItem.fsControl         = 0;
    dragItem.fsSupportedOps    = DO_COPYABLE | DO_MOVEABLE | DO_LINKABLE;

    dimg.cb       = sizeof(dimg);
    dimg.hImage   = WinQuerySysPointer( HWND_DESKTOP, SPTR_FILE, FALSE);
    dimg.fl       = DRG_ICON | DRG_TRANSPARENT;
    dimg.cxOffset = 0;
    dimg.cyOffset = 0;

    DrgSetDragitem( pSourceDraginfo, &dragItem, sizeof(dragItem), 0);

    // start PM dragging
    hwndTarget = DrgDrag( m_hWnd, pSourceDraginfo, &dimg, 1L, VK_BUTTON2, NULL);
    if (hwndTarget == NULLHANDLE) {
        // post a dummy message to ourselves to allow freeing resources
        // (yes, we could call endConversation() directly)
        WinPostMsg( m_hWnd, DM_AOO_ENDCONVERSATION,
                    0, MPFROMSHORT(DMFL_TARGETFAIL));
    }

    debug_printf("DragSource::startDrag hwndTarget %x", hwndTarget);

}

// XDragSourceContext
sal_Int32 SAL_CALL DragSource::getCurrentCursor(  )
throw( RuntimeException)
{
    return 0;
}

void SAL_CALL DragSource::setCursor( sal_Int32 /*cursorId*/ )
throw( RuntimeException)
{
}

void SAL_CALL DragSource::setImage( sal_Int32 /*imageId*/ )
throw( RuntimeException)
{
}

void SAL_CALL DragSource::transferablesFlavorsChanged(  )
throw( RuntimeException)
{
}


//
// XServiceInfo
//
OUString SAL_CALL DragSource::getImplementationName(  ) throw (RuntimeException)
{
    return OUString(RTL_CONSTASCII_USTRINGPARAM(OS2_DNDSOURCE_IMPL_NAME));;
}

sal_Bool SAL_CALL DragSource::supportsService( const OUString& ServiceName ) throw (RuntimeException)
{
    if( ServiceName.equals(OUString(RTL_CONSTASCII_USTRINGPARAM(OS2_DNDSOURCE_SERVICE_NAME ))))
        return sal_True;
    return sal_False;
}

Sequence< OUString > SAL_CALL DragSource::getSupportedServiceNames(  ) throw (RuntimeException)
{
    OUString names[1]= {OUString(RTL_CONSTASCII_USTRINGPARAM(OS2_DNDSOURCE_SERVICE_NAME))};
    return Sequence<OUString>(names, 1);
}


//
// OS/2 specific platform code
//


//
// AOO as source window vs external drop target.
//
// this is sent when drop target requests the render
//
MRESULT DragSource::render( PDRAGTRANSFER pdxfer)
{
    APIRET rc;
    ULONG ulLength;
    PSZ pRMF;
    bool rv = false;

    ulLength = DrgQueryStrNameLen( pdxfer->hstrSelectedRMF) + 1;
    pRMF = (PSZ) malloc( ulLength);
    DrgQueryStrName( pdxfer->hstrSelectedRMF, ulLength, pRMF);
    debug_printf("DragSource::render RMF:%s", pRMF);
    debug_printf("DragSource::render hstrRenderToName:%x", pdxfer->hstrRenderToName);

    if (strstr( pRMF, "<DRM_DTSHARE") != 0) {

        char DTShareName[CCHMAXPATH];

        DataFlavor df;
        df.MimeType = OUString::createFromAscii( "text/plain;charset=utf-16" );
        df.DataType = getCppuType( static_cast<OUString*>(0));

        Any aAny = DragSource::g_XTransferable->getTransferData(df);
        OUString utext;
        aAny >>= utext;
        OString text = ::rtl::OUStringToOString( utext, RTL_TEXTENCODING_UTF8).getStr();
        debug_printf("DragSource::render text:%s", text.getStr());
        debug_printf("DragSource::render text.getLength():%d", text.getLength());

        DrgQueryStrName( pdxfer->hstrRenderToName, sizeof(DTShareName),
                         DTShareName);
        debug_printf("DragSource::render hstrRenderToName:%s", DTShareName);

        rc = DosGetNamedSharedMem( (PPVOID) &pDTShareMem, DTShareName,
                                   PAG_WRITE | PAG_READ);
        debug_printf("DragSource::render DosGetNamedSharedMem rc:%d", rc);
        debug_printf("DragSource::render DosGetNamedSharedMem pSharedMem:%x", pSharedMem);

        // the memory is still not committed
        rc = DosSetMem( pDTShareMem, text.getLength()+1, PAG_DEFAULT | PAG_COMMIT);
        debug_printf("DragSource::render DosSetMem rc:%d", rc);

        // first ULONG is text length
        *(ULONG*) pDTShareMem = text.getLength();
        // text data from 2nd ULONG
        memcpy( pDTShareMem + sizeof(ULONG), text.getStr(),
                text.getLength()+1);

        // return success
        rv = true;

    } else if (strstr( pRMF, "<DRM_SHAREDMEM") != 0) {

        PID pid;
        TID tid;
        DataFlavor df;
        df.MimeType = OUString::createFromAscii( "text/plain;charset=utf-16" );
        df.DataType = getCppuType( static_cast<OUString*>(0));

        Any aAny = DragSource::g_XTransferable->getTransferData(df);
        OUString utext;
        aAny >>= utext;
        OString text = ::rtl::OUStringToOString( utext, RTL_TEXTENCODING_UTF8).getStr();
        debug_printf("DragSource::render text:%s", text.getStr());
        debug_printf("DragSource::render text.getLength():%d", text.getLength());

        rc = DosAllocSharedMem( (PPVOID) &pSharedMem, NULL,
                                       text.getLength()+sizeof(ULONG)+1,
                                       OBJ_GIVEABLE | PAG_COMMIT |
                                       PAG_WRITE | PAG_READ /*| OBJ_ANY*/);

        rc = WinQueryWindowProcess( pdxfer->hwndClient, &pid, &tid);
        rc = DosGiveSharedMem( pSharedMem, pid, PAG_READ);

        debug_printf("DragSource::render rc:%d", rc);
        *(ULONG *) pSharedMem = text.getLength();
        memcpy( pSharedMem + sizeof(ULONG), text.getStr(), text.getLength()+1);
        pdxfer->hstrRenderToName = (HSTR) pSharedMem;

        // return success
        rv = true;

    } else if (strstr( pRMF, "<DRM_OS2FILE") != 0) {

        char fileName[CCHMAXPATH];

        DataFlavor df;
        df.MimeType = OUString::createFromAscii( "text/plain;charset=utf-16" );
        df.DataType = getCppuType( static_cast<OUString*>(0));

        Any aAny = DragSource::g_XTransferable->getTransferData(df);
        OUString utext;
        aAny >>= utext;
        OString text = ::rtl::OUStringToOString( utext, RTL_TEXTENCODING_UTF8).getStr();
        debug_printf("DragSource::render text:%s", text.getStr());
        debug_printf("DragSource::render text.getLength():%d", text.getLength());

        DrgQueryStrName( pdxfer->hstrRenderToName, sizeof(fileName), fileName);
        debug_printf("DragSource::render hstrRenderToName:%s", fileName);

        // write data to target file
        FILE* tmp = fopen( fileName, "wb");
        if (tmp) {
            fwrite( text.getStr(), 1, text.getLength(), tmp);
            fclose( tmp);
            rv = true;
        }

    } else {

        debug_printf("DragSource::render INTERNAL ERROR unknown type");

    }

    free( pRMF);

    // post rendered data
    int renderOK = (rv==true) ? DMFL_RENDEROK : DMFL_RENDERFAIL;
    debug_printf("DragSource::render render:%d", renderOK);
    rc = DrgPostTransferMsg( pdxfer->hwndClient, DM_RENDERCOMPLETE, pdxfer,
                        renderOK, 0, FALSE);
    debug_printf("DragSource::render DrgPostTransferMsg:%d", rc);

    // free resources
    DrgFreeDragtransfer(pdxfer);

    return (MRESULT) rv;
}

//
// AOO as source window vs external drop target.
//
// this is sent when external drop target requests the render
//
MRESULT DragSource::endConversation( ULONG itemID, ULONG flags)
{
    sal_Bool success = ((flags==DMFL_TARGETSUCCESSFUL) ? true : false);
    sal_Int8 effect = ACTION_NONE;

    debug_printf("DragSource::endConversation itemID %d, flags %d", itemID, flags);

    if (pDTShareMem)
        DosFreeMem( pDTShareMem);
    pDTShareMem = NULL;
    if (pSharedMem)
        DosFreeMem( pSharedMem);
    pSharedMem = NULL;

    if (pSourceDraginfo) {
        // query which kind of operation the target did with our data
        if (success == true)
            effect = SystemToOfficeDragActions( pSourceDraginfo->usOperation);
        debug_printf("DragSource::endConversation usOperation 0x%x", pSourceDraginfo->usOperation);
        DrgDeleteDraginfoStrHandles( pSourceDraginfo);
        DrgFreeDraginfo( pSourceDraginfo);
    }
    pSourceDraginfo = NULL;

    // terminate AOO drag
    DragSourceDropEvent de(static_cast<OWeakObject*>(this),
                           static_cast<XDragSourceContext*>(this),
                           static_cast<XDragSource*>(this),
                           effect,
                           success);
    dragSourceListener->dragDropEnd( de);

    // clear globals
    g_XTransferable = Reference<XTransferable>();
    g_DragSourceHwnd = NULLHANDLE;
    dragSourceListener = Reference<XDragSourceListener>();

    // Reserved value, should be 0
    return 0;
}
