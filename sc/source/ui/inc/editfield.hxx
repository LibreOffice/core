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

#ifndef INCLUDED_SC_SOURCE_UI_INC_EDITFIELD_HXX
#define INCLUDED_SC_SOURCE_UI_INC_EDITFIELD_HXX

#include <vcl/edit.hxx>
#include <vcl/weld.hxx>

/** An edit control that contains a double precision floating-point value. */
class ScDoubleField : public Edit
{
public:
    explicit            ScDoubleField( vcl::Window* pParent, WinBits nStyle );

    bool                GetValue( double& rfValue ) const;
    void                SetValue( double fValue,
                                  sal_Int32 nDecPlaces = 12 );
};

class DoubleField
{
private:
    std::unique_ptr<weld::Entry> m_xEntry;
public:
    explicit            DoubleField(std::unique_ptr<weld::Entry> xEntry);

    bool GetValue(double& rfValue) const;
    void SetValue(double fValue, sal_Int32 nDecPlaces = 12);

    weld::Entry& get_widget() { return *m_xEntry; }

    void grab_focus() { m_xEntry->grab_focus(); }
    bool get_sensitive() const { return m_xEntry->get_sensitive(); }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
