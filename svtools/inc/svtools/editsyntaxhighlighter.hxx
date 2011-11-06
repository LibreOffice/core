/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
