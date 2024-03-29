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

#include <standard/vclxaccessibledropdowncombobox.hxx>

#include <comphelper/sequence.hxx>
#include <toolkit/awt/vclxwindow.hxx>
#include <vcl/vclevent.hxx>


using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;


VCLXAccessibleDropDownComboBox::VCLXAccessibleDropDownComboBox (VCLXWindow* pVCLWindow)
    : VCLXAccessibleBox (pVCLWindow, VCLXAccessibleBox::COMBOBOX, true)
{
}


bool VCLXAccessibleDropDownComboBox::IsValid() const
{
    return GetWindow();
}

void VCLXAccessibleDropDownComboBox::ProcessWindowEvent (const VclWindowEvent& rVclWindowEvent)
{
    switch ( rVclWindowEvent.GetId() )
    {
        case VclEventId::DropdownOpen:
        case VclEventId::DropdownClose:
        {
            break;
        }

        default:
            VCLXAccessibleBox::ProcessWindowEvent( rVclWindowEvent );
    }
}


// XServiceInfo

OUString VCLXAccessibleDropDownComboBox::getImplementationName()
{
    return u"com.sun.star.comp.toolkit.AccessibleDropDownComboBox"_ustr;
}


Sequence< OUString > VCLXAccessibleDropDownComboBox::getSupportedServiceNames()
{
    return comphelper::concatSequences(VCLXAccessibleBox::getSupportedServiceNames(),
                                       Sequence<OUString>{u"com.sun.star.accessibility.AccessibleDropDownComboBox"_ustr});
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
