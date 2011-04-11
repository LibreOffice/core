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

#ifndef _EDITSYNTAXHIGHLIGHTER_HXX
#define _EDITSYNTAXHIGHLIGHTER_HXX

#include <svtools/svmedit.hxx>
#include <svtools/syntaxhighlight.hxx>
#include <svtools/svtdllapi.h>
#include <svtools/colorcfg.hxx>

class SVT_DLLPUBLIC MultiLineEditSyntaxHighlight : public MultiLineEdit
{
    private:
        bool                mbDoBracketHilight;
        SyntaxHighlighter   aHighlighter;
        svtools::ColorConfig m_aColorConfig;

    private:
        virtual void DoBracketHilight(sal_uInt16 aKey);

    protected:
        virtual long PreNotify( NotifyEvent& rNEvt );

    public:
        MultiLineEditSyntaxHighlight( Window* pParent, WinBits nWinStyle = WB_LEFT | WB_BORDER , HighlighterLanguage aLanguage = HIGHLIGHT_SQL);
        MultiLineEditSyntaxHighlight( Window* pParent, const ResId& rResId , HighlighterLanguage aLanguage = HIGHLIGHT_SQL);
        ~MultiLineEditSyntaxHighlight();

        void EnableBracketHilight(bool aHilight = true);
        bool IsBracketHilight();
        virtual void UpdateData();
        virtual void SetText(const String& rNewText);
        virtual void SetText( const XubString& rStr, const Selection& rNewSelection )
                    { SetText( rStr ); SetSelection( rNewSelection ); }

        Color GetColorValue(TokenTypes aToken);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
