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

#ifndef INCLUDED_SFX2_NOTEBOOKBAR_NOTEBOOKBARPOPUP_HXX
#define INCLUDED_SFX2_NOTEBOOKBAR_NOTEBOOKBARPOPUP_HXX

#include <vcl/layout.hxx>
#include <vcl/toolkit/floatwin.hxx>

/*
 * Popup - shows hidden content, controls are moved to this popup
 * and after close moved to the original parent
 */

class NotebookbarPopup : public FloatingWindow
{
private:
    VclPtr<VclHBox> m_pBox;
    ScopedVclPtr<VclHBox> m_pParent;

public:
    explicit NotebookbarPopup(const VclPtr<VclHBox>& pParent);

    virtual ~NotebookbarPopup() override;

    VclHBox* getBox();

    virtual void PopupModeEnd() override;

    void hideSeparators(bool bHide);

    void dispose() override;

    void ApplyBackground(vcl::Window* pWindow);

    void RemoveBackground(vcl::Window* pWindow);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
