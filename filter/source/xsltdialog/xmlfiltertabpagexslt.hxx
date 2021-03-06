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

#include <vcl/weld.hxx>
#include <svtools/inettbc.hxx>

namespace vcl { class Window; }
class filter_info_impl;

class XMLFilterTabPageXSLT
{
private:
    void SetURL(SvtURLBox& rURLBox, const OUString& rURL);
    static OUString GetURL(const SvtURLBox& rURLBox);

    OUString sInstPath;

public:
    explicit XMLFilterTabPageXSLT(weld::Widget* pPage, weld::Dialog* pDialog);
    ~XMLFilterTabPageXSLT();

    void FillInfo( filter_info_impl* pInfo );
    void SetInfo(const filter_info_impl* pInfo);

    DECL_LINK( ClickBrowseHdl_Impl, weld::Button&, void );

    weld::Dialog*                  m_pDialog;
    std::unique_ptr<weld::Builder> m_xBuilder;
    std::unique_ptr<weld::Widget>  m_xContainer;
    std::unique_ptr<weld::Entry>   m_xEDDocType;
    std::unique_ptr<SvtURLBox>     m_xEDExportXSLT;
    std::unique_ptr<weld::Button>  m_xPBExprotXSLT;
    std::unique_ptr<SvtURLBox>     m_xEDImportXSLT;
    std::unique_ptr<weld::Button>  m_xPBImportXSLT;
    std::unique_ptr<SvtURLBox>     m_xEDImportTemplate;
    std::unique_ptr<weld::Button>  m_xPBImportTemplate;
    std::unique_ptr<weld::CheckButton> m_xCBNeedsXSLT2;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
