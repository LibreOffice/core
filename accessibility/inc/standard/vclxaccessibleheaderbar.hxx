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
#ifndef INCLUDED_ACCESSIBILITY_INC_STANDARD_VCLXACCESSIBLEHEADERBAR_HXX
#define INCLUDED_ACCESSIBILITY_INC_STANDARD_VCLXACCESSIBLEHEADERBAR_HXX

#include <vcl/headbar.hxx>
#include <toolkit/awt/vclxaccessiblecomponent.hxx>
#include <toolkit/awt/vclxwindow.hxx>

//  class VCLXAccessibleHeaderBar

typedef std::vector< css::uno::WeakReference< css::accessibility::XAccessible > >
    ListItems;

class VCLXAccessibleHeaderBar : public VCLXAccessibleComponent
{

    VclPtr<HeaderBar>  m_pHeadBar;
public:
    virtual ~VCLXAccessibleHeaderBar() override;

    VCLXAccessibleHeaderBar( VCLXWindow* pVCLXindow );

    // XAccessibleContext
    virtual sal_Int32 SAL_CALL getAccessibleChildCount(  ) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleChild( sal_Int32 i ) override;
    virtual sal_Int16 SAL_CALL getAccessibleRole(  ) override;


    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

public:
    virtual void SAL_CALL disposing() override;
    css::uno::Reference< css::accessibility::XAccessible > CreateChild(sal_Int32 i);

private:
    ListItems m_aAccessibleChildren;
};

#endif // INCLUDED_ACCESSIBILITY_INC_STANDARD_VCLXACCESSIBLEHEADERBAR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
