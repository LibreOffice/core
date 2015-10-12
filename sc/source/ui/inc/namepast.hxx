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

#ifndef INCLUDED_SC_SOURCE_UI_INC_NAMEPAST_HXX
#define INCLUDED_SC_SOURCE_UI_INC_NAMEPAST_HXX

#include <vcl/dialog.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include "namemgrtable.hxx"

#include <vector>

#include "scui_def.hxx"
class ScRangeName;
class ScDocShell;

class ScNamePasteDlg : public ModalDialog
{
    DECL_LINK_TYPED( ButtonHdl, Button *, void );

private:
    VclPtr<PushButton> m_pBtnPasteAll;
    VclPtr<PushButton> m_pBtnPaste;
    VclPtr<PushButton> m_pBtnClose;

    VclPtr<ScRangeManagerTable> mpTable;

    std::vector<OUString> maSelectedNames;
    boost::ptr_map<OUString, ScRangeName> maRangeMap;
public:
    ScNamePasteDlg( vcl::Window * pParent, ScDocShell* pShell, bool bInsList=true );

    virtual ~ScNamePasteDlg();
    virtual void dispose() override;

    std::vector<OUString> GetSelectedNames() const;
};

#endif // INCLUDED_SC_SOURCE_UI_INC_NAMEPAST_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
