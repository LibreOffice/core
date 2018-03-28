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


#ifndef _OTRANSFERABLE_HXX_
#define _OTRANSFERABLE_HXX_

#include <svpm.h>

#include <cppuhelper/implbase1.hxx>
#include <rtl/ustring.hxx>
#include <rtl/unload.h>
#include <com/sun/star/datatransfer/DataFlavor.hpp>
#include <com/sun/star/datatransfer/XTransferable.hpp>

using namespace cppu;
using namespace osl;
using namespace rtl;
using namespace com::sun::star::io;
using namespace com::sun::star::uno;
using namespace com::sun::star::datatransfer;


// undocumented(?)
#ifndef DC_PREPAREITEM
#define DC_PREPAREITEM   0x0040
#endif

// rendering mechanism
#define DTSHARE_NAME    "\\SHAREMEM\\AOO_DND"
#define DTSHARE_RMF     "<DRM_DTSHARE,DRF_TEXT>"

#define SHAREDMEM_RMF   "<DRM_SHAREDMEM,DRF_TEXT>"

#define OS2FILE_NAME    "AOO_TGT.TMP"
#define OS2FILE_TXTRMF  "<DRM_OS2FILE,DRF_TEXT>"
#define OS2FILE_UNKRMF  "<DRM_OS2FILE,DRF_UNKNOWN>"


class OTransferable: public ::cppu::WeakImplHelper1<XTransferable>
{
    com::sun::star::uno::Sequence<DataFlavor> m_aFlavorList;

private:
    HWND mHwndTarget;
    PDRAGINFO mDragInfo;
    bool removeOnExit;
    char fullpath[CCHMAXPATH];
    void *pDTShare;
    char *pSharedMem;
    enum _DRM { DRM_NULL, DRM_ATOM, DRM_DTSHARE, DRM_OS2FILE, DRM_SHAREDMEM};
    _DRM renderDRM;
    enum _MIMETYPE { MIMETYPE_NULL, MIMETYPE_TEXT, MIMETYPE_FILE};
    _MIMETYPE mimeType;

public:

public:
    OTransferable( HWND hwnd, PDRAGINFO dragInfo);
    ~OTransferable();

    virtual Any SAL_CALL getTransferData( const DataFlavor& aFlavor)
        throw(UnsupportedFlavorException, IOException, RuntimeException);
    virtual Sequence< DataFlavor > SAL_CALL getTransferDataFlavors()
        throw(RuntimeException);
    virtual sal_Bool SAL_CALL isDataFlavorSupported( const DataFlavor& aFlavor)
        throw(RuntimeException);

    bool renderComplete( PDRAGTRANSFER pdxfer);
    bool requestRendering( void);

private:
    bool RequestFileRendering( PDRAGITEM pditem, HWND hwnd, PCSZ pRMF, PCSZ pName);
    bool RenderToOS2File( PDRAGITEM pditem, HWND hwnd);
    bool RenderToDTShare( PDRAGITEM pditem, HWND hwnd);
    bool RenderToSharedMem( PDRAGITEM pditem, HWND hwnd);

};

#endif // _OTRANSFERABLE_HXX_
