/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: FieldControls.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 03:11:22 $
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
    namespace
    {
        void lcl_setSpecialReadOnly( BOOL _bReadOnly, Window* _pWin )
        {
            StyleSettings aSystemStyle = Application::GetSettings().GetStyleSettings();
            const Color& rNewColor = _bReadOnly ? aSystemStyle.GetDialogColor() : aSystemStyle.GetFieldColor();
            _pWin->SetBackground(Wallpaper(rNewColor));
            _pWin->SetControlBackground(rNewColor);
        }
    }

    //==================================================================
    class OPropColumnEditCtrl : public OSQLNameEdit
    {
        short   m_nPos;
        String  m_strHelpText;
    public:
        inline OPropColumnEditCtrl(Window* pParent, ::rtl::OUString& _rAllowedChars, USHORT nHelpId, short nPosition = -1, WinBits nWinStyle = 0);

        inline BOOL IsModified() const { return GetText() != GetSavedValue(); }

        short GetPos() const { return m_nPos; }
        String GetHelp() const { return m_strHelpText; }

        virtual void SetSpecialReadOnly(BOOL _bReadOnly)
        {
            SetReadOnly(_bReadOnly);
            lcl_setSpecialReadOnly(_bReadOnly,this);
        }
    };
    inline OPropColumnEditCtrl::OPropColumnEditCtrl(Window* pParent,
                                                    ::rtl::OUString& _rAllowedChars,
                                                    USHORT nHelpId,
                                                    short nPosition,
                                                    WinBits nWinStyle)
        :OSQLNameEdit(pParent, _rAllowedChars,nWinStyle)
        ,m_nPos(nPosition)
    {
        m_strHelpText=String(ModuleRes(nHelpId));
    }
    //==================================================================
    class OPropEditCtrl :   public Edit
    {
        short   m_nPos;
        String  m_strHelpText;

    public:
        inline OPropEditCtrl(Window* pParent, USHORT nHelpId, short nPosition = -1, WinBits nWinStyle = 0);
        inline OPropEditCtrl(Window* pParent, USHORT nHelpId, const ResId& _rRes,short nPosition = -1);

        inline BOOL IsModified() const { return GetText() != GetSavedValue(); }

        short GetPos() const { return m_nPos; }
        String GetHelp() const { return m_strHelpText; }

        virtual void SetSpecialReadOnly(BOOL _bReadOnly)
        {
            SetReadOnly(_bReadOnly);
            lcl_setSpecialReadOnly(_bReadOnly,this);
        }
    };

    inline OPropEditCtrl::OPropEditCtrl(Window* pParent, USHORT nHelpId, short nPosition, WinBits nWinStyle)
        :Edit(pParent, nWinStyle)
        ,m_nPos(nPosition)
    {
        m_strHelpText =String(ModuleRes(nHelpId));
    }
    inline OPropEditCtrl::OPropEditCtrl(Window* pParent, USHORT nHelpId, const ResId& _rRes,short nPosition)
        :Edit(pParent, _rRes)
        ,m_nPos(nPosition)
    {
        m_strHelpText =String(ModuleRes(nHelpId));
    }

    //==================================================================
    class OPropNumericEditCtrl : public NumericField
    {
        short   m_nPos;
        String  m_strHelpText;

    public:
        inline OPropNumericEditCtrl(Window* pParent, USHORT nHelpId, short nPosition = -1, WinBits nWinStyle = 0);
        inline OPropNumericEditCtrl(Window* pParent, USHORT nHelpId, const ResId& _rRes,short nPosition = -1);

        inline BOOL IsModified() const { return GetText() != GetSavedValue(); }

        short GetPos() const { return m_nPos; }
        String GetHelp() const { return m_strHelpText; }

        virtual void SetSpecialReadOnly(BOOL _bReadOnly)
        {
            SetReadOnly(_bReadOnly);
            lcl_setSpecialReadOnly(_bReadOnly,this);
        }
    };

    inline OPropNumericEditCtrl::OPropNumericEditCtrl(Window* pParent, USHORT nHelpId, short nPosition, WinBits nWinStyle)
        :NumericField(pParent, nWinStyle)
        ,m_nPos(nPosition)
    {
        m_strHelpText =String(ModuleRes(nHelpId));
    }
    inline OPropNumericEditCtrl::OPropNumericEditCtrl(Window* pParent, USHORT nHelpId, const ResId& _rRes,short nPosition)
        :NumericField(pParent, _rRes)
        ,m_nPos(nPosition)
    {
        m_strHelpText =String(ModuleRes(nHelpId));
    }

    //==================================================================
    class OPropListBoxCtrl : public ListBox
    {
        short   m_nPos;
        String  m_strHelpText;

    public:
        inline OPropListBoxCtrl(Window* pParent, USHORT nHelpId, short nPosition = -1, WinBits nWinStyle = 0);
        inline OPropListBoxCtrl(Window* pParent, USHORT nHelpId, const ResId& _rRes,short nPosition = -1);

        inline BOOL IsModified() const { return GetSelectEntryPos() != GetSavedValue(); }

        short GetPos() const { return m_nPos; }
        String GetHelp() const { return m_strHelpText; }

        virtual void SetSpecialReadOnly(BOOL _bReadOnly)
        {
            SetReadOnly(_bReadOnly);
            lcl_setSpecialReadOnly(_bReadOnly,this);
        }
    };

    inline OPropListBoxCtrl::OPropListBoxCtrl(Window* pParent, USHORT nHelpId, short nPosition, WinBits nWinStyle)
        :ListBox(pParent, nWinStyle)
        ,m_nPos(nPosition)
    {
        m_strHelpText =String(ModuleRes(nHelpId));
    }
    inline OPropListBoxCtrl::OPropListBoxCtrl(Window* pParent, USHORT nHelpId, const ResId& _rRes,short nPosition)
        :ListBox(pParent, _rRes)
        ,m_nPos(nPosition)
    {
        m_strHelpText =String(ModuleRes(nHelpId));
    }
}
#endif // DBAUI_FIELDCONTROLS_HXX



