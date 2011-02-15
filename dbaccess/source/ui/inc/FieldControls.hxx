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
#ifndef DBAUI_FIELDCONTROLS_HXX
#define DBAUI_FIELDCONTROLS_HXX

#ifndef _SV_FIELD_HXX
#include <vcl/field.hxx>
#endif
#ifndef _SV_LSTBOX_HXX
#include <vcl/lstbox.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef DBAUI_SQLNAMEEDIT_HXX
#include "SqlNameEdit.hxx"
#endif
#ifndef _DBAUI_MODULE_DBU_HXX_
#include "moduledbu.hxx"
#endif


namespace dbaui
{
    namespace
    {
        void lcl_setSpecialReadOnly( sal_Bool _bReadOnly, Window* _pWin )
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
        OModuleClient m_aModuleClient;
        short                m_nPos;
        String               m_strHelpText;
    public:
        inline OPropColumnEditCtrl(Window* pParent, ::rtl::OUString& _rAllowedChars, sal_uInt16 nHelpId, short nPosition = -1, WinBits nWinStyle = 0);

        inline sal_Bool IsModified() const { return GetText() != GetSavedValue(); }

        short GetPos() const { return m_nPos; }
        String GetHelp() const { return m_strHelpText; }

        virtual void SetSpecialReadOnly(sal_Bool _bReadOnly)
        {
            SetReadOnly(_bReadOnly);
            lcl_setSpecialReadOnly(_bReadOnly,this);
        }
    };
    inline OPropColumnEditCtrl::OPropColumnEditCtrl(Window* pParent,
                                                    ::rtl::OUString& _rAllowedChars,
                                                    sal_uInt16 nHelpId,
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
        OModuleClient m_aModuleClient;
        short                m_nPos;
        String               m_strHelpText;

    public:
        inline OPropEditCtrl(Window* pParent, sal_uInt16 nHelpId, short nPosition = -1, WinBits nWinStyle = 0);
        inline OPropEditCtrl(Window* pParent, sal_uInt16 nHelpId, const ResId& _rRes,short nPosition = -1);

        inline sal_Bool IsModified() const { return GetText() != GetSavedValue(); }

        short GetPos() const { return m_nPos; }
        String GetHelp() const { return m_strHelpText; }

        virtual void SetSpecialReadOnly(sal_Bool _bReadOnly)
        {
            SetReadOnly(_bReadOnly);
            lcl_setSpecialReadOnly(_bReadOnly,this);
        }
    };

    inline OPropEditCtrl::OPropEditCtrl(Window* pParent, sal_uInt16 nHelpId, short nPosition, WinBits nWinStyle)
        :Edit(pParent, nWinStyle)
        ,m_nPos(nPosition)
    {
        m_strHelpText =String(ModuleRes(nHelpId));
    }
    inline OPropEditCtrl::OPropEditCtrl(Window* pParent, sal_uInt16 nHelpId, const ResId& _rRes,short nPosition)
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
        inline OPropNumericEditCtrl(Window* pParent, sal_uInt16 nHelpId, short nPosition = -1, WinBits nWinStyle = 0);
        inline OPropNumericEditCtrl(Window* pParent, sal_uInt16 nHelpId, const ResId& _rRes,short nPosition = -1);

        inline sal_Bool IsModified() const { return GetText() != GetSavedValue(); }

        short GetPos() const { return m_nPos; }
        String GetHelp() const { return m_strHelpText; }

        virtual void SetSpecialReadOnly(sal_Bool _bReadOnly)
        {
            SetReadOnly(_bReadOnly);
            lcl_setSpecialReadOnly(_bReadOnly,this);
        }
    };

    inline OPropNumericEditCtrl::OPropNumericEditCtrl(Window* pParent, sal_uInt16 nHelpId, short nPosition, WinBits nWinStyle)
        :NumericField(pParent, nWinStyle)
        ,m_nPos(nPosition)
    {
        m_strHelpText =String(ModuleRes(nHelpId));
    }
    inline OPropNumericEditCtrl::OPropNumericEditCtrl(Window* pParent, sal_uInt16 nHelpId, const ResId& _rRes,short nPosition)
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
        inline OPropListBoxCtrl(Window* pParent, sal_uInt16 nHelpId, short nPosition = -1, WinBits nWinStyle = 0);
        inline OPropListBoxCtrl(Window* pParent, sal_uInt16 nHelpId, const ResId& _rRes,short nPosition = -1);

        inline sal_Bool IsModified() const { return GetSelectEntryPos() != GetSavedValue(); }

        short GetPos() const { return m_nPos; }
        String GetHelp() const { return m_strHelpText; }

        virtual void SetSpecialReadOnly(sal_Bool _bReadOnly)
        {
            SetReadOnly(_bReadOnly);
            lcl_setSpecialReadOnly(_bReadOnly,this);
        }
    };

    inline OPropListBoxCtrl::OPropListBoxCtrl(Window* pParent, sal_uInt16 nHelpId, short nPosition, WinBits nWinStyle)
        :ListBox(pParent, nWinStyle)
        ,m_nPos(nPosition)
    {
        m_strHelpText =String(ModuleRes(nHelpId));
    }
    inline OPropListBoxCtrl::OPropListBoxCtrl(Window* pParent, sal_uInt16 nHelpId, const ResId& _rRes,short nPosition)
        :ListBox(pParent, _rRes)
        ,m_nPos(nPosition)
    {
        m_strHelpText =String(ModuleRes(nHelpId));
    }
}
#endif // DBAUI_FIELDCONTROLS_HXX



