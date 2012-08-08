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

#ifndef INCLUDED_unotools_ITEMHOLDERBASE_HXX_
#define INCLUDED_unotools_ITEMHOLDERBASE_HXX_

#include <vector>
#include <osl/mutex.hxx>

namespace utl { namespace detail { class Options; } }

//-----------------------------------------------
// definitions

struct ItemHolderMutexBase
{
    ::osl::Mutex m_aLock;
};

enum EItem
{
    E_ACCELCFG                      ,   //
    E_ACCESSIBILITYOPTIONS          ,   // 2
    E_ADDXMLTOSTORAGEOPTIONS        ,   //
    E_APEARCFG                      ,   // 2

    E_CJKOPTIONS                    ,   // 2
    E_CMDOPTIONS                    ,   //
    E_COLORCFG                      ,   // 2
    E_COMPATIBILITY                 ,   //
    E_CTLOPTIONS                    ,   // 2

    E_DEFAULTOPTIONS                ,   //
    E_DYNAMICMENUOPTIONS            ,   //

    E_EVENTCFG                      ,   //
    E_EXTENDEDSECURITYOPTIONS       ,   //

    E_FLTRCFG                       ,   //
    E_FONTOPTIONS                   ,   //
    E_FONTSUBSTCONFIG               ,   // 2

    E_HELPOPTIONS                   ,   // 2
    E_HISTORYOPTIONS                ,   //

    E_INTERNALOPTIONS               ,   //

    E_JAVAOPTIONS                   ,   //

    E_LANGUAGEOPTIONS               ,   // 2
    E_LINGUCFG                      ,   //
    E_LOCALISATIONOPTIONS           ,   //

    E_MENUOPTIONS                   ,   //
    E_MISCCFG                       ,   // 2
    E_MISCOPTIONS                   ,   //
    E_MODULEOPTIONS                 ,   //

    E_OPTIONSDLGOPTIONS             ,   //

    E_PATHOPTIONS                   ,   //
    E_PRINTOPTIONS                  ,   // 2
    E_PRINTFILEOPTIONS              ,   // 2
    E_PRINTWARNINGOPTIONS           ,   //

    E_REGOPTIONS                    ,   //

    E_SAVEOPTIONS                   ,   //
    E_SEARCHOPT                     ,   //
    E_SECURITYOPTIONS               ,   //
    E_STARTOPTIONS                  ,   //
    E_SYSLOCALEOPTIONS              ,   // 2

    E_USEROPTIONS                   ,   // 2

    E_VIEWOPTIONS_DIALOG            ,   //
    E_VIEWOPTIONS_TABDIALOG         ,   //
    E_VIEWOPTIONS_TABPAGE           ,   //
    E_VIEWOPTIONS_WINDOW            ,   //

    E_WORKINGSETOPTIONS             ,   //

    E_XMLACCELCFG                       //
};

struct TItemInfo
{
    TItemInfo()
        : pItem(0)
    {}

    utl::detail::Options * pItem;
    EItem eItem;
};

typedef ::std::vector< TItemInfo > TItems;

#endif // INCLUDED_unotools_ITEMHOLDERBASE_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
