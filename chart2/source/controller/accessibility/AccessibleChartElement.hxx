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

#include <AccessibleBase.hxx>

namespace com::sun::star::accessibility { class XAccessible; }

namespace chart
{
class AccessibleTextHelper;

/** Base class for all Chart Accessibility objects except the root node (see AccessibleChartView)

    This class contains a reference to the ChartModel, thus, components can easily access all core functionality.

    Usage Instructions:

    <ul>
     <li>define the getAccessibleName() method of XAccessibleContext</li>
     <li>set the ChartModel using SetChartModel() for the first node before
         creating any children</li>
     <li>override UpdateChildren()</li>
    </ul>
 */

class AccessibleChartElement : public AccessibleBase
{
public:
    AccessibleChartElement( const AccessibleElementInfo & rAccInfo,
                            bool bMayHaveChildren );
    virtual ~AccessibleChartElement() override;

    // ________ AccessibleBase ________
    virtual bool ImplUpdateChildren() override;
    virtual css::uno::Reference< css::accessibility::XAccessible >
        ImplGetAccessibleChildById( sal_Int64 i ) const override;
    virtual sal_Int64 ImplGetAccessibleChildCount() const override;

    // ________ XAccessibleContext ________
    virtual OUString SAL_CALL getAccessibleName() override;
    virtual OUString SAL_CALL getAccessibleDescription() override;

    // ________ XAccessibleExtendedComponent ________
    virtual OUString SAL_CALL getToolTipText() override;

    // ________ XServiceInfo ________
    virtual OUString SAL_CALL getImplementationName() override;

private:
    bool                m_bHasText;
    rtl::Reference< ::chart::AccessibleTextHelper >
                        m_xTextHelper;

    void InitTextEdit();
};

}  // namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
