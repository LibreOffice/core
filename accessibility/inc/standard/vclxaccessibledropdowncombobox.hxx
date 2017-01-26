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

#ifndef INCLUDED_ACCESSIBILITY_INC_STANDARD_VCLXACCESSIBLEDROPDOWNCOMBOBOX_HXX
#define INCLUDED_ACCESSIBILITY_INC_STANDARD_VCLXACCESSIBLEDROPDOWNCOMBOBOX_HXX

#include <standard/vclxaccessiblebox.hxx>
#include <standard/vclxaccessibleedit.hxx>
#include <com/sun/star/accessibility/XAccessibleAction.hpp>

/** The accessible drop down combobox has two children.  The first is the
    text field represented by an object of the
    VCLXAccessibleEdit class.  The second is the list
    containing all items and is represented by an object of the
    VCLXAccessibleList class which does not support selection
    at the moment.
*/
class VCLXAccessibleDropDownComboBox : public VCLXAccessibleBox
{
public:
    VCLXAccessibleDropDownComboBox (VCLXWindow* pVCLXindow);

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    // Return drop down combo box specific services.
    virtual css::uno::Sequence< OUString > SAL_CALL
        getSupportedServiceNames() override;

protected:
    virtual ~VCLXAccessibleDropDownComboBox() override;

    virtual bool IsValid() const override;
    virtual void ProcessWindowEvent (const VclWindowEvent& rVclWindowEvent) override;
};

#endif // INCLUDED_ACCESSIBILITY_INC_STANDARD_VCLXACCESSIBLEDROPDOWNCOMBOBOX_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
