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


#ifndef DBAUI_SQLEDIT_HXX
#define DBAUI_SQLEDIT_HXX

#include <svtools/editsyntaxhighlighter.hxx>
#include <svl/lstner.hxx>
#include <svtools/colorcfg.hxx>
#include <unotools/sourceviewconfig.hxx>

namespace dbaui
{
    class OQueryTextView;
    class OSqlEdit : public MultiLineEditSyntaxHighlight, utl::ConfigurationListener
    {
    private:
        Timer                   m_timerInvalidate;
        Timer                   m_timerUndoActionCreation;
        Link                    m_lnkTextModifyHdl;
        String                  m_strOrigText;      // wird beim Undo wiederhergestellt
        OQueryTextView*         m_pView;
        sal_Bool                    m_bAccelAction;     // Wird bei Cut, Copy, Paste gesetzt
        sal_Bool                    m_bStopTimer;
        utl::SourceViewConfig   m_SourceViewConfig;
        svtools::ColorConfig    m_ColorConfig;

        DECL_LINK(OnUndoActionTimer, void*);
        DECL_LINK(OnInvalidateTimer, void*);

    private:
        void            ImplSetFont();

    protected:
        virtual void KeyInput( const KeyEvent& rKEvt );
        virtual void GetFocus();

        DECL_LINK(ModifyHdl, void*);

    public:
        OSqlEdit( OQueryTextView* pParent,  WinBits nWinStyle = WB_LEFT | WB_VSCROLL |WB_BORDER);
        virtual ~OSqlEdit();

        // Edit overridables
        virtual void SetText(const String& rNewText);
        using MultiLineEditSyntaxHighlight::SetText;

        // own functionality
        sal_Bool IsInAccelAct();

        void SetTextModifyHdl(const Link& lnk) { m_lnkTextModifyHdl = lnk; }
            // bitte nicht SetModifyHdl benutzen, den brauche ich selber, der hier wird von dem damit gesetzten Handler
            // gerufen
            // der Link bekommt einen Pointer-to-string, der nach dem Link nicht mehr gueltig ist

        void stopTimer();
        void startTimer();

        virtual void    ConfigurationChanged( utl::ConfigurationBroadcaster*, sal_uInt32 );
        using MultiLineEditSyntaxHighlight::Notify;
    };
}

#endif // DBAUI_SQLEDIT_HXX


