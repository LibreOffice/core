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
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_NUMBERINGTYPELISTBOX_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_NUMBERINGTYPELISTBOX_HXX

#include <vcl/lstbox.hxx>
#include "swdllapi.h"
#include <o3tl/typed_flags_set.hxx>

enum class SwInsertNumTypes
{
    NoNumbering              = 0x01,
    PageStyleNumbering       = 0x02,
    Bitmap                   = 0x04,
    Bullet                   = 0x08,
    Extended                 = 0x10
};

namespace o3tl {
   template<> struct typed_flags<SwInsertNumTypes> : is_typed_flags<SwInsertNumTypes, 0x1f> {};
};

struct SwNumberingTypeListBox_Impl;

class SW_DLLPUBLIC SwNumberingTypeListBox : public ListBox
{
    SwNumberingTypeListBox_Impl* pImpl;

public:
    SwNumberingTypeListBox( vcl::Window* pWin, WinBits nStyle = WB_BORDER );
    virtual ~SwNumberingTypeListBox();
    virtual void dispose() override;

    virtual bool set_property(const OString &rKey, const OString &rValue) override;

    void        Reload(SwInsertNumTypes nTypeFlags);

    sal_Int16   GetSelectedNumberingType();
    bool    SelectNumberingType(sal_Int16 nType);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
