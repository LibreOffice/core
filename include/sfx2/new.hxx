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
#ifndef INCLUDED_SFX2_NEW_HXX
#define INCLUDED_SFX2_NEW_HXX

#include <sal/config.h>
#include <sfx2/dllapi.h>

#include <vcl/button.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/edit.hxx>
#include <vcl/fixed.hxx>
#include <sfx2/basedlgs.hxx>
#include <o3tl/typed_flags_set.hxx>



class SfxObjectShellLock;
class SfxObjectShell;



#define SFXWB_PREVIEW 0x0003
#define SFXWB_LOAD_TEMPLATE 0x0004

enum class SfxTemplateFlags
{
    NONE                = 0x00,
    LOAD_TEXT_STYLES    = 0x01,
    LOAD_FRAME_STYLES   = 0x02,
    LOAD_PAGE_STYLES    = 0x04,
    LOAD_NUM_STYLES     = 0x08,
    MERGE_STYLES        = 0x10,
};
namespace o3tl
{
    template<> struct typed_flags<SfxTemplateFlags> : is_typed_flags<SfxTemplateFlags, 0x1f> {};
}

#define RET_TEMPLATE_LOAD       100

class SfxNewFileDialog_Impl;
class SFX2_DLLPUBLIC SfxNewFileDialog : public SfxModalDialog
{
    friend class SfxNewFileDialog_Impl;

private:
    SfxNewFileDialog_Impl* pImpl;

public:

    SfxNewFileDialog(vcl::Window *pParent, sal_uInt16 nFlags = 0);
    virtual ~SfxNewFileDialog();
    virtual void dispose() override;

    // Returns false, when '- No -' is set as Template
    // Template names can only be obtained when IsTemplate() returns true.
    bool IsTemplate() const;
    OUString GetTemplateFileName() const;

    // load template methods
    SfxTemplateFlags  GetTemplateFlags() const;
    void              SetTemplateFlags(SfxTemplateFlags nSet);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
