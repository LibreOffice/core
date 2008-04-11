/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: help.hxx,v $
 * $Revision: 1.4 $
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

#define QUICKHELP_LEFT      ((USHORT)0x0001)
#define QUICKHELP_CENTER    ((USHORT)0x0002)
#define QUICKHELP_RIGHT     ((USHORT)0x0004)
#define QUICKHELP_TOP       ((USHORT)0x0008)
#define QUICKHELP_VCENTER   ((USHORT)0x0010)
#define QUICKHELP_BOTTOM    ((USHORT)0x0020)
#define QUICKHELP_NOAUTOPOS (QUICKHELP_LEFT | QUICKHELP_CENTER | QUICKHELP_RIGHT | QUICKHELP_TOP | QUICKHELP_VCENTER | QUICKHELP_BOTTOM)
#define QUICKHELP_CTRLTEXT  ((USHORT)0x0040)
#define QUICKHELP_BIDI_RTL  ((USHORT)0x8000)

// By changes you must also change: rsc/vclrsc.hxx
#define OOO_HELP_INDEX          ((ULONG)0xFFFFFFFF)
#define OOO_HELP_HELPONHELP     ((ULONG)0xFFFFFFFE)

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

    virtual BOOL        Start( ULONG nHelpId, const Window* pWindow  );
    virtual BOOL        Start( const XubString& rKeyWord, const Window* pWindow );
    virtual XubString   GetHelpText( ULONG nHelpId, const Window* pWindow );
    virtual XubString   GetHelpText( const String& aHelpURL, const Window* pWindow );

    static void         EnableContextHelp();
    static void         DisableContextHelp();
    static BOOL         IsContextHelpEnabled();
    static BOOL         StartContextHelp();

    static void         EnableExtHelp();
    static void         DisableExtHelp();
    static BOOL         IsExtHelpEnabled();
    static BOOL         StartExtHelp();
    static BOOL         EndExtHelp();
    static BOOL         IsExtHelpActive();

    static void         EnableBalloonHelp();
    static void         DisableBalloonHelp();
    static BOOL         IsBalloonHelpEnabled();
    static BOOL         ShowBalloon( Window* pParent,
                                     const Point& rScreenPos,
                                     const XubString& rHelpText );
    static BOOL         ShowBalloon( Window* pParent,
                                     const Point& rScreenPos,
                                     const Rectangle&,
                                     const XubString& rHelpText );

    static void         EnableQuickHelp();
    static void         DisableQuickHelp();
    static BOOL         IsQuickHelpEnabled();
    static BOOL         ShowQuickHelp( Window* pParent,
                                       const Rectangle& rScreenRect,
                                       const XubString& rHelpText,
                                       const XubString& rLongHelpText,
                                       USHORT nStyle = 0 );
    static BOOL         ShowQuickHelp( Window* pParent,
                                       const Rectangle& rScreenRect,
                                       const XubString& rHelpText,
                                       USHORT nStyle = 0 )
                            { return Help::ShowQuickHelp( pParent, rScreenRect, rHelpText, XubString(), nStyle ); }

    static ULONG        ShowTip( Window* pParent,
                                 const Rectangle& rScreenRect,
                                 const XubString& rText, USHORT nStyle = 0 );
    static void         HideTip( ULONG nId );
};

#endif  // _SV_HELP_HXX
