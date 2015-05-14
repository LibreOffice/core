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
#include <tools/solar.h>
#include <vcl/dllapi.h>
#include <o3tl/typed_flags_set.hxx>

class Point;
class Rectangle;
namespace vcl { class Window; }


// - Help-Types -

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
/// force the existent tip window to be re-positioned, even if the previous incarnation has the same text. Applies to ShowBallon and ShowQuickHelp.
    ForceReposition   = 0x0080,
/// no delay when opening the quick help. Applies to ShowBallon and ShowQuickHelp
    NoDelay           = 0x0100,
/// force balloon-style in ShowTip
    TipStyleBalloon   = 0x0200,
    NoEvadePointer    = 0x4000,
    BiDiRtl           = 0x8000,
};
namespace o3tl
{
    template<> struct typed_flags<QuickHelpFlags> : is_typed_flags<QuickHelpFlags, 0xc3ff> {};
}

#define OOO_HELP_INDEX          ".help:index"

// - Help -

class VCL_DLLPUBLIC Help
{
public:
                        Help();
    virtual             ~Help();

    virtual bool        Start( const OUString& rHelpId, const vcl::Window* pWindow );
    virtual bool        SearchKeyword( const OUString& rKeyWord );
    virtual OUString    GetHelpText( const OUString& aHelpURL, const vcl::Window* pWindow );

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
    static bool         ShowBalloon( vcl::Window* pParent,
                                     const Point& rScreenPos,
                                     const OUString& rHelpText );
    static bool         ShowBalloon( vcl::Window* pParent,
                                     const Point& rScreenPos,
                                     const Rectangle&,
                                     const OUString& rHelpText );

    static void         EnableQuickHelp();
    static void         DisableQuickHelp();
    static bool         IsQuickHelpEnabled();
    static bool         ShowQuickHelp( vcl::Window* pParent,
                                       const Rectangle& rScreenRect,
                                       const OUString& rHelpText,
                                       const OUString& rLongHelpText,
                                       QuickHelpFlags nStyle = QuickHelpFlags::NONE );
    static bool         ShowQuickHelp( vcl::Window* pParent,
                                       const Rectangle& rScreenRect,
                                       const OUString& rHelpText,
                                       QuickHelpFlags nStyle = QuickHelpFlags::NONE )
                            { return Help::ShowQuickHelp( pParent, rScreenRect, rHelpText, OUString(), nStyle ); }

    static void         HideBalloonAndQuickHelp();

    static sal_uLong    ShowTip( vcl::Window* pParent,
                                 const Rectangle& rScreenRect,
                                 const OUString& rText, QuickHelpFlags nStyle = QuickHelpFlags::NONE );
    static void         UpdateTip( sal_uLong nId,
                                   vcl::Window* pParent,
                                   const Rectangle& rScreenRect,
                                   const OUString& rText );
    static void         HideTip( sal_uLong nId );
};

#endif // INCLUDED_VCL_HELP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
