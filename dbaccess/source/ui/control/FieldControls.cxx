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

#include <FieldControls.hxx>
#include <SqlNameEdit.hxx>
#include <core_resource.hxx>

namespace dbaui {

OPropColumnEditCtrl::OPropColumnEditCtrl(std::unique_ptr<weld::Entry> xEntry,
                                         OUString const & _rAllowedChars,
                                         TranslateId pHelpId,
                                         short nPosition)
    : OSQLNameEntry(std::move(xEntry), _rAllowedChars, pHelpId, nPosition)
{
}

OPropEditCtrl::OPropEditCtrl(std::unique_ptr<weld::Entry> xEntry, TranslateId pHelpId, short nPosition)
    : OWidgetBase(xEntry.get(), pHelpId, nPosition)
    , m_xEntry(std::move(xEntry))
{
}

OPropNumericEditCtrl::OPropNumericEditCtrl(std::unique_ptr<weld::SpinButton> xSpinButton, TranslateId pHelpId, short nPosition)
    : OWidgetBase(xSpinButton.get(), pHelpId, nPosition)
    , m_xSpinButton(std::move(xSpinButton))
{
}

OPropListBoxCtrl::OPropListBoxCtrl(std::unique_ptr<weld::ComboBox> xComboBox, TranslateId pHelpId, short nPosition)
    : OWidgetBase(xComboBox.get(), pHelpId, nPosition)
    , m_xComboBox(std::move(xComboBox))
{
}

} // end namespace dbaui
