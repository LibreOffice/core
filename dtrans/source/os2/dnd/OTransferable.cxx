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

#define INCL_DOSERRORS

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <rtl/string.hxx>

#include "OTransferable.hxx"
#include "globals.hxx"


OTransferable::OTransferable( HWND hwndTarget, PDRAGINFO dragInfo)
    : m_aFlavorList( 1),
      mHwndTarget( hwndTarget),
      mDragInfo( dragInfo),
      removeOnExit( false),
      pDTShare( NULL),
      renderDRM( DRM_NULL),
      mimeType( MIMETYPE_NULL)
{
    USHORT cItems;
    PDRAGITEM dragItem;
    PSZ pNativeRMF;

    strcpy( fullpath, "");

    cItems = DrgQueryDragitemCount(dragInfo);
    if (cItems > 1) {
        debug_printf("OTransferable::OTransferable multiple drop not supported");
        return;
    }

    ULONG ulLength;
    PSZ pBuffer;

    // get 1st item
    dragItem = DrgQueryDragitemPtr(dragInfo, 0);

    // dump true type
    ulLength = DrgQueryTrueTypeLen( dragItem) + 1;
    pBuffer = (PSZ) malloc( ulLength);
    DrgQueryTrueType( dragItem, ulLength, pBuffer);
    debug_printf("DrgQueryTrueType %s", pBuffer);
    free( pBuffer);

    // get native RMF format
    ulLength = DrgQueryNativeRMFLen( dragItem) + 1;
    pNativeRMF = (PSZ) malloc( ulLength);
    DrgQueryNativeRMF( dragItem, ulLength, pNativeRMF);
    debug_printf("OTransferable::OTransferable DrgQueryNativeRMF %s", pNativeRMF);

    debug_printf("OTransferable::OTransferable DrgVerifyRMF DRM_ATOM %d", DrgVerifyRMF( dragItem, "DRM_ATOM", NULL));
    debug_printf("OTransferable::OTransferable DrgVerifyRMF DRM_OS2FILE %d", DrgVerifyRMF( dragItem, "DRM_OS2FILE", NULL));
    debug_printf("OTransferable::OTransferable DrgVerifyRMF DRM_PRINTOBJECT %d", DrgVerifyRMF( dragItem, "DRM_PRINTOBJECT", NULL));
    debug_printf("OTransferable::OTransferable DrgVerifyRMF DRM_SHAREDMEM %d", DrgVerifyRMF( dragItem, "DRM_SHAREDMEM", NULL));
    debug_printf("OTransferable::OTransferable DrgVerifyRMF DRM_DTSHARE %d", DrgVerifyRMF( dragItem, "DRM_DTSHARE", NULL));

    DataFlavor df;

    if (strstr( pNativeRMF, "<DRM_ATOM") != 0
            || strstr( pNativeRMF, "<DRM_DTSHARE") != 0
            || strstr( pNativeRMF, "<DRM_SHAREDMEM") != 0) {

        df.MimeType = OUString::createFromAscii( "text/plain;charset=utf-16" );
        df.DataType = getCppuType( static_cast<rtl::OUString*>(0));
        m_aFlavorList[0] = df;
        mimeType = MIMETYPE_TEXT;

    } else if (strstr( pNativeRMF, "<DRM_OS2FILE") != 0) {

        df.MimeType = OUString::createFromAscii(
                    "application/x-openoffice-file;windows_formatname=\"FileName\"");
        df.DataType = getCppuType( static_cast<OUString*>(0));
        m_aFlavorList[0] = df;
        mimeType = MIMETYPE_FILE;

    } else {

        mimeType = MIMETYPE_NULL;
        debug_printf("OTransferable::OTransferable UNKNOWN native RMF");

    }

    free( pNativeRMF);

}

OTransferable::~OTransferable()
{
    if (removeOnExit) {
        int rc;
        rc = unlink( fullpath);
        debug_printf( "OTransferable::~OTransferable unlink rc=%d", rc);
    }
}


//
// a generic request dispatcher
//
bool OTransferable::RequestFileRendering( PDRAGITEM pditem, HWND hwnd,
                                          PCSZ pRMF, PCSZ pName)
{
    PDRAGTRANSFER pdxfer;

    pdxfer = DrgAllocDragtransfer( 1);
    if (!pdxfer)
        return true;

    pdxfer->cb = sizeof(DRAGTRANSFER);
    pdxfer->hwndClient = hwnd;
    pdxfer->pditem = pditem;
    pdxfer->hstrSelectedRMF = DrgAddStrHandle( pRMF);
    pdxfer->hstrRenderToName = 0;
    pdxfer->ulTargetInfo = pditem->ulItemID;
    pdxfer->usOperation = (USHORT)DO_COPY;
    pdxfer->fsReply = 0;

    // send the msg before setting a render-to name
    if (pditem->fsControl & DC_PREPAREITEM)
        DrgSendTransferMsg( pditem->hwndItem, DM_RENDERPREPARE, (MPARAM)pdxfer, 0);

    if (pName)
        pdxfer->hstrRenderToName = DrgAddStrHandle( pName);
    else
        pdxfer->hstrRenderToName = 0;

    // send the msg after setting a render-to name
    if ((pditem->fsControl & (DC_PREPARE | DC_PREPAREITEM)) == DC_PREPARE)
        DrgSendTransferMsg( pditem->hwndItem, DM_RENDERPREPARE, (MPARAM)pdxfer, 0);

    // ask the source to render the selected item
    if (!DrgSendTransferMsg( pditem->hwndItem, DM_RENDER, (MPARAM)pdxfer, 0))
        return true;

    return false;
}

// currently, the same filename is used for every render request;
// it is deleted when the drag session ends
//
bool OTransferable::RenderToOS2File( PDRAGITEM pditem, HWND hwnd)
{
    bool rv = true;

    const char * pszRMF;
    if (DrgVerifyRMF(pditem, "DRM_OS2FILE", "DRF_TEXT"))
        pszRMF = OS2FILE_TXTRMF;
    else
        pszRMF = OS2FILE_UNKRMF;

    // create temp name
    strcpy( fullpath, tempnam( NULL, "AOO"));
    debug_printf("OTransferable::RenderToOS2File to %s", fullpath);

    rv = RequestFileRendering( pditem, hwnd, pszRMF, fullpath);

    return rv;
}

// DTShare uses 1mb of uncommitted named-shared memory
// (next time I'll do it differently - rw)
//
bool OTransferable::RenderToDTShare( PDRAGITEM pditem, HWND hwnd)
{
    bool rv;

    APIRET rc = DosAllocSharedMem( &pDTShare, DTSHARE_NAME, 0x100000,
                                   PAG_WRITE | PAG_READ | OBJ_ANY);
    if (rc != NO_ERROR &&
            rc != ERROR_ALREADY_EXISTS) { // Did the kernel handle OBJ_ANY?
        // Try again without OBJ_ANY and if the first failure was not caused
        // by OBJ_ANY then we will get the same failure, else we have taken
        // care of pre-FP13 systems where the kernel couldn't handle it.
        rc = DosAllocSharedMem( &pDTShare, DTSHARE_NAME, 0x100000,
                                PAG_WRITE | PAG_READ);
    }

    if (rc == ERROR_ALREADY_EXISTS)
        rc = DosGetNamedSharedMem( &pDTShare, DTSHARE_NAME,
                                   PAG_WRITE | PAG_READ);
    if (rc)
        rv = true; // error
    else
        rv = RequestFileRendering( pditem, hwnd, DTSHARE_RMF, DTSHARE_NAME);

    return rv;
}

// SharedMem rendering, memory is allocated by source window
//
bool OTransferable::RenderToSharedMem( PDRAGITEM pditem, HWND hwnd)
{
    bool rv;

    rv = RequestFileRendering( pditem, hwnd, SHAREDMEM_RMF, NULL);

    return rv;
}

bool OTransferable::requestRendering( void)
{
    char path[CCHMAXPATH];
    char file[CCHMAXPATH];
    PDRAGITEM dragItem;

    // unknown rendering
    renderDRM = DRM_NULL;

    // only 1st item supported
    dragItem = DrgQueryDragitemPtr( mDragInfo, 0);

    // check if we already have all necessary fields or a rendering
    // request must be sent to source window

    switch( mimeType) {
    case MIMETYPE_NULL:
        debug_printf("OTransferable::requestRendering INTERNAL ERROR, mimetype undef");
        break;

    case MIMETYPE_FILE:
        if (DrgVerifyRMF( dragItem, "DRM_OS2FILE", NULL)
                && dragItem->hstrSourceName == NULLHANDLE) {

            // if hstrSourceName is NULL we need to ask source for rendering
            bool rv;
            debug_printf("OTransferable::requestRendering request rendering");
            rv = RenderToOS2File( dragItem, mHwndTarget);
            debug_printf("OTransferable::requestRendering requested rendering rv=%d", rv);
            renderDRM = DRM_OS2FILE;

            // notify rendering request ongoing
            return true;

        } else if (DrgVerifyRMF( dragItem, "DRM_OS2FILE", NULL)) {

            // we have hstrSourceName, no need for rendering,
            // we already have enough data for rendering path now

            // get full path
            DrgQueryStrName(dragItem->hstrContainerName, sizeof(path), path);
            debug_printf("OTransferable::getTransferData hstrSourceName %x", dragItem->hstrSourceName);
            debug_printf("OTransferable::getTransferData hstrTargetName %x", dragItem->hstrTargetName);
            DrgQueryStrName(dragItem->hstrSourceName, sizeof(file), file);
            sprintf( fullpath, "%s%s", path, file);
            debug_printf("OTransferable::getTransferData fullpath %s", fullpath);
            renderDRM = DRM_OS2FILE;

        } else {
            debug_printf("OTransferable::requestRendering UNKNOWN request for FILE mimetype");
        }
        break;

    case MIMETYPE_TEXT:
        if (DrgVerifyRMF( dragItem, "DRM_ATOM", NULL)) {

            DrgQueryStrName(dragItem->ulItemID, sizeof(fullpath), fullpath);
            debug_printf("OTransferable::requestRendering DRM_ATOM '%s'", fullpath);
            renderDRM = DRM_ATOM;

            // no request rendering necessary
            return false;

        } else if (DrgVerifyRMF( dragItem, "DRM_DTSHARE", NULL)) {

            bool rv;
            debug_printf("OTransferable::requestRendering request DRM_DTSHARE rendering");
            rv = RenderToDTShare( dragItem, mHwndTarget);
            debug_printf("OTransferable::requestRendering requested DRM_DTSHARE rendering rv=%d", rv);
            renderDRM = DRM_DTSHARE;

            // notify rendering request ongoing
            return true;

        } else if (DrgVerifyRMF( dragItem, "DRM_SHAREDMEM", NULL)) {

            bool rv;
            debug_printf("OTransferable::requestRendering request DRM_SHAREDMEM rendering");
            rv = RenderToSharedMem( dragItem, mHwndTarget);
            debug_printf("OTransferable::requestRendering requested DRM_SHAREDMEM rendering rv=%d", rv);
            renderDRM = DRM_SHAREDMEM;

            // notify rendering request ongoing
            return true;

        } else {
            debug_printf("OTransferable::requestRendering UNKNOWN request for TEXT mimetype");
        }
        break;
    }

    // request rendering not necessary
    return false;

}

//
// AOO window received DM_RENDERCOMPLETE message
//
bool OTransferable::renderComplete( PDRAGTRANSFER pdxfer)
{
    switch( renderDRM) {
    case DRM_NULL:
        // already handled in requestRendering()
        break;
    case DRM_ATOM:
        // set full path from source rendered name string
        DrgQueryStrName( pdxfer->hstrRenderToName, sizeof(fullpath), fullpath);
        debug_printf("OTransferable::setDragTransfer fullpath %s", fullpath);
        break;
    case DRM_DTSHARE:
        // handled in getTransferData()
        break;
    case DRM_SHAREDMEM:
        // save pointer
        pSharedMem = (char *) pdxfer->hstrRenderToName;
        // extraction handled in getTransferData()
        break;
    case DRM_OS2FILE:
        // we already know the path, no need to use hstrRenderToName
        debug_printf("OTransferable::setDragTransfer fullpath %s", fullpath);
        // remove tmp file on destruction
        removeOnExit = true;
        break;
    }

    // send success to source window
    DrgSendTransferMsg( pdxfer->hwndClient, DM_ENDCONVERSATION,
                        (MPARAM) pdxfer->ulTargetInfo,
                        (MPARAM) DMFL_TARGETSUCCESSFUL);

    // free resources
    DrgDeleteStrHandle( pdxfer->hstrSelectedRMF);
    DrgDeleteStrHandle( pdxfer->hstrRenderToName);
    DrgFreeDragtransfer( pdxfer);

    return false;
}

Any SAL_CALL OTransferable::getTransferData( const DataFlavor& df)
throw(UnsupportedFlavorException, IOException, RuntimeException)
{
    OUString m_aData;
    char * pszText = 0;
    int pszLen;
    ULONG size = ~0;
    ULONG flags = 0;
    APIRET rc;
    bool renderOk = false;

    debug_printf("OTransferable::getTransferData MimeType %s",
                 ::rtl::OUStringToOString( df.MimeType, RTL_TEXTENCODING_UTF8 ).getStr());

    // handle shared memory cases
    switch( renderDRM) {
    case DRM_DTSHARE:

        pszLen = ((ULONG*)pDTShare)[0];
        pszText = (char*) malloc( pszLen + 1);
        if (pszText) {
            strcpy(pszText, &((char*)pDTShare)[sizeof(ULONG)] );
        }
        // using DosGetNamedSharedMem() on memory we allocated appears
        // to increment its usage ctr, so we have to free it 2x
        DosFreeMem(pDTShare);
        DosFreeMem(pDTShare);
        // reset pointer
        pDTShare = NULL;

        // prepare data for AOO
        m_aData = OUString( pszText, pszLen, RTL_TEXTENCODING_UTF8);
        break;

    case DRM_SHAREDMEM:
        rc = DosQueryMem((PVOID) pSharedMem, &size, &flags);
        renderOk = rc == 0;
        if (renderOk) {
            renderOk = (flags & (PAG_COMMIT | PAG_READ | PAG_BASE)) ==
                       (PAG_COMMIT | PAG_READ | PAG_BASE);
        }
        if (renderOk) {
            ULONG realSize = *(ULONG *) pSharedMem;
            renderOk = realSize <= size;
            if (renderOk) {
                // prepare data for AOO
                m_aData = OUString( pSharedMem + sizeof(ULONG), realSize, RTL_TEXTENCODING_UTF8);
            }
        }
        // free memory only if it is given by another process,
        // otherwise DefaultDragWorker will free it
        if (flags & PAG_SHARED)
            DosFreeMem((PVOID) pSharedMem);
        break;

    case DRM_ATOM:
    case DRM_OS2FILE:
        // data is in fullpath string
        // prepare data for AOO
        m_aData = OUString( fullpath, strlen(fullpath), RTL_TEXTENCODING_UTF8);
        break;

    default:
        debug_printf( "OTransferable::getTransferData unsupported DRM_* type %d",
                      renderDRM);
        break;
    }

    // return data
    return makeAny( m_aData );
}

// -----------------------------------------------------------------------

Sequence< DataFlavor > SAL_CALL OTransferable::getTransferDataFlavors(  )
throw(RuntimeException)
{
    return m_aFlavorList;
}

// -----------------------------------------------------------------------

sal_Bool SAL_CALL OTransferable::isDataFlavorSupported( const DataFlavor& )
throw(RuntimeException)
{
    return sal_True;
}
