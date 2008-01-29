/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: helpwin.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-29 16:16:00 $
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

#ifndef _SV_HELPWIN_HXX
#define _SV_HELPWIN_HXX

#ifndef _SV_FLOATWIN_HXX
#include <vcl/floatwin.hxx>
#endif

#ifndef _SV_TIMER_HXX
#include <vcl/timer.hxx>
#endif

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
