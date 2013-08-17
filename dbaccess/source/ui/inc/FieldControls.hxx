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
#ifndef DBAUI_FIELDCONTROLS_HXX
#define DBAUI_FIELDCONTROLS_HXX

#include <vcl/field.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/svapp.hxx>
#include "SqlNameEdit.hxx"
#include "moduledbu.hxx"

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

    class OPropColumnEditCtrl : public OSQLNameEdit
    {
        OModuleClient m_aModuleClient;
        short                m_nPos;
        String               m_strHelpText;
    public:
        inline OPropColumnEditCtrl(Window* pParent, OUString& _rAllowedChars, sal_uInt16 nHelpId, short nPosition = -1, WinBits nWinStyle = 0);

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
                                                    OUString& _rAllowedChars,
                                                    sal_uInt16 nHelpId,
                                                    short nPosition,
                                                    WinBits nWinStyle)
        :OSQLNameEdit(pParent, _rAllowedChars,nWinStyle)
        ,m_nPos(nPosition)
    {
        m_strHelpText=String(ModuleRes(nHelpId));
    }
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
