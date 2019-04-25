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

#include <sal/config.h>
#include <sfx2/dllapi.h>

#include <tools/ref.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <memory>

namespace com { namespace sun { namespace star { namespace uno
{
    class Any;
}}}}
namespace weld { class Window; }

template <typename Arg, typename Ret> class Link;
namespace com::sun::star::io { class XInputStream; }

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
    std::unique_ptr<SvLinkSource_Impl>  pImpl; // compatible area

public:

                        SvLinkSource();
    virtual             ~SvLinkSource() override;

    bool                HasDataLinks() const;

    void                Closed();

    sal_uInt64          GetUpdateTimeout() const;
    void                SetUpdateTimeout( sal_uInt64 nTimeMs );
                        // notify the sink, the mime type is not
                        // a selection criterion
    void                DataChanged( const OUString & rMimeType,
                                    const css::uno::Any & rVal );
    void                SendDataChanged();
    void                NotifyDataChanged();

    virtual bool        Connect( SvBaseLink* );
    virtual bool        GetData( css::uno::Any & rData /*out param*/,
                                const OUString & rMimeType,
                                bool bSynchron = false );

                        // sal_True => waitinmg for data
    virtual bool        IsPending() const;
                        // sal_True => data complete loaded
    virtual bool        IsDataComplete() const;

    virtual void        Edit(weld::Window *, SvBaseLink *, const Link<const OUString&, void>& rEndEditHdl);


    void                AddDataAdvise( SvBaseLink *, const OUString & rMimeType,
                                        sal_uInt16 nAdviceMode );
    void                RemoveAllDataAdvise( SvBaseLink const * );

    void                AddConnectAdvise( SvBaseLink * );
    void                RemoveConnectAdvise( SvBaseLink const * );

    struct StreamToLoadFrom{
        StreamToLoadFrom(
            const css::uno::Reference<css::io::XInputStream>& xInputStream, bool bIsReadOnly )
            :m_xInputStreamToLoadFrom(xInputStream),
             m_bIsReadOnly(bIsReadOnly)
        {
        }

        css::uno::Reference<css::io::XInputStream>
             m_xInputStreamToLoadFrom;
        bool const m_bIsReadOnly;
    };

    StreamToLoadFrom getStreamToLoadFrom();
    void setStreamToLoadFrom(const css::uno::Reference<css::io::XInputStream>& xInputStream, bool bIsReadOnly );
    void clearStreamToLoadFrom();
};

typedef tools::SvRef<SvLinkSource> SvLinkSourceRef;

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
