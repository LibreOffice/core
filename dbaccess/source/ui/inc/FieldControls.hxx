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
#pragma once

#include "SqlNameEdit.hxx"

namespace dbaui
{

    class OPropColumnEditCtrl : public OSQLNameEntry
    {
        short                m_nPos;
        OUString             m_strHelpText;
    public:
        OPropColumnEditCtrl(std::unique_ptr<weld::Entry> xEntry, OUString const & _rAllowedChars, const char* pHelpId, short nPosition);

        short GetPos() const { return m_nPos; }
        const OUString& GetHelp() const { return m_strHelpText; }
    };

    class OPropEditCtrl : public OWidgetBase
    {
        std::unique_ptr<weld::Entry> m_xEntry;
        short                m_nPos;
        OUString             m_strHelpText;

    public:
        OPropEditCtrl(std::unique_ptr<weld::Entry> xEntry, const char* pHelpId, short nPosition);

        void set_text(const OUString& rText) { m_xEntry->set_text(rText); }
        OUString get_text() const { return m_xEntry->get_text(); }
        void set_editable(bool bEditable) { m_xEntry->set_editable(bEditable); }

        virtual void save_value() override { m_xEntry->save_value(); }
        virtual bool get_value_changed_from_saved() const override { return m_xEntry->get_value_changed_from_saved(); }

        short GetPos() const { return m_nPos; }
        const OUString& GetHelp() const { return m_strHelpText; }
    };

    class OPropNumericEditCtrl : public OWidgetBase
    {
        std::unique_ptr<weld::SpinButton> m_xSpinButton;
        short     m_nPos;
        OUString  m_strHelpText;

    public:
        OPropNumericEditCtrl(std::unique_ptr<weld::SpinButton> xSpinButton, const char* pHelpId, short nPosition);

        void set_text(const OUString& rText) { m_xSpinButton->set_text(rText); }
        OUString get_text() const { return m_xSpinButton->get_text(); }

        virtual void save_value() override { m_xSpinButton->save_value(); }
        virtual bool get_value_changed_from_saved() const override { return m_xSpinButton->get_value_changed_from_saved(); }
        void set_digits(int nLen) { m_xSpinButton->set_digits(nLen); }
        void set_min(int nMin) { m_xSpinButton->set_min(nMin); }
        void set_max(int nMax) { m_xSpinButton->set_max(nMax); }
        void set_range(int nMin, int nMax) { m_xSpinButton->set_range(nMin, nMax); }
        int get_value() const { return m_xSpinButton->get_value(); }

        short GetPos() const { return m_nPos; }
        const OUString& GetHelp() const { return m_strHelpText; }

        void set_editable(bool bEditable) { m_xSpinButton->set_editable(bEditable); }
    };

    class OPropListBoxCtrl : public OWidgetBase
    {
        std::unique_ptr<weld::ComboBox> m_xComboBox;
        short     m_nPos;
        OUString  m_strHelpText;

    public:
        OPropListBoxCtrl(std::unique_ptr<weld::ComboBox> xComboBox, const char* pHelpId, short nPosition);
        virtual ~OPropListBoxCtrl() override
        {
            m_xComboBox->clear();
        }

        virtual void save_value() override { m_xComboBox->save_value(); }
        virtual bool get_value_changed_from_saved() const override { return m_xComboBox->get_value_changed_from_saved(); }

        weld::ComboBox& GetComboBox() { return *m_xComboBox; }

        OUString get_active_text() const { return m_xComboBox->get_active_text(); }
        void set_active_text(const OUString &rText) { m_xComboBox->set_active_text(rText); }

        int get_active() const { return m_xComboBox->get_active(); }
        void set_active(int nPos) { m_xComboBox->set_active(nPos); }

        int get_count() const { return m_xComboBox->get_count(); }

        void append_text(const OUString &rText) { m_xComboBox->append_text(rText); }
        void remove_text(const OUString &rText) { m_xComboBox->remove_text(rText); }
        int find_text(const OUString &rText) const { return m_xComboBox->find_text(rText); }

        short GetPos() const { return m_nPos; }
        const OUString& GetHelp() const { return m_strHelpText; }
    };

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
