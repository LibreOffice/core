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
#ifndef INCLUDED_CUI_SOURCE_INC_MULTIPAT_HXX
#define INCLUDED_CUI_SOURCE_INC_MULTIPAT_HXX

#include <vcl/weld.hxx>
#include "radiobtnbox.hxx"

// define ----------------------------------------------------------------

// different delimiter for Unix (:) and Windows (;)

#ifdef UNX
#define CLASSPATH_DELIMITER ':'
#else
#define CLASSPATH_DELIMITER ';'
#endif

class SvxMultiPathDialog : public weld::GenericDialogController
{
private:
    std::unique_ptr<weld::TreeView> m_xRadioLB;
    std::unique_ptr<weld::Button> m_xAddBtn;
    std::unique_ptr<weld::Button> m_xDelBtn;

    void AppendEntry(const OUString& rText, const OUString& rId);
    void HandleEntryChecked(int nRow);

    DECL_LINK(AddHdl_Impl, weld::Button&, void);
    DECL_LINK(DelHdl_Impl, weld::Button&, void);
    DECL_LINK(SelectHdl_Impl, weld::TreeView&, void);
    typedef std::pair<int, int> row_col;
    DECL_LINK(CheckHdl_Impl, const row_col&, void);

public:
    SvxMultiPathDialog(weld::Window* pParent);
    virtual ~SvxMultiPathDialog() override;

    OUString        GetPath() const;
    void            SetPath(const OUString& rPath);
    void            SetTitle(const OUString& rTitle) { m_xDialog->set_title(rTitle); }
};

class SvxPathSelectDialog : public weld::GenericDialogController
{
private:
    std::unique_ptr<weld::TreeView> m_xPathLB;
    std::unique_ptr<weld::Button> m_xAddBtn;
    std::unique_ptr<weld::Button> m_xDelBtn;

    DECL_LINK(AddHdl_Impl, weld::Button&, void);
    DECL_LINK(DelHdl_Impl, weld::Button&, void);
    DECL_LINK(SelectHdl_Impl, weld::TreeView&, void);

public:
    SvxPathSelectDialog(weld::Window* pParent);

    OUString        GetPath() const;
    void            SetPath( const OUString& rPath );
    void            SetTitle(const OUString& rTitle) { m_xDialog->set_title(rTitle); }
};

#endif // INCLUDED_CUI_SOURCE_INC_MULTIPAT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
