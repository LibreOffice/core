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

#ifndef ACCESSIBILITY_STANDARD_VCLXACCESSIBLELISTBOX_HXX
#define ACCESSIBILITY_STANDARD_VCLXACCESSIBLELISTBOX_HXX

#include <accessibility/standard/vclxaccessiblebox.hxx>
#include <com/sun/star/accessibility/XAccessibleAction.hpp>
#include <com/sun/star/accessibility/XAccessibleEventBroadcaster.hpp>
#include <com/sun/star/accessibility/XAccessibleSelection.hpp>


/** The accessible drop down combobox has one children.  It is the list
    containing all items and is represented by an object of the
    <type>VCLXAccessibleListBoxList</type> class which does support
    selection.
*/
class VCLXAccessibleListBox : public VCLXAccessibleBox
{
public:
    VCLXAccessibleListBox (VCLXWindow* pVCLXindow);

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName (void)
        throw (::com::sun::star::uno::RuntimeException);
    // Return list box specific services.
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL
        getSupportedServiceNames (void)
        throw (::com::sun::star::uno::RuntimeException);

protected:
    virtual ~VCLXAccessibleListBox (void);

    virtual bool IsValid (void) const;
    virtual void ProcessWindowEvent (const VclWindowEvent& rVclWindowEvent);
};

#endif // ACCESSIBILITY_STANDARD_VCLXACCESSIBLELISTBOX_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
