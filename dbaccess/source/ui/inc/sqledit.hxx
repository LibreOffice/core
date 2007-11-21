/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sqledit.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-21 16:05:46 $
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
#ifndef DBAUI_SQLEDIT_HXX
#define DBAUI_SQLEDIT_HXX

#ifndef _SVEDIT_HXX
#include <svtools/svmedit.hxx>
#endif

namespace dbaui
{
    class OQueryTextView;
    class OSqlEdit : public MultiLineEdit
    {
    private:
        Timer   m_timerInvalidate;
        Timer   m_timerUndoActionCreation;
        Link    m_lnkTextModifyHdl;
        String  m_strOrigText;      // wird beim Undo wiederhergestellt
        OQueryTextView* m_pView;
        BOOL    m_bAccelAction;     // Wird bei Cut, Copy, Paste gesetzt
        BOOL    m_bStopTimer;

        DECL_LINK(OnUndoActionTimer, void*);
        DECL_LINK(OnInvalidateTimer, void*);

    protected:
        virtual void KeyInput( const KeyEvent& rKEvt );
        virtual void GetFocus();

        DECL_LINK(ModifyHdl, void*);

    public:
        OSqlEdit( OQueryTextView* pParent,  WinBits nWinStyle = WB_LEFT | WB_VSCROLL |WB_BORDER);
        virtual ~OSqlEdit();

        // Edit overridables
        virtual void SetText(const String& rNewText);
        using MultiLineEdit::SetText;

        // own functionality
        BOOL IsInAccelAct();

        void SetTextModifyHdl(const Link& lnk) { m_lnkTextModifyHdl = lnk; }
            // bitte nicht SetModifyHdl benutzen, den brauche ich selber, der hier wird von dem damit gesetzten Handler
            // gerufen
            // der Link bekommt einen Pointer-to-string, der nach dem Link nicht mehr gueltig ist

        void stopTimer();
        void startTimer();
    };
}

#endif // DBAUI_SQLEDIT_HXX


