/*************************************************************************
 *
 *  $RCSfile: FieldControls.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: oj $ $Date: 2001-03-14 10:35:10 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the License); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an AS IS basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef DBAUI_FIELDCONTROLS_HXX
#define DBAUI_FIELDCONTROLS_HXX

#ifndef _SV_FIELD_HXX
#include <vcl/field.hxx>
#endif
#ifndef _SV_LSTBOX_HXX
#include <vcl/lstbox.hxx>
#endif
#ifndef _DBAUI_MODULE_DBU_HXX_
#include "moduledbu.hxx"
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef DBAUI_SQLNAMEEDIT_HXX
#include "SqlNameEdit.hxx"
#endif


namespace dbaui
{
    class OSpecialReadOnly
    {
    protected:
        void SetSpecialReadOnly(BOOL _bReadOnly,Window *pWin)
        {
            StyleSettings aSystemStyle = Application::GetSettings().GetStyleSettings();
            const Color& rNewColor = _bReadOnly ? aSystemStyle.GetDialogColor() : aSystemStyle.GetFieldColor();
            pWin->SetBackground(Wallpaper(rNewColor));
            pWin->SetControlBackground(rNewColor);
        }
    public:
        virtual void SetSpecialReadOnly(BOOL _bReadOnly) = 0;
    };
    //==================================================================
    class OPropColumnEditCtrl : public OSQLNameEdit
                                ,public OSpecialReadOnly
    {
        short   m_nPos;
        String  m_strHelpText;
    public:
        inline OPropColumnEditCtrl(Window* pParent, ::rtl::OUString& _rAllowedChars, INT32 nHelpId, short nPosition = -1, WinBits nWinStyle = 0);

        inline BOOL IsModified() { return GetText() != GetSavedValue(); }

        short GetPos() const { return m_nPos; }
        String GetHelp() const { return m_strHelpText; }

        virtual void SetSpecialReadOnly(BOOL _bReadOnly)
        {
            SetReadOnly(_bReadOnly);
            OSpecialReadOnly::SetSpecialReadOnly(_bReadOnly,this);
        }
    };
    inline OPropColumnEditCtrl::OPropColumnEditCtrl(Window* pParent,
                                                    ::rtl::OUString& _rAllowedChars,
                                                    INT32 nHelpId,
                                                    short nPosition,
                                                    WinBits nWinStyle)
        :OSQLNameEdit(pParent, _rAllowedChars,nWinStyle)
        ,m_nPos(nPosition)
    {
        m_strHelpText = String(ModuleRes(nHelpId));
    }
    //==================================================================
    class OPropEditCtrl :   public Edit
                            ,public OSpecialReadOnly
    {
        short   m_nPos;
        String  m_strHelpText;

    public:
        inline OPropEditCtrl(Window* pParent, INT32 nHelpId, short nPosition = -1, WinBits nWinStyle = 0);

        inline BOOL IsModified() { return GetText() != GetSavedValue(); }

        short GetPos() const { return m_nPos; }
        String GetHelp() const { return m_strHelpText; }

        virtual void SetSpecialReadOnly(BOOL _bReadOnly)
        {
            SetReadOnly(_bReadOnly);
            OSpecialReadOnly::SetSpecialReadOnly(_bReadOnly,this);
        }
    };

    inline OPropEditCtrl::OPropEditCtrl(Window* pParent, INT32 nHelpId, short nPosition, WinBits nWinStyle)
        :Edit(pParent, nWinStyle)
        ,m_nPos(nPosition)
    {
        m_strHelpText = String(ModuleRes(nHelpId));
    }

    //==================================================================
    class OPropNumericEditCtrl : public NumericField
                                ,public OSpecialReadOnly
    {
        short   m_nPos;
        String  m_strHelpText;

    public:
        inline OPropNumericEditCtrl(Window* pParent, INT32 nHelpId, short nPosition = -1, WinBits nWinStyle = 0);

        inline BOOL IsModified() { return GetText() != GetSavedValue(); }

        short GetPos() const { return m_nPos; }
        String GetHelp() const { return m_strHelpText; }

        virtual void SetSpecialReadOnly(BOOL _bReadOnly)
        {
            SetReadOnly(_bReadOnly);
            OSpecialReadOnly::SetSpecialReadOnly(_bReadOnly,this);
        }
    };

    inline OPropNumericEditCtrl::OPropNumericEditCtrl(Window* pParent, INT32 nHelpId, short nPosition, WinBits nWinStyle)
        :NumericField(pParent, nWinStyle)
        ,m_nPos(nPosition)
    {
        m_strHelpText = String(ModuleRes(nHelpId));
    }

    //==================================================================
    class OPropListBoxCtrl : public ListBox
                            ,public OSpecialReadOnly
    {
        short   m_nPos;
        String  m_strHelpText;

    public:
        inline OPropListBoxCtrl(Window* pParent, INT32 nHelpId, short nPosition = -1, WinBits nWinStyle = 0);

        inline BOOL IsModified() { return GetSelectEntryPos() != GetSavedValue(); }

        short GetPos() const { return m_nPos; }
        String GetHelp() const { return m_strHelpText; }

        virtual void SetSpecialReadOnly(BOOL _bReadOnly)
        {
            SetReadOnly(_bReadOnly);
            OSpecialReadOnly::SetSpecialReadOnly(_bReadOnly,this);
        }
    };

    inline OPropListBoxCtrl::OPropListBoxCtrl(Window* pParent, INT32 nHelpId, short nPosition, WinBits nWinStyle)
        :ListBox(pParent, nWinStyle)
        ,m_nPos(nPosition)
    {
        m_strHelpText = String(ModuleRes(nHelpId));
    }
}
#endif // DBAUI_FIELDCONTROLS_HXX



