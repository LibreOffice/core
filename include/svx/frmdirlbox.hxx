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

#ifndef INCLUDED_SVX_FRMDIRLBOX_HXX
#define INCLUDED_SVX_FRMDIRLBOX_HXX

#include <vcl/weld.hxx>
#include <editeng/frmdir.hxx>
#include <svx/svxdllapi.h>

class SvxFrameDirectionItem;

namespace svx {

/** This listbox contains entries to select horizontal text direction.

    The control works on the SvxFrameDirection enumeration (i.e. left-to-right,
    right-to-left), used i.e. in conjunction with the SvxFrameDirectionItem.
 */
class SAL_WARN_UNUSED SVX_DLLPUBLIC FrameDirectionListBox
{
private:
    std::unique_ptr<weld::ComboBox> m_xControl;
public:
    explicit FrameDirectionListBox(std::unique_ptr<weld::ComboBox> pControl);
    virtual ~FrameDirectionListBox();
    bool get_visible() const { return m_xControl->get_visible(); }
    void save_value() { m_xControl->save_value(); }
    bool get_value_changed_from_saved() const { return m_xControl->get_value_changed_from_saved(); }
    SvxFrameDirection get_active_id() const { return static_cast<SvxFrameDirection>(m_xControl->get_active_id().toUInt32()); }
    void set_active_id(SvxFrameDirection eDir) { m_xControl->set_active_id(OUString::number(static_cast<sal_uInt32>(eDir))); }
    void remove_id(SvxFrameDirection eDir) { m_xControl->remove_id(OUString::number(static_cast<sal_uInt32>(eDir))); }
    void set_active(int pos) { m_xControl->set_active(pos); }
    int get_active() const { return m_xControl->get_active(); }
    void set_sensitive(bool bSensitive) { m_xControl->set_sensitive(bSensitive); }
    void hide() { m_xControl->hide(); }
    void set_visible(bool bVisible) { m_xControl->set_visible(bVisible); }
    void show() { m_xControl->show(); }
    int get_count() const { return m_xControl->get_count(); }
    /** Inserts a string with corresponding direction enum into the listbox. */
    void append(SvxFrameDirection eDirection, const OUString& rString)
    {
        m_xControl->append(OUString::number(static_cast<sal_uInt32>(eDirection)), rString);
    }
    void connect_changed(const Link<weld::ComboBox&, void>& rLink) { m_xControl->connect_changed(rLink); }
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
