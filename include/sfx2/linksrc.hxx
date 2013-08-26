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
#ifndef _LINKSRC_HXX
#define _LINKSRC_HXX

#include "sal/config.h"
#include "sfx2/dllapi.h"

#include <tools/link.hxx>
#include <tools/ref.hxx>
#include <tools/rtti.hxx>
#include <com/sun/star/io/XInputStream.hpp>

#include <sal/config.h>

namespace com { namespace sun { namespace star { namespace uno
{
    class Any;
}}}}
class Window;
namespace rtl {
    class OUString;
}

#ifndef ADVISEMODE_NODATA
// Must be the same value as Ole2 ADVF_*
#define ADVISEMODE_NODATA       0x01
#define ADVISEMODE_PRIMEFIRST   0x02
#define ADVISEMODE_ONLYONCE     0x04
#define ADVISEMODE_DATAONSTOP   0x40
#endif

namespace sfx2
{

class SvBaseLink;
struct SvLinkSource_Impl;

class SFX2_DLLPUBLIC SvLinkSource : public SvRefBase
{
private:
    SvLinkSource_Impl*  pImpl; // compatible area

public:
                        TYPEINFO();

                        SvLinkSource();
    virtual             ~SvLinkSource();

    sal_Bool                HasDataLinks( const SvBaseLink* = 0 ) const;

    void                Closed();

    sal_uIntPtr                 GetUpdateTimeout() const;
    void                SetUpdateTimeout( sal_uIntPtr nTime );
                        // notify the sink, the mime type is not
                        // a selection criterion
    void                DataChanged( const rtl::OUString & rMimeType,
                                    const ::com::sun::star::uno::Any & rVal );
    void                SendDataChanged();
    void                NotifyDataChanged();

    virtual sal_Bool        Connect( SvBaseLink* );
    virtual sal_Bool        GetData( ::com::sun::star::uno::Any & rData /*out param*/,
                                const rtl::OUString & rMimeType,
                                sal_Bool bSynchron = sal_False );

                        // sal_True => waitinmg for data
    virtual sal_Bool        IsPending() const;
                        // sal_True => data complete loaded
    virtual sal_Bool        IsDataComplete() const;

    // Link impl: DECL_LINK( MyEndEditHdl, sfx2::FileDialogHelper* ); <= param is the dialog
    virtual void        Edit( Window *, SvBaseLink *, const Link& rEndEditHdl );


    void                AddDataAdvise( SvBaseLink *, const rtl::OUString & rMimeType,
                                        sal_uInt16 nAdviceMode );
    void                RemoveAllDataAdvise( SvBaseLink * );

    void                AddConnectAdvise( SvBaseLink * );
    void                RemoveConnectAdvise( SvBaseLink * );

    struct StreamToLoadFrom{
        StreamToLoadFrom(
            const com::sun::star::uno::Reference<com::sun::star::io::XInputStream>& xInputStream,sal_Bool bIsReadOnly )
            :m_xInputStreamToLoadFrom(xInputStream),
             m_bIsReadOnly(bIsReadOnly)
        {
        }

        com::sun::star::uno::Reference<com::sun::star::io::XInputStream>
        m_xInputStreamToLoadFrom;
        sal_Bool m_bIsReadOnly;
    };

    StreamToLoadFrom getStreamToLoadFrom();
    void setStreamToLoadFrom(const com::sun::star::uno::Reference<com::sun::star::io::XInputStream>& xInputStream,sal_Bool bIsReadOnly );
    void clearStreamToLoadFrom();
};

SV_DECL_IMPL_REF(SvLinkSource);

}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
