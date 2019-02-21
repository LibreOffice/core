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

#ifndef INCLUDED_VCL_HELP_HXX
#define INCLUDED_VCL_HELP_HXX

#include <rtl/ustring.hxx>
#include <vcl/dllapi.h>
#include <o3tl/typed_flags_set.hxx>

class Point;
namespace tools { class Rectangle; }
namespace vcl { class Window; }


enum class QuickHelpFlags
{
    NONE              = 0x0000,
    Left              = 0x0001,
    Center            = 0x0002,
    Right             = 0x0004,
    Top               = 0x0008,
    VCenter           = 0x0010,
    Bottom            = 0x0020,
    NoAutoPos         = Left | Center | Right | Top | VCenter | Bottom,
    CtrlText          = 0x0040,
/// force balloon-style in ShowPopover and ShowQuickHelp
    TipStyleBalloon   = 0x0100,
    NoEvadePointer    = 0x0200,
    BiDiRtl           = 0x0400,
};
namespace o3tl
{
    template<> struct typed_flags<QuickHelpFlags> : is_typed_flags<QuickHelpFlags, 0x77f> {};
}

#define OOO_HELP_INDEX          ".help:index"

namespace weld
{
    class Widget;
}

class VCL_DLLPUBLIC Help
{
public:
                        Help();
    virtual             ~Help();

    virtual bool        Start(const OUString& rHelpId, const vcl::Window* pWindow);
    virtual bool        Start(const OUString& rHelpId, weld::Widget* pWidget);
    virtual void        SearchKeyword( const OUString& rKeyWord );
    virtual OUString    GetHelpText(const OUString& aHelpURL, const vcl::Window* pWindow);
    virtual OUString    GetHelpText(const OUString& aHelpURL, const weld::Widget* pWidget);

    static void         EnableContextHelp();
    static void         DisableContextHelp();
    static bool         IsContextHelpEnabled();

    static void         EnableExtHelp();
    static void         DisableExtHelp();
    static bool         IsExtHelpEnabled();
    static bool         StartExtHelp();
    static bool         EndExtHelp();

    static void         EnableBalloonHelp();
    static void         DisableBalloonHelp();
    static bool         IsBalloonHelpEnabled();
    static void         ShowBalloon( vcl::Window* pParent,
                                     const Point& rScreenPos,
                                     const tools::Rectangle&,
                                     const OUString& rHelpText );

    static void         EnableQuickHelp();
    static void         DisableQuickHelp();
    static bool         IsQuickHelpEnabled();
    static void         ShowQuickHelp( vcl::Window* pParent,
                                       const tools::Rectangle& rScreenRect,
                                       const OUString& rHelpText,
                                       QuickHelpFlags nStyle = QuickHelpFlags::NONE );

    static void         HideBalloonAndQuickHelp();

    static void*        ShowPopover(vcl::Window* pParent,
                                    const tools::Rectangle& rScreenRect,
                                    const OUString& rText,
                                    QuickHelpFlags nStyle);
    static void         UpdatePopover(void* nId,
                                      vcl::Window* pParent,
                                      const tools::Rectangle& rScreenRect,
                                      const OUString& rText);
    static void         HidePopover(vcl::Window const * pParent, void* nId);
};

#endif // INCLUDED_VCL_HELP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
