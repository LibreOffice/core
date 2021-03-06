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

namespace formula
{

class FormulaToken;


class StructPage final
{
private:
    std::unique_ptr<weld::Builder> m_xBuilder;
    std::unique_ptr<weld::Container> m_xContainer;
    std::unique_ptr<weld::TreeView> m_xTlbStruct;

    Link<StructPage&,void>  aSelLink;

    OUString        maImgEnd;
    OUString        maImgError;

    const FormulaToken* pSelectedToken;
    bool            bActiveFlag;

    DECL_LINK(SelectHdl, weld::TreeView&, void);

    const FormulaToken* GetFunctionEntry(const weld::TreeIter* pEntry);

    void            SetActiveFlag(bool bFlag);
    bool            GetActiveFlag() const { return bActiveFlag;}

public:

    explicit StructPage(weld::Container* pParent);
    ~StructPage();

    void            ClearStruct();
    bool InsertEntry(const OUString& rText, const weld::TreeIter* pParent,
                     sal_uInt16 nFlag, int nPos,
                     const FormulaToken* pIFormulaToken,
                     weld::TreeIter& rRet);

    OUString        GetEntryText(const weld::TreeIter* pEntry) const;

    void            SetSelectionHdl( const Link<StructPage&,void>& rLink ) { aSelLink = rLink; }

    weld::TreeView&  GetTlbStruct() const { return *m_xTlbStruct; }

    bool            IsVisible() const { return m_xContainer->get_visible(); }
};

} // formula

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
