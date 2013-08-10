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

#ifndef _EDITSYNTAXHIGHLIGHTER_HXX
#define _EDITSYNTAXHIGHLIGHTER_HXX

#include <comphelper/syntaxhighlight.hxx>
#include <svtools/svmedit.hxx>
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
        ~MultiLineEditSyntaxHighlight();

        virtual void UpdateData();
        virtual void SetText(const OUString& rNewText);
        virtual void SetText( const OUString& rStr, const Selection& rNewSelection )
                    { SetText( rStr ); SetSelection( rNewSelection ); }

        Color GetColorValue(TokenTypes aToken);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
