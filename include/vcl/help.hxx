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

class Point;
class Rectangle;
class Window;

// --------------
// - Help-Types -
// --------------

#define QUICKHELP_LEFT              ((sal_uInt16)0x0001)
#define QUICKHELP_CENTER            ((sal_uInt16)0x0002)
#define QUICKHELP_RIGHT             ((sal_uInt16)0x0004)
#define QUICKHELP_TOP               ((sal_uInt16)0x0008)
#define QUICKHELP_VCENTER           ((sal_uInt16)0x0010)
#define QUICKHELP_BOTTOM            ((sal_uInt16)0x0020)
#define QUICKHELP_NOAUTOPOS         (QUICKHELP_LEFT | QUICKHELP_CENTER | QUICKHELP_RIGHT | QUICKHELP_TOP | QUICKHELP_VCENTER | QUICKHELP_BOTTOM)
#define QUICKHELP_CTRLTEXT          ((sal_uInt16)0x0040)
/// force the existent tip window to be re-positioned, even if the previous incarnation has the same text. Applies to ShowBallon and ShowQuickHelp.
#define QUICKHELP_FORCE_REPOSITION  ((sal_uInt16)0x0080)
/// no delay when opening the quick help. Applies to ShowBallon and ShowQuickHelp
#define QUICKHELP_NO_DELAY          ((sal_uInt16)0x0100)
/// force balloon-style in ShowTip
#define QUICKHELP_TIP_STYLE_BALLOON ((sal_uInt16)0x0200)
#define QUICKHELP_NOEVADEPOINTER    ((sal_uInt16)0x4000)
#define QUICKHELP_BIDI_RTL          ((sal_uInt16)0x8000)

#define OOO_HELP_INDEX          ".help:index"
#define OOO_HELP_HELPONHELP     ".help:helponhelp"

// --------
// - Help -
// --------

class VCL_DLLPUBLIC Help
{
public:
                        Help();
    virtual             ~Help();

    virtual bool    Start( const OUString& rHelpId, const Window* pWindow );
    virtual bool    SearchKeyword( const OUString& rKeyWord );
    virtual OUString    GetHelpText( const OUString& aHelpURL, const Window* pWindow );

    static void         EnableContextHelp();
    static void         DisableContextHelp();
    static bool     IsContextHelpEnabled();

    static void         EnableExtHelp();
    static void         DisableExtHelp();
    static bool     IsExtHelpEnabled();
    static bool     StartExtHelp();
    static bool     EndExtHelp();

    static void         EnableBalloonHelp();
    static void         DisableBalloonHelp();
    static bool     IsBalloonHelpEnabled();
    static bool     ShowBalloon( Window* pParent,
                                     const Point& rScreenPos,
                                     const OUString& rHelpText );
    static bool     ShowBalloon( Window* pParent,
                                     const Point& rScreenPos,
                                     const Rectangle&,
                                     const OUString& rHelpText );

    static void         EnableQuickHelp();
    static void         DisableQuickHelp();
    static bool     IsQuickHelpEnabled();
    static bool     ShowQuickHelp( Window* pParent,
                                       const Rectangle& rScreenRect,
                                       const OUString& rHelpText,
                                       const OUString& rLongHelpText,
                                       sal_uInt16 nStyle = 0 );
    static bool     ShowQuickHelp( Window* pParent,
                                       const Rectangle& rScreenRect,
                                       const OUString& rHelpText,
                                       sal_uInt16 nStyle = 0 )
                            { return Help::ShowQuickHelp( pParent, rScreenRect, rHelpText, OUString(), nStyle ); }

    static void         HideBalloonAndQuickHelp();

    static sal_uLong    ShowTip( Window* pParent,
                                 const Rectangle& rScreenRect,
                                 const OUString& rText, sal_uInt16 nStyle = 0 );
    static void         UpdateTip( sal_uLong nId,
                                   Window* pParent,
                                   const Rectangle& rScreenRect,
                                   const OUString& rText );
    static void         HideTip( sal_uLong nId );
};

#endif // INCLUDED_VCL_HELP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
