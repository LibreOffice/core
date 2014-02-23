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

#ifndef INCLUDED_VCL_INC_HELPWIN_HXX
#define INCLUDED_VCL_INC_HELPWIN_HXX

#include <vcl/floatwin.hxx>
#include <vcl/timer.hxx>


// - HelpTextWindow -


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
    virtual OUString    GetText() const;
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

#endif // INCLUDED_VCL_INC_HELPWIN_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
