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

#include <vector>
#include <osl/mutex.hxx>

namespace utl { namespace detail { class Options; } }

struct ItemHolderMutexBase
{
    ::osl::Mutex m_aLock;
};

enum class EItem
{
    AccessibilityOptions          ,   // 2

    CJKOptions                    ,   // 2
    CmdOptions                    ,
    ColorConfig                   ,   // 2
    Compatibility                 ,
    CTLOptions                    ,   // 2

    DefaultOptions                ,
    DynamicMenuOptions            ,

    EventConfig                   ,
    ExtendedSecurityOptions       ,

    FontOptions                   ,

    HelpOptions                   ,   // 2
    HistoryOptions                ,

    LinguConfig                   ,

    MenuOptions                   ,
    MiscConfig                    ,   // 2
    MiscOptions                   ,
    ModuleOptions                 ,

    OptionsDialogOptions          ,

    PathOptions                   ,
    PrintOptions                  ,   // 2
    PrintFileOptions              ,   // 2
    PrintWarningOptions           ,

    SaveOptions                   ,
    SecurityOptions               ,
    SysLocaleOptions              ,   // 2

    UserOptions                   ,   // 2

    ViewOptionsDialog             ,
    ViewOptionsTabDialog          ,
    ViewOptionsTabPage            ,
    ViewOptionsWindow
};

struct TItemInfo
{
    TItemInfo()
        : pItem(nullptr)
        , eItem(EItem::UserOptions)
    {
    }

    utl::detail::Options * pItem;
    EItem eItem;
};

typedef ::std::vector< TItemInfo > TItems;

#endif // INCLUDED_UNOTOOLS_ITEMHOLDERBASE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
