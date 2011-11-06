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


#ifndef _FILEOBJ_HXX
#define _FILEOBJ_HXX

#include <tools/string.hxx>
#include <sfx2/linksrc.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/linkmgr.hxx>

class Graphic;
struct Impl_DownLoadData;
namespace sfx2 { class FileDialogHelper; }

class SvFileObject : public sfx2::SvLinkSource
{
    String              sFileNm;
    String              sFilter;
    String              sReferer;
    Link                aEndEditLink;
    SfxMediumRef        xMed;
    Impl_DownLoadData*  pDownLoadData;
    Window*             pOldParent;

    sal_uInt8 nType;

    sal_Bool bLoadAgain : 1;
    sal_Bool bSynchron : 1;
    sal_Bool bLoadError : 1;
    sal_Bool bWaitForData : 1;
    sal_Bool bInNewData : 1;
    sal_Bool bDataReady : 1;
    sal_Bool bMedUseCache : 1;
    sal_Bool bNativFormat : 1;
    sal_Bool bClearMedium : 1;
    sal_Bool bStateChangeCalled : 1;
    sal_Bool bInCallDownLoad : 1;

    sal_Bool GetGraphic_Impl( Graphic&, SvStream* pStream = 0 );
    sal_Bool LoadFile_Impl();
    void SendStateChg_Impl( sfx2::LinkManager::LinkState nState );

    DECL_STATIC_LINK( SvFileObject, DelMedium_Impl, SfxMediumRef* );
    DECL_STATIC_LINK( SvFileObject, LoadGrfReady_Impl, void* );
    DECL_STATIC_LINK( SvFileObject, LoadGrfNewData_Impl, void* );
    DECL_LINK( DialogClosedHdl, sfx2::FileDialogHelper* );

protected:
    virtual ~SvFileObject();

public:
    SvFileObject();

    virtual sal_Bool GetData( ::com::sun::star::uno::Any & rData /*out param*/,
                            const String & rMimeType,
                            sal_Bool bSynchron = sal_False );

    virtual sal_Bool    Connect( sfx2::SvBaseLink* );
    virtual void    Edit( Window *, sfx2::SvBaseLink *, const Link& rEndEditHdl );

    // erfrage ob das man direkt auf die Daten zugreifen kann oder ob das
    // erst angestossen werden muss
    virtual sal_Bool IsPending() const;
    virtual sal_Bool IsDataComplete() const;

    void CancelTransfers();
};


#endif

