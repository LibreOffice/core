/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _SV_HELP_HXX
#define _SV_HELP_HXX

#include <tools/string.hxx>
#include <vcl/sv.h>
#include <vcl/dllapi.h>

class Point;
class Rectangle;
class Window;

// --------------
// - Help-Types -
// --------------

#define QUICKHELP_LEFT      ((sal_uInt16)0x0001)
#define QUICKHELP_CENTER    ((sal_uInt16)0x0002)
#define QUICKHELP_RIGHT     ((sal_uInt16)0x0004)
#define QUICKHELP_TOP       ((sal_uInt16)0x0008)
#define QUICKHELP_VCENTER   ((sal_uInt16)0x0010)
#define QUICKHELP_BOTTOM    ((sal_uInt16)0x0020)
#define QUICKHELP_NOAUTOPOS (QUICKHELP_LEFT | QUICKHELP_CENTER | QUICKHELP_RIGHT | QUICKHELP_TOP | QUICKHELP_VCENTER | QUICKHELP_BOTTOM)
#define QUICKHELP_CTRLTEXT  ((sal_uInt16)0x0040)
#define QUICKHELP_NOEVADEPOINTER ((sal_uInt16)0x4000)
#define QUICKHELP_BIDI_RTL  ((sal_uInt16)0x8000)

// By changes you must also change: rsc/vclrsc.hxx
#define OOO_HELP_INDEX          ".help:index"
#define OOO_HELP_HELPONHELP     ".help:helponhelp"

// --------
// - Help -
// --------

class VCL_DLLPUBLIC Help
{
private:
    String              maHelpFile;

public:
                        Help();
    virtual             ~Help();

    void                SetHelpFile( const String& rFileName ) { maHelpFile = rFileName; }
    const String&       GetHelpFile() const { return maHelpFile; }

    virtual sal_Bool        Start( const XubString& rHelpId, const Window* pWindow );
    virtual sal_Bool        SearchKeyword( const XubString& rKeyWord );
    virtual void        OpenHelpAgent( const rtl::OString& rHelpId );
    virtual XubString   GetHelpText( const String& aHelpURL, const Window* pWindow );

    static void         EnableContextHelp();
    static void         DisableContextHelp();
    static sal_Bool         IsContextHelpEnabled();
    static sal_Bool         StartContextHelp();

    static void         EnableExtHelp();
    static void         DisableExtHelp();
    static sal_Bool         IsExtHelpEnabled();
    static sal_Bool         StartExtHelp();
    static sal_Bool         EndExtHelp();
    static sal_Bool         IsExtHelpActive();

    static void         EnableBalloonHelp();
    static void         DisableBalloonHelp();
    static sal_Bool         IsBalloonHelpEnabled();
    static sal_Bool         ShowBalloon( Window* pParent,
                                     const Point& rScreenPos,
                                     const XubString& rHelpText );
    static sal_Bool         ShowBalloon( Window* pParent,
                                     const Point& rScreenPos,
                                     const Rectangle&,
                                     const XubString& rHelpText );

    static void         EnableQuickHelp();
    static void         DisableQuickHelp();
    static sal_Bool         IsQuickHelpEnabled();
    static sal_Bool         ShowQuickHelp( Window* pParent,
                                       const Rectangle& rScreenRect,
                                       const XubString& rHelpText,
                                       const XubString& rLongHelpText,
                                       sal_uInt16 nStyle = 0 );
    static sal_Bool         ShowQuickHelp( Window* pParent,
                                       const Rectangle& rScreenRect,
                                       const XubString& rHelpText,
                                       sal_uInt16 nStyle = 0 )
                            { return Help::ShowQuickHelp( pParent, rScreenRect, rHelpText, XubString(), nStyle ); }

    static sal_uLong        ShowTip( Window* pParent,
                                 const Rectangle& rScreenRect,
                                 const XubString& rText, sal_uInt16 nStyle = 0 );
    static void         HideTip( sal_uLong nId );
};

#endif  // _SV_HELP_HXX
