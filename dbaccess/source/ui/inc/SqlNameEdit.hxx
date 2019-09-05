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
#ifndef INCLUDED_DBACCESS_SOURCE_UI_INC_SQLNAMEEDIT_HXX
#define INCLUDED_DBACCESS_SOURCE_UI_INC_SQLNAMEEDIT_HXX

#include <vcl/edit.hxx>
#include <vcl/combobox.hxx>
#include <vcl/weld.hxx>

namespace dbaui
{
    class OSQLNameChecker
    {
        OUString m_sAllowedChars;
        bool        m_bCheck;           // true when we should check for invalid chars
    public:
        OSQLNameChecker(const OUString& _rAllowedChars)
            :m_sAllowedChars(_rAllowedChars)
            ,m_bCheck(true)
        {
        }

        void setAllowedChars(const OUString& _rAllowedChars)
        {
            m_sAllowedChars = _rAllowedChars;
        }
        void setCheck(bool _bCheck)
        {
            m_bCheck = _bCheck;
        }
        bool checkString(const OUString& _sToCheck,OUString& _rsCorrected);
    };

    class OSQLNameEdit : public Edit
                        ,public OSQLNameChecker
    {
    public:
        OSQLNameEdit(vcl::Window* _pParent,WinBits nStyle = WB_BORDER, const OUString& _rAllowedChars = OUString())
            : Edit(_pParent,nStyle)
            , OSQLNameChecker(_rAllowedChars)
        {
        }

        // Edit overrides
        virtual void Modify() override;
    };

    class OSQLNameEntry : public OSQLNameChecker
    {
    private:
        std::unique_ptr<weld::Entry> m_xEntry;
    public:
        OSQLNameEntry(std::unique_ptr<weld::Entry> xEntry, const OUString& _rAllowedChars = OUString())
            : OSQLNameChecker(_rAllowedChars)
            , m_xEntry(std::move(xEntry))
        {
        }

        weld::Widget* GetWidget() { return m_xEntry.get(); }

        OUString get_text() const { return m_xEntry->get_text(); }
        void set_text(const OUString& rText) { m_xEntry->set_text(rText); }
        void set_max_length(int nLen) { m_xEntry->set_max_length(nLen); }
        void hide() { m_xEntry->hide(); }
        void show() { m_xEntry->show(); }
        void save_value() { m_xEntry->save_value(); }

        //TODO virtual void Modify() override;
    };

}
#endif // INCLUDED_DBACCESS_SOURCE_UI_INC_SQLNAMEEDIT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
