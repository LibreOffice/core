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

#ifndef INCLUDED_UNOTOOLS_ITEMHOLDERBASE_HXX
#define INCLUDED_UNOTOOLS_ITEMHOLDERBASE_HXX

#include <memory>
#include <unotools/options.hxx>

enum class EItem
{
    CmdOptions                    ,
    ColorConfig                   ,   // 2
    CTLOptions                    ,   // 2

    EventConfig                   ,

    LinguConfig                   ,

    MiscOptions                   ,
    ModuleOptions                 ,

    PathOptions                   ,

    SysLocaleOptions              ,   // 2

    UserOptions                   ,   // 2
};

struct TItemInfo
{
    TItemInfo()
        : eItem(EItem::UserOptions)
    {
    }

    TItemInfo(TItemInfo&& other) noexcept
        : pItem(std::move(other.pItem))
        , eItem(other.eItem)
    {
    }

    std::unique_ptr<utl::detail::Options> pItem;
    EItem eItem;
};

#endif // INCLUDED_UNOTOOLS_ITEMHOLDERBASE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
