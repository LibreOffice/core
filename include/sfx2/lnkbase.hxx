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
#ifndef _LNKBASE_HXX
#define _LNKBASE_HXX

#include <com/sun/star/io/XInputStream.hpp>
#include <rtl/ustring.hxx>
#include <sal/config.h>
#include <sfx2/dllapi.h>
#include <sfx2/linksrc.hxx>
#include <sot/exchange.hxx>
#include <tools/ref.hxx>

namespace com { namespace sun { namespace star { namespace uno
{
    class Any;
}}}}

namespace sfx2
{

struct ImplBaseLinkData;
class LinkManager;
class SvLinkSource;
class FileDialogHelper;

#ifndef OBJECT_DDE_EXTERN
#define OBJECT_INTERN       0x00
//#define   OBJECT_SO_EXTERN    0x01
#define OBJECT_DDE_EXTERN   0x02
#endif

#define OBJECT_CLIENT_SO              0x80 // a Link
#define OBJECT_CLIENT_DDE           0x81
//#define   OBJECT_CLIENT_OLE         0x82 // a Ole-Link
//#define   OBJECT_CLIENT_OLE_CACHE   0x83 // a Ole-Link with SvEmbeddedObject
#define OBJECT_CLIENT_FILE          0x90
#define OBJECT_CLIENT_GRF           0x91
#define OBJECT_CLIENT_OLE           0x92 // embedded link

enum sfxlink {
    // Ole2 compatibel and persistent
    LINKUPDATE_ALWAYS = 1,
    LINKUPDATE_ONCALL = 3,

    LINKUPDATE_END      // dummy!
};

struct BaseLink_Impl;

class SFX2_DLLPUBLIC SvBaseLink : public SvRefBase
{
private:
    friend class LinkManager;
    friend class SvLinkSource;

    SvLinkSourceRef         xObj;
    OUString                aLinkName;
    BaseLink_Impl*          pImpl;
    sal_uInt16                  nObjType;
    sal_Bool                    bVisible : 1;
    sal_Bool                    bSynchron : 1;
    sal_Bool                    bUseCache : 1;  // for Graphics Links!
    sal_Bool                    bWasLastEditOK : 1;

    DECL_LINK( EndEditHdl, OUString* );

    bool                    ExecuteEdit( const OUString& _rNewName );

protected:
    void            SetObjType( sal_uInt16 );

    // Set LinkSourceName without action
    void            SetName( const OUString & rLn );

    ImplBaseLinkData* pImplData;

    sal_Bool            m_bIsReadOnly;
    com::sun::star::uno::Reference<com::sun::star::io::XInputStream>
                        m_xInputStreamToLoadFrom;

                    SvBaseLink();
                    SvBaseLink( sal_uInt16 nLinkType, sal_uIntPtr nContentType = FORMAT_STRING );
    virtual         ~SvBaseLink();

    void            _GetRealObject( sal_Bool bConnect = sal_True );

    SvLinkSource*   GetRealObject()
                    {
                        if( !xObj.Is() )
                            _GetRealObject();
                        return xObj;
                    }

public:
                    TYPEINFO();

    virtual void    Closed();
                    SvBaseLink( const OUString& rNm, sal_uInt16 nObjectType,
                                 SvLinkSource* );

    sal_uInt16          GetObjType() const { return nObjType; }

    void            SetObj( SvLinkSource * pObj );
    SvLinkSource*   GetObj() const  { return xObj; }

    void            SetLinkSourceName( const OUString & rName );
    OUString        GetLinkSourceName() const;

    enum UpdateResult {
        SUCCESS = 0,
        ERROR_GENERAL = 1
    };

    virtual UpdateResult DataChanged(
        const OUString & rMimeType, const ::com::sun::star::uno::Any & rValue );

    void            SetUpdateMode( sal_uInt16 );
    sal_uInt16          GetUpdateMode() const;
    sal_uIntPtr             GetContentType() const;
    sal_Bool            SetContentType( sal_uIntPtr nType );

    LinkManager*          GetLinkManager();
    const LinkManager*    GetLinkManager() const;
    void                    SetLinkManager( LinkManager* _pMgr );

    sal_Bool            Update();
    void            Disconnect();

    // Link impl: DECL_LINK( MyEndDialogHdl, SvBaseLink* ); <= param is this
    virtual void    Edit( Window*, const Link& rEndEditHdl );

    // should the link appear in the dialog? (to the left in the link in the...)
    sal_Bool            IsVisible() const           { return bVisible; }
    void            SetVisible( sal_Bool bFlag )    { bVisible = bFlag; }
    // should the Link be loaded synchronous or asynchronous?
    sal_Bool            IsSynchron() const          { return bSynchron; }
    void            SetSynchron( sal_Bool bFlag )   { bSynchron = bFlag; }

    sal_Bool            IsUseCache() const          { return bUseCache; }
    void            SetUseCache( sal_Bool bFlag )   { bUseCache = bFlag; }

    void            setStreamToLoadFrom(
                        const com::sun::star::uno::Reference<com::sun::star::io::XInputStream>& xInputStream,
                        sal_Bool bIsReadOnly )
                            { m_xInputStreamToLoadFrom = xInputStream;
                              m_bIsReadOnly = bIsReadOnly; }
    // #i88291#
    void            clearStreamToLoadFrom();

    inline sal_Bool         WasLastEditOK() const       { return bWasLastEditOK; }
    FileDialogHelper & GetInsertFileDialog(const OUString& rFactory) const;
};

SV_DECL_IMPL_REF(SvBaseLink);

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
