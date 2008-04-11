/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: helpwin.hxx,v $
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

#ifndef _SV_HELPWIN_HXX
#define _SV_HELPWIN_HXX

#include <vcl/floatwin.hxx>
#include <vcl/timer.hxx>

// ------------------
// - HelpTextWindow -
// ------------------

class HelpTextWindow : public FloatingWindow
{
private:
    Point               maPos;
    Rectangle           maHelpArea; // Wenn naechste Hilfe fuers gleiche Rectangle, gleicher Text, dann Fenster stehen lassen

    Rectangle           maTextRect; // Bei umgebrochenen Text in QuickHelp

    String              maHelpText;
    String              maStatusText;

    Timer               maShowTimer;
    Timer               maHideTimer;

    USHORT              mnHelpWinStyle;
    USHORT              mnStyle;

protected:
                        DECL_LINK( TimerHdl, Timer* );
    virtual void        Paint( const Rectangle& );
    virtual void        RequestHelp( const HelpEvent& rHEvt );
    virtual String      GetText() const;
    void                ImplShow();

public:
                        HelpTextWindow( Window* pParent, const String& rText, USHORT nHelpWinStyle, USHORT nStyle );
                        ~HelpTextWindow();

    const String&       GetHelpText() const { return maHelpText; }
    void                SetHelpText( const String& rHelpText );
    USHORT              GetWinStyle() const { return mnHelpWinStyle; }

    // Nur merken:
    void                SetStatusText( const String& rStatusText ) { maStatusText = rStatusText; }
    void                SetHelpArea( const Rectangle& rRect ) { maHelpArea = rRect; }

    void                ShowHelp( USHORT nDelayMode );

    Size                CalcOutSize() const;
    const Rectangle&    GetHelpArea() const { return maHelpArea; }

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > CreateAccessible();

    BOOL                RegisterAccessibleParent();
    void                RevokeAccessibleParent();
};

void ImplShowHelpWindow( Window* pParent, USHORT nHelpWinStyle, USHORT nStyle,
        const String& rHelpText, const String& rStatusText,
        const Point& rScreenPos, const Rectangle* pHelpArea = NULL );
void ImplDestroyHelpWindow( bool bUpdateHideTime );
void ImplSetHelpWindowPos( Window* pHelpWindow, USHORT nHelpWinStyle, USHORT nStyle,
                            const Point& rPos, const Rectangle* pHelpArea );

#endif // _SV_HELPWIN_HXX
