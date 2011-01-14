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


