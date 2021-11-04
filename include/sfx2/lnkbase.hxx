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

#pragma once

#include <rtl/ustring.hxx>
#include <sal/config.h>
#include <sfx2/dllapi.h>
#include <sfx2/linksrc.hxx>
#include <sot/formats.hxx>
#include <tools/ref.hxx>
#include <tools/link.hxx>
#include <memory>

namespace com::sun::star::uno
{
    class Any;
}

namespace com::sun::star::io { class XInputStream; }

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
class FileDialogHelper;

enum class SvBaseLinkObjectType {
    Internal      = 0x00,
    DdeExternal   = 0x02,
    ClientSo      = 0x80, // a Link
    ClientDde     = 0x81,
    ClientFile    = 0x90,
    ClientGraphic = 0x91,
    ClientOle     = 0x92 // embedded link
};

constexpr bool isClientType(SvBaseLinkObjectType t)
{
    return static_cast<int>(t) & static_cast<int>(SvBaseLinkObjectType::ClientSo);
}
constexpr bool isClientFileType(SvBaseLinkObjectType t)
{
    auto check = static_cast<int>(SvBaseLinkObjectType::ClientFile);
    return (static_cast<int>(t) & check) == check;
}

class SFX2_DLLPUBLIC SvBaseLink : public SvRefBase
{
private:
    friend class LinkManager;
    friend class SvLinkSource;

    Link<SvBaseLink&,void>  m_aEndEditLink;
    LinkManager*            m_pLinkMgr;
    weld::Window*           m_pParentWin;
    std::unique_ptr<FileDialogHelper>
                            m_pFileDlg;
    SvLinkSourceRef         xObj;
    OUString                aLinkName;
    std::unique_ptr<ImplBaseLinkData> pImplData;
    SvBaseLinkObjectType    mnObjType;
    bool                    bVisible : 1;
    bool                    bSynchron : 1;
    bool                    bWasLastEditOK : 1;
    bool                    m_bIsConnect : 1;

    DECL_DLLPRIVATE_LINK( EndEditHdl, const OUString&, void );

    bool                    ExecuteEdit( const OUString& _rNewName );

protected:
    void            SetObjType( SvBaseLinkObjectType );

    // Set LinkSourceName without action
    void            SetName( const OUString & rLn );

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
                    SvBaseLink( const OUString& rNm, SvBaseLinkObjectType nObjectType,
                                 SvLinkSource* );
#endif

    SvBaseLinkObjectType GetObjType() const { return mnObjType; }

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

    virtual void    Edit(weld::Window*, const Link<SvBaseLink&,void>& rEndEditHdl);

    // should the link appear in the dialog? (to the left in the link in the...)
    bool            IsVisible() const           { return bVisible; }
    void            SetVisible( bool bFlag )    { bVisible = bFlag; }
    // should the Link be loaded synchronous or asynchronous?
    bool            IsSynchron() const          { return bSynchron; }
    void            SetSynchron( bool bFlag )   { bSynchron = bFlag; }

    void            setStreamToLoadFrom(
                        const css::uno::Reference<css::io::XInputStream>& xInputStream,
                        bool bIsReadOnly )
                            { m_xInputStreamToLoadFrom = xInputStream;
                              m_bIsReadOnly = bIsReadOnly; }
    // #i88291#
    void            clearStreamToLoadFrom();

    bool     WasLastEditOK() const       { return bWasLastEditOK; }
    FileDialogHelper & GetInsertFileDialog(const OUString& rFactory);
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
