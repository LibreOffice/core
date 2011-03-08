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
#ifndef _LNKBASE_HXX
#define _LNKBASE_HXX

#include <com/sun/star/io/XInputStream.hpp>
#include "sal/config.h"
#include "sfx2/dllapi.h"
#include <sot/exchange.hxx>
#include <tools/ref.hxx>
#include <sfx2/linksrc.hxx>

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

#define OBJECT_CLIENT_SO            0x80 // ein Link
#define OBJECT_CLIENT_DDE           0x81
//#define   OBJECT_CLIENT_OLE           0x82 // ein Ole-Link
//#define   OBJECT_CLIENT_OLE_CACHE     0x83 // ein Ole-Link mit SvEmbeddedObject
#define OBJECT_CLIENT_FILE          0x90
#define OBJECT_CLIENT_GRF           0x91
#define OBJECT_CLIENT_OLE           0x92 // embedded link

enum sfxlink {
    // Ole2 compatibel und persistent
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
    String                  aLinkName;
    BaseLink_Impl*          pImpl;
    USHORT                  nObjType;
    BOOL                    bVisible : 1;
    BOOL                    bSynchron : 1;
    BOOL                    bUseCache : 1;      // fuer GrafikLinks!
    BOOL                    bWasLastEditOK : 1;

    DECL_LINK( EndEditHdl, String* );

    bool                    ExecuteEdit( const String& _rNewName );

protected:
    void            SetObjType( USHORT );

                    // setzen des LinkSourceName ohne aktion
    void            SetName( const String & rLn );
                    // LinkSourceName der im SvLinkBase steht
    String          GetName() const;

    ImplBaseLinkData* pImplData;

    sal_Bool            m_bIsReadOnly;
    com::sun::star::uno::Reference<com::sun::star::io::XInputStream>
                        m_xInputStreamToLoadFrom;

                    SvBaseLink();
                    SvBaseLink( USHORT nLinkType, ULONG nContentType = FORMAT_STRING );
    virtual         ~SvBaseLink();

    void            _GetRealObject( BOOL bConnect = TRUE );

    SvLinkSource*   GetRealObject()
                    {
                        if( !xObj.Is() )
                            _GetRealObject();
                        return xObj;
                    }

public:
                    TYPEINFO();
                    // ask JP
    virtual void    Closed();
                    SvBaseLink( const String& rNm, USHORT nObjectType,
                                 SvLinkSource* );

    USHORT          GetObjType() const { return nObjType; }

    void            SetObj( SvLinkSource * pObj );
    SvLinkSource*   GetObj() const  { return xObj; }

    void            SetLinkSourceName( const String & rName );
    String          GetLinkSourceName() const;

    virtual void    DataChanged( const String & rMimeType,
                                const ::com::sun::star::uno::Any & rValue );

    void            SetUpdateMode( USHORT );
    USHORT          GetUpdateMode() const;
    ULONG           GetContentType() const;
    BOOL            SetContentType( ULONG nType );

    LinkManager*          GetLinkManager();
    const LinkManager*    GetLinkManager() const;
    void                    SetLinkManager( LinkManager* _pMgr );

    BOOL            Update();
    void            Disconnect();

    // Link impl: DECL_LINK( MyEndDialogHdl, SvBaseLink* ); <= param is this
    virtual void    Edit( Window*, const Link& rEndEditHdl );

        // soll der Link im Dialog angezeigt werden ? (Links im Link im ...)
    BOOL            IsVisible() const           { return bVisible; }
    void            SetVisible( BOOL bFlag )    { bVisible = bFlag; }
        // soll der Link synchron oder asynchron geladen werden?
    BOOL            IsSynchron() const          { return bSynchron; }
    void            SetSynchron( BOOL bFlag )   { bSynchron = bFlag; }

    BOOL            IsUseCache() const          { return bUseCache; }
    void            SetUseCache( BOOL bFlag )   { bUseCache = bFlag; }

    void            setStreamToLoadFrom(
                        const com::sun::star::uno::Reference<com::sun::star::io::XInputStream>& xInputStream,
                        sal_Bool bIsReadOnly )
                            { m_xInputStreamToLoadFrom = xInputStream;
                              m_bIsReadOnly = bIsReadOnly; }
    // #i88291#
    void            clearStreamToLoadFrom();

    inline BOOL         WasLastEditOK() const       { return bWasLastEditOK; }
    FileDialogHelper*   GetFileDialog( sal_uInt32 nFlags, const String& rFactory ) const;
};

SV_DECL_IMPL_REF(SvBaseLink);

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
