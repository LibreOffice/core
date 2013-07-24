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
#ifndef _XMLFILTERTABPAGEBASIC_HXX_
#define _XMLFILTERTABPAGEBASIC_HXX_

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
    XMLFilterTabPageBasic(Window* pParent);
    virtual ~XMLFilterTabPageBasic();

    bool FillInfo( filter_info_impl* pInfo );
    void SetInfo(const filter_info_impl* pInfo);

    static OUString decodeComment( const OUString& rComment );
    static OUString encodeComment( const OUString& rComment );

    Edit*              m_pEDFilterName;
    ComboBox*          m_pCBApplication;
    Edit*              m_pEDInterfaceName;
    Edit*              m_pEDExtension;
    VclMultiLineEdit*  m_pEDDescription;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
