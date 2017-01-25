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
#ifndef INCLUDED_SFX2_LNKBASE_HXX
#define INCLUDED_SFX2_LNKBASE_HXX

#include <com/sun/star/io/XInputStream.hpp>
#include <rtl/ustring.hxx>
#include <sal/config.h>
#include <sfx2/dllapi.h>
#include <sfx2/linksrc.hxx>
#include <sot/exchange.hxx>
#include <tools/ref.hxx>
#include <memory>

namespace com { namespace sun { namespace star { namespace uno
{
    class Any;
}}}}

enum class SfxLinkUpdateMode {
    NONE   = 0,
    // Ole2 compatible and persistent
    ALWAYS = 1,
    ONCALL = 3
};

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

struct BaseLink_Impl;

class SFX2_DLLPUBLIC SvBaseLink : public SvRefBase
{
private:
    friend class LinkManager;
    friend class SvLinkSource;

    SvLinkSourceRef         xObj;
    OUString                aLinkName;
    std::unique_ptr<BaseLink_Impl>  pImpl;
    sal_uInt16              nObjType;
    bool                    bVisible : 1;
    bool                    bSynchron : 1;
    bool                    bUseCache : 1;  // for Graphics Links!
    bool                    bWasLastEditOK : 1;

    DECL_LINK( EndEditHdl, const OUString&, void );

    bool                    ExecuteEdit( const OUString& _rNewName );

protected:
    void            SetObjType( sal_uInt16 );

    // Set LinkSourceName without action
    void            SetName( const OUString & rLn );

    ImplBaseLinkData* pImplData;

    bool            m_bIsReadOnly;
    css::uno::Reference<css::io::XInputStream>
                    m_xInputStreamToLoadFrom;

                    SvBaseLink();
                    SvBaseLink( SfxLinkUpdateMode nLinkType, SotClipboardFormatId nContentType );
    virtual         ~SvBaseLink() override;

    void            GetRealObject_( bool bConnect = true );

    SvLinkSource*   GetRealObject()
                    {
                        if( !xObj.is() )
                            GetRealObject_();
                        return xObj.get();
                    }

public:

    virtual void    Closed();

#if defined(_WIN32)
                    SvBaseLink( const OUString& rNm, sal_uInt16 nObjectType,
                                 SvLinkSource* );
#endif

    sal_uInt16          GetObjType() const { return nObjType; }

    void            SetObj( SvLinkSource * pObj );
    SvLinkSource*   GetObj() const  { return xObj.get(); }

    void            SetLinkSourceName( const OUString & rName );
    const OUString& GetLinkSourceName() const { return aLinkName;}

    enum UpdateResult {
        SUCCESS = 0,
        ERROR_GENERAL = 1
    };

    virtual UpdateResult DataChanged(
        const OUString & rMimeType, const css::uno::Any & rValue );

    void                 SetUpdateMode( SfxLinkUpdateMode );
    SfxLinkUpdateMode    GetUpdateMode() const;
    SotClipboardFormatId GetContentType() const;
    void                 SetContentType( SotClipboardFormatId nType );

    LinkManager*          GetLinkManager();
    const LinkManager*    GetLinkManager() const;
    void                  SetLinkManager( LinkManager* _pMgr );

    bool            Update();
    void            Disconnect();

    virtual void    Edit( vcl::Window*, const Link<SvBaseLink&,void>& rEndEditHdl );

    // should the link appear in the dialog? (to the left in the link in the...)
    bool            IsVisible() const           { return bVisible; }
    void            SetVisible( bool bFlag )    { bVisible = bFlag; }
    // should the Link be loaded synchronous or asynchronous?
    bool            IsSynchron() const          { return bSynchron; }
    void            SetSynchron( bool bFlag )   { bSynchron = bFlag; }

    void            SetUseCache( bool bFlag )   { bUseCache = bFlag; }

    void            setStreamToLoadFrom(
                        const css::uno::Reference<css::io::XInputStream>& xInputStream,
                        bool bIsReadOnly )
                            { m_xInputStreamToLoadFrom = xInputStream;
                              m_bIsReadOnly = bIsReadOnly; }
    // #i88291#
    void            clearStreamToLoadFrom();

    inline bool     WasLastEditOK() const       { return bWasLastEditOK; }
    FileDialogHelper & GetInsertFileDialog(const OUString& rFactory) const;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
