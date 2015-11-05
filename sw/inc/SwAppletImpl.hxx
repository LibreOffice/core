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

#include <config_features.h>

enum class SwHtmlOptType {
    IGNORE = 0,
    TAG = 1,
    PARAM = 2,
    SIZE = 3
};

#include <com/sun/star/embed/XEmbeddedObject.hpp>

#include <sfx2/frmhtml.hxx>
#include <sfx2/frmhtmlw.hxx>
#include <vcl/wrkwin.hxx>
#include <sot/storage.hxx>
#include <svl/itemset.hxx>

#include <svl/ownlist.hxx>

class SfxItemSet;

#define OOO_STRING_SW_HTML_O_Hidden "HIDDEN"

class SwApplet_Impl
{
    css::uno::Reference < css::embed::XEmbeddedObject > xApplet;
    SvCommandList     aCommandList;
    SfxItemSet        aItemSet;
    OUString   sAlt;

public:
    static SwHtmlOptType GetOptionType( const OUString& rName, bool bApplet );
    SwApplet_Impl( SfxItemPool& rPool, sal_uInt16 nWhich1, sal_uInt16 nWhich2 );
    SwApplet_Impl( SfxItemSet& rSet ): aItemSet ( rSet) {}
    ~SwApplet_Impl();
    void CreateApplet( const OUString& rCode, const OUString& rName,
                       bool bMayScript, const OUString& rCodeBase,
                       const OUString& rBaseURL );
#if HAVE_FEATURE_JAVA
    bool CreateApplet( const OUString& rBaseURL );
    void AppendParam( const OUString& rName, const OUString& rValue );
#endif
    void FinishApplet();
    css::uno::Reference < css::embed::XEmbeddedObject > GetApplet() { return xApplet; }
    SfxItemSet& GetItemSet() { return aItemSet; }
    const OUString& GetAltText() const { return sAlt; }
    void SetAltText( const OUString& rAlt ) {sAlt = rAlt;}
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
