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
#ifndef INCLUDED_SVX_RELFLD_HXX
#define INCLUDED_SVX_RELFLD_HXX

#include <tools/fldunit.hxx>
#include <svtools/unitconv.hxx>
#include <vcl/field.hxx>
#include <vcl/weld.hxx>
#include <svx/svxdllapi.h>

class SVX_DLLPUBLIC SvxRelativeField : public MetricField
{
private:
    bool                bRelative;

public:
    SvxRelativeField(vcl::Window* pParent, WinBits nBits, FieldUnit eUnit);

    void            SetRelative( bool bRelative );
};

class SVX_DLLPUBLIC RelativeField
{
private:
    std::unique_ptr<weld::MetricSpinButton> m_xSpinButton;

    sal_uInt16          nRelMin;
    sal_uInt16          nRelMax;
    bool                bRelativeMode;
    bool                bRelative;
    bool                bNegativeEnabled;

    DECL_LINK(ModifyHdl, weld::Entry&, void);

public:
    RelativeField(weld::MetricSpinButton* pControl);

    void            EnableRelativeMode( sal_uInt16 nMin, sal_uInt16 nMax );
    void            SetRelative( bool bRelative );
    bool            IsRelative() const { return bRelative; }
    void            EnableNegativeMode() {bNegativeEnabled = true;}

    void set_sensitive(bool sensitive) { m_xSpinButton->set_sensitive(sensitive); }
    void set_value(int nValue, FieldUnit eValueUnit) { m_xSpinButton->set_value(nValue, eValueUnit); }
    int get_value(FieldUnit eDestUnit) const { return m_xSpinButton->get_value(eDestUnit); }
    int get_min(FieldUnit eValueUnit) const { return m_xSpinButton->get_min(eValueUnit); }
    void set_min(int min, FieldUnit eValueUnit) { m_xSpinButton->set_min(min, eValueUnit); }
    void set_max(int max, FieldUnit eValueUnit) { m_xSpinButton->set_max(max, eValueUnit); }
    int normalize(int nValue) const { return m_xSpinButton->normalize(nValue); }
    int denormalize(int nValue) const { return m_xSpinButton->denormalize(nValue); }
    void connect_value_changed(const Link<weld::MetricSpinButton&, void>& rLink) { m_xSpinButton->connect_value_changed(rLink); }
    OUString get_text() const { return m_xSpinButton->get_text(); }
    void set_text(const OUString& rText) { m_xSpinButton->set_text(rText); }
    void save_value() { m_xSpinButton->save_value(); }
    bool get_value_changed_from_saved() const { return m_xSpinButton->get_value_changed_from_saved(); }

    int GetCoreValue(MapUnit eUnit) const { return ::GetCoreValue(*m_xSpinButton, eUnit); }
    void SetFieldUnit(FieldUnit eUnit, bool bAll = false) { ::SetFieldUnit(*m_xSpinButton, eUnit, bAll); }
    void SetMetricValue(int lCoreValue, MapUnit eUnit) { ::SetMetricValue(*m_xSpinButton, lCoreValue, eUnit); }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
