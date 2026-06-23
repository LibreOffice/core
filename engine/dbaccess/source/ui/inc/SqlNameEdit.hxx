/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <svtools/editbrowsebox.hxx>
#include <utility>
#include <vcl/weld.hxx>

namespace dbaui
{
    class OSQLNameChecker
    {
        OUString m_sAllowedChars;
        bool        m_bCheck;           // true when we should check for invalid chars
    public:
        OSQLNameChecker(OUString _sAllowedChars)
            :m_sAllowedChars(std::move(_sAllowedChars))
            ,m_bCheck(true)
        {
        }

        void setCheck(bool _bCheck)
        {
            m_bCheck = _bCheck;
        }
        bool checkString(std::u16string_view _sToCheck,OUString& _rsCorrected);
    };

    class OWidgetBase
    {
    private:
        weld::Widget* m_pWidget;
        short m_nPos;

    public:
        OWidgetBase(weld::Widget *pWidget, short nPosition);

        void hide() { m_pWidget->hide(); }
        void show() { m_pWidget->show(); }
        void set_sensitive(bool bSensitive) { m_pWidget->set_sensitive(bSensitive); }

        weld::Widget* GetWidget() { return m_pWidget; }

        short GetPos() const { return m_nPos; }

        virtual bool get_value_changed_from_saved() const = 0;
        virtual void save_value() = 0;

        virtual ~OWidgetBase() {}
    };

    class OSQLNameEntry : public OWidgetBase
                        , public OSQLNameChecker
    {
    private:
        std::unique_ptr<weld::Entry> m_xEntry;

        DECL_LINK(ModifyHdl, weld::Entry&, void);

    public:
        OSQLNameEntry(std::unique_ptr<weld::Entry> xEntry, const OUString& _rAllowedChars,
                      short nPosition)
            : OWidgetBase(xEntry.get(), nPosition)
            , OSQLNameChecker(_rAllowedChars)
            , m_xEntry(std::move(xEntry))
        {
            m_xEntry->connect_changed(LINK(this, OSQLNameEntry, ModifyHdl));
        }

        OUString get_text() const { return m_xEntry->get_text(); }
        void set_text(const OUString& rText) { m_xEntry->set_text(rText); }
        void set_max_length(int nLen) { m_xEntry->set_max_length(nLen); }
        virtual void save_value() override { m_xEntry->save_value(); }
        virtual bool get_value_changed_from_saved() const override
        {
            return m_xEntry->get_value_changed_from_saved();
        }
    };

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
