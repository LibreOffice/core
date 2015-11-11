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

#ifndef INCLUDED_SD_SOURCE_UI_INC_TEXTOBJECTBAR_HXX
#define INCLUDED_SD_SOURCE_UI_INC_TEXTOBJECTBAR_HXX

#include <sfx2/module.hxx>
#include <sfx2/shell.hxx>
#include "glob.hxx"

class CommandEvent;

namespace sd {

class View;
class ViewShell;

class TextObjectBar
    : public SfxShell
{
public:
    SFX_DECL_INTERFACE(SD_IF_SDDRAWTEXTOBJECTBAR)

private:
    /// SfxInterface initializer.
    static void InitInterface_Impl();

public:
    TextObjectBar (
        ViewShell* pSdViewShell,
        SfxItemPool& rItemPool,
        ::sd::View* pSdView);
    virtual ~TextObjectBar();

    void GetAttrState( SfxItemSet& rSet );
    void GetCharState( SfxItemSet& rSet );
    void Execute( SfxRequest &rReq );

private:
    ViewShell* mpViewShell;
    ::sd::View* mpView;
};

} // end of namespace sd

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
