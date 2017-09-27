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

#ifndef INCLUDED_SVX_SOURCE_SIDEBAR_TEXT_TEXTUNDERLINECONTROL_HXX
#define INCLUDED_SVX_SOURCE_SIDEBAR_TEXT_TEXTUNDERLINECONTROL_HXX

#include <sfx2/bindings.hxx>
#include <vcl/button.hxx>
#include <vcl/vclenum.hxx>
#include <svtools/valueset.hxx>
#include <sfx2/tbxctrl.hxx>

namespace svx {

class TextUnderlineControl : public SfxPopupWindow
{
public:
    explicit TextUnderlineControl(sal_uInt16 nId, vcl::Window* pParent);
    virtual ~TextUnderlineControl() override;
    virtual void dispose() override;

private:
    VclPtr<PushButton> maNone;
    VclPtr<PushButton> maSingle;
    VclPtr<PushButton> maDouble;
    VclPtr<PushButton> maBold;
    VclPtr<PushButton> maDot;
    VclPtr<PushButton> maDotBold;
    VclPtr<PushButton> maDash;
    VclPtr<PushButton> maDashLong;
    VclPtr<PushButton> maDashDot;
    VclPtr<PushButton> maDashDotDot;
    VclPtr<PushButton> maWave;
    VclPtr<PushButton> maMoreOptions;

    FontLineStyle getLineStyle(Button const * pButton);

    DECL_LINK(PBClickHdl, Button*, void);
};
}


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
