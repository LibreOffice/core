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
#ifndef INCLUDED_DBACCESS_SOURCE_UI_INC_FIELDCONTROLS_HXX
#define INCLUDED_DBACCESS_SOURCE_UI_INC_FIELDCONTROLS_HXX

#include <vcl/field.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/svapp.hxx>
#include "SqlNameEdit.hxx"
#include "moduledbu.hxx"

namespace dbaui
{

    class OPropColumnEditCtrl : public OSQLNameEdit
    {
        OModuleClient m_aModuleClient;
        short                m_nPos;
        OUString             m_strHelpText;
    public:
        OPropColumnEditCtrl(Window* pParent, OUString& _rAllowedChars, sal_uInt16 nHelpId, short nPosition = -1, WinBits nWinStyle = 0);

        virtual bool IsModified() const SAL_OVERRIDE { return GetText() != GetSavedValue(); }

        short GetPos() const { return m_nPos; }
        OUString GetHelp() const { return m_strHelpText; }

        virtual void SetSpecialReadOnly(sal_Bool _bReadOnly);

    };

    class OPropEditCtrl :   public Edit
    {
        OModuleClient m_aModuleClient;
        short                m_nPos;
        OUString             m_strHelpText;

    public:
        OPropEditCtrl(Window* pParent, sal_uInt16 nHelpId, short nPosition = -1, WinBits nWinStyle = 0);
        OPropEditCtrl(Window* pParent, sal_uInt16 nHelpId, const ResId& _rRes,short nPosition = -1);

        virtual bool IsModified() const SAL_OVERRIDE { return GetText() != GetSavedValue(); }

        short GetPos() const { return m_nPos; }
        OUString GetHelp() const { return m_strHelpText; }

        virtual void SetSpecialReadOnly(sal_Bool _bReadOnly);
    };

    class OPropNumericEditCtrl : public NumericField
    {
        short     m_nPos;
        OUString  m_strHelpText;

    public:
        OPropNumericEditCtrl(Window* pParent, sal_uInt16 nHelpId, short nPosition = -1, WinBits nWinStyle = 0);
        OPropNumericEditCtrl(Window* pParent, sal_uInt16 nHelpId, const ResId& _rRes,short nPosition = -1);

        virtual bool IsModified() const SAL_OVERRIDE { return GetText() != GetSavedValue(); }

        short GetPos() const { return m_nPos; }
        OUString GetHelp() const { return m_strHelpText; }

        virtual void SetSpecialReadOnly(sal_Bool _bReadOnly);

    };

    class OPropListBoxCtrl : public ListBox
    {
        short     m_nPos;
        OUString  m_strHelpText;

    public:
        OPropListBoxCtrl(Window* pParent, sal_uInt16 nHelpId, short nPosition = -1, WinBits nWinStyle = 0);
        OPropListBoxCtrl(Window* pParent, sal_uInt16 nHelpId, const ResId& _rRes,short nPosition = -1);

        sal_Bool IsModified() const { return GetSelectEntryPos() != GetSavedValue(); }

        short GetPos() const { return m_nPos; }
        OUString GetHelp() const { return m_strHelpText; }

        virtual void SetSpecialReadOnly(sal_Bool _bReadOnly);

    };

}
#endif // INCLUDED_DBACCESS_SOURCE_UI_INC_FIELDCONTROLS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
