/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
    Rectangle           maHelpArea; // If next Help for the same rectangle w/ same text, then keep window

    Rectangle           maTextRect; // For wrapped text in QuickHelp

    OUString            maHelpText;
    OUString            maStatusText;

    Timer               maShowTimer;
    Timer               maHideTimer;

    sal_uInt16              mnHelpWinStyle;
    sal_uInt16              mnStyle;

protected:
                        DECL_LINK( TimerHdl, Timer* );
    virtual void        Paint( const Rectangle& );
    virtual void        RequestHelp( const HelpEvent& rHEvt );
    virtual String      GetText() const;
    void                ImplShow();

public:
                        HelpTextWindow( Window* pParent, const OUString& rText, sal_uInt16 nHelpWinStyle, sal_uInt16 nStyle );
                        ~HelpTextWindow();

    const OUString&     GetHelpText() const { return maHelpText; }
    void                SetHelpText( const OUString& rHelpText );
    sal_uInt16          GetWinStyle() const { return mnHelpWinStyle; }
    sal_uInt16          GetStyle() const { return mnStyle; }

    // Nur merken:
    void                SetStatusText( const OUString& rStatusText ) { maStatusText = rStatusText; }
    void                SetHelpArea( const Rectangle& rRect ) { maHelpArea = rRect; }

    void                ShowHelp( sal_uInt16 nDelayMode );

    Size                CalcOutSize() const;
    const Rectangle&    GetHelpArea() const { return maHelpArea; }

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > CreateAccessible();
};

void ImplShowHelpWindow( Window* pParent, sal_uInt16 nHelpWinStyle, sal_uInt16 nStyle,
        const OUString& rHelpText, const OUString& rStatusText,
        const Point& rScreenPos, const Rectangle* pHelpArea = NULL );
void ImplDestroyHelpWindow( bool bUpdateHideTime );
void ImplSetHelpWindowPos( Window* pHelpWindow, sal_uInt16 nHelpWinStyle, sal_uInt16 nStyle,
                            const Point& rPos, const Rectangle* pHelpArea );

#endif // _SV_HELPWIN_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
