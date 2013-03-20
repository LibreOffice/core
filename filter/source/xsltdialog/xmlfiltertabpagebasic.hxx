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
#if 1

#include <sfx2/tabdlg.hxx>
#include <vcl/fixed.hxx>
#include <vcl/edit.hxx>
#include <vcl/combobox.hxx>
#include <svtools/svmedit.hxx>

class Window;
class ResMgr;
class filter_info_impl;

class XMLFilterTabPageBasic : public TabPage
{
public:
    XMLFilterTabPageBasic( Window* pParent, ResMgr& rResMgr );
    virtual ~XMLFilterTabPageBasic();

    bool FillInfo( filter_info_impl* pInfo );
    void SetInfo(const filter_info_impl* pInfo);

    static rtl::OUString decodeComment( const rtl::OUString& rComment );
    static rtl::OUString encodeComment( const rtl::OUString& rComment );

    FixedText       maFTFilterName;
    Edit            maEDFilterName;

    FixedText       maFTApplication;
    ComboBox        maCBApplication;

    FixedText       maFTInterfaceName;
    Edit            maEDInterfaceName;

    FixedText       maFTExtension;
    Edit            maEDExtension;
    FixedText       maFTDescription;
    MultiLineEdit   maEDDescription;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
