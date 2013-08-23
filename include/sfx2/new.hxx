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
#ifndef _SFXNEW_HXX
#define _SFXNEW_HXX

#include "sal/config.h"
#include "sfx2/dllapi.h"

#include <vcl/button.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/edit.hxx>
#include <vcl/fixed.hxx>
#include <sfx2/basedlgs.hxx>

//=========================================================================

class SfxObjectShellLock;
class SfxObjectShell;
class MoreButton;

//=========================================================================

#define SFXWB_PREVIEW 0x0003
#define SFXWB_LOAD_TEMPLATE 0x0004

#define SFX_LOAD_TEXT_STYLES    0x0001
#define SFX_LOAD_FRAME_STYLES   0x0002
#define SFX_LOAD_PAGE_STYLES    0x0004
#define SFX_LOAD_NUM_STYLES     0x0008
#define SFX_MERGE_STYLES        0x0010

#define RET_TEMPLATE_LOAD       100

class SfxNewFileDialog_Impl;
class SFX2_DLLPUBLIC SfxNewFileDialog : public SfxModalDialog
{
    friend class SfxNewFileDialog_Impl;

private:
    SfxNewFileDialog_Impl* pImpl;

public:

    SfxNewFileDialog(Window *pParent, sal_uInt16 nFlags = 0);
    ~SfxNewFileDialog();

    // Returns sal_False, when '- No -' is set as Template
    // Template names can only be obtained when IsTemplate() returns sal_True.
    sal_Bool IsTemplate() const;
    OUString GetTemplateFileName() const;

    // load template methods
    sal_uInt16  GetTemplateFlags()const;
    void    SetTemplateFlags(sal_uInt16 nSet);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
