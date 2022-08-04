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

#ifndef INCLUDED_SW_INC_SWAPPLETIMPL_HXX
#define INCLUDED_SW_INC_SWAPPLETIMPL_HXX

#include <config_java.h>

#include <svl/itemset.hxx>
#include <svl/ownlist.hxx>
#include <utility>

namespace com::sun::star::embed { class XEmbeddedObject; }

enum class SwHtmlOptType {
    IGNORE = 0,
    TAG = 1,
    PARAM = 2,
    SIZE = 3
};

#define OOO_STRING_SW_HTML_O_Hidden "HIDDEN"

class SwApplet_Impl
{
    css::uno::Reference < css::embed::XEmbeddedObject > m_xApplet;
    SvCommandList     m_aCommandList;
    SfxItemSet        m_aItemSet;
    OUString   m_sAlt;

public:
    static SwHtmlOptType GetOptionType( const OUString& rName, bool bApplet );
    SwApplet_Impl( SfxItemPool& rPool );
    SwApplet_Impl( SfxItemSet aSet ): m_aItemSet (std::move(aSet)) {}
    ~SwApplet_Impl();
    void CreateApplet( const OUString& rCode, const OUString& rName,
                       bool bMayScript, const OUString& rCodeBase,
                       std::u16string_view rBaseURL );
#if HAVE_FEATURE_JAVA
    bool CreateApplet( std::u16string_view rBaseURL );
    void AppendParam( const OUString& rName, const OUString& rValue );
#endif
    void FinishApplet();
    const css::uno::Reference < css::embed::XEmbeddedObject >& GetApplet() const { return m_xApplet; }
    SfxItemSet& GetItemSet() { return m_aItemSet; }
    const OUString& GetAltText() const { return m_sAlt; }
    void SetAltText( const OUString& rAlt ) {m_sAlt = rAlt;}
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
