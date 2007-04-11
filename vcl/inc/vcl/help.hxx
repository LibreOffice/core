/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: help.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 17:56:21 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _SV_HELP_HXX
#define _SV_HELP_HXX

#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif

#ifndef _SV_SV_H
#include <vcl/sv.h>
#endif

#ifndef _VCL_DLLAPI_H
#include <vcl/dllapi.h>
#endif

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
#define HELP_INDEX          ((ULONG)0xFFFFFFFF)
#define HELP_HELPONHELP     ((ULONG)0xFFFFFFFE)

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
