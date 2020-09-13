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

#include <svtools/editbrowsebox.hxx>
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

    class OSQLNameEditControl : public svt::EditControl
                              , public OSQLNameChecker
    {
    public:
        OSQLNameEditControl(BrowserDataWin* pParent, const OUString& rAllowedChars)
            : svt::EditControl(pParent)
            , OSQLNameChecker(rAllowedChars)
        {
            m_xWidget->connect_changed(LINK(this, OSQLNameEditControl, ModifyHdl));
        }

        virtual void connect_changed(const Link<weld::Entry&, void>& rLink) override
        {
            m_ChainChangedHdl = rLink;
        }

    private:
        DECL_LINK(ModifyHdl, weld::Entry&, void);

        Link<weld::Entry&,void> m_ChainChangedHdl;
    };

    class OWidgetBase
    {
    private:
        weld::Widget* m_pWidget;
    public:
        OWidgetBase(weld::Widget *pWidget)
            : m_pWidget(pWidget)
        {
        }

        void hide() { m_pWidget->hide(); }
        void show() { m_pWidget->show(); }
        void set_sensitive(bool bSensitive) { m_pWidget->set_sensitive(bSensitive); }

        weld::Widget* GetWidget() { return m_pWidget; }

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
        OSQLNameEntry(std::unique_ptr<weld::Entry> xEntry, const OUString& _rAllowedChars = OUString())
            : OWidgetBase(xEntry.get())
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
